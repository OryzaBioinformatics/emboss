/* @source dbifasta application
**
** Index fasta sequence file databases
**
** @author: Copyright (C) Peter Rice, Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

/******************************************************************************
**
** EMBOSS/Staden/EMBLCD indexing
**
** This version reads a flat file database, in fsata format,
** and writes entryname and field (e.g. accession) index files.
**
** It needs to know the format in order to
** parse the entryname and accession number.
**
** To save memory, it is also helpful to know the maximum number of
** entries in the database and the maximum entryname length so that
** space can be preallocated for storage.
**
** Entry names and accession numbers are held in list structures,
** then converted to arrays and sorted.
**
** Multiple input files are allowed.
**
** EMBLCD and Staden index files use different names but have essentially
** the same contents.
**
******************************************************************************/

#include "emboss.h"



#define FASTATYPE_SIMPLE    1
#define FASTATYPE_IDACC     2
#define FASTATYPE_GCGID     3
#define FASTATYPE_GCGIDACC 4
#define FASTATYPE_NCBI      5
#define FASTATYPE_DBID      6
#define FASTATYPE_ACCID     7
#define FASTATYPE_GCGACCID  8




static AjBool dbifasta_ParseFasta(AjPFile libr, ajint *dpos,
				  AjPStr* id, AjPList* fdl,
				  ajint* maxFieldLen,
				  AjPRegexp exp,
				  ajint type, AjPFile* alistfile,
				  AjBool systemsort, AjPStr const * fields);

static EmbPEntry dbifasta_NextFlatEntry(AjPFile libr, ajint ifile,
					const AjPStr idformat, AjPRegexp exp,
					ajint type, AjBool systemsort,
					AjPStr const * fields,
					ajint* maxFieldLen,
					ajint* maxidlen,
					AjPFile elistfile,
					AjPFile* alistfile);

static AjPRegexp dbifasta_getExpr(const AjPStr idformat, ajint *type);




/* @prog dbifasta *************************************************************
**
** Index a fasta database
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList idlist;
    AjPList* fieldList = NULL;

    AjBool systemsort;
    AjBool cleanup;

    ajint maxindex;
    ajint maxidlen = 0;
    ajint maxlen;

    AjPFile elistfile  = NULL;
    AjPFile* alistfile = NULL;

    AjPStr dbname   = NULL;
    AjPStr release  = NULL;
    AjPStr datestr  = NULL;
    AjPStr sortopt  = NULL;
    void **entryIds = NULL;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr curfilename = NULL;

    AjPFile libr=NULL;
    AjPStr idformat = NULL;

    EmbPEntry entry;
    ajint idtype  = 0;

    ajint idCount = 0;
    ajint idDone;
    AjPList listInputFiles = NULL;
    void ** inputFiles = NULL;
    ajint nfiles;
    ajint ifile;

    ajint filesize;
    short recsize;
    ajint maxfilelen = 20;
    char date[4] =
    {
	0,0,0,0
    };

    AjPStr tmpfname = NULL;
    AjPStr* fields  = NULL;

    AjPFile entFile  = NULL;

    AjPStr* divfiles   = NULL;
    AjPRegexp exp      = NULL;
    ajint* maxFieldLen = NULL;

    ajint ifield  = 0;
    ajint nfields = 0;

    embInit("dbifasta", argc, argv);

    idformat   = ajAcdGetListI("idformat",1);
    fields     = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    indexdir   = ajAcdGetDirectoryName("indexdirectory");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    systemsort = ajAcdGetBool("systemsort");
    cleanup    = ajAcdGetBool("cleanup");
    sortopt    = ajAcdGetString("sortoptions");
    maxindex   = ajAcdGetInt("maxindex");

    while(fields[nfields])		/* array ends with a NULL */
	nfields++;

    if(nfields)
    {
	AJCNEW(maxFieldLen, nfields);
	for(ifield=0; ifield < nfields; ifield++)
	    maxFieldLen[ifield] = -maxindex;

	if(systemsort)
	    AJCNEW(alistfile, nfields);
	else
	{
	    AJCNEW(fieldList, nfields);
	    for(ifield=0; ifield < nfields; ifield++)
		fieldList[ifield] = ajListNew();
	}
    }

    ajStrCleanWhite(&dbname);		/* used for temp filenames */
    embDbiDateSet(datestr, date);
    idlist = ajListNew();

    exp = dbifasta_getExpr(idformat, &idtype);

    ajDebug("reading '%S/%S'\n", directory, filename);
    ajDebug("writing '%S/'\n", indexdir);

    listInputFiles = embDbiFileListExc(directory, filename, exclude);
    ajListSort(listInputFiles, ajStrCmp);
    nfiles = ajListToArray(listInputFiles, &inputFiles);
    if(!nfiles)
	ajFatal("No files selected");

    AJCNEW0(divfiles, nfiles);

    /*
    ** process each input file, one at a time
    */

    for(ifile=0; ifile < nfiles; ifile++)
    {
	curfilename =(AjPStr) inputFiles[ifile];
	embDbiFlatOpenlib(curfilename, &libr);
	ajFileNameTrim(&curfilename);
	if(ajStrLen(curfilename) >= maxfilelen)
	    maxfilelen = ajStrLen(curfilename) + 1;

	ajDebug("processing filename '%S' ...\n", curfilename);
	ajDebug("processing file '%F' ...\n", libr);
	ajStrAssS(&divfiles[ifile], curfilename);

	if(systemsort)	 /* elistfile for entries, alist for fields */
	    elistfile = embDbiSortOpen(alistfile, ifile,
				       dbname, fields, nfields);

	while((entry=dbifasta_NextFlatEntry(libr, ifile, idformat,
					    exp, idtype,
					    systemsort, fields, maxFieldLen,
					    &maxidlen, elistfile, alistfile)))

	{
	    idCount++;
	    if(!systemsort)	    /* save the entry data in lists */
		embDbiMemEntry(idlist, fieldList, nfields, entry, ifile);
	}
	if(systemsort)
	    embDbiSortClose(&elistfile, alistfile, nfields);
    }

    /*  write the division.lkp file */
    embDbiWriteDivision(indexdir, dbname, release, date,
			maxfilelen, nfiles, divfiles, NULL);

    /* Write the entryname.idx index */
    ajStrAssC(&tmpfname, "entrynam.idx");
    entFile = ajFileNewOutD(indexdir, tmpfname);

    recsize = maxidlen+10;
    filesize = 300 + (idCount*(ajint)recsize);
    embDbiHeader(entFile, filesize, idCount, recsize, dbname, release, date);

    if(systemsort)
        idDone = embDbiSortWriteEntry(entFile, maxidlen,
				      dbname, nfiles, cleanup, sortopt);
    else			  /* save entries in entryIds array */
    {
        idDone = embDbiMemWriteEntry(entFile, maxidlen,
				     idlist, &entryIds);
	if(idDone != idCount)
	    ajFatal("Duplicates not allowed for in-memory processing");
    }

    embDbiHeaderSize(entFile, 300+(idDone*(ajint)recsize), idDone);
    ajFileClose(&entFile);

    /* Write the fields index files */
    for(ifield=0; ifield < nfields; ifield++)
    {
        if(maxindex)
	    maxlen = maxindex;
	else
	    maxlen = maxFieldLen[ifield];

        if(systemsort)
	    embDbiSortWriteFields(dbname, release, date, indexdir,
				  fields[ifield], maxlen,
				  nfiles, idCount, cleanup, sortopt);
	else
	    embDbiMemWriteFields(dbname, release, date, indexdir,
				 fields[ifield], maxlen,
				 fieldList[ifield], entryIds);
    }

    if(systemsort)
	embDbiRmEntryFile(dbname, cleanup);

    ajListDel(&listInputFiles);

    ajRegFree(&exp);

    ajExit();

    return 0;
}




/* @funcstatic dbifasta_NextFlatEntry *****************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] libr [AjPFile] Database file
** @param [r] ifile [ajint] File number.
** @param [r] idformat [const AjPStr] type of id line
** @param [u] exp [AjPRegexp] Regular expression for id parsing
** @param [r] type [ajint] type of fasta id.
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] maxidlen [ajint*] Maximum entry ID length
** @param [u] elistfile [AjPFile] entry file
** @param [u] alistfile [AjPFile*] field data files array
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbifasta_NextFlatEntry(AjPFile libr, ajint ifile,
					const AjPStr idformat, AjPRegexp exp,
					ajint type, AjBool systemsort,
					AjPStr const * fields,
					ajint* maxFieldLen,
					ajint* maxidlen,
					AjPFile elistfile, AjPFile* alistfile)
{

    static EmbPEntry ret = NULL;
    ajint ir;
    ajint is = 0;
    static AjPStr id = NULL;
    char* token;
    ajint i;
    static AjPList* fdl = NULL;
    static ajint nfields;
    ajint ifield;

    if(!fdl)
    {
	nfields = 0;
	while(fields[nfields])
	    nfields++;
	if(nfields)
	    AJCNEW(fdl, nfields);
	for(i=0; i < nfields; i++)
	{
	    fdl[i] = ajListNew();
	}
    }

    if(!ret || !systemsort)
	ret = embDbiEntryNew(nfields);

    if(!dbifasta_ParseFasta(libr, &ir, &id, fdl, maxFieldLen, exp, type,
			    alistfile, systemsort, fields))
	return NULL;

    /* id to ret->entry */
    if(ajStrLen(id) > *maxidlen)
	*maxidlen = ajStrLen(id);

    if(systemsort)
	ajFmtPrintF(elistfile, "%S %d %d %d\n", id, ir, is, ifile+1);
    else
    {
	ret->entry   = ajCharNew(id);
	ret->rpos    = ir;
	ret->spos    = is;
	ret->filenum = ifile+1;

	/* field tokens as list, then move to ret->field */
	for(ifield=0; ifield < nfields; ifield++)
	{
	    ret->nfield[ifield] = ajListLength(fdl[ifield]);

	    if(ret->nfield[ifield])
	    {
		AJCNEW(ret->field[ifield],ret->nfield[ifield]);

		i = 0;
		while(ajListPop(fdl[ifield], (void**) &token))
		    ret->field[ifield][i++] = token;
	    }
	    else
		ret->field[ifield] = NULL;
	}
    }

    return ret;
}




/* @funcstatic dbifasta_getExpr ***********************************************
**
** Compile regular expression
**
** @param [r] idformat [const AjPStr] type of ID line
** @param [w] type [ajint *] numeric type
** @return [AjPRegexp] ajTrue on success.
** @@
******************************************************************************/

static AjPRegexp dbifasta_getExpr(const AjPStr idformat, ajint *type)
{
    AjPRegexp exp = NULL;

    if(ajStrMatchC(idformat,"simple"))
    {
	*type = FASTATYPE_SIMPLE;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"idacc"))
    {
	*type = FASTATYPE_IDACC;
	exp   = ajRegCompC("^>([.A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"accid"))
    {
	*type = FASTATYPE_ACCID;
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgid"))
    {
	*type = FASTATYPE_GCGID;
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgidacc"))
    {
	*type = FASTATYPE_GCGIDACC;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgaccid"))
    {
	*type = FASTATYPE_GCGACCID;
	exp   = ajRegCompC(
		     "^>[A-Za-z0-9_-]+:([A-Za-z0-9_-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"ncbi"))
    {
	exp   = ajRegCompC("^>([A-Za-z0-9_-]+)"); /* dummy regexp */
	*type = FASTATYPE_NCBI;
    }
    else if(ajStrMatchC(idformat,"dbid"))
    {
	exp   = ajRegCompC("^>[A-Za-z0-9_-]+[ \t]+([A-Za-z0-9_-]+)");
	*type = FASTATYPE_DBID;
    }
    else
	return NULL;

    return exp;
}




/* @funcstatic dbifasta_ParseFasta ********************************************
**
** Parse the ID, accession from a FASTA format sequence
**
** @param [u] libr [AjPFile] Input database file
** @param [w] dpos [ajint*] Byte offset
** @param [w] id [AjPStr*] ID
** @param [w] fdlist [AjPList*] Lists of field tokens
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [u] exp [AjPRegexp] regular expression
** @param [r] type [ajint] type of id line
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbifasta_ParseFasta(AjPFile libr, ajint* dpos,
				  AjPStr* id, AjPList* fdlist,
				  ajint* maxFieldLen, AjPRegexp exp,
				  ajint type, AjPFile* alistfile,
				  AjBool systemsort, AjPStr const * fields)
{
    static AjPRegexp wrdexp = NULL;
    static AjPStr tmpac  = NULL;
    static AjPStr tmpsv  = NULL;
    static AjPStr tmpgi  = NULL;
    static AjPStr tmpdes = NULL;
    static AjPStr tmpfd  = NULL;
    static AjPStr rline  = NULL;
    char* fd;
    ajint ipos;
    static AjPStr tstr = NULL;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    if(ajStrMatchCaseC(fields[numFields], "acnum"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "seqvn"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else
		ajWarn("EMBL parsing unknown field '%S' ignored",
		       fields[numFields]);

	    numFields++;
	}
	reset = ajFalse;
    }

    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9]+)");

    if(!tstr)
	tstr = ajStrNew();

    *dpos = ajFileTell(libr);
    ajFileGets(libr, &rline);

    if(!ajRegExec(exp,rline))
    {
	ajStrDelReuse(&tmpac);
	ajDebug("Invalid ID line [%S]",rline);
	return ajFalse;
    }

    /*
    ** each case needs to set id, tmpac, tmpsv, tmpdes
    ** using empty values if they are not found
    */

    ajStrAssC(&tmpsv, "");
    ajStrAssC(&tmpgi, "");
    ajStrAssC(&tmpdes, "");
    ajStrAssC(&tmpac, "");
    ajStrAssC(id, "");

    switch(type)
    {
    case FASTATYPE_SIMPLE:
	ajRegSubI(exp,1,id);
	ajStrAssS(&tmpac,*id);
	ajRegPost(exp, &tmpdes);
	break;
    case FASTATYPE_DBID:
	ajRegSubI(exp,1,id);
	ajStrAssS(&tmpac,*id);
	ajRegPost(exp, &tmpdes);
	break;
    case FASTATYPE_GCGID:
	ajRegSubI(exp,1,id);
	ajStrAssS(&tmpac,*id);
	ajRegPost(exp, &tmpdes);
	break;
    case FASTATYPE_NCBI:
	if(!ajSeqParseNcbi(rline,id,&tmpac,&tmpsv,&tmpgi,&tmpdes))
	{
	    ajStrDelReuse(&tmpac);
	    return ajFalse;
	}
	break;
    case FASTATYPE_GCGIDACC:
	ajRegSubI(exp,1,id);
	ajRegSubI(exp,2,&tmpac);
	ajRegPost(exp, &tmpdes);
	break;
    case FASTATYPE_GCGACCID:
	ajRegSubI(exp,1,&tmpac);
	ajRegSubI(exp,2,id);
	ajRegPost(exp, &tmpdes);
	break;
    case FASTATYPE_IDACC:
	ajRegSubI(exp,1,id);
	ajRegSubI(exp,2,&tmpac);
	ajRegPost(exp, &tmpdes);
	break;
    case FASTATYPE_ACCID:
	ajRegSubI(exp,1,&tmpac);
	ajRegSubI(exp,2,id);
	ajRegPost(exp, &tmpdes);
	break;
    default:
	ajStrDelReuse(&tmpac);
	return ajFalse;
    }

    ajStrToUpper(id);
    ajStrToUpper(&tmpac);

    if(accfield >= 0)
	embDbiMaxlen(&tmpac, &maxFieldLen[accfield]);
    if(svnfield >= 0)
    {
	embDbiMaxlen(&tmpsv, &maxFieldLen[svnfield]);
	embDbiMaxlen(&tmpgi, &maxFieldLen[svnfield]);
    }

    if(systemsort)
    {
	if(accfield >= 0 && ajStrLen(tmpac))
	    ajFmtPrintF(alistfile[accfield],"%S %S\n",*id,tmpac);

	if(svnfield >= 0 && ajStrLen(tmpsv))
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *id, tmpsv);

	if(svnfield >= 0 && ajStrLen(tmpgi))
	    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *id, tmpgi);

	if(desfield >= 0 && ajStrLen(tmpdes))
	    while(ajRegExec(wrdexp, tmpdes))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);
		ajStrToUpper(&tmpfd);
		ajDebug("++des '%S' tmpdes '%S\n", tmpfd, tmpdes);
		ajFmtPrintF(alistfile[desfield], "%S %S\n", *id, tmpfd);
		ajRegPost(wrdexp, &tmpdes);
	    }
    }
    else
    {
	if(accfield >= 0 && ajStrLen(tmpac))
	{
	    fd = ajCharNew(tmpac);
	    ajListPushApp(fdlist[accfield],fd);
	}

	if(svnfield >= 0 && ajStrLen(tmpsv))
	{
	    fd = ajCharNew(tmpsv);
	    ajListPushApp(fdlist[svnfield], fd);
	}

	if(svnfield >= 0 && ajStrLen(tmpgi))
	{
	    fd = ajCharNew(tmpgi);
	    ajListPushApp(fdlist[svnfield], fd);
	}

	if(desfield >= 0 && ajStrLen(tmpdes))
	    while(ajRegExec(wrdexp, tmpdes))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);
		ajStrToUpper(&tmpfd);
		ajDebug("++des '%S' tmpdes: '%S'\n", tmpfd, tmpdes);
		fd = ajCharNew(tmpfd);
		ajListPushApp(fdlist[desfield], fd);
		ajRegPost(wrdexp, &tmpdes);
	    }
    }

    ajStrDelReuse(&tmpac);
    ajStrDelReuse(&tmpsv);
    ajStrDelReuse(&tmpgi);
    ajStrDelReuse(&tmpdes);

    ipos = ajFileTell(libr);

    while(ajFileGets(libr, &rline))
    {
	if(ajStrChar(rline,0) == '>')
	{
	    ajFileSeek(libr, ipos, 0);
	    return ajTrue;
	}
	ipos = ajFileTell(libr);
    }

    ajFileSeek(libr, ipos, 0);		/* end of file reached */

    return ajTrue;
}

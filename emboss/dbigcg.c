/* @source dbigcg application
**
** Index gcg/pir/accelrys databases
**
** @author Copyright (C) Peter Rice, Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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
** This version reads a GCG formatted database,
** and writes entryname and accession index files.
**
** It needs to know the reference text format in order to
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
#ifndef WIN32
#include <dirent.h>
#include <sys/wait.h>
#endif
#include <string.h>

#define GCGTYPE_OTHER 0
#define GCGTYPE_ID 1
#define GCGTYPE_ACC 2
#define GCGTYPE_DES 3
#define GCGTYPE_KEY 4
#define GCGTYPE_TAX 5
#define GCGTYPE_VER 6

/* Definition of global variables */

static AjPList* dbigcgGFdl  = NULL;

static AjPRegexp dbigcgGRegGetentR     = NULL;
static AjPRegexp dbigcgGRegGetentS     = NULL;
static AjPRegexp dbigcgGRegGetentSplit = NULL;

static AjPRegexp dbigcgGRegEmblTyp = NULL;
static AjPRegexp dbigcgGRegEmblId  = NULL;
static AjPRegexp dbigcgGRegEmblVer = NULL;
static AjPRegexp dbigcgGRegEmblWrd = NULL;
static AjPRegexp dbigcgGRegEmblPhr = NULL;
static AjPRegexp dbigcgGRegEmblTax = NULL;
static AjPRegexp dbigcgGRegEmblPir = NULL;

static AjPRegexp dbigcgGRegPirId   = NULL;
static AjPRegexp dbigcgGRegPirAc1  = NULL;
static AjPRegexp dbigcgGRegPirAc2  = NULL;
static AjPRegexp dbigcgGRegPirKey  = NULL;
static AjPRegexp dbigcgGRegPirTax1 = NULL;
static AjPRegexp dbigcgGRegPirTax2 = NULL;
static AjPRegexp dbigcgGRegPirWrd  = NULL;
static AjPRegexp dbigcgGRegPirPhr  = NULL;

static EmbPEntry dbigcgGEntry = NULL;

static EmbPEntry dbigcg_nextentry(AjPFile libr, AjPFile libs,
				  ajuint ifile, const AjPStr idformat,
				  AjBool systemsort,
				  AjPStr const * fields, ajint* maxFieldLen,
				  ajuint* maxidlen, ajuint* countfield,
				  AjPFile elistfile, AjPFile* alistfile);
static AjBool dbigcg_gcgopenlib(const AjPStr lname,
				AjPFile* libr, AjPFile* lib);
static ajint dbigcg_gcggetent(const AjPStr idformat,
			      AjPFile libr, AjPFile libs,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr const * fields,
			      ajint* maxFieldLen, ajuint* countfield,
			      AjPStr * libstr);
static ajint dbigcg_pirgetent(const AjPStr idformat,
			      AjPFile libr, AjPFile libs, AjPFile* alistfile,
			      AjBool systemsort, AjPStr const * fields,
			      ajint* maxFieldLen, ajuint* countfield,
			      AjPStr* libstr);
static ajint dbigcg_gcgappent(AjPFile libr, AjPFile libs,
			      AjPRegexp rexp, AjPRegexp sexp,
			      AjPStr* libstr);

static AjBool dbigcg_ParseEmbl(AjPFile libr,
			       AjPFile* alistfile,
			       AjBool systemsort, AjPStr const * fields,
			       ajint* maxFieldLen, ajuint* countfield,
			       AjPStr *id);
static AjBool dbigcg_ParsePir(AjPFile libr,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr const * fields,
			      ajint* maxFieldLen, ajuint* countfield,
			      AjPStr *id);
static AjBool dbigcg_ParseGenbank(AjPFile libr,
				  AjPFile* alistfile,
				  AjBool systemsort, AjPStr const * fields,
				  ajint* maxFieldLen, ajuint* countfield,
				  AjPStr *id);




/* @datastatic OParser **************************************************
**
** Parser definition structure
**
** @alias SParser
**
** @attr Name [const char*] Parser name
** @attr GcgFormat [AjBool] If true, use GCG format, if false, use NBRF
** @attr Padding [ajint] Padding to alignment boundary
** @attr Parser [AjBool function] Parser function
** @@
******************************************************************************/

typedef struct SParser
{
    const char* Name;
    AjBool GcgFormat;
    ajint  Padding;
    AjBool (*Parser) (AjPFile libr,
		      AjPFile* alistfile,
		      AjBool systemsort, AjPStr const * fields,
		      ajint* maxFieldLen, ajuint* countfield,
		      AjPStr *id);
} OParser;




static OParser parser[] =
{
    { "EMBL", AJTRUE, 0, dbigcg_ParseEmbl },
    { "SWISS", AJTRUE, 0, dbigcg_ParseEmbl },
    { "GENBANK", AJTRUE, 0, dbigcg_ParseGenbank },
    { "PIR", AJFALSE, 0, dbigcg_ParsePir },
    { NULL, 0, 0, NULL }
};




/* @prog dbigcg ***************************************************************
**
** Index a GCG formatted database
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPList idlist;
    AjPList* fieldList = NULL;

    AjBool systemsort;
    AjBool cleanup;

    ajint maxindex;
    ajuint maxidlen = 0;
    ajuint maxlen;

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

    AjPFile libr = NULL;
    AjPFile libs = NULL;
    AjPStr idformat = NULL;

    EmbPEntry entry;

    ajuint idCount = 0;
    ajuint idDone;
    AjPList listInputFiles = NULL;
    void ** inputFiles = NULL;
    ajint nfiles;
    ajint ifile;

    ajint filesize;
    short recsize;
    ajuint maxfilelen = 20;
    char date[4] =
    {
	0,0,0,0
    };

    AjPStr tmpfname = NULL;
    AjPStr* fields  = NULL;

    AjPFile entFile = NULL;

    AjPStr* reffiles = NULL;
    AjPStr* seqfiles = NULL;
    ajint* maxFieldLen = NULL;

    ajint ifield  = 0;
    ajint nfields = 0;

    AjPFile logfile = NULL;
    ajuint* countField = NULL;
    ajuint* fieldTot = NULL;
    ajuint idCountFile = 0;
    ajint i;

    embInit("dbigcg", argc, argv);

    idformat   = ajAcdGetListSingle("idformat");
    fields     = ajAcdGetList("fields");
    directory  = ajAcdGetDirectoryName("directory");
    indexdir   = ajAcdGetOutdirName("indexoutdir");
    filename   = ajAcdGetString("filenames");
    exclude    = ajAcdGetString("exclude");
    dbname     = ajAcdGetString("dbname");
    release    = ajAcdGetString("release");
    datestr    = ajAcdGetString("date");
    systemsort = ajAcdGetBoolean("systemsort");
    cleanup    = ajAcdGetBoolean("cleanup");
    sortopt    = ajAcdGetString("sortoptions");
    maxindex   = ajAcdGetInt("maxindex");
    logfile    = ajAcdGetOutfile("outfile");

    while(fields[nfields])		/* array ends with a NULL */
	nfields++;

    if(nfields)
    {
	AJCNEW(maxFieldLen, nfields);
	AJCNEW0(countField, nfields);
	AJCNEW0(fieldTot, nfields);
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

    if(ajStrMatchC(datestr, "00/00/00"))
	ajFmtPrintS(&datestr, "%D", ajTimeRefTodayFmt("dbindex"));

    ajStrRemoveWhite(&dbname);		/* used for temp filenames */
    embDbiDateSet(datestr, date);
    idlist = ajListNew();

    ajDebug("reading '%S/%S'\n", directory, filename);
    ajDebug("writing '%S/'\n", indexdir);

    listInputFiles = embDbiFileListExc(directory, filename, exclude);
    ajListSort(listInputFiles, &ajStrVcmp);
    nfiles = (ajuint) ajListToarray(listInputFiles, &inputFiles);
    if(!nfiles)
        ajDie("No input files in '%S' matched filename '%S'",
              directory, filename);

    embDbiLogHeader(logfile, dbname, release, datestr,
		     indexdir, maxindex);

    embDbiLogFields(logfile, fields, nfields);
    embDbiLogSource(logfile, directory, filename, exclude,
		    (AjPStr*) inputFiles, nfiles);
    embDbiLogCmdline(logfile);

    AJCNEW0(reffiles, nfiles);
    AJCNEW0(seqfiles, nfiles);

    /* process each input file, one at a time */

    for(ifile=0; ifile < nfiles; ifile++)
    {
	ajStrAssignS(&curfilename, (AjPStr) inputFiles[ifile]);
	dbigcg_gcgopenlib(curfilename, &libr, &libs);

	ajFmtPrintS(&reffiles[ifile], "%F", libr);
	ajFilenameTrimPath(&reffiles[ifile]);
	ajFmtPrintS(&seqfiles[ifile], "%F", libs);
	ajFilenameTrimPath(&seqfiles[ifile]);
	ajDebug("processing filename '%S' ...\n", curfilename);
	ajDebug("processing reffile '%S' ...\n", reffiles[ifile]);
	ajDebug("processing seqfile '%S' ...\n", seqfiles[ifile]);
	if((ajStrGetLen(reffiles[ifile])+
	    ajStrGetLen(seqfiles[ifile])) >= maxfilelen)
	    maxfilelen = ajStrGetLen(reffiles[ifile]) +
		ajStrGetLen(seqfiles[ifile])+2;

	if(systemsort)	 /* elistfile for entries, alist for fields */
	    elistfile = embDbiSortOpen(alistfile, ifile,
				       dbname, fields, nfields);

	idCountFile = 0;
	for(i=0;i<nfields;i++)
	    countField[i] = 0;
	while((entry=dbigcg_nextentry(libr, libs,
				      ifile, idformat,
				      systemsort, fields, maxFieldLen,
				      &maxidlen, countField,
				      elistfile, alistfile)))
	{
	    idCountFile++;
	    if(!systemsort)	    /* save the entry data in lists */
	    {
		embDbiMemEntry(idlist, fieldList, nfields, entry, ifile);
		entry = NULL;
	    }
	}
	idCount += idCountFile;
	if(systemsort)
	{
	    embDbiSortClose(&elistfile, alistfile, nfields);
	    AJFREE(entry);
	}
	else
	{
	    embDbiEntryDel(&dbigcgGEntry);
	}
	embDbiLogFile(logfile, curfilename, idCountFile, fields,
		      countField, nfields);
    }


    /* write the division.lkp file */
    embDbiWriteDivision(indexdir, dbname, release, date,
			maxfilelen, nfiles, reffiles, seqfiles);


    /* Write the entryname.idx index */
    ajStrAssignC(&tmpfname, "entrynam.idx");
    entFile = ajFileNewOutNamePathS(tmpfname, indexdir);

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
	{
	    if(maxFieldLen[ifield] >= 0)
		maxlen = maxFieldLen[ifield];
	    else
		maxlen = - maxFieldLen[ifield];
	}

        if(systemsort)
	    fieldTot[ifield] = embDbiSortWriteFields(dbname, release,
						     date, indexdir,
						     fields[ifield], maxlen,
						     nfiles, idCount,
						     cleanup, sortopt);
	else
	    fieldTot[ifield] = embDbiMemWriteFields(dbname, release,
						    date, indexdir,
						    fields[ifield], maxlen,
						    fieldList[ifield],
						    entryIds);
    }

    embDbiLogFinal(logfile,maxindex, maxFieldLen, fields, fieldTot,
		   nfields, nfiles, idDone, idCount);

    if(systemsort)
	embDbiRmEntryFile(dbname, cleanup);

    ajStrDel(&idformat);
    ajStrDel(&directory);
    ajStrDel(&indexdir);
    ajStrDel(&filename);
    ajStrDel(&exclude);
    ajStrDel(&dbname);
    ajStrDel(&release);
    ajStrDel(&datestr);
    ajStrDel(&sortopt);
    ajFileClose(&logfile);
    ajStrDelarray(&fields);
    ajFileClose(&elistfile);


    for(i=0;i<nfields;i++)
    {
	if(systemsort)
	{
	    ajFileClose(&alistfile[i]);
	}
	else
	{
	    ajListMap(fieldList[i], &embDbiFieldDelMap, NULL);
	    ajListFree(&fieldList[i]);
	}
    }

    AJFREE(alistfile);
    AJFREE(fieldList);
    AJFREE(maxFieldLen);
    AJFREE(countField);
    AJFREE(fieldTot);

    for(i=0;i<nfiles;i++)
    {
	ajStrDel(&reffiles[i]);
	ajStrDel(&seqfiles[i]);
    }
    AJFREE(reffiles);
    AJFREE(seqfiles);
    AJFREE(inputFiles);
 
    ajStrDel(&tmpfname);

    ajFileClose(&libr);
    ajFileClose(&libs);

    ajRegFree(&dbigcgGRegGetentR);
    ajRegFree(&dbigcgGRegGetentS);
    ajRegFree(&dbigcgGRegGetentSplit);
    ajRegFree(&dbigcgGRegEmblTyp);
    ajRegFree(&dbigcgGRegEmblId);
    ajRegFree(&dbigcgGRegEmblVer);
    ajRegFree(&dbigcgGRegEmblWrd);
    ajRegFree(&dbigcgGRegEmblPhr);
    ajRegFree(&dbigcgGRegEmblTax);
    ajRegFree(&dbigcgGRegEmblPir);
    ajRegFree(&dbigcgGRegPirId);
    ajRegFree(&dbigcgGRegPirAc1);
    ajRegFree(&dbigcgGRegPirAc2);
    ajRegFree(&dbigcgGRegPirKey);
    ajRegFree(&dbigcgGRegPirTax1);
    ajRegFree(&dbigcgGRegPirTax2);
    ajRegFree(&dbigcgGRegPirWrd);
    ajRegFree(&dbigcgGRegPirPhr);

    embDbiEntryDel(&dbigcgGEntry);

    if(dbigcgGFdl)
    {
	for(i=0; i < nfields; i++)
	    ajListFree(&dbigcgGFdl[i]);
	AJFREE(dbigcgGFdl);
    }

    ajListMap(idlist, &embDbiEntryDelMap, NULL);
    ajListFree(&idlist);
    ajListstrFreeData(&listInputFiles);
    AJFREE(entryIds);
    ajStrDel(&curfilename);

    embExit();

    return 0;
}




/* @funcstatic dbigcg_nextentry ***********************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [u] libr [AjPFile] Reference file
** @param [u] libs [AjPFile] Sequence file
** @param [r] ifile [ajuint] File number.
** @param [r] idformat [const AjPStr] Id format in GCG file
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const*] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] maxidlen [ajuint*] Maximum entry ID length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [u] elistfile [AjPFile] entry file
** @param [u] alistfile [AjPFile*] field data files array
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbigcg_nextentry(AjPFile libr, AjPFile libs,
				  ajuint ifile, const AjPStr idformat,
				  AjBool systemsort,
				  AjPStr const * fields, ajint* maxFieldLen,
				  ajuint* maxidlen, ajuint* countfield,
				  AjPFile elistfile, AjPFile* alistfile)
{
    ajlong ir;
    ajlong is = 0L;
    AjPStr id = NULL;
    char* token;
    char *p;
    char *q;
    ajint i;
    static ajint nfields;
    ajint ifield;

    if(!dbigcgGFdl)
    {
	nfields = 0;
	while(fields[nfields])
	    nfields++;

	if(nfields)
	    AJCNEW(dbigcgGFdl, nfields);

	for(i=0; i < nfields; i++)
	    dbigcgGFdl[i] = ajListNew();
    }

    ir = ajFileResetPos(libr);
    is = ajFileResetPos(libs);

    if(!dbigcgGEntry || !systemsort)
	dbigcgGEntry = embDbiEntryNew(nfields);

    if(!dbigcg_gcggetent(idformat, libr, libs,
			 alistfile, systemsort, fields, maxFieldLen,
			 countfield, &id) &&
       !dbigcg_pirgetent(idformat, libr, libs,
			 alistfile, systemsort, fields, maxFieldLen,
			 countfield, &id))
	return NULL;

    ajDebug("id '%S' ir:%Ld is:%Ld nfields: %d\n",
	    id, ir, is, nfields);

    p = ajCharNewS(id);

    if(ajStrSuffixC(id,"_0") ||
       ajStrSuffixC(id,"_00") ||
       ajStrSuffixC(id,"_000") ||
       ajStrSuffixC(id,"_0000"))
    {
        q = strrchr(p, '_');
	*q = '\0';
    }

    if(ajStrGetLen(id) > *maxidlen)
	*maxidlen = ajStrGetLen(id);

    if(systemsort)
    {
      ajDebug("write tmp '%s' ir '%Lu' is '%Lu' div %u\n",
	      p, ir, is, (ajuint) (ifile+1));

	ajFmtPrintF(elistfile, "%s %Lu %Lu %d\n",
		    p,
		    ir, is, ifile+1);
        ajCharDel(&p);
    }
    else
    {
	dbigcgGEntry->entry   = p;
	dbigcgGEntry->rpos    = (ajuint) ir; /* Lossy cast */
	dbigcgGEntry->spos    = (ajuint) is; /* Lossy cast */
	dbigcgGEntry->filenum = ifile+1;

	/* field tokens as list, then move to dbigcgGEntry->field */
	for(ifield=0; ifield < nfields; ifield++)
	{
	    dbigcgGEntry->nfield[ifield] = (ajuint) ajListGetLength(dbigcgGFdl[ifield]);

	    if(dbigcgGEntry->nfield[ifield])
	    {
	        AJCNEW(dbigcgGEntry->field[ifield],
		       dbigcgGEntry->nfield[ifield]);

		i=0;
		while(ajListPop(dbigcgGFdl[ifield], (void**) &token))
		    dbigcgGEntry->field[ifield][i++] = token;
	    }
	    else
	        dbigcgGEntry->field[ifield] = NULL;
	}
    }

    ajStrDel(&id);

    return dbigcgGEntry;
}




/* @funcstatic dbigcg_gcgopenlib **********************************************
**
** Open a GCG library
**
** @param [r] lname [const AjPStr] Source file basename
** @param [u] libr [AjPFile*] Reference file
** @param [u] libs [AjPFile*] Sequence file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbigcg_gcgopenlib(const AjPStr lname,
				AjPFile* libr, AjPFile* libs)
{
    AjPStr rname = NULL;
    AjPStr sname = NULL;

    ajStrAssignS(&rname, lname);
    ajStrAssignS(&sname, lname);

    ajFilenameReplaceExtC(&rname,"ref");
    ajFilenameReplaceExtC(&sname,"seq");

    ajFileClose(libr);
    ajFileClose(libs);

    *libr = ajFileNewInNameS(rname);
    if(!*libr)
	ajFatal("Failed to open %S for reading",rname);

    *libs = ajFileNewInNameS(sname);
    if(!*libs)
	ajFatal("Failed to open %S for reading",sname);

    ajStrDel(&rname);
    ajStrDel(&sname);

    return ajTrue;
}




/* @funcstatic dbigcg_gcggetent ***********************************************
**
** get a single entry from the GCG database files
**
** @param [r] idformat [const AjPStr] Id format in FASTA file
** @param [u] libr [AjPFile] Reference file
** @param [u] libs [AjPFile] Sequence file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const*] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] libstr [AjPStr*] ID
** @return [ajint] Sequence length
** @@
******************************************************************************/

static ajint dbigcg_gcggetent(const AjPStr idformat,
			      AjPFile libr, AjPFile libs,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr const * fields,
			      ajint* maxFieldLen, ajuint* countfield,
			      AjPStr* libstr)
{
    AjPStr gcgtype   = NULL;
    static ajint gcglen;
    ajint rblock;
    AjPStr reflibstr = NULL;
    ajint i;
    AjPStr tmpstr  = NULL;
    static ajint called   = 0;
    static ajint iparser  = -1;
    AjPStr rline = NULL;
    AjPStr sline = NULL;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(idformat, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("idformat '%S' unknown", idformat);

	ajDebug("idformat '%S' Parser %d\n", idformat, iparser);
	called = 1;
    }

    if(!parser[iparser].GcgFormat)
	return 0;

    if(!dbigcgGRegGetentR)
	dbigcgGRegGetentR = ajRegCompC("^>>>>([^ \t\r\n]+)");

    if(!dbigcgGRegGetentS)
	dbigcgGRegGetentS =
	    ajRegCompC("^>>>>(\\S+)\\s+.*\\s+(\\S+)\\s+Len:\\s+(\\d+)");

    if(!dbigcgGRegGetentSplit)
	dbigcgGRegGetentSplit = ajRegCompC("_0+$");


    /* check for seqid first line */
    while(ajStrGetCharFirst(sline)!='>')
    {
	if(!ajReadline(libs, &sline))
	{
	    ajStrDel(&sline);
	    return 0;			/* end of file */
	}

	ajDebug("... read until next seq %d '%S'\n",
		ajFileResetPos(libs), sline);
    }

    ajDebug("dbigcg_gcggetent .seq (%S) %d '%S'\n",
	    idformat, ajFileResetPos(libs), sline);

    /* get the encoding/sequence length info */
    if(!ajRegExec(dbigcgGRegGetentS, sline))
    {
        ajDebug("dbigcg_gcggetent sequence expression FAILED\n");
	ajStrDel(&sline);
	return 0;
    }

    ajRegSubI(dbigcgGRegGetentS, 1, libstr);		/* Entry ID returned */

    ajRegSubI(dbigcgGRegGetentS, 2, &gcgtype);
    ajRegSubI(dbigcgGRegGetentS, 3, &tmpstr);
    ajStrToInt(tmpstr, &gcglen);

    ajDebug("new entry '%S' type:'%S' len:'%S'=%d\n",
	    *libstr, gcgtype, tmpstr, gcglen);

    ajStrAssignC(&rline, "");

    ajDebug("dbigcg_gcggetent .ref (%S) %d '%S'\n",
	    idformat, ajFileResetPos(libr), rline);

    /* check for refid first line */
    while(ajStrGetCharFirst(rline)!='>')
    {
	if(!ajReadline(libr, &rline))
	{
	    ajErr("ref ended before seq");
	    break;			/* end of file */
	}
	ajDebug("... read until next ref %d '%S'\n", ajFileResetPos(libr), rline);
    }

    /* get the encoding/sequence length info */

    ajRegExec(dbigcgGRegGetentR, rline);
    ajRegSubI(dbigcgGRegGetentR, 1, &reflibstr);

    (*parser[iparser].Parser)(libr, alistfile, systemsort,
                              fields, maxFieldLen, countfield,
                              &reflibstr); /* writes alistfile data */

    /* get the description line */
    ajReadline(libs, &sline);

    /* seek to the end of the sequence; +1 to jump over newline */
    if(ajStrGetCharFirst(gcgtype)=='2')
    {
	rblock = (gcglen+3)/4;
	ajFileSeek(libs,rblock+1,SEEK_CUR);
    }
    else ajFileSeek(libs,gcglen+1,SEEK_CUR);

    /*
    **  for big entries, need to append until we have all the parts.
    **  They are named with _0 on the first part, _1 on the second and so on.
    **  or _00 on the first part, _01 on the second and so on.
    **  We can look for the "id_" prefix.
    */

    ajDebug("libstr '%S'\n", *libstr);
    if(ajRegExec(dbigcgGRegGetentSplit, *libstr))
	gcglen += dbigcg_gcgappent(libr, libs,
				   dbigcgGRegGetentR, dbigcgGRegGetentS,
				   libstr);

    ajStrDel(&gcgtype);
    ajStrDel(&reflibstr);
    ajStrDel(&tmpstr);
    ajStrDel(&rline);
    ajStrDel(&sline);

    return gcglen;
}




/* @funcstatic dbigcg_pirgetent ***********************************************
**
** get a single entry from the PIR database files
**
** @param [r] idformat [const AjPStr] Id format in FASTA file
** @param [u] libr [AjPFile] Reference file
** @param [u] libs [AjPFile] Sequence file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const*] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] libstr [AjPStr*] ID
** @return [ajint] Sequence length
** @@
******************************************************************************/

static ajint dbigcg_pirgetent(const AjPStr idformat,
			      AjPFile libr, AjPFile libs,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr const * fields,
			      ajint* maxFieldLen, ajuint* countfield,
			      AjPStr* libstr)
{
    AjPStr reflibstr = NULL;
    ajint i;
    static ajint called  = 0;
    static ajint iparser = -1;
    ajint gcglen;
    AjPStr rline = NULL;
    AjPStr sline = NULL;
    ajlong spos = 0;

    if(!called)
    {
	for(i=0; parser[i].Name; i++)
	    if(ajStrMatchC(idformat, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if(iparser < 0)
	    ajFatal("idformat '%S' unknown", idformat);
	ajDebug("idformat '%S' Parser %d\n", idformat, iparser);
	called = 1;
    }

    if(parser[iparser].GcgFormat)
	return 0;

    if(!dbigcgGRegEmblPir)
	dbigcgGRegEmblPir = ajRegCompC("^>..;([^ \t\r\n]+)");

    /* skip to seqid first line */
    while(ajStrGetCharFirst(sline)!='>')
	if(!ajReadline(libs, &sline))
	{
	    ajStrDel(&rline);
	    ajStrDel(&sline);
	    return 0;			/* end of file */
	}

    ajDebug("dbigcg_pirgetent .seq (%S) %d '%S' \n",
	    idformat, ajFileResetPos(libs), sline);

    ajRegExec(dbigcgGRegEmblPir, sline);

    /* skip to refid first line */
    while(ajStrGetCharFirst(rline)!='>')
	if(!ajReadline(libr, &rline))
	{
	    ajErr("ref ended before seq"); /* end of file */
	    ajStrDel(&rline);
	    ajStrDel(&sline);
	    break;
	}

    /* get the encoding/sequence length info */

    ajRegExec(dbigcgGRegEmblPir, rline);
    ajRegSubI(dbigcgGRegEmblPir, 1, &reflibstr);
    ajRegSubI(dbigcgGRegEmblPir, 1, libstr);

    ajDebug("dbigcg_pirgetent seqid '%S' spos: %Ld\n",
	    *libstr, ajFileResetPos(libs));
    ajDebug("dbigcg_pirgetent refid '%S' spos: %Ld\n",
	    *libstr, ajFileResetPos(libr));

    (*parser[iparser].Parser)(libr, alistfile,
                              systemsort, fields, maxFieldLen, countfield,
                              &reflibstr); /* writes alistfile data */

    /* get the description line */
    ajReadline(libs, &sline);
    gcglen = 0;

    /* seek to the end of the sequence; +1 to jump over newline */
    while(ajStrGetCharFirst(sline)!='>')
    {
	spos = ajFileResetPos(libs);
	if(!ajReadline(libs, &sline))
	{
	    spos = 0;
	    break;
	}
	gcglen += ajStrGetLen(sline);
    }

    if(spos)
	ajFileSeek(libs, spos, 0);

    ajDebug("dbigcg_pirgetent end spos %Ld line '%S'\n", spos, sline);

    ajStrDel(&reflibstr);
    ajStrDel(&rline);
    ajStrDel(&sline);
    return gcglen;
}




/* @funcstatic dbigcg_gcgappent ***********************************************
**
** Go to end of a split GCG entry
**
** @param [u] libr [AjPFile] Reference file
** @param [u] libs [AjPFile] Sequence file
** @param [u] rexp [AjPRegexp] Regular expression to find ID in ref file
** @param [u] sexp [AjPRegexp] Regular expression to find ID in seq file
** @param [w] libstr [AjPStr*] ID
** @return [ajint] Sequence length for this section
** @@
******************************************************************************/

static ajint dbigcg_gcgappent(AjPFile libr, AjPFile libs,
			      AjPRegexp rexp, AjPRegexp sexp,
			      AjPStr* libstr)
{
    AjPStr reflibstr = NULL;
    AjPStr seqlibstr = NULL;
    AjPStr testlibstr = NULL;
    ajint ilen;
    AjPStr tmpstr = NULL;
    AjPStr rline  = NULL;
    AjPStr sline  = NULL;

    AjBool isend;
    const char *p;
    char *q;
    ajlong rpos;
    ajlong spos;

    /*
    ** keep reading until the end of entry is reached
    ** and return the extra number of bases
    */

    if(!testlibstr)
	testlibstr = ajStrNew();

    ajStrAssignS(&tmpstr,*libstr);

    ajDebug("dbi_gcgappent '%S'\n", tmpstr);

    p = ajStrGetPtr(tmpstr);
    q = strrchr(p,'_');
    *q = '\0';


    ajFmtPrintS(&testlibstr, "%s_",p);
    ilen = ajStrGetLen(testlibstr);

    isend = ajFalse;

    while(!isend)
    {
        spos = ajFileResetPos(libs);
	ajReadline(libs,&sline);
	while(strncmp(ajStrGetPtr(sline),">>>>",4))
	{
	    spos = ajFileResetPos(libs);
	    if(!ajReadline(libs, &sline))
	    {
		ajDebug("end of file on seq\n");
		ajStrDel(&rline);
		ajStrDel(&sline);
		ajStrDel(&tmpstr);
		ajStrDel(&testlibstr);
		ajStrDel(&seqlibstr);
		ajStrDel(&reflibstr);
		return 1;
	    }
	}

	ajRegExec(sexp, sline);
	ajRegSubI(sexp, 1, &seqlibstr);

	rpos = ajFileResetPos(libr);
	ajReadline(libr, &rline);

	while(ajStrGetCharFirst(rline)!='>')
	{
	    rpos = ajFileResetPos(libr);
	    if(!ajReadline(libr, &rline))
	    {
		ajDebug("ref ended before seq\n");
		ajErr("ref ended before seq\n");
		break;
	    }
	}

	ajRegExec(rexp, rline);
	ajRegSubI(rexp, 1, &reflibstr);

	if(ajStrCmpLenS(reflibstr, testlibstr, ilen) ||
	   ajStrCmpLenS(seqlibstr, testlibstr, ilen))
	    isend = ajTrue;

	ajDebug("gcgappent %B test: '%S' seq: '%S' ref: '%S'\n",
		isend, testlibstr, seqlibstr, reflibstr);
    }

    ajDebug("gcgappent done at seq: '%S' ref: '%S'\n", seqlibstr, reflibstr);

    ajStrAssignC(libstr,p);

    ajFileSeek(libr, rpos, 0);
    ajFileSeek(libs, spos, 0);

    ajStrDel(&reflibstr);
    ajStrDel(&seqlibstr);
    ajStrDel(&testlibstr);
    ajStrDel(&tmpstr);
    ajStrDel(&rline);
    ajStrDel(&sline);

    return 1;
}




/* @funcstatic dbigcg_ParseEmbl ***********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [u] libr [AjPFile] Input file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbigcg_ParseEmbl(AjPFile libr,
			       AjPFile* alistfile,
			       AjBool systemsort, AjPStr const * fields,
			       ajint* maxFieldLen, ajuint* countfield,
			       AjPStr* id)
{
    AjPStr tmpstr  = NULL;
    AjPStr tmpline = NULL;
    AjPStr tmpfd   = NULL;
    AjPStr typStr  = NULL;
    AjPStr tmpacnum = NULL;
    char* fd;
    ajint lineType;
    ajlong rpos;
    AjPStr rline = NULL;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint keyfield = -1;
    static ajint taxfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = keyfield = taxfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "key"))
		keyfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "org"))
		taxfield=numFields;
	    else
		ajWarn("EMBL parsing unknown field '%S' ignored",
		       fields[numFields]);
	    numFields++;
	}
	reset = ajFalse;
    }

    if(!dbigcgGRegEmblTyp)
	dbigcgGRegEmblTyp = ajRegCompC("^([A-Z][A-Z]) +");

    if(!dbigcgGRegEmblWrd)
	dbigcgGRegEmblWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbigcgGRegEmblVer)
	dbigcgGRegEmblVer = ajRegCompC("([A-Za-z0-9]+[.][0-9]+)");

    if(!dbigcgGRegEmblPhr)
	dbigcgGRegEmblPhr = ajRegCompC(" *([^;.\n\r]+)");

    if(!dbigcgGRegEmblTax)
	dbigcgGRegEmblTax = ajRegCompC(" *([^;.\n\r()]+)");

    if(!dbigcgGRegEmblId)
	dbigcgGRegEmblId = ajRegCompC("^ID   ([^ \t;]+)");

    rpos = ajFileResetPos(libr);
    while(ajReadline(libr, &rline) && ajStrGetCharFirst(rline)!='>')
    {
        rpos = ajFileResetPos(libr);
	ajStrAssignS(&tmpstr,rline);

	if(ajRegExec(dbigcgGRegEmblTyp, tmpstr))
	{
	    ajRegSubI(dbigcgGRegEmblTyp, 1, &typStr);
	    if(ajStrMatchC(typStr, "ID"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(typStr, "SV"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(typStr, "AC"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DE"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(typStr, "KW"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(typStr, "OS"))
		lineType = GCGTYPE_TAX;
	    else if(ajStrMatchC(typStr, "OC"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(dbigcgGRegEmblTyp, &tmpline);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(dbigcgGRegEmblId, rline);
	    ajRegSubI(dbigcgGRegEmblId, 1, id);
	    ajDebug("++id '%S'\n", *id);
	    continue;
	}

	if(lineType == GCGTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(dbigcgGRegEmblWrd, tmpline))
	    {
		ajRegSubI(dbigcgGRegEmblWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);

		if(!tmpacnum)
		    ajStrAssignS(&tmpacnum, tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[accfield], fd);
		}
		ajRegPost(dbigcgGRegEmblWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(dbigcgGRegEmblWrd, tmpline))
	    {
		ajRegSubI(dbigcgGRegEmblWrd, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++des '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[desfield], fd);
		}
		ajRegPost(dbigcgGRegEmblWrd, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_VER && svnfield >= 0)
	{
	    while(ajRegExec(dbigcgGRegEmblVer, tmpline))
	    {
		ajRegSubI(dbigcgGRegEmblVer, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++sv '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[svnfield], fd);
		}
		ajRegPost(dbigcgGRegEmblVer, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(dbigcgGRegEmblPhr, tmpline))
	    {
		ajRegSubI(dbigcgGRegEmblPhr, 1, &tmpfd);
		ajRegPost(dbigcgGRegEmblPhr, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		ajDebug("++key '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(dbigcgGRegEmblTax, tmpline))
	    {
		ajRegSubI(dbigcgGRegEmblTax, 1, &tmpfd);
		ajRegPost(dbigcgGRegEmblTax, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrFmtUpper(&tmpfd);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajDebug("++tax '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[taxfield], fd);
		}
	    }
	    continue;
	}
    }

    if(rpos)
        ajFileSeek(libr, rpos, 0);

    ajStrDel(&tmpacnum);
    ajStrDel(&rline);
    ajStrDel(&tmpstr);
    ajStrDel(&tmpline);
    ajStrDel(&tmpfd);
    ajStrDel(&typStr);

    return ajFalse;
}




/* @funcstatic dbigcg_ParseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [u] libr [AjPFile] Input file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbigcg_ParseGenbank(AjPFile libr,
				  AjPFile* alistfile,
				  AjBool systemsort, AjPStr const * fields,
				  ajint* maxFieldLen, ajuint* countfield,
				  AjPStr* id)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp morexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp verexp = NULL;
    ajlong rpos = 0L;
    static AjPStr tmpstr  = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr rline   = NULL;
    static AjPStr tmpfd   = NULL;
    static AjPStr typStr  = NULL;
    ajint lineType=GCGTYPE_OTHER;
    char* fd;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint keyfield = -1;
    static ajint taxfield = -1;
    static ajint svnfield = -1;
    static AjBool reset = AJTRUE;

    if(!fields)
    {
	reset = ajTrue;
	accfield = svnfield = desfield = keyfield = taxfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
		svnfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "key"))
		keyfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "org"))
		taxfield=numFields;
	    else
		ajWarn("EMBL parsing unknown field '%S' ignored",
		       fields[numFields]);

	    numFields++;
	}
	reset = ajFalse;
    }

    if(!typexp)
	typexp = ajRegCompC("^(  )?([A-Z]+)");

    if(!morexp)
	morexp = ajRegCompC("^            ");

    if(!wrdexp)
	wrdexp = ajRegCompC("([A-Za-z0-9_]+)");

    if(!phrexp)
	phrexp = ajRegCompC(" *([^;.\n\r]+)");

    if(!taxexp)
	taxexp = ajRegCompC(" *([^;.\n\r()]+)");

    if(!verexp)
	verexp = ajRegCompC("([A-Za-z0-9]+)( +GI:([0-9]+))?");

    while(ajReadline(libr, &rline) && ajStrGetCharFirst(rline)!='>')
    {
        rpos = ajFileResetPos(libr);
	ajStrAssignS(&tmpstr,rline);

	if(ajRegExec(typexp, tmpstr))
	{
	    ajRegSubI(typexp, 2, &typStr);
	    if(ajStrMatchC(typStr, "LOCUS"))
		lineType = GCGTYPE_ID;
	    else if(ajStrMatchC(typStr, "VERSION"))
		lineType = GCGTYPE_VER;
	    else if(ajStrMatchC(typStr, "ACCESSION"))
		lineType = GCGTYPE_ACC;
	    else if(ajStrMatchC(typStr, "DEFINITION"))
		lineType = GCGTYPE_DES;
	    else if(ajStrMatchC(typStr, "KEYWORDS"))
		lineType = GCGTYPE_KEY;
	    else if(ajStrMatchC(typStr, "ORGANISM"))
		lineType = GCGTYPE_TAX;
	    else
		lineType=GCGTYPE_OTHER;

	    if(lineType != GCGTYPE_OTHER)
		ajRegPost(typexp, &tmpline);
	    ajDebug("++type line %d\n", lineType);
	}
	else if(lineType != GCGTYPE_OTHER && ajRegExec(morexp, rline))
	{
	    ajRegPost(morexp, &tmpline);
	    ajDebug("++more line %d\n", lineType);
	}
	else
	    lineType = GCGTYPE_OTHER;

	if(lineType == GCGTYPE_ID)
	{
	    ajRegExec(wrdexp, tmpline);
	    ajRegSubI(wrdexp, 1, id);
	}
	else if(lineType == GCGTYPE_ACC && accfield >= 0)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI(wrdexp, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[accfield], fd);
		}
		ajRegPost(wrdexp, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_DES && desfield >= 0)
	{
	    while(ajRegExec(wrdexp, tmpline))
	    {
	        ajRegSubI(wrdexp, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++des '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);

		countfield[desfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[desfield],
				"%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[desfield], fd);
		}
		ajRegPost(wrdexp, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_KEY && keyfield >= 0)
	{
	    while(ajRegExec(phrexp, tmpline))
	    {
	        ajRegSubI(phrexp, 1, &tmpfd);
		ajRegPost(phrexp, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		ajDebug("++key '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);

		countfield[keyfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[keyfield],
				"%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_TAX && taxfield >= 0)
	{
	    while(ajRegExec(taxexp, tmpline))
	    {
	        ajRegSubI(taxexp, 1, &tmpfd);
		ajRegPost(taxexp, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
		ajStrTrimWhiteEnd(&tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		ajDebug("++tax '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);

		countfield[taxfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[taxfield],
				"%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[taxfield], fd);
		}
	    }
	    continue;
	}
	else if(lineType == GCGTYPE_VER && svnfield >= 0)
	{
	    if(ajRegExec(verexp, tmpline))
	    {
		ajRegSubI(verexp, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++ver '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[svnfield], fd);
		}
		ajRegSubI(verexp, 3, &tmpfd);
		if(!ajStrGetLen(tmpfd))
		    continue;
		ajStrFmtUpper(&tmpfd);
		ajDebug("++ver gi: '%S'\n", tmpfd);

		countfield[svnfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[svnfield], fd);
		}
	    }
	    continue;
	}

    }

    if(rpos)
	ajFileSeek(libr, rpos, 0);

    return ajFalse;
}




/* @funcstatic dbigcg_ParsePir ************************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [u] libr [AjPFile] Input file
** @param [u] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr const *] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum field token length
** @param [w] countfield [ajuint*] Number of tokens for each field
** @param [w] id [AjPStr*] ID
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbigcg_ParsePir(AjPFile libr,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr const * fields,
			      ajint* maxFieldLen, ajuint* countfield,
			      AjPStr* id)
{
    ajlong rpos;
    AjPStr tmpstr  = NULL;
    AjPStr tmpline = NULL;
    AjPStr rline   = NULL;
    AjPStr tmpfd   = NULL;
    char* fd;
    static ajint numFields;
    static ajint accfield = -1;
    static ajint desfield = -1;
    static ajint keyfield = -1;
    static ajint taxfield = -1;
    static AjBool reset = AJTRUE;

    if(!fields)
    {
	reset = ajTrue;
	accfield = desfield = keyfield = taxfield = -1;
	return ajFalse;
    }

    if(reset)
    {
	numFields = 0;
	while(fields[numFields])
	{
	    countfield[numFields]=0;
	    if(ajStrMatchCaseC(fields[numFields], "acc"))
		accfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "sv"))
	    {
		++numFields;
		continue;
	    }
	    else if(ajStrMatchCaseC(fields[numFields], "des"))
		desfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "key"))
		keyfield=numFields;
	    else if(ajStrMatchCaseC(fields[numFields], "org"))
		taxfield=numFields;
	    else
		ajWarn("EMBL parsing unknown field '%S' ignored",
		       fields[numFields]);

	    numFields++;
	}
	reset = ajFalse;
    }

    if(!dbigcgGRegPirWrd)
	dbigcgGRegPirWrd = ajRegCompC("([A-Za-z0-9_]+)");

    if(!dbigcgGRegPirId)
	dbigcgGRegPirId = ajRegCompC("^>..;([^;.\n\r]+)");

    if(!dbigcgGRegPirPhr)				/* allow . for "sp." */
	dbigcgGRegPirPhr = ajRegCompC(" *([^,;\n\r]+)");

    if(!dbigcgGRegPirTax2)				/* allow . for "sp." */
	dbigcgGRegPirTax2 = ajRegCompC(" *([^,;\n\r()]+)");

    if(!dbigcgGRegPirAc1)
	dbigcgGRegPirAc1 = ajRegCompC("^C;Accession:");

    if(!dbigcgGRegPirAc2)
	dbigcgGRegPirAc2 = ajRegCompC("([A-Za-z0-9]+)");

    if(!dbigcgGRegPirTax1)
	dbigcgGRegPirTax1 = ajRegCompC("^C;Species:");

    if(!dbigcgGRegPirKey)
	dbigcgGRegPirKey = ajRegCompC("^C;Keywords:");

    rpos = ajFileResetPos(libr);

    ajDebug("++id '%S'\n", *id);


    ajReadline(libr, &rline);
    ajDebug("line-2 '%S'\n", rline);
    if(desfield >= 0)
    {
	while(ajRegExec(dbigcgGRegPirWrd, rline))
	{
	    ajRegSubI(dbigcgGRegPirWrd, 1, &tmpfd);
	    ajStrFmtUpper(&tmpfd);
	    ajDebug("++des '%S'\n", tmpfd);
	    embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);

	    countfield[desfield]++;
	    if(systemsort)
		ajFmtPrintF(alistfile[desfield], "%S %S\n", *id, tmpfd);
	    else
	    {
		fd = ajCharNewS(tmpfd);
		ajListPushAppend(dbigcgGFdl[desfield], fd);
	    }
	    ajRegPost(dbigcgGRegPirWrd, &tmpstr);
            ajStrAssignS(&rline, tmpstr);
	}
    }

    while(ajStrGetCharFirst(rline)!='>')
    {
        rpos = ajFileResetPos(libr);
	ajStrAssignS(&tmpstr,rline);

	if(ajRegExec(dbigcgGRegPirAc1, rline))
	{
	    ajRegPost(dbigcgGRegPirAc1, &tmpline);
	    while(ajRegExec(dbigcgGRegPirAc2, tmpline))
	    {
		ajRegSubI(dbigcgGRegPirAc2, 1, &tmpfd);
		ajStrFmtUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);

		countfield[accfield]++;
		if(systemsort)
		    ajFmtPrintF(alistfile[accfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNewS(tmpfd);
		    ajListPushAppend(dbigcgGFdl[accfield], fd);
		}
		ajRegPost(dbigcgGRegPirAc2, &tmpstr);
                ajStrAssignS(&tmpline, tmpstr);
	    }
	}

	if(keyfield >= 0)
	{
	    if(ajRegExec(dbigcgGRegPirKey, rline))
	    {
		ajRegPost(dbigcgGRegPirKey, &tmpline);
		while(ajRegExec(dbigcgGRegPirPhr, tmpline))
		{
		    ajRegSubI(dbigcgGRegPirPhr, 1, &tmpfd);
		    ajStrFmtUpper(&tmpfd);
		    ajStrTrimWhiteEnd(&tmpfd);
		    ajDebug("++key '%S'\n", tmpfd);
		    embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);

		    countfield[keyfield]++;
		    if(systemsort)
			ajFmtPrintF(alistfile[keyfield], "%S %S\n", *id,
				    tmpfd);
		    else
		    {
			fd = ajCharNewS(tmpfd);
			ajListPushAppend(dbigcgGFdl[keyfield], fd);
		    }
		    ajRegPost(dbigcgGRegPirPhr, &tmpstr);
                    ajStrAssignS(&tmpline, tmpstr);
		}
	    }
	}

	if(taxfield >= 0)
	{
	    if(ajRegExec(dbigcgGRegPirTax1, rline))
	    {
		ajRegPost(dbigcgGRegPirTax1, &tmpline);
		while(ajRegExec(dbigcgGRegPirTax2, tmpline))
		{
		    ajRegSubI(dbigcgGRegPirTax2, 1, &tmpfd);
		    ajStrFmtUpper(&tmpfd);
		    ajDebug("++tax '%S'\n", tmpfd);
		    embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);

		    countfield[taxfield]++;
		    if(systemsort)
			ajFmtPrintF(alistfile[taxfield], "%S %S\n", *id,
				    tmpfd);
		    else
		    {
			fd = ajCharNewS(tmpfd);
			ajListPushAppend(dbigcgGFdl[taxfield], fd);
		    }
		    ajRegPost(dbigcgGRegPirTax2, &tmpstr);
                    ajStrAssignS(&tmpline, tmpstr);
		}
	    }
	}

	if(!ajReadline(libr, &rline))
	{
	    rpos = 0;
	    break;
	}
    }

    if(rpos)
	ajFileSeek(libr, rpos, 0);

    ajStrDel(&tmpstr);
    ajStrDel(&tmpline);
    ajStrDel(&rline);
    ajStrDel(&tmpfd);

    return ajFalse;
}

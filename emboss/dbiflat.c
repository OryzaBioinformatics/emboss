/* @source dbiflat application
**
** Index flatfile databases
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
** This version reads a flat file database,
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

#define FLATTYPE_OTHER 0
#define FLATTYPE_ID 1
#define FLATTYPE_ACC 2
#define FLATTYPE_DES 3
#define FLATTYPE_KEY 4
#define FLATTYPE_TAX 5
#define FLATTYPE_VER 6

static AjBool dbiflat_ParseEmbl    (AjPFile libr, AjPFile* alistfile,
				    AjBool systemsort, AjPStr* fields,
				    ajint* maxFieldLen,
				    ajint *dpos, AjPStr* id, AjPList* acl);
static AjBool dbiflat_ParseGenbank (AjPFile libr, AjPFile* alistfile,
				    AjBool systemsort, AjPStr* fields,
				    ajint* maxFieldLen,
				    ajint *dpos, AjPStr* id, AjPList* acl);

typedef struct SParser
{
  char* Name;
  AjBool (*Parser) (AjPFile libr, AjPFile* alistfile,
		    AjBool systemsort, AjPStr* fields, ajint* maxFieldLen,
		    ajint *dpos, AjPStr* id, AjPList* acl);
} OParser;

static OParser parser[] = {
  {"EMBL", dbiflat_ParseEmbl},
  {"SWISS", dbiflat_ParseEmbl},
  {"GB", dbiflat_ParseGenbank},
  {NULL, NULL}
};

static EmbPEntry dbiflat_NextFlatEntry (AjPFile libr, ajint ifile,
					AjPStr idformat, AjBool systemsort,
					AjPStr* fields, ajint* maxFieldLen,
					ajint* maxidlen,
					AjPFile elistfile, AjPFile* alistfile);

/* @prog dbiflat **************************************************************
**
** Index a flat file database
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPList idlist;
    AjPList* fieldList=NULL;

    AjBool systemsort;
    AjBool cleanup;

    ajint maxindex;
    ajint maxidlen = 0;
    ajint maxlen;

    AjPFile elistfile=NULL;
    AjPFile* alistfile=NULL;

    AjPStr dbname = NULL;
    AjPStr release = NULL;
    AjPStr datestr = NULL;
    AjPStr sortopt = NULL;
    void **entryIds = NULL;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr curfilename = NULL;

    AjPFile libr=NULL;
    AjPStr idformat = NULL;

    EmbPEntry entry;

    ajint idCount=0;
    AjPList listInputFiles = NULL;
    void ** inputFiles = NULL;
    ajint nfiles;
    ajint ifile;

    ajint filesize;
    short recsize;
    ajint maxfilelen=20;
    char date[4] = {0,0,0,0};

    AjPStr tmpfname = NULL;
    AjPStr* fields = NULL;

    AjPFile entFile = NULL;

    AjPStr* divfiles = NULL;
    ajint* maxFieldLen = NULL;

    ajint ifield=0;
    ajint nfields=0;


    embInit ("dbiflat", argc, argv);

    idformat = ajAcdGetListI ("idformat",1);
    fields = ajAcdGetList ("fields");
    directory = ajAcdGetString ("directory");
    indexdir = ajAcdGetString ("indexdirectory");
    filename = ajAcdGetString ("filenames");
    exclude = ajAcdGetString ("exclude");
    dbname = ajAcdGetString ("dbname");
    release = ajAcdGetString ("release");
    datestr = ajAcdGetString ("date");
    systemsort = ajAcdGetBool ("systemsort");
    cleanup = ajAcdGetBool ("cleanup");
    sortopt = ajAcdGetString ("sortoptions");
    maxindex = ajAcdGetInt ("maxindex");

    while(fields[nfields])	/* array ends with a NULL */
      nfields++;

    if (nfields) {

      AJCNEW(maxFieldLen, nfields);
      for (ifield=0; ifield < nfields; ifield++)
	  maxFieldLen[ifield] = -maxindex;

      if (systemsort)
	  AJCNEW(alistfile, nfields);
      else
      {
	  AJCNEW(fieldList, nfields);
	  for (ifield=0; ifield < nfields; ifield++)
	      fieldList[ifield] = ajListNew ();
      }
    }

    ajStrCleanWhite(&dbname);	/* used for temp filenames */
    embDbiDateSet (datestr, date);
    idlist = ajListNew ();

    ajDebug ("reading '%S/%S'\n", directory, filename);
    ajDebug ("writing '%S/'\n", indexdir);

    listInputFiles = embDbiFileListExc (directory, filename, exclude);
    ajListSort (listInputFiles, ajStrCmp);
    nfiles = ajListToArray(listInputFiles, &inputFiles);
    if (!nfiles)
	ajFatal ("No files selected");

    AJCNEW0(divfiles, nfiles);

    /*
    ** process each input file, one at a time
    */

    for (ifile=0; ifile < nfiles; ifile++)
    {
	curfilename = (AjPStr) inputFiles[ifile];
	embDbiFlatOpenlib (curfilename, &libr);
	if (ajStrLen(curfilename) >= maxfilelen)
	    maxfilelen = ajStrLen(curfilename) + 1;

	ajDebug ("processing filename '%S' ...\n", curfilename);
	ajDebug ("processing file '%F' ...\n", libr);
	ajStrAssS (&divfiles[ifile], curfilename);
	ajFileNameTrim(&divfiles[ifile]);

	if (systemsort)		/* elistfile for entries, alist for fields */
	  elistfile = embDbiSortOpen (alistfile, ifile,
				      dbname, fields, nfields);

	while ((entry=dbiflat_NextFlatEntry(libr, ifile, idformat,
					    systemsort, fields, maxFieldLen,
					    &maxidlen,
					    elistfile, alistfile)))
	{
	    idCount++;
	    if (!systemsort)	/* save the entry data in lists */
	      embDbiMemEntry (idlist, fieldList, nfields, entry, ifile);
	}
	if (systemsort)
	  embDbiSortClose (&elistfile, alistfile, nfields);
    }

    /*
    ** write the division.lkp file
    */

    embDbiWriteDivision (indexdir, dbname, release, date,
			 maxfilelen, nfiles, divfiles, NULL);

    /*
    ** Write the entryname.idx index
    */

    ajStrAssC (&tmpfname, "entrynam.idx");
    entFile = ajFileNewOutD(indexdir, tmpfname);

    recsize = maxidlen+10;
    filesize = 300 + (idCount*(ajint)recsize);
    embDbiHeader (entFile, filesize, idCount, recsize, dbname, release, date);

    if (systemsort)
        idCount = embDbiSortWriteEntry (entFile, maxidlen,
					dbname, nfiles, cleanup, sortopt);
    else			/* save entries in entryIds array */
        embDbiMemWriteEntry (entFile, maxidlen,
			     idlist, &entryIds);

    ajFileClose (&entFile);

    /*
    ** Write the fields index files
    */

    for (ifield=0; ifield < nfields; ifield++)
    {
        if (maxindex)
	  maxlen = maxindex;
	else
	  maxlen = maxFieldLen[ifield];

        if (systemsort)
	  embDbiSortWriteFields (dbname, release, date, indexdir,
				 fields[ifield], maxlen,
				  nfiles, idCount, cleanup, sortopt);
	else
	  embDbiMemWriteFields (dbname, release, date, indexdir,
				fields[ifield], maxlen,
				fieldList[ifield], entryIds);
    }

    if (systemsort)
      embDbiRmEntryFile (dbname, cleanup);

    ajListDel(&listInputFiles);    

    ajExit ();
    return 0;
}

/* @funcstatic dbiflat_NextFlatEntry *****************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [r] libr [AjPFile] Database file
** @param [r] ifile [ajint] File number.
** @param [r] idformat [AjPStr] Format to be used
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr*] Fields to be indexed
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] maxidlen [ajint*] Maximum entry ID length
** @param [r] elistfile [AjPFile] entry file
** @param [r] alistfile [AjPFile*] field data files array
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbiflat_NextFlatEntry (AjPFile libr, ajint ifile,
					AjPStr idformat, AjBool systemsort,
					AjPStr* fields, ajint* maxFieldLen,
					ajint* maxidlen,
					AjPFile elistfile, AjPFile* alistfile)
{
    static EmbPEntry ret=NULL;
    ajint ir;
    ajint is = 0;
    static AjPStr id = NULL;
    char* token;
    ajint i;
    static AjPList* fdl = NULL;
    static ajint called = 0;
    static ajint iparser = -1;
    static ajint nfields;
    ajint ifield;

    if (!called)
    {
	for (i=0; parser[i].Name; i++)
	    if (ajStrMatchC (idformat, parser[i].Name))
	    {
		iparser = i;
		break;
	    }

	if (iparser < 0)
	    ajFatal ("idformat '%S' unknown", idformat);
    }

    if (!fdl)
    {
      nfields=0;
      while (fields[nfields])
	nfields++;
      if (nfields)
	AJCNEW(fdl, nfields);
      for (i=0; i < nfields; i++)
      {
	fdl[i] = ajListNew();
      }
    }
    if (!ret || !systemsort)
	ret = embDbiEntryNew(nfields);

    if (!parser[iparser].Parser (libr, alistfile, systemsort, fields,
				 maxFieldLen, &ir, &id, fdl))
	return NULL;

    /* id to ret->entry */

    if (ajStrLen(id) > *maxidlen)
      *maxidlen = ajStrLen(id);

    if (systemsort)
	ajFmtPrintF (elistfile, "%S %d %d %d\n", id, ir, is, ifile+1);
    else
    {
	ret->entry = ajCharNew(id);
	ret->rpos = ir;
	ret->spos = is;
	ret->filenum = ifile+1;

	/* field tokens as list, then move to ret->field */
	for (ifield=0; ifield < nfields; ifield++)
	{
	    ret->nfield[ifield] = ajListLength(fdl[ifield]);

	    if (ret->nfield[ifield])
	    {
	        AJCNEW(ret->field[ifield],ret->nfield[ifield]);

		i=0;
		while (ajListPop(fdl[ifield], (void**) &token))
		    ret->field[ifield][i++] = token;
	    }
	    else
	        ret->field[ifield] = NULL;
	/*
	 *  ajDebug("id '%s' %d %d nfield: %d\n",
	 *          ret->entry, ir, is, ret->nfield[ifield]);
	 *  for (i=0; i<ret->nfield[ifield]; i++)
	 *  ajDebug("   %3d %s\n", i, ret->field[ifield][i]);
	 */

	}
    }

    return ret;
}

/* @funcstatic dbiflat_ParseEmbl ********************************************
**
** Parse the ID, accession from an EMBL entry.
**
** Reads to the end of the entry and then returns.
**
** @param [r] libr [AjPFile] Input database file
** @param [r] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [w] fields [AjPStr*] Fields required
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] dpos [ajint*] Byte offset
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field values
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbiflat_ParseEmbl (AjPFile libr, AjPFile* alistfile,
				 AjBool systemsort, AjPStr* fields,
				 ajint* maxFieldLen,
				 ajint* dpos, AjPStr* id,
				 AjPList* fdl)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp idexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp endexp = NULL;
    static AjPStr rline=NULL;
    static AjPStr tmpline = NULL;
    static AjPStr tmpfd = NULL;
    static AjPStr typStr = NULL;
    AjPStr tmpacnum = NULL;
    char* fd;
    ajint lineType;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint keyfield=-1;
    static ajint taxfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;
    AjBool done = ajFalse;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = keyfield = taxfield = -1;
      return ajFalse;
    }

    if (reset)
    {
      numFields = 0;
      while (fields[numFields])
      {
	if (ajStrMatchCaseC(fields[numFields], "acnum"))
	  accfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "seqvn"))
	  svnfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "des"))
	  desfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "keyword"))
	  keyfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "taxon"))
	  taxfield=numFields;
	else
	  ajWarn ("EMBL parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }

    if (!typexp)
	typexp = ajRegCompC ("^([A-Z][A-Z]) +");

    if (!wrdexp)
	wrdexp = ajRegCompC ("([A-Za-z0-9_]+)");

    if (!phrexp)
	phrexp = ajRegCompC (" *([^;.\n\r]+)");

    if (!taxexp)
	taxexp = ajRegCompC (" *([^;.\n\r()]+)");

    if (!idexp)
	idexp = ajRegCompC ("^ID   ([^ \t]+)");

    if (!endexp)
	endexp = ajRegCompC ("^//");

    *dpos = ajFileTell(libr);

    while (ajFileGets (libr, &rline))
    {
	if (ajRegExec (endexp, rline))
	{
	    done = ajTrue;
	    break;
	}

	if (ajRegExec (typexp, rline))
	{
	    ajRegSubI (typexp, 1, &typStr);
	    if (ajStrMatchC(typStr, "ID")) lineType = FLATTYPE_ID;
	    else if (ajStrMatchC(typStr, "SV")) lineType = FLATTYPE_VER;
	    else if (ajStrMatchC(typStr, "AC")) lineType = FLATTYPE_ACC;
	    else if (ajStrMatchC(typStr, "DE")) lineType = FLATTYPE_DES;
	    else if (ajStrMatchC(typStr, "KW")) lineType = FLATTYPE_KEY;
	    else if (ajStrMatchC(typStr, "OS")) lineType = FLATTYPE_TAX;
	    else if (ajStrMatchC(typStr, "OC")) lineType = FLATTYPE_TAX;
	    else lineType=FLATTYPE_OTHER;
	    if (lineType != FLATTYPE_OTHER)
	      ajRegPost (typexp, &tmpline);
	}
	else
	  lineType = FLATTYPE_OTHER;

	if (lineType == FLATTYPE_ID)
	{
	    ajRegExec (idexp, rline);
	    ajRegSubI (idexp, 1, id);
	    ajDebug("++id '%S'\n", *id);
	    continue;
	}

	if (lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);
		if (!tmpacnum)
		  ajStrAssS(&tmpacnum, tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[accfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[accfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[accfield], fd);
		}
		ajRegPost (wrdexp, &tmpline);
	    }
	    continue;
	}
	else if (lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++des '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[desfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[desfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[desfield], fd);
		}
		ajRegPost (wrdexp, &tmpline);
	    }
	    continue;
	}
	else if (lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++sv '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[svnfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[svnfield], fd);
		}
		ajRegPost (wrdexp, &tmpline);
	    }
	    continue;
	}
	else if (lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while (ajRegExec(phrexp, tmpline))
	    {
		ajRegSubI (phrexp, 1, &tmpfd);
		ajRegPost (phrexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajStrToUpper(&tmpfd);
		ajDebug("++key '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[keyfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[keyfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[keyfield], fd);
		}
	    }
	    continue;
	}
	else if (lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while (ajRegExec(taxexp, tmpline))
	    {
		ajRegSubI (taxexp, 1, &tmpfd);
		ajRegPost (taxexp, &tmpline);
		ajStrToUpper(&tmpfd);
		ajStrChompEnd(&tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajDebug("++tax '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[taxfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[taxfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[taxfield], fd);
		}
	    }
	    continue;
	}
    }

    if (!done)
      return ajFalse;

    if (svnfield >= 0 && tmpacnum)
    {
      ajFmtPrintS(&tmpfd, "%S.0", tmpacnum);
      embDbiMaxlen (&tmpfd, &maxFieldLen[svnfield]);
      if (systemsort)
	ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpfd);
      else
      {
	  fd = ajCharNew(tmpfd);
	  ajListPushApp (fdl[svnfield], fd);
      }
   }

    ajStrDel(&tmpacnum);
    return ajTrue;
}




/* @funcstatic dbiflat_ParseGenbank ******************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [r] libr [AjPFile] Input database file
** @param [r] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [w] fields [AjPStr*] Fields required
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] dpos [ajint*] Byte offset
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field values
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbiflat_ParseGenbank (AjPFile libr, AjPFile* alistfile,
				    AjBool systemsort, AjPStr* fields,
				    ajint* maxFieldLen,
				    ajint* dpos, AjPStr* id,
				    AjPList* fdl)
{

    static AjPRegexp typexp = NULL;
    static AjPRegexp morexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp verexp = NULL;
    static AjPRegexp endexp = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr tmpfd = NULL;
    static AjPStr rline=NULL;
    static AjPStr typStr=NULL;
    ajint lineType=FLATTYPE_OTHER;
    AjPStr tmpacnum = NULL;
    char* fd;
    ajlong ipos = 0;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint keyfield=-1;
    static ajint taxfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;
    AjBool done = ajFalse;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = keyfield = taxfield = -1;
      return ajFalse;
    }

    if (reset)
    {
      numFields = 0;
      while (fields[numFields])
      {
	if (ajStrMatchCaseC(fields[numFields], "acnum"))
	  accfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "seqvn"))
	  svnfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "des"))
	  desfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "keyword"))
	  keyfield=numFields;
	else if (ajStrMatchCaseC(fields[numFields], "taxon"))
	  taxfield=numFields;
	else
	  ajWarn ("GenBank parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }

    if (!typexp)
	typexp = ajRegCompC ("^(  )?([A-Z]+)");

    if (!morexp)
	morexp = ajRegCompC ("^            ");

    if (!wrdexp)
	wrdexp = ajRegCompC ("([A-Za-z0-9_]+)");

    if (!phrexp)
	phrexp = ajRegCompC (" *([^;.\n\r]+)");

    if (!taxexp)
	taxexp = ajRegCompC (" *([^;.\n\r()]+)");

    if (!verexp)
	verexp = ajRegCompC ("([A-Za-z0-9]+)( +GI:([0-9]+))?");
    if (!endexp)
	endexp = ajRegCompC ("^//");

    ipos = ajFileTell(libr);
  
    while (ajFileGets (libr, &rline))
    {
	if (ajRegExec (endexp, rline))
	{
	    done = ajTrue;
	    break;
	}

	if (ajRegExec (typexp, rline))
	{
	    ajRegSubI (typexp, 2, &typStr);
	    if (ajStrMatchC(typStr, "LOCUS")) lineType = FLATTYPE_ID;
	    else if (ajStrMatchC(typStr, "VERSION")) lineType = FLATTYPE_VER;
	    else if (ajStrMatchC(typStr, "ACCESSION")) lineType = FLATTYPE_ACC;
	    else if (ajStrMatchC(typStr, "DEFINITION")) lineType = FLATTYPE_DES;
	    else if (ajStrMatchC(typStr, "KEYWORDS")) lineType = FLATTYPE_KEY;
	    else if (ajStrMatchC(typStr, "ORGANISM")) lineType = FLATTYPE_TAX;
	    else lineType=FLATTYPE_OTHER;
	    if (lineType != FLATTYPE_OTHER)
	      ajRegPost (typexp, &tmpline);
	    ajDebug("++type line %d\n", lineType);
	}
	else if (lineType != FLATTYPE_OTHER && ajRegExec (morexp, rline))
	{
	  ajRegPost (morexp, &tmpline);
	  ajDebug("++more line %d\n", lineType);
	}
	else
	  lineType = FLATTYPE_OTHER;

	if (lineType == FLATTYPE_ID)
	{
	  ajRegExec (wrdexp, tmpline);
	  ajRegSubI (wrdexp, 1, id);
	  *dpos = ipos;
	}

	else if (lineType == FLATTYPE_ACC && accfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[accfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[accfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[accfield], fd);
		}
		ajRegPost (wrdexp, &tmpline);
	    }
	    continue;
	}

	else if (lineType == FLATTYPE_DES && desfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
	        ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++des '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[desfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[desfield],
				 "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[desfield], fd);
		}
		ajRegPost (wrdexp, &tmpline);
	    }
	    continue;
	}

	else if (lineType == FLATTYPE_KEY && keyfield >= 0)
	{
	    while (ajRegExec(phrexp, tmpline))
	    {
	        ajRegSubI (phrexp, 1, &tmpfd);
		ajRegPost (phrexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajStrToUpper(&tmpfd);
		ajDebug("++key '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[keyfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[keyfield],
				 "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[keyfield], fd);
		}
	    }
	    continue;
	}

	else if (lineType == FLATTYPE_TAX && taxfield >= 0)
	{
	    while (ajRegExec(taxexp, tmpline))
	    {
	        ajRegSubI (taxexp, 1, &tmpfd);
		ajRegPost (taxexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajStrToUpper(&tmpfd);
		ajDebug("++tax '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[taxfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[taxfield],
				 "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[taxfield], fd);
		}
	    }
	    continue;
	}

	else if (lineType == FLATTYPE_VER && svnfield >= 0)
	{
	    if (ajRegExec(verexp, tmpline))
	    {
		ajRegSubI (verexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++ver '%S'\n", tmpfd);
		embDbiMaxlen (&tmpfd, &maxFieldLen[svnfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[svnfield], fd);
		}
		ajRegSubI (verexp, 3, &tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajStrToUpper(&tmpfd);
		ajDebug("++ver gi: '%S'\n", tmpfd);
		if (systemsort)
		    ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[svnfield], fd);
		}
	    }
	    continue;
	}

	ipos = ajFileTell(libr);
    }

    if (!done)
      return ajFalse;

    if (svnfield >= 0 && tmpacnum)
    {
      ajFmtPrintS(&tmpfd, "%S.0", tmpacnum);
      embDbiMaxlen (&tmpfd, &maxFieldLen[svnfield]);
      if (systemsort)
	ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpfd);
      else
      {
	  fd = ajCharNew(tmpfd);
	  ajListPushApp (fdl[svnfield], fd);
      }
   }

    ajStrDel(&tmpacnum);
    return ajTrue;
}

/* @source dbigcg application
**
** Index gcg/pir/accelrys databases
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
#include <dirent.h>
#include <sys/wait.h>
#include <string.h>

#define GCGTYPE_OTHER 0
#define GCGTYPE_ID 1
#define GCGTYPE_ACC 2
#define GCGTYPE_DES 3
#define GCGTYPE_KEY 4
#define GCGTYPE_TAX 5
#define GCGTYPE_VER 6

static EmbPEntry dbigcg_nextentry (AjPFile libr, AjPFile libs,
				   ajint ifile, AjPStr idformat,
				   AjBool systemsort,
				   AjPStr* fields, ajint* maxFieldLen,
				   ajint* maxidlen,
				   AjPFile elistfile, AjPFile* alistfile);
static AjBool dbigcg_gcgopenlib(AjPStr lname, AjPFile* libr, AjPFile* lib);
static ajint dbigcg_gcggetent(AjPStr idformat,
			      AjPFile libr, AjPFile libs,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr* fields,
			      ajint* maxFieldLen,
			      AjPStr* libstr, AjPList* fdl);
static ajint dbigcg_pirgetent(AjPStr idformat,
			      AjPFile libr, AjPFile libs, AjPFile* alistfile,
			      AjBool systemsort, AjPStr* fields,
			      ajint* maxFieldLen,
			      AjPStr* libstr, AjPList* fdl);
static ajint dbigcg_gcgappent(AjPFile libr, AjPFile libs,
			      AjPRegexp rexp, AjPRegexp sexp,
			      AjPStr* libstr);

static AjBool dbigcg_ParseEmbl (AjPFile libr,
				AjPFile* alistfile,
				AjBool systemsort, AjPStr* fields,
				ajint* maxFieldLen,
				AjPStr *id, AjPList* fdl);
static AjBool dbigcg_ParsePir (AjPFile libr,
			       AjPFile* alistfile,
			       AjBool systemsort, AjPStr* fields,
			       ajint* maxFieldLen,
			       AjPStr *id, AjPList* fdl);
static AjBool dbigcg_ParseGenbank (AjPFile libr,
				   AjPFile* alistfile,
				   AjBool systemsort, AjPStr* fields,
				   ajint* maxFieldLen,
				   AjPStr *id, AjPList* fdl);

typedef struct SParser
{
  char* Name;
  AjBool (*Parser) (AjPFile libr,
		    AjPFile* alistfile,
		    AjBool systemsort, AjPStr* fields,
		    ajint* maxFieldLen,
		    AjPStr *id, AjPList* fdl);
} OParser;

static OParser parser[] =
{
  { "EMBL", dbigcg_ParseEmbl },
  { "SWISS", dbigcg_ParseEmbl },
  { "GENBANK", dbigcg_ParseGenbank },
  { "PIR", dbigcg_ParsePir },
  { NULL, NULL }
};



/* @prog dbigcg ***************************************************************
**
** Index a GCG formatted database
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
    AjPFile libs=NULL;
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

    AjPStr* reffiles = NULL;
    AjPStr* seqfiles = NULL;
    ajint* maxFieldLen = NULL;

    ajint ifield=0;
    ajint nfields=0;

    embInit ("dbigcg", argc, argv);

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

    AJCNEW0(reffiles, nfiles);
    AJCNEW0(seqfiles, nfiles);

    /*
    ** process each input file, one at a time
    */

    for (ifile=0; ifile < nfiles; ifile++)
    {
	curfilename = (AjPStr) inputFiles[ifile];
	dbigcg_gcgopenlib (curfilename, &libr, &libs);

	ajFmtPrintS(&reffiles[ifile], "%F", libr);
	ajFileNameTrim(&reffiles[ifile]);
	ajFmtPrintS(&seqfiles[ifile], "%F", libs);
	ajFileNameTrim(&seqfiles[ifile]);
	ajDebug ("processing filename '%S' ...\n", curfilename);
	ajDebug ("processing reffile '%S' ...\n", reffiles[ifile]);
	ajDebug ("processing seqfile '%S' ...\n", seqfiles[ifile]);
	if ((ajStrLen(reffiles[ifile])+
	     ajStrLen(seqfiles[ifile])) >= maxfilelen)
	  maxfilelen = ajStrLen(reffiles[ifile])+ajStrLen(seqfiles[ifile])+2;

	if (systemsort)		/* elistfile for entries, alist for fields */
	  elistfile = embDbiSortOpen (alistfile, ifile,
				      dbname, fields, nfields);

	while ((entry=dbigcg_nextentry(libr, libs,
				       ifile, idformat,
				       systemsort, fields, maxFieldLen,
				       &maxidlen, elistfile, alistfile)))
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
			 maxfilelen, nfiles, reffiles, seqfiles);

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

/* @funcstatic dbigcg_nextentry ********************************************
**
** Returns next database entry as an EmbPEntry object
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [r] ifile [ajint] File number.
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbigcg_nextentry (AjPFile libr, AjPFile libs,
				   ajint ifile, AjPStr idformat,
				   AjBool systemsort,
				   AjPStr* fields, ajint* maxFieldLen,
				   ajint* maxidlen,
				   AjPFile elistfile, AjPFile* alistfile)
{
    static EmbPEntry ret=NULL;
    ajint ir;
    ajint is = 0;
    static AjPStr id = NULL;
    static AjPStr tmpline2 = NULL;
    char* token;
    char *p;
    ajint i;
    static AjPList* fdl = NULL;
    static ajint nfields;
    ajint ifield;

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

    ir = ajFileTell(libr);
    is = ajFileTell(libs);

    if (!dbigcg_gcggetent (idformat, libr, libs,
			   alistfile, systemsort, fields, maxFieldLen,
			   &id, fdl) &&
	!dbigcg_pirgetent (idformat, libr, libs,
			   alistfile, systemsort, fields, maxFieldLen,
			   &id, fdl))
	return NULL;

    ajDebug("id '%S' ir:%d is:%d nfields: %d\n",
	    id, ir, is, nfields);

    ajStrAssC(&tmpline2,ajStrStr(id));
    if(ajStrSuffixC(id,"_0") ||
       ajStrSuffixC(id,"_00") ||
       ajStrSuffixC(id,"_000"))
    {
	p = strrchr(ajStrStr(tmpline2),'_');
	*p = '\0';
    }

    if (ajStrLen(id) > *maxidlen)
      *maxidlen = ajStrLen(id);

    if(systemsort)
    {
	ajFmtPrintF (elistfile, "%s %d %d %d\n", ajStrStr(tmpline2),
		     ir, is, ifile+1);
    }
    else
    {
	ret->entry = ajCharNew(tmpline2);
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
	}
   }

    return ret;
}


/* @funcstatic dbigcg_gcgopenlib ********************************************
**
** Open a GCG library
**
** @param [r] lname [AjPStr] Source file basename
** @param [r] libr [AjPFile*] Reference file
** @param [r] libs [AjPFile*] Sequence file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbigcg_gcgopenlib (AjPStr lname, AjPFile* libr, AjPFile* libs)
{

    static AjPStr rname = NULL;
    static AjPStr sname = NULL;

    ajStrAssS (&rname, lname);
    ajStrAssS (&sname, lname);

    ajFileNameExtC(&rname,"ref");
    ajFileNameExtC(&sname,"seq");

    ajFileClose(libr);
    ajFileClose(libs);

    *libr = ajFileNewIn(rname);
    if(!*libr)
	ajFatal("Failed to open %S for reading",rname);
  
    *libs = ajFileNewIn(sname);
    if(!*libs)
	ajFatal("Failed to open %S for reading",sname);

    return ajTrue;
}


/* @funcstatic dbigcg_gcggetent **********************************************
**
** get a single entry from the GCG database files
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [w] libstr [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field tokens
** @return [ajint] Sequence length
** @@
******************************************************************************/

static ajint dbigcg_gcggetent(AjPStr idformat,
			      AjPFile libr, AjPFile libs,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr* fields,
			      ajint* maxFieldLen,
			      AjPStr* libstr, AjPList* fdl)
{
    static AjPStr gcgtype = NULL;
    static ajint gcglen;
    static AjPStr gcgdate = NULL;
    ajint rblock;
    /*  ajint ddone = 0;*/
    static AjPStr reflibstr = NULL;
    /*  ajint iac;*/
    ajint i;
    static AjPStr tmpstr = NULL;
    static ajint called = 0;
    static ajint iparser = -1;
    static AjPRegexp rexp = NULL;
    static AjPRegexp sexp = NULL;
    static AjPStr rline=NULL;
    static AjPStr sline=NULL;

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
	ajDebug ("idformat '%S' Parser %d\n", idformat, iparser);
	called = 1;
    }

    if (!rexp)
	rexp = ajRegCompC ("^....([^ \t\n]+)");
    if (!sexp)
	sexp = ajRegCompC("^....([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)"
			  "[ \t]+([^ \t]+)[ \t]+([0-9]+)");

    ajStrAssC(&sline, "");

    /* check for seqid first line */
    while (ajStrChar(sline,0)!='>')
    {
	if (!ajFileGets(libs, &sline))
	{
	    return 0;		/* end of file */
	}
	ajDebug("... read until next seq %d '%S'\n",
		ajFileTell(libs), sline);
    }

    ajDebug("dbigcg_gcggetent .seq (%S) %d '%S'\n",
	    idformat, ajFileTell(libs), sline);
    /* get the encoding/sequence length info */

    if (!ajRegExec(sexp, sline))
	return 0;

    ajRegSubI(sexp, 1, libstr);	/* Entry ID returned */

    ajRegSubI(sexp, 2, &gcgdate);
    ajRegSubI(sexp, 3, &gcgtype);
    ajRegSubI(sexp, 5, &tmpstr);
    ajStrToInt (tmpstr, &gcglen);

    ajDebug("new entry '%S' date:'%S' type:'%S' len:'%S'=%d\n",
	    *libstr, gcgdate, gcgtype, tmpstr, gcglen);

    ajStrAssC (&rline, "");

    ajDebug("dbigcg_gcggetent .ref (%S) %d '%S'\n",
	    idformat, ajFileTell(libr), rline);

    /* check for refid first line */
    while (ajStrChar(rline,0)!='>')
    {
	if (!ajFileGets(libr, &rline))
	{
	    ajErr("ref ended before seq");
	    break;		/* end of file */
	}
	ajDebug("... read until next ref %d '%S'\n", ajFileTell(libr), rline);
    }

    /* get the encoding/sequence length info */

    ajRegExec (rexp, rline);
    ajRegSubI(rexp, 1, &reflibstr);

    /*
     *  if (!ajStrMatch(*libstr, reflibstr))
     *   ajDebug ("refid: '%S' seqid: '%S'\n", reflibstr, *libstr);
     */

    /*  iac = 0;*/
    parser[iparser].Parser(libr, alistfile, systemsort,
			   fields, maxFieldLen,
			   &reflibstr, fdl); /* writes alistfile data */

    /*
     *  if (ajStrMatch(*libstr, reflibstr))
     *  ajDebug ("refid '%S' seqid '%S'\n", reflibstr, *libstr);
     */

    /* get the description line */
    ajFileGets(libs, &sline);

    /* seek to the end of the sequence; +1 to jump over newline */
    if (ajStrChar(gcgtype,0)=='2')
    {
	rblock = (gcglen+3)/4;
	ajFileSeek(libs,rblock+1,SEEK_CUR);
    }
    else ajFileSeek(libs,gcglen+1,SEEK_CUR);

    /*
     *  for big entries, need to append until we have all the parts.
     *  They are named with _0 on the first part, _1 on the second and so on.
     *  or _00 on the first part, _01 on the second and so on.
     *  We can look for the "id_" prefix.
     */

    /*
     * i = ajStrLen(*libstr);
     */

    if (!ajStrSuffixC(*libstr, "_0") &&
	!ajStrSuffixC(*libstr,"_00") &&
	!ajStrSuffixC(*libstr,"_000"))
      return gcglen;
  
    gcglen += dbigcg_gcgappent (libr, libs, rexp, sexp,
				libstr);

    return gcglen;
}




/* @funcstatic dbigcg_pirgetent **********************************************
**
** get a single entry from the PIR database files
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [w] libstr [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field tokens
** @return [ajint] Sequence length
** @@
******************************************************************************/

static ajint dbigcg_pirgetent(AjPStr idformat,
			      AjPFile libr, AjPFile libs,
			      AjPFile* alistfile,
			      AjBool systemsort, AjPStr* fields,
			      ajint* maxFieldLen,
			      AjPStr* libstr, AjPList* fdl)
{
    static AjPStr reflibstr = NULL;
    ajint i;
    static ajint called = 0;
    static ajint iparser = -1;
    static AjPRegexp pirexp = NULL;
    ajint gcglen;
    static AjPStr rline=NULL;
    static AjPStr sline=NULL;
    ajint spos = 0;

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
	ajDebug ("idformat '%S' Parser %d\n", idformat, iparser);
	called = 1;
    }

    if (!pirexp)
	pirexp = ajRegCompC ("^>..;([^ \t\n]+)");

    ajStrAssC(&sline, "");
    ajStrAssC(&rline, "");

    /* skip to seqid first line */
    while (ajStrChar(sline,0)!='>')
    {
	if (!ajFileGets(libs, &sline))
	{
	    return 0;		/* end of file */
	}
    }

    /* get the encoding/sequence length info */

    ajDebug ("pirgetent line '%S' \n", sline);

    ajRegExec(pirexp, sline);

    /* skip to refid first line */
    while (ajStrChar(rline,0)!='>')
    {
	if (!ajFileGets(libr, &rline))
	{
	    ajErr("ref ended before seq");		/* end of file */
	    break;
	}
    }

    /* get the encoding/sequence length info */

    ajRegExec (pirexp, rline);
    ajRegSubI(pirexp, 1, &reflibstr);
    ajRegSubI(pirexp, 1, libstr);

    ajDebug ("pirgetent seqid '%S' spos: %ld\n", *libstr, ajFileTell(libs));
    ajDebug ("pirgetent refid '%S' spos: %ld\n", *libstr, ajFileTell(libr));

    /*
     *  if (!ajStrMatch(*libstr, reflibstr))
     *  ajDebug ("refid: '%S' seqid: '%S'\n", reflibstr, *libstr);
     */

    /*  iac = 0;*/
    parser[iparser].Parser (libr, alistfile,
			    systemsort, fields, maxFieldLen,
			    &reflibstr, fdl); /* writes alistfile data */

    /*
     *  if (ajStrMatch(*libstr, reflibstr))
     *   ajDebug ("refid '%S' seqid '%S'\n", reflibstr, *libstr);
     */

    /* get the description line */
    ajFileGets(libs, &sline);
    gcglen = 0;

    /* seek to the end of the sequence; +1 to jump over newline */
    while (ajStrChar(sline,0)!='>')
    {
	spos = ajFileTell(libs);
	if (!ajFileGets(libs, &sline))
	{
	    spos=0;
	    break;
	}
	gcglen += ajStrLen(sline);
    }

    if (spos)
      ajFileSeek(libs, spos, 0);

    ajDebug ("pirgetent end spos %ld line '%S'\n", spos, sline);

    return gcglen;
}



/* @funcstatic dbigcg_gcgappent ***********************************************
**
** Go to end of a split GCG entry
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [w] libstr [AjPStr*] ID
** @return [ajint] Sequence length for this section
** @@
******************************************************************************/

static ajint dbigcg_gcgappent (AjPFile libr, AjPFile libs,
			       AjPRegexp rexp, AjPRegexp sexp,
			       AjPStr* libstr)
{

    /* keep reading until we reach the end of entry
       and return the extra number of bases*/

    static AjPStr reflibstr = NULL;
    static AjPStr seqlibstr = NULL;
    static AjPStr testlibstr = NULL;
    ajint ilen;
    static AjPStr tmpstr = NULL;
    static AjPStr rline=NULL;
    static AjPStr sline=NULL;

    AjBool isend;
    char *p;
    char *q;
    ajint rpos;
    ajint spos;
  
    if(!testlibstr)
	testlibstr = ajStrNew();

    ajStrAssS(&tmpstr,*libstr);
  
    ajDebug("dbi_gcgappent '%S'\n", tmpstr);

    p = ajStrStr(tmpstr);
    q = strrchr(p,'_');
    *q='\0';
  

    ajFmtPrintS(&testlibstr, "%s_",p);
    ilen = ajStrLen(testlibstr);

    isend = ajFalse;
  
    while(!isend)
    {
        spos = ajFileTell(libs);
	ajFileGets(libs,&sline);
	while (strncmp(ajStrStr(sline),">>>>",4))
	{
	    spos = ajFileTell(libs);
	    if (!ajFileGets(libs, &sline)) {
		ajDebug ("end of file on seq\n");
		return 1;
	    }
	}
      
	ajRegExec (sexp, sline);
	ajRegSubI(sexp, 1, &seqlibstr);

	rpos = ajFileTell(libr);
	ajFileGets(libr, &rline);
      
	while (ajStrChar(rline,0)!='>') {
	    rpos = ajFileTell(libr);
	    if (!ajFileGets(libr, &rline))
	    {
		ajDebug ("ref ended before seq\n");
		ajErr ("ref ended before seq\n");
		break;
	    }
	}

	ajRegExec (rexp, rline);
	ajRegSubI (rexp, 1, &reflibstr);

	if (ajStrNCmpO(reflibstr, testlibstr, ilen) ||
	    ajStrNCmpO(seqlibstr, testlibstr, ilen))
	    isend = ajTrue;

	ajDebug("gcgappent %B test: '%S' seq: '%S' ref: '%S'\n",
		isend, testlibstr, seqlibstr, reflibstr);
    }
  
    ajDebug("gcgappent done at seq: '%S' ref: '%S'\n", seqlibstr, reflibstr);

    ajStrAssC(libstr,p);
  
    ajFileSeek (libr, rpos, 0);
    ajFileSeek (libs, spos, 0);

    return 1;
}


/* @funcstatic dbigcg_ParseEmbl ********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field tokens
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbigcg_ParseEmbl (AjPFile libr,
				AjPFile* alistfile,
				AjBool systemsort, AjPStr* fields,
				ajint* maxFieldLen,
				AjPStr* id, AjPList* fdl)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp idexp = NULL;
    static AjPRegexp verexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPStr tmpstr = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr tmpfd = NULL;
    static AjPStr typStr = NULL;
    AjPStr tmpacnum = NULL;
    char* fd;
    ajint lineType;
    ajint rpos;
    static AjPStr rline=NULL;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint keyfield=-1;
    static ajint taxfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;
  
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
	wrdexp = ajRegCompC ("([A-Za-z0-9]+)");

    if (!verexp)
	verexp = ajRegCompC ("([A-Za-z0-9]+[.][0-9]+)");

    if (!phrexp)
	phrexp = ajRegCompC (" *([^;.\n\r]+)");

    if (!taxexp)
	taxexp = ajRegCompC (" *([^;.\n\r()]+)");

    if (!idexp)
	idexp = ajRegCompC ("^ID   ([^ \t]+)");

    rpos = ajFileTell(libr);
    while (ajFileGets(libr, &rline) && ajStrChar(rline,0)!='>')
    {
        rpos = ajFileTell(libr);
	ajStrAssS(&tmpstr,rline);
      
	if (ajRegExec (typexp, tmpstr))
	{
	    ajRegSubI (typexp, 1, &typStr);
	    if (ajStrMatchC(typStr, "ID")) lineType = GCGTYPE_ID;
	    else if (ajStrMatchC(typStr, "SV")) lineType = GCGTYPE_VER;
	    else if (ajStrMatchC(typStr, "AC")) lineType = GCGTYPE_ACC;
	    else if (ajStrMatchC(typStr, "DE")) lineType = GCGTYPE_DES;
	    else if (ajStrMatchC(typStr, "KW")) lineType = GCGTYPE_KEY;
	    else if (ajStrMatchC(typStr, "OS")) lineType = GCGTYPE_TAX;
	    else if (ajStrMatchC(typStr, "OC")) lineType = GCGTYPE_TAX;
	    else lineType=GCGTYPE_OTHER;
	    if (lineType != GCGTYPE_OTHER)
	      ajRegPost (typexp, &tmpline);
	}
	else
	  lineType = GCGTYPE_OTHER;

	if (lineType == GCGTYPE_ID)
	{
	    ajRegExec (idexp, rline);
	    ajRegSubI (idexp, 1, id);
	    ajDebug("++id '%S'\n", *id);
	    continue;
	}

	if (lineType == GCGTYPE_ACC && accfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);
		if (!tmpacnum)
		  ajStrAssS(&tmpacnum, tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);
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
	else if (lineType == GCGTYPE_DES && desfield >= 0)
	{
	    while (ajRegExec(wrdexp, tmpline))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++des '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);
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
	else if (lineType == GCGTYPE_VER && svnfield >= 0)
	{
	    while (ajRegExec(verexp, tmpline))
	    {
		ajRegSubI (verexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++sv '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[svnfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[svnfield], fd);
		}
		ajRegPost (verexp, &tmpline);
	    }
	    continue;
	}
	else if (lineType == GCGTYPE_KEY && keyfield >= 0)
	{
	    while (ajRegExec(phrexp, tmpline))
	    {
		ajRegSubI (phrexp, 1, &tmpfd);
		ajRegPost (phrexp, &tmpline);
		ajStrChompEnd(&tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajStrToUpper(&tmpfd);
		ajDebug("++key '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);
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
	else if (lineType == GCGTYPE_TAX && taxfield >= 0)
	{
	    while (ajRegExec(taxexp, tmpline))
	    {
		ajRegSubI (taxexp, 1, &tmpfd);
		ajRegPost (taxexp, &tmpline);
		ajStrToUpper(&tmpfd);
		ajStrChompEnd(&tmpfd);
		if (!ajStrLen(tmpfd)) continue;
		ajDebug("++tax '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);
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
    if (rpos)
        ajFileSeek(libr, rpos, 0);

    ajStrDel(&tmpacnum);

    return ajFalse;
}




/* @funcstatic dbigcg_ParseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field tokens
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbigcg_ParseGenbank (AjPFile libr,
				   AjPFile* alistfile,
				    AjBool systemsort, AjPStr* fields,
				    ajint* maxFieldLen,
				    AjPStr* id, AjPList* fdl)
{
    static AjPRegexp typexp = NULL;
    static AjPRegexp morexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp verexp = NULL;
    ajint rpos = 0;
    static AjPStr tmpstr = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr rline=NULL;
    static AjPStr tmpfd = NULL;
    static AjPStr typStr=NULL;
    ajint lineType=GCGTYPE_OTHER;
    char* fd;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint keyfield=-1;
    static ajint taxfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

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
	typexp = ajRegCompC ("^(  )?([A-Z]+)");

    if (!morexp)
	morexp = ajRegCompC ("^            ");

    if (!wrdexp)
	wrdexp = ajRegCompC ("([A-Za-z0-9]+)");

    if (!phrexp)
	phrexp = ajRegCompC (" *([^;.\n\r]+)");

    if (!taxexp)
	taxexp = ajRegCompC (" *([^;.\n\r()]+)");

    if (!verexp)
	verexp = ajRegCompC ("([A-Za-z0-9]+)( +GI:([0-9]+))?");

    while (ajFileGets(libr, &rline) && ajStrChar(rline,0)!='>')
    {
        rpos = ajFileTell(libr);
	ajStrAssS(&tmpstr,rline);
      
	if (ajRegExec (typexp, tmpstr))
	{
	    ajRegSubI (typexp, 2, &typStr);
	    if (ajStrMatchC(typStr, "LOCUS")) lineType = GCGTYPE_ID;
	    else if (ajStrMatchC(typStr, "VERSION")) lineType = GCGTYPE_VER;
	    else if (ajStrMatchC(typStr, "ACCESSION")) lineType = GCGTYPE_ACC;
	    else if (ajStrMatchC(typStr, "DEFINITION")) lineType = GCGTYPE_DES;
	    else if (ajStrMatchC(typStr, "KEYWORDS")) lineType = GCGTYPE_KEY;
	    else if (ajStrMatchC(typStr, "ORGANISM")) lineType = GCGTYPE_TAX;
	    else lineType=GCGTYPE_OTHER;
	    if (lineType != GCGTYPE_OTHER)
	      ajRegPost (typexp, &tmpline);
	    ajDebug("++type line %d\n", lineType);
	}
	else if (lineType != GCGTYPE_OTHER && ajRegExec (morexp, rline))
	{
	  ajRegPost (morexp, &tmpline);
	  ajDebug("++more line %d\n", lineType);
	}
	else
	  lineType = GCGTYPE_OTHER;

	if (lineType == GCGTYPE_ID)
	{
	  ajRegExec (wrdexp, tmpline);
	  ajRegSubI (wrdexp, 1, id);
	}

	else if (lineType == GCGTYPE_ACC && accfield >= 0)
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

	else if (lineType == GCGTYPE_DES && desfield >= 0)
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

	else if (lineType == GCGTYPE_KEY && keyfield >= 0)
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

	else if (lineType == GCGTYPE_TAX && taxfield >= 0)
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

	else if (lineType == GCGTYPE_VER && svnfield >= 0)
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

    }
    if (rpos)
      ajFileSeek(libr, rpos, 0);

    return ajFalse;
}


/* @funcstatic dbigcg_ParsePir ********************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Lists of field tokens
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbigcg_ParsePir (AjPFile libr,
			       AjPFile* alistfile,
			       AjBool systemsort, AjPStr* fields,
			       ajint* maxFieldLen,
			       AjPStr* id, AjPList* fdl)
{
    static AjPRegexp idexp = NULL;
    static AjPRegexp acexp = NULL;
    static AjPRegexp ac2exp = NULL;
    static AjPRegexp keyexp = NULL;
    static AjPRegexp taxexp = NULL;
    static AjPRegexp wrdexp = NULL;
    static AjPRegexp phrexp = NULL;
    ajint rpos;
    static AjPStr tmpstr = NULL;
    static AjPStr tmpline = NULL;
    static AjPStr rline = NULL;
    static AjPStr tmpfd = NULL;
    char* fd;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint keyfield=-1;
    static ajint taxfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

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

    if (!wrdexp)
	wrdexp = ajRegCompC ("([A-Za-z0-9]+)");

    if (!idexp)
	idexp = ajRegCompC ("^>..;([^;.\n\r]+)");

    if (!phrexp)		/* allow . for "sp." */
	phrexp = ajRegCompC (" *([^,;\n\r]+)");

    if (!acexp)
	acexp = ajRegCompC ("^C;Accession:");

    if (!ac2exp)
	ac2exp = ajRegCompC ("([A-Za-z0-9]+)");

    if (!taxexp)
	taxexp = ajRegCompC ("^C;Species:");

    if (!keyexp)
	keyexp = ajRegCompC ("^C;Keywords:");

    rpos = ajFileTell(libr);

    ajDebug ("++id '%S'\n", *id);

    /*
    ajFileGets(libr, &rline);
    ajDebug ("line-1 '%S'\n", rline);
    if (!ajRegExec(idexp, rline))
      return ajFalse;
    ajRegSubI (idexp, 1, id);

    ajDebug ("++id '%S'\n", *id);
    */

    ajFileGets(libr, &rline);
    ajDebug ("line-2 '%S'\n", rline);
    while (ajRegExec(wrdexp, rline))
    {
	ajRegSubI (wrdexp, 1, &tmpfd);
	ajStrToUpper(&tmpfd);
	ajDebug("++des '%S'\n", tmpfd);
	embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);
	if (systemsort)
	  ajFmtPrintF (alistfile[desfield], "%S %S\n", *id, tmpfd);
	else
	{
	    fd = ajCharNew(tmpfd);
	    ajListPushApp (fdl[desfield], fd);
	}
	ajRegPost (wrdexp, &rline);
    }

    while (ajStrChar(rline,0)!='>')
    {
        rpos = ajFileTell(libr);
	ajStrAssS(&tmpstr,rline);
      
	if (ajRegExec (acexp, rline))
	{
	    ajRegPost (acexp, &tmpline);
	    while (ajRegExec(ac2exp, tmpline))
	    {
		ajRegSubI (ac2exp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++acc '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[accfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[accfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[accfield], fd);
		}
		ajRegPost (ac2exp, &tmpline);
	    }
	}

	if (ajRegExec (keyexp, rline))
	{
	    ajRegPost (keyexp, &tmpline);
	    while (ajRegExec(phrexp, tmpline))
	    {
		ajRegSubI (phrexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++key '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[keyfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[keyfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[keyfield], fd);
		}
		ajRegPost (phrexp, &tmpline);
	    }
	}

	if (ajRegExec (taxexp, rline))
	{
	    ajRegPost (taxexp, &tmpline);
	    while (ajRegExec(phrexp, tmpline))
	    {
		ajRegSubI (phrexp, 1, &tmpfd);
		ajStrToUpper(&tmpfd);
		ajDebug("++tax '%S'\n", tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[taxfield]);
		if (systemsort)
		    ajFmtPrintF (alistfile[taxfield], "%S %S\n", *id, tmpfd);
		else
		{
		    fd = ajCharNew(tmpfd);
		    ajListPushApp (fdl[taxfield], fd);
		}
		ajRegPost (phrexp, &tmpline);
	    }
	}

	if (!ajFileGets(libr, &rline))
	{
	    /*      ddone = 1;*/
	    rpos = 0;
	    break;
	}
    }

    if (rpos)
      ajFileSeek (libr, rpos, 0);

    return ajFalse;
}

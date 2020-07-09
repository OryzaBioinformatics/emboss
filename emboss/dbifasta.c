/******************************************************************************
**
** EMBOSS/Staden/EMBLCD indexing
**
** This version reads a flat file database,
** and writes entryname and accession index files.
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

#define BIGOVERLAP 10000;
#define SIMPLE    1
#define IDACC     2
#define GCGID     3
#define GCGIDACC  4
#define NCBI      5
#define DBID      6
#define ACCID     7
#define GCGACCID  8

AjPList idlist;
AjPList aclist;

AjBool systemsort;
AjBool cleanup;

static ajint maxidlen = 20;
static ajint maxaclen = 20;

static AjPStr rline = NULL;

static AjPStr lastidstr = NULL;

static AjPFile elistfile=NULL;
static AjPFile alistfile=NULL;
static AjPFile blistfile=NULL;

static AjPStr dbname = NULL;
static AjPStr release = NULL;
static AjPStr datestr = NULL;
static AjPStr sortopt = NULL;

static AjBool dbifastaParseFasta   (AjPFile libr, ajint *dpos,
			    AjPStr* id, AjPList acl, AjPRegexp exp,
			    ajint type);

static EmbPentry dbifastaNextFlatEntry (AjPFile libr, ajint ifile,
					AjPStr idformat, AjPRegexp exp,
					ajint type);

static AjPRegexp getExpr(AjPStr idformat, ajint *type);



/* @prog dbifasta *************************************************************
**
** Index a fasta database
**
******************************************************************************/

int main(int argc, char **argv)
{

  AjBool staden;
  AjPStr directory;
  AjPStr indexdir;
  AjPStr filename;
  AjPStr exclude;
  AjPStr curfilename = NULL;
  AjPStr elistfname = NULL;
  AjPStr alistfname = NULL;
  AjPStr blistfname = NULL;
  AjPStr idformat = NULL;
  AjPFile libr=NULL;

  EmbPentry entry;
  EmbPac acnum=NULL;
  char* lastac=NULL;
  ajint idtype=0;
  
  ajint i;
  ajint j;
  ajint k;
  ajint nac;
  ajint nid=0;
  ajint iac=0;
  void **ids = NULL;
  void **acs = NULL;
  AjPList inlist = NULL;
  void ** files = NULL;
  ajint nfiles;
  ajint ifile;

  ajint dsize;
  ajint esize;
  ajint asize;
  ajint ahsize;
  short recsize;
  short elen;
  short alen;
  ajint maxfilelen=20;
  char date[4] = {0,0,0,0};
  char padding[256];
  ajint ient;
  ajint filenum;
  ajint rpos;
  ajint spos;
  ajint idnum;

  AjPStr tmpstr = NULL;
  AjPStr idstr = NULL;
  AjPStr acstr = NULL;

  AjPRegexp datexp = NULL;
  AjPRegexp idsrtexp = NULL;
  AjPRegexp acsrtexp = NULL;
  AjPRegexp acsrt2exp = NULL;

  AjPStr  dfname = NULL;
  AjPStr  efname = NULL;
  AjPStr atfname = NULL;
  AjPStr ahfname = NULL;

  AjPFile  dfile = NULL;
  AjPFile  efile = NULL;
  AjPFile atfile = NULL;
  AjPFile ahfile = NULL;

  AjPStr* divfiles = NULL;
  AjPRegexp exp = NULL;
  
  datexp = ajRegCompC("^([0-9]+).([0-9]+).([0-9]+)");
  idsrtexp = ajRegCompC ("^([^ ]+) +([0-9]+) +([0-9]+) +([0-9]+)");
  acsrtexp = ajRegCompC ("^([^ ]+) +([^ \n]+)");
  acsrt2exp = ajRegCompC ("^([^ ]+) +([0-9]+)");

  for (i=0;i<256;i++)
    padding[i] = ' ';

  embInit ("dbifasta", argc, argv);

  staden = ajAcdGetBool ("staden");
  idformat = ajAcdGetListI ("idformat",1);
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


  exp = getExpr(idformat, &idtype);


  if (ajRegExec (datexp, datestr)) {
    for (i=1; i<4; i++) {
      ajRegSubI (datexp, i+1, &tmpstr);
      ajStrToInt (tmpstr, &j);
      date[3-i] = j;
    }
  }
  ajDebug ("staden: %B idformat: '%S'\n", staden, idformat);
  ajDebug ("reading '%S/%S'\n", directory, filename);
  ajDebug ("writing '%S/'\n", indexdir);

  idlist = ajListNew ();
  aclist = ajListNew ();

  inlist = embDbiFileListExc (directory, filename, exclude);
  ajListSort (inlist, ajStrCmp);
  nfiles = ajListToArray(inlist, &files);

  if (!nfiles)
    ajFatal ("No files selected");

  AJCNEW0(divfiles, nfiles);

  if (systemsort)
    acnum = embDbiAcnumNew();

  for (ifile=0; ifile<nfiles; ifile++) {
    curfilename = (AjPStr) files[ifile];
    embDbiFlatOpenlib (curfilename, &libr);
    ajDebug ("processing '%S' ...\n", curfilename);
    ajDebug ("processing '%F' ...\n", libr);
    ajStrAssS (&divfiles[ifile], curfilename);
    ajFileNameTrim(&divfiles[ifile]);
    if (systemsort) {
      ajFmtPrintS (&elistfname, "%S%02d.list", dbname, ifile+1);
      elistfile = ajFileNewOut (elistfname);
      if(!elistfile)
	  ajFatal("Cannot open %S for writing",elistfname);
      
      ajDebug ("elistfile %F\n", elistfile);
      ajFmtPrintS (&alistfname, "%S%02d.acid", dbname, ifile+1);
      alistfile = ajFileNewOut (alistfname);
      if(!alistfile)
	  ajFatal("Cannot open %S for writing",alistfname);
      ajDebug ("alistfile %F\n", alistfile);
    }

    if (ajStrLen(curfilename) >= maxfilelen)
      maxfilelen = ajStrLen(curfilename) + 1;
    while ((entry=dbifastaNextFlatEntry(libr, ifile, idformat,exp,idtype))) {
      if (systemsort) {
	nid++;
      }
      else {
	entry->filenum = ifile+1;
	ajListPushApp (idlist, entry);
	for (i=0;i<entry->nac; i++) {
	  if (!systemsort)
	    acnum = embDbiAcnumNew();
	  acnum->entry = entry->entry;
	  acnum->ac = entry->ac[i];
	  ajListPushApp (aclist, acnum);
	}
      }

    }
    ajFileClose (&elistfile);
    ajFileClose (&alistfile);
  }
  if (systemsort) {
    embDbiSortFile (dbname, "list", "idsrt", nfiles, cleanup, sortopt);
    embDbiSortFile (dbname, "acid", "acsrt", nfiles, cleanup, sortopt);

    /* put in the entry numbers and remove the names */
    /* read dbname.acsrt, for each entry, increment the count */

    ajFmtPrintS (&alistfname, "%S.acsrt", dbname);
    alistfile = ajFileNewIn (alistfname);
    if(!alistfile)
	ajFatal("Cannot open %S for reading",alistfname);
    ajFmtPrintS (&blistfname, "%S.acid2", dbname);
    blistfile = ajFileNewOut (blistfname);
    if(!blistfile)
	ajFatal("Cannot open %S for writing",blistfname);

    ient=0;
    nac = 0;
    while (ajFileGets (alistfile, &rline)) {
      ajRegExec (acsrtexp, rline);
      ajRegSubI (acsrtexp, 1, &idstr);
      ajRegSubI (acsrtexp, 2, &acstr);
      if (!nac)
	ajStrAssS (&lastidstr, idstr);
      if (!ajStrMatch (idstr, lastidstr)) {
	ient++;
	ajStrAssS (&lastidstr, idstr);
      }
      ajFmtPrintF (blistfile, "%S %d\n", acstr, ient+1);
      nac++;
    }

    ajFileClose (&alistfile);
    ajFileClose (&blistfile);

    /* sort again */

    embDbiRmFile (dbname, "acsrt", 0, cleanup);
    embDbiSortFile (dbname, "acid2", "acsrt2", 0, cleanup, sortopt);
 
  }
    
  else {
    nid = ajListToArray (idlist, &ids);
    nac = ajListToArray (aclist, &acs);

    ajDebug ("ids: %d %x acs: %d %x\n", nid, ids, nac, acs);
    /*
      for (i=0; i<nid; i++) {
      ajDebug("ids %3d %x %x '%s'\n",
      i, &ids[i], ids[i], ((EmbPentry)ids[i])->entry);
      }
    */
    qsort (ids, nid, sizeof(void*), embDbiCmpId);
    ajDebug ("ids sorted\n");

    /*
      for (i=0; i<nid; i++) {
      ajDebug("sort ids %3d %x %x '%s'\n",
      i, &ids[i], ids[i], ((EmbPentry)ids[i])->entry);
      }
    */

    qsort (acs, nac, sizeof(void*), embDbiCmpAcId);
    ajDebug ("acs sorted by id\n");
    /*
      for (i=0; i<nac; i++) {
      ajDebug("sort acs %3d %x %x '%s' '%s'\n",
      i, &acs[i], acs[i], ((EmbPac)acs[i])->entry,((EmbPac)acs[i])->ac);
      }
    */
    i=0;
    j=0;

    while (ids[i] && acs[j]) {
      k = strcmp(((EmbPentry)ids[i])->entry, ((EmbPac)acs[j])->entry);
      if (k < 0) {
	i++;
      }
      else if (k > 0) {
	j++;
      }

      else {
	((EmbPac)acs[j++])->nid = i+1;	/* we need (i+1) */
      }
    }
    ajDebug ("checked ids: %d %d acs: %d %d\n", i, nid, j, nac);

    qsort (acs, nac, sizeof(void*), embDbiCmpAcAc);
    ajDebug ("acs sorted by ac\n");
    /*
      for (i=0; i<nac; i++) {
      ajDebug("sort acs %3d %x %x '%s' '%s' %d\n",
      i, &acs[i], acs[i], ((EmbPac)acs[i])->entry,((EmbPac)acs[i])->ac,
      ((EmbPac)acs[i])->nid);
      }
    */

  }

  /* write the division file */

  ajStrAssC (&dfname, "division.lkp");
  dfile = ajFileNewOutD(indexdir, dfname);
  if(!dfile)
      ajFatal("Cannot open %S for writing",dfname);

  dsize = 256 + 44 + (nfiles * (maxfilelen+2));
  ajFileWriteInt4 (dfile, dsize); /* filesize */

  ajFileWriteInt4 (dfile, nfiles); /* #records */

  recsize = maxfilelen + 2;
  ajFileWriteInt2 (dfile, recsize); /* recordsize */

  /* rest of the header */
  ajFileWriteStr  (dfile, dbname,  20); /* dbname */
  ajFileWriteStr  (dfile, release, 10); /* release */
  ajFileWriteByte (dfile, date[0]); /* release date */
  ajFileWriteByte (dfile, date[1]); /* release date */
  ajFileWriteByte (dfile, date[2]); /* release date */
  ajFileWriteByte (dfile, date[3]); /* release date */
  ajFileWrite (dfile, padding, 256, 1); /* padding 256 bytes */

  for (i=0; i<nfiles; i++) {
    ajFileWriteInt2 (dfile, (short)(i+1));
    ajFileWriteStr (dfile, divfiles[i], maxfilelen);
  }
  ajFileClose (&dfile);

  /* write the entry file */

  ajStrAssC (&efname, "entrynam.idx");
  efile = ajFileNewOutD(indexdir, efname);
  if(!efile)
      ajFatal("Cannot open %S for writing",efname);

  ajDebug("writing entrynam.idx %d\n", nid);

  elen = maxidlen+10;
  esize = 300 + (nid*(ajint)elen);
  ajFileWriteInt4 (efile, esize);
  ajFileWriteInt4 (efile, nid);
  ajFileWriteInt2 (efile, elen);

  /* rest of the header */
  ajFileWriteStr  (efile, dbname,  20); /* dbname */
  ajFileWriteStr  (efile, release, 10); /* release */
  ajFileWriteByte (efile, date[0]); /* release date */
  ajFileWriteByte (efile, date[1]); /* release date */
  ajFileWriteByte (efile, date[2]); /* release date */
  ajFileWriteByte (efile, date[3]); /* release date */
  ajFileWrite (efile, padding, 256, 1); /* padding 256 bytes */

  if (systemsort) {
    ajStrAssC(&lastidstr, " ");
    ajFmtPrintS (&elistfname, "%S.idsrt", dbname);
    elistfile = ajFileNewIn (elistfname);
    if(!elistfile)
	ajFatal("Cannot open %S for reading",elistfname);
    while (ajFileGets (elistfile, &rline)) {
      ajRegExec (idsrtexp, rline);
      ajRegSubI (idsrtexp, 1, &idstr);
      if (ajStrMatchCase(idstr, lastidstr)) {
	ajWarn ("Duplicate ID skipped: '%S'", idstr);
	continue;
      }
      ajRegSubI (idsrtexp, 2, &tmpstr);
      ajStrToInt (tmpstr, &rpos);
      ajRegSubI (idsrtexp, 3, &tmpstr);
      ajStrToInt (tmpstr, &spos);
      ajRegSubI (idsrtexp, 4, &tmpstr);
      ajStrToInt (tmpstr, &filenum);
      ajFileWriteStr (efile, idstr, maxidlen);
      ajFileWriteInt4 (efile, rpos);
      ajFileWriteInt4 (efile, spos);
      ajFileWriteInt2 (efile, filenum);
      ajStrAss  (&lastidstr, idstr);
    }
    ajFileClose (&elistfile);
    embDbiRmFile (dbname, "idsrt", 0, cleanup);
  }
  else {
    for (i = 0; i < nid; i++) {
      entry = (EmbPentry)ids[i];
      ajFileWriteChar (efile, entry->entry, maxidlen);
      ajFileWriteInt4 (efile, entry->rpos);
      ajFileWriteInt4 (efile, entry->spos);
      ajFileWriteInt2 (efile, entry->filenum);
    }
  }
  ajFileClose (&efile);

  /* write the accession files */

  ajStrAssC (&atfname, "acnum.trg");
  atfile = ajFileNewOutD(indexdir, atfname);
  if(!atfile)
      ajFatal("Cannot open %S for writing",atfname);
  ajStrAssC (&ahfname, "acnum.hit");
  ahfile = ajFileNewOutD(indexdir, ahfname);
  if(!ahfile)
      ajFatal("Cannot open %S for writing",ahfname);
  if (!systemsort)
    lastac = ((EmbPac)acs[0])->ac;

  ajDebug("writing acnum.hit %d\n", nac);

  alen = maxaclen+8;
  asize = 300 + (nac*(ajint)alen); /* to be fixed later */
  ajFileWriteInt4 (atfile, asize);
  ajFileWriteInt4 (atfile, nac);
  ajFileWriteInt2 (atfile, alen);

  /* rest of the header */
  ajFileWriteStr  (atfile, dbname,  20); /* dbname */
  ajFileWriteStr  (atfile, release, 10); /* release */
  ajFileWriteByte (atfile, date[0]); /* release date */
  ajFileWriteByte (atfile, date[1]); /* release date */
  ajFileWriteByte (atfile, date[2]); /* release date */
  ajFileWriteByte (atfile, date[3]); /* release date */
  ajFileWrite (atfile, padding, 256, 1); /* padding 256 bytes */

  ahsize = 300 + (nac*4);
  ajFileWriteInt4 (ahfile, ahsize);
  ajFileWriteInt4 (ahfile, nac);
  ajFileWriteInt2 (ahfile, 4);

  /* rest of the header */
  ajFileWriteStr  (ahfile, dbname,  20); /* dbname */
  ajFileWriteStr  (ahfile, release, 10); /* release */
  ajFileWriteByte (ahfile, date[0]); /* release date */
  ajFileWriteByte (ahfile, date[1]); /* release date */
  ajFileWriteByte (ahfile, date[2]); /* release date */
  ajFileWriteByte (ahfile, date[3]); /* release date */
  ajFileWrite (ahfile, padding, 256, 1); /* padding 256 bytes */

  iac=0;
  j = 0;
  k = 0;
  if (systemsort) {
    i=0;
    ajFmtPrintS (&alistfname, "%S.acsrt2", dbname);
    alistfile = ajFileNewIn (alistfname);
    if(!alistfile)
	ajFatal("Cannot open %S for reading",alistfname);
    while (ajFileGets (alistfile, &rline)) {
      ajRegExec (acsrt2exp, rline);
      ajRegSubI (acsrt2exp, 1, &idstr);
      ajRegSubI (acsrt2exp, 2, &tmpstr);
      ajStrToInt (tmpstr, &idnum);
      ajFileWriteInt4 (ahfile, idnum);
      if (!i)
	ajStrAssS (&lastidstr, idstr);
      if (!ajStrMatch(lastidstr, idstr)) {
	ajFileWriteInt4 (atfile, j);
	ajFileWriteInt4 (atfile, k);
	ajFileWriteStr (atfile, lastidstr, maxaclen);
	j = 0;			/* number of hits */
	k = i+1;		/* first hit */
	ajStrAssS (&lastidstr, idstr);
	iac++;
      }
      j++;
      i++;
    }
    ajFileClose (&alistfile);
    embDbiRmFile (dbname, "acsrt2", 0, cleanup);
    ajFileWriteInt4 (atfile, j);
    ajFileWriteInt4 (atfile, k);
    ajFileWriteStr (atfile, lastidstr, maxaclen);
    iac++;
  }
  else {
    for (i = 0; i < nac; i++) {
      acnum = (EmbPac)acs[i];
      ajFileWriteInt4 (ahfile, acnum->nid);
      if (strcmp(lastac, acnum->ac)) {
	ajFileWriteInt4 (atfile, j);
	ajFileWriteInt4 (atfile, k);
	ajFileWriteChar (atfile, lastac, maxaclen);
	j = 0;
	k = i;
	lastac = acnum->ac;
	iac++;
      }
      j++;
    }
    ajFileWriteInt4 (atfile, j);
    ajFileWriteInt4 (atfile, k);
    ajFileWriteChar (atfile, lastac, maxaclen);
    iac++;
  }

  ajDebug ("wrote acnum.trg %d\n", iac);
  ajFileSeek (atfile, 0, 0);	/* fix up the record count */
  ajFileWriteInt4 (atfile, 300+iac*(ajint)alen);
  ajFileWriteInt4 (atfile, iac);

  ajFileClose (&atfile);
  ajFileClose (&ahfile);

  ajDebug ("finished...\n%7d files\n%7d entries\n%7d acnum.trg\n%7d acnum.hit\n",
	  nfiles, nid, iac, nac);
  ajExit ();
  return 0;
}

/* @funcstatic dbifastaNextFlatEntry ******************************************
**
** Returns next database entry as an EmbPentry object
**
** @param [r] libr [AjPFile] Database file
** @param [r] ifile [ajint] File number.
** @param [r] idformat [AjPStr] type of id line
** @param [r] exp [AjPRegexp] Regular expression for id parsing
** @param [r] type [ajint] type of fasta id.
** @return [EmbPentry] Entry data object.
** @@
******************************************************************************/

static EmbPentry dbifastaNextFlatEntry (AjPFile libr, ajint ifile,
					AjPStr idformat, AjPRegexp exp,
					ajint type)
{

  static EmbPentry ret=NULL;
  ajint ir;
  ajint is = 0;
  static AjPStr id = NULL;
  char* ac;
  ajint i;
  static AjPList acl = NULL;

  if (!acl)
    acl = ajListNew();

  if (!ret || !systemsort)
    ret = embDbiEntryNew();

  if (!dbifastaParseFasta(libr, &ir, &id, acl, exp, type))
     return NULL;

  /* id to ret->entry */

  if (systemsort) {
    ajFmtPrintF (elistfile, "%S %d %d %d\n", id, ir, is, ifile+1);
  }
  else {
    ret->entry = ajCharNew(id);
    ret->rpos = ir;
    ret->spos = is;
    ret->filenum = ifile+1;

  /* ac as list, then move to ret->ac */

    ret->nac = ajListLength(acl);

    if (ret->nac) {
      AJCNEW(ret->ac,ret->nac);

      i=0;
      while (ajListPop(acl, (void**) &ac)) {
	ret->ac[i++] = ac;
      }
    }
    else {
      ret->ac = NULL;
    }

    /*
      ajDebug("id '%s' %d %d nac: %d\n", ret->entry, ir, is, ret->nac);
      for (i=0; i<ret->nac; i++)
      ajDebug("   %3d %s\n", i, ret->ac[i]);
    */
  }

  return ret;
}

/* @funcstatic getExpr *****************************************************
**
** Compile regular expression
**
** @param [r] idformat [AjPStr] type of ID line
** @param [w] type [ajint *] numeric type
** @return [AjPRegexp] ajTrue on success.
** @@
******************************************************************************/
static AjPRegexp getExpr(AjPStr idformat, ajint *type)
{
    AjPRegexp exp=NULL;
    
    if(ajStrMatchC(idformat,"simple"))
    {
	*type = SIMPLE;
	exp = ajRegCompC("^>([.A-Za-z0-9_-]+)");
    }
    else if(ajStrMatchC(idformat,"idacc"))
    {
	*type = IDACC;
	exp = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"accid"))
    {
	*type = ACCID;
	exp = ajRegCompC("^>([A-Za-z0-9_-]+)+[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgid"))
    {
	*type = GCGID;
	exp = ajRegCompC("^>[A-Za-z0-9_-]+:([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgidacc"))
    {
	*type = GCGIDACC;
	exp = ajRegCompC(
			 "^>[A-Za-z0-9_-]+:([A-Za-z0-9-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"gcgaccid"))
    {
	*type = GCGACCID;
	exp = ajRegCompC(
			 "^>[A-Za-z0-9_-]+:([A-Za-z0-9-]+)[ \t]+([A-Za-z0-9-]+)");
    }
    else if(ajStrMatchC(idformat,"ncbi"))
    {
	exp = ajRegCompC("^>([A-Za-z0-9_-]+)"); /* dummy regexp */
	*type = NCBI;
    }
    else if(ajStrMatchC(idformat,"dbid"))
    {
	exp = ajRegCompC("^>[A-Za-z0-9_-]+[ \t]+([A-Za-z0-9_-]+)");
	*type = DBID;
    }
    else
	return NULL;
    
    return exp;
}

/* @funcstatic dbifastaParseFasta *********************************************
**
** Parse the ID, accession from a FASTA format sequence
**
** @param [r] libr [AjPFile] Input database file
** @param [w] dpos [ajint*] Byte offset
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @param [r] exp [AjPRegexp] regular expression
** @param [r] type [ajint] type of id line
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool dbifastaParseFasta (AjPFile libr, ajint* dpos,
				  AjPStr* id, AjPList acl, AjPRegexp exp,
				  ajint type)
{
  static AjPStr tmpac = NULL; 
  static AjPStr token = NULL;
  char* ac;
  ajint ipos;
  static ajint num = 1;
  static AjPStr tstr = NULL;


  if(!tstr)
      tstr = ajStrNew();

  *dpos = ajFileTell(libr);
  ajFileGets (libr, &rline);
  

  if(!ajRegExec(exp,rline))
  {
      ajStrDelReuse(&tmpac);
      ajDebug("Invalid ID line [%S]",rline);
      return ajFalse;
  }
  

  switch(type)
  {
  case SIMPLE:
      ajRegSubI(exp,1,id);
      (void) ajStrToUpper(id);
      (void) ajStrAss(&tmpac,*id);
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
          ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      ajStrDelReuse(&tmpac);
      break;
  case DBID:
      ajRegSubI(exp,1,id);
      (void) ajStrToUpper(id);
      (void) ajStrAss(&tmpac,*id);
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
          ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      ajStrDelReuse(&tmpac);
      break;
  case GCGID:
      ajRegSubI(exp,1,id);
      (void) ajStrToUpper(id);
      (void) ajStrAss(&tmpac,*id);
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
          ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      ajStrDelReuse(&tmpac);
      break;
  case NCBI:
      if(!ajSeqParseNcbi(rline,id,&tmpac,&token))
      {
	  (void) ajStrDelReuse(&tmpac);
	  return ajFalse;
      }
      (void) ajStrToUpper(id);

      if(!ajStrLen(tmpac))
	  (void) ajFmtPrintS(&tmpac,"**%d",num++);


      if (ajIsAccession(tmpac))
      {
	  (void) ajStrToUpper(&tmpac);
	  if(systemsort)
	      ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
	  else
	  {
	      ac = ajCharNew(tmpac);
	      ajListPushApp(acl,ac);
	  }
      }
      else
      {
	  ajStrAss(&tmpac,*id);
	  if(systemsort)
	      ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
	  else
	  {
	      ac = ajCharNew(tmpac);
	      ajListPushApp(acl,ac);
	  }
      }
      break;
  case GCGIDACC:
      ajRegSubI(exp,1,id);
      (void) ajStrToUpper(id);
      ajRegSubI(exp,2,&tmpac);
      (void) ajStrToUpper(&tmpac);
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
	  ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      break;
  case GCGACCID:
      ajRegSubI(exp,1,&tmpac);
      (void) ajStrToUpper(&tmpac);
      ajRegSubI(exp,2,id);
      (void) ajStrToUpper(id);
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
	  ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      break;
  case IDACC:
      ajRegSubI(exp,1,id);
      (void) ajStrToUpper(id);
      ajRegSubI(exp,2,&tmpac);
      (void) ajStrToUpper(&tmpac);
      
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
	  ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      break;
  case ACCID:
      ajRegSubI(exp,1,&tmpac);
      (void) ajStrToUpper(&tmpac);
      ajRegSubI(exp,2,id);
      (void) ajStrToUpper(id);
      
      if(systemsort)
	  ajFmtPrintF(alistfile,"%S %S\n",*id,tmpac);
      else
      {
	  ac = ajCharNew(tmpac);
	  ajListPushApp(acl,ac);
      }
      break;
  default:
      (void) ajStrDelReuse(&tmpac);
      return ajFalse;
      break;
  }

  ipos = ajFileTell(libr);

  while (ajFileGets (libr, &rline)) {
    if (ajStrChar(rline,0) == '>') {
      ajFileSeek(libr, ipos, 0);
      return ajTrue;
    }
    ipos = ajFileTell(libr);
  }

  ajFileSeek(libr, ipos, 0);	/* end of file reached */
  return ajTrue;
}

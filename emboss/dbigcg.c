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

#define BIGOVERLAP 10000;

AjPList idlist;
AjPList aclist;

AjBool systemsort;
AjBool cleanup;

static AjPStr gcgtype = NULL;
static ajint gcglen;

static ajint maxidlen = 12;
static ajint maxaclen = 12;

static AjPStr rline = NULL;
static AjPStr sline = NULL;

static ajint rpos;
static ajint spos;

static AjPRegexp rexp = NULL;
static AjPRegexp pirexp = NULL;
static AjPRegexp sexp = NULL;
static AjPStr lastidstr = NULL;

static AjPFile elistfile=NULL;
static AjPFile alistfile=NULL;
static AjPFile blistfile=NULL;

static AjPStr dbname = NULL;
static AjPStr release = NULL;
static AjPStr datestr = NULL;
static AjPStr sortopt = NULL;

static EmbPentry nextentry (AjPFile libr, AjPFile libs, ajint ifile);
static AjBool gcgopenlib(AjPStr lname, AjPFile* libr, AjPFile* libs);
static ajint gcggetent(AjPFile libr, AjPFile libs,
		     ajint *d_pos, ajint *s_pos, AjPStr* libstr, AjPList alc);
static ajint pirgetent(AjPFile libr, AjPFile libs,
		     ajint *d_pos, ajint *s_pos, AjPStr* libstr, AjPList alc);
static ajint gcgappent( AjPFile libr, AjPFile libs,
		     AjPStr* libstr);

static AjPStr idformat = NULL;

static AjBool dbigcgParseEmbl (AjPStr line, AjPStr* id, AjPList acl);
static AjBool dbigcgParsePir (AjPStr line, AjPStr* id, AjPList acl);
static AjBool dbigcgParseGenbank (AjPStr line, AjPStr* id, AjPList acl);

typedef struct SParser {
  char* Name;
  AjBool (*Parser) (AjPStr line, AjPStr* id, AjPList acl);
} OParser;

static OParser parser[] = {
  {"EMBL", dbigcgParseEmbl},
  {"SWISS", dbigcgParseEmbl},
  {"GENBANK", dbigcgParseGenbank},
  {"PIR", dbigcgParsePir},
  {NULL, NULL}
};


int main(int argc, char **argv)
{

  AjBool staden;
  AjPStr directory;
  AjPStr indexdir;
  AjPStr filename;
  AjPStr curfilename = NULL;
  AjPStr elistfname = NULL;
  AjPStr alistfname = NULL;
  AjPStr blistfname = NULL;

  AjPFile libr=NULL;
  AjPFile libs=NULL;

  EmbPentry entry;
  EmbPac acnum=NULL;
  char* lastac=NULL;

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

  AjPStr rdline = NULL;
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

  AjPStr* reffiles = NULL;
  AjPStr* seqfiles = NULL;
  AjPStr* twofiles = NULL;

  datexp = ajRegCompC("^([0-9]+).([0-9]+).([0-9]+)");
  idsrtexp = ajRegCompC ("^([^ ]+) +([0-9]+) +([0-9]+) +([0-9]+)");
  acsrtexp = ajRegCompC ("^([^ ]+) +([^ \n]+)");
  acsrt2exp = ajRegCompC ("^([^ ]+) +([0-9]+)");

  for (i=0;i<256;i++)
    padding[i] = ' ';

  embInit ("dbigcg", argc, argv);

  staden = ajAcdGetBool ("staden");
  idformat = ajAcdGetListI ("idformat",1);
  directory = ajAcdGetString ("directory");
  indexdir = ajAcdGetString ("indexdirectory");
  filename = ajAcdGetString ("filename");
  dbname = ajAcdGetString ("dbname");
  release = ajAcdGetString ("release");
  datestr = ajAcdGetString ("date");
  systemsort = ajAcdGetBool ("systemsort");
  cleanup = ajAcdGetBool ("cleanup");
  sortopt = ajAcdGetString ("sortoptions");

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

  inlist = embDbiFileList (directory, filename, NULL);
  ajListSort (inlist, ajStrCmp);
  nfiles = ajListToArray(inlist, &files);

  AJCNEW0(reffiles, nfiles);
  AJCNEW0(seqfiles, nfiles);
  AJCNEW0(twofiles, nfiles);

  if (systemsort)
    acnum = embDbiAcnumNew();

  for (ifile=0; ifile<nfiles; ifile++) {
    curfilename = (AjPStr) files[ifile];
    gcgopenlib (curfilename, &libr, &libs);
    ajDebug ("processing '%S' ...\n", curfilename);
    ajDebug ("processing '%F' ...\n", libr);
    ajDebug ("processing '%F' ...\n", libs);
    ajFmtPrintS(&reffiles[ifile], "%F", libr);
    ajFileNameTrim(&reffiles[ifile]);
    ajFmtPrintS(&seqfiles[ifile], "%F", libs);
    ajFileNameTrim(&seqfiles[ifile]);
    ajDebug ("processing '%S' ...\n", reffiles[ifile]);
    ajDebug ("processing '%S' ...\n", seqfiles[ifile]);
    if (systemsort) {
      ajFmtPrintS (&elistfname, "%S%02d.list", dbname, ifile+1);
      elistfile = ajFileNewOut (elistfname);
      if (!elistfile)
	ajFatal("Failed to open %S for writing", elistfname);
      ajDebug ("elistfile %F\n", elistfile);
      ajFmtPrintS (&alistfname, "%S%02d.acid", dbname, ifile+1);
      alistfile = ajFileNewOut (alistfname);
      if (!alistfile)
	ajFatal("Failed to open %S for writing", alistfname);
      ajDebug ("alistfile %F\n", alistfile);
    }
    ajFmtPrintS(&twofiles[ifile], "%S %S",
		reffiles[ifile], seqfiles[ifile]);
    if (ajStrLen(twofiles[ifile]) >= maxfilelen)
      maxfilelen = ajStrLen(twofiles[ifile]) +1;
    while ((entry=nextentry(libr, libs, ifile))) {
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
	ajFatal("Failed to open %S for reading",alistfname);
    ajFmtPrintS (&blistfname, "%S.acid2", dbname);
    blistfile = ajFileNewOut (blistfname);
    if (!blistfile)
      ajFatal("Failed to open %S for reading", blistfname);

    ient=0;
    nac = 0;
    while (ajFileGets (alistfile, &rdline)) {
      ajRegExec (acsrtexp, rdline);
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
	ajDebug("ids[%d] '%S' < acd[%d] '%S'\n",
		i, ((EmbPentry)ids[i])->entry,
		j, ((EmbPac)acs[j])->entry);
	i++;
      }
      else if (k > 0) {
	ajDebug("ids[%d] '%S' >> acd[%d] '%S'\n",
		i, ((EmbPentry)ids[i])->entry,
		j, ((EmbPac)acs[j])->entry);
	j++;
      }
      else {
	ajDebug("ids[%d] '%S' == acd[%d] '%S'\n",
		i, ((EmbPentry)ids[i])->entry,
		j, ((EmbPac)acs[j])->entry);
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
  if (!dfile)
    ajFatal("Failed to open %S for writing", dfname);

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
    ajFileWriteStr (dfile, twofiles[i], maxfilelen);
  }
  ajFileClose (&dfile);

  /* write the entry file */

  ajStrAssC (&efname, "entrynam.idx");
  efile = ajFileNewOutD(indexdir, efname);
  if (!efile)
    ajFatal("Failed to open %S for writing", efname);

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
    ajFmtPrintS (&elistfname, "%S.idsrt", dbname);
    elistfile = ajFileNewIn (elistfname);
    if(!elistfile)
	ajFatal("Failed to open %S for reading",elistfname);
    while (ajFileGets (elistfile, &rdline)) {
      ajRegExec (idsrtexp, rdline);
      ajRegSubI (idsrtexp, 1, &idstr);
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
  if (!atfile)
    ajFatal("Failed to open %S for writing", atfname);
  ajStrAssC (&ahfname, "acnum.hit");
  ahfile = ajFileNewOutD(indexdir, ahfname);
  if (!ahfile)
    ajFatal("Failed to open %S for writing", ahfname);

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
	ajFatal("Failed to open %S for reading",alistfname);
    while (ajFileGets (alistfile, &rdline)) {
      ajRegExec (acsrt2exp, rdline);
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
	ajStrAssS(&lastidstr, idstr);
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

/* @funcstatic nextentry ********************************************
**
** Returns next database entry as an EmbPentry object
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [r] ifile [ajint] File number.
** @return [EmbPentry] Entry data object.
** @@
******************************************************************************/

static EmbPentry nextentry (AjPFile libr, AjPFile libs, ajint ifile) {

  static EmbPentry ret=NULL;
  ajint ir;
  ajint is;
  static AjPStr id = NULL;
  static AjPStr tmpline2 = NULL;
  char* ac;
  char *p;
  ajint i;
  static AjPList acl = NULL;

  if (!acl)
    acl = ajListNew();

  if (!ret || !systemsort)
    ret = embDbiEntryNew();

  if (!gcggetent (libr, libs, &ir, &is, &id, acl) &&
      !pirgetent (libr, libs, &ir, &is, &id, acl))
    return NULL;

  /* id to ret->entry */

  if (systemsort) {
      ajStrAssC(&tmpline2,ajStrStr(id));
      if(ajStrSuffixC(id,"_0") || ajStrSuffixC(id,"_00"))
      {
	  p = strrchr(ajStrStr(tmpline2),'_');
	  *p = '\0';
      }
    ajFmtPrintF (elistfile, "%s %d %d %d\n", ajStrStr(tmpline2),
		 ir, is, ifile+1);
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

/* @funcstatic gcgopenlib ********************************************
**
** Open a GCG library
**
** @param [r] lname [AjPStr] Source file basename
** @param [r] libr [AjPFile*] Reference file
** @param [r] libs [AjPFile*] Sequence file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool gcgopenlib(AjPStr lname, AjPFile* libr, AjPFile* libs) {

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

  if (!*libr) {
    ajErr(" cannot open GCG library: %S\n",
	    rname);
    return ajFalse;
  }
  
  *libs = ajFileNewIn(sname);
  if(!libs)
      ajFatal("Failed to open %S for reading",sname);
  if (!*libs) {
    ajErr(" cannot open GCG library sequence file: %S\n",
	    sname);
    return ajFalse;
  }
  
  rpos = ajFileTell(*libr);
  spos = ajFileTell(*libs);
  if (!ajFileGets(*libr, &rline)) return ajFalse;
  if (!ajFileGets(*libs, &sline)) return ajFalse;

  return ajTrue;
}

/* @funcstatic gcggetent ******************************************************
**
** get a single entry from the GCG database files
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [w] d_pos [ajint*] Reference file offset returned
** @param [w] s_pos [ajint*] Sequence  file offset returned
** @param [w] libstr [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list
** @return [ajint] Sequence length
** @@
******************************************************************************/

static ajint gcggetent(AjPFile libr, AjPFile libs,
		     ajint *d_pos, ajint *s_pos, AjPStr* libstr, AjPList acl) {

  static AjPStr gcgdate = NULL;
  ajint rblock;
  /*  ajint ddone = 0;*/
  static AjPStr reflibstr = NULL;
  /*  ajint iac;*/
  ajint i;
  static AjPStr tmpstr = NULL;
  static ajint called = 0;
  static ajint iparser = -1;

  if (!called) {
    for (i=0; parser[i].Name; i++) {
      if (ajStrMatchC (idformat, parser[i].Name)) {
	iparser = i;
	break;
      }
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

  /* check for seqid line */
  while (ajStrChar(sline,0)!='>') {
    spos = ajFileTell(libs);
    if (!ajFileGets(libs, &sline)) {
      *s_pos = spos;		/* end of file */
      return 0;
    }
  }
  *s_pos = spos;
  /* get the encoding/sequence length info */

  if (!ajRegExec(sexp, sline))
    return 0;

  ajRegSubI(sexp, 1, libstr);

  


  ajRegSubI(sexp, 2, &gcgdate);
  ajRegSubI(sexp, 3, &gcgtype);
  ajRegSubI(sexp, 5, &tmpstr);
  ajStrToInt (tmpstr, &gcglen);

  /* check for refid line */
  while (ajStrChar(rline,0)!='>') {
    rpos = ajFileTell(libr);
    if (!ajFileGets(libr, &rline)) {
      *d_pos = rpos;		/* end of file */
      ajErr("ref ended before seq");
      break;
    }
  }
  *d_pos = rpos;

  /* get the encoding/sequence length info */

  ajRegExec (rexp, rline);
  ajRegSubI(rexp, 1, &reflibstr);

  /*
  if (!ajStrMatch(*libstr, reflibstr))
    ajDebug ("refid: '%S' seqid: '%S'\n", reflibstr, *libstr);
  */

  /*  iac = 0;*/
  ajFileGets(libr, &rline);
  while (ajStrChar(rline,0)!='>') {
      ajStrAssS(&tmpstr,*libstr);
      
    parser[iparser].Parser(rline, libstr, acl); /* writes alistfile data */

      ajStrAssS(libstr,tmpstr);
      
    rpos = ajFileTell(libr);
    if (!ajFileGets(libr, &rline)) {
      /*      ddone = 1;*/
      break;
    }
  }

  /*
  if (ajStrMatch(*libstr, reflibstr))
    ajDebug ("refid '%S' seqid '%S'\n", reflibstr, *libstr);
  */

  /* get the description line */
  ajFileGets(libs, &sline);

  /* seek to the end of the sequence; +1 to jump over newline */
  if (ajStrChar(gcgtype,0)=='2') {
    rblock = (gcglen+3)/4;
    ajFileSeek(libs,rblock+1,SEEK_CUR);
  }
  else ajFileSeek(libs,gcglen+1,SEEK_CUR);

  spos = ajFileTell(libs);
  ajFileGets(libs, &sline);

  /* for big entries, need to append until we have all the parts.
     They are named with _0 on the first part, _1 on the second and so on.
     We can look for the "id_" prefix.
  */

  i = ajStrLen(*libstr);
  if (!ajStrSuffixC(*libstr, "_0") && !ajStrSuffixC(*libstr,"_00"))
      return gcglen;
  
  gcglen += gcgappent (libr, libs, libstr);

  return gcglen;
}

/* @funcstatic pirgetent ******************************************************
**
** get a single entry from the PIR database files
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [w] d_pos [ajint*] Reference file offset returned
** @param [w] s_pos [ajint*] Sequence file offset  returned
** @param [w] libstr [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list
** @return [ajint] Sequence length
** @@
******************************************************************************/

static ajint pirgetent(AjPFile libr, AjPFile libs,
		     ajint *d_pos, ajint *s_pos, AjPStr* libstr, AjPList acl) {

  static AjPStr reflibstr = NULL;
  ajint i;
  static ajint called = 0;
  static ajint iparser = -1;

  if (!called) {
    for (i=0; parser[i].Name; i++) {
      if (ajStrMatchC (idformat, parser[i].Name)) {
	iparser = i;
	break;
      }
    }
    if (iparser < 0)
      ajFatal ("idformat '%S' unknown", idformat);
    ajDebug ("idformat '%S' Parser %d\n", idformat, iparser);
    called = 1;
  }

  if (!pirexp)
    pirexp = ajRegCompC ("^>..;([^ \t\n]+)");

  /* skip to seqid line */
  while (ajStrChar(sline,0)!='>') {
    spos = ajFileTell(libs);
    if (!ajFileGets(libs, &sline)) {
      *s_pos = spos;		/* end of file */
      return 0;
    }
  }
  *s_pos = spos;
  /* get the encoding/sequence length info */

  ajDebug ("pirgetent '%S' \n", sline);
  ajDebug ("pirgetent '%S' spos: %ld\n", *libstr, spos);

  ajRegExec(pirexp, sline);

  /* skip to refid line */
  while (ajStrChar(rline,0)!='>') {
    rpos = ajFileTell(libr);
    if (!ajFileGets(libr, &rline)) {
      *d_pos = rpos;		/* end of file */
      ajErr("ref ended before seq");
      break;
    }
  }
  *d_pos = rpos;

  /* get the encoding/sequence length info */

  ajRegExec (pirexp, rline);
  ajRegSubI(pirexp, 1, &reflibstr);
  ajRegSubI(pirexp, 1, libstr);

  /*
  if (!ajStrMatch(*libstr, reflibstr))
    ajDebug ("refid: '%S' seqid: '%S'\n", reflibstr, *libstr);
  */

  /*  iac = 0;*/
  ajFileGets(libr, &rline);
  while (ajStrChar(rline,0)!='>') {

    parser[iparser].Parser(rline, libstr, acl); /* writes alistfile data */

    rpos = ajFileTell(libr);
    if (!ajFileGets(libr, &rline)) {
      /*      ddone = 1;*/
      break;
    }
  }

  /*
  if (ajStrMatch(*libstr, reflibstr))
    ajDebug ("refid '%S' seqid '%S'\n", reflibstr, *libstr);
  */

  /* get the description line */
  ajFileGets(libs, &sline);
  gcglen = 0;

  /* seek to the end of the sequence; +1 to jump over newline */
  while (ajStrChar(sline,0)!='>') {
    spos = ajFileTell(libs);
    if (!ajFileGets(libs, &sline)) {
      break;
    }
    gcglen += ajStrLen(sline);
  }


  ajDebug ("pirgetent end spos %ld line '%S'\n", spos, sline);

  return gcglen;
}

/* @funcstatic gcgappent ******************************************************
**
** Go to end of a split GCG entry
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [w] libstr [AjPStr*] ID
** @return [ajint] Sequence length for this section
** @@
******************************************************************************/

static ajint gcgappent (AjPFile libr, AjPFile libs, AjPStr* libstr) {

  /* keep reading until we reach the end of entry
   and return the extra number of bases*/

  static AjPStr reflibstr = NULL;
  static AjPStr seqlibstr = NULL;
  static AjPStr testlibstr = NULL;
  ajint ilen;
  static AjPStr tmpstr = NULL;

  AjBool isend;
  char *p;
  char *q;
  
  if(!testlibstr)
      testlibstr = ajStrNew();

  ajStrAssS(&tmpstr,*libstr);
  

  p = ajStrStr(tmpstr);
  q = strrchr(p,'_');
  *q='\0';
  

  ajFmtPrintS(&testlibstr, "%s_",p);
  ilen = ajStrLen(testlibstr);

  isend = ajFalse;
  
  while(!isend)
  {
      ajFileGets(libs,&sline);
/*      while (ajStrChar(sline,0)!='>')*/
      while (strncmp(ajStrStr(sline),">>>>",4))
      {
	  if (!ajFileGets(libs, &sline))
	      return 1;
      }
      
      ajRegExec (sexp, sline);
      ajRegSubI(sexp, 1, &seqlibstr);

      ajFileGets(libr, &rline);
      
      while (ajStrChar(rline,0)!='>')
      {
	  if (!ajFileGets(libr, &rline))
	  {
	      ajErr ("ref ended before seq\n");
	      break;
	  }
      }
      ajRegExec (rexp, rline);
      ajRegSubI (rexp, 1, &reflibstr);

      if (ajStrNCmpO(reflibstr, testlibstr, ilen) ||
	  ajStrNCmpO(seqlibstr, testlibstr, ilen))
	  isend = ajTrue;
  }
  


  ajStrAssC(libstr,p);
  
  
  return 1;
}
  



/* @funcstatic dbigcgParseEmbl ********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbigcgParseEmbl (AjPStr line, AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPRegexp acexp = NULL;
  static AjPRegexp ac2exp = NULL;
  static AjPStr tmpline = NULL;
  static AjPStr tmpline2 = NULL;
  static AjPStr tmpac = NULL;
  char* ac;
  char *p;
  
  if (!idexp)
    idexp = ajRegCompC ("^ID   ([^ \t]+)");

  if (!acexp)
    acexp = ajRegCompC ("^AC   ");

  if (!ac2exp)
    ac2exp = ajRegCompC ("([A-Za-z0-9]+)");

  if (ajRegExec (idexp, line)) {
    ajRegSubI (idexp, 1, id);
    return ajTrue;
  }

  if (ajRegExec (acexp, line)) {
    ajRegPost (acexp, &tmpline);
    while (ajRegExec(ac2exp, tmpline)) {
      ajRegSubI (ac2exp, 1, &tmpac);

      ajStrAssC(&tmpline2,ajStrStr(*id));
      if(ajStrSuffixC(*id,"_0") || ajStrSuffixC(*id,"_00"))
      {
	  p = strrchr(ajStrStr(tmpline2),'_');
	  *p = '\0';
      }
      
      if (systemsort) {
	ajFmtPrintF (alistfile, "%s %S\n", ajStrStr(tmpline2), tmpac);
      }
      else {
	ac = ajCharNew(tmpac);
	ajListPushApp (acl, ac);
      }
      ajRegPost (ac2exp, &tmpline);
    }
    return ajTrue;
  }
  return ajFalse;
}


/* @funcstatic dbigcgParseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbigcgParseGenbank (AjPStr line, AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPRegexp acexp = NULL;
  static AjPRegexp ac2exp = NULL;
  static AjPStr tmpline = NULL;
  static AjPStr tmpac = NULL;
  char* ac;

  if (!idexp)
    idexp = ajRegCompC ("^ID   ([^ \t]+)");

  if (!acexp)
    acexp = ajRegCompC ("^ACCESSION   ");

  if (!ac2exp)
    ac2exp = ajRegCompC ("([A-Za-z0-9]+)");

  if (ajRegExec (idexp, line)) {
    ajRegSubI (idexp, 1, id);
    return ajTrue;
  }

  if (ajRegExec (acexp, line)) {
    ajRegPost (acexp, &tmpline);
    while (ajRegExec(ac2exp, tmpline)) {
      ajRegSubI (ac2exp, 1, &tmpac);
      if (systemsort) {
	ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
      }
      else {
	ac = ajCharNew(tmpac);
	ajListPushApp (acl, ac);
      }
      ajRegPost (ac2exp, &tmpline);
    }
    return ajTrue;
  }
  return ajFalse;
}


/* @funcstatic dbigcgParsePir ********************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool dbigcgParsePir (AjPStr line, AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPRegexp acexp = NULL;
  static AjPRegexp ac2exp = NULL;
  static AjPStr tmpline = NULL;
  static AjPStr tmpac = NULL;
  char* ac;

  if (!idexp)
    idexp = ajRegCompC ("^ID   ([^ \t]+)");

  if (!acexp)
    acexp = ajRegCompC ("^C;Accession:");

  if (!ac2exp)
    ac2exp = ajRegCompC ("([A-Za-z0-9]+)");

  if (ajRegExec (idexp, line)) {
    ajRegSubI (idexp, 1, id);
    return ajTrue;
  }

  if (ajRegExec (acexp, line)) {
    ajRegPost (acexp, &tmpline);
    while (ajRegExec(ac2exp, tmpline)) {
      ajRegSubI (ac2exp, 1, &tmpac);
      if (systemsort) {
	ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
      }
      else {
	ac = ajCharNew(tmpac);
	ajListPushApp (acl, ac);
      }
      ajRegPost (ac2exp, &tmpline);
    }
    return ajTrue;
  }
  return ajFalse;
}

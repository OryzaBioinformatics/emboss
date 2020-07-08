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
#include <dirent.h>
#include <sys/wait.h>

#define BIGOVERLAP 10000;

static AjBool doReverse = AJFALSE;

AjPList idlist;
AjPList aclist;

AjBool systemsort;
AjBool cleanup;

typedef struct Sac {
  char* ac;
  char* entry;
  int nid;			/* entry number */
} Oac, *Pac;

typedef struct Sentry {
  int nac;
  int rpos;
  int spos;
  int filenum;
  char* entry;
  char** ac;
} Oentry, *Pentry;


static int maxidlen = 12;
static int maxaclen = 12;

static AjPStr rline = NULL;
static AjPStr idformat = NULL;

static AjPStr lastidstr = NULL;

static AjPFile elistfile=NULL;
static AjPFile alistfile=NULL;
static AjPFile blistfile=NULL;

static AjPStr dbname = NULL;
static AjPStr release = NULL;
static AjPStr datestr = NULL;
static AjPStr sortopt = NULL;

static AjBool parseEmbl    (AjPFile libr, int *dpos,
			    AjPStr* id, AjPList acl);
static AjBool parseGenbank (AjPFile libr, int *dpos,
			    AjPStr* id, AjPList acl);

typedef struct SParser {
  char* Name;
  AjBool (*Parser) (AjPFile libr, int *dpos, AjPStr* id, AjPList acl);
} OParser;

static OParser parser[] = {
  {"EMBL", parseEmbl},
  {"SWISS", parseEmbl},
  {"GB", parseGenbank},
  {NULL, NULL}
};

static Pac    acnumNew (void);
static Pentry entryNew (void);
static Pentry nextflatentry (AjPFile libr, int ifile);
static AjBool flatopenlib(AjPStr lname, AjPFile* libr);

static char* newcharS (AjPStr str);
static char* newcharCI (char* str, int i);

static int cmpid (const void* a, const void* b);
static int cmpacid (const void* a, const void* b);
static int cmpacac (const void* a, const void* b);
static AjPList fileList (AjPStr dir, AjPStr wildfile, AjPStr exclude);

static int  writeInt2 (short i, AjPFile file);
static int  writeInt4 (int i, AjPFile file);
static int writeStr (AjPStr str, int len, AjPFile file);
static int writeChar (char* str, int len, AjPFile file);
static int writeByte (char ch, AjPFile file);

static void syscmd (AjPStr cmdstr);
static void sortfile (const char* ext1, const char* ext2, int nfiles);
static void rmfile (const char* ext, int nfiles);
static void rmfileI (const char* ext, int ifile);

int main (int argc, char * argv[]) {

  AjBool staden;
  AjPStr directory;
  AjPStr indexdir;
  AjPStr filename;
  AjPStr exclude;
  AjPStr curfilename = NULL;
  AjPStr elistfname = NULL;
  AjPStr alistfname = NULL;
  AjPStr blistfname = NULL;

  AjPFile libr=NULL;

  Pentry entry;
  Pac acnum=NULL;
  char* lastac=NULL;

  int i;
  int j;
  int k;
  int nac;
  int nid=0;
  int iac=0;
  void **ids = NULL;
  void **acs = NULL;
  AjPList inlist = NULL;
  void ** files = NULL;
  int nfiles;
  int ifile;

  int dsize;
  int esize;
  int asize;
  int ahsize;
  short recsize;
  short elen;
  short alen;
  int maxfilelen=20;
  char date[4] = {0,0,0,0};
  char padding[256];
  int ient;
  int filenum;
  int rpos;
  int spos;
  int idnum;

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

  datexp = ajRegCompC("^([0-9]+).([0-9]+).([0-9]+)");
  idsrtexp = ajRegCompC ("^([^ ]+) +([0-9]+) +([0-9]+) +([0-9]+)");
  acsrtexp = ajRegCompC ("^([^ ]+) +([^ \n]+)");
  acsrt2exp = ajRegCompC ("^([^ ]+) +([0-9]+)");

  for (i=0;i<256;i++)
    padding[i] = ' ';

  embInit ("dbiflat", argc, argv);

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

  inlist = fileList (directory, filename, exclude);
  ajListSort (inlist, ajStrCmp);
  nfiles = ajListToArray(inlist, &files);

  if (!nfiles)
    ajFatal ("No files selected");

  AJCNEW0(divfiles, nfiles);

  if (systemsort)
    acnum = acnumNew();

  for (ifile=0; ifile<nfiles; ifile++) {
    curfilename = (AjPStr) files[ifile];
    flatopenlib (curfilename, &libr);
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
    while ((entry=nextflatentry(libr, ifile))) {
      if (systemsort) {
	nid++;
      }
      else {
	entry->filenum = ifile+1;
	ajListPushApp (idlist, entry);
	for (i=0;i<entry->nac; i++) {
	  if (!systemsort)
	    acnum = acnumNew();
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
    sortfile ("list", "idsrt", nfiles);
    sortfile ("acid", "acsrt", nfiles);

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

    rmfile ("acsrt", 0);
    sortfile ("acid2", "acsrt2", 0);
 
  }
    
  else {
    nid = ajListToArray (idlist, &ids);
    nac = ajListToArray (aclist, &acs);

    ajDebug ("ids: %d %x acs: %d %x\n", nid, ids, nac, acs);
    /*
      for (i=0; i<nid; i++) {
      ajDebug("ids %3d %x %x '%s'\n",
      i, &ids[i], ids[i], ((Pentry)ids[i])->entry);
      }
    */
    qsort (ids, nid, sizeof(void*), cmpid);
    ajDebug ("ids sorted\n");

    /*
      for (i=0; i<nid; i++) {
      ajDebug("sort ids %3d %x %x '%s'\n",
      i, &ids[i], ids[i], ((Pentry)ids[i])->entry);
      }
    */

    qsort (acs, nac, sizeof(void*), cmpacid);
    ajDebug ("acs sorted by id\n");
    /*
      for (i=0; i<nac; i++) {
      ajDebug("sort acs %3d %x %x '%s' '%s'\n",
      i, &acs[i], acs[i], ((Pac)acs[i])->entry,((Pac)acs[i])->ac);
      }
    */
    i=0;
    j=0;

    while (ids[i] && acs[j]) {
      k = strcmp(((Pentry)ids[i])->entry, ((Pac)acs[j])->entry);
      if (k < 0) {
	i++;
      }
      else if (k > 0) {
	j++;
      }

      else {
	((Pac)acs[j++])->nid = i+1;	/* we need (i+1) */
      }
    }
    ajDebug ("checked ids: %d %d acs: %d %d\n", i, nid, j, nac);

    qsort (acs, nac, sizeof(void*), cmpacac);
    ajDebug ("acs sorted by ac\n");
    /*
      for (i=0; i<nac; i++) {
      ajDebug("sort acs %3d %x %x '%s' '%s' %d\n",
      i, &acs[i], acs[i], ((Pac)acs[i])->entry,((Pac)acs[i])->ac,
      ((Pac)acs[i])->nid);
      }
    */

  }

  if (ajUtilBigendian())
    doReverse = ajTrue;
  else
    doReverse = ajFalse;

  /* write the division file */

  ajStrAssC (&dfname, "division.lkp");
  dfile = ajFileNewOutD(indexdir, dfname);
  if(!dfile)
      ajFatal("Cannot open %S for writing",dfname);

  dsize = 256 + 44 + (nfiles * (maxfilelen+2));
  writeInt4 (dsize, dfile); /* filesize */

  writeInt4 (nfiles, dfile); /* #records */

  recsize = maxfilelen + 2;
  writeInt2 (recsize, dfile); /* recordsize */

  /* rest of the header */
  writeStr  (dbname,  20, dfile); /* dbname */
  writeStr  (release, 10, dfile); /* release */
  writeByte (date[0], dfile); /* release date */
  writeByte (date[1], dfile); /* release date */
  writeByte (date[2], dfile); /* release date */
  writeByte (date[3], dfile); /* release date */
  ajFileWrite (padding, 256, 1, dfile); /* padding 256 bytes */

  for (i=0; i<nfiles; i++) {
    writeInt2 ((short)(i+1), dfile);
    writeStr (divfiles[i], maxfilelen, dfile);
  }
  ajFileClose (&dfile);

  /* write the entry file */

  ajStrAssC (&efname, "entrynam.idx");
  efile = ajFileNewOutD(indexdir, efname);
  if(!efile)
      ajFatal("Cannot open %S for writing",efname);

  ajDebug("writing entrynam.idx %d\n", nid);

  elen = maxidlen+10;
  esize = 300 + (nid*(int)elen);
  writeInt4 (esize, efile);
  writeInt4 (nid, efile);
  writeInt2 (elen, efile);

  /* rest of the header */
  writeStr  (dbname,  20, efile); /* dbname */
  writeStr  (release, 10, efile); /* release */
  writeByte (date[0], efile); /* release date */
  writeByte (date[1], efile); /* release date */
  writeByte (date[2], efile); /* release date */
  writeByte (date[3], efile); /* release date */
  ajFileWrite (padding, 256, 1, efile); /* padding 256 bytes */

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
      writeStr (idstr, maxidlen, efile);
      writeInt4 (rpos, efile);
      writeInt4 (spos, efile);
      writeInt2 (filenum, efile);
      ajStrAss  (&lastidstr, idstr);
    }
    ajFileClose (&elistfile);
    rmfile ("idsrt", 0);
  }
  else {
    for (i = 0; i < nid; i++) {
      entry = (Pentry)ids[i];
      writeChar (entry->entry, maxidlen, efile);
      writeInt4 (entry->rpos, efile);
      writeInt4 (entry->spos, efile);
      writeInt2 (entry->filenum, efile);
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
    lastac = ((Pac)acs[0])->ac;

  ajDebug("writing acnum.hit %d\n", nac);

  alen = maxaclen+8;
  asize = 300 + (nac*(int)alen); /* to be fixed later */
  writeInt4 (asize, atfile);
  writeInt4 (nac, atfile);
  writeInt2 (alen, atfile);

  /* rest of the header */
  writeStr  (dbname,  20, atfile); /* dbname */
  writeStr  (release, 10, atfile); /* release */
  writeByte (date[0], atfile); /* release date */
  writeByte (date[1], atfile); /* release date */
  writeByte (date[2], atfile); /* release date */
  writeByte (date[3], atfile); /* release date */
  ajFileWrite (padding, 256, 1, atfile); /* padding 256 bytes */

  ahsize = 300 + (nac*4);
  writeInt4 (ahsize, ahfile);
  writeInt4 (nac, ahfile);
  writeInt2 (4, ahfile);

  /* rest of the header */
  writeStr  (dbname,  20, ahfile); /* dbname */
  writeStr  (release, 10, ahfile); /* release */
  writeByte (date[0], ahfile); /* release date */
  writeByte (date[1], ahfile); /* release date */
  writeByte (date[2], ahfile); /* release date */
  writeByte (date[3], ahfile); /* release date */
  ajFileWrite (padding, 256, 1, ahfile); /* padding 256 bytes */

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
      writeInt4 (idnum, ahfile);
      if (!i)
	ajStrAssS (&lastidstr, idstr);
      if (!ajStrMatch(lastidstr, idstr)) {
	writeInt4 (j, atfile);
	writeInt4 (k, atfile);
	writeStr (lastidstr, maxaclen, atfile);
	j = 0;			/* number of hits */
	k = i+1;		/* first hit */
	ajStrAssS (&lastidstr, idstr);
	iac++;
      }
      j++;
      i++;
    }
    ajFileClose (&alistfile);
    rmfile ("acsrt2", 0);
    writeInt4 (j, atfile);
    writeInt4 (k, atfile);
    writeStr (lastidstr, maxaclen, atfile);
    iac++;
  }
  else {
    for (i = 0; i < nac; i++) {
      acnum = (Pac)acs[i];
      writeInt4 (acnum->nid, ahfile);
      if (strcmp(lastac, acnum->ac)) {
	writeInt4 (j, atfile);
	writeInt4 (k, atfile);
	writeChar (lastac, maxaclen, atfile);
	j = 0;
	k = i;
	lastac = acnum->ac;
	iac++;
      }
      j++;
    }
    writeInt4 (j, atfile);
    writeInt4 (k, atfile);
    writeChar (lastac, maxaclen, atfile);
    iac++;
  }

  ajDebug ("wrote acnum.trg %d\n", iac);
  ajFileSeek (atfile, 0, 0);	/* fix up the record count */
  writeInt4 (300+iac*(int)alen, atfile);
  writeInt4 (iac, atfile);

  ajFileClose (&atfile);
  ajFileClose (&ahfile);

  ajDebug ("finished...\n%7d files\n%7d entries\n%7d acnum.trg\n%7d acnum.hit\n",
	  nfiles, nid, iac, nac);
  ajExit ();
  return 0;
}

/* @funcstatic cmpid *******************************************************
**
** Comparison function for two entries.
**
** @param [r] a [const void*] First id (Pentry*)
** @param [r] b [const void*] Second id (Pentry*)
** @return [int] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

static int cmpid (const void* a, const void* b) {

  Pentry aa = *(Pentry*) a;
  Pentry bb = *(Pentry*) b;

  return strcmp(aa->entry, bb->entry);
}

/* @funcstatic cmpacid *******************************************************
**
** Comparison function for two accession entries.
**
** @param [r] a [const void*] First id (Pac*)
** @param [r] b [const void*] Second id (Pentryca*)
** @return [int] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

static int cmpacid (const void* a, const void* b) {
  Pac aa = *(Pac*) a;
  Pac bb = *(Pac*) b;

  return strcmp(aa->entry, bb->entry);
}

/* @funcstatic cmpacac *******************************************************
**
** Comparison function for two accession numbers.
**
** @param [r] a [const void*] First id (Pac*)
** @param [r] b [const void*] Second id (Pentryca*)
** @return [int] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

static int cmpacac (const void* a, const void* b) {
  Pac aa = *(Pac*) a;
  Pac bb = *(Pac*) b;

  return strcmp(aa->ac, bb->ac);
}


/******************************************************************************
**
** Step through, allocating one ID and multiple AC for each entry.
** Allocate more space for these as needed, using maxidlen and maxaclen.
** 
**
******************************************************************************/


/* @funcstatic acnumNew ********************************************
**
** Constructor for accession structures.
**
** @return [Pac] Accession structure.
******************************************************************************/

static Pac acnumNew (void) {

  Pac ret;
  AJNEW0 (ret);

  return ret;
}


/* @funcstatic entryNew ********************************************
**
** Constructor for entry structures.
**
** @return [Pentry] Entry structure.
******************************************************************************/

static Pentry entryNew (void) {

  Pentry ret;

  AJNEW0 (ret);

  return ret;
}

/* @funcstatic nextflatentry ********************************************
**
** Returns next database entry as a Pentry object
**
** @param [r] libr [AjPFile] Database file
** @param [r] ifile [int] File number.
** @return [Pentry] Entry data object.
** @@
******************************************************************************/

static Pentry nextflatentry (AjPFile libr, int ifile) {

  static Pentry ret=NULL;
  int ir;
  int is = 0;
  static AjPStr id = NULL;
  char* ac;
  int i;
  static AjPList acl = NULL;
  static int called = 0;
  static int iparser = -1;

  if (!called) {
    for (i=0; parser[i].Name; i++) {
      if (ajStrMatchC (idformat, parser[i].Name)) {
	iparser = i;
	break;
      }
    }
    if (iparser < 0)
      ajFatal ("idformat '%S' unknown", idformat);
  }

  if (!acl)
    acl = ajListNew();

  if (!ret || !systemsort)
    ret = entryNew();

  if (!parser[iparser].Parser (libr, &ir, &id, acl))
     return NULL;

  /* id to ret->entry */

  if (systemsort) {
    ajFmtPrintF (elistfile, "%S %d %d %d\n", id, ir, is, ifile+1);
  }
  else {
    ret->entry = newcharS(id);
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

/* @funcstatic newcharS ********************************************
**
** Constructor for a text string from an AjPStr
**
** @param [r] str [AjPStr] String object
** @return [char*] New text string.
******************************************************************************/

static char* newcharS (AjPStr str) {

  return newcharCI (ajStrStr(str), ajStrLen(str)+1);
}

/* @funcstatic newcharCI ********************************************
**
** Constructor for a text string from an AjPStr
**
** @param [r] str [char*] Text object
** @param [r] i [int] Length
** @return [char*] New text string.
******************************************************************************/

static char* newcharCI (char* str, int i) {

  static char* buffer = NULL;
  static int ipos=0;
  static int imax=0;

  char* ret;

  if ((ipos+i) > imax) {
    AJCNEW(buffer, 1000000);
    ajDebug ("newchar need more memory ipos: %d i: %d  imax: %d buffer: %x\n",
	     ipos, i, imax, buffer);
    imax = 1000000;
    ipos = 0;
  }
  ret = &buffer[ipos];
  strncpy (ret, str, i);
  ipos += i;
  return ret;
}


/* @funcstatic flatopenlib ********************************************
**
** Open a flat file library
**
** @param [r] lname [AjPStr] Source file basename
** @param [r] libr [AjPFile*] Database file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool flatopenlib(AjPStr lname, AjPFile* libr) {

  ajFileClose(libr);

  *libr = ajFileNewIn(lname);
  if(!*libr)
	ajFatal("Cannot open %S for reading",lname);
  if (!*libr) {
    ajErr(" cannot open library flat file: %S\n",
	    lname);
    return ajFalse;
  }
  
  return ajTrue;
}


/* @funcstatic parseEmbl ********************************************
**
** Parse the ID, accession from an EMBL entry.
**
** Reads to the end of the entry and then returns.
**
** @param [r] libr [AjPFile] Input database file
** @param [w] dpos [int*] Byte offset
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool parseEmbl (AjPFile libr, int* dpos,
			 AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPRegexp acexp = NULL;
  static AjPRegexp ac2exp = NULL;
  static AjPRegexp endexp = NULL;
  static AjPStr tmpline = NULL;
  static AjPStr tmpac = NULL;
  char* ac;

  if (!idexp)
    idexp = ajRegCompC ("^ID   ([^ \t]+)");

  if (!acexp)
    acexp = ajRegCompC ("^AC   ");

  if (!ac2exp)
    ac2exp = ajRegCompC ("([A-Za-z0-9]+)");

  if (!endexp)
    endexp = ajRegCompC ("^//");

  *dpos = ajFileTell(libr);

  while (ajFileGets (libr, &rline)) {
    if (ajRegExec (endexp, rline)) {
      return ajTrue;
    }
    if (ajRegExec (idexp, rline)) {
      ajRegSubI (idexp, 1, id);
      continue;
    }

    if (ajRegExec (acexp, rline)) {
      ajRegPost (acexp, &tmpline);
      while (ajRegExec(ac2exp, tmpline)) {
	ajRegSubI (ac2exp, 1, &tmpac);
	if (systemsort) {
	  ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
	}
	else {
	  ac = newcharS (tmpac);
	  ajListPushApp (acl, ac);
	}
	ajRegPost (ac2exp, &tmpline);
      }
      continue;
    }
  }
  return ajFalse;
}

/* @funcstatic parseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [r] libr [AjPFile] Input database file
** @param [w] dpos [int*] Byte offset
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool parseGenbank (AjPFile libr, int* dpos,
			    AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPRegexp acexp = NULL;
  static AjPRegexp ac2exp = NULL;
  static AjPRegexp endexp = NULL;
  static AjPStr tmpline = NULL;
  static AjPStr tmpac = NULL;
  char* ac;
  long ipos = 0;

  if (!idexp)
    idexp = ajRegCompC ("^LOCUS +([^ ]+)");

  if (!acexp)
    acexp = ajRegCompC ("^ACCESSION");

  if (!ac2exp)
    ac2exp = ajRegCompC ("([A-Za-z0-9]+)");

  if (!endexp)
    endexp = ajRegCompC ("^//");


  while (ajFileGets (libr, &rline)) {
    if (ajRegExec (endexp, rline)) {
      return ajTrue;
    }
    if (ajRegExec (idexp, rline)) {
      ajRegSubI (idexp, 1, id);
      *dpos = ipos;
    }

    else if (ajRegExec (acexp, rline)) {
      ajRegPost (acexp, &tmpline);
      while (ajRegExec(ac2exp, tmpline)) {
	ajRegSubI (ac2exp, 1, &tmpac);
	if (systemsort) {
	  ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
	}
	else {
	  ac = newcharS (tmpac);
	  ajListPushApp (acl, ac);
	}
	ajRegPost (ac2exp, &tmpline);
      }
      continue;
    }
    ipos = ajFileTell(libr);
  }
  return ajFalse;
}

/* @funcstatic fileList ********************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [AjPStr] Directory
** @param [r] wildfile [AjPStr] Wildcard file list
** @param [r] exclude [AjPStr] Wildcard file list
** @return [AjPList] New list of all files with full paths
** @@ 
******************************************************************************/

static AjPList fileList (AjPStr dir, AjPStr wildfile, AjPStr exclude) {

  AjPList retlist = NULL;

  DIR* dp;
  struct dirent* de;
  int dirsize;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;
  static AjPStr fname = NULL;

  if (ajStrLen(dir))
    (void) ajStrAss (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dirfix, -1) != '/')
    (void) ajStrAppC (&dirfix, "/");

  dp = opendir (ajStrStr(dirfix));
  if (!dp)
    ajFatal("opendir failed on '%S'", dirfix);

  dirsize = 0;
  retlist = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    ajStrAssC (&fname, de->d_name);
    if (!ajFileTestSkip(fname, exclude, wildfile, ajFalse)) continue;
    dirsize++;
    ajDebug ("accept '%S'\n", fname);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%S", dirfix, fname);
    ajListstrPushApp (retlist, name);
  }

  (void) closedir (dp);
  ajDebug ("%d files for '%S' '%S'\n", dirsize, dir, wildfile);

  return retlist;

}

/* @funcstatic writeInt2 ********************************************
**
** Writes a 2 byte integer to a binary file, with the correct byte orientation
**
** @param [r] i [short] Integer
** @param [r] file [AjPFile] Output file
** @return [int] Return value from fwrite
** @@
******************************************************************************/

static int  writeInt2 (short i, AjPFile file) {
  short j = i;
  if (doReverse)ajUtilRev2(&j);
    
  return fwrite (&j, 2, 1, ajFileFp(file));
}

/* @funcstatic writeInt4 ********************************************
**
** Writes a 4 byte integer to a binary file, with the correct byte orientation
**
** @param [r] i [int] Integer
** @param [r] file [AjPFile] Output file
** @return [int] Return value from fwrite
** @@
******************************************************************************/

static int  writeInt4 (int i, AjPFile file) {
  int j=i;
  if (doReverse)ajUtilRev4(&j);
  return fwrite (&j, 4, 1, ajFileFp(file));
}

/* @funcstatic writeStr ********************************************
**
** Writes a string to a binary file
**
** @param [r] str [AjPStr] String
** @param [r] len [int] Length (padded) to use in the file
** @param [r] file [AjPFile] Output file
** @return [int] Return value from fwrite
** @@
******************************************************************************/

static int writeStr (AjPStr str, int len, AjPFile file) {
  static char buf[256];
  int i = ajStrLen(str);
  strcpy(buf, ajStrStr(str));
  if (i < len)
    memset (&buf[i], '\0', len-i);

  return fwrite (buf, len, 1, ajFileFp(file));
}
/* @funcstatic writeChar ********************************************
**
** Writes a text string to a binary file
**
** @param [r] str [char*] Text string
** @param [r] len [int] Length (padded) to use in the file
** @param [r] file [AjPFile] Output file
** @return [int] Return value from fwrite
** @@
******************************************************************************/

static int writeChar (char* str, int len, AjPFile file) {
  static char buf[256];
  int i = strlen(str);
  strcpy(buf, str);
  if (i < len)
    memset (&buf[i], '\0', len-i);

  return fwrite (buf, len, 1, ajFileFp(file));
}

/* @funcstatic writeByte ********************************************
**
** Writes a single byte to a binary file
**
** @param [r] ch [char] Character
** @param [r] file [AjPFile] Output file
** @return [int] Return value from fwrite
** @@
******************************************************************************/

static int writeByte (char ch, AjPFile file) {
  return fwrite (&ch, 1, 1, ajFileFp(file));
}

/* @funcstatic sortfile ********************************************
**
** Sort a file, or a set of numbered files, individually
**
** @param [r] ext1 [const char*] Input file extension
** @param [r] ext2 [const char*] Output file extension
** @param [r] nfiles [int] NUmber of files to sort (zero if unnumbered)
** @return [void]
** @@
******************************************************************************/

static void sortfile (const char* ext1, const char* ext2, int nfiles) {

  static AjPStr cmdstr = NULL;
  int i;
  static AjPStr infname = NULL;
  static AjPStr outfname = NULL;
  static AjPStr srtext = NULL;

  if (nfiles) {
    for (i=1; i<=nfiles; i++) {
      ajFmtPrintS (&infname, "%S%02d.%s ", dbname, i, ext1);
      ajFmtPrintS (&outfname, "%S%02d.%s.srt", dbname, i, ext1);
      ajFmtPrintS (&cmdstr, "sort -o %S %S %S",
		   outfname, sortopt, infname);

      syscmd (cmdstr);

      rmfileI (ext1, i);
    }

    ajFmtPrintS (&cmdstr, "sort -m -o %S.%s %S",
		 dbname, ext2, sortopt);
    for (i=1; i<=nfiles; i++) {
      ajFmtPrintAppS (&cmdstr, " %S%02d.%s.srt", dbname, i, ext1);
    }
    syscmd (cmdstr);

    ajFmtPrintS (&srtext, "%s.srt ", ext1);
    for (i=1; i<=nfiles; i++) {
      rmfileI (ajStrStr(srtext), i);
    }
  }
  else {
    ajFmtPrintS (&infname, "%S.%s ", dbname, ext1);
    ajFmtPrintS (&outfname, "%S.%s", dbname, ext2);
    ajFmtPrintS (&cmdstr, "sort -o %S %S %S",
		 outfname, sortopt, infname);
    syscmd (cmdstr);
    rmfile (ext1, 0);
  }

  return;
}

/* @funcstatic rmfileI ********************************************
**
** Remove a numbered file
**
** @param [r] ext [const char*] Base file extension
** @param [r] ifile [int] File number.
** @return [void]
******************************************************************************/

static void rmfileI (const char* ext, int ifile) {

  static AjPStr cmdstr = NULL;

  if (!cleanup) return;

  ajFmtPrintS (&cmdstr, "rm %S%02d.%s ", dbname, ifile, ext);

  syscmd (cmdstr);

  return;
}

/* @funcstatic rmfile ********************************************
**
** Remove a file or a set of numbered files
**
** @param [r] ext [const char*] Base file extension
** @param [r] nfiles [int] Number of files, or zero for unnumbered.
** @return [void]
** @@
******************************************************************************/

static void rmfile (const char* ext, int nfiles) {

  static AjPStr cmdstr = NULL;
  int i;

  if (!cleanup) return;

  if (nfiles) {
    ajFmtPrintS (&cmdstr, "rm ");
    for (i=1; i<= nfiles; i++) {
      ajFmtPrintAppS (&cmdstr, "%S%02d.%s ", dbname, i, ext);
    }
  }
  else
    ajFmtPrintS (&cmdstr, "rm %S.%s", dbname, ext);
    

  syscmd (cmdstr);

  return;
}

/* @funcstatic syscmd ********************************************
**
** Fork a system command
**
** @param [r] cmdstr [AjPStr] Command line
** @return [void]
** @@
******************************************************************************/

static void syscmd (AjPStr cmdstr) {

  char** arglist = NULL;
  char* pgm;
  pid_t pid;
  int status;

  ajDebug ("forking '%S'", cmdstr);
  (void) ajSysArglist (cmdstr, &pgm, &arglist);

  pid=fork();
  if(pid==-1)
    ajFatal("System fork failed");

  if(!pid) {
    (void) execvp (pgm, arglist);
    return;
  }
  while(wait(&status)!=pid);

  ajSysArgListFree (&arglist);
  ajCharFree (pgm);

  return;
}

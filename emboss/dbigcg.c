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

static AjBool doReverse = AJFALSE;

AjPList idlist;
AjPList aclist;

AjBool systemsort;
AjBool cleanup;

typedef struct Sac {
  char* ac;
  char* entry;
  ajint nid;			/* entry number */
} Oac, *Pac;

typedef struct Sentry {
  ajint nac;
  ajint rpos;
  ajint spos;
  ajint filenum;
  char* entry;
  char** ac;
} Oentry, *Pentry;

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

static Pac    acnumNew (void);
static Pentry entryNew (void);
static Pentry nextentry (AjPFile libr, AjPFile libs, ajint ifile);
static AjBool gcgopenlib(AjPStr lname, AjPFile* libr, AjPFile* libs);
static ajint gcggetent(AjPFile libr, AjPFile libs,
		     ajint *d_pos, ajint *s_pos, AjPStr* libstr, AjPList alc);
static ajint pirgetent(AjPFile libr, AjPFile libs,
		     ajint *d_pos, ajint *s_pos, AjPStr* libstr, AjPList alc);
static ajint gcgappent( AjPFile libr, AjPFile libs,
		     AjPStr* libstr);

static char* newcharS (AjPStr str);
static char* newcharCI (char* str, ajint i);

static AjPStr idformat = NULL;

static AjBool parseEmbl (AjPStr line, AjPStr* id, AjPList acl);
static AjBool parsePir (AjPStr line, AjPStr* id, AjPList acl);
static AjBool parseGenbank (AjPStr line, AjPStr* id, AjPList acl);

typedef struct SParser {
  char* Name;
  AjBool (*Parser) (AjPStr line, AjPStr* id, AjPList acl);
} OParser;

static OParser parser[] = {
  {"EMBL", parseEmbl},
  {"SWISS", parseEmbl},
  {"GENBANK", parseGenbank},
  {"PIR", parsePir},
  {NULL, NULL}
};

static ajint cmpid (const void* a, const void* b);
static ajint cmpacid (const void* a, const void* b);
static ajint cmpacac (const void* a, const void* b);
static AjPList fileList (AjPStr dir, AjPStr wildfile);

static ajint  writeInt2 (short i, AjPFile file);
static ajint  writeInt4 (ajint i, AjPFile file);
static ajint writeStr (AjPStr str, ajint len, AjPFile file);
static ajint writeChar (char* str, ajint len, AjPFile file);
static ajint writeByte (char ch, AjPFile file);

static void syscmd (AjPStr cmdstr);
static void sortfile (const char* ext1, const char* ext2, ajint nfiles);
static void rmfile (const char* ext, ajint nfiles);
static void rmfileI (const char* ext, ajint ifile);

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

  Pentry entry;
  Pac acnum=NULL;
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

  inlist = fileList (directory, filename);
  ajListSort (inlist, ajStrCmp);
  nfiles = ajListToArray(inlist, &files);

  AJCNEW0(reffiles, nfiles);
  AJCNEW0(seqfiles, nfiles);
  AJCNEW0(twofiles, nfiles);

  if (systemsort)
    acnum = acnumNew();

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
	ajDebug("ids[%d] '%S' < acd[%d] '%S'\n",
		i, ((Pentry)ids[i])->entry,
		j, ((Pac)acs[j])->entry);
	i++;
      }
      else if (k > 0) {
	ajDebug("ids[%d] '%S' >> acd[%d] '%S'\n",
		i, ((Pentry)ids[i])->entry,
		j, ((Pac)acs[j])->entry);
	j++;
      }
      else {
	ajDebug("ids[%d] '%S' == acd[%d] '%S'\n",
		i, ((Pentry)ids[i])->entry,
		j, ((Pac)acs[j])->entry);
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
  if (!dfile)
    ajFatal("Failed to open %S for writing", dfname);

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
    writeStr (twofiles[i], maxfilelen, dfile);
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
      writeStr (idstr, maxidlen, efile);
      writeInt4 (rpos, efile);
      writeInt4 (spos, efile);
      writeInt2 (filenum, efile);
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
  if (!atfile)
    ajFatal("Failed to open %S for writing", atfname);
  ajStrAssC (&ahfname, "acnum.hit");
  ahfile = ajFileNewOutD(indexdir, ahfname);
  if (!ahfile)
    ajFatal("Failed to open %S for writing", ahfname);

  if (!systemsort)
    lastac = ((Pac)acs[0])->ac;

  ajDebug("writing acnum.hit %d\n", nac);

  alen = maxaclen+8;
  asize = 300 + (nac*(ajint)alen); /* to be fixed later */
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
	ajFatal("Failed to open %S for reading",alistfname);
    while (ajFileGets (alistfile, &rdline)) {
      ajRegExec (acsrt2exp, rdline);
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
	ajStrAssS(&lastidstr, idstr);
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
  writeInt4 (300+iac*(ajint)alen, atfile);
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
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

static ajint cmpid (const void* a, const void* b) {

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
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

static ajint cmpacid (const void* a, const void* b) {
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
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

static ajint cmpacac (const void* a, const void* b) {
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

/* @funcstatic nextentry ********************************************
**
** Returns next database entry as a Pentry object
**
** @param [r] libr [AjPFile] Reference file
** @param [r] libs [AjPFile] Sequence file
** @param [r] ifile [ajint] File number.
** @return [Pentry] Entry data object.
** @@
******************************************************************************/

static Pentry nextentry (AjPFile libr, AjPFile libs, ajint ifile) {

  static Pentry ret=NULL;
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
    ret = entryNew();

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
** Constructor for a text string from another text string
**
** @param [r] str [char*] Text object
** @param [r] i [ajint] Length
** @return [char*] New text string.
******************************************************************************/

static char* newcharCI (char* str, ajint i) {

  static char* buffer = NULL;
  static ajint ipos=0;
  static ajint imax=0;

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
** @param [w] d_pos [int*] Reference file offset returned
** @param [w] s_pos [int*] Sequence  file offset returned
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
** @param [w] d_pos [int*] Reference file offset returned
** @param [w] s_pos [int*] Sequence file offset  returned
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
  



/* @funcstatic parseEmbl ********************************************
**
** Parse the ID, accession from an EMBL or SWISSPROT entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool parseEmbl (AjPStr line, AjPStr* id, AjPList acl) {

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
	ac = newcharS (tmpac);
	ajListPushApp (acl, ac);
      }
      ajRegPost (ac2exp, &tmpline);
    }
    return ajTrue;
  }
  return ajFalse;
}


/* @funcstatic parseGenbank ********************************************
**
** Parse the ID, accession from a Genbank entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool parseGenbank (AjPStr line, AjPStr* id, AjPList acl) {

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
	ac = newcharS (tmpac);
	ajListPushApp (acl, ac);
      }
      ajRegPost (ac2exp, &tmpline);
    }
    return ajTrue;
  }
  return ajFalse;
}


/* @funcstatic parsePir ********************************************
**
** Parse the ID, accession from a PIR entry
**
** @param [r] line [AjPStr] Input line
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] List of accession numbers
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool parsePir (AjPStr line, AjPStr* id, AjPList acl) {

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
	ac = newcharS (tmpac);
	ajListPushApp (acl, ac);
      }
      ajRegPost (ac2exp, &tmpline);
    }
    return ajTrue;
  }
  return ajFalse;
}


/* @funcstatic fileList ********************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [AjPStr] Directory
** @param [r] wildfile [AjPStr] Wildcard file name
** @return [AjPList] New list of all files with full paths
** @@ 
******************************************************************************/

static AjPList fileList (AjPStr dir, AjPStr wildfile) {

  AjPList retlist = NULL;

  DIR* dp;
  struct dirent* de;
  ajint dirsize;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;

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
    if (!ajStrMatchWildCO(de->d_name, wildfile)) continue;
    dirsize++;
    ajDebug ("accept '%s'\n", de->d_name);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%s", dirfix, de->d_name);
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
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

static ajint  writeInt2 (short i, AjPFile file) {
  short j = i;
  if (doReverse)ajUtilRev2(&j);
    
  return fwrite (&j, 2, 1, ajFileFp(file));
}

/* @funcstatic writeInt4 ********************************************
**
** Writes a 4 byte integer to a binary file, with the correct byte orientation
**
** @param [r] i [ajint] Integer
** @param [r] file [AjPFile] Output file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

static ajint  writeInt4 (ajint i, AjPFile file) {
  ajint j=i;
  if (doReverse)ajUtilRev4(&j);
  return fwrite (&j, 4, 1, ajFileFp(file));
}

/* @funcstatic writeStr ********************************************
**
** Writes a string to a binary file
**
** @param [r] str [AjPStr] String
** @param [r] len [ajint] Length (padded) to use in the file
** @param [r] file [AjPFile] Output file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

static ajint writeStr (AjPStr str, ajint len, AjPFile file) {
  static char buf[256];
  ajint i = ajStrLen(str);
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
** @param [r] len [ajint] Length (padded) to use in the file
** @param [r] file [AjPFile] Output file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

static ajint writeChar (char* str, ajint len, AjPFile file) {
  static char buf[256];
  ajint i = strlen(str);
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
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

static ajint writeByte (char ch, AjPFile file) {
  return fwrite (&ch, 1, 1, ajFileFp(file));
}

/* @funcstatic sortfile ********************************************
**
** Sort a file, or a set of numbered files, individually
**
** @param [r] ext1 [const char*] Input file extension
** @param [r] ext2 [const char*] Output file extension
** @param [r] nfiles [ajint] NUmber of files to sort (zero if unnumbered)
** @return [void]
** @@
******************************************************************************/

static void sortfile (const char* ext1, const char* ext2, ajint nfiles) {

  static AjPStr cmdstr = NULL;
  ajint i;
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
** @param [r] ifile [ajint] File number.
** @return [void]
******************************************************************************/

static void rmfileI (const char* ext, ajint ifile) {

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
** @param [r] nfiles [ajint] Number of files, or zero for unnumbered.
** @return [void]
** @@
******************************************************************************/

static void rmfile (const char* ext, ajint nfiles) {

  static AjPStr cmdstr = NULL;
  ajint i;

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
  ajint status;

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

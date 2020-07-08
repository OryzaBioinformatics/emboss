/******************************************************************************
**
** EMBOSS/Staden/EMBLCD indexing
**
** This version reads a BLAST formatted database,
** and writes entryname and accession index files.
**
** It helps to know the format in order to
** parse the entryname and accession number,
** but it can guess if necessary.
**
** It also helps to know the type (blast1 or blast2)
** and the sequence type (protein or nucleic) but again
** it can guess by looking at the file extensions.
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
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <string.h>

#ifndef MAP_FILE      /* Solaris does not have MAP_FILE */
#define MAP_FILE 0
#endif

#define BLASTIDUNKNOWN 0
#define BLASTIDANY     1
#define BLASTIDNCBI    2
#define BLASTIDGCG     3
#define BLASTIDSANGER  4
#define BLASTPREFNCBI  1


static AjBool doReverse = AJFALSE;
static AjBool readReverse = AJFALSE;

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

typedef struct SMemFile {
  AjBool IsMem;
  AjPFile File;
  int Fd;
  long Pos;
  long Size;
  AjPStr Name;
  caddr_t Mem;
} OMemFile, *PMemFile;


typedef struct SBlastDb {
  int DbType;			/* database type indicator */
  int DbFormat;			/* database format (version) indicator */
  int IsProtein;		/* 1 for protein */
  int IsBlast2;			/* 1 for blast2, 0 for blast1 */
  int TitleLen;			/* length of database title */
  int DateLen;			/* length of database date string */
  int LineLen;			/* length of database lines */
  int HeaderLen;		/* bytes before tables start */
  int Size;			/* number of database entries */
  int CompLen;			/* length of compressed seq file */
  int MaxSeqLen;		/* max. entry length */
  int TotLen;			/* number of bases or residues in database */
  int CleanCount;		/* count of cleaned 8mers */
  int TopCmp;			/* bytes before compressed table starts */
  int TopSrc;			/* bytes before source table starts */
  int TopHdr;			/* bytes before headers table starts */
  int TopAmb;			/* bytes before ambiguity table starts */
  int IdType;			/* ID type */
  int IdPrefix;			/* ID prefix type */
  PMemFile TFile;		/* table of offsets, also DB info */
  PMemFile HFile;		/* description lines */
  PMemFile SFile;		/* binary sequence data */
  PMemFile FFile;		/* source sequence data */
  AjPStr Title;			/* database title */
  AjPStr Date;			/* database date */
  AjPStr Name;			/* database base file name */
} OBlastDb, *PBlastDb;

typedef struct SBlastType {
  char* ExtT;
  char* ExtH;
  char* ExtS;
  AjBool  IsProtein;
  AjBool IsBlast2;
  int   Type;
} OBlastType, *PBlastType;

enum blastdbtype {BLAST1P, BLAST1N, BLAST2P, BLAST2N};

static OBlastType blasttypes[] = {
  {"atb", "ahd", "bsq",	AJTRUE,  AJFALSE, BLAST1P},
  {"ntb", "nhd", "csq", AJFALSE, AJFALSE, BLAST1N},
  {"pin", "phr", "psq", AJTRUE,  AJTRUE, BLAST2P},
  {"nin", "nhr", "nsq", AJFALSE, AJTRUE, BLAST2N},
  {NULL, NULL, NULL, 0, 0, 0}
};

static int blastv=0;
static char dbtype='\0';

static int maxidlen = 12;
static int maxaclen = 12;

static AjPStr rline = NULL;
static AjPStr idformat = NULL;
static AjPStr version = NULL;
static AjPStr seqtype = NULL;

static AjPStr lastidstr = NULL;

static AjPFile elistfile=NULL;
static AjPFile alistfile=NULL;
static AjPFile blistfile=NULL;

static AjPStr dbname = NULL;
static AjPStr release = NULL;
static AjPStr datestr = NULL;
static AjPStr sortopt = NULL;

AjBool usesrc = AJTRUE;

static AjBool parseNcbi    (AjPStr line, PBlastDb db,
			    AjPStr* id, AjPList acl);
static AjBool parseGcg     (AjPStr line, PBlastDb db,
			    AjPStr* id, AjPList acl);
static AjBool parseSimple  (AjPStr line, PBlastDb db,
			    AjPStr* id, AjPList acl);
static AjBool parseId      (AjPStr line, PBlastDb db,
			    AjPStr* id, AjPList acl);
static AjBool parseUnknown (AjPStr line, PBlastDb db,
			    AjPStr* id, AjPList acl);

typedef struct SParser {
  char* Name;
  AjBool (*Parser) (AjPStr line, PBlastDb db, AjPStr* id, AjPList acl);
} OParser;

static OParser parser[] = {
  {"NCBI", parseNcbi},
  {"GCG", parseGcg},
  {"SIMPLE", parseSimple},
  {"ID", parseId},
  {"UNKNOWN", parseUnknown},
  {NULL, NULL}
};

static void   stripncbi (AjPStr* line);
static Pac    acnumNew (void);
static Pentry entryNew (void);
static Pentry nextblastentry (PBlastDb db, int ifile);
static AjBool blastopenlib(AjPStr lname, PBlastDb* pdb);

static char* newcharS (AjPStr str);
static char* newcharCI (char* str, int i);

static void newname(AjPStr* nname, AjPStr oname, char *suff);

static int cmpid (const void* a, const void* b);
static int cmpacid (const void* a, const void* b);
static int cmpacac (const void* a, const void* b);
static AjPList fileList (AjPStr dir, AjPStr wildfile);

static int  writeInt2 (short i, AjPFile file);
static int  writeInt4 (int i, AjPFile file);
static int writeStr (AjPStr str, int len, AjPFile file);
static int writeChar (char* str, int len, AjPFile file);
static int writeByte (char ch, AjPFile file);

static void readUInt4(PMemFile fd, unsigned int *val);

static PMemFile memfopenfile (AjPStr name);
static size_t memfseek (PMemFile mf, long offset, int whence);
static size_t memfread (void* dest, size_t size, size_t num_items, PMemFile mf);
static size_t memfreadS (AjPStr dest, size_t size, size_t num_items, PMemFile mf);

static void syscmd (AjPStr cmdstr);
static void sortfile (const char* ext1, const char* ext2, int nfiles);
static void rmfile (const char* ext, int nfiles);
static void rmfileI (const char* ext, int ifile);
static int loadtable (unsigned int* table, int isize, PBlastDb db,
	       int top, int pos);
static int ncblreadhdr (AjPStr* hline, PBlastDb db,
			int start, int end);

int main (int argc, char * argv[]) {

  AjBool staden;
  AjPStr directory;
  AjPStr indexdir;
  AjPStr filename;
  AjPStr curfilename = NULL;
  AjPStr elistfname = NULL;
  AjPStr alistfname = NULL;
  AjPStr blistfname = NULL;

  Pentry entry;
  Pac acnum=NULL;
  char* lastac=NULL;

  PBlastDb db=NULL;

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

  embInit ("dbiblast", argc, argv);

  if (ajUtilBigendian()) {
    doReverse = ajTrue;
    readReverse = ajFalse;
  }
  else {
    doReverse = ajFalse;
    readReverse = ajTrue;
  }

  staden = ajAcdGetBool ("staden");
/*  idformat = ajAcdGetListI ("idformat",1);*/
  idformat = ajStrNewC("NCBI");
  directory = ajAcdGetString ("directory");
  indexdir = ajAcdGetString ("indexdirectory");
  filename = ajAcdGetString ("filename");
  dbname = ajAcdGetString ("dbname");
  release = ajAcdGetString ("release");
  datestr = ajAcdGetString ("date");
  systemsort = ajAcdGetBool ("systemsort");
  cleanup = ajAcdGetBool ("cleanup");
  sortopt = ajAcdGetString ("sortoptions");
  version = ajAcdGetListI ("blastversion",1);
  seqtype = ajAcdGetListI ("seqtype",1);
  usesrc = ajAcdGetBool ("sourcefile");

  if (ajRegExec (datexp, datestr)) {
    for (i=1; i<4; i++) {
      ajRegSubI (datexp, i+1, &tmpstr);
      ajStrToInt (tmpstr, &j);
      date[3-i] = j;
    }
  }
  ajStrToInt (version, &blastv);
  dbtype = ajStrChar(seqtype,0);

  ajDebug ("staden: %B idformat: '%S'\n", staden, idformat);
  ajDebug ("version: '%S' %d seqtype: '%S'\n", version, blastv, seqtype);
  ajDebug ("reading '%S/%S'\n", directory, filename);
  ajDebug ("writing '%S/'\n", indexdir);

  idlist = ajListNew ();
  aclist = ajListNew ();

  inlist = fileList (directory, filename);
  ajListSort (inlist, ajStrCmp);
  nfiles = ajListToArray(inlist, &files);

  AJCNEW0(divfiles, nfiles);

  if (systemsort)
    acnum = acnumNew();

  for (ifile=0; ifile<nfiles; ifile++) {
    curfilename = (AjPStr) files[ifile];
    blastopenlib (curfilename, &db);
    ajDebug ("processing '%S' ...\n", curfilename);
    ajDebug ("processing '%S' ...\n", db->TFile->Name);
    ajStrAssS (&divfiles[ifile], db->TFile->Name);
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
    while ((entry=nextblastentry(db, ifile))) {
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
	  ajFatal("Cannot open %S for reading",elistfname);
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
    ajFmtPrintS (&elistfname, "%S.idsrt", dbname);
    elistfile = ajFileNewIn (elistfname);
    if(!elistfile)
	  ajFatal("Cannot open %S for reading",elistfname);
    while (ajFileGets (elistfile, &rline)) {
      ajRegExec (idsrtexp, rline);
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
	  ajFatal("Cannot open %S for writing",alistfname);
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

/* @funcstatic cmpid ********************************************
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

/* @funcstatic cmpacid ********************************************
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

/* @funcstatic cmpacac ********************************************
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

/* @funcstatic nextblastentry ********************************************
**
** Returns next  database entry as a Pentry object
**
** @param [r] db [PBlastDb] Blast database object
** @param [r] ifile [int] File number.
** @return [Pentry] Entry data object.
** @@
******************************************************************************/

static Pentry nextblastentry (PBlastDb db, int ifile) {

#define TABLESIZE 10000
#define HDRSIZE 1000

  static Pentry ret=NULL;
  int i;
  static AjPList acl = NULL;
  static int lastfile = -1;
  static int iparser = -1;
  static int called = 0;
  static unsigned int tabhdr[TABLESIZE];
  static int iload = TABLESIZE-1;
  static int irest = 0;
  static AjPStr id = NULL;
  static AjPStr hline = NULL;
  static AjPStr acc = NULL;
  static int ipos = 0;
  /*  static int isize = 0;*/
  static int jpos = 0;
  int ir;
  int j;
  static int is = 0;
  char* ac;

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
    ajStrModL (&id, HDRSIZE);
    ajStrModL (&acc, HDRSIZE);
    ajStrModL (&hline, HDRSIZE);
    acl = ajListNew();
    called = 1;
  }

  if (lastfile != ifile) {
    lastfile = ifile;
    ipos = 1;
    /*    isize = 0;*/
    irest = 0;
    iload = TABLESIZE-1;
  }

  if (!ret || !systemsort)
    ret = entryNew();

  /* pick up the next entry, parse it and dump it */


  if (ipos > db->Size)
    return NULL;

  if ( ipos >= irest) {
    ajDebug("ipos: %d iload: %d irest: %d\n", ipos, iload, irest);
    irest = ipos + TABLESIZE - 2;
    if (irest > db->Size) {
      iload = db->Size - ipos + 1;
      irest = db->Size;
    }

    jpos=0;
    j = loadtable(tabhdr, iload, db, db->TopHdr, ipos-1);
    if(!j)
      ajDebug("No elements read");
  }

  j = ncblreadhdr(&hline, db, tabhdr[jpos], tabhdr[jpos+1]);

/*  if (db->IsBlast2)
    stripncbi (&hline);*/
  
  if (!parser[iparser].Parser(hline, db, &id, acl)) {
    ajFatal("failed to parse '%S'", hline);
  }
  ir = ipos;

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

  }
  ipos++;
  jpos++;
  
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


/* @funcstatic blastopenlib ********************************************
**
** Open BLAST library
**
** @param [r] name [AjPStr] Source file name
** @param [u] pdb [PBlastDb*] Blast dababase structure.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool blastopenlib (AjPStr name, PBlastDb* pdb) {

  AjPStr hname = NULL;
  AjPStr sname = NULL;
  AjPStr tname = NULL;
  int rdtmp=0;
  int rdtmp2=0;
  int itype;
  int ttop;

  PBlastDb ret;
  
  AJNEW0 (*pdb);

  ret = *pdb;

  ajStrAssS(&ret->Name, name);
  ajDebug ("Name '%S'\n", ret->Name);

  /* find and open the 'table' file */

  for (itype=0; blasttypes[itype].ExtT; itype++) {
    if ((blastv == 1) && blasttypes[itype].IsBlast2) continue;
    if ((blastv == 2) && !blasttypes[itype].IsBlast2) continue;
    if ((dbtype == 'P') && !blasttypes[itype].IsProtein) continue;
    if ((dbtype == 'N') && blasttypes[itype].IsProtein) continue;
    newname(&tname,name,blasttypes[itype].ExtT);
    ret->TFile = memfopenfile(tname);
      if (ret->TFile) break;
  }
  if (!ret->TFile) {
    ajFatal(" cannot open %S table file %S\n", name, tname);
  }

  ajDebug("Successfully opened table file for type %d\n", itype);

  ret->IsProtein = blasttypes[itype].IsProtein;
  ret->IsBlast2 = blasttypes[itype].IsBlast2;

  /* read the type and format - all databases */

  readUInt4(ret->TFile,(unsigned int*)&ret->DbType);
  readUInt4(ret->TFile,(unsigned int*)&ret->DbFormat);
  ret->HeaderLen += 8;

  ajDebug ("dbtype: %x dbformat: %x\n", ret->DbType, ret->DbFormat);

  /* Open the header and (compressed) sequence files */
  /* for DNA, also look for the FASTA file */

  newname(&hname,name,blasttypes[itype].ExtH);
  if ((ret->HFile = memfopenfile(hname))==NULL) {
    ajFatal(" cannot open %S header file\n",hname);
  }
  newname(&sname,name,blasttypes[itype].ExtS);
  if ((ret->SFile = memfopenfile(sname))==NULL) {
    ajFatal(" cannot open %S sequence file\n",sname);
  }

  if (!ret->IsBlast2 && !ret->IsProtein && usesrc) {
    if ((ret->FFile = memfopenfile(name))==NULL) { /* this can fail */
      ajDebug(" cannot open %S source file\n",name);
    }
  }

  /* read the title - all formats */

  readUInt4(ret->TFile,(unsigned int*)&ret->TitleLen);
  if (ret->IsBlast2) {		/* blast2 does not align after the title */
    rdtmp = ret->TitleLen;
  }
  else
    rdtmp = ret->TitleLen + ((ret->TitleLen%4 !=0 ) ? 4-(ret->TitleLen%4) : 0);
  ajStrAssCL(&ret->Title, "", rdtmp+1);
  ajDebug ("IsBlast2: %B title_len: %d rdtmp: %d title_str: '%S'\n",
	  ret->IsBlast2, ret->TitleLen, rdtmp, ret->Title);
  ajStrTrace(ret->Title);
  memfreadS(ret->Title,(size_t)1,(size_t)rdtmp,ret->TFile);
  if (ret->IsBlast2)
    ajStrFixI (ret->Title, ret->TitleLen);
  else
    ajStrFixI (ret->Title, ret->TitleLen-1);

  ajDebug ("title_len: %d rdtmp: %d title_str: '%S'\n",
	  ret->TitleLen, rdtmp, ret->Title);

  ret->HeaderLen += 4 + rdtmp;

  /* read the date - blast2 */

  if (ret->IsBlast2) {
    readUInt4(ret->TFile,(unsigned int*)&ret->DateLen);
    rdtmp2 = ret->DateLen;
    ajStrAssCL(&ret->Date, "", rdtmp2+1);
    memfreadS (ret->Date,(size_t)1,(size_t)rdtmp2,ret->TFile);
    ajDebug ("datelen: %d rdtmp: %d date_str: '%S'\n",
	    ret->DateLen, rdtmp2, ret->Date);
    ret->HeaderLen += 4 + rdtmp2;
  }

  /* read the rest of the header (different for protein and DNA) */

  if (!ret->IsBlast2 && !ret->IsProtein) {
    readUInt4(ret->TFile,(unsigned int*)&ret->LineLen);	/* length of source lines */
    ret->HeaderLen += 4;
  }

  /* all formats have the next 3 */

  readUInt4 (ret->TFile,(unsigned int*)&ret->Size);
  if (ret->IsProtein) {		/* mad, but they are the other way for DNA */
    readUInt4 (ret->TFile,(unsigned int*)&ret->TotLen);
    readUInt4 (ret->TFile,(unsigned int*)&ret->MaxSeqLen);
  }
  else {
    readUInt4 (ret->TFile,(unsigned int*)&ret->MaxSeqLen);
    readUInt4 (ret->TFile,(unsigned int*)&ret->TotLen);
  }

  ret->HeaderLen += 12;

  if (!ret->IsBlast2 && !ret->IsProtein)  {     /* Blast 1.4 DNA only */
    readUInt4 (ret->TFile,(unsigned int*)&ret->CompLen);	/* compressed db length */
    readUInt4 (ret->TFile,(unsigned int*)&ret->CleanCount);	/* count of nt's cleaned */
    ret->HeaderLen += 8;
  }

  ajDebug(" size: %u, totlen: %d maxseqlen: %u\n",
	 ret->Size, ret->TotLen, ret->MaxSeqLen);
  ajDebug(" linelen: %u, complen: %d cleancount: %d\n",
	 ret->LineLen, ret->CompLen, ret->CleanCount);


  /* Now for the tables of offsets. Again maddeningly different in each */
  if (ret->IsBlast2) {
    ttop = ret->TopHdr = ret->HeaderLen; /* header first */
    ttop = ret->TopCmp = ttop + (ret->Size+1) * 4;     /* then sequence */
    if (!ret->IsProtein)      /* Blast 2 DNA only */
      ttop = ret->TopAmb = ttop + (ret->Size+1) * 4;
  }
  else {
    ttop = ret->TopCmp = ret->HeaderLen + ret->CleanCount*4; /* comp seq */
    if (!ret->IsProtein)      /* Blast 1.4 DNA only */
      ttop = ret->TopSrc = ttop + (ret->Size+1) * 4;
    ttop = ret->TopHdr = ttop + (ret->Size+1) * 4;     /* headers for all */
    if (!ret->IsProtein)      /* Blast 1.4 DNA only */
      ttop = ret->TopAmb = ttop + (ret->Size+1) * 4;
  }

  ajDebug("table file index  starts at %d\n", ret->HeaderLen);
  ajDebug("table file csq    starts at %d\n", ret->TopCmp);
  ajDebug("table file src    starts at %d\n", ret->TopSrc);
  ajDebug("table file hdr    starts at %d\n", ret->TopHdr);
  ajDebug("table file amb    starts at %d\n", ret->TopAmb);

  return ajTrue;
}



/* @funcstatic stripncbi ********************************************
**
** trim the ncbi line.
**
** @param [r] line [AjPStr*] Input line
** @return [void] 
** @@
******************************************************************************/

static void stripncbi (AjPStr* line) {

  static AjPRegexp gnlexp = NULL;
  static AjPRegexp giexp = NULL;
  static AjPStr tmpline = NULL;
  static AjPStr tmpstr = NULL;

  if (!gnlexp)
    gnlexp = ajRegCompC("^gnl[|][^|]+[|][^ ]+ +");

  if (!giexp)
    giexp = ajRegCompC("^gi[|][^|]+[|]");

  ajStrAssS (&tmpline, *line);

  ajDebug ("parseNCBI '%S'\n", tmpline);
  if (ajRegExec(gnlexp, tmpline)) {
    ajRegPost(gnlexp, &tmpstr);
    ajStrAssS (&tmpline, tmpstr);
  }

  if (ajRegExec(giexp, tmpline)) {
    ajRegPost(giexp, &tmpstr);
    ajStrAssS (&tmpline, tmpstr);
  }

  ajStrAssS (line, tmpline);
  ajDebug ("trim to   '%S'\n", tmpline);

  return;
}

/* @funcstatic parseNcbi ********************************************
**
** Parses an NCBI style header from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] db [PBlastDb] Database object
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list 
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool parseNcbi (AjPStr line, PBlastDb db,
			 AjPStr* id, AjPList acl) {

  char* ac;
  static AjPStr desc = NULL;
  static AjPStr tmpac = NULL;
  static AjPStr t = NULL;
  static int v=1;
  
  (void) ajStrAssC(&desc,"");
  (void) ajStrAssC(&t,"");
  (void) ajStrAssC(&tmpac,"");
  
  (void) ajFmtPrintS(&t,">%S",line);
  
  if(!ajSeqParseNcbi(t,id,&tmpac,&desc))
      return ajFalse;

  if(!ajStrLen(tmpac))
      (void) ajFmtPrintS(&tmpac,"ZZ%07d",v++);
  

  ajDebug ("parseNCBI success\n");

  if (systemsort) {
    ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
  }
  else {
    ac = newcharS (tmpac);
    ajListPushApp (acl, ac);
  }

  ajDebug ("parseNCBI '%S' '%S'\n", *id, tmpac);
  return ajTrue;
}

/* @funcstatic parseGcg ********************************************
**
** Parses a GCG style header from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] db [PBlastDb] Database object
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list 
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool parseGcg (AjPStr line, PBlastDb db,
			 AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPStr tmpac = NULL;
  char* ac;

  if (!idexp)
    idexp = ajRegCompC("^[^:]+:([^ ]+)( +([A-Za-z][A-Za-z0-9]+[0-9]))");

  if (!ajRegExec(idexp, line))
    return ajFalse;

  ajRegSubI (idexp, 1, id);
  ajRegSubI (idexp, 3, &tmpac);
  ajStrToUpper (&tmpac);	/* GCG mixes case on new SwissProt acnums */

  if (systemsort) {
    ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
  }
  else {
    ac = newcharS (tmpac);
    ajListPushApp (acl, ac);
  }

  ajDebug ("parseGCG '%S' '%S'\n", *id, tmpac);
  return ajTrue;
}

/* @funcstatic parseSimple ********************************************
**
** Parses a plain header from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] db [PBlastDb] Database object
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list 
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool parseSimple (AjPStr line, PBlastDb db,
			 AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;
  static AjPStr tmpac = NULL;
  char* ac;

  if (!idexp)
    idexp = ajRegCompC("^([^ ]+)( +([A-Za-z][A-Za-z0-9]+[0-9]))");

  if (!ajRegExec(idexp, line))
    return ajFalse;

  ajRegSubI (idexp, 1, id);
  ajRegSubI (idexp, 3, &tmpac);
  ajStrToUpper (&tmpac);	/* GCG mixes case on new SwissProt acnums */

  if (systemsort) {
    ajFmtPrintF (alistfile, "%S %S\n", *id, tmpac);
  }
  else {
    ac = newcharS (tmpac);
    ajListPushApp (acl, ac);
  }

  ajDebug ("parseSimple '%S' '%S'\n", *id, tmpac);
  return ajTrue;
}

/* @funcstatic parseId ********************************************
**
** Parses a simple FASTA ID from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] db [PBlastDb] Database object
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list 
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool parseId (AjPStr line, PBlastDb db,
			 AjPStr* id, AjPList acl) {

  static AjPRegexp idexp = NULL;

  if (!idexp)
    idexp = ajRegCompC("^([^ ]+)");

  if (!ajRegExec(idexp, line))
    return ajFalse;

  ajRegSubI (idexp, 1, id);

  ajDebug ("parseId '%S'\n", *id);
  return ajTrue;
}

/* @funcstatic parseUnknown ********************************************
**
** Parses an unknown type ID from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] db [PBlastDb] Database object
** @param [w] id [AjPStr*] ID
** @param [w] acl [AjPList] Accession number list 
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool parseUnknown (AjPStr line, PBlastDb db,
			 AjPStr* id, AjPList acl) {

  static int called = 0;

  if (!called) {		/* first time - find out the format */
    called = 1;
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

  AjPList retlist=NULL;
    
  DIR* dp;
  struct dirent* de;
  int dirsize;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;
  AjPStr tmp;
  AjPStr s;
  AjPStr s2;
  AjPStr t;
  
  char *p;
  char *q;
  AjPList l;
  int ll;
  int i;
  AjBool d;
  
  tmp = ajStrNewC(ajStrStr(wildfile));

  if (ajStrLen(dir))
    (void) ajStrAss (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dirfix, -1) != '/')
    (void) ajStrAppC (&dirfix, "/");

  (void) ajStrAppC(&wildfile,"*");

  dp = opendir (ajStrStr(dirfix));
  if (!dp)
    ajFatal("opendir failed on '%S'", dirfix);

  s = ajStrNew();
  l = ajListNew();
  dirsize = 0;
  retlist = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    if (!ajStrMatchWildCO(de->d_name, wildfile)) continue;
    (void) ajStrAssC(&s,de->d_name);
    p=q=ajStrStr(s);
    p=strrchr(p,(int)'.');
    if(p)
	*p='\0';
    s2 = ajStrNewC(q);
    
    ll=ajListLength(l);
    
    d=ajFalse;
    for(i=0;i<ll;++i)
    {
	ajListPop(l,(void *)&t);
	if(ajStrMatch(t,s2))
	   d=ajTrue;
	ajListPushApp(l,(void *)t);
    }
    if(!d)
	ajListPush(l,(void *)s2);
    else
    {
	ajStrDel(&s2);
	continue;
    }

    dirsize++;
    ajDebug ("accept '%S'\n", s2);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%S", dirfix, s2);
    ajListstrPushApp (retlist, name);
  }

  if(!ajListLength(retlist))
      ajFatal("No match for file specification %S",tmp);

  while(ajListPop(l,(void *)&t))
      ajStrDel(&t);
  ajStrDel(&s);
  ajStrDel(&tmp);

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

/* @funcstatic readUInt4 ********************************************
**
** Reads a 4 byte unsigned integer from a (possibly memory mapped)
** binary file, with the correct byte orientation
**
** @param [r] fd [PMemFile] Input file
** @param [r] val [unsigned int *] Unsigned integer
** @return [void]
** @@
******************************************************************************/

static void readUInt4 (PMemFile fd, unsigned int *val) {

  memfread((char *)val,(size_t)4,(size_t)1,fd);
  if (readReverse) ajUtilRev4((int *)val);
}

/* @funcstatic memfreadS ********************************************
**
** Reads a string from a (possibly memory mapped)
** binary file, with the correct byte orientation
**
** @param [w] dest [AjPStr] Output string, must be already the right size
** @param [r] size [size_t] Size of string (1)
** @param [r] num_items [size_t] Number of bytes
** @param [r] mf [PMemFile] Input file
** @return [size_t] fread return code
** @@
******************************************************************************/

static size_t memfreadS (AjPStr dest, size_t size, size_t num_items,
			 PMemFile mf) {

  return memfread (ajStrStr(dest), size, num_items, mf);
}

/* @funcstatic memfseek ********************************************
**
** fseek in a (possibly memory mapped) binary file
**
** @param [r] mf [PMemFile] Input file
** @param [r] offset [long] Offset in file
** @param [r] whence [int] Start of offset, as defined for 'fseek'
** @return [size_t] Result of 'fseek'
** @@
******************************************************************************/

static size_t memfseek (PMemFile mf, long offset, int whence) {

  if (mf->IsMem) {		/* memory mapped */
    switch (whence) {
    case 0:
      mf->Pos = offset;
      break;
    case 1:
      mf->Pos += offset;
      break;
    case 2:
      mf->Pos = mf->Size + offset;
      break;
    default:
      ajErr("invalid memfseek code %d", whence);
      exit (0);
    }
    if (mf->Pos > mf->Size)
      mf->Pos = mf->Size;
    if (mf->Pos < 0)
      mf->Pos = 0;
    return 0;
  }
  else {
    return ajFileSeek ( mf->File, offset, whence);
  }
}

/* @funcstatic memfread ********************************************
**
** fread in a (possibly memory mapped) binary file
**
** @param [w] dest [void*] Output text string
** @param [r] size [size_t] Size of string (1)
** @param [r] num_items [size_t] Number of bytes
** @param [r] mf [PMemFile] Input file
** @return [size_t] Result of 'fread'
** @@
******************************************************************************/

static size_t memfread (void* dest, size_t size, size_t num_items, PMemFile mf) {
  size_t i;
  if (mf->IsMem) {		/* memory mapped */
    i = size * num_items;
    memcpy (dest, &mf->Mem[mf->Pos], i);
    mf->Pos += i;
    return i;
  }
  else {
    return ajFileRead (dest, size, num_items, mf->File);
  }
}

/* @funcstatic sortfile ********************************************
**
** Sort a file, or a set of numbered files, individually
**
** @param [r] ext1 [const char*] Input file extension
** @param [r] ext2 [const char*] Output file extension
** @param [r] nfiles [int] Number of files to sort (zero if unnumbered)
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

/* @funcstatic newname ********************************************
**
** Generate a new filename with a different suffix.
**
** @param [w] nname [AjPStr*] New filename
** @param [r] oname [AjPStr] Original file name
** @param [r] suff [char*] New suffix
** @return [void]
** @@
******************************************************************************/

static void newname(AjPStr* nname, AjPStr oname, char *suff) {

  ajStrAssS (nname, oname);
  if (ajStrChar(oname,0)=='@')
    ajStrTrim (nname, 0);
  ajStrAppK (nname, '.');
  ajStrAppC (nname, suff);

}

/* @funcstatic memfopenfile ********************************************
**
** Open a (possibly memory mapped) binary file
**
** @param [r] name [AjPStr] File name
** @return [PMemFile] Memory mapped file object created
** @@
******************************************************************************/

static PMemFile memfopenfile (AjPStr name) {

  PMemFile ret;
  AjPFile fp;

  fp = ajFileNewIn(name);
  if (!fp) {
    return NULL;
  }

  AJNEW0(ret);

  ajStrAssS(&ret->Name, name);
  ret->IsMem = 0;
  ret->File = fp;
  ret->Size = 0;
  ret->Mem = NULL;

  ajDebug ("fopened '%S'\n", name);

  return ret;
  
}

/* @funcstatic loadtable ********************************************
**
** Load part of the BLAST binary table into memory
**
** @param [w] table [unsigned int*] table array to be read
** @param [r] isize [int] Number of elements to read
** @param [r] db [PBlastDb] Blast database structure
** @param [r] top [int] Byte offset for start of table
** @param [r] pos [int] Current element number in table.
** @return [int] Number of elements read.
** @@
******************************************************************************/

static int loadtable (unsigned int* table, int isize, PBlastDb db,
	       int top, int pos) {
  int i;
  int j;
  int imax;

  imax = pos + isize;
  if (imax > (db->Size+1)) imax = db->Size+1;

  ajDebug("loadtable size %d top %d db->Size %d pos %d imax %d\n",
	 isize, top, db->Size, pos, imax);

  memfseek (db->TFile, top + 4*(pos), 0);
  j=0;
  for (i=pos; i<=imax; i++) {
    ajDebug ("reading at %d\n", ajFileTell(db->TFile->File));
    readUInt4(db->TFile,&table[j++]);
    ajDebug ("read i: %d j: %d value: %d\n", i, j-1, table[j-1]);
  }

  return imax - pos + 1;

}

/* @funcstatic ncblreadhdr ********************************************
**
** Read the FASTA header line for one entry
**
** @param [w] hline [AjPStr*] Header line
** @param [r] db [PBlastDb] Blast database structure
** @param [r] start [int] Byte offset for start of header
** @param [r] end [int] Byte offset for end of header
** @return [int] Number of bytes read.
** @@
******************************************************************************/

static int ncblreadhdr (AjPStr* hline, PBlastDb db,
			int start, int end) {

  int size = ajStrSize (*hline);
  int llen;
  PMemFile hfp = db->HFile;

  if (end) {
    llen = end - start;
    if (db->IsBlast2)
      llen += 1;
    if (llen > size)
      llen = size;
  }
  else
    llen = size;

  ajDebug ("ncblreadhdr start %d end %d llen %d\n", start, end, llen);

  if (db->IsBlast2) {
    memfseek(hfp,start,0);
    memfreadS(*hline,(size_t)1,(size_t)(llen-1),hfp);
  }
  else {
    memfseek(hfp,start+1,0);		/* skip the '>' character */
    memfreadS(*hline,(size_t)1,(size_t)(llen-1),hfp);
  }

  ajStrFixI (*hline, (llen-1));

  return llen;
}

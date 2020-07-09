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
#ifndef _AIX
#include <sys/fcntl.h>
#else
#include <fcntl.h>
#endif
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


static AjBool readReverse = AJFALSE;

AjPList idlist;
AjPList aclist;

AjBool systemsort;
AjBool cleanup;

typedef struct SMemFile {
  AjBool IsMem;
  AjPFile File;
  ajint Fd;
  ajlong Pos;
  ajlong Size;
  AjPStr Name;
  caddr_t Mem;
} OMemFile, *PMemFile;


typedef struct SBlastDb {
  ajint DbType;			/* database type indicator */
  ajint DbFormat;			/* database format (version) indicator */
  ajint IsProtein;		/* 1 for protein */
  ajint IsBlast2;			/* 1 for blast2, 0 for blast1 */
  ajint TitleLen;			/* length of database title */
  ajint DateLen;			/* length of database date string */
  ajint LineLen;			/* length of database lines */
  ajint HeaderLen;		/* bytes before tables start */
  ajint Size;			/* number of database entries */
  ajint CompLen;			/* length of compressed seq file */
  ajint MaxSeqLen;		/* max. entry length */
  ajint TotLen;			/* number of bases or residues in database */
  ajint CleanCount;		/* count of cleaned 8mers */
  ajint TopCmp;			/* bytes before compressed table starts */
  ajint TopSrc;			/* bytes before source table starts */
  ajint TopHdr;			/* bytes before headers table starts */
  ajint TopAmb;			/* bytes before ambiguity table starts */
  ajint IdType;			/* ID type */
  ajint IdPrefix;			/* ID prefix type */
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
  ajint   Type;
} OBlastType, *PBlastType;

enum blastdbtype {BLAST1P, BLAST1N, BLAST2P, BLAST2N};

static OBlastType blasttypes[] = {
  {"atb", "ahd", "bsq",	AJTRUE,  AJFALSE, BLAST1P},
  {"ntb", "nhd", "csq", AJFALSE, AJFALSE, BLAST1N},
  {"pin", "phr", "psq", AJTRUE,  AJTRUE, BLAST2P},
  {"nin", "nhr", "nsq", AJFALSE, AJTRUE, BLAST2N},
  {NULL, NULL, NULL, 0, 0, 0}
};

static ajint blastv=0;
static char dbtype='\0';

static ajint maxidlen = 12;
static ajint maxaclen = 12;

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

/* static void   stripncbi (AjPStr* line); */
static EmbPentry nextblastentry (PBlastDb db, ajint ifile);
static AjBool blastopenlib(AjPStr lname, PBlastDb* pdb);

static void newname(AjPStr* nname, AjPStr oname, char *suff);

static void memreadUInt4(PMemFile fd, ajuint *val);

static PMemFile memfopenfile (AjPStr name);
static size_t memfseek (PMemFile mf, ajlong offset, ajint whence);
static size_t memfread (void* dest, size_t size, size_t num_items, PMemFile mf);
static size_t memfreadS (AjPStr dest, size_t size, size_t num_items, PMemFile mf);

static ajint loadtable (ajuint* table, ajint isize, PBlastDb db,
	       ajint top, ajint pos);
static ajint ncblreadhdr (AjPStr* hline, PBlastDb db,
			ajint start, ajint end);


/* @prog dbiblast *************************************************************
**
** Index a BLAST database
**
******************************************************************************/

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

  EmbPentry entry;
  EmbPac acnum=NULL;
  char* lastac=NULL;

  PBlastDb db=NULL;

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

  datexp = ajRegCompC("^([0-9]+).([0-9]+).([0-9]+)");
  idsrtexp = ajRegCompC ("^([^ ]+) +([0-9]+) +([0-9]+) +([0-9]+)");
  acsrtexp = ajRegCompC ("^([^ ]+) +([^ \n]+)");
  acsrt2exp = ajRegCompC ("^([^ ]+) +([0-9]+)");

  for (i=0;i<256;i++)
    padding[i] = ' ';

  embInit ("dbiblast", argc, argv);

  if (ajUtilBigendian()) {
    readReverse = ajFalse;
  }
  else {
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

  inlist = embDbiFileList (directory, filename, ajTrue);
  ajListSort (inlist, ajStrCmp);
  nfiles = ajListToArray(inlist, &files);

  AJCNEW0(divfiles, nfiles);

  if (systemsort)
    acnum = embDbiAcnumNew();

  for (ifile=0; ifile<nfiles; ifile++) {
    curfilename = (AjPStr) files[ifile];
    ajDebug ("processing '%S' ...\n", curfilename);
    blastopenlib (curfilename, &db);
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
	  ajFatal("Cannot open %S for writing",alistfname);
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
    embDbiRmFile (dbname,"acsrt2", 0, cleanup);
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

/* @funcstatic nextblastentry ********************************************
**
** Returns next  database entry as an EmbPentry object
**
** @param [r] db [PBlastDb] Blast database object
** @param [r] ifile [ajint] File number.
** @return [EmbPentry] Entry data object.
** @@
******************************************************************************/

static EmbPentry nextblastentry (PBlastDb db, ajint ifile) {

#define TABLESIZE 10000
#define HDRSIZE 1000

  static EmbPentry ret=NULL;
  ajint i;
  static AjPList acl = NULL;
  static ajint lastfile = -1;
  static ajint iparser = -1;
  static ajint called = 0;
  static ajuint tabhdr[TABLESIZE];
  static ajint iload = TABLESIZE-1;
  static ajint irest = 0;
  static AjPStr id = NULL;
  static AjPStr hline = NULL;
  static AjPStr acc = NULL;
  static ajint ipos = 0;
  /*  static ajint isize = 0;*/
  static ajint jpos = 0;
  ajint ir;
  ajint j;
  static ajint is = 0;
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
    ret = embDbiEntryNew();

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

  }
  ipos++;
  jpos++;
  
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
  ajint rdtmp=0;
  ajint rdtmp2=0;
  ajint itype;
  ajint ttop;

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

  memreadUInt4(ret->TFile,(ajuint*)&ret->DbType);
  memreadUInt4(ret->TFile,(ajuint*)&ret->DbFormat);
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

  memreadUInt4(ret->TFile,(ajuint*)&ret->TitleLen);
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
    memreadUInt4(ret->TFile,(ajuint*)&ret->DateLen);
    rdtmp2 = ret->DateLen;
    ajStrAssCL(&ret->Date, "", rdtmp2+1);
    memfreadS (ret->Date,(size_t)1,(size_t)rdtmp2,ret->TFile);
    ajDebug ("datelen: %d rdtmp: %d date_str: '%S'\n",
	    ret->DateLen, rdtmp2, ret->Date);
    ret->HeaderLen += 4 + rdtmp2;
  }

  /* read the rest of the header (different for protein and DNA) */

  if (!ret->IsBlast2 && !ret->IsProtein) {
    memreadUInt4(ret->TFile,(ajuint*)&ret->LineLen);	/* length of source lines */
    ret->HeaderLen += 4;
  }

  /* all formats have the next 3 */

  memreadUInt4 (ret->TFile,(ajuint*)&ret->Size);
  if (ret->IsProtein) {		/* mad, but they are the other way for DNA */
    memreadUInt4 (ret->TFile,(ajuint*)&ret->TotLen);
    memreadUInt4 (ret->TFile,(ajuint*)&ret->MaxSeqLen);
  }
  else {
    memreadUInt4 (ret->TFile,(ajuint*)&ret->MaxSeqLen);
    memreadUInt4 (ret->TFile,(ajuint*)&ret->TotLen);
  }

  ret->HeaderLen += 12;

  if (!ret->IsBlast2 && !ret->IsProtein)  {     /* Blast 1.4 DNA only */
    memreadUInt4 (ret->TFile,(ajuint*)&ret->CompLen);	/* compressed db length */
    memreadUInt4 (ret->TFile,(ajuint*)&ret->CleanCount);	/* count of nt's cleaned */
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



/* #funcstatic stripncbi ********************************************
**
** trim the ncbi line.
**
** #param [r] line [AjPStr*] Input line
** #return [void] 
** ##
******************************************************************************/

/* static void stripncbi (AjPStr* line) {

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
*/
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
  static ajint v=1;
  
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
    ac = ajCharNew (tmpac);
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
    ac = ajCharNew (tmpac);
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
    ac = ajCharNew (tmpac);
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

  static ajint called = 0;

  if (!called) {		/* first time - find out the format */
    called = 1;
  }

  return ajFalse;
}

/* @funcstatic memreadUInt4 ********************************************
**
** Reads a 4 byte unsigned integer from a (possibly memory mapped)
** binary file, with the correct byte orientation
**
** @param [r] fd [PMemFile] Input file
** @param [r] val [ajuint *] Unsigned integer
** @return [void]
** @@
******************************************************************************/

static void memreadUInt4 (PMemFile fd, ajuint *val) {

  memfread((char *)val,(size_t)4,(size_t)1,fd);
  if (readReverse) ajUtilRev4((ajint *)val);
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
** @param [r] offset [ajlong] Offset in file
** @param [r] whence [ajint] Start of offset, as defined for 'fseek'
** @return [size_t] Result of 'fseek'
** @@
******************************************************************************/

static size_t memfseek (PMemFile mf, ajlong offset, ajint whence) {

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
** @param [w] table [ajuint*] table array to be read
** @param [r] isize [ajint] Number of elements to read
** @param [r] db [PBlastDb] Blast database structure
** @param [r] top [ajint] Byte offset for start of table
** @param [r] pos [ajint] Current element number in table.
** @return [ajint] Number of elements read.
** @@
******************************************************************************/

static ajint loadtable (ajuint* table, ajint isize, PBlastDb db,
	       ajint top, ajint pos) {
  ajint i;
  ajint j;
  ajint imax;

  imax = pos + isize;
  if (imax > (db->Size+1)) imax = db->Size+1;

  ajDebug("loadtable size %d top %d db->Size %d pos %d imax %d\n",
	 isize, top, db->Size, pos, imax);

  memfseek (db->TFile, top + 4*(pos), 0);
  j=0;
  for (i=pos; i<=imax; i++) {
    ajDebug ("reading at %d\n", ajFileTell(db->TFile->File));
    memreadUInt4(db->TFile,&table[j++]);
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
** @param [r] start [ajint] Byte offset for start of header
** @param [r] end [ajint] Byte offset for end of header
** @return [ajint] Number of bytes read.
** @@
******************************************************************************/

static ajint ncblreadhdr (AjPStr* hline, PBlastDb db,
			ajint start, ajint end) {

  ajint size = ajStrSize (*hline);
  ajint llen;
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

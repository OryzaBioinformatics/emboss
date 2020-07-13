/* @source dbiblast application
**
** Index blast databases
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

typedef struct SMemFile
{
  AjBool IsMem;
  AjPFile File;
  ajint Fd;
  ajlong Pos;
  ajlong Size;
  AjPStr Name;
  caddr_t Mem;
} OMemFile, *PMemFile;

typedef struct SBlastDb
{
  ajint DbType;			/* database type indicator */
  ajint DbFormat;		/* database format (version) indicator */
  ajint IsProtein;		/* 1 for protein */
  ajint IsBlast2;		/* 1 for blast2, 0 for blast1 */
  ajint TitleLen;		/* length of database title */
  ajint DateLen;		/* length of database date string */
  ajint LineLen;		/* length of database lines */
  ajint HeaderLen;		/* bytes before tables start */
  ajint Size;			/* number of database entries */
  ajint CompLen;		/* length of compressed seq file */
  ajint MaxSeqLen;		/* max. entry length */
  ajint TotLen;			/* number of bases or residues in database */
  ajint CleanCount;		/* count of cleaned 8mers */
  ajint TopCmp;			/* bytes before compressed table starts */
  ajint TopSrc;			/* bytes before source table starts */
  ajint TopHdr;			/* bytes before headers table starts */
  ajint TopAmb;			/* bytes before ambiguity table starts */
  ajint IdType;			/* ID type */
  ajint IdPrefix;		/* ID prefix type */
  PMemFile TFile;		/* table of offsets, also DB info */
  PMemFile HFile;		/* description lines */
  PMemFile SFile;		/* binary sequence data */
  PMemFile FFile;		/* source sequence data */
  AjPStr Title;			/* database title */
  AjPStr Date;			/* database date */
  AjPStr Name;			/* database base file name */
} OBlastDb, *PBlastDb;

typedef struct SBlastType
{
  char* ExtT;
  char* ExtH;
  char* ExtS;
  AjBool  IsProtein;
  AjBool IsBlast2;
  ajint   Type;
} OBlastType, *PBlastType;

enum blastdbtype {BLAST1P, BLAST1N, BLAST2P, BLAST2N};

static OBlastType blasttypes[] =
{
  {"atb", "ahd", "bsq",	AJTRUE,  AJFALSE, BLAST1P},
  {"ntb", "nhd", "csq", AJFALSE, AJFALSE, BLAST1N},
  {"pin", "phr", "psq", AJTRUE,  AJTRUE, BLAST2P},
  {"nin", "nhr", "nsq", AJFALSE, AJTRUE, BLAST2N},
  {NULL, NULL, NULL, 0, 0, 0}
};

static AjBool dbiblast_parseNcbi    (AjPStr line, AjPFile* alistfile,
				     AjBool systemsort, AjPStr* fields,
				     PBlastDb db, ajint* maxFieldLen,
				     AjPStr* id, AjPList* fdl);
static AjBool dbiblast_parseGcg     (AjPStr line, AjPFile* alistfile,
				     AjBool systemsort, AjPStr* fields,
				     PBlastDb db, ajint* maxFieldLen,
				     AjPStr* id, AjPList* fdl);
static AjBool dbiblast_parseSimple  (AjPStr line, AjPFile* alistfile,
				     AjBool systemsort, AjPStr* fields,
				     PBlastDb db, ajint* maxFieldLen,
				     AjPStr* id, AjPList* fdl);
static AjBool dbiblast_parseId      (AjPStr line, AjPFile* alistfile,
				     AjBool systemsort, AjPStr* fields,
				     PBlastDb db, ajint* maxFieldLen,
				     AjPStr* id, AjPList* fdl);
static AjBool dbiblast_parseUnknown (AjPStr line, AjPFile* alistfile,
				     AjBool systemsort, AjPStr* fields,
				     PBlastDb db, ajint* maxFieldLen,
				     AjPStr* id, AjPList* fdl);

/* @datastatic DbiblastPParser ************************************************
**
** Parser definition structure
**
** @alias DbiblastSParser
** @alias DbiblastOParser
**
** @attr Name [char*] Parser name
** @attr Parser [(AjBool*)] Parser function
** @@
******************************************************************************/

typedef struct SParser
{
  char* Name;
  AjBool (*Parser) (AjPStr line, AjPFile* alistfile,
		    AjBool systemsort, AjPStr* fields,
		    PBlastDb db, ajint* maxFieldLen,
		    AjPStr* id, AjPList* fdl);
} OParser;

static OParser parser[] =
{
  {"NCBI", dbiblast_parseNcbi},
  {"GCG", dbiblast_parseGcg},
  {"SIMPLE", dbiblast_parseSimple},
  {"ID", dbiblast_parseId},
  {"UNKNOWN", dbiblast_parseUnknown},
  {NULL, NULL}
};

static EmbPEntry dbiblast_nextblastentry (PBlastDb db, ajint ifile,
					  AjPStr idformat, AjBool systemsort,
					  AjPStr* fields,
					  ajint* maxFieldLen, ajint* maxidlen,
					  AjPFile elistfile,
					  AjPFile* alistfile);
static AjBool dbiblast_blastopenlib(AjPStr lname, AjBool usesrc,
				    ajint blastv, char dbtype,
				    PBlastDb* pdb);

static void dbiblast_dbfree (PBlastDb* pdb);

static void dbiblast_dbname(AjPStr* dbname, AjPStr oname, char *suff);
static void dbiblast_newname(AjPStr* nname, AjPStr oname, char *suff);

static void dbiblast_memreadUInt4(PMemFile fd, ajuint *val);

static PMemFile dbiblast_memfopenfile (AjPStr name);
static void dbiblast_memfclosefile (PMemFile* pfd);
static size_t dbiblast_memfseek (PMemFile mf, ajlong offset, ajint whence);
static size_t dbiblast_memfread (void* dest, size_t size, size_t num_items,
				 PMemFile mf);
static size_t dbiblast_memfreadS (AjPStr dest, size_t size, size_t num_items,
				  PMemFile mf);

static ajint dbiblast_loadtable (ajuint* table, ajint isize, PBlastDb db,
				 ajint top, ajint pos);
static ajint dbiblast_ncblreadhdr (AjPStr* hline, PBlastDb db,
				   ajint start, ajint end);
static AjBool dbiblast_wrongtype(AjPStr oname, char *suff);

static AjBool readReverse = AJFALSE;

/* @prog dbiblast *************************************************************
**
** Index a BLAST database
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPList idlist;
    AjPList* fieldList=NULL;

    AjBool systemsort;
    AjBool cleanup;

    ajint blastv=0;
    char dbtype='\0';

    ajint maxindex;
    ajint maxidlen = 0;
    ajint maxlen;

    AjPStr version = NULL;
    AjPStr seqtype = NULL;

    AjPFile elistfile=NULL;
    AjPFile* alistfile=NULL;

    AjPStr dbname = NULL;
    AjPStr release = NULL;
    AjPStr datestr = NULL;
    AjPStr sortopt = NULL;
    void **entryIds = NULL;

    AjBool usesrc = AJTRUE;

    AjPStr directory;
    AjPStr indexdir;
    AjPStr filename;
    AjPStr exclude;
    AjPStr curfilename = NULL;

    AjPStr idformat = NULL;

    EmbPEntry entry;

    PBlastDb db=NULL;

    ajint idCount=0;
    ajint idDone;
    AjPList listTestFiles = NULL;
    void ** testFiles = NULL;
    ajint nfiles;
    ajint ifile;
    ajint jfile;

    ajint filesize;
    short recsize;
    ajint maxfilelen=20;
    char date[4] = {0,0,0,0};

    AjPStr tmpfname = NULL;
    AjPStr* fields = NULL;

    AjPFile entFile = NULL;

    AjPStr* divfiles = NULL;
    ajint* maxFieldLen=NULL;

    ajint ifield=0;
    ajint nfields=0;

    embInit ("dbiblast", argc, argv);

    idformat = ajStrNewC("NCBI");
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
    version = ajAcdGetListI ("blastversion",1);
    seqtype = ajAcdGetListI ("seqtype",1);
    usesrc = ajAcdGetBool ("sourcefile");

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

    if (ajUtilBigendian())
	readReverse = ajFalse;
    else
	readReverse = ajTrue;

    ajStrToInt (version, &blastv);
    dbtype = ajStrChar(seqtype,0);

    ajDebug ("reading '%S/%S'\n", directory, filename);
    ajDebug ("writing '%S/'\n", indexdir);

    listTestFiles = embDbiFileListExc (directory, filename, exclude);
    ajListSort (listTestFiles, ajStrCmp);
    nfiles = ajListToArray(listTestFiles, &testFiles);

    if (!nfiles)
	ajFatal ("No files selected");

    AJCNEW0(divfiles, nfiles);

    /*
    ** process each input file, one at a time
    */

    jfile = 0;
    for (ifile=0; ifile < nfiles; ifile++)
    {
	curfilename = (AjPStr) testFiles[ifile];
	if (!dbiblast_blastopenlib (curfilename,
				    usesrc, blastv, dbtype, &db))
	  continue;		/* could be the wrong file type with "*.*" */

	ajDebug ("processing filename '%S' ...\n", curfilename);
	ajDebug ("processing file '%S' ...\n", db->TFile->Name);


	ajStrAssS (&divfiles[jfile], db->TFile->Name);
	ajFileNameTrim(&divfiles[jfile]);
	if (ajStrLen(divfiles[jfile]) >= maxfilelen)
	    maxfilelen = ajStrLen(divfiles[jfile]) + 1;

	if (systemsort)		/* elistfile for entries, alist for fields */
	  elistfile = embDbiSortOpen (alistfile, jfile,
				      dbname, fields, nfields);

	while ((entry=dbiblast_nextblastentry(db, jfile,
					      idformat, systemsort,
					      fields,
					      maxFieldLen,
					      &maxidlen,
					      elistfile, alistfile)))
	{
	    idCount++;
	    if (!systemsort)	/* save the entry data in lists */
	      embDbiMemEntry (idlist, fieldList, nfields, entry, jfile);
	}
	if (systemsort)
	  embDbiSortClose (&elistfile, alistfile, nfields);

	dbiblast_dbfree(&db);
	jfile++;
    }
    nfiles = jfile;

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
        idDone = embDbiSortWriteEntry (entFile, maxidlen,
					dbname, nfiles, cleanup, sortopt);
    else			/* save entries in entryIds array */
    {
        idDone = embDbiMemWriteEntry (entFile, maxidlen,
				      idlist, &entryIds);
	if (idDone != idCount)
	  ajFatal ("Duplicates not allowed for in-memory processing");
    }

    embDbiHeaderSize (entFile, 300+(idDone*(ajint)recsize), idDone);
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

    ajListDel(&listTestFiles);

    ajExit ();
    return 0;
}

/* @funcstatic dbiblast_nextblastentry ****************************************
**
** Returns next  database entry as an EmbPEntry object
**
** @param [r] db [PBlastDb] Blast database object
** @param [r] ifile [ajint] File number.
** @param [r] idformat [AjPStr] Id format in FASTA file
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr*] Field names to be indexed
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] maxidlen [ajint*] Maximum entry ID length
** @param [r] elistfile [AjPFile] entry file
** @param [r] alistfile [AjPFile*] field data files array
** @return [EmbPEntry] Entry data object.
** @@
******************************************************************************/

static EmbPEntry dbiblast_nextblastentry (PBlastDb db, ajint ifile,
					  AjPStr idformat, AjBool systemsort,
					  AjPStr* fields,
					  ajint* maxFieldLen,
					  ajint* maxidlen,
					  AjPFile elistfile,
					  AjPFile* alistfile)
{

#define TABLESIZE 10000
#define HDRSIZE 1000

    static EmbPEntry ret=NULL;
    ajint i;
    static AjPList* fdl = NULL;
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

    static ajint jpos = 0;
    ajint ir;
    ajint j;
    static ajint is = 0;
    char* token;
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
	ajDebug ("idformat '%S' Parser %d\n", idformat, iparser);
	ajStrModL (&id, HDRSIZE);
	ajStrModL (&acc, HDRSIZE);
	ajStrModL (&hline, HDRSIZE);
	called = 1;
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

    if (lastfile != ifile)
    {
	lastfile = ifile;
	ipos = 1;
	/*    isize = 0;*/
	irest = 0;
	iload = TABLESIZE-1;
    }

    if (!ret || !systemsort)
	ret = embDbiEntryNew(nfields);

    /* pick up the next entry, parse it and dump it */

    if (ipos > db->Size)
	return NULL;

    if ( ipos >= irest)
    {
	ajDebug("ipos: %d iload: %d irest: %d\n", ipos, iload, irest);
	irest = ipos + TABLESIZE - 2;
	if (irest > db->Size)
	{
	    iload = db->Size - ipos + 1;
	    irest = db->Size;
	}

	jpos=0;
	j = dbiblast_loadtable(tabhdr, iload, db, db->TopHdr, ipos-1);
	if(!j)
	    ajDebug("No elements read");
    }

    j = dbiblast_ncblreadhdr(&hline, db, tabhdr[jpos], tabhdr[jpos+1]);

    if (!parser[iparser].Parser(hline, alistfile, systemsort, fields,
				db, maxFieldLen, &id, fdl))
	ajFatal("failed to parse '%S'", hline);

    ir = ipos;

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
	}
    }
    ipos++;
    jpos++;

    return ret;
}




/* @funcstatic dbiblast_dbfree ************************************************
**
** Free BLAST library object
**
** @param [u] pdb [PBlastDb*] Blast dababase structure.
** @return [void]
** @@
******************************************************************************/

static void dbiblast_dbfree ( PBlastDb* pdb)
{
  PBlastDb db = *pdb;

  if (!pdb) return;
  if (!*pdb) return;

  db = *pdb;

  dbiblast_memfclosefile (&db->TFile);
  dbiblast_memfclosefile (&db->HFile);
  dbiblast_memfclosefile (&db->SFile);
  dbiblast_memfclosefile (&db->FFile);

  ajStrDel (&db->Name);
  ajStrDel (&db->Date);
  ajStrDel (&db->Title);

  AJFREE (*pdb);

  return;
}

/* @funcstatic dbiblast_blastopenlib ******************************************
**
** Open BLAST library
**
** @param [r] name [AjPStr] Source file name
** @param [u] usesrc [AjBool] If ajTrue, use the source (fasta) file
** @param [r] blastv [ajint] Blast version number (1 or 2)
** @param [r] dbtype [char] Blast database type (p)rotein or (n)ucleotide
** @param [u] pdb [PBlastDb*] Blast dababase structure.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbiblast_blastopenlib (AjPStr name, AjBool usesrc,
				     ajint blastv, char dbtype,
				     PBlastDb* pdb)
{

    AjPStr hname = NULL;
    AjPStr sname = NULL;
    AjPStr tname = NULL;
    static AjPStr dbname = NULL;
    ajint rdtmp=0;
    ajint rdtmp2=0;
    ajint itype;
    ajint ttop;
    PMemFile TFile=NULL;

    PBlastDb ret;

    for (itype=0; blasttypes[itype].ExtT; itype++)
    {
	if ((blastv == 1) && blasttypes[itype].IsBlast2) continue;
	if ((blastv == 2) && !blasttypes[itype].IsBlast2) continue;
	if ((dbtype == 'P') && !blasttypes[itype].IsProtein) continue;
	if ((dbtype == 'N') && blasttypes[itype].IsProtein) continue;
	if (dbiblast_wrongtype(name, blasttypes[itype].ExtT))
	  continue;
        dbiblast_dbname(&dbname,name,blasttypes[itype].ExtT);
	dbiblast_newname(&tname,dbname,blasttypes[itype].ExtT);
	TFile = dbiblast_memfopenfile(tname);
	if (TFile) break;
    }
    if (!TFile)
      return ajFalse;

    AJNEW0 (*pdb);

    ret = *pdb;

    ret->TFile = TFile;

    ajStrAssS(&ret->Name, dbname);
    ajDebug ("Name '%S'\n", ret->Name);

    /* find and open the 'table' file(s) */

    if (!ret->TFile)
	ajFatal(" cannot open %S table file %S\n", dbname, tname);

    ajDebug("Successfully opened table file for type %d\n", itype);

    ret->IsProtein = blasttypes[itype].IsProtein;
    ret->IsBlast2 = blasttypes[itype].IsBlast2;

    /* read the type and format - all databases */

    dbiblast_memreadUInt4(ret->TFile,(ajuint*)&ret->DbType);
    dbiblast_memreadUInt4(ret->TFile,(ajuint*)&ret->DbFormat);
    ret->HeaderLen += 8;

    ajDebug ("dbtype: %x dbformat: %x\n", ret->DbType, ret->DbFormat);

    /* Open the header and (compressed) sequence files */
    /* for DNA, also look for the FASTA file */

    dbiblast_newname(&hname,dbname,blasttypes[itype].ExtH);
    if ((ret->HFile = dbiblast_memfopenfile(hname))==NULL)
	ajFatal(" cannot open %S header file\n",hname);


    dbiblast_newname(&sname,dbname,blasttypes[itype].ExtS);
    if ((ret->SFile = dbiblast_memfopenfile(sname))==NULL)
	ajFatal(" cannot open %S sequence file\n",sname);


    if (!ret->IsBlast2 && !ret->IsProtein && usesrc)
	/* this can fail */
	if ((ret->FFile = dbiblast_memfopenfile(dbname))==NULL)
	    ajDebug(" cannot open %S source file\n",dbname);

    /* read the title - all formats */

    dbiblast_memreadUInt4(ret->TFile,(ajuint*)&ret->TitleLen);

    /* blast2 does not align after the title */
    if (ret->IsBlast2)
	rdtmp = ret->TitleLen;
    else
	rdtmp = ret->TitleLen + ((ret->TitleLen%4 !=0 ) ?
				 4-(ret->TitleLen%4) : 0);
    ajStrAssCL(&ret->Title, "", rdtmp+1);
    ajDebug ("IsBlast2: %B title_len: %d rdtmp: %d title_str: '%S'\n",
	     ret->IsBlast2, ret->TitleLen, rdtmp, ret->Title);
    ajStrTrace(ret->Title);
    dbiblast_memfreadS(ret->Title,(size_t)1,(size_t)rdtmp,ret->TFile);
    if (ret->IsBlast2)
	ajStrFixI (ret->Title, ret->TitleLen);
    else
	ajStrFixI (ret->Title, ret->TitleLen-1);

    ajDebug ("title_len: %d rdtmp: %d title_str: '%S'\n",
	     ret->TitleLen, rdtmp, ret->Title);

    ret->HeaderLen += 4 + rdtmp;

    /* read the date - blast2 */

    if (ret->IsBlast2)
    {
	dbiblast_memreadUInt4(ret->TFile,(ajuint*)&ret->DateLen);
	rdtmp2 = ret->DateLen;
	ajStrAssCL(&ret->Date, "", rdtmp2+1);
	dbiblast_memfreadS (ret->Date,(size_t)1,(size_t)rdtmp2,ret->TFile);
	ajDebug ("datelen: %d rdtmp: %d date_str: '%S'\n",
		 ret->DateLen, rdtmp2, ret->Date);
	ret->HeaderLen += 4 + rdtmp2;
    }

    /* read the rest of the header (different for protein and DNA) */

    if (!ret->IsBlast2 && !ret->IsProtein)
    {
	/* length of source lines */
	dbiblast_memreadUInt4(ret->TFile,(ajuint*)&ret->LineLen);
	ret->HeaderLen += 4;
    }

    /* all formats have the next 3 */

    dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->Size);
    if (ret->IsProtein)
    {	/* mad, but they are the other way for DNA */
	dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->TotLen);
	dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->MaxSeqLen);
    }
    else
    {
	dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->MaxSeqLen);
	dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->TotLen);
    }

    ret->HeaderLen += 12;

    if (!ret->IsBlast2 && !ret->IsProtein)
    {	/* Blast 1.4 DNA only */
	/* compressed db length */
	dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->CompLen);
	/* count of nt's cleaned */
	dbiblast_memreadUInt4 (ret->TFile,(ajuint*)&ret->CleanCount);
	ret->HeaderLen += 8;
    }

    ajDebug(" size: %u, totlen: %d maxseqlen: %u\n",
	    ret->Size, ret->TotLen, ret->MaxSeqLen);
    ajDebug(" linelen: %u, complen: %d cleancount: %d\n",
	    ret->LineLen, ret->CompLen, ret->CleanCount);


    /* Now for the tables of offsets. Again maddeningly different in each */
    if (ret->IsBlast2)
    {
	ttop = ret->TopHdr = ret->HeaderLen; /* header first */
	ttop = ret->TopCmp = ttop + (ret->Size+1) * 4; /* then sequence */
	if (!ret->IsProtein)		/* Blast 2 DNA only */
	    ttop = ret->TopAmb = ttop + (ret->Size+1) * 4;
    }
    else
    {
	ttop = ret->TopCmp = ret->HeaderLen + ret->CleanCount*4; /* comp seq */
	if (!ret->IsProtein)		/* Blast 1.4 DNA only */
	    ttop = ret->TopSrc = ttop + (ret->Size+1) * 4;
	ttop = ret->TopHdr = ttop + (ret->Size+1) * 4; /* headers for all */
	if (!ret->IsProtein)		/* Blast 1.4 DNA only */
	    ttop = ret->TopAmb = ttop + (ret->Size+1) * 4;
    }

    ajDebug("table file index  starts at %d\n", ret->HeaderLen);
    ajDebug("table file csq    starts at %d\n", ret->TopCmp);
    ajDebug("table file src    starts at %d\n", ret->TopSrc);
    ajDebug("table file hdr    starts at %d\n", ret->TopHdr);
    ajDebug("table file amb    starts at %d\n", ret->TopAmb);

    ajStrDel(&hname);
    ajStrDel(&sname);
    ajStrDel(&tname);

    return ajTrue;
}



/* @funcstatic dbiblast_parseNcbi *********************************************
**
** Parses an NCBI style header from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [w] alistfile [AjPFile*] List of field temporary files
** @param [r] systemsort [AjBool] If ajTrue, use the system sort utility,
**                                else sort in memory
** @param [w] fields [AjPStr*] Field names
** @param [r] db [PBlastDb] Database object
** @param [r] maxFieldLen [ajint*] Maximum token lengths for each field
** @param [w] id [AjPStr*] ID
** @param [w] fdlist [AjPList*] Field token lists (one list for each field)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbiblast_parseNcbi (AjPStr line, AjPFile* alistfile,
				  AjBool systemsort, AjPStr* fields,
				  PBlastDb db, ajint* maxFieldLen,
				  AjPStr* id,
				  AjPList* fdlist)
{
    static AjPRegexp wrdexp = NULL;
    char* fd;
    static AjPStr tmpdes = NULL;
    static AjPStr tmpfd = NULL;
    static AjPStr tmpac = NULL;
    static AjPStr tmpsv = NULL;
    static AjPStr tmpgi = NULL;
    static AjPStr t = NULL;
    /* static ajint v=1; */ /* for faking ZZ accession numbers */
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = -1;
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
	else
	  ajWarn ("EMBL parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }

    if (!wrdexp)
	wrdexp = ajRegCompC ("([A-Za-z0-9]+)");

    (void) ajStrAssC(&tmpdes,"");
    (void) ajStrAssC(&t,"");
    (void) ajStrAssC(&tmpac,"");
    (void) ajStrAssC(&tmpsv,"");
    (void) ajStrAssC(&tmpgi,"");

    (void) ajFmtPrintS(&t,">%S",line);

    if(!ajSeqParseNcbi(t,id,&tmpac,&tmpsv,&tmpgi,&tmpdes))
	return ajFalse;

    if(ajStrLen(tmpac))
      (void) ajStrToUpper(&tmpac);

    if (accfield >= 0)
      embDbiMaxlen(&tmpac, &maxFieldLen[accfield]);

    if (svnfield >= 0)
    {
	embDbiMaxlen(&tmpsv, &maxFieldLen[svnfield]);
	embDbiMaxlen(&tmpgi, &maxFieldLen[svnfield]);
    }

    /*
    else
      (void) ajFmtPrintS(&tmpac,"ZZ%07d",v++);
    */

    (void) ajStrToUpper(id);

    /*ajDebug ("parseNCBI success\n");*/

    if (systemsort)
    {
      if (accfield >= 0 && ajStrLen(tmpac))
	ajFmtPrintF (alistfile[accfield], "%S %S\n", *id, tmpac);
      if (svnfield >= 0 && ajStrLen(tmpsv))
	ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpsv);
      if (svnfield >= 0 && ajStrLen(tmpgi))
	ajFmtPrintF (alistfile[svnfield], "%S %S\n", *id, tmpgi);
      if (desfield >= 0 && ajStrLen(tmpdes))
      {
	while (ajRegExec(wrdexp, tmpdes))
	{
	  ajRegSubI (wrdexp, 1, &tmpfd);
	  embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);
	  ajStrToUpper(&tmpfd);
	  ajDebug("++des '%S'\n", tmpfd);
	  ajFmtPrintF (alistfile[desfield], "%S %S\n", *id, tmpfd);
	  ajRegPost (wrdexp, &tmpdes);
	}
      }
    }
    else
    {
        if (accfield >= 0 && ajStrLen(tmpac))
	{
	    fd = ajCharNew (tmpac);
	    ajListPushApp (fdlist[accfield], fd);
	}
        if (svnfield >= 0 && ajStrLen(tmpsv))
	{
	    fd = ajCharNew (tmpsv);
	    ajListPushApp (fdlist[svnfield], fd);
	}
        if (svnfield >= 0 && ajStrLen(tmpgi))
	{
	    fd = ajCharNew (tmpgi);
	    ajListPushApp (fdlist[svnfield], fd);
	}
        if (desfield >= 0 && ajStrLen(tmpdes))
	{
	    while (ajRegExec(wrdexp, tmpdes))
	    {
		ajRegSubI (wrdexp, 1, &tmpfd);
		embDbiMaxlen(&tmpfd, &maxFieldLen[desfield]);
		ajStrToUpper(&tmpfd);
		ajDebug("++des '%S'\n", tmpfd);
		fd = ajCharNew(tmpfd);
		ajListPushApp (fdlist[desfield], fd);
		ajRegPost (wrdexp, &tmpdes);
	    }
	}
    }

    /*ajDebug ("parseNCBI '%S' '%S'\n", *id, tmpac);*/

    return ajTrue;
}

/* @funcstatic dbiblast_parseGcg **********************************************
**
** Parses a GCG style header from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr*] Field names to be indexed
** @param [r] db [PBlastDb] Database object
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Accession number list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbiblast_parseGcg (AjPStr line, AjPFile* alistfile,
				 AjBool systemsort, AjPStr* fields,
				 PBlastDb db, ajint* maxFieldLen,
				 AjPStr* id, AjPList* fdl)
{
    static AjPRegexp idexp = NULL;
    static AjPStr tmpac = NULL;
    char* ac;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = -1;
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
	else
	  ajWarn ("EMBL parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }


    if (!idexp)
	idexp = ajRegCompC("^[^:]+:([^ ]+)( +([A-Za-z][A-Za-z0-9]+[0-9]))");

    if (!ajRegExec(idexp, line))
	return ajFalse;

    ajRegSubI (idexp, 1, id);
    ajRegSubI (idexp, 3, &tmpac);
    (void) ajStrToUpper(id);
    (void) ajStrToUpper (&tmpac); /* GCG mixes case on new SwissProt acnums */

    if (accfield >= 0)
    {
        embDbiMaxlen(&tmpac, &maxFieldLen[accfield]);

	if (systemsort)
	    ajFmtPrintF (alistfile[accfield], "%S %S\n", *id, tmpac);
	else
	{
	    ac = ajCharNew (tmpac);
	    ajListPushApp (fdl[accfield], ac);
	}
    }

    ajDebug ("parseGCG '%S' '%S'\n", *id, tmpac);

    return ajTrue;
}




/* @funcstatic dbiblast_parseSimple *******************************************
**
** Parses a plain header from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr*] Field names to be indexed
** @param [r] db [PBlastDb] Database object
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Accession number list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbiblast_parseSimple (AjPStr line, AjPFile* alistfile,
				    AjBool systemsort, AjPStr* fields,
				    PBlastDb db, ajint* maxFieldLen,
				    AjPStr* id,
				    AjPList* fdl)
{
    static AjPRegexp idexp = NULL;
    static AjPStr tmpac = NULL;
    char* ac;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = -1;
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
	else
	  ajWarn ("EMBL parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }


    if (!idexp)
	idexp = ajRegCompC("^([^ ]+)( +([A-Za-z][A-Za-z0-9]+[0-9]))");

    if (!ajRegExec(idexp, line))
	return ajFalse;

    ajRegSubI (idexp, 1, id);
    ajRegSubI (idexp, 3, &tmpac);
    (void) ajStrToUpper(id);
    (void) ajStrToUpper (&tmpac); /* GCG mixes case on new SwissProt acnums */

    if (accfield >= 0)
    {
        embDbiMaxlen(&tmpac, &maxFieldLen[accfield]);
	if (systemsort)
	    ajFmtPrintF (alistfile[accfield], "%S %S\n", *id, tmpac);
	else
	{
	    ac = ajCharNew (tmpac);
	    ajListPushApp (fdl[accfield], ac);
	}
    }

    ajDebug ("parseSimple '%S' '%S'\n", *id, tmpac);

    return ajTrue;
}




/* @funcstatic dbiblast_parseId ***********************************************
**
** Parses a simple FASTA ID from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr*] Field names to be indexed
** @param [r] db [PBlastDb] Database object
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Accession number list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbiblast_parseId (AjPStr line, AjPFile* alistfile,
				AjBool systemsort, AjPStr* fields,
				PBlastDb db, ajint* maxFieldLen,
				AjPStr* id,
				AjPList* fdl)
{
    static AjPRegexp idexp = NULL;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = -1;
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
	else
	  ajWarn ("EMBL parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }


    if (!idexp)
	idexp = ajRegCompC("^([^ ]+)");

    if (!ajRegExec(idexp, line))
	return ajFalse;

    ajRegSubI (idexp, 1, id);
    ajStrToUpper(id);

    ajDebug ("parseId '%S'\n", *id);

    return ajTrue;
}




/* @funcstatic dbiblast_parseUnknown ******************************************
**
** Parses an unknown type ID from the BLAST header table.
**
** @param [r] line [AjPStr] Input line
** @param [r] alistfile [AjPFile*] field data files array
** @param [r] systemsort [AjBool] If ajTrue use system sort, else internal sort
** @param [r] fields [AjPStr*] Field names to be indexed
** @param [r] db [PBlastDb] Database object
** @param [w] maxFieldLen [ajint*] Maximum token length for each field
** @param [w] id [AjPStr*] ID
** @param [w] fdl [AjPList*] Accession number list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool dbiblast_parseUnknown (AjPStr line, AjPFile* alistfile,
				     AjBool systemsort, AjPStr* fields,
				     PBlastDb db, ajint* maxFieldLen,
				     AjPStr* id,
				     AjPList* fdl)
{
    static ajint called = 0;
    static ajint numFields;
    static ajint accfield=-1;
    static ajint desfield=-1;
    static ajint svnfield=-1;
    static AjBool reset = AJTRUE;

    if (!fields)
    {
      reset = ajTrue;
      accfield = svnfield = desfield = -1;
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
	else
	  ajWarn ("EMBL parsing unknown field '%S' ignored",
		  fields[numFields]);
	numFields++;
      }
      reset = ajFalse;
    }


    if (!called)	/* first time - find out the format */
	called = 1;

    return ajFalse;
}




/* @funcstatic dbiblast_memreadUInt4 ******************************************
**
** Reads a 4 byte unsigned integer from a (possibly memory mapped)
** binary file, with the correct byte orientation
**
** @param [r] fd [PMemFile] Input file
** @param [r] val [ajuint *] Unsigned integer
** @return [void]
** @@
******************************************************************************/

static void dbiblast_memreadUInt4 (PMemFile fd, ajuint *val)
{

    dbiblast_memfread((char *)val,(size_t)4,(size_t)1,fd);
    if (readReverse)
	ajUtilRev4((ajint *)val);

    return;
}




/* @funcstatic dbiblast_memfreadS *********************************************
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

static size_t dbiblast_memfreadS (AjPStr dest, size_t size, size_t num_items,
				  PMemFile mf)
{

    return dbiblast_memfread (ajStrStr(dest), size, num_items, mf);
}




/* @funcstatic dbiblast_memfseek **********************************************
**
** fseek in a (possibly memory mapped) binary file
**
** @param [r] mf [PMemFile] Input file
** @param [r] offset [ajlong] Offset in file
** @param [r] whence [ajint] Start of offset, as defined for 'fseek'
** @return [size_t] Result of 'fseek'
** @@
******************************************************************************/

static size_t dbiblast_memfseek (PMemFile mf, ajlong offset, ajint whence)
{

    if (mf->IsMem)
    {	/* memory mapped */
	switch (whence)
	{
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

    return ajFileSeek ( mf->File, offset, whence);
}




/* @funcstatic dbiblast_memfread **********************************************
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

static size_t dbiblast_memfread (void* dest, size_t size, size_t num_items,
				 PMemFile mf)
{
    size_t i;

    if (mf->IsMem)
    {					/* memory mapped */
	i = size * num_items;
	memcpy (dest, &mf->Mem[mf->Pos], i);
	mf->Pos += i;
	return i;
    }

    return ajFileRead (dest, size, num_items, mf->File);
}




/* @funcstatic dbiblast_newname ***********************************************
**
** Generate a new filename with a different suffix.
**
** @param [w] nname [AjPStr*] New filename
** @param [r] oname [AjPStr] Original file name
** @param [r] suff [char*] New suffix
** @return [void]
** @@
******************************************************************************/

static void dbiblast_newname(AjPStr* nname, AjPStr oname, char *suff)
{

    ajStrAssS (nname, oname);
    if (ajStrChar(oname,0)=='@')
	ajStrTrim (nname, 0);
    ajStrAppK (nname, '.');
    ajStrAppC (nname, suff);

    return;
}

/* @funcstatic dbiblast_dbname ************************************************
**
** Generate the database name (original fasta file name)
** by stripping off the suffix
**
** @param [w] dbname [AjPStr*] Database filename
** @param [r] oname [AjPStr] Original file name
** @param [r] suff [char*] New suffix
** @return [void]
** @@
******************************************************************************/

static void dbiblast_dbname(AjPStr* dbname, AjPStr oname, char *suff)
{
    AjPStr suffix=NULL;

    ajFmtPrintS(&suffix, ".%s", suff);

    ajStrAssS (dbname, oname);

    if (ajStrChar(oname,0)=='@')
	ajStrTrim (dbname, 0);

    if (!ajStrSuffix(*dbname, suffix))
    {
	ajStrDel(&suffix);
	return;
    }

    ajStrTrim (dbname, -ajStrLen(suffix));

    ajStrDel(&suffix);

    return;
}

/* @funcstatic dbiblast_wrongtype *********************************************
**
** Tests for the other database filenames in case the user asked
** for "*.*". Used to test we have the *.suff fiel before opening all files.
**
** @param [r] oname [AjPStr] Original file name
** @param [r] suff [char*] Required suffix
** @return [AjBool] ajTrue if any other filename suffix is recognized
** @@
******************************************************************************/

static AjBool dbiblast_wrongtype(AjPStr oname, char *suff)
{
    ajint itype;

    for (itype=0; blasttypes[itype].ExtT; itype++)
    {
      if (strcmp(suff, blasttypes[itype].ExtT))
      {
	if (ajStrSuffixC(oname, blasttypes[itype].ExtT))
	{
	  return ajTrue;
	}
      }

      if (strcmp(suff, blasttypes[itype].ExtH))
      {
	if (ajStrSuffixC(oname, blasttypes[itype].ExtH))
	{
	  return ajTrue;
	}
      }

      if (strcmp(suff, blasttypes[itype].ExtS))
      {
	if (ajStrSuffixC(oname, blasttypes[itype].ExtS))
	{
	  return ajTrue;
	}
      }
    }
    return ajFalse;
}

/* @funcstatic dbiblast_memfclosefile *****************************************
**
** Close a (possibly memory mapped) binary file
**
** @param [d] pfd [PMemFile*] File
** @return [void]
** @@
******************************************************************************/

static void dbiblast_memfclosefile (PMemFile* pfd)
{
  PMemFile fd;

  if (!pfd) return;
  if (!*pfd) return;

  fd = *pfd;
  ajFileClose(&fd->File);
  ajStrDel(&fd->Name);

  AJFREE (*pfd);
}

/* @funcstatic dbiblast_memfopenfile ******************************************
**
** Open a (possibly memory mapped) binary file
**
** @param [r] name [AjPStr] File name
** @return [PMemFile] Memory mapped file object created
** @@
******************************************************************************/

static PMemFile dbiblast_memfopenfile (AjPStr name)
{
    PMemFile ret;
    AjPFile fp;

    fp = ajFileNewIn(name);
    if (!fp)
	return NULL;

    AJNEW0(ret);

    ajStrAssS(&ret->Name, name);
    ret->IsMem = 0;
    ret->File = fp;
    ret->Size = 0;
    ret->Mem = NULL;

    ajDebug ("fopened '%S'\n", name);

    return ret;
}




/* @funcstatic dbiblast_loadtable *********************************************
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

static ajint dbiblast_loadtable (ajuint* table, ajint isize, PBlastDb db,
				 ajint top, ajint pos)
{
    ajint i;
    ajint j;
    ajint imax;

    imax = pos + isize;
    if (imax > (db->Size+1)) imax = db->Size+1;

    ajDebug("loadtable size %d top %d db->Size %d pos %d imax %d\n",
	    isize, top, db->Size, pos, imax);

    dbiblast_memfseek (db->TFile, top + 4*(pos), 0);
    j=0;
    for (i=pos; i<=imax; i++)
    {
      /*	ajDebug ("reading at %d\n", ajFileTell(db->TFile->File));*/
	dbiblast_memreadUInt4(db->TFile,&table[j++]);
	/* ajDebug ("read i: %d j: %d value: %d\n", i, j-1, table[j-1]);*/
    }

    return imax - pos + 1;
}




/* @funcstatic dbiblast_ncblreadhdr *******************************************
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

static ajint dbiblast_ncblreadhdr (AjPStr* hline, PBlastDb db, ajint start,
				   ajint end)
{
    ajint size = ajStrSize (*hline);
    ajint llen;
    PMemFile hfp = db->HFile;

    if (end)
    {
	llen = end - start;
	if (db->IsBlast2)
	    llen += 1;
	if (llen > size)
	    llen = size;
    }
    else
	llen = size;

    /*ajDebug ("ncblreadhdr start %d end %d llen %d\n", start, end, llen);*/

    if (db->IsBlast2)
    {
	dbiblast_memfseek(hfp,start,0);
	dbiblast_memfreadS(*hline,(size_t)1,(size_t)(llen-1),hfp);
    }
    else
    {
	dbiblast_memfseek(hfp,start+1,0); /* skip the '>' character */
	dbiblast_memfreadS(*hline,(size_t)1,(size_t)(llen-1),hfp);
    }

    ajStrFixI (*hline, (llen-1));

    return llen;
}

/******************************************************************************
** @source AJAX SEQ (sequence) functions
**
** These functions control all aspects of AJAX sequence
** reading and writing and include simple utilities.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
** @modified Jun 25 pmr First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "ajmem.h"
#include "ajfile.h"
#include "limits.h"
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>

static AjBool seqCdReverse = AJFALSE;

/* @datastatic SeqPCdDiv ******************************************************
**
** EMBLCD division file record structure
**
** @alias SeqSCdDiv
** @alias SeqOCdDiv
**
** @attr DivCode [ajint] Division code
** @attr FileName [AjPStr] Filename(s)
** @@
******************************************************************************/

typedef struct SeqSCdDiv
{
  ajint DivCode;
  AjPStr FileName;
} SeqOCdDiv, *SeqPCdDiv;

/* @datastatic SeqPCdEntry ****************************************************
**
** EMBLCD entrynam.idx file record structure
**
** @alias SeqSCdEntry
** @alias SeqOCdEntry
**
** @attr div [ajint] division file record
** @attr annoff [ajint] data file offset
** @attr seqoff [ajint] sequence file offset (if any)
** @@
******************************************************************************/

typedef struct SeqSCdEntry
{
  ajint div;
  ajint annoff;
  ajint seqoff;
} SeqOCdEntry, *SeqPCdEntry;

/* @datastatic SeqPCdFHeader **************************************************
**
** EMBLCD index file header structure, same for all index files.
**
** @alias SeqSCdFHeader
** @alias SeqOCdFHeader
**
** @attr FileSize [ajuint] Index file size
** @attr NRecords [ajuint] Index record count
** @attr IdSize [ajuint] Index string length
** @attr RelDay [ajint] Release date - day
** @attr RelMonth [ajint] Release date - month
** @attr RelYear [ajint] Release date - year
** @attr RecSize [short] Record size
** @attr DbName [char[24]] Database name
** @attr Release [char[12]] Release name/number
** @attr Date [char[4]] Data as read - 4 bytes
** @@
******************************************************************************/

typedef struct SeqSCdFHeader
{
  ajuint FileSize;
  ajuint NRecords;
  ajuint IdSize;
  ajint RelDay;
  ajint RelMonth;
  ajint RelYear;
  short RecSize;
  char DbName[24];
  char Release[12];
  char Date[4];
} SeqOCdFHeader, *SeqPCdFHeader;

/* @datastatic SeqPCdFile *****************************************************
**
** EMBLCD file data structure
**
** @alias SeqSCdFile
** @alias SeqOCdFile
**
** @attr Header [SeqPCdFHeader] Header data
** @attr File [AjPFile] File
** @attr NRecords [ajint] Number of records
** @attr RecSize [ajint] Record length (for calculating record offsets)
** @@
******************************************************************************/

typedef struct SeqSCdFile
{
  SeqPCdFHeader Header;
  AjPFile File;
  ajint NRecords;
  ajint RecSize;
} SeqOCdFile, *SeqPCdFile;

/* @datastatic SeqPCdHit ******************************************************
**
** EMBLCD hit file record structure
**
** @alias SeqSCdHit
** @alias SeqOCdHit
**
** @attr NHits [ajuint] Number of hits in HitList array
** @attr HitList [ajuint*] Array of hits, as record numbers in the
**                         entrynam file
** @@
******************************************************************************/

typedef struct SeqSCdHit
{
  ajuint NHits;
  ajuint* HitList;
} SeqOCdHit, *SeqPCdHit;

/* @datastatic SeqPCdIdx ******************************************************
**
** EMBLCD entryname index file record structure
**
** @alias SeqSCdIdx
** @alias SeqOCdIdx
**
** @attr AnnOffset [ajuint] Data file offset (see DivCode)
** @attr SeqOffset [ajuint] Sequence file offset (if any) (see DivCode)
** @attr EntryName [AjPStr] Entry ID - the file is sorted by these
** @attr DivCode [short] Division file record
** @@
******************************************************************************/

typedef struct SeqSCdIdx
{
  ajuint AnnOffset;
  ajuint SeqOffset;
  AjPStr EntryName;
  short DivCode;
} SeqOCdIdx, *SeqPCdIdx;

/* @datastatic SeqPCdTrg ******************************************************
**
** EMBLCD target (,trg) file record structure
**
** @alias SeqSCdTrg
** @alias SeqOCdTrg
**
** @attr FirstHit [ajuint] First hit record in .hit file
** @attr NHits [ajuint] Number of hit records in .hit file
** @attr Target [AjPStr] Indexed target string (the file is sorted by these)
** @@
******************************************************************************/

typedef struct SeqSCdTrg
{
  ajuint FirstHit;
  ajuint NHits;
  AjPStr Target;
} SeqOCdTrg, *SeqPCdTrg;

/* @datastatic SeqPCdQry ******************************************************
**
** EMBLCD query structure
**
** @alias SeqSCdQry
** @alias SeqOCdQry
**
** @attr divfile [AjPStr] division.lkp
** @attr idxfile [AjPStr] entryname.idx
** @attr datfile [AjPStr] main data reference
** @attr seqfile [AjPStr] sequence
** @attr tblfile [AjPStr] BLAST table
** @attr srcfile [AjPStr] BLAST FASTA source data
** @attr dfp [SeqPCdFile] division.lkp
** @attr ifp [SeqPCdFile] entryname.idx
** @attr trgfp [SeqPCdFile] acnum.trg
** @attr hitfp [SeqPCdFile] acnum.hit
** @attr trgLine [SeqPCdTrg]acnum input line
** @attr name [char*] filename from division.lkp
** @attr nameSize [ajint] division.lkp filename length
** @attr div [ajint] current division number
** @attr maxdiv [ajint] max division number
** @attr type [ajint] BLAST type
** @attr idnum [ajint] current BLAST entry offset
** @attr libr [AjPFile] main data reference or BLAST header
** @attr libs [AjPFile] sequence or BLAST compressed sequence
** @attr libt [AjPFile] blast table
** @attr libf [AjPFile] blast FASTA source data
** @attr TopHdr [ajint] BLAST table headers offset
** @attr TopCmp [ajint] BLAST table sequence offset
** @attr TopAmb [ajint] BLAST table ambiguities offset
** @attr TopSrc [ajint] BLAST table FASTA source offset
** @attr Size [ajint] BLAST database size
** @attr List [AjPList] list of entries
** @attr Skip [AjBool*] skip file(s) in division.lkp
** @attr idxLine [SeqPCdIdx] entryname.idx input line
** @@
******************************************************************************/

typedef struct SeqSCdQry
{
  AjPStr divfile;		/* division.lkp */
  AjPStr idxfile;		/* entryname.idx */
  AjPStr datfile;		/* main data reference */
  AjPStr seqfile;		/* sequence */
  AjPStr tblfile;		/* BLAST table */
  AjPStr srcfile;		/* BLAST FASTA source data */

  SeqPCdFile dfp;		/* division.lkp */
  SeqPCdFile ifp;		/* entryname.idx */
  SeqPCdFile trgfp;		/* acnum.trg */
  SeqPCdFile hitfp;		/* acnum.hit */
  SeqPCdTrg trgLine;		/* acnum input line */

  char* name;			/* filename from division.lkp */
  ajint nameSize;		/* division.lkp filename length */
  ajint div;			/* current division number */
  ajint maxdiv;			/* max division number */

  ajint type;			/* BLAST type */
  ajint idnum;			/* current BLAST entry offset */

  AjPFile libr;			/* main data reference or BLAST header */
  AjPFile libs;			/* sequence or BLAST compressed sequence */
  AjPFile libt;			/* blast table */
  AjPFile libf;			/* blast FASTA source data */

  ajint TopHdr;			/* BLAST table headers offset */
  ajint TopCmp;			/* BLAST table sequence offset */
  ajint TopAmb;			/* BLAST table ambiguities offset */
  ajint TopSrc;			/* BLAST table FASTA source offset */
  ajint Size;			/* BLAST database size */

  AjPList List;			/* list of entries */
  AjBool* Skip;			/* skip file(s) in division.lkp */
  SeqPCdIdx idxLine;		/* entryname.idx input line */
} SeqOCdQry, *SeqPCdQry;

static AjBool     seqAccessApp (AjPSeqin seqin);
static AjBool     seqAccessBlast (AjPSeqin seqin);
/* static AjBool     seqAccessCmd (AjPSeqin seqin);*/ /* not implemented */
static AjBool     seqAccessDirect (AjPSeqin seqin);
static AjBool     seqAccessEmblcd (AjPSeqin seqin);
static AjBool     seqAccessGcg (AjPSeqin seqin);
/* static AjBool     seqAccessNbrf (AjPSeqin seqin); */ /* obsolete */
static AjBool     seqAccessSrs (AjPSeqin seqin);
static AjBool     seqAccessSrsfasta (AjPSeqin seqin);
static AjBool     seqAccessSrswww (AjPSeqin seqin);
static AjBool     seqAccessUrl (AjPSeqin seqin);

static AjBool     seqBlastOpen (AjPSeqQuery qry, AjBool next);
static ajint      seqCdDivNext (AjPSeqQuery qry);
static AjBool     seqBlastAll (const AjPSeqin seqin);
static AjPFile    seqBlastFileOpen (AjPStr dir, AjPStr name);
static AjBool     seqBlastLoadBuff (const AjPSeqin seqin);
static AjBool     seqBlastQryNext (AjPSeqQuery qry);
static AjBool     seqBlastReadTable (const AjPSeqin seqin,
				     AjPStr* hline, AjPStr* seq);
static void       seqBlastStripNcbi (AjPStr* line);

static AjBool     seqCdAll (AjPSeqin seqin);
static int        seqCdEntryCmp (const void* a, const void* b);
static void       seqCdEntryDel (void** pentry, void* cl);
static SeqPCdFile seqCdFileOpen (AjPStr dir, char* name, AjPStr* fullname);
static size_t     seqCdFileRead (void* ptr, size_t element_size,
				 SeqPCdFile thys);
static size_t     seqCdFileReadInt (ajint* i, SeqPCdFile thys);
static size_t     seqCdFileReadName (char* name, size_t namesize,
				     SeqPCdFile thys);
static size_t     seqCdFileReadShort (short* i, SeqPCdFile thys);
static void       seqCdFileClose (SeqPCdFile *thys);
static ajint      seqCdFileSeek (SeqPCdFile fil, ajuint ipos);
static void       seqCdIdxLine (SeqPCdIdx idxLine,  ajuint ipos,
				SeqPCdFile fp);
static char*      seqCdIdxName (ajuint ipos, SeqPCdFile fp);
static AjBool     seqCdIdxQuery (AjPSeqQuery qry);
static ajint      seqCdIdxSearch (SeqPCdIdx idxLine, AjPStr entry,
				  SeqPCdFile fp);
static AjBool     seqCdQryClose (AjPSeqQuery qry);
static AjBool     seqCdQryEntry (AjPSeqQuery qry);
static AjBool     seqCdQryFile (AjPSeqQuery qry);
static AjBool     seqCdQryOpen (AjPSeqQuery qry);
static AjBool     seqCdQryNext (AjPSeqQuery qry);
static AjBool     seqCdQryQuery (AjPSeqQuery qry);
static AjBool     seqCdQryReuse (AjPSeqQuery qry);
static AjBool     seqCdReadHeader (SeqPCdFile fp);
static AjBool     seqCdTrgClose (SeqPCdFile *trgfil, SeqPCdFile *hitfil);
static ajint      seqCdTrgFind (AjPSeqQuery qry, char* indexname,
				AjPStr qrystring);
static void       seqCdTrgLine (SeqPCdTrg trgLine, ajuint ipos, SeqPCdFile fp);
static char*      seqCdTrgName (ajuint ipos, SeqPCdFile fp);
static AjBool     seqCdTrgOpen (AjPStr dir, char* name,
			    SeqPCdFile *trgfil, SeqPCdFile *hitfil);
static AjBool     seqCdTrgQuery (AjPSeqQuery qry);
static ajint      seqCdTrgSearch (SeqPCdTrg trgLine, AjPStr name,
				  SeqPCdFile fp);

static AjBool     seqGcgAll (const AjPSeqin seqin);
static void       seqGcgBinDecode (AjPStr thys, ajint rdlen);
static void       seqGcgLoadBuff (const AjPSeqin seqin);
static AjBool     seqGcgReadRef (const AjPSeqin seqin);
static AjBool     seqGcgReadSeq (const AjPSeqin seqin);

/* @funclist seqAccess ********************************************************
**
** Functions to access each database or sequence access method
**
******************************************************************************/

static SeqOAccess seqAccess[] =
{
  {"emblcd", seqAccessEmblcd},
  {"srs",seqAccessSrs},
  {"srsfasta",seqAccessSrsfasta},
  {"srswww",seqAccessSrswww},
  {"url",seqAccessUrl},
  /* {"cmd",seqAccessCmd}, */		/* not yet implemented */
  {"app",seqAccessApp},
  {"external",seqAccessApp},
  /* {"asis",ajSeqAccessAsis}, */      /* called directly by seqUsaProcess */
  /* {"file",ajSeqAccessFile}, */      /* called directly by seqUsaProcess */
  /* {"offset",ajSeqAccessOffset}, */  /* called directly by seqUsaProcess */
  {"direct",seqAccessDirect},
  /* {"nbrf",seqAccessNbrf}, */	       /* obsolete */
  {"gcg",seqAccessGcg},
  {"blast",seqAccessBlast},
  {NULL, NULL}
};

static char aa_btoa[27] = {"-ARNDCQEGHILKMFPSTWYVBZX*"};
static char aa_btoa2[27]= {"-ABCDEFGHIKLMNPQRSTVWXYZ*"};

/* @func ajSeqMethod **********************************************************
**
** Sets the access function for a named method for sequence reading.
**
** @param [r] method [AjPStr] Method required.
** @param [w] access [SeqPAccess*] Access function to use.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqMethod (AjPStr method, SeqPAccess* access)
{
    ajint i = 0;

    while (seqAccess[i].Name)
    {
	if (ajStrMatchCaseC (method, seqAccess[i].Name))
	{
	    ajDebug ("Matched seqAccess[%d] '%s'\n", i, seqAccess[i].Name);
	    *access = &seqAccess[i];
	    return ajTrue;
	}
	i++;
    }
    return ajFalse;
}

/* @section EMBL CD Database Indexing *****************************************
**
** These functions manage the EMBL CD-ROM index access methods.
** These include the "efetch" indexing used at the Sanger Centre
** based on Erik Sonnhammer's indexseqlibs code
** and a dirct copy of the database and index files from the
** EMBL CD-RM distribution.
**
** The index files start with a file "division.lkp" which contains
** the list of database filenames and an index number for each.
**
** "entrynam.idx" is a sorted index by entry name for each entry
** which points to a file number and a byte offset within the file.
**
** "acnum.trg" and "acnum.hit" index accession numbers and link them
** to record numbers in "entrynam.idx"
**
** Other index files are not used yet by EMBOSS but could be added
** using the "des" field in queries to search descriptions, and so on.
**
******************************************************************************/

/* @funcstatic seqAccessEmblcd ************************************************
**
** Reads a sequence using EMBL CD-ROM index files.
**
** @param [r] seqin [AjPSeqin] Sequence input, including query data.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessEmblcd (AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    static ajint qrycalled = 0;	/*  check bigendian once */


    ajDebug ("seqAccessEmblcd type %d\n", qry->Type);

    if (qry->Type == QRY_ALL)
	return seqCdAll (seqin);

    /* we need to search the index files and return a query */

    if (!qrycalled)
    {
	if (ajUtilBigendian())
	    seqCdReverse = ajTrue;
	qrycalled = 1;
    }

    if (qry->QryData)
    {					/* reuse unfinished query data */
	if (!seqCdQryReuse(qry))	/* oops, we're finished */
	    return ajFalse;
    }
    else
    {					/* starting out, set up query */
	seqin->Single = ajTrue;

	if (!seqCdQryOpen(qry))
	{
	    ajWarn("seqCdQry failed");
	    return ajFalse;
	}


	qryd = qry->QryData;

	/* binary search for the entryname we need */

	if (qry->Type == QRY_ENTRY)
	{
	    ajDebug ("entry id: '%S' acc: '%S'\n", qry->Id, qry->Acc);
	    if (!seqCdQryEntry (qry))
	    {
	      ajDebug ("EMBLCD Entry failed\n");
	      if (ajStrLen(qry->Id))
		  ajErr ("Database Entry '%S' not found", qry->Id);
	      else
		  ajErr ("Database Entry '%S' not found", qry->Acc);
	    }
	}
	if (qry->Type == QRY_QUERY)
	{
	    ajDebug ("query id: '%S' acc: '%S'\n", qry->Id, qry->Acc);
	    if (!seqCdQryQuery (qry))
	    {
	      ajDebug ("EMBLCD Query failed\n");
	      if (ajStrLen(qry->Id))
		  ajErr ("Database Query '%S' not found", qry->Id);
	      else if (ajStrLen(qry->Id))
		  ajErr ("Database Query '%S' not found", qry->Acc);
	      else if (ajStrLen(qry->Sv))
		  ajErr ("Database Query 'sv:%S' not found", qry->Sv);
	      else if (ajStrLen(qry->Des))
		  ajErr ("Database Query 'des:%S' not found", qry->Des);
	      else if (ajStrLen(qry->Key))
		  ajErr ("Database Query 'key:%S' not found", qry->Key);
	      else if (ajStrLen(qry->Org))
		  ajErr ("Database Query 'org:%S' not found", qry->Org);
	      else
		  ajErr ("Database Query '%S' not found", qry->Acc);
	    }
	}
    }

    if (ajListLength(qryd->List))
    {
	retval = seqCdQryNext (qry);
	if (retval)
	    ajFileBuffSetFile (&seqin->Filebuff, qryd->libr);
    }

    if (!ajListLength(qryd->List)) /* could have been emptied by code above */
    {
	seqCdQryClose (qry);
	/* AJB addition */
        /*
	 * This was for the old code where seqsets had different
         * memory handling ... and the reason for the multi
         * flag in the first place. So far this seems
         * unnecessary for the revised code but is left here
         * for a while as a reminder and 'just in case'
	 */
	if((qry->Type == QRY_ENTRY) && !seqin->multi)
	{
/*	    if(seqin->Ftquery->Handle)
		ajFileBuffClear(seqin->Ftquery->Handle,0); */
	    AJFREE(qry->QryData);
	    qryd = NULL;
	}

    }

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    return retval;
}

/* @funcstatic seqCdAll *******************************************************
**
** Reads the EMBLCD division lookup file and opens a list of all the
** database files for plain reading.
**
** @param [P] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdAll (AjPSeqin seqin)
{
    static AjPStr divfile = NULL;

    SeqPCdFile dfp;

    AjPList list;

    AjPSeqQuery qry = seqin->Query;

    ajint i;
    short j;
    ajint nameSize;
    char *name;
    AjPStr fullName = NULL;

    static ajint called = 0;	/*  we test once for bigendian */

    if (!called)
    {
	if (ajUtilBigendian())
	    seqCdReverse = ajTrue;
	called = 1;
    }

    if (!ajStrLen(qry->IndexDir))
    {
	ajDebug ("no indexdir defined for database %S\n", qry->DbName);
	ajErr ("no indexdir defined for database %S", qry->DbName);
	return ajFalse;
    }

    ajDebug ("EMBLCD All index directory '%S'\n", qry->IndexDir);

    dfp = seqCdFileOpen(qry->IndexDir, "division.lkp", &divfile);
    if (!dfp) {
	ajWarn("Cannot open division file '%S' for database '%S'",
		divfile, qry->DbName);
	return ajFalse;
    }

    nameSize = dfp->RecSize - 2;
    name = ajCharNewL (nameSize+1);

    list = ajListstrNew ();

    (void) seqCdFileSeek (dfp, 0);
    for (i=0; i < dfp->Header->NRecords; i++)
    {
	(void) seqCdFileReadShort (&j, dfp);
	(void) seqCdFileReadName (name, nameSize, dfp);
	fullName = ajStrNewC(name);
	ajFileNameDirSet (&fullName, qry->Directory);

	/* test exclusion list and add file if OK */

	if (ajFileTestSkip (fullName, qry->Exclude, qry->Filename,
			    ajTrue, ajTrue)) {
	    ajDebug("qrybufflist add '%S'\n", fullName);
	    ajListstrPushApp (list, fullName);
	    fullName = NULL;
	}
	else {
	    ajDebug("qrybufflist *delete* '%S'\n", fullName);
	    ajStrDel(&fullName);
	}
    }
    seqin->Filebuff = ajFileBuffNewInList(list);
    fullName = NULL;

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    seqCdFileClose (&dfp);
    ajStrDelReuse (&divfile);
    ajCharFree(name);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqBlastFileOpen ***********************************************
**
** Opens a named BLAST index file.
**
** @param [r] dir [AjPStr] Directory
** @param [r] name [AjPStr] File name.
** @return [AjPFile] file object.
** @@
******************************************************************************/

static AjPFile seqBlastFileOpen (AjPStr dir, AjPStr name)
{
    AjPFile thys;

    thys = ajFileNewDF(dir, name);
    if (!thys)
	return NULL;

    ajDebug ("seqBlastFileOpen '%F'\n", thys);

    return thys;
}

/* @funcstatic seqCdFileOpen **************************************************
**
** Opens a named EMBL CD-ROM index file.
**
** @param [r] dir [AjPStr] Directory
** @param [r] name [char*] File name.
** @param [w] fullname [AjPStr*] Full file name with directory path
** @return [SeqPCdFile] EMBL CD-ROM index file object.
** @@
******************************************************************************/

static SeqPCdFile seqCdFileOpen (AjPStr dir, char* name, AjPStr* fullname)
{
    SeqPCdFile thys=NULL;


    AJNEW0(thys);

    thys->File = ajFileNewDC(dir, name);

    if (!thys->File)
    {
	AJFREE(thys);
	return NULL;
    }


    AJNEW0(thys->Header);

    (void) seqCdReadHeader (thys);
    thys->NRecords = thys->Header->NRecords;
    thys->RecSize = thys->Header->RecSize;

    (void) ajStrAssS (fullname, ajFileGetName(thys->File));

    ajDebug ("seqCdFileOpen '%F' NRecords: %d RecSize: %d\n",
	     thys->File, thys->NRecords, thys->RecSize);


    return thys;
}

/* @funcstatic seqCdFileSeek **************************************************
**
** Sets the file position in an EMBL CD-ROM index file.
**
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index file object.
** @param [r] ipos [ajuint] Offset.
** @return [ajint] Return value from the seek operation.
** @@
******************************************************************************/


static ajint seqCdFileSeek (SeqPCdFile fil, ajuint ipos)
{
    ajint ret;
    ajuint jpos;

    jpos = 300 + ipos*fil->RecSize;
    ret = ajFileSeek(fil->File, jpos, 0);
    ajDebug("seqCdFileSeek rec %u pos %u tell %ld returns %d\n",
	    ipos, jpos, ajFileTell(fil->File), ret);

    return ret;
}

/* @funcstatic seqCdFileRead **************************************************
**
** Reads a specified number of bytes from an EMBL CD-ROM index file.
**
** @param [w] ptr [void*] Buffer to read into.
** @param [r] element_size [size_t] Number of bytes to read.
** @param [r] thys [SeqPCdFile] EMBL CD-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileRead (void* ptr, size_t element_size,
		   SeqPCdFile thys)
{
    return ajFileRead (ptr, element_size, 1, thys->File);
}

/* @funcstatic seqCdFileReadName **********************************************
**
** Reads a character string from an EMBL CD-ROM index file. Trailing spaces
** (if any) are truncated. EMBLCD indices normally have a trailing NULL
** character.
**
** @param [w] name [char*] Buffer to read into. Must be at least namesize
**                         bytes in size.
** @param [r] namesize [size_t] Number of bytes to read from index file.
** @param [r] thys [SeqPCdFile] EMBL CD-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadName (char* name, size_t namesize,
				 SeqPCdFile thys)
{
    size_t ret;
    char* sp;

    ajDebug("seqCdFileReadName pos %ld\n", ajFileTell(thys->File));
    ret =  ajFileRead (name, namesize, 1, thys->File);

    ajDebug("seqCdFileReadName was '%s' ret %d\n", name, ret);

    name[namesize] = '\0';
    sp = &name[strlen(name)];
    while (sp > name)
    {
        sp--;
	if (*sp != ' ')
	  break;
	*sp = '\0';
    }

    ajDebug("seqCdFileReadName now '%s'\n", name);
    return ret;
}

/* @funcstatic seqCdFileReadInt ***********************************************
**
** Reads a 4 byte integer from an EMBL CD-ROM index file. If the byte
** order in the index file does not match the current system the bytes
** are reversed automatically.
**
** @param [w] i [ajint*] Integer read from file.
** @param [r] thys [SeqPCdFile] EMBL CR-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadInt (ajint* i, SeqPCdFile thys)
{
    size_t ret;

    ret = ajFileRead (i, 4, 1, thys->File);

    if (seqCdReverse)
	ajUtilRev4(i);

    return ret;
}

/* @funcstatic seqCdFileReadShort *********************************************
**
** Reads a 2 byte integer from an EMBL CD-ROM index file. If the byte
** order in the index file does not match the current system the bytes
** are reversed automatically.
**
** @param [w] i [short*] Integer read from file.
** @param [r] thys [SeqPCdFile] EMBL CR-ROM index file.
** @return [size_t] Number of bytes read.
** @@
******************************************************************************/

static size_t seqCdFileReadShort (short* i, SeqPCdFile thys)
{
    size_t ret;

    ret = ajFileRead (i, 2, 1, thys->File);

    if (seqCdReverse)
	ajUtilRev2(i);

    return ret;
}

/* @funcstatic seqCdFileClose *************************************************
**
** Closes an EMBL CD-ROM index file.
**
** @param [u] pthis [SeqPCdFile*] EMBL CD-ROM index file.
** @return [void]
** @@
******************************************************************************/

static void seqCdFileClose (SeqPCdFile* pthis)
{
    SeqPCdFile thys = *pthis;


    ajDebug ("seqCdFileClose of %F\n", thys->File);
    ajFileClose(&thys->File);
    AJFREE(thys->Header);
    AJFREE(*pthis);

    return;
}

/* @funcstatic seqCdIdxSearch *************************************************
**
** Binary search through an EMBL CD-ROM index file for an exact match.
**
** @param [u] idxLine [SeqPCdIdx] Index file record.
** @param [r] entry [AjPStr] Entry name to search for.
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [ajint] Record number on success, -1 on failure.
** @@
******************************************************************************/

static ajint seqCdIdxSearch (SeqPCdIdx idxLine, AjPStr entry, SeqPCdFile fil)
{
    AjPStr entrystr = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos=0;
    ajint icmp=0;
    char *name;

    (void) ajStrAssS (&entrystr, entry);
    (void) ajStrToUpper (&entrystr);

    ajDebug("seqCdIdxSearch (entry '%S') records: %d\n",
	    entry, fil->NRecords);

    if (fil->NRecords < 1)
      return -1;

    ilo = 0;
    ihi = fil->NRecords - 1;
    while (ilo <= ihi)
    {
	ipos = (ilo + ihi)/2;
	name = seqCdIdxName (ipos, fil);
	icmp = ajStrCmpC(entrystr, name);
	ajDebug ("idx test %u '%s' %2d (+/- %u)\n", ipos, name, icmp, ihi-ilo);
	if (!icmp) break;
	if (icmp < 0)
	    ihi = ipos-1;
	else
	    ilo = ipos+1;
    }

    ajStrDel (&entrystr);

    if (icmp)
	return -1;

    seqCdIdxLine (idxLine, ipos, fil);

    return ipos;
}

/* @funcstatic seqCdIdxQuery **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard entry name.
**
** @param [r] qry [AjPSeqQuery] Sequence query object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdIdxQuery (AjPSeqQuery qry)
{
    SeqPCdQry qryd = qry->QryData;

    AjPList list = qryd->List;
    SeqPCdIdx idxLine = qryd->idxLine;
    AjPStr idname = qry->Id;
    SeqPCdFile fil = qryd->ifp;

    AjPStr idstr = NULL;
    AjPStr idpref = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos=0;
    ajint icmp;
    char *name;
    ajint i;
    ajint ilen;
    ajint jlo;
    ajint jhi;
    ajint khi;
    AjBool first;
    ajint ifail=0;
    ajint iskip=0;

    SeqPCdEntry entry;

    (void) ajStrAssS (&idstr,idname);
    (void) ajStrToUpper (&idstr);
    (void) ajStrAssS (&idpref, idstr);

    (void) ajStrWildPrefix (&idpref);

    ajDebug ("seqCdIdxQuery (wild '%S' prefix '%S')\n",
	     idstr, idpref);

    jlo = ilo = 0;
    khi = jhi = ihi = fil->NRecords-1;

    ilen = ajStrLen(idpref);
    first = ajTrue;
    if (ilen)
    {					/* find first entry with this prefix */
	while (ilo <= ihi)
	{
	    ipos = (ilo + ihi)/2;
	    name = seqCdIdxName (ipos, fil);
	    name[ilen] = '\0';
	    icmp = ajStrCmpC(idpref, name); /* test prefix */
	    ajDebug ("idx test %d '%s' %2d (+/- %d)\n",
		     ipos, name, icmp, ihi-ilo);
	    if (!icmp)
	    {				/* hit prefix - test for first */
		ajDebug ("idx hit %d\n", ipos);
		if (first)
		{
		    jhi = ihi;
		    first = ajFalse;
		    khi = ipos;
		}
		jlo = ipos;
	    }
	    if (icmp > 0)
		ilo = ipos+1;
	    else
		ihi = ipos-1;
	}

	if (first)
	{				/* failed to find any with prefix */
	    ajStrDel (&idstr);
	    ajStrDel (&idpref);
	    return ajFalse;
	}

	ajDebug ("first pass: ipos %d jlo %d jhi %d\n", ipos, jlo, jhi);

	/* now search below for last */

	ilo = jlo+1;
	ihi = jhi;
	while (ilo <= ihi)
	{
	    ipos = (ilo + ihi)/2;
	    name = seqCdIdxName (ipos, fil);
	    name[ilen] = '\0';
	    icmp = ajStrCmpC(idpref, name);
	    ajDebug ("idx test %d '%s' %2d (+/- %d)\n",
		     ipos, name, icmp, ihi-ilo);
	    if (!icmp)
	    {				/* hit prefix */
		ajDebug ("idx hit %d\n", ipos);
		khi = ipos;
	    }
	    if (icmp < 0)
		ihi = ipos-1;
	    else
		ilo = ipos+1;
	}
	ajDebug ("second pass: ipos %d jlo %d khi %d\n",
		 ipos, jlo, khi);

	name = seqCdIdxName (jlo, fil);
	ajDebug ("first  %d '%s'\n", jlo, name);
	name = seqCdIdxName (khi, fil);
	ajDebug (" last  %d '%s'\n", khi, name);

    }

    for (i=jlo; i <= khi; i++)
    {
	seqCdIdxLine (idxLine, i, fil);
	if (ajStrMatchWild(idxLine->EntryName, idstr))
	{
	    if (!qryd->Skip[idxLine->DivCode-1])
	    {
	      if (ifail)
	      {
		ajDebug ("FAIL: %d entries\n", ifail);
		ifail=0;
	      }
	      if (iskip)
	      {
		ajDebug ("SKIP: %d entries\n", iskip);
		iskip=0;
	      }
	      ajDebug ("  OK: '%S'\n", idxLine->EntryName);
		AJNEW0(entry);
		entry->div = idxLine->DivCode;
		entry->annoff = idxLine->AnnOffset;
		entry->seqoff = idxLine->SeqOffset;
		ajListPushApp (list, (void*)entry);
	    }
	    else
	    {
	      ajDebug ("SKIP: '%S' [file %d]\n",
			 idxLine->EntryName, idxLine->DivCode);
	      iskip++;
	    }
	}
	else
	{
	  ++ifail;
	  /* ajDebug ("FAIL: '%S' '%S'\n", idxLine->EntryName, idstr);*/
	}
    }

    if (ifail)
    {
      ajDebug ("FAIL: %d entries\n", ifail);
      ifail=0;
    }
    if (iskip)
    {
      ajDebug ("SKIP: %d entries\n", iskip);
      ifail=0;
    }

    ajStrDel (&idstr);
    ajStrDel (&idpref);

    if (ajListLength(list))
	return ajTrue;

    return ajFalse;
}

/* @funcstatic seqCdTrgSearch *************************************************
**
** Binary search of EMBL CD-ROM target file, for example an accession number
** search.
**
** @param [u] trgLine [SeqPCdTrg] Target file record.
** @param [r] entry [AjPStr] Entry name or accession number.
** @param [r] fp [SeqPCdFile] EMBL CD-ROM target file
** @return [ajint] Record number, or -1 on failure.
** @@
******************************************************************************/

static ajint seqCdTrgSearch (SeqPCdTrg trgLine, AjPStr entry, SeqPCdFile fp)
{
    AjPStr entrystr = NULL;
    ajint ihi;
    ajint ilo;
    ajint ipos;
    ajint icmp;
    ajint itry;
    char *name;

    (void) ajStrAssS (&entrystr, entry);
    (void) ajStrToUpper (&entrystr);

    if (fp->NRecords < 1)
      return -1;

    ilo = 0;
    ihi = fp->NRecords;
    ipos = (ilo + ihi)/2;
    icmp = -1;
    ajDebug ("seqCdTrgSearch '%S' recSize: %d\n", entry, fp->RecSize);
    name = seqCdTrgName (ipos, fp);
    icmp = ajStrCmpC(entrystr, name);

    ajDebug ("trg testa %d '%s' %2d (+/- %d)\n", ipos, name, icmp, ihi-ilo);

    while (icmp)
    {
	if (icmp < 0)
	    ihi = ipos;
	else
	    ilo = ipos;
	itry = (ilo + ihi)/2;
	if (itry == ipos)
	{
	    ajDebug("'%S' not found in .trg\n", entrystr);
	    ajStrDel (&entrystr);
	    return -1;
	}
	ipos = itry;
	name = seqCdTrgName (ipos, fp);
	icmp = ajStrCmpC(entrystr, name);
	ajDebug ("trg testb %d '%s' %2d (+/- %d)\n", ipos, name, icmp, ihi-ilo);
    }

    seqCdTrgLine (trgLine, ipos, fp);

    ajStrDel (&entrystr);

    if (!trgLine->NHits)
	return -1;

    ajDebug("found in .trg at record %d\n", ipos);


    return ipos;
}

/* @funcstatic seqCdIdxName ***************************************************
**
** Reads the name from record ipos of an EMBL CD-ROM index file.
** The name length is known from the index file object.
**
** @param [r] ipos [ajuint] Record number.
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [char*] Name read from file.
** @@
******************************************************************************/

static char* seqCdIdxName (ajuint ipos, SeqPCdFile fil)
{
    static char* name = NULL;
    static ajint maxNameSize = 0;
    ajint nameSize;

    nameSize = fil->RecSize-10;

    if (maxNameSize < nameSize)
    {
	maxNameSize = nameSize;
	if (name)
	    (void) ajCharFree(name);
	name = ajCharNewL (maxNameSize);
    }

    (void) seqCdFileSeek (fil, ipos);
    (void) seqCdFileReadName (name, nameSize, fil);

    return name;
}

/* @funcstatic seqCdIdxLine ***************************************************
**
** Reads a numbered record from an EMBL CD-ROM index file.
**
** @param [u] idxLine [SeqPCdIdx] Index file record.
** @param [r] ipos [ajuint] Record number.
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [void]
** @@
******************************************************************************/

static void seqCdIdxLine (SeqPCdIdx idxLine, ajuint ipos, SeqPCdFile fil)
{
    static char* name = NULL;
    static ajint maxNameSize = 0;
    ajint nameSize;

    nameSize = fil->RecSize-10;

    if (maxNameSize < nameSize)
    {
	maxNameSize = nameSize;
	if (name)
	    (void) ajCharFree(name);
	name = ajCharNewL (maxNameSize);
    }

    (void) seqCdFileSeek (fil, ipos);
    (void) seqCdFileReadName (name, nameSize, fil);

    (void) ajStrAssC(&idxLine->EntryName,name);

    (void) seqCdFileReadInt (&idxLine->AnnOffset, fil);
    (void) seqCdFileReadInt (&idxLine->SeqOffset, fil);
    (void) seqCdFileReadShort (&idxLine->DivCode, fil);

    return;
}

/* @funcstatic seqCdTrgName ***************************************************
**
** Reads the target name from an EMBL CD-ROM index target file.
**
** @param [r] ipos [ajuint] Record number.
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index target file.
** @return [char*] Name.
** @@
******************************************************************************/

static char* seqCdTrgName (ajuint ipos, SeqPCdFile fil)
{
    static char* name = NULL;
    static ajint maxNameSize = 0;
    ajint nameSize;

    nameSize = fil->RecSize-8;

    if (maxNameSize < nameSize)
    {
	maxNameSize = nameSize;
	if (name)
	    (void) ajCharFree(name);
	name = ajCharNewL (maxNameSize);
    }

    (void) seqCdFileSeek (fil, ipos);
    (void) seqCdFileRead (name, 8, fil);
    (void) seqCdFileReadName (name, nameSize, fil);

    ajDebug("seqCdTrgName maxNameSize:%d nameSize:%d name '%s'\n",
	    maxNameSize, nameSize, name);

    return name;
}

/* @funcstatic seqCdTrgLine ***************************************************
**
** Reads a line from an EMBL CD-ROM index target file.
**
** @param [w] trgLine [SeqPCdTrg] Target file record.
** @param [r] ipos [ajuint] Record number.
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index target file.
** @return [void].
** @@
******************************************************************************/

static void seqCdTrgLine (SeqPCdTrg trgLine, ajuint ipos, SeqPCdFile fil)
{
    static char* name = NULL;
    static ajint maxNameSize = 0;
    ajint nameSize;

    nameSize = fil->RecSize-8;

    if (maxNameSize < nameSize)
    {
	maxNameSize = nameSize;
	if (name)
	    (void) ajCharFree(name);
	name = ajCharNewL (maxNameSize);
    }

    (void) seqCdFileSeek (fil, ipos);

    (void) seqCdFileReadInt(&trgLine->NHits, fil);
    (void) seqCdFileReadInt(&trgLine->FirstHit, fil);
    (void) seqCdFileReadName (name, nameSize, fil);

    trgLine->Target = ajStrNewC(name);

    ajDebug("seqCdTrgLine %d nHits %d firstHit %d target '%S'\n",
	    ipos, trgLine->NHits, trgLine->FirstHit, trgLine->Target);

    return;
}

/* @funcstatic seqCdReadHeader ************************************************
**
** Reads the header of an EMBL CD-ROM index file.
**
** @param [r] fil [SeqPCdFile] EMBL CD-ROM index file.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdReadHeader (SeqPCdFile fil)
{
    SeqPCdFHeader header = fil->Header;

    (void) seqCdFileReadInt (&header->FileSize, fil);
    (void) seqCdFileReadInt (&header->NRecords, fil);
    (void) seqCdFileReadShort (&header->RecSize, fil);

    header->IdSize = header->RecSize - 10;

    (void) seqCdFileReadName (header->DbName, 20, fil);
    (void) seqCdFileReadName (header->Release, 10, fil);

    (void) seqCdFileReadName (header->Date, 4, fil);
    header->RelYear = header->Date[1];
    header->RelMonth = header->Date[2];
    header->RelDay = header->Date[3];

    ajDebug ("seqCdReadHeader file %F\n", fil->File);
    ajDebug("  FileSize: %d NRecords: %hd recsize: %d idsize: %d\n",
	    header->FileSize, header->NRecords,
	    header->RecSize, header->IdSize);

    return ajTrue;
}

/* @funcstatic seqCdTrgOpen ***************************************************
**
** Opens an EMBL CD-ROM target file pair.
**
** @param [r] dir [AjPStr] Directory.
** @param [r] name [char*] File name.
** @param [w] trgfil [SeqPCdFile*] Target file.
** @param [w] hitfil [SeqPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdTrgOpen (AjPStr dir, char* name, SeqPCdFile* trgfil,
			    SeqPCdFile* hitfil)
{
    static AjPStr tmpname = NULL;
    static AjPStr fullname = NULL;

    (void) ajFmtPrintS (&tmpname, "%s.trg",name);
    *trgfil = seqCdFileOpen (dir, ajStrStr(tmpname), &fullname);
    if (!*trgfil)
	return ajFalse;

    (void) ajFmtPrintS (&tmpname, "%s.hit",name);
    *hitfil = seqCdFileOpen (dir, ajStrStr(tmpname), &fullname);
    if (!*hitfil)
	return ajFalse;

    return ajTrue;
}

/* @funcstatic seqCdTrgClose **************************************************
**
** Close an EMBL CD-ROM target file pair.
**
** @param [w] ptrgfil [SeqPCdFile*] Target file.
** @param [w] phitfil [SeqPCdFile*] Hit file.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdTrgClose (SeqPCdFile* ptrgfil, SeqPCdFile* phitfil)
{

    seqCdFileClose (ptrgfil);
    seqCdFileClose (phitfil);

    return ajTrue;
}

/*=============================================================================
** SRS indexed database access
**===========================================================================*/

/* @section SRS Database Indexing *********************************************
**
** These functions manage the SRS (getz) index access methods.
**
******************************************************************************/

/* @funcstatic seqAccessSrs ***************************************************
**
** Reads sequence(s) using SRS. Opens a file using the results of an SRS
** query and returns to the caller to read the data.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSrs (AjPSeqin seqin)
{
    static AjPStr searchdb = NULL;

    AjPSeqQuery qry = seqin->Query;

    if (!ajNamDbGetDbalias(qry->DbName, &searchdb))
	(void) ajStrAssS (&searchdb, qry->DbName);

    if (!ajStrLen (qry->Application))
	(void) ajStrAssC (&qry->Application, "getz");

    ajDebug ("seqAccessSrs %S:%S\n", searchdb, qry->Id);
    if (ajStrLen(qry->Id)) {
	(void) ajFmtPrintS(&seqin->Filename, "%S -e '[%S-id:%S]",
			   qry->Application, searchdb, qry->Id);
	if (ajStrMatch(qry->Id, qry->Acc)) /* or accnumber */
	    (void) ajFmtPrintAppS(&seqin->Filename, "|[%S-acc:%S]'|",
				  searchdb, qry->Id);
	else				/* just the ID query */
	    (void) ajFmtPrintAppS(&seqin->Filename, "'|",
				  searchdb, qry->Id);
    }
    else if (ajStrLen(qry->Acc))
	(void) ajFmtPrintS(&seqin->Filename, "%S -e '[%S-acc:%S]'|",
			   qry->Application, searchdb, qry->Acc);
    else if (ajStrLen(qry->Sv))
      (void) ajFmtPrintS(&seqin->Filename,"%S -e '[%S-sv:%S]'|",
			 qry->Application, searchdb, qry->Sv);
    else if (ajStrLen(qry->Des))
	(void) ajFmtPrintS(&seqin->Filename, "%S -e '[%S-des:%S]'|",
			   qry->Application, searchdb, qry->Des);
    else if (ajStrLen(qry->Org))
	(void) ajFmtPrintS(&seqin->Filename, "%S -e '[%S-org:%S]'|",
			   qry->Application, searchdb, qry->Org);
    else if (ajStrLen(qry->Key))
	(void) ajFmtPrintS(&seqin->Filename, "%S -e '[%S-key:%S]'|",
			   qry->Application, searchdb, qry->Key);
    else
	(void) ajFmtPrintS(&seqin->Filename, "%S -e '%S'|",
			   qry->Application, searchdb);

    seqin->Filebuff = ajFileBuffNewIn (seqin->Filename);
    if (!seqin->Filebuff)
    {
	ajDebug ("unable to open file '%S'\n", seqin->Filename);
	return ajFalse;
    }
    (void) ajStrAssS (&seqin->Db, qry->DbName);

    ajStrDel (&searchdb);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqAccessSrsfasta **********************************************
**
** Reads sequence(s) using SRS. Opens a file using the results of an SRS
** query with FASTA format sequence output and returns to the caller to
** read the data.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSrsfasta (AjPSeqin seqin)
{
    static AjPStr searchdb = NULL;

    AjPSeqQuery qry = seqin->Query;

    if (!ajNamDbGetDbalias (qry->DbName, &searchdb))
	(void) ajStrAssS (&searchdb, qry->DbName);

    if (!ajStrLen (qry->Application))
	(void) ajStrAssC (&qry->Application, "getz");

    ajDebug ("seqAccessSrsfasta %S:%S\n", searchdb, qry->Id);
    if (ajStrLen(qry->Id))
    {
	(void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta [%S-id:%S]|",
			   qry->Application, searchdb, qry->Id);
	if (ajStrMatch(qry->Id, qry->Acc))
	    (void) ajFmtPrintAppS(&seqin->Filename, "[%S-acc:%S]|",
				  searchdb, qry->Id);
    }
    else if (ajStrLen(qry->Acc))
	(void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta [%S-acc:%S]|",
			   qry->Application, searchdb, qry->Acc);
    else if (ajStrLen(qry->Sv)) {
        (void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta [%S-sv:%S]|",
			   qry->Application, searchdb, qry->Sv);
    }
    else if (ajStrLen(qry->Des))
	(void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta [%S-des:%S]|",
			   qry->Application, searchdb, qry->Des);
    else if (ajStrLen(qry->Org))
	(void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta [%S-org:%S]|",
			   qry->Application, searchdb, qry->Org);
    else if (ajStrLen(qry->Key))
	(void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta [%S-key:%S]|",
			   qry->Application, searchdb, qry->Key);
    else
	(void) ajFmtPrintS(&seqin->Filename, "%S -d -sf fasta %S|",
			   qry->Application, searchdb);

    ajDebug ("searching with SRS command '%S'\n", seqin->Filename);
    seqin->Filebuff = ajFileBuffNewIn (seqin->Filename);
    if (!seqin->Filebuff)
    {
	ajDebug ("unable to open file '%S'\n", seqin->Filename);
	return ajFalse;
    }

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    ajStrDel (&searchdb);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqAccessSrswww ************************************************
**
** Reads sequence(s) using SRS. Sends a query to a remote SRS web server.
** Opens a file using the results and returns to the caller to
** read the data.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessSrswww (AjPSeqin seqin)
{
    static AjPRegexp urlexp = NULL;
    static AjPRegexp proxexp = NULL;

    static AjPStr url = NULL;
    static AjPStr host = NULL;
    static AjPStr port = NULL;
    static AjPStr urlget = NULL;
    static AjPStr get = NULL;
    ajint iport = 80;
    static AjPStr searchdb = NULL;
    static AjPStr errstr = NULL;
    struct sockaddr_in sin;
    struct hostent *hp;

    ajint sock;
    ajint status;
    FILE *fp;

    AjPStr proxyName=NULL;	/* host for proxy access.*/
    ajint proxyPort=0;		/* port for proxy axxess */
    AjPStr proxyStr=NULL;
    AjPStr proxy=NULL;		/* proxy from variable or query */
    AjPStr httpver=NULL;	/* HTTP version for GET */

    AjPSeqQuery qry = seqin->Query;

    if (!ajNamDbGetUrl (qry->DbName, &url))
    {
	ajErr ("no URL defined for database %S", qry->DbName);
	return ajFalse;
    }

    urlexp = ajRegCompC("^http://([a-z0-9.-]+)(:[0-9]+)?(.*)");
    if (!ajRegExec(urlexp, url))
    {
	ajErr ("invalid URL '%S' for database '%S'", url, qry->DbName);
	return ajFalse;
    }

    ajRegSubI(urlexp, 1, &host);

    ajRegSubI(urlexp, 2, &port);
    if (ajStrLen(port))
    {
	(void) ajStrTrim(&port, 1);
	(void) ajStrToInt (port, &iport);
    }
    ajRegSubI(urlexp, 3, &urlget);

    if (!ajNamDbGetDbalias (qry->DbName, &searchdb))
	(void) ajStrAssS (&searchdb, qry->DbName);

    ajDebug ("seqAccessSrswww %S:%S url: '%S'\n", searchdb, qry->Id, urlget);

    /* check for proxy definition */

    ajNamGetValueC ("proxy", &proxy);
    if (ajStrLen(qry->DbProxy))
      ajStrAssS (&proxy, qry->DbProxy);

    if (ajStrMatchC(proxy, ":"))
      ajStrAssC (&proxy, "");

    proxexp = ajRegCompC("^([a-z0-9.-]+):([0-9]+)$");
    if (ajRegExec (proxexp, proxy))
    {
	ajRegSubI(proxexp, 1, &proxyName);
	ajRegSubI(proxexp, 2, &proxyStr);
	(void) ajStrToInt (proxyStr, &proxyPort);
    }

    if (ajStrLen(proxyName))
      (void) ajFmtPrintS(&get, "GET http://%S:%d%S?-e+-ascii",
			 host, iport, urlget);
    else
      (void) ajFmtPrintS(&get, "GET %S?-e+-ascii", urlget);

    if (ajStrLen(qry->Id))
    {
	(void) ajFmtPrintAppS(&get, "+[%S-id:%S]",
			   searchdb, qry->Id);
	if (ajStrMatch(qry->Id, qry->Acc))
	    (void) ajFmtPrintAppS(&get, "|[%S-acc:%S]",
				  searchdb, qry->Id);
    }
    else if (ajStrLen(qry->Acc))
	(void) ajFmtPrintAppS(&get, "+[%S-acc:%S]",
			   searchdb, qry->Acc);
    else if (ajStrLen(qry->Sv)) {
      (void) ajFmtPrintAppS(&get,"+[%S-sv:%S]",
			 searchdb, qry->Sv);
    }
    else if (ajStrLen(qry->Des))
	(void) ajFmtPrintAppS(&get, "+[%S-des:%S]",
			   searchdb, qry->Des);
    else if (ajStrLen(qry->Org))
	(void) ajFmtPrintAppS(&get, "+[%S-org:%S]",
			   searchdb, qry->Org);
    else if (ajStrLen(qry->Key))
	(void) ajFmtPrintAppS(&get, "+[%S-key:%S]",
			   searchdb, qry->Key);
    else
	(void) ajFmtPrintAppS(&get, "+%S",
			   searchdb);

    /* check for HTTP version definition */

    ajNamGetValueC ("httpversion", &httpver);
    ajDebug ("httpver getValueC '%S'\n", httpver);

    if (ajStrLen(qry->DbHttpVer))
      ajStrAssS (&httpver, qry->DbHttpVer);
    ajDebug("httpver after qry '%S'\n", httpver);

    if (!ajStrLen(httpver))
      ajStrAssC (&httpver, "1.0");

    if (!ajStrIsFloat(httpver)) {
      ajWarn ("Invalid HTTPVERSION '%S', reset to 1.0", httpver);
      ajStrAssC(&httpver, "1.0");
    }
    ajDebug ("httpver final '%S'\n", httpver);

    ajDebug ("searching with SRS url '%S'\n", get);

    (void) ajFmtPrintAppS(&get, " HTTP/%S\n", httpver);
    ajStrDel(&httpver);

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    ajStrDel (&searchdb);

    ajDebug ("host '%S' port '%S' (%d) get '%S'\n", host, port, iport, get);

    if (ajStrLen(proxyName))
      hp = gethostbyname(ajStrStr(proxyName));
    else
      hp = gethostbyname(ajStrStr(host));

    if (!hp)
    {
	ajDebug ("Unable to get host '%S'\n", host);
	ajErr ("Unable to get host '%S' for database '%S'", host, qry->DbName);
	return ajFalse;
    }

    ajDebug ("creating socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
	ajDebug ("Socket create failed, sock: %d\n", sock);
	ajErr ("Socket create failed for database '%S'", qry->DbName);
	return ajFalse;
    }

    ajDebug ("setup socket data \n");
    sin.sin_family = AF_INET;

    if (ajStrLen(proxyName))	/* convert to short in network byte order */
      sin.sin_port = htons(proxyPort);
    else
      sin.sin_port = htons(iport);

#ifndef __VMS
    (void) memcpy (&sin.sin_addr, hp->h_addr, hp->h_length);
#endif
    ajDebug ("connecting to socket %d\n", sock);
    status = connect (sock, (struct sockaddr*) &sin, sizeof(sin));
    if (status < 0)
    {
	ajDebug ("socket connect failed, status: %d\n", status);
	ajFmtPrintS(&errstr, "socket connect failed for database '%S'",
		    qry->DbName);
	ajErr ("%S", errstr);
	perror(ajStrStr(errstr));
	return ajFalse;
    }

    ajDebug ("sending: '%S'\n", get);
    (void) send (sock, ajStrStr(get), ajStrLen(get), 0);

    /*
       (void) ajFmtPrintS(&get, "Accept: \n");
       ajDebug ("sending: '%S'\n", get);
       (void) send (sock, ajStrStr(get), ajStrLen(get), 0);

       (void) ajFmtPrintS(&get, "User-Agent: EMBOSS\n");
       ajDebug ("sending: '%S'\n", get);
       (void) send (sock, ajStrStr(get), ajStrLen(get), 0);
       */

    (void) ajFmtPrintS(&get, "Host: %S:%d\n", host, iport);
    ajDebug ("sending: '%S'\n", get);
    (void) send (sock, ajStrStr(get), ajStrLen(get), 0);

    (void) ajFmtPrintS(&get, "\n");
    ajDebug ("sending: '%S'\n", get);
    (void) send (sock, ajStrStr(get), ajStrLen(get), 0);

    fp = ajSysFdopen (sock, "r");
    if (!fp)
    {
	ajDebug ("socket open failed sock: %d\n", sock);
	ajErr ("socket open failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    seqin->Filebuff = ajFileBuffNewF(fp);
    if (!seqin->Filebuff)
    {
	ajDebug ("socket buffer attach failed\n");
	ajErr ("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    ajFileBuffLoad(seqin->Filebuff);


    if(!ajFileBuffStripSrs(seqin->Filebuff))
	ajFileBuffStripHtml(seqin->Filebuff);

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    ajStrDelReuse (&url);
    ajStrDelReuse (&host);
    ajStrDelReuse (&port);
    ajStrDelReuse (&get);
    ajStrDelReuse (&urlget);
    ajStrDel(&proxyName);
    ajStrDel(&proxyStr);
    ajStrDel(&proxy);
    ajStrDel(&httpver);
    ajRegFree (&urlexp);
    ajRegFree (&proxexp);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqCdQryReuse **************************************************
**
** Tests whether Cd index query data can be reused or whether we are finished.
**
** Clears qryData structure when finished.
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryReuse (AjPSeqQuery qry)
{
    SeqPCdQry qryd = qry->QryData;


    if(!qry || !qryd)
	return ajFalse;


/*    ajDebug ("qryd->list  %x\n",qryd->List);*/
    if (!qryd->List)
    {
	ajDebug ("query data all finished\n");
	AJFREE (qry->QryData);
	qryd = NULL;
	return ajFalse;
    }
    else
    {
	ajDebug ("reusing data from previous call %x\n", qry->QryData);
	ajDebug ("listlen  %d\n", ajListLength(qryd->List));
	ajDebug ("divfile '%S'\n", qryd->divfile);
	ajDebug ("idxfile '%S'\n", qryd->idxfile);
	ajDebug ("datfile '%S'\n", qryd->datfile);
	ajDebug ("seqfile '%S'\n", qryd->seqfile);
	ajDebug ("name    '%s'\n", qryd->name);
	ajDebug ("nameSize %d\n",  qryd->nameSize);
	ajDebug ("div      %d\n",  qryd->div);
	ajDebug ("maxdiv   %d\n",  qryd->maxdiv);
	ajDebug ("qryd->List length %d\n", ajListLength(qryd->List));
	/*ajListTrace (qryd->List);*/
    }

    return ajTrue;
}

/* @funcstatic seqCdQryOpen ***************************************************
**
** Opens everything for a new CD query
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryOpen (AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    ajint i;
    short j;
    static char *name;
    static AjPStr fullName = NULL;

    if (!ajStrLen(qry->IndexDir))
    {
	ajDebug ("no indexdir defined for database '%S'\n", qry->DbName);
	ajErr ("no indexdir defined for database '%S'", qry->DbName);
	return ajFalse;
    }

    ajDebug ("directory '%S' entry '%S' acc '%S'\n",
	     qry->IndexDir, qry->Id, qry->Acc);

    qry->QryData = AJNEW0 (qryd);
    qryd->List = ajListNew();
    AJNEW0(qryd->idxLine);
    AJNEW0(qryd->trgLine);
    qryd->dfp = seqCdFileOpen(qry->IndexDir, "division.lkp", &qryd->divfile);
    if (!qryd->dfp)
    {
	ajWarn("Cannot open division file '%S' for database '%S'",
	       qryd->divfile, qry->DbName);
	return ajFalse;
    }


    qryd->nameSize = qryd->dfp->RecSize - 2;
    qryd->maxdiv = qryd->dfp->NRecords;
    ajDebug ("nameSize: %d\n", qryd->nameSize);
    qryd->name = ajCharNewL (qryd->nameSize+1);
    name = ajCharNewL (qryd->nameSize+1);
    AJCNEW0(qryd->Skip, qryd->maxdiv);
    (void) seqCdFileSeek (qryd->dfp, 0);
    for (i=0; i < qryd->maxdiv; i++)
    {
	(void) seqCdFileReadShort (&j, qryd->dfp);
	(void) seqCdFileReadName (name, qryd->nameSize, qryd->dfp);

	(void) ajStrAssC(&fullName, name);
	ajFileNameDirSet (&fullName, qry->Directory);

	if (!ajFileTestSkip (fullName, qry->Exclude, qry->Filename,
			     ajTrue, ajTrue))
	    qryd->Skip[i] = ajTrue;
    }

    qryd->ifp = seqCdFileOpen(qry->IndexDir, "entrynam.idx", &qryd->idxfile);
    if (!qryd->ifp)
	ajFatal("Cannot open index file '%S'", qryd->idxfile);

    ajStrDelReuse (&fullName);
    ajCharFree (name);

    return ajTrue;
}

/* @funcstatic seqCdQryEntry **************************************************
**
** Queries for a single entry in an EMBLCD index
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryEntry (AjPSeqQuery qry)
{
    SeqPCdEntry entry = NULL;
    ajint ipos = -1;
    ajint trghit;

    SeqPCdQry qryd = qry->QryData;

    ajDebug ("entry id: '%S' acc: '%S'\n", qry->Id, qry->Acc);

    if (ajStrLen(qry->Id))
    {					/* search by ID */
	ipos = seqCdIdxSearch (qryd->idxLine, qry->Id, qryd->ifp);
	if (ipos >= 0)
	{
	    if (!qryd->Skip[qryd->idxLine->DivCode-1])
	    {
		AJNEW0(entry);
		entry->div = qryd->idxLine->DivCode;
		entry->annoff = qryd->idxLine->AnnOffset;
		entry->seqoff = qryd->idxLine->SeqOffset;
		ajListPushApp (qryd->List, (void*)entry);
	    }
	    else
		ajDebug("SKIP: '%S' [file %d]\n",
			qry->Id, qryd->idxLine->DivCode);
	}
    }

    if (ipos < 0 &&			/* if needed, search by accnum */
	ajStrLen(qry->Acc) &&
	seqCdTrgOpen (qry->IndexDir, "acnum",
		      &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch (qryd->trgLine, qry->Acc, qryd->trgfp);
	if (trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    (void) seqCdFileSeek (qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("acnum First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for (i = 0; i < qryd->trgLine->NHits; i++)
	    {
		(void) seqCdFileReadInt (&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		(void) seqCdIdxLine (qryd->idxLine, j, qryd->ifp);

		if (!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushApp (qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: accnum '%S' [file %d]\n",
			    qry->Acc, qryd->idxLine->DivCode);
	    }

	}
	(void) seqCdTrgClose (&qryd->trgfp, &qryd->hitfp);
	ajStrDel (&qryd->trgLine->Target);
    }

    /*
    ** if needed, search by SeqVersion
    */

    if (ipos < 0 &&
	ajStrLen(qry->Sv) &&
	seqCdTrgOpen (qry->IndexDir, "seqvn",
		      &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch (qryd->trgLine, qry->Sv, qryd->trgfp);
	if (trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    (void) seqCdFileSeek (qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("seqvn First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for (i = 0; i < qryd->trgLine->NHits; i++)
	    {
		(void) seqCdFileReadInt (&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		(void) seqCdIdxLine (qryd->idxLine, j, qryd->ifp);

		if (!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushApp (qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: seqvn '%S' [file %d]\n",
			    qry->Acc, qryd->idxLine->DivCode);
	    }

	}
	(void) seqCdTrgClose (&qryd->trgfp, &qryd->hitfp);
	ajStrDel (&qryd->trgLine->Target);
    }

    /*
    ** if needed, search by Description
    */

    if (ipos < 0 &&
	ajStrLen(qry->Des) &&
	seqCdTrgOpen (qry->IndexDir, "des",
		      &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch (qryd->trgLine, qry->Des, qryd->trgfp);
	if (trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    (void) seqCdFileSeek (qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("des First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for (i = 0; i < qryd->trgLine->NHits; i++)
	    {
		(void) seqCdFileReadInt (&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		(void) seqCdIdxLine (qryd->idxLine, j, qryd->ifp);

		if (!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushApp (qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: des '%S' [file %d]\n",
			    qry->Acc, qryd->idxLine->DivCode);
	    }

	}
	(void) seqCdTrgClose (&qryd->trgfp, &qryd->hitfp);
	ajStrDel (&qryd->trgLine->Target);
    }

    /*
    ** if needed, search by Keyword
    */

    if (ipos < 0 &&
	ajStrLen(qry->Key) &&
	seqCdTrgOpen (qry->IndexDir, "keyword",
		      &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch (qryd->trgLine, qry->Key, qryd->trgfp);
	if (trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    (void) seqCdFileSeek (qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("key First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for (i = 0; i < qryd->trgLine->NHits; i++)
	    {
		(void) seqCdFileReadInt (&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		(void) seqCdIdxLine (qryd->idxLine, j, qryd->ifp);

		if (!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushApp (qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: key '%S' [file %d]\n",
			    qry->Acc, qryd->idxLine->DivCode);
	    }

	}
	(void) seqCdTrgClose (&qryd->trgfp, &qryd->hitfp);
	ajStrDel (&qryd->trgLine->Target);
    }

    /*
    ** if needed, search by Taxonomy
    */

    if (ipos < 0 &&
	ajStrLen(qry->Org) &&
	seqCdTrgOpen (qry->IndexDir, "taxon",
		      &qryd->trgfp, &qryd->hitfp))
    {
	trghit = seqCdTrgSearch (qryd->trgLine, qry->Org, qryd->trgfp);
	if (trghit >= 0)
	{
	    ajint i;
	    ajint j;
	    (void) seqCdFileSeek (qryd->hitfp, qryd->trgLine->FirstHit-1);
	    ajDebug("tax First: %d Count: %d\n",
		    qryd->trgLine->FirstHit, qryd->trgLine->NHits);
	    ipos = qryd->trgLine->FirstHit;
	    for (i = 0; i < qryd->trgLine->NHits; i++)
	    {
		(void) seqCdFileReadInt (&j, qryd->hitfp);
		j--;
		ajDebug("hitlist[%d] entry = %d\n", i, j);
		(void) seqCdIdxLine (qryd->idxLine, j, qryd->ifp);

		if (!qryd->Skip[qryd->idxLine->DivCode-1])
		{
		    AJNEW0(entry);
		    entry->div = qryd->idxLine->DivCode;
		    entry->annoff = qryd->idxLine->AnnOffset;
		    entry->seqoff = qryd->idxLine->SeqOffset;
		    ajListPushApp (qryd->List, (void*)entry);
		}
		else
		    ajDebug("SKIP: tax '%S' [file %d]\n",
			    qry->Acc, qryd->idxLine->DivCode);
	    }

	}
	(void) seqCdTrgClose (&qryd->trgfp, &qryd->hitfp);
	ajStrDel (&qryd->trgLine->Target);
    }

    if (ipos < 0)
	return ajFalse;
    if (!ajListLength(qryd->List))
	return ajFalse;

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqCdQryQuery **************************************************
**
** Queries for one or more entries in an EMBLCD index
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryQuery (AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    qry->QryDone = ajTrue;

    if(ajStrLen(qry->Id))
    {
	if (seqCdIdxQuery (qry))
	{
	  qryd = qry->QryData;
	  ajListUnique(qryd->List, seqCdEntryCmp, seqCdEntryDel);
	  return ajTrue;
	}
    }
    if(ajStrLen(qry->Acc) ||
	    ajStrLen(qry->Sv) ||
	    ajStrLen(qry->Des) ||
	    ajStrLen(qry->Key) ||
	    ajStrLen(qry->Org))
    {
	if(!seqCdTrgQuery(qry))
	    return ajFalse;
	qryd = qry->QryData;
	ajListUnique(qryd->List, seqCdEntryCmp, seqCdEntryDel);
	return ajTrue;
    }


    return ajFalse;
}

/* @funcstatic seqCdEntryCmp **************************************************
**
** Compares two SeqPEntry objects
**
** @param [r] pa [const void*] SeqPEntry object
** @param [r] pb [const void*] SeqPEntry object
** @return [int] -1 if first entry should sort before second, +1 if the
**         second entry should sort first. 0 if they are identical
** @@
******************************************************************************/
static int seqCdEntryCmp (const void* pa, const void* pb) {

  SeqPCdEntry a = *(SeqPCdEntry*) pa;
  SeqPCdEntry b = *(SeqPCdEntry*) pb;

  ajDebug ("seqCdEntryCmp %x %d %d : %x %d %d\n",
	  a, a->div, a->annoff,
	  b, b->div, b->annoff);

  if (a->div != b->div)
    return (a->div - b->div);

  return (a->annoff - b->annoff);
}

/* @funcstatic seqCdEntryDel***************************************************
**
** Deletes a SeqPCdEntry object
**
** @param [r] pentry [void**] Address of a SeqPCdEntry object
** @param [r] cl [void*] Standard unused argument, usually NULL.
** @return [void]
** @@
******************************************************************************/
static void seqCdEntryDel (void** pentry, void* cl) {

  /* SeqPCdEntry a = (SeqPCdEntry) *pentry; */

  /*
  ajDebug ("seqCdEntryDel %x %x %d %d\n", pentry,
	  a, a->div, a->annoff);
  */

  AJFREE (*pentry);
  return;
}

/* @funcstatic seqCdQryNext ***************************************************
**
** Processes the next query for an EMBLCD index
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool seqCdQryNext (AjPSeqQuery qry)
{
    SeqPCdEntry entry;

    SeqPCdQry qryd = qry->QryData;
    void* item;

    if (!ajListLength(qryd->List))
	return ajFalse;

    ajDebug ("qryd->List (b) length %d\n", ajListLength(qryd->List));
    ajListTrace (qryd->List);
    (void) ajListPop (qryd->List, &item);
    entry = (SeqPCdEntry) item;

    ajDebug ("entry: %x div: %d (%d) ann: %d seq: %d\n",
	     entry, entry->div, qryd->div, entry->annoff, entry->seqoff);

    qryd->idnum = entry->annoff - 1;

    ajDebug ("idnum: %d\n", qryd->idnum);

    if (entry->div != qryd->div)
    {
	qryd->div = entry->div;
    ajDebug ("div: %d\n", qryd->div);
	seqCdQryFile (qry);
    }

    ajDebug ("Offsets (cd) %d %d\n", entry->annoff, entry->seqoff);
    ajDebug ("libr %x\n", qryd->libr);
    ajDebug ("libr %F\n", qryd->libr);

    ajFileSeek (qryd->libr, entry->annoff,0);
    if (qryd->libs)
	ajFileSeek (qryd->libs, entry->seqoff,0);

    AJFREE(entry);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqBlastQryNext ************************************************
**
** Processes the next query for an EMBLCD index for a Blast index
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool seqBlastQryNext (AjPSeqQuery qry)
{
    SeqPCdEntry entry;

    SeqPCdQry qryd = qry->QryData;
    void* item;

    if (!ajListLength(qryd->List))
	return ajFalse;

    ajDebug ("seqBlastQryNext qryd %x qryd->List (c) length: %d\n",
	     qryd, ajListLength(qryd->List));

    /* ajListTrace (qryd->List);*/

    (void) ajListPop (qryd->List, &item);
    entry = (SeqPCdEntry) item;

    ajDebug ("entry: %X div: %d (%d) ann: %d seq: %d\n",
	     entry, entry->div, qryd->div, entry->annoff, entry->seqoff);

    if (entry->div != qryd->div)
    {
	qryd->div = entry->div;
	seqBlastOpen (qry, ajFalse);		/* replaces qry->QryData */
	qryd = qry->QryData;
    }
    qryd->idnum = entry->annoff - 1;

    ajDebug ("Offsets (blast) %d %d [%d] qryd: %x\n",
	     entry->annoff, entry->seqoff, qryd->idnum, qryd);

    /* entry->annoff as qryd->idnum sets table position */

    AJFREE(entry);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqCdQryClose **************************************************
**
** Closes query data for an EMBLCD index
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue if we can continue,
**                  ajFalse if all is done.
** @@
******************************************************************************/

static AjBool seqCdQryClose (AjPSeqQuery qry)
{
    SeqPCdQry qryd = qry->QryData;

    ajDebug ("seqCdQryClose clean up qryd\n");
    (void) ajCharFree(qryd->name);
    ajStrDel (&qryd->divfile);
    ajStrDel (&qryd->idxfile);
    ajStrDel (&qryd->datfile);
    ajStrDel (&qryd->seqfile);
    ajStrDel (&qryd->idxLine->EntryName);

    seqCdFileClose (&qryd->ifp);
    seqCdFileClose (&qryd->dfp);
    ajListFree(&qryd->List);
    AJFREE(qryd->trgLine);
    AJFREE (qryd->idxLine);
    AJFREE (qryd->Skip);

    /* keep QryData for use at top of loop */

    return ajTrue;
}

/* @section GCG Database Indexing *********************************************
**
** These functions manage the GCG index access methods.
**
******************************************************************************/

/* @funcstatic seqAccessGcg ***************************************************
**
** Reads sequence(s) from a GCG formatted database, using EMBLCD index
** files. Returns with the file pointer set to the position in the
** sequence file.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessGcg (AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    static ajint qrycalled = 0;

    ajDebug ("seqAccessGcg type %d\n", qry->Type);

    if (qry->Type == QRY_ALL)
	return seqGcgAll (seqin);

    /* we need to search the index files and return a query */

    if (!qrycalled)
    {
	if (ajUtilBigendian())
	    seqCdReverse = ajTrue;
	qrycalled = 1;
    }

    if (qry->QryData)
    {					/* reuse unfinished query data */
	if (!seqCdQryReuse(qry))
	    return ajFalse;
    }
    else
    {
	seqin->Single = ajTrue;

	if (!seqCdQryOpen(qry))
	    ajFatal ("seqCdQryOpen failed");

	qryd = qry->QryData;
	seqin->Filebuff = ajFileBuffNew();

	/* binary search for the entryname we need */

	if (qry->Type == QRY_ENTRY)
	{
	    ajDebug ("entry id: '%S' acc: '%S'\n", qry->Id, qry->Acc);
	    if (!seqCdQryEntry (qry))
		ajDebug ("GCG Entry failed");
	}
	if (qry->Type == QRY_QUERY)
	{
	    ajDebug ("query id: '%S' acc: '%S'\n", qry->Id, qry->Acc);
	    if (!seqCdQryQuery (qry))
		ajDebug ("GCG Query failed");
	}
	AJFREE(qryd->trgLine);
    }

    if (ajListLength(qryd->List))
    {
	retval = seqCdQryNext (qry);
	if (retval)
	    seqGcgLoadBuff (seqin);
    }

    if (!ajListLength(qryd->List))
    {
	ajFileClose (&qryd->libr);
	ajFileClose (&qryd->libs);
	seqCdQryClose (qry);
    }

    if (retval)
      (void) ajStrAssS (&seqin->Db, qry->DbName);

    return retval;
}

/* @funcstatic seqGcgLoadBuff *************************************************
**
** Copies text data to a buffered file, and sequence data for an
** AjPSeqin internal data structure for reading later
**
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [void]
** @@
******************************************************************************/

static void seqGcgLoadBuff (const AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    if (!qry->QryData)
	ajFatal ("seqGcgLoadBuff Query Data not initialised");

    /* copy all the ref data */

    seqGcgReadRef(seqin);

    /* write the sequence (do we care about the format?) */
    seqGcgReadSeq(seqin);

    /* ajFileBuffTraceFull (seqin->Filebuff, 9999, 100); */

    if (!qryd->libr)
	ajFileClose (&qryd->libs);

    return;
}

/* @funcstatic seqGcgReadRef **************************************************
**
** Copies text data to a buffered file for reading later
**
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgReadRef (const AjPSeqin seqin)
{
    static AjPStr line = NULL;
    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;
    ajlong rpos;
    static AjPStr id = NULL;
    static AjPStr idc = NULL;
    static AjPRegexp idexp = NULL;
    AjBool ispir = ajFalse;
    AjBool continued=ajFalse;
    AjBool testcontinue=ajFalse;
    char *p=NULL;

    if (!idexp)
    {
	idexp =ajRegCompC("^>...([^ \n]+)");
    }

    if (!ajFileGets (qryd->libr, &line)) /* end of file */
	return ajFalse;

    if (ajStrChar(line, 0) != '>')	/* not start of entry */
	ajFatal("seqGcgReadRef bad entry start:\n'%S'", line);

    if (ajStrChar(line, 3) == ';')	/* PIR entry */
	ispir = ajTrue;

    if (ispir)
	ajFileBuffLoadS (seqin->Filebuff, line);

    if (ajRegExec(idexp, line))
    {
	continued = ajFalse;
	ajRegSubI (idexp, 1, &id);
	if(ajStrSuffixC(id,"_0") || ajStrSuffixC(id,"_00"))
	{
	    continued = ajTrue;
	    p = ajStrStr(id);
	    p = strrchr(p,(ajint)'_');
	    *(++p)='\0';
	    (void) ajStrFix(id);
	}
    }
    else
    {
      ajDebug("seqGcgReadRef bad ID line\n'%S'\n", line);
      ajFatal ("seqGcgReadRef bad ID line\n'%S'\n", line);
    }

    if (!ajFileGets (qryd->libr, &line)) /* blank desc line */
	return ajFalse;

    if (ispir)
	ajFileBuffLoadS (seqin->Filebuff, line);

    rpos = ajFileTell (qryd->libr);
    while (ajFileGets (qryd->libr, &line))
    {					/* end of file */
	if (ajStrChar(line, 0) == '>')
	{				/* start of next entry */

	  /* skip over split entries so it can be used for "all" */

	    if (continued)
	    {
	      testcontinue=ajTrue;
	      ajRegExec(idexp, line);
	      ajRegSubI (idexp, 1, &idc);

	      if(!ajStrPrefix(idc,id))
	      {
		ajFileSeek (qryd->libr, rpos, 0);
		return ajTrue;
	      }
	    }
	    else
	    {
	      ajFileSeek (qryd->libr, rpos, 0);
	      return ajTrue;
	    }
	}
	rpos = ajFileTell (qryd->libr);
	if (!testcontinue)
	{
	  ajStrSubstituteCC (&line, ". .", "..");
	  ajFileBuffLoadS (seqin->Filebuff, line);
	}
    }


    /* at end of file */

    ajFileClose (&qryd->libr);

    return ajTrue;
}

/* @funcstatic seqGcgReadSeq **************************************************
**
** Copies sequence data with a reformatted sequence to the "Inseq"
** data structure of the AjPSeqin object for later reuse.
**
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgReadSeq (const AjPSeqin seqin)
{
    static AjPStr line = NULL;
    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;
    static AjPRegexp idexp = NULL;
    static AjPRegexp contexp = NULL;
    static AjPRegexp idexp2 = NULL;
    static AjPStr gcgtype = NULL;
    static AjPStr tmpstr = NULL;
    static AjPStr dstr = NULL;
    static AjPStr id = NULL;
    static AjPStr idc = NULL;
    static AjPStr contseq=NULL;

    ajint gcglen;
    ajint pos;
    ajint rblock;
    ajlong spos;
    AjBool ispir = ajFalse;
    char *p=NULL;
    AjBool continued=ajFalse;

    if (!idexp)
    {
	idexp =ajRegCompC("^>...([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([0-9]+)");
	idexp2=ajRegCompC("^>[PF]1;([^ ]+)");
    }

    ajDebug("seqGcgReadSeq pos: %ld\n", ajFileTell(qryd->libs));

    if (!ajFileGets (qryd->libs, &line)) /* end of file */
	return ajFalse;

    ajDebug("test ID line\n'%S'\n", line);

    if (ajRegExec(idexp, line))
    {
	continued = ajFalse;
	ajRegSubI (idexp, 3, &gcgtype);
	ajRegSubI (idexp, 5, &tmpstr);
	ajRegSubI (idexp, 1, &id);
	if(ajStrSuffixC(id,"_0") || ajStrSuffixC(id,"_00"))
	{
	    continued = ajTrue;
	    p = ajStrStr(id);
	    p = strrchr(p,(ajint)'_');
	    *(++p)='\0';
	    (void) ajStrFix(id);
	    if(!contseq)
		contseq = ajStrNew();
	    if(!dstr)
		dstr = ajStrNew();
	}

	ajStrToInt (tmpstr, &gcglen);
    }
    else if (ajRegExec(idexp2, line))
    {
	(void) ajStrAssC(&gcgtype, "ASCII");
	ajRegSubI (idexp, 1, &tmpstr);
	ispir = ajTrue;
    }
    else
    {
	ajDebug("seqGcgReadSeq bad ID line\n'%S'\n", line);
	ajFatal("seqGcgReadSeq bad ID line\n'%S'\n", line);
	return ajFalse;
    }
    if (!ajFileGets (qryd->libs, &line)) /* desc line */
	return ajFalse;

    /* need to pick up the length and type, and read to the end of sequence */
    /* see fasta code to get a real sequence for this */

    /* also need to handle split entries and go find the rest */

    if (ispir)
    {
	spos = ajFileTell (qryd->libs);
	while (ajFileGets (qryd->libs, &line))
	{ /* end of file */
	    if (ajStrChar(line, 0) == '>')
	    {				/* start of next entry */
		ajFileSeek (qryd->libs, spos, 0);
		break;
	    }
	    spos = ajFileTell (qryd->libs);
	    ajFileBuffLoadS (seqin->Filebuff, line);
	}
    }
    else
    {
	ajStrModL (&seqin->Inseq, gcglen+3);
	rblock = gcglen;
	if (ajStrChar(gcgtype, 0) == '2')
	    rblock = (rblock+3)/4;

	if (!ajFileRead (ajStrStr(seqin->Inseq), 1, rblock, qryd->libs))
	  ajFatal ("error reading file %F", qryd->libs);

	if (ajStrChar(gcgtype, 0) == '2')
	{				/* convert 2bit to ascii */
	    seqGcgBinDecode (seqin->Inseq, gcglen);
	}
	else if (ajStrChar(gcgtype, 0) == 'A')
	{   /* are seq chars OK? */
	    ajStrFixI (seqin->Inseq, gcglen);
	}
	else
	{
	    ajRegSubI (idexp, 1, &tmpstr);
	    ajFatal ("Unknown GCG entry type '%S', entry name '%S'",
		     gcgtype, tmpstr);
	}
	if (!ajFileGets (qryd->libs, &line)) /* newline at end */
	  ajFatal ("error reading file %F", qryd->libs);

	if(continued)
	{
	    spos = ajFileTell (qryd->libs);
	    while(ajFileGets(qryd->libs,&line))
	    {
		ajRegExec(idexp, line);
		ajRegSubI (idexp, 5, &tmpstr);
		ajRegSubI (idexp, 1, &idc);

		if(!ajStrPrefix(idc,id))
		{
		  ajFileSeek (qryd->libs, spos, 0);
		  break;
		}

		ajStrToInt (tmpstr, &gcglen);
		if (!ajFileGets (qryd->libs, &dstr)) /* desc line */
		{
		  return ajFalse;
		}

		ajStrModL (&contseq, gcglen+3);

		rblock = gcglen;
		if (ajStrChar(gcgtype, 0) == '2')
		    rblock = (rblock+3)/4;

		if (!ajFileRead (ajStrStr(contseq), 1, rblock, qryd->libs))
		  ajFatal ("error reading file %F", qryd->libs);

		if (ajStrChar(gcgtype, 0) == '2')
		{			/* convert 2bit to ascii */
		    seqGcgBinDecode (contseq, gcglen);
		}
		else if (ajStrChar(gcgtype, 0) == 'A')
		{   /* are seq chars OK? */
		    ajStrFixI (contseq, gcglen);
		}
		else
		{
		    ajRegSubI (idexp, 1, &tmpstr);
		    ajFatal ("Unknown GCG entry: name '%S'",
			     tmpstr);
		}
		if (!ajFileGets (qryd->libs, &line)) /* newline at end */
		  ajFatal ("error reading file %F", qryd->libs);

		if(!contexp)
		    contexp = ajRegCompC("^([^ ]+) +([^ ]+) +([^ ]+) +"
					 "([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +"
					 "([^ ]+) +([0-9]+)");

		ajRegExec(contexp, dstr);
		ajRegSubI (contexp, 9, &tmpstr);
		ajStrToInt (tmpstr, &pos);
		seqin->Inseq->Len = pos-1;

		ajStrApp(&seqin->Inseq,contseq);
		spos = ajFileTell (qryd->libs);
	    }
	}

/*	ajFileGets (qryd->libs, &line); */
    }

    return ajTrue;
}

/* @funcstatic seqGcgBinDecode ************************************************
**
** Convert GCG binary to ASCII sequence.
**
** @param [r] thys [AjPStr] Binary string
** @param [r] sqlen [ajint] Expected sequence length
** @return [void]
** @@
******************************************************************************/

static void seqGcgBinDecode (AjPStr thys, ajint sqlen)
{
    char* seqp;
    char* cp;
    char* start = ajStrStr(thys);
    char* gcgbton="CTAG";
    char stmp;
    ajint rdlen = (sqlen+3)/4;

    seqp = start + rdlen;
    cp = start + 4*rdlen;

    ajDebug("seqp:%x start:%x cp:%x sqlen:%d len:%d size:%d (seqp-start):%d\n",
	    seqp, start, cp, sqlen,
	    ajStrLen(thys), ajStrSize(thys),
	    (seqp - start));

    while (seqp > start)
    {
	stmp = *--seqp;
	*--cp = gcgbton[stmp&3];
	*--cp = gcgbton[(stmp >>= 2)&3];
	*--cp = gcgbton[(stmp >>= 2)&3];
	*--cp = gcgbton[(stmp >>= 2)&3];
    }

    start[sqlen] = '\0';
    ajStrFixI (thys, sqlen);

    return;
}

/* @funcstatic seqGcgAll ******************************************************
**
** Opens the first or next GCG file for further reading
**
** @param [P] seqin [const AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqGcgAll (const AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    static ajint called = 0;

    if (!called)
    {
	if (ajUtilBigendian())
	    seqCdReverse = ajTrue;
	called = 1;
    }

    ajDebug ("seqGcgAll\n");

    if (!qry->QryData)
    {
	ajDebug ("seqGcgAll initialising\n");
	seqin->Single = ajTrue;
	if (!seqCdQryOpen(qry))
	    ajFatal ("seqGcgAll failed");

    }

    qryd = qry->QryData;
    seqin->Filebuff = ajFileBuffNew();

    if (!qryd->libr)
    {
        if (!seqCdDivNext(qry))
	{
	    ajDebug ("seqGcgAll finished\n");
	    return ajFalse;
	}
	if (!seqCdQryFile (qry))
	    ajFatal ("seqGcgAll out of data");
	ajDebug("seqCdQryOpen processing file %2d '%F'\n", qryd->div,
		qryd->libr);
	if (qryd->libs)
	    ajDebug("               sequence file    '%F'\n", qryd->libs);
    }
    seqGcgLoadBuff (seqin);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @section BLAST Database Indexing *******************************************
**
** These functions manage the BLAST index access methods.
**
******************************************************************************/

/* @funcstatic seqAccessBlast *************************************************
**
** Reads sequence(s) using BLAST index files. Returns with the file pointer
** set to the position in the sequence file.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessBlast (AjPSeqin seqin)
{
    AjBool retval = ajFalse;

    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    static ajint qrycalled = 0;

    ajDebug ("seqAccessBlast type %d\n", qry->Type);

    if (qry->Type == QRY_ALL)
	return seqBlastAll (seqin);

    /* we need to search the index files and return a query */

    if (!qrycalled)
    {
	if (ajUtilBigendian())
	    seqCdReverse = ajTrue;
	qrycalled = 1;
    }

    if (qry->QryData)
    {					/* reuse unfinished query data */
	if (!seqCdQryReuse(qry))
	    return ajFalse;
    }
    else
    {
	seqin->Single = ajTrue;

	if (!seqCdQryOpen(qry))		/* open the table file */
	    ajFatal ("seqCdQryOpen failed");

	qryd = qry->QryData;
	seqin->Filebuff = ajFileBuffNew();

	/* binary search for the entryname we need */

	if (qry->Type == QRY_ENTRY)
	{
	    ajDebug ("entry id: '%S' acc: '%S'\n", qry->Id, qry->Acc);
	    if (!seqCdQryEntry (qry))
		ajErr ("BLAST Entry failed");
	}
	if (qry->Type == QRY_QUERY)
	{
	    ajDebug ("query id: '%S' acc: '%S'\n", qry->Id, qry->Acc);
	    if (!seqCdQryQuery (qry))
		ajErr ("BLAST Query failed");
	}
	AJFREE(qryd->trgLine);
    }

    if (ajListLength(qryd->List))
    {
	retval = seqBlastQryNext (qry); /* changes qry->QryData */
	qryd = qry->QryData;
	if (retval)
	    seqBlastLoadBuff (seqin);
    }

    if (!ajListLength(qryd->List))
    {
	ajFileClose (&qryd->libr);
	ajFileClose (&qryd->libs);
	ajFileClose (&qryd->libt);
	ajFileClose (&qryd->libf);
	seqCdQryClose (qry);
    }

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    return retval;
}

/* @funcstatic seqBlastOpen ***************************************************
**
** Opens a blast database. The query object can specify protein or DNA type.
** The blast version (1 or 2) is derived from the table file name.
**
** @param [r] qry [AjPSeqQuery] Sequence query object
** @param [r] next [AjBool] Skip to next file (when reading all entries)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqBlastOpen (AjPSeqQuery qry, AjBool next)
{
    static char* seqext[] = {"bsq", "csq", "psq", "nsq"};
    static char* hdrext[] = {"ahd", "nhd", "phr", "nhr"};
    static char* tblext[] = {"atb", "ntb", "pin", "nin"};

    static AjPRegexp divexp = NULL;
    short j;
    AjBool isblast2 = ajFalse;
    AjBool isdna = ajFalse;
    ajint rdtmp=0;
    ajint rdtmp2=0;

    ajint DbType;				/* database type indicator */
    ajint DbFormat;			/* database format (version) indicator */
    ajint TitleLen;			/* length of database title */
    ajint DateLen;			/* length of database date string */
    ajint LineLen;			/* length of database lines */
    ajint HeaderLen;			/* bytes before tables start */
    ajint Size;				/* number of database entries */
    ajint CompLen;			/* length of compressed seq file */
    ajint MaxSeqLen;			/* max. entry length */
    ajint TotLen;				/* number of bases or residues in database */
    ajint CleanCount;			/* count of cleaned 8mers */
    static AjPStr Title;		/* database title */
    static AjPStr Date;			/* database date */

    SeqPCdQry qryd;
    AjBool bigend = ajTrue;		/* Blast indices are bigendian */

    if (!divexp)
	divexp = ajRegCompC("^([^ ]+)( +([^ ]+))?");

    if (!qry->QryData)
    {
	if (!seqCdQryOpen (qry))
	    ajFatal ("Blast database open failed");
    }

    if (next)
    {
	if (!seqCdDivNext(qry))	/* set qryd->div to next (included) file */
	  return ajFalse;
    }

    qryd = qry->QryData;

    qryd->type = 0;


    HeaderLen = 0;

    (void) seqCdFileSeek (qryd->dfp, (qryd->div - 1)); /* first (only) file */

    (void) seqCdFileReadShort (&j, qryd->dfp);
    (void) seqCdFileRead (qryd->name, qryd->nameSize, qryd->dfp);

    ajDebug ("div: %d namesize: %d name '%s'\n",
	     qryd->div, qryd->nameSize, qryd->name);

    if (!ajRegExecC (divexp, qryd->name))
    {
	ajWarn("index division file error '%s'", qryd->name);
	return ajFalse;
    }


    ajRegSubI (divexp, 1, &qryd->datfile);
    ajRegSubI (divexp, 3, &qryd->seqfile);
    ajDebug ("File(s) '%S' '%S'\n", qryd->datfile, qryd->seqfile);

    ajDebug("seqBlastOpen next: %B '%S' '%s'\n",
	    next, qryd->datfile, qryd->name);

    if (ajStrChar(qryd->datfile, -1) == 'b')
    {
	qryd->type = 0;			/* *tb : blast1 */
    }
    else
    {
	qryd->type = 2;			/* *in : blast2 */
	isblast2 = ajTrue;
    }

    if (ajStrMatchCaseC(qry->DbType, "N"))
    {
	qryd->type += 1;
	isdna = ajTrue;
    }

    (void) ajStrAssS (&qryd->srcfile, qryd->datfile);
    ajFileNameExtC (&qryd->srcfile, NULL);
    ajFmtPrintS (&qryd->seqfile, "%S.%s", qryd->srcfile, seqext[qryd->type]);
    ajFmtPrintS (&qryd->datfile, "%S.%s", qryd->srcfile, hdrext[qryd->type]);
    ajFmtPrintS (&qryd->tblfile, "%S.%s", qryd->srcfile, tblext[qryd->type]);

    qryd->libs = seqBlastFileOpen(qry->Directory, qryd->seqfile);
    qryd->libr = seqBlastFileOpen(qry->Directory, qryd->datfile);
    qryd->libt = seqBlastFileOpen(qry->Directory, qryd->tblfile);
    qryd->libf = seqBlastFileOpen(qry->Directory, qryd->srcfile);

    ajDebug("seqfile '%F'\n", qryd->libs);
    ajDebug("datfile '%F'\n", qryd->libr);
    ajDebug("tblfile '%F'\n", qryd->libt);
    ajDebug("srcfile '%F'\n", qryd->libf);

    /* read the first part of the table file and set up the offsets */

    DbType = ajFileReadUint (qryd->libt, bigend);
    DbFormat = ajFileReadUint (qryd->libt, bigend);
    HeaderLen += 8;
    ajDebug ("dbtype: %x dbformat: %x\n", DbType, DbFormat);

    TitleLen = ajFileReadUint(qryd->libt, bigend);
    if (isblast2)
	rdtmp = TitleLen;
    else
	rdtmp = TitleLen + ((TitleLen%4 !=0 ) ? 4-(TitleLen%4) : 0);

    (void) ajStrAssCL(&Title, "", rdtmp+1);
    ajDebug ("IsBlast2: %B title_len: %d rdtmp: %d title_str: '%S'\n",
	     isblast2, TitleLen, rdtmp, Title);
    ajStrTrace(Title);

    if (rdtmp)
    {
      if (!ajFileRead(ajStrStr(Title), (size_t)1, (size_t)rdtmp, qryd->libt))
	ajFatal ("error reading file %F", qryd->libt);
    }
    else
    {
      ajStrAssC(&Title, "");
    }

    if (isblast2)
	ajStrFixI (Title, TitleLen);
    else
	ajStrFixI (Title, TitleLen-1);

    ajDebug ("title_len: %d rdtmp: %d title_str: '%S'\n",
	     TitleLen, rdtmp, Title);

    HeaderLen += 4 + rdtmp;

    /* read the date - blast2 */

    if (isblast2)
    {
	DateLen = ajFileReadUint (qryd->libt, bigend);
	rdtmp2 = DateLen;
	(void) ajStrAssCL(&Date, "", rdtmp2+1);
	if (!ajFileRead (ajStrStr(Date),(size_t)1,(size_t)rdtmp2,qryd->libt))
	  ajFatal ("error reading file %F", qryd->libt);
	ajStrFixI (Date, DateLen);
	ajDebug ("datelen: %d rdtmp: %d date_str: '%S'\n",
		 DateLen, rdtmp2, Date);
	HeaderLen += 4 + rdtmp2;
    }

    /* read the rest of the header (different for protein and DNA) */

    if (isdna && !isblast2)
    {
	/* length of source lines */
	LineLen = ajFileReadUint (qryd->libt, bigend);
	HeaderLen += 4;
    }
    else
	LineLen = 0;

    /* all formats have the next 3 */

    Size = ajFileReadUint (qryd->libt, bigend);

    qryd->Size = Size;

    if (!isdna)
    {	/* mad, but they are the other way for DNA */
	TotLen = ajFileReadUint (qryd->libt, bigend);
	MaxSeqLen = ajFileReadUint (qryd->libt, bigend);
    }
    else
    {
	MaxSeqLen = ajFileReadUint (qryd->libt, bigend);
	TotLen = ajFileReadUint (qryd->libt, bigend);
    }

    HeaderLen += 12;

    if (isdna && !isblast2)
    {					/* Blast 1.4 DNA only */
	/* compressed db length */
	CompLen = ajFileReadUint (qryd->libt, bigend);
	/* count of nt's cleaned */
	CleanCount = ajFileReadUint (qryd->libt, bigend);
	HeaderLen += 8;
    }
    else
    {
	CompLen = 0;
	CleanCount = 0;
    }

    ajDebug(" size: %u, totlen: %d maxseqlen: %u\n",
	    Size, TotLen, MaxSeqLen);
    ajDebug(" linelen: %u, complen: %d cleancount: %d\n",
	    LineLen, CompLen, CleanCount);

    /* Now for the tables of offsets. Again maddeningly different in each */

    if (isblast2)
    {					/* NCBI BLAST 2.x */
	qryd->TopHdr = HeaderLen;	/* header first */
	qryd->TopCmp = qryd->TopHdr + (Size+1) * 4;
	if (isdna)
	    qryd->TopAmb = qryd->TopCmp + (Size+1) * 4; /* DNA only */
	else
	    qryd->TopAmb = 0;
    }
    else
    {					/* NCBI BLAST 1.x and WU-BLAST */
	qryd->TopCmp = HeaderLen + CleanCount*4;
	if (isdna)
	{
	    /* DNA, if FASTA file used */
	    qryd->TopSrc = qryd->TopCmp + (Size+1) * 4;
	    qryd->TopHdr = qryd->TopSrc + (Size+1) * 4;
	    qryd->TopAmb = qryd->TopHdr + (Size+1) * 4; /* DNA */
	}
	else
	{
	    qryd->TopSrc = 0;
	    qryd->TopHdr = qryd->TopCmp + (Size+1) * 4;
	    qryd->TopAmb = 0;
	}
    }

    ajDebug("table file csq    starts at %d\n", qryd->TopCmp);
    ajDebug("table file src    starts at %d\n", qryd->TopSrc);
    ajDebug("table file hdr    starts at %d\n", qryd->TopHdr);
    ajDebug("table file amb    starts at %d\n", qryd->TopAmb);

    return ajTrue;
}

/* @funcstatic seqCdDivNext ***************************************************
**
** Sets the division count to the next included file. We need the division
** file to be already open.
**
** @param [r] qry [AjPSeqQuery] sequence query object.
** @return [ajint] File number (starting at 1) or zero if all files are done.
** @@
******************************************************************************/

static ajint seqCdDivNext (AjPSeqQuery qry)
{

    SeqPCdQry qryd = qry->QryData;
    AjPStr fullName = NULL;
    ajint i;

    ajDebug("seqCdDivNext div: %d dfp: %x nameSize: %d name '%s'\n",
	    qryd->div, qryd->maxdiv);

    for (i=qryd->div; i < qryd->maxdiv; i++)
    {
        if (!qryd->Skip[i])
	{
	    qryd->div = i+1;
	    (void) ajDebug("next file is %d '%S'\n", qryd->div, fullName);
	    return qryd->div;
	}
	else {
	    (void) ajDebug("skip %d  '%S'\n", (i+1), fullName);
	}
    }

    return 0;
}


/* @funcstatic seqBlastLoadBuff ***********************************************
**
** Fill a buffered file with text data and preloads the sequence in
** an AjPSeqin data structure for reuse.
**
** @param [r] seqin [const AjPSeqin] AjPSeqin sequence input object.
** @return [AjBool] true if text data loaded.
** @@
******************************************************************************/

static AjBool seqBlastLoadBuff (const AjPSeqin seqin)
{
    static AjPStr hdrstr = NULL;
    static AjPStr seqstr = NULL;

    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    if (!qry->QryData)
	ajFatal ("seqBlastLoadBuff Query Data not initialised");

    ajDebug ("seqBlastLoadBuff libt: %F %d\n", qryd->libt, qryd->idnum);

    return seqBlastReadTable(seqin, &hdrstr, &seqstr);
}

/* @funcstatic seqBlastAll ****************************************************
**
** Reads the EMBLCD division lookup file and opens a list of all the
** database files for plain reading.
**
** @param [P] seqin [const AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqBlastAll (const AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;

    static ajint called = 0;

    if (!called)
    {
	if (ajUtilBigendian())
	    seqCdReverse = ajTrue;
	called = 1;
    }

    ajDebug ("seqBlastAll\n");

    if (!qry->QryData)
    {
	ajDebug ("seqBlastAll initialising\n");
	seqin->Single = ajTrue;
	if (!seqBlastOpen(qry, ajTrue))		/* replaces qry->QryData */
	    ajFatal ("seqBlastAll failed");

	qryd = qry->QryData;
	seqin->Filebuff = ajFileBuffNew();
	qryd->idnum = 0;
    }
    else {
	qryd = qry->QryData;
    }

    if (!qryd->libr)
    {
	ajDebug ("seqBlastAll finished\n");
	return ajFalse;
    }

    if (!seqBlastLoadBuff (seqin))
    {
	if (!seqBlastOpen(qry, ajTrue))		/* try the next file */
	  return ajFalse;
	qryd = qry->QryData;
	qryd->idnum = 0;
	if (!qryd->libr)
	{
	    ajDebug ("seqBlastAll finished\n");
	    return ajFalse;
        }
	if (!seqBlastLoadBuff (seqin))
	  return ajFalse;

    }

    qryd->idnum++;
    (void) ajStrAssS (&seqin->Db, qry->DbName);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @funcstatic seqCdQryFile ***************************************************
**
** Opens a specific file number for an EMBLCD index
**
** @param [r] qry [AjPSeqQuery] Query data
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqCdQryFile (AjPSeqQuery qry)
{
    SeqPCdQry qryd;

    short j;
    static AjPRegexp divexp = NULL;

    if (!divexp)
	divexp = ajRegCompC("^([^ ]+)( +([^ ]+))?");

    ajDebug ("seqCdQryFile qry %x\n",qry);
    qryd = qry->QryData;
    ajDebug ("seqCdQryFile qryd %x\n",qryd);
    ajDebug ("seqCdQryFile %F\n",qryd->dfp->File);

    (void) seqCdFileSeek (qryd->dfp, (qryd->div - 1));

    /* note - we must not use seqCdFileReadName - we need spaces for GCG */

    (void) seqCdFileReadShort (&j, qryd->dfp);

    (void) seqCdFileRead (qryd->name, qryd->nameSize, qryd->dfp);
    ajDebug ("DivCode: %d, code: %2hd '%s'\n",
	     qryd->div, j, qryd->name);

    /**(void) ajCharToLower(qryd->name);**/
    if (!ajRegExecC (divexp, qryd->name))
	ajFatal ("index division file error '%S'", qryd->name);

    ajRegSubI (divexp, 1, &qryd->datfile);
    ajRegSubI (divexp, 3, &qryd->seqfile);
    ajDebug ("File(s) '%S' '%S'\n", qryd->datfile, qryd->seqfile);

    ajFileClose(&qryd->libr);
    qryd->libr = ajFileNewDF (qry->Directory, qryd->datfile);
    if (!qryd->libr)
	ajFatal("Cannot open database file '%S'", qryd->datfile);
    if (ajStrLen(qryd->seqfile))
    {
        ajFileClose(&qryd->libs);
	qryd->libs = ajFileNewDF (qry->Directory, qryd->seqfile);
	if (!qryd->libs)
	    ajFatal("Cannot open sequence file '%S'", qryd->seqfile);
    }
    else
	qryd->libs = NULL;

    return ajTrue;
}

/* @section Remote URL Database Access ****************************************
**
** These functions manage the remote URL database access methods.
**
******************************************************************************/

/* @funcstatic seqAccessUrl ***************************************************
**
** Reads sequence(s) using a remote URL. Reads the results into a buffer
** and strips out HTML before returning to the caller.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessUrl (AjPSeqin seqin)
{
    struct sockaddr_in sin;
    struct hostent *hp;
    ajint sock;
    ajint status;
    FILE *fp;

    ajint proxyPort=0;		/* port for proxy axxess */
    AjPStr proxyName=NULL;	/* host for proxy access.*/
    AjPStr proxyStr=NULL;
    AjPStr proxy=NULL;		/* proxy from variable or query */
    AjPStr httpver=NULL;	/* HTTP version 1.0, 1.1, ... */

    AjPSeqQuery qry = seqin->Query;

    static AjPRegexp urlexp = NULL;
    static AjPRegexp proxexp = NULL;

    static AjPStr url = NULL;
    static AjPStr host = NULL;
    static AjPStr port = NULL;
    static AjPStr urlget = NULL;
    static AjPStr get = NULL;
    static AjPStr gethead = NULL;
    static AjPStr errstr = NULL;
    ajint iport = 80;
    ajint ipos;

    ajNamGetValueC ("proxy", &proxy);
    if (ajStrLen(qry->DbProxy))
      ajStrAssS (&proxy, qry->DbProxy);

    if (ajStrMatchC(proxy, ":"))
      ajStrAssC (&proxy, "");

    ajNamGetValueC ("httpversion", &httpver);
    ajDebug ("httpver getValueC '%S'\n", httpver);

    if (ajStrLen(qry->DbHttpVer))
      ajStrAssS (&httpver, qry->DbHttpVer);
    ajDebug("httpver after qry '%S'\n", httpver);

    if (!ajStrLen(httpver))
      ajStrAssC (&httpver, "1.0");

    if (!ajStrIsFloat(httpver)) {
      ajWarn ("Invalid HTTPVERSION '%S', reset to 1.0", httpver);
      ajStrAssC(&httpver, "1.0");
    }
    ajDebug("httpver final: '%S'\n", httpver);

    proxexp = ajRegCompC("^([a-z0-9.-]+):([0-9]+)$");
    if (ajRegExec (proxexp, proxy))
    {
	ajRegSubI(proxexp, 1, &proxyName);
	ajRegSubI(proxexp, 2, &proxyStr);
	(void) ajStrToInt (proxyStr, &proxyPort);
    }

    if (!ajNamDbGetUrl (qry->DbName, &url))
    {
	ajErr ("no URL defined for database %S", qry->DbName);
	return ajFalse;
    }

    urlexp = ajRegCompC("^http://([a-z0-9.-]+)(:[0-9]+)?(.*)");
    if (!ajRegExec(urlexp, url))
    {
	ajErr ("invalid URL '%S' for database %S", url, qry->DbName);
	return ajFalse;
    }

    ajRegSubI(urlexp, 1, &host);

    ajRegSubI(urlexp, 2, &port);
    if (ajStrLen(port))
    {
	(void) ajStrTrim(&port, 1);
	(void) ajStrToInt (port, &iport);
    }
    ajRegSubI(urlexp, 3, &urlget);

    if (ajStrLen(proxyName))
      (void) ajFmtPrintS(&get, "GET http://%S:%d%S HTTP/%S\n",
			 host, iport, urlget, httpver);
    else
      (void) ajFmtPrintS(&get, "GET %S HTTP/%S\n", urlget, httpver);

    ipos = ajStrFindC(get, "%s");
    while (ipos >= 0)
    {
	ajDebug ("get '%S' qryid '%S'\n", get, qry->Id);
	(void) ajFmtPrintS(&urlget, ajStrStr(get), ajStrStr(qry->Id));
	(void) ajStrAssS (&get, urlget);
	ipos = ajStrFindC(get, "%s");
    }

    ajDebug ("host '%S' port '%S' (%d) get '%S'\n", host, port, iport, get);

    if (ajStrLen(proxyName))
      hp = gethostbyname(ajStrStr(proxyName));
    else
      hp = gethostbyname(ajStrStr(host));

    if (!hp)
    {
	ajErr ("Unable to get host '%S' for database '%S'", host, qry->DbName);
	return ajFalse;
    }

    ajDebug ("creating socket\n");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
	ajErr ("Socket create failed for database '%S'", qry->DbName);
	return ajFalse;
    }


    ajDebug ("setup socket data \n");
    sin.sin_family = AF_INET;

    if (ajStrLen(proxyName))	/* convert to short in network byte order */
      sin.sin_port = htons(proxyPort);
    else
      sin.sin_port = htons(iport);

#ifndef __VMS
    (void) memcpy (&sin.sin_addr, hp->h_addr, hp->h_length);
#endif
    ajDebug ("connecting to socket %d\n\n");
    status = connect (sock, (struct sockaddr*) &sin, sizeof(sin));
    if (status < 0)
    {
	ajDebug ("socket connect failed, status: %d\n", status);
	ajFmtPrintS(&errstr, "socket connect failed for database '%S'",
		    qry->DbName);
	ajErr ("%S", errstr);
	perror(ajStrStr(errstr));
	return ajFalse;
    }


    ajDebug ("sending: '%S'\n", get); /* the URL we want */
    (void) send (sock, ajStrStr(get), ajStrLen(get), 0);

    /*
       (void) ajFmtPrintS(&gethead, "Accept: \n");
       ajDebug ("sending: '%S'\n", gethead);
       (void) send (sock, ajStrStr(gethead), ajStrLen(gethead), 0);

       (void) ajFmtPrintS(&gethead, "User-Agent: EMBOSS\n");
       ajDebug ("sending: '%S'\n", gethead);
       (void) send (sock, ajStrStr(gethead), ajStrLen(gethead), 0);
    */



    (void) ajFmtPrintS(&gethead, "Host: %S:%d\n", host, iport);
    ajDebug ("sending: '%S'\n", gethead);
    (void) send (sock, ajStrStr(gethead), ajStrLen(gethead), 0);

    (void) ajFmtPrintS(&gethead, "\n");
    ajDebug ("sending: '%S'\n", gethead);
    (void) send (sock, ajStrStr(gethead), ajStrLen(gethead), 0);

    fp = ajSysFdopen (sock, "r");
    if (!fp)
    {
	ajDebug ("socket open failed sock: %d\n", sock);
	ajErr ("socket open failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    seqin->Filebuff = ajFileBuffNewF(fp);
    if (!seqin->Filebuff)
    {
	ajDebug ("socket buffer attach failed\n");
	ajErr ("socket buffer attach failed for database '%S'", qry->DbName);
	return ajFalse;
    }
    ajFileBuffLoad(seqin->Filebuff);


    if(!ajFileBuffStripSrs(seqin->Filebuff))
	ajFileBuffStripHtml(seqin->Filebuff);

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    ajStrDelReuse (&url);
    ajStrDelReuse (&host);
    ajStrDelReuse (&port);
    ajStrDelReuse (&get);
    ajStrDelReuse (&urlget);
    ajStrDel(&proxyName);
    ajStrDel(&proxyStr);
    ajStrDel(&proxy);
    ajStrDel(&httpver);
    ajRegFree (&urlexp);
    ajRegFree (&proxexp);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @section Application Database Access ***************************************
**
** These functions manage the application database access methods.
**
******************************************************************************/

/* @funcstatic seqAccessApp ***************************************************
**
** Reads sequence data using an application which can accept a specification
** in the form "database:entry" such as Erik Sonnhammer's 'efetch'.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessApp (AjPSeqin seqin)
{
    static AjPStr pipename = NULL;

    AjPSeqQuery qry = seqin->Query;

    if (!ajStrLen(qry->Application))
    {
	ajErr ("APP access: application not defined for %S", qry->DbName);
	return ajFalse;
    }

    ajDebug ("seqAccessApp '%S' dbname '%S'\n", qry->Application, qry->DbName);

    if (ajStrMatchWildC(qry->Application, "*%s*"))
    {
	if (ajStrLen(qry->Id))
	    (void) ajFmtPrintS(&pipename, ajStrStr(qry->Application),
			       ajStrStr(qry->Id));
	else if (ajStrLen(qry->Acc))
	    (void) ajFmtPrintS(&pipename, ajStrStr(qry->Application),
			       ajStrStr(qry->Acc));
	ajStrAppC (&pipename, " |");
    }
    else
    {
	if (ajStrLen(qry->Id))
	    (void) ajFmtPrintS(&pipename, "%S %S:%S|",
			       qry->Application, qry->DbName, qry->Id);
	else if (ajStrLen(qry->Acc))
	    (void) ajFmtPrintS(&pipename, "%S %S:%S|",
			       qry->Application, qry->DbName, qry->Acc);
    }

    if (!ajStrLen(pipename))
    {
	ajErr ("APP access: bad query format");
	return ajFalse;
    }


    seqin->Filebuff = ajFileBuffNewIn (pipename);
    if (!seqin->Filebuff)
    {
	ajErr ("unable to open file '%S'", pipename);
	ajStrDel (&pipename);
	return ajFalse;
    }

    (void) ajStrAssS (&seqin->Db, qry->DbName);

    ajStrDel (&pipename);

    qry->QryDone = ajTrue;

    return ajTrue;
}

/* @section ASIS Sequence Access **********************************************
**
** These functions manage the ASIS sequence access methods.
**
******************************************************************************/

/* @func ajSeqAccessAsis ******************************************************
**
** Reads a sequence using the 'filename' as the sequence data.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessAsis (AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;

    if (!ajStrLen(qry->Filename))
    {
	ajErr ("ASIS access: no sequence");
	return ajFalse;
    }

    ajDebug ("ajSeqAccessAsis %S\n", qry->Filename);

    seqin->Filebuff = ajFileBuffNewS (qry->Filename);
    if (!seqin->Filebuff)
    {
	ajDebug ("Asis access: unable to use sequence '%S'\n", qry->Filename);
	return ajFalse;
    }
    (void) ajStrAssC (&seqin->Filename, "asis");
    /*ajFileBuffTrace (seqin->Filebuff);*/

    return ajTrue;
}

/* @section File Access *******************************************************
**
** These functions manage the sequence file access methods.
**
******************************************************************************/

/* @func ajSeqAccessFile ******************************************************
**
** Reads a sequence from a named file.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessFile (AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;

    if (!ajStrLen(qry->Filename))
    {
	ajErr ("FILE access: no filename");
	return ajFalse;
    }

    ajDebug ("ajSeqAccessFile %S\n", qry->Filename);

    /* ajStrTraceT (qry->Filename, "qry->Filename (before):"); */

    seqin->Filebuff = ajFileBuffNewIn (qry->Filename);
    if (!seqin->Filebuff)
    {
	ajDebug ("FILE access: unable to open file '%S'\n", qry->Filename);
	return ajFalse;
    }

    /* ajStrTraceT (seqin->Filename, "seqin->Filename:"); */
    /* ajStrTraceT (qry->Filename, "qry->Filename (after):"); */

    (void) ajStrAssS (&seqin->Filename, qry->Filename);

    return ajTrue;
}

/* @func ajSeqAccessOffset ****************************************************
**
** Reads a sequence from a named file, at a given offset within the file.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAccessOffset (AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;

    if (!ajStrLen(qry->Filename))
    {
	ajErr ("FILE access: no filename");
	return ajFalse;
    }

    ajDebug ("ajSeqAccessOffset %S %ld\n", qry->Filename, qry->Fpos);

    /* ajStrTraceT (qry->Filename, "qry->Filename (before):"); */
    seqin->Filebuff = ajFileBuffNewIn (qry->Filename);
    if (!seqin->Filebuff)
    {
	ajDebug ("OFFSET access: unable to open file '%S'\n", qry->Filename);
	return ajFalse;
    }
    ajFileSeek (ajFileBuffFile(seqin->Filebuff), qry->Fpos, 0);
    /* ajStrTraceT (seqin->Filename, "seqin->Filename:"); */
    /* ajStrTraceT (qry->Filename, "qry->Filename (after):"); */
    (void) ajStrAssS (&seqin->Filename, qry->Filename);

    return ajTrue;
}

/* @section File Direct Access ************************************************
**
** These functions manage the sequence file direct access methods.
**
******************************************************************************/

/* @funcstatic seqAccessDirect ************************************************
**
** Reads a sequence from a database which may have multiple files.
** The sequence input object holds a directory name and a (wildcard)
** file specification.
**
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqAccessDirect (AjPSeqin seqin)
{
    AjPSeqQuery qry = seqin->Query;

    ajDebug ("seqAccessDirect %S\n", qry->DbName);

    if (!ajStrLen(qry->Filename))
    {
	ajErr ("DIRECT access: filename not specified");
	return ajFalse;
    }

    ajDebug ("Try to open %S%S.seq\n", qry->Directory, qry->Filename);

    seqin->Filebuff = ajFileBuffNewDW (qry->Directory, qry->Filename);
    if (!seqin->Filebuff)
    {
	ajDebug ("DIRECT access: unable to open file '%S/%S'\n",
		 qry->Directory, qry->Filename);
	return ajFalse;
    }

    (void) ajStrAssS (&seqin->Db, qry->DbName);
    (void) ajStrAssS (&seqin->Filename, qry->Filename);

    return ajTrue;
}

/* @funcstatic seqBlastReadTable **********************************************
**
** Read one entry in the BLAST binary table into memory, and
** load the header and sequence for it. All that is needed is
** a set of open blast files (in qryd) and an idnum set.
**
** @param [r] seqin [const AjPSeqin] Sequence input.
** @param [P] hline [AjPStr*] header line.
** @param [P] sline [AjPStr*] sequence line.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqBlastReadTable (const AjPSeqin seqin, AjPStr* hline,
				 AjPStr* sline)
{
    ajint ipos;
    ajint start;
    ajint end;
    ajint hsize;
    ajint seq_len = -1;
    ajint seqcnt = -1;
    char* sptr;
    ajint c_len;
    ajint a_len;
    ajint astart=0;
    ajint fstart=0;
    ajint fend=0;
    ajint i;
    ajint j;
    size_t tmp;
    char* btoa;
    static ajint c_pad;
    char* tptr;
    ajint s_chunk;
    char stmp;
    unsigned char tmpbyte;
    static char bases[] = "NACGT";
    static char abases[] = "NACMGRSVTWYHKDBN";
    ajint spos;
    ajint apos;
    ajuint bc;
    ajuint bn;
    ajuint ip;
    ajuint iamb;
    ajuint ui;
    static AjPStr rdline=NULL;
    ajint nbpn = 2;
    ajint char_bit = 8;
    unsigned char nt_magic_byte = 0xfc;
    ajint nsentinels = 2;
    char* seq=NULL;

    AjPSeqQuery qry = seqin->Query;
    SeqPCdQry qryd = qry->QryData;
    AjBool bigend = ajTrue;

    ipos = qryd->idnum;

    ajDebug("seqBlastReadTable %d\n", ipos);

    if (qryd->idnum >= qryd->Size)
    {
	ajDebug ("beyond end of database\n");
	return ajFalse;
    }

    /* find the table record and read the positions we want */

    /* find the header record and read it */

    /* find the sequence in the binary or FASTA file and read it */

    ajFileSeek (qryd->libt, qryd->TopHdr + 4*(qryd->idnum), 0);
    ajDebug ("hdr reading at %d\n", ajFileTell(qryd->libt));
    start = ajFileReadUint (qryd->libt, bigend);
    ajDebug ("hdr read i: %d value: %d\n", qryd->idnum, start);
    end = ajFileReadUint (qryd->libt, bigend);
    ajDebug ("hdr read i: %d value: %d\n", qryd->idnum, end);

    if (end)
	hsize = end - start;
    else
	hsize = qryd->Size - start;

    (void) ajStrAssCL (hline, "", hsize+1);

    ajDebug ("type: %d hsize: %d start: %d end: %d dbSize: %d\n",
	     qryd->type, hsize, start, end, qryd->Size);

    ajFileSeek (qryd->libr, start, 0);
    if (!ajFileRead (ajStrStr(*hline), 1, hsize, qryd->libr))
	ajFatal ("error reading file %F", qryd->libr);
    ajStrFixI (*hline, hsize);


    if (qryd->type >= 2)
	seqBlastStripNcbi (hline);	/* trim the gnl| prefix */
    /* The above now just adds a > */

    ajFileBuffClear(seqin->Filebuff, -1); /* delete all lines */

    ajDebug ("Load FASTA file with '%S'\n", *hline);
    ajFileBuffLoadS (seqin->Filebuff, *hline);

    ajDebug ("\n** Blast Sequence Reading **\n");

    ajFileSeek (qryd->libt, qryd->TopCmp + 4*(qryd->idnum), 0);
    ajDebug ("seq reading at %d\n", ajFileTell(qryd->libt));
    start = ajFileReadUint (qryd->libt, bigend);
    ajDebug ("seq read i: %d start: %d\n", qryd->idnum, start);
    end = ajFileReadUint (qryd->libt, bigend);
    ajDebug ("seq read i: %d   end: %d\n", qryd->idnum, end);

    if (qryd->type == 1 && qryd->libf)
    {					/* BLAST 1 FASTA file */
	ajFileSeek (qryd->libt, qryd->TopSrc + 4*(qryd->idnum), 0);
	ajDebug ("src reading at %d\n", ajFileTell(qryd->libt));
	fstart = ajFileReadUint (qryd->libt, bigend);
	ajDebug ("src read i: %d fstart: %d\n", qryd->idnum, fstart);
	fend = ajFileReadUint (qryd->libt, bigend);
	ajDebug ("src read i: %d   fend: %d\n", qryd->idnum, fend);
    }

    if (qryd->type == 3)
    {					/* BLAST 2 DNA ambuguities */
	ajFileSeek (qryd->libt, qryd->TopAmb + 4*(qryd->idnum), 0);
	ajDebug ("amb reading at %d\n", ajFileTell(qryd->libt));
	astart = ajFileReadUint (qryd->libt, bigend);
	ajDebug ("amb read i: %d astart: %d\n", qryd->idnum, astart);
    }

    switch (qryd->type)
    {
    case 0:				/* protein 1 */
    case 2:				/* protein 2 */
	ajDebug ("reading protein sequence file\n");
	if (qryd->type == 2)
	    btoa = aa_btoa2;
	else
	    btoa = aa_btoa;

	spos = start;
	ajFileSeek(qryd->libs,spos-1,0);

	seq_len = end - start - 1;
	ajDebug ("seq_len: %d spos: %d %x\n", seq_len, spos, spos);

	(void) ajStrAssCL (sline, "", seq_len+1);

	if (!ajFileRead(&tmpbyte, 1, 1, qryd->libs)) /* skip the null byte */
	  ajFatal ("error reading file %F", qryd->libs);
	if (tmpbyte)
	    ajErr(" phase error: %d:%d found\n",qryd->idnum,(ajint)tmpbyte);

	if ((tmp=ajFileRead(ajStrStr(*sline),(size_t)1,(size_t)seq_len,
			    qryd->libs)) != (size_t)seq_len)
	{
	    ajErr(" could not read sequence record (a): %d %d != %d\n",
		  start,tmp,seq_len);
	    ajErr(" error reading seq at %d\n",start);
	    return ajFalse;
	}
	if (btoa[(ajint)ajStrChar(*sline, -1)] =='*')
	{				/* skip * at end */
	    seqcnt = seq_len-1;
	    ajStrTrim (sline, -1);
	}
	else seqcnt=seq_len;

	seq = ajStrStr(*sline);
	sptr = seq+seqcnt;

	while (--sptr >= seq)
	    *sptr = btoa[(ajint)*sptr];

	ajStrFixI (*sline, seqcnt);
	ajDebug ("Read sequence %d %d\n'%S'\n", seqcnt, ajStrLen(*sline),
		 *sline);
	(void) ajStrAssS (&seqin->Inseq, *sline);
	return ajTrue;


    case 3:				/* DNA 2 */
	ajDebug ("reading blast2 DNA file\n");
	ajFileSeek(qryd->libs,start,0);
	spos = (start)/(char_bit/nbpn);
	c_len = astart - start;	/* we have ambiguities in the nsq file */
	seq_len = c_len*4;

	ajDebug ("c_len %d spos %d seq_len %d\n",
		 c_len, spos, seq_len);

	(void) ajStrAssCL (sline, "", seq_len+1);

	seq = ajStrStr(*sline);

	/* read the sequence here */

	seqcnt = c_len;
	if ((tmp=ajFileRead(ajStrStr(*sline),(size_t)1,(size_t)seqcnt,
			    qryd->libs)) != (size_t)seqcnt)
	{
	    ajErr (" could not read sequence record (c): %d %d != %d: %d\n",
		   qryd->idnum,tmp,seqcnt,*seq);
	    exit (0);
	}
	sptr = seq + seqcnt;

	/* the last byte is either '0' (no remainder) or the last 1-3
	   chars and the remainder */

	c_pad = *(sptr-1);
	c_pad &= 0x3;		/* get the last (low) 2 bits */
	seq_len -= (4 - c_pad);	/* if the last 2 bits are 0, its a NULL byte */
	ajDebug ("(a) c_pad %d seq_len %d seqcnt %d\n",
		 c_pad, seq_len, seqcnt);

	/* point to the last packed byte and to the end of the array
	   seqcnt is the exact number of bytes read tptr points to the
	   destination, use multiple of 4 to simplify math sptr points
	   to the source, note that the last byte will be read 4 cycles
	   before it is written
	   */

	tptr = seq + 4*seqcnt;
	s_chunk = seqcnt/8;

	ajDebug ("sptr +%d tptr +%d s_chunk %d\n",
		 sptr-seq, tptr-seq, s_chunk);

	/* do we need this first section or is it just for parallel code? */

	while (s_chunk-- > 0)
	{
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	}

	ajDebug ("after: sptr +%d tptr +%d\n",
		 sptr-seq, tptr-seq);

	while (tptr>seq)
	{
	    stmp = *--sptr;
	    *--tptr = bases[(stmp&3) +1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	    *--tptr = bases[((stmp >>= 2)&3)+1];
	}

	if (astart < end)
	{				/* read ambiguities */
	    a_len = end - astart;
	    apos = astart;
	    ajDebug ("Read ambiguities: a_len %d apos: %d\n", a_len, apos);
	    ajFileSeek(qryd->libs,apos,0);
	    iamb = ajFileReadUint(qryd->libs, bigend);
	    ajDebug ("iamb %d\n", iamb);
	    for (i=0;i<iamb;i++)
	    {
		ui = ajFileReadUint(qryd->libs, bigend);
		bc = (ui & 0xF0000000);
		bc >>=28;
		bn = (ui & 0x0F000000);
		bn >>=24;
		ip = (ui & 0x00FFFFFF);

		ajDebug ("amb[%3d] %x %5d %x %2d %8x %10d\n",
			 i, ip, ip, bc, bc, bn, bn, ui, ui);
		for (j=0; j<= bn; j++)
		    seq[ip+j] = abases[bc];
	    }
	}
	else
	    a_len = 0;

	ajStrFixI (*sline, seq_len);
	(void) ajStrAssS (&seqin->Inseq, *sline);
	return ajTrue;


    case 1:
	if (qryd->libf)
	{	/* we have the FASTA source file */
	    seq_len = fend - fstart;
	    (void) ajStrAssCL (sline, "", seq_len+1);
	    ajDebug ("reading FASTA file\n");
	    ajFileSeek(qryd->libf,fstart,0);
	    while (ajFileGetsTrim(qryd->libf, &rdline))
	    {	/* line + newline + 1 */
		ajDebug("Read: '%S'\n", rdline);
		if (ajStrChar(rdline,0) == '>') /* the FASTA line */
		    break;
		ajStrApp (sline, rdline);
	    }
	    (void) ajStrAssS (&seqin->Inseq, *sline);
	    return ajTrue;
	}
	else
	{	/* DNA Blast 1.4 from the csq file */

	    /*
	     ** Start and End offsets are in bases which are compressed
	     ** so we need to convert them to bytes for the file offsets
	     */

	    ajDebug ("reading blast 1.4 compressed DNA file\n");
	    spos = (start)/(char_bit/nbpn);
	    ajFileSeek(qryd->libs,spos-1,0);

	    c_len = end/(char_bit/nbpn) - start/(char_bit/nbpn);
	    c_len -= nsentinels;	/* trim first 2 (magic) bytes */

	    seq_len = c_len*(char_bit/nbpn);
	    c_pad = start & ((char_bit/nbpn)-1);
	    if (c_pad != 0)
		seq_len -= ((char_bit/nbpn) - c_pad);

	    ajDebug ("c_len %d c_pad %d spos %d seq_len %d\n",
		     c_len, c_pad, spos, seq_len);

	    (void) ajStrAssCL (sline, "", seq_len+1);

	    if (!ajFileRead(&tmpbyte, (size_t)1, (size_t)1,
			    qryd->libs))	/* skip the null byte */
	      ajFatal ("error reading file %F", qryd->libs);
	    if (tmpbyte != nt_magic_byte)
	    {
		ajDebug (" phase error: %d:%d (%d/%d) found\n",
			 qryd->idnum,seq_len,(ajint)tmpbyte,(ajint)nt_magic_byte);
		ajDebug (" error reading seq at %d\n",start);
		ajErr (" phase error: %d:%d (%d/%d) found\n",
		       qryd->idnum,seq_len,(ajint)tmpbyte,(ajint)nt_magic_byte);
		ajErr (" error reading seq at %d\n",start);
		return ajFalse;
	    }

	    seqcnt=(seq_len+3)/4;
	    if (seqcnt==0)
		seqcnt++;
	    if ((tmp=ajFileRead(ajStrStr(*sline),(size_t)1,(size_t)seqcnt,
				qryd->libs)) != (size_t)seqcnt)
	    {
		ajDebug (
			 " could not read sequence record (e): %S %d %d"
			 " != %d: %d\n",
			 *sline,start,tmp,seqcnt,*seq);
		ajDebug (" error reading seq at %d\n",start);
		ajErr (
		       " could not read sequence record (f): %S %d %d"
		       " != %d: %d\n",
		       *sline,start,tmp,seqcnt,*seq);
		ajErr (" error reading seq at %d\n",start);
		return ajFalse;
	    }
	    /* skip the null byte */
	    if (!ajFileRead(&tmpbyte, (size_t)1, (size_t)1, qryd->libs))
	      ajFatal ("error reading file %F", qryd->libs);

	    if (tmpbyte != nt_magic_byte)
	    {
		ajDebug (" phase2 error: %d:%d (%d/%d) next \n",
			 qryd->idnum,seqcnt,(ajint)tmpbyte,(ajint)nt_magic_byte);
		ajDebug (" error reading seq at %d\n",start);
		ajErr (" phase2 error: %d:%d (%d/%d) next ",
		       qryd->idnum,seqcnt,(ajint)tmpbyte,(ajint)nt_magic_byte);
		ajErr (" error reading seq at %d\n",start);
		return ajFalse;
	    }

	    /*
	       point to the last packed byte and to the end of the array
	       seqcnt is the exact number of bytes read
	       tptr points to the destination, use multiple of 4 to simplify
	       math
	       sptr points to the source, note that the last byte will be
	       read 4 cycles before it is written
	       */

	    seq = ajStrStr(*sline);

	    sptr = seq + seqcnt;
	    tptr = seq + 4*seqcnt;
	    while (sptr>seq)
	    {
		stmp = *--sptr;
		*--tptr = bases[(stmp&3) +1];
		*--tptr = bases[((stmp >>= 2)&3)+1];
		*--tptr = bases[((stmp >>= 2)&3)+1];
		*--tptr = bases[((stmp >>= 2)&3)+1];
	    }

	    if (seqcnt*4 >= seq_len)
	    {				/* there was enough room */
		seq[seq_len]= '\0';
		ajDebug("enough room: seqlen %d\n",seq_len);
	    }
	    else
	    {				/* not enough room */
		seq[seqcnt*4]='\0';
		seq_len -= 4*seqcnt;
		ajDebug("not enough room: seqcnt: %d partial seqlen %d\n",
			seqcnt, seq_len);
	    }

	    ajStrFixI (*sline, seq_len);
	    (void) ajStrAssS (&seqin->Inseq, *sline);
	    return ajTrue;
	}

    default:
	ajErr ("Unknown blast database type %d", qryd->type);

    }

    return ajFalse;
}

/* @funcstatic seqBlastStripNcbi **********************************************
**
** Removes the extra part of an NCBI style header from the BLAST header table.
**
** @param [r] line [AjPStr*] Input line
** @return [void]
** @@
******************************************************************************/

static void seqBlastStripNcbi (AjPStr* line)
{

    /*  static AjPRegexp gnlexp = NULL;
	static AjPRegexp giexp = NULL;*/
    static AjPStr tmpline = NULL;
    /*  static AjPStr tmpstr = NULL;*/

    /*  if (!gnlexp)
	gnlexp = ajRegCompC("^gnl[|][^|]+[|][^ ]+ +");

	if (!giexp)
	giexp = ajRegCompC("^gi[|][^|]+[|]");*/

    (void) ajStrAssS (&tmpline, *line);

    /*  ajDebug ("parseNCBI '%S'\n", tmpline);
	if (ajRegExec(gnlexp, tmpline))
	{
	ajRegPost(gnlexp, &tmpstr);
	(void) ajStrAssS (&tmpline, tmpstr);
	}

	if (ajRegExec(giexp, tmpline))
	{
	ajRegPost(giexp, &tmpstr);
	(void) ajStrAssS (&tmpline, tmpstr);
	}
	*/
    ajFmtPrintS (line, ">%S", tmpline);
    ajDebug ("trim to   '%S'\n", tmpline);

    return;
}


/* @funcstatic seqCdTrgQuery **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [r] qry [AjPSeqQuery] Sequence query object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqCdTrgQuery (AjPSeqQuery qry)
{
  ajint ret=0;

    if (ajStrLen(qry->Org))
    {
      ret += seqCdTrgFind (qry, "taxon", qry->Org);
    }
    if (ajStrLen(qry->Key))
    {
      ret += seqCdTrgFind (qry, "keyword", qry->Key);
    }
    if (ajStrLen(qry->Des))
    {
      ret += seqCdTrgFind (qry, "des", qry->Des);
    }
    if (ajStrLen(qry->Sv))
    {
      ret += seqCdTrgFind (qry, "seqvn", qry->Sv);
    }
    if (ajStrLen(qry->Acc))
    {
      ret += seqCdTrgFind (qry, "acnum", qry->Acc);
    }

    if (ret) return ajTrue;
    return ajFalse;
}

/* @funcstatic seqCdTrgFind **************************************************
**
** Binary search of an EMBL CD-ROM index file for entries matching a
** wildcard query.
**
** Where more than one query field is defined (usually acc and sv) it
** can test all and append to a single list.
**
** @param [r] qry [AjPSeqQuery] Sequence query object.
** @param [r] indexname [char*] Index name.
** @param [r] queryName [AjPStr] Query string.
** @return [ajint] Number of matches found
** @@
******************************************************************************/

static ajint seqCdTrgFind (AjPSeqQuery qry, char* indexname, AjPStr queryName)
{
    SeqPCdQry wild = qry->QryData;
    AjPList   l = wild->List;
    SeqPCdTrg trgline = wild->trgLine;
    SeqPCdIdx idxline = wild->idxLine;
    SeqPCdFile idxfp = wild->ifp;
    SeqPCdFile trgfp = wild->trgfp;
    SeqPCdFile hitfp = wild->hitfp;
    AjBool *skip = wild->Skip;

    AjPStr fdstr = NULL;
    AjPStr fdprefix = NULL;

    ajint t;
    ajint b;
    ajint t2;
    ajint b2;
    ajint t3;
    ajint pos=0;
    ajint prefixlen;
    ajint start;
    ajint end;
    ajint i;
    ajint j;
    ajint k;
    ajint cmp;
    AjBool match;

    AjBool first;
    char   *name;

    SeqPCdEntry entry;

    if (!seqCdTrgOpen (qry->IndexDir, indexname, &trgfp, &hitfp))
      return 0;

    /* fdstr is the original query string, in uppercase */

    /* fdprefix is the fixed (no wildcard) prefix of fdstr */

    (void) ajStrAssS (&fdstr,queryName);
    (void) ajStrToUpper(&fdstr);
    (void) ajStrAssS(&fdprefix,fdstr);

    (void) ajStrWildPrefix(&fdprefix);

    ajDebug("queryName '%S' fdstr '%S' fdprefix '%S'\n",
	    queryName, fdstr, fdprefix);
    b = b2 = 0;
    t = t2 = t3 = trgfp->NRecords - 1;

    prefixlen = ajStrLen(fdprefix);
    first = ajTrue;

    if(prefixlen)
    {

      /*
      ** (1a) we have a prefix (no wildcard at the start)
      ** look for the prefix fdprefix
      ** Set range of records that match (will be consecutive of course)
      ** from first match
      */

	while(b<=t)
	{
	    pos = (t+b)/2;
	    name = seqCdTrgName(pos,trgfp);
	    name[prefixlen]='\0';	/* truncate to prefix length */
	    cmp = ajStrCmpC(fdprefix,name);
/*	    match = ajStrMatchWildC(fdstr,name);*/
	    ajDebug(" trg testc %d '%s' '%S' %2d (+/- %d)\n",
		    pos,name,fdprefix,t-b);
	    if(!cmp)
	    {
		ajDebug(" trg hit %d\n",pos);
		if(first)
		{
		    first = ajFalse;
		    t2 = t;
		    t3 = pos;
		}
		b2 = pos;
	    }
	    if(cmp>0)
		b = pos+1;
	    else
		t = pos-1;
	}

	if(first)
	{
	    ajStrDel(&fdprefix);
	    ajStrDel(&fdstr);
	    seqCdTrgClose(&trgfp,&hitfp);
	    return ajFalse;
	}
	ajDebug("first pass: pos:%d b2:%d t2:%d\n",pos,b2,t2);

    /*
    ** (1b) Process below
    */

	b = b2-1;
	t = t2;
	while(b<=t)
	{
	    pos = (t+b)/2;
	    name = seqCdTrgName(pos,trgfp);
	    name[prefixlen]='\0';
	    cmp = ajStrCmpC(fdprefix,name);
	    /* match = ajStrMatchWildC(fdstr,name); */
	    ajDebug(" trg testd %d '%s' '%S' %B (+/- %d)\n",
		    pos,name,fdprefix,cmp,t-b);
	    if(!cmp)
	    {
		ajDebug(" trg hit %d\n",pos);
		t3 = pos;
	    }
	    if(cmp<0)
		t = pos-1;
	    else
		b = pos+1;
	}

	ajDebug("second pass: pos:%d b2:%d t3:%d\n",pos,b2,t3);
	name = seqCdTrgName(b2,trgfp);
	ajDebug("first %d '%s'\n",b2,name);
	name = seqCdTrgName(t3,trgfp);
	ajDebug("last %d '%s'\n",t3,name);

    }


    start = b2;
    end   = t3;
    for(i=start;i<=end;++i)
    {
	name = seqCdTrgName(i,trgfp);
	match = ajStrMatchWildCC(name, ajStrStr(fdstr));

	ajDebug("third pass: match:%B i:%d name '%s' queryName '%S'\n",
		match, i, name, fdstr);
	if (!match) continue;

	seqCdTrgLine (trgline, i, trgfp);
	(void) seqCdFileSeek (hitfp,trgline->FirstHit-1);
	ajDebug("Query First: %d Count: %d\n",
		trgline->FirstHit, trgline->NHits);
	pos = trgline->FirstHit;

	for (j=0;j<trgline->NHits;++j)
	{
	    (void) seqCdFileReadInt (&k,hitfp);
	    --k;
	    ajDebug("hitlist[%d] entry = %d\n",j,k);
	    (void) seqCdIdxLine (idxline,k,idxfp);

	    if (!skip[idxline->DivCode-1])
	    {
		AJNEW0(entry);
		entry->div = idxline->DivCode;
		entry->annoff = idxline->AnnOffset;
		entry->seqoff = idxline->SeqOffset;
		ajListPushApp(l,(void*)entry);
	    }
	    else
	    {
		ajDebug("SKIP: token '%S' [file %d]\n",
			queryName,idxline->DivCode);
	    }
	}

    }

    (void) seqCdTrgClose (&trgfp, &hitfp);


    ajStrDel(&trgline->Target);
    ajStrDel(&fdstr);
    ajStrDel(&fdprefix);

    return ajListLength(l);
}

/* @func ajSeqPrintAccess *****************************************************
**
** Reports the internal data structures
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintAccess (AjPFile outf, AjBool full)
{
    ajint i=0;

    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "# sequence access methods\n");
    ajFmtPrintF (outf, "# Name\n");
    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "method {\n");
    for (i=0; seqAccess[i].Name; i++)
    {
	ajFmtPrintF (outf, "  %-12s\n",
		     seqAccess[i].Name);
    }
    ajFmtPrintF (outf, "}\n\n");

    return;
}

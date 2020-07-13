/* 
** This is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "limits.h"

static ajint seqMaxGcglines = 5000;
static AjPRegexp qrywildexp = 0;

static AjBool seqInFormatSet = AJFALSE;

typedef struct SeqSInFormat
{
  char *Name;
  AjBool Try;
  AjBool (*Read) (AjPSeq thys, AjPSeqin seqin);
} SeqOInFormat, *SeqPInFormat;

typedef struct SeqSMsfData
{
  AjPTable Table;
  AjPStr* Names;
  ajint Count;
  ajint Nseq;
  ajint Bufflines;
} SeqOMsfData, *SeqPMsfData;

typedef struct SeqSMsfItem
{
  AjPStr Name;
  ajint Len;
  ajint Check;
  float Weight;
  AjPStr Seq;
} SeqOMsfItem, *SeqPMsfItem;

typedef struct SeqSListUsa
{
  ajint Begin;
  ajint End;
  AjBool Rev;
  ajint Format;
  AjBool Features;
  AjPStr Formatstr;
  AjPStr Usa;
} SeqOListUsa, *SeqPListUsa;

enum fmtcode {FMT_OK, FMT_NOMATCH, FMT_BADTYPE, FMT_FAIL};

static AjBool     seqReadAbi (AjPSeq thys, AjPSeqin seqin);

static void       seqAccSave (AjPSeq thys, AjPStr acc);
static ajint      seqAppend (AjPStr* seq, AjPStr line);
static AjBool     seqClustalReadseq (AjPStr rdline, AjPTable msftable);
static AjBool     seqFindInFormat (AjPStr format, ajint *iformat);
static AjBool     seqFormatSet (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqGcgDots (AjPSeq thys, AjPSeqin seqin, AjPStr* pline,
			      ajint maxlines, ajint *len);
static AjBool     seqGcgMsfDots (AjPSeq thys, AjPSeqin seqin, AjPStr* pline,
				 ajint maxlines, ajint *len);
static AjBool     seqGcgMsfHeader (AjPStr line, SeqPMsfItem* msfitem);
static AjBool     seqGcgMsfReadseq(AjPStr rdline, AjPTable msftable);
static AjBool     seqHennig86Readseq (AjPStr rdline, AjPTable msftable);
static AjBool     seqinUfoLocal (AjPSeqin thys);
static void       seqListNoComment (AjPStr* text);
static AjBool     seqListProcess (AjPSeq thys, AjPSeqin seqin, AjPStr usa);
static void       seqMsfTabDel (const void *key, void **value, void *cl);
static void       seqMsfTabList (const void *key, void **value, void *cl);
static AjBool     seqPhylipReadseq (AjPStr rdline, AjPTable phytable,
				    AjPStr token);
static AjBool     seqQueryMatch (AjPSeq thys, AjPSeqQuery query);
static AjBool     seqQueryField (AjPSeqQuery qry, AjPStr field);
static void       seqQryWildComp (void);
static AjBool     seqRead (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadAcedb (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadClustal (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadCodata (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadDbId (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadEmbl (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFasta (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadFmt (AjPSeq thys, AjPSeqin seqin,
			      ajint format);
static AjBool     seqReadGcg (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGenbank (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadGff (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadHennig86 (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadIg (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadJackknifer (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadJackknifernon (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMega (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMeganon (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadMsf (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNbrf (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNcbi (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNexus (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadNexusnon (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadPhylip (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadRaw (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSelex(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStockholm(AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStaden (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadStrider (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadSwiss (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadText (AjPSeq thys, AjPSeqin seqin);
static AjBool     seqReadTreecon (AjPSeq thys, AjPSeqin seqin);
static void       seqSelexAppend(AjPStr src, AjPStr *dest, ajint beg,
				 ajint end);
static void       seqSelexCopy(AjPSeq *thys, AjPSeqin seqin, ajint n);
static AjBool     seqSelexHeader(AjPSelex *thys, AjPStr line, ajint n,
				 AjBool *named, ajint *sqcnt);
static void       seqSelexPos(AjPStr line, ajint *begin, ajint *end);
static AjBool     seqSelexReadBlock(AjPSelex *thys, AjBool *named, ajint n,
				    AjPStr *line, AjPFileBuff buff);
static AjBool     seqSetInFormat (AjPStr format);
static void       seqSetName (AjPStr* name, AjPStr str);

static void       seqStockholmCopy(AjPSeq *thys, AjPSeqin seqin, ajint n);
static void       seqSvSave (AjPSeq thys, AjPStr sv);
static void       seqTaxSave (AjPSeq thys, AjPStr tax);
static void       seqTextSeq (AjPStr* textptr, AjPStr seq);
static void       seqUsaListTrace (AjPList list);
static AjBool     seqUsaProcess (AjPSeq thys, AjPSeqin seqin);
static void       seqUsaRestore (AjPSeqin seqin, SeqPListUsa node);
static void       seqUsaSave (SeqPListUsa node, AjPSeqin seqin);

/* static data that needs the function definitions and so must come later */

/* @funclist seqInFormatDef ***************************************************
**
** Functions to read each sequence format
**
******************************************************************************/

static SeqOInFormat seqInFormatDef[] = { /* AJFALSE = ignore (duplicates) */
  {"unknown",    AJFALSE, seqReadText},	/* alias for text */
  {"gcg",        AJTRUE,  seqReadGcg}, /* test first ... headers mislead */
  {"gcg8",       AJFALSE, seqReadGcg}, /* alias for gcg (reads pre-9.0 too) */
  {"embl",       AJTRUE,  seqReadEmbl},
  {"em",         AJFALSE, seqReadEmbl},	/* alias for embl */
  {"swiss",      AJTRUE,  seqReadSwiss},
  {"sw",         AJFALSE, seqReadSwiss}, /* alias for swiss */
  {"swissprot",  AJTRUE,  seqReadSwiss},
  {"nbrf",       AJTRUE,  seqReadNbrf},	/* test before NCBI */
  {"pir",        AJFALSE, seqReadNbrf},	/* alias for nbrf */
  {"fasta",      AJTRUE,  seqReadNcbi}, /* alias for ncbi, preferred name */
  {"ncbi",       AJFALSE, seqReadNcbi}, /* test before pearson */
  {"pearson",    AJTRUE,  seqReadFasta}, /* plain fasta can read bad files */
  {"genbank",    AJTRUE,  seqReadGenbank},
  {"gb",         AJFALSE, seqReadGenbank}, /* alias for genbank */
  {"ddbj",       AJFALSE, seqReadGenbank}, /* alias for genbank */
  {"codata",     AJTRUE,  seqReadCodata},
  {"strider",    AJTRUE,  seqReadStrider},
  {"clustal",    AJTRUE,  seqReadClustal},
  {"aln",        AJFALSE, seqReadClustal}, /* alias for clustal */
  {"phylip",     AJTRUE,  seqReadPhylip},
  {"acedb",      AJTRUE,  seqReadAcedb},
  {"dbid",       AJFALSE, seqReadDbId},	/* odd fasta with id as second token */
  {"msf",        AJTRUE,  seqReadMsf},
  {"hennig86",   AJTRUE,  seqReadHennig86},
  {"jackknifer", AJTRUE,  seqReadJackknifer},
  {"jackknifernon", AJTRUE,  seqReadJackknifernon},
  {"nexus",      AJTRUE,  seqReadNexus},
  {"nexusnon",   AJTRUE,  seqReadNexusnon},
  {"paup",       AJFALSE,  seqReadNexus}, /* alias for nexus */
  {"paupnon",    AJFALSE,  seqReadNexusnon}, /* alias for nexusnon */
  {"treecon",    AJTRUE,  seqReadTreecon},
  {"mega",       AJTRUE,  seqReadMega},
  {"meganon",    AJTRUE,  seqReadMeganon},
  {"ig",         AJFALSE, seqReadIg}, /* can read almost anything */
  {"experiment", AJFALSE, seqReadStaden}, /* can read almost anything */
  {"staden",     AJFALSE, seqReadStaden}, /* alias for experiment */
  {"text",       AJFALSE, seqReadText}, /* can read almost anything */
  {"plain",      AJFALSE, seqReadText},	/* alias for text */
  {"abi",        AJTRUE,  seqReadAbi},
  {"gff",        AJTRUE,  seqReadGff},
  {"selex",      AJTRUE,  seqReadSelex},
  {"stockholm",  AJTRUE,  seqReadStockholm},
  {"pfam",       AJTRUE,  seqReadStockholm},
  {"raw",        AJTRUE,  seqReadRaw}, /* OK - only sequence chars allowed */
  {NULL, 0, NULL}
};

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Input Constructors ***************************************
**
** All constructors return a new sequence input object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence input object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/

/* @func ajSeqinNew ***********************************************************
**
** Creates a new sequence input object.
**
** @return [AjPSeqin] New sequence input object.
** @@
******************************************************************************/

AjPSeqin ajSeqinNew (void)
{
    AjPSeqin pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    pthis->Acc = ajStrNew();
    pthis->Inputtype = ajStrNew();
    pthis->Db = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Date = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc = ajStrNew();
    pthis->Rev = ajFalse;
    pthis->Begin = 0;
    pthis->End = 0;
    pthis->Usa = ajStrNew();
    pthis->Ufo = ajStrNew();
    pthis->List = NULL;
    pthis->Formatstr = ajStrNew();
    pthis->Format = 0;
    pthis->Filename = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->Filebuff = NULL;
    pthis->Search = ajTrue;
    pthis->Single = ajFalse;
    pthis->Features = ajFalse;
    pthis->Upper = ajFalse;
    pthis->Lower = ajFalse;
    pthis->Text  = ajFalse;
    pthis->Count = 0;
    pthis->Filecount = 0;
    pthis->Query = ajSeqQueryNew();
    pthis->Data = NULL;
    pthis->Ftquery = ajFeattabInNew(); /* empty object */
    pthis->multi = ajFalse;

    return pthis;
}


/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Input Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence input object.
**
******************************************************************************/

/* @func ajSeqinDel ***********************************************************
**
** Deletes a sequence input object.
**
** @param [P] pthis [AjPSeqin*] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajSeqinDel (AjPSeqin* pthis)
{
    AjPSeqin thys = *pthis;
  
    ajStrDel(&thys->Name);
    ajStrDel(&thys->Acc);

    ajStrDel(&thys->Inputtype);

    ajStrDel(&thys->Db);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Date);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Doc);

    ajListFree(&thys->List);

    ajStrDel(&thys->Usa);
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Entryname);
    ajStrDel(&thys->Inseq);
    ajSelexDel(&thys->Selex);
    ajStockholmDel(&thys->Stockholm);
    ajSeqQueryDel(&thys->Query);

    ajFileBuffDel(&thys->Filebuff);

    if(thys->Fttable)
    {
	ajFeattableDel(&thys->Fttable);
    }
  
/*
//    if(thys->Ftquery && ! thys->multi)
//    {
//	if(thys->Ftquery->Handle)
//	    ajStrDel(&thys->Ftquery->Handle->File->Name);
//	if(thys->Ftquery->Handle)
//	    ajStrDel(&thys->Ftquery->Handle->File->Buff);
//    }
*/

    if(thys->Ftquery)		/* this deletes filebuff stuff above anyway */
    { 
        ajFeattabInDel(&thys->Ftquery);
    }

    AJFREE(*pthis);

    return;
}

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Input Modifiers ******************************************
**
** These functions use the contents of a sequence input object and
** update them.
**
******************************************************************************/

/* @func ajSeqinSetNuc *****************************************************
**
** Sets the type to be forced as nucleic for a sequence input object
**
** @param [P] seqin [AjPSeqin] Sequence input object to be set.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetNuc (AjPSeqin seqin)
{
  seqin->IsNuc = ajTrue;
}

/* @func ajSeqinSetProt *****************************************************
**
** Sets the type to be forced as protein for a sequence input object
**
** @param [P] seqin [AjPSeqin] Sequence input object to be set.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetProt (AjPSeqin seqin)
{
  seqin->IsProt = ajTrue;
}

/* @func ajSeqinSetRange *****************************************************
**
** Sets the start and end positions for a sequence input object
**
** @param [P] seqin [AjPSeqin] Sequence input object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqinSetRange (AjPSeqin seqin, ajint ibegin, ajint iend)
{

    if (ibegin)
	seqin->Begin = ibegin;

    if (iend)
	seqin->End = iend;

    return;
}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Input Assignments ***************************************
**
** These functions overwrite the sequence input object provided as the
** first argument.
**
******************************************************************************/

/* @func ajSeqinUsa ***********************************************************
**
** Creates or resets a sequence input object using a new Universal
** Sequence Address
**
** @param [uP] pthis [AjPSeqin*] Sequence input object.
** @param [P] Usa [AjPStr] USA
** @return [void]
** @@
******************************************************************************/

void ajSeqinUsa (AjPSeqin* pthis, AjPStr Usa)
{
    AjPSeqin thys;

    if (!*pthis)
    {
	thys = *pthis = ajSeqinNew();
    }
    else
    {
	thys = *pthis;
	ajSeqinClear(thys);
    }

    (void) ajStrAss (&thys->Usa, Usa);

    return;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Input Operators ******************************************
**
** These functions use the contents of a sequence input object but do
** not make any changes.
**
******************************************************************************/

/* @func ajSeqAllRead *********************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Return the results in the AjPSeq object but leave the file open for
** future calls.
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqAllRead (AjPSeq thys, AjPSeqin seqin)
{
    AjBool ret = ajFalse;
    AjPStr tmpformat = NULL;

    if (!seqInFormatSet)
    {					/* we need a copy of the formatlist */
	if (ajNamGetValueC("format", &tmpformat))
	{
	    (void) seqSetInFormat(tmpformat);
	    ajDebug ("seqSetInFormat '%S' from EMBOSS_FORMAT\n", tmpformat);
	}
	ajStrDel(&tmpformat);
	seqInFormatSet = ajTrue;
    }


    if (!seqin->Filebuff) {	/* First call. No file open yet ... */
      if (!seqUsaProcess (thys, seqin))	/* ... so process the USA */
	return ajFalse;		/* if this fails, we read no sequence at all */
    }
  
    ret = seqRead (thys, seqin); /* read the sequence */

    if (ret)			/* clone any specified DB or entryname */
    {
	(void) ajStrSet (&thys->Db, seqin->Db);
	(void) ajStrSet (&thys->Entryname, seqin->Entryname);

	if (!ajStrLen(thys->Type)) /* make sure the type is set */
	    ajSeqType (thys);
    }

    return ret;
}

/* @func ajSeqallFile *********************************************************
**
** Parse a USA Uniform Sequence Address
**
** Return the results in the AjPSeqall object but leave the file open for
** future calls.
**
** @param [r] usa [AjPStr] sequence usa.
** @return [AjPSeqall] seqall object
** @@
******************************************************************************/

AjPSeqall ajSeqallFile(AjPStr usa)
{
    AjPSeqall seqall=NULL;
    AjPSeqin  seqin=NULL;
    AjPSeq    seq=NULL;
    
    seqall = ajSeqallNew();
    
    seqin = seqall->Seqin;
    seqin->multi = ajTrue;
    seqin->Single = ajFalse;
    seq = seqall->Seq;

    ajSeqinUsa(&seqin,usa);

    if(!ajSeqAllRead(seq,seqin))
    {
	ajSeqallDel(&seqall);
	return NULL;
    }

    return seqall;
}
    
/* @func ajSeqallNext *********************************************************
**
** Reads the next sequence into a sequence stream. For the first call this
** simply returns the sequence already loaded. For later calls a new
** sequence is read.
**
** @param [uP] seqall [AjPSeqall] Sequence stream
** @param [wP] retseq [AjPSeq*] Sequence
** @return [AjBool] ajTrue if a sequence was refound. ajFalse when all is done.
** @@
******************************************************************************/

AjBool ajSeqallNext (AjPSeqall seqall, AjPSeq* retseq)
{

    if (!seqall->Count)
    {
	seqall->Count = 1;
	ajSeqSetRange (seqall->Seq, seqall->Begin, seqall->End);

	/*
//	seqall->Seq->Begin = seqall->Begin;
//	seqall->Seq->End = seqall->End;
	*/

	*retseq = seqall->Seq;
	return ajTrue;
    }

    /*
//    ajMemStat("ajSeqAllNext starting");
//    ajStrStat("ajSeqAllNext starting");
    */

    if (ajSeqRead (seqall->Seq, seqall->Seqin))
    {
	seqall->Count++;
	ajSeqSetRange (seqall->Seq, seqall->Begin, seqall->End);

	/*
//	seqall->Seq->Begin = seqall->Begin;
//	seqall->Seq->End = seqall->End;
	*/

	*retseq = seqall->Seq;

	/*
//	ajMemStat("ajSeqAllNext done");
//	ajStrStat("ajSeqAllNext done");
	*/

	ajDebug("ajSeqallNext success\n");
	return ajTrue;
    }

    *retseq = NULL;
    ajDebug("ajSeqallNext failed\n");
  
    return ajFalse;
}

/* @func ajSeqinClear *********************************************************
**
** Clears a Sequence input object back to "as new" condition, except
** for the USA list and the features setting which must be preserved.
**
** @param [P] thys [AjPSeqin] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajSeqinClear (AjPSeqin thys)
{

    ajDebug ("ajSeqInClear called\n");

    (void) ajStrClear(&thys->Name);
    (void) ajStrClear(&thys->Acc);
    /* preserve thys->Inputtype */
    (void) ajStrClear(&thys->Db);
    (void) ajStrClear(&thys->Full);
    (void) ajStrClear(&thys->Date);
    (void) ajStrClear(&thys->Desc);
    (void) ajStrClear(&thys->Doc);
    /* preserve thys->List */
    (void) ajStrClear(&thys->Usa);
    (void) ajStrClear(&thys->Ufo);
    (void) ajStrClear(&thys->Formatstr);
    (void) ajStrClear(&thys->Filename);
    (void) ajStrClear(&thys->Entryname);
    (void) ajStrClear(&thys->Inseq);

    /* preserve thys->Query */

    if (thys->Filebuff)
	ajFileBuffDel(&thys->Filebuff);
    if (thys->Filebuff)
	ajFatal("ajSeqinClear did not delete Filebuff");

    if(thys->Fttable)
    {
	ajFeattableDel(&thys->Fttable);
    }
  
/*
//    if(thys->Ftquery && ! thys->multi)
//    {
//	if(thys->Ftquery->Handle)
//	    ajStrDel(&thys->Ftquery->Handle->File->Name);
//	if(thys->Ftquery->Handle)
//	    ajStrDel(&thys->Ftquery->Handle->File->Buff);
//   }
*/

    if(thys->Ftquery)  		/* this clears filebuff stuff above anyway */
    { 
        ajFeattabInClear(thys->Ftquery);
    }

    ajSeqQueryClear(thys->Query);
    thys->Data = NULL;

    thys->Rev = ajFalse;
    thys->Format = 0;

    thys->Search = ajTrue;
    thys->Single = ajFalse;
    /* keep thys->Features */
    /* thys->Features = ajFalse;*/
    thys->Count = 0;
    thys->Filecount = 0;
  
    return;
}

/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Sequence Input Casts ********************************************
**
** These functions examine the contents of a sequence input object and
** return some derived information. Some of them provide access to the
** internal components of a sequence input object. They are provided
** for programming convenience but should be used with caution.
**
******************************************************************************/

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Assignments **********************************************
**
** These functions overwrite the sequence provided as the first argument.
**
******************************************************************************/

/* @func ajSeqRead ************************************************************
**
** If the file is not yet open, calls seqUsaProcess to convert the USA into
** an open file stream.
**
** Uses seqRead for the actual file reading.
**
** Returns the results in the AjPSeq object.
**
** @param [w] thys [AjPSeq] Sequence returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqRead (AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpformat = NULL;
    AjBool ret = ajFalse;
    SeqPListUsa node = NULL;

    if (!seqInFormatSet)
    {					/* we need a copy of the formatlist */
	if (ajNamGetValueC("format", &tmpformat))
	{
	    (void) seqSetInFormat(tmpformat);
	    ajDebug ("seqSetInFormat '%S' from EMBOSS_FORMAT\n", tmpformat);
	}
	ajStrDel(&tmpformat);
	seqInFormatSet = ajTrue;
    }

    if (seqin->Filebuff)
    {
	/* (a) if file still open, keep reading */
	ajDebug("ajSeqRead: input file '%F' still there, try again\n",
		seqin->Filebuff->File);
    }
    else
    {
	/* (b) if we have a list, try the next USA in the list */
	if (ajListLength (seqin->List))
	{
	    (void) ajListPop (seqin->List, (void**) &node);

	    ajDebug("++pop from list '%S'\n", node->Usa);
	    ajSeqinUsa (&seqin, node->Usa);
	    ajDebug("++SAVE SEQIN '%S' %d..%d(%b) '%S' %d\n",
		    seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		    seqin->Formatstr, seqin->Format);
	    seqUsaRestore(seqin, node);

	    ajStrDel(&node->Usa);
	    ajStrDel(&node->Formatstr);
	    AJFREE (node);

	    ajDebug("ajSeqRead: open list, try '%S'\n", seqin->Usa);
	}
	else
	{
	    ajDebug("ajSeqRead: no file yet - test USA '%S'\n", seqin->Usa);
	}
	/* (c) Must be a USA - decode it */
	if (!seqUsaProcess (thys, seqin))    
	    return ajFalse;
    }

    /* Now read whatever we got */

    ret = seqRead (thys, seqin);
  
    while (!ret && ajListLength (seqin->List))
    {
	/* Failed, but we have a list still - keep trying it */
	(void) ajListPop (seqin->List, (void**) &node);
	ajDebug("++try again: pop from list '%S'\n", node->Usa);
	ajSeqinUsa (&seqin, node->Usa);
	ajDebug("++SAVE (AGAIN) SEQIN '%S' %d..%d(%b) '%S' %d\n",
		seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
		seqin->Formatstr, seqin->Format);
	seqUsaRestore(seqin, node);

	ajStrDel(&node->Usa);
	ajStrDel(&node->Formatstr);
	AJFREE (node);

	/* must exit if this fails ... for bad list USAs */

	if (!seqUsaProcess (thys, seqin))
	    continue;

	/*	seqUsaProcess (thys, seqin);*/

	ret = seqRead (thys, seqin);
    }

    if (!ret)
	return ajFalse;

  

    /* if values are missing in the sequence object, we can use defaults
       from seqin or calculate where possible */

    ajDebug ("++keep restored %d..%d (%b) '%S' %d\n",
	     seqin->Begin, seqin->End, seqin->Rev,
	     seqin->Formatstr, seqin->Format);
    ajDebug ("ajSeqRead: thys->Db '%S', seqin->Db '%S'\n",
	     thys->Db, seqin->Db);
    ajDebug ("ajSeqRead: thys->Name '%S'\n",
	     thys->Name);
    ajDebug ("ajSeqRead: thys->Entryname '%S', seqin->Entryname '%S'\n",
	     thys->Entryname, seqin->Entryname);
    (void) ajStrSet (&thys->Db, seqin->Db);
    (void) ajStrSet (&thys->Entryname, seqin->Entryname);
    (void) ajStrSet (&thys->Name, thys->Entryname);
    ajDebug ("ajSeqRead: thys->Name '%S'\n",
	     thys->Name);
    if (!ajStrLen(thys->Type))
	ajSeqType (thys);


    return ajTrue;
}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Set Assignments *****************************************
**
** These functions overwrite the sequence set object provided as the
** first argument.
**
******************************************************************************/

/* @func ajSeqsetRead *********************************************************
**
** Parse a USA Uniform Sequence Address into format, access, file and entry
**
** Split at delimiters. Check for the first part as a valid format
** Check for the remaining first part as a database name or as a file
** that can be opened.
** Anything left is an entryname spec.
**
** Read all the sequences until done
**
** Return the results in the AjPSeqset object.
**
** @param [w] thys [AjPSeqset] Sequence set returned.
** @param [u] seqin [AjPSeqin] Sequence input definitions
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqsetRead (AjPSeqset thys, AjPSeqin seqin)
{
    AjPSeq seq;
    AjPList setlist;

    ajint iseq = 0;

    seq = ajSeqNew();

    ajDebug ("ajSeqsetRead\n");

    if (!seqUsaProcess (seq, seqin))
	return ajFalse;

    (void) ajStrAss (&thys->Usa, seqin->Usa);
    (void) ajStrAss (&thys->Ufo, seqin->Ufo);
    thys->Begin = seqin->Begin;
    thys->End = seqin->End;

    setlist = ajListNew();

    ajDebug("ready to start reading format '%S' '%S' %d..%d\n",
	    seqin->Formatstr, seq->Formatstr, seqin->Begin, seqin->End);

    while (ajSeqRead (seq, seqin))
    {
	/*ajDebug("read name '%S' length %d format '%S' '%S' seqindata: %x\n",
	  seq->Entryname, ajSeqLen(seq),
	  seqin->Formatstr, seq->Formatstr, seqin->Data);*/
	(void) ajStrSet (&seq->Db, seqin->Db);
	if (!ajStrLen(seq->Type))
	    ajSeqType (seq);

	/*ajDebug ("ajSeqsetRead read sequence %d '%s' %d..%d\n",
	  iseq, ajSeqName(seq), seq->Begin, seq->End);*/
	/*ajSeqTrace(seq);*/
	iseq++;

	ajListPushApp (setlist, seq);

	/*ajDebug("appended to list\n");*/

	/* add to a list of sequences */
    
	seq = ajSeqNew();
	(void) seqFormatSet (seq, seqin);
    }
    ajSeqDel(&seq);

    if (!iseq)
	return ajFalse;

    /* convert the list of sequences into a seqset structure */

    ajSeqsetFromList (thys, setlist);

    ajListFree (&setlist);

    ajDebug ("ajSeqsetRead total %d sequences\n", iseq);

    return ajTrue;
}

/* @func ajSeqsetFromList *****************************************************
**
** Builds a sequence set from a list of sequences
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] list [AjPList] List of sequence objects
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetFromList (AjPSeqset thys, AjPList list)
{

  ajint i;
  AjIList iter;
  AjPSeq seq;

  thys->Size = ajListLength(list);
  thys->Seq = AJCALLOC0(thys->Size, sizeof(AjPSeq));
  thys->Seqweight = AJCALLOC0(thys->Size, sizeof(float));

  i = 0;
  iter = ajListIter (list);
  while ((seq = ajListIterNext(iter)))
  {
    if (!i)
    {
      thys->EType = seq->EType;
      (void) ajStrAss (&thys->Type, seq->Type);
      thys->Format = seq->Format;
      (void) ajStrAss (&thys->Formatstr, seq->Formatstr);
      (void) ajStrAss (&thys->Filename, seq->Filename);
      (void) ajStrAss (&thys->Full, seq->Full);
    }
    thys->Seqweight[i] = seq->Weight;
    thys->Seq[i] = seq;
    thys->Totweight += seq->Weight;
    if (ajSeqLen(seq) > thys->Len)
      thys->Len = ajSeqLen(seq);
    i++;
  }
  ajListIterFree(iter);

  return thys->Size;
}

/* @func ajSeqsetFromPair *****************************************************
**
** Builds a sequence set from a pair of sequences
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] seqa [AjPSeq] Sequence 1
** @param [r] seqb [AjPSeq] Sequence 2
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetFromPair (AjPSeqset thys, AjPSeq seqa, AjPSeq seqb)
{

  (void) ajSeqsetApp (thys, seqa);
  (void) ajSeqsetApp (thys, seqb);

  return thys->Size;
}

/* @func ajSeqsetApp *****************************************************
**
** Adds a sequence to a sequence set
**
** @param [w] thys [AjPSeqset] Sequence set
** @param [r] seq [AjPSeq] Sequence
** @return [ajint] Number of sequences in the set.
******************************************************************************/

ajint ajSeqsetApp (AjPSeqset thys, AjPSeq seq)
{
  ajint iseq;

  iseq = thys->Size;

  ajDebug ("ajSeqsetApp '%S' size %d len %d add '%S' len %d\n",
	   thys->Full, thys->Size, thys->Len,
	   seq->Full, ajSeqLen(seq));

  thys->Size ++;
  AJCRESIZE(thys->Seq, thys->Size);
  AJCRESIZE(thys->Seqweight, thys->Size);

  if (!iseq)
  {
    thys->EType = seq->EType;
    (void) ajStrSet (&thys->Type, seq->Type);
    thys->Format = seq->Format;
    (void) ajStrSet (&thys->Formatstr, seq->Formatstr);
    (void) ajStrSet (&thys->Filename, seq->Filename);
    (void) ajStrSet (&thys->Full, seq->Full);
  }

  thys->Seqweight[iseq] = seq->Weight;
  thys->Seq[iseq] = ajSeqNewS(seq);
  thys->Totweight += seq->Weight;
  if (ajSeqLen(seq) > thys->Len)
    thys->Len = ajSeqLen(seq);

  ajDebug ("result '%S' size %d len\n",
	   thys->Full, thys->Size, thys->Len);

  return thys->Size;
}

/* @funcstatic seqReadFmt ****************************************************
**
** Tests whether a sequence can be read using the specified format.
** Then tests whether the sequence matches sequence query criteria
** and checks any specified type. Applies upper and lower case.
**
** @param [r] thys [AjPSeq] Sequence object
** @param [r] seqin [AjPSeqin] Sequence input object
** @param [r] format [ajint] input format code
** @return [ajint] 0 if successful.
**                 1 if the query match failed.
**                 2 if the sequence type failed
**                 3 if it failed to read a sequence
** @@
** This is the only function that calls the appropriate Read function
** seqReadXxxxxx where Xxxxxxx is the supported sequence format.
**
** Some of the seqReadXxxxxx functions fail to reset the buffer correctly,
** which is a very serious problem when cycling through all of them to
** identify an unknown format. The extra ajFileBuffReset call at the end is
** intended to address this problem. The individual functions should still
** reset the buffer in case they are called from elsewhere.
**
******************************************************************************/

static ajint seqReadFmt (AjPSeq thys, AjPSeqin seqin,
		       ajint format)
{
    ajDebug ("++seqReadFmt format %d (%s) '%S' feat %B\n",
	     format, seqInFormatDef[format].Name,
	     seqin->Usa, seqin->Features);

    /* Calling funclist seqInFormatDef() */
    if (seqInFormatDef[format].Read (thys, seqin))
    {
	ajDebug ("seqReadFmt success with format %d (%s)\n",
		 format, seqInFormatDef[format].Name);
	seqin->Format = format;
	(void) ajStrAssC(&seqin->Formatstr, seqInFormatDef[format].Name);
	(void) ajStrAssC(&thys->Formatstr, seqInFormatDef[format].Name);
	(void) ajStrAssS(&thys->Db, seqin->Db);
	(void) ajStrAssS(&thys->Entryname, seqin->Entryname);
	(void) ajStrAssS(&thys->Filename, seqin->Filename);

	if (seqQueryMatch(thys, seqin->Query))
	{
	    if (seqin->Features && !thys->Fttable)
	    {
		(void) ajStrSet (&seqin->Ftquery->Seqname, thys->Name);
		if (!ajFeatUfoRead (&seqin->Fttable, seqin->Ftquery,
				    seqin->Ufo))
		{
		    ajDebug ("seqReadFmt features input failed UFO: '%S'\n",
		       seqin->Ufo);
		    /*
		     *  GWW 21 Aug 2000 - don't warn about missing feature
		     *  tables. Caveat emptor!
		     */
		    /* ajWarn ("seqReadFmt features input failed UFO: '%S'",
		       seqin->Ufo); */
		    /*	   return FMT_FAIL;*/
		}
		else
		{
		    ajFeattableDel(&thys->Fttable);
		    ajFeattableTrace(seqin->Fttable);
		    thys->Fttable = seqin->Fttable;
		    seqin->Fttable = NULL;
		}
	    }

	    if (ajSeqTypeCheck(thys, seqin))
	    {
		ajSeqinTrace (seqin);
		if(seqin->Upper)
		    ajSeqToUpper(thys);
		if(seqin->Lower)
		    ajSeqToLower(thys);
		if (seqin->Begin)
		  thys->Begin = seqin->Begin;
		if (seqin->End)
		  thys->End = seqin->End;
		if (seqin->Rev)
		  thys->Rev = seqin->Rev;
		return FMT_OK;
	    }
	    else
		return FMT_BADTYPE;
	}
	ajDebug ("query match failed, continuing ...\n");
	ajSeqClear (thys);
	return FMT_NOMATCH;
    }
    else
    {
      ajFileBuffReset(seqin->Filebuff);
      ajDebug("Format %d (%s) failed, file buffer reset by seqReadFmt\n",
	      format, seqInFormatDef[format].Name);
      /* ajFileBuffTraceFull(seqin->Filebuff, 10, 10);*/
    }
    ajDebug ("++seqReadFmt failed - nothing read\n");
    return FMT_FAIL;
}

/* @funcstatic seqRead ********************************************************
**
** Given data in a seqin structure, tries to read everything needed
** using the specified format or by trial and error.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqRead (AjPSeq thys, AjPSeqin seqin)
{
    ajint i;
    ajint stat;

/*    AjPFileBuff buff = seqin->Filebuff; unused */

    if (seqin->Single && seqin->Count)
    {
      /*
      ** we read one sequence at a time.
      ** the first sequence was read by ACD
      ** for the following ones we need to reset the AjPSeqin
      **
      ** Single is set by the access method
      */

	ajDebug ("seqRead: single access - count %d - call access"
		 " routine again\n",
		 seqin->Count);
	/* Calling funclist seqAccess() */
	if (!seqin->Query->Access->Access(seqin))
	{
	    ajDebug ("seqRead: seqin->Query->Access->Access(seqin) "
		     "*failed*\n");
	    return ajFalse;
	}
    }

    ajDebug("seqRead: seqin format %d '%S'\n", seqin->Format,
	    seqin->Formatstr);
    ajSeqClear (thys);

    seqin->Count++;

    if (!seqin->Format)
    {	/* no format specified, try all defaults */

	for (i = 1; seqInFormatDef[i].Name; i++)
	{
	    if (!seqInFormatDef[i].Try) /* skip if Try is ajFalse */
		continue;

	    ajDebug ("seqRead:try format %d (%s)\n",
		     i, seqInFormatDef[i].Name);

	    stat = seqReadFmt (thys, seqin, i);
	    switch (stat) {
	    case FMT_OK:
	      ajDebug("++seqRead OK, set format %d\n", seqin->Format);
	      return ajTrue;
	    case FMT_BADTYPE:
	      ajDebug ("seqRead: (a1) seqReadFmt stat == BADTYPE *failed*\n");
	      return ajFalse;
	    case FMT_FAIL:
	      ajDebug ("seqRead: (b1) seqReadFmt stat == FAIL *failed*\n");
	      break;		/* we can try next format */
	    case FMT_NOMATCH:
	      ajDebug ("seqRead: (c1) seqReadFmt stat == NOMATCH *try again*\n");
	      break;
	    default:
	      ajDebug("unknown code %d from seqReadFmt\n", stat);
	    }

	    if (seqin->Format) break;	/* we read something */
	}

	if (!seqin->Format)
	{   /* all default formats failed, give up */
	    ajDebug ("seqRead:all default formats failed, give up\n");
	    return ajFalse;
	}
	ajDebug("++seqRead set format %d\n", seqin->Format);
    }
    else
    {	/* one format specified */
	ajFileBuffNobuff (seqin->Filebuff);

	ajDebug ("++seqRead known format %d\n", seqin->Format);
	stat = seqReadFmt (thys, seqin, seqin->Format);
	switch (stat) {
	case FMT_OK:
	  return ajTrue;
	case FMT_BADTYPE:
	  ajDebug ("seqRead: (a2) seqReadFmt stat == BADTYPE *failed*\n");
	  return ajFalse;
	case FMT_FAIL:
	  ajDebug ("seqRead: (b2) seqReadFmt stat == FAIL *failed*\n");
	  break;		/* could be simply end-of-file */
	case FMT_NOMATCH:
	  ajDebug ("seqRead: (c2) seqReadFmt stat == NOMATCH *try again*\n");
	  break;
	default:
	  ajDebug("unknown code %d from seqReadFmt\n", stat);
	}

	ajSeqClear (thys);	/* 1 : read, failed to match id/acc/query */
    }

    /* failed - probably entry/accession query failed. Can we try again? */

    ajDebug("seqRead failed - try again with format %d '%s'\n",
	    seqin->Format, seqInFormatDef[seqin->Format].Name);

    /*while (seqin->Search && !ajFileBuffEmpty (buff))*/
    while (seqin->Search)
    {
	stat = seqReadFmt (thys, seqin, seqin->Format);
	switch (stat) {
	case FMT_OK:
	  return ajTrue;
	case FMT_BADTYPE:
	  ajDebug ("seqRead: (a3) seqReadFmt stat == BADTYPE *failed*\n");
	  return ajFalse;
	case FMT_FAIL:
	  ajDebug ("seqRead: (b3) seqReadFmt stat == FAIL *failed*\n");
	  return ajFalse;
	case FMT_NOMATCH:
	  ajDebug ("seqRead: (c3) seqReadFmt stat == NOMATCH *try again*\n");
	  break;
	default:
	  ajDebug("unknown code %d from seqReadFmt\n", stat);
	}
	ajSeqClear (thys);	/* 1 : read, failed to match id/acc/query */
    }

    if (seqin->Format)
	ajDebug ("seqRead: *failed* to read sequence %S using format %s\n",
		 seqin->Usa, seqInFormatDef[seqin->Format].Name);
    else
	ajDebug ("seqRead: *failed* to read sequence %S using any format\n",
		 seqin->Usa);

    return ajFalse;
}

/* @funcstatic seqReadFasta ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTA format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadFasta (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    AjPFileBuff buff = seqin->Filebuff;
    static AjPStr id = NULL;
    static AjPStr acc = NULL;
    static AjPStr sv = NULL;
    static AjPStr desc = NULL;

    char *cp;
    ajint bufflines = 0;
    ajlong fpos = 0;
    ajlong fposb = 0;
    AjBool ok = ajTrue;

    ajDebug ("seqReadFasta\n");
    /* ajFileBuffTrace (buff); */

    ok = ajFileBuffGetL (buff, &rdline, &fpos);
    if (!ok)
	return ajFalse;

    bufflines++;

    if (ajStrChar(rdline, 3) == ';')	/* then it is really PIR format */
	return ajFalse;

    cp = ajStrStr(rdline);
    if (*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFileBuffReset (buff);
	return ajFalse;
    }

    if (!ajSeqParseFasta(rdline, &id, &acc, &sv, &desc)) {
      ajFileBuffReset (buff);
      return ajFalse;
    }

    seqSetName (&thys->Name, id);

    if (ajStrLen(sv))
      (void) seqSvSave (thys, sv);

    if (ajStrLen(acc))
      (void) seqAccSave (thys, acc);
    
    (void) ajStrAssS (&thys->Desc, desc);

    if (ajStrLen(seqin->Inseq))
    {	/* we have a sequence to use */
        ajDebug("++fasta use Inseq '%S'\n", seqin->Inseq);
	ajStrAssS (&thys->Seq, seqin->Inseq);
	if (seqin->Text)
	{
	  seqTextSeq(&thys->TextPtr, seqin->Inseq);
	}
	ajFileBuffClear (buff, 0);
    }
    else
    {
	ok = ajFileBuffGetL (buff, &rdline, &fposb);
	while (ok && !ajStrPrefixC(rdline, ">"))
	{
	    (void) seqAppend (&thys->Seq, rdline);
	    bufflines++;
	    ajDebug("++fasta append line '%S'\n", rdline);
	    ok = ajFileBuffGetL (buff, &rdline, &fposb);
	}
	if (ok)
	    ajFileBuffClear (buff, 1);
	else
	    ajFileBuffClear (buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %ld ok: %B fposb: %ld\n", fpos, ok, fposb);

    return ajTrue;
}

/* @funcstatic seqReadDbId ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using the FASTA >db id format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadDbId (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    AjPFileBuff buff = seqin->Filebuff;

    char *cp;
    AjPStr vacc = NULL;
    ajint bufflines = 0;
    ajlong fpos = 0;
    ajlong fposb = 0;
    AjBool ok = ajTrue;

    ajDebug ("seqReadDbId\n");
    /* ajFileBuffTrace (buff); */

    ok = ajFileBuffGetL (buff, &rdline, &fpos);
    if (!ok)
	return ajFalse;

    bufflines++;

    if (ajStrChar(rdline, 3) == ';')	/* then it is really PIR format */
	return ajFalse;

    cp = ajStrStr(rdline);
    if (*cp != '>')
    {
	ajDebug("first line is not FASTA\n");
	ajFileBuffReset (buff);
	return ajFalse;
    }

    (void) ajStrTokenAss (&handle, rdline, "> ");
    (void) ajStrToken (&token, &handle, " \t\n\r");
    (void) ajStrToken (&token, &handle, " \t\n\r");
    seqSetName (&thys->Name, token);

    (void) ajStrToken (&token, &handle, NULL);

    vacc = ajIsSeqversion(token);
    if (vacc)
    {
	(void) seqSvSave (thys, token);
	(void) seqAccSave (thys, vacc);
	(void) ajStrToken (&thys->Desc, &handle, "\n\r");
    }
    else if (ajIsAccession(token))
    {
	(void) seqAccSave (thys, token);
	(void) ajStrToken (&thys->Desc, &handle, "\n\r");
    }
    else
    {
	(void) ajStrAss (&thys->Desc, token);
	if (ajStrToken (&token, &handle, "\n\r"))
	{
	    (void) ajStrAppC (&thys->Desc, " ");
	    (void) ajStrApp (&thys->Desc, token);
	}
    }

    (void) ajStrDelReuse(&token);	/* duplicate of accession or description */
    (void) ajStrTokenReset (&handle);

    if (ajStrLen(seqin->Inseq))
    {					/* we have a sequence to use */
	ajStrAssS (&thys->Seq, seqin->Inseq);
	if (seqin->Text)
	{
	  seqTextSeq(&thys->TextPtr, seqin->Inseq);
	}
	ajFileBuffClear (buff, 0);
    }
    else
    {
	ok = ajFileBuffGetL (buff, &rdline, &fposb);
	while (ok && !ajStrPrefixC(rdline, ">"))
	{
	    (void) seqAppend (&thys->Seq, rdline);
	    bufflines++;
	    ok = ajFileBuffGetL (buff, &rdline, &fposb);
	}
	if (ok)
	    ajFileBuffClear (buff, 1);
	else
	    ajFileBuffClear (buff, 0);
    }

    thys->Fpos = fpos;

    ajDebug("started at fpos %ld ok: %B fposb: %ld\n", fpos, ok, fposb);

    return ajTrue;
}

/* @funcstatic seqReadNbrf ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NBRF format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNbrf (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    AjPFileBuff buff = seqin->Filebuff;
    AjPFileBuff ftfile = NULL;
    static AjPStr ftfmt = NULL;
    AjBool dofeat = ajFalse;

    static AjPStrTok handle2=NULL;
    static AjPStr    token2=NULL;
    static AjPStr    rdline2=NULL;
    
    AjBool ok;

    static AjPRegexp idexp = NULL;
    ajDebug ("seqReadNbrf\n");

    if(!token2)
    {
	token2 = ajStrNew();
	rdline2 = ajStrNew();
    }

    if (!ftfmt)
	ajStrAssC (&ftfmt, "pir");

    if (!idexp)
	idexp = ajRegCompC("^>(..)[>;]([^ \t\n]+)");

    if (!ajFileBuffGet (buff, &rdline))
	return ajFalse;

    ajDebug ("nbrf first line:\n%S", rdline);

    if (!ajRegExec (idexp, rdline))
    {
	ajFileBuffReset (buff);
	return ajFalse;
    }
    ajRegSubI(idexp, 1, &token);
    ajRegSubI(idexp, 2, &thys->Name);
    ajDebug ("parsed line name '%S' token '%S' token(1) '%c'\n",
	     thys->Name, token, ajStrChar(token, 0));

    if(seqin->Text)
	ajStrAssC(&thys->TextPtr,ajStrStr(rdline));

    /* token has the NBRF 2-char type. First char is the type
     ** and second char is Linear, Circular, or 1
     ** or, for GCG databases, thys is just '>>'
     */

    switch (toupper((ajint) ajStrChar(token, 0)))
    {
    case 'P':
    case 'F':
	ajSeqSetProt (thys);
	break;
    case 'B':				/* used by DIANA */
    case 'D':				/* DNA */
    case 'R':				/* RNA */
	ajSeqSetNuc (thys);
	break;
    default:
	ajWarn ("Unknown NBRF sequence type '%S'", token);
    }

    /* next line is the description, with no prefix */

    if (!ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr))
    {
	ajFileBuffReset (buff);
	return ajFalse;
    }

    (void) ajStrAss (&thys->Desc, rdline);
    if (ajStrChar(thys->Desc, -1) == '\n')
	(void) ajStrTrim (&thys->Desc, -1);

    /* read on, looking for feature and sequence lines */

    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
    while (ok && !ajStrPrefixC(rdline, ">"))
    {
	if (ajStrChar(rdline, 1) != ';')
	{
	    (void) seqAppend (&thys->Seq, rdline);
	}
	else
	{
	    if (ajStrPrefixC(rdline, "C;Accession:"))
	    {
		ajStrAssC(&rdline2,ajStrStr(rdline)+13);
		ajStrTokenAss (&handle2,rdline2, " ;\n\r");
		while (ajStrToken (&token2, &handle2, NULL))
		    seqAccSave (thys, token2);
	    }

	    if (ajStrChar(rdline,0) == 'R')
	    {			/* skip reference lines with no prefix */
		while((ok=ajFileBuffGetStore(buff,&rdline,
					     seqin->Text, &thys->TextPtr)))
		{
		    if(ajStrChar(rdline,1)==';' || ajStrChar(rdline,0)=='>')
			break;	/* X; line or next sequence */
		}
		if(ok)
		    continue;
	    }
	    else if (ajStrChar(rdline,0) == 'F')
	    {			/* feature lines */
	      if (seqinUfoLocal(seqin))
	      {
		if (!dofeat)
		{
		  dofeat = ajTrue;
		  ftfile = ajFileBuffNew();
		}
		ajFileBuffLoadS (ftfile, rdline);
		/* ajDebug ("NBRF FEAT saved line:\n%S", rdline); */
	      }
	    }
	}
	if (ok)
	    ok = ajFileBuffGetStore (buff, &rdline,
				     seqin->Text, &thys->TextPtr);
    }
    if (ajStrChar(thys->Seq, -1) == '*')
	(void) ajStrTrim(&thys->Seq, -1);

    if (ok)
	ajFileBuffClear (buff, 1);
    else
	ajFileBuffClear (buff, 0);

    if (dofeat)
    {
        ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF (ftfmt, thys->Name, "P", ftfile);
	ajDebug ("PIR FEAT TabIn %x\n", seqin->Ftquery);
	ftfile = NULL;			/* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeatRead (seqin->Ftquery);
	ajFeattableTrace(seqin->Fttable);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }

     return ajTrue;
}

/* @funcstatic seqReadGcg *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GCG format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGcg (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjBool ok;
    ajint len=0;
    AjPFileBuff buff = seqin->Filebuff;
    AjBool seqed=ajFalse;
    char *p=NULL;
  
    ok = ajFileBuffGet (buff, &rdline);
    if (!ok)
      return ajFalse;
    bufflines++;

    ajDebug ("seqReadGcg first line ok: %B\n", ok);

    /* test GCG 9.x file types if available */
    /* any type on the .. line will override this */

    if (ajStrPrefixC(rdline, "!!NA_SEQUENCE"))
	ajSeqSetNuc(thys);
    else if (ajStrPrefixC(rdline, "!!AA_SEQUENCE"))
	ajSeqSetProt(thys);

    if (!seqGcgDots (thys, seqin, &rdline, seqMaxGcglines, &len))
    {
	ajFileBuffReset (buff);
	return ajFalse;
    }
    ajDebug ("   Gcg dots read ok len: %d\n", len);


    while (ok &&  (ajSeqLen(thys) < len))
    {
	ok = ajFileBuffGet (buff, &rdline);
	if (ok)
	{
	    if(!seqed)
	    {
		p = ajStrStr(rdline);
		if(strpbrk(p,"<>"))
		    seqed = ajTrue;
		else
		    (void) seqAppend (&thys->Seq, rdline);
		bufflines++;
		continue;
	    }
	    else
	    {
		ajStrCleanWhite(&rdline);
		p = ajStrLen(rdline) + ajStrStr(rdline) - 1;
		if(*p=='>' || *p=='<')
		    seqed = ajFalse;
		++bufflines;
	    }
	}
	ajDebug ("line %d seqlen: %d ok: %B\n", bufflines, ajSeqLen(thys), ok);
    }
    ajDebug ("lines: %d ajSeqLen : %d len: %d ok: %B\n",
	     bufflines, ajSeqLen(thys), len, ok);
    ajFileBuffClear (buff, 0);


    return ok;
}

/* @funcstatic seqReadNcbi ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using NCBI format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNcbi (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr rdline = NULL;
    static AjPStr id = NULL;
    static AjPStr acc = NULL;
    static AjPStr sv = NULL;
    static AjPStr gi = NULL;
    static AjPStr desc = NULL;
  
    AjPFileBuff buff = seqin->Filebuff;

    ajint bufflines = 0;
    AjBool ok;

    ok = ajFileBuffGet (buff, &rdline);
    if (!ok)
      return ajFalse;

    (void) ajStrAssC(&id,"");
    (void) ajStrAssC(&acc,"");
    (void) ajStrAssC(&sv,"");
    (void) ajStrAssC(&gi,"");
    (void) ajStrAssC(&desc,"");


    if(!ajSeqParseNcbi(rdline,&id,&acc,&sv,&gi,&desc))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
  
    ajDebug("parsed id '%S' acc '%S' sv '%S' gi '%S' desc '%S'\n",
	    id, acc, sv, gi, desc);
    if (ajStrLen(gi))
      ajStrAssS(&thys->Gi, gi);

    if (ajStrLen(sv))
	(void) seqSvSave (thys, sv);

    if (ajStrLen(acc))
	(void) seqAccSave (thys, acc);

    seqSetName (&thys->Name, id);
    (void) ajStrAss (&thys->Desc, desc);



    if (ajStrLen(seqin->Inseq))
    {					/* we have a sequence to use */
	ajStrAssS (&thys->Seq, seqin->Inseq);
	if (seqin->Text)
	{
	  seqTextSeq(&thys->TextPtr, seqin->Inseq);
	}
	ajFileBuffClear (buff, 1);
    }
    else
    {
	ok = ajFileBuffGet (buff, &rdline);
	while (ok && !ajStrPrefixC(rdline, ">"))
	{
	    (void) seqAppend (&thys->Seq, rdline);
	    bufflines++;
	    ok = ajFileBuffGet (buff, &rdline);
	}
	if (ok)
	    ajFileBuffClear (buff, 1);
	else
	    ajFileBuffClear (buff, 0);
    }

    (void) ajStrTokenReset (&handle);

    return ajTrue;
}


/* @funcstatic seqReadSelex ***************************************************
**
** Read a Selex file. (temporary)
**
** @param [r] thys [AjPSeq] Sequence object
** @param [w] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSelex(AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    AjPSelex    selex;
    ajint       n     = 0;
    char        *p    = NULL;
    AjBool      ok    = ajFalse;
    AjBool      isseq = ajFalse;
    AjBool      named  = ajFalse;
    AjBool      head   = ajTrue;
    ajint       sqcnt  = 0;
    ajint       i;
    char        c='\0';
    AjBool      first=ajTrue;
    ajint       filestat;
    
    line = ajStrNew();

    
    if(!seqin->Selex)
    {
        if (ajFileBuffEof(buff) && ajFileStdin(ajFileBuffFile(buff)))
	  return ajFalse;
	ajFileBuffClear(buff,-1);
	ajFileBuffReset(buff);
	buff->Fpos = 0;
	filestat = ajFileSeek(buff->File, 0L, 0);
	ajDebug ("filestat %d\n", filestat);

	/* First count the sequences, and get any header information */
	while(!isseq && (ok=ajFileBuffGet(buff,&line)))
	{
	    if(first)
	    {
		first=ajFalse;
		if(!ajStrPrefixC(line,"#="))
		{
		    ajStrDel(&line);
		    ajFileBuffReset(buff);
		    return ajFalse;
		}
	    }
	    ajStrClean(&line);
	    p = ajStrStr(line);
	    if(!*p || *p=='#')
		continue;
	    else
		isseq = ajTrue;
	}
	if(!ok && !isseq)
	    return ajFalse;
	++n;

	ok = ajTrue;
	while(ok && ajFileBuffGet(buff,&line))
	{
	    ajStrClean(&line);
	    p = ajStrStr(line);
	    if(*p=='#')
		continue;
	    if(!*p)
		ok = ajFalse;
	    else
		++n;
	}

	ajFileBuffClear(buff,-1);
	ajFileBuffReset(buff);
	buff->Fpos = 0;
	ajFileSeek(buff->File, 0L, 0);
	selex = ajSelexNew(n);
    
	while(head && ajFileBuffGet(buff,&line))
	{
	    if(ajStrPrefixC(line,"#=RF") ||ajStrPrefixC(line,"#=CS"))
		break;
	    
	    if(ajStrPrefixC(line,"#="))
	    {
		head=seqSelexHeader(&selex,line,n,&named,&sqcnt);
		continue;
	    }
	    c = *ajStrStr(line);
	    if(c>='0')
		head = ajFalse;
	}

	/* Should now be at start of first block, whether RF or sequence */
	ajDebug("First Block Line: %S",line);
	
	ok = ajTrue;
	while(ok)
	{
	    seqSelexReadBlock(&selex,&named,n,&line,buff);
	    ok = ajFileBuffGet(buff,&line);
	}
	seqin->Selex = selex;
    }


    /* At this point the Selex structure is fully loaded */
    if(seqin->Selex->Count >= seqin->Selex->n)
    {
	ajStrDel(&line);
	return ajFalse;
    }

    i = seqin->Selex->Count;
    
    seqSelexCopy(&thys,seqin,i);

    ++seqin->Selex->Count;

    ajFileBuffClear(buff,0);
    
    ajStrDel(&line);
    return ajTrue;
}



/* @funcstatic seqReadStockholm ***********************************************
**
** Read a Stockholm file.
**
** @param [w] thys [AjPSeq] Stockholm input file
** @param [r] seqin [AjPSeqin] seqin object
** @return [AjBool] ajTrue if success
** @@
******************************************************************************/

static AjBool seqReadStockholm(AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff  = seqin->Filebuff;
    AjPStr      line  = NULL;
    AjPStr      word  = NULL;
    AjPStr      token = NULL;
    AjPStr      post  = NULL;
    AjBool      ok    = ajFalse;
    AjBool      bmf   = ajTrue;
    AjBool      dcf   = ajTrue;
    AjBool      drf   = ajTrue;
    AjBool      ccf   = ajTrue;
    AjBool      gsf   = ajTrue;
    AjBool      reff  = ajTrue;

    AjPRegexp    sexp  = NULL;
    AjPStockholm stock = NULL;

    ajint i = 0;
    ajint n = 0;
    ajlong lpos=0L;
    ajint  scnt=0;
    
    line = ajStrNew();

    if(!seqin->Stockholm)
    {
	lpos = ajFileTell(buff->File);
	ok=ajFileBuffGet(buff,&line);
	if (!ok)
	  return ajFalse;

	if(!ok || !ajStrPrefixC(line,"# STOCKHOLM 1.0"))
	{
/*	    ajFileSeek(buff->File,lpos,0);
	    ajFileBuffClear(buff,-1);*/
	    ajFileBuffReset(buff);
	    ajStrDel(&line);
	    return ajFalse;
	}
    
	while(ok && !ajStrPrefixC(line,"//") && !n)
	{
	    if(ajStrPrefixC(line,"#=GF SQ"))
		ajFmtScanS(line,"%*s%*s%d",&n);
	    ok=ajFileBuffGet(buff,&line);
	}
	if(!ok || ajStrPrefixC(line,"//"))
	{
	    ajFileSeek(buff->File,lpos,0);
	    ajFileBuffClear(buff,-1);
	    ajFileBuffReset(buff);
	    ajStrDel(&line);
	    return ajFalse;
	}

	ajFileSeek(buff->File,lpos,0);
	ajFileBuffClear(buff,-1);
	ajFileBuffReset(buff);

	ok=ajFileBuffGet(buff,&line);
	ok=ajFileBuffGet(buff,&line);
	stock = ajStockholmNew(n);

	word  = ajStrNew();
	token = ajStrNew();
	post  = ajStrNew();

	sexp = ajRegCompC("^([^ \t\n]+)[ \t]+([^ \t\n]+)[ \t]+");
	while(ok && !ajStrPrefixC(line,"//"))
	{
	    if(ajRegExec(sexp,line))
	    {
		ajRegSubI(sexp,1,&word);
		ajRegSubI(sexp,2,&token);
		ajRegPost(sexp,&post);
		ajStrRemoveNewline(&post);
		
		if(!ajStrCmpC(word,"#=GF"))
		{
		    if(!ajStrCmpC(token,"ID"))
			ajStrAssS(&stock->id,post);
		    else if(!ajStrCmpC(token,"AC"))
			ajStrAssS(&stock->ac,post);
		    else if(!ajStrCmpC(token,"DE"))
			ajStrAssS(&stock->de,post);
		    else if(!ajStrCmpC(token,"AU"))
			ajStrAssS(&stock->au,post);
		    else if(!ajStrCmpC(token,"AL"))
			ajStrAssS(&stock->al,post);
		    else if(!ajStrCmpC(token,"SE"))
			ajStrAssS(&stock->se,post);
		    else if(!ajStrCmpC(token,"TP"))
			ajStrAssS(&stock->se,post);
		    else if(!ajStrCmpC(token,"GA"))
			ajFmtScanS(post,"%d%d",&stock->ga[0],
				   &stock->ga[1]);
		    else if(!ajStrCmpC(token,"TC"))
			ajFmtScanS(post,"%f%f",&stock->tc[0],
				   &stock->tc[1]);
		    else if(!ajStrCmpC(token,"NC"))
			ajFmtScanS(post,"%f%f",&stock->nc[0],
				   &stock->nc[1]);
		    else if(!ajStrCmpC(token,"BM"))
		    {
			if(bmf)
			{
			    bmf = ajFalse;
			    ajStrAssS(&stock->bm,line);
			}
			else
			    ajStrApp(&stock->bm,line);
		    }
		    else if(!ajStrCmpC(token,"DC"))
		    {
			if(dcf)
			{
			    dcf = ajFalse;
			    ajStrAssS(&stock->dc,line);
			}
			else
			    ajStrApp(&stock->dc,line);
		    }
		    else if(!ajStrCmpC(token,"DR"))
		    {
			if(drf)
			{
			    drf = ajFalse;
			    ajStrAssS(&stock->dr,line);
			}
			else
			    ajStrApp(&stock->dr,line);
		    }
		    else if(!ajStrCmpC(token,"CC"))
		    {
			if(ccf)
			{
			    ccf = ajFalse;
			    ajStrAssS(&stock->cc,line);
			}
			else
			    ajStrApp(&stock->cc,line);
		    }
		    else if(*ajStrStr(token)=='R')
		    {
			if(reff)
			{
			    reff = ajFalse;
			    ajStrAssS(&stock->ref,line);
			}
			else
			    ajStrApp(&stock->ref,line);
		    }
		}

		if(!ajStrCmpC(word,"#=GS"))
		{
		    if(gsf)
		    {
			gsf = ajFalse;
			ajStrAssS(&stock->gs,line);
		    }
		    else
			ajStrApp(&stock->gs,line);
		}

		if(!ajStrCmpC(word,"#=GC"))
		{
		    if(!ajStrCmpC(token,"SS_cons"))
			ajStrAssS(&stock->sscons,post);
		    else if(!ajStrCmpC(token,"SA_cons"))
			ajStrAssS(&stock->sacons,post);
		}

	    }
	    else
	    {
		ajFmtScanS(line,"%S%S",&stock->name[scnt],&stock->str[scnt]);
		ajStrRemoveNewline(&stock->str[scnt]);
		++scnt;
	    }

	    ok = ajFileBuffGet(buff,&line);
	}

	ajStrDel(&word);
	ajStrDel(&token);
	ajStrDel(&post);
	ajRegFree(&sexp);
	seqin->Stockholm = stock;
    }


    /* At this point the Stockholm structure is fully loaded */
    if(seqin->Stockholm->Count >= seqin->Stockholm->n)
    {
	ajStrDel(&line);
	return ajFalse;
    }

    i = seqin->Stockholm->Count;

    seqStockholmCopy(&thys,seqin,i);

    ++seqin->Stockholm->Count;

    ajFileBuffClear(buff,0);
    

    ajStrDel(&line);
    return ajTrue;
}





/* @funcstatic seqSelexCopy ************************************************
**
** Copy Selex data to sequence object.
** Pad with gaps to make lengths equal.
**
** @param [w] thys [AjPSeq*] sequence object
** @param [r] seqin [AjPSeqin] seqin containing selex info
** @param [r] n [ajint] index into selex object
** @return [void]
** @@
******************************************************************************/

static void seqSelexCopy(AjPSeq *thys, AjPSeqin seqin, ajint n)
{
    AjPSeq pthis   = *thys;
    AjPSelex selex = seqin->Selex;
    AjPSelexdata sdata;
    
    ajStrAssS(&pthis->Seq, selex->str[n]);
    ajStrAssS(&pthis->Name, selex->name[n]);
    pthis->Weight = selex->sq[n]->wt;

    if(!(*thys)->Selexdata)
	(*thys)->Selexdata = ajSelexdataNew();
    
    sdata = (*thys)->Selexdata;

    ajStrAssS(&sdata->id,selex->id);
    ajStrAssS(&sdata->ac,selex->ac);
    ajStrAssS(&sdata->de,selex->de);
    ajStrAssS(&sdata->au,selex->au);
    ajStrAssS(&sdata->cs,selex->cs);
    ajStrAssS(&sdata->rf,selex->rf);
    ajStrAssS(&sdata->name,selex->name[n]);
    ajStrAssS(&sdata->str,selex->str[n]);
    ajStrAssS(&sdata->ss,selex->ss[n]);

    sdata->ga[0] = selex->ga[0];
    sdata->ga[1] = selex->ga[1];
    sdata->tc[0] = selex->tc[0];
    sdata->tc[1] = selex->tc[1];
    sdata->nc[0] = selex->nc[0];
    sdata->nc[1] = selex->nc[1];

    ajStrAssS(&sdata->sq->name,selex->sq[n]->name);

    ajStrAssS(&sdata->sq->ac,selex->sq[n]->ac);
    ajStrAssS(&sdata->sq->source,selex->sq[n]->source);
    ajStrAssS(&sdata->sq->de,selex->sq[n]->de);

    sdata->sq->wt    = selex->sq[n]->wt;
    sdata->sq->start = selex->sq[n]->start;
    sdata->sq->stop  = selex->sq[n]->stop;
    sdata->sq->len   = selex->sq[n]->len;

    return;
}


/* @funcstatic seqStockholmCopy ********************************************
**
** Copy Stockholm data to sequence object.
** Pad with gaps to make lengths equal.
**
** @param [w] thys [AjPSeq*] sequence object
** @param [r] seqin [AjPSeqin] seqin containing selex info
** @param [r] n [ajint] index into stockholm object
** @return [void]
** @@
******************************************************************************/

static void seqStockholmCopy(AjPSeq *thys, AjPSeqin seqin, ajint n)
{
    AjPSeq pthis   = *thys;
    AjPStockholm stock = seqin->Stockholm;
    AjPStockholmdata sdata;
    
    ajStrAssS(&pthis->Seq, stock->str[n]);
    ajStrAssS(&pthis->Name, stock->name[n]);


    if(!(*thys)->Stock)
	(*thys)->Stock = ajStockholmdataNew();
    
    sdata = (*thys)->Stock;

    ajStrAssS(&sdata->id,stock->id);
    ajStrAssS(&sdata->ac,stock->ac);
    ajStrAssS(&sdata->de,stock->de);
    ajStrAssS(&sdata->au,stock->au);
    ajStrAssS(&sdata->al,stock->al);
    ajStrAssS(&sdata->tp,stock->tp);
    ajStrAssS(&sdata->se,stock->se);
    ajStrAssS(&sdata->gs,stock->gs);
    ajStrAssS(&sdata->dc,stock->dc);
    ajStrAssS(&sdata->dr,stock->dr);
    ajStrAssS(&sdata->cc,stock->cc);
    ajStrAssS(&sdata->ref,stock->ref);
    ajStrAssS(&sdata->sacons,stock->sacons);
    ajStrAssS(&sdata->sscons,stock->sscons);
    sdata->ga[0] = stock->ga[0];
    sdata->ga[1] = stock->ga[1];
    sdata->tc[0] = stock->tc[0];
    sdata->tc[1] = stock->tc[1];
    sdata->nc[0] = stock->nc[0];
    sdata->nc[1] = stock->nc[1];

    return;
}


/* @funcstatic seqSelexAppend ************************************************
**
** Append sequence and related Selex info to selex object.
** Pad with gaps to make lengths equal.
**
** @param [r] src [AjPStr] source line from Selex file
** @param [w] dest [AjPStr*] Destination in Selex object
** @param [r] beg  [ajint] start of info in src
** @param [r] end  [ajint] end of info in src
** @return [void]
** @@
******************************************************************************/

static void seqSelexAppend(AjPStr src, AjPStr *dest, ajint beg, ajint end)
{
    char *p=NULL;
    char c;
    ajint len;
    ajint i;
    ajint pad=0;

    len = end-beg+1;
    p = ajStrStr(src);

    if(beg>=ajStrLen(src))
    {
	for(i=0;i<len;++i)
	    ajStrAppK(dest,'-');
	return;
    }

    p += beg;
    pad = end - ajStrLen(src) + 2;
    
    while((c=*p) && *p!='\n')
    {
	if(c=='.' || c=='_' || c==' ')
	    c='-';
	ajStrAppK(dest,c);
	++p;
    }

    for(i=0;i<pad;++i)
	ajStrAppK(dest,'-');

    return;
}


/* @funcstatic seqSelexHeader ************************************************
**
** Load a Selex object with header information for a single line
**
** @param [w] thys [AjPSelex*] Selex object
** @param [r] line [AjPStr] Selex header line
** @param [r] n  [ajint] Number of sequences in Selex file
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [w] sqcnt  [ajint*] Number of SQ names read
** @return [AjBool] ajTrue if the line contained header information
** @@
******************************************************************************/

static AjBool seqSelexHeader(AjPSelex *thys, AjPStr line, ajint n,
			     AjBool *named, ajint *sqcnt)
{
    AjPSelex pthis = *thys;
    AjPStrTok token=NULL;
    AjPStr handle=NULL;

    if(ajStrPrefixC(line,"#=ID"))
    {
	ajFmtScanS(line,"#=ID %S",&pthis->id);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AC"))
    {
	ajFmtScanS(line,"#=AC %S",&pthis->ac);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=DE"))
    {
	ajStrAssC(&pthis->de,ajStrStr(line)+5);
	ajStrClean(&pthis->de);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=AU"))
    {
	ajStrAssC(&pthis->au,ajStrStr(line)+5);
	ajStrClean(&pthis->au);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=GA"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->ga[0],&pthis->ga[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=TC"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->tc[0],&pthis->tc[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=NC"))
    {
	ajFmtScanS(line,"%*s %f %f",&pthis->nc[0],&pthis->nc[1]);
	return ajTrue;
    }
    else if(ajStrPrefixC(line,"#=SQ"))
    {
	handle = ajStrNew();
	token = ajStrTokenInit(line," \t\n");
	ajStrToken(&handle,&token,NULL);
	
	ajStrToken(&pthis->sq[*sqcnt]->name,&token,NULL);
	ajStrAssS(&pthis->name[*sqcnt],pthis->sq[*sqcnt]->name);
	
	ajStrToken(&handle,&token,NULL);
	ajStrToFloat(handle,&pthis->sq[*sqcnt]->wt);

	ajStrToken(&handle,&token,NULL);
	ajStrAssS(&pthis->sq[*sqcnt]->source,handle);

	ajStrToken(&handle,&token,NULL);
	ajStrAssS(&pthis->sq[*sqcnt]->ac,handle);

	ajStrToken(&handle,&token,NULL);
	ajFmtScanS(handle,"%d..%d:%d",&pthis->sq[*sqcnt]->start,
		   &pthis->sq[*sqcnt]->stop,&pthis->sq[*sqcnt]->len);

	ajStrToken(&handle,&token,"\n");
	ajStrAssS(&pthis->sq[*sqcnt]->de,handle);

	ajStrTokenClear(&token);
	ajStrDel(&handle);
	*named = ajTrue;
	++(*sqcnt);
	return ajTrue;
    }
    

    return ajFalse;
}


/* @funcstatic seqSelexPos ************************************************
**
** Find start and end positions of sequence & related Selex information
**
** @param [r] line [AjPStr] Selex sequence or related line
** @param [w] begin  [ajint*] start pos
** @param [w] end  [ajint*] end pos
** @return [void]
** @@
******************************************************************************/

static void seqSelexPos(AjPStr line, ajint *begin, ajint *end)
{
    ajint pos = 0;
    ajint len   = 0;
    
    char  *p;

    /*
     *  Selex sequence info can start any number of spaces
     *  after the names so we need to find out where to
     *  start counting chars from and where to end
     */
    len = ajStrLen(line) - 1;
    pos = len -1;
    *end = (pos > *end) ? pos : *end;
    p = ajStrStr(line);

    while(*p && *p!=' ')
	++p;
    while(*p && *p==' ')
	++p;
    if(p)
	pos = p - ajStrStr(line);
    *begin = (pos < *begin) ? pos : *begin;

    
    return;
}


/* @funcstatic seqSelexReadBlock **********************************************
**
** Read a block of sequence information from a selex file
**
** @param [w] thys [AjPSelex*] Selex object
** @param [w] named  [AjBool*] Whether names of sequences have been read
** @param [r] n  [ajint] Number of sequences in Selex file
** @param [rw] line [AjPStr*] Line from Selex file
** @param [r] buff  [AjPFileBuff] Selex file buffer
** @return [AjBool] ajTrue if success
** @@
******************************************************************************/

static AjBool seqSelexReadBlock(AjPSelex *thys, AjBool *named, ajint n,
				AjPStr *line, AjPFileBuff buff)
{
    AjPSelex pthis = *thys;
    AjPStr *seqs=NULL;
    AjPStr *ss=NULL;
    
    AjPStr rf=NULL;
    AjPStr cs=NULL;
    ajint  i;
    ajint  begin;
    ajint  end;
    AjBool ok;
    ajint  cnt;
    AjPStr tmp=NULL;
    AjBool haverf=ajFalse;
    AjBool havecs=ajFalse;
    AjBool havess=ajFalse;

    begin = INT_MAX;
    end   = -(INT_MAX);

    tmp = ajStrNew();
    rf = ajStrNew();
    cs = ajStrNew();
    AJCNEW(seqs,n);
    AJCNEW(ss,n);
    for(i=0;i<n;++i)
    {
	seqs[i] = ajStrNew();
	ss[i]  = ajStrNew();
    }
    
    ok = ajTrue;
    cnt = 0;
    
    while(ok)
    {
	seqSelexPos(*line,&begin,&end);
	if(ajStrPrefixC(*line,"#=RF"))
	{
	    haverf=ajTrue;
	    ajStrAssS(&rf,*line);
	}
	if(ajStrPrefixC(*line,"#=CS"))
	{
	    havecs=ajTrue;
	    ajStrAssS(&cs,*line);
	}
	if(ajStrPrefixC(*line,"#=SS"))
	{
	    havess=ajTrue;
	    ajStrAssS(&ss[--cnt],*line);
	    ++cnt;
	}

	if(!ajStrPrefixC(*line,"#"))
	{
	    if(!*named)
	    {
		ajFmtScanS(*line,"%S",&pthis->name[cnt]);
		ajStrAssS(&pthis->sq[cnt]->name,pthis->name[cnt]);
	    }
	    else
	    {
		ajFmtScanS(*line,"%S",&tmp);
		if(!ajStrPrefix(pthis->name[cnt],tmp))
		    ajWarn("Sequence names do not match [%S %S]",
			   pthis->name[cnt],tmp);
	    }
	    
	    ajStrAssS(&seqs[cnt],*line);
	    ++cnt;
	}

	ok = ajFileBuffGet(buff,line);
	if(ajStrPrefixC(*line,"\n"))
	    ok = ajFalse;
    }
    

    if(haverf)
	seqSelexAppend(rf,&pthis->rf,begin,end);
    if(havecs)
	seqSelexAppend(cs,&pthis->cs,begin,end);
    for(i=0;i<n;++i)
    {
	seqSelexAppend(seqs[i],&pthis->str[i],begin,end);
	if(havess)
	    seqSelexAppend(ss[i],&pthis->ss[i],begin,end);
    }
    

    for(i=0;i<n;++i)
    {
	ajStrDel(&seqs[i]);
	ajStrDel(&ss[i]);
    }
    AJFREE(seqs);
    AJFREE(ss);

    ajStrDel(&rf);
    ajStrDel(&cs);
    ajStrDel(&tmp);

    *named = ajTrue;
    
    return ajTrue;
}


/* @funcstatic seqReadStaden **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Staden experiment file format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadStaden (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    static AjPStr token = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;
    static AjPRegexp idexp = NULL;

    if (!idexp)
	idexp = ajRegCompC ("[<]([^>-]+)[-]*[>]");

    if (!ajFileBuffGet (buff, &rdline))
	return ajFalse;
    bufflines++;

    if (ajRegExec(idexp, rdline))
    {
	(void) ajRegSubI (idexp, 1, &token);
	(void) seqSetName (&thys->Name, token);
	ajDebug("seqReadStaden name '%S' token '%S'\n",
		thys->Name, token);
	(void) ajRegPost (idexp, &token);
	(void) seqAppend (&thys->Seq, token);
    }
    else
    {
	(void) seqSetName (&thys->Name, thys->Filename);
	(void) seqAppend (&thys->Seq, rdline);
    }
    
    while (ajFileBuffGet (buff, &rdline))
    {
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
    }

    ajFileBuffClear (buff, 0);

    if (!bufflines) return ajFalse;
    return ajTrue;
}

/* @funcstatic seqReadText ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using plain text format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadText (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;

    ajDebug ("seqReadText\n");

    while (ajFileBuffGet (buff, &rdline))
    {
	ajDebug ("read '%S'\n", rdline);
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
    }

    ajDebug ("read %d lines\n", bufflines);
    ajFileBuffClear (buff, 0);

    if (!bufflines) return ajFalse;
    return ajTrue;
}

/* @funcstatic seqReadRaw ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using raw format, which accepts only alphanumeric and whietspace
** characters and rejects anything else.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadRaw (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;
    static AjPRegexp rawexp = NULL;

    ajDebug ("seqReadRaw\n");

    if (!rawexp)
	rawexp = ajRegCompC("[^A-Za-z0-9 \t\n\r]");

    while (ajFileBuffGet (buff, &rdline))
    {
	ajDebug ("read '%S'\n", rdline);
	if (ajRegExec(rawexp, rdline))
	{
	    ajDebug("Bad character found in line: %S\n", rdline);
	    ajFileBuffReset (buff);
	    return ajFalse;
	}
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
    }

    ajDebug ("read %d lines\n", bufflines);
    ajFileBuffClear (buff, 0);

    if (!bufflines) return ajFalse;
    return ajTrue;
}

/* @funcstatic seqReadIg ******************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using IntelliGenetics format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadIg (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjBool ok = ajTrue;

    do
    {
	ok = ajFileBuffGet (buff, &rdline); /* skip comments with ';' prefix */
	bufflines++;
    } while (ok && ajStrPrefixC (rdline, ";"));

    if (!ok)
	return ajFalse;

    (void) ajStrAssS (&thys->Name, rdline);
    (void) ajStrTrim (&thys->Name, -1);
    bufflines++;

    while (ajFileBuffGet (buff, &rdline) && !ajStrPrefixC(rdline, "\014"))
    {
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
    }

    ajFileBuffClear (buff, 0);

    return ajTrue;
}

/* @funcstatic seqReadClustal *************************************************
**
** Tries to read input in Clustal ALN format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadClustal (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    static AjPStr seqstr = NULL;
    ajint bufflines = 0;
    AjBool ok = ajFalse;
    ajint iseq = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjPTable alntable = NULL;
    SeqPMsfItem alnitem = NULL;
    AjPList alnlist = NULL;
    SeqPMsfData alndata = NULL;
    static AjPRegexp blankexp = NULL;
    static AjPRegexp markexp = NULL;
    static AjPRegexp seqexp = NULL;
    ajint i;

    ajDebug("seqReadClustal seqin->Data %x\n", seqin->Data);

    if (!blankexp)
	blankexp = ajRegCompC ("^[ \t\n\r]*$");

    if (!markexp)
	markexp = ajRegCompC ("^[ \t\n\r]"); /* space or empty line */

    if (!seqexp)
	seqexp = ajRegCompC ("^([^ \t\n\r]+)");

    if (!seqin->Data)
    {					/* start of file */
	ok = ajFileBuffGet (buff, &rdline);
	bufflines++;
	if (!ok)
	  return ajFalse;

	ajDebug("first line:\n'%S'\n", rdline);

	if (!ajStrPrefixC(rdline, "CLUSTAL")) {	/* first line test */
	    ajFileBuffReset (buff);
	    return ajFalse;
	}

	ajDebug ("first line OK: '%S'\n", rdline);

	while (ok)
	{				/* skip blank lines */
	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;
	    if (!ajRegExec(blankexp, rdline))
		break;
	}
	if (!ok)
	{
	    ajDebug ("FAIL (blankexp only)\n");
	    ajFileBuffReset (buff);
	    return ajFalse;
	}

	seqin->Data = AJNEW0(alndata);
	alndata->Table = alntable = ajTableNew (0, ajStrTableCmp, ajStrTableHash);
	alnlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajTrue;
	while (ok && !ajRegExec (markexp, rdline))
	{				/* first set - create table */
	    if (!ajRegExec (seqexp, rdline))
	    {
		ajDebug ("FAIL (not seqexp): '%S'\n", rdline);
		ajFileBuffReset (buff);
		return ajFalse;
	    }
	    AJNEW0(alnitem);
	    ajRegSubI (seqexp, 1, &alnitem->Name);
	    alnitem->Weight = 1.0;
	    (void) ajRegPost (seqexp, &seqstr);
	    (void) seqAppend (&alnitem->Seq, seqstr);

	    (void) ajTablePut(alntable, alnitem->Name, alnitem);
	    ajListstrPushApp (alnlist, alnitem->Name);
	    iseq++;
	    ajDebug ("first set %d: '%S'\n", iseq, rdline);

	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;
	}

	ajDebug ("Header has %d sequences\n", iseq);
	ajListstrTrace (alnlist);
	ajTableTrace (alntable);
	ajTableMap (alntable, seqMsfTabList, NULL);

	alndata->Names = AJCALLOC (iseq, sizeof(*alndata->Names));
	for (i=0; i < iseq; i++)
	{
	    (void) ajListstrPop (alnlist, &alndata->Names[i]);
	    ajDebug ("list [%d] '%S'\n", i, alndata->Names[i]);
	}
	ajListstrFree(&alnlist);

	while (ajFileBuffGet (buff, &rdline))
	{				/* now read the rest */
	    bufflines++;
	    (void) seqClustalReadseq(rdline, alntable);
	}

	ajTableMap (alntable, seqMsfTabList, NULL);
	alndata->Nseq = iseq;
	alndata->Count = 0;
	alndata->Bufflines = bufflines;
	ajDebug("ALN format read %d lines\n", bufflines);
    }

    alndata = seqin->Data;
    alntable = alndata->Table;
    if (alndata->Count >=alndata->Nseq)
    {					/* all done */
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMap(alntable, seqMsfTabDel, NULL);
	ajTableFree(&alntable);
	AJFREE(alndata->Names);
	AJFREE(alndata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = alndata->Count;
    ajDebug ("returning [%d] '%S'\n", i, alndata->Names[i]);
    alnitem = ajTableGet(alntable, alndata->Names[i]);
    (void) ajStrAss(&thys->Name, alndata->Names[i]);
    ajStrDel(&alndata->Names[i]);

    thys->Weight = alnitem->Weight;
    (void) ajStrAss (&thys->Seq, alnitem->Seq);
    ajStrDel(&alnitem->Seq);

    alndata->Count++;

    return ajTrue;
}

/* @funcstatic seqClustalReadseq ******************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the alntable structure.
**
** @param [P] rdline [AjPStr] Line from input file.
** @param [P] msftable [AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqClustalReadseq (AjPStr rdline, AjPTable msftable)
{
  static AjPRegexp seqexp = NULL;
  SeqPMsfItem msfitem;
  static AjPStr token = NULL;
  static AjPStr seqstr = NULL;

  if (!seqexp)
    seqexp = ajRegCompC("^[^ \t\n\r]+"); /* must be at start of line */

  if (!ajRegExec(seqexp, rdline))
    return ajFalse;

  ajRegSubI (seqexp, 0, &token);
  msfitem = ajTableGet(msftable, token);
  if (!msfitem)
  {
    ajStrDel(&token);
    return ajFalse;
  }

  (void) ajRegPost (seqexp, &seqstr);
  (void) seqAppend (&msfitem->Seq, seqstr);

  return ajTrue;
}

/* @funcstatic seqReadPhylip *************************************************
**
** Tries to read input in Phylip interleaved format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadPhylip (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    static AjPStr seqstr = NULL;
    static AjPStr tmpstr = NULL;
    ajint bufflines = 0;
    AjBool ok = ajFalse;
    ajint iseq = 0;
    ajint jseq = 0;
    ajint len = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjPTable phytable = NULL;
    SeqPMsfItem phyitem = NULL;
    AjPList phylist = NULL;
    SeqPMsfData phydata = NULL;
    static AjPRegexp topexp = NULL;
    static AjPRegexp headexp = NULL;
    static AjPRegexp seqexp = NULL;
    ajint i;

    ajDebug("seqReadPhylip seqin->Data %x\n", seqin->Data);

    if (!topexp)
	topexp = ajRegCompC ("^ *([0-9]+) +([0-9]+)");

    if (!headexp)
	headexp = ajRegCompC ("^(..........) ?"); /* 10 chars */

    if (!seqexp)
	seqexp = ajRegCompC ("^[ \t\n\r]*$");

    if (!seqin->Data)
    {	/* start of file */
	ok = ajFileBuffGet (buff, &rdline);
	if (!ok)
	  return ajFalse;
	bufflines++;

	ajDebug("first line:\n'%S'\n", rdline);

	if (!ajRegExec(topexp, rdline)) { /* first line test */
	  ajFileBuffReset (buff);
	  return ajFalse;
	}

	ajRegSubI(topexp, 1, &tmpstr);
	(void) ajStrToInt (tmpstr, &iseq);
	ajRegSubI(topexp, 2, &tmpstr);
	(void) ajStrToInt (tmpstr, &len);
	ajDebug ("first line OK: '%S' iseq; %d len: %d\n",
		 rdline, iseq, len);

	seqin->Data = AJNEW0(phydata);
	phydata->Table = phytable = ajTableNew (0, ajStrTableCmp,
						ajStrTableHash);
	phylist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajFileBuffGet (buff, &rdline);
	bufflines++;
	while (ok && (jseq < iseq))
	{   /* first set - create table */
	    if (!ajRegExec (headexp, rdline))
	    {
		ajDebug ("FAIL (not headexp): '%S'\n", rdline);
		ajFileBuffReset(buff);
		return ajFalse;
	    }
	    AJNEW0(phyitem);
	    ajRegSubI (headexp, 1, &phyitem->Name);
	    phyitem->Weight = 1.0;
	    (void) ajRegPost (headexp, &seqstr);
	    (void) seqAppend (&phyitem->Seq, seqstr);

	    (void) ajTablePut(phytable, phyitem->Name, phyitem);
	    ajListstrPushApp (phylist, phyitem->Name);
	    jseq++;
	    ajDebug ("first set %d: '%S'\n", jseq, rdline);

	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;
	}

	ajDebug ("Header has %d sequences\n", jseq);
	ajListstrTrace (phylist);
	ajTableTrace (phytable);
	ajTableMap (phytable, seqMsfTabList, NULL);

	phydata->Names = AJCALLOC (iseq, sizeof(*phydata->Names));
	for (i=0; i < iseq; i++)
	{
	    (void) ajListstrPop (phylist, &phydata->Names[i]);
	    ajDebug ("list [%d] '%S'\n", i, phydata->Names[i]);
	}
	ajListstrFree(&phylist);

	jseq=0;
	while (ajFileBuffGet (buff, &rdline))
	{   /* now read the rest */
	    bufflines++;
	    if (seqPhylipReadseq(rdline, phytable, phydata->Names[jseq]))
	    {
		jseq++;
		if (jseq == iseq) jseq = 0;
	    }
	}
	if (jseq)
	    ajWarn ("seqReadPhylip %d sequences partly read at end", jseq);

	ajTableMap (phytable, seqMsfTabList, NULL);
	phydata->Nseq = iseq;
	phydata->Count = 0;
	phydata->Bufflines = bufflines;
	ajDebug("PHYLIP format read %d lines\n", bufflines);
    }

    phydata = seqin->Data;
    phytable = phydata->Table;
    if (phydata->Count >=phydata->Nseq)
    {
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMap(phytable, seqMsfTabDel, NULL);
	ajTableFree(&phytable);
	AJFREE(phydata->Names);
	AJFREE(phydata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = phydata->Count;
    ajDebug ("returning [%d] '%S'\n", i, phydata->Names[i]);
    phyitem = ajTableGet(phytable, phydata->Names[i]);
    (void) ajStrAss(&thys->Name, phydata->Names[i]);
    ajStrDel(&phydata->Names[i]);

    thys->Weight = phyitem->Weight;
    (void) ajStrAss (&thys->Seq, phyitem->Seq);
    ajStrDel(&phyitem->Seq);

    phydata->Count++;

    return ajTrue;
}

/* @funcstatic seqPhylipReadseq ******************************************
**
** Reads sequence from the input line, and appends the sequence data
** to the named sequence in the phytable structure.
**
** @param [P] rdline [AjPStr] Line from input file.
** @param [P] phytable [AjPTable] MSF format sequence table.
** @param [r] token [AjPStr] Name of sequence so it can append
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqPhylipReadseq (AjPStr rdline, AjPTable phytable,
				AjPStr token)
{
    static AjPRegexp seqexp = NULL;
    SeqPMsfItem phyitem;

    if (!seqexp)
	seqexp = ajRegCompC("[^ \t\n\r]");

    if (!ajRegExec(seqexp, rdline))
	return ajFalse;

    phyitem = ajTableGet(phytable, token);
    if (!phyitem)
	return ajFalse;

    (void) seqAppend (&phyitem->Seq, rdline);

    return ajTrue;
}

/* @funcstatic seqReadHennig86 ************************************************
**
** Tries to read input in Hennig86 format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadHennig86 (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    static AjPStr seqstr = NULL;
    static AjPStr tmpstr = NULL;
    ajint bufflines = 0;
    AjBool ok = ajFalse;
    ajint iseq = 0;
    ajint len = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjPTable fmttable = NULL;
    SeqPMsfItem fmtitem = NULL;
    AjPList fmtlist = NULL;
    SeqPMsfData fmtdata = NULL;
    char *cp;
    static AjPRegexp blankexp = NULL;
    static AjPRegexp markexp = NULL;
    static AjPRegexp seqexp = NULL;
    static AjPRegexp topexp = NULL;
    static AjPRegexp headexp = NULL;

    ajint i;
    ajint jseq=0;

    ajDebug("seqReadHennig86 seqin->Data %x\n", seqin->Data);

    if (!headexp)
	headexp = ajRegCompC ("[^1-4? \t]");

    if (!topexp)
	topexp = ajRegCompC ("^ *([0-9]+) +([0-9]+)");

    if (!blankexp)
	blankexp = ajRegCompC ("^[ \t\n\r]*$");

    if (!markexp)
	markexp = ajRegCompC ("^[ \t]");

    if (!seqexp)
	seqexp = ajRegCompC ("^([^ \t\n\r]+)");

    if (!seqin->Data)
    {					/* start: load in file */
	ok = ajFileBuffGet (buff, &rdline);
	if (!ok)
	  return ajFalse;
	bufflines++;

	ajDebug("first line:\n'%S'\n", rdline);

	if (!ajStrPrefixC(rdline, "xread")) { /* first line test */
	    ajFileBuffReset (buff);
	    return ajFalse;
	}

	ajDebug ("first line OK: '%S'\n", rdline);

	/* skip title line */
	for (i=0; i<2; i++)
	{
	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;

	    if (!ok)
	    {
		ajDebug ("FAIL (bad header)\n");
		ajFileBuffReset (buff);
		return ajFalse;
	    }
	}

	if (!ajRegExec(topexp, rdline)) /* first line test */
	    return ajFalse;

	ajRegSubI(topexp, 1, &tmpstr);
	(void) ajStrToInt (tmpstr, &iseq);
	ajRegSubI(topexp, 2, &tmpstr);
	(void) ajStrToInt (tmpstr, &len);
	ajDebug ("first line OK: '%S' iseq; %d len: %d\n",
		 rdline, iseq, len);

	seqin->Data = AJNEW0(fmtdata);
	fmtdata->Table = fmttable = ajTableNew (0, ajStrTableCmp, ajStrTableHash);
	fmtlist = ajListstrNew();
	seqin->Filecount = 0;

	ok = ajFileBuffGet (buff, &rdline);
	bufflines++;
	while (ok && (jseq < iseq))
	{				/* first set - create table */
	    if (!ajRegExec (headexp, rdline))
	    {
		ajDebug ("FAIL (not headexp): '%S'\n", rdline);
		return ajFalse;
	    }
	    AJNEW0(fmtitem);
	    ajStrAssS (&fmtitem->Name, rdline);
	    fmtitem->Weight = 1.0;
	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;
	    while (ok && ajRegExec (seqexp, rdline))
	    {
		(void) ajRegPost (seqexp, &seqstr);
		for (cp = ajStrStr(seqstr); cp; cp++)
		{
		    switch (*cp)
		    {
		    case 0: *cp = 'A';break;
		    case 1: *cp = 'T';break;
		    case 2: *cp = 'G';break;
		    case 3: *cp = 'C';break;
		    default: *cp = '.';break;
		    }
		}
		(void) seqAppend (&fmtitem->Seq, seqstr);
	    }

	    (void) ajTablePut(fmttable, fmtitem->Name, fmtitem);
	    ajListstrPushApp (fmtlist, fmtitem->Name);
	    jseq++;
	    ajDebug ("first set %d: '%S'\n", jseq, rdline);

	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;
	}

	ajDebug ("Header has %d sequences\n", iseq);
	ajListstrTrace (fmtlist);
	ajTableTrace (fmttable);
	ajTableMap (fmttable, seqMsfTabList, NULL);

	fmtdata->Names = AJCALLOC (iseq, sizeof(*fmtdata->Names));
	for (i=0; i < iseq; i++)
	{
	    (void) ajListstrPop (fmtlist, &fmtdata->Names[i]);
	    ajDebug ("list [%d] '%S'\n", i, fmtdata->Names[i]);
	}
	ajListstrFree(&fmtlist);

	while (ajFileBuffGet (buff, &rdline))
	{				/* now read the rest */
	    bufflines++;
	    (void) seqHennig86Readseq(rdline, fmttable);
	}

	ajTableMap (fmttable, seqMsfTabList, NULL);
	fmtdata->Nseq = iseq;
	fmtdata->Count = 0;
	fmtdata->Bufflines = bufflines;
	ajDebug("... format read %d lines\n", bufflines);
    }

    /* processing entries */

    fmtdata = seqin->Data;
    fmttable = fmtdata->Table;
    if (fmtdata->Count >=fmtdata->Nseq)
    {					/* all done */
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMap(fmttable, seqMsfTabDel, NULL);
	ajTableFree(&fmttable);
	AJFREE(fmtdata->Names);
	AJFREE(fmtdata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = fmtdata->Count;
    ajDebug ("returning [%d] '%S'\n", i, fmtdata->Names[i]);
    fmtitem = ajTableGet(fmttable, fmtdata->Names[i]);
    (void) ajStrAss(&thys->Name, fmtdata->Names[i]);
    ajStrDel(&fmtdata->Names[i]);

    thys->Weight = fmtitem->Weight;
    (void) ajStrAss (&thys->Seq, fmtitem->Seq);
    ajStrDel(&fmtitem->Seq);

    fmtdata->Count++;

    return ajTrue;
}

/* @funcstatic seqHennig86Readseq ******************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the fmttable structure.
**
** @param [P] rdline [AjPStr] Line from input file.
** @param [P] msftable [AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqHennig86Readseq (AjPStr rdline, AjPTable msftable)
{
    static AjPRegexp seqexp = NULL;
    SeqPMsfItem msfitem;
    static AjPStr token = NULL;
    static AjPStr seqstr = NULL;

    if (!seqexp)
	seqexp = ajRegCompC("^[^ \t\n\r]+"); /* must be at start of line */

    if (!ajRegExec(seqexp, rdline))
	return ajFalse;

    ajRegSubI (seqexp, 0, &token);
    msfitem = ajTableGet(msftable, token);
    if (!msfitem)
    {
	ajStrDel(&token);
	return ajFalse;
    }

    (void) ajRegPost (seqexp, &seqstr);
    (void) seqAppend (&msfitem->Seq, seqstr);

    return ajTrue;
}

/* @funcstatic seqReadTreecon *************************************************
**
** Tries to read input in Treecon format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadTreecon (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadJackknifer **********************************************
**
** Tries to read input in Jackknifer format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadJackknifer (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadJackknifernon *******************************************
**
** Tries to read input in Jackknifer non-interleaved format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadJackknifernon (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadNexus *************************************************
**
** Tries to read input in Nexus format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNexus (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadNexusnon ************************************************
**
** Tries to read input in Nexus non-interleaved format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadNexusnon (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadMega *************************************************
**
** Tries to read input in Mega format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMega (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadMeganon *************************************************
**
** Tries to read input in Mega non-interleaved format.
**
** To be implemented
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMeganon (AjPSeq thys, AjPSeqin seqin)
{
    return ajFalse;
}

/* @funcstatic seqReadCodata **************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using CODATA format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadCodata (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjBool ok = ajTrue;

    if (!ajFileBuffGet (buff, &rdline))
	return ajFalse;

    bufflines++;

    ajDebug ("first line '%S'\n", rdline);

    if (!ajStrPrefixC(rdline, "ENTRY "))
    {
	ajFileBuffReset (buff);
	return ajFalse;
    }
    (void) ajStrTokenAss (&handle, rdline, " \n\r");
    (void) ajStrToken (&token, &handle, NULL); /* 'ENTRY' */
    (void) ajStrToken (&token, &handle, NULL); /* entry name */

    seqSetName (&thys->Name, token);

    ok = ajFileBuffGet (buff, &rdline);

    while (ok && !ajStrPrefixC(rdline, "SEQUENCE"))
    {
	bufflines++;
	if (ajStrPrefixC(rdline, "ACCESSION "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ;\n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'AC' */
	    (void) ajStrToken (&token, &handle, NULL); /* accnum */
	    (void) seqAccSave (thys, token);
	}
	if (ajStrPrefixC(rdline, "TITLE "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ");
	    (void) ajStrToken (&token, &handle, NULL); /* 'DE' */
	    (void) ajStrToken (&token, &handle, "\n\r"); /* desc */
	    while (ok && ajStrPrefixC(rdline, " "))
	    {
		bufflines++;
		(void) ajStrTokenAss (&handle, rdline, " ");
		(void) ajStrToken (&token, &handle, "\n\r"); 
		(void) ajStrAppC (&thys->Desc, " ");
		(void) ajStrApp (&thys->Desc, token);
		ok = ajFileBuffGet (buff, &rdline);
	    }
	}
	ok = ajFileBuffGet (buff, &rdline);
    }

    ok = ajFileBuffGet (buff, &rdline);
    while (ok && !ajStrPrefixC(rdline, "///"))
    {
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
	ok = ajFileBuffGet (buff, &rdline);
    }
    ajFileBuffClear (buff, 0);

    (void) ajStrTokenReset (&handle);

    return ajTrue;
}

/* @funcstatic seqReadAcedb *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ACEDB format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAcedb (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjBool ok = ajTrue;

    ajDebug("seqReadAcedb\n");

    do
    {
	ok = ajFileBuffGet (buff, &rdline);
	bufflines++;
    } while (ok && (ajStrPrefixC (rdline, "//") || ajStrPrefixC(rdline, "\n")));

    if (!ok)
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }
    ajDebug("first line:\n'%S'\n", rdline);


    (void) ajStrTokenAss (&handle, rdline, " \n\r");
    (void) ajStrToken (&token, &handle, " \t"); /* 'DNA' or 'Peptide'*/
    ajDebug("Token 1 '%S'\n", token);

    if (ajStrMatchCaseC (token, "Peptide"))
    {
	ajDebug("Protein\n");
	ajSeqSetProt(thys);
    }
    else if (ajStrMatchCaseC (token, "DNA"))
    {
	ajDebug("DNA\n");
	ajSeqSetNuc(thys); 
    }
    else
    {
	ajDebug("unknown - failed\n");
	ajFileBuffReset(buff);
	return ajFalse;
    }
    (void) ajStrToken (&token, &handle, " \t\""); /* : */
    if (!ajStrMatchC(token, ":"))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    (void) ajStrToken (&token, &handle, "\""); /* name */
    if (!ajStrLen(token))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    seqSetName (&thys->Name, token);

    /* OK, we have the name. Now look for the sequence */

    ok = ajFileBuffGet (buff, &rdline);
    while (ok && !ajStrPrefixC(rdline,"\n"))
    {
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
	ok = ajFileBuffGet (buff, &rdline);
    }

    ajFileBuffClear (buff, 0);

    (void) ajStrTokenReset (&handle);

    return ajTrue;
}


/* @funcstatic seqReadStrider *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using DNA strider format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadStrider (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjBool ok = ajTrue;

    do
    {
	ok = ajFileBuffGet (buff, &rdline);
	if (ajStrPrefixC(rdline, "; DNA sequence"))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ;\t,\n");
	    (void) ajStrToken (&token, &handle, NULL); /* 'DNA' */
	    (void) ajStrToken (&token, &handle, NULL); /* sequence */
	    (void) ajStrToken (&token, &handle, NULL); /* entry name */
	}
	bufflines++;
    } while (ok && ajStrPrefixC (rdline, ";"));

    if (!ok || !ajStrLen(token))
    {
	ajFileBuffReset(buff);
	return ajFalse;
    }

    seqSetName (&thys->Name, token);

    /* OK, we have the name. Now look for the sequence */

    while (ok && !ajStrPrefixC(rdline, "//"))
    {
	(void) seqAppend (&thys->Seq, rdline);
	bufflines++;
	ok = ajFileBuffGet (buff, &rdline);
    }

    ajFileBuffClear (buff, 0);

    (void) ajStrTokenReset (&handle);
    return ajTrue;
}

/* @funcstatic seqReadMsf *****************************************************
**
** Tries to read input in MSF format. If successful, can repeat for the
** next call to return the second, third, ... sequence from the same file.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadMsf (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    ajint len;
    AjBool ok = ajFalse;
    ajint iseq = 0;
    AjPFileBuff buff = seqin->Filebuff;
    AjPTable msftable = NULL;
    SeqPMsfItem msfitem = NULL;
    AjPList msflist = NULL;
    SeqPMsfData msfdata = NULL;

    ajint i;

    ajDebug("seqReadMsf seqin->Data %x\n", seqin->Data);

    if (!seqin->Data)
    {
	ok = ajFileBuffGet (buff, &rdline);
	if (!ok)
	  return ajFalse;
	bufflines++;

	if (ajStrPrefixC(rdline, "!!"))
	{
	    if (ajStrPrefixC(rdline, "!!AA_MULTIPLE_ALIGNMENT"))
		ajSeqSetProt(thys);

	    if (ajStrPrefixC(rdline, "!!NA_MULTIPLE_ALIGNMENT"))
		ajSeqSetNuc(thys);
	}

	if (!seqGcgMsfDots (thys, seqin, &rdline, seqMaxGcglines, &len))
	{
	    ajDebug("seqGcgMsfDots failed\n");
	    ajFileBuffReset(buff);
	    return ajFalse;
	}

	seqin->Data = AJNEW0(msfdata);
	msfdata->Table = msftable = ajTableNew (0, ajStrTableCmp, ajStrTableHash);
	msflist = ajListstrNew();
	seqin->Filecount = 0;
	ok = ajFileBuffGet (buff, &rdline);
	bufflines++;
	while (ok && !ajStrPrefixC (rdline, "//"))
	{
	    ok = ajFileBuffGet (buff, &rdline);
	    bufflines++;
	    if (bufflines > seqMaxGcglines)
		ok = ajFalse;
	    if (seqGcgMsfHeader(rdline, &msfitem))
	    {
		(void) ajTablePut(msftable, msfitem->Name, msfitem);
		ajListstrPushApp (msflist, msfitem->Name);
		iseq++;
	    }
	}

	ajDebug ("Header has %d sequences\n", iseq);
	ajListstrTrace (msflist);
	ajTableTrace (msftable);
	ajTableMap (msftable, seqMsfTabList, NULL);

	msfdata->Names = AJCALLOC (iseq, sizeof(*msfdata->Names));
	for (i=0; i < iseq; i++)
	{
	    (void) ajListstrPop (msflist, &msfdata->Names[i]);
	    ajDebug ("list [%d] '%S'\n", i, msfdata->Names[i]);
	}
	ajListstrFree(&msflist);
	while (ajFileBuffGet (buff, &rdline))
	{
	    bufflines++;
	    (void) seqGcgMsfReadseq(rdline, msftable);
	}

	ajTableMap (msftable, seqMsfTabList, NULL);
	msfdata->Nseq = iseq;
	msfdata->Count = 0;
	msfdata->Bufflines = bufflines;
	ajDebug("MSF format read %d lines\n", bufflines);
    }

    msfdata = seqin->Data;
    msftable = msfdata->Table;
    if (msfdata->Count >= msfdata->Nseq)
    {
	ajFileBuffClear(seqin->Filebuff, 0);
	ajTableMap(msftable, seqMsfTabDel, NULL);
	ajTableFree(&msftable);
	AJFREE(msfdata->Names);
	AJFREE(msfdata);
	seqin->Data = NULL;
	return ajFalse;
    }
    i = msfdata->Count;
    ajDebug ("returning [%d] '%S'\n", i, msfdata->Names[i]);
    msfitem = ajTableGet(msftable, msfdata->Names[i]);
    (void) ajStrAss(&thys->Name, msfdata->Names[i]);
    ajStrDel(&msfdata->Names[i]);

    thys->Weight = msfitem->Weight;
    (void) ajStrAss (&thys->Seq, msfitem->Seq);
    ajStrDel(&msfitem->Seq);

    msfdata->Count++;

    return ajTrue;
}

/* @funcstatic seqGcgMsfReadseq ***********************************************
**
** Reads sequence name from first token on the input line, and appends
** the sequence data to that sequence in the msftable structure.
**
** @param [P] rdline [AjPStr] Line from input file.
** @param [P] msftable [AjPTable] MSF format sequence table.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqGcgMsfReadseq (AjPStr rdline, AjPTable msftable)
{
    static AjPRegexp seqexp = NULL;
    SeqPMsfItem msfitem;
    static AjPStr token = NULL;
    static AjPStr seqstr = NULL;


    if (!seqexp)
	seqexp = ajRegCompC("[^ \t\n\r]+");

    if (!ajRegExec(seqexp, rdline))
	return ajFalse;

    ajRegSubI (seqexp, 0, &token);
    msfitem = ajTableGet(msftable, token);
    if (!msfitem)
    {
	ajStrDel(&token);
	return ajFalse;
    }

    (void) ajRegPost (seqexp, &seqstr);
    (void) seqAppend (&msfitem->Seq, seqstr);

    return ajTrue;
}

/* @funcstatic seqMsfTabList **************************************************
**
** Writes a debug report of the contents of an MSF table.
**
** @param [P] key [const void*] Standard argument, key from current table item
**                              which is a string for MSF internal tables.
** @param [P] value [void**] Standard argument, data from current table item, 
**                           converted to an MSF internal table item.
** @param [P] cl [void*] Standard argument, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabList (const void* key, void** value, void* cl)
{
    SeqPMsfItem msfitem = (SeqPMsfItem) *value;

    ajDebug ("key '%S' Name '%S' Seqlen %d\n",
	     key, msfitem->Name, ajStrLen(msfitem->Seq));
    return;
}

/* @funcstatic seqMsfTabDel **************************************************
**
** Deletes entries from the MSf internal table. Called for each entry in turn.
**
** @param [P] key [const void*] Standard argument, table key.
** @param [P] value [void**] Standard argument, table data item.
** @param [P] cl [void*] Standard argument, usually NULL
** @return [void]
** @@
******************************************************************************/

static void seqMsfTabDel (const void* key, void** value, void* cl)
{
    SeqPMsfItem msfitem = (SeqPMsfItem) *value;

    AJFREE(msfitem);

    return;
}

/* @funcstatic seqReadSwiss ***************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using SWISS format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadSwiss (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjBool ok;
    AjPFileBuff buff = seqin->Filebuff;
    AjPFileBuff ftfile = NULL;
    static AjPStr ftfmt = NULL;
    static AjPStr tmpstr = NULL;
    AjBool dofeat = ajFalse;
    AjPStr liststr;		/* for lists, do not delete */
   
    if (!ftfmt)
	ajStrAssC (&ftfmt, "swissprot");

    if (!ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr))
	return ajFalse;

    bufflines++;

    ajDebug ("seqReadSwiss first line '%S'\n", rdline);

    if (!ajStrPrefixC(rdline, "ID   "))
    {
	ajFileBuffReset (buff);
	return ajFalse;
    }
    (void) ajStrTokenAss (&handle, rdline, " \n\r");
    (void) ajStrToken (&token, &handle, NULL); /* 'ID' */
    (void) ajStrToken (&token, &handle, NULL); /* entry name */

    seqSetName (&thys->Name, token);

    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
    while (ok && !ajStrPrefixC(rdline, "SQ   "))
    {
	bufflines++;
	if (ajStrPrefixC(rdline, "AC   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ;\n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'AC' */
	    while (ajStrToken (&token, &handle, NULL))
	    {
		seqAccSave (thys, token);
	    }
	}
	if (ajStrPrefixC(rdline, "DE   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ");
	    (void) ajStrToken (&token, &handle, NULL); /* 'DE' */
	    (void) ajStrToken (&token, &handle, "\n\r"); /* desc */
	    if (ajStrLen(thys->Desc))
	    {
		(void) ajStrAppC (&thys->Desc, " ");
		(void) ajStrApp (&thys->Desc, token);
	    }
	    else
	    {
		(void) ajStrAss (&thys->Desc, token);
	    }
	}
	if (ajStrPrefixC(rdline, "KW   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'KW' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      liststr = ajStrNewS(token);
	      ajStrChomp(&liststr);
	      ajListstrPushApp (thys->Keylist, liststr);
	    }
	}
	if (ajStrPrefixC(rdline, "OS   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'OS' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      ajStrAssS(&tmpstr, token);
	      ajStrChomp(&tmpstr);
	      seqTaxSave (thys, tmpstr);
	      ajStrDel(&tmpstr);
	    }
	}
	if (ajStrPrefixC(rdline, "OC   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'OC' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      ajStrAssS(&tmpstr, token);
	      ajStrChomp(&tmpstr);
	      seqTaxSave (thys, tmpstr);
	      ajStrDel(&tmpstr);
	    }
	}
	if (ajStrPrefixC(rdline, "FT   "))
	{
	    if (seqinUfoLocal(seqin))
	    {
		if (!dofeat)
		{
		    dofeat = ajTrue;
		    ftfile = ajFileBuffNew();
		}
		ajFileBuffLoadS (ftfile, rdline);
		/* ajDebug ("EMBL FEAT saved line:\n%S", rdline); */
	    }
	}
	ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
    }

    if (dofeat)
    {
        ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF (ftfmt, thys->Name, "P", ftfile);
	ajDebug ("SWISS FEAT TabIn %x\n", seqin->Ftquery);
	ftfile = NULL;			/* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeatRead (seqin->Ftquery);
	ajFeattableTrace(seqin->Fttable);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }

    if (ajStrLen(seqin->Inseq))
    {	/* we have a sequence to use */
	ajStrAssS (&thys->Seq, seqin->Inseq);
	if (seqin->Text)
	{
	  seqTextSeq(&thys->TextPtr, seqin->Inseq);
	  ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {	/* read the sequence and terminator */
	ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
	while (ok && !ajStrPrefixC(rdline, "//"))
	{
	    (void) seqAppend (&thys->Seq, rdline);
	    bufflines++;
	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
	}
    }

    ajFileBuffClear (buff, 0);

    (void) ajStrDelReuse (&token);

    (void) ajStrTokenReset (&handle);

    return ajTrue;
}

/* @funcstatic seqReadEmbl ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using EMBL format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadEmbl (AjPSeq thys, AjPSeqin seqin)
{
    
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjBool ok;
    AjPFileBuff buff = seqin->Filebuff;
    static AjPStr ftfmt = NULL;
    static AjPStr tmpstr = NULL;
    AjBool dofeat = ajFalse;
    AjPStr liststr;		/* for lists, do not delete */

    if (!ftfmt)
	ajStrAssC (&ftfmt, "embl");
    
    if (!ajFileBuffGet (buff, &rdline))
	return ajFalse;
    
    bufflines++;
    
    /* for GCG formatted databases */

    while (ajStrPrefixC(rdline, "WP ")) {
      if (!ajFileBuffGet (buff, &rdline))
	return ajFalse;
      bufflines++;
    }

    ajDebug ("seqReadEmbl first line '%S'\n", rdline);
    
    if (!ajStrPrefixC(rdline, "ID   "))
    {
	ajFileBuffReset (buff);
	return ajFalse;
    }
    if(seqin->Text)
	ajStrAssC(&thys->TextPtr,ajStrStr(rdline));

    ajDebug("seqReadEmbl ID line found\n");
    (void) ajStrTokenAss (&handle, rdline, " \n\r");
    (void) ajStrToken (&token, &handle, NULL); /* 'ID' */
    (void) ajStrToken (&token, &handle, NULL); /* entry name */
    
    seqSetName (&thys->Name, token);
    
    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
    while (ok && !ajStrPrefixC(rdline, "SQ   "))
    {
	bufflines++;
	if (ajStrPrefixC(rdline, "AC   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ;\n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'AC' */
	    while (ajStrToken (&token, &handle, NULL))
		seqAccSave (thys, token);
	}
	if (ajStrPrefixC(rdline, "SV   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'AC' */
	    (void) ajStrToken (&token, &handle, NULL); /* version */
	    seqSvSave (thys, token);
	}
	if (ajStrPrefixC(rdline, "DE   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " ");
	    (void) ajStrToken (&token, &handle, NULL); /* 'DE' */
	    (void) ajStrToken (&token, &handle, "\n\r"); /* desc */
	    if (ajStrLen(thys->Desc))
	    {
		(void) ajStrAppC (&thys->Desc, " ");
		(void) ajStrApp (&thys->Desc, token);
	    }
	    else
		(void) ajStrAss (&thys->Desc, token);
	}
	if (ajStrPrefixC(rdline, "KW   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'KW' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      liststr = ajStrNewS(token);
	      ajStrChomp(&liststr);
	      ajListstrPushApp (thys->Keylist, liststr);
	    }
	}
	if (ajStrPrefixC(rdline, "OS   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'OS' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      ajStrAssS(&tmpstr, token);
	      ajStrChomp(&tmpstr);
	      seqTaxSave (thys, tmpstr);
	      ajStrDel(&tmpstr);
	    }
	}
	if (ajStrPrefixC(rdline, "OC   "))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'OC' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      ajStrAssS(&tmpstr, token);
	      ajStrChomp(&tmpstr);
	      seqTaxSave (thys, tmpstr);
	      ajStrDel(&tmpstr);
	    }
	}
	if (ajStrPrefixC(rdline, "FT   "))
	{
	    if (seqinUfoLocal(seqin))
	    {
		if (!dofeat)
		{
		  dofeat = ajTrue;
		  ajFeattabInDel(&seqin->Ftquery);
		  seqin->Ftquery = ajFeattabInNewSS (ftfmt, thys->Name, "N");
		  ajDebug("seqin->Ftquery ftfile %x\n", seqin->Ftquery->Handle);
		}
		ajFileBuffLoadS (seqin->Ftquery->Handle, rdline);
		/* ajDebug ("EMBL FEAT saved line:\n%S", rdline); */
	    }
	}
	ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
    }
    
    

    if (dofeat)
    {
	ajDebug ("EMBL FEAT TabIn %x\n", seqin->Ftquery);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = ajFeatRead (seqin->Ftquery);
	ajFeattableTrace(thys->Fttable);
    }
    
    if (ajStrLen(seqin->Inseq))
    {	/* we have a sequence to use */
	ajStrAssS (&thys->Seq, seqin->Inseq);
	if (seqin->Text)
	{
	  seqTextSeq(&thys->TextPtr, seqin->Inseq);
	  ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {	/* read the sequence and terminator */
	ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
	while (ok && !ajStrPrefixC(rdline, "//"))
	{
	    (void) seqAppend (&thys->Seq, rdline);
	    bufflines++;
	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
	}
    }
    
    ajFileBuffClear (buff, 0);
    
    (void) ajStrDelReuse (&token);
				/* 3033
 */
    (void) ajStrTokenReset (&handle);


    ajSeqTrace(thys);

    return ajTrue;
}

/* @funcstatic seqReadGenbank *************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using Genbank format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGenbank (AjPSeq thys, AjPSeqin seqin)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr rdline = NULL;
    ajint bufflines = 0;
    AjBool ok;
    AjBool done = ajFalse;
    AjPFileBuff buff = seqin->Filebuff;
    AjPFileBuff ftfile = NULL;
    static AjPStr ftfmt = NULL;
    static AjPStr tmpstr = NULL;
    AjBool dofeat = ajFalse;
    AjPSeqQuery qry = seqin->Query;
    AjPStr liststr;		/* for lists, do not delete */

    ajDebug("seqReadGenbank\n");

    if (!ftfmt)
	ajStrAssC (&ftfmt, "genbank");

    if (!ajFileBuffGet (buff, &rdline))
      return ajFalse;
    bufflines++;

    ok = ajTrue;

    /* for GCG formatted databases */

    if (ajStrPrefixC(rdline, "WPCOMMENT"))
    {
      ajFileBuffGet (buff, &rdline);
      bufflines++;
      ok = ajTrue;
      while (ok && ajStrPrefixC(rdline, " "))
      {
	ok = ajFileBuffGet (buff, &rdline);
 	bufflines++;
      }
    }
    if (!ok)
    {
      ajFileBuffReset (buff);
      return ajFalse;
    }

    /* This loop necessary owing to headers on GB distro files */
    if(ajStrFindC(rdline,"Genetic Sequence Data Bank") >= 0)
	while (!ajStrPrefixC(rdline, "LOCUS"))
	{
	    if (!ajFileBuffGet (buff, &rdline))
		return ajFalse;
	    bufflines++;
	}

    if (!ajStrPrefixC(rdline, "LOCUS"))
    {
	ajDebug("failed - LOCUS not found - first line was\n%S\n", rdline);
	ajFileBuffReset (buff);
	return ajFalse;
    }
    if(seqin->Text)
	ajStrAssC(&thys->TextPtr,ajStrStr(rdline));
    
    (void) ajStrTokenAss (&handle, rdline, " \n\r");
    (void) ajStrToken (&token, &handle, NULL); /* 'ID' */
    (void) ajStrToken (&token, &handle, NULL); /* entry name */

    seqSetName (&thys->Name, token);

    ok = ajFileBuffGetStore(buff, &rdline, seqin->Text, &thys->TextPtr);
    while (ok &&
	   !ajStrPrefixC(rdline, "ORIGIN") &&
	   !ajStrPrefixC(rdline, "BASE COUNT"))
    {
	done = ajFalse;
	bufflines++;
	if (ajStrPrefixC(rdline, "ACCESSION"))
	{
	    ajDebug("accession found\n");

	    (void) ajStrTokenAss (&handle, rdline, " ;\n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'ACCESSION' */
	    while (ajStrToken (&token, &handle, NULL))
	    {
		seqAccSave (thys, token);
	    }
	}
	if (ajStrPrefixC(rdline, "VERSION"))
	{
	    ajDebug("seqversion found\n");

	    (void) ajStrTokenAss (&handle, rdline, " \n\r");
	    (void) ajStrToken (&token, &handle, NULL); /* 'VERSION' */
	    (void) ajStrToken (&token, &handle, NULL);
	    seqSvSave (thys, token);
	    if (ajStrToken (&token, &handle, ": \n\r"))	/* GI: */
	    {
	      (void) ajStrToken (&token, &handle, NULL);
	      ajStrAssS(&thys->Gi, token);
	    }
	}
	if (ajStrPrefixC(rdline, "FEATURES"))
	{
	    if (seqinUfoLocal(seqin))
	    {
		ajDebug("features found\n");
		if (!dofeat)
		{
		    dofeat = ajTrue;
		    ftfile = ajFileBuffNew();
		    /* ajDebug ("GENBANK FEAT first line:\n%S", rdline); */
		}
		ajFileBuffLoadS (ftfile, rdline);
		ok = ajFileBuffGet (buff, &rdline);
		done = ajTrue;
		while (ok && ajStrPrefixC(rdline, " "))
		{
		    bufflines++;
		    ajFileBuffLoadS (ftfile, rdline);
		    /* ajDebug ("GENBANK FEAT saved line:\n%S", rdline); */
		    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
					     &thys->TextPtr);
		}
	    }
	}

	if (ajStrPrefixC(rdline, "DEFINITION"))
	{
	    ajDebug("definition found\n");
	    (void) ajStrTokenAss (&handle, rdline, " ");
	    (void) ajStrToken (&token, &handle, NULL); /* 'DEFINITION' */
	    (void) ajStrToken (&token, &handle, "\n\r"); /* desc */
	    (void) ajStrAss (&thys->Desc, token);
	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
	    done = ajTrue;
	    while (ok && ajStrPrefixC(rdline, " "))
	    {
		bufflines++;
		(void) ajStrTokenAss (&handle, rdline, " ");
		(void) ajStrToken (&token, &handle, "\n\r"); 
		(void) ajStrAppC (&thys->Desc, " ");
		(void) ajStrApp (&thys->Desc, token);
		ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
					 &thys->TextPtr);
	    }
	}

	if (ajStrPrefixC(rdline, "KEYWORDS"))
	{
	    ajDebug("keywords found\n");
	    (void) ajStrTokenAss (&handle, rdline, " ");
	    (void) ajStrToken (&token, &handle, NULL); /* 'KEYWORDS' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      liststr = ajStrNewS(token);
	      ajStrChomp(&liststr);
	      ajListstrPushApp (thys->Keylist, liststr);
	    }

	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
	    done = ajTrue;
	    while (ok && ajStrPrefixC(rdline, " "))
	    {
		bufflines++;
		(void) ajStrTokenAss (&handle, rdline, " ");
		while (ajStrToken (&token, &handle, ".;\n\r"))
		{
		    liststr = ajStrNewS(token);
		    ajStrChomp(&liststr);
		    ajListstrPushApp (thys->Keylist, liststr);
		}
		ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
					 &thys->TextPtr);
	    }
	}

	if (ajStrPrefixC(rdline, "  ORGANISM"))
	{
	    ajDebug("organism found\n");
	    (void) ajStrTokenAss (&handle, rdline, " ");
	    (void) ajStrToken (&token, &handle, NULL); /* 'ORGANISM' */
	    while (ajStrToken (&token, &handle, ".;\n\r"))
	    {
	      ajStrAssS(&tmpstr, token);
	      ajStrChomp(&tmpstr);
	      seqTaxSave(thys, tmpstr);
	    }

	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
	    done = ajTrue;
	    while (ok && ajStrPrefixC(rdline, "    "))
	    {
		bufflines++;
		(void) ajStrTokenAss (&handle, rdline, " ");
		while (ajStrToken (&token, &handle, ".;\n\r"))
		{
		  ajStrAssS(&tmpstr, token);
		  ajStrChomp(&tmpstr);
		  seqTaxSave(thys, tmpstr);
		}
		ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
					 &thys->TextPtr);
	    }
	}

	if (!done)
	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
    }

    if (dofeat)
    {
        ajFeattabInDel(&seqin->Ftquery);
	seqin->Ftquery = ajFeattabInNewSSF (ftfmt, thys->Name, "N", ftfile);
	ajDebug ("GENBANK FEAT TabIn %x\n", seqin->Ftquery);
	ftfile = NULL;			/* now copied to seqin->FeattabIn */
	ajFeattableDel(&seqin->Fttable);
	seqin->Fttable = ajFeatRead (seqin->Ftquery);
	ajFeattableTrace(seqin->Fttable);
	ajFeattableDel(&thys->Fttable);
	thys->Fttable = seqin->Fttable;
	seqin->Fttable = NULL;
    }

    if (ajStrLen(seqin->Inseq))
    {					/* we have a sequence to use */
	ajDebug("Got an Inseq sequence\n");
	if(ajStrMatchC(qry->Method,"gcg"))
	{
	    while(!ajStrPrefixC(rdline,"ORIGIN"))
	      ajFileBuffGetStore(buff,&rdline, seqin->Text, &thys->TextPtr);
	}
	ajStrAssS (&thys->Seq, seqin->Inseq);
	if (seqin->Text)
	{
	  seqTextSeq(&thys->TextPtr, seqin->Inseq);
	  ajFmtPrintAppS(&thys->TextPtr, "//\n");
	}
    }
    else
    {					/* read the sequence and terminator */
	ajDebug("sequence start at '%S'\n", rdline);
	while(!ajStrPrefixC(rdline,"ORIGIN") &&
	      !ajStrPrefixC(rdline,"BASE COUNT"))
	    if(!ajFileBuffGetStore(buff,&rdline, seqin->Text, &thys->TextPtr))
		break;
	ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
	while (ok && !ajStrPrefixC(rdline, "//"))
	{
	    if (!ajStrPrefixC(rdline, "ORIGIN") &&
		!ajStrPrefixC(rdline,"BASE COUNT"))
		(void) seqAppend (&thys->Seq, rdline);
	    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text,
				     &thys->TextPtr);
	    bufflines++;
	}
    }

    if(!ajStrMatchC(qry->Method,"gcg"))
    {
	while(!ajStrPrefixC(rdline,"//"))
	    ajFileBuffGetStore(buff,&rdline, seqin->Text, &thys->TextPtr);
    }

    ajFileBuffClear (buff, 0);

    (void) ajStrTokenReset (&handle);

    return ajTrue;
}

/* @funcstatic seqReadGff *****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using GFF format.
**
** GFF only offers the sequence, and the type, with the DNA, RNA and
** Protein and End-xxx headers. GFF allows other header lines to be defined,
** so EMBOSS can add more lines for accession number and description
**
** GFF also defines Type and sequence-region headers, but they only
** provide information that is also in the DNA, RNA or Protein header
** and these are required for sequence storage so we ignore the alternatives.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadGff (AjPSeq thys, AjPSeqin seqin)
{
    
  static AjPRegexp typexp = NULL;
  static AjPStr rdline = NULL;
  ajint bufflines = 0;
  AjBool ok;
  AjBool isseq = ajFalse;
  AjPFileBuff buff = seqin->Filebuff;
  AjPFileBuff ftfile = NULL;
  static AjPStr ftfmt = NULL;
  AjBool dofeat = ajFalse;
  static AjPStr typstr;
  static AjPStr verstr = NULL;	/* copy of version line */
  static AjPStr outstr = NULL;	/* generated Type line */

  if (!typexp)
    typexp = ajRegCompC("^##([DR]NA|Protein) +([^ \t\r\n]+)");

  if (!ftfmt)
    ajStrAssC (&ftfmt, "gff");
    
  ok = ajFileBuffGet (buff, &rdline);
  if (!ok)
    return ajFalse;

  bufflines++;

  ajDebug ("seqReadGff first line '%S'\n", rdline);

  if (!ajStrPrefixC(rdline, "##gff-version ")) {
    ajFileBuffReset (buff);
    return ajFalse;
  }
  ajStrAssS (&verstr, rdline);

  if(seqin->Text)
    ajStrAssS(&thys->TextPtr,rdline);

  ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
  while (ok && ajStrPrefixC (rdline, "##")) {
    if (ajRegExec(typexp, rdline)) {
      isseq = ajTrue;
      ajRegSubI(typexp, 1, &typstr);
      ajRegSubI(typexp, 2, &thys->Name);
      if (ajStrMatchC(typstr, "Protein"))
	ajSeqSetProt(thys);
      else
	ajSeqSetNuc(thys);
      ajFmtPrintS (&outstr, "##Type %S %S", typstr, thys->Name);
    }
    else if (ajStrPrefixC(rdline, "##end-")) {
      isseq = ajFalse;
    }
    else if (isseq) {
      (void) seqAppend (&thys->Seq, rdline);
    }
    ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
  }

  if (!ajSeqLen(thys)) {
    ajFileBuffReset (buff);
    return ajFalse;
  }

  /* do we want the features now? */

  if (ok & seqinUfoLocal(seqin)) {
    dofeat = ajTrue;
    ftfile = ajFileBuffNew();
    ajFileBuffLoadS (ftfile, verstr);
    ajFileBuffLoadS (ftfile, outstr);
    while (ok && !ajStrPrefixC(rdline, "##")) {
      ajFileBuffLoadS (ftfile, rdline);
      /* ajDebug ("GFF FEAT saved line:\n%S", rdline); */
      ok = ajFileBuffGetStore (buff, &rdline, seqin->Text, &thys->TextPtr);
    }
  }

  if (dofeat) {
    ajFeattabInDel(&seqin->Ftquery);
    seqin->Ftquery = ajFeattabInNewSSF (ftfmt, thys->Name,
					ajStrStr(seqin->Type), ftfile);
    ajDebug ("GFF FEAT TabIn %x\n", seqin->Ftquery);
    ftfile = NULL;			/* now copied to seqin->FeattabIn */
    ajFeattableDel(&seqin->Fttable);
    seqin->Fttable = ajFeatRead (seqin->Ftquery);
    ajFeattableTrace(seqin->Fttable);
    ajFeattableDel(&thys->Fttable);
    thys->Fttable = seqin->Fttable;
    seqin->Fttable = NULL;
  }

  ajFileBuffClear (buff, 0);

  return ajTrue;
}
   

/* @funcstatic seqReadAbi ****************************************************
**
** Given data in a sequence structure, tries to read everything needed
** using ABI format.
**
** @param [wP] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqReadAbi (AjPSeq thys, AjPSeqin seqin)
{
    AjPFileBuff buff = seqin->Filebuff;
    AjBool  ok=ajFalse;
    ajlong baseO=0L;
    ajlong numBases=0L;
    AjPStr sample=NULL;
    AjPStr smpl=NULL;
    static AjPRegexp dotsexp = NULL;
    AjPFile fp = ajFileBuffFile (buff);
    ajint filestat;

    ajDebug("seqReadAbi file %F\n", fp);
    /* ajFileBuffTraceFull(buff, 10, 10); */

    if (ajFileBuffEnd(buff))
      return ajFalse;

    if(!ajSeqABITest(fp))
    {
        ajDebug("seqReadAbi ajSeqABITest failed on %F\n", fp);
	ajFileBuffResetPos(buff);
	return ajFalse;
    }

    filestat = ajFileSeek(fp,0L,0);
    ajDebug ("filestat %d\n", filestat);
    
    numBases = ajSeqABIGetNBase(fp);
    /* Find BASE tag & get offset                    */
    baseO = ajSeqABIGetBaseOffset(fp);
    /* Read in sequence         */
    ok = ajSeqABIReadSeq(fp,baseO,numBases,&thys->Seq);
    if (!ok) {
      ajFileBuffResetPos(buff);
      return ajFalse;
    }

    sample = ajStrNew();
    ajSeqABISampleName(fp, &sample);

    /* replace dots in the sample name with underscore */
    dotsexp = ajRegCompC ("^(.*)[.](.*)$");
    smpl = ajStrNew();

    while(ajRegExec(dotsexp,sample))
    {
      ajStrClear(&sample);
      ajRegSubI(dotsexp,1,&smpl);
      ajStrAppC(&smpl,"_");
      ajStrApp(&sample,smpl);
      ajRegSubI(dotsexp,2,&smpl);
      ajStrApp(&sample,smpl);
    }

    ajStrAssC(&thys->Name,ajStrStr(sample));
    
    ajSeqSetNuc (thys);

    ajFileBuffClear(buff, -1);
    buff->File->End=ajTrue;

    ajStrDel(&smpl);
    ajStrDel(&sample);

    return ajTrue;
}

/* @func ajSeqPrintInFormat ************************************************
**
** Reports the internal data structures
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintInFormat (AjPFile outf, AjBool full)
{
    ajint i=0;

    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "# sequence input formats\n");
    ajFmtPrintF (outf, "# Name         Try (test for unknown input)"
		 " files)\n");
    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "InFormat {\n");
    for (i=0; seqInFormatDef[i].Name; i++)
    {
	if (full || seqInFormatDef[i].Try)
	    ajFmtPrintF (outf, "  %-12s %B\n", seqInFormatDef[i].Name,
			 seqInFormatDef[i].Try);
    }
    ajFmtPrintF (outf, "}\n\n");

    return;
}

/* @funcstatic seqFindInFormat ************************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Sets iformat as the recognized format, and returns ajTrue.
**
** @param [P] format [AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/


static AjBool seqFindInFormat (AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajint i = 0;

    /* ajDebug ("seqFindInFormat '%S'\n", format); */
    if (!ajStrLen(format))
	return ajFalse;

    (void) ajStrAss (&tmpformat, format);
    (void) ajStrToLower(&tmpformat);

    for (i=0; seqInFormatDef[i].Name; i++)
    {
	/* ajDebug ("test %d '%s' \n", i, seqInFormatDef[i].Name); */
	if (ajStrMatchCaseC(tmpformat, seqInFormatDef[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    /* ajDebug ("found '%s' at %d\n", seqInFormatDef[i].Name, i); */
	    return ajTrue;
	}
    }

    ajErr ("Unknown input format '%S'", format);

    ajStrDel(&tmpformat);
    return ajFalse;
}

/* @funcstatic seqSetInFormat ********************************************
**
** Steps through a list of default formats, setting the Try value for
** each known format to ajTrue if it is in the list, and ajFalse
** if not.
**
** @param [P] format [AjPStr] Format list, punctuated by whitespace or commas
** @return [AjBool] ajTrue if all formats were accepted
** @@
******************************************************************************/

static AjBool seqSetInFormat (AjPStr format)
{
    ajint i;
    AjPStr fmtstr = NULL;
    static AjPStrTok handle = NULL;
    ajint ifound;
    AjBool ret = ajTrue;

    for (i=0; seqInFormatDef[i].Name; i++)
	seqInFormatDef[i].Try = ajFalse;

    ajDebug("seqSetInformat '%S'\n", format);

    (void) ajStrTokenAss (&handle, format, " \t\n\r,;:");
    while (ajStrToken (&fmtstr, &handle, " \t\n\r,;:"))
    {
	ifound = 0;
	for (i=0; seqInFormatDef[i].Name; i++)
	{
	    if (ajStrMatchCaseC(fmtstr, seqInFormatDef[i].Name))
	    {
		/* ajDebug("found '%S' %d\n", fmtstr, i); */
		seqInFormatDef[i].Try = ajTrue;
		ifound = 1;
		break;
	    }
	}
	if (!ifound)
	{
	    /* ajDebug("not found '%S'\n", fmtstr); */

	    ajErr("Input format '%S' not known", fmtstr);
	    ret = ajFalse;
	}
    }

    (void) ajStrTokenReset (&handle);

    return ret;
}
 

/* @funcstatic seqAppend ******************************************************
**
** Appends sequence characters in the input line to a growing sequence.
** Non sequence characters are simply ignored.
**
** @param [uP] pseq [AjPStr*] Sequence as a string
** @param [r] line [AjPStr] Input line.
** @return [ajint] Sequence length to date.
** @@
******************************************************************************/

static ajint seqAppend (AjPStr* pseq, AjPStr line)
{
    static AjPStr token = NULL;
    const char* cp;
    static AjPRegexp seqexp = NULL;
    ajint i=0;

    if (!seqexp)
	seqexp = ajRegCompC ("[A-Za-z*.~-]+");

    cp = ajStrStr(line);
    while (cp && ajRegExecC(seqexp, cp))
    {
	ajRegSubI (seqexp, 0, &token);
	(void) ajStrApp (pseq, token);
	i += ajStrLen(token);
	(void) ajRegPostC (seqexp, &cp);
    }
 
    return i;
}

/* @funcstatic seqGcgDots *****************************************************
**
** Looks for the ".." line in the header of a GCG format sequence.
** Care is needed to make sure this is not an MSF header which
** has a very similar format.
**
** Data found on the header line is extracted and returned.
**
** The number of lines searched is limited to avoid parsing large data
** files that are not in GCG format. The user should set this limit to
** be large enough to handle large EMBL/Genbank annotations
**
** @param [P] thys [AjPSeq] Sequence.
** @param [P] seqin [AjPSeqin] Sequence input.
** @param [P] pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajint] Maximum number of lines to read before giving up.
** @param [w] len [ajint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgDots (AjPSeq thys, AjPSeqin seqin, AjPStr* pline,
			  ajint maxlines, ajint* len)
{
    static AjPStr token = NULL;
    ajint check=0;
    ajint nlines=0;

    AjPFileBuff buff = seqin->Filebuff;

    static AjPRegexp dotexp = NULL;
    static AjPRegexp chkexp = NULL;
    static AjPRegexp lenexp = NULL;
    static AjPRegexp typexp = NULL;
    static AjPRegexp namexp = NULL;
    static AjPRegexp msfexp = NULL;

    if (!dotexp)
    {
	dotexp = ajRegCompC("[.][.]");
	chkexp = ajRegCompC("[Cc][Hh][Ee][Cc][Kk]:[ \t]*([0-9]+)");
	lenexp = ajRegCompC("[Ll][Ee][Nn][Gg][Tt][Hh]:[ \t]*([0-9]+)");
	typexp = ajRegCompC("[Tt][Yy][Pp][Ee]:[ \t]*([NP])");
	namexp = ajRegCompC("[^ \t>]+");
	msfexp = ajRegCompC("[Mm][Ss][Ff]:[ \t]*([0-9]+)");
    }

    while (nlines < maxlines)
    {
	if (nlines++)
	    if (!ajFileBuffGet(buff, pline))
		return ajFalse; 

	if (nlines > maxlines)
	    return ajFalse;

	if (!ajRegExec(dotexp, *pline))
	    continue;

	ajDebug ("seqGcgDots   .. found\n'%S'\n", *pline);
	if (!ajRegExec(chkexp, *pline)) /* checksum required */
	    return ajFalse;

	if (ajRegExec(msfexp, *pline))	/* oops - it's an MSF file */
	    return ajFalse;

	ajRegSubI (chkexp, 1, &token);
	(void) ajStrToInt (token, &check);

	ajDebug ("   checksum %d\n", check);

	if (ajRegExec (lenexp, *pline))
	{
	    ajRegSubI (lenexp, 1, &token);
	    (void) ajStrToInt (token, len);
	    ajDebug ("   length %d\n", *len);
	}
    
	if (ajRegExec (namexp, *pline))
	{
	    ajRegSubI (namexp, 0, &thys->Name);
	    ajDebug ("   name '%S'\n", thys->Name);
	}

	if (ajRegExec (typexp, *pline))
	{
	    ajRegSubI (typexp, 1, &thys->Type);
	    ajDebug ("   type '%S'\n", thys->Type);
	}
	return ajTrue;
    }

    return ajFalse;
}

/* @funcstatic seqGcgMsfDots **************************************************
**
** Looks for the ".." line in the header of an MSF format sequence.
** Care is needed to make sure this is not a simple GCG header which
** has a very similar format.
**
** Data found on the header line is extracted and returned.
**
** The number of lines searched is limited to avoid parsing large data
** files that are not in GCG format. The user should set this limit to
** be large enough to handle large EMBL/Genbank annotations
**
** @param [P] thys [AjPSeq] Sequence.
** @param [P] seqin [AjPSeqin] Sequence input.
** @param [P] pline [AjPStr*] Input buffer.
** @param [r] maxlines [ajint] Maximum number of lines to read before giving up.
** @param [w] len [ajint*] Length of sequence read.
** @return [AjBool] ajTrue on success. ajFalse on failure or aborting.
** @@
******************************************************************************/

static AjBool seqGcgMsfDots (AjPSeq thys, AjPSeqin seqin, AjPStr* pline,
			     ajint maxlines, ajint* len)
{
    static AjPStr token = NULL;
    ajint check=0;
    ajint nlines=0;

    AjPFileBuff buff = seqin->Filebuff;

    static AjPRegexp dotexp = NULL;
    static AjPRegexp chkexp = NULL;
    static AjPRegexp lenexp = NULL;
    static AjPRegexp typexp = NULL;
    static AjPRegexp namexp = NULL;

    if (!dotexp)
    {
	dotexp = ajRegCompC("[.][.]");
	chkexp = ajRegCompC("[Cc][Hh][Ee][Cc][Kk]:[ \t]+([0-9]+)");
	lenexp = ajRegCompC("[Mm][Ss][Ff]:[ \t]*([0-9]+)");
	typexp = ajRegCompC("[Tt][Yy][Pp][Ee]:[ \t]+([NP])");
	namexp = ajRegCompC("[^ \t]+");
    }

    ajDebug ("seqGcgMsfDots maxlines: %d\nline: '%S'\n", maxlines,*pline);

    while (nlines < maxlines)
    {
	if (nlines++)
	    if (!ajFileBuffGet(buff, pline))
		return ajFalse; 

	ajDebug ("testing line %d\n'%S'\n", nlines,*pline);
	if (nlines > maxlines)
	    return ajFalse;

	if (!ajRegExec(dotexp, *pline))
	    continue;

	/* dots found. This must be the line if this is MSF format */

	if (!ajRegExec(chkexp, *pline)) /* check: is required */
	    return ajFalse;

	if (!ajRegExec (lenexp, *pline)) /* MSF: len is required for GCG*/
	    return ajFalse;


	ajRegSubI (lenexp, 1, &token);
	(void) ajStrToInt (token, len);

	ajRegSubI (chkexp, 1, &token);
	(void) ajStrToInt (token, &check);

	if (ajRegExec (namexp, *pline))
	    ajRegSubI (namexp, 0, &thys->Name);

	if (ajRegExec (typexp, *pline))
	    ajRegSubI (typexp, 1, &thys->Type);

	return ajTrue;
    }

    return ajFalse;
}

/* @funcstatic seqGcgMsfHeader ************************************************
**
** Parses data from a line of an MSF file header. The header stores
** names and other data for all sequences in the file. Each file
** is definied on a separate line. The results are stored
** in the MSF internal table. The sequence data is read later in the
** input file and added to the table.
**
** @param [P] line [AjPStr] Input line.
** @param [P] pmsfitem [SeqPMsfItem*] MSF internal table item.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqGcgMsfHeader (AjPStr line, SeqPMsfItem* pmsfitem)
{
    AjPStr name = NULL;			/* NOTE: not static. New each time for list */
    static AjPStr token = NULL;
    SeqPMsfItem msfitem = NULL;

    static AjPRegexp chkexp = NULL;
    static AjPRegexp lenexp = NULL;
    static AjPRegexp wgtexp = NULL;
    static AjPRegexp namexp = NULL;

    if (!chkexp)
    {
	chkexp = ajRegCompC("[Cc][Hh][Ee][Cc][Kk]:[ \t]*([0-9]+)");
	lenexp = ajRegCompC("[Ll][Ee][Nn][Gg]?[Tt]?[Hh]?:[ \t]*([0-9]+)");
	wgtexp = ajRegCompC("[Ww][Ee][Ii][Gg][Hh][Tt]:[ \t]*([0-9.]+)");
	namexp = ajRegCompC("[Nn][Aa][Mm][Ee]:[ \t]*([^ \t]+)");
    }

    ajDebug ("seqGcgMsfHeader '%S'\n", line);

    if (!ajRegExec (namexp, line))
	return ajFalse;

    ajRegSubI (namexp, 1, &name);
    /*ajDebug ("Name found\n");*/

    if (!ajRegExec(chkexp, line))
	return ajFalse;

    /*ajDebug ("Check found\n");*/

    *pmsfitem = AJNEW0(msfitem);
    msfitem->Name = name;

    ajRegSubI (chkexp, 1, &token);
    (void) ajStrToInt (token, &msfitem->Check);

    if (ajRegExec (lenexp, line))
    {
	ajRegSubI (lenexp, 1, &token);
	(void) ajStrToInt (token, &msfitem->Len);
    }
    else
	msfitem->Len = 0;

    msfitem->Seq = ajStrNewL(msfitem->Len+1);

    if (ajRegExec (wgtexp, line))
    {
	ajRegSubI (wgtexp, 1, &token);
	(void) ajStrToFloat (token, &msfitem->Weight);
    }
    else
	msfitem->Weight = 1.0;

    ajDebug ("MSF header name '%S' check %d len %d weight %.3f\n",
	     msfitem->Name, msfitem->Check, msfitem->Len, msfitem->Weight);

    return ajTrue;
}

/* @funcstatic seqUsaProcess **************************************************
**
** Converts a USA Universal Sequence Address into an open file.
**
** First tests for "[n:n:r]" range and sets this if it is found
**
** Then tests for asis:: in which the "filename" is really the sequence
** and no format is needed.
**
** Then tests for "format::" and sets this if it is found
**
** Then tests for "list:" or "@" and processes as a list file
** using seqListProcess which in turn invokes seqUsaProcess
** until a valid USA is found.
**
** Then tests for dbname:query and opens the file (at the correct position
** if the database definition defines it)
**
** If there is no database, looks for file:query and opens the file.
** In this case the file position is not known and sequence reading
** will have to scan for the entry/entries we need.
**
** @param [u] thys [AjPSeq] Sequence to be read.
** @param [u] seqin [AjPSeqin] Sequence input structure.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqUsaProcess (AjPSeq thys, AjPSeqin seqin)
{
    static AjPRegexp fmtexp = NULL;
    static AjPRegexp dbexp = NULL;
    static AjPRegexp idexp = NULL;
    static AjPRegexp wildexp = NULL;
    static AjPRegexp listexp = NULL;
    static AjPRegexp asisexp = NULL;
    static AjPRegexp rangeexp = NULL;

    static AjPStr usatest = NULL;

    static AjPStr qrydb = NULL;
    static AjPStr qrylist = NULL;
    static AjPStr qrychr = NULL;
    static AjPStr tmpstr = NULL;

    AjPSeqQuery qry = seqin->Query;

    AjBool fmtstat = ajFalse;		/* status returns from regex tests */
    AjBool regstat = ajFalse;
    AjBool dbstat = ajFalse;
    AjBool methstat = ajFalse;
    AjBool accstat = ajFalse;	/* return status from reading something */
    AjBool liststat = ajFalse;
    AjBool asisstat = ajFalse;
    AjBool rangestat = ajFalse;

    /*
    ajint ibegin=0;
    ajint iend=0;
    AjBool irev=ajFalse;
    */

    ajStrDel(&qry->Field);	/* clear it. we test this for regstat */

    ajDebug("++seqUsaProcess '%S' %d..%d(%b) '%S' %d \n",
	    seqin->Usa, seqin->Begin, seqin->End, seqin->Rev,
	    seqin->Formatstr, seqin->Format);

    if (!fmtexp)
	fmtexp = ajRegCompC ("^([A-Za-z0-9]*)::(.*)$");
    /* \1 format */
    /* \2 remainder */

    if (!dbexp)
	dbexp = ajRegCompC ("^([A-Za-z0-9_]+)([-]([A-Za-z]+))?([:{]([^}]*)}?)?$");
    /*	dbexp = ajRegCompC ("^([A-Za-z0-9_]+)([-]([Ii][Dd]|[Oo][Rr][Gg]|[Dd][Ee][Ss]|[Ss][Vv]|[Aa][Cc][Cc]|[Kk][Ee][Yy]))?([:{]([^}]*)}?)?$"); */

    /* \1 dbname */
    /* \2 -id or -acc */
    /* \3 qry->Field (id or acc) */
    /* \4 :qry->QryString */
    /* \5 qry->QryString */

    if (!idexp)				/* \1 is filename \4 is the qry->QryString */
	idexp = ajRegCompC ("^([^|]+[|]|[^:{%]+)(([:{%])(([^:}]+):)?([^:}]*)}?)?$");

    if (!listexp)			/* \1 is filename \3 is the qry->QryString */
	listexp = ajRegCompC ("^(@|[Ll][Ii][Ss][Tt]:+)(.+)$");

    if (!asisexp)			/* \1 is filename \3 is the qry->QryString */
	asisexp = ajRegCompC ("^[Aa][Ss][Ii][Ss]:+(.+)$");

    if (!wildexp)
	wildexp = ajRegCompC ("(.*[*].*)");
    /* \1 wildcard query */

    if (!rangeexp)	    /* \1 is rest of USA \2 start \3 end \5 reverse*/
	rangeexp = ajRegCompC ("(.*)[[](-?[0-9]*):(-?[0-9]*)(:([Rr])?)?[]]$");

    (void) ajStrAss (&usatest, seqin->Usa);
    /* Strip any leading spaces */
    ajStrTrimC(&usatest," \t\n");
    
    ajDebug("USA to test: '%S'\n\n", usatest);

    rangestat = ajRegExec (rangeexp, usatest);
    if (rangestat) {
      ajRegSubI(rangeexp, 2, &tmpstr);
      if (ajStrLen(tmpstr))
	ajStrToInt (tmpstr, &seqin->Begin);
      ajRegSubI(rangeexp, 3, &tmpstr);
      if (ajStrLen(tmpstr))
	ajStrToInt (tmpstr, &seqin->End);
      ajRegSubI(rangeexp, 5, &tmpstr);
      if (ajStrLen(tmpstr))
	seqin->Rev = ajTrue;
      ajRegSubI(rangeexp, 1, &usatest);
      ajDebug ("range found [%d:%d:%b]\n",
	       seqin->Begin, seqin->End, seqin->Rev);
    }

    asisstat = ajRegExec (asisexp, usatest);
    if (asisstat)
    {
	ajRegSubI (asisexp, 1, &qry->Filename);
	(void) ajStrAssC (&qry->Formatstr, "text");
	(void) ajStrAss (&seqin->Formatstr, qry->Formatstr);
	(void) seqFormatSet (thys, seqin);
	ajDebug ("asis sequence '%S'\n", qry->Filename);
	return ajSeqAccessAsis (seqin);
    }

    liststat = ajRegExec (listexp, usatest);
    fmtstat = ajRegExec (fmtexp, usatest);
    ajDebug("format regexp: %B list:%B\n", fmtstat, liststat);

    if (fmtstat && !liststat)
    {
	ajRegSubI (fmtexp, 1, &qry->Formatstr);
	/* default unknown */
	(void) ajStrSetC (&qry->Formatstr, seqInFormatDef[0].Name);
	ajRegSubI (fmtexp, 2, &usatest);
	ajDebug ("found format %S\n", qry->Formatstr);
	if (seqFindInFormat (qry->Formatstr, &seqin->Format))
	    (void) ajStrAss (&seqin->Formatstr, qry->Formatstr);
	else
	    ajDebug ("unknown format '%S'\n", qry->Formatstr);
    }
    else
    {
	ajDebug("no format specified in USA\n");
    }
    ajDebug ("\n");

    (void) seqFormatSet (thys, seqin);

    liststat = ajRegExec (listexp, usatest);
    if (liststat)
    {
	ajRegSubI (listexp, 2, &qrylist);
	ajDebug ("list found @%S fmt:%B range:%B\n",
		 qrylist, fmtstat, rangestat);
	if (seqin->Count && fmtstat)
	  ajWarn("List includes another list with format. Results undefined\n");
	if (seqin->Count && rangestat)
	  ajWarn("List includes another list with range. Results undefined\n");
	return seqListProcess (thys, seqin, qrylist);
    }

    regstat = ajRegExec (dbexp, usatest);
    ajDebug("dbname dbexp: %B\n", regstat);

    if (regstat)		/* clear it if this was really a file */
    {
	ajRegSubI (dbexp, 3, &qry->Field);
	ajRegSubI (dbexp, 1, &qrydb);
	if (!ajNamDatabase(qrydb))
	{
	    ajDebug ("unknown dbname %S, try filename\n", qrydb);
	    regstat = ajFalse;
	}
    }

    if (regstat)
    {
	ajRegSubI (dbexp, 5, &qry->QryString);
	(void) ajStrAss (&qry->DbName, qrydb);
	ajDebug ("found dbname '%S' level: '%S' qry->QryString: '%S'\n",
		 qry->DbName, qry->Field, qry->QryString);
	dbstat = ajNamDbData (qry);

	if (dbstat && ajStrLen(qry->QryString))
	{
	    /* ajDebug ("  qry->QryString %S\n", qry->QryString); */
	    if (ajStrLen(qry->Field))
	    {
		ajDebug ("  db QryString '%S' Field '%S'\n",
			 qry->QryString, qry->Field);
		if (ajStrMatchCaseC (qry->Field, "id"))
		    (void) ajStrAss (&qry->Id, qry->QryString);
		else if (ajStrMatchCaseC (qry->Field, "acc"))
		    (void) ajStrAss (&qry->Acc, qry->QryString);
		else {
		  if (!seqQueryField(qry, qry->Field)) {
		      ajErr ("USA '%S' query field '%S' not defined"
			     " for database '%S'",
			     usatest, qry->Field, qry->DbName);
		      return ajFalse;
		  }

		  /* treat Gi as another Sv, so no new query field */
		  if (ajStrMatchCaseC (qry->Field, "sv"))
		    (void) ajStrAss (&qry->Sv, qry->QryString);
		  else if (ajStrMatchCaseC (qry->Field, "des"))
		    (void) ajStrAss (&qry->Des, qry->QryString);
		  else if (ajStrMatchCaseC (qry->Field, "org"))
		    (void) ajStrAss (&qry->Org, qry->QryString);
		  else if (ajStrMatchCaseC (qry->Field, "key"))
		    (void) ajStrAss (&qry->Key, qry->QryString);
		  else
		    {
		      ajErr ("USA '%S' query level '%S' not supported",
			     usatest, qry->Field);
		      return ajFalse;
		    }
		}
	    }
	    else
	    {
		(void) ajStrAss (&qry->Id, qry->QryString);
		(void) ajStrAss (&qry->Acc, qry->QryString);
	    }
	}
	dbstat = ajNamDbQuery (qry);
	if (dbstat)
	{
	    ajDebug("database type: '%S' format '%S'\n",
		    qry->DbType, qry->Formatstr);
	    if (seqFindInFormat (qry->Formatstr, &seqin->Format))
		(void) ajStrAss (&seqin->Formatstr, qry->Formatstr);
	    else
		ajDebug ("unknown format '%S'\n", qry->Formatstr);

	    ajDebug ("use access method '%S'\n", qry->Method);
	    methstat = ajSeqMethod (qry->Method, &qry->Access);
	    if (!methstat)
	    {
		ajErr ("Access method '%S' unknown", qry->Method);
		return ajFalse;
	    }
	    else
	    {
		/* ajDebug ("trying access method '%S'\n", qry->Method); */

	      /* Calling funclist seqAccess() */
		accstat = qry->Access->Access (seqin);
		if (accstat)
		  return ajTrue;

		ajDebug ("Database '%S' : access method '%s' failed\n",
			 qry->DbName, qry->Access->Name);
		return ajFalse;
	    }
	}
	else
	{
	    ajErr ("no access method available for '%S'", usatest);
	    return ajFalse;
	}
    }
    else
    {
	ajDebug ("no dbname specified\n");
    }
    ajDebug ("\n");

    /* no database name, try filename */

    if (!dbstat)
    {
	regstat = ajRegExec (idexp, usatest);
	ajDebug ("entry-id regexp: %B\n", regstat);

	if (regstat)
	{
	    ajRegSubI (idexp, 1, &qry->Filename);
	    ajRegSubI (idexp, 3, &qrychr);
	    ajRegSubI (idexp, 5, &qry->Field);
	    ajRegSubI (idexp, 6, &qry->QryString);
	    ajDebug ("found filename %S\n", qry->Filename);
	    if (ajStrMatchC(qrychr, "%")) {
		ajStrToLong(qry->QryString, &qry->Fpos);
		accstat = ajSeqAccessOffset (seqin);
		if (accstat)
		  return ajTrue;
	    }
	    else
	    {
	      if (ajStrLen(qry->QryString))
	      {
		  ajDebug ("file QryString '%S' Field '%S' qrychr '%S'\n",
			   qry->QryString, qry->Field, qrychr);
		  if (ajStrLen(qry->Field)) /* set by dbname above */
		  {
		    /* ajDebug ("    qry->Field %S\n", qry->Field); */
		    if (ajStrMatchCaseC (qry->Field, "id"))
		      (void) ajStrAss (&qry->Id, qry->QryString);
		    else if (ajStrMatchCaseC (qry->Field, "acc"))
		      (void) ajStrAss (&qry->Acc, qry->QryString);

		  /* treat Gi as another Sv, so no new query field */

		    else if (ajStrMatchCaseC (qry->Field, "sv"))
		      (void) ajStrAss (&qry->Sv, qry->QryString);
		    else if (ajStrMatchCaseC (qry->Field, "des"))
		      (void) ajStrAss (&qry->Des, qry->QryString);
		    else if (ajStrMatchCaseC (qry->Field, "org"))
		      (void) ajStrAss (&qry->Org, qry->QryString);
		    else if (ajStrMatchCaseC (qry->Field, "key"))
		      (void) ajStrAss (&qry->Key, qry->QryString);
		    else	/* assume it was part of the filename */
		    {
		      ajErr("Unknown query field '%S' in USA '%S'",
			    qry->Field, usatest);
		      return ajFalse;
		    }
		  }
		  else
		  {
		    (void) ajStrAss (&qry->Id, qry->QryString);
		    (void) ajStrAss (&qry->Acc, qry->QryString);
		  }
	      }
	      accstat = ajSeqAccessFile (seqin);
	      if (accstat)
		return ajTrue;
	    }
	    ajErr ("failed to open filename %S ", qry->Filename);
	    return ajFalse;
	}
	else			/* dbstat and regstat both failed */
	{
	    ajDebug ("no filename specified\n");
	}
	ajDebug ("\n");
    }
  
    return accstat;
}

/* @funcstatic seqQueryField **************************************************
**
** Checks whether a query field is defined for a database as a "fields:"
** string in the database definition.
**
** @param [w] qry [AjPSeqQuery] Sequence query object
** @param [r] field [AjPStr] field name
** @return [AjBool] ajTrue if the field is defined
******************************************************************************/

static AjBool seqQueryField (AjPSeqQuery qry, AjPStr field) {

  static AjPStrTok handle = NULL;
  static AjPStr token = NULL;

  ajDebug("seqQueryField usa '%S' fields '%S'\n", field, qry->DbFields);
  (void) ajStrTokenAss (&handle, qry->DbFields, " ");
  while (ajStrToken (&token, &handle, NULL)) {
    ajDebug("seqQueryField test '%S'\n", token);
    if (ajStrMatchCase(token, field)) {
    ajDebug("seqQueryField match '%S'\n", token);
      (void) ajStrTokenReset (&handle);
      (void) ajStrDelReuse(&token);
      return ajTrue;
    }
  }

  return ajFalse;
}

/* @funcstatic seqUsaRestore **************************************************
**
** Restores a sequence input specification from a SeqPListUsa node
**
** @param [w] seqin [AjPSeqin] Sequence input object
** @param [w] node [SeqPListUsa] Usa list node
** @return [void]
******************************************************************************/

static void seqUsaRestore (AjPSeqin seqin, SeqPListUsa node) {

  seqin->Begin = node->Begin;
  seqin->End = node->End;
  seqin->Rev = node->Rev;
  seqin->Format = node->Format;
  seqin->Features = node->Features;
  ajStrAssS(&seqin->Formatstr, node->Formatstr);

  return;
}

/* @funcstatic seqUsaSave *****************************************************
**
** Saves a sequence input specification in a SeqPListUsa node
**
** @param [r] node [SeqPListUsa] Usa list node
** @param [w] seqin [AjPSeqin] Sequence input object
** @return [void]
******************************************************************************/

static void seqUsaSave (SeqPListUsa node, AjPSeqin seqin) {
  node->Begin = seqin->Begin;
  node->End = seqin->End;
  node->Rev = seqin->Rev;
  node->Format = seqin->Format;
  node->Features = seqin->Features;
  ajStrAssS(&node->Formatstr, seqin->Formatstr);

  return;
}

/* @funcstatic seqUsaListTrace ************************************************
**
** Traces the nodes in a USA list
**
** @param [r] list [AjPList] The USA list
** @return [void]
******************************************************************************/

static void seqUsaListTrace (AjPList list) {
  AjIList iter;
  SeqPListUsa node;
  ajint i=0;

  iter = ajListIter(list);

  ajDebug ("SeqUsaListTrace %d nodes\n", ajListLength(list));
  while (ajListIterMore(iter)) {
    node = (SeqPListUsa) ajListIterNext(iter);
    ajDebug ("%3d: '%S' %4d..%d (%b) '%S' %d\n",
	     ++i, node->Usa, node->Begin, node->End, node->Rev,
	     node->Formatstr, node->Format);
  }
  ajListIterFree(iter);
  ajDebug("...Done...\n");
}

/* @funcstatic seqListProcess *************************************************
**
** Processes a file of USAs.
** This function is called by, and calls, seqUsaProcess. There is
** a depth check to avoid infinite loops, for example where a list file
** refers to itself.
**
** This function produces a list (AjPList) of USAs with all list references
** expanded into lists of USAs.
**
** Because USAs in a list can have their own begin, end and reverse settings
** the prior setting are stored with each USA in the list node so that they
** can be restored after.
**
** @param [r] seq [AjPSeq] Sequence
** @param [r] seqin [AjPSeqin] Sequence input
** @param [r] listfile [AjPStr] Name of list file.,
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqListProcess (AjPSeq seq, AjPSeqin seqin, AjPStr listfile)
{
    AjPList list = NULL;
    AjPFile file = NULL;
    AjPStr rdline = NULL;
    AjPStr token = NULL;
    static AjPStrTok handle = NULL;
    AjBool ret = ajFalse;
    SeqPListUsa node = NULL;

    static ajint depth = 0;
    static ajint MAXDEPTH = 16;

    depth++;
    ajDebug ("++seqListProcess %S depth %d\n", listfile, depth);
    if (depth > MAXDEPTH)
	ajFatal ("USA List too deep");

    if (!ajListLength(seqin->List))
	seqin->List = ajListNew();

    list = ajListNew();

    file = ajFileNewIn(listfile);
    if (!file)
    {
	ajErr ("unable to open list file '%S'", listfile);
	depth--;
	return ret;
    }
    while (ajFileGetsTrim(file, &rdline))
    {
	seqListNoComment (&rdline);
	if (ajStrLen(rdline))
	{
	    (void) ajStrTokenAss (&handle, rdline, " \t\n\r");
	    (void) ajStrToken (&token, &handle, NULL);
	    /* (void) ajDebug ("Line  '%S'\ntoken '%S'\n", rdline, token); */
	    if (ajStrLen(token))
	    {
	        ajDebug("++Add to list: '%S'\n", token);
	        AJNEW0 (node);
	        ajStrAssS(&node->Usa, token);
	        seqUsaSave(node, seqin);
	        ajListPushApp(list, node);
	    }
	    ajStrDel(&token);
	    token = NULL;
	}
    }
    ajFileClose(&file);
    ajStrDel (&rdline);

    ajDebug("Trace seqin->List\n");
    seqUsaListTrace (seqin->List);
    ajDebug("Trace new list\n");
    seqUsaListTrace (list);
    ajListPushList (seqin->List, &list);

    ajDebug("Trace combined seqin->List\n");
    seqUsaListTrace (seqin->List);

    /* now try the first item on the list */
    /* this can descend recursively if it is also a list */
    /* which is why we check the depth above */

    if (ajListPop (seqin->List, (void**) &node))
    {
        ajDebug("++pop first item '%S'\n", node->Usa);
	ajSeqinUsa (&seqin, node->Usa);
	ajStrDel (&node->Usa);
	ajStrDel (&node->Formatstr);
	AJFREE (node);
	ajDebug ("descending with usa '%S'\n", seqin->Usa);
	ret = seqUsaProcess (seq, seqin);
    }

    (void) ajStrTokenReset (&handle);
    depth--;
    ajDebug("++seqListProcess depth: %d returns: %B\n", depth, ret);
    return ret;
}

/* @funcstatic seqListNoComment ***********************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file.
** @return [void]
** @@
******************************************************************************/

static void seqListNoComment (AjPStr* text)
{
    ajint i;
    char *cp;

    i = ajStrLen (*text);

    if (!i)				/* empty string */
	return;

    cp = strchr(ajStrStr(*text), '#');
    if (cp)
    {					/* comment found */
	*cp = '\0';
	ajStrFix (*text);
    }

    return;
}

/* @funcstatic seqFormatSet ***************************************************
**
** Sets the input format for a sequence using the sequence input object's
** defined format, or a default from variable 'EMBOSS_FORMAT'.
**
** @param [r] thys [AjPSeq] Sequence.
** @param [r] seqin [AjPSeqin] Sequence input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqFormatSet (AjPSeq thys, AjPSeqin seqin)
{

    if (ajStrLen(seqin->Formatstr))
    {
	ajDebug ("... input format value '%S'\n", seqin->Formatstr);
	if (seqFindInFormat (seqin->Formatstr, &seqin->Format))
	{
	    (void) ajStrAss (&thys->Formatstr, seqin->Formatstr);
	    thys->Format = seqin->Format;
	    ajDebug ("...format OK '%S' = %d\n", seqin->Formatstr,
		     seqin->Format);
	}
	else
	{
	    ajDebug ("...format unknown '%S'\n", seqin->Formatstr);
	}
	return ajTrue;
    }
    else
    {
	ajDebug ("...input format not set\n");
    }

    return ajFalse;
}

/* @funcstatic seqinUfoLocal **************************************************
**
** Tests whether a sequence input object will read features from the
** sequence input file. The alternative is to use a separate UFO.
**
** @param [u] thys [AjPSeqin] Sequence input object.
** @return [AjBool] ajTrue if the features will be read from the sequence
** @@
******************************************************************************/

static AjBool seqinUfoLocal (AjPSeqin thys) {
  if (thys->Features && ! ajStrLen(thys->Ufo))
    return ajTrue;

  return ajFalse;
}
/* @funcstatic seqSetName *****************************************************
**
** Sets the name for a sequence object by applying simple conversion
** rules to the input which could be, for example, the name from a
** FASTA format file.
**
** @param [u] name [AjPStr*] Sequence name derived.
** @param [r] str [AjPStr] User supplied name.
** @return [void]
** @@
******************************************************************************/

static void seqSetName (AjPStr* name, AjPStr str)
{
    static AjPRegexp idexp = NULL;

    if (!idexp)
	idexp = ajRegCompC("([^ \t\n\r,<>|;]+:)?([^ \t\n\r,<>|;]+)");

    if (ajRegExec(idexp, str))
	ajRegSubI(idexp, 2, name);
    else
	(void) ajStrAssS (name, str);

    ajDebug("seqSetName '%S' result: '%S'\n", str, *name);

    return;
}

/* @funcstatic seqAccSave ********************************************
**
** Adds an accession number to the stored list for a sequence.
** The first accession number is also saved as the primary number.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] acc [AjPStr] Accession number
** @return [void]
** @@
******************************************************************************/

static void seqAccSave (AjPSeq thys, AjPStr acc)
{
    AjPStr liststr;		/* do not free - it is stored in a list */

    liststr = ajStrNewS (acc);
    ajListstrPushApp (thys->Acclist, liststr);

    if (!ajStrLen(thys->Acc))
	ajStrAssS (&thys->Acc, acc);

    return;
}

/* @funcstatic seqTaxSave ********************************************
**
** Adds an organism taxonomy level to the stored list for a sequence.
** The first is also saved as the primary 'Tax' (should be the species).
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] tax [AjPStr] Organism taxonomy
** @return [void]
** @@
******************************************************************************/

static void seqTaxSave (AjPSeq thys, AjPStr tax)
{
    AjPStr liststr;		/* do not free - it is stored in a list */

    liststr = ajStrNewS (tax);
    ajListstrPushApp (thys->Taxlist, liststr);

    if (!ajStrLen(thys->Tax))
	ajStrAssS (&thys->Tax, tax);

    return;
}

/* @funcstatic seqSvSave ********************************************
**
** Adds a sequence version number to the stored data for a sequence.
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] sv [AjPStr] SeqVersion number
** @return [void]
** @@
******************************************************************************/

static void seqSvSave (AjPSeq thys, AjPStr sv)
{
    if (!ajStrLen(thys->Sv))
	ajStrAssS (&thys->Sv, sv);

    return;
}

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Query Constructors **************************************
**
** All constructors return a new sequence query object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequenceoutput object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/

/* @func ajSeqQueryNew ********************************************************
**
** Creates a new sequence query object
**
** @return [AjPSeqQuery] New sequence query object.
** @@
******************************************************************************/

AjPSeqQuery ajSeqQueryNew (void)
{
    AjPSeqQuery pthis;

    AJNEW0(pthis);

    pthis->DbName = ajStrNew();
    pthis->Id = ajStrNew();
    pthis->Acc = ajStrNew();
    pthis->Sv = ajStrNew();
    pthis->Des = ajStrNew();
    pthis->Org = ajStrNew();
    pthis->Key = ajStrNew();
    pthis->Method = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->IndexDir = ajStrNew();
    pthis->Directory = ajStrNew();
    pthis->Filename = ajStrNew();
    pthis->Application = ajStrNew();
    pthis->Field = ajStrNew();

    pthis->Type = QRY_UNKNOWN;
    pthis->Access = NULL;
    pthis->QryData = NULL;
    pthis->Fpos = NULLFPOS;
    pthis->QryDone = ajFalse;

    return pthis;
}

/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Query Destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence query object.
**
******************************************************************************/

/* @func ajSeqQueryDel ********************************************************
**
** Deletes a sequence query object
**
** @param [wP] pthis [AjPSeqQuery*] Address of sequence query object
** @return [void]
** @@
******************************************************************************/

void ajSeqQueryDel (AjPSeqQuery* pthis)
{
    AjPSeqQuery thys = *pthis;

    ajStrDel(&thys->DbName);
    ajStrDel(&thys->Id);
    ajStrDel(&thys->Acc);
    ajStrDel(&thys->Sv);
    ajStrDel(&thys->Des);
    ajStrDel(&thys->Org);
    ajStrDel(&thys->Key);
    ajStrDel(&thys->Method);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->IndexDir);
    ajStrDel(&thys->Directory);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Application);
    ajStrDel(&thys->QryString);

    ajStrDel(&thys->Field);

    if(thys->QryData)
	AJFREE(thys->QryData);

    AJFREE(*pthis);

    return;
}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Query Assignments ***************************************
**
** These functions overwrite the sequence query object provided as
** the first argument.
**
******************************************************************************/

/* @func ajSeqQueryClear ******************************************************
**
** Resets a Sequence query object to a clean state for reuse
**
** @param [uP] thys [AjPSeqQuery] Sequence query object
** @return [void]
** @@
******************************************************************************/


void ajSeqQueryClear (AjPSeqQuery thys)
{

    (void) ajStrClear(&thys->DbName);
    (void) ajStrClear(&thys->Id);
    (void) ajStrClear(&thys->Acc);
    (void) ajStrClear(&thys->Sv);
    (void) ajStrClear(&thys->Des);
    (void) ajStrClear(&thys->Org);
    (void) ajStrClear(&thys->Key);
    (void) ajStrClear(&thys->Method);
    (void) ajStrClear(&thys->Formatstr);
    (void) ajStrClear(&thys->IndexDir);
    (void) ajStrClear(&thys->Directory);
    (void) ajStrClear(&thys->Filename);
    (void) ajStrClear(&thys->Application);

    thys->Type = QRY_UNKNOWN;
    thys->Access = NULL;
    if(thys->QryData)
	AJFREE(thys->QryData);
    thys->QryDone = ajFalse;

    return;
}

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Query Modifiers *****************************************
**
** These functions use the contents of a sequence query object and
** update them.
**
******************************************************************************/


/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Query Operators *****************************************
**
** These functions use the contents of a sequence query object but do
** not make any changes.
**
******************************************************************************/

/* @funcstatic seqQueryMatch **************************************************
**
** Compares a sequence to a query and returns true if they match.
**
** @param [P] thys [AjPSeq] Sequence.
** @param [P] query [AjPSeqQuery] Sequence query.
** @return [AjBool] ajTrue if the sequence matches the query.
** @@
******************************************************************************/

static AjBool seqQueryMatch (AjPSeq thys, AjPSeqQuery query)
{
    AjBool tested = ajFalse;
    AjIList iter = NULL;
    AjPStr accstr;		/* from list, do not delete */
    AjPStr keystr;		/* from list, do not delete */
    AjPStr taxstr;		/* from list, do not delete */
    AjBool ok = ajFalse;

    ajDebug ("seqQueryMatch '%S' id '%S' acc '%S' Sv '%S' Des '%S'"
	     " Key '%S' Org '%S'\n",
	     thys->Name, query->Id, query->Acc, query->Sv,
	     query->Des, query->Key, query->Org);

    ajStrAssS (&thys->Entryname, thys->Name);

    if (!query)			/* no query to test, that's fine */
	return ajTrue;

    if (query->QryDone)		/* do we need to test here? */
	return ajTrue;

    /* test the query field(s) */

    if (ajStrLen(query->Id))
    {
	if (ajStrMatchWild (thys->Name, query->Id))
	    return ajTrue;

	ajDebug ("id test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if (ajStrLen(query->Sv))	/* test Sv and Gi */
    {
	if (ajStrMatchWild (thys->Sv, query->Sv))
	    return ajTrue;

	if (ajStrMatchWild (thys->Gi, query->Sv))
	    return ajTrue;

	ajDebug ("sv test failed\n");
	tested = ajTrue;
	ok = ajFalse;
    }

    if (!ajStrLen(query->Acc))
    {
	ajDebug ("No accession number to test\n");
    }
    else if (ajListLength(thys->Acclist))
    {			   /* accession number test - check the entire list */
	iter = ajListIter (thys->Acclist);
	while (ajListIterMore(iter))
	{
	    accstr = ajListIterNext(iter);
	    ajDebug ("... try accession '%S' '%S'\n", accstr,
		     query->Acc);
	    
	    if (ajStrMatchWild (accstr, query->Acc))
	      return ajTrue;
	}
	tested = ajTrue;
	ajDebug ("acc test failed\n");
	ajListIterFree(iter);
    }

    if (!ajStrLen(query->Org))
    {
	ajDebug ("No taxonomy to test\n");
    }
    else if (ajListLength(thys->Taxlist))
    {			   /* taxonomy test - check the entire list */
	iter = ajListIter (thys->Taxlist);
	while (ajListIterMore(iter))
	{
	    taxstr = ajListIterNext(iter);
	    ajDebug ("... try organism '%S' '%S'\n", taxstr,
		     query->Org);
	    
	    if (ajStrMatchWild (taxstr, query->Org))
	      return ajTrue;
	}
	tested = ajTrue;
	ajDebug ("org test failed\n");
	ajListIterFree(iter);
    }
    else
    {
      ajDebug ("org test failed - nothing to test\n");
      return ajFalse;
    }

    if (!ajStrLen(query->Key))
    {
	ajDebug ("No keyword to test\n");
    }
    else if (ajListLength(thys->Keylist))
    {			   /* keyword test - check the entire list */
	iter = ajListIter (thys->Keylist);
	while (ajListIterMore(iter))
	{
	    keystr = ajListIterNext(iter);
	    ajDebug ("... try keyword '%S' '%S'\n", keystr,
		     query->Key);
	    
	    if (ajStrMatchWild (keystr, query->Key))
	      return ajTrue;
	}
	tested = ajTrue;
	ajDebug ("key test failed\n");
	ajListIterFree(iter);
    }
    else
    {
      ajDebug ("key test failed - nothing to test\n");
      return ajFalse;
    }

    if (!ajStrLen(query->Des))
    {
	ajDebug ("No description to test\n");
	ok = ajFalse;
    }
    else if (ajStrLen(thys->Desc))
    {			   /* description test - check the string */
	ajDebug ("... try description '%S' '%S'\n", thys->Desc,
		     query->Des);
	    
	if (ajStrMatchWord (thys->Desc, query->Des))
	  return ajTrue;

	tested = ajTrue;
	ajDebug ("des test failed\n");
	ajListIterFree(iter);
    }
    else
    {
      ajDebug ("des test failed - nothing to test\n");
      return ajFalse;
    }

    if (!tested)		/* nothing to test, so accept it anyway */
	return ajTrue;

    ajDebug ("result: %B\n", ok);
    return ok;
}

/* @func ajSeqQueryWild *******************************************************
**
** Tests whether a query includes wild cards in any element,
** of can return more than one entry.
**
** @param [r] qry [AjPSeqQuery] Query object.
** @return [AjBool] ajTrue if query had wild cards.
** @@
******************************************************************************/

AjBool ajSeqQueryWild (AjPSeqQuery qry)
{

    if (!qrywildexp)
	seqQryWildComp();

    ajDebug("ajSeqQueryWild id '%S' acc '%S' sv '%S' des '%S'"
	    " org '%S' key '%S'\n",
	    qry->Id, qry->Acc, qry->Sv, qry->Des, qry->Org, qry->Key);

    ajSeqQueryStarclear (qry);

    if (ajRegExec(qrywildexp, qry->Id))
    {
	ajDebug("wild query Id '%S'\n", qry->Id);
	return ajTrue;
    }

    if (ajStrLen(qry->Acc))
    {
        if (!ajStrLen(qry->Id))
	{
	    ajDebug("wild (has, but no Id) query Acc '%S'\n", qry->Acc);
	    return ajTrue;
	}
	else if (ajRegExec(qrywildexp, qry->Id))
        {
	    ajDebug ("wild query Acc '%S'\n", qry->Acc);
	    return ajTrue;
	}
    }

    if (ajStrLen(qry->Sv))
    {
	ajDebug("wild (has) query Sv '%S'\n", qry->Sv);
	return ajTrue;
    }

    if (ajStrLen(qry->Des))
    {
	ajDebug("wild (has) query Des '%S'\n", qry->Des);
	return ajTrue;
    }

    if (ajStrLen(qry->Org))
    {
	ajDebug("wild (has) query Org '%S'\n", qry->Org);
	return ajTrue;
    }

    if (ajStrLen(qry->Key))
    {
	ajDebug("wild (has) query Key '%S'\n", qry->Key);
	return ajTrue;
    }

    ajDebug("no wildcard in stored qry\n");

    return ajFalse;
}

/* @func ajSeqQueryStarclear **************************************************
**
** Clears elements of a query object if they are simply "*" because this
** is equivalent to a null string.
**
** @param [u] qry [AjPSeqQuery] Query object.
** @return [void]
** @@
******************************************************************************/

void ajSeqQueryStarclear (AjPSeqQuery qry)
{
    if (ajStrMatchC(qry->Id, "*"))
    {
	ajDebug ("ajSeqQueryWild clear Id '%S'\n", qry->Id);
	(void) ajStrClear(&qry->Id);
    }

    if (ajStrMatchC(qry->Acc, "*"))
    {
	ajDebug ("ajSeqQueryWild clear Acc '%S'\n", qry->Acc);
	(void) ajStrClear(&qry->Acc);
    }

    if (ajStrMatchC(qry->Sv, "*"))
    {
	ajDebug ("ajSeqQueryWild clear Sv '%S'\n", qry->Sv);
	(void) ajStrClear(&qry->Sv);
    }

    if (ajStrMatchC(qry->Des, "*"))
    {
	ajDebug ("ajSeqQueryWild clear Des '%S'\n", qry->Des);
	(void) ajStrClear(&qry->Des);
    }

    if (ajStrMatchC(qry->Org, "*"))
    {
	ajDebug ("ajSeqQueryWild clear Org '%S'\n", qry->Org);
	(void) ajStrClear(&qry->Org);
    }

     if (ajStrMatchC(qry->Key, "*"))
    {
	ajDebug ("ajSeqQueryWild clear Key '%S'\n", qry->Key);
	(void) ajStrClear(&qry->Key);
    }

    return;
}

/* @func ajSeqQueryIs *********************************************************
**
** Tests whether any element of a query has been set. Elements which
** are simply '*' are cleared as this has the same meaning.
**
** @param [u] qry [AjPSeqQuery] Query object.
** @return [AjBool] ajTrue if query should be made. ajFalse if the query
**                  includes all entries.
** @@
******************************************************************************/

AjBool ajSeqQueryIs (AjPSeqQuery qry)
{

    ajSeqQueryStarclear (qry);

    if (ajStrLen(qry->Id)) return ajTrue;
    if (ajStrLen(qry->Acc)) return ajTrue;
    if (ajStrLen(qry->Sv)) return ajTrue;
    if (ajStrLen(qry->Des)) return ajTrue;
    if (ajStrLen(qry->Org)) return ajTrue;
    if (ajStrLen(qry->Key)) return ajTrue;

    return ajFalse;
}

/* @funcstatic seqQryWildComp ************************************************
**
** Compiles the reqular expressions for testing wild cards ion queries.
** These are held in static storage and built once only if needed.
**
** @return [void]
** @@
******************************************************************************/

static void seqQryWildComp (void)
{
    if (!qrywildexp)
	qrywildexp = ajRegCompC ("[*?]");

    return;
}

/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */

/* @section Sequence Query Casts ********************************************
**
** These functions examine the contents of a sequence query object
** and return some derived information. Some of them provide access to
** the internal components of a sequence query object. They are
** provided for programming convenience but should be used with
** caution.
**
******************************************************************************/


/* @func ajSeqParseFasta ******************************************************
**
** Parse an NCBI format fasta line. Return id acc sv and description
**
** @param [r] instr [AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if fasta format
** @@
******************************************************************************/

AjBool ajSeqParseFasta(AjPStr instr, AjPStr* id, AjPStr* acc,
		       AjPStr* sv, AjPStr* desc)
{
    static AjPStrTok handle = NULL;
    static AjPStr token = NULL;
    static AjPStr str = NULL;
    AjBool ok = ajFalse;

    ajDebug("ajSeqParseFasta '%S'\n", instr);

    ajStrAssS (&str, instr);

    if (!ajStrPrefixC(str, ">"))
      return ajFalse;

    (void) ajStrTokenAss (&handle, str, "> ");
    (void) ajStrToken (id, &handle, " \t\n\r");
  
    ok = ajStrToken (&token, &handle, NULL);

    if (ok && ajIsSeqversion(token))
    {
        (void) ajStrAssS (acc, ajIsSeqversion(token));
	(void) ajStrAssS (sv, token);
	(void) ajStrToken (desc, &handle, "\n\r");
    }
    else if (ok && ajIsAccession(token))
    {
	(void) ajStrAssS (acc, token);
        (void) ajStrAssC (sv, "");
	(void) ajStrToken (desc, &handle, "\n\r");
    }
    else
    {
        (void) ajStrAssC (acc, "");
        (void) ajStrAssC (sv, "");
	(void) ajStrAssS (desc, token);
	if (ajStrToken (&token, &handle, "\n\r"))
	{
	    (void) ajStrAppC (desc, " ");
	    (void) ajStrApp (desc, token);
	}
    }

    (void) ajStrDelReuse(&token);  /* duplicate of accession or description */
    (void) ajStrTokenReset (&handle);

    ajDebug("result id: '%S' acc: '%S' desc: '%S'\n", *id, *acc, *desc);

    return ajTrue;
}


/* @func ajSeqParseNcbi *******************************************************
**
** Parse an NCBI format fasta line. Return id acc and description.
**
** Tries to cope with the amazing variety of identifiers NCBI inflicts
** on us all - see the BLAST document README.formatdb from NCBI for
** some of the gory detail, and look at some real files for clues
** to what can really happen. Sadly,' real files' also includes
** internal IDs in blast databases reformatted by formatdb.
**
** @param [r] instr [AjPStr]   fasta line.
** @param [w] id [AjPStr*]   id.
** @param [w] acc [AjPStr*]  accession number.
** @param [w] sv [AjPStr*]  sequence version number.
** @param [w] gi [AjPStr*]  GI version number.
** @param [w] desc [AjPStr*] description.
** @return [AjBool] ajTrue if ncbi format
** @@
******************************************************************************/
AjBool ajSeqParseNcbi(AjPStr instr, AjPStr* id, AjPStr* acc,
		      AjPStr* sv, AjPStr* gi, AjPStr* desc)
{
    static AjPStrTok idhandle = NULL;
    static AjPStrTok handle = NULL;
    static AjPStr idstr = NULL;
    static AjPStr reststr = NULL;
    static AjPStr prefix = NULL;
    static AjPStr token = NULL;
    static AjPStr numtoken = NULL;
    static AjPStr str = NULL;
    static AjPStr vacc = NULL;
    char *q;
    ajint  i;
    ajint  nt;
    
    /* NCBI's list of standard identifiers June 2001
    ** ftp://ncbi.nlm.nih.gov/blast/db/README.formatdb
    **
    ** Database Name                         Identifier Syntax
    ** 
    ** GenBank                               gb|accession|locus
    ** EMBL Data Library                     emb|accession|locus
    ** DDBJ, DNA Database of Japan           dbj|accession|locus
    ** SWISS-PROT                            sp|accession|entry name
    ** NCBI Reference Sequence               ref|accession|locus
    **
    ** General database identifier           gnl|database|identifier
    ** BLAST formatdb                        gnl|BL_ORD_ID|number
    **   (prefix for normal FASTA header - remove)
    **
    ** NBRF PIR                              pir||entry
    ** Protein Research Foundation           prf||name
    **   (Japanese SEQDB protein DB)
    **
    ** Brookhaven Protein Data Bank          pdb|entry|chain
    **
    ** Patents                               pat|country|number 
    **
    ** GenInfo Backbone Id                   bbs|number 
    ** Local Sequence identifier             lcl|identifier
    **
    ** GenInfo identifier prefix             gi|gi_identifier
    **   (prefix - remove)
    */

    /* (void) ajDebug("ajSeqParseNcbi '%S'\n", instr);  */

    if (ajStrChar(instr, 3) == ';')	/* then it is really PIR format */
	return ajFalse;

     ajStrAssS (&str, instr);

     /* (void) ajDebug ("id test %B %B\n",
		    !strchr(MAJSTRSTR(str), (ajint)'|'),
		    (*MAJSTRSTR(str)!='>')); */

    /* Line must start with '>', and include '|' bar, hopefully in the ID */

    if(*MAJSTRSTR(str)!='>') {
	return ajFalse;
    }

    /* pick out the ID */

    (void) ajStrTokenAss(&idhandle,str,"> \t\r\n");
    (void) ajStrToken(&idstr, &idhandle, NULL);
    (void) ajStrToken(&reststr, &idhandle, "\r\n");
    (void) ajStrTokenReset(&idhandle);

    /* check we have an ID */

    if (!ajStrLen(idstr)) {
      /* (void) ajDebug ("No ID string found\n"); */
      return ajFalse;
    }

    /* NCBI ids always have | somewhere. Else we try a simple FASTA format */

    if(!strchr(MAJSTRSTR(idstr),(ajint)'|')) {
	return ajSeqParseFasta (str, id, acc, sv, desc);
    }

    (void) ajStrTokenAss(&handle,idstr,"|");

    (void) ajStrToken(&prefix, &handle, NULL);
    q = MAJSTRSTR(prefix);

    /* (void) ajDebug (" idstr: '%S'\n", idstr); */
    /* (void) ajDebug ("prefix: '%S'\n", prefix); */

    if(!strncmp(q,"gi",2))
    {
        /* (void) ajDebug("gi prefix\n"); */
	(void) ajStrToken(&token, &handle, NULL);
	ajStrAssS(gi, token);
	if (! ajStrToken(&prefix, &handle, NULL)) {
	  /* we only have a gi prefix */
	  /* (void) ajDebug("*only* gi prefix\n"); */
	  (void) ajStrAssS(id, token);
	  (void) ajStrAssC(acc, "");
	  (void) ajStrAssS (desc, reststr);
	  /* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	     prefix, *id, *acc, *desc); */
	  return ajTrue;
	}

	/* otherwise we continue to parse the rest */
	q = MAJSTRSTR(prefix);
	/* (void) ajDebug("continue with '%S'\n", prefix); */
    }


/*
 * This next routine and associated function could be used if
 * whatever is appended to gnl lines is consistent
 */
   
    if(!strncmp(MAJSTRSTR(idstr),"gnl|BL_ORD_ID|",14))
    {
        /* (void) ajDebug("gnl|BL_ORD_ID stripping\n"); */
	(void) ajStrToken(&token, &handle, NULL); /* BL_ORD_ID */
	(void) ajStrToken(&numtoken, &handle, NULL); /* number */
	(void) ajStrInsertC(&reststr, 0, ">");

	if(ajSeqParseNcbi(reststr,id,acc,sv,gi,desc)) {	/* recursive ... */
	  /* (void) ajDebug("ajSeqParseNcbi recursive success\n"); */
	  /* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' sv: '%S' desc: '%S'\n",
	     prefix, *id, *acc, *sv, *desc); */
	  return ajTrue;
	}
        /* (void) ajDebug("ajSeqParseNcbi recursive failed - use the gnl id\n"); */
	(void) ajStrAss(id,numtoken);
	(void) ajStrAssC(acc,"");
	/* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' sv: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *sv, *desc); */
	return ajTrue;
    }

/* works for NCBI formatdb reformatted blast databases
** still checking for any misformatted databases elsewhere */

    if(!strcmp(q,"bbs") || !strcmp(q,"lcl"))
    {
        /* (void) ajDebug("bbs or lcl prefix\n"); */
	(void) ajStrToken(id, &handle, NULL);
	(void) ajStrAssC(acc,"");
	(void) ajStrAssS(desc, reststr);
	/* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	return ajTrue;
    }
    
    if(!strcmp(q,"gnl") || !strcmp(q,"pat"))
    {
        /* (void) ajDebug("gnl or pat or pdb prefix\n"); */
	(void) ajStrToken(&token, &handle, NULL);
	(void) ajStrToken(id, &handle, NULL);
	(void) ajStrAssC(acc,""); /* no accession number */
	(void) ajStrAssS(desc, reststr);
	/* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	  prefix, *id, *acc, *desc); */
	return ajTrue;
    }
    
	
    if(!strcmp(q,"pdb"))
    {
        /* (void) ajDebug("gnl or pat or pdb prefix\n"); */
	(void) ajStrToken(id, &handle, NULL);
	if (ajStrToken(&token, &handle, NULL)) {
	  /* chain identifier to append */
	  (void) ajStrApp(id, token);
	}
	(void) ajStrAssC(acc,""); /* no accession number */
	(void) ajStrAssS(desc, reststr);
	/* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	return ajTrue;
    }
    
	
    if(!strcmp(q,"gb") || !strcmp(q,"emb") || !strcmp(q,"dbj")
       || !strcmp(q,"sp") || !strcmp(q,"ref"))
    {
        /* (void) ajDebug("gb,emb,dbj,sp,ref prefix\n"); */
	(void) ajStrToken(&token, &handle, NULL);
	vacc = ajIsSeqversion(token);
	if (vacc)
	  {
	    (void) ajStrAss(sv,token);
	    (void) ajStrAss(acc,vacc);
	  }
	else if (ajIsAccession(token))
	  (void) ajStrAss(acc,token);

	if (!ajStrToken(id, &handle, NULL)) {
	  /* no ID, reuse accession token */
	  (void) ajStrAssS (id, token);
	}
	(void) ajStrAssS(desc, reststr);
	/* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	return ajTrue;
    }


    if(!strcmp(q,"pir") || !strcmp(q,"prf"))
    {
        /* (void) ajDebug("pir,prf prefix\n"); */
	(void) ajStrToken(id, &handle, NULL);
	(void) ajStrAssS(desc, reststr);
	(void) ajStrAssC(acc, "");
	/* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
	   prefix, *id, *acc, *desc); */
	return ajTrue;
    }


  /* else assume that the last two barred tokens contain [acc]|id */

    /* (void) ajDebug("No prefix accepted - try the last 2 fields\n"); */

    nt = ajStrTokenCount(&idstr,"|");

    /* (void) ajDebug ("Barred tokens - %d found\n", nt); */

  if(nt < 2)
    return ajFalse;

  /* restart parsing with only bars */

  (void) ajStrTokenAss(&handle,str,"|");
  for(i=0;i<nt-2;++i)
      (void) ajStrToken(&token, &handle, NULL);
  
  (void) ajStrToken (&token, &handle, NULL);
  /* (void) ajDebug ("token acc: '%S'\n", token); */
  vacc = ajIsSeqversion(token);
  if (vacc)
  {
    (void) ajStrAss(sv,token);
    (void) ajStrAss(acc,vacc);
  }
  else if (ajIsAccession(token))
    (void) ajStrAss(acc,token);

  (void) ajStrToken (&token, &handle, " \n\t\r");
  /* (void) ajDebug ("token id: '%S'\n", token); */

  (void) ajStrAss(id,token);

  (void) ajStrToken (&token, &handle, "\n\r");
  (void) ajStrAss (desc, token);

  /* (void) ajDebug ("found pref: '%S' id: '%S', acc: '%S' desc: '%S'\n",
     prefix, *id, *acc, *desc); */
  return ajTrue;
}


/* @func ajSeqGetFromUsa ***********************************************
**
** Returns a sequence given a USA
**
** @param [r] thys [AjPStr] USA
** @param [r] protein [AjBool] True if protein
** @param [w] seq [AjPSeq*] sequence
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajSeqGetFromUsa (AjPStr thys, AjBool protein, AjPSeq *seq)
{
    AjPSeqin seqin;
    AjBool ok;
  
  
    seqin = ajSeqinNew();
    seqin->multi = ajFalse;
    seqin->Text  = ajFalse;
  
    if(!protein)
	ajSeqinSetNuc (seqin);
    else
	ajSeqinSetProt (seqin);

    ajSeqinUsa (&seqin, thys);
    ok = ajSeqRead(*seq, seqin);
    ajSeqinDel (&seqin);


    if(!ok)
	return ajFalse;

    return ajTrue;
}

/* @funcstatic seqTextSeq *****************************************************
**
** Saves a sequence from a string into the text output pointer
**
** Could do some extra formatting here (left margin, numbering)
** but as the EMBOSS formats are not too fussy that can wait.
**
** @param [W] textptr [AjPStr*] Text output
** @param [R] seq [AjPStr] sequence as a string
** @return [void]
******************************************************************************/

static void seqTextSeq (AjPStr* textptr, AjPStr seq) {

  ajint i;
  ajint istart;
  ajint iend;
  ajint ilen = ajStrLen(seq);
  static ajint iwidth=60;
  static AjPStr tmpstr=NULL;

  for (i=0; i < ilen; i += iwidth) {
    istart = i;
    iend = AJMIN(ilen-1, istart+iwidth-1);
    ajStrAssSub(&tmpstr, seq, istart, iend);
    ajFmtPrintAppS (textptr, "%S\n", tmpstr);
  }

  return;
}

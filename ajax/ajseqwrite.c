#include "ajax.h"

/* @datastatic SeqPOutFormat **************************************************
**
** Sequence output formats
**
** @attr Name [char*] format name
** @attr Single [AjBool] Write each sequence to a new file if true (e.g. GCG)
** @attr Save [AjBool] Save in memory and write at end (e.g. MSF alignments)
** @attr Write [(void*)] Function to write the format
** @@
******************************************************************************/

typedef struct SeqSOutFormat
{
    char *Name;
    AjBool Single;
    AjBool Save;
    void (*Write) (AjPSeqout outseq);
} SeqOOutFormat, *SeqPOutFormat;

/* @datastatic SeqPSeqFormat **************************************************
**
** Data structure to hold definitions when writing sequence data.
**
** Most output functions generate the sequence header, then build
** this data structure for the actual output using function seqWriteSeq
**
** @attr linepos [ajint] Undocumented
** @attr namewidth [ajint] Name format width
** @attr numline [ajint] Undocumented
** @attr numwidth [ajint] Number format width
** @attr spacer [ajint] Undocumented
** @attr tab [ajint] Undocumented
** @attr width [ajint] Number of bases per line
** @attr baseonlynum [AjBool] Undocumented
** @attr degap [AjBool] Remove gap characters
** @attr domatch [AjBool] Show matching line
** @attr isactive [AjBool] Undocumented
** @attr nameright [AjBool] Sequence name in right margin
** @attr nameleft [AjBool] Sequence name in left margin
** @attr noleaves [AjBool] Undocumented
** @attr numjust [AjBool] Justify numbers
** @attr numleft [AjBool] Base number on left
** @attr numright [AjBool] Base number on right
** @attr pretty [AjBool] Undocumented
** @attr skipafter [AjBool] Undocumented
** @attr skipbefore [AjBool] Undocumented
** @attr gapchar [char] gap character
** @attr matchchar [char] matching character
** @attr endstr [char[20]] Last line(s)
** @attr leftstr [char[20]] string in left margin
** @@
******************************************************************************/

typedef struct SeqSSeqFormat
{
    ajint linepos;
    ajint namewidth;
    ajint numline;
    ajint numwidth;
    ajint spacer;
    ajint tab;
    ajint width;
    AjBool baseonlynum;
    AjBool degap;
    AjBool domatch;
    AjBool isactive;
    AjBool nameright;
    AjBool nameleft;
    AjBool noleaves;
    AjBool numjust;
    AjBool numleft;
    AjBool numright;
    AjBool pretty;
    AjBool skipafter;
    AjBool skipbefore;
    char gapchar;
    char matchchar;
    char endstr[20];
    char leftstr[20];

    /* obsolete
       AjBool numtop;
       AjBool numbot;
       AjBool nametop;
       
       char padding[2];
       ajint interline;
       */

} SeqOSeqFormat, *SeqPSeqFormat;

static ajint seqSpaceAll = -9;


static void       seqAllClone (AjPSeqout outseq, AjPSeq seq);
static void       seqClone (AjPSeqout outseq, AjPSeq seq);
static void       seqDbName (AjPStr* name, AjPStr db);
static void       seqDeclone (AjPSeqout outseq);
static void       seqDefName (AjPStr* name, AjBool multi);
static AjBool     seqFileReopen (AjPSeqout outseq);
static AjBool     seqoutUfoLocal (AjPSeqout thys);
static AjBool     seqoutUsaProcess (AjPSeqout thys);
static void       seqsetClone (AjPSeqout outseq, AjPSeqset seq, ajint i);

static void       seqSeqFormat (ajint seqlen, SeqPSeqFormat* psf);
static void       seqWriteAcedb (AjPSeqout outseq);
static void       seqWriteAsn1 (AjPSeqout outseq);
static void       seqWriteClustal (AjPSeqout outseq);
static void       seqWriteCodata (AjPSeqout outseq);
static void       seqWriteDebug (AjPSeqout outseq);
static void       seqWriteEmbl (AjPSeqout outseq);
static void       seqWriteFasta (AjPSeqout outseq);
static void       seqWriteFitch (AjPSeqout outseq);
static void       seqWriteGcg (AjPSeqout outseq);
static void       seqWriteGenbank (AjPSeqout outseq);
static void       seqWriteGff (AjPSeqout outseq);
static void       seqWriteHennig86 (AjPSeqout outseq);
static void       seqWriteIg (AjPSeqout outseq);
static void       seqWriteJackknifer (AjPSeqout outseq);
static void       seqWriteJackknifernon (AjPSeqout outseq);
static void       seqWriteListAppend (AjPSeqout outseq, AjPSeq seq);
static void       seqWriteMega (AjPSeqout outseq);
static void       seqWriteMeganon (AjPSeqout outseq);
static void       seqWriteMsf (AjPSeqout outseq);
static void       seqWriteNbrf (AjPSeqout outseq);
static void       seqWriteNcbi (AjPSeqout outseq);
static void       seqWriteNexus (AjPSeqout outseq);
static void       seqWriteNexusnon (AjPSeqout outseq);
static void       seqWritePhylip (AjPSeqout outseq);
static void       seqWritePhylip3 (AjPSeqout outseq);
static void       seqWriteSelex (AjPSeqout outseq);
static void       seqWriteSeq (AjPSeqout outseq, SeqPSeqFormat sf);
static void       seqWriteStaden (AjPSeqout outseq);
static void       seqWriteStrider (AjPSeqout outseq);
static void       seqWriteSwiss (AjPSeqout outseq);
static void       seqWriteText (AjPSeqout outseq);
static void       seqWriteTreecon (AjPSeqout outseq);

/* @funclist seqOutFormat *****************************************************
**
** Functions to write each sequence format
**
******************************************************************************/

static SeqOOutFormat seqOutFormat[] =
{					/* AJFALSE = write one file */
    {"unknown",    AJFALSE, AJFALSE, seqWriteFasta}, /* internal default
							writes FASTA */
    /* set 'fasta' in ajSeqOutFormatDefault */
    {"gcg",        AJFALSE, AJFALSE, seqWriteGcg},
    {"gcg8",       AJFALSE, AJFALSE, seqWriteGcg}, /* alias for gcg */
    {"embl",       AJFALSE, AJFALSE, seqWriteEmbl},
    {"em",         AJFALSE, AJFALSE, seqWriteEmbl}, /* alias for embl */
    {"swiss",      AJFALSE, AJFALSE, seqWriteSwiss},
    {"sw",         AJFALSE, AJFALSE, seqWriteSwiss}, /* alias for swiss */
    {"fasta",      AJFALSE, AJFALSE, seqWriteFasta},
    {"pearson",    AJFALSE, AJFALSE, seqWriteFasta}, /* alias for fasta */
    {"ncbi",       AJFALSE, AJFALSE, seqWriteNcbi},
    {"nbrf",       AJFALSE, AJFALSE, seqWriteNbrf},
    {"pir",        AJFALSE, AJFALSE, seqWriteNbrf}, /* alias for nbrf */
    {"genbank",    AJFALSE, AJFALSE, seqWriteGenbank},
    {"gb",         AJFALSE, AJFALSE, seqWriteGenbank}, /* alias for genbank */
    {"gff",        AJFALSE, AJFALSE, seqWriteGff},
    {"ig",         AJFALSE, AJFALSE, seqWriteIg},
    {"codata",     AJFALSE, AJFALSE, seqWriteCodata},
    {"strider",    AJFALSE, AJFALSE, seqWriteStrider},
    {"acedb",      AJFALSE, AJFALSE, seqWriteAcedb},
    {"experiment", AJFALSE, AJFALSE, seqWriteStaden},
    {"staden",     AJFALSE, AJFALSE, seqWriteStaden}, /* alias for experiment*/
    {"text",       AJFALSE, AJFALSE, seqWriteText},
    {"plain",      AJFALSE, AJFALSE, seqWriteText}, /* alias for text */
    {"raw",        AJFALSE, AJFALSE, seqWriteText}, /* alias for text output */
    {"fitch",      AJFALSE, AJFALSE, seqWriteFitch},
    {"msf",        AJFALSE, AJTRUE,  seqWriteMsf},
    {"clustal",    AJFALSE, AJTRUE,  seqWriteClustal},
    {"selex",      AJFALSE, AJTRUE,  seqWriteSelex},
    {"aln",        AJFALSE, AJTRUE,  seqWriteClustal}, /* alias for clustal */
    {"phylip",     AJFALSE, AJTRUE,  seqWritePhylip},
    {"phylip3",    AJFALSE, AJTRUE,  seqWritePhylip3},
    {"asn1",       AJFALSE, AJFALSE, seqWriteAsn1},
    {"hennig86",   AJFALSE, AJTRUE,  seqWriteHennig86},
    {"mega",       AJFALSE, AJTRUE,  seqWriteMega},
    {"meganon",    AJFALSE, AJTRUE,  seqWriteMeganon},
    {"nexus",      AJFALSE, AJTRUE,  seqWriteNexus},
    {"nexusnon",   AJFALSE, AJTRUE,  seqWriteNexusnon},
    {"paup",       AJFALSE, AJTRUE,  seqWriteNexus}, /* alias for nexus */
    {"paupnon",    AJFALSE, AJTRUE,  seqWriteNexusnon},	/* alias for nexusnon*/
    {"jackknifer", AJFALSE, AJTRUE,  seqWriteJackknifer},
    {"jackknifernon", AJFALSE, AJTRUE,  seqWriteJackknifernon},
    {"treecon",    AJFALSE, AJTRUE,  seqWriteTreecon},
    {"debug",      AJFALSE, AJFALSE, seqWriteDebug}, /* trace report */
    {NULL, 0, 0, NULL}
};


/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Stream Operators *****************************************
**
** These functions use the contents of a sequence stream object but do
** not make any changes.
**
******************************************************************************/

/* @func ajSeqAllWrite ********************************************************
**
** Write next sequence out - continue until done.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

void ajSeqAllWrite (AjPSeqout outseq, AjPSeq seq)
{
    
    ajDebug ("ajSeqAllWrite '%s' len: %d\n", ajSeqName(seq), ajSeqLen(seq));
    
    if (!outseq->Format)
    {
	if (!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format))
	{
	    ajErr ("unknown output format '%S'", outseq->Formatstr);
	}
    }
    
    ajDebug ("ajSeqAllWrite %d '%s' single: %B feat: %B Save: %B\n",
	     outseq->Format,
	     seqOutFormat[outseq->Format].Name,
	     seqOutFormat[outseq->Format].Single,
	     outseq->Features,
	     seqOutFormat[outseq->Format].Save);
    
    
    if (seqOutFormat[outseq->Format].Save)
    {
	seqWriteListAppend (outseq, seq);
	outseq->Count++;
	return;
    }
    
    seqAllClone (outseq, seq);
    
    seqDefName (&outseq->Name, !outseq->Single);
    
    if (outseq->Single)
	(void) seqFileReopen(outseq);
    
    /* Calling funclist seqOutFormat() */
    seqOutFormat[outseq->Format].Write (outseq);
    outseq->Count++;
    
    ajDebug ("ajSeqAllWrite tests features %B tabouitisopen %B "
	     "UfoLocal %B ftlocal %B\n",
	     outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
	     seqoutUfoLocal(outseq), ajFeattabOutIsLocal(outseq->Ftquery));

    if (outseq->Features &&
	!ajFeattabOutIsLocal(outseq->Ftquery)) /* not already done */
    {
	if (!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("ajSeqAllWrite features output needed\n");
	    ajFeattabOutSetBasename (outseq->Ftquery, outseq->Filename);
	    if (!ajFeattabOutOpen (outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("ajSeqAllWrite features output file open failed '%S%S'",
		       outseq->Ftquery->Directory, outseq->Ftquery->Filename);
		return;
	    }
	    (void) ajStrSet (&outseq->Ftquery->Seqname, seq->Name);
	    (void) ajStrSet (&outseq->Ftquery->Type, seq->Type);
	}
	/* ajFeattableTrace(outseq->Fttable); */
	if (!ajFeatUfoWrite (outseq->Fttable, outseq->Ftquery,
			     outseq->Ufo))
	{
	    ajWarn ("ajSeqAllWrite features output failed UFO: '%S'",
		    outseq->Ufo);
	    return;
	}
   }
    
    seqDeclone (outseq);
    
    return;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Set Operators ********************************************
**
** These functions use the contents of a sequence set object but do
** not make any changes.
**
******************************************************************************/

/* @func ajSeqsetWrite ********************************************************
**
** Write a set of sequences out.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeqset] Sequence set.
** @return [void]
** @@
******************************************************************************/

void ajSeqsetWrite (AjPSeqout outseq, AjPSeqset seq)
{

    ajint i = 0;

    ajDebug("ajSeqsetWrite\n");

    if (!outseq->Format)
    {
	if (!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format))
	{
	    ajErr ("unknown output format '%S'", outseq->Formatstr);
	}
    }

    ajDebug ("ajSeqSetWrite %d '%s' single: %B feat: %B Save: %B\n",
	     outseq->Format,
	     seqOutFormat[outseq->Format].Name,
	     seqOutFormat[outseq->Format].Single,
	     outseq->Features,
	     seqOutFormat[outseq->Format].Save);

    for (i=0; i < seq->Size; i++)
    {

	if (seqOutFormat[outseq->Format].Save)
	{
	    seqWriteListAppend (outseq, seq->Seq[i]);
	    outseq->Count++;
	    continue;
	}

	seqsetClone (outseq, seq, i);
	seqDefName (&outseq->Name, !outseq->Single);

	if (outseq->Single)
	    (void) seqFileReopen(outseq);

	/* Calling funclist seqOutFormat() */
	seqOutFormat[outseq->Format].Write (outseq);
	outseq->Count++;

	ajDebug ("ajSeqsetWrite tests features %B tabouitisopen %B "
		 "UfoLocal %B ftlocal %B\n",
		 outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
		 seqoutUfoLocal(outseq), ajFeattabOutIsLocal(outseq->Ftquery));

	if (outseq->Features &&
	    !ajFeattabOutIsLocal(outseq->Ftquery)) /* not already done */
	{
	    if (!ajFeattabOutIsOpen(outseq->Ftquery))
	    {
		ajDebug("ajSeqsetWrite features output needed\n");
		ajFeattabOutSetBasename (outseq->Ftquery, outseq->Filename);
		if (!ajFeattabOutOpen (outseq->Ftquery, outseq->Ufo))
		{
		    ajWarn ("ajSeqsetWrite features output "
			    "failed to open UFO '%S'",
			    outseq->Ufo);
		    return;
		}
		(void) ajStrSet (&outseq->Ftquery->Seqname, seq->Name);
		(void) ajStrSet (&outseq->Ftquery->Type, seq->Type);
	    }
	    /* ajFeattableTrace(outseq->Fttable); */
	    if (!ajFeatUfoWrite (outseq->Fttable,
				 outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn ("ajSeqsetWrite features output failed UFO: '%S'",
			outseq->Ufo);
		return;
	    }
	}
	
	seqDeclone (outseq);
    }
    
    return;
}

/* @funcstatic seqWriteListAppend *********************************************
**
** Add the latest sequence to the output list. If we are in single
** sequence mode, also write it out now though it does not seem
** a great idea in most cases to ask for this.
**
** @param [P] outseq [AjPSeqout] Sequence output
** @param [P] seq [AjPSeq] Sequence to be appended
** @return [void]
** @@
******************************************************************************/

static void seqWriteListAppend (AjPSeqout outseq, AjPSeq seq)
{

    AjPSeq listseq;

    ajDebug ("seqWriteListAppend '%F' %S\n", outseq->File, ajSeqGetName(seq));

    if (!outseq->Savelist)
	outseq->Savelist = ajListNew();

    listseq = ajSeqNewS(seq);
    ajSeqTrim (listseq);
    /* if (listseq->Rev)
	ajSeqReverse(listseq); */ /* already done */

    seqDefName (&listseq->Name, !outseq->Single);

    ajListPushApp(outseq->Savelist, listseq);

    if (outseq->Single)
    {
	ajDebug("single sequence mode: write immediately\n");
	seqDefName (&outseq->Name, !outseq->Single);
	/* Calling funclist seqOutFormat() */
	seqOutFormat[outseq->Format].Write (outseq);
    }

    if (outseq->Features &&
	!ajFeattabOutIsLocal(outseq->Ftquery))
    {
	seqClone (outseq, seq); /* need to clone feature table) */
	if (!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("seqWriteListAppend features output needed table\n");
	    
	    ajFeattabOutSetBasename (outseq->Ftquery, outseq->Filename);
	    if (!ajFeattabOutOpen (outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn ("seqWriteListAppend features output "
			"failed to open UFO '%S'",
			outseq->Ufo);
		return;
	    }
	    (void) ajStrSet (&outseq->Ftquery->Seqname, seq->Name);
	    (void) ajStrSet (&outseq->Ftquery->Type, seq->Type);
	}
	/* ajFeattableTrace(outseq->Fttable); */
	if (!ajFeatUfoWrite (outseq->Fttable, outseq->Ftquery, outseq->Ufo))
	{
	    ajWarn ("seqWriteListAppend features output failed UFO: '%S'",
		    outseq->Ufo);
	    return;
	}
	    
	seqDeclone (outseq);
    }
    
    return;
}

/* @func ajSeqWriteClose ******************************************************
**
** Close a sequence output file. For formats that save everything up
** and write at the end, call the Write function first.
**
** @param [P] outseq [AjPSeqout] Sequence output
** @return [void]
** @@
******************************************************************************/

void ajSeqWriteClose (AjPSeqout outseq)
{

    ajDebug ("ajSeqWriteClose '%F'\n", outseq->File);

    if (seqOutFormat[outseq->Format].Save)
    {
	/* Calling funclist seqOutFormat() */
	seqOutFormat[outseq->Format].Write (outseq);
    }

    if (outseq->Knownfile)
	outseq->File = NULL;
    else
	ajFileClose (&outseq->File);

    return;
}

/* @func ajSeqWrite ***********************************************************
**
** Write a sequence out. For formats that save everything up
** and write at the end, just append to the output list.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @param [P] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqWrite (AjPSeqout outseq, AjPSeq seq)
{
    
    if (!outseq->Format)
    {
	if (!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format))
	{
	    ajErr ("unknown output format '%S'", outseq->Formatstr);
	}
    }
    
    ajDebug ("ajSeqWrite %d '%s' single: %B feat: %B Save: %B\n",
	     outseq->Format,
	     seqOutFormat[outseq->Format].Name,
	     seqOutFormat[outseq->Format].Single,
	     outseq->Features,
	     seqOutFormat[outseq->Format].Save);
    
    if (seqOutFormat[outseq->Format].Save)
    {
	seqWriteListAppend (outseq, seq);
	outseq->Count++;
	return;
    }
    
    seqClone (outseq, seq);
    
    seqDefName (&outseq->Name, !outseq->Single);
    
    if (outseq->Single)
	(void) seqFileReopen(outseq);
    
    /* Calling funclist seqOutFormat() */
    seqOutFormat[outseq->Format].Write (outseq);
    outseq->Count++;

    ajDebug ("ajSeqWrite tests features %B tabouitisopen %B UfoLocal %B\n",
	     outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
	     seqoutUfoLocal(outseq));
    if (outseq->Features && 
	!ajFeattabOutIsLocal(outseq->Ftquery))
    {
	if (!ajFeattabOutIsOpen(outseq->Ftquery))
    {
	ajDebug("ajSeqWrite features output needed\n");
	ajFeattabOutSetBasename (outseq->Ftquery, outseq->Filename);
	if (!ajFeattabOutOpen (outseq->Ftquery, outseq->Ufo))
	{
	    ajWarn ("ajSeqWrite features output failed to open UFO '%S'",
		    outseq->Ufo);
	    return;
	}
	(void) ajStrSet (&outseq->Ftquery->Seqname, seq->Name);
	(void) ajStrSet (&outseq->Ftquery->Type, seq->Type);
	/* ajFeattableTrace(outseq->Fttable); */
	if (!ajFeatUfoWrite (outseq->Fttable,
			     outseq->Ftquery, outseq->Ufo))
	{
	    ajWarn ("ajSeqWrite features output failed UFO: '%S'",
		    outseq->Ufo);
	    return;
	}
     }
   }

    seqDeclone (outseq);
    
    return;
}

/* @funcstatic seqWriteFasta **************************************************
**
** Writes a sequence in FASTA format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteFasta (AjPSeqout outseq)
{
    ajint i;
    ajint ilen;
    static AjPStr seq = NULL;
    ajint linelen = 60;
    ajint iend;

    seqDbName (&outseq->Name, outseq->Setdb);

    (void) ajFmtPrintF (outseq->File, ">%S", outseq->Name);

    if (ajStrLen(outseq->Sv))
	(void) ajFmtPrintF (outseq->File, " %S", outseq->Sv);
    else if (ajStrLen(outseq->Acc))
	(void) ajFmtPrintF (outseq->File, " %S", outseq->Acc);

    /* no need to bother with outseq->Gi because we have Sv anyway */

    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, " %S", outseq->Desc);
    (void) ajFmtPrintF (outseq->File, "\n");

    ilen = ajStrLen(outseq->Seq);
    for (i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	(void) ajStrAssSub (&seq, outseq->Seq, i, iend);
	(void) ajFmtPrintF (outseq->File, "%S\n", seq);
    }

    return;
}

/* @funcstatic seqWriteNcbi ***************************************************
**
** Writes a sequence in NCBI format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNcbi (AjPSeqout outseq)
{

    ajint i;
    ajint ilen;
    static AjPStr seq = NULL;
    ajint linelen = 60;
    ajint iend;

    if (ajStrLen(outseq->Gi))
	(void) ajFmtPrintF (outseq->File, ">gi|%S|gnl|", outseq->Gi);
    else
	(void) ajFmtPrintF (outseq->File, ">gnl|");
    if (ajStrLen(outseq->Db))
	(void) ajFmtPrintF (outseq->File, "%S|", outseq->Db);
    else if (ajStrLen(outseq->Setdb))
	(void) ajFmtPrintF (outseq->File, "%S|", outseq->Setdb);
    else
	(void) ajFmtPrintF (outseq->File, "unk|");

    (void) ajFmtPrintF (outseq->File, "%S", outseq->Name);

    if (ajStrLen(outseq->Sv))
	(void) ajFmtPrintF (outseq->File, " (%S)", outseq->Sv);
    else if (ajStrLen(outseq->Acc))
	(void) ajFmtPrintF (outseq->File, " (%S)", outseq->Acc);

    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, " %S", outseq->Desc);

    (void) ajFmtPrintF (outseq->File, "\n");

    ilen = ajStrLen(outseq->Seq);
    for (i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	(void) ajStrAssSub (&seq, outseq->Seq, i, iend);
	(void) ajFmtPrintF (outseq->File, "%S\n", seq);
    }

    return;
}

/* @funcstatic seqWriteGcg ****************************************************
**
** Writes a sequence in GCG format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGcg (AjPSeqout outseq)
{

    ajint ilen = ajStrLen(outseq->Seq);
    char ctype = 'N';
    ajint check;
    SeqPSeqFormat sf = NULL;

    if (!outseq->Type)
	(void) ajFmtPrintF (outseq->File, "!!NA_SEQUENCE 1.0\n\n");
    else if (ajStrMatchC(outseq->Type, "P"))
    {
	(void) ajFmtPrintF (outseq->File, "!!AA_SEQUENCE 1.0\n\n");
	ctype = 'P';
    }
    else
	(void) ajFmtPrintF (outseq->File, "!!NA_SEQUENCE 1.0\n\n");

    ajSeqGapS (&outseq->Seq, '.');
    check = ajSeqoutCheckGcg (outseq);

    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, "%S\n\n", outseq->Desc);

    (void) ajFmtPrintF (outseq->File,
			"%S  Length: %d  Type: %c  Check: %4d ..\n",
			outseq->Name, ilen, ctype, check);

    if (sf)
	seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    else
    {
	seqSeqFormat(ajStrLen(outseq->Seq), &sf);
	sf->spacer = 11;
	sf->numleft = ajTrue;
	sf->skipbefore = ajTrue;
	(void) strcpy (sf->endstr, "\n"); /* to help with misreads at EOF */
    }

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteStaden *************************************************
**
** Writes a sequence in Staden format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteStaden (AjPSeqout outseq)
{

    static SeqPSeqFormat sf=NULL;

    ajFmtPrintF (outseq->File, "<%S---->\n", outseq->Name);
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);

    sf->width = 60;
    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteText ***************************************************
**
** Writes a sequence in plain Text format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteText (AjPSeqout outseq)
{
    static SeqPSeqFormat sf=NULL;

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteHennig86 ***********************************************
**
** Writes a sequence in Hennig86 format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteHennig86 (AjPSeqout outseq)
{
    
    /*  static SeqPSeqFormat sf=NULL;*/
    
    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    char* cp;
    
    ajDebug ("seqWriteHennig86 list size %d\n",
	     ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if (!isize) return;
    
    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++) {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }
    
    (void) ajFmtPrintF (outseq->File,	/* header text */
			"xread\n");
    
    (void) ajFmtPrintF (outseq->File,	/* title text */
			"' Written by EMBOSS %D '\n", ajTimeToday());
    
    (void) ajFmtPrintF (outseq->File,	/* length, count */
			"%d %d\n", ilen, isize);
    
    for (i=0; i < isize; i++) {	/* loop over sequences */
	seq = seqarr[i];
	(void) ajStrAss(&sseq, seq->Seq);
	
	cp = ajStrStr(sseq);
	while (*cp)
	{
	    switch (*cp)
	    {
	    case 'A':
	    case 'a':
		*cp = '0';
		break;
	    case 'T':
	    case 't':
	    case 'U':
	    case 'u':
		*cp = '1';
		break;
	    case 'G':
	    case 'g':
		*cp = '2';
		break;
	    case 'C':
	    case 'c':
		*cp = '3';
		break;
	    default:
		*cp = '?';
		break;
	    }
	    cp++;
	}
	(void) ajFmtPrintF (outseq->File,
			    "%S\n%S\n",
			    seq->Name, sseq);
    }
    
    (void) ajFmtPrintF (outseq->File,	/* terminate with ';' */
			";\n", ilen, isize);
    
    return;
}

/* @funcstatic seqWriteMega ***************************************************
**
** Writes a sequence in Mega format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMega (AjPSeqout outseq)
{

    /*  static SeqPSeqFormat sf=NULL;*/

    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;

    ajDebug ("seqWriteMega list size %d\n", ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if (!isize) return;

    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }

    (void) ajFmtPrintF (outseq->File,	/* header text */
			"#mega\n");
    (void) ajFmtPrintF (outseq->File,	/* dummy title */
			"TITLE: Written by EMBOSS %D\n", ajTimeToday());

    for (ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos + wid -1;
	if (iend > ilen)
	    iend = ilen;

	(void) ajFmtPrintF (outseq->File, /* blank space for comments */
			    "\n");
	for (i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    (void) ajFmtPrintF (outseq->File,
				"#%-20.20S %S\n",
				seq->Name, sseq);
	}
    }

    return;
}

/* @funcstatic seqWriteMeganon ************************************************
**
** Writes a sequence in Mega non-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMeganon (AjPSeqout outseq)
{

    /*  static SeqPSeqFormat sf=NULL;*/

    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;

    ajDebug ("seqWriteMeganon list size %d\n", ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if (!isize) return;

    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }

    (void) ajFmtPrintF (outseq->File,	/* header text */
			"#mega\n");
    (void) ajFmtPrintF (outseq->File,	/* dummy title */
			"TITLE: Written by EMBOSS %D\n", ajTimeToday());
    (void) ajFmtPrintF (outseq->File,	/* blank space for comments */
			"\n");

    for (i=0; i < isize; i++)
    {					/* loop over sequences */
	seq = seqarr[i];
	(void) ajStrAss(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	(void) ajFmtPrintF (outseq->File,
			    "#%-20.20S\n%S\n",
			    seq->Name, sseq);
    }

    return;
}

/* @funcstatic seqWriteNexus **************************************************
**
** Writes a sequence in Nexus interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNexus (AjPSeqout outseq)
{
    
    /*  static SeqPSeqFormat sf=NULL;*/
    
    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;
    
    ajDebug ("seqWriteNexus list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if (!isize) return;
    
    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }
    
    (void) ajFmtPrintF (outseq->File,	/* header text */
			"#NEXUS\n");
    (void) ajFmtPrintF (outseq->File,	/* dummy title */
			"[TITLE: Written by EMBOSS %D]\n\n", ajTimeToday());
    (void) ajFmtPrintF (outseq->File,
			"begin data;\n");
    (void) ajFmtPrintF (outseq->File,	/* count, length */
			"dimensions ntax=%d nchar=%d;\n", isize, ilen);
    (void) ajFmtPrintF (outseq->File,
			"format interleave datatype=DNA missing=N gap=-;\n");
    (void) ajFmtPrintF (outseq->File, "\n");
    
    (void) ajFmtPrintF (outseq->File,
			"matrix\n");
    for (ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos +wid -1;
	if (iend > ilen)
	    iend = ilen;

	if (ipos > 1)
	    (void) ajFmtPrintF (outseq->File, "\n");

	for (i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    (void) ajFmtPrintF (outseq->File,
				"%-20.20S %S\n",
				seq->Name, sseq);
	}
    }
    
    (void) ajFmtPrintF (outseq->File,
			";\n\n");
    (void) ajFmtPrintF (outseq->File,
			"endblock;\n");
    (void) ajFmtPrintF (outseq->File,
			"begin assumptions;\n");
    (void) ajFmtPrintF (outseq->File,
			"options deftype=unord;\n");
    return;
}

/* @funcstatic seqWriteNexusnon ***********************************************
**
** Writes a sequence in Nexus non-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNexusnon (AjPSeqout outseq)
{

    /*  static SeqPSeqFormat sf=NULL;*/

    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;

    ajDebug ("seqWriteNexusnon list size %d\n",
	     ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if (!isize) return;

    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }

    (void) ajFmtPrintF (outseq->File,	/* header text */
			"#NEXUS\n");
    (void) ajFmtPrintF (outseq->File,	/* dummy title */
			"[TITLE: Written by EMBOSS %D]\n\n", ajTimeToday());
    (void) ajFmtPrintF (outseq->File,
			"begin data;\n");
    (void) ajFmtPrintF (outseq->File,	/* count, length */
			"dimensions ntax=%d nchar=%d;\n", isize, ilen);
    (void) ajFmtPrintF (outseq->File,
			"format datatype=DNA missing=N gap=-;\n");
    (void) ajFmtPrintF (outseq->File, "\n");

    (void) ajFmtPrintF (outseq->File,
			"matrix\n");
    for (i=0; i < isize; i++)
    {					/* loop over sequences */
	seq = seqarr[i];
	(void) ajStrAss(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	(void) ajFmtPrintF (outseq->File,
			    "%S\n%S\n",
			    seq->Name, sseq);
    }

    (void) ajFmtPrintF (outseq->File,
			";\n\n");
    (void) ajFmtPrintF (outseq->File,
			"endblock;\n");
    (void) ajFmtPrintF (outseq->File,
			"begin assumptions;\n");
    (void) ajFmtPrintF (outseq->File,
			"options deftype=unord;\n");
    return;
}

/* @funcstatic seqWriteJackknifer *********************************************
**
** Writes a sequence in Jackknifer format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteJackknifer (AjPSeqout outseq)
{

    /*  static SeqPSeqFormat sf=NULL;*/

    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;
    static AjPStr tmpid = NULL;

    ajDebug ("seqWriteJackknifer list size %d\n",
	     ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if (!isize) return;

    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }

    (void) ajFmtPrintF (outseq->File,	/* header text */
			"' Written by EMBOSS %D \n", ajTimeToday());

    for (ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos +wid -1;
	if (iend > ilen)
	    iend = ilen;

	for (i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajFmtPrintS (&tmpid, "(%S)", seq->Name);
	    (void) ajFmtPrintF (outseq->File,
				"%-20.20S %S\n",
				tmpid, sseq);
	}
    }

    (void) ajFmtPrintF (outseq->File, ";\n");

    return;
}

/* @funcstatic seqWriteJackknifernon ******************************************
**
** Writes a sequence in Jackknifer on-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteJackknifernon (AjPSeqout outseq)
{

    /*  static SeqPSeqFormat sf=NULL;*/

    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;
    static AjPStr tmpid = NULL;

    ajDebug ("seqWriteJackknifernon list size %d\n",
	     ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if (!isize) return;

    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }

    (void) ajFmtPrintF (outseq->File,	/* header text */
			"' Written by EMBOSS %D \n", ajTimeToday());

    for (i=0; i < isize; i++)
    {					/* loop over sequences */
	seq = seqarr[i];
	for (ipos=1; ipos <= ilen; ipos += wid)
	{				/* interleaved */
	    iend = ipos +wid -1;
	    if (iend > ilen)
		iend = ilen;

	    (void) ajStrAssSub (&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    if (ipos == 1)
	    {
		ajFmtPrintS (&tmpid, "(%S)", seq->Name);
		(void) ajFmtPrintF (outseq->File,
				    "%-20.20S %S\n",
				    tmpid, sseq);
	    }
	    else
		(void) ajFmtPrintF (outseq->File,
				    "%S\n",
				    sseq);
	}
    }

    (void) ajFmtPrintF (outseq->File, ";\n");

    return;
}

/* @funcstatic seqWriteTreecon ************************************************
**
** Writes a sequence in Treecon format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteTreecon (AjPSeqout outseq)
{

    /*  static SeqPSeqFormat sf=NULL;*/

    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;

    ajDebug ("seqWriteTreecon list size %d\n", ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if (!isize) return;

    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }

    (void) ajFmtPrintF (outseq->File,	/* count */
			"%d\n", ilen);

    for (i=0; i < isize; i++)
    {					/* loop over sequences */
	seq = seqarr[i];
	(void) ajStrAss(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	(void) ajFmtPrintF (outseq->File,
			    "%S\n%S\n",
			    seq->Name, sseq);
    }

    return;
}

/* @funcstatic seqWriteClustal ************************************************
**
** Writes a sequence in Clustal (ALN) format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteClustal (AjPSeqout outseq)
{
    
    /*  static SeqPSeqFormat sf=NULL;*/
    
    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    
    ajDebug ("seqWriteClustal list size %d\n", ajListLength(outseq->Savelist));
    
    
    isize = ajListLength(outseq->Savelist);
    if (!isize) return;
    
    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }
    
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen > ajSeqLen(seq))
	    ajSeqFill(seq, ilen);
    }
    
    (void) ajFmtPrintF (outseq->File,
			"CLUSTAL W(1.4) multiple sequence alignment\n");
    
    (void) ajFmtPrintF (outseq->File, "\n\n");
    
    for (ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;
	if (iend > ilen)
	    iend = ilen;

	for (i=0; i < isize; i++)
	{
	    seq = seqarr[i];
	    (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    (void) ajStrBlock (&sseq, 10);
	    (void) ajFmtPrintF (outseq->File,
				"%-15.15S %S\n",
				seq->Name, sseq);
	}
	(void) ajFmtPrintF (outseq->File, /* *. conserved line */
			    "%-15.15s %54.54s\n", "", "");
	(void) ajFmtPrintF (outseq->File, "\n"); /* blank line */
    }
    
    return;
}


/* @funcstatic seqWriteSelex **************************************************
**
** Writes a sequence in Selex format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSelex (AjPSeqout outseq)
{
    ajint n;
    ajint len=0;
    ajint i=0;
    ajint j=0;
    ajint k=0;
    
    AjPSeq seq=NULL;
    AjPSeq* seqs=NULL;
    ajint test;
    AjPSelexdata sdata=NULL;
    AjPSelexSQ qdata=NULL;
    ajint namelen=0;
    ajint v=0;
    AjBool sep=ajFalse;
    AjPStr rfstr=NULL;
    AjPStr csstr=NULL;
    AjPStr ssstr=NULL;
    char *p=NULL;
    AjPStr *names;
    ajint  extra;
    ajint  nlen=0;
    ajint  slen=0;
    AjPStr *aseqs=NULL;
    
    ajDebug ("seqWriteSelex list size %d\n", ajListLength(outseq->Savelist));
    
    rfstr = ajStrNewC("#=RF");
    csstr = ajStrNewC("#=CS");
    ssstr = ajStrNewC("#=SS");
    
    n = ajListLength(outseq->Savelist);
    if (!n)
	return;
    
    test = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", test);
    
    
    
    for (i=0; i < n; ++i)
    {
	seq = seqs[i];
	if (len < ajSeqLen(seq))
	    len = ajSeqLen(seq);
    }
    
    sdata = seqs[0]->Selexdata;
    if(sdata)
    {
	
	if(ajStrLen(sdata->id))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=ID %S\n",sdata->id);
	}
	if(ajStrLen(sdata->ac))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=AC %S\n",sdata->ac);
	}
	if(ajStrLen(sdata->de))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=DE %S\n",sdata->de);
	}
	
	if(sdata->ga[0] || sdata->ga[1])
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=GA %.2f %.2f\n",sdata->ga[0],
			sdata->ga[1]);
	}
	
	if(sdata->tc[0] || sdata->tc[1])
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=TC %.2f %.2f\n",sdata->tc[0],
			sdata->tc[1]);
	}
	
	if(sdata->nc[0] || sdata->nc[1])
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=NC %.2f %.2f\n",sdata->nc[0],
			sdata->nc[1]);
	}
	
	if(ajStrLen(sdata->au))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=AU %S\n",sdata->au);
	}
	
	if(sep)
	    ajFmtPrintF(outseq->File,"\n");
	
	
	v=4;
	for(i=0;i<n;++i)
	{
	    v = ajStrLen(seqs[i]->Selexdata->sq->name);
	    namelen = (namelen > v) ? namelen : v;
	}
	for(i=0;i<n;++i)
	{
	    v = namelen - ajStrLen(seqs[i]->Selexdata->sq->name);
	    for(j=0;j<v;++j)
		ajStrAppK(&seqs[i]->Selexdata->sq->name,' ');
	}
	
	
	if(ajStrLen(sdata->sq->ac))
	{
	    for(i=0;i<n;++i)
	    {
		qdata = seqs[i]->Selexdata->sq;
		ajFmtPrintF(outseq->File,"#=SQ %S %.2f %S %S %d..%d:%d %S\n",
			    qdata->name,qdata->wt,qdata->source,qdata->ac,
			    qdata->start,qdata->stop,qdata->len,qdata->de);
	    }
	}
	ajFmtPrintF(outseq->File,"\n");
	
	
	
	if(ajStrLen(seqs[0]->Selexdata->rf))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppK(&rfstr,' ');
	}
	if(ajStrLen(seqs[0]->Selexdata->cs))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppK(&csstr,' ');
	}
	if(ajStrLen(seqs[0]->Selexdata->ss))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppK(&ssstr,' ');
	}
	
	
	
	for(i=0;i<len;i+=50)
	{
	    if(ajStrLen(seqs[0]->Selexdata->rf))
	    {
		p = ajStrStr(seqs[0]->Selexdata->rf);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",rfstr,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",rfstr,
				&p[i]);
	    }

	    if(ajStrLen(seqs[0]->Selexdata->cs))
	    {
		p = ajStrStr(seqs[0]->Selexdata->cs);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",csstr,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",csstr,
				&p[i]);
	    }


	    for(j=0;j<n;++j)
	    {
		sdata = seqs[j]->Selexdata;

		p = ajStrStr(sdata->str);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",sdata->sq->name,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",sdata->sq->name,
				&p[i]);
		if(ajStrLen(seqs[0]->Selexdata->ss))
		{
		    p = ajStrStr(seqs[0]->Selexdata->ss);
		    if(i+50>=len)
			ajFmtPrintF(outseq->File,"%S %s\n",ssstr,&p[i]);
		    else
			ajFmtPrintF(outseq->File,"%S %-50.50s\n",ssstr,
				    &p[i]);
		}

	    }
	    if(i+50<len)
		ajFmtPrintF(outseq->File,"\n");

	}
    }
    else	/* Wasn't originally Selex format */
    {
	AJCNEW0(aseqs,n);
	AJCNEW0(names,n);
	for (i=0; i < n; ++i)
	{
	    seq = seqs[i];
	    aseqs[i] = ajStrNew();
	    names[i] = ajStrNew();
	    ajStrAssS(&names[i],seq->Name);
	    if((len=ajStrLen(names[i])) > nlen)
		nlen = len;
	    if((len=ajStrLen(seq->Seq)) > slen)
		slen = len;
	    ajStrAssS(&aseqs[i],seq->Seq);
	}
	for(i=0;i<n;++i)
	{
	    seq = seqs[i];
	    extra = nlen - ajStrLen(names[i]);
	    for(j=0;j<extra;++j)
		ajStrAppK(&names[i],' ');
	    extra = slen - ajStrLen(seq->Seq);
	    for(j=0;j<extra;++j)
		ajStrAppK(&aseqs[i],' ');

	    ajFmtPrintF(outseq->File,"#=SQ %S %.2f - - 0..0:0 ",
			names[i],seq->Weight);
	    if(ajStrLen(seq->Desc))
		ajFmtPrintF(outseq->File,"%S\n",seq->Desc);
	    else
		ajFmtPrintF(outseq->File,"-\n");
	}
	ajFmtPrintF(outseq->File,"\n");


	for(i=0;i<slen;i+=50)
	{
	    for(j=0;j<n;++j)
	    {
		p = ajStrStr(aseqs[j]);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",names[j],&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",names[j],
				&p[i]);
	    }
	    if(i+50<len)
		ajFmtPrintF(outseq->File,"\n");

	}

	for(i=0;i<n;++i)
	{
	    ajStrDel(&names[i]);
	    ajStrDel(&aseqs[i]);
	}
	AJFREE(names);
	AJFREE(aseqs);
    }
    
    AJFREE(seqs);
    
    ajStrDel(&rfstr);
    ajStrDel(&csstr);
    ajStrDel(&ssstr);
    
    return;
}

/* @funcstatic seqWriteMsf ****************************************************
**
** Writes a sequence in GCG Multiple Sequence File format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMsf (AjPSeqout outseq)
{
    
    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint checktot = 0;
    ajint check;
    ajint itest;
    static AjPStr sbeg = NULL;
    static AjPStr send = NULL;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint igap;
    
    ajDebug ("seqWriteMsf list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if (!isize) return;
    
    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }
    
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	ajSeqGapLen (seq, '.', '~', ilen); /* need to pad if any are shorter */
	check = ajSeqCheckGcg(seq);
	ajDebug (" '%S' len: %d checksum: %d\n",
		 ajSeqGetName(seq), ajSeqLen(seq), check);
	checktot += check;
	checktot = checktot % 10000;
    }
    
    ajDebug ("checksum %d\n", checktot);
    ajDebug ("outseq->Type '%S'\n", outseq->Type);
    
    if (!ajStrLen(outseq->Type))
    {
	ajSeqType (seqarr[0]);
	(void) ajStrSet(&outseq->Type, seqarr[0]->Type);
    }
    ajDebug ("outseq->Type '%S'\n", outseq->Type);
    
    if (ajStrMatchC(outseq->Type, "P"))
    {
	(void) ajFmtPrintF (outseq->File, "!!AA_MULTIPLE_ALIGNMENT 1.0\n\n");
	(void) ajFmtPrintF (outseq->File,
			    "  %F MSF:  %d Type: P %D CompCheck: %4d ..\n\n",
			    outseq->File, ilen, ajTimeToday(), checktot);
    }
    else
    {
	(void) ajFmtPrintF (outseq->File, "!!NA_MULTIPLE_ALIGNMENT 1.0\n\n");
	(void) ajFmtPrintF (outseq->File,
			    "  %F MSF: %d Type: N %D CompCheck: %4d ..\n\n",
			    outseq->File, ilen, ajTimeToday(), checktot);
    }
    
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	check = ajSeqCheckGcg(seq);
	(void) ajFmtPrintF (outseq->File,
			    "  Name: %S Len: %d  Check: %4d Weight: %.2f\n",
			    seq->Name, ajStrLen(seq->Seq),
			    check, seq->Weight);
    }
    
    (void) ajFmtPrintF (outseq->File, "\n//\n\n");
    
    for (ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;
	if (iend > ilen)
	    iend = ilen;
	(void) ajFmtPrintS (&sbeg, "%d", ipos);
	(void) ajFmtPrintS (&send, "%d", iend);
	if (iend == ilen) {
	    igap = iend - ipos - ajStrLen(sbeg);
	    ajDebug ("sbeg: %S send: %S ipos: %d iend: %d igap: %d len: %d\n",
		     sbeg, send, ipos, iend, igap, ajStrLen(send));
	    if (igap >= ajStrLen(send))
		(void) ajFmtPrintF (outseq->File,
				    "           %S %*S\n", sbeg, igap, send);
	    else
		(void) ajFmtPrintF (outseq->File, "           %S\n", sbeg);
	}
	else
	    (void) ajFmtPrintF (outseq->File, "           %-25S%25S\n",
				sbeg, send);
	for (i=0; i < isize; i++)
	{
	    seq = seqarr[i];
	    check = ajSeqCheckGcg(seq);
	    (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
	    (void) ajFmtPrintF (outseq->File,
				"%-10S %S\n",
				seq->Name, sseq);
	}
	(void) ajFmtPrintF (outseq->File, "\n");
    }
    
    /*
       seqSeqFormat(ajStrLen(outseq->Seq), &sf);
       
       seqWriteSeq (outseq, sf);
       */
    
    /* AJB: Shouldn't this be left to ajSeqoutDel? */
    while(ajListPop(outseq->Savelist,(void **)&seq))
	ajSeqDel(&seq);
    ajListDel(&outseq->Savelist);
    
    
    AJFREE(seqs);
    
    return;
}

/* @funcstatic seqWriteCodata *************************************************
**
** Writes a sequence in Codata format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteCodata (AjPSeqout outseq)
{

    static SeqPSeqFormat sf=NULL;
    ajint j;

    (void) ajFmtPrintF (outseq->File, "ENTRY           %S \n", outseq->Name);
    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, "TITLE           %S, %d bases\n",
			    outseq->Desc, ajStrLen(outseq->Seq));
    if (ajStrLen(outseq->Acc))
	(void) ajFmtPrintF (outseq->File, "ACCESSION       %S\n",
			    outseq->Acc);
    (void) ajFmtPrintF (outseq->File, "SEQUENCE        \n");

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    sf->numwidth = 7;
    sf->width = 30;
    sf->numleft = ajTrue;
    sf->spacer = seqSpaceAll;
    (void) strcpy(sf->endstr, "\n///");

    for (j = 0; j <= sf->numwidth; j++)
	(void) ajFmtPrintF(outseq->File, " ");
    for (j = 5; j <= sf->width; j+=5)
	(void) ajFmtPrintF(outseq->File, "%10d", j);
    (void) ajFmtPrintF(outseq->File, "\n");

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteNbrf ***************************************************
**
** Writes a sequence in NBRF format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNbrf (AjPSeqout outseq)
{

    static SeqPSeqFormat sf=NULL;
    static AjPStr ftfmt = NULL;

    if (!ftfmt)
	ajStrAssC (&ftfmt, "pir");

    if (!outseq->Type)
	(void) ajFmtPrintF (outseq->File, ">D1;%S\n", outseq->Name);
    else if (ajStrMatchC(outseq->Type, "P"))
	(void) ajFmtPrintF (outseq->File, ">P1;%S\n", outseq->Name);
    else
	(void) ajFmtPrintF (outseq->File, ">D1;%S\n", outseq->Name);

    (void) ajFmtPrintF (outseq->File, "%S, %d bases\n",
			outseq->Desc, ajStrLen(outseq->Seq));

    if (seqoutUfoLocal(outseq))
    {
	outseq->Ftquery = ajFeattabOutNewSSF (ftfmt, outseq->Name,
					      ajStrStr(outseq->Type),
					      outseq->File);
	if (!ajFeatWrite (outseq->Ftquery, outseq->Fttable))
	    ajWarn ("seqWriteNbrf features output failed UFO: '%S'",
		    outseq->Ufo);
   }

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    sf->spacer = 11;
    (void) strcpy(sf->endstr, "*\n");
    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteEmbl ***************************************************
**
** Writes a sequence in EMBL format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteEmbl (AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf = NULL;
    ajint b[5];
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    
    if (!ftfmt)
	ajStrAssC (&ftfmt, "embl");
    
    if (ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss (outseq);
	return;
    }
    
    (void) ajFmtPrintF (outseq->File,
			"ID   %-10.10S standard; DNA; UNC; %d BP.\n",
			outseq->Name, ajStrLen(outseq->Seq));
    
    if (ajListLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIter(outseq->Acclist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen + ajStrLen(cur) > 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "AC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }

	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);

	}
	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ";\n", cur);
    }
    
    if (ajStrLen(outseq->Sv))
	(void) ajFmtPrintF (outseq->File, "SV   %S\n", outseq->Sv);
    
    /* no need to bother with outseq->Gi because EMBL doesn't use it */
    
    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, "DE   %S\n", outseq->Desc);
    
    if (ajListLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIter(outseq->Keylist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen+ajStrLen(cur) >= 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }
	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);
	}
	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ".\n", cur);
    }
    
    if (ajStrLen(outseq->Tax))
	(void) ajFmtPrintF (outseq->File, "OS   %S\n", outseq->Tax);
    
    if (ajListLength(outseq->Taxlist))
    {
	ilen=0;
	it = ajListIter(outseq->Taxlist);
	cur = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen+ajStrLen(cur) >= 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }
	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);
	}
	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ".\n", cur);
    }
    
    if (seqoutUfoLocal(outseq))
    {
        outseq->Ftquery = ajFeattabOutNewSSF (ftfmt, outseq->Name,
					      ajStrStr(outseq->Type),
					      outseq->File);
	if (!ajFeatWrite (outseq->Ftquery, outseq->Fttable))
	    ajWarn ("seqWriteEmbl features output failed UFO: '%S'",
		    outseq->Ufo);
    }
    
    ajSeqCount (outseq->Seq, b);
    (void) ajFmtPrintF (outseq->File,
		    "SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
			ajStrLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq (outseq, sf);
    
    return;
}

/* @funcstatic seqWriteSwiss **************************************************
**
** Writes a sequence in SWISSPROT format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSwiss (AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf=NULL;
    ajint mw;
    /*  ajuint crc; old 32-bit crc */
    unsigned long long crc;
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    
    if (!ftfmt)
	ajStrAssC (&ftfmt, "swiss");
    
    if (ajStrMatchC(outseq->Type, "N"))
    {
	seqWriteEmbl (outseq);
	return;
    }
    
    (void) ajFmtPrintF (outseq->File,
			"ID   %-10.10S     STANDARD;      PRT; %5d AA.\n",
			outseq->Name, ajStrLen(outseq->Seq));
    
    if (ajListLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIter(outseq->Acclist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen + ajStrLen(cur) > 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "AC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }

	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);

	}
	
	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ";\n", cur);
    }
    
    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, "DE   %S\n", outseq->Desc);
    
    if (ajStrLen(outseq->Tax))
	(void) ajFmtPrintF (outseq->File, "OS   %S\n", outseq->Tax);
    
    if (ajListLength(outseq->Taxlist))
    {
	ilen=0;
	it = ajListIter(outseq->Taxlist);
	cur = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen+ajStrLen(cur) >= 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }
	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);
	}
	
	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ".\n", cur);
    }
    
    if (ajListLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIter(outseq->Keylist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen+ajStrLen(cur) >= 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }
	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);
	}

	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ".\n", cur);
    }
    
    if (seqoutUfoLocal(outseq)) {

	outseq->Ftquery = ajFeattabOutNewSSF (ftfmt, outseq->Name,
					      ajStrStr(outseq->Type),
					      outseq->File);
	if (!ajFeatWrite (outseq->Ftquery, outseq->Fttable)) {
	    ajWarn ("seqWriteSwiss features output failed UFO: '%S'",
		    outseq->Ufo);
	}
    }

    /*  crc = ajSeqCrc (outseq->Seq);    old 32-bit crc*/
    crc = ajSp64Crc(outseq->Seq);
    mw = (ajint) (0.5+ajSeqMW (outseq->Seq));
    
    /* old 32-bit crc
       (void) ajFmtPrintF (outseq->File,
			   "SQ   SEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
			   ajStrLen(outseq->Seq), mw, crc);
			   */
    
    (void) ajFmtPrintF (outseq->File,
			"SQ   SEQUENCE %5d AA; %6d MW;  %08X",
			ajStrLen(outseq->Seq), mw, (crc>>32)&0xffffffff);
    (void) ajFmtPrintF (outseq->File,
			"%08X CRC64;\n",crc&0xffffffff);
    
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    
    seqWriteSeq (outseq, sf);
    
    return;
}

/* @funcstatic seqWriteGenbank ************************************************
**
** Writes a sequence in GENBANK format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGenbank (AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf=NULL;
    ajint b[5];
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    
    if (!ftfmt)
	ajStrAssC (&ftfmt, "genbank");
    
    ajSeqoutTrace (outseq);
    
    (void) ajFmtPrintF (outseq->File, "LOCUS       %S\n", outseq->Name);
    if (ajStrLen(outseq->Desc))
	(void) ajFmtPrintF (outseq->File, "DEFINITION  %S\n", outseq->Desc);
    if (ajListLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIter(outseq->Acclist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen + ajStrLen(cur) > 77)
	    {
		(void) ajFmtPrintF (outseq->File, "\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "ACCESSION   ", cur);
		ilen = 12;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, " ", cur);
		ilen += 1;
	    }

	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);

	}

	ajListIterFree(it) ;
	if (ilen > 0)
	    (void) ajFmtPrintF (outseq->File, "\n", cur);
    }
    
    if (ajStrLen(outseq->Sv))
    {
	if (ajStrLen(outseq->Gi))
	    (void) ajFmtPrintF (outseq->File, "VERSION     %S  GI:%S\n",
				outseq->Sv, outseq->Gi);
	else
	    (void) ajFmtPrintF (outseq->File, "VERSION     %S\n", outseq->Sv);
    }
    
    if (ajListLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIter(outseq->Keylist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen+ajStrLen(cur) >= 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "KEYWORDS    ", cur);
		ilen = 12;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }
	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);
	}

	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ".\n", cur);
    }
    
    if (ajStrLen(outseq->Tax) && ajListLength(outseq->Taxlist))
    {
	(void) ajFmtPrintF (outseq->File, "SOURCE      %S.\n", outseq->Tax);

	(void) ajFmtPrintF (outseq->File, "  ORGANISM  %S\n", outseq->Tax);

	ilen=0;
	it = ajListIter(outseq->Taxlist);
	cur = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    if (ilen+ajStrLen(cur) >= 77)
	    {
		(void) ajFmtPrintF (outseq->File, ";\n", cur);
		ilen = 0;
	    }
	    if (ilen == 0)
	    {
		(void) ajFmtPrintF (outseq->File, "            ", cur);
		ilen = 12;
	    }
	    else
	    {
		(void) ajFmtPrintF (outseq->File, "; ", cur);
		ilen += 2;
	    }
	    (void) ajFmtPrintF (outseq->File, "%S", cur);
	    ilen += ajStrLen(cur);
	}

	ajListIterFree(it) ;
	(void) ajFmtPrintF (outseq->File, ".\n", cur);
    }

    if (seqoutUfoLocal(outseq))
    {
        outseq->Ftquery = ajFeattabOutNewSSF (ftfmt, outseq->Name,
					      ajStrStr(outseq->Type),
					      outseq->File);
	if (!ajFeatWrite (outseq->Ftquery, outseq->Fttable))
	    ajWarn ("seqWriteGenbank features output failed UFO: '%S'",
		    outseq->Ufo);
    }
    
    ajSeqCount (outseq->Seq, b);
    if (b[4])
	(void) ajFmtPrintF (outseq->File,
			  "BASE COUNT   %6d a %6d c %6d g %6d t %6d others\n",
			    b[0], b[1], b[2], b[3], b[4]);
    else
	(void) ajFmtPrintF (outseq->File,
			    "BASE COUNT   %6d a %6d c %6d g %6d t\n",
			    b[0], b[1], b[2], b[3]);
    (void) ajFmtPrintF (outseq->File, "ORIGIN\n");
    
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "\n//");
    sf->tab = 1;
    sf->spacer = 11;
    sf->width = 60;
    sf->numleft = ajTrue;
    sf->numwidth = 8;
    
    seqWriteSeq (outseq, sf);
    
    return;
}

/* @funcstatic seqWriteGff ****************************************************
**
** Writes a sequence in GFF format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGff (AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    static AjPStr version = NULL;
    static AjPStr ftfmt = NULL;
    
    if (!ftfmt)
	ajStrAssC (&ftfmt, "gff");
    
    if (!version) ajNamRootVersion(&version);
    
    (void) ajFmtPrintF (outseq->File,
			"##gff-version 2\n");
    (void) ajFmtPrintF (outseq->File,
			"##source-version EMBOSS %S\n", version);
    (void) ajFmtPrintF (outseq->File,
			"##date %D\n", ajTimeTodayF("GFF"));
    if (ajStrMatchC(outseq->Type, "P")) {
	(void) ajFmtPrintF (outseq->File,
			    "##Protein %S\n", outseq->Name);
    }
    else {
	(void) ajFmtPrintF (outseq->File,
			    "##DNA %S\n", outseq->Name);
    }
    
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    
    strcpy (sf->leftstr, "##");
    sf->width = 60;
    /*
       sf->tab = 4;
       sf->spacer = 11;
       sf->numright = ajTrue;
       sf->numwidth = 9;
       sf->numjust = ajTrue;
       */
    
    seqWriteSeq (outseq, sf);
    
    if (ajStrMatchC(outseq->Type, "P"))
	(void) ajFmtPrintF (outseq->File, "##end-Protein\n");
    else
	(void) ajFmtPrintF (outseq->File, "##end-DNA\n");
   
    if (seqoutUfoLocal(outseq))
    {
	outseq->Ftquery = ajFeattabOutNewSSF (ftfmt, outseq->Name,
					      ajStrStr(outseq->Type),
					      outseq->File);
	if (ajStrMatchC(outseq->Type, "P"))
	    ajFeattableSetProt(outseq->Fttable);
	else
	    ajFeattableSetDna(outseq->Fttable);
	
	if (!ajFeatWrite (outseq->Ftquery, outseq->Fttable))
	    ajWarn ("seqWriteGff features output failed UFO: '%S'",
		    outseq->Ufo);

    }
    
    return;
}

/* @funcstatic seqWriteStrider ************************************************
**
** Writes a sequence in DNA STRIDER format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteStrider (AjPSeqout outseq)
{
    static SeqPSeqFormat sf=NULL;

    (void) ajFmtPrintF (outseq->File, "; ### from DNA Strider ;-)\n");
    (void) ajFmtPrintF (outseq->File, "; DNA sequence  %S, %d bases\n;\n",
			outseq->Name, ajStrLen(outseq->Seq));

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy(sf->endstr, "\n//");

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteFitch **************************************************
**
** Writes a sequence in FITCH format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteFitch (AjPSeqout outseq)
{

    static SeqPSeqFormat sf=NULL;

    (void) ajFmtPrintF (outseq->File, "%S, %d bases\n",
			outseq->Name, ajStrLen(outseq->Seq));

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    sf->spacer = 4;
    sf->width = 60;

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWritePhylip *************************************************
**
** Writes a sequence in PHYLIP interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWritePhylip (AjPSeqout outseq)
{
    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    ajint j = 0;
    char *p=NULL;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    AjPStr tstr=NULL;
    
    ajDebug ("seqWritePhylip list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if (!isize) return;
    
    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }
    
    tstr = ajStrNewL(ilen+1);
    (void) ajFmtPrintF (outseq->File, " %d %d\n", isize, ilen);
    
    for (ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;
	if (iend > ilen)
	    iend = ilen;
	
	for (i=0; i < isize; i++)
	{
	    seq = seqarr[i];

	    ajStrAssC(&tstr,ajStrStr(seq->Seq));
	    p = ajStrStr(tstr);
	    for(j=ajStrLen(tstr);j<ilen;++j)
		*(p+j)='-';
	    *(p+j)='\0';
	    tstr->Len=ilen;
	    (void) ajStrAssSub(&sseq, tstr, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    (void) ajStrBlock (&sseq, 10);
	    if (ipos == 1)
		(void) ajFmtPrintF (outseq->File,
				    "%-10.10S%S\n",
				    seq->Name, sseq);
	    else
		(void) ajFmtPrintF (outseq->File,
				    "%10s%S\n",
				    " ", sseq);
	}
	if (iend < ilen)
	    (void) ajFmtPrintF (outseq->File, "\n");
    }
    
    ajStrDel(&tstr);
    return;
}

/* @funcstatic seqWritePhylip3 ************************************************
**
** Writes a sequence in PHYLIP non-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWritePhylip3 (AjPSeqout outseq)
{
    ajint isize;
    ajint ilen=0;
    ajint i = 0;
    ajint j = 0;
    ajint n = 0;
    char *p=NULL;
    void** seqs;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend=0;
    AjPStr tstr=NULL;
    
    ajDebug ("seqWritePhylip3 list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if (!isize) return;
    
    itest = ajListToArray (outseq->Savelist, (void***) &seqs);
    ajDebug ("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for (i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if (ilen < ajSeqLen(seq))
	    ilen = ajSeqLen(seq);
    }
    
    tstr = ajStrNewL(ilen+1);
    (void) ajFmtPrintF (outseq->File, "1 %d YF\n", ilen);
    
    for(n=0;n<isize;++n)
    {
	seq = seqarr[n];
	ajStrAssC(&tstr,ajStrStr(seq->Seq));
	p = ajStrStr(tstr);
	for(j=ajStrLen(tstr);j<ilen;++j)
	    *(p+j)='-';
	*(p+j)='\0';
	tstr->Len=ilen;


	for (ipos=1; ipos <= ilen; ipos += 50)
	{
	    iend = ipos + 50 -1;
	    if (iend > ilen)
		iend = ilen;

	    (void) ajStrAssSub(&sseq, tstr, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    (void) ajStrBlock (&sseq, 10);
	    if (ipos == 1)
		(void) ajFmtPrintF (outseq->File,
				    "%-10.10S%S\n",
				    seq->Name, sseq);
	    else
		(void) ajFmtPrintF (outseq->File,
				    "%10s%S\n",
				    " ", sseq);
	}
	if (iend < ilen)
	    (void) ajFmtPrintF (outseq->File, "\n");

    }
    
    ajStrDel(&tstr);
    return;
}

/* @funcstatic seqWriteAsn1 ***************************************************
**
** Writes a sequence in ASN.1 format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteAsn1 (AjPSeqout outseq)
{

    static SeqPSeqFormat sf=NULL;

    (void) ajFmtPrintF (outseq->File, "  seq {\n");
    (void) ajFmtPrintF (outseq->File, "    id { local id 1 },\n");
    (void) ajFmtPrintF (outseq->File, "    descr { title \"%S\" },\n",
			outseq->Desc);
    (void) ajFmtPrintF (outseq->File, "    inst {\n");

    if (!outseq->Type)
	(void) ajFmtPrintF (outseq->File,
			    "      repr raw, mol dna, length %d, "
			    "topology linear,\n {\n",
			    ajStrLen(outseq->Seq));
    else if (ajStrMatchC(outseq->Type, "P")) {
	(void) ajFmtPrintF (outseq->File,
			    "      repr raw, mol aa, length %d, "
			    "topology linear,\n {\n",
			    ajStrLen(outseq->Seq));
    }
    else
	(void) ajFmtPrintF (outseq->File,
			    "      repr raw, mol dna, length %d, "
			    "topology linear,\n",
			    ajStrLen(outseq->Seq));

    (void) ajFmtPrintF (outseq->File, "      seq-data\n");
    if (ajStrMatchC(outseq->Type, "P"))
	(void) ajFmtPrintF (outseq->File, "        iupacaa \"");
    else
	(void) ajFmtPrintF (outseq->File, "        iupacna \"");

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    sf->linepos = 17;
    sf->spacer = 0;
    sf->width = 78;
    sf->tab = 0;
    (void) strcpy (sf->endstr, "\"\n      } } ,");

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteIg *****************************************************
**
** Writes a sequence in INTELLIGENETICS format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteIg (AjPSeqout outseq)
{
    static SeqPSeqFormat sf=NULL;

    (void) ajFmtPrintF (outseq->File, ";%S, %d bases\n",
			outseq->Desc, ajStrLen(outseq->Seq));
    (void) ajFmtPrintF (outseq->File, "%S\n", outseq->Name);

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "1");	/* linear (DNA) */

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteAcedb **************************************************
**
** Writes a sequence in ACEDB format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteAcedb (AjPSeqout outseq)
{
    static SeqPSeqFormat sf=NULL;

    if (ajStrMatchC(outseq->Type, "P"))
	(void) ajFmtPrintF (outseq->File, "Peptide : \"%S\"\n", outseq->Name);
    else
	(void) ajFmtPrintF (outseq->File, "DNA : \"%S\"\n", outseq->Name);

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "\n");

    seqWriteSeq (outseq, sf);

    return;
}

/* @funcstatic seqWriteDebug **************************************************
**
** Writes a sequence in debug report format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteDebug (AjPSeqout outseq)
{
    static SeqPSeqFormat sf=NULL;
    AjIList it;
    AjPStr cur;
    
    (void) ajFmtPrintF (outseq->File, "Sequence output trace\n");
    (void) ajFmtPrintF (outseq->File, "=====================\n\n");
    (void) ajFmtPrintF (outseq->File, "  Name: '%S'\n", outseq->Name);
    (void) ajFmtPrintF (outseq->File, "  Accession: '%S'\n", outseq->Acc);
    if (ajListLength(outseq->Acclist))
    {
	(void) ajFmtPrintF (outseq->File, "  Acclist: (%d)",
			    ajListLength(outseq->Acclist));
	it = ajListIter(outseq->Acclist);
	while ((cur = (AjPStr) ajListIterNext(it))) {
	    (void) ajFmtPrintF (outseq->File, " %S\n", cur);
	}
	(void) ajListIterFree (it);
	(void) ajFmtPrintF (outseq->File, "\n");
    }
    
    (void) ajFmtPrintF (outseq->File, "  SeqVersion: '%S'\n", outseq->Sv);
    (void) ajFmtPrintF (outseq->File, "  GI Version: '%S'\n", outseq->Gi);
    (void) ajFmtPrintF (outseq->File, "  Description: '%S'\n", outseq->Desc);
    if (ajListLength(outseq->Keylist))
    {
	(void) ajFmtPrintF (outseq->File, "  Keywordlist: (%d)\n",
			    ajListLength(outseq->Keylist));
	it = ajListIter(outseq->Keylist);
	while ((cur = (AjPStr) ajListIterNext(it))) {
	    (void) ajFmtPrintF (outseq->File, "    '%S'\n", cur);
	}
	(void) ajListIterFree (it);
    }
    (void) ajFmtPrintF (outseq->File, "  Taxonomy: '%S'\n", outseq->Tax);
    if (ajListLength(outseq->Taxlist))
    {
	(void) ajFmtPrintF (outseq->File, "  Taxlist: (%d)\n",
			    ajListLength(outseq->Taxlist));
	it = ajListIter(outseq->Taxlist);
	while ((cur = (AjPStr) ajListIterNext(it))) {
	    (void) ajFmtPrintF (outseq->File, "    '%S'\n", cur);
	}
	(void) ajListIterFree (it);
    }
    (void) ajFmtPrintF (outseq->File, "  Type: '%S'\n", outseq->Type);
    (void) ajFmtPrintF (outseq->File, "  Database: '%S'\n", outseq->Db);
    (void) ajFmtPrintF (outseq->File, "  Full name: '%S'\n", outseq->Full);
    (void) ajFmtPrintF (outseq->File, "  Date: '%S'\n", outseq->Date);
    (void) ajFmtPrintF (outseq->File, "  Usa: '%S'\n", outseq->Usa);
    (void) ajFmtPrintF (outseq->File, "  Ufo: '%S'\n", outseq->Ufo);
    (void) ajFmtPrintF (outseq->File, "  Input format: '%S'\n",
			outseq->Informatstr);
    (void) ajFmtPrintF (outseq->File, "  Output format: '%S'\n",
			outseq->Formatstr);
    (void) ajFmtPrintF (outseq->File, "  Filename: '%S'\n", outseq->Filename);
    (void) ajFmtPrintF (outseq->File, "  Directory: '%S'\n",
			outseq->Directory);
    (void) ajFmtPrintF (outseq->File, "  Entryname: '%S'\n",
			outseq->Entryname);
    (void) ajFmtPrintF (outseq->File, "  File name: '%S'\n",
			outseq->File->Name);
    (void) ajFmtPrintF (outseq->File, "  Extension: '%S'\n",
			outseq->Extension);
    (void) ajFmtPrintF (outseq->File, "  Single: '%B'\n", outseq->Single);
    (void) ajFmtPrintF (outseq->File, "  Features: '%B'\n", outseq->Features);
    (void) ajFmtPrintF (outseq->File, "  Count: '%B'\n", outseq->Count);
    (void) ajFmtPrintF (outseq->File, "  Documentation:...\n%S\n",
			outseq->Doc);
    
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    sf->numright = ajTrue;
    sf->numleft = ajTrue;
    sf->numjust = ajTrue;
    sf->tab = 1;
    sf->spacer = 11;
    sf->width = 50;
    
    seqWriteSeq (outseq, sf);
    
    return;
}

/* @func ajSeqFileNewOut ******************************************************
**
** Opens an output file for sequence writing. 'stdout' and 'stderr' are
** special cases using standard output and standard error respectively.
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] name [AjPStr] Output filename.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqFileNewOut (AjPSeqout seqout, AjPStr name)
{

    AjBool single = seqout->Single;
    AjBool features = seqout->Features;

    if (ajStrMatchCaseC(name, "stdout"))
	single = ajFalse;
    if (ajStrMatchCaseC(name, "stderr"))
	single = ajFalse;

    if (single)
    {				     /* ok, but nothing to open yet */
	(void) ajStrSet (&seqout->Extension, seqout->Formatstr);
	return ajTrue;
    }
    else
    {
	seqout->File = ajFileNewOut(name);
	if (seqout->File)
	    return ajTrue;
    }

    if (features)
	ajWarn ("ajSeqFileNewOut features not yet implemented");

    return ajFalse;
}

/* @funcstatic seqoutUfoLocal *************************************************
**
** Tests whether a sequence output object will write features to the
** sequence output file. The alternative is to use a separate UFO.
**
** @param [u] thys [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue if the features will be written to the sequence
** @@
******************************************************************************/

static AjBool seqoutUfoLocal (AjPSeqout thys)
{
    ajDebug ("seqoutUfoLocal Features %B Ufo %d '%S'\n",
	     thys->Features, ajStrLen(thys->Ufo), thys->Ufo);

    if (thys->Features && !ajStrLen(thys->Ufo))
	return ajTrue;

    return ajFalse;
}

/* @funcstatic seqoutUsaProcess ***********************************************
**
** Converts a USA Universal Sequence Address into an open output file.
**
** First tests for format:: and sets this if it is found
**
** Then looks for file:id and opens the file.
** In this case the file position is not known and sequence reading
** will have to scan for the entry/entries we need.
**
** @param [u] thys [AjPSeqout] Sequence output definition.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqoutUsaProcess (AjPSeqout thys)
{
    static AjPRegexp fmtexp = NULL;
    static AjPRegexp idexp = NULL;
    
    AjBool fmtstat;
    AjBool regstat;
    
    static AjPStr usatest = NULL;
    
    ajDebug("seqoutUsaProcess\n");
    if (!fmtexp)
	fmtexp = ajRegCompC ("^([A-Za-z0-9]*)::?(.*)$");
    /* \1 format */
    /* \2 remainder */
    
    if (!idexp)			/* \1 is filename \3 is the qryid */
	idexp = ajRegCompC ("^(.*)$");
    
    (void) ajStrAss (&usatest, thys->Usa);
    ajDebug("output USA to test: '%S'\n\n", usatest);
    
    fmtstat = ajRegExec (fmtexp, usatest);
    ajDebug("format regexp: %B\n", fmtstat);
    
    if (fmtstat)
    {
	ajRegSubI (fmtexp, 1, &thys->Formatstr);
	(void) ajStrSetC (&thys->Formatstr, seqOutFormat[0].Name); /* default
								      unknown */
	ajRegSubI (fmtexp, 2, &usatest);
	ajDebug ("found format %S\n", thys->Formatstr);
	if (!ajSeqFindOutFormat (thys->Formatstr, &thys->Format))
	{
	    ajDebug ("unknown format '%S'\n", thys->Formatstr);
	    return ajFalse;
	}
    }
    else {
	ajDebug("no format specified in USA\n");
    }
    ajDebug ("\n");
    
    regstat = ajRegExec (idexp, usatest);
    ajDebug ("file:id regexp: %B\n", regstat);
    
    if (regstat)
    {
	ajRegSubI (idexp, 1, &thys->Filename);
	ajDebug ("found filename %S single: %B dir: '%S'\n",
		 thys->Filename, thys->Single, thys->Directory);
	if (thys->Single)
	{
	    ajDebug ("single output file per sequence, open later\n");
	}
	else
	{
	    if (thys->Knownfile)
		thys->File = thys->Knownfile;
	    else
		thys->File = ajFileNewOutD (thys->Directory, thys->Filename);
	    if (!thys->File)
	    {
		if (ajStrLen(thys->Directory))
		    ajErr ("failed to open filename '%S' in directory '%S'",
			   thys->Filename, thys->Directory);
		else
		    ajErr ("failed to open filename '%S'", thys->Filename);

		return ajFalse;
	    }
	}
    }
    else
    {
	ajDebug ("no filename specified\n");
    }
    ajDebug ("\n");
    
    return ajTrue;
}

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Output Modifiers *****************************************
**
** These functions use the contents of a sequence output object and
** update them.
**
******************************************************************************/

/* @func ajSeqoutOpen *********************************************************
**
** If the file is not yet open, calls seqoutUsaProcess to convert the USA into
** an open output file stream.
**
** Returns the results in the AjPSeqout object.
**
** @param [w] thys [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqoutOpen (AjPSeqout thys)
{

    AjBool ret = ajFalse;

    if (thys->Ftquery)
	ajDebug("ajSeqoutOpen dir '%S' qrydir '%S'\n",
		thys->Directory, thys->Ftquery->Directory);
    else
	ajDebug("ajSeqoutOpen dir '%S' (no ftquery)\n",
		thys->Directory);
    ret = seqoutUsaProcess (thys);

    if (!ret) return ajFalse;

    if (!thys->Features)
	return ret;

    (void) ajStrSet (&thys->Ftquery->Seqname, thys->Name);
    ajFeattabOutSetBasename (thys->Ftquery, thys->Filename);
    ret = ajFeattabOutSet (thys->Ftquery, thys->Ufo);
    return ret;
}


/* @func ajSeqOutFormatSingle *************************************************
**
** Checks whether an output format should go to single files, rather than
** all sequences being written to one file. Some formats do not work when
** more than one sequence is writte to a file. Obvious examples are plain
** text and GCG formats.
**
** @param [P] format [AjPStr] Output format required.
** @return [AjBool] ajTrue if separate file is needed for each sequence.
** @@
******************************************************************************/

AjBool ajSeqOutFormatSingle (AjPStr format)
{
    ajint iformat;
    if (!ajSeqFindOutFormat (format, &iformat))
    {
	ajErr ("Unknown output format '%S'", format);
	return ajFalse;
    }

    return seqOutFormat[iformat].Single;
}

/* @func ajSeqOutSetFormat ****************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [wP] thys [AjPSeqout] Sequence output object.
** @param [r] format [AjPStr] Output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutSetFormat (AjPSeqout thys, AjPStr format)
{
    static AjPStr fmt = NULL;

    ajDebug ("ajSeqOutSetFormat '%S'\n", format);
    (void) ajStrAssS (&fmt, format);
    (void) ajSeqOutFormatDefault(&fmt);

    (void) ajStrSet(&thys->Formatstr, fmt);
    ajDebug ("... output format set to '%S'\n", fmt);

    return ajTrue;
}

/* @func ajSeqOutSetFormatC ***************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [wP] thys [AjPSeqout] Sequence output object.
** @param [r] format [char *] Output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutSetFormatC (AjPSeqout thys, char* format)
{
    AjPStr fmt = NULL;
    AjBool ret;

    fmt = ajStrNewC(format);
    ret = ajSeqOutSetFormat(thys,fmt);
    ajStrDel(&fmt);

    return ret;
}

/* @func ajSeqOutFormatDefault ************************************************
**
** Sets the default output format.
** Checks the _OUTFORMAT variable,
** and uses FASTA if no other definition is found.
**
** @param [wP] pformat [AjPStr*] Default output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutFormatDefault (AjPStr* pformat)
{

    if (ajStrLen(*pformat))
    {
	ajDebug ("... output format '%S'\n", *pformat);
    }
    else
    {
	/* ajStrSetC (pformat, seqOutFormat[0].Name);*/
	if  (ajNamGetValueC("outformat", pformat))
	{
	    ajDebug ("ajSeqOutFormatDefault '%S' from EMBOSS_OUTFORMAT\n",
		     *pformat);
	}
	else
	{
	    (void) ajStrSetC (pformat, "fasta"); /* use the real name */
	    ajDebug ("... output format not set, default to '%S'\n", *pformat);
	}
    }

    return ajTrue;
}

/* @func ajSeqPrintOutFormat **************************************************
**
** Reports the internal data structures
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintOutFormat (AjPFile outf, AjBool full)
{

    ajint i=0;

    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "# sequence output formats\n");
    ajFmtPrintF (outf, "# Single: If true, write each sequence to new file\n");
    ajFmtPrintF (outf, "# Save: If true, save sequences, write when closed\n");
    ajFmtPrintF (outf, "# Name         Single Save\n");
    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "OutFormat {\n");
    for (i=0; seqOutFormat[i].Name; i++)
    {
	ajFmtPrintF (outf, "  %-12s    %3B  %3B\n",
		     seqOutFormat[i].Name,
		     seqOutFormat[i].Single,
		     seqOutFormat[i].Save);
    }
    ajFmtPrintF (outf, "}\n\n");

    return;
}

/* @func ajSeqFindOutFormat ***************************************************
**
** Looks for the specified output format in the internal definitions and
** returns the index.
**
** @param [P] format [AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqFindOutFormat (AjPStr format, ajint* iformat)
{

    AjPStr tmpformat = NULL;
    ajint i = 0;

    if (!ajStrLen(format))
    {
	if  (ajNamGetValueC("outformat", &tmpformat))
	{
	    ajDebug ("ajSeqFindOutFormat '%S' from EMBOSS_OUTFORMAT\n",
		     tmpformat);
	}
	else {
	    return ajFalse;
	}
    }
    else {
	(void) ajStrAss (&tmpformat, format);
    }

    (void) ajStrToLower(&tmpformat);

    while (seqOutFormat[i].Name)
    {
	if (ajStrMatchCaseC(tmpformat, seqOutFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    return ajTrue;
	}
	i++;
    }

    ajStrDel(&tmpformat);
    return ajFalse;
}

/* @funcstatic seqSeqFormat ***************************************************
**
** Initialises sequence output formatting parameters.
**
** @param [r] seqlen [ajint] Sequence length
** @param [uP] psf [SeqPSeqFormat*] Sequence format object
** @return [void]
** @@
******************************************************************************/

static void seqSeqFormat (ajint seqlen, SeqPSeqFormat* psf)
{

    char numform[20];
    SeqPSeqFormat sf;
    ajint i;
    ajint j;

    j = 1;

    for (i = seqlen; i; i /= 10)
	j++;

    (void) sprintf(numform, "%d", seqlen);
    ajDebug ("seqSeqFormat numwidth old: %d new: %d\n", strlen(numform)+1, j);

    if (!*psf)
    {
	sf = AJNEW0(*psf);
	sf->namewidth = 8;
	sf->spacer = 0;
	sf->width = 50;
	sf->tab = 0;
	sf->numleft = ajFalse;
	sf->numright = sf->numleft = sf->numjust = ajFalse;
	sf->nameright = sf->nameleft = ajFalse;
	sf->numline = 0;
	sf->linepos = 0;

	sf->skipbefore = ajFalse;
	sf->skipafter = ajFalse;
	sf->isactive = ajFalse;
	sf->baseonlynum = ajFalse;
	sf->gapchar = '-';
	sf->matchchar = '.';
	sf->noleaves = sf->domatch = sf->degap = ajFalse;
	sf->pretty = ajFalse;
	(void) strcpy (sf->endstr, "");
	/*sf->interline = 1;*/
    }
    else
    {
	sf = *psf;
    }

    sf->numwidth = j;		    /* or 8 as a reasonable minimum */

    return;
}

/* @func ajSeqoutCheckGcg *****************************************************
**
** Calculates a GCG checksum for an output sequence.
**
** @param [r] outseq [AjPSeqout] Output sequence.
** @return [ajint] GCG checksum.
** @@
******************************************************************************/

ajint ajSeqoutCheckGcg (AjPSeqout outseq)
{
    register ajlong  i, check = 0, count = 0;
    char *cp = ajStrStr(outseq->Seq);
    ajint ilen = ajStrLen(outseq->Seq);

    for (i = 0; i < ilen; i++)
    {
	count++;
	check += count * toupper((ajint) cp[i]);
	if (count == 57)
	    count = 0;
    }
    check %= 10000;

    return check;
}

/* @funcstatic seqWriteSeq ****************************************************
**
** Writes an output sequence. The format and all other information is
** already stored in the output sequence object and the formatting structure.
**
** @param [r] outseq [AjPSeqout] Output sequence.
** @param [w] sf [SeqPSeqFormat] Output formatting structure.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSeq (AjPSeqout outseq, SeqPSeqFormat sf)
{
    
    /* code adapted from what readseq did */
    
    static ajint maxSeqWidth = 250;
    static char* defNocountSymbols = "_.-?";
    
    ajint i=0, j=0, l=0, ibase=0;
    ajint linesout = 0;
    ajint seqlen = ajStrLen(outseq->Seq);
    char *seq = ajStrStr(outseq->Seq);
    char *idword;
    char *cp;
    char s[1024];			/* the output line */
    
    char nameform[20];
    char numform[20];
    char nocountsymbols[20];
    
    /* these are changed when writing */
    
    ajint width = sf->width;
    ajint l1 = sf->linepos;
    
    AjPFile file = outseq->File;
    FILE* outf = ajFileFp(file);
    
    /*
       ajint numline = 0;
       
       ajint namewidth = sf->namewidth;
       ajint numwidth = sf->numwidth;
       ajint spacer = sf->spacer;
       ajint tab = sf->tab;
       AjBool nameleft = sf->nameleft;
       AjBool nameright = sf->nameright;
       AjBool numleft = sf->numleft;
       AjBool numright = sf->numright;
       AjBool numjust = sf->numjust;
       AjBool skipbefore = sf->skipbefore;
       AjBool skipafter = sf->skipafter;
       AjBool baseonlynum = sf->baseonlynum;
       AjBool pretty = sf->pretty;
       char *endstr = sf->endstr;
       char *leftstr = sf->leftstr;
       */
    
    ajDebug("seqWriteSeq\n");
    /* if (sf->numline) numline = 1;*/
    
    if (sf->nameleft || sf->nameright)
	(void) sprintf(nameform, "%%%d.%ds ",sf->namewidth,sf->namewidth);
    if (sf->numline)
	(void) sprintf(numform, "%%%ds ",sf->numwidth);
    else
	(void) sprintf(numform, "%%%dd",sf->numwidth);
    
    (void) strcpy( nocountsymbols, defNocountSymbols);
    if (sf->baseonlynum)
    {				      /* add gap character to skips */
	if (strchr(nocountsymbols,sf->gapchar)==NULL)
	{
	    (void) strcat(nocountsymbols," ");
	    nocountsymbols[strlen(nocountsymbols)-1]= sf->gapchar;
	}
	if (sf->domatch &&	 /* remove gap character from skips */
	    (cp=strchr(nocountsymbols,sf->matchchar))!=NULL)
	{
	    *cp= ' ';
	}
    }
    
    if (sf->numline)
	idword= "";
    else
	idword = ajStrStr(outseq->Name);
    
    width = AJMIN(width,maxSeqWidth);
    
    i=0;		     /* seqpos position in seq[]*/
    l=0;		     /* linepos position in output line s[] */
    
    ibase = 1;				/* base count */
    
    while (i < seqlen)
    {
	
	if (l1 < 0)
	    l1 = 0;
	else if (l1 == 0) {		/* start of a new line */
	    if (sf->skipbefore) {
		(void) fprintf(outf, "\n"); /* blank line before writing */
		linesout++;
	    }
	    if (*(sf->leftstr))
	    {
		(void) fprintf(outf, sf->leftstr); /* string at line start */
	    }
	    if (sf->nameleft)
		(void) fprintf(outf, nameform, idword);
	    if (sf->numleft)
	    {
		if (sf->numline)
		    (void) fprintf(outf, numform, "");
		else
		    (void) fprintf(outf, numform, ibase);
	    }
	    for (j=0; j < sf->tab; j++)
		(void) fputc(' ',outf);
	}
	
	l1++;                 /* don't count spaces for width*/
	if (sf->numline) {
	    if (sf->spacer==seqSpaceAll ||
		(sf->spacer != 0 && (l+1) % sf->spacer == 1))
	    {
		if (sf->numline) (void) fputc(' ',outf);
		s[l++] = ' ';
	    }
	    if (l1 % 10 == 1 || l1 == width)
	    {
		if (sf->numline) (void) fprintf(outf,"%-9d ",i+1);
		s[l++]= '|';		/* == put a number here */
	    }
	    else s[l++]= ' ';
	    i++;
	}
	
	else {
	    if (sf->spacer==seqSpaceAll ||
		(sf->spacer != 0 && (l+1) % sf->spacer == 1))
		s[l++] = ' ';
	    if (!sf->baseonlynum)
		ibase++;
	    else if (0==strchr(nocountsymbols,seq[i]))
		ibase++;
	    s[l++] = seq[i++];
	}
	
	if (l1 == width || i == seqlen)
	{
	    if (sf->pretty || sf->numjust)
	    {
		for ( ; l1<width; l1++)
		{
		    if (sf->spacer==seqSpaceAll ||
			(sf->spacer != 0 && (l+1) % sf->spacer == 1))
			s[l++] = ' ';
		    s[l++]=' ';		/* pad with blanks */
		}
	    }
	    s[l] = '\0';
	    l = 0; l1 = 0;

	    if (!sf->numline)
	    {
		(void) fprintf(outf,"%s",s);
		if (sf->numright || sf->nameright)
		    (void) fputc(' ',outf);
		if (sf->numright)
		    (void) fprintf(outf,numform, ibase-1);
		if (sf->nameright)
		    (void) fprintf(outf, nameform,idword);
		if (i == seqlen)
		    (void) fprintf(outf,"%s",sf->endstr);
	    }
	    (void) fputc('\n',outf);
	    linesout++;
	    if (sf->skipafter) {
		(void) fprintf(outf, "\n");
		linesout++;
	    }
	}
    }
    
    return;
}

/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */

/* @section Sequence Output Casts *********************************************
**
** These functions examine the contents of a sequence output object
** and return some derived information. Some of them provide access to
** the internal components of a sequence output object. They are
** provided for programming convenience but should be used with
** caution.
**
******************************************************************************/

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Output Assignments ***************************************
**
** These functions overwrite the sequence output object provided as
** the first argument.
**
******************************************************************************/

/* @funcstatic seqClone *******************************************************
**
** Copies data from a sequence into a sequence output object.
** Used before writing the sequence.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

static void seqClone (AjPSeqout outseq, AjPSeq seq)
{

    ajint ibegin = 1;
    ajint iend = ajStrLen(seq->Seq);

    if (seq->Begin)
    {
	ibegin = ajSeqBegin(seq);
	ajDebug ("seqClone begin: %d\n", ibegin);
    }

    if (seq->End)
    {
	iend = ajSeqEnd(seq);
	ajDebug ("seqClone begin: %d\n", ibegin);
    }

    (void) ajStrSet (&outseq->Name, seq->Name);
    (void) ajStrSet (&outseq->Acc, seq->Acc);
    (void) ajListstrClone(seq->Acclist, outseq->Acclist);
    (void) ajStrSet (&outseq->Sv, seq->Sv);
    (void) ajStrSet (&outseq->Gi, seq->Gi);
    (void) ajStrSet (&outseq->Tax, seq->Tax);
    (void) ajListstrClone(seq->Taxlist, outseq->Taxlist);
    (void) ajListstrClone(seq->Keylist, outseq->Keylist);
    (void) ajStrSet (&outseq->Desc, seq->Desc);
    (void) ajStrSet (&outseq->Type, seq->Type);
    (void) ajStrSet (&outseq->Informatstr, seq->Formatstr);
    (void) ajStrSet (&outseq->Entryname, seq->Entryname);
    (void) ajStrSet (&outseq->Db, seq->Db);

    outseq->Offset = ibegin - 1;

    if (iend >= ibegin)
	(void) ajStrAssSub (&outseq->Seq, seq->Seq, ibegin-1, iend-1);
    else				/* empty sequence */
	(void) ajStrAssC (&outseq->Seq, "");

    outseq->Fttable = seq->Fttable;
    if (outseq->Fttable)
	ajFeattableTrimOff (outseq->Fttable,
			    outseq->Offset, ajStrLen(outseq->Seq));

    ajDebug ("seqClone %d .. %d %d .. %d len: %d type: '%S'\n",
	     seq->Begin, seq->End, ibegin, iend,
	     ajStrLen(outseq->Seq), outseq->Type);
    ajDebug ("  Db: '%S' Name: '%S' Entryname: '%S'\n",
	     outseq->Db, outseq->Name, outseq->Entryname);

    ajSeqTypeCheckS(&outseq->Seq, outseq->Outputtype);

    return;
}

/* @funcstatic seqAllClone ****************************************************
**
** Copies data from a sequence into a sequence output object.
** Used before writing the sequence. This version works with sequence streams.
** The difference is that the output object must be overwritten.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

static void seqAllClone (AjPSeqout outseq, AjPSeq seq)
{

    ajint ibegin = 1;
    ajint iend = ajStrLen(seq->Seq);

    if (seq->Begin)
    {
	ibegin = ajSeqBegin(seq);
	ajDebug ("seqAllClone begin: %d\n", ibegin);
    }

    if (seq->End)
    {
	iend = ajSeqEnd(seq);
	ajDebug ("seqAllClone end: %d\n", iend);
    }

    (void) ajStrAssS (&outseq->Db, seq->Db);
    (void) ajStrAssS (&outseq->Name, seq->Name);
    (void) ajStrAssS (&outseq->Acc, seq->Acc);
    (void) ajListstrClone(seq->Acclist, outseq->Acclist);
    (void) ajStrAssS (&outseq->Sv, seq->Sv);
    (void) ajStrAssS (&outseq->Gi, seq->Gi);
    (void) ajStrAssS (&outseq->Tax, seq->Tax);
    (void) ajListstrClone(seq->Taxlist, outseq->Taxlist);
    (void) ajListstrClone(seq->Keylist, outseq->Keylist);
    (void) ajStrAssS (&outseq->Desc, seq->Desc);
    (void) ajStrAssS (&outseq->Type, seq->Type);
    (void) ajStrAssS (&outseq->Informatstr, seq->Formatstr);
    (void) ajStrAssS (&outseq->Entryname, seq->Entryname);

    outseq->Offset = ibegin - 1;

    if (iend >= ibegin)
	(void) ajStrAssSub (&outseq->Seq, seq->Seq, ibegin-1, iend-1);
    else				/* empty sequence */
	(void) ajStrAssC (&outseq->Seq, "");

    outseq->Fttable = seq->Fttable;
    if (outseq->Fttable)
	ajFeattableTrimOff (outseq->Fttable,
			    outseq->Offset, ajStrLen(outseq->Seq));

    ajDebug ("seqAllClone %d .. %d %d .. %d len: %d type: '%S'\n",
	     seq->Begin, seq->End, ibegin, iend,
	     ajStrLen(outseq->Seq), outseq->Type);
    ajDebug ("  Db: '%S' Name: '%S' Entryname: '%S'\n",
	     outseq->Db, outseq->Name, outseq->Entryname);

    ajSeqTypeCheckS(&outseq->Seq, outseq->Outputtype);

    return;
}

/* @funcstatic seqsetClone ****************************************************
**
** Clones one sequence from a set ready for output.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seqset [AjPSeqset] Sequence set.
** @param [r] i [ajint] Sequence number, zero for the first sequence.
** @return [void]
** @@
******************************************************************************/

static void seqsetClone (AjPSeqout outseq, AjPSeqset seqset, ajint i)
{

    /* intended to clone ith sequence in the set */

    AjPSeq seq = seqset->Seq[i];

    seqAllClone (outseq, seq);

    return;
}

/* @funcstatic seqDeclone *****************************************************
**
** Clears clones data in a sequence output object.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @return [void]
** @@
******************************************************************************/

static void seqDeclone (AjPSeqout outseq)
{

    AjPStr ptr=NULL;

    (void) ajStrClear (&outseq->Db);
    (void) ajStrClear (&outseq->Name);
    (void) ajStrClear (&outseq->Acc);
    (void) ajStrClear (&outseq->Sv);
    (void) ajStrClear (&outseq->Gi);
    (void) ajStrClear (&outseq->Tax);
    (void) ajStrClear (&outseq->Desc);
    (void) ajStrClear (&outseq->Type);
    (void) ajStrClear (&outseq->Informatstr);
    (void) ajStrClear (&outseq->Entryname);
    while(ajListstrPop(outseq->Acclist,&ptr))
	ajStrDel(&ptr);
    while(ajListstrPop(outseq->Keylist,&ptr))
	ajStrDel(&ptr);
    while(ajListstrPop(outseq->Taxlist,&ptr))
	ajStrDel(&ptr);
    (void) ajStrClear (&outseq->Seq);

    return;
}

/* @funcstatic seqFileReopen **************************************************
**
** Reopen a sequence output file. Used after the file name has been changed
** when writing a set of sequences one to each file.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqFileReopen (AjPSeqout outseq)
{
    AjPStr name = NULL;

    if (outseq->File)
	ajFileClose (&outseq->File);

    if (outseq->Knownfile)
	outseq->Knownfile = NULL;

    (void) ajFmtPrintS(&name, "%S.%S", outseq->Name, outseq->Extension);
    (void) ajStrToLower (&name);
    outseq->File = ajFileNewOutD(outseq->Directory, name);
    ajDebug("seqFileReopen single: %B file '%S'\n", outseq->Single, name);
    ajStrDel(&name);

    if (!outseq->File)
	return ajFalse;

    return ajTrue;
}

/* @func ajSeqoutUsa **********************************************************
**
** Creates or resets a sequence output object using a new Universal
** Sequence Address
**
** @param [uP] pthis [AjPSeqout*] Sequence output object.
** @param [P] Usa [AjPStr] USA
** @return [void]
** @@
******************************************************************************/

void ajSeqoutUsa (AjPSeqout* pthis, AjPStr Usa)
{
    AjPSeqout thys;

    if (!*pthis)
    {
	thys = *pthis = ajSeqoutNew();
    }
    else
    {
	thys = *pthis;
	ajSeqoutClear(thys);
    }

    (void) ajStrAss (&thys->Usa, Usa);

    return;
}

/* @func ajSeqoutClear ********************************************************
**
** Clears a Sequence output object back to "as new" condition
**
** @param [P] thys [AjPSeqout] Sequence output object
** @return [void]
** @@
******************************************************************************/

void ajSeqoutClear (AjPSeqout thys)
{

    AjPStr ptr=NULL;

    ajDebug ("ajSeqoutClear called\n");

    (void) ajStrClear(&thys->Name);
    (void) ajStrClear(&thys->Acc);
    (void) ajStrClear(&thys->Sv);
    (void) ajStrClear(&thys->Gi);
    (void) ajStrClear(&thys->Tax);
    (void) ajStrClear(&thys->Desc);
    (void) ajStrClear(&thys->Type);
    (void) ajStrClear(&thys->Outputtype);
    (void) ajStrClear(&thys->Full);
    (void) ajStrClear(&thys->Date);
    (void) ajStrClear(&thys->Doc);
    (void) ajStrClear(&thys->Usa);
    (void) ajStrClear(&thys->Ufo);
    (void) ajStrClear(&thys->Informatstr);
    (void) ajStrClear(&thys->Formatstr);
    (void) ajStrClear(&thys->Filename);
    (void) ajStrClear(&thys->Directory);
    (void) ajStrClear(&thys->Entryname);
    (void) ajStrClear(&thys->Extension);
    (void) ajStrClear(&thys->Seq);
    thys->EType = 0;
    thys->Rev = ajFalse;
    thys->Format = 0;
    if (thys->File)
    {
	if (thys->Knownfile)
	    thys->File = NULL;
	else
	    ajFileClose(&thys->File);
    }
    thys->Count = 0;
    thys->Single = ajFalse;
    thys->Features = ajFalse;
    while(ajListstrPop(thys->Acclist,&ptr))
	ajStrDel(&ptr);
    while(ajListstrPop(thys->Keylist,&ptr))
	ajStrDel(&ptr);
    while(ajListstrPop(thys->Taxlist,&ptr))
	ajStrDel(&ptr);

    return;
}

/* @funcstatic seqDbName ******************************************************
**
** Adds the database name (if any) to the name provided.
**
** @param [w] name [AjPStr*] Derived name.
** @param [r] db [AjPStr] Database name (if any)
** @return [void]
** @@
******************************************************************************/

static void seqDbName (AjPStr* name, AjPStr db)
{

    static AjPStr tmpname = 0;

    if (!ajStrLen(db))
	return;

    (void) ajStrAssS (&tmpname, *name);
    (void) ajFmtPrintS (name, "%S:%S", db, tmpname);

    return;
}

/* @funcstatic seqDefName *****************************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] name [AjPStr*] Derived name.
** @param [r] multi [AjBool] If true, appends a number to the name.
** @return [void]
** @@
******************************************************************************/

static void seqDefName (AjPStr* name, AjBool multi)
{

    static ajint count=0;

    if (ajStrLen(*name)) {
	ajDebug("seqDefName already has a name '%S'\n", *name);
	return;
    }

    if (multi)
	(void) ajFmtPrintS(name, "EMBOSS_%3.3d", ++count);
    else
	(void) ajStrAssC (name, "EMBOSS");

    ajDebug("seqDefName set to  '%S'\n", *name);

    return;
}

/* @func ajSeqoutTrace ********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

void ajSeqoutTrace (AjPSeqout seq)
{
    AjIList it;
    AjPStr cur;
    
    ajDebug ("\n\n\nSequence Out trace\n");
    ajDebug ( "==============\n\n");
    ajDebug ( "  Name: '%S'\n", seq->Name);
    if (ajStrLen(seq->Acc))
	ajDebug ( "  Accession: '%S'\n", seq->Acc);
    if (ajListLength(seq->Acclist))
    {
	(void) ajDebug ("  Acclist: (%d)",
			ajListLength(seq->Acclist));
	it = ajListIter(seq->Acclist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    (void) ajDebug (" %S\n", cur);
	}
	(void) ajListIterFree (it);
	(void) ajDebug ("\n");
    }
    if (ajStrLen(seq->Sv))
	ajDebug ( "  SeqVersion: '%S'\n", seq->Sv);
    if (ajStrLen(seq->Gi))
	ajDebug ( "  GI Version: '%S'\n", seq->Gi);
    if (ajStrLen(seq->Desc))
	ajDebug ( "  Description: '%S'\n", seq->Desc);
    if (ajStrSize(seq->Seq))
	ajDebug ( "  Reserved: %d\n", ajStrSize(seq->Seq));
    if (ajListLength(seq->Keylist))
    {
	(void) ajDebug ("  Keywordlist: (%d)",
			ajListLength(seq->Keylist));
	it = ajListIter(seq->Keylist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    (void) ajDebug ("   '%S'\n", cur);
	}
	(void) ajListIterFree (it);
	(void) ajDebug ("\n");
    }
    (void) ajDebug ("  Taxonomy: '%S'\n", seq->Tax);
    if (ajListLength(seq->Taxlist))
    {
	(void) ajDebug ("  Taxlist: (%d)",
			ajListLength(seq->Taxlist));
	it = ajListIter(seq->Taxlist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	{
	    (void) ajDebug ("   '%S'\n", cur);
	}
	(void) ajListIterFree (it);
    }
    if (ajStrLen(seq->Type))
	ajDebug ( "  Type: '%S'\n", seq->Type);
    if (ajStrLen(seq->Outputtype))
	ajDebug ( "  Output type: '%S'\n", seq->Outputtype);
    if (ajStrLen(seq->Db))
	ajDebug ( "  Database: '%S'\n", seq->Db);
    if (ajStrLen(seq->Full))
	ajDebug ( "  Full name: '%S'\n", seq->Full);
    if (ajStrLen(seq->Date))
	ajDebug ( "  Date: '%S'\n", seq->Date);
    if (ajStrLen(seq->Usa))
	ajDebug ( "  Usa: '%S'\n", seq->Usa);
    if (ajStrLen(seq->Ufo))
	ajDebug ( "  Ufo: '%S'\n", seq->Ufo);
    if (ajStrLen(seq->Formatstr))
	ajDebug ( "  Output format: '%S'\n", seq->Formatstr);
    if (ajStrLen(seq->Filename))
	ajDebug ( "  Filename: '%S'\n", seq->Filename);
    if (ajStrLen(seq->Directory))
	ajDebug ( "  Directory: '%S'\n", seq->Directory);
    if (ajStrLen(seq->Entryname))
	ajDebug ( "  Entryname: '%S'\n", seq->Entryname);
    if (ajStrLen(seq->Doc))
	ajDebug ( "  Documentation:...\n%S\n", seq->Doc);
    if(seq->Fttable)
	ajFeattableTrace(seq->Fttable);
    else
	ajDebug( "  No Feature table present\n");
    if(seq->Features)
	ajDebug( "  Features ON\n");
    else
	ajDebug( "  Features OFF\n");
    
}

/* @func ajSeqWriteXyz ********************************************************
**
** Writes a sequence in SWISSPROT format.
**
** @param [w] outf [AjPFile] output stream
** @param [r] seq [AjPStr] sequence
** @param [r] prefix [char *] identifier code - should be 2 char's long
** @return [void]
** @@
******************************************************************************/

void ajSeqWriteXyz(AjPFile outf, AjPStr seq, char *prefix)
{
    AjPSeqout outseq=NULL;
    static SeqPSeqFormat sf=NULL;

    ajint mw;
    ajuint crc;

    outseq = ajSeqoutNew();

    outseq->File = outf;
    ajStrAssS(&outseq->Seq,seq);

    crc = ajSeqCrc (outseq->Seq);
    mw = (ajint) (0.5+ajSeqMW (outseq->Seq));
    (void) ajFmtPrintF (outseq->File,
			"%-5sSEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
			prefix, ajStrLen(outseq->Seq), mw, crc);

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;

    seqWriteSeq (outseq, sf);

    ajSeqoutDel(&outseq);

    return;
}

/* @func ajSssWriteXyz ********************************************************
**
** Writes a sequence in SWISSPROT format w/o checksum or molecular weight -
** used for printing secondary structure strings.
**
** @param [w] outf [AjPFile] output stream
** @param [r] seq [AjPStr] sequence
** @param [r] prefix [char *] identifier code - should be 2 char's long
** @return [void]
** @@
******************************************************************************/

void ajSssWriteXyz(AjPFile outf, AjPStr seq, char *prefix)
{
    AjPSeqout outseq=NULL;
    static SeqPSeqFormat sf=NULL;

    ajint mw;
    ajint crc;

    outseq = ajSeqoutNew();

    outseq->File = outf;
    ajStrAssS(&outseq->Seq,seq);

    crc = ajSeqCrc (outseq->Seq);
    mw = (ajint) (0.5+ajSeqMW (outseq->Seq));
    (void) ajFmtPrintF (outseq->File,
			"%-5sSEQUENCE %5d AA;\n",
			prefix, ajStrLen(outseq->Seq));

    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    (void) strcpy (sf->endstr, "");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;

    seqWriteSeq (outseq, sf);

    ajSeqoutDel(&outseq);

    return;
}

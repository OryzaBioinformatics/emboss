/******************************************************************************
** @source AJAX seqwrite  functions
**
** @author Copyright (C) 2001 Peter Rice
** @version 1.0
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


static AjPRegexp seqoutRegFmt = NULL;
static AjPRegexp seqoutRegId  = NULL;

static AjPStr seqoutUsaTest = NULL;

/* @datastatic SeqPOutFormat **************************************************
**
** Sequence output formats
**
** @attr Name [char*] Format name
** @attr Desc [char*] Format description
** @attr Alias [AjBool] Name is an alias for an identical definition
** @attr Single [AjBool] Write each sequence to a new file if true (e.g. GCG)
** @attr Save [AjBool] Save in memory and write at end (e.g. MSF alignments)
** @attr Nucleotide [AjBool] True if nucleotide data is supported
** @attr Protein [AjBool] True if protein data is supported
** @attr Feature [AjBool] True if feature data can be written
** @attr Gap [AjBool] True if gap characters are supported
** @attr Multiset [AjBool] True if sets of sets (seqsetall) are supported
** @attr Write [(void*)] Function to write the format
** @@
******************************************************************************/

typedef struct SeqSOutFormat
{
    char *Name;
    char *Desc;
    AjBool Alias;
    AjBool Single;
    AjBool Save;
    AjBool Nucleotide;
    AjBool Protein;
    AjBool Feature;
    AjBool Gap;
    AjBool Multiset;
    void (*Write) (AjPSeqout outseq);
} SeqOOutFormat;

#define SeqPOutFormat SeqOOutFormat*




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
} SeqOSeqFormat;

#define SeqPSeqFormat SeqOSeqFormat*




static ajint seqSpaceAll = -9;




static void       seqAllClone(AjPSeqout outseq, const AjPSeq seq);
static void       seqClone(AjPSeqout outseq, const AjPSeq seq);
static void       seqDbName(AjPStr* name, const AjPStr db);
static void       seqDeclone(AjPSeqout outseq);
static AjBool     seqFileReopen(AjPSeqout outseq);
static void       seqFormatDel(SeqPSeqFormat* pformat);
static AjBool     seqoutUfoLocal(const AjPSeqout thys);
static AjBool     seqoutUsaProcess(AjPSeqout thys);
static void       seqsetClone(AjPSeqout outseq, const AjPSeqset seq, ajint i);

static void       seqSeqFormat(ajint seqlen, SeqPSeqFormat* psf);
static void       seqWriteAcedb(AjPSeqout outseq);
static void       seqWriteAsn1(AjPSeqout outseq);
static void       seqWriteClustal(AjPSeqout outseq);
static void       seqWriteCodata(AjPSeqout outseq);
static void       seqWriteDebug(AjPSeqout outseq);
static void       seqWriteEmbl(AjPSeqout outseq);
static void       seqWriteEmblnew(AjPSeqout outseq);
static void       seqWriteExperiment(AjPSeqout outseq);
static void       seqWriteFasta(AjPSeqout outseq);
static void       seqWriteFitch(AjPSeqout outseq);
static void       seqWriteGcg(AjPSeqout outseq);
static void       seqWriteGenbank(AjPSeqout outseq);
static void       seqWriteGff(AjPSeqout outseq);
static void       seqWriteHennig86(AjPSeqout outseq);
static void       seqWriteIg(AjPSeqout outseq);
static void       seqWriteJackknifer(AjPSeqout outseq);
static void       seqWriteJackknifernon(AjPSeqout outseq);
static void       seqWriteListAppend(AjPSeqout outseq, const AjPSeq seq);
static void       seqWriteMase(AjPSeqout outseq);
static void       seqWriteMega(AjPSeqout outseq);
static void       seqWriteMeganon(AjPSeqout outseq);
static void       seqWriteMsf(AjPSeqout outseq);
static void       seqWriteNbrf(AjPSeqout outseq);
static void       seqWriteNcbi(AjPSeqout outseq);
static void       seqWriteNexus(AjPSeqout outseq);
static void       seqWriteNexusnon(AjPSeqout outseq);
static void       seqWritePhylip(AjPSeqout outseq);
static void       seqWritePhylipnon(AjPSeqout outseq);
static void       seqWriteSelex(AjPSeqout outseq);
static void       seqWriteSeq(AjPSeqout outseq, const SeqPSeqFormat sf);
static void       seqWriteStaden(AjPSeqout outseq);
static void       seqWriteStrider(AjPSeqout outseq);
static void       seqWriteSwiss(AjPSeqout outseq);
static void       seqWriteSwissnew(AjPSeqout outseq);
static void       seqWriteText(AjPSeqout outseq);
static void       seqWriteTreecon(AjPSeqout outseq);




/* @funclist seqOutFormat *****************************************************
**
** Functions to write each sequence format
**
******************************************************************************/

static SeqOOutFormat seqOutFormat[] =
{
/*   Name,         Description */
/*      Alias     Single,  Save,    Nucleotide, Protein */
/*      Feature, Gap,     Multiset,WriteFunction */
    {"unknown",    "Unknown format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteFasta}, /* internal default
							writes FASTA */
    /* set 'fasta' in ajSeqOutFormatDefault */
    {"gcg",        "GCG sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteGcg},
    {"gcg8",       "GCG old (version 8) sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteGcg}, /* alias for gcg */
    {"embl",       "EMBL entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteEmbl},
    {"emblold",         "EMBL entry format (alias)",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteEmbl}, /* alias for embl */
    {"em",         "EMBL entry format (alias)",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteEmbl}, /* alias for embl */
    {"emblnew",       "EMBL new entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteEmblnew},
    {"swiss",      "Swissprot entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwiss},
    {"swissold",      "Swissprot entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwiss},
    {"sw",         "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwiss}, /* alias for swiss */
    {"swissprot",  "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwiss}, /* alias for swiss */
    {"swissnew",      "Swissprot entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwissnew},
    {"swnew",         "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwissnew}, /* alias for swiss */
    {"swissprotnew",  "Swissprot entry format(alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJFALSE, AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteSwissnew}, /* alias for swiss */
    {"fasta",      "FASTA format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteFasta},
    {"pearson",    "FASTA format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteFasta}, /* alias for fasta */
    {"ncbi",       "NCBI fasta format with NCBI-style IDs",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteNcbi},
    {"nbrf",       "NBRF/PIR entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteNbrf},
    {"pir",        "NBRF/PIR entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteNbrf}, /* alias for nbrf */
    {"genbank",    "Genbank entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteGenbank},
    {"gb",         "Genbank entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteGenbank}, /* alias for genbank */
    {"ddbj",       "Genbank/DDBJ entry format (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJFALSE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteGenbank}, /* alias for genbank */
    {"gff",        "GFF feature file with sequence in the header",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJTRUE,  AJTRUE,  AJFALSE, seqWriteGff},
    {"ig",         "Intelligenetics sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteIg},
    {"codata",     "Codata entry format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteCodata},
    {"strider",    "DNA strider output format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteStrider},
    {"acedb",      "ACEDB sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteAcedb},
    {"experiment", "Staden experiment file",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteExperiment},
    {"staden",     "Old staden package sequence format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteStaden},
    {"text",       "Plain text",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteText},
    {"plain",      "Plain text (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteText}, /* alias for text */
    {"raw",        "Plain text (alias)",
	 AJTRUE,  AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteText}, /* alias for text output */
    {"fitch",      "Fitch program format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteFitch},
    {"msf",        "GCG MSF (mutiple sequence file) file format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteMsf},
    {"clustal",    "Clustalw output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteClustal},
    {"aln",        "Clustalw output format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteClustal}, /* alias for clustal */
    {"selex",      "Selex format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteSelex},
    {"phylip",     "Phylip interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJTRUE,  seqWritePhylip},
    {"phylipnon",  "Phylip non-interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWritePhylipnon},
    {"phylip3",    "Phylip non-interleaved format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,  /* alias for phylipnon*/
	 AJFALSE, AJTRUE,  AJFALSE, seqWritePhylipnon},
    {"asn1",       "NCBI ASN.1 format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteAsn1},
    {"hennig86",   "Hennig86 output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteHennig86},
    {"mega",       "Mega interleaved output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteMega},
    {"meganon",    "Mega non-interleaved output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteMeganon},
    {"nexus",      "Nexus/paup interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteNexus},
    {"paup",       "Nexus/paup interleaved format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteNexus}, /* alias for nexus */
    {"nexusnon",   "Nexus/paup non-interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteNexusnon},
    {"paupnon",    "Nexus/paup non-interleaved format (alias)",
	 AJTRUE,  AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteNexusnon},	/* alias for nexusnon*/
    {"jackknifer", "Jackknifer output interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteJackknifer},
    {"jackknifernon", "Jackknifer output non-interleaved format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteJackknifernon},
    {"treecon",    "Treecon output format",
	 AJFALSE, AJFALSE, AJTRUE,  AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteTreecon},
    {"mase",       "Mase program format",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteMase},
    {"debug",      "Debugging trace of full internal data content",
	 AJFALSE, AJFALSE, AJFALSE, AJTRUE,  AJTRUE,
	 AJFALSE, AJTRUE,  AJFALSE, seqWriteDebug}, /* trace report */
    {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL}
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
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

void ajSeqAllWrite(AjPSeqout outseq, const AjPSeq seq)
{
    
    ajDebug("ajSeqAllWrite '%s' len: %d\n",
	    ajSeqGetNameS(seq), ajSeqGetLen(seq));
    
    if(!outseq->Format)
	if(!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format))
	    ajErr("unknown output format '%S'", outseq->Formatstr);
    
    ajDebug("ajSeqAllWrite %d '%s' single: %B feat: %B Save: %B\n",
	    outseq->Format,
	    seqOutFormat[outseq->Format].Name,
	    seqOutFormat[outseq->Format].Single,
	    outseq->Features,
	    seqOutFormat[outseq->Format].Save);
    
    seqAllClone(outseq, seq);
    if(seqOutFormat[outseq->Format].Save)
    {
	seqWriteListAppend(outseq, seq);
	outseq->Count++;
	return;
    }
           
    ajSeqoutDefName(outseq, outseq->Entryname, !outseq->Single);
    if(outseq->Fttable)
	ajFeatDefName(outseq->Fttable, outseq->Name);

    if(outseq->Single)
	seqFileReopen(outseq);
    
    /* Calling funclist seqOutFormat() */
    seqOutFormat[outseq->Format].Write(outseq);
    outseq->Count++;
    
    ajDebug("ajSeqAllWrite tests features %B tabouitisopen %B "
	    "UfoLocal %B ftlocal %B\n",
	    outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
	    seqoutUfoLocal(outseq), ajFeattabOutIsLocal(outseq->Ftquery));

    if(outseq->Features &&
       !ajFeattabOutIsLocal(outseq->Ftquery)) /* not already done */
    {
	if(!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("ajSeqAllWrite features output needed\n");
	    ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);
	    if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("ajSeqAllWrite features output file open failed '%S%S'",
		       outseq->Ftquery->Directory, outseq->Ftquery->Filename);
		return;
	    }
	    ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
	    ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);
	}

	/* ajFeattableTrace(outseq->Fttable); */
	if(!ajFeatUfoWrite(outseq->Fttable, outseq->Ftquery,
			   outseq->Ufo))
	{
	    ajWarn("ajSeqAllWrite features output failed UFO: '%S'",
		   outseq->Ufo);
	    return;
	}
    }
    
    seqDeclone(outseq);
    
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
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeqset] Sequence set.
** @return [void]
** @category output [AjPSeqset] Writes out all sequences in a set
** @category modify [AjPSeqout] Master sequence set output
**                routine
** @@
******************************************************************************/

void ajSeqsetWrite(AjPSeqout outseq, const AjPSeqset seq)
{
    ajint i = 0;

    ajDebug("ajSeqsetWrite\n");

    if(!outseq->Format)
	if(!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format))
	    ajErr("unknown output format '%S'", outseq->Formatstr);

    ajDebug("ajSeqSetWrite %d '%s' single: %B feat: %B Save: %B\n",
	    outseq->Format,
	    seqOutFormat[outseq->Format].Name,
	    seqOutFormat[outseq->Format].Single,
	    outseq->Features,
	    seqOutFormat[outseq->Format].Save);

    for(i=0; i < seq->Size; i++)
    {
	seqsetClone(outseq, seq, i);
	if(seqOutFormat[outseq->Format].Save)
	{
	    seqWriteListAppend(outseq, seq->Seq[i]);
	    outseq->Count++;
	    continue;
	}

	ajSeqoutDefName(outseq, outseq->Entryname, !outseq->Single);
	if(outseq->Fttable)
	    ajFeatDefName(outseq->Fttable, outseq->Name);

	if(outseq->Single)
	    seqFileReopen(outseq);

	/* Calling funclist seqOutFormat() */
	seqOutFormat[outseq->Format].Write(outseq);
	outseq->Count++;

	ajDebug("ajSeqsetWrite tests features %B tabouitisopen %B "
		"UfoLocal %B ftlocal %B\n",
		outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
		seqoutUfoLocal(outseq), ajFeattabOutIsLocal(outseq->Ftquery));

	if(outseq->Features &&
	   !ajFeattabOutIsLocal(outseq->Ftquery))
	{
	    /* not already done */
	    if(!ajFeattabOutIsOpen(outseq->Ftquery))
	    {
		ajDebug("ajSeqsetWrite features output needed\n");
		ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);
		if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
		{
		    ajWarn("ajSeqsetWrite features output "
			   "failed to open UFO '%S'",
			   outseq->Ufo);
		    return;
		}
		ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
		ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);
	    }

	    /* ajFeattableTrace(outseq->Fttable); */

	    if(!ajFeatUfoWrite(outseq->Fttable,
			       outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("ajSeqsetWrite features output failed UFO: '%S'",
		       outseq->Ufo);
		return;
	    }
	}
	
	seqDeclone(outseq);
    }
    
    return;
}




/* @funcstatic seqWriteListAppend *********************************************
**
** Add the latest sequence to the output list. If we are in single
** sequence mode, also write it out now though it does not seem
** a great idea in most cases to ask for this.
**
** @param [u] outseq [AjPSeqout] Sequence output
** @param [r] seq [const AjPSeq] Sequence to be appended
** @return [void]
** @@
******************************************************************************/

static void seqWriteListAppend(AjPSeqout outseq, const AjPSeq seq)
{
    AjPSeq listseq;

    ajDebug("seqWriteListAppend '%F' %S\n", outseq->File, ajSeqGetNameS(seq));

    if(!outseq->Savelist)
	outseq->Savelist = ajListNew();

    listseq = ajSeqNewS(seq);
    ajSeqTrim(listseq);

    /* if(listseq->Rev)
       ajSeqReverse(listseq); */ /* already done */

    ajSeqDefName(listseq, outseq->Entryname, !outseq->Single);
    if(listseq->Fttable)
	ajFeatDefName(listseq->Fttable, listseq->Name);

    ajListPushApp(outseq->Savelist, listseq);

    if(outseq->Single)
    {
	ajDebug("single sequence mode: write immediately\n");
	ajSeqoutDefName(outseq, outseq->Entryname, !outseq->Single);
	if(outseq->Fttable)
	    ajFeatDefName(outseq->Fttable, outseq->Name);
	/* Calling funclist seqOutFormat() */
	seqOutFormat[outseq->Format].Write(outseq);
    }

    ajDebug("seqWriteListAppend Features: %B IsLocal: %B Count: %d\n",
	    outseq->Features, ajFeattabOutIsLocal(outseq->Ftquery),
	    ajFeattableSize(outseq->Fttable));

    if(outseq->Features &&
       !ajFeattabOutIsLocal(outseq->Ftquery))
    {
/*	seqClone(outseq, seq);	*/    /* already cloned feature table */
	ajDebug("seqWriteListAppend after seqClone Count: %d\n",
		ajFeattableSize(outseq->Fttable));
	if(!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("seqWriteListAppend features output needed table\n");
	    
	    ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);
	    if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("seqWriteListAppend features output "
		       "failed to open UFO '%S'",
		       outseq->Ufo);
		return;
	    }
	    ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
	    ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);
	}

	ajDebug("seqWriteListAppend after ajFeattabOutOpen Count: %d\n",
		ajFeattableSize(outseq->Fttable));
	ajFeattableTrace(outseq->Fttable);

	if(!ajFeatUfoWrite(outseq->Fttable, outseq->Ftquery, outseq->Ufo))
	{
	    ajWarn("seqWriteListAppend features output failed UFO: '%S'",
		   outseq->Ufo);
	    return;
	}
	    
	seqDeclone(outseq);
    }
    
    return;
}




/* @func ajSeqWriteClose ******************************************************
**
** Close a sequence output file. For formats that save everything up
** and write at the end, call the Write function first.
**
** @param [u] outseq [AjPSeqout] Sequence output
** @return [void]
** @@
******************************************************************************/

void ajSeqWriteClose(AjPSeqout outseq)
{

    ajDebug("ajSeqWriteClose '%F'\n", outseq->File);

    if(seqOutFormat[outseq->Format].Save)
    {
	/* Calling funclist seqOutFormat() */
	seqOutFormat[outseq->Format].Write(outseq);
    }

    if(outseq->Knownfile)
	outseq->File = NULL;
    else
	ajFileClose(&outseq->File);

    return;
}




/* @func ajSeqWrite ***********************************************************
**
** Write a sequence out. For formats that save everything up
** and write at the end, just append to the output list.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @param [r] seq [const AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeqout] Master sequence output routine
** @category output [AjPSeq] Master sequence output routine
** @@
******************************************************************************/

void ajSeqWrite(AjPSeqout outseq, const AjPSeq seq)
{
    
    if(!outseq->Format)
	if(!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format))
	    ajErr("unknown output format '%S'", outseq->Formatstr);
    
    ajDebug("ajSeqWrite %d '%s' single: %B feat: %B Save: %B\n",
	    outseq->Format,
	    seqOutFormat[outseq->Format].Name,
	    seqOutFormat[outseq->Format].Single,
	    outseq->Features,
	    seqOutFormat[outseq->Format].Save);
    
    ajDebug(" outseq '%S' seq '%S' '%S'\n",
	    outseq->Name, seq->Name, seq->Entryname);

    seqClone(outseq, seq);
    
    if(seqOutFormat[outseq->Format].Save)
    {
	seqWriteListAppend(outseq, seq);
	outseq->Count++;
	return;
    }
    
    ajSeqoutDefName(outseq, outseq->Entryname, !outseq->Single);
    if(outseq->Fttable)
	ajFeatDefName(outseq->Fttable, outseq->Name);
    
    if(outseq->Single)
	seqFileReopen(outseq);
    
    if (outseq->Knownfile && !outseq->File)
	outseq->File = outseq->Knownfile;
    
    /* Calling funclist seqOutFormat() */
    seqOutFormat[outseq->Format].Write(outseq);
    outseq->Count++;

    ajDebug("ajSeqWrite tests features %B tabouitisopen %B UfoLocal %B\n",
	    outseq->Features, ajFeattabOutIsOpen(outseq->Ftquery),
	    seqoutUfoLocal(outseq));
    if(outseq->Features && 
       !ajFeattabOutIsLocal(outseq->Ftquery))
    {
	if(!ajFeattabOutIsOpen(outseq->Ftquery))
	{
	    ajDebug("ajSeqWrite features output needed\n");
	    ajFeattabOutSetBasename(outseq->Ftquery, outseq->Filename);
	    if(!ajFeattabOutOpen(outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("ajSeqWrite features output failed to open UFO '%S'",
		       outseq->Ufo);
		return;
	    }
	    ajStrAssignEmptyS(&outseq->Ftquery->Seqname, seq->Name);
	    ajStrAssignEmptyS(&outseq->Ftquery->Type, seq->Type);

	    /* ajFeattableTrace(outseq->Fttable); */

	    if(!ajFeatUfoWrite(outseq->Fttable,
			       outseq->Ftquery, outseq->Ufo))
	    {
		ajWarn("ajSeqWrite features output failed UFO: '%S'",
		       outseq->Ufo);
		return;
	    }
	}
    }

    seqDeclone(outseq);
    
    return;
}




/* @funcstatic seqWriteFasta **************************************************
**
** Writes a sequence in FASTA format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteFasta(AjPSeqout outseq)
{
    ajint i;
    ajint ilen;
    AjPStr seq = NULL;
    ajint linelen     = 60;
    ajint iend;

    seqDbName(&outseq->Name, outseq->Setdb);

    ajFmtPrintF(outseq->File, ">%S", outseq->Name);

    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, " %S", outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajFmtPrintF(outseq->File, " %S", outseq->Acc);

    /* no need to bother with outseq->Gi because we have Sv anyway */

    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, " %S", outseq->Desc);
    ajFmtPrintF(outseq->File, "\n");

    ilen = ajStrGetLen(outseq->Seq);
    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajFmtPrintF(outseq->File, "%S\n", seq);
    }

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWriteNcbi ***************************************************
**
** Writes a sequence in NCBI format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNcbi(AjPSeqout outseq)
{

    ajint i;
    ajint ilen;
    AjPStr seq = NULL;
    ajint linelen     = 60;
    ajint iend;

    if(ajStrGetLen(outseq->Gi))
	ajFmtPrintF(outseq->File, ">gi|%S|gnl|", outseq->Gi);
    else
	ajFmtPrintF(outseq->File, ">gnl|");

    if(ajStrGetLen(outseq->Setdb))
	ajFmtPrintF(outseq->File, "%S|", outseq->Setdb);
    else if(ajStrGetLen(outseq->Db))
	ajFmtPrintF(outseq->File, "%S|", outseq->Db);
    else
	ajFmtPrintF(outseq->File, "unk|");

    ajFmtPrintF(outseq->File, "%S", outseq->Name);

    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, " (%S)", outseq->Sv);
    else if(ajStrGetLen(outseq->Acc))
	ajFmtPrintF(outseq->File, " (%S)", outseq->Acc);

    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, " %S", outseq->Desc);

    ajFmtPrintF(outseq->File, "\n");

    ilen = ajStrGetLen(outseq->Seq);
    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajFmtPrintF(outseq->File, "%S\n", seq);
    }

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWriteGcg ****************************************************
**
** Writes a sequence in GCG format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGcg(AjPSeqout outseq)
{

    ajint ilen;
    char ctype = 'N';
    ajint check;
    SeqPSeqFormat sf = NULL;

    ilen = ajStrGetLen(outseq->Seq);

    if(!outseq->Type)
	ajFmtPrintF(outseq->File, "!!NA_SEQUENCE 1.0\n\n");
    else if(ajStrMatchC(outseq->Type, "P"))
    {
	ajFmtPrintF(outseq->File, "!!AA_SEQUENCE 1.0\n\n");
	ctype = 'P';
    }
    else
	ajFmtPrintF(outseq->File, "!!NA_SEQUENCE 1.0\n\n");

    ajSeqGapS(&outseq->Seq, '.');
    check = ajSeqoutCheckGcg(outseq);

    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "%S\n\n", outseq->Desc);

    ajFmtPrintF(outseq->File,
		"%S  Length: %d  Type: %c  Check: %4d ..\n",
		outseq->Name, ilen, ctype, check);

    if(sf)
	seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    else
    {
	seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
	sf->spacer = 11;
	sf->numleft = ajTrue;
	sf->skipbefore = ajTrue;
	strcpy(sf->endstr, "\n");   /* to help with misreads at EOF */
    }

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteStaden *************************************************
**
** Writes a sequence in Staden format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteStaden(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "<%S---->\n", outseq->Name);
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);

    sf->width = 60;
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteText ***************************************************
**
** Writes a sequence in plain Text format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteText(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteHennig86 ***********************************************
**
** Writes a sequence in Hennig86 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteHennig86(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    char* cp;
    
    ajDebug("seqWriteHennig86 list size %d\n",
	    ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;
    
    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    ajFmtPrintF(outseq->File,		/* header text */
		"xread\n");
    
    ajFmtPrintF(outseq->File,		/* title text */
		"' Written by EMBOSS %D '\n", ajTimeTodayRef());
    
    ajFmtPrintF(outseq->File,		/* length, count */
		"%d %d\n", ilen, isize);
    
    for(i=0; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	
	cp = ajStrGetuniquePtr(&sseq);
	while(*cp)
	{
	    switch(*cp)
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
	ajFmtPrintF(outseq->File,
		    "%S\n%S\n",
		    seq->Name, sseq);
    }
    
    ajFmtPrintF(outseq->File,		/* terminate with ';' */
		";\n", ilen, isize);
    
    return;
}




/* @funcstatic seqWriteMega ***************************************************
**
** Writes a sequence in Mega format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMega(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;

    ajDebug("seqWriteMega list size %d\n", ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;

    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"#mega\n");
    ajFmtPrintF(outseq->File,		/* dummy title */
		"TITLE: Written by EMBOSS %D\n", ajTimeTodayRef());

    for(ipos=1; ipos <= ilen; ipos += wid)
    {
	/* interleaved */
	iend = ipos + wid -1;
	if(iend > ilen)
	    iend = ilen;

	ajFmtPrintF(outseq->File,	/* blank space for comments */
		    "\n");
	for(i=0; i < isize; i++)
	{
	    /* loop over sequences */
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajFmtPrintF(outseq->File, "#%-20.20S %S\n", seq->Name, sseq);
	}
    }

    return;
}




/* @funcstatic seqWriteMeganon ************************************************
**
** Writes a sequence in Mega non-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMeganon(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;

    ajDebug("seqWriteMeganon list size %d\n", ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;

    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"#mega\n");
    ajFmtPrintF(outseq->File,		/* dummy title */
		"TITLE: Written by EMBOSS %D\n", ajTimeTodayRef());
    ajFmtPrintF(outseq->File,		/* blank space for comments */
		"\n");

    for(i=0; i < isize; i++)
    {					/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	ajFmtPrintF(outseq->File,
		    "#%-20.20S\n%S\n",
		    seq->Name, sseq);
    }

    return;
}




/* @funcstatic seqWriteNexus **************************************************
**
** Writes a sequence in Nexus interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNexus(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;
    
    ajDebug("seqWriteNexus list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;
    
    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	ajSeqGapLen(seq, '-', '-', ilen); /* need to pad if any are shorter */
    }
    
    ajFmtPrintF(outseq->File,		/* header text */
		"#NEXUS\n");
    ajFmtPrintF(outseq->File,		/* dummy title */
		"[TITLE: Written by EMBOSS %D]\n\n", ajTimeTodayRef());
    ajFmtPrintF(outseq->File,
		"begin data;\n");
    ajFmtPrintF(outseq->File,		/* count, length */
		"dimensions ntax=%d nchar=%d;\n", isize, ilen);
    ajDebug("seqWriteNexus outseq->Type '%S'\n", outseq->Type);
    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "format interleave datatype=protein missing=X gap=-;\n");
    else
	ajFmtPrintF(outseq->File,
		    "format interleave datatype=DNA missing=N gap=-;\n");

    ajFmtPrintF(outseq->File, "\n");
    
    ajFmtPrintF(outseq->File,
		"matrix\n");
    for(ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos +wid -1;
	if(iend > ilen)
	    iend = ilen;

	if(ipos > 1)
	    ajFmtPrintF(outseq->File, "\n");

	for(i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajFmtPrintF(outseq->File,
			"%-20.20S %S\n",
			seq->Name, sseq);
	}
    }
    
    ajFmtPrintF(outseq->File,
		";\n\n");
    ajFmtPrintF(outseq->File,
		"end;\n");
    ajFmtPrintF(outseq->File,
		"begin assumptions;\n");
    ajFmtPrintF(outseq->File,
		"options deftype=unord;\n");
    ajFmtPrintF(outseq->File,
		"end;\n");
    return;
}




/* @funcstatic seqWriteNexusnon ***********************************************
**
** Writes a sequence in Nexus non-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNexusnon(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;

    ajDebug("seqWriteNexusnon list size %d\n",
	    ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;

    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"#NEXUS\n");
    ajFmtPrintF(outseq->File,		/* dummy title */
		"[TITLE: Written by EMBOSS %D]\n\n", ajTimeTodayRef());
    ajFmtPrintF(outseq->File,
		"begin data;\n");
    ajFmtPrintF(outseq->File,		/* count, length */
		"dimensions ntax=%d nchar=%d;\n", isize, ilen);
    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "format datatype=protein missing=X gap=-;\n");
    else
	ajFmtPrintF(outseq->File,
		    "format datatype=DNA missing=N gap=-;\n");

    ajFmtPrintF(outseq->File, "\n");

    ajFmtPrintF(outseq->File,
		"matrix\n");

    for(i=0; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	ajFmtPrintF(outseq->File,
		    "%S\n%S\n",
		    seq->Name, sseq);
    }

    ajFmtPrintF(outseq->File,
		";\n\n");
    ajFmtPrintF(outseq->File,
		"end;\n");
    ajFmtPrintF(outseq->File,
		"begin assumptions;\n");
    ajFmtPrintF(outseq->File,
		"options deftype=unord;\n");
    ajFmtPrintF(outseq->File,
		"end;\n");
    return;
}




/* @funcstatic seqWriteJackknifer *********************************************
**
** Writes a sequence in Jackknifer format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteJackknifer(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;
    static AjPStr tmpid = NULL;

    ajDebug("seqWriteJackknifer list size %d\n",
	    ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;

    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"' Written by EMBOSS %D \n", ajTimeTodayRef());

    for(ipos=1; ipos <= ilen; ipos += wid)
    {					/* interleaved */
	iend = ipos +wid -1;
	if(iend > ilen)
	    iend = ilen;

	for(i=0; i < isize; i++)
	{				/* loop over sequences */
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajFmtPrintS(&tmpid, "(%S)", seq->Name);
	    ajFmtPrintF(outseq->File,
			"%-20.20S %S\n",
			tmpid, sseq);
	}
    }

    ajFmtPrintF(outseq->File, ";\n");

    return;
}




/* @funcstatic seqWriteJackknifernon ******************************************
**
** Writes a sequence in Jackknifer on-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteJackknifernon(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint wid = 50;
    static AjPStr tmpid = NULL;

    ajDebug("seqWriteJackknifernon list size %d\n",
	    ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;

    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* header text */
		"' Written by EMBOSS %D \n", ajTimeTodayRef());

    for(i=0; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	for(ipos=1; ipos <= ilen; ipos += wid)
	{				/* interleaved */
	    iend = ipos +wid -1;
	    if(iend > ilen)
		iend = ilen;

	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    if(ipos == 1)
	    {
		ajFmtPrintS(&tmpid, "(%S)", seq->Name);
		ajFmtPrintF(outseq->File,
			    "%-20.20S %S\n",
			    tmpid, sseq);
	    }
	    else
		ajFmtPrintF(outseq->File,
			    "%S\n",
			    sseq);
	}
    }

    ajFmtPrintF(outseq->File, ";\n");

    return;
}




/* @funcstatic seqWriteTreecon ************************************************
**
** Writes a sequence in Treecon format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteTreecon(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;

    ajDebug("seqWriteTreecon list size %d\n", ajListLength(outseq->Savelist));

    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;

    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }

    ajFmtPrintF(outseq->File,		/* count */
		"%d\n", ilen);

    for(i=0; i < isize; i++)
    {
	/* loop over sequences */
	seq = seqarr[i];
	ajStrAssignS(&sseq, seq->Seq);
	ajSeqGapS(&sseq, '-');
	ajFmtPrintF(outseq->File,
		    "%S\n%S\n",
		    seq->Name, sseq);
    }

    return;
}




/* @funcstatic seqWriteClustal ************************************************
**
** Writes a sequence in Clustal (ALN) format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteClustal(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    ajint iwidth = 50;
    
    ajDebug("seqWriteClustal list size %d\n", ajListLength(outseq->Savelist));
    
    
    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;
    
    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen > ajSeqGetLen(seq))
	    ajSeqFill(seq, ilen);
    }
    
    ajFmtPrintF(outseq->File,
		"CLUSTAL W (1.83) multiple sequence alignment\n");
    
    ajFmtPrintF(outseq->File, "\n\n");
    
    iwidth = 60;
    for(ipos=1; ipos <= ilen; ipos += 60)
    {
	iend = ipos + 60 -1;
	if(iend > ilen)
	{
	    iend = ilen;
	    iwidth = ilen - ipos + 1;
	}

	for(i=0; i < isize; i++)
	{
	    seq = seqarr[i];
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    /* clustalw no longer uses blocks of 10 - after version 1.4 */
	    /*ajStrFmtBlock(&sseq, 10);*/ 
	    ajFmtPrintF(outseq->File,
			"%-15.15S %S\n",
			seq->Name, sseq);
	}
	ajFmtPrintF(outseq->File,	/* *. conserved line */
		    "%-15.15s %*.*s\n", "", iwidth, iwidth, "");
	if(iend < ilen)
	    ajFmtPrintF(outseq->File, "\n");
    }
    
    return;
}




/* @funcstatic seqWriteSelex **************************************************
**
** Writes a sequence in Selex format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSelex(AjPSeqout outseq)
{
    ajint n;
    ajint len = 0;
    ajint i   = 0;
    ajint j   = 0;
    
    AjPSeq seq   = NULL;
    AjPSeq* seqs = NULL;
    ajint test;
/*
    ajint k   = 0;
    ajint namelen = 0;
    ajint v       = 0;
    AjBool sep    = ajFalse;
*/
    AjPStr rfstr  = NULL;
    AjPStr csstr  = NULL;
    AjPStr ssstr  = NULL;
    const char *p       = NULL;
    AjPStr *names;
    ajint  extra;
    ajint  nlen   = 0;
    ajint  slen   = 0;
    AjPStr *aseqs = NULL;
    
    ajDebug("seqWriteSelex list size %d\n", ajListLength(outseq->Savelist));
    
    rfstr = ajStrNewC("#=RF");
    csstr = ajStrNewC("#=CS");
    ssstr = ajStrNewC("#=SS");
    
    n = ajListLength(outseq->Savelist);
    if(!n)
	return;
    
    test = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", test);
    
    
    
    for(i=0; i < n; ++i)
    {
	seq = seqs[i];
	if(len < ajSeqGetLen(seq))
	    len = ajSeqGetLen(seq);
    }
    /*
    sdata = seqs[0]->Selexdata;
    if(sdata)
    {
	
	if(ajStrGetLen(sdata->id))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=ID %S\n",sdata->id);
	}

	if(ajStrGetLen(sdata->ac))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=AC %S\n",sdata->ac);
	}

	if(ajStrGetLen(sdata->de))
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
	
	if(ajStrGetLen(sdata->au))
	{
	    sep=ajTrue;
	    ajFmtPrintF(outseq->File,"#=AU %S\n",sdata->au);
	}
	
	if(sep)
	    ajFmtPrintF(outseq->File,"\n");
	
	
	v=4;
	for(i=0;i<n;++i)
	{
	    v = ajStrGetLen(seqs[i]->Selexdata->sq->name);
	    namelen = (namelen > v) ? namelen : v;
	}

	for(i=0;i<n;++i)
	{
	    v = namelen - ajStrGetLen(seqs[i]->Selexdata->sq->name);
	    for(j=0;j<v;++j)
		ajStrAppendK(&seqs[i]->Selexdata->sq->name,' ');
	}
	
	
	if(ajStrGetLen(sdata->sq->ac))
	    for(i=0;i<n;++i)
	    {
		qdata = seqs[i]->Selexdata->sq;
		ajFmtPrintF(outseq->File,"#=SQ %S %.2f %S %S %d..%d:%d %S\n",
			    qdata->name,qdata->wt,qdata->source,qdata->ac,
			    qdata->start,qdata->stop,qdata->len,qdata->de);
	    }
	ajFmtPrintF(outseq->File,"\n");
	
	
	
	if(ajStrGetLen(seqs[0]->Selexdata->rf))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppendK(&rfstr,' ');
	}

	if(ajStrGetLen(seqs[0]->Selexdata->cs))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppendK(&csstr,' ');
	}
	if(ajStrGetLen(seqs[0]->Selexdata->ss))
	{
	    v = namelen - 4;
	    for(k=0;k<v;++k)
		ajStrAppendK(&ssstr,' ');
	}
	
	
	
	for(i=0;i<len;i+=50)
	{
	    if(ajStrGetLen(seqs[0]->Selexdata->rf))
	    {
		p = ajStrGetPtr(seqs[0]->Selexdata->rf);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",rfstr,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",rfstr,
				&p[i]);
	    }

	    if(ajStrGetLen(seqs[0]->Selexdata->cs))
	    {
		p = ajStrGetPtr(seqs[0]->Selexdata->cs);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",csstr,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",csstr,
				&p[i]);
	    }


	    for(j=0;j<n;++j)
	    {
		sdata = seqs[j]->Selexdata;

		p = ajStrGetPtr(sdata->str);
		if(i+50>=len)
		    ajFmtPrintF(outseq->File,"%S %s\n",sdata->sq->name,&p[i]);
		else
		    ajFmtPrintF(outseq->File,"%S %-50.50s\n",sdata->sq->name,
				&p[i]);

		if(ajStrGetLen(seqs[0]->Selexdata->ss))
		{
		    p = ajStrGetPtr(seqs[0]->Selexdata->ss);
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
    else	/ * Wasn't originally Selex format * /
    {
*/
	AJCNEW0(aseqs,n);
	AJCNEW0(names,n);
	for(i=0; i < n; ++i)
	{
	    seq = seqs[i];
	    aseqs[i] = ajStrNew();
	    names[i] = ajStrNew();
	    ajStrAssignS(&names[i],seq->Name);
	    if((len=ajStrGetLen(names[i])) > nlen)
		nlen = len;
	    if((len=ajStrGetLen(seq->Seq)) > slen)
		slen = len;
	    ajStrAssignS(&aseqs[i],seq->Seq);
	}

	for(i=0;i<n;++i)
	{
	    seq = seqs[i];
	    extra = nlen - ajStrGetLen(names[i]);
	    for(j=0;j<extra;++j)
		ajStrAppendK(&names[i],' ');
	    extra = slen - ajStrGetLen(seq->Seq);
	    for(j=0;j<extra;++j)
		ajStrAppendK(&aseqs[i],' ');

	    ajFmtPrintF(outseq->File,"#=SQ %S %.2f - - 0..0:0 ",
			names[i],seq->Weight);
	    if(ajStrGetLen(seq->Desc))
		ajFmtPrintF(outseq->File,"%S\n",seq->Desc);
	    else
		ajFmtPrintF(outseq->File,"-\n");
	}
	ajFmtPrintF(outseq->File,"\n");


	for(i=0;i<slen;i+=50)
	{
	    for(j=0;j<n;++j)
	    {
		p = ajStrGetPtr(aseqs[j]);
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
/*
    }
  */  
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
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMsf(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    void** seqs = NULL;
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
    ajint maxnamelen = 10;
    
    ajDebug("seqWriteMsf list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;
    
    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    maxnamelen = 10;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
	if (ajStrGetLen(seq->Name) > maxnamelen)
	    maxnamelen = ajStrGetLen(seq->Name);
    }
    
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	ajSeqGapLen(seq, '.', '~', ilen); /* need to pad if any are shorter */
	check = ajSeqCalcCheckgcg(seq);
	ajDebug(" '%S' len: %d checksum: %d\n",
		ajSeqGetNameS(seq), ajSeqGetLen(seq), check);
	checktot += check;
	checktot = checktot % 10000;
    }
    
    ajDebug("checksum %d\n", checktot);
    ajDebug("outseq->Type '%S'\n", outseq->Type);
    
    if(!ajStrGetLen(outseq->Type))
    {
	ajSeqType(seqarr[0]);
	ajStrAssignEmptyS(&outseq->Type, seqarr[0]->Type);
    }
    ajDebug("outseq->Type '%S'\n", outseq->Type);
    
    if(ajStrMatchC(outseq->Type, "P"))
    {
	ajFmtPrintF(outseq->File, "!!AA_MULTIPLE_ALIGNMENT 1.0\n\n");
	ajFmtPrintF(outseq->File,
		    "  %F MSF:  %d Type: P %D CompCheck: %4d ..\n\n",
		    outseq->File, ilen, ajTimeTodayRef(), checktot);
    }
    else
    {
	ajFmtPrintF(outseq->File, "!!NA_MULTIPLE_ALIGNMENT 1.0\n\n");
	ajFmtPrintF(outseq->File,
		    "  %F MSF: %d Type: N %D CompCheck: %4d ..\n\n",
		    outseq->File, ilen, ajTimeTodayRef(), checktot);
    }
    
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	check = ajSeqCalcCheckgcg(seq);
	ajFmtPrintF(outseq->File,
		    "  Name: %-*S Len: %d  Check: %4d Weight: %.2f\n",
		    maxnamelen, seq->Name, ajStrGetLen(seq->Seq),
		    check, seq->Weight);
    }
    
    ajFmtPrintF(outseq->File, "\n//\n\n");
    
    for(ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;
	if(iend > ilen)
	    iend = ilen;
	ajFmtPrintS(&sbeg, "%d", ipos);
	ajFmtPrintS(&send, "%d", iend);
	if(iend == ilen) {
	    igap = iend - ipos - ajStrGetLen(sbeg);
	    ajDebug("sbeg: %S send: %S ipos: %d iend: %d igap: %d len: %d\n",
		    sbeg, send, ipos, iend, igap, ajStrGetLen(send));
	    if(igap >= ajStrGetLen(send))
		ajFmtPrintF(outseq->File,
			    "%*s %S %*S\n", maxnamelen, " ", sbeg, igap, send);
	    else
		ajFmtPrintF(outseq->File, "           %S\n", sbeg);
	}
	else
	    ajFmtPrintF(outseq->File, "           %-25S%25S\n",
			sbeg, send);
	for(i=0; i < isize; i++)
	{
	    seq = seqarr[i];
	    check = ajSeqCalcCheckgcg(seq);
	    ajStrAssignSubS(&sseq, seq->Seq, ipos-1, iend-1);
	    ajFmtPrintF(outseq->File,
			"%-*S %S\n",
			maxnamelen, seq->Name, sseq);
	}
	ajFmtPrintF(outseq->File, "\n");
    }
    
    
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
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteCodata(AjPSeqout outseq)
{

    static SeqPSeqFormat sf = NULL;
    ajint j;

    ajFmtPrintF(outseq->File, "ENTRY           %S \n", outseq->Name);
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "TITLE           %S, %d bases\n",
		    outseq->Desc, ajStrGetLen(outseq->Seq));
    if(ajStrGetLen(outseq->Acc))
	ajFmtPrintF(outseq->File, "ACCESSION       %S\n",
		    outseq->Acc);
    ajFmtPrintF(outseq->File, "SEQUENCE        \n");

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->numwidth = 7;
    sf->width = 30;
    sf->numleft = ajTrue;
    sf->spacer = seqSpaceAll;
    strcpy(sf->endstr, "\n///");

    for(j = 0; j <= sf->numwidth; j++)
	ajFmtPrintF(outseq->File, " ");
    for(j = 5; j <= sf->width; j+=5)
	ajFmtPrintF(outseq->File, "%10d", j);
    ajFmtPrintF(outseq->File, "\n");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteNbrf ***************************************************
**
** Writes a sequence in NBRF format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNbrf(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    static AjPStr ftfmt     = NULL;

    if(!ftfmt)
	ajStrAssignC(&ftfmt, "pir");

    if(!outseq->Type)
	ajFmtPrintF(outseq->File, ">D1;%S\n", outseq->Name);
    else if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, ">P1;%S\n", outseq->Name);
    else
	ajFmtPrintF(outseq->File, ">D1;%S\n", outseq->Name);

    ajFmtPrintF(outseq->File, "%S, %d bases\n",
		outseq->Desc, ajStrGetLen(outseq->Seq));

    if(seqoutUfoLocal(outseq))
    {
	outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteNbrf features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->spacer = 11;
    strcpy(sf->endstr, "*\n");
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteExperiment *********************************************
**
** Writes a sequence in Staden experiment format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteExperiment(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajint b[5];
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    ajint i;
    ajint j;
    ajint jend;
    
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "embl");
    
    if(ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss(outseq);
	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S standard; DNA; UNC; %d BP.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }

	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);

	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ";\n", cur);
    }
    
    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, "SV   %S\n", outseq->Sv);
    
    /* no need to bother with outseq->Gi because Staden doesn't use it */
    
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "EX   %S\n", outseq->Desc);
    
    if(ajListLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListLength(outseq->Taxlist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Taxlist);
	cur = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(seqoutUfoLocal(outseq))
    {
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteEmbl features output failed UFO: '%S'",
		   outseq->Ufo);
    }
    

    if(outseq->Accuracy)
    {
	ilen = ajStrGetLen(outseq->Seq);
	for(i=0; i<ilen;i+=20)
	{
	    ajFmtPrintF(outseq->File, "AV  ");
	    jend = i+20;
	    if(jend > ilen)
		jend = ilen;
	    for(j=i;j<jend;j++)
		ajFmtPrintF(outseq->File, " %2d", outseq->Accuracy[j]);
	    ajFmtPrintF(outseq->File, "\n");
	}
    }

    ajSeqoutCount(outseq, b);
    ajFmtPrintF(outseq->File,
		"SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
		ajStrGetLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    return;
}




/* @funcstatic seqWriteEmbl ***************************************************
**
** Writes a sequence in EMBL format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteEmbl(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajint b[5];
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    AjPStr tmpstr = NULL;
    const AjPStr tmpline = NULL;
    
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "embl");
    
    if(ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss(outseq);
	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S standard; DNA; UNC; %d BP.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }

	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);

	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ";\n", cur);
    }
    
    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, "SV   %S\n", outseq->Sv);
    
    /* no need to bother with outseq->Gi because EMBL doesn't use it */
    
    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);
	ajStrFmtWrap(&tmpstr, 75);
	tmpline = ajStrParseC(tmpstr, "\n");
	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "DE   %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListLength(outseq->Taxlist) > 1)
    {
	ilen=0;
	it = ajListIterRead(outseq->Taxlist);
	cur = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(seqoutUfoLocal(outseq))
    {
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteEmbl features output failed UFO: '%S'",
		   outseq->Ufo);
    }
    
    ajSeqoutCount(outseq, b);
    ajFmtPrintF(outseq->File,
		"SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
		ajStrGetLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic seqWriteEmblnew ************************************************
**
** Writes a sequence in new EMBL forma, introduced in EMBL release 87t.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteEmblnew(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajint b[5];
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    ajint i;
    AjPStr idstr = NULL;
    AjPStr svstr = NULL;
    AjPStr tmpstr = NULL;
    const AjPStr tmpline = NULL;

    if(!ftfmt)
	ajStrAssignC(&ftfmt, "embl");
    
    if(ajStrMatchC(outseq->Type, "P"))
    {
	seqWriteSwiss(outseq);
	return;
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	ajStrAssignS(&svstr, outseq->Sv);
	i = ajStrFindC(svstr, ".");
	if(i >= 0)
	    ajStrCutStart(&svstr, i+1);
    }
    else
       ajStrAssignC(&svstr, "1");

    if(ajStrGetLen(outseq->Acc))
	ajStrAssignS(&idstr, outseq->Acc);
    else
	ajStrAssignS(&idstr, outseq->Name);

    ajFmtPrintF(outseq->File,
		"ID   %S; SV %S; linear; DNA; STD; UNC; %d BP.\n",
		idstr, svstr, ajStrGetLen(outseq->Seq));
    ajStrDel(&svstr);
    
    if(ajListLength(outseq->Acclist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n");
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ");
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ");
		ilen += 2;
	    }

	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);

	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ";\n", cur);
    }

    /* no SV line in the new format - see the ID line */
    /*
    if(ajStrGetLen(outseq->Sv))
	ajFmtPrintF(outseq->File, "SV   %S\n", outseq->Sv);
    */
    
    /* no need to bother with outseq->Gi because EMBL doesn't use it */
    
    if(ajStrGetLen(outseq->Desc))
    {
	ajStrAssignS(&tmpstr,  outseq->Desc);
	ajStrFmtWrap(&tmpstr, 75);
	tmpline = ajStrParseC(tmpstr, "\n");
	while (tmpline)
	{
	    ajFmtPrintF(outseq->File, "DE   %S\n", tmpline);
	    tmpline = ajStrParseC(NULL, "\n");
	}
    }

    if(ajListLength(outseq->Keylist))
    {
	ilen=0;
	it = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListLength(outseq->Taxlist) > 1)
    {
	ilen=0;
	it = ajListIterRead(outseq->Taxlist);
	cur = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(seqoutUfoLocal(outseq))
    {
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteEmbl features output failed UFO: '%S'",
		   outseq->Ufo);
    }
    
    ajSeqoutCount(outseq, b);
    ajFmtPrintF(outseq->File,
		"SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
		ajStrGetLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    sf->numright = ajTrue;
    sf->numwidth = 9;
    sf->numjust = ajTrue;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic seqWriteSwiss **************************************************
**
** Writes a sequence in SWISSPROT format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSwiss(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajint mw;
    /*  ajuint crc; old 32-bit crc */
    unsigned long long crc;
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "swiss");
    
    if(ajStrMatchC(outseq->Type, "N"))
    {
	seqWriteEmbl(outseq);
	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S     STANDARD;      PRT; %5d AA.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListLength(outseq->Acclist))
    {
	ilen = 0;
	it = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }

	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);

	}
	
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ";\n", cur);
    }
    
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "DE   %S\n", outseq->Desc);
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListLength(outseq->Taxlist) > 1)
    {
	ilen = 0;
	it   = ajListIterRead(outseq->Taxlist);
	cur  = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(ajListLength(outseq->Keylist))
    {
	ilen = 0;
	it   = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(seqoutUfoLocal(outseq))
    {
	outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteSwiss features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    /*  crc = ajSeqstrCalcCrc(outseq->Seq);    old 32-bit crc*/
    crc = ajSp64Crc(outseq->Seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(outseq->Seq));
    
    /* old 32-bit crc
       ajFmtPrintF(outseq->File,
       "SQ   SEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
       ajStrGetLen(outseq->Seq), mw, crc);
       */
    
    ajFmtPrintF(outseq->File,
		"SQ   SEQUENCE %5d AA; %6d MW;  %08X",
		ajStrGetLen(outseq->Seq), mw, (crc>>32)&0xffffffff);
    ajFmtPrintF(outseq->File,
		"%08X CRC64;\n",crc&0xffffffff);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    return;
}




/* @funcstatic seqWriteSwissnew ***********************************************
**
** Writes a sequence in SWISSPROT/UNIPROT format, revised in September 2006
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSwissnew(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    ajint mw;
    /*  ajuint crc; old 32-bit crc */
    unsigned long long crc;
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "swiss");
    
    if(ajStrMatchC(outseq->Type, "N"))
    {
	seqWriteEmbl(outseq);
	return;
    }
    
    ajFmtPrintF(outseq->File,
		"ID   %-10S     Unreviewed;    %7d AA.\n",
		outseq->Name, ajStrGetLen(outseq->Seq));
    
    if(ajListLength(outseq->Acclist))
    {
	ilen = 0;
	it = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "AC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }

	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);

	}
	
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ";\n", cur);
    }
    
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "DE   %S\n", outseq->Desc);
    
    if(ajStrGetLen(outseq->Tax))
	ajFmtPrintF(outseq->File, "OS   %S\n", outseq->Tax);
    
    if(ajListLength(outseq->Taxlist) > 1)
    {
	ilen = 0;
	it   = ajListIterRead(outseq->Taxlist);
	cur  = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "OC   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}
	
	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(ajListLength(outseq->Keylist))
    {
	ilen = 0;
	it   = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KW   ", cur);
		ilen = 6;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(seqoutUfoLocal(outseq))
    {
	outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteSwiss features output failed UFO: '%S'",
		   outseq->Ufo);
    }

    /*  crc = ajSeqstrCalcCrc(outseq->Seq);    old 32-bit crc*/
    crc = ajSp64Crc(outseq->Seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(outseq->Seq));
    
    /* old 32-bit crc
       ajFmtPrintF(outseq->File,
       "SQ   SEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
       ajStrGetLen(outseq->Seq), mw, crc);
       */
    
    ajFmtPrintF(outseq->File,
		"SQ   SEQUENCE %5d AA; %6d MW;  %08X",
		ajStrGetLen(outseq->Seq), mw, (crc>>32)&0xffffffff);
    ajFmtPrintF(outseq->File,
		"%08X CRC64;\n",crc&0xffffffff);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 4;
    sf->spacer = 11;
    sf->width = 60;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    return;
}




/* @funcstatic seqWriteGenbank ************************************************
**
** Writes a sequence in GENBANK format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGenbank(AjPSeqout outseq)
{
    
    static SeqPSeqFormat sf = NULL;
    ajint b[5];
    static AjPStr ftfmt = NULL;
    AjIList it;
    AjPStr cur;
    ajint ilen;
    
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "genbank");
    
    ajSeqoutTrace(outseq);
    
    ajFmtPrintF(outseq->File, "LOCUS       %S\n", outseq->Name);
    if(ajStrGetLen(outseq->Desc))
	ajFmtPrintF(outseq->File, "DEFINITION  %S\n", outseq->Desc);

    if(ajListLength(outseq->Acclist))
    {
	ilen = 0;
	it   = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen + ajStrGetLen(cur) > 79)
	    {
		ajFmtPrintF(outseq->File, "\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "ACCESSION   ", cur);
		ilen = 12;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, " ", cur);
		ilen += 1;
	    }

	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);

	}

	ajListIterFree(&it) ;
	if(ilen > 0)
	    ajFmtPrintF(outseq->File, "\n", cur);
    }
    
    if(ajStrGetLen(outseq->Sv))
    {
	if(ajStrGetLen(outseq->Gi))
	    ajFmtPrintF(outseq->File, "VERSION     %S  GI:%S\n",
			outseq->Sv, outseq->Gi);
	else
	    ajFmtPrintF(outseq->File, "VERSION     %S\n", outseq->Sv);
    }
    
    if(ajListLength(outseq->Keylist))
    {
	ilen = 0;
	it = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "KEYWORDS    ", cur);
		ilen = 12;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }
    
    if(ajStrGetLen(outseq->Tax) && ajListLength(outseq->Taxlist))
    {
	ajFmtPrintF(outseq->File, "SOURCE      %S.\n", outseq->Tax);

	ajFmtPrintF(outseq->File, "  ORGANISM  %S\n", outseq->Tax);

	ilen = 0;
	it   = ajListIterRead(outseq->Taxlist);
	cur  = (AjPStr) ajListIterNext(it); /* skip first, should be Tax */
	while((cur = (AjPStr) ajListIterNext(it)))
	{
	    if(ilen+ajStrGetLen(cur) >= 79)
	    {
		ajFmtPrintF(outseq->File, ";\n", cur);
		ilen = 0;
	    }

	    if(ilen == 0)
	    {
		ajFmtPrintF(outseq->File, "            ", cur);
		ilen = 12;
	    }
	    else
	    {
		ajFmtPrintF(outseq->File, "; ", cur);
		ilen += 2;
	    }
	    ajFmtPrintF(outseq->File, "%S", cur);
	    ilen += ajStrGetLen(cur);
	}

	ajListIterFree(&it) ;
	ajFmtPrintF(outseq->File, ".\n", cur);
    }

    if(seqoutUfoLocal(outseq))
    {
        outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteGenbank features output failed UFO: '%S'",
		   outseq->Ufo);
    }
    
    ajSeqoutCount(outseq, b);
    if(b[4])
	ajFmtPrintF(outseq->File,
		    "BASE COUNT   %6d a %6d c %6d g %6d t %6d others\n",
		    b[0], b[1], b[2], b[3], b[4]);
    else
	ajFmtPrintF(outseq->File,
		    "BASE COUNT   %6d a %6d c %6d g %6d t\n",
		    b[0], b[1], b[2], b[3]);
    ajFmtPrintF(outseq->File, "ORIGIN\n");
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");
    sf->tab = 1;
    sf->spacer = 11;
    sf->width = 60;
    sf->numleft = ajTrue;
    sf->numwidth = 8;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    return;
}




/* @funcstatic seqWriteGff ****************************************************
**
** Writes a sequence in GFF format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGff(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    static AjPStr version   = NULL;
    static AjPStr ftfmt     = NULL;
    
    if(!ftfmt)
	ajStrAssignC(&ftfmt, "gff");
    
    if(!version)
	ajNamRootVersion(&version);
    
    ajFmtPrintF(outseq->File,
		"##gff-version 2\n");
    ajFmtPrintF(outseq->File,
		"##source-version EMBOSS %S\n", version);
    ajFmtPrintF(outseq->File,
		"##date %D\n", ajTimeTodayRefF("GFF"));
    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "##Protein %S\n", outseq->Name);
    else
	ajFmtPrintF(outseq->File,
		    "##DNA %S\n", outseq->Name);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    
    strcpy(sf->leftstr, "##");
    sf->width = 60;
    /*
       sf->tab = 4;
       sf->spacer = 11;
       sf->numright = ajTrue;
       sf->numwidth = 9;
       sf->numjust = ajTrue;
       */
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, "##end-Protein\n");
    else
	ajFmtPrintF(outseq->File, "##end-DNA\n");
   
    if(seqoutUfoLocal(outseq))
    {
	outseq->Ftquery = ajFeattabOutNewSSF(ftfmt, outseq->Name,
					     ajStrGetPtr(outseq->Type),
					     outseq->File);
	if(ajStrMatchC(outseq->Type, "P"))
	    ajFeattableSetProt(outseq->Fttable);
	else
	    ajFeattableSetNuc(outseq->Fttable);
	
	if(!ajFeatWrite(outseq->Ftquery, outseq->Fttable))
	    ajWarn("seqWriteGff features output failed UFO: '%S'",
		   outseq->Ufo);

    }
    
    return;
}




/* @funcstatic seqWriteStrider ************************************************
**
** Writes a sequence in DNA STRIDER format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteStrider(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "; ### from DNA Strider ;-)\n");
    ajFmtPrintF(outseq->File, "; DNA sequence  %S, %d bases\n;\n",
		outseq->Name, ajStrGetLen(outseq->Seq));

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n//");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteFitch **************************************************
**
** Writes a sequence in FITCH format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteFitch(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "%S, %d bases\n",
		outseq->Name, ajStrGetLen(outseq->Seq));

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->spacer = 4;
    sf->width  = 60;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteMase **************************************************
**
** Writes a sequence in MASE format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMase(AjPSeqout outseq)
{
    ajint i;
    ajint ilen;
    AjPStr seq = NULL;
    ajint linelen = 60;
    ajint iend;

    if (!ajFileTell(outseq->File))
	ajFmtPrintF(outseq->File, ";;Written by EMBOSS on %D\n",
		ajTimeTodayRefF("report"));

    ajFmtPrintF(outseq->File, ";%S\n",
		outseq->Desc);

    ajFmtPrintF(outseq->File, "%S\n",
		outseq->Name);

    ilen = ajStrGetLen(outseq->Seq);
    for(i=0; i < ilen; i += linelen)
    {
	iend = AJMIN(ilen-1, i+linelen-1);
	ajStrAssignSubS(&seq, outseq->Seq, i, iend);
	ajFmtPrintF(outseq->File, "%S\n", seq);
    }

    ajStrDel(&seq);

    return;
}




/* @funcstatic seqWritePhylip *************************************************
**
** Writes a sequence in PHYLIP interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWritePhylip(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    ajint j    = 0;
    char *p    = NULL;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend;
    AjPStr tstr = NULL;
    
    ajDebug("seqWritePhylip list size %d\n", ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;
    
    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    tstr = ajStrNewRes(ilen+1);
    ajFmtPrintF(outseq->File, " %d %d\n", isize, ilen);
    
    for(ipos=1; ipos <= ilen; ipos += 50)
    {
	iend = ipos + 50 -1;
	if(iend > ilen)
	    iend = ilen;
	
	for(i=0; i < isize; i++)
	{
	    seq = seqarr[i];

	    ajStrAssignC(&tstr,ajStrGetPtr(seq->Seq));
	    p = ajStrGetuniquePtr(&tstr);
	    for(j=ajStrGetLen(tstr);j<ilen;++j)
		*(p+j)='-';
	    *(p+j)='\0';
	    tstr->Len=ilen;
	    ajStrAssignSubS(&sseq, tstr, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajStrFmtBlock(&sseq, 10);
	    if(ipos == 1)
		ajFmtPrintF(outseq->File,
			    "%-10.10S%S\n",
			    seq->Name, sseq);
	    else
		ajFmtPrintF(outseq->File,
			    "%10s%S\n",
			    " ", sseq);
	}

	if(iend < ilen)
	    ajFmtPrintF(outseq->File, "\n");
    }
    
    ajStrDel(&tstr);
    return;
}




/* @funcstatic seqWritePhylipnon **********************************************
**
** Writes a sequence in PHYLIP non-interleaved format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWritePhylipnon(AjPSeqout outseq)
{
    ajint isize;
    ajint ilen = 0;
    ajint i    = 0;
    ajint j    = 0;
    ajint n    = 0;
    char *p = NULL;
    void** seqs = NULL;
    AjPSeq seq;
    AjPSeq* seqarr;
    ajint itest;
    static AjPStr sseq = NULL;
    ajint ipos;
    ajint iend  = 0;
    AjPStr tstr = NULL;
    
    ajDebug("seqWritePhylipnon list size %d\n",
	    ajListLength(outseq->Savelist));
    
    isize = ajListLength(outseq->Savelist);
    if(!isize)
	return;
    
    itest = ajListToArray(outseq->Savelist, (void***) &seqs);
    ajDebug("ajListToArray listed %d items\n", itest);
    seqarr = (AjPSeq*) seqs;
    for(i=0; i < isize; i++)
    {
	seq = seqarr[i];
	if(ilen < ajSeqGetLen(seq))
	    ilen = ajSeqGetLen(seq);
    }
    
    tstr = ajStrNewRes(ilen+1);
    ajFmtPrintF(outseq->File, "%d %d\n", isize, ilen);
    
    for(n=0;n<isize;++n)
    {
	seq = seqarr[n];
	ajStrAssignC(&tstr,ajStrGetPtr(seq->Seq));
	p = ajStrGetuniquePtr(&tstr);
	for(j=ajStrGetLen(tstr);j<ilen;++j)
	    *(p+j)='-';
	*(p+j)='\0';
	tstr->Len=ilen;


	for(ipos=1; ipos <= ilen; ipos += 50)
	{
	    iend = ipos + 50 -1;
	    if(iend > ilen)
		iend = ilen;

	    ajStrAssignSubS(&sseq, tstr, ipos-1, iend-1);
	    ajSeqGapS(&sseq, '-');
	    ajStrFmtBlock(&sseq, 10);
	    if(ipos == 1)
		ajFmtPrintF(outseq->File,
			    "%-10.10S%S\n",
			    seq->Name, sseq);
	    else
		ajFmtPrintF(outseq->File,
			    "%10s%S\n",
			    " ", sseq);
	}

	if(iend < ilen)
	    ajFmtPrintF(outseq->File, "\n");

    }
    
    ajStrDel(&tstr);

    return;
}




/* @funcstatic seqWriteAsn1 ***************************************************
**
** Writes a sequence in ASN.1 format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteAsn1(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, "  seq {\n");
    ajFmtPrintF(outseq->File, "    id { local id 1 },\n");
    ajFmtPrintF(outseq->File, "    descr { title \"%S\" },\n",
		outseq->Desc);
    ajFmtPrintF(outseq->File, "    inst {\n");

    if(!outseq->Type)
	ajFmtPrintF(outseq->File,
		    "      repr raw, mol dna, length %d, "
		    "topology linear,\n {\n",
		    ajStrGetLen(outseq->Seq));
    else if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File,
		    "      repr raw, mol aa, length %d, "
		    "topology linear,\n {\n",
		    ajStrGetLen(outseq->Seq));
    else
	ajFmtPrintF(outseq->File,
		    "      repr raw, mol dna, length %d, "
		    "topology linear,\n",
		    ajStrGetLen(outseq->Seq));

    ajFmtPrintF(outseq->File, "      seq-data\n");

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, "        iupacaa \"");
    else
	ajFmtPrintF(outseq->File, "        iupacna \"");

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->linepos = 17;
    sf->spacer  = 0;
    sf->width   = 78;
    sf->tab     = 0;
    strcpy(sf->endstr, "\"\n      } } ,");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteIg *****************************************************
**
** Writes a sequence in INTELLIGENETICS format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteIg(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    ajFmtPrintF(outseq->File, ";%S, %d bases\n",
			outseq->Desc, ajStrGetLen(outseq->Seq));
    ajFmtPrintF(outseq->File, "%S\n", outseq->Name);

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "1");	/* linear (DNA) */

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteAcedb **************************************************
**
** Writes a sequence in ACEDB format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteAcedb(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;

    if(ajStrMatchC(outseq->Type, "P"))
	ajFmtPrintF(outseq->File, "Peptide : \"%S\"\n", outseq->Name);
    else
	ajFmtPrintF(outseq->File, "DNA : \"%S\"\n", outseq->Name);

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "\n");

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}




/* @funcstatic seqWriteDebug **************************************************
**
** Writes a sequence in debug report format.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteDebug(AjPSeqout outseq)
{
    static SeqPSeqFormat sf = NULL;
    AjIList it;
    AjPStr cur;
    
    ajFmtPrintF(outseq->File, "Sequence output trace\n");
    ajFmtPrintF(outseq->File, "=====================\n\n");
    ajFmtPrintF(outseq->File, "  Name: '%S'\n", outseq->Name);
    ajFmtPrintF(outseq->File, "  Accession: '%S'\n", outseq->Acc);

    if(ajListLength(outseq->Acclist))
    {
	ajFmtPrintF(outseq->File, "  Acclist: (%d)",
		    ajListLength(outseq->Acclist));
	it = ajListIterRead(outseq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajFmtPrintF(outseq->File, " %S\n", cur);

	ajListIterFree(&it);
	ajFmtPrintF(outseq->File, "\n");
    }
    
    ajFmtPrintF(outseq->File, "  SeqVersion: '%S'\n", outseq->Sv);
    ajFmtPrintF(outseq->File, "  GenInfo Id: '%S'\n", outseq->Gi);
    ajFmtPrintF(outseq->File, "  Description: '%S'\n", outseq->Desc);
    if(ajListLength(outseq->Keylist))
    {
	ajFmtPrintF(outseq->File, "  Keywordlist: (%d)\n",
		    ajListLength(outseq->Keylist));
	it = ajListIterRead(outseq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajFmtPrintF(outseq->File, "    '%S'\n", cur);

	ajListIterFree(&it);
    }
    ajFmtPrintF(outseq->File, "  Taxonomy: '%S'\n", outseq->Tax);

    if(ajListLength(outseq->Taxlist))
    {
	ajFmtPrintF(outseq->File, "  Taxlist: (%d)\n",
		    ajListLength(outseq->Taxlist));
	it = ajListIterRead(outseq->Taxlist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajFmtPrintF(outseq->File, "    '%S'\n", cur);

	ajListIterFree(&it);
    }
    ajFmtPrintF(outseq->File, "  Type: '%S'\n", outseq->Type);
    ajFmtPrintF(outseq->File, "  Database: '%S'\n", outseq->Db);
    ajFmtPrintF(outseq->File, "  Full name: '%S'\n", outseq->Full);
    ajFmtPrintF(outseq->File, "  Date: '%S'\n", outseq->Date);
    ajFmtPrintF(outseq->File, "  Usa: '%S'\n", outseq->Usa);
    ajFmtPrintF(outseq->File, "  Ufo: '%S'\n", outseq->Ufo);
    ajFmtPrintF(outseq->File, "  Input format: '%S'\n",
		outseq->Informatstr);
    ajFmtPrintF(outseq->File, "  Output format: '%S'\n",
		outseq->Formatstr);
    ajFmtPrintF(outseq->File, "  Filename: '%S'\n", outseq->Filename);
    ajFmtPrintF(outseq->File, "  Directory: '%S'\n",
		outseq->Directory);
    ajFmtPrintF(outseq->File, "  Entryname: '%S'\n",
		outseq->Entryname);
    ajFmtPrintF(outseq->File, "  File name: '%S'\n",
		outseq->File->Name);
    ajFmtPrintF(outseq->File, "  Extension: '%S'\n",
		outseq->Extension);
    ajFmtPrintF(outseq->File, "  Single: '%B'\n", outseq->Single);
    ajFmtPrintF(outseq->File, "  Features: '%B'\n", outseq->Features);
    ajFmtPrintF(outseq->File, "  Count: '%B'\n", outseq->Count);
    ajFmtPrintF(outseq->File, "  Documentation:...\n%S\n",
		outseq->Doc);
    
    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    sf->numright = ajTrue;
    sf->numleft  = ajTrue;
    sf->numjust  = ajTrue;
    sf->tab      = 1;
    sf->spacer   = 11;
    sf->width    = 50;
    
    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);
    
    return;
}




/* @func ajSeqFileNewOut ******************************************************
**
** Opens an output file for sequence writing. 'stdout' and 'stderr' are
** special cases using standard output and standard error respectively.
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] name [const AjPStr] Output filename.
** @return [AjBool] ajTrue on success.
** @category modify [AjPSeqout] Opens an output file for sequence
**                writing.
** @@
******************************************************************************/

AjBool ajSeqFileNewOut(AjPSeqout seqout, const AjPStr name)
{
    AjBool single;
    AjBool features;

    single   = seqout->Single;
    features = seqout->Features;

    if(ajStrMatchCaseC(name, "stdout"))
	single = ajFalse;
    if(ajStrMatchCaseC(name, "stderr"))
	single = ajFalse;

    if(single)
    {				     /* ok, but nothing to open yet */
	ajStrAssignEmptyS(&seqout->Extension, seqout->Formatstr);
	return ajTrue;
    }
    else
    {
	seqout->File = ajFileNewOut(name);
	if(seqout->File)
	    return ajTrue;
    }

    if(features)
	ajWarn("ajSeqFileNewOut features not yet implemented");

    return ajFalse;
}




/* @funcstatic seqoutUfoLocal *************************************************
**
** Tests whether a sequence output object will write features to the
** sequence output file. The alternative is to use a separate UFO.
**
** @param [r] thys [const AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue if the features will be written to the sequence
** @@
******************************************************************************/

static AjBool seqoutUfoLocal(const AjPSeqout thys)
{
    ajDebug("seqoutUfoLocal Features %B Ufo %d '%S'\n",
	    thys->Features, ajStrGetLen(thys->Ufo), thys->Ufo);

    if(thys->Features && !ajStrGetLen(thys->Ufo))
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

static AjBool seqoutUsaProcess(AjPSeqout thys)
{    
    AjBool fmtstat;
    AjBool regstat;
    
#ifdef __CYGWIN__
    AjPStr usatmp = NULL;
#endif

    ajDebug("seqoutUsaProcess\n");
    if(!seqoutRegFmt)
#ifndef WIN32
	seqoutRegFmt = ajRegCompC("^([A-Za-z0-9]*)::?(.*)$");
    /* \1 format */
    /* \2 remainder */
#else
    /* Windows file names can start with e.g.: 'C:\' */
    /* -> Require that format names have at least 2 letters */
    seqoutRegFmt = ajRegCompC("^([A-Za-z0-9][A-Za-z0-9][A-Za-z0-9]*)::?(.*)$");
    /* \1 format */
    /* \2 remainder */
#endif


    if(!seqoutRegId)			  /* \1 is filename \3 is the qryid */
	seqoutRegId = ajRegCompC("^(.*)$");
    
    ajStrAssignS(&seqoutUsaTest, thys->Usa);

#ifdef __CYGWIN__
    if(*(ajStrGetPtr(seqoutUsaTest)+1)==':')
    {
	usatmp = ajStrNew();
        ajFmtPrintS(&usatmp,"/cygdrive/%c/%s",*ajStrGetPtr(seqoutUsaTest),
		    ajStrGetPtr(seqoutUsaTest)+2);
        ajStrAssignRef(&seqoutUsaTest,usatmp);
        ajStrDel(&usatmp);
    }
#endif

    ajDebug("output USA to test: '%S'\n\n", seqoutUsaTest);
    
    fmtstat = ajRegExec(seqoutRegFmt, seqoutUsaTest);
    ajDebug("format regexp: %B\n", fmtstat);
    
    if(fmtstat)
    {
	ajRegSubI(seqoutRegFmt, 1, &thys->Formatstr);
	ajStrAssignEmptyC(&thys->Formatstr, seqOutFormat[0].Name);
	/* default  unknown */

	ajRegSubI(seqoutRegFmt, 2, &seqoutUsaTest);
	ajDebug("found format %S\n", thys->Formatstr);
	if(!ajSeqFindOutFormat(thys->Formatstr, &thys->Format))
	{
	    ajDebug("unknown format '%S'\n", thys->Formatstr);
	    return ajFalse;
	}
    }
    else
	ajDebug("no format specified in USA\n");

    ajDebug("\n");
    
    regstat = ajRegExec(seqoutRegId, seqoutUsaTest);
    ajDebug("file:id regexp: %B\n", regstat);
    
    if(regstat)
    {
	ajRegSubI(seqoutRegId, 1, &thys->Filename);
	ajDebug("found filename %S single: %B dir: '%S'\n",
		thys->Filename, thys->Single, thys->Directory);
	if(thys->Single)
	    ajDebug("single output file per sequence, open later\n");
	else
	{
	    if(thys->Knownfile)
		thys->File = thys->Knownfile;
	    else
		thys->File = ajFileNewOutD(thys->Directory, thys->Filename);

	    if(!thys->File)
	    {
		if(ajStrGetLen(thys->Directory))
		    ajErr("failed to open filename '%S' in directory '%S'",
			  thys->Filename, thys->Directory);
		else
		    ajErr("failed to open filename '%S'", thys->Filename);

		return ajFalse;
	    }
	}
    }
    else
	ajDebug("no filename specified\n");

    ajDebug("\n");
    
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
** @category modify [AjPSeqout] If the file is not yet open, calls
**                seqoutUsaProcess
** @@
******************************************************************************/

AjBool ajSeqoutOpen(AjPSeqout thys)
{
    AjBool ret = ajFalse;

    if(thys->Ftquery)
	ajDebug("ajSeqoutOpen dir '%S' qrydir '%S'\n",
		thys->Directory, thys->Ftquery->Directory);
    else
	ajDebug("ajSeqoutOpen dir '%S' (no ftquery)\n",
		thys->Directory);

    ret = seqoutUsaProcess(thys);

    if(!ret)
	return ajFalse;

    if(!thys->Features)
	return ret;

    ajStrAssignEmptyS(&thys->Ftquery->Seqname, thys->Name);
    ajFeattabOutSetBasename(thys->Ftquery, thys->Filename);
    ret = ajFeattabOutSet(thys->Ftquery, thys->Ufo);

    return ret;
}




/* @func ajSeqOutFormatSingle *************************************************
**
** Checks whether an output format should go to single files, rather than
** all sequences being written to one file. Some formats do not work when
** more than one sequence is writte to a file. Obvious examples are plain
** text and GCG formats.
**
** @param [u] format [AjPStr] Output format required.
** @return [AjBool] ajTrue if separate file is needed for each sequence.
** @@
******************************************************************************/

AjBool ajSeqOutFormatSingle(AjPStr format)
{
    ajint iformat;

    if(!ajSeqFindOutFormat(format, &iformat))
    {
	ajErr("Unknown output format '%S'", format);
	return ajFalse;
    }

    return seqOutFormat[iformat].Single;
}




/* @func ajSeqOutSetFormat ****************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [u] thys [AjPSeqout] Sequence output object.
** @param [r] format [const AjPStr] Output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutSetFormat(AjPSeqout thys, const AjPStr format)
{
    static AjPStr fmt = NULL;

    ajDebug("ajSeqOutSetFormat '%S'\n", format);
    ajStrAssignS(&fmt, format);
    ajSeqOutFormatDefault(&fmt);

    ajStrAssignEmptyS(&thys->Formatstr, fmt);
    ajDebug("... output format set to '%S'\n", fmt);

    return ajTrue;
}




/* @func ajSeqOutSetFormatC ***************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [u] thys [AjPSeqout] Sequence output object.
** @param [r] format [const char *] Output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutSetFormatC(AjPSeqout thys, const char* format)
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
** @param [w] pformat [AjPStr*] Default output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutFormatDefault(AjPStr* pformat)
{

    if(ajStrGetLen(*pformat))
	ajDebug("... output format '%S'\n", *pformat);
    else
    {
	/* ajStrAssignEmptyC(pformat, seqOutFormat[0].Name);*/
	if (ajNamGetValueC("outformat", pformat))
	    ajDebug("ajSeqOutFormatDefault '%S' from EMBOSS_OUTFORMAT\n",
		     *pformat);
	else
	{
	    ajStrAssignEmptyC(pformat, "fasta"); /* use the real name */
	    ajDebug("... output format not set, default to '%S'\n", *pformat);
	}
    }

    return ajTrue;
}




/* @func ajSeqoutUsa **********************************************************
**
** Creates or resets a sequence output object using a new Universal
** Sequence Address
**
** @param [u] pthis [AjPSeqout*] Sequence output object.
** @param [r] Usa [const AjPStr] USA
** @return [void]
** @category modify [AjPSeqout] Resets using a new USA
** @@
******************************************************************************/

void ajSeqoutUsa(AjPSeqout* pthis, const AjPStr Usa)
{
    AjPSeqout thys;

    if(!*pthis)
	thys = *pthis = ajSeqoutNew();
    else
    {
	thys = *pthis;
	ajSeqoutClear(thys);
    }

    ajStrAssignS(&thys->Usa, Usa);

    return;
}




/* @func ajSeqoutClear ********************************************************
**
** Clears a Sequence output object back to "as new" condition
**
** @param [u] thys [AjPSeqout] Sequence output object
** @return [void]
** @category modify [AjPSeqout] Resets ready for reuse.
** @@
******************************************************************************/

void ajSeqoutClear(AjPSeqout thys)
{

    AjPStr ptr = NULL;

    ajDebug("ajSeqoutClear called\n");

    ajStrSetClear(&thys->Name);
    ajStrSetClear(&thys->Acc);
    ajStrSetClear(&thys->Sv);
    ajStrSetClear(&thys->Gi);
    ajStrSetClear(&thys->Tax);
    ajStrSetClear(&thys->Desc);
    ajStrSetClear(&thys->Type);
    ajStrSetClear(&thys->Outputtype);
    ajStrSetClear(&thys->Full);
    ajStrSetClear(&thys->Date);
    ajStrSetClear(&thys->Doc);
    ajStrSetClear(&thys->Usa);
    ajStrSetClear(&thys->Ufo);
    ajStrSetClear(&thys->Informatstr);
    ajStrSetClear(&thys->Formatstr);
    ajStrSetClear(&thys->Filename);
    ajStrSetClear(&thys->Directory);
    ajStrSetClear(&thys->Entryname);
    ajStrSetClear(&thys->Extension);
    ajStrSetClear(&thys->Seq);
    thys->EType  = 0;
    thys->Rev    = ajFalse;
    thys->Format = 0;

    if(thys->File)
    {
	if(thys->Knownfile)
	    thys->File = NULL;
	else
	    ajFileClose(&thys->File);
    }

    thys->Count    = 0;
    thys->Single   = ajFalse;
    thys->Features = ajFalse;

    while(ajListstrPop(thys->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(thys->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(thys->Taxlist,&ptr))
	ajStrDel(&ptr);

    AJFREE(thys->Accuracy);

    return;
}




/* @func ajSeqPrintOutFormat **************************************************
**
** Reports the internal data structures
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintOutFormat(AjPFile outf, AjBool full)
{

    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# sequence output formats\n");
    ajFmtPrintF(outf, "# Alias Alias name\n");
    ajFmtPrintF(outf, "# Single: If true, write each sequence to new file\n");
    ajFmtPrintF(outf, "# Save: If true, save sequences, write when closed\n");
    ajFmtPrintF(outf, "# Nuc   Can read nucleotide input\n");
    ajFmtPrintF(outf, "# Pro   Can read protein input\n");
    ajFmtPrintF(outf, "# Feat  Can read feature annotation\n");
    ajFmtPrintF(outf, "# Gap   Can read gap characters\n");
    ajFmtPrintF(outf, "# Mset  Can read seqsetall (multiple seqsets)\n");
    ajFmtPrintF(outf, "# Name          Alias Single Save  Pro  Nuc Feat  "
		"Gap MSet Description\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "OutFormat {\n");
    for(i=0; seqOutFormat[i].Name; i++)
    {
	ajFmtPrintF(outf,
		    "  %-15s %3B    %3B  %3B  %3B  %3B  %3B  %3B  %3B \"%s\"\n",
		    seqOutFormat[i].Name,
		    seqOutFormat[i].Alias,
		    seqOutFormat[i].Single,
		    seqOutFormat[i].Save,
		    seqOutFormat[i].Nucleotide,
		    seqOutFormat[i].Protein,
		    seqOutFormat[i].Feature,
		    seqOutFormat[i].Gap,
		    seqOutFormat[i].Multiset,
		    seqOutFormat[i].Desc);
    }
    ajFmtPrintF(outf, "}\n\n");

    return;
}




/* @func ajSeqFindOutFormat ***************************************************
**
** Looks for the specified output format in the internal definitions and
** returns the index.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqFindOutFormat(const AjPStr format, ajint* iformat)
{

    AjPStr tmpformat = NULL;
    ajint i = 0;

    if(!ajStrGetLen(format))
    {
	if (ajNamGetValueC("outformat", &tmpformat))
	    ajDebug("ajSeqFindOutFormat '%S' from EMBOSS_OUTFORMAT\n",
		    tmpformat);
	else
	    return ajFalse;

    }
    else
	ajStrAssignS(&tmpformat, format);

    ajStrFmtLower(&tmpformat);

    while(seqOutFormat[i].Name)
    {
	if(ajStrMatchCaseC(tmpformat, seqOutFormat[i].Name))
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
** @param [u] psf [SeqPSeqFormat*] Sequence format object
** @return [void]
** @@
******************************************************************************/

static void seqSeqFormat(ajint seqlen, SeqPSeqFormat* psf)
{
    char numform[20];
    SeqPSeqFormat sf;
    ajint i;
    ajint j;

    j = 1;

    for(i = seqlen; i; i /= 10)
	j++;

    sprintf(numform, "%d", seqlen);
    ajDebug("seqSeqFormat numwidth old: %d new: %d\n", strlen(numform)+1, j);

    if(!*psf)
    {
	sf = AJNEW0(*psf);
	sf->namewidth = 8;
	sf->spacer    = 0;
	sf->width     = 50;
	sf->tab       = 0;
	sf->numleft   = ajFalse;
	sf->numright  = sf->numleft = sf->numjust = ajFalse;
	sf->nameright = sf->nameleft = ajFalse;
	sf->numline   = 0;
	sf->linepos   = 0;

	sf->skipbefore  = ajFalse;
	sf->skipafter   = ajFalse;
	sf->isactive    = ajFalse;
	sf->baseonlynum = ajFalse;
	sf->gapchar     = '-';
	sf->matchchar   = '.';
	sf->noleaves    = sf->domatch = sf->degap = ajFalse;
	sf->pretty = ajFalse;
	strcpy(sf->endstr, "");
	/*sf->interline = 1;*/
    }
    else
	sf = *psf;

    sf->numwidth = j;		    /* or 8 as a reasonable minimum */

    return;
}




/* @funcstatic seqWriteSeq ****************************************************
**
** Writes an output sequence. The format and all other information is
** already stored in the output sequence object and the formatting structure.
**
** @param [u] outseq [AjPSeqout] Output sequence.
** @param [r] sf [const SeqPSeqFormat] Output formatting structure.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSeq(AjPSeqout outseq, const SeqPSeqFormat sf)
{
    /* code adapted from what readseq did */
    
    static ajint maxSeqWidth       = 250;
    static char* defNocountSymbols = "_.-?";
    
    ajint i = 0;
    ajint j = 0;
    ajint l = 0;
    ajint ibase    = 0;
    ajint linesout = 0;
    ajint seqlen;
    const char *seq;
    const char *idword;
    char *cp;
    char s[1024];			/* the output line */
    
    char nameform[20];
    char numform[20];
    char nocountsymbols[20];
    
    ajint width;
    ajint l1;
    
    AjPFile file;
    FILE* outf;
    
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


    seqlen = ajStrGetLen(outseq->Seq);
    seq    = ajStrGetPtr(outseq->Seq);
    width  = sf->width;
    l1     = sf->linepos;
    file   = outseq->File;
    outf   = ajFileFp(file);


    /* if(sf->numline) numline = 1;*/
    
    if(sf->nameleft || sf->nameright)
	sprintf(nameform, "%%%d.%ds ",sf->namewidth,sf->namewidth);

    if(sf->numline)
	sprintf(numform, "%%%ds ",sf->numwidth);
    else
	sprintf(numform, "%%%dd",sf->numwidth);
    
    strcpy( nocountsymbols, defNocountSymbols);
    if(sf->baseonlynum)
    {				      /* add gap character to skips */
	if(strchr(nocountsymbols,sf->gapchar)==NULL)
	{
	    strcat(nocountsymbols," ");
	    nocountsymbols[strlen(nocountsymbols)-1]= sf->gapchar;
	}

	if(sf->domatch &&	 /* remove gap character from skips */
	   (cp=strchr(nocountsymbols,sf->matchchar))!=NULL)
	    *cp= ' ';
    }
    
    if(sf->numline)
	idword= "";
    else
	idword = ajStrGetPtr(outseq->Name);
    
    width = AJMIN(width,maxSeqWidth);
    
    i=0;				/* seqpos position in seq[]*/
    l=0;		     /* linepos position in output line s[] */
    
    ibase = 1;				/* base count */
    
    while(i < seqlen)
    {
	
	if(l1 < 0)
	    l1 = 0;
	else if(l1 == 0)
	{
	    /* start of a new line */
	    if(sf->skipbefore)
	    {
		fprintf(outf, "\n");   /* blank line before writing */
		linesout++;
	    }

	    if(*(sf->leftstr))
		fprintf(outf, sf->leftstr); /* string at line start */

	    if(sf->nameleft)
		fprintf(outf, nameform, idword);

	    if(sf->numleft)
	    {
		if(sf->numline)
		    fprintf(outf, numform, "");
		else
		    fprintf(outf, numform, ibase);
	    }

	    for(j=0; j < sf->tab; j++)
		fputc(' ',outf);
	}
	
	l1++;			     /* don't count spaces for width*/
	if(sf->numline)
	{
	    if(sf->spacer==seqSpaceAll ||
	       (sf->spacer != 0 && (l+1) % sf->spacer == 1))
	    {
		if(sf->numline) fputc(' ',outf);
		s[l++] = ' ';
	    }
	
	    if(l1 % 10 == 1 || l1 == width)
	    {
		if(sf->numline) fprintf(outf,"%-9d ",i+1);
		s[l++]= '|';		/* == put a number here */
	    }
	    else s[l++]= ' ';
	    i++;
	}
	else
	{
	    if(sf->spacer==seqSpaceAll ||
	       (sf->spacer != 0 && (l+1) % sf->spacer == 1))
		s[l++] = ' ';

	    if(!sf->baseonlynum)
		ibase++;
	    else if(0==strchr(nocountsymbols,seq[i]))
		ibase++;
	    s[l++] = seq[i++];
	}
	
	if(l1 == width || i == seqlen)
	{
	    if(sf->pretty || sf->numjust)
		for( ; l1<width; l1++)
		{
		    if(sf->spacer==seqSpaceAll ||
		       (sf->spacer != 0 && (l+1) % sf->spacer == 1))
			s[l++] = ' ';
		    s[l++]=' ';		/* pad with blanks */
		}

	    s[l] = '\0';
	    l = 0; l1 = 0;

	    if(!sf->numline)
	    {
		fprintf(outf,"%s",s);
		if(sf->numright || sf->nameright)
		    fputc(' ',outf);
		if(sf->numright)
		    fprintf(outf,numform, ibase-1);
		if(sf->nameright)
		    fprintf(outf, nameform,idword);
		if(i == seqlen)
		    fprintf(outf,"%s",sf->endstr);
	    }
	    fputc('\n',outf);
	    linesout++;
	    if(sf->skipafter)
	    {
		fprintf(outf, "\n");
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

/* @func ajSeqoutCheckGcg *****************************************************
**
** Calculates a GCG checksum for an output sequence.
**
** @param [r] outseq [const AjPSeqout] Output sequence.
** @return [ajint] GCG checksum.
** @category cast [AjPSeqout] Calculates the GCG checksum for a
**                sequence set.
** @@
******************************************************************************/

ajint ajSeqoutCheckGcg(const AjPSeqout outseq)
{
    ajlong  i;
    ajlong check = 0;
    ajlong count = 0;
    const char *cp;
    ajint ilen;

    cp   = ajStrGetPtr(outseq->Seq);
    ilen = ajStrGetLen(outseq->Seq);

    for(i = 0; i < ilen; i++)
    {
	count++;
	check += count * toupper((ajint) cp[i]);
	if(count == 57)
	    count = 0;
    }
    check %= 10000;

    return check;
}




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
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

static void seqClone(AjPSeqout outseq, const AjPSeq seq)
{

    ajint ibegin = 1;
    ajint iend;
    ajint ilen;
    ajint i;

    iend = ajStrGetLen(seq->Seq);

    if(seq->Begin)
    {
	ibegin = ajSeqGetBegin(seq);
	ajDebug("seqClone begin: %d\n", ibegin);
    }

    if(seq->End)
    {
	iend = ajSeqGetEnd(seq);
	ajDebug("seqClone end: %d\n", iend);
    }

    ajStrAssignEmptyS(&outseq->Name, seq->Name);
    ajStrAssignEmptyS(&outseq->Acc, seq->Acc);
    ajListstrClone(seq->Acclist, outseq->Acclist);
    ajStrAssignEmptyS(&outseq->Sv, seq->Sv);
    ajStrAssignEmptyS(&outseq->Gi, seq->Gi);
    ajStrAssignEmptyS(&outseq->Tax, seq->Tax);
    ajListstrClone(seq->Taxlist, outseq->Taxlist);
    ajListstrClone(seq->Keylist, outseq->Keylist);
    ajStrAssignEmptyS(&outseq->Desc, seq->Desc);
    ajStrAssignEmptyS(&outseq->Type, seq->Type);
    ajStrAssignEmptyS(&outseq->Informatstr, seq->Formatstr);
    ajStrAssignEmptyS(&outseq->Entryname, seq->Entryname);
    ajStrAssignEmptyS(&outseq->Db, seq->Db);

    AJFREE(outseq->Accuracy);
    if(seq->Accuracy)
    {
	ilen = ajStrGetLen(seq->Seq);
	AJCNEW(outseq->Accuracy, ilen);
	for(i=0;i<ilen;i++)
	    outseq->Accuracy[i] = seq->Accuracy[i];
    }

    outseq->Offset = ibegin - 1;

    if(iend >= ibegin)
	ajStrAssignSubS(&outseq->Seq, seq->Seq, ibegin-1, iend-1);
    else				/* empty sequence */
	ajStrAssignC(&outseq->Seq, "");

    outseq->Fttable = seq->Fttable;

    if(outseq->Fttable)
	ajFeattableTrimOff(outseq->Fttable,
			   outseq->Offset, ajStrGetLen(outseq->Seq));

    ajDebug("seqClone %d .. %d %d .. %d len: %d type: '%S'\n",
	    seq->Begin, seq->End, ibegin, iend,
	    ajStrGetLen(outseq->Seq), outseq->Type);
    ajDebug("  Db: '%S' Name: '%S' Entryname: '%S'\n",
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
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

static void seqAllClone(AjPSeqout outseq, const AjPSeq seq)
{

    ajint ibegin = 1;
    ajint iend;
    ajint ilen;
    ajint i;

    iend = ajStrGetLen(seq->Seq);

    if(seq->Begin)
    {
	ibegin = ajSeqGetBegin(seq);
	ajDebug("seqAllClone begin: %d\n", ibegin);
    }

    if(seq->End)
    {
	iend = ajSeqGetEnd(seq);
	ajDebug("seqAllClone end: %d\n", iend);
    }
    ajDebug("ajSeqAllClone outseq->Type '%S' seq->Type '%S'\n",
	    outseq->Type, seq->Type);
    ajStrAssignS(&outseq->Db, seq->Db);
    ajStrAssignS(&outseq->Name, seq->Name);
    ajStrAssignS(&outseq->Acc, seq->Acc);
    ajListstrClone(seq->Acclist, outseq->Acclist);
    ajStrAssignS(&outseq->Sv, seq->Sv);
    ajStrAssignS(&outseq->Gi, seq->Gi);
    ajStrAssignS(&outseq->Tax, seq->Tax);
    ajListstrClone(seq->Taxlist, outseq->Taxlist);
    ajListstrClone(seq->Keylist, outseq->Keylist);
    ajStrAssignS(&outseq->Desc, seq->Desc);
    ajStrAssignS(&outseq->Type, seq->Type);
    ajStrAssignS(&outseq->Informatstr, seq->Formatstr);
    ajStrAssignS(&outseq->Entryname, seq->Entryname);

    AJFREE(outseq->Accuracy);
    if(seq->Accuracy)
    {
	ilen = ajStrGetLen(seq->Seq);
	AJCNEW(outseq->Accuracy, ilen);
	for(i=0;i<ilen;i++)
	    outseq->Accuracy[i] = seq->Accuracy[i];
    }

    outseq->Offset = ibegin - 1;

    if(iend >= ibegin)
	ajStrAssignSubS(&outseq->Seq, seq->Seq, ibegin-1, iend-1);
    else				/* empty sequence */
	ajStrAssignC(&outseq->Seq, "");

    outseq->Fttable = seq->Fttable;
    if(outseq->Fttable)
	ajFeattableTrimOff(outseq->Fttable,
			    outseq->Offset, ajStrGetLen(outseq->Seq));

    ajDebug("seqAllClone %d .. %d %d .. %d len: %d type: '%S'\n",
	     seq->Begin, seq->End, ibegin, iend,
	     ajStrGetLen(outseq->Seq), outseq->Type);
    ajDebug("  Db: '%S' Name: '%S' Entryname: '%S'\n",
	     outseq->Db, outseq->Name, outseq->Entryname);

    ajSeqTypeCheckS(&outseq->Seq, outseq->Outputtype);

    return;
}




/* @funcstatic seqsetClone ****************************************************
**
** Clones one sequence from a set ready for output.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @param [r] seqset [const AjPSeqset] Sequence set.
** @param [r] i [ajint] Sequence number, zero for the first sequence.
** @return [void]
** @@
******************************************************************************/

static void seqsetClone(AjPSeqout outseq, const AjPSeqset seqset, ajint i)
{
    /* intended to clone ith sequence in the set */
    AjPSeq seq;

    seq = seqset->Seq[i];

    seqAllClone(outseq, seq);

    return;
}




/* @funcstatic seqDeclone *****************************************************
**
** Clears clones data in a sequence output object.
**
** @param [u] outseq [AjPSeqout] Sequence output.
** @return [void]
** @@
******************************************************************************/

static void seqDeclone(AjPSeqout outseq)
{
    AjPStr ptr = NULL;

    ajStrSetClear(&outseq->Db);
    ajStrSetClear(&outseq->Name);
    ajStrSetClear(&outseq->Acc);
    ajStrSetClear(&outseq->Sv);
    ajStrSetClear(&outseq->Gi);
    ajStrSetClear(&outseq->Tax);
    ajStrSetClear(&outseq->Desc);
    ajStrSetClear(&outseq->Type);
    ajStrSetClear(&outseq->Informatstr);
    ajStrSetClear(&outseq->Entryname);

    while(ajListstrPop(outseq->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(outseq->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(outseq->Taxlist,&ptr))
	ajStrDel(&ptr);

    ajStrSetClear(&outseq->Seq);
    AJFREE(outseq->Accuracy);

    return;
}




/* @funcstatic seqFileReopen **************************************************
**
** Reopen a sequence output file. Used after the file name has been changed
** when writing a set of sequences one to each file.
**
** @param [u] outseq [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqFileReopen(AjPSeqout outseq)
{
    AjPStr name = NULL;

    if(outseq->File)
	ajFileClose(&outseq->File);

    if(outseq->Knownfile)
	outseq->Knownfile = NULL;

    ajFmtPrintS(&name, "%S.%S", outseq->Name, outseq->Extension);
    ajStrFmtLower(&name);
    outseq->File = ajFileNewOutD(outseq->Directory, name);
    ajDebug("seqFileReopen single: %B file '%S'\n", outseq->Single, name);
    ajStrDel(&name);

    if(!outseq->File)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic seqDbName ******************************************************
**
** Adds the database name (if any) to the name provided.
**
** @param [w] name [AjPStr*] Derived name.
** @param [r] db [const AjPStr] Database name (if any)
** @return [void]
** @@
******************************************************************************/

static void seqDbName(AjPStr* name, const AjPStr db)
{
    static AjPStr tmpname = 0;

    if(!ajStrGetLen(db))
	return;

    ajStrAssignS(&tmpname, *name);
    ajFmtPrintS(name, "%S:%S", db, tmpname);

    return;
}




/* @func ajSeqoutDefName ******************************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] thys [AjPSeqout] Sequence output object
** @param [r] setname [const AjPStr] Name set by caller
** @param [r] multi [AjBool] If true, appends a number to the name.
** @return [void]
** @@
******************************************************************************/

void ajSeqoutDefName(AjPSeqout thys, const AjPStr setname, AjBool multi)
{
    static ajint count = 0;

    if(ajStrGetLen(thys->Name))
    {
	ajDebug("ajSeqoutDefName already has a name '%S'\n", thys->Name);
	return;
    }

    if (ajStrGetLen(setname))
    {
	if(multi && count)
	    ajFmtPrintS(&thys->Name, "%S_%3.3d", setname, ++count);
	else
	{
	    ajStrAssignS(&thys->Name, setname);
	    ++count;
	}
    }
    else
    {
	if(multi)
	    ajFmtPrintS(&thys->Name, "EMBOSS_%3.3d", ++count);
	else
	{
	    ajStrAssignC(&thys->Name, "EMBOSS");
	    ++count;
	}
    }

    ajDebug("ajSeqoutDefName set to  '%S'\n", thys->Name);

    return;
}




/* @func ajSeqoutTrace ********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [const AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

void ajSeqoutTrace(const AjPSeqout seq)
{
    AjIList it;
    AjPStr cur;
    
    ajDebug("\n\n\nSequence Out trace\n");
    ajDebug( "==============\n\n");
    ajDebug( "  Name: '%S'\n", seq->Name);

    if(ajStrGetLen(seq->Acc))
	ajDebug( "  Accession: '%S'\n", seq->Acc);

    if(ajListLength(seq->Acclist))
    {
	ajDebug("  Acclist: (%d)",
		ajListLength(seq->Acclist));
	it = ajListIterRead(seq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" %S\n", cur);

	ajListIterFree(&it);
	ajDebug("\n");
    }

    if(ajStrGetLen(seq->Sv))
	ajDebug( "  SeqVersion: '%S'\n", seq->Sv);

    if(ajStrGetLen(seq->Gi))
	ajDebug( "  GenInfo Id: '%S'\n", seq->Gi);

    if(ajStrGetLen(seq->Desc))
	ajDebug( "  Description: '%S'\n", seq->Desc);

    if(ajStrGetRes(seq->Seq))
	ajDebug( "  Reserved: %d\n", ajStrGetRes(seq->Seq));

    if(ajListLength(seq->Keylist))
    {
	ajDebug("  Keywordlist: (%d)",
		ajListLength(seq->Keylist));
	it = ajListIterRead(seq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug("   '%S'\n", cur);

	ajListIterFree(&it);
	ajDebug("\n");
    }
    ajDebug("  Taxonomy: '%S'\n", seq->Tax);

    if(ajListLength(seq->Taxlist))
    {
	ajDebug("  Taxlist: (%d)",
		ajListLength(seq->Taxlist));
	it = ajListIterRead(seq->Taxlist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug("   '%S'\n", cur);

	ajListIterFree(&it);
    }

    if(ajStrGetLen(seq->Type))
	ajDebug( "  Type: '%S'\n", seq->Type);

    if(ajStrGetLen(seq->Outputtype))
	ajDebug( "  Output type: '%S'\n", seq->Outputtype);

    if(ajStrGetLen(seq->Db))
	ajDebug( "  Database: '%S'\n", seq->Db);

    if(ajStrGetLen(seq->Full))
	ajDebug( "  Full name: '%S'\n", seq->Full);

    if(ajStrGetLen(seq->Date))
	ajDebug( "  Date: '%S'\n", seq->Date);

    if(ajStrGetLen(seq->Usa))
	ajDebug( "  Usa: '%S'\n", seq->Usa);

    if(ajStrGetLen(seq->Ufo))
	ajDebug( "  Ufo: '%S'\n", seq->Ufo);

    if(ajStrGetLen(seq->Formatstr))
	ajDebug( "  Output format: '%S'\n", seq->Formatstr);

    if(ajStrGetLen(seq->Filename))
	ajDebug( "  Filename: '%S'\n", seq->Filename);

    if(ajStrGetLen(seq->Directory))
	ajDebug( "  Directory: '%S'\n", seq->Directory);

    if(ajStrGetLen(seq->Entryname))
	ajDebug( "  Entryname: '%S'\n", seq->Entryname);

    if(ajStrGetLen(seq->Doc))
	ajDebug( "  Documentation:...\n%S\n", seq->Doc);

    if(seq->Fttable)
	ajFeattableTrace(seq->Fttable);
    else
	ajDebug( "  No Feature table present\n");

    if(seq->Features)
	ajDebug( "  Features ON\n");
    else
	ajDebug( "  Features OFF\n");

    return;
}




/* @func ajSeqoutCount ********************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [r] seqout [const AjPSeqout] Sequence output object
** @param [w] b [ajint*] integer array, minimum size 5, to hold the results.
** @return [void]
** @@
******************************************************************************/

void ajSeqoutCount(const AjPSeqout seqout, ajint* b)
{
    const char* cp;

    ajDebug("ajSeqoutCount %d bases\n", ajStrGetLen(seqout->Seq));

    b[0] = b[1] = b[2] = b[3] = b[4] = 0;

    cp = ajStrGetPtr(seqout->Seq);

    while(*cp)
    {
	switch (*cp)
	{
	case 'A':
	case 'a':
	    b[0]++;
	    break;
	case 'C':
	case 'c':
	    b[1]++;
	    break;
	case 'G':
	case 'g':
	    b[2]++;
	    break;
	case 'T':
	case 't':
	case 'U':
	case 'u':
	    b[3]++;
	    break;
	default:
	    break;
	}
	cp++;
    }

    b[4] = ajStrGetLen(seqout->Seq) - b[0] - b[1] - b[2] - b[3];

    return;
}




/* @func ajSeqWriteXyz ********************************************************
**
** Writes a sequence in SWISSPROT format.
**
** @param [w] outf [AjPFile] output stream
** @param [r] seq [const AjPStr] sequence
** @param [r] prefix [const char *] identifier code - should be 2 char's long
** @return [void]
** @@
******************************************************************************/

void ajSeqWriteXyz(AjPFile outf, const AjPStr seq, const char *prefix)
{
    AjPSeqout outseq        = NULL;
    static SeqPSeqFormat sf = NULL;

    ajint mw;
    ajuint crc;


    crc = ajSeqstrCalcCrc(seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(seq));
    ajFmtPrintF(outf,
		"%-5sSEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
		prefix, ajStrGetLen(seq), mw, crc);

    outseq = ajSeqoutNewF(outf);

    ajStrAssignS(&outseq->Seq,seq);

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "");
    sf->tab    = 4;
    sf->spacer = 11;
    sf->width  = 60;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    ajSeqoutDel(&outseq);

    return;
}




/* @func ajSssWriteXyz ********************************************************
**
** Writes a sequence in SWISSPROT format w/o checksum or molecular weight -
** used for printing secondary structure strings.
**
** @param [w] outf [AjPFile] output stream
** @param [r] seq [const AjPStr] sequence
** @param [r] prefix [const char *] identifier code - should be 2 char's long
** @return [void]
** @@
******************************************************************************/

void ajSssWriteXyz(AjPFile outf, const AjPStr seq, const char *prefix)
{
    AjPSeqout outseq        = NULL;
    static SeqPSeqFormat sf = NULL;

    ajint mw;
    ajint crc;

    outseq = ajSeqoutNew();

    outseq->File = outf;
    ajStrAssignS(&outseq->Seq,seq);

    crc = ajSeqstrCalcCrc(outseq->Seq);
    mw = (ajint) (0.5+ajSeqstrCalcMolwt(outseq->Seq));
    ajFmtPrintF(outseq->File,
		"%-5sSEQUENCE %5d AA;\n",
		prefix, ajStrGetLen(outseq->Seq));

    seqSeqFormat(ajStrGetLen(outseq->Seq), &sf);
    strcpy(sf->endstr, "");
    sf->tab    = 4;
    sf->spacer = 11;
    sf->width  = 60;

    seqWriteSeq(outseq, sf);
    seqFormatDel(&sf);

    return;
}


/* @funcstatic seqFormatDel ***************************************************
**
** Delete a sequence format object
**
** @param [d] pformat [SeqPSeqFormat*] Sequence format
** @return [void]
******************************************************************************/

static void seqFormatDel(SeqPSeqFormat* pformat)
{
    AJFREE(*pformat);
    return;
}




/* @func ajSeqWriteExit *******************************************************
**
** Cleans up sequence output processing internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqWriteExit(void)
{
    ajRegFree(&seqoutRegFmt);
    ajRegFree(&seqoutRegId);

    ajStrDel(&seqoutUsaTest);

    return;
}

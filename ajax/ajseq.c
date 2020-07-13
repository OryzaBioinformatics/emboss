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
#include <limits.h>

static ajulong seqCrcTable[256];

static void       seqCrcGen( void );
static AjPSelexdata seqSelexClone(AjPSelexdata thys);


/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Stream Constructors **************************************
**
** All constructors return a new sequence stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence stream object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/

/* @func ajSeqallNew **********************************************************
**
** Creates a new sequence stream object to hold one sequence at a time.
**
** @return [AjPSeqall] New sequence stream object.
** @@
******************************************************************************/

AjPSeqall ajSeqallNew (void)
{
    AjPSeqall pthis;

    AJNEW0(pthis);

    pthis->Seq = ajSeqNew();
    pthis->Seqin = ajSeqinNew ();
    pthis->Count = 0;

    return pthis;
}


/* @funcstatic seqSelexClone **************************************************
**
** Clone a Selexdata object
**
** @param [r] thys [AjPSelexdata] selex data object
**
** @return [AjPSelexdata] New selex data object.
** @@
******************************************************************************/

static AjPSelexdata seqSelexClone(AjPSelexdata thys)
{
    AjPSelexdata pthis;

    pthis = ajSelexdataNew();

    ajStrAssS(&pthis->id, thys->id);
    ajStrAssS(&pthis->ac, thys->ac);
    ajStrAssS(&pthis->de, thys->de);
    ajStrAssS(&pthis->au, thys->au);
    ajStrAssS(&pthis->cs, thys->cs);
    ajStrAssS(&pthis->rf, thys->rf);
    ajStrAssS(&pthis->name, thys->name);
    ajStrAssS(&pthis->str, thys->str);
    ajStrAssS(&pthis->ss, thys->ss);

    pthis->ga[0] = thys->ga[0];
    pthis->ga[1] = thys->ga[1];
    pthis->tc[0] = thys->tc[0];
    pthis->tc[1] = thys->tc[1];
    pthis->nc[0] = thys->nc[0];
    pthis->nc[1] = thys->nc[1];

    ajStrAssS(&pthis->sq->name, thys->sq->name);
    ajStrAssS(&pthis->sq->source, thys->sq->source);
    ajStrAssS(&pthis->sq->ac, thys->sq->ac);
    ajStrAssS(&pthis->sq->de, thys->sq->de);

    pthis->sq->wt    = thys->sq->wt;
    pthis->sq->start = thys->sq->start;
    pthis->sq->stop  = thys->sq->stop;
    pthis->sq->len   = thys->sq->len;


    return pthis;
}



/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Stream Destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
******************************************************************************/

/* @func ajSeqallDel **********************************************************
**
** Destructor for sequence stream objects
**
** @param [d] thys [AjPSeqall*] Sequence stream object reference
** @return [void]
** @@
******************************************************************************/

void ajSeqallDel(AjPSeqall *thys)
{

    ajSeqDel(&(*thys)->Seq);
    ajSeqinDel(&(*thys)->Seqin);

    AJFREE(*thys);

    return;
}


/* @func ajSeqsetDel **********************************************************
**
** Destructor for sequence set objects
**
** @param [d] thys [AjPSeqset*] Sequence set object reference
** @return [void]
** @@
******************************************************************************/

void ajSeqsetDel(AjPSeqset *thys)
{
    ajint n;
    ajint i;
    AjPSeqset pthis=NULL;

    if(!thys || !*thys)
	return;

    pthis = *thys;
    if(!(n = pthis->Size))
	return;

    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Formatstr);
    ajStrDel(&pthis->Filename);
    ajStrDel(&pthis->Full);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Usa);
    ajStrDel(&pthis->Ufo);

    for(i=0; i<n; ++i)
	ajSeqDel(&pthis->Seq[i]);

    AJFREE(pthis->Seq);
    AJFREE(pthis->Seqweight);

    AJFREE(pthis);

    return;
}


/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Stream Assignments ***************************************
**
** These functions overwrite the sequence stream object provided as
** the first argument.
**
******************************************************************************/

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Stream Modifiers *****************************************
**
** These functions use the contents of a sequence stream object and
** update them.
**
******************************************************************************/

/* @func ajSeqallToLower ******************************************************
**
** Converts the latest sequence in a stream to lower case.
**
** @param [P] seqall [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallToLower (AjPSeqall seqall)
{
    (void) ajSeqToLower(seqall->Seq);

    return;
}

/* @func ajSeqallToUpper ******************************************************
**
** Converts the latest sequence in a stream to upper case.
**
** @param [P] seqall [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallToUpper (AjPSeqall seqall)
{
    (void) ajSeqToUpper(seqall->Seq);

    return;
}

/* @func ajSeqallReverse ******************************************************
**
** Reverse complements the latest sequence in a stream.
**
** @param [P] thys [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallReverse (AjPSeqall thys)
{
    ajint ibegin = thys->Begin;
    ajint iend = thys->End;

    ajDebug ("ajSeqallReverse len: %d Begin: %d End: %d\n",
	     ajSeqallLen(thys), thys->Begin, thys->End);

    thys->End = -(ibegin);
    thys->Begin = -(iend);

    (void) ajSeqReverse(thys->Seq);

    ajDebug ("  all result len: %d Begin: %d End: %d\n",
	     ajSeqallLen(thys), thys->Begin, thys->End);

    return;
}

/* @func ajSeqallSetRange *****************************************************
**
** Sets the start and end positions for a sequence stream.
**
** @param [P] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqallSetRange (AjPSeqall seq, ajint ibegin, ajint iend)
{
    ajDebug ("ajSeqallSetRange (len: %d %d, %d)\n",
	     ajSeqLen(seq->Seq), ibegin, iend);

    if (ibegin)
	seq->Begin = seq->Seq->Begin = ibegin;

    if (iend)
	seq->End = seq->Seq->End = iend;

    ajDebug ("      result: (len: %d %d, %d)\n",
	     ajSeqLen(seq->Seq), seq->Begin, seq->End);

    return;
}

/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Sequence Stream Casts *********************************************
**
** These functions examine the contents of a sequence stream object
** and return some derived information. Some of them provide access to
** the internal components of a sequence stream object. They are
** provided for programming convenience but should be used with
** caution.
**
******************************************************************************/

/* @func ajSeqallLen **********************************************************
**
** Returns the length of a sequence stream, which is the length of the
** latest sequence read.
**
** @param [P] seqall [AjPSeqall] Sequence stream object
** @return [ajint] sequence length.
** @@
******************************************************************************/

ajint ajSeqallLen (AjPSeqall seqall)
{
    return ajSeqLen(seqall->Seq);
}

/* @func ajSeqallBegin ********************************************************
**
** Returns the sequence stream start position, or 1 if no start has been set.
**
** @param [P] seq [AjPSeqall] Sequence stream object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqallBegin (AjPSeqall seq)
{
    if (!seq->Begin)
	return 1;

    return ajSeqPos(seq->Seq, seq->Begin);
}

/* @func ajSeqallEnd **********************************************************
**
** Returns the sequence stream end position, or the sequence length if no end
** has been set.
**
** @param [P] seq [AjPSeqall] Sequence stream object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqallEnd (AjPSeqall seq)
{
    if (!seq->End)
	return ajSeqLen(seq->Seq);

    return ajSeqPosI(seq->Seq, ajSeqallBegin(seq), seq->End);
}

/* @func ajSeqallGetRange *****************************************************
**
** Returns the sequence range for a sequence stream
**
** @param [r] thys [AjPSeqall] Sequence stream object.
** @param [r] begin [ajint*] Sequence range begin
** @param [r] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqallGetRange (AjPSeqall thys, ajint* begin, ajint* end)
{
    ajDebug ("ajSeqallGetRange '%S'\n", thys->Seq->Name);

    return ajSeqGetRange(thys->Seq, begin, end);
}

/* @func ajSeqsetGetFormat ****************************************************
**
** Returns the sequence format for a sequence set
**
** @param [r] thys [AjPSeqset] Sequence set object.
** @return [AjPStr] Sequence format
** @@
******************************************************************************/

AjPStr ajSeqsetGetFormat (AjPSeqset thys)
{
    return thys->Formatstr;
}

/* @func ajSeqsetGetRange *****************************************************
**
** Returns the sequence range for a sequence set
**
** @param [r] thys [AjPSeqset] Sequence set object.
** @param [r] begin [ajint*] Sequence range begin
** @param [r] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqsetGetRange (AjPSeqset thys, ajint* begin, ajint* end)
{
    ajDebug ("ajSeqsetGetRange '%S' begin %d end %d len: %d\n",
	     thys->Name, thys->Begin, thys->End, thys->Len);
    *begin = ajSeqPosII(thys->Len, 1, thys->Begin);

    if (thys->End)
	*end = ajSeqPosII(thys->Len, *begin, thys->End);
    else
	*end = ajSeqPosII(thys->Len, *begin, thys->Len);

    return (*end - *begin + 1);
}

/* @func ajSeqGetRange ********************************************************
**
** Returns the sequence range for a sequence.
**
** @param [r] thys [AjPSeq] Sequence object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqGetRange (AjPSeq thys, ajint* begin, ajint* end)
{
    ajDebug ("ajSeqGetRange '%S'\n", thys->Name);
    *begin = ajSeqPos(thys, thys->Begin);

    if (thys->End)
	*end = ajSeqPosI(thys, *begin, thys->End);
    else
	*end = ajSeqPosI(thys, *begin, ajSeqLen(thys));

    return (*end - *begin + 1);
}

/* @func ajSeqallGetName ******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqall] Sequence stream object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqallGetName (AjPSeqall thys)
{
    ajDebug ("ajSeqallGetName '%S'\n", thys->Seqin->Name);

    return thys->Seqin->Name;
}

/* @func ajSeqallGetNameSeq ***************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqall] Sequence stream object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqallGetNameSeq (AjPSeqall thys)
{
    ajDebug ("ajSeqallGetNameSeq '%S'\n", thys->Seq->Name);

    return ajSeqGetName(thys->Seq);
}

/* @func ajSeqsetGetUsa *******************************************************
**
** Returns the sequence name of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqset] Sequence set object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqsetGetUsa (AjPSeqset thys)
{
    ajDebug ("ajSeqetGetUsa '%S'\n", thys->Usa);

    return thys->Usa;
}

/* @func ajSeqGetUsa **********************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetUsa (AjPSeq thys)
{
    ajDebug ("ajSeqGetUsa '%S'\n", thys->Usa);

    if (!ajStrLen(thys->Usa))
	ajSeqMakeUsa (thys, NULL);

    return thys->Usa;
}

/* @func ajSeqallGetUsa *******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqall] Sequence object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqallGetUsa (AjPSeqall thys)
{
    ajDebug ("ajSeqallGetUsa '%S'\n", thys->Seqin->Usa);

    return thys->Seqin->Usa;
}

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Set Constructors *****************************************
**
** All constructors return a new sequence set object by pointer. It is the
** responsibility of the user to first destroy any previous
** sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
******************************************************************************/

/* @func ajSeqsetNew **********************************************************
**
** Creates a new sequence set object to hold all sequences in memory.
**
** @return [AjPSeqset] New sequence set object.
** @@
******************************************************************************/

AjPSeqset ajSeqsetNew (void)
{
    AjPSeqset pthis;

    AJNEW0(pthis);

    pthis->Size = 0;
    pthis->Len = 0;
    pthis->Begin = 0;
    pthis->End = 0;
    pthis->Totweight = 0.0;
    pthis->Name = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Usa = ajStrNew();
    pthis->Ufo = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Filename = ajStrNew();

    pthis->Seq = NULL;
    pthis->Seqweight = NULL;

    pthis->EType = 0;
    pthis->Format = 0;

    return pthis;
}

/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Set Destructors ******************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence set object.
**
******************************************************************************/

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Set Modifiers ********************************************
**
** These functions use the contents of a sequence set object and
** update them.
**
******************************************************************************/

/* @func ajSeqsetToLower ******************************************************
**
** Converts all sequences in a set to lower case.
**
** @param [P] seqset [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetToLower (AjPSeqset seqset)
{
    ajint i;

    for (i=0; i < seqset->Size; i++)
	(void) ajSeqToLower (seqset->Seq[i]);

    return;
}

/* @func ajSeqsetToUpper ******************************************************
**
** Converts all sequences in a set to upper case.
**
** @param [P] seqset [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetToUpper (AjPSeqset seqset)
{
    ajint i;

    for (i=0; i < seqset->Size; i++)
	(void) ajSeqToUpper (seqset->Seq[i]);

    return;
}

/* @func ajSeqsetReverse ******************************************************
**
** Reverse complements all sequences in a sequence set.
**
** @param [P] thys [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetReverse (AjPSeqset thys)
{
    ajint i;
    ajint ibegin = thys->Begin;
    ajint iend = thys->End;

    ajDebug ("ajSeqsetReverse len: %d Begin: %d End: %d\n",
	     ajSeqsetLen(thys), thys->Begin, thys->End);

    if (ibegin)
	thys->End = -(ibegin);
    if (iend)
	thys->Begin = -(iend);

    for (i=0; i < thys->Size; i++)
	(void) ajSeqReverse (thys->Seq[i]);

    ajDebug ("  set result len: %d Begin: %d End: %d\n",
	     ajSeqsetLen(thys), thys->Begin, thys->End);

    return;
}

/* @func ajSeqsetSetRange *****************************************************
**
** Sets the start and end positions for a sequence set.
**
** @param [P] seq [AjPSeqset] Sequence set object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetRange (AjPSeqset seq, ajint ibegin, ajint iend)
{
    ajint i;

    ajDebug ("ajSeqsetSetRange (len: %d %d, %d)\n", seq->Len, ibegin, iend);

    if (ibegin)
	seq->Begin = ibegin;

    if (iend)
	seq->End = iend;

    for (i=0; i< seq->Size; i++)
    {
	if (ibegin)
	    seq->Seq[i]->Begin = ibegin;
	if (iend)
	    seq->Seq[i]->End   = iend;
    }

    ajDebug ("      result: (len: %d %d, %d)\n",
	     seq->Len, seq->Begin, seq->End);

    return;
}

/* @func ajSeqsetFill *********************************************************
**
** Fills a sequence set with gaps at the ends of any shorter sequences.
**
** @param [P] seq [AjPSeqset] Sequence set object to be set.
** @return [ajint] Number of gaps inserted
** @@
******************************************************************************/

ajint ajSeqsetFill (AjPSeqset seq)
{
    ajint i;
    ajint ifix = 0;
    ajint nfix = 0;
    ajint ilen;

    ajDebug ("ajSeqsetFill (len: %d)\n", seq->Len);

    for (i=0; i< seq->Size; i++)
	if (ajSeqLen(seq->Seq[i]) < seq->Len)
	{
	    nfix++;
	    ilen = seq->Len - ajSeqLen(seq->Seq[i]);
	    if (ilen > ifix)
		ifix = ilen;
	    ajStrFill (&seq->Seq[i]->Seq, seq->Len, '-');
	}

    ajDebug ("      result: (len: %d added: %d number of seqs fixed: nfix\n",
	     seq->Len, ifix, nfix);

    return ifix;
}

/* @func ajSeqsetIsNuc ********************************************************
**
** Tests whether a sequence set is nucleotide.
**
** @param [P] thys [AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
** @@
******************************************************************************/

AjBool ajSeqsetIsNuc (AjPSeqset thys)
{
    AjPSeq seq;

    if (ajStrMatchC(thys->Type, "N"))
	return ajTrue;

    seq = thys->Seq[0];
    if (!ajSeqTypeGapnuc(seq))
	return ajTrue;

    return ajFalse;
}

/* @func ajSeqsetIsRna ********************************************************
**
** Tests whether a sequence set is rna.
**
** @param [P] thys [AjPSeqset] Sequence set
** @return [AjBool] ajTrue for an rna sequence set.
** @@
******************************************************************************/

AjBool ajSeqsetIsRna (AjPSeqset thys)
{
    AjPSeq seq;

    seq = thys->Seq[0];
    if (!ajSeqTypeGaprna(seq))
	return ajTrue;

    return ajFalse;
}


/* @func ajSeqsetIsDna ********************************************************
**
** Tests whether a sequence set is dna.
**
** @param [P] thys [AjPSeqset] Sequence set
** @return [AjBool] ajTrue for an dna sequence set.
** @@
******************************************************************************/

AjBool ajSeqsetIsDna (AjPSeqset thys)
{
    AjPSeq seq;

    seq = thys->Seq[0];
    if (!ajSeqTypeGapdna(seq))
	return ajTrue;

    return ajFalse;
}


/* @func ajSeqsetIsProt *******************************************************
**
** Tests whether a sequence set is protein.
**
** @param [P] thys [AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a protein sequence set.
** @@
******************************************************************************/

AjBool ajSeqsetIsProt (AjPSeqset thys)
{
    AjPSeq seq;

    if (ajStrMatchC(thys->Type, "P"))
	return ajTrue;

    if (ajSeqsetIsNuc(thys))
	return ajFalse;

    seq = thys->Seq[0];
    if (!ajSeqTypeAnyprot(seq))
	return ajTrue;

    return ajFalse;
}

/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Sequence Set Casts ************************************************
**
** These functions examine the contents of a sequence set object and
** return some derived information. Some of them provide access to the
** internal components of a sequence set object. They are provided for
** programming convenience but should be used with caution.
**
******************************************************************************/

/* @func ajSeqsetLen **********************************************************
**
** Returns the length of a sequence set, which is the maximum sequence
** length in the set.
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [ajint] sequence set length.
** @@
******************************************************************************/

ajint ajSeqsetLen (AjPSeqset seq)
{
    return seq->Len;
}

/* @func ajSeqsetBegin ********************************************************
**
** Returns the sequence set start position, or 1 if no start has been set.
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqsetBegin (AjPSeqset seq)
{
    if (!seq->Begin)
	return 1;

    return ajSeqPosII(seq->Len, 1, seq->Begin);
}

/* @func ajSeqsetEnd **********************************************************
**
** Returns the sequence set end position, or the sequence length if no end
** has been set.
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqsetEnd (AjPSeqset seq)
{
    if (!seq->End)
	return (seq->Len);

    return ajSeqPosII(seq->Len, ajSeqsetBegin(seq), seq->End);
}

/* @func ajSeqsetSeq **********************************************************
**
** Returns the sequence data of a sequence in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [char*] Sequence as a NULL terminated string.
** @@
******************************************************************************/

char* ajSeqsetSeq (AjPSeqset seq, ajint i)
{
    if (i >= seq->Size)
	return NULL;

    return ajStrStr(seq->Seq[i]->Seq);
}

/* @func ajSeqsetSize *********************************************************
**
** Returns the number of sequences in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [ajint] sequence set size.
** @@
******************************************************************************/

ajint ajSeqsetSize (AjPSeqset seq)
{
    return seq->Size;
}

/* @func ajSeqsetName *********************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [AjPStr] sequence name as a string.
** @@
******************************************************************************/

AjPStr ajSeqsetName (AjPSeqset seq, ajint i)
{
    if (i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Name;
}

/* @func ajSeqsetWeight *******************************************************
**
** Returns the weight of a sequence in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetWeight (AjPSeqset seq, ajint i)
{
    if (i >= seq->Size)
	return 0.0;

    return seq->Seq[i]->Weight;
}

/* @func ajSeqsetTotweight ****************************************************
**
** Returns the weight of all sequences in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetTotweight (AjPSeqset seq)
{
    ajint i;
    float ret = 0.0;

    ajDebug("ajSeqsetTotweight Size %d\n", seq->Size);
    for (i=0; i < seq->Size; i++)
    {
	ret += seq->Seq[i]->Weight;
	ajDebug("seq %d weight %d\n", i, seq->Seq[i]->Weight);
    }

    return ret;
}

/* @func ajSeqsetGetName ******************************************************
**
** Returns the sequence name of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqset] Sequence set object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqsetGetName (AjPSeqset thys)
{
    ajDebug ("ajSeqsetGetName '%S'\n", thys->Name);

    if (ajStrLen(thys->Name))
      return thys->Name;

    return thys->Usa;
}

/* @func ajSeqsetGetSeq *******************************************************
**
** Returns one sequence from a sequence set.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqset] Sequence set object.
** @param [r] i [ajint] Sequence index number in set
** @return [AjPSeq] Sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqsetGetSeq (AjPSeqset thys, ajint i)
{
    ajDebug ("ajSeqsetGetSeq '%S' %d/%d\n", thys->Name,i, thys->Size);
    if (i >= thys->Size)
	return NULL;

    return thys->Seq[i];
}

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Constructors *********************************************
**
** All constructors return a new sequence by pointer. It is the
** responsibility of the user to first destroy any previous
** sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
******************************************************************************/

/* @func ajSeqNew *************************************************************
**
** Creates and initialises a sequence object.
**
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNew (void)
{
    return ajSeqNewL (0);
}

/* @func ajSeqNewL ************************************************************
**
** Creates and initialises a sequence object with a specified sequence length.
**
** @param [r] size [size_t] Reserved space for the sequence, including
**                          a trailing null character.
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewL (size_t size)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    pthis->Acc = ajStrNew();
    pthis->Sv = ajStrNew();
    pthis->Gi = ajStrNew();
    pthis->Tax = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Date = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc = ajStrNew();
    pthis->Usa = ajStrNew();
    pthis->Ufo = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Filename = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr = ajStrNew();
    if (size)
	pthis->Seq = ajStrNewL(size);
    else
	pthis->Seq = ajStrNew();

    pthis->Rev = ajFalse;
    pthis->EType = 0;
    pthis->Format = 0;
    pthis->Begin = 0;
    pthis->End = 0;
    pthis->Offset = 0;
    pthis->Offend = 0;
    pthis->Weight = 1.0;
    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();
    pthis->Selexdata = NULL;

    return pthis;
}

/* @func ajSeqNewS ************************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence.
**
** @param [r] seq [AjPSeq] Old sequence object
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewS (AjPSeq seq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    (void) ajStrAssS(&pthis->Name, seq->Name);
    (void) ajStrAssS(&pthis->Acc, seq->Acc);
    (void) ajStrAssS(&pthis->Sv, seq->Sv);
    (void) ajStrAssS(&pthis->Gi, seq->Gi);
    (void) ajStrAssS(&pthis->Tax, seq->Tax);
    (void) ajStrAssS(&pthis->Type, seq->Type);
    (void) ajStrAssS(&pthis->Db, seq->Db);
    (void) ajStrAssS(&pthis->Full, seq->Full);
    (void) ajStrAssS(&pthis->Date, seq->Date);
    (void) ajStrAssS(&pthis->Desc, seq->Desc);
    (void) ajStrAssS(&pthis->Doc, seq->Doc);
    (void) ajStrAssS(&pthis->Usa, seq->Usa);
    (void) ajStrAssS(&pthis->Ufo, seq->Ufo);
    (void) ajStrAssS(&pthis->Formatstr, seq->Formatstr);
    (void) ajStrAssS(&pthis->Filename, seq->Filename);
    (void) ajStrAssS(&pthis->Entryname, seq->Entryname);
    (void) ajStrAssS(&pthis->Seq, seq->Seq);

    if(seq->TextPtr)
	(void) ajStrAssS(&pthis->TextPtr, seq->TextPtr);


    pthis->Rev = seq->Rev;
    pthis->EType = seq->EType;
    pthis->Format = seq->Format;
    pthis->Begin = seq->Begin;
    pthis->End = seq->End;
    pthis->Weight = seq->Weight;
    ajListstrDel(&pthis->Acclist);
    pthis->Acclist = ajListstrNew();
    ajListstrDel(&pthis->Keylist);
    pthis->Keylist = ajListstrNew();
    ajListstrDel(&pthis->Taxlist);
    pthis->Taxlist = ajListstrNew();

    if(seq->Selexdata)
	pthis->Selexdata = seqSelexClone(seq->Selexdata);

    return pthis;
}


/* @func ajStockholmNew *******************************************************
**
** Creates and initialises a Stockholm object.
**
** @param [r] i [ajint] Number of sequences
** @return [AjPStockholm] New sequence object.
** @@
******************************************************************************/

AjPStockholm ajStockholmNew(ajint i)
{
    AjPStockholm thys=NULL;

    AJNEW0(thys);

    thys->id  = ajStrNew();
    thys->ac  = ajStrNew();
    thys->de  = ajStrNew();
    thys->au  = ajStrNew();
    thys->al  = ajStrNew();
    thys->tp  = ajStrNew();
    thys->se  = ajStrNew();
    thys->bm  = ajStrNew();
    thys->dc  = ajStrNew();
    thys->dr  = ajStrNew();
    thys->cc  = ajStrNew();
    thys->gs  = ajStrNew();
    thys->ref = ajStrNew();
    thys->sacons  = ajStrNew();
    thys->sscons  = ajStrNew();

    thys->n = i;

    AJCNEW0(thys->name,i);
    AJCNEW0(thys->str,i);

    for(i=0;i<thys->n;++i)
    {
	thys->name[i] = ajStrNew();
	thys->str[i]  = ajStrNew();
    }

    return thys;
}



/* @func ajStockholmDel *******************************************************
**
** Deletes a Stockholm object.
**
** @param [w] thys [AjPStockholm*] Stockholm object
** @return [void]
** @@
******************************************************************************/

void ajStockholmDel(AjPStockholm *thys)
{
    AjPStockholm pthis = NULL;
    ajint i;

    if(!thys)
	return;
    pthis = *thys;
    if(!pthis)
	return;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->al);
    ajStrDel(&pthis->tp);
    ajStrDel(&pthis->se);
    ajStrDel(&pthis->bm);
    ajStrDel(&pthis->dc);
    ajStrDel(&pthis->dr);
    ajStrDel(&pthis->cc);
    ajStrDel(&pthis->gs);
    ajStrDel(&pthis->ref);
    ajStrDel(&pthis->sacons);
    ajStrDel(&pthis->sscons);

    for(i=0;i<pthis->n;++i)
    {
	ajStrDel(&pthis->name[i]);
	ajStrDel(&pthis->str[i]);
    }

    AJFREE(pthis->name);
    AJFREE(pthis->str);
    AJFREE(pthis);

    return;
}


/* @func ajStockholmdataNew ***************************************************
**
** Creates and initialises a Stockholm data object.
**
** @return [AjPStockholmdata] New sequence object.
** @@
******************************************************************************/

AjPStockholmdata ajStockholmdataNew(void)
{
    AjPStockholmdata thys=NULL;

    AJNEW0(thys);

    thys->id  = ajStrNew();
    thys->ac  = ajStrNew();
    thys->de  = ajStrNew();
    thys->au  = ajStrNew();
    thys->al  = ajStrNew();
    thys->tp  = ajStrNew();
    thys->se  = ajStrNew();
    thys->bm  = ajStrNew();
    thys->dc  = ajStrNew();
    thys->dr  = ajStrNew();
    thys->cc  = ajStrNew();
    thys->gs  = ajStrNew();
    thys->ref = ajStrNew();
    thys->sacons  = ajStrNew();
    thys->sscons  = ajStrNew();

    return thys;
}



/* @func ajStockholmdataDel ***************************************************
**
** Deletes a Stockholm data object.
**
** @param [w] thys [AjPStockholmdata*] Stockholm object
** @return [void]
** @@
******************************************************************************/

void ajStockholmdataDel(AjPStockholmdata *thys)
{
    AjPStockholmdata pthis = NULL;

    if(!thys)
	return;
    pthis = *thys;
    if(!pthis)
	return;

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->al);
    ajStrDel(&pthis->tp);
    ajStrDel(&pthis->se);
    ajStrDel(&pthis->bm);
    ajStrDel(&pthis->dc);
    ajStrDel(&pthis->dr);
    ajStrDel(&pthis->cc);
    ajStrDel(&pthis->gs);
    ajStrDel(&pthis->ref);
    ajStrDel(&pthis->sacons);
    ajStrDel(&pthis->sscons);

    AJFREE(pthis);

    return;
}



/* @func ajSelexSQNew *********************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @return [AjPSelexSQ] New sequence object.
** @@
******************************************************************************/

AjPSelexSQ ajSelexSQNew()
{
    AjPSelexSQ thys=NULL;

    AJNEW0(thys);

    thys->name   = ajStrNew();
    thys->source = ajStrNew();
    thys->ac     = ajStrNew();
    thys->de     = ajStrNew();

    return thys;
}


/* @func ajSelexNew ***********************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @param [r] n [ajint] Number of sequences
** @return [AjPSelex] New sequence object.
** @@
******************************************************************************/

AjPSelex ajSelexNew(ajint n)
{
    AjPSelex thys=NULL;
    ajint    i;

    AJNEW0(thys);
    thys->id = ajStrNew();
    thys->ac = ajStrNew();
    thys->de = ajStrNew();
    thys->au = ajStrNew();
    thys->cs = ajStrNew();
    thys->rf = ajStrNew();
    thys->n  = n;

    AJCNEW(thys->name,n);
    AJCNEW(thys->str,n);
    AJCNEW(thys->ss,n);
    AJCNEW(thys->sq,n);

    for(i=0;i<n;++i)
    {
	thys->name[i] = ajStrNew();
	thys->str[i]  = ajStrNew();
	thys->ss[i]   = ajStrNew();
	thys->sq[i]   = ajSelexSQNew();
    }

    return thys;
}


/* @func ajSelexdataNew *******************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @return [AjPSelexdata] New sequence object.
** @@
******************************************************************************/

AjPSelexdata ajSelexdataNew(void)
{
    AjPSelexdata thys=NULL;

    AJNEW0(thys);
    thys->id = ajStrNew();
    thys->ac = ajStrNew();
    thys->de = ajStrNew();
    thys->au = ajStrNew();
    thys->cs = ajStrNew();
    thys->rf = ajStrNew();

    thys->name = ajStrNew();
    thys->str  = ajStrNew();
    thys->ss   = ajStrNew();
    thys->sq   = ajSelexSQNew();

    return thys;
}



/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Destructors **********************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
******************************************************************************/

/* @func ajSeqDel *************************************************************
**
** Deletes a sequence object.
**
** @param [wP] pthis [AjPSeq*] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajSeqDel (AjPSeq* pthis)
{
    AjPSeq thys = pthis ? *pthis : 0;
    AjPStr ptr=NULL;

    if (!pthis)
	return;
    if (!*pthis)
	return;

    ajStrDel (&thys->Name);
    ajStrDel (&thys->Acc);
    ajStrDel (&thys->Sv);
    ajStrDel (&thys->Gi);
    ajStrDel (&thys->Tax);
    ajStrDel (&thys->Type);
    ajStrDel (&thys->Db);
    ajStrDel (&thys->Full);
    ajStrDel (&thys->Date);
    ajStrDel (&thys->Desc);
    ajStrDel (&thys->Doc);
    ajStrDel (&thys->Usa);
    ajStrDel (&thys->Ufo);
    ajStrDel (&thys->Formatstr);
    ajStrDel (&thys->Filename);
    ajStrDel (&thys->Entryname);
    ajStrDel (&thys->TextPtr);
    ajStrDel (&thys->Seq);

    if(thys->Fttable)
	ajFeattableDel(&thys->Fttable);

    while(ajListstrPop(thys->Acclist,&ptr))
	ajStrDel(&ptr);
    ajListDel(&thys->Acclist);

    while(ajListstrPop(thys->Keylist,&ptr))
	ajStrDel(&ptr);
    ajListDel(&thys->Keylist);

    while(ajListstrPop(thys->Taxlist,&ptr))
	ajStrDel(&ptr);
    ajListDel(&thys->Taxlist);

    if(thys->Selexdata)
	ajSelexdataDel(&thys->Selexdata);
    if(thys->Stock)
	ajStockholmdataDel(&thys->Stock);

    AJFREE (*pthis);
    return;
}


/* @func ajSelexSQDel *********************************************************
**
** Deletes a Selex object.
**
** @param [wP] thys [AjPSelexSQ*] Selex #=SQ object
** @return [void]
** @@
******************************************************************************/

void ajSelexSQDel(AjPSelexSQ *thys)
{
    AjPSelexSQ pthis = *thys;

    if(!thys || !pthis)
	return;

    ajStrDel(&pthis->name);
    ajStrDel(&pthis->source);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);

    AJFREE(pthis);
    *thys = NULL;

    return;
}


/* @func ajSelexDel ***********************************************************
**
** Deletes a Selex object.
**
** @param [wP] thys [AjPSelex*] Selex object
** @return [void]
** @@
******************************************************************************/

void ajSelexDel(AjPSelex *thys)
{
    AjPSelex pthis = *thys;
    ajint    i;
    ajint    n;

    if(!thys || !pthis)
	return;

    n = pthis->n;
    for(i=0;i<n;++i)
    {
	ajStrDel(&pthis->name[i]);
	ajStrDel(&pthis->str[i]);
	ajStrDel(&pthis->ss[i]);
	ajSelexSQDel(&pthis->sq[i]);
    }
    if(n)
    {
	AJFREE(pthis->name);
	AJFREE(pthis->str);
	AJFREE(pthis->ss);
	AJFREE(pthis->sq);
    }

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->cs);
    ajStrDel(&pthis->rf);

    AJFREE(pthis);
    *thys = NULL;

    return;
}


/* @func ajSelexdataDel *******************************************************
**
** Deletes a Selex data object.
**
** @param [wP] thys [AjPSelexdata*] Selex data object
** @return [void]
** @@
******************************************************************************/

void ajSelexdataDel(AjPSelexdata *thys)
{
    AjPSelexdata pthis = *thys;

    if(!thys || !pthis)
	return;


    ajStrDel(&pthis->name);
    ajStrDel(&pthis->str);
    ajStrDel(&pthis->ss);
    ajSelexSQDel(&pthis->sq);

    ajStrDel(&pthis->id);
    ajStrDel(&pthis->ac);
    ajStrDel(&pthis->de);
    ajStrDel(&pthis->au);
    ajStrDel(&pthis->cs);
    ajStrDel(&pthis->rf);

    AJFREE(pthis);
    *thys = NULL;

    return;
}


/* @func ajSeqClear ***********************************************************
**
** Resets all data for a sequence object so that it can be reused.
**
** @param [uP] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqClear (AjPSeq thys)
{
    AjPStr ptr=NULL;

    (void) ajStrClear (&thys->Name);
    (void) ajStrClear (&thys->Acc);
    (void) ajStrClear (&thys->Sv);
    (void) ajStrClear (&thys->Gi);
    (void) ajStrClear (&thys->Tax);
    (void) ajStrClear (&thys->Type);
    (void) ajStrClear (&thys->Db);
    (void) ajStrClear (&thys->Full);
    (void) ajStrClear (&thys->Date);
    (void) ajStrClear (&thys->Desc);
    (void) ajStrClear (&thys->Doc);
    (void) ajStrClear (&thys->Usa);
    (void) ajStrClear (&thys->Ufo);

    (void) ajStrClear (&thys->Formatstr);
    (void) ajStrClear (&thys->Filename);
    (void) ajStrClear (&thys->Entryname);
    (void) ajStrClear (&thys->TextPtr);
    (void) ajStrClear (&thys->Seq);

    thys->Begin=0;
    thys->End=0;
    thys->Rev = ajFalse;

    while(ajListstrPop(thys->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(thys->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(thys->Taxlist,&ptr))
	ajStrDel(&ptr);

    ajFeattableDel(&thys->Fttable);

    return;
}


/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Modifiers ************************************************
**
** These functions use the contents of a sequence object and update them.
**
******************************************************************************/

/* @func ajSeqAssName *********************************************************
**
** Assigns the sequence name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssName (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Name, str);

    return;
}


/* @func ajSeqAssNameC ********************************************************
**
** Assigns the sequence name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] Name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssNameC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Name, text);

    return;
}

/* @func ajSeqAssAcc **********************************************************
**
** Assigns the sequence accession number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Accession number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssAcc (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Acc, str);

    return;
}

/* @func ajSeqAssAccC *********************************************************
**
** Assigns the sequence accession number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] Accession number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssAccC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Acc, text);

    return;
}

/* @func ajSeqAssSv ***********************************************************
**
** Assigns the sequence version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] SeqVersion number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSv (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Sv, str);

    return;
}

/* @func ajSeqAssSvC **********************************************************
**
** Assigns the sequence version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] SeqVersion number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSvC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Sv, text);

    return;
}

/* @func ajSeqAssGi ***********************************************************
**
** Assigns the GI version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] GI number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssGi (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Gi, str);

    return;
}

/* @func ajSeqAssGiC **********************************************************
**
** Assigns the GI version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] GI number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssGiC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Gi, text);

    return;
}

/* @func ajSeqAssUfo **********************************************************
**
** Assigns the sequence feature full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] UFO as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUfo (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Ufo, str);

    return;
}

/* @func ajSeqAssUfoC *********************************************************
**
** Assigns the sequence feature name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] UFO as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUfoC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Ufo, text);

    return;
}

/* @func ajSeqAssUsa **********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] USA as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUsa (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Usa, str);

    return;
}

/* @func ajSeqAssUsaC *********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] USA as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUsaC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Usa, text);

    return;
}

/* @func ajSeqAssEntry ********************************************************
**
** Assigns the sequence entry name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Entry name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssEntry (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Entryname, str);

    return;
}

/* @func ajSeqAssEntryC *******************************************************
**
** Assigns the sequence entryname.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] Entry name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssEntryC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Entryname, text);

    return;
}

/* @func ajSeqAssFull *********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Full name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFull (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Full, str);

    return;
}

/* @func ajSeqAssFullC ********************************************************
**
** Assigns the sequence name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] Full name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFullC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Full, text);

    return;
}

/* @func ajSeqAssFile *********************************************************
**
** Assigns the sequence file name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] File name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFile (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Filename, str);

    return;
}

/* @func ajSeqAssFileC ********************************************************
**
** Assigns the sequence filename.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] File name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFileC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Filename, text);

    return;
}


/* @func ajSeqAssSeq **********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] New sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeq (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Seq, str);
    thys->Begin = 0;
    thys->End = 0;
    thys->Rev = ajFalse;

    return;
}

/* @func ajSeqAssSeqC *********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] New sequence as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeqC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Seq, text);

    return;
}

/* @func ajSeqAssSeqCI ********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] New sequence as a C character string.
** @param [r] ilen [ajint] Numbur of characters to use
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeqCI (AjPSeq thys, char* text, ajint ilen)
{
    (void) ajStrAssCI(&thys->Seq, text, ilen);

    return;
}

/* @func ajSeqAssDesc *********************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] New description as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssDesc (AjPSeq thys, AjPStr str)
{
    (void) ajStrAssS (&thys->Desc, str);

    return;
}

/* @func ajSeqAssDescC ********************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] New description as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssDescC (AjPSeq thys, char* text)
{
    (void) ajStrAssC(&thys->Desc, text);

    return;
}

/* @func ajSeqMod *************************************************************
**
** Makes a sequence modifiable by making sure there is no duplicate
** copy of the sequence.
**
** @param [uP] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqMod (AjPSeq thys)
{
    (void) ajStrMod (&thys->Seq);

    return;
}

/* @func ajSeqReplace *********************************************************
**
** Replaces a sequence with the contents of a string.
**
** @param [uP] thys [AjPSeq] Sequence
** @param [P] seq [AjPStr] New sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReplace (AjPSeq thys, AjPStr seq)
{
    (void) ajStrAssS (&thys->Seq, seq);
    thys->Begin = 0;
    thys->End = 0;
    thys->Rev = ajFalse;

    return;
}

/* @func ajSeqReplaceC ********************************************************
**
** Replaces a sequence with the contents of a C text string.
**
** @param [uP] thys [AjPSeq] Sequence
** @param [P] seq [char*] New sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReplaceC (AjPSeq thys, char* seq)
{
    (void) ajStrAssC (&thys->Seq, seq);
    thys->Begin = 0;
    thys->End = 0;
    thys->Rev = ajFalse;
    return;
}

/* @func ajSeqSetRange ********************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set).
**
** @param [P] seq [AjPSeq] Sequence object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqSetRange (AjPSeq seq, ajint ibegin, ajint iend)
{
    ajDebug ("ajSeqSetRange (len: %d %d..%d old %d..%d)\n",
	     ajSeqLen(seq), ibegin, iend,
	     seq->Begin, seq->End);

    if (ibegin && !seq->Begin)
	seq->Begin = ibegin;

    if (iend && !seq->End)
	seq->End = iend;

    ajDebug ("      result: (len: %d %d..%d)\n",
	     ajSeqLen(seq), seq->Begin, seq->End);

    return;
}

/* @func ajSeqMakeUsa *********************************************************
**
** Sets the USA for a sequence.
**
** @param [P] thys [AjPSeq] Sequence object to be set.
** @param [P] seqin [AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqMakeUsa (AjPSeq thys, AjPSeqin seqin)
{
    AjPStr tmpstr = NULL;

    ajDebug ("ajSeqMakeUsa (Name <%S> Formatstr <%S> Db <%S> "
	     "Entryname <%S> Filename <%S>)\n",
	     thys->Name, thys->Formatstr, thys->Db,
	     thys->Entryname, thys->Filename);

    ajSeqTrace (thys);

    if (seqin)
	ajSeqinTrace (seqin);

    if (ajStrLen(thys->Db))
	ajFmtPrintS (&thys->Usa, "%S-id:%S", thys->Db, thys->Entryname);
    else
    {
	/*ajFmtPrintS (&thys->Usa, "%S::%S (%S)",
	  thys->Formatstr, thys->Filename, thys->Entryname);*/
	if (ajStrLen(thys->Entryname))
	    ajFmtPrintS (&thys->Usa, "%S::%S:%S", thys->Formatstr,
			 thys->Filename,thys->Entryname);
	else
	    ajFmtPrintS (&thys->Usa, "%S::%S", thys->Formatstr,
			 thys->Filename);

    }

    ajFmtPrintS (&tmpstr, "[");

    if (thys->Rev)
    {
      if (thys->End)
	ajFmtPrintAppS (&tmpstr, "%d", -thys->End);

      ajFmtPrintAppS (&tmpstr, ":");

      if (thys->Begin)
	ajFmtPrintAppS (&tmpstr, "%d", -thys->Begin);

      ajFmtPrintAppS (&tmpstr, ":r");
    }
    else
    {
      if (thys->Begin)
	ajFmtPrintAppS (&tmpstr, "%d", thys->Begin);

      ajFmtPrintAppS (&tmpstr, ":");

      if (thys->End)
	ajFmtPrintAppS (&tmpstr, "%d", thys->End);
    }

    ajFmtPrintAppS (&tmpstr, "]");

    if (ajStrLen(tmpstr) > 3)
      ajStrApp (&thys->Usa, tmpstr);

    ajStrDel(&tmpstr);
    ajDebug ("      result: <%S>\n",
	     thys->Usa);

    return;
}

/* @func ajSeqToUpper *********************************************************
**
** Converts a sequence to upper case.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqToUpper (AjPSeq thys)
{
    (void) ajStrToUpper(&thys->Seq);

    return;
}

/* @func ajSeqToLower *********************************************************
**
** Converts a sequence to lower case.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqToLower (AjPSeq thys)
{
    (void) ajStrToLower(&thys->Seq);

    return;
}

/* @func ajSeqReverse *********************************************************
**
** Reverses and complements a nucleotide sequence.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReverse (AjPSeq thys)
{
    ajint ibegin = thys->Begin;
    ajint iend = thys->End;

    ajDebug ("ajSeqReverse len: %d Begin: %d End: %d\n",
	     ajSeqLen(thys), thys->Begin, thys->End);

    thys->End = -(ibegin);
    thys->Begin = -(iend);

    (void) ajSeqReverseStr(&thys->Seq);

    ajDebug ("      result len: %d Begin: %d End: %d\n",
	     ajSeqLen(thys), thys->Begin, thys->End);

    return;
}

/* @func ajSeqReverseStr ******************************************************
**
** Reverses and complements a nucleotide sequence provided as a string.
**
** @param [u] pthis [AjPStr*] Sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqReverseStr (AjPStr* pthis)
{
    char *cp;
    char *cq;
    char tmp;

    (void) ajStrMod (pthis);

    cp = ajStrStr(*pthis);
    cq = cp + ajStrLen(*pthis) - 1;

    while (cp < cq)
    {
	tmp = ajSeqBaseComp(*cp);
	*cp = ajSeqBaseComp(*cq);
	*cq = tmp;
	cp++;
	cq--;
    }
    if (cp == cq)
	*cp = ajSeqBaseComp(*cp);

    return;
}

/* @func ajSeqCompOnly ********************************************************
**
** Complements but does not reverse a nucleotide sequence.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqCompOnly (AjPSeq thys)
{
    ajSeqCompOnlyStr (&thys->Seq);

    return;
}


/* @func ajSeqRevOnly *********************************************************
**
** Reverses but does not complement a nucleotide sequence.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqRevOnly (AjPSeq thys)
{
    ajint ibegin = thys->Begin;
    ajint iend = thys->End;

    ajDebug ("ajSeqRevOnly len: %d Begin: %d End: %d\n",
	     ajSeqLen(thys), thys->Begin, thys->End);

    if (ibegin)
	thys->End = -(ibegin);
    if (iend)
	thys->Begin = -(iend);

    (void) ajStrRev (&thys->Seq);

    ajDebug (" only result len: %d Begin: %d End: %d\n",
	     ajSeqLen(thys), thys->Begin, thys->End);

    return;
}

/* @func ajSeqCompOnlyStr *****************************************************
**
** Complements but does not reverse a nucleotide sequence provided as a string.
**
** @param [u] pthis [AjPStr*] Sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqCompOnlyStr (AjPStr* pthis)
{
    char *cp;

    (void) ajStrMod (pthis);

    cp = ajStrStr(*pthis);

    while (*cp)
    {
	*cp = ajSeqBaseComp(*cp);
	cp++;
    }

    return;
}

/* @func ajSeqBaseComp ********************************************************
**
** Complements a nucleotide base.
**
** @param [u] base [char] Base character.
** @return [char] Complementary base.
** @@
******************************************************************************/

char ajSeqBaseComp (char base)
{
    static char fwd[]="ACGTURYWSMKBDHVNXacgturywsmkbdhvnx";
    static char rev[]="TGCAAYRWSKMVHDBNXtgcaayrwskmvhdbnx";
    char *cp;
    char *cq;

    cp = strchr (fwd,base);
    if (cp)
    {
	cq = cp - fwd + rev;
	return *cq;
    }

    return base;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Operators ************************************************
**
** These functions use the contents of a sequence but do not make any changes.
**
******************************************************************************/

/* @func ajSeqIsNuc ***********************************************************
**
** Tests whether a sequence is nucleotide.
**
** @param [P] thys [AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
** @@
******************************************************************************/

AjBool ajSeqIsNuc (AjPSeq thys)
{
    ajDebug ("ajSeqIsNuc Type '%S'\n", thys->Type);

    if (ajStrMatchC(thys->Type, "N"))
	return ajTrue;

    if (ajStrMatchC(thys->Type, "P"))
	return ajFalse;

    if (!ajSeqTypeGapnuc(thys))		/* returns char 0 on success */
	return ajTrue;

    ajDebug ("ajSeqIsNuc failed\n", thys->Type);

    return ajFalse;
}

/* @func ajSeqIsProt **********************************************************
**
** Tests whether a sequence is protein.
**
** @param [P] thys [AjPSeq] Sequence
** @return [AjBool] ajTrue for a protein sequence.
** @@
******************************************************************************/

AjBool ajSeqIsProt (AjPSeq thys)
{
    ajDebug ("ajSeqIsProt Type '%S'\n", thys->Type);

    if (ajStrMatchC(thys->Type, "P"))
	return ajTrue;

    if (ajStrMatchC(thys->Type, "N"))
	return ajFalse;

    if (!ajSeqTypeAnyprot(thys))	/* returns char 0 on success */
	return ajTrue;

    ajDebug ("ajSeqIsProt failed\n", thys->Type);

    return ajFalse;
}

/* @func ajIsAccession ********************************************************
**
** Tests whether a string is a potential sequence accession number.
** The current definition is one or two alpha characters,
** then a possible underscore (for REFSEQ accessions),
** followed by a string of digits and a minimum length of 6.
**
** Revised for new Swiss-Prot accession number format AnXXXn
**
** @param [P] accnum [AjPStr] String to be tested
** @return [AjBool] ajTrue if the string is a possible accession number.
** @@
******************************************************************************/

AjBool ajIsAccession (AjPStr accnum)
{
    ajint i;
    char *cp;

    if (!accnum)
	return ajFalse;

    i = ajStrLen(accnum);
    if (i < 6)
	return ajFalse;

    cp = ajStrStr(accnum);

    /* must have an alphabetic start */

    if (!isalpha((ajint)*cp++))
	return ajFalse;

    /* two choices for the next character */

    if (isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
	cp++;

	if (*cp == '_') cp++;	/* REFSEQ NM_nnnnnn */

	while(*cp)
	    if(isdigit((ajint)*cp))
		++cp;
	    else
		return ajFalse;

	return ajTrue;
    }
    else if (isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
	cp++;				/* or SWISS AnXXXn */

	for (i=0; i<3; i++)
	    if (isalpha((ajint)*cp) || isdigit((ajint)*cp))
		cp++;
	    else
		return ajFalse;

	if (!isdigit((ajint)*cp))
	    return ajFalse;

	while(*cp)
	    if(isdigit((ajint)*cp))
		++cp;
	    else
		return ajFalse;

	return ajTrue;
    }

    return ajFalse;
}


/* @func ajIsSeqversion *******************************************************
**
** Tests whether a string is a potential sequence version number.
** The current definition is an accession number, followed by a dot and
** a number.
**
** Revised for new Swiss-Prot accession number format AnXXXn
** Revised for REFSEQ accession number format NM_nnnnnn
**
** @param [P] sv [AjPStr] String to be tested
** @return [AjPStr] accession number part of the string if successful
** @@
******************************************************************************/

AjPStr ajIsSeqversion (AjPStr sv)
{
    ajint i;
    char *cp;
    AjBool dot = ajFalse;	/* have we found the '.' */
    AjBool v = 0;		/* number of digits of version after '.' */
    static AjPStr accnum=NULL;

    if (!sv)
	return NULL;

    i = ajStrLen(sv);
    if (i < 8)
	return NULL;

    cp = ajStrStr(sv);
    ajStrAssCL (&accnum, "", 12);

    /* must have an alphabetic start */

    if (!isalpha((ajint)*cp))
	return NULL;

    ajStrAppK(&accnum, *cp++);

    /* two choices for the next character */

    if (isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
        ajStrAppK(&accnum, *cp);
	cp++;

	if (*cp == '_') cp++;	/* REFSEQ NM_nnnnnn */

	while(*cp)			/* optional trailing .version */
	{
	    if(isdigit((ajint)*cp) || *cp=='.')
	    {
	      if (*cp == '.')
	      {
		if (dot) return NULL; /* one '.' only */
		dot = ajTrue;
	      }
	      else
	      {
		if (dot)
		  v++;
		else
		  ajStrAppK(&accnum, *cp);
	      }
		++cp;
	    }
	    else
		return NULL;
	}
	if (v)
	  return accnum;
	else
	  return NULL;
    }
    else if (isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
                                        /* or SWISS AnXXXn */
        ajStrAppK(&accnum, *cp);
	cp++;

	for (i=0; i<3; i++)
	    if (isalpha((ajint)*cp) || isdigit((ajint)*cp))
	    {
	        ajStrAppK(&accnum, *cp);
		cp++;
	    }
	    else
		return NULL;

	if (!isdigit((ajint)*cp))
	    return NULL;

	while(*cp)			/* optional trailing .version */
	{
	    if(isdigit((ajint)*cp) || *cp=='.')
	    {
	      if (*cp == '.')
	      {
		if (dot) return NULL; /* one '.' only */
		dot = ajTrue;
	      }
	      else
	      {
		if (dot)
		  v++;
		else
		  ajStrAppK(&accnum, *cp);
	      }
		++cp;
	    }
	    else
		return NULL;
	}
	if (v)
	  return accnum;
	else
	  return NULL;
    }

    return NULL;
}

/* @func ajSeqTrace ***********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

void ajSeqTrace (AjPSeq seq)
{
    AjIList it;
    AjPStr cur;

    ajDebug ("Sequence trace\n");
    ajDebug ( "==============\n\n");
    ajDebug ( "  Name: '%S'\n", seq->Name);
    if (ajStrLen(seq->Acc))
	ajDebug ( "  Accession: '%S'\n", seq->Acc);
    if (ajListLength(seq->Acclist))
    {
	ajDebug ( "  Acclist: (%d) ", ajListLength(seq->Acclist));
	it = ajListIter(seq->Acclist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" %S", cur);

	ajListIterFree (it);
	ajDebug(" \n");
    }
    if (ajStrLen(seq->Sv))
	ajDebug ( "  SeqVersion: '%S'\n", seq->Sv);
    if (ajStrLen(seq->Gi))
	ajDebug ( "  GI Version: '%S'\n", seq->Gi);
    if (ajStrLen(seq->Type))
	ajDebug ( "  Type: '%S' (%d)\n", seq->Type, seq->EType);
    if (ajStrLen(seq->Desc))
	ajDebug ( "  Description: '%S'\n", seq->Desc);
    if (ajStrLen(seq->Tax))
	ajDebug ( "  Taxonomy: '%S'\n", seq->Tax);
    if (ajListLength(seq->Taxlist))
    {
	ajDebug ( "  Taxlist: (%d)", ajListLength(seq->Taxlist));
	it = ajListIter(seq->Taxlist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" '%S'", cur);

	ajListIterFree (it);
	ajDebug("\n");
    }
    if (ajListLength(seq->Keylist))
    {
	ajDebug ( "  Keywordlist: (%d)", ajListLength(seq->Keylist));
	it = ajListIter(seq->Keylist);
	while ((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" '%S'", cur);

	ajListIterFree (it);
	ajDebug("\n");
    }
    if (ajSeqLen(seq))
	ajDebug ( "  Length: %d\n", ajSeqLen(seq));
    if (seq->Rev)
	ajDebug ( "     Rev: %B\n", seq->Rev);
    if (seq->Begin)
	ajDebug ( "   Begin: %d\n", ajSeqBegin(seq));
    if (seq->End)
	ajDebug ( "     End: %d\n", ajSeqEnd(seq));
    if (seq->Offset)
	ajDebug ( "  Offset: %d\n", seq->Offset);
    if (seq->Offend)
	ajDebug ( "  Offend: %d\n", seq->Offend);
    if (ajStrSize(seq->Seq))
	ajDebug ( "  Reserved: %d\n", ajStrSize(seq->Seq));
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
    if (seq->Fttable)
	ajDebug ( "  Fttable: exists\n");
    if (ajStrLen(seq->Formatstr))
	ajDebug ( "  Input format: '%S' (%d)\n", seq->Formatstr, seq->Format);
    if (ajStrLen(seq->Filename))
	ajDebug ( "  Filename: '%S'\n", seq->Filename);
    if (ajStrLen(seq->Entryname))
	ajDebug ( "  Entryname: '%S'\n", seq->Entryname);
    if (seq->Weight)
	ajDebug ( "  Weight: %.3f\n", seq->Weight);
    if (ajStrLen(seq->Doc))
	ajDebug ( "  Documentation:...\n%S\n", seq->Doc);

    return;
}

/* @func ajSeqinTrace *********************************************************
**
** Debug calls to trace the data in a sequence input object.
**
** @param [r] thys [AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqinTrace (AjPSeqin thys)
{
    ajDebug ("Sequence input trace\n");
    ajDebug ( "====================\n\n");
    ajDebug ( "  Name: '%S'\n", thys->Name);
    if (ajStrLen(thys->Acc))
	ajDebug ( "  Accession: '%S'\n", thys->Acc);
    if (ajStrLen(thys->Inputtype))
	ajDebug ( "  Inputtype: '%S'\n", thys->Inputtype);
    if (ajStrLen(thys->Desc))
	ajDebug ( "  Description: '%S'\n", thys->Desc);
    if (ajStrLen(thys->Inseq))
	ajDebug ( "  Inseq len: %d\n", ajStrLen(thys->Inseq));
    if (thys->Rev)
	ajDebug ( "     Rev: %B\n", thys->Rev);
    if (thys->Begin)
	ajDebug ( "   Begin: %d\n", thys->Begin);
    if (thys->End)
	ajDebug ( "     End: %d\n", thys->End);
    if (ajStrLen(thys->Db))
	ajDebug ( "  Database: '%S'\n", thys->Db);
    if (ajStrLen(thys->Full))
	ajDebug ( "  Full name: '%S'\n", thys->Full);
    if (ajStrLen(thys->Date))
	ajDebug ( "  Date: '%S'\n", thys->Date);
    if (ajListLength(thys->List))
	ajDebug ( "  List: (%d)\n", ajListLength(thys->List));
    if (thys->Filebuff)
	ajDebug ( "  Filebuff: %F (%ld)\n",
		 ajFileBuffFile(thys->Filebuff),
		 ajFileTell(ajFileBuffFile(thys->Filebuff)));
    if (ajStrLen(thys->Usa))
	ajDebug ( "  Usa: '%S'\n", thys->Usa);
    if (ajStrLen(thys->Ufo))
	ajDebug ( "  Ufo: '%S'\n", thys->Ufo);
    if (thys->Fttable)
	ajDebug ( "  Fttable: exists\n");
    if (thys->Ftquery)
	ajDebug ( "  Ftquery: exists\n");
    if (ajStrLen(thys->Formatstr))
	ajDebug ( "  Input format: '%S' (%d)\n", thys->Formatstr,
		 thys->Format);
    if (ajStrLen(thys->Filename))
	ajDebug ( "  Filename: '%S'\n", thys->Filename);
    if (ajStrLen(thys->Entryname))
	ajDebug ( "  Entryname: '%S'\n", thys->Entryname);
    if (thys->Search)
	ajDebug ( "  Search: %B\n", thys->Search);
    if (thys->Single)
	ajDebug ( "  Single: %B\n", thys->Single);
    if (thys->Features)
	ajDebug ( "  Features: %B\n", thys->Features);
    if (thys->IsNuc)
	ajDebug ( "  IsNuc: %B\n", thys->IsNuc);
    if (thys->IsProt)
	ajDebug ( "  IsProt: %B\n", thys->IsProt);
    if (thys->Count)
	ajDebug ( "  Count: %d\n", thys->Count);
    if (thys->Filecount)
	ajDebug ( "  Filecount: %d\n", thys->Filecount);
    if (thys->Fpos)
	ajDebug ( "  Fpos: %l\n", thys->Fpos);
    if (thys->Query)
	ajSeqQueryTrace (thys->Query);
    if (thys->Data)
	ajDebug ( "  Data: exists\n");
    if (ajStrLen(thys->Doc))
	ajDebug ( "  Documentation:...\n%S\n", thys->Doc);

    return;
}

/* @func ajSeqQueryTrace ******************************************************
**
** Debug calls to trace the data in a sequence query object.
**
** @param [r] thys [AjPSeqQuery] Sequence query object.
** @return [void]
** @@
******************************************************************************/

void ajSeqQueryTrace (AjPSeqQuery thys)
{
    ajDebug ( "  Query Trace\n");
    if (ajStrLen(thys->DbName))
	ajDebug ( "    DbName: '%S'\n", thys->DbName);
    if (ajStrLen(thys->DbType))
	ajDebug ( "    DbType: '%S' (%d)\n", thys->DbType, thys->Type);
    ajDebug ( "   QryDone: %B\n", thys->QryDone);
    if (ajStrLen(thys->Id))
	ajDebug ( "    Id: '%S'\n", thys->Id);
    if (ajStrLen(thys->Acc))
	ajDebug ( "    Acc: '%S'\n", thys->Acc);
    if (ajStrLen(thys->Sv))
	ajDebug ( "    Sv: '%S'\n", thys->Sv);
    if (ajStrLen(thys->Des))
	ajDebug ( "    Des: '%S'\n", thys->Des);
    if (ajStrLen(thys->Key))
	ajDebug ( "    Key: '%S'\n", thys->Key);
    if (ajStrLen(thys->Org))
	ajDebug ( "    Org: '%S'\n", thys->Org);
    if (ajStrLen(thys->Method))
	ajDebug ( "    Method: '%S'\n", thys->Method);
    if (ajStrLen(thys->Formatstr))
	ajDebug ( "    Formatstr: '%S'\n", thys->Formatstr);
    if (ajStrLen(thys->IndexDir))
	ajDebug ( "    IndexDir: '%S'\n", thys->IndexDir);
    if (ajStrLen(thys->Directory))
	ajDebug ( "    Directory: '%S'\n", thys->Directory);
    if (ajStrLen(thys->Filename))
	ajDebug ( "    Filename: '%S'\n", thys->Filename);
    if (ajStrLen(thys->Exclude))
	ajDebug ( "    Exclude: '%S'\n", thys->Exclude);
    if (ajStrLen(thys->Application))
	ajDebug ( "    Application: '%S'\n", thys->Application);
    if (thys->Access)
	ajDebug ( "    Access: exists\n");
    if (thys->QryData)
	ajDebug ( "    QryData: exists\n");

    return;
}

/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */

/* @section Sequence Casts ****************************************************
**
** These functions examine the contents of a sequence and return some
** derived information. Some of them provide access to the internal
** components of a sequence. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/

/* @func ajSeqBegin ***********************************************************
**
** Returns the sequence start position, or 1 if no start has been set.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqBegin (AjPSeq seq)
{
    if (!seq->Begin)
	return 1;

    return ajSeqPos(seq, seq->Begin);
}

/* @func ajSeqEnd *************************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [ajint] End position.
** @@
******************************************************************************/

ajint ajSeqEnd (AjPSeq seq)
{
    if (!seq->End)
	return (ajSeqLen(seq));

    return ajSeqPosI(seq, ajSeqBegin(seq), seq->End);
}

/* @func ajSeqName ************************************************************
**
** Returns the sequence name as a C character string. This is a pointer to the
** real internal string. The caller must take care not to change the
** sequence in any way because this could corrupt the internal data
** structures.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [char*] Sequence name as a null terminated character string.
** @@
******************************************************************************/

char* ajSeqName (AjPSeq seq)
{
    return ajStrStr(seq->Name);
}

/* @func ajSeqOffset **********************************************************
**
** Returns the sequence offset from -sbegin originally.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [ajint] Sequence offset.
** @@
******************************************************************************/

ajint ajSeqOffset (AjPSeq seq)
{
    return seq->Offset;
}

/* @func ajSeqOffend **********************************************************
**
** Returns the sequence offend value.
** This is the number of positions removed from the original end.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [ajint] Sequence offend.
** @@
******************************************************************************/

ajint ajSeqOffend (AjPSeq seq)
{
    return seq->Offend;
}

/* @func ajSeqLen *************************************************************
**
** Returns the sequence length.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [ajint] Sequence length.
** @@
******************************************************************************/

ajint ajSeqLen (AjPSeq seq)
{
    return ajStrLen(seq->Seq);
}

/* @func ajSeqGetReverse ******************************************************
**
** Returns the sequence direction.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
** @@
******************************************************************************/

AjBool ajSeqGetReverse (AjPSeq seq)
{
    return seq->Rev;
}

/* @func ajSeqCharCopy ********************************************************
**
** Returns a sequence as a C character string. This is a copy of the string
** so the caller can do anything with it.
** It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [char*] Sequence as a null terminated character string.
** @@
******************************************************************************/

char* ajSeqCharCopy (AjPSeq seq)
{
    return ajCharNew(seq->Seq);
}

/* @func ajSeqCharCopyL *******************************************************
**
** Returns a sequence as a C character string. This is a copy of the string
** so the caller can do anything with it.
** It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [P] seq [AjPSeq] Sequence object
** @param [r] size [size_t] Maximum length as returned by strlen
** @return [char*] Sequence as a null terminated character string.
** @@
******************************************************************************/

char* ajSeqCharCopyL (AjPSeq seq, size_t size)
{
    return ajCharNewLS(size, seq->Seq);
}

/* @func ajSeqNum *************************************************************
**
** Converts a sequence to numbers using a conversion table.
**
** @param [r] thys [AjPSeq] Sequence.
** @param [r] cvt [AjPSeqCvt] Conversion table.
** @param [w] numseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqNum (AjPSeq thys, AjPSeqCvt cvt, AjPStr* numseq)
{
    char *cp = ajSeqChar(thys);
    char *ncp;

    (void) ajStrAssS (numseq, thys->Seq);
    ncp = ajStrStr(*numseq);

    while (*cp)
    {
	*ncp = cvt->table[(ajint)*cp];
	cp++;
	ncp++;
    }

    return ajTrue;
}

/* @func ajSeqCvtTrace ********************************************************
**
** Traces a conversion table with debug calls.
**
** @param [r] cvt [AjPSeqCvt] Conversion table.
** @return [void]
** @@
******************************************************************************/

void ajSeqCvtTrace (AjPSeqCvt cvt)
{
    ajint i;
    ajDebug ("Cvt table for '%S'\n\n", cvt->bases);
    ajDebug ("index num ch\n");
    ajDebug ("----- --- --\n");
    for (i=0; i < cvt->size; i++)
	if (cvt->table[i])
	    ajDebug ("%5d %3d <%c>\n", i, cvt->table[i], ajSysItoC(i));

    ajDebug ("... all others are zero ...\n", cvt->bases);

    return;
}

/* @func ajSeqCvtNewZero ******************************************************
**
** Generates a new conversion table in which the first character in the
** string provided is converted to 1, the second to 2, and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
**
** @param [r] bases [char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNewZero (char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    char *cp = bases;

    AJNEW0(ret);
    ret->len = strlen(bases);
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNewC (bases);
    ret->missing = 0;

    i = 0;
    while (*cp)
    {
	i++;
	ret->table[toupper((ajint) *cp)] = ajSysItoC(i);
	ret->table[tolower((ajint) *cp)] = ajSysItoC(i);
	cp++;
    }

    return ret;
}

/* @func ajSeqCvtNew **********************************************************
**
** Generates a new conversion table in which the first character in the
** string provided is converted to 0, the second to 1, and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are converted to one more than the highest char.
**
** @param [r] bases [char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNew (char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    ajint j;
    ajint imax;
    char *cp = bases;

    imax = strlen(bases);

    AJNEW0(ret);
    ret->len = imax;
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNewC (bases);
    ret->missing = imax;

    for (j=0; j < ret->size; j++)
	ret->table[j] = ajSysItoC(imax);

    i = 0;
    while (*cp)
    {
	ret->table[toupper((ajint) *cp)] = ajSysItoC(i);
	ret->table[tolower((ajint) *cp)] = ajSysItoC(i);
	cp++;
	i++;
    }

    return ret;
}

/* @func ajSeqCvtDel **********************************************************
**
** Delete a conversion table
**
** @param [w] thys [AjPSeqCvt*] Conversion table reference
** @return [void]
** @@
******************************************************************************/

void ajSeqCvtDel (AjPSeqCvt* thys)
{

    if(!*thys || !thys)
	return;

    AJFREE((*thys)->table);
    ajStrDel(&(*thys)->bases);
    AJFREE(*thys);

    return;
}

/* @func ajSeqCvtNewText ******************************************************
**
** Generates a new conversion table in which the characters are retained
** as upper case, numbers are set to -1 and all other characters
** are set to -2.
**
** @param [r] bases [char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNewText (char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    ajint j;
    char *cp = bases;
    char c;

    AJNEW0(ret);
    ret->len = strlen(bases);
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNewC (bases);
    ret->missing = -1;

    for (j=0; j < ret->size; j++)
	if (isdigit(j))
	    ret->table[j] = -1;
	else
	    ret->table[j] = -2;


    i = 0;
    while (*cp)
    {
	c = ajSysItoC(toupper((ajint)*cp));
	ret->table[toupper((ajint) *cp)] = c;
	ret->table[tolower((ajint) *cp)] = c;
	cp++;
	i++;
    }

    return ret;
}

/* @func ajSeqCvtLen **********************************************************
**
** Returns the length of a conversion table string (number of sequence
** characterers explicitly included)
**
** @param [r] thys [AjPSeqCvt] Conversion table
**
** @return [ajint] Length
** @@
******************************************************************************/

ajint ajSeqCvtLen (AjPSeqCvt thys)
{
    return thys->len;
}

/* @func ajSeqCvtK ************************************************************
**
** Returns the integer code corresponding to a sequence character
** in a conversion tabkle
**
** @param [r] thys [AjPSeqCvt] Conversion table
** @param [r] ch [char] Sequence character
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtK (AjPSeqCvt thys, char ch)
{
    return thys->table[(ajint)ch];
}

/* @func ajSeqStr *************************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [AjPSeq] Sequence.
** @return [AjPStr] Sequence as a string.
** @@
******************************************************************************/

AjPStr ajSeqStr (AjPSeq thys)
{
    if (!thys)
	return NULL;

    return thys->Seq;
}

/* @func ajSeqStrCopy *********************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a copy of the internal string
** the caller may change the string. It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] thys [AjPSeq] Sequence.
** @return [AjPStr] Sequence as a string.
** @@
******************************************************************************/

AjPStr ajSeqStrCopy (AjPSeq thys)
{
    static AjPStr str;

    str= 0;
    (void) ajStrAssS (&str, thys->Seq);

    return str;
}

/* @func ajSeqChar ************************************************************
**
** Returns the sequence in a sequence object as a null terminated
** character string. Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [AjPSeq] Sequence.
** @return [char*] Sequence as a null terminated character string.
** @@
******************************************************************************/

char* ajSeqChar (AjPSeq thys)
{
    if (!thys)
	return "";

    return ajStrStr(thys->Seq);
}

/* @func ajSeqGetAcc **********************************************************
**
** Returns the sequence accession number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] Accession number as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetAcc (AjPSeq thys)
{
    return thys->Acc;
}

/* @func ajSeqGetSv ***********************************************************
**
** Returns the sequence version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] SeqVersion number as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetSv (AjPSeq thys)
{
    return thys->Sv;
}

/* @func ajSeqGetGi ***********************************************************
**
** Returns the GI version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] SeqVersion number as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetGi (AjPSeq thys)
{
    return thys->Gi;
}

/* @func ajSeqGetDesc *********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] Description as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetDesc (AjPSeq thys)
{
    return thys->Desc;
}

/* @func ajSeqGetFeat *********************************************************
**
** Returns the sequence feature table.
** Because this is a pointer to the real internal table
** the caller must take care not to change it in any way,
** or to delete it.
**
** If the table is to be changed or deleted then it must first
** be copied with ajSeqCopyFeat
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPFeattable] feature table (if any)
** @@
******************************************************************************/

AjPFeattable ajSeqGetFeat (AjPSeq thys)
{
    return thys->Fttable;
}

/* @func ajSeqCopyFeat ********************************************************
**
** Returns a copy of the sequence feature table.
** Because this is a copy of all the data, the caller is responsible
** for deleting it after use.
**
** If the table is not to be changed or deleted then ajSeqGetFeat
** can return a copy of the internal pointer.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPFeattable] feature table (if any)
** @@
******************************************************************************/

AjPFeattable ajSeqCopyFeat (AjPSeq thys)
{
  AjPFeattable ret = NULL;
  ajFeattableCopy (&ret, thys->Fttable);
  return ret;
}

/* @func ajSeqGetName *********************************************************
**
** Returns the sequence name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] Name as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetName (AjPSeq thys)
{
    return thys->Name;
}

/* @func ajSeqGetEntry ********************************************************
**
** Returns the sequence full text entry.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPStr] Entry as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetEntry (AjPSeq thys)
{
    return thys->TextPtr;
}


/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Output Constructors **************************************
**
** All constructors return a new sequence output object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequenceoutput object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
******************************************************************************/

/* @func ajSeqoutNew **********************************************************
**
** Creates a new sequence output object.
**
** @return [AjPSeqout] New sequence output object.
** @@
******************************************************************************/

AjPSeqout ajSeqoutNew (void)
{
    AjPSeqout pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    /* pthis->Acc = ajStrNew(); */
    pthis->Sv = ajStrNew();
    pthis->Gi = ajStrNew();
    pthis->Tax = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->EType = 0;
    pthis->Db = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Date = ajStrNew();
    pthis->Doc = ajStrNew();
    pthis->Rev = ajFalse;
    pthis->Usa = ajStrNew();
    pthis->Ufo = ajStrNew();
    pthis->Informatstr = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Format = 0;
    pthis->Filename = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->Seq = ajStrNew();
    pthis->File = NULL;
    pthis->Count = 0;
    pthis->Single = ajFalse;
    pthis->Features = ajFalse;
    pthis->Extension = ajStrNew();
    pthis->Savelist = NULL;

    pthis->Ftquery = ajFeattabOutNew();
    pthis->Fttable = NULL;

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();

    return pthis;
}

/* @func ajSeqoutNewF *********************************************************
**
** Creates a new sequence output object using a preopened file.
**
** @param [R] file [AjPFile] Open file object
** @return [AjPSeqout] New sequence output object.
** @@
******************************************************************************/

AjPSeqout ajSeqoutNewF (AjPFile file)
{
  AjPSeqout pthis;
  pthis = ajSeqoutNew();
  pthis->Knownfile = file;

  return pthis;
}

/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Output Destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence output object.
**
******************************************************************************/

void ajSeqoutDel (AjPSeqout* pthis)
{
    AjPSeqout thys = *pthis;
    AjPSeq    seq=NULL;
    AjPStr    tmpstr=NULL;

    ajStrDel (&thys->Name);
    /* ajStrDel (&thys->Acc); */
    ajStrDel (&thys->Sv);
    ajStrDel (&thys->Gi);
    ajStrDel (&thys->Tax);
    ajStrDel (&thys->Desc);
    ajStrDel (&thys->Type);
    ajStrDel (&thys->Db);
    ajStrDel (&thys->Full);
    ajStrDel (&thys->Date);
    ajStrDel (&thys->Doc);
    ajStrDel (&thys->Usa);
    ajStrDel (&thys->Ufo);
    ajStrDel (&thys->Informatstr);
    ajStrDel (&thys->Formatstr);
    ajStrDel (&thys->Filename);
    ajStrDel (&thys->Entryname);
    ajStrDel (&thys->Seq);
    ajStrDel (&thys->Extension);

    while(ajListPop(thys->Acclist,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListDel(&thys->Acclist);

    while(ajListPop(thys->Keylist,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListDel(&thys->Keylist);

    while(ajListPop(thys->Taxlist,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListDel(&thys->Taxlist);

    while(ajListPop(thys->Savelist,(void **)&seq))
	ajSeqDel(&seq);
    ajListDel(&thys->Savelist);

    AJFREE(thys->Ftquery);

    AJFREE(*pthis);
    return;
}

/* @func ajSeqMW **************************************************************
**
** Calculates the molecular weight of a protein sequence.
**
** @param [P] seq [AjPStr] Sequence as a string.
** @return [float] Molecular weight.
** @@
******************************************************************************/

float ajSeqMW (AjPStr seq)
{
    /* source: Biochemistry LABFAX */
    static float aa[26] = { 089.10, 132.61, 121.16, 133.11, /* A-D */
				147.13, 165.19, 075.07, 155.16, /* E-H */
				131.18, 000.00, 146.19, 131.18, /* I-L */
				149.21, 132.12, 000.00, 115.13, /* M-P */
				146.15, 174.20, 105.09, 119.12, /* Q-T */
				000.00, 117.15, 204.23, 128.16, /* U-X */
				181.19, 146.64};
    float mw = 18.015;
    ajint i;
    char* cp = ajStrStr(seq);

    while (*cp)
    {
	i = toupper((ajint) *cp)-'A';
	if (i > 25 || i < 0)
	{
	    ajDebug("seqMW bad character '%c' %d\n", *cp, *cp);
	    i = 'X' - 'A';
	}
	mw += aa[i] - (float) 18.015;
	cp++;
    }

    ajDebug("seqMW calculated %.2f\n", mw);
    return mw;
}

/* @func ajSeqCrc *************************************************************
**
** Calculates the SwissProt style CRC32 checksum for a protein sequence.
** This seems to be a bit reversal of a standard CRC32 checksum.
**
** @param [P] seq [AjPStr] Sequence as a string
** @return [ajuint] CRC32 checksum.
** @@
******************************************************************************/

ajuint ajSeqCrc (AjPStr seq)
{
    register ajulong crc;
    ajint     c;
    char* cp;
    static ajint calls = 0;

    if (!calls)
    {
	seqCrcGen ();
	calls = 1;
    }

    cp = ajStrStr(seq);

    crc = 0xFFFFFFFFL;
    while( *cp )
    {
	c = toupper((ajint) *cp);
	crc = ((crc >> 8) & 0x00FFFFFFL) ^ seqCrcTable[ (crc^c) & 0xFF ];
	cp++;
    }
    ajDebug("CRC32 calculated %08lX\n", crc);
    return( crc );
}

/* @funcstatic seqCrcGen ******************************************************
**
** Generates data for a CRC32 calculation in a static data structure.
**
** @return [void]
** @@
******************************************************************************/

static void seqCrcGen (void)
{
    ajulong crc;
    ajulong poly;
    ajint   i;
    ajint   j;

    poly = 0xEDB88320L;
    for (i=0; i<256; i++)
    {
	crc = i;
	for (j=8; j>0; j--)
	    if (crc&1)
		crc = (crc >> 1) ^ poly;
	    else
		crc >>= 1;

	seqCrcTable[i] = crc;
    }

    return;
}

/* @func ajSeqCount ***********************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [P] thys [AjPStr] Sequence as a string
** @param [w] b [ajint*] integer array, minimum size 5, to hold the results.
** @return [void]
** @@
******************************************************************************/

void ajSeqCount (AjPStr thys, ajint* b)
{
    char* cp = ajStrStr(thys);

    b[0] = b[1] = b[2] = b[3] = b[4] = 0;

    ajDebug ("ajSeqCount %d bases\n", ajStrLen(thys));
    while (*cp)
    {
	if (toupper((ajint) *cp) == 'A') b[0]++;
	if (toupper((ajint) *cp) == 'C') b[1]++;
	if (toupper((ajint) *cp) == 'G') b[2]++;
	if (toupper((ajint) *cp) == 'T') b[3]++;
	if (toupper((ajint) *cp) == 'U') b[3]++;
	cp++;
    }

    b[4] = ajStrLen(thys) - b[0] - b[1] - b[2] - b[3];

    return;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Output Operators *****************************************
**
** These functions use the contents of a sequence output object but do
** not make any changes.
**
******************************************************************************/

/* @func ajSeqCheckGcg ********************************************************
**
** Calculates a GCG checksum for a sequence.
**
** @param [r] thys [AjPSeq] Squence.
** @return [ajint] GCG checksum.
** @@
******************************************************************************/

ajint ajSeqCheckGcg (AjPSeq thys)
{
    register ajlong  i;
    register ajlong  check = 0;
    register ajlong  count = 0;
    char *cp = ajStrStr(thys->Seq);
    ajint ilen = ajStrLen(thys->Seq);

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

/* @func ajSeqPos *************************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [wP] thys [AjPSeq] Target sequence.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqPos (AjPSeq thys, ajint ipos)
{
    return ajSeqPosII (ajSeqLen(thys), 1, ipos);
}

/* @func ajSeqPosI ************************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [wP] thys [AjPSeq] Target sequence.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqPosI (AjPSeq thys, ajint imin, ajint ipos)
{
    return ajSeqPosII (ajSeqLen(thys), imin, ipos);
}

/* @func ajSeqPosII ***********************************************************
**
** Converts a position into a true position. If ipos is negative,
** it is counted from the end of the sequence rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [r] ilen [ajint] maximum length.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqPosII (ajint ilen, ajint imin, ajint ipos)
{
    ajint jpos;

    if (ipos < 0)
	jpos = ilen + ipos + 1;
    else
    {
	if (ipos)
	    jpos = ipos;
	else
	    jpos = 1;
    }

    if (jpos > ilen)
	jpos = ilen;

    if (jpos < imin)
	jpos = imin;

    ajDebug("ajSeqPosII ilen: %d imin: %d ipos: %d) = %d\n",
	    ilen, imin, ipos, jpos);

    return jpos;
}

/* @func ajSeqTrim ************************************************************
**
** Trim a sequence using the Begin and Ends.
**
** @param [rw] thys [AjPSeq] Sequence to be trimmed.
** @return [AjBool] AjTrue returned if successful.
** @@
******************************************************************************/

AjBool ajSeqTrim(AjPSeq thys)
{
    AjBool okay=ajTrue;
    ajint begin;
    ajint end;

    ajDebug("Trimming %d from %d to %d\n",
	    thys->Seq->Len,thys->Begin,thys->End);

    begin = ajSeqPos(thys, thys->Begin);
    end = ajSeqPos(thys, thys->End);

    ajDebug("Trimming %d from %d (%d) to %d (%d)\n",
	    thys->Seq->Len,thys->Begin,begin, thys->End, end);
    if (thys->End)
    {
	if(end < begin)
	    return ajFalse;
	okay = ajStrTrim(&(thys->Seq),(0 - (thys->Seq->Len-(end)) ));
	thys->Offend = thys->Seq->Len-(end);
	thys->End = 0;
    }
    if(thys->Begin)
    {
	okay = ajStrTrim(&thys->Seq,begin-1);
	thys->Offset = begin-1;
	thys->Begin =0;
    }
    ajDebug("After Trimming len = %d\n",thys->Seq->Len);
    /*ajDebug("After Trimming len = %d '%S'\n",thys->Seq->Len, thys->Seq);*/

    return okay;
}

/* @func ajSeqGapCount ********************************************************
**
** returns the number of gaps in a sequence (counting any possible
** gap character
**
** @param [w] thys [AjPSeq] Sequence object
** @return [ajint] Number of gaps
******************************************************************************/

ajint ajSeqGapCount (AjPSeq thys) {
  return ajSeqGapCountS (thys->Seq);
}

/* @func ajSeqGapCountS *******************************************************
**
** returns the number of gaps in a string (counting any possible
** gap character
**
** @param [w] str [AjPStr] String object
** @return [ajint] Number of gaps
******************************************************************************/

ajint ajSeqGapCountS (AjPStr str) {

  ajint ret=0;

  static char testchars[] = "-~."; /* all known gap characters */
  char *testgap = testchars;

  ajDebug("ajSeqGapCountS '%S'\n", str);

  while (*testgap) {
    ret += ajStrCountK(str, *testgap);
    testgap++;
  }

  return ret;
}

/* @func ajSeqGapStandard *****************************************************
**
** Makes all gaps in a sequence use a standard gap character
**
** @param [w] thys [AjPSeq] Sequence object
** @param [r] gapch [char] Gap character (or '-' if zero)
** @return [void]
******************************************************************************/

void ajSeqGapStandard (AjPSeq thys, char gapch) {

  char newgap = '-';
  static char testchars[] = "-~."; /* all known gap characters */
  char *testgap = testchars;

  if (gapch)
    newgap = gapch;

  ajDebug("ajSeqGapStandard '%c'=>'%c' '%S'\n", gapch, newgap, thys->Seq);

  while (*testgap) {
    if (newgap != *testgap) {
      ajStrSubstituteKK (&thys->Seq, *testgap, newgap);
      ajDebug(" replaced         '%c'=>'%c' '%S'\n",
	      *testgap, newgap, thys->Seq);
    }
    testgap++;
  }

  return;
}

/* @func ajSeqFill ************************************************************
**
** Fills a single sequence with gaps up to a specified length.
**
** @param [P] seq [AjPSeq] Sequence object to be set.
** @param [P] len [ajint] Length to pad fill to.
** @return [ajint] Number of gaps inserted
** @@
******************************************************************************/

ajint ajSeqFill (AjPSeq seq, ajint len)
{
    ajint ilen=0;

    ajDebug ("ajSeqFill (len: %d -> ilen:%d)\n", ajSeqLen(seq), ilen);

    if (ajSeqLen(seq) < len)
    {
      ilen = len - ajSeqLen(seq);
      ajStrFill (&seq->Seq, len, '-');
    }

    ajDebug ("      result: (len: %d added: %d\n",
	     ajSeqLen(seq), ilen);

    return ilen;
}


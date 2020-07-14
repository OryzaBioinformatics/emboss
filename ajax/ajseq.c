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
#include <float.h>

static ajulong seqCrcTable[256];




static void       seqCrcGen( void );
static AjPSelexdata seqSelexClone(const AjPSelexdata thys);




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
** @category new [AjPSeqall] Default constructor
** @@
******************************************************************************/

AjPSeqall ajSeqallNew(void)
{
    AjPSeqall pthis;

    AJNEW0(pthis);

    pthis->Seq   = ajSeqNew();
    pthis->Seqin = ajSeqinNew();
    pthis->Count = 0;

    return pthis;
}




/* @funcstatic seqSelexClone **************************************************
**
** Clone a Selexdata object
**
** @param [r] thys [const AjPSelexdata] selex data object
**
** @return [AjPSelexdata] New selex data object.
** @@
******************************************************************************/

static AjPSelexdata seqSelexClone(const AjPSelexdata thys)
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
** @category delete [AjPSeqall] Default destructor
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
** @category delete [AjPSeqset] Default destructor
** @@
******************************************************************************/

void ajSeqsetDel(AjPSeqset *thys)
{
    ajint n;
    ajint i;
    AjPSeqset pthis = NULL;

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
** @param [u] seqall [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallToLower(AjPSeqall seqall)
{
    ajSeqToLower(seqall->Seq);

    return;
}




/* @func ajSeqallToUpper ******************************************************
**
** Converts the latest sequence in a stream to upper case.
**
** @param [u] seqall [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallToUpper(AjPSeqall seqall)
{
    ajSeqToUpper(seqall->Seq);

    return;
}




/* @func ajSeqallReverse ******************************************************
**
** Reverse complements the latest sequence in a stream.
**
** @param [u] thys [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallReverse(AjPSeqall thys)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqallReverse len: %d Begin: %d End: %d\n",
	    ajSeqallLen(thys), thys->Begin, thys->End);

    ibegin = thys->Begin;
    iend   = thys->End;

    thys->End   = -(ibegin);
    thys->Begin = -(iend);

    ajSeqReverse(thys->Seq);

    ajDebug("  all result len: %d Begin: %d End: %d\n",
	    ajSeqallLen(thys), thys->Begin, thys->End);

    return;
}




/* @func ajSeqallSetRange *****************************************************
**
** Sets the start and end positions for a sequence stream.
**
** @param [u] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqallSetRange(AjPSeqall seq, ajint ibegin, ajint iend)
{
    ajDebug("ajSeqallSetRange (len: %d %d, %d)\n",
	    ajSeqLen(seq->Seq), ibegin, iend);

    if(ibegin)
	seq->Begin = seq->Seq->Begin = ibegin;

    if(iend)
	seq->End = seq->Seq->End = iend;

    ajDebug("      result: (len: %d %d, %d)\n",
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
** @param [r] seqall [const AjPSeqall] Sequence stream object
** @return [ajint] sequence length.
** @@
******************************************************************************/

ajint ajSeqallLen(const AjPSeqall seqall)
{
    return ajSeqLen(seqall->Seq);
}




/* @func ajSeqallBegin ********************************************************
**
** Returns the sequence stream start position, or 1 if no start has been set.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqallBegin(const AjPSeqall seq)
{
    if (seq->Begin)
	return ajSeqPos(seq->Seq, seq->Begin);

    if(seq->Seq->Begin)
	return ajSeqPos(seq->Seq, seq->Seq->Begin);

    return 1;
}




/* @func ajSeqallEnd **********************************************************
**
** Returns the sequence stream end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeqall] Sequence stream object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqallEnd(const AjPSeqall seq)
{
    if (seq->End)
	return ajSeqPosI(seq->Seq, ajSeqallBegin(seq), seq->End);

    if(seq->Seq->End)
	return ajSeqPosI(seq->Seq, ajSeqallBegin(seq), seq->Seq->End);

    return ajSeqLen(seq->Seq);
}




/* @func ajSeqallGetRange *****************************************************
**
** Returns the sequence range for a sequence stream
**
** @param [r] thys [const AjPSeqall] Sequence stream object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqallGetRange(const AjPSeqall thys, ajint* begin, ajint* end)
{
    ajDebug("ajSeqallGetRange '%S'\n", thys->Seq->Name);

    return ajSeqGetRange(thys->Seq, begin, end);
}




/* @func ajSeqsetGetFormat ****************************************************
**
** Returns the sequence format for a sequence set
**
** @param [r] thys [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Sequence format
** @@
******************************************************************************/

const AjPStr ajSeqsetGetFormat(const AjPSeqset thys)
{
    return thys->Formatstr;
}




/* @func ajSeqsetGetRange *****************************************************
**
** Returns the sequence range for a sequence set
**
** @param [r] thys [const AjPSeqset] Sequence set object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqsetGetRange(const AjPSeqset thys, ajint* begin, ajint* end)
{
    ajDebug("ajSeqsetGetRange '%S' begin %d end %d len: %d\n",
	    thys->Name, thys->Begin, thys->End, thys->Len);
    *begin = ajSeqPosII(thys->Len, 1, thys->Begin);

    if(thys->End)
	*end = ajSeqPosII(thys->Len, *begin, thys->End);
    else
	*end = ajSeqPosII(thys->Len, *begin, thys->Len);

    return (*end - *begin + 1);
}




/* @func ajSeqGetRange ********************************************************
**
** Returns the sequence range for a sequence.
**
** @param [r] thys [const AjPSeq] Sequence object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqGetRange(const AjPSeq thys, ajint* begin, ajint* end)
{
    ajDebug("ajSeqGetRange '%S'\n", thys->Name);
    *begin = ajSeqPos(thys, thys->Begin);

    if(thys->End)
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
** @param [r] thys [const AjPSeqall] Sequence stream object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetName(const AjPSeqall thys)
{
    ajDebug("ajSeqallGetName '%S'\n", thys->Seqin->Name);

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
** @param [r] thys [const AjPSeqall] Sequence stream object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetNameSeq(const AjPSeqall thys)
{
    ajDebug("ajSeqallGetNameSeq '%S'\n", thys->Seq->Name);

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
** @param [r] thys [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqsetGetUsa(const AjPSeqset thys)
{
    ajDebug("ajSeqetGetUsa '%S'\n", thys->Usa);

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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetUsa(const AjPSeq thys)
{
    static AjPStr usa = NULL;
    ajDebug("ajSeqGetUsa '%S'\n", thys->Usa);

    if(ajStrLen(thys->Usa))
	return thys->Usa;

    ajSeqMakeUsaS(thys, NULL, &usa);
    return usa;
}




/* @func ajSeqallGetUsa *******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPSeqall] Sequence object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetUsa(const AjPSeqall thys)
{
    ajDebug("ajSeqallGetUsa '%S'\n", thys->Seqin->Usa);

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
** @category new [AjPSeqset] Default constructor
** @@
******************************************************************************/

AjPSeqset ajSeqsetNew(void)
{
    AjPSeqset pthis;

    AJNEW0(pthis);

    pthis->Size      = 0;
    pthis->Len       = 0;
    pthis->Begin     = 0;
    pthis->End       = 0;
    pthis->Totweight = 0.0;

    pthis->Name = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();

    pthis->Seq       = NULL;
    pthis->Seqweight = NULL;

    pthis->EType  = 0;
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
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [void]
** @category modify [AjPSeqset] Converts a sequence set to lower
**                case
** @@
******************************************************************************/

void ajSeqsetToLower(AjPSeqset seqset)
{
    ajint i;

    for(i=0; i < seqset->Size; i++)
	ajSeqToLower(seqset->Seq[i]);

    return;
}




/* @func ajSeqsetToUpper ******************************************************
**
** Converts all sequences in a set to upper case.
**
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [void]
** @category modify [AjPSeqset] Converts a sequence set to upper
**                case
** @@
******************************************************************************/

void ajSeqsetToUpper(AjPSeqset seqset)
{
    ajint i;

    for(i=0; i < seqset->Size; i++)
	ajSeqToUpper(seqset->Seq[i]);

    return;
}




/* @func ajSeqsetReverse ******************************************************
**
** Reverse complements all sequences in a sequence set.
**
** @param [u] thys [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetReverse(AjPSeqset thys)
{
    ajint i;
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqsetReverse len: %d Begin: %d End: %d\n",
	    ajSeqsetLen(thys), thys->Begin, thys->End);

    ibegin = thys->Begin;
    iend   = thys->End;

    if(ibegin)
	thys->End = -(ibegin);
    if(iend)
	thys->Begin = -(iend);

    for(i=0; i < thys->Size; i++)
	ajSeqReverse(thys->Seq[i]);

    ajDebug("  set result len: %d Begin: %d End: %d\n",
	    ajSeqsetLen(thys), thys->Begin, thys->End);

    return;
}




/* @func ajSeqsetSetRange *****************************************************
**
** Sets the start and end positions for a sequence set.
**
** @param [u] seq [AjPSeqset] Sequence set object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetRange(AjPSeqset seq, ajint ibegin, ajint iend)
{
    ajint i;

    ajDebug("ajSeqsetSetRange(len: %d %d, %d)\n", seq->Len, ibegin, iend);

    if(ibegin)
	seq->Begin = ibegin;

    if(iend)
	seq->End = iend;

    for(i=0; i< seq->Size; i++)
    {
	if(ibegin)
	    seq->Seq[i]->Begin = ibegin;
	if(iend)
	    seq->Seq[i]->End   = iend;
    }

    ajDebug("      result: (len: %d %d, %d)\n",
	    seq->Len, seq->Begin, seq->End);

    return;
}




/* @func ajSeqsetFill *********************************************************
**
** Fills a sequence set with gaps at the ends of any shorter sequences.
**
** @param [u] seq [AjPSeqset] Sequence set object to be set.
** @return [ajint] Number of gaps inserted
** @@
******************************************************************************/

ajint ajSeqsetFill(AjPSeqset seq)
{
    ajint i;
    ajint ifix = 0;
    ajint nfix = 0;
    ajint ilen;

    ajDebug("ajSeqsetFill(len: %d)\n", seq->Len);

    for(i=0; i< seq->Size; i++)
	if(ajSeqLen(seq->Seq[i]) < seq->Len)
	{
	    nfix++;
	    ilen = seq->Len - ajSeqLen(seq->Seq[i]);
	    if(ilen > ifix)
		ifix = ilen;
	    ajStrFill(&seq->Seq[i]->Seq, seq->Len, '-');
	}

    ajDebug("      result: (len: %d added: %d number of seqs fixed: nfix\n",
	    seq->Len, ifix, nfix);

    return ifix;
}




/* @func ajSeqsetTrim ******************************************************
**
** Trims a sequence set to start and end positions
**
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [void]
** @category modify [AjPSeqset] Converts a sequence set to lower
**                case
** @@
******************************************************************************/

void ajSeqsetTrim(AjPSeqset seqset)
{
    ajint i;

    for(i=0; i < seqset->Size; i++)
	ajSeqTrim(seqset->Seq[i]);

    return;
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
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] sequence set length.
** @category cast [AjPSeqset] Returns the maximum length of a
**                sequence set
** @@
******************************************************************************/

ajint ajSeqsetLen(const AjPSeqset seq)
{
    return seq->Len;
}




/* @func ajSeqsetBegin ********************************************************
**
** Returns the sequence set start position, or 1 if no start has been set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqsetBegin(const AjPSeqset seq)
{
    if(!seq->Begin)
	return 1;

    return ajSeqPosII(seq->Len, 1, seq->Begin);
}




/* @func ajSeqsetEnd **********************************************************
**
** Returns the sequence set end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqsetEnd(const AjPSeqset seq)
{
    if(!seq->End)
	return (seq->Len);

    return ajSeqPosII(seq->Len, ajSeqsetBegin(seq), seq->End);
}




/* @func ajSeqsetSeq **********************************************************
**
** Returns the sequence data of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [const char*] Sequence as a NULL terminated string.
** @category cast [AjPSeqset] Returns the char* pointer to a
**                sequence in a set
** @@
******************************************************************************/

const char* ajSeqsetSeq(const AjPSeqset seq, ajint i)
{
    if(i >= seq->Size)
	return NULL;

    return ajStrStr(seq->Seq[i]->Seq);
}




/* @func ajSeqsetSize *********************************************************
**
** Returns the number of sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] sequence set size.
** @category cast [AjPSeqset] Returns the number of sequences in a
**                sequence set
** @@
******************************************************************************/

ajint ajSeqsetSize(const AjPSeqset seq)
{
    return seq->Size;
}




/* @func ajSeqsetName *********************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [const AjPStr] sequence name as a string.
** @category cast [AjPSeqset] Returns the name of a sequence in a
**                set
** @@
******************************************************************************/

const AjPStr ajSeqsetName(const AjPSeqset seq, ajint i)
{
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Name;
}




/* @func ajSeqsetWeight *******************************************************
**
** Returns the weight of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetWeight(const AjPSeqset seq, ajint i)
{
    if(i >= seq->Size)
	return 0.0;

    return seq->Seq[i]->Weight;
}




/* @func ajSeqsetTotweight ****************************************************
**
** Returns the weight of all sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetTotweight(const AjPSeqset seq)
{
    ajint i;
    float ret = 0.0;

    for(i=0; i < seq->Size; i++)
    {
	ret += seq->Seq[i]->Weight;
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
** @param [r] thys [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqsetGetName(const AjPSeqset thys)
{
    ajDebug("ajSeqsetGetName '%S' usa: '%S'\n", thys->Name, thys->Usa);

    if(ajStrLen(thys->Name))
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
** @param [r] thys [const AjPSeqset] Sequence set object.
** @param [r] i [ajint] Sequence index number in set
** @return [const AjPSeq] Sequence object.
** @@
******************************************************************************/

const AjPSeq ajSeqsetGetSeq(const AjPSeqset thys, ajint i)
{
    ajDebug("ajSeqsetGetSeq '%S' %d/%d\n", thys->Name,i, thys->Size);
    if(i >= thys->Size)
	return NULL;

    return thys->Seq[i];
}




/* @func ajSeqsetGetSeqArray **************************************************
**
** Returns an array of sequences.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** The array is 1 larger than the sequence set,
** with the last element set to NULL.
** @param [r] thys [const AjPSeqset] Sequence set object.
** @return [AjPSeq*] Sequence object.
** @@
******************************************************************************/

AjPSeq* ajSeqsetGetSeqArray(const AjPSeqset thys)
{
    AjPSeq* ret;
    ajint i;

    ajDebug("ajSeqsetGetSeqArray '%S' %d\n", thys->Name, thys->Size);
    AJCNEW0(ret, (thys->Size+1));
    for (i=0; i<thys->Size;i++)
    {
	ret[i] = ajSeqNewS(thys->Seq[i]);
    }
    return ret;
}




/* @func ajSeqsetIsNuc ********************************************************
**
** Tests whether a sequence set is nucleotide.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
** @category cast [AjPSeqset] Tests whether the sequence set is
**                nucleotide
** @@
******************************************************************************/

AjBool ajSeqsetIsNuc(const AjPSeqset thys)
{
    AjPSeq seq;

    if(ajStrMatchC(thys->Type, "N"))
	return ajTrue;

    seq = thys->Seq[0];
    if(ajSeqTypeGapnucS(seq->Seq))
    	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetIsDna ********************************************************
**
** Tests whether a sequence set is DNA.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
** @category cast [AjPSeqset] Tests whether the sequence set is
**                nucleotide
** @@
******************************************************************************/

AjBool ajSeqsetIsDna(const AjPSeqset thys)
{
    AjPSeq seq;

    if(ajStrMatchC(thys->Type, "P"))
	return ajFalse;

    seq = thys->Seq[0];
    if(ajSeqTypeGapdnaS(seq->Seq))
    	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetIsRna ********************************************************
**
** Tests whether a sequence set is RNA.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
** @category cast [AjPSeqset] Tests whether the sequence set is
**                nucleotide
** @@
******************************************************************************/

AjBool ajSeqsetIsRna(const AjPSeqset thys)
{
    AjPSeq seq;

    if(ajStrMatchC(thys->Type, "P"))
	return ajFalse;

    seq = thys->Seq[0];
    if(ajSeqTypeGaprnaS(seq->Seq))
    	return ajFalse;

    return ajTrue;
}




/* @func ajSeqsetIsProt *******************************************************
**
** Tests whether a sequence set is protein.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a protein sequence set.
** @category cast [AjPSeqset] Tests whether the sequence set is
**                protein
** @@
******************************************************************************/

AjBool ajSeqsetIsProt(const AjPSeqset thys)
{
    AjPSeq seq;

    if(ajStrMatchC(thys->Type, "P"))
	return ajTrue;

    if(ajSeqsetIsNuc(thys))
	return ajFalse;

    seq = thys->Seq[0];
    if(ajSeqTypeAnyprotS(seq->Seq))
	return ajFalse;

    return ajTrue;
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
** @category new [AjPSeq] Default constructor
** @@
******************************************************************************/

AjPSeq ajSeqNew(void)
{
    return ajSeqNewL(0);
}




/* @func ajSeqNewL ************************************************************
**
** Creates and initialises a sequence object with a specified sequence length.
**
** @param [r] size [size_t] Reserved space for the sequence, including
**                          a trailing null character.
** @return [AjPSeq] New sequence object.
** @category new [AjPSeq] Constructor with expected maximum size.
** @@
******************************************************************************/

AjPSeq ajSeqNewL(size_t size)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Date = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();
    if(size)
	pthis->Seq = ajStrNewL(size);
    else
	pthis->Seq = ajStrNew();

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;

    pthis->EType   = 0;
    pthis->Format  = 0;
    pthis->Begin   = 0;
    pthis->End     = 0;
    pthis->Offset  = 0;
    pthis->Offend  = 0;
    pthis->Weight  = 1.0;
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
** @param [r] seq [const AjPSeq] Old sequence object
** @return [AjPSeq] New sequence object.
** @category new [AjPSeq] Constructor with sequence object to be cloned.
** @@
******************************************************************************/

AjPSeq ajSeqNewS(const AjPSeq seq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajStrAssS(&pthis->Name, seq->Name);
    ajStrAssS(&pthis->Acc, seq->Acc);
    ajStrAssS(&pthis->Sv, seq->Sv);
    ajStrAssS(&pthis->Gi, seq->Gi);
    ajStrAssS(&pthis->Tax, seq->Tax);
    ajStrAssS(&pthis->Type, seq->Type);
    ajStrAssS(&pthis->Db, seq->Db);
    ajStrAssS(&pthis->Full, seq->Full);
    ajStrAssS(&pthis->Date, seq->Date);
    ajStrAssS(&pthis->Desc, seq->Desc);
    ajStrAssS(&pthis->Doc, seq->Doc);
    ajStrAssS(&pthis->Usa, seq->Usa);
    ajStrAssS(&pthis->Ufo, seq->Ufo);
    ajStrAssS(&pthis->Formatstr, seq->Formatstr);
    ajStrAssS(&pthis->Filename, seq->Filename);
    ajStrAssS(&pthis->Entryname, seq->Entryname);
    ajStrAssS(&pthis->Seq, seq->Seq);

    if(seq->TextPtr)
	ajStrAssS(&pthis->TextPtr, seq->TextPtr);


    pthis->Rev      = seq->Rev;
    pthis->Reversed = seq->Reversed;

    pthis->EType  = seq->EType;
    pthis->Format = seq->Format;
    pthis->Begin  = seq->Begin;
    pthis->End    = seq->End;
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




/* @func ajSeqNewStr **********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string
**
** @param [r] seq [const AjPStr] Sequence string
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewStr(const AjPStr seq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNew();
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Date = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();

    ajStrAssS(&pthis->Seq, seq);

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;

    pthis->EType  = 0;
    pthis->Format = 0;
    pthis->Begin  = 0;
    pthis->End    = 0;
    pthis->Offset = 0;
    pthis->Offend = 0;
    pthis->Weight = 1.0;

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();
    pthis->Selexdata = NULL;

    return pthis;
}




/* @func ajSeqNewC **********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a char*
**
** @param [r] seq [const char*] Sequence string
** @param [r] name [const char*] Sequence name
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewC(const char* seq, const char* name)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajStrAssC(&pthis->Name, name);
    pthis->Acc  = ajStrNew();
    pthis->Sv   = ajStrNew();
    pthis->Gi   = ajStrNew();
    pthis->Tax  = ajStrNew();
    pthis->Type = ajStrNew();
    pthis->Db   = ajStrNew();
    pthis->Full = ajStrNew();
    pthis->Date = ajStrNew();
    pthis->Desc = ajStrNew();
    pthis->Doc  = ajStrNew();
    pthis->Usa  = ajStrNew();
    pthis->Ufo  = ajStrNew();

    pthis->Formatstr = ajStrNew();
    pthis->Filename  = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->TextPtr   = ajStrNew();

    ajStrAssC(&pthis->Seq, seq);

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;

    pthis->EType  = 0;
    pthis->Format = 0;
    pthis->Begin  = 0;
    pthis->End    = 0;
    pthis->Offset = 0;
    pthis->Offend = 0;
    pthis->Weight = 1.0;

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();
    pthis->Selexdata = NULL;

    return pthis;
}




/* @func ajStockholmNew *******************************************************
**
** Creates and initialises a Stockholm object.
**
** @param [r] i [ajint] Number of sequences
** @return [AjPStockholm] New sequence object.
** @category new [AjPStockholm] Default constructor
** @@
******************************************************************************/

AjPStockholm ajStockholmNew(ajint i)
{
    AjPStockholm thys = NULL;

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




/* @func ajStockholmdataNew ***************************************************
**
** Creates and initialises a Stockholm data object.
**
** @return [AjPStockholmdata] New sequence object.
** @category new [AjPStockholmdata] Default constructor
** @@
******************************************************************************/

AjPStockholmdata ajStockholmdataNew(void)
{
    AjPStockholmdata thys = NULL;

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




/* @func ajSelexSQNew *********************************************************
**
** Creates and initialises a selex #=SQ line object.
**
** @return [AjPSelexSQ] New sequence object.
** @category new [AjPSelexSQ] Default constructor
** @@
******************************************************************************/

AjPSelexSQ ajSelexSQNew()
{
    AjPSelexSQ thys = NULL;

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
** @category new [AjPSelex] Default constructor
** @@
******************************************************************************/

AjPSelex ajSelexNew(ajint n)
{
    AjPSelex thys = NULL;
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
** @category new [AjPSelexdata] Default constructor
** @@
******************************************************************************/

AjPSelexdata ajSelexdataNew(void)
{
    AjPSelexdata thys = NULL;

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
** @param [d] pthis [AjPSeq*] Sequence object
** @return [void]
** @category delete [AjPSeq] Default destructor
** @@
******************************************************************************/

void ajSeqDel(AjPSeq* pthis)
{
    AjPSeq thys;
    AjPStr ptr = NULL;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;
    if(!*pthis)
	return;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Acc);
    ajStrDel(&thys->Sv);
    ajStrDel(&thys->Gi);
    ajStrDel(&thys->Tax);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Db);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Date);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Doc);
    ajStrDel(&thys->Usa);
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Entryname);
    ajStrDel(&thys->TextPtr);
    ajStrDel(&thys->Seq);

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

    AJFREE(*pthis);
    return;
}




/* @func ajStockholmDel *******************************************************
**
** Deletes a Stockholm object.
**
** @param [d] thys [AjPStockholm*] Stockholm object
** @return [void]
** @category delete [AjPStockholm] Default destructor
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




/* @func ajStockholmdataDel ***************************************************
**
** Deletes a Stockholm data object.
**
** @param [d] thys [AjPStockholmdata*] Stockholm object
** @return [void]
** @category delete [AjPStockholmdata] Default destructor
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




/* @func ajSelexSQDel *********************************************************
**
** Deletes a Selex object.
**
** @param [d] thys [AjPSelexSQ*] Selex #=SQ object
** @return [void]
** @category delete [AjPSelexSQ] Default destructor
** @@
******************************************************************************/

void ajSelexSQDel(AjPSelexSQ *thys)
{
    AjPSelexSQ pthis;

    pthis = *thys;

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
** @param [d] thys [AjPSelex*] Selex object
** @return [void]
** @category delete [AjPSelex] Default destructor
** @@
******************************************************************************/

void ajSelexDel(AjPSelex *thys)
{
    AjPSelex pthis;
    ajint    i;
    ajint    n;

    pthis = *thys;

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
** @param [d] thys [AjPSelexdata*] Selex data object
** @return [void]
** @category delete [AjPSelexdata] Default destructor
** @@
******************************************************************************/

void ajSelexdataDel(AjPSelexdata *thys)
{
    AjPSelexdata pthis;

    pthis = *thys;

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
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqClear(AjPSeq thys)
{
    AjPStr ptr = NULL;

    ajStrClear(&thys->Name);
    ajStrClear(&thys->Acc);
    ajStrClear(&thys->Sv);
    ajStrClear(&thys->Gi);
    ajStrClear(&thys->Tax);
    ajStrClear(&thys->Type);
    ajStrClear(&thys->Db);
    ajStrClear(&thys->Full);
    ajStrClear(&thys->Date);
    ajStrClear(&thys->Desc);
    ajStrClear(&thys->Doc);
    ajStrClear(&thys->Usa);
    ajStrClear(&thys->Ufo);

    ajStrClear(&thys->Formatstr);
    ajStrClear(&thys->Filename);
    ajStrClear(&thys->Entryname);
    ajStrClear(&thys->TextPtr);
    ajStrClear(&thys->Seq);

    thys->Begin = 0;
    thys->End   = 0;
    thys->Rev      = ajFalse;
    thys->Reversed = ajFalse;

    while(ajListstrPop(thys->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(thys->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(thys->Taxlist,&ptr))
	ajStrDel(&ptr);

    ajFeattableDel(&thys->Fttable);

    return;
}




/* @func ajSeqallClear ********************************************************
**
** Resets all data for a sequence stream object so that it can be reused.
**
** @param [u] thys [AjPSeqall] Sequence stream
** @return [void]
** @@
******************************************************************************/

void ajSeqallClear(AjPSeqall thys)
{
    ajSeqClear(thys->Seq);
    ajSeqinClear(thys->Seqin);
    thys->Count = 0;
    thys->Begin = 0;
    thys->End   = 0;
    thys->Rev   = ajFalse;
  
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
** @param [r] str [const AjPStr] Name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssName(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Name, str);

    return;
}




/* @func ajSeqAssNameC ********************************************************
**
** Assigns the sequence name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] Name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssNameC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Name, text);

    return;
}




/* @func ajSeqAssAcc **********************************************************
**
** Assigns the sequence accession number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Accession number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssAcc(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Acc, str);

    return;
}




/* @func ajSeqAssAccC *********************************************************
**
** Assigns the sequence accession number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] Accession number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssAccC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Acc, text);

    return;
}




/* @func ajSeqAssSv ***********************************************************
**
** Assigns the sequence version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] SeqVersion number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSv(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Sv, str);

    return;
}




/* @func ajSeqAssSvC **********************************************************
**
** Assigns the sequence version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] SeqVersion number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSvC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Sv, text);

    return;
}




/* @func ajSeqAssGi ***********************************************************
**
** Assigns the GI version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] GI number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssGi(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Gi, str);

    return;
}




/* @func ajSeqAssGiC **********************************************************
**
** Assigns the GI version number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] GI number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssGiC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Gi, text);

    return;
}




/* @func ajSeqAssUfo **********************************************************
**
** Assigns the sequence feature full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] UFO as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUfo(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Ufo, str);

    return;
}




/* @func ajSeqAssUfoC *********************************************************
**
** Assigns the sequence feature name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] UFO as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUfoC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Ufo, text);

    return;
}




/* @func ajSeqAssUsa **********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] USA as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUsa(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Usa, str);

    return;
}




/* @func ajSeqAssUsaC *********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] USA as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUsaC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Usa, text);

    return;
}




/* @func ajSeqAssEntry ********************************************************
**
** Assigns the sequence entry name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Entry name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssEntry(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Entryname, str);

    return;
}




/* @func ajSeqAssEntryC *******************************************************
**
** Assigns the sequence entryname.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] Entry name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssEntryC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Entryname, text);

    return;
}




/* @func ajSeqAssFull *********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Full name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFull(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Full, str);

    return;
}




/* @func ajSeqAssFullC ********************************************************
**
** Assigns the sequence name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] Full name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFullC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Full, text);

    return;
}




/* @func ajSeqAssFile *********************************************************
**
** Assigns the sequence file name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] File name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFile(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Filename, str);

    return;
}




/* @func ajSeqAssFileC ********************************************************
**
** Assigns the sequence filename.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] File name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFileC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Filename, text);

    return;
}




/* @func ajSeqAssSeq **********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] New sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeq(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Seq, str);
    thys->Begin  = 0;
    thys->End    = 0;
    thys->Offset = 0;
    thys->Offend = 0;
    thys->Rev      = ajFalse;
    thys->Reversed = ajFalse;

    return;
}




/* @func ajSeqAssSeqC *********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] New sequence as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeqC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Seq, text);

    return;
}




/* @func ajSeqAssSeqCI ********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] New sequence as a C character string.
** @param [r] ilen [ajint] Numbur of characters to use
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeqCI(AjPSeq thys, const char* text, ajint ilen)
{
    ajStrAssCI(&thys->Seq, text, ilen);

    return;
}




/* @func ajSeqAssDesc *********************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] New description as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssDesc(AjPSeq thys, const AjPStr str)
{
    ajStrAssS(&thys->Desc, str);

    return;
}




/* @func ajSeqAssDescC ********************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [const char*] New description as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssDescC(AjPSeq thys, const char* text)
{
    ajStrAssC(&thys->Desc, text);

    return;
}




/* @func ajSeqMod *************************************************************
**
** Makes a sequence modifiable by making sure there is no duplicate
** copy of the sequence.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeq] Sets a sequence as modifiable by making
**                           its sequence into a unique AjPStr.
** @@
******************************************************************************/

void ajSeqMod(AjPSeq thys)
{
    ajStrMod(&thys->Seq);

    return;
}




/* @func ajSeqReplace *********************************************************
**
** Replaces a sequence with the contents of a string.
**
** @param [u] thys [AjPSeq] Sequence
** @param [r] seq [const AjPStr] New sequence
** @return [void]
** @category modify [AjPSeq] Replaces a sequence with a string containing a
**                           modified version.
** @@
******************************************************************************/

void ajSeqReplace(AjPSeq thys, const AjPStr seq)
{
    ajStrAssS(&thys->Seq, seq);
    thys->Begin  = 0;
    thys->End    = 0;
    thys->Offset = 0;
    thys->Offend = 0;
    thys->Rev      = ajFalse;
    thys->Reversed = ajFalse;

    return;
}




/* @func ajSeqReplaceC ********************************************************
**
** Replaces a sequence with the contents of a C text string.
**
** @param [u] thys [AjPSeq] Sequence
** @param [r] seq [const char*] New sequence
** @return [void]
** @category modify [AjPSeq] Replaces a sequence with a char* containing a
**                           modified version.
** @@
******************************************************************************/

void ajSeqReplaceC(AjPSeq thys, const char* seq)
{
    ajStrAssC(&thys->Seq, seq);
    thys->Begin  = 0;
    thys->End    = 0;
    thys->Offset = 0;
    thys->Offend = 0;
    thys->Rev      = ajFalse;
    thys->Reversed = ajFalse;
    return;
}




/* @func ajSeqSetRange ********************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set).
** At one time reverse complemented a nucleotide sequence if required
** but this is now not done. It upsets sequence trimming.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @category modify [AjPSeq] Sets a sequence using specified start and end
**                           positions.
** @@
******************************************************************************/

void ajSeqSetRange(AjPSeq seq, ajint ibegin, ajint iend)
{
    ajDebug("ajSeqSetRange (len: %d %d..%d old %d..%d)\n",
	    ajSeqLen(seq), ibegin, iend,
	    seq->Begin, seq->End);

    if(ibegin && !seq->Begin)
	seq->Begin = ibegin;

    if(iend && !seq->End)
	seq->End = iend;

    ajDebug("      result: (len: %d %d..%d)\n",
	    ajSeqLen(seq), seq->Begin, seq->End);

    if(seq->Rev)
	ajSeqReverse(seq);

    return;
}




/* @func ajSeqMakeUsa ********************************************************
**
** Sets the USA for a sequence.
**
** @param [u] thys [AjPSeq] Sequence object to be set.
** @param [r] seqin [const AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqMakeUsa(AjPSeq thys, const AjPSeqin seqin)
{
    ajSeqMakeUsaS(thys, seqin, &thys->Usa);
}

/* @func ajSeqMakeUsaS ********************************************************
**
** Sets the USA for a sequence.
**
** @param [r] thys [const AjPSeq] Sequence object
** @param [r] seqin [const AjPSeqin] Sequence input object.
** @param [w] usa [AjPStr*] USA in full
** @return [void]
** @@
******************************************************************************/

void ajSeqMakeUsaS(const AjPSeq thys, const AjPSeqin seqin, AjPStr* usa)
{
    AjPStr tmpstr = NULL;

    ajDebug("ajSeqMakeUsa (Name <%S> Formatstr <%S> Db <%S> "
	    "Entryname <%S> Filename <%S>)\n",
	    thys->Name, thys->Formatstr, thys->Db,
	    thys->Entryname, thys->Filename);

    /* ajSeqTrace(thys); */

    if(seqin)
	ajSeqinTrace(seqin);

    if(ajStrLen(thys->Db))
	ajFmtPrintS(usa, "%S-id:%S", thys->Db, thys->Entryname);
    else
    {
	/*ajFmtPrintS(&thys->Usa, "%S::%S (%S)",
	  thys->Formatstr, thys->Filename, thys->Entryname);*/
	if(ajStrLen(thys->Entryname))
	    ajFmtPrintS(usa, "%S::%S:%S", thys->Formatstr,
			thys->Filename,thys->Entryname);
	else
	    ajFmtPrintS(usa, "%S::%S", thys->Formatstr,
			thys->Filename);

    }

    ajFmtPrintS(&tmpstr, "[");

    if(thys->Rev)
    {
	if(thys->End)
	    ajFmtPrintAppS(&tmpstr, "%d", -thys->End);

	ajFmtPrintAppS(&tmpstr, ":");

	if(thys->Begin)
	    ajFmtPrintAppS(&tmpstr, "%d", -thys->Begin);

	ajFmtPrintAppS(&tmpstr, ":r");
    }
    else
    {
	if(thys->Begin)
	    ajFmtPrintAppS(&tmpstr, "%d", thys->Begin);

	ajFmtPrintAppS(&tmpstr, ":");

	if(thys->End)
	    ajFmtPrintAppS(&tmpstr, "%d", thys->End);
    }

    ajFmtPrintAppS(&tmpstr, "]");

    if(ajStrLen(tmpstr) > 3)
	ajStrApp(usa, tmpstr);

    ajStrDel(&tmpstr);
    ajDebug("      result: <%S>\n",
	    *usa);

    return;
}




/* @func ajSeqToUpper *********************************************************
**
** Converts a sequence to upper case.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeq] Converts a sequence to upper case
** @@
******************************************************************************/

void ajSeqToUpper(AjPSeq thys)
{
    ajStrToUpper(&thys->Seq);

    return;
}




/* @func ajSeqToLower *********************************************************
**
** Converts a sequence to lower case.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeq] Converts a sequence to lower case
** @@
******************************************************************************/

void ajSeqToLower(AjPSeq thys)
{
    ajStrToLower(&thys->Seq);

    return;
}




/* @func ajSeqReverse *********************************************************
**
** Reverses and complements a nucleotide sequence, nuless it is already done.
**
** If the sequence may have been reversed already, use ajSeqReverseForce
** to make sure the sequence is reversed.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeq] Reverse complements a nucleotide sequence
** @@
******************************************************************************/

void ajSeqReverse(AjPSeq thys)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqReverse len: %d Begin: %d End: %d Rev: %B Reversed: %B\n",
	    ajSeqLen(thys), thys->Begin, thys->End,
	    thys->Rev, thys->Reversed);

    if(thys->Reversed)	       /* means we have already reversed it */
	return;

    ibegin = thys->Begin;
    iend   = thys->End;

    thys->End   = -(ibegin);
    thys->Begin = -(iend);

    thys->Reversed = ajTrue;
    if(!thys->Rev)
	thys->Rev = ajTrue;

    ajSeqReverseStr(&thys->Seq);

    ajDebug("      result len: %d Begin: %d End: %d\n",
	    ajSeqLen(thys), thys->Begin, thys->End);

    if(thys->Fttable)
	ajFeattableReverse(thys->Fttable);

    return;
}




/* @func ajSeqReverseForce ****************************************************
**
** Reverses and complements a nucleotide sequence.
** Forces reversal to be done even if the sequence is flagged
** as already reversed.
**
** This happens, for example, where an input sequence has been reversed
** with -sreverse on the command line, but the application needs to reverse it
** in processing both directions.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReverseForce(AjPSeq thys)
{
    ajDebug("ajSeqReverse len: %d Begin: %d End: %d Rev: %B Reversed: %B\n",
	    ajSeqLen(thys), thys->Begin, thys->End,
	    thys->Rev, thys->Reversed);

    if(thys->Reversed)	          /* means we have already reversed it */
	thys->Reversed = ajFalse; /* but we want to reverse it anyway */

    ajSeqReverse(thys);

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

void ajSeqReverseStr(AjPStr* pthis)
{
    char *cp;
    char *cq;
    char tmp;

    cp = ajStrStrMod(pthis);
    cq = cp + ajStrLen(*pthis) - 1;

    while(cp < cq)
    {
	tmp = ajSeqBaseComp(*cp);
	*cp = ajSeqBaseComp(*cq);
	*cq = tmp;
	cp++;
	cq--;
    }

    if(cp == cq)
	*cp = ajSeqBaseComp(*cp);

    return;
}




/* @func ajSeqCompOnly ********************************************************
**
** Complements but does not reverse a nucleotide sequence.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeq] Complements a nucleotide sequence
**                           (does not reverse)
** @@
******************************************************************************/

void ajSeqCompOnly(AjPSeq thys)
{
    ajSeqCompOnlyStr(&thys->Seq);

    return;
}




/* @func ajSeqRevOnly *********************************************************
**
** Reverses but does not complement a nucleotide sequence.
**
** @param [u] thys [AjPSeq] Sequence
** @return [void]
** @category modify [AjPSeq] Reverses a sequence (does not complement)
** @@
******************************************************************************/

void ajSeqRevOnly(AjPSeq thys)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqRevOnly len: %d Begin: %d End: %d\n",
	    ajSeqLen(thys), thys->Begin, thys->End);

    ibegin = thys->Begin;
    iend   = thys->End;

    if(ibegin)
	thys->End   = -(ibegin);
    if(iend)
	thys->Begin = -(iend);

    ajStrRev(&thys->Seq);

    ajDebug(" only result len: %d Begin: %d End: %d\n",
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

void ajSeqCompOnlyStr(AjPStr* pthis)
{
    char *cp;

    cp = ajStrStrMod(pthis);

    while(*cp)
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
** @param [r] base [char] Base character.
** @return [char] Complementary base.
** @@
******************************************************************************/

char ajSeqBaseComp(char base)
{
    static char fwd[]="ACGTURYWSMKBDHVNXacgturywsmkbdhvnx";
    static char rev[]="TGCAAYRWSKMVHDBNXtgcaayrwskmvhdbnx";
    char *cp;
    char *cq;

    cp = strchr(fwd,base);
    if(cp)
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
** @param [r] thys [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
** @category use [AjPSeq] tests whether a sequence is nucleotide
** @@
******************************************************************************/

AjBool ajSeqIsNuc(const AjPSeq thys)
{
    ajDebug("ajSeqIsNuc Type '%S'\n", thys->Type);

    if(ajStrMatchC(thys->Type, "N"))
	return ajTrue;

    if(ajStrMatchC(thys->Type, "P"))
	return ajFalse;

    if(ajSeqTypeGapnucS(thys->Seq)) /* returns char 0 on success */
    {
	ajDebug ("ajSeqIsNuc failed\n", thys->Type);
	return ajFalse;
    }

    return ajTrue;
}




/* @func ajSeqIsProt **********************************************************
**
** Tests whether a sequence is protein.
**
** @param [r] thys [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a protein sequence.
** @category use [AjPSeq] tests whether a sequence is protein
** @@
******************************************************************************/

AjBool ajSeqIsProt(const AjPSeq thys)
{
    ajDebug("ajSeqIsProt Type '%S'\n", thys->Type);

    if(ajStrMatchC(thys->Type, "P"))
	return ajTrue;

    if(ajStrMatchC(thys->Type, "N"))
	return ajFalse;

    if(ajSeqTypeAnyprotS(thys->Seq))	/* returns char 0 on success */
    {
	ajDebug ("ajSeqIsProt failed\n", thys->Type);
	return ajFalse;
    }

    return ajTrue;
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
** @param [r] accnum [const AjPStr] String to be tested
** @return [AjBool] ajTrue if the string is a possible accession number.
** @@
******************************************************************************/

AjBool ajIsAccession(const AjPStr accnum)
{
    ajint i;
    const char *cp;

    if(!accnum)
	return ajFalse;

    i = ajStrLen(accnum);
    if(i < 6)
	return ajFalse;

    cp = ajStrStr(accnum);

    /* must have an alphabetic start */

    if(!isalpha((ajint)*cp++))
	return ajFalse;

    /* two choices for the next character */

    if(isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
	cp++;

	if(*cp == '_') cp++;		/* REFSEQ NM_nnnnnn */

	while(*cp)
	    if(isdigit((ajint)*cp))
		++cp;
	    else
		return ajFalse;

	return ajTrue;
    }
    else if(isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
	cp++;				/* or SWISS AnXXXn */

	for(i=0; i<3; i++)
	    if(isalpha((ajint)*cp) || isdigit((ajint)*cp))
		cp++;
	    else
		return ajFalse;

	if(!isdigit((ajint)*cp))
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
** @param [r] sv [const AjPStr] String to be tested
** @return [AjPStr] accession number part of the string if successful
** @@
******************************************************************************/

AjPStr ajIsSeqversion(const AjPStr sv)
{
    ajint i;
    const char *cp;
    AjBool dot = ajFalse;		/* have we found the '.' */
    AjBool v = 0;	   /* number of digits of version after '.' */
    static AjPStr accnum = NULL;

    if(!sv)
	return NULL;

    i = ajStrLen(sv);
    if(i < 8)
	return NULL;

    cp = ajStrStr(sv);
    ajStrAssCL(&accnum, "", 12);

    /* must have an alphabetic start */

    if(!isalpha((ajint)*cp))
	return NULL;

    ajStrAppK(&accnum, *cp++);

    /* two choices for the next character */

    if(isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
        ajStrAppK(&accnum, *cp);
	cp++;

	if(*cp == '_')		/* REFSEQ NM_nnnnnn */
	{
	    ajStrAppK(&accnum, *cp);
	    cp++;
	}
	while(*cp)		      /* optional trailing .version */
	{
	    if(isdigit((ajint)*cp) || *cp=='.')
	    {
		if(*cp == '.')
		{
		    if(dot)
			return NULL;	/* one '.' only */
		    dot = ajTrue;
		}
		else
		{
		    if(dot)
			v++;
		    else
			ajStrAppK(&accnum, *cp);
		}
		++cp;
	    }
	    else
		return NULL;
	}
	if(v)
	    return accnum;
	else
	    return NULL;
    }
    else if(isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
	/* or SWISS AnXXXn */
        ajStrAppK(&accnum, *cp);
	cp++;

	for(i=0; i<3; i++)
	    if(isalpha((ajint)*cp) || isdigit((ajint)*cp))
	    {
	        ajStrAppK(&accnum, *cp);
		cp++;
	    }
	    else
		return NULL;

	if(!isdigit((ajint)*cp))
	    return NULL;

	while(*cp)		      /* optional trailing .version */
	{
	    if(isdigit((ajint)*cp) || *cp=='.')
	    {
		if(*cp == '.')
		{
		    if(dot)
			return NULL; /* one '.' only */
		    dot = ajTrue;
		}
		else
		{
		    if(dot)
			v++;
		    else
			ajStrAppK(&accnum, *cp);
		}
		++cp;
	    }
	    else
		return NULL;
	}
	if(v)
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
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
** @category output [AjPSeq] Reports the contents of a sequence
** @@
******************************************************************************/

void ajSeqTrace(const AjPSeq seq)
{
    AjIList it;
    AjPStr cur;

    ajDebug("Sequence trace\n");
    ajDebug( "==============\n\n");
    ajDebug( "  Name: '%S'\n", seq->Name);

    if(ajStrLen(seq->Acc))
	ajDebug( "  Accession: '%S'\n", seq->Acc);

    if(ajListLength(seq->Acclist))
    {
	ajDebug( "  Acclist: (%d) ", ajListLength(seq->Acclist));
	it = ajListIterRead(seq->Acclist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" %S", cur);

	ajListIterFree(&it);
	ajDebug(" \n");
    }

    if(ajStrLen(seq->Sv))
	ajDebug( "  SeqVersion: '%S'\n", seq->Sv);

    if(ajStrLen(seq->Gi))
	ajDebug( "  GI Version: '%S'\n", seq->Gi);

    if(ajStrLen(seq->Type))
	ajDebug( "  Type: '%S' (%d)\n", seq->Type, seq->EType);

    if(ajStrLen(seq->Desc))
	ajDebug( "  Description: '%S'\n", seq->Desc);

    if(ajStrLen(seq->Tax))
	ajDebug( "  Taxonomy: '%S'\n", seq->Tax);

    if(ajListLength(seq->Taxlist))
    {
	ajDebug( "  Taxlist: (%d)", ajListLength(seq->Taxlist));
	it = ajListIterRead(seq->Taxlist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" '%S'", cur);

	ajListIterFree(&it);
	ajDebug("\n");
    }

    if(ajListLength(seq->Keylist))
    {
	ajDebug( "  Keywordlist: (%d)", ajListLength(seq->Keylist));
	it = ajListIterRead(seq->Keylist);
	while((cur = (AjPStr) ajListIterNext(it)))
	    ajDebug(" '%S'", cur);

	ajListIterFree(&it);
	ajDebug("\n");
    }

    if(ajSeqLen(seq))
	ajDebug( "  Length: %d\n", ajSeqLen(seq));

    if(seq->Rev)
	ajDebug( "     Rev: %B\n", seq->Rev);

    if(seq->Reversed)
	ajDebug( "Reversed: %B\n", seq->Reversed);

    if(seq->Begin)
	ajDebug( "   Begin: %d\n", ajSeqBegin(seq));

    if(seq->End)
	ajDebug( "     End: %d\n", ajSeqEnd(seq));

    if(seq->Offset)
	ajDebug( "  Offset: %d\n", seq->Offset);

    if(seq->Offend)
	ajDebug( "  Offend: %d\n", seq->Offend);

    if(ajStrSize(seq->Seq))
	ajDebug( "  Reserved: %d\n", ajStrSize(seq->Seq));

    if(ajStrLen(seq->Db))
	ajDebug( "  Database: '%S'\n", seq->Db);

    if(ajStrLen(seq->Full))
	ajDebug( "  Full name: '%S'\n", seq->Full);

    if(ajStrLen(seq->Date))
	ajDebug( "  Date: '%S'\n", seq->Date);

    if(ajStrLen(seq->Usa))
	ajDebug( "  Usa: '%S'\n", seq->Usa);

    if(ajStrLen(seq->Ufo))
	ajDebug( "  Ufo: '%S'\n", seq->Ufo);

    if(seq->Fttable)
	ajDebug( "  Fttable: exists\n");

    if(ajStrLen(seq->Formatstr))
	ajDebug( "  Input format: '%S' (%d)\n", seq->Formatstr, seq->Format);

    if(ajStrLen(seq->Filename))
	ajDebug( "  Filename: '%S'\n", seq->Filename);

    if(ajStrLen(seq->Entryname))
	ajDebug( "  Entryname: '%S'\n", seq->Entryname);

    if(seq->Weight)
	ajDebug( "  Weight: %.3f\n", seq->Weight);

    if(ajStrLen(seq->Doc))
	ajDebug( "  Documentation:...\n%S\n", seq->Doc);

    return;
}




/* @func ajSeqinTrace *********************************************************
**
** Debug calls to trace the data in a sequence input object.
**
** @param [r] thys [const AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqinTrace(const AjPSeqin thys)
{
    ajDebug("Sequence input trace\n");
    ajDebug( "====================\n\n");
    ajDebug( "  Name: '%S'\n", thys->Name);

    if(ajStrLen(thys->Acc))
	ajDebug( "  Accession: '%S'\n", thys->Acc);

    if(ajStrLen(thys->Inputtype))
	ajDebug( "  Inputtype: '%S'\n", thys->Inputtype);

    if(ajStrLen(thys->Desc))
	ajDebug( "  Description: '%S'\n", thys->Desc);

    if(ajStrLen(thys->Inseq))
	ajDebug( "  Inseq len: %d\n", ajStrLen(thys->Inseq));

    if(thys->Rev)
	ajDebug( "     Rev: %B\n", thys->Rev);

    if(thys->Begin)
	ajDebug( "   Begin: %d\n", thys->Begin);

    if(thys->End)
	ajDebug( "     End: %d\n", thys->End);

    if(ajStrLen(thys->Db))
	ajDebug( "  Database: '%S'\n", thys->Db);

    if(ajStrLen(thys->Full))
	ajDebug( "  Full name: '%S'\n", thys->Full);

    if(ajStrLen(thys->Date))
	ajDebug( "  Date: '%S'\n", thys->Date);

    if(ajListLength(thys->List))
	ajDebug( "  List: (%d)\n", ajListLength(thys->List));

    if(thys->Filebuff)
	ajDebug( "  Filebuff: %F (%ld)\n",
		ajFileBuffFile(thys->Filebuff),
		ajFileTell(ajFileBuffFile(thys->Filebuff)));

    if(ajStrLen(thys->Usa))
	ajDebug( "  Usa: '%S'\n", thys->Usa);

    if(ajStrLen(thys->Ufo))
	ajDebug( "  Ufo: '%S'\n", thys->Ufo);

    if(thys->Fttable)
	ajDebug( "  Fttable: exists\n");

    if(thys->Ftquery)
	ajDebug( "  Ftquery: exists\n");

    if(ajStrLen(thys->Formatstr))
	ajDebug( "  Input format: '%S' (%d)\n", thys->Formatstr,
		thys->Format);

    if(ajStrLen(thys->Filename))
	ajDebug( "  Filename: '%S'\n", thys->Filename);

    if(ajStrLen(thys->Entryname))
	ajDebug( "  Entryname: '%S'\n", thys->Entryname);

    if(thys->Search)
	ajDebug( "  Search: %B\n", thys->Search);

    if(thys->Single)
	ajDebug( "  Single: %B\n", thys->Single);

    if(thys->Features)
	ajDebug( "  Features: %B\n", thys->Features);

    if(thys->IsNuc)
	ajDebug( "  IsNuc: %B\n", thys->IsNuc);

    if(thys->IsProt)
	ajDebug( "  IsProt: %B\n", thys->IsProt);

    if(thys->Count)
	ajDebug( "  Count: %d\n", thys->Count);

    if(thys->Filecount)
	ajDebug( "  Filecount: %d\n", thys->Filecount);

    if(thys->Fpos)
	ajDebug( "  Fpos: %l\n", thys->Fpos);

    if(thys->Query)
	ajSeqQueryTrace(thys->Query);

    if(thys->Data)
	ajDebug( "  Data: exists\n");

    if(ajStrLen(thys->Doc))
	ajDebug( "  Documentation:...\n%S\n", thys->Doc);

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
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Start position.
** @category cast [AjPSeq] Returns the sequence start position
** @@
******************************************************************************/

ajint ajSeqBegin(const AjPSeq seq)
{
    if(!seq->Begin)
	return 1;

    return ajSeqPos(seq, seq->Begin);
}




/* @func ajSeqEnd *************************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] End position.
** @category cast [AjPSeq] Returns the sequence end position
** @@
******************************************************************************/

ajint ajSeqEnd(const AjPSeq seq)
{
    if(!seq->End)
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
** @param [r] seq [const AjPSeq] Sequence object
** @return [const char*] Sequence name as a null terminated character string.
** @category cast [AjPSeq] Returns the sequence name as char
** @@
******************************************************************************/

const char* ajSeqName(const AjPSeq seq)
{
    return ajStrStr(seq->Name);
}




/* @func ajSeqOffset **********************************************************
**
** Returns the sequence offset from -sbegin originally.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Sequence offset.
** @@
******************************************************************************/

ajint ajSeqOffset(const AjPSeq seq)
{
    return seq->Offset;
}




/* @func ajSeqOffend **********************************************************
**
** Returns the sequence offend value.
** This is the number of positions removed from the original end.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Sequence offend.
** @@
******************************************************************************/

ajint ajSeqOffend(const AjPSeq seq)
{
    return seq->Offend;
}




/* @func ajSeqLen *************************************************************
**
** Returns the sequence length.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Sequence length.
** @category cast [AjPSeq] Returns the sequence length
** @@
******************************************************************************/

ajint ajSeqLen(const AjPSeq seq)
{
    return ajStrLen(seq->Seq);
}




/* @func ajSeqGetReverse ******************************************************
**
** Returns the sequence direction.
**
** See ajSeqReversed for whether it has already been reverse-complemented
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
** @@
******************************************************************************/

AjBool ajSeqGetReverse(const AjPSeq seq)
{
    return seq->Rev;
}




/* @func ajSeqGetReversed *****************************************************
**
** Returns whether the sequence has been reversed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
** @@
******************************************************************************/

AjBool ajSeqGetReversed(const AjPSeq seq)
{
    return seq->Reversed;
}




/* @func ajSeqCharCopy ********************************************************
**
** Returns a sequence as a C character string. This is a copy of the string
** so the caller can do anything with it.
** It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [char*] Sequence as a null terminated character string.
** @category cast [AjPSeq] Returns a copy of the sequence as char*.
** @@
******************************************************************************/

char* ajSeqCharCopy(const AjPSeq seq)
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
** @param [r] seq [const AjPSeq] Sequence object
** @param [r] size [size_t] Maximum length as returned by strlen
** @return [char*] Sequence as a null terminated character string.
** @category cast [AjPSeq] Returns a copy of the sequence as char* with a
**                         specified minimum reserved length.
** @@
******************************************************************************/

char* ajSeqCharCopyL(const AjPSeq seq, size_t size)
{
    return ajCharNewLS(size, seq->Seq);
}




/* @func ajSeqNum *************************************************************
**
** Converts a sequence to numbers using a conversion table.
**
** @param [r] thys [const AjPSeq] Sequence.
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @param [w] numseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
** @category cast [AjPSeq] Convert sequence to numbers
** @@
******************************************************************************/

AjBool ajSeqNum(const AjPSeq thys, const AjPSeqCvt cvt, AjPStr* numseq)
{
    return ajSeqNumS(thys->Seq, cvt, numseq);;
}




/* @func ajSeqNumS ************************************************************
**
** Converts a string of sequence characters to numbers using
** a conversion table.
**
** @param [r] thys [const AjPStr] Sequence as a string
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @param [w] numseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqNumS(const AjPStr thys, const AjPSeqCvt cvt, AjPStr* numseq)
{
    const char *cp;
    char *ncp;

    cp = ajStrStr(thys);

    ajStrAssS(numseq, thys);
    ncp = ajStrStrMod(numseq);

    while(*cp)
    {
	*ncp = cvt->table[(ajint)*cp];
	cp++;
	ncp++;
    }

    return ajTrue;
}




/* @section Sequence Conversion Functions *************************************
**
******************************************************************************/

/* @func ajSeqCvtTrace ********************************************************
**
** Traces a conversion table with debug calls.
**
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @return [void]
** @category output [AjPSeqCvt] Reports on contents for debugging
** @@
******************************************************************************/

void ajSeqCvtTrace(const AjPSeqCvt cvt)
{
    ajint i;

    ajDebug("Cvt table for '%S'\n\n", cvt->bases);
    ajDebug("index num ch\n");
    ajDebug("----- --- --\n");
    for(i=0; i < cvt->size; i++)
	if(cvt->table[i])
	    ajDebug("%5d %3d <%c>\n", i, cvt->table[i], ajSysItoC(i));

    ajDebug("... all others are zero ...\n", cvt->bases);

    return;
}




/* @func ajSeqCvtNewZero ******************************************************
**
** Generates a new conversion table in which the first character in the
** string provided is converted to 1, the second to 2, and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
** @category new [AjPSeqCvt] Creates from a character string of valid bases.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNewZero(const char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    const char *cp;

    cp = bases;

    AJNEW0(ret);
    ret->len     = strlen(bases);
    ret->size    = CHAR_MAX - CHAR_MIN + 1;
    ret->table   = AJCALLOC0(ret->size, sizeof(char));
    ret->bases   = ajStrNewC(bases);
    ret->missing = 0;

    i = 0;
    while(*cp)
    {
	i++;
	ret->table[toupper((ajint) *cp)] = ajSysItoC(i);
	ret->table[tolower((ajint) *cp)] = ajSysItoC(i);
	cp++;
    }

    return ret;
}




/* @func ajSeqCvtNewZeroS *****************************************************
**
** Generates a new conversion table in which the first character of the first 
** string in the array provided is converted to 1, the first character of the 
** second string is converted to 2, the first character of the third string is
** converted to 3 and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
**
** @param [r] bases [const AjPPStr] Allowed sequence character strings (size
**                            specified by parameter n)
** @param [r] n [int] Number of strings
** @return [AjPSeqCvt] Conversion table.
** @category new [AjPSeqCvt] Creates from an array of strings of valid bases.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNewZeroS (const AjPPStr bases, int n)
{
    static AjPSeqCvt ret;
    ajint i;
    

    AJNEW0(ret);
    ret->len = n;
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNew();
    ret->missing = 0;
    AJCNEW0(ret->labels, n);
    for(i=0; i<n; i++)
	ret->labels[i] = ajStrNew();


    for(i=0; i<n; i++)
    {
	ajStrAssS(&ret->labels[i], bases[i]);
	ajStrAppK(&ret->bases, ajStrChar(bases[i], 0));
	ret->table[toupper((ajint) ajStrChar(bases[i], 0))] = ajSysItoC(i+1);
	ret->table[tolower((ajint) ajStrChar(bases[i], 0))] = ajSysItoC(i+1);
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
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
** @category new [AjPSeqCvt] Creates from a character string of valid bases.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNew(const char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    ajint j;
    ajint imax;
    const char *cp;

    cp = bases;

    imax = strlen(bases);

    AJNEW0(ret);
    ret->len     = imax;
    ret->size    = CHAR_MAX - CHAR_MIN + 1;
    ret->table   = AJCALLOC0(ret->size, sizeof(char));
    ret->bases   = ajStrNewC(bases);
    ret->missing = imax;

    for(j=0; j < ret->size; j++)
	ret->table[j] = ajSysItoC(imax);

    i = 0;
    while(*cp)
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
    ajint i=0;
    
    if(!*thys || !thys)
	return;

    AJFREE((*thys)->table);
    ajStrDel(&(*thys)->bases);

    if((*thys)->labels)
    {
	for(i=0;i<(*thys)->len;i++)
	    ajStrDel(&(*thys)->labels[i]);
	AJFREE((*thys)->labels);
    }
    
    AJFREE(*thys);

    return;
}




/* @func ajSeqCvtNewText ******************************************************
**
** Generates a new conversion table in which the characters are retained
** as upper case, numbers are set to -1 and all other characters
** are set to -2.
**
** @param [r] bases [const char*] Allowed sequence characters.
** @return [AjPSeqCvt] Conversion table.
** @category new [AjPSeqCvt] Creates from a character string of valid bases.
** @@
******************************************************************************/

AjPSeqCvt ajSeqCvtNewText(const char* bases)
{
    static AjPSeqCvt ret;
    ajint i;
    ajint j;
    const char *cp;
    char c;

    cp = bases;

    AJNEW0(ret);
    ret->len     = strlen(bases);
    ret->size    = CHAR_MAX - CHAR_MIN + 1;
    ret->table   = AJCALLOC0(ret->size, sizeof(char));
    ret->bases   = ajStrNewC(bases);
    ret->missing = -1;

    for(j=0; j < ret->size; j++)
	if(isdigit(j))
	    ret->table[j] = -1;
	else
	    ret->table[j] = -2;


    i = 0;
    while(*cp)
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
** @param [r] thys [const AjPSeqCvt] Conversion table
**
** @return [ajint] Length
** @@
******************************************************************************/

ajint ajSeqCvtLen(const AjPSeqCvt thys)
{
    return thys->len;
}




/* @func ajSeqCvtK ************************************************************
**
** Returns the integer code corresponding to a sequence character
** in a conversion table
**
** @param [r] thys [const AjPSeqCvt] Conversion table
** @param [r] ch [char] Sequence character
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtK(const AjPSeqCvt thys, char ch)
{
    return thys->table[(ajint)ch];
}




/* @func ajSeqCvtKS ***********************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table
**
** @param [r] thys [const AjPSeqCvt] Conversion table
** @param [r] ch [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtKS (const AjPSeqCvt thys, const AjPStr ch)
{
    ajint i=0;
    
    for(i=0;i<thys->len;i++)
	if(ajStrMatch(ch, thys->labels[i]))
	    return i+1;
    /* i+1 is returned because the size of a matrix is always 1 bigger than
       the number of labels. This is the "padding" first row/column which 
       has all values of 0. */


    ajWarn("Sequence character string not found in ajSeqCvtKS");
    return 0;
}




/* @func ajSeqStr *************************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPSeq] Sequence.
** @return [const AjPStr] Sequence as a string.
** @category cast [AjPSeq] Returns the actual AjPStr holding the sequence.
** @@
******************************************************************************/

const AjPStr ajSeqStr(const AjPSeq thys)
{
    if(!thys)
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
** @param [r] thys [const AjPSeq] Sequence.
** @return [AjPStr] Sequence as a string.
** @@
******************************************************************************/

AjPStr ajSeqStrCopy(const AjPSeq thys)
{
    static AjPStr str;

    str = ajStrNew();
    ajStrAssS(&str, thys->Seq);

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
** @param [r] thys [const AjPSeq] Sequence.
** @return [const char*] Sequence as a null terminated character string.
** @category cast [AjPSeq] Returns the actual char* holding the sequence.
** @@
******************************************************************************/

const char* ajSeqChar(const AjPSeq thys)
{
    if(!thys)
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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] Accession number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetAcc(const AjPSeq thys)
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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] SeqVersion number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetSv(const AjPSeq thys)
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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] SeqVersion number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetGi(const AjPSeq thys)
{
    return thys->Gi;
}




/* @func ajSeqGetTax **********************************************************
**
** Returns the sequence primary taxon (species).
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] Description as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetTax(const AjPSeq thys)
{
    return thys->Tax;
}




/* @func ajSeqGetDesc *********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] Description as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetDesc(const AjPSeq thys)
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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPFeattable] feature table (if any)
** @@
******************************************************************************/

const AjPFeattable ajSeqGetFeat(const AjPSeq thys)
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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [AjPFeattable] feature table (if any)
** @@
******************************************************************************/

AjPFeattable ajSeqCopyFeat(const AjPSeq thys)
{
    return ajFeattableCopy(thys->Fttable);
}




/* @func ajSeqGetName *********************************************************
**
** Returns the sequence name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetName(const AjPSeq thys)
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
** @param [r] thys [const AjPSeq] Sequence object.
** @return [const AjPStr] Entry as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetEntry(const AjPSeq thys)
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
** @category new [AjPSeqout] Default constructor
** @@
******************************************************************************/

AjPSeqout ajSeqoutNew(void)
{
    AjPSeqout pthis;

    AJNEW0(pthis);

    pthis->Name  = ajStrNew();
    /* pthis->Acc = ajStrNew(); */
    pthis->Sv    = ajStrNew();
    pthis->Gi    = ajStrNew();
    pthis->Tax   = ajStrNew();
    pthis->Desc  = ajStrNew();
    pthis->Type  = ajStrNew();
    pthis->EType = 0;

    pthis->Outputtype = ajStrNew();

    pthis->Db    = ajStrNew();
    pthis->Setdb = ajStrNew();
    pthis->Full  = ajStrNew();
    pthis->Date  = ajStrNew();
    pthis->Doc   = ajStrNew();
    pthis->Rev   = ajFalse;
    pthis->Usa   = ajStrNew();
    pthis->Ufo   = ajStrNew();

    pthis->Informatstr = ajStrNew();
    pthis->Formatstr   = ajStrNew();

    pthis->Format    = 0;
    pthis->Filename  = ajStrNew();
    pthis->Directory = ajStrNew();
    pthis->Entryname = ajStrNew();
    pthis->Seq       = ajStrNew();
    pthis->File      = NULL;
    pthis->Count     = 0;
    pthis->Single    = ajFalse;
    pthis->Features  = ajFalse;
    pthis->Extension = ajStrNew();
    pthis->Savelist  = NULL;

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
** @param [u] file [AjPFile] Open file object
** @return [AjPSeqout] New sequence output object.
** @@
******************************************************************************/

AjPSeqout ajSeqoutNewF(AjPFile file)
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

/* @func ajSeqoutDel **********************************************************
**
** Destructor for AjPSeqout objects
**
** @param [d] pthis [AjPSeqout*] Sequence output object
** @return [void]
** @category delete [AjPSeqout] Default destructor
** @@
******************************************************************************/

void ajSeqoutDel(AjPSeqout* pthis)
{
    AjPSeqout thys;
    AjPSeq    seq    = NULL;
    AjPStr    tmpstr = NULL;

    thys = *pthis;

    ajStrDel(&thys->Name);
    /* ajStrDel(&thys->Acc); */
    ajStrDel(&thys->Sv);
    ajStrDel(&thys->Gi);
    ajStrDel(&thys->Tax);
    ajStrDel(&thys->Desc);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Outputtype);
    ajStrDel(&thys->Db);
    ajStrDel(&thys->Setdb);
    ajStrDel(&thys->Full);
    ajStrDel(&thys->Date);
    ajStrDel(&thys->Doc);
    ajStrDel(&thys->Usa);
    ajStrDel(&thys->Ufo);
    ajStrDel(&thys->FtFormat);
    ajStrDel(&thys->FtFilename);
    ajStrDel(&thys->Informatstr);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Filename);
    ajStrDel(&thys->Directory);
    ajStrDel(&thys->Entryname);
    ajStrDel(&thys->Seq);
    ajStrDel(&thys->Extension);

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
** @param [r] seq [const AjPStr] Sequence as a string.
** @return [float] Molecular weight.
** @@
******************************************************************************/

float ajSeqMW(const AjPStr seq)
{
    /* source: Biochemistry LABFAX */
    static float aa[26] = { 089.10, 132.61, 121.16, 133.11, /* A-D */
				147.13, 165.19, 075.07, 155.16,	/* E-H */
				131.18, 000.00, 146.19, 131.18,	/* I-L */
				149.21, 132.12, 000.00, 115.13,	/* M-P */
				146.15, 174.20, 105.09, 119.12,	/* Q-T */
				000.00, 117.15, 204.23, 128.16,	/* U-X */
				181.19, 146.64};
    float mw;
    ajint i;
    const char* cp;

    cp = ajStrStr(seq);
    mw = 18.015;
    
    while(*cp)
    {
	i = toupper((ajint) *cp)-'A';
	if(i > 25 || i < 0)
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
** @param [r] seq [const AjPStr] Sequence as a string
** @return [ajuint] CRC32 checksum.
** @@
******************************************************************************/

ajuint ajSeqCrc(const AjPStr seq)
{
    register ajulong crc;
    ajint c;
    const char* cp;
    static ajint calls = 0;

    if(!calls)
    {
	seqCrcGen();
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

static void seqCrcGen(void)
{
    ajulong crc;
    ajulong poly;
    ajint   i;
    ajint   j;

    poly = 0xEDB88320L;
    for(i=0; i<256; i++)
    {
	crc = i;
	for(j=8; j>0; j--)
	    if(crc&1)
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
** @param [r] thys [const AjPStr] Sequence as a string
** @param [w] b [ajint*] integer array, minimum size 5, to hold the results.
** @return [void]
** @@
******************************************************************************/

void ajSeqCount(const AjPStr thys, ajint* b)
{
    const char* cp;

    ajDebug("ajSeqCount %d bases\n", ajStrLen(thys));

    b[0] = b[1] = b[2] = b[3] = b[4] = 0;

    cp = ajStrStr(thys);

    while(*cp)
    {
	if(toupper((ajint) *cp) == 'A')
	    b[0]++;
	if(toupper((ajint) *cp) == 'C')
	    b[1]++;
	if(toupper((ajint) *cp) == 'G')
	    b[2]++;
	if(toupper((ajint) *cp) == 'T')
	    b[3]++;
	if(toupper((ajint) *cp) == 'U')
	    b[3]++;
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
** @param [r] thys [const AjPSeq] Squence.
** @return [ajint] GCG checksum.
** @category cast [AjPSeq] Calculates the GCG checksum for a
**                         sequence.
** @@
******************************************************************************/

ajint ajSeqCheckGcg(const AjPSeq thys)
{
    register ajlong  i;
    register ajlong  check = 0;
    register ajlong  count = 0;
    const char *cp;
    ajint ilen;

    cp   = ajStrStr(thys->Seq);
    ilen = ajStrLen(thys->Seq);

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




/* @func ajSeqPos *************************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [r] thys [const AjPSeq] Target sequence.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqPos(const AjPSeq thys, ajint ipos)
{
    return ajSeqPosII(ajSeqLen(thys), 1, ipos);
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
** @param [r] thys [const AjPSeq] Target sequence.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqPosI(const AjPSeq thys, ajint imin, ajint ipos)
{
    return ajSeqPosII(ajSeqLen(thys), imin, ipos);
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

ajint ajSeqPosII(ajint ilen, ajint imin, ajint ipos)
{
    ajint jpos;

    if(ipos < 0)
	jpos = ilen + ipos + 1;
    else
    {
	if(ipos)
	    jpos = ipos;
	else
	    jpos = 1;
    }

    if(jpos > ilen)
	jpos = ilen;

    if(jpos < imin)
	jpos = imin;

    ajDebug("ajSeqPosII ilen: %d imin: %d ipos: %d) = %d\n",
	    ilen, imin, ipos, jpos);

    return jpos;
}




/* @func ajSeqTrim ************************************************************
**
** Trim a sequence using the Begin and Ends.
** Also reverse complements a nucleotide sequence if required.
**
** @param [u] thys [AjPSeq] Sequence to be trimmed.
** @return [AjBool] AjTrue returned if successful.
** @@
******************************************************************************/

AjBool ajSeqTrim(AjPSeq thys)
{
    AjBool okay = ajTrue;
    ajint begin;
    ajint end;

    ajDebug("Trimming %d from %d to %d Rev: %B Reversed: %B\n",
	    thys->Seq->Len,thys->Begin,thys->End, thys->Rev, thys->Reversed);

    if(thys->Rev)
	ajSeqReverse(thys);

    begin = ajSeqPos(thys, thys->Begin);
    end   = ajSeqPos(thys, thys->End);

    ajDebug("Trimming %d from %d (%d) to %d (%d) Rev: %B Reversed: %B\n",
	    thys->Seq->Len,thys->Begin,begin, thys->End, end,
	    thys->Rev, thys->Reversed);

    if(thys->End)
    {
	if(end < begin)
	    return ajFalse;
	okay = ajStrTrim(&(thys->Seq),(0 - (thys->Seq->Len-(end)) ));
	thys->Offend = thys->Seq->Len-(end);
	thys->End    = 0;
    }

    if(thys->Begin)
    {
	okay = ajStrTrim(&thys->Seq,begin-1);
	thys->Offset = begin-1;
	thys->Begin =0;
    }

    ajDebug("After Trimming len = %d\n",thys->Seq->Len);
    /*ajDebug("After Trimming len = %d '%S'\n",thys->Seq->Len, thys->Seq);*/


    if(okay && thys->Fttable)
	okay = ajFeattableTrimOff(thys->Fttable, thys->Offset, thys->Seq->Len);

    return okay;
}




/* @func ajSeqGapCount ********************************************************
**
** Returns the number of gaps in a sequence (counting any possible
** gap character
**
** @param [r] thys [const AjPSeq] Sequence object
** @return [ajint] Number of gaps
******************************************************************************/

ajint ajSeqGapCount(const AjPSeq thys)
{
    return ajSeqGapCountS(thys->Seq);
}




/* @func ajSeqGapCountS *******************************************************
**
** returns the number of gaps in a string (counting any possible
** gap character
**
** @param [r] str [const AjPStr] String object
** @return [ajint] Number of gaps
******************************************************************************/

ajint ajSeqGapCountS(const AjPStr str)
{

    ajint ret = 0;

    static char testchars[] = "-~.?"; /* all known gap characters */
    char *testgap;

    ajDebug("ajSeqGapCountS '%S'\n", str);

    testgap = testchars;

    while(*testgap)
    {
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

void ajSeqGapStandard(AjPSeq thys, char gapch)
{
    char newgap = '-';
    static char testchars[] = "-~.?"; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    if(gapch)
	newgap = gapch;

    /*ajDebug("ajSeqGapStandard '%c'=>'%c' '%S'\n",
            gapch, newgap, thys->Seq);*/

    while(*testgap)
    {
	if(newgap != *testgap)
	{
	    ajStrSubstituteKK(&thys->Seq, *testgap, newgap);
	    /*ajDebug(" ajSeqGapStandard replaced         '%c'=>'%c' '%S'\n",
		    *testgap, newgap, thys->Seq);*/
	}
	testgap++;
    }

    return;
}




/* @func ajSeqFill ************************************************************
**
** Fills a single sequence with gaps up to a specified length.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] len [ajint] Length to pad fill to.
** @return [ajint] Number of gaps inserted
** @@
******************************************************************************/

ajint ajSeqFill(AjPSeq seq, ajint len)
{
    ajint ilen = 0;

    ajDebug("ajSeqFill(len: %d -> ilen:%d)\n", ajSeqLen(seq), ilen);

    if(ajSeqLen(seq) < len)
    {
	ilen = len - ajSeqLen(seq);
	ajStrFill(&seq->Seq, len, '-');
    }

    ajDebug("      result: (len: %d added: %d\n",
	     ajSeqLen(seq), ilen);

    return ilen;
}

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

static AjPStr seqVersionAccnum = NULL;

static AjPStr seqTempUsa = NULL;


static void       seqCrcGen( void );
static void seqMakeUsa(const AjPSeq thys, AjPStr* usa);


/* @filesection ajseq ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
******************************************************************************/



/* @datasection [AjPSeq] Sequence ********************************************
**
** Function is for manipulating sequence objects
**
** @nam2rule Seq
**
******************************************************************************/


/* @section constructors *********************************************
**
** All constructors return a new sequence by pointer. It is the
** responsibility of the user to first destroy any previous
** sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [AjPSeq]
** @fcategory new
**
** @nam3rule New Constructor
** @nam4rule NewName Constructor with new name
** @nam4rule NewRange Constructor with range and direction
** @nam4rule NewSeq Constructor with all details in an existing sequence
**
** @suffix Res [size_t] Reserved length
** @suffix S [const AjPSeq] Source sequence
** @suffix C [const char*] Source sequence
**
** @argrule Res size [size_t] Reserved size including the terminating NULL
** @argrule C txt [const char*] Source sequence
** @argrule S str [const AjPStr] Source sequence
** @argrule NewSeq seq [const AjPSeq] Source sequence to be copied
** @argrule NameC name [const char*] Sequence name
** @argrule NameS name [const AjPStr] Sequence name
** @argrule Range offset [ajint] Offset at start
** @argrule Range offend [ajint] Offset at end
** @argrule Range rev [AjBool] True if sequence is to be reversed
**
** @valrule * [AjPSeq]
**
******************************************************************************/




/* @func ajSeqNew *************************************************************
**
** Creates and initialises a sequence object.
**
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNew(void)
{
    return ajSeqNewL(0);
}




/* @func ajSeqNewNameC ********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a char*
**
** @param [r] txt[const char*] Sequence string
** @param [r] name [const char*] Sequence name
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewNameC(const char* txt, const char* name)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajStrAssignC(&pthis->Name, name);
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

    ajStrAssignC(&pthis->Seq, txt);

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;
    pthis->Trimmed  = ajFalse;

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

    return pthis;
}


/* @func ajSeqNewNameS ********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string
**
** @param [r] str [const AjPStr] Sequence string
** @param [r] name [const AjPStr] Sequence name
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewNameS(const AjPStr str, const AjPStr name)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    pthis->Name = ajStrNewS(name);
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

    ajStrAssignS(&pthis->Seq, str);

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;
    pthis->Trimmed  = ajFalse;

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

    return pthis;
}





/* @obsolete ajSeqNewC
** @rename ajSeqNewNameC
*/

AjPSeq __deprecated ajSeqNewC(const char* seq, const char* name)
{
    return ajSeqNewNameC(seq, name);
}


/* @obsolete ajSeqNewStr
** @rename ajSeqNewNameS
*/

AjPSeq __deprecated ajSeqNewStr(const AjPStr seq)
{
    AjPStr name = ajStrNew();
    return ajSeqNewNameS(seq, name);
}


/* @func ajSeqNewRangeC *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string,and provides offsets, and direction.
**
** The sequence is set to be already trimmed and if necessary reversed.
**
** Start and end positions are 0 (full sequence), as it is trimmed.
** Any start and end are represented by the offsets.
**
** @param [r] txt [const char*] Sequence string
** @param [r] offset [ajint] Offset at start
** @param [r] offend [ajint] Offset at end
** @param [r] rev [AjBool] Reversed if true (reverses offsets)
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewRangeC(const char* txt,
		       ajint offset, ajint offend, AjBool rev)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajDebug("ajSeqNewRangeC %d %d %B\n",
	    offset, offend, rev);
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

    ajStrAssignC(&pthis->Seq, txt);

    pthis->Rev      = ajFalse;
    pthis->Reversed = rev;		/* we are setting everything here */
    pthis->Trimmed  = ajTrue;		/* we are setting everything here */

    pthis->EType  = 0;
    pthis->Format = 0;
    pthis->Begin  = 0;
    pthis->End    = 0;
    if (rev)
    {
	pthis->Offset = offend;
	pthis->Offend = offset;
    }
    else
    {
	pthis->Offset = offset;
	pthis->Offend = offend;
    }
    pthis->Weight = 1.0;

    pthis->Acclist = ajListstrNew();
    pthis->Keylist = ajListstrNew();
    pthis->Taxlist = ajListstrNew();

    ajDebug("ajSeqNewRangeC rev:%B offset:%d/%d offend:%d/%d\n",
	    rev, offset, pthis->Offset, offend, pthis->Offend);
    
    return pthis;
}




/* @func ajSeqNewRangeS *******************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence as a string,and provides offsets, and direction.
**
** The sequence is set to be already trimmed and if necessary reversed.
**
** Start and end positions are 0 (full sequence), as it is trimmed.
** Any start and end are represented by the offsets.
**
** @param [r] str [const AjPStr] Sequence string
** @param [r] offset [ajint] Offset at start
** @param [r] offend [ajint] Offset at end
** @param [r] rev [AjBool] Reversed if true (reverses offsets)
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewRangeS(const AjPStr str,
			ajint offset, ajint offend, AjBool rev)
{
    return ajSeqNewRangeC(ajStrGetPtr(str), offset, offend, rev);
}




/* @obsolete ajSeqNewRange
** @rename ajSeqNewRangeS
*/

AjPSeq __deprecated ajSeqNewRange(const AjPStr seq,
				  ajint offset, ajint offend, AjBool rev)
{
    return ajSeqNewRangeS(seq, offset, offend, rev);
}

/* @obsolete ajSeqNewRangeCI
** @replace ajSeqNewRangeC (1,2,3,4,5/1,3,4,5)
*/

AjPSeq __deprecated ajSeqNewRangeCI(const char* seq, ajint len,
				    ajint offset, ajint offend, AjBool rev)
{
    return ajSeqNewRangeC(seq, offset, offend, rev);
}

/* @func ajSeqNewRes **********************************************************
**
** Creates and initialises a sequence object with a specified sequence length.
**
** @param [r] size [size_t] Reserved space for the sequence, including
**                          a trailing null character.
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewRes(size_t size)
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
	pthis->Seq = ajStrNewRes(size);
    else
	pthis->Seq = ajStrNew();

    pthis->Rev      = ajFalse;
    pthis->Reversed = ajFalse;
    pthis->Trimmed = ajFalse;

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

    pthis->Garbage = ajFalse;

    return pthis;
}

/* @obsolete ajSeqNewL
** @rename ajSeqNewRes
*/
AjPSeq __deprecated ajSeqNewL(size_t size)
{
    return ajSeqNewRes(size);
}





/* @func ajSeqNewSeq **********************************************************
**
** Creates and initialises a sequence object with a specified existing
** sequence.
**
** @param [r] seq [const AjPSeq] Old sequence object
** @return [AjPSeq] New sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqNewSeq(const AjPSeq seq)
{
    AjPSeq pthis;

    AJNEW0(pthis);

    ajStrAssignS(&pthis->Name, seq->Name);
    ajStrAssignS(&pthis->Acc, seq->Acc);
    ajStrAssignS(&pthis->Sv, seq->Sv);
    ajStrAssignS(&pthis->Gi, seq->Gi);
    ajStrAssignS(&pthis->Tax, seq->Tax);
    ajStrAssignS(&pthis->Type, seq->Type);

    pthis->EType  = seq->EType;

    ajStrAssignS(&pthis->Db, seq->Db);
    ajStrAssignS(&pthis->Setdb, seq->Setdb);
    ajStrAssignS(&pthis->Full, seq->Full);
    ajStrAssignS(&pthis->Date, seq->Date);
    ajStrAssignS(&pthis->Desc, seq->Desc);
    ajStrAssignS(&pthis->Doc, seq->Doc);

    pthis->Rev      = seq->Rev;
    pthis->Reversed = seq->Reversed;
    pthis->Trimmed  = seq->Trimmed;
    pthis->Garbage  = seq->Garbage;

    pthis->Begin  = seq->Begin;
    pthis->End    = seq->End;
    pthis->Offset = seq->Offset;
    pthis->Offend = seq->Offend;
    pthis->Weight = seq->Weight;
    pthis->Fpos   = seq->Fpos;

    ajStrAssignS(&pthis->Usa, seq->Usa);
    ajStrAssignS(&pthis->Ufo, seq->Ufo);
    ajStrAssignS(&pthis->Formatstr, seq->Formatstr);
    pthis->Format = seq->Format;

    ajStrAssignS(&pthis->Filename, seq->Filename);
    ajStrAssignS(&pthis->Entryname, seq->Entryname);

    if(seq->TextPtr)
	ajStrAssignS(&pthis->TextPtr, seq->TextPtr);


    pthis->Acclist = ajListstrNew();
    ajListstrClone(seq->Acclist, pthis->Acclist);

    pthis->Keylist = ajListstrNew();
    ajListstrClone(seq->Keylist, pthis->Keylist);

    pthis->Taxlist = ajListstrNew();
    ajListstrClone(seq->Taxlist, pthis->Taxlist);

    ajStrAssignS(&pthis->Seq, seq->Seq);
    if (seq->Fttable)
	pthis->Fttable = ajFeattableCopy(seq->Fttable);

    return pthis;
}


/* @obsolete ajSeqNewS
** @rename ajSeqNewSeq
*/

AjPSeq __deprecated ajSeqNewS(const AjPSeq seq)
{
    return ajSeqNewSeq(seq);
}




/* @section modifiers ************************************************
**
** These functions update contents of a sequence object.
**
** @fdata [AjPSeq]
** @fcategory modify
**
** @nam3rule Assign Assign one attribute of a sequence
** @nam4rule AssignAcc Assign accession number
** @nam4rule AssignDesc Assign description text
** @nam4rule AssignEntry Assign entry name
** @nam4rule AssignFile Assign file name
** @nam4rule AssignFull Assign full name
** @nam4rule AssignGi Assign GI number
** @nam4rule AssignName Assign sequence name
** @nam4rule AssignSeq Assign sequence
** @nam4rule AssignSv Assign sequence version number
** @nam4rule AssignUfo Assign feature address
** @nam4rule AssignUsa Assign sequence address
** @nam3rule Set Set sequence properties
** @nam4rule SetOffsets Set sequence offsets as a subsequence of an original
** @nam4rule SetRange Set start and end position within sequence
** @nam5rule SetRangeRev Set start and end position and reverse direction
**                       of a sequence
**
** @suffix Len [ajint] Length of character string
** @suffix C [const char*] Character string
** @suffix S [const AjPStr] String
** 
** @argrule * seq [AjPSeq] Sequence object
** @argrule C txt [const char*] Character string to assign
** @argrule Len len [ajint] Character string length
** @argrule S str [const AjPStr] String to assign
** @argrule Offsets offset [ajint] Offset at start
** @argrule Offsets origlen [ajint] Length of original sequence
** @argrule Range pos1 [ajint] Start position
** @argrule Range pos2 [ajint] End  position
**
** @valrule * [void]
**
******************************************************************************/




/* @func ajSeqAssignAccC ******************************************************
**
** Assigns the sequence accession number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Accession number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignAccC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Acc, txt);

    return;
}




/* @func ajSeqAssignAccS ******************************************************
**
** Assigns the sequence accession number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Accession number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignAccS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Acc, str);

    return;
}


/* @obsolete ajSeqAssAccC
** @rename ajSeqAssignAccC
*/

void __deprecated ajSeqAssAccC(AjPSeq thys, const char* text)
{
    ajSeqAssignAccC(thys, text);
    return;
}

/* @obsolete ajSeqAssAcc
** @rename ajSeqAssignAccS
*/

void __deprecated ajSeqAssAcc(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignAccS(thys, str);
    return;
}

/* @func ajSeqAssignDescC *****************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New description as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignDescC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Desc, txt);

    return;
}




/* @func ajSeqAssignDescS *****************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] New description as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignDescS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Desc, str);

    return;
}


/* @obsolete ajSeqAssDescC
** @rename ajSeqAssignDescC
*/

void __deprecated ajSeqAssDescC(AjPSeq thys, const char* txt)
{
    ajSeqAssignDescC(thys, txt);
    return;
}


/* @obsolete ajSeqAssDesc
** @rename ajSeqAssignDescS
*/

void __deprecated ajSeqAssDesc(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignDescS(thys, str);
    return;
}


/* @func ajSeqAssignEntryC ****************************************************
**
** Assigns the sequence entryname.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Entry name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignEntryC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Entryname, txt);

    return;
}




/* @func ajSeqAssignEntryS ****************************************************
**
** Assigns the sequence entryname.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Entry name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignEntryS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Entryname, str);

    return;
}




/* @obsolete ajSeqAssEntryC
** @rename ajSeqAssignEntryC
*/

void __deprecated ajSeqAssEntryC(AjPSeq thys, const char* text)
{
    ajSeqAssignEntryC(thys, text);
    return;
}


/* @obsolete ajSeqAssEntry
** @rename ajSeqAssignEntryS
*/

void __deprecated ajSeqAssEntry(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignEntryS(thys, str);
    return;
}

/* @func ajSeqAssignFileC *****************************************************
**
** Assigns the sequence filename.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] File name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignFileC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Filename, txt);

    return;
}




/* @func ajSeqAssignFileS *****************************************************
**
** Assigns the sequence file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] File name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignFileS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Filename, str);

    return;
}


/* @obsolete ajSeqAssFileC
** @rename ajSeqAssignFileC
*/

void __deprecated ajSeqAssFileC(AjPSeq thys, const char* text)
{
    ajSeqAssignFileC(thys, text);
    return;
}

/* @obsolete ajSeqAssFile
** @rename ajSeqAssignFileS
*/

void __deprecated ajSeqAssFile(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignFileS(thys, str);
    return;
}



/* @func ajSeqAssignFullC *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Full name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignFullC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Full, txt);

    return;
}




/* @func ajSeqAssignFullS *****************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Full name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignFullS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Full, str);

    return;
}



/* @obsolete ajSeqAssFullC
** @rename ajSeqAssignFullC
*/

void __deprecated ajSeqAssFullC(AjPSeq thys, const char* text)
{
    ajSeqAssignFullC(thys, text);
    return;
}

/* @obsolete ajSeqAssFull
** @rename ajSeqAssignFullS
*/

void __deprecated ajSeqAssFull(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignFullS(thys, str);
    return;
}




/* @func ajSeqAssignGiC *******************************************************
**
** Assigns the GI version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] GI number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignGiC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Gi, txt);

    return;
}




/* @func ajSeqAssignGiS *******************************************************
**
** Assigns the GI version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] GI number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignGiS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Gi, str);

    return;
}




/* @obsolete ajSeqAssGiC
** @rename ajSeqAssignGiC
*/

void __deprecated ajSeqAssGiC(AjPSeq thys, const char* text)
{
    ajSeqAssignGiC(thys, text);
    return;
}

/* @obsolete ajSeqAssGi
** @rename ajSeqAssignGiS
*/

void __deprecated ajSeqAssGi(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignGiS(thys, str);
    return;
}




/* @func ajSeqAssignNameC *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] Name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignNameC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Name, txt);

    return;
}


/* @func ajSeqAssignNameS *****************************************************
**
** Assigns the sequence name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] Name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignNameS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Name, str);

    return;
}


/* @obsolete ajSeqAssName
** @rename ajSeqAssignNameS
*/

void __deprecated ajSeqAssName(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignNameS(thys, str);

    return;
}


/* @obsolete ajSeqAssNameC
** @rename ajSeqAssignNameS
*/

void __deprecated ajSeqAssNameC(AjPSeq thys, const char* str)
{
    ajSeqAssignNameC(thys, str);

    return;
}


/* @func ajSeqAssignSeqC ******************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New sequence as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignSeqC(AjPSeq seq, const char* txt)
{
    ajint i = strlen(txt);
    ajSeqAssignSeqLenC(seq, txt, i);

    return;
}




/* @func ajSeqAssignSeqLenC ***************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] New sequence as a C character string.
** @param [r] len [ajint] Numbur of characters to use
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignSeqLenC(AjPSeq seq, const char* txt, ajint len)
{
    ajStrAssignLenC(&seq->Seq, txt, len);

    seq->Begin  = 0;
    seq->End    = 0;
    seq->Offset = 0;
    seq->Offend = 0;
    seq->Rev      = ajFalse;
    seq->Reversed = ajFalse;
    seq->Trimmed  = ajFalse;

    return;
}




/* @func ajSeqAssignSeqS ******************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] New sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignSeqS(AjPSeq seq, const AjPStr str)
{
    ajSeqAssignSeqLenC(seq, ajStrGetPtr(str), ajStrGetLen(str));

    return;
}


/* @obsolete ajSeqAssSeq
** @rename ajSeqAssignSeqS
*/
void __deprecated ajSeqAssSeq(AjPSeq seq, const AjPStr str)
{
    ajSeqAssignSeqS(seq, str);
    return;
}


/* @obsolete ajSeqAssSeqC
** @rename ajSeqAssignSeqC
*/
void __deprecated ajSeqAssSeqC (AjPSeq thys, const char* text)
{
    ajSeqAssignSeqC(thys, text);
    return;
}

/* @obsolete ajSeqAssSeqCI
** @replace ajSeqAssignSeqC (1,2,3/1,2)
*/
void __deprecated ajSeqAssSeqCI (AjPSeq thys, const char* text, ajint ilen)
{
    ajSeqAssignSeqC(thys, text);
    return;
}




/* @func ajSeqAssignSvC *******************************************************
**
** Assigns the sequence version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] SeqVersion number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignSvC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Sv, txt);

    return;
}




/* @func ajSeqAssignSvS *******************************************************
**
** Assigns the sequence version number.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] SeqVersion number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignSvS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Sv, str);

    return;
}




/* @obsolete ajSeqAssSvC
** @rename ajSeqAssignSvC
*/
void __deprecated ajSeqAssSvC(AjPSeq thys, const char* text)
{
    ajSeqAssignSvC(thys, text);
    return;
}

/* @obsolete ajSeqAssSv
** @rename ajSeqAssignSvS
*/
void __deprecated ajSeqAssSv(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignSvS(thys, str);
    return;
}


/* @func ajSeqAssignUfoC ******************************************************
**
** Assigns the sequence feature file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] UFO as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignUfoC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Ufo, txt);

    return;
}




/* @func ajSeqAssignUfoS ******************************************************
**
** Assigns the sequence feature file name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] UFO as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignUfoS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Ufo, str);

    return;
}



/* @obsolete ajSeqAssUfoC
** @rename ajSeqAssignUfoC
*/
void __deprecated ajSeqAssUfoC(AjPSeq thys, const char* text)
{
    ajSeqAssignUfoC(thys, text);
    return;
}

/* @obsolete ajSeqAssUfo
** @rename ajSeqAssignUfoS
*/
void __deprecated ajSeqAssUfo(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignUfoS(thys, str);
    return;
}


/* @func ajSeqAssignUsaC ******************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] txt [const char*] USA as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignUsaC(AjPSeq seq, const char* txt)
{
    ajStrAssignC(&seq->Usa, txt);

    return;
}


/* @func ajSeqAssignUsaS ******************************************************
**
** Assigns the sequence full name.
**
** @param [u] seq [AjPSeq] Sequence object.
** @param [r] str [const AjPStr] USA as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssignUsaS(AjPSeq seq, const AjPStr str)
{
    ajStrAssignS(&seq->Usa, str);

    return;
}




/* @obsolete ajSeqAssUsaC
** @rename ajSeqAssignUsaC
*/
void __deprecated ajSeqAssUsaC(AjPSeq thys, const char* text)
{
    ajSeqAssignUsaC(thys, text);
    return;
}

/* @obsolete ajSeqAssUsa
** @rename ajSeqAssignUsaS
*/
void __deprecated ajSeqAssUsa(AjPSeq thys, const AjPStr str)
{
    ajSeqAssignUsaS(thys, str);
    return;
}




/* @func ajSeqSetOffsets ******************************************************
**
** Sets the offsets for each end of a subsequence.
**
** Needed mainly for local alignments so the original sequence numbering
** can be preserved.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] offset [ajint] Offset from start of original sequence
** @param [r] origlen [ajint] Original length, used to calculate the offset
**                             from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqSetOffsets(AjPSeq seq, ajint offset, ajint origlen)
{
    ajDebug("ajSeqSetOffsets(len:%d gap:%d off:%d origlen:%d) "
	    "Offset:%d Offend:%d\n",
	    ajSeqGetLen(seq), ajSeqGapCount(seq),
	    offset, origlen, seq->Offset, seq->Offend);

    if(seq->Trimmed)
    {
	ajWarn("Sequence '%s' already trimmed in ajSeqSetOffsets",
	       ajSeqName(seq));
    }

    if(seq->Reversed)
    {
	if(offset && !seq->Offend)
	    seq->Offend = offset;

	if(origlen && !seq->Offset)
	{
	    seq->Offset = origlen - offset - ajSeqGetLen(seq) + ajSeqGapCount(seq);
	    if (seq->Offend < 0)
		seq->Offend = 0;
	}
    }
    else
    {
	if(offset && !seq->Offset)
	    seq->Offset = offset;

	if(origlen && !seq->Offend)
	{
	    seq->Offend = origlen - offset - ajSeqGetLen(seq) + ajSeqGapCount(seq);
	    if (seq->Offend < 0)
		seq->Offend = 0;
	}
    }

    ajDebug("      result: (len: %d truelen:%d Offset:%d Offend:%d)\n",
	    ajSeqGetLen(seq), ajSeqGetLen(seq)-ajSeqGapCount(seq),
	    seq->Offset, seq->Offend);

    return;
}




/* @func ajSeqSetRange ********************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set).
** Trim the sequence to convert to the subsequence.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqSetRange(AjPSeq seq, ajint pos1, ajint pos2)
{
    ajDebug("ajSeqSetRange (len: %d %d..%d old %d..%d) rev:%B reversed:%B\n",
	    ajSeqGetLen(seq), pos1, pos2,
	    seq->Begin, seq->End, seq->Rev, seq->Reversed);

    if(seq->Trimmed)
    {
	ajWarn("Sequence '%s' already trimmed in ajSeqSetRange",
	       ajSeqName(seq));
    }

    if(pos1 && !seq->Begin)
	seq->Begin = pos1;

    if(pos2 && !seq->End)
	seq->End = pos2;

    ajDebug("      result: (len: %d %d..%d)\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    if(seq->Rev && !seq->Reversed)
	ajSeqReverseDo(seq);

    return;
}




/* @func ajSeqSetRangeRev *****************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set),
** and set the sequence to be reversed.
**
** @param [u] seq [AjPSeq] Sequence object to be set.
** @param [r] pos1 [ajint] Start position. Negative values are from the end.
** @param [r] pos2 [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqSetRangeRev(AjPSeq seq, ajint pos1, ajint pos2)
{
    
    ajDebug("ajSeqSetRange (len: %d %d..%d old %d..%d) rev:%B reversed:%B\n",
	    ajSeqGetLen(seq), pos1, pos2,
	    seq->Begin, seq->End, seq->Rev, seq->Reversed);

    if(seq->Trimmed)
    {
	ajWarn("Sequence '%s' already trimmed in ajSeqSetRange",
	       ajSeqName(seq));
    }

    if(pos1 && !seq->Begin)
	seq->Begin = pos1;

    if(pos2 && !seq->End)
	seq->End = pos2;

    ajDebug("      result: (len: %d %d..%d)\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    if(!seq->Rev)
    {
	seq->Rev = ajTrue;
	seq->Reversed = ajFalse;
    }

    ajSeqReverseDo(seq);

    return;
}

/* @obsolete ajSeqSetRangeDir
** @replace ajSeqSetRange (1,2,3,ajFalse/1,2,3)
** @replace ajSeqSetRangeRev (1,2,3,ajTrue/1,2,3)
*/
void __deprecated ajSeqSetRangeDir(AjPSeq seq,
				   ajint ibegin, ajint iend, AjBool rev)
{
    ajSeqSetRangeRev(seq, ibegin, iend);
    return;
}



/* @obsolete ajSeqReplace
** @rename ajSeqAssignSeqS
*/
void __deprecated ajSeqReplace(AjPSeq thys, const AjPStr seq)
{
    ajSeqAssignSeqS(thys, seq);
    return;
}


/* @obsolete ajSeqReplaceC
** @rename ajSeqAssignSeqC
*/
void __deprecated ajSeqReplaceC(AjPSeq thys, const char* seq)
{
    ajSeqAssignSeqC(thys, seq);
    return;
}



/* @obsolete ajSeqMakeUsa
** @remove made static
*/

void __deprecated ajSeqMakeUsa(AjPSeq thys, const AjPSeqin seqin)
{
    seqMakeUsa(thys, &thys->Usa);
    return;
}

/* @obsolete ajSeqMakeUsa
** @remove made static
*/

void __deprecated ajSeqMakeUsaS(const AjPSeq thys,
				const AjPSeqin seqin, AjPStr* usa)
{
    seqMakeUsa(thys, usa);
    return;
}

/* @funcstatic seqMakeUsa ****************************************************
**
** Sets the USA for a sequence.
**
** @param [r] thys [const AjPSeq] Sequence object
** @param [w] usa [AjPStr*] USA in full
** @return [void]
** @@
******************************************************************************/

static void seqMakeUsa(const AjPSeq thys, AjPStr* usa)
{
    AjPStr tmpstr = NULL;

    ajDebug("ajSeqMakeUsa (Name <%S> Formatstr <%S> Db <%S> "
	    "Entryname <%S> Filename <%S>)\n",
	    thys->Name, thys->Formatstr, thys->Db,
	    thys->Entryname, thys->Filename);

    /* ajSeqTrace(thys); */

    if(ajStrGetLen(thys->Db))
	ajFmtPrintS(usa, "%S-id:%S", thys->Db, thys->Entryname);
    else
    {
	/*ajFmtPrintS(&thys->Usa, "%S::%S (%S)",
	  thys->Formatstr, thys->Filename, thys->Entryname);*/
	if(ajStrGetLen(thys->Entryname))
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

    if(ajStrGetLen(tmpstr) > 3)
	ajStrAppendS(usa, tmpstr);

    ajStrDel(&tmpstr);
    ajDebug("      result: <%S>\n",
	    *usa);

    return;
}



/* @section process *******************************************************
**
** These functions use the contents of a sequence object to produce a
** subsequence from the range, or to reverse a sequence whose direction
** has been set to be reversed.
**
** @fdata [AjPSeq]
** @fcategory modify
**
** @nam3rule Complement Complement the bases
** @nam4rule ComplementOnly Complement the bases but do not
**                          reverse the sequence
** @nam3rule Fmt Format sequence characters
** @nam4rule FmtLower Format sequence characters to lower case
** @nam4rule FmtUpper Format sequence characters to upper case
** @nam3rule Reverse Reverse the sequence
** @nam4rule ReverseDo Reverse if Rev attribute is set
** @nam4rule ReverseForce Reverse the sequence without testing the
**                        Rev attribute
** @nam4rule ReverseOnly Reverse the sequence but do not complement the bases
**
** @argrule * seq [AjPSeq] Sequence to be processed
**
** @valrule * [void]
******************************************************************************/


/* @func ajSeqComplementOnly **************************************************
**
** Complements but does not reverse a nucleotide sequence.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqComplementOnly(AjPSeq seq)
{
    ajSeqstrComplementOnly(&seq->Seq);

    return;
}

/* @obsolete ajSeqCompOnly
** @rename ajSeqComplementOnly
*/
void __deprecated ajSeqCompOnly(AjPSeq seq)
{
    ajSeqComplementOnly(seq);

    return;
}




/* @func ajSeqFmtLower ********************************************************
**
** Converts a sequence to lower case.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqFmtLower(AjPSeq seq)
{
    ajStrFmtLower(&seq->Seq);

    return;
}


/* @obsolete ajSeqToLower
** @rename ajSeqFmtLower
*/

void __deprecated ajSeqToLower(AjPSeq seq)
{
    ajStrFmtLower(&seq->Seq);

    return;
}




/* @func ajSeqFmtUpper ********************************************************
**
** Converts a sequence to upper case.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqFmtUpper(AjPSeq seq)
{
    ajStrFmtUpper(&seq->Seq);

    return;
}



/* @obsolete ajSeqToUpper
** @rename ajSeqFmtUpper
*/

void __deprecated ajSeqToUpper(AjPSeq seq)
{
    ajStrFmtUpper(&seq->Seq);

    return;
}


/* @func ajSeqReverseDo *******************************************************
**
** Reverses and complements a nucleotide sequence, unless it is already done.
**
** If the sequence is not flagged for reversal, use ajSeqReverseForce instead.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReverseDo(AjPSeq seq)
{
    ajint ibegin;
    ajint iend;
    ajint itemp;

    ajDebug("ajSeqReverse len: %d Begin: %d End: %d Rev: %B Reversed: %B\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End,
	    seq->Rev, seq->Reversed);

    if(!seq->Rev)			/* Not flagged for reversal */
	return;

    ibegin = seq->Begin;
    iend   = seq->End;

    seq->End   = -(ibegin);
    seq->Begin = -(iend);

    itemp = seq->Offend;
    seq->Offend = seq->Offset;
    seq->Offset = itemp;

    seq->Rev = ajFalse;

    if(seq->Reversed)
	seq->Reversed = ajFalse;
    else
	seq->Reversed = ajTrue;

    ajSeqstrReverse(&seq->Seq);

    ajDebug("      result len: %d Begin: %d End: %d\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    if(seq->Fttable)
	ajFeattableReverse(seq->Fttable);

    return;
}


/* @obsolete ajSeqReverse
** @rename ajSeqReverseDo
*/
AjBool __deprecated ajSeqReverse(AjPSeq seq)
{
    if(!seq->Rev)
	return ajFalse;
    ajSeqReverseDo(seq);
    return ajTrue;
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
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReverseForce(AjPSeq seq)
{
    ajDebug("ajSeqReverseForce len: %d Begin: %d End: %d Rev: %B "
	    "Reversed: %B\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End,
	    seq->Rev, seq->Reversed);

    seq->Rev = ajTrue;
    ajSeqReverseDo(seq);

    return;
}




/* @func ajSeqReverseOnly *****************************************************
**
** Reverses but does not complement a nucleotide sequence.
**
** Intended for sequence display, but can be used to fix incorrect
** sequence entries.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReverseOnly(AjPSeq seq)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqRevOnly len: %d Begin: %d End: %d\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    ibegin = seq->Begin;
    iend   = seq->End;

    if(ibegin)
	seq->End   = -(ibegin);
    if(iend)
	seq->Begin = -(iend);

    ajStrReverse(&seq->Seq);

    ajDebug(" only result len: %d Begin: %d End: %d\n",
	    ajSeqGetLen(seq), seq->Begin, seq->End);

    return;
}

/* @obsolete ajSeqRevOnly
** @rename ajSeqReverseOnly
*/

void __deprecated ajSeqRevOnly(AjPSeq seq)
{
    ajSeqReverseOnly(seq);
    return;
}




/* @section element retrieval
**
** These functions use the contents of a sequence object to produce a
** subsequence from the range, or to reverse a sequence whose direction
** has been set to be reversed.
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Get Return sequence attribute(s)
** @nam4rule GetAcc Return sequence accession number
** @nam4rule GetBegin Return sequence begin
** @nam4rule GetDesc Return sequence description
** @nam4rule GetEnd Return sequence end
** @nam4rule GetEntry Return sequence ID
** @nam4rule GetFeat Return sequence feature table
** @nam4rule GetGi Return sequence GI number
** @nam4rule GetLen Return sequence length
** @nam4rule GetName Return sequence name
** @nam4rule GetOffend Return sequence end offset
** @nam4rule GetOffset Return sequence start offset
** @nam4rule GetRange Return sequence begin and end
** @nam4rule GetRev Return sequence reverse attribute (not yet reversed)
** @nam4rule GetSeq Return sequence
** @nam4rule GetSv Return sequence version
** @nam4rule GetTax Return taxonomy
** @nam4rule GetUsa Return sequence USA
**
** @suffix S Return a string
** @suffix C Return a character string
** @suffix True True position in original sequence
** @suffix Copy Editable copy
**
** @argrule * seq [const AjPSeq] Sequence
** @argrule Range begin [ajint*] Start position
** @argrule Range end [ajint*] End position
**
** @valrule C [const char*]
** @valrule S [const AjPStr]
** @valrule Begin [ajint] Sequence position
** @valrule End [ajint] Sequence position
** @valrule Len [ajint] Sequence length
** @valrule Offend [ajint] Sequence end offset
** @valrule Offset [ajint] Sequence start offset
** @valrule Range [ajint] Sequence length
** @valrule Rev [AjBool] Reverse attribute
** @valrule *Feat [const AjPFeattable] Link to internal feature table
** @valrule *FeatCopy [AjPFeattable] New feature table with original contents
** @valrule *SeqCopyC [char*] New sequence with original contents
** @valrule *SeqCopyS [AjPStr] New sequence with original contents
******************************************************************************/

/* @func ajSeqGetAccC *********************************************************
**
** Returns the sequence accession number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Accession number as a character string.
** @@
******************************************************************************/

const char* ajSeqGetAccC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Acc);
}




/* @func ajSeqGetAccS *********************************************************
**
** Returns the sequence accession number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Accession number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetAccS(const AjPSeq seq)
{
    return seq->Acc;
}




/* @obsolete ajSeqGetAcc
** @rename ajSeqGetAccS
*/
const AjPStr __deprecated ajSeqGetAcc(const AjPSeq seq)
{
    return ajSeqGetAccS(seq);
}


/* @func ajSeqGetBegin ********************************************************
**
** Returns the sequence start position within the current stored sequence,
** or 1 if no start has been set.
**
** To return the position within the original sequence, which may be different
** if the sequence has been trimmed, use ajSeqTrueBegin
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqGetBegin(const AjPSeq seq)
{
    ajint i;
    ajint j;

    if(!seq->Begin)
	return 1;

    i = seq->Begin;

    if(seq->Begin > 0)
	i = seq->Begin - 1;

    j =  1 + ajMathPosI(ajSeqGetLen(seq), 0, i);

    return j;
}

/* @obsolete ajSeqBegin
** @rename ajSeqGetBegin
*/

ajint __deprecated ajSeqBegin(const AjPSeq seq)
{
    return ajSeqGetBegin(seq);
}


/* @func ajSeqGetBeginTrue ****************************************************
**
** Returns the sequence start position in the original sequence,
** which may have been trimmed.
**
** To return the position within the current stored sequence,
** which may be different if the sequence has been trimmed, use ajSeqBegin
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajSeqGetBeginTrue(const AjPSeq seq)
{
    if(!seq->Begin)
	return ajSeqCalcTruepos(seq, 1);

    return ajSeqCalcTruepos(seq, seq->Begin);
}


/* @obsolete ajSeqTrueBegin
** @rename ajSeqGetBeginTrue
*/
ajint __deprecated ajSeqTrueBegin(const AjPSeq seq)
{
    return ajSeqGetBeginTrue(seq);
}

/* @func ajSeqGetDescC ********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Description as a character string.
** @@
******************************************************************************/

const char* ajSeqGetDescC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Desc);
}




/* @func ajSeqGetDescS ********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Description as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetDescS(const AjPSeq seq)
{
    return seq->Desc;
}

/* @obsolete ajSeqGetDesc
** @rename ajSeqGetDescS
*/
const AjPStr __deprecated ajSeqGetDesc(const AjPSeq seq)
{
    return seq->Desc;
}




/* @func ajSeqGetEnd **********************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] End position.
** @@
******************************************************************************/

ajint ajSeqGetEnd(const AjPSeq seq)
{
    ajint i;

    if(!seq->End)
	return (ajSeqGetLen(seq));

    i = seq->End;
    if(seq->End > 0)
	i--;

    i = 1 + ajMathPosI(ajSeqGetLen(seq), ajSeqGetBegin(seq)-1, i);

    return i;
}



/* @obsolete ajSeqEnd
** @rename ajSeqGetEnd
*/
ajint __deprecated ajSeqEnd(const AjPSeq seq)
{
    return ajSeqGetEnd(seq);
}

/* @func ajSeqGetEndTrue ******************************************************
**
** Returns the sequence end position, or the sequence length if no end
** has been set.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] End position.
** @@
******************************************************************************/

ajint ajSeqGetEndTrue(const AjPSeq seq)
{
    if(!seq->End)
    {
	if(ajSeqIsReversed(seq))
	    return seq->Offend + ajSeqGetLen(seq);
	else
	    return seq->Offset + ajSeqGetLen(seq);
    }
    return ajSeqCalcTrueposMin(seq, ajSeqGetBeginTrue(seq), seq->End);
}



/* @obsolete ajSeqTrueEnd
** @rename ajSeqGetEndTrue
*/
ajint __deprecated ajSeqTrueEnd(const AjPSeq seq)
{
    return ajSeqGetEndTrue(seq);
}

/* @func ajSeqGetEntryC *******************************************************
**
** Returns the sequence full text entry.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Entry as a character string.
** @@
******************************************************************************/

const char* ajSeqGetEntryC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->TextPtr);
}

/* @func ajSeqGetEntryS *******************************************************
**
** Returns the sequence full text entry.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Entry as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetEntryS(const AjPSeq seq)
{
    return seq->TextPtr;
}

/* @obsolete ajSeqGetEntry
** @rename ajSeqGetEntryS
*/
const AjPStr __deprecated ajSeqGetEntry(const AjPSeq seq)
{
    return ajSeqGetEntryS(seq);
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
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPFeattable] feature table (if any)
** @@
******************************************************************************/

const AjPFeattable ajSeqGetFeat(const AjPSeq seq)
{
    return seq->Fttable;
}




/* @func ajSeqGetFeatCopy *****************************************************
**
** Returns a copy of the sequence feature table.
** Because this is a copy of all the data, the caller is responsible
** for deleting it after use.
**
** If the table is not to be changed or deleted then ajSeqGetFeat
** can return a copy of the internal pointer.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [AjPFeattable] feature table (if any)
** @@
******************************************************************************/

AjPFeattable ajSeqGetFeatCopy(const AjPSeq seq)
{
    return ajFeattableCopy(seq->Fttable);
}


/* @obsolete ajSeqCopyFeat
** @rename ajSeqGetFeatCopy
*/
AjPFeattable __deprecated ajSeqCopyFeat(const AjPSeq seq)
{
    return ajFeattableCopy(seq->Fttable);
}




/* @func ajSeqGetGiC **********************************************************
**
** Returns the GI version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] SeqVersion number as a character string.
** @@
******************************************************************************/

const char* ajSeqGetGiC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Gi);
}




/* @func ajSeqGetGiS **********************************************************
**
** Returns the GI version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] SeqVersion number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetGiS(const AjPSeq seq)
{
    return seq->Gi;
}

/* @obsolete ajSeqGetGi
** @rename ajSeqGetGiS
*/
const AjPStr __deprecated ajSeqGetGi(const AjPSeq seq)
{
    return seq->Gi;
}




/* @func ajSeqGetLen **********************************************************
**
** Returns the sequence length.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Sequence length.
** @@
******************************************************************************/

ajint ajSeqGetLen(const AjPSeq seq)
{
    return ajStrGetLen(seq->Seq);
}

/* @obsolete ajSeqLen
** @rename ajSeqGetLen
*/
ajint __deprecated ajSeqLen(const AjPSeq seq)
{
    return ajStrGetLen(seq->Seq);
}




/* @func ajSeqGetLenTrue ******************************************************
**
** Returns the length of the original sequence, including any gap characters.
**
** @param [r] seq [const AjPSeq] Target sequence.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqGetLenTrue(const AjPSeq seq)
{
    return (ajStrGetLen(seq->Seq) + seq->Offset + seq->Offend);
}


/* @obsolete ajSeqTrueLen
** @rename ajSeqGetLenTrue
*/
ajint __deprecated ajSeqTrueLen(const AjPSeq seq)
{
    return ajSeqGetLenTrue(seq);
}



/* @func ajSeqGetNameC ********************************************************
**
** Returns the sequence name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Name as a character string.
** @@
******************************************************************************/

const char* ajSeqGetNameC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Name);
}



/* @func ajSeqGetNameS ********************************************************
**
** Returns the sequence name.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetNameS(const AjPSeq seq)
{
    return seq->Name;
}



/* @obsolete ajSeqName
** @rename ajSeqGetNameC
*/
const char* __deprecated ajSeqName(const AjPSeq seq)
{
    return ajStrGetPtr(seq->Name);
}




/* @obsolete ajSeqGetName
** @rename ajSeqGetNameS
*/
const AjPStr __deprecated ajSeqGetName(const AjPSeq seq)
{
    return seq->Name;
}




/* @func ajSeqGetOffend *******************************************************
**
** Returns the sequence offend value.
** This is the number of positions removed from the original end.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Sequence offend.
** @@
******************************************************************************/

ajint ajSeqGetOffend(const AjPSeq seq)
{
    return seq->Offend;
}




/* @obsolete ajSeqOffend
** @rename ajSeqGetOffend
*/
ajint __deprecated ajSeqOffend(const AjPSeq seq)
{
    return ajSeqGetOffend(seq);
}

/* @func ajSeqGetOffset *******************************************************
**
** Returns the sequence offset from -sbegin originally.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [ajint] Sequence offset.
** @@
******************************************************************************/

ajint ajSeqGetOffset(const AjPSeq seq)
{
    return seq->Offset;
}


/* @obsolete ajSeqOffset
** @rename ajSeqGetOffset
*/
ajint __deprecated ajSeqOffset(const AjPSeq seq)
{
    return ajSeqGetOffset(seq);
}

/* @func ajSeqGetRange ********************************************************
**
** Returns the sequence range for a sequence.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqGetRange(const AjPSeq seq, ajint* begin, ajint* end)
{
    ajint jbegin;
    ajint jend;

    jbegin = seq->Begin;
    if(seq->Begin > 0)
	jbegin--;

    jend = seq->End;
    if(seq->End > 0)
	jend--;

    ajDebug("ajSeqGetRange '%S'\n", seq->Name);
    *begin = ajMathPos(ajSeqGetLen(seq), jbegin); /* string position */

    if(seq->End)
	*end = 1 + ajMathPosI(ajSeqGetLen(seq), *begin, jend);
    else
	*end = 1 + ajMathPosI(ajSeqGetLen(seq), *begin, ajSeqGetLen(seq));

    (*begin)++;				/* sequence positions are 1..end */

    return (*end - *begin + 1);
}



/* @func ajSeqGetRev **********************************************************
**
** Returns the sequence direction.
**
** See ajSeqReversed for whether it has already been reverse-complemented
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
** @@
******************************************************************************/

AjBool ajSeqGetRev(const AjPSeq seq)
{
    return seq->Rev;
}

/* @obsolete ajSeqGetReverse
** @rename ajSeqGetRev
*/
AjBool __deprecated ajSeqGetReverse(const AjPSeq seq)
{
    return seq->Rev;
}



/* @obsolete ajSeqGetReversed
** @rename ajSeqIsReversed
*/
AjBool __deprecated ajSeqGetReversed(const AjPSeq seq)
{
    return seq->Reversed;
}





/* @func ajSeqGetSeqC *********************************************************
**
** Returns the sequence in a sequence object as a character string.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [const char*] Sequence as a character string.
** @@
******************************************************************************/

const char* ajSeqGetSeqC(const AjPSeq seq)
{
    if(!seq)
	return "";

    return MAJSTRGETPTR(seq->Seq);
}

/* @func ajSeqGetSeqS *********************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [const AjPStr] Sequence as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetSeqS(const AjPSeq seq)
{
    if(!seq)
	return NULL;

    return seq->Seq;
}

/* @obsolete ajSeqStr
** @rename ajSeqGetSeqS
*/
const AjPStr __deprecated ajSeqStr(const AjPSeq seq)
{
    return ajSeqGetSeqS(seq);
}

/* @obsolete ajSeqChar
** @rename ajSeqGetSeqC
*/
const char* __deprecated ajSeqChar(const AjPSeq seq)
{
    if(!seq)
	return "";

    return ajStrGetPtr(seq->Seq);
}




/* @func ajSeqGetSeqCopyC *****************************************************
**
** Returns a sequence as a C character string. This is a copy of the string
** so the caller can do anything with it.
** It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [char*] Sequence as a null terminated character string.
** @@
******************************************************************************/

char* ajSeqGetSeqCopyC(const AjPSeq seq)
{
    return ajCharNewS(seq->Seq);
}

/* @obsolete ajSeqCharCopy
** @rename ajSeqGetSeqCopyC
*/
char* __deprecated ajSeqCharCopy(const AjPSeq seq)
{
    return ajCharNewS(seq->Seq);
}




/* @func ajSeqGetSeqCopyS *****************************************************
**
** Returns the sequence in a sequence object as a string.
** Because this is a copy of the internal string
** the caller may change the string. It must be copied back
** to a sequence (e.g. with ajSeqReplace) before output.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [AjPStr] Sequence as a string.
** @@
******************************************************************************/

AjPStr ajSeqGetSeqCopyS(const AjPSeq seq)
{
    static AjPStr str;

    str = ajStrNewS(seq->Seq);

    return str;
}


/* @obsolete ajSeqStrCopy
** @rename ajSeqGetSeqCopyS
*/
AjPStr __deprecated ajSeqStrCopy(const AjPSeq seq)
{
    return ajSeqGetSeqCopyS(seq);
}

/* @obsolete ajSeqCharCopyL
** @replace ajSeqGetSeqCopyC (1,2/1,ajSeqGetLen[2])
*/
char* __deprecated ajSeqCharCopyL(const AjPSeq seq, size_t size)
{
    return ajCharNewResS(seq->Seq, size);
}


/* @func ajSeqGetSvC **********************************************************
**
** Returns the sequence version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] SeqVersion number as a character string.
** @@
******************************************************************************/

const char* ajSeqGetSvC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Sv);
}




/* @func ajSeqGetSvS **********************************************************
**
** Returns the sequence version number.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] SeqVersion number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetSvS(const AjPSeq seq)
{
    return seq->Sv;
}

/* @obsolete ajSeqGetSv
** @rename ajSeqGetSvS
*/
const AjPStr __deprecated ajSeqGetSv(const AjPSeq seq)
{
    return seq->Sv;
}




/* @func ajSeqGetTaxC *********************************************************
**
** Returns the sequence primary taxon (species).
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Description as a character string.
** @@
******************************************************************************/

const char* ajSeqGetTaxC(const AjPSeq seq)
{
    return MAJSTRGETPTR(seq->Tax);
}




/* @func ajSeqGetTaxS *********************************************************
**
** Returns the sequence primary taxon (species).
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Description as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetTaxS(const AjPSeq seq)
{
    return seq->Tax;
}

/* @obsolete ajSeqGetTax
** @rename ajSeqGetTaxS
*/
const AjPStr __deprecated ajSeqGetTax(const AjPSeq seq)
{
    return seq->Tax;
}




/* @func ajSeqGetUsaC *********************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const char*] Name as a character string.
** @@
******************************************************************************/

const char* ajSeqGetUsaC(const AjPSeq seq)
{
    return MAJSTRGETPTR(ajSeqGetUsaS(seq));
}

/* @func ajSeqGetUsaS *********************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeq] Sequence object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqGetUsaS(const AjPSeq seq)
{
    ajDebug("ajSeqGetUsa '%S'\n", seq->Usa);

    if(ajStrGetLen(seq->Usa))
	return seq->Usa;

    seqMakeUsa(seq, &seqTempUsa);
    return seqTempUsa;
}


/* @obsolete ajSeqGetUsa
** @rename ajSeqGetUsaS
*/
const AjPStr __deprecated ajSeqGetUsa(const AjPSeq seq)
{
    return ajSeqGetUsaS(seq);
}

/* @section testing properties
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Is Test sequence property
** @nam4rule IsNuc Sequence is nucleotide
** @nam4rule IsProt Sequence is protein
** @nam4rule IsReversed Sequence is reversed
** @nam4rule IsTrimmed Sequence is trimmed to a subsequence
**
** @suffix True Sequence proerties relative to the original sequence
**
** @argrule Is seq [const AjPSeq] Sequence
**
** @valrule * [AjBool] Sequence boolean property
**
******************************************************************************/

/* @func ajSeqIsNuc ***********************************************************
**
** Tests whether a sequence is nucleotide.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
** @@
******************************************************************************/

AjBool ajSeqIsNuc(const AjPSeq seq)
{
    ajDebug("ajSeqIsNuc Type '%S'\n", seq->Type);

    if(ajStrMatchC(seq->Type, "N"))
	return ajTrue;

    if(ajStrMatchC(seq->Type, "P"))
	return ajFalse;

    if(ajSeqTypeGapnucS(seq->Seq)) /* returns char 0 on success */
    {
	ajDebug ("ajSeqIsNuc failed\n", seq->Type);
	return ajFalse;
    }

    return ajTrue;
}




/* @func ajSeqIsProt **********************************************************
**
** Tests whether a sequence is protein.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [AjBool] ajTrue for a protein sequence.
** @@
******************************************************************************/

AjBool ajSeqIsProt(const AjPSeq seq)
{
    ajDebug("ajSeqIsProt Type '%S'\n", seq->Type);

    if(ajStrMatchC(seq->Type, "P"))
	return ajTrue;

    if(ajStrMatchC(seq->Type, "N"))
	return ajFalse;

    if(ajSeqTypeAnyprotS(seq->Seq))	/* returns char 0 on success */
    {
	ajDebug ("ajSeqIsProt failed\n", seq->Type);
	return ajFalse;
    }

    return ajTrue;
}


/* @func ajSeqIsReversedTrue **************************************************
**
** Returns ajTrue if the sequence is reversed relative to the original sequence
**
** If the sequence has already been reversed, or is set to be reversed,
** the result will be true.
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] ajTrue if sequence is set to be reversed
** @@
******************************************************************************/

AjBool ajSeqIsReversedTrue(const AjPSeq seq)
{
    if (seq->Reversed)
    {
	if (seq->Rev)
	    return ajFalse;
	else
	    return ajTrue;
    }

    return seq->Rev;

}


/* @obsolete ajSeqRev
** @rename ajSeqIsReversed
*/
AjBool __deprecated ajSeqRev(const AjPSeq seq)
{
    return ajSeqIsReversed(seq);
}

/* @func ajSeqIsReversed ******************************************************
**
** Returns whether the sequence has been reversed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] Sequence Direction.
** @@
******************************************************************************/

AjBool ajSeqIsReversed(const AjPSeq seq)
{
    return seq->Reversed;
}




/* @func ajSeqIsTrimmed ******************************************************
**
** Returns ajTrue if the sequence is already trimmed
**
** @param [r] seq [const AjPSeq] Sequence object
** @return [AjBool] ajTrue if sequence is set to be reversed
** @@
******************************************************************************/

AjBool ajSeqIsTrimmed(const AjPSeq seq)
{
    return seq->Trimmed;
}



/* @section calculated properties
**
** @fdata [AjPSeq]
** @fcategory use
**
** @nam3rule Calc Calculate a value
** @nam4rule CalcCheckgcg Calculate GCG checksum
** @nam4rule CalcCount Count nucleotide bases
** @nam4rule CalcCrc Calculate cyclic reduncancy checksum
** @nam4rule CalcMolwt Calculate molecular weight
** @nam4rule CalcTruepos Calculate sequence position
** @nam5rule CalcTrueposMin Calculate sequence position in range
**
** @argrule Seq seq [const AjPSeq] Sequence
** @argrule CalcCount b [ajint*] Nucleotide base count
** @argrule Min imin [ajint] Minimum relative position
** @argrule Truepos ipos [ajint] Relative position
**
** @valrule CalcCheckgcg [ajint] GCG checksum
** @valrule CalcMolwt [float] Molecular weight
** @valrule CalcCrc [ajuint] Cyclic redundancy checksum
** @valrule CalcCount [void]
** @valrule CalcTruepos [ajint] Sequence position in original
******************************************************************************/

/* @func ajSeqCalcCheckgcg ****************************************************
**
** Calculates a GCG checksum for a sequence.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [ajint] GCG checksum.
** @@
******************************************************************************/

ajint ajSeqCalcCheckgcg(const AjPSeq seq)
{
    register ajlong  i;
    register ajlong  check = 0;
    register ajlong  count = 0;
    const char *cp;
    ajint ilen;

    cp   = ajStrGetPtr(seq->Seq);
    ilen = ajStrGetLen(seq->Seq);

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

/* @obsolete ajSeqCheckGcg
** @rename ajSeqCalcCheckgcg
*/
ajint __deprecated ajSeqCheckGcg(const AjPSeq seq)
{
    return ajSeqCalcCheckgcg(seq);
}

/* @func ajSeqCalcCount *******************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [r] seq [const AjPSeq] Sequence object
** @param [w] b [ajint*] integer array, minimum size 5, to hold the results.
** @return [void]
** @@
******************************************************************************/

void ajSeqCalcCount(const AjPSeq seq, ajint* b)
{
    const char* cp;

    ajDebug("ajSeqCount %d bases\n", ajSeqGetLen(seq));

    b[0] = b[1] = b[2] = b[3] = b[4] = 0;

    cp = ajStrGetPtr(seq->Seq);

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

    b[4] = ajSeqGetLen(seq) - b[0] - b[1] - b[2] - b[3];

    return;
}


/* @obsolete ajSeqCount
** @rename ajSeqCalcCount
*/
void __deprecated ajSeqCount(const AjPSeq seq, ajint* b)
{
    ajSeqCalcCount(seq, b);
    return;
}

/* @func ajSeqCalcCrc *********************************************************
**
** Calculates the SwissProt style CRC32 checksum for a protein sequence.
** This seems to be a bit reversal of a standard CRC32 checksum.
**
** @param [r] seq [const AjPSeq] Sequence as a string
** @return [ajuint] CRC32 checksum.
** @@
******************************************************************************/

ajuint ajSeqCalcCrc(const AjPSeq seq)
{
    return ajSeqstrCalcCrc(seq->Seq);
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




/* @func ajSeqCalcMolwt *******************************************************
**
** Calculates the molecular weight of a protein sequence.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [float] Molecular weight.
** @@
******************************************************************************/

float ajSeqCalcMolwt(const AjPSeq seq)
{
    return ajSeqstrCalcMolwt(seq->Seq);
}



/* @obsolete ajSeqPos
** @replace ajMathPos (1,2/'ajSeqGetLen[1]',2)
*/

ajint __deprecated ajSeqPos(const AjPSeq seq, ajint ipos)
{
    return 1+ajMathPosI(ajSeqGetLen(seq), 0, ipos);
}




/* @obsolete ajSeqPosI
** @replace ajMathPosI (1,2,3/'ajSeqGetLen[1]',2,3)
*/
ajint __deprecated ajSeqPosI(const AjPSeq seq, ajint imin, ajint ipos)
{
    return 1+ajMathPosI(ajSeqGetLen(seq), imin, ipos);
}




/* @obsolete ajSeqPosII
** @rename ajMathPosI
*/

ajint __deprecated ajSeqPosII(ajint ilen, ajint imin, ajint ipos)
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

    ajDebug("ajSeqPosII (ilen: %d imin: %d ipos: %d) = %d\n",
	    ilen, imin, ipos, jpos);

    return jpos;
}

/* @func ajSeqCalcTrueposMin **************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [r] seq [const AjPSeq] Target sequence.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqCalcTrueposMin(const AjPSeq seq, ajint imin, ajint ipos)
{
    ajint jmin;
    ajint jpos;

    jmin = imin;
    if (imin > 0)
	jmin--;

    jpos = ipos;
    if(ipos > 0)
	jpos--;

    if(ajSeqGetRev(seq))
	return 1 + seq->Offend + ajMathPosI(ajSeqGetLen(seq),
			  jmin, jpos);
    else
	return 1 + seq->Offset + ajMathPosI(ajSeqGetLen(seq),
			  jmin, jpos);
}

/* @obsolete ajSeqTruePosI
** @rename ajSeqCalcTrueposMin
*/

ajint __deprecated ajSeqTruePosI(const AjPSeq thys, ajint imin, ajint ipos)
{
    return ajSeqCalcTrueposMin(thys, imin, ipos);
}

/* @obsolete ajSeqTruePosII
** @rename ajMathPosI
*/
ajint __deprecated ajSeqTruePosII(ajint ilen, ajint imin, ajint ipos)
{
    return ajMathPosI(ilen, imin, ipos);
}








/* @func ajSeqCalcTruepos *****************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base
**
** @param [r] seq [const AjPSeq] Target sequence.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajSeqCalcTruepos(const AjPSeq seq, ajint ipos)
{
    ajint jpos;

    jpos = ipos;
    if (ipos > 0)
	jpos = ipos - 1;

    if(ajSeqGetRev(seq))
	return 1 + seq->Offend + ajMathPosI(ajSeqGetLen(seq), 0, jpos);
    else
	return 1 + seq->Offset + ajMathPosI(ajSeqGetLen(seq), 0, jpos);
}


/* @obsolete ajSeqTruePos
** @rename ajSeqCalcTruepos
*/
ajint __deprecated ajSeqTruePos(const AjPSeq thys, ajint ipos)
{
    return ajSeqCalcTruepos(thys, ipos);
}


/* @section exit
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPSeq]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/





/* @func ajSeqExit ************************************************************
**
** Cleans up sequence processing  internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajSeqExit(void)
{
    ajSeqReadExit();
    ajSeqWriteExit();
    ajSeqDbExit();
    ajSeqTypeExit();

    ajStrDel(&seqVersionAccnum);
    ajStrDel(&seqTempUsa);


    return;
}



/* @datasection [AjPSeqall] Sequence stream************************************
**
** Function is for manipulating sequence stream objects
**
** @nam2rule Seqall
**
******************************************************************************/


/* @section Sequence Stream Constructors **************************************
**
** @fdata [AjPSeqall]
**
** All constructors return a new sequence stream object by pointer. It
** is the responsibility of the user to first destroy any previous
** sequence stream object. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so
** anyway.
**
** @nam3rule New Constructor
**
** @valrule * [AjPSeqall]
** @fcategory new
**
******************************************************************************/



/* @func ajSeqallNew **********************************************************
**
** Creates a new sequence stream object to hold one sequence at a time.
**
** @return [AjPSeqall] New sequence stream object.
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




/* @section  destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeqall]
**
** @nam3rule Del Destructor
**
** @argrule Del Pseq [AjPSeqall*] Sequence stream object
** @valrule * [void]
** @fcategory delete
**
******************************************************************************/



/* @func ajSeqallDel **********************************************************
**
** Destructor for sequence stream objects
**
** @param [d] Pseq [AjPSeqall*] Sequence stream object reference
** @return [void]
** @@
******************************************************************************/

void ajSeqallDel(AjPSeqall *Pseq)
{
    if(!*Pseq)
	return;

    if(!(*Pseq)->Returned)
	ajSeqDel(&(*Pseq)->Seq);

    ajSeqinDel(&(*Pseq)->Seqin);

    AJFREE(*Pseq);

    return;
}





/* @section modifiers *****************************************
**
** These functions use the contents of a sequence stream object and
** update them.
**
** @fdata [AjPSeqall]
** @fcategory modify
**
** @nam4rule SetRange Set start and end position within sequence
** @nam5rule SetRangeRev Set start and end position and reverse direction
**                       of a sequence
** @argrule Range pos1 [ajint] Start position
** @argrule Range pos2 [ajint] End  position
******************************************************************************/



/* @obsolete ajSeqallReverse
** @remove sequence processed separately afetr ajSeqallNext
*/
void __deprecated ajSeqallReverse(AjPSeqall seq)
{
    ajint ibegin;
    ajint iend;

    ajDebug("ajSeqallReverse len: %d Begin: %d End: %d\n",
	    ajSeqallLen(seq), seq->Begin, seq->End);

    ibegin = seq->Begin;
    iend   = seq->End;

    seq->End   = -(ibegin);
    seq->Begin = -(iend);

    ajSeqReverseDo(seq->Seq);

    ajDebug("  all result len: %d Begin: %d End: %d\n",
	    ajSeqallLen(seq), seq->Begin, seq->End);

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
	    ajSeqGetLen(seq->Seq), ibegin, iend);

    if(ibegin)
	seq->Begin = seq->Seq->Begin = ibegin;

    if(iend)
	seq->End = seq->Seq->End = iend;

    ajDebug("      result: (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), seq->Begin, seq->End);

    return;
}




/* @func ajSeqallSetRangeRev **************************************************
**
** Sets the start and end positions for a sequence stream,
** and set the sequences to be reversed.
**
** @param [u] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] ibegin [ajint] Start position. Negative values are from the end.
** @param [r] iend [ajint] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqallSetRangeRev(AjPSeqall seq, ajint ibegin, ajint iend)
{
    ajDebug("ajSeqallSetRange (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), ibegin, iend);

    if(ibegin)
	seq->Begin = seq->Seq->Begin = ibegin;

    if(iend)
	seq->End = seq->Seq->End = iend;

    ajDebug("      result: (len: %d %d, %d)\n",
	    ajSeqGetLen(seq->Seq), seq->Begin, seq->End);

    if(!seq->Rev)
    {
	seq->Rev = ajTrue;
    }
    return;
}





/* @obsolete ajSeqallToLower
** @remove done when sequence is read
*/
void __deprecated ajSeqallToLower(AjPSeqall seqall)
{
    ajSeqFmtLower(seqall->Seq);
    return;
}


/* @obsolete ajSeqallToUpper
** @remove done when sequence is read
*/
void __deprecated ajSeqallToUpper(AjPSeqall seqall)
{
    ajSeqFmtUpper(seqall->Seq);
    return;
}




/* @section casts *********************************************
**
** These functions examine the contents of a sequence stream object
** and return some derived information. Some of them provide access to
** the internal components of a sequence stream object. They are
** provided for programming convenience but should be used with
** caution.
**
** @fdata [AjPSeqall]
** @fcategory cast
**
******************************************************************************/




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
    ajint jbegin;

    if (seq->Begin)
    {
	jbegin = seq->Begin;
	if(jbegin > 0)
	    jbegin--;

	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq), 0, jbegin);
    }
    if(seq->Seq->Begin)
    {
	jbegin = seq->Seq->Begin;
	if(jbegin > 0)
	    jbegin--;

	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq), 0, jbegin);
    }
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
    ajint jend;

    if (seq->End)
    {
	jend = seq->End;
	if(jend > 0)
	    jend--;
	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq),
			      ajSeqallBegin(seq)-1, jend);
    }

    if(seq->Seq->End)
    {
	jend = seq->Seq->End;
	if(jend > 0)
	    jend--;
	return 1 + ajMathPosI(ajSeqGetLen(seq->Seq),
			      ajSeqallBegin(seq)-1, jend);
    }
    return ajSeqGetLen(seq->Seq);
}



/* @func ajSeqallGetFilename **************************************************
**
** Returns the filename of a seqall object.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Seqall object pointer.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetFilename(const AjPSeqall seq)
{
    if(!seq)
	return NULL;
    if(!seq->Seqin)
	return NULL;

    ajDebug("ajSeqallGetFilename '%S' usa: '%S'\n",
	    seq->Seqin->Name, seq->Seqin->Usa);



    if(ajStrGetLen(seq->Seqin->Filename))
	return seq->Seqin->Filename;

    return NULL;
}





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
    return ajSeqGetLen(seqall->Seq);
}






/* @func ajSeqallGetName ******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetName(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetName '%S'\n", seq->Seqin->Name);

    return seq->Seqin->Name;
}




/* @func ajSeqallGetNameSeq ***************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetNameSeq(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetNameSeq '%S'\n", seq->Seq->Name);

    return ajSeqGetNameS(seq->Seq);
}




/* @func ajSeqallGetRange *****************************************************
**
** Returns the sequence range for a sequence stream
**
** @param [r] seq [const AjPSeqall] Sequence stream object.
** @param [w] begin [ajint*] Sequence range begin
** @param [w] end [ajint*] Sequence range end
** @return [ajint] Sequence range length
** @@
******************************************************************************/

ajint ajSeqallGetRange(const AjPSeqall seq, ajint* begin, ajint* end)
{
    ajDebug("ajSeqallGetRange '%S'\n", seq->Seq->Name);

    return ajSeqGetRange(seq->Seq, begin, end);
}



/* @func ajSeqallGetUsa *******************************************************
**
** Returns the sequence name of a sequence stream.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] seq [const AjPSeqall] Sequence object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqallGetUsa(const AjPSeqall seq)
{
    ajDebug("ajSeqallGetUsa '%S'\n", seq->Seqin->Usa);

    return seq->Seqin->Usa;
}




/* @datasection [AjPSeqset] Sequence set **************************************
**
** Function is for manipulating sequence set objects
**
** @nam2rule Seqset
**
******************************************************************************/



/* @section constructors *****************************************
**
** All constructors return a new sequence set object by pointer. It is the
** responsibility of the user to first destroy any previous
** sequence. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [AjPSeqset]
**
******************************************************************************/






/* @func ajSeqsetNew **********************************************************
**
** Creates a new sequence set object to hold all sequences in memory.
**
** @return [AjPSeqset] New sequence set object.
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




/* @section  destructors ***************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the sequence.
**
** @fdata [AjPSeqall]
**
** @nam3rule Del Destructor
** @nam3rule Delarray Array destructor
**
** @argrule Del seq [AjPSeqall*] Sequence stream object
** @argrule Delarray seq [AjPSeqall**] Sequence stream object array
** @valrule * [void]
** @fcategory delete
**
******************************************************************************/



/* @func ajSeqsetDel **********************************************************
**
** Destructor for sequence set objects
**
** @param [d] Pseq [AjPSeqset*] Sequence set object reference
** @return [void]
** @@
******************************************************************************/

void ajSeqsetDel(AjPSeqset *Pseq)
{
    ajint i;
    AjPSeqset seq = NULL;

    if(!Pseq || !*Pseq)
	return;

    seq = *Pseq;

    ajDebug("ajSeqsetDel size: %d\n", seq->Size);

    ajStrDel(&seq->Type);
    ajStrDel(&seq->Formatstr);
    ajStrDel(&seq->Filename);
    ajStrDel(&seq->Full);
    ajStrDel(&seq->Name);
    ajStrDel(&seq->Usa);
    ajStrDel(&seq->Ufo);

    for(i=0; i<seq->Size; ++i)
	ajSeqDel(&seq->Seq[i]);

    AJFREE(seq->Seq);
    AJFREE(seq->Seqweight);

    AJFREE(seq);

    return;
}




/* @func ajSeqsetDelarray *****************************************************
**
** Destructor for array of sequence set objects
**
** @param [d] thys [AjPSeqset**] Sequence set object reference
** @return [void]
** @@
******************************************************************************/

void ajSeqsetDelarray(AjPSeqset **thys)
{
    ajint i = 0;

    if(!thys || !*thys)
	return;

    while(*thys[i])
    {
	ajSeqsetDel(&(*thys)[i]);
	i++;
    }

    ajDebug("ajSeqsetallDel size: %d\n", i);

    AJFREE(*thys);

    return;
}



/* @section casts *********************************************
**
** These functions examine the contents of a sequence set object
** and return some derived information. Some of them provide access to
** the internal components of a sequence stream object. They are
** provided for programming convenience but should be used with
** caution.
**
** @fdata [AjPSeqset]
** @fcategory cast
**
******************************************************************************/



/* @func ajSeqsetGetFilename **************************************************
**
** Returns the filename of a sequence set.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [r] thys [const AjPSeqset] Sequence set object.
** @return [const AjPStr] Name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqsetGetFilename(const AjPSeqset thys)
{
    ajDebug("ajSeqsetGetFilename '%S' usa: '%S'\n", thys->Name, thys->Usa);

    if(!thys)
	return NULL;

    if(ajStrGetLen(thys->Filename))
	return thys->Filename;

    return NULL;
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
    ajint jbegin;
    ajint jend;

    jbegin = thys->Begin;
    if(jbegin > 0)
	jbegin--;

    jend = thys->End;
    if(jend > 0)
	jend--;

    ajDebug("ajSeqsetGetRange '%S' begin %d end %d len: %d\n",
	    thys->Name, thys->Begin, thys->End, thys->Len);
    *begin = ajMathPosI(thys->Len, 0, jbegin);

    if(thys->End)
	*end = 1 + ajMathPosI(thys->Len, *begin, jend);
    else
	*end = 1 + ajMathPosI(thys->Len, *begin, thys->Len);

    (*begin)++;

    return (*end - *begin + 1);
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






/* @section modifiers ********************************************
**
** These functions use the contents of a sequence set object and
** update them.
**
**
** @fdata [AjPSeqset]
** @fcategory modify
**
******************************************************************************/




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
	if(ajSeqGetLen(seq->Seq[i]) < seq->Len)
	{
	    nfix++;
	    ilen = seq->Len - ajSeqGetLen(seq->Seq[i]);
	    if(ilen > ifix)
		ifix = ilen;
	    ajStrAppendCountK(&seq->Seq[i]->Seq, '-', ilen);
	}

    ajDebug("      result: (len: %d added: %d number of seqs fixed: nfix\n",
	    seq->Len, ifix, nfix);

    return ifix;
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
	ajSeqReverseDo(thys->Seq[i]);

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




/* @func ajSeqsetToLower ******************************************************
**
** Converts all sequences in a set to lower case.
**
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetToLower(AjPSeqset seqset)
{
    ajint i;

    for(i=0; i < seqset->Size; i++)
	ajSeqFmtLower(seqset->Seq[i]);

    return;
}




/* @func ajSeqsetToUpper ******************************************************
**
** Converts all sequences in a set to upper case.
**
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetToUpper(AjPSeqset seqset)
{
    ajint i;

    for(i=0; i < seqset->Size; i++)
	ajSeqFmtUpper(seqset->Seq[i]);

    return;
}





/* @func ajSeqsetTrim ******************************************************
**
** Trims a sequence set to start and end positions
**
** @param [u] seqset [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetTrim(AjPSeqset seqset)
{
    ajint i;

    for(i=0; i < seqset->Size; i++)
	ajSeqTrim(seqset->Seq[i]);

    return;
}




/* @section casts ************************************************
**
** These functions examine the contents of a sequence set object and
** return some derived information. Some of them provide access to the
** internal components of a sequence set object. They are provided for
** programming convenience but should be used with caution.
**
** @fdata [AjPSeqset]
** @fcategory cast
**
******************************************************************************/




/* @func ajSeqsetAcc **********************************************************
**
** Returns the accession number of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [const AjPStr] accession number as a string.
** @@
******************************************************************************/

const AjPStr ajSeqsetAcc(const AjPSeqset seq, ajint i)
{
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Acc;
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
    ajint jbegin;

    if(!seq->Begin)
	return 1;

    jbegin = seq->Begin;
    if(jbegin > 0)
	jbegin--;

    return 1 + ajMathPosI(seq->Len, 0, jbegin);
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
    ajint jend;

    if(!seq->End)
	return (seq->Len);

    jend = seq->End;
    if(jend > 0)
	jend--;

    return 1 + ajMathPosI(seq->Len, ajSeqsetBegin(seq)-1, jend);
}




/* @func ajSeqsetLen **********************************************************
**
** Returns the length of a sequence set, which is the maximum sequence
** length in the set.
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] sequence set length.
** @@
******************************************************************************/

ajint ajSeqsetLen(const AjPSeqset seq)
{
    return seq->Len;
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

    if(ajStrGetLen(thys->Name))
      return thys->Name;

    return thys->Usa;
}




/* @func ajSeqsetName *********************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [const AjPStr] sequence name as a string.
** @@
******************************************************************************/

const AjPStr ajSeqsetName(const AjPSeqset seq, ajint i)
{
    if(i >= seq->Size)
	return NULL;

    return seq->Seq[i]->Name;
}


/* @func ajSeqsetSeq **********************************************************
**
** Returns the sequence data of a sequence in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @param [r] i [ajint] Sequence index
** @return [const char*] Sequence as a NULL terminated string.
** @@
******************************************************************************/

const char* ajSeqsetSeq(const AjPSeqset seq, ajint i)
{
    if(i >= seq->Size)
	return NULL;

    return ajStrGetPtr(seq->Seq[i]->Seq);
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




/* @func ajSeqsetSize *********************************************************
**
** Returns the number of sequences in a sequence set
**
** @param [r] seq [const AjPSeqset] Sequence set object
** @return [ajint] sequence set size.
** @@
******************************************************************************/

ajint ajSeqsetSize(const AjPSeqset seq)
{
    return seq->Size;
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



/* @section test **************************************************************
**
** Tests properties of a sequence set
**
** @fdata [AjPSeqset]
** @fcategory
**
******************************************************************************/

/* @func ajSeqsetIsDna ********************************************************
**
** Tests whether a sequence set is DNA.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
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




/* @func ajSeqsetIsNuc ********************************************************
**
** Tests whether a sequence set is nucleotide.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
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




/* @func ajSeqsetIsProt *******************************************************
**
** Tests whether a sequence set is protein.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a protein sequence set.
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


/* @func ajSeqsetIsRna ********************************************************
**
** Tests whether a sequence set is RNA.
**
** @param [r] thys [const AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
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
** @param [d] Pseq [AjPSeq*] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajSeqDel(AjPSeq* Pseq)
{
    AjPSeq seq;
    AjPStr ptr = NULL;

    seq = Pseq ? *Pseq : 0;

    if(!Pseq)
	return;
    if(!*Pseq)
	return;

    ajStrDel(&seq->Name);
    ajStrDel(&seq->Acc);
    ajStrDel(&seq->Sv);
    ajStrDel(&seq->Gi);
    ajStrDel(&seq->Tax);
    ajStrDel(&seq->Type);
    ajStrDel(&seq->Db);
    ajStrDel(&seq->Setdb);
    ajStrDel(&seq->Full);
    ajStrDel(&seq->Date);
    ajStrDel(&seq->Desc);
    ajStrDel(&seq->Doc);
    ajStrDel(&seq->Usa);
    ajStrDel(&seq->Ufo);
    ajStrDel(&seq->Formatstr);
    ajStrDel(&seq->Filename);
    ajStrDel(&seq->Entryname);
    ajStrDel(&seq->TextPtr);
    ajStrDel(&seq->Seq);
    AJFREE(seq->Accuracy);

    if(seq->Fttable)
	ajFeattableDel(&seq->Fttable);

    while(ajListstrPop(seq->Acclist,&ptr))
	ajStrDel(&ptr);
    ajListDel(&seq->Acclist);

    while(ajListstrPop(seq->Keylist,&ptr))
	ajStrDel(&ptr);
    ajListDel(&seq->Keylist);

    while(ajListstrPop(seq->Taxlist,&ptr))
	ajStrDel(&ptr);
    ajListDel(&seq->Taxlist);

    AJFREE(*Pseq);
    return;
}




/* @func ajSeqClear ***********************************************************
**
** Resets all data for a sequence object so that it can be reused.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqClear(AjPSeq seq)
{
    AjPStr ptr = NULL;

    ajStrSetClear(&seq->Name);
    ajStrSetClear(&seq->Acc);
    ajStrSetClear(&seq->Sv);
    ajStrSetClear(&seq->Gi);
    ajStrSetClear(&seq->Tax);
    ajStrSetClear(&seq->Type);
    ajStrSetClear(&seq->Db);
    ajStrSetClear(&seq->Full);
    ajStrSetClear(&seq->Date);
    ajStrSetClear(&seq->Desc);
    ajStrSetClear(&seq->Doc);
    ajStrSetClear(&seq->Usa);
    ajStrSetClear(&seq->Ufo);

    ajStrSetClear(&seq->Formatstr);
    ajStrSetClear(&seq->Filename);
    ajStrSetClear(&seq->Entryname);
    ajStrSetClear(&seq->TextPtr);
    ajStrSetClear(&seq->Seq);

    AJFREE(seq->Accuracy);

    seq->Begin = 0;
    seq->End   = 0;
    seq->Rev      = ajFalse;
    seq->Reversed = ajFalse;
    seq->Trimmed  = ajFalse;

    while(ajListstrPop(seq->Acclist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seq->Keylist,&ptr))
	ajStrDel(&ptr);

    while(ajListstrPop(seq->Taxlist,&ptr))
	ajStrDel(&ptr);

    ajFeattableDel(&seq->Fttable);

    return;
}




/* @func ajSeqallClear ********************************************************
**
** Resets all data for a sequence stream object so that it can be reused.
**
** @param [u] seq [AjPSeqall] Sequence stream
** @return [void]
** @@
******************************************************************************/

void ajSeqallClear(AjPSeqall seq)
{
    ajSeqClear(seq->Seq);
    ajSeqinClear(seq->Seqin);
    seq->Count = 0;
    seq->Begin = 0;
    seq->End   = 0;
    seq->Rev   = ajFalse;
    seq->Returned = ajFalse;
 
    return;
}





/* @section modifiable sequence retrieval *************************************
**
** Functions for returning elements of a sequence object.
**
** @fdata       [AjPSeq]
** @fcategory modify
**
******************************************************************************/

/* @func ajSeqMod *************************************************************
**
** Makes a sequence modifiable by making sure there is no duplicate
** copy of the sequence.
**
** @param [u] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqMod(AjPSeq seq)
{
    ajStrGetuniqueStr(&seq->Seq);

    return;
}






/* @section debug *************************************************************
**
** Reports sequence contents for debugging purposes
**
** @fdata [AjPSeq]
** @fcategory misc
**
******************************************************************************/


/* @func ajSeqTrace ***********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

void ajSeqTrace(const AjPSeq seq)
{
    AjIList it;
    AjPStr cur;
    ajint i;

    ajDebug("Sequence trace\n");
    ajDebug( "==============\n\n");
    ajDebug( "  Name: '%S'\n", seq->Name);

    if(ajStrGetLen(seq->Acc))
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

    if(ajStrGetLen(seq->Sv))
	ajDebug( "  SeqVersion: '%S'\n", seq->Sv);

    if(ajStrGetLen(seq->Gi))
	ajDebug( "  GenInfo Id: '%S'\n", seq->Gi);

    if(ajStrGetLen(seq->Type))
	ajDebug( "  Type: '%S' (%d)\n", seq->Type, seq->EType);

    if(ajStrGetLen(seq->Desc))
	ajDebug( "  Description: '%S'\n", seq->Desc);

    if(ajStrGetLen(seq->Tax))
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

    if(ajSeqGetLen(seq))
	ajDebug( "  Length: %d\n", ajSeqGetLen(seq));
    i = ajSeqGapCount(seq);
    if(i)
	ajDebug( "  Gap count: %d\n", i);

    if(seq->Rev)
	ajDebug( "     Rev: %B\n", seq->Rev);

    if(seq->Reversed)
	ajDebug( "Reversed: %B\n", seq->Reversed);

    if(seq->Begin)
	ajDebug( "   Begin: %d\n", ajSeqGetBegin(seq));

    if(seq->End)
	ajDebug( "     End: %d\n", ajSeqGetEnd(seq));

    if(seq->Offset)
	ajDebug( "  Offset: %d\n", seq->Offset);

    if(seq->Offend)
	ajDebug( "  Offend: %d\n", seq->Offend);

    if(ajStrGetRes(seq->Seq))
	ajDebug( "  Reserved: %d\n", ajStrGetRes(seq->Seq));

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

    if(seq->Fttable)
	ajDebug( "  Fttable: exists\n");

    if(ajStrGetLen(seq->Formatstr))
	ajDebug( "  Input format: '%S' (%d)\n", seq->Formatstr, seq->Format);

    if(ajStrGetLen(seq->Filename))
	ajDebug( "  Filename: '%S'\n", seq->Filename);

    if(ajStrGetLen(seq->Entryname))
	ajDebug( "  Entryname: '%S'\n", seq->Entryname);

    if(seq->Weight)
	ajDebug( "  Weight: %.3f\n", seq->Weight);

    if(ajStrGetLen(seq->Doc))
	ajDebug( "  Documentation:...\n%S\n", seq->Doc);

	ajDebug( "Sequence:...\n%S\n", seq->Seq);
    ajDebug( "\n");

    return;
}




/* @func ajSeqTraceT **********************************************************
**
** Reports an AjPSeq object to debug output
**
** @param [r] seq [const AjPSeq] alignment object
** @param [r] title [const char*] Trace report title
** @return [void]
******************************************************************************/

void ajSeqTraceT(const AjPSeq seq, const char* title)
{
    ajDebug("\n%s\n",title);
    ajSeqTrace(seq);

    return;
}




/* @func ajSeqNum *************************************************************
**
** Converts a sequence to numbers using a conversion table.
**
** @param [r] seq [const AjPSeq] Sequence.
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @param [w] numseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqNum(const AjPSeq seq, const AjPSeqCvt cvt, AjPStr* numseq)
{
    return ajSeqNumS(seq->Seq, cvt, numseq);;
}




/* @func ajSeqNumS ************************************************************
**
** Converts a string of sequence characters to numbers using
** a conversion table.
**
** @param [r] seq [const AjPStr] Sequence as a string
** @param [r] cvt [const AjPSeqCvt] Conversion table.
** @param [w] numseq [AjPStr*] Output numeric version of the sequence.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqNumS(const AjPStr seq, const AjPSeqCvt cvt, AjPStr* numseq)
{
    const char *cp;
    char *ncp;

    cp = ajStrGetPtr(seq);

    ajStrAssignS(numseq, seq);
    ncp = ajStrGetuniquePtr(numseq);

    while(*cp)
    {
	*ncp = cvt->table[(ajint)*cp];
	cp++;
	ncp++;
    }

    return ajTrue;
}




/* @func ajSeqIsGarbage *******************************************************
**
** Returns the Garbage element of an Seq object.
**
** @param [r] seq [const AjPSeq] Sequence.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqIsGarbage(const AjPSeq seq)
{
    return seq->Garbage;
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


    AJCNEW0(ret->rlabels, n);
    for(i=0; i<n; i++)
	ret->rlabels[i] = ajStrNew();
    for(i=0; i<n; i++)
	ajStrAssignS(&ret->rlabels[i], bases[i]);


    AJCNEW0(ret->clabels, n);
    for(i=0; i<n; i++)
	ret->clabels[i] = ajStrNew();
    for(i=0; i<n; i++)
	ajStrAssignS(&ret->clabels[i], bases[i]);

    for(i=0; i<n; i++)
    {
	ajStrAppendK(&ret->bases, ajStrGetCharFirst(bases[i]));
	ret->table[toupper((ajint) ajStrGetCharFirst(bases[i]))] = ajSysItoC(i+1);
	ret->table[tolower((ajint) ajStrGetCharFirst(bases[i]))] = ajSysItoC(i+1);
    }

    return ret;
}





/* @func ajSeqCvtNewZeroSS ****************************************************
**
** Generates a new conversion table in which the first character of the first 
** string in the array provided is converted to 1, the first character of the 
** second string is converted to 2, the first character of the third string is
** converted to 3 and so on.
** Upper and lower case characters are converted to the same numbers.
** All other characters are set to zero.
** For use with assymetrical matrices. 
**
** @param [r] bases [const AjPPStr] Allowed sequence character strings (size
**                            specified by parameter n)
** @param [r] n [int] Number of strings
** @param [r] rbases [const AjPPStr] Allowed sequence character strings for
** rows (size specified by parameter rn)
** @param [r] rn [int] Number of strings (rows)
** @return [AjPSeqCvt] Conversion table.
** @@
******************************************************************************/
AjPSeqCvt ajSeqCvtNewZeroSS (const AjPPStr bases, int n, 
			     const AjPPStr rbases, int rn)
{
    static AjPSeqCvt ret;
    ajint i;
    

    AJNEW0(ret);
    ret->len = n;
    ret->nclabels = n;
    ret->nrlabels = rn;
    ret->size = CHAR_MAX - CHAR_MIN + 1;
    ret->table = AJCALLOC0(ret->size, sizeof(char));
    ret->bases = ajStrNew();
    ret->missing = 0;


    AJCNEW0(ret->rlabels, rn);
    for(i=0; i<rn; i++)
	ret->rlabels[i] = ajStrNew();
    for(i=0; i<rn; i++)
	ajStrAssignS(&ret->rlabels[i], rbases[i]);


    AJCNEW0(ret->clabels, n);
    for(i=0; i<n; i++)
	ret->clabels[i] = ajStrNew();
    for(i=0; i<n; i++)
	ajStrAssignS(&ret->clabels[i], bases[i]);


    for(i=0; i<n; i++)
    {
	/* ajStrAssignS(&ret->labels[i], bases[i]); */
	ajStrAppendK(&ret->bases, ajStrGetCharFirst(bases[i]));
	ret->table[toupper((ajint) ajStrGetCharFirst(bases[i]))] = ajSysItoC(i+1);
	ret->table[tolower((ajint) ajStrGetCharFirst(bases[i]))] = ajSysItoC(i+1);
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
** @param [w] Pcvt [AjPSeqCvt*] Conversion table reference
** @return [void]
** @@
******************************************************************************/

void ajSeqCvtDel (AjPSeqCvt* Pcvt)
{
    ajint i=0;
    
    if(!*Pcvt|| !Pcvt)
	return;

    AJFREE((*Pcvt)->table);
    ajStrDel(&(*Pcvt)->bases);

    if((*Pcvt)->rlabels)
    {
	for(i=0;i<(*Pcvt)->nrlabels;i++)
	    ajStrDel(&(*Pcvt)->rlabels[i]);
	AJFREE((*Pcvt)->rlabels);
    }
    
    if((*Pcvt)->clabels)
    {
	for(i=0;i<(*Pcvt)->nclabels;i++)
	    ajStrDel(&(*Pcvt)->clabels[i]);
	AJFREE((*Pcvt)->clabels);
    }
    
    AJFREE(*Pcvt);

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
** @param [r] cvt [const AjPSeqCvt] Conversion table
**
** @return [ajint] Length
** @@
******************************************************************************/

ajint ajSeqCvtLen(const AjPSeqCvt cvt)
{
    return cvt->len;
}




/* @func ajSeqCvtK ************************************************************
**
** Returns the integer code corresponding to a sequence character
** in a conversion table
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] ch [char] Sequence character
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtK(const AjPSeqCvt cvt, char ch)
{
    return cvt->table[(ajint)ch];
}




/* @func ajSeqCvtKS ***********************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table.  For use with symetrical matrices.
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] ch [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtKS (const AjPSeqCvt cvt, const AjPStr ch)
{
    /* Row and column labels will be identical. */
    return(ajSeqCvtKSRow(cvt, ch));
    
    /*    ajWarn("Sequence character string not found in ajSeqCvtKS");
    return 0; */
}






/* @func ajSeqCvtKSRow ********************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table (for rows in assymetrical matrices).
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] ch [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtKSRow (const AjPSeqCvt cvt, const AjPStr ch)
{
    ajint i=0;

    for(i=0;i<cvt->nrlabels;i++)
	if(ajStrMatchS(ch, cvt->rlabels[i]))
	    return i+1;
    /* i+1 is returned because the size of a matrix is always 1 bigger than
       the number of labels. This is the "padding" first row/column which 
       has all values of 0. */


    ajWarn("Sequence character string not found in ajSeqCvtKSRow");
    return 0;
}




/* @func ajSeqCvtKSColumn ****************************************************
**
** Returns the integer code corresponding to a sequence character string
** in a conversion table (for columns in assymetrical matrices).
**
** @param [r] cvt [const AjPSeqCvt] Conversion table
** @param [r] ch [const AjPStr] Sequence character string
**
** @return [ajint] Conversion code
** @@
******************************************************************************/

ajint ajSeqCvtKSColumn (const AjPSeqCvt cvt, const AjPStr ch)
{
    ajint i=0;
    
    for(i=0;i<cvt->nclabels;i++)
	if(ajStrMatchS(ch, cvt->clabels[i]))
	    return i+1;
    /* i+1 is returned because the size of a matrix is always 1 bigger than
       the number of labels. This is the "padding" first row/column which 
       has all values of 0. */


    ajWarn("Sequence character string not found in ajSeqCvtKSColumn");
    return 0;
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
    pthis->File = file;

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
** @param [d] Pseqout [AjPSeqout*] Sequence output object
** @return [void]
** @@
******************************************************************************/

void ajSeqoutDel(AjPSeqout* Pseqout)
{
    AjPSeqout seqout;
    AjPSeq    seq    = NULL;
    AjPStr    tmpstr = NULL;

    seqout = *Pseqout;

    if(!seqout)
	return;

    ajStrDel(&seqout->Name);
    ajStrDel(&seqout->Acc);
    ajStrDel(&seqout->Sv);
    ajStrDel(&seqout->Gi);
    ajStrDel(&seqout->Tax);
    ajStrDel(&seqout->Desc);
    ajStrDel(&seqout->Type);
    ajStrDel(&seqout->Outputtype);
    ajStrDel(&seqout->Db);
    ajStrDel(&seqout->Setdb);
    ajStrDel(&seqout->Full);
    ajStrDel(&seqout->Date);
    ajStrDel(&seqout->Doc);
    ajStrDel(&seqout->Usa);
    ajStrDel(&seqout->Ufo);
    ajStrDel(&seqout->FtFormat);
    ajStrDel(&seqout->FtFilename);
    ajStrDel(&seqout->Informatstr);
    ajStrDel(&seqout->Formatstr);
    ajStrDel(&seqout->Filename);
    ajStrDel(&seqout->Directory);
    ajStrDel(&seqout->Entryname);
    ajStrDel(&seqout->Seq);
    ajStrDel(&seqout->Extension);

    while(ajListPop(seqout->Acclist,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListDel(&seqout->Acclist);

    while(ajListPop(seqout->Keylist,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListDel(&seqout->Keylist);
    
    while(ajListPop(seqout->Taxlist,(void **)&tmpstr))
	ajStrDel(&tmpstr);
    ajListDel(&seqout->Taxlist);

    while(ajListPop(seqout->Savelist,(void **)&seq))
	ajSeqDel(&seq);
    ajListDel(&seqout->Savelist);
    ajFeattabOutDel(&seqout->Ftquery);

    if(seqout->Knownfile)
	seqout->File = NULL;
    else
	ajFileClose(&seqout->File);

    AJFREE(seqout->Accuracy);
    AJFREE(*Pseqout);

    return;
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

    ajint jbegin;
    ajint jend;

    if(thys->Trimmed)
    {
	ajWarn("Sequence '%s' already trimmed", ajSeqName(thys));
	return okay;
    }

    if(thys->Rev)
	ajSeqReverseDo(thys);

    jbegin = thys->Begin;
    if(jbegin > 0)
	jbegin--;
    jend = thys->End;
    if(jend > 0)
	jend--;

    begin = 1 + ajMathPosI(ajSeqGetLen(thys), 0, jbegin);
    end   = 1 + ajMathPosI(ajSeqGetLen(thys), begin-1, jend);

    ajDebug("Trimming %d from %d (%d) to %d (%d) "
	    "Rev: %B Reversed: %B Trimmed: %B\n",
	    thys->Seq->Len,thys->Begin,begin, thys->End, end,
	    thys->Rev, thys->Reversed, thys->Trimmed);

    if(thys->End)
    {
	if(end < begin)
	    return ajFalse;
	thys->Offend = thys->Seq->Len-(end);
	okay = ajStrCutEnd(&(thys->Seq),thys->Seq->Len-(end));
	thys->End    = 0;
    }

    if(thys->Begin)
    {
	okay = ajStrCutStart(&thys->Seq,begin-1);
	thys->Offset = begin-1;
	thys->Begin = 0;
    }

    ajDebug("After Trimming len = %d off = %d offend = %d\n",
	    thys->Seq->Len, thys->Offset, thys->Offend);
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

    static char testchars[] = "-~.? "; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    while(*testgap)
    {
	ret += ajStrCalcCountK(str, *testgap);
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
    ajSeqGapStandardS(thys->Seq, gapch);
    return;
}




/* @func ajSeqGapStandardS ****************************************************
**
** Makes all gaps in a string use a standard gap character
**
** @param [w] thys [AjPStr] Sequence string
** @param [r] gapch [char] Gap character (or '-' if zero)
** @return [void]
******************************************************************************/

void ajSeqGapStandardS(AjPStr thys, char gapch)
{
    char newgap = '-';
    static char testchars[] = "-~.? "; /* all known gap characters */
    char *testgap;

    testgap = testchars;

    if(gapch)
	newgap = gapch;

    /*ajDebug("ajSeqGapStandardS '%c'=>'%c' '%S'\n",
            gapch, newgap, thys->Seq);*/

    while(*testgap)
    {
	if(newgap != *testgap)
	{
	    ajStrExchangeKK(&thys, *testgap, newgap);
	    /*ajDebug(" ajSeqGapStandardS replaced         '%c'=>'%c' '%S'\n",
		    *testgap, newgap, thys);*/
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

    ajDebug("ajSeqFill(len: %d -> ilen:%d)\n", ajSeqGetLen(seq), ilen);

    if(ajSeqGetLen(seq) < len)
    {
	ilen = len - ajSeqGetLen(seq);
	ajStrAppendCountK(&seq->Seq, '-', ilen);
    }

    ajDebug("      result: (len: %d added: %d\n",
	     ajSeqGetLen(seq), ilen);

    return ilen;
}
/* @func ajSeqDefName ******************************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] thys [AjPSeq] Sequence object
** @param [r] setname [const AjPStr] Name set by caller
** @param [r] multi [AjBool] If true, appends a number to the name.
** @return [void]
** @@
******************************************************************************/

void ajSeqDefName(AjPSeq thys, const AjPStr setname, AjBool multi)
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

    ajDebug("ajSeqDefName set to  '%S'\n", thys->Name);

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

    i = ajStrGetLen(accnum);
    if(i < 6)
	return ajFalse;

    cp = ajStrGetPtr(accnum);

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
** @return [const AjPStr] accession number part of the string if successful
** @@
******************************************************************************/

const AjPStr ajIsSeqversion(const AjPStr sv)
{
    ajint i;
    const char *cp;
    AjBool dot = ajFalse;		/* have we found the '.' */
    AjBool v = 0;	   /* number of digits of version after '.' */

    if(!sv)
	return NULL;

    i = ajStrGetLen(sv);
    if(i < 8)
	return NULL;

    cp = ajStrGetPtr(sv);

    /* must have an alphabetic start */

    if(!isalpha((ajint)*cp))
	return NULL;

    ajStrAssignResC(&seqVersionAccnum, 12, "");
    ajStrAppendK(&seqVersionAccnum, *cp++);

    /* two choices for the next character */

    if(isalpha((ajint)*cp))
    {					/* EMBL/GenBank AAnnnnnn */
        ajStrAppendK(&seqVersionAccnum, *cp);
	cp++;

	if(*cp == '_')		/* REFSEQ NM_nnnnnn */
	{
	    ajStrAppendK(&seqVersionAccnum, *cp);
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
			ajStrAppendK(&seqVersionAccnum, *cp);
		}
		++cp;
	    }
	    else
		return NULL;
	}
	if(v)
	    return seqVersionAccnum;
	else
	    return NULL;
    }
    else if(isdigit((ajint)*cp))
    {					/* EMBL/GenBank old Annnnn */
	/* or SWISS AnXXXn */
        ajStrAppendK(&seqVersionAccnum, *cp);
	cp++;

	for(i=0; i<3; i++)
	    if(isalpha((ajint)*cp) || isdigit((ajint)*cp))
	    {
	        ajStrAppendK(&seqVersionAccnum, *cp);
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
			ajStrAppendK(&seqVersionAccnum, *cp);
		}
		++cp;
	    }
	    else
		return NULL;
	}
	if(v)
	    return seqVersionAccnum;
	else
	    return NULL;
    }

    return NULL;
}






/* @func ajSeqstrReverse ******************************************************
**
** Reverses and complements a nucleotide sequence provided as a string.
**
** @param [u] pthis [AjPStr*] Sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqstrReverse(AjPStr* pthis)
{
    char *cp;
    char *cq;
    char tmp;

    cp = ajStrGetuniquePtr(pthis);
    cq = cp + ajStrGetLen(*pthis) - 1;

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

/* @obsolete ajSeqReverseStr
** @rename ajSeqstrReverse
*/
void __deprecated ajSeqReverseStr(AjPStr* pthis)
{
    ajSeqstrReverse(pthis);
    return;
}


/* @func ajSeqstrComplementOnly ***********************************************
**
** Complements but does not reverse a nucleotide sequence provided as a string.
**
** @param [u] pthis [AjPStr*] Sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqstrComplementOnly(AjPStr* pthis)
{
    char *cp;

    cp = ajStrGetuniquePtr(pthis);

    while(*cp)
    {
	*cp = ajSeqBaseComp(*cp);
	cp++;
    }

    return;
}

/* @obsolete ajSeqCompOnlyStr
** @rename ajSeqstrComplementOnly
*/
void __deprecated ajSeqCompOnlyStr(AjPStr* pthis)
{
    ajSeqstrComplementOnly(pthis);
    return;
}


/* @func ajSeqstrCalcMolwt ****************************************************
**
** Calculates the molecular weight of a protein sequence.
**
** @param [r] seq [const AjPSeq] Sequence
** @return [float] Molecular weight.
** @@
******************************************************************************/

float ajSeqstrCalcMolwt(const AjPStr seq)
{
    /* source: Biochemistry LABFAX */
    static double aa[26] = {     89.10, 132.61, 121.16, 133.11, /* A-D */
				147.13, 165.19,  75.07, 155.16,	/* E-H */
				131.18,   0.00, 146.19, 131.18,	/* I-L */
				149.21, 132.12,   0.00, 115.13,	/* M-P */
				146.15, 174.20, 105.09, 119.12,	/* Q-T */
				  0.00, 117.15, 204.23, 128.16,	/* U-X */
				181.19, 146.64};
    float mw;
    ajint i;
    const char* cp;

    cp = ajStrGetPtr(seq);
    mw = (float) 18.015;
    
    while(*cp)
    {
	i = toupper((ajint) *cp)-'A';
	if(i > 25 || i < 0)
	{
	    ajDebug("seqMW bad character '%c' %d\n", *cp, *cp);
	    i = 'X' - 'A';
	}
	mw += (float) aa[i] - (float) 18.015;
	cp++;
    }

    ajDebug("seqMW calculated %.2f\n", mw);

    return mw;
}


/* @obsolete ajSeqMW
** @rename ajSeqstrCalcMolwt
*/
float __deprecated ajSeqMW(const AjPStr seq)
{
    return ajSeqstrCalcMolwt(seq);
}




/* @func ajSeqstrCalcCrc ******************************************************
**
** Calculates the SwissProt style CRC32 checksum for a protein sequence.
** This seems to be a bit reversal of a standard CRC32 checksum.
**
** @param [r] seq [const AjPStr] Sequence as a string
** @return [ajuint] CRC32 checksum.
** @@
******************************************************************************/

ajuint ajSeqstrCalcCrc(const AjPStr seq)
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

    cp = ajStrGetPtr(seq);

    crc = 0xFFFFFFFFL;
    while( *cp )
    {
	c = toupper((ajint) *cp);
	crc = ((crc >> 8) & 0x00FFFFFFL) ^ seqCrcTable[ (crc^c) & 0xFF ];
	cp++;
    }
    ajDebug("CRC32 calculated %08lX\n", crc);

    return (ajuint)crc;
}


/* @obsolete ajSeqCrc
** @rename ajSeqstrCalcCrc
*/
ajuint __deprecated ajSeqCrc(const AjPStr seq)
{
    return ajSeqstrCalcCrc(seq);
}


/* @func ajSeqGarbageOn *******************************************************
**
** Sets Garbage element of a Seq object to True.
**
** @param [u] thys [AjPSeq *] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqGarbageOn(AjPSeq *thys)
{
    (*thys)->Garbage = ajTrue;
    
    return;
}




/* @func ajSeqGarbageOff ******************************************************
**
** Sets Garbage element of a Seq object to False.
**
** @param [u] thys [AjPSeq *] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqGarbageOff(AjPSeq *thys)
{
    (*thys)->Garbage = ajFalse;
    
    return;
}





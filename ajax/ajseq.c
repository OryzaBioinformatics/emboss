/********************************************************************
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
********************************************************************/

#include "ajax.h"
#include <limits.h>

static unsigned long seqCrcTable[256];

static void       seqCrcGen( void );


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

AjPSeqall ajSeqallNew (void) {

  AjPSeqall pthis;

  AJNEW0(pthis);

  pthis->Seq = ajSeqNew();
  pthis->Seqin = ajSeqinNew ();
  pthis->Count = 0;

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

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Stream Assignments **************************************
**
** These functions overwrite the sequence stream object provided as
** the first argument.
**
******************************************************************************/

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Stream Modifiers ****************************************
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

void ajSeqallToLower (AjPSeqall seqall) {
  (void) ajSeqToLower(seqall->Seq);

  return;
}

/* @func ajSeqallToUpper ******************************************************
**
** Convertes the latest sequence in a stream to upper case.
**
** @param [P] seqall [AjPSeqall] Sequence stream object
** @return [void]
** @@
******************************************************************************/

void ajSeqallToUpper (AjPSeqall seqall) {
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

void ajSeqallReverse (AjPSeqall thys) {
  int ibegin = thys->Begin;
  int iend = thys->End;

  ajDebug ("ajSeqallReverse len: %d Begin: %d End: %d\n",
	   ajSeqallLen(thys), thys->Begin, thys->End);

  if (ibegin)
    thys->End = -(ibegin);
  if (iend)
    thys->Begin = -(iend);

  (void) ajSeqReverse(thys->Seq);

  ajDebug ("  all result len: %d Begin: %d End: %d\n",
	   ajSeqallLen(thys), thys->Begin, thys->End);

  return;
}

/* @func ajSeqallSetRange ****************************************************
**
** Sets the start and end positions for a sequence stream.
**
** @param [P] seq [AjPSeqall] Sequence stream object to be set.
** @param [r] ibegin [int] Start position. Negative values are from the end.
** @param [r] iend [int] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqallSetRange (AjPSeqall seq, int ibegin, int iend) {

  ajDebug ("ajSeqallSetRange (len: %d %d, %d)\n",
	   ajSeqLen(seq->Seq), ibegin, iend);
  if (ibegin) {
    seq->Begin = seq->Seq->Begin = ibegin;

  }
  if (iend) {
    seq->End = seq->Seq->End = iend;
  }

  ajDebug ("      result: (len: %d %d, %d)\n",
	   ajSeqLen(seq->Seq), seq->Begin, seq->End);

  return;
}

/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Sequence Stream Casts ********************************************
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
** @return [int] sequence length.
** @@
******************************************************************************/

int ajSeqallLen (AjPSeqall seqall) {
  return ajSeqLen(seqall->Seq);
}

/* @func ajSeqallBegin ********************************************************
**
** Returns the sequence stream start position, or 1 if no start has been set.
**
** @param [P] seq [AjPSeqall] Sequence stream object
** @return [int] Start position.
** @@
******************************************************************************/

int ajSeqallBegin (AjPSeqall seq) {
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
** @return [int] Start position.
** @@
******************************************************************************/

int ajSeqallEnd (AjPSeqall seq) {

  if (!seq->End)
    return ajSeqLen(seq->Seq);

  return ajSeqPosI(seq->Seq, ajSeqallBegin(seq), seq->End);
}

/* @func ajSeqallGetRange **************************************************
**
** Returns the sequence range for a sequence stream
**
** @param [r] thys [AjPSeqall] Sequence stream object.
** @param [r] begin [int*] Sequence range begin
** @param [r] end [int*] Sequence range end
** @return [int] Sequence range length
** @@
******************************************************************************/

int ajSeqallGetRange (AjPSeqall thys, int* begin, int* end) {
  ajDebug ("ajSeqallGetRange '%S'\n", thys->Seq->Name);

  return ajSeqGetRange(thys->Seq, begin, end);
}

/* @func ajSeqsetGetRange **************************************************
**
** Returns the sequence range for a sequence set
**
** @param [r] thys [AjPSeqset] Sequence set object.
** @param [r] begin [int*] Sequence range begin
** @param [r] end [int*] Sequence range end
** @return [int] Sequence range length
** @@
******************************************************************************/

int ajSeqsetGetRange (AjPSeqset thys, int* begin, int* end) {
  ajDebug ("ajSeqsetGetRange '%S' begin %d end %d len: %d\n",
	   thys->Name, thys->Begin, thys->End, thys->Len);
  *begin = ajSeqPosII(thys->Len, 1, thys->Begin);
  if (thys->End)
    *end = ajSeqPosII(thys->Len, *begin, thys->End);
  else
    *end = ajSeqPosII(thys->Len, *begin, thys->Len);

  return (*end - *begin + 1);
}

/* @func ajSeqGetRange **************************************************
**
** Returns the sequence range for a sequence.
**
** @param [r] thys [AjPSeq] Sequence object.
** @param [w] begin [int*] Sequence range begin
** @param [w] end [int*] Sequence range end
** @return [int] Sequence range length
** @@
******************************************************************************/

int ajSeqGetRange (AjPSeq thys, int* begin, int* end) {
  ajDebug ("ajSeqGetRange '%S'\n", thys->Name);
  *begin = ajSeqPos(thys, thys->Begin);
  if (thys->End)
    *end = ajSeqPosI(thys, *begin, thys->End);
  else
    *end = ajSeqPosI(thys, *begin, ajSeqLen(thys));

  return (*end - *begin + 1);
}

/* @func ajSeqallGetName *****************************************************
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

AjPStr ajSeqallGetName (AjPSeqall thys) {
  ajDebug ("ajSeqallGetName '%S'\n", thys->Seqin->Name);

  return thys->Seqin->Name;
}

/* @func ajSeqallGetNameSeq **************************************************
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

AjPStr ajSeqallGetNameSeq (AjPSeqall thys) {
  ajDebug ("ajSeqallGetNameSeq '%S'\n", thys->Seq->Name);

  return ajSeqGetName(thys->Seq);
}

/* @func ajSeqGetUsa *****************************************************
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

AjPStr ajSeqGetUsa (AjPSeq thys) {
  ajDebug ("ajSeqGetUsa '%S'\n", thys->Usa);

  if (!ajStrLen(thys->Usa))
    ajSeqMakeUsa (thys, NULL);
  return thys->Usa;
}

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Sequence Set Constructors ****************************************
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

AjPSeqset ajSeqsetNew (void) {

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

void ajSeqsetToLower (AjPSeqset seqset) {
  int i;

  for (i=0; i < seqset->Size; i++) {
    (void) ajSeqToLower (seqset->Seq[i]);
  }

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

void ajSeqsetToUpper (AjPSeqset seqset) {
  int i;

  for (i=0; i < seqset->Size; i++) {
    (void) ajSeqToUpper (seqset->Seq[i]);
  }

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

void ajSeqsetReverse (AjPSeqset thys) {
  int i;
  int ibegin = thys->Begin;
  int iend = thys->End;

  ajDebug ("ajSeqsetReverse len: %d Begin: %d End: %d\n",
	   ajSeqsetLen(thys), thys->Begin, thys->End);

  if (ibegin)
    thys->End = -(ibegin);
  if (iend)
    thys->Begin = -(iend);

  for (i=0; i < thys->Size; i++) {
    (void) ajSeqReverse (thys->Seq[i]);
  }

  ajDebug ("  set result len: %d Begin: %d End: %d\n",
	   ajSeqsetLen(thys), thys->Begin, thys->End);

  return;
}

/* @func ajSeqsetSetRange ****************************************************
**
** Sets the start and end positions for a sequence set.
**
** @param [P] seq [AjPSeqset] Sequence set object to be set.
** @param [r] ibegin [int] Start position. Negative values are from the end.
** @param [r] iend [int] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetRange (AjPSeqset seq, int ibegin, int iend) {
  int i;

  ajDebug ("ajSeqsetSetRange (len: %d %d, %d)\n", seq->Len, ibegin, iend);

  if (ibegin) {
    seq->Begin = ibegin;

  }
  if (iend) {
    seq->End = iend;
  }

  for (i=0; i< seq->Size; i++) {
    if (ibegin)
      seq->Seq[i]->Begin = ibegin;
    if (iend)
      seq->Seq[i]->End   = iend;
  }

  ajDebug ("      result: (len: %d %d, %d)\n",
	   seq->Len, seq->Begin, seq->End);

  return;
}

/* @func ajSeqsetFill ****************************************************
**
** Fills a sequence set with gaps at the ends of any shorter sequences.
**
** @param [P] seq [AjPSeqset] Sequence set object to be set.
** @return [int] Number of gaps inserted
** @@
******************************************************************************/

int ajSeqsetFill (AjPSeqset seq) {
  int i;
  int ifix = 0;
  int nfix = 0;
  int ilen;

  ajDebug ("ajSeqsetFill (len: %d)\n", seq->Len);

  for (i=0; i< seq->Size; i++) {
    if (ajSeqLen(seq->Seq[i]) < seq->Len) {
      nfix++;
      ilen = seq->Len - ajSeqLen(seq->Seq[i]);
      if (ilen > ifix)
	ifix = ilen;
      ajStrFill (&seq->Seq[i]->Seq, seq->Len, '-');
    }
  }

  ajDebug ("      result: (len: %d added: %d number of seqs fixed: nfix\n",
	   seq->Len, ifix, nfix);

  return ifix;
}

/* @func ajSeqsetIsNuc *************************************************
**
** Tests whether a sequence set is nucleotide.
**
** @param [P] thys [AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a nucleotide sequence set.
** @@
******************************************************************************/

AjBool ajSeqsetIsNuc (AjPSeqset thys) {

  AjPSeq seq;

  if (ajStrMatchC(thys->Type, "N"))
     return ajTrue;

  seq = thys->Seq[0];
  if (!ajSeqTypeGapnuc(seq))
    return ajTrue;

  return ajFalse;
}

/* @func ajSeqsetIsProt ****************************************************
**
** Tests whether a sequence set is protein.
**
** @param [P] thys [AjPSeqset] Sequence set
** @return [AjBool] ajTrue for a protein sequence set.
** @@
******************************************************************************/

AjBool ajSeqsetIsProt (AjPSeqset thys) {

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

/* @section Sequence Set Casts ********************************************
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
** @return [int] sequence set length.
** @@
******************************************************************************/

int ajSeqsetLen (AjPSeqset seq) {

  return seq->Len;
}

/* @func ajSeqsetBegin ********************************************************
**
** Returns the sequence set start position, or 1 if no start has been set.
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [int] Start position.
** @@
******************************************************************************/

int ajSeqsetBegin (AjPSeqset seq) {

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
** @return [int] Start position.
** @@
******************************************************************************/

int ajSeqsetEnd (AjPSeqset seq) {

  if (!seq->End)
    return (seq->Len);

  return ajSeqPosII(seq->Len, ajSeqsetBegin(seq), seq->End);
}

/* @func ajSeqsetSeq *********************************************************
**
** Returns the sequence data of a sequence in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @param [r] i [int] Sequence index
** @return [char*] Sequence as a NULL terminated string.
** @@
******************************************************************************/

char* ajSeqsetSeq (AjPSeqset seq, int i) {
  if (i >= seq->Size) return NULL;

  return ajStrStr(seq->Seq[i]->Seq);
}

/* @func ajSeqsetSize *********************************************************
**
** Returns the number of sequences in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @return [int] sequence set size.
** @@
******************************************************************************/

int ajSeqsetSize (AjPSeqset seq) {

  return seq->Size;
}

/* @func ajSeqsetName *********************************************************
**
** Returns the name of a sequence in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @param [r] i [int] Sequence index
** @return [AjPStr] sequence name as a string.
** @@
******************************************************************************/

AjPStr ajSeqsetName (AjPSeqset seq, int i) {
  if (i >= seq->Size) return NULL;

  return seq->Seq[i]->Name;
}

/* @func ajSeqsetWeight *******************************************************
**
** Returns the weight of a sequence in a sequence set
**
** @param [P] seq [AjPSeqset] Sequence set object
** @param [r] i [int] Sequence index
** @return [float] sequence weight as a float.
** @@
******************************************************************************/

float ajSeqsetWeight (AjPSeqset seq, int i) {
  if (i >= seq->Size) return 0.0;

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

float ajSeqsetTotweight (AjPSeqset seq) {
  int i;
  float ret = 0.0;

  ajDebug("ajSeqsetTotweight Size %d\n", seq->Size);
  for (i=0; i < seq->Size; i++) {
    ret += seq->Seq[i]->Weight;
    ajDebug("seq %d weight %d\n", i, seq->Seq[i]->Weight);
  }

  return ret;
}

/* @func ajSeqsetGetName *****************************************************
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

AjPStr ajSeqsetGetName (AjPSeqset thys) {
  ajDebug ("ajSeqsetGetName '%S'\n", thys->Name);

  return thys->Name;
}

/* @func ajSeqsetGetSeq *****************************************************
**
** Returns one sequence from a sequence set.
** Because this is a pointer to the real internal sequence
** the caller must take care not to change the data in any way.
** If the sequence is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeqset] Sequence set object.
** @param [r] i [int] Sequence index number in set
** @return [AjPSeq] Sequence object.
** @@
******************************************************************************/

AjPSeq ajSeqsetGetSeq (AjPSeqset thys, int i) {
  ajDebug ("ajSeqsetGetSeq '%S' %d/%d\n", thys->Name,i, thys->Size);
  if (i >= thys->Size) return NULL;

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

AjPSeq ajSeqNew (void) {

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

AjPSeq ajSeqNewL (size_t size) {

  AjPSeq pthis;

  AJNEW0(pthis);

  pthis->Name = ajStrNew();
  pthis->Acc = ajStrNew();
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

AjPSeq ajSeqNewS (AjPSeq seq) {

  AjPSeq pthis;

  AJNEW0(pthis);

  (void) ajStrAssS(&pthis->Name, seq->Name);
  (void) ajStrAssS(&pthis->Acc, seq->Acc);
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

  pthis->Rev = seq->Rev;
  pthis->EType = seq->EType;
  pthis->Format = seq->Format;
  pthis->Begin = seq->Begin;
  pthis->End = seq->End;
  pthis->Weight = seq->Weight;
  ajListstrDel(&pthis->Acclist);
  pthis->Acclist = ajListstrNew();

  return pthis;
}

/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Sequence Destructors *********************************************
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

void ajSeqDel (AjPSeq* pthis) {

  AjPSeq thys = pthis ? *pthis : 0;
  AjPStr ptr=NULL;
  AjPFeatLexicon dict=NULL;
  
  if (!pthis) return;
  if (!*pthis) return;

  ajStrDel (&thys->Name);
  ajStrDel (&thys->Acc);
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
  ajStrDel (&thys->Seq);

  if(thys->Fttable)
  {
      dict = ajFeatTableDict(thys->Fttable);
      ajFeatDeleteDict(dict);
      ajFeatTabDel(&thys->Fttable);
  }

  while(ajListstrPop(thys->Acclist,&ptr))
      ajStrDel(&ptr);
  ajListDel(&thys->Acclist);
  
/*  ajListstrDel(&thys->Acclist);*/

  AJFREE (*pthis);
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

void ajSeqClear (AjPSeq thys) {

  (void) ajStrClear (&thys->Name);
  (void) ajStrClear (&thys->Acc);
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
  (void) ajStrClear (&thys->Seq);
  
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

/* @func ajSeqAssName ********************************************************
**
** Assigns the sequence name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssName (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Name, str);
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

void ajSeqAssNameC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Name, text);

  return;
}

/* @func ajSeqAssAcc ********************************************************
**
** Assigns the sequence accession number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Accesion number as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssAcc (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Acc, str);

  return;
}

/* @func ajSeqAssAccC ********************************************************
**
** Assigns the sequence accession number.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] Accesion number as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssAccC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Acc, text);

  return;
}

/* @func ajSeqAssUfo ********************************************************
**
** Assigns the sequence feature full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] UFO as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUfo (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Ufo, str);

  return;
}

/* @func ajSeqAssUfoC ********************************************************
**
** Assigns the sequence feature name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] UFO as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUfoC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Ufo, text);

  return;
}

/* @func ajSeqAssUsa ********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] USA as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUsa (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Usa, str);

  return;
}

/* @func ajSeqAssUsaC ********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] USA as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssUsaC (AjPSeq thys, char* text) {

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

void ajSeqAssEntry (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Entryname, str);

  return;
}

/* @func ajSeqAssEntryC ******************************************************
**
** Assigns the sequence entryname.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] Entry name as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssEntryC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Entryname, text);

  return;
}

/* @func ajSeqAssFull ********************************************************
**
** Assigns the sequence full name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] Full name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFull (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Full, str);

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

void ajSeqAssFullC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Full, text);

  return;
}

/* @func ajSeqAssFile ********************************************************
**
** Assigns the sequence file name.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] File name as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssFile (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Filename, str);

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

void ajSeqAssFileC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Filename, text);

  return;
}
/* @func ajSeqAssSeq ********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] New sequence as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeq (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Seq, str);
  thys->Begin = 0;
  thys->End = 0;
  thys->Rev = ajFalse;

  return;
}

/* @func ajSeqAssSeqC ********************************************************
**
** Assigns a modified sequence to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] text [char*] New sequence as a C character string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssSeqC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Seq, text);

  return;
}

/* @func ajSeqAssDesc ********************************************************
**
** Assigns a modified description to an existing AjPSeq sequence.
**
** @param [u] thys [AjPSeq] Sequence object.
** @param [r] str [AjPStr] New description as a string.
** @return [void]
** @@
******************************************************************************/

void ajSeqAssDesc (AjPSeq thys, AjPStr str) {

  (void) ajStrAss(&thys->Desc, str);

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

void ajSeqAssDescC (AjPSeq thys, char* text) {

  (void) ajStrAssC(&thys->Desc, text);

  return;
}

/* @func ajSeqMod ***********************************************************
**
** Makes a sequence modifiable by making sure there is no duplicate
** copy of the sequence.
**
** @param [uP] thys [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqMod (AjPSeq thys) {

  (void) ajStrMod (&thys->Seq);

  return;
}

/* @func ajSeqReplace *******************************************************
**
** Replaces a sequence with the contents of a string.
**
** @param [uP] thys [AjPSeq] Sequence
** @param [P] seq [AjPStr] New sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReplace (AjPSeq thys, AjPStr seq) {

  (void) ajStrAss (&thys->Seq, seq);
  thys->Begin = 0;
  thys->End = 0;
  thys->Rev = ajFalse;
  return;
}

/* @func ajSeqReplaceC *******************************************************
**
** Replaces a sequence with the contents of a C text string.
**
** @param [uP] thys [AjPSeq] Sequence
** @param [P] seq [char*] New sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqReplaceC (AjPSeq thys, char* seq) {

  (void) ajStrAssC (&thys->Seq, seq);
  thys->Begin = 0;
  thys->End = 0;
  thys->Rev = ajFalse;
  return;
}

/* @func ajSeqSetRange *******************************************************
**
** Sets the start and end positions for a sequence (not for a sequence set).
**
** @param [P] seq [AjPSeq] Sequence object to be set.
** @param [r] ibegin [int] Start position. Negative values are from the end.
** @param [r] iend [int] End position. Negative values are from the end.
** @return [void]
** @@
******************************************************************************/

void ajSeqSetRange (AjPSeq seq, int ibegin, int iend) {

  ajDebug ("ajSeqSetRange (len: %d %d, %d)\n", ajSeqLen(seq), ibegin, iend);
  if (ibegin) {
    seq->Begin = ibegin;

  }
  if (iend) {
    seq->End = iend;
  }

  ajDebug ("      result: (len: %d %d, %d)\n",
	   ajSeqLen(seq), seq->Begin, seq->End);

  return;
}

/* @func ajSeqMakeUsa *******************************************************
**
** Sets the USA for a sequence.
**
** @param [P] thys [AjPSeq] Sequence object to be set.
** @param [P] seqin [AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqMakeUsa (AjPSeq thys, AjPSeqin seqin) {

  ajDebug ("ajSeqMakeUsa (Name <%S> Formatstr <%S> Db <%S> "
	   "Entryname <%S> Filename <%S>)\n",
	   thys->Name, thys->Formatstr, thys->Db,
	   thys->Entryname, thys->Filename);

  ajSeqTrace (thys);
  if (seqin) ajSeqinTrace (seqin);
  if (ajStrLen(thys->Db)) {
    ajFmtPrintS (&thys->Usa, "%S-id:%S", thys->Db, thys->Entryname);
  }
  else {
    /*ajFmtPrintS (&thys->Usa, "%S::%S (%S)",
      thys->Formatstr, thys->Filename, thys->Entryname);*/
    ajFmtPrintS (&thys->Usa, "%S::%S", thys->Formatstr, thys->Filename);
  }

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

void ajSeqToUpper (AjPSeq thys) {
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

void ajSeqToLower (AjPSeq thys) {
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

void ajSeqReverse (AjPSeq thys) {

  int ibegin = thys->Begin;
  int iend = thys->End;

  ajDebug ("ajSeqReverse len: %d Begin: %d End: %d\n",
	   ajSeqLen(thys), thys->Begin, thys->End);

  if (ibegin)
    thys->End = -(ibegin);
  if (iend)
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

void ajSeqReverseStr (AjPStr* pthis) {
  char *cp;
  char *cq;
  char tmp;

  (void) ajStrMod (pthis);

  cp = ajStrStr(*pthis);
  cq = cp + ajStrLen(*pthis) - 1;

  while (cp < cq) {
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

void ajSeqCompOnly (AjPSeq thys) {

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

void ajSeqRevOnly (AjPSeq thys) {

  int ibegin = thys->Begin;
  int iend = thys->End;

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

void ajSeqCompOnlyStr (AjPStr* pthis) {

  char *cp;

  (void) ajStrMod (pthis);

  cp = ajStrStr(*pthis);

  while (*cp) {
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

char ajSeqBaseComp (char base) {
  static char fwd[]="ACGTURYWSMKBDHVNXacgturywsmkbdhvnx";
  static char rev[]="TGCAAYRWSKMVHDBNXtgcaayrwskmvhdbnx";
  char *cp;
  char *cq;

  cp = strchr (fwd,base);
  if (cp) {
    cq = cp - fwd + rev;
    return *cq;
  }
  return base;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Operators ********************************************
**
** These functions use the contents of a sequence but do not make any changes.
**
******************************************************************************/

/* @func ajSeqIsNuc ****************************************************
**
** Tests whether a sequence is nucleotide.
**
** @param [P] thys [AjPSeq] Sequence
** @return [AjBool] ajTrue for a nucleotide sequence.
** @@
******************************************************************************/

AjBool ajSeqIsNuc (AjPSeq thys) {

  ajDebug ("ajSeqIsNuc Type '%S'\n", thys->Type);

  if (ajStrMatchC(thys->Type, "N"))
     return ajTrue;

  if (ajStrMatchC(thys->Type, "P"))
     return ajFalse;

  if (!ajSeqTypeGapnuc(thys))	/* returns char 0 on success */
    return ajTrue;

  ajDebug ("ajSeqIsNuc failed\n", thys->Type);

  return ajFalse;
}

/* @func ajSeqIsProt *******************************************************
**
** Tests whether a sequence is protein.
**
** @param [P] thys [AjPSeq] Sequence
** @return [AjBool] ajTrue for a protein sequence.
** @@
******************************************************************************/

AjBool ajSeqIsProt (AjPSeq thys) {

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
** The current definition is one or two alpha characters, followed
** by a string of digits and a minimum length of 6.
**
** This may need evision in future is Swiss-Prot adopts its planned
** new accession number format.
**
** @param [P] accnum [AjPStr] String to be tested
** @return [AjBool] ajTrue if the string is a possible accession number.
** @@
******************************************************************************/

AjBool ajIsAccession (AjPStr accnum) {

  int i = ajStrLen(accnum);
  char *cp = ajStrStr(accnum);

  if (i < 6)
    return ajFalse;

  if (!isalpha((int)*cp++))
    return ajFalse;
  if (isalpha((int)*cp))
    cp++;

  while (*cp) {
    if (!isdigit((int)*cp++))
      return ajFalse;
  }

  return ajTrue;
}


/* @func ajSeqTrace ***********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

void ajSeqTrace (AjPSeq seq) {
  AjIList it;
  AjPStr cur;
  ajDebug ("Sequence trace\n");
  ajDebug ( "==============\n\n");
  ajDebug ( "  Name: '%S'\n", seq->Name);
  if (ajStrLen(seq->Acc))
    ajDebug ( "  Accession: '%S'\n", seq->Acc);
  if (ajListLength(seq->Acclist)) {
    ajDebug ( "  Acclist: (%d) '", ajListLength(seq->Acclist));
    it = ajListIter(seq->Acclist);
    while ((cur = (AjPStr) ajListIterNext(it))) {
      ajDebug(" %S", cur);
    }
    ajListIterFree (it);
    ajDebug(" '\n");
  }
  if (ajStrLen(seq->Type))
    ajDebug ( "  Type: '%S' (%d)\n", seq->Type, seq->EType);
  if (ajStrLen(seq->Desc))
    ajDebug ( "  Description: '%S'\n", seq->Desc);
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
}

/* @func ajSeqinTrace *********************************************************
**
** Debug calls to trace the data in a sequence input object.
**
** @param [r] thys [AjPSeqin] Sequence input object.
** @return [void]
** @@
******************************************************************************/

void ajSeqinTrace (AjPSeqin thys) {

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
    ajDebug ( "  Inseq: '%S'\n", thys->Inseq);
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
    ajDebug ( "  Input format: '%S' (%d)\n", thys->Formatstr, thys->Format);
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
}

/* @func ajSeqQueryTrace ******************************************************
**
** Debug calls to trace the data in a sequence query object.
**
** @param [r] thys [AjPSeqQuery] Sequence query object.
** @return [void]
** @@
******************************************************************************/

void ajSeqQueryTrace (AjPSeqQuery thys) {

  ajDebug ( "  Query Trace\n");
  if (ajStrLen(thys->DbName))
    ajDebug ( "    DbName: '%S'\n", thys->DbName);
  if (ajStrLen(thys->DbType))
    ajDebug ( "    DbType: '%S' (%d)\n", thys->DbType, thys->Type);
  if (ajStrLen(thys->Id))
    ajDebug ( "    Id: '%S'\n", thys->Id);
  if (ajStrLen(thys->Acc))
    ajDebug ( "    Acc: '%S'\n", thys->Acc);
  if (ajStrLen(thys->Des))
    ajDebug ( "    Des: '%S'\n", thys->Des);
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

/* @section Sequence Casts ********************************************
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
** @return [int] Start position.
** @@
******************************************************************************/

int ajSeqBegin (AjPSeq seq) {

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
** @return [int] Start position.
** @@
******************************************************************************/

int ajSeqEnd (AjPSeq seq) {

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

char* ajSeqName (AjPSeq seq) {
  return ajStrStr(seq->Name);
}

/* @func ajSeqOffset *************************************************************
**
** Returns the sequence offset from -sbegin originally.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [int] Sequence offset.
** @@
******************************************************************************/

int ajSeqOffset (AjPSeq seq) {
  return seq->Offset;
}
/* @func ajSeqOffend *************************************************************
**
** Returns the sequence offend value. Len choped off from -send originally.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [int] Sequence offend.
** @@
******************************************************************************/

int ajSeqOffend (AjPSeq seq) {
  return seq->Offend;
}

/* @func ajSeqLen *************************************************************
**
** Returns the sequence length.
**
** @param [P] seq [AjPSeq] Sequence object
** @return [int] Sequence length.
** @@
******************************************************************************/

int ajSeqLen (AjPSeq seq) {
  return ajStrLen(seq->Seq);
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

char* ajSeqCharCopy (AjPSeq seq) {
  return ajCharNew(seq->Seq);
}

/* @func ajSeqCharCopyL ******************************************************
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

char* ajSeqCharCopyL (AjPSeq seq, size_t size) {
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

AjBool ajSeqNum (AjPSeq thys, AjPSeqCvt cvt, AjPStr* numseq) {

  char *cp = ajSeqChar(thys);
  char *ncp;

  (void) ajStrAss (numseq, thys->Seq);
  ncp = ajStrStr(*numseq);

  while (*cp) {
    *ncp = cvt->table[(int)*cp];
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

void ajSeqCvtTrace (AjPSeqCvt cvt) {
  int i;
  ajDebug ("Cvt table for '%S'\n\n", cvt->bases);
  ajDebug ("index num ch\n");
  ajDebug ("----- --- --\n");
  for (i=0; i < cvt->size; i++) {
    if (cvt->table[i])
      ajDebug ("%5d %3d <%c>\n", i, cvt->table[i], ajSysItoC(i));
  }
  ajDebug ("... all others are zero ...\n", cvt->bases);
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

AjPSeqCvt ajSeqCvtNewZero (char* bases) {
  static AjPSeqCvt ret;
  int i;
  char *cp = bases;

  AJNEW0(ret);
  ret->len = strlen(bases);
  ret->size = CHAR_MAX - CHAR_MIN + 1;
  ret->table = AJCALLOC0(ret->size, sizeof(char));
  ret->bases = ajStrNewC (bases);
  ret->missing = 0;

  i = 0;
  while (*cp) {
    i++;
    ret->table[toupper((int) *cp)] = ajSysItoC(i);
    ret->table[tolower((int) *cp)] = ajSysItoC(i);
    cp++;
  }

  return ret;
}

/* @func ajSeqCvtNew ******************************************************
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

AjPSeqCvt ajSeqCvtNew (char* bases) {
  static AjPSeqCvt ret;
  int i;
  int j;
  int imax;
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
  while (*cp) {
    ret->table[toupper((int) *cp)] = ajSysItoC(i);
    ret->table[tolower((int) *cp)] = ajSysItoC(i);
    cp++;
    i++;
  }

  return ret;
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

AjPSeqCvt ajSeqCvtNewText (char* bases) {
  static AjPSeqCvt ret;
  int i;
  int j;
  char *cp = bases;
  char c;
  
  AJNEW0(ret);
  ret->len = strlen(bases);
  ret->size = CHAR_MAX - CHAR_MIN + 1;
  ret->table = AJCALLOC0(ret->size, sizeof(char));
  ret->bases = ajStrNewC (bases);
  ret->missing = -1;

  for (j=0; j < ret->size; j++) {
    if (isdigit(j))
      ret->table[j] = -1;
    else
      ret->table[j] = -2;
  }

  i = 0;
  while (*cp) {
      c = ajSysItoC(toupper((int)*cp));
      ret->table[toupper((int) *cp)] = c;
      ret->table[tolower((int) *cp)] = c;
      cp++;
      i++;
  }

  return ret;
}

/* @func ajSeqCvtLen *******************************************************
**
** Returns the length of a conversion table string (number of sequence
** characterers explicitly included)
**
** @param [r] thys [AjPSeqCvt] Conversion table
**
** @return [int] Length
** @@
******************************************************************************/

int ajSeqCvtLen (AjPSeqCvt thys) {

  return thys->len;
}

/* @func ajSeqCvtK *******************************************************
**
** Returns the integer code corresponding to a sequence character
** in a conversion tabkle
**
** @param [r] thys [AjPSeqCvt] Conversion table
** @param [r] ch [char] Sequence character
**
** @return [int] Conversion code
** @@
******************************************************************************/

int ajSeqCvtK (AjPSeqCvt thys, char ch) {

  return thys->table[(int)ch];
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

AjPStr ajSeqStr (AjPSeq thys) {
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

AjPStr ajSeqStrCopy (AjPSeq thys) {
  static AjPStr str;

  str= 0;
  (void) ajStrAss (&str, thys->Seq);

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

char* ajSeqChar (AjPSeq thys) {
  if (!thys)
    return "";

  return ajStrStr(thys->Seq);
}

/* @func ajSeqGetAcc *********************************************************
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

AjPStr ajSeqGetAcc (AjPSeq thys) {

  return thys->Acc;
}

/* @func ajSeqGetDesc ********************************************************
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

AjPStr ajSeqGetDesc (AjPSeq thys) {

  return thys->Desc;
}

/* @func ajSeqGetFeat ********************************************************
**
** Returns the sequence description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPSeq] Sequence object.
** @return [AjPFeatTable] feature table (if any)
** @@
******************************************************************************/

AjPFeatTable ajSeqGetFeat (AjPSeq thys) {

  return thys->Fttable;
}

/* @func ajSeqGetName ********************************************************
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

AjPStr ajSeqGetName (AjPSeq thys) {

  return thys->Name;
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

AjPSeqout ajSeqoutNew (void) {

  AjPSeqout pthis;

  AJNEW0(pthis);

  pthis->Name = ajStrNew();
  pthis->Acc = ajStrNew();
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

  pthis->Ftquery = ajFeatTabOutNew();
  pthis->Fttable = NULL;
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

void ajSeqoutDel (AjPSeqout* pthis) {

  AjPSeqout thys = *pthis;

  ajStrDel (&thys->Name);
  ajStrDel (&thys->Acc);
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

  ajListDel(&thys->Savelist);

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

float ajSeqMW (AjPStr seq) {
  static float aa[26] = { 089.10, 132.61, 121.16, 133.11, /* A-D */
			  147.13, 165.19, 075.07, 155.16, /* E-H */
			  131.18, 000.00, 146.19, 131.18, /* I-L */
			  149.21, 132.12, 000.00, 115.13, /* M-P */
			  146.15, 174.20, 105.09, 119.12, /* Q-T */
			  000.00, 117.15, 204.23, 128.16, /* U-X */
			  181.19, 146.64}; /* source: Biochemistry LABFAX */
  float mw = 18.015;
  int i;
  char* cp = ajStrStr(seq);

  while (*cp) {
    i = toupper((int) *cp)-'A';
    if (i > 25 || i < 0) {
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
** @return [unsigned int] CRC32 checksum.
** @@
******************************************************************************/

unsigned int ajSeqCrc (AjPStr seq) {
  register unsigned long crc;
  int     c;
  char* cp;
  static int calls = 0;

  if (!calls) {
    seqCrcGen ();
    calls = 1;
  }

  cp = ajStrStr(seq);

  crc = 0xFFFFFFFFL;
  while( *cp ) {
    c = toupper((int) *cp);
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

static void seqCrcGen (void) {
  unsigned long crc, poly;
  int     i, j;

  poly = 0xEDB88320L;
  for (i=0; i<256; i++) {
    crc = i;
    for (j=8; j>0; j--) {
      if (crc&1) {
	crc = (crc >> 1) ^ poly;
      }
      else {
	crc >>= 1;
      }
    }
    seqCrcTable[i] = crc;
  }
}

/* @func ajSeqCount ***********************************************************
**
** Counts the numbers of A, C, G and T in a nucleotide sequence.
**
** @param [P] thys [AjPStr] Sequence as a string
** @param [w] b [int*] integer array, minimum size 5, to hold the results.
** @return [void]
** @@
******************************************************************************/

void ajSeqCount (AjPStr thys, int* b) {

  char* cp = ajStrStr(thys);

  b[0] = b[1] = b[2] = b[3] = b[4] = 0;

  ajDebug ("ajSeqCount %d bases\n", ajStrLen(thys));
  while (*cp) {
    if (toupper((int) *cp) == 'A') b[0]++;
    if (toupper((int) *cp) == 'C') b[1]++;
    if (toupper((int) *cp) == 'G') b[2]++;
    if (toupper((int) *cp) == 'T') b[3]++;
    if (toupper((int) *cp) == 'U') b[3]++;
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

/* @func ajSeqCheckGcg *****************************************************
**
** Calculates a GCG checksum for a sequence.
**
** @param [r] thys [AjPSeq] Squence.
** @return [int] GCG checksum.
** @@
******************************************************************************/

int ajSeqCheckGcg (AjPSeq thys) {
  register long  i, check = 0, count = 0;
  char *cp = ajStrStr(thys->Seq);
  int ilen = ajStrLen(thys->Seq);

  for (i = 0; i < ilen; i++) {
    count++;
    check += count * toupper((int) cp[i]);
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
** @param [r] ipos [int] Position.
** @return [int] string position between 1 and length.
** @@
******************************************************************************/

int ajSeqPos (AjPSeq thys, int ipos) {

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
** @param [r] imin [int] Start position.
** @param [r] ipos [int] Position.
** @return [int] string position between 1 and length.
** @@
******************************************************************************/

int ajSeqPosI (AjPSeq thys, int imin, int ipos) {

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
** @param [r] ilen [int] maximum length.
** @param [r] imin [int] Start position.
** @param [r] ipos [int] Position.
** @return [int] string position between 1 and length.
** @@
******************************************************************************/

int ajSeqPosII (int ilen, int imin, int ipos) {
  int jpos;

  if (ipos < 0)
    jpos = ilen + ipos + 1;
  else {
    if (ipos)
      jpos = ipos;
    else
      jpos = 1;
  }

  if (jpos > ilen)
    jpos = ilen;

  if (jpos < imin)
    jpos = imin;

  ajDebug("ajSeqPosII(ilen: %d imin: %d ipos: %d) = %d\n",
	  ilen, imin, ipos, jpos);

  return jpos;
}

/* @func ajSeqTrim **************************************************
**
** Trim a sequence using the Begin and Ends.
**
** @param [rw] thys [AjPSeq] Sequence to be trimmed.
** @return [AjBool] Ajtrue returned if successfull.
** @@
******************************************************************************/

AjBool ajSeqTrim(AjPSeq thys){
  AjBool okay=ajTrue;

  ajDebug("Triming %d from %d to %d\n",thys->Seq->Len,thys->Begin,thys->End);

  if (thys->End){
    if(thys->End <= thys->Begin)
      return ajFalse;
    okay = ajStrTrim(&(thys->Seq),(0 - (thys->Seq->Len-(thys->End)) ));
    thys->Offend = thys->Seq->Len-(thys->End);
    thys->End = 0;
  }
  if(thys->Begin){
    okay = ajStrTrim(&thys->Seq,thys->Begin);
    thys->Offset = thys->Begin;
    thys->Begin =0;
  }
  ajDebug("After Triming len = %d\n",thys->Seq->Len);

  return okay;
}

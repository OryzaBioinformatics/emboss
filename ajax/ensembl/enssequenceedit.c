/* @source enssequenceedit ****************************************************
**
** Ensembl Sequence Edit functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.35 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enssequenceedit.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listSequenceeditCompareStartAscending(
    const void *item1,
    const void *item2);

static int listSequenceeditCompareStartDescending(
    const void *item1,
    const void *item2);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enssequenceedit ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPSequenceedit] Ensembl Sequence Edit **********************
**
** @nam2rule Sequenceedit Functions for manipulating
** Ensembl Sequence Edit objects
**
** @cc Bio::EnsEMBL::SeqEdit
** @cc CVS Revision: 1.8
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Sequence Edit by pointer.
** It is the responsibility of the user to first destroy any previous
** Sequence Edit. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPSequenceedit]
**
** @nam3rule New Constructor
** @nam4rule Attribute Constructor with an Ensembl Attribute object
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Attribute attribute [EnsPAttribute] Ensembl Attribute
** @argrule Cpy se [const EnsPSequenceedit] Ensembl Sequence Edit
** @argrule Ini at [EnsPAttributetype] Ensembl Attribute Type
** @argrule Ini sequence [AjPStr] Alternative sequence
** @argrule Ini start [ajuint] Start coordinate
** @argrule Ini end [ajuint] End coordinate
** @argrule Ref se [EnsPSequenceedit] Ensembl Sequence Edit
**
** @valrule * [EnsPSequenceedit] Ensembl Sequence Edit or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensSequenceeditNewAttribute ******************************************
**
** Constructor for an Ensembl Sequence Edit with an Ensembl Attribute.
**
** @cc Bio::EnsEMBL:SeqEdit::new
** @param [u] attribute [EnsPAttribute] Ensembl Attribute
**
** @return [EnsPSequenceedit] Ensembl Sequence Edit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSequenceedit ensSequenceeditNewAttribute(EnsPAttribute attribute)
{
    ajint start = 0;
    ajint end   = 0;

    AjPStr altseq = NULL;

    EnsPSequenceedit se = NULL;

    if (!attribute)
        return NULL;

    altseq = ajStrNew();

    ajFmtScanS(attribute->Value, "%d %d %S", &start, &end, &altseq);

    if (start > (end + 1))
    {
        ajDebug("ensSequenceeditNewAttribute start %d must be less than or "
                "equal to end %d + 1 in Ensembl Attribute value '%S'.\n",
                start, end, attribute->Value);

        return NULL;
    }

    if (start < 1)
    {
        ajDebug("ensSequenceeditNewAttribute start %d must be greater than or "
                "equal to 1 in Ensembl Attribute value '%S'.\n",
                start, attribute->Value);

        return NULL;
    }

    if (end < 0)
    {
        ajDebug("ensSequenceeditNewAttribute end %d must be greater than or "
                "equal to 0 in Ensembl Attribute value '%S' .\n",
                end, attribute->Value);

        return NULL;
    }

    AJNEW0(se);

    se->Attribute = ensAttributeNewRef(attribute);

    if (altseq)
        se->Sequence = ajStrNewRef(altseq);
    else
        se->Sequence = ajStrNew();

    se->Start = start;

    se->End = end;

    se->Use = 1U;

    ajStrDel(&altseq);

    return se;
}




/* @func ensSequenceeditNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [EnsPSequenceedit] Ensembl Sequence Edit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSequenceedit ensSequenceeditNewCpy(const EnsPSequenceedit se)
{
    EnsPSequenceedit pthis = NULL;

    AJNEW0(pthis);

    pthis->Attribute = ensAttributeNewRef(se->Attribute);

    if (se->Sequence)
        pthis->Sequence = ajStrNewRef(se->Sequence);

    pthis->Start = se->Start;

    pthis->End = se->End;

    pthis->Use = 1U;

    return pthis;
}




/* @func ensSequenceeditNewIni ************************************************
**
** Ensembl Sequence Edit constructor with initial values.
**
** @cc Bio::EnsEMBL:SeqEdit::new
** @param [u] at [EnsPAttributetype] Ensembl Attribute Type
** @param [u] sequence [AjPStr] Alternative sequence
** @param [r] start [ajuint] Start coordinate
** @param [r] end [ajuint] End coordinate
**
** @return [EnsPSequenceedit] Ensembl Sequence Edit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSequenceedit ensSequenceeditNewIni(EnsPAttributetype at,
                                       AjPStr sequence,
                                       ajuint start,
                                       ajuint end)
{
    AjPStr value = NULL;

    EnsPSequenceedit se = NULL;

    if (!sequence)
    {
        ajDebug("ensSequenceeditNewIni requires an alternate sequence.\n");

        return NULL;
    }

    if (start > (end + 1))
    {
        ajDebug("ensSequenceeditNewIni start %d must be less than or "
                "equal to end %d + 1.\n", start, end);

        return NULL;
    }

    if (start < 1)
    {
        ajDebug("ensSequenceeditNewIni start %d must be greater than or "
                "equal to 1.\n", start);

        return NULL;
    }

    value = ajFmtStr("%u %u %S", start, end, sequence);

    AJNEW0(se);

    se->Attribute = ensAttributeNewIni(at, value);

    if (sequence)
        se->Sequence = ajStrNewRef(sequence);
    else
        se->Sequence = ajStrNew();

    se->Start = start;
    se->End   = end;
    se->Use   = 1U;

    ajStrDel(&value);

    return se;
}




/* @func ensSequenceeditNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] se [EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [EnsPSequenceedit] Ensembl Sequence Edit or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPSequenceedit ensSequenceeditNewRef(EnsPSequenceedit se)
{
    if (!se)
        return NULL;

    se->Use++;

    return se;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Sequence Edit object.
**
** @fdata [EnsPSequenceedit]
**
** @nam3rule Del Destroy (free) an Ensembl Sequence Edit
**
** @argrule * Pse [EnsPSequenceedit*] Ensembl Sequence Edit address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensSequenceeditDel ***************************************************
**
** Default destructor for an Ensembl Sequence Edit.
**
** @param [d] Pse [EnsPSequenceedit*] Ensembl Sequence Edit address
**
** @return [void]
**
** @release 6.4.0
** @@
******************************************************************************/

void ensSequenceeditDel(EnsPSequenceedit *Pse)
{
    EnsPSequenceedit pthis = NULL;

    if (!Pse)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensSequenceeditDel"))
    {
        ajDebug("ensSequenceeditDel\n"
                "  *Pse %p\n",
                *Pse);

        ensSequenceeditTrace(*Pse, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pse) || --pthis->Use)
    {
        *Pse = NULL;

        return;
    }

    ensAttributeDel(&pthis->Attribute);

    ajStrDel(&pthis->Sequence);

    ajMemFree((void **) Pse);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Sequence Edit object.
**
** @fdata [EnsPSequenceedit]
**
** @nam3rule Get Return Sequence Edit attribute(s)
** @nam4rule Attribute Return the Ensembl Attribute
** @nam4rule Sequence Return the alternate sequence
** @nam4rule Start Return the description
** @nam4rule End Return the value
**
** @argrule * se [const EnsPSequenceedit] Sequence Edit
**
** @valrule Attribute [EnsPAttribute] Ensembl Attribute or NULL
** @valrule Sequence [AjPStr] Alternate sequence or NULL
** @valrule Start [ajuint] Start coordinate or 0U
** @valrule End [ajuint] End coordinate or 0U
**
** @fcategory use
******************************************************************************/




/* @func ensSequenceeditGetAttribute ******************************************
**
** Get the Ensembl Attribute member of an Ensembl Sequence Edit.
**
** @cc Bio::EnsEMBL:SeqEdit::get_Attribute
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [EnsPAttribute] Ensembl Attribute or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAttribute ensSequenceeditGetAttribute(const EnsPSequenceedit se)
{
    return (se) ? se->Attribute : NULL;
}




/* @func ensSequenceeditGetEnd ************************************************
**
** Get the end coordinate member of an Ensembl Sequence Edit.
**
** Coordinates are inclusive and one-based, which means that inserts are
** unusually represented by a start one base pair higher than the end. Hence,
** start = 1, end = 1 is a replacement of the first base, but
** start = 1, end = 0 is an insert BEFORE the first base.
**
** @cc Bio::EnsEMBL:SeqEdit::end
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [ajuint] End coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensSequenceeditGetEnd(const EnsPSequenceedit se)
{
    return (se) ? se->End : 0U;
}




/* @func ensSequenceeditGetSequence *******************************************
**
** Get the (alternative) sequence member of an Ensembl Sequence Edit.
**
** The sequence may either be a string of amino acids or nucleotides depending
** on the context in which this Sequence Edit is used.
** In the case of a deletion the replacement sequence is an empty string.
**
** @cc Bio::EnsEMBL:SeqEdit::alt_seq
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [AjPStr] Alternative sequence or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensSequenceeditGetSequence(const EnsPSequenceedit se)
{
    return (se) ? se->Sequence : NULL;
}




/* @func ensSequenceeditGetStart **********************************************
**
** Get the start coordinate member of an Ensembl Sequence Edit.
**
** Coordinates are inclusive and one-based, which means that inserts are
** unusually represented by a start one base pair higher than the end. Hence,
** start = 1, end = 1 is a replacement of the first base, but
** start = 1, end = 0 is an insert BEFORE the first base.
**
** @cc Bio::EnsEMBL:SeqEdit::start
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [ajuint] Start coordinate or 0U
**
** @release 6.4.0
** @@
******************************************************************************/

ajuint ensSequenceeditGetStart(const EnsPSequenceedit se)
{
    return (se) ? se->Start : 0U;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Sequence Edit object.
**
** @fdata [EnsPSequenceedit]
**
** @nam3rule Trace Report Ensembl Sequence Edit members to debug file
**
** @argrule Trace se [const EnsPSequenceedit] Ensembl Sequence Edit
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceeditTrace *************************************************
**
** Trace an Ensembl Sequence Edit.
**
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSequenceeditTrace(const EnsPSequenceedit se, ajuint level)
{
    AjPStr indent = NULL;

    if (!se)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensSequenceeditTrace %p\n"
            "%S  Attribute %p\n"
            "%S  Sequence '%S'\n"
            "%S  Start %u\n"
            "%S  End %u\n"
            "%S  Use %u\n",
            indent, se,
            indent, se->Attribute,
            indent, se->Sequence,
            indent, se->Start,
            indent, se->End,
            indent, se->Use);

    ensAttributeTrace(se->Attribute, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an Ensembl Sequence Edit object.
**
** @fdata [EnsPSequenceedit]
**
** @nam3rule Calculate Calculate Ensembl Sequence Edit information
** @nam4rule Difference Calculate the length difference
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @valrule Difference [ajint] Length difference or 0
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceeditCalculateDifference ***********************************
**
** Calculate the length difference an Ensembl Sequence Edit would cause.
**
** @cc Bio::EnsEMBL:SeqEdit::length_diff
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [ajint] Length difference or 0
**
** @release 6.4.0
** @@
******************************************************************************/

ajint ensSequenceeditCalculateDifference(const EnsPSequenceedit se)
{
    const char *Ptr = NULL;

    register ajuint i = 0U;

    if (!se)
        return 0;

    /*
    ** NOTE: Since ajStrGetLen returns size_t, which exceeds ajint,
    ** the length of the alternative sequence needs to be determined here.
    **
    ** return ajStrGetLen(se->Sequence) - (se->End - se->Start + 1);
    */

    for (i = 0U, Ptr = ajStrGetPtr(se->Sequence); (Ptr && *Ptr); i++, Ptr++)
        if (i == UINT_MAX)
            ajFatal("ensSequenceeditCalculateDifference exeeded UINT_MAX.");

    return i - (se->End - se->Start + 1);
}




/* @func ensSequenceeditCalculateMemsize **************************************
**
** Calculate the memory size in bytes of an Ensembl Sequence Edit.
**
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensSequenceeditCalculateMemsize(const EnsPSequenceedit se)
{
    size_t size = 0;

    if (!se)
        return 0;

    size += sizeof (EnsOSequenceedit);

    size += ensAttributeCalculateMemsize(se->Attribute);

    if (se->Sequence)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(se->Sequence);
    }

    return size;
}




/* @section apply *************************************************************
**
** Functions for applying Ensembl Sequence Edit objects.
**
** @fdata [EnsPSequenceedit]
**
** @nam3rule Apply Apply Ensembl Sequence Edit objects
** @nam4rule String Apply an Ensembl Sequence Edit to an AJAX String
**
** @argrule String se [const EnsPSequenceedit] Ensembl Sequence Edit
** @argrule String offset [ajint] Offset into sequence
** @argrule String Psequence [AjPStr*] Sequence address
**
** @valrule String [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensSequenceeditApplyString *******************************************
**
** Apply an Ensembl Sequence Edit to an AJAX String.
**
** @cc Bio::EnsEMBL:SeqEdit::apply_edit
** @cc Bio::EnsEMBL::DBSQL::SequenceAdaptor::_rna_edit
** @param [r] se [const EnsPSequenceedit] Ensembl Sequence Edit
** @param [rE] offset [ajint] Offset into sequence
** @param [u] Psequence [AjPStr*] Sequence address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensSequenceeditApplyString(const EnsPSequenceedit se,
                                  ajint offset,
                                  AjPStr *Psequence)
{
    ajlong pos1 = 0;
    ajlong pos2 = 0;

    if (!se)
        return ajFalse;

    if (!Psequence)
        return ajFalse;

    if (!*Psequence)
        return ajFalse;

    pos1 = se->Start - offset;
    pos2 = se->End   - offset;

    if (pos2 < 1)
    {
        ajDebug("ensSequenceeditApplyString got an Ensembl Sequence Edit, "
                "which end position (%d) corrected for the offset (%d) lies "
                "beyond the sequence start (1).",
                se->End, offset);

        return ajFalse;
    }

    if (pos1 > (ajlong) ajStrGetLen(*Psequence))
    {
        ajDebug("ensSequenceeditApplyString got an Ensembl Sequence Edit, "
                "which start position (%d) corrected for the offset (%d) lies "
                "beyond the sequence end (%lu).",
                se->Start, offset, ajStrGetLen(*Psequence));
        /* FIXME: size_t can be shorter than ajulong */

        return ajFalse;
    }

    /* Adjust to zero-based coordinates. */

    ajStrCutRange(Psequence, pos1 - 1, pos2 - 1);

    ajStrInsertS(Psequence, pos1 - 1, se->Sequence);

    return ajTrue;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listSequenceeditCompareStartAscending **************************
**
** AJAX List of Ensembl Sequence Edit objects comparison function to sort by
** start member in ascending order.
**
** @param [r] item1 [const void*] Ensembl Sequence Edit address 1
** @param [r] item2 [const void*] Ensembl Sequence Edit address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listSequenceeditCompareStartAscending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPSequenceedit se1 = *(EnsOSequenceedit *const *) item1;
    EnsPSequenceedit se2 = *(EnsOSequenceedit *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSequenceeditCompareStartAscending"))
    {
        ajDebug("listSequenceeditCompareStartAscending\n"
                "  se1 %p\n"
                "  se2 %p\n",
                se1,
                se2);

        ensSequenceeditTrace(se1, 1);
        ensSequenceeditTrace(se2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX list. */

    if (se1 && (!se2))
        return -1;

    if ((!se1) && (!se2))
        return 0;

    if ((!se1) && se2)
        return +1;

    if (se1->Start < se2->Start)
        result = -1;

    if (se1->Start > se2->Start)
        result = +1;

    return result;
}




/* @funcstatic listSequenceeditCompareStartDescending *************************
**
** AJAX List of Ensembl Sequence Edit objects comparison function to sort by
** start member in descending order.
**
** @param [r] item1 [const void*] Ensembl Sequence Edit address 1
** @param [r] item2 [const void*] Ensembl Sequence Edit address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listSequenceeditCompareStartDescending(
    const void *item1,
    const void *item2)
{
    int result = 0;

    EnsPSequenceedit se1 = *(EnsOSequenceedit *const *) item1;
    EnsPSequenceedit se2 = *(EnsOSequenceedit *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listSequenceeditCompareStartDescending"))
    {
        ajDebug("listSequenceeditCompareStartDescending\n"
                "  se1 %p\n"
                "  se2 %p\n",
                se1,
                se2);

        ensSequenceeditTrace(se1, 1);
        ensSequenceeditTrace(se2, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX list. */

    if (se1 && (!se2))
        return -1;

    if ((!se1) && (!se2))
        return 0;

    if ((!se1) && se2)
        return +1;

    if (se1->Start < se2->Start)
        result = +1;

    if (se1->Start > se2->Start)
        result = -1;

    return result;
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Sequenceedit Functions for manipulating AJAX List objects of
** Ensembl Sequence Edit objects
** @nam4rule Sort Sort functions
** @nam5rule Start Sort by start member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Ascending ses [AjPList] AJAX List of Ensembl Sequence Edit objects
** @argrule Descending ses [AjPList] AJAX List of Ensembl Sequence Edit objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListSequenceeditSortStartAscending ********************************
**
** Sort an AJAX List of Ensembl Sequence Edit objects by start member in
** ascending order.
**
** @param [u] ses [AjPList] AJAX List of Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSequenceeditSortStartAscending(AjPList ses)
{
    if (!ses)
        return ajFalse;

    ajListSort(ses, &listSequenceeditCompareStartAscending);

    return ajTrue;
}




/* @func ensListSequenceeditSortStartDescending *******************************
**
** Sort an AJAX List of Ensembl Sequence Edit objects by start member in
** descending order.
**
** @param [u] ses [AjPList] AJAX List of Ensembl Sequence Edit objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListSequenceeditSortStartDescending(AjPList ses)
{
    if (!ses)
        return ajFalse;

    ajListSort(ses, &listSequenceeditCompareStartDescending);

    return ajTrue;
}

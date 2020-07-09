/* @source transeq application
**
** Translate nucleic acid sequences
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** Mar  4 17:18 1999 (ajb)
** Jul 19 19:24 2000 (ajb)
** Jun 29 16:50 2001 (gww) use ajTrnSeqFramePep()
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

#include "emboss.h"

static void transeq_GetRegions(AjPRange regions, AjPSeq seq);
static void transeq_Trim (AjPSeq seq);


/* @prog transeq **************************************************************
**
** Translate nucleic acid sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    AjPTrn trnTable;
    AjPSeq pep;
    AjPStr *framelist;
    AjPStr frame;
    AjPStr *tablelist;
    ajint table;
    AjPRange regions;
    AjBool trim;
    AjBool defr=ajFalse; /* true if the range covers the whole sequence */
    AjBool first=ajTrue; /* true if this is the first sequence done     */
    
    ajint frameno;
  
    (void) embInit ("transeq", argc, argv);

    seqout = ajAcdGetSeqoutall ("outseq");
    seqall = ajAcdGetSeqall ("sequence");
    framelist = ajAcdGetList ("frame");
    tablelist = ajAcdGetList ("table");
    regions = ajAcdGetRange ("regions");
    trim = ajAcdGetBool ("trim");

    /* get first item from the frames list */
    frame = framelist[0];

    /* initialise the translation table */
    (void) ajStrToInt(tablelist[0], &table);
    trnTable = ajTrnNewI (table);

    /* shift values of translate region to match -sbegin=n parameter */
    /*  (void) ajRangeBegin (regions, ajSeqallBegin(seqall));*/

    /* get multi-frame special cases */
    if (!ajStrCmpC(frame, "F"))
    {
	while (ajSeqallNext(seqall, &seq))
	{
	    if (first)
	    {
		first=ajFalse;
		if (ajRangeDefault(regions, ajSeqStr(seq)))
		    defr = ajTrue;
	    }

	    /* get regions to translate */
	    if (!defr)
		(void) transeq_GetRegions(regions, seq);

	    pep = ajTrnSeqOrig(trnTable, seq, 1);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, 2);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, 3);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	}
    }
    else if (!ajStrCmpC(frame, "R"))
    {
	while (ajSeqallNext(seqall, &seq))
	{
	    if (first)
	    {
		first=ajFalse;
		if (ajRangeDefault(regions,ajSeqStr(seq)))
		    defr = ajTrue;
	    }

	    /* get regions to translate */
	    if (!defr)
		(void) transeq_GetRegions(regions, seq);

	    pep = ajTrnSeqOrig(trnTable, seq, -1);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, -2);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, -3);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	}
    }
    else if (!ajStrCmpC(frame, "6"))
    {
	while (ajSeqallNext(seqall, &seq))
	{
	    if (first)
	    {
		first=ajFalse;
		if (ajRangeDefault(regions,ajSeqStr(seq)))
		    defr = ajTrue;
	    }

	    /* get regions to translate */
	    if (!defr)
		(void) transeq_GetRegions(regions, seq);
  
	    pep = ajTrnSeqOrig(trnTable, seq, 1);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, 2);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, 3);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, -1);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, -2);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	    pep = ajTrnSeqOrig(trnTable, seq, -3);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	}
    }
    else
    {
	(void) ajStrToInt(frame, &frameno);
	while (ajSeqallNext(seqall, &seq))
	{
	    if (first)
	    {
		first=ajFalse;
		if (ajRangeDefault(regions,ajSeqStr(seq)))
		    defr = ajTrue;
	    }

	    /* get regions to translate */
	    if (!defr)
		(void) transeq_GetRegions(regions, seq);

	    pep = ajTrnSeqOrig(trnTable, seq, frameno);
	    if (trim)
		transeq_Trim(pep);
	    (void) ajSeqAllWrite (seqout, pep);
	    (void) ajSeqDel (&pep);
	}
    }
  
    (void) ajSeqWriteClose (seqout);

    /* tidy up */
    (void) ajTrnDel(&trnTable);

    (void) ajExit ();
    return 0;
}


/* @funcstatic transeq_GetRegions ********************************************
**
** Changes a sequence to only the specified regions
** A set of regions is specified by a set of pairs of positions.
** The positions are integers.
** They are separated by any non-digit, non-alpha character.
** Examples of region specifications are:
** 24-45, 56-78
** 1:45, 67=99;765..888
** 1,5,8,10,23,45,57,99
**
** @param [r] regions [AjPRange] regions to extract
** @param [r] seq [AjPSeq] sequence to extract sequence from
** @return [void]
** @@
******************************************************************************/

static void transeq_GetRegions(AjPRange regions, AjPSeq seq)
{

    AjPStr newstr = NULL;

    newstr = ajStrNew();

    (void) ajRangeStrExtract (&newstr, regions, ajSeqStr(seq));
    (void) ajSeqReplace(seq, newstr);

    (void) ajStrDel(&newstr);  

    return;
}


/* @funcstatic transeq_Trim **************************************************
**
** Removes X, and/or * characters from the end of the translation
**
**
** @param [u] seq [AjPSeq] sequence to trim
** @return [void]
** @@
******************************************************************************/

static void transeq_Trim (AjPSeq seq)
{
    AjPStr s = ajSeqStr(seq);
    char * p = ajStrStr(s);
    char c;
    ajint i;
    ajint len = ajStrLen(s)-1;
  
    for (i=len; i>=0; i--)
    {
	c = *(p+i);
	if (c != 'X' && c != '*' )
	    break;
    }

    if (i < len)
	ajStrTruncate(&s, i+1);

    return;
}


/* @source palindrome application
**
** Brute force inverted repeat finder. Allows mismatches but not gaps
**
** @author: Copyright (C) Mark Faller
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

static AjBool overlap;

typedef struct palindrome
{
   ajint forwardStart;
   ajint forwardEnd;
   ajint revStart;
   ajint revEnd;
   struct palindrome *next;
} *Palindrome;

static Palindrome palindrome_New( ajint fstart, ajint fend, ajint rstart,
				 ajint rend);
static AjBool palindrome_AInB( Palindrome a, Palindrome b);
static AjBool palindrome_AOverB( Palindrome a, Palindrome b);
static AjBool palindrome_Over ( ajint astart, ajint aend, ajint bstart,
			       ajint bend);
static void palindrome_Print( AjPFile outfile, AjPStr seq, Palindrome pal);
static AjBool palindrome_Longer( Palindrome a, Palindrome b );
static void palindrome_Swap ( Palindrome a, Palindrome b );

/* @prog palindrome ***********************************************************
**
** Looks for inverted repeats in a nucleotide sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeq sequence;
    AjPFile outfile;
    ajint minLen;
    ajint maxLen;
    ajint maxGap;
    ajint beginPos;
    ajint endPos;
    ajint maxmismatches;

    AjPStr seqstr = NULL;
    ajint current;
    ajint rev;
    ajint count;
    ajint gap;

    ajint begin;
    ajint end;
    ajint mismatches;
    ajint mismatchAtEnd;
    ajint istart;
    ajint iend;
    ajint ic;
    ajint ir;

    Palindrome pfirstpal = NULL;
    Palindrome plastpal = NULL;
    Palindrome ppal = NULL;
    Palindrome pnext = NULL;

    AjBool found = AJFALSE;

    embInit("palindrome", argc, argv);

    /*   minGap = 0; */
    sequence = ajAcdGetSeq( "insequence");
    minLen = ajAcdGetInt( "minpallen");
    maxLen = ajAcdGetInt( "maxpallen");
    maxGap = ajAcdGetInt( "gaplimit");
    outfile = ajAcdGetOutfile( "outfile");
    maxmismatches = ajAcdGetInt( "nummismatches");
    overlap = ajAcdGetBool("overlap");

    beginPos = ajSeqBegin( sequence );
    endPos = ajSeqEnd( sequence );

    /* write header to file */

    ajFmtPrintF( outfile, "Palindromes of:  %s \n", ajSeqName( sequence));
    ajFmtPrintF( outfile, "Sequence length is: %d \n", ajSeqLen( sequence));
    ajFmtPrintF( outfile, "Start at position: %d\nEnd at position: %d\n",
                beginPos, endPos);
    ajFmtPrintF( outfile, "Minimum length of Palindromes is: %d \n", minLen);
    ajFmtPrintF( outfile, "Maximum length of Palindromes is: %d \n", maxLen);
    ajFmtPrintF( outfile, "Maximum gap between elements is: %d \n", maxGap);
    ajFmtPrintF( outfile, "Number of mismatches allowed in Palindrome: %d\n", 
                maxmismatches);
    ajFmtPrintF( outfile, "\n\n\n");
    ajFmtPrintF( outfile, "Palindromes:\n");

    /* check sequence is of type nucleotide else return error */
    if (!ajSeqIsNuc( sequence))
    {
	ajFmtPrintF( outfile, "Error, sequence must be a nucleotide sequence");
	ajExit();
    }


    /* set vars in readiness to enter loop */
    seqstr = ajStrNewC(ajSeqChar( sequence));
    begin = beginPos - 1;
    end = endPos - 1;

    /* loop to look for inverted repeats */
    for (current = begin; current < end; current++)
    {
	iend = current + 2*(maxLen) + maxGap;
	if (iend > end) iend = end;
	istart = current + minLen;

	for (rev = iend; rev > istart; rev--)
	{
	    count = 0;
	    mismatches = 0;
	    mismatchAtEnd = 0;
	    ic = current;
	    ir = rev;
	    if (ajStrChar(seqstr, ic) ==
		ajSeqBaseComp(ajStrChar(seqstr, ir)))
	    {
		while (mismatches <= maxmismatches && ic < ir)
		{
		    if (ajStrChar(seqstr, ic++) ==
			ajSeqBaseComp(ajStrChar(seqstr, ir--)))
		    {
			mismatchAtEnd = 0;
		    }
		    else
		    {
			mismatches++;
			mismatchAtEnd++;
		    }
		    count++;
		}
	    }
	    count -=mismatchAtEnd;
	    gap = rev - current - count - count + 1;

            /* Find out if we have found reverse repeat long enough*/
	    if (count >= minLen && gap <= maxGap)
	    {
                /* create new palindrome struct to hold new palindrome data */ 
		ppal = palindrome_New(current,(current+count),rev,(rev-count));

                /*
                 *  if it is our first palindrome find then save it as start
                 *  of palindrome list
                 */
		if (pfirstpal == NULL)
		{
		    pfirstpal = ppal;
		    plastpal = ppal;
		}
		else
		{
                   /* check this isn't a subset of a palindrome already found */
		    pnext = pfirstpal;
		    found = AJFALSE;
		    while (pnext != NULL)
		    {
			if (overlap && palindrome_AInB( ppal, pnext))
			{
			    found = AJTRUE;
			    break;
			}
			if (!overlap && palindrome_AOverB( ppal, pnext))
			{
			    if (palindrome_Longer(ppal, pnext))
			    {
				ajDebug("swap...\n");
				palindrome_Swap(ppal, pnext);
			    }
			    else
			    {
				ajDebug("keep...\n");
			    }
			    found = AJTRUE;
			    break;
			}
			pnext = pnext->next;
		    }

                    /* if new palindrome add to end of list */
		    if (!found)
		    {
			plastpal->next = ppal;
			plastpal = ppal;
		    }
		    else
			AJFREE (ppal);
		}
	    }
	}
    }



    /*Print out palindromes*/
    ppal = pfirstpal;
    while (ppal != NULL)
    {
	palindrome_Print( outfile, seqstr, ppal);
	ppal = ppal->next;
    }


    /*free memory used for palindrome list*/
    ppal = pfirstpal;
    while (ppal != NULL)
    {
	pnext = ppal->next;
	AJFREE (ppal);
	ppal = pnext;
    }

    ajExit();
    return 0;
}




/* @funcstatic palindrome_New *************************************************
**
** Undocumented.
**
** @param [?] fstart [ajint] Undocumented
** @param [?] fend [ajint] Undocumented
** @param [?] rstart [ajint] Undocumented
** @param [?] rend [ajint] Undocumented
** @return [Palindrome] Undocumented
** @@
******************************************************************************/

static Palindrome palindrome_New( ajint fstart, ajint fend, ajint rstart,
				 ajint rend)
{

    Palindrome pal;

    AJNEW(pal);
    pal->forwardStart = fstart;
    pal->forwardEnd = fend;
    pal->revStart = rstart;
    pal->revEnd = rend;
    pal->next = NULL;

    return pal;
}

/* @funcstatic  palindrome_AInB **********************************************
**
** Undocumented.
**
** @param [?] a [Palindrome] Undocumented
** @param [?] b [Palindrome] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_AInB( Palindrome a, Palindrome b)
{

    if ((a->forwardStart >= b->forwardStart) &&
	(a->forwardEnd <=b->forwardEnd))
	if ((a->revStart <= b->revStart) &&
	    (a->revEnd >= b->revEnd))
	    return AJTRUE;

    return AJFALSE;
}

/* @funcstatic  palindrome_AOverB ********************************************
**
** Undocumented.
**
** @param [?] a [Palindrome] Undocumented
** @param [?] b [Palindrome] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_AOverB( Palindrome a, Palindrome b)
{

/*ajDebug ("overlap %d..%d %d..%d\n",
      a->forwardStart, a->forwardEnd,
      a->revStart, a->revEnd);
      ajDebug ("   with %d..%d %d..%d\n",
      b->forwardStart, b->forwardEnd,
      b->revStart, b->revEnd);*/

    if (palindrome_Over(a->forwardStart, a->forwardEnd,
		       b->forwardStart, b->forwardEnd) &&
	palindrome_Over(a->revEnd, a->revStart,
		       b->revEnd, b->revStart))
	return AJTRUE;

    return AJFALSE;
}

/* @funcstatic  palindrome_Over **********************************************
**
** Undocumented.
**
** @param [?] astart [ajint] Undocumented
** @param [?] aend [ajint] Undocumented
** @param [?] bstart [ajint] Undocumented
** @param [?] bend [ajint] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_Over( ajint astart, ajint aend, ajint bstart,
			      ajint bend)
{
    if (astart >= bstart && astart <= bend)
	return ajTrue;
    if (bstart >= astart && bstart <= aend)
	return ajTrue;

    return ajFalse;
}

/* @funcstatic  palindrome_Longer ********************************************
**
** Undocumented.
**
** @param [?] a [Palindrome] Undocumented
** @param [?] b [Palindrome] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool palindrome_Longer( Palindrome a, Palindrome b )
{
    if ((a->forwardEnd - a->forwardStart) >
	(b->forwardEnd - b->forwardStart))
	return ajTrue;

    return ajFalse;
}

/* @funcstatic  palindrome_Swap **********************************************
**
** Undocumented.
**
** @param [?] a [Palindrome] Undocumented
** @param [?] b [Palindrome] Undocumented
** @@
******************************************************************************/

static void palindrome_Swap ( Palindrome a, Palindrome b )
{
    b->forwardStart =  a->forwardStart;
    b->forwardEnd =  a->forwardEnd;
    b->revStart =  a->revStart;
    b->revEnd =  a->revEnd;

    return;
}


/* @funcstatic palindrome_Print ***********************************************
**
** Undocumented.
**
** @param [?] outfile [AjPFile] Undocumented
** @param [?] seq [AjPStr] Undocumented
** @param [?] pal [Palindrome] Undocumented
** @@
******************************************************************************/

static void palindrome_Print( AjPFile outfile, AjPStr seq, Palindrome pal)
{

    ajint i;

    ajFmtPrintF( outfile, "%-5d ", (pal->forwardStart+1));
    for (i = pal->forwardStart; i < pal->forwardEnd; i++)
	ajFmtPrintF( outfile, "%c", ajStrChar( seq, i));

    ajFmtPrintF(outfile, " %5d\n      ", pal->forwardEnd);
    for (i = pal->forwardStart; i < pal->forwardEnd; i++)
	ajFmtPrintF( outfile, "|");

    ajFmtPrintF( outfile, "\n%-5d ", (pal->revStart+1));
    for (i = pal->revStart; i > pal->revEnd; i--)
	ajFmtPrintF( outfile, "%c", ajStrChar(seq, i));

    ajFmtPrintF( outfile, " %5d\n\n", (pal->revEnd+2));

    return;
}

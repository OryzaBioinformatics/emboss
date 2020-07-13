/* wordmatch
** Create a word table for the first sequence.  Then go down second
** sequence checking to see if the word matches.  If word matches then
** check to see if the position lines up with the last position if it
** does continue else stop.
**
*/

#include "ajax.h"
#include "emboss.h"


/* @prog wordmatch ************************************************************
**
** Finds all exact matches of a given size between 2 sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

  AjPSeq seq1,seq2;
  ajint wordlen;
  AjPTable seq1MatchTable =0 ;
  AjPList matchlist = NULL;
  AjPFile outf = NULL;
  AjPFeattable Tab1=NULL,Tab2=NULL;
  AjPFeattabOut seq1out = NULL, seq2out = NULL;
  AjPAlign align = NULL;
  AjIList iter = NULL;
  ajint start1, start2, len;
  embInit("wordmatch", argc, argv);

  wordlen = ajAcdGetInt ("wordsize");

  seq1 = ajAcdGetSeq ("asequence");

  seq2 = ajAcdGetSeq ("bsequence");

  /* outf = ajAcdGetOutfile ("outfile"); */
  align     = ajAcdGetAlign("outfile");

  ajAlignSetExternal (align, ajTrue);

  seq1out     =  ajAcdGetFeatout("afeatout");
  seq2out     =  ajAcdGetFeatout("bfeatout");

  embWordLength (wordlen);
  if(embWordGetTable(&seq1MatchTable, seq1))
  { /* get table of words */
    matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2, ajTrue);
  }

  if(matchlist && outf)
    ajFmtPrintF(outf, "FINALLY length = %d\n",ajListLength(matchlist));

  embWordFreeTable(seq1MatchTable);               /* free table of words */

  if (outf)
    ajFmtPrintF(outf, "%10s %10s Length\n", ajSeqName(seq1), ajSeqName(seq2));

  if(matchlist)
  {
    if (outf)
      embWordMatchListPrint(outf, matchlist);

    iter = ajListIter(matchlist) ;
    while(embWordMatchIter(iter, &start1, &start2, &len))
    {
      ajAlignDefineSS (align, seq1, seq2);
      ajAlignSetScoreI(align, len);
      ajAlignSetSubRange (align,
			  start1, start1 + 1, start1 + len,
			  start2, start2 + 1, start2 + len);
    }
    ajListIterFree(iter) ;

    embWordMatchListConvToFeat(matchlist,&Tab1,&Tab2,seq1, seq2);

    embWordMatchListDelete(&matchlist); /* free the match structures */
  }
  ajAlignWrite (align);
  ajFeatWrite(seq1out, Tab1);
  ajFeatWrite(seq2out, Tab2);

  ajAlignClose(align);
  ajAlignDel(&align);

  ajExit();
  return 0;
}

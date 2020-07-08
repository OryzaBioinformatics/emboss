/*  Last edited: Dec  3 12:44 1999 (pmr) */
/* wordmatch
** Create a word table for the first sequence.
** Then go down second sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last position
** if it does continue else stop.
**
**
*/
/*
wordmatch -sequencea /nfs/adnah/il/wordtest/eclac.seq -sequenceb /nfs/adnah/il/wordtest/eclaci.seq -wordsize 6q
*/
#include "ajax.h"
#include "emboss.h"

int main(int argc, char * argv[]) {

  AjPSeq seq1,seq2;
  int wordlen;
  AjPTable seq1MatchTable =0 ;
  AjPList matchlist = NULL;
  AjPFile outf;
  AjPFeatTable Tab1=NULL,Tab2=NULL;
  AjPFeatTabOut seq1out = NULL, seq2out = NULL;

  embInit("wordmatch", argc, argv);

  wordlen = ajAcdGetInt ("wordsize");

  seq1 = ajAcdGetSeq ("asequence");

  seq2 = ajAcdGetSeq ("bsequence");

  outf = ajAcdGetOutfile ("outfile");

  seq1out     =  ajAcdGetFeatout("afeatout");
  seq2out     =  ajAcdGetFeatout("bfeatout");

  embWordLength (wordlen);
  if(embWordGetTable(&seq1MatchTable, seq1)){ /* get table of words */
    matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2, ajTrue);
  }

  if(matchlist)
    ajFmtPrintF(outf, "FINALLY length = %d\n",ajListLength(matchlist));
     
  embWordFreeTable(seq1MatchTable);               /* free table of words */

  ajFmtPrintF(outf, "%10s %10s Length\n", ajSeqName(seq1), ajSeqName(seq2));
  if(matchlist) {
    embWordMatchListPrint(outf, matchlist);
    embWordMatchListConvToFeat(matchlist,&Tab1,&Tab2,seq1->Name, seq2->Name);
    
    embWordMatchListDelete(matchlist); /* free the match structures */
  }
  ajFeaturesWrite(seq1out, Tab1);
  ajFeaturesWrite(seq2out, Tab2);
  ajExit();
  return 0;
}



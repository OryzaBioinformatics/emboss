#include "ajax.h"
#include "emboss.h"
static AjPSeq seq2;
static AjPSeq seq1;

ajint statwordlen;

static void matchListPrint(void **x,void *cl) {
  EmbPWordMatch p = (EmbPWordMatch)*x;
  AjPFile outfile = (AjPFile) cl;

  ajFmtPrintF(outfile, "%d  %d %d %s %d %d %s\n",
	 (*p).length,
	 (*p).seq1start+1,(*p).seq1start+(*p).length,seq1->Name->Ptr,
	 (*p).seq2start+1,(*p).seq2start+(*p).length,seq2->Name->Ptr);

  /*  printf("%d\t %d\t %d \n",(*p).seq1start,(*p).seq2start,(*p).length);*/
}

static void listPrint(AjPFile outfile, AjPList list){
  ajListMap(list,matchListPrint, outfile);
}

int main(int argc, char **argv)
{
  AjPTable seq1MatchTable =0 ;
  AjPList matchlist ;
  AjPSeqset seqset;
  AjPFile outfile;

  ajint i,j;

  
  embInit ("seqmatchall", argc, argv);

  seqset = ajAcdGetSeqset ("sequence1");

  statwordlen = ajAcdGetInt ("wordsize");

  outfile = ajAcdGetOutfile ("outfile");

  embWordLength (statwordlen);

  for(i=0;i<ajSeqsetSize(seqset);i++){
    seq1 = ajSeqsetGetSeq(seqset,i);
    seq1MatchTable = 0;
    if(ajSeqLen(seq1) > statwordlen){
      if(embWordGetTable(&seq1MatchTable, seq1)) /* get table of words */
	{ 
	  for(j=i+1;j<ajSeqsetSize(seqset);j++){
	    seq2 = ajSeqsetGetSeq(seqset,j);
	    if(ajSeqLen(seq2) > statwordlen){
	      matchlist = embWordBuildMatchTable(&seq1MatchTable,
						 seq2, ajTrue);
	      if(matchlist){
		listPrint(outfile, matchlist);
		
		embWordMatchListDelete(matchlist); /* free the match structures */
	      }
	    }
	  }
	}
      embWordFreeTable(seq1MatchTable);               /* free table of words */
    }
  }

  ajExit();
  return 0;
}

/* supermatcher
** Create a word table for the first sequence.
** Then go down second sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last position
** if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding the biggest
** match and calculating start and ends for both sequences. 
*/

#include "emboss.h"
#include <limits.h>
#include <math.h>

typedef struct concatS {
  int offset;
  int count;
  int total;
  AjPList list;
} concat;

concat *conmax = NULL;
int maxgap = 0;


static void matchListOrder(void **x,void *cl) {
  EmbPWordMatch p = (EmbPWordMatch)*x;
  AjPList ordered = (AjPList) cl;
  int offset;
  AjIList listIter;
  concat *con,*c=NULL;

  offset = (*p).seq1start-(*p).seq2start;

  /* iterate through ordered list to find if it exist already*/
  listIter = ajListIter(ordered);

  while (!ajListIterDone( listIter))
    {
      con = ajListIterNext(listIter);
      if(con->offset == offset){
	/* found so add count and set offset to the new value */
	con->offset = offset;
	con->total+= (*p).length;
	con->count++;
	ajListPushApp(con->list,p); 
	ajListIterFree(listIter);
	return;
      }
    }
  ajListIterFree(listIter);

  /* not found so add it */
  AJNEW(c);
  c->offset = offset;
  c->total = (*p).length;
  c->count = 1;
  c->list  = ajListNew();
  ajListPushApp(c->list,p); 
  ajListPushApp(ordered, c);

}

static void orderandconcat(AjPList list,AjPList ordered){

  ajListMap(list,matchListOrder, ordered);

}

static void removelists(void **x,void *cl) {
  concat *p = (concat *)*x;

  ajListFree(&(p)->list);  
  AJFREE(p);
}

static void findmax(void **x,void *cl) {
  concat *p = (concat *)*x;
  int *max = (int *) cl;

  if(p->total > *max){
    *max = p->total; 
    conmax = p;
  }

}

static int findstartpoints(AjPTable *seq1MatchTable,AjPSeq b,AjPSeq a,
			   int *start1,int *start2,int *end1,int *end2,
			   int width){
  int hwidth=0,max=-10,offset=0;
  AjPList matchlist = NULL,ordered=NULL;
  int amax=ajSeqLen(a)-1;
  int bmax =ajSeqLen(b)-1;
  ajDebug ("findstartpoints len %d %d\n", amax, bmax);
  matchlist = embWordBuildMatchTable(seq1MatchTable, b, ajTrue);
  
  if(!matchlist)
    return 0;
  else if(!matchlist->Count)
    return 0;
  
  
  /* order and add if the gap is gapmax or less */
  /* create list header bit*/
  ordered = ajListNew();
  
  orderandconcat(matchlist, ordered);
  
  ajListMap(ordered,findmax, &max);
  
  offset = conmax->offset;

  ajListMap(ordered,removelists, NULL);
  ajListFree(&ordered);
  embWordMatchListDelete(matchlist); /* free the match structures */
  
  
  hwidth = (int) width/2;
  
  offset+=hwidth;

  if(offset > 0){
    *start1=offset;
    *start2=0;
  }
  else{
    *start2=0-offset;
    *start1=0;
  }
  *end1=*start1;
  *end2=*start2;
  while(*end1<amax && *end2<bmax){
    (*end1)++; (*end2)++;
  }


  ajDebug ("findstartpoints has %d..%d [%d] %d..%d [%d]\n",
	   *start1, *end1, ajSeqLen(a), *start2, *end2, ajSeqLen(b));
  return 1;
}  



int main(int argc, char **argv)
{
    AjPSeqall seq1;
    AjPSeqset seq2;
    AjPSeq a;
    AjPSeq b;
    AjPStr m=0;
    AjPStr n=0;

    AjPFile outf,errorf;
    AjBool show,scoreonly, showalign;
    
    int    lena=0;
    int    lenb=0;

    char   *p;
    char   *q;

    AjPMatrixf matrix;
    AjPSeqCvt  cvt=0;
    float      **sub;
    int      *compass=0;
    float      *path=0;

    float gapopen;
    float gapextend;
    float score;


    int begina,i,k;
    int beginb;
    int start1=0,start2=0,end1=0,end2=0,width=0;
    AjPTable seq1MatchTable =0 ;
    int wordlen=6;
    int oldmax = 0;

    embInit("supermatcher", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    seq1      = ajAcdGetSeqall("seqa");
    seq2      = ajAcdGetSeqset("seqb");
    show      = ajAcdGetBool("showinternals");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    scoreonly = ajAcdGetBool("scoreonly");
    showalign = ajAcdGetBool("showalign");
    width     = ajAcdGetInt("width");
    wordlen   = ajAcdGetInt("wordlen");
    outf      = ajAcdGetOutfile("outfile");
    errorf    = ajAcdGetOutfile("errorfile");

    gapopen   = ajRoundF (gapopen, 8);
    gapextend = ajRoundF (gapextend, 8);

    if (!showalign)
      scoreonly = ajTrue;

    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    begina=ajSeqallBegin(seq1);
    beginb=ajSeqsetBegin(seq2);
    
    embWordLength (wordlen);

    while(ajSeqallNext(seq1,&a))
    {
      if(!m)
	m=ajStrNewL(ajSeqLen(a));

      lena = ajSeqLen(a);

      if(!embWordGetTable(&seq1MatchTable, a)) /* get table of words */
	{
	  ajFmtPrintF(errorf,
		      "Could not generate table for %s there ignoring\n",
		      ajSeqName(a)); 
	}


      for(k=0;k<ajSeqsetSize(seq2);k++)
	{
	  b = ajSeqsetGetSeq (seq2, k);
	  lenb = ajSeqLen(b);
	  if(!n)
	    n=ajStrNewL(ajSeqLen(b));
	  
	  ajDebug ("Processing '%S'\n", ajSeqGetName (b));
	  p = ajSeqChar(a);
	  q = ajSeqChar(b);
  
	  ajStrAssC(&m,"");
	  ajStrAssC(&n,"");
	  
	  
	  if(!findstartpoints(&seq1MatchTable,b,a,&start1,&start2,
			      &end1,&end2,width)){
	    start1 =0; end1= lena-1;
	    start2 = (int)(width/2); end2= lenb-1;
	    ajFmtPrintF(errorf,
			"Could not find suitable start points for %s vs %s. Therefore ignoring\n",
			ajSeqName(a),ajSeqName(b));
	    continue;
	  }
	  if(end1-start1 > oldmax){
	    oldmax = ((end1-start1)+1)*width;
	    AJRESIZE(path,oldmax*width*sizeof(float));
	    AJRESIZE(compass,oldmax*width*sizeof(int));
	  }
	  for(i=0;i<((end1-start1)+1)*width;i++)
	    path[i] = 0.0;
	  
	  ajDebug ("Calling embAlignPathCalcFast "
		   "%d..%d [%d/%d] %d..%d [%d/%d]\n",
		   start1, end1, (end1 - start1 + 1), lena,
		   start2, end2, (end2 - start2 + 1), lenb);

	  embAlignPathCalcFast(&p[start1],&q[start2],
			       end1-start1+1,end2-start2+1,
			       gapopen,gapextend,path,sub,cvt,
			       compass,show,width);
	  
	  
	  ajDebug ("Calling embAlignScoreSWMatrixFast\n");

	  score = embAlignScoreSWMatrixFast(path,compass,gapopen,gapextend,
					   a,b,end1-start1+1,end2-start2+1,
					   sub,cvt,&start1,&start2,width);
	  
	  if(scoreonly){
	    ajFmtPrintF(outf,"%s %s %.2f\n",ajSeqName(a),ajSeqName(b),score); 
	  }
	  else {
	    ajDebug ("Calling embAlignWalkSWMatrixFast\n");
	    embAlignWalkSWMatrixFast(path,compass,gapopen,gapextend,a,b,
				     &m,&n,end1-start1+1,end2-start2+1,
				     sub,cvt,&start1,&start2,width);
	    
	    ajDebug ("Calling embAlignPrintLocal\n");
	    embAlignPrintLocal(outf,ajSeqChar(a),ajSeqChar(b),
			       m,n,start1,start2,
			       score,1,sub,cvt,ajSeqName(a),ajSeqName(b),
			       begina,beginb);
	  }
	}
      
      embWordFreeTable(seq1MatchTable);               /* free table of words */
      seq1MatchTable=0;

    }
    ajStrDel(&n);
    ajStrDel(&m);

    ajExit();
    return 0;
}

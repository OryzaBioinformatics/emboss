/*  Last edited: Mar  2 15:57 2000 (pmr) */
/* polydot
** Create a polydot from a multiple sequence file.
**
**
*/
/*
polydot -sequences /nfs/adnah/il/emboss/test/data/globin.msf -wordsize 6 -graph ps
*/
#include "emboss.h"

#include "ajgraph.h"

float xstart=0,ystart=0;
int *lines;
int *pts;
int which;

static void drawPlotlines(void **x, void *cl){
  EmbPWordMatch p  = (EmbPWordMatch)*x;
  PLFLT x1,y1,x2,y2;

  lines[which]++;
  pts[which]+= (*p).length;
  x1 = x2 = ((*p).seq1start)+xstart;
  y1 = y2 = (PLFLT)((*p).seq2start)+ystart;
  x2 += (*p).length;
  y2 += (PLFLT)(*p).length;
 
  ajGraphLine(x1, y1, x2, y2);
  

}

static void plotMatches(AjPList list) {
  ajListMap(list,drawPlotlines, NULL);
}

int main(int argc, char * argv[]) {
  
  AjPSeqset seqset;
  AjPSeq seq1,seq2;
  int wordlen;
  AjPTable seq1MatchTable =0 ;
  AjPList matchlist ;
  AjPGraph graph = 0;
  int i,j;
  float total=0;
  int acceptableticks[]={1,10,50,100,200,500,1000,1500,10000,50000,
			 100000,500000,1000000,5000000};
  int numbofticks = 10;
  int gap,tickgap;
  AjBool boxit = AJTRUE, dumpfeat = AJFALSE;
  float xmargin,ymargin;
  float k;
  char ptr[10];
  float ticklen;
  float onefifth;
  AjPFeatTable *tabptr=NULL;
  AjPStr ufo=NULL,format=NULL,ext=NULL;
  AjPFeatTabOut seq1out = NULL;

  (void) ajGraphInit("polydot", argc, argv);

  wordlen = ajAcdGetInt ("wordsize");

  seqset = ajAcdGetSeqset ("sequences");

  graph = ajAcdGetGraph ("graph");
  
  gap = ajAcdGetInt ("gap");

  boxit = ajAcdGetBool("boxit");

  dumpfeat = ajAcdGetBool("dumpfeat");

  format = ajAcdGetString("format");

  ext = ajAcdGetString("ext");

  embWordLength (wordlen);

  AJCNEW(lines,ajSeqsetSize(seqset));
  AJCNEW(pts,ajSeqsetSize(seqset));
  AJCNEW(tabptr,ajSeqsetSize(seqset));

  for(i=0;i<ajSeqsetSize(seqset);i++){
    seq1 = ajSeqsetGetSeq (seqset, i);
    total += (float)ajSeqLen(seq1);
    
  }

  total +=(float)(gap*(ajSeqsetSize(seqset)-1));

  xmargin = total*0.15;
  ymargin = total*0.15;

  ticklen = xmargin*0.1;
  onefifth  = xmargin*0.2;

  i=0;
  while(acceptableticks[i]*numbofticks < ajSeqsetLen(seqset)){
    i++;
  }
  if(i<=13)
    tickgap = acceptableticks[i];
  else
    tickgap = acceptableticks[13];

  ajGraphOpenWin(graph, 0.0-xmargin,(total+xmargin)*1.35,0.0-ymargin,
		 total+ymargin);
  ajGraphTextMid ((total+xmargin)*0.5,(total+ymargin)*0.9,
			  ajStrStr(graph->title));
  ajGraphSetCharSize(0.3);
  for(i=0;i<ajSeqsetSize(seqset);i++){
    which = i;
    seq1 = ajSeqsetGetSeq (seqset, i);
    if(embWordGetTable(&seq1MatchTable, seq1)){ /* get table of words */
      for(j=0;j<ajSeqsetSize(seqset);j++){
	seq2 = ajSeqsetGetSeq (seqset, j);
	if(boxit)
	  ajGraphRect(xstart,ystart,
		      xstart+(float)ajSeqLen(seq1),
		      ystart+(float)ajSeqLen(seq2));
	if(j==0){
	  
	}
	matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2, ajTrue);
	if(matchlist)
	  plotMatches(matchlist);
	if(i<j && dumpfeat)
	  embWordMatchListConvToFeat(matchlist,&tabptr[i],&tabptr[j],seq1->Name, seq2->Name);
     
	if(matchlist) 
	  embWordMatchListDelete(matchlist); /* free the match structures */
	if(j==0){
	  for(k=0.0;k<ajSeqLen(seq1);k+=tickgap){
	    ajGraphLine(xstart+k,ystart,xstart+k,ystart-ticklen);
	    sprintf(ptr,"%d",(int)k);
	    ajGraphTextMid (xstart+k,ystart-(onefifth),ptr);
	  }
	  ajGraphTextMid (xstart+((float)ajSeqLen(seq1)/2.0),
				  ystart-(3*onefifth),
				ajStrStr(ajSeqsetName(seqset, i)));
	}
	if(i==0){
	  for(k=0.0;k<ajSeqLen(seq2);k+=tickgap){
	    ajGraphLine(xstart,ystart+k,xstart-ticklen,ystart+k);
	    sprintf(ptr,"%d",(int)k);
	    ajGraphTextEnd (xstart-(onefifth),ystart+k,ptr);
	  }
	  ajGraphTextLine(xstart-(3*onefifth),
				ystart+((float)ajSeqLen(seq2)/2.0),
				xstart-(3*onefifth),ystart+ajSeqLen(seq2), 
				ajStrStr(ajSeqsetName(seqset, j)),0.5);
	}
	ystart += (float)ajSeqLen(seq2)+(float)gap;
      }
    }
    embWordFreeTable(seq1MatchTable);              
    seq1MatchTable = NULL;
    xstart += (float)ajSeqLen(seq1)+(float)gap;
    ystart = 0.0;
  }

  ajGraphTextStart (total+onefifth,total-(onefifth),
			 ajFmtString("No. Length  Lines  Points Sequence"));

  for(i=0;i<ajSeqsetSize(seqset);i++){
    seq1 = ajSeqsetGetSeq (seqset, i);
    /*    ajUser("%d %d %d %d %s",i+1,ajSeqLen(seq1),lines[i],pts[i],
                 ajSeqName(seq1));*/
    ajGraphTextStart (total+onefifth,total-(onefifth*(i+2)),
			   ajFmtString("%3d %6d %5d %6d %s",i+1,
				       ajSeqLen(seq1),lines[i],
				       pts[i],ajSeqName(seq1)));
  }

  if(dumpfeat){
    seq1out = ajFeatTabOutNew();
    for(i=0;i<ajSeqsetSize(seqset);i++){
      seq1 = ajSeqsetGetSeq (seqset, i);
      ajStrAss(&ufo,format);
      ajStrAppC(&ufo,":");
      ajStrAppC(&ufo,ajSeqName(seq1));
      ajStrAppC(&ufo,".");
      ajStrApp(&ufo,ext);
      ajFeatTabOutOpen(seq1out, ufo);
      ajFeaturesWrite(seq1out, tabptr[i]);
    }
  }
  ajGraphClose();
  AJFREE(lines);
  AJFREE(pts);
  ajExit();
  return 0;
}

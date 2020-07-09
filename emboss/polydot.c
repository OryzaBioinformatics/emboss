/* polydot
** Create a polydot from a multiple sequence file.
**
**
*/

#include "emboss.h"

#include "ajgraph.h"

static float xstart=0;
static float ystart=0;
static ajint *lines;
static ajint *pts;
static ajint which;
static AjPFile outf=NULL;




static void polydot_drawPlotlines(void **x, void *cl);
static void polydot_dataPlotlines(void **x, void *cl);
static void polydot_plotMatches(AjPList list, AjBool text);




/* @prog polydot **************************************************************
**
** Displays all-against-all dotplots of a set of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
  
    AjPSeqset seqset;
    AjPSeq seq1,seq2;
    ajint wordlen;
    AjPTable seq1MatchTable =0 ;
    AjPList matchlist ;
    AjPGraph graph = 0;
    ajint i,j;
    float total=0;
    ajint acceptableticks[]={1,10,50,100,200,500,1000,1500,10000,50000,
			       100000,500000,1000000,5000000};
    ajint numbofticks = 10;
    ajint gap,tickgap;
    AjBool boxit = AJTRUE, dumpfeat = AJFALSE;
    float xmargin,ymargin;
    float k;
    char ptr[10];
    float ticklen;
    float onefifth;
    AjPFeattable *tabptr=NULL;
    AjPStr ufo=NULL,format=NULL,ext=NULL;
    AjPFeattabOut seq1out = NULL;
    AjBool text;
    AjPStr sajb=NULL;
    
    (void) ajGraphInit("polydot", argc, argv);

    wordlen = ajAcdGetInt ("wordsize");
    seqset = ajAcdGetSeqset ("sequences");
    graph = ajAcdGetGraph ("graph");
    gap = ajAcdGetInt ("gap");
    boxit = ajAcdGetBool("boxit");
    dumpfeat = ajAcdGetBool("dumpfeat");
    format = ajAcdGetString("format");
    text = ajAcdGetBool("data");
    outf = ajAcdGetOutfile("outfile");
    
    ext = ajAcdGetString("ext");

    sajb = ajStrNew();
    embWordLength (wordlen);

    AJCNEW(lines,ajSeqsetSize(seqset));
    AJCNEW(pts,ajSeqsetSize(seqset));
    AJCNEW(tabptr,ajSeqsetSize(seqset));

    for(i=0;i<ajSeqsetSize(seqset);i++)
    {
	seq1 = ajSeqsetGetSeq (seqset, i);
	total += (float)ajSeqLen(seq1);
    
    }

    total +=(float)(gap*(ajSeqsetSize(seqset)-1));

    xmargin = total*0.15;
    ymargin = total*0.15;

    ticklen = xmargin*0.1;
    onefifth  = xmargin*0.2;

    i=0;
    while(acceptableticks[i]*numbofticks < ajSeqsetLen(seqset))
	i++;

    if(i<=13)
	tickgap = acceptableticks[i];
    else
	tickgap = acceptableticks[13];

    if(!text)
    {
	ajGraphOpenWin(graph, 0.0-xmargin,(total+xmargin)*1.35,0.0-ymargin,
		       total+ymargin);
	ajGraphTextMid ((total+xmargin)*0.5,(total+ymargin)*0.9,
			ajStrStr(graph->title));
	ajGraphSetCharSize(0.3);
    }
    else
	ajFmtPrintF(outf,"##Graphic\n##Screen x1 %f y1 %f x2 %f y2 %f\n",
		    0.0-xmargin,(total+xmargin)*1.35,0.0-ymargin,
		    total+ymargin);
    
    

    for(i=0;i<ajSeqsetSize(seqset);i++)
    {
	which = i;
	seq1 = ajSeqsetGetSeq (seqset, i);
	if(embWordGetTable(&seq1MatchTable, seq1)){ /* get table of words */
	    for(j=0;j<ajSeqsetSize(seqset);j++)
	    {
		seq2 = ajSeqsetGetSeq (seqset, j);
		if(boxit && !text)
		    ajGraphRect(xstart,ystart,
				xstart+(float)ajSeqLen(seq1),
				ystart+(float)ajSeqLen(seq2));
		if(boxit && text)
		    ajFmtPrintF(outf,"Rectangle x1 %f y1 %f x2 %f y2 %f"
				" colour 0\n",xstart,ystart,
				xstart+(float)ajSeqLen(seq1),
				ystart+(float)ajSeqLen(seq2));

		matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2,
						   ajTrue);
		if(matchlist)
		    polydot_plotMatches(matchlist,text);
		if(i<j && dumpfeat)
		    embWordMatchListConvToFeat(matchlist,&tabptr[i],
					       &tabptr[j],seq1,
					       seq2);
     
		if(matchlist)  /* free the match structures */
		    embWordMatchListDelete(&matchlist);

		if(j==0)
		{
		    for(k=0.0;k<ajSeqLen(seq1);k+=tickgap)
		    {
			if(!text)
			    ajGraphLine(xstart+k,ystart,xstart+k,
					ystart-ticklen);
			else
			    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f "
					"colour 0\n",
					xstart+k,ystart,xstart+k,
					ystart-ticklen);

			sprintf(ptr,"%d",(ajint)k);
			if(!text)
			    ajGraphTextMid (xstart+k,ystart-(onefifth),ptr);
			else
			    ajFmtPrintF(outf,"Text2 x1 %f y1 %f colour 0 "
					"size 0.3 %s\n",
					xstart+k,ystart-(onefifth),ptr);
		    }
		    if(!text)
			ajGraphTextMid (xstart+((float)ajSeqLen(seq1)/2.0),
				    ystart-(3*onefifth),
				    ajStrStr(ajSeqsetName(seqset, i)));
		    else
			ajFmtPrintF(outf,"Text2 x1 %f y1 %f colour 0 "
				    "size 0.3 %s\n",
				    xstart+((float)ajSeqLen(seq1)/2.0),
				    ystart-(3*onefifth),
				    ajStrStr(ajSeqsetName(seqset, i)));

		}
		if(i==0)
		{
		    for(k=0.0;k<ajSeqLen(seq2);k+=tickgap)
		    {
			if(!text)
			    ajGraphLine(xstart,ystart+k,xstart-ticklen,
					ystart+k);
			else
			    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f "
					"colour 0\n",xstart,ystart+k,
					xstart-ticklen,
					ystart+k);

			sprintf(ptr,"%d",(ajint)k);
			if(!text)
			    ajGraphTextEnd (xstart-(onefifth),ystart+k,ptr);
			else
			    ajFmtPrintF(outf,"Text3 x1 %f y1 %f colour 0 "
					"size 0.3 %s\n",
					xstart-(onefifth),ystart+k,ptr);
		    }
		    if(!text)
			ajGraphTextLine(xstart-(3*onefifth),
				    ystart+((float)ajSeqLen(seq2)/2.0),
				    xstart-(3*onefifth),ystart+ajSeqLen(seq2), 
				    ajStrStr(ajSeqsetName(seqset, j)),0.5);
		    else
			ajFmtPrintF(outf,"Textline x1 %f y1 %f x2 %f y2 %f "
				    "colour 0 size 0.5 %s\n",
				    xstart-(3*onefifth),
				    ystart+((float)ajSeqLen(seq2)/2.0),
				    xstart-(3*onefifth),ystart+ajSeqLen(seq2), 
				    ajStrStr(ajSeqsetName(seqset, j)));
		}
		ystart += (float)ajSeqLen(seq2)+(float)gap;
	    }
	}
	embWordFreeTable(seq1MatchTable);              
	seq1MatchTable = NULL;
	xstart += (float)ajSeqLen(seq1)+(float)gap;
	ystart = 0.0;
    }

    if(!text)
	ajGraphTextStart (total+onefifth,total-(onefifth),
		          "No. Length  Lines  Points Sequence");
    else
	ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.3 %s\n",
		    total+onefifth,total-(onefifth),
		    "No. Length  Lines  Points Sequence");


    for(i=0;i<ajSeqsetSize(seqset);i++)
    {
	seq1 = ajSeqsetGetSeq (seqset, i);
	/*    ajUser("%d %d %d %d %s",i+1,ajSeqLen(seq1),lines[i],pts[i],
	      ajSeqName(seq1));*/
	ajFmtPrintS(&sajb,"%3d %6d %5d %6d %s",i+1,
		    ajSeqLen(seq1),lines[i],
		    pts[i],ajSeqName(seq1));

	if(!text)

	    ajGraphTextStart (total+onefifth,total-(onefifth*(i+2)),
			      ajStrStr(sajb));
	else
	    ajFmtPrintF(outf,"Text1 x1 %f y1 %f colour 0 size 0.3 %s\n",
			total+onefifth,total-(onefifth*(i+2)),
			ajStrStr(sajb));
    }

    if(dumpfeat)
    {
	seq1out = ajFeattabOutNew();
	for(i=0;i<ajSeqsetSize(seqset);i++)
	{
	    seq1 = ajSeqsetGetSeq (seqset, i);
	    ajStrAss(&ufo,format);
	    ajStrAppC(&ufo,":");
	    ajStrAppC(&ufo,ajSeqName(seq1));
	    ajStrAppC(&ufo,".");
	    ajStrApp(&ufo,ext);
	    ajFeattabOutOpen(seq1out, ufo);
	    ajFeatWrite(seq1out, tabptr[i]);
	}
    }

    if(!text)
	ajGraphClose();
    else
	ajFileClose(&outf);


    ajStrDel(&sajb);
    AJFREE(lines);
    AJFREE(pts);

    ajExit();
    return 0;
}




/* @funcstatic polydot_drawPlotlines *****************************************
**
** Undocumented.
**
** @@
******************************************************************************/

static void polydot_drawPlotlines(void **x, void *cl)
{
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




/* @funcstatic  polydot_dataPlotlines ****************************************
**
** Undocumented.
**
** @@
******************************************************************************/

static void polydot_dataPlotlines(void **x, void *cl)
{
    EmbPWordMatch p  = (EmbPWordMatch)*x;
    PLFLT x1,y1,x2,y2;

    lines[which]++;
    pts[which]+= (*p).length;
    x1 = x2 = ((*p).seq1start)+xstart;
    y1 = y2 = (PLFLT)((*p).seq2start)+ystart;
    x2 += (*p).length;
    y2 += (PLFLT)(*p).length;
 
    ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",x1, y1, x2, y2);
}




/* @funcstatic polydot_plotMatches *******************************************
**
** Undocumented.
**
** @param [?] list [AjPList] Undocumented
** @param [?] text [AjBool] Undocumented
** @@
******************************************************************************/

static void polydot_plotMatches(AjPList list, AjBool text)
{
    if(!text)
	ajListMap(list,polydot_drawPlotlines, NULL);
    else
	ajListMap(list,polydot_dataPlotlines, NULL);

    return;
}

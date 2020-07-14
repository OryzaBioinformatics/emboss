/* @source dottup application
**
** Dotplot of two sequences
**
** @author: Copyright (C) Ian Longden
** @modified: Alan Bleasby. Added non-proportional plot
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




static void dottup_drawPlotlines(void *x, void *cl);
static void dottup_plotMatches(const AjPList list);
static void dottup_stretchplot(AjPGraph graph, const AjPList matchlist,
			       const AjPSeq seq1, const AjPSeq seq2,
			       ajint begin1, ajint begin2,
			       ajint end1, ajint end2);




/* @prog dottup ***************************************************************
**
** Displays a wordmatch dotplot of two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq1;
    AjPSeq seq2;
    ajint wordlen;
    AjPTable seq1MatchTable = 0;
    AjPList matchlist = NULL;
    AjPGraph graph    = NULL;
    AjPGraph xygraph  = NULL;
    AjBool boxit;
    /*
    ** Different ticks as they need to be different for x and y due to
    ** length of string being important on x
    */
    ajint acceptableticksx[]=
    {
	1,10,50,100,500,1000,1500,10000,
	500000,1000000,5000000
    };
    ajint acceptableticks[]=
    {
	1,10,50,100,200,500,1000,2000,5000,10000,15000,
	500000,1000000,5000000
    };
    ajint numbofticks = 10;
    float xmargin;
    float ymargin;
    float ticklen;
    float tickgap;
    float onefifth = 0.0;
    ajint i;
    float k;
    float max;
    char ptr[10];
    ajint begin1;
    ajint begin2;
    ajint end1;
    ajint end2;
    AjBool stretch;

    ajGraphInit("dottup", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seq1    = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");
    graph   = ajAcdGetGraph("graph");
    boxit   = ajAcdGetBool("boxit");
    stretch = ajAcdGetToggle("stretch");
    xygraph = ajAcdGetGraphxy("xygraph");

    begin1 = ajSeqBegin(seq1);
    begin2 = ajSeqBegin(seq2);
    end1   = ajSeqEnd(seq1);
    end2   = ajSeqEnd(seq2);

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2, ajTrue);


    if(stretch)
    {
	dottup_stretchplot(xygraph,matchlist,seq1,seq2,begin1,begin2,end1,
			   end2);
	if(matchlist)
	    embWordMatchListDelete(&matchlist); /* free the match structures */
	ajExit();
	return 0;
    }

    /* only get here if stretch is false */

    max= ajSeqLen(seq1);
    if(ajSeqLen(seq2) > max)
	max = ajSeqLen(seq2);

    xmargin = ymargin = max *0.15;

    ajGraphOpenWin(graph, 0.0-ymargin,(max*1.35)+ymargin,
		   0.0-xmargin,(float)max+xmargin);

    ajGraphTextMid(max*0.5,(ajSeqLen(seq2))+(xmargin*0.5),
		   ajGraphGetTitleC(graph));
    ajGraphSetCharSize(0.5);

    if(matchlist)
	dottup_plotMatches(matchlist);

    if(boxit)
    {
	ajGraphRect(0.0,0.0,(float)ajSeqLen(seq1),(float)ajSeqLen(seq2));
	i = 0;
	while(acceptableticksx[i]*numbofticks < ajSeqLen(seq1))
	    i++;

	if(i<=13)
	    tickgap = acceptableticksx[i];
	else
	    tickgap = acceptableticksx[10];

	ticklen = xmargin*0.1;
	onefifth  = xmargin*0.2;
	ajGraphTextMid((ajSeqLen(seq1))*0.5,0.0-(onefifth*3),
			ajGraphGetYTitleC(graph));

	if(ajSeqLen(seq2)/ajSeqLen(seq1) > 10 )
	{
	    /* a lot smaller then just label start and end */
	    ajGraphLine(0.0,0.0,0.0,0.0-ticklen);
	    sprintf(ptr,"%d",ajSeqOffset(seq1));
	    ajGraphTextMid( 0.0,0.0-(onefifth),ptr);

	    ajGraphLine((float)(ajSeqLen(seq1)),0.0,
			(float)ajSeqLen(seq1),0.0-ticklen);
	    sprintf(ptr,"%d",ajSeqLen(seq1)+ajSeqOffset(seq1));
	    ajGraphTextMid((float)ajSeqLen(seq1),0.0-(onefifth),ptr);
	    
	}
	else
	    for(k=0.0;k<ajSeqLen(seq1);k+=tickgap)
	    {
		ajGraphLine(k,0.0,k,0.0-ticklen);
		sprintf(ptr,"%d",(ajint)k+ajSeqOffset(seq1));
		ajGraphTextMid( k,0.0-(onefifth),ptr);
	    }

	i = 0;
	while(acceptableticks[i]*numbofticks < ajSeqLen(seq2))
	    i++;

	tickgap   = acceptableticks[i];
	ticklen   = ymargin*0.1;
	onefifth  = ymargin*0.2;
	ajGraphTextLine(0.0-(onefifth*4),(ajSeqLen(seq2))*0.5,
			0.0-(onefifth*4),(float)ajSeqLen(seq2),
			ajGraphGetXTitleC(graph),0.5);

	if(ajSeqLen(seq1)/ajSeqLen(seq2) > 10 )
	{
	    /* a lot smaller then just label start and end */
	    ajGraphLine(0.0,0.0,0.0-ticklen,0.0);
	    sprintf(ptr,"%d",ajSeqOffset(seq2));
	    ajGraphTextEnd( 0.0-(onefifth),0.0,ptr);

	    ajGraphLine(0.0,(float)ajSeqLen(seq2),0.0-ticklen,
			(float)ajSeqLen(seq2));
	    sprintf(ptr,"%d",ajSeqLen(seq2)+ajSeqOffset(seq2));
	    ajGraphTextEnd( 0.0-(onefifth),(float)ajSeqLen(seq2),ptr);
	}
	else
	    for(k=0.0;k<ajSeqLen(seq2);k+=tickgap)
	    {
		ajGraphLine(0.0,k,0.0-ticklen,k);
		sprintf(ptr,"%d",(ajint)k+ajSeqOffset(seq2));
		ajGraphTextEnd( 0.0-(onefifth),k,ptr);
	    }
    }

    ajGraphClose();

    if(matchlist)
	embWordMatchListDelete(&matchlist); /* free the match structures */

    ajExit();

    return 0;
}




#ifndef NO_PLOT

/* @funcstatic dottup_drawPlotlines *******************************************
**
** Undocumented.
**
** @param [r] x [void*] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_drawPlotlines(void *x, void *cl)
{
    EmbPWordMatch p;
    PLFLT x1;
    PLFLT y1;
    PLFLT x2;
    PLFLT y2;

    p  = (EmbPWordMatch)x;

    x1 = x2 = (PLFLT)(p->seq1start);
    y1 = y2 = (PLFLT)(p->seq2start);

    x2 += (PLFLT)p->length;
    y2 += (PLFLT)p->length;

    ajGraphLine(x1, y1, x2, y2);

    return;
}




/* @funcstatic dottup_plotMatches *********************************************
**
** Undocumented.
**
** @param [r] list [const AjPList] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_plotMatches(const AjPList list)
{
    ajListMapRead(list,dottup_drawPlotlines, NULL);

    return;
}

#endif




/* @funcstatic dottup_stretchplot *********************************************
**
** Undocumented.
**
** @param [u] graph [AjPGraph] Undocumented
** @param [r] matchlist [const AjPList] Undocumented
** @param [r] seq1 [const AjPSeq] Undocumented
** @param [r] seq2 [const AjPSeq] Undocumented
** @param [r] begin1 [ajint] Undocumented
** @param [r] begin2 [ajint] Undocumented
** @param [r] end1 [ajint] Undocumented
** @param [r] end2 [ajint] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_stretchplot(AjPGraph graph, const AjPList matchlist,
			       const AjPSeq seq1, const AjPSeq seq2,
			       ajint begin1, ajint begin2,
			       ajint end1, ajint end2)
{
    EmbPWordMatch wmp = NULL;
    float xa[1];
    float ya[2];
    AjPGraphPlpData gdata = NULL;
    AjPStr tit = NULL;
    float x1;
    float y1;
    float x2;
    float y2;
    AjIList iter = NULL;

    tit = ajStrNew();
    ajFmtPrintS(&tit,"%S",ajGraphGetTitle(graph));


    gdata = ajGraphPlpDataNewI(1);
    xa[0] = (float)begin1;
    ya[0] = (float)begin2;

    ajGraphSetTitleC(graph,ajStrStr(tit));

    ajGraphSetXTitleC(graph,ajSeqName(seq1));
    ajGraphSetYTitleC(graph,ajSeqName(seq2));

    ajGraphPlpDataSetTypeC(gdata,"2D Plot Float");
    ajGraphPlpDataSetMaxMin(gdata,(float)begin1,(float)end1,(float)begin2,
			   (float)end2);
    ajGraphPlpDataSetMaxima(gdata,(float)begin1,(float)end1,(float)begin2,
			   (float)end2);
    ajGraphxySetXStart(graph,(float)begin1);
    ajGraphxySetXEnd(graph,(float)end1);
    ajGraphxySetYStart(graph,(float)begin2);
    ajGraphxySetYEnd(graph,(float)end2);

    ajGraphxySetXRangeII(graph,begin1,end1);
    ajGraphxySetYRangeII(graph,begin2,end2);


    if(matchlist)
    {
	iter = ajListIterRead(matchlist);
	while((wmp = ajListIterNext(iter)))
	{
	    x1 = x2 = (float) (wmp->seq1start + begin1);
	    y1 = y2 = (float) (wmp->seq2start + begin2);
	    x2 += (float) wmp->length-1;
	    y2 += (float) wmp->length-1;
	    ajGraphAddLine(graph,x1,y1,x2,y2,0);
	}
	ajListIterFree(&iter);
    }

    ajGraphPlpDataSetXY(gdata,xa,ya);
    ajGraphDataReplace(graph,gdata);


    ajGraphxyDisplay(graph,ajFalse);
    ajGraphClose();

    ajStrDel(&tit);

    return;
}

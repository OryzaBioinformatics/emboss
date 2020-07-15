/* @source dottup application
**
** Dotplot of two sequences
**
** @author Copyright (C) Ian Longden
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
    ajuint acceptableticksx[]=
    {
	1,10,50,100,500,1000,1500,10000,
	500000,1000000,5000000
    };
    ajuint acceptableticks[]=
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
    ajuint len1;
    ajuint len2;
    float flen1;
    float flen2;
    AjBool stretch;
    ajuint tui;
    
    ajGraphInit("dottup", argc, argv);

    wordlen = ajAcdGetInt("wordsize");
    seq1    = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");
    graph   = ajAcdGetGraph("graph");
    boxit   = ajAcdGetBool("boxit");
    stretch = ajAcdGetToggle("stretch");
    xygraph = ajAcdGetGraphxy("xygraph");

    begin1 = ajSeqGetBegin(seq1);
    begin2 = ajSeqGetBegin(seq2);
    end1   = ajSeqGetEnd(seq1);
    end2   = ajSeqGetEnd(seq2);
    len1   = ajSeqGetLen(seq1);
    len2   = ajSeqGetLen(seq2);

    tui    = ajSeqGetLen(seq1);
    flen1  = (float) tui;
    tui    = ajSeqGetLen(seq2);
    flen2 = (float) tui;
    
    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    embWordLength(wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	matchlist = embWordBuildMatchTable(seq1MatchTable, seq2, ajTrue);


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

    max= flen1;
    if(flen2 > max)
	max = flen2;

    xmargin = ymargin = max * (float)0.15;

    ajGraphOpenWin(graph, (float)0.0-ymargin,(max*(float)1.35)+ymargin,
		   (float)0.0-xmargin,(float)max+xmargin);

    ajGraphTextMid(max*(float)0.5,flen2+(xmargin*(float)0.5),
		   ajGraphGetTitleC(graph));
    ajGraphSetCharScale(0.5);

    if(matchlist)
	dottup_plotMatches(matchlist);

    if(boxit)
    {
	ajGraphRect(0.0,0.0,flen1,flen2);
	i = 0;
	while(acceptableticksx[i]*numbofticks < len1)
	    i++;

	if(i<=13)
	    tickgap = (float) acceptableticksx[i];
	else
	    tickgap = (float) acceptableticksx[10];

	ticklen = xmargin*(float)0.1;
	onefifth  = xmargin*(float)0.2;
	ajGraphTextMid(flen1*(float)0.5,(float)0.0-(onefifth*(float)3.),
			ajGraphGetYTitleC(graph));

	if(len2/len1 > 10 )
	{
	    /* a lot smaller then just label start and end */
	    ajGraphLine((float)0.0,(float)0.0,(float)0.0,(float)0.0-ticklen);
	    sprintf(ptr,"%d",ajSeqGetOffset(seq1));
	    ajGraphTextMid((float)0.0,(float)0.0-(onefifth),ptr);

	    ajGraphLine(flen1,(float)0.0,
			flen1,(float)0.0-ticklen);
	    sprintf(ptr,"%d",len1+ajSeqGetOffset(seq1));
	    ajGraphTextMid(flen1,(float)0.0-(onefifth),ptr);
	    
	}
	else
	    for(k=0.0;k<len1;k+=tickgap)
	    {
		ajGraphLine(k,(float)0.0,k,(float)0.0-ticklen);
		sprintf(ptr,"%d",(ajint)k+ajSeqGetOffset(seq1));
		ajGraphTextMid( k,(float)0.0-(onefifth),ptr);
	    }

	i = 0;
	while(acceptableticks[i]*numbofticks < len2)
	    i++;

	tickgap   = (float) acceptableticks[i];
	ticklen   = ymargin*(float)0.1;
	onefifth  = ymargin*(float)0.2;
	ajGraphTextLine((float)0.0-(onefifth*(float)4.),flen2*(float)0.5,
			(float)0.0-(onefifth*(float)4.),flen2,
			ajGraphGetXTitleC(graph),0.5);

	if(len1/len2 > 10 )
	{
	    /* a lot smaller then just label start and end */
	    ajGraphLine((float)0.0,(float)0.0,(float)0.0-ticklen,(float)0.0);
	    sprintf(ptr,"%d",ajSeqGetOffset(seq2));
	    ajGraphTextEnd((float)0.0-(onefifth),(float)0.0,ptr);

	    ajGraphLine((float)0.0,flen2,(float)0.0-ticklen,
			flen2);
	    sprintf(ptr,"%d",len2+ajSeqGetOffset(seq2));
	    ajGraphTextEnd((float)0.0-(onefifth),flen2,ptr);
	}
	else
	    for(k=0.0;k<len2;k+=tickgap)
	    {
		ajGraphLine((float)0.0,k,(float)0.0-ticklen,k);
		sprintf(ptr,"%d",(ajint)k+ajSeqGetOffset(seq2));
		ajGraphTextEnd((float)0.0-(onefifth),k,ptr);
	    }
    }

    ajGraphClose();
    ajSeqDel(&seq1);
    ajSeqDel(&seq2);
    ajGraphxyDel(&graph);
    ajGraphxyDel(&xygraph);

    embWordFreeTable(&seq1MatchTable);

    if(matchlist)
	embWordMatchListDelete(&matchlist); /* free the match structures */

    embExit();

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

    (void) cl;				/* make it used */

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

    ajGraphSetTitleC(graph,ajStrGetPtr(tit));

    ajGraphSetXTitleC(graph,ajSeqGetNameC(seq1));
    ajGraphSetYTitleC(graph,ajSeqGetNameC(seq2));

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

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


static void dottup_objtofile(void **x,void *cl);
static void dottup_drawPlotlines(void **x, void *cl);
static void dottup_plotMatches(AjPList list);
static void dottup_stretchplot(AjPGraph graph, AjPList matchlist, AjPSeq seq1,
			AjPSeq seq2, ajint begin1, ajint begin2, ajint end1,
			ajint end2);




/* @prog dottup ***************************************************************
**
** Displays a wordmatch dotplot of two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    EmbPWordMatch wmp=NULL;
    ajint wplen;
    AjPSeq seq1,seq2;
    ajint wordlen;
    AjPTable seq1MatchTable=0;
    AjPList matchlist=NULL;
    AjPGraph graph = NULL;
    AjPGraph xygraph = NULL;
    AjPFile outfile;
    AjBool boxit,text;
    /* Different ticks as they need to be different for x and y due to
       length of string being important on x */
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
    float xmargin,ymargin;
    float ticklen,tickgap;
    float onefifth=0.0;
    ajint i;
    float k,max;
    char ptr[10];
    ajint begin1;
    ajint begin2;
    ajint end1;
    ajint end2;
    AjBool stretch;

    ajGraphInit("dottup", argc, argv);

    wordlen = ajAcdGetInt ("wordsize");
    seq1 = ajAcdGetSeq ("sequencea");
    seq2 = ajAcdGetSeq ("sequenceb");
    graph = ajAcdGetGraph ("graph");
    text = ajAcdGetBool("data");
    boxit = ajAcdGetBool("boxit");
    outfile = ajAcdGetOutfile ("outfile");
    stretch = ajAcdGetBool("stretch");
    xygraph = ajAcdGetGraphxy ("xygraph");

    begin1 = ajSeqBegin(seq1);
    begin2 = ajSeqBegin(seq2);
    end1   = ajSeqEnd(seq1);
    end2   = ajSeqEnd(seq2);

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    embWordLength (wordlen);
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

    /* We get  here only if stretch is false */

    max= ajSeqLen(seq1);
    if(ajSeqLen(seq2) > max)
	max = ajSeqLen(seq2);

    xmargin = ymargin = max *0.15;

    if(!text)
    {
	ajGraphOpenWin(graph, 0.0-ymargin,(max*1.35)+ymargin,
		       0.0-xmargin,(float)max+xmargin);

	ajGraphTextMid (max*0.5,(ajSeqLen(seq2))+(xmargin*0.5),
			ajStrStr(graph->title));
	ajGraphSetCharSize(0.5);

	if(matchlist)
	    dottup_plotMatches(matchlist);
	if(boxit)
	{
	    ajGraphRect( 0.0,0.0,(float)ajSeqLen(seq1),(float)ajSeqLen(seq2));
	    i=0;
	    while(acceptableticksx[i]*numbofticks < ajSeqLen(seq1))
		i++;

	    if(i<=13)
		tickgap = acceptableticksx[i];
	    else
	/*	tickgap = acceptableticksx[13];  bugfixed AJB 26/7/00 */
		tickgap = acceptableticksx[10];
	    ticklen = xmargin*0.1;
	    onefifth  = xmargin*0.2;
	    ajGraphTextMid ((ajSeqLen(seq1))*0.5,0.0-(onefifth*3),
			    ajStrStr(graph->yaxis));
	    if(ajSeqLen(seq2)/ajSeqLen(seq1) > 10 )
	    {		     /* a lot smaller then just label start and end */
		ajGraphLine(0.0,0.0,0.0,0.0-ticklen);
		sprintf(ptr,"%d",ajSeqOffset(seq1));
		ajGraphTextMid ( 0.0,0.0-(onefifth),ptr);

		ajGraphLine((float)(ajSeqLen(seq1)),0.0,
			    (float)ajSeqLen(seq1),0.0-ticklen);
		sprintf(ptr,"%d",ajSeqLen(seq1)+ajSeqOffset(seq1));
		ajGraphTextMid ( (float)ajSeqLen(seq1),0.0-(onefifth),ptr);

	    }
	    else
	    {
		for(k=0.0;k<ajSeqLen(seq1);k+=tickgap)
		{
		    ajGraphLine(k,0.0,k,0.0-ticklen);
		    sprintf(ptr,"%d",(ajint)k+ajSeqOffset(seq1));
		    ajGraphTextMid ( k,0.0-(onefifth),ptr);
		}
	    }


	    i=0;
	    while(acceptableticks[i]*numbofticks < ajSeqLen(seq2))
		i++;

	    tickgap = acceptableticks[i];
	    ticklen = ymargin*0.1;
	    onefifth  = ymargin*0.2;
	    ajGraphTextLine(0.0-(onefifth*4),(ajSeqLen(seq2))*0.5,
			    0.0-(onefifth*4),(float)ajSeqLen(seq2),
			    ajStrStr(graph->xaxis),0.5);
	    if(ajSeqLen(seq1)/ajSeqLen(seq2) > 10 )
	    {		    /* a lot smaller then just label start and end */
		ajGraphLine(0.0,0.0,0.0-ticklen,0.0);
		sprintf(ptr,"%d",ajSeqOffset(seq2));
		ajGraphTextEnd ( 0.0-(onefifth),0.0,ptr);

		ajGraphLine(0.0,(float)ajSeqLen(seq2),0.0-ticklen,
			    (float)ajSeqLen(seq2));
		sprintf(ptr,"%d",ajSeqLen(seq2)+ajSeqOffset(seq2));
		ajGraphTextEnd ( 0.0-(onefifth),(float)ajSeqLen(seq2),ptr);
	    }
	    else
	    {
		for(k=0.0;k<ajSeqLen(seq2);k+=tickgap)
		{
		    ajGraphLine(0.0,k,0.0-ticklen,k);
		    sprintf(ptr,"%d",(ajint)k+ajSeqOffset(seq2));
		    ajGraphTextEnd ( 0.0-(onefifth),k,ptr);
		}
	    }
	}
	ajGraphClose();

    }
    else
    {
/*	if(matchlist)
	    ajFmtPrintF (outfile, "%d matches found\n\n",
			 ajListLength(matchlist)+1);
*/
	embWordFreeTable(seq1MatchTable); /* free table of words */


	ajFmtPrintF(outfile,"##2D Plot\n##Title dottup (%D)\n",ajTimeToday());
	ajFmtPrintF(outfile,"##Graphs 1\n##Number 1\n##Points 0\n");
	ajFmtPrintF(outfile,"##XminA %f XmaxA %f YminA %f YmaxA %f\n",0.,
		    (float)ajSeqLen(seq1),0.,(float)ajSeqLen(seq2));
	ajFmtPrintF(outfile,"##Xmin %f Xmax %f Ymin %f Ymax %f\n",0.,
		    (float)ajSeqLen(seq1),0.,(float)ajSeqLen(seq2));
	ajFmtPrintF(outfile,"##ScaleXmin %f ScaleXmax %f "
		    "ScaleYmin %f ScaleYmax %f\n",(float)begin1,(float)end1,
		    (float)begin2,(float)end2);
	ajFmtPrintF(outfile,"##Maintitle\n");
	ajFmtPrintF(outfile,"##Xtitle %s\n##Ytitle %s\n",ajSeqName(seq1),
		    ajSeqName(seq2));
	ajFmtPrintF(outfile,"##DataObjects\n##Number %d\n",
		    ajListLength(matchlist));

	if(matchlist)
	{
	    wplen = ajListLength(matchlist);
	    for(i=0;i<wplen;++i)
	    {
		ajListPop(matchlist,(void **)&wmp);
		wmp->seq1start += begin1;
		wmp->seq2start += begin2;
		ajListPushApp(matchlist,(void *)wmp);
	    }
	}


	if(matchlist)
	    ajListMap(matchlist,dottup_objtofile, outfile);
	ajFmtPrintF(outfile,"##GraphObjects\n##Number 0\n");

    }

    if(matchlist)
	embWordMatchListDelete(&matchlist); /* free the match structures */

    ajExit();
    return 0;
}


/* @funcstatic dottup_objtofile ***********************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/


static void dottup_objtofile(void **x,void *cl)
{
    EmbPWordMatch p = (EmbPWordMatch)*x;
    AjPFile file = (AjPFile) cl;
    ajint x1;
    ajint x2;
    ajint y1;
    ajint y2;

    x1 = (*p).seq1start;
    y1 = (*p).seq2start;
    x2 = x1 + (*p).length;
    y2 = y1 + (*p).length;

    (void) ajFmtPrintF(file, "Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
		       (float)x1,(float)y1,(float)x2,(float)y2);
    return;
}

#ifndef NO_PLOT

/* @funcstatic dottup_drawPlotlines *******************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_drawPlotlines(void **x, void *cl)
{
    EmbPWordMatch p  = (EmbPWordMatch)*x;
    PLFLT x1,y1,x2,y2;

    x1 = x2 = ((*p).seq1start)+1;
    y1 = y2 = (PLFLT)((*p).seq2start)+1;
    x2 += (*p).length;
    y2 += (PLFLT)(*p).length;

    ajGraphLine(x1, y1, x2, y2);

    return;
}




/* @funcstatic dottup_plotMatches *********************************************
**
** Undocumented.
**
** @param [?] list [AjPList] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_plotMatches(AjPList list)
{
    ajListMap(list,dottup_drawPlotlines, NULL);

    return;
}

#endif







/* @funcstatic dottup_stretchplot *********************************************
**
** Undocumented.
**
** @param [?] graph [AjPGraph] Undocumented
** @param [?] matchlist [AjPList] Undocumented
** @param [?] seq1 [AjPSeq] Undocumented
** @param [?] seq2 [AjPSeq] Undocumented
** @param [?] begin1 [ajint] Undocumented
** @param [?] begin2 [ajint] Undocumented
** @param [?] end1 [ajint] Undocumented
** @param [?] end2 [ajint] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dottup_stretchplot(AjPGraph graph, AjPList matchlist, AjPSeq seq1,
			AjPSeq seq2, ajint begin1, ajint begin2, ajint end1,
			ajint end2)
{
    EmbPWordMatch wmp=NULL;
    float xa[1];
    float ya[2];
    AjPGraphData gdata=NULL;
    AjPStr tit=NULL;
    float x1;
    float y1;
    float x2;
    float y2;
    AjIList iter=NULL;

    tit = ajStrNew();
    ajFmtPrintS(&tit,"%S",graph->title);


    gdata = ajGraphxyDataNewI(1);
    xa[0] = (float)begin1;
    ya[0] = (float)begin2;

    ajGraphxyTitleC(graph,ajStrStr(tit));

    ajGraphxyXtitleC(graph,ajSeqName(seq1));
    ajGraphxyYtitleC(graph,ajSeqName(seq2));

    ajGraphDataxySetTypeC(gdata,"2D Plot Float");
    ajGraphDataxySetMaxMin(gdata,(float)begin1,(float)end1,(float)begin2,
			   (float)end2);
    ajGraphDataxySetMaxima(gdata,(float)begin1,(float)end1,(float)begin2,
			   (float)end2);
    ajGraphxySetXStart(graph,(float)begin1);
    ajGraphxySetXEnd(graph,(float)end1);
    ajGraphxySetYStart(graph,(float)begin2);
    ajGraphxySetYEnd(graph,(float)end2);

    ajGraphxySetXRangeII(graph,begin1,end1);
    ajGraphxySetYRangeII(graph,begin2,end2);


    if(matchlist)
    {
	iter = ajListIter(matchlist);
	while((wmp = ajListIterNext(iter)))
	{
	    x1 = x2 = (float) (wmp->seq1start + begin1);
	    y1 = y2 = (float) (wmp->seq2start + begin2);
	    x2 += (float) wmp->length-1;
	    y2 += (float) wmp->length-1;
	    ajGraphObjAddLine(graph,x1,y1,x2,y2,0);
	}
	ajListIterFree(iter);
    }

    ajGraphxyAddDataPtrPtr(gdata,xa,ya);
    ajGraphxyReplaceGraph(graph,gdata);


    ajGraphxyDisplay(graph,ajFalse);
    ajGraphClose();

    ajStrDel(&tit);

    return;
}

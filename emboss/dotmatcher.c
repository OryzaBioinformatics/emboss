/* @source dotmatcher application
**
** dotmatcher displays a dotplot for two sequences.
**
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
** @modified: Added non-proportional plot. Copyright (C) Alan Bleasby
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
#include "ajtime.h"




static void dotmatcher_pushpoint(AjPList *l, float x1, float y1, float x2,
				 float y2, AjBool stretch);




/* @datastatic PPoint *********************************************************
**
** Dotmatcher point data
**
** @alias SPoint
** @alias OPoint
**
** @attr x1 [float] x1 coordinate
** @attr y1 [float] y1 coordinate
** @attr x2 [float] x2 coordinate
** @attr y2 [float] y2 coordinate
******************************************************************************/

typedef struct SPoint
{
    float x1;
    float y1;
    float x2;
    float y2;
} OPoint, *PPoint;




/* @prog dotmatcher ***********************************************************
**
** Displays a thresholded dotplot of two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPList list = NULL;
    AjPSeq seq;
    AjPSeq seq2;
    AjPStr aa0str = 0;
    AjPStr aa1str = 0;
    char *s1;
    char *s2;
    char *strret = NULL;
    ajint i;
    ajint j;
    ajint k;
    ajint l;
    ajint abovethresh;
    ajint total;
    ajint starti = 0;
    ajint startj = 0;
    ajint windowsize;
    float thresh;
    AjPGraph graph   = NULL;
    AjPGraph xygraph = NULL;

    AjOTime ajtime;
    time_t tim;
    AjBool boxit=AJTRUE;
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
    float xmargin;
    float ymargin;
    float ticklen;
    float tickgap;
    float onefifth;
    float k2;
    float max;
    char ptr[10];
    AjPMatrix matrix = NULL;
    ajint** sub;
    AjPSeqCvt cvt;
    AjPStr  subt = NULL;

    ajint b1;
    ajint b2;
    ajint e1;
    ajint e2;
    AjPStr se1;
    AjPStr se2;
    ajint ithresh;
    AjBool stretch;
    PPoint ppt = NULL;
    float xa[1];
    float ya[1];
    AjPGraphData gdata=NULL;
    AjPStr tit   = NULL;
    AjIList iter = NULL;
    float x1 = 0.;
    float x2 = 0.;
    float y1 = 0.;
    float y2 = 0.;

    se1 = ajStrNew();
    se2 = ajStrNew();

    tim = time(0);

    ajTimeLocal(tim,&ajtime);
    ajtime.format = 0;
    
    ajGraphInit("dotmatcher", argc, argv);
    
    seq        = ajAcdGetSeq("asequence");
    seq2       = ajAcdGetSeq("bsequence");
    stretch    = ajAcdGetBool("stretch");
    graph      = ajAcdGetGraph("graph");
    xygraph    = ajAcdGetGraphxy("xygraph");
    windowsize = ajAcdGetInt("windowsize");
    ithresh    = ajAcdGetInt("threshold");
    matrix     = ajAcdGetMatrix("matrixfile");
    
    sub = ajMatrixArray(matrix);
    cvt = ajMatrixCvt(matrix);
    
    thresh = (float)ithresh;
    
    b1 = ajSeqBegin(seq);
    b2 = ajSeqBegin(seq2);
    e1 = ajSeqEnd(seq);
    e2 = ajSeqEnd(seq2);
    
    ajStrAssSubC(&se1,ajSeqChar(seq),b1-1,e1-1);
    ajStrAssSubC(&se2,ajSeqChar(seq2),b2-1,e2-1);
    ajSeqReplace(seq,se1);
    ajSeqReplace(seq2,se2);
    
    
    s1 = ajStrStr(ajSeqStr(seq));
    s2 = ajStrStr(ajSeqStr(seq2));
    
    
    aa0str = ajStrNewL(1+ajSeqLen(seq)); /* length plus trailing blank */
    aa1str = ajStrNewL(1+ajSeqLen(seq2));
    
    list = ajListNew();
    
    
    for(i=0;i<ajSeqLen(seq);i++)
	ajStrAppK(&aa0str,(char)ajSeqCvtK(cvt, *s1++));
    
    for(i=0;i<ajSeqLen(seq2);i++)
	ajStrAppK(&aa1str,(char)ajSeqCvtK(cvt, *s2++));
    
    max= ajSeqLen(seq);
    if(ajSeqLen(seq2) > max)
	max = ajSeqLen(seq2);
    
    xmargin = ymargin = max *0.15;
    ticklen = xmargin*0.1;
    onefifth  = xmargin*0.2;
    
    subt = ajStrNewC((strret=
		      ajFmtString("(windowsize = %d, threshold = %3.2f  %D)",
				  windowsize,thresh,&ajtime)));
    
    
    if(!stretch)
	if( ajStrLen(graph->subtitle) <=1)
	    ajStrApp(&graph->subtitle,subt);
    
    
    if(!stretch)
    {
	ajGraphOpenWin(graph, 0.0-ymargin,(max*1.35)+ymargin,
		       0.0-xmargin,(float)max+xmargin);

	ajGraphTextMid(max*0.5,(ajSeqLen(seq2))+xmargin-onefifth,
		       ajStrStr(graph->title));
	ajGraphTextMid((ajSeqLen(seq))*0.5,0.0-(xmargin/2.0),
		       ajStrStr(graph->xaxis));
	ajGraphTextLine(0.0-(xmargin*0.75),(ajSeqLen(seq2))*0.5,
			0.0-(xmargin*0.75),(ajSeqLen(seq)),
			ajStrStr(graph->yaxis),0.5);

	ajGraphSetCharSize(0.5);
	ajGraphTextMid(max*0.5,(ajSeqLen(seq2))+xmargin-(onefifth*3),
		       ajStrStr(graph->subtitle));
    }
    
    
    
    s1= ajStrStr(aa0str);
    s2 = ajStrStr(aa1str);
    
    for(j=0;j < ajSeqLen(seq2)-windowsize;j++)
    {
	i =0;
	total = 0;
	abovethresh =0;

	k = j;
	for(l=0;l<windowsize;l++)
	    total = total + sub[(ajint)s1[i++]][(ajint)s2[k++]];

	if(total >= thresh)
	{
	    abovethresh=1;
	    starti = i-windowsize;
	    startj = k-windowsize;
	}

	while(i < ajSeqLen(seq) && k < ajSeqLen(seq2))
	{
	    total = total - sub[(ajint)s1[i-windowsize]]
		[(ajint)s2[k-windowsize]];
	    total = total + sub[(ajint)s1[i]][(ajint)s2[k]];

	    if(abovethresh)
	    {
		if(total < thresh)
		{
		    abovethresh = 0;
		    /* draw the line */
		    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
					 (float)i-1,(float)k-1,stretch);
		}
	    }
	    else if(total >= thresh)
	    {
		starti = i-windowsize;
		startj = k-windowsize;
		abovethresh= 1;
	    }
	    i++;
	    k++;
	}

	if(abovethresh)
	    /* draw the line */
	    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
				 (float)i-1,(float)k-1,
				 stretch);
    }
    
    for(i=0;i < ajSeqLen(seq)-windowsize;i++)
    {
	j = 0;
	total = 0;
	abovethresh =0;

	k = i;
	for(l=0;l<windowsize;l++)
	    total = total + sub[(ajint)s1[k++]][(ajint)s2[j++]];

	if(total >= thresh)
	{
	    abovethresh=1;
	    starti = k-windowsize;
	    startj = j-windowsize;
	}

	while(k < ajSeqLen(seq) && j < ajSeqLen(seq2))
	{
	    total = total - sub[(ajint)s1[k-windowsize]]
		[(ajint)s2[j-windowsize]];
	    total = total + sub[(ajint)s1[k]][(ajint)s2[j]];

	    if(abovethresh)
	    {
		if(total < thresh)
		{
		    abovethresh = 0;
		    /* draw the line */
		    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
					 (float)k-1,(float)j-1,stretch);
		}
	    }
	    else if(total >= thresh)
	    {
		starti = k-windowsize;
		startj = j-windowsize;
		abovethresh= 1;
	    }
	    j++;
	    k++;
	}

	if(abovethresh)
	    /* draw the line */
	    dotmatcher_pushpoint(&list,(float)starti,(float)startj,
				 (float)k-1,(float)j-1,
				 stretch);
    }
    
    if(boxit && !stretch)
    {
	ajGraphRect(0.0,0.0,(float)ajSeqLen(seq),(float)ajSeqLen(seq2));

	i=0;
	while(acceptableticksx[i]*numbofticks < ajSeqLen(seq))
	    i++;

	if(i<=13)
	    tickgap = acceptableticksx[i];
	else
	    tickgap = acceptableticksx[10];
	ticklen   = xmargin*0.1;
	onefifth  = xmargin*0.2;

	if(ajSeqLen(seq2)/ajSeqLen(seq) > 10 )
	{
	    /* alot smaller then just label start and end */
	    ajGraphLine(0.0,0.0,0.0,0.0-ticklen);
	    sprintf(ptr,"%d",b1-1);
	    ajGraphTextMid( 0.0,0.0-(onefifth),ptr);

	    ajGraphLine((float)(ajSeqLen(seq)),0.0,
			(float)ajSeqLen(seq),0.0-ticklen);
	    sprintf(ptr,"%d",ajSeqLen(seq)+b1-1);
	    ajGraphTextMid((float)ajSeqLen(seq),0.0-(onefifth),ptr);

	}
	else
	    for(k2=0.0;k2<ajSeqLen(seq);k2+=tickgap)
	    {
		ajGraphLine(k2,0.0,k2,0.0-ticklen);
		sprintf(ptr,"%d",(ajint)k2+b1-1);
		ajGraphTextMid( k2,0.0-(onefifth),ptr);
	    }

	i = 0;
	while(acceptableticks[i]*numbofticks < ajSeqLen(seq2))
	    i++;

	tickgap   = acceptableticks[i];
	ticklen   = ymargin*0.01;
	onefifth  = ymargin*0.02;

	if(ajSeqLen(seq)/ajSeqLen(seq2) > 10 )
	{
	    /* alot smaller then just label start and end */
	    ajGraphLine(0.0,0.0,0.0-ticklen,0.0);
	    sprintf(ptr,"%d",b2-1);
	    ajGraphTextEnd( 0.0-(onefifth),0.0,ptr);

	    ajGraphLine(0.0,(float)ajSeqLen(seq2),0.0-ticklen,
			(float)ajSeqLen(seq2));
	    sprintf(ptr,"%d",ajSeqLen(seq2)+b2-1);
	    ajGraphTextEnd( 0.0-(onefifth),(float)ajSeqLen(seq2),ptr);
	}
	else
	    for(k2=0.0;k2<ajSeqLen(seq2);k2+=tickgap)
	    {
		ajGraphLine(0.0,k2,0.0-ticklen,k2);
		sprintf(ptr,"%d",(ajint)k2+b2-1);
		ajGraphTextEnd( 0.0-(onefifth),k2,ptr);
	    }
    }
    
    
    if(!stretch)
	ajGraphClose();
    else			/* the xy graph for -stretch */
    {
	tit = ajStrNew();
	ajFmtPrintS(&tit,"%S",xygraph->title);


	gdata = ajGraphxyDataNewI(1);
	xa[0] = (float)b1;
	ya[0] = (float)b2;

	ajGraphxyTitleC(xygraph,ajStrStr(tit));

	ajGraphxyXtitleC(xygraph,ajSeqName(seq));
	ajGraphxyYtitleC(xygraph,ajSeqName(seq2));

	ajGraphDataxySetTypeC(gdata,"2D Plot Float");
	ajGraphxyDataSetTitle(gdata,subt);
	ajGraphDataxySetMaxMin(gdata,(float)b1,(float)e1,(float)b2,
			       (float)e2);
	ajGraphDataxySetMaxima(gdata,(float)b1,(float)e1,(float)b2,
			       (float)e2);
	ajGraphxySetXStart(xygraph,(float)b1);
	ajGraphxySetXEnd(xygraph,(float)e1);
	ajGraphxySetYStart(xygraph,(float)b2);
	ajGraphxySetYEnd(xygraph,(float)e2);

	ajGraphxySetXRangeII(xygraph,b1,e1);
	ajGraphxySetYRangeII(xygraph,b2,e2);


	if(list)
	{
	    iter = ajListIter(list);
	    while((ppt = ajListIterNext(iter)))
	    {
		x1 = ppt->x1+b1-1;
		y1 = ppt->y1+b2-1;
		x2 = ppt->x2+b1-1;
		y2 = ppt->y2+b2-1;
		ajGraphObjAddLine(xygraph,x1,y1,x2,y2,0);
	    }
	    ajListIterFree(iter);
	}

	ajGraphxyAddDataPtrPtr(gdata,xa,ya);
	ajGraphxyReplaceGraph(xygraph,gdata);


	ajGraphxyDisplay(xygraph,ajFalse);
	ajGraphClose();

	ajStrDel(&tit);
    }
    
    
    
    ajListDel(&list);
    
    
    /* deallocate memory */
    ajStrDel(&aa0str);
    ajStrDel(&aa1str);
    ajStrDel(&se1);
    ajStrDel(&se2);
    
    AJFREE(strret);			/* created withing ajFmtString */
    
    ajExit();

    return 0;
}




/* @funcstatic dotmatcher_pushpoint *******************************************
**
** Undocumented.
**
** @param [?] l [AjPList*] Undocumented
** @param [?] x1 [float] Undocumented
** @param [?] y1 [float] Undocumented
** @param [?] x2 [float] Undocumented
** @param [?] y2 [float] Undocumented
** @param [r] stretch [AjBool] Do a stretch plot
** @return [void]
** @@
******************************************************************************/

static void dotmatcher_pushpoint(AjPList *l, float x1, float y1, float x2,
				 float y2, AjBool stretch)
{
    PPoint p;

    if(!stretch)
    {
	ajGraphLine(x1+1,y1+1,x2+1,y2+1);
	return;
    }

    AJNEW0(p);
    p->x1 = x1+1;
    p->y1 = y1+1;
    p->x2 = x2+1;
    p->y2 = y2+1;
    ajListPush(*l,(void *)p);

    return;
}

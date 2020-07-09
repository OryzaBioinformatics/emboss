/* @source dotmatcher application
**
** dotmatcher displays a dotplot for two sequences.
**
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
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



static void pushpoint(AjPList *l, float x1, float y1, float x2, float y2,
		      AjBool text);
static void datapoints(AjPList *l, AjPFile outf);




typedef struct SPoint
{
    float x1;
    float y1;
    float x2;
    float y2;
} OPoint, *PPoint;



int main(int argc, char **argv)
{
    AjPList list=NULL;
    AjPSeq seq,seq2;
    AjPStr aa0str=0,aa1str=0;
    char *s1,*s2;
    char *strret=NULL;
    ajint i,j,k,l,abovethresh,total;
    ajint starti=0,startj=0;
    ajint windowsize;
    float thresh;
    AjPGraph graph = 0;
    /* AjPStr title=0,subtitle=0;*/
    AJTIME ajtime;
    const time_t tim = time(0);
    AjBool boxit=AJTRUE;
    /* Different ticks as they need to be different for x and y due to
       length of string being important on x */
    ajint acceptableticksx[]={1,10,50,100,500,1000,1500,10000,
				500000,1000000,5000000};
    ajint acceptableticks[]={1,10,50,100,200,500,1000,2000,5000,10000,15000,
			       500000,1000000,5000000};
    ajint numbofticks = 10;
    float xmargin,ymargin;
    float ticklen,tickgap;
    float onefifth;
    float k2,max;
    char ptr[10];
    AjPMatrix matrix = NULL;
    ajint** sub;
    AjPSeqCvt cvt;
    AjBool text;
    AjPFile outf=NULL;
    AjPStr  subt=NULL;

    ajint b1;
    ajint b2;
    ajint e1;
    ajint e2;
    AjPStr se1;
    AjPStr se2;

    se1 = ajStrNew();
    se2 = ajStrNew();
    
    
    ajtime.time = localtime(&tim);
    ajtime.format = 0;

    ajGraphInit("dotmatcher", argc, argv);

    seq = ajAcdGetSeq ("sequencea");
    seq2 = ajAcdGetSeq ("sequenceb");
    graph = ajAcdGetGraph ("graph");
    windowsize = ajAcdGetInt("windowsize");
    thresh = ajAcdGetFloat("threshold");  
    matrix  = ajAcdGetMatrix("matrixfile");
    text = ajAcdGetBool("data");
    outf = ajAcdGetOutfile("outfile");
  
    sub = ajMatrixArray(matrix);
    cvt = ajMatrixCvt(matrix);

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
    

    if(!text)
	if( ajStrLen(graph->subtitle) <=1)
	    ajStrApp(&graph->subtitle,subt);


    if(!text)
    {
	ajGraphOpenWin(graph, 0.0-ymargin,(max*1.35)+ymargin,
		       0.0-xmargin,(float)max+xmargin);

	ajGraphTextMid (max*0.5,(ajSeqLen(seq2))+xmargin-onefifth,
			ajStrStr(graph->title));
	ajGraphTextMid ((ajSeqLen(seq))*0.5,0.0-(xmargin/2.0),
			ajStrStr(graph->xaxis));
	ajGraphTextLine (0.0-(xmargin*0.75),(ajSeqLen(seq2))*0.5,
			 0.0-(xmargin*0.75),(ajSeqLen(seq)),
			 ajStrStr(graph->yaxis),0.5);
  
	ajGraphSetCharSize(0.5);
	ajGraphTextMid (max*0.5,(ajSeqLen(seq2))+xmargin-(onefifth*3),
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
	    total = total - sub[(ajint)s1[i-windowsize]][(ajint)s2[k-windowsize]];
	    total = total + sub[(ajint)s1[i]][(ajint)s2[k]];
	    if(abovethresh)
	    {
		if(total < thresh)
		{
		    abovethresh = 0;
		    /* draw the line */
		    pushpoint(&list,(float)starti,(float)startj,
				(float)i-1,(float)k-1,text);
		}	    
	    }
	    else if (total >= thresh)
	    {
		starti = i-windowsize;
		startj = k-windowsize;
		abovethresh= 1;
	    }      
	    i++;k++;
	}
	if(abovethresh)
	{
	    /* draw the line */
	    pushpoint(&list,(float)starti,(float)startj,(float)i-1,(float)k-1,
		      text);
	}
    }

    for(i=0;i < ajSeqLen(seq)-windowsize;i++)
    {
	j =0;
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
	    total = total - sub[(ajint)s1[k-windowsize]][(ajint)s2[j-windowsize]];
	    total = total + sub[(ajint)s1[k]][(ajint)s2[j]];
	    if(abovethresh)
	    {
		if(total < thresh)
		{
		    abovethresh = 0;
		    /* draw the line */
		    pushpoint(&list,(float)starti,(float)startj,
				(float)k-1,(float)j-1,text);
		}	    
	    }
	    else if (total >= thresh)
	    {
		starti = k-windowsize;
		startj = j-windowsize;
		abovethresh= 1;
	    }      
	    j++;k++;
	}
	if(abovethresh)
	{
	    /* draw the line */
	    /*      printf("line (%d,%d) (%d,%d)\n",starti,startj,i-1,k-1);*/
	    pushpoint(&list,(float)starti,(float)startj,(float)k-1,(float)j-1,
		      text);
	}
    }

    if(!text && boxit)
    {
	ajGraphRect( 0.0,0.0,(float)ajSeqLen(seq),(float)ajSeqLen(seq2));

	i=0;
	while(acceptableticksx[i]*numbofticks < ajSeqLen(seq))
	    i++;

	if(i<=13)
	    tickgap = acceptableticksx[i];
	else
	    /*      tickgap = acceptableticksx[13]; bugfixed AJB 26/7/00 */
	    tickgap = acceptableticksx[10];
	ticklen = xmargin*0.1;
	onefifth  = xmargin*0.2;
	if(ajSeqLen(seq2)/ajSeqLen(seq) > 10 )
	{		/* alot smaller then just label start and end */
	    ajGraphLine(0.0,0.0,0.0,0.0-ticklen);
	    sprintf(ptr,"%d",b1-1);
	    ajGraphTextMid ( 0.0,0.0-(onefifth),ptr);
      
	    ajGraphLine((float)(ajSeqLen(seq)),0.0,
			(float)ajSeqLen(seq),0.0-ticklen);
	    sprintf(ptr,"%d",ajSeqLen(seq)+b1-1);
	    ajGraphTextMid ( (float)ajSeqLen(seq),0.0-(onefifth),ptr);
      
	}
	else
	{
	    for(k2=0.0;k2<ajSeqLen(seq);k2+=tickgap)
	    {
		ajGraphLine(k2,0.0,k2,0.0-ticklen);
		sprintf(ptr,"%d",(ajint)k2+b1-1);
		ajGraphTextMid ( k2,0.0-(onefifth),ptr);
	    }
	}
    
	i=0;
	while(acceptableticks[i]*numbofticks < ajSeqLen(seq2))
	    i++;

	tickgap = acceptableticks[i];
	ticklen = ymargin*0.01;
	onefifth  = ymargin*0.02;
	if(ajSeqLen(seq)/ajSeqLen(seq2) > 10 )
	{		/* alot smaller then just label start and end */
	    ajGraphLine(0.0,0.0,0.0-ticklen,0.0);
	    sprintf(ptr,"%d",b2-1);
	    ajGraphTextEnd ( 0.0-(onefifth),0.0,ptr);
      
	    ajGraphLine(0.0,(float)ajSeqLen(seq2),0.0-ticklen,
			(float)ajSeqLen(seq2));
	    sprintf(ptr,"%d",ajSeqLen(seq2)+b2-1);
	    ajGraphTextEnd ( 0.0-(onefifth),(float)ajSeqLen(seq2),ptr);
	}
	else
	{
	    for(k2=0.0;k2<ajSeqLen(seq2);k2+=tickgap)
	    {
		ajGraphLine(0.0,k2,0.0-ticklen,k2);
		sprintf(ptr,"%d",(ajint)k2+b2-1);
		ajGraphTextEnd ( 0.0-(onefifth),k2,ptr);
	    }
	}
    }


    if(!text)
	ajGraphClose();
    else
    {
	ajFmtPrintF(outf,"##2D Plot\n##Title dotmatcher: %s vs %s\n",
		    ajSeqName(seq),ajSeqName(seq2));
	ajFmtPrintF(outf,"##Graphs 1\n##Number 1\n##Points 0\n");
	ajFmtPrintF(outf,"##XminA %f XmaxA %f YminA %f YmaxA %f\n",0.,
		    (float)ajSeqLen(seq),0.,(float)ajSeqLen(seq2));
	ajFmtPrintF(outf,"##Xmin %f Xmax %f Ymin %f Ymax %f\n",0.,
		    (float)ajSeqLen(seq),0.,(float)ajSeqLen(seq2));
	ajFmtPrintF(outf,"##ScaleXmin %f ScaleXmax %f "
		    "ScaleYmin %f ScaleYmax %f\n",0.,(float)ajSeqLen(seq),0.,
		    (float)ajSeqLen(seq2));
	ajFmtPrintF(outf,"##Maintitle %S\n",subt);
	ajFmtPrintF(outf,"##Xtitle %s\n##Ytitle %s\n",ajSeqName(seq),
		    ajSeqName(seq2));
	ajFmtPrintF(outf,"##DataObjects\n##Number %d\n",
		    ajListLength(list));

	datapoints(&list,outf);

	ajFmtPrintF(outf,"##GraphObjects\n##Number 0\n");

    }
  
    ajListDel(&list);


    /* deallocate memory */
    ajStrDel(&aa0str);
    ajStrDel(&aa1str);
    ajStrDel(&se1);
    ajStrDel(&se2);
    
    AJFREE (strret);			/* created withing ajFmtString */

    ajExit();
    return 0;
}


static void pushpoint(AjPList *l, float x1, float y1, float x2, float y2,
		      AjBool text)
{
    PPoint p;

    if(!text)
    {
	ajGraphLine(x1,y1,x2,y2);
	return;
    }
    
    AJNEW0(p);
    p->x1=x1;
    p->y1=y1;
    p->x2=x2;
    p->y2=y2;
    ajListPush(*l,(void *)p);
    return;
}


static void datapoints(AjPList *l,AjPFile outf)
{
    PPoint p;

    while(ajListPop(*l,(void **)&p))
    {
	ajFmtPrintF(outf,"Line x1 %f y1 %f x2 %f y2 %f colour 0\n",
		    p->x1,p->y1,p->x2,p->y2);
	AJFREE(p);
    }

    return;
}

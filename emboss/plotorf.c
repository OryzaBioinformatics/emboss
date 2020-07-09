/* @source plotorf application
**
** Plot potential open reading frames
**
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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
#include <string.h>


static void plotorf_norfs(char *seq, char *rev, ajint n, float **x, float **y,
			  AjPInt *cnt, ajint beg, AjPStr *starts,
			  ajint nstarts, AjPStr *stops, ajint nstops);
static AjBool plotorf_isin(char *p, AjPStr *str, ajint n);



/* @prog plotorf **************************************************************
**
** Plot potential open reading frames
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq     seq;
    AjPStr     str;
    AjPStr     rev;
    AjPStr     *starts=NULL;
    AjPStr     *stops=NULL;
    AjPStr     start;
    AjPStr     stop;
    ajint      nstarts;
    ajint      nstops;
    
    AjPGraph   graph;
    AjPGraphData data;
    
    float      *x[6];
    float      *y[6];
    AjPInt     cnt;
    ajint        beg;
    ajint	       end;
    
    ajint        i;
    ajint        j;
    
    char *ftit[6]=
    {
	"F1","F2","F3","R1","R2","R3"
    };
    
    
    
	
    
    (void) ajGraphInit("plotorf", argc, argv);

    seq       = ajAcdGetSeq("sequence");
    graph     = ajAcdGetGraphxy("graph");
    start     = ajAcdGetString("start");
    stop      = ajAcdGetString("stop");

    ajStrToUpper(&start);
    ajStrToUpper(&stop);

    nstarts = ajArrCommaList(start,&starts);
    nstops  = ajArrCommaList(stop,&stops);

    beg = ajSeqBegin(seq);
    end = ajSeqEnd(seq);

    str = ajStrNew();
    cnt = ajIntNew();
    
    ajSeqToUpper(seq);
    (void) ajStrAssSubC(&str,ajSeqChar(seq),beg-1,end-1);
    
    rev = ajStrNewC(ajStrStr(str));
    ajSeqReverseStr(&rev);

    for(i=0;i<6;++i)
    {
	plotorf_norfs(ajStrStr(str),ajStrStr(rev),i,x,y,&cnt,beg,starts,
		      nstarts,stops,nstops);
	data = ajGraphxyDataNewI(2);
	data->numofpoints = 0;
	
	
	ajGraphxyAddGraph(graph,data);
	ajGraphxySetOverLap(graph,ajFalse);
	ajGraphxySetYTick(graph, ajFalse);
	ajGraphDataxySetMaxima(data,(float)beg,(float)end,0.0,1.0);
	ajGraphDataxySetTypeC(data,"Multi 2D Plot Small");
	ajGraphxyDataSetYtitleC(data,"Orf");
	ajGraphxyDataSetXtitleC(data,"Sequence");
	ajGraphxyDataSetTitleC(data,ftit[i]);

	for(j=0;j<ajIntGet(cnt,i);++j)
	    ajGraphDataObjAddRect(data,y[i][j],0.0,
					      x[i][j],1.0,4,1);
    }
    

    ajGraphxySetTitleDo(graph, ajTrue);
    ajGraphxySetMaxMin(graph,(float)beg,(float)end,0.0,1.0);
    
    ajGraphxySetYStart(graph,0.0);
    ajGraphxySetYEnd(graph,2.0);
    ajGraphxyTitleC(graph,"Potential codons (rectangles)");
    ajGraphxyDisplay(graph,ajTrue);
    
 
    ajStrDel(&str);
    ajStrDel(&rev);
    ajStrDel(&start);
    ajStrDel(&stop);
    ajIntDel(&cnt);

    for(i=0;i<nstarts;++i)
	ajStrDel(&starts[i]);
    AJFREE(starts);
    for(i=0;i<nstops;++i)
	ajStrDel(&stops[i]);
    AJFREE(stops);
    
    ajExit();
    return 0;
}

/* @funcstatic plotorf_norfs **************************************************
**
** Undocumented.
**
** @param [r] seq [char*] nucleic sequence
** @param [r] rev [char*] reverse sequence
** @param [r] n [ajint] length
** @param [w] x [float**] xpos
** @param [w] y [float**] ypos
** @param [w] cnt [AjPInt*] orf count
** @param [r] beg [ajint] sequence strat
** @param [w] starts [AjPStr*] start posns
** @param [r] nstarts [ajint] number of starts
** @param [w] stops [AjPStr*] stop posns
** @param [r] nstops [ajint] number of stops
** @@
******************************************************************************/

static void plotorf_norfs(char *seq, char *rev, ajint n, float **x, float **y,
			  AjPInt *cnt, ajint beg, AjPStr *starts,
			  ajint nstarts, AjPStr *stops, ajint nstops)
{

    ajint len;
    ajint i;
    ajint count;
    AjBool inframe;
    ajint po;
    char *p;
    


    len = strlen(seq);
    if(n<3)
    {
	p = seq;
	po = n%3;
    }
    else
    {
	p=rev;
	po=len%3;
	po-=n%3;
	if(po<0)
	    po+=3;
    }
    



    inframe=ajFalse;
    count = 0;
    
    for(i=po;i<len-2;i+=3)
    {
	if(plotorf_isin(&p[i],starts,nstarts))
	{
	    if(!inframe)
	    {
		++count;
		inframe=ajTrue;
		continue;
	    }
	}
	
	if(plotorf_isin(&p[i],stops,nstops))
	    if(inframe)
		inframe=ajFalse;
    }

    if(count)
    {
      AJCNEW (x[n], count);
      AJCNEW (y[n], count);
    }
    (void) ajIntPut(cnt,n,count);

    
    count = 0;
    inframe = ajFalse;
    
    for(i=po;i<len-2;i+=3)
    {
	if(plotorf_isin(&p[i],starts,nstarts))
	{
	    if(!inframe)
	    {
		if(ajIntGet(*cnt,n))
		{
		    if(n<3)
			x[n][count]=(float)(i+beg);
		    else
			x[n][count]=(float)((len-i-1)+beg);
		}
		++count;
		inframe=ajTrue;
		continue;
	    }
	}

	if(plotorf_isin(&p[i],stops,nstops))
	    if(inframe)
	    {
		if(ajIntGet(*cnt,n))
		{
		    if(n<3)
			y[n][count-1]=(float)(i+beg);
		    else
			y[n][count-1]=(float)((len-i-1)+beg);
		}
		inframe=ajFalse;
	    }
    }

    if(inframe)
    {
	if(ajIntGet(*cnt,n))
	{
	    if(n<3)
		y[n][count-1]=(float)(len+beg-1);
	    else
		y[n][count-1]=(float) beg;
	}
    }

    
    
    return;
}

/* @funcstatic plotorf_isin **************************************************
**
** True if codon at p occurs in string str
**
** @param [r] p [char*] codon
** @param [r] str [AjPStr*] sequence
** @param [r] n [ajint] str length
** @return [AjBool] true if found
** @@
******************************************************************************/

static AjBool plotorf_isin(char *p, AjPStr *str, ajint n)
{
    ajint i;
    AjBool ret;

    ret = ajFalse;

    for(i=0;i<n && !ret;++i)
	if(!strncmp(p,ajStrStr(str[i]),3))
	    ret = ajTrue;

    return ret;
}

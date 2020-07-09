/* @source hmoment application
**
** Calculate hydrophobic moment
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
#include <math.h>


static float calchm(char *p, int pos, int window, ajint angle);
static void  addgraph(AjPGraph graph, ajint limit, float *x, float *y,
		      float ymax, ajint colour, ajint angle,
		      ajint window, float baseline);


int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq     seq;
    AjPFile    outf;
    AjPStr     str=NULL;
    AjPStr     st=NULL;
    
    AjBool     plot;
    AjPGraph   graph=NULL;
    AjBool     twin;
    float      baseline;
    float      ymax;
    
    ajint beg;
    ajint end;
    ajint window;
    ajint len;
    ajint limit;

    ajint aangle;
    ajint bangle;
    
    ajint i;

    float *x=NULL;
    float *ya=NULL;
    float *yb=NULL;
    


    char *p;
    char *sname;
    
    
    ajGraphInit("hmoment", argc, argv);

    seqall    = ajAcdGetSeqall("seqall");
    plot      = ajAcdGetBool("plot");
    window    = ajAcdGetInt("window");
    aangle    = ajAcdGetInt("aangle");
    bangle    = ajAcdGetInt("bangle");
    baseline  = ajAcdGetFloat("baseline");
    twin      = ajAcdGetBool("double");
    
    if(!plot)
	outf  = ajAcdGetOutfile("outfile");
    else
	graph = ajAcdGetGraphxy("graph");
    

    str = ajStrNew();
    st  = ajStrNew();
    
    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallBegin(seqall);
	end = ajSeqallEnd(seqall);
	len = end-beg+1;
	
	limit = len-window+1;
	sname = ajSeqName(seq);
	
	ajStrAssSubC(&str,ajSeqChar(seq),--beg,--end);
	ajStrToUpper(&str);
	p = ajStrStr(str);

	AJCNEW0(x,limit);
	AJCNEW0(ya,limit);
	AJCNEW0(yb,limit);
	

	for(i=0;i<limit;++i)
	{
	    x[i] = (float)i+1+beg;
	    ya[i] = calchm(p,i,window,aangle);
	    yb[i] = calchm(p,i,window,bangle);
	}

	for(i=0,ymax=-100.;i<limit;++i)
	{
	    ymax = ymax > ya[i] ? ymax : ya[i];
	    if(twin)
		ymax = ymax > yb[i] ? ymax : yb[i];
	}

	if(!plot)
	{
	    ajFmtPrintF(outf,"HMOMENT of %s from %d to %d\n\n",sname,
			beg+1,end+1);
	    if(twin)
	    {
		ajFmtPrintF(outf,"Window: %d First angle: %d second angle:"
			    " %d Max uH: %.3f\n",window, aangle, bangle,
			    ymax);
		ajFmtPrintF(outf,"Position\tFirst\tSecond\n");
		for(i=0;i<limit;++i)
		    ajFmtPrintF(outf,"%d\t\t%.3lf\t%.3lf\n",(int)x[i],ya[i],
				yb[i]);
	    }
	    else
	    {
		ajFmtPrintF(outf,"Window: %d Angle: %d Max uH: %.3f\n",
			    window, aangle, ymax);
		ajFmtPrintF(outf,"Position\tAngle\n");
		for(i=0;i<limit;++i)
		    ajFmtPrintF(outf,"%d\t\t%.3lf\n",(int)x[i],ya[i]);
	    }

	}
	else
	{
	    if(twin)
		ajGraphSetMulti(graph,2);
	    else
		ajGraphSetMulti(graph,1);

	    ajGraphxySetOverLap(graph,ajFalse);
	    ajFmtPrintS(&st,"HMOMENT of %s. Window:%d",sname,window);

	    addgraph(graph,limit,x,ya,ymax,BLACK,aangle,window,baseline);
	    if(twin)
		addgraph(graph,limit,x,yb,ymax,RED,bangle,window,baseline);

	    ajGraphxyDisplay(graph,ajTrue);
	}
	

	AJFREE(x);
	AJFREE(ya);
	AJFREE(yb);
    }
    
    if(!plot)
	ajFileClose(&outf);
    ajStrDel(&str);
    ajStrDel(&st);

    ajExit();
    return 0;
}



static void addgraph(AjPGraph graph, ajint limit, float *x, float *y,
		     float ymax, ajint colour, ajint angle, ajint window,
		     float baseline)
{
    ajint i;

    AjPGraphData data;
    AjPStr st=NULL;


    data = ajGraphxyDataNewI(limit);

    st = ajStrNew();

    for(i=0;i<limit;++i)
    {
	data->x[i] = x[i];
	data->y[i] = y[i];
    }
    
    ajGraphxySetColour(data,colour);
    ajGraphDataxySetMaxMin(data,x[0],x[limit-1],0.,ymax);  

    ajFmtPrintS(&st,"uH (%d deg)",angle);
    ajGraphxyDataSetYtitle(data,st);

    ajFmtPrintS(&st,"Position (w=%d)",window);    
    ajGraphxyDataSetXtitle(data,st);

    ajGraphDataObjAddLine(data,x[0],baseline,x[limit-1],baseline,BLUE);

    ajGraphxyAddGraph(graph,data);

    ajStrDel(&st);

    return;
}



static float calchm(char *p, int pos, int window, ajint angle)
{
    ajint  i;
    double h;
    ajint  res;
    double sumsin;
    double sumcos;
    double tangle;
    double hm;

    double hydata[]=
    {
	.62, -100., .29, -.90, -.74, 1.19, .48, -.4, 1.38, -100., -1.5, 1.06,
	0.64, -.78, -100., 0.12, -.85, -2.53, -.18, -.05, -100., 1.08, .81,
	-100., .26, -100.
    };

    sumsin = sumcos = (double)0.;
    tangle = angle;
    
    for(i=0;i<window;++i)
    {
	res = p[pos+i];
	h   = hydata[ajAZToInt(res)];
	
	sumsin  += (h * sin(ajDegToRad(tangle)));
	sumcos  += (h * cos(ajDegToRad(tangle)));
	tangle = (double) (((ajint)tangle+angle) % 360);
    }


    sumsin *= sumsin;
    sumcos *= sumcos;

    
    hm = sqrt(sumsin+sumcos) / (double)window;

    return (float)hm;
}
/* @source charge application
**
** Calculate protein charge within a sliding window
**
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @modified: David Martin July 2001 (dmartin@hgmp.mrc.ac.uk)
** @modified: Alan Bleasby Oct 2001 (ableasby@hgmp.mrc.ac.uk)
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

#define AMINOFILE "Eamino.dat"




static void  charge_addgraph(AjPGraph graph, ajint limit, float *x,
			     float *y, float ymax, float ymin,
			     ajint window, char *sname);
static AjPFloat charge_read_amino(AjPFile* fp);




/* @prog charge ***************************************************************
**
** Protein charge plot
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall  seqall;
    AjPSeq     seq;

    AjPFloat   chg = NULL;

    AjPFile    outf;
    AjPFile    cdata;
    AjPStr     str    = NULL;
    AjPStr     aadata = NULL;

    AjBool     plot;
    AjPGraph   graph = NULL;

    float      ymax;
    float      ymin;

    ajint beg;
    ajint end;
    ajint window;
    ajint len;
    ajint limit;

    ajint i;
    ajint j;
    ajint idx;

    float *x = NULL;
    float *y = NULL;

    float sum = 0.;


    char *p;
    char *sname;


    ajGraphInit("charge", argc, argv);

    seqall    = ajAcdGetSeqall("seqall");
    plot      = ajAcdGetBool("plot");
    window    = ajAcdGetInt("window");
    aadata    = ajAcdGetString("aadata");

    /* only one will be used - see variable 'plot' */

    outf  = ajAcdGetOutfile("outfile");
    graph = ajAcdGetGraphxy("graph");

    ajFileDataNew(aadata,&cdata);
    if(!cdata)
	ajFatal("Cannot open amino acid data file %S",aadata);


    chg = charge_read_amino(&cdata);

    str = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	beg = ajSeqallBegin(seqall);
	end = ajSeqallEnd(seqall);
	len = end-beg+1;

	limit = len-window+1;
	sname = ajSeqName(seq);

	ymin = (float)0.;
	ymax = (float)0.;

	ajStrAssSubC(&str,ajSeqChar(seq),--beg,--end);
	ajStrToUpper(&str);
	p = ajStrStr(str);

	if(limit>0)
	{
	    AJCNEW0(x,limit);
	    AJCNEW0(y,limit);
	}


	for(i=0;i<limit;++i)
	{
	    x[i] = (float)i+1+beg;
	    sum=0.;
	    for(j=0;j<window;++j)
	    {
		idx = ajAZToInt(toupper((int)*(p+i+j)));
		sum += ajFloatGet(chg,idx);
	    }
	    sum /= (float)window;
	    y[i] = sum;
	}


	for(i=0;i<limit;++i)
	{
	    ymax = ymax > y[i] ? ymax : y[i];
	    ymin = ymin < y[i] ? ymin : y[i];

	}

	if(!plot)
	{
	    ajFmtPrintF(outf,"CHARGE of %s from %d to %d: window %d\n\n",sname,
			beg+1,end+1,window);
	    ajFmtPrintF(outf,"Position\tCharge\n");
	    for(i=0;i<limit;++i)
		ajFmtPrintF(outf,"%d\t\t%.3lf\n",(int)x[i],y[i]);
	}
	else
	{
	    ajGraphSetMulti(graph,1);
	    ajGraphxySetOverLap(graph,ajFalse);
	    ajGraphxyXtitleC(graph,"Position");
	    ajGraphxyYtitleC(graph,"Charge");
	    charge_addgraph(graph,limit,x,y,ymax,ymin,window,sname);
	    if(limit>0)
		ajGraphxyDisplay(graph,ajFalse);
	}

	if(limit>0)
	{
	    AJFREE(x);
	    AJFREE(y);
	}
    }

    if(plot)
        ajGraphClose();
    else
	ajFileClose(&outf);

    ajStrDel(&str);

    ajFileClose(&cdata);

    ajExit();

    return 0;
}



/* @funcstatic charge_addgraph ************************************************
**
** Undocumented.
**
** @param [?] graph [AjPGraph] graph object
** @param [?] limit [ajint] range
** @param [?] x [float*] x co-ords
** @param [?] y [float*] y co-ords
** @param [?] ymax [float] max y value
** @param [?] ymin [float] max x value
** @param [?] window [ajint] window
** @param [?] sname [char*] sequence name
** @@
******************************************************************************/

static void charge_addgraph(AjPGraph graph, ajint limit, float *x,
			    float *y, float ymax, float ymin,
			    ajint window, char *sname)
{
    ajint i;

    AjPGraphData data;
    AjPStr st = NULL;
    float baseline = 0.;

    if(limit<1)
	return;

    data = ajGraphxyDataNewI(limit);

    st = ajStrNew();

    for(i=0;i<limit;++i)
    {
	data->x[i] = x[i];
	data->y[i] = y[i];
    }

    ajGraphxySetColour(data,BLACK);
    ajGraphDataxySetMaxMin(data,x[0],x[limit-1],ymin,ymax);
    ajGraphDataxySetMaxima(data,x[0],x[limit-1],ymin,ymax);

    ajFmtPrintS(&st,"CHARGE of %s. Window:%d",sname,window);
    ajGraphxyDataSetTitle(data,st);
    ajGraphxyTitleC(graph,ajStrStr(st));

    ajGraphDataxySetTypeC(data,"2D Plot Float");
    ajFmtPrintS(&st,"Charge");
    ajGraphxyDataSetYtitle(data,st);

    ajFmtPrintS(&st,"Position");
    ajGraphxyDataSetXtitle(data,st);

    ajGraphDataObjAddLine(data,x[0],baseline,x[limit-1],baseline,BLUE);

    ajGraphxyAddGraph(graph,data);

    ajStrDel(&st);

    return;
}




/* @funcstatic charge_read_amino **********************************************
**
** Undocumented.
**
** @param [?] fp [AjPFile*] Undocumented
** @return [AjPFloat] Undocumented
** @@
******************************************************************************/

static AjPFloat charge_read_amino(AjPFile* fp)
{
    AjPStr   line;
    AjPFloat chg = NULL;
    char     c;
    float    v = 0.;
    ajint    idx;

    line = ajStrNew();
    chg  = ajFloatNew();


    while(ajFileReadLine(*fp,&line))
    {
	if(*ajStrStr(line)=='#' || !ajStrLen(line))
	    continue;

	ajFmtScanS(line,"%c%*f%*d%*d%*d%*d%*d%*d%f",&c,&v);
	idx = toupper(ajAZToInt((int)c));
	ajFloatPut(&chg,idx,v);
    }

    ajFileClose(fp);
    ajStrDel(&line);

    return chg;
}

/* @source syco application
**
** Gribskov statistical plot of synonymous codon usage
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
#include <limits.h>
#include <float.h>
#include <math.h>




#ifdef PLD_png

extern int PNGWidth;
extern int PNGHeight;

#endif




/* @prog syco *****************************************************************
**
** Synonymous codon usage Gribskov statistic plot
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq a;
    AjPFile outf;
    AjBool plot;
    AjBool show;
    AjPCod codon;
    AjPCod cdup;
    AjPStr substr;
    AjPStr tmp;
    float min;

    AjPGraph graph = NULL;
    AjPGraphData this = NULL;

    char *frames[]=
    {
	"Frame 1","Frame 2","Frame 3"
    };


    float *xarr[3];
    float *farr[3];
    ajint *unc[3];

    float sum;

    char *p;
    char *q;

    ajint base;
    ajint limit;
    ajint startp;
    ajint len;
    ajint i;
    ajint j;
    ajint count;
    ajint pos;
    ajint idx;
    ajint w;

    float miny;
    float maxy;

    float amin;
    float amax;

    ajint beg;
    ajint end;
    ajint window;

#ifdef PLD_png

    /*
    PNGWidth = 1280;
    PNGHeight = 960;
    */

    PNGWidth = 960;
    PNGHeight = 960;

#endif

    ajGraphInit("syco", argc, argv);
    
    a      = ajAcdGetSeq("sequence");
    codon  = ajAcdGetCodon("cfile");
    window = ajAcdGetInt("window");
    plot   = ajAcdGetBool("plot");
    outf   = ajAcdGetOutfile("outfile");
    show   = ajAcdGetBool("uncommon");
    min    = ajAcdGetFloat("minimum");
    
    if(plot)
    {
        graph = ajAcdGetGraphxy("graph");
	ajGraphSetCharSize(0.60);
    }
    
    
    cdup = ajCodDup(codon);
    
    substr = ajStrNew();
    tmp    = ajStrNew();
    
    beg = ajSeqBegin(a);
    end = ajSeqEnd(a);
    ajStrAssSubC(&substr,ajSeqChar(a),beg-1,end-1);
    
    
    
    p   = ajStrStr(substr);
    len = ajStrLen(substr);
    ajStrToUpper(&substr);
    
    w = window*3;
    
    limit = len-w-3;
    count = (limit/3)+1;
    if(count>0)
    {
	for(i=0;i<3;++i)
	{
	    AJCNEW(farr[i],count);
	    AJCNEW(unc[i],count);
	    AJCNEW(xarr[i],count);
	}
    }
    else
    {
	ajFmtPrintF(outf,"Insufficient data points\n");
	ajExit();
	return 0;
    }
    
    
    
    if(!plot)
	ajFmtPrintF(outf,"SYCO of %s from %d to %d\n",ajSeqName(a),beg,
		    end);
    
    
    miny = FLT_MAX;
    maxy = FLT_MIN;
    
    for(base=0;base<3;++base)
    {
	q = p+base;
	ajStrAssC(&tmp,q);

	ajCodCalcGribskov(&cdup, tmp);
	startp = (w/2)+base;
	for(i=0,pos=base;i<count;++i,pos+=3)
	{
	    sum = 0.0;
	    for(j=0;j<w;j+=3)
	    {
		idx = ajCodIndexC(&p[pos+j]);
		sum += cdup->tcount[idx];
	    }
	    xarr[base][i] = (float)(beg+startp);
	    farr[base][i] = (float)exp((double)((double)sum/(double)w));
	    startp += 3;
	}

	for(j=0;j<count;++j)
	{
	    miny = (miny<farr[base][j]) ? miny : farr[base][j];
	    maxy = (maxy>farr[base][j]) ? maxy : farr[base][j];
	}

	if(plot)
	{
	    this = ajGraphxyDataNewI(count);
	    ajGraphDataxySetTypeC(this,"Multi 2D plot");

	    ajGraphxySetOverLap(graph,ajFalse);
	    for(i=0;i<count;++i)
	    {
		this->x[i]=xarr[base][i];
		this->y[i]=farr[base][i];
	    }
	    ajGraphDataxyMaxMin(this->y,count,&amin,&amax);
	    ajGraphDataxySetMaxima(this,(float)beg,(float)end,amin,amax);

	    ajGraphxyAddGraph(graph,this);
	    if(show)
	    {
		for(i=0;i<count;++i)
		{
		    idx = ajCodIndexC(&q[i*3]);
		    if(codon->fraction[idx]<=min)
			unc[base][i] = 1;
		    else
			unc[base][i] = 0;
		}

		for(i=0;i<count;++i)
		    if(unc[base][i])
			ajGraphDataObjAddLine(this,xarr[base][i],1.,
					      xarr[base][i],1.005,3);
	    }


	    ajGraphxyDataSetYtitleC(this,"Gribskov value");
	    ajGraphxyDataSetXtitleC(this,"Sequence position");
	    ajGraphxyDataSetTitleC(this,frames[base]);
	    ajGraphDataObjAddLine(this,(float)beg,1.0,(float)end,
				  1.0,4);
	}
	else
	{
	    ajFmtPrintF(outf,"\n\n%s\n",frames[base]);
	    ajFmtPrintF(outf,"Mid base\tGribskov value\n");
	    for(i=0;i<count;++i)
		ajFmtPrintF(outf,"%-8d\t%.3f\n",(ajint)xarr[base][i],
			    farr[base][i]);
	}
    }
    
    if(plot)
    {
	ajGraphxySetTitleDo(graph, ajTrue);
	ajGraphxySetMaxMin(graph,(float)beg,(float)end,miny,maxy);
	ajGraphxySetYStart(graph,0.0);
	ajGraphxySetYEnd(graph,2.0);
	ajGraphxyTitleC(graph,"Gribskov Codon Plot");
	ajGraphxyDisplay(graph,ajTrue);
    }
    
    
    ajStrDel(&substr);
    ajFileClose(&outf);
    ajCodDel(&codon);
    ajCodDel(&cdup);
    
    ajExit();

    return 0;
}

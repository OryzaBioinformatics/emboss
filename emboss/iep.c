/* @source iep application
**
** Calculate isoelectric point of a protein
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
#include <stdlib.h>
#include <limits.h>

#define GSTEP 0.1



/* @prog iep ***************************************************************
**
** Calculates the isoelectric point of a protein
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall all;
    AjPSeq   a;
    AjPStr   substr;
    AjPFile  outf;
    AjBool   termini;
    AjBool   doplot;
    AjBool   dofile;
    AjPGraph graph=NULL;

    float step;
    ajint   amino=1;

    double H;
    double pH;
    double iep;
    
    ajint    *c=NULL;
    ajint    *op=NULL;
    double *K=NULL;
    double *pro=NULL;
    double sum;
    double charge;

    AjPGraphData phGraph=NULL;
    AjPStr tit=NULL;
    AjPStr tmp=NULL;
    
    float *xa=NULL;
    float *ya=NULL;
    float minchg=0.0;
    float maxchg=0.0;
    ajint   npoints;
    ajint   k;
    ajint   be;
    ajint   en;
    ajint   i;
    
    (void) ajGraphInit("iep", argc, argv);

    all       = ajAcdGetSeqall("sequencea");
    doplot    = ajAcdGetBool("plot");
    dofile    = ajAcdGetBool("report");
    step      = ajAcdGetFloat("step");
    termini   = ajAcdGetBool("termini");
    amino     = ajAcdGetInt("amino");
    outf      = ajAcdGetOutfile("outfile");


    substr=ajStrNew();
    AJCNEW (K,   EMBIEPSIZE);
    AJCNEW (c,   EMBIEPSIZE);
    AJCNEW (op,  EMBIEPSIZE);
    AJCNEW (pro, EMBIEPSIZE);

    embIepPkRead();				/* read pK's */
    embIepCalcK(K);				/* Convert to dissoc consts */

    if(doplot)
      graph = ajAcdGetGraphxy("graph");

    while(ajSeqallNext(all,&a))
    {
	be=ajSeqallBegin(all);
	en=ajSeqallEnd(all);
	ajStrAssSubC(&substr,ajSeqChar(a),be-1,en-1);
	
	for(i=0;i<EMBIEPSIZE;++i)
	{
	    c[i]=op[i]=0;
	    pro[i]=0.;
	}
	
	embIepComp(ajStrStr(substr),amino,c); /* Get sequence composition */


	if (dofile)
	{
	    ajFmtPrintF(outf,"IEP of %s from %d to %d\n",ajSeqName(a),be,en);
	    if(!embIepIEP(ajStrStr(substr),amino,&iep,termini))
		ajFmtPrintF(outf,"Isoelectric Point = None\n\n");
	    else
		ajFmtPrintF(outf,"Isoelectric Point = %-6.4lf\n\n",iep);

	    ajFmtPrintF(outf,"   pH     Bound    Charge\n");
    
	    for(pH=1.0;pH<=14.0;pH+=step)
	    {
		H=embIepPhToHConc(pH);
		if(!termini)
		    c[EMBIEPAMINO]=c[EMBIEPCARBOXYL]=0;
		embIepGetProto(K,c,op,H,pro);
		charge=embIepGetCharge(c,pro,&sum);
		ajFmtPrintF(outf,"%6.2lf   %6.2lf   %6.2lf\n",pH,sum,charge);
	    }
	}
	if(doplot)
	{
	    npoints = (ajint) ((14.0-1.0)/(double)GSTEP) +1;
	    AJCNEW (xa, npoints);
	    AJCNEW (ya, npoints);
	
	    minchg = (float) INT_MAX;
	    maxchg = (float)-INT_MAX;

	    for(pH=1.0,k=0;pH<=14.0;pH+=GSTEP,++k)
	    {
		xa[k]=(float)pH;
		H=embIepPhToHConc(pH);
		if(!termini)
		    c[EMBIEPAMINO]=c[EMBIEPCARBOXYL]=0;
		embIepGetProto(K,c,op,H,pro);
		charge=embIepGetCharge(c,pro,&sum);
		ya[k]=(float)charge;
		minchg = (minchg<charge) ? minchg : (float)charge;
		maxchg = (maxchg>charge) ? maxchg : (float)charge;
	    }
	    npoints=k;

	    tit = ajStrNew();
	    tmp = ajStrNew();
	    ajFmtPrintS(&tit,"%s %d-%d IEP=",ajSeqName(a),be,en);

	    if(!embIepIEP(ajStrStr(substr),amino,&iep,termini))
		ajStrAssC(&tmp,"none");
	    else
		ajFmtPrintS(&tmp,"%-8.4f",iep);
	    ajStrApp(&tit,tmp);


	    phGraph=ajGraphxyDataNewI(npoints);
	    ajGraphxyTitleC(graph,ajStrStr(tit));
	    ajGraphxyXtitleC(graph,"pH");
	    ajGraphxyYtitleC(graph,"Charge");

	    ajGraphDataxySetTypeC(phGraph,"2D Plot Float");
	    ajGraphDataxySetMaxMin(phGraph,1.0,14.0,minchg,maxchg);
	    ajGraphDataxySetMaxima(phGraph,1.0,14.0,minchg,maxchg);
	    ajGraphxySetXStart(graph,1.0);
	    ajGraphxySetXEnd(graph,14.0);
	    ajGraphxySetYStart(graph,minchg);
	    ajGraphxySetYEnd(graph,maxchg);
	    ajGraphxySetXRangeII(graph,1,14);
	    ajGraphxySetYRangeII(graph,(ajint)minchg,(ajint)maxchg);
    
	    ajGraphxyAddDataPtrPtr(phGraph,xa,ya);
	    ajGraphxyReplaceGraph(graph,phGraph);
	

	    ajGraphxyDisplay(graph,ajFalse);
	    ajStrDel(&tmp);
	    ajStrDel(&tit);
	    AJFREE (ya);
	    AJFREE (xa);
	}
    }
    ajGraphClose();
    AJFREE (K);    
    AJFREE (pro);
    AJFREE (op);
    AJFREE (c);

    ajStrDel(&substr);
    ajFileClose(&outf);

    ajExit();
    return 0;
}

/* @source dan application
**
** Displays and plots nucleic acid duplex melting temperatures
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** Contributions from Michael Schmitz
** @@
**
** Replaces program "MELTDNA/MELTPLOT" by Rodrigo Lopez (EGCG)
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
#include <stdlib.h>




static void dan_reportgc(AjPReport report, AjPSeq seq,
			 AjPFeattable TabRpt, ajint window,
			 ajint shift, float formamide, float mismatch,
			 ajint prodLen, float dna, float salt,
			 float temperature, AjBool isDNA, AjBool isProduct,
			 AjBool dothermo, AjBool doplot,
			 float *xa, float *ta, float *tpa, float *cga,
			 ajint *npoints);

static void dan_findgc(AjPStr *strand, ajint begin, ajint end, ajint window,
		       ajint shift, float formamide, float mismatch,
		       ajint prodLen, float dna, float salt,
		       float temperature, AjBool isDNA, AjBool isProduct,
		       AjBool dothermo, AjPFile outf, AjBool doplot,
		       float *xa, float *ta, float *tpa, float *cga,
		       ajint *npoints);

static void dan_plotit(AjPSeq *seq, float *xa, float *ta, float *cga,
		       float *tpa, ajint npoints, ajint ibegin, ajint iend,
		       AjPGraph mult, float mintemp);

static void dan_unfmall(float *xa, float *ta, float *tpa, float *cga);




/* @prog dan ******************************************************************
**
** Calculates DNA RNA/DNA melting temperature
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPFile outf = NULL;
    AjPReport report    = NULL;
    AjPFeattable TabRpt = NULL;
    ajint begin;
    ajint end;
    ajint window;
    ajint shift;
    float DNAConc;
    float saltConc;
    float temperature = 0.0;
    AjBool doThermo;
    AjBool isProduct;
    AjBool isDNA  = ajTrue;
    AjBool isRNA  = ajFalse;
    AjBool doplot = ajFalse;
    AjPGraph mult = NULL;
    float mintemp = 0.0;
    float formamide;
    float mismatch;
    float prodLen;

    AjPSeq seq;
    AjPStr strand = NULL;
    ajint len;

    static float *xa;
    static float *ta;
    static float *tpa;
    static float *cga;
    static ajint npoints;
    ajint n;


    ajGraphInit("dan", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    report    = ajAcdGetReport("outfile");
    window    = ajAcdGetInt("windowsize");
    shift     = ajAcdGetInt("shiftincrement");
    DNAConc   = ajAcdGetFloat("dnaconc");
    saltConc  = ajAcdGetFloat("saltconc");
    doThermo  = ajAcdGetBool("thermo");
    isProduct = ajAcdGetBool("product");
    isRNA     = ajAcdGetBool("rna");
    doplot    = ajAcdGetBool("plot");

    formamide = ajAcdGetFloat("formamide");
    mismatch  = ajAcdGetFloat("mismatch");
    prodLen   = ajAcdGetInt("prodLen");

    if(!isProduct)
    {
	formamide = mismatch = 0.0;
	prodLen   = window;
    }

    temperature = ajAcdGetFloat("temperature");
    if (!doThermo)
	temperature = 0.0;

    mintemp = ajAcdGetFloat("mintemp");
    mult = ajAcdGetGraphxy ("graph");
    if(!doplot)
    {
	mintemp = 0.0;
	mult = NULL;
    }



    if(isRNA)
	isDNA = ajFalse;
    else
	isDNA = ajTrue;


    while(ajSeqallNext(seqall, &seq))
    {
	npoints = 0;
	strand  = ajSeqStrCopy(seq);
	len     = ajStrLen(strand);
	begin   = ajSeqallBegin(seqall);
	end     = ajSeqallEnd(seqall);

	TabRpt = ajFeattableNewSeq(seq);

	if(!doplot)
	    if(outf)
		ajFmtPrintF(outf,"DAN of: %s   from: %d  to: %d\n\n",
			    ajSeqName(seq), begin, end);

	ajStrToUpper(&strand);


	n=ajRound(len,shift);

	AJCNEW(xa, n);
	AJCNEW(ta, n);
	AJCNEW(tpa, n);
	AJCNEW(cga, n);

	if(outf)
	    dan_findgc(&strand, begin, end,
		       window,shift,formamide,mismatch,
		       prodLen,DNAConc,saltConc, temperature, isDNA, isProduct,
		       doThermo, outf, doplot, xa, ta, tpa, cga, &npoints);

	dan_reportgc(report, seq, TabRpt,
		     window,shift,formamide,mismatch,
		     prodLen,DNAConc,saltConc, temperature, isDNA, isProduct,
		     doThermo, doplot, xa, ta, tpa, cga, &npoints);

	if(doplot)
	    dan_plotit(&seq,xa,ta,cga,tpa,npoints,begin,end, mult,
		       mintemp);

	dan_unfmall(xa, ta, tpa, cga);
	ajStrDel(&strand);

	if(!doplot)
	    ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);
    }

    if(mult)
    {
	ajGraphCloseWin();
	ajGraphxyDel(&mult);
    }

    ajSeqDel(&seq);
    if(outf)
	ajFileClose(&outf);

    ajReportClose(report);

    ajExit();
    return 0;
}




/* @funcstatic dan_findgc *****************************************************
**
** Undocumented
**
** @param [r] strand [AjPStr*] Undocumented
** @param [r] begin [ajint] Undocumented
** @param [r] end [ajint] Undocumented
** @param [r] window [ajint] Undocumented
** @param [r] shift [ajint] Undocumented
** @param [r] formamide [float] Undocumented
** @param [r] mismatch [float] Undocumented
** @param [r] prodLen [ajint] Undocumented
** @param [r] dna [float] Undocumented
** @param [r] salt [float] Undocumented
** @param [r] temperature [float] Undocumented
** @param [r] isDNA [AjBool] Undocumented
** @param [r] isproduct [AjBool] Undocumented
** @param [r] dothermo [AjBool] Undocumented
** @param [r] outf [AjPFile] Undocumented
** @param [r] doplot [AjBool] Undocumented
** @param [r] xa [float[]] Undocumented
** @param [r] ta [float[]] Undocumented
** @param [r] tpa [float[]] Undocumented
** @param [r] cga [float[]] Undocumented
** @param [r] np [ajint*] Undocumented
** @@
******************************************************************************/

static void dan_findgc(AjPStr *strand, ajint begin, ajint end, ajint window,
		       ajint shift, float formamide, float mismatch,
		       ajint prodLen, float dna, float salt,float temperature,
		       AjBool isDNA, AjBool isproduct,  AjBool dothermo,
		       AjPFile outf, AjBool doplot, float xa[],
		       float ta[], float tpa[], float cga[], ajint *np)
{
    static AjBool initialised = 0;
    AjPStr type   = NULL;
    AjPStr substr = NULL;
    float  fwindow;
    ajint    i;
    ajint ibegin;
    ajint iend;
    float fprodlen;
    float TmP1;
    float TmP2;
    ajint   e;

    float DeltaG = 0.0;
    float DeltaH;
    float DeltaS;

    --begin;
    --end;


    if(!initialised)
    {
	type = ajStrNew();
	if(isDNA)
	    ajStrAssC(&type,"dna");
	else
	    ajStrAssC(&type,"rna");
	ajMeltInit(&type, window);
	ajStrDel(&type);
    }

    fwindow  = (float) window;
    fprodlen = (float) prodLen;
    substr   = ajStrNew();

    e = (end - window) + 2;
    for(i=begin; i < e; i+=shift)
    {
	ibegin = i;
	iend   = i + window -1;
	ajStrAssSubC(&substr, ajStrStr(*strand), ibegin, iend);

	xa[*np]  = (float)(i+1);
	ta[*np]  = ajTm(&substr, (iend-ibegin)+1, shift, salt, dna, isDNA);
	cga[*np] = 100.0 * ajMeltGC(&substr, window);

	if(dothermo)
	{
	    DeltaG = -1. * ajMeltEnergy(&substr, (iend-ibegin)+1, shift, isDNA,
					ajFalse, &DeltaH, &DeltaS);

	    DeltaH = -1. * DeltaH;
	    DeltaS = -1. * DeltaS;
	    DeltaG = DeltaH - 0.001*DeltaS*(273.15+temperature);
	}

	if(!doplot)
	{
	    ajFmtPrintF(outf,"%4d %s",ibegin+1,ajStrStr(substr));
	    if(iend-ibegin+1 > 40)
		ajFmtPrintF(outf,"...");
	    if(dothermo)
	        ajFmtPrintF(outf," %4d Tm=%2.1f GC%%=%2.1f dG %f dH %f dS %f",
			    iend+1,ta[*np], cga[*np], DeltaG, DeltaH, DeltaS);
	    else
	        ajFmtPrintF(outf," %4d Tm=%2.1f GC%%=%2.1f",iend+1,ta[*np],
			    cga[*np]);
	}


	if(isproduct)
	{
	    TmP1 = 81.5 + 16.6 * (float)log10((double)(salt/1000.0)) + 0.41 *
		cga[*np];
	    TmP2 = -(0.65 * formamide) - (675.0/fprodlen) - mismatch;
	    tpa[*np] = TmP1 + TmP2;
	    if(!doplot)
		ajFmtPrintF(outf," Tm(prod)=%1.1f",tpa[*np]);
	}
	if(!doplot)
	    ajFmtPrintF(outf,"\n");

	if(doplot)
	    xa[*np] += fwindow / 2.0;

	++(*np);
    }

    ajStrDel(&substr);

    return;
}




/* @funcstatic dan_reportgc ***************************************************
**
** Undocumented
**
** @param [r] report [AjPReport] Undocumented
** @param [r] seq [AjPSeq] Undocumented
** @param [r] TabRpt [AjPFeattable] Undocumented
** @param [r] window [ajint] Undocumented
** @param [r] shift [ajint] Undocumented
** @param [r] formamide [float] Undocumented
** @param [r] mismatch [float] Undocumented
** @param [r] prodLen [ajint] Undocumented
** @param [r] dna [float] Undocumented
** @param [r] salt [float] Undocumented
** @param [r] temperature [float] Undocumented
** @param [r] isDNA [AjBool] Undocumented
** @param [r] isproduct [AjBool] Undocumented
** @param [r] dothermo [AjBool] Undocumented
** @param [r] doplot [AjBool] Undocumented
** @param [r] xa [float[]] Undocumented
** @param [r] ta [float[]] Undocumented
** @param [r] tpa [float[]] Undocumented
** @param [r] cga [float[]] Undocumented
** @param [r] np [ajint*] Undocumented
** @@
******************************************************************************/


static void dan_reportgc(AjPReport report, AjPSeq seq, AjPFeattable TabRpt,
			 ajint window,
			 ajint shift, float formamide, float mismatch,
			 ajint prodLen, float dna, float salt,
			 float temperature,
			 AjBool isDNA, AjBool isproduct,  AjBool dothermo,
			 AjBool doplot, float xa[],
			 float ta[], float tpa[], float cga[], ajint *np)
{

    AjPFeature gf = NULL;
    static AjPStr tmpStr = NULL;
    static AjBool initialised = 0;
    AjPStr type   = NULL;
    AjPStr substr = NULL;
    float  fwindow;
    ajint    i;
    ajint ibegin;
    ajint iend;
    float fprodlen;
    float TmP1;
    float TmP2;
    ajint   e;

    float DeltaG = 0.0;
    float DeltaH;
    float DeltaS;

    ajint begin;
    ajint end;

    begin = ajSeqBegin(seq);
    end   = ajSeqEnd(seq);

    --begin;
    --end;

    if(!initialised)
    {
	type = ajStrNew();
	if(isDNA)
	    ajStrAssC(&type,"dna");
	else
	    ajStrAssC(&type,"rna");
	ajMeltInit(&type, window);
	ajStrDel(&type);
    }

    fwindow  = (float) window;
    fprodlen = (float) prodLen;
    substr   = ajStrNew();

    e = (end - window) + 2;
    for(i=begin; i < e; i+=shift)
    {
	ibegin = i;
	iend = i + window -1;
	ajStrAssSub(&substr, ajSeqStr(seq), ibegin, iend);

	xa[*np]  = (float)(i+1);
	ta[*np]  = ajTm(&substr, (iend-ibegin)+1, shift, salt, dna, isDNA);
	cga[*np] = 100.0 * ajMeltGC(&substr, window);

	if(dothermo)
	{
	    DeltaG = -1. * ajMeltEnergy(&substr, (iend-ibegin)+1, shift, isDNA,
					ajFalse, &DeltaH, &DeltaS);

	    DeltaH = -1. * DeltaH;
	    DeltaS = -1. * DeltaS;
	    DeltaG = DeltaH - 0.001*DeltaS*(273.15+temperature);
	}

	if(!doplot)
	{
	  gf = ajFeatNewII(TabRpt,
			   ibegin+1, iend+1);
	  ajFmtPrintS(&tmpStr, "*tm %4.1f", ta[*np]);
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  ajFmtPrintS(&tmpStr, "*gc %4.1f", cga[*np]);
	  ajFeatTagAdd(gf,  NULL, tmpStr);
	  if(dothermo)
	  {
	      ajFmtPrintS (&tmpStr, "*dg %.3f", DeltaG);
	      ajFeatTagAdd(gf,  NULL, tmpStr);
	      ajFmtPrintS (&tmpStr, "*dh %.3f", DeltaH);
	      ajFeatTagAdd(gf,  NULL, tmpStr);
	      ajFmtPrintS (&tmpStr, "*ds %.3f", DeltaS);
	      ajFeatTagAdd(gf,  NULL, tmpStr);
	  }
	}
	if(isproduct)
	{
	    TmP1 = 81.5 + 16.6 * (float)log10((double)(salt/1000.0)) + 0.41 *
		cga[*np];
	    TmP2 = -(0.65 * formamide) - (675.0/fprodlen) - mismatch;
	    tpa[*np] = TmP1 + TmP2;
	    if(!doplot)
	    {
		ajFmtPrintS (&tmpStr, "*tmprod %4.1f", tpa[*np]);
		ajFeatTagAdd(gf,  NULL, tmpStr);
	    }
	}

	if(doplot)
	    xa[*np] += fwindow / 2.0;

	++(*np);
    }

    ajStrDel(&substr);

    return;
}




/* @funcstatic dan_unfmall ****************************************************
**
** Undocumented.
**
** @param [?] xa [float*] Undocumented
** @param [?] ta [float*] Undocumented
** @param [?] tpa [float*] Undocumented
** @param [?] cga [float*] Undocumented
** @@
******************************************************************************/

static void dan_unfmall(float *xa, float *ta, float *tpa, float *cga)
{
    AJFREE( xa);
    AJFREE( ta);
    AJFREE( tpa);
    AJFREE( cga);

    return;
}




/* @funcstatic dan_plotit *****************************************************
**
** Undocumented.
**
** @param [?] seq [AjPSeq*] Undocumented
** @param [?] xa [float*] Undocumented
** @param [?] ta [float*] Undocumented
** @param [?] cga [float*] Undocumented
** @param [?] tpa [float*] Undocumented
** @param [?] npoints [ajint] Undocumented
** @param [?] ibegin [ajint] Undocumented
** @param [?] iend [ajint] Undocumented
** @param [?] graphs [AjPGraph] Undocumented
** @param [?] mintemp [float] Undocumented
** @@
******************************************************************************/

static void dan_plotit(AjPSeq *seq, float *xa, float *ta, float *cga,
		       float *tpa, ajint npoints, ajint ibegin, ajint iend,
		       AjPGraph graphs, float mintemp)
{
    AjPGraphData tmGraph = NULL;
    float max = -64000.;
    float min = 64000.;

    ajint i;

    for(i=0;i<npoints;++i)
    {
	min = (min<ta[i]) ? min : ta[i];
	max = (max>ta[i]) ? max : ta[i];
    }

    tmGraph = ajGraphxyDataNewI(npoints);
    ajGraphxySetTitleDo(graphs,ajTrue);
    ajGraphxySetXLabel(graphs,ajTrue);
    ajGraphxySetYLabel(graphs,ajTrue);

    ajGraphxyTitleC(graphs,ajSeqName(*seq));
    ajGraphxyXtitleC(graphs,"Base number");
    ajGraphxyYtitleC(graphs,"Melt temp (C)");

    ajGraphxySetXStart(graphs,ibegin);
    ajGraphxySetXEnd(graphs,iend);
    ajGraphxySetYStart(graphs,0.0);
    ajGraphxySetYEnd(graphs,100.0);
    ajGraphxySetXRangeII(graphs,ibegin,iend);
    ajGraphxySetYRangeII(graphs,(ajint)mintemp,100);

    ajGraphDataxySetTypeC(tmGraph,"2D Plot");
    ajGraphDataxySetMaxMin(tmGraph,(float)ibegin,(float)iend,min,max);
    ajGraphDataxySetMaxima(tmGraph,(float)ibegin,(float)iend,min,max);


    ajGraphxyAddDataPtrPtr(tmGraph,xa,ta);
    ajGraphxyAddGraph(graphs,tmGraph);

    ajGraphxyDisplay(graphs,AJTRUE);

    return;
}

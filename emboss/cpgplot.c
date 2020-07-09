/* @source cpgplot application
**
** Plots CpG island areas
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
#include <stdlib.h>


void findbases(AjPStr *substr, ajint begin, ajint len, ajint window, ajint shift,
               float *obsexp, float *xypc, AjPStr *bases, float *obsexpmax,
	       ajint *plstart, ajint *plend);
void countbases(char *seq, char *bases, ajint window, ajint *cx, ajint *cy,
		ajint *cxpy);
void identify(AjPFile outf, float *obsexp, float *xypc, AjBool *thresh,
	      ajint begin, ajint len, ajint shift, char *bases, char *name,
	      ajint minlen, float minobsexp, float minpc,AjPFeatTabOut featout);
void reportislands(AjPFile outf, AjBool *thresh, char *bases, char *name,
		   float minobsexp, float minpc, ajint minlen, ajint begin,
		   ajint len);
void plotit(char *seq, ajint begin, ajint len, ajint shift, float *obsexp,
	    float *xypc, AjBool *thresh, char *bases, float obsexpmax,
	    ajint plstart, ajint plend, AjBool doobsexp,
	    AjBool docg, AjBool dopc, AjPGraph mult);

void dumpfeatout(AjPFeatTabOut featout, AjBool *thresh, char *seqname,int begin,int len);


int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    bases=NULL;
    AjPGraph mult;
    AjBool    doobsexp;
    AjBool    dopc;
    AjBool    docg;
    
    ajint begin;
    ajint end;
    ajint len;

    ajint minlen;
    float minobsexp;
    float minpc;

    ajint window;
    ajint shift;
    ajint plstart;
    ajint plend;
    
    float  *xypc=NULL;
    float  *obsexp=NULL;
    AjBool *thresh=NULL;
    float  obsexpmax;
    
    ajint i;
    ajint maxarr;
    AjPFeatTabOut featout=NULL;

    
    (void) ajGraphInit("cpgplot",argc,argv);
    
    seqall    = ajAcdGetSeqall("sequence");
    window    = ajAcdGetInt("window");
    shift     = ajAcdGetInt("shift");
    outf      = ajAcdGetOutfile("outfile");
    minobsexp = ajAcdGetFloat("minoe");
    minlen    = ajAcdGetInt("minlen");
    minpc     = ajAcdGetFloat("minpc");
    mult      = ajAcdGetGraphxy ("graph");
    doobsexp  = ajAcdGetBool("obsexp");
    docg      = ajAcdGetBool("cg");
    dopc      = ajAcdGetBool("pc");
    featout   = ajAcdGetFeatout("featout");
    
    

    substr = ajStrNew();
    bases  = ajStrNewC("CG");
    

    maxarr = 0;

    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	strand = ajSeqStrCopy(seq);
	(void) ajStrToUpper(&strand);

	(void) ajStrAssSubC(&substr,ajStrStr(strand),--begin,--end);
	len=ajStrLen(substr);

	if (len > maxarr) {
	  AJCRESIZE(obsexp, len);
	  AJCRESIZE(thresh, len);
	  AJCRESIZE(xypc, len);
	}
	for(i=0;i<len;++i)
	{
	    thresh[i]=ajFalse;
	    obsexp[i]=xypc[i]=0.0;
	}
	
	

	findbases(&substr, begin, len, window, shift, obsexp, xypc,
		  &bases, &obsexpmax, &plstart, &plend);

	
	identify(outf, obsexp, xypc, thresh, 0, len, shift,
		 ajStrStr(bases), ajSeqName(seq), minlen, minobsexp, minpc,
		 featout);
	

	plotit(ajSeqName(seq), begin, len, shift, obsexp, xypc, thresh,
	       ajStrStr(bases), obsexpmax, plstart, plend,
	       doobsexp, docg, dopc, mult);

	ajStrDel(&strand);
   }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);
    
    ajExit();
    return 0;
}



void findbases(AjPStr *substr, ajint begin, ajint len, ajint window,
	       ajint shift, float *obsexp, float *xypc, AjPStr *bases,
	       float *obsexpmax, ajint *plstart, ajint *plend)
{
    ajint cxpy;
    ajint cx;
    ajint cy;
    float cxf;
    float cyf;
    float windowf;
    

    float obs;
    float exp;
    ajint i;
    ajint j=0;
    ajint offset;

    char *p;
    char *q;

    windowf = (float)window;
    *obsexpmax = 0.0;
    offset=window/2;
    *plstart = offset;
    q = ajStrStr(*bases);	

    for(i=0; i<(len-window+1);i+=shift)
    {
	j = i+offset;
	p = ajStrStr(*substr) + i;
	countbases(p, q, window, &cx, &cy, &cxpy);
	
	obs = (float) cxpy;
	exp = ((float)(cx*cy))/windowf;
	cxf=(float)cx;
	cyf=(float)cy;
	if(!exp) obsexp[j]=0.0;
	else
	{
	    obsexp[j]=obs/exp;
	    *obsexpmax = (*obsexpmax > obsexp[j]) ? *obsexpmax : obsexp[j];
	}
	xypc[j]=(cxf/windowf)*100.0 + (cyf/windowf)*100.0;
    }
    
    *plend = j;
    return;
}





void countbases(char *seq, char *bases, ajint window, ajint *cx, ajint *cy,
		ajint *cxpy)
{
    ajint i;
    
    ajint codex;
    ajint codey;
    ajint codea;
    ajint codeb;

    *cxpy = *cx = *cy = 0;

    codex = ajAZToBin(bases[0]);
    codey = ajAZToBin(bases[1]);

    codeb = ajAZToBin(seq[0]);

    for(i=0; i<window; ++i)
    {
	codea=codeb;
	codeb=ajAZToBin(seq[i+1]);
	if(codea && !(codea & (15-codex)))
	{
	    ++*cx;
	    if(codeb && !(codeb & (15-codey))) ++*cxpy;
	}
	if(codea && !(codea & (15-codey))) ++*cy;
    }

    return;
}

/*
**    This subroutine identifies the CpG line, identifying the possible
**    dinucleotide 'islands' in the sequence. These are defined as
**    base positions where, over an average of 10 windows, the calculated
**    % composition is over 50% and the calculated Obs/Exp ratio is over 0.6
**    and the conditions hold for a minimum of 200 bases.
**
*/

void identify(AjPFile outf, float *obsexp, float *xypc, AjBool *thresh,
	      ajint begin, ajint len, ajint shift, char *bases, char *name,
	      ajint minlen, float minobsexp, float minpc,
	      AjPFeatTabOut featout)
{
  static ajint avwindow=10;
  float avpc;
  float avobsexp;
  float sumpc;
  float sumobsexp;
    
  ajint i;
  ajint pos;
  ajint sumlen;
  ajint first=0;
  
  for(i=0; i<len; ++i) thresh[i]=ajFalse;

  sumlen=0;
/*  posmin = begin;*/
  for(pos=0,first=0;pos<(len-avwindow*shift);pos+=shift)
  {
    sumpc=sumobsexp=0.0;
    ajDebug("pos: %d max: %d\n", pos, pos+avwindow*shift);
    for(i=pos;i<=(pos+avwindow*shift);++i)
    {
      ajDebug("obsexp[%d] %.2f xypc[%d] %.2f\n",
	      i, obsexp[i], i, xypc[i]);
      sumpc += xypc[i];
      sumobsexp += obsexp[i];
    }
	
    avpc = sumpc/(float)avwindow;
    avobsexp = sumobsexp/(float)avwindow;
    ajDebug("sumpc: %.2f sumobsexp: %.2f\n", sumpc, sumobsexp);
    ajDebug(" avpc: %.2f  avobsexp: %.2f\n", avpc, avobsexp);
    if((avobsexp>minobsexp)&&(avpc>minpc))
    {
      if(!sumlen) first=pos; /* start a new island */
      sumlen += shift;
      ajDebug(" ** hit first: %d sumlen: %d\n", first, sumlen);
    }
    else
    {
      if(sumlen >= minlen) {/* island ajlong enough? */
	for(i=first; i<=pos-shift;++i)
	  thresh[i]=ajTrue;
      }
      sumlen=0;
    }
  }

  if(sumlen>=minlen) {
    for(i=first;i<len;++i)
      thresh[i]=ajTrue;
  }

  reportislands(outf, thresh, bases, name, minobsexp, minpc,
		minlen, begin, len);

  dumpfeatout(featout,thresh, name, begin, len );
  return;
}




void reportislands(AjPFile outf, AjBool *thresh, char *bases, char *name,
		   float minobsexp, float minpc, ajint minlen, ajint begin,
		   ajint len)
{
    AjBool island;
    ajint startpos=0;
    ajint endpos;
    ajint slen;
    ajint i;


    (void) ajFmtPrintF(outf,"\n\nCPGPLOT islands of unusual %s composition\n",
		bases);
    (void) ajFmtPrintF(outf,"%s from %d to %d\n\n",name,begin+1,begin+len);
    (void) ajFmtPrintF(outf,"     Observed/Expected ratio > %.2f\n",minobsexp);
    (void) ajFmtPrintF(outf,"     Percent %c + Percent %c > %.2f\n",bases[0],
		bases[1],minpc);
    (void) ajFmtPrintF(outf,"     Length > %d\n",minlen);

    island = ajFalse;
    for(i=0;i<len;++i)
    {
	if(island)
	{
	    island = thresh[i];
	    if(!island)
	    {
		slen = i - startpos;
		endpos = i;
		(void) ajFmtPrintF(outf,"\n Length %d (%d..%d)\n",slen,
				   startpos+begin+1,endpos+begin);
	    }
	}
	else
	{
	    island=thresh[i];
	    if(island) startpos=i;
	}
    }

    if(island)
    {
	slen=len-startpos+1;
	endpos=len-1;
	(void) ajFmtPrintF(outf,"\n Length %d (%d..%d)\n",slen,
			   startpos+begin+1, endpos+begin);
    }

    return;
}






void plotit(char *seq, ajint begin, ajint len, ajint shift, float *obsexp,
	    float *xypc, AjBool *thresh, char *bases, float obsexpmax,
	    ajint plstart, ajint plend, AjBool doobsexp,
	    AjBool docg, AjBool dopc, AjPGraph graphs)

{
    AjPGraphData tmGraph  = NULL;
    AjPGraphData tmGraph2 = NULL; 
    AjPGraphData tmGraph3 = NULL;   
    float *tmp=NULL;
    ajint i;
    float min=0.;
    float max=0.;
    
    if(doobsexp)
    {

	tmGraph2=ajGraphxyDataNew();
	ajGraphxyDataSetTitleC(tmGraph2,"Observed vs Expected");
	ajGraphxyDataSetXtitleC(tmGraph2,"Base number");
	ajGraphxyDataSetYtitleC(tmGraph2,"Obs/Exp");

	min = 64000.;
	max = -64000.;
	for(i=0;i<len;++i)
	{
	    min = (min<obsexp[i]) ? min : obsexp[i];
	    max = (max>obsexp[i]) ? max : obsexp[i];
	}

	ajGraphDataxySetMaxMin(tmGraph2,(float)begin,(float)begin+len-1,
			       min,max);
	ajGraphDataxySetMaxima(tmGraph2,(float)begin,(float)begin+len-1,
			       0.0,max);
	ajGraphDataxySetTypeC(tmGraph2,"Multi 2D Plot");

	ajGraphxySetXStart(graphs,(float)begin);
	ajGraphxySetXEnd(graphs,(float)(begin+len-1));
    
	ajGraphxySetYStart(graphs,0.0);
	ajGraphxySetYEnd(graphs,obsexpmax);
	ajGraphxySetXRangeII(graphs,begin,begin+len-1);
	ajGraphxySetYRangeII(graphs,0,(ajint)(obsexpmax+1.0));

	ajGraphxyAddDataCalcPtr(tmGraph2,len,(float)begin,1.0,obsexp);    
	ajGraphxyAddGraph(graphs,tmGraph2);
    }
    
    if(dopc)
    {
	tmGraph3=ajGraphxyDataNew();
	ajGraphxyDataSetTitleC(tmGraph3,"Percentage");
	ajGraphxyDataSetXtitleC(tmGraph3,"Base number");
	ajGraphxyDataSetYtitleC(tmGraph3,"Percentage");

	min = 64000.;
	max = -64000.;
	for(i=0;i<len;++i)
	{
	    min = (min<xypc[i]) ? min : xypc[i];
	    max = (max>xypc[i]) ? max : xypc[i];
	}

	ajGraphDataxySetMaxMin(tmGraph3,(float)begin,(float)begin+len-1,
			       min,max);
	ajGraphDataxySetMaxima(tmGraph3,(float)begin,(float)begin+len-1,
			      0.0,max);
	ajGraphDataxySetTypeC(tmGraph3,"Multi 2D Plot");
    
	ajGraphxySetXStart(graphs,(float)begin);
	ajGraphxySetXEnd(graphs,(float)(begin+len-1));
	ajGraphxySetYStart(graphs,0);
	ajGraphxySetYEnd(graphs,100);
	ajGraphxySetXRangeII(graphs,begin,begin+len-1);
	ajGraphxySetYRangeII(graphs,0,100);
    
	ajGraphxyAddDataCalcPtr(tmGraph3,len,(float)begin,1.0,xypc);
	ajGraphxyAddGraph(graphs,tmGraph3);
    }

    if(docg)
    {
	AJCNEW (tmp, len);
	for(i=0;i<len;++i)
	{
	    if(thresh[i])
		tmp[i]=1.0;
	    else
		tmp[i]=0.0;
	}
	
	tmGraph=ajGraphxyDataNew();
	ajGraphxyDataSetTitleC(tmGraph,"Putative Islands");
	ajGraphxyDataSetXtitleC(tmGraph,"Base Number");
	ajGraphxyDataSetYtitleC(tmGraph,"Threshold");

	ajGraphDataxySetMaxMin(tmGraph,(float)begin,(float)begin+len-1,
			       0.,1.);
	ajGraphDataxySetTypeC(tmGraph,"Multi 2D Plot");

    
	ajGraphxySetXStart(graphs,(float)begin);
	ajGraphxySetXEnd(graphs,(float)(begin+len-1));
	ajGraphxySetYStart(graphs,0);
	ajGraphxySetYEnd(graphs,2);
	ajGraphxySetXRangeII(graphs,begin,begin+len-1);
	ajGraphxySetYRangeII(graphs,0,2);
	ajGraphDataxySetMaxMin(tmGraph,(float)begin,(float)(begin+len-1),
			       0.0,1.2);
	ajGraphDataxySetMaxima(tmGraph,(float)begin,(float)(begin+len-1),
			       0.0,1.0);
    
	ajGraphxyAddDataCalcPtr(tmGraph,len,(float)begin,1.0,tmp);
	ajGraphxyAddGraph(graphs,tmGraph);
    }


    if(docg || dopc || doobsexp)
    {
	ajGraphxyTitleC(graphs,seq);
	ajGraphxySetOverLap(graphs,ajFalse);    
	ajGraphxyDisplay(graphs, AJTRUE);
    }
    

    if(docg)
	AJFREE (tmp);
}

void dumpfeatout(AjPFeatTabOut featout, AjBool *thresh, char *seqname,
		 ajint begin,int len)
{
    AjBool island;
    ajint startpos=0;
    ajint endpos;
    ajint i;
    AjPFeatTable feattable;
    AjPFeatLexicon dict=NULL;
    AjPStr name=NULL,score=NULL,desc=NULL,source=NULL,type=NULL;
    AjEFeatStrand strand=AjStrandWatson;
    AjEFeatFrame frame=AjFrameUnknown;
    AjPFeature feature;
    
    ajStrAssC(&name,seqname);
    
    feattable = ajFeatTabNew(name,dict);
    dict = feattable->Dictionary;
    
    ajStrAssC(&source,"cpgplot");
    ajStrAssC(&type,"misc_feature");


    island = ajFalse;
    for(i=0;i<len;++i)
    {
	if(island)
	{
	    island = thresh[i];
	    if(!island)
	    {
		endpos = i;
		feature = ajFeatureNew(feattable, source, type,
				       startpos+begin+1,endpos+begin, score,
				       strand, frame,
				       desc , 0, 0) ;    
		if(!feature)
		  ajDebug("Error feature not added to feature table");
	    }
	}
	else
	{
	    island=thresh[i];
	    if(island) startpos=i;
	}
    }

    if(island)
    {
	endpos=len-1;
	feature = ajFeatureNew(feattable, source, type,
			       startpos+begin+1,endpos+begin, score, strand,
			       frame, desc , 0, 0) ;    
    }
    ajFeatSortByStart(feattable);
    ajFeaturesWrite (featout, feattable);
    ajFeatTabDel(&feattable);
    ajFeatDeleteDict(dict);

    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&name);
    
    return;
}

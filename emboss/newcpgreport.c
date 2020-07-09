#include "emboss.h"
#include <math.h>
#include <stdlib.h>


void findbases(AjPStr *substr, ajint begin, ajint len, ajint window, ajint shift,
               float *obsexp, float *xypc, AjPStr *bases, float *obsexpmax,
	       ajint *plstart, ajint *plend, char *seq);
void countbases(char *seq, char *bases, ajint window, ajint *cx, ajint *cy,
		ajint *cxpy);
void identify(AjPFile outf, float *obsexp, float *xypc, AjBool *thresh,
	      ajint begin, ajint len, ajint shift, char *bases, char *name,
	      ajint minlen, float minobsexp, float minpc, char *seq);
void reportislands(AjPFile outf, AjBool *thresh, char *bases, char *name,
		   float minobsexp, float minpc, ajint minlen, ajint begin,
		   ajint len, char *seq);
void compisl(AjPFile outf, char *p, ajint begin1, ajint end1);



int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    bases=NULL;
    /*AjBool    doobsexp;
    AjBool    dopc;
        AjBool    docg; NOT USED*/
    
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

    
    embInit("newcpgreport",argc,argv);
    
    seqall    = ajAcdGetSeqall("sequence");
    window    = ajAcdGetInt("window");
    shift     = ajAcdGetInt("shift");
    outf      = ajAcdGetOutfile("outfile");
    minobsexp = ajAcdGetFloat("minoe");
    minlen    = ajAcdGetInt("minlen");
    minpc     = ajAcdGetFloat("minpc");
    /*doobsexp  = ajAcdGetBool("obsexp");
    docg      = ajAcdGetBool("cg");
    dopc      = ajAcdGetBool("pc");  NOT USED */
    
    
    substr = ajStrNew();
    bases  = ajStrNewC("CG");
    maxarr = 0;

    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	strand = ajSeqStrCopy(seq);
	ajStrToUpper(&strand);

	ajStrAssSubC(&substr,ajStrStr(strand),--begin,--end);
	len=ajStrLen(substr);

	if (len > maxarr) {
	  AJCRESIZE(obsexp, len);
	  AJCRESIZE(thresh, len);
	  AJCRESIZE(xypc, len);
	}
	for(i=0;i<len;++i)
	    obsexp[i]=xypc[i]=0.0;
	

	findbases(&substr, begin, len, window, shift, obsexp, xypc,
		  &bases, &obsexpmax, &plstart, &plend, ajStrStr(strand));
	
	identify(outf, obsexp, xypc, thresh, 0, len, shift,
		 ajStrStr(bases), ajSeqName(seq), minlen, minobsexp, minpc,
		 ajStrStr(strand));
	
	ajStrDel(&strand);
   }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);
    
    ajExit();
    return 0;
}



void findbases(AjPStr *substr, ajint begin, ajint len, ajint window, ajint shift,
               float *obsexp, float *xypc, AjPStr *bases, float *obsexpmax,
	       ajint *plstart, ajint *plend, char *seq)
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
    /*    ajint start;
	  ajint stop;*/

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
	/*	start = i;
		stop = i+window-1; NOT USED */
	p = ajStrStr(*substr) + i;
	countbases(p, q, window, &cx, &cy, &cxpy);
	obs = (float) cxpy;
	exp = (float)(cx*cy)/windowf;
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
	      ajint minlen, float minobsexp, float minpc, char *seq) 
{
  static ajint avwindow=10;
  float avpc;
  float avobsexp;
  float sumpc;
  float sumobsexp;
    
  ajint i;
  ajint pos;
  ajint posmin;
  /*  ajint posmax;*/
  ajint sumlen;
  ajint first;


  for(i=0; i<len; ++i) thresh[i]=ajFalse;

  sumlen=0;
  posmin = begin;
  for(pos=0,first=0;pos<(len-avwindow*shift);pos+=shift) {
    sumpc=sumobsexp=0.0;
    /*    posmax = pos+avwindow*shift; NOT USED */
    ajDebug("pos: %d max: %d\n", pos, pos+avwindow*shift);
    for(i=pos;i<=(pos+avwindow*shift);++i) {
      ajDebug("obsexp[%d] %.2f xypc[%d] %.2f\n",
	      i, obsexp[i], i, xypc[i]);
      sumpc += xypc[i];
      sumobsexp += obsexp[i];
    }
	
    avpc = sumpc/(float)avwindow;
    avobsexp = sumobsexp/(float)avwindow;
    ajDebug("sumpc: %.2f sumobsexp: %.2f\n", sumpc, sumobsexp);
    ajDebug(" avpc: %.2f  avobsexp: %.2f\n", avpc, avobsexp);
    if((avobsexp>minobsexp)&&(avpc>minpc)) {
      if(!sumlen) first=pos; /* start a new island */
      sumlen += shift;
      ajDebug(" ** hit first: %d sumlen: %d\n", first, sumlen);
    }
    else {
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
		minlen, begin, len, seq);

  return;
}




void reportislands(AjPFile outf, AjBool *thresh, char *bases, char *name,
		   float minobsexp, float minpc, ajint minlen, ajint begin,
		   ajint len, char *seq)
{
    AjBool island;
    ajint startpos=0;
    ajint endpos;
    ajint slen;
    ajint i;
    ajint cnt=0;
    
    ajFmtPrintF(outf,"ID   %s  %d BP.\n",name, len);
    ajFmtPrintF(outf,"XX\n");
    ajFmtPrintF(outf,"DE   CpG Island report.\n");
    ajFmtPrintF(outf,"XX\n");
    ajFmtPrintF(outf,"CC   Obs/Exp ratio > %.2f.\n",minobsexp);
    ajFmtPrintF(outf,"CC   %% %c + %% %c > %.2f.\n",bases[0],
		bases[1],minpc);
    ajFmtPrintF(outf,"CC   Length > %d.\n",minlen);
    ajFmtPrintF(outf,"XX\n");
    ajFmtPrintF(outf,"FH   Key              Location/Qualifiers\n");
    
    
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
		ajFmtPrintF(outf,"FT   CpG island       %d..%d\n",
			    startpos+begin+1, endpos+begin);
		ajFmtPrintF(outf,"FT                    /size=%d\n",
			    slen);
		compisl(outf, seq, startpos+begin+1, endpos+begin);	
		++cnt;
		
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
       	ajFmtPrintF(outf,"FT   CpG island       %d..%d\n",
			    startpos+begin+1, endpos+begin);
        ajFmtPrintF(outf,"FT                    /size=%d\n",
		    slen);
	compisl(outf, seq, startpos+begin+1, endpos+begin);
	++cnt;
    }
    
    if(cnt < 1)
    {
	ajFmtPrintF(outf,"FT   no islands detected\n");
    }
    else
    {
	ajFmtPrintF(outf,"FT   numislands       %d\n",cnt);
    }
    
    ajFmtPrintF(outf,"//\n");
    

    return;
}

void compisl(AjPFile outf, char *p, ajint begin1, ajint end1)
{
    ajint C=0;
    ajint G=0;
    ajint CG=0;
    ajint i=0;
    ajint sumcg=0;
    ajint len=0;
    float pcg;
    float oe;

    len = end1-begin1+1;
    
    
    for(i=begin1;i<end1;++i)
    {
	if(p[i]=='C')
	{
	    ++C;
	    if (p[i+1]=='G')
	    {
		++CG;
	    }
	    
	}
	
	if (p[i]=='G')
	{
	    ++G;
	}
    }
    
    sumcg = C + G;
    pcg   = ((float)sumcg/(float)len) * 100.;
    oe    = (float)(CG * len)/(float)(C * G);

    ajFmtPrintF(outf,"FT                    /Sum C+G=%d\n",sumcg);
    ajFmtPrintF(outf,"FT                    /Percent CG=%.2f\n",pcg);
    ajFmtPrintF(outf,"FT                    /ObsExp=%.2f\n",oe);
    
    
    return;
    
}

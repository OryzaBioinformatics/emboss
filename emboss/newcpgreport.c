/* @source newcpgreport application
**
** Reports ALL cpg rich regions in a sequence
** @author: Copyright (C) Rodrigo Lopez (rls@ebi.ac.uk)
** @@
**
** Original program "CPGREPORT" by Rodrigo Lopez (EGCG 1995)
**  CpG island finder. Larsen et al Genomics 13 1992 p1095-1107
**  "usually defined as >200bp with %GC > 50% and obs/exp CpG >
**  0.6". Here use running sum rather than window to score: if not CpG
**  at position i, then decrement runSum counter, but if CpG then runSum
**  += CPGSCORE.     Spans > threshold are searched
**  for recursively.
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




static void newcpgreport_findbases(AjPStr *substr, ajint begin, ajint len,
				   ajint window, ajint shift, float *obsexp,
				   float *xypc, AjPStr *bases,
				   float *obsexpmax, ajint *plstart,
				   ajint *plend);
static void newcpgreport_countbases(char *seq, char *bases, ajint window,
				    ajint *cx, ajint *cy, ajint *cxpy);
static void newcpgreport_identify(AjPFile outf, float *obsexp, float *xypc,
				  AjBool *thresh, ajint begin, ajint len,
				  ajint shift, char *bases, char *name,
				  ajint minlen, float minobsexp, float minpc,
				  char *seq);
static void newcpgreport_reportislands(AjPFile outf, AjBool *thresh,
				       char *bases, char *name,
				       float minobsexp, float minpc,
				       ajint minlen, ajint begin,
				       ajint len, char *seq);
static void newcpgreport_compisl(AjPFile outf, char *p, ajint begin1,
				 ajint end1);




/* @prog newcpgreport *********************************************************
**
** Report CpG rich areas
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq    = NULL;
    AjPFile outf  = NULL;
    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr bases  = NULL;

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

    float  *xypc   = NULL;
    float  *obsexp = NULL;
    AjBool *thresh = NULL;
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

    substr = ajStrNew();
    bases  = ajStrNewC("CG");
    maxarr = 0;

    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	strand = ajSeqStrCopy(seq);
	ajStrToUpper(&strand);

	ajStrAssSubC(&substr,ajStrStr(strand),--begin,--end);
	len=ajStrLen(substr);

	if(len > maxarr)
	{
	    AJCRESIZE(obsexp, len);
	    AJCRESIZE(thresh, len);
	    AJCRESIZE(xypc, len);
	    maxarr = len;
	}
	for(i=0;i<len;++i)
	    obsexp[i]=xypc[i]=0.0;


	newcpgreport_findbases(&substr, begin, len, window, shift, obsexp,
			       xypc, &bases, &obsexpmax, &plstart, &plend);

	newcpgreport_identify(outf, obsexp, xypc, thresh, 0, len, shift,
			      ajStrStr(bases), ajSeqName(seq), minlen,
			      minobsexp, minpc, ajStrStr(strand));

	ajStrDel(&strand);
   }


    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);

    ajExit();

    return 0;
}




/* @funcstatic newcpgreport_findbases *****************************************
**
** Undocumented.
**
** @param [r] substr [AjPStr*] sequence
** @param [r] begin [ajint] start in sequence
** @param [r] len [ajint] length
** @param [r] window [ajint] window
** @param [r] shift [ajint] shift
** @param [w] obsexp [float*] observed/expected
** @param [w] xypc [float*] CG content
** @param [w] bases [AjPStr*] bases to look for
** @param [w] obsexpmax [float*] maximum obsexp
** @param [w] plstart [ajint*] start
** @param [w] plend [ajint*] end
** @@
******************************************************************************/


static void newcpgreport_findbases(AjPStr *substr, ajint begin, ajint len,
				   ajint window, ajint shift, float *obsexp,
				   float *xypc, AjPStr *bases,
				   float *obsexpmax, ajint *plstart,
				   ajint *plend)
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
    ajint j = 0;
    ajint offset;

    char *p;
    char *q;

    windowf = (float)window;
    *obsexpmax = 0.0;
    offset     = window/2;
    *plstart   = offset;
    q = ajStrStr(*bases);

    for(i=0; i<(len-window+1);i+=shift)
    {
	j = i+offset;
	p = ajStrStr(*substr) + i;
	newcpgreport_countbases(p, q, window, &cx, &cy, &cxpy);
	obs = (float) cxpy;
	exp = (float)(cx*cy)/windowf;
	cxf = (float)cx;
	cyf = (float)cy;
	if(!exp)
	    obsexp[j] = 0.0;
	else
	{
	    obsexp[j]  = obs/exp;
	    *obsexpmax = (*obsexpmax > obsexp[j]) ? *obsexpmax : obsexp[j];
	}
	xypc[j] = (cxf/windowf)*100.0 + (cyf/windowf)*100.0;
    }

    *plend = j;
    return;
}




/* @funcstatic newcpgreport_countbases ****************************************
**
** Undocumented.
**
** @param [r] seq [char*] sequence
** @param [r] bases [char*] two-base to look for e.g. GC
** @param [r] window [ajint] window
** @param [w] cx [ajint*] amount C
** @param [w] cy [ajint*] amount G
** @param [w] cxpy [ajint*] amount CG
** @@
******************************************************************************/

static void newcpgreport_countbases(char *seq, char *bases, ajint window,
				    ajint *cx, ajint *cy, ajint *cxpy)
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
	codea = codeb;
	codeb = ajAZToBin(seq[i+1]);
	if(codea && !(codea & (15-codex)))
	{
	    ++*cx;

	    if(codeb && !(codeb & (15-codey)))
		++*cxpy;
	}

	if(codea && !(codea & (15-codey)))
	    ++*cy;
    }

    return;
}




/* @funcstatic newcpgreport_identify ******************************************
**
**    This subroutine identifies the CpG line, identifying the possible
**    dinucleotide 'islands' in the sequence. These are defined as
**    base positions where, over an average of 10 windows, the calculated
**    % composition is over 50% and the calculated Obs/Exp ratio is over 0.6
**    and the conditions hold for a minimum of 200 bases.
**
** @param [w] outf [AjPFile] Undocumented
** @param [r] obsexp [float*] Undocumented
** @param [?] xypc [float*] Undocumented
** @param [?] thresh [AjBool*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] len [ajint] Undocumented
** @param [?] shift [ajint] Undocumented
** @param [?] bases [char*] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] minlen [ajint] Undocumented
** @param [?] minobsexp [float] Undocumented
** @param [?] minpc [float] Undocumented
** @param [?] seq [char*] Undocumented
** @@
******************************************************************************/

static void newcpgreport_identify(AjPFile outf, float *obsexp, float *xypc,
				  AjBool *thresh, ajint begin, ajint len,
				  ajint shift, char *bases, char *name,
				  ajint minlen, float minobsexp, float minpc,
				  char *seq)
{
    static ajint avwindow = 10;
    float avpc;
    float avobsexp;
    float sumpc;
    float sumobsexp;

    ajint i;
    ajint pos;
    ajint posmin;

    ajint sumlen;
    ajint first;


    for(i=0; i<len; ++i)
	thresh[i] = ajFalse;

    sumlen=0;
    posmin = begin;
    for(pos=0,first=0;pos<(len-avwindow*shift);pos+=shift)
    {
	sumpc = sumobsexp = 0.0;

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
	    if(!sumlen)
		first=pos;	/* start a new island */
	    sumlen += shift;
	    ajDebug(" ** hit first: %d sumlen: %d\n", first, sumlen);
	}
	else
	{
	    if(sumlen >= minlen)
	    {				/* island long enough? */
		for(i=first; i<=pos-shift;++i)
		    thresh[i]=ajTrue;
	    }
	    sumlen = 0;
	}
    }

    if(sumlen>=minlen)
    {
	for(i=first;i<len;++i)
	    thresh[i] = ajTrue;
    }




    newcpgreport_reportislands(outf, thresh, bases, name, minobsexp, minpc,
			       minlen, begin, len, seq);

    return;
}




/* @funcstatic newcpgreport_reportislands *************************************
**
** Undocumented.
**
** @param [?] outf [AjPFile] Undocumented
** @param [?] thresh [AjBool*] Undocumented
** @param [?] bases [char*] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] minobsexp [float] Undocumented
** @param [?] minpc [float] Undocumented
** @param [?] minlen [ajint] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] len [ajint] Undocumented
** @param [?] seq [char*] Undocumented
** @@
******************************************************************************/


static void newcpgreport_reportislands(AjPFile outf, AjBool *thresh,
				       char *bases, char *name,
				       float minobsexp, float minpc,
				       ajint minlen, ajint begin,
				       ajint len, char *seq)
{
    AjBool island;
    ajint startpos = 0;
    ajint endpos;
    ajint slen;
    ajint i;
    ajint cnt = 0;

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
		newcpgreport_compisl(outf, seq, startpos+begin+1,
				     endpos+begin);
		++cnt;

	    }
	}
	else
	{
	    island = thresh[i];
	    if(island)
		startpos=i;

	}
    }

    if(island)
    {
        slen   = len-startpos+1;
	endpos = len-1;
       	ajFmtPrintF(outf,"FT   CpG island       %d..%d\n",
			    startpos+begin+1, endpos+begin);
        ajFmtPrintF(outf,"FT                    /size=%d\n",
		    slen);
	newcpgreport_compisl(outf, seq, startpos+begin+1, endpos+begin);
	++cnt;
    }

    if(cnt < 1)
	ajFmtPrintF(outf,"FT   no islands detected\n");
    else
	ajFmtPrintF(outf,"FT   numislands       %d\n",cnt);

    ajFmtPrintF(outf,"//\n");


    return;
}




/* @funcstatic newcpgreport_compisl *******************************************
**
** Undocumented.
**
** @param [?] outf [AjPFile] Undocumented
** @param [?] p [char*] Undocumented
** @param [?] begin1 [ajint] Undocumented
** @param [?] end1 [ajint] Undocumented
** @@
******************************************************************************/

static void newcpgreport_compisl(AjPFile outf, char *p, ajint begin1,
				 ajint end1)
{
    ajint C  = 0;
    ajint G  = 0;
    ajint CG = 0;
    ajint i  = 0;
    ajint sumcg = 0;
    ajint len = 0;
    float pcg;
    float oe;

    len = end1-begin1+1;

    for(i=begin1;i<end1;++i)
    {
	if(p[i]=='C')
	{
	    ++C;
	    if(p[i+1]=='G')
		++CG;
	}

	if(p[i]=='G')
	    ++G;
    }

    sumcg = C + G;
    pcg   = ((float)sumcg/(float)len) * 100.;
    oe    = (float)(CG * len)/(float)(C * G);

    ajFmtPrintF(outf,"FT                    /Sum C+G=%d\n",sumcg);
    ajFmtPrintF(outf,"FT                    /Percent CG=%.2f\n",pcg);
    ajFmtPrintF(outf,"FT                    /ObsExp=%.2f\n",oe);


    return;
}

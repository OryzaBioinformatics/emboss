/* @source water application
**
** True Smith-Waterman best local alignment
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


/* @prog water ***************************************************************
**
** Smith-Waterman local alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPAlign align;
    AjPSeqall seqall;
    AjPSeq a;
    AjPSeq b;
    AjPStr m;
    AjPStr n;
    AjPStr ss;
    
    AjPFile outf=NULL;
    AjBool  show=ajFalse;
    
    ajint    lena;
    ajint    lenb;
    ajint    i;
    
    char   *p;
    char   *q;

    ajint start1=0;
    ajint start2=0;
    
    
    float  *path;
    ajint    *compass;
    
    AjPMatrixf matrix;
    AjPSeqCvt  cvt=0;
    float      **sub;

    float gapopen;
    float gapextend;

    ajint maxarr=1000;
    ajint len;

    float score;
    ajint begina;
    ajint beginb;

    AjBool dosim=ajFalse;
    AjBool fasta=ajFalse;
    
    float id=0.;
    float sim=0.;
    float idx=0.;
    float simx=0.;


    embInit("water", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    a         = ajAcdGetSeq("sequencea");
    ajSeqTrim(a);
    seqall    = ajAcdGetSeqall("seqall");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    dosim     = ajAcdGetBool("similarity");
    fasta     = ajAcdGetBool("fasta");
    align     = ajAcdGetAlign("outfile");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */
    /* show      = ajAcdGetBool("showinternals"); */

    m  = ajStrNew();
    n  = ajStrNew();
    ss = ajStrNew();
    
    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    begina=ajSeqBegin(a)+ajSeqOffset(a);

    while(ajSeqallNext(seqall,&b))
    {
	lena = ajSeqLen(a);
	ajSeqTrim(b);
	lenb = ajSeqLen(b);

	len = lena*lenb;
	if(len>maxarr)
	{
	    AJCRESIZE(path,len);
	    if(!path)
		ajFatal("Sequences too big. Try 'matcher' or 'supermatcher'");
	    AJCRESIZE(compass,len);
	    if(!compass)
		ajFatal("Sequences too big. Try 'matcher' or 'supermatcher'");
	    maxarr=len;
	}

	beginb=ajSeqBegin(b)+ajSeqOffset(b);

	p = ajSeqChar(a);
	q = ajSeqChar(b);

	ajStrAssC(&m,"");
	ajStrAssC(&n,"");
	
	embAlignPathCalc(p,q,lena,lenb,gapopen,gapextend,path,sub,cvt,
			compass,show);

	score=embAlignScoreSWMatrix(path,compass,gapopen,gapextend,a,b,lena,
			    lenb,sub,cvt,&start1,&start2);

	embAlignWalkSWMatrix(path,compass,gapopen,gapextend,a,b,&m,&n,lena,
			    lenb,sub,cvt,&start1,&start2);

	if(outf && !fasta)
	{
	  embAlignPrintLocal(outf,ajSeqChar(a),ajSeqChar(b),m,n,start1,
			     start2,score,1,sub,cvt,ajSeqName(a),
			     ajSeqName(b),begina,beginb);

	  if(dosim)
	  {
	    embAlignCalcSimilarity(m,n,sub,cvt,lena,lenb,&id,&sim,&idx,
				   &simx);
	    ajFmtPrintF(outf,"\n%%id = %5.2f\t\t\t%%similarity = %5.2f\n",
			id,sim);
	    ajFmtPrintF(outf,"Overall %%id = %5.2f\t\tOverall "
			"%%similarity = %5.2f\n",idx,simx);
	  }
	}
	else if (outf)	{
	  ajFmtPrintF(outf,">%s\n",ajSeqName(a));

	  len = ajStrLen(m);
	  for (i=0;i<len; i+=60)
	  {
		(void) ajStrAssSub (&ss,m,i,i+60-1);
		(void) ajFmtPrintF (outf,"%S\n", ss);
	    }
	    ajFmtPrintF(outf,">%s\n",ajSeqName(b));
	    len = ajStrLen(n);
	    for (i=0;i<len; i+=60)
	    {
		(void) ajStrAssSub (&ss,n,i,i+60-1);
		(void) ajFmtPrintF (outf,"%S\n", ss);
	    }
	}
	ajDebug("ReportLocal call start1:%d begina:%d start2:%d beginb:%d\n",
		start1, begina, start2, beginb);
	embAlignReportLocal (align, a, b, m, n,
			     start1, start2,
			     gapopen, gapextend,
			     score, matrix, begina, beginb);
    }

    ajAlignClose(align);
    ajAlignDel(&align);

    AJFREE (compass);
    AJFREE (path);
    
    AJFREE(compass);
    AJFREE(path);

    ajStrDel(&n);
    ajStrDel(&m);
    ajStrDel(&ss);
    
    ajExit();
    return 0;
}

/* @source needle application
**
** True Needleman-Wunsch global alignment
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




/* @prog needle ***************************************************************
**
** Needleman-Wunsch global alignment
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

    AjPFile outf = NULL;
    AjBool show  = ajFalse;

    ajint    lena;
    ajint    lenb;
    ajint    i;

    char   *p;
    char   *q;

    ajint start1 = 0;
    ajint start2 = 0;

    float *path;
    ajint *compass;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;

    float gapopen;
    float gapextend;
    ajint maxarr = 1000; 	/* arbitrary. realloc'd if needed */
    ajint len;			

    float score;
    ajint begina;
    ajint beginb;

    AjBool dobrief = ajTrue;

    float id   = 0.;
    float sim  = 0.;
    float idx  = 0.;
    float simx = 0.;

    AjBool fasta = ajFalse;
    AjPStr tmpstr = NULL;

    embInit("needle", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    a         = ajAcdGetSeq("asequence");
    ajSeqTrim(a);
    seqall    = ajAcdGetSeqall("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    dobrief   = ajAcdGetBool("brief");

    align     = ajAcdGetAlign("outfile");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */
    /* show      = ajAcdGetBool("showinternals"); */

    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);

    m  = ajStrNew();
    n  = ajStrNew();
    ss = ajStrNew();

    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    begina = ajSeqBegin(a)+ajSeqOffset(a);

    while(ajSeqallNext(seqall,&b))
    {
	lena = ajSeqLen(a);
	ajSeqTrim(b);
	lenb = ajSeqLen(b);
	len = lena*lenb;

	if(len < 0)
	    ajFatal("Sequences too big. Try 'matcher' or 'supermatcher'");

	if(len>maxarr)
	{
	    AJCRESIZE(path,len);
	    if(!path)
		ajFatal("Sequences too big. Try 'stretcher'");
	    AJCRESIZE(compass,len);
	    if(!compass)
		ajFatal("Sequences too big. Try 'stretcher'");
	    maxarr=len;
	}

	beginb=ajSeqBegin(b)+ajSeqOffset(b);

	p = ajSeqChar(a);
	q = ajSeqChar(b);

	ajStrAssC(&m,"");
	ajStrAssC(&n,"");

	embAlignPathCalc(p,q,lena,lenb,gapopen,gapextend,path,sub,cvt,
			compass,show);

	score = embAlignScoreNWMatrix(path,a,b,sub,cvt,lena,lenb,
				      gapopen,compass,
				      gapextend,&start1,&start2);


	embAlignWalkNWMatrix(path,a,b,&m,&n,lena,lenb,&start1,&start2,gapopen,
			    gapextend,cvt,compass,sub);

	if(!fasta && outf)
	{
	    embAlignPrintGlobal(outf,p,q,m,n,start1,start2,score,1,sub,cvt,
				ajSeqName(a),ajSeqName(b),begina,beginb);

	    if(!dobrief)
	    {
		embAlignCalcSimilarity(m,n,sub,cvt,lena,lenb,&id,&sim,&idx,
				       &simx);
		ajFmtPrintF(outf,"\n%%id = %5.2f\t\t\t%%similarity = %5.2f\n",
			    id,sim);
		ajFmtPrintF(outf,"Overall %%id = %5.2f\t\tOverall "
			    "%%similarity = %5.2f\n",idx,simx);
	    }
	}
	else if(outf)		/* fasta format */
	{
	    ajFmtPrintF(outf,">%s\n",ajSeqName(a));

	    len = ajStrLen(m);
	    for(i=0;i<len; i+=60)
	    {
		ajStrAssSub(&ss,m,i,i+60-1);
		ajFmtPrintF(outf,"%S\n", ss);
	    }
	    ajFmtPrintF(outf,">%s\n",ajSeqName(b));
	    len = ajStrLen(n);
	    for(i=0;i<len; i+=60)
	    {
		ajStrAssSub(&ss,n,i,i+60-1);
		ajFmtPrintF(outf,"%S\n", ss);
	    }
	}

	embAlignReportGlobal(align, a, b ,m, n,
			     start1,start2,
			     gapopen, gapextend,
			     score, matrix, begina, beginb);

	if(!dobrief)
	{
	  embAlignCalcSimilarity(m,n,sub,cvt,lena,lenb,&id,&sim,&idx,
				 &simx);
	  ajFmtPrintAppS(&tmpstr,"Longest_Identity = %5.2f%%\n",
			 id);
	  ajFmtPrintAppS(&tmpstr,"Longest_Similarity = %5.2f%%\n",
			 sim);
	  ajFmtPrintAppS(&tmpstr,"Shortest_Identity = %5.2f%%\n",
			 idx);
	  ajFmtPrintAppS(&tmpstr,"Shortest_Similarity = %5.2f%%",
			 simx);
	  ajAlignSetSubHeaderApp(align, tmpstr);
	}
	ajAlignWrite(align);
	ajAlignReset(align);

    }

    ajAlignClose(align);
    ajAlignDel(&align);

    AJFREE(compass);
    AJFREE(path);

    ajStrDel(&n);
    ajStrDel(&m);
    ajStrDel(&ss);
    ajStrDel(&tmpstr);

    ajExit();

    return 0;
}

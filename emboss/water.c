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

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq a;
    AjPSeq b;
    AjPStr m;
    AjPStr n;

    AjPFile outf;
    AjBool  show;
    
    int    lena;
    int    lenb;

    char   *p;
    char   *q;

    int start1=0;
    int start2=0;
    
    
    float  *path;
    int    *compass;
    
    AjPMatrixf matrix;
    AjPSeqCvt  cvt=0;
    float      **sub;

    float gapopen;
    float gapextend;

    int maxarr=1000;
    int len;

    float score;
    int begina;
    int beginb;
    

    embInit("water", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    a         = ajAcdGetSeq("sequencea");
    ajSeqTrim(a);
    seqall    = ajAcdGetSeqall("seqall");
    show      = ajAcdGetBool("showinternals");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    outf      = ajAcdGetOutfile("outfile");

    m=ajStrNew();
    n=ajStrNew();

    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    begina=ajSeqBegin(a);

    while(ajSeqallNext(seqall,&b))
    {
	lena = ajSeqLen(a);
	ajSeqTrim(b);
	lenb = ajSeqLen(b);

	len = lena*lenb;
	if(len>maxarr)
	{
	    AJCRESIZE(path,len);
	    AJCRESIZE(compass,len);
	    maxarr=len;
	}

	beginb=ajSeqallBegin(seqall);

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

	embAlignPrintLocal(outf,ajSeqChar(a),ajSeqChar(b),m,n,start1,start2,
			  score,1,sub,cvt,ajSeqName(a),ajSeqName(b),
			  begina,beginb);
    }

    AJFREE (compass);
    AJFREE (path);
    
    ajStrDel(&n);
    ajStrDel(&m);

    ajExit();
    return 0;
}

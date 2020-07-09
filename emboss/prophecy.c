/* @source prophecy application
**
** Creates profiles and simple freuency matrices
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
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define AZ 27
#define GRIBSKOV_LENGTH 27
#define HENIKOFF_LENGTH 27

void simple_matrix(AjPSeqset seqset, AjPFile outf, AjPStr name,
		   int thresh);
void gribskov_profile(AjPSeqset seqset, float **sub,
		      AjPFile outf, AjPStr name, int thresh,
		      float gapopen, float gapextend, AjPStr *cons);
void henikoff_profile(AjPSeqset seqset, AjPMatrixf imtx, float **sub,
		      int thresh, AjPSeqCvt cvt, AjPFile outf, AjPStr name,
		      float gapopen, float gapextend, AjPStr *cons);


int main (int argc, char * argv[])
{

    AjPSeqset seqset;
    AjPFile   outf;
    AjPStr    name;
    AjPStr    cons;
    
    int       thresh;
    char *p;
    AjPStr *type;

    float **sub=NULL;
    AjPMatrixf imtx=0;
    AjPSeqCvt cvt=0;
    float gapopen;
    float gapextend;
    
    embInit ("prophecy", argc, argv);


    seqset = ajAcdGetSeqset ("sequence");
    name   = ajAcdGetString("name");
    thresh = ajAcdGetInt("threshold");
    imtx   = ajAcdGetMatrixf("datafile");
    type   = ajAcdGetList("type");
    outf   = ajAcdGetOutfile ("outf");

    gapopen   = ajAcdGetFloat("open");
    gapextend = ajAcdGetFloat("extension");
    
    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    p = ajStrStr(*type);
    cons = ajStrNewC("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
    ajStrAssC(&cons,"");
    
    if(*p=='F') simple_matrix(seqset,outf,name,thresh);
    if(*p=='G') gribskov_profile(seqset,sub,outf,name,thresh,
				 gapopen,gapextend,&cons);
    if(*p=='H') henikoff_profile(seqset,imtx,sub,thresh,cvt,outf,name,
				 gapopen,gapextend,&cons);
  

  
    ajFileClose(&outf);
    ajExit ();
    return 0;
}




void simple_matrix(AjPSeqset seqset, AjPFile outf, AjPStr name,
		   int thresh)
{
    char *p;
    int nseqs;
    int mlen;
    int len;
    int i;
    int j;
    int x;
    int px;
    
    int maxscore;
    int score;
    int *matrix[AZ+2];
    AjPStr cons=NULL;
    
    nseqs = ajSeqsetSize(seqset);
    if(nseqs<2)
	ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetLen(seqset);
    
    /* Check sequences are the same length. Warn if not */
    for(i=0;i<nseqs;++i)
    {
	p = ajSeqsetSeq(seqset,i);
	if(strlen(p)!=mlen)
	    ajWarn("Sequence lengths are not equal!");
    }
    
    for(i=0;i<AZ+2;++i)
	AJCNEW0(matrix[i], mlen);

    /* Load matrix */
    for(i=0;i<nseqs;++i)
    {
	p = ajSeqsetSeq(seqset,i);	
	len = strlen(p);
	for(j=0;j<len;++j)
	{
	    x = toupper((int)*p++);
	    ++matrix[ajAZToInt(x)][j];
	}
    }

    /* Get consensus sequence */
    cons = ajStrNew();
    for(i=0;i<mlen;++i)
    {
	px=x=-INT_MAX;
	for(j=0;j<AZ-1;++j)
	    if(matrix[j][i]>x)
	    {
		x=matrix[j][i];
		px=j;
	    }
	ajStrAppK(&cons,(char)(px+'A'));
    }
    
    /* Find maximum score for matrix */
    maxscore=0;
    for(i=0;i<mlen;++i)
    {
	for(j=score=0;j<AZ;++j)
	    score = AJMAX(score,matrix[j][i]);
	maxscore += score;
    }

    ajFmtPrintF(outf,"# Pure Frequency Matrix\n");
    ajFmtPrintF(outf,"# Columns are amino acid counts A->Z\n");
    ajFmtPrintF(outf,"# Rows are alignment positions 1->n\n");
    ajFmtPrintF(outf,"Simple\n");
    ajFmtPrintF(outf,"Name\t\t%s\n",ajStrStr(name));
    ajFmtPrintF(outf,"Length\t\t%d\n",mlen);
    ajFmtPrintF(outf,"Maximum score\t%d\n",maxscore);
    ajFmtPrintF(outf,"Thresh\t\t%d\n",thresh);
    ajFmtPrintF(outf,"Consensus\t%s\n",ajStrStr(cons));


    for(i=0;i<mlen;++i)
    {
	for(j=0;j<AZ;++j)
	    ajFmtPrintF(outf,"%-2d ",matrix[j][i]);
	ajFmtPrintF(outf,"\n");
    }
    
    for(i=0;i<AZ+2;++i)
	AJFREE (matrix[i]);

    ajStrDel(&cons);

    return;
}

    
	    
void gribskov_profile(AjPSeqset seqset, float **sub,
		      AjPFile outf, AjPStr name, int thresh,
		      float gapopen, float gapextend, AjPStr *cons)
{
    AjPMatrixf imtx=0;
    AjPSeqCvt cvt=0;
    AjPStr mname=NULL;
    
    float **mat;
    int nseqs;
    int mlen;
    int i;
    int j;
    static char *valid="ACDEFGHIKLMNPQRSTVWY";
    char *p;
    char *q;
    float score;
    float sum;
    int   gsum;
    float   mmax;
    float   pmax;
    float   psum;
    int  start;
    int  end;
    int  pos;
    float  x;
    int  px;
    
    float **weights;
    int *gaps;

    
    mname=ajStrNewC("Epprofile");
    ajMatrixfRead(&imtx,mname);
    ajStrDel(&mname);
		 
    nseqs = ajSeqsetSize(seqset);
    mlen  = ajSeqsetLen(seqset);

    sub = ajMatrixfArray(imtx);
    cvt = ajMatrixfCvt(imtx);


    /* Set gaps to be maximum length of gap that can occur
     * including that position
     */
    AJCNEW(gaps, mlen);
    for(i=0;i<mlen;++i)
    {
	gsum=0;
	for(j=0;j<nseqs;++j)
	{
	    p=ajSeqsetSeq(seqset,j);
	    if(ajAZToInt(p[i])!=27) continue; /* if not a gap */
	    pos = i;
	    while(pos>-1 && ajAZToInt(p[pos])==27) --pos;
	    start = ++pos;
	    pos=i;
	    while(pos<mlen && ajAZToInt(p[pos])==27) ++pos;
	    end = pos-1;
	    gsum = AJMAX(gsum, (end-start)+1);
	}
	gaps[i]=gsum;
    }


    /* get maximum score in scoring matrix */
    mmax=0.0;
    p=valid;
    while(*p)
    {
	q=valid;
	while(*q)
	{
	    mmax=(mmax>sub[ajSeqCvtK(cvt,*p)][ajSeqCvtK(cvt,*q)]) ? mmax :
		sub[ajSeqCvtK(cvt,*p)][ajSeqCvtK(cvt,*q)];
	    ++q;
	}
	++p;
    }


    /* Create the weight matrix and zero it */
    AJCNEW(weights, mlen);
    for(i=0;i<mlen;++i)
      AJCNEW0(weights[i], GRIBSKOV_LENGTH+1);

    /*
     *  count the number of times each residue occurs at each
     *  position in the alignment
     */
    for(i=0;i<mlen;++i)
	for(j=0;j<nseqs;++j)
	{
	    p=ajSeqsetSeq(seqset,j);
	    weights[i][ajAZToInt(p[i])] += ajSeqsetWeight(seqset,j);
	}


    px = -INT_MAX;
    for(i=0;i<mlen;++i)
    {
	x = (float)-INT_MAX;
	for(j=0;j<AZ-1;++j)
	    if(weights[i][j]>x)
	    {
		x=weights[i][j];
		px=j;
	    }
	ajStrAppK(cons,(char)(px+'A'));
    }
    
    
    /* Now normalise the weights */
    for(i=0;i<mlen;++i)
	for(j=0;j<GRIBSKOV_LENGTH;++j)
	    weights[i][j] /= (float)nseqs;


    /* Create the profile matrix n*GRIBSKOV_LENGTH and zero it */
    AJCNEW(mat, mlen);
    for(i=0;i<mlen;++i)
	AJCNEW0(mat[i],GRIBSKOV_LENGTH);

    /* Fill the profile with aa scores */
    for(i=0;i<mlen;++i)
	for(p=valid;*p;++p)
	{
	    sum = 0.0;
	    q = valid;
	    while(*q)
	    {
		score = weights[i][ajAZToInt(*q)];
		score *= (float)(sub[ajSeqCvtK(cvt,*p)][ajSeqCvtK(cvt,*q)]);
		sum += score;
		++q;
	    }
	    mat[i][ajAZToInt(*p)] = sum;
	}

    /* Calculate gap penalties */
    for(i=0;i<mlen;++i)
	mat[i][GRIBSKOV_LENGTH-1]= (mmax / (gapopen+gapextend+(float)gaps[i]));


    /* Get maximum matrix score */
    psum=0.0;
    for(i=0;i<mlen;++i)
    {
	pmax= (float)-INT_MAX;
	for(j=0;j<AZ;++j)
	    pmax=(pmax>mat[i][j]) ? pmax : mat[i][j];
	psum+=pmax;
    }
    
    /* Print matrix */

    ajFmtPrintF(outf,"# Gribskov Protein Profile\n");
    ajFmtPrintF(outf,"# Columns are amino acids A->Z\n");
    ajFmtPrintF(outf,"# Last column is indel penalty\n");
    ajFmtPrintF(outf,"# Rows are alignment positions 1->n\n");
    ajFmtPrintF(outf,"Gribskov\n");
    ajFmtPrintF(outf,"Name\t\t%s\n",ajStrStr(name));
    ajFmtPrintF(outf,"Matrix\t\tpprofile\n");
    ajFmtPrintF(outf,"Length\t\t%d\n",mlen);
    ajFmtPrintF(outf,"Max_score\t%.2f\n",psum);
    ajFmtPrintF(outf,"Threshold\t%d\n",thresh);
    ajFmtPrintF(outf,"Gap_open\t%.2f\n",gapopen);
    ajFmtPrintF(outf,"Gap_extend\t%.2f\n",gapextend);
    ajFmtPrintF(outf,"Consensus\t%s\n",ajStrStr(*cons));
    
    for(i=0;i<mlen;++i)
    {
	for(j=0;j<GRIBSKOV_LENGTH;++j)
	    ajFmtPrintF(outf,"%.2f ",mat[i][j]);
	ajFmtPrintF(outf,"%.2f\n",mat[i][GRIBSKOV_LENGTH-1]);
    }


    for(i=0;i<mlen;++i)
    {
	AJFREE (mat[i]);
	AJFREE (weights[i]);
    }
    AJFREE (mat);
    AJFREE (weights);

    AJFREE (gaps);
    return;
}




void henikoff_profile(AjPSeqset seqset, AjPMatrixf imtx, float **sub,
		      int thresh, AjPSeqCvt cvt, AjPFile outf, AjPStr name,
		      float gapopen, float gapextend, AjPStr *cons)
{
    float **mat;
    int nseqs;
    int mlen;
    int i;
    int j;
    static char *valid="ACDEFGHIKLMNPQRSTVWY";
    char *p;
    char *q;
    float score;
    float sum;
    float psum;
    float pmax;
    int   gsum;
    int   mmax;
    int  start;
    int  end;
    int  pos;
    
    float **weights;
    int *gaps;
    int *pcnt;

    float x;
    int px;
    

    nseqs = ajSeqsetSize(seqset);
    mlen  = ajSeqsetLen(seqset);

    sub = ajMatrixfArray(imtx);
    cvt = ajMatrixfCvt(imtx);


    /* Set gaps to be maximum length of gap that can occur
     * including that position
     */
    AJCNEW(gaps, mlen);
    for(i=0;i<mlen;++i)
    {
	gsum=0;
	for(j=0;j<nseqs;++j)
	{
	    p=ajSeqsetSeq(seqset,j);
	    if(ajAZToInt(p[i])!=27) continue; /* if not a gap */
	    pos = i;
	    while(pos>-1 && ajAZToInt(p[pos])==27) --pos;
	    start = ++pos;
	    pos=i;
	    while(pos<mlen && ajAZToInt(p[pos])==27) ++pos;
	    end = pos-1;
	    gsum = AJMAX(gsum, (end-start)+1);
	}
	gaps[i]=gsum;
    }

    /* get maximum score in scoring matrix */
    mmax=0;
    p=valid;
    while(*p)
    {
	q=valid;
	while(*q)
	{
	    mmax=(mmax>sub[ajSeqCvtK(cvt,*p)][ajSeqCvtK(cvt,*q)]) ? mmax :
		sub[ajSeqCvtK(cvt,*p)][ajSeqCvtK(cvt,*q)];
	    ++q;
	}
	++p;
    }


    /* Create the weight matrix and zero it */
    AJCNEW(weights, mlen);
    for(i=0;i<mlen;++i)
	AJCNEW0(weights[i],HENIKOFF_LENGTH+1);

    /*
     *  count the number of times each residue occurs at each
     *  position in the alignment
     */
    for(i=0;i<mlen;++i)
	for(j=0;j<nseqs;++j)
	{
	    p=ajSeqsetSeq(seqset,j);
	    weights[i][ajAZToInt(p[i])] += ajSeqsetWeight(seqset,j);
	}

    px = -INT_MAX;
    for(i=0;i<mlen;++i)
    {
	x=(float)-INT_MAX;
	for(j=0;j<AZ-1;++j)
	    if(weights[i][j]>x)
	    {
		x = weights[i][j];
		px=j;
	    }
	ajStrAppK(cons,(char)(px+'A'));
    }
    


    /* Count the number of different residues at each position */

    AJCNEW0(pcnt, mlen);

    for(i=0;i<mlen;++i)
	for(j=0;j<HENIKOFF_LENGTH-1;++j)
	    if(weights[i][j])
		++pcnt[i];
    
    /* weights = 1/(num diff res * count of residues at that position */
    for(i=0;i<mlen;++i)
	for(j=0;j<HENIKOFF_LENGTH-1;++j)
	    if(weights[i][j])
		weights[i][j] = 1.0/(weights[i][j]*(float)pcnt[i]);


    /* Create the profile matrix n*HENIKOFF_LENGTH */
    AJCNEW(mat, mlen);
    for(i=0;i<mlen;++i)
      AJCNEW0(mat[i],HENIKOFF_LENGTH);
    
    /* Fill the profile with aa scores */
    for(i=0;i<mlen;++i)
	for(p=valid;*p;++p)
	{
	    sum = 0.0;
	    q = valid;
	    while(*q)
	    {
		score = weights[i][ajAZToInt(*q)];
		score *= sub[ajSeqCvtK(cvt,*p)][ajSeqCvtK(cvt,*q)];
		sum += score;
		++q;
	    }
	    mat[i][ajAZToInt(*p)] = sum;
	}

    /* Calculate gap penalties */
    for(i=0;i<mlen;++i)
	mat[i][HENIKOFF_LENGTH-1]=(mmax / (gapopen+gapextend+
					  (float)gaps[i]));


    /* Get maximum matrix score */
    psum=0.0;
    for(i=0;i<mlen;++i)
    {
	pmax= (float)-INT_MAX;
	for(j=0;j<HENIKOFF_LENGTH-1;++j)
	    pmax=(pmax>mat[i][j]) ? pmax : mat[i][j];
	psum+=pmax;
    }

    /* Print matrix */

    ajFmtPrintF(outf,"# Henikoff Protein Profile\n");
    ajFmtPrintF(outf,"# Columns are amino acids A->Z\n");
    ajFmtPrintF(outf,"# Last column is indel penalty\n");
    ajFmtPrintF(outf,"# Rows are alignment positions 1->n\n");
    ajFmtPrintF(outf,"Henikoff\n");
    ajFmtPrintF(outf,"Name\t\t%s\n",ajStrStr(name));
    ajFmtPrintF(outf,"Matrix\t\t%s\n",ajStrStr(ajMatrixfName(imtx)));
    ajFmtPrintF(outf,"Length\t\t%d\n",mlen);
    ajFmtPrintF(outf,"Max_score\t%.2f\n",psum);
    ajFmtPrintF(outf,"Threshold\t%d\n",thresh);
    ajFmtPrintF(outf,"Gap_open\t%.2f\n",gapopen);
    ajFmtPrintF(outf,"Gap_extend\t%.2f\n",gapextend);
    ajFmtPrintF(outf,"Consensus\t%s\n",ajStrStr(*cons));

    for(i=0;i<mlen;++i)
    {
	for(j=0;j<HENIKOFF_LENGTH;++j)
	    ajFmtPrintF(outf,"%.2f ",mat[i][j]);
	ajFmtPrintF(outf,"%.2f\n",mat[i][j-1]);
    }
    

    for(i=0;i<mlen;++i)
    {
	AJFREE (mat[i]);
	AJFREE (weights[i]);
    }
    AJFREE (mat);
    AJFREE (weights);
    AJFREE (gaps);
    AJFREE (pcnt);
    return;
}

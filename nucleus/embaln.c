/* @source embaln.c
**
** General routines for alignment.
** Copyright (c) 1999 Alan Bleasby
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
#include <math.h>


/* @func embAlignPathCalc  *************************************************
**
** Create path matrix for Smith-Waterman and Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [char *] first sequence
** @param [r] b [char *] second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [int *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** 
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** @return [void]
******************************************************************************/
void embAlignPathCalc(char *a, char *b, int lena, int lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     int *compass, AjBool show)
{
    int xpos;
    int i;
    int j;

    float match;
    float mscore;
    float fnew;
    float *maxa,*maxb;
    float *oval;
    int   *cnt;
    
    static AjPStr outstr = NULL;
    float bx;
    int   bv;
    
    ajDebug("embAlignPathCalc\n");

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(float));
    maxb = AJALLOC(lenb*sizeof(float));
    oval = AJALLOC(lena*sizeof(float));
    cnt  = AJALLOC(lena*sizeof(int));
    

    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*lenb] = sub[ajSeqCvtK(cvt,a[i])][ajSeqCvtK(cvt,b[0])];
	compass[i*lenb] = 0;
    }

    for(i=0;i<lena;++i)
    {
      maxa[i] = oval[i] = path[i*lenb]-(gapopen);
      cnt[i] = 0;
    }
    

    for(j=0;j<lenb;++j)
    {
	path[j] = sub[ajSeqCvtK(cvt,a[0])][ajSeqCvtK(cvt,b[j])];
	compass[j] = 0;
    }

   for(j=0;j<lenb;++j)
      maxb[j] = path[j]-(gapopen);

    /* xpos and ypos are our diagonal steps so start at 1 */
    xpos=1;
    while(xpos!=lenb)
    {
      i=1;
      bx = maxb[xpos-1];
      bv = 0;
      
      while(i<lena)
	{
	  /* get match for current xpos/ypos */
	  match = sub[ajSeqCvtK(cvt,a[i])][ajSeqCvtK(cvt,b[xpos])];
	  
	  /* Get diag score */
	  mscore = path[(i-1)*lenb+xpos-1] + match;
	  
	  /*	  ajDebug("Opt %d %6.2f ",i*lenb+xpos,mscore);*/

	  /* Set compass to diagonal value 0 */
	  compass[i*lenb+xpos] = 0;
	  path[i*lenb+xpos] = mscore;

	  
	  /* Now parade back along X axis */
	  if(xpos-2>-1)
	    {
	      fnew=path[(i-1)*lenb+xpos-2];
	      fnew-=gapopen;
	      if(maxa[i-1] < fnew)
	      {
		oval[i-1] = maxa[i-1] = fnew;
		cnt[i-1] = 0;
	      }
	      ++cnt[i-1];
	      
	      if( maxa[i-1]+match > mscore){
		mscore = maxa[i-1]+match;
		path[i*lenb+xpos] = mscore;
		compass[i*lenb+xpos] = 1; /* Score comes from left */
	      }
	      
	    }

	    /* And then bimble down Y axis */
	  if(i-2>-1)
	    {
	      fnew = path[(i-2)*lenb+xpos-1];
	      fnew-=gapopen;
	      if(fnew>maxb[xpos-1])
	      {
		 maxb[xpos-1]=bx=fnew;
		 bv=0;
	      }
	      ++bv;
	      
	      if(maxb[xpos-1]+match > mscore){
		mscore = maxb[xpos-1]+match;
		path[i*lenb+xpos] = mscore;
		compass[i*lenb+xpos] = 2; /* Score comes from bottom */
	      }
	    }


	  maxa[i-1]= oval[i-1] - ((float)cnt[i-1]*gapextend);
	  maxb[xpos-1]= bx - ((float)bv*gapextend);
	  i++;
	}
      ++xpos;
      
    }
    
    if(show)
      {
	for(i=lena-1;i>-1;--i)
	{
	    ajStrDelReuse(&outstr);
	    for(j=0;j<lenb;++j)
		(void) ajFmtPrintAppS(&outstr, "%6.2f ",path[i*lenb+j]);
	    (void) ajUser("%S", outstr);
	}
    }
    AJFREE(maxa);
    AJFREE(maxb);
    AJFREE(oval);
    AJFREE(cnt);
    
    ajStrDelReuse(&outstr);

    return;
}


/* @funcstatic alignPathCalcOld ********************************************
**
** Create path matrix for Smith-Waterman and Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [char *] first sequence
** @param [r] b [char *] second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [int *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** 
** @return [void]
******************************************************************************/

static void alignPathCalcOld(char *a, char *b, int lena, int lenb,
			       float gapopen, float gapextend, float *path,
			       float **sub, AjPSeqCvt cvt,
			       int *compass, AjBool show)
{
    int xpos;
    int ypos;
    int i;
    int j;

    int im;
    int jm;
    
    float match;
    float mscore;
    float tsc;
    float pen;
    static AjPStr outstr = NULL;
    
    ajDebug("alignPathCalcOld\n");

    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*lenb] = sub[ajSeqCvtK(cvt,a[i])][ajSeqCvtK(cvt,b[0])];
	compass[i*lenb] = 0;
    }
    for(j=0;j<lenb;++j)
    {
	path[j] = sub[ajSeqCvtK(cvt,a[0])][ajSeqCvtK(cvt,b[j])];
	compass[j] = 0;
    }


    /* xpos and ypos are our diagonal steps so start at 1 */
    xpos=ypos=1;
    while(xpos!=lenb && ypos!=lena)
    {
	for(i=ypos;i<lena;++i)
	{
	    /* get match for current xpos/ypos */
	    match = sub[ajSeqCvtK(cvt,a[i])][ajSeqCvtK(cvt,b[xpos])];

	    /* Get diag score */
	    mscore = path[(i-1)*lenb+xpos-1] + match;

	    /* Set compass to diagonal value 0 */
	    compass[i*lenb+xpos] = 0;
	    path[i*lenb+xpos] = mscore;
	
	    /* Now parade back along X axis */
	    if(xpos-2>-1)
	    {
	    
		for(jm=xpos-2;jm>-1;--jm)
		{
		    tsc = path[(i-1)*lenb+jm];
		    pen = (float) -1.0 * (gapopen + ((xpos-jm-2)*gapextend));
		    tsc += pen + match;
		    if(tsc>mscore)
		    {
			mscore=tsc;
			path[i*lenb+xpos] = tsc;
			compass[i*lenb+xpos] = 1; /* Score comes from left */
		    }
		}
	    }

	    /* And then bimble down Y axis */
	    if(i-2>-1)
	    {
		for(im=i-2;im>-1;--im)
		{
		    tsc = path[im*lenb+xpos-1];
		    pen = (float) -1.0 * (gapopen + ((i-im-2)*gapextend));
		    tsc += pen + match;
		    if(tsc>mscore)
		    {
			mscore=tsc;
			path[i*lenb+xpos] = tsc;
			compass[i*lenb+xpos] = 2; /* Score comes from bottom */
		    }
		}
	    }
	}

	/* move along */
	if(xpos+1 != lenb)
	{
	    for(j=xpos+1;j<lenb;++j)
	    {
		match=sub[ajSeqCvtK(cvt,a[ypos])][ajSeqCvtK(cvt,b[j])];
		mscore = path[(ypos-1)*lenb+j-1] + match;
		compass[ypos*lenb+j]=0;
		path[ypos*lenb+j]=mscore;

		/* parade once again back X */
		if(j-2>-1)
		{
		    for(jm=j-2;jm>-1;--jm)
		    {
			tsc = path[(ypos-1)*lenb+jm];
			pen = (float) -1.0 * (gapopen + ((j-jm-2)*gapextend));
			tsc += pen+match;
			if(tsc>mscore)
			{
			    mscore = tsc;
			    path[ypos*lenb+j] = tsc; /* Came from left */
			    compass[ypos*lenb+j]=1;
			}
		    }
		}

		/* Re-bimble down Y */
		if(ypos-2>-1)
		{
		    for(im=ypos-2;im>-1;--im)
		    {
			tsc = path[im*lenb+j-1];
			pen = (float) -1.0 * (gapopen+((ypos-im-2)*gapextend));
			tsc += pen+match;
			if(tsc>mscore)
			{
			    mscore=tsc;
			    path[ypos*lenb+j]=tsc;
			    compass[ypos*lenb+j]=2;	/* from bottom */
			}
		    }
		}
	    }
	}

	++xpos;
	++ypos;
    
    }

    if(show)
    {
	for(i=lena-1;i>-1;--i)
	{
	    ajStrDelReuse(&outstr);
	    for(j=0;j<lenb;++j)
		(void) ajFmtPrintAppS(&outstr,"%6.2f ",path[i*lenb+j]);
	    (void) ajUser("%S", outstr);
	}
    }
    
    ajStrDelReuse (&outstr);
    return;
}





/* @func embAlignProfilePathCalc  ******************************************
**
** Create path matrix for a profile
** Nucleotides or proteins as needed.
**
** @param [r] a [char *] sequence
** @param [r] mlen [int] length of profile
** @param [r] slen [int] length of sequence
** @param [r] gapopen [float] gap opening coefficient
** @param [r] gapextend [float] gap extension coefficient
** @param [w] path [float *] path matrix
** @param [r] fmatrix [float **] profile matrix
** @param [w] compass [int *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** 
** @return [void]
******************************************************************************/

void embAlignProfilePathCalc(char *a, int mlen, int slen, float gapopen,
			    float gapextend, float *path, float **fmatrix,
			    int *compass, AjBool show)
{
    int xpos;
    int ypos;
    int i;
    int j;

    int im;
    int jm;
    
    float match;
    float mscore;
    float tsc;
    float pen;
    static AjPStr outstr = NULL;
    
    ajDebug("embAlignProfilePathCalc\n");

    /* First initialise the first column and row */
    for(i=0;i<slen;++i)
    {
	path[i] = fmatrix[0][ajAZToInt(a[i])];
	compass[i] = 0;
    }
    for(j=0;j<mlen;++j)
    {
	path[j*slen] = fmatrix[j][ajAZToInt(*a)];
	compass[j*slen] = 0;
    }


    /* xpos and ypos are our diagonal steps so start at 1 */
    xpos=ypos=1;
    while(xpos!=slen && ypos!=mlen)
    {
	for(i=ypos;i<mlen;++i)
	{
	    /* get match for current xpos/ypos */
	    match = fmatrix[i][ajAZToInt(a[xpos])];

	    /* Get diag score */
	    mscore = path[(i-1)*slen+xpos-1] + match;

	    /* Set compass to diagonal value 0 */
	    compass[i*slen+xpos] = 0;
	    path[i*slen+xpos] = mscore;
	
	    /* Now parade back along X axis */
	    if(xpos-2>-1)
	    {
	    
		for(jm=xpos-2;jm>-1;--jm)
		{
		    tsc = path[(i-1)*slen+jm];
		    pen = (float) -1.0 * (fmatrix[i-1][PAZ] * gapopen +
			    ((xpos-jm-2) *gapextend*fmatrix[i-1][PAZ1]));
		    
		    tsc += pen + match;
		    if(tsc>mscore)
		    {
			mscore=tsc;
			path[i*slen+xpos] = tsc;
			compass[i*slen+xpos] = 1; /* Score comes from left */
		    }
		}
	    }

	    /* And then bimble down Y axis */
	    if(i-2>-1)
	    {
		for(im=i-2;im>-1;--im)
		{
		    tsc = path[im*slen+xpos-1];
		    pen = (float) -1.0 * (fmatrix[im][PAZ] * gapopen+((i-im-2)
				 *gapextend*fmatrix[im][PAZ1]));
		    tsc += pen + match;
		    if(tsc>mscore)
		    {
			mscore=tsc;
			path[i*slen+xpos] = tsc;
			compass[i*slen+xpos] = 2; /* Score comes from bottom */
		    }
		}
	    }
	}

	/* move along */
	if(xpos+1 != slen)
	{
	    for(j=xpos+1;j<slen;++j)
	    {
		match=fmatrix[ypos][ajAZToInt(a[j])];
		mscore = path[(ypos-1)*slen+j-1] + match;
		compass[ypos*slen+j]=0;
		path[ypos*slen+j]=mscore;

		/* parade once again back X */
		if(j-2>-1)
		{
		    for(jm=j-2;jm>-1;--jm)
		    {
			tsc = path[(ypos-1)*slen+jm];
			pen = (float) -1.0 * (fmatrix[ypos-1][PAZ] * gapopen +
				      ((j-jm-2)
				 *gapextend*fmatrix[ypos-1][PAZ1]));
			tsc += pen+match;
			if(tsc>mscore)
			{
			    mscore = tsc;
			    path[ypos*slen+j] = tsc; /* Came from left */
			    compass[ypos*slen+j]=1;
			}
		    }
		}

		/* Re-bimble down Y */
		if(ypos-2>-1)
		{
		    for(im=ypos-2;im>-1;--im)
		    {
			tsc = path[im*slen+j-1];
			pen = (float) -1.0 * (fmatrix[im][PAZ] * gapopen +
				      ((ypos-im-2)
				 *gapextend*fmatrix[im][PAZ1]));

			tsc += pen+match;
			if(tsc>mscore)
			{
			    mscore=tsc;
			    path[ypos*slen+j]=tsc;
			    compass[ypos*slen+j]=2;	/* from bottom */
			}
		    }
		}
	    }
	}

	++xpos;
	++ypos;
    
    }

    if(show)
    {
	for(i=mlen-1;i>-1;--i)
	{
	    ajStrDelReuse(&outstr);
	    for(j=0;j<slen;++j)
		(void) ajFmtPrintAppS(&outstr,"%6.2f ",path[i*slen+j]);
	    (void) ajUser("%S", outstr);
	}
    }
    

    ajStrDelReuse (&outstr);
    return;
}


/* @func embAlignScoreNWMatrix *************************************************
**
** Score a  matrix for Needleman Wunsch.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [r] fmatrix [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] gapopen [float] gap opening coefficient
** @param [r] compass [int*] Path direction pointer array
** @param [r] gapextend [float] gap extension coefficient
** @param [w] start1 [int *] start of alignment in first sequence
** @param [w] start2 [int *] start of alignment in second sequence
** 
** @return [float] Maximum path axis score
** @@
******************************************************************************/

float embAlignScoreNWMatrix(float *path, AjPSeq a, AjPSeq b, float **fmatrix,
			   AjPSeqCvt cvt, int lena, int lenb, float gapopen,
			   int *compass,
			   float gapextend, int *start1, int *start2)
{
    int i;
    int j;

    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    float wscore;
    float errbounds = gapextend;
    
    int ix;
    int iy;
    int t;
    
    int xpos=0;
    int ypos=0;
    char *p;
    char *q;

    ajDebug ("embAlignScoreNWMatrix\n");

    /* Get maximum path axis score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lenb;++i)
      if(path[(lena-1)*lenb+i]>=pmax)
	{
	  pmax = path[(lena-1)*lenb+i];
	  xpos = i;
	  ypos = lena-1;
	}
    for(j=0;j<lena;++j)
      if(path[j*lenb+lenb-1]>pmax)
	{
	  pmax=path[j*lenb+lenb-1];
	  xpos=lenb-1;
	  ypos=j;
	}
    
    p = ajSeqChar(a);
    q = ajSeqChar(b);
    
    wscore = fmatrix[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
    /*    ajUser("Match %c %c %f",p[ypos],q[xpos],wscore);*/

    while(xpos && ypos)
    {

	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	  wscore += fmatrix[ajSeqCvtK(cvt,p[--ypos])][ajSeqCvtK(cvt,q[--xpos])];
	  /*ajUser("Match %c %c %f",p[ypos],q[xpos],wscore);*/
	}
	else if(compass[ypos*lenb+xpos]==1)	/* Left, gap(s) in vertical */
	  {
	    score = path[ypos*lenb+xpos];
	    gapcnt=0.;
	    ix=xpos-2;
	    match = fmatrix[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	      {
		bimble=path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)< errbounds) break;
		--ix;
		if(ix<0){


		  gapcnt=0.0;
		  ix=xpos-2;
		  t=ix+1;
		  while(ix)
		    {
		      
		      bimble=path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		      if(fabs((double)score-(double)bimble)< /*gapopen*/errbounds)
			ajDebug("NW:%f: %f %f %f\n",gapcnt,score,bimble);
		      --ix;
		      ++gapcnt;
		    }
		  


		  ajFatal("NW: Error walking left");
		}
		++gapcnt;
	      }
	    /*	    if(score<0.0) break;*/
	    t -= (int)gapcnt;
	    
	    wscore += fmatrix[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[t-1])];
	    wscore -= (gapopen + (gapextend*gapcnt));
	    /*ajUser("LEFT GAP %c %c number=%f score = %f",p[ypos],q[t-1],gapcnt,wscore);*/

	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*lenb+xpos];
	    gapcnt=0.;
	    match = fmatrix[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)< errbounds) break;
		--iy;
		if(iy<0) {
		  /*ajDebug("NW: Error walking down %d < 0 gapcnt: %d\n",
		    iy, gapcnt);*/
		    ajFatal("NW: Error walking down");
		}
		++gapcnt;
	    }
	    /*	    if(score<0.0) break;*/
	    t -= (int)gapcnt;

	    /*	    ajUser("DOWN GAP %c %c number=%f score = %f",p[t-1],q[xpos],gapcnt,wscore);*/
	    wscore += fmatrix[ajSeqCvtK(cvt,p[t-1])][ajSeqCvtK(cvt,q[xpos])];
	    /*ajUser("DOWN GAP %c %c number=%f score = %f",p[t-1],q[xpos],gapcnt,wscore);*/
	    wscore -= (gapopen + (gapextend*gapcnt));
	    /*ajUser("DOWN GAP %c %c number=%f score = %f",p[t-1],q[xpos],gapcnt,wscore);*/

	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in NW");
    }	    
    

    
    
    *start1 = ypos;
    *start2 = xpos;
    
    return wscore;
}



/* @func embAlignScoreProfileMatrix *******************************************
**
** Score a profile path matrix for Smith waterman.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [int *] Path direction pointer array
** @param [r] gapopen [float] gap opening coeff
** @param [r] gapextend [float] gap extension coeff
** @param [r] b [AjPStr] second sequence
** @param [r] clen [int] length of consensus sequence
** @param [r] slen [int] length of test sequence
** @param [r] fmatrix [float **] profile
** @param [w] start1 [int *] start of alignment in consensus sequence
** @param [w] start2 [int *] start of alignment in test sequence
** 
** @return [float] profile alignment score
******************************************************************************/

float embAlignScoreProfileMatrix(float *path, int *compass, float gapopen,
				float gapextend, AjPStr b,
				int clen, int slen, float **fmatrix,
				int *start1, int *start2)
{
    int i;
    int j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    int ix=0;
    int iy;
    int t;
    
    int xpos=0;
    int ypos=0;

    char *q;

    float wscore;
    float errbounds = gapextend;

    ajDebug ("embAlignScoreProfileMatrix\n");

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<clen;++i)
	for(j=0;j<slen;++j)
	    if(path[i*slen+j]>pmax)
	    {
		pmax=path[i*slen+j];
		xpos=j;
		ypos=i;
	    }
    
    q = ajStrStr(b);

    wscore = fmatrix[ypos][ajAZToInt(q[xpos])];
    
    while(xpos && ypos)
    {
	if(!compass[ypos*slen+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*slen+xpos-1]<0.) break;
	    wscore = fmatrix[--ypos][ajAZToInt(q[--xpos])];
	    continue;
	}
	else if(compass[ypos*slen+xpos]==1)	/* Left, gap(s) in vertical */
	{
	    score = path[ypos*slen+xpos];
	    gapcnt=0.;
	    ix=xpos-2;
	    match = fmatrix[ypos][ajAZToInt(q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	    {
		bimble=path[ypos*slen+ix]-(gapopen*fmatrix[ypos][PAZ])-
		    (gapcnt*fmatrix[ypos][PAZ1]*gapextend)+match;
		if(fabs((double)score-(double)bimble)<errbounds) break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
	    }
	    if(score<0.0) break;
	    t -= (int)gapcnt;

	    wscore += fmatrix[ypos][ajAZToInt(q[t])];
	    wscore -= (gapopen*fmatrix[ypos][PAZ] +
		       (gapextend*gapcnt*fmatrix[ypos][PAZ1]));

	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*slen+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*slen+xpos];
	    gapcnt=0.;
	    match = fmatrix[ypos][ajAZToInt(q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    while(1)
	    {
		bimble=path[iy*slen+xpos]-(gapopen*fmatrix[iy][PAZ])-
		    (gapcnt*fmatrix[iy][PAZ1]*gapextend)+match;
		if(fabs((double)score-(double)bimble)<errbounds) break;
		--iy;
		if(iy<0) {
		  /*ajDebug("SW: Error walking down %d < 0 gapcnt: %d\n",
		    iy, gapcnt);*/
		    ajFatal("SW: Error walking down");
		}
		++gapcnt;
	    }
	    if(score<0.0) break;
	    t -= (int)gapcnt;

	    wscore += fmatrix[iy][ajAZToInt(q[t])];
	    wscore -= (gapopen*fmatrix[iy][PAZ] +
		       (gapextend*gapcnt*fmatrix[iy][PAZ1]));

	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 = ypos;
    *start2 = xpos;
    
    return wscore;
}



/* @func embAlignScoreSWMatrix ************************************************
**
** Walk down a matrix for Smith waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [int *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [int *] start of alignment in first sequence
** @param [w] start2 [int *] start of alignment in second sequence
** 
** @return [float] Score of best matching segment
******************************************************************************/

float embAlignScoreSWMatrix(float *path, int *compass, float gapopen,
			  float gapextend,  AjPSeq a, AjPSeq b,
			  int lena, int lenb, float **sub,
			  AjPSeqCvt cvt, int *start1, int *start2)
{
    int i;
    int j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    float wscore;
    
    int ix;
    int iy;
    int t;
    
    int xpos=0;
    int ypos=0;
    char *p;
    char *q;
    float errbounds=gapextend;

    ajDebug ("embAlignScoreSWMatrix\n");

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<lenb;++j)
	    if(path[i*lenb+j]>pmax)
	    {
		pmax=path[i*lenb+j];
		xpos=j;
		ypos=i;
	    }
    
    p = ajSeqChar(a);
    q = ajSeqChar(b);

    wscore = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
    
    while(xpos && ypos)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*lenb+xpos-1]<0.) break;
	    /*	    ajUser(" DIAG %c %c",p[ypos],q[xpos]);*/
	    wscore += sub[ajSeqCvtK(cvt,p[--ypos])][ajSeqCvtK(cvt,q[--xpos])];
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==1)	/* Left, gap(s) in vertical */
	{
	    score = path[ypos*lenb+xpos];
	    gapcnt=0.;
	    ix=xpos-2;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	    {
		bimble=path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<errbounds) break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
	    }
	    if(score<0.0) break;
	    t -= (int)gapcnt;

	    wscore += sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[t-1])];
	    /*	    ajUser(" LEFT %c %c",p[ypos],q[t-1]);*/
	    wscore -= (gapopen + (gapextend*gapcnt));

	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*lenb+xpos];
	    gapcnt=0.;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    /*ajUser("match = %f, gapcnt= %f, score = %f, iy = %d",
	      match,gapcnt,score,iy);*/
	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		/*ajUser("score = %f bimble = %f fabs = %f",
		  score,bimble,fabs((double)score-(double)bimble));*/
		if( fabs((double)score-(double)bimble) < errbounds) break;
		/*ajUser("iy = %d",iy);*/
		--iy;
		/*ajUser("iy = %d",iy);*/
		if(iy<0){
		  /*ajDebug("SW: Error walking down %d < 0 gapcnt: %d\n",
		    iy, gapcnt);*/
		  ajFatal("SW: Error walking down");
		}
		++gapcnt;
	    }
	    if(score<0.0) break;
	    t -= (int)gapcnt;

	    wscore += sub[ajSeqCvtK(cvt,p[t-1])][ajSeqCvtK(cvt,q[xpos])];
	    wscore -= (gapopen + (gapextend*gapcnt));
	    /*	    ajUser(" DOWN %c %c",p[t-1],q[xpos]);*/

	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 = ypos;
    *start2 = xpos;
    
    return wscore;
}


/* @func embAlignWalkSWMatrix *************************************************
**
** Walk down a matrix for Smith waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [int *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [int *] start of alignment in first sequence
** @param [w] start2 [int *] start of alignment in second sequence
** 
** @return [void]
******************************************************************************/

void embAlignWalkSWMatrix(float *path, int *compass, float gapopen,
			 float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, int lena, int lenb, float **sub,
			 AjPSeqCvt cvt, int *start1, int *start2)
{
    int i;
    int j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    int ix;
    int iy;
    int t;
    
    int xpos=0;
    int ypos=0;
    char *p;
    char *q;
    char r[2]="?";
    char dot[2]=".";

    float ic;
    float errbounds = gapextend;
    
    ajDebug ("embAlignWalkSWMatrix\n");

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<lenb;++j)
	    if(path[i*lenb+j]>pmax)
	    {
		pmax=path[i*lenb+j];
		xpos=j;
		ypos=i;
	    }
    
    p = ajSeqChar(a);
    q = ajSeqChar(b);

    *r = p[ypos];
    (void) ajStrAssC(m,r);
    *r = q[xpos];
    (void) ajStrAssC(n,r);

    
    while(xpos && ypos)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*lenb+xpos-1]<0.) break;
	    *r=p[--ypos];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[--xpos];
	    (void) ajStrInsertC(n,0,r);
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==1)	/* Left, gap(s) in vertical */
	{
	    score = path[ypos*lenb+xpos];
	    gapcnt=0.;
	    ix=xpos-2;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	    {
		bimble=path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<errbounds) break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
	    }
	    if(score<0.0) break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrInsertC(m,0,dot);
		*r=q[t--];
		(void) ajStrInsertC(n,0,r);
	    }
	    *r=q[t];
	    (void) ajStrInsertC(n,0,r);
	    *r=p[ypos];
	    (void) ajStrInsertC(m,0,r);

	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*lenb+xpos];
	    gapcnt=0.;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<errbounds) break;
		--iy;
		if(iy<0)
		    ajFatal("SW: Error walking down");
		++gapcnt;
	    }
	    if(score<0.0) break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrInsertC(n,0,dot);
		*r=p[t--];
		(void) ajStrInsertC(m,0,r);
	    }
	    *r=p[t];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[xpos];
	    (void) ajStrInsertC(n,0,r);
	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 = ypos;
    *start2 = xpos;
    
    return;
}

/* @func embAlignWalkNWMatrix *************************************************
**
** Walk down a matrix for Needleman Wunsch. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [w] start1 [int *] start of alignment in first sequence
** @param [w] start2 [int *] start of alignment in second sequence
** @param [r] gapopen [float] gap open penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [r] compass [int *] Path direction pointer array
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** 
** @return [void]
******************************************************************************/

void embAlignWalkNWMatrix(float *path, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, int lena, int lenb, int *start1,
			 int *start2, float gapopen, 
			 float gapextend, AjPSeqCvt cvt, int *compass,
			 float **sub)
{
    int i;
    int j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    int ix;
    int iy;
    int t;
    
    int xpos=0;
    int ypos=0;
    char *p;
    char *q;
    char r[2]="?";
    char dot[2]=".";

    float ic;
    float errbounds=gapextend;

    ajDebug("embAlignWalkNWMatrix\n");

    /* Get maximum path axis score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lenb;++i)
	if(path[(lena-1)*lenb+i]>=pmax)
	{
	    pmax = path[(lena-1)*lenb+i];
	    xpos = i;
	    ypos = lena-1;
	}
    for(j=0;j<lena;++j)
	if(path[j*lenb+lenb-1]>pmax)
	{
	    pmax=path[j*lenb+lenb-1];
	    xpos=lenb-1;
	    ypos=j;
	}
    
    p = ajSeqChar(a);
    q = ajSeqChar(b);


    *r=p[ypos];
    (void) ajStrInsertC(m,0,r);
    *r=q[xpos];
    (void) ajStrInsertC(n,0,r);
    while(xpos && ypos)
    {
	if(!compass[ypos*lenb+xpos])	/* diagonal */
	{
	  /*	    if(path[(ypos-1)*lenb+xpos-1]<0.) break;*/
	    *r=p[--ypos];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[--xpos];
	    (void) ajStrInsertC(n,0,r);
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==1)	/* Left, gap(s) in vertical */
	{
	    score = path[ypos*lenb+xpos];
	    gapcnt=0.;
	    ix=xpos-2;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	    {
		bimble=path[ypos*lenb+ix]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)< errbounds) break;
		--ix;
		if(ix<0)
		    ajFatal("NW: Error walking left");
		++gapcnt;
	    }
	    /*	    if(score<0.0) break;*/
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrInsertC(m,0,dot);
		*r=q[t--];
		(void) ajStrInsertC(n,0,r);
	    }
	    *r=q[t];
	    (void) ajStrInsertC(n,0,r);
	    *r=p[ypos];
	    (void) ajStrInsertC(m,0,r);

	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*lenb+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*lenb+xpos];
	    gapcnt=0.;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    while(1)
	    {
		bimble=path[iy*lenb+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)< errbounds) break;
		--iy;
		if(iy<0)
		    ajFatal("NW: Error walking down");
		++gapcnt;
	    }
	    /*	    if(score<0.0) break;*/
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrInsertC(n,0,dot);
		*r=p[t--];
		(void) ajStrInsertC(m,0,r);
	    }
	    *r=p[t];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[xpos];
	    (void) ajStrInsertC(n,0,r);
	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in NW");

    }

    *start1 = ypos;
    *start2 = xpos;
    
    return;
}



/* @func embAlignWalkProfileMatrix *********************************************
**
** Walk down a profile path matrix for Smith waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [int *] Path direction pointer array
** @param [r] gapopen [float] gap opening coeff
** @param [r] gapextend [float] gap extension coeff
** @param [r] cons [AjPStr] consensus sequence
** @param [r] b [AjPStr] second sequence
** @param [w] m [AjPStr *] alignment for consensus sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] clen [int] length of consensus sequence
** @param [r] slen [int] length of test sequence
** @param [r] fmatrix [float **] profile
** @param [w] start1 [int *] start of alignment in consensus sequence
** @param [w] start2 [int *] start of alignment in test sequence
** 
** @return [void]
******************************************************************************/

void embAlignWalkProfileMatrix(float *path, int *compass, float gapopen,
			 float gapextend, AjPStr cons, AjPStr b, AjPStr *m,
			 AjPStr *n, int clen, int slen, float **fmatrix,
			 int *start1, int *start2)
{
    int i;
    int j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    int ix=0;
    int iy;
    int t;
    
    int xpos=0;
    int ypos=0;
    char *p;
    char *q;
    char r[2]="?";
    char dot[2]=".";

    float ic;
    float errbounds=gapextend;
    
    ajDebug("embAlignWalkProfileMatrix\n");

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<clen;++i)
	for(j=0;j<slen;++j)
	    if(path[i*slen+j]>pmax)
	    {
		pmax=path[i*slen+j];
		xpos=j;
		ypos=i;
	    }
    
    p = ajStrStr(cons);
    q = ajStrStr(b);

    *r = p[ypos];
    (void) ajStrAssC(m,r);
    *r = q[xpos];
    (void) ajStrAssC(n,r);

    
    while(xpos && ypos)
    {
	if(!compass[ypos*slen+xpos])	/* diagonal */
	{
	    if(path[(ypos-1)*slen+xpos-1]<0.) break;
	    *r=p[--ypos];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[--xpos];
	    (void) ajStrInsertC(n,0,r);
	    continue;
	}
	else if(compass[ypos*slen+xpos]==1)	/* Left, gap(s) in vertical */
	{
	    score = path[ypos*slen+xpos];
	    gapcnt=0.;
	    ix=xpos-2;
	    match = fmatrix[ypos][ajAZToInt(q[xpos])];
	    --ypos;
	    t=ix+1;
	    while(1)
	    {
		bimble=path[ypos*slen+ix]-(gapopen*fmatrix[ypos][PAZ])-
		    (gapcnt*fmatrix[ypos][PAZ1]*gapextend)+match;
		if(fabs((double)score-(double)bimble)< errbounds) break;
		--ix;
		if(ix<0)
		    ajFatal("SW: Error walking left");
		++gapcnt;
	    }
	    if(score<0.0) break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrInsertC(m,0,dot);
		*r=q[t--];
		(void) ajStrInsertC(n,0,r);
	    }
	    *r=q[t];
	    (void) ajStrInsertC(n,0,r);
	    *r=p[ypos];
	    (void) ajStrInsertC(m,0,r);

	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*slen+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*slen+xpos];
	    gapcnt=0.;
	    match = fmatrix[ypos][ajAZToInt(q[xpos])];
	    --xpos;
	    iy=ypos-2;
	    t=iy+1;

	    while(1)
	    {
		bimble=path[iy*slen+xpos]-(gapopen*fmatrix[iy][PAZ])-
		    (gapcnt*fmatrix[iy][PAZ1]*gapextend)+match;
		if(fabs((double)score-(double)bimble)<errbounds) break;
		--iy;
		if(iy<0)
		    ajFatal("SW: Error walking down");
		++gapcnt;
	    }
	    if(score<0.0) break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrInsertC(n,0,dot);
		*r=p[t--];
		(void) ajStrInsertC(m,0,r);
	    }
	    *r=p[t];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[xpos];
	    (void) ajStrInsertC(n,0,r);
	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
    }

    *start1 = ypos;
    *start2 = xpos;
    
    return;
}



/* @func embAlignPrintGlobal *******************************************
**
** Print a global alignment
** Nucleotides or proteins as needed.
**
** @param [w] outf [AjPFile] output stream
** @param [r] a [char *] complete first sequence
** @param [r] b [char *] complete second sequence
** @param [r] m [AjPStr] Walk alignment for first sequence
** @param [r] n [AjPStr] Walk alignment for second sequence
** @param [r] start1 [int] start of alignment in first sequence
** @param [r] start2 [int] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float **] substitution matrix
** @param [r] cvt [AjPSeqCvt] conversion table for matrix
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [int] first sequence offset
** @param [r] beginb [int] second sequence offset
** 
** @return [void]
******************************************************************************/

void embAlignPrintGlobal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			int start1, int start2, float score, AjBool mark,
			float **sub, AjPSeqCvt cvt, char *namea,
			char *nameb, int begina, int beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;
    
    int i;
    int nc;
    int olen;
    char *p;
    char *q;
    char *r=NULL;
    
    float match=0.0;

    int apos;
    int bpos;
    int alen;
    int blen;
    int acnt;
    int bcnt;
    int aend;
    int bend;
    
    int len;
    int pos;
    
    fa = ajStrNewC("");
    fb = ajStrNewC("");
    fm = ajStrNewC("");
    ap = ajStrNewC("");
    bp = ajStrNewC("");
    mp = ajStrNewC("");
    

    if(start1>start2)
    {
	for(i=0;i<start1;++i)
	{
	    (void) ajStrAppK(&fa,a[i]);
	    if(mark) (void) ajStrAppC(&fm," ");
	}
	nc=start1-start2;
	for(i=0;i<nc;++i) (void) ajStrAppC(&fb," ");
	for(++nc;i<start1;++i) (void) ajStrAppK(&fb,b[i-nc]);
    }
    else if(start2>start1)
    {
	for(i=0;i<start2;++i)
	{
	    (void) ajStrAppK(&fb,b[i]);
	    if(mark) (void) ajStrAppC(&fm," ");
	}
	nc=start2-start1;
	for(i=0;i<nc;++i) (void) ajStrAppC(&fa," ");
	for(++nc;i<start2;++i) (void) ajStrAppK(&fa,a[i-nc]);
    }

    len=ajStrLen(fa);

    /* Now deal with the alignment overlap */
    p=ajStrStr(m);
    q=ajStrStr(n);
    olen=strlen(p);
    for(i=0;i<olen;++i)
    {
	(void) ajStrAppK(&fa,p[i]);
	(void) ajStrAppK(&fb,q[i]);
	if(mark)
	{
	    if(p[i]=='.' || q[i]=='.')
	    {
		(void) ajStrAppC(&fm," ");
		continue;
	    }
	    match=sub[ajSeqCvtK(cvt,p[i])][ajSeqCvtK(cvt,q[i])];
	    if(p[i]==q[i])
	    {
		(void) ajStrAppC(&fm,"|");
		continue;
	    }
	    if(match>0.0)
		(void) ajStrAppC(&fm,":");
	    else
		(void) ajStrAppC(&fm," ");
	}
    }
    /* Set pointers to sequence remainders */
    for(i=0,apos=start1,bpos=start2;i<olen;++i)
    {
	if(p[i]!='.') ++apos;
	if(q[i]!='.') ++bpos;
    }


    alen=strlen(&a[apos]);
    blen=strlen(&b[bpos]);

    if(alen>blen)
    {
	(void) ajStrAppC(&fa,&a[apos]);
	for(i=0;i<blen;++i)
	{
	    (void) ajStrAppK(&fb,b[bpos+i]);
	    if(mark) (void) ajStrAppC(&fm," ");
	}
	nc=alen-blen;
	for(i=0;i<nc;++i)
	{
	    (void) ajStrAppC(&fb," ");
	    if(mark) (void) ajStrAppC(&fm," ");
	}
    }
    else if(blen>alen)
    {
	(void) ajStrAppC(&fb,&b[bpos]);
	for(i=0;i<alen;++i)
	{
	    (void) ajStrAppK(&fa,a[apos+i]);
	    if(mark) (void) ajStrAppC(&fm," ");
	}
	nc=blen-alen;
	for(i=0;i<nc;++i)
	{
	    (void) ajStrAppC(&fa," ");
	    if(mark) (void) ajStrAppC(&fm," ");
	}
    }
    else
    {
	(void) ajStrAppC(&fa,&a[apos]);
	(void) ajStrAppC(&fb,&b[bpos]);
	if(mark)
	    for(i=0;i<alen;++i)
		(void) ajStrAppC(&fm," ");
    }
    
    /* Get start residues */
    p=ajStrStr(fa);
    q=ajStrStr(fb);
    for(i=0,acnt=start1,bcnt=start2;i<len;++i)
    {
	if(p[i]!=' ') --acnt;
	if(q[i]!=' ') --bcnt;
    }
    acnt+=begina;
    bcnt+=beginb;
    
    len=ajStrLen(fa);
    pos=0;
    if(mark) r=ajStrStr(fm);


    /* Add header stuff here */
    ajFmtPrintF(outf,"Global: %s vs %s\n",namea,nameb);
    ajFmtPrintF(outf,"Score: %.2f\n\n",score);
    
    while(pos<len)
    {
	if(pos+45 < len)
	{
	    (void) ajStrAssSubC(&ap,p,pos,pos+45-1);
	    (void) ajStrAssSubC(&bp,q,pos,pos+45-1);
	    if(mark)
		(void) ajStrAssSubC(&mp,r,pos,pos+45-1);
	    for(i=0,aend=acnt,bend=bcnt;i<45;++i)
	    {
		if(p[pos+i]!=' ' && p[pos+i]!='.') ++aend;
		if(q[pos+i]!=' ' && q[pos+i]!='.') ++bend;
	    }


	    ajFmtPrintF(outf,"%-15.15s ",namea);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d ",acnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45s ",ajStrStr(ap));
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d\n",aend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    acnt=aend;
	    
	    if(mark)
		ajFmtPrintF(outf,"                         %s\n",ajStrStr(mp));
		
	    ajFmtPrintF(outf,"%-15.15s ",nameb);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d ",bcnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45s ",ajStrStr(bp));
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d\n",bend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    bcnt=bend;

	    ajFmtPrintF(outf,"\n");
	    pos += 45;
	    continue;
	}
	
	(void) ajStrAssC(&ap,&p[pos]);
	(void) ajStrAssC(&bp,&q[pos]);
	if(mark)
	    (void) ajStrAssC(&mp,&r[pos]);
	for(i=0,aend=acnt,bend=bcnt;i<45 && p[pos+i];++i)
	{
	    if(p[pos+i]!=' ' && p[pos+i]!='.') ++aend;
	    if(q[pos+i]!=' ' && q[pos+i]!='.') ++bend;
	}
	
	
	ajFmtPrintF(outf,"%-15.15s ",namea);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d ",acnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45s ",ajStrStr(ap));
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d\n",aend-1);
	else
	    ajFmtPrintF(outf,"\n");
	acnt=aend;
	
	if(mark)
	    ajFmtPrintF(outf,"                         %s\n",ajStrStr(mp));
	
	ajFmtPrintF(outf,"%-15.15s ",nameb);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d ",bcnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45s ",ajStrStr(bp));
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d\n",bend-1);
	else
	    ajFmtPrintF(outf,"\n");
	bcnt=bend;
	
	pos=len;
    }

    ajStrDel(&mp);
    ajStrDel(&bp);
    ajStrDel(&ap);
    ajStrDel(&fa);
    ajStrDel(&fb);
    ajStrDel(&fm);
}



/* @func embAlignPrintLocal *******************************************
**
** Print a local alignment
** Nucleotides or proteins as needed.
**
** @param [w] outf [AjPFile] output stream
** @param [r] a [char *] complete first sequence
** @param [r] b [char *] complete second sequence
** @param [r] m [AjPStr] Walk alignment for first sequence
** @param [r] n [AjPStr] Walk alignment for second sequence
** @param [r] start1 [int] start of alignment in first sequence
** @param [r] start2 [int] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float **] substitution matrix
** @param [r] cvt [AjPSeqCvt] conversion table for matrix
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [int] first sequence offset
** @param [r] beginb [int] second sequence offset
** 
** @return [void]
******************************************************************************/

void embAlignPrintLocal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			int start1, int start2, float score, AjBool mark,
			float **sub, AjPSeqCvt cvt, char *namea,
			char *nameb, int begina, int beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;
    
    int i;
    int olen;
    char *p;
    char *q;
    char *r=NULL;
    
    float match=0.0;

    int acnt;
    int bcnt;
    int aend;
    int bend;
    
    int len;
    int pos;
    
    fm = ajStrNewC("");
    ap = ajStrNewC("");
    bp = ajStrNewC("");
    mp = ajStrNewC("");
    

    /* Now deal with the alignment overlap */
    p=ajStrStr(m);
    q=ajStrStr(n);
    olen=strlen(p);
    fa=m;
    fb=n;
    if(mark)
    {
	for(i=0;i<olen;++i)
	{
	    if(p[i]=='.' || q[i]=='.')
	    {
		(void) ajStrAppC(&fm," ");
		continue;
	    }
	    match=sub[ajSeqCvtK(cvt,p[i])][ajSeqCvtK(cvt,q[i])];
	    if(p[i]==q[i])
	    {
		(void) ajStrAppC(&fm,"|");
		continue;
	    }
	    if(match>0.0)
		(void) ajStrAppC(&fm,":");
	    else
		(void) ajStrAppC(&fm," ");
	}
    }

    /* Get start residues */
    p=ajStrStr(fa);
    q=ajStrStr(fb);
    acnt=begina+start1;
    bcnt=beginb+start2;
    
    len=ajStrLen(fa);
    pos=0;
    if(mark) r=ajStrStr(fm);


    /* Add header stuff here */
    ajFmtPrintF(outf,"Local: %s vs %s\n",namea,nameb);
    ajFmtPrintF(outf,"Score: %.2f\n\n",score);
    
    while(pos<len)
    {
	if(pos+45 < len)
	{
	    (void) ajStrAssSubC(&ap,p,pos,pos+45-1);
	    (void) ajStrAssSubC(&bp,q,pos,pos+45-1);
	    if(mark)
		(void) ajStrAssSubC(&mp,r,pos,pos+45-1);
	    for(i=0,aend=acnt,bend=bcnt;i<45;++i)
	    {
		if(p[pos+i]!=' ' && p[pos+i]!='.') ++aend;
		if(q[pos+i]!=' ' && q[pos+i]!='.') ++bend;
	    }


	    ajFmtPrintF(outf,"%-15.15s ",namea);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d ",acnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45s ",ajStrStr(ap));
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d\n",aend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    acnt=aend;
	    
	    if(mark)
		ajFmtPrintF(outf,"                         %s\n",ajStrStr(mp));
		
	    ajFmtPrintF(outf,"%-15.15s ",nameb);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d ",bcnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45s ",ajStrStr(bp));
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d\n",bend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    bcnt=bend;

	    ajFmtPrintF(outf,"\n");
	    pos += 45;
	    continue;
	}
	
	(void) ajStrAssC(&ap,&p[pos]);
	(void) ajStrAssC(&bp,&q[pos]);
	if(mark)
	    (void) ajStrAssC(&mp,&r[pos]);
	for(i=0,aend=acnt,bend=bcnt;i<45 && p[pos+i];++i)
	{
	    if(p[pos+i]!=' ' && p[pos+i]!='.') ++aend;
	    if(q[pos+i]!=' ' && q[pos+i]!='.') ++bend;
	}
	
	
	ajFmtPrintF(outf,"%-15.15s ",namea);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d ",acnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45s ",ajStrStr(ap));
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d\n",aend-1);
	else
	    ajFmtPrintF(outf,"\n");
	acnt=aend;
	
	if(mark)
	    ajFmtPrintF(outf,"                         %s\n",ajStrStr(mp));
	
	ajFmtPrintF(outf,"%-15.15s ",nameb);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d ",bcnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45s ",ajStrStr(bp));
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d\n",bend-1);
	else
	    ajFmtPrintF(outf,"\n");
	bcnt=bend;
	
	pos=len;
    }

    ajStrDel(&mp);
    ajStrDel(&bp);
    ajStrDel(&ap);
    ajStrDel(&fm);
}




/* @func embAlignPrintProfile *******************************************
**
** Print a profile alignment
** Nucleotides or proteins as needed.
**
** @param [w] outf [AjPFile] output stream
** @param [r] a [char *] complete first sequence
** @param [r] b [char *] complete second sequence
** @param [r] m [AjPStr] Walk alignment for first sequence
** @param [r] n [AjPStr] Walk alignment for second sequence
** @param [r] start1 [int] start of alignment in first sequence
** @param [r] start2 [int] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] fmatrix [float **] profile
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [int] first sequence offset
** @param [r] beginb [int] second sequence offset
** 
** @return [void]
******************************************************************************/

void embAlignPrintProfile(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			int start1, int start2, float score, AjBool mark,
			float **fmatrix, char *namea,
			char *nameb, int begina, int beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;
    
    int i;
    int olen;
    char *p;
    char *q;
    char *r=NULL;
    
    float match=0.0;

    int acnt;
    int bcnt;
    int aend;
    int bend;
    
    int len;
    int pos;
    
    fm = ajStrNewC("");
    ap = ajStrNewC("");
    bp = ajStrNewC("");
    mp = ajStrNewC("");
    

    /* Now deal with the alignment overlap */
    p=ajStrStr(m);
    q=ajStrStr(n);
    olen=strlen(p);
    fa=m;
    fb=n;
    if(mark)
    {
	for(i=0;i<olen;++i)
	{
	    if(p[i]=='.' || q[i]=='.')
	    {
		(void) ajStrAppC(&fm," ");
		continue;
	    }
	    match=fmatrix[start2+i][ajAZToInt(p[i])];
	    if(p[i]==q[i])
	    {
		(void) ajStrAppC(&fm,"|");
		continue;
	    }
	    if(match>0.0)
		(void) ajStrAppC(&fm,":");
	    else
		(void) ajStrAppC(&fm," ");
	}
    }

    /* Get start residues */
    p=ajStrStr(fa);
    q=ajStrStr(fb);
    acnt=begina+start1;
    bcnt=beginb+start2;
    
    len=ajStrLen(fa);
    pos=0;
    if(mark) r=ajStrStr(fm);


    /* Add header stuff here */
    ajFmtPrintF(outf,"Local: %s vs %s\n",namea,nameb);
    ajFmtPrintF(outf,"Score: %.2f\n\n",score);
    
    while(pos<len)
    {
	if(pos+45 < len)
	{
	    (void) ajStrAssSubC(&ap,p,pos,pos+45-1);
	    (void) ajStrAssSubC(&bp,q,pos,pos+45-1);
	    if(mark)
		(void) ajStrAssSubC(&mp,r,pos,pos+45-1);
	    for(i=0,aend=acnt,bend=bcnt;i<45;++i)
	    {
		if(p[pos+i]!=' ' && p[pos+i]!='.') ++aend;
		if(q[pos+i]!=' ' && q[pos+i]!='.') ++bend;
	    }


	    ajFmtPrintF(outf,"%-15.15s ",namea);
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d ",acnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45s ",ajStrStr(ap));
	    if(aend!=acnt)
		ajFmtPrintF(outf,"%-8d\n",aend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    acnt=aend;
	    
	    if(mark)
		ajFmtPrintF(outf,"                         %s\n",ajStrStr(mp));
		
	    ajFmtPrintF(outf,"%-15.15s ",nameb);
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d ",bcnt);
	    else
		ajFmtPrintF(outf,"         ");
	    ajFmtPrintF(outf,"%-45s ",ajStrStr(bp));
	    if(bend!=bcnt)
		ajFmtPrintF(outf,"%-8d\n",bend-1);
	    else
		ajFmtPrintF(outf,"\n");
	    bcnt=bend;

	    ajFmtPrintF(outf,"\n");
	    pos += 45;
	    continue;
	}
	
	(void) ajStrAssC(&ap,&p[pos]);
	(void) ajStrAssC(&bp,&q[pos]);
	if(mark)
	    (void) ajStrAssC(&mp,&r[pos]);
	for(i=0,aend=acnt,bend=bcnt;i<45 && p[pos+i];++i)
	{
	    if(p[pos+i]!=' ' && p[pos+i]!='.') ++aend;
	    if(q[pos+i]!=' ' && q[pos+i]!='.') ++bend;
	}
	
	
	ajFmtPrintF(outf,"%-15.15s ",namea);
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d ",acnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45s ",ajStrStr(ap));
	if(aend!=acnt)
	    ajFmtPrintF(outf,"%-8d\n",aend-1);
	else
	    ajFmtPrintF(outf,"\n");
	acnt=aend;
	
	if(mark)
	    ajFmtPrintF(outf,"                         %s\n",ajStrStr(mp));
	
	ajFmtPrintF(outf,"%-15.15s ",nameb);
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d ",bcnt);
	else
	    ajFmtPrintF(outf,"         ");
	ajFmtPrintF(outf,"%-45s ",ajStrStr(bp));
	if(bend!=bcnt)
	    ajFmtPrintF(outf,"%-8d\n",bend-1);
	else
	    ajFmtPrintF(outf,"\n");
	bcnt=bend;
	
	pos=len;
    }

    ajStrDel(&mp);
    ajStrDel(&bp);
    ajStrDel(&ap);
    ajStrDel(&fm);
}



void embAlignUnused(void)
{
    char *a=NULL;
    char *b=NULL;
    int  lena=0;
    int lenb=0;
    float gapopen=0.0;
    float gapextend=0.0;
    float *path=NULL;
    float **sub=NULL;
    AjPSeqCvt cvt=NULL;
    int *compass=NULL;
    AjBool show=0;
    
    alignPathCalcOld(a,b,lena,lenb,gapopen,gapextend,path,sub,cvt,
		       compass,show);
}
/* @func embAlignPathCalcFast *************************************************
**
** Create path matrix for Smith-Waterman and Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [char *] first sequence
** @param [r] b [char *] second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [int *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** @param [r] width [int] width of path matrix
** 
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** Further speeded up by using only width calculstions instead of lena.
**
** @return [void]
******************************************************************************/
void embAlignPathCalcFast(char *a, char *b, int lena, int lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     int *compass, AjBool show,int width)
{
    int xpos;
    int i;
    int j;
    float match;
    float mscore;
    float fnew;
    float *maxa,*maxb;
    int jlena;

    float max;
    static AjPStr outstr = NULL;

    jlena = lena - 10;
    if (jlena < 0) jlena = lena-1;
    /*    jlenb = lenb - 10;
	  if (jlena < 0) jlenb = lenb-1;*/

    ajDebug ("embAlignPathCalcFast\n");

    /*ajDebug ("lena: %d lenb: %d width: %d\n", lena, lenb, width);
      ajDebug ("a: '%10.10s .. %10.10s' %d\n", a, &a[jlena], lena);
      ajDebug ("b: '%10.10s .. %10.10s' %d\n", b, &b[jlenb], lenb);*/

    /* Create stores for the maximum values in a row or column */

    /*    ajUser("%d %d",lena,lenb);*/
    maxa = AJALLOC(lena*sizeof(float));
    maxb = AJALLOC(lenb*sizeof(float));
    
    /* First initialise the first column and row */
    for(i=0;i<lena;++i)
    {
	path[i*width] = sub[ajSeqCvtK(cvt,a[i])][ajSeqCvtK(cvt,b[0])];
	compass[i*width] = 0;
    }

    for(i=0;i<lena;++i)
      maxa[i] = path[i*width]-(gapopen); 

    for(j=0;j<width;++j)
    {
	path[j] = sub[ajSeqCvtK(cvt,a[0])][ajSeqCvtK(cvt,b[j])];
	compass[j] = 0;
    }

    for(j=width;j<lenb;++j)
      maxb[j] = -1000.0;
    
    for(j=0;j<width;++j)
      maxb[j] = path[j]-(gapopen);
    
    /*    ajUser("2   %d %d",lena,lenb);*/
    /* now step through and build the path matrix */
    i=1;
    while(i<lena)
    {
      xpos=0;
      while(xpos<width && i+xpos < lenb)
	{
	  /* get match for current xpos/ypos */
	  match = sub[ajSeqCvtK(cvt,a[i])][ajSeqCvtK(cvt,b[i+xpos])];
	  
	  /* Get diag score */
	  mscore = path[(i-1)*width+xpos] + match;
	  if(mscore < 0.0)
	    mscore =0.0;

	  /* Set compass to diagonal value 0 */
	  compass[i*width+xpos] = 0;
	  path[i*width+xpos] = mscore;
	  
	  /* update the maximum against the previous point */
	  if(xpos > 0){
	    fnew=path[(i-1)*width+xpos-1];
	    fnew-=gapopen;
	    if(maxa[i-1] < fnew)
	      maxa[i-1]=fnew;
	    else
	      maxa[i-1]-=gapextend;
	  }
	  if(i>1){
	    if(xpos < width-1){
	      fnew=path[(i-2)*width+xpos+1];	    
	      fnew-=gapopen;
	      if(fnew>maxb[i+xpos-1])
		maxb[i+xpos-1]=fnew;
	      else
		maxb[i+xpos-1]-=gapextend;
	    }
	    else
	      maxb[i+xpos-1]-=gapextend;
	  }
	  /* Now parade back along X axis */
	  if( maxa[i-1]+match > mscore){
	    mscore = maxa[i-1]+match;
	    path[i*width+xpos] = mscore;
	    compass[i*width+xpos] = 1; /* Score comes from left */
	  }
	      

	    /* And then bimble down Y axis */
	  if(maxb[i+xpos-1]+match > mscore){
	    mscore = maxb[i+xpos-1]+match;
	    path[i*width+xpos] = mscore;
	    compass[i*width+xpos] = 2; /* Score comes from bottom */
	  }


	  xpos++;
	}
      ++i;
      
    }
    
    max = -1000.0;
    if(show)
      {
	for(i=0;i<lena;++i)
	{
	  ajStrDelReuse (&outstr);
	  for(j=0;j<width;++j){
	    (void) ajFmtPrintAppS(&outstr,"%6.2f ",path[i*width+j]);
	    if(path[i*width+j] > max){
	      max = path[i*width+j];
	    }
	  }
	  (void) ajUser("%S", outstr);
	}
    }

    AJFREE(maxa);
    AJFREE(maxb);
    ajStrDelReuse (&outstr);

    return;
}

/* @func embAlignScoreSWMatrixFast ********************************************
**
** Walk down a matrix for Smith waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [int *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [int *] start of alignment in first sequence
** @param [w] start2 [int *] start of alignment in second sequence
** @param [r] width [int] width of path matrix
** 
** @return [float] Score of best matching segment
******************************************************************************/
float embAlignScoreSWMatrixFast(float *path, int *compass, float gapopen,
                          float gapextend,  AjPSeq a, AjPSeq b,
                          int lena, int lenb, float **sub,
                          AjPSeqCvt cvt, int *start1, int *start2,int width)
{
    int i;
    int j;
    float pmax=-1000.0;
    float score=0.,wscore=0.;
    float match=0.;
    float gapcnt=0.;
    float bimble=0.;
    
    int ix;
    int iy;
    int t;
    
    int xpos=0,xpos2=0;
    int ypos=0;
    char *p;
    char *q;

    ajDebug("embAlignScoreSWMatrixFast\n");

    /*ajDebug("SeqA '%s' %d %d\n", ajSeqName(a), ajSeqLen(a), lena);
      ajDebug("SeqB '%s' %d %d\n", ajSeqName(b), ajSeqLen(b), lenb);
      ajDebug("start1: %d start2: %d width; %d\n", *start1, *start2, width);*/

    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<width;++j)
	    if(path[i*width+j]>pmax)
	    {
		pmax=path[i*width+j];
		xpos=j;
		ypos=i;
	    }

  p = ajSeqChar(a);
  q = ajSeqChar(b);
  
  p+=(*start1);
  q+=(*start2);

  xpos2 = ypos+xpos;
  
  wscore = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])];
  

  while(xpos>=0 && ypos && path[ypos*width+xpos] >0.)
    {
      /*ajDebug ("(*) '%c' '%c' xpos: %d ypos: %d path[%d] %.2f\n",
	       p[xpos], q[ypos], xpos, ypos,
	       ypos*width+xpos, path[ypos*width+xpos]);*/

      if(!compass[ypos*width+xpos])    /* diagonal */
        {
	  if(path[(ypos-1)*width+xpos] +
	     sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])]
	     != path[(ypos)*width+xpos])
	    {
	      ajFatal("SW: Error walking match");
	    }	  
	  if(path[(ypos-1)*width+xpos]<=0.0) break;
	  wscore += sub[ajSeqCvtK(cvt,p[--ypos])][ajSeqCvtK(cvt,q[--xpos2])];
	  continue;
        }
 
      else if(compass[ypos*width+xpos]==1)     /* Left, gap(s) in vertical */
        {
	  score = path[ypos*width+xpos];
	  gapcnt=0.;
	  ix=xpos-1;
	  match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])];
	  --ypos;
	  t=xpos2-1;
	  while(1)
            {
	      /*ajDebug ("(1) ypos: %d * %d + %d\n",
		ypos, width, ix);*/
	      /*ajDebug ("(1) path[%d] = %.2f gapcnt: %.0f\n",
		ypos*width+ix, path[ypos*width+ix], gapcnt);*/

	      bimble=path[ypos*width+ix]-gapopen-(gapcnt*gapextend)+match;

	      /*ajDebug ("(1) fabs (%.2f - %.2f) = %.2f\n",
		score, bimble, fabs((double)score-(double)bimble));*/

	      if(fabs((double)score-(double)bimble)<0.1) break;
	      --ix;
	      if(ix<0)
		ajFatal("SW: Error walking left");
	      ++gapcnt;
            }
	  if(score<=0.0) break;
	  t -= (int)gapcnt+1;
	  wscore += sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[t])];
	  wscore -= (gapopen + (gapextend*gapcnt));

	  xpos2 = t;
	  xpos=ix;
	  /*ajDebug("xpos => %d\n", xpos);*/
	  continue;
        }

      else if(compass[ypos*width+xpos]==2) /* Down, gap(s) in horizontal */
        {
	  score=path[ypos*width+xpos];
	  gapcnt=0.;
	  match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])];
	  xpos++;
	  iy=ypos-2;
	  t=iy+1;
	  while(1)
	    {
	      /*ajDebug ("(2) %d * %d + xpos: %d\n",
		iy, width, xpos);*/
	      /*ajDebug ("(2) path[%d] = %.2f gapcnt: %.0f\n",
		iy*width+xpos, path[iy*width+xpos], gapcnt);*/

	      bimble=path[iy*width+xpos]-gapopen-(gapcnt*gapextend)+match;

	      /*ajDebug ("(2) fabs (%.2f - %.2f) = %.2f\n",
		score, bimble, fabs((double)score-(double)bimble));*/

	      if(fabs((double)score-(double)bimble)<0.1) break;
	      --iy;++xpos;

	      if(iy<0) {
		/*ajDebug("SW: Error walking down %d < 0 gapcnt: %d\n",
		  iy, gapcnt);*/

		ajFatal("SW: Error walking down");
	      }
	      ++gapcnt;
            }
	  if(score<=0.0) break;
	  t -= (int)gapcnt;

	  wscore += sub[ajSeqCvtK(cvt,p[t])][ajSeqCvtK(cvt,q[xpos2])];
	  wscore -= (gapopen + (gapextend*gapcnt));

	    
	  ypos=iy;
	  /*ajDebug("ypos => %d\n", ypos);*/
	  xpos2--;
	  continue;
        }
      else
	ajFatal("Walk Error in SW");
    }
  return wscore;
}
/* @func embAlignWalkSWMatrixFast *********************************************
**
** Walk down a matrix for Smith waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [int *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [int] length of first sequence
** @param [r] lenb [int] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [int *] start of alignment in first sequence
** @param [w] start2 [int *] start of alignment in second sequence
** @param [r] width [int] width of path matrix
** 
** @return [void]
******************************************************************************/

void embAlignWalkSWMatrixFast(float *path, int *compass, float gapopen,
			 float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, int lena, int lenb, float **sub,
			 AjPSeqCvt cvt, int *start1, int *start2,int width)
{
    int i;
    int j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    int ix;
    int iy;
    int t;
    
    int xpos=0,xpos2=0;
    int ypos=0;
    char *p;
    char *q;
    char r[2]="?";
    char dot[2] = ".";

    float ic;
    
    ajDebug("embAlignWalkSWMatrixFast\n");

    /* Get maximum path score and save position */
    pmax = (float) (-1*INT_MAX);
    for(i=0;i<lena;++i)
	for(j=0;j<width;++j)
	    if(path[i*width+j]>pmax)
	    {
		pmax=path[i*width+j];
		xpos=j;
		ypos=i;
	    }
    
    p = ajSeqChar(a);
    q = ajSeqChar(b);
    p+=(*start1);
    q+=(*start2);

    xpos2 = xpos+ypos;

    *r = p[ypos];
    (void) ajStrAssC(m,r);
    *r = q[xpos2];
    (void) ajStrAssC(n,r);

    
    while(xpos>=0 && ypos && path[ypos*width+xpos] >0.)
    {

	if(!compass[ypos*width+xpos])	/* diagonal */
	{
	  if(path[(ypos-1)*width+xpos] + sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])]
	     != path[(ypos)*width+xpos]){
	    ajFatal("SW: Error walking match");
	    
	  }
	  if(path[(ypos-1)*width+xpos]<=0.0) break;
	  (void) ajStrAppK(m,p[--ypos]);
	  (void) ajStrAppK(n,q[--xpos2]);
	  continue;
	}
	else if(compass[ypos*width+xpos]==1)	/* Left, gap(s) in vertical */
	{
	    score = path[ypos*width+xpos];
	    gapcnt=0.;
	    ix=xpos-1;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])];
	    --ypos;
	    t=xpos2-1;
	    while(1)
	    {
		bimble=path[ypos*width+ix]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<0.1) break;
		--ix;
		if(ix<0){
		  ajFatal("SW: Error walking left");
		}
		++gapcnt;
	    }
	    if(score<=0.0) break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrAppK(m,dot[0]);
		(void) ajStrAppK(n,q[t--]);
	    }
	    (void) ajStrAppK(n,q[t]);
	    (void) ajStrAppK(m,p[ypos]);

	    xpos2 =t;       /* change xpos2 */
	    xpos=ix;
	    continue;
	}
	else if(compass[ypos*width+xpos]==2) /* Down, gap(s) in horizontal */
	{
	    score=path[ypos*width+xpos];
	    gapcnt=0.;
	    match = sub[ajSeqCvtK(cvt,p[ypos])][ajSeqCvtK(cvt,q[xpos2])];
	    xpos++;
	    iy=ypos-2;
	    t=iy+1;
	 
	    while(1)
	    {
		bimble=path[iy*width+xpos]-gapopen-(gapcnt*gapextend)+match;
		if(fabs((double)score-(double)bimble)<0.1) break;
		--iy; ++xpos;
		if(iy<0){
		  ajFatal("SW: Error walking down");
		}
		++gapcnt;
	    }
	    if(score<=0.0) break;
	    for(ic=-1;ic<gapcnt;++ic)
	    {
		(void) ajStrAppK(n,dot[0]);
		(void) ajStrAppK(m,p[t--]);
	    }
	    (void) ajStrAppK(m,p[t]);
	    (void) ajStrAppK(n,q[--xpos2]);
	    ypos=iy;
	    continue;
	}
	else
	    ajFatal("Walk Error in SW");
	

    }
    
    (void) ajStrRev(m);
    (void) ajStrRev(n);

    *start1 += ypos;
    *start2 += xpos2;
    
    return;
}


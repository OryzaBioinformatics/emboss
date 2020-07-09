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


#define GAPO 26
#define GAPE 27

#define DIAG 0
#define LEFT 1
#define DOWN 2

/* @func embAlignPathCalc  *************************************************
**
** Create path matrix for Smith-Waterman and Needleman-Wunsch
** Nucleotides or proteins as needed.
**
** @param [r] a [char *] first sequence
** @param [r] b [char *] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** 
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** @return [void]
******************************************************************************/
void embAlignPathCalc(char *a, char *b, ajint lena, ajint lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     ajint *compass, AjBool show)
{
    ajint xpos;
    ajint i;
    ajint j;

    float match;
    float mscore;
    float fnew;
    float *maxa,*maxb;
    float *oval;
    ajint   *cnt;
    
    static AjPStr outstr = NULL;
    float bx;
    ajint   bv;
    
    ajDebug("embAlignPathCalc\n");

    /* Create stores for the maximum values in a row or column */

    maxa = AJALLOC(lena*sizeof(float));
    maxb = AJALLOC(lenb*sizeof(float));
    oval = AJALLOC(lena*sizeof(float));
    cnt  = AJALLOC(lena*sizeof(ajint));
    

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
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** 
** @return [void]
******************************************************************************/

static void alignPathCalcOld(char *a, char *b, ajint lena, ajint lenb,
			       float gapopen, float gapextend, float *path,
			       float **sub, AjPSeqCvt cvt,
			       ajint *compass, AjBool show)
{
    ajint xpos;
    ajint ypos;
    ajint i;
    ajint j;

    ajint im;
    ajint jm;
    
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
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening coefficient
** @param [r] compass [ajint*] Path direction pointer array
** @param [r] gapextend [float] gap extension coefficient
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** 
** @return [float] Maximum path axis score
** @@
******************************************************************************/

float embAlignScoreNWMatrix(float *path, AjPSeq a, AjPSeq b, float **fmatrix,
			   AjPSeqCvt cvt, ajint lena, ajint lenb, float gapopen,
			   ajint *compass,
			   float gapextend, ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;

    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    float wscore;
    float errbounds = gapextend;
    
    ajint ix;
    ajint iy;
    ajint t;
    
    ajint xpos=0;
    ajint ypos=0;
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
	    t -= (ajint)gapcnt;
	    
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
	    t -= (ajint)gapcnt;

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





/* @func embAlignScoreSWMatrix ************************************************
**
** Walk down a matrix for Smith waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** 
** @return [float] Score of best matching segment
******************************************************************************/

float embAlignScoreSWMatrix(float *path, ajint *compass, float gapopen,
			  float gapextend,  AjPSeq a, AjPSeq b,
			  ajint lena, ajint lenb, float **sub,
			  AjPSeqCvt cvt, ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    float wscore;
    
    ajint ix;
    ajint iy;
    ajint t;
    
    ajint xpos=0;
    ajint ypos=0;
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
	    if(bimble<0.0)
	    {
		++ypos;
		break;
	    }
	    
	    t -= (ajint)gapcnt;

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
	    if(bimble<0.0)
	    {
		++xpos;
		break;
	    }
	    
	    t -= (ajint)gapcnt;

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
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** 
** @return [void]
******************************************************************************/

void embAlignWalkSWMatrix(float *path, ajint *compass, float gapopen,
			 float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, ajint lena, ajint lenb, float **sub,
			 AjPSeqCvt cvt, ajint *start1, ajint *start2)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    ajint ix;
    ajint iy;
    ajint t;
    
    ajint xpos=0;
    ajint ypos=0;
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
	    if(bimble<0.0)
	    {
		++ypos;
		break;
	    }
	    
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
	    if(bimble<0.0)
	    {
		++xpos;
		break;
	    }
	    
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
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] gapopen [float] gap open penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** 
** @return [void]
******************************************************************************/

void embAlignWalkNWMatrix(float *path, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, ajint lena, ajint lenb, ajint *start1,
			 ajint *start2, float gapopen, 
			 float gapextend, AjPSeqCvt cvt, ajint *compass,
			 float **sub)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    ajint ix;
    ajint iy;
    ajint t;
    
    ajint xpos=0;
    ajint ypos=0;
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
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float **] substitution matrix
** @param [r] cvt [AjPSeqCvt] conversion table for matrix
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
** 
** @return [void]
******************************************************************************/

void embAlignPrintGlobal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			ajint start1, ajint start2, float score, AjBool mark,
			float **sub, AjPSeqCvt cvt, char *namea,
			char *nameb, ajint begina, ajint beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;
    
    ajint i;
    ajint nc;
    ajint olen;
    char *p;
    char *q;
    char *r=NULL;
    
    float match=0.0;

    ajint apos;
    ajint bpos;
    ajint alen;
    ajint blen;
    ajint acnt;
    ajint bcnt;
    ajint aend;
    ajint bend;
    
    ajint len;
    ajint pos;
    
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
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float **] substitution matrix
** @param [r] cvt [AjPSeqCvt] conversion table for matrix
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
** 
** @return [void]
******************************************************************************/

void embAlignPrintLocal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			ajint start1, ajint start2, float score, AjBool mark,
			float **sub, AjPSeqCvt cvt, char *namea,
			char *nameb, ajint begina, ajint beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;
    
    ajint i;
    ajint olen;
    char *p;
    char *q;
    char *r=NULL;
    
    float match=0.0;

    ajint acnt;
    ajint bcnt;
    ajint aend;
    ajint bend;
    
    ajint len;
    ajint pos;
    
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

/* @func embAlignUnused *******************************************************
**
** Calls unused functions to avoid warning messages
**
******************************************************************************/

void embAlignUnused(void)
{
    char *a=NULL;
    char *b=NULL;
    ajint  lena=0;
    ajint lenb=0;
    float gapopen=0.0;
    float gapextend=0.0;
    float *path=NULL;
    float **sub=NULL;
    AjPSeqCvt cvt=NULL;
    ajint *compass=NULL;
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
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [w] path [float *] path matrix
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** @param [r] pathwidth [ajint] width of path matrix
** 
** Optimised to keep a maximum value to avoid looping down or left
** to find the maximum. (il 29/07/99)
**
** Further speeded up by using only width calculstions instead of lena.
**
** @return [void]
******************************************************************************/
void embAlignPathCalcFast(char *a, char *b, ajint lena, ajint lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     ajint *compass, AjBool show, ajint pathwidth)
{
    ajint xpos;
    ajint i;
    ajint j;
    float match;
    float mscore;
    float fnew;
    float *maxa,*maxb;
    ajint jlena;
    ajint jlenb;
    ajint width = pathwidth;

    float max;
    static AjPStr outstr = NULL;

    if (lena < width)
      width = lena;
    if (lenb < width)
      width = lenb;

    jlena = lena - 10;
    if (jlena < 0) jlena = lena-1;

        jlenb = lenb - 10;
	  if (jlena < 0) jlenb = lenb-1;

    ajDebug ("embAlignPathCalcFast\n");

    ajDebug ("lena: %d lenb: %d width: %d\n", lena, lenb, width);
    ajDebug ("a: '%10.10s .. %10.10s' %d\n", a, &a[jlena], lena);
    ajDebug ("b: '%10.10s .. %10.10s' %d\n", b, &b[jlenb], lenb);

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
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] width [ajint] width of path matrix
** 
** @return [float] Score of best matching segment
******************************************************************************/
float embAlignScoreSWMatrixFast(float *path, ajint *compass, float gapopen,
				float gapextend,  AjPSeq a, AjPSeq b,
				ajint lena, ajint lenb, float **sub,
				AjPSeqCvt cvt, ajint *start1, ajint *start2,
				ajint width)
{
    ajint i;
    ajint j;
    float pmax=-1000.0;
    float score=0.,wscore=0.;
    float match=0.;
    float gapcnt=0.;
    float bimble=0.;
    
    ajint ix;
    ajint iy;
    ajint t;
    
    ajint xpos=0,xpos2=0;
    ajint ypos=0;
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
	  t -= (ajint)gapcnt+1;
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
	  t -= (ajint)gapcnt;

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
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] gapopen [float] gap opening penalty
** @param [r] gapextend [float] gap extension penalty
** @param [r] a [AjPSeq] first sequence
** @param [r] b [AjPSeq] second sequence
** @param [w] m [AjPStr *] alignment for first sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] lena [ajint] length of first sequence
** @param [r] lenb [ajint] length of second sequence
** @param [r] sub [float **] substitution matrix from AjPMatrixf
** @param [r] cvt [AjPSeqCvt] Conversion array for AjPMatrixf
** @param [w] start1 [ajint *] start of alignment in first sequence
** @param [w] start2 [ajint *] start of alignment in second sequence
** @param [r] width [ajint] width of path matrix
** 
** @return [void]
******************************************************************************/

void embAlignWalkSWMatrixFast(float *path, ajint *compass, float gapopen,
			      float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			      AjPStr *n, ajint lena, ajint lenb, float **sub,
			      AjPSeqCvt cvt, ajint *start1, ajint *start2,
			      ajint width)
{
    ajint i;
    ajint j;
    float pmax;
    float score;
    float match;
    float gapcnt;
    float bimble;
    
    ajint ix;
    ajint iy;
    ajint t;
    
    ajint xpos=0,xpos2=0;
    ajint ypos=0;
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

/* @func embAlignProfilePathCalc  ******************************************
**
** Create path matrix for a profile
** Nucleotides or proteins as needed.
**
** @param [r] a [char *] sequence
** @param [r] proflen [ajint] length of profile
** @param [r] seqlen [ajint] length of sequence
** @param [r] gapopen [float] gap opening coefficient
** @param [r] gapextend [float] gap extension coefficient
** @param [w] path [float *] path matrix
** @param [r] fmatrix [float **] profile matrix
** @param [w] compass [ajint *] Path direction pointer array
** @param [r] show [AjBool] Display path matrix
** 
** @return [void]
******************************************************************************/

void embAlignProfilePathCalc(char *a, ajint proflen, ajint seqlen,
			     float gapopen,
			     float gapextend, float *path, float **fmatrix,
			     ajint *compass, AjBool show)
{
    ajint row;		/* profile position in path */
    ajint rowx;
    
    
    ajint column;	/* sequence position in path */
    ajint columnx;
    
    float penalty;
    static AjPStr outstr = NULL;

    float score;
    float fmscore;
    float diagscore;
    float currmax;
    
    
    ajDebug("embAlignProfilePathCalc\n");

    /* First initialise the first column and row */
    for(column=0;column<seqlen;++column)
    {
	path[column] = fmatrix[0][ajAZToInt(a[column])];
	compass[column] = DIAG;
    }
    for(row=0;row<proflen;++row)
    {
	path[row*seqlen] = fmatrix[row][ajAZToInt(*a)];
	compass[row*seqlen] = DIAG;
    }


    /* diagonal steps start at 1 */
    column=1;
    while(column!=seqlen)
    {
	for(row=1;row<proflen;++row)
	{
	    /* get match for current xpos/ypos */
	    fmscore = fmatrix[row][ajAZToInt(a[column])];

	    /* Get diag score */
	    diagscore = path[(row-1)*seqlen+(column-1)];

	    /* Initialise current maximum to diagonal score */
	    currmax = diagscore + fmscore;

	    /* Initialise compass to diagonal value */
	    compass[row*seqlen+column] = DIAG;
	    path[row*seqlen+column] = currmax;
	
	    /* Now parade back along X axis */
	    if(column-2>-1)
	    {
		for(columnx=column-2;columnx>-1;--columnx)
		{
		    score = path[(row-1)*seqlen+columnx];
		    score += fmscore;
		    penalty  = -(fmatrix[(row-1)][GAPO] * gapopen +
				 ((column-columnx-2) * gapextend *
				  fmatrix[(row-1)][GAPE]));
		    score += penalty;
		    

		    if(score>currmax)
		    {
			currmax=score;
			path[row*seqlen+column] = currmax;
			compass[row*seqlen+column] = LEFT;
		    }
		}
	    }

	    /* And then bimble down Y axis */
	    if(row-2>-1)
	    {
		for(rowx=row-2;rowx>-1;--rowx)
		{
		    score = path[rowx*seqlen+(column-1)];
		    score += fmscore;
		    
		    penalty  = -(fmatrix[rowx][GAPO] * gapopen +
				 ((float) (row-rowx-2.)
				  * gapextend * fmatrix[rowx][GAPE]));
		    score += penalty;

		    if(score>currmax)
		    {
			currmax = score;
			path[row*seqlen+column] = currmax;
			compass[row*seqlen+column] = DOWN;
		    }
		}
	    }
	}

	++column;
    }





    if(show)
    {
	for(row=proflen-1;row>-1;--row)
	{
	    ajStrDelReuse(&outstr);
	    for(column=0;column<seqlen;++column)
		(void) ajFmtPrintAppS(&outstr,"%6.2f ",
				      path[row*seqlen+column]);
	    (void) ajUser("%S", outstr);
	}
    }
    

    ajStrDelReuse (&outstr);
    return;
}








/* @func embAlignWalkProfileMatrix *******************************************
**
** Walk down a profile path matrix for Smith Waterman. Form aligned strings.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] gapopen [float] gap opening coeff
** @param [r] gapextend [float] gap extension coeff
** @param [r] cons [AjPStr] consensus sequence
** @param [r] seq [AjPStr] second sequence
** @param [w] m [AjPStr *] alignment for consensus sequence
** @param [w] n [AjPStr *] alignment for second sequence
** @param [r] proflen [ajint] length of consensus sequence
** @param [r] seqlen [ajint] length of test sequence
** @param [r] fmatrix [float **] profile
** @param [w] start1 [ajint *] start of alignment in consensus sequence
** @param [w] start2 [ajint *] start of alignment in test sequence
** 
** @return [void]
******************************************************************************/

void embAlignWalkProfileMatrix(float *path, ajint *compass, float gapopen,
			       float gapextend, AjPStr cons, AjPStr seq,
			       AjPStr *m, AjPStr *n, ajint proflen,
			       ajint seqlen, float **fmatrix,
			       ajint *start1, ajint *start2)
{
    ajint i;
    float pathmax;
    float targetscore;
    float currscore;
    
    float match;
    ajint gapcnt;
    float penalty=0.;
    
    ajint row    = 0;
    ajint column = 0;

    ajint colstep;
    ajint rowstep;
    

    ajint direction=0;

    ajint xpos=0;
    ajint ypos=0;

    char *p;
    char *q;
    char r[2]="?";
    char dot[2]=".";

    float errbounds=0.01;
    
    ajDebug("embAlignWalkProfileMatrix\n");

    /* Get maximum path score and save position */
    pathmax = -(float) INT_MAX;
    for(row=0;row<proflen;++row)
	for(column=0;column<seqlen;++column)
	    if(path[row*seqlen+column] > pathmax)
	    {
		pathmax=path[row*seqlen+column];
		xpos=column;
		ypos=row;
	    }
    

    column = xpos;
    row = ypos;

    p = ajStrStr(cons);
    q = ajStrStr(seq);

    *r = p[row];
    (void) ajStrAssC(m,r);
    *r = q[column];
    (void) ajStrAssC(n,r);

    
    while(row && column)
    {
	direction = compass[row*seqlen+column];
	if(direction == DIAG)
	{
	    if(path[(row-1)*seqlen+(column-1)]<0.)
		break;
	    *r=p[--row];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[--column];
	    (void) ajStrInsertC(n,0,r);
	    continue;
	}
	else if(direction == LEFT)
	{
	    targetscore = path[row*seqlen+column];
	    gapcnt = 0;
	    colstep=column-2;
	    currscore = -(float)INT_MAX;
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[(row-1)*seqlen+colstep];
		penalty = -(fmatrix[row-1][GAPO] * gapopen +
			    fmatrix[row-1][GAPE] * (float)gapcnt * gapextend);
		currscore += penalty;
		currscore += match;
		
		++gapcnt;
		if(currscore-penalty < 0.)
		    break;

		--colstep;
	    }
	    
	    for(i=0;i<gapcnt;++i)
	    {
		(void) ajStrInsertC(m,0,dot);
		*r=q[--column];
		(void) ajStrInsertC(n,0,r);
	    }

	    *r=q[--column];
	    (void) ajStrInsertC(n,0,r);
	    *r=p[--row];
	    (void) ajStrInsertC(m,0,r);

	    continue;
	}
	else if(direction == DOWN)
	{
	    targetscore=path[row*seqlen+column];
	    gapcnt=0;
	    rowstep=row-2;
	    currscore = -(float)(INT_MAX);
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[rowstep*seqlen+(column-1)];
		penalty = -(fmatrix[rowstep][GAPO] * gapopen +
			    fmatrix[rowstep][GAPE] * (float)gapcnt *
			    gapextend);
		currscore += penalty;
		currscore += match;
		
		++gapcnt;

		if(currscore-penalty < 0.)
		    break;

		--rowstep;
	    }

	    for(i=0;i<gapcnt;++i)
	    {
		(void) ajStrInsertC(n,0,dot);
		*r=p[--row];
		(void) ajStrInsertC(m,0,r);
	    }

	    *r=p[--row];
	    (void) ajStrInsertC(m,0,r);
	    *r=q[column];
	    (void) ajStrInsertC(n,0,r);

	    continue;
	}
	else
	    ajFatal("Walk Error in Profile Walk");
    }

    *start1 = row;
    *start2 = column;
    
    return;
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
** @param [r] start1 [ajint] start of alignment in first sequence
** @param [r] start2 [ajint] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] fmatrix [float **] profile
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [ajint] first sequence offset
** @param [r] beginb [ajint] second sequence offset
** 
** @return [void]
******************************************************************************/

void embAlignPrintProfile(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			ajint start1, ajint start2, float score, AjBool mark,
			float **fmatrix, char *namea,
			char *nameb, ajint begina, ajint beginb)
{
    AjPStr fa;
    AjPStr fb;
    AjPStr fm;
    AjPStr ap;
    AjPStr bp;
    AjPStr mp;
    
    ajint i;
    ajint olen;
    char *p;
    char *q;
    char *r=NULL;
    
    float match=0.0;

    ajint acnt;
    ajint bcnt;
    ajint aend;
    ajint bend;
    
    ajint len;
    ajint pos;
    ajint cpos;
    
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


    cpos = start1;
    --cpos;
    
    if(mark)
    {
	for(i=0;i<olen;++i)
	{
	    if(p[i]!='.')
		++cpos;
	    if(p[i]=='.' || q[i]=='.')
	    {
		(void) ajStrAppC(&fm," ");
		continue;
	    }
	    match=fmatrix[cpos][ajAZToInt(q[i])];
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

    return;
}





/* @func embAlignCalcSimilarity *******************************************
**
** Calculate Similarity of two sequences (same length)
** Nucleotides or proteins as needed.
**
** @param [r] m [AjPStr] Walk alignment for first sequence
** @param [r] n [AjPStr] Walk alignment for second sequence
** @param [r] sub [float **] substitution matrix
** @param [r] cvt [AjPSeqCvt] conversion table for matrix
** @param [r] lenm [ajint] length of first sequence
** @param [r] lenn [ajint] length of second sequence
** @param [w] id [float *] % identity
** @param [w] sim [float *] % similarity
** @param [w] idx [float *] % identity wrt longest sequence
** @param [w] simx [float *] % similarity wrt longest sequence
** 
** @return [void]
******************************************************************************/

void embAlignCalcSimilarity(AjPStr m, AjPStr n, float **sub, AjPSeqCvt cvt,
			    ajint lenm, ajint lenn, float *id, float *sim,
			    float *idx, float *simx)
{
    ajint   i;
    ajint   olen;
    char  *p=NULL;
    char  *q=NULL;
    float match=0.;
    ajint   max;
    ajint   gaps=0;


    p=ajStrStr(m);
    q=ajStrStr(n);
    olen=strlen(p);


    *id = *sim = 0.;
    

    for(i=0;i<olen;++i)
    {
	if(p[i] =='.' || q[i]=='.')
	{
	    ++gaps;
	    continue;
	}

	match=sub[ajSeqCvtK(cvt,toupper(p[i]))][ajSeqCvtK(cvt,toupper(q[i]))];
	if(p[i]==q[i])
	{
	    ++(*id);
	    ++(*sim);
	    continue;
	}
	if(match>0.0)
	    ++(*sim);
    }

    max = (lenm>lenn) ? lenm : lenn;
    
    *idx  = *id / (float)max * 100.;
    *simx = *sim / (float)max * 100.;
    *id   *= (100. / (float)(olen-gaps));
    *sim  *= (100. / (float)(olen-gaps));

    return;
}


/* @func embAlignScoreProfileMatrix *******************************************
**
** Score a profile path matrix for Smith waterman.
** Nucleotides or proteins as needed.
**
** @param [r] path [float *] path matrix
** @param [r] compass [ajint *] Path direction pointer array
** @param [r] gapopen [float] gap opening coeff
** @param [r] gapextend [float] gap extension coeff
** @param [r] seq [AjPStr] second sequence
** @param [r] proflen [ajint] length of consensus sequence
** @param [r] seqlen [ajint] length of test sequence
** @param [r] fmatrix [float **] profile
** @param [w] start1 [ajint *] start of alignment in consensus sequence
** @param [w] start2 [ajint *] start of alignment in test sequence
** 
** @return [float] profile alignment score
******************************************************************************/

float embAlignScoreProfileMatrix(float *path, ajint *compass, float gapopen,
				float gapextend, AjPStr seq,
				ajint proflen, ajint seqlen, float **fmatrix,
				ajint *start1, ajint *start2)
{
    ajint i;
    float pathmax;
    float targetscore;
    float currscore;
    float wscore=0.;
    
    float match;
    ajint gapcnt;
    float penalty=0.;
    
    ajint row    = 0;
    ajint column = 0;

    ajint colstep;
    ajint rowstep;
    

    ajint direction=0;

    ajint xpos=0;
    ajint ypos=0;

    char *q;

    float errbounds=0.01;
    
    ajDebug("embAlignWalkProfileMatrix\n");

    /* Get maximum path score and save position */
    pathmax = -(float) INT_MAX;
    for(row=0;row<proflen;++row)
	for(column=0;column<seqlen;++column)
	    if(path[row*seqlen+column] > pathmax)
	    {
		pathmax=path[row*seqlen+column];
		xpos=column;
		ypos=row;
	    }
    

    column = xpos;
    row = ypos;

    q = ajStrStr(seq);

    wscore = fmatrix[row][ajAZToInt(q[column])];
    
    while(row && column)
    {
	direction = compass[row*seqlen+column];
	if(direction == DIAG)
	{
	    if(path[(row-1)*seqlen+(column-1)]<0.)
		break;
	    wscore += fmatrix[--row][ajAZToInt(q[--column])];
	    continue;
	}
	else if(direction == LEFT)
	{
	    targetscore = path[row*seqlen+column];
	    gapcnt = 0;
	    colstep=column-2;
	    currscore = -(float)INT_MAX;
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[(row-1)*seqlen+colstep];
		penalty = -(fmatrix[row-1][GAPO] * gapopen +
			    fmatrix[row-1][GAPE] * (float)gapcnt * gapextend);
		currscore += penalty;
		currscore += match;
		
		++gapcnt;
		if(currscore-penalty < 0.)
		    break;

		--colstep;
	    }

	    
	    for(i=0;i<gapcnt;++i)
		--column;

	    wscore += fmatrix[--row][ajAZToInt(q[--column])];	    

	    continue;
	}
	else if(direction == DOWN)
	{
	    targetscore=path[row*seqlen+column];
	    gapcnt=0;
	    rowstep=row-2;
	    currscore = -(float)(INT_MAX);
	    match = fmatrix[row][ajAZToInt(q[column])];

	    while(fabs(targetscore-currscore) > errbounds)
	    {
		currscore = path[rowstep*seqlen+(column-1)];
		penalty = -(fmatrix[rowstep][GAPO] * gapopen +
			    fmatrix[rowstep][GAPE] * (float)gapcnt *
			    gapextend);
		currscore += penalty;
		currscore += match;
		
		++gapcnt;

		if(currscore-penalty < 0.)
		    break;

		--rowstep;
	    }

	    for(i=0;i<gapcnt;++i)
		--row;

	    wscore += fmatrix[--row][ajAZToInt(q[--column])];	    

	    continue;
	}
	else
	    ajFatal("Walk Error in Profile Score");
    }

    *start1 = row;
    *start2 = column;
    
    return wscore;
}

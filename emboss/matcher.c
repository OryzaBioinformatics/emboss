/* @source matcher application
**
** MATCHER finds the best local alignments between two sequences
** version 2.0u4 Feb. 1996
** Please cite:
** X. Huang and W. Miller (1991) Adv. Appl. Math. 12:373-381
**
** @author: Copyright (C) Ian Longden (il@sanger.ac.uk)
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

#define min(x,y) ((x)<=(y) ? (x) : (y))
#define YES 1 
#define NO 0


int SIM(char A[], char B[], int M, int N, int K, int Q, int R, int nseq);
int big_pass(char A[], char B[], int M, int N, int K, int nseq);
int locate(char A[], char B[], int nseq);
int small_pass(char A[], char B[], int count, int nseq);
int diff(char A[], char B[], int M, int N, int tb, int te);
int calcons(char *aa0, int n0, char *aa1, int n1, int *res,
	    int *nc, int *nident);
int discons(char *seqc0, char *seqc1, int nc);



#define gap(k)  ((k) <= 0 ? 0 : q+r*(k))	/* k-symbol indel score */
static int tt;

static int markx;                               /* what to display ? */
static int llen;

static int *sapp;				/* Current script append ptr */
static int  last;				/* Last script op appended */

static int I, J;				/* current positions of A ,B */
static int no_mat; 				/* number of matches */ 
static int no_mis; 				/* number of mismatches */ 
static int al_len; 				/* length of alignment */
						/* Append "Delete k" op */
#define DEL(k)				\
{ I += k;				\
  al_len += k;				\
  if (last < 0)				\
    last = sapp[-1] -= (k);		\
  else					\
    last = *sapp++ = -(k);		\
}
						/* Append "Insert k" op */
#define INS(k)				\
{ J += k;				\
  al_len += k;				\
  if (last < 0)				\
    { sapp[-1] = (k); *sapp++ = last; }	\
  else					\
    last = *sapp++ = (k);		\
}

						/* Append "Replace" op */
#define REP 				\
{ last = *sapp++ = 0; 			\
  al_len += 1;				\
}
/* DIAG() assigns value to x if (ii,jj) is never used before */
#define DIAG(ii, jj, x, value)				\
{                                                \
 for ( tt = 1, z = row[(ii)]; z != PAIRNULL; z = z->NEXT )	\
    if ( z->COL == (jj) )				\
      { tt = 0; break; }				\
  if ( tt )						\
    x = ( value );					\
}

/* replace (ss1, xx1, yy1) by (ss2, xx2, yy2) if the latter is large */
#define ORDER(ss1, xx1, yy1, ss2, xx2, yy2)		\
{ if ( ss1 < ss2 )					\
    { ss1 = ss2; xx1 = xx2; yy1 = yy2; }		\
  else							\
    if ( ss1 == ss2 )					\
      { if ( xx1 < xx2 )				\
	  { xx1 = xx2; yy1 = yy2; }			\
	else						\
	  if ( xx1 == xx2 && yy1 < yy2 )		\
	    yy1 = yy2;					\
      }							\
}

static  AjPSeq seq,seq2;
static int **sub;

typedef struct NODE
	{ int  SCORE;
	  int  STARI;
	  int  STARJ;
	  int  ENDI;
	  int  ENDJ;
	  int  TOP;
	  int  BOT;
	  int  LEFT;
	  int  RIGHT; }  vertex,
     *vertexptr;

		
vertexptr  *LIST;			/* an array for saving k best scores */
vertexptr  low = 0;			/* lowest score node in LIST */
vertexptr  most = 0;			/* latestly accessed node in LIST */
static int numnode;			/* the number of nodes in LIST */
static int *CC, *DD;			/* saving matrix scores */
static int *RR, *SS, *EE, *FF; 		/* saving start-points */
static int *HH, *WW;		 	/* saving matrix scores */
static int *II, *JJ, *XX, *YY; 		/* saving start-points */
static int  m1, mm, n1, nn;		/* boundaries of recomputed area */
static int  rl, cl;			/* left and top boundaries */
static int  lmin;			/* minimum score in LIST */
static int flag;			/* indicate if recomputation necessary*/

static int q, r;			/* gap penalties */
static int qr;				/* qr = q + r */
typedef struct ONE { int COL ;  struct ONE  *NEXT ;} pair, *pairptr;
pairptr *row, z; 			/* for saving used aligned pairs */
#define PAIRNULL (pairptr)NULL

char *seqc0, *seqc1;   /* aligned sequences */

int min0,min1,max0,max1;
int smin0, smin1;
AjPFile outf;
AjPMatrix matrix=NULL;
AjPSeqCvt cvt = NULL;

int main (int argc, char * argv[]) {
  AjPStr aa0str=0,aa1str=0;
  char *s1,*s2;
  int gdelval,ggapval;
  int i,K;

  embInit("matcher", argc, argv);
  seq = ajAcdGetSeq ("sequencea");
  ajSeqTrim(seq);
  seq2 = ajAcdGetSeq ("sequenceb");
  ajSeqTrim(seq2);
  matrix = ajAcdGetMatrix("datafile");
  K = ajAcdGetInt("alternatives");
  gdelval = ajAcdGetInt("gappenalty");
  ggapval = ajAcdGetInt("gaplength");
  markx = ajAcdGetInt("markx");
  llen = ajAcdGetInt("length");
  outf = ajAcdGetOutfile("outfile");

  /* create sequences indexes. i.e. A->0, B->1 ... Z->25 etc.
     This is done so that ajAZToInt has only to be done once for
     each residue in the sequence
     */

  ajSeqToUpper(seq);
  ajSeqToUpper(seq2);

  s1 = ajStrStr(ajSeqStr(seq));
  s2 = ajStrStr(ajSeqStr(seq2));

  sub = ajMatrixArray(matrix);
  cvt = ajMatrixCvt(matrix);

  /*
  ajMatrixSeqNum (matrix, seq,  &aa0str);
  ajMatrixSeqNum (matrix, seq2, &aa1str);
  */

  aa0str = ajStrNewL(2+ajSeqLen(seq)); /* length + blank + trailing null */
  aa1str = ajStrNewL(2+ajSeqLen(seq2));
  ajStrAppK(&aa0str,' ');   
  ajStrAppK(&aa1str,' ');   

  for(i=0;i<ajSeqLen(seq);i++)
    ajStrAppK(&aa0str,(char)ajSeqCvtK(cvt, *s1++));

  for(i=0;i<ajSeqLen(seq2);i++)
    ajStrAppK(&aa1str,ajSeqCvtK(cvt, *s2++));

  SIM(ajStrStr(aa0str),ajStrStr(aa1str),ajSeqLen(seq),ajSeqLen(seq2),
      K,(gdelval-ggapval),ggapval,2);

  ajStrDel(&aa0str);
  ajStrDel(&aa1str);
  ajExit();
  return 0;
}

int SIM(A,B,M,N,K,Q,R,nseq)
	char A[],B[];
	int M,N,K,nseq;
	int Q,R;
{
  int endi, endj, stari, starj; 	/* endpoint and startpoint */ 
  int  score;   			/* the max score in LIST */
  int count;				/* maximum size of list */	
  register  int  i, j;			/* row and column indices */
  int  *S;				/* saving operations for diff */
  int nc, ns, nident;		        /* for display */
  vertexptr cur; 			/* temporary pointer */
  vertexptr findmax();	 		/* return the largest score node */
  double percent;
  

  /* allocate space for consensus */
  i = (min(M,N))*2;
  AJCNEW(seqc0,i);
  AJCNEW(seqc1,i);

  /* allocate space for all vectors */

  j = (N + 1) /* * sizeof(int)*/;
  AJCNEW(CC, j);
  AJCNEW(DD, j);
  AJCNEW(RR, j);
  AJCNEW(SS, j);
  AJCNEW(EE, j);
  AJCNEW(FF, j);

  i = (M + 1) /* * sizeof(int)*/;
  AJCNEW(HH, i);
  AJCNEW(WW, i);
  AJCNEW(II, i);
  AJCNEW(JJ, i);
  AJCNEW(XX, i);
  AJCNEW(YY, i);
  AJCNEW(S,  min(i,j)*5/4);
  AJCNEW0(row, (M + 1));

  /* set up list for each row (already zeroed by AJCNEW0 macro) */
  /* for ( i = 1; i <= M; i++ ) row[i]= PAIRNULL;*/

  /*  vv = *sub[0];*/
  q = Q;
  r = R;
  qr = q + r;

  AJCNEW(LIST, K);
  for ( i = 0; i < K ; i++ ){
    AJNEW0(LIST[i]);
  }

  numnode = lmin = 0;
  big_pass(A,B,M,N,K,nseq);

  /* Report the K best alignments one by one. After each alignment is
     output, recompute part of the matrix. First determine the size
     of the area to be recomputed, then do the recomputation         */
  
  for ( count = K - 1; count >= 0 ; count-- )
    { if ( numnode == 0 )
	ajFatal("The number of alignments computed is too large");
      cur = findmax();	/* Return a pointer to a node with max score*/
      score = cur->SCORE;
      stari = ++cur->STARI;
      starj = ++cur->STARJ;
      endi = cur->ENDI;
      endj = cur->ENDJ;
      m1 = cur->TOP;
      mm = cur->BOT;
      n1 = cur->LEFT;
      nn = cur->RIGHT;
      rl = endi - stari + 1;
      cl = endj - starj + 1;
      I = stari - 1;
      J = starj - 1;
      sapp = S;
      last = 0;
      al_len = 0;
      no_mat = 0;
      no_mis = 0;
      diff(&A[stari]-1, &B[starj]-1,rl,cl,q,q);

      min0 = stari;
      min1 = starj;
      max0 = stari+rl-1;
      max1 = starj+cl-1;
      ns=calcons(A+1,M,B+1,N,S,&nc,&nident);
      percent = (double)nident*100.0/(double)nc;

      if (markx < 10) 
	ajFmtPrintF(outf,
		    "\n %5.1f%% identity in %d %s overlap; score: %4d\n",
		    percent,nc,ajSeqName(seq),score);
      else if (markx==10) {
	ajFmtPrintF(outf,">>#%d\n",K-count);
	ajFmtPrintF(outf,"; sw_score: %d\n",score);
	ajFmtPrintF(outf,"; sw_ident: %5.3f\n",percent/100.0);
	ajFmtPrintF(outf,"; sw_overlap: %d\n",nc);
      }

      discons(seqc0,seqc1,ns);
      if (markx < 10) ajFmtPrintF(outf, "\n----------\n");

      if ( count )
	{ flag = 0;
	  locate(A,B,nseq);
	  if ( flag )
	    small_pass(A,B,count,nseq);
	}
    }
  /* now free all the memory */
  AJFREE(seqc0);
  AJFREE(seqc1);
  AJFREE(CC);
  AJFREE(DD);
  AJFREE(RR);
  AJFREE(SS);
  AJFREE(EE);
  AJFREE(FF);
  AJFREE(HH);
  AJFREE(WW);
  AJFREE(II);
  AJFREE(JJ);
  AJFREE(XX);
  AJFREE(YY);
  AJFREE(S);

  for(i=1; i<ajSeqLen(seq);i++){
    pairptr this,next;
    if(row[i]){
      this = row[i];
      next = this->NEXT;
      while(next){
	AJFREE(this);
	this = next;
	next= this->NEXT;
      }
      AJFREE(this);
    }
  }

  AJFREE(row);
  for( i = 0; i < K ; i++ )
    AJFREE(LIST[i]);
  AJFREE(LIST);
  return 0;
}

/* return 1 if no node in LIST share vertices with the area */
int no_cross()
{ vertexptr  cur;
  register int i;

      for ( i = 0; i < numnode; i++ )
	{ cur = LIST[i];
	  if ( cur->STARI <= mm && cur->STARJ <= nn && cur->BOT >= m1-1 && 
	       cur->RIGHT >= n1-1 && ( cur->STARI < rl || cur->STARJ < cl ))
	     { if ( cur->STARI < rl ) rl = cur->STARI;
	       if ( cur->STARJ < cl ) cl = cur->STARJ;
	       flag = 1;
	       break;
	     }
	}
      if ( i == numnode )
	return 1;
      else
	return 0;
}


int big_pass(A,B,M,N,K,nseq) char A[],B[]; int M,N,K,nseq;
{ 
  register  int  i, j;			/* row and column indices */
  register  int  c;			/* best score at current point */
  register  int  f;			/* best score ending with insertion */
  register  int  d;			/* best score ending with deletion */
  register  int  p;			/* best score at (i-1, j-1) */
  register  int  ci, cj;		/* end-point associated with c */ 
  register  int  di, dj;		/* end-point associated with d */
  register  int  fi, fj;		/* end-point associated with f */
  register  int  pi, pj;		/* end-point associated with p */
  int  *va;				/* pointer to vv(A[i], B[j]) */
  int   addnode();			/* function for inserting a node */
  
	/* Compute the matrix and save the top K best scores in LIST
	   CC : the scores of the current row
	   RR and EE : the starting point that leads to score CC
	   DD : the scores of the current row, ending with deletion
	   SS and FF : the starting point that leads to score DD        */
 	/* Initialize the 0 th row */
  for ( j = 1; j <= N ; j++ )
    {  CC[j] = 0;
    RR[j] = 0;
    EE[j] = j;
    DD[j] = - (q);
    SS[j] = 0;
    FF[j] = j;
    }

  for ( i = 1; i <= M; i++) 
    {  c = 0;				/* Initialize column 0 */
    f = - (q);
    ci = fi = i;
    va = sub[(int)A[i]];
    if ( nseq == 2 )
      { p = 0;
      pi = i - 1;
      cj = fj = pj = 0;
      }
    else
      { p = CC[i];
      pi = RR[i];
      pj = EE[i];
      cj = fj = i;
      }
    for ( j = (nseq == 2 ? 1 : (i+1)) ; j <= N ; j++ )  
      {  f = f - r;
      c = c - qr;
      ORDER(f, fi, fj, c, ci, cj)
		  c = CC[j] - qr; 
      ci = RR[j];
      cj = EE[j];
      d = DD[j] - r;
      di = SS[j];
      dj = FF[j];
      ORDER(d, di, dj, c, ci, cj)
	c = 0;
      DIAG(i, j, c, p+va[(int)B[j]])		/* diagonal */
	/*		    ajDebug("     B[%d]=%d",j,B[j]);*/
	if ( c <= 0 )
	  { c = 0; ci = i; cj = j; }
	else
	  { ci = pi; cj = pj; }
      ORDER(c, ci, cj, d, di, dj)
	ORDER(c, ci, cj, f, fi, fj)
	p = CC[j];
      CC[j] = c;
      pi = RR[j];
      pj = EE[j];
      RR[j] = ci;
      EE[j] = cj;
      DD[j] = d;
      SS[j] = di;
      FF[j] = dj;
      if ( c > lmin )	/* add the score into list */
	lmin = addnode(c, ci, cj, i, j, K, lmin);
      }
    }
  return 0;
}

int locate(A,B,nseq) char A[],B[]; int nseq;
{ register  int  i, j;			/* row and column indices */
  register  int  c;			/* best score at current point */
  register  int  f;			/* best score ending with insertion */
  register  int  d;			/* best score ending with deletion */
  register  int  p;			/* best score at (i-1, j-1) */
  register  int  ci, cj;		/* end-point associated with c */ 
  register  int  di=0, dj=0;		/* end-point associated with d */
  register  int  fi, fj;		/* end-point associated with f */
  register  int  pi, pj;		/* end-point associated with p */
  int  cflag, rflag;			/* for recomputation */
  int  *va;				/* pointer to vv(A[i], B[j]) */
  int   addnode();			/* function for inserting a node */
  int  limit;				/* the bound on j */

	/* Reverse pass
	   rows
	   CC : the scores on the current row
	   RR and EE : the endpoints that lead to CC
	   DD : the deletion scores 
	   SS and FF : the endpoints that lead to DD

	   columns
	   HH : the scores on the current columns
	   II and JJ : the endpoints that lead to HH
	   WW : the deletion scores
	   XX and YY : the endpoints that lead to WW
	*/

  for ( j = nn; j >= n1 ; j-- )
    {  CC[j] = 0;
    EE[j] = j;
    DD[j] = - (q);
    FF[j] = j;
    if ( nseq == 2 || j > mm )
      RR[j] = SS[j] = mm + 1;
    else
      RR[j] = SS[j] = j;
    }
  
  for ( i = mm; i >= m1; i-- )
    {  c = p = 0;
    f = - (q);
    ci = fi = i;
    pi = i + 1;
    cj = fj = pj = nn + 1;
    va = sub[(int)A[i]];
    if ( nseq == 2 || n1 > i )
      limit = n1;
    else
      limit = i + 1;
    for ( j = nn; j >= limit ; j-- )  
      {  f = f - r;
      c = c - qr;
      ORDER(f, fi, fj, c, ci, cj)
	c = CC[j] - qr; 
      ci = RR[j];
      cj = EE[j];
      d = DD[j] - r;
      di = SS[j];
      dj = FF[j];
      ORDER(d, di, dj, c, ci, cj)
	c = 0;
      DIAG(i, j, c, p+va[(int)B[j]])		/* diagonal */
	if ( c <= 0 )
	  { c = 0; ci = i; cj = j; }
	else
	  { ci = pi; cj = pj; }
      ORDER(c, ci, cj, d, di, dj)
	ORDER(c, ci, cj, f, fi, fj)
	p = CC[j];
      CC[j] = c;
      pi = RR[j];
      pj = EE[j];
      RR[j] = ci;
      EE[j] = cj;
      DD[j] = d;
      SS[j] = di;
      FF[j] = dj;
      if ( c > lmin )
	flag = 1;
      }
    if ( nseq == 2 || i < n1 )
      { HH[i] = CC[n1];
      II[i] = RR[n1];
      JJ[i] = EE[n1];
      WW[i] = DD[n1];
      XX[i] = SS[n1];
      YY[i] = FF[n1];
      }
    }
  
  for ( rl = m1, cl = n1; ; )
    { for ( rflag = cflag = 1; ( rflag && m1 > 1 ) || ( cflag && n1 > 1 ) ;  )
      { if ( rflag && m1 > 1 )	/* Compute one row */
	{ rflag = 0;
	m1--;
	c = p = 0;
	f = - (q);
	ci = fi = m1;
	pi = m1 + 1;
	cj = fj = pj = nn + 1;
	va = sub[(int)A[m1]];
	for ( j = nn; j >= n1 ; j-- )  
	  { f = f - r;
	  c = c - qr;
	  ORDER(f, fi, fj, c, ci, cj)
	    c = CC[j] - qr; 
	  ci = RR[j];
	  cj = EE[j];
	  d = DD[j] - r;
	  di = SS[j];
	  dj = FF[j];
	  ORDER(d, di, dj, c, ci, cj)
	    c = 0;
	  DIAG(m1, j, c, p+va[(int)B[j]])		/* diagonal */
	    if ( c <= 0 )
	      { c = 0; ci = m1; cj = j; }
	    else
	      { ci = pi; cj = pj; }
	  ORDER(c, ci, cj, d, di, dj)
	    ORDER(c, ci, cj, f, fi, fj)
	    p = CC[j];
	  CC[j] = c;
	  pi = RR[j];
	  pj = EE[j];
	  RR[j] = ci;
	  EE[j] = cj;
	  DD[j] = d;
	  SS[j] = di;
	  FF[j] = dj;
	  if ( c > lmin )
	    flag = 1;
	  if ( ! rflag && ( (ci > rl && cj > cl) || (di > rl && dj > cl)
			    || (fi > rl && fj > cl )) )
	    rflag = 1;
	  }
	HH[m1] = CC[n1];
	II[m1] = RR[n1];
	JJ[m1] = EE[n1];
	WW[m1] = DD[n1];
	XX[m1] = SS[n1];
	YY[m1] = FF[n1];
	if ( ! cflag && ( (ci > rl && cj > cl) || (di > rl && dj > cl)
			  || (fi > rl && fj > cl) ) )
	  cflag = 1;
	}
      
      if ( nseq == 1 && n1 == (m1 + 1) && ! rflag )
	cflag = 0;
      if ( cflag && n1 > 1 )	/* Compute one column */
	{ cflag = 0;
	n1--;
	c = 0;
	f = - (q);
	cj = fj = n1;
	va = sub[(int)B[n1]];
	if ( nseq == 2 || mm < n1 )
	  { p = 0;
	  ci = fi = pi = mm + 1;
	  pj = n1 + 1;
	  limit = mm;
	  }
	else
	  { p = HH[n1];
	  pi = II[n1];
	  pj = JJ[n1];
	  ci = fi = n1;
	  limit = n1 - 1;
	  }
	for ( i = limit; i >= m1 ; i-- )  
	  { f = f - r;
	  c = c - qr;
	  ORDER(f, fi, fj, c, ci, cj)
	    c = HH[i] - qr; 
	  ci = II[i];
	  cj = JJ[i];
	  d = WW[i] - r;
	  di = XX[i];
	  dj = YY[i];
	  ORDER(d, di, dj, c, ci, cj)
	    c = 0;
	  DIAG(i, n1, c, p+va[(int)A[i]])
	    if ( c <= 0 )
	      { c = 0; ci = i; cj = n1; }
	    else
	      { ci = pi; cj = pj; }
	  ORDER(c, ci, cj, d, di, dj)
	    ORDER(c, ci, cj, f, fi, fj)
	    p = HH[i];
	  HH[i] = c;
	  pi = II[i];
	  pj = JJ[i];
	  II[i] = ci;
	  JJ[i] = cj;
	  WW[i] = d;
	  XX[i] = di;
	  YY[i] = dj;
	  if ( c > lmin )
	    flag = 1;
	  if ( ! cflag && ( (ci > rl && cj > cl) || (di > rl && dj > cl)
			    || (fi > rl && fj > cl) ) )
	    cflag = 1;
	  }
	CC[n1] = HH[m1];
	RR[n1] = II[m1];
	EE[n1] = JJ[m1];
	DD[n1] = WW[m1];
	SS[n1] = XX[m1];
	FF[n1] = YY[m1];
	if ( ! rflag && ( (ci > rl && cj > cl) || (di > rl && dj > cl)
			  || (fi > rl && fj > cl) ) )
	  rflag = 1;
	}
      }
    if ( (m1 == 1 && n1 == 1) || no_cross() )
      break;
    }
  m1--;
  n1--;
  return 0;
}

int small_pass(A,B,count,nseq) char A[], B[]; int count, nseq;
{ register  int  i, j;			/* row and column indices */
  register  int  c;			/* best score at current point */
  register  int  f;			/* best score ending with insertion */
  register  int  d;			/* best score ending with deletion */
  register  int  p;			/* best score at (i-1, j-1) */
  register  int  ci, cj;		/* end-point associated with c */ 
  register  int  di, dj;		/* end-point associated with d */
  register  int  fi, fj;		/* end-point associated with f */
  register  int  pi, pj;		/* end-point associated with p */
  int  *va;				/* pointer to vv(A[i], B[j]) */
  int   addnode();			/* function for inserting a node */
  int  limit;				/* lower bound on j */

  for ( j = n1 + 1; j <= nn ; j++ )
    {  CC[j] = 0;
    RR[j] = m1;
    EE[j] = j;
    DD[j] = - (q);
    SS[j] = m1;
    FF[j] = j;
    }
  for ( i = m1 + 1; i <= mm; i++) 
    {  c = 0;				/* Initialize column 0 */
    f = - (q);
    ci = fi = i;
    va = sub[(int)A[i]];
    if ( nseq == 2 || i <= n1 )
      { p = 0;
      pi = i - 1;
      cj = fj = pj = n1;
      limit = n1 + 1;
      }
    else
      { p = CC[i];
      pi = RR[i];
      pj = EE[i];
      cj = fj = i;
      limit = i + 1;
      }
    for ( j = limit ; j <= nn ; j++ )  
      {  f = f - r;
      c = c - qr;
      ORDER(f, fi, fj, c, ci, cj)
	c = CC[j] - qr; 
      ci = RR[j];
      cj = EE[j];
      d = DD[j] - r;
      di = SS[j];
      dj = FF[j];
      ORDER(d, di, dj, c, ci, cj)
	c = 0;
      DIAG(i, j, c, p+va[(int)B[j]])		/* diagonal */
	if ( c <= 0 )
	  { c = 0; ci = i; cj = j; }
	else
	  { ci = pi; cj = pj; }
      ORDER(c, ci, cj, d, di, dj)
	ORDER(c, ci, cj, f, fi, fj)
	p = CC[j];
      CC[j] = c;
      pi = RR[j];
      pj = EE[j];
      RR[j] = ci;
      EE[j] = cj;
      DD[j] = d;
      SS[j] = di;
      FF[j] = dj;
      if ( c > lmin )	/* add the score into list */
	lmin = addnode(c, ci, cj, i, j, count, lmin);
      }
    }
  return 0;
}

int addnode(c, ci, cj, i, j, K, cost)  int c, ci, cj, i, j, K, cost;
{ int found;				/* 1 if the node is in LIST */
  register int d;

  found = 0;
  if ( most != 0 && most->STARI == ci && most->STARJ == cj )
    found = 1;
  else
     for ( d = 0; d < numnode ; d++ )
	{ most = LIST[d];
	  if ( most->STARI == ci && most->STARJ == cj )
	    { found = 1;
	      break;
	    }
        }
  if ( found )
    { if ( most->SCORE < c )
        { most->SCORE = c;
          most->ENDI = i;
          most->ENDJ = j;
        }
      if ( most->TOP > i ) most->TOP = i;
      if ( most->BOT < i ) most->BOT = i;
      if ( most->LEFT > j ) most->LEFT = j;
      if ( most->RIGHT < j ) most->RIGHT = j;
    }
  else
    { if ( numnode == K )	/* list full */
	 most = low;
      else
         most = LIST[numnode++];
      most->SCORE = c;
      most->STARI = ci;
      most->STARJ = cj;
      most->ENDI = i;
      most->ENDJ = j;
      most->TOP = most->BOT = i;
      most->LEFT = most->RIGHT = j;
    }
  if ( numnode == K )
    { if ( low == most || ! low ) 
        { for ( low = LIST[0], d = 1; d < numnode ; d++ )
            if ( LIST[d]->SCORE < low->SCORE )
              low = LIST[d];
	}
      return ( low->SCORE ) ;
    }
  else
    return cost;
}

vertexptr findmax()
{ vertexptr  cur;
  register int i, j;

  for ( j = 0, i = 1; i < numnode ; i++ )
    if ( LIST[i]->SCORE > LIST[j]->SCORE )
       j = i;
  cur = LIST[j];

  if ( j != --numnode )
    { LIST[j] = LIST[numnode];
      LIST[numnode] =  cur;
    }
  most = LIST[0];
  if ( low == cur ) low = LIST[0];
  return ( cur );

}

int diff(A,B,M,N,tb,te) char *A, *B; int M, N; int tb, te;
{int   midi, midj, type;	/* Midpoint, type, and cost */
  int midc;
  int  zero = 0;				/* int type zero        */

{ register int   i, j;
  register int c, e, d, s;
  int t, *va;

/* Boundary cases: M <= 1 or N == 0 */

  if (N <= 0)
    { if (M > 0) DEL(M)
      return - gap(M);
    }
  if (M <= 1)
    { if (M <= 0)
        { INS(N);
          return - gap(N);
        }
      if (tb > te) tb = te;
      midc = - (tb + r + gap(N) );
      midj = 0;
      va = sub[(int)A[1]];
      for (j = 1; j <= N; j++)
        {  for ( tt = 1, z = row[I+1]; z != PAIRNULL; z = z->NEXT )	
              if ( z->COL == j+J )			
	         { tt = 0; break; }		
           if ( tt )			
            { c = va[(int)B[j]] - ( gap(j-1) + gap(N-j) );
              if (c > midc)
               { midc = c;
                 midj = j;
               }
	    }
	}
      if (midj == 0)
        { INS(N) DEL(1) }
      else
        { if (midj > 1) INS(midj-1)
          REP
	  if ( A[1] == B[midj] )
	     no_mat += 1;
	  else
	     no_mis += 1;
	  /* mark (A[I],B[J]) as used: put J into list row[I] */	
          I++; J++;
	  AJNEW0(z);
          z->COL = J;			
          z->NEXT = row[I];				
	  row[I] = z;
          if (midj < N) INS(N-midj)
        }
      return midc;
    }

/* Divide: Find optimum midpoint (midi,midj) of cost midc */

  midi = M/2;			/* Forward phase:                          */
  CC[0] = 0;			/*   Compute C(M/2,k) & D(M/2,k) for all k */
  t = -q;
  for (j = 1; j <= N; j++)
    { CC[j] = t = t-r;
      DD[j] = t-q;
    }
  t = -tb;
  for (i = 1; i <= midi; i++)
    { s = CC[0];
      CC[0] = c = t = t-r;
      e = t-q;
      va = sub[(int)A[i]];
      for (j = 1; j <= N; j++)
        { if ((c = c - qr) > (e = e - r)) e = c;
          if ((c = CC[j] - qr) > (d = DD[j] - r)) d = c;
	  DIAG(i+I, j+J, c, s+va[(int)B[j]])
          if (c < d) c = d;
          if (c < e) c = e;
          s = CC[j];
          CC[j] = c;
          DD[j] = d;
        }
    }
  DD[0] = CC[0];

  RR[N] = 0;			/* Reverse phase:                          */
  t = -q;			/*   Compute R(M/2,k) & S(M/2,k) for all k */
  for (j = N-1; j >= 0; j--)
    { RR[j] = t = t-r;
      SS[j] = t-q;
    }
  t = -te;
  for (i = M-1; i >= midi; i--)
    { s = RR[N];
      RR[N] = c = t = t-r;
      e = t-q;
      va = sub[(int)A[i+1]];
      for (j = N-1; j >= 0; j--)
        { if ((c = c - qr) > (e = e - r)) e = c;
          if ((c = RR[j] - qr) > (d = SS[j] - r)) d = c;
	  DIAG(i+1+I, j+1+J, c, s+va[(int)B[j+1]])
          if (c < d) c = d;
          if (c < e) c = e;
          s = RR[j];
          RR[j] = c;
          SS[j] = d;
        }
    }
  SS[N] = RR[N];

  midc = CC[0]+RR[0];		/* Find optimal midpoint */
  midj = 0;
  type = 1;
  for (j = 0; j <= N; j++)
    if ((c = CC[j] + RR[j]) >= midc)
      if (c > midc || (CC[j] != DD[j] && RR[j] == SS[j]))
        { midc = c;
          midj = j;
        }
  for (j = N; j >= 0; j--)
    if ((c = DD[j] + SS[j] + q) > midc)
      { midc = c;
        midj = j;
        type = 2;
      }
}

/* Conquer: recursively around midpoint */

  if (type == 1)
    { diff(A,B,midi,midj,tb,q);
      diff(A+midi,B+midj,M-midi,N-midj,q,te);
    }
  else
    { diff(A,B,midi-1,midj,tb,zero);
      DEL(2);
      diff(A+midi+1,B+midj,M-midi-1,N-midj,zero,te);
    }
  return midc;

}

int calcons(aa0,n0,aa1,n1,res,nc,nident)
     char *aa0, *aa1;
     int n0, n1;
     int *res;
     int *nc;
     int *nident;
{
  int i0, i1;
  int op, nid, lenc, nd;
  char *sp0, *sp1;
  int *rp;
  /*  int mins =0; always 0 so why bother ? */
  char *sq1,*sq2;

  /* first fill in the ends */
  min0--; min1--;

  smin0 = min0;
  smin1 = min1;
  /*  smins = mins;*/

/* now get the middle */

  sp0 = seqc0/*+mins*/;
  sp1 = seqc1/*+mins*/;
  rp = res;
  lenc = nid = op = 0;
  i0 = min0;
  i1 = min1;

  sq1 = ajStrStr(ajSeqStr(seq));
  sq2 = ajStrStr(ajSeqStr(seq2));

  while (i0 < max0 || i1 < max1) {
    if (op == 0 && *rp == 0) {
      op = *rp++;
      *sp0 = sq1[i0++];
      *sp1 = sq2[i1++]; 
      lenc++;
      if (*sp0 == *sp1) nid++;
      /*      else if ((dnaseq==1) && (*sp0=='T' && *sp1=='U') ||
	       (*sp0=='U' && *sp1=='T')) nid++;*/
      sp0++; sp1++;
    }
    else {
      if (op==0) op = *rp++;
      if (op>0) {
	*sp0++ = '-';
	*sp1++ =  sq2[i1++]; 
	op--;
	lenc++;
      }
      else {
	*sp0++ = sq1[i0++];
	*sp1++ = '-';
	op++;
	lenc++;
      }
    }
  }

  *nident = nid;
  *nc = lenc;
/*	now we have the middle, get the right end */
  nd = 0;
  return /*mins+*/lenc+nd;
}

#define MAXOUT 201

int discons(seqc0, seqc1, nc)
     char *seqc0, *seqc1;
     int nc;
{
  char line[3][MAXOUT], cline[2][MAXOUT+10];
  int il, i, lend, loff, il1, il2;
  int del0, del1, ic, ll0, ll1, ll01, cl0, cl1, rl0, rl1;
  int i00, i0n, i10, i1n;
  int ioff0, ioff1;
  long qqoff, lloff;
  int have_res;
  char *name01;
  char *name0= ajSeqName(seq);
  char *name1= ajSeqName(seq2);
  char n0 = ajSeqLen(seq);
  int smark[4] = {-10000,-10000,-10000,-10000}; /* BIT WEIRD THIS */

  if (markx==2) name01=name1; else name01 = "\0";

  i00 = smark[0];
  i0n = smark[1];
  i10 = smark[2];
  i1n = smark[3];
  
  /* (il) smins is always 0 ?? so why bither with this ?? 
  ioff0=smin0-smins;
  ioff1=smin1-smins;
  */
  ioff0=smin0;
  ioff1=smin1;

  if (markx==4) return 0;

  if (markx==3) {
    ajFmtPrintF(outf, ">%s ..\n",name0);
    for (i=0; i<nc; i++) {
      ajFmtPrintF(outf, "%c", seqc0[i]);
      if (i%50 == 49) ajFmtPrintF(outf, "\n");
    }
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, ">%s ..\n",name1);
    for (i=0; i<nc; i++) {
      ajFmtPrintF(outf, "%c", seqc1[i]);
      if (i%50 == 49) ajFmtPrintF(outf, "\n");
    }
    ajFmtPrintF(outf, "\n");
    return 0;
  }

  if (markx==10) {
    ajFmtPrintF(outf, ">%s ..\n",name0);
    ajFmtPrintF(outf, "; sq_len: %d\n",n0);
    /*    ajFmtPrintF(outf, "; sq_type: %c\n",sqtype[0]);*/
    ajFmtPrintF(outf, "; al_start: %d\n",min0+1);
    ajFmtPrintF(outf, "; al_stop: %d\n",max0);
    ajFmtPrintF(outf, "; al_display_start: %d\n",ioff0+1);

    have_res = 0;
    for (i=0; i<nc; i++) {
      if (!have_res && seqc0[i]==' ') ajFmtPrintF(outf, "-");
      else if (seqc0[i]==' ') break;
      else {
	have_res = 1;
	ajFmtPrintF(outf, "%c", seqc0[i]);
      }
      if (i%50 == 49) ajFmtPrintF(outf, "\n");
    }
    if ((i-1)%50!=49 || seqc0[i-1]==' ') ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, ">%s ..\n",name1);
    ajFmtPrintF(outf, "; sq_len: %d\n",n1);
    /*    ajFmtPrintF(outf, "; sq_type: %c\n",sqtype[0]);*/
    ajFmtPrintF(outf, "; al_start: %ld\n",/*loffset+*/(long)min1+1);
    ajFmtPrintF(outf, "; al_stop: %ld\n",/*loffset+*/(long)max1);
    ajFmtPrintF(outf, "; al_display_start: %d\n",/*loffset+*/ioff1+1);

    have_res = 0;
    for (i=0; i<nc; i++) {
      if (!have_res && seqc1[i]==' ') ajFmtPrintF(outf, "-");
      else if (seqc1[i]==' ') break;
      else {
	have_res = 1;
	ajFmtPrintF(outf, "%c", seqc1[i]);
      }
      if (i%50 == 49) ajFmtPrintF(outf, "\n");
    }
    if ((i-1)%50!=49 || seqc1[i-1]==' ') ajFmtPrintF(outf, "\n");
    return 0;
  }

  for (i=0; i<3; i++) memset(line[i],' ',MAXOUT);

  ic = 0; del0=del1=0;
  for (il=0; il<(nc+llen-1)/llen; il++) {
    loff=il*llen;
    lend=min(llen,nc-loff);

    ll0 = NO; ll1 = NO;

    for (i=0; i<2; i++) memset(cline[i],' ',MAXOUT);

    for (i=0; i<lend; i++, ic++,ioff0++,ioff1++) {
      cl0 =  cl1 = rl0 = rl1 = YES;
      if ((line[0][i]=seqc0[ic])=='-') {
	del0++; cl0=rl0=NO;
      }
      if ((line[2][i]=seqc1[ic])=='-') {
	del1++; cl1=rl1=NO;
      }

      if (seqc0[ic]==' ') {del0++; cl0=rl0=NO;}
      else ll0 = YES;
      if (seqc1[ic]==' ') {del1++; cl1=rl1=NO;}
      else ll1 = YES;

      qqoff = ajSeqBegin(seq) - 1 + (long)(ioff0-del0)+seq->Offset;
      if (cl0 && qqoff%10 == 9)  {
	sprintf(&cline[0][i],"%8ld",qqoff+1l);
	cline[0][i+8]=' ';
	rl0 = NO;
      }
      else if (cl0 && qqoff== -1) {
	sprintf(&cline[0][i],"%8ld",0l);
	cline[0][i+8]=' ';
	rl0 = NO;
      }
      else if (rl0 && (qqoff+1)%10 == 0) {
	sprintf(&cline[0][i],"%8ld",qqoff+1);
	cline[0][i+8]=' ';
      }
      
      lloff = ajSeqBegin(seq2)-1 + /*loffset +*/ (long)(ioff1-del1)+seq2->Offset;
      if (cl1 && lloff%10 == 9)  {
	sprintf(&cline[1][i],"%8ld",lloff+1l);
	cline[1][i+8]=' ';
	rl1 = NO;
      }
      else if (cl1 && lloff== -1) {
	sprintf(&cline[1][i],"%8ld",0l);
	cline[1][i+8]=' ';
	rl1 = NO;
      }
      else if (rl1 && (lloff+1)%10 == 0) {
	sprintf(&cline[1][i],"%8ld",lloff+1);
	cline[1][i+8]=' ';
      }
      

      line[1][i] = ' ';
      if (ioff0-del0 >= min0 && ioff0-del0 <= max0) {
	if (toupper((int)line[0][i])==toupper((int)line[2][i]) /*|| (dnaseq && (
	    (toupper(line[0][i])=='T' && toupper(line[2][i])=='U') ||
	    (toupper(line[0][i])=='U' && toupper(line[2][i])=='T')))*/)
	  switch (markx) {
	  case 0: line[1][i]= ':';
	    break;
	  case 1: line[1][i]= ' ';
	    break;
	  case 2: line[1][i]= '.';
	    break;
	  }
	else if (markx==2) line[1][i]=line[2][i];
	else if ((il1 = ajSeqCvtK(cvt, line[0][i])) &&
		 (il2 = ajSeqCvtK(cvt, line[2][i])) &&
		 sub[il1][il2]>= 0)
	    line[1][i]= (markx) ? 'x':'.';
	else if ((il1 = ajSeqCvtK(cvt, line[0][i])) &&
		 (il2 = ajSeqCvtK(cvt, line[2][i])))
	    line[1][i]= (markx) ? 'X':' ';
      }
      else if (markx==2) line[1][i]=line[2][i];

      if (markx==0) {
	if (ioff0-del0 == i00 && ioff1-del1 == i10) {
	  line[1][i]='X';
	  i00 = i10 = -1;
	}
	if (ioff0-del0 == i0n && ioff1-del1 == i1n) {
	  line[1][i]='X';
	  i0n = i1n = -1;
	}
	if ((ioff0-del0 == i00) || (ioff0-del0 == i0n)) {
	  line[1][i]='^';
	  if(ioff0-del0 == i00) i00= -1;
	  else i0n = -1;
	}
	if (ioff1-del1 == i10 || ioff1-del1 == i1n) {
	  line[1][i]='v';
	  if(ioff1-del1 == i10) i10= -1;
	  else i1n = -1;
	}
      }
    }
    
    for (i=0; i<3; i++) {line[i][lend]=0;}
    for (i=0; i<2; i++) {cline[i][lend+7]=0;}
    
    ll01 = ll0&&ll1;
    if (markx==2 && (ll0)) ll1=0;
    ajFmtPrintF(outf,"\n");
    if (ll0) ajFmtPrintF(outf,"%s\n",cline[0]);
    if (ll0) ajFmtPrintF(outf,"%6.6s %s\n",name0,line[0]);
    if (ll01) ajFmtPrintF(outf,"%-6.6s %s\n",name01,line[1]);
    if (ll1) ajFmtPrintF(outf,"%6.6s %s\n",name1,line[2]);
    if (ll1) ajFmtPrintF(outf,"%s\n",cline[1]);
  }
  return 0;
}

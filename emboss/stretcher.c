/* @source stretcher application
**
** STRETCHER calculates a global alignment of two sequences
** version 2.0u
** Please cite: Myers and Miller, CABIOS (1989)
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




/* @macro stretchergap ********************************************************
**
** k-symbol indel score
**
** @param [r] k [ajint] Symbol
** @return [void]
******************************************************************************/

#define stretchergap(k)  ((k) <= 0 ? 0 : g+hh*(k)) /* k-symbol indel score */




static ajint stretcher_Ealign(const char *A, const char *B,
			      const AjPSeq seq0, const AjPSeq seq1, ajint G,
			      ajint H,ajint *S, ajint* NC);
static ajint stretcher_Calcons(const char *aa0, ajint n0,
			       const char *aa1, ajint n1,
			       const ajint* res);
static ajint stretcher_Align(const char *A, const char *B,
			     ajint M, ajint N, ajint tb,
			     ajint te);
static ajint stretcher_CheckScore(const unsigned char *A,
				  const unsigned char *B,
				  const AjPSeq seq0, const AjPSeq seq1,
				  ajint *S,ajint *NC);

static ajint *sapp;				/* Current script append ptr */
static ajint  last;				/* Last script op appended */

                                                /* gap penalties */
static ajint g;
static ajint hh;
static ajint m;					/* g = G, hh = H, m = g+h */




/* @macro STRETCHERDEL ********************************************************
**
** Macro for a "Delete k" operation
**
** @param [r] k [ajint] Undocumented
** @return [void]
******************************************************************************/

#define STRETCHERDEL(k)			\
{ if (last < 0)				\
    last = sapp[-1] -= (k);		\
  else					\
    last = *sapp++ = -(k);		\
}
						/* Append "Insert k" op */




/* @macro STRETCHERINS ********************************************************
**
** Macro for an "Insert k" operation
**
** @param [r] k [ajint] Undocumented
** @return [void]
******************************************************************************/

#define STRETCHERINS(k)			\
{ if (last < 0)				\
    { sapp[-1] = (k); *sapp++ = last; }	\
  else					\
    last = *sapp++ = (k);		\
}




/* @macro STRETCHERREP ********************************************************
**
** Macro for a "Replace" operation
**
** @return [void]
******************************************************************************/

#define STRETCHERREP { last = *sapp++ = 0; } /* Append "Replace" op */


static AjPSeq seq0;
static AjPSeq seq1;

static ajint *CC, *DD;			/* Forward cost-only vectors */
static ajint *RR, *SS;		        /* Reverse cost-only vectors */

static ajint nd;
static ajint *res;
static ajint nres;
static ajint nc;

static AjPMatrix matrix = NULL;

static ajint **sub;
static AjPSeqCvt cvt = NULL;

static char *seqc0;
static char *seqc1;   /* aligned sequences */




/* @prog stretcher ************************************************************
**
** Finds the best global alignment between two sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr aa0str = 0;
    AjPStr aa1str = 0;
    const char *s1;
    const char *s2;
    ajint gdelval;
    ajint ggapval;
    ajint i;
    ajint gscore;
    float percent;
    AjPAlign align   = NULL;
    AjPSeqset seqset = NULL;

    AjPSeq res0 = NULL;
    AjPSeq res1 = NULL;
    ajint beg0;
    ajint beg1;


    embInit("stretcher", argc, argv);

    seq0    = ajAcdGetSeq("asequence");
    seq1    = ajAcdGetSeq("bsequence");
    matrix  = ajAcdGetMatrix("datafile");
    gdelval = ajAcdGetInt("gappenalty");
    ggapval = ajAcdGetInt("gaplength");
    align   = ajAcdGetAlign("outfile");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /*
    ** create sequences indexes. i.e. A->0, B->1 ... Z->25 etc.
    ** This is done so that ajAZToInt has only to be done once for
    ** each residue in the sequence
    */

    ajSeqTrim(seq0);
    ajSeqTrim(seq1);
    beg0 = 1 + ajSeqOffset(seq0);
    beg1 = 1 + ajSeqOffset(seq1);

    ajSeqToUpper(seq0);
    ajSeqToUpper(seq1);

    s1 = ajStrStr(ajSeqStr(seq0));
    s2 = ajStrStr(ajSeqStr(seq1));

    sub = ajMatrixArray(matrix);
    cvt = ajMatrixCvt(matrix);

    /*
    ** ajMatrixSeqNum(matrix, seq0,  &aa0str);
    ** ajMatrixSeqNum(matrix, seq1, &aa1str);
    */

    aa0str = ajStrNewL(2+ajSeqLen(seq0)); /* length + blank + trailing null */
    aa1str = ajStrNewL(2+ajSeqLen(seq1));
    ajStrAppK(&aa0str,' ');
    ajStrAppK(&aa1str,' ');

    for(i=0;i<ajSeqLen(seq0);i++)
	ajStrAppK(&aa0str,(char)ajSeqCvtK(cvt, *s1++));

    for(i=0;i<ajSeqLen(seq1);i++)
	ajStrAppK(&aa1str,ajSeqCvtK(cvt, *s2++));

    AJCNEW(res,   ajSeqLen(seq0)+ajSeqLen(seq1));
    AJCNEW(seqc0, ajSeqLen(seq0)+ajSeqLen(seq1));
    AJCNEW(seqc1, ajSeqLen(seq0)+ajSeqLen(seq1));

    gscore = stretcher_Ealign(ajStrStr(aa0str),ajStrStr(aa1str),
			      seq0, seq1,
			      (gdelval-ggapval),ggapval,res,&nres);

    nc = stretcher_Calcons(ajStrStr(aa0str),ajSeqLen(seq0),ajStrStr(aa1str),
			   ajSeqLen(seq1),res);
    percent = (double)nd*100.0/(double)nc;

/*
    seqset = ajSeqsetNew();
    res0   = ajSeqNewS(seq0);
    res1   = ajSeqNewS(seq1);
    ajSeqReplaceC(res0, seqc0);
    ajSeqReplaceC(res1, seqc1);
    ajSeqsetFromPair(seqset, res0, res1);

    ajAlignDefine(align, seqset);

    ajAlignSetGapI(align, gdelval, ggapval);
    ajAlignSetMatrixInt(align, matrix);
    ajAlignSetRange(align, beg0, beg0+ajSeqLen(seq0),
		    ajSeqLen(seq), ajSeqOffset(seq0),
		    beg1, beg1+ajSeqLen(seq1),
		    ajSeqLen(seq2), ajSeqOffset(seq1));
    ajAlignSetScoreI(align, gscore);
*/

    res0 =  ajSeqNewRangeCI(seqc0, nc, ajSeqOffset(seq0),
			       ajSeqOffend(seq0),
			       ajSeqIsReversed(seq0));
    ajSeqAssUsa(res0, ajSeqGetUsa(seq0));
    ajSeqAssName(res0, ajSeqGetName(seq0));

    res1 =  ajSeqNewRangeCI(seqc1, nc, ajSeqOffset(seq1),
			       ajSeqOffend(seq1),
			       ajSeqIsReversed(seq1));
    ajSeqAssUsa(res1, ajSeqGetUsa(seq1));
    ajSeqAssName(res1, ajSeqGetName(seq1));

    ajAlignDefineSS(align, res0, res1);

    ajAlignSetGapI(align, gdelval, ggapval);
    ajAlignSetScoreI(align, gscore);
    ajAlignSetMatrixInt(align, matrix);
    ajAlignSetStats(align, -1, nc, nd, -1, -1, NULL);

    ajAlignWrite(align);

    ajAlignClose(align);

    ajSeqsetDel(&seqset);
    ajAlignDel(&align);


    AJFREE(res);
    AJFREE(seqc0);
    AJFREE(seqc1);
    AJFREE(CC);
    AJFREE(DD);
    AJFREE(RR);
    AJFREE(SS);
    ajStrDel(&aa0str);
    ajStrDel(&aa1str);

    ajSeqDel(&res0);
    ajSeqDel(&res1);


    ajExit();

    return 0;
}




/* Interface and top level of comparator */

static ajint nmax=0;

/* @funcstatic stretcher_Ealign ***********************************************
**
** Undocumented
**
** @param [r] A [const char*] Sequence A with trailing blank
** @param [r] B [const char*] Sequence B with trailing blank
** @param [r] seq0 [const AjPSeq] Sequence A
** @param [r] seq1 [const AjPSeq] Sequence B
** @param [r] G [ajint] Gap penalty (minus extension penalty)
** @param [r] H [ajint] Gap extension penalty
** @param [w] S [ajint*] Result
** @param [w] NC [ajint*] Alignment length returned
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_Ealign(const char *A,const char *B,
			      const AjPSeq seq0, const AjPSeq seq1, ajint G,
			      ajint H,ajint *S,ajint *NC)
{
    ajint c;
    ajint ck;

    ajint M = ajSeqLen(seq0);
    ajint N = ajSeqLen(seq1);

    /*  if(N > NMAX) return -1;*/	/* Error check */

    /* Setup global parameters */
    g    = G;
    hh   = H;
    m    = g+hh;
    sapp = S;
    last = 0;

    if(CC==NULL)
    {
	nmax = ajSeqLen(seq1);
	AJCNEW(CC, nmax+1);
	AJCNEW(DD, nmax+1);
	AJCNEW(RR, nmax+1);
	AJCNEW(SS, nmax+1);
    }
    else if(N > nmax)
    {
	nmax = ajSeqLen(seq1);
	AJCRESIZE(CC, nmax+1);
	AJCRESIZE(DD, nmax+1);
	AJCRESIZE(RR, nmax+1);
	AJCRESIZE(SS, nmax+1);
    }

    if(CC==NULL || DD==NULL || RR==NULL || SS==NULL)
    {
	ajErr(" cannot allocate llmax arrays\n");
	exit(1);
    }

    c  = stretcher_Align(A,B,M,N,-g,-g);	/* OK, do it */
    ck = stretcher_CheckScore((unsigned char *)A,(unsigned char *)B,
                              seq0, seq1,S,NC);

    if(c != ck)
	ajWarn("stretcher CheckScore failed");

    return c;
}




/* @funcstatic stretcher_Align ************************************************
**
** align(A,B,M,N,tb,te) returns the cost of an optimum conversion between
** A[1..M] and B[1..N] that begins(ends) with a delete if tb(te) is zero
** and appends such a conversion to the current script.
**
** @param [r] A [const char*] Undocumented
** @param [r] B [const char*] Undocumented
** @param [r] M [ajint] Undocumented
** @param [r] N [ajint] Undocumented
** @param [r] tb [ajint] Undocumented
** @param [r] te [ajint] Undocumented
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_Align(const char *A,const char *B,
			     ajint M,ajint N,ajint tb,ajint te)
{
    ajint midi;
    ajint midj;
    ajint type;				/* Midpoint, type, and cost */
    ajint midc;
    ajint c1;
    ajint c2;


    register ajint i;
    register ajint j;
    register ajint c;
    register ajint e;
    register ajint d;
    register ajint s;
    ajint t;
    ajint *wa;

    /* Boundary cases: M <= 1 or N == 0 */

    if(N <= 0)
    {
	if(M > 0)
	    STRETCHERDEL(M);

	return -stretchergap(M);
    }

    if(M <= 1)
    {
	if(M <= 0)
	{
	    STRETCHERINS(N)
	    return -stretchergap(N);
	}

	if(tb < te)
	    tb = te;

	midc = (tb-hh) - stretchergap(N);
	midj = 0;
	wa = sub[(ajint)A[1]];
	for(j = 1; j <= N; j++)
        {
	    c = -stretchergap(j-1) + wa[(ajint)B[j]] - stretchergap(N-j);

	    if(c > midc)
            {
		midc = c;
		midj = j;
            }
        }

	if(midj == 0)
        {
	    STRETCHERINS(N) STRETCHERDEL(1)
	}
	else
        {
	    if(midj > 1)
		STRETCHERINS(midj-1)
		    STRETCHERREP
			if(midj < N)
			    STRETCHERINS(N-midj)
        }

	return midc;
    }

    /* Divide: Find optimum midpoint (midi,midj) of cost midc */

    midi  = M/2;	 /* Forward phase:                          */
    CC[0] = 0;		 /*   Compute C(M/2,k) & D(M/2,k) for all k */
    t     = -g;
    for(j = 1; j <= N; j++)
    {
	CC[j] = t = t-hh;
	DD[j] = t-g;
    }
    t = tb;

    for(i = 1; i <= midi; i++)
    {
	s = CC[0];
	CC[0] = c = t = t-hh;
	e = t-g;
	wa = sub[(ajint)A[i]];
	for(j = 1; j <= N; j++)
        {
	    if((c =   c   - m) > (e =   e   - hh))
		e = c;
	    if((c = CC[j] - m) > (d = DD[j] - hh))
		d = c;
	    c = s + wa[(ajint)B[j]];

	    if(e > c)
		c = e;

	    if(d > c)
		c = d;

	    s = CC[j];
	    CC[j] = c;
	    DD[j] = d;
        }
    }
    DD[0] = CC[0];

    RR[N] = 0;		 /* Reverse phase:                          */
    t = -g;		 /*   Compute R(M/2,k) & S(M/2,k) for all k */
    for(j = N-1; j >= 0; j--)
    {
	RR[j] = t = t-hh;
	SS[j] = t-g;
    }
    t = te;

    for(i = M-1; i >= midi; i--)
    {
	s = RR[N];
	RR[N] = c = t = t-hh;
	e = t-g;
	wa = sub[(ajint)A[i+1]];
	for(j = N-1; j >= 0; j--)
        {
	    if((c =   c   - m) > (e =   e   - hh))
		e = c;

	    if((c = RR[j] - m) > (d = SS[j] - hh))
		d = c;
	    c = s + wa[(ajint)B[j+1]];

	    if(e > c)
		c = e;

	    if(d > c)
		c = d;

	    s = RR[j];
	    RR[j] = c;
	    SS[j] = d;
        }
    }
    SS[N] = RR[N];

    midc = CC[0]+RR[0];			/* Find optimal midpoint */
    midj = 0;
    type = 1;
    for(j = 0; j <= N; j++)
	if((c = CC[j] + RR[j]) >= midc)
	    if(c > midc || (CC[j] != DD[j] && RR[j] == SS[j]))
	    {
		midc = c;
		midj = j;
	    }

    for(j = N; j >= 0; j--)
	if((c = DD[j] + SS[j] + g) > midc)
	{
	    midc = c;
	    midj = j;
	    type = 2;
	}


    /* Conquer: recursively around midpoint */

    if(type == 1)
    {
	c1 = stretcher_Align(A,B,midi,midj,tb,-g);
	c2 = stretcher_Align(A+midi,B+midj,M-midi,N-midj,-g,te);
	ajDebug("c1=%d, c2 = %d\n",c1,c2);
    }
    else
    {
	stretcher_Align(A,B,midi-1,midj,tb,0);
	STRETCHERDEL(2);
	stretcher_Align(A+midi+1,B+midj,M-midi-1,N-midj,0,te);
    }

    return midc;
}




/* @funcstatic stretcher_Calcons **********************************************
**
** Undocumented
**
** @param [r] aa0 [const char*] Undocumented
** @param [r] n0 [ajint] Undocumented
** @param [r] aa1 [const char*] Undocumented
** @param [r] n1 [ajint] Undocumented
** @param [r] res [const ajint*] Undocumented
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_Calcons(const char *aa0,ajint n0,
			       const char *aa1,ajint n1,
			       const ajint *res)
{
    ajint i0;
    ajint i1;
    ajint op;
    ajint nc;
    char *sp0;
    char *sp1;
    const ajint *rp;
    const char *sq1;
    const char *sq2;

    int min0;
    int min1;
    int max0;
    int max1;

    sp0 = seqc0;
    sp1 = seqc1;
    rp  = res;
    nc = nd = i0 = i1 = op = 0;
    min0 = min1 = 0;

    sq1 = ajStrStr(ajSeqStr(seq0));
    sq2 = ajStrStr(ajSeqStr(seq1));

    while(i0 < n0 || i1 < n1)
    {
	if(op == 0 && *rp == 0)
	{
	    op = *rp++;
	    *sp0 = sq1[i0++];
	    *sp1 = sq2[i1++];
	    nc++;

	    if(*sp0++ == *sp1++)
		nd++;
	}
	else
	{
	    if(op==0)
		op = *rp++;

	    if(op>0)
	    {
		*sp0++ = '-';
		*sp1++ = sq2[i1++];
		op--;
		nc++;
	    }
	    else
	    {
		*sp0++ = sq1[i0++];
		*sp1++ = '-';
		op++;
		nc++;
	    }
	}
    }

    max0 = max1 = nc;
    ajDebug("calcons donenc %d min0 %d min1 %d  max0 %d max1 %d\n",
	     nc, min0, min1, max0, max1);

    return nc;
}




/* @funcstatic stretcher_CheckScore *******************************************
**
** return the score of the alignment stored in S
**
** @param [r] A [const unsigned char*] Undocumented
** @param [r] B [const unsigned char*] Undocumented
** @param [r] seq0 [const AjPSeq] Sequence A
** @param [r] seq1 [const AjPSeq] Sequence B
** @param [w] S [ajint*] Undocumented
** @param [w] NC [ajint*] Alignment length returned
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_CheckScore(const unsigned char *A,
				  const unsigned char *B,
				  const AjPSeq seq0, const AjPSeq seq1,
                                  ajint *S,ajint *NC)
{
    register ajint i;
    register ajint j;
    register ajint op;
    register ajint nc1;
    ajint score;

    score = i = j = op = nc1 = 0;
    while(i < ajSeqLen(seq0) || j < ajSeqLen(seq1))
    {
	op = *S++;
	if(op == 0)
	{
	    score = sub[A[++i]][B[++j]] + score;
	    nc1++;
	}
	else if(op > 0)
	{
	    score = score - (g+op*hh);
	    j = j+op;
	    nc1 += op;
	}
	else
	{
	    score = score - (g-op*hh);
	    i = i-op;
	    nc1 -= op;
	}
    }

    *NC = nc1;

    return(score);
}

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
			      ajint M, ajint N, ajint G,
			      ajint H,ajint *S, ajint* NC);
static ajint stretcher_Calcons(const char *aa0, ajint n0,
			       const char *aa1, ajint n1,
			       const ajint* res);
static ajint stretcher_Discons(const char* seqc0, const char *seqc1, ajint nc);
static ajint stretcher_Align(const char *A, const char *B,
			     ajint M, ajint N, ajint tb,
			     ajint te);
static ajint stretcher_CheckScore(const unsigned char *A,
				  const unsigned char *B,
				  ajint M,
				   ajint N,ajint *S,ajint *NC);


static ajint markx;                               /* what to display ? */

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


static AjPSeq seq;
static AjPSeq seq2;

static ajint *CC, *DD;			/* Forward cost-only vectors */
static ajint *RR, *SS;		        /* Reverse cost-only vectors */

static AjPFile outf = NULL;

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

    AjPSeq res1 = NULL;
    AjPSeq res2 = NULL;
    ajint beg;
    ajint beg2;

    embInit("stretcher", argc, argv);

    seq     = ajAcdGetSeq("asequence");
    seq2    = ajAcdGetSeq("bsequence");
    matrix  = ajAcdGetMatrix("datafile");
    gdelval = ajAcdGetInt("gappenalty");
    ggapval = ajAcdGetInt("gaplength");
    align   = ajAcdGetAlign("outfile");

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */
    /* llen = ajAcdGetInt("length"); */ /* use awidth */
    /* markx = ajAcdGetInt("markx"); */ /* use aformat markx0 */

    /*
    ** create sequences indexes. i.e. A->0, B->1 ... Z->25 etc.
    ** This is done so that ajAZToInt has only to be done once for
    ** each residue in the sequence
    */

    ajSeqTrim(seq);
    ajSeqTrim(seq2);
    beg  = 1 + ajSeqOffset(seq);
    beg2 = 1 + ajSeqOffset(seq2);

    ajSeqToUpper(seq);
    ajSeqToUpper(seq2);

    s1 = ajStrStr(ajSeqStr(seq));
    s2 = ajStrStr(ajSeqStr(seq2));

    sub = ajMatrixArray(matrix);
    cvt = ajMatrixCvt(matrix);

    /*
    ** ajMatrixSeqNum(matrix, seq,  &aa0str);
    ** ajMatrixSeqNum(matrix, seq2, &aa1str);
    */

    aa0str = ajStrNewL(2+ajSeqLen(seq)); /* length + blank + trailing null */
    aa1str = ajStrNewL(2+ajSeqLen(seq2));
    ajStrAppK(&aa0str,' ');
    ajStrAppK(&aa1str,' ');

    for(i=0;i<ajSeqLen(seq);i++)
	ajStrAppK(&aa0str,(char)ajSeqCvtK(cvt, *s1++));

    for(i=0;i<ajSeqLen(seq2);i++)
	ajStrAppK(&aa1str,ajSeqCvtK(cvt, *s2++));

    AJCNEW(res,   ajSeqLen(seq)+ajSeqLen(seq2));
    AJCNEW(seqc0, ajSeqLen(seq)+ajSeqLen(seq2));
    AJCNEW(seqc1, ajSeqLen(seq)+ajSeqLen(seq2));

    gscore = stretcher_Ealign(ajStrStr(aa0str),ajStrStr(aa1str),
			      ajSeqLen(seq),ajSeqLen(seq2),
			      (gdelval-ggapval),ggapval,res,&nres);

    nc = stretcher_Calcons(ajStrStr(aa0str),ajSeqLen(seq),ajStrStr(aa1str),
			   ajSeqLen(seq2),res);
    percent = (double)nd*100.0/(double)nc;

    if(outf)
    {
	ajFmtPrintF(outf,"%-50s %4d  vs.\n%-50s %4d \n",
		    ajSeqName(seq),ajSeqLen(seq),
		    ajSeqName(seq2),ajSeqLen(seq2));
	ajFmtPrintF(outf,"scoring matrix: %S, gap penalties: %d/%d\n",
		    ajMatrixName(matrix),gdelval,ggapval);
	ajFmtPrintF(outf,"%4.1f%% identity;\t\tGlobal alignment score: %d\n",
		    percent,gscore);
    }

    if(outf)
	stretcher_Discons(seqc0,seqc1,nc);

    seqset = ajSeqsetNew();
    res1   = ajSeqNewS(seq);
    res2   = ajSeqNewS(seq2);
    ajSeqReplaceC(res1, seqc0);
    ajSeqReplaceC(res2, seqc1);
    ajSeqsetFromPair(seqset, res1, res2);

    ajAlignDefine(align, seqset);

    ajAlignSetGapI(align, gdelval, ggapval);
    ajAlignSetMatrixInt(align, matrix);
    ajAlignSetRange(align, beg, beg+ajSeqLen(seq), beg2, beg2+ajSeqLen(seq2));
    ajAlignSetScoreI(align, gscore);

    ajAlignWrite(align);

    if(outf)
	ajFileClose(&outf);

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

    ajSeqDel(&res1);
    ajSeqDel(&res2);


    ajExit();

    return 0;
}




/* Interface and top level of comparator */

static ajint nmax=0;

/* @funcstatic stretcher_Ealign ***********************************************
**
** Undocumented
**
** @param [r] A [const char*] Undocumented
** @param [r] B [const char*] Undocumented
** @param [r] M [ajint] Undocumented
** @param [r] N [ajint] Undocumented
** @param [r] G [ajint] Undocumented
** @param [r] H [ajint] Undocumented
** @param [w] S [ajint*] Undocumented
** @param [w] NC [ajint*] Undocumented
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_Ealign(const char *A,const char *B,
			      ajint M,ajint N,ajint G,
			      ajint H,ajint *S,ajint *NC)
{
    ajint c;
    ajint ck;

    /*  if(N > NMAX) return -1;*/	/* Error check */

    /* Setup global parameters */
    g    = G;
    hh   = H;
    m    = g+hh;
    sapp = S;
    last = 0;

    if(CC==NULL)
    {
	nmax = N;
	AJCNEW(CC, nmax+1);
	AJCNEW(DD, nmax+1);
	AJCNEW(RR, nmax+1);
	AJCNEW(SS, nmax+1);
    }
    else if(N > nmax)
    {
	nmax = N;
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
    ck = stretcher_CheckScore((unsigned char *)A,(unsigned char *)B,M,N,S,NC);

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

    sq1 = ajStrStr(ajSeqStr(seq));
    sq2 = ajStrStr(ajSeqStr(seq2));

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




/* @funcstatic stretcher_Discons **********************************************
**
** Undocumented
**
** @param [r] seqc0 [const char*] Undocumented
** @param [r] seqc1 [const char*] Undocumented
** @param [r] nc [ajint] Undocumented
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_Discons(const char *seqc0, const char *seqc1, ajint nc)
{
#define MAXOUT 201

    static ajint smin0;
    static ajint  smin1;

    static ajint min0;
    static ajint min1;
    static ajint max0;
    static ajint max1;

    static ajint llen;

#define YES 1
#define NO 0

    static AjPSeqCvt cvt = NULL;
    static ajint **sub;

    char line[3][MAXOUT];
    char cline[2][MAXOUT+10];
    ajint il;
    ajint i;
    ajint lend;
    ajint loff;
    ajint il1;
    ajint il2;
    ajint del0;
    ajint del1;
    ajint ic;
    ajint ll0;
    ajint ll1;
    ajint ll01;
    ajint cl0;
    ajint cl1;
    ajint rl0;
    ajint rl1;
    ajint i00;
    ajint i0n;
    ajint i10;
    ajint i1n;
    ajint ioff0;
    ajint ioff1;
    ajlong qqoff;
    ajint lloff;
    ajint have_res;
    const char *name01;
    const char *name0;
    const char *name1;
    ajint n0;
    ajint smark[4] =
    {
	-10000,-10000,-10000,-10000
    }; /* BIT WEIRD THIS */


    name0 = ajSeqName(seq);
    name1 = ajSeqName(seq2);
    n0    = ajSeqLen(seq);

    if(markx==2)
	name01 = name1;
    else
	name01 = "\0";

    i00 = smark[0];
    i0n = smark[1];
    i10 = smark[2];
    i1n = smark[3];

    /*
    ** (il) smins is always 0 ?? so why bother with this ??
    ** ioff0=smin0-smins;
    ** ioff1=smin1-smins;
    */

    ioff0 = smin0;
    ioff1 = smin1;

    if(markx==4)
	return 0;

    if(markx==3)
    {
	ajFmtPrintF(outf,">%s ..\n",name0);
	for(i=0; i<nc; i++)
	{
	    ajFmtPrintF(outf, "%c",seqc0[i]);
	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}
	ajFmtPrintF(outf, "\n");
	ajFmtPrintF(outf,">%s ..\n",name1);

	for(i=0; i<nc; i++)
	{
	    ajFmtPrintF(outf, "%c", seqc1[i]);
	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}

	ajFmtPrintF(outf, "\n");

	return 0;
    }

    if(markx==10)
    {
	ajFmtPrintF(outf,">%s ..\n",name0);
	ajFmtPrintF(outf,"; sq_len: %d\n",n0);
	/*    ajFmtPrintF(outf,"; sq_type: %c\n",sqtype[0]);*/
	ajFmtPrintF(outf,"; al_start: %d\n",min0+1);
	ajFmtPrintF(outf,"; al_stop: %d\n",max0);
	ajFmtPrintF(outf,"; al_display_start: %d\n",ioff0+1);

	have_res = 0;
	for(i=0; i<nc; i++)
	{
	    if(!have_res && seqc0[i]==' ')
		ajFmtPrintF(outf, "-");
	    else if(seqc0[i]==' ')
		break;
	    else
	    {
		have_res = 1;
		ajFmtPrintF(outf, "%c", seqc0[i]);
	    }

	    if(i%50 == 49)
		ajFmtPrintF(outf, "-");
	}

	if((i-1)%50!=49 || seqc0[i-1]==' ')
	    ajFmtPrintF(outf, "-");

	ajFmtPrintF(outf,"\n>%s ..\n",name1);
	ajFmtPrintF(outf,"; sq_len: %d\n",ajSeqLen(seq2));
	/*    ajFmtPrintF(outf,"; sq_type: %c\n",sqtype[0]);*/
	ajFmtPrintF(outf,"; al_start: %ld\n", /*loffset+*/(ajlong)min1+1);
	ajFmtPrintF(outf,"; al_stop: %ld\n", /*loffset+*/(ajlong)max1);
	ajFmtPrintF(outf,"; al_display_start: %d\n", /*loffset+*/ioff1+1);

	have_res = 0;
	for(i=0; i<nc; i++)
	{
	    if(!have_res && seqc1[i]==' ')
		ajFmtPrintF(outf, "-");
	    else if(seqc1[i]==' ')
		break;
	    else
	    {
		have_res = 1;
		ajFmtPrintF(outf, "%c", seqc1[i]);
	    }

	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}

	if((i-1)%50!=49 || seqc1[i-1]==' ')
	    ajFmtPrintF(outf, "\n");

	return 0;
    }

    for(i=0; i<3; i++)
	memset(line[i],' ',MAXOUT);

    ic = 0;
    del0 = del1 = 0;
    for(il=0; il<(nc+llen-1)/llen; il++)
    {
	loff=il*llen;
	lend=AJMIN(llen,nc-loff);

	ll0 = NO;
	ll1 = NO;

	for(i=0; i<2; i++)
	    memset(cline[i],' ',MAXOUT);

	for(i=0; i<lend; i++, ic++,ioff0++,ioff1++)
	{
	    cl0 =  cl1 = rl0 = rl1 = YES;
	    if((line[0][i]=seqc0[ic])=='-')
	    {
		del0++;
		cl0 = rl0 = NO;
	    }

	    if((line[2][i]=seqc1[ic])=='-')
	    {
		del1++;
		cl1 = rl1 = NO;
	    }

	    if(seqc0[ic]==' ')
	    {
		del0++;
		cl0 = rl0 = NO;
	    }
	    else
		ll0 = YES;

	    if(seqc1[ic]==' ')
	    {
		del1++;
		cl1 = rl1 = NO;
	    }
	    else
		ll1 = YES;

	    qqoff = ajSeqBegin(seq) - 1 + (ajlong)(ioff0-del0);
	    if(cl0 && qqoff%10 == 9)
	    {
		sprintf(&cline[0][i],"%8ld",(long)qqoff+1l);
		cline[0][i+8] = ' ';
		rl0 = NO;
	    }
	    else if(cl0 && qqoff== -1)
	    {
		sprintf(&cline[0][i],"%8ld",0l);
		cline[0][i+8] = ' ';
		rl0 = NO;
	    }
	    else if(rl0 && (qqoff+1)%10 == 0)
	    {
		sprintf(&cline[0][i],"%8ld",(long)qqoff+1);
		cline[0][i+8] = ' ';
	    }

	    lloff = ajSeqBegin(seq2)-1 + /*loffset +*/ (ajlong)(ioff1-del1);
	    if(cl1 && lloff%10 == 9)
	    {
		sprintf(&cline[1][i],"%8ld",(long)lloff+1l);
		cline[1][i+8] = ' ';
		rl1 = NO;
	    }
	    else if(cl1 && lloff== -1)
	    {
		sprintf(&cline[1][i],"%8ld",0l);
		cline[1][i+8] = ' ';
		rl1 = NO;
	    }
	    else if(rl1 && (lloff+1)%10 == 0)
	    {
		sprintf(&cline[1][i],"%8ld",(long)lloff+1);
		cline[1][i+8] = ' ';
	    }


	    line[1][i] = ' ';
	    if(ioff0-del0 >= min0 && ioff0-del0 <= max0)
	    {
		if(toupper((ajint)line[0][i])==toupper((ajint)line[2][i]))
		    switch(markx)
		    {
		    case 0:
			line[1][i] = ':';
			break;
		    case 1:
			line[1][i] = ' ';
			break;
		    case 2:
			line[1][i] = '.';
			break;
		    }
		else if(markx==2)
		    line[1][i] = line[2][i];
		else if((il1 = ajSeqCvtK(cvt, line[0][i])) &&
			(il2 = ajSeqCvtK(cvt, line[2][i])) &&
			sub[il1][il2]>= 0)
		    line[1][i] = (markx) ? 'x':'.';
		else if((il1 = ajSeqCvtK(cvt, line[0][i])) &&
			(il2 = ajSeqCvtK(cvt, line[2][i])))
		    line[1][i] = (markx) ? 'X':' ';
	    }
	    else if(markx==2)
		line[1][i] = line[2][i];

	    if(markx==0)
	    {
		if(ioff0-del0 == i00 && ioff1-del1 == i10)
		{
		    line[1][i] = 'X';
		    i00 = i10 = -1;
		}

		if(ioff0-del0 == i0n && ioff1-del1 == i1n)
		{
		    line[1][i] = 'X';
		    i0n = i1n = -1;
		}

		if((ioff0-del0 == i00) || (ioff0-del0 == i0n))
		{
		    line[1][i] = '^';
		    if(ioff0-del0 == i00)
			i00 = -1;
		    else
			i0n = -1;
		}

		if(ioff1-del1 == i10 || ioff1-del1 == i1n)
		{
		    line[1][i] = 'v';
		    if(ioff1-del1 == i10)
			i10= -1;
		    else
			i1n = -1;
		}
	    }
	}

	for(i=0; i<3; i++)
	    line[i][lend] = 0;

	for(i=0; i<2; i++)
	    cline[i][lend+7] = 0;

	ll01 = ll0&&ll1;

	if(markx==2 && (ll0))
	    ll1=0;
	ajFmtPrintF(outf,"\n");

	if(ll0)
	    ajFmtPrintF(outf,"%s\n",cline[0]);

	if(ll0)
	    ajFmtPrintF(outf,"%6.6s %s\n",name0,line[0]);

	if(ll01)
	    ajFmtPrintF(outf,"%-6.6s %s\n",name01,line[1]);

	if(ll1)
	    ajFmtPrintF(outf,"%6.6s %s\n",name1,line[2]);

	if(ll1)
	    ajFmtPrintF(outf,"%s\n",cline[1]);
    }

    return 0;
}




/* @funcstatic stretcher_CheckScore *******************************************
**
** return the score of the alignment stored in S
**
** @param [r] A [const unsigned char*] Undocumented
** @param [r] B [const unsigned char*] Undocumented
** @param [r] M [ajint] Undocumented
** @param [r] N [ajint] Undocumented
** @param [w] S [ajint*] Undocumented
** @param [w] NC [ajint*] Undocumented
** @return [ajint] Undocumented
******************************************************************************/

static ajint stretcher_CheckScore(const unsigned char *A,
				  const unsigned char *B,
				  ajint M,
				  ajint N,ajint *S,ajint *NC)
{
    register ajint i;
    register ajint j;
    register ajint op;
    register ajint nc1;
    ajint score;

    score = i = j = op = nc1 = 0;
    while(i < M || j < N)
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






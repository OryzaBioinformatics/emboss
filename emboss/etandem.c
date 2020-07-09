/* @source etandem application
**
** Tandem searches for tandem repeats
** @author: Copyright (C) Richard Durbin (rd@sanger.ac.uk)
** and Jean Thierry-Mieg 1992
** @@
** The original application is part of the ACEDB genome database
** package, written by ** Richard Durbin (MRC LMB, UK)
** rd@mrc-lmba.cam.ac.uk, and Jean Thierry-Mieg (CRBM du CNRS,
** France) mieg@crbm1.cnusc.fr
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

#define DEBUG 0

#define NMAX 1000

#include "emboss.h"

typedef struct cons {
  ajint phase ;
  ajint *tab ;
  ajint *max ;
  ajint start ;
  ajint score ;
  ajint bestScore ;
  ajint ibest ;
  ajint *bestMax ;
  ajint repeat ; 
  struct cons *next ;
} *Cons ;

AjPFile outfile;
static struct cons rootStruct ;
static Cons root = &rootStruct ;
static ajint *ring ;
static char letter[5] = "acgtn" ;
static AjBool mismatch = AJFALSE ;
static AjBool uniform = AJFALSE ;
static ajint thresh = 20 ;
static ajint nbase;
static ajint nmin, nmax;
static AjPSeqCvt cvt;

static Cons consCreate (void);
static void consDestroy (Cons cons);
static void basicReport (Cons a);
static void report (Cons a);
static void finalReport (void);

#define ATAB(x,y) (a->tab[x+5*y])

/***************************/

static ajint nCons = 0 ;

static Cons consCreate (void) {
  static Cons res;

  AJNEW0(res);
  AJCNEW0(res->max, nmax+1);
  AJCNEW0(res->bestMax, nmax+1);
  AJCNEW0(res->tab, 5*nmax+5);
  ++nCons ;
  return res ;
}

static void consDestroy (Cons cons) {
  if (!cons)
    return ;
  --nCons ;
  AJFREE (cons->max);
  AJFREE (cons->bestMax);
  AJFREE (cons->tab);
  AJFREE (cons) ;
}

/***************** reporting code *****************/

static struct cons reportRootStruct ;
static Cons reportRoot = &reportRootStruct ;

static void basicReport (Cons a) {
  ajint j, copies, n = a->repeat ;
  float perc ;

  ajDebug("basicReport\n");

  copies = (a->ibest - a->start + 1) / n ;
  perc = 100.0 * (a->bestScore + n * (copies + 1)) / (2.0 * n * copies) ;
  ajFmtPrintF (outfile, "%6d %10d %10d %2d %3d %5.1f ",
	  a->bestScore, a->start+1, a->ibest+1, 
	  n, copies, perc) ;
  for (j = (a->phase+1) % n ; j < n ; ++j) {
      ajDebug("      bestMax[%d] letter[%d] '%c'\n",
	      j, a->bestMax[j], letter[a->bestMax[j]]);
    ajFmtPrintF (outfile, "%c", letter[a->bestMax[j]]) ;
  }
  if ((a->phase+1) % n) {
    for (j = 0 ; j <= a->phase ; ++j) {
      ajDebug("more: bestMax[%d] letter[%d] '%c'\n",
	      j, a->bestMax[j], letter[a->bestMax[j]]);
       ajFmtPrintF (outfile, "%c", letter[a->bestMax[j]]) ;
    }
  }
   ajFmtPrintF (outfile, "\n") ;
}

static void report (Cons a) { 
  ajint j ;
  ajint firstchar ;
  
  if (a->bestScore >= thresh) {
    if (uniform)
      goto good ;
				/* else check not a single letter pattern */
    firstchar = a->bestMax[0] ;
    for (j = 1 ; j < a->repeat ; j++) {
      if (a->bestMax[j] != firstchar)
	goto good ;
    }
  }

  consDestroy (a) ;		/* don't destroy, since reporting repeatedly */
  return ;

 good:
  a->next = reportRoot->next ;
  reportRoot->next = a ;
  return ;
}

static void finalReport (void) {
  ajint start, end ; 
  Cons a, top, olda ;

  ajDebug ("finalReport\n");
  while (reportRoot->next) {	/* find top score */
    top = reportRoot ;
    for (a = reportRoot->next ; a ; a = a->next) {
      if (a->bestScore > top->bestScore ||
	  (a->bestScore == top->bestScore && a->repeat < top->repeat))
	top = a ;
    }
				/* report that */
    basicReport (top) ;
				/* destroy all overlapping entries, including self  */
    start = top->start ; end = top->ibest ;
    olda = reportRoot ;
    for (a = olda->next ; a ; olda = a, a = a->next) {
      if (a->ibest >= start && a->start <= end) {
        olda->next = a->next ;
	consDestroy (a) ;
	a = olda ;
      }
    }
  }
}

/**************************/

int main(int argc, char **argv)
{

  ajint ibase, base;
  char *cp ;
  AjPSeq sequence = NULL ;
  ajint i, j, x, phase, n ;
  Cons new, a, b, olda, oldb ;
  AjPStr nseq = NULL;

  embInit ("etandem", argc, argv);

  nmin = ajAcdGetInt("minrepeat");
  nmax = ajAcdGetInt("maxrepeat");
  mismatch = ajAcdGetBool ("mismatch");
  thresh = ajAcdGetInt("threshold");
  uniform = ajAcdGetBool("uniform");
  outfile = ajAcdGetOutfile ("outfile");
  sequence = ajAcdGetSeq ("sequence");
  nbase = ajSeqLen(sequence);

  cvt = ajSeqCvtNewZero ("ACGTN");
  ajSeqNum (sequence, cvt, &nseq);

  AJCNEW(ring, nbase);

  reportRoot->bestScore = thresh - 1 ;

  for (n = nmin ; n <= nmax ; ++n) {
    cp = ajStrStr(nseq);
    for (ibase = 0 ; ibase < nbase ; ++ibase, ++cp) {
      base = *cp - 1;

      /* set up local ring */
      phase = ibase % n ;
      ring[phase] = base ;

      if (ibase < n-1)
	continue ;

      /* start new Cons */
      new = consCreate () ;
      new->start = ibase - n + 1 ;
      new->next = root->next ;
      new->score = new->bestScore = -n ;
      new->phase = phase ;	/* phase of the last base of pattern */
      new->repeat = n ;
      root->next = new ;

      /* add last nmer to active Cons's */
      olda = root ;

      if (DEBUG)
	ajDebug ("%d\n", ibase) ;

      for (a = olda->next ; a ; olda = a, a = a->next) {
	if (a->phase == phase) {
	  for (i = 0 ; i < n ; ++i) {
	    x = ring[i];
	    if (x == 4 && mismatch) {
	      --a->score ;
	      continue ;
	    }
	    ++ATAB(x,i);
	    if (x == a->max[i])
	      ++a->score ;
	    else if (ATAB(x,i) > ATAB(a->max[i],i)) {
	      a->max[i] = x ;
	      ++a->score ;
	    }
	    else {
	      --a->score ;
	      if (ATAB(x,i) == ATAB(a->max[i], i))
		a->max[i] = x ;
	    }
	  }
	  if (a->score > a->bestScore) {
	    a->bestScore = a->score ;
	    a->ibest = ibase ;
	    for (j = 0 ; j < n ; ++j)
	      a->bestMax[j] = a->max[j] ;
	  }
	  else if (a->score < 0) {
	    if (DEBUG) ajDebug ("D") ;
	    olda->next = a->next ;
	    report (a) ;
	    a = olda ;
	  }
	}
      }
      /* remove duplicate max tables */
      olda = root ;
      for (a = olda->next ; a ; olda = a, a = a->next) {
	if (a->phase == phase) {
	  oldb = a ;
	  for (b = a->next ; b ; b = b->next) { /* all phases */
	    for (j = 0 ; j < n ; ++j) {
	      if (a->max[j] != b->max[j])
		goto nextb ;
	    }
	    if (a->bestScore > b->bestScore) { /* remove b */
	      oldb->next = b->next ;
	      if (DEBUG) {
		ajDebug ("B") ;
		basicReport (b) ;
	      }
	      consDestroy (b) ;
	      b = oldb ;
	    }
	    else {		/* remove a */
	      olda->next = a->next ;
	      if (DEBUG) {
		ajDebug ("A") ;
		basicReport (a) ;
	      }
	      consDestroy (a) ;
	      a = olda ;
	      goto nexta ;
	    }
	  nextb:
	    oldb = b ;
	  }
	nexta: 
	  ;
	}
      }
      if (DEBUG) {
        for (a = root->next ; a ; a = a->next)
          basicReport (a) ;
      }
    }

    while ((a = root->next)) {
      root->next = a->next ;
      report (a) ;
    }
  }

  finalReport () ;
  ajStrDel (&nseq);
  ajExit();
  return 0;
}

/******************** end of file ******************/

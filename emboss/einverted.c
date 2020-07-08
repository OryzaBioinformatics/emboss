/*  File: inverted.c
 *  Author: Richard Durbin (rd@mrc-lmba.cam.ac.uk)
 *  Copyright (C) J Thierry-Mieg and R Durbin, 1993
 *-------------------------------------------------------------------
 * This file is part of the ACEDB genome database package, written by
 * 	Richard Durbin (MRC LMB, UK) rd@mrc-lmba.cam.ac.uk, and
 *	Jean Thierry-Mieg (CRBM du CNRS, France) mieg@crbm1.cnusc.fr
 *
 * Description: looks for inverted repeats using dynamic programming.
		Ideas for fast implementation taken from James Crook's
		thesis, fused/replaced by Gene Myers code.
 * Exported functions:
 * HISTORY:
 * Last edited: Feb 24 14:09 2000 (pmr)
 * Created: Sat Jan 23 15:16:29 1993 (rd)
 *-------------------------------------------------------------------
 */

#define MYERS			/* Gene Myers DP code */

#include "emboss.h"

#define MAXSAVE 2000
#define TEST

int match;
int mismatch;
int threshold;
int gap;
AjPFile outfile;
static AjPSeqCvt cvt;
int rogue = 1000000;

static char base[] = "acgt-" ;

char *sq ;
int *revmatch[5] ;
int length ;
int matrix[MAXSAVE][MAXSAVE] ;

void report (int max, int imax) ;

int main (int argc, char **argv)
{
  int i, j, irel, imax, jmax=0, *ip, *t1Base ;
  int lastReported = -1 ;
  char *cp ;
  register int a, c, d, *t0, *t1, max ;
  int localMax[MAXSAVE], back[MAXSAVE] ;

  AjPSeq sequence = NULL ;
  AjPStr nseq = NULL;

  embInit ("einverted", argc, argv);
  outfile = ajAcdGetOutfile ("outfile");
  sequence = ajAcdGetSeq ("sequence");
  threshold = ajAcdGetInt ("threshold");
  match = ajAcdGetInt ("match");
  mismatch = ajAcdGetInt ("mismatch");
  gap = ajAcdGetInt ("gap");

  length = ajSeqLen(sequence);
  cvt = ajSeqCvtNew ("ACGT");
  ajSeqNum (sequence, cvt, &nseq);
  sq = ajStrStr(nseq);

  ajDebug("sequence length: %d\n", length);

  /* build revmatch etc. to be a,t,g,c matched to reverse sequence
     ending in MAXSAVE ROGUE values
  */
  for (i = 5 ; i-- ;)
    { AJCNEW(revmatch[i], (length+MAXSAVE));
      ip = revmatch[i];
      for (j = length ; j-- ; )
	*ip++ = mismatch ;
      for (j = MAXSAVE ; j-- ;)
	*ip++ = rogue ;
    }

  cp = ajStrStr(nseq);
  for (j = length ; j-- ;)	/* reverse order important here */
    switch (*cp++)
      {
      case 0: revmatch[3][j] = match ; break ; /* A */
      case 1: revmatch[2][j] = match ; break ; /* C */
      case 2: revmatch[1][j] = match ; break ; /* G */
      case 3: revmatch[0][j] = match ; break ; /* T */
      }

  for (i = 0; i < MAXSAVE ; i++) back[i] = localMax[i] = 0;

  for (i = 0 ; i < length+MAXSAVE ; ++i) /* +MAXSAVE to report at end */
    {
      irel = i % MAXSAVE ;

      ajDebug ("i: %d irel: %d back[irel] %d\n", i, irel, back[irel]);

      if (back[irel])		/* something to report */
	{ imax = 0 ;
	  for (j = back[irel] ; j > i-MAXSAVE ; --j)
	    if (localMax[j%MAXSAVE] > imax)
	      { jmax = j ;
		imax = localMax[j%MAXSAVE] ;
	      }
	  report (imax, jmax) ;
	  lastReported = jmax ;
	  for (j = jmax ; j >= i-MAXSAVE ; --j)
	    { localMax[j%MAXSAVE] = 0 ;
	      back[j%MAXSAVE] = 0 ;
	    }
	}

      if (i >= length)		/* report only */
	continue ;

      if (i == 0)
	t0 = matrix[MAXSAVE-1] - 1 ; /* NB offset by 1 */
      else
	t0 = matrix[(i-1) % MAXSAVE] - 1 ; /* NB offset by 1 */
      t1 = matrix[irel] ;
      memcpy (t1, &revmatch[(int)sq[i]][length-i], (MAXSAVE-1)*sizeof(int)) ;
      t1[MAXSAVE-2] = t1[MAXSAVE-1] = rogue ;

/* Gene Myers' version of dynamic progamming: 
   a is current *t0, d is diagonal sum, c is working *t1 value 
*/

#ifdef TEST
      ajDebug ("\n%2d %c: ", i, base[(int)sq[i]]) ;
      for (j = length-i ; --j ;)
	ajDebug ("      ") ;
      ajDebug (" ") ;
      if (*t1 > 0)
	ajDebug ("*") ;
      else
	ajDebug (" ") ;
      ajDebug ("%2d  ", *t1) ;
#endif

      max = threshold-1 ;
      jmax = 0 ;
      t1Base = t1 ;

#ifdef MYERS
      c = *t1 ;
      a = -rogue ;
      while (1)			/* inner loop */
	{ d = *++t1 ;
	  if (a > 0) d += a ;
	  a = *++t0 ;
	  if (a > c) c = a ;
	  c -= gap ;
	  if (d > c) c = d ;
#ifdef TEST
	  if (c == d)
	  {
	    if (d == *t1)
	      ajDebug (".") ;
	    else
		ajDebug ("\\") ;
	  }
	  else if (c + gap == a)
	    ajDebug ("|") ;
	  else
	    ajDebug ("-") ;
	  if (*t1 > 0)
	    ajDebug ("*") ;
	  else
	    ajDebug (" ") ;
	  ajDebug ("%2d  ", c) ;
#endif
	  *t1 = c ;
	  if (c > max)
	    { if (c >= rogue)
		goto done ;
	      max = c ; jmax = t1 - t1Base ;
	    }
	}
#endif

    done:
      if (jmax)			/* max was broken */
	{ localMax[irel] = max ;
	  j = (i-jmax-1) % MAXSAVE ;
	  if (i-jmax-1 > lastReported && 
	      (!back[j] || localMax[back[j] % MAXSAVE] < max))
	    back[j] = i ;
	}
      else
	localMax[irel] = 0 ;
/*
      if (!((i+1) % 1000))
	ajDebug ("%d", i+1) ;
*/
    }

  ajExit();
  return 0;
}

void report (int max, int imax)
{
  int *t1, *ip, *jp, i, j ;
  static int align1[2*MAXSAVE], align2[2*MAXSAVE] ;
  int nmatch = 0, nmis = 0, ngap = 0 ;
  int saveMax = max ;

  ajDebug ("report (%d %d)\n", max, imax);

				/* reconstruct maximum path */
  t1 = matrix[imax % MAXSAVE] ;
  for (j = 0 ; j < MAXSAVE ; ++j)
    if (t1[j] == max)
      break ;
  i = imax ;
  ip = align1 ; jp = align2 ;
  while (max > 0 && j >= 0)	/* original missed blunt joins */
    { *ip++ = i ;
      *jp++ = i-j ;		/* seqpt + 1 */
#ifdef TEST
      ajDebug ("i j, max (local): %d %d, %4d (%2d)\n", 
	      i, j, max, revmatch[(int)sq[i]][length-i+j]) ;
#endif
      if (t1[j-1] == max + gap)
	{ max += gap ; ++ngap ;
	  --j ; continue ;
	}
      t1 = matrix[(i-1) % MAXSAVE] ;
      if (t1[j-1] == max + gap)
	{ max += gap ; ++ngap ;
	  --i ; --j ; continue ;
	}
      max -= revmatch[(int)sq[i]][length-i+j] ;
      if (revmatch[(int)sq[i]][length-i+j] == match)
	++nmatch ;
      else
	++nmis ;
      --i ; j-=2 ;
    }
  *ip = *jp = 0 ;
#ifdef TEST
  ajDebug ("\n") ;
#endif
				/* report reconstruction */

  ajFmtPrintF (outfile, "\nScore %d: %d/%d (%3d%%) matches, %d gaps\n", 
	  saveMax, nmatch, (nmatch+nmis), 
	  (100*nmatch)/(nmatch+nmis), ngap) ;

  ajFmtPrintF (outfile, "%8d ", *align2) ;	/* NB *jp is 1+coord */
  for (jp = align2 ; *jp ; ++jp)
    if (*jp == *(jp+1))
      ajFmtPrintF (outfile, "-") ;
    else
      ajFmtPrintF (outfile, "%c", base[(int)sq[*jp-1]]) ;
  ajFmtPrintF (outfile, " %-8d\n", *(jp-1)) ;

  ajFmtPrintF (outfile, "         ") ;
  for (ip = align1, jp = align2 ; *ip ; ++ip, ++jp)
    if (*ip == *(ip+1) || *jp == *(jp+1))
      ajFmtPrintF (outfile, " ") ;
    else if (sq[*ip] + sq[*jp-1] == 3) /* pmr: was 1 or 5 */
      ajFmtPrintF (outfile, "|") ;
    else
      ajFmtPrintF (outfile, " ") ;
  ajFmtPrintF (outfile, "\n") ;

  ajFmtPrintF (outfile, "%8d ", *align1 + 1) ;
  for (ip = align1 ; *ip ; ++ip)
    if (*ip == *(ip+1))
      ajFmtPrintF (outfile, "-") ;
    else
      ajFmtPrintF (outfile, "%c", base[(int)sq[*ip]]) ;
  ajFmtPrintF (outfile, " %-8d\n", *(ip-1)+1) ;
}

/************* end of file ************/

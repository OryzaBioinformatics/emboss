/*  File: quicktandem.c
 *  Author: Richard Durbin (rd@mrc-lmba.cam.ac.uk)
 *  Copyright (C) J Thierry-Mieg and R Durbin, 1993
 *-------------------------------------------------------------------
 * This file is part of the ACEDB genome database package, written by
 * 	Richard Durbin (MRC LMB, UK) rd@mrc-lmba.cam.ac.uk, and
 *	Jean Thierry-Mieg (CRBM du CNRS, France) mieg@crbm1.cnusc.fr
 *
 * Description: to prescreen sequences for tandem repeats.  Use
 		tandem on anything that looks significant.
 * Exported functions:
 * HISTORY:
 * Created: Tue Jan 19 21:25:59 1993 (rd)
 *-------------------------------------------------------------------
 */

#include "emboss.h"

AjPFile outfile;
static AjPSeqCvt cvt;

static void report (AjPFile outf);
char *back, *front, *maxback=NULL, *maxfront=NULL;
char* sq;
int gap, max, score ;

int main (int argc, char **argv)
{
  int thresh;
  int maxrepeat;
  AjPSeq sequence = NULL ;
  AjPStr tseq = NULL;

  embInit ("equicktandem", argc, argv);
  outfile = ajAcdGetOutfile ("outfile");
  sequence = ajAcdGetSeq ("sequence");
  thresh = ajAcdGetInt ("threshold");
  maxrepeat = ajAcdGetInt ("maxrepeat");

  cvt = ajSeqCvtNewText ("ACGTN");
  ajSeqNum (sequence, cvt, &tseq);
  sq = ajStrStr(tseq);

  for (gap = 1 ; gap <= maxrepeat ; ++gap)
    { back = sq ; front = back + gap ;
      score = max = 0 ;
      while (*front)
      { if (*front == 'Z')
	{
	    if (max >= thresh)
	      { report (outfile);
		back = maxfront ; front = back + gap ;
		score = max = 0 ;
	      }
	    else
	      { back = front ; front = back + gap ;
		score = max = 0 ;
	    }
	 }
	  else if (*front != *back)
	    --score ;
	  else if (score <= 0)
	  {
	    if (max >= thresh)
	      { report (outfile);
		back = maxfront ; front = back + gap ;
		score = max = 0 ;
	      }
	    else
	      { maxback = back ;
		score = 1 ;
	    }
	  }
	  
	  else if (++score > max)
	    { max = score ;
	      maxfront = front ;
	    }
	  ++back ; ++front ;
	}

      if (max >= thresh)
	report (outfile);
    }
  ajExit();
  return 0;
}

static void report (AjPFile outf) {
  char* cp;
  ajFmtPrintF (outf, "%6d %10d %10d %2d %3d\n",
	       max, 1+maxback-sq, 1+maxfront-sq,
	       gap, (maxfront-maxback+1)/gap) ;
  for (cp = maxback ; cp <= maxfront ; ++cp)
    *cp = 'Z' ;
}

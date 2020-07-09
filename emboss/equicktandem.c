/* @source equicktandem application
**
** Quick tandem repeat finder
**
** @author: Copyright (C) Richard Durbin, J Thierry-Mieg
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

static AjPFile outfile;
static AjPSeqCvt cvt;

static void equicktandem_report (AjPFile outf, ajint begin);

static char *back;
static char *front;
static char *maxback=NULL;
static char *maxfront=NULL;
char* sq;
ajint gap;
ajint max;
ajint score;


/* @prog equicktandem *********************************************************
**
** Finds tandem repeats
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint thresh;
    ajint maxrepeat;
    AjPSeq sequence = NULL ;
    AjPStr tseq = NULL;
    AjPStr str = NULL;
    AjPStr substr = NULL;
  
    ajint  begin;
    ajint  end;
    ajint  len;
    
    embInit ("equicktandem", argc, argv);
    outfile = ajAcdGetOutfile ("outfile");
    sequence = ajAcdGetSeq ("sequence");
    thresh = ajAcdGetInt ("threshold");
    maxrepeat = ajAcdGetInt ("maxrepeat");

    begin = ajSeqBegin(sequence) - 1;
    end   = ajSeqEnd(sequence) - 1;

    substr = ajStrNew();
    str = ajSeqStrCopy(sequence);
    ajStrAssSub(&substr,str,begin,end);
    ajSeqReplace(sequence,substr);

    cvt = ajSeqCvtNewText ("ACGTN");
    ajSeqNum (sequence, cvt, &tseq);
    sq = ajStrStr(tseq);

    /* careful - sequence can be shgorter than the maximum repeat length */

    if ((len=ajStrLen(substr)) < maxrepeat)
      maxrepeat = ajStrLen(substr);

    for (gap = 1 ; gap <= maxrepeat ; ++gap)
    {
	back = sq ; front = back + gap;
	score = max = 0 ;
	while (front-sq<=len)
	{
	    if (*front == 'Z')
	    {
		if (max >= thresh)
		{
		    equicktandem_report (outfile, begin);
		    back = maxfront ; front = back + gap ;
		    score = max = 0 ;
		}
		else
		{
		    back = front ; front = back + gap ;
		    score = max = 0 ;
		}
	    }
	    else if (*front != *back)
		--score ;
	    else if (score <= 0)
	    {
		if (max >= thresh)
		{
		    equicktandem_report (outfile, begin);
		    back = maxfront ; front = back + gap ;
		    score = max = 0 ;
		}
		else
		{
		    maxback = back ;
		    score = 1 ;
		}
	    }
	  
	    else if (++score > max)
	    {
		max = score ;
		maxfront = front ;
	    }
	    ++back ; ++front ;
	}

	if (max >= thresh)
	    equicktandem_report (outfile, begin);
    }

    ajStrDel(&str);
    ajStrDel(&substr);

    ajExit();
    return 0;
}




/* @funcstatic equicktandem_report *******************************************
**
** Undocumented.
**
** @param [?] outf [AjPFile] Undocumented
** @param [?] begin [ajint] Undocumented
** @@
******************************************************************************/

static void equicktandem_report (AjPFile outf, ajint begin)
{
    char* cp;

    ajFmtPrintF (outf, "%6d %10d %10d %2d %3d\n",
		 max, 1+maxback-sq+begin, 1+maxfront-sq+begin,
		 gap, (maxfront-maxback+1)/gap) ;
    for (cp = maxback ; cp <= maxfront ; ++cp)
	*cp = 'Z' ;

    return;
}

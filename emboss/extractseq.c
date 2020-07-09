/* @source extractseq application
**
** Extract regions from a sequence
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** Fri Apr 16 16:47:32 BST 1999 (ajb)
** 7 Sept 1999 - GWW rewrote to use ajRange routines.
** 15 March 2000 - GWW added '-separate' option
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


/* @prog extractseq ***********************************************************
**
** Extract regions from a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    AjPRange regions;
    AjPStr newstr=NULL;
    AjBool separate;
    AjPList strlist;
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    AjPStr str;
    AjPStr name = NULL;		/* new name of the sequence */
    AjPStr value = NULL;	/* string value of start or end position */
    AjPSeq newseq = NULL;

    (void) embInit ("extractseq", argc, argv);

    seqout = ajAcdGetSeqoutall ("outseq");
    seqall = ajAcdGetSeqall ("sequence");
    regions = ajAcdGetRange("regions");
    separate = ajAcdGetBool("separate");

    /* (void) ajRangeBegin (regions, ajSeqallBegin(seqall)); */

    while (ajSeqallNext(seqall, &seq))
    {
	/* are we writing each region out to a separate sequence? */
	if (separate)
	{
	    strlist = ajListstrNew();
	    (void) ajRangeStrExtractList (strlist, regions, ajSeqStr(seq));
	    nr = ajRangeNumber(regions);
	    for(i=0; i<nr; i++)
	    {
		(void) ajRangeValues(regions, i, &st, &en);
		(void) ajListstrPop (strlist, &str);

		/* new sequence */
		newseq = ajSeqNew ();

		/* create a nice name for the new sequence */        
		(void) ajStrAss(&name, ajSeqGetName(seq));
		(void) ajStrAppC(&name, "_");
		(void) ajStrFromInt(&value, st);
		(void) ajStrApp(&name, value);
		(void) ajStrAppC(&name, "_");
		(void) ajStrFromInt(&value, en);
		(void) ajStrApp(&name, value);
		ajSeqAssName(newseq, name);

		/* set the sequence description */
		ajSeqAssDesc(newseq, ajSeqGetDesc(seq));

		/* set the extracted sequence */
		ajSeqReplace (newseq, str);

		/* set the type */
		if (ajSeqIsNuc(seq))
		    ajSeqSetNuc (newseq);
		else
		    ajSeqSetProt (newseq);


		/* write this region of the sequence */
		(void) ajSeqAllWrite (seqout, newseq);

		/* tidy up */        
		(void) ajStrDel(&name);
		(void) ajStrDel(&value);
		(void) ajStrDel(&str);
		(void) ajSeqDel(&newseq);
	    }

	    ajListstrFree(&strlist);

	}
	else
	{
	    /*
	     *  concatenate all regions from the sequence into the same
	     *  sequence
	     */
	    (void) ajRangeStrExtract (&newstr, regions, ajSeqStr(seq));
	    (void) ajSeqReplace(seq, newstr);
	    (void) ajStrClear(&newstr);
	    (void) ajSeqAllWrite (seqout, seq);
	}
    	
    }
  
    ajSeqWriteClose (seqout);

    ajExit ();
    return 0;
}

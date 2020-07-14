/* @source extractseq application
**
** Extract regions from a sequence
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** Fri Apr 16 16:47:32 BST 1999 (ajb)
** 7 Sept 1999 - GWW rewrote to use ajRange routines.
** 15 March 2000 - GWW added '-separate' option
** 22 May 2002 - GWW changed to only read one sequence, not a seqall
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
    AjPSeq seq;
    AjPSeqout seqout;
    AjPRange regions;
    AjPStr newstr = NULL;
    AjBool separate;
    AjPList strlist;
    ajint nr;
    ajint i;
    ajint st;
    ajint en;
    AjPStr str;
    AjPStr name   = NULL;		/* new name of the sequence */
    AjPStr value  = NULL;  /* string value of start or end position */
    AjPSeq newseq = NULL;

    embInit("extractseq", argc, argv);

    seq      = ajAcdGetSeq("sequence");
    regions  = ajAcdGetRange("regions");
    separate = ajAcdGetBool("separate");
    seqout   = ajAcdGetSeqoutall("outseq");


    /* Writing each region out to a separate sequence? */
    if(separate)
    {
	strlist = ajListstrNew();
	ajRangeStrExtractList(regions, ajSeqStr(seq), strlist);
	nr = ajRangeNumber(regions);
	for(i=0; i<nr; i++)
	{
	    ajRangeValues(regions, i, &st, &en);
	    ajListstrPop(strlist, &str);

	    /* new sequence */
	    newseq = ajSeqNew();

	    /* create a name for the new sequence */
	    ajStrAssS(&name, ajSeqGetName(seq));
	    ajStrAppC(&name, "_");
	    ajStrFromInt(&value, st);
	    ajStrApp(&name, value);
	    ajStrAppC(&name, "_");
	    ajStrFromInt(&value, en);
	    ajStrApp(&name, value);
	    ajSeqAssName(newseq, name);

	    /* set the sequence description */
	    ajSeqAssDesc(newseq, ajSeqGetDesc(seq));

	    /* set the extracted sequence */
	    ajSeqReplace(newseq, str);

	    /* set the type */
	    if(ajSeqIsNuc(seq))
		ajSeqSetNuc(newseq);
	    else
		ajSeqSetProt(newseq);

	    /* write this region of the sequence */
	    ajSeqAllWrite(seqout, newseq);

	    ajStrDel(&name);
	    ajStrDel(&value);
	    ajStrDel(&str);
	    ajSeqDel(&newseq);
	}

	ajListstrFree(&strlist);

    }
    else
    {
	/*
	**  concatenate all regions from the sequence into the same
	**  sequence
	*/
	ajRangeStrExtract(regions, ajSeqStr(seq), &newstr);
	ajSeqReplace(seq, newstr);
	ajStrClear(&newstr);
	ajSeqAllWrite(seqout, seq);
    }


    ajSeqWriteClose(seqout);

    ajExit();

    return 0;
}

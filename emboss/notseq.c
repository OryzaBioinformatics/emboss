/* @source notseq  application
**
** Excludes a set of sequences and writes out the remaining ones
**
** @author: Copyright (C) Gary Williams
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


/* @prog notseq ***************************************************************
**
** Excludes a set of sequences and writes out the remaining ones
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeqout junkout;
    AjPSeq seq = NULL;
    AjPStr pattern = NULL;
    AjPStr name=NULL;
    AjPStr acc=NULL;
    AjBool gotone=ajFalse;

    embInit ("notseq", argc, argv);

    seqout = ajAcdGetSeqoutall ("outseq");
    junkout = ajAcdGetSeqoutall ("junkout");
    seqall = ajAcdGetSeqall ("sequence");
    pattern = ajAcdGetString ("exclude");

    while (ajSeqallNext(seqall, &seq))
    {
	(void) ajStrAss(&name, ajSeqGetName(seq));
	(void) ajStrAss(&acc, ajSeqGetAcc(seq));
      
	if (embMiscMatchPattern(name, pattern) ||
	    embMiscMatchPattern(acc, pattern))
	{
	    ajSeqAllWrite (junkout, seq);    	
	    gotone = ajTrue;
	}
	else
	    /* no match, so not excluded */
	    ajSeqAllWrite (seqout, seq);

	ajStrClear(&name);
	ajStrClear(&acc);
    }

    ajSeqWriteClose (seqout);
    ajSeqWriteClose (junkout);

    if (gotone)
	ajExit ();
    else
    {
	ajWarn("No matches found.");
	ajExitBad ();
    }

    return 0;
}

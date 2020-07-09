/* @source shuffleseq application
**
** Randomises sequences maintaining composition
** @author: Copyright (C) Michael Schmitz (mschmitz@lbl.gov)
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




/* @prog shuffleseq ***********************************************************
**
** Shuffles a set of sequences maintaining composition
**
******************************************************************************/

int main(int argc, char **argv)
{

    ajint shuffles=1;
    ajint n;
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    AjPStr seq_str;
    
    embInit ("shuffleseq", argc, argv);

    ajRandomSeed();

    seqout = ajAcdGetSeqoutall ("outseq");
    seqall = ajAcdGetSeqall ("sequence");

    shuffles = ajAcdGetInt("shuffle");

    while (ajSeqallNext(seqall, &seq))
    {
	for(n=0;n<shuffles;++n)
	{
	    /* get a COPY of the sequence string object */
	    seq_str = ajSeqStrCopy(seq);
	    /* shuffle it */
	    ajStrRandom(&seq_str);
	    /* stuff back the string */
	    ajSeqReplace (seq, seq_str);

	    ajSeqAllWrite (seqout, seq);

	    ajStrDel(&seq_str);
	}
    }

    ajSeqWriteClose (seqout);

    ajExit ();
    return 0;
}


/* @source degapseq application
**
** Remove gaps from sequences
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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




/* @prog degapseq *************************************************************
**
** Remove gaps from a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq = NULL;
    AjPStr str = NULL;

    embInit("degapseq", argc, argv);

    seqout = ajAcdGetSeqoutall ("outseq");
    seqall = ajAcdGetSeqall ("sequence");

    while(ajSeqallNext(seqall, &seq))
    {
	/* get a copy of the sequence string */
	str = ajStrNew();
	ajStrAssignS(&str, ajSeqGetSeqS(seq));

	ajStrRemoveGap(&str);
	ajSeqAssignSeqS(seq, str);
	ajStrDel(&str);

	ajSeqAllWrite(seqout, seq);
    }

    ajSeqWriteClose(seqout);

    ajSeqallDel(&seqall);
    ajSeqDel(&seq);
    ajSeqoutDel(&seqout);

    ajExit();
    return 0;
}


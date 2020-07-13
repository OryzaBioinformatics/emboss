/* @source newseq application
**
** Type in a short new sequence
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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


/* @prog newseq ***************************************************************
**
** Type in a short new sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqout seqout;
    AjPSeq seq = NULL;
    AjPStr name = NULL;
    AjPStr desc = NULL;
    AjPStr sequence = NULL;
    AjPStr *type;

    (void) embInit ("newseq", argc, argv);

    seqout   = ajAcdGetSeqout ("outseq");
    name     = ajAcdGetString ("name");
    desc     = ajAcdGetString ("description");
    sequence = ajAcdGetString ("sequence");
    type     = ajAcdGetList   ("type");

    /* initialise the sequence */
    seq = ajSeqNewL(ajStrLen(sequence));

    /* assign some things to the sequence */
    (void) ajSeqAssName(seq, name);
    (void) ajSeqAssDesc(seq, desc);
    if (!ajStrCmpC(type[0], "N"))
	(void) ajSeqSetNuc(seq);
    else
	(void) ajSeqSetProt(seq);

    (void) ajSeqReplace(seq, sequence);

    (void) ajSeqWrite (seqout, seq);
    (void) ajSeqWriteClose (seqout);

    ajExit ();
    return 0;
}

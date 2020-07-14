/* @source descseq application
**
** Alter the name or description of a sequence
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




/* @prog descseq **************************************************************
**
** Alter the name or description of a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqout seqout;
    AjPSeq seq;
    AjPStr name = NULL;
    AjPStr desc = NULL;
    AjPStr temp = NULL;
    AjBool append;

    embInit("descseq", argc, argv);

    seqout = ajAcdGetSeqout("outseq");
    seq    = ajAcdGetSeq("sequence");
    append = ajAcdGetBool("append");
    name   = ajAcdGetString("name");
    desc   = ajAcdGetString("description");


    /* if appending, then do this */
    if(append)
    {
	/* is there a name? */
	if(ajStrLen(name))
	{
	    ajStrAssS(&temp, ajSeqGetName(seq));
	    ajStrApp(&temp, name);
	    ajSeqAssName(seq, temp);
	}

	/* is there a description? */
	if(ajStrLen(desc))
	{
	    ajStrAssS(&temp, ajSeqGetDesc(seq));
	    ajStrApp(&temp, desc);
	    ajSeqAssDesc(seq, temp);
	}

	/* otherwise, just overwrite the existing values */
    }
    else
    {
	/* is there a name? */
	if(ajStrLen(name))
	    ajSeqAssName(seq, name);

	/* is there a description? */
	if(ajStrLen(desc))
	    ajSeqAssDesc(seq, desc);
    }

    ajSeqWrite(seqout, seq);
    ajSeqWriteClose(seqout);

    ajExit();

    return 0;
}

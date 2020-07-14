/* @source cons application
**
** Calculates a consensus
** @author: Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
**
** -plurality  	- defines no. of +ve scoring matches below
**                which there is no consensus.
**
** -identity   	- defines the number of identical symbols
**                requires in an alignment column for it to
**                included in the consensus.
**
** -setcase   	- upper/lower case given if score above/below
**                user defined +ve matching threshold.
**
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




/* @prog cons *****************************************************************
**
** Creates a consensus from multiple alignments
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint   nseqs;
    ajint   mlen;
    ajint   i;
    ajint   identity;
    float fplural;
    float setcase;
    const char  *p;
    AjPSeqset seqset;
    AjPSeqout seqout;
    AjPSeq    seqo;
    AjPStr    name = NULL;
    AjPStr    cons;
    AjPMatrix cmpmatrix = 0;


    embInit ("cons", argc, argv);

    seqset    = ajAcdGetSeqset ("sequence");
    cmpmatrix = ajAcdGetMatrix("datafile");
    fplural   = ajAcdGetFloat("plurality");
    setcase   = ajAcdGetFloat("setcase");
    identity  = ajAcdGetInt("identity");
    seqout    = ajAcdGetSeqout("outseq");
    name      = ajAcdGetString ("name");

    nseqs = ajSeqsetSize(seqset);
    if(nseqs<2)
	ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetLen(seqset);
    for(i=0;i<nseqs;++i)	/* check sequences are same length */
    {
	p = ajSeqsetSeq(seqset,i);
	if(strlen(p)!=mlen)
	{
	    ajWarn("Sequence lengths are not equal!");
	    break;
	}
    }

    ajSeqsetToUpper(seqset);

    cons = ajStrNew();
    embConsCalc (seqset, cmpmatrix, nseqs, mlen,
		 fplural, setcase, identity, &cons);

    /* write out consensus sequence */
    seqo = ajSeqNew();
    ajSeqAssSeq(seqo,cons);
    if(name == NULL)
	ajSeqAssName(seqo,ajSeqsetGetName(seqset));
    else
	ajSeqAssName(seqo,name);

    ajSeqWrite(seqout,seqo);

    ajStrDel(&cons);
    ajSeqDel(&seqo);

    ajExit ();

    return 0;
}

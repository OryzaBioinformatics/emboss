/* @source tranalign application
**
** Align nucleic sequences guided by the alignment of the translation
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




static void tranalign_AddGaps (AjPSeq newseq, AjPSeq nseq, AjPSeq pseq,
	ajint npos);




/* @prog tranalign ************************************************************
**
** Align nucleic sequences guided by the alignment of the translation
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall nucseq;	/* input nucleic sequences */
    AjPSeqset protseq;	/* input aligned protein sequences */
    AjPSeqout seqout;
    AjPSeq nseq;	/* next nucleic sequence to align */
    AjPSeq pseq;	/* next protein sequence use in alignmnet */
    AjPTrn trnTable;
    AjPSeq pep;		/* translation of nseq */
    AjPStr *tablelist;
    ajint table;
    AjPSeqset outseqset; /* set of aligned nucleic sequences */
    ajint proteinseqcount = 0;
    AjPStr degapstr = NULL;
    ajint pos = 0;       /* start position of guide protein in translation */
    AjPSeq newseq = NULL;	/* output aligned nucleic sequence */
    ajint frame;

    embInit("tranalign", argc, argv);

    nucseq    = ajAcdGetSeqall("asequence");
    protseq   = ajAcdGetSeqset("bsequence");
    tablelist = ajAcdGetList("table");
    seqout    = ajAcdGetSeqoutset("outseq");

    outseqset = ajSeqsetNew();
    degapstr  = ajStrNew();

    /* initialise the translation table */
    ajStrToInt(tablelist[0], &table);
    trnTable = ajTrnNewI(table);

    ajSeqsetFill(protseq);

    while(ajSeqallNext(nucseq, &nseq))
    {
    	if((pseq = ajSeqsetGetSeq(protseq, proteinseqcount++)) == NULL)
    	    ajErr("No guide protein sequence available for "
		  "nucleic sequence %S",
		  ajSeqGetName(nseq));

	ajDebug("Aligning %S and %S\n",
		ajSeqGetName(nseq), ajSeqGetName(pseq));

        /* get copy of pseq string with no gaps */
        ajStrAssS(&degapstr, ajSeqStr(pseq));
        ajStrDegap(&degapstr);

        /*
	** for each translation frame look for subset of pep that
	** matches pseq
	*/
        for(frame = 1; frame <4; frame++)
	{
	    ajDebug("trying frame %d\n", frame);
            pep = ajTrnSeqOrig(trnTable, nseq, frame);
            pos = ajStrFindCase(ajSeqStr(pep), degapstr);
            ajSeqDel(&pep);

            if(pos != -1)
            	break;
        }

        if(pos == -1)
	    ajErr("Guide protein sequence %S not found in nucleic sequence %S",
		  ajSeqGetName(pseq), ajSeqGetName(nseq));
	else
	{
	    ajDebug("got a match with frame=%d\n", frame);
            /* extract the coding region of nseq with gaps */
            newseq = ajSeqNew();
            ajSeqSetNuc(newseq);
            ajSeqAssName(newseq, ajSeqGetName(nseq));
            ajSeqAssDesc(newseq, ajSeqGetDesc(nseq));
            tranalign_AddGaps(newseq, nseq, pseq, (pos*3)+frame-1);

            /* output the gapped nucleic sequence */
            ajSeqsetApp(outseqset, newseq);

            ajSeqDel(&newseq);
        }

        ajStrClean(&degapstr);
    }

    ajSeqsetWrite(seqout, outseqset);
    ajSeqWriteClose(seqout);

    ajTrnDel(&trnTable);
    ajSeqsetDel(&outseqset);
    ajStrDel(&degapstr);

    ajExit();

    return 0;
}




/* @funcstatic tranalign_AddGaps **********************************************
**
** Adds bases or gaps to newstr from nseq guided by pseq
** starting at npos
**
** @param [r] newseq [AjPSeq] newseq
** @param [r] nseq [AjPSeq] nseq
** @param [r] pseq [AjPSeq] pseq
** @param [r] npos [ajint] nseq start pos
** @return [void]
** @@
******************************************************************************/

static void tranalign_AddGaps(AjPSeq newseq, AjPSeq nseq, AjPSeq pseq,
			      ajint npos)
{

    AjPStr newstr = NULL;
    ajint ppos = 0;

    newstr = ajStrNew();

    for(; ppos<ajSeqLen(pseq); ppos++)
    	if(ajSeqChar(pseq)[ppos] == '-')
    	    ajStrAppC(&newstr, "---");
	else
	{
    	    ajStrAppSub(&newstr, ajSeqStr(nseq), npos, npos+2);
    	    npos+=3;
    	}

    ajDebug("aligned seq=%S\n", newstr);
    ajSeqReplace(newseq, newstr);

    ajStrDel(&newstr);

    return;
}

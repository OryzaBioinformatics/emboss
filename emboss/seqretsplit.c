/* @source seqretsplit application
**
** Read and write sequences as individual files
**
** @author: Copyright (C) Peter Rice
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




static AjPStr seqretsplit_Name(AjPTable table, AjPSeq seq);




/* @prog seqretsplit **********************************************************
**
** Reads and writes (returns) sequences in individual files
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq = NULL;
    AjBool firstonly;
    AjPTable table = NULL;
    AjPStr name = NULL;

    embInit("seqretsplit", argc, argv);

    seqout    = ajAcdGetSeqoutall("outseq");
    seqall    = ajAcdGetSeqall("sequence");
    firstonly = ajAcdGetBool("firstonly");

    table = ajStrTableNewCase(1000); /* 1000 sequences. Number not critical */

    while(ajSeqallNext(seqall, &seq))
    {
	name = seqretsplit_Name(table, seq);
	ajSeqAllWrite(seqout, seq);

	if(firstonly)
	    break;
    }

    ajSeqWriteClose(seqout);
    ajStrTableFree(&table);

    ajExit();

    return 0;
}




/* @funcstatic seqretsplit_Name ***********************************************
**
** Catches duplicate names, and builds a new unique name for this run
**
** @param [w] table [AjPTable] Table of names used so far
** @param [r] seq [AjPSeq] Sequence object
** @return [AjPStr] Pointer to old name, or NULL if unchanged
******************************************************************************/

static AjPStr seqretsplit_Name(AjPTable table, AjPSeq seq)
{

    static AjPStr oldname = NULL;
    static AjPStr newname = NULL;
    AjPStr tabname  = NULL;
    AjPStr tabvalue = NULL;
    static ajint nseq = 1;
    AjPStr ret = NULL;

    ajint i;

    if(ajTableGet(table, ajSeqGetName(seq)))
    {
	nseq++;
	ajDebug("seqretsplit_Name test nseq:%d name '%S'\n",
		nseq, ajSeqGetName(seq));

	for(i=2; i <= nseq; i++)
	{
	    ajFmtPrintS(&newname, "%S.%03d", ajSeqGetName(seq), i);

	    if(!ajTableGet(table, newname))
	    {
		ajStrAssS(&oldname, ajSeqGetName(seq));
		ajSeqAssName(seq, newname);
		ajWarn("Duplicate name '%S' changed to '%S'",
		       oldname, newname);
		ajDebug("seqretsplit_Name oldname '%S' newname '%S'\n",
			oldname, newname);
		ret = oldname;
		break;
	    }
	}

	if(!ret)
	    ajWarn("Unable to set new name for duplicate sequence "
		   "number %d '%S'", nseq, ajSeqGetName(seq));
    }
    else
	ajDebug("seqretsplit_Name OK name '%S'\n",
		ajSeqGetName(seq));

    ajStrAssS(&tabname, ajSeqGetName(seq));
    ajStrAssC(&tabvalue, ""); /* can't be NULL - needed to test Get result */
    ajTablePut(table, tabname, tabvalue);

    ajDebug("seqretsplit_Name add to table '%S'\n", tabname);
    ajStrTableTrace(table);

    return ret;
}




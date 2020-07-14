/* @source seqmatchall application
**
** All against all comparison of a set of sequences
**
** @author: Copyright (C) Ian Longden (guess by ajb)
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




static void seqmatchall_matchListPrint(void **x,void *cl);
static void seqmatchall_listPrint(AjPFile outfile, AjPList list);




static AjPSeq seq2;
static AjPSeq seq1;

ajint statwordlen;




/* @prog seqmatchall **********************************************************
**
** Does an all-against-all comparison of a set of sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPTable seq1MatchTable = 0;
    AjPList matchlist;
    AjPSeqset seqset;
    AjPFile outfile;

    ajint i;
    ajint j;


    embInit("seqmatchall", argc, argv);

    seqset      = ajAcdGetSeqset("sequence1");
    statwordlen = ajAcdGetInt("wordsize");
    outfile     = ajAcdGetOutfile("outfile");

    embWordLength(statwordlen);

    for(i=0;i<ajSeqsetSize(seqset);i++)
    {
	seq1 = ajSeqsetGetSeq(seqset,i);
	seq1MatchTable = 0;
	if(ajSeqLen(seq1) > statwordlen)
	{
	    if(embWordGetTable(&seq1MatchTable, seq1)) /* get table of words */
	    {
		for(j=i+1;j<ajSeqsetSize(seqset);j++)
		{
		    seq2 = ajSeqsetGetSeq(seqset,j);
		    if(ajSeqLen(seq2) > statwordlen)
		    {
			matchlist = embWordBuildMatchTable(&seq1MatchTable,
							   seq2, ajTrue);
			if(matchlist)
			{
			    seqmatchall_listPrint(outfile, matchlist);
			    /* free the match structures */
			    embWordMatchListDelete(&matchlist);
			}
		    }
		}
	    }
	    embWordFreeTable(seq1MatchTable); /* free table of words */
	}
    }

    ajExit();

    return 0;
}




/* @funcstatic seqmatchall_matchListPrint *************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void seqmatchall_matchListPrint(void **x,void *cl)
{
    EmbPWordMatch p;
    AjPFile outfile;

    p = (EmbPWordMatch)*x;
    outfile = (AjPFile) cl;

    ajFmtPrintF(outfile, "%d  %d %d %s %d %d %s\n",
		(*p).length,
		(*p).seq1start+1,(*p).seq1start+(*p).length,seq1->Name->Ptr,
		(*p).seq2start+1,(*p).seq2start+(*p).length,seq2->Name->Ptr);

    return;
}




/* @funcstatic seqmatchall_listPrint ******************************************
**
** Undocumented.
**
** @param [?] outfile [AjPFile] Undocumented
** @param [?] list [AjPList] Undocumented
** @@
******************************************************************************/

static void seqmatchall_listPrint(AjPFile outfile, AjPList list)
{
    ajListMap(list,seqmatchall_matchListPrint, outfile);

    return;
}

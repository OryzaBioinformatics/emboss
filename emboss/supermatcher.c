/* @source supermatcher application
**
** Local alignment of large sequences
**
** @author: Copyright (C) Ian Longden
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

/* supermatcher
** Create a word table for the first sequence.
** Then go down second sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last
** position if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding
** the biggest match and calculating start and ends for both sequences.
*/

#include "emboss.h"
#include <limits.h>
#include <math.h>




/* @datastatic concat *********************************************************
**
** supermatcher internals
**
** @alias concatS
**
** @attr offset [ajint] Undocumented
** @attr count [ajint] Undocumented
** @attr total [ajint] Undocumented
** @attr list [AjPList] Undocumented
******************************************************************************/

typedef struct concatS
{
    ajint offset;
    ajint count;
    ajint total;
    AjPList list;
} concat;




static void supermatcher_matchListOrder(void **x,void *cl);
static void supermatcher_orderandconcat(AjPList list,AjPList ordered);
static void supermatcher_removelists(void **x,void *cl);
static ajint supermatcher_findstartpoints(AjPTable *seq1MatchTable,AjPSeq b,
					  AjPSeq a, ajint *start1,
					  ajint *start2, ajint *end1,
					  ajint *end2, ajint width);
static void supermatcher_findmax(void **x,void *cl);




concat *conmax = NULL;
ajint maxgap   = 0;




/* @prog supermatcher *********************************************************
**
** Finds a match of a large sequence against one or more sequences
**
** Create a word table for the first sequence.
** Then go down second sequence checking to see if the word matches.
** If word matches then check to see if the position lines up with the last
** position if it does continue else stop.
** This gives us the start (offset) for the smith-waterman match by finding
** the biggest match and calculating start and ends for both sequences.
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seq1;
    AjPSeqset seq2;
    AjPSeq a;
    AjPSeq b;
    AjPStr m = 0;
    AjPStr n = 0;

    AjPFile outf = NULL;
    AjPFile errorf;
    AjBool show = ajFalse;
    AjBool scoreonly = ajFalse;
    AjBool showalign = ajTrue;

    ajint    lena = 0;
    ajint    lenb = 0;

    char   *p;
    char   *q;

    AjPMatrixf matrix;
    AjPSeqCvt cvt = 0;
    float **sub;
    ajint *compass = 0;
    float *path = 0;

    float gapopen;
    float gapextend;
    float score;


    ajint begina;
    ajint i;
    ajint k;
    ajint beginb;
    ajint start1 = 0;
    ajint start2 = 0;
    ajint end1   = 0;
    ajint end2   = 0;
    ajint width  = 0;
    AjPTable seq1MatchTable = 0;
    ajint wordlen = 6;
    ajint oldmax = 0;

    AjPAlign align = NULL;

    embInit("supermatcher", argc, argv);

    matrix    = ajAcdGetMatrixf("datafile");
    seq1      = ajAcdGetSeqall("asequence");
    seq2      = ajAcdGetSeqset("bsequence");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    wordlen   = ajAcdGetInt("wordlen");
    align     = ajAcdGetAlign("outfile");
    errorf    = ajAcdGetOutfile("errorfile");
    width     = ajAcdGetInt("width");	/* not the same as awidth */

    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */
    /* show      = ajAcdGetBool("showinternals");*/
    /* scoreonly = ajAcdGetBool("scoreonly"); */
    /* showalign = ajAcdGetBool("showalign"); */

    gapopen   = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    if(!showalign)
	scoreonly = ajTrue;

    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    embWordLength(wordlen);

    for(k=0;k<ajSeqsetSize(seq2);k++)
    {
	b = ajSeqsetGetSeq(seq2, k);
	ajSeqTrim(b);
    }

    while(ajSeqallNext(seq1,&a))
    {
        ajSeqTrim(a);
	begina = 1 + ajSeqOffset(a);

	m = ajStrNewL(1+ajSeqLen(a));

	lena = ajSeqLen(a);

	ajDebug("Read '%S'\n", ajSeqGetName(a));

	if(!embWordGetTable(&seq1MatchTable, a)) /* get table of words */
	    ajErr("Could not generate table for %s\n",
		  ajSeqName(a));

	for(k=0;k<ajSeqsetSize(seq2);k++)
	{
	    b      = ajSeqsetGetSeq(seq2, k);
	    lenb   = ajSeqLen(b);
	    beginb = 1 + ajSeqOffset(b);

	    n=ajStrNewL(1+ajSeqLen(b));

	    ajDebug("Processing '%S'\n", ajSeqGetName(b));
	    p = ajSeqChar(a);
	    q = ajSeqChar(b);

	    ajStrAssC(&m,"");
	    ajStrAssC(&n,"");


	    if(!supermatcher_findstartpoints(&seq1MatchTable,b,a,
					     &start1, &start2,
					     &end1, &end2,
					     width))
	    {
		start1 = 0;
		end1   = lena-1;
		start2 = (ajint)(width/2);
		end2   = lenb-1;

		ajFmtPrintF(errorf,
			    "No wordmatch start points for "
			    "%s vs %s. No alignment\n",
			    ajSeqName(a),ajSeqName(b));
		ajStrDel(&n);
		continue;
	    }
	    ajDebug("++ %S v %S end1: %d start1: %d\n",
		    ajSeqGetName(a), ajSeqGetName(b), end1, start1);

	    if(end1-start1 > oldmax)
	    {
		oldmax = ((end1-start1)+1)*width;
		AJRESIZE(path,oldmax*width*sizeof(float));
		AJRESIZE(compass,oldmax*width*sizeof(ajint));
		ajDebug("++ resize to oldmax: %d\n", oldmax);
	    }

	    for(i=0;i<((end1-start1)+1)*width;i++)
		path[i] = 0.0;

	    ajDebug("Calling embAlignPathCalcFast "
		     "%d..%d [%d/%d] %d..%d [%d/%d]\n",
		     start1, end1, (end1 - start1 + 1), lena,
		     start2, end2, (end2 - start2 + 1), lenb);

	    embAlignPathCalcFast(&p[start1],&q[start2],
				 end1-start1+1,end2-start2+1,
				 gapopen,gapextend,path,sub,cvt,
				 compass,show,width);


	    ajDebug("Calling embAlignScoreSWMatrixFast\n");

	    score = embAlignScoreSWMatrixFast(path,compass,gapopen,gapextend,
					      a,b,end1-start1+1,end2-start2+1,
					      sub,cvt,&start1,&start2,width);

	    if(scoreonly)
	    {
		if(outf)
		    ajFmtPrintF(outf,"%s %s %.2f\n",ajSeqName(a),ajSeqName(b),
				score);
	    }
	    else
	    {
		ajDebug("Calling embAlignWalkSWMatrixFast\n");
		embAlignWalkSWMatrixFast(path,compass,gapopen,gapextend,a,b,
					 &m,&n,end1-start1+1,end2-start2+1,
					 sub,cvt,&start1,&start2,width);

		ajDebug("Calling embAlignPrintLocal\n");
		if(outf)
		    embAlignPrintLocal(outf,ajSeqChar(a),ajSeqChar(b),
				       m,n,start1,start2,
				       score,1,sub,cvt,ajSeqName(a),
				       ajSeqName(b),
				       begina,beginb);
		embAlignReportLocal(align, a, b,
				    m,n,start1,start2,
				    gapopen, gapextend,
				    score,matrix, begina, beginb);
		ajAlignWrite(align);
		ajAlignReset(align);
	    }
	    ajStrDel(&n);
	}

	embWordFreeTable(seq1MatchTable); /* free table of words */
	seq1MatchTable=0;

	ajStrDel(&m);

    }

    ajAlignClose(align);
    ajAlignDel(&align);

    ajExit();

    return 0;
}




/* @funcstatic supermatcher_matchListOrder ************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void supermatcher_matchListOrder(void **x,void *cl)
{
    EmbPWordMatch p;
    AjPList ordered;
    ajint offset;
    AjIList listIter;
    concat *con;
    concat *c=NULL;

    p = (EmbPWordMatch)*x;
    ordered = (AjPList) cl;

    offset = (*p).seq1start-(*p).seq2start;

    /* iterate through ordered list to find if it exists already*/
    listIter = ajListIter(ordered);

    while(!ajListIterDone( listIter))
    {
	con = ajListIterNext(listIter);
	if(con->offset == offset)
	{
	    /* found so add count and set offset to the new value */
	    con->offset = offset;
	    con->total+= (*p).length;
	    con->count++;
	    ajListPushApp(con->list,p);
	    ajListIterFree(listIter);
	    return;
	}
    }
    ajListIterFree(listIter);

    /* not found so add it */
    AJNEW(c);
    c->offset = offset;
    c->total  = (*p).length;
    c->count  = 1;
    c->list   = ajListNew();
    ajListPushApp(c->list,p);
    ajListPushApp(ordered, c);

    return;
}




/* @funcstatic supermatcher_orderandconcat ************************************
**
** Undocumented.
**
** @param [r] list [AjPList] unordered input list
** @param [w] ordered [AjPList] ordered output list
** @return [void]
** @@
******************************************************************************/

static void supermatcher_orderandconcat(AjPList list,AjPList ordered)
{
    ajListMap(list,supermatcher_matchListOrder, ordered);

    return;
}




/* @funcstatic supermatcher_removelists ***************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void supermatcher_removelists(void **x,void *cl)
{
    concat *p;

    p = (concat *)*x;

    ajListFree(&(p)->list);
    AJFREE(p);

    return;
}




/* @funcstatic supermatcher_findmax *******************************************
**
** Undocumented.
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void supermatcher_findmax(void **x,void *cl)
{
    concat *p;
    ajint *max;

    p   = (concat *)*x;
    max = (ajint *) cl;

    if(p->total > *max)
    {
	*max = p->total;
	conmax = p;
    }

    return;
}




/* @funcstatic supermatcher_findstartpoints ***********************************
**
** Undocumented.
**
** @param [w] seq1MatchTable [AjPTable*] match table
** @param [r] b [AjPSeq] second sequence
** @param [r] a [AjPSeq] first sequence
** @param [w] start1 [ajint*] start in sequence 1
** @param [w] start2 [ajint*] start in sequence 2
** @param [w] end1 [ajint*] end in sequence 1
** @param [w] end2 [ajint*] end in sequence 2
** @param [r] width [ajint] width
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint supermatcher_findstartpoints(AjPTable *seq1MatchTable,AjPSeq b,
					  AjPSeq a, ajint *start1,
					  ajint *start2, ajint *end1,
					  ajint *end2, ajint width)
{
    ajint hwidth = 0;
    ajint max = -10;
    ajint offset = 0;
    AjPList matchlist = NULL;
    AjPList ordered = NULL;
    ajint amax;
    ajint bmax;
    ajint bega;
    ajint begb;

    amax = ajSeqLen(a)-1;
    bmax = ajSeqLen(b)-1;
    bega = ajSeqOffset(a);
    begb = ajSeqOffset(b);


    ajDebug("supermatcher_findstartpoints len %d %d off %d %d\n",
	     amax, bmax, bega, begb);
    matchlist = embWordBuildMatchTable(seq1MatchTable, b, ajTrue);

    if(!matchlist)
	return 0;
    else if(!matchlist->Count)
    {
        embWordMatchListDelete(&matchlist);
	return 0;
    }


    /* order and add if the gap is gapmax or less */

    /* create list header bit*/
    ordered = ajListNew();

    supermatcher_orderandconcat(matchlist, ordered);

    ajListMap(ordered,supermatcher_findmax, &max);

    ajDebug("findstart conmax off:%d count:%d total:%d\n",
	    conmax->offset, conmax->count, conmax->total,
	    ajListLength(conmax->list));
    offset = conmax->offset;

    ajListMap(ordered,supermatcher_removelists, NULL);
    ajListFree(&ordered);
    embWordMatchListDelete(&matchlist);	/* free the match structures */


    hwidth = (ajint) width/2;

    /*offset+=hwidth;*/

    if(offset > 0)
    {
	*start1 = offset;
	*start2 = 0;
    }
    else
    {
	*start2 = 0-offset;
	*start1 = 0;
    }
    *end1 = *start1;
    *end2 = *start2;

    ajDebug("++ end1 %d -> %d end2 %d -> %d\n", *end1, amax, *end2, bmax);
    while(*end1<amax && *end2<bmax)
    {
	(*end1)++;
	(*end2)++;
    }

    ajDebug("++ end1 %d end2 %d\n", *end1, *end2);
    
    
    ajDebug("supermatcher_findstartpoints has %d..%d [%d] %d..%d [%d]\n",
	    *start1, *end1, ajSeqLen(a), *start2, *end2, ajSeqLen(b));

    return 1;
}

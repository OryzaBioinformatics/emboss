/* @source abiview application
**
** Display an ABI trace file and write out the sequence.
** @author: Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
** The sequence is taken from a ABI trace file and written to a
** sequence file.
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


static AjPGraphPlpData abiview_graphDisplay(AjPGraph graphs,
					    const AjPInt2d trace,
					    ajint nstart, ajint nstop,
					    const AjPShort  basePositions,
					    ajint base,
					    ajint colour, AjBool overlay,
					    float tmax, ajint* ntrace);

static AjPGraphPlpData abiview_graphTextDisplay(AjPGraph graphs, ajint nstart,
						ajint nstop,
						const AjPShort basePositions,
						AjBool overlay,
						const AjPStr nseq,
						float tmax, ajint nt);

static void abiview_TextDisplay(AjPGraph graphs, ajint nstart, ajint nstop,
				const AjPStr nseq, float tmax);

static AjBool abiview_drawbase(const char* res, const AjPStr baseN);
static ajint  abiview_getResColour(char B);




/* @prog abiview **************************************************************
**
** Reads ABI file and display the trace
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr  fname;
    AjPFile fp;
    AjPSeqout seqout;
    AjPSeq    seqo;
    AjPStr    nseq;
    AjPStr    baseN;
    AjPInt2d  trace = NULL;
    AjPShort  basePositions = NULL;
    AjPGraph  graphs = NULL;

    AjBool graph1;
    AjBool graph2;
    AjBool graph3;
    AjBool graph4;
    AjBool overlay;
    AjBool separate;
    AjBool yticks;
    AjBool dseq;

    AjPGraphPlpData gd1 = NULL;
    AjPGraphPlpData gd2 = NULL;
    AjPGraphPlpData gd3 = NULL;
    AjPGraphPlpData gd4 = NULL;
    AjPGraphPlpData gd5 = NULL;

    ajint ntrace;
    ajint strace;
    ajlong fwo_;

    float tmax;

    ajint base_start;
    ajint base_end;
    ajint nstart;
    ajint nstop;
    ajint window;
    ajint iloop;
    ajint ibase;
    ajint nbases;
    ajlong baseO;
    ajlong numBases;
    ajlong  basePosO;
    ajlong numPoints;
    ajlong dataOffset[4];

    char res1;
    char res2;
    char res3;
    char res4;

    /* BYTE[i] is a byte mask for byte i */
    const ajlong BYTE[] = { 0x000000ff };


    ajGraphInit("abiview", argc, argv);

    fp         = ajAcdGetInfile("infile");
    graphs     = ajAcdGetGraphxy("graph");
    base_start = ajAcdGetInt("startbase");
    base_end   = ajAcdGetInt("endbase");
    seqout     = ajAcdGetSeqout("outseq");
    separate   = ajAcdGetBool("separate");
    yticks     = ajAcdGetBool("yticks");
    dseq       = ajAcdGetBool("sequence");
    window     = ajAcdGetInt("window");
    baseN      = ajAcdGetString("bases");

    fname = ajStrNewC(ajFileName(fp));

    ajStrToUpper(&baseN);
    nbases  = ajStrLen(baseN);
    overlay = !separate;


    if(!ajSeqABITest(fp))
        ajFatal("%s not an ABI file",ajFileName(fp));

    numBases = ajSeqABIGetNBase(fp);

    baseO = ajSeqABIGetBaseOffset(fp);	/* find BASE tag & get offset */
    nseq  = ajStrNew();			/* read in sequence */

    if(base_end != 0 && base_end < numBases)      /* define end base */
        numBases = base_end;

    if(numBases < base_start)
        ajFatal("-startbase (%d) larger than the number of bases (%d).",
                 base_start,numBases);

    /* For data, all in one 'plot' */
    if(ajGraphIsData(graphs))
	window = numBases+1;

    trace = ajInt2dNew();
    basePositions = ajShortNew();

    numPoints = ajSeqABIGetNData(fp);  /* find DATA tag & get no. of points */

    /* get data trace offsets           */
    ajSeqABIGetTraceOffset(fp,dataOffset);
    ajSeqABIGetData(fp,dataOffset,numPoints,trace); /* read in trace data   */

    fwo_ = ajSeqABIGetFWO(fp);	       /* find FWO tag - field order GATC   */

    res1 = (char)(fwo_>>24&BYTE[0]);
    res2 = (char)(fwo_>>16&BYTE[0]);
    res3 = (char)(fwo_>>8&BYTE[0]);
    res4 = (char)(fwo_&BYTE[0]);


    /* decide whether to draw graph for each base */
    graph1 = abiview_drawbase(&res1,baseN);
    graph2 = abiview_drawbase(&res2,baseN);
    graph3 = abiview_drawbase(&res3,baseN);
    graph4 = abiview_drawbase(&res4,baseN);


    ajSeqABIReadSeq(fp,baseO,numBases,&nseq);
    basePosO = ajSeqABIGetBasePosOffset(fp); /* find PLOC tag & get offset */
    ajFileSeek(fp,basePosO,SEEK_SET);
    ajSeqABIGetBasePosition(fp,numBases,&basePositions);


    /* find trace max */
    tmax = 0.;
    for(iloop=0;iloop<numPoints;iloop++)
	for(ibase=0;ibase<4;ibase++)
	    if(tmax < (float)ajInt2dGet(trace,ibase,iloop))
		tmax = (float)ajInt2dGet(trace,ibase,iloop);


    /* setup graph parameters */
    if(base_start > -1)
        nstart = base_start;
    else
        nstart = 0;

    nstop  = window+1+nstart;

    ajGraphSetTitle(graphs,fname);
    ajGraphSetYTitleC(graphs,"Signal");
    if(yticks)
    {
	ajGraphxySetYTick(graphs,ajTrue);
	ajGraphxySetYInvTicks(graphs,ajTrue);
    }
    else
	ajGraphxySetYTick(graphs,ajFalse);

    ajGraphxySetXInvTicks(graphs,ajTrue);

    ntrace = 0;
    strace = 0;


    /* loop over pages to be displayed */
    while(nstart < numBases-1)
    {
	if(nstop > numBases)
	    nstop = numBases;

	ajGraphSetMulti(graphs,nbases+1);
	ajGraphxySetOverLap(graphs,overlay);

	if(graph1)
	    gd1 = abiview_graphDisplay(graphs,trace,nstart,nstop,basePositions,
				       0,abiview_getResColour(res1),overlay,
				       tmax,&ntrace);

	ntrace = strace;
	if(graph2)
	    gd2 = abiview_graphDisplay(graphs,trace,nstart,nstop,basePositions,
				       1,abiview_getResColour(res2),overlay,
				       tmax,&ntrace);
	ntrace = strace;

	if(graph3)
	    gd3 = abiview_graphDisplay(graphs,trace,nstart,nstop,basePositions,
				       2,abiview_getResColour(res3),overlay,
				       tmax,&ntrace);
	ntrace = strace;

	if(graph4)
	    gd4 = abiview_graphDisplay(graphs,trace,nstart,nstop,basePositions,
				       3,abiview_getResColour(res4),overlay,
				       tmax,&ntrace);


	/* Sequence text display */
	if(dseq)
	{
	    if(!overlay)
		gd5 = abiview_graphTextDisplay(graphs,nstart,nstop,
					       basePositions,overlay,nseq,
					       tmax,strace);
	    else
		abiview_TextDisplay(graphs,nstart,nstop,nseq,tmax);
	}

	strace = ntrace;

	ajGraphxyDisplay(graphs,ajFalse);

	/* Clean up */
	if(nstop<numBases)
	{
	    if(graph1) ajGraphPlpDataDel(&gd1); /* free graph data mem */
	    if(graph2) ajGraphPlpDataDel(&gd2);
	    if(graph3) ajGraphPlpDataDel(&gd3);
	    if(graph4) ajGraphPlpDataDel(&gd4);

	    if(dseq)
	    {
		if(!overlay)
		{
		    ajGraphPlpDataDel(&gd5); /* free seq text mem */
		}
		else
		    ajGraphClear(graphs); /* free seq text mem */
	    }


	    ajGraphNewPage(ajFalse);	/* display new page  */
	}

	nstart = nstop-1;
	nstop  = nstart+window+1;

    }


    ajFileClose(&fp);

    ajGraphCloseWin();
    if(dseq && overlay)
	ajGraphClear(graphs);   /* free seq text mem */

    if(dseq && !overlay)
	ajGraphPlpDataDel(&gd5);  /* free seq text mem */

/*    ajGraphxyDel(&graphs);*/
    ajInt2dDel(&trace);
    ajShortDel(&basePositions);

    /* write out consensus sequence */
    seqo = ajSeqNew();
    ajSeqAssName(seqo,fname);
    ajSeqAssSeq(seqo,nseq);
    ajSeqSetRange(seqo,base_start,ajSeqEnd(seqo));
    ajSeqWrite(seqout,seqo);
    ajStrDel(&nseq);
    ajSeqDel(&seqo);

    ajExit ();

    return 0;
}




/* @funcstatic abiview_graphDisplay *******************************************
**
** Load in ABI trace data into graph data object.
**
** @param [u] graphs [AjPGraph] Graph
** @param [r] trace [const AjPInt2d] Trace array
** @param [r] nstart [ajint] Start position
** @param [r] nstop [ajint] End position
** @param [r] basePositions [const AjPShort] Number of bases
** @param [r] base [ajint] Base number
** @param [r] colour [ajint] Colour code
** @param [r] overlay [AjBool] Overlay plot
** @param [r] tmax [float] Maximum
** @param [w] nt [ajint*] Number of nucleotides
** @return [AjPGraphPlpData] graph data object
**
******************************************************************************/

static AjPGraphPlpData abiview_graphDisplay(AjPGraph graphs,
					    const AjPInt2d trace,
					    ajint nstart, ajint nstop,
					    const AjPShort  basePositions,
					    ajint base, ajint colour,
					    AjBool overlay, float tmax,
					    ajint* nt)
{
    ajint i;
    ajshort bP;
    ajshort lastbP;
    ajint bstart;

    AjPGraphPlpData gdata;


    /* create graph data object */
    gdata = ajGraphPlpDataNewI(ajShortGet(basePositions,nstop-1)-(*nt));

    if(nstart>0)
	lastbP = ajShortGet(basePositions,nstart-1);
    else
	lastbP = 0;

    bstart = *nt;

    for(i=nstart;i<nstop;i++)
    {
	bP = ajShortGet(basePositions,i);
	while(*nt < bP)
	{
	    gdata->x[*nt-bstart] = (float)i + (float)(*nt+1-lastbP)/
		(float)(bP-lastbP);
	    gdata->y[*nt-bstart] = (float)ajInt2dGet(trace,base,*nt);
	    *nt = *nt+1;
	}
	lastbP = bP;
    }

    ajGraphPlpDataSetColour(gdata,colour);
    ajGraphPlpDataSetMaxMin(gdata,(float)nstart+1.,
                           (float)nstop,0.,tmax+80.);

    /* add graph to list in a multiple graph */
    ajGraphDataAdd(graphs,gdata);

    return gdata;
}




/* @funcstatic abiview_graphTextDisplay ***************************************
**
** Draw sequence in a separate graph if the trace data is plotted
** in separate graphs (i.e. not overlayed).
**
** @param [u] graphs [AjPGraph] Graph
** @param [r] nstart [ajint] Start
** @param [r] nstop [ajint] Stop
** @param [r] basePositions [const AjPShort] Base positions
** @param [r] overlay [AjBool] Overlay
** @param [r] nseq [const AjPStr] Sequence number
** @param [r] tmax [float] Maximum
** @param [r] nt [ajint] Nt data
** @return [AjPGraphPlpData] graph data object containing the sequence text
**
******************************************************************************/

static AjPGraphPlpData abiview_graphTextDisplay(AjPGraph graphs, ajint nstart,
					     ajint nstop,
					     const AjPShort basePositions,
					     AjBool overlay, const AjPStr nseq,
					     float tmax, ajint nt)
{
    ajint i;
    ajint colres;

    AjPGraphPlpData gdata;
    char res[2];

    res[1]='\0';

    /* create graph data object */
    gdata = ajGraphPlpDataNewI(ajShortGet(basePositions,nstop-1)-nt);

    for(i=nstart;i<nstop;i++)
    {
	*res = ajStrChar(nseq,i);
	colres = abiview_getResColour(*res);
	ajGraphPlpDataAddText(gdata,(float)i+1.,tmax+75.,colres,res);
    }

    ajGraphPlpDataSetMaxMin(gdata,(float)nstart+1,
			   (float)nstop,tmax+70.,tmax+80.);

    /* add graph to list in a multiple graph */
    ajGraphDataAdd(graphs,gdata);

    return gdata;
}




/* @funcstatic abiview_TextDisplay ********************************************
**
** Add sequence on top of the same graph as the trace data
** (i.e. overlayed).
**
** @param [u] graphs [AjPGraph] Graph
** @param [r] nstart [ajint] Start
** @param [r] nstop [ajint] Stop
** @param [r] nseq [const AjPStr] Sequence number
** @param [r] tmax [float] Maximum
** @return [void]
**
******************************************************************************/

static void abiview_TextDisplay(AjPGraph graphs, ajint nstart, ajint nstop,
				const AjPStr nseq, float tmax)
{
    ajint i;
    ajint colres;

    char res[2];

    res[1] = '\0';
    for(i=nstart;i<nstop-1;i++)
    {
	*res = ajStrChar(nseq,i);
	colres = abiview_getResColour(*res);
	ajGraphAddText(graphs,(float)i+1.,tmax+30.,colres,res);
    }

    return;
}




/* @funcstatic abiview_drawbase ***********************************************
**
** Test to see if this base, i.e. res, is selected to be drawn
** (default is to draw graphs for all bases);
**
** @param [r] res [const char*] Base
** @param [r] baseN [const AjPStr] Base number
** @return [AjBool] ajTrue on success
**
******************************************************************************/

static AjBool abiview_drawbase(const char* res, const AjPStr baseN)
{

    AjPRegexp rexp = NULL;
    AjBool draw = ajFalse;
    AjPStr b;
    ajint i;


    b = ajStrNew();
    ajStrAssSubC(&b,res,0,0);

    for(i=0;i<4;i++)
    {
	rexp = ajRegComp(b);
	if(ajRegExec(rexp,baseN))
	    draw = ajTrue;

	ajRegFree(&rexp);
    }

    ajStrDel(&b);

    return draw;
}




/* @funcstatic abiview_getResColour *******************************************
**
** Assign colour to a given nucleotide.
**
** @param [r] B [char] Base
** @return [ajint] base colour code
**
******************************************************************************/

static ajint abiview_getResColour(char B)
{
  /*return ((B)=='C'?RED:(B)=='A'?GREEN:(B)=='G'?BLUE:(B)=='T'?BLACK:YELLOW);*/
/* use ABI standard colours - Clemens Broger, Roche */
    return ((B)=='C'?BLUE:(B)=='A'?GREEN:(B)=='G'?BLACK:(B)=='T'?RED:YELLOW);
}

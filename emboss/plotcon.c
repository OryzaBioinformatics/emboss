/* @source plotcon application
**
** Plots similarity over the length of a multiple sequence alignment
** @author Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
**
**   Mandatory qualifiers:
** [-msf]              seqset     File containing a sequence alignment
** -graph              xygraph    Graph type
** -winsize            integer    Window size for calculating the quality of
**                                the alignment
**
** Optional qualifiers:
**
** -scorefile    The data file holding the matrix comparison table.
**
**
**
** Average similarity calculation:
**
** Av. Sim. =       sum( Mij*wi + Mji*w2  )
**            -----------------------------------
**               (Nseq*Wsize)*((Nseq-1)*Wsize)
**
**     sum   - over column*window size
**     M     - matrix comparison table
**     i,j   - wrt residue i or j
**     Nseq  - no. of sequences in the alignment
**     Wsize - window size
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
#include "ajtime.h"




/* @datastatic PPoint *********************************************************
**
** Plotcon internals
**
** @alias SPoint
** @alias OPoint
**
** @attr x1 [float] x1 coordinate
** @attr y1 [float] y1 coordinate
** @attr x2 [float] x2 coordinate
** @attr y2 [float] y2 coordinate
******************************************************************************/


typedef struct SPoint
{
    float x1;
    float y1;
    float x2;
    float y2;
} OPoint;
#define PPoint OPoint*




/* @prog plotcon **************************************************************
**
** Plots the quality of conservation of a sequence alignment
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqset seqset;
    ajint i;
    ajint numseq;
    ajint lenseq;
    ajint j = 0;
    ajint k;
    AjPMatrix cmpmatrix = 0;
    AjPSeqCvt cvt = 0;
    AjOTime ajtime;
    time_t tim;

    const char **seqcharptr;

    float *x;
    float *y;
    AjPFloat2d score = NULL;
    float *sumscore  = 0;
    float scoremax   = 0;
    float ymin;
    float ymax;
    float contri = 0;
    float contrj = 0;
    ajint **matrix;
    ajint m1 = 0;
    ajint m2 = 0;
    ajint highindex = 0;
    ajint winsize;	/* window size */
    ajint numbins;	/* total no. of bins making up the seq length */
    ajint binup;
    ajint binlo;
    ajint binmid;
    ajint bin;
    ajint bin2;
    AjPGraph graphs = NULL;
    AjPGraphPlpData gdata;
    AjPList list = NULL;

    ajGraphInit("plotcon", argc, argv);

    seqset = ajAcdGetSeqset("sequences");

    ajSeqsetFill(seqset);               /* Pads seq set with gap characters */
    numseq = ajSeqsetSize (seqset);
    lenseq = ajSeqsetLen(seqset);

    winsize   = ajAcdGetInt("winsize");
    cmpmatrix = ajAcdGetMatrix("scorefile");

    graphs = ajAcdGetGraphxy( "graph");

    tim = time(0);

    ajTimeLocal(tim,&ajtime);
    ajtime.format = 0;

    matrix  = ajMatrixArray(cmpmatrix);
    cvt     = ajMatrixCvt(cmpmatrix);	/* Returns conversion table */


    /****************** Memory allocation *****************/

    AJCNEW(seqcharptr, numseq);		/* sequence string */
    score = ajFloat2dNew();

    numbins = lenseq;
    AJCNEW(sumscore,numbins);		/* sum of scores for each column */
    AJCNEW(x,numbins);			/* x data                        */
    AJCNEW(y,numbins);			/* y data                        */

    bin2 = (ajint)((float)winsize/2.);

    for(i=0;i<numseq;i++)
    {
	ajSeqsetToUpper(seqset);

	/* get sequence as a string */
	seqcharptr[i] =  ajSeqsetSeq (seqset, i);

	for(j=0;j<ajSeqsetLen(seqset);j++)
	    ajFloat2dPut(&score,i,j,0.);
    }

    for(bin=0; bin<numbins; bin++)
	sumscore[bin] = 0.;


    /************** Loop over the sequence set length **************/
    for(k=0; k< ajSeqsetLen(seqset); k++)
    {
	/* Generate a score for each column */
	for(i=0;i<numseq;i++)
	{
	    /* get ajint code from conversion table */
	    m1 = ajSeqCvtK(cvt, seqcharptr[i][k]);

	    for(j=i+1;j<numseq;j++)
	    {
		m2 = ajSeqCvtK(cvt, seqcharptr[j][k]);
		if(m1 && m2)
		{
		    contrj = (float)matrix[m1][m2]*ajSeqsetWeight(seqset, j)+
			ajFloat2dGet(score,i,k);
		    contri = (float)matrix[m1][m2]*ajSeqsetWeight(seqset, i)+
			ajFloat2dGet(score,j,k);

		    ajFloat2dPut(&score,i,k,contrj);
		    ajFloat2dPut(&score,j,k,contri);

		}
	    }
	}


	/* calculate score for the entire window */
	if(k >= (winsize-1))
	{
	    binup = k+1;
	    binlo = k - winsize + 1;
	    binmid = binlo+bin2;
	    for(i=0;i<numseq;i++)
		for(bin=binlo;bin<binup;bin++)
		    sumscore[binmid] += ajFloat2dGet(score,i,bin);
	}

	/* find the highest score */
	highindex = -1;
	scoremax = -3;

    }

    /*************** End of Loop ***************/

    ajGraphSetCharSize(0.50);
    gdata = ajGraphPlpDataNewI(ajSeqsetLen(seqset));


    for(bin=0;bin<numbins;bin++)
	sumscore[bin] = sumscore[bin]/((float)(numseq*(numseq-1)*winsize*
					       winsize));

    /* plot out results */

    for(bin=0;bin<numbins;bin++)
    {
	gdata->x[bin] = (float)(bin+1);
	gdata->y[bin] = sumscore[bin];
    }


    ajGraphArrayMaxMin(gdata->y,ajSeqsetLen(seqset),&ymin,&ymax);
    ajGraphPlpDataSetMaxima(gdata,0,ajSeqsetLen(seqset),ymin,ymax);
    
    ajGraphPlpDataSetTypeC(gdata,"2D Plot");
    ajGraphDataAdd(graphs,gdata);
    ajGraphxySetYTick(graphs, ajTrue);
    ajGraphSetTitleC(graphs,"Similarity Plot of Aligned Sequences");
    ajGraphSetYTitleC(graphs,"Similarity");
    
    ajGraphxyDisplay(graphs,ajTrue);
    
    ajGraphCloseWin();
    ajGraphxyDel(&graphs);

    AJFREE(x);
    AJFREE(y);
    AJFREE(sumscore);
    AJFREE(seqcharptr);
    ajListDel(&list);
    ajFloat2dDel(&score);

    ajSeqsetDel(&seqset);
    ajMatrixDel(&cmpmatrix);

    embExit ();

    return 0;
}

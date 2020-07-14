/* @source embcons.c
**
** General routines for consensus.
** Copyright (c) Copyright 2001 (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
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
#include <ctype.h>          /* for tolower function */
#include <limits.h>         /* for INT_MAX */




/* @func embConsCalc **********************************************************
**
** Calculates a consensus.
**
** @param [r] seqset [AjPSeqset] multiple alignment
** @param [r] cmpmatrix [AjPMatrix] comparison scoring matrix
** @param [r] nseqs [ajint] number of sequences in alignment
** @param [r] mlen [ajint] length of alignment
** @param [r] fplural [float] no. of +ve scoring matches below which
**                            there is no consensus
** @param [r] setcase [float] case set if score above/below +ve matching
**                            threshold
** @param [r] identity [ajint] number of identical symbols required
**                             for consesnsus
** @param [w] cons [AjPStr *] the created consensus sequence
** @return [void]
******************************************************************************/

void embConsCalc(AjPSeqset seqset,AjPMatrix cmpmatrix,
		 ajint nseqs, ajint mlen,float fplural,float setcase,
		 ajint identity, AjPStr *cons)
{
    ajint i;
    ajint j;
    ajint k;
    ajint **matrix;
    ajint m1 = 0;
    ajint m2 = 0;
    ajint highindex;
    ajint matsize;
    ajint matchingmaxindex;
    ajint identicalmaxindex;

    float max;
    float contri = 0;
    float contrj = 0;
    float *identical;
    float *matching;

    AjPSeqCvt cvt  = 0;
    AjPFloat score = NULL;
    char **seqcharptr;
    char res;
    char nocon;


    matrix  = ajMatrixArray(cmpmatrix);
    cvt     = ajMatrixCvt(cmpmatrix);	/* return conversion table */
    matsize = ajMatrixSize(cmpmatrix);

    AJCNEW(seqcharptr,nseqs);
    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);

    score = ajFloatNew();

    nocon = '-';
    if(ajSeqsetIsNuc(seqset))        /* set non-consensus character */
	nocon = 'N';
    else if ( ajSeqsetIsProt(seqset))
	nocon = 'X';


    for(i=0;i<nseqs;i++)		/* get sequence as string */
	seqcharptr[i] =  ajSeqsetSeq(seqset, i);

    for(k=0; k< mlen; k++)
    {
	res = nocon;

	for(i=0;i<matsize;i++)	      /* reset id's and +ve matches */
	{
	    identical[i] = 0.0;
	    matching[i] = 0.0;
	}

	for(i=0;i<nseqs;i++)
	    ajFloatPut(&score,i,0.);

	for(i=0;i<nseqs;i++)	      /* generate score for columns */
	{
	    m1 = ajSeqCvtK(cvt,seqcharptr[i][k]);
	    if(m1)
		identical[m1] += ajSeqsetWeight(seqset,i);
	    for(j=i+1;j<nseqs;j++)
	    {
		m2 = ajSeqCvtK(cvt,seqcharptr[j][k]);
		if(m1 && m2)
		{
		    contri = (float)matrix[m1][m2]*ajSeqsetWeight(seqset,j)
			+ajFloatGet(score,i);
		    contrj = (float)matrix[m1][m2]*ajSeqsetWeight(seqset,i)
			+ajFloatGet(score,j);

		    ajFloatPut(&score,i,contri);
		    ajFloatPut(&score,j,contrj);
		}
	    }
	}

	highindex = -1;
	max  = -(float)INT_MAX;
	for(i=0;i<nseqs;i++)
	    if( ajFloatGet(score,i) > max ||
	       (ajFloatGet(score,i) == max &&
		seqcharptr[highindex][k] == '-') )      
	    {
		highindex = i;
		max       = ajFloatGet(score,i);
	    }

	for(i=0;i<nseqs;i++)	  /* find +ve matches in the column */
	{
	    m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
	    if(!matching[m1])
		for(j=0;j<nseqs;j++)
		    if( i != j)
		    {
			m2 = ajSeqCvtK (cvt, seqcharptr[j][k]);
			if(m1 && m2 && matrix[m1][m2] > 0)
			    matching[m1] += ajSeqsetWeight(seqset, j);
		    }
	}


	matchingmaxindex  = 0;	  /* get max matching and identical */
	identicalmaxindex = 0;
	for(i=0;i<nseqs;i++)
	{
	    m1 = ajSeqCvtK(cvt,seqcharptr[i][k]);
	    if(identical[m1] > identical[identicalmaxindex])
		identicalmaxindex = m1;
	}

	for(i=0;i<nseqs;i++)
	{
	    m1 = ajSeqCvtK(cvt,seqcharptr[i][k]);
	    if(matching[m1] > matching[matchingmaxindex])
		matchingmaxindex = m1;
	    else if(matching[m1] ==  matching[matchingmaxindex])
		if(identical[m1] > identical[matchingmaxindex])
		    matchingmaxindex = m1;
	}

	/* plurality check */
        m1 = ajSeqCvtK(cvt,seqcharptr[highindex][k]);
	if(matching[m1] >= fplural
	   && seqcharptr[highindex][k] != '-')
	    res = seqcharptr[highindex][k];

	if(matching[m1]<= setcase)
	    res = tolower((int)res);

	if(identity)			/* if just looking for id's */
	{
	    j = 0;
	    for(i=0;i<nseqs;i++)
		if(matchingmaxindex == ajSeqCvtK(cvt,seqcharptr[i][k]))
		    j++;

	    if(j<identity)
		res = nocon;
	}

	ajStrAppK(cons,res);
    }

    AJFREE(seqcharptr);
    AJFREE(matching);
    AJFREE(identical);
    ajFloatDel(&score);

    return;
}


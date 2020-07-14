/* @source showalign application
**
** Display a multiple sequence alignment with consensus
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 8 May 2001 - GWW - written
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

/*
**
** Outputs a set of sequence as compared to a reference sequence as follows:
**
** With reference: AII
** and sequence:   ALW
** ie. 		identical, similar, not-similar
**
** Show
** ----
**
** All		ALw
** Identical	A..
** Non-id		.lW
** Similar		Al.
** Dissimilar	..W
*/

#include "emboss.h"
#include <ctype.h>
#include <math.h>	/* for log10() */




/* @datastatic AjPOrder *******************************************************
**
** showalign internals
**
** @alias AjSOrder
** @alias AjOOrder
**
** @attr seq [AjPSeq] Sequence
** @attr similarity [ajint] total of similarity scores to consensus
**                          for sort order 
** @attr idcount [ajint] count of identical residues for stats
** @attr simcount [ajint] count of similar residues for stats
******************************************************************************/

typedef struct AjSOrder
{
    AjPSeq seq;
    /* total of similarity scores to consensus for sort order */
    ajint similarity;
    ajint idcount;	   /* count of identical residues for stats */
    ajint simcount;	     /* count of similar residues for stats */
} AjOOrder, *AjPOrder;




static ajint showalign_Getrefseq(AjPStr refseq, AjPSeqset seqset);
static void showalign_NiceMargin(AjPSeqset seqset, ajint *margin, AjBool docon,
				 ajint nrefseq);
static void showalign_Convert(AjPSeqset seqset, AjPStr *show,
			      AjBool similarcase, ajint nrefseq, ajint **sub,
			      AjPSeqCvt cvt, AjPSeq consensus);
static void showalign_MakeAll(AjPSeq ref, AjPSeq seq, ajint **sub,
			      AjPSeqCvt cvt,AjBool similarcase);
static void showalign_MakeIdentity(AjPSeq ref, AjPSeq seq);
static void showalign_MakeNonidentity(AjPSeq ref, AjPSeq seq, ajint **sub,
				      AjPSeqCvt cvt, AjBool similarcase);
static void showalign_MakeSimilar(AjPSeq ref, AjPSeq seq, ajint **sub,
				  AjPSeqCvt cvt, AjBool similarcase);
static void showalign_MakeDissimilar(AjPSeq ref, AjPSeq seq, ajint **sub,
				     AjPSeqCvt cvt);
static void showalign_Order(AjPStr *order, AjPSeqset seqset, AjPSeq consensus,
			    ajint nrefseq, ajint **sub, AjPSeqCvt cvt,
			    AjOOrder *aorder);
static ajint showalign_Output(AjPFile outf, AjPSeqset seqset, ajint nrefseq,
			      ajint width, ajint margin, AjPSeq consensus,
			      AjBool docon, AjBool bottom, AjOOrder * aorder,
			      AjBool html, AjPRange highlight,
			      AjPRange uppercase, AjBool number,
			      AjBool ruler);
static ajint showalign_OutputNums(AjPFile outf, ajint pos, ajint width,
				  ajint margin);
static ajint showalign_OutputTicks(AjPFile outf, ajint pos, ajint width,
				   ajint margin);
static ajint showalign_OutputSeq(AjPFile outf, AjPSeq seq,
				 ajint pos, ajint end, ajint width,
				 ajint margin, AjBool html,
				 AjPRange highlight, AjPRange uppercase);
static ajint showalign_CompareTwoSeqNames(const void * a, const void * b);
static ajint showalign_CompareTwoSeqSimilarities(const void * a,
						 const void * b);




/* @prog showalign ************************************************************
**
** Display a multiple sequence alignment
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf;
    AjPSeqset seqset = NULL;
    AjPStr refseq;		/* input name/number of reference sequence */
    ajint  nrefseq;		/* numeric reference sequence */
    AjPStr *show;		/* what to show */
    ajint width;		/* width of displayed sequence line */
    ajint margin;		/* width of displayed margin on left side */
    AjPMatrix matrix;		/* scoring matrix structure */
    ajint **sub;		/* integer scoring matrix */
    AjPSeqCvt  cvt = 0;		/* conversion table for scoring matrix */
    /* True if want to change case based on Similarity */
    AjBool similarcase;
    float identity;
    ajint ident;
    float fplural;
    float setcase;
    AjPStr cons;
    AjPSeq consensus;
    AjBool html;		/* format for HTML display */
    AjPRange highlight;		/* ranges to colour in HTML */
    AjPRange uppercase;		/* ranges to uppercase */
    AjBool docon;		/* show the consensus line */
    /*
     *  show the refseq line at the bottom of the alignment as well
     *  as the top
     */
    AjBool bottom;
    AjPStr *order;		/* input required order */
    AjOOrder *aorder;		/* the output order array */
    AjBool number;		/* display number line */
    AjBool ruler;		/* display ruler line */
    AjPStr xxx = NULL;

    embInit("showalign", argc, argv);

    seqset      = ajAcdGetSeqset("sequence");
    outf        = ajAcdGetOutfile("outfile");
    refseq      = ajAcdGetString("refseq");
    show        = ajAcdGetList("show");
    order       = ajAcdGetList("order");
    width       = ajAcdGetInt("width");
    margin      = ajAcdGetInt("margin");
    matrix      = ajAcdGetMatrix("matrix");
    similarcase = ajAcdGetBool("similarcase");
    docon       = ajAcdGetBool("consensus");
    bottom      = ajAcdGetBool("bottom");
    number      = ajAcdGetBool("number");
    ruler       = ajAcdGetBool("ruler");

    /* html and range formatting parameters */
    html      = ajAcdGetBool("html");
    uppercase = ajAcdGetRange("uppercase");
    highlight = ajAcdGetRange("highlight");

    /* consensus parameters */
    fplural   = ajAcdGetFloat("plurality");
    setcase   = ajAcdGetFloat("setcase");
    identity  = ajAcdGetFloat("identity");

    cons      = ajStrNew();
    consensus = ajSeqNew();


    /* get conversion table and scoring matrix */
    cvt = ajMatrixCvt(matrix);
    sub = ajMatrixArray(matrix);

    /* get the number of the reference sequence */
    nrefseq = showalign_Getrefseq(refseq, seqset);

    /* change the % plurality to the fraction of absolute total weight */
    fplural = ajSeqsetTotweight(seqset) * fplural / 100;

    /*
    ** change the % identity to the number of identical sequences at a
    ** position required for consensus
    */
    ident = ajSeqsetSize(seqset) * identity / 100;

    /* get the consensus sequence */
    embConsCalc(seqset, matrix, ajSeqsetSize(seqset), ajSeqsetLen(seqset),
		fplural, setcase, ident, &cons);
    ajSeqAssSeq(consensus, cons);	/* set the sequence string */

    /* name the sequence */

    ajSeqAssName(consensus,(xxx=ajStrNewC("Consensus")));

    /* if margin is given as -1 ensure it is reset to a nice value */
    showalign_NiceMargin(seqset, &margin, docon, nrefseq);

    /* order the output */
    AJCNEW(aorder, ajSeqsetSize(seqset));
    showalign_Order(order, seqset, consensus, nrefseq, sub, cvt, aorder);

    /* convert all sequences except the refseq to the required symbols */
    showalign_Convert(seqset, show, similarcase, nrefseq, sub, cvt, consensus);

    /* output the sequences */
    showalign_Output(outf, seqset, nrefseq, width, margin, consensus, docon,
		     bottom, aorder, html, highlight, uppercase, number,
		     ruler);


    ajFileClose(&outf);
    ajSeqDel(&consensus);
    AJFREE(aorder);

    ajStrDel(&xxx);

    ajExit();

    return 0;
}




/* @funcstatic showalign_Getrefseq ********************************************
**
** Determines which sequence should be the reference sequence.
** The first sequence in the set is returned as 0.
** -1 is returned as the consensus sequence.
**
** @param [r] refseq [AjPStr] input name/number of reference sequence
** @param [r] seqset [AjPSeqset] the sequences
** @return [ajint] the number of the reference sequence
** @@
******************************************************************************/

static ajint showalign_Getrefseq(AjPStr refseq, AjPSeqset seqset)
{
    ajint i;
    AjPSeq seq;

    for(i=0; i<ajSeqsetSize(seqset); i++)
    {
	seq = ajSeqsetGetSeq(seqset, i);

	if(!ajStrCmpO(ajSeqGetName(seq), refseq))
	    return i;
    }

    /* not a name of a sequence, so it must be a number */
    if(ajStrToInt(refseq, &i))
    {
	if(i < 0 || i > ajSeqsetSize(seqset))
	    ajFatal("Reference sequence number < 0 or > number of input "
		    "sequences: %d", i);
	return i-1;
    }
    else
	ajFatal("Reference sequence is not a sequence ID or a number: %S",
		refseq);

    return 0;
}




/* @funcstatic showalign_NiceMargin *******************************************
**
** If margin is input as -1, change it to the margin which allows
** the longest name to be displayed plus one extra space after it
**
** @param [r] seqset [AjPSeqset] the sequences
** @param [u] margin [ajint*] margin
** @param [r] docon [AjBool] displaying the consensus line
** @param [r] nrefseq [ajint] the sequence being displayed
** @return [void]
** @@
******************************************************************************/

static void showalign_NiceMargin(AjPSeqset seqset, ajint *margin,
				 AjBool docon, ajint nrefseq)
{
    ajint longest = 0;
    ajint len;
    ajint i;
    AjPSeq seq;

    /* if margin has been explicitly set, use that value */
    if(*margin != -1)
	return;

    /*
    **  Displaying the consensus at the end or using it as the ref
    **  seq?
    */
    if(docon || nrefseq == -1)
	longest = 9;			/* the length of "Consensus" */

    /* get length of longest sequence name */
    for(i=0; i<ajSeqsetSize(seqset); i++)
    {
	seq = ajSeqsetGetSeq(seqset, i);
	len = ajStrLen(ajSeqGetName(seq));
	if(len > longest)
	    longest = len;
    }

    *margin = longest+1;	/* use longest name plus one extra space */

    return;
}




/* @funcstatic showalign_Convert **********************************************
**
** Convert all sequences except the refseq to the required symbols
**
** @param [r] seqset [AjPSeqset] the sequences
** @param [r] show [AjPStr*] type of thing to show
** @param [r] similarcase [AjBool] change case according to similarity
** @param [r] nrefseq [ajint] number of the refseq
** @param [r] sub [ajint**] scoring matrix
** @param [r] cvt [AjPSeqCvt] conversion table for scoring matrix
** @param [r] consensus [AjPSeq] consensus sequence
** @return [void]
** @@
******************************************************************************/

static void showalign_Convert(AjPSeqset seqset, AjPStr *show,
			      AjBool similarcase, ajint nrefseq, ajint **sub,
			      AjPSeqCvt cvt, AjPSeq consensus)
{
    ajint i;
    AjPSeq seq;
    AjPSeq ref;
    char showchar;


    showchar = ajStrStr(show[0])[0]; /* first char of 'show' */

    /* get the reference sequence */
    if(nrefseq == -1)
	ref = consensus;
    else
	ref = ajSeqsetGetSeq(seqset, nrefseq);

    /* convert the other sequences */
    for(i=0; i<ajSeqsetSize(seqset); i++)
	if(i != nrefseq)
	{
	    /* don't convert the reference sequence */
	    seq = ajSeqsetGetSeq(seqset, i);
	    switch(showchar)
	    {
	    case 'A':			/* All - no change, except for case */
		showalign_MakeAll(ref, seq, sub, cvt, similarcase);
		break;
	    case 'I':			/* Identities */
		showalign_MakeIdentity(ref, seq);
		break;
	    case 'N':			/* Non-identities */
		showalign_MakeNonidentity(ref, seq, sub, cvt, similarcase);
		break;
	    case 'S':			/* Similarities */
		showalign_MakeSimilar(ref, seq, sub, cvt, similarcase);
		break;
	    case 'D':			/* Dissimilarities */
		showalign_MakeDissimilar(ref, seq, sub, cvt);
		break;
	    default:
		ajFatal("Unknown option for '-show': %S", show[0]);
		break;
	    }
	}

    return;
}




/* @funcstatic showalign_MakeAll **********************************************
**
** Leave the sequence unchanged unless we are changing the case depending
** on the similarity to the reference sequence
**
** @param [r] ref [AjPSeq] the reference sequence
** @param [r] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint **] scoring matrix
** @param [r] cvt [AjPSeqCvt] conversion table for scoring matrix
** @param [r] similarcase [AjBool] change case depending on similarity
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeAll(AjPSeq ref, AjPSeq seq, ajint **sub,
			      AjPSeqCvt cvt, AjBool similarcase)
{
    ajint i;
    ajint lenseq;
    ajint lenref;

    char *s;
    char *r;

    lenseq = ajSeqLen(seq);
    lenref = ajSeqLen(ref);
    s = ajSeqChar(seq);
    r = ajSeqChar(ref);


    /* if not changing the case, do nothing */
    if(!similarcase)
	return;

    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	{
	    if(sub[ajSeqCvtK(cvt, r[i])][ajSeqCvtK(cvt, s[i])] <= 0)
		s[i] = tolower((int)s[i]);	/* dissimilar to lowercase */
	    else
		s[i] = toupper((int)s[i]);	/* similar to uppercase */
	}

    /* is seq longer than ref? */
    if(lenseq > lenref)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = tolower((int)s[i]);	/* dissimilar to lowercase */;

    return;
}




/* @funcstatic showalign_MakeIdentity *****************************************
**
** Convert 'seq' to '.'s except where identical to 'ref'
**
** @param [r] ref [AjPSeq] the reference sequence
** @param [r] seq [AjPSeq] the sequence to be changed
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeIdentity(AjPSeq ref, AjPSeq seq)
{

    ajint i;
    ajint lenseq;
    ajint lenref;

    char *s;
    char *r;

    lenseq = ajSeqLen(seq);
    lenref = ajSeqLen(ref);
    s = ajSeqChar(seq);
    r = ajSeqChar(ref);



    for(i=0; i<lenref && s[i] != '\0'; i++)
	if(toupper((int)s[i]) != toupper((int)r[i]) && s[i] != '-')
	    s[i] = '.';

    /* is seq longer than ref? */
    if(lenseq > lenref)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = '.';

    return;
}




/* @funcstatic showalign_MakeNonidentity **************************************
**
** Convert 'seq' to '.'s where identical to 'ref'
** Change case to lowercase where similar as given by scoring matrix.
** Change remaining dissimilar residues to uppercase.
**
** @param [r] ref [AjPSeq] the reference sequence
** @param [r] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint **] scoring matrix
** @param [r] cvt [AjPSeqCvt] conversion table for scoring matrix
** @param [r] similarcase [AjBool] change case depending on similarity
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeNonidentity(AjPSeq ref, AjPSeq seq, ajint **sub,
				      AjPSeqCvt cvt, AjBool similarcase)
{
    ajint i;
    ajint lenseq;
    ajint lenref;

    char *s;
    char *r;

    lenseq = ajSeqLen(seq);
    lenref = ajSeqLen(ref);
    s = ajSeqChar(seq);
    r = ajSeqChar(ref);


    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	{
	    if(toupper((int)s[i]) == toupper((int)r[i]))
		s[i] = '.';
	    else
	    {
		/* change case based on similarity? */
		if(similarcase)
		{
		    if(sub[ajSeqCvtK(cvt, r[i])][ajSeqCvtK(cvt, s[i])] <= 0)
			s[i] = toupper((int)s[i]);
		    else
			s[i] = tolower((int)s[i]);
		}
	    }
	}


    /* Is seq longer than ref? Make it all upppercase */
    if(lenseq > lenref && similarcase)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = toupper((int)s[i]);


    return;
}




/* @funcstatic showalign_MakeSimilar ******************************************
**
** Convert 'seq' to '.'s except where similar to 'ref', as defined by
** similarity scoring matrix.
** Change identities to upper-case, all others to lower-case.
**
** @param [r] ref [AjPSeq] the reference sequence
** @param [r] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint **] scoring matrix
** @param [r] cvt [AjPSeqCvt] conversion table for scoring matrix
** @param [r] similarcase [AjBool] change case depending on similarity
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeSimilar(AjPSeq ref, AjPSeq seq, ajint **sub,
				  AjPSeqCvt cvt, AjBool similarcase)
{
    ajint i;
    ajint lenseq;
    ajint lenref;

    char *s;
    char *r;


    lenseq = ajSeqLen(seq);
    lenref = ajSeqLen(ref);
    s = ajSeqChar(seq);
    r = ajSeqChar(ref);



    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	{
	    if(sub[ajSeqCvtK(cvt, r[i])][ajSeqCvtK(cvt, s[i])] <= 0)
		s[i] = '.';
	    else
	    {
		/* change case based on similarity? */
		if(similarcase)
		{
		    if(toupper((int)s[i]) == toupper((int)r[i]))
			s[i] = toupper((int)s[i]);
		    else
			s[i] = tolower((int)s[i]);
		}
	    }
	}


    /* is seq longer than ref? */
    if(lenseq > lenref)
	for(; i<lenseq; i++)
	    if(s[i] != '-')
		s[i] = '.';

    return;
}




/* @funcstatic showalign_MakeDissimilar ***************************************
**
** Convert 'seq' to '.'s where identical or similar to 'ref', as defined by
** similarity scoring matrix.
**
** @param [r] ref [AjPSeq] the reference sequence
** @param [r] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint **] scoring matrix
** @param [r] cvt [AjPSeqCvt] conversion table for scoring matrix
** @return [void]
** @@
******************************************************************************/

static void showalign_MakeDissimilar(AjPSeq ref, AjPSeq seq, ajint **sub,
				     AjPSeqCvt cvt)
{
    ajint i;
    ajint lenref;

    char *s;
    char *r;

    lenref = ajSeqLen(ref);
    s = ajSeqChar(seq);
    r = ajSeqChar(ref);

    for(i=0; i<lenref; i++)
	if(s[i] != '-')
	    if(sub[ajSeqCvtK(cvt, r[i])][ajSeqCvtK(cvt, s[i])] > 0)
		s[i] = '.';

    return;
}




/* @funcstatic showalign_Order ************************************************
**
** Orders the sequences
**
** @param [r] order [AjPStr *] how to order the sequences
** @param [r] seqset [AjPSeqset] input sequences
** @param [r] consensus [AjPSeq] consensus sequence
** @param [r] nrefseq [ajint] number of reference sequence
** @param [r] sub [ajint **] substitution matrix
** @param [r] cvt [AjPSeqCvt] substitution conversion table
** @param [u] aorder [AjOOrder *] output order to display the sequences
** @return [void]
** @@
******************************************************************************/

static void showalign_Order(AjPStr *order, AjPSeqset seqset, AjPSeq consensus,
			    ajint nrefseq, ajint **sub, AjPSeqCvt cvt,
			    AjOOrder *aorder)
{
    char orderchar;
    AjPSeq ref;
    ajint i;
    ajint j;
    ajint k;
    char *s;
    char *r;
    ajint rlen;
    ajint len;

    orderchar = ajStrStr(order[0])[0]; /* first char of 'order' */

    /* get the reference sequence */
    if(nrefseq == -1)
	ref = consensus;
    else
	ref = ajSeqsetGetSeq(seqset, nrefseq);

    rlen = ajSeqLen(ref);

    /* initialise all order positions as unused */
    for(i=0; i<ajSeqsetSize(seqset); i++)
	aorder[i].seq = NULL;


    /* do the ordering */
    switch(orderchar)
    {
    case 'I':				/* Input order */
	for(i=0, j=0; i<ajSeqsetSize(seqset); i++)
	    if(i != nrefseq)
		aorder[j++].seq = ajSeqsetGetSeq(seqset, i);
	break;

    case 'A':				/* Alphabetical name order */
	for(i=0, j=0; i<ajSeqsetSize(seqset); i++)
	    if(i != nrefseq)
		aorder[j++].seq = ajSeqsetGetSeq(seqset, i);

	/* sort alphabetically by name */
	qsort(aorder, j, sizeof(AjOOrder), showalign_CompareTwoSeqNames);
	break;

    case 'S':			/* Similarity to the reference sequence */
	if(nrefseq == -1)
	    r = ajSeqChar(consensus);
	else
	    r = ajSeqChar(ajSeqsetGetSeq(seqset, nrefseq));

	for(i=0, j=0; i<ajSeqsetSize(seqset); i++)
	    if(i != nrefseq)
	    {
		aorder[j].seq = ajSeqsetGetSeq(seqset, i);
		aorder[j].similarity = 0;
		len = ajSeqLen(aorder[j].seq);
		s = ajSeqChar(aorder[j].seq);
		for(k=0; k<len && k<rlen; k++)
		    aorder[j].similarity += sub[ajSeqCvtK(cvt,
			             r[k])][ajSeqCvtK(cvt,s[k])];
		j++;
	    }

	/* sort by similarity */
	qsort(aorder, j, sizeof(AjOOrder),
	      showalign_CompareTwoSeqSimilarities);
	break;

    default:
	ajFatal("Unknown option for '-order': %S", order[0]);
	break;
    }

    /* debug
       ajUser("sorted names:");
       for(i=0; i<ajSeqsetSize(seqset); i++)
       if(aorder[i].seq != NULL)
       ajUser("%s", ajSeqName(aorder[i].seq));

       */

    return;
}




/* @funcstatic showalign_Output ***********************************************
**
** Writes the sequences to the output file
**
** @param [r] outf [AjPFile] output file handle
** @param [r] seqset [AjPSeqset] the sequences
** @param [r] nrefseq [ajint] number of the refseq
** @param [r] width [ajint] width of displayed sequence line
** @param [r] margin [ajint] width of margin on left side
** @param [r] consensus [AjPSeq] consensus sequence
** @param [r] docon [AjBool] display consensus sequence at the bottom
** @param [r] bottom [AjBool] display refseq at the botton of the alignment
**                            as well
** @param [u] aorder [AjOOrder *] order to display the sequences
** @param [r] html [AjBool] format for html display
** @param [r] highlight [AjPRange] ranges to highlight
** @param [r] uppercase [AjPRange] ranges to uppercase
** @param [r] number [AjBool] display number line
** @param [r] ruler [AjBool] display ruler line
** @return [ajint] Always 0
** @@
******************************************************************************/

static ajint showalign_Output(AjPFile outf, AjPSeqset seqset, ajint nrefseq,
			      ajint width, ajint margin, AjPSeq consensus,
			      AjBool docon, AjBool bottom, AjOOrder * aorder,
			      AjBool html, AjPRange highlight,
			      AjPRange uppercase, AjBool number, AjBool ruler)
{
    ajint pos;		/* start position in sequences of next line */
    ajint i;
    ajint nseqs;
    AjPSeq ref;				/* the reference sequence */

    ajint begin;
    ajint end;

    nseqs = ajSeqsetSize(seqset);	/* number of sequences */
    begin = ajSeqsetBegin(seqset)-1;
    end   = ajSeqsetEnd(seqset)-1;


    /*
    **  if consensus line is the refseq, then aorder holds all the seqset
    **  sequences else it is one less than the seqset size
    */
    if(nrefseq != -1)
    {
	nseqs--;
	ref = ajSeqsetGetSeq(seqset, nrefseq);
    }
    else
	ref = consensus;


    /* format for html display */
    if(html)
	ajFmtPrintF(outf, "<pre>\n");

    /* get next set of lines to output */
    for(pos=begin; pos<=end; pos+=width)
    {
	/* numbers line */
	if(number)
	    showalign_OutputNums(outf, pos, width, margin);

	/* ruler/ticks line */
	if(ruler)
	    showalign_OutputTicks(outf, pos, width, margin);


	/* refseq is always displayed at the top if it is not the consensus */
        if(nrefseq != -1) 
        {
	    showalign_OutputSeq(outf, ref, pos, end, width, margin, html,
			    highlight, uppercase);
	} 
	else if(docon) 
	{
	    /* if refseq is consensus and docon is true then display consensus */
	    showalign_OutputSeq(outf, consensus, pos, end, width, margin,
			html, highlight, uppercase);
	}

	/* sequences */
	for(i=0; i<nseqs; i++)
	    showalign_OutputSeq(outf, aorder[i].seq, pos, end, width, margin,
				html, highlight, uppercase);


	/* refseq at the bottom also */
        if(nrefseq != -1) 
        {
	    if(bottom)
	        showalign_OutputSeq(outf, ref, pos, end, width, margin, html,
				highlight, uppercase);
            if(docon)
	        showalign_OutputSeq(outf, consensus, pos, end, width, margin,
			    html, highlight, uppercase);
	} 
	else if(docon && bottom)
	    showalign_OutputSeq(outf, consensus, pos, end, width, margin,
			html, highlight, uppercase);



	/* blank line */
	ajFmtPrintF(outf, "\n");
    }

    /* format for html display */
    if(html)
	ajFmtPrintF(outf, "</pre>\n");


    return 0;
}




/* @funcstatic showalign_OutputNums *******************************************
**
** Writes the numbers line
**
** @param [r]  outf [AjPFile] output file handle
** @param [r] pos [ajint] position in sequence to start line
** @param [r] width [ajint] width of line
** @param [r] margin [ajint] length of left hand margin
** @return [ajint] Always 0
** @@
******************************************************************************/

static ajint showalign_OutputNums(AjPFile outf, ajint pos, ajint width,
				  ajint margin)
{

    AjPStr line;
    ajint i;
    ajint firstpos;
    AjPStr marginfmt;

    line      = ajStrNewL(81);		/* line of ticks to print */
    marginfmt = ajStrNewL(10);

    /* margin and first number which may be partly in the margin */
    if(pos>0 && (ajint)log10((double)pos)+1 > margin + 10-(pos%10))
    {
	/* number is too long to fit in the margin, so just write spaces */
	ajStrAppKI(&line, ' ', margin+(10-(pos)%10));
	firstpos = pos + 10-(pos%10);
    }
    else
    {
	ajFmtPrintS(&marginfmt, "%%%dd", margin+(10-(pos)%10));
	firstpos = pos + 10-(pos%10);
	ajFmtPrintF(outf, ajStrStr(marginfmt), firstpos);
    }

    /* make the numbers line */
    for(i=firstpos+10; i<=pos+width; i+=10)
	ajFmtPrintAppS(&line, "%10d", i);

    /* print the numbers */
    ajFmtPrintF(outf, "%S\n", line);

    ajStrDel(&line);
    ajStrDel(&marginfmt);

    return 0;
}




/* @funcstatic showalign_OutputTicks ******************************************
**
** Writes the ticks line
**
** @param [r]  outf [AjPFile] output file handle
** @param [r] pos [ajint] position in sequence to start line
** @param [r] width [ajint] width of line
** @param [r] margin [ajint] length of left hand margin
** @return [ajint] Always 0
** @@
******************************************************************************/

static ajint showalign_OutputTicks(AjPFile outf, ajint pos, ajint width,
				   ajint margin)
{
    AjPStr line;
    ajint i;

    line = ajStrNewL(81);		/* line of ticks to print */


    /* margin */
    ajStrAppKI(&line, ' ', margin);

    /* make the ticks line */
    for(i=pos+1; i<pos+width+1; i++)
    {
	if(!(i % 10))
	    ajStrAppC(&line, "|");
	else if(!(i % 5))
	    ajStrAppC(&line, ":");
	else
	    ajStrAppC(&line, "-");
    }

    /* print the ticks */
    ajFmtPrintF(outf, "%S\n", line);

    /* tidy up */
    ajStrDel(&line);

    return 0;
}




/* @funcstatic showalign_OutputSeq ********************************************
**
** Writes the specified sequence line to the output file
**
** @param [r] outf [AjPFile] output file handle
** @param [r] seq [AjPSeq] the sequence to display
** @param [r] pos [ajint] position in sequence to start line
** @param [r] end [ajint] position to end sequence at
** @param [r] width [ajint] width of line
** @param [r] margin [ajint] length of left hand margin
** @param [r] html [AjBool] display formatted for html
** @param [r] highlight [AjPRange] ranges to highlight
** @param [r] uppercase [AjPRange] ranges to uppercase
** @return [ajint] Always 0
** @@
******************************************************************************/

static ajint showalign_OutputSeq(AjPFile outf, AjPSeq seq, ajint pos,
				 ajint end, ajint width, ajint margin,
				 AjBool html, AjPRange highlight,
				 AjPRange uppercase)
{
    AjPStr line;
    AjPStr marginfmt;

    line      = ajStrNew();		/* next line of sequence to print */
    marginfmt = ajStrNewL(10);

    /* get end to display up to */
    if(end > pos+width-1)
	end = pos+width-1;

    /* the bit of the sequence to be output */
    ajStrAssSub(&line, ajSeqStr(seq), pos, end);

    /* name of sequence */
    ajFmtPrintS(&marginfmt, "%%-%d.%dS", margin, margin);
    if(margin > 0)
	ajFmtPrintF(outf, ajStrStr(marginfmt), ajSeqGetName(seq));

    /* change required ranges to uppercase */
    embShowUpperRange(&line, uppercase, pos);

    /* +++ colour the sequences */

    /* highlight required ranges */
    if(html)
	embShowColourRange(&line, highlight, pos);

    /* print the sequence */
    ajFmtPrintF(outf, "%S\n", line);


    ajStrDel(&line);
    ajStrDel(&marginfmt);

    return 0;
}




/* @funcstatic showalign_CompareTwoSeqNames ***********************************
**
** Compare two Sequences' Names
**
** @param [r] a [const void *] First sequence
** @param [r] b [const void *] Second sequence
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showalign_CompareTwoSeqNames(const void * a, const void * b)
{
    return strcmp(ajSeqName((*(AjOOrder *)a).seq),
		  ajSeqName((*(AjOOrder *)b).seq));
}




/* @funcstatic showalign_CompareTwoSeqSimilarities ****************************
**
** Compare two Sequences by their similarity to the reference sequence
**
** @param [r] a [const void *] First sequence similarity
** @param [r] b [const void *] Second sequence similarity
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

static ajint showalign_CompareTwoSeqSimilarities(const void * a,
						 const void * b)
{
    return (*(AjOOrder *)b).similarity - (*(AjOOrder *)a).similarity;
}

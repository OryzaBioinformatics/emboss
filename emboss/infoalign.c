/* @source infoalign application
**
** Display information on a multiple sequence alignment with consensus
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 23 May 2001 - GWW - written
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
#include <ctype.h>

/* declare functions */

static int infoalign_Getrefseq (AjPStr refseq, AjPSeqset seqset);
static void infoalign_OutputFloat (AjPFile outfile, float num, AjBool html,
				   AjBool after);
static void infoalign_OutputInt (AjPFile outfile, ajint num, AjBool html,
				 AjBool after);
static void infoalign_OutputStr (AjPFile outfile, AjPStr str, AjBool html,
				 AjBool after, ajint minlength);
static void infoalign_Compare(AjPSeq ref, AjPSeq seq, ajint **sub,
			      AjPSeqCvt cvt, ajint *seqlength,
			      ajint *alignlength, ajint *gaps,
			      ajint *gapcount, ajint *idcount,
			      ajint *simcount, ajint *difcount, float *change);


#define NOLIMIT -1


/* @prog infoalign *******************************************************
**
** Information on a multiple sequence alignment
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqset seqset = NULL;
    AjPStr refseq;	/* input name/number of reference sequence */
    ajint  nrefseq;	/* numeric reference sequence */
    AjPMatrix matrix;	/* scoring matrix structure */
    ajint **sub;	/* integer scoring matrix */
    AjPSeqCvt  cvt=0;	/* conversion table for scoring matrix */
    float identity;
    ajint ident;
    float fplural; 
    AjPStr cons = ajStrNew();
    AjPSeq consensus = ajSeqNew();

    AjPSeq ref;
    AjPSeq seq;
    ajint i;
    
    AjBool html;
    AjBool doheader;
    AjBool dousa;
    AjBool doname;
    AjBool doseqlength;
    AjBool doalignlength;
    AjBool dogaps;
    AjBool dogapcount;    
    AjBool doidcount;    
    AjBool dosimcount;    
    AjBool dodifcount;    
    AjBool dochange;    
    AjBool dodesc;
    ajint  seqlength;
    ajint  alignlength;
    ajint  gaps;
    ajint  gapcount;
    ajint  idcount;
    ajint  simcount;
    ajint  difcount;
    float  change;
    
    AjPFile outfile;
    
    AjPStr usa;
    AjPStr name;
    AjPStr altusa=ajStrNewC("-");	/* default name when the real name
					   is not known */
    AjPStr altname=ajStrNewC("-");
    /*    ajint length; */
    AjPStr desc;
    AjPStr xxx=NULL;
    
    (void) embInit ("infoalign", argc, argv);
    

    seqset = ajAcdGetSeqset ("sequence");
    refseq = ajAcdGetString ("refseq");
    matrix  = ajAcdGetMatrix ("matrix");

    outfile = ajAcdGetOutfile ("outfile");
    
    html = ajAcdGetBool("html");
    doheader = ajAcdGetBool("heading");
    dousa = ajAcdGetBool("usa");
    doname = ajAcdGetBool("name");
    doseqlength = ajAcdGetBool("seqlength");
    doalignlength = ajAcdGetBool("alignlength");
    dogaps = ajAcdGetBool("gaps");
    dogapcount = ajAcdGetBool("gapcount");
    doidcount = ajAcdGetBool("idcount");
    dosimcount = ajAcdGetBool("simcount");
    dodifcount = ajAcdGetBool("diffcount");
    dochange = ajAcdGetBool("change");
    dodesc = ajAcdGetBool("description");

    /* consensus parameters */
    fplural   = ajAcdGetFloat("plurality");
    identity  = ajAcdGetFloat("identity");

    /* get conversion table and scoring matrix */
    cvt = ajMatrixCvt(matrix);
    sub = ajMatrixArray(matrix);  

    /* get the number of the reference sequence */
    nrefseq = infoalign_Getrefseq(refseq, seqset);
  
    /* change the % plurality to the fraction of absolute total weight */
    fplural = ajSeqsetTotweight(seqset) * fplural / 100;

    /* change the % identity to the number of identical sequences at a
       position required for consensus */
    ident = ajSeqsetSize(seqset) * identity / 100;

    /* get the consensus sequence */
    embConsCalc (seqset, matrix, ajSeqsetSize(seqset), ajSeqsetLen(seqset),
		 fplural, 0.0, ident, &cons);
    ajSeqAssSeq(consensus, cons);	/* set the sequence string */
    /* name the sequence */
    ajSeqAssName(consensus, (xxx=ajStrNewC("Consensus")));

    /* get the reference sequence */
    if (nrefseq == -1)
	ref = consensus;
    else
	ref = ajSeqsetGetSeq(seqset, nrefseq);


    
    /* start the HTML table */
    if (html)
	(void) ajFmtPrintF(outfile,
			   "<table border cellpadding=4 bgcolor="
			   "\"#FFFFF0\">\n");
    
    /* print the header information */
    if (doheader)
    {
	if (html)			/* start the HTML table title line and
					   output the Name header */
	    (void) ajFmtPrintF(outfile, "<tr>");
	else
	    (void) ajFmtPrintF(outfile, "%s", "# ");
    
	if (dousa)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>USA</th>");
	    else
		(void) ajFmtPrintF(outfile, "%-16s", "USA");
	}

	if (doname)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Name</th>");
	    else
		(void) ajFmtPrintF(outfile, "%-12s", "Name");
	}

	if (doseqlength)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Sequence Length</th>");
	    else
		(void) ajFmtPrintF(outfile, "SeqLen\t");
	}

	if (doalignlength)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Aligned Length</th>");
	    else
		(void) ajFmtPrintF(outfile, "AlignLen\t");
	}

	if (dogaps)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Gaps</th>");
	    else
		(void) ajFmtPrintF(outfile, "Gaps\t");
	}

	if (dogapcount)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Gap Length</th>");
	    else
		(void) ajFmtPrintF(outfile, "GapLen\t");
	}

	if (doidcount)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Identity</th>");
	    else
		(void) ajFmtPrintF(outfile, "Ident\t");
	}

	if (dosimcount)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Similarity</th>");
	    else
		(void) ajFmtPrintF(outfile, "Similar\t");
	}

	if (dodifcount)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Difference</th>");
	    else
		(void) ajFmtPrintF(outfile, "Differ\t");
	}

	if (dochange)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>%% Change</th>");
	    else
		(void) ajFmtPrintF(outfile, "%% Change\t");
	}

	if (dodesc)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Description</th>");
	    else
		(void) ajFmtPrintF(outfile, "Description");
	}

	/* end the HTML table title line */
	if (html) 
	    (void) ajFmtPrintF(outfile, "</tr>\n");
	else
	    (void) ajFmtPrintF(outfile, "\n");
    }  	


    for (i=0; i<ajSeqsetSize(seqset); i++)
    {
    	seq = ajSeqsetGetSeq(seqset, i);

	/* get the usa ('-' if unknown) */    
	usa = ajSeqGetUsa(seq);
	if (ajStrLen(usa) == 0)
	    usa = altusa;

	/* get the name ('-' if unknown) */    
	name = ajSeqGetName(seq);
	if (ajStrLen(name) == 0)
	    name = altname;

	/*	length = ajSeqLen(seq); */
	desc = ajSeqGetDesc(seq);

	/* get the stats from the comparison to the reference sequence */
	infoalign_Compare(ref, seq, sub, cvt, &seqlength, &alignlength, 
			  &gaps, &gapcount, &idcount, &simcount,
			  &difcount, &change);


	/* start table line */
	if (html)
	    ajFmtPrintF(outfile, "<tr>");

	if (dousa)
	    infoalign_OutputStr(outfile, usa, html,
				(dodesc || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount || dogaps ||
				 doseqlength || doalignlength || doname), 18);

	if (doname)
	    infoalign_OutputStr(outfile, name, html,
				(dodesc || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount || dogaps ||
				 doseqlength || doalignlength), 14);

	if (doseqlength)
	    infoalign_OutputInt(outfile, seqlength, html,
				(dodesc || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount ||
				 dogaps || doalignlength));

	if (doalignlength)
	    infoalign_OutputInt(outfile, alignlength, html,
				(dodesc || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount || dogaps));

	if (dogaps)
	    infoalign_OutputInt(outfile, gaps, html,
				(dodesc || dochange ||
				 dodifcount || dosimcount ||
				 doidcount || dogapcount));

	if (dogapcount)
	    infoalign_OutputInt(outfile, gapcount, html,
				(dodesc || dochange ||
				 dodifcount || dosimcount ||
				 doidcount));

	if (doidcount)
	    infoalign_OutputInt(outfile, idcount, html,
				(dodesc || dochange || dodifcount ||
				 dosimcount));

	if (dosimcount)
	    infoalign_OutputInt(outfile, simcount, html, (dodesc || dochange ||
							  dodifcount));

	if (dodifcount)
	    infoalign_OutputInt(outfile, difcount, html, (dodesc || dochange));

	if (dochange)
	    infoalign_OutputFloat(outfile, change, html, dodesc);

	if (dodesc)
	    infoalign_OutputStr(outfile, desc, html, ajFalse, NOLIMIT);

	/* end table line */
	if (html)
	    (void) ajFmtPrintF(outfile, "</tr>\n");
	else
	    (void) ajFmtPrintF(outfile, "\n");
    }


    /* end the HTML table */
    if (html)
	(void) ajFmtPrintF(outfile, "</table>\n");

    (void) ajFileClose(&outfile);

    /* tidy up */
    ajStrDel(&altusa);
    ajStrDel(&altname);
    ajStrDel(&xxx);
    ajSeqDel(&consensus);

    ajExit();
    return 0;
}




/* @funcstatic infoalign_OutputFloat ***************************************
**
** Output a number.
** If this is not formatted for HTML, and is not the last item output
** then add a TAB after it, else ensure that the last data item has no
** characters after it.
**
** @param [r] outfile [AjPFile] output file handle
** @param [r] num [float] number to be printed
** @param [r] html [AjBool] True if want it formatted as an HTML table data item
** @param [r] after [AjBool] True if we want to output more columns after this one
** @return [void] 
** @@
******************************************************************************/

static void infoalign_OutputFloat (AjPFile outfile, float num, AjBool html,
				   AjBool after)
{
    if (html)
	(void) ajFmtPrintF(outfile, "<td>");	

    (void) ajFmtPrintF(outfile, "%f", num);  

    if (html)
	(void) ajFmtPrintF(outfile, "</td>\n");	
    else
	if (after)
	    (void) ajFmtPrintF(outfile, "\t");

    return;
}




/* @funcstatic infoalign_OutputInt ****************************************
**
** Output a number.
** If this is not formatted for HTML, and is not the last item output
** then add a TAB after it, else ensure that the last data item has no
** characters after it.
**
** @param [r] outfile [AjPFile] output file handle
** @param [r] num [ajint] number to be printed
** @param [r] html [AjBool] True if want it formatted as an HTML table data item
** @param [r] after [AjBool] True if we want to output more columns after this one
** @return [void] 
** @@
******************************************************************************/

static void infoalign_OutputInt (AjPFile outfile, ajint num, AjBool html,
				 AjBool after)
{

    if (html)
	(void) ajFmtPrintF(outfile, "<td>");	

    (void) ajFmtPrintF(outfile, "%d", num);  
    if (html)
	(void) ajFmtPrintF(outfile, "</td>\n");	
    else
	if (after)
	    (void) ajFmtPrintF(outfile, "\t");

    return;
}




/* @funcstatic infoalign_OutputStr ********************************************
**
** Output the string.
** If this is not formatted for HTML, and is not the last item output
** then try to format it within a minimum length field.
** If it is longer than this field, just add a TAB.
**
** @param [r] outfile [AjPFile] output file handle
** @param [r] str [AjPStr] string to be printed
** @param [r] html [AjBool] True if want it formatted as an HTML table data item
** @param [r] after [AjBool] True if we want to output more columns after this one
** @param [r] minlength [ajint] minimum length of field to print the string in (NOLIMIT = no limit)
** @return [void] 
** @@
******************************************************************************/

static void infoalign_OutputStr (AjPFile outfile, AjPStr str, AjBool html,
				 AjBool after, ajint minlength)
{
    AjPStr marginfmt=ajStrNewL(10);
  
    /* ajFmtPrintF doesn't seem to deal with formats like "%-*S"
       correctly, so ...  */
    ajFmtPrintS(&marginfmt, "%%-%dS", minlength);
     
    if (html)
	(void) ajFmtPrintF(outfile, "<td>");	

    if (html || !after || minlength == NOLIMIT)
	(void) ajFmtPrintF(outfile, "%S", str);
    else
	/* 
	 *  Make the formatting nice:
	 *  If this is the last item, don't put spaces or TABs after it.
	 *  Try to fit the name in 'minlength' spaces, else just add a
	 *  TAB after it
	 */
	(void) ajFmtPrintF(outfile, ajStrStr(marginfmt), str);

    if (html)
	(void) ajFmtPrintF(outfile, "</td>\n");	
    else
	if (after &&  ajStrLen(str) >= minlength)
	    (void) ajFmtPrintF(outfile, "\t");

	
    /* tidy up */
    ajStrDel(&marginfmt);

    return;
}




/* @funcstatic infoalign_Getrefseq ********************************************
**
** Determines which sequence should be the reference sequence.
** The first sequence in the set is returned as 0.
** -1 is returned as the consensus sequence.
**
** @param [r] refseq [AjPStr] input name/number of reference sequence 
** @param [r] seqset [AjPSeqset] the sequences 
** @return [int] the number of the reference sequence
** @@
******************************************************************************/

static int infoalign_Getrefseq (AjPStr refseq, AjPSeqset seqset)
{
    ajint i;
    AjPSeq seq;

    for (i=0; i<ajSeqsetSize(seqset); i++)
    {
	seq = ajSeqsetGetSeq(seqset, i);
	if (!ajStrCmpO(ajSeqGetName(seq), refseq))
	    return i;
    }

    /* not a name of a sequence, so it must be a number */
    if (ajStrToInt(refseq, &i))
    {
	if (i < 0 || i > ajSeqsetSize(seqset))
	    ajFatal("Reference sequence number < 0 or > number "
		    "of input sequences: %d", i);
	return i-1;
    }
    else
	ajFatal("Reference sequence is not a sequence ID or a number: %S",
		refseq);

    return -1;		/* to make the compiler happy */
}
 



/* @funcstatic infoalign_Compare ********************************************
**
** Returns lots of trivial measures of comparison between a sequence and a
** reference sequence
**
** @param [r] ref [AjPSeq] the reference sequence
** @param [r] seq [AjPSeq] the sequence to be changed
** @param [r] sub [ajint **] scoring matrix
** @param [r] cvt [AjPSeqCvt] conversion table for scoring matrix
** @param [r] seqlength [ajint *] sequence length
** @param [r] alignlength [ajint *] alignment length
** @param [r] gaps [ajint *] number of gaps
** @param [r] gapcount [ajint *] number of gap characters
** @param [r] idcount [ajint *] number of identical positions
** @param [r] simcount [ajint *] number of similar positions
** @param [r] difcount [ajint *] number of different positions
** @param [r] change [float *] % difference 
** @return [void]
** @@
******************************************************************************/

static void infoalign_Compare(AjPSeq ref, AjPSeq seq, ajint **sub,
			      AjPSeqCvt cvt, ajint *seqlength,
			      ajint *alignlength, ajint *gaps,
			      ajint *gapcount, ajint *idcount,
			      ajint *simcount, ajint *difcount,
			      float *change) 
{
    ajint i;

    ajint lenseq = ajSeqLen(seq);
    ajint lenref = ajSeqLen(ref);

    char *s = ajSeqChar(seq);	/* the original char * of the sequence */
    char *r = ajSeqChar(ref);

    AjBool inGap = ajFalse;
    ajint begin, end;

    /* initialise counts */
    *seqlength = 0;
    *alignlength = 0;
    *gaps = 0;
    *gapcount = 0;
    *idcount = 0;
    *simcount = 0;
    *difcount = 0;
    *change = 0.0;

    /* ignore gaps at the ends of the sequence */
    for (begin = 0; s[begin] == '-'; begin++);
    for (end = lenseq-1; s[end] == '-'; end--);
  
    for (i=begin; i<=end; i++)
    {
	/* count gaps and their length */
	if (s[i] == '-')
	{
	    if (!inGap)
	    {
		inGap = ajTrue;
		(*gaps)++;
	    }
	    (*gapcount)++;
	}
	else
	    inGap = ajFalse;


	/* count identity, similarity, differences */
	/* are we past the end of the reference sequence ? */
	if (i >= lenref)
	    (*difcount)++;
	else
	{
	    /* identity */
	    if (toupper(r[i]) == toupper(s[i]))
		(*idcount)++;
	    /* similarity */
	    else if (sub[ajSeqCvtK(cvt, r[i])][ajSeqCvtK(cvt, s[i])] > 0)
		(*simcount)++;
	    /* difference */
	    else
		(*difcount)++;
	}
    }

    /* get lengths */
    /* length of sequence excluding gaps */
    *seqlength = *idcount + *simcount + *difcount;;
    /* length of aligned sequence with internal gaps */
    *alignlength = *gapcount + *seqlength;

    /* calculate change */
    *change = (float)(end-begin+1 - *idcount)*100.0/(float)(end-begin+1);

    return;
}



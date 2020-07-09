/* @source diffseq application
** 
** Find differences (SNPs) between nearly identical sequences
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

static void diffseq_diff (AjPList matchlist, AjPSeq seq1, AjPSeq seq2,
			  AjPFile outfile, AjBool columns);
static void diffseq_diffrpt (AjPList matchlist, AjPSeq seq1, AjPSeq seq2,
			     AjPReport report, AjPFeattable ftab,
			     AjBool columns);


static void diffseq_WordMatchListConvDiffToFeat(AjPList list,
						AjPFeattable *tab1,
						AjPFeattable *tab2,
						AjPSeq seq1, AjPSeq seq2);

static void diffseq_Features(AjPFile outfile, AjPFeattable feat, ajint start,
			     ajint end);
static void diffseq_FeaturesRpt(char* typefeat, AjPFeature rf,
				AjPFeattable feat,
				ajint start, ajint end);
static void diffseq_AddTags(AjPFile outfile, AjPFeature feat, AjBool values);
static void diffseq_AddTagsRpt(AjPStr* strval, AjPFeature feat,
			       AjBool values);

                                

/* @prog diffseq **************************************************************
**
** Find differences (SNPs) between nearly identical sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
  
    AjPSeq seq1,seq2;
    ajint wordlen;
    AjPTable seq1MatchTable =0 ;
    AjPList matchlist=NULL ;
    AjPFile outfile = NULL;
    AjPReport report;
    AjPFeattable Tab1=NULL,Tab2=NULL;
    AjPFeattable TabRpt = NULL;
    AjPFeattabOut seq1out = NULL;
    AjPFeattabOut seq2out = NULL;
    AjBool columns;		/* format output report files in columns */
    AjPStr tmpstr=NULL;

    embInit("diffseq", argc, argv);

    wordlen = ajAcdGetInt ("wordsize");
    seq1 = ajAcdGetSeq ("asequence");
    seq2 = ajAcdGetSeq ("bsequence");
    report = ajAcdGetReport ("outfile");
    seq1out = ajAcdGetFeatout("afeatout");
    seq2out = ajAcdGetFeatout("bfeatout");
    columns = ajAcdGetBool ("columns");

    /*
    ** Replaced by report. Original code exists
    ** but is skipped if outfile is NULL
    **
    outfile = ajAcdGetOutfile ("outfile");
    */

    ajSeqTrim(seq1);
    ajSeqTrim(seq2);

    TabRpt = ajFeattableNewSeq(seq1);

    embWordLength (wordlen);
    if(embWordGetTable(&seq1MatchTable, seq1))
	/* get table of words */
	matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2, ajTrue);


    /* get the minimal set of overlapping matches */    
    if (matchlist)
	(void) embWordMatchMin(matchlist, ajSeqLen(seq1), ajSeqLen(seq2));

  
    if (matchlist)
    {
	/* output the gff files */                                
	(void) diffseq_WordMatchListConvDiffToFeat(matchlist, &Tab1, &Tab2, 
						   seq1, seq2);

	/* make the output file */
	if (outfile)
	  (void) diffseq_diff (matchlist, seq1, seq2, outfile, columns);

	(void) diffseq_diffrpt (matchlist, seq1, seq2,
				report, TabRpt, columns);
    
	/* tidy up */
	embWordMatchListDelete(&matchlist); /* free the match structures */
    }
   
    /* tidy up */
    if (outfile)
      (void) ajFileClose(&outfile);

    ajFeatWrite(seq1out, Tab1);
    ajFeatWrite(seq2out, Tab2);

    tmpstr=NULL;
    ajFmtPrintS(&tmpstr, "Feature file for first sequence");
    ajReportFileAdd (report, ajFeattabOutFile(seq1out), tmpstr);

    tmpstr=NULL;
    ajFmtPrintS(&tmpstr, "Feature file for second sequence");
    ajReportFileAdd (report, ajFeattabOutFile(seq2out), tmpstr);

    ajReportWrite(report, TabRpt, seq1);
    (void) ajFeattableDel(&TabRpt);
    (void) ajReportClose(report);
    ajReportDel(&report);
    
    ajExit();
    return 0;
}


/* @funcstatic diffseq_WordMatchListConvDiffToFeat ***************************
**
** convert the word table differences to feature tables.
**
** @param [Pr] list [AjPList] non-overlapping match list to be printed
**                            (sorted by position).
** @param [rw] tab1 [AjPFeattable*] feature table for sequence 1
** @param [rw] tab2 [AjPFeattable*] feature table for sequence 2
** @param [r] seq1 [AjPSeq] sequence 1
** @param [r] seq2 [AjPSeq] sequence 2
** @return [void]
** @@
******************************************************************************/

static void diffseq_WordMatchListConvDiffToFeat(AjPList list,
						AjPFeattable *tab1,
						AjPFeattable *tab2,
						AjPSeq seq1, AjPSeq seq2)
{
    char strand='+';
    ajint frame=0;
    AjPStr source=NULL;
    AjPStr type=NULL;
    AjPStr note=NULL;
    AjPStr replace=NULL;
    AjPFeature feature;
    AjIList iter=NULL;
    ajint misstart1 = -1;		/* start of mismatch region in seq1 */
    ajint misstart2 = -1;		/* start of mismatch region in seq2 */
    ajint misend1;
    ajint misend2;			/* end of mismatch region */
    AjPStr notestr=NULL, replacestr=NULL;
    float score = 0.0;

    if(!*tab1)
	*tab1 = ajFeattableNewSeq(seq1);
    if(!*tab2)
	*tab2 = ajFeattableNewSeq(seq2);

    source     = ajStrNew();
    type       = ajStrNew();
    note       = ajStrNew();
    replace    = ajStrNew();
    replacestr = ajStrNew();
    notestr    = ajStrNew();
  
    ajStrAssC(&source,"diffseq");
    ajStrAssC(&type,"conflict");
    ajStrAssC(&note,"note");
    ajStrAssC(&replace,"replace");
    score = 1.0;

    iter = ajListIter(list);
    while(ajListIterMore(iter))
    {
	EmbPWordMatch p = (EmbPWordMatch) ajListIterNext (iter) ;

	misend1 = p->seq1start-1;
	misend2 = p->seq2start-1;

	if (misstart1 != -1)
	{
	    /* check that we have seen a match already */
	    if (misstart1 <= misend1)
	    {	/* is there a gap between the matches? */
		feature = ajFeatNew(*tab1, source, type,
				    misstart1+1, misend1+1, 
				    score, strand, frame) ;
		if (misstart1 == misend1 && misstart2 == misend2)
		    ajFmtPrintS(&notestr, "SNP in %S", ajSeqGetName(seq2));
		else if (misstart2 > misend2)
		    ajFmtPrintS(&notestr, "Insertion of %d bases in %S",
				misend1 - misstart1 +1, ajSeqGetName(seq1));
		else
		    /*
		     *  ajFmtPrintS(&notestr, "%S mismatch length=%d.
		     *  %S mismatch length=%d.", ajSeqGetName(seq1),
		     *  misend1 - misstart1 +1, ajSeqGetName(seq2),
		     *  misend2 - misstart2 +1);
		     */
		    ajFmtPrintS(&notestr, "%S", ajSeqGetName(seq2));

		ajFeatTagSet(feature,  note, notestr);

		if (misstart2 <= misend2)
		    ajStrAssSub(&replacestr, ajSeqStr(seq2), misstart2,
				misend2);
		else
		    ajStrAssC(&replacestr, "");

		ajFeatTagSet(feature, replace, replacestr);
	    }

	    if (misstart2 <= misend2)
	    {	/* is there a gap between the matches? */
		feature = ajFeatNew(*tab2, source, type,
				    misstart2+1, misend2+1, 
				    score, strand, frame) ;

		if (misstart2 == misend2 && misstart1 == misend1)
		    ajFmtPrintS(&notestr, "SNP in %S", ajSeqGetName(seq1));
		else if (misstart1 > misend1)
		    ajFmtPrintS(&notestr, "Insertion of %d bases in %S",
				misend2 - misstart2 +1, ajSeqGetName(seq2));
		else
		    /*
		     *  ajFmtPrintS(&notestr, "%S mismatch length=%d.
		     *  %S mismatch length=%d.", ajSeqGetName(seq2),
		     *  misend2 - misstart2 +1, ajSeqGetName(seq1),
		     *  misend1 - misstart1 +1);
		     */
		    ajFmtPrintS(&notestr, "%S", ajSeqGetName(seq1));

		ajFeatTagSet(feature, note, notestr);

		if (misstart1 <= misend1)
		    ajStrAssSub(&replacestr, ajSeqStr(seq1), misstart1,
				misend1);
		else
		    ajStrAssC(&replacestr, "");

		ajFeatTagSet(feature, replace, replacestr);
	    }

	}
    
	/* note the start position of the mismatch */
	misstart1 = misend1 + p->length +1;
	misstart2 = misend2 + p->length +1;

    }
    /* delete the iterator */
    ajListIterFree(iter);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&note);
    ajStrDel(&replace);
    ajStrDel(&replacestr);
    ajStrDel(&notestr);

    return;
}




/* @funcstatic diffseq_AddTags ***********************************************
**  
** Writes the tags to the output file
** Don't write out the translation - is it often far too long!
**
** Obsolete. Not used unless outfile is reenabled.
**
** @param [r] outfile [AjPFile] output file
** @param [r] feat [AjPFeature] Feature to be processed
** @param [r] values [AjBool] display values of tags
**
** @return [void]
** @@
******************************************************************************/

static void diffseq_AddTags(AjPFile outfile, AjPFeature feat, AjBool values)
{
    AjIList titer;			/* iterator for taglist */
    static AjPStr tagnam = NULL;
    static AjPStr tagval = NULL;

    /*
    ** Obsolete. Return now unless outfile is defined
    */

    if (!outfile)
      return;

    /* iterate through the tags and test for match to patterns */

    titer = ajFeatTagIter(feat);
    while (ajFeatTagval (titer, &tagnam, &tagval))
	/* don't display the translation tag - it is far too long :-) */
	if (ajStrCmpC(tagnam, "translation"))
	{
	    if (values == ajTrue)
		(void) ajFmtPrintF(outfile, " %S=%S", tagnam, tagval);
	    else
		(void) ajFmtPrintF(outfile, " %S", tagnam);
	}

    (void) ajListIterFree(titer);
    (void) ajFmtPrintF(outfile, "\n");

    return;
}

/* @funcstatic diffseq_AddTagsRpt *********************************************
**  
** Appends feature tag values to a string in a simple format.
** Don't write out the translation - is it often far too long!
**
** @param [r] strval [AjPStr*] String
** @param [r] feat [AjPFeature] Feature to be processed
** @param [r] values [AjBool] display values of tags
**
** @return [void]
** @@
******************************************************************************/

static void diffseq_AddTagsRpt (AjPStr* strval, AjPFeature feat, AjBool values)
{
    AjIList titer;			/* iterator for taglist */
    static AjPStr tagnam = NULL;
    static AjPStr tagval = NULL;

    /* iterate through the tags and test for match to patterns */

    titer = ajFeatTagIter(feat);
    while (ajFeatTagval (titer, &tagnam, &tagval))
	/* don't display the translation tag - it is far too long :-) */
	if (ajStrCmpC(tagnam, "translation"))
	{
	    if (values == ajTrue)
		(void) ajFmtPrintAppS(strval, " %S='%S'", tagnam, tagval);
	    else
		(void) ajFmtPrintAppS(strval, " %S", tagnam);
	}

    (void) ajListIterFree(titer);

    return;
}

/* @funcstatic diffseq_Features *****************************************
**
** Write out any features which overlap this region.
** Don't write out the source feature - far too irritating!
**
** Obsolete. Only called if outfile is defined
**
** @param [r] outfile [AjPFile] Output file containing report.
** @param [r] feat [AjPFeattable] Feature table to search
** @param [r] start [ajint] Start position of region (in human coordinates)
** @param [r] end [ajint] End position of region (in human coordinates)
** @return [void] 
** @@
******************************************************************************/

static void diffseq_Features(AjPFile outfile, AjPFeattable feat, ajint start,
			     ajint end)
{
    AjIList    iter = NULL ;
    AjPFeature gf   = NULL ;
      
    /*
    ** Obsolete. Return now unless outfile is defined.
    */

    if (!outfile)
      return;

    if(!feat)
	return;

    if (feat->Features)
    {
	iter = ajListIter(feat->Features) ;
	while(ajListIterMore(iter))
	{
	    gf = ajListIterNext (iter) ;

	    /* check that the feature is within the range we wish to display */
	    if (start > ajFeatGetEnd(gf) || end < ajFeatGetStart(gf))
		continue;

	    /* don't output the 'source' feature - it is very irritating! */
	    if (!ajStrCmpC(ajFeatGetType(gf), "source"))
		continue;

	    /* write out the feature details */
	    (void) ajFmtPrintF(outfile, "Feature: %S %d-%d", ajFeatGetType(gf),
			       ajFeatGetStart(gf), ajFeatGetEnd(gf));
	    (void) diffseq_AddTags(outfile, gf, ajTrue);
	}
	ajListIterFree(iter) ;
    }

    return;
}

/* @funcstatic diffseq_Features *****************************************
**
** Write out any features which overlap this region.
** Don't write out the source feature - far too irritating!
**
** @param [r] typefeat [char*] Report feature tag type
** @param [r] rf [AjPFeature] Report feature to store results in
** @param [r] feat [AjPFeattable] Feature table to search
** @param [r] start [ajint] Start position of region (in human coordinates)
** @param [r] end [ajint] End position of region (in human coordinates)
** @return [void] 
** @@
******************************************************************************/

static void diffseq_FeaturesRpt(char* typefeat, AjPFeature rf,
				AjPFeattable feat,
				ajint start, ajint end)
{
    AjIList    iter = NULL ;
    AjPFeature gf   = NULL ;
    AjPStr tmp = NULL;

    if(!feat)
	return;

    if (feat->Features)
    {
	iter = ajListIter(feat->Features) ;
	while(ajListIterMore(iter))
	{
	    gf = ajListIterNext (iter) ;

	    /* check that the feature is within the range we wish to display */
	    if (start > ajFeatGetEnd(gf) || end < ajFeatGetStart(gf))
		continue;

	    /* don't output the 'source' feature - it is very irritating! */
	    if (!ajStrCmpC(ajFeatGetType(gf), "source"))
		continue;

	    /* write out the feature details */
	    (void) ajFmtPrintS(&tmp, "*%s %S %d-%d",
			       typefeat, ajFeatGetType(gf),
			       ajFeatGetStart(gf), ajFeatGetEnd(gf));
	    (void) diffseq_AddTagsRpt(&tmp, gf, ajTrue);
	    ajFeatTagAdd (rf, NULL,  tmp);
	}
	ajListIterFree(iter) ;
    }

    ajStrDel(&tmp);
    return;
}




/* @funcstatic diffseq_diff ********************************************
**
** Do a diff and write a report on the diff of the two sequences.
**
** Obsolete. Only used if outfile is defined.
**
** @param [r] matchlist [AjPList] List of minimal non-overlapping matches
** @param [r] seq1 [AjPSeq] Sequence to be diff'd.
** @param [r] seq2 [AjPSeq] Sequence to be diff'd.
** @param [r] outfile [AjPFile] Output file containing report.
** @param [r] columns [AjBool] format in columns
** @return [void] 
** @@
******************************************************************************/

static void diffseq_diff (AjPList matchlist, AjPSeq seq1, AjPSeq seq2, AjPFile
			  outfile, AjBool columns)
{

    AjIList iter=NULL;			/* match list iterator */
    EmbPWordMatch p=NULL;		/* match structure */
    ajint count=0;			/* count of matches */
    AjPStr s1 = ajSeqStr(seq1);		/* string of seq1 */
    AjPStr s2 = ajSeqStr(seq2);		/* string of seq2 */
    ajint prev1end=0;
    ajint prev2end=0;		/* end positions (+1) of previous match */
    AjPStr tmp = ajStrNew();		/* temporary string */
    AjPStr name1 = ajStrNewC(ajSeqName(seq1)); /* name of seq1 */
    AjPStr name2 = ajStrNewC(ajSeqName(seq2)); /* name of seq2 */
    ajint start;
    ajint end;	/* start and end of the difference (using human coords) */

    /* stuff for counting SNPs, transitions & transversions */
    ajint snps=0;
    ajint transitions=0;
    ajint transversions = 0;		/* counts of SNP types */
    char base1='\0', base2='\0';
    ajint len1, len2;

    /* get the feature table of the sequences */
    AjPFeattable feat1 = ajSeqCopyFeat(seq1);
    AjPFeattable feat2 = ajSeqCopyFeat(seq2);


    /*
    ** Obsolete. Return now unless outfile is defined.
    */

    if (!outfile)
      return;

    /* title line */
    (void) ajFmtPrintF(outfile, "# Report of diffseq of: %S and %S\n\n",
		       name1, name2);

    iter = ajListIter(matchlist);
    while(ajListIterMore(iter))
    {
	p = (EmbPWordMatch) ajListIterNext (iter) ;
	/* first match? */
	if (!count++)
	{
	    if (columns)
		(void) ajFmtPrintF(outfile, "# ");
	    (void) ajFmtPrintF(outfile, "%S overlap starts at %d\n", name1,
			       p->seq1start+1);
	    if (columns)
		(void) ajFmtPrintF(outfile, "# ");
	    (void) ajFmtPrintF(outfile, "%S overlap starts at %d\n\n", name2,
			       p->seq2start+1);

	    if (columns)
		(void) ajFmtPrintF(outfile, "# (%S) start end length sequence "
				   " (%S) start end length sequence\n\n",
				   name1, name2);

	}
	else
	{
	    /* output the difference between the matching regions */   
	    /* seq1 details */
	    start = prev1end+1;
	    end = p->seq1start;
	    if (prev1end<p->seq1start)
	    {
		if (columns)
		    (void) ajFmtPrintF(outfile, "%d\t%d\t%d\t", start, end,
				       end-start+1);
		else
		    (void) ajFmtPrintF(outfile, "\n%S %d-%d Length: %d\n",
				       name1, start, end, end-start+1);
		ajStrAssSub(&tmp, s1, prev1end, p->seq1start-1);
		len1=end-start+1;
		base1 = * ajStrStr(tmp);
	    }
	    else
	    {
		if (columns)
		    (void) ajFmtPrintF(outfile, "%d\t%d\t0\t", start-1,
				       start-1);
		else
		    (void) ajFmtPrintF(outfile, "\n%S %d Length: 0\n", name1,
				       start-1);
		ajStrAssC(&tmp, "");
		len1=0;
	    }
	    if (!columns)
		(void) diffseq_Features(outfile, feat1, start, end);

	    if (columns)
		(void) ajFmtPrintF(outfile, "'%S'\t", tmp);
	    else
		(void) ajFmtPrintF(outfile, "Sequence: %S\n", tmp);

	    /* seq2 details */
	    start = prev2end+1;
	    end = p->seq2start;
	    if (prev2end<p->seq2start)
	    {
		ajStrAssSub(&tmp, s2, prev2end, p->seq2start-1);
		if (columns)
		    (void) ajFmtPrintF(outfile, "%d\t%d\t%d\t'%S'\n", start,
				       end, end-start+1, tmp);
		else
		{
		    (void) ajFmtPrintF(outfile, "Sequence: %S\n", tmp);
		    if (!columns)
			(void) diffseq_Features(outfile, feat2, start, end);
		    (void) ajFmtPrintF(outfile, "%S %d-%d Length: %d\n",
				       name2, start, end, end-start+1);
		}
		len2=end-start+1;
		base2 = * ajStrStr(tmp);
	    }
	    else
	    {
		ajStrAssC(&tmp, "");
		if (columns)
		    (void) ajFmtPrintF(outfile, "%d\t%d\t0\t'%S'\n", start-1,
				       start-1, tmp);
		else
		{
		    (void) ajFmtPrintF(outfile, "Sequence: %S\n", tmp);
		    if (!columns)
			(void) diffseq_Features(outfile, feat2, start, end);
		    (void) ajFmtPrintF(outfile, "%S %d Length: 0\n", name2,
				       start-1);
		}
		len2=0;
	    }

	    /* count SNPs, transitions & transversions */
	    if (len1 == 1 && len2 == 1)
	    {
		snps++;
		transitions += (ajint) embPropTransition(base1, base2);
		transversions += (ajint) embPropTransversion(base1, base2);
	    }

	}
    
	/* output the match */
	/*
	 *  (void) ajFmtPrintF(outfile, "Matching region %S %d-%d : %S
	 *  %d-%d\n", name1, p->seq1start+1, p->seq1start + p->length,
	 *  name2, p->seq2start+1, p->seq2start + p->length);
	 * (void) ajFmtPrintF(outfile, "Length of match: %d\n", p->length);
	 */

	/*
	 *  note the end positions (+1) to get the intervening region
	 *  between matches
	 */
	prev1end = p->seq1start + p->length;
	prev2end = p->seq2start + p->length;

    }

    ajListIterFree(iter);

    /* end of overlapping region */
    if (p)
    {	/* no iterations of the match list done - ie no matches */
	(void) ajFmtPrintF(outfile, "\n\n");
	if (columns)
	    (void) ajFmtPrintF(outfile, "# ");
	(void) ajFmtPrintF(outfile, "%S overlap ends at %d\n", name1,
			   p->seq1start+p->length);
	if (columns)
	    (void) ajFmtPrintF(outfile, "# ");
	(void) ajFmtPrintF(outfile, "%S overlap ends at %d\n\n", name2,
			   p->seq2start+p->length);

	/* report the counts of SNP types */
	(void) ajFmtPrintF(outfile, "\n\n");
	if (columns)
	    (void) ajFmtPrintF(outfile, "# ");
	(void) ajFmtPrintF(outfile, "No. of SNPs = %d\n", snps);
	if (columns)
	    (void) ajFmtPrintF(outfile, "# ");
	(void) ajFmtPrintF(outfile, "No. of transitions = %d\n", transitions);
	if (columns)
	    (void) ajFmtPrintF(outfile, "# ");
	(void) ajFmtPrintF(outfile, "No. of transversions = %d\n",
			   transversions);
    }
    else
    {
	(void) ajFmtPrintF(outfile, "\n\n");
	if (columns)
	    (void) ajFmtPrintF(outfile, "# ");
	(void) ajFmtPrintF(outfile, "No regions of alignment found.\n");
    }

    /* tidy up */
    ajStrDel(&tmp);
    ajStrDel(&name1);
    ajStrDel(&name2);
    (void) ajFeattableDel(&feat1);
    (void) ajFeattableDel(&feat2);

    return;
}

/* @funcstatic diffseq_diffrpt ********************************************
**
** Do a diff and build a report on the diff of the two sequences.
**
** @param [r] matchlist [AjPList] List of minimal non-overlapping matches
** @param [r] seq1 [AjPSeq] Sequence to be diff'd.
** @param [r] seq2 [AjPSeq] Sequence to be diff'd.
** @param [r] report [AjPReport] Report object.
** @param [r] ftab [AjPFeattable] Report feature table
** @param [r] columns [AjBool] format in columns
** @return [void] 
** @@
******************************************************************************/

static void diffseq_diffrpt (AjPList matchlist, AjPSeq seq1, AjPSeq seq2,
			     AjPReport report, AjPFeattable ftab,
			     AjBool columns)
{

    AjIList iter=NULL;			/* match list iterator */
    EmbPWordMatch p=NULL;		/* match structure */
    ajint count=0;			/* count of matches */
    AjPStr s1 = ajSeqStr(seq1);		/* string of seq1 */
    AjPStr s2 = ajSeqStr(seq2);		/* string of seq2 */
    ajint prev1end=0;
    ajint prev2end=0;		/* end positions (+1) of previous match */
    AjPStr tmp = ajStrNew();		/* temporary string */
    ajint start;
    ajint end;	/* start and end of the difference (using human coords) */

    /* stuff for counting SNPs, transitions & transversions */
    ajint snps=0;
    ajint transitions=0;
    ajint transversions = 0;		/* counts of SNP types */
    char base1='\0', base2='\0';
    ajint len1, len2;
    AjPStr tmpstr=NULL;
    static AjPStr tmpseq = NULL;

    /* get the feature table of the sequences */

    AjPFeattable feat1 = ajSeqCopyFeat(seq1);
    AjPFeattable feat2 = ajSeqCopyFeat(seq2);

    AjPFeature gf = NULL;

    /* title line */
    (void) ajFmtPrintS(&tmpstr, "Compare: %S     from: %d   to: %d\n\n",
		       ajReportSeqName(report, seq2),
		       ajSeqBegin(seq2), ajSeqEnd(seq2));


    iter = ajListIter(matchlist);
    while(ajListIterMore(iter))
    {
	p = (EmbPWordMatch) ajListIterNext (iter) ;
	/* first match? this is the start of the overall overlap region */
	if (!count++)
	{
	    (void) ajFmtPrintAppS(&tmpstr, "%S overlap starts at %d\n",
			       ajReportSeqName(report, seq1),
			       p->seq1start+1);
	    (void) ajFmtPrintAppS(&tmpstr, "%S overlap starts at %d\n\n",
			       ajReportSeqName(report, seq2),
			       p->seq2start+1);

	    (void) ajFmtPrintAppS(&tmpstr, "(%S) start end length sequence\n"
				  "(%S) start end length sequence\n\n",
				  ajReportSeqName(report, seq1),
				  ajReportSeqName(report, seq2));

	    ajReportSetHeader(report, tmpstr);

	}
	else			/* difference (gap to next match) */
	{
	    /* save the difference between the matching regions */   
	    /* seq1 details */
	    start = prev1end+1;
	    end = p->seq1start;
	    if (prev1end<p->seq1start)
	    {
	      gf = ajFeatNewII(ftab, start, end);
	      ajStrAssSub(&tmp, s1, prev1end, p->seq1start-1); /* subseq1 */
	      len1=end-start+1;
	      base1 = * ajStrStr(tmp);
	    }
	    else
	    {
	      gf = ajFeatNewII(ftab, start-1, start-2);
	      ajStrAssC(&tmp, "");
	      len1=0;
	    }
	    (void) diffseq_FeaturesRpt("first_feature", gf,
				       feat1, start, end);

	    /* seq2 details */
	    start = prev2end+1;
	    end = p->seq2start;

	    (void) diffseq_FeaturesRpt("second_feature", gf,
				       feat2, start, end);

	    ajFmtPrintS(&tmp, "*name %S", ajReportSeqName(report, seq2));
	    ajFeatTagAdd (gf, NULL, tmp);
	    len2=end-start+1;
	    if (len2 > 0) {
	      ajFmtPrintS(&tmp, "*length %d", len2);
	      ajFeatTagAdd (gf, NULL, tmp);
	      ajFmtPrintS(&tmp, "*start %d", start);
	      ajFeatTagAdd (gf, NULL, tmp);
	      ajFmtPrintS(&tmp, "*end %d", end);
	      ajFeatTagAdd (gf, NULL, tmp);
	      ajStrAssSub(&tmpseq, s2, prev2end, p->seq2start-1);
	      ajFmtPrintS(&tmp, "*sequence %S", tmpseq);
	      ajFeatTagAdd (gf, NULL, tmp);
	      base2 = * ajStrStr(tmpseq);
	    }
	    else {
	      ajFmtPrintS(&tmp, "*length %d", 0);
	      ajFmtPrintS(&tmp, "*start %d", start-1);
	      ajFeatTagAdd (gf, NULL, tmp);
	      ajFmtPrintS(&tmp, "*end %d", start-2);
	      ajFeatTagAdd (gf, NULL, tmp);
	    }

	    /* count SNPs, transitions & transversions */
	    if (len1 == 1 && len2 == 1)
	    {
		snps++;
		transitions += (ajint) embPropTransition(base1, base2);
		transversions += (ajint) embPropTransversion(base1, base2);
	    }

	}
    
	/* output the match */
	/*
	 *  (void) ajFmtPrintF(rptfile, "Matching region %S %d-%d : %S
	 *  %d-%d\n", name1, p->seq1start+1, p->seq1start + p->length,
	 *  name2, p->seq2start+1, p->seq2start + p->length);
	 * (void) ajFmtPrintF(rptfile, "Length of match: %d\n", p->length);
	 */

	/*
	 *  note the end positions (+1) to get the intervening region
	 *  between matches
	 */
	prev1end = p->seq1start + p->length;
	prev2end = p->seq2start + p->length;

    }

    ajListIterFree(iter);

    /* end of overlapping region */
    if (p)
    {
      ajFmtPrintS (&tmp, "Overlap_end: %d in %S\n",
		   p->seq1start+p->length,
		   ajReportSeqName(report, seq1));
      ajFmtPrintAppS (&tmp, "Overlap_end: %d in %S\n",
		   p->seq2start+p->length,
		   ajReportSeqName(report, seq2));
      ajFmtPrintAppS (&tmp, "\n");
      ajFmtPrintAppS (&tmp, "SNP_count: %d\n", snps);
      ajFmtPrintAppS (&tmp, "Transitions: %d\n", transitions);
      ajFmtPrintAppS (&tmp, "Transversions: %d\n", transversions);
    }
    else
    {	/* no iterations of the match list done - ie no matches */
	(void) ajFmtPrintS(&tmp, "No regions of alignment found.\n");
    }

    ajReportSetTail(report, tmp);

    /* tidy up */
    ajStrDel(&tmp);
    (void) ajFeattableDel(&feat1);
    (void) ajFeattableDel(&feat2);

    ajStrDel(&tmpstr);
    
    return;
}

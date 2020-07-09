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

static void diff (AjPList matchlist, AjPSeq seq1, AjPSeq seq2, AjPFile
	outfile, AjBool columns);


static void WordMatchListConvDiffToFeat(AjPList list, AjPFeatTable *tab1,
                AjPFeatTable *tab2, AjPStr seq1name, AjPStr seq2name, AjPSeq
                seq1, AjPSeq seq2);
                                
int main(int argc, char **argv)
{
  
  AjPSeq seq1,seq2;
  ajint wordlen;
  AjPTable seq1MatchTable =0 ;
  AjPList matchlist=NULL ;
  AjPFile outfile;
  AjPFeatTable Tab1=NULL,Tab2=NULL;
  AjPFeatTabOut seq1out = NULL, seq2out = NULL;
  AjBool columns;	/* format output report files in columns */
    
  embInit("diffseq", argc, argv);

  wordlen = ajAcdGetInt ("wordsize");
  seq1 = ajAcdGetSeq ("asequence");
  seq2 = ajAcdGetSeq ("bsequence");
  outfile = ajAcdGetOutfile ("outfile");
  seq1out = ajAcdGetFeatout("afeatout");
  seq2out = ajAcdGetFeatout("bfeatout");
  columns = ajAcdGetBool ("columns");
    
  ajSeqTrim(seq1);
  ajSeqTrim(seq2);

  embWordLength (wordlen);
  if(embWordGetTable(&seq1MatchTable, seq1)){ /* get table of words */
    matchlist = embWordBuildMatchTable(&seq1MatchTable, seq2, ajTrue);
  }

/* get the minimal set of overlapping matches */    
  if (matchlist) {
    (void) embWordMatchMin(matchlist, ajSeqLen(seq1), ajSeqLen(seq2));
  }
  
  if (matchlist) {
/* output the gff files */                                
    (void) WordMatchListConvDiffToFeat(matchlist, &Tab1, &Tab2, 
	  seq1->Name, seq2->Name, seq1, seq2);

/* make the output file */
    (void) diff (matchlist, seq1, seq2, outfile, columns);
    
/* tidy up */
    embWordMatchListDelete(matchlist); /* free the match structures */
  }
   
/* tidy up */
  (void) ajFileClose(&outfile);
  ajFeaturesWrite(seq1out, Tab1);
  ajFeaturesWrite(seq2out, Tab2);
      
  ajExit();
  return 0;
}


/* @funcstatic WordMatchListConvDiffToFeat ***********************************************
**
** convert the word table differences to feature tables.
**
** @param [Pr] list [AjPList] non-overlapping match list to be printed (sorted by position).
** @param [rw] tab1 [AjPFeatTable*] feature table for sequence 1
** @param [rw] tab2 [AjPFeatTable*] feature table for sequence 2
** @param [r] seq1name [AjPStr] sequence name
** @param [r] seq2name [AjPStr] secondsequence name
** @param [r] seq1 [AjPSeq] sequence 1
** @param [r] seq2 [AjPSeq] sequence 2
** @return [void]
** @@
******************************************************************************/

void WordMatchListConvDiffToFeat(AjPList list, AjPFeatTable *tab1,
        AjPFeatTable *tab2, AjPStr seq1name, AjPStr seq2name, AjPSeq
        seq1, AjPSeq seq2) {
  AjEFeatStrand strand=AjStrandWatson;
  AjEFeatFrame frame=AjFrameUnknown;
  AjPStr score=NULL,source=NULL,type=NULL,note=NULL,replace=NULL,desc=NULL;
  AjPFeature feature;
  AjIList iter=NULL;
  AjPFeatLexicon dict=NULL;
  ajint misstart1 = -1;   /* start of mismatch region in seq1 */
  ajint misstart2 = -1;   /* start of mismatch region in seq2 */
  ajint misend1, misend2; /* end of mismatch region */
  AjPStr notestr=NULL, replacestr=NULL;

  dict = ajFeatGffDictionaryCreate(); 
  if(!*tab1)
    *tab1 = ajFeatTabNew(seq1name,dict);
  if(!*tab2)
    *tab2 = ajFeatTabNew(seq2name,dict);
  
  ajStrAssC(&source,"diffseq");
  ajStrAssC(&type,"conflict");
  ajStrAssC(&score,"1.0");
  ajStrAssC(&note,"note");
  ajStrAssC(&replace,"replace");
  
  iter = ajListIter(list);
  while(ajListIterMore(iter)) {
    EmbPWordMatch p = (EmbPWordMatch) ajListIterNext (iter) ;

    misend1 = p->seq1start-1;
    misend2 = p->seq2start-1;

    if (misstart1 != -1) {      /* check that we have seen a match already */
      if (misstart1 <= misend1) {       /* is there a gap between the matches? */
        notestr = ajStrNew();
        replacestr = ajStrNew();
        feature = ajFeatureNew(*tab1, source, type,
                           misstart1+1, misend1+1, score, 
                           strand, frame, desc, 0, 0) ;
        if (misstart1 == misend1 && misstart2 == misend2) {
          ajFmtPrintS(&notestr, "SNP in %S", seq2name);
        } else if (misstart2 > misend2) {
          ajFmtPrintS(&notestr, "Insertion of %d bases in %S", misend1 - misstart1 +1, seq1name);
        } else {
/*          ajFmtPrintS(&notestr, "%S mismatch length=%d. %S mismatch length=%d.", seq1name, misend1 - misstart1 +1, seq2name, misend2 - misstart2 +1);*/
          ajFmtPrintS(&notestr, "%S", seq2name);
          
        }

        ajFeatSetTagValue(feature, note, notestr, ajFalse);

        if (misstart2 <= misend2) {
          ajStrAssSub(&replacestr, ajSeqStr(seq2), misstart2, misend2);
        } else {
          ajStrAssC(&replacestr, "");
        }
        ajFeatSetTagValue(feature, replace, replacestr, ajFalse);
      }

      if (misstart2 <= misend2) {       /* is there a gap between the matches? */
        notestr = ajStrNew();
        replacestr = ajStrNew();
        feature = ajFeatureNew(*tab2, source, type,
                           misstart2+1, misend2+1, score, 
                           strand, frame, desc, 0, 0) ;

        if (misstart2 == misend2 && misstart1 == misend1) {
          ajFmtPrintS(&notestr, "SNP in %S", seq1name);
        } else if (misstart1 > misend1) {
          ajFmtPrintS(&notestr, "Insertion of %d bases in %S", misend2 - misstart2 +1, seq2name);
        } else {
/*          ajFmtPrintS(&notestr, "%S mismatch length=%d. %S mismatch length=%d.", seq2name, misend2 - misstart2 +1, seq1name, misend1 - misstart1 +1);*/
          ajFmtPrintS(&notestr, "%S", seq1name);
        }

        ajFeatSetTagValue(feature, note, notestr, ajFalse);

        if (misstart1 <= misend1) {
          ajStrAssSub(&replacestr, ajSeqStr(seq1), misstart1, misend1);
        } else {
          ajStrAssC(&replacestr, "");
        }
        ajFeatSetTagValue(feature, replace, replacestr, ajFalse);
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
  ajStrDel(&score);
  ajStrDel(&note);
  ajStrDel(&replace);

/* These are still used in ajFeatSetTagValue()
  ajStrDel(&notestr);
  ajStrDel(&replacestr);
*/
  return;
}

/** @funcstatic AddTags ***********************************************
**  
** Writes the tags to the output file
** Don't write out the translation - is it often far too long!
**
** @param [r] outfile [AjPFile] output file
** @param [r] taglist [AjPList] list of tags
** @param [r] values [AjBool] display values of tags
**
** @return [void]
** @@
******************************************************************************/

static void AddTags(AjPFile outfile, AjPList taglist, AjBool values) {

  AjIList titer;                /* iterator for taglist */
  LPFeatTagValue tagstr;        /* tag structure */

/* iterate through the tags and test for match to patterns */

  titer = ajListIter(taglist);
  while (ajListIterMore(titer)) {
    tagstr = (LPFeatTagValue)ajListIterNext(titer);
/* don't display the translation tag - it is far too ajlong :-) */
    if (ajStrCmpC(tagstr->Tag->VocTag->name, "translation")) {
      if (values == ajTrue) {
        (void) ajFmtPrintF(outfile, " %S=\"%S\"", tagstr->Tag->VocTag->name, tagstr->Value);
      } else {
        (void) ajFmtPrintF(outfile, " %S", tagstr->Tag->VocTag->name);
      }
    }
  }
  (void) ajListIterFree(titer);
  (void) ajFmtPrintF(outfile, "\n");
}


/* @funcstatic Features ********************************************
**
** Write out any features which overlap this region.
** Don't write out the source feature - far too irritating!
**
** @param [r] outfile [AjPFile] Output file containing report.
** @param [r] feat [AjPFeatTable] Feature table to search
** @param [r] start [ajint] Start position of region (in human coordinates)
** @param [r] end [ajint] End position of region (in human coordinates)
** @return [void] 
** @@
******************************************************************************/

static void Features(AjPFile outfile, AjPFeatTable feat, ajint start, ajint end) {

  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;
      
/* reminder of the AjSFeature structure for handy reference
*
*
*  AjEFeatClass      Class ;
*  AjPFeatTable      Owner ;
*  AjPFeatVocFeat     Source ;
*  AjPFeatVocFeat     Type ;
*  ajint               Start ;
*  ajint               End; 
*  ajint               Start2;
*  ajint               End2;
*  AjPStr            Score ;
*  AjPList           Tags ;     a.k.a. the [group] field tag-values of GFF2 
*  AjPStr            Comment ;
*  AjEFeatStrand     Strand ;
*  AjEFeatFrame      Frame ;
*  AjPStr            desc ;
*  ajint               Flags;
*
*/

  if(!feat)
    return;

/* Check arguments */
  ajFeatObjVerify(feat, AjCFeatTable ) ;

  if (feat->Features) {
    iter = ajListIter(feat->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;

/* check that the feature is within the range we wish to display */
      if (start > gf->End || end < gf->Start) continue;

/* don't output the 'source' feature - it is very irritating! */
      if (!ajStrCmpC(gf->Type->name, "source")) continue;

/* write out the feature details */
      (void) ajFmtPrintF(outfile, "Feature: %S %d-%d", gf->Type->name, gf->Start, gf->End);
      (void) AddTags(outfile, gf->Tags, ajTrue);
    }
    ajListIterFree(iter) ;
  }

}

/* @funcstatic diff ********************************************
**
** Do a diff and write a report on the diff of the two sequences.
**
** @param [r] matchlist [AjPList] List of minimal non-overlapping matches
** @param [r] seq1 [AjPSeq] Sequence to be diff'd.
** @param [r] seq2 [AjPSeq] Sequence to be diff'd.
** @param [r] outfile [AjPFile] Output file containing report.
** @param [r] columns [AjBool] format in columns
** @return [void] 
** @@
******************************************************************************/

static void diff (AjPList matchlist, AjPSeq seq1, AjPSeq seq2, AjPFile
	outfile, AjBool columns) {

  AjIList iter=NULL;		/* match list iterator */
  EmbPWordMatch p=NULL;		/* match structure */
  ajint count=0;			/* count of matches */
  AjPStr s1 = ajSeqStr(seq1);	/* string of seq1 */
  AjPStr s2 = ajSeqStr(seq2);	/* string of seq2 */
  ajint prev1end=0, prev2end=0;	/* end positions (+1) of previous match */
  AjPStr tmp = ajStrNew();	/* temporary string */
  AjPStr name1 = ajStrNewC(ajSeqName(seq1));	/* name of seq1 */
  AjPStr name2 = ajStrNewC(ajSeqName(seq2));	/* name of seq2 */
  ajint start, end;		/* start and end of the difference (using human coords) */

/* stuff for counting SNPs, transitions & transversions */
  ajint snps=0, transitions=0, transversions = 0;	/* counts of SNP types */
  char base1='\0', base2='\0';
  ajint len1, len2;

/* get the feature table of the sequences */
  AjPFeatTable feat1 = ajSeqGetFeat(seq1);
  AjPFeatTable feat2 = ajSeqGetFeat(seq2);


/* title line */
  (void) ajFmtPrintF(outfile, "# Report of diffseq of: %S and %S\n\n", name1, name2);

  iter = ajListIter(matchlist);
  while(ajListIterMore(iter)) {
    p = (EmbPWordMatch) ajListIterNext (iter) ;
/* first match? */
    if (!count++) {
      if (columns) (void) ajFmtPrintF(outfile, "# ");
      (void) ajFmtPrintF(outfile, "%S overlap starts at %d\n", name1, p->seq1start+1);
      if (columns) (void) ajFmtPrintF(outfile, "# ");
      (void) ajFmtPrintF(outfile, "%S overlap starts at %d\n\n", name2, p->seq2start+1);
      if (columns) (void) ajFmtPrintF(outfile, "# (%S) start end length sequence  (%S) start end length sequence\n\n", name1, name2);

    } else {
/* output the difference between the matching regions */   
/* seq1 details */
      start = prev1end+1;
      end = p->seq1start;
      if (prev1end<p->seq1start) {
        if (columns) {
          (void) ajFmtPrintF(outfile, "%d\t%d\t%d\t", start, end, end-start+1);
        } else {
          (void) ajFmtPrintF(outfile, "\n%S %d-%d Length: %d\n", name1, start, end, end-start+1);
        }
        ajStrAssSub(&tmp, s1, prev1end, p->seq1start-1);
        len1=end-start+1;
        base1 = * ajStrStr(tmp);
      } else {
      	if (columns) {
          (void) ajFmtPrintF(outfile, "%d\t%d\t0\t", start-1, start-1);
        } else {
          (void) ajFmtPrintF(outfile, "\n%S %d Length: 0\n", name1, start-1);
        }
        ajStrAssC(&tmp, "");
        len1=0;
      }
      if (!columns) (void) Features(outfile, feat1, start, end);
      if (columns) {
      	(void) ajFmtPrintF(outfile, "'%S'\t", tmp);
      } else {
        (void) ajFmtPrintF(outfile, "Sequence: %S\n", tmp);
      }

/* seq2 details */
      start = prev2end+1;
      end = p->seq2start;
      if (prev2end<p->seq2start) {
        ajStrAssSub(&tmp, s2, prev2end, p->seq2start-1);
        if (columns) {
          (void) ajFmtPrintF(outfile, "%d\t%d\t%d\t'%S'\n", start, end, end-start+1, tmp);
        } else {
          (void) ajFmtPrintF(outfile, "Sequence: %S\n", tmp);
          if (!columns) (void) Features(outfile, feat2, start, end);
          (void) ajFmtPrintF(outfile, "%S %d-%d Length: %d\n", name2, start, end, end-start+1);
        }
        len2=end-start+1;
        base2 = * ajStrStr(tmp);
      } else {
        ajStrAssC(&tmp, "");
        if (columns) {
          (void) ajFmtPrintF(outfile, "%d\t%d\t0\t'%S'\n", start-1, start-1, tmp);
        } else {
          (void) ajFmtPrintF(outfile, "Sequence: %S\n", tmp);
          if (!columns) (void) Features(outfile, feat2, start, end);
          (void) ajFmtPrintF(outfile, "%S %d Length: 0\n", name2, start-1);
        }
        len2=0;
      }

/* count SNPs, transitions & transversions */
      if (len1 == 1 && len2 == 1) {
        snps++;
        transitions += (ajint) embPropTransition(base1, base2);
        transversions += (ajint) embPropTransversion(base1, base2);
      }

    }
    
/* output the match */
/*
    (void) ajFmtPrintF(outfile, "Matching region %S %d-%d : %S %d-%d\n", name1, p->seq1start+1, p->seq1start + p->length, name2, p->seq2start+1, p->seq2start + p->length);
    (void) ajFmtPrintF(outfile, "Length of match: %d\n", p->length);
*/

/* note the end positions (+1) to get the intervening region between matches */
    prev1end = p->seq1start + p->length;
    prev2end = p->seq2start + p->length;

  }   	

/* end of overlapping region */
  if (p) {	/* no iterations of the match list done - ie no matches */
    (void) ajFmtPrintF(outfile, "\n\n");
    if (columns) (void) ajFmtPrintF(outfile, "# ");
    (void) ajFmtPrintF(outfile, "%S overlap ends at %d\n", name1, p->seq1start+p->length);
    if (columns) (void) ajFmtPrintF(outfile, "# ");
    (void) ajFmtPrintF(outfile, "%S overlap ends at %d\n\n", name2, p->seq2start+p->length);

/* report the counts of SNP types */
    (void) ajFmtPrintF(outfile, "\n\n");
    if (columns) (void) ajFmtPrintF(outfile, "# ");
    (void) ajFmtPrintF(outfile, "No. of SNPs = %d\n", snps);
    if (columns) (void) ajFmtPrintF(outfile, "# ");
    (void) ajFmtPrintF(outfile, "No. of transitions = %d\n", transitions);
    if (columns) (void) ajFmtPrintF(outfile, "# ");
    (void) ajFmtPrintF(outfile, "No. of transversions = %d\n", transversions);
  } else {
    (void) ajFmtPrintF(outfile, "\n\n");
    if (columns) (void) ajFmtPrintF(outfile, "# ");
    (void) ajFmtPrintF(outfile, "No regions of alignment found.\n");
  }

/* tidy up */
  ajStrDel(&s1);
  ajStrDel(&s2);
  ajStrDel(&tmp);
  (void) ajFeatTabDel(&feat1);
  (void) ajFeatTabDel(&feat2);

}

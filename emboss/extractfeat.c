/* @source extractseq application
**
** Extract features from a sequence
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


/*
**  Required enhancements:
**
**  When the feature is a join(), concatenate all the members of the
**  join into one sequence and write them out
**
**  When the feature is a remote ID, read it in from the current database???
*/


#include "emboss.h"


static void extractfeat_FeatSeqExtract (AjPSeq seq, AjPSeqout seqout, 
	AjPFeattable featab, ajint before, ajint after);

static void extractfeat_WriteFeat (AjPSeq seq, AjPSeqout seqout, 
	AjPStr type, ajint start, ajint end, ajint before, ajint after,
	char sense);

static AjBool extractfeat_MatchFeature (AjPFeature gf, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value);

static AjBool extractfeat_MatchPatternTags (AjPFeature feat, AjPStr tpattern,
                                         AjPStr vpattern);

static void extractfeat_FeatureFilter(AjPFeattable featab, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value);


/* @prog extractfeat *************************************************************
**
** Extract features from a sequence
**
******************************************************************************/



int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPSeq seq;
    ajint before;
    ajint after;
    AjPFeattable featab;

    /* feature filter criteria */
    AjPStr source = NULL;
    AjPStr type = NULL;
    ajint sense;
    float minscore;
    float maxscore;
    AjPStr tag = NULL;
    AjPStr value = NULL;


    (void) embInit ("extractfeat", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    seqout = ajAcdGetSeqout ("outseq");
    type = ajAcdGetString ("type");
    before = ajAcdGetInt ("before");
    after = ajAcdGetInt ("after");

    /* feature filter criteria */
    source = ajAcdGetString ("source");
    type = ajAcdGetString ("type");
    sense = ajAcdGetInt ("sense");
    minscore = ajAcdGetFloat ("minscore");
    maxscore = ajAcdGetFloat ("maxscore");
    tag = ajAcdGetString ("tag");
    value = ajAcdGetString ("value");

    while (ajSeqallNext(seqall, &seq))
    {

	/* get the feature table of the sequence */
	featab = ajSeqCopyFeat(seq);

        /* delete features in the table that don't match our criteria */
        extractfeat_FeatureFilter(featab, source, type, sense,
                              minscore, maxscore, tag, value);

        /* extract the features */
        (void) extractfeat_FeatSeqExtract (seq, seqout, featab, before, after);

        /* tidy up */
        ajFeattableDel(&featab);

    }
  
    ajSeqWriteClose (seqout);

    ajExit ();
    return 0;
}


/* @funcstatic extractfeat_FeatSeqExtract *****************************************
**
** Extract features from a sequence
**
** @param [u] seq [AjPSeq] sequence
** @param [u] seqout [AjPSeqout] output sequence
** @param [r] featab [AjPFeattable] features to extract
** @param [r] before [ajint] region before feature to add to extraction
** @param [r] after [ajint] region after feature to add to extraction
** @return [void] 
** @@
******************************************************************************/


static void extractfeat_FeatSeqExtract (AjPSeq seq, AjPSeqout seqout, 
	AjPFeattable featab, ajint before, ajint after)
{
    AjIList    iter = NULL ;
    AjPFeature gf   = NULL ;
      
    /* For all features... */                    
    if (featab && featab->Features)
    {
	iter = ajListIter(featab->Features) ;
	while(ajListIterMore(iter))
	{
	    gf = ajListIterNext (iter) ;
            if (! ajFeatIsLocal(gf))	/* don't process Remote IDs */
            	continue;
	    extractfeat_WriteFeat (seq, seqout, gf->Type, 
		    		gf->Start, gf->End, before, after,
		    		gf->Strand);
	}
	ajListIterFree(iter) ;
    }
     
    return;
}

  


/* @funcstatic extractfeat_WriteFeat *****************************************
**
** Write feature sequence to a file
**
** @param [u] seq [AjPSeq] input sequence
** @param [u] seqout [AjPSeqout] output sequence
** @param [u] type [AjPStr] type of feature
** @param [r] start [ajint] start of sequence to write
** @param [r] end [ajint] end of sequence to write
** @param [r] before [ajint] region before feature to add to extraction
** @param [r] after [ajint] region after feature to add to extraction
** @param [r] sense [char] sense of feature: '+' or '-'
** @return [void] 
** @@
******************************************************************************/

static void extractfeat_WriteFeat (AjPSeq seq, AjPSeqout seqout, 
	AjPStr type, ajint start, ajint end, ajint before, ajint after,
	char sense)
{

    AjPSeq newseq = NULL;
    AjPStr name = NULL;		/* new name of the sequence */
    AjPStr value = NULL;	/* string value of start or end position */
    AjPStr str = NULL;		/* sequence string */
    AjPStr substr = NULL;	/* sequence sub-string */
    AjPStr desc = NULL;		/* sequence description */
    ajint pad;
    ajint tmp;

    /* new sequence */
    newseq = ajSeqNew ();

    /* if the feature is reverse sense, swap 'before' and 'after' */
    if (sense == '-') {
        tmp = before;
        before = after;
        after = tmp;
    }


    str = ajSeqStr(seq);	/* NB don't alter this sequence string */
    /* get required region around the start of the feature */
    if (before >= 0) {
    	before = start-1 - before;
    } else {
    	before = end + before;
    }
    /* get required region around the end of the feature */
    if (after >= 0) {
    	after = end-1 + after;
    } else {
    	after = start-2 - after;
    }
    if (after < before) {
    	ajWarn("Extraction region end less than start for %S [%d-%d] in %S",
    		type, start, end, ajSeqGetName(seq));
    	return;
    }

    /* pad with N/X if before sequence start or after end */
    substr = ajStrNew();
    if (before < 0) {
    	pad = -before;
	if (ajSeqIsNuc(seq))
            ajStrAppKI(&substr, 'N', pad);
        else 
            ajStrAppKI(&substr, 'X', pad);
    	before = 0;
    }

    pad = 0;
    if (after+1 > ajSeqLen(seq)) {
    	pad = after+1 - ajSeqLen(seq);
    	after = ajSeqLen(seq)-1;
    }
    
    ajStrAppSub (&substr, str, before, after);
    
    if (pad > 0) {
	if (ajSeqIsNuc(seq))
            ajStrAppKI(&substr, 'N', pad);
        else 
            ajStrAppKI(&substr, 'X', pad);
    }
    
     /* create a nice name for the new sequence */        
    (void) ajStrAss(&name, ajSeqGetName(seq));
    (void) ajStrAppC(&name, "_");
    (void) ajStrFromInt(&value, start);
    (void) ajStrApp(&name, value);
    (void) ajStrAppC(&name, "_");
    (void) ajStrFromInt(&value, end);
    (void) ajStrApp(&name, value);
    ajSeqAssName(newseq, name);

    /* set the sequence description */
    ajStrAssC(&desc, "[");
    ajStrApp(&desc, type);
    ajStrAppC(&desc, "] ");
    ajStrApp(&desc, ajSeqGetDesc(seq));
    ajSeqAssDesc(newseq, desc);

    /* set the extracted sequence */
    ajSeqReplace (newseq, substr);

    /* set the type */
    if (ajSeqIsNuc(seq))
        ajSeqSetNuc (newseq);
    else
        ajSeqSetProt (newseq);

    /* if feature was in reverse sense, then get reverse complement */
    if (sense == '-') {
    	ajSeqReverse(newseq);
    }

    /* write this region of the sequence */
    (void) ajSeqAllWrite (seqout, newseq);

    /* tidy up */        
    (void) ajStrDel(&name);
    (void) ajStrDel(&value);
    (void) ajSeqDel(&newseq);
    (void) ajStrDel(&substr);
    (void) ajStrDel(&desc);
}


/* @funcstatic extractfeat_FeatureFilter *******************************
**
** Removes unwanted features from a feature table
**
** @param [r] featab [AjPFeattable] Feature table to filter
** @param [r] source [AjPStr] Required Source pattern
** @param [r] type [AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value$
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [AjPStr] Required Tag pattern
** @param [r] value [AjPStr] Required Value pattern
** @return [void]
** @@
******************************************************************************/

static void extractfeat_FeatureFilter(AjPFeattable featab, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value)
{

  AjIList iter = NULL;
  AjPFeature gf = NULL;

  /* foreach feature in the feature table */
  if (featab) {
    iter = ajListIter(featab->Features);
    while(ajListIterMore(iter)) {
      gf = (AjPFeature)ajListIterNext(iter);
      if (!extractfeat_MatchFeature(gf, source, type, sense,
          minscore, maxscore, tag, value)) {
        /* no match, so delete feature from feature table */
        ajFeatDel(&gf);
        ajListRemove(iter);
      }  
    }
  }
}

/* @funcstatic extractfeat_MatchFeature *****************************************
**
** Test if a feature matches a set of criteria
**
** @param [r] gf [AjPFeature] Feature to test
** @param [r] source [AjPStr] Required Source pattern
** @param [r] type [AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value$
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [AjPStr] Required Tag pattern
** @param [r] value [AjPStr] Required Value pattern
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool extractfeat_MatchFeature (AjPFeature gf, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value)
{

/* if maxscore < minscore, then don't test the scores */
AjBool scoreok = (minscore < maxscore);

/* ignore remote IDs */
  if (!ajFeatIsLocal(gf))
    return ajFalse;
  
/* check source, type, sense, score, tags, values */
/* Match anything:
**      for strings, '*'
**      for sense, 0
**      for score, maxscore <= minscore
*/
  if (!embMiscMatchPattern (gf->Source, source) ||
      !embMiscMatchPattern (gf->Type, type) ||
      (gf->Strand == '+' && sense == -1) ||
      (gf->Strand == '-' && sense == +1) ||
      (scoreok && gf->Score < minscore) ||
      (scoreok && gf->Score > maxscore) ||
      !extractfeat_MatchPatternTags(gf, tag, value))
    return ajFalse;

  return ajTrue;                        
}
        
/* @funcstatic extractfeat_MatchPatternTags **************************************
**
** Checks for a match of the tagpattern and valuepattern to at least one
** tag=value pair
**
** @param [r] feat [AjPFeature] Feature to process
** @param [r] tpattern [AjPStr] tags pattern to match with
** @param [r] vpattern [AjPStr] values pattern to match with
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

static AjBool extractfeat_MatchPatternTags (AjPFeature feat, AjPStr tpattern,
                                         AjPStr vpattern)
{
    AjIList titer;                      /* iterator for feat */
    static AjPStr tagnam=NULL;          /* tag structure */
    static AjPStr tagval=NULL;          /* tag structure */
    AjBool val = ajFalse;               /* returned value */
    AjBool tval;                        /* tags result */
    AjBool vval;                        /* value result */


    /*
     *  if there are no tags to match, but the patterns are
     *  both '*', then allow this as a match
     */
    if (!ajStrCmpC(tpattern, "*") &&
        !ajStrCmpC(vpattern, "*"))
        return ajTrue;

    /* iterate through the tags and test for match to patterns */
    titer = ajFeatTagIter(feat);
    while (ajFeatTagval(titer, &tagnam, &tagval))
    {
        tval = embMiscMatchPattern(tagnam, tpattern);
        if(!ajStrLen(tagval))           /* if tag has no value */
            return val;
        vval = embMiscMatchPattern(tagval, vpattern);
        if (tval && vval)
        {
            val = ajTrue;
            break;
        }
    }
    (void) ajListIterFree(titer);

    return val;
}



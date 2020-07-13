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


#include "emboss.h"


static void extractfeat_FeatSeqExtract (AjPSeq seq, AjPSeqout seqout, 
	AjPFeattable featab, ajint before, ajint after, AjBool join);

static void extractfeat_GetFeatseq(AjPSeq seq, AjPFeature gf, AjPStr
	*gfstr, AjBool sense);

static void extractfeat_WriteOut (AjPSeqout seqout, AjPStr *featstr,
	AjBool compall, AjBool sense, ajint firstpos, ajint lastpos,
	ajint before, ajint after, AjPSeq seq, AjBool remote, AjPStr type);

static void extractfeat_BeforeAfter (AjPSeq seq, AjPStr * featstr,
	ajint firstpos, ajint lastpos, ajint before, ajint after,
	AjBool sense);

static void extractfeat_GetRegionPad(AjPSeq seq, AjPStr *featstr, ajint
	start, ajint end, AjBool sense, AjBool beginning);

static void extractfeat_FeatureFilter(AjPFeattable featab, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value);

static AjBool extractfeat_MatchFeature (AjPFeature gf, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value, AjBool *tagsmatch);

static AjBool extractfeat_MatchPatternTags (AjPFeature feat, AjPStr tpattern,
        AjPStr vpattern);


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
    AjBool join;
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
    before = ajAcdGetInt ("before");
    after = ajAcdGetInt ("after");
    join = ajAcdGetBool ("join");

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
        (void) extractfeat_FeatSeqExtract (seq, seqout, featab, before,
		after, join);

        /* tidy up */
        ajFeattableDel(&featab);

    }
  
    ajSeqWriteClose (seqout);

    ajExit ();
    return 0;
}


/* @funcstatic extractfeat_FeatSeqExtract *************************************
**
** Extract features from a sequence
** Build up the complete sequence of a feature, concatenating child features
** to the parent if this is a multiple join and 'join' is set TRUE.
** When writing out, get the amount 'before' and 'after' the feature.
**
** @param [u] seq [AjPSeq] sequence
** @param [u] seqout [AjPSeqout] output sequence
** @param [r] featab [AjPFeattable] features to extract
** @param [r] before [ajint] region before feature to add to extraction
** @param [r] after [ajint] region after feature to add to extraction
** @param [r] join [AjBool] concatenate 'join()' features
** @return [void] 
** @@
******************************************************************************/


static void extractfeat_FeatSeqExtract (AjPSeq seq, AjPSeqout seqout, 
	AjPFeattable featab, ajint before, ajint after, AjBool join)
{
    AjIList    iter = NULL ;
    AjPFeature gf   = NULL ;
    AjBool single, parent, child, compall, sense, remote;
    AjPStr     type = NULL ;	/* name of feature */
    AjPStr     featseq = NULL ;	/* feature sequence string */
    AjPStr     tmpseq = NULL ;	/* temporary sequence string */
    ajint      firstpos, lastpos;	/* bounds of feature in sequence */

    /* For all features... */                    
    if (featab && featab->Features)
    {

/* initialise details of a feature */
        featseq = ajStrNew();
        tmpseq = ajStrNew();
        type = ajStrNew();
        remote = ajFalse;
        compall = ajFalse;
        sense = ajTrue;
        firstpos = 0;
        lastpos = 0;


	iter = ajListIter(featab->Features) ;
	while(ajListIterMore(iter))
	{
	    gf = ajListIterNext (iter) ;

/* determine what sort of thing this feature is */
	    child = ajFalse;
	    parent = ajFalse;
	    single = ajFalse;
            if (ajFeatIsMultiple(gf)) {
            	if (ajFeatIsChild(gf)) {
            	    child = ajTrue;
            	} else {
            	    parent = ajTrue;
            	}
            } else {
            	single = ajTrue;
	    }

/* if we do not wish to assemble joins(), then force all features to be
treated as single */
	    if (!join) {
	    	child = ajFalse;
	    	parent = ajFalse;
	    	single = ajTrue;
	    }
	    	


	    ajDebug ("feature %d=%d is parent %B, child %B, single %B\n",
			gf->Start, gf->End, parent, child, single);

/* if single or parent, write out any previous sequence(s) */
            if (single || parent) {
            	extractfeat_WriteOut(seqout, &featseq, compall, sense,
			firstpos, lastpos, before, after, seq, remote, type);

                /* reset joined feature information */
                ajStrClear(&featseq);
                ajStrClear(&tmpseq);
                ajStrClear(&type);
                remote = ajFalse;
                compall = ajFalse;
                sense = ajTrue;
                firstpos = 0;
                lastpos = 0;
            }

/* don't process Remote IDs and abort a multiple join if one contains 
a Remote ID */
            if (! ajFeatIsLocal(gf)) {
                remote = ajTrue;
            }

            if (remote)
            	continue;


/* if parent, note if have Complemented Join */
            if (parent) {
                compall = ajFeatIsCompMult(gf);	
            }

/* 
** Get the sense of the feature
** NB.  if we are complementing several joined features, then pretend they
** are forward sense until we can reverse-complement them all together. 
*/
	    if (!compall && gf->Strand == '-') 
	        sense = ajFalse;

/* get 'type' name of feature */
	    if (single || parent)
	    	ajStrAss(&type, gf->Type);

/* if single or parent, get 'before' + 'after' sequence positions */
            if (single || parent) {
                firstpos = gf->Start-1;
                lastpos = gf->End-1;
            }

/* if child, update the boundary positions */
            if (child) {
                if (sense) {
                    lastpos = gf->End-1;
		} else {
		    firstpos = gf->Start-1;
		}
            }
/* get feature sequence (complement if required) */
            extractfeat_GetFeatseq(seq, gf, &tmpseq, sense);
	    ajDebug("extracted feature = %d bases\n", ajStrLen(tmpseq));
/* if child, append to previous sequence */
            if (child) {
            	ajStrApp(&featseq, tmpseq);
            } else {
            	ajStrAss(&featseq, tmpseq);
            }

	}
	ajListIterFree(iter) ;

/* write out any previous sequence(s) 
- add before + after, complement all */
        extractfeat_WriteOut(seqout, &featseq, compall, sense,
		firstpos, lastpos, before, after, seq, remote, type);

/* tidy up */
        ajStrDel(&featseq);
        ajStrDel(&tmpseq);
        ajStrDel(&type);
    }
     
    return;
}

  


/* @funcstatic extractfeat_GetFeatseq *****************************************
**
** Get the sequence string of a feature (complement if necessary)
**
** @param [u] seq [AjPSeq] input sequence
** @param [r] gf [AjPFeature] feature
** @param [r] gfstr [AjPStr *] the resulting feature sequence string
** @param [r] sense [AjBool] FALSE if reverse sense
** @return [void] 
** @@
******************************************************************************/

static void extractfeat_GetFeatseq(AjPSeq seq, AjPFeature gf, AjPStr
	*gfstr, AjBool sense)
{

    AjPStr str = NULL;		/* sequence string */
    AjPStr tmp = NULL;

    str = ajSeqStr(seq);	
    tmp = ajStrNew();

    ajDebug("about to get sequence from %d-%d, len=%d\n", gf->Start, gf->End, gf->End-gf->Start+1);
    
    ajStrAppSub (&tmp, str, gf->Start-1, gf->End-1);

    /* if feature was in reverse sense, then get reverse complement */
    if (!sense) {
    	ajSeqReverseStr(&tmp);
    }

    ajStrApp(gfstr, tmp);


    /* tidy up */
    ajStrDel(&tmp);

}


/* @funcstatic extractfeat_WriteOut *****************************************
**
** Get sequence with 'before' and 'after' additions/truncation
** Write feature sequence to a file
**
** @param [u] seqout [AjPSeqout] output sequence
** @param [u] featstr [AjPStr *] sequence string of feature
** @param [r] compall [AjBool] TRUE if we need to complement a join
** @param [r] sense [AjBool] FALSE if any feature in this was complemented
** @param [r] firstpos [ajint] position of start of feature
** @param [r] lastpos [ajint] position of end of feature
** @param [r] before [ajint] region before feature to get
** @param [r] after [ajint] region after feature to get
** @param [u] seq [AjPSeq] input sequence
** @param [r] remote [AjBool] TRUE if must abort becuase it includes Remote IDs
** @param [u] type [AjPStr] type of feature
** @return [void] 
** @@
******************************************************************************/

static void extractfeat_WriteOut (AjPSeqout seqout, AjPStr *featstr,
	AjBool compall, AjBool sense, ajint firstpos, ajint lastpos,
	ajint before, ajint after, AjPSeq seq, AjBool remote, AjPStr type)

{

    AjPSeq newseq = NULL;
    AjPStr name = NULL;		/* new name of the sequence */
    AjPStr value = NULL;	/* string value of start or end position */
    AjPStr desc = NULL;		/* sequence description */
    ajint tmp;

    /* see if there is a sequence to be written out */
    if (!ajStrLen(*featstr)) {
        ajDebug("feature not written out because it has length=0\n");
    	return;
    }

    /* see if must abort because there were Remote IDs in the features */
    if (remote) {
        ajDebug("feature not written out because it has Remote IDs\n");
        return;
    }

    /* if complementing the whole sequence, swap before and after */
    if (compall) {
        tmp = before;
        before = after;
        after = tmp;
    }
    
    ajDebug("feature = %d bases\n", ajStrLen(*featstr));

    /* featstr may be edited, so it is a AjPStr* */
    extractfeat_BeforeAfter (seq, featstr, firstpos, lastpos, before,
		after, sense);
    
    ajDebug("feature+before/after = %d bases\n", ajStrLen(*featstr));

    /* if join was all in reverse sense, now finally get reverse complement */
    if (compall) {
    	ajSeqReverseStr(featstr);
    }

    /* set the extracted sequence */
    newseq = ajSeqNew ();
    ajSeqReplace (newseq, *featstr);

     /* create a nice name for the new sequence */        
    name = ajStrNew();
    (void) ajStrAss(&name, ajSeqGetName(seq));
    (void) ajStrAppC(&name, "_");
    value = ajStrNew();
    (void) ajStrFromInt(&value, firstpos+1);
    (void) ajStrApp(&name, value);
    (void) ajStrAppC(&name, "_");
    (void) ajStrFromInt(&value, lastpos+1);
    (void) ajStrApp(&name, value);

    ajSeqAssName(newseq, name);

    /* set the sequence description with the 'type' added */
    ajStrAssC(&desc, "[");
    ajStrApp(&desc, type);
    ajStrAppC(&desc, "] ");
    ajStrApp(&desc, ajSeqGetDesc(seq));
    ajSeqAssDesc(newseq, desc);

    /* set the type */
    if (ajSeqIsNuc(seq))
        ajSeqSetNuc (newseq);
    else
        ajSeqSetProt (newseq);


    /* write the new sequence */
    (void) ajSeqAllWrite (seqout, newseq);


    /* tidy up */        
    (void) ajSeqDel(&newseq);
    (void) ajStrDel(&name);
    (void) ajStrDel(&value);
    (void) ajStrDel(&desc);
}


/* @funcstatic extractfeat_BeforeAfter *****************************************
**
** Extracts regions before and after a feature (complement if necessary)
**
** @param [u] seq [AjPSeq] input sequence
** @param [u] featstr [AjPStr *] sequence string of feature
** @param [r] firstpos [ajint] position of start of feature
** @param [r] lastpos [ajint] position of end of feature
** @param [r] before [ajint] region before feature to get
** @param [r] after [ajint] region after feature to get
** @param [r] sense [AjBool] FALSE if any feature in this was complemented
** @return [void] 
** @@
******************************************************************************/

static void extractfeat_BeforeAfter (AjPSeq seq, AjPStr * featstr,
		ajint firstpos, ajint lastpos, ajint before, ajint after,
		AjBool sense)

{

    AjPStr str = NULL;		/* sequence string */
    ajint start;
    ajint end;
    ajint featlen;
    ajint len;



/*
** We now have:
** A complete set of joined features in featstr.
** 'sense' is FALSE if any feature was reverse sense (ignore 'compall'
** in this routine).
** 'firstpos' and 'lastpos' set to the first and last positions of the feature
*/

    
    str = ajSeqStr(seq);	/* NB don't alter this sequence string */

/* get start and end positions to truncate featstr at or to extract from seq */

/* do negative values of before/after */
    featlen = ajStrLen(*featstr)-1;
    start = 0;
    end = featlen;

    if (before < 0) {		/* negative, so before end */
    	start = end + before+1;
    }

    if (after < 0) {		/* negative, so after start */
    	end = start - after-1;
    }
    
    if (end < start) {
        ajWarn("Extraction region end less than start for %S [%d-%d]",
		ajSeqGetName(seq), firstpos+1, lastpos+1);
        return;
    }


/* 
** truncate the featstr
** if start or end are past start/end of featstr, use 0 or featlen 
*/
    if (before < 0 || after < 0) {
        ajDebug("truncating featstr to %d-%d\n", start<0?0:start, end>featlen?featlen:end);
        ajDebug("featstr len=%d bases\n", ajStrLen(*featstr));
    	ajStrSub(featstr, start<0?0:start, end>featlen?featlen:end);
        ajDebug("featstr len=%d bases\n", ajStrLen(*featstr));
    }


/* add surrounding sequence if past 0/featlen when before/after are negative */
    if (start < 0) {
        ajDebug("start < 0\n");
        len = -start;
        if (sense) {
            extractfeat_GetRegionPad(seq, featstr, firstpos-len,
		firstpos-1, sense, ajTrue);
	} else {
            extractfeat_GetRegionPad(seq, featstr, lastpos+1,
		lastpos+len, sense, ajTrue);
	}
    }
    if (end > featlen) {	/* NB use the original length of featstr */
        ajDebug("end > featlen\n");
        len = end-featlen;
        ajDebug("len=%d, end=%d\n", len, end);
        if (sense) {
            extractfeat_GetRegionPad(seq, featstr, lastpos+1,
		lastpos+len, sense, ajFalse);
	} else {
            extractfeat_GetRegionPad(seq, featstr, firstpos-len,
		firstpos-1, sense, ajFalse);
	}
    }


/* add surrounding sequence if have positive values of before/after */
    if (before > 0) {
        ajDebug("before > 0\n");
        if (sense) {
            ajDebug("get Before firstpos=%d\n", firstpos);
            extractfeat_GetRegionPad(seq, featstr, firstpos-before,
		firstpos-1, sense, ajTrue);
	} else {
	    ajDebug("get Before (reverse sense) lastpos=%d\n", lastpos);
            extractfeat_GetRegionPad(seq, featstr, lastpos+1, lastpos+before,
		sense, ajTrue);
	}
    }

    if (after > 0) {
        ajDebug("after > 0\n");
        if (sense) {
	    ajDebug("get After lastpos=%d\n", lastpos);
            extractfeat_GetRegionPad(seq, featstr, lastpos+1, lastpos+after,
		sense, ajFalse);
	} else {
	    ajDebug("get After (reverse sense) firstpos=%d\n", firstpos);
            extractfeat_GetRegionPad(seq, featstr, firstpos-after,
		firstpos-1, sense, ajFalse);
	}
    }

}

/* @funcstatic extractfeat_GetRegionPad *******************************
**
** Gets a subsequence string and pads with N or X if it is off the end
**
** @param [r] seq [AjPSeq] Sequence to extract from
** @param [u] featstr [AjPStr *] sequence string of feature
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @param [r] sense [AjBool] FALSE if reverse sense
** @param [r] beginning [AjBool] TRUE if prepend, FALSE if want to append
** @return [void]
** @@
******************************************************************************/

static void extractfeat_GetRegionPad(AjPSeq seq, AjPStr *featstr, ajint
	start, ajint end, AjBool sense, AjBool beginning)
{
    ajint tmp;
    ajint pad;

    AjPStr result = ajStrNew();

    ajDebug("In extractfeat_GetRegionPad start=%d, end=%d\n", start, end);

    if (start > end) {
    	return;
    }


    if (start < 0) {
        pad = -start;
        if (ajSeqIsNuc(seq))
            ajStrAppKI(&result, 'N', pad);
        else 
            ajStrAppKI(&result, 'X', pad);
        start = 0;
    }

    if (end > ajSeqLen(seq)-1) {
    	tmp = ajSeqLen(seq)-1;
    } else {
    	tmp = end;
    }

    if (start <= ajSeqLen(seq) && tmp >= 0) {
        ajDebug("Get subsequence %d-%d\n", start, tmp);
        ajStrAppSub (&result, ajSeqStr(seq), start, tmp);
        ajDebug("result=%S\n", result);
    }
    
    if (end > ajSeqLen(seq)-1) {
        pad = end - ajSeqLen(seq)+1;
        if (ajSeqIsNuc(seq))
            ajStrAppKI(&result, 'N', pad);
        else 
            ajStrAppKI(&result, 'X', pad);
        ajDebug("result=%S\n", result);
    }


    /* if feature was in reverse sense, then get reverse complement */
    if (!sense) {
	ajDebug("get reverse sense of subsequence\n");
    	ajSeqReverseStr(&result);
	ajDebug("result=%S\n", result);
    }

    if (beginning) {
	ajDebug("Prepend to featstr: %S\n", result);
        ajStrInsert(featstr, 0, result);
    } else {
	ajDebug("Append to featstr: %S\n", result);
    	ajStrApp(featstr, result);
    }
    ajDebug("featstr=%S\n", *featstr);


/* tidy up */
    ajStrDel(&result);
    
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

/* 
** Set true if a parent of a join() has matching tag/values.
** We have to remeber the value of a the pattern match to the tags
** of the parent of a join() because the children of the join()
** don't contain the tags information in their gf's
*/
  AjBool tagsmatch = ajFalse;

  /* foreach feature in the feature table */
  if (featab) {
    iter = ajListIter(featab->Features);
    while(ajListIterMore(iter)) {
      gf = (AjPFeature)ajListIterNext(iter);
      if (!extractfeat_MatchFeature(gf, source, type, sense,
          minscore, maxscore, tag, value, &tagsmatch)) {
        /* no match, so delete feature from feature table */
        ajFeatDel(&gf);
        ajListRemove(iter);
      }  
    }
  }
}

/* @funcstatic extractfeat_MatchFeature ***************************************
**
** Test if a feature matches a set of criteria
**
** @param [r] gf [AjPFeature] Feature to test
** @param [r] source [AjPStr] Required Source pattern
** @param [r] type [AjPStr] Required Type pattern
** @param [r] sense [ajint] Required Sense pattern +1,0,-1 (or other value)
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] tag [AjPStr] Required Tag pattern
** @param [r] value [AjPStr] Required Value pattern
** @param [rw] tagsmatch [AjBool *] true if a join has matching tag/values
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool extractfeat_MatchFeature (AjPFeature gf, AjPStr
        source, AjPStr type, ajint sense, float minscore,
        float maxscore, AjPStr tag, AjPStr value, AjBool *tagsmatch)
{

/* if maxscore < minscore, then don't test the scores */
AjBool scoreok = (minscore < maxscore);


/*
** is this not a child of a join() ?
** if it is a child, then we use the previous result of MatchPatternTags
*/
  if (!ajFeatIsMultiple(gf) || !ajFeatIsChild(gf)) {
        *tagsmatch = extractfeat_MatchPatternTags(gf, tag, value);
  }
	   
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
      !*tagsmatch)
    return ajFalse;

  return ajTrue;                        
}
        
/* @funcstatic extractfeat_MatchPatternTags ***********************************
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
    while (ajFeatTagval(titer, &tagnam, &tagval)) {
        tval = embMiscMatchPattern(tagnam, tpattern);
/*
** If tag has no value then
**   If vpattern is '*' the value pattern is a match
** Else check vpattern
*/
        if (!ajStrLen(tagval)) {
            if (!ajStrCmpC(vpattern, "*"))
            	vval = ajTrue;
            else
		vval = ajFalse;
        } else
            vval = embMiscMatchPattern(tagval, vpattern);

        if (tval && vval) {
            val = ajTrue;
            break;
        }
    }
    (void) ajListIterFree(titer);

    return val;
}



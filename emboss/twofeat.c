/* @source twofeat application
**
** Finds neighbouring pairs of features in sequences
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
#include "stdlib.h"




/* @datastatic PHit ***********************************************************
**
** twofeat internals
**
** @alias SHit
** @alias OHit
**
** @attr gfA [AjPFeature] Undocumented
** @attr gfB [AjPFeature] Undocumented
** @attr Start [ajint] Undocumented
** @attr End [ajint] Undocumented
** @attr distance [ajint] Undocumented
******************************************************************************/

typedef struct SHit
{
    AjPFeature gfA;
    AjPFeature gfB;
    ajint Start;
    ajint End;
    ajint distance;	   
} OHit, *PHit;


 	  

static void twofeat_rippledown(AjPFeattable tabA, AjPFeattable tabB,
			       ajint overlapi, ajint minrange,
			       ajint maxrange, ajint
			       rangetypei, ajint sensei, ajint orderi,
			       AjPFeattable outtab,
			       AjBool twoout, AjPStr typeout);
static AjBool twofeat_check_match(AjPFeature gfA, AjPFeature gfB,
				  PHit *detail, ajint overlapi,
				  ajint minrange, ajint maxrange, ajint
				  rangetypei, ajint sensei, ajint orderi);
static void twofeat_sort_hits(AjPList hitlist, AjBool twoout, AjPStr
			      typeout, AjPFeattable outtab);
static void twofeat_find_features(AjPSeq seq, AjPFeattable tab,
				  ajint begin, ajint end, AjPStr source,
				  AjPStr type, ajint sense, float minscore,
				  float maxscore, AjPStr tag, AjPStr value);
static AjBool twofeat_MatchFeature(AjPFeature gf, AjPStr source, AjPStr type,
				   ajint sense, float minscore,
				   float maxscore, AjPStr tag, AjPStr value,
				   AjBool *tagsmatch);
static AjBool twofeat_MatchPatternTags(AjPFeature feat, AjPStr tpattern,
				       AjPStr vpattern);
static PHit twofeat_HitsNew();
static void twofeat_HitsDel(PHit *pthis);
static ajint twofeat_get_overlap_type(AjPStr overlap);
static ajint twofeat_get_range_type(AjPStr rangetype);
static ajint twofeat_get_sense_type(AjPStr sense);
static ajint twofeat_get_order_type(AjPStr order);




/********* relation criterion types *****************/
enum OVERLAP_TYPE {OV_ANY, OV_OVERLAP, OV_NOTOVER, OV_NOTIN, OV_AIN, OV_BIN};
enum RANGE_TYPE {RA_NEAREST, RA_LEFT, RA_RIGHT, RA_FURTHEST};
enum SENSE_TYPE {SN_ANY, SN_SAME, SN_OPPOSITE};
enum ORDER_TYPE {OR_ANY, OR_AB, OR_BA};




/* @prog twofeat **************************************************************
**
** Finds neighbouring pairs of features in sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* input */
    AjPSeqall seqall;

    /* feature a */
    AjPStr asource;
    AjPStr atype;
    AjPStr asense;
    ajint  asensei;
    ajint aminscore;
    ajint amaxscore;
    AjPStr atag;
    AjPStr avalue;

    /* feature b */
    AjPStr bsource;
    AjPStr btype;
    AjPStr bsense;
    ajint  bsensei;
    ajint bminscore;
    ajint bmaxscore;
    AjPStr btag;
    AjPStr bvalue;

    /* relation */ 
    AjPStr overlap;
    ajint minrange;
    ajint maxrange;
    AjPStr rangetype;
    AjPStr sense;
    AjPStr order;
    ajint overlapi;
    ajint rangetypei;
    ajint sensei;
    ajint orderi;

    /* output */
    AjBool twoout;
    AjPStr typeout;
    AjPReport report = NULL;


    AjPSeq seq;
    AjPFeattable tabA = NULL;
    AjPFeattable tabB = NULL;
    AjPFeattable outtab = NULL;

    AjPStr seqname = NULL;
    
    ajint    begin;
    ajint    end;


    embInit("twofeat", argc, argv);

    /* input */    
    seqall   = ajAcdGetSeqall("sequence");

    /* feature a */
    asource   = ajAcdGetString("asource");
    atype     = ajAcdGetString("atype");
    asense    = ajAcdGetListI("asense", 1);
    aminscore = ajAcdGetFloat("aminscore");
    amaxscore = ajAcdGetFloat("amaxscore");
    atag      = ajAcdGetString("atag");
    avalue    = ajAcdGetString("avalue");
    
    /* feature b */
    bsource   = ajAcdGetString("bsource");
    btype     = ajAcdGetString("btype");
    bsense    = ajAcdGetListI("bsense", 1);
    bminscore = ajAcdGetFloat("bminscore");
    bmaxscore = ajAcdGetFloat("bmaxscore");
    btag      = ajAcdGetString("btag");
    bvalue    = ajAcdGetString("bvalue");

    /* relation */
    overlap   = ajAcdGetListI("overlap", 1); 
    minrange  = ajAcdGetInt("minrange");
    maxrange  = ajAcdGetInt("maxrange");
    rangetype = ajAcdGetListI("rangetype", 1);
    sense     = ajAcdGetListI("sense", 1);
    order     = ajAcdGetListI("order", 1);
    
    /* output */
    twoout   = ajAcdGetBool("twoout");
    typeout  = ajAcdGetString("typeout");
    report   = ajAcdGetReport("outfile");

    /* convert feature sense to integer */
    if(ajStrMatchC(asense, "+"))
    	asensei = +1;
    else if(ajStrMatchC(asense, "+"))
    	asensei = -1;
    else
    	asensei = 0;


    if(ajStrMatchC(bsense, "+"))
    	bsensei = +1;
    else if(ajStrMatchC(bsense, "+"))
    	bsensei = -1;
    else
    	bsensei = 0;


    /* convert relation codes to integer values */
    overlapi = twofeat_get_overlap_type(overlap);
    rangetypei = twofeat_get_range_type(rangetype);
    sensei = twofeat_get_sense_type(sense);
    orderi = twofeat_get_order_type(order);

    seqname = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {

	ajStrAssC(&seqname, ajSeqName(seq));
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);

	/* make new feature table for A */
        if(!tabA)
	    tabA = ajFeattableNewSeq(seq);

	/* make new feature table for B */
        if(!tabB)
	    tabB = ajFeattableNewSeq(seq);


	/* go through seq's features adding those that match A to table A */
        twofeat_find_features(seq, tabA, begin, end, asource, atype, asensei,
			      aminscore, amaxscore, atag, avalue);
        
	/* go through seq's features adding those that match B to table B */
        twofeat_find_features(seq, tabB, begin, end, bsource, btype, bsensei,
			      bminscore, bmaxscore, btag, bvalue);


        ajDebug("No of hits in tabA: %d\n", ajFeattableSize(tabA));
        ajDebug("No of hits in tabB: %d\n", ajFeattableSize(tabB));

	if(ajFeattableSize(tabA) && ajFeattableSize(tabB))
	{
	    /* initialise the output feature table */
            outtab = ajFeattableNewSeq(seq);


            /*
	    **  find pairs of hits within the required distance and output
	    **  the results
	    */
	    twofeat_rippledown(tabA, tabB, overlapi, minrange, maxrange,
			       rangetypei, sensei, orderi, outtab, twoout,
			       typeout);

	    /* write features and tidy up */
	    ajReportWrite(report, outtab, seq);        

	    /*
	    ** try not deleting - now works OK - not sure why it fails in
	    ** ajExit if this is included
	    ** ajDebug("ajFeattableDel(&outtab)\n");
	    ** ajFeattableDel(&outtab);
	    */
	}
	

        ajDebug("ajFeattableDel(&tabA)\n");	 
	ajFeattableDel(&tabA);
        ajDebug("ajFeattableDel(&tabB)\n");	 
	ajFeattableDel(&tabB);
    }
    
    ajStrDel(&seqname);
    ajSeqDel(&seq);

    ajReportClose(report);
    ajReportDel(&report);    

    ajExit();

    return 0;
}




/* @funcstatic twofeat_rippledown ********************************************
**
** Go down the two lists of matches looking for hits within the required
** maximum distance.
** 
** Foreach feature in tabA
**   look at each feature in tabB 
**   if all the match criteria are valid
**     add the matching pair to the list of hits
** 
** Add any required hits from the hit-list to the output feature table
** 
**
** @param [r] tabA [AjPFeattable] table A of features to compare to tabB
** @param [r] tabB [AjPFeattable] table B of features to compare to tabA
** @param [r] overlapi [ajint] types of overlap allowed
** @param [r] minrange [ajint] min distance allowed
** @param [r] maxrange [ajint] max distance allowed
** @param [r] rangetypei [ajint] where to measure the distance from
** @param [r] sensei [ajint] sense relationships allowed
** @param [r] orderi [ajint] order relationships allowed
** @param [r] outtab [AjPFeattable] output feature table
** @param [r] twoout [AjBool] True=write both features, else make a single one
** @param [r] typeout [AjPStr] if a single feature, this is its type name
** @return [void]
** @@
******************************************************************************/

static void twofeat_rippledown(AjPFeattable tabA, AjPFeattable tabB,
			       ajint overlapi, ajint minrange,
			       ajint maxrange, ajint rangetypei,
			       ajint sensei, ajint orderi,
			       AjPFeattable outtab, AjBool twoout,
			       AjPStr typeout)
{


    AjIList    iterA = NULL;
    AjPFeature gfA   = NULL;

    AjIList    iterB = NULL;
    AjPFeature gfB   = NULL;

    AjPList hitlist;
    PHit detail = NULL;


    hitlist = ajListNew(); 
    
    if(tabA && tabA->Features)
    {
        /* For all features in tabA ... */
    	iterA = ajListIter(tabA->Features);
    	while(ajListIterMore(iterA))
    	{
    	    gfA = ajListIterNext(iterA);
            ajDebug("In rippledown gfA=%S %d..%d\n", gfA->Type, gfA->Start,
		    gfA->End);

            /* For all features in tabB ... */
            if(tabB && tabB->Features) 
            {
   	        iterB = ajListIter(tabB->Features);
                while(ajListIterMore(iterB))
                {
                    gfB = ajListIterNext(iterB);
                    ajDebug("In rippledown gfB=%S %d..%d\n", gfB->Type,
			    gfB->Start, gfB->End);

		    /*
		    ** check for overlap, minrange, maxrange, rangetype,
		    ** sense, order
		    */
                    if(twofeat_check_match(gfA, gfB, &detail, overlapi,
					   minrange, maxrange, rangetypei,
					   sensei, orderi))
                        /* push details on hitlist */
                        ajListPush(hitlist, detail);
                }
                ajListIterFree(iterB);
            }
	}
	ajListIterFree(iterA);
    }

    /* Put hits in outtab */
    twofeat_sort_hits(hitlist, twoout, typeout, outtab);


    ajListFree(&hitlist);

    return;
}




/* @funcstatic twofeat_sort_hits ***********************************
**
** Outputs the pairs of hits to the output feature table
**
** @param [r] hitlist [AjPList] list of matches (PHit) 
** @param [r] twoout [AjBool] True if want pairs of features output
** @param [r] typeout [AjPStr]  name of type if want single features
** @param [u] outtab [AjPFeattable] output feature table
** @return [void]
** @@
******************************************************************************/

static void twofeat_sort_hits(AjPList hitlist, AjBool twoout, AjPStr
			      typeout, AjPFeattable outtab)
{
    char strand;
    ajint frame = 0;
    float score = 0.0;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    AjPFeature feature;
    static AjPStr tmp = NULL;
    ajint start;
    ajint end;

    AjIList iter = NULL;
    PHit detail  = NULL;

    	
    ajStrAssC(&source,"twofeat");
    ajStrAss(&type, typeout);


    iter = ajListIter(hitlist);
    while(ajListIterMore(iter))
    {
        detail = ajListIterNext(iter);

        if(twoout)
	{
            ajFeattableAdd(outtab, detail->gfA);
            ajFeattableAdd(outtab, detail->gfB);
    
        }
	else
	{
            start = detail->Start;
            end = detail->End;

            /* if both features are -ve then output this, else +ve */
            if((detail->gfA)->Strand == '-' &&
                (detail->gfB)->Strand == '-')
                strand = '-';
	    else
                strand = '+';

            feature = ajFeatNew(outtab, source, type, start, end,
				score, strand, frame);

            ajFmtPrintS(&tmp, "*startA %d", (detail->gfA)->Start);
            ajFeatTagAdd(feature, NULL, tmp);

            ajFmtPrintS(&tmp, "*endA %d", (detail->gfA)->End);
            ajFeatTagAdd(feature, NULL, tmp);

            ajFmtPrintS(&tmp, "*startB %d", (detail->gfB)->Start);
            ajFeatTagAdd(feature, NULL, tmp);

            ajFmtPrintS(&tmp, "*endB %d", (detail->gfB)->End);
            ajFeatTagAdd(feature, NULL, tmp);
	}

        /* delete hit */
        twofeat_HitsDel(&detail);
    }
    ajListIterFree(iter);
    
    return;
}




/* @funcstatic twofeat_find_features ***********************************
**
** Finds seq features matching the required criteria and puts them in tab
**
** @param [r] seq [AjPSeq] the sequence
** @param [u] tab [AjPFeattable] Feature table to populate
** @param [r] begin [ajint] start position in sequence
** @param [r] end [ajint] end position in sequence
** @param [r] source [AjPStr] source criterion
** @param [r] type [AjPStr] type criterion
** @param [r] sense [ajint] sense criterion
** @param [r] minscore [float] min score criterion
** @param [r] maxscore [float] max score criterion
** @param [r] tag [AjPStr] tag criterion
** @param [r] value [AjPStr] tag value criterion
** @return [void] 
** @@
******************************************************************************/

static void twofeat_find_features(AjPSeq seq, AjPFeattable tab,
				  ajint begin, ajint end, AjPStr source,
				  AjPStr type, ajint sense, float minscore,
				  float maxscore, AjPStr tag, AjPStr value)
{

    /* get feature table of sequence */
    AjPFeattable seqtab = ajSeqGetFeat(seq);

    AjIList    iter = NULL;
    AjPFeature gf   = NULL;
    AjPFeature gfcopy = NULL;
    AjBool     tagsmatch;
    
   
    tagsmatch = ajFalse;

    /* For all features... */
    if(seqtab && seqtab->Features) 
    {
    	iter = ajListIter(seqtab->Features);
    	while(ajListIterMore(iter))
    	{
    	    gf = ajListIterNext(iter);

            /* is this feature local and in the sequence range? */
            if(! ajFeatIsLocalRange(gf, begin, end))
    	        continue;

    	    /* do criterion match */
            if(twofeat_MatchFeature(gf, source, type, sense, minscore,
				    maxscore, tag, value, &tagsmatch))
	    {
                ajDebug("Found feature source=%S type=%S %d..%d\n", 
                	gf->Source, gf->Type, gf->Start, gf->End);
		/*
		** could explicitly make a new feature like this, but it
		** is probably safer to let ajFeatCopy do it automatically
		**  gfcopy = ajFeatNew(tab, gf->Source, gf->Type, gf->Start,
		** gf->End, gf->Score, gf->Strand, gf->Frame);
		*/
		gfcopy = NULL;	   /* force a new object to be made */
		ajFeatCopy(&gfcopy, gf);
		ajFeattableAdd(tab, gfcopy);

		/* ajFeatTrace(gfcopy); */
	    }
	}
	ajListIterFree(iter);
    }

    ajDebug("(In twofeat_find_features) No of hits in tab: %d\n",
	    ajFeattableSize(tab));
    
    return;
}




/* @funcstatic twofeat_MatchFeature *****************************************
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
** @param [u] tagsmatch [AjBool *] true if a join has matching tag/values
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool twofeat_MatchFeature(AjPFeature gf, AjPStr source, AjPStr type,
				   ajint sense, float minscore,
				   float maxscore, AjPStr tag, AjPStr value,
				   AjBool *tagsmatch)
{
    AjBool scoreok;

    scoreok = (minscore < maxscore);

     /* 
     ** is this a child of a join() ? 
     ** if it is a child, then we use the previous result of MatchPatternTags
     */
    if(!ajFeatIsMultiple(gf) || !ajFeatIsChild(gf))
	*tagsmatch = twofeat_MatchPatternTags(gf, tag, value);

    /* ignore remote IDs */
    if(!ajFeatIsLocal(gf))
	return ajFalse;
  
     /* check source, type, sense, score, tags, values
     ** Match anything:
     **      for strings, '*'
     **      for sense, 0
     **      for score, maxscore <= minscore
     */

    if(!embMiscMatchPattern(gf->Source, source) ||
       !embMiscMatchPattern(gf->Type, type) ||
       (gf->Strand == '+' && sense == -1) ||
       (gf->Strand == '-' && sense == +1) ||
       (scoreok && gf->Score < minscore) ||
       (scoreok && gf->Score > maxscore) ||
       !*tagsmatch)
	return ajFalse;

    return ajTrue;                        
}




/* @funcstatic twofeat_MatchPatternTags **************************************
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

static AjBool twofeat_MatchPatternTags(AjPFeature feat, AjPStr tpattern,
				       AjPStr vpattern)
{
    AjIList titer;                      /* iterator for feat */
    static AjPStr tagnam = NULL;        /* tag structure */
    static AjPStr tagval = NULL;        /* tag structure */
    AjBool val = ajFalse;               /* returned value */
    AjBool tval;                        /* tags result */
    AjBool vval;                        /* value result */


    /*
    **  if there are no tags to match, but the patterns are
    **  both '*', then allow this as a match
    */
    if(!ajStrCmpC(tpattern, "*") &&
       !ajStrCmpC(vpattern, "*"))
        return ajTrue;
    
    /* iterate through the tags and test for match to patterns */
    titer = ajFeatTagIter(feat);
    while(ajFeatTagval(titer, &tagnam, &tagval))
    {
        tval = embMiscMatchPattern(tagnam, tpattern);
	 /*
	 ** If tag has no value then
	 **   If vpattern is '*' the value pattern is a match
	 ** Else check vpattern
	 */

        if(!ajStrLen(tagval))
	{
            if(!ajStrCmpC(vpattern, "*"))
            	vval = ajTrue;
            else
		vval = ajFalse;
        }
	else
            vval = embMiscMatchPattern(tagval, vpattern);

        if(tval && vval)
	{
            val = ajTrue;
            break;
        }
    }
    ajListIterFree(titer);
    
    return val;
}




/* @funcstatic twofeat_HitsNew *****************************************
**
** Creates a new PHit object
**
** @return [PHit] Hit object
** @@
******************************************************************************/

static PHit twofeat_HitsNew()
{
    PHit pthis;
    AJNEW0(pthis);
	    
    return pthis;
} 




/* @funcstatic twofeat_HitsDel *****************************************
**
** Deletes a PHit object
**
** @param [wP] pthis [PHit *] Pointer to object to be deleted
** @return [void]
** @@
******************************************************************************/

static void twofeat_HitsDel(PHit *pthis)
{
    if(! pthis)
	return;

    if(! *pthis)
	return;
	         
    AJFREE(*pthis);
             
    *pthis = NULL;

    return;
}



   
/* @funcstatic twofeat_check_match *********************************
**
** check for overlap, minrange, maxrange, rangetype, sense, order
** put the results in a PHit object
**
** @param [r] gfA [AjPFeature] Feature A
** @param [r] gfB [AjPFeature] Feature B
** @param [r] detail [PHit *] Returned details of match 
** @param [r] overlapi [ajint] types of overlap allowed
** @param [r] minrange [ajint] min distance allowed
** @param [r] maxrange [ajint] max distance allowed
** @param [r] rangetypei [ajint] where to measure the distance from
** @param [r] sensei [ajint] sense relationships allowed
** @param [r] orderi [ajint] order relationships allowed
** @return [AjBool] True if got a match
** @@
******************************************************************************/

static AjBool twofeat_check_match(AjPFeature gfA, AjPFeature gfB,
				  PHit *detail, ajint overlapi,
				  ajint minrange, ajint maxrange, ajint
				  rangetypei, ajint sensei, ajint orderi)
{
    ajint distance = 0;
    ajint sA;
    ajint eA;
    ajint sB;
    ajint eB;				/* start and end positions */
    ajint ss;
    ajint ee;
    ajint se;
    ajint es;		  /* distances from start and end positions */
    ajint tmp1;
    ajint tmp2;

    sA = gfA->Start;
    eA = gfA->End;
    sB = gfB->Start;
    eB = gfB->End;


    ajDebug("Next gfA=%S %d..%d\n", gfA->Type, gfA->Start, gfA->End);
    ajDebug("Next gfB=%S %d..%d\n", gfB->Type, gfB->Start, gfB->End);

    /* get distances and absolute distances from each end */
    ss = abs(sA - sB);
    ee = abs(eA - eB);
    se = abs(sA - eB);
    es = abs(eA - sB);

    switch(rangetypei)
    {
    case RA_NEAREST:
        tmp1 = (ss < ee) ? ss : ee;
        tmp2 = (se < es) ? se : es;
        distance = (tmp1 < tmp2) ? tmp1 : tmp2;
        break;
 
    case RA_LEFT:
        distance = ss;
        break;

    case RA_RIGHT:
        distance = ee;
        break;

    case RA_FURTHEST:
        tmp1 = (ss > ee) ? ss : ee;
        tmp2 = (se > es) ? se : es;
        distance = (tmp1 > tmp2) ? tmp1 : tmp2;
        break;

    default:
        ajFatal("Unknown range type: %d", rangetypei);
    }

    ajDebug("Distance: %d\n", distance);

    /* check distance criteria */
    if(minrange < maxrange)
    {
	/* ignore distance if these are the same */
        ajDebug("Distance < %d: %B\n", minrange, distance < minrange);
        ajDebug("Distance > %d: %B\n", maxrange, distance > maxrange);
        if(distance < minrange)
	    return ajFalse;

        if(distance > maxrange)
	    return ajFalse;
    }
    ajDebug("Distance OK\n");
    
    /* check overlap criteria */
    switch(overlapi)
    {
    case OV_ANY:
	break;

    case OV_OVERLAP:			/* want sA in B or sB in A */
        if((sA<sB || sA>eB) && (sB<sA || sB>eA))
	    return ajFalse;
	break;

    case OV_NOTOVER:		  	/* want eA before B or sA after B */
        if((sA>=sB && sA<=eB) || (sB>=sA && sB<=eA))
	    return ajFalse;
	break;

    case OV_NOTIN:  /* want sA or eA out of B and sB or eB out of A */
        if((sA<=sB && eA>=eB) || (sB<=sA && eB>=eA))
	    return ajFalse;
	break;

    case OV_AIN:			/* want A in B */
        if(sA<sB || eA>eB)
	    return ajFalse;
	break;

    case OV_BIN:			/* want B in A */
        if(sB<sA || eB>eA)
	    return ajFalse;
	break;
   
    default:
        ajFatal("Unknown overlap type: %d", overlapi);
    }
    ajDebug("Overlap OK\n");

    /* check sense criteria */
    switch(sensei)
    {
    case SN_ANY:
	break;

    case SN_SAME:	
        if(gfA->Strand != gfB->Strand)
	    return ajFalse;
	break;

    case SN_OPPOSITE:	
        if(gfA->Strand == gfB->Strand) return ajFalse;

    default:
        ajFatal("Unknown sense type: %d", sensei);
    }
    ajDebug("Sense OK\n");

    /* check order criteria */
    /* measure the order from the mid-point of each */
    switch(sensei)
    {
    case SN_ANY:
	break;

    case OR_AB:
        tmp1 = (eA-sA)/2;
        tmp2 = (eB-sB)/2;
        if(tmp1>tmp2)
	    return ajFalse;
	break;

    case OR_BA:
        tmp1 = (eA-sA)/2;
        tmp2 = (eB-sB)/2;
        if(tmp1<tmp2)
	    return ajFalse;
	break;

    default:
        ajFatal("Unknown sense type: %d", sensei);
    }
    ajDebug("Order OK\n");
    
    /*
    ** reject any match found to the same feature (same position,
    ** sense and type)
    */
    if(ss == 0 &&
       ee == 0 &&
       gfA->Strand == gfB->Strand &&
       ajStrMatch(gfA->Type, gfB->Type))
    {
        ajDebug("Found match of feature to itself\n");
        return ajFalse;    
    }

    /* if we have a hit, make a PHit object */
    *detail = twofeat_HitsNew();
    (*detail)->gfA = gfA;
    (*detail)->gfB = gfB;
    (*detail)->distance = distance;
    (*detail)->Start = (sA < sB) ? sA : sB;
    (*detail)->End = (eA > eB) ? eA : eB;

    ajDebug("Hit found\n");

    return ajTrue;

}




/* @funcstatic twofeat_get_overlap_type *********************************
**
** converts the overlap code to an integer
** 
**
** @param [r] overlap [AjPStr] Overlap code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_overlap_type(AjPStr overlap)
{

    if(ajStrMatchC(overlap, "A"))
	return OV_ANY;

    if(ajStrMatchC(overlap, "O"))
	return OV_OVERLAP;

    if(ajStrMatchC(overlap, "NO"))
	return OV_NOTOVER;

    if(ajStrMatchC(overlap, "NW"))
	return OV_NOTIN;

    if(ajStrMatchC(overlap, "AW"))
	return OV_AIN;

    if(ajStrMatchC(overlap, "BW")) return OV_BIN;

    ajFatal("Unknown Overlap code: %S", overlap);
    return -1;
}




/* @funcstatic twofeat_get_range_type *********************************
**
** converts the range code to an integer
** 
**
** @param [r] rangetype [AjPStr] Range code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_range_type(AjPStr rangetype)
{
    if(ajStrMatchC(rangetype, "N"))
	return RA_NEAREST;

    if(ajStrMatchC(rangetype, "L"))
	return RA_LEFT;

    if(ajStrMatchC(rangetype, "R"))
	return RA_RIGHT;

    if(ajStrMatchC(rangetype, "F"))
	return RA_FURTHEST;

    ajFatal("Unknown rangetype code: %S", rangetype);

    return -1;
}




/* @funcstatic twofeat_get_sense_type *********************************
**
** converts the sense code to an integer
** 
**
** @param [r] sense [AjPStr] sense code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_sense_type(AjPStr sense)
{
    if(ajStrMatchC(sense, "A"))
	return SN_ANY;

    if(ajStrMatchC(sense, "S"))
	return SN_SAME;

    if(ajStrMatchC(sense, "O"))
	return SN_OPPOSITE;

    ajFatal("Unknown sense code: %S", sense);

    return -1;
}




/* @funcstatic twofeat_get_order_type *********************************
**
** converts the order code to an integer
** 
**
** @param [r] order [AjPStr] order code
** @return [ajint] integer value
** @@
******************************************************************************/

static ajint twofeat_get_order_type(AjPStr order)
{
    if(ajStrMatchC(order, "A"))
	return OR_ANY;

    if(ajStrMatchC(order, "AB"))
	return OR_AB;

    if(ajStrMatchC(order, "BA"))
	return OR_BA;

    ajFatal("Unknown order code: %S", order);

    return -1;
}

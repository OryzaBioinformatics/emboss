/* @source showseq application
**
** Display a sequence with translations, features and other bits
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 14 Sept 1999 - GWW - written
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


/* declare functions */

static void showseq_FormatShow(EmbPShow ss, AjPStr format, AjPTrn trnTable,
			       AjPRange translaterange, AjPRange uppercase,
			       AjPRange highlight, AjBool threeletter,
			       AjBool numberseq, AjPFeattable feat,
			       ajint orfminsize, AjPList restrictlist,
			       AjBool flat, AjPRange annotation);

static void showseq_read_equiv(AjPFile *equfile, AjPTable *table);
static void showseq_read_file_of_enzyme_names(AjPStr *enzymes);
static AjBool showseq_MatchFeature (AjPFeature gf, AjPStr
        matchsource, AjPStr matchtype, ajint matchsense, float minscore,
        float maxscore, AjPStr matchtag, AjPStr matchvalue, AjBool *tagsmatch);
static AjBool showseq_MatchPatternTags (AjPFeature feat, AjPStr tpattern,
                                         AjPStr vpattern);
static void showseq_FeatureFilter(AjPFeattable featab, AjPStr
        matchsource, AjPStr matchtype, ajint matchsense, float minscore,
        float maxscore, AjPStr matchtag, AjPStr matchvalue);

#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "embossre.equ"
#define EQUGUESS 3500     /* Estimate of number of equivalent names */



/* @prog showseq **************************************************************
**
** Display a sequence with features, translation etc
**
******************************************************************************/

int main(int argc, char **argv)
{

    ajint begin, end;
    AjPSeqall seqall;
    AjPSeq seq;
    EmbPShow ss;
    AjPFile outfile;
    AjPStr * formatlist;
    AjPStr * thinglist;
    AjPStr * tablelist;
    ajint table=0;
    AjPRange translaterange;
    AjPRange uppercase;
    AjPRange highlight;
    AjBool threeletter;
    AjBool numberseq;
    AjBool nameseq;
    ajint width;
    ajint length;
    ajint margin;
    AjBool description;
    ajint offset;
    AjBool html;
    AjPStr descriptionline;
    AjPFeattable feat;
    ajint orfminsize;
    AjBool flat;
    AjPTrn trnTable;
    AjPRange annotation;

    /* holds ACD or constructed format for output */
    AjPStr format=ajStrNew();
    ajint i;

    /* stuff lifted from Alan's 'restrict.c' */
    AjPStr    enzymes=NULL;
    ajint mincuts;
    ajint maxcuts;
    ajint sitelen;
    AjBool single;
    AjBool blunt;
    AjBool sticky;
    AjBool ambiguity;
    AjBool plasmid;
    AjBool commercial;
    AjBool limit;
    AjBool equiv;
    AjPFile   enzfile=NULL;
    AjPFile   equfile=NULL;
    AjPTable  retable=NULL;
    ajint       hits;
    AjPList   restrictlist=NULL;

    /* feature filter criteria */
    AjPStr matchsource = NULL;
    AjPStr matchtype = NULL;
    ajint matchsense;
    float minscore;
    float maxscore;
    AjPStr matchtag = NULL;
    AjPStr matchvalue = NULL;


    (void) embInit ("showseq", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    outfile = ajAcdGetOutfile ("outfile");
    formatlist = ajAcdGetList ("format");
    thinglist = ajAcdGetList ("things");
    tablelist = ajAcdGetList ("table");
    translaterange = ajAcdGetRange ("translate");
    uppercase = ajAcdGetRange ("uppercase");
    highlight = ajAcdGetRange ("highlight");
    annotation = ajAcdGetRange ("annotation");
    threeletter = ajAcdGetBool ("threeletter");
    numberseq = ajAcdGetBool ("number");
    width = ajAcdGetInt ("width");
    length = ajAcdGetInt ("length");
    margin = ajAcdGetInt ("margin");
    nameseq = ajAcdGetBool ("name");
    description = ajAcdGetBool ("description");
    offset = ajAcdGetInt ("offset");
    html = ajAcdGetBool ("html");
    orfminsize = ajAcdGetInt ("orfminsize");
    flat = ajAcdGetBool ("flatreformat");

    /*  restriction enzyme stuff */
    mincuts = ajAcdGetInt ("mincuts");
    maxcuts = ajAcdGetInt ("maxcuts");
    sitelen  = ajAcdGetInt ("sitelen");
    single = ajAcdGetBool ("single");
    blunt = ajAcdGetBool ("blunt");
    sticky = ajAcdGetBool ("sticky");
    ambiguity = ajAcdGetBool ("ambiguity");
    plasmid = ajAcdGetBool ("plasmid");
    commercial = ajAcdGetBool ("commercial");
    limit = ajAcdGetBool ("limit");
    enzymes = ajAcdGetString ("enzymes");
    equiv = ajAcdGetBool("preferred");

    /* feature filter criteria */
    matchsource = ajAcdGetString ("matchsource");
    matchtype = ajAcdGetString ("matchtype");
    matchsense = ajAcdGetInt ("matchsense");
    minscore = ajAcdGetFloat ("minscore");
    maxscore = ajAcdGetFloat ("maxscore");
    matchtag = ajAcdGetString ("matchtag");
    matchvalue = ajAcdGetString ("matchvalue");

    /* read the local file of enzymes names */
    showseq_read_file_of_enzyme_names(&enzymes);

    /* check that the translate range is ordered */
    if (!ajRangeOrdered(translaterange))
	(void) ajFatal ("Translation ranges are not in ascending, "
			"non-overlapping order.");


    /* get the format to use */
    if (!ajStrCmpC(formatlist[0], "0"))
	for (i=0; thinglist[i]; i++)
	{
	    (void) ajStrApp(&format, thinglist[i]);
	    (void) ajStrAppC(&format, " ");
	}
    else if (!ajStrCmpC(formatlist[0], "1"))
	(void) ajStrAssC(&format, "S A ");
    else if (!ajStrCmpC(formatlist[0], "2"))
	(void) ajStrAssC(&format, "B N T S A F ");
    else if (!ajStrCmpC(formatlist[0], "3"))
	(void) ajStrAssC(&format, "B N T S A ");
    else if (!ajStrCmpC(formatlist[0], "4"))
	(void) ajStrAssC(&format, "B N T S B 1 A F ");
    else if (!ajStrCmpC(formatlist[0], "5"))
	(void) ajStrAssC(&format, "B N T S B 1 2 3 A F ");
    else if (!ajStrCmpC(formatlist[0], "6"))
	(void) ajStrAssC(&format, "B N T S B 1 2 3 T -3 -2 -1 A F ");
    else if (!ajStrCmpC(formatlist[0], "7"))
	(void) ajStrAssC(&format, "B R S N T C -R B 1 2 3 T -3 -2 -1 A ");
    else if (!ajStrCmpC(formatlist[0], "8"))
	(void) ajStrAssC(&format, "B 1 2 3 N T R S T C -R T -3 -2 -1 A F ");
    else
	(void) ajFatal("Invalid format type: %S", formatlist[0]);


    /* make the format upper case */
    (void) ajStrToUpper(&format);

    /* create the translation table */
    trnTable = ajTrnNewI(table);

    while (ajSeqallNext(seqall, &seq))
    {
	/* get begin and end positions */
	begin = ajSeqBegin(seq)-1;
	end = ajSeqEnd(seq)-1;

	/* do the name and description */
	if (nameseq)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<H2>%S</H2>\n",
				   ajSeqGetName(seq));
	    else
		(void) ajFmtPrintF(outfile, "%S\n", ajSeqGetName(seq));
	}

	if (description)
	{
	    /*
	     *  wrap the description line at the width of the sequence
	     *  plus margin
	     */
	    if (html)
		(void) ajFmtPrintF(outfile, "<H3>%S</H3>\n",
				   ajSeqGetDesc(seq));
	    else
	    {
		descriptionline = ajStrNew();
		(void) ajStrAss(&descriptionline, ajSeqGetDesc(seq));
		(void) ajStrWrap(&descriptionline, width+margin);
		(void) ajFmtPrintF(outfile, "%S\n", descriptionline);
		(void) ajStrDel(&descriptionline);
	    }
	}

	/* get the feature table of the sequence */
	feat = ajSeqCopyFeat(seq);

        /* delete features in the table that don't match our criteria */
        showseq_FeatureFilter(feat, matchsource, matchtype, matchsense,
                              minscore, maxscore, matchtag, matchvalue);

	/* get the restriction cut sites */
	/*
	 *  most of this is lifted from the program 'restrict.c' by Alan
	 *  Bleasby
	 */
	if (ajStrFindC(format, "R") != -1)
	{
	    if (single)
		maxcuts=mincuts=1;
	    retable = ajStrTableNew(EQUGUESS);
	    ajFileDataNewC(ENZDATA, &enzfile);
	    if(!enzfile)
		ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");
	    if (equiv)
	    {
		ajFileDataNewC(EQUDATA,&equfile);
		if (!equfile)
		    equiv=ajFalse;
		else
		    showseq_read_equiv(&equfile, &retable);
	    }

	    ajFileSeek(enzfile, 0L, 0);
	    hits =embPatRestrictMatch(seq, 1, ajSeqLen(seq), enzfile, enzymes,
				      sitelen, plasmid, ambiguity, mincuts,
				      maxcuts, blunt, sticky, commercial,
				      &restrictlist);
	    if (hits)
		/* this bit is lifted from printHits */
		(void) embPatRestrictRestrict(&restrictlist, hits, !limit,
					      ajFalse);

	    /* tidy up */
	    ajFileClose(&enzfile);
	}



	/* make the Show Object */
	ss = embShowNew(seq, begin, end, width, length, margin, html, offset);

	/* get the number of the genetic code used */
	(void) ajStrToInt(tablelist[0], &table);

	if (html)
	    (void) ajFmtPrintF(outfile, "<PRE>");

	(void) showseq_FormatShow(ss, format, trnTable, translaterange,
				  uppercase, highlight, threeletter,
				  numberseq, feat, orfminsize, restrictlist,
				  flat, annotation);

	(void) embShowPrint(outfile, ss);

	/* tidy up */
	(void) embShowDel(&ss);

	(void) ajListDel(&restrictlist);

	/* add a gratuitous newline at the end of the sequence */
	(void) ajFmtPrintF(outfile, "\n");

	if (html)
	    (void) ajFmtPrintF(outfile, "<PRE>");
    }


    /* tidy up */
    (void) ajStrDel(&format);
    ajFileClose(&outfile);
    ajTrnDel (&trnTable);

    (void) ajExit ();
    return 0;
}




/* @funcstatic showseq_FormatShow *********************************************
**
** Set up the EmbPShow object, according to the required format
**
** @param [u] ss [EmbPShow] Show sequence object
** @param [u] format [AjPStr] format codes for the required things to display
** @param [r] trnTable [AjPTrn] genetic code translation table
** @param [r] translaterange [AjPRange] ranges to translate
** @param [r] uppercase [AjPRange] ranges to uppercase
** @param [r] highlight [AjPRange] ranges to colour in HTML
** @param [r] threeletter [AjBool] use 3-letter code
** @param [r] numberseq [AjBool] put numbers on sequences
** @param [r] feat [AjPFeattable] sequence's feature table
** @param [r] orfminsize [ajint] minimum size of ORFs to display
**                              (0 for no ORFs)
** @param [r] restrictlist [AjPList] restriction enzyme cut site list (or NULL)
** @param [r] flat [AjBool] show restriction sites in flat format
** @param [r] annotation [AjPRange] ranges to annotate
** @return [void]
** @@
******************************************************************************/

static void showseq_FormatShow(EmbPShow ss, AjPStr format, AjPTrn trnTable,
			       AjPRange translaterange, AjPRange uppercase,
			       AjPRange highlight,  AjBool threeletter,
			       AjBool numberseq, AjPFeattable feat,
			       ajint orfminsize, AjPList restrictlist,
			       AjBool flat, AjPRange annotation)
{
    AjPStrTok tok;
    char white[] = " \t\n\r";
    char whiteplus[] = " \t,.!@#$%^&*()_+|~`\\={}[]:;\"'<>,.?/";
    AjPStr code = NULL;

    /* start token to parse format */
    tok = ajStrTokenInit(format,  white);
    while (ajStrToken (&code, &tok, whiteplus))
    {
	(void) ajStrToUpper(&code);

	if (!ajStrCmpC(code, "S"))
	    (void) embShowAddSeq(ss, numberseq, threeletter, uppercase,
				 highlight);
	else if (!ajStrCmpC(code, "B"))
	    (void) embShowAddBlank(ss);
	else if (!ajStrCmpC(code, "1"))
	    (void) embShowAddTran (ss, trnTable, 1, threeletter, numberseq, translaterange,
				   orfminsize, AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if (!ajStrCmpC(code, "2"))
	    (void) embShowAddTran (ss, trnTable, 2, threeletter, numberseq, NULL,
				   orfminsize, AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if (!ajStrCmpC(code, "3"))
	    (void) embShowAddTran (ss, trnTable, 3, threeletter, numberseq, NULL, 
				   orfminsize, AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if (!ajStrCmpC(code, "-1"))
	    (void) embShowAddTran (ss, trnTable, -1, threeletter, numberseq, NULL, 
				   orfminsize, AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if (!ajStrCmpC(code, "-2"))
	    (void) embShowAddTran (ss, trnTable, -2, threeletter, numberseq, NULL, 
				   orfminsize, AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if (!ajStrCmpC(code, "-3"))
	    (void) embShowAddTran (ss, trnTable, -3, threeletter, numberseq, NULL, 
				   orfminsize, AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	else if (!ajStrCmpC(code, "T"))
	    (void) embShowAddTicks(ss);
	else if (!ajStrCmpC(code, "N"))
	    (void) embShowAddTicknum(ss);
	else if (!ajStrCmpC(code, "C"))
	    (void) embShowAddComp(ss, numberseq);
	else if (!ajStrCmpC(code, "F"))
	    (void) embShowAddFT(ss, feat);
	else if (!ajStrCmpC(code, "R"))
	    (void) embShowAddRE (ss, 1, restrictlist, flat);
	else if (!ajStrCmpC(code, "-R"))
	    (void) embShowAddRE (ss, -1, restrictlist, flat);
	else if (!ajStrCmpC(code, "A"))
	    (void) embShowAddNote (ss, annotation);
	else
	    (void) ajFatal ("Formatting code not recognised: '%S'", code);
    }

    /* tidy up */
    (void) ajStrDel(&code);
    (void) ajStrTokenClear(&tok);

    return;
}

/* @funcstatic showseq_read_equiv *********************************************
**
** Lifted from Alan's restrict.c but reads the equ file.
**
** @param [r] equfile [AjPFile*] file to read then close.
** @param [wP] table [AjPTable*] table to write to.
** @return [void]
** @@
******************************************************************************/

static void showseq_read_equiv(AjPFile *equfile, AjPTable *table)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    char *p;

    line = ajStrNew();

    while(ajFileReadLine(*equfile,&line))
    {
        p=ajStrStr(line);
        if(!*p || *p=='#' || *p=='!')
            continue;
        p=strtok(p," \t\n");
        key=ajStrNewC(p);
        p=strtok(NULL," \t\n");
        value=ajStrNewC(p);
        ajTablePut(*table,(const void *)key, (void *)value);
    }

    ajFileClose(equfile);
    return;
}


/* @funcstatic showseq_read_file_of_enzyme_names ******************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [r] enzymes [AjPStr*] names of enzymes to search for or 'all'
**                              or '@file'
** @return [void]
** @@
******************************************************************************/

static void showseq_read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file=NULL;
    AjPStr line;
    char   *p=NULL;

    if (ajStrFindC(*enzymes, "@") == 0)
    {
	ajStrTrimC(enzymes, "@");	/* remove the @ */
	file = ajFileNewIn(*enzymes);
	if (file == NULL)
	    ajFatal ("Cannot open the file of enzyme names: '%S'", enzymes);

	/* blank off the enzyme file name and replace with the enzyme names */
	ajStrClear(enzymes);
	line = ajStrNew();
	while(ajFileReadLine(file, &line))
	{
	    p = ajStrStr(line);
	    if (!*p || *p == '#' || *p == '!') continue;
	    ajStrApp(enzymes, line);
	    ajStrAppC(enzymes, ",");
	}
	ajStrDel(&line);

	ajFileClose(&file);
    }

    return;
}

/* @funcstatic showseq_FeatureFilter ******************************************
**
** Removes unwanted features from a feature table
**
** @param [r] featab [AjPFeattable] Feature table to filter
** @param [r] matchsource [AjPStr] Required Source pattern
** @param [r] matchtype [AjPStr] Required Type pattern
** @param [r] matchsense [ajint] Required Sense pattern +1,0,-1
**                               (or other value$
** @param [r] minscore [float] Min required Score pattern
** @param [r] maxscore [float] Max required Score pattern
** @param [r] matchtag [AjPStr] Required Tag pattern
** @param [r] matchvalue [AjPStr] Required Value pattern
** @return [void]
** @@
******************************************************************************/

static void showseq_FeatureFilter(AjPFeattable featab, AjPStr
				  matchsource, AjPStr matchtype,
				  ajint matchsense, float minscore,
				  float maxscore, AjPStr matchtag,
				  AjPStr matchvalue)
{

    AjIList iter = NULL;
    AjPFeature gf = NULL;
    AjBool tagsmatch;

    tagsmatch = ajFalse;

    /* foreach feature in the feature table */
    if (featab)
    {
	iter = ajListIter(featab->Features);
	while(ajListIterMore(iter))
	{
	    gf = (AjPFeature)ajListIterNext(iter);
	    if (!showseq_MatchFeature(gf, matchsource, matchtype, matchsense,
				      minscore, maxscore, matchtag,
				      matchvalue, &tagsmatch))
	    {
		/* no match, so delete feature from feature table */
		ajFeatDel(&gf);
		ajListRemove(iter);
	    }
	}
	ajListIterFree(iter);
    }

    return;
}

/* @funcstatic showseq_MatchFeature *******************************************
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
** @param [rw] tagsmatch [AjBool *] true if a join has matching tag/values
** @return [AjBool] True if feature matches criteria
** @@
******************************************************************************/

static AjBool showseq_MatchFeature (AjPFeature gf, AjPStr
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
      *tagsmatch = showseq_MatchPatternTags(gf, tag, value);
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

/* @funcstatic showseq_MatchPatternTags ***************************************
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

static AjBool showseq_MatchPatternTags (AjPFeature feat, AjPStr tpattern,
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


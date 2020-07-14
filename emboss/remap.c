/* @source remap application
**
** Display a sequence with restriction cut sites
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 18 Jan 2000 - GWW - written
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


#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "embossre.equ"
#define EQUGUESS 3500     /* Estimate of number of equivalent names */

#define TABLEGUESS 200




static void remap_read_equiv(AjPFile *equfile, AjPTable *table);
static void remap_RemoveMinMax(AjPList restrictlist,
			       AjPTable hittable, ajint mincuts,
			       ajint maxcuts);
static void remap_CutList(AjPFile outfile,
	AjPTable hittable, AjBool isos, AjBool html, ajint mincuts,
	ajint maxcuts);
static void remap_NoCutList(AjPFile outfile, AjPTable hittable,
			    AjBool html, AjPStr enzymes, AjBool blunt,
			    AjBool sticky, ajint sitelen, AjBool commercial,
			    AjBool ambiguity, AjBool limit, AjPTable retable);
static void remap_DelTable(AjPTable * table);
static void remap_read_file_of_enzyme_names(AjPStr *enzymes);
static int remap_ajStrCmpCase(const void* str1, const void* str2);
static void remap_ajStrDel(void** str, void* cl);
static void rebase_RenamePreferred(AjPList list, AjPTable table, 
				   AjPList newlist);
static void remap_RestrictPreferred(AjPList l, AjPTable t);
static AjBool remap_Ambiguous(AjPStr str);



/* @datastatic PValue *********************************************************
**
** structure for counts and isoschizomers of a restriction enzyme hit
**
** @alias SValue
** @alias OValue
**
** @attr count [ajint] Undocumented
** @attr iso [AjPStr] Undocumented
******************************************************************************/

typedef struct SValue
{
    ajint  count;
    AjPStr iso;
} OValue, *PValue;




/* @prog remap ****************************************************************
**
** Display a sequence with restriction cut sites, translation etc
**
******************************************************************************/

int main(int argc, char **argv)
{

    ajint begin, end;
    AjPSeqall seqall;
    AjPSeq seq;
    EmbPShow ss;
    AjPFile outfile;
    AjPStr * tablelist;
    ajint table;
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
    ajint orfminsize;
    AjPTrn trnTable;
    AjBool translation;
    AjBool reverse;
    AjBool cutlist;
    AjBool flat;
    EmbPMatMatch mm = NULL;

    /* stuff for tables and lists of enzymes and hits */
    ajint default_mincuts = 1;
    ajint default_maxcuts = 2000000000;
    AjPTable hittable; /* enzyme hits */

    /* stuff lifted from Alan's 'restrict.c' */
    AjPStr enzymes = NULL;
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
    AjPFile enzfile  = NULL;
    AjPFile equfile  = NULL;
    AjPTable retable = NULL;
    ajint hits;
    AjPList restrictlist = NULL;

    embInit("remap", argc, argv);

    seqall      = ajAcdGetSeqall("sequence");
    outfile     = ajAcdGetOutfile("outfile");
    tablelist   = ajAcdGetList("table");
    uppercase   = ajAcdGetRange("uppercase");
    highlight   = ajAcdGetRange("highlight");
    threeletter = ajAcdGetBool("threeletter");
    numberseq   = ajAcdGetBool("number");
    width       = ajAcdGetInt("width");
    length      = ajAcdGetInt("length");
    margin      = ajAcdGetInt("margin");
    nameseq     = ajAcdGetBool("name");
    description = ajAcdGetBool("description");
    offset      = ajAcdGetInt("offset");
    html        = ajAcdGetBool("html");
    orfminsize  = ajAcdGetInt("orfminsize");
    translation = ajAcdGetBool("translation");
    reverse     = ajAcdGetBool("reverse");
    cutlist     = ajAcdGetBool("cutlist");
    flat        = ajAcdGetBool("flatreformat");

    /*  restriction enzyme stuff */
    mincuts    = ajAcdGetInt("mincuts");
    maxcuts    = ajAcdGetInt("maxcuts");
    sitelen    = ajAcdGetInt("sitelen");
    single     = ajAcdGetBool("single");
    blunt      = ajAcdGetBool("blunt");
    sticky     = ajAcdGetBool("sticky");
    ambiguity  = ajAcdGetBool("ambiguity");
    plasmid    = ajAcdGetBool("plasmid");
    commercial = ajAcdGetBool("commercial");
    limit      = ajAcdGetBool("limit");
    enzymes    = ajAcdGetString("enzymes");

    if(!blunt  && !sticky)
	ajFatal("Blunt/Sticky end cutters shouldn't both be disabled.");

    /* get the number of the genetic code used */
    ajStrToInt(tablelist[0], &table);
    trnTable = ajTrnNewI(table);

    /* read the local file of enzymes names */
    remap_read_file_of_enzyme_names(&enzymes);


    while(ajSeqallNext(seqall, &seq))
    {
	/* get begin and end positions */
	begin = ajSeqBegin(seq)-1;
	end   = ajSeqEnd(seq)-1;

	/* do the name and description */
	if(nameseq)
	{
	    if(html)
		ajFmtPrintF(outfile, "<H2>%S</H2>\n",
				   ajSeqGetName(seq));
	    else
		ajFmtPrintF(outfile, "%S\n", ajSeqGetName(seq));
	}

	if(description)
	{
	    /*
	    **  wrap the description line at the width of the sequence
	    **  plus margin
	    */
	    if(html)
		ajFmtPrintF(outfile, "<H3>%S</H3>\n",
				   ajSeqGetDesc(seq));
	    else
	    {
		descriptionline = ajStrNew();
		ajStrAss(&descriptionline, ajSeqGetDesc(seq));
		ajStrWrap(&descriptionline, width+margin);
		ajFmtPrintF(outfile, "%S\n", descriptionline);
		ajStrDel(&descriptionline);
	    }
	}

	/* get the restriction cut sites */
	/*
	**  most of this is lifted from the program 'restrict.c' by Alan
	**  Bleasby
	 */
	if(single)
	    maxcuts=mincuts=1;
	retable = ajStrTableNew(EQUGUESS);
	ajFileDataNewC(ENZDATA, &enzfile);
	if(!enzfile)
	    ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");

	if(limit)
	{
	    ajFileDataNewC(EQUDATA,&equfile);
	    if(!equfile)
		limit = ajFalse;
	    else
		remap_read_equiv(&equfile, &retable);
	}

	ajFileSeek(enzfile, 0L, 0);
	/* search for hits, but don't use mincuts and maxcuts criteria yet */
	hits = embPatRestrictMatch(seq, begin+1, end+1, enzfile, enzymes,
				   sitelen,plasmid, ambiguity, default_mincuts,
				   default_maxcuts, blunt, sticky, commercial,
				   &restrictlist);

	if(hits)
	{
	    /* this bit is lifted from printHits */
	    embPatRestrictRestrict(&restrictlist, hits, !limit,
					  ajFalse);
	    if(limit)
		remap_RestrictPreferred(restrictlist,retable);
	}


	ajFileClose(&enzfile);


	/*
	** Remove those violating the mincuts and maxcuts
	** criteria, but save them in hittable for printing out later.
	** Keep a count of how many hits each enzyme gets in hittable.
	*/
        hittable = ajStrTableNewCase(TABLEGUESS);
	remap_RemoveMinMax(restrictlist, hittable, mincuts, maxcuts);


	/* make the Show Object */
	ss = embShowNew(seq, begin, end, width, length, margin, html, offset);

	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	/* create the format to display */
	embShowAddBlank(ss);
	embShowAddRE(ss, 1, restrictlist, flat);
	embShowAddSeq(ss, numberseq, threeletter, uppercase, highlight);

	if(!numberseq)
	    embShowAddTicknum(ss);
	embShowAddTicks(ss);

	if(reverse)
	{
	    embShowAddComp(ss, numberseq);
	    embShowAddRE(ss, -1, restrictlist, flat);
	}

	if(translation)
	{
	    if(reverse)
		embShowAddBlank(ss);
	    
	    embShowAddTran(ss, trnTable, 1, threeletter,
			   numberseq, NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	    embShowAddTran(ss, trnTable, 2, threeletter,
			   numberseq, NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	    embShowAddTran(ss, trnTable, 3, threeletter,
			   numberseq, NULL, orfminsize,
			   AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	    
	    if(reverse)
	    {
		embShowAddTicks(ss);
		embShowAddTran(ss, trnTable, -3, threeletter,
			       numberseq, NULL, orfminsize,
			       AJFALSE, AJFALSE, AJFALSE, AJFALSE);
		embShowAddTran(ss, trnTable, -2, threeletter,
			       numberseq, NULL, orfminsize,
			       AJFALSE, AJFALSE, AJFALSE, AJFALSE);
		embShowAddTran(ss, trnTable, -1, threeletter,
			       numberseq, NULL, orfminsize,
			       AJFALSE, AJFALSE, AJFALSE, AJFALSE);
	    }
	}

	embShowPrint(outfile, ss);

	/* display a list of the Enzymes that cut and don't cut */
	if(cutlist)
	{
	    remap_CutList(outfile, hittable,
	    		limit, html, mincuts, maxcuts);
	    remap_NoCutList(outfile, hittable, html, enzymes, blunt,
			sticky, sitelen, commercial, ambiguity, 
			limit, retable);
	}

	/* add a gratuitous newline at the end of the sequence */
	ajFmtPrintF(outfile, "\n");

	/* tidy up */
	embShowDel(&ss);

	while(ajListPop(restrictlist,(void **)&mm))
	    embMatMatchDel(&mm);
	ajListFree(&restrictlist);

        remap_DelTable(&hittable);

	ajStrTableFree(&retable);
    }


    ajTrnDel(&trnTable);

    ajExit();

    return 0;
}




/* @funcstatic remap_DelTable *************************************************
**
** Delete the tables with PValue structures
**
** @param [r] table [AjPTable *] table to delete
** @return [void]
** @@
******************************************************************************/

static void remap_DelTable(AjPTable * table)
{

    void **array;		/* array for table */
    ajint i;
    PValue value;

    if(ajTableLength(*table))
    {
      array = ajTableToarray(*table, NULL);
      for(i = 0; array[i]; i += 2)
      {
          value = (PValue) array[i+1];
          ajStrDel(&(value->iso));
          AJFREE(array[i+1]);	/* free the ajint* value */
	  ajStrDel((AjPStr*)&array[i]);
      }
      AJFREE(array);
    }
    ajTableFree(table);

    return;
}




/* @funcstatic remap_RemoveMinMax *********************************************
**
** Remove the enzymes that hit more than maxcut or less than mincut from
** restrictlist.
** Populate hittable with enzymes names and hit counts.
**
** @param [r] restrictlist [AjPList] List to prune
** @param [r] hittable [AjPTable] table of number of hits for each enzyme
** @param [r] mincuts [ajint] mincuts
** @param [r] maxcuts [ajint] maxcuts
** @return [void]
** @@
******************************************************************************/

static void remap_RemoveMinMax(AjPList restrictlist,
	AjPTable hittable, ajint mincuts, ajint maxcuts)
{

    AjIList miter;		/* iterator for matches list */
    EmbPMatMatch m = NULL;	/* restriction enzyme match structure */
    PValue value;
    AjPStr key  = NULL;
    AjPStr keyv = NULL;


    key = ajStrNew();

    /* if no hits then ignore much of this routine */
    if(ajListLength(restrictlist))
    {
        /* count the enzymes */
	miter = ajListIter(restrictlist);
	while((m = ajListIterNext(miter)) != NULL)
	{
	    ajStrAssS(&key, m->cod);

	    /* increment the count of key */
	    value = (PValue) ajTableGet(hittable, (const void *)key);
	    if(value == NULL)
	    {
		AJNEW0(value);
		value->count = 1;
		value->iso = ajStrNew();
		ajStrAssS(&(value->iso), m->iso);
		keyv = ajStrNew();
		ajStrAssS(&keyv,key);
		ajTablePut(hittable, (const void *)keyv, (void *)value);
	    }
	    else
		value->count++;
	}
	ajListIterFree(miter);


	/* now remove enzymes from restrictlist if <mincuts | >maxcuts */
	miter = ajListIter(restrictlist);
	while((m = ajListIterNext(miter)) != NULL)
	{
	    value = (PValue) ajTableGet(hittable, (const void *)(m->cod));
            if(value->count < mincuts || value->count > maxcuts)
	    {
            	ajListRemove(miter);
                embMatMatchDel(&m);
            }
	}
	ajListIterFree(miter);
    }

    ajStrDel(&key);

    return;
}




/* @funcstatic remap_CutList **************************************************
**
** display a list of the enzymes that cut
**
** @param [r] outfile [AjPFile] file to print to.
** @param [r] hittable [AjPTable] table of number of hits for each enzyme
** @param [r] isos [AjBool] True if allow isoschizomers
** @param [r] html [AjBool] dump out html if true.
** @param [r] mincuts [ajint] min required cuts
** @param [r] maxcuts [ajint] max required cuts
** @return [void]
** @@
******************************************************************************/

static void remap_CutList(AjPFile outfile, AjPTable hittable, AjBool isos,
			  AjBool html, ajint mincuts, ajint maxcuts)
{
    PValue value;
    void **array = NULL;		/* array for table */
    ajint i;

    /* print title */
    if(html)
	ajFmtPrintF(outfile, "<H2>");
    ajFmtPrintF(outfile, "\n\n# Enzymes that cut  Frequency");
    if(isos)
	ajFmtPrintF(outfile, "\tIsoschizomers\n");
    else
	ajFmtPrintF(outfile, "\n");

    if(html)
	ajFmtPrintF(outfile, "</H2>\n");

    if(ajTableLength(hittable))
    {
        array = ajTableToarray(hittable, NULL);
        qsort(array, ajTableLength(hittable), 2*sizeof (*array), ajStrCmp);

	/* enzymes that cut the required number of times */
	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	for(i = 0; array[i]; i += 2)
	{
	    value = (PValue) array[i+1];
	    if(value->count >= mincuts && value->count <= maxcuts)
	    ajFmtPrintF(outfile, "%10S\t    %d\t%S\n",
		    (AjPStr) array[i], value->count,
		    value->iso);
        }
        ajFmtPrintF(outfile, "\n");

        if(html)
	    ajFmtPrintF(outfile, "</PRE>\n");
    }

    /* enzymes that cut <mincuts */

    /* print title */
    if(html)
        ajFmtPrintF(outfile, "<H2>");
    ajFmtPrintF(outfile, "\n\n# Enzymes which cut less frequently than the ");
    ajFmtPrintF(outfile, "MINCUTS criterion\n# Enzymes < MINCUTS Frequency");

    if(isos)
        ajFmtPrintF(outfile, "\tIsoschizomers\n");
    else
	ajFmtPrintF(outfile, "\n");

    if(html)
	ajFmtPrintF(outfile, "</H2>\n");

    if(ajTableLength(hittable))
    {
	/* print out results */
	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	for(i = 0; array[i]; i += 2)
	{
	    value = (PValue) array[i+1];
	    if(value->count < mincuts)
	    ajFmtPrintF(outfile, "%10S\t    %d\t%S\n",
			    (AjPStr) array[i], value->count,
			    value->iso);
	}
        ajFmtPrintF(outfile, "\n");

        if(html)
	    ajFmtPrintF(outfile, "</PRE>\n");
    }

    /* enzymes that cut >maxcuts */

    /* print title */
    if(html)
	ajFmtPrintF(outfile, "<H2>");
    ajFmtPrintF(outfile, "\n\n# Enzymes which cut more frequently than the ");
    ajFmtPrintF(outfile, "MAXCUTS criterion\n# Enzymes > MAXCUTS Frequency");

    if(isos)
	ajFmtPrintF(outfile, "\tIsoschizomers\n");
    else
	ajFmtPrintF(outfile, "\n");

    if(html)
	ajFmtPrintF(outfile, "</H2>\n");

    if(ajTableLength(hittable))
    {
	/* print out results */
	if(html)
	    ajFmtPrintF(outfile, "<PRE>");

	for(i = 0; array[i]; i += 2)
	{
	    value = (PValue) array[i+1];
	    if(value->count > maxcuts)
	    ajFmtPrintF(outfile, "%10S\t    %d\t%S\n",
			    (AjPStr) array[i], value->count,
			    value->iso);
	}

        ajFmtPrintF(outfile, "\n");

        if(html)
	    ajFmtPrintF(outfile, "</PRE>\n");

	AJFREE(array);
    }

    return;
}




/* @funcstatic remap_NoCutList ************************************************
**
** display a list of the enzymes that do NOT cut
**
** @param [r] outfile [AjPFile] file to print to.
** @param [r] hittable [AjPTable] Enzymes that cut
** @param [r] html [AjBool] dump out html if true.
** @param [r] enzymes [AjPStr] names of enzymes to search for or 'all'
** @param [r] blunt [AjBool] Allow blunt cutters
** @param [r] sticky [AjBool] Allow sticky cutters
** @param [r] sitelen [ajint] minimum length of recognition site
** @param [r] commercial [AjBool] Allow commercially supplied cutters
** @param [r] ambiguity [AjBool] Allow ambiguous patterns
** @param [r] limit [AjBool] True if allow isoschizomers
** @param [r] retable [AjPTable] Table from embossre.equ file 
** @return [void]
** @@
******************************************************************************/

static void remap_NoCutList(AjPFile outfile, AjPTable hittable,
			    AjBool html, AjPStr enzymes, AjBool blunt,
			    AjBool sticky, ajint sitelen, AjBool commercial,
			    AjBool ambiguity, AjBool limit, AjPTable retable)
{

    /* for iterating over hittable */
    PValue value;
    void **array;			/* array for table */
    ajint i;

    /* list of enzymes that cut */
    AjPList cutlist;
    AjIList citer;			/* iterator for cutlist */
    AjPStr cutname = NULL;
    AjBool found;

    /* for parsing value->iso string */
    AjPStrTok tok;
    char tokens[] = " ,";
    AjPStr code = NULL;
    char *p;

    /* for reading in enzymes names */
    AjPFile enzfile = NULL;
    AjPStr *ea;
    ajint ne;				/* number of enzymes */
    AjBool isall = ajTrue;

    /* list of enzymes that don't cut */
    AjPList nocutlist;
    AjIList niter;			/* iterator for nocutlist */
    AjPStr nocutname = NULL;

    /* count of rejected enzymes not matching criteria */
    ajint rejected_count = 0;

    EmbPPatRestrict enz;

    /* for renaming preferred isoschizomers */
    AjPList newlist;

     /*
     **
     ** Make a list of enzymes('cutlist') that hit
     ** including the isoschizomer names
     **
     */
    ajDebug("Make a list of all enzymes that cut\n");


    cutlist   = ajListstrNew();
    nocutlist = ajListstrNew();


    array = ajTableToarray(hittable, NULL);
    for(i = 0; array[i]; i += 2)
    {
        value = (PValue) array[i+1];
        cutname = ajStrNew();
        ajStrCopy(&cutname, array[i]);
        ajListstrPushApp(cutlist, cutname);

        /* Add to cutlist all isoschizomers of enzymes that cut */
        ajDebug("Add to cutlist all isoschizomers of enzymes that cut\n");

        /* start token to parse isoschizomers names */
        tok = ajStrTokenInit(value->iso,  tokens);
        while(ajStrToken(&code, &tok, tokens))
        {
            cutname = ajStrNew();
            ajStrAss(&cutname, code);
            ajListstrPushApp(cutlist, cutname);
        }
        ajStrTokenClear(&tok);
    }
    ajStrDel(&code);
    AJFREE(array);



     /*
     ** Read in list of enzymes ('nocutlist') - either all or
     ** the input enzyme list.
     ** Exclude those that don't match the selection criteria - count these.
     */

    ajDebug("Read in a list of all input enzyme names\n");

    ne = 0;
    if(!enzymes)
	isall = ajTrue;
    else
    {
	/* get input list of enzymes into ea[] */
	ne = ajArrCommaList(enzymes, &ea);
	if(ajStrMatchCaseC(ea[0], "all"))
	    isall = ajTrue;
	else
	{
	    isall = ajFalse;
	    for(i=0; i<ne; ++i)
		ajStrCleanWhite(&ea[i]);
	}
    }

    ajFileDataNewC(ENZDATA, &enzfile);

    /* push all enzyme names without the required criteria onto nocutlist */

    enz = embPatRestrictNew();
    while(embPatRestrictReadEntry(&enz, &enzfile))
    {
         /* 
	 ** If user entered explicit enzyme list, then check to see if
	 ** this is one of that explicit list 
	 */
        if(!isall)
	{
            found = AJFALSE;
            for(i=0; i<ne; ++i)
                if(ajStrMatchCase(ea[i], enz->cod))
		{
		    found = AJTRUE;
                    break;
                }

	    if(!found)			/* not in the explicit list */
		continue;

	    ajDebug("RE %S is in the input explicit list of REs\n", enz->cod);
	}

	/* ignore ncuts==0 as they are unknown */
	if(!enz->ncuts)
	{
	    /* number of cut positions */
            ajDebug("RE %S has an unknown number of cut positions\n",
		    enz->cod);
	    continue;
	}
        ajDebug("RE %S has a known number of cut sites\n", enz->cod);

	if(enz->len < sitelen)
	{
	    /* recognition site length */
            ajDebug("RE %S does not have a long enough recognition site\n", 
		    enz->cod);
	    rejected_count++;
	    continue;
	}

	if(!blunt && enz->blunt)
	{
	    /* blunt/sticky */
            ajDebug("RE %S is blunt\n", enz->cod);
	    rejected_count++;
	    continue;
	}

	if(!sticky && !enz->blunt)
	{
	    /* blunt/sticky */
            ajDebug("RE %S is sticky\n", enz->cod);
	    rejected_count++;
	    continue;
	}

	/* commercially available enzymes have uppercase patterns */
	p = ajStrStr(enz->pat);

         /* 
	 ** The -commercial qualifier is only used if we are searching
	 ** through 'all' of the REBASE database - if we have specified an
	 ** explicit list of enzymes then they are searched for whether or
	 ** not they are commercially available
	 */
	if((*p >= 'a' && *p <= 'z') && commercial && isall)
	{
            ajDebug("RE %S is not commercial\n", enz->cod);
	    rejected_count++;
	    continue;
        }

	if(!ambiguity && remap_Ambiguous(enz->pat)) {
	    ajDebug("RE %S is ambiguous\n", enz->cod);
	    rejected_count++;
	    continue;	
	}

        ajDebug("RE %S matches all required criteria\n", enz->cod);

        code = ajStrNew();
	ajStrAssS(&code, enz->cod);
	ajListstrPushApp(nocutlist, code);
    }
    embPatRestrictDel(&enz);
    ajFileClose(&enzfile);


    for(i=0; i<ne; ++i)
	if(ea[i])
	    ajStrDel(&ea[i]);

    if(ne)
	AJFREE(ea);

    /*
     ** Change names of enzymes in the non-cutter list
     ** to that of preferred (prototype) 
     ** enzyme name so that the isoschizomers of cutters
     ** will be removed from the 
     ** non-cutter list in the next bit.
     ** Remove duplicate prototype names.
     */
    if(limit)
    {
        newlist = ajListstrNew();
        rebase_RenamePreferred(nocutlist, retable, newlist);
        ajListstrFree(&nocutlist);
        nocutlist = newlist;
        ajListUnique(nocutlist, remap_ajStrCmpCase, remap_ajStrDel);
    }


     /*
     ** Iterate through the list of input enzymes removing those that are in
     ** the cutlist.
     */

    ajDebug("Remove from the nocutlist all enzymes and isoschizomers "
	    "that cut\n");

     /*
     **  This steps down both lists at the same time, comparing names and
     **  iterating to the next name in whichever list whose name compares
     **  alphabetically before the other.  Where a match is found, the
     **  nocutlist item is deleted.
     */

    ajListSort(nocutlist, remap_ajStrCmpCase);
    ajListSort(cutlist, remap_ajStrCmpCase);

    citer = ajListIter(cutlist);
    niter = ajListIter(nocutlist);

    /*
       while((cutname = (AjPStr)ajListIterNext(citer)) != NULL)
       ajDebug("dbg cutname = %S\n", cutname);
       */

    nocutname = (AjPStr)ajListIterNext(niter);
    cutname   = (AjPStr)ajListIterNext(citer);

    ajDebug("initial cutname, nocutname: '%S' '%S'\n", cutname, nocutname);

    while(nocutname != NULL && cutname != NULL)
    {
	i = ajStrCmpCase(cutname, nocutname);
	ajDebug("compare cutname, nocutname: %S %S ", cutname, nocutname);
	ajDebug("ajStrCmpCase=%d\n", i);
	if(i == 0)
	{
	    /* match - so remove from nocutlist */
	    ajDebug("ajListstrRemove %S\n", nocutname);
	    ajListstrRemove(niter);
	    nocutname = (AjPStr)ajListIterNext(niter);
	     /* 
	     ** Don't increment the cutname list pointer here
	     ** - there may be more than one entry in the nocutname
	     ** list with the same name because we have converted 
	     ** isoschizomers to their preferred name
	     */
	    /* cutname = (AjPStr)ajListIterNext(citer); */
	}
	else if(i == -1)
	    /* cutlist name sorts before nocutlist name */
	    cutname = (AjPStr)ajListIterNext(citer);
	else if(i == 1)
	    /* nocutlist name sorts before cutlist name */
	    nocutname = (AjPStr)ajListIterNext(niter);
    }

    ajListIterFree(citer);
    ajListIterFree(niter);
    ajListstrFree(&cutlist);


     /* Print the resulting list of those that do not cut*/

    ajDebug("Print out the list\n");

    /* print the title */
    if(html)
	ajFmtPrintF(outfile, "<H2>");
    ajFmtPrintF(outfile, "\n\n# Enzymes that do not cut\n\n");

    if(html)
	ajFmtPrintF(outfile, "</H2>\n");

    if(html)
	ajFmtPrintF(outfile, "<PRE>");

    /*  ajListSort(nocutlist, ajStrCmp);*/
    niter = ajListIter(nocutlist);
    i = 0;
    while((nocutname = (AjPStr)ajListIterNext(niter)) != NULL)
    {
	ajFmtPrintF(outfile, "%-10S", nocutname);
	/* new line after every 7 names printed */
	if(i++ == 7)
	{
	    ajFmtPrintF(outfile, "\n");
	    i = 0;
	}
    }
    ajListIterFree(niter);


    /* end the output */
    ajFmtPrintF(outfile, "\n");
    if(html) {ajFmtPrintF(outfile, "</PRE>\n");}



     /*
     ** Print the count of rejected enzymes
     ** N.B. This is the count of ALL rejected enzymes including all
     ** isoschizomers
     */

    if(html)
        ajFmtPrintF(outfile, "<H2>");
    ajFmtPrintF(outfile,
		"\n\n# No. of cutting enzymes which do not match the\n"
		"# SITELEN, BLUNT, STICKY, COMMERCIAL, AMBIGUOUS citeria\n\n");
    if(html)
	ajFmtPrintF(outfile, "</H2>\n");
    ajFmtPrintF(outfile, "%d\n", rejected_count);

    ajDebug("Tidy up\n");
    ajListstrFree(&nocutlist);
    ajListstrFree(&cutlist);

    return;
}




/* @funcstatic remap_read_equiv ***********************************************
**
** Lifted from Alan's restrict.c - reads the embossre.equ file.
**
** @param [r] equfile [AjPFile*] file to read then close.
** @param [wP] table [AjPTable*] table to write to.
** @return [void]
** @@
******************************************************************************/

static void remap_read_equiv(AjPFile *equfile, AjPTable *table)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    char *p;

    line = ajStrNew();

    while(ajFileReadLine(*equfile,&line))
    {
        p = ajStrStr(line);

        if(!*p || *p=='#' || *p=='!')
            continue;
        p = strtok(p," \t\n");
        key = ajStrNewC(p);
        p = strtok(NULL," \t\n");
        value = ajStrNewC(p);
        ajTablePut(*table,(const void *)key, (void *)value);
    }

    ajFileClose(equfile);
    ajStrDel(&line);

    return;
}




/* @funcstatic remap_read_file_of_enzyme_names ********************************
**
** If the list of enzymes starts with a '@' it opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [r] enzymes [AjPStr*] names of enzymes to search for or 'all' or
**                              '@file'
** @return [void]
** @@
******************************************************************************/

static void remap_read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file = NULL;
    AjPStr line;
    char *p = NULL;

    if(ajStrFindC(*enzymes, "@") == 0)
    {
	ajStrTrimC(enzymes, "@");	/* remove the @ */
	file = ajFileNewIn(*enzymes);
	if(file == NULL)
	    ajFatal("Cannot open the file of enzyme names: '%S'", enzymes);

	/* blank off the enzyme file name and replace with the enzyme names */
	ajStrClear(enzymes);
	line = ajStrNew();
	while(ajFileReadLine(file, &line))
	{
	    p = ajStrStr(line);
	    if(!*p || *p == '#' || *p == '!')
		continue;
	    ajStrApp(enzymes, line);
	    ajStrAppC(enzymes, ",");
	}
	ajStrDel(&line);

	ajFileClose(&file);
    }

    return;
}




/* @funcstatic remap_ajStrCmpCase *********************************************
**
** Compares the value of two strings for use in sorting (e.g. ajListSort)
** Case Independent!
**
** @param [r] str1 [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

static int remap_ajStrCmpCase(const void* str1, const void* str2) 
{
    const char* cp;
    const char* cq;

    for(cp = (*(AjPStr*)str1)->Ptr, cq = (*(AjPStr*)str2)->Ptr;
	*cp && *cq; cp++, cq++)
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}

    if(*cp)
	return 1;

    if(*cq)
	return -1;

    return 0;
}




/* @funcstatic remap_ajStrDel *********************************************
**
** Deletes a string when called by ajListUnique
**
** @param [r] str [void**] string to delete
** @param [r] cl [void*] not used
** @return [void]
** @@
******************************************************************************/

static void remap_ajStrDel(void** str, void* cl) 
{
    ajStrDel((AjPStr*)str);

    return;
}




/* @funcstatic rebase_RenamePreferred *****************************************
**
** Iterates through a list of strings
** Forteach string it checks if that string occurs as
** a key in a table.
** If a match is found then the value of the table entry is appended
** to the output list, else the old string name is appended to the output list.
**
** @param [r] list [AjPList] Inout list of strings
** @param [r] table [AjPTable] Table of replacements
** @param [u] newlist [AjPList] Returned new list of strings
** @return [void]
** @@
******************************************************************************/

static void rebase_RenamePreferred(AjPList list, AjPTable table, 
				   AjPList newlist) 
{
    AjIList iter = NULL;
    AjPStr key   = NULL;
    AjPStr value = NULL;
    AjPStr name  = NULL;

    iter = ajListIter(list);
               
    while((key = (AjPStr)ajListIterNext(iter)))
    {
        /* 
        ** If a key-value entry found, write the new value to the new list
        ** else write the old key name to the new list
        */
        value = ajTableGet(table, key);
        name = ajStrNew();
        if(value)
	{
	    ajDebug("Rename: %S renamed to %S\n", key, value);
            ajStrAssS(&name, value);
	}
	else
	{
	    ajDebug("Rename: %S not found\n", key);
	    ajStrAssS(&name, key);
	}
        ajListstrPushApp(newlist, name);
    }
                      
    ajListIterFree(iter);

    return; 
}




/* @funcstatic remap_RestrictPreferred ***************************************
**
** Replace RE names by the name of the prototype for that RE
** This is derived from embPatRestrictPreferred - it differs in that it also
** converts the name of the prototype RE in the list of isoschizomers
** into the name that is being changed in m->cod.
** i.e a name of X with prototype name B and isoschizomer list of "A, B, C"
** will be change to a name of B and isoschizomer list of "A, X, C"
** If the old name is not in the isoschizomer list, it will be added to it.
**
** @param [u] l [AjPList] list of EmbPMatMatch hits
** @param [r] t [AjPTable] table from embossre.equ file
**
** @return [void]
** @@
******************************************************************************/

static void remap_RestrictPreferred(AjPList l, AjPTable t)
{
    AjIList iter   = NULL;
    EmbPMatMatch m = NULL;
    AjPStr value   = NULL;
    AjPStr newiso  = NULL;
    AjBool found;		/* name found in isoschizomer list */
        	    
    /* for parsing value->iso string */
    AjPStrTok tok;
    char tokens[] = " ,";
    AjPStr code = NULL;

    iter = ajListIter(l);
    
    while((m = (EmbPMatMatch)ajListIterNext(iter)))
    {
	found = ajFalse;
	
    	/* get prototype name */
    	value = ajTableGet(t, m->cod);
    	if(value) 
    	{
    	    newiso = ajStrNew();
	    /* parse isoschizomer names from m->iso */
            tok = ajStrTokenInit(m->iso,  tokens);
            while(ajStrToken(&code, &tok, tokens))
            {
	        if(ajStrLen(newiso) > 0)
	            ajStrAppC(&newiso, ",");

		/* found the prototype name? */
	        if(!ajStrCmpCase(code, value)) 
	        {
	            ajStrApp(&newiso, m->cod);
	            found = ajTrue;
	        }
		else
	            ajStrApp(&newiso, code);
            }
            ajStrTokenClear(&tok);

	    /* if the name was not replaced, then add it in now */
	    if(!found)
	    {
	        if(ajStrLen(newiso) > 0)
	            ajStrAppC(&newiso, ",");

	        ajStrApp(&newiso, m->cod);	
	    }
	    
	    ajDebug("RE: %S -> %S iso=%S newiso=%S\n", m->cod, value,
		    m->iso, newiso);

	    /* replace the old iso string with the new one */
	    ajStrAss(&m->iso, newiso);

	    /* rename the enzyme to the prototype name */
    	    ajStrAssS(&m->cod, value);
    	}
    }
    
    ajListIterFree(iter);     
    ajStrDel(&newiso);
    ajStrDel(&code);

    return; 
}

/* @funcstatic remap_Ambiguous  ***************************************
**
** Tests whether there are ambiguity base codes in a string
**
** @param [r] str [AjPStr] String to test
**
** @return [ajBool] True is ambiguous bases found
** @@
******************************************************************************/

static AjBool remap_Ambiguous(AjPStr str)
{
    ajint ipos;
    char chr;
    
    for (ipos=0; ipos<ajStrLen(str); ipos++) 
    {
    	chr = ajStrChar(str, ipos);
    	if (tolower(chr) != 'a' &&
    	    tolower(chr) != 'c' &&
    	    tolower(chr) != 'g' &&
    	    tolower(chr) != 't'
    	    )
            return ajTrue;
    }

    return ajFalse;
}

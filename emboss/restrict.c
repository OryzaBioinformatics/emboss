 /* @source restrict application
 **
 ** Reports restriction enzyme cleavage sites
 ** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
 ** @@
 **
 ** The author wishes to thank Helge Horch for important
 ** discussions and suggestions in the production of this program
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
 *****************************************************************************/


#include "emboss.h"
#include <string.h>
#include <limits.h>

#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "embossre.equ"


#define EQUGUESS 3500	  /* Estimate of number of equivalent names */


static void restrict_reportHits(AjPReport report, AjPSeq seq,
				AjPFeattable TabRpt, AjPList *l,
				ajint hits, ajint begin, ajint end,
				AjBool ambiguity, ajint mincut, ajint maxcut,
				AjBool plasmid, AjBool blunt, AjBool sticky,
				ajint sitelen, AjBool limit,
				AjPTable table, AjBool alpha, AjBool frags,
				AjBool nameit, AjBool ifrag);
static void restrict_printHits(AjPFile *outf, AjPList *l, AjPStr *name,
			       ajint hits, ajint begin, ajint end,
			       AjBool ambiguity, ajint mincut, ajint maxcut,
			       AjBool plasmid, AjBool blunt, AjBool sticky,
			       ajint sitelen, AjBool limit,
			       AjPTable table, AjBool alpha, AjBool frags,
			       AjBool nameit);
static void restrict_read_equiv(AjPFile *equfile, AjPTable *table);
static void restrict_read_file_of_enzyme_names(AjPStr *enzymes);
static ajint restrict_enzcompare(const void *a, const void *b);


/* @prog restrict *************************************************************
**
** Finds restriction enzyme cleavage sites
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPStr    enzymes=NULL;
    AjPFile   outf=NULL;
    AjPReport report=NULL;
    AjPFeattable TabRpt=NULL;
    ajint begin;
    ajint end;
    ajint min;
    ajint max;
    ajint sitelen;
    AjBool alpha;
    AjBool single;
    AjBool blunt;
    AjBool sticky;
    AjBool ambiguity;
    AjBool plasmid;
    AjBool commercial;
    AjBool nameit;
    AjBool limit;
    AjBool frags;
    AjPStr dfile;

    AjPFile   enzfile=NULL;
    AjPFile   equfile=NULL;

    AjPStr    name=NULL;

    AjPTable  table=NULL;

    ajint       hits;


    AjPList     l;
    AjBool      ifrag;

    
    embInit("restrict", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    enzymes   = ajAcdGetString("enzymes");
    report    = ajAcdGetReport("outfile");
    min       = ajAcdGetInt("min");
    max       = ajAcdGetInt("max");
    sitelen   = ajAcdGetInt("sitelen");
    blunt     = ajAcdGetBool("blunt");
    sticky    = ajAcdGetBool("sticky");
    single    = ajAcdGetBool("single");
    alpha     = ajAcdGetBool("alphabetic");
    ambiguity = ajAcdGetBool("ambiguity");
    plasmid   = ajAcdGetBool("plasmid");
    commercial = ajAcdGetBool("commercial");
    limit      = ajAcdGetBool("limit");
    frags      = ajAcdGetBool("fragments");
    nameit     = ajAcdGetBool("name");
    dfile      = ajAcdGetString("datafile");
    ifrag      = ajAcdGetBool("solofragment");
    
    /* obsolete. Can be uncommented in acd file and here to reuse */

    /* outf      = ajAcdGetOutfile("originalfile"); */

    if (!blunt  && !sticky)
	ajFatal("Blunt/Sticky end cutters shouldn't both be disabled.");


    if(single) max=min=1;

    table = ajStrTableNew(EQUGUESS);


    /* read the local file of enzymes names */
    restrict_read_file_of_enzyme_names(&enzymes);

     if(!*ajStrStr(dfile))
    {
	ajFileDataNewC(ENZDATA,&enzfile);
	if(!enzfile)
	    ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");
    }
    else
    {
	enzfile = ajFileNewIn(dfile);
	if(!enzfile)
	    ajFatal("Cannot locate user supplied enzyme file %S.",dfile);
    }



    if(limit)
    {
	ajFileDataNewC(EQUDATA,&equfile);
	if(!equfile)
	    limit=ajFalse;
	else
	    restrict_read_equiv(&equfile,&table);
    }



    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	ajFileSeek(enzfile,0L,0);

	TabRpt = ajFeattableNewSeq(seq);

	hits = embPatRestrictMatch(seq,begin,end,enzfile,enzymes,sitelen,
				   plasmid,ambiguity,min,max,blunt,sticky,
				   commercial,&l);

	if(hits)
	{
	    name = ajStrNewC(ajSeqName(seq));
	      restrict_reportHits(report, seq, TabRpt,
				  &l,hits,begin,end,
				  ambiguity,min,max,
				  plasmid,blunt,sticky,sitelen,
				  limit,table,
				  alpha,frags,nameit,ifrag);
	    if (outf)
	      restrict_printHits(&outf,&l,&name,hits,begin,end,
				 ambiguity,min,max,
				 plasmid,blunt,sticky,sitelen,
				 limit,table,
				 alpha,frags,nameit);
	    ajStrDel(&name);
	}

	ajReportWrite(report, TabRpt, seq);
	ajFeattableDel(&TabRpt);

	ajListFree(&l);
    }


    ajSeqDel(&seq);
    ajFileClose(&enzfile);
    if (outf)
      ajFileClose(&outf);

    (void) ajReportClose(report);

    ajExit();
    return 0;
}





/* @funcstatic restrict_printHits *********************************************
**
** Print restriction sites
**
** @param [w] outf [AjPFile*] outfile
** @param [w] l [AjPList*] hits
** @param [r] name [AjPStr*] sequence name
** @param [r] hits [ajint] number of hits
** @param [r] begin [ajint] start position
** @param [r] end [ajint] end position
** @param [r] ambiguity [AjBool] allow ambiguities
** @param [r] mincut [ajint] minimum cuts
** @param [r] maxcut [ajint] maximum cuts
** @param [r] plasmid [AjBool] circular
** @param [r] blunt [AjBool] allow blunt cutters
** @param [r] sticky [AjBool] allow sticky cutters
** @param [r] sitelen [ajint] length of cut site
** @param [r] limit [AjBool] limit count
** @param [r] table [AjPTable] supplier table
** @param [r] alpha [AjBool] alphabetic sort
** @param [r] frags [AjBool] show fragment lengths
** @param [r] nameit [AjBool] show name
** @@
******************************************************************************/


static void restrict_printHits(AjPFile *outf, AjPList *l, AjPStr *name,
			       ajint hits, ajint begin, ajint end,
			       AjBool ambiguity, ajint mincut, ajint maxcut,
			       AjBool plasmid, AjBool blunt, AjBool sticky,
			       ajint sitelen, AjBool limit,
			       AjPTable table, AjBool alpha, AjBool frags,
			       AjBool nameit)
{
    EmbPMatMatch m=NULL;
    AjPStr  ps=NULL;
    ajint *fa=NULL;
    ajint *fx=NULL;
    ajint fc=0;
    ajint fn=0;
    ajint fb=0;
    ajint last=0;

    AjPStr value=NULL;

    ajint i;
    ajint c=0;

    ps=ajStrNew();
    fn = 0;


    ajFmtPrintF(*outf,"# Restrict of %S from %d to %d\n#\n",
		*name,begin,end);
    ajFmtPrintF(*outf,"# Minimum cuts per enzyme: %d\n",mincut);
    ajFmtPrintF(*outf,"# Maximum cuts per enzyme: %d\n",maxcut);
    ajFmtPrintF(*outf,"# Minimum length of recognition site: %d\n",
		sitelen);
    if(blunt)
	ajFmtPrintF(*outf,"# Blunt ends allowed\n");
    if(sticky)
	ajFmtPrintF(*outf,"# Sticky ends allowed\n");
    if(plasmid)
	ajFmtPrintF(*outf,"# DNA is circular\n");
    else
	ajFmtPrintF(*outf,"# DNA is linear\n");
    if(!ambiguity)
	ajFmtPrintF(*outf,"# No ambiguities allowed\n");
    else
	ajFmtPrintF(*outf,"# Ambiguities allowed\n");



    hits = embPatRestrictRestrict(l,hits,!limit,alpha);

    if(frags)
    {
	fa = AJALLOC(hits*2*sizeof(ajint));
	fx = AJALLOC(hits*2*sizeof(ajint));
    }


    ajFmtPrintF(*outf,"# Number of hits: %d\n",hits);
    ajFmtPrintF(*outf,"# Base Number\tEnzyme\t\tSite\t\t5'\t3'\t[5'\t3']\n");
    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	if(!plasmid && (m->cut1-m->start>100 ||
			m->cut2-m->start>100))
	{
	    embMatMatchDel(&m);
	    continue;
	}


	if(limit)
	{
	    value=ajTableGet(table,m->cod);
	    if(value)
		ajStrAss(&m->cod,value);
	}


	ajFmtPrintF(*outf,"\t%-d\t%-16s%-16s%d\t%d\t",m->start,
		    ajStrStr(m->cod),ajStrStr(m->pat),m->cut1,
		    m->cut2);
	if(frags)
	    fa[fn++] = m->cut1;

	if(m->cut3 && m->cut4)
	{
	    if(frags)
		fa[fn++] = m->cut3;
	    ajFmtPrintF(*outf,"%d\t%d",m->cut3,m->cut4);
	}
	if(nameit)
	    ajFmtPrintF(*outf,"  %S",*name);

	ajFmtPrintF(*outf,"\n");
	embMatMatchDel(&m);
    }



    if(frags)
    {
	ajSortIntInc(fa,fn);
	ajFmtPrintF(*outf,"\n\nFragment lengths:\n");
	if(!fn || (fn==1 && plasmid))
	    ajFmtPrintF(*outf,"    %d\n",end-begin+1);
	else
	{
	    last = -1;
	    fb=0;
	    for(i=0;i<fn;++i)
	    {
		if((c=fa[i])!=last)
		    fa[fb++]=c;
		last=c;
	    }
	    fn=fb;
	    /* Calc lengths */
	    for(i=0;i<fn-1;++i)
		fx[fc++]=fa[i+1]-fa[i];
	    if(!plasmid)
	    {
		fx[fc++]=fa[0]-begin+1;
		fx[fc++]=end-fa[fn-1];
	    }
	    else
		fx[fc++]=(fa[0]-begin+1)+(end-fa[fn-1]);
	    ajSortIntDec(fx,fc);
	    for(i=0;i<fc;++i)
		ajFmtPrintF(*outf,"    %d\n",fx[i]);
	}
	AJFREE(fa);
	AJFREE(fx);
    }


    ajListDel(l);
    ajStrDel(&ps);


    return;
}


/* @funcstatic restrict_reportHits ********************************************
**
** Print restriction sites
**
** @param [w] report [AjPReport] report
** @param [r] seq [AjPSeq] sequence object
** @param [w] TabRpt [AjPFeattable] feature table object to store results
** @param [w] l [AjPList*] hits
** @param [r] hits [ajint] number of hits
** @param [r] begin [ajint] start position
** @param [r] end [ajint] end position
** @param [r] ambiguity [AjBool] allow ambiguities
** @param [r] mincut [ajint] minimum cuts
** @param [r] maxcut [ajint] maximum cuts
** @param [r] plasmid [AjBool] circular
** @param [r] blunt [AjBool] allow blunt cutters
** @param [r] sticky [AjBool] allow sticky cutters
** @param [r] sitelen [ajint] length of cut site
** @param [r] limit [AjBool] limit count
** @param [r] table [AjPTable] supplier table
** @param [r] alpha [AjBool] alphabetic sort
** @param [r] frags [AjBool] show fragment lengths
** @param [r] nameit [AjBool] show name
** @param [r] ifrag [AjBool] show fragments for individual REs
** @@
******************************************************************************/


static void restrict_reportHits(AjPReport report, AjPSeq seq,
				AjPFeattable TabRpt, AjPList *l,
				ajint hits, ajint begin, ajint end,
				AjBool ambiguity, ajint mincut, ajint maxcut,
				AjBool plasmid, AjBool blunt, AjBool sticky,
				ajint sitelen, AjBool limit,
				AjPTable table, AjBool alpha, AjBool frags,
				AjBool nameit, AjBool ifrag)
{
    AjPFeature gf = NULL;
    EmbPMatMatch m=NULL;
    AjPStr  ps=NULL;
    ajint *fa=NULL;
    ajint *fx=NULL;
    ajint fc=0;
    ajint fn=0;
    ajint fb=0;
    ajint last=0;

    AjPStr value=NULL;
    AjPStr tmpStr=NULL;
    AjPStr fthit = NULL;
    AjPStr fragStr = NULL;
    AjPStr codStr = NULL;
    
    ajint i;
    ajint j;
    
    ajint c=0;
    ajint len;
    
    AjPInt farray = NULL;
    ajint  nfrags;



    farray = ajIntNew();
    ps=ajStrNew();
    fragStr = ajStrNew();
    codStr  = ajStrNew();
    
    fn = 0;
    len = ajSeqLen(seq);
    
    ajStrAssC(&fthit, "hit");

    ajFmtPrintAppS(&tmpStr,"Minimum cuts per enzyme: %d\n",mincut);
    ajFmtPrintAppS(&tmpStr,"Maximum cuts per enzyme: %d\n",maxcut);
    ajFmtPrintAppS(&tmpStr,"Minimum length of recognition site: %d\n",
		sitelen);
    if(blunt)
	ajFmtPrintAppS(&tmpStr,"Blunt ends allowed\n");
    if(sticky)
	ajFmtPrintAppS(&tmpStr,"Sticky ends allowed\n");
    if(plasmid)
	ajFmtPrintAppS(&tmpStr,"DNA is circular\n");
    else
	ajFmtPrintAppS(&tmpStr,"DNA is linear\n");
    if(!ambiguity)
	ajFmtPrintAppS(&tmpStr,"No ambiguities allowed\n");
    else
	ajFmtPrintAppS(&tmpStr,"Ambiguities allowed\n");


    hits = embPatRestrictRestrict(l,hits,!limit,alpha);

    if(frags)
    {
	fa = AJALLOC(hits*2*sizeof(ajint));
	fx = AJALLOC(hits*2*sizeof(ajint));
    }

    ajReportSetHeader(report, tmpStr);

    /* not needed - column headings from the old report */
    /*
    // ajFmtPrintAppS(&tmpStr,
    //		   "Base Number\tEnzyme\t\tSite\t\t5'\t3'\t[5'\t3']\n");
    */

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	ajListPushApp(*l,(void *)m);	/* Might need for ifrag display */
	if(!plasmid && (m->cut1 - m->start>100 ||
			m->cut2 - m->start>100))
	{
/*	    embMatMatchDel(&m); */
	    continue;
	}


	if(limit)
	{
	    value=ajTableGet(table,m->cod);
	    if(value)
		ajStrAssS(&m->cod,value);
	}

	gf = ajFeatNewII (TabRpt,
			   m->start, m->start+ajStrLen(m->pat)-1);
	ajFmtPrintS(&tmpStr, "*enzyme %S", m->cod);
	ajFeatTagAdd(gf,  NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*site %S", m->pat);
	ajFeatTagAdd(gf,  NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*5prime %d", m->cut1);
	ajFeatTagAdd(gf,  NULL, tmpStr);
	ajFmtPrintS(&tmpStr, "*3prime %d", m->cut2);
	ajFeatTagAdd(gf,  NULL, tmpStr);

	if(frags)
	    fa[fn++] = m->cut1;

	if(m->cut3 && m->cut4)
	{
	    if(frags)
		fa[fn++] = m->cut3;
	    ajFmtPrintS(&tmpStr, "*5primerev %d", m->cut3);
	    ajFeatTagAdd(gf,  NULL, tmpStr);
	    ajFmtPrintS(&tmpStr, "*3primerev %d", m->cut4);
	    ajFeatTagAdd(gf,  NULL, tmpStr);
	}
    }


    ajStrAssC (&tmpStr, "");


    if(frags)
    {
        ajStrAssC (&tmpStr, "");
	ajSortIntInc(fa,fn);
	ajFmtPrintAppS(&tmpStr,"Fragment lengths:\n");
	if(!fn || (fn==1 && plasmid)) {
	    ajFmtPrintAppS(&tmpStr,"    %d\n",end-begin+1);
	}
	else
	{
	    last = -1;
	    fb=0;
	    for(i=0;i<fn;++i)
	    {
		if((c=fa[i])!=last)
		    fa[fb++]=c;
		last=c;
	    }
	    fn=fb;
	    /* Calc lengths */
	    for(i=0;i<fn-1;++i)
		fx[fc++]=fa[i+1]-fa[i];
	    if(!plasmid)
	    {
		fx[fc++]=fa[0]-begin+1;
		fx[fc++]=end-fa[fn-1];
	    }
	    else
		fx[fc++]=(fa[0]-begin+1)+(end-fa[fn-1]);
	    ajSortIntDec(fx,fc);

	    for(i=0;i<fc;++i)
		ajFmtPrintAppS(&tmpStr,"    %d\n",fx[i]);
	}



	AJFREE(fa);
	AJFREE(fx);
    }


    if(ifrag)
    {
	ajListSort(*l,restrict_enzcompare);

	nfrags = 0;
	ajStrAssC(&fragStr,"");

	for(i=0;i<hits;++i)
	{
	    ajListPop(*l,(void **)&m);
	    ajListPushApp(*l,(void *)m);

	    if(!plasmid && (m->cut1 - m->start>100 ||
			    m->cut2 - m->start>100))
		continue;


	    if(limit)
	    {
		value=ajTableGet(table,m->cod);
		if(value)
		    ajStrAssS(&m->cod,value);
	    }


	    if(!ajStrLen(codStr))
		ajStrAssS(&codStr,m->cod);


	    if(ajStrMatch(codStr,m->cod))
	    {
		if(m->cut3 && m->cut4)
		    ajIntPut(&farray,nfrags++,m->cut3);
		ajIntPut(&farray,nfrags++,m->cut1);
	    }
	    else
	    {
		ajFmtPrintAppS(&fragStr,"\n%S:\n[%S]",
			       codStr,m->pat);
		ajStrAssS(&codStr,m->cod);

		ajSortIntInc(ajIntInt(farray),nfrags);


		fa = AJALLOC(nfrags*sizeof(ajint)+1);		
		last = 0;
		for(j=0;j<nfrags;++j)
		{
		    fa[j] = ajIntGet(farray,j) - last;
		    last  = ajIntGet(farray,j);
		}

		if(!(nfrags && plasmid))
		{
		    fa[j] = len - ajIntGet(farray,j-1);
		    ++nfrags;
		}
		else
		{
		    if(nfrags == 1)
			fa[0] = len;
		    else
			fa[0] += (len - ajIntGet(farray,j-1));
		}

		ajSortIntInc(fa,nfrags);

		for(j=0;j<nfrags;++j)
		{
		    if(!(j%6))
			ajFmtPrintAppS(&fragStr,"\n");
		    ajFmtPrintAppS(&fragStr,"\t%d",fa[j]);
		}
		ajFmtPrintAppS(&fragStr,"\n");

		AJFREE(fa);
		nfrags = 0;
		if(m->cut3 && m->cut4)
		    ajIntPut(&farray,nfrags++,m->cut3);
		ajIntPut(&farray,nfrags++,m->cut1);

	    }
	}


	if(nfrags)
	{
	    ajFmtPrintAppS(&fragStr,"\n%S:\n[%S]",
			   codStr,m->pat);
	    ajStrAssS(&codStr,m->cod);

	    ajSortIntInc(ajIntInt(farray),nfrags);


	    fa = AJALLOC(nfrags*sizeof(ajint)+1);		
	    last = 0;
	    for(j=0;j<nfrags;++j)
	    {
		fa[j] = ajIntGet(farray,j) - last;
		last  = ajIntGet(farray,j);
	    }

	    if(!(nfrags && plasmid))
	    {
		fa[j] = len - ajIntGet(farray,j-1);
		++nfrags;
	    }
	    else
	    {
		if(nfrags == 1)
		    fa[0] = len;
		else
		    fa[0] += (len - ajIntGet(farray,j-1));
	    }

	    ajSortIntInc(fa,nfrags);

	    for(j=0;j<nfrags;++j)
	    {
		if(!(j%6))
		    ajFmtPrintAppS(&fragStr,"\n");
		ajFmtPrintAppS(&fragStr,"\t%d",fa[j]);
	    }
	    ajFmtPrintAppS(&fragStr,"\n");
	    AJFREE(fa);
	}
    }
    


    if(ifrag)
	ajStrApp(&tmpStr,fragStr);
    
    ajReportSetTail (report, tmpStr);

    

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	embMatMatchDel(&m);
    }
    
    ajIntDel(&farray);
    ajListDel(l);
    ajStrDel(&fragStr);
    ajStrDel(&codStr);
    ajStrDel(&ps);
    ajStrDel(&tmpStr);
    ajStrDel(&fthit);

    return;
}

/* @funcstatic restrict_read_equiv ********************************************
**
** Read table of equivalents
**
** @param [r] equfile [AjPFile*] equivalent name file
** @param [w] table [AjPTable*]  equivalent names
** @@
******************************************************************************/


static void restrict_read_equiv(AjPFile *equfile, AjPTable *table)
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

/* @funcstatic restrict_read_file_of_enzyme_names *****************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [r] enzymes [AjPStr*] enzymes to search for or 'all' or '@file'
** @return [void]
** @@
******************************************************************************/

static void restrict_read_file_of_enzyme_names(AjPStr *enzymes)
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



/* @funcstatic restrict_enzcompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] enzyme1
** @param [r] b [const void*] enzyme2
** @return [ajint] 0 = bases match
** @@
******************************************************************************/


static ajint restrict_enzcompare(const void *a, const void *b)
{
    return strcmp((*(EmbPMatMatch*)a)->cod->Ptr,
		  (*(EmbPMatMatch*)b)->cod->Ptr);
}

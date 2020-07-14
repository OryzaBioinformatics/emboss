/* @source restover application
 **
 ** Reports restriction enzyme that produce specific overhangs
 ** @author: Copyright (C) Bernd Jagla
 ** @@ modified source from Alan
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




static void restover_printHits(const AjPSeq seq, const AjPStr seqcmp,
			       AjPFile outf, AjPList l,
			       const AjPStr name, ajint hits, ajint begin,
			       ajint end, AjBool ambiguity, ajint mincut,
			       ajint maxcut, AjBool plasmid, AjBool blunt,
			       AjBool sticky, ajint sitelen, AjBool limit,
			       const AjPTable table, AjBool alpha,
			       AjBool frags, AjBool nameit, AjBool html);
static void restover_read_equiv(AjPFile equfile, AjPTable table);
static void restover_read_file_of_enzyme_names(AjPStr *enzymes);




/* @prog restover *************************************************************
**
** Finds restriction enzymes that produce a specific overhang
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq     = NULL;
    AjPStr seqcmp  = NULL;
    AjPStr enzymes = NULL;
    AjPFile outf   = NULL;
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
    AjBool threeprime;
    AjBool commercial;
    AjBool nameit;
    AjBool html;
    AjBool limit;
    AjBool frags;
    AjPFile dfile;

    AjPFile enzfile = NULL;
    AjPFile equfile = NULL;

    AjPStr name = NULL;

    AjPTable table = NULL;

    ajint hits;

    AjPList l = NULL;

    embInit("restover", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    seqcmp    = ajAcdGetString("seqcomp");
    ajStrToUpper(&seqcmp);
    outf      = ajAcdGetOutfile("outfile");

    /*
    ** Some of these are not needed but I left them in case someone wants to
    ** use them some time ...
    */
    enzymes   = ajStrNewC("all");

    min        = ajAcdGetInt("min");
    max        = ajAcdGetInt("max");
    sitelen    = 2;
    threeprime = ajAcdGetBool("threeprime");
    blunt      = ajAcdGetBool("blunt");
    sticky     = ajAcdGetBool("sticky");
    single     = ajAcdGetBool("single");
    html       = ajAcdGetBool("html");
    alpha      = ajAcdGetBool("alphabetic");
    ambiguity  = ajAcdGetBool("ambiguity");
    plasmid    = ajAcdGetBool("plasmid");
    commercial = ajAcdGetBool("commercial");
    limit      = ajAcdGetBool("limit");
    frags      = ajAcdGetBool("fragments");
    nameit     = ajAcdGetBool("name");
    dfile      = ajAcdGetDatafile("datafile");

    if(single)
	max = min = 1;

    table = ajStrTableNew(EQUGUESS);
    l = ajListNew();

    if(threeprime)
	ajStrRev(&seqcmp);

    /* read the local file of enzymes names */
    restover_read_file_of_enzyme_names(&enzymes);

    if(!dfile)
    {
	ajFileDataNewC(ENZDATA,&enzfile);
	if(!enzfile)
	    ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");
    }
    else
    {
	enzfile = dfile;
    }



    if(limit)
    {
	ajFileDataNewC(EQUDATA,&equfile);
	if(!equfile)
	    limit=ajFalse;
	else
	{
	    restover_read_equiv(equfile,table);
	    ajFileClose(&equfile);
	}
    }



    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	ajFileSeek(enzfile,0L,0);
	ajSeqToUpper(seq);

	hits = embPatRestrictMatch(seq,begin,end,enzfile,enzymes,sitelen,
				   plasmid,ambiguity,min,max,blunt,sticky,
				   commercial,l);

	if(hits)
	{
	    name = ajStrNewC(ajSeqName(seq));
	    restover_printHits(seq, seqcmp, outf,l,name,hits,begin,end,
			       ambiguity,min,max,plasmid,blunt,sticky,
			       sitelen,limit,table,alpha,frags,nameit,
			       html);
	    ajStrDel(&name);
	}

	ajListFree(&l);
    }


    ajListDel(&l);
    ajSeqDel(&seq);
    ajFileClose(&enzfile);
    ajFileClose(&outf);

    ajExit();

    return 0;
}




/* @funcstatic restover_printHits *********************************************
**
** Print restover hits
**
** @param [r] seq [const AjPSeq] Sequence
** @param [r] seqcmp [const AjPStr] Undocumented
** @param [w] outf [AjPFile] outfile
** @param [u] l [AjPList] hits
** @param [r] name [const AjPStr] sequence name
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
** @param [r] table [const AjPTable] supplier table
** @param [r] alpha [AjBool] alphabetic sort
** @param [r] frags [AjBool] show fragment lengths
** @param [r] nameit [AjBool] show name
** @param [r] html [AjBool] show html
** @@
******************************************************************************/

static void restover_printHits(const AjPSeq seq, const AjPStr seqcmp,
			       AjPFile outf,
			       AjPList l, const AjPStr name, ajint hits,
			       ajint begin, ajint end, AjBool ambiguity,
			       ajint mincut, ajint maxcut, AjBool plasmid,
			       AjBool blunt, AjBool sticky, ajint sitelen,
			       AjBool limit, const AjPTable table,
			       AjBool alpha, AjBool frags,AjBool nameit,
			       AjBool html)
{
    EmbPMatMatch m = NULL;
    AjPStr ps = NULL;
    ajint *fa = NULL;
    ajint *fx = NULL;
    ajint fc = 0;
    ajint fn = 0;
    ajint fb = 0;
    ajint last = 0;
    AjPStr overhead = NULL;

    AjPStr value = NULL;

    ajint i;
    ajint c = 0;

    ps = ajStrNew();
    fn = 0;

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Restrict of %S from %d to %d\n",name,begin,end);

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"#\n");

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Minimum cuts per enzyme: %d\n",mincut);

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Maximum cuts per enzyme: %d\n",maxcut);

    if(html)
	ajFmtPrintF(outf,"<BR>");
    ajFmtPrintF(outf,"# Minimum length of recognition site: %d\n",
		sitelen);
    if(html)
	ajFmtPrintF(outf,"<BR>");

    hits = embPatRestrictRestrict(l,hits,!limit,alpha);

    if(frags)
    {
	fa = AJALLOC(hits*2*sizeof(ajint));
	fx = AJALLOC(hits*2*sizeof(ajint));
    }


    ajFmtPrintF(outf,"# Number of hits with any overlap: %d\n",hits);

    if(html)
	ajFmtPrintF(outf,"<BR>");

    if(html)
	ajFmtPrintF(outf,"</p><table  border cellpadding=4 "
		    "bgcolor=\"#FFFFF0\">\n");
    if(html)
	ajFmtPrintF(outf,
		    "<th>Base Number</th><th>Enzyme</th><th>Site</th>"
		    "<th>5'</th><th>3'</th><th>[5'</th><th>3']</th>\n");
    else
	ajFmtPrintF(outf,"# Base Number\tEnzyme\t\tSite\t\t5'\t3'\t"
		    "[5'\t3']\n");

    for(i=0;i<hits;++i)
    {
	ajListPop(l,(void **)&m);
	ajDebug("hit %d start:%d cut1:%d cut2:%d\n",
		i, m->start, m->cut1, m->cut2);


	if(!plasmid && (m->cut1-m->start>100 || m->cut2-m->start>100))
	{
	    embMatMatchDel(&m);
	    continue;
	}

	if(limit)
	{
	    value=ajTableGet(table,m->cod);
	    if(value)
		ajStrAssS(&m->cod,value);
	}

	if(m->cut2 >= m->cut1)
	    ajStrAssSub(&overhead, ajSeqStr( seq), m->cut1, m->cut2-1);
	else
	{
	    ajStrAssSub(&overhead, ajSeqStr( seq), m->cut2, m->cut1-1);
	    ajStrRev(&overhead);
	}

	ajDebug("overhead:%S seqcmp:%S\n", overhead, seqcmp);

	/* Print out only those who have the same overhang. */
	if(ajStrMatchCase(overhead, seqcmp))
	{
	    if(html)
	    {
		ajFmtPrintF(outf,
			    "<tr><td>%-d</td><td>%-16s</td><td>%-16s"
			    "</td><td>%d</td><td>%d</td></tr>\n",
			    m->start,ajStrStr(m->cod),ajStrStr(m->pat),
			    m->cut1,m->cut2);
	    }
	    else
		ajFmtPrintF(outf,"\t%-d\t%-16s%-16s%d\t%d\t\n",
			    m->start,ajStrStr(m->cod),ajStrStr(m->pat),
			    m->cut1,m->cut2);
	}

	if(frags)
	    fa[fn++] = m->cut1;

	if(m->cut3 && m->cut4)
	{
	    if(m->cut4 >= m->cut3)
		ajStrAssSub(&overhead, ajSeqStr( seq), m->cut3, m->cut4-1);
	    else
	    {
		ajStrAssSub(&overhead, ajSeqStr( seq), m->cut4, m->cut3-1);
		ajStrRev(&overhead);
	    }

	    if(ajStrMatchCase(overhead, seqcmp))
	    {
		if(html)
		    ajFmtPrintF(outf,
				"<tr><td>%-d</td><td>%-16s</td><td>%-16s"
				"</td><td></td><td></td><td>%d</td><td>%d"
				"</td></tr>\n",
				m->start,ajStrStr(m->cod),ajStrStr(m->pat),
				m->cut1,m->cut2);
		else
		    ajFmtPrintF(outf,"\t%-d\t%-16s%-16s\t\t%d\t%d\t\n",
				m->start,ajStrStr(m->cod),ajStrStr(m->pat),
				m->cut1,m->cut2);
	    }
	}

	/* I am not sure what fragments are doing so I left it in ...*/
	if(m->cut3 && m->cut4)
	{
	    if(frags)
		fa[fn++] = m->cut3;
	    /*	       ajFmtPrintF(*outf,"%d\t%d",m->cut3,m->cut4);*/
	}
	ajStrDel(&overhead);

	embMatMatchDel(&m);
    }



    if(frags)
    {
	ajSortIntInc(fa,fn);
	ajFmtPrintF(outf,"\n\nFragment lengths:\n");
	if(!fn || (fn==1 && plasmid))
	    ajFmtPrintF(outf,"    %d\n",end-begin+1);
	else
	{
	    last = -1;
	    fb = 0;
	    for(i=0;i<fn;++i)
	    {
		if((c=fa[i])!=last)
		    fa[fb++]=c;
		last = c;
	    }
	    fn = fb;
	    /* Calc lengths */

	    for(i=0;i<fn-1;++i)
		fx[fc++] = fa[i+1]-fa[i];
	    if(!plasmid)
	    {
		fx[fc++] = fa[0]-begin+1;
		fx[fc++] = end-fa[fn-1];
	    }
	    else
		fx[fc++] = (fa[0]-begin+1)+(end-fa[fn-1]);

	    ajSortIntDec(fx,fc);
	    for(i=0;i<fc;++i)
		ajFmtPrintF(outf,"    %d\n",fx[i]);
	}
	AJFREE(fa);
	AJFREE(fx);
    }


    ajStrDel(&ps);

    if(html)
	ajFmtPrintF(outf,"</table>\n");

    return;
}




/* @funcstatic restover_read_equiv ********************************************
**
** Load table with equivalent RE names
**
** @param [u] equfile [AjPFile] names
** @param [u] table [AjPTable] table to store names in
** @@
******************************************************************************/

static void restover_read_equiv(AjPFile equfile, AjPTable table)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    const char *p;

    line = ajStrNew();

    while(ajFileReadLine(equfile,&line))
    {
	p=ajStrStr(line);
	if(!*p || *p=='#' || *p=='!')
	    continue;
	p=ajSysStrtok(p," \t\n");
	key=ajStrNewC(p);
	p=ajSysStrtok(NULL," \t\n");
	value=ajStrNewC(p);
	ajTablePut(table,(const void *)key, (void *)value);
    }

    ajStrDel(&line);

    return;
}




/* @funcstatic restover_read_file_of_enzyme_names *****************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [w] enzymes [AjPStr*] enzymes to search for, can be passed as
**                             'all' or '@file'
** @return [void]
** @@
******************************************************************************/

static void restover_read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file = NULL;
    AjPStr line;
    const char *p = NULL;

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
	    if (!*p || *p == '#' || *p == '!')
		continue;

	    ajStrApp(enzymes, line);
	    ajStrAppC(enzymes, ",");
	}
	ajStrDel(&line);

	ajFileClose(&file);
    }

    return;
}

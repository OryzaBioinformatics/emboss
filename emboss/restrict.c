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


void printHits(AjPFile *outf, AjPList *l, AjPStr *name, ajint hits, ajint begin,
	       ajint end, AjBool ambiguity, ajint mincut, ajint maxcut, AjBool
	       plasmid, AjBool blunt, AjBool sticky, ajint sitelen,
	       AjBool limit, AjBool equiv, AjPTable table, AjBool alpha,
	       AjBool frags, AjBool nameit);
void read_equiv(AjPFile *equfile, AjPTable *table);
static void read_file_of_enzyme_names(AjPStr *enzymes);

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPStr    enzymes=NULL;
    AjPFile   outf=NULL;
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
    AjBool equiv;
    AjBool frags;
    AjPStr dfile;
    
    AjPFile   enzfile=NULL;
    AjPFile   equfile=NULL;
    
    AjPStr    name=NULL;

    AjPTable  table=NULL;
    
    ajint       hits;

    
    AjPList     l;

    embInit("restrict", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    enzymes   = ajAcdGetString("enzymes");
    outf      = ajAcdGetOutfile("outfile");
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
    equiv      = ajAcdGetBool("preferred");
    frags      = ajAcdGetBool("fragments");
    nameit     = ajAcdGetBool("name");
    dfile      = ajAcdGetString("datafile");
    
    if(single) max=min=1;
    
    table = ajStrTableNew(EQUGUESS);

    
/* read the local file of enzymes names */
    read_file_of_enzyme_names(&enzymes);
  
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
    
	

    if(equiv)
    {
	ajFileDataNewC(EQUDATA,&equfile);
	if(!equfile)
	    equiv=ajFalse;
	else
	    read_equiv(&equfile,&table);
    }
    

    
    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	ajFileSeek(enzfile,0L,0);

	hits = embPatRestrictMatch(seq,begin,end,enzfile,enzymes,sitelen,
				   plasmid,ambiguity,min,max,blunt,sticky,
				   commercial,&l);

	if(hits)
	{
	    name = ajStrNewC(ajSeqName(seq));
	    printHits(&outf,&l,&name,hits,begin,end,ambiguity,min,max,
		      plasmid,blunt,sticky,sitelen,limit,equiv,table,alpha,
		      frags,nameit);
	    ajStrDel(&name);
	}

	ajListFree(&l);
    }


    ajSeqDel(&seq);
    ajFileClose(&enzfile);
    ajFileClose(&outf);

    ajExit();
    return 0;
}






void printHits(AjPFile *outf, AjPList *l, AjPStr *name, ajint hits, ajint begin,
	       ajint end, AjBool ambiguity, ajint mincut, ajint maxcut, AjBool
	       plasmid, AjBool blunt, AjBool sticky, ajint sitelen,
	       AjBool limit, AjBool equiv, AjPTable table, AjBool alpha,
	       AjBool frags,AjBool nameit)
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


	if(equiv && limit)
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
	ajSortIntDec(fa,fn);
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
	    ajSortIntInc(fx,fc);
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



void read_equiv(AjPFile *equfile, AjPTable *table)
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
}

/* @funcstatic read_file_of_enzyme_names *************************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**  
** @param [r] enzymes [AjPStr*] enzymes to search for or 'all' or '@file'
** @return [void]
** @@
******************************************************************************/
      
static void read_file_of_enzyme_names(AjPStr *enzymes)
{
    AjPFile file=NULL;
    AjPStr line;
    char   *p=NULL;

    if (ajStrFindC(*enzymes, "@") == 0)
    {
	ajStrTrimC(enzymes, "@");	/* remove the @ */
	file = ajFileNewIn(*enzymes);
	if (file == NULL)
	    ajDie("Cannot open the file of enzyme names: '%S'", enzymes);

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

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


void printHits(AjPFile *outf, AjPList *l, AjPStr *name, int hits, int begin,
	       int end, AjBool ambiguity, int mincut, int maxcut, AjBool
	       plasmid, AjBool blunt, AjBool sticky, int sitelen,
	       AjBool limit, AjBool equiv, AjPTable table, AjBool alpha);
void read_equiv(AjPFile *equfile, AjPTable *table);
static void read_file_of_enzyme_names(AjPStr *enzymes);

int main( int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPStr    enzymes=NULL;
    AjPFile   outf=NULL;
    int begin;
    int end;
    int min;
    int max;
    int sitelen;
    AjBool alpha;
    AjBool single;
    AjBool blunt;
    AjBool sticky;
    AjBool ambiguity;
    AjBool plasmid;
    AjBool commercial;
    AjBool limit;
    AjBool equiv;
    AjPStr dfile;
    
    AjPFile   enzfile=NULL;
    AjPFile   equfile=NULL;
    
    AjPStr    name=NULL;

    AjPTable  table=NULL;
    
    int       hits;

    
    /*    EmbPPatRestrict enz;*/
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
    

    
    /*    enz = embPatRestrictNew(); NOT USED ?? */
    

    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	ajFileSeek(enzfile,0L,0);
	ajUser("\nScanning %s...",ajSeqName(seq));

	hits = embPatRestrictMatch(seq,begin,end,enzfile,enzymes,sitelen,
				   plasmid,ambiguity,min,max,blunt,sticky,
				   commercial,&l);

	if(hits)
	{
	    name = ajStrNewC(ajSeqName(seq));
	    printHits(&outf,&l,&name,hits,begin,end,ambiguity,min,max,
		      plasmid,blunt,sticky,sitelen,limit,equiv,table,alpha);
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






void printHits(AjPFile *outf, AjPList *l, AjPStr *name, int hits, int begin,
	       int end, AjBool ambiguity, int mincut, int maxcut, AjBool
	       plasmid, AjBool blunt, AjBool sticky, int sitelen,
	       AjBool limit, AjBool equiv, AjPTable table, AjBool alpha)
{
    EmbPMatMatch m=NULL;
    AjPStr  ps=NULL;

    AjPStr value=NULL;

    int i;

    ps=ajStrNew();
    
    ajFmtPrintF(*outf,"# Restrict of %s from %d to %d\n#\n",
		ajStrStr(*name),begin,end);
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
	if(m->cut3 && m->cut4)
	    ajFmtPrintF(*outf,"%d\t%d",m->cut3,m->cut4);
	ajFmtPrintF(*outf,"\n");

	embMatMatchDel(&m);
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
** @param [r] enzymes [AjPStr*] names of enzymes to search for or 'all' or '@file'
** @return [void]
** @@
******************************************************************************/
      
static void read_file_of_enzyme_names(AjPStr *enzymes) {

  AjPFile file=NULL;
  AjPStr line;
  char   *p=NULL;

  if (ajStrFindC(*enzymes, "@") == 0) {
    ajStrTrimC(enzymes, "@");   /* remove the @ */
    file = ajFileNewIn(*enzymes);
    if (file == NULL) {
      ajDie("Cannot open the file of enzyme names: '%S'", enzymes);
    }
/* blank off the enzyme file name and replace with the enzyme names */
    ajStrClear(enzymes);
    line = ajStrNew();
    while(ajFileReadLine(file, &line)) {
      p = ajStrStr(line);
      if (!*p || *p == '#' || *p == '!') continue;
      ajStrApp(enzymes, line);
      ajStrAppC(enzymes, ",");
    }
    ajStrDel(&line);

    ajFileClose(&file);
  }

}


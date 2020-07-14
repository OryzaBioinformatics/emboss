/* @source tfscan application
**
** Finds transcription factors
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** 12/03/2000: (AJB) Added accession numbers, end points and matching sequence
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
#include "string.h"




static void tfscan_print_hits(const AjPStr name, AjPList *l, ajint hits,
			      AjPFile outf, ajint begin, ajint end,
			      const AjPTable t, const AjPSeq seq,
			      ajint minlength,
			      const AjPTable btable);




/* @prog tfscan ***************************************************************
**
** Scans DNA sequences for transcription factors
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq   = NULL;
    AjPFile outf = NULL;
    AjPFile inf  = NULL;

    ajint begin;
    ajint end;

    AjPList l = NULL;

    AjPStr strand = NULL;
    AjPStr substr = NULL;
    AjPStr line   = NULL;
    AjPStr name   = NULL;
    AjPStr acc    = NULL;
    AjPStr bf     = NULL;
    AjPStr *menu;
    AjPStr pattern  = NULL;
    AjPStr opattern = NULL;
    AjPStr pname    = NULL;
    AjPStr key      = NULL;
    AjPStr value    = NULL;
    AjPTable atable = NULL;
    AjPTable btable = NULL;
    
    ajint mismatch;
    ajint minlength;
    
    ajint sum;
    ajint v;

    const char *p;


    embInit("tfscan", argc, argv);

    seqall     = ajAcdGetSeqall("sequence");
    outf       = ajAcdGetOutfile("outfile");
    mismatch   = ajAcdGetInt("mismatch");
    minlength  = ajAcdGetInt("minlength");
    menu       = ajAcdGetList("menu");

    pname = ajStrNew();
    p=ajStrStr(*menu);

    if(*p=='F')
	ajStrAssC(&pname,"tffungi");
    else if(*p=='I')
	ajStrAssC(&pname,"tfinsect");
    else if(*p=='O')
	ajStrAssC(&pname,"tfother");
    else if(*p=='P')
	ajStrAssC(&pname,"tfplant");
    else if(*p=='V')
	ajStrAssC(&pname,"tfvertebrate");
    else if(*p=='C')
	inf = ajAcdGetDatafile("custom");

    if(*p!='C')
    {
	ajFileDataNew(pname,&inf);
	if(!inf)
	    ajFatal("Either EMBOSS_DATA undefined or TFEXTRACT needs running");
    }

    name     = ajStrNew();
    acc      = ajStrNew();
    bf       = ajStrNewC("");
    substr   = ajStrNew();
    line     = ajStrNew();
    pattern  = ajStrNewC("AA");
    opattern = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	ajStrAssC(&name,ajSeqName(seq));
	strand=ajSeqStrCopy(seq);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
	ajStrToUpper(&substr);

	l=ajListNew();
	atable = ajStrTableNew(1000);
	btable = ajStrTableNew(1000);
	
	sum=0;
	while(ajFileReadLine(inf,&line))
	{
	    p = ajStrStr(line);

	    if(!*p || *p=='#' || *p=='\n' || *p=='!')
		continue;

	    ajFmtScanS(line,"%S%S%S",&pname,&pattern,&acc);
	    p += ajStrLen(pname);
	    while(*p && *p==' ')
		++p;
	    p += ajStrLen(pattern);
	    while(*p && *p==' ')
		++p;
	    p += ajStrLen(acc);
	    while(*p && *p==' ')
		++p;

	    ajStrAssS(&opattern,pattern);
	    ajStrAssC(&bf,p);
	    
	    v = embPatVariablePattern(pattern,substr,pname,l,0,
				      mismatch,begin);
	    if(v)
	    {
		key = ajStrNewC(ajStrStr(pname));
		value = ajStrNewC(ajStrStr(acc));
		ajTablePut(atable,(const void *)key,(void *)value);
		key = ajStrNewC(ajStrStr(pname));
		value = ajStrNewC(ajStrStr(bf));
		ajTablePut(btable,(const void *)key,(void *)value);
	    }
	    sum += v;
	}

	if(sum)
	    tfscan_print_hits(name,&l,sum,outf,begin,end,atable,seq,minlength,
			      btable);

	ajFileSeek(inf,0L,0);
	ajListDel(&l);
	ajStrTableFree(&atable);
	ajStrTableFree(&btable);
	ajStrDel(&strand);
    }

    ajStrDel(&line);
    ajStrDel(&name);
    ajStrDel(&acc);
    ajStrDel(&pname);
    ajStrDel(&pattern);
    ajStrDel(&substr);
    ajSeqDel(&seq);
    ajFileClose(&inf);
    ajFileClose(&outf);

    ajExit();

    return 0;
}




/* @funcstatic tfscan_print_hits **********************************************
**
** Print matches to transcription factor sites
**
** @param [r] name [const AjPStr] name of test sequence
** @param [w] l [AjPList*] list of hits
** @param [r] hits [ajint] number of hits
** @param [u] outf [AjPFile] output file
** @param [r] begin [ajint] sequence start
** @param [r] end [ajint] sequence end
** @param [r] t [const AjPTable] table of accession numbers
** @param [r] seq [const AjPSeq] test sequence
** @param [r] minlength [ajint] minimum length of pattern
** @param [r] btable [const AjPTable] BF lines from transfac (if any)
** @@
******************************************************************************/

static void tfscan_print_hits(const AjPStr name, AjPList *l,
			      ajint hits, AjPFile outf,
			      ajint begin, ajint end, const AjPTable t,
			      const AjPSeq seq, ajint minlength,
			      const  AjPTable btable)
{
    ajint i;
    EmbPMatMatch m;
    AjPStr acc = NULL;
    AjPStr s   = NULL;
    AjPStr bf  = NULL;
    AjPStr lastnam = NULL;
    
    s       = ajStrNew();
    lastnam = ajStrNewC("");

    ajFmtPrintF(outf,"TFSCAN of %s from %d to %d\n\n",ajStrStr(name),
		begin,end);

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	acc = ajTableGet(t,(const void *)m->seqname);



	if((ajStrCmpO(m->seqname,lastnam)) && ajStrLen(lastnam))
	{
	    bf  = ajTableGet(btable,(const void *)lastnam);
	    if(ajStrLen(bf))
		ajFmtPrintF(outf,"                     %S\n",bf);
	}
	
	ajStrAssS(&lastnam,m->seqname);

	ajStrAssSubC(&s,ajSeqChar(seq),m->start-1,m->start+m->len-2);

	if(ajStrLen(s) >= minlength)
	    ajFmtPrintF(outf,"%-20s %-8s %-5d %-5d %s\n",ajStrStr(m->seqname),
			ajStrStr(acc),m->start,
			m->start+m->len-1,ajStrStr(s));

	embMatMatchDel(&m);
    }

    ajStrDel(&s);
    ajStrDel(&lastnam);
    
    return;
}

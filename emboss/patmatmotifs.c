/* @source patmatmotifs.c
** @author: Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
** @@
** Application for pattern matching, a sequence against a database of motifs.
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




/* @prog patmatmotifs *********************************************************
**
** Search a PROSITE motif database with a protein sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPFile inf	 = NULL;
    AjPFile inf2 = NULL;
    AjPFeattable tab = NULL;
    AjPReport report = NULL;

    AjPSeq sequence = NULL;

    AjPStr redatanew = NULL;
    AjPStr str	     = NULL;
    AjPStr regexp    = NULL;
    AjPStr temp	     = NULL;
    AjPStr text      = NULL;
    AjPStr docdata   = NULL;
    AjPStr data      = NULL;
    AjPStr accession = NULL;
    AjPStr name      = NULL;
    EmbPPatMatch match = NULL;
    AjPStr savereg = NULL;
    AjPStr fthit   = NULL;

    AjBool full;
    AjBool prune;

    ajint i;
    ajint number;
    ajint start;
    ajint end;
    ajint length;
    ajint zstart;
    ajint zend;
    const char *p;
    ajint seqlength;
    AjPStr tmpstr  = NULL;
    AjPStr tailstr = NULL;
    AjPFeature gf;

    embInit("patmatmotifs", argc, argv);

    ajStrAssC(&fthit, "hit");

    savereg   = ajStrNew();
    str       = ajStrNew();
    regexp    = ajStrNew();
    temp      = ajStrNew();
    data      = ajStrNew();
    accession = ajStrNew();
    text      = ajStrNew();
    name      = ajStrNew();

    sequence = ajAcdGetSeq("sequence");
    report   = ajAcdGetReport("outfile");
    full     = ajAcdGetBool("full");
    prune    = ajAcdGetBool("prune");

    tab = ajFeattableNewSeq(sequence);
    ajStrAssC(&tailstr, "");

    seqlength = ajStrLen(str);
    str       = ajSeqStrCopy(sequence);

    redatanew = ajStrNewC("PROSITE/prosite.lines");
    docdata   = ajStrNewC("PROSITE/");

    ajFileDataNew(redatanew, &inf);
    if(!inf)
	ajFatal("Either EMBOSS_DATA undefined or PROSEXTRACT needs running");

    ajFmtPrintAppS(&tmpstr, "Full: %B\n", full);
    ajFmtPrintAppS(&tmpstr, "Prune: %B\n", prune);
    ajFmtPrintAppS(&tmpstr, "Data_file: %F\n", inf);
    ajReportSetHeader(report, tmpstr);

    while(ajFileReadLine(inf, &regexp))
    {
	p=ajStrStr(regexp);
	if(*p && *p!=' ' && *p!='^')
	{
	    p=ajSysStrtok(p," ");
	    ajStrAssC(&name,p);
	    if(prune)
		if(ajStrMatchCaseC(name,"myristyl") ||
		   ajStrMatchCaseC(name,"asn_glycosylation") ||
		   ajStrMatchCaseC(name,"camp_phospho_site") ||
		   ajStrMatchCaseC(name,"pkc_phospho_site") ||
		   ajStrMatchCaseC(name,"ck2_phospho_site") ||
		   ajStrMatchCaseC(name,"tyr_phospho_site"))
		{
		    for(i=0;i<4;++i)
			ajFileReadLine(inf, &regexp);
		    continue;
		}
	    p=ajSysStrtok(NULL," ");
	    ajStrAssC(&accession,p);
	}

	if(ajStrPrefixC(regexp, "^"))
	{
	    p = ajStrStr(regexp);

	    ajStrAssC(&temp,p+1);
	    ajStrAssC(&savereg,p+1);

	    match = embPatMatchFind(temp, str, ajFalse, ajFalse);
	    number = embPatMatchGetNumber(match);

	    for(i=0; i<number; i++)
	    {
		seqlength = ajStrLen(str);

		start = 1+embPatMatchGetStart(match, i);

		end = 1+embPatMatchGetEnd(match, i);

		length = embPatMatchGetLen(match, i);

		gf = ajFeatNew(tab, NULL, fthit, start, end,
			       (float) length, ' ', 0);

		ajFmtPrintS(&tmpstr, "*motif %S", name);
		ajFeatTagAdd(gf, NULL, tmpstr);

		if(start-5<0)
		    zstart = 0;
		else
		    zstart = start-5;

		if(end+5> seqlength)
		    zend = end;
		else
		    zend = end+5;


		ajStrAssSub(&temp, str, zstart, zend);
	    }


	    if(full && number)
	    {
		ajStrAssC(&redatanew,ajStrStr(docdata));
		ajStrAppC(&redatanew,ajStrStr(accession));
		ajFileDataNew(redatanew, &inf2);
		if(!inf2)
		    continue;

		/*
		** Insert Prosite documentation from files made by
		** prosextract.c
		*/
		ajFmtPrintAppS(&tailstr, "Motif: %S\n", name);
		ajFmtPrintAppS(&tailstr, "Count: %d\n\n", number);
		while(ajFileReadLine(inf2, &text))
		    ajFmtPrintAppS(&tailstr, "%S\n", text);

		ajFmtPrintAppS(&tailstr, "\n***************\n\n");
		ajFileClose(&inf2);

	    }
	    embPatMatchDel(&match);
	}
    }

    ajReportSetTail(report,tailstr);
    ajReportWrite(report, tab, sequence);

    ajStrDel(&temp);
    ajStrDel(&regexp);
    ajStrDel(&savereg);
    ajStrDel(&str);
    ajStrDel(&data);
    ajStrDel(&docdata);
    ajStrDel(&text);
    ajStrDel(&redatanew);
    ajStrDel(&accession);
    ajSeqDel(&sequence);

    ajFeattableDel(&tab);
    ajFileClose(&inf);

    ajExit();

    return 0;
}

/* @source redata application
**
** Reports isoschizomers, references and suppliers for restriction enzymes
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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
#define REFDATA "REBASE/embossre.ref"
#define SUPDATA "REBASE/embossre.sup"

#define SUPPGUESS 50	/* Estimate of number of suppliers. */


static AjPTable redata_supply_table(AjPFile inf);







/* @prog redata ***************************************************************
**
** Search REBASE for enzyme name, references, suppliers etc
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr    enzyme=NULL;

    AjPFile   outf=NULL;

    AjPFile   enzfile=NULL;
    AjPFile   reffile=NULL;
    AjPFile   supfile=NULL;

    AjBool isoschizomers;
    AjBool references;
    AjBool suppliers;

    AjPTable t;
    AjPStr key   = NULL;
    AjPStr value = NULL;

    AjPStr line    = NULL;
    AjPStr enzline = NULL;

    char   *p;
    char   *q;
    AjPStr str;
    AjPStr iso;

    AjPStr    *ea;
    ajint       ne=0;

    ajint len;
    ajint ncuts;
    AjBool blunt;
    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;

    ajint i;
    ajint n;

    embInit("redata", argc, argv);

    ajFileDataNewC(ENZDATA,&enzfile);
    ajFileDataNewC(REFDATA,&reffile);
    ajFileDataNewC(SUPDATA,&supfile);
    if(!enzfile || !reffile || !supfile)
	ajFatal("EMBOSS_DATA undefined or REBASEEXTRACT needs running");




    enzyme        = ajAcdGetString("enzyme");
    outf          = ajAcdGetOutfile("outfile");
    isoschizomers = ajAcdGetBool("isoschizomers");
    references    = ajAcdGetBool("references");
    suppliers     = ajAcdGetBool("suppliers");


    ajStrCleanWhite(&enzyme);

    line   = ajStrNew();
    enzline=ajStrNew();
    str    =ajStrNew();
    key    =ajStrNewC(".");
    iso    =ajStrNew();



    /* Read in and close supplier file for later use */
    t=redata_supply_table(supfile);
    ajFileClose(&supfile);

    /* Read the enzyme line */
    while(ajFileReadLine(enzfile,&enzline))
    {
	p=ajStrStr(enzline);
	if(*p=='#' || *p=='\n' || *p=='!')
	    continue;
	p=strtok(p," \t\n");
	ajStrAssC(&str,p);
	while(*p) ++p;
	*p=' ';
	if(ajStrMatchCase(str,enzyme)) break;
    }

    /* Only do the rest if a matching enzyme was found */
    if(ajStrMatchCase(str,enzyme))
    {
	ajFmtPrintF(outf,"%s\n\n",ajStrStr(str));
	while(ajStrMatchCase(str,enzyme))
	{
	    p=ajStrStr(enzline);
	    p=strtok(p," \t\n");
	    ajStrAssC(&str,p);
	    p=strtok(NULL," \t\n");
	    ajStrAssC(&line,p);
	    while(*p) ++p;
	    ++p;
	    sscanf(p,"%d%d",&len,&ncuts);
	    if(ncuts==2)
		sscanf(p,"%d%d%d%d%d",&len,&ncuts,&blunt,&cut1,&cut2);
	    else
		sscanf(p,"%d%d%d%d%d%d%d",&len,&ncuts,&blunt,&cut1,&cut2,
		       &cut3,&cut4);
	    ajStrToUpper(&line);
	    ajFmtPrintF(outf,"Recognition site is %s leaving ",
			ajStrStr(line));
	    if(blunt)
		ajFmtPrintF(outf,"blunt ends\n");
	    else
		ajFmtPrintF(outf,"sticky ends\n");
	    if(ncuts==2)
		ajFmtPrintF(outf,"  Cut positions 5':%d 3':%d\n",cut1,cut2);
	    else
		ajFmtPrintF(outf,"  Cut positions 5':%d 3':%d [5':%d 3':%d]\n",
			    cut1,cut2,cut3,cut4);

	    if(!ajFileReadLine(enzfile,&enzline)) break;
	    p=ajStrStr(enzline);
	    p=strtok(p," \t\n");
	    ajStrAssC(&str,p);
	    while(*p) ++p;
	    *p=' ';
	}

	/* Read the reference file */
	while(ajFileReadLine(reffile,&line))
	{
	    p=ajStrStr(line);
	    if(*p=='#' || *p=='\n' || *p=='!')
		continue;
	    if(ajStrMatchCase(line,enzyme)) break;
	    while(!ajStrMatchC(line,"//"))
		ajFileReadLine(reffile,&line);
	}

	ajFileReadLine(reffile,&line);
	ajFmtPrintF(outf,"Organism: %s\n",ajStrStr(line));
	ajFileReadLine(reffile,&iso);
	if(ajStrLen(iso))
	    ne = ajArrCommaList(iso,&ea);
	ajFileReadLine(reffile,&line);
	if(ajStrLen(line))
	    ajFmtPrintF(outf,"Methylated: %s\n",ajStrStr(line));
	ajFileReadLine(reffile,&line);
	if(ajStrLen(line))
	    ajFmtPrintF(outf,"Source: %s\n",ajStrStr(line));
	if(isoschizomers && ajStrLen(iso))
	{
	    ajFmtPrintF(outf,"\nIsoschizomers:\n");
	    n=0;
	    ajFmtPrintF(outf,"   ");
	    for(i=0;i<ne;++i)
	    {
		ajFmtPrintF(outf,"%-12s",ajStrStr(ea[i]));
		if(++n==6)
		{
		    ajFmtPrintF(outf,"\n   ");
		    n=0;
		}
	    }
	    ajFmtPrintF(outf,"\n");
	}
	ajFileReadLine(reffile,&line);
	if(suppliers && ajStrLen(line))
	{
	    ajFmtPrintF(outf,"\nSuppliers:\n");
	    p=ajStrStr(line);
	    q=ajStrStr(key);

	    while(*p)
	    {
		*q=*p;
		value=ajTableGet(t,key);
		ajFmtPrintF(outf,"%s\n",ajStrStr(value));
		++p;
	    }
	}
	ajFileReadLine(reffile,&line);
	if(references && ajStrLen(line))
	{
	    p=ajStrStr(line);
	    sscanf(p,"%d",&n);
	    ajFmtPrintF(outf,"\nReferences:\n");
	    for(i=0;i<n;++i)
	    {
		ajFileReadLine(reffile,&line);
		ajFmtPrintF(outf,"%s\n",ajStrStr(line));
	    }
	}

	if(ajStrLen(iso))
	{
	    for(i=0;i<ne;++i)
	        ajStrDel(&ea[i]);
	    AJFREE(ea);
        }
    }
    else
	ajFmtPrintF(outf,"Restriction enzyme %s not found\n",ajStrStr(enzyme));



    ajStrDel(&str);
    ajStrDel(&iso);
    ajStrDel(&enzyme);
    ajStrDel(&line);
    ajStrDel(&enzline);
    ajStrDel(&key);
    ajStrTableFree(&t);
    ajFileClose(&enzfile);
    ajFileClose(&reffile);
    ajFileClose(&outf);

    ajExit();
    return 0;
}


/* @funcstatic redata_supply_table ********************************************
**
** Read list of RE suppliers into table
**
** @param [r] inf [AjPFile] infile
** @return [AjPTable] Undocumented
** @@
******************************************************************************/


static AjPTable redata_supply_table(AjPFile inf)
{
    AjPTable t;

    AjPStr   line;

    AjPStr   key;
    AjPStr   value;

    char     *p;
    char     *q;
    char     c;

    t = ajStrTableNew(SUPPGUESS);
    line = ajStrNew();

    while(ajFileReadLine(inf,&line))
    {
	p=ajStrStr(line);
	q=p;
	if(!*p || *p=='#' || *p=='\n' || *p=='!')
	    continue;
	p=ajSysStrtok(p," \t\n");
	key = ajStrNewC(p);
	q=strstr(q,p);
	while((c=*q)!=' ' && c!='\t' && c!='\n' && c)
	    ++q;
	while((c=*q)==' ' || c=='\t' || c=='\n')
	    ++q;
	value=ajStrNewC(q);
	ajTablePut(t,(const void *)key, (void *)value);
    }

    ajStrDel(&line);
    return t;
}

/* @source tfextract application
**
** Extracts pattern information from TRANSFAC site.dat file
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




/* @prog tfextract ************************************************************
**
** Extract data from TRANSFAC
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf  = NULL;
    AjPFile fout = NULL;
    AjPFile iout = NULL;
    AjPFile vout = NULL;
    AjPFile pout = NULL;
    AjPFile oout = NULL;
    AjPFile fp   = NULL;

    AjPStr line;
    AjPStr acc;
    AjPStr id;
    AjPStr bf;
    AjPStr pattern;
    AjPStr pfname;

    const char *p;
    char *q;

    AjBool gid=ajFalse;
    AjBool done=ajFalse;

    embInit("tfextract",argc,argv);

    inf = ajAcdGetInfile("infile");

    pfname = ajStrNewC("tffungi");
    ajFileDataNewWrite(pfname,&fout);

    ajStrAssC(&pfname,"tfinsect");
    ajFileDataNewWrite(pfname,&iout);

    ajStrAssC(&pfname,"tfvertebrate");
    ajFileDataNewWrite(pfname,&vout);

    ajStrAssC(&pfname,"tfplant");
    ajFileDataNewWrite(pfname,&pout);

    ajStrAssC(&pfname,"tfother");
    ajFileDataNewWrite(pfname,&oout);

    ajStrDel(&pfname);

    line    = ajStrNew();
    id      = ajStrNewC("");
    bf      = ajStrNew();
    acc     = ajStrNew();
    pattern = ajStrNew();


    while(ajFileReadLine(inf,&line))
    {
	p = ajStrStr(line);
	
	if(ajStrPrefixC(line,"ID"))
	{
	    gid  = ajTrue;
	    done = ajFalse;
	    fp = oout;
	    p = ajSysStrtok(p," \t\n");
	    p = ajSysStrtok(NULL," \t\n");
	    ajStrAssC(&id,p);
	}

	if(ajStrPrefixC(line,"AC"))
	{
	    p = ajSysStrtok(p," \t\n");
	    p = ajSysStrtok(NULL," \t\n");
	    ajStrAssC(&acc,p);
	}


	if(ajStrPrefixC(line,"BF"))
	{
	    p = strpbrk(p," \t\n");
	    while(*p && *p==' ')
		++p;
	    ajStrAssC(&bf,p);
	}

	if(ajStrPrefixC(line,"SQ") || ajStrPrefixC(line,"SE"))
	{
	    p = ajSysStrtok(p," .\t\n");
	    p = ajSysStrtok(NULL," .\t\n");
	    if(!p)
		ajStrAssC(&pattern,"");
	    else
		ajStrAssC(&pattern,p);
	    q = ajStrStrMod(&pattern);

	    while(*q)
	    {
		if(*q=='[')
		    *q = '(';

		if(*q==':')
		    *q = ',';

		if(*q==']')
		    *q = ')';
		++q;
	    }
	}

	if(ajStrPrefixC(line,"OC") && !done)
	{
	    if(strstr(p,"Fungi"))
	    {
		done = ajTrue;
		fp = fout;
	    }
	    else if(strstr(p,"saccharomycetaceae"))
	    {
		done = ajTrue;
		fp = fout;
	    }
	    else if(strstr(p,"arthropoda"))
	    {
		done = ajTrue;
		fp = iout;
	    }
	    else if(strstr(p,"vertebrata"))
	    {
		done=ajTrue;
		fp = vout;
	    }
	    else if(strstr(p,"plantae"))
		fp = pout;
	}

	if(ajStrPrefixC(line,"//") && ajStrLen(pattern) && gid)
	{
	    if(!ajStrLen(bf))
		ajFmtPrintF(fp,"%-20s %S %S\n",ajStrStr(id),pattern,acc);
	    else
	    {
		ajFmtPrintF(fp,"%-20s %S %S %S\n",ajStrStr(id),pattern,acc,bf);
		ajStrAssC(&bf,"");
	    }
	}
    }


    ajFileClose(&inf);
    ajFileClose(&fout);
    ajFileClose(&iout);
    ajFileClose(&vout);
    ajFileClose(&pout);
    ajFileClose(&oout);

    ajStrDel(&line);
    ajStrDel(&id);
    ajStrDel(&bf);
    ajStrDel(&acc);
    ajStrDel(&pattern);


    ajExit();

    return 0;
}

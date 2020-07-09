/* @source scope application
**
** Convert modified scop flatfile to EMBOSS scop format
**
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


int main(int argc, char**argv)
{
    AjPRegexp rexp   = NULL;
    AjPRegexp exp2   = NULL;
    AjPStr line      = NULL;
    AjPStr str       = NULL;
    AjPStr entry     = NULL;
    AjPStr pdb       = NULL;
    AjPStr chains    = NULL;
    AjPStr numbers   = NULL;
    AjPStr text      = NULL;
    AjPStr token     = NULL;
    
    AjPStrTok handle  = NULL;
    AjPStrTok bhandle = NULL;
    
    AjPFile inf  = NULL;
    AjPFile outf = NULL;

    char *p=NULL;
    char *q=NULL;
    char c=' ';
    
    int n;
    int i;
    int from;
    int to;
    
    AjPScop scop=NULL;
    
    
    rexp  = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+([^ \t\r\n]+)[ \t\r\n]+"
		       "([^ \t\r\n]+)[ \t\n\r]+([^ \t\n\r]+)[ \t\n\r]+");
    exp2  = ajRegCompC("^([0-9]+)([A-Za-z]+)[-]([0-9]+)");
    
    
    embInit("scope", argc, argv);
    
    inf  = ajAcdGetInfile("infile");
    outf = ajAcdGetOutfile("outfile");
    
    line    = ajStrNew();
    str     = ajStrNew();
    entry   = ajStrNew();
    pdb     = ajStrNew();
    chains  = ajStrNew();
    numbers = ajStrNew();
    text    = ajStrNew();
    token   = ajStrNew();
    
    
    while(ajFileReadLine(inf,&line))
    {
	if(!ajRegExec(rexp,line))
	{
	    ajWarn("Bad line: %S\n",line);
	    continue;
	}

	ajRegSubI(rexp,1,&entry);
	ajRegSubI(rexp,2,&pdb);
	ajRegSubI(rexp,3,&chains);
	ajRegSubI(rexp,4,&numbers);
	ajRegPost(rexp,&text);

	if(ajStrPrefixC(chains,"-"))
	    n = 0;
	else
	    n = ajStrTokenCount(&chains,",");

	scop = ajScopNew(n);

	ajStrTrimC(&entry,"_");
	ajStrToUpper(&entry);
	ajStrAssS(&scop->Entry,entry);
	ajStrToUpper(&pdb);
	ajStrAssS(&scop->Pdb,pdb);

	handle = ajStrTokenInit(chains,",");
	for(i=0;i<n;++i)
	{
	    ajStrToken(&token,&handle,NULL);
	    p = ajStrStr(token);
	    if(sscanf(p,"%d-%d",&from,&to)==2)
	    {
		scop->Chain[i]='.';
		ajFmtPrintS(&scop->Start[i],"%d",from);
		ajFmtPrintS(&scop->End[i],"%d",to);
	    }
	    else if(sscanf(p,"%c:%d-%d",&c,&from,&to)==3)
	    {
		ajFmtPrintS(&scop->Start[i],"%d",from);
		ajFmtPrintS(&scop->End[i],"%d",to);
		scop->Chain[i]=c;
	    }
	    else if(ajStrChar(token,1)==':')
	    {
		ajStrAssC(&scop->Start[i],".");
		ajStrAssC(&scop->End[i],".");
		scop->Chain[i]=*ajStrStr(token);
	    }
	    else if(ajRegExec(exp2,token))
	    {
		ajRegSubI(exp2,1,&str);
		ajStrAss(&scop->Start[i],str);
		ajRegSubI(exp2,2,&str);
		scop->Chain[i] = *ajStrStr(str);
		ajRegSubI(exp2,3,&str);
		ajStrAss(&scop->End[i],str);
	    }
	    else
		ajFatal("Unparseable chain line [%S]\n",chains);
	}
	ajStrTokenClear(&handle);

	ajStrAssC(&scop->Source,"Unknown");	
	bhandle = ajStrTokenInit(text,"|\n");
	while(ajStrToken(&token,&bhandle,NULL))
	{
	    ajStrRemoveHtml(&token);
	    ajStrClean(&token);
	    p = ajStrStr(token);
	    if(ajStrPrefixC(token,"Class:"))
		ajStrAssC(&scop->Class,p+7);
	    if(ajStrPrefixC(token,"Fold:"))
		ajStrAssC(&scop->Fold,p+6);
	    if(ajStrPrefixC(token,"Superfamily:"))
		ajStrAssC(&scop->Superfamily,p+13);
	    if(ajStrPrefixC(token,"Family:"))
		ajStrAssC(&scop->Family,p+8);
	    if(ajStrPrefixC(token,"Protein:"))
		ajStrAssC(&scop->Domain,p+9);
	    if(ajStrPrefixC(token,"Species:"))
	    {
		ajStrAssC(&scop->Source,p+9);
	    }
	}
	ajStrTokenClear(&bhandle);
	
	ajScopWrite(outf,scop);
	ajScopDel(&scop);
    }
    

    ajRegFree(&rexp);


    ajStrDel(&line);
    ajStrDel(&str);
    ajStrDel(&entry);
    ajStrDel(&pdb);
    ajStrDel(&chains);
    ajStrDel(&numbers);
    ajStrDel(&text);
    ajStrDel(&token);


    ajFileClose(&inf);
    ajFileClose(&outf);

    ajExit();
    return 0;

}

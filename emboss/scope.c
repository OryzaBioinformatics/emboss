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
    AjPRegexp t1exp  = NULL;
    AjPRegexp t2exp  = NULL;
    AjPStr line      = NULL;
    AjPStr str       = NULL;
    AjPStr entry     = NULL;
    AjPStr pdb       = NULL;
    AjPStr chains    = NULL;
    AjPStr numbers   = NULL;
    AjPStr text      = NULL;
    AjPStr token     = NULL;
    
    AjPStrTok handle = NULL;

    AjPFile inf  = NULL;
    AjPFile outf = NULL;

    char *p=NULL;
    char c=' ';
    
    int n;
    int i;
    int from;
    int to;
    
    AjPScop scop=NULL;
    
    
    rexp  = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+([^ \t\r\n]+)[ \t\r\n]+"
		       "([^ \t\r\n]+)[ \t\n\r]+([^ \t\n\r]+)[ \t\n\r]+");
    exp2  = ajRegCompC("^([0-9]+)([A-Za-z]+)[-]([0-9]+)");
    t1exp = ajRegCompC("^([^:]+)[:]([^:]+)[:]([^:]+)[:]([^:]+)[:]([^:]+)[:]"
		       "([^:]+)[:]([^:]+)[:]");
    t2exp = ajRegCompC("^([^:]+)[:]([^:]+)[:]([^:]+)[:]([^:]+)[:]([^:]+)[:]"
		       "([^:]+)[:]");
    
    
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
		ajStrAssC(&scop->Chain[i],"Unnamed");
		scop->Start[i] = from;
		scop->End[i]   = to;
	    }
	    else if(sscanf(p,"%c:%d-%d",&c,&from,&to)==3)
	    {
		scop->Start[i] = from;
		scop->End[i]   = to;
		ajStrAssSub(&scop->Chain[i],chains,0,0);
	    }
	    else if(ajStrChar(token,1)==':')
	    {
		scop->Start[i] = 0;
		scop->End[i]   = 0;
		ajStrAssSub(&scop->Chain[i],chains,0,0);
	    }
	    else if(ajRegExec(exp2,token))
	    {
		ajRegSubI(exp2,1,&str);
		ajStrToInt(str,&scop->Start[i]);
		ajRegSubI(exp2,2,&scop->Chain[i]);
		ajRegSubI(exp2,3,&str);
		ajStrToInt(str,&scop->End[i]);
	    }
	    else
		ajFatal("Unparseable chain line [%S]\n",chains);
	}
	ajStrTokenClear(&handle);

	if(ajRegExec(t1exp,text))
	{
	    ajRegSubI(t1exp,1,&scop->Db);
	    ajRegSubI(t1exp,2,&scop->Class);
	    ajRegSubI(t1exp,3,&scop->Fold);
	    ajRegSubI(t1exp,4,&scop->Superfamily);
	    ajRegSubI(t1exp,5,&scop->Family);
	    ajRegSubI(t1exp,6,&scop->Domain);
	    ajRegSubI(t1exp,7,&scop->Source);
	}
	else if(ajRegExec(t2exp,text))
	{
	    ajRegSubI(t2exp,1,&scop->Db);
	    ajRegSubI(t2exp,2,&scop->Class);
	    ajRegSubI(t2exp,3,&scop->Fold);
	    ajRegSubI(t2exp,4,&scop->Superfamily);
	    ajRegSubI(t2exp,5,&scop->Family);
	    ajRegSubI(t2exp,6,&scop->Domain);
	    ajStrAssC(&scop->Source,"Unknown");
	}
	else
	    ajFatal("Unparseable Text [%S]",text);

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

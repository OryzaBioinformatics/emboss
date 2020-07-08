/*  Last edited: Mar  1 18:30 2000 (pmr) */
/* @source prosextract.c
** @author: Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
**
** Application for extracting relevent lines from the Prosite motif database.
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

#define DATANAME "PROSITE/prosite.lines" 

int main (int argc, char *argv[] )
{
    AjPFile infdat 	=NULL;
    AjPFile infdoc 	=NULL;
    AjPFile outf	=NULL;
    AjPFile outs	=NULL;
    
    AjBool  haspattern;

    char   *p;
    
    
    AjPStr line =NULL;
    AjPStr text =NULL;
    AjPStr temp = NULL;
    AjPStr id=NULL;
    AjPStr ac=NULL;
    AjPStr de = NULL;
    AjPStr pa=NULL;
    AjPStr ps =NULL;
    AjPStr fn=NULL;
    AjPStr re =NULL; 
    AjPStr fname =NULL;
    AjBool flag;
    AjBool isopen;
    AjBool goback;

    long storepos=0L;
    

    
    embInit ("prosextract", argc, argv);

    temp = ajAcdGetString("infdat");

    line =ajStrNew();
    text =ajStrNew();

    id = ajStrNew();
    ac = ajStrNew();
    de = ajStrNew();
    pa = ajStrNew();
    ps = ajStrNew();
   
   

    fn=ajStrNew();
    ajStrAssC(&fn,ajStrStr(temp));
    ajStrAppC(&fn,"/prosite.dat");
    if(!(infdat=ajFileNewIn(fn)))
	ajFatal("Cannot open file %s",ajStrStr(fn));
    ajStrDel(&fn);
    
    

    fn=ajStrNewC("PROSITE/prosite.lines");
    ajFileDataNewWrite(fn,&outf);
    ajStrDel(&fn);
    
    
	
    haspattern=ajFalse;

    while (ajFileReadLine(infdat, &line) )
    {
	if (ajStrPrefixC(line, "ID"))
	{
	    if(ajStrSuffixC(line,"PATTERN."))
	    {
		haspattern=ajTrue;
		/*save id*/
		p=ajStrStr(line);
		p=strtok(p," \t;");
		p=strtok(NULL," \t;");
		ajStrAssC(&id,p);
		ajFmtPrintF (outf, "%s ", ajStrStr(id));
		continue;
	    }
	    else
	    {
		haspattern=ajFalse;
		continue;
	    }
	}

	if(!haspattern) continue;


	if(ajStrPrefixC(line, "AC") )
	{
	    p=ajStrStr(line);
	    p=strtok(p, " \t;");
	    p=strtok(NULL, " \t;");
	    ajStrAssC(&ac,p);
	    ajFmtPrintF (outf, "%s\n ", ajStrStr(ac));
	    continue;
	}

    	if(ajStrPrefixC(line, "DE") )
	{
	    p=ajStrStr(line);
	    p=strtok(p, " \t.");
	    p=strtok(NULL, " \t.");
	    ajStrAssC(&de,p);
	    ajFmtPrintF (outf, "%s\n ", ajStrStr(de));
	    continue;
	}
    
    
	if (ajStrPrefixC(line, "PA"))
	{	
	    ajStrAssC(&pa,"");
		
	    while(ajStrPrefixC(line,"PA"))
	    {
		p=ajStrStr(line);
		p=strtok(p, " \t.");
		p=strtok(NULL, " \t.");
		ajStrAppC(&pa,p);
		ajFileReadLine(infdat, &line);
	    }
	    
	    ajFmtPrintF (outf, "%s\n", ajStrStr(pa));	
	    re=embPatPrositeToRegExp(&pa);
	    ajFmtPrintF (outf, "^%s\n\n", ajStrStr(re));		
	    ajStrDel(&re);
	    continue;
	}
    
		
    }    
    
    
  /* Now we've finished processing prosite.dat so look at prosite.doc */
        

    fn=ajStrNew();
    ajStrAssC(&fn,ajStrStr(temp));
    ajStrAppC(&fn,"/prosite.doc");
    if(!(infdoc=ajFileNewIn(fn)))
	ajFatal("Cannot open file %s",ajStrStr(fn));
    ajStrDel(&fn);



    fname = ajStrNewC("PROSITE/");
    flag  = ajFalse;
    isopen = ajFalse;
    goback = ajFalse;
    

    while (ajFileReadLine(infdoc, &text))
    {

	if(ajStrPrefixC(text, "{PS") && isopen && !goback)
	    goback = ajTrue;
	    


	if(ajStrPrefixC(text, "{PS") && !isopen) 
	{
	    storepos = ajFileTell(infdoc);
	    /* save out the documentation text to acc numbered outfiles . */
	    p=ajStrStr(text)+1;
	    p=strtok(p, ";");
	    ajStrAssC(&temp, ajStrStr(fname));
	    ajStrAppC(&temp, p);
	    
	    ajFileDataNewWrite(temp, &outs); 
	    flag = ajTrue;
	    isopen = ajTrue;
	    continue;
	}
            
            
	if(ajStrPrefixC(text, "{BEGIN}") && flag)
	{		
	    while(ajFileReadLine(infdoc, &text))
	    {
		if(ajStrPrefixC(text,"{END}")) break;
		ajFmtPrintF(outs, "%s\n", ajStrStr(text)); 

	    }
	    ajFileClose(&outs);
	    isopen = ajFalse;
	    if(goback)
	    {
		goback = ajFalse;
		ajFileSeek(infdoc,storepos,0);
	    }
	    
	}
    }  
    
    ajStrDel(&line);
    ajStrDel(&text);
    ajStrDel(&temp);
    ajStrDel(&id);
    ajStrDel(&ac);
    ajStrDel(&de);
    ajStrDel(&pa);
    ajStrDel(&re);
    ajStrDel(&ps);
    ajStrDel(&fname);
   
    
    ajFileClose(&infdat);
    ajFileClose(&infdoc);
    ajFileClose(&outf);
    ajExit();
    return 0;
}

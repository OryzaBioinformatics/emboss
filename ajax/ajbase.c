/********************************************************************
** @source AJAX IUB base nucleic acid functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0 
** @modified Feb 28 ajb First version
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
********************************************************************/

#include "ajax.h"
#include <string.h>

#define IUBFILE "Ebases.iub"

AjIUB aj_base_iubS[256];	/* Base letters and their alternatives */
int aj_base_table[256];		/* Base letter numerical codes         */
float aj_base_prob[32][32];     /* Assym base probability matches      */



AjBool aj_base_I= 0;




/* @func ajAZToInt *********************************************************
**
** Returns A=0 to Z=25  or 27 otherwise
**
** @param  [r] c [int] character to convert
**
** @return [int] A=0 to Z=25 or 27 if unknown
** @@
******************************************************************************/

int ajAZToInt(int c)
{
  if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )
    return(toupper(c)-(int)'A');
  return 27;
}






/* @func ajIntToAZ *********************************************************
**
** Returns 'A' for 0 to  'Z' for 25
**
** @param  [r] n [int] character to convert
**
** @return [int] 0 as 'A' up to  25 as 'Z'
** @@
******************************************************************************/

int ajIntToAZ(int n)
{
    return(n+(int)'A');
}





/* @func ajAZToBin *********************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by ajBaseInit
**
** @param  [r] c [int] character to convert
**
** @return [int] Binary OR'd representation
******************************************************************************/

int ajAZToBin(int c)
{
    if(!aj_base_I) ajBaseInit();
    return (aj_base_table[toupper(c)]);
}



/* @func ajAZToBinC *********************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by ajBaseInit
**
** @param  [r] c [char] character to convert
**
** @return [char] Binary OR'd representation
******************************************************************************/

char ajAZToBinC(char c)
{
    if(!aj_base_I) ajBaseInit();
    return ajSysItoC(aj_base_table[toupper((int) c)]);
}





/* @func ajBaseInit *********************************************************
**
** Sets up binary OR'd representation of an IUB bases in a table
** aj_base_table where A=1, C=2, G=4 and T=8
** Also sets up a match probability array aj_base_prob holding the
** probability of one IUB base matching any other.
** Uses the Ebases.iub file
** Is initialised if necessary from other AJAX functions.
**
** @return [void]
******************************************************************************/

void ajBaseInit(void)
{
    AjPFile bfptr=NULL;
    AjPStr  bfname=NULL;
    AjPStr  line=NULL;
    AjPStr  code=NULL;
    AjPStr  list=NULL;

    int i;
    int j;
    int k;

    int c;
    
    int l1;
    int l2;
    
    int x;
    int y;
    
    int n;
    char *p;
    char *q;
    
    if(aj_base_I) return;


    for(i=0;i<256;++i)
    {
	aj_base_iubS[i].code = ajStrNewC("");
	aj_base_iubS[i].list = ajStrNewC("");
	aj_base_table[i] = 0;
    }
    
    code = ajStrNew();
    list = ajStrNew();
    (void) ajStrAssC(&code,"");
    (void) ajStrAssC(&list,"ACGT");
    



    bfname=ajStrNew();
    (void) ajStrAssC(&bfname,IUBFILE);
    ajFileDataNew(bfname, &bfptr);
    if(!bfptr) ajFatal("Ebases.iub file not found\n");


    line = ajStrNew();

    
    while(ajFileGets(bfptr, &line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || *p=='\n') continue;
	p=ajSysStrtok(p," \t\r");
	(void) ajStrAssC(&code,p);
	p=ajSysStrtok(NULL," \t\r");
	if(sscanf(p,"%d",&n)!=1)
	    ajFatal("Bad format IUB file");
	p=ajSysStrtok(NULL," \t\r");
	(void) ajStrAssC(&list,p);
	q=ajStrStr(code);
	p=ajStrStr(list);
	(void) ajStrAssC(&aj_base_iubS[toupper((int) *q)].code,q);
	(void) ajStrAssC(&aj_base_iubS[toupper((int) *q)].list,p);
	(void) ajStrAssC(&aj_base_iubS[tolower((int) *q)].code,q);
	(void) ajStrAssC(&aj_base_iubS[tolower((int) *q)].list,p);
	aj_base_table[toupper((int) *q)] = n;
	aj_base_table[tolower((int) *q)] = n;
    }
    
    ajStrDel(&code);
    ajStrDel(&list);
    ajStrDel(&line);
    ajStrDel(&bfname);
    
    ajFileClose(&bfptr);


    for(i=0;i<32;++i)
    {
	x = ajIntToAZ(i);
	for(j=0;j<32;++j)
	{
	    y = ajIntToAZ(j);
	    if(!(l1=ajStrLen(aj_base_iubS[x].code)))
	    {
		aj_base_prob[i][j]=0.0;
		continue;
	    }
	    if(l1!=1) ajFatal("Bad IUB letter");
	    

	    p = ajStrStr(aj_base_iubS[x].list);
	    q = ajStrStr(aj_base_iubS[y].list);
	    l1 = strlen(p);
	    l2 = strlen(q);
	    for(k=0,c=0;k<l1;++k)
		if(strchr(q,(int)*(p+k))) ++c;
	    if(l2) aj_base_prob[i][j] = (float)c / (float)l2;
	    else aj_base_prob[i][j]=0.0;
	}
    }

    aj_base_I = ajTrue;
}

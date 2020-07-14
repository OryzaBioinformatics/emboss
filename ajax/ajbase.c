/******************************************************************************
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
******************************************************************************/

#include "ajax.h"
#include <string.h>

#define IUBFILE "Ebases.iub"

AjIUB aj_base_iubS[256];	/* Base letters and their alternatives */
ajint aj_base_table[256];	/* Base letter numerical codes         */
float aj_base_prob[32][32];     /* Assym base probability matches      */



AjBool aj_base_I = 0;




/* @func ajAZToInt ************************************************************
**
** Returns A=0 to Z=25  or 27 otherwise
**
** @param  [r] c [ajint] character to convert
**
** @return [ajint] A=0 to Z=25 or 27 if unknown
** @@
******************************************************************************/

ajint ajAZToInt(ajint c)
{
    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') )
	return(toupper(c)-(ajint)'A');

    return 27;
}




/* @func ajIntToAZ ************************************************************
**
** Returns 'A' for 0 to  'Z' for 25
**
** @param  [r] n [ajint] character to convert
**
** @return [ajint] 0 as 'A' up to  25 as 'Z'
** @@
******************************************************************************/

ajint ajIntToAZ(ajint n)
{
    return(n+(ajint)'A');
}




/* @func ajAZToBin ************************************************************
**
** Returns a binary OR'd representation of an IUB base where A=1, C=2,
** G=4 and T=8
** Uses the base table set up by ajBaseInit
**
** @param  [r] c [ajint] character to convert
**
** @return [ajint] Binary OR'd representation
******************************************************************************/

ajint ajAZToBin(ajint c)
{
    if(!aj_base_I)
	ajBaseInit();

    return (aj_base_table[toupper(c)]);
}




/* @func ajAZToBinC ***********************************************************
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
    if(!aj_base_I)
	ajBaseInit();

    return ajSysItoC(aj_base_table[toupper((ajint) c)]);
}




/* @func ajBaseInit ***********************************************************
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
    AjPFile bfptr  = NULL;
    AjPStr  bfname = NULL;
    AjPStr  line   = NULL;
    AjPStr  code   = NULL;
    AjPStr  list   = NULL;

    ajint i;
    ajint j;
    ajint k;

    ajint c;

    ajint l1;
    ajint l2;

    ajint x;
    ajint y;

    ajint n;
    char *p;
    char *q;

    if(aj_base_I)
	return;


    for(i=0;i<256;++i)
    {
	aj_base_iubS[i].code = ajStrNewC("");
	aj_base_iubS[i].list = ajStrNewC("");
	aj_base_table[i] = 0;
    }

    code = ajStrNew();
    list = ajStrNew();
    ajStrAssC(&code,"");
    ajStrAssC(&list,"ACGT");


    bfname = ajStrNew();
    ajStrAssC(&bfname,IUBFILE);
    ajFileDataNew(bfname, &bfptr);
    if(!bfptr) ajFatal("Ebases.iub file not found\n");


    line = ajStrNew();


    while(ajFileGets(bfptr, &line))
    {
	p = ajStrStr(line);
	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;
	p = ajSysStrtok(p," \t\r");
	ajStrAssC(&code,p);
	p=ajSysStrtok(NULL," \t\r");
	if(sscanf(p,"%d",&n)!=1)
	    ajFatal("Bad format IUB file");
	p = ajSysStrtok(NULL," \t\r");
	ajStrAssC(&list,p);
	q = ajStrStr(code);
	p = ajStrStr(list);
	ajStrAssC(&aj_base_iubS[toupper((ajint) *q)].code,q);
	ajStrAssC(&aj_base_iubS[toupper((ajint) *q)].list,p);
	ajStrAssC(&aj_base_iubS[tolower((ajint) *q)].code,q);
	ajStrAssC(&aj_base_iubS[tolower((ajint) *q)].list,p);
	aj_base_table[toupper((ajint) *q)] = n;
	aj_base_table[tolower((ajint) *q)] = n;
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
	    if(l1!=1)
		ajFatal("Bad IUB letter");


	    p = ajStrStr(aj_base_iubS[x].list);
	    q = ajStrStr(aj_base_iubS[y].list);
	    l1 = strlen(p);
	    l2 = strlen(q);
	    for(k=0,c=0;k<l1;++k)
		if(strchr(q,(ajint)*(p+k))) ++c;
	    if(l2)
		aj_base_prob[i][j] = (float)c / (float)l2;
	    else
		aj_base_prob[i][j]=0.0;
	}
    }

    aj_base_I = ajTrue;

    return;
}




/* @func ajBaseAa1ToAa3 *******************************************************
**
** Writes an AjPStr with a amino acid 3 letter code
** JCI - This should probably be an emb function and might replace the use
** of embPropCharToThree &  embPropIntToThree
**
** @param [r] aa1 [char]    Single letter identifier of amino acid
** @param [w] aa3  [AjPStr *] AjPStr object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajBaseAa1ToAa3(char aa1, AjPStr *aa3)
{
    ajint idx;

    static char *tab[]=
    {
	"ALA\0","ASX\0","CYS\0","ASP\0","GLU\0","PHE\0","GLY\0","HIS\0",
	"ILE\0","---\0","LYS\0","LEU\0","MET\0","ASN\0","---\0","PRO\0",
	"GLN\0","ARG\0","SER\0","THR\0","---\0","VAL\0","TRP\0","XAA\0",
	"TYR\0","GLX\0"
    };

    if((idx=ajAZToInt(aa1))==27)
	return ajFalse;

    ajStrAssC(aa3, tab[idx]);
    return ajTrue;
}

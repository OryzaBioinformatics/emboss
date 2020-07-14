/* @source rebaseextract application
**
** Extracts restriction enzyme information from REBASE
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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>


#define DATANAME  "REBASE/embossre.enz"
#define DATANAME2 "REBASE/embossre.ref"
#define DATANAME3 "REBASE/embossre.sup"
#define DATANAME4 "embossre.equ"
#define EQUGUESS  5000


static void rebex_process_pattern(AjPStr *pattern, AjPStr *code, AjPFile outf,
		     AjBool hassup);
static void rebex_printEnzHeader(AjPFile outf);
static void rebex_printRefHeader(AjPFile outf);
static void rebex_printSuppHeader(AjPFile outf);



/* @prog rebaseextract ********************************************************
**
** Extract data from REBASE
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf   = NULL;
    AjPFile infp  = NULL;
    AjPFile outf  = NULL;
    AjPFile outf2 = NULL;
    AjPFile outf3 = NULL;

    AjPStr  line;
    AjPStr  code;
    AjPStr  pattern;
    AjPStr  isoschiz;
    AjPStr  meth;
    AjPStr  tit;
    AjPStr  sou;
    AjPStr  comm;
    AjPStr  pfname;
    AjBool  isrefm = ajFalse;
    AjBool  isref  = ajFalse;
    AjBool  hassup;

    ajint     count;
    ajlong    pos;
    ajint     i;

    AjBool    doequ;
    AjPFile   oute      = NULL;
    AjPStr    isostr    = NULL;
    AjPTable  ptable    = NULL;
    AjPStr    key       = NULL;
    AjPStr    value     = NULL;
    AjPStrTok handle    = NULL;
    AjPStr    token     = NULL;
    AjPStr    line2     = NULL;
    
    AjBool    isproto   = ajFalse;
    char      c;
    char      *sptr     = NULL;
    
    embInit("rebaseextract",argc,argv);

    inf   = ajAcdGetInfile("inf");
    infp  = ajAcdGetInfile("proto");
    doequ = ajAcdGetBool("equivalences");
    

    pfname = ajStrNewC(DATANAME);
    ajFileDataNewWrite(pfname,&outf);
    rebex_printEnzHeader(outf);

    ajStrAssC(&pfname,DATANAME2);
    ajFileDataNewWrite(pfname,&outf2);
    rebex_printRefHeader(outf2);

    ajStrAssC(&pfname,DATANAME3);
    ajFileDataNewWrite(pfname,&outf3);
    rebex_printSuppHeader(outf3);

    if(doequ)
    {
	ajStrAssC(&pfname,DATANAME4);
	ajFileDataNewWrite(pfname,&oute);
	ptable = ajStrTableNew(EQUGUESS);
	isostr = ajStrNew();
    }
    ajStrDel(&pfname);

    line     = ajStrNew();
    line2    = ajStrNew();
    code     = ajStrNew();
    pattern  = ajStrNew();
    isoschiz = ajStrNew();
    meth     = ajStrNew();
    tit      = ajStrNew();
    sou      = ajStrNew();
    comm     = ajStrNew();
    token    = ajStrNew();

    /*
     *  Extract Supplier information
     */
    while(ajFileReadLine(inf,&line))
    {
	if(ajStrFindC(line,"withrefm.")>=0)
	    isrefm = ajTrue;
	if(ajStrFindC(line,"withref.")>=0)
	    isref = ajTrue;

	if(strstr(ajStrStr(line),"REBASE codes"))
	    break;
    }


    while(ajFileReadLine(infp,&line))
    {
	if(ajStrFindC(line,"proto.")>=0)
	    isproto = ajTrue;

	
	if(strstr(ajStrStr(line),"Rich Roberts"))
	    break;
    }

    if(!isrefm)
    {
	if(isref)
	    ajFatal("WITHREF file specified by mistake. Use WITHREFM instead");
	else
	    ajFatal("Invalid withrefm file");
    }


    if(!isproto)
	ajFatal("Invalid PROTO file specified");



    while(doequ && ajFileReadLine(infp,&line))
    {
	if(!ajStrLen(line))
	    continue;
	sptr = ajStrStr(line);
	c = *sptr;
	if(c>='A' && c<='Z')
	{
	    while(*sptr!=' ')
		++sptr;
	    while(*sptr==' ')
		++sptr;
	    
	    key   = ajStrNew();
	    value = ajStrNewC(sptr);
	    ajStrCleanWhite(&value);
	    ajFmtScanS(line,"%S",&key);
	    ajTablePut(ptable,(const void *)key, (void *)value);
	}
    }
    
    if(!ajFileReadLine(inf,&line))
	ajFatal("No Supplier Information Found");
    if(!ajFileReadLine(inf,&line))
	ajFatal("Unexpected EOF");

    while(ajStrLen(line))
    {
	ajStrClean(&line);
	ajFmtPrintF(outf3,"%s\n",ajStrStr(line));
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");
    }
    ajFileClose(&outf3);



    while(ajFileReadLine(inf,&line))
    {
	/* Get RE */
	if(!ajStrPrefixC(line,"<1>"))
	    continue;
	ajStrAssC(&code,ajStrStr(line)+3);
	/* Get isoschizomers */
	if(!ajFileReadLine(inf,&line2))
	    ajFatal("Unexpected EOF");

	if(ajStrLen(line2)>3)
	{
	    ajStrAssC(&isoschiz,ajStrStr(line2)+3);
	}
	else
	    ajStrAssC(&isoschiz,"");


	/* Get recognition sequence */
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrLen(line)>3)
	{
	    ajStrAssC(&pattern,ajStrStr(line)+3);

	    if(doequ && ajStrLen(line2)>3)
	    {
		ajStrAssS(&isostr,isoschiz);
		handle = ajStrTokenInit(isostr,"\t\n>,");
	        ajStrToken (&token, &handle, NULL);
		if((value=ajTableGet(ptable,(const void *)token)))
		    if(ajStrMatch(value,pattern))
			ajFmtPrintF(oute,"%S %S\n",code,token);
		ajStrTokenClear(&handle);
	    }
	}
	else
	    ajStrAssC(&pattern,"");

	/* Get methylation */
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrLen(line)>3)
	    ajStrAssC(&meth,ajStrStr(line)+3);
	else
	    ajStrAssC(&meth,"");

	/* Get title */
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrLen(line)>3)
	    ajStrAssC(&tit,ajStrStr(line)+3);
	else
	    ajStrAssC(&tit,"");

	/* Get source */
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");

	if(ajStrLen(line)>3)
	    ajStrAssC(&sou,ajStrStr(line)+3);
	else
	    ajStrAssC(&sou,"");

	/* Get commercial supplier */
	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");

	hassup=ajFalse;
	if(ajStrLen(line)>3)
	{
	    hassup=ajTrue;
	    ajStrAssC(&comm,ajStrStr(line)+3);
	}
	else
	    ajStrAssC(&comm,"");

	ajFmtPrintF(outf2,"%s\n",ajStrStr(code));
	ajFmtPrintF(outf2,"%s\n",ajStrStr(tit));
	ajFmtPrintF(outf2,"%s\n",ajStrStr(isoschiz));
	ajFmtPrintF(outf2,"%s\n",ajStrStr(meth));
	ajFmtPrintF(outf2,"%s\n",ajStrStr(sou));
	ajFmtPrintF(outf2,"%s\n",ajStrStr(comm));

	/* Get references */
	count = -1;
	pos = ajFileTell(inf);
	while(ajStrLen(line))
	{
	    if(!ajFileReadLine(inf,&line))
		break;
	    ++count;
	}
	ajFileSeek(inf,pos,0);


	if(!ajFileReadLine(inf,&line))
	    ajFatal("Unexpected EOF");
	if(ajStrLen(line)==3)
	    ajFmtPrintF(outf2,"0\n");
	else
	{
	    ajFmtPrintF(outf2,"%d\n%s\n",count,ajStrStr(line)+3);
	    for(i=1;i<count;++i)
	    {
		if(!ajFileReadLine(inf,&line))
		    ajFatal("Unexpected EOF");
		ajFmtPrintF(outf2,"%s\n",ajStrStr(line));
	    }
	}
	ajFmtPrintF(outf2,"//\n");


	rebex_process_pattern(&pattern,&code,outf,hassup);

    }


    if(doequ)
    {
	ajStrDel(&isostr);
	ajFileClose(&oute);
	ajStrTableFree(&ptable);
    }

    ajFileClose(&inf);
    ajFileClose(&infp);
    ajFileClose(&outf);
    ajFileClose(&outf2);


    ajStrDel(&line);
    ajStrDel(&line2);
    ajStrDel(&token);
    ajStrDel(&isoschiz);
    ajStrDel(&tit);
    ajStrDel(&meth);
    ajStrDel(&sou);
    ajStrDel(&comm);
    ajStrDel(&pattern);
    ajStrDel(&code);

    ajExit();
    return 0;
}




/* @funcstatic rebex_process_pattern ******************************************
**
** Convert rebase pattern into emboss pattern
**
** @param [r] pattern [AjPStr*] rebase recog sequence
** @param [r] code [AjPStr*] re name
** @param [w] outf [AjPFile] outfile
** @param [r] hassup [AjBool] has a supplier
** @@
******************************************************************************/


static void rebex_process_pattern(AjPStr *pattern, AjPStr *code, AjPFile outf,
				  AjBool hassup)
{
    AjPStr temp;
    AjPStr ppat;
    AjPStrTok tokens;

    char   *p;
    char   *q;
    char   *r;
    char   *t;

    /*  ajint    carat;*/
    AjBool hascarat;

    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
    ajint len;
    ajint ncuts;
    ajint nc;
    ajint i;
    AjBool blunt=ajFalse;


    ajStrToUpper(pattern);
    temp = ajStrNew();
    ppat = ajStrNew();

    tokens=ajStrTokenInit(*pattern,",");

    while(ajStrToken(&ppat,&tokens,","))
    {
	ajFmtPrintF(outf,"%s\t",ajStrStr(*code));

	ajStrAssC(&temp,ajStrStr(ppat));

	hascarat = ajFalse;
	/*      carat=0; NOT USED */
	p=ajStrStr(ppat);

	if(*p=='?')
	{
	    ajFmtPrintF(outf,"?\t0\t0\t0\t0\t0\t0\t0\n");
	    continue;
	}


	t=p;
	if(*p=='(')
	{
	    sscanf(p+1,"%d/%d",&cut1,&cut2);
	    q=p+1;
	    if(!(q=strchr(q,(ajint)'(')))
		ajFatal("Bad pattern %s in %s",ajStrStr(*code),
			ajStrStr(*pattern));
	    sscanf(q+1,"%d/%d",&cut3,&cut4);
	    cut1 *= -1;
	    cut2 *= -1;
	    --cut1;
	    --cut2;

	    if(!(p=strchr(p,(ajint)')')))
		ajFatal("%s mismatched parentheses",ajStrStr(*code));

	    p=ajStrStr(ppat);
	    r=ajStrStr(temp);
	    while(*p)
	    {
		if(*p>='A' && *p<='Z')
		    *r++ = *p;
		++p;
	    }
	    *r = '\0';
	    ajStrAssC(&ppat,ajStrStr(temp));
	    len=ajStrLen(ppat);
	    cut3 += len;
	    cut4 += len;
	    ncuts = 4;
	    if(cut1==cut2 && cut3==cut4) blunt=ajTrue;
	    else blunt=ajFalse;
	    p=t;
	}
	else
	{
	    ncuts=2;
	    cut3=cut4=0;
	    if((p=strchr(p,(ajint)'(')))
	    {
		sscanf(p+1,"%d/%d",&cut1,&cut2);
		if(cut1==cut2) blunt=ajTrue;
		else blunt=ajFalse;
		p=ajStrStr(ppat);
		r=ajStrStr(temp);
		while(*p)
		{
		    if(*p>='A' && *p<='Z')
			*r++ = *p;
		    ++p;
		}
		*r = '\0';
		ajStrAssC(&ppat,ajStrStr(temp));
		len=ajStrLen(ppat);
		cut1 += len;
		cut2 += len;
		if(cut1<=0) --cut1;
		if(cut2<=0) --cut2;
	    }
	    else			/* probably a carat */
	    {
		p=ajStrStr(ppat);
		r=ajStrStr(temp);
		cut1=0;
		hascarat=ajFalse;

		while(*p)
		{
		    if(*p>='A' && *p<='Z')
		    {
			*r++ = *p;
			if(!hascarat) ++cut1;
		    }
		    if(*p=='^') hascarat=ajTrue;
		    ++p;
		}
		*r = '\0';
		ajStrAssC(&ppat,ajStrStr(temp));
		len=ajStrLen(ppat);
		if(!hascarat)
		{
		    ncuts=0;
		    blunt=ajFalse;
		    cut1=0;
		    cut2=0;
		}
		else
		{
		    if(len==cut1*2)
		    {
			blunt=ajTrue;
			cut2=cut1;
		    }
		    else if(!cut1)
		    {
			cut1=-1;
			cut2=len;
		    }
		    else
		    {
			p=ajStrStr(ppat);
			if(p[cut1-1]=='N' && cut1==len)
			{
			    for(i=cut1-1,nc=0;i>-1;--i)
				if(p[i]=='N')
				    ++nc;
			    cut2=len-cut1-nc-1;
			}
			else if(cut1==len) cut2=-1;
			else
			    cut2=len-cut1;
			blunt=ajFalse;
		    }
		}
	    }
	}

	/* Mark RE's with no suppliers with lc sequence */
	if(!hassup)
	    ajStrToLower(&ppat);


	if(ncuts==4)
	    ajFmtPrintF(outf,"%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			ajStrStr(ppat),len,ncuts,blunt,cut1,cut2,cut3,cut4);
	else
	    ajFmtPrintF(outf,"%s\t%d\t%d\t%d\t%d\t%d\t0\t0\n",ajStrStr(ppat),
			len,ncuts,blunt,cut1,cut2);
    }
    ajStrDel(&temp);
    ajStrDel(&ppat);
    ajStrTokenClear(&tokens);

    return;
}




/* @funcstatic rebex_printEnzHeader *******************************************
**
** print comments at start of embossre.enz
**
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/


static void rebex_printEnzHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# REBASE enzyme patterns for EMBOSS\n#\n");
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# name<ws>pattern<ws>len<ws>ncuts<ws>");
    ajFmtPrintF(outf,"blunt<ws>c1<ws>c2<ws>c3<ws>c4\n");
    ajFmtPrintF(outf,"#\n");
    ajFmtPrintF(outf,"# Where:\n");
    ajFmtPrintF(outf,"# name = name of enzyme\n");
    ajFmtPrintF(outf,"# pattern = recognition site\n");
    ajFmtPrintF(outf,"# len = length of pattern\n");
    ajFmtPrintF(outf,"# ncuts = number of cuts made by enzyme\n");
    ajFmtPrintF(outf,"#         Zero represents unknown\n");
    ajFmtPrintF(outf,"# blunt = true if blunt end cut, false if sticky\n");
    ajFmtPrintF(outf,"# c1 = First 5' cut\n");
    ajFmtPrintF(outf,"# c2 = First 3' cut\n");
    ajFmtPrintF(outf,"# c3 = Second 5' cut\n");
    ajFmtPrintF(outf,"# c4 = Second 3' cut\n#\n# Examples:\n");
    ajFmtPrintF(outf,"# AAC^TGG -> 6 2 1 3 3 0 0\n");
    ajFmtPrintF(outf,"# A^ACTGG -> 6 2 0 1 5 0 0\n");
    ajFmtPrintF(outf,"# AACTGG  -> 6 0 0 0 0 0 0\n");
    ajFmtPrintF(outf,"# AACTGG(-5/-1) -> 6 2 0 1 5 0 0\n");
    ajFmtPrintF(outf,"# (8/13)GACNNNNNNTCA(12/7) -> 12 4 0 -9 -14 24 19\n");
    ajFmtPrintF(outf,"#\n");
    ajFmtPrintF(outf,"# i.e. cuts are always to the right of the given\n");
    ajFmtPrintF(outf,"# residue and sequences are always with reference to\n");
    ajFmtPrintF(outf,"# the 5' strand.\n");
    ajFmtPrintF(outf,"# Sequences are numbered ... -3 -2 -1 1 2 3 ... with\n");
    ajFmtPrintF(outf,"# the first residue of the pattern at base number 1.\n");
    ajFmtPrintF(outf,"#\n");

    return;
}



/* @funcstatic rebex_printRefHeader *******************************************
**
** Print header to embossre.ref
**
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/


static void rebex_printRefHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# REBASE enzyme information for EMBOSS\n#\n");
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# Line 1: Name of Enzyme\n");
    ajFmtPrintF(outf,"# Line 2: Organism\n");
    ajFmtPrintF(outf,"# Line 3: Isoschizomers\n");
    ajFmtPrintF(outf,"# Line 4: Methylation\n");
    ajFmtPrintF(outf,"# Line 5: Source\n");
    ajFmtPrintF(outf,"# Line 6: Suppliers\n");
    ajFmtPrintF(outf,"# Line 7: Number of following references\n");
    ajFmtPrintF(outf,"# Lines 8..n: References\n");
    ajFmtPrintF(outf,"# // (end of entry marker)\n");
    ajFmtPrintF(outf,"#\n");

    return;
}


/* @funcstatic rebex_printSuppHeader ******************************************
**
** Print header to embossre.sup
**
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/


static void rebex_printSuppHeader(AjPFile outf)
{
    ajFmtPrintF(outf,"# REBASE Supplier information for EMBOSS\n#\n");
    ajFmtPrintF(outf,"# Format:\n");
    ajFmtPrintF(outf,"# Code of Supplier<ws>Supplier name\n");
    ajFmtPrintF(outf,"#\n");

    return;
}

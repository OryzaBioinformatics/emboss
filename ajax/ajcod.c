/******************************************************************************
** @source AJAX codon functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Aug 07 ajb First version
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

#include <math.h>
#include "ajax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <limits.h>


static double codRandom(ajint NA, ajint NC, ajint NG, ajint NT, ajint len,
			const char *p);


#define AJCODSIZE 66
#define AJCODSTART 64
#define AJCODSTOP 65
#define AJCODEND  65
#define AJCODAMINOS 28




/* @section Codon Constructors ************************************************
**
** All constructors return a new object by pointer. It is the responsibility
** of the user to first destroy any previous object. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/




/* @func ajCodNew *************************************************************
**
** Default constructor for empty AJAX codon objects.
**
** @return [AjPCod] Pointer to an codon object
** @@
******************************************************************************/

AjPCod ajCodNew(void)
{
    AjPCod thys;

    AJNEW0(thys);

    thys->Name = ajStrNew();
    thys->Desc = ajStrNew();
    AJCNEW(thys->back, AJCODAMINOS);
    AJCNEW(thys->aa, AJCODSIZE);
    AJCNEW(thys->num, AJCODSIZE);
    AJCNEW(thys->tcount, AJCODSIZE);
    AJCNEW(thys->fraction, AJCODSIZE);

    return thys;
}




/* @func ajCodDup *************************************************************
**
** Duplicate a codon object
**
** @param [r] thys [const AjPCod] Codon to duplicate
**
** @return [AjPCod] Pointer to an codon object
** @@
******************************************************************************/

AjPCod ajCodDup(const AjPCod thys)
{
    AjPCod dup;
    ajint  i;

    dup = ajCodNew();

    ajStrAssS(&dup->Name,thys->Name);
    ajStrAssS(&dup->Desc,thys->Desc);

    for(i=0;i<AJCODSIZE;++i)
    {
	dup->aa[i]       = thys->aa[i];
	dup->num[i]      = thys->num[i];
	dup->tcount[i]   = thys->tcount[i];
	dup->fraction[i] = thys->fraction[i];
    }

    for(i=0;i<AJCODAMINOS;++i)
	dup->back[i] = thys->back[i];

    return dup;
}



/* @section Codon Destructors ************************************************
**
** Desctuctor(s) for AjPCod objects
**
******************************************************************************/


/* @func ajCodDel *************************************************************
**
** Default destructor for AJAX codon objects.
**
** @param [w] thys [AjPCod *] codon usage structure
**
** @return [void]
** @@
******************************************************************************/

void ajCodDel(AjPCod *thys)
{
    AJFREE((*thys)->fraction);
    AJFREE((*thys)->tcount);
    AJFREE((*thys)->num);
    AJFREE((*thys)->aa);
    AJFREE((*thys)->back);
    ajStrDel(&(*thys)->Name);
    ajStrDel(&(*thys)->Desc);

    AJFREE(*thys);

    return;
}




/* @section Codon Functions ************************************************
**
** Desctuctor(s) for AjPCod objects
**
******************************************************************************/


/* @func ajCodBacktranslate ***************************************************
**
** Backtranslate a string
**
** @param [w] b [AjPStr *] backtranslated sequence
** @param [r] a [const AjPStr] sequence
** @param [r] thys [const AjPCod] codon usage object
**
** @return [void]
** @@
******************************************************************************/
void ajCodBacktranslate(AjPStr *b, const AjPStr a, const AjPCod thys)
{
    const char *p;
    char q;
    ajint idx;

    p = ajStrStr(a);
    while(*p)
    {
	if(*p=='-')
	{
	    ++p;
	    continue;
	}

	if(toupper((int)*p)==(int)'X')
	{
	    ajStrAppC(b,"NNN");
	    ++p;
	    continue;
	}

	q = *p;
	if(toupper((int)q)==(int)'B')
	  q = 'D';
	if(toupper((int)q)==(int)'Z')
	  q = 'E';

	idx = thys->back[ajAZToInt(q)];
	if(thys->aa[idx]==27)
	{
	    ajStrAppC(b,"End");
	    ++p;
	    continue;
	}
	ajStrAppC(b,ajCodTriplet(idx));
	++p;
    }

    return;
}




/* @func ajCodBase ************************************************************
**
** Return one codon value given a possibly ambiguous base
**
** @param [r] c [ajint] base
**
** @return [ajint] single base value
** @@
******************************************************************************/
ajint ajCodBase(ajint c)
{
    ajint v;

    v = ajAZToBin(c);

    if(v & 1) return 0;
    if(v & 2) return 1;
    if(v & 4) return 2;
    if(v & 8) return 3;

    return 0;
}




/* @func ajCodCalcGribskov ****************************************************
**
** Calculate Gribskov statistic (count per thousand) in AjPCod internals
**
** @param [u] thys [AjPCod] codon usage for sequence
** @param [r] s [const AjPStr] sequence
**
** @return [void]
** @@
******************************************************************************/
void ajCodCalcGribskov(AjPCod thys, const AjPStr s)
{
    ajint i;
    ajint j;

    ajint NA;
    ajint NC;
    ajint NG;
    ajint NT;

    const char *p;
    ajint len;
    ajint aa;
    double fam[64];
    double frct[64];

    double x;
    double z;

    len = ajStrLen(s);

    for(i=0;i<AJCODSTART;++i)
	frct[i] = thys->fraction[i];

    NA = NC = NG = NT = 0;
    ajCodComp(&NA,&NC,&NG,&NT,ajStrStr(s));

    /* Get expected frequencies */
    for(i=0;i<AJCODSTART;++i)
    {
	p = ajCodTriplet(i);
	thys->tcount[i] = codRandom(NA,NC,NG,NT,len,p);
    }


    /* Calculate expected family */
    for(i=0;i<AJCODSTART;++i)
    {
	x=0.0;
	aa = thys->aa[i];
	for(j=0;j<AJCODSTART;++j)
	    if(thys->aa[j]==aa) x+=thys->tcount[j];
	fam[i] = x;
    }


    /* Calculate expected ratio etc */

    for(i=0;i<AJCODSTART;++i)
    {
	z = thys->tcount[i] / fam[i];

	/* Work out ln(real/expected) */
	thys->tcount[i]= log(frct[i] / z);
    }

    return;
}




/* @func ajCodCalcNc **********************************************************
**
** Calculate effective number of codons
** Wright, F. (1990) Gene 87:23-29
**
** @param [r] thys [const AjPCod] codon usage
**
** @return [double] Nc
** @@
******************************************************************************/

double ajCodCalcNc(const AjPCod thys)
{
    ajint *df = NULL;
    ajint *n  = NULL;
    ajint i;
    ajint j;
    ajint v;

    ajint max;
    ajint ndx;

    ajint    *nt = NULL;
    double *Fbar = NULL;
    double *F    = NULL;
    double sum;
    double num = 0.0;

    AJCNEW0(df, AJCODAMINOS);
    AJCNEW0(n, AJCODAMINOS);

    for(i=0;i<AJCODSTART;++i)
    {
	v = thys->aa[i];
	if(v==27)
	    continue;

	++df[v];
	n[thys->aa[i]] += thys->num[i];
    }


    for(i=0,max=INT_MIN;i<AJCODAMINOS;++i)
	max = (max > df[i]) ? max : df[i];

    AJCNEW0(Fbar, max);
    AJCNEW0(nt, max);
    AJCNEW0(F, AJCODAMINOS);

    for(i=0;i<AJCODAMINOS-2;++i)
    {
	if(df[i])
	    ++nt[df[i]-1];
    }

    for(i=0;i<AJCODAMINOS-2;++i)
      for(j=0;j<AJCODSTART;++j)
	{
	    if(thys->aa[j]==27)
		continue;

	    if(thys->aa[j]==i)
		F[i] += pow(thys->fraction[j],2.0);
	}


    for(i=0;i<AJCODAMINOS-2;++i)
    {
	if(n[i]-1 <1  || (num=((double)n[i]*F[i]-1.0))<0.05)
	{
	    F[i] = 0.0;
	    if(df[i])
		--nt[df[i]-1];
	    continue;
	}
	F[i]= num / ((double)n[i]-1.0);
    }


    for(i=0;i<AJCODAMINOS-2;++i)
    {
	ndx=df[i];
	if(!ndx)
	    continue;
	--ndx;
	Fbar[ndx] += F[i];
    }

    for(i=0;i<max;++i)
	if(nt[i])
	    Fbar[i] /= (double)nt[i];

    if(!Fbar[2])				/* Ile fix */
	Fbar[2] = (Fbar[1]+Fbar[3]) / 2.0;


    for(i=1,sum=2.0;i<max;++i)
    {
	if(!Fbar[i])
	    continue;
	if(i==1)
	    sum += 9.0/Fbar[i];
	else if(i==2)
	    sum += 1.0/Fbar[i];
	else if(i==3)
	    sum += 5.0/Fbar[i];
	else if(i==5)
	    sum += 3.0/Fbar[i];
    }


    AJFREE(F);
    AJFREE(nt);
    AJFREE(Fbar);
    AJFREE(n);
    AJFREE(df);

    if(sum>61.0)
	return 61.0;

    return sum;
}




/* @func ajCodCalculateUsage **************************************************
**
** Calculate fractional and thousand elements of a codon object
** Used for creating a codon usage table
** Requires pre-running of ajCodCountTriplets
**
** @param [u] thys [AjPCod *] Codon object
** @param [r] c [ajint] triplet count
**
** @return [void]
** @@
******************************************************************************/

void ajCodCalculateUsage(AjPCod *thys, ajint c)
{
    ajint i;
    ajint *aasum;

    /* Calculate thousands */
    for(i=0;i<AJCODSTART;++i)
	if(!c)
	    (*thys)->tcount[i]=0.0;
	else
	    (*thys)->tcount[i] = ((double)(*thys)->num[i] / (double)c) * 1000.0;

    /* Get number of codons per amino acid */
    AJCNEW0(aasum, AJCODAMINOS);
    for(i=0;i<AJCODSTART;++i)
	if((*thys)->aa[i]==27)
	    aasum[27] += (*thys)->num[i];
	else
	    aasum[(*thys)->aa[i]] += (*thys)->num[i];


    /* Calculate fraction */
    for(i=0;i<AJCODSTART;++i)
	if((*thys)->aa[i]==27)
	{
	    if(!aasum[27])
		(*thys)->fraction[i] = 0.0;
	    else
		(*thys)->fraction[i] = (double)(*thys)->num[i] /
		    (double)aasum[27];
	}
	else
	{
	    if(!aasum[(*thys)->aa[i]])
		(*thys)->fraction[i] = 0.0;
	    else
		(*thys)->fraction[i] = (double)(*thys)->num[i] /
		    (double)aasum[(*thys)->aa[i]];
	}

    AJFREE(aasum);

    return;
}




/* @func ajCodClear ***********************************************************
**
** Zero the name, number count and fraction codon entries
**
** @param [w] thys [AjPCod *] codon usage structure
**
** @return [void]
** @@
******************************************************************************/
void ajCodClear(AjPCod *thys)
{
    ajint i;

    ajStrAssC(&((*thys)->Name),"");
    ajStrAssC(&((*thys)->Desc),"");
    for(i=0;i<AJCODSIZE;++i)
    {
	(*thys)->fraction[i] = (*thys)->tcount[i] = 0.0;
	(*thys)->num[i] = 0;
    }

    for(i=0;i<AJCODAMINOS;++i)
	(*thys)->back[i] = 0;

    return;
}




/* @func ajCodCountTriplets ***************************************************
**
** Load the num array of a codon structure
** Used for creating a codon usage table
**
** @param [w] thys [AjPCod *] Codon object
** @param [r] s [const AjPStr] dna sequence
** @param [w] c [ajint *] triplet count
**
** @return [void]
** @@
******************************************************************************/
void ajCodCountTriplets(AjPCod *thys, const AjPStr s, ajint *c)
{
    const char *p;
    ajint  len;
    ajint  i;
    ajint  idx;

    p = ajStrStr(s);
    len = ajStrLen(s);

    for(i=0;i<len;i+=3,p+=3,++(*c))
	if((idx=ajCodIndexC(p))!=-1)
	    ++(*thys)->num[idx];

    return;
}




/* @func ajCodIndex  **********************************************************
**
** Return a codon index given a three character codon
**
** @param [r] s [const AjPStr] Codon
**
** @return [ajint] Codon index AAA=0 TTT=3f
** @@
******************************************************************************/

ajint ajCodIndex(const AjPStr s)
{
    return ajCodIndexC(ajStrStr(s));
}




/* @func ajCodIndexC **********************************************************
**
** Return a codon index given a three character codon
**
** @param [r] codon [const char *] Codon pointer
**
** @return [ajint] codon index AAA=0 TTT=3f
** @@
******************************************************************************/

ajint ajCodIndexC(const char *codon)
{
    const char *p;
    ajint  sum;
    ajint c;


    p   = codon;
    sum = 0;

    if(!(c=*(p++)))
	return -1;
    sum += (ajCodBase(toupper(c))<<4);
    if(!(c=*(p++)))
	return -1;
    sum += (ajCodBase(toupper(c))<<2);
    if(!(c=*p))
	return -1;
    sum += ajCodBase(toupper(c));

    return sum;
}




/* @func ajCodRead ************************************************************
**
** Read a codon index from a filename
**
** @param [w] thys [AjPCod] Codon object
** @param [r] fn [const AjPStr] filename
**
** @return [AjBool] ajTrue on success
** @category input [AjPCod] Read codon index from a file
** @@
******************************************************************************/
AjBool ajCodRead(AjPCod thys, const AjPStr fn)
{
    AjPFile inf = NULL;
    AjPStr  fname;
    AjPStr  line;
    AjPStr  t;
    const char    *p;
    ajint     idx;
    ajint     c;
    ajint     num;
    double  tcount;
    double  fraction;

    fname = ajStrNewC(ajStrStr(fn));

    ajFileDataNew(fname,&inf);
    if(!inf)
    {
	ajStrAssC(&fname,"CODONS/");
	ajStrAppC(&fname,ajStrStr(fn));
	ajFileDataNew(fname,&inf);
	if(!inf)
	{
	    ajStrDel(&fname);
	    return ajFalse;
	}
    }
    ajStrDel(&fname);

    line = ajStrNew();
    t    = ajStrNew();

    while(ajFileGets(inf,&line))
    {
	p=ajStrStr(line);
	if(*p=='\n')
	    continue;
	else if(*p=='#')
	    continue;
	else if(*p=='!')
	    continue;
	p = ajSysStrtok(p," \t\r\n");
	ajStrAssC(&t,p);
	p = ajSysStrtok(NULL," \t\r\n");
	c = ajAZToInt((ajint)*p);
	if(c>25)
	    c=27;
	p = ajSysStrtok(NULL," \t\r\n");
	if(sscanf(p,"%lf",&fraction)!=1)
	    ajFatal("No fraction in codon file %S", fn);
	p = ajSysStrtok(NULL," \t\r\n");
	if(sscanf(p,"%lf",&tcount)!=1)
	    ajFatal("No tcount in codon file %S", fn);
	p = ajSysStrtok(NULL," \t\r\n");
	if(sscanf(p,"%d",&num)!=1)
	    ajFatal("No num in codon file %S", fn);

	idx = ajCodIndex(t);
	if(idx<0)
	{
	    ajWarn("Corrupt codon index file %S", fn);
	    return ajFalse;
	}

	thys->aa[idx]       = c;
	thys->num[idx]      = num;
	thys->tcount[idx]   = tcount;
	thys->fraction[idx] =fraction;
    }

    ajStrAssC(&(thys->Name),ajStrStr(fn));

    ajStrDel(&t);
    ajStrDel(&line);
    ajFileClose(&inf);

    return ajTrue;
}




/* @func ajCodSetBacktranslate ************************************************
**
** Fill the codon usage object "back" element with the most commonly
** used triplet index for the amino acids
**
** @param [u] thys [AjPCod *] codon usage structure
**
** @return [void]
** @@
******************************************************************************/

void ajCodSetBacktranslate(AjPCod *thys)
{
    ajint i;
    ajint aa;
    double f;
    double f2;

    if(!*thys)
	ajFatal("Codon usage object uninitialised");

    for(i=0;i<AJCODAMINOS;++i)
	(*thys)->back[i] = -1;

    for(i=0;i<AJCODSTART;++i)
    {
	aa = (*thys)->aa[i];

	if((*thys)->back[aa]<0)
	    (*thys)->back[aa] = i;

	f = (*thys)->fraction[i];
	f2 = (*thys)->fraction[(*thys)->back[aa]];
	if(f>f2)
	    (*thys)->back[aa] = i;
    }

    return;
}




/* @func ajCodTriplet *********************************************************
**
** Convert triplet index to triple
**
** @param [r] idx [ajint] triplet index
**
** @return [char*] Triplet
** @@
******************************************************************************/

char* ajCodTriplet(ajint idx)
{
    static char ret[4] = "AAA";
    char *conv = "ACGT";

    char *p;

    p=ret;

    *p++ = conv[idx>>4];
    *p++ = conv[(idx & 0xf)>>2];
    *p   = conv[idx & 0x3];

    return ret;
}




/* @func ajCodWrite ***********************************************************
**
** Write codon structure to output file
**
** @param [r] thys  [const AjPCod]  codon usage
** @param [u] outf [AjPFile] output file
**
** @return [void]
** @category output [AjPCod] Write codon structure to output file
** @@
******************************************************************************/

void ajCodWrite(const AjPCod thys, AjPFile outf)
{
    ajint i;
    ajint j;


    for(i=0;i<AJCODAMINOS-2;++i)
	for(j=0;j<AJCODSTART;++j)
	    if(thys->aa[j]==i)
		ajFmtPrintF(outf,"%s\t%c\t\t%.3f\t%.3f\t%d\n",ajCodTriplet(j),
			    i+'A',thys->fraction[j],thys->tcount[j],
			    thys->num[j]);
    for(j=0;j<AJCODSTART;++j)
	if(thys->aa[j]==27)
	    ajFmtPrintF(outf,"%s\t*\t\t%.3f\t%.3f\t%d\n",ajCodTriplet(j),
			thys->fraction[j],thys->tcount[j],thys->num[j]);

    return;
}




/* @func ajCodComp ************************************************************
**
** Calculate sequence composition
**
** @param [w] NA [ajint *] number of A's
** @param [w] NC [ajint *] number of C's
** @param [w] NG [ajint *] number of G's
** @param [w] NT [ajint *] number of T'
** @param [r] str [const char *] sequence
**
** @return [void]
** @@
******************************************************************************/

void ajCodComp(ajint *NA, ajint *NC, ajint *NG, ajint *NT, const char *str)
{
    const char *p;
    ajint c;

    p = str;

    while((c = *p))
    {
	if(c=='A') ++(*NA);
	else if(c=='C') ++(*NC);
	else if(c=='G') ++(*NG);
	else if(c=='T') ++(*NT);
	++p;
    }

    return;
}




/* @funcstatic codRandom ****************************************************
**
** Calculate expected frequency of a codon
**
** @param [r] NA [ajint] number of A's
** @param [r] NC [ajint] number of C's
** @param [r] NG [ajint] number of G's
** @param [r] NT [ajint] number of T'
** @param [r] len [ajint] sequence length
** @param [r] p [const char *] triplet
**
** @return [double] triplet frequency
** @@
******************************************************************************/

static double codRandom(ajint NA, ajint NC, ajint NG, ajint NT,
			  ajint len, const char *p)
{
    ajint i;
    double prod;
    double tot;
    ajint c;


    prod = 1;
    tot  = 1.0;

    for(i=0;i<3;++i)
    {
	c = p[i];
	if(c=='A')
	    prod = (double)NA;
	if(c=='C')
	    prod = (double)NC;
	if(c=='G')
	    prod = (double)NG;
	if(c=='T')
	    prod = (double)NT;
	tot *= prod/(double)len;
    }

    return tot;
}




/* @func ajCodCalcCai *********************************************************
**
** Calculate codon adaptive index using equation 8
** NAR 15:1281-1295
**
** @param [r] thys [const AjPCod] codon usage
**
** @return [double] CAI
** @@
******************************************************************************/

double ajCodCalcCai(const AjPCod thys)
{
    double cai;
    double max;
    double sum;
    double res;
    double xij;
    double total;
    ajint  i;
    ajint  k;

    total = (double)0.;
    for(i=0;i<AJCODAMINOS-2;++i)
    {
	max = (double)0.;
	for(k=0;k<AJCODSTART;++k)
	{
	    if(thys->aa[k]==27)
		continue;
	    if(thys->aa[k]==i)
		max = (max> thys->fraction[k]) ? max : thys->fraction[k];
	}

	sum = (double)0.;
	for(k=0;k<AJCODSTART && max;++k)
	{
	    if(thys->aa[k]==27)
		continue;
	    if(thys->aa[k]==i)
	    {
		xij = thys->fraction[k];
		if(xij)
		{
		    res = thys->tcount[k] * log(xij/max);
		    sum += res;
		}
	    }
	}

	total += sum;
    }

    total /= (double)1000.;
    cai = exp(total);

    return cai;
}




/* @func ajCodCaiW ************************************************************
**
** Calculate codon adaptive index W values
** NAR 15:1281-1295
**
** @param [r] thys [const AjPCod] codon usage
**
** @return [double*] w value array
** @@
******************************************************************************/

double* ajCodCaiW(const AjPCod thys)
{
    ajint i;
    ajint j;
    double *wk;
    ajint thisaa;
    double aamax;

    AJCNEW0(wk,AJCODSTART);


    for(i=0;i<AJCODSTART;++i)
    {
	thisaa = thys->aa[i];
	aamax = (double)INT_MIN;

	for(j=0;j<AJCODSTART;++j)
	    if(thys->aa[j]==thisaa)
		aamax = aamax > thys->tcount[j] ? aamax : thys->tcount[j];
	if(aamax)
	    wk[i] = thys->tcount[i] / aamax;
    }

    return wk;
}




/* @func ajCodCai *************************************************************
**
** Calculate codon adaptive index using equation 7
** NAR 15:1281-1295
**
** @param [r] thys [const AjPCod] codon usage
** @param [r] str [const AjPStr] sequence
**
** @return [double] CAI
** @@
******************************************************************************/

double ajCodCai(const AjPCod thys, const AjPStr str)
{
    double *wk;
    ajint  i;
    ajint  len;
    const char   *p;
    ajint  limit;
    ajint  idx;
    double total;

    wk = ajCodCaiW(thys);

    len = ajStrLen(str);
    p   = ajStrStr(str);

    limit = len / 3;
    total = (double)0.;

    for(i=0;i<limit; ++i,p+=3)
    {
	idx = ajCodIndexC(p);
	if(wk[idx])
	    total += log(wk[idx]);
    }

    total /= (double)limit;

    AJFREE(wk);

    return exp(total);
}

/* @func ajCodGetName *********************************************************
**
** Returns the name of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Original filename
******************************************************************************/

const AjPStr ajCodGetName(const AjPCod thys)
{
    return thys->Name;
}

/* @func ajCodGetNameC ********************************************************
**
** Returns the name of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const char*] Original filename
******************************************************************************/

const char* ajCodGetNameC(const AjPCod thys)
{
    return ajStrStr(thys->Name);
}

/* @func ajCodGetDesc *********************************************************
**
** Returns the description of a codon table
**
** @param [r] thys [const AjPCod] Codon usage object
** @return [const AjPStr] Original filename
******************************************************************************/

const AjPStr ajCodGetDesc(const AjPCod thys)
{
    return thys->Desc;
}

/* @func ajCodGetDescC ********************************************************
**
** Returns the description of a codon table
**
** @param [r] thys [const AjPCod] Codon usgage object
** @return [const char*] Original filename
******************************************************************************/

const char* ajCodGetDescC(const AjPCod thys)
{
    return ajStrStr(thys->Desc);
}

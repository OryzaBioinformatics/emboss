/* @source embiep.c
**
** Acid/base routines
** Copyright (c) 1999 Alan Bleasby
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#define PKFILE "Epk.dat"

static  AjBool AjIEPinit=0;		/* Has Epk.dat file been read? */
static  double AjpK[EMBIEPSIZE];	/* pK values from Epk.dat      */


/* @func embIepPhToHConc  *************************************************
**
** Convert pH to hydrogen ion concontration
**
** @param [r] pH [double] pH
**
** @return [double] hydrogen ion concentrration
******************************************************************************/
double embIepPhToHConc(double pH)
{
    return pow(10.0,-pH);
}


/* @func embIepHConcToPh  *************************************************
**
** Convert hydrogen ion concontration to pH
**
** @param [r] H [double] H
**
** @return [double] pH
******************************************************************************/
double embIepHConcToPh(double H)
{
    return -log10(H);
}


/* @func embIepPkToK  *************************************************
**
** Convert pK to dissociation constant
**
** @param [r] pK [double] pK
**
** @return [double] dissociation constant
******************************************************************************/
double embIepPkToK(double pK)
{
    return pow(10.0,-pK);
}


/* @func embIePkTopK  *************************************************
**
** Convert dissociation constant to pK
**
** @param [r] K [double] K
**
** @return [double] pK
******************************************************************************/
double embIePkTopK(double K)
{
    return -log10(K);
}


/* @func embIepPkRead  *************************************************
**
** Read the pK values from Epk.dat
**
** @return [void]
******************************************************************************/
void embIepPkRead(void)
{
    AjPFile inf=NULL;
    AjPStr  line;
    char    *p;
    double  amino=8.6;
    double  carboxyl=3.6;
    char    ch;
    int     i;
    
    if(AjIEPinit) return;
    

    ajFileDataNewC(PKFILE,&inf);
    if(!inf)
	ajFatal("%s file not found",PKFILE);

    for(i=0;i<EMBIEPSIZE;++i)
	AjpK[i]=0.0;

    line=ajStrNew();
    while(ajFileGets(inf,&line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || *p=='\n')
	    continue;
	if(!ajStrNCmpC(line,"Amino",5))
	{
	    p=ajSysStrtok(p," \t\n");
	    p=ajSysStrtok(NULL," \t\n");
	    (void) sscanf(p,"%lf",&amino);
	    continue;
	}
	if(!ajStrNCmpC(line,"Carboxyl",8))
	{
	    p=ajSysStrtok(p," \t\n");
	    p=ajSysStrtok(NULL," \t\n");
	    (void) sscanf(p,"%lf",&carboxyl);
	    continue;
	}
	p=ajSysStrtok(p," \t\n");
	ch=ajSysItoC(toupper((int)*p));
	p=ajSysStrtok(NULL," \t\n");
	(void) sscanf(p,"%lf",&AjpK[ajAZToInt(ch)]);
    }

    AjpK[EMBIEPAMINO]=amino;
    AjpK[EMBIEPCARBOXYL]=carboxyl;

    AjIEPinit=ajTrue;
    ajStrDel(&line);
    ajFileClose(&inf);
    
    return;
}


/* @func embIepComp  *************************************************
**
** Calculate the amino acid composition of a protein sequence
**
** @param [r] s [char *] protein sequence
** @param [r] amino [int] number of amino termini
** @param [w] c [int *] amino acid composition
**
** @return [void]
******************************************************************************/
void embIepComp(char *s, int amino, int *c)
{
    int i;
    char *p;
    
    for(i=0;i<EMBIEPSIZE;++i)
	c[i]=0;

    p=s;
    while(*p)
    {
	++c[ajAZToInt(ajSysItoC(toupper((int)*p)))];
	++p;
    }
    
    c[EMBIEPAMINO]=amino;
    c[EMBIEPCARBOXYL]=1;
    return;
}


/* @func embIepCalcK  *************************************************
**
** Calculate the dissociation constants
** Amino acids for which there is no entry in Epk.dat have K set to 0.0
**
** @param [w] K [double *] dissociation constants
**
** @return [void]
******************************************************************************/
void embIepCalcK(double *K)
{
    int i;
    
    if(!AjIEPinit)
	embIepPkRead();

    for(i=0;i<EMBIEPSIZE;++i)
	if(!AjpK[i])
	    K[i]=0.0;
	else
	    K[i]=embIepPkToK(AjpK[i]);
    return;
}


/* @func embIepGetProto  *************************************************
**
** Calculate the number of H+ bound
** Amino acids for which there is no entry in Epk.dat have this set to 0.0
**
** @param [r] K [double *] dissociation constants
** @param [r] c [int *] sequence composition
** @param [w] op [int *] printout flags
** @param [r] H [double] hydrogen ion concentration
** @param [w] pro [double *] number of protons bound
**
** @return [void]
******************************************************************************/

void embIepGetProto(double *K, int *c, int *op, double H, double *pro)
{
    int i;

    for(i=0;i<EMBIEPSIZE;++i)
	if(!K[i])
	{
	    pro[i]=0.0;
	    op[i]=0;
	}
	else
	{
	    if(!c[i])
		op[i]=0;
	    else
		op[i]=1;
	    pro[i]=(double)c[i] * (H/(H+K[i]));
	}

    return;
}


/* @func embIepGetCharge  *************************************************
**
** Calculate the number of H+ bound
**
** @param [r] c [int *] sequence composition
** @param [r] pro [double *] number of protons
** @param [w] total [double *] total protons
**
** @return [double] charge
******************************************************************************/

double embIepGetCharge(int *c, double *pro, double *total)
{
    int i;
    double C;
    
    for(i=0,*total=0.0;i<EMBIEPSIZE;++i)
	*total += pro[i];

    C = (pro[10]+pro[17]+pro[7]+pro[EMBIEPAMINO]) -
	((double)c[24]-pro[24] +
	 (double)c[2]-pro[2] +
	 (double)c[3]-pro[3] +
	 (double)c[4]-pro[4] +
	 (double)c[EMBIEPCARBOXYL]-pro[EMBIEPCARBOXYL] );

    return C;
}




/* @func embIepPhConverge  *************************************************
**
** Calculate the pH nearest the IEP or return 0.0 if one doesn't exist
**
** @param [r] c [int *] sequence composition
** @param [r] K [double *] sequence dissociation constants
** @param [w] op [int *] printout flags
** @param [w] pro [double *] number of protons
**
** @return [double] IEP or 0.0
******************************************************************************/

double embIepPhConverge(int *c, double *K, int *op, double *pro)
{
    double sum=0.0;
    double charge;
    double top;
    double mid;
    double bot;
    double H;
    double tph=1.0;
    double bph=14.0;

    H=embIepPhToHConc(tph);
    embIepGetProto(K,c,op,H,pro);
    top=embIepGetCharge(c,pro,&sum);
    H=embIepPhToHConc(bph);
    embIepGetProto(K,c,op,H,pro);
    bot=embIepGetCharge(c,pro,&sum);
    if((top>0.0 && bot>0.0) || (top<0.0 && bot<0.0))
	return 0.0;

    while(bph-tph>0.0001)
    {
	mid = ((bph-tph) / 2.0) + tph;
	H=embIepPhToHConc(mid);
	embIepGetProto(K,c,op,H,pro);
	charge=embIepGetCharge(c,pro,&sum);
	if(charge>0.0)
	{
	    tph=mid;
	    continue;
	}
	if(charge<0.0)
	{
	    bph=mid;
	    continue;
	}
	else
	    return mid;
    }

    return tph;
}

	

/* @func embIepIEP  *************************************************
**
** Calculate the pH nearest the IEP.
**
** @param [r] s [char *] sequence
** @param [r] amino [int] number of N-termini
** @param [w] iep [double *] IEP
** @param [r] termini [AjBool] use termini
**
** @return [AjBool] True if IEP exists
******************************************************************************/
AjBool embIepIEP(char *s, int amino, double *iep, AjBool termini)
{
    int *c=NULL;
    int *op=NULL;
    double *K=NULL;
    double *pro=NULL;
    
    *iep=0.0;

    AJCNEW (c,   EMBIEPSIZE);
    AJCNEW (op,  EMBIEPSIZE);
    AJCNEW (K,   EMBIEPSIZE);
    AJCNEW (pro, EMBIEPSIZE);

    embIepPkRead();			/* read pK's */
    embIepCalcK(K);			/* Convert to dissoc consts */
    embIepComp(s,amino,c);		/* Get sequence composition */
    if(!termini)
	c[EMBIEPAMINO]=c[EMBIEPCARBOXYL]=0;
    *iep=embIepPhConverge(c,K,op,pro);

    AJFREE (pro);
    AJFREE (K);
    AJFREE (op);
    AJFREE (c);

    if(!*iep)
	return ajFalse;

    return ajTrue;
}

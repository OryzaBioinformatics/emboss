/********************************************************************
** @source AJAX nucleic acid functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0 
** @modified Feb 25 ajb First version
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
#include <math.h>
#include <string.h>
#include <ctype.h>

#define DNAMELTFILE "Edna.melt"
#define RNAMELTFILE "Erna.melt"
#define MAXMELTSAVE 10000

AjMelt aj_m_table[256];
AjBool aj_melt_I = 0;

ajint aj_melt_savesize=0;
AjBool aj_melt_saveinit=0;
AjBool aj_melt_saveshift=1;




/* @func ajMeltInit *********************************************************
**
** Initialises melt entropies, enthalpies and energies. Different data
** files are read for DNA or RNA heteroduplex. Also sets optional flag
** for array saving of the above.
**
** @param  [rP] type [AjPStr*] Pointer to a "dna" or "rna" string
** @param  [r]  savesize [ajint] Size of array to save, or zero if none
** @return [void] Number of energies to save
******************************************************************************/

void ajMeltInit(AjPStr *type, ajint savesize)
{
    AjPFile mfptr;
    AjPStr  mfname=NULL;

    AjPStr  pair=NULL;
    AjPStr  pair1=NULL;
    AjPStr  pair2=NULL;
    AjPStr  acgt=NULL;
    AjPStr  line=NULL;
    
    ajint i,j,k;
    char *p;
    char *q;
    float enthalpy, entropy, energy;

    AjBool got1;
    AjBool got2;
    

    aj_melt_savesize = savesize;
    aj_melt_saveinit = ajFalse;
    
    if(aj_melt_I) return;
    

    mfname = ajStrNew();
    
    if(!ajStrCmpC(*type, "rna"))
    {
	(void) ajStrSetC(&mfname,RNAMELTFILE);
	ajFileDataNew(mfname, &mfptr);
    }
    if(!ajStrCmpC(*type, "dna"))
    {
	(void) ajStrSetC(&mfname,DNAMELTFILE);
	ajFileDataNew(mfname, &mfptr);
    }
    if(!mfptr) ajFatal("Entropy/enthalpy/energy file not found\n");

    pair=ajStrNew();
    pair1=ajStrNew();
    pair2=ajStrNew();
    acgt=ajStrNew();
    line=ajStrNew();
    
    (void) ajStrAssC(&pair,"AA");
    (void) ajStrAssC(&acgt,"ACGT");
    p=ajStrStr(pair);
    q=ajStrStr(acgt);
    
    for(i=0,k=0;i<4;++i)
    {
	*p = *(q+i);
	for(j=0;j<4;++j)
	{
	    *(p+1) = *(q+j);
	    aj_m_table[k++].pair = ajStrNewC(p);
	}
    }


    while(ajFileGets(mfptr, &line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || !*p) continue;

	p=ajSysStrtok(p," \t\n\r");
	(void) ajStrAssC(&pair1,p);
	p=ajSysStrtok(NULL," \t\n\r");
	(void) ajStrAssC(&pair2,p);
	p=ajSysStrtok(NULL," \t\n\r");
	if(sscanf(p,"%f",&enthalpy)!=1)
	    ajFatal("No enthalpy found");
	p=ajSysStrtok(NULL," \t\n\r");
	if(sscanf(p,"%f",&entropy)!=1)
	    ajFatal("No entropy found");
	p=ajSysStrtok(NULL," \t\n\r");
	if(sscanf(p,"%f",&energy)!=1)
	    ajFatal("No energy found");
	
	got1 = got2 = ajFalse;
	
	for(k=0;k<16;++k)
	{
	    if(!ajStrCmpO(aj_m_table[k].pair,pair1))
	    {
		aj_m_table[k].enthalpy = enthalpy;
		aj_m_table[k].entropy  = entropy;
		aj_m_table[k].energy   = energy;
		got1 = ajTrue;
	    }
	}
	
	for(k=0;k<16;++k)
	{
	    if(!ajStrCmpO(aj_m_table[k].pair,pair2))
	    {
		aj_m_table[k].enthalpy = enthalpy;
		aj_m_table[k].entropy  = entropy;
		aj_m_table[k].energy   = energy;
		got2 = ajTrue;
	    }
	}
	
	if(!got1 || !got2)
	    ajFatal("ajMeltInit data error");
    }



    
    ajStrDel(&mfname);
    ajStrDel(&pair);
    ajStrDel(&pair1);
    ajStrDel(&pair2);
    ajStrDel(&acgt);
    ajStrDel(&line);

    ajFileClose(&mfptr);

    aj_melt_I = ajTrue;
}





/* @func ajProbScore *********************************************************
**
** Gives a score for the probability of two sequences being the same.
** The sequences are the same length.
** Uses UB ambiguity codes. The result is the sum of the probabilities
** at each position.
**
** @param  [rP] seq1 [AjPStr*] Pointer to a sequence string
** @param  [rP] seq2 [AjPStr*] Pointer to a another sequence
** @param  [r]  len [ajint] Length of sequences
** @return [float] Match probability
******************************************************************************/

float ajProbScore(AjPStr *seq1, AjPStr *seq2, ajint len)
{
    ajint mlen;
    float score;
    ajint i;
    ajint x;
    ajint y;
    char *p;
    char *q;
    

    mlen = (ajStrLen(*seq1) < ajStrLen(*seq2)) ? ajStrLen(*seq1) :
	ajStrLen(*seq2);

    if(len > 0)
	mlen = (mlen < len) ? mlen : len;

    if(!aj_base_I) ajBaseInit();
    score = 0.0;
    if(!mlen) return score;

    score = 1.0;
    p = ajStrStr(*seq1);
    q = ajStrStr(*seq2);
    
    for(i=0; i<mlen; ++i)
    {
	x = ajAZToInt(*(p+i));
	y = ajAZToInt(*(q+i));
	score *= aj_base_prob[x][y];
    }

    return score;
}






/* @func ajMeltEnergy *******************************************************
**
** Calculates melt energy and enthalpy/entropy for a sequence string.
** An optional shift is given for stepping along the sequence and loading
** up energy arrays.
**
** @param  [rP] strand [AjPStr*] Pointer to a sequence string
** @param  [r] len [ajint] Length of sequence
** @param  [r] shift [ajint] Stepping value
** @param  [r] isDNA [AjBool] DNA or RNA
** @param  [r] maySave [AjBool] May use the save arrays for speedup
** @param  [w] enthalpy [float*] enthalpy
** @param  [w] entropy [float*] entropy
**
** @return [float] Melt energy
******************************************************************************/

float ajMeltEnergy(AjPStr *strand, ajint len, ajint shift, AjBool isDNA,
		   AjBool maySave, float *enthalpy, float *entropy)
{
    AjPStr fname;
    AjPStr line;
    ajint i;
    ajint j;
    ajint k;
    ajint ipos;
    char *p;
    static float energy;
    float ident;
    AjBool doShift;

    static float saveEnthalpy[MAXMELTSAVE];
    static float saveEntropy[MAXMELTSAVE];
    static float saveEnergy[MAXMELTSAVE];


    if(!aj_melt_I)
    {
	if(isDNA)
	{
	    fname = ajStrNewC("dna");
	    ajMeltInit(&fname,len);
	}
	else
	{
	    fname = ajStrNewC("rna");
	    ajMeltInit(&fname,len);
	}
	ajStrDel(&fname);
    }

    if (maySave == ajFalse) aj_melt_saveshift = ajFalse;

    doShift = (aj_melt_saveshift && aj_melt_savesize > 0) ? ajTrue : ajFalse;


    ipos = 0;
    
    if(doShift)
    {
	if(!aj_melt_saveinit)
	{
	    ipos = 0;
	    for(i=0;i<aj_melt_savesize;++i)
		saveEnergy[i]=saveEntropy[i]=saveEnthalpy[i]=0.0;
	    energy = *entropy = *enthalpy = 0.0;
	    aj_melt_saveinit = ajTrue;
	}
	else
	{
	    ipos = (len - shift) - 1;
	    for(i=0;i<shift;++i)
	    {
		energy -= saveEnergy[i];
		*entropy -= saveEntropy[i];
		*enthalpy -= saveEnthalpy[i];
	    }

	    for(i=0,k=shift; k < aj_melt_savesize; ++i, ++k)
	    {
		saveEnergy[i]=saveEnergy[k];
		saveEntropy[i]=saveEntropy[k];
		saveEnthalpy[i]=saveEnthalpy[k];
	    }
	}
    }
    else 
    { 
        ipos=0;
	energy = *entropy = *enthalpy = 0.0;
    }

    line = ajStrNew();
    p = ajStrStr(*strand);
    
    while(ipos < len-1)
    {
	if(doShift)
	{
	    saveEnthalpy[ipos]=0.0;
	    saveEntropy[ipos]=0.0;
	    saveEnergy[ipos]=0.0;
	}

	for(j=0;j<16;++j)
	{
	    (void) ajStrAssSubC(&line,p+ipos,0,1);
	    ident = ajProbScore(&aj_m_table[j].pair, &line, 2);
	    
	    if(ident>0.9)
	    {
		if(doShift)
		{
		    saveEnergy[ipos] += (ident * aj_m_table[j].energy);
		    saveEntropy[ipos] += (ident * aj_m_table[j].entropy);
		    saveEnthalpy[ipos] += (ident * aj_m_table[j].enthalpy);
		}
		else
		{
		    energy += (ident * aj_m_table[j].energy);
		    *entropy += (ident * aj_m_table[j].entropy);
		    *enthalpy += (ident * aj_m_table[j].enthalpy);
		}
	    }
	}


	if(doShift)
	{
	    energy += saveEnergy[ipos];
	    *entropy += saveEntropy[ipos];
	    *enthalpy += saveEnthalpy[ipos];
	}
	++ipos;
    }

    ajStrDel(&line);

    return energy;
}




/* @func ajTm *******************************************************
**
** Calculates melt temperature of DNA or RNA
** An optional shift is given for stepping along the sequence and loading
** up energy arrays.
**
** @param  [rP] strand [AjPStr*] Pointer to a sequence string
** @param  [r] len [ajint] Length of sequence
** @param  [r] shift [ajint] Stepping value
** @param  [r] saltconc [float] mM salt concentration
** @param  [r] DNAconc [float] nM DNA concentration
** @param  [r] isDNA [AjBool] DNA or RNA
**
** @return [float] Melt temperature
******************************************************************************/

float ajTm(AjPStr *strand, ajint len, ajint shift, float saltconc,
	      float DNAconc, AjBool isDNA)
{
    double entropy;
    double enthalpy;
    double dTm;
    float Tm;
    static float sumEntropy;
    static float sumEnthalpy;
    float To;
    float R;
    double LogDNA;
    /*    double LogSalt;*/


    /* LogSalt = 16.6 * (float) (log10((double) (saltconc/1000.0)));*/    /* mM */
    R = 1.987;			/* molar gas constant (cal/c * mol)        */
    LogDNA = R * (float)log((double)(DNAconc/4000000000.0));	     /* nM */
    To = 273.15;
    
    (void) ajMeltEnergy(strand, len, shift, isDNA, ajFalse, &sumEnthalpy,
			&sumEntropy);

    entropy = -10.8 - sumEntropy;
    entropy += (len-1) * (log10((double) (saltconc/1000.0))) *
	(float) 0.368;



    enthalpy = -sumEnthalpy;

    dTm = ((enthalpy*1000.0) / (entropy+LogDNA)) /*+ LogSalt*/ - To;
    Tm = (float) dTm;  /* slight loss of precision here but no matter */
    
    return Tm;
}






/* @func ajMeltGC *******************************************************
**
** Calculates GC fraction of a sequence allowing for ambiguity
**
** @param  [rP] strand [AjPStr*] Pointer to a sequence string
** @param  [r] len [ajint] Length of sequence
**
** @return [float] GC fraction
******************************************************************************/

float ajMeltGC(AjPStr *strand, ajint len)
{
    ajint t;
    ajint i;
    char *p;
    double count;
    
    p=ajStrStr(*strand);
    count = 0.0;
    
    for(i=0;i<len;++i)
    {
	t = toupper((ajint) *(p+i));
	if(strchr("GCS",t)) ++count;
	else if(strchr("ATUW",t)) count += 0.0;
	else if(strchr("RYMK",t)) count += 0.5;
	else if(strchr("NX",t))   count += 0.5;
	else if(strchr("BV",t))   count += 0.6666667;
	else if(strchr("DH",t))   count += 0.3333333;
    }

    return ((float)(count/(double)len));
}



/* @func ajMeltEnergy2 *******************************************************
**
** Calculates melt energy for use with programs like prima
**
** Giving this routine the complete sequence on the first call and
** setting aj_melt_I to false will initialise the energy, entropy
** and enthalpy arrays. Subsequent calls will not look at the
** sequence directly.
**
** @param  [r] strand [char *] Pointer to a sequence string
** @param  [r] pos [ajint] Position within sequence
** @param  [r] len [ajint] Length of sequence segment
** @param  [r] isDNA [AjBool] true if dna
** @param  [w] enthalpy [float *] calculated enthalpy
** @param  [w] entropy [float *] calculated entropy
** @param  [w] saveentr [float **] entropy save array
** @param  [w] saveenth [float **] enthalpy save array
** @param  [w] saveener [float **] energy save array
**
** @return [float] melt energy
******************************************************************************/

float ajMeltEnergy2(char *strand, ajint pos, ajint len, AjBool isDNA,
		    float *enthalpy, float *entropy, float **saveentr,
		    float **saveenth, float **saveener)
{
    AjPStr fname=NULL;

    ajint i;
    ajint j;
    
    ajint limit=0;
    AjPStr line=NULL;
    float ident=0.0;
    float energy;



    limit=len-1;
    
    if(!aj_melt_I)
    {
	if(isDNA)
	{
	    fname=ajStrNewC("dna");
	    ajMeltInit(&fname,len);
	}
	else
	{
	    fname=ajStrNewC("rna");
	    ajMeltInit(&fname,len);
	}
	ajStrDel(&fname);
	
	AJCNEW0 (*saveentr, limit);
	AJCNEW0 (*saveenth, limit);
	AJCNEW0 (*saveener, limit);

	line=ajStrNew();
	
	for(i=0;i<limit;++i)
	{
	    for(j=0;j<16;++j)
	    {
		(void) ajStrAssSubC(&line,strand+i,0,1);
		ident = ajProbScore(&aj_m_table[j].pair,&line,2);
		if(ident>.9)
		{
		    (*saveentr)[i] += (ident * aj_m_table[j].entropy);
		    (*saveenth)[i] += (ident * aj_m_table[j].enthalpy);
		    (*saveener)[i] += (ident * aj_m_table[j].energy);
		}
	    }
	}

	ajStrDel(&line);
	aj_melt_I = ajTrue;
    }

    energy=*enthalpy=*entropy=(float)0.0;

    
    for(i=0;i<limit;++i)
    {
	energy += (*saveener)[pos+i];
	*entropy += (*saveentr)[pos+i];
	*enthalpy += (*saveenth)[pos+i];
    }

    return energy;
}




/* @func ajTm2 *******************************************************
**
** Calculates melt temperature of DNA or RNA
**
** @param  [r] strand [char *] Pointer to a sequence string
** @param  [r] pos [ajint] position within sequence
** @param  [r] len [ajint] Length of sequence (segment)
** @param  [r] saltconc [float] mM salt concentration
** @param  [r] DNAconc [float] nM DNA concentration
** @param  [r] isDNA [AjBool] DNA or RNA
**
** @return [float] Melt temperature
******************************************************************************/

float ajTm2(char *strand, ajint pos, ajint len, float saltconc,
	      float DNAconc, AjBool isDNA)
{
    static float *saveentr;
    static float *saveenth;
    static float *saveener;
    double entropy;
    double enthalpy;
    double dTm;
    float Tm;
    static float sumEntropy;
    static float sumEnthalpy;
    float To;
    float R;
    double LogDNA;
    /* double LogSalt;*/


    /* LogSalt = 16.6 * (float) (log10((double) (saltconc/1000.0))); */    /* mM */
    R = 1.987;			/* molar gas constant (cal/c * mol)        */
    LogDNA = R * (float)log((double)(DNAconc/4000000000.0));	     /* nM */
    To = 273.15;
    
    (void) ajMeltEnergy2(strand, pos, len, isDNA, &sumEnthalpy,
			 &sumEntropy, &saveentr, &saveenth, &saveener);

    entropy = -10.8 - sumEntropy;

    /* Added for santalucia */
    entropy += (len-1) * (log10((double) (saltconc/1000.0))) *
	(float) 0.368;


    enthalpy = -sumEnthalpy;

    /* logsalt removed for santalucia */
    dTm = ((enthalpy*1000.0) / (entropy+LogDNA)) + /*LogSalt*/ - To;
    Tm = (float) dTm;  /* slight loss of precision here but no matter */

    return Tm;
}




/* @func ajProdTm *******************************************************
**
** Calculates product melt temperature of DNA
**
** @param  [r] gc [float] GC percentage
** @param  [r] saltconc [float] mM salt concentration
** @param  [r] len [ajint] Length of sequence (segment)
**
** @return [float] Melt temperature
******************************************************************************/

float ajProdTm(float gc, float saltconc, ajint len)
{
    float ptm;
    float LogSalt;
    

    LogSalt = (float)16.6 * (float) (log10((double) (saltconc/1000.0)));

    ptm = (float)81.5 - (float)(675/len) + LogSalt + ((float)0.41 * gc);
    
    return ptm;
}



/* @func ajAnneal *******************************************************
**
** Calculates annealing temperature of product and primer
**
** @param  [r] tmprimer [float] primer Tm
** @param  [r] tmproduct [float] product Tm
**
** @return [float] Annealing temperature
******************************************************************************/

float ajAnneal(float tmprimer, float tmproduct)
{
    return ((float).7*tmproduct)-(float)14.9+((float).3*tmprimer);
}

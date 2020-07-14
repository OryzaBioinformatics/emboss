/* @source embprop.c
**
** Residue/sequence properties
** @author Copyright (c) 1999 Alan Bleasby
** @version 1.0
** @modified 24 Nov 1999 - GWW - Added embPropProtGaps and embPropProt1to3
** @modified 1 Sept 2000 - GWW - Added embPropTransition embPropTranversion
** @modified 4 July 2001 - DMAM - Modified embPropAminoRead embPropCalcMolwt
** @modified 4 July 2001 - DMAM - Added embPropCalcMolwtMod
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
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>




#define PROPENZTRYPSIN 0
#define PROPENZLYSC    1
#define PROPENZARGC    2
#define PROPENZASPN    3
#define PROPENZV8B     4
#define PROPENZV8P     5
#define PROPENZCHYMOT  6
#define PROPENZCNBR    7

#define AMINODATFILE "Eamino.dat"

static AjBool propInit = 0;

double *EmbPropTable[EMBPROPSIZE];

static char propPurines[]     = "agrAGR";
static char propPyrimidines[] = "ctuyCTUY";




static ajint propFragCompare(const void *a, const void *b);




/* @func embPropAminoRead *****************************************************
**
** Read amino acid properties from Eamino.dat
**
** @param [r] mfptr [AjPFile] Input file object
** @return [void]
** @@
******************************************************************************/

void embPropAminoRead(AjPFile mfptr)
{
    AjPStr  line  = NULL;
    AjPStr  delim = NULL;

    char *p;

    ajint cols = 0;

    if(propInit)
	return;

    line  = ajStrNew();
    delim = ajStrNewC(" :\t\n");

    while(ajFileGets(mfptr, &line))
    {
	p = ajStrStr(line);
	if(*p=='#' || *p=='!' || !*p)
	    continue;

	while(*p && (*p<'A' || *p>'Z'))
	    ++p;

	cols = ajStrTokenCount(&line,ajStrStr(delim));
	EmbPropTable[ajAZToInt(toupper((ajint)*p))] =
	    ajArrDoubleLine(&line,ajStrStr(delim),cols,2,cols);
    }


    ajStrDel(&line);
    ajStrDel(&delim);

    propInit = ajTrue;

    return;
}




/* @func embPropCalcMolwt  ****************************************************
**
** Calculate the molecular weight of a protein sequence
** This is a shell around embPropCalcMolwtMod using water as the modifier.
**
** @param [r] s [char *] sequence
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
**
** @return [double] molecular weight
** @@
******************************************************************************/

double embPropCalcMolwt(char *s, ajint start, ajint end)
{
    return embPropCalcMolwtMod(s,start,end,EMBPROPMSTN_H, EMBPROPMSTC_OH );
}




/* @func embPropCalcMolwtMod  *************************************************
**
** Calculate the molecular weight of a protein sequence
** with chemically modified termini
**
** @param [r] s [char *] sequence
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
** @param [r] nmass [double] mass of the N-terminal group
** @param [r] cmass [double] mass of the C-terminal group
**
** @return [double] molecular weight
** @@
******************************************************************************/

double embPropCalcMolwtMod(char *s, ajint start, ajint end, double nmass,
			   double cmass)
{
    char *p;
    double sum;
    ajint i;
    ajint len;

    if(!propInit)
      ajFatal("Amino Acid data not initialised. Call embPropAminoRead");

    len = end-start+1;

    p = s+start;
    sum = 0.0;

    for(i=0;i<len;++i)
	sum += EmbPropTable[ajAZToInt(toupper((ajint)p[i]))][EMBPROPMOLWT];

    return sum + nmass + cmass;
}




/* @func embPropCalcMolextcoeff*********************************************
**
** Calculate the molecular extinction coefficient of a protein sequence
**
** @param [r] s [char *] sequence
** @param [r] start [ajint] start position
** @param [r] end [ajint] end position
**
** @return [double] molar extinction coefficient
** @@
******************************************************************************/

double embPropCalcMolextcoeff(char *s, ajint start, ajint end)
{

    char *p;
    double sum;
    ajint i;
    ajint len;
    
    if(!propInit)
	ajFatal("Amino Acid data not initialised. Call embPropAminoRead");

    len = end-start+1;
    
    p   = s+start;
    sum = 0.0;

    for(i=0;i<len;++i)
	sum += EmbPropTable[ajAZToInt(toupper((ajint)p[i]))]
	    [EMBPROPABSORBANCE];

    return sum;
}




/* @func embPropCharToThree  **************************************************
**
** Return 3 letter amino acid code A=Ala B=Asx C=Cys etc
**
** @param [r] c [char] integer code
**
** @return [char*] three letter amino acid code
** @@
******************************************************************************/

char* embPropCharToThree(char c)
{
    return embPropIntToThree(ajAZToInt(c));
}




/* @func embPropIntToThree  ***************************************************
**
** Return 3 letter amino acid code 0=Ala 1=Asx 2=Cys etc
**
** @param [r] c [ajint] integer code
**
** @return [char*] three letter amino acid code
** @@
******************************************************************************/

char* embPropIntToThree(ajint c)
{
    static char *tab[]=
    {
	"Ala","Asx","Cys","Asp","Glu","Phe","Gly","His","Ile","---","Lys",
	"Leu","Met","Asn","---","Pro","Gln","Arg","Ser","Thr","---",
	"Val","Trp","Xaa","Tyr","Glx"
    };

    return tab[c];
}




/* @func embPropCalcFragments  ************************************************
**
** Read amino acd properties
**
** @param [r] s [char *] sequence
** @param [r] n [ajint] "enzyme" number
** @param [r] begin [ajint] sequence offset
** @param [w] l [AjPList *] list to push hits to
** @param [w] pa [AjPList *] list to push partial hits to
** @param [r] unfavoured [AjBool] allow unfavoured cuts
** @param [r] overlap [AjBool] show overlapping partials
** @param [r] allpartials [AjBool] show all possible partials
** @param [w] ncomp [ajint *] number of complete digest fragments
** @param [w] npart [ajint *] number of partial digest fragments
** @param [w] rname [AjPStr *] name of reagent
**
** @return [void]
** @@
******************************************************************************/

void embPropCalcFragments(char *s, ajint n, ajint begin,
			  AjPList *l, AjPList *pa,
			  AjBool unfavoured, AjBool overlap,
			  AjBool allpartials, ajint *ncomp, ajint *npart,
			  AjPStr *rname)
{
    static char *PROPENZReagent[]=
    {
	"Trypsin","Lys-C","Arg-C","Asp-N","V8-bicarb","V8-phosph",
	"Chymotrypsin","CNBr"
    };

    static char *PROPENZSite[]=
    {
	"KR","K","R","D","E","DE","FYWLM","M"
    };

    static char *PROPENZAminoCarboxyl[]=
    {
	"CC","C","C","N","C","CC","CCCCC","C"
    };

    static char *PROPENZUnfavoured[]=
    {
	"KRIFLP","P","P","","KREP","P","P",""
    };


    ajint i;
    ajint j;
    ajint lim;
    ajint len;
    AjPList t;
    EmbPPropFrag fr;
    ajint *begsa = NULL;
    ajint *endsa = NULL;
    double molwt;
    double *molwtsa = NULL;
    AjBool *afrag   = NULL;
    ajint mark;
    ajint bwp;
    ajint ewp;
    ajint *ival;
    ajint defcnt;


    ajStrAssC(rname,PROPENZReagent[n]);
    defcnt = 0;
    len = strlen(s);

    t = ajListNew();			/* Temporary list */


    /* First get all potential cut points */
    for(i=0;i<len;++i)
    {
	if(!strchr(PROPENZSite[n],s[i]))
	    continue;

	if(len==i+1)
	    continue;

	if(strchr(PROPENZUnfavoured[n],s[i+1])
	   && !unfavoured)
	    continue;

	AJNEW0(ival);
	*ival = i;
	ajListPushApp(t,(void *)ival);
	++defcnt;
    }

    if(defcnt)
    {
	AJCNEW(begsa,(defcnt+1));
	AJCNEW(endsa,(defcnt+1));
	AJCNEW(molwtsa,(defcnt+1));
	AJCNEW(afrag,(defcnt+1));
    }

    for(i=0;i<defcnt;++i)  /* Pop them into a temporary array 	 */
    {
	ajListPop(t,(void **)&ival);
	endsa[i] = *ival;
	AJFREE(ival);
    }


    mark = 0;
    for(i=0;i<defcnt;++i)  /* Work out true starts, ends and molwts */
    {
	bwp = mark;
	ewp = endsa[i];
	if(strchr(PROPENZAminoCarboxyl[n],'N'))
	    --ewp;
	molwt=embPropCalcMolwt(s,bwp,ewp);

	if(n==PROPENZCNBR)
	    molwt -= (17.045 + 31.095);

	begsa[i]   = mark;
	endsa[i]   = ewp;
	molwtsa[i] = molwt;
	afrag[i]   = ajFalse;
	mark       = ewp+1;
    }

    if(defcnt)		   /* Special treatment for last fragment   */
    {
	molwt = embPropCalcMolwt(s,mark,len-1);
	if(n==PROPENZCNBR)
	    molwt -= (17.045 + 31.095);
	begsa[i]   = mark;
	endsa[i]   = len-1;
	molwtsa[i] = molwt;
	afrag[i]   = ajFalse;
	++defcnt;
    }

    /* Push the hits */
    for(i=0;i<defcnt;++i)
    {
	AJNEW0(fr);
	fr->start  = begsa[i];
	fr->end    = endsa[i];
	fr->molwt  = molwtsa[i];
	fr->isfrag = afrag[i];
	ajListPush(*l,(void *) fr);
    }

    ajListSort(*l,propFragCompare);
    *ncomp = defcnt;


    /* Now deal with overlaps */
    *npart = 0;

    lim = defcnt -1;
    if(overlap && !allpartials)
    {
	for(i=0;i<lim;++i)
	{
	    AJNEW0(fr);
	    fr->isfrag = ajTrue;
	    fr->molwt = embPropCalcMolwt(s,begsa[i],endsa[i+1]);
	    if(n==PROPENZCNBR)
		fr->molwt -= (17.045 + 31.095);
	    fr->start = begsa[i];
	    fr->end   = endsa[i+1];
	    ajListPush(*pa,(void *)fr);
	    ++(*npart);
	}

	if(*npart)			/* Remove complete sequence */
	{
	    --(*npart);
	    ajListPop(*pa,(void **)&fr);
	}
	ajListSort(*pa,propFragCompare);
    }

    if(allpartials)
    {
	for(i=0;i<lim;++i)
	    for(j=i+1;j<lim;++j)
	    {
		AJNEW0(fr);
		fr->isfrag = ajTrue;
		fr->molwt = embPropCalcMolwt(s,begsa[i],endsa[j]);
		if(n==PROPENZCNBR)
		    fr->molwt -= (17.045 + 31.095);
		fr->start = begsa[i];
		fr->end   = endsa[j];
		ajListPush(*pa,(void *)fr);
		++(*npart);
	    }

	if(*npart)			/* Remove complete sequence */
	{
	    --(*npart);
	    ajListPop(*pa,(void **)&fr);
	}
	ajListSort(*pa,propFragCompare);
    }

    if(defcnt)
    {
	AJFREE(molwtsa);
	AJFREE(endsa);
	AJFREE(begsa);
	AJFREE(afrag);
    }

    ajListFree(&t);

    return;
}




/* @funcstatic propFragCompare  ***********************************************
**
** compare two molecular weight AjPFrag list elements for sorting
**
** @param [r] a [const void*] First element
** @param [r] b [const void*] Second element
**
** @return [ajint] 0=equal +ve=(a greater than b) -ve=(a less than b)
** @@
******************************************************************************/

static ajint propFragCompare(const void *a, const void *b)
{
    return (ajint)((*(EmbPPropFrag *)b)->molwt - (*(EmbPPropFrag *)a)->molwt);
}




/* @func embPropProtGaps ******************************************************
**
** Creates a string of a protein sequence which has been padded out with
** two spaces after every residue to aid aligning a translation under a
** nucleic sequence
**
** @param [u] seq [AjPSeq] protein sequence to add spaces into
** @param [r] pad [ajint] number of spaces to insert at the start of the result
** @return [AjPStr] New string with the padded sequence
** @@
******************************************************************************/

AjPStr embPropProtGaps(AjPSeq seq, ajint pad)
{
    char *p;
    AjPStr temp;
    ajint i;

    temp = ajStrNewL(ajSeqLen(seq)*3 + pad+1);

    /* put any required padding spaces at the start */
    for(i=0; i<pad; i++)
	ajStrAppC(&temp, " ");


    for(p=ajSeqChar(seq); *p; p++)
    {
	ajStrAppK(&temp, *p);
	ajStrAppC(&temp, "  ");
    }

    return temp;
}




/* @func embPropProt1to3 ******************************************************
**
** Creates a a 3-letter sequence protein string from single-letter sequence
** EMBOSS is bad at reading 3-letter sequences, but this may be useful
** when displaying translations.
**
** @param [u] seq [AjPSeq] protein sequence to convert to 3-letter codes
** @param [r] pad [ajint] number of spaces to insert at the start of the result
** @return [AjPStr] string containing 3-letter protein sequence
** @@
******************************************************************************/

AjPStr embPropProt1to3(AjPSeq seq, ajint pad)
{
    char *p;
    char *p3;
    AjPStr temp;
    ajint i;

    temp = ajStrNewL(ajSeqLen(seq)*3 + pad+1);

    /* put any required padding spaces at the start */
    for(i=0; i<pad; i++)
	ajStrAppC(&temp, " ");


    for(p=ajSeqChar(seq); *p; p++)
    {
	if(*p == '*')
	    ajStrAppC(&temp, "***");
	else if(*p == '.')
	    ajStrAppC(&temp, "...");
	else if(*p == '-')
	    ajStrAppC(&temp, "---");
	else if(!isalpha((ajint)*p))
	    ajStrAppC(&temp, "???");
	else
	{
	    p3 = embPropCharToThree(*p);
	    ajStrAppK(&temp, *p3);
	    ajStrAppK(&temp, *(p3+1));
	    ajStrAppK(&temp, *(p3+2));
	}
    }

    return temp;
}




/* @func embPropPurine ********************************************************
**
** Returns ajTrue if the input base is a Purine.
** Returns ajFalse if it is a Pyrimidine or it is ambiguous.
**
** @param [r] base [char] base
** @return [AjBool] return ajTrue if this is a Purine
** @@
******************************************************************************/

AjBool embPropPurine(char base)
{
    return (strchr(propPurines, (ajint)base) != NULL);
}




/* @func embPropPyrimidine ****************************************************
**
** Returns ajTrue if the input base is a Pyrimidine.
** Returns ajFalse if it is a Purine or it is ambiguous.
**
** @param [r] base [char] base
** @return [AjBool] return ajTrue if this is a Pyrimidine
** @@
******************************************************************************/

AjBool embPropPyrimidine(char base)
{
    return (strchr(propPyrimidines, (ajint)base) != NULL);
}




/* @func embPropTransversion **************************************************
**
** Returns ajTrue if the input two bases have undergone a tranversion.
** (Pyrimidine to Purine, or vice versa)
** Returns ajFalse if this is not a transversion or it can not be determined
** (e.g. no change A->A, transition C->T, unknown A->N)
**
** @param [r] base1 [char] first base
** @param [r] base2 [char] second base
** @return [AjBool] return ajTrue if this is a transversion
** @@
******************************************************************************/

AjBool embPropTransversion(char base1, char base2)
{
    AjBool u1;
    AjBool u2;
    AjBool y1;
    AjBool y2;

    u1 = embPropPurine(base1);
    u2 = embPropPurine(base2);

    y1 = embPropPyrimidine(base1);
    y2 = embPropPyrimidine(base2);

    ajDebug("base1 py = %d, pu = %d", u1, y1);
    ajDebug("base2 py = %d, pu = %d", u2, y2);


    /* not a purine or a pyrimidine - ambiguous - return ajFalse */
    if(!u1 && !y1)
	return ajFalse;

    if(!u2 && !y2)
	return ajFalse;

    ajDebug("embPropTransversion result = %d", (u1 != u2));

    return (u1 != u2);
}




/* @func embPropTransition ****************************************************
**
** Returns ajTrue if the input two bases have undergone a transition.
** (Pyrimidine to Pyrimidine, or Purine to Purine)
** Returns ajFalse if this is not a transition or it can not be determined
** (e.g. no change A->A, transversion A->T, unknown A->N)
**
** @param [r] base1 [char] first base
** @param [r] base2 [char] second base
** @return [AjBool] return ajTrue if this is a transition
** @@
******************************************************************************/

AjBool embPropTransition(char base1, char base2)
{
    AjBool u1;
    AjBool u2;
    AjBool y1;
    AjBool y2;

    u1 = embPropPurine(base1);
    u2 = embPropPurine(base2);

    y1 = embPropPyrimidine(base1);
    y2 = embPropPyrimidine(base2);

    ajDebug("base1 py = %d, pu = %d", u1, y1);
    ajDebug("base2 py = %d, pu = %d", u2, y2);

    /* not a purine or a pyrimidine - ambiguous - return ajFalse */
    if(!u1 && !y1)
	return ajFalse;

    if(!u2 && !y2)
	return ajFalse;

    /* no change - return ajFalse */
    if(tolower((int)base1) == tolower((int)base2))
	return ajFalse;

    /* U to T is not a transition */
    if(tolower((int)base1) == 't' && tolower((int)base2) == 'u')
	return ajFalse;

    if(tolower((int)base1) == 'u' && tolower((int)base2) == 't')
	return ajFalse;

    /* C to Y, T to Y, A to R, G to R - ambiguous - not a transition */
    if(u1 && tolower((int)base2) == 'r')
	return ajFalse;

    if(u2 && tolower((int)base1) == 'r')
	return ajFalse;

    if(y1 && tolower((int)base2) == 'y')
	return ajFalse;

    if(y2 && tolower((int)base1) == 'y')
	return ajFalse;

    ajDebug("embPropTransition result = %d", (u1 == u2));

    return (u1 == u2);
}

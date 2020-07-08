/* @source embprop.c
**
** Residue/sequence properties
** @author Copyright (c) 1999 Alan Bleasby
** @version 1.0
** @modified 24 Nov 1999 - GWW - Added embPropProtGaps and embPropProt1to3
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

static AjBool propInit=0;

double *EmbPropTable[EMBPROPSIZE];


static int propFragCompare(const void *a, const void *b);



/* @func embPropAminoRead  *************************************************
**
** Read amino acid properties from Eamino.dat
**
** @return [void]
** @@
******************************************************************************/

void embPropAminoRead(void)
{
    AjPFile mfptr=NULL;
    AjPStr  line=NULL;
    AjPStr  delim=NULL;
    
    char *p;

    int cols=0;


    if(propInit)
	return;

    ajFileDataNewC(AMINODATFILE, &mfptr);
    if(!mfptr) ajFatal("%s  not found\n",AMINODATFILE);

    line=ajStrNew();
    delim=ajStrNewC(" :\t\n");

    while(ajFileGets(mfptr, &line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || !*p) continue;
	while(*p && (*p<'A' || *p>'Z')) ++p;
	cols = ajStrTokenCount(&line,ajStrStr(delim));
	EmbPropTable[ajAZToInt(toupper((int)*p))] =
	    ajArrDoubleLine(&line,ajStrStr(delim),cols,2,cols);
    }


    ajStrDel(&line);
    ajStrDel(&delim);
    ajFileClose(&mfptr);

    propInit=ajTrue;
    return;
}



/* @func embPropCalcMolwt  ************************************************
**
** Calculate the molecular weight of a protein sequence
**
** @param [r] s [char *] sequence
** @param [r] start [int] start position
** @param [r] end [int] end position
**
** @return [double] molecular weight
** @@
******************************************************************************/
double embPropCalcMolwt(char *s, int start, int end)
{
    char *p;
    double sum;
    int i;
    int len;
    
    if(!propInit)
	embPropAminoRead();

    len = end-start+1;
    
    p=s+start;
    sum=0.0;

    for(i=0;i<len;++i)
	sum += EmbPropTable[ajAZToInt(toupper((int)p[i]))][EMBPROPMOLWT];

    return sum+18.0153;
}




/* @func embPropCharToThree  ************************************************
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



/* @func embPropIntToThree  ************************************************
**
** Return 3 letter amino acid code 0=Ala 1=Asx 2=Cys etc
**
** @param [r] c [int] integer code
**
** @return [char*] three letter amino acid code
** @@
******************************************************************************/
char* embPropIntToThree(int c)
{
    static char *tab[]=
    {
	"Ala","Asx","Cys","Asp","Glu","Phe","Gly","His","Ile","---","Lys",
	"Leu","Met","Asn","---","Pro","Gln","Arg","Ser","Thr","---",
	"Val","Trp","Xxx","Tyr","Glx" 
    };
    
    
    return tab[c];
}


/* @func embPropCalcFragments  ************************************************
**
** Read amino acd properties
**
** @param [r] s [char *] sequence
** @param [r] n [int] "enzyme" number
** @param [r] begin [int] sequence offset
** @param [w] l [AjPList *] list to push hits to
** @param [w] pa [AjPList *] list to push partial hits to
** @param [r] unfavoured [AjBool] allow unfavoured cuts
** @param [r] overlap [AjBool] show overlapping partials
** @param [r] allpartials [AjBool] show all possible partials
** @param [w] ncomp [int *] number of complete digest fragments
** @param [w] npart [int *] number of partial digest fragments
** @param [w] rname [AjPStr *] name of reagent
**
** @return [void]
** @@
******************************************************************************/
void embPropCalcFragments(char *s, int n, int begin, AjPList *l, AjPList *pa,
			 AjBool unfavoured, AjBool overlap,
			 AjBool allpartials, int *ncomp, int *npart,
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


    int i;
    int j;
    int lim;
    int len;
    AjPList t;
    EmbPPropFrag fr;
    int *begsa=NULL;
    int *endsa=NULL;
    double molwt;
    double *molwtsa=NULL;
    int v;
    int mark;
    int bwp;
    int ewp;
    
    int defcnt;


    (void) ajStrAssC(rname,PROPENZReagent[n]);
    defcnt = 0;
    len = strlen(s);

    t=ajListNew();		/* Temporary list */
    

    /* First get all potential cut points */
    for(i=0;i<len;++i)
    {
	if(!strchr(PROPENZSite[n],s[i])) continue;   /* A cut residue?       */
	if(len==i+1) continue;			   /* Is there a followup? */
	if(strchr(PROPENZUnfavoured[n],s[i+1])      /* Followed by naughty? */
	   && !unfavoured) continue;
	ajListPushApp(t,(void *)i);
	++defcnt;
    }

    if(defcnt) {
      AJCNEW (begsa, (defcnt+1));
      AJCNEW (endsa, (defcnt+1));
      AJCNEW (molwtsa, (defcnt+1));
    }

    for(i=0;i<defcnt;++i)	/* Pop them into a temporary array 	 */
    {
	(void) ajListPop(t,(void **)&v);
	endsa[i]=v;
    }


    mark=0;
    for(i=0;i<defcnt;++i)	/* Work out true starts, ends and molwts */
    {
	bwp=mark;
	ewp=endsa[i];
	if(strchr(PROPENZAminoCarboxyl[n],'N')) --ewp;
	molwt=embPropCalcMolwt(s,bwp,ewp);
	if(n==PROPENZCNBR)
	    molwt -= (17.045 + 31.095);
	begsa[i]=mark;
	endsa[i]=ewp;
	molwtsa[i]=molwt;
	mark=ewp+1;
    }
    if(defcnt)			/* Special treatment for last fragment   */
    {
	molwt=embPropCalcMolwt(s,mark,len-1);
	if(n==PROPENZCNBR)
	    molwt -= (17.045 + 31.095);
	begsa[i]=mark;
	endsa[i]=len-1;
	molwtsa[i]=molwt;
	++defcnt;
    }

    /* Push the hits */
    for(i=0;i<defcnt;++i)
    {
	AJNEW0 (fr);
	fr->start = begsa[i];
	fr->end   = endsa[i];
	fr->molwt = molwtsa[i];
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
	    AJNEW0 (fr);
	    fr->molwt=embPropCalcMolwt(s,begsa[i],endsa[i+1]);
	    if(n==PROPENZCNBR)
		fr->molwt -= (17.045 + 31.095);
	    fr->start = begsa[i];
	    fr->end   = endsa[i+1];
	    ajListPush(*pa,(void *)fr);
	    ++(*npart);
	}
	if(*npart)		/* Remove complete sequence */
	{
	    --(*npart);
	    (void) ajListPop(*pa,(void **)&fr);
	}
	ajListSort(*pa,propFragCompare);
    }

    if(allpartials)
    {
	for(i=0;i<lim;++i)
	    for(j=i+1;j<lim;++j)
	    {
		AJNEW0 (fr);
		fr->molwt=embPropCalcMolwt(s,begsa[i],endsa[j]);
		if(n==PROPENZCNBR)
		    fr->molwt -= (17.045 + 31.095);
		fr->start = begsa[i];
		fr->end   = endsa[j];
		ajListPush(*pa,(void *)fr);
		++(*npart);
	    }
	if(*npart)		/* Remove complete sequence */
	{
	    --(*npart);
	    (void) ajListPop(*pa,(void **)&fr);
	}
	ajListSort(*pa,propFragCompare);
    }
		
    if(defcnt)
    {
	AJFREE (molwtsa);
	AJFREE (endsa);
	AJFREE (begsa);
    }
    
    ajListFree(&t);
    return;
}



/* @funcstatic propFragCompare  *******************************************
**
** compare two molecular weight AjPFrag list elements for sorting
**
** @param [r] a [const void*] First element
** @param [r] b [const void*] Second element
**
** @return [int] 0=equal +ve=(a greater than b) -ve=(a less than b)
** @@
******************************************************************************/
static int propFragCompare(const void *a, const void *b)
{
    return (int)((*(EmbPPropFrag *)b)->molwt - (*(EmbPPropFrag *)a)->molwt);
}

/* @func embPropProtGaps ******************************************************
**
** Creates a string of a protein sequence which has been padded out with
** two spaces after every residue to aid aligning a translation under a
** nucleic sequence
**
** @param [u] seq [AjPSeq] protein sequence to add spaces into
** @param [r] pad [int] number of spaces to insert at the start of the result
** @return [AjPStr] New string with the padded sequence
** @@
******************************************************************************/

AjPStr embPropProtGaps (AjPSeq seq, int pad) {
  
  char *p;
  AjPStr temp = ajStrNewL(ajSeqLen(seq)*3 + pad+1);
  int i;

/* put any required padding spaces at the start */
  for (i=0; i<pad; i++) {
    (void) ajStrAppC(&temp, " ");
  }

  for (p=ajSeqChar(seq); *p; p++) {
    (void) ajStrAppK(&temp, *p);
    (void) ajStrAppC(&temp, "  ");
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
** @param [r] pad [int] number of spaces to insert at the start of the result
** @return [AjPStr] string containing 3-letter protein sequence
** @@
******************************************************************************/

AjPStr embPropProt1to3 (AjPSeq seq, int pad) {

  char *p, *p3;
  AjPStr temp = ajStrNewL(ajSeqLen(seq)*3 + pad+1);
  int i;

/* put any required padding spaces at the start */
  for (i=0; i<pad; i++) {
    (void) ajStrAppC(&temp, " ");
  }
 
  for (p=ajSeqChar(seq); *p; p++) {
    if (*p == '*') {
      (void) ajStrAppC(&temp, "***");
    } else if (*p == '.') {
      (void) ajStrAppC(&temp, "...");
    } else if (*p == '-') {
      (void) ajStrAppC(&temp, "---");
    } else if (!isalpha((int)*p)) {
      (void) ajStrAppC(&temp, "???");
    } else {
      p3 = embPropCharToThree(*p);
      (void) ajStrAppK(&temp, *p3);
      (void) ajStrAppK(&temp, *(p3+1));
      (void) ajStrAppK(&temp, *(p3+2));
    }
  }

  return temp;
}

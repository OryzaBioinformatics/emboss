/* @source prima.c
** @author: Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
** @@
** Application for selecting forward and reverse primers for PCR and 
** DNA amplification.
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
*****************************************************************************/
 
#include "emboss.h"
#include <math.h>
#include <limits.h>

#define SIMLIMIT 30
#define SIMLIMIT2 70


/* Definition of the primer object */
typedef struct AjSPrimer
{
  AjPStr substr;
  ajint start;
  ajint primerlen;
  float primerTm;
  float primGCcont;
  float prodTm;
  float prodGC;
  ajint   score;
} AjOPrimer, *AjPPrimer;


/* Object to hold awesome primer pairs */
typedef struct AjSPair
{
    AjPPrimer f;
    AjPPrimer r;
} AjOPair, *AjPPair;


static ajint prima_primalign(char *a, char *b);
static void prima_reject_self(AjPList *forlist,AjPList *revlist,
			      ajint *neric, ajint *nfred);
static void prima_best_primer(AjPList *forlist, AjPList *revlist,
			      ajint *neric, ajint *nfred);
static void prima_test_multi(AjPList *forlist, AjPList *revlist,
			     ajint *neric, ajint *nfred,
			     AjPStr seq, AjPStr rseq, ajint len);
static ajint prima_seq_align(char *a, char *b, ajint len);
static void  prima_PrimerDel(AjPPrimer *p);
static ajint prima_Compare(const void *a, const void *b);
static ajint prima_PosCompare(const void *a, const void *b);
static ajint prima_PosEndCompare(const void *a, const void *b);
/*static float prima_probAlign(AjPStr *seq1, AjPStr *seq2);*/
static void prima_prune_nearby(AjPList *pairlist, ajint *npair, ajint range);
static void prima_check_overlap(AjPList *pairlist, ajint *npair,
				ajint overlap);
static void prima_TwoSortscorepos(AjPList *pairlist);
static void prima_RevSort(AjPList *alist);


static void prima_testproduct(AjPStr seqstr, ajint startpos, ajint endpos,
			      ajint primerlen, ajint minprimerlen,
			      ajint maxprimerlen, float minpmGCcont, 
			      float maxpmGCcont, ajint minprimerTm,
			      ajint maxprimerTm, ajint minprodlen,
			      ajint maxprodlen, float prodTm, 
			      float prodGC, ajint seqlen, AjPPrimer *eric, 
			      AjPPrimer *fred, AjPList *forlist,
			      AjPList *revlist, ajint *neric, ajint *nfred,
			      ajint stepping_value, float saltconc,
			      float dnaconc, AjBool isDNA, ajint begin);      

static void prima_testtarget(AjPStr seqstr, AjPStr revstr, ajint targetstart,
			     ajint targetend, ajint minprimerlen,
			     ajint maxprimerlen, ajint seqlen,
			     float minprimerTm, float maxprimerTm,
			     float minpmGCcont, float maxpmGCcont,
			     float minprodGCcont, float maxprodGCcont,
			     float saltconc, float dnaconc,
			     AjPList *pairlist, ajint *npair);


/* @prog prima ***************************************************************
**
** Selects primers for PCR and DNA amplification
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf=NULL;

    AjPSeq sequence=NULL;
    AjPStr substr = ajStrNew();
    AjPStr seqstr=NULL;
    AjPStr revstr=NULL;
    
    AjPStr p1;
    AjPStr p2;
    
    AjPPrimer eric=NULL, fred=NULL;

    AjPPrimer f;
    AjPPrimer r;

    AjPPair pair;
    
    AjPList forlist=NULL;
    AjPList revlist=NULL;        
    AjPList pairlist=NULL;
    
    AjBool targetrange;
    AjBool isDNA=ajTrue;
    AjBool dolist=ajFalse;
    
    ajint primerlen=0;
    ajint minprimerlen=0;
    ajint maxprimerlen=0;
    ajint minprodlen=0;
    ajint maxprodlen=0;
    ajint prodlen=0;
    
    ajint seqlen=0;	
    ajint stepping_value=1;
  
    ajint targetstart=0;
    ajint targetend=0;
       
    ajint limit=0;
    ajint limit2=0;
    ajint lastpos=0;
    ajint startpos=0;
    ajint endpos=0;

    ajint begin;
    ajint end;
    ajint v1;
    ajint v2;

    ajint overlap;
    
    float minpmGCcont=0.;
    float maxpmGCcont=0.;
    float minprodGCcont=0.;
    float maxprodGCcont=0.;
    float prodTm;
    float prodGC;
    
    ajint i;
    ajint j;

    ajint neric=0;
    ajint nfred=0;
    ajint npair=0;
    
    float minprimerTm=0.0;
    float maxprimerTm=0.0;

    float saltconc=0.0;
    float dnaconc=0.0;
    
    forlist=ajListNew();
    revlist=ajListNew();
    pairlist=ajListNew();
    
    embInit ("prima", argc, argv);



    p1 = ajStrNew();
    p2 = ajStrNew();


    sequence = ajAcdGetSeq("sequence");
    outf = ajAcdGetOutfile("outf");

    minprimerlen = ajAcdGetInt("minprimerlen");
    maxprimerlen = ajAcdGetInt("maxprimerlen");
    minpmGCcont = ajAcdGetFloat("minpmGCcont");
    maxpmGCcont = ajAcdGetFloat("maxpmGCcont");
    minprimerTm = ajAcdGetFloat("minprimerTm");
    maxprimerTm = ajAcdGetFloat("maxprimerTm");
	
    minprodlen = ajAcdGetInt("minprodlen");
    maxprodlen = ajAcdGetInt("maxprodlen");
    minprodGCcont = ajAcdGetFloat("minprodGCcont");
    maxprodGCcont = ajAcdGetFloat("maxprodGCcont");
    
    saltconc = ajAcdGetFloat("saltconc");
    dnaconc = ajAcdGetFloat("dnaconc");
    
    targetrange = ajAcdGetBool("targetrange");
    targetstart = ajAcdGetInt("targetstart");
    targetend = ajAcdGetInt("targetend");

    overlap = ajAcdGetInt("overlap");
    dolist  = ajAcdGetBool("list");
    
    seqstr = ajSeqStrCopy(sequence);
    ajStrToUpper(&seqstr);
    
    begin = ajSeqBegin(sequence);
    end   = ajSeqEnd(sequence);
    seqlen = end-begin+1;
    
    ajStrAssSubC(&substr,ajStrStr(seqstr),begin-1,end-1);
    revstr=ajStrNewC(ajStrStr(substr));
    ajSeqReverseStr(&revstr);

    /* Initialise Tm calculation arrays */
    ajTm2(ajStrStr(substr),0,seqlen,saltconc,dnaconc,1);
    


    /************** END OF DECLARATIONS ******************************/
	
    
    ajFmtPrintF(outf, "\n\nINPUT SUMMARY\n");
    ajFmtPrintF(outf, "*************\n\n");

    if(targetrange)
	ajFmtPrintF
	    (outf, "Prima of %s from positions %d to %d bps\n",
	     ajSeqName(sequence),targetstart, targetend);
    else
	ajFmtPrintF(outf, "Prima of %s\n", ajSeqName(sequence));
 
    ajFmtPrintF(outf, "PRIMER CONSTRAINTS:\n");
    ajFmtPrintF
	(outf, "PRIMA DOES NOT ALLOW PRIMER SEQUENCE AMBIGUITY OR ");
    ajFmtPrintF(outf,"DUPLICATE PRIMER ENDPOINTS\n");
    ajFmtPrintF(outf,
		"Primer size range is %d-%d\n",minprimerlen,maxprimerlen);
    ajFmtPrintF(outf,
		"Primer GC content range is %.2f-%.2f\n",minpmGCcont,
		maxpmGCcont);
    ajFmtPrintF(outf,"Primer melting Temp range is %.2f - %.2f C\n", 
		minprimerTm, maxprimerTm);

    ajFmtPrintF (outf, "PRODUCT CONSTRAINTS:\n");

    ajFmtPrintF(outf,"Product GC content range is %.2f-%.2f\n",
		minprodGCcont, maxprodGCcont);

    ajFmtPrintF(outf, "Salt concentration is %.2f (mM)\n", saltconc);
    ajFmtPrintF(outf, "DNA concentration is %.2f (nM)\n", dnaconc);



    if(targetrange)
	ajFmtPrintF(outf, "Targeted range to amplify is from %d to %d\n",
		    targetstart,targetend);   
    else
    {
	ajFmtPrintF(outf,"Considering all suitable Primer pairs with ");
	ajFmtPrintF(outf,"Product length ranges %d to %d\n\n\n", minprodlen,
		    maxprodlen);
    }
    

    ajFmtPrintF(outf, "\n\nPRIMER/PRODUCT PAIR CALCULATIONS & OUTPUT\n");
    ajFmtPrintF(outf, "*****************************************\n\n");




    /*************/

 
    if(seqlen-minprimerlen < 0)
	ajFatal("Sequence too short");




    if(targetrange)
    {
	ajStrAssSubC(&p1,ajStrStr(substr),targetstart-begin,targetend-begin);

	prodGC=ajMeltGC(&substr,seqlen);	
	prodTm=ajProdTm(prodGC,saltconc,seqlen);

	if(prodGC<minprodGCcont || prodGC>maxprodGCcont)
	{
	    ajFmtPrintF(outf,
			"Product GC content [%.2f] outside acceptable range\n",
			prodGC);
	    ajExit();
	    return 0;
	}

	prima_testtarget(substr, revstr, targetstart-begin, targetend-begin,
			 minprimerlen, maxprimerlen,
			 seqlen, minprimerTm, maxprimerTm, minpmGCcont,
			 maxpmGCcont, minprodGCcont, maxprodGCcont, saltconc,
			 dnaconc, &pairlist, &npair);
    }
    


    if(!targetrange)
    {
	
    limit=seqlen-minprimerlen-minprodlen+1;
    lastpos=seqlen-minprodlen;
    limit2=maxprodlen-minprodlen;
    
    /* Outer loop selects all possible product start points */
    for(i=minprimerlen; i<limit; ++i)
    {
	startpos=i;
	ajDebug("Position in sequence %d\n",startpos);
	endpos=i+minprodlen-1;
	/* Inner loop selects all possible product lengths  */
	for(j=0; j<limit2; ++j, ++endpos)
	{
	    if(endpos>lastpos) break;

	    v1 = endpos-startpos+1;
	    ajStrAssSubC(&p1,ajStrStr(substr),startpos,endpos);
	    prodGC = ajMeltGC(&p1,v1);
	    prodTm = ajProdTm(prodGC,saltconc,v1);

	    if(prodGC<minprodGCcont || prodGC>maxprodGCcont)
		continue;

	    /* Only accept primers with acceptable Tm and GC */
	    neric=0;
	    nfred=0;
	    prima_testproduct(substr, startpos, endpos, primerlen,
			      minprimerlen, maxprimerlen,minpmGCcont,
			      maxpmGCcont, minprimerTm, maxprimerTm,
			      minprodlen, maxprodlen, prodTm, prodGC, seqlen,
			      &eric,&fred,&forlist,&revlist,&neric,&nfred,
			      stepping_value, saltconc,dnaconc, isDNA, begin); 
	    if(!neric)
		continue;
	
	

	    /* Now reject those primers with self-complementarity */
	   
	    prima_reject_self(&forlist,&revlist,&neric,&nfred);
	    if(!neric)
		continue;

	    /* Reject any primers that could bind elsewhere in the
               sequence */
	    prima_test_multi(&forlist,&revlist,&neric,&nfred,substr,revstr,
			     seqlen);
	    


	    /* Now select the least complementary pair (if any) */
	    prima_best_primer(&forlist, &revlist, &neric, &nfred);
	    if(!neric)
		continue;
	    
	    AJNEW(pair);
	    ajListPop(forlist,(void **)&f);
	    ajListPop(revlist,(void **)&r);
	    pair->f = f;
	    pair->r = r;
	    ++npair;
	    ajListPush(pairlist,(void *)pair);
	}
     }
   
  }


    if(!targetrange)
    {
	/* Get rid of primer pairs nearby the top scoring ones */
	prima_TwoSortscorepos(&pairlist);
	prima_prune_nearby(&pairlist, &npair, maxprimerlen-1);
	ajListSort(pairlist,prima_PosCompare);
	prima_check_overlap(&pairlist,&npair,overlap);
    }


    
    if(npair)
    {
	if(!targetrange)
	    ajFmtPrintF(outf,"%d pairs found\n\n",npair);
	else
	    ajFmtPrintF(outf,
			"Closest primer pair to specified product is:\n\n");
	if((maxprimerlen<26 && seqlen<999999 && !dolist))
	    ajFmtPrintF(outf,"\n\t\tForward\t\t\t\t\tReverse\n\n");
    }
    
    

    for(i=0;i<npair;++i)
    {
	if(!targetrange)
	    ajFmtPrintF(outf,"[%d]\n",i+1);

	ajListPop(pairlist,(void **)&pair);


	prodlen = pair->r->start - (pair->f->start + pair->f->primerlen);

	if((maxprimerlen<26 && seqlen<999999 && !dolist))
	{
	    v1 = pair->f->start;
	    v2 = v1 + pair->f->primerlen -1;

	    ajStrAssSub(&p1,substr,v1,v2);
	    ajFmtPrintF(outf,"%6d %-25.25s %d\t", v1+begin, ajStrStr(p1),
			v2+begin);


	    v1 = pair->r->start;
	    v2 = v1 + pair->r->primerlen -1;
	    ajStrAssSub(&p2,substr,v1,v2);
	    ajSeqReverseStr(&p2);
	    ajFmtPrintF(outf,
			"%6d %-25.25s %d\n", v1+begin, ajStrStr(p2), v2+begin);
	

	    ajFmtPrintF(outf,"       Tm  %.2f C  (GC %.2f%%)\t\t       ",
			pair->f->primerTm,pair->f->primGCcont*100.);
	    ajFmtPrintF(outf,"Tm  %.2f C  (GC %.2f%%)\n",
			pair->r->primerTm,pair->r->primGCcont*100.);

	    ajFmtPrintF(outf,"             Length: %-32dLength: %d\n",
			pair->f->primerlen,pair->r->primerlen);
	    ajFmtPrintF(outf,"             Tma:    %.2f C\t\t\t",
			ajAnneal(pair->f->primerTm,pair->f->prodTm));
	    ajFmtPrintF(outf,"     Tma:    %.2f C\n\n\n",
			ajAnneal(pair->r->primerTm,pair->f->prodTm));


	    ajFmtPrintF(outf,"       Product GC: %.2f%%\n",
			pair->f->prodGC * 100.0);
	    ajFmtPrintF(outf,"       Product Tm: %.2f C\n",
			pair->f->prodTm);
	    ajFmtPrintF(outf,"       Length:     %d\n\n\n",prodlen);
	}
	else
	{
	    ajFmtPrintF(outf,"    Product from %d to %d\n",pair->f->start+
			pair->f->primerlen+begin,pair->r->start-1+begin);
	    ajFmtPrintF(outf,"                 Tm: %.2f C   GC: %.2f%%\n",
			pair->f->prodTm,pair->f->prodGC*(float)100.);
	    ajFmtPrintF(outf,"                 Length: %d\n\n\n",prodlen);


	    v1 = pair->f->start;
	    v2 = v1 + pair->f->primerlen -1;
	    ajStrAssSub(&p1,substr,v1,v2);
	    ajFmtPrintF(outf,"    Forward: 5' %s 3'\n",ajStrStr(p1));
	    ajFmtPrintF(outf,"             Start: %d\n",v1+begin);
	    ajFmtPrintF(outf,"             End:   %d\n",v2+begin);
	    ajFmtPrintF(outf,"             Tm:    %.2f C\n",
			pair->f->primerTm);
	    ajFmtPrintF(outf,"             GC:    %.2f%%\n",
			pair->f->primGCcont*(float)100.);
	    ajFmtPrintF(outf,"             Len:   %d\n",
			pair->f->primerlen);
	    ajFmtPrintF(outf,"             Tma:   %.2f C\n\n\n",
			ajAnneal(pair->f->primerTm,pair->f->prodTm));
	    
	    v1 = pair->r->start;
	    v2 = v1 + pair->r->primerlen -1;
	    ajStrAssSub(&p2,substr,v1,v2);
	    ajSeqReverseStr(&p2);
	    ajStrAssSub(&p1,substr,v1,v2);
	    ajFmtPrintF(outf,"    Reverse: 5' %s 3'\n",ajStrStr(p1));
	    ajFmtPrintF(outf,"             Start: %d\n",v1+begin);
	    ajFmtPrintF(outf,"             End:   %d\n",v2+begin);
	    ajFmtPrintF(outf,"             Tm:    %.2f C\n",
			pair->r->primerTm);
	    ajFmtPrintF(outf,"             GC:    %.2f%%\n",
			pair->r->primGCcont*(float)100.);
	    ajFmtPrintF(outf,"             Len:   %d\n",
			pair->r->primerlen);
	    ajFmtPrintF(outf,"             Tma:   %.2f C\n\n\n",
			ajAnneal(pair->r->primerTm,pair->f->prodTm));
	}

	

	
	prima_PrimerDel(&pair->f);
	prima_PrimerDel(&pair->r);
	AJFREE(pair);
}


  
    ajStrDel(&seqstr);
    ajStrDel(&substr);

    ajListDel(&forlist);
    ajListDel(&revlist);
    ajListDel(&pairlist);
    

    ajExit();
    return 0;
  
}


/* @funcstatic prima_primalign ************************************************
**
** Align two sequences and return match percentage
**
** @param [r] a [char*] sequence a
** @param [r] b [char*] sequence b
** @return [ajint] percent match
** @@
******************************************************************************/

static ajint prima_primalign(char *a, char *b)
{
    ajint plen, qlen, limit, i, n=0, mm=0, j;
    char *p, *q;
    ajint alen,blen;
    
    alen=strlen(a);
    blen=strlen(b);
    
    
    if ( alen > blen)
    {
        plen = alen;
	qlen = blen;
        p=a;
        q=b;
    }
    else
    {
        plen = blen;
        qlen = alen;
        p=b;
        q=a;
    }
        
    limit= plen-qlen+1;
    
    for(i=0; i<limit; ++i)
    {
        for(j=0; j<qlen; ++j)
	    if(p[j]==q[j])
		++n;
        
        mm = AJMAX(mm, n);
        ++p;
    }
    
    return (ajint)(((float)mm/(float)qlen)*100.0);
}


/* #funcstatic prima_probAlign ************************************************
**
** prob score
**
** #param [r] seq1 [AjPStr*] seq1
** #param [r] seq2 [AjPStr*] seq2
** #return [float] probability
** ##
******************************************************************************/
/*
//static float prima_probAlign(AjPStr *seq1, AjPStr *seq2)
//{
//
//    float score;
//    ajint i;
//    ajint x;
//    ajint y;
//    char *p;
//    char *q;
//    ajint len;
//    
//
//    len = (ajStrLen(*seq1) <= ajStrLen(*seq2)) ? ajStrLen(*seq1) :
//	ajStrLen(*seq2);
//
//   
//    if(!aj_base_I) ajBaseInit();
//    score = 0.0;
//    if(!len) return score;
//
//    score = 1.0;
//    p = ajStrStr(*seq1);
//    q = ajStrStr(*seq2);
//    
//    for(i=0; i<len; ++i)
//    {
//	x = ajAZToInt(*(p+i));
//	y = ajAZToInt(*(q+i));
//	score *= aj_base_prob[x][y];
//    }
//
//    return score;
//}
*/


/* @funcstatic prima_testproduct **********************************************
**
** Undocumented.
**
** @param [?] seqstr [AjPStr] Undocumented
** @param [?] startpos [ajint] Undocumented
** @param [?] endpos [ajint] Undocumented
** @param [?] primerlen [ajint] Undocumented
** @param [?] minprimerlen [ajint] Undocumented
** @param [?] maxprimerlen [ajint] Undocumented
** @param [?] minpmGCcont [float] Undocumented
** @param [?] maxpmGCcont [float] Undocumented
** @param [?] minprimerTm [ajint] Undocumented
** @param [?] maxprimerTm [ajint] Undocumented
** @param [?] minprodlen [ajint] Undocumented
** @param [?] maxprodlen [ajint] Undocumented
** @param [?] prodTm [float] Undocumented
** @param [?] prodGC [float] Undocumented
** @param [?] seqlen [ajint] Undocumented
** @param [?] eric [AjPPrimer*] Undocumented
** @param [?] fred [AjPPrimer*] Undocumented
** @param [?] forlist [AjPList*] Undocumented
** @param [?] revlist [AjPList*] Undocumented
** @param [?] neric [ajint*] Undocumented
** @param [?] nfred [ajint*] Undocumented
** @param [?] stepping_value [ajint] Undocumented
** @param [?] saltconc [float] Undocumented
** @param [?] dnaconc [float] Undocumented
** @param [?] isDNA [AjBool] Undocumented
** @param [?] begin [ajint] Undocumented
** @@
******************************************************************************/

static void prima_testproduct
 (AjPStr seqstr, ajint startpos, ajint endpos, ajint primerlen, 
 ajint minprimerlen, ajint maxprimerlen, float minpmGCcont, float maxpmGCcont, 
 ajint minprimerTm, ajint maxprimerTm, ajint minprodlen, ajint maxprodlen, 
 float prodTm, float prodGC, ajint seqlen, AjPPrimer *eric, 
 AjPPrimer *fred, AjPList *forlist, AjPList *revlist, ajint *neric,
 ajint *nfred, ajint stepping_value, float saltconc, float dnaconc, 
 AjBool isDNA, ajint begin)

{
    AjPStr substr=NULL;
    AjPPrimer rubbish=NULL;
    ajint forpstart;
    ajint forpend;
    ajint revpstart;
    ajint revpend;
    ajint i;
    ajint tnum=0;
    ajint thisplen;
    
    float primerTm=0.0;
    float primGCcont=0.0;
	
    forpend = startpos -1;
    revpstart = endpos +1;
    
    tnum = maxprimerlen-minprimerlen+1;


    substr=ajStrNew();
                
    /* FORWARD PRIMERS */
    
    forpstart = forpend-minprimerlen+1;
            
    for(i=0; i<tnum; ++i,--forpstart)
    {
	if(forpstart<0) break;

	ajStrAssSubC(&substr,ajStrStr(seqstr),forpstart,forpend);
	thisplen = minprimerlen + i;
    
	primerTm = ajTm2("",forpstart,thisplen, saltconc,
			dnaconc, isDNA);
	
	/* If temp out of range ignore rest of loop iteration */
	if(primerTm<minprimerTm || primerTm>maxprimerTm)
	    continue;
	
	primGCcont = ajMeltGC(&substr, thisplen);
	
	/* If GC content out of range ignore rest of loop iteration */
	if(primGCcont<minpmGCcont || primGCcont>maxpmGCcont)
	    continue;
	
	/*
	 *  This is a valid primer as far as Tm & GC is concerned
	 *  so push it to the storage list
         */
	AJNEW0(*eric);
   
	(*eric)->substr = ajStrNewC(ajStrStr(substr));
	(*eric)->start = forpstart+begin;
	(*eric)->primerlen = thisplen;
	(*eric)->primerTm = primerTm;
	(*eric)->primGCcont = primGCcont;
	(*eric)->prodTm = prodTm;
	(*eric)->prodGC = prodGC;
	ajListPush(*forlist, (void*)*eric);
	(*neric)++;
    }

    if(!*neric)
    {
	ajStrDel(&substr);
	return;
    }
        
    /* REVERSE PRIMERS */
    revpend=revpstart + minprimerlen-1;  
    for(i=0; i<tnum; ++i,++revpend)
    {
	if(revpend>seqlen) break;

	ajStrAssSubC(&substr,ajStrStr(seqstr),revpstart,revpend);
	ajSeqReverseStr(&substr);
	
	thisplen = minprimerlen + i;
    
	primerTm = ajTm2("",revpstart,thisplen, saltconc,
			dnaconc, isDNA);
	/* If temp out of range ignore rest of loop iteration */
	if(primerTm<minprimerTm || primerTm>maxprimerTm)
	    continue;
	
	primGCcont = ajMeltGC(&substr, thisplen);
	/* If GC content out of range ignore rest of loop iteration */
	if(primGCcont<minpmGCcont || primGCcont>maxpmGCcont)
	    continue;
	
	/*
	 *  This is a valid primer as far as Tm & GC is concerned
	 *  so push it to the reverse primer storage list
         */
	AJNEW0(*fred);
	(*fred)->substr = ajStrNewC(ajStrStr(substr));
	(*fred)->start = revpstart+begin;
	(*fred)->primerlen = thisplen;
	(*fred)->primerTm = primerTm;
	(*fred)->primGCcont = primGCcont;
	ajListPush(*revlist, (void*)*fred);
	(*nfred)++;
    }

    ajStrDel(&substr);
    

    if(!*nfred)
    {
	*neric = 0;
	while(ajListPop(*forlist,(void**)&rubbish))
	    prima_PrimerDel(&rubbish);
    }
    
    return;
}



/* @funcstatic prima_reject_self **********************************************
**
** reject self complementary primers
**
** @param [?] forlist [AjPList*] Undocumented
** @param [?] revlist [AjPList*] Undocumented
** @param [?] neric [ajint*] Undocumented
** @param [?] nfred [ajint*] Undocumented
** @@
******************************************************************************/

static void prima_reject_self(AjPList *forlist,AjPList *revlist, ajint *neric,
			      ajint *nfred)
{
    ajint count;
    ajint j;
    ajint i;
    AjPPrimer tmp;
    
    ajint len;
    ajint cut;
    AjPStr str1;
    AjPStr str2;
    ajint x;
    

    str1=ajStrNew();
    str2=ajStrNew();
    
    /* deal with forwards */
    count = *neric;
    for(i=0;i<*neric;++i)
    {
	ajListPop(*forlist,(void **)&tmp);
	len = tmp->primerlen;
	cut = (len/2)-1;
	ajStrAssSubC(&str1,ajStrStr(tmp->substr),0,cut);
	ajStrAssSubC(&str2,ajStrStr(tmp->substr),cut+1,len-1);
	x = prima_primalign(ajStrStr(str1),ajStrStr(str2));
	if(x<SIMLIMIT)
	    ajListPushApp(*forlist,(void *)tmp);
	else
	{
	    prima_PrimerDel(&tmp);
	    --count;
	}
    }
    *neric = count;
    if (!*neric)
    {
	ajStrDel(&str1);
	ajStrDel(&str2);
	while(ajListPop(*revlist,(void**)&tmp))
	    prima_PrimerDel(&tmp);
	*nfred=0;
	return;
    }
    

    
/****** reverses ********/

    count = *nfred;
    
    for(j=0; j<*nfred; ++j)
    {
	ajListPop(*revlist,(void **)&tmp);
	len = tmp ->primerlen;
	cut = (len/2)-1;
	ajStrAssSubC(&str1,ajStrStr(tmp->substr),0,cut);
	ajStrAssSubC(&str2,ajStrStr(tmp->substr),cut+1,len-1);
	x = prima_primalign(ajStrStr(str1),ajStrStr(str2));
	if(x<SIMLIMIT)
	    ajListPushApp(*revlist,(void *)tmp);
	else
	{
	    --count;
	    prima_PrimerDel(&tmp);
	}
    }
    *nfred = count;
    
    if(!*nfred)
    {
	while(ajListPop(*forlist,(void**)&tmp))
	    prima_PrimerDel(&tmp);
	*neric=0;
    }
    ajStrDel(&str1);
    ajStrDel(&str2);
    return;
    
}

/* @funcstatic prima_best_primer **********************************************
**
** BEST PRIMER FUNCTION
**
** @param [?] forlist [AjPList*] Undocumented
** @param [?] revlist [AjPList*] Undocumented
** @param [?] neric [ajint*] Undocumented
** @param [?] nfred [ajint*] Undocumented
** @@
******************************************************************************/

static void prima_best_primer(AjPList *forlist, AjPList *revlist,
			      ajint *neric, ajint *nfred)
{
    ajint bestf;
    ajint bestr;
    ajint lowx;
    ajint i;
    ajint j;
    ajint x;
    
    AjPPrimer temp;
    AjPPrimer temp2;

    AjPPrimer hitf;
    AjPPrimer hitr;

    AjBool good;
    

    lowx =INT_MAX;
    bestf=bestr=0;
    

    /* First find the best primer (if any) */
    good = ajFalse;
    
    for(i=0;i<*neric; ++i)
    {
	ajListPop(*forlist, (void**)&temp);
	
	for(j=0; j<*nfred; ++j)
	{
	    ajListPop(*revlist, (void**)&temp2);
	    
	    x=prima_primalign(ajStrStr(temp->substr),
			      ajStrStr(temp2->substr));    
	    
	    if(x<=SIMLIMIT)
		good = ajTrue;
	    
	    if (x < lowx)
		{
		    temp->score=x;
		    bestf=i;
		    bestr=j;
		    lowx=x;
		}
	    ajListPushApp(*revlist, (void *)temp2);
	    
	}
	ajListPushApp(*forlist, (void *)temp);
    }


    if(!good)
    {
	while(ajListPop(*forlist,(void **)&temp))
	    prima_PrimerDel(&temp);
	while(ajListPop(*revlist,(void **)&temp))
	    prima_PrimerDel(&temp);
	*neric = 0;
	*nfred = 0;
	return;
    }
    

    /* Get the best fwd one in hitf, discard the rest */
    /* Discard ones before our hit */
    for(i=0;i<bestf;++i)
    {
	ajListPop(*forlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }
    /* Next on the list is our hit */
    ajListPop(*forlist,(void **)&hitf);
    /* Get rid of anything left on the list */
    for(i++;i<*neric;++i)
    {
	ajListPop(*forlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }
    

    /* Get the best rev one in hitr, discard the rest */
    for(i=0;i<bestr;++i)
    {
	ajListPop(*revlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }
    ajListPop(*revlist,(void **)&hitr);
    for(i++;i<*nfred;++i)
    {
	ajListPop(*revlist,(void **)&temp);
	prima_PrimerDel(&temp);
    }

    ajListPushApp(*forlist,(void *)hitf);
    ajListPushApp(*revlist,(void *)hitr);
    
    *neric = 1;
    *nfred = 1;

    return;
}


/* @funcstatic prima_PrimerDel ************************************************
**
** Free memory from primers
**
** @param [w] p [AjPPrimer*] Undocumented
** @@
******************************************************************************/

static void prima_PrimerDel(AjPPrimer *p)
{
    ajStrDel(&((*p)->substr));
    AJFREE(*p);
    return;
}

  
  

/* @funcstatic prima_PrimerCompare ********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_Compare(const void *a, const void *b)
{
    return (*(AjPPair *)a)->f->score -
		   (*(AjPPair *)b)->f->score;
}



/* @funcstatic prima_PosCompare ***********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_PosCompare(const void *a, const void *b)
{

    return ((*(AjPPair *)a)->f->start + (*(AjPPair *)a)->f->primerlen - 1)  -
	   ((*(AjPPair *)b)->f->start + (*(AjPPair *)b)->f->primerlen - 1);
}


/* @funcstatic prima_PosEndCompare ********************************************
**
** Undocumented.
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint prima_PosEndCompare(const void *a, const void *b)
{

    return ((*(AjPPair *)a)->r->start)  -
	   ((*(AjPPair *)b)->r->start);
}




/* @funcstatic prima_testtarget ***********************************************
**
** Undocumented.
**
** @param [?] seqstr [AjPStr] Undocumented
** @param [?] revstr [AjPStr] Undocumented
** @param [?] targetstart [ajint] Undocumented
** @param [?] targetend [ajint] Undocumented
** @param [?] minprimerlen [ajint] Undocumented
** @param [?] maxprimerlen [ajint] Undocumented
** @param [?] seqlen [ajint] Undocumented
** @param [?] minprimerTm [float] Undocumented
** @param [?] maxprimerTm [float] Undocumented
** @param [?] minpmGCcont [float] Undocumented
** @param [?] maxpmGCcont [float] Undocumented
** @param [?] minprodGCcont [float] Undocumented
** @param [?] maxprodGCcont [float] Undocumented
** @param [?] saltconc [float] Undocumented
** @param [?] dnaconc [float] Undocumented
** @param [?] pairlist [AjPList*] Undocumented
** @param [?] npair [ajint*] Undocumented
** @@
******************************************************************************/

static void prima_testtarget(AjPStr seqstr, AjPStr revstr, ajint targetstart,
			     ajint targetend, ajint minprimerlen,
			     ajint maxprimerlen, ajint seqlen,
			     float minprimerTm, float maxprimerTm,
			     float minpmGCcont, float maxpmGCcont,
			     float minprodGCcont, float maxprodGCcont,
			     float saltconc, float dnaconc,
			     AjPList *pairlist, ajint *npair)
{
    
	
    AjPStr fstr;
    AjPStr rstr;
    
    AjPStr str1;
    AjPStr str2;
    AjPPrimer f;
    AjPPrimer r;
    
    AjPPair   ppair;

    ajint i;
    ajint j;
    ajint forstart=0;
    ajint forend;
    ajint revstart=0;
    ajint revend;
    ajint Limit;
    ajint tnum;
    ajint thisplen;
    ajint cut;
    
    float primerTm=0.0;
    float primGCcont=0.0;
    float prodgc=0.0;
    
    AjBool found=ajFalse;
    AjBool revfound=ajFalse;
    AjBool isDNA=ajTrue;
    
    ajint flen=0;
    ajint rlen=0;
    
    float ftm=0.0;
    float rtm=0.0;
    float fgc=0.0;
    float rgc=0.0;
    ajint fsc=0;
    ajint rsc=0;

    char *s;
    char *s2;
    char *p;
    ajint  pv;
    ajint  plimit;
    ajint  pcount;
    ajint  k;

    
    fstr  =ajStrNew();
    rstr  =ajStrNew();
    str1 =ajStrNew();
    str2 = ajStrNew();

    

    tnum=maxprimerlen-minprimerlen+1;

    /******FORWARDS  *******/

    for(i=targetstart-minprimerlen; i>-1; --i)
    {
	forstart=i;
	forend=i+minprimerlen-1;
	    
    
	for(j=0; j<tnum; ++j,++forend)
	{
	    if(forend==targetstart)
		break;
	    
	    ajStrAssSubC(&fstr, ajStrStr(seqstr), forstart, forend);

	    thisplen = ajStrLen(fstr);
	    primerTm =ajTm2("",forstart,thisplen, saltconc, dnaconc, isDNA);

	    if(primerTm <minprimerTm || primerTm>maxprimerTm)
		continue;
		
	    primGCcont= ajMeltGC(&fstr, thisplen);
	    if (primGCcont< minpmGCcont || primGCcont >maxpmGCcont)
		continue;

		
	    /*instead of calling the self-reject function */
	    cut =(thisplen/2)-1;
		
	    ajStrAssSubC(&str1, ajStrStr(fstr), 0, cut);
	    ajStrAssSubC(&str2, ajStrStr(fstr), cut+1, thisplen-1);
		
	    if((fsc=prima_primalign(ajStrStr(str1), ajStrStr(str2))) >
	       SIMLIMIT)
		continue;

	    /* Test for match with rest of sequence */
	    s = ajStrStr(seqstr);
	    s2 = ajStrStr(revstr);
	    p = ajStrStr(fstr);
	    pv = thisplen;
	    pcount=0;
	    plimit=seqlen-pv+1;
	    for(k=0;k<plimit && pcount<2;++k)
	    {
		if(prima_seq_align(s+k,p,pv)>SIMLIMIT2)
		    ++pcount;
		if(prima_seq_align(s2+k,p,pv)>SIMLIMIT2)
		    ++pcount;
	    }
	    
	    if(pcount<2)
	    {
		found =ajTrue;
		flen = thisplen;
		ftm = primerTm;
		fgc = primGCcont;
		break;
	    }
	}
		
	if(found)
	    break;
    }

	    
		
    /******* REVERSES IN TARGETRANGE *****/


    Limit = seqlen-minprimerlen;
	 
    if(found) 
	for(i=targetend+1; i<Limit; ++i)
	{
	    revstart=i;
	    revend=i+minprimerlen-1;
				
	    for(j=0; j<tnum; ++j,++revend)
	    {
		if (revend==seqlen)
		    break;

		ajStrAssSubC(&rstr, ajStrStr(seqstr), revstart, revend);
		ajSeqReverseStr(&rstr);
		
		thisplen = ajStrLen(rstr);
		primerTm =ajTm2("", revstart, thisplen, saltconc, dnaconc, 1);
		    
		if(primerTm <minprimerTm || primerTm>maxprimerTm)
		    continue;
		
		primGCcont= ajMeltGC(&rstr, thisplen);
		if (primGCcont< minpmGCcont || primGCcont >maxpmGCcont)
		    continue;
		
		/*instead of calling the self-reject function */
		cut =(thisplen/2)-1;
		
		ajStrAssSubC(&str1, ajStrStr(rstr), 0, cut);
		ajStrAssSubC(&str2, ajStrStr(rstr), cut+1, thisplen-1);

		if((rsc=prima_primalign(ajStrStr(str1), ajStrStr(str2))) <
		   SIMLIMIT)
		    continue;

		/* Test for match with rest of sequence */
		s = ajStrStr(seqstr);
		s2 = ajStrStr(revstr);
		p = ajStrStr(rstr);
		pv = thisplen;
		pcount=0;
		plimit=seqlen-pv+1;
		for(k=0;k<plimit && pcount<2;++k)
		{
		    if(prima_seq_align(s+k,p,pv)>SIMLIMIT2)
			++pcount;
		    if(prima_seq_align(s2+k,p,pv)>SIMLIMIT2)
			++pcount;
		}
		
		if(pcount<2)
		{
		    revfound =ajTrue;
		    rlen = thisplen;
		    rtm = primerTm;
		    rgc = primGCcont;
		    break;
		}
	    }
		    
	    if(revfound)
		break;
	}
		

    if(found && !revfound)
    {
	found=ajFalse;
	ajWarn("No reverse primers found in targetrange");
	*npair=0;
	return;
    }

		

    if(!found)
    {
	ajWarn("No forward primers found in targetrange");
	*npair=0;
	return;
    }

    ajStrAssSubC(&str1,ajStrStr(seqstr),forstart+flen,revstart-1);
    prodgc = ajMeltGC(&str1,revstart-(forstart+flen));
    

    
    AJNEW0(f);
    f->substr = ajStrNewC(ajStrStr(fstr));
    f->start = forstart;
    f->primerlen = flen;
    f->primerTm = ftm;
    f->primGCcont = fgc;
    f->score = fsc;
    f->prodGC = prodgc;
    f->prodTm = ajProdTm(prodgc,saltconc,revstart-(forstart+flen));
    

    AJNEW0(r);
    r->substr = ajStrNewC(ajStrStr(rstr));
    r->start = revstart;
    r->primerlen = rlen;
    r->primerTm = rtm;
    r->primGCcont = rgc;
    r->score = rsc;
    

    AJNEW0(ppair);
    ppair->f = f;
    ppair->r = r;
    ajListPush(*pairlist,(void *)ppair);
    *npair=1;

    return;
}




/* @funcstatic prima_test_multi ***********************************************
**
** Undocumented.
**
** @param [?] forlist [AjPList*] Undocumented
** @param [?] revlist [AjPList*] Undocumented
** @param [?] neric [ajint*] Undocumented
** @param [?] nfred [ajint*] Undocumented
** @param [?] seq [AjPStr] Undocumented
** @param [?] rseq [AjPStr] Undocumented
** @param [?] len [ajint] Undocumented
** @@
******************************************************************************/


static void prima_test_multi(AjPList *forlist, AjPList *revlist, ajint *neric,
			     ajint *nfred, AjPStr seq, AjPStr rseq, ajint len)
{
    AjPPrimer tmp;
    AjPStr st=ajStrNew();
    
    ajint i;
    ajint j;
    ajint v;
    ajint pc;
    ajint count;
    ajint limit;
    
    char *s;
    char *r;
    char *p;

    s=ajStrStr(seq);
    r=ajStrStr(rseq);
    
    pc=*neric;

    for(i=0;i<*neric;++i)
    {
	ajListPop(*forlist,(void **)&tmp);
	count=0;
	v=tmp->primerlen;
	limit=len-v+1;
	p=ajStrStr(tmp->substr);
	for(j=0;j<limit && count<2;++j)
	{
	    if(prima_seq_align(s+j,p,v)>SIMLIMIT2)
		++count;
	    if(prima_seq_align(r+j,p,v)>SIMLIMIT2)
		++count;
	}
	

	if(count>1)
	{
	    prima_PrimerDel(&tmp);
	    --pc;
	}
	else
	    ajListPushApp(*forlist,(void *)tmp);
    }

    *neric=pc;
    if(!*neric)
    {
	while(ajListPop(*revlist,(void **)&tmp))
	    prima_PrimerDel(&tmp);
	*nfred=0;
	ajStrDel(&st);
	return;
    }


    pc=*nfred;
    for(i=0;i<*nfred;++i)
    {
	ajListPop(*revlist,(void **)&tmp);
	count=0;
	v=tmp->primerlen;
	limit=len-v+1;
	ajStrAssC(&st,ajStrStr(tmp->substr));
	ajSeqReverseStr(&st);
	p=ajStrStr(st);
	for(j=0;j<limit && count<2;++j)
	{
	    if(prima_seq_align(s+j,p,v)>SIMLIMIT2)
		++count;
	    if(prima_seq_align(r+j,p,v)>SIMLIMIT2)
		++count;
	}
	
	
	if(count>1)
	{
	    prima_PrimerDel(&tmp);
	    --pc;
	}
	else
	    ajListPushApp(*revlist,(void *)tmp);
    }

    *nfred=pc;
    if(!*nfred)
    {
	while(ajListPop(*forlist,(void **)&tmp))
	    prima_PrimerDel(&tmp);
	*neric=0;
    }

    ajStrDel(&st);
    return;
}


/* @funcstatic prima_seq_align ************************************************
**
** Undocumented.
**
** @param [?] a [char*] Undocumented
** @param [?] b [char*] Undocumented
** @param [?] len [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/


static ajint prima_seq_align(char *a, char *b, ajint len)
{
    ajint i;
    ajint count;

    count=0;
    for(i=0;i<len;++i)
	if(a[i]==b[i])
	    ++count;
    
    return (ajint)(((float)count/(float)len)*(float)100.0);
}





/* @funcstatic prima_prune_nearby *********************************************
**
** Undocumented.
**
** @param [?] pairlist [AjPList*] Undocumented
** @param [?] npair [ajint*] Undocumented
** @param [?] range [ajint] Undocumented
** @@
******************************************************************************/


static void prima_prune_nearby(AjPList *pairlist, ajint *npair, ajint range)
{
    AjPPair pair;

    ajint count;
    ajint fst;
    ajint fst2;
    ajint blim;
    ajint blim2;
    ajint elim;
    ajint elim2;
    ajint i;
    ajint j;
    ajint len;

    for(i=0;i<*npair;++i)
    {
	for(j=0;j<i;++j)	/* Ignore those already processed */
	{
	    ajListPop(*pairlist,(void **)&pair);
	    ajListPushApp(*pairlist,(void *)pair);
	}

	ajListPop(*pairlist,(void **)&pair);/* Get next high scoring pair */
	len = pair->f->primerlen;
	fst = pair->f->start + len -1;
	blim=fst-/*len*/range;
	elim=fst+/*len*/range;

	len = pair->r->primerlen;
	blim2 = pair->r->start - /*len*/range;
	elim2 = pair->r->start + /*len*/range;

	ajListPushApp(*pairlist,(void *)pair);
	count = *npair;
	for(j=i+1;j<*npair;++j)
	{
	    ajListPop(*pairlist,(void **)&pair);
	    fst2=pair->f->start+pair->f->primerlen-1;
	    if((fst2<blim || fst2>elim) && (pair->r->start<blim2 ||
					    pair->r->start>elim2))
		ajListPushApp(*pairlist,(void *)pair);
	    else
	    {
		prima_PrimerDel(&pair->f);
		prima_PrimerDel(&pair->r);
		AJFREE(pair);
		--count;
	    }
	}

	*npair=count;
    }

    return;
}



/* @funcstatic prima_check_overlap ********************************************
**
** Undocumented.
**
** @param [?] pairlist [AjPList*] Undocumented
** @param [?] npair [ajint*] Undocumented
** @param [?] overlap [ajint] Undocumented
** @@
******************************************************************************/


static void prima_check_overlap(AjPList *pairlist, ajint *npair, ajint overlap)
{
    AjPPair pair;
    
    ajint i;
    ajint j;
    ajint end;
    ajint limit;
    ajint count;
    
    for(i=0;i<*npair;++i)
    {
	for(j=0;j<i;++j)
	{
	    ajListPop(*pairlist,(void **)&pair);
	    ajListPushApp(*pairlist,(void *)pair);
	}
	
	ajListPop(*pairlist,(void **)&pair);
	
	end=pair->r->start;
	limit=end-overlap;
	ajListPushApp(*pairlist,(void *)pair);
	
	count = *npair;
	for(j=i+1;j<*npair;++j)
	{
	    ajListPop(*pairlist,(void **)&pair);
	    if(pair->f->start+pair->f->primerlen-1 < limit)
	    {
		prima_PrimerDel(&pair->f);
		prima_PrimerDel(&pair->r);
		--count;
	    }
	    else
		ajListPushApp(*pairlist,(void *)pair);
	}
	
	*npair=count;
    }

    return;
}

/* @funcstatic prima_TwoSortscorepos ******************************************
**
** Sort on basis of score then, within that block on the basis of
** product start then within that block on the basis of product end
** This requires the two functions TwoSortscorepos for the double
** double sort and RevSort, called within that, to sort on the
** primer end position
**
** @param [?] pairlist [AjPList*] Undocumented
** @@
******************************************************************************/

static void prima_TwoSortscorepos(AjPList *pairlist)
{
    AjPPair tmp=NULL;
    AjPList intlist=NULL;
    AjPList filist=NULL;
    AjPPair save=NULL;
    float   score=0.0;
    

    ajListSort(*pairlist,prima_Compare);
    intlist = ajListNew();
    filist  = ajListNew();

    score = (float) -1.0;

    while(ajListPop(*pairlist,(void **)&tmp))
    {
	if(tmp->f->score == score)
	{
	    ajListPush(intlist,(void *)tmp);
	    continue;
	}

	save = tmp;
	ajListSort(intlist,prima_PosCompare);
	score = tmp->f->score;
	prima_RevSort(&intlist);
	
	while(ajListPop(intlist,(void **)&tmp))
	    ajListPushApp(filist,(void *)tmp);
	ajListPush(intlist,(void *)save);
    }

    ajListSort(intlist,prima_PosCompare);
    prima_RevSort(&intlist);
    
    while(ajListPop(intlist,(void **)&tmp))
	ajListPushApp(filist,(void *)tmp);

    ajListDel(&intlist);
    ajListDel(pairlist);

    *pairlist = filist;
    return;
}

/* @funcstatic prima_RevSort **************************************************
**
** See TwoSortscorepos
**
** @param [?] alist [AjPList*] Undocumented
** @@
******************************************************************************/

static void prima_RevSort(AjPList *alist)
{
    AjPPair tmp=NULL;
    AjPList intlist=NULL;
    AjPList filist=NULL;
    AjPPair save=NULL;
    ajint     pos=-1;
    

    intlist = ajListNew();
    filist  = ajListNew();

    pos = -1;

    while(ajListPop(*alist,(void **)&tmp))
    {
	if(tmp->f->start+tmp->f->primerlen == pos)
	{
	    ajListPush(intlist,(void *)tmp);
	    continue;
	}

	save = tmp;
	ajListSort(intlist,prima_PosEndCompare);
	pos = tmp->f->start+tmp->f->primerlen;
	while(ajListPop(intlist,(void **)&tmp))
	    ajListPushApp(filist,(void *)tmp);
	ajListPush(intlist,(void *)save);
    }

    ajListSort(intlist,prima_PosEndCompare);
    while(ajListPop(intlist,(void **)&tmp))
	ajListPushApp(filist,(void *)tmp);

    ajListDel(&intlist);
    ajListDel(alist);

    *alist = filist;
    return;
}

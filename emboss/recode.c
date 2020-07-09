/* @source recode
**
** Find restriction sites in a nucleotide sequence and remove 
** them whilst maintaining the same translation.
** 
**
** This program uses a similar output style to that of
** "silent". Also, the enzyme reading function was taken
** straight out of "silent".
**
** @author: Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*****************************************************************************/ 
                                                                               
#include "emboss.h"
#include <limits.h>

#define IUBFILE "Ebases.iub"

AjIUB aj_base_iubS[256];      /* base letters and their alternatives */


typedef struct AjSRinfo
{
    AjPStr code;              /* structure for RE info */ 
    AjPStr site;
    ajint ncuts;
    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
} AjORinfo, *AjPRinfo;


typedef struct Mutant
{
    AjPStr code;              /* structure for mutation sites */
    AjPStr site;
    ajint match;
    ajint base;
    AjPStr seqaa;
    AjPStr reaa;
    char   obase;
    char   nbase;
} OMutant, *Mutant;



static ajint  readRE(AjPList *relist, AjPStr enzymes);
static AjPList rematch(AjPStr sstr, AjPList ressite, AjPFile outf,
                       AjPStr sname, ajint RStotal, ajint radj, AjBool rev,
                       ajint begin, ajint end, AjBool tshow);
static AjPList checkTrans(AjPStr seq,AjPFile outf,EmbPMatMatch match,
                        AjPRinfo rlp, ajint begin,int radj, AjBool rev,
	                ajint end, ajint pos,AjBool* empty);
static AjBool checkPat(AjPStr seq,AjPFile outf,EmbPMatMatch match, 
                     AjPRinfo rlp, ajint radj, AjBool rev, ajint begin,
                     ajint end);
static ajint  changebase(char pbase, char* tbase);
static void mutFree(Mutant* mut);
static ajint basecompare(const void *a, const void *b);

static void fmt_seq(AjPStr seq, AjPFile outf, ajint start, AjBool num);
static void fmt_muts(AjPList muts, AjPFile outf);
 


int main(int argc, char **argv)
{
    AjPSeq seq=NULL;     
    AjPFile outf=NULL;

    AjPStr sstr=NULL;	
    AjPStr sname=NULL;		     /* seq name */
    AjPStr revcomp=NULL;	     /* rev complement of seq */ 
    AjPStr enzymes=NULL;             /* string for RE selection */    

    ajint    RStotal;
    ajint    begin;                    /* specified start by user */ 
    ajint    end;                      /* specified end by user */ 
    ajint    radj;
    ajint    start; 
    AjBool sshow; 
    AjBool tshow;
 
    AjPList relist=NULL; 
    AjPList muts;
    AjPList nmuts;
    AjPRinfo re;
 

    embInit("recode", argc, argv);
	
    seq = ajAcdGetSeq("seq");             /* sequence to investigate */
    enzymes = ajAcdGetString("enzymes");  /* enzyme list             */  
    sshow  = ajAcdGetBool("sshow");       /* display seq             */
    tshow  = ajAcdGetBool("tshow");       /* display translated seq  */
    outf   = ajAcdGetOutfile("outf");     /* output filename         */


    RStotal=readRE(&relist,enzymes);      /* read in RE info */

    begin = ajSeqBegin(seq);              /* seq start posn, or 1     */
    end   = ajSeqEnd(seq);                /* seq end posn, or seq len */
    radj  = begin+end+1;                  /* posn adjustment for compl seq */

    aj_base_I= 1;

    /* --begin and --end to convert counting from 0-N, not 1-N */
    ajStrAssSubC(&sstr,ajSeqChar(seq),--begin,--end);     
    ajStrToUpper(&sstr);  
	
    sname = ajSeqGetName(seq);	
        
    ajStrAssC(&revcomp,ajStrStr(sstr));	 /* copying seq into revcomp */ 
    ajSeqReverseStr(&revcomp);		 /* getting rev complement   */
    start=begin+1; 

    if(sshow)
    { 
	ajFmtPrintF(outf,"SEQUENCE:\n");
        fmt_seq(sstr,outf,start,ajTrue);
    } 
 
    /******* get de-restriction site *******/
    /******* forward strand          *******/
    muts = rematch(sstr,relist,outf,sname,RStotal,  
                   radj,ajFalse,begin,end,tshow);
 
  
    ajFmtPrintF(outf,"Results for %S:\n\n",sname); 
    ajFmtPrintF(outf,"KEY:\n\tEnzyme\t\tEnzyme name\n"
     "\tRS-Pattern\tRestriction enzyme recognition site pattern\n" 
     "\tMatch-Posn\tPosition of the first base of RS pattern in sequence\n"
     "\tAA\t\tAmino acid. Original sequence(.)After mutation\n"
     "\tBase-Posn\tPosition of base to be mutated in sequence\n"
		"\tMutation\tThe base mutation to perform\n\n");


    ajFmtPrintF(outf,"Creating silent mutations\n\n");
    fmt_muts(muts,outf);
    ajFmtPrintF(outf,"\n\n");
 
       
    if(sshow)
    {
	ajFmtPrintF(outf,"REVERSE COMPLEMENT SEQUENCE:\n");
	fmt_seq(revcomp,outf,start,ajTrue);
    } 

    /******* reverse strand ****************/
    nmuts = rematch(revcomp,relist,outf,sname,RStotal,
                    radj,ajTrue,begin,end,tshow);

    ajFmtPrintF(outf,"\nResults for reverse of %S:\n\n",sname); 
    ajFmtPrintF(outf,"Creating silent mutations\n\n");
    fmt_muts(nmuts,outf);
    ajFmtPrintF(outf,"\n\n");


    while(ajListPop(relist,(void **)&re))
    {
     ajStrDel(&re->code);
     ajStrDel(&re->site);
     AJFREE(re);
    }
    ajListDel(&relist);

    ajSeqDel(&seq);
    ajStrDel(&revcomp);
    ajStrDel(&enzymes); 
    ajStrDel(&sstr);

    ajListDel(&muts);  
    ajListDel(&nmuts);
    
    ajFileClose(&outf);

    ajExit();
    return 0;

}




/* @funcstatic rematch **************************************************
**
** Looks for RE matches and test new bases to find those that
** give the same translation.
**
** returns: Mutant list of those RS sites removed but 
**          maintaining same translation.
*************************************************************************/

static AjPList rematch(AjPStr sstr, AjPList relist, AjPFile outf, 
                 AjPStr sname, ajint RStotal, ajint radj, AjBool rev,
	         ajint begin, ajint end, AjBool tshow)
{ 
    AjPList res;
    AjPList results; 
    AjPStr str;                        /* holds RS patterns */ 
    AjPStr tstr; 
    AjPStr pep=NULL;                   /* string to hold protein */

    AjBool dummy=ajFalse;              /* need a bool for ajPatClassify */
    AjBool empty;

    ajint mm;                            /* no. of mismatches */
    ajint pats;                          /* no. of pattern matches */
    ajint patlen;
    ajint pos;
    ajint aw;                             
    ajint start;                         

    AjPList patlist=NULL;              /* list for pattern matches of.. */
    EmbPMatMatch match;                /* ..AjMatMatch structures*/
    AjPRinfo rlp=NULL;        
    AjPTrn   table=NULL;               /* translation table object */


    str  = ajStrNew();
    tstr = ajStrNew(); 
    pep  = ajStrNew();
    table = ajTrnNewI(0);              /* 0 for std DNA (see fuzztran) */

    results = ajListNew();       
    start = 1; 

    if(!rev)                           /* forward strand */  
    { 
	ajTrnStrFrame(table,sstr,1,&pep); /* frame 1 */
    	if(tshow)
    	{
	    ajFmtPrintF(outf,"\n\nTRANSLATED SEQUENCE:\n"); 
            fmt_seq(pep,outf,start,ajFalse);
	    ajFmtPrintF(outf,"\n\n");
     	}
    }
    else                               /* reverse strand */ 
    {
	ajStrAssC(&tstr,ajStrStr(sstr)+(end-begin+1)%3);
	ajTrnStrFrame(table,tstr,1,&pep);
        if(tshow)
	{
	     ajFmtPrintF(outf,"\n\nREVERSE TRANSLATED SEQUENCE:\n");
	     fmt_seq(pep,outf,start,ajFalse);
   	     ajFmtPrintF(outf,"\n\n");
	}
    }	


    for(aw=0;aw<RStotal;aw++)          /* read in RE patterns */
    {
	                               /* pop off first RE */
	(void) ajListPop(relist,(void **)&rlp);  
        /* ignore unknown cut sites & zero cutters */
       	if(*ajStrStr(rlp->site)=='?'||!rlp->ncuts)     
        {
       	     ajListPushApp(relist,(void*)rlp);
       	     continue;
        } 
        ajStrToUpper(&rlp->site);          /* RS to upper case */ 
        ajStrAss(&str,rlp->site);          /* str holds RS pat */ 
        
	patlist = ajListNew();             /* list for matches */

	/* convert our RS pattern to a reg expression    */
	/* "0" means DNA & so does any ambig conversions */
	
        if(!embPatClassify(&str,&dummy,&dummy,&dummy,
                    &dummy,&dummy,&dummy,0)) continue;

	/* find pattern matches in seq with NO mismatches */
	mm=0;
	pats=embPatBruteForce(&sstr,&str,ajFalse,ajFalse,
                              &patlist,begin+1,mm,&sname);
                    
	if(pats)
        {
            while(ajListPop(patlist,(void**)&match))  
            {
              patlen = match->len;

              if(checkPat(sstr,outf,match,rlp,radj,
                          rev,begin,end))
              {
                  for(pos=0;pos<patlen;pos++)
                  {  
                    res = checkTrans(sstr,outf,match,rlp,begin,
                                     radj,rev,end,pos,&empty);
                    if(empty) 
                        ajListDel(&res);
                    else
                        ajListPushList(results,&res);
                  }
              }
              embMatMatchDel(&match);  
            }
        }

        /* push RE info back to top of the list */
        ajListPushApp(relist,(void *)rlp);
	ajListDel(&patlist);
    }
       
    ajStrDel(&str);
    ajStrDel(&tstr); 
    ajStrDel(&pep);
    ajTrnDel(&table);

    return results; 
}	


/* @funcstatic readRE ***************************************************
**
** Read in RE information from REBASE file.
**
** returns: restriction site information as a list
**
*************************************************************************/
static ajint readRE(AjPList *relist,AjPStr enzymes)
{
    EmbPPatRestrict rptr=NULL;		/* store RE info */
    AjPFile fin=NULL;			/* file pointer to RE file data */
    AjPStr refilename=NULL;		/* .. & string for the filename */
    register ajint RStotal=0;		/* counts no of RE */  
    AjPRinfo rinfo=NULL;
    AjBool isall=ajFalse;
    ajint ne=0;
    ajint i;
    AjPStr *ea=NULL;

  
    refilename=ajStrNewC("REBASE/embossre.enz");

    rptr=embPatRestrictNew();           /* allocate a restrict struc */
        
    *relist=ajListNew();                /* list the RS code and info */

    ajFileDataNew(refilename,&fin);
    if(!fin)
	ajFatal("Aborting...restriction file not found");

    if(!enzymes)                         /* parse user-selected enzyme list */
	isall=ajTrue;
    else
    {
	ne=ajArrCommaList(enzymes,&ea);  /* no. of RE's specified */
                                         /* ea points to enzyme list */
        for(i=0;i<ne;++i)
	    ajStrCleanWhite(&ea[i]);     /* remove all whitespace */
        if(ajStrMatchCaseC(ea[0],"all"))
            isall=ajTrue;
        else
            isall=ajFalse;
    } 

    /* read RE data into AjPRestrict obj */

    while(embPatRestrictReadEntry(&rptr,&fin))
    {  
     	if(!isall)                        /* only select enzymes on command line */
	{  
	     for(i=0;i<ne;++i)
		if(ajStrMatchCase(ea[i],rptr->cod))
			break;
	     if(i==ne)
		continue; 
        }

        AJNEW(rinfo);
        rinfo->code = ajStrNewC(ajStrStr(rptr->cod)); 
  	rinfo->site = ajStrNewC(ajStrStr(rptr->pat));
        rinfo->ncuts= rptr->ncuts;                         
        rinfo->cut1 = rptr->cut1;
        rinfo->cut2 = rptr->cut2;
        rinfo->cut3 = rptr->cut3;
        rinfo->cut4 = rptr->cut4;
	ajListPush(*relist,(void *)rinfo); 
	RStotal++;
    }

    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    AJFREE(ea);
    embPatRestrictDel(&rptr);
    ajFileClose(&fin);
    ajStrDel(&refilename);

    return RStotal;

}



/* @funcstatic checkPat ***************************************************
**
** Checks whether the RS pattern falls within the sequence string
**
*************************************************************************/
static AjBool checkPat(AjPStr seq,AjPFile outf,EmbPMatMatch match, 
            AjPRinfo rlp, ajint radj, AjBool rev, ajint begin, ajint end)
{
    ajint mpos;
    ajint rmpos;

    ajint  min=INT_MAX;             /* reverse sense intentional! */
    ajint  max=-INT_MAX;


    mpos  = match->start;         /* start posn of match in seq */
    rmpos = radj-mpos-match->len; /* start posn of match in rev seq */

    if(rlp->ncuts==4)             /* test if cut site is within seq */
    {
        min = AJMIN(rlp->cut1,rlp->cut2);
        max = AJMAX(rlp->cut3,rlp->cut4);
    }
    else if(rlp->ncuts==2)
    {
        min = AJMIN(rlp->cut1,rlp->cut2);
        max = AJMAX(rlp->cut1,rlp->cut2);
    }
    else
    {
        ajFmtPrintF(outf,"Possibly corrupt RE file\n");
        return ajFalse;
    }

    if(!rev)                      /* forward strand */
    {
        if(mpos+min<0||mpos+max>end+1)
             return ajFalse;     /* cut not in seq */
    }
    else                         /* reverse strand */
    {
        if(radj-mpos-1-min>end+1||radj-mpos-1-max<begin)
             return ajFalse;     /* cut not in seq */
    }
   
    return ajTrue;
}



/* @funcstatic checkTrans ***********************************************
**
** Identify mutations at a site in the RS pattern that result in the 
** same translation.
**
** returns: list of de-restricted sites which maintain same translation.
**          
*************************************************************************/
static AjPList checkTrans(AjPStr seq,AjPFile outf,EmbPMatMatch match, 
                  AjPRinfo rlp, ajint begin,int radj, AjBool rev, 
		  ajint end, ajint pos, AjBool* empty)
{
    char *pseq;
    char *pseqm;
    char *prs;
    char *s;

    Mutant  tresult;
    AjPList res;

    ajint mpos;
    ajint framep;
    ajint i;
    AjPTrn table=NULL;
    AjPStr s1=NULL;
    AjPStr s2=NULL;
    char base;
    char pbase;
    char tbase[4];
    
    ajint  rmpos;
    ajint  x;   
    ajint  nb;



    *empty = ajTrue;
    mpos  = match->start;         /* start posn of match in seq */ 
    rmpos = radj-mpos-match->len; /* start posn of match in rev seq */ 
    
    pseq  = ajStrStr(seq);        /* pointer to start of seq */ 
    pseqm = pseq+mpos-(begin+1);  /* pointer to start of match in seq */ 
    prs   = ajStrStr(rlp->site);  /* pointer to start of RS pattern */ 
    
    framep=(end-begin+1)%3;       /* where frame starts on reverse strand */

    base  = pseqm[pos];           /* store orig seq base */
    pbase = prs[pos];

                                  /* use IUB codes to get other bases */
    nb = changebase(pbase,&tbase[0]);

    x=mpos+pos-(begin+1);

    if(!rev)                      /* forward strand */
      s=pseq+x-x%3;
    else                          /* reverse strand */
      s=pseq+x-(x-framep)%3;

    table = ajTrnNewI(0); 
    s1 = ajStrNewC(ajStrStr(ajTrnCodonC(table,s)));

    res=ajListNew();


    for(i=0;i<nb;i++)             /* try out other bases */
    {
      pseq[x] = tbase[i];
      s2 = ajStrNewC(ajStrStr(ajTrnCodonC(table,s)));

      if(ajStrMatch(s1,s2)){      /* if same translation */
          AJNEW(tresult);
          tresult->obase = base;
          tresult->nbase = tbase[i];
          tresult->code  = ajStrNewC(ajStrStr(rlp->code));
          tresult->site  = ajStrNewC(ajStrStr(rlp->site));
          tresult->seqaa = ajStrNewC(ajStrStr(s1));
          tresult->reaa  = ajStrNewC(ajStrStr(s2));

          if(!rev)
          {
       	    tresult->match = mpos;
            tresult->base  = mpos+pos;
          }
          else
          {
            tresult->match = rmpos;
            tresult->base  = rmpos+match->len-1-pos;
          }  
          ajListPushApp(res,(void *)tresult);
          *empty = ajFalse;
      }
      ajStrDel(&s2);
    }

    pseq[x] = base;          /* resubstitute orig base */

    ajStrDel(&s1);
    ajTrnDel(&table); 

    return res;
 
}



/* @funcstatic changebase ***********************************************
**
** Use IUB code to return alternative nucleotides to that provided
** same translation.
**
** returns: alternative bases in tbase and the number of bases
**
*************************************************************************/
static ajint changebase(char pbase, char* tbase)
{
    ajint setBase[] = {1,1,1,1};
    AjIStr splits = NULL;
    AjPStr bt = NULL;
    char bs;
    ajint i;
    ajint nb;
    ajint len;

    

    ajBaseInit();

    len = ajStrLen(aj_base_iubS[(ajint)pbase].list)-1; 

    bt = ajStrNew();
    ajStrAssI(&bt,aj_base_iubS[(ajint)pbase].list,len);
    splits = ajStrIter(bt);

    while(!ajStrIterDone(splits)) 
    { 
      bs = ajStrIterGetK(splits);

      if( ajAZToBin(bs) & ajAZToBin('G') ) setBase[0] = 0;
      if( ajAZToBin(bs) & ajAZToBin('A') ) setBase[1] = 0;
      if( ajAZToBin(bs) & ajAZToBin('T') ) setBase[2] = 0;
      if( ajAZToBin(bs) & ajAZToBin('C') ) setBase[3] = 0;
      ajStrIterNext(splits);
    }

    ajStrIterFree(&splits);

    nb = 0;
    for(i=0;i<4;i++) 
    {
      if( setBase[i] == 1 )
      {
        if(i==0)
           tbase[nb] = 'G';
        else if(i==1)
           tbase[nb] = 'A';
        else if(i==2)
           tbase[nb] = 'T';
        else if(i==3)
           tbase[nb] = 'C';
        nb++;
      }
    }

    ajStrDel(&bt); 

    return nb;

}



/* @funcstatic fmt_seq **************************************************
**
** Write sequence to the output file.
**
*************************************************************************/
static void fmt_seq(AjPStr seq, AjPFile outf, ajint start, AjBool num)
{
    char *p;
    ajint m;
    ajint i;
    ajint tlen;


    if(num)
    { 
    	p=ajStrStr(seq);
    	ajFmtPrintF(outf,"%-7d",start);
    	tlen=ajStrLen(seq);
    	for(i=0; i<tlen ; i++)
    	{
		ajFmtPrintF(outf,"%c",p[i]);
        	m=i+1;	
        	if(m%10==0)ajFmtPrintF(outf," ");
		if(m%60==0)ajFmtPrintF(outf,"\n%-7d",(start+m+1));
    	} 
    } 
    else
    {
	p=ajStrStr(seq);
        ajFmtPrintF(outf,"%-7d",start);
        tlen=ajStrLen(seq);
        for(i=0; i<tlen ; i++)
        {
                ajFmtPrintF(outf,"%c",p[i]);
                m=i+1; 
                if(m%10==0)ajFmtPrintF(outf," ");
                if(m%60==0)ajFmtPrintF(outf,"\n%-7d",(start+m));
        }
    } 
 
    ajFmtPrintF(outf,"\n");
    return;

}



/* @funcstatic fmt_muts *************************************************
**
** Write de-restricted sites to outputfile
**
*************************************************************************/
static void fmt_muts(AjPList muts, AjPFile outf)
{
    Mutant res;

    ajFmtPrintF(outf,
    "Enzyme      RS-Pattern  Match-Posn   AA  Base-Posn Mutation\n");

    ajListSort(muts,basecompare);

    while(ajListPop(muts,(void **)&res))
    {
       ajFmtPrintF(outf,"%-10S  %-13S  %-8d  %S.%S    %-7d  %c->%c  \n",
		   res->code,res->site,res->match,res->seqaa,res->reaa,
		   res->base,res->obase,res->nbase);	
       mutFree(&res);
    }

    return;

}



/* @funcstatic basecompare **********************************************
**
** Combare 2 base positions in the nucleotide sequence
**
*************************************************************************/
static ajint basecompare(const void *a, const void *b)
{
    return((*(Mutant *)a)->base)-((*(Mutant *)b)->base);
}	
 


/* @funcstatic mutFree **************************************************
**
** Free allocated memory for mutant structure
**
*************************************************************************/
static void mutFree(Mutant* mut)
{
    ajStrDel(&(*mut)->code);
    ajStrDel(&(*mut)->site);   
    ajStrDel(&(*mut)->seqaa);   
    ajStrDel(&(*mut)->reaa); 
    AJFREE(*mut);
}



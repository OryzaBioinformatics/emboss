/* @source silent application
**
** Find silent mutation sites for restriction enzyme/SDM experiments
**
** @author: Copyright (C) Amy Williams (bmbawi@bmb.leeds.ac.uk)
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
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*****************************************************************************/ 
                                                                               
#include "emboss.h"
#include <limits.h>

typedef struct AjSRinfo
{
    AjPStr code;   /*structure for silent mutation info*/ 
    AjPStr site;
    ajint ncuts;
    ajint cut1;
    ajint cut2;
    ajint cut3;
    ajint cut4;
} AjORinfo, *AjPRinfo;


typedef struct AjSSilent
{
    AjPStr code;
    AjPStr site;
    ajint match;
    ajint base;
    AjPStr seqaa;
    AjPStr reaa;
    AjBool issilent; 
    char   obase;
    char   nbase;
} AjOSilent, *AjPSilent;


/* Prototypes */

AjPList mismatch(AjPStr sstr, AjPList ressite, AjPFile outf, AjPStr sname,
                 ajint RStotal,int begin,int radj, AjBool rev, ajint end, 
                 AjBool tshow);
ajint restr_read(AjPList *relist, AjPStr enzymes);
AjBool checktrans(AjPStr seq,AjPFile outf,EmbPMatMatch match, AjPRinfo rlp,
		  ajint begin,int radj, AjBool rev, ajint end, AjPSilent *res);
void fmt_sequence(AjPStr seq, AjPFile outf, ajint start, AjBool num);
void fmt_hits(AjPList hits, AjPFile outf);
void split_hits(AjPList *hits, AjPList *silents, AjPList *nonsilents, 
                AjBool allmut);
static ajint basecompare(const void *a, const void *b);


 




int main(int argc, char **argv)
{
    AjPSeq seq=NULL;		     /*3 things we need from ACD/embInit*/
    AjPFile outf=NULL;
    AjPStr sstr=NULL;		     /*ajPatBruteForce requires the sequence
                                         as an AjPStr so we need one*/
    AjPStr sname=NULL;		     /*and it wants a name as well*/
    AjPStr revcomp=NULL;	     /*to hold reverse complement of sequence*/ 
    ajint RStotal;
    AjPStr enzymes=NULL;                /* string for RE selection */    

    AjPList relist=NULL;
    ajint   begin;                        /* specified start by user */ 
    ajint   end;                          /* specified end by user */ 
    ajint   radj;
    ajint start; 
    AjBool sshow; 
    AjBool tshow;
    AjBool allmut; 
  
    AjPList results1=NULL;              /* for forward strand */
    AjPList results2=NULL;              /* for reverse strand */    
    AjPList shits;
    AjPList nshits;
 
    /*get all user input*/
    embInit("silent", argc, argv);
	
    /*and recover it from somewhere in memory*/
    seq = ajAcdGetSeq("seq");
    enzymes=ajAcdGetString("enzymes");          /* enzymes specified by user */
    sshow=ajAcdGetBool("sshow");
    tshow=ajAcdGetBool("tshow");
    allmut=ajAcdGetBool("allmut"); 
    outf = ajAcdGetOutfile("outf");

    shits=ajListNew();
    nshits=ajListNew();
 
    RStotal=restr_read(&relist,enzymes);        /*calling function to read in 
                                                  RE info*/

    begin = ajSeqBegin(seq);             /* returns the seq start posn, or 1 
                                            if no start has been set */ 
    end   = ajSeqEnd(seq);               /* returns the seq end posn, or seq
                                            length if no end has been set */ 
    radj=begin+end+1;                    /* posn adjustment for complementary 
                                            strand */ 
 
    /*
     *  copy sequence. It constructs as well (if it doesn't exist and it has 
     *  been initialised as NULL)!
     */ 
    ajStrAssSubC(&sstr,ajSeqChar(seq),--begin,--end);     

    /* --begin and --end to convert counting from 0-N, not 1-N */ 
    ajStrToUpper(&sstr);                 /* converts seq to uppercase */ 
	
    sname = ajSeqGetName(seq);		 /*get sequence name. Another 
                                           constructor!*/
        
    ajStrAssC(&revcomp,ajStrStr(sstr));	 /*copying sequence into revcomp*/ 
    ajSeqReverseStr(&revcomp);		 /*getting reverse complement of 
                                           sequence*/ 
    start=begin+1; 
    if(sshow)
    { 
	ajFmtPrintF(outf,"SEQUENCE:\n");
        fmt_sequence(sstr,outf,start,ajTrue);
    } 
 
    results1=mismatch(sstr,relist,outf,sname,RStotal,begin,radj,ajFalse,end,
                      tshow); 
   
    split_hits(&results1,&shits,&nshits,allmut); 

    ajFmtPrintF(outf,"Results for %S:\n\n",sname); 
    ajFmtPrintF(outf,"KEY:\n\tEnzyme\t\tEnzyme name\n"
     "\tRS-Pattern\tRestriction enzyme recognition site pattern\n" 
     "\tMatch-Posn\tPosition of the first base of RS pattern in sequence\n"
     "\tAA\t\tAmino acid. Original sequence(.)After mutation\n"
     "\tBase-Posn\tPosition of base to be mutated in sequence\n"
		"\tMutation\tThe base mutation to perform\n\n");

    ajFmtPrintF(outf,"Silent mutations\n\n");
    fmt_hits(shits,outf);
    ajFmtPrintF(outf,"\n\n");
    if(allmut)
    {
	ajFmtPrintF(outf,"Non-Silent mutations\n\n");
	fmt_hits(nshits,outf);
	ajFmtPrintF(outf,"\n\n");
    }
        
    if(sshow)
    {
	ajFmtPrintF(outf,"REVERSE SEQUENCE:\n");
	fmt_sequence(revcomp,outf,start,ajTrue);
    } 

    results2=mismatch(revcomp,relist,outf,sname,RStotal,begin,radj,ajTrue,end,
                      tshow);

    split_hits(&results2,&shits,&nshits,allmut);

    ajFmtPrintF(outf,"\nResults for reverse of %S:\n\n",sname); 
    ajFmtPrintF(outf,"Silent mutations\n\n");
    fmt_hits(shits,outf);
    ajFmtPrintF(outf,"\n\n");
    if(allmut)
    {
        ajFmtPrintF(outf,"Non-Silent mutations\n\n");
	fmt_hits(nshits,outf);
 	ajFmtPrintF(outf,"\n\n");
    }

    ajSeqDel(&seq);
    ajStrDel(&revcomp);
    ajStrDel(&enzymes);    
    
    ajListDel(&results1);   
    ajListDel(&results2);
    ajListDel(&shits);  
    ajListDel(&nshits);  
    
    ajFileClose(&outf);
    ajExit();
    return 0;
}








AjPList mismatch(AjPStr sstr, AjPList relist, AjPFile outf, AjPStr sname,
	         ajint RStotal,int begin,int radj,AjBool rev,int end,
                 AjBool tshow)
{ 
    AjPSilent res;
    AjPList results; 
    AjPStr str;                        /*holds RS patterns*/ 
    AjPStr tstr; 
    AjBool dummy=ajFalse;              /*need a bool for ajPatClassify*/
    ajint mm;                            /*number of mismatches*/
    ajint hits;                          /*number of pattern matches found*/
    ajint i;
    ajint aw;                             
    ajint start;                         
    AjPList patlist=NULL;              /*a list for pattern matches. 
                                             a list of ..*/
    EmbPMatMatch match;                /*..AjMatMatch structures*/
    AjPRinfo rlp=NULL;        
    AjPStr pep=NULL;                   /*string to hold protein*/
    AjPTrn table=NULL;                 /*object to hold translation table*/

    str=ajStrNew();
    tstr=ajStrNew(); 
    pep=ajStrNew();
    table=ajTrnNewI(0);                /*0 stands for standard DNA-
                                         see fuzztran.acd*/
    results=ajListNew();       
    start=1; 
    if(!rev)                           /* forward strand */  
    { 
	ajTrnStrFrame(table,sstr,1,&pep); /*1 stands for frame number*/
    	if(tshow)
    	{
		ajFmtPrintF(outf,"\n\nTRANSLATED SEQUENCE:\n"); 
                fmt_sequence(pep,outf,start,ajFalse);
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
		fmt_sequence(pep,outf,start,ajFalse);
   		ajFmtPrintF(outf,"\n\n");
	}
    }	

    for(aw=0;aw<RStotal;aw++)          /* loop to read in restriction 
                                              enzyme patterns */ 
    {
	/* Pop off the first RE */
	(void) ajListPop(relist,(void **)&rlp);  
        /* ignore unknown cut sites and zero cutters */
       	if(*ajStrStr(rlp->site)=='?'||!rlp->ncuts)     
        {
           	ajListPushApp(relist,(void*)rlp);
           	continue;
         } 
         ajStrToUpper(&rlp->site);          /* convert all RS to upper case */ 
         ajStrAss(&str,rlp->site);          /* str now holds RS patterns */ 
        
   	/*our pattern matching will return a list of matches so allocate our 
	  list*/
	 patlist = ajListNew();

   	/*and allocate the AjPStr structs. These ones do the ajStrNew()s in 
          the library .. a lot of functions don't*/
	/*convert our pattern string to a regular expression*/
	/*the "0" means its DNA and so it'll do any ambig conversions*/
	
        if(!embPatClassify(&str,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,0))
	    continue;

	/*now see if the pattern typed matches the sequence with NO mismatches
        */ 

	mm=0;
	hits=embPatBruteForce(&sstr,&str,ajFalse,ajFalse,&patlist,begin+1,mm,
                              &sname);
                    
	if(hits)
        {
            	while(ajListPop(patlist,(void**)&match))
		    embMatMatchDel(&match);
        }
 	else
        { 
		mm=1;
          	hits=embPatBruteForce(&sstr,&str,ajFalse,ajFalse,&patlist,
                                      begin+1,mm,&sname);

		if(hits)
		for(i=0;i<hits;++i)
		{
                	(void)ajListPop(patlist,(void**)&match);
 			if(checktrans(sstr,outf,match,rlp,begin,radj,rev,end,
                                      &res)) 
			    ajListPushApp(results,(void *)res);
			embMatMatchDel(&match);
         	}
   	}  
        
        /* Push our RE info back to the list, but on top of it */
        ajListPushApp(relist,(void *)rlp);

	ajListDel(&patlist);
    }
       
    ajStrDel(&str);
    ajStrDel(&tstr); 
    ajStrDel(&pep);
    ajTrnDel(&table);
    return (results);   
}	








ajint restr_read(AjPList *relist,AjPStr enzymes)
{
    EmbPPatRestrict rptr=NULL;		/*somewhere to store RE info*/
    AjPFile fin=NULL;			/*we need an input file pointer to read
                                          in the RE file data*/
    AjPStr refilename=NULL;		/*...and a string for the filename*/
    register ajint RStotal=0;		/*counts no of RE*/  
    AjPRinfo rinfo=NULL;
    AjBool isall=ajFalse;
    ajint ne=0;
    ajint i;
    AjPStr *ea=NULL;
    
    /*open the RE file*/
    /*first allocate a string and at the same time initialise it*/
    refilename=ajStrNewC("REBASE/embossre.enz");
    /*keeping all constructors together now allocate a restrict structure*/
    rptr=embPatRestrictNew();
        
    /* and a list on which to store the code and site info */
    *relist=ajListNew();

    /*now open re file. note that the function below will construct an AjPFile
      so you don't need to do an ajFileNewIn() call*/

    ajFileDataNew(refilename,&fin);
    /*if fin is NULL then the file wasn't found*/
    if(!fin)
	ajFatal("Aborting...restriction file not found");

    /* Parse the user-selected enzyme list */
    if(!enzymes)
	isall=ajTrue;
    else
    {
	ne=ajArrCommaList(enzymes,&ea);       /* ne=number of enzymes specified
                                                 by users */ 
        for(i=0;i<ne;++i)                     /* ea is a pointer to AjPStr 
                                                 array created enzyme list */ 
	    ajStrCleanWhite(&ea[i]);          /* removes all whitespace from a
                                                 string */ 
        if(ajStrMatchCaseC(ea[0],"all"))
            isall=ajTrue;
        else
            isall=ajFalse;
    } 

    /*now we just go into a loop reading the RE data into the AjPRestrict 
      object using our open file pointer*/ 

    while(embPatRestrictReadEntry(&rptr,&fin))
    {  
	/* Only select the enzymes on the command line */
     	if(!isall)
	{  
		for(i=0;i<ne;++i)
		if(ajStrMatchCase(ea[i],rptr->cod))
			break;
	    	if(i==ne)
			continue; 
        }

        AJNEW(rinfo);
        /* reading in RE info into rinfo from EmbPPatRestrict structure */ 
        rinfo->code=ajStrNewC(ajStrStr(rptr->cod)); 
	rinfo->site=ajStrNewC(ajStrStr(rptr->pat));
        rinfo->ncuts=rptr->ncuts;                         
        rinfo->cut1=rptr->cut1;
        rinfo->cut2=rptr->cut2;
        rinfo->cut3=rptr->cut3;
        rinfo->cut4=rptr->cut4;
	ajListPush(*relist,(void *)rinfo); 
	RStotal++;
    }

    /* tidy up and return no of REs */
    for(i=0;i<ne;++i)
	ajStrDel(&ea[i]);
    AJFREE(ea);

    embPatRestrictDel(&rptr);
    ajFileClose(&fin);
    ajStrDel(&refilename);
    return RStotal;
}







AjBool checktrans(AjPStr seq,AjPFile outf,EmbPMatMatch match, AjPRinfo rlp,
		  ajint begin,int radj, AjBool rev, ajint end, AjPSilent *res)
{
    char *p=NULL;
    char *q=NULL; 
    char *s=NULL;
    char *t;
    char *u;
    ajint matchpos;
    ajint framep;

    ajint count;
    AjPTrn table=NULL;
    AjPStr s1=NULL;
    AjPStr s2=NULL;
    char c;
    char rc;
    ajint  min=INT_MAX;          /* Reverse sense intentional! */ 
    ajint  max=-INT_MAX;
    ajint fpos;
    ajint rpos;
    ajint x;    
 
    matchpos = match->start;          /* posn of start of match in seq */ 
    fpos=matchpos;
    rpos=radj-fpos-match->len;        /* posn of start of match in rev seq */ 

    t = ajStrStr(seq);                /* pointer to start of seq */ 
    
    p=t+fpos-(begin+1);               /* pointer to start of match in seq */ 

    u=q=ajStrStr(rlp->site);          /* pointer to start of RS pattern */ 
    
    /* Test here for whether cut site is within sequence substring */
    if(rlp->ncuts==4)
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
  
    if(!rev)                  /* forward strand */ 
    {
	if(matchpos+min<0||matchpos+max>end+1)
	{
		/*Cut site not in sequence range*/	
		return ajFalse; 
	}
    }
    else                       /* reverse strand */ 
    {
	if(radj-matchpos-1-min>end+1||radj-matchpos-1-max<begin)
	{
		/*Cut site not in sequence range*/
		return ajFalse;
	}
    } 

 
    count=0;   /* loop to count to base which is different in seq and RS,
                  starting from beginning of match */ 
    while(ajAZToBin(*q++) & ajAZToBin(*p++))
          ++count;

    /* Changed base postion */
    x=fpos+count-(begin+1);
    /* Where the frame starts on the reverse strand */
    framep=(end-begin+1)%3;

    c = t[x];         /* store base in seq which is different in RS pattern */
    rc = u[count];    /* store base in RS pattern that is different in seq */ 

    if(!rev)            /* forward strand */ 
	s=t+x-x%3;
    else                /* reverse strand */ 
        s=t+x-(x-framep)%3;
 
    table=ajTrnNewI(0);
 
    /* translates codon pointed to by s (original seq) */ 
    s1 = ajStrNewC(ajStrStr(ajTrnCodonC(table,s)));    

    t[x] = rc;        /* changes base in seq to one in equivalent posn in RS 
                         pattern */ 
    
    /*  translates codon pointed to by s (mutated base from RS pattern */ 
    s2 = ajStrNewC(ajStrStr(ajTrnCodonC(table,s)));  

    t[x] = c;  /* changes mutated base in seq back to original base */  

    AJNEW(*res);
    (*res)->obase = c;
    (*res)->nbase = rc;
    (*res)->code=ajStrNewC(ajStrStr(rlp->code));
    (*res)->site=ajStrNewC(ajStrStr(rlp->site));
    (*res)->seqaa=ajStrNewC(ajStrStr(s1));
    (*res)->reaa=ajStrNewC(ajStrStr(s2));
    if(ajStrMatch(s1,s2))
	(*res)->issilent=ajTrue;
    else
	(*res)->issilent=ajFalse; 
    if(!rev)
    {
       	(*res)->match=matchpos;
        (*res)->base=matchpos+count;
    }
    else
    {
	(*res)->match=rpos;
	(*res)->base=rpos+match->len-1-count;
    }  
 
    ajStrDel(&s1);
    ajStrDel(&s2);
    ajTrnDel(&table);  
    return ajTrue; 
}








void fmt_sequence(AjPStr seq, AjPFile outf, ajint start, AjBool num)
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








void fmt_hits(AjPList hits, AjPFile outf)
{
    AjPSilent res;

    ajFmtPrintF(outf,
    "Enzyme      RS-Pattern  Match-Posn   AA  Base-Posn Mutation\n");

    ajListSort(hits,basecompare);

    while(ajListPop(hits,(void **)&res))
    {
       ajFmtPrintF(outf,"%-10S  %-13S  %-8d  %S.%S    %-7d  %c->%c  \n",
		   res->code,res->site,res->match,res->seqaa,res->reaa,
		   res->base,res->obase,res->nbase);	
       ajStrDel(&res->code);
       ajStrDel(&res->site);
       ajStrDel(&res->seqaa); 
       ajStrDel(&res->reaa);
       AJFREE(res);
    }

    return;
}








void split_hits(AjPList *hits, AjPList *silents, AjPList *nonsilents, 
                AjBool allmut)
{
    AjPSilent res;

    while(ajListPop(*hits,(void **)&res))
    {
	if(res->issilent)
	{
           	ajListPushApp(*silents,(void *)res);
	   	continue;
	}
	if(allmut)
           	ajListPushApp(*nonsilents,(void *)res);
	else
	{
	    ajStrDel(&res->code);
	    ajStrDel(&res->site);
	    ajStrDel(&res->seqaa); 
	    ajStrDel(&res->reaa);
	    AJFREE(res);
	}
    }

    return;
}








static ajint basecompare(const void *a, const void *b)
{
    return((*(AjPSilent *)a)->base)-((*(AjPSilent *)b)->base);
}	
 

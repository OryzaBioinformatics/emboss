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
******************************************************************************/

#include "emboss.h"
#include <limits.h>




/* @datastatic AjPRinfo *******************************************************
**
** recoder internals for RE information
**
** @alias AjSRinfo
** @alias AjORinfo
**
** @attr code [AjPStr] Undocumented
** @attr site [AjPStr] Undocumented
** @attr ncuts [ajint] Undocumented
** @attr cut1 [ajint] Undocumented
** @attr cut2 [ajint] Undocumented
** @attr cut3 [ajint] Undocumented
** @attr cut4 [ajint] Undocumented
******************************************************************************/

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




/* @datastatic AjPSilent ******************************************************
**
** recoder internals for silent sites
**
** @alias AjSSilent
** @alias AjOSilent
**
** @attr code [AjPStr] Undocumented
** @attr site [AjPStr] Undocumented
** @attr match [ajint] Undocumented
** @attr base [ajint] Undocumented
** @attr seqaa [AjPStr] Undocumented
** @attr reaa [AjPStr] Undocumented
** @attr issilent [AjBool] Undocumented
** @attr obase [char] Undocumented
** @attr nbase [char] Undocumented
******************************************************************************/

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




static AjPList silent_mismatch(AjPStr sstr, AjPList ressite, AjPStr* tailstr,
			       AjPStr sname, ajint RStotal, ajint begin,
			       ajint radj, AjBool rev, ajint end,
			       AjBool tshow);
static ajint silent_restr_read(AjPList *relist, AjPStr enzymes);
static AjBool silent_checktrans(AjPStr seq,EmbPMatMatch match,
				AjPRinfo rlp, ajint begin, ajint radj,
				AjBool rev, ajint end, AjPSilent *res);
static void silent_fmt_sequence(char* title, AjPStr seq, AjPStr* tailstr,
				ajint start, AjBool num);
static void silent_fmt_hits(AjPList hits, AjPFeattable feat,
			    AjBool silent, AjBool rev);
static void silent_split_hits(AjPList *hits, AjPList *silents,
			      AjPList *nonsilents, AjBool allmut);
static ajint silent_basecompare(const void *a, const void *b);




/* @prog silent ***************************************************************
**
** Silent mutation restriction enzyme scan
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeq seq    = NULL;
    AjPReport report = NULL;
    AjPFeattable feat=NULL;
    AjPStr sstr   = NULL;

    AjPStr sname   = NULL;
    AjPStr revcomp = NULL;
    ajint RStotal;
    AjPStr enzymes = NULL;                /* string for RE selection */

    AjPList relist = NULL;
    ajint begin;
    ajint end;
    ajint radj;
    ajint start;
    AjBool sshow;
    AjBool tshow;
    AjBool allmut;

    AjPList results1 = NULL;              /* for forward strand */
    AjPList results2 = NULL;              /* for reverse strand */
    AjPList shits;
    AjPList nshits;
    AjPStr tailstr = NULL;



    embInit("silent", argc, argv);

    seq     = ajAcdGetSeq("sequence");
    enzymes = ajAcdGetString("enzymes");
    sshow   = ajAcdGetBool("sshow");
    tshow   = ajAcdGetBool("tshow");
    allmut  = ajAcdGetBool("allmut");
    report = ajAcdGetReport ("outfile");

    shits  = ajListNew();
    nshits = ajListNew();

    /*calling function to read in RE info*/
    RStotal = silent_restr_read(&relist,enzymes);

    begin = ajSeqBegin(seq);             /* returns the seq start posn, or 1
                                            if no start has been set */
    end   = ajSeqEnd(seq);               /* returns the seq end posn, or seq
                                            length if no end has been set */
    radj=begin+end+1;                    /* posn adjustment for complementary
                                            strand */


    ajStrAssSubC(&sstr,ajSeqChar(seq),--begin,--end);
    ajStrToUpper(&sstr);

    sname = ajSeqGetName(seq);
    ajStrAssC(&revcomp,ajStrStr(sstr));
    ajSeqReverseStr(&revcomp);
    start  = begin+1;

    feat = ajFeattableNewDna(ajSeqGetName(seq));

    if(sshow)
    {
        silent_fmt_sequence("SEQUENCE", sstr,&tailstr,start,ajTrue);
    }

    results1 = silent_mismatch(sstr,relist,&tailstr,sname,RStotal,begin,radj,
			       ajFalse,end,tshow);

    silent_split_hits(&results1,&shits,&nshits,allmut);

    ajReportSetHeaderC(report,
		       "KEY:\n"
		       "EnzymeName: Enzyme name\n"
		       "RS-Pattern: Restriction enzyme recognition site "
		       "pattern\n"
		       "Base-Posn: Position of base to be mutated\n"
		       "AAs: Amino acid. Original sequence(.)After mutation\n"
		       "Silent: Yes for unchanged amino acid\n"
		       "Mutation: The base mutation to perform\n\n"
		       "Creating silent and non-silent mutations\n");

    silent_fmt_hits(shits,feat, ajTrue, ajFalse);
    if(allmut)
    {
	silent_fmt_hits(nshits,feat, ajFalse, ajFalse);
    }

    if(sshow)
    {
	silent_fmt_sequence("REVERSE SEQUENCE", revcomp,&tailstr,start,ajTrue);
    }

    results2 = silent_mismatch(revcomp,relist,&tailstr,
			       sname,RStotal,begin,radj,
			       ajTrue,end,tshow);

    silent_split_hits(&results2,&shits,&nshits,allmut);

    silent_fmt_hits(shits,feat, ajTrue, ajTrue);
    if(allmut)
    {
	silent_fmt_hits(nshits,feat, ajFalse, ajTrue);
    }

    ajReportSetTail(report, tailstr);
    (void) ajReportWrite (report,feat,seq);
    ajFeattableDel(&feat);

    ajSeqDel(&seq);
    ajStrDel(&revcomp);
    ajStrDel(&enzymes);

    ajListDel(&results1);
    ajListDel(&results2);
    ajListDel(&shits);
    ajListDel(&nshits);

    ajReportClose(report);
    ajReportDel(&report);

    ajExit();

    return 0;
}







/* @funcstatic silent_mismatch ************************************************
**
** Undocumented.
**
** @param [r] sstr [AjPStr] sequence
** @param [r] relist [AjPList] restriction enzymes
** @param [w] tailstr [AjPStr*] Report tail as a string
** @param [r] sname [AjPStr] sequence names
** @param [r] RStotal [ajint] number of REs
** @param [r] begin [ajint] start position
** @param [r] radj [ajint] reverse sequence numbering adjustment
** @param [r] rev [AjBool] do complement
** @param [r] end [ajint] end position
** @param [r] tshow [AjBool] show translated sequence
** @return [AjPList] hit list
** @@
******************************************************************************/

static AjPList silent_mismatch(AjPStr sstr, AjPList relist, AjPStr* tailstr,
			       AjPStr sname, ajint RStotal, ajint begin,
			       ajint radj,AjBool rev, ajint end,
			       AjBool tshow)
{
    AjPSilent res;
    AjPList results;
    AjPStr str;                          /*holds RS patterns*/
    AjPStr tstr;
    AjBool dummy = ajFalse;              /*need a bool for ajPatClassify*/
    ajint mm;                            /*number of mismatches*/
    ajint hits;                          /*number of pattern matches found*/
    ajint i;
    ajint aw;
    ajint start;
    AjPList patlist = NULL;            /*a list for pattern matches.
                                             a list of ..*/
    EmbPMatMatch match;                /*..AjMatMatch structures*/
    AjPRinfo rlp = NULL;
    AjPStr pep   = NULL;               /*string to hold protein*/
    AjPTrn table = NULL;               /*object to hold translation table*/

    str   = ajStrNew();
    tstr  = ajStrNew();
    pep   = ajStrNew();
    table = ajTrnNewI(0);                /*0 stands for standard DNA-
                                         see fuzztran.acd*/
    results = ajListNew();
    start = 1;
    if(!rev)                           /* forward strand */
    {
	ajTrnStrFrame(table,sstr,1,&pep); /*1 stands for frame number*/
    	if(tshow)
    	{
                silent_fmt_sequence("TRANSLATED SEQUENCE",
				    pep,tailstr,start,ajFalse);
     	}
    }
    else                               /* reverse strand */
    {
	ajStrAssC(&tstr,ajStrStr(sstr)+(end-begin+1)%3);
	ajTrnStrFrame(table,tstr,1,&pep);
        if(tshow)
	{
		silent_fmt_sequence("REVERSE TRANSLATED SEQUENCE",
				    pep,tailstr,start,ajFalse);
	}
    }

    for(aw=0;aw<RStotal;aw++)          /* loop to read in restriction
					  enzyme patterns */
    {
	/* Pop off the first RE */
	ajListPop(relist,(void **)&rlp);
        /* ignore unknown cut sites and zero cutters */
       	if(*ajStrStr(rlp->site)=='?'||!rlp->ncuts)
        {
	    ajListPushApp(relist,(void*)rlp);
	    continue;
	}
	ajStrToUpper(&rlp->site);   /* convert all RS to upper case */
	ajStrAss(&str,rlp->site);      /* str now holds RS patterns */

	patlist = ajListNew();
        if(!embPatClassify(&str,&dummy,&dummy,&dummy,&dummy,&dummy,&dummy,0))
	    continue;


	mm = 0;
	hits=embPatBruteForce(&sstr,&str,ajFalse,ajFalse,&patlist,begin+1,mm,
                              &sname);

	if(hits)
	    while(ajListPop(patlist,(void**)&match))
		embMatMatchDel(&match);
 	else
        {
	    mm = 1;
	    hits = embPatBruteForce(&sstr,&str,ajFalse,ajFalse,&patlist,
				  begin+1,mm,&sname);

	    if(hits)
		for(i=0;i<hits;++i)
		{
		    ajListPop(patlist,(void**)&match);
		    if(silent_checktrans(sstr,match,rlp,begin,radj,
					 rev,end,&res))
			ajListPushApp(results,(void *)res);
		    embMatMatchDel(&match);
         	}
   	}

        ajListPushApp(relist,(void *)rlp);

	ajListDel(&patlist);
    }
    
    ajStrDel(&str);
    ajStrDel(&tstr);
    ajStrDel(&pep);
    ajTrnDel(&table);
    return (results);
}




/* @funcstatic silent_restr_read **********************************************
**
** Read restriction enzyme data
**
** @param [w] relist [AjPList*] restriction enzyme data
** @param [r] enzymes [AjPStr] restriction enzyme data to fetch
** @return [ajint] number of enzymes read
** @@
******************************************************************************/

static ajint silent_restr_read(AjPList *relist,AjPStr enzymes)
{
    EmbPPatRestrict rptr = NULL;
    AjPFile fin = NULL;

    AjPStr refilename = NULL;
    register ajint RStotal = 0;
    AjPRinfo rinfo = NULL;
    AjBool isall = ajFalse;
    ajint ne = 0;
    ajint i;
    AjPStr *ea = NULL;

    refilename = ajStrNewC("REBASE/embossre.enz");
    rptr       = embPatRestrictNew();
    *relist    = ajListNew();

    ajFileDataNew(refilename,&fin);
    if(!fin)
	ajFatal("Aborting...restriction file not found");

    /* Parse the user-selected enzyme list */
    if(!enzymes)
	isall = ajTrue;
    else
    {
	ne = ajArrCommaList(enzymes,&ea);
        for(i=0;i<ne;++i)
	    ajStrCleanWhite(&ea[i]);

        if(ajStrMatchCaseC(ea[0],"all"))
            isall = ajTrue;
        else
            isall = ajFalse;
    }

    while(embPatRestrictReadEntry(&rptr,&fin))
    {
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
        rinfo->code  = ajStrNewC(ajStrStr(rptr->cod));
	rinfo->site  = ajStrNewC(ajStrStr(rptr->pat));
        rinfo->ncuts = rptr->ncuts;
        rinfo->cut1  = rptr->cut1;
        rinfo->cut2  = rptr->cut2;
        rinfo->cut3  = rptr->cut3;
        rinfo->cut4  = rptr->cut4;
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




/* @funcstatic silent_checktrans **********************************************
**
** Determine whether silent mutation exists
**
** @param [r] seq [AjPStr] sequence
** @param [r] match [EmbPMatMatch] pattern match
** @param [r] rlp [AjPRinfo] RE information
** @param [r] begin [ajint] start position
** @param [r] radj [ajint] reverse numbering adjustment
** @param [r] rev [AjBool] do complement
** @param [r] end [ajint] end position
** @param [w] res [AjPSilent*] Undocumented
** @return [AjBool] true if silent found
** @@
******************************************************************************/

static AjBool silent_checktrans(AjPStr seq,EmbPMatMatch match,
				AjPRinfo rlp, ajint begin, ajint radj,
				AjBool rev, ajint end, AjPSilent *res)
{
    char *p = NULL;
    char *q = NULL;
    char *s = NULL;
    char *t;
    char *u;
    ajint matchpos;
    ajint framep;

    ajint count;
    AjPTrn table = NULL;
    AjPStr s1 = NULL;
    AjPStr s2 = NULL;
    char c;
    char rc;
    ajint  min = INT_MAX;          /* Reverse sense intentional! */
    ajint  max = -INT_MAX;
    ajint fpos;
    ajint rpos;
    ajint x;

    matchpos = match->start;
    fpos = matchpos;
    rpos=radj-fpos-match->len;
    t = ajStrStr(seq);

    p = t+fpos-(begin+1);

    u = q = ajStrStr(rlp->site);

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
        ajWarn("Possibly corrupt RE file");
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


    count=0;
    while(ajAZToBin(*q++) & ajAZToBin(*p++))
          ++count;

    /* Changed base postion */
    x = fpos+count-(begin+1);
    /* Where the frame starts on the reverse strand */
    framep = (end-begin+1)%3;

    c  = t[x];
    rc = u[count];

    if(!rev)            /* forward strand */
	s = t+x-x%3;
    else                /* reverse strand */
        s = t+x-(x-framep)%3;

    table = ajTrnNewI(0);

    /* translates codon pointed to by s (original seq) */
    s1 = ajStrNewC(ajStrStr(ajTrnCodonC(table,s)));

    t[x] = rc;

    /*  translates codon pointed to by s (mutated base from RS pattern */
    s2 = ajStrNewC(ajStrStr(ajTrnCodonC(table,s)));

    t[x] = c;  /* changes mutated base in seq back to original base */

    AJNEW(*res);
    (*res)->obase = c;
    (*res)->nbase = rc;
    (*res)->code  = ajStrNewC(ajStrStr(rlp->code));
    (*res)->site  = ajStrNewC(ajStrStr(rlp->site));
    (*res)->seqaa = ajStrNewC(ajStrStr(s1));
    (*res)->reaa  = ajStrNewC(ajStrStr(s2));
    if(ajStrMatch(s1,s2))
	(*res)->issilent = ajTrue;
    else
	(*res)->issilent = ajFalse;
    if(!rev)
    {
       	(*res)->match = matchpos;
        (*res)->base  = matchpos+count;
    }
    else
    {
	(*res)->match = rpos;
	(*res)->base  = rpos+match->len-1-count;
    }

    ajStrDel(&s1);
    ajStrDel(&s2);
    ajTrnDel(&table);

    return ajTrue;
}




/* @funcstatic silent_fmt_sequence ********************************************
**
** Output sequence information
**
** @param [r] seq [AjPStr] sequence
** @param [w] outf [AjPFile] outfile
** @param [r] start [ajint] start position
** @param [r] num [AjBool] Adjust numbering
** @@
******************************************************************************/

static void silent_fmt_sequence(char* title,
				AjPStr seq, AjPStr* tailstr, ajint start,
				AjBool num)
{
    char *p;
    ajint m;
    ajint i;
    ajint tlen;

    ajFmtPrintAppS(tailstr,"%s:\n",title);
    if(num)
    {
    	p = ajStrStr(seq);
    	ajFmtPrintAppS(tailstr,"%-7d",start);
    	tlen = ajStrLen(seq);
    	for(i=0; i<tlen ; i++)
    	{
		ajFmtPrintAppS(tailstr,"%c",p[i]);
        	m=i+1;
        	if(m%10==0)
		    ajFmtPrintAppS(tailstr," ");
		if(m%60==0 && m<tlen)
		    ajFmtPrintAppS(tailstr,"\n%-7d",(start+m+1));
    	}
    }
    else
    {
	p = ajStrStr(seq);
        tlen = ajStrLen(seq);
        for(i=0; i<tlen ; i++)
        {
                ajFmtPrintAppS(tailstr,"%c",p[i]);
                m=i+1;
                if(m%10==0)
		    ajFmtPrintAppS(tailstr," ");
                if(m%60==0 && m<tlen)
		    ajFmtPrintAppS(tailstr,"\n");
        }
    }

    ajFmtPrintAppS(tailstr,"\n\n");

    return;
}




/* @funcstatic silent_fmt_hits ************************************************
**
** Output silent mutation information
**
** @param [r] hits [AjPList] results
** @param [w] feat [AjPFeattable] Feature table object
** @param [r] silent [AjBool] Silent mutation
** @param [r] rev [AjBool] Reverse direction
** @@
******************************************************************************/

static void silent_fmt_hits(AjPList hits, AjPFeattable feat,
			    AjBool silent, AjBool rev)
{
    AjPSilent res;
    AjPFeature sf = NULL;
    AjPStr tmpFeatStr = NULL;

    ajListSort(hits,silent_basecompare);

    while(ajListPop(hits,(void **)&res))
    {
	if (rev)
	{
	    sf = ajFeatNewIIRev(feat,
				res->match, res->match+ajStrLen(res->site)-1);
	    ajFmtPrintS(&tmpFeatStr, "*dir Rev");
	    ajFeatTagAdd (sf, NULL, tmpFeatStr);
	}
	else
	{
	    sf = ajFeatNewII(feat,
			     res->match, res->match+ajStrLen(res->site)-1);
	}
	if (silent)
	{
	    ajFmtPrintS(&tmpFeatStr, "*silent Yes");
	    ajFeatTagAdd (sf, NULL, tmpFeatStr);
	}
	ajFmtPrintS(&tmpFeatStr, "*enzyme %S", res->code);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*rspattern %S", res->site);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*baseposn %d", res->base);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*aa %S.%S", res->seqaa, res->reaa);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	ajFmtPrintS(&tmpFeatStr, "*mutation %c->%c", res->obase,res->nbase);
	ajFeatTagAdd (sf, NULL, tmpFeatStr);
	
       ajStrDel(&res->code);
       ajStrDel(&res->site);
       ajStrDel(&res->seqaa);
       ajStrDel(&res->reaa);
       AJFREE(res);
    }

    ajStrDel(&tmpFeatStr);
    return;
}




/* @funcstatic silent_split_hits **********************************************
**
** Divide hitlist into separate silent/non-silent lists
**
** @param [r] hits [AjPList*] hitlist
** @param [w] silents [AjPList*] silent hits
** @param [w] nonsilents [AjPList*] non-silent hits
** @param [r] allmut [AjBool] do non-silents
** @@
******************************************************************************/

static void silent_split_hits(AjPList *hits, AjPList *silents,
			      AjPList *nonsilents, AjBool allmut)
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




/* @funcstatic silent_basecompare *********************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] Undocumented
** @param [r] b [const void*] Undocumented
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint silent_basecompare(const void *a, const void *b)
{
    return((*(AjPSilent *)a)->base)-((*(AjPSilent *)b)->base);
}

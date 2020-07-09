/* @source fuzznuc application
 **
 ** Finds fuzzy patterns in nucleic acid sequences
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
#include "stdlib.h"


static void fuzznuc_print_hits(AjPList *l, ajint hits, AjPFile outf,
			       AjPStr seq, AjBool mms, AjBool sc, ajint thits,
			       ajint adj, ajint begin, AjPStr desc,
			       AjBool dodesc, AjPStr acc, AjBool doacc,
			       AjPStr usa, AjBool dousa);


/* @prog fuzznuc **************************************************************
**
** Nucleic acid pattern search
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPFile outf;
    AjPStr pattern=NULL;
    AjPStr opattern=NULL;
    AjPStr seqname=NULL;
    AjPStr text=NULL;
    AjPStr desc=NULL;
    AjBool dodesc;
    AjPStr acc=NULL;
    AjBool doacc;
    AjPStr usa=NULL;
    AjBool dousa;
    
    AjPList l;
    
    ajint plen;
    ajint mismatch;
    ajint thits=0;
    
    AjBool amino;
    AjBool carboxyl;
    AjBool mms;
    AjBool sc;
    ajint    type=0;

    ajint    hits=0;
    ajint    m;
    ajint    i;
    ajint    begin;
    ajint    end;
    ajint    adj;
    
    EmbOPatBYPNode off[AJALPHA];
    ajuint         *sotable=NULL;
    ajuint         solimit;
    AjPStr	   regexp=NULL;
    ajint          **skipm=NULL;
    ajint          *buf=NULL;    

    void   *tidy=NULL;

    embInit ("fuzznuc", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequence");
    outf     = ajAcdGetOutfile("outf");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");
    mms      = ajAcdGetBool("mmshow");
    sc       = ajAcdGetBool("complement");
    dodesc   = ajAcdGetBool("descshow");
    doacc    = ajAcdGetBool("accshow");
    dousa    = ajAcdGetBool("usashow");
    
    seqname = ajStrNew();
    opattern=ajStrNew();


    /* Copy original pattern for dear Henry */
    ajStrAssC(&opattern,ajStrStr(pattern));

    if(!(type=embPatGetType(&pattern,mismatch,0,&m,&amino,&carboxyl)))
	ajFatal("Illegal pattern");
    embPatCompile(type,pattern,opattern,&plen,&buf,off,&sotable,&solimit,&m,
		  &regexp,&skipm,mismatch);
    
    text = ajStrNew();
    

    while(ajSeqallNext(seqall,&seq))
    {
	l = ajListNew();
	ajStrAssC(&seqname,ajSeqName(seq));
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	ajStrAssSubC(&text,ajSeqChar(seq),begin-1,end-1);
	ajStrToUpper(&text);
	adj = begin+end+1;

	embPatFuzzSearch(type,begin,pattern,opattern,seqname,text,&l,
			 plen,mismatch,amino,carboxyl,buf,off,sotable,
			 solimit,regexp,skipm,&hits,m,&tidy);
	if(sc)
	{
	    ajSeqReverseStr(&text);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,text,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&thits,m,&tidy);
	    ajSeqReverseStr(&text);
	}
	
	    

	if(hits || (thits&&sc))
	{
	    desc = ajSeqGetDesc(seq);
	    acc  = ajSeqGetAcc(seq);
	    usa  = ajSeqGetUsa(seq);
	    fuzznuc_print_hits(&l,hits,outf,text,mms,sc,thits,adj,begin,desc,
			       dodesc,acc,doacc,usa,dousa);
	}
	
	

	ajListDel(&l);
    }
    


    if(type==6)
	for(i=0;i<m;++i) AJFREE(skipm[i]);
    
    if(tidy) AJFREE(tidy);
    
    ajStrDel(&pattern);
    ajStrDel(&seqname);
    ajSeqDel(&seq);
    ajFileClose(&outf);
    ajExit();
    return 0;
}



/* @funcstatic fuzznuc_print_hits *********************************************
**
** Undocumented.
**
** @param [?] l [AjPList*] Undocumented
** @param [?] hits [ajint] Undocumented
** @param [?] outf [AjPFile] Undocumented
** @param [?] seq [AjPStr] Undocumented
** @param [?] mms [AjBool] Undocumented
** @param [?] sc [AjBool] Undocumented
** @param [?] thits [ajint] Undocumented
** @param [?] adj [ajint] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] desc [AjPStr] Undocumented
** @param [?] dodesc [AjBool] Undocumented
** @param [?] acc [AjPStr] Undocumented
** @param [?] doacc [AjBool] Undocumented
** @param [?] usa [AjPStr] Undocumented
** @param [?] dousa [AjBool] Undocumented
** @@
******************************************************************************/


static void fuzznuc_print_hits(AjPList *l, ajint hits, AjPFile outf,
			       AjPStr seq, AjBool mms, AjBool sc, ajint thits,
			       ajint adj, ajint begin, AjPStr desc,
			       AjBool dodesc, AjPStr acc, AjBool doacc,
			       AjPStr usa, AjBool dousa)
{
    ajint i;
    EmbPMatMatch m;
    AjPStr s;

    s=ajStrNew();

    ajListReverse(*l);
    
    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	ajStrAssSubC(&s,ajStrStr(seq),m->start-begin,m->start+m->len-1-begin);
	if(doacc)
	    ajFmtPrintF(outf,"%S ",acc);
	if(dodesc)
	    ajFmtPrintF(outf,"%S",desc);
	if(doacc || dodesc)
	    ajFmtPrintF(outf,"\n");
	
	if(dousa)
	    ajFmtPrintF(outf,"%S\t",usa);
	    
	if(!mms)
	    ajFmtPrintF(outf,"%15s %8d %s\n",ajStrStr(m->seqname),m->start,
			ajStrStr(s));
	else
	    ajFmtPrintF(outf,"%15s %8d %5d %s\n",ajStrStr(m->seqname),m->start,
			m->mm,ajStrStr(s));

	embMatMatchDel(&m);
    }

    if(sc)
    {
	ajListReverse(*l);
	ajSeqReverseStr(&seq);
	for(i=0;i<thits;++i)
	{
	    ajListPop(*l,(void **)&m);
	    ajStrAssSubC(&s,ajStrStr(seq),m->start-begin,m->start+m->len-1-
			 begin);
	    if(doacc)
		ajFmtPrintF(outf,"%S ",acc);
	    if(dodesc)
		ajFmtPrintF(outf,"%S",desc);
	    if(doacc || dodesc)
		ajFmtPrintF(outf,"\n");

	    if(dousa)
	        ajFmtPrintF(outf,"%S\t",usa);

	    if(!mms)
		ajFmtPrintF(outf,"%15s %8d [%s]\n",ajStrStr(m->seqname),
			    adj-m->start-m->len,ajStrStr(s));
	    else
		ajFmtPrintF(outf,"%15s %8d %5d [%s]\n",ajStrStr(m->seqname),
			    adj-m->start-m->len,m->mm,ajStrStr(s));

	    embMatMatchDel(&m);
	}
    }
    
    ajStrDel(&s);
    
    return;
}

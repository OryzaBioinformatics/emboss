/* @source fuzztran application
**
** Finds fuzzy protein patterns in nucleic acid sequences
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


void print_hits(AjPList *l, int hits, AjPFile outf, AjPStr seq, AjBool mms,
		int begin, int fnum, int end, AjPStr desc, AjBool dodesc,
		AjPStr acc, AjBool doacc);


int main (int argc, char * argv[])
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPFile outf;
    AjPStr pattern=NULL;
    AjPStr opattern=NULL;
    AjPStr seqname=NULL;
    AjPStr text=NULL;
    
    AjPList l;

    AjPStr *lframe;
    AjPStr *lgcode;
    AjPTrn ttable;
    AjPStr frame;
    int    table;
    AjPStr pro=NULL;
    int    frameno;

    
    int plen;
    int mismatch;
    
    AjBool amino;
    AjBool carboxyl;
    AjBool mms;
    AjPStr desc=NULL;
    AjPStr acc=NULL;
    AjBool doacc;
    AjBool dodesc;
    
    int    type=0;
    int    *buf=NULL;
    int    hits=0;
    int    m;
    int    i;
    int    begin;
    int    end;
    
    EmbOPatBYPNode off[AJALPHA];

    unsigned int *sotable=NULL;
    unsigned int solimit;

    AjPStr	 regexp=NULL;

    int **skipm=NULL;
    

    void   *tidy=NULL;

    embInit ("fuzztran", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequence");
    outf     = ajAcdGetOutfile("outf");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");
    mms      = ajAcdGetBool("mmshow");
    lgcode   = ajAcdGetList("table");
    lframe   = ajAcdGetList("frame");
    doacc    = ajAcdGetBool("accshow");
    dodesc   = ajAcdGetBool("descshow");
    
    seqname = ajStrNew();
    opattern=ajStrNew();

    frame = lframe[0];
    (void) ajStrToInt(lgcode[0],&table);
    ttable = ajTrnNewI(table);
    

    plen = ajStrLen(pattern);
    ajStrAssC(&opattern,ajStrStr(pattern));

    if(!(type=embPatGetType(&pattern,mismatch,1,&m,&amino,&carboxyl)))
	ajFatal("Illegal pattern");
    embPatCompile(type,pattern,opattern,&plen,&buf,off,&sotable,&solimit,
		  &m,&regexp,&skipm,mismatch);


    text = ajStrNew();
    pro  = ajStrNew();
    

    while(ajSeqallNext(seqall,&seq))
    {
	l = ajListNew();
	ajStrAssC(&seqname,ajSeqName(seq));
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	ajStrAssSubC(&text,ajSeqCharCopy(seq),begin-1,end-1);
	ajStrToUpper(&text);

	if(!ajStrCmpC(frame,"F"))
	{
	    ajTrnStrFrame(ttable,text,1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }

	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");
	}
	else if(!ajStrCmpC(frame,"R"))
	{
	    ajTrnStrFrame(ttable,text,-1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");
	}
	else if(!ajStrCmpC(frame,"6"))
	{
	    ajTrnStrFrame(ttable,text,1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");
	    
	    ajTrnStrFrame(ttable,text,2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-1,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-2,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");

	    ajTrnStrFrame(ttable,text,-3,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");
	}
	else
	{
	    (void) ajStrToInt(frame,&frameno);
	    ajTrnStrFrame(ttable,text,frameno,&pro);
	    embPatFuzzSearch(type,begin,pattern,opattern,seqname,pro,&l,
			     plen,mismatch,amino,carboxyl,buf,off,sotable,
			     solimit,regexp,skipm,&hits,m,&tidy);
	    if(hits)
	    {
		desc = ajSeqGetDesc(seq);
		acc  = ajSeqGetAcc(seq);
		print_hits(&l,hits,outf,pro,mms,begin,1,end,desc,dodesc,
			   acc,doacc);
	    }
	    ajStrAssC(&pro,"");
	}
	

	ajListDel(&l);
    }
    


    if(type==6)
	for(i=0;i<m;++i) AJFREE(skipm[i]);
    
    if(tidy) AJFREE(tidy);
    
    ajStrDel(&pro);
    ajStrDel(&text);
    ajStrDel(&pattern);
    ajStrDel(&seqname);
    ajSeqDel(&seq);
    ajFileClose(&outf);
    ajExit();
    return 0;
}




void print_hits(AjPList *l, int hits, AjPFile outf, AjPStr seq, AjBool mms,
		int begin, int fnum, int end, AjPStr desc, AjBool dodesc,
		AjPStr acc, AjBool doacc)
{
    int i;
    EmbPMatMatch m;
    AjPStr s;
    int ff;
    AjBool forward;
    int slen;
    int npos;
    

    forward=ajTrue;
    if(fnum<0)
	forward=ajFalse;
    
    slen = end-begin+1;
    
    s=ajStrNew();
    ff = abs(fnum);
    if(ff>3)
    {
	ff-=3;
	forward=ajFalse;
    }
    ff%=3;
    if(ff)
	ff-=3;
    
    ajListReverse(*l);
    
    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);

	ajStrAssSubC(&s,ajStrStr(seq),m->start-1,m->start+m->len-2);

	if(doacc)
	    ajFmtPrintF(outf,"%S ",acc);
	if(dodesc)
	    ajFmtPrintF(outf,"%S",desc);
	if(doacc || dodesc)
	    ajFmtPrintF(outf,"\n");


	if(forward)
	    npos = (m->start*3) + ff + (begin-1);
	else
	    npos = (slen/3)*3  - (m->start*3) - (fnum-1) + (begin);
	

	if(!mms)
	    ajFmtPrintF(outf,"%15s(%2d) %8d %s\n",ajStrStr(m->seqname),fnum,
			npos,
			ajStrStr(s));
	else
	    ajFmtPrintF(outf,"%15s(%2d) %8d %5d %s\n",ajStrStr(m->seqname),
			fnum,npos,
			m->mm,ajStrStr(s));

	embMatMatchDel(&m);
    }


    ajStrDel(&s);
    
    return;
}

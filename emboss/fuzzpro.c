/* @source fuzzpro application
**
** Finds fuzzy patterns in proteins
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


void print_hits(AjPList *l, ajint hits, AjPFile outf, AjPStr seq, AjBool mms,
		ajint begin, AjPStr desc, AjBool dodesc, AjPStr acc,
		AjBool doacc, AjPStr usa, AjBool dousa);



int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq;
    AjPFile outf;
    AjPStr pattern=NULL;
    AjPStr opattern=NULL;
    AjPStr seqname=NULL;
    AjPStr text=NULL;
    
    AjPList l;
    
    ajint plen;
    ajint mismatch;

    AjBool amino;
    AjBool carboxyl;
    AjBool mms;
    ajint    type=0;
    ajint    *buf=NULL;
    ajint    hits=0;
    ajint    m;
    ajint    i;
    ajint    end;
    ajint    begin;
    AjPStr desc=NULL;
    AjPStr acc=NULL;
    AjBool dodesc;
    AjBool doacc;
    AjPStr usa=NULL;
    AjBool dousa;
        

    EmbOPatBYPNode off[AJALPHA];

    ajuint *sotable=NULL;
    ajuint solimit;

    AjPStr	regexp=NULL;

    ajint **skipm=NULL;
    

    void   *tidy=NULL;

    embInit ("fuzzpro", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequence");
    outf     = ajAcdGetOutfile("outf");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");
    mms      = ajAcdGetBool("mmshow");
    dodesc   = ajAcdGetBool("descshow");
    doacc    = ajAcdGetBool("accshow");
    dousa    = ajAcdGetBool("usashow");
    
    seqname = ajStrNew();
    opattern=ajStrNew();

    plen = ajStrLen(pattern);

    ajStrAssC(&opattern,ajStrStr(pattern));
    if(!(type=embPatGetType(&pattern,mismatch,1,&m,&amino,&carboxyl)))
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
	ajStrAssSubC(&text,ajSeqCharCopy(seq),begin-1,end-1);
	ajStrToUpper(&text);
	
	embPatFuzzSearch(type,begin,pattern,opattern,seqname,text,&l,
			 plen,mismatch,amino,carboxyl,buf,off,sotable,
			 solimit,regexp,skipm,&hits,m,&tidy);
	
	if(hits)
	{
	    desc = ajSeqGetDesc(seq);
	    acc  = ajSeqGetAcc(seq);
            usa  = ajSeqGetUsa(seq);
	    print_hits(&l,hits,outf,text,mms,begin,desc,dodesc,acc,doacc,
	    	usa,dousa);
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




void print_hits(AjPList *l, ajint hits, AjPFile outf, AjPStr seq, AjBool mms,
		ajint begin, AjPStr desc, AjBool dodesc, AjPStr acc,
		AjBool doacc, AjPStr usa, AjBool dousa)
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
	    ajFmtPrintF(outf,"%15s %5d %s\n",ajStrStr(m->seqname),m->start,
			ajStrStr(s));
	else
	    ajFmtPrintF(outf,"%15s %5d %5d %s\n",ajStrStr(m->seqname),m->start,
			m->mm,ajStrStr(s));
	embMatMatchDel(&m);
    }

    ajStrDel(&s);

    return;
}

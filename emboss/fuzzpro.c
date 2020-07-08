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


void print_hits(AjPList *l, int hits, AjPFile outf, AjPStr seq, AjBool mms,
		int begin, AjPStr desc, AjBool dodesc, AjPStr acc,
		AjBool doacc);



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
    
    int plen;
    int mismatch;

    AjBool amino;
    AjBool carboxyl;
    AjBool mms;
    int    type=0;
    int    *buf=NULL;
    int    hits=0;
    int    m;
    int    i;
    int    end;
    int    begin;
    AjPStr desc=NULL;
    AjPStr acc=NULL;
    AjBool dodesc;
    AjBool doacc;
    
    EmbOPatBYPNode off[AJALPHA];

    unsigned int *sotable=NULL;
    unsigned int solimit;

    AjPStr	regexp=NULL;

    int **skipm=NULL;
    

    void   *tidy=NULL;

    embInit ("fuzzpro", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequence");
    outf     = ajAcdGetOutfile("outf");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");
    mms      = ajAcdGetBool("mmshow");
    dodesc   = ajAcdGetBool("descshow");
    doacc    = ajAcdGetBool("accshow");
    
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
	    print_hits(&l,hits,outf,text,mms,begin,desc,dodesc,acc,doacc);
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




void print_hits(AjPList *l, int hits, AjPFile outf, AjPStr seq, AjBool mms,
		int begin, AjPStr desc, AjBool dodesc, AjPStr acc,
		AjBool doacc)
{
    int i;
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

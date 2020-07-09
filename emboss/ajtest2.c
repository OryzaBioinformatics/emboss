#include "emboss.h"
#include "stdlib.h"


void print_hits(AjPList *l, ajint hits, AjPFile outf);



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
    ajint slen=0;
    ajint mismatch;

    AjBool amino;
    AjBool carboxyl;
    AjBool fclass;
    AjBool compl;
    AjBool dontcare;
    AjBool range;
    ajint    type=0;
    ajint    *buf=NULL;
    ajint    hits=0;
    ajint    m;
    ajint    n;
    ajint    i;
    ajint    start;
    ajint    end;
    
    EmbOPatBYPNode off[AJALPHA];

    ajuint *sotable;
    ajuint solimit;

    EmbPPatMatch ppm;
    AjPStr	   regexp=NULL;

    ajint **skipm=NULL;
    

    void   *tidy=NULL;

    embInit ("sea", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequence");
    outf     = ajAcdGetOutfile("outf");
    pattern  = ajAcdGetString("pattern");
    mismatch = ajAcdGetInt("mismatch");
    
    seq = ajSeqNew();
    seqname = ajStrNew();
    opattern=ajStrNew();


    /* Copy original pattern for dear Henry */
    ajStrAssC(&opattern,ajStrStr(pattern));

    
    if(!embPatClassify(&pattern,&amino,&carboxyl,&fclass,&compl,&dontcare,
		      &range,1))
	ajFatal("Illegal pattern");

    plen = ajStrLen(pattern);

    /*
     *  Select type of search depending on pattern
     */

    if(!range && !dontcare && !fclass && !compl && !mismatch && plen>4)
    {
	/* Boyer Moore Horspool is the choice for ajlong exact patterns */
	type = 1;
	plen = ajStrLen(pattern);
	AJCNEW(buf, AJALPHA);
	embPatBMHInit(&pattern,plen,buf);
    }
    
    if(mismatch && !range && !dontcare && !fclass && !compl)
    {
	/* Baeza-Yates Perleberg for exact patterns plus don't cares */
	type = 2;
	plen = ajStrLen(pattern);
	AJCNEW(buf, AJALPHA);
	embPatBYPInit(&pattern,plen,off,buf);
    }


    if(!range && !dontcare && !fclass && !compl && !mismatch)
    {
	/* Shift-OR is the choice for small exact patterns */
	type = 3;
	AJCNEW(sotable, AJALPHA2);
	embPatSOInit(&pattern,sotable,&solimit);
    }


    AJCNEW(sotable, AJALPHA2);
    embPatBYGCInit(&pattern,&m,sotable,&solimit);

    if(!range && (fclass || compl) && !mismatch && m<=AJWORD)
	/*
	 *  Baeza-Yates Gonnet for classes and dontcares.
	 *  No mismatches or ranges. Patterns less than (e.g.) 32
         */
	type = 4;
	

    if(!mismatch && (range || m>AJWORD))
    {
	/*
	 *  Henry Spencer for ranges and simple classes longer than
         *  e.g. 32. No mismatches allowed
         */
	regexp=embPatPrositeToRegExp(&opattern);
	type = 5;
    }
    
    if(mismatch && !range)
    {
	/* Try a Tarhio-Ukkonen-Bleasby */
	AJCNEW(skipm, m);
	for(i=0;i<m;++i)
	  AJCNEW(skipm[i], AJALPHA);
	embPatTUBInit(&pattern,skipm,m,mismatch,plen);
	type = 6;
    }
    

    while(ajSeqallNext(seqall,&seq))
    {
	l = ajListNew();
	ajStrAssC(&seqname,ajSeqName(seq));
	text=ajSeqStr(seq);
	ajStrMod(&text);
	
	switch (type)
	{
	case 1:
	    hits=embPatBMHSearch(&text,&pattern,ajStrLen(text),
				ajStrLen(pattern),buf,0,amino,carboxyl,&l,
				&seqname,ajSeqallBegin(seqall));
	    tidy = (void *) buf;
	    break;

	case 2:
	    hits=embPatBYPSearch(&text,&seqname,ajSeqallBegin(seqall),
				ajStrLen(text),plen,mismatch,off,buf,l,
				amino,carboxyl, pattern);
	    tidy = (void *) buf;
	    break;

	case 3:
	    hits=embPatSOSearch(&text,&seqname,*ajStrStr(pattern),
			       ajSeqallBegin(seqall),plen,sotable,solimit,l,
			       amino,carboxyl);
	    tidy = (void *) sotable;
	    break;

	case 4:
	    plen=m;
	    hits=embPatSOSearch(&text,&seqname,*ajStrStr(pattern),
			       ajSeqallBegin(seqall),plen,sotable,solimit,l,
			       amino,carboxyl);
	    tidy = (void *) sotable;
	    break;

	case 5:
	    ppm=embPatPosMatchFind(regexp,text);
	    n=embPatPosMatchGetNumber(ppm);
	    for(i=0;i<n;++i)
	    {
		start=embPatPosMatchGetStart(ppm,i);
		end=embPatPosMatchGetEnd(ppm,i);
		if(amino && start)
		{
		    n=0;
		    break;
		}
		if(!carboxyl || (carboxyl && start==slen-(end-start+1)))
		    embPatPushHit(l,&seqname,start,end-start+1,
				 ajSeqallBegin(seqall),0);

	    }
	    embPatMatchDel(&ppm);
	    hits=n;
	    break;

	case 6:
	    hits = embPatTUBSearch(&pattern,&text,ajStrLen(text),skipm,
				  m,mismatch,ajSeqallBegin(seqall),
				  l,amino,carboxyl,&seqname,plen);
	    tidy = (void *) skipm;
	    break;
	    
	default:
	    ajErr("No routine in place for this search type yet");
	    break;
	}


	if(hits)
	    print_hits(&l,hits,outf);
	

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




void print_hits(AjPList *l, ajint hits, AjPFile outf)
{
    ajint i;
    EmbPMatMatch m;

    for(i=0;i<hits;++i)
    {
	ajListPop(*l,(void **)&m);
	ajFmtPrintF(outf,"%s\t%d\n",ajStrStr(m->seqname),m->start);
	embMatMatchDel(&m);
    }

    return;
}



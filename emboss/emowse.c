/* @source emowse application
**
** Finds proteins matching mass spectrometry data
**
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
#include <math.h>
#include <string.h>

#define FGUESS 128000		/* Uncritical guess of freq data size */
#define MILLION (double)1000000.
#define MAXLIST 50

typedef struct EmbSMdata
{
    double mwt;
    AjPStr sdata;
} EmbOMdata, *EmbPMdata;


typedef struct SHits
{
    AjPStr seq;
    AjPStr name;
    AjPStr desc;
    AjPInt found;
    double score;
    double mwt;
    EmbPMolFrag* frags;
    int    nf;
} OHits, *PHits;




void read_freqs(AjPStr ffile, AjPDouble *freqs);
AjBool molwt_outofrange(double thys, double given, double range);
int read_data(AjPFile inf, EmbPMdata** data);
int sort_data(const void *a, const void *b);
int hit_sort(const void *a, const void *b);
void match(EmbPMdata* data, int dno, AjPList flist, int nfrags,
	   double tol, AjPSeq seq, AjPList* hlist, double partials,
	   double cmw, int enz, AjPDouble freqs);

int seq_comp(int bidx, int thys, AjPSeq seq, EmbPMdata *data,
	     EmbPMolFrag *frags);
int get_index(double actmw, double maxmw, double minmw, EmbPMolFrag *frags,
	      int fno, double *bestmw, int *index, int thys, AjPSeq seq,
	      EmbPMdata *data);

int seq_search(AjPStr substr, char *s);
AjBool msearch(char *seq, char *pat, AjBool term);
void mreverse(char *s);
int get_orc(AjPStr *orc, char *s, int pos);
AjBool comp_search(AjPStr substr, char *s);
void print_hits(AjPFile outf, AjPList hlist, int dno, EmbPMdata* data);



int main(int argc, char **argv)
{
    AjPSeq       seq;
    AjPSeqall    seqall;
    AjPFile      outf;
    AjPFile      mwinf;
    AjPStr       ffile;
    AjPStr       *enzyme;
    int          smolwt;
    int 	 range;
    float        tol;
    float        partials;
    AjPDouble    freqs;
    int 	 begin;
    int 	 end;
    double 	 smw;
    int 	 rno;

    AjPList      flist;
    EmbPMdata    *data;
    int 	 dno;
    int          nfrags;    
    AjPList      hlist=NULL;

    

    (void) embInit("emowse", argc, argv);

    seqall   = ajAcdGetSeqall("sequences");
    mwinf    = ajAcdGetInfile("infile");
    enzyme   = ajAcdGetList("enzyme");
    smolwt   = ajAcdGetInt("weight");
    range    = ajAcdGetInt("pcrange");
    ffile    = ajAcdGetString("frequencies");
    tol      = ajAcdGetFloat("tolerance");
    partials = ajAcdGetFloat("partials");
    outf     = ajAcdGetOutfile("outfile");


    freqs = ajDoubleNewL(FGUESS);
    read_freqs(ffile, &freqs);
    if(sscanf(ajStrStr(*enzyme),"%d",&rno)!=1)
	ajFatal("Illegal enzyme entry [%S]",*enzyme);
    

    if(!(dno = read_data(mwinf,&data)))
	ajFatal("No molecular weights in the file");
    ajFileClose(&mwinf);
    

    hlist = ajListNew();
    
    

    while(ajSeqallNext(seqall,&seq))
    {
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);

	
	
	smw = embPropCalcMolwt(ajSeqChar(seq),--begin,--end);
	if(smolwt)
	    if(molwt_outofrange(smw,(double)smolwt,(double)range))
		continue;

	flist = ajListNew();
	nfrags = embMolGetFrags(ajSeqStr(seq),rno,&flist);

	match(data,dno,flist,nfrags,(double)tol,seq,&hlist,(double)partials,
	      smw,rno,freqs);

	ajListDel(&flist);
    }


    print_hits(outf,hlist,dno,data);

    ajListDel(&hlist);
    
    
    ajExit();
    return 0;
}




void read_freqs(AjPStr ffile, AjPDouble *freqs)
{
    AjPFile finf=NULL;
    int c;
    AjPStr  str;
    double f;
    
    ajFileDataNew(ffile,&finf);
    if(!finf)
	ajFatal("Cannot open frequencies file %S",ffile);

    c = 0;
    str = ajStrNew();

    while(ajFileReadLine(finf,&str))
    {
       	if(sscanf(ajStrStr(str),"%lf",&f)==1)
	    ajDoublePut(freqs,c,f);
	else
	    ajDoublePut(freqs,c,0.);
	++c;
    }

    ajStrDel(&str);
    ajFileClose(&finf);

    return;
}



AjBool molwt_outofrange(double thys, double given, double range)
{
    double diff;

    diff = given * range / (double)100.;
    if(thys>=given-diff && thys<=given+diff)
	return ajFalse;

    return ajTrue;
}



int read_data(AjPFile inf, EmbPMdata** data)
{
    int c;
    AjPStr str=NULL;
    double v;
    AjPStrTok token=NULL;
    EmbPMdata ptr=NULL;
    AjPList l;
    int n;

    c=0;
    str = ajStrNew();
    l = ajListNew();
    
    while(ajFileReadLine(inf,&str))
    {
	if(sscanf(ajStrStr(str),"%lf",&v)==1)
	{
	    AJNEW0(ptr);
	    ptr->mwt = v;
	    ptr->sdata=ajStrNew();
	    ajStrClean(&str);
	    token = ajStrTokenInit(str," \t\r\n");
	    ajStrToken(&ptr->sdata,&token," \t\r\n");
	    ajStrToken(&ptr->sdata,&token," \t\r\n");
	    ajStrTokenClear(&token);
	    ajListPush(l,(void *)ptr);
	}
    }

    ajListSort(l,sort_data);
    n = ajListToArray(l,(void ***)data);
    ajListDel(&l);
    ajStrDel(&str);

    return n;
}



int sort_data(const void *a, const void *b)
{
    return (int)((*(EmbPMdata*)a)->mwt - (*(EmbPMdata*)b)->mwt);
}



int hit_sort(const void *a, const void *b)
{
    double x;
    double y;

    x = (*(PHits*)a)->score;
    y = (*(PHits*)b)->score;

    if(x==y)
	return 0;
    else if(x<y)
	return -1;
    return 1;
}



void match(EmbPMdata* data, int dno, AjPList flist, int nfrags,
	   double tol, AjPSeq seq, AjPList* hlist, double partials,
	   double cmw, int rno, AjPDouble freqs)
{
    double actmw;
    double minmw;
    double maxmw;
    double smw;
    int ft;
    double qtol;
    double f;
    double sumf;
    double bestmw;
    static double min=(double)0.;
    static int    n = 0;
    
    int    i;
    int    j;
    int    nd;
    
    int    x;
    int    index;
    AjBool ispart=ajFalse;
    int    isumf;
    AjPInt found=NULL;
    PHits  hits=NULL;
    EmbPMolFrag *frags=NULL;
    

    ajListReverse(flist);
    ajListToArray(flist,(void ***)&frags);


    sumf = (double)1.;

    smw = cmw;
    
    
    cmw /= (double)10000.;
    if(cmw>(double)79.)
	cmw = (double)79.;
    

    found = ajIntNew();
    
    
    for(i=0;i<dno;++i)
    {
	actmw = data[i]->mwt;
	qtol = actmw / (double)100.;
	minmw = actmw - (tol*qtol);
	if(minmw<(double)0.)
	    minmw = (double)0.;
	maxmw = actmw + (tol*qtol);

	x = get_index(actmw,maxmw,minmw,frags,nfrags,&bestmw,&index,i,seq,
		      data);

	if(bestmw > MILLION)
	{
	    bestmw -= MILLION;
	    ispart = ajTrue;
	}
	else
	    ispart = ajFalse;

	if(x == -2)
	{
	    for(j=0;j<dno;++j)
		ajIntPut(&found,j,-1);
	    break;
	}


	if(x<0)
	{
	    ajIntPut(&found,i,-1);
	    continue;
	}
	ajIntPut(&found,i,x);

	if(ispart)
	    bestmw *= partials;
	if(ispart && bestmw < (double)101.)
	    f = (double).99;
	else
	{
	    f = bestmw / (double)100.;
	    if((int)f >= 200)
		f = (double) 199.;

	    ft = (rno-1)*16000 + (int)cmw*200 + (int)f;
	    f = ajDoubleGet(freqs,ft);
	    if(!f)
		continue;
	}

	sumf *= f;
	if(sumf < (double)1.0e-31)
	    sumf = (double)1.0e-31;
    }


    isumf = (int)sumf;
    
    sumf = (double)1. / sumf;
    sumf *= (double)50000. / (cmw*(double)10000.);


    if(n<MAXLIST && isumf!=1)
    {
	AJNEW0(hits);
	hits->seq = ajSeqStrCopy(seq);
	hits->desc = ajStrNewC(ajStrStr(ajSeqGetDesc(seq)));
	hits->found = found;
	hits->score = sumf;
	hits->mwt = smw;
	hits->name = ajStrNewC(ajSeqName(seq));
	hits->frags = frags;
	hits->nf = nfrags;
	ajListPush(*hlist,(void *)hits);
	ajListSort(*hlist,hit_sort);
	ajListPop(*hlist,(void **)&hits);
	min = hits->score;
	ajListPush(*hlist,(void *)hits);
	++n;
    }
    else if(sumf>min && isumf!=1)
    {
	ajListPop(*hlist,(void **)&hits);
	ajStrDel(&hits->seq);
	ajStrDel(&hits->name);
	ajStrDel(&hits->desc);
	ajIntDel(&hits->found);
        nd = hits->nf;
	for(i=0;i<nd;++i)
	    AJFREE(hits->frags[i]);

	AJFREE(hits->frags);
	    
	AJFREE(hits);
	
	AJNEW0(hits);
	hits->seq = ajSeqStrCopy(seq);
	hits->desc = ajStrNewC(ajStrStr(ajSeqGetDesc(seq)));
	hits->found = found;
	hits->name = ajStrNewC(ajSeqName(seq));
	hits->score = sumf;
	hits->mwt = smw;
	hits->frags = frags;
        hits->nf = nfrags;
    
	ajListPush(*hlist,(void *)hits);
	ajListSort(*hlist,hit_sort);
	ajListPop(*hlist,(void **)&hits);
	min = hits->score;
	ajListPush(*hlist,(void *)hits);
    }
    else
    {
	ajIntDel(&found);
	for(i=0;i<nfrags;++i)
	    AJFREE(frags[i]);
	AJFREE(frags);
    }

    return;
}



int get_index(double actmw, double maxmw, double minmw, EmbPMolFrag *frags,
	      int fno, double *bestmw, int *index, int thys, AjPSeq seq,
	      EmbPMdata *data)
{
    double mw1;
    double mw2;
    double best;
    int cnt;
    int fl;
    int bidx=0;


    cnt = fl = 0;
    best = (double)-1.0;

    /*
     *  This assumes the lowest array entry has been sorted to be the highest
     *  mw
     */

    while(cnt<fno)
    {
	mw1 = frags[cnt]->mwt;
	
	if(mw1>MILLION || mw1>maxmw)
	{
	    ++cnt;
	    continue;
	}
	if(mw1<minmw)
	    break;
	

	best = mw1;
	bidx = cnt;


	while(cnt+1<fno)
	{
	    mw2 = frags[cnt+1]->mwt;
	    if(mw2>MILLION)
	    {
		++cnt;
		continue;
	    }
	    if(abs(mw2-actmw)>abs(mw1-actmw))
		break;
	    else
	    {
		mw1 = best = mw2;
		bidx = cnt;
		++cnt;
	    }
	}

	break;
    }

    fl = (cnt==fno) ? 1 : 0;

    if(best != (double)-1.)
    {
	if(!seq_comp(bidx,thys,seq,data,frags))
	    return -1;
	*bestmw = best;
	*index  = bidx;
	return bidx;
    }

    cnt = 0;
    while(cnt<fno)
    {
	mw1 = frags[cnt]->mwt;
	if(mw1<MILLION)
	{
	    ++cnt;
	    continue;
	}
	mw1 -= MILLION;
	if(mw1<minmw)
	    break;
	if(mw1>maxmw)
	{
	    ++cnt;
	    continue;
	}

	best = mw1;
	bidx = cnt;
	while(cnt+1>=fno)
	{
	    mw2 = frags[cnt+1]->mwt;
	    if(mw2<MILLION)
	    {
		++cnt;
		continue;
	    }
	    mw2 -= MILLION;
	    if(fabs(mw2-actmw)>fabs(mw1-actmw))
		break;
	    else
	    {
		mw1 = best = mw2;
		bidx = cnt++;
	    }
	}

	break;
    }
    
    if(cnt==fno && fl)
	return -2;
    if(best == (double)-1.)
	return -1;
    if(!seq_comp(bidx,thys,seq,data,frags))
	return -1;

    *bestmw = best + MILLION;
    *index = bidx + (int)MILLION;

    return *index;
}




int seq_comp(int bidx, int thys, AjPSeq seq, EmbPMdata *data,
	     EmbPMolFrag *frags)
{
    int beg;
    int end;
    int len;
    AjPStr result=NULL;
    AjPStr str=NULL;
    AjPStr substr=NULL;
    AjPStrTok token=NULL;
    char *p;
    

    if(!ajStrLen(data[thys]->sdata))
	return 1;
    
    beg = frags[bidx]->begin - 1;
    end = frags[bidx]->end   - 1;
    
    str = ajSeqStr(seq);

    result = ajStrNew();
    substr = ajStrNew();
    ajStrAssSub(&substr,str,beg,end);
    ajStrToUpper(&substr);
    
    token = ajStrTokenInit(substr," \r\t\n");
    while(ajStrToken(&result,&token," \r\t\n"))
    {
	len = ajStrLen(result);
	ajStrToUpper(&result);
	p = ajStrStr(result);
	if(p[len-1]!=')')
	    ajFatal("Missing ')' in subline %S",substr);

	if(ajStrPrefixC(result,"SEQ("))
	{
	    ajStrAssC(&result,p+4);
	    *(ajStrStr(result)+5)='\0';
	    if(!seq_search(substr,ajStrStr(result)))
		return 0;
	}
	else if(ajStrPrefixC(result,"COMP("))
	{
	    ajStrAssC(&result,p+5);
	    *(ajStrStr(result)+5)='\0';
	    if(!comp_search(substr,ajStrStr(result)))
		return 0;
	}
	else
	    ajFatal("Unknown Query type [%S]",result);
	    
    }
    ajStrTokenClear(&token);
    

    ajStrDel(&substr);
    ajStrDel(&result);

    return 1;
}


void mreverse(char *s)
{
    int i;
    int len;
    char *p;
    AjPStr rev;
    
    rev = ajStrNewC(s);
    ajStrRev(&rev);
    p = ajStrStr(rev);
    
    len = strlen(s);
    for(i=0;i<len;++i)
    {
	if(p[i]==']')
	{
	    p[i]='[';
	    continue;
	}
	if(p[i]=='[')
	    p[i]=']';
    }

    strcpy(s,ajStrStr(rev));

    ajStrDel(&rev);
    return;
}



AjBool seq_search(AjPStr substr, char *s)
{
    char *p;
    char *q;
    static AjPStr t=NULL;
    
    if(!t)
	t=ajStrNew();
    
    p = ajStrStr(substr);
    ajStrAssC(&t,p);
    q = ajStrStr(t);
    
    if(!strncmp(s,"B-",2))
    {
	if(!msearch(q,s+2,ajFalse))
	    return ajFalse;
    }
    else if(!strncmp(s,"N-",2))
    {
	if(!msearch(q,s+2,ajTrue))
	    return ajFalse;
    }
    else if(!strncmp(s,"C-",2))
    {
	mreverse(s+2);
	mreverse(q);
	if(!msearch(q,s+2,ajTrue))
	    return ajFalse;
    }
    else if(!strncmp(s,"*-",2))
    {
	if(!msearch(q,s+2,ajFalse))
	    return ajTrue;
	mreverse(s+2);
	if(!msearch(q,s+2,ajFalse))
	    return ajFalse;
    }
    
    return ajTrue;
}


AjBool msearch(char *seq, char *pat, AjBool term)
{
    AjPStr orc=NULL;
    
    int qpos;
    int fpos;
    
    AjBool fl;
    int i;
    int t;
    int ofpos;
    char *p;
    
    qpos = ofpos = fpos = 0;
    fl = ajFalse;
    

    orc = ajStrNew();

    while(pat[qpos])
    {
	if((term && fl) || !seq[fpos])
	{
	    ajStrDel(&orc);
	    return ajFalse;
	}
	

	if(pat[qpos]=='X')
	{
	    ++qpos;
	    ++fpos;
	    continue;
	}
	if(pat[qpos]=='[')
	{
	    ++qpos;
	    while(pat[qpos]!=']')
	    {
		if(!pat[qpos])
		    ajFatal("Missing ']' in term %s",pat);
		ajStrAppK(&orc,pat[qpos++]);
	    }
	    
	    t = ajStrLen(orc);
	    p = ajStrStr(orc);
	    for(i=0;i<t;++i)
		if(p[i]==seq[fpos])
		    break;
	    if(t==i)
	    {
		fpos = ++ofpos;
		qpos=0;
		fl=ajTrue;
		continue;
	    }
	    else
	    {
		++fpos;
		++qpos;
		continue;
	    }
	}

	if(pat[qpos]==seq[fpos])
	{
	    ++fpos;
	    ++qpos;
	    continue;
	}
	else
	{
	    fpos = ++ofpos;
	    qpos=0;
	    fl=ajTrue;
	}
    }


    ajStrDel(&orc);

    return ajTrue;
}

	       
AjBool comp_search(AjPStr substr, char *s)
{
    AjPInt arr;
    int i;
    int len;
    int v;
    int n;
    int w;
    
    int qpos;
    char c;
    AjPStr orc;
    char *r;
    
    
    char *p;
    AjPStr t;
    
    p = ajStrStr(substr);
    len = ajStrLen(substr);

    arr = ajIntNewL(256);

    for(i=0;i<256;++i)
	ajIntPut(&arr,i,0);
    
    for(i=0;i<len;++i)
    {
	v = ajIntGet(arr,(int)p[i]);
	ajIntPut(&arr,(int)p[i],v+1);
    }


    t = ajStrNewC(s);
    ajStrCleanWhite(&t);
    
    p = ajStrStr(t);
    qpos = 0;
    orc = ajStrNew();
    
    while((c=p[qpos]))
    {
	if(c=='*')
	{
	    n = get_orc(&orc,p,qpos);
	    r = ajStrStr(orc);
	    qpos += (n+3);
	    for(i=0;i<n;++i)
		if(ajIntGet(arr,r[i]))
		    break;
	    if(i==n)
	    {
		ajStrDel(&t);
		ajStrDel(&orc);
		ajIntDel(&arr);
		return ajFalse;
	    }
	    
	    continue;
	}
	i = qpos;
	while((c=p[i])>='0' && c<='9')
	    ++i;
	if(i==qpos || p[i]!='[')
	    ajFatal("Bad integer [%s]",p);
	if(sscanf(p+qpos,"%d",&v)!=1)
	    ajFatal("Bad integer [%s]",p);
	qpos = --i;
	ajStrClear(&orc);
	n = get_orc(&orc,p,qpos);
	r = ajStrStr(orc);
	qpos += (n+3);
	w = 0;
	for(i=0;i<n;++i)
	    w += ajIntGet(arr,(int)r[i]);
	if(w!=v)
	{
	    ajStrDel(&t);
	    ajStrDel(&orc);
	    ajIntDel(&arr);
	    return ajFalse;
	}
    }
    

    ajStrDel(&orc);
    ajStrDel(&t);
    ajIntDel(&arr);
    
    return ajTrue;
}


int get_orc(AjPStr *orc, char *s, int pos)
{
    int i;

    i=0;
    if(s[++pos]!='[')
	ajFatal("Bad query given [%s]",s);
    ++pos;
    while(s[pos]!=']')
    {
	if(!s[pos])
	    ajFatal("Unterminated square brackets [%s]",s);
	ajStrAppK(orc,s[pos++]);
	++i;
    }

    return i;
}




void print_hits(AjPFile outf, AjPList hlist, int dno, EmbPMdata* data)
{
    PHits hits=NULL;
    AjIList iter=NULL;
    int   n;
    int   c;
    int   i;
    int   j;
    int   pvt;
    double conf;
    AjBool partial;
    AjPFloat nmarray=NULL;
    int    nmn;
    int    len;
    int    v;
    AjPStr substr=NULL;
    

    nmarray = ajFloatNew();
    substr  = ajStrNew();

    n = ajListLength(hlist);
    ajListReverse(hlist);


    ajFmtPrintF(outf,"\nUsing data fragments of:\n");
    
    for(i=0;i<dno;++i)
	ajFmtPrintF(outf,"         %7.1f  %S\n",data[i]->mwt,data[i]->sdata);



    ajFmtPrintF(outf,"\n");
    
    iter = ajListIter(hlist);
    c=0;
    while(ajListIterMore(iter))
    {
	hits = (PHits) ajListIterNext(iter);
	ajFmtPrintF(outf,"%-3d %-13S%-62.62S\n",++c,hits->name,hits->desc);
    }
    ajListIterFree(iter);


    iter = ajListIter(hlist);
    c=0;
    while(ajListIterMore(iter))
    {
	hits = (PHits) ajListIterNext(iter);

	for(i=pvt=0;i<dno;++i)
	    if(ajIntGet(hits->found,i) > -1)
		++pvt;
	conf = (double)pvt / (double)dno;
	

	ajFmtPrintF(outf,"\n    %-3d: %-12S   %.3e %-8.1f   %-6.3f\n",++c,
		    hits->name,hits->score,hits->mwt,conf);
	ajFmtPrintF(outf,"         %S\n",hits->desc);
	ajFmtPrintF(outf,"         Mw     Start  End    Seq\n");
	
	for(i=nmn=0;i<dno;++i)
	{
	    partial = (ajIntGet(hits->found,i)>1000000) ? ajTrue : ajFalse;

	    if((v=ajIntGet(hits->found,i))>-1)
	    {
		if(v>=(int)MILLION)
		    v -= (int)MILLION;
		
		ajStrAssSubC(&substr,ajStrStr(hits->seq),
			     hits->frags[v]->begin-1,
			     hits->frags[v]->end-1);

		len = ajStrLen(substr);
		ajFmtPrintF(outf,"        ");
		if(partial)
		    ajFmtPrintF(outf,"*");
		else
		    ajFmtPrintF(outf," ");
		if(hits->frags[v]->mwt > MILLION)
		    hits->frags[v]->mwt -= MILLION;
		ajFmtPrintF(outf,"%-6.1f %-6d %-6d %-45.45S",
			    hits->frags[v]->mwt,hits->frags[v]->begin,
			    hits->frags[v]->end,substr);
		if(len>45)
		    ajFmtPrintF(outf,"...");
		ajFmtPrintF(outf,"\n");
	    }
	    else
		ajFloatPut(&nmarray,nmn++,data[i]->mwt);
	}
	if(nmn)
	{
	    ajFmtPrintF(outf,"         No Match      ");
	    for(i=0,j=0;i<nmn;++i,++j)
	    {
		ajFmtPrintF(outf,"%-6.1f ",ajFloatGet(nmarray,i));
		if(j==6 && i!=nmn-1)
		{
		    ajFmtPrintF(outf,"\n                       ");
		    j=0;
		}
	    }
	    ajFmtPrintF(outf,"\n");
	}
	ajStrDel(&hits->name);
	ajStrDel(&hits->seq);
	ajStrDel(&hits->desc);
	ajIntDel(&hits->found);
	len = hits->nf;
	for(i=0;i<len;++i)
	    AJFREE(hits->frags[i]);
	AJFREE(hits->frags);
    }

    ajListIterFree(iter);
    ajStrDel(&substr);
    ajFloatDel(&nmarray);
    

    return;
}

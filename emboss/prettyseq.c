/* @source prettyseq application
**
** Pretty translation of DNA sequences for publication
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
#include <string.h>
#include <ctype.h>


#define POFF 1000000	/* Printing flag */

static void makeRuler(int len, int begin, char *ruler, int *npos);
static void calcProteinPos(int *ppos, AjPStr pro, int len);
static void showTrans(int *ppos, int *npos, AjPStr pro, AjPStr substr,
		 int len, char *ruler, int begin,
		 AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		 int width, char *name);
static void showTransb(int *ppos, int *npos, AjPStr pro, AjPStr substr,
		  int len, char *ruler, int begin,
		  AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		  int start, int end);
static void prettyTranslate(AjPFile outf,int beg,int end,AjPStr s,AjPCod codon,
		       AjPRange range, int width, AjPStr pro);

int main(int argc, char **argv)
{
    AjPSeq       a;
    AjPFile      outf;
    AjPCod       codon;
    AjPStr       substr;
    AjPRange     range;
    AjBool       isrule;
    AjBool       isp;
    AjBool       isn;
    
    AjPStr       pro;
    
    int beg;
    int end;
    int len;
    
    int width;

    char *ruler;
    int  *ppos=NULL;
    int  *npos=NULL;
    

    embInit("prettyseq", argc, argv);

    a         = ajAcdGetSeq("sequence");
    codon     = ajAcdGetCodon("cfile");
    width     = ajAcdGetInt("width");
    range     = ajAcdGetRange("range");
    outf      = ajAcdGetOutfile("outfile");
    isrule    = ajAcdGetBool("ruler");
    isp       = ajAcdGetBool("plabel");
    isn       = ajAcdGetBool("nlabel");

    beg = ajSeqBegin(a);
    end = ajSeqEnd(a);

    substr = ajStrNew();
    ajStrAssSubC(&substr,ajSeqChar(a),beg-1,end-1);
    pro=ajStrNewC(ajStrStr(substr));
    len = ajStrLen(substr);

    AJCNEW (ruler, len);
    AJCNEW (npos, len);
    AJCNEW (ppos, len);

    prettyTranslate(outf,beg,end,substr,codon,range,width,pro);
    makeRuler(len,beg,ruler,npos);
    calcProteinPos(ppos,pro,len);
    showTrans(ppos,npos,pro,substr,len,ruler,beg,
		outf,isrule,isp,isn,width,ajSeqName(a));

    AJFREE (npos);
    AJFREE (ppos);
    AJFREE (ruler);

    ajStrDel(&substr);
    ajCodDel(&codon);
    ajRangeDel(&range);
    
    ajExit();
    return 0;
}



static void prettyTranslate(AjPFile outf,int beg,int end,AjPStr s,AjPCod codon,
		       AjPRange range, int width, AjPStr pro)
{
    int limit;
    int i;
    int j;
    
    int nr;
    int st;
    int en;

    char *p;
    char *q;
    char c;
    
    char tri[4];
    int  idx;
    
    
    tri[3]='0';
    
    ajStrToUpper(&s);

    /* Convert ranges to subsequence values */
    nr = ajRangeNumber(range);
    for(i=0;i<nr;++i)
    {
	range->start[i] -= beg;
	range->end[i] -= beg;
    }
    limit = ajStrLen(s);
    
    /* Test ranges for validity */
    for(i=0;i<nr;++i)
    {
	st=range->start[i];
	en=range->end[i];
	if(st<0 || st>=limit || en<0 || en>=limit)
	    ajFatal("Range outside length of sequence [%d-%d]",st+beg,
		    end+beg);
    }

    /* Set areas of sequence to translate to lower case */
    p=ajStrStr(s);
    for(i=0;i<nr;++i)
    {
	ajRangeValues(range,i,&st,&en);
	for(j=st;j<=en;++j)
	    p[j] = (char)tolower((int)p[j]);
    }
    

    /* Do the translation */
    q=ajStrStr(pro);
    for(i=0;i<limit;++i)
    {
	if(isupper((int)p[i]))
	{
	    q[i]=' ';
	    continue;
	}
	tri[0]=p[i];
	if(!p[i+1] || isupper((int)p[i+1]) || !p[i+2] || isupper((int)p[i+2]))
	{
	    q[i]=' ';
	    if(q[i+1])
		q[i+1]=' ';
	    if(q[i+2])
		q[i+2]=' ';
	    i+=2;
	    continue;
	}
	tri[1]=p[i+1];
	tri[2]=p[i+2];
	idx = ajCodIndexC(tri);
	if(codon->aa[idx]==27)
	    c='*';
	else
	    c=(char)(codon->aa[idx]+'A');
	q[i]=c;
	q[i+1]=q[i+2]=' ';
	i+=2;
    }

    return;
}




static void makeRuler(int len, int begin, char *ruler, int *npos)
{
    int i;

    for(i=0;i<len;++i)
    {
	npos[i]=i+begin;
	if(!((i+begin)%10))
	    ruler[i]='|';
	else
	    ruler[i]='-';
    }
}





static void calcProteinPos(int *ppos, AjPStr pro, int len)
{
    int j;
    
    int pos;
    int v;
    
    char *p;
    


    pos=0;
    v=1;
	
    p=ajStrStr(pro);
    while(p[pos]==' ')
	ppos[pos++]=0;
	    
    while(pos<len)
    {
	if(p[pos]=='*')
	{
	    ppos[pos]=0;
	    ++pos;
	    while(p[pos]==' ')
	    {
		ppos[pos]=0;
		++pos;
	    }
	    v=1;
	    continue;
	}
	    
	if(p[pos]!=' ')
	{
	    ppos[pos]=v+POFF;
	    ++pos;
	    for(j=0;j<2 && p[pos];++j,++pos)
		ppos[pos]=v;
	    if(p[pos]==' ')
		v=1;
	    else
		++v;
	}
	else
	    ppos[pos++]=0;
    }

    return;
}







static void showTrans(int *ppos, int *npos, AjPStr pro, AjPStr substr,
		 int len, char *ruler, int begin,
		 AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		 int width, char *name)
{
    int pos;

    ajFmtPrintF(outf,"PRETTYSEQ of %s from %d to %d\n\n",name,begin,
		begin+len-1);
    

    pos=0;
    while(pos<len)
    {
	if(pos+width<len)
	{
	    showTransb(ppos,npos,pro,substr,len,ruler,begin,
			 outf,isrule,isp,isn,pos,pos+width-1);
	    pos += width;
	    continue;
	}
	showTransb(ppos,npos,pro,substr,len,ruler,begin,
		     outf,isrule,isp,isn,pos,len-1);
	break;
    }

    return;
}




static void showTransb(int *ppos, int *npos, AjPStr pro, AjPStr substr,
		  int len, char *ruler, int begin,
		  AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		  int start, int end)
{
    AjPStr s;
    int b;
    int e=0;
    int v;
    int pos;
    
    s=ajStrNew();
    
    if(isrule)
    {
	ajStrAssSubC(&s,ruler,start,end);
	ajFmtPrintF(outf,"           %s\n",ajStrStr(s));
    }

    if(isn)
	ajFmtPrintF(outf,"%10d ",npos[start]);
    else
	ajFmtPrintF(outf,"           ");
    ajStrAssSub(&s,substr,start,end);
    ajFmtPrintF(outf,"%s ",ajStrStr(s));
    if(isn)
	ajFmtPrintF(outf,"%d",npos[end]);
    ajFmtPrintF(outf,"\n");

    if(isp)
    {
	pos=start;
	b=e=0;
	while(pos<=end)
	{
	    if(!(v=ppos[pos]))
	    {
		++pos;
		continue;
	    }
	    if(v<POFF)
	    {
		while(ppos[pos]==v && pos<=end)
		    ++pos;
		if(pos>end)
		    break;
		continue;
	    }
	    b=v-POFF;
	    break;
	}
	pos=end;
	while(pos>=start)
	{
	    if(!(v=ppos[pos]))
	    {
		--pos;
		continue;
	    }
	    if(v>POFF)
		v-=POFF;
	    else
		while(ppos[pos]==v)
		{
		    --pos;
		    if(pos<start)
		    {
			v=0;
			break;
		    }
		}
	    e=v;
	    break;
	}

	if(!b)
	    ajFmtPrintF(outf,"           ");
	else
	    ajFmtPrintF(outf,"   %7d ",b);
    }
    else
	ajFmtPrintF(outf,"           ");
	
    ajStrAssSub(&s,pro,start,end);
    ajFmtPrintF(outf,"%s ",ajStrStr(s));
    if(isp && e)
	ajFmtPrintF(outf,"%d",e);
    ajFmtPrintF(outf,"\n");

    
    ajFmtPrintF(outf,"\n");

    ajStrDel(&s);
    
    return;
}

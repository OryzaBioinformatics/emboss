/* @source showorf application
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


#define POFF 1000000


void ajSixTranslate(AjPStr substr, AjPStr revstr, ajint len,
		    AjPStr *pseqs, ajint begin, AjPCod codon);
void ajDoTrans(AjPStr s, AjPStr r, ajint n, ajint len, AjPStr *pseqs,
	       AjPCod codon, ajint begin);
void ajMakeRuler(ajint len, ajint begin, char *ruler, ajint *npos);
void ajCalcProteinPos(ajint **ppos, AjPStr *pseqs, ajint len);
static void showTrans(ajint **ppos, ajint *npos, AjPStr *pseqs, AjPStr substr,
		 ajint len, ajint *mark, char *ruler, ajint begin,
		 AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		 ajint width, char *name);
static void showTransb(ajint **ppos, ajint *npos, AjPStr *pseqs, AjPStr substr,
		  ajint len, ajint *mark, char *ruler, ajint begin,
		  AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		  ajint start, ajint end);





int main(int argc, char **argv)
{
    AjPSeq       a;
    AjPFile      outf;
    AjPCod       codon;
    AjPStr       substr;
    AjPStr	 revstr;
    AjPStr       *frames;
    AjPStr	 *f;
    AjBool	 isrule;
    AjBool       isp;
    AjBool       isn;


    AjPStr       pseqs[6];
    char	 *ruler;

    ajint          *npos=NULL;
    ajint		 *ppos[6];
    ajint          mark[6];
    
    ajint beg;
    ajint end;
    ajint len;
    ajint i;
    ajint v=0;
    
    char *p;
    
    ajint width;
    
    embInit("showorf", argc, argv);

    a         = ajAcdGetSeq("sequence");
    codon     = ajAcdGetCodon("cfile");
    width     = ajAcdGetInt("width");
    outf      = ajAcdGetOutfile("outfile");
    frames    = ajAcdGetList("frames");
    isrule    = ajAcdGetBool("ruler");
    isp       = ajAcdGetBool("plabel");
    isn       = ajAcdGetBool("nlabel");
    
    for(i=0;i<6;++i)
	mark[i]=0;
    f=frames;
    while(*f)
    {
	p=ajStrStr(*f);
	sscanf(p,"%d",&v);
	if(!v)
	{
	    for(i=0;i<6;++i)
		mark[i]=0;
	    break;
	}
	mark[v-1]=1;
	++f;
    }

    
    beg = ajSeqBegin(a);
    end = ajSeqEnd(a);

    substr = ajStrNew();
    ajStrAssSubC(&substr,ajSeqChar(a),beg-1,end-1);
    len = ajStrLen(substr);

    revstr = ajStrNewC(ajStrStr(substr));
    ajSeqReverseStr(&revstr);

    /*
     *  Allocate memory for translations and positions
     */
    for(i=0;i<6;++i)
    {
	pseqs[i]=ajStrNewC(ajStrStr(substr));
	AJCNEW(ppos[i], len);
    }
    AJCNEW (ruler, len);
    AJCNEW (npos, len);
    
    ajSixTranslate(substr,revstr,len,pseqs,beg,codon);
    ajMakeRuler(len,beg,ruler,npos);
    ajCalcProteinPos(ppos,pseqs,len);
    showTrans(ppos,npos,pseqs,substr,len,mark,ruler,beg,
		outf,isrule,isp,isn,width,ajSeqName(a));
    

    for(i=0;i<6;++i)
    {
	ajStrDel(&pseqs[i]);
	AJFREE (ppos[i]);
    }
    AJFREE (ruler);
    AJFREE (npos);
    
    ajStrDel(&revstr);
    ajStrDel(&substr);
    ajCodDel(&codon);
    
    ajExit();
    return 0;
}



void ajSixTranslate(AjPStr substr, AjPStr revstr, ajint len,
		    AjPStr *pseqs, ajint begin, AjPCod codon)
{
    ajint i;
    


    for(i=0;i<6;++i)
    {
	ajDoTrans(substr, revstr, i, len, pseqs, codon, begin);
	if(i>2)
	    ajStrRev(&pseqs[i]);
    }

    return;
}



void ajDoTrans(AjPStr s, AjPStr r, ajint n, ajint len, AjPStr *pseqs,
	       AjPCod codon, ajint begin)
{
    char *p;
    char *q;

    ajint po;
    
    ajint i;
    ajint c;
    ajint idx;
    
    char tri[4];
    

    if(n<3)
    {
	p=ajStrStr(s);
	po = n%3;
    }
    else
    {
	p=ajStrStr(r);
	po = len%3;
	po -= n%3;
	if(po<0)
	    po+=3;
    }

    for(i=0,q=ajStrStr(pseqs[n]);i<po;++i)
	q[i]=' ';


    for(i=po,c=0,tri[3]='\0';i<len;++i,++c)
    {
	if(c%3 || len-i<3)
	{
	    q[i]=' ';
	    continue;
	}

	tri[0]=p[i];
	tri[1]=p[i+1];
	tri[2]=p[i+2];
	idx = ajCodIndexC(tri);
	if(codon->aa[idx]==27)
	    q[i]='*';
	else
	    q[i]=(char)(codon->aa[idx]+'A');
    }

    return;
}




void ajMakeRuler(ajint len, ajint begin, char *ruler, ajint *npos)
{
    ajint i;

    for(i=0;i<len;++i)
    {
	npos[i]=i+begin;
	if(!((i+begin)%10))
	    ruler[i]='|';
	else
	    ruler[i]='-';
    }
}




void ajCalcProteinPos(ajint **ppos, AjPStr *pseqs, ajint len)
{
    ajint i;
    ajint j;
    
    ajint pos;
    ajint v;
    
    char *p;
    

    for(i=0;i<3;++i)
    {
	pos=0;
	v=1;
	
	p=ajStrStr(pseqs[i]);
	while(p[pos]==' ')
	    ppos[i][pos++]=0;
	    
	while(pos<len)
	{
	    if(p[pos]=='*')
	    {
		ppos[i][pos]=0;
		++pos;
		while(p[pos]==' ')
		{
		    ppos[i][pos]=0;
		    ++pos;
		}
		v=1;
		continue;
	    }
	    
	    if(p[pos]!=' ')
	    {
		ppos[i][pos]=v+POFF;
		++pos;
		for(j=0;j<2 && p[pos];++j,++pos)
		    ppos[i][pos]=v;
		if(p[pos]==' ')
		    v=1;
		else
		    ++v;
	    }
	    else
		ppos[i][pos++]=0;
	}
    }





    for(i=3;i<6 && len;++i)
    {
	pos=len-1;
	v=1;
	
	p=ajStrStr(pseqs[i]);
	while(p[pos]==' ')
	    ppos[i][pos--]=0;
	    
	while(pos>-1)
	{
	    if(p[pos]=='*')
	    {
		ppos[i][pos]=0;
		--pos;
		while(p[pos]==' ')
		{
		    ppos[i][pos]=0;
		    --pos;
		}
		v=1;
		continue;
	    }
	    
	    if(p[pos]!=' ')
	    {
		ppos[i][pos]=v+POFF;
		--pos;
		for(j=0;j<2 && p[pos];++j,--pos)
		    ppos[i][pos]=v;
		if(pos<0) continue;
		if(p[pos]==' ')
		    v=1;
		else
		    ++v;
	    }
	    else
		ppos[i][pos--]=0;
	}
    }

    return;
}




static void showTrans(ajint **ppos, ajint *npos, AjPStr *pseqs, AjPStr substr,
		 ajint len, ajint *mark, char *ruler, ajint begin,
		 AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		 ajint width, char *name)
{
    ajint pos;

    ajFmtPrintF(outf,"SHOWORF of %s from %d to %d\n\n",name,begin,
		begin+len-1);
    

    pos=0;
    while(pos<len)
    {
	if(pos+width<len)
	{
	    showTransb(ppos,npos,pseqs,substr,len,mark,ruler,begin,
			 outf,isrule,isp,isn,pos,pos+width-1);
	    pos += width;
	    continue;
	}
	showTransb(ppos,npos,pseqs,substr,len,mark,ruler,begin,
		     outf,isrule,isp,isn,pos,len-1);
	break;
    }

    return;
}




static void showTransb(ajint **ppos, ajint *npos, AjPStr *pseqs, AjPStr substr,
		  ajint len, ajint *mark, char *ruler, ajint begin,
		  AjPFile outf, AjBool isrule, AjBool isp, AjBool isn,
		  ajint start, ajint end)
{
    AjPStr s;
    static char *fr[]=
    {
	"F1","F2","F3","R1","R2","R3"
    };
    ajint i;
    ajint b;
    ajint e=0;
    ajint v;
    ajint pos;
    
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

    for(i=0;i<3;++i)
    {
	if(!mark[i]) continue;
	ajFmtPrintF(outf,"%s ",fr[i]);
	if(isp)
	{
	    pos=start;
	    b=e=0;
	    while(pos<=end)
	    {
		if(!(v=ppos[i][pos]))
		{
		    ++pos;
		    continue;
		}
		if(v<POFF)
		{
		    while(ppos[i][pos]==v && pos<=end)
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
		if(!(v=ppos[i][pos]))
		{
		    --pos;
		    continue;
		}
		if(v>POFF)
		    v-=POFF;
		else
		    while(ppos[i][pos]==v)
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
		ajFmtPrintF(outf,"        ");
	    else
		ajFmtPrintF(outf,"%7d ",b);
	}
	else
	    ajFmtPrintF(outf,"        ");
	
	ajStrAssSub(&s,pseqs[i],start,end);
	ajFmtPrintF(outf,"%s ",ajStrStr(s));
	if(isp && e)
	    ajFmtPrintF(outf,"%d",e);
	ajFmtPrintF(outf,"\n");
    }
    




    for(i=3;i<6;++i)
    {
	if(!mark[i]) continue;
	ajFmtPrintF(outf,"%s ",fr[i]);
	if(isp)
	{
	    pos=start;
	    b=e=0;
	    while(pos<=end)
	    {
		if(!(v=ppos[i][pos]))
		{
		    ++pos;
		    continue;
		}
		if(v>POFF)
		    v-=POFF;
		else
		    while(ppos[i][pos]==v)
		    {
			++pos;
			if(pos>end)
			{
			    v=0;
			    break;
			}
		    }

		b=v;
		break;
	    }
	    pos=end;
	    while(pos>=start)
	    {
		if(!(v=ppos[i][pos]))
		{
		    --pos;
		    continue;
		}
		if(v<POFF)
		{
		    while(ppos[i][pos]==v && pos>=start)
			--pos;
		    if(pos<start)
			break;
		    continue;
		}
		e=v-POFF;
		break;
	    }
	    if(!b)
		ajFmtPrintF(outf,"        ");
	    else
		ajFmtPrintF(outf,"%7d ",b);
	}
	else
	    ajFmtPrintF(outf,"        ");
	
	ajStrAssSub(&s,pseqs[i],start,end);
	ajFmtPrintF(outf,"%s ",ajStrStr(s));
	if(isp && e)
	    ajFmtPrintF(outf,"%d",e);
	ajFmtPrintF(outf,"\n");
    }

    ajFmtPrintF(outf,"\n");

    ajStrDel(&s);
    
    return;
}

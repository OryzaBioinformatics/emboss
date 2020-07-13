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

static void prettyseq_makeRuler(ajint len, ajint begin, char *ruler,
				ajint *npos);
static void prettyseq_calcProteinPos(ajint *ppos, AjPStr pro,
				     ajint len);
static void prettyseq_showTrans(ajint *ppos, ajint *npos, AjPStr pro,
				AjPStr substr, ajint len, char *ruler,
				ajint begin, AjPFile outf, AjBool isrule,
				AjBool isp, AjBool isn, ajint width,
				char *name);
static void prettyseq_showTransb(ajint *ppos, ajint *npos, AjPStr pro,
				 AjPStr substr, ajint len, char *ruler,
				 ajint begin, AjPFile outf, AjBool isrule,
				 AjBool isp, AjBool isn, ajint start,
				 ajint end);
static void prettyseq_Translate(AjPFile outf, ajint beg, ajint end, AjPStr s,
				AjPCod codon, AjPRange range, ajint width,
				AjPStr pro);


/* @prog prettyseq ************************************************************
**
** Output sequence with translated ranges
**
******************************************************************************/

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
    
    ajint beg;
    ajint end;
    ajint len;
    
    ajint width;

    char *ruler;
    ajint  *ppos=NULL;
    ajint  *npos=NULL;
    

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

    prettyseq_Translate(outf,beg,end,substr,codon,range,width,pro);
    prettyseq_makeRuler(len,beg,ruler,npos);
    prettyseq_calcProteinPos(ppos,pro,len);
    prettyseq_showTrans(ppos,npos,pro,substr,len,ruler,beg,
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


/* @funcstatic prettyseq_Translate ********************************************
**
** Undocumented.
**
** @param [w] outf [AjPFile] outfile
** @param [r] beg [ajint] start position
** @param [r] end [ajint] end position
** @param [r] s [AjPStr] nucleic acid sequence
** @param [r] codon [AjPCod] translation CU table
** @param [r] range [AjPRange] region to translate
** @param [r] width [ajint] screen width
** @param [w] pro [AjPStr] protein
** @@
******************************************************************************/


static void prettyseq_Translate(AjPFile outf, ajint beg, ajint end, AjPStr s,
				AjPCod codon, AjPRange range, ajint width,
				AjPStr pro)
{
    ajint limit;
    ajint i;
    ajint j;
    
    ajint nr;
    ajint st;
    ajint en;

    char *p;
    char *q;
    char c;
    
    char tri[4];
    ajint  idx;
    
    
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
	    p[j] = (char)tolower((ajint)p[j]);
    }
    

    /* Do the translation */
    q=ajStrStr(pro);
    for(i=0;i<limit;++i)
    {
	if(isupper((ajint)p[i]))
	{
	    q[i]=' ';
	    continue;
	}
	tri[0]=p[i];
	if(!p[i+1] || isupper((ajint)p[i+1]) || !p[i+2] || isupper((ajint)p[i+2]))
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



/* @funcstatic prettyseq_makeRuler ********************************************
**
** Create a ruler
**
** @param [r] len [ajint] length for ruler
** @param [r] begin [ajint] numbering start
** @param [w] ruler [char*] ruler
** @param [w] npos [ajint*] numbering
** @@
******************************************************************************/


static void prettyseq_makeRuler(ajint len, ajint begin, char *ruler,
				ajint *npos)
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




/* @funcstatic prettyseq_calcProteinPos ***************************************
**
** Protein translation positions
**
** @param [w] ppos [ajint*] positions
** @param [r] pro [AjPStr] protein
** @param [r] len [ajint] length
** @@
******************************************************************************/


static void prettyseq_calcProteinPos(ajint *ppos, AjPStr pro, ajint len)
{
    ajint j;
    
    ajint pos;
    ajint v;
    
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







/* @funcstatic prettyseq_showTrans ********************************************
**
** Show translations
**
** @param [r] ppos [ajint*] protein positions
** @param [r] npos [ajint*] nucleic positions
** @param [r] pro [AjPStr] protein
** @param [r] substr [AjPStr] dna
** @param [r] len [ajint] length
** @param [r] ruler [char*] ruler
** @param [r] begin [ajint] start in dna
** @param [w] outf [AjPFile] outfile
** @param [r] isrule [AjBool] show ruler
** @param [r] isp [AjBool] show protein
** @param [r] isn [AjBool] show dna
** @param [r] width [ajint] display width
** @param [r] name [char*] name of dna
** @@
******************************************************************************/

static void prettyseq_showTrans(ajint *ppos, ajint *npos, AjPStr pro,
				AjPStr substr, ajint len, char *ruler,
				ajint begin, AjPFile outf, AjBool isrule,
				AjBool isp, AjBool isn, ajint width,
				char *name)
{
    ajint pos;

    ajFmtPrintF(outf,"PRETTYSEQ of %s from %d to %d\n\n",name,begin,
		begin+len-1);
    

    pos=0;
    while(pos<len)
    {
	if(pos+width<len)
	{
	    prettyseq_showTransb(ppos,npos,pro,substr,len,ruler,begin,
				 outf,isrule,isp,isn,pos,pos+width-1);
	    pos += width;
	    continue;
	}
	prettyseq_showTransb(ppos,npos,pro,substr,len,ruler,begin,
			     outf,isrule,isp,isn,pos,len-1);
	break;
    }

    return;
}



/* @funcstatic prettyseq_showTransb *******************************************
**
** Low level display
**
** @param [r] ppos [ajint*] protein positions
** @param [r] npos [ajint*] nucleic positions
** @param [r] pro [AjPStr] protein
** @param [r] substr [AjPStr] dna
** @param [r] len [ajint] length
** @param [r] ruler [char*] ruler
** @param [r] begin [ajint] start in dna
** @param [w] outf [AjPFile] outfile
** @param [r] isrule [AjBool] show ruler
** @param [r] isp [AjBool] show protein
** @param [r] isn [AjBool] show dna
** @param [r] start [ajint] start pos
** @param [r] end [ajint] end pos
** @@
******************************************************************************/


static void prettyseq_showTransb(ajint *ppos, ajint *npos, AjPStr pro,
				 AjPStr substr, ajint len, char *ruler,
				 ajint begin, AjPFile outf, AjBool isrule,
				 AjBool isp, AjBool isn, ajint start,
				 ajint end)
{
    AjPStr s;
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

/* @source sigcleave application
**
** Displays protein signal cleavage sites
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
**
** Original program "SIGCLEAVE" Peter Rice (EGCG 1988)
**
** This program uses the method of von Heijne (1986); Nuc. Acids Res. 14:4683
** as modified by von Heijne (1987) in "Sequence Analysis in Molecular
** Biology, where the procedure for positions -1 and -3 is altered.
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
#include <stdlib.h>

#define EUKFILE "Esig.euk"
#define PROFILE "Esig.pro"

int readSig(AjPFloat2d *matrix,AjBool prokaryote);





int main( int argc, char **argv, char **env)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPStr    stmp=NULL;
    AjPStr    sstr=NULL;
    
    AjBool    prokaryote=ajFalse;

    AjPFloat2d matrix=NULL;
    
    int begin;
    int end;
    int len;
    int pval;
    int nval;
    int opval;			/* save value from updating */
    float minweight;
    float xweight;
    float weight;
    float maxweight;

    AjPInt hi=NULL;
    AjPInt hp=NULL;
    AjPFloat hwt=NULL;
    
    char *p;
    char *q;
    
    int i;
    int j;

    int n;
    int maxsite;
    int istart;
    int iend;
    int ic;
    int isite;
    int se;

    embInit("sigcleave",argc,argv);
    
    seqall    = ajAcdGetSeqall("sequence");
    opval     = ajAcdGetInt("pval");
    nval      = ajAcdGetInt("nval");
    outf      = ajAcdGetOutfile("outfile");
    prokaryote   = ajAcdGetBool("prokaryote");
    minweight    = ajAcdGetFloat("minweight");

    matrix = ajFloat2dNew();
    hwt    = ajFloatNew();
    hi     = ajIntNew();
    hp     = ajIntNew();
    
    substr = ajStrNew();
    stmp   = ajStrNew();
    sstr   = ajStrNew();

    
    readSig(&matrix,prokaryote);


    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	pval = opval;

	ajStrAssS(&strand, ajSeqStr(seq));
	ajStrToUpper(&strand);
        ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);
        ajStrAssSubC(&sstr,ajStrStr(strand),begin-1,end-1);
	len    = ajStrLen(substr);

        q = p = ajStrStr(substr);
	for(i=0;i<len;++i,++p)
	    *p = (char) ajAZToInt(*p);
	p=q;

	maxsite = n = 0;
	maxweight = 0.0;

	ajDebug ("begin: %d end: %d len: %d nval: %d pval: %d\n",
		 begin, end, len, nval, pval);

	for(i=0;i<len;++i)
	{
	    weight=0.0;
	    istart = (0 > i+pval) ? 0 : i+pval;
	    iend = (i+nval-1 < len-1) ? i+nval-1 : len-1;
	    ic = 13+pval;
	    j=istart;
	    ajDebug ("i: %3d j: %3d istart: %3d iend: %3d ic: %3d\n",
		     i, j, istart, iend, ic);
	    while(j<=iend)
	    {
	        if(j>=0 && j<len) {
	            ajDebug ("j: %d ic: %d aa: '%c'\n",
			     j, ic, ajStrChar(ajSeqStr(seq), j));
		    weight += ajFloat2dGet(matrix,(int)*(p+j),ic);
	        }
		++ic;
		++j;
	    }
	    if(weight>maxweight)
	    {
		maxweight=weight;
		maxsite=i;
	    }
	    if(weight>minweight)
	    {
		ajFloatPut(&hwt,n,weight);;
		ajIntPut(&hp,n,i);
		ajIntPut(&hi,n,n);
		ajDebug ("hit[%d] at weight: %.3f i: %d\n", n, weight, i);
		++n;
	    }
	}

	ajFmtPrintF(outf,"\n\nSIGCLEAVE of %s from %d to %d\n\n",
		    ajSeqName(seq),begin,end);
	ajFmtPrintF(outf,"\nReporting scores over %.2f\n",minweight);
	if(!n)
	{
	    ajFmtPrintF(outf,"\nNo scores over %.2f\n",minweight);
	}
	else
	{
	    ajFmtPrintF(outf,"Maximum score %.1f at residue %d\n\n",
			maxweight, maxsite+begin);
	    if(maxsite+pval<0) pval = -maxsite;

            ajStrAssSubC(&stmp,ajStrStr(sstr),maxsite+pval,maxsite-1);
	    ajFmtPrintF(outf," Sequence:  %s-",ajStrStr(stmp));

	    if(maxsite+49<len)
	      se=maxsite+49;
	    else
	      se = len-1;

	    ajStrAssSubC(&stmp,ajStrStr(sstr),maxsite,se);
	    ajFmtPrintF(outf,"%s\n",ajStrStr(stmp));
	    ajFmtPrintF(outf,"            | (signal)    | (mature peptide)\n");
	    ajFmtPrintF(outf,"%13d             %d\n",maxsite+pval+begin,
		      maxsite+begin);
	    ajSortFloatIncI(ajFloatFloat(hwt),ajIntInt(hi),n);
	    if (n <= 1)
		ajFmtPrintF(outf,"\n\n\n No other entries above %.2f\n",
			    minweight);
	    else
	    {
	      ajFmtPrintF(outf,"\n\n\n Other entries above %.2f\n",minweight);
	      for(i=0;i<n;++i)
	      {
		isite=ajIntGet(hp,ajIntGet(hi,i));
		if(isite != maxsite)
		{
		    if(isite+pval<0) /*pval = -isite*/ continue;

		    xweight=ajFloatGet(hwt,ajIntGet(hi,i));
		    ajFmtPrintF(outf,"\n\nScore %.1f at residue %d\n\n",
				xweight,isite+begin);

		    
		    ajStrAssSubC(&stmp,ajStrStr(sstr),isite+pval,isite-1);
		    ajFmtPrintF(outf," Sequence:  %s-",ajStrStr(stmp));
		    if(isite+49<len) se=isite+49;
		    else se = len-1;
		    ajStrAssSubC(&stmp,ajStrStr(sstr),isite,se);
		    ajFmtPrintF(outf,"%s\n",ajStrStr(stmp));
		    ajFmtPrintF(outf,
			   "            | (signal)    | (mature peptide)\n");
		    ajFmtPrintF(outf,"%13d             %d\n",isite+pval+begin,
				isite+begin);
		}
	      }
	    }
	}

	ajStrDelReuse(&strand);
    }
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajStrDel(&sstr);
    ajStrDel(&stmp);

    ajFloat2dDel(&matrix);
    ajFloatDel(&hwt);
    ajIntDel(&hi);
    ajIntDel(&hp);

    ajFileClose(&outf);
    
    ajExit();
    return 0;
}





int readSig(AjPFloat2d *matrix,AjBool prokaryote)
{
    AjPFile mfptr=NULL;
    AjPStr  line=NULL;
    AjPStr  delim=NULL;
    AjBool  pass;

    float **mat;
    
    char *p;
    char *q;
    
    int xcols=0;
    int cols=0;
    float rt;
    float v;
    
    float sample;
    float expected;

    int   i;
    int   j;
    int   c;
    
    int d1;
    int d2;
    
    if(prokaryote)
	ajFileDataNewC(PROFILE,&mfptr);
    else
	ajFileDataNewC(EUKFILE,&mfptr);
    if(!mfptr) ajFatal("SIG file  not found\n");


    line=ajStrNew();
    delim=ajStrNewC(" :\t\n");

    pass = ajTrue;
    
    while(ajFileGets(mfptr, &line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || *p=='\n') continue;
	if(ajStrPrefixC(line,"Sample:"))
	{
	    if(sscanf(p,"%*s%f",&sample)!=1)
		ajFatal("No sample size given");
	    continue;
	}
	while((*p!='\n') && (*p<'A' || *p>'Z')) ++p;
	cols = ajStrTokenCount(&line,ajStrStr(delim));
	if(pass)
	{
	    pass=ajFalse;
	    xcols = cols;
	}
	else
	    if(xcols!=cols)
		ajFatal("Assymetric table");

	q=ajStrStr(line);
	q=ajSysStrtok(q,ajStrStr(delim));

	d1 = ajAZToInt((char)toupper((int)*p));
	c  = 0;
	while((q=ajSysStrtok(NULL,ajStrStr(delim))))
	{
	    sscanf(q,"%f",&v);
	    ajFloat2dPut(matrix,d1,c++,v);
	}
    }


    ajFloat2dLen(*matrix,&d1,&d2);
    mat = ajFloat2dFloat(*matrix);
    
    for(j=0;j<d2;++j)
    {
	rt=0.;
	
	for(i=0;i<d1;++i)
	    rt += mat[i][j];

	if(j==cols-2)
	{
	    if(fabs((double)(sample-rt)) > 0.9)
		ajFatal("Error in 'Expected' column");
	}
	else
	    if(fabs((double)(sample-rt)) > 0.05)
		ajFatal("Sample size doesn't match column");
    }

    for(i=0;i<d1;++i)
    {
	expected = mat[i][d2-1];
	if(expected > 0.0)
	{
	    for(j=0;j<d2-1;++j)
	    {
		if(j==10 || j==12)
		{
		    if(!(int)mat[i][j]) mat[i][j]=1.0e-10;
		}
		else
		{
		    if(!(int)mat[i][j]) mat[i][j]=1.0;
		}
		mat[i][j] = (float)(log((double)(mat[i][j]/expected)));
	    }
	}
    }

    
    for(i=0;i<d1;++i)
	for(j=0;j<d2;++j)
	    ajFloat2dPut(matrix,i,j,mat[i][j]);

    for(i=0;i<d1;++i)
	AJFREE(mat[i]);
    AJFREE(mat);


    ajStrDel(&line);
    ajStrDel(&delim);
    ajFileClose(&mfptr);

    return cols;
}

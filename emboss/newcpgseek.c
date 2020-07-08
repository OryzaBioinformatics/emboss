/*  Last edited: Sep  9 13:39 1999 (pmr) */
/* @source newcpgseek application
**
** Reports ALL cpg rich regions in a sequence
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @author: Modified by Rodrigo Lopez (rls@ebi.ac.uk)
** @@
**
** Original program "CPGREPORT" by Rodrigo Lopez (EGCG 1995)
**  CpG island finder. Larsen et al Genomics 13 1992 p1095-1107
**  "usually defined as >200bp with %GC > 50% and obs/exp CpG >
**  0.6". Here use running sum rather than window to score: if not CpG
**  at position i, then decrement runSum counter, but if CpG then runSum
**  += CPGSCORE.     Spans > threshold are searched
**  for recursively.
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


void cpgsearch(AjPFile *outf, int s, int len, char *seq, char *name,
	       int begin, int *score);
void calcgc(int from, int to, char *p, int *dcg, int *dgc, int *gc);

int main( int argc, char **argv, char **env)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    
    int begin;
    int end;
    int len;
    int score;

    embInit("newcpgseek",argc,argv);
    
    seqall    = ajAcdGetSeqall("sequence");
    score     = ajAcdGetInt("score");
    outf      = ajAcdGetOutfile("outfile");

    
    seq=ajSeqNew();
    substr = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	
	strand = ajSeqStrCopy(seq);
	ajStrToUpper(&strand);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);

	len=ajStrLen(substr);

	ajFmtPrintF(outf,"\n\nNEWCPGSEEK of %s from %d to %d\n",
		    ajSeqName(seq),begin,begin+len-1);
	ajFmtPrintF(outf,"with score > %d \n\n",score);
	
	ajFmtPrintF(outf," Begin    End  Score");
	ajFmtPrintF(outf,"        CpG  %%CG  CG/GC\n");

	cpgsearch(&outf,0,len,ajStrStr(substr),ajSeqName(seq),begin,&score);
	ajFmtPrintF(outf,"-------------------------------------------\n");
	
	ajStrDel(&strand);
    }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);
    
    ajExit();
    return 0;
}


void cpgsearch(AjPFile *outf, int from, int to, char *p, char *name,
	       int begin, int *score)
{
    int i;
    int c;
    int z;
    
    int sum;
    int ssum; 
    int lsum;
    int t;
    int top;
  
    int dcg;
    int dgc;
    int gc;



    for(i=from,c=to-1,sum=ssum=t=top=0,lsum=-1,z=begin-1;i<to;++i,ssum=sum)
    {
	if(p[i]=='C' && p[i+1]=='G' && c-i) sum+=*score+1;
	--sum;
	if(sum<0) sum=0;
	if(!sum && ssum)
	{
	    calcgc(lsum+1,t+2,p,&dcg,&dgc,&gc);
	    if(dgc)
	    {
		ajFmtPrintF(*outf,"%6d %6d %5d ",lsum+2+z,
			    t+2+z,top);
		ajFmtPrintF(*outf,"     %5d %5.1f %6.2f\n",
			    dcg,(float)gc*100.0/(float)(t+1-lsum),
			    (float)(dcg/dgc));
	    }
	    else
	    {
/* Don't know what the fuck this is doing here!
		ajFmtPrintF(*outf,"%6d %6d %5d ",lsum+2+z,t+2+z,
			    top);
		ajFmtPrintF(*outf,"     %5d %5.1f    -\n",
			    dcg,(float)gc*100.0/(float)(t+1-lsum));
*/
	    }
	    cpgsearch(outf,t+2,i,p,name,begin,score);
	    sum=ssum=lsum=t=top=0;
	}
	if(sum>top)
	{
	    t=i;
	    top=sum;
	}
	if(!sum) lsum=i;
    }
  
  
    if(sum)
    {
	calcgc(lsum+1,t+2,p,&dcg,&dgc,&gc);
	if(dgc)
	{
	    ajFmtPrintF(*outf,"*%6d %6d %5d ",lsum+2+z,t+2+z,
			top);
	    ajFmtPrintF(*outf,"     %5d %5.1f %6.2f\n",
			dcg,(float)gc*100.0/(float)(t+1-lsum),
			((float)dcg/(float)dgc));
	}
	else
	{
/*	    ajFmtPrintF(*outf,"%6d %6d %5d ",lsum+2+z,t+2+z,top);
	    ajFmtPrintF(*outf,"     %5d %5.1f    -\n",dcg,
			(float)gc*100.0/(float)(t+1-lsum));
*/
	}
	cpgsearch(outf,t+2,to,p,name,begin,score);
    }
}

void calcgc(int from, int to, char *p, int *dcg, int *dgc, int *gc)
{

    int i;
    int c;

    c=to-1;

    for(i=from,*gc=*dgc=*dcg=0;i<=c;++i) 
    {
	if(p[i]=='G' || p[i]=='C') ++*gc;
	if(p[i]=='C' && p[i+1]=='G' && c-i) ++*dcg ; 
	if(p[i]=='G' && p[i+1]=='C' && c-i ) ++*dgc ; 
    }
}

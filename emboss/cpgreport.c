/* @source cpgreport application
**
** Reports ALL cpg rich regions in a sequence
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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


static void cpgreport_cpgsearch(AjPFile *outf, ajint s, ajint len, char *seq,
				char *name, ajint begin, ajint *score,
				AjPFeattabOut featout,
				AjPFeattable *feattable);
static void cpgreport_calcgc(ajint from, ajint to, char *p, ajint *dcg,
			     ajint *dgc, ajint *gc);




/* @prog cpgreport ************************************************************
**
** Reports all CpG rich regions
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPFeattabOut featout;
    AjPFeattable feattable=NULL;    
    ajint begin;
    ajint end;
    ajint len;
    ajint score;

    embInit("cpgreport",argc,argv);
    
    seqall    = ajAcdGetSeqall("sequence");
    score     = ajAcdGetInt("score");
    outf      = ajAcdGetOutfile("outfile");
    featout   = ajAcdGetFeatout("featout");

    
    substr = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
	begin=ajSeqallBegin(seqall);
	end=ajSeqallEnd(seqall);
	
	strand = ajSeqStrCopy(seq);
	ajStrToUpper(&strand);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1);

	len=ajStrLen(substr);

	ajFmtPrintF(outf,"\n\nCPGREPORT of %s from %d to %d\n\n",
		    ajSeqName(seq),begin,begin+len-1);
	ajFmtPrintF(outf,"Sequence              Begin    End Score");
	ajFmtPrintF(outf,"        CpG   %%CG  CG/GC\n");

	cpgreport_cpgsearch(&outf,0,len,ajStrStr(substr),ajSeqName(seq),
			    begin,&score,featout,&feattable);
	ajStrDel(&strand);
    }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);
    ajFeatSortByStart(feattable);
    ajFeatWrite (featout, feattable);
    ajFeattableDel(&feattable);
    
    ajExit();
    return 0;
}








/* @funcstatic cpgreport_cpgsearch *******************************************
**
** Undocumented.
**
** @param [?] outf [AjPFile*] Undocumented
** @param [?] from [ajint] Undocumented
** @param [?] to [ajint] Undocumented
** @param [?] p [char*] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] score [ajint*] Undocumented
** @param [?] featout [AjPFeattabOut] Undocumented
** @param [?] feattable [AjPFeattable*] Undocumented
** @@
******************************************************************************/



static void cpgreport_cpgsearch(AjPFile *outf, ajint from, ajint to, char *p,
				char *name, ajint begin, ajint *score,
				AjPFeattabOut featout,
				AjPFeattable *feattable)
{
    ajint i;
    ajint c;
    ajint z;
    
    ajint sum;
    ajint ssum; 
    ajint lsum;
    ajint t;
    ajint top;
  
    ajint dcg;
    ajint dgc;
    ajint gc;
    static AjPStr name2=NULL,source=NULL,type=NULL;
    char  strand='+';
    ajint frame=0;
    AjPFeature feature;
    float score2 = 0.0;
    
    if(!name2)
    {
      ajStrAssC(&name2,name);
      
      *feattable = ajFeattableNewDna(name2);
      
      ajStrAssC(&source,"cpgreport");
      ajStrAssC(&type,"misc_feature");
    }


    for(i=from,c=to-1,sum=ssum=t=top=0,lsum=-1,z=begin-1;i<to;++i,ssum=sum)
    {
	if(p[i]=='C' && p[i+1]=='G' && c-i) sum+=*score+1;
	--sum;
	if(sum<0) sum=0;
	if(!sum && ssum)
	{
	    cpgreport_calcgc(lsum+1,t+2,p,&dcg,&dgc,&gc);
	    if(dgc)
	    {	      
	        score2 = (float) top;
		/*ajFmtPrintS(&score2,"%d.0",top);*/
	        feature = ajFeatNew(*feattable, source, type,
				    lsum+2+z,t+2+z,
				    score2, strand, frame) ;
		if(!feature)
		  ajDebug("Error feature not added to feature table");
		ajFmtPrintF(*outf,"%-20.20s %6d %6d %5d ",name,lsum+2+z,
			    t+2+z,top);
		ajFmtPrintF(*outf,"     %5d %5.1f %6.2f\n",
			    dcg,(float)gc*100.0/(float)(t+1-lsum),
			    (float)(dcg/dgc));
	    }
	    else
	    {
	      score2 = (float) top;
	      /*score2 = ajFmtPrintS(&score2,"%d.0",top);*/
	      feature = ajFeatNew(*feattable, source, type,
				  lsum+2+z,t+2+z,
				  score2, strand, frame) ;
	      ajFmtPrintF(*outf,"%-20s %6d %6d %5d ",name,lsum+2+z,t+2+z,
			  top);
		ajFmtPrintF(*outf,"     %5d %5.1f    -\n",
			    dcg,(float)gc*100.0/(float)(t+1-lsum));
	    }
	    cpgreport_cpgsearch(outf,t+2,i,p,name,begin,score,featout,
				feattable);
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
	cpgreport_calcgc(lsum+1,t+2,p,&dcg,&dgc,&gc);
	if(dgc)
	{
	    ajFmtPrintF(*outf,"%-20s %6d %6d %5d ",name,lsum+2+z,t+2+z,
			top);
	    ajFmtPrintF(*outf,"     %5d %5.1f %6.2f\n",
			dcg,(float)gc*100.0/(float)(t+1-lsum),
			((float)dcg/(float)dgc));
	    score2 = (float) top;
	    /*score2 = ajFmtPrintS(&score2,"%d.0",top);*/
	    feature = ajFeatNew(*feattable, source, type,
				lsum+2+z,t+2+z,
				score2, strand, frame) ;
	}
	else
	{
	    ajFmtPrintF(*outf,"%-20s %6d %6d %5d ",name,lsum+2+z,t+2+z,top);
	    ajFmtPrintF(*outf,"     %5d %5.1f    -\n",dcg,
			(float)gc*100.0/(float)(t+1-lsum));
	    score2 = (float) top;
	    /*score2 = ajFmtPrintS(&score2,"%d.0",top);*/
	    feature = ajFeatNew(*feattable, source, type,
				lsum+2+z,t+2+z,
				score2, strand, frame) ;
	}
	cpgreport_cpgsearch(outf,t+2,to,p,name,begin,score,featout,feattable);
    }

    return;
}





/* @funcstatic cpgreport_calcgc ***********************************************
**
** Undocumented.
**
** @param [?] from [ajint] Undocumented
** @param [?] to [ajint] Undocumented
** @param [?] p [char*] Undocumented
** @param [?] dcg [ajint*] Undocumented
** @param [?] dgc [ajint*] Undocumented
** @param [?] gc [ajint*] Undocumented
** @@
******************************************************************************/

static void cpgreport_calcgc(ajint from, ajint to, char *p, ajint *dcg,
			     ajint *dgc, ajint *gc)
{

    ajint i;
    ajint c;

    c=to-1;

    for(i=from,*gc=*dgc=*dcg=0;i<=c;++i) 
    {
	if(p[i]=='G' || p[i]=='C') ++*gc;
	if(p[i]=='C' && p[i+1]=='G' && c-i) ++*dcg ; 
	if(p[i]=='G' && p[i+1]=='C' && c-i ) ++*dgc ; 
    }

    return;
}

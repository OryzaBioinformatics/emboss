/* @source antigenic application
**
** Displays antigenic sites in proteins
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
** Original program "ANTIGENIC" by Peter Rice (EGCG 1991)
** Prediction of antigenic regions of protein sequences by method of:
** Kolaskar AS and Tongaonkar PC (1990) FEBS Letters 276:172-174
** "A semi-emipirical method for prediction of antigenic determinants
** on protein antigens"
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
#include <string.h>


#define DATAFILE "Eantigenic.dat"

void readAunty(AjPFloat *agp);
void padit(AjPFile *outf, int b, int e);
void dumptoFeat(int nhits, AjPInt hp,AjPInt hpos,AjPInt hlen,AjPFloat thisap,AjPFloat hwt, AjPFeatTabOut featout, 
		char *seqname,int begin);



int main( int argc, char **argv, char **env)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPFeatTabOut featout=NULL;
    AjPStr    strand=NULL;
    AjPStr    sstr=NULL;
    AjPStr    stmp=NULL;
    AjPStr    substr=NULL;
    
    int begin;
    int end;
    int len;
    int start;
    int stop;
    char *p;
    char *q;
    int i;
    int j;
    int k;
    int m;
    int fpos;
    int lpos;
    int maxlen;
    int maxpos;
    int minlen;
    int lenap;
    int istart;
    int iend;
    int nhits;

    AjPFloat thisap=NULL;
    AjPFloat hwt=NULL;
    AjPInt   hpos=NULL;
    AjPInt   hp=NULL;
    AjPInt   hlen=NULL;
    
    float resap;
    float totap;
    float averap;
    float minap;
    float score;
    float v;
    
    AjPFloat agp=NULL;

    embInit("antigenic", argc, argv);

    thisap = ajFloatNew();
    hwt    = ajFloatNew();
    hpos   = ajIntNew();
    hp     = ajIntNew();
    hlen   = ajIntNew();

    agp = ajFloatNew();

    readAunty(&agp);

    seqall    = ajAcdGetSeqall("sequence");
    minlen    = ajAcdGetInt("minlen");
    outf      = ajAcdGetOutfile("outfile");
    featout   = ajAcdGetFeatout("featout");

    substr = ajStrNew();
    sstr = ajStrNew();
    stmp = ajStrNew();

    while(ajSeqallNext(seqall, &seq))
    {
	begin = ajSeqallBegin(seqall);
	end   = ajSeqallEnd(seqall);
	start = begin-1;
	stop  = end-1;

	strand = ajSeqStrCopy(seq);
	
	ajStrToUpper(&strand);
	ajStrAssSubC(&substr,ajStrStr(strand),start,stop);
	ajStrAssSubC(&sstr,ajStrStr(strand),start,stop);
	len    = ajStrLen(substr);
	
	q = p = ajStrStr(substr);
	for(i=0;i<len;++i,++p)
	    *p = (char) ajAZToInt(*p);

	totap = 0.0;
	fpos  = 0;
	lpos  = len-7;
	
	for(i=0;i<len;++i)
	    ajFloatPut(&thisap,i,0.0);

	p=q;
	for(i=0;i<len;++i)
	{
	    resap = ajFloatGet(agp,(int)*(p+i));
	    totap += resap;
	    if( (i>=fpos) && (i<=lpos))
	    {
		ajFloatPut(&thisap,i+3,resap);
		for(j=i+1;j<=i+6;++j)
		    ajFloatPut(&thisap,i+3,ajFloatGet(thisap,i+3) +
			       ajFloatGet(agp,(int) *(p+j)));
		ajFloatPut(&thisap,i+3,ajFloatGet(thisap,i+3)/7.0);
	    }
	}

	averap=totap/(float)len;
	minap = (averap < 1.0) ? averap : 1.0;
	lenap = nhits = maxlen = maxpos = 0;

	for(i=fpos+3;i<=lpos+4;++i)
	    if(ajFloatGet(thisap,i) >= minap)
		++lenap;
	    else
	    {
		if(lenap >= minlen)
		{
		    score = 0.0;
		    for(j=i-lenap;j<=i-1;++j)
		    {
			v = ajFloatGet(thisap,j);
			score = (score > v) ? score : v;
		    }
		    ajIntPut(&hp,nhits,nhits);
		    ajIntPut(&hpos,nhits,i-lenap);
		    ajFloatPut(&hwt,nhits,score);
		    ajIntPut(&hlen,nhits++,lenap);
		}
		if(lenap>maxlen)
		{
		    maxlen=lenap;
		    maxpos=i-lenap;
		}
		lenap=0;
	    }
	    
	
	ajFmtPrintF(outf,"ANTIGENIC of %s  from: %d  to: %d\n\n",
		    ajSeqName(seq),begin,end);
	ajFmtPrintF(outf,"Length %d residues, score calc from %d to %d\n",
		ajSeqLen(seq),fpos+3+begin,lpos+3+begin);
	ajFmtPrintF(outf,"Reporting all peptides over %d residues\n\n",minlen);
	ajFmtPrintF(outf,
		    "Found %d hits scoring over %.2f (true average %.2f)\n",
		nhits,minap,averap);
	istart=maxpos;
	iend=maxpos+maxlen-1;
	ajFmtPrintF(outf,"Maximum length %d at residues %d->%d\n\n", maxlen,
		istart+begin, iend+begin);
	ajStrAssSubC(&stmp,ajStrStr(sstr),istart,iend);    
	ajFmtPrintF(outf," Sequence:  %S\n",stmp);
	ajFmtPrintF(outf,"            |");
	padit(&outf,istart,iend);
	ajFmtPrintF(outf,"|\n");
	ajFmtPrintF(outf,"%13d",istart+begin);
	padit(&outf,istart,iend);
	ajFmtPrintF(outf,"%d\n",iend+begin);

	if(nhits)
	{
	    ajSortFloatDecI(ajFloatFloat(hwt),ajIntInt(hp),nhits);
	    ajFmtPrintF(outf,
			"\nEntries in score order, max score at \"*\"\n\n");
	    for(i=nhits-1,j=0;i>-1;--i)
	    {
		k = ajIntGet(hp,i);
		istart = ajIntGet(hpos,k);
	    
		iend = istart + ajIntGet(hlen,k) -1;
		ajFmtPrintF(outf,
			    "\n[%d] Score %.3f length %d at residues %d->%d\n",
			    ++j,ajFloatGet(hwt,k),ajIntGet(hlen,k),
			    istart+begin,iend+begin);
		ajFmtPrintF(outf,"            ");
		for(m=istart;m<=iend;++m)
		    if(ajFloatGet(thisap,m) == ajFloatGet(hwt,k))
			ajFmtPrintF(outf,"*");
		    else
			ajFmtPrintF(outf," ");
		ajFmtPrintF(outf,"\n");
		ajStrAssSubC(&stmp,ajStrStr(sstr),istart,iend);	   ;
		ajFmtPrintF(outf," Sequence:  %S\n",stmp);
		ajFmtPrintF(outf,"            |");
		padit(&outf,istart,iend);
		ajFmtPrintF(outf,"|\n");
		ajFmtPrintF(outf,"%13d",istart+begin);
		padit(&outf,istart,iend);
		ajFmtPrintF(outf,"%d\n",iend+begin);
	    }
	    dumptoFeat(nhits,hp,hpos,hlen,thisap,hwt,featout,ajSeqName(seq),begin);
	}
	

	ajStrDel(&strand);

    }
    
    

    ajStrDel(&stmp);
    ajStrDel(&sstr);
    ajSeqDel(&seq);
    ajStrDel(&strand);
    ajFileClose(&outf);

    ajFloatDel(&thisap);
    ajFloatDel(&hwt);
    ajFloatDel(&agp);

    ajIntDel(&hpos);
    ajIntDel(&hp);
    ajIntDel(&hlen);
    
    ajExit();
    return 0;
}





void readAunty(AjPFloat *agp)
{
    AjPFile mfptr=NULL;
    AjPStr  line=NULL;
    int Etot;
    int Stot;
    int Ptot;
    int n;

    int v1;
    int v2;
    int v3;
    float vf1;
    float vf2;
    float vf3;
    
    int deltae;
    int deltas;
    int deltap;
    float deltaaf;
    float deltasf;
    
    char *p;
    char *q;
    
    ajFileDataNewC(DATAFILE, &mfptr);
    if(!mfptr) ajFatal("Antigenicity file not found\n");

    line=ajStrNew();

    deltae=deltas=deltap=0;
    deltaaf=deltasf=0.0;
    
    while(ajFileGets(mfptr, &line))
    {
	p=ajStrStr(line);
	if(*p=='#' || *p=='!' || *p=='\n') continue;
	if(strstr(p,"Total"))
	{
	    if(sscanf(p,"%*s%d%d%d",&Etot,&Stot,&Ptot) != 3)
		ajErr("Wrong number of fields in totals\n%s",
			ajStrStr(line));
	    continue;
	}
	
	ajCharToUpper(p);
	q=p;
	q=ajSysStrtok(q," \t");
	n=ajAZToInt(*q);
	
	if(sscanf(p,"%*s%d%d%d%f%f%f",&v1,&v2,&v3,&vf1,&vf2,&vf3)!=6)
	{
	    ajErr("Error in table: %s",p);
	    exit(0);
	}

	ajFloatPut(agp,n,vf3);
	
	deltae += v1;
	deltas += v2;
	deltap += v3;
	deltaaf += vf1;
	deltasf += vf2;

	if(fabs((double) (vf2-vf1 / vf3)) > 0.02)
	    ajErr ("propensity != afreq/sfreq in line\n%s",
		    p);
    }


    if(deltae != Etot)
	ajErr ("epitope total %d != total %d",Etot,
		deltae);
    if(deltas != Stot)
	ajErr ("surface total %d != total %d",Stot,
		deltas);
    if(deltap != Ptot)
	ajErr ("protein total %d != total %d",Ptot,
		deltap);
    if(fabs((double) (1.0-deltaaf)) > 0.005)
	ajErr ("afreq total %1.5f should be 1.0",deltaaf);
    if(fabs((double) (1.0-deltasf)) > 0.005)
	ajErr ("sfreq total %1.5f should be 1.0",deltasf);


    ajStrDel(&line);
    ajFileClose(&mfptr);
}



void padit(AjPFile *outf, int b, int e)
{
    int i;

    for(i=0;i<e-b-1;++i)
	ajFmtPrintF(*outf," ");

}


void dumptoFeat(int nhits, AjPInt hp, AjPInt hpos, AjPInt hlen,AjPFloat thisap,
		AjPFloat hwt, AjPFeatTabOut featout, char *seqname,int begin){
  AjPFeatLexicon dict=NULL;
  AjPFeatTable feattable;
  AjPStr name=NULL,score=NULL,desc=NULL,source=NULL,type=NULL,tag=NULL,val=NULL;
  AjEFeatStrand strand=AjStrandWatson;
  AjEFeatFrame frame=AjFrameUnknown;
  int i=0,k=0,m=0,iend,istart,new;
  AjPFeature feature;

  ajStrAssC(&name,seqname);

  feattable = ajFeatTabNew(name,dict);
  dict = feattable->Dictionary;

  ajStrAssC(&source,"antigenic");
  ajStrAssC(&type,"misc_feature");

  ajStrAssC(&tag,"note");

  for(i=nhits-1;i>-1;--i)
    {
      k = ajIntGet(hp,i);

      istart = ajIntGet(hpos,k);
      
      iend = istart + ajIntGet(hlen,k)-1;

      score = ajFmtPrintS(&score,"%.3f",ajFloatGet(hwt,k));
      feature = ajFeatureNew(feattable, source, type,
                          istart+begin, iend+begin, score, strand, frame,
                          desc , 0, 0) ;    


      
      new = 0;
      for(m=istart;m<=iend;++m)
	if(ajFloatGet(thisap,m) == ajFloatGet(hwt,k)){  
	  if(!new){
	    val = ajStrNew();
	    val = ajFmtPrintS(&val,"max score at %d",m);
	    ajFeatSetTagValue(feature,tag,val,AJFALSE);
	    new++;
	  }
	  else{
	    val = ajStrNew();
	    val = ajFmtPrintS(&val,",%d",m);
	    ajFeatSetTagValue(feature,tag,val,AJTRUE);	    
	  }
	}
    }
  ajFeatSortByStart(feattable);
  ajFeaturesWrite (featout, feattable);
  ajFeatTabDel(&feattable);
  
}

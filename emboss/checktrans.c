/* @source checktrans application
**
** Check translations made with transeq (document these translations)
**
** @author: Copyright (C) Rodrigo Lopez & Alan Bleasby
** @@
** Adapted from work done by Alan Bleasy
** Modified by Gary Williams 19 April 2000 to remove output to STDOUT and to
**	write ORF sequences to a single file instead of many individual ones.
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

void findorfs( AjPSeqout *outseq, AjPFile *outf, int s, int len,
	char *seq, char *name, int begin, int orfml);

void ajbseq(AjPSeqout *outseq, char *seq, int begin, int
	end, char *name, int count);

void dumptofeat(AjPFeatTabOut featout,int from, int to,
	char *p, char *seqname, int begin, int min_orflength);

int main( int argc, char **argv, char **env)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPSeqout outseq=NULL;
    AjPFeatTabOut featout=NULL;
    
    int begin;
    int end;
    int len;
    int orfml;

    embInit("checktrans",argc,argv);
    seqall    = ajAcdGetSeqall("sequence");
    outf      = ajAcdGetOutfile("report");
    orfml     = ajAcdGetInt("orfml");
    outseq    = ajAcdGetSeqoutall("outseq");
    featout   = ajAcdGetFeatout("featout");

    substr    = ajStrNew();


    while(ajSeqallNext(seqall, &seq)) {
        begin=ajSeqBegin(seq);
        end=ajSeqEnd(seq);

        strand = ajSeqStr(seq);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1); 
        ajStrToUpper(&substr);

        len=ajStrLen(substr);

	ajFmtPrintF(outf,"\n\nCHECKTRANS of %s from %d to %d\n\n",
		      ajSeqName(seq),begin,begin+len-1);

        findorfs(&outseq, &outf,0,len,ajStrStr(substr),ajSeqName(seq),begin,
		 orfml);

	dumptofeat(featout,0,len,ajStrStr(substr),ajSeqName(seq),begin,
		   orfml);
    }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);
    ajSeqWriteClose (outseq);

    ajExit();
    return 0;
}

/* findorfs - finds ORFs and prints report. */

void findorfs (AjPSeqout *outseq, AjPFile *outf, int from, int to,
	char *p, char *name, int begin, int min_orflength)

{
    int i;
    int count = 1;
    int last_stop = 0;
    int orflength = 0;

    ajFmtPrintF(*outf,"\tORF#\tPos\tLen\tORF Range\tSequence name\n\n");

    for (i=from;i<to;++i) {

	if(p[i]=='*') {
	    orflength=i-last_stop;
	    if (orflength >= min_orflength) {

		    
              ajFmtPrintF(*outf,"\t%d\t%d\t%d\t%d-%d\t%s_%d\n", count,
                i+1, orflength, i-orflength+1, i, name,count);
              ajbseq(outseq, p,i-orflength,i-1,name,count);
		    
	    }
	    last_stop = ++i;
	    ++count;
            while (p[i] == '*') {	/* check to see if we have consecutive ****s */
              last_stop = ++i;
              ++count;
            }
	}
    }

	ajFmtPrintF(*outf,"\n\tTotal STOPS: %5d\n\n ",count-1);

}




void ajbseq(AjPSeqout *outseq, char *seq, int begin, int
	end, char *name, int count) {
    AjPSeq sq;
    AjPStr str;
    AjPStr nm;
    
    sq  = ajSeqNew();
    str = ajStrNew();
    nm  = ajStrNew();

    ajStrAssSubC(&str,seq,begin,end);
    ajSeqReplace(sq,str);
    
    ajFmtPrintS(&nm,"%s_%d",name,count);
    ajSeqAssName(sq,nm);
    
    ajSeqWrite(*outseq, sq);

    ajStrDel(&nm);
    ajStrDel(&str);
    ajSeqDel(&sq);
    
    return;
}

void dumptofeat(AjPFeatTabOut featout,int from, int to,
	char *p, char *seqname, int begin, int min_orflength){
  int i;
  int count = 1;
  int last_stop = 0;
  int orflength = 0;
  AjPFeatTable feattable;
  AjPFeatLexicon dict=NULL;
  AjPStr name=NULL,score=NULL,desc=NULL,source=NULL,type=NULL;
  AjEFeatStrand strand=AjStrandWatson;
  AjEFeatFrame frame=AjFrameUnknown;
  AjPFeature feature;
  
  ajStrAssC(&name,seqname);
  
  feattable = ajFeatTabNew(name,dict);
  dict = feattable->Dictionary;

  ajStrAssC(&source,"checktrans");
  ajStrAssC(&type,"misc_feature");


  for (i=from;i<to;++i) {
    
    if(p[i]=='*') {
      orflength=i-last_stop;
      if (orflength >= min_orflength) {
	
	
	feature = ajFeatureNew(feattable, source, type,
			       i-orflength+1,i, score, strand, frame,
                          desc , 0, 0) ;    
	if(!feature)
	  ajDebug("Error adding feature to table");
      }
      last_stop = ++i;
      ++count;
      while (p[i] == '*') {	/* check to see if we have consecutive ****s */
	last_stop = ++i;
	++count;
      }
    }
  }
  ajFeatSortByStart(feattable);
  ajFeaturesWrite (featout, feattable);
  ajFeatTabDel(&feattable);
    
}

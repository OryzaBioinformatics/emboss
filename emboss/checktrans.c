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

static void checktrans_findorfs( AjPSeqout *outseq, AjPFile *outf, ajint s,
				ajint len, char *seq, char *name, ajint begin,
				ajint orfml);

static void checktrans_ajbseq(AjPSeqout *outseq, char *seq, ajint begin,
			      int end, char *name, ajint count);

static void checktrans_dumptofeat(AjPFeattabOut featout, ajint from, ajint to,
				  char *p, char *seqname, ajint begin,
				  ajint min_orflength);


/* @prog checktrans ***********************************************************
**
** Reports STOP codons and ORF statistics of a protein sequence
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq    seq=NULL;
    AjPFile   outf=NULL;
    AjPStr    strand=NULL;
    AjPStr    substr=NULL;
    AjPSeqout outseq=NULL;
    AjPFeattabOut featout=NULL;
    
    ajint begin;
    ajint end;
    ajint len;
    ajint orfml;

    embInit("checktrans",argc,argv);
    seqall    = ajAcdGetSeqall("sequence");
    outf      = ajAcdGetOutfile("report");
    orfml     = ajAcdGetInt("orfml");
    outseq    = ajAcdGetSeqoutall("outseq");
    featout   = ajAcdGetFeatout("featout");

    substr    = ajStrNew();


    while(ajSeqallNext(seqall, &seq))
    {
        begin=ajSeqBegin(seq);
        end=ajSeqEnd(seq);

        strand = ajSeqStr(seq);

	ajStrAssSubC(&substr,ajStrStr(strand),begin-1,end-1); 
        ajStrToUpper(&substr);

        len=ajStrLen(substr);

	ajFmtPrintF(outf,"\n\nCHECKTRANS of %s from %d to %d\n\n",
		    ajSeqName(seq),begin,begin+len-1);

        checktrans_findorfs(&outseq, &outf,0,len,ajStrStr(substr),
			    ajSeqName(seq),begin,orfml);

	checktrans_dumptofeat(featout,0,len,ajStrStr(substr),ajSeqName(seq),
			      begin,orfml);
    }
    
    
    ajSeqDel(&seq);
    ajStrDel(&substr);
    ajFileClose(&outf);
    ajSeqWriteClose (outseq);

    ajExit();
    return 0;
}




/* findorfs - finds ORFs and prints report. */

/* @funcstatic checktrans_findorfs ********************************************
**
** Undocumented.
**
** @param [?] outseq [AjPSeqout*] Undocumented
** @param [?] outf [AjPFile*] Undocumented
** @param [?] from [ajint] Undocumented
** @param [?] to [ajint] Undocumented
** @param [?] p [char*] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] min_orflength [ajint] Undocumented
** @@
******************************************************************************/

static void checktrans_findorfs (AjPSeqout *outseq, AjPFile *outf, ajint from,
				 ajint to, char *p, char *name, ajint begin,
				 ajint min_orflength)

{
    ajint i;
    ajint count = 1;
    ajint last_stop = 0;
    ajint orflength = 0;

    ajFmtPrintF(*outf,"\tORF#\tPos\tLen\tORF Range\tSequence name\n\n");

    for (i=from;i<to;++i)
    {
	if(p[i]=='*')
	{
	    orflength=i-last_stop;
	    if (orflength >= min_orflength)
	    {
		ajFmtPrintF(*outf,"\t%d\t%d\t%d\t%d-%d\t%s_%d\n", count,
			    i+1, orflength, i-orflength+1, i, name,count);
		checktrans_ajbseq(outseq, p,i-orflength,i-1,name,count);
		    
	    }
	    last_stop = ++i;
	    ++count;
            while (p[i] == '*')
	    {	/* check to see if we have consecutive ****s */
		last_stop = ++i;
		++count;
            }
	}
    }

    ajFmtPrintF(*outf,"\n\tTotal STOPS: %5d\n\n ",count-1);

    return;
}




/* @funcstatic checktrans_ajbseq **********************************************
**
** Undocumented.
**
** @param [?] outseq [AjPSeqout*] Undocumented
** @param [?] seq [char*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] end [int] Undocumented
** @param [?] name [char*] Undocumented
** @param [?] count [ajint] Undocumented
** @@
******************************************************************************/



static void checktrans_ajbseq(AjPSeqout *outseq, char *seq, ajint begin, int
			      end, char *name, ajint count)
{
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




/* @funcstatic checktrans_dumptofeat *****************************************
**
** Undocumented.
**
** @param [?] featout [AjPFeattabOut] Undocumented
** @param [?] from [ajint] Undocumented
** @param [?] to [ajint] Undocumented
** @param [?] p [char*] Undocumented
** @param [?] seqname [char*] Undocumented
** @param [?] begin [ajint] Undocumented
** @param [?] min_orflength [ajint] Undocumented
** @@
******************************************************************************/

static void checktrans_dumptofeat(AjPFeattabOut featout, ajint from, ajint to,
				  char *p, char *seqname, ajint begin,
				  ajint min_orflength)
{
    ajint i;
    ajint count = 1;
    ajint last_stop = 0;
    ajint orflength = 0;
    AjPFeattable feattable;
    AjPStr name=NULL;
    AjPStr source=NULL;
    AjPStr type=NULL;
    char strand='+';
    ajint frame=0;
    AjPFeature feature;
    float score = 0.0;
  
    name = ajStrNew();
    source = ajStrNew();
    type = ajStrNew();


    ajStrAssC(&name,seqname);
  
    feattable = ajFeattableNewProt(name);

    ajStrAssC(&source,"checktrans");
    ajStrAssC(&type,"misc_feature");


    for (i=from;i<to;++i)
    {
	if(p[i]=='*')
	{
	    orflength=i-last_stop;
	    if (orflength >= min_orflength)
	    {
		feature = ajFeatNew(feattable, source, type,
				    i-orflength+1,i, score, strand, frame) ;
		if(!feature)
		    ajDebug("Error adding feature to table");
	    }
	    last_stop = ++i;
	    ++count;
	    while (p[i] == '*')
	    {	/* check to see if we have consecutive ****s */
		last_stop = ++i;
		++count;
	    }
	}
    }

    ajFeatSortByStart(feattable);
    ajFeatWrite (featout, feattable);
    ajFeattableDel(&feattable);

    ajStrDel(&name);
    ajStrDel(&source);
    ajStrDel(&type);

    return;
}

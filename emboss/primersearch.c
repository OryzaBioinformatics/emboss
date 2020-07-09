/* @source primersearch application
**
** Searches a set of primer pairs against a set of DNA sequences in both
** forward and reverse sense.
** Modification of fuzznuc.
** @author: Copyright (C) Val Curwen (vcurwen@hgmp.mrc.ac.uk)
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define AJA2 50

/* the internals of a primer; each Primer has two of these,
   one forward and one reverse */
typedef struct primerguts{
  AjPStr patstr;
  AjPStr origpat;
  ajint type;
  ajint len;
  ajint real_len;
  AjBool amino;
  AjBool carboxyl;

  ajint mm;

  ajint* buf;
  ajuint* sotable;
  ajuint solimit;
  EmbOPatBYPNode off[AJALPHA];
  AjPStr re;
  ajint **skipm;
  void* tidy;
}*PGuts;

/* holds details of a hit against a sequence ie this primer will amplify */
typedef struct primerhit{
  AjPStr seqname;
  AjPStr desc;
  AjPStr acc;
  AjPStr forward; /* pattern that hits forward strand */
  AjPStr reverse; /* pattern that hits reverse strand */
  ajint forward_pos;
  ajint reverse_pos;
  ajint amplen;
  ajint forward_mismatch;
  ajint reverse_mismatch;
}*PHit;

/* primer pairs will be read into a list of these structs */
typedef struct primers {
  AjPStr Name;
  PGuts forward;
  PGuts reverse;
  AjPList hitlist;
} *Primer;

/* "constructors" */
static void initialise_pguts(PGuts* primer);

/* "destructors" */
static void free_pguts(PGuts* primer);
static void free_primer(void** x, void* cl);
static void clean_hitlist(AjPList hlist);

/* utilities */
static void read_primers(AjPList* primerList, AjPFile primerFile, ajint mmp);
static AjBool classify_and_compile(Primer* primdata);
static void primer_search(AjPList primerList, AjPSeq seq, AjPFile outf);
static void scan_seq(Primer primdata, AjPSeq seq, AjBool reverse, AjPFile outf);
static void store_hits(Primer primdata, AjPList fhits_list, AjPList rhits_list, AjPSeq seq, AjBool reverse);
static void print_hits(AjPList primerList, AjPFile outf);


int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq = NULL;
    AjPFile primerFile; /* we read the primer pairs from a file */
    AjPFile outf;
    AjPList primerList;

    ajint mmp=0;

    embInit ("primersearch", argc, argv);
    
    seqall   = ajAcdGetSeqall("sequences");
    outf     = ajAcdGetOutfile("out");
    primerFile  = ajAcdGetInfile("primers");
    mmp = ajAcdGetInt("mismatchpercent");

    /* build list of forward/reverse primer pairs as read from primerfile */
    primerList = ajListNew();
    
    /* read in primers from primerfile, classify and compile them */
    read_primers(&primerList,primerFile, mmp);

    /* check there are primers to be searched */
    if(!ajListLength(primerList))
       {
	 ajUser("\nNo suitable primers found - exiting\n");
	 ajExit();
	 return 0;
  
       }
    /* query sequences one by one */
    while(ajSeqallNext(seqall,&seq)) 
      primer_search(primerList, seq, outf); 

    /* output the results */
    print_hits(primerList, outf);
    
    /* delete all nodes of list, then the list itself */
    ajListMap(primerList, free_primer, NULL);
    ajListFree(&primerList);
    ajListDel(&primerList);

    ajFileClose(&outf);

    ajExit();
    return 0;
}

/* "constructors" */
static void initialise_pguts(PGuts* primer)
{
  AJNEW(*primer);
  (*primer)->patstr=NULL;
  (*primer)->origpat=ajStrNew();
  (*primer)->type=0;
  (*primer)->len=0;
  (*primer)->real_len=0;
  (*primer)->re=NULL;
  (*primer)->amino=0;
  (*primer)->carboxyl=0;
  (*primer)->tidy=NULL;
}

/* "destructors" */
/* Frees up all the internal members of a PGuts struct */
static void free_pguts(PGuts* primer)
{
  ajint i=0;
   
  ajStrDel(&(*primer)->patstr);
  ajStrDel(&(*primer)->origpat);
  ajStrDel(&(*primer)->re);

  
  if(((*primer)->type==1 || (*primer)->type==2) && ((*primer)->buf))
    free((*primer)->buf);

  if(((*primer)->type==3 || (*primer)->type==4) && ((*primer)->sotable))
    free((*primer)->sotable);

  if((*primer)->type==6)
    for(i=0;i<(*primer)->real_len;++i) AJFREE((*primer)->skipm[i]);
  AJFREE(*primer);
}

/* frees up the internal members of a Primer */
void free_primer(void **x, void *cl){
  Primer* p;
  Primer primdata;
  AjIList lIter;

  p = (Primer*) x;
  primdata = *p;

  free_pguts(&primdata->forward);
  free_pguts(&primdata->reverse);
  ajStrDel(&primdata->Name);

  /* clean up hitlist */
  lIter = ajListIter(primdata->hitlist);
  while(!ajListIterDone(lIter))
    {
      PHit phit = ajListIterNext(lIter);
      ajStrDel(&phit->forward);
      ajStrDel(&phit->reverse);
      ajStrDel(&phit->seqname);
      ajStrDel(&phit->acc);
      ajStrDel(&phit->desc);
      
      AJFREE(phit);
    }
  
  ajListFree(&primdata->hitlist);
  ajListDel(&primdata->hitlist);
  ajListIterFree(lIter);

  AJFREE(primdata);
}

static void clean_hitlist(AjPList hlist)
{
  AjIList lIter;
  
  lIter = ajListIter(hlist);
  while(!ajListIterDone(lIter))
    {
      EmbPMatMatch fm = ajListIterNext(lIter);
      embMatMatchDel(&fm);
    }
  ajListFree(&hlist);
  ajListDel(&hlist);
  ajListIterFree(lIter);

  return;
}

/* utilities */
/* read primers in from primerfile, classify and compile the patterns */
static void read_primers(AjPList *primerList, AjPFile primerFile, ajint mmp)
{
  AjPStr rdline = NULL;
  AjPStrTok handle = NULL;

  ajint nprimers = 0;
  Primer primdata = NULL;


  while (ajFileReadLine (primerFile, &rdline)) {

    primdata = NULL;
    if (ajStrChar(rdline, 0) == '#')
      continue;
    if (ajStrSuffixC(rdline, ".."))
      continue;
    
    AJNEW(primdata);
    primdata->Name = NULL;

    initialise_pguts(&primdata->forward);
    initialise_pguts(&primdata->reverse);

    primdata->hitlist = ajListNew();

    handle = ajStrTokenInit (rdline, " \t"); 
    ajStrToken (&primdata->Name, &handle, NULL);
    
    ajStrToken (&primdata->forward->patstr, &handle, NULL); 
    ajStrToUpper(&primdata->forward->patstr); 
    ajStrToken (&primdata->reverse->patstr, &handle, NULL);
    ajStrToUpper(&primdata->reverse->patstr);
    ajStrTokenClear (&handle); 

    /* copy patterns for Henry Spencer code */
    ajStrAssC(&primdata->forward->origpat,
	      ajStrStr(primdata->forward->patstr));
    ajStrAssC(&primdata->reverse->origpat, 
	      ajStrStr(primdata->forward->patstr));

    /* set the mismatch level */
    primdata->forward->mm = (ajint) (ajStrLen(primdata->forward->patstr)*mmp)/100;
    primdata->reverse->mm = (ajint) (ajStrLen(primdata->reverse->patstr)*mmp)/100;

    if(classify_and_compile(&primdata))
      {
	ajListPushApp (*primerList, primdata);
	nprimers++;
      }
    else /* there was something funny about the primer sequences */
      {
	printf ("Cannot use %s\n", ajStrStr(primdata->Name));
	free_pguts(&primdata->forward);
	free_pguts(&primdata->reverse);
	ajStrDel(&primdata->Name);
	ajListFree(&primdata->hitlist);
	ajListDel(&primdata->hitlist);
	AJFREE(primdata);
      }
  }

  ajStrDel(&rdline);
  ajFileClose(&primerFile);

}

/* determines pattern type and compiles it */
static AjBool classify_and_compile(Primer* primdata)
{

 /* forward primer */
   if(!((*primdata)->forward->type = 
	 embPatGetType(&((*primdata)->forward->patstr),
		       (*primdata)->forward->mm,0,
		       &((*primdata)->forward->real_len), 
		       &((*primdata)->forward->amino), 
		       &((*primdata)->forward->carboxyl))))
     ajFatal("Illegal pattern");

   /* reverse primer */
   if(!((*primdata)->reverse->type = 
	 embPatGetType(&((*primdata)->reverse->patstr),
		       (*primdata)->reverse->mm,0,
		       &((*primdata)->reverse->real_len), 
		       &((*primdata)->reverse->amino), 
		       &((*primdata)->reverse->carboxyl))))
     ajFatal("Illegal pattern");

   embPatCompile((*primdata)->forward->type, 
		 (*primdata)->forward->patstr, 
		 (*primdata)->forward->origpat, 
		 &((*primdata)->forward->len), 
		 &((*primdata)->forward->buf), 
		 (*primdata)->forward->off, 
		 &((*primdata)->forward->sotable), 
		 &((*primdata)->forward->solimit), 
		 &((*primdata)->forward->real_len), 
		 &((*primdata)->forward->re), 
		 &((*primdata)->forward->skipm),
		 (*primdata)->forward->mm );

   embPatCompile((*primdata)->reverse->type, 
		 (*primdata)->reverse->patstr, 
		 (*primdata)->reverse->origpat, 
		 &((*primdata)->reverse->len), 
		 &((*primdata)->reverse->buf), 
		 (*primdata)->reverse->off, 
		 &((*primdata)->reverse->sotable), 
		 &((*primdata)->reverse->solimit), 
		 &((*primdata)->reverse->real_len), 
		 &((*primdata)->reverse->re), 
		 &((*primdata)->reverse->skipm),
		 (*primdata)->reverse->mm );

  return AJTRUE; /* this is a useable primer */
}


/* tests the primers in primdata against seq and writes results to outfile */
static void primer_search(AjPList primerList, AjPSeq seq, AjPFile outf)
{
  AjIList listIter;
  
  /* test each list node against this sequence */
  listIter = ajListIter(primerList);
  while(!ajListIterDone(listIter))
    {
      Primer curr_primer = ajListIterNext(listIter);
  
       scan_seq(curr_primer, seq, AJFALSE, outf); 
       scan_seq(curr_primer, seq, AJTRUE, outf); 
    }

  ajListIterFree(listIter);

  return;
}


/* scans the primer pairs against the sequence in either forward 
   sense or reverse complemented 
   works out amplimer length if the two primers both hit */
static void scan_seq(Primer primdata, AjPSeq seq, AjBool reverse, AjPFile outf)
{
  AjPStr seqstr = NULL;
  AjPStr revstr = NULL;
  AjPStr seqname = NULL;
  ajint fhits = 0;
  ajint rhits = 0;
  AjPList fhits_list = NULL;
  AjPList rhits_list = NULL;
  
  /* initialise variables for search */
  ajStrAssC(&seqname,ajSeqName(seq));
  ajSeqToUpper(seq);
  ajStrAss(&seqstr, ajSeqStr(seq));
  ajStrAss(&revstr, ajSeqStr(seq));
  ajSeqReverseStr(&revstr);
  fhits_list = ajListNew();
  rhits_list = ajListNew();
  
  if(!reverse)
   {
      /* test OligoA against forward sequence, and OligoB against reverse */
     embPatFuzzSearch(primdata->forward->type, 
		      ajSeqBegin(seq), 
		      primdata->forward->patstr,
		      primdata->forward->origpat, 
		      seqname, 
		      seqstr, 
		      &fhits_list,
		      primdata->forward->len, 
		      primdata->forward->mm, 
		      primdata->forward->amino,
		      primdata->forward->carboxyl, 
		      primdata->forward->buf,
		      primdata->forward->off, 
		      primdata->forward->sotable,
		      primdata->forward->solimit, 
		      primdata->forward->re,
		      primdata->forward->skipm, 
		      &fhits, 
		      primdata->forward->real_len,
		      &(primdata->forward->tidy));
     if(fhits) 
       embPatFuzzSearch(primdata->reverse->type, 
			ajSeqBegin(seq), 
			primdata->reverse->patstr,
			primdata->reverse->origpat, 
			seqname, 
			revstr, 
			&rhits_list,
			primdata->reverse->len, 
			primdata->reverse->mm, 
			primdata->reverse->amino,
			primdata->reverse->carboxyl, 
			primdata->reverse->buf,
			primdata->reverse->off, 
			primdata->reverse->sotable,
			primdata->reverse->solimit, 
			primdata->reverse->re,
			primdata->reverse->skipm, 
			&rhits, 
			primdata->reverse->real_len,
			&(primdata->reverse->tidy));
     
   }
  else
    {
      /*test OligoB against forward sequence, and OligoA against reverse  */
      embPatFuzzSearch(primdata->reverse->type, 
		       ajSeqBegin(seq), 
		       primdata->reverse->patstr,
		       primdata->reverse->origpat, 
		       seqname, 
		       seqstr, 
		       &fhits_list,
		       primdata->reverse->len, 
		       primdata->reverse->mm, 
		       primdata->reverse->amino,
		       primdata->reverse->carboxyl, 
		       primdata->reverse->buf,
		       primdata->reverse->off, 
		       primdata->reverse->sotable,
		       primdata->reverse->solimit, 
		       primdata->reverse->re,
		       primdata->reverse->skipm, 
		       &fhits, 
		       primdata->reverse->real_len,
		       &(primdata->reverse->tidy));
      
      if(fhits) 
	embPatFuzzSearch(primdata->forward->type, 
			 ajSeqBegin(seq), 
			 primdata->forward->patstr,
			 primdata->forward->origpat, 
			 seqname, 
			 revstr, 
			 &rhits_list,
			 primdata->forward->len, 
			 primdata->forward->mm, 
			 primdata->forward->amino,
			 primdata->forward->carboxyl, 
			 primdata->forward->buf,
			 primdata->forward->off, 
			 primdata->forward->sotable,
			 primdata->forward->solimit, 
			 primdata->forward->re,
			 primdata->forward->skipm, 
			 &rhits, 
			 primdata->forward->real_len,
			 &(primdata->forward->tidy));
    }
  
  if(fhits && rhits)
      /* get amplimer length(s) and write out the hit */
      store_hits(primdata, fhits_list, rhits_list, seq, reverse);

  /* tidy up */
  clean_hitlist(fhits_list);
  clean_hitlist(rhits_list);
  
  ajStrDel(&seqstr);
  ajStrDel(&revstr);
  ajStrDel(&seqname);
  
  return;
}

static void store_hits(Primer primdata, AjPList fhits, AjPList rhits, AjPSeq seq, AjBool reverse)
{
  ajint amplen = 0;
  AjIList fi;
  AjIList ri;

  PHit primerhit = NULL;

  fi = ajListIter(fhits);
  while(!ajListIterDone(fi))
    {
      EmbPMatMatch fm = NULL;
      EmbPMatMatch rm = NULL;
      amplen = 0;

      fm = ajListIterNext(fi);
      ri = ajListIter(rhits);
      while(!ajListIterDone(ri))
	{
	  ajint seqlen = ajSeqLen(seq);
	  ajint s = (fm->start);
	  ajint e;

	  rm = ajListIterNext(ri);
	  e = (rm->start-1); 
	  amplen = seqlen-(s-1)-e;
	  if (amplen > 0) /* no point making a hit if -ve length! */
	    {
	      primerhit = NULL;
	      AJNEW(primerhit);
	      primerhit->desc=NULL; /* strings must be NULL for ajStrAss */
	      primerhit->seqname=NULL; /* strings must be NULL for ajStrAss */
	      primerhit->acc=NULL;
	      primerhit->forward=NULL;
	      primerhit->reverse=NULL;
	      ajStrAssC(&primerhit->seqname,ajSeqName(seq));
	      ajStrAss(&primerhit->desc, ajSeqGetDesc(seq));
	      ajStrAss(&primerhit->acc, ajSeqGetAcc(seq));
	      primerhit->forward_pos = fm->start;
	      primerhit->reverse_pos = rm->start;
	      primerhit->forward_mismatch = fm->mm;
	      primerhit->reverse_mismatch = rm->mm;
	      primerhit->amplen = amplen;
	      if(!reverse)
		{
		  ajStrAss(&primerhit->forward, primdata->forward->patstr);
		  ajStrAss(&primerhit->reverse, primdata->reverse->patstr);
		}
	      else
		{
		  ajStrAss(&primerhit->forward, primdata->reverse->patstr);
		  ajStrAss(&primerhit->reverse, primdata->forward->patstr);
		}
	      ajListPushApp(primdata->hitlist, primerhit);


	    }
	}
      /* clean up rListIter here as it will be new'ed again next time through */
      ajListIterFree(ri);
    }
  
  ajListIterFree(fi);
  return;
}


static void print_hits(AjPList primerList, AjPFile outf)
{
  /* iterate through list of hits */
  AjIList lIter;

  ajint count = 1;
  lIter = ajListIter(primerList);
  while(!ajListIterDone(lIter))
  {
    Primer primer = ajListIterNext(lIter);
    AjIList hIter = ajListIter(primer->hitlist);
    count = 1;
    
    ajFmtPrintF(outf, "\nPrimer name %s\n", ajStrStr(primer->Name));
    
    while(!ajListIterDone(hIter))
    {  
      PHit hit = ajListIterNext(hIter);
      ajFmtPrintF(outf, "Amplimer %d\n", count);
      ajFmtPrintF(outf, "\tSequence: %s %s \n\t%s\n", ajStrStr(hit->seqname), 
		  ajStrStr(hit->acc), ajStrStr(hit->desc));
      ajFmtPrintF(outf, "\t%s hits forward strand at %d with %d mismatches\n", 
		  ajStrStr(hit->forward), hit->forward_pos,
		  hit->forward_mismatch);
      ajFmtPrintF(outf, "\t%s hits reverse strand at [%d] with %d mismatches\n", 
		  ajStrStr(hit->reverse), (hit->reverse_pos), 
		  (hit->reverse_mismatch));
      ajFmtPrintF(outf, "\tAmplimer length: %d bp\n", hit->amplen);
      count++;
    }
    ajListIterFree(hIter);
  }
  ajListIterFree(lIter);
}


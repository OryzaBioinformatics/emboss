/* @source vectorstrip application
**
** Looks for user defined linkers in a sequence and outputs
** only the sequence that lies between those linkers.
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
#include "stdlib.h"

typedef struct clip_pattern{
  AjPStr patstr;
  AjPStr origpat;
  int type;
  int len;
  int real_len;
  AjBool amino;
  AjBool carboxyl;

  int* buf;
  unsigned int* sotable;
  unsigned int solimit;
  EmbOPatBYPNode off[AJALPHA];
  AjPStr re;
  int **skipm;
  void* tidy;
}*CPattern;

typedef struct vector{
  AjPStr name;
  AjPStr fiveprime;
  AjPStr threeprime;
}*Vector;

/* constructors */
static void initialise_cp(CPattern* pat);
static void initialise_vector(Vector* vec, AjPStr name, AjPStr five, 
			      AjPStr three);
static void read_vector_data(AjPFile vectorfile, AjPList* vectorlist);

/* destructors */
static void free_list(AjPList list);
static void free_cp(CPattern* pat);
static void free_vector(Vector* vec);


/* data processing */
static void process_pattern(AjPStr pat, AjPList* hitlist, 
			    AjPStr seqname, AjPStr seqstr, 
			    int threshold, int begin, AjBool besthits);
static void process_hits(AjPList fivelist, AjPList threelist, 
			 AjPSeq sequence, AjPSeqout seqout, AjPFile outf);
static void scan_sequence(Vector vector, AjPSeqout seqout, AjPFile outf, 
			  AjPSeq sequence, int mis_per, AjBool besthits);
static void ccs_pattern(AjPStr pattern, AjPList* hitlist, AjPStr seqname, 
			AjPStr seqstr, int begin, int* hits, int mm);
/* result output */
static void write_sequence(AjPSeq sequence, AjPSeqout seqout, 
			   int start, int end, AjPFile outf);
static void print_hits(AjPList l, AjPFile outf, AjPStr seq, int begin);
static void reportseq(AjPStr seqstr, AjPFile outf);

int main (int argc, char * argv[])
{
  /* sequence related */
  AjPSeqall seqall;
  AjPSeq seq;
  AjPSeqout seqout;
  AjPFile outf;
  int threshold;
  AjBool vec = AJFALSE;
  AjPFile vectorfile;
  AjPList vectorlist = NULL;
  AjBool besthits = AJTRUE;

  /* pattern related */
  AjPStr fiveprime=NULL;
  AjPStr threeprime=NULL;
  
  /* get values for parameters */
  embInit ("vectorstrip", argc, argv);
  
  seqall = ajAcdGetSeqall("sequence");
  outf = ajAcdGetOutfile("outf");
  seqout = ajAcdGetSeqoutall ("outseq");
  vec = ajAcdGetBool("vectorfile");
  besthits = ajAcdGetBool("besthits");
  vectorlist = ajListNew();

  /* data from command line or file? */
  if(vec == AJTRUE)
    {
      vectorfile = ajAcdGetInfile("vectors");
      read_vector_data(vectorfile, &vectorlist);
    }
  else
    {
      Vector v=NULL;
      AjPStr name = NULL;
      name = ajStrNewC("no_name");

      fiveprime = ajAcdGetString("linkerA");
      threeprime = ajAcdGetString("linkerB");
      initialise_vector(&v, name, fiveprime, threeprime);
      ajListPushApp (vectorlist, v);
      ajStrDel(&name);
    }

  threshold = ajAcdGetInt("mismatch");
  
  /* check there are vectors to be searched */
  if(!ajListLength(vectorlist))
    {
      ajUser("\nNo suitable vectors found - exiting\n");
      ajExit();
      return 0;
    }

  /* search each sequence for the vectors */
  while(ajSeqallNext(seqall,&seq))
    {
      AjIList iter=ajListIter(vectorlist);
      while(!ajListIterDone(iter))
	{
	  Vector vec = ajListIterNext(iter);
	  scan_sequence(vec, seqout, outf, seq, threshold, besthits);
	}
      ajListIterFree(iter);
    }

  /* clearing up */
  ajSeqWriteClose (seqout);

  ajStrDel(&fiveprime);
  ajStrDel(&threeprime);

  ajFileClose(&outf);
  ajExit();
  return 0;
}

/* "constructors" */

/* @funcstatic initialise_cp **************************************************
**
** Initialises data members of a CPattern. 
**
** Parameters:
**
** CPattern* pat - CPattern to be initialised
**
** Parameters modified:
** data members of pat are initialised
**
** Returns:
** void
******************************************************************************/
static void initialise_cp(CPattern* pat)
{
  AJNEW(*pat);
  (*pat)->patstr=NULL;
  (*pat)->origpat=ajStrNew();
  (*pat)->type=0;
  (*pat)->len=0;
  (*pat)->real_len=0;
  (*pat)->re=NULL;
  (*pat)->amino=0;
  (*pat)->carboxyl=0;
}

/* @funcstatic initialise_vector **********************************************
**
** Initialises data members of a Vector. 
**
** Parameters:
**
** Vector* vec - Vector to be initialised
** AjPStr name - string representing name of vector
** AjPStr five - string representing 5' pattern
** AjPStr three - string representing 3' pattern
**
** Parameters modified:
** data members of vec are initialised
**
** Returns:
** void
******************************************************************************/
static void initialise_vector(Vector* vec, AjPStr name, 
			      AjPStr five, AjPStr three)
{
  AJNEW(*vec);
  (*vec)->name=ajStrNewS(name);
  (*vec)->fiveprime=ajStrNewS(five);
  (*vec)->threeprime=ajStrNewS(three);
}

/* @funcstatic read_vector_data ***********************************************
**
** REads vector data from a file into a list of Vectors. 
**
** Parameters:
**
** AjPFile vectorfile - the file containing vector data
** AjPList* vectorlist - list to store vector data
**
** Parameters modified:
** list contains one node for each set of vector data in vectorfile
**
** Returns:
** void
******************************************************************************/
static void read_vector_data(AjPFile vectorfile, AjPList* vectorlist)
{
  AjPStr rdline = NULL;
  AjPStrTok handle = NULL;

  Vector vector = NULL;

  while (ajFileReadLine (vectorfile, &rdline)) 
    {
      AjPStr name=NULL;
      AjPStr five=NULL;
      AjPStr three=NULL;
      vector = NULL;
      
      if (ajStrChar(rdline, 0) == '#')
	continue;
      if (ajStrSuffixC(rdline, ".."))
	continue;
      
      handle = ajStrTokenInit (rdline, " \t"); 
      ajStrToken (&name, &handle, NULL);
      
      ajStrToken (&five, &handle, NULL); 
      ajStrToUpper(&five); 
      ajStrToken (&three, &handle, NULL);
      ajStrToUpper(&three);
      ajStrTokenClear (&handle); 
      
      if(ajStrLen(five) || ajStrLen(three))
	 {
	   initialise_vector(&vector, name, five, three);
	   ajListPushApp (*vectorlist, vector);
	 }
      ajStrDel(&name);
      ajStrDel(&five);
      ajStrDel(&three);
    }
  ajStrDel(&rdline);
  ajFileClose(&vectorfile);
}
 
/* "destructors" */
/* @funcstatic free_list **************************************************
**
** Frees a list of EmbPMatMatch. 
**
** Parameters:
**
** AjPList list - the list of EmbPMatMatch to be free'd
**
** Parameters modified:
** elements of list, and list itself, are free'd
**
** Returns:
** void
******************************************************************************/
static void free_list(AjPList list)
{
  AjIList iter;
  iter = ajListIter(list);
  while(!ajListIterDone(iter))
    {
      EmbPMatMatch fm = ajListIterNext(iter);
      embMatMatchDel(&fm);
    }
  ajListFree(&list);
  ajListDel(&list);
  ajListIterFree(iter);

  return;  
}

/* @funcstatic free_cp **************************************************
**
** Frees a CPattern. 
**
** Parameters:
**
** CPattern* pat - the pattern to be to be free'd
**
** Parameters modified:
** pat is free'd
**
** Returns:
** void
******************************************************************************/
static void free_cp(CPattern* pat)
{
  int i=0;

  ajStrDel(&(*pat)->patstr);
  ajStrDel(&(*pat)->origpat);
  ajStrDel(&(*pat)->re);

  if(((*pat)->type==1 || (*pat)->type==2) && ((*pat)->buf))
    free((*pat)->buf);

  if(((*pat)->type==3 || (*pat)->type==4) && ((*pat)->sotable))
    free((*pat)->sotable);

  if((*pat)->type==6)
    for(i=0;i<(*pat)->real_len;++i) AJFREE((*pat)->skipm[i]); 

  AJFREE(*pat);
}

/* @funcstatic free_vector **************************************************
**
** Frees a Vector. 
**
** Parameters:
**
** Vector* vec - the Vector to be to be free'd
**
** Parameters modified:
** vec is free'd
**
** Returns:
** void
******************************************************************************/
static void free_vector(Vector* vec)
{
  ajStrDel(&(*vec)->name);
  ajStrDel(&(*vec)->fiveprime);
  ajStrDel(&(*vec)->threeprime);
  
  AJFREE(*vec);
}

/* data processing */
/* @funcstatic process_pattern **********************************************
**
** searches pattern against sequence starting with no mismatches
** then increases allowed mismatches until a) hits are found 
** or b) the number of mismatches >= threshold% of the pattern length
**
** The pattern may be repeatedly recompiled using embPatCompile in order to
** allow searching with different numbers of mismatches.
**
** Parameters:
**
** AjPStr pattern - the pattern to be searched
** AjPList* hitlist - list to which hits will be written
** AjPStr seqname - the name of the sequence
** AjPStr seqstr - string representing the sequence to be searched
** int threshold - max allowable % mismatch based on length of pattern
** int begin - start position of sequence
** AjBool besthits - 
**
** Parameters modified:
** list has been populated with hits of pattern against sequence
**
** Returns:
** void
******************************************************************************/
static void process_pattern(AjPStr pattern, AjPList* hitlist, 
			    AjPStr seqname, AjPStr seqstr, 
			    int threshold, int begin, AjBool besthits)
{
  int mm = 0;
  int max_mm = 1;
  int hits = 0;
  
  /* calculate max allowed mismatches based on threshold */
  if(threshold)
    max_mm = (int) (ajStrLen(pattern) * threshold)/100;
  else max_mm = 0;

  if(!besthits)
    /* report all hits */
    ccs_pattern(pattern, hitlist, seqname, seqstr, begin, &hits, max_mm);
  else
    /* start with mm=0, keep going till we get a hit, then STOP */
    while((!hits) && (mm <= max_mm)) 
      {
	ccs_pattern(pattern, hitlist, seqname, seqstr, begin, &hits, mm);
	mm++;
      }
}

/* @funcstatic process_hits **************************************************
**
** Output the hits of the patterns against a sequence; write out the 
** relevant trimmed subsequences
** 
** Parameters:
**
** AjPList fivelist - list of EmbPMatMatch representing hits of 5' 
** pattern against the sequence
** AjPList threelist - list of EmbPMatMatch representing hits of 3'
** pattern against the sequence
** AjPSeq sequence - the sequence itself
** AjPSeqout seqout - place to writesubsequences
** AjPFile outf - file for writing information about the hits
** 
** Parameters modified:
** results written to outf and seqout
**
** Returns:
** void
******************************************************************************/
static void process_hits(AjPList fivelist, AjPList threelist, 
			 AjPSeq sequence, AjPSeqout seqout, 
			 AjPFile outf)
{
  int i=0;
  int j=0;
  int type = 0;

  AjPInt five = ajIntNew(); /* start positions for hits with 5' pattern */
  AjPInt three = ajIntNew(); /* start positions for hits with 3' pattern */

  EmbPMatMatch m;
  AjIList iter;

  iter = ajListIter(fivelist);

  /* populate five and three with start positions */
  while(!ajListIterDone(iter))
    {
      m = ajListIterNext(iter);
      ajIntPut(&five, i, ((m->len) + (m->start)));
      i++;
    }
  ajListIterFree(iter);

  iter = ajListIter(threelist);
  while(!ajListIterDone(iter))
    {
      m = ajListIterNext(iter);
      ajIntPut(&three, j, (m->start -1));
      j++;
    }
  ajListIterFree(iter);

  /* classify the hits */
  if((ajListLength(fivelist) ==1 && ajListLength(threelist) == 1) 
	  && ((ajIntGet(three, 0) + m->len + 1) == ajIntGet(five, 0)))
    /* the patterns are identical and only match once in the sequence */
    type = 1;
  else if(ajIntLen(five) && ajIntLen(three))
    /* both patterns have hit */
    type = 2;
  else if(ajIntLen(five)) 
    /* five but not three */
    type = 3;
  else if(ajIntLen(three))
    /* three but not five */
    type = 4;
  
  /* write out subsequences */
  switch(type)
    {
    case 1:
      ajUser("5' and 3' sequence matches are identical; inconclusive\n");
      break;
      
    case 2:
      /* generally, every 5' hit will be matched against every 3' hit to
	 produce subsequences. Special case: 3' pattern matches upstream
	 of 5' pattern - to be consistent, this will cause everything 
	 from the start of the sequence to the 3' hit to be written out, 
	 and also everything from the 5' hit to the end of the sequence.
	 It's a bit back to front ... */
      for(i=0; i<ajIntLen(five); i++)
	{
	  int hit = 0;
	  for(j=0; j<ajIntLen(three); j++)
	    {
	      if(ajIntGet(five,i) <= ajIntGet(three,j))
		{
		  hit = 1;
		  write_sequence(sequence, seqout, ajIntGet(five,i), 
				 ajIntGet(three,j), outf); 
		}
	    }
	  if(!hit)
	    {
	      write_sequence(sequence, seqout, ajIntGet(five, i), 
			     ajSeqEnd(sequence),outf);
	    }
	}

      for(i=0; i<ajIntLen(three); i++)
	{
	  int hit = 0;
	  for(j=0; j<ajIntLen(five); j++)
	    if(ajIntGet(three,i) >= ajIntGet(five,j))
	      hit=1;
	  if(!hit)
	    {
	      write_sequence(sequence, seqout, ajSeqBegin(sequence), 
			     ajIntGet(three,i),outf);
	    }
	}
      break;
      
    case 3:
      for(i=0; i<ajIntLen(five); i++)
	write_sequence(sequence, seqout, ajIntGet(five, i), 
		       ajSeqEnd(sequence), outf);
      break;
    case 4:
      for(j=0; j<ajIntLen(three); j++)
	write_sequence(sequence, seqout, ajSeqBegin(sequence), 
		       ajIntGet(three, j), outf);
      break;

    default:
      break;
    }
  
  /* tidy up */
  ajIntDel(&five);
  ajIntDel(&three);
}

/* @funcstatic scan_sequence **************************************************
**
** Scans a Vector against a sequence
**
** Parameters:
**
** Vector vector - the vector data
** AjPSeqout seqout - where toi write out subsequences
** AjPFile outf - file for writing results details
** AjPSeq sequence - the sequence to be scanned
** int mis_per - max mistmatch percentage
** AjBool besthits - stop scanning when we get hits, even if mis_per 
**                   not reached yet?
**
** Parameters modified:
** subsequences are written to seqout, and hit details to outf
**
** Returns:
** void
******************************************************************************/
static void scan_sequence(Vector vector, AjPSeqout seqout, AjPFile outf, 
			  AjPSeq sequence, int mis_per, AjBool besthits)
{
  int i = 0;
  int start = 0;
  int begin = 0;
  int end = 0;
  
  /* set up seq related vars */
  AjPStr seqname=ajStrNew();
  AjPStr text=NULL;

  /* need new hitlists for each pattern for each sequence */
  AjPList fivelist=ajListNew();
  AjPList threelist=ajListNew();
    
  ajStrAssC(&seqname,ajSeqName(sequence));
  begin = ajSeqBegin(sequence);
  end   = ajSeqEnd(sequence);
  ajStrAssSubC(&text,ajStrStr(ajSeqStr(sequence)),begin-1,end-1);
  ajStrToUpper(&text);
  
  if(ajStrLen(vector->fiveprime))
    process_pattern(vector->fiveprime, &fivelist, seqname, text, 
		    mis_per, begin, besthits);

  if(ajStrLen(vector->threeprime))
    process_pattern(vector->threeprime, &threelist, seqname, text, 
		    mis_per, begin, besthits);

  if(!(ajListLength(fivelist) || ajListLength(threelist)))
     {
       ajFmtPrintF(outf, "\nSequence: %s \t Vector: %s\tNo match\n", 
	      ajStrStr(seqname), ajStrStr(vector->name));
     }
  else
    {
      ajFmtPrintF(outf, "\n\nSequence: %s \t Vector: %s\n", 
		  ajStrStr(seqname), ajStrStr(vector->name));
      ajFmtPrintF(outf, "5' sequence matches:\n");
      print_hits(fivelist,outf,text,begin);
      ajFmtPrintF(outf, "3' sequence matches:\n");
      print_hits(threelist,outf,text,begin);
    
      ajFmtPrintF(outf, "Sequences output to file:\n");
      process_hits(fivelist, threelist, sequence, seqout, outf);
    }
  /* tidy up */
  free_list(fivelist);
  free_list(threelist);
  ajStrDel(&seqname);
  ajStrDel(&text);
}

/* @funcstatic ccs_pattern **************************************************
**
** Classifies, compiles and searches pattern against seqstr with 
** mm mismatches
**
** Parameters:
**
** AjPStr pattern - patrern to be searched for
** AjPList* hitlist - list of hits
** AjPStr seqname - name of sequence to be searched
** AjPStr seqstr - string representing sequence to be searched
** int begin - start position of sequence
** int* hits - number of hits
** int mm - number of mismatches
**
** Parameters modified:
** hits and hitlist hold the number of hits and the hits 
** themselves, respectively
**
** Returns:
** void
******************************************************************************/
static void ccs_pattern(AjPStr pattern, AjPList* hitlist, AjPStr seqname, 
			AjPStr seqstr, int begin, int* hits, int mm)
{
  int i=0;
  /* set up CPattern */
  CPattern cpat = NULL;
  initialise_cp(&cpat);
  ajStrAssC(&(cpat->patstr), ajStrStr(pattern));
  /* copy the original pattern for Henry Spencer code */
  ajStrAssC(&(cpat->origpat), ajStrStr(pattern));
  
  if(!(cpat->type = embPatGetType(&(cpat->patstr),mm,0,
				  &(cpat->real_len), 
				  &(cpat->amino), 
				  &(cpat->carboxyl))))
    {    
      ajWarn("Illegal pattern: %s", ajStrStr(cpat->patstr));
      free_cp(&cpat);
      return;
    }
  
  embPatCompile(cpat->type, cpat->patstr, 
		cpat->origpat, &(cpat->len), 
		&(cpat->buf), cpat->off, 
		&(cpat->sotable), &(cpat->solimit), 
		&(cpat->real_len), &(cpat->re), 
		&(cpat->skipm),mm );
  
  embPatFuzzSearch(cpat->type, begin, cpat->patstr,
		   cpat->origpat, seqname, seqstr, hitlist,
		   cpat->len, mm, cpat->amino,
		   cpat->carboxyl, cpat->buf,
		   cpat->off, cpat->sotable,
		   cpat->solimit, cpat->re,
		   cpat->skipm, hits, cpat->real_len,
		   &(cpat->tidy));
  free_cp(&cpat);
  
}

/* result output */


/* @funcstatic write_sequence *************************************************
**
** Writes out a subsequence.
**
** Parameters:
**
** AjPSeq sequence - the entire sequence
** AjPSeqout seqout - where to write out subsequences
** int start - start position of desired subsequence relative to sequence
** int end - end position of desired subsequence relative to sequence 
** AjPFile outf - file to write details of output subsequence
**
** Parameters modified:
** subsequence has been written to seqout and details of the output 
** sequence (hit positions, number of mismatches, sequences trimmed 
** from 5' and 3' ends) have been written to outf
**
** Returns:
** void
******************************************************************************/
static void write_sequence(AjPSeq sequence, AjPSeqout seqout, int start, int end, AjPFile outf)
{
  AjPStr name = NULL;
  AjPStr num = NULL;
  char *p;
  int pos;
  int len;

  /* copy the sequence */
  AjPSeq seqcp = NULL;
  AjPStr fivetrim=NULL;
  AjPStr threetrim=NULL;
  AjPStr outs = NULL;

  seqcp = ajSeqNewS(sequence);
  name = ajStrDup(ajSeqGetName(seqcp));
  num = ajStrNew();

  if (start <= end)
    {
      ajSeqSetRange(seqcp, start, end);
      
      ajStrAppC(&name, "_from_");
      ajStrFromInt(&num, start);
      ajStrApp(&name,num);
      ajStrAppC(&name, "_to_");
      ajStrFromInt(&num, end);
      ajStrApp(&name, num);
      
      ajSeqAssName(seqcp, name);
      ajSeqAllWrite (seqout, seqcp);

      /* report the hit to outf */
      ajFmtPrintF(outf, "\tfrom %d to %d\n", start, end);
      ajStrAssSub(&outs, ajSeqStr(seqcp), start-1, end-1);
      reportseq(outs, outf);
      if(start !=1)
	{
	  ajStrAssSub(&fivetrim, ajSeqStr(seqcp), 0, start-2); 
	  ajFmtPrintF(outf, "\tsequence trimmed from 5' end:\n");
	  reportseq(fivetrim, outf);
	}
      if(end!=ajSeqLen(seqcp))
	{
	  ajStrAssSub(&threetrim, ajSeqStr(seqcp), end, ajSeqLen(seqcp));
	  (void) ajFmtPrintF(outf, "\tsequence trimmed from 3' end:\n");
	  (void) reportseq(threetrim, outf);
	}
      ajFmtPrintF(outf, "\n");
    }
  
  /* clean up */
  ajSeqDel(&seqcp);
  ajStrDel(&fivetrim);
  ajStrDel(&threetrim);
  ajStrDel(&outs);
  ajStrDel(&name);
  ajStrDel(&num);
  
}

/* @funcstatic print_hits **************************************************
**
** Output hit positions and number of mismatches of a pattern against a 
** sequence to file 
**
** Parameters:
**
** AjPList hitlist - list of hits of pattern against the sequence
** AjPFile outf - file for writing information about the hits
** AjPStr seq - string representation of sequence
** AjPSeqout seqout - place to write sequence
** int begin - start of sequence
** 
** Parameters modified:
** results written to outf
**
** Returns:
** void
******************************************************************************/
static void print_hits(AjPList hitlist, AjPFile outf, AjPStr seq, int begin)
{
    EmbPMatMatch m;
    AjPStr s;
    AjIList iter;

    s=ajStrNew();

    ajListReverse(hitlist);
    iter = ajListIter(hitlist);

    while(!ajListIterDone(iter))
      {
	m = ajListIterNext(iter); 
	ajStrAssSubC(&s,ajStrStr(seq),m->start-begin,m->end-begin);
	
	ajFmtPrintF(outf,"\tFrom %d to %d with %d mismatches\n",m->start,m->end, m->mm);
	
      }
    
    ajListIterFree(iter);
    ajStrDel(&s);
    
    return;
}

/* @funcstatic reportseq **************************************************
**
** Formatted output of sequence data. 
**
** Parameters:
**
** AjPStr seqstr - sequence to be output
** AjPFile outf - file to write to
**
** Parameters modified:
** sequence is written to outf in lines of length 50.
**
** Returns:
** void
******************************************************************************/
static void reportseq(AjPStr seqstr, AjPFile outf)
{
  AjPStr tmp = NULL;
  int x=0;
  int linelen = 50;
  
  for(x=0; x<ajStrLen(seqstr); x+= linelen)
    {
      (void) ajStrAssSub(&tmp, seqstr, x, x+linelen-1);
      ajFmtPrintF(outf, "\t\t%S\n", tmp);
    }
  ajStrDel(&tmp);
}


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

typedef struct clip_pattern
{
  AjPStr patstr;
  AjPStr origpat;
  ajint type;
  ajint len;
  ajint real_len;
  AjBool amino;
  AjBool carboxyl;

  ajint* buf;
  ajuint* sotable;
  ajuint solimit;
  EmbOPatBYPNode off[AJALPHA];
  AjPStr re;
  ajint **skipm;
  void* tidy;
}*CPattern;

typedef struct vector
{
  AjPStr name;
  AjPStr fiveprime;
  AjPStr threeprime;
}*Vector;

/* constructors */
static void vectorstrip_initialise_cp(CPattern* pat);
static void vectorstrip_initialise_vector(Vector* vec, AjPStr name,
					  AjPStr five, AjPStr three);
static void vectorstrip_read_vector_data(AjPFile vectorfile,
					 AjPList* vectorlist);

/* destructors */
static void vectorstrip_free_list(AjPList list);
static void vectorstrip_free_cp(CPattern* pat);
/*static void vectorstrip_free_vector(Vector* vec);*/


/* data processing */
static void vectorstrip_process_pattern(AjPStr pat, AjPList* hitlist, 
					AjPStr seqname, AjPStr seqstr, 
					ajint threshold, ajint begin,
					AjBool besthits);
static void vectorstrip_process_hits(AjPList fivelist, AjPList threelist, 
				     AjPSeq sequence, AjPSeqout seqout,
				     AjPFile outf);
static void vectorstrip_scan_sequence(Vector vector, AjPSeqout seqout,
				      AjPFile outf, AjPSeq sequence,
				      ajint mis_per, AjBool besthits);
static void vectorstrip_ccs_pattern(AjPStr pattern, AjPList* hitlist,
				    AjPStr seqname, AjPStr seqstr,
				    ajint begin, ajint* hits, ajint mm);
/* result output */
static void vectorstrip_write_sequence(AjPSeq sequence, AjPSeqout seqout, 
				       ajint start, ajint end, AjPFile outf);
static void vectorstrip_print_hits(AjPList l, AjPFile outf, AjPStr seq,
				   ajint begin);
static void vectorstrip_reportseq(AjPStr seqstr, AjPFile outf);


/* @prog vectorstrip **********************************************************
**
** Strips out DNA between a pair of vector sequences
**
******************************************************************************/

int main(int argc, char **argv)
{
    /* sequence related */
    AjPSeqall seqall;
    AjPSeq seq;
    AjPSeqout seqout;
    AjPFile outf;
    ajint threshold;
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
	vectorstrip_read_vector_data(vectorfile, &vectorlist);
    }
    else
    {
	Vector v=NULL;
	AjPStr name = NULL;
	name = ajStrNewC("no_name");

	fiveprime = ajAcdGetString("linkerA");
	threeprime = ajAcdGetString("linkerB");
	vectorstrip_initialise_vector(&v, name, fiveprime, threeprime);
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
	    vectorstrip_scan_sequence(vec, seqout, outf, seq, threshold,
				      besthits);
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

/* @funcstatic vectorstrip_initialise_cp *************************************
**
** Initialises data members of a CPattern. 
**
** @param [w] pat [CPattern*] CPattern to be initialised
** @return [void]
******************************************************************************/

static void vectorstrip_initialise_cp(CPattern* pat)
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

    (*pat)->buf=NULL;
    (*pat)->sotable=NULL;
    (*pat)->solimit=0;
    (*pat)->skipm=NULL;
    (*pat)->tidy=NULL;

    return;
}

/* @funcstatic vectorstrip_initialise_vector **********************************
**
** Initialises data members of a Vector. 
**
** @param [w] vec [Vector*] New vector object
** @param [r] name [AjPStr] string representing name of vector
** @param [r] five [AjPStr] string representing 5' pattern
** @param [r] three [AjPStr] string representing 3' pattern
** @return [void]
******************************************************************************/
static void vectorstrip_initialise_vector(Vector* vec, AjPStr name, 
					  AjPStr five, AjPStr three)
{
    AJNEW(*vec);
    (*vec)->name=ajStrNewS(name);
    (*vec)->fiveprime=ajStrNewS(five);
    (*vec)->threeprime=ajStrNewS(three);

    return;
}

/* @funcstatic vectorstrip_read_vector_data ***********************************
**
** Reads vector data from a file into a list of Vectors. 
**
** @param [r] vectorfile [AjPFile] the file containing vector data
** @param [w] vectorlist [AjPList*] list to store vector data
**                                  contains one node for each set of
**                                  vector data in vectorfile
** @return [void]
******************************************************************************/

static void vectorstrip_read_vector_data(AjPFile vectorfile,
					 AjPList* vectorlist)
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
	    vectorstrip_initialise_vector(&vector, name, five, three);
	    ajListPushApp (*vectorlist, vector);
	}
	ajStrDel(&name);
	ajStrDel(&five);
	ajStrDel(&three);
    }

    ajStrDel(&rdline);
    ajFileClose(&vectorfile);

    return;
}
 
/* "destructors" */

/* @funcstatic vectorstrip_free_list *****************************************
**
** Frees a list of EmbPMatMatch. 
**
** @param [d] list [AjPList] the list of EmbPMatMatch to be freed
** @return [void]
******************************************************************************/

static void vectorstrip_free_list(AjPList list)
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

/* @funcstatic vectorstrip_free_cp ********************************************
**
** Frees a CPattern. 
**
** @param [d] pat [CPattern*] the pattern to be to be freed
** @return [void]
******************************************************************************/

static void vectorstrip_free_cp(CPattern* pat)
{
    ajint i=0;

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

    return;
}

/* #funcstatic vectorstrip_free_vector ***************************************
**
** Frees a Vector. 
**
** #param [d] vec [Vector*] the vector to be freed
** #return [void]
******************************************************************************/
/*static void vectorstrip_free_vector(Vector* vec)
{
  ajStrDel(&(*vec)->name);
  ajStrDel(&(*vec)->fiveprime);
  ajStrDel(&(*vec)->threeprime);
  
  AJFREE(*vec);
  return;
}*/

/* data processing */

/* @funcstatic vectorstrip_process_pattern ***********************************
**
** searches pattern against sequence starting with no mismatches
** then increases allowed mismatches until a) hits are found 
** or b) the number of mismatches >= threshold% of the pattern length
**
** The pattern may be repeatedly recompiled using embPatCompile in order to
** allow searching with different numbers of mismatches.
**
** @param [r] pattern [AjPStr] the pattern to be searched
** @param [w] hitlist [AjPList*] list to which hits will be written
** @param [r] seqname [AjPStr] the name of the sequence
** @param [r] seqstr [AjPStr] string representing the sequence to be searched
** @param [r] threshold [ajint] max allowable percent mismatch based on length
**                              of pattern
** @param [r] begin [ajint] start position of sequence
** @param [r] besthits [AjBool] Best hits
** @return [void]
******************************************************************************/
static void vectorstrip_process_pattern(AjPStr pattern, AjPList* hitlist, 
					AjPStr seqname, AjPStr seqstr, 
					ajint threshold, ajint begin,
					AjBool besthits)
{
    ajint mm = 0;
    ajint max_mm = 1;
    ajint hits = 0;
  
    /* calculate max allowed mismatches based on threshold */
    if(threshold)
	max_mm = (ajint) (ajStrLen(pattern) * threshold)/100;
    else max_mm = 0;

    if(!besthits)
	/* report all hits */
	vectorstrip_ccs_pattern(pattern, hitlist, seqname, seqstr, begin,
				&hits, max_mm);
    else
	/* start with mm=0, keep going till we get a hit, then STOP */
	while((!hits) && (mm <= max_mm)) 
	{
	    vectorstrip_ccs_pattern(pattern, hitlist, seqname, seqstr, begin,
				    &hits, mm);
	    mm++;
	}

    return;
}

/* @funcstatic vectorstrip_process_hits **************************************
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
** @param [r] fivelist [AjPList] list of EmbPMatMatch representing hits of 5' 
**                               pattern against the sequence
** @param [r] threelist [AjPList] list of EmbPMatMatch representing hits of 3'
**                                pattern against the sequence
** @param [r] sequence [AjPSeq] the sequence itself
** @param [r] seqout [AjPSeqout] place to writesubsequences
** @param [r] outf [AjPFile] file for writing information about the hits
** @return [void]
******************************************************************************/

static void vectorstrip_process_hits(AjPList fivelist, AjPList threelist, 
				     AjPSeq sequence, AjPSeqout seqout, 
				     AjPFile outf)
{
    ajint i=0;
    ajint j=0;
    ajint type = 0;

    AjPInt five = ajIntNew();	/* start positions for hits with 5' pattern */
    AjPInt three = ajIntNew();	/* start positions for hits with 3' pattern */

    EmbPMatMatch m=NULL;
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
	/*
	 * generally, every 5' hit will be matched against every 3' hit to
	 * produce subsequences. Special case: 3' pattern matches upstream
	 * of 5' pattern - to be consistent, this will cause everything 
	 * from the start of the sequence to the 3' hit to be written out, 
	 * and also everything from the 5' hit to the end of the sequence.
	 * It's a bit back to front ...
	 */
	for(i=0; i<ajIntLen(five); i++)
	{
	    ajint hit = 0;
	    for(j=0; j<ajIntLen(three); j++)
	    {
		if(ajIntGet(five,i) <= ajIntGet(three,j))
		{
		    hit = 1;
		    vectorstrip_write_sequence(sequence, seqout,
					       ajIntGet(five,i),
					       ajIntGet(three,j), outf); 
		}
	    }
	    if(!hit)
	    {
		vectorstrip_write_sequence(sequence, seqout, ajIntGet(five, i), 
					   ajSeqEnd(sequence),outf);
	    }
	}

	for(i=0; i<ajIntLen(three); i++)
	{
	    ajint hit = 0;
	    for(j=0; j<ajIntLen(five); j++)
		if(ajIntGet(three,i) >= ajIntGet(five,j))
		    hit=1;
	    if(!hit)
	    {
		vectorstrip_write_sequence(sequence, seqout,
					   ajSeqBegin(sequence), 
					   ajIntGet(three,i),outf);
	    }
	}
	break;
      
    case 3:
	for(i=0; i<ajIntLen(five); i++)
	    vectorstrip_write_sequence(sequence, seqout, ajIntGet(five, i), 
				       ajSeqEnd(sequence), outf);
	break;
    case 4:
	for(j=0; j<ajIntLen(three); j++)
	    vectorstrip_write_sequence(sequence, seqout, ajSeqBegin(sequence), 
				       ajIntGet(three, j), outf);
	break;

    default:
	break;
    }
  
    /* tidy up */
    ajIntDel(&five);
    ajIntDel(&three);

    return;
}

/* @funcstatic vectorstrip_scan_sequence *************************************
**
** Scans a Vector against a sequence
**
** @param [r] vector [Vector] the vector data
** @param [r] seqout [AjPSeqout] where to write out subsequences
** @param [w] outf [AjPFile] file for writing results details
** @param [w] sequence [AjPSeq] the sequence to be scanned
** @param [r] mis_per [ajint] max mismatch percentage
** @param [r] besthits [AjBool] stop scanning when we get hits, even if
**                              mis_per is not reached yet
** @return [void]
******************************************************************************/

static void vectorstrip_scan_sequence(Vector vector, AjPSeqout seqout,
				      AjPFile outf, AjPSeq sequence,
				      ajint mis_per, AjBool besthits)
{
    ajint begin = 0;
    ajint end = 0;
  
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
	vectorstrip_process_pattern(vector->fiveprime, &fivelist, seqname,
				    text, mis_per, begin, besthits);

    if(ajStrLen(vector->threeprime))
	vectorstrip_process_pattern(vector->threeprime, &threelist, seqname,
				    text, mis_per, begin, besthits);

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
	vectorstrip_print_hits(fivelist,outf,text,begin);
	ajFmtPrintF(outf, "3' sequence matches:\n");
	vectorstrip_print_hits(threelist,outf,text,begin);
    
	ajFmtPrintF(outf, "Sequences output to file:\n");
	vectorstrip_process_hits(fivelist, threelist, sequence, seqout, outf);
    }

    /* tidy up */
    vectorstrip_free_list(fivelist);
    vectorstrip_free_list(threelist);
    ajStrDel(&seqname);
    ajStrDel(&text);

    return;
}

/* @funcstatic vectorstrip_ccs_pattern ****************************************
**
** Classifies, compiles and searches pattern against seqstr with 
** mm mismatches
**
** @param [r] pattern [AjPStr] pattern to be searched for
** @param [w] hitlist [AjPList*] list of hits
** @param [r] seqname [AjPStr] name of sequence to be searched
** @param [r] seqstr [AjPStr] string representing sequence to be searched
** @param [r] begin [ajint] start position of sequence
** @param [w] hits [ajint*] number of hits
** @param [r] mm [ajint] number of mismatches
** @return [void]
******************************************************************************/

static void vectorstrip_ccs_pattern(AjPStr pattern, AjPList* hitlist,
				    AjPStr seqname, AjPStr seqstr,
				    ajint begin, ajint* hits, ajint mm)
{
    /* set up CPattern */
    CPattern cpat = NULL;
    vectorstrip_initialise_cp(&cpat);
    ajStrAssC(&(cpat->patstr), ajStrStr(pattern));
    /* copy the original pattern for Henry Spencer code */
    ajStrAssC(&(cpat->origpat), ajStrStr(pattern));
  
    if(!(cpat->type = embPatGetType(&(cpat->patstr),mm,0,
				    &(cpat->real_len), 
				    &(cpat->amino), 
				    &(cpat->carboxyl))))
    {    
	ajWarn("Illegal pattern: %s", ajStrStr(cpat->patstr));
	vectorstrip_free_cp(&cpat);
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

    vectorstrip_free_cp(&cpat);
    return;
}

/* result output */


/* @funcstatic vectorstrip_write_sequence ************************************
**
** Details of the output 
** sequence (hit positions, number of mismatches, sequences trimmed 
** from 5' and 3' ends) are written to outf
**
** @param [r] sequence [AjPSeq] the entire sequence
** @param [w] seqout [AjPSeqout] where to write out subsequences
** @param [r] start [ajint] start position of desired subsequence relative
**                          to sequence
** @param [r] end [ajint] end position of desired subsequence relative
**                        to sequence 
** @param [w] outf [AjPFile] file to write details of output subsequence
** @return [void]
******************************************************************************/

static void vectorstrip_write_sequence(AjPSeq sequence, AjPSeqout seqout,
				       ajint start, ajint end, AjPFile outf)
{
    AjPStr name = NULL;
    AjPStr num = NULL;

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
	vectorstrip_reportseq(outs, outf);
	if(start !=1)
	{
	    ajStrAssSub(&fivetrim, ajSeqStr(seqcp), 0, start-2); 
	    ajFmtPrintF(outf, "\tsequence trimmed from 5' end:\n");
	    vectorstrip_reportseq(fivetrim, outf);
	}
	if(end!=ajSeqLen(seqcp))
	{
	    ajStrAssSub(&threetrim, ajSeqStr(seqcp), end, ajSeqLen(seqcp));
	    (void) ajFmtPrintF(outf, "\tsequence trimmed from 3' end:\n");
	    (void) vectorstrip_reportseq(threetrim, outf);
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

    return;
}

/* @funcstatic vectorstrip_print_hits ****************************************
**
** Output hit positions and number of mismatches of a pattern against a 
** sequence to file 
**
** @param [r] hitlist [AjPList] list of hits of pattern against the sequence
** @param [w] outf [AjPFile] file for writing information about the hits
** @param [r] seq [AjPStr] string representation of sequence
** @param [r] begin [ajint] start of sequence
** @return [void]
******************************************************************************/
static void vectorstrip_print_hits(AjPList hitlist, AjPFile outf, AjPStr seq,
				   ajint begin)
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
	
	ajFmtPrintF(outf,"\tFrom %d to %d with %d mismatches\n",m->start,
		    m->end, m->mm);
	
    }
    
    ajListIterFree(iter);
    ajStrDel(&s);
    
    return;
}

/* @funcstatic vectorstrip_reportseq ****************************************
**
** Formatted output of sequence data. 
** sequence is written to outf in lines of length 50.
**
** @param [r] seqstr [AjPStr] sequence to be output
** @param [r] outf [AjPFile] file to write to
** @return [void]
******************************************************************************/
static void vectorstrip_reportseq(AjPStr seqstr, AjPFile outf)
{
    AjPStr tmp = NULL;
    ajint x=0;
    ajint linelen = 50;
  
    for(x=0; x<ajStrLen(seqstr); x+= linelen)
    {
	(void) ajStrAssSub(&tmp, seqstr, x, x+linelen-1);
	ajFmtPrintF(outf, "\t\t%S\n", tmp);
    }

    ajStrDel(&tmp);

    return;
}


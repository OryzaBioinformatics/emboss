/* @source primersearch application
**
** Searches a set of primer pairs against a set of DNA sequences in both
** forward and reverse sense.
** Modification of fuzznuc.
** @author: Copyright (C) Val Curwen (vac@sanger.ac.uk)
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




/* @datastatic PGuts **********************************************************
**
** the internals of a primer; each Primer has two of these,
** one forward and one reverse
**
** @alias primerguts
**
** @attr patstr [AjPStr] Undocumented
** @attr origpat [AjPStr] Undocumented
** @attr type [ajint] Undocumented
** @attr len [ajint] Undocumented
** @attr real_len [ajint] Undocumented
** @attr amino [AjBool] Undocumented
** @attr carboxyl [AjBool] Undocumented
** @attr mm [ajint] Undocumented
** @attr buf [ajint*] Undocumented
** @attr sotable [ajuint*] Undocumented
** @attr solimit [ajuint] Undocumented
** @attr off [EmbOPatBYPNode[AJALPHA]] Undocumented
** @attr re [AjPStr] Undocumented
** @attr skipm [ajint**] Undocumented
** @attr tidy [void*] Undocumented
******************************************************************************/

typedef struct primerguts
{
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
    ajint** skipm;
    void* tidy;
} *PGuts;




/* @datastatic PHit ***********************************************************
**
** holds details of a hit against a sequence ie this primer will amplify
**
** @alias primerhit
**
** @attr seqname [AjPStr] Undocumented
** @attr desc [AjPStr] Undocumented
** @attr acc [AjPStr] Undocumented
** @attr forward [AjPStr] pattern that hits forward strand 
** @attr reverse [AjPStr] pattern that hits reverse strand 
** @attr forward_pos [ajint] Undocumented
** @attr reverse_pos [ajint] Undocumented
** @attr amplen [ajint] Undocumented
** @attr forward_mismatch [ajint] Undocumented
** @attr reverse_mismatch [ajint] Undocumented
******************************************************************************/

typedef struct primerhit
{
    AjPStr seqname;
    AjPStr desc;
    AjPStr acc;
    AjPStr forward;
    AjPStr reverse;
    ajint forward_pos;
    ajint reverse_pos;
    ajint amplen;
    ajint forward_mismatch;
    ajint reverse_mismatch;
} *PHit;




/* @datastatic Primer *********************************************************
**
** primer pairs will be read into a list of these structs
**
** @alias primers
**
** @attr Name [AjPStr] Undocumented
** @attr forward [PGuts] Undocumented
** @attr reverse [PGuts] Undocumented
** @attr hitlist [AjPList] Undocumented
******************************************************************************/

typedef struct primers
{
    AjPStr Name;
    PGuts forward;
    PGuts reverse;
    AjPList hitlist;
} *Primer;




/* "constructors" */
static void psearch_initialise_pguts(PGuts* primer);

/* "destructors" */
static void psearch_free_pguts(PGuts* primer);
static void psearch_free_primer(void** x, void* cl);
static void psearch_clean_hitlist(AjPList* hlist);

/* utilities */
static void psearch_read_primers(AjPList* primerList, AjPFile primerFile,
				 ajint mmp);
static AjBool psearch_classify_and_compile(Primer* primdata);
static void psearch_primer_search(const AjPList primerList, const AjPSeq seq,
				  AjPFile outf);
static void psearch_scan_seq(const Primer primdata,
			     const AjPSeq seq, AjBool reverse,
			     AjPFile outf);
static void psearch_store_hits(const Primer primdata, AjPList fhits_list,
			       AjPList rhits_list, const AjPSeq seq,
			       AjBool reverse);
static void psearch_print_hits(const AjPList primerList, AjPFile outf);




/* @prog primersearch *********************************************************
**
** Searches DNA sequences for matches with primer pairs
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPSeqall seqall;
    AjPSeq seq = NULL;
    AjPFile primerFile;		  /* read the primer pairs from a file */
    AjPFile outf;
    AjPList primerList;

    ajint mmp = 0;

    embInit("primersearch", argc, argv);

    seqall     = ajAcdGetSeqall("seqall");
    outf       = ajAcdGetOutfile("outfile");
    primerFile = ajAcdGetInfile("infile");
    mmp        = ajAcdGetInt("mismatchpercent");

    /* build list of forward/reverse primer pairs as read from primerfile */
    primerList = ajListNew();

    /* read in primers from primerfile, classify and compile them */
    psearch_read_primers(&primerList,primerFile, mmp);

    /* check there are primers to be searched */
    if(!ajListLength(primerList))
    {
	ajUser("\nNo suitable primers found - exiting\n");
	ajExit();
	return 0;

    }

    /* query sequences one by one */
    while(ajSeqallNext(seqall,&seq))
	psearch_primer_search(primerList, seq, outf);

    /* output the results */
    psearch_print_hits(primerList, outf);

    /* delete all nodes of list, then the list itself */
    ajListMap(primerList, psearch_free_primer, NULL);
    ajListFree(&primerList);
    ajListDel(&primerList);

    ajFileClose(&outf);

    ajExit();

    return 0;
}




/* "constructors" */

/* @funcstatic psearch_initialise_pguts ***************************************
**
** Initialise primer guts
**
** @param [w] primer [PGuts*] primer guts
** @@
******************************************************************************/

static void psearch_initialise_pguts(PGuts* primer)
{

    AJNEW(*primer);
    (*primer)->patstr  = NULL;
    (*primer)->origpat = ajStrNew();
    (*primer)->type = 0;
    (*primer)->len  = 0;
    (*primer)->real_len = 0;
    (*primer)->re = NULL;
    (*primer)->amino = 0;
    (*primer)->carboxyl = 0;
    (*primer)->tidy = NULL;

    (*primer)->mm = 0;
    (*primer)->buf = NULL;
    (*primer)->sotable = NULL;
    (*primer)->solimit = 0;
    (*primer)->re = NULL;
    (*primer)->skipm = NULL;

    return;
}




/* "destructors" */

/* @funcstatic psearch_free_pguts *********************************************
**
** Frees up all the internal members of a PGuts struct
**
** @param [d] primer [PGuts*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void psearch_free_pguts(PGuts* primer)
{
    ajint i = 0;

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

    return;
}




/* @funcstatic psearch_free_primer ********************************************
**
** frees up the internal members of a Primer
**
** @param [r] x [void**] Undocumented
** @param [r] cl [void*] Undocumented
** @@
******************************************************************************/

static void psearch_free_primer(void **x, void *cl)
{
    Primer* p;
    Primer primdata;
    AjIList lIter;

    p = (Primer*) x;
    primdata = *p;

    psearch_free_pguts(&primdata->forward);
    psearch_free_pguts(&primdata->reverse);
    ajStrDel(&primdata->Name);

    /* clean up hitlist */
    lIter = ajListIterRead(primdata->hitlist);
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
    ajListIterFree(&lIter);

    AJFREE(primdata);

    return;
}




/* @funcstatic psearch_clean_hitlist ******************************************
**
** Clean the hitlist
**
** @param [d] hlist [AjPList*] Undocumented
** @@
******************************************************************************/

static void psearch_clean_hitlist(AjPList* hlist)
{
    AjIList lIter;

    lIter = ajListIterRead(*hlist);
    while(!ajListIterDone(lIter))
    {
	EmbPMatMatch fm = ajListIterNext(lIter);
	embMatMatchDel(&fm);
    }
    ajListFree(hlist);
    ajListDel(hlist);
    ajListIterFree(&lIter);

    return;
}




/* utilities */

/* @funcstatic psearch_read_primers *******************************************
**
** read primers in from primerfile, classify and compile the patterns
**
** @param [w] primerList [AjPList*] primer list
** @param [u] primerFile [AjPFile] Undocumented
** @param [r] mmp [ajint] Undocumented
** @@
******************************************************************************/

static void psearch_read_primers(AjPList *primerList, AjPFile primerFile,
				 ajint mmp)
{
    AjPStr rdline = NULL;
    AjPStrTok handle = NULL;

    ajint nprimers = 0;
    Primer primdata = NULL;


    while (ajFileReadLine (primerFile, &rdline))
    {
	primdata = NULL;
	if (ajStrChar(rdline, 0) == '#')
	    continue;
	if (ajStrSuffixC(rdline, ".."))
	    continue;

	AJNEW(primdata);
	primdata->Name = NULL;

	psearch_initialise_pguts(&primdata->forward);
	psearch_initialise_pguts(&primdata->reverse);

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
		  ajStrStr(primdata->reverse->patstr));

	/* set the mismatch level */
	primdata->forward->mm = (ajint) (ajStrLen(primdata->forward->patstr)*
					 mmp)/100;
	primdata->reverse->mm = (ajint) (ajStrLen(primdata->reverse->patstr)*
					 mmp)/100;

	if(psearch_classify_and_compile(&primdata))
	{
	    ajListPushApp (*primerList, primdata);
	    nprimers++;
	}
	else	/* there was something funny about the primer sequences */
	{
	    ajUser("Cannot use %s\n", ajStrStr(primdata->Name));
	    psearch_free_pguts(&primdata->forward);
	    psearch_free_pguts(&primdata->reverse);
	    ajStrDel(&primdata->Name);
	    ajListFree(&primdata->hitlist);
	    ajListDel(&primdata->hitlist);
	    AJFREE(primdata);
	}
    }

    ajStrDel(&rdline);
    ajFileClose(&primerFile);

    return;
}




/* @funcstatic psearch_classify_and_compile ***********************************
**
** determines pattern type and compiles it
**
** @param [w] primdata [Primer*] primer data
** @return [AjBool] true if useable primer
** @@
******************************************************************************/

static AjBool psearch_classify_and_compile(Primer* primdata)
{

    /* forward primer */
    if(!((*primdata)->forward->type =
	 embPatGetType(((*primdata)->forward->origpat),
		       &((*primdata)->forward->patstr),
		       (*primdata)->forward->mm,0,
		       &((*primdata)->forward->real_len),
		       &((*primdata)->forward->amino),
		       &((*primdata)->forward->carboxyl))))
	ajFatal("Illegal pattern");

    /* reverse primer */
    if(!((*primdata)->reverse->type =
	 embPatGetType(((*primdata)->reverse->origpat),
		       &((*primdata)->reverse->patstr),
		       (*primdata)->reverse->mm,0,
		       &((*primdata)->reverse->real_len),
		       &((*primdata)->reverse->amino),
		       &((*primdata)->reverse->carboxyl))))
	ajFatal("Illegal pattern");

    embPatCompile((*primdata)->forward->type,
		  (*primdata)->forward->patstr,
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
		  &((*primdata)->reverse->len),
		  &((*primdata)->reverse->buf),
		  (*primdata)->reverse->off,
		  &((*primdata)->reverse->sotable),
		  &((*primdata)->reverse->solimit),
		  &((*primdata)->reverse->real_len),
		  &((*primdata)->reverse->re),
		  &((*primdata)->reverse->skipm),
		  (*primdata)->reverse->mm );

    return AJTRUE;			/* this is a useable primer */
}




/* @funcstatic psearch_primer_search ******************************************
**
** tests the primers in primdata against seq and writes results to outfile
**
** @param [r] primerList [const AjPList] primer list
** @param [r] seq [const AjPSeq] sequence
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void psearch_primer_search(const AjPList primerList, const AjPSeq seq,
				  AjPFile outf)
{
    AjIList listIter;

    /* test each list node against this sequence */
    listIter = ajListIterRead(primerList);
    while(!ajListIterDone(listIter))
    {
	Primer curr_primer = ajListIterNext(listIter);

	psearch_scan_seq(curr_primer, seq, AJFALSE, outf);
	psearch_scan_seq(curr_primer, seq, AJTRUE, outf);
    }

    ajListIterFree(&listIter);

    return;
}




/* @funcstatic psearch_scan_seq ***********************************************
**
** scans the primer pairs against the sequence in either forward
** sense or reverse complemented
** works out amplimer length if the two primers both hit
**
** @param [r] primdata [const Primer] primer data
** @param [r] seq [const AjPSeq] sequence
** @param [r] reverse [AjBool] do reverse
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void psearch_scan_seq(const Primer primdata,
			     const AjPSeq seq, AjBool reverse,
			     AjPFile outf)
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
    ajStrAssS(&seqstr, ajSeqStr(seq));
    ajStrAssS(&revstr, ajSeqStr(seq));
    ajStrToUpper(&seqstr);
    ajStrToUpper(&revstr);
    ajSeqReverseStr(&revstr);
    fhits_list = ajListNew();
    rhits_list = ajListNew();

    if(!reverse)
    {
	/* test OligoA against forward sequence, and OligoB against reverse */
	embPatFuzzSearch(primdata->forward->type,
			 ajSeqBegin(seq),
			 primdata->forward->patstr,
			 seqname,
			 seqstr,
			 fhits_list,
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
			     seqname,
			     revstr,
			     rhits_list,
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
			 seqname,
			 seqstr,
			 fhits_list,
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
			     seqname,
			     revstr,
			     rhits_list,
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
	psearch_store_hits(primdata, fhits_list, rhits_list, seq, reverse);

    /* tidy up */
    psearch_clean_hitlist(&fhits_list);
    psearch_clean_hitlist(&rhits_list);

    ajStrDel(&seqstr);
    ajStrDel(&revstr);
    ajStrDel(&seqname);

    return;
}




/* @funcstatic psearch_store_hits *********************************************
**
** Store primer hits
**
** @param [r] primdata [const Primer] primer data
** @param [w] fhits [AjPList] forward hits
** @param [w] rhits [AjPList] reverse hits
** @param [r] seq [const AjPSeq] sequence
** @param [r] reverse [AjBool] do reverse
** @@
******************************************************************************/

static void psearch_store_hits(const Primer primdata,
			       AjPList fhits, AjPList rhits,
			       const AjPSeq seq, AjBool reverse)
{
    ajint amplen = 0;
    AjIList fi;
    AjIList ri;

    PHit primerhit = NULL;

    fi = ajListIterRead(fhits);
    while(!ajListIterDone(fi))
    {
	EmbPMatMatch fm = NULL;
	EmbPMatMatch rm = NULL;
	amplen = 0;

	fm = ajListIterNext(fi);
	ri = ajListIterRead(rhits);
	while(!ajListIterDone(ri))
	{
	    ajint seqlen = ajSeqLen(seq);
	    ajint s = (fm->start);
	    ajint e;

	    rm = ajListIterNext(ri);
	    e = (rm->start-1);
	    amplen = seqlen-(s-1)-e;

	    if (amplen > 0)	   /* no point making a hit if -ve length! */
	    {
		primerhit = NULL;
		AJNEW(primerhit);
		primerhit->desc=NULL;	 /* must be NULL for ajStrAss */
		primerhit->seqname=NULL; /* must be NULL for ajStrAss */
		primerhit->acc=NULL;
		primerhit->forward=NULL;
		primerhit->reverse=NULL;
		ajStrAssC(&primerhit->seqname,ajSeqName(seq));
		ajStrAssS(&primerhit->desc, ajSeqGetDesc(seq));
		ajStrAssS(&primerhit->acc, ajSeqGetAcc(seq));
		primerhit->forward_pos = fm->start;
		primerhit->reverse_pos = rm->start;
		primerhit->forward_mismatch = fm->mm;
		primerhit->reverse_mismatch = rm->mm;
		primerhit->amplen = amplen;
		if(!reverse)
		{
		    ajStrAssS(&primerhit->forward, primdata->forward->patstr);
		    ajStrAssS(&primerhit->reverse, primdata->reverse->patstr);
		}
		else
		{
		    ajStrAssS(&primerhit->forward, primdata->reverse->patstr);
		    ajStrAssS(&primerhit->reverse, primdata->forward->patstr);
		}
		ajListPushApp(primdata->hitlist, primerhit);


	    }
	}
	/*
	**  clean up rListIter here as it will be new'ed again next
	**  time through
	*/
	ajListIterFree(&ri);
    }

    ajListIterFree(&fi);
    return;
}




/* @funcstatic psearch_print_hits *********************************************
**
** Print primer hits
**
** @param [r] primerList [const AjPList] primer hits
** @param [w] outf [AjPFile] outfile
** @@
******************************************************************************/

static void psearch_print_hits(const AjPList primerList, AjPFile outf)
{
    /* iterate through list of hits */
    AjIList lIter;

    ajint count = 1;
    lIter = ajListIterRead(primerList);
    while(!ajListIterDone(lIter))
    {
	Primer primer = ajListIterNext(lIter);
	AjIList hIter = ajListIterRead(primer->hitlist);
	count = 1;

	ajFmtPrintF(outf, "\nPrimer name %s\n", ajStrStr(primer->Name));

	while(!ajListIterDone(hIter))
	{
	    PHit hit = ajListIterNext(hIter);
	    ajFmtPrintF(outf, "Amplimer %d\n", count);
	    ajFmtPrintF(outf, "\tSequence: %s %s \n\t%s\n",
			ajStrStr(hit->seqname),
			ajStrStr(hit->acc), ajStrStr(hit->desc));
	    ajFmtPrintF(outf, "\t%s hits forward strand at %d with %d "
			"mismatches\n",
			ajStrStr(hit->forward), hit->forward_pos,
			hit->forward_mismatch);
	    ajFmtPrintF(outf, "\t%s hits reverse strand at [%d] with %d "
			"mismatches\n",
			ajStrStr(hit->reverse), (hit->reverse_pos),
			(hit->reverse_mismatch));
	    ajFmtPrintF(outf, "\tAmplimer length: %d bp\n", hit->amplen);
	    count++;
	}
	ajListIterFree(&hIter);
    }
    ajListIterFree(&lIter);

    return;
}

/* @source getorf application
**
** Finds and extracts open reading frames (ORFs)
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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

static void WriteORF(AjPSeq seq, int len, int seqlen, AjBool sense, int find,
		     int *orf_no, int start, int pos, AjPStr str,
		     AjPSeqout seqout, int around);

static void AppORF(int find, AjPStr *str, char *chrseq, int pos, char aa);

static void FindORFs(AjPSeq seq, int len, AjPTrn trnTable, int minsize,
		     AjPSeqout seqout, AjBool sense, AjBool circular,
		     int find, int *orf_no, AjBool methionine, int around);


/* types of control codon */
#define START 1
#define STOP -1

/* types of ORF to find */
#define P_STOP2STOP 0
#define P_START2STOP 1
#define N_STOP2STOP 2
#define N_START2STOP 3
#define AROUND_START 4
#define AROUND_INIT_STOP 5
#define AROUND_END_STOP 6

int main (int argc, char * argv[])
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPStr *tablelist;
    int table;
    int minsize;
    AjPStr *findlist;
    int find;
    AjBool methionine;
    AjBool circular;
    AjBool reverse;
    int around;
  
    AjPSeq seq=NULL;
    AjPTrn trnTable;
    AjPStr sseq=NULL;			/* sequence string */

    int orf_no;				/* ORF number to append to name of
					   sequence to create unique name */

    AjBool sense;			/* ajTrue = forward sense,
					   ajFalse = reverse sense */
    int len;

    (void) embInit ("getorf", argc, argv);

    seqout = ajAcdGetSeqoutall ("outseq");
    seqall = ajAcdGetSeqall ("sequence");
    tablelist = ajAcdGetList ("table");
    minsize = ajAcdGetInt ("minsize");
    findlist = ajAcdGetList ("find");
    methionine = ajAcdGetBool("methionine");
    circular = ajAcdGetBool("circular");
    reverse = ajAcdGetBool("reverse");
    around = ajAcdGetInt ("flanking");
  

    /* initialise the translation table */
    (void) ajStrToInt(tablelist[0], &table);
    trnTable = ajTrnNewI (table);

    /* what sort of ORF are we looking for */
    (void) ajStrToInt(findlist[0], &find);  

    /* get the minimum size converted to protein length if we are storing
       protein sequences */
    if (find == P_STOP2STOP || find == P_START2STOP || find == AROUND_START)
	minsize /= 3;

    while (ajSeqallNext(seqall, &seq))
    {
	orf_no = 1;			/* number of the next ORF */
	sense = ajTrue;			/* we are doing this in the forward
					   sense initially */

	/* get the length of the sequence */
	len = ajSeqLen(seq);
    
	/* if the sequence is circular, append it to itself to triple its
	   length so we can deal easily with wrapped ORFs, but don't update
	   len */
	if (circular)
	{
	    (void) ajStrAss(&sseq, ajSeqStr(seq));
	    (void) ajStrApp(&sseq, ajSeqStr(seq));
	    (void) ajStrApp(&sseq, ajSeqStr(seq));
	    (void) ajSeqReplace(seq, sseq);
	    /* GWW 3 Sept 1999 - is this required/useful?
	       (void) ajStrDel(sseq); */
	}

	/* find the ORFs */
	(void) FindORFs(seq, len, trnTable, minsize, seqout, sense, circular,
			find, &orf_no, methionine, around);

	/* now reverse complement the sequence and do it again */
	if (reverse)
	{
	    sense = ajFalse;
	    (void) ajSeqReverse(seq);
	    (void) FindORFs(seq, len, trnTable, minsize, seqout, sense,
			    circular, find, &orf_no, methionine, around);
	}
    }
  
    (void) ajSeqWriteClose (seqout);

    /* tidy up */
    (void) ajTrnDel(&trnTable);

    ajExit ();
    return 0;
}






/******************************************************************************/
/* finds all orfs in the current sense and writes them out */

static void FindORFs(AjPSeq seq, int len, AjPTrn trnTable, int minsize,
		     AjPSeqout seqout, AjBool sense, AjBool circular,
		     int find, int *orf_no, AjBool methionine, int around)
{

    AjBool ORF[3];			/* true if found an ORF */
    AjBool LASTORF[3];			/* true if hit the end of an ORF past
					   the end on the genome in this
					   frame */
    AjBool GOTSTOP[3];			/* true if found a STOP in a circular
					   genome's frame when
					   find = P_STOP2STOP or
					   N_STOP2STOP */
    int start[3];			/* possible starting position of the
					   three frames */
    int pos;
    int codon;
    char aa;
    int frame;
    AjPStr newstr[3];			/* strings of the three frames of ORF
					   sequences that we are growing */
    AjPSeq pep=NULL;
    int i;
  
    int seqlen=ajSeqLen(seq);		/* length of the sequence passed
					   over - this will differ from 'len'
					   if circular=true */
    char *chrseq = ajSeqChar(seq);

    /* initialise the ORF sequences */
    newstr[0] = NULL;
    newstr[1] = NULL;
    newstr[2] = NULL;

    /* initialise flags for found the last ORF past the end of a circular
       genome */
    LASTORF[0] = ajFalse;
    LASTORF[1] = ajFalse;
    LASTORF[2] = ajFalse;

    /* initialise flags for found at least one STOP codon in a frame */
    GOTSTOP[0] = ajFalse;
    GOTSTOP[1] = ajFalse;
    GOTSTOP[2] = ajFalse;
  
    if (circular || find == P_START2STOP || find == N_START2STOP ||
	find == AROUND_START)
    {
	ORF[0] = ajFalse;
	ORF[1] = ajFalse;
	ORF[2] = ajFalse;
    }
    else
    {
	/* assume we are already in a ORF so we get ORFs at the start of the
	   sequence */
	ORF[0] = ajTrue;
	ORF[1] = ajTrue;
	ORF[2] = ajTrue;
	start[0] = 0;
	start[1] = 1;
	start[2] = 2;
    }

    for (pos=0; pos<seqlen-2; pos++)
    {
	codon = ajTrnStartStopC(trnTable, &chrseq[pos], &aa);
	frame = pos % 3;
	(void) ajDebug("len=%d, Pos=%d, Frame=%d start/stop=%d, aa=%c",
		       len, pos, frame, codon, aa);
	if (LASTORF[frame]) continue;	/* don't want to find extra ORFs when
					   we have been round a circular
					   genome once */
    
	if (find == P_STOP2STOP || find == N_STOP2STOP ||
	    find == AROUND_INIT_STOP || find == AROUND_END_STOP)
	{
	    /* note that we had at least one STOP in a circular genome */
	    if (codon == STOP)
	    {
		GOTSTOP[frame] = ajTrue;
	    }

	    /* write details if we hit a STOP or the end of the sequence */
	    if (codon == STOP || pos >= seqlen-5)
	    {

		/* did we hit the end of the sequence? If so, append any
		   last codon to the sequence - otherwise, ignore the STOP
		   codon */
		if (pos >= seqlen-5 && pos < seqlen-2)
		    (void) AppORF(find, &newstr[frame], chrseq, pos, aa);

		/* see if we already have a sequence to write out */
		if (ORF[frame])
		{
		    if (ajStrLen(newstr[frame]) >= minsize)
		    {
			/* create a new sequence */
			if (codon == STOP)
			    (void) WriteORF(seq, len, seqlen, sense, find,
					    orf_no, start[frame], pos-1,
					    newstr[frame], seqout, around);
			else
			    (void) WriteORF(seq, len, seqlen, sense, find,
					    orf_no, start[frame], pos+2,
					    newstr[frame], seqout, around);
		    }
		    /* reset the newstr to zero length so that we can start
		       storing the next ORF for this frame in it */
		    (void) ajStrClear(&newstr[frame]);

		}

		/* if we have a circular genome and we have hit the STOP past
		   the end of the genome in all frames, we want to break */
		if (circular && pos >= len)
		{
		    ORF[frame] = ajFalse; /* no longer in an ORF as we are
					     past the end of the genome */
		    LASTORF[frame] = ajTrue; /* note that we have finished
						getting ORFs in this frame */
		    if (LASTORF[0] && LASTORF[1] && LASTORF[2]) break;
		}
		else
		{
		    /* as we hit a STOP, we have a potential ORF to write
		       out next time, even if the genome is circular */
		    ORF[frame] = ajTrue;
		    start[frame] = pos+3; /* next start of the ORF for this
					     frame */
		}
	
	    }
	    else if (ORF[frame])
	    {
		/* append sequence to newstr if we are in an ORF */
		(void) AppORF(find, &newstr[frame], chrseq, pos, aa);
	    }

	}
	else
	{
	    /* if (find == P_START2STOP || find == N_START2STOP ||
	       find == AROUND_START) { */
	    if (codon == START && !ORF[frame])
	    {				/* not in a ORF already and found a
					   START */
		if (pos < len)
		{
		    /* reset the newstr to zero length so that we can start
		       storing the ORF for this frame in it */
		    (void) ajStrClear(&newstr[frame]);
		    ORF[frame] = ajTrue; /* we are now in an ORF (as long as
					    we are not circular and past the
					    length of the genome) */
		    start[frame] = pos;	/* start of the ORF for this frame */
		    if (methionine)
		    {
			(void) AppORF(find, &newstr[frame], chrseq, pos, 'M');
			/* start sequence with Met */
		    }
		    else
		    {
			(void) AppORF(find, &newstr[frame], chrseq, pos, aa);
			/* append sequence to newstr */
		    }
		}

	    }
	    else if (codon == STOP || pos >= seqlen-5)
	    {	/* hit a STOP or the end of the sequence */
		/* see if we already have a sequence to write out */
		if (ORF[frame])
		{
		    ORF[frame] = ajFalse; /* we are now not in an ORF */

		    /* did we hit the end of the sequence? If so, append any
		       last codon to the sequence - otherwise, ignore the
		       STOP codon */
		    if (pos >= seqlen-5 && pos < seqlen-2)
			(void) AppORF(find, &newstr[frame], chrseq, pos, aa);

		    if (ajStrLen(newstr[frame]) >= minsize)
		    {
			/* create a new sequence */
			if (codon == STOP)
			    (void) WriteORF(seq, len, seqlen, sense, find,
					    orf_no, start[frame], pos-1,
					    newstr[frame], seqout, around);
			else
			{
			    (void) WriteORF(seq, len, seqlen, sense, find,
					    orf_no, start[frame], pos+2,
					    newstr[frame], seqout, around);
			}
		    }
		}

		/* if we have a circular genome and we have hit the STOP past
		   the end of the genome in all frames, we want to break */
		if (circular && pos >= len)
		{
		    LASTORF[frame] = ajTrue; /* note that we have finished
						getting ORFs in this frame */
		    if (LASTORF[0] && LASTORF[1] && LASTORF[2]) break;
		}
	
		/* reset the newstr to zero length so that we can start
		   storing the next ORF for this frame in it */
		(void) ajStrClear(&newstr[frame]);
	    }
	    else
	    {
		/* append sequence to newstr if we are in an ORF */
		if (ORF[frame])
		    (void) AppORF(find, &newstr[frame], chrseq, pos, aa);
        	
	    }      	
	}
	
    }
    
    /* So far we will currently miss reporting a STOP-to-STOP ORF that is
       the full length of a circular genome when there are no STOP codons in
       that frame - is this a problem? It is an unlikely situation, but I'm
       sure someone will complain about it.  Sigh.  Here we go! */
    if ((find == P_STOP2STOP || find == N_STOP2STOP) && circular)
    {
	if (!GOTSTOP[0])
	{
	    /* translate frame 1 into pep */
	    pep = ajTrnSeqOrig(trnTable, seq, 1);
	    if (ajSeqLen(pep) >= minsize)
		(void) WriteORF(seq, len, seqlen, sense, find, orf_no, 0,
				seqlen-1, ajSeqStr(pep), seqout, around);
	    (void) ajSeqDel (&pep);
	}
	if (!GOTSTOP[1])
	{
	    /* translate frame 2 into pep */
	    pep = ajTrnSeqOrig(trnTable, seq, 2);
	    if (ajSeqLen(pep) >= minsize)
	    {
		(void) WriteORF(seq, len, seqlen, sense, find, orf_no, 1,
				seqlen-1, ajSeqStr(pep), seqout, around);
	    }
	    (void) ajSeqDel (&pep);
	}
	if (!GOTSTOP[2])
	{
	    /* translate frame 3 into pep */
	    pep = ajTrnSeqOrig(trnTable, seq, 3);
	    if (ajSeqLen(pep) >= minsize)
	    {
		(void) WriteORF(seq, len, seqlen, sense, find, orf_no, 2,
				seqlen-1, ajSeqStr(pep), seqout, around);
	    }
	    (void) ajSeqDel (&pep);
	}
    }

    for(i=0;i<3;++i)
	ajStrDel(&newstr[i]);

    return;
}

/******************************************************************************/
static void WriteORF(AjPSeq seq, int len, int seqlen, AjBool sense,
		     int find, int *orf_no, int start, int pos, AjPStr str,
		     AjPSeqout seqout, int around)
{
    /* pos is the sequence position of the last nucleotide in the ORF */

    AjPSeq new;
    AjPStr name=NULL;			/* name of the ORF */
    AjPStr value=NULL;			/* string value of the ORF number */
    int s, e;				/* start and end positions */
    AjPStr aroundstr=NULL;		/* holds sequence string around the
					   codon of interest */
    int codonpos=0;			/* holds position of start of codon
					   of interest */

    /* convert numbers in the range 0..seqlen+1 into numbers in the
       range 1..seqlen for human readability */
    s = start+1;
    e = pos+1;

    /* it is possible for an ORF in a circular genome to appear to start
       past the end of the genome if we have a START at the very end,
       eg:
       START (end of genome) ORF ORF ORF ORF STOP 
       */
    while (s > len)
    {
	s -= len;
	e -= len;
    }

    new = ajSeqNew();
    if (find == N_STOP2STOP || find == N_START2STOP ||
	find == AROUND_INIT_STOP || find == AROUND_END_STOP ||
	find == AROUND_START)
	(void) ajSeqSetNuc (new);
    else
	(void) ajSeqSetProt (new);


    /* Set the start and end positions to report and get the sequence if we
       want the AROUND* sequences */
    if (find == AROUND_INIT_STOP)
    {
	codonpos = s-3;
	s = codonpos - around;		/* 50 before the initial STOP */
	e = codonpos + around+2;	/* 50 after the end of the STOP */
	if (s < 1) return;	        /* don't report this if the sequence
					   goes over either end */
	if (e > seqlen) return;
	(void) ajStrAssSub(&aroundstr, ajSeqStr(seq), s-1, e-1);
    
    }
    else if (find == AROUND_START)
    {
	codonpos = s;
	s = codonpos - around;		/* 50 before the initial STOP */
	e = codonpos + around+2;	/* 50 after the end of the STOP */
	if (s < 1) return;		/* don't report this if the sequence
					   goes over either end */
	if (e > seqlen) return;
	(void) ajStrAssSub(&aroundstr, ajSeqStr(seq), s-1, e-1);

    }
    else if (find == AROUND_END_STOP)
    {
	codonpos = e+1;
	s = codonpos - around;		/* 50 before the initial STOP */
	e = codonpos + around+2;	/* 50 after the end of the STOP */
	if (s < 1) return;		/* don't report this if the sequence
					   goes over either end */
	if (e > seqlen) return;
	(void) ajStrAssSub(&aroundstr, ajSeqStr(seq), s-1, e-1);
    }
  


    /* set the name and description */
    (void) ajStrAss(&name, ajSeqGetName(seq));
    (void) ajStrAppC(&name, "_");
    (void) ajStrFromInt(&value, (*orf_no)++); /* post-increment the ORF
						 number for the next ORF */
    (void) ajStrApp(&name, value);
    (void) ajSeqAssName(new, name);
  
    /* set the description of the translation */
    (void) ajStrAssC(&name, "[");
    if (sense)
    {					/* we want to reverse the reported
					   positions if this is the reverse
					   sense */
	(void) ajStrFromInt(&value, s);
    }
    else
    {
	(void) ajStrFromInt(&value, e);	/* the base before the stop codon
					   (numbering bases from 1) */    
    }
    (void) ajStrApp(&name, value);
    (void) ajStrAppC(&name, " - ");
    if (sense)
	(void) ajStrFromInt(&value, e);	/* the base before the stop codon
					   (numbering bases from 1) */
    else
	(void) ajStrFromInt(&value, s);    

    (void) ajStrApp(&name, value);
    (void) ajStrAppC(&name, "] ");
    if (find == AROUND_INIT_STOP || find == AROUND_START ||
	find == AROUND_END_STOP)
    {
	(void) ajStrAppC(&name, "Around codon at ");
	(void) ajStrFromInt(&value, codonpos);
	(void) ajStrApp(&name, value);
	(void) ajStrAppC(&name, ". ");
    }
    (void) ajStrApp(&name, ajSeqGetDesc(seq));
    (void) ajSeqAssDesc(new, name);


    /* replace newstr in new */
    if (find == N_STOP2STOP || find == N_START2STOP || find == P_STOP2STOP ||
	find == P_START2STOP)
    {
	(void) ajSeqReplace (new, str);
    }
    else
    {	/* we want the sequence 50 bases around the codon */
	(void) ajSeqReplace (new, aroundstr);
    }
    /* write out the sequence */
    (void) ajSeqAllWrite (seqout, new);
    /* get rid of the old orf sequence */
    (void) ajSeqDel(&new);
    ajStrDel(&value);
    ajStrDel(&name);


}



/***************************************************************************/
/* append aa to ORF sequence string */
static void AppORF(int find, AjPStr *str, char *chrseq, int pos, char aa)
{

    if (find == N_STOP2STOP || find == N_START2STOP ||
	find == AROUND_INIT_STOP || find == AROUND_END_STOP)
    {
	(void) ajStrAppK(str, chrseq[pos]);
	(void) ajStrAppK(str, chrseq[pos+1]);
	(void) ajStrAppK(str, chrseq[pos+2]);
    }
    else if(find == P_STOP2STOP || find == P_START2STOP ||
	    find == AROUND_START)
	(void) ajStrAppK(str, aa);            
}

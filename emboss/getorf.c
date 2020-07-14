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




static void getorf_WriteORF(AjPSeq seq, ajint len, ajint seqlen,
			    AjBool sense, ajint find, ajint *orf_no,
			    ajint start, ajint pos, AjPStr str,
			    AjPSeqout seqout, ajint around);

static void getorf_AppORF(ajint find, AjPStr *str, char *chrseq, ajint pos,
			  char aa);

static void getorf_FindORFs(AjPSeq seq, ajint len, AjPTrn trnTable,
			    ajint minsize, ajint maxsize, AjPSeqout seqout, 
			    AjBool sense, AjBool circular, ajint find, 
			    ajint *orf_no, AjBool methionine, ajint around);




/* types of control codon */
#define START 1
#define STOP -1

/* types of ORF to find */
#define P_STOP2STOP      0
#define P_START2STOP     1
#define N_STOP2STOP      2
#define N_START2STOP     3
#define AROUND_START     4
#define AROUND_INIT_STOP 5
#define AROUND_END_STOP  6




/* @prog getorf ***************************************************************
**
** Finds and extracts open reading frames (ORFs)
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeqout seqout;
    AjPStr *tablelist;
    ajint table;
    ajint minsize;
    ajint maxsize;
    AjPStr *findlist;
    ajint find;
    AjBool methionine;
    AjBool circular;
    AjBool reverse;
    ajint around;

    AjPSeq seq = NULL;
    AjPTrn trnTable;
    AjPStr sseq = NULL;	/* sequence string */

    /* ORF number to append to name of sequence to create unique name */
    ajint orf_no;
			   

    AjBool sense;	/* ajTrue = forward sense */
    ajint len;

    embInit("getorf", argc, argv);
    
    seqout     = ajAcdGetSeqoutall("outseq");
    seqall     = ajAcdGetSeqall("sequence");
    tablelist  = ajAcdGetList("table");
    minsize    = ajAcdGetInt("minsize");
    maxsize    = ajAcdGetInt("maxsize");
    findlist   = ajAcdGetList("find");
    methionine = ajAcdGetBool("methionine");
    circular   = ajAcdGetBool("circular");
    reverse    = ajAcdGetBool("reverse");
    around     = ajAcdGetInt("flanking");
    
    
    /* initialise the translation table */
    ajStrToInt(tablelist[0], &table);
    trnTable = ajTrnNewI(table);
    
    /* what sort of ORF are we looking for */
    ajStrToInt(findlist[0], &find);
    
    /*
    ** get the minimum size converted to protein length if storing
    ** protein sequences
    */
    if(find == P_STOP2STOP || find == P_START2STOP || find == AROUND_START) 
    {
	minsize /= 3;
	maxsize /= 3;
    }
    
    while(ajSeqallNext(seqall, &seq))
    {
	orf_no = 1;		   /* number of the next ORF */
	sense = ajTrue;		   /* forward sense initially */

	/* get the length of the sequence */
	len = ajSeqLen(seq);

	/*
	** if the sequence is circular, append it to itself to triple its
	**  length so can deal easily with wrapped ORFs, but don't update
	** len
	*/
	if(circular)
	{
	    ajStrAss(&sseq, ajSeqStr(seq));
	    ajStrApp(&sseq, ajSeqStr(seq));
	    ajStrApp(&sseq, ajSeqStr(seq));
	    ajSeqReplace(seq, sseq);
	}

	/* find the ORFs */
	getorf_FindORFs(seq, len, trnTable, minsize, maxsize, seqout, sense,
			circular, find, &orf_no, methionine, around);

	/* now reverse complement the sequence and do it again */
	if(reverse)
	{
	    sense = ajFalse;
	    ajSeqReverse(seq);
	    getorf_FindORFs(seq, len, trnTable, minsize, maxsize, seqout, sense,
			    circular, find, &orf_no, methionine,
			    around);
	}
    }
    
    ajSeqWriteClose(seqout);
    ajTrnDel(&trnTable);
    
    ajExit();

    return 0;
}




/* @funcstatic getorf_FindORFs ************************************************
**
** finds all orfs in the current sense and writes them out
**
** @param [?] seq [AjPSeq] Undocumented
** @param [?] len [ajint] Undocumented
** @param [?] trnTable [AjPTrn] Undocumented
** @param [?] minsize [ajint] Minimum size ORF to find
** @param [?] maxsize [ajint] Maximum size ORF to find
** @param [?] seqout [AjPSeqout] Undocumented
** @param [?] sense [AjBool] Undocumented
** @param [?] circular [AjBool] Undocumented
** @param [?] find [ajint] Undocumented
** @param [?] orf_no [ajint*] Undocumented
** @param [?] methionine [AjBool] Undocumented
** @param [?] around [ajint] Undocumented
** @@
******************************************************************************/

static void getorf_FindORFs(AjPSeq seq, ajint len, AjPTrn trnTable,
			    ajint minsize, ajint maxsize, AjPSeqout seqout, 
			    AjBool sense, AjBool circular, ajint find, 
			    ajint *orf_no, AjBool methionine, ajint around)
{
    AjBool ORF[3];			/* true if found an ORF */
    AjBool LASTORF[3];		 /* true if hit the end of an ORF past
				    the end on the genome in this
				    frame */
    AjBool GOTSTOP[3];		 /* true if found a STOP in a circular
				    genome's frame when
				    find = P_STOP2STOP or
				    N_STOP2STOP */
    ajint start[3];		  /* possible starting position of the
				     three frames */
    ajint pos;
    ajint codon;
    char aa;
    ajint frame;
    AjPStr newstr[3];		 /* strings of the three frames of ORF
				    sequences that we are growing */
    AjPSeq pep = NULL;
    ajint i;

    ajint seqlen;
    char *chrseq;

    seqlen = ajSeqLen(seq);
    chrseq = ajSeqChar(seq);

    /* initialise the ORF sequences */
    newstr[0] = NULL;
    newstr[1] = NULL;
    newstr[2] = NULL;

    /*
    ** initialise flags for found the last ORF past the end of a circular
    ** genome
    */
    LASTORF[0] = ajFalse;
    LASTORF[1] = ajFalse;
    LASTORF[2] = ajFalse;

    /* initialise flags for found at least one STOP codon in a frame */
    GOTSTOP[0] = ajFalse;
    GOTSTOP[1] = ajFalse;
    GOTSTOP[2] = ajFalse;

    if(circular || find == P_START2STOP || find == N_START2STOP ||
       find == AROUND_START)
    {
	ORF[0] = ajFalse;
	ORF[1] = ajFalse;
	ORF[2] = ajFalse;
    }
    else
    {
	/*
	** assume already in a ORF so we get ORFs at the start of the
	** sequence
	*/
	ORF[0] = ajTrue;
	ORF[1] = ajTrue;
	ORF[2] = ajTrue;
	start[0] = 0;
	start[1] = 1;
	start[2] = 2;
    }

    for(pos=0; pos<seqlen-2; pos++)
    {
	codon = ajTrnStartStopC(trnTable, &chrseq[pos], &aa);
	frame = pos % 3;
	ajDebug("len=%d, Pos=%d, Frame=%d start/stop=%d, aa=%c",
		len, pos, frame, codon, aa);
	
	/* don't want to find extra ORFs when already been round circ */
	if(LASTORF[frame])
	    continue;
	
	if(find == P_STOP2STOP || find == N_STOP2STOP ||
	   find == AROUND_INIT_STOP || find == AROUND_END_STOP)
	{
	    /* note that there was at least one STOP in a circular genome */
	    if(codon == STOP)
	    {
		GOTSTOP[frame] = ajTrue;
	    }

	    /* write details if a STOP is hit or the end of the sequence */
	    if(codon == STOP || pos >= seqlen-5)
	    {

		/*
		** End of the sequence? If so, append any
		** last codon to the sequence - otherwise, ignore the STOP
		** codon
		*/
		if(pos >= seqlen-5 && pos < seqlen-2)
		    getorf_AppORF(find, &newstr[frame], chrseq, pos,
				  aa);

		/* Already have a sequence to write out? */
		if(ORF[frame])
		{
		    if(ajStrLen(newstr[frame]) >= minsize && 
		       ajStrLen(newstr[frame]) <= maxsize)
		    {
			/* create a new sequence */
			if(codon == STOP)
			    getorf_WriteORF(seq, len, seqlen, sense,
					    find, orf_no, start[frame],
					    pos-1, newstr[frame],
					    seqout, around);
			else
			    getorf_WriteORF(seq, len, seqlen, sense,
					    find, orf_no, start[frame],
					    pos+2, newstr[frame],
					    seqout, around);
		    }

		    ajStrClear(&newstr[frame]);
		}

		/*
		** if its a circular genome the STOP codon hit past
		** the end of the genome in all frames, then break
		*/
		if(circular && pos >= len)
		{
		    ORF[frame] = ajFalse; /* past the end of the genome */
		    LASTORF[frame] = ajTrue; /* finished getting ORFs */
		    if(LASTORF[0] && LASTORF[1] && LASTORF[2])
			break;
		}
		else
		{
		    /*
		    ** hit a STOP, therefore a potential ORF to write
		    ** out next time, even if the genome is circular
		    */
		    ORF[frame]   = ajTrue;
		    start[frame] = pos+3; /* next start of the ORF */
		}

	    }
	    else if(ORF[frame])
		/* append sequence to newstr if in an ORF */
		getorf_AppORF(find, &newstr[frame], chrseq, pos, aa);
	}
	else
	{

	    if(codon == START && !ORF[frame])
	    {
		/* not in a ORF already and found a START */
		if(pos < len)
		{
		    /*
		    **  reset the newstr to zero length to enable
		    **  storing the ORF for this
		    */
		    ajStrClear(&newstr[frame]);
		    ORF[frame] = ajTrue; /* now in an ORF */
		    start[frame] = pos;	/* start of the ORF for this frame */
		    if(methionine)
			getorf_AppORF(find, &newstr[frame], chrseq,
				      pos, 'M');
		    else
			getorf_AppORF(find, &newstr[frame], chrseq,
				      pos, aa);
		}
	    }
	    else if(codon == STOP || pos >= seqlen-5)
	    {
		/* hit a STOP or the end of the sequence */

		/* Already have a sequence to write out? */
		if(ORF[frame])
		{
		    ORF[frame] = ajFalse; /* not in an ORF */

		    /*
		    ** End of the sequence? If so, append any
		    ** last codon to the sequence - otherwise, ignore the
		    ** STOP codon
		    */
		    if(pos >= seqlen-5 && pos < seqlen-2)
			getorf_AppORF(find, &newstr[frame], chrseq,
				      pos, aa);

		    if(ajStrLen(newstr[frame]) >= minsize &&
		       ajStrLen(newstr[frame]) <= maxsize)
		    {
			/* create a new sequence */
			if(codon == STOP)
			    getorf_WriteORF(seq, len, seqlen, sense,
					    find, orf_no, start[frame],
					    pos-1, newstr[frame],
					    seqout, around);
			else
			    getorf_WriteORF(seq, len, seqlen, sense,
					    find, orf_no, start[frame],
					    pos+2, newstr[frame],
					    seqout, around);
		    }
		}

		/*
		** if a circular genome and hit the STOP past
		** the end of the genome in all frames, then break
		*/
		if(circular && pos >= len)
		{
		    LASTORF[frame] = ajTrue; /* finished getting ORFs */
		    if(LASTORF[0] && LASTORF[1] && LASTORF[2]) break;
		}

		ajStrClear(&newstr[frame]);
	    }
	    else
		if(ORF[frame])
		    getorf_AppORF(find, &newstr[frame], chrseq, pos,
				  aa);

	}
    }

    /*
    ** Currently miss reporting a STOP-to-STOP ORF that is
    ** the full length of a circular genome when there are no STOP codons in
    ** that frame
    */
    if((find == P_STOP2STOP || find == N_STOP2STOP) && circular)
    {
	if(!GOTSTOP[0])
	{
	    /* translate frame 1 into pep */
	    pep = ajTrnSeqOrig(trnTable, seq, 1);
	    if(ajSeqLen(pep) >= minsize && 
	       ajSeqLen(pep) <= maxsize)
		getorf_WriteORF(seq, len, seqlen, sense, find, orf_no,
				0, seqlen-1, ajSeqStr(pep), seqout,
				around);
	    ajSeqDel(&pep);
	}

	if(!GOTSTOP[1])
	{
	    /* translate frame 2 into pep */
	    pep = ajTrnSeqOrig(trnTable, seq, 2);
	    if(ajSeqLen(pep) >= minsize &&
	       ajSeqLen(pep) <= maxsize)
		getorf_WriteORF(seq, len, seqlen, sense, find, orf_no,
				1, seqlen-1, ajSeqStr(pep), seqout,
				around);
	    ajSeqDel(&pep);
	}

	if(!GOTSTOP[2])
	{
	    /* translate frame 3 into pep */
	    pep = ajTrnSeqOrig(trnTable, seq, 3);
	    if(ajSeqLen(pep) >= minsize && 
	       ajSeqLen(pep) >= maxsize)
		getorf_WriteORF(seq, len, seqlen, sense, find, orf_no,
				2, seqlen-1, ajSeqStr(pep), seqout,
				around);
	    ajSeqDel(&pep);
	}
    }

    for(i=0;i<3;++i)
	ajStrDel(&newstr[i]);

    return;
}




/* @funcstatic getorf_WriteORF ************************************************
**
** Undocumented.
**
** @param [?] seq [AjPSeq] Undocumented
** @param [?] len [ajint] Undocumented
** @param [?] seqlen [ajint] Undocumented
** @param [?] sense [AjBool] Undocumented
** @param [?] find [ajint] Undocumented
** @param [?] orf_no [ajint*] Undocumented
** @param [?] start [ajint] Undocumented
** @param [?] pos [ajint] Undocumented
** @param [?] str [AjPStr] Undocumented
** @param [?] seqout [AjPSeqout] Undocumented
** @param [?] around [ajint] Undocumented
** @@
******************************************************************************/

static void getorf_WriteORF(AjPSeq seq, ajint len, ajint seqlen, AjBool sense,
			    ajint find, ajint *orf_no, ajint start, ajint pos,
			    AjPStr str, AjPSeqout seqout, ajint around)
{
    AjPSeq new;
    AjPStr name  = NULL;       		/* name of the ORF */
    AjPStr value = NULL;       		/* string value of the ORF number */
    ajint s;
    ajint e;				/* start and end positions */
    AjPStr aroundstr = NULL;		/* holds sequence string around the
					   codon of interest */
    ajint codonpos = 0;			/* holds position of start of codon
					   of interest */

    s = start+1;
    e = pos+1;

    /*
    ** it is possible for an ORF in a circular genome to appear to start
    ** past the end of the genome.
    ** Move the reported positions back to start in the range 1..len
    ** for readability.
    */
    while(s > len)
    {
	s -= len;
	e -= len;
    }

    new = ajSeqNew();
    if(find == N_STOP2STOP || find == N_START2STOP ||
	find == AROUND_INIT_STOP || find == AROUND_END_STOP ||
	find == AROUND_START)
	ajSeqSetNuc(new);
    else
	ajSeqSetProt(new);


    /*
    ** Set the start and end positions to report and get the sequence for
    ** the AROUND* sequences
    */
    if(find == AROUND_INIT_STOP)
    {
	codonpos = s-3;
	s = codonpos - around;		/* 50 before the initial STOP */
	e = codonpos + around+2;	/* 50 after the end of the STOP */
	if(s < 1)
	    return;

	if(e > seqlen)
	    return;
	ajStrAssSub(&aroundstr, ajSeqStr(seq), s-1, e-1);

    }
    else if(find == AROUND_START)
    {
	codonpos = s;
	s = codonpos - around;		/* 50 before the initial STOP */
	e = codonpos + around+2;	/* 50 after the end of the STOP */
	if(s < 1)
	    return;

	if(e > seqlen)
	    return;
	ajStrAssSub(&aroundstr, ajSeqStr(seq), s-1, e-1);

    }
    else if(find == AROUND_END_STOP)
    {
	codonpos = e+1;
	s = codonpos - around;		/* 50 before the initial STOP */
	e = codonpos + around+2;	/* 50 after the end of the STOP */
	if(s < 1)
	    return;
				
	if(e > seqlen)
	    return;
	ajStrAssSub(&aroundstr, ajSeqStr(seq), s-1, e-1);
    }



    /* set the name and description */
    ajStrAss(&name, ajSeqGetName(seq));
    ajStrAppC(&name, "_");

    /* post-increment the ORF number for the next ORF */
    ajStrFromInt(&value,(*orf_no)++);
	
    ajStrApp(&name, value);
    ajSeqAssName(new, name);

    /* set the description of the translation */
    ajStrAssC(&name, "[");

    /* Reverse the reported positions if this is the reverse sense */
    if(!sense)
    {
	s = len-s+1;
	e = len-e+1;
    
        /*
	** shift the positions back into the range 1..len as far as possible
        ** without going into negative numbers
	*/
        while(e > len)
	{
            s -= len;	
            e -= len;	
        }

        while(e < 0 || s < 0)
	{
            s += len;	
            e += len;	
        }
    }

    /* the base before the stop codon (numbering bases from 1) */
    ajStrFromInt(&value, s);	
					   
    ajStrApp(&name, value);
    ajStrAppC(&name, " - ");

    /* the base before the stop codon (numbering bases from 1) */
    ajStrFromInt(&value, e);
					   

    ajStrApp(&name, value);
    ajStrAppC(&name, "] ");

    /* make it clear if this is the reverse sense */
    if(!sense)
        ajStrAppC(&name, "(REVERSE SENSE) ");    

    
    /*
    ** make it clear if this is a circular genome and the ORF crosses
    ** the breakpoint
    */
    if(s> len || e > len)
    	ajStrAppC(&name, "(ORF crosses the breakpoint) ");


    if(find == AROUND_INIT_STOP || find == AROUND_START ||
	find == AROUND_END_STOP)
    {
	ajStrAppC(&name, "Around codon at ");
	ajStrFromInt(&value, codonpos);
	ajStrApp(&name, value);
	ajStrAppC(&name, ". ");
    }

    ajStrApp(&name, ajSeqGetDesc(seq));
    ajSeqAssDesc(new, name);


    /* replace newstr in new */
    if(find == N_STOP2STOP || find == N_START2STOP || find == P_STOP2STOP ||
	find == P_START2STOP)
	ajSeqReplace(new, str);
    else
	/* sequence to be 50 bases around the codon */
	ajSeqReplace(new, aroundstr);

    ajSeqAllWrite(seqout, new);

    ajSeqDel(&new);
    ajStrDel(&value);
    ajStrDel(&name);

    return;
}




/* @funcstatic getorf_AppORF **************************************************
**
** append aa to ORF sequence string
**
** @param [?] find [ajint] Undocumented
** @param [?] str [AjPStr*] Undocumented
** @param [?] chrseq [char*] Undocumented
** @param [?] pos [ajint] Undocumented
** @param [?] aa [char] Undocumented
** @@
******************************************************************************/

static void getorf_AppORF(ajint find, AjPStr *str, char *chrseq, ajint pos,
			  char aa)
{

    if(find == N_STOP2STOP || find == N_START2STOP ||
	find == AROUND_INIT_STOP || find == AROUND_END_STOP)
    {
	ajStrAppK(str, chrseq[pos]);
	ajStrAppK(str, chrseq[pos+1]);
	ajStrAppK(str, chrseq[pos+2]);
    }
    else if(find == P_STOP2STOP || find == P_START2STOP ||
	    find == AROUND_START)
	ajStrAppK(str, aa);

    return;
}

/******************************************************************************
** @source AJAX ALIGN (ajax alignment output) functions
**
** These functions align AJAX sequences and report them in a variety
** of formats.
**
** @author Copyright (C) 2001 Peter Rice, LION Bioscience Ltd.
** @version 1.0
** @modified Aug 21 2001 First version
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "ajax.h"


/* @datastatic AlignPData *****************************************************
**
** Ajax Align Data object.
**
** Holds definition of feature alignment data. Stored in AjPAlign structure as
** an AjPList.
**
** @alias AlignSData
** @alias AlignOData
**
** @attr Nseqs [ajint] Number of sequences, size of each array
** @attr Start [ajint*] Start position for each sequence
** @attr End [ajint*] End position for each sequence
** @attr Offset [ajint*] Offset for numbering start of each sequence
** @attr SubOffset [ajint*] Offset of used subsequence within given sequence
**                          generally for cases where SeqExternal is set
**                          in the AjPAlign object that includes this
**                          AlignPData object.
** @attr Rev [AjBool*] Reverse each sequence (so far not used)
** @attr Seq [AjPSeq*] Sequences usually as copies of data from the
**                     application unless SeqExternal in the AjPAlign
**                     object is true
** @attr Len [ajint] Length of the alignment
** @attr NumId [ajint] Number of identical positions, usually calculated
** @attr NumSim [ajint] Number of similar positions, usually calculated
** @attr NumGap [ajint] Number of gap positions, usually calculated
** @attr Score [AjPStr] Score to be reported - stored as a string for output
**                      set by functions using ajint or float input
** @@
******************************************************************************/

typedef struct AlignSData
{
    ajint  Nseqs;		/* Number of sequences */
    ajint* Start;		/* Start position in original sequence */
    ajint* End;			/* End position in original sequence */
    ajint* Offset;		/* Offset for numbering start of sequence */
    ajint* SubOffset;		/* Substring offset within sequence */
    AjBool* Rev;		/* Reverse sequence? */
    AjPSeq* Seq;		/* Sequence data */
    ajint Len;			/* Alignment length */
    ajint NumId;		/* Number of identical positions */
    ajint NumSim;		/* Number of similar positions */
    ajint NumGap;		/* Number of gap positions */
    AjPStr Score;		/* Score statistic int/float as a string */
} AlignOData;

#define AlignPData AlignOData*




/* @datastatic AlignPFormat ***************************************************
**
** Ajax Align Formats
**
** List of alignment ouptut formats available
**
** @alias AlignSFormat
** @alias AlignOFormat
**
** @attr Name [char*] format name
** @attr Nuc [AjBool] ajTrue if format can work with nucleotide sequences
** @attr Prot [AjBool] ajTrue if format can work with protein sequences
** @attr MinSeq [ajint] Minimum number of sequences, 2 for pairwise
** @attr MaxSeq [ajint] Maximum number of sequences, 2 for pairwise
** @attr Write [(void*)] Function to write alignment
** @@
******************************************************************************/

typedef struct AlignSFormat
{
    char *Name;
    AjBool Nuc;
    AjBool Prot;
    ajint Minseq;
    ajint Maxseq;
    void (*Write) (AjPAlign thys);
} AlignOFormat;

#define AlignPFormat AlignOFormat*




static void       alignConsStats(AjPAlign thys, ajint iali, AjPStr *cons,
				 ajint* retident, ajint* retsim, ajint* retgap,
				 ajint* retlen);
static AlignPData alignData(const AjPAlign thys, ajint iali);
static void       alignDataDel(AlignPData* pthys, AjBool external);
static void       alignDiff(AjPStr* pmark, const AjPStr seq);
static ajint      alignLen(const AjPAlign thys, ajint iali);
static AjPSeq     alignSeq(const AjPAlign thys, ajint iseq, ajint iali);
static AjPStr     alignSeqName(const AjPAlign thys, ajint i);
static AjPSeq*    alignSeqs(const AjPAlign thys, ajint iali);
static void       alignSim(AjPStr* pmark, const char idch, const char simch,
			   const char misch, const char gapch);
static float      alignTotweight(const AjPAlign thys, ajint iali);
static void       alignTraceData(const AjPAlign thys);
static void       alignWriteFasta(AjPAlign thys);

static void       alignWriteMark(AjPAlign thys, ajint iali, ajint markx);
static void       alignWriteMarkX0(AjPAlign thys);
static void       alignWriteMarkX1(AjPAlign thys);
static void       alignWriteMarkX2(AjPAlign thys);
static void       alignWriteMarkX3(AjPAlign thys);
static void       alignWriteMarkX10(AjPAlign thys);
static void       alignWriteMatch(AjPAlign thys);
static void       alignWriteMsf(AjPAlign thys);
static void       alignWriteScore(AjPAlign thys);
static void       alignWriteSrs(AjPAlign thys);
static void       alignWriteSrsAny(AjPAlign thys, ajint imax, AjBool mark);
static void       alignWriteSrsPair(AjPAlign thys);
static void       alignWriteTCoffee(AjPAlign thys);

static void       alignWriteTrace(AjPAlign thys);
static void       alignWriteSimple(AjPAlign thys);




/* @funclist alignFormat ******************************************************
**
** Functions to write alignments
**
******************************************************************************/

static AlignOFormat alignFormat[] = {
  /* name       dna      protein min max function */
  /* standard sequence formats */
  {"unknown",   AJFALSE, AJFALSE, 0, 0, alignWriteSimple},
  {"fasta",     AJTRUE,  AJTRUE,  0, 0, alignWriteFasta},
  {"msf",       AJTRUE,  AJTRUE,  0, 0, alignWriteMsf},
  /* trace  for debug */
  {"trace",     AJTRUE,  AJTRUE,  0, 0, alignWriteTrace},
  /* alignment formats */
  {"markx0",    AJTRUE,  AJTRUE,  2, 2, alignWriteMarkX0},
  {"markx1",    AJTRUE,  AJTRUE,  2, 2, alignWriteMarkX1},
  {"markx2",    AJTRUE,  AJTRUE,  2, 2, alignWriteMarkX2},
  {"markx3",    AJTRUE,  AJTRUE,  2, 2, alignWriteMarkX3},
  {"markx10",   AJTRUE,  AJTRUE,  2, 2, alignWriteMarkX10},
  {"match",     AJTRUE,  AJTRUE,  2, 2, alignWriteMatch},
  {"multiple",  AJTRUE,  AJTRUE,  0, 0, alignWriteSimple},
  {"pair",      AJTRUE,  AJTRUE,  2, 2, alignWriteSimple},
  {"simple",    AJTRUE,  AJTRUE,  0, 0, alignWriteSimple},
  {"score",     AJTRUE,  AJTRUE,  0, 0, alignWriteScore},
  {"srs",       AJTRUE,  AJTRUE,  0, 0, alignWriteSrs},
  {"srspair",   AJTRUE,  AJTRUE,  2, 2, alignWriteSrsPair},
  {"tcoffee",   AJTRUE,  AJTRUE,  0, 0, alignWriteTCoffee},
  {NULL, 0, 0, 0, 0, NULL}
};




/* pair only works if the alignment is defined as 2 sequences */

/* other formats to be defined:
**
** markx1 .. markx10 as for FASTA (code in matcher.c)
** blast to be implemented
*/




/* @funcstatic alignWriteTrace ************************************************
**
** Writes an alignment in Trace format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteTrace(AjPAlign thys)
{
    ajFmtPrintF(thys->File, "Trace output\n");

    return;
}




/* @funcstatic alignWriteMsf **************************************************
**
** Writes an alignment in MSF format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMsf(AjPAlign thys)
{
    static AjPStr sqfmt = NULL;
    AjPSeqset seqset    = NULL;
    AjPSeqout seqout;

    ajint i;


    seqout = ajSeqoutNewF(thys->File);

    if(!sqfmt)
	ajStrAssC(&sqfmt, "msf");

    thys->SeqOnly = ajTrue;

    ajStrAssS(&seqout->Formatstr, sqfmt);
    seqout->File = thys->File;

    ajSeqoutOpen(seqout);

    seqset = ajSeqsetNew();
    for(i=0; i< thys->Nseqs; i++)
	ajSeqsetApp(seqset, alignSeq(thys, i, 0));

    ajSeqsetWrite(seqout, seqset);
    ajSeqWriteClose(seqout);
    seqout->File = NULL;

    ajSeqsetDel(&seqset);
    ajSeqoutDel(&seqout);

    return;
}




/* @funcstatic alignWriteFasta ************************************************
**
** Writes an alignment in FASTA format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteFasta(AjPAlign thys)
{
    static AjPStr sqfmt = NULL;
    AjPSeqset seqset    = NULL;
    AjPSeqout seqout;

    ajint i;


    seqout = ajSeqoutNewF(thys->File);

    if(!sqfmt)
	ajStrAssC(&sqfmt, "fasta");

    thys->SeqOnly = ajTrue;

    ajStrAssS(&seqout->Formatstr, sqfmt);
    seqout->File = thys->File;

    ajSeqoutOpen(seqout);

    seqset = ajSeqsetNew();
    for(i=0; i< thys->Nseqs; i++)
    {
	/* gap whitespace and change gaps from '-' to '.'*/
	if(!thys->SeqExternal)
	    ajSeqGapStandard(alignSeq(thys, i, 0), '.');
	ajSeqsetApp(seqset, alignSeq(thys, i, 0));
    }

    ajSeqsetWrite(seqout, seqset);
    ajSeqWriteClose(seqout);
    seqout->File = NULL;

    ajSeqsetDel(&seqset);
    ajSeqoutDel(&seqout);

    return;
}




/* @funcstatic alignWriteMarkX0 ***********************************************
**
** Writes an alignment in Fasta MarkX 0 format.
**
** This is the standard default output format for FASTA programs.
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX0(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListLength(thys->Data);
    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 0);

    return;
}




/* @funcstatic alignWriteMarkX1 ***********************************************
**
** Writes an alignment in Fasta MarkX 1 format.
**
** This is an alternative output format for FASTA programs in which
** identities are not marked.
** Instead conservative replacements are denoted by 'x'
** and non-conservative substitutions by 'X'.
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX1(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListLength(thys->Data);
    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 1);

    return;
}




/* @funcstatic alignWriteMarkX2 ***********************************************
**
** Writes an alignment in Fasta MarkX 2 format
**
** This is an alternative output format for FASTA programs in which
** the residues in the second sequence are only shown if they are
** different from the first.
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX2(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListLength(thys->Data);
    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 2);

    return;
}




/* @funcstatic alignWriteMarkX3 ***********************************************
**
** Writes an alignment in Fasta MarkX 3 format
**
** This is an alternative output format for FASTA programs in which
** the aligned library sequences are displayed in FASTA format
** These can be used to build a primitive multiple alignment.
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX3(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListLength(thys->Data);
    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 3);

    return;
}




/* @funcstatic alignWriteMarkX10 **********************************************
**
** Writes an alignment in Fasta MarkX 10 format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMarkX10(AjPAlign thys)
{
    ajint iali;
    ajint nali;

    nali = ajListLength(thys->Data);
    for(iali=0; iali < nali; iali++)
	alignWriteMark(thys, iali, 10);

    return;
}




/* @funcstatic alignWriteMark *************************************************
**
** Writes an alignment in a Fasta MarkX format.
**
** For now, seems to work with 0, 1, 2, 3 or 10.
** FASTA 3.4 has 4, 5, 6 and 9 as possible options
** but most seem to make no difference on pairwise comparisons.
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @param [r] markx [ajint] Markup type (as defined in Bill Pearson's
**                          FASTA suite
** @return [void]
** @@
******************************************************************************/

static void alignWriteMark(AjPAlign thys, ajint iali, ajint markx)
{    
    ajint maxout;
    
    static AjPStr cons = NULL;

    AjPSeqCvt cvt = NULL;	    /* cvt = ajMatrixCvt(matrix);   */
    ajint **sub   = NULL;	    /* sub = ajMatrixArray(matrix); */
    float **fsub  = NULL;	    /* sub = ajMatrixArray(matrix); */
    
    ajint llen;
    
    
    ajint nc;
    AjPSeq seq  = NULL;
    AjPSeq seq2 = NULL;
    
    ajint min0 = 0;
    ajint min1 = 0;
    ajint max0;
    ajint max1;
    
    char *line[3]  = {NULL, NULL, NULL}; /* [MAXOUT] */
    char *cline[2] = {NULL, NULL};	 /* [MAXOUT+10] */
    ajint il;
    ajint i;
    ajint lend;
    ajint loff;
    ajint il1;
    ajint il2;
    ajint del0 = 0;
    ajint del1 = 0;
    ajint ic;
    ajint ll0;
    ajint ll1;
    ajint ll01;
    ajint cl0;
    ajint cl1;
    ajint rl0;
    ajint rl1;
    ajint i00;
    ajint i0n;
    ajint i10;
    ajint i1n;
    ajint ioff0 = 0;
    ajint ioff1 = 0;
    ajlong qqoff;
    ajint lloff;
    ajint have_res;
    char *name01;
    char *name0= NULL;
    char *name1= NULL;
    ajint n0;
    ajint smark[4] = {-10000,-10000,-10000,-10000}; /* BIT WEIRD THIS */
    char *seqc0 = NULL;
    char *seqc1 = NULL;
    AlignPData data;
    
    AjPFile outf;
    ajint calcid;
    ajint calcsim;
    ajint calcgap;
    ajint calclen;
    ajint sbegin0;
    ajint sbegin1;
    
    ajDebug("alignWriteMark\n");
    
    llen = thys->Width;
    outf = thys->File;
    maxout = thys->Width+50;   /* alignment with plus names+numbers */


    for(i=0; i < 3; i++)
    {
	ajDebug("Resize line[%d] %d\n", i, maxout);
	AJCRESIZE(line[i], maxout);
    }
    
    for(i=0; i < 2; i++)
    {
	ajDebug("Resize cline[%d] %d\n", i, maxout+10);
	AJCRESIZE(cline[i],(maxout+10));
    }
    
    seq  = alignSeq(thys, 0, iali);
    seq2 = alignSeq(thys, 1, iali);
    data = alignData(thys, iali);
    
    name0 = ajSeqName(seq);
    name1 = ajSeqName(seq2);
    seqc0 = ajSeqChar(seq)+data->SubOffset[0];
    seqc1 = ajSeqChar(seq2)+data->SubOffset[1];
    nc = alignLen(thys, iali);
    n0 = ajSeqLen(seq);
    max0 = nc;
    max1 = nc;
    /*
       min0=data->Offset[0] + data->Start[0];
       min1=data->Offset[1] + data->Start[1];
       */
    min0=data->Offset[0];
    min1=data->Offset[1];
    sbegin0=data->Start[0];
    sbegin1=data->Start[1];
    ioff0=data->Offset[0];
    ioff1=data->Offset[1];
    
    ajDebug("min0:%d min1:%d\n", min0, min1);

    alignConsStats(thys, iali, &cons, &calcid, &calcsim, &calcgap, &calclen);
    ajAlignSetStats(thys, iali, nc, calcid, calcsim, calcgap, NULL);
    ajAlignSetSubStandard(thys, iali);
    ajAlignWriteHeader(thys);
    
    if(thys->IMatrix)
    {
	cvt = ajMatrixCvt(thys->IMatrix);
	sub = ajMatrixArray(thys->IMatrix);
    }
    else
    {
	cvt  = ajMatrixfCvt(thys->FMatrix);
	fsub = ajMatrixfArray(thys->FMatrix);
    }
    
    if(markx==2)
	name01 = name1;
    else
	name01 = "\0";
    
    i00 = smark[0];
    i0n = smark[1];
    i10 = smark[2];
    i1n = smark[3];
    
    /*(il) smins is always 0 ?? so why bother with this ??
      ioff0=smin0-smins;
      ioff1=smin1-smins;
      */
    
    
    if(markx==4)
    {
	for(i=0;i<3;++i)
	    AJFREE(line[i]);
	for(i=0;i<2;++i)
	    AJFREE(cline[i]);

	return;
    }
    
    
    if(markx==3)
    {
	ajFmtPrintF(outf,">%s ..\n",name0);
	for(i=0; i<nc; i++)
	{
	    ajFmtPrintF(outf, "%c",seqc0[i]);
	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}
	ajFmtPrintF(outf, "\n");
	ajFmtPrintF(outf,">%s ..\n",name1);
	for(i=0; i<nc; i++)
	{
	    ajFmtPrintF(outf, "%c", seqc1[i]);
	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}
	ajFmtPrintF(outf, "\n");

	for(i=0;i<3;++i)
	    AJFREE(line[i]);
	for(i=0;i<2;++i)
	    AJFREE(cline[i]);

	return;
    }
    
    if(markx==10)
    {
	ajFmtPrintF(outf,">%s ..\n",name0);
	ajFmtPrintF(outf,"; sq_len: %d\n",n0);
	/*    ajFmtPrintF(outf,"; sq_type: %c\n",sqtype[0]);*/
	ajFmtPrintF(outf,"; al_start: %d\n",
		    data->SubOffset[0]+min0+1);
	ajFmtPrintF(outf,"; al_stop: %d\n",
		    data->SubOffset[0]+max0);
	ajFmtPrintF(outf,"; al_display_start: %d\n",
		    data->SubOffset[0]+ioff0+1);
	
	have_res = 0;
	for(i=0; i<nc; i++)
	{
	    if(!have_res && seqc0[i]==' ')
		ajFmtPrintF(outf, "-");
	    else if(seqc0[i]==' ')
		break;
	    else
	    {
		have_res = 1;
		ajFmtPrintF(outf, "%c", seqc0[i]);
	    }
	    
	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}
	
	if((i-1)%50!=49 || seqc0[i-1]==' ')
	    ajFmtPrintF(outf, "\n");
	
	ajFmtPrintF(outf,">%s ..\n",name1);
	ajFmtPrintF(outf,"; sq_len: %d\n",ajSeqLen(seq2));
	/*    ajFmtPrintF(outf,"; sq_type: %c\n",sqtype[0]);*/
	ajFmtPrintF(outf,"; al_start: %ld\n",
		    data->SubOffset[1]+(ajlong)min1+1);
	ajFmtPrintF(outf,"; al_stop: %ld\n",
		    data->SubOffset[1]+(ajlong)max1);
	ajFmtPrintF(outf,"; al_display_start: %d\n",
		    data->SubOffset[1]+ioff1+1);
	
	have_res = 0;
	for(i=0; i<nc; i++)
	{
	    if(!have_res && seqc1[i]==' ')
		ajFmtPrintF(outf, "-");
	    else if(seqc1[i]==' ')
		break;
	    else
	    {
		have_res = 1;
		ajFmtPrintF(outf, "%c", seqc1[i]);
	    }
	    if(i%50 == 49)
		ajFmtPrintF(outf, "\n");
	}
	
	if((i-1)%50!=49 || seqc1[i-1]==' ')
	    ajFmtPrintF(outf, "\n");
	
	for(i=0;i<3;++i)
	    AJFREE(line[i]);
	for(i=0;i<2;++i)
	    AJFREE(cline[i]);
	
	return;
    }
    
    for(i=0; i<3; i++)
	memset(line[i],' ',maxout);
    
    ic = 0;
    del0=del1=0;
    for(il=0; il<(nc+llen-1)/llen; il++)
    {
	loff=il*llen;
	lend=AJMIN(llen,nc-loff);

	ll0 = ajFalse;
	ll1 = ajFalse;
	
	for(i=0; i<2; i++)
	    memset(cline[i],' ', maxout);
	
	for(i=0; i<lend; i++, ic++,ioff0++,ioff1++)
	{
	    cl0 =  cl1 = rl0 = rl1 = ajTrue;
	    if((line[0][i]=seqc0[ic])=='-')
	    {
		del0++;
		cl0 = rl0 =ajFalse;
	    }
	    if((line[2][i]=seqc1[ic])=='-')
	    {
		del1++;
		cl1 = rl1 = ajFalse;
	    }
	    
	    if(seqc0[ic]==' ')
	    {
		del0++;
		cl0 = rl0 = ajFalse;
	    }
	    else
		ll0 = ajTrue;
	    
	    if(seqc1[ic]==' ')
	    {
		del1++;
		cl1 = rl1 = ajFalse;
	    }
	    else
		ll1 = ajTrue;
	    
	    qqoff = sbegin0 - 1 + (ajlong)(ioff0-del0);
	    if(cl0 && qqoff%10 == 9)
	    {
		sprintf(&cline[0][i],"%8ld",(long)qqoff+1l);
		cline[0][i+8] = ' ';
		rl0 = ajFalse;
	    }
	    else if(cl0 && qqoff== -1)
	    {
		sprintf(&cline[0][i],"%8ld",0l);
		cline[0][i+8] = ' ';
		rl0 = ajFalse;
	    }
	    else if(rl0 && (qqoff+1)%10 == 0)
	    {
		sprintf(&cline[0][i],"%8ld",(long)qqoff+1);
		cline[0][i+8] = ' ';
	    }
	    
	    lloff = sbegin1-1 +	   /*loffset +*/ (ajlong)(ioff1-del1);
	    if(cl1 && lloff%10 == 9)
	    {
		sprintf(&cline[1][i],"%8ld",(long)lloff+1l);
		cline[1][i+8] = ' ';
		rl1 = ajFalse;
	    }
	    else if(cl1 && lloff== -1)
	    {
		sprintf(&cline[1][i],"%8ld",0l);
		cline[1][i+8] = ' ';
		rl1 = ajFalse;
	    }
	    else if(rl1 && (lloff+1)%10 == 0)
	    {
		sprintf(&cline[1][i],"%8ld",(long)lloff+1);
		cline[1][i+8] = ' ';
	    }
	    
	    
	    line[1][i] = ' ';
	    if(ioff0-del0 >= min0 && ioff0-del0 <= max0)
	    {
		if(toupper((ajint)line[0][i])==toupper((ajint)line[2][i]))
		    switch(markx)
		    {
		    case 0:
			line[1][i] = ':';
			break;
		    case 1:
			line[1][i] = ' ';
			break;
		    case 2:
			line[1][i] = '.';
			break;
		    }
		else if(markx==2)
		    line[1][i]=line[2][i];
		else if((il1 = ajSeqCvtK(cvt, line[0][i])) &&
			(il2 = ajSeqCvtK(cvt, line[2][i])) &&
			thys->IMatrix &&
			sub[il1][il2]>= 0)
		    line[1][i]= (markx) ? 'x':'.';
		else if((il1 = ajSeqCvtK(cvt, line[0][i])) &&
			(il2 = ajSeqCvtK(cvt, line[2][i])) &&
			thys->FMatrix &&
			fsub[il1][il2]>= 0.0)
		    line[1][i]= (markx) ? 'x':'.';
		else if((il1 = ajSeqCvtK(cvt, line[0][i])) &&
			(il2 = ajSeqCvtK(cvt, line[2][i])))
		    line[1][i]= (markx) ? 'X':' ';
	    }
	    else if(markx==2)
		line[1][i]=line[2][i];
	    
	    if(markx==0)
	    {
		if(ioff0-del0 == i00 && ioff1-del1 == i10)
		{
		    line[1][i] = 'X';
		    i00 = i10  = -1;
		}
		if(ioff0-del0 == i0n && ioff1-del1 == i1n)
		{
		    line[1][i] = 'X';
		    i0n = i1n  = -1;
		}

		if((ioff0-del0 == i00) || (ioff0-del0 == i0n))
		{
		    line[1][i] = '^';
		    if(ioff0-del0 == i00)
			i00 = -1;
		    else
			i0n = -1;
		}

		if(ioff1-del1 == i10 || ioff1-del1 == i1n)
		{
		    line[1][i] = 'v';
		    if(ioff1-del1 == i10)
			i10 = -1;
		    else
			i1n = -1;
		}
	    }
	}
	
	for(i=0; i<3; i++)
	    line[i][lend] = 0;
	
	for(i=0; i<2; i++)
	    cline[i][lend+7] = 0;
	
	ll01 = ll0&&ll1;
	if(markx==2 && (ll0))
	    ll1 = 0;
	ajFmtPrintF(outf,"\n");
	if(ll0)
	    ajFmtPrintF(outf,"%s\n",cline[0]);
	if(ll0)
	    ajFmtPrintF(outf,"%6.6s %s\n",name0,line[0]);
	if(ll01)
	    ajFmtPrintF(outf,"%-6.6s %s\n",name01,line[1]);
	if(ll1)
	    ajFmtPrintF(outf,"%6.6s %s\n",name1,line[2]);
	if(ll1)
	    ajFmtPrintF(outf,"%s\n",cline[1]);
    }
    
    for(i=0;i<3;++i)
	AJFREE(line[i]);
    for(i=0;i<2;++i)
	AJFREE(cline[i]);
    
    return;
}




/* @funcstatic alignWriteMatch ************************************************
**
** Writes an alignment in Match format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteMatch(AjPAlign thys)
{
    AjPFile outf;
    ajint nali;
    ajint iali;
    ajint len0;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    outf = thys->File;
    nali = ajListToArray(thys->Data, (void***) &pdata);

    ajAlignWriteHeader(thys);

    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	len0 = data->End[0] - data->Start[0] + 1;
	ajFmtPrintF(outf, "%6d %-15.15S %8d..%-8d %-15.15S %8d..%d\n",
		    len0,
		    alignSeqName(thys, 0), data->Start[0], data->End[0],
		    alignSeqName(thys, 1), data->Start[1], data->End[1]);
    }

    AJFREE(pdata);

    return;
}




/* @funcstatic alignWriteSimple ***********************************************
**
** Writes an alignment in Simple format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSimple(AjPAlign thys)
{
    AjPFile outf;
    int nseq;
    int nali;

    AjPStr seq = NULL;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    ajint iali;
    ajint iseq;
    ajint i;
    ajint istart;
    ajint iend;
    ajint ilen;
    ajint iwidth = 50;

    AjPStr tmpstr  = NULL;
    AjPStr mrkstr  = NULL;
    AjPStr mrkcons = NULL;
    AjPStr cons    = NULL;

    ajint identity   = 0;
    ajint similarity = 0;
    ajint gaps       = 0;
    ajint seqlen     = 0;
    ajint icnt;

    ajint* ipos   = NULL;
    AjPStr tmphdr = NULL;
    
    ajDebug("alignWriteSimple\n");


    nseq = thys->Nseqs;
    outf = thys->File;
    
    if(thys->Width)
	iwidth = thys->Width;
    
    nali = ajListToArray(thys->Data, (void***) &pdata);
    
    AJCNEW0(ipos, nseq);
    
    for(iali=0; iali<nali; iali++)
    {
	ajStrDel(&cons);
	ajStrDel(&tmphdr);
	
	data = pdata[iali];
	ilen = data->Len;
	
	alignConsStats(thys, iali, &cons,
		       &identity, &similarity, &gaps, &seqlen);
	
	ajAlignSetStats(thys, iali, seqlen, identity, similarity, gaps, NULL);
	ajAlignSetSubStandard(thys, iali);
	ajAlignWriteHeader(thys);
	
	ajDebug("# Consens: '%S'\n\n", cons);
	
	alignSim(&cons, '|', ':', '.', ' ');
	
	ajDebug("# Modcons: '%S'\n\n", cons);
	ajDebug("# Nali:%d nseq:%d\n", nali, nseq);
	
	ajDebug("# AliData [%d] len %d \n", iali, ilen);
	for(iseq=0; iseq < nseq; iseq++)
	{
	    /*      ipos[iseq]=data->Offset[iseq] + data->Start[iseq]; */
	    ipos[iseq]=data->Offset[iseq] + data->Start[iseq]-1;
	    ajDebug("#   Seq[%d]'%S'\n",
		    iseq, ajSeqStr(data->Seq[iseq]));
	}
	for(iseq=0; iseq < nseq; iseq++)
	{
	    ajDebug("#   Seq[%d]  Len:%d Start:%d End:%d Rev:%B\n",
		    iseq, ajSeqLen(data->Seq[iseq]),
		    data->Start[iseq], data->End[iseq],
		    data->Rev[iseq]);
	}
	
	for(i=0; i < ilen; i += iwidth)
	{	    
	    for(iseq=0; iseq < nseq; iseq++)
	    {
		seq = ajSeqStr(data->Seq[iseq]);
		istart = i + data->SubOffset[iseq];
		iend = AJMIN(data->SubOffset[iseq]+ilen-1, istart+iwidth-1);
		ajStrAssSub(&tmpstr, seq, istart, iend);
		ajStrAssSub(&mrkcons, cons,
			    istart - data->SubOffset[iseq],
			    iend - data->SubOffset[iseq]);
		
		ajStrSubstituteCC(&tmpstr, ".", "-");
		icnt = ajStrLen(tmpstr)
		    - ajStrCountK(tmpstr, '-')
			- ajStrCountK(tmpstr, ' ');
		
		if(!iseq)
		    ajStrAssS(&mrkstr, tmpstr);
		else
		    alignDiff(&mrkstr, tmpstr);

		
		if(nseq==2 && iseq==1) /* 2 seqs, markup between them */
		    ajFmtPrintF(outf,
				"                     %S\n",
				mrkcons);

		if(ajStrLen(tmpstr))
		{
		    ajFmtPrintF(outf,
				"%-13.13S %6d %S %6d\n",
				alignSeqName(thys, iseq),
				ipos[iseq]+1, tmpstr, ipos[iseq]+icnt);
		}
		else
		    ajFmtPrintF(outf,
				"%-13.13S\n",
				alignSeqName(thys, iseq));

		ipos[iseq] += icnt;
	    }

	    if(nseq > 2) 	    /* 3 or more seqs, markup under */
		ajFmtPrintF(outf,
			    "                     %S\n",
			    mrkcons);

	    ajFmtPrintF(outf, "\n");
	}
    }
    
    
    ajStrDel(&cons);
    ajStrDel(&mrkcons);
    ajStrDel(&mrkstr);
    ajStrDel(&tmpstr);
    AJFREE(ipos);
    AJFREE(pdata);
    
    return;
}




/* @funcstatic alignWriteScore ************************************************
**
** Writes an alignment in Score-only format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteScore(AjPAlign thys)
{
    AjPFile outf;
    ajint nali;
    ajint iali;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;


    outf = thys->File;
    nali = ajListToArray(thys->Data, (void***) &pdata);

    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];

	if(ajStrLen(data->Score))
	    ajFmtPrintF(outf, "%S %S %d (%S)\n",
			alignSeqName(thys, 0), alignSeqName(thys, 1),
			data->Len, data->Score);
	else
	    ajFmtPrintF(outf, "%S %S %d\n",
			alignSeqName(thys, 0), alignSeqName(thys, 1),
			data->Len);
    }

    AJFREE(pdata);

    return;
}




/* @funcstatic alignWriteSrs **************************************************
**
** Writes an alignment in Srs format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSrs(AjPAlign thys)
{
    alignWriteSrsAny(thys, 0, ajFalse);

    return;
}




/* @funcstatic alignWriteSrsPair **********************************************
**
** Writes an alignment in SrsPair format
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteSrsPair(AjPAlign thys)
{
    alignWriteSrsAny(thys, 2, ajTrue);

    return;
}




/* @funcstatic alignWriteSrsAny ***********************************************
**
** Writes an alignment in Srs format (with switches for pairwise and general
** formatting)
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] imax [ajint] Maximum number of sequences (0 for unknown)
** @param [r] mark [AjBool] Markup the alignment
** @return [void]
** @@
******************************************************************************/

static void alignWriteSrsAny(AjPAlign thys, ajint imax, AjBool mark)
{    
    AjPFile outf;
    int nseq;
    int nali;

    AjPStr seq = NULL;

    AlignPData* pdata = NULL;
    AlignPData data = NULL;

    ajint iali;
    ajint iseq;
    ajint i;
    ajint istart;
    ajint iend;
    ajint ilen;

    AjPStr tmpstr  = NULL;
    AjPStr mrkstr  = NULL;
    AjPStr mrkcons = NULL;
    AjPStr cons    = NULL;

    ajint iwidth     = 50;
    ajint identity   = 0;
    ajint similarity = 0;
    ajint gaps=0;
    ajint seqlen=0;
    ajint icnt;

    ajint* ipos = NULL;
    AjBool pair = ajFalse;
    
    AjPStr tmphdr = NULL;


    outf = thys->File;
    nseq = thys->Nseqs;
    
    if(nseq < 1)
    {
	ajAlignWriteHeader(thys);

	return;
    }
    
    if(thys->Width)
	iwidth = thys->Width;
    
    /*
     ** pair: if true, change consensus into |:. markup
     ** and put markup line between sequences
     */
    
    if(imax == 2 && nseq == 2 && mark)
	pair = ajTrue;
    
    nali = ajListToArray(thys->Data, (void***) &pdata);
    
    AJCNEW0(ipos, nseq);
    
    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	ilen = data->Len;
	
	ajStrDel(&cons);
	ajStrDel(&tmphdr);
	alignConsStats(thys, iali, &cons,
		       &identity, &similarity, &gaps, &seqlen);
	
	ajAlignSetStats(thys, iali, seqlen, identity, similarity, gaps,
			NULL);
	ajAlignSetSubStandard(thys, iali);
	ajAlignWriteHeader(thys);
	
	ajDebug("# Consens: '%S'\n\n", cons);
	
	if(pair)
	    alignSim(&cons, '|', ':', '.', ' ');
	
	ajDebug("# Modcons: '%S'\n\n", cons);
	ajDebug("# Nali:%d nseq:%d\n", nali, nseq);
	
	ajDebug("# AliData [%d] len %d \n", iali, ilen);
	
	for(iseq=0; iseq < nseq; iseq++)
	{
	    ipos[iseq]=data->Offset[iseq] + data->Start[iseq]-1;
	    ajDebug("#   Seq[%d] Off:%d Sta:%d End:%d '%S'\n",
		    iseq, data->Offset[iseq], data->Start[iseq],
		    data->End[iseq], ajSeqStr(data->Seq[iseq]));
	}
	
	for(iseq=0; iseq < nseq; iseq++)
	    ajDebug("#   Seq[%d] Start:%d End:%d Rev:%B\n",
		    iseq, data->Start[iseq], data->End[iseq],
		    data->Rev[iseq]);
	
	for(i=0; i < ilen; i += iwidth)
	{	    
	    for(iseq=0; iseq < nseq; iseq++)
	    {
		seq = ajSeqStr(data->Seq[iseq]);
		istart = i + data->SubOffset[iseq];
		iend = AJMIN(data->SubOffset[iseq]+ilen-1, istart+iwidth-1);
		ajStrAssSub(&tmpstr, seq, istart, iend);
		ajStrAssSub(&mrkcons, cons,
			    istart - data->SubOffset[iseq],
			    iend - data->SubOffset[iseq]);

		ajStrSubstituteCC(&tmpstr, ".", "-");
		icnt = ajStrLen(tmpstr)
		    - ajStrCountK(tmpstr, '-')
			- ajStrCountK(tmpstr, ' ');

		if(!iseq)
		    ajStrAssS(&mrkstr, tmpstr);
		else
		    alignDiff(&mrkstr, tmpstr);

		if(pair && iseq==1)  /* 2 seqs, markup between them */
		    ajFmtPrintF(outf,
				"                     %S\n",
				mrkcons);

		if(ajStrLen(tmpstr))
		    ajFmtPrintF(outf,
				"%-13.13S %6d %S %6d\n",
				alignSeqName(thys, iseq),
				ipos[iseq]+1, tmpstr, ipos[iseq]+icnt);
		else
		    ajFmtPrintF(outf,
				"%-13.13S\n",
				alignSeqName(thys, iseq));

		ipos[iseq] += icnt;
	    }

	    if(mark && !pair)	    /* 3 or more seqs, markup under */
		ajFmtPrintF(outf,
			    "                     %S\n",
			    mrkcons);

	    ajFmtPrintF(outf, "\n");
	}
    }
    
    ajStrDel(&mrkcons);
    ajStrDel(&tmpstr);
    ajStrDel(&mrkstr);
    ajStrDel(&cons);
    AJFREE(ipos);
    AJFREE(pdata);
    
    return;
}


/* @funcstatic alignWriteTCoffee **********************************************
**
** Writes an alignment as a T-COFFEE library
**
** @param [R] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

static void alignWriteTCoffee (AjPAlign thys)
{
    AjPFile outf = thys->File;
    int nseq;
    int nali;
    AlignPData* pdata;
    AlignPData data;
    ajint iali;
    ajint iseq;
    ajint i;
    ajint s1,s2;
    ajint n1,n2;
    ajint ilen;
    AjPStr sseq= NULL;
    AjPStr sseq1= NULL;
    AjPStr sseq2= NULL;
    ajint pidentity=0;

    ajDebug("alignWriteTCoffee\n");

    nseq = thys->Nseqs;
    nali = ajListToArray (thys->Data, (void***) &pdata);
    thys->SeqOnly = ajTrue; /* suppress output of tail */

    /* print header */
    ajFmtPrintF (outf, "%d\n", nseq); /* number of sequences */
    for (iseq=0; iseq < nseq; iseq++) {
	ajStrAss (&sseq,ajSeqStr(pdata[0]->Seq[iseq]));
	ajStrCleanWhite (&sseq);
	ajStrSubstituteCC (&sseq, "-","");
	/* <seqname> <seqlen> <sequence> */
	ajFmtPrintF (outf, "%S %d %S\n",
		     alignSeqName(thys, iseq),
		     ajStrLen(sseq),
		     sseq);
	ajStrDel(&sseq);
    }


    for (iali = 0; iali<nali; iali++)
    {
	data = pdata[iali];
	ilen = data->Len;
	pidentity = (int)(0.5 + 100.0 *
			  (float) data->NumId / (float) data->Len);
	ajFmtPrintF (outf, "! score=%S\n",data->Score);
	ajFmtPrintF (outf, "! matrix=%S\n",thys->Matrix);
	ajFmtPrintF (outf, "! gapopen=%S gapext=%S\n",
		     thys->GapPen,thys->ExtPen);
	/* go through all pairwise alignments */
	for (s1=0; s1<nseq-1; s1++)
	{
	    sseq1 = ajSeqStr(pdata[0]->Seq[s1]);
	    for (s2=s1+1; s2<nseq; s2++) 
	    {
		ajFmtPrintF (outf, "#%d %d\n", s1+1, s2+1);
		n1 = 1; n2 = 1;
		sseq2 = ajSeqStr(pdata[0]->Seq[s2]);

		for (i=0; i<ilen; i++)
		{
		    /* check if position is ungapped */
		    if (ajStrChar(sseq1,i)>='A' && ajStrChar(sseq2,i)>='A')
		    {
			/* output aligned pair,
			   sequence weight,
			   residue weight,
			   match weight
			   (format as guessed from the T-coffee docs) */
			ajFmtPrintF (outf, "%d %d %d %d %d\n", 
				     n1, n2, pidentity, 1, 0);
		    }
		    if (ajStrChar(sseq1,i)>='A')
		    {
			n1++;
		    }
		    if (ajStrChar(sseq2,i)>='A')
		    {
			n2++;
		    }
		}
	    }
	}
    }
    /* write footer */
    ajFmtPrintF (outf, "! SEQ_1_TO_N\n");

    AJFREE (pdata);

    return;
}

/* @func ajAlignDefine ********************************************************
**
** Defines a sequence set as an alignment
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] seqset [AjPSeqset] Sequence set object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignDefine(AjPAlign thys, AjPSeqset seqset)
{    
    AlignPData data = NULL;
    ajint i;
    
    AJNEW0(data);
    
    if(!thys->Nseqs)
	thys->Nseqs = ajSeqsetSize(seqset);

    
    data->Nseqs = thys->Nseqs;
    
    AJCNEW0(data->Start, thys->Nseqs);
    AJCNEW0(data->End, thys->Nseqs);
    AJCNEW0(data->Offset, thys->Nseqs);
    AJCNEW0(data->SubOffset, thys->Nseqs);
    AJCNEW0(data->Rev, thys->Nseqs);
    AJCNEW0(data->Seq, thys->Nseqs);
    
    for(i=0; i < thys->Nseqs; i++)
    {
	data->Start[i]  = 1;
	data->End[i]    = ajSeqLen(ajSeqsetGetSeq(seqset, i));
	data->Offset[i] = 0;
	data->Rev[i]    = ajFalse;
	if(thys->SeqExternal)
	    data->Seq[i] = ajSeqsetGetSeq(seqset, i);
	else
	{
	    data->Seq[i] = ajSeqNewS(ajSeqsetGetSeq(seqset, i));
	    ajSeqGapStandard(data->Seq[i], '-');
	}
    }
    
    data->Len = ajSeqsetLen(seqset);
    
    ajListPushApp(thys->Data, data);
    
    return ajTrue;
}




/* @func ajAlignDefineSS ******************************************************
**
** Defines a sequence pair as an alignment
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] seqa [AjPSeq] Sequence object
** @param [r] seqb [AjPSeq] Second sequence object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignDefineSS(AjPAlign thys, AjPSeq seqa, AjPSeq seqb)
{
    AlignPData data = NULL;


    AJNEW0(data);

    if(!thys->Nseqs)
	thys->Nseqs = 2;

    AJCNEW0(data->Start, 2);
    AJCNEW0(data->End, 2);
    AJCNEW0(data->Offset, 2);
    AJCNEW0(data->SubOffset, 2);
    AJCNEW0(data->Rev, 2);
    AJCNEW0(data->Seq, 2);

    data->Start[0] = 1;
    data->End[0] = ajSeqLen(seqa);
    data->Offset[0] = 0;
    data->Rev[0] = ajFalse;
    if(thys->SeqExternal)
	data->Seq[0] = seqa;
    else
    {
	data->Seq[0] = ajSeqNewS(seqa);
	ajSeqGapStandard(data->Seq[0], '-');
    }

    data->Start[1] = 1;
    data->End[1] = ajSeqLen(seqb);
    data->Offset[1] = 0;
    data->Rev[1] = ajFalse;
    if(thys->SeqExternal)
	data->Seq[1] = seqb;
    else
    {
	data->Seq[1] = ajSeqNewS(seqb);
	ajSeqGapStandard(data->Seq[1], '-');
    }

    data->Len = AJMIN(ajSeqLen(seqa), ajSeqLen(seqb));

    ajListPushApp(thys->Data, data);

    return ajTrue;
}




/* @func ajAlignDel ***********************************************************
**
** Destructor for Alignment objects
**
** @param [d] pthys [AjPAlign*] Alignment object reference
** @return [void]
** @@
******************************************************************************/

void ajAlignDel(AjPAlign* pthys)
{
    AlignPData data = NULL;
    AjPAlign thys;

    thys = *pthys;

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Formatstr);
    ajStrDel(&thys->Extension);
    ajStrDel(&thys->Type);
    ajStrDel(&thys->Usa);
    ajStrDel(&thys->SubHeader);
    ajStrDel(&thys->Header);
    ajStrDel(&thys->Tail);
    ajStrDel(&thys->Matrix);
    ajStrDel(&thys->GapPen);
    ajStrDel(&thys->ExtPen);

    ajMatrixDel(&thys->IMatrix);
    ajMatrixfDel(&thys->FMatrix);

    while(ajListPop(thys->Data, (void**) &data))
	alignDataDel(&data, thys->SeqExternal);

    ajListFree(&thys->Data);

    AJFREE(*pthys);

    return;
}




/* @func ajAlignReset *********************************************************
**
** Reset for Alignment objects
**
** @param [d] thys [AjPAlign] Alignment object reference
** @return [void]
** @@
******************************************************************************/

void ajAlignReset(AjPAlign thys)
{
    AlignPData data = NULL;

    while(ajListPop(thys->Data, (void**) &data))
	alignDataDel(&data, thys->SeqExternal);

    ajListFree(&thys->Data);
    thys->Data = ajListNew();

    thys->Nseqs = 0;

    return;
}




/* @func ajAlignOpen **********************************************************
**
** Opens a new align file
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] name [const AjPStr] File name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignOpen(AjPAlign thys, const AjPStr name)
{
    if(!ajAlignValid(thys))
	return ajFalse;

    thys->File = ajFileNewOut(name);
    if(thys->File)
	return ajTrue;

    return ajFalse;
}




/* @func ajAlignFormatDefault *************************************************
**
** Sets the default format for an alignment
**
** @param [w] pformat [AjPStr*] Default format returned
** @return [AjBool] ajTrue is format was returned
** @@
******************************************************************************/

AjBool ajAlignFormatDefault(AjPStr* pformat)
{
    if(ajStrLen(*pformat))
	ajDebug("... output format '%S'\n", *pformat);
    else
    {
	/* ajStrSetC(pformat, alignFormat[0].Name);*/
	ajStrSetC(pformat, "gff");	/* use the real name */
	ajDebug("... output format not set, default to '%S'\n", *pformat);
    }

    return ajTrue;
}




/* @func ajAlignFindFormat ****************************************************
**
** Looks for the specified align format in the internal definitions and
** returns the index.
**
** @param [r] format [const AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajAlignFindFormat(const AjPStr format, ajint* iformat)
{
    AjPStr tmpformat = NULL;
    ajint i = 0;

    ajDebug("ajAlignFindFormat '%S'\n", format);

    if(!ajStrLen(format))
	return ajFalse;

    ajStrAssS(&tmpformat, format);
    ajStrToLower(&tmpformat);

    while(alignFormat[i].Name)
    {
	if(ajStrMatchCaseC(tmpformat, alignFormat[i].Name))
	{
	    *iformat = i;
	    ajStrDel(&tmpformat);
	    ajDebug("... found at %d\n", i);
	    return ajTrue;
	}
	i++;
    }

    ajDebug("... not found\n");
    ajStrDel(&tmpformat);
    return ajFalse;
}




/* @func ajAlignValid *********************************************************
**
** Test for an alignment object.
**
** Checks the format works with the number of sequences.
** Checks the format works with the type (protein or nucleotide).
**
** @param [r] thys [AjPAlign] Alignment object
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajAlignValid(AjPAlign thys)
{
    ajDebug("ajAlignValid format '%S' %d Nmin %d Nmax %d\n",
	    thys->Formatstr, thys->Format, thys->Nmin, thys->Nmax);

    if(!thys->Format)			/* test acdc-alignbadformat */
	if(!ajAlignFindFormat(thys->Formatstr, &thys->Format))
	{
	    ajErr("Unknown alignment format '%S'", thys->Formatstr);
	    return ajFalse;
	}

    if( alignFormat[thys->Format].Minseq  &&
       thys->Nmin < alignFormat[thys->Format].Minseq)
    {
	ajErr("Alignment format specifies at least %d sequences, "
	      "alignment has only %d",
	      alignFormat[thys->Format].Minseq, thys->Nmin);
	return ajFalse;
    }

    if( alignFormat[thys->Format].Maxseq  &&
       thys->Nmax > alignFormat[thys->Format].Maxseq)
    {
	ajErr("Alignment format specifies at most %d sequences, "
	      "alignment has %d",
	      alignFormat[thys->Format].Maxseq, thys->Nmax);
	return ajFalse;
    }

    if(thys->Width < 10)
    {
	ajWarn("Alignment width (-awidth=%d) too narrow, reset to 10",
	       thys->Width);
	thys->Width=10;
    }

    return ajTrue;
}




/* @func ajAlignNew ***********************************************************
**
** Constructor for an alignment object
**
** @return [AjPAlign] New Alignment object
** @@
******************************************************************************/

AjPAlign ajAlignNew(void)
{
    AjPAlign pthis;

    AJNEW0(pthis);

    pthis->Count     = 0;
    pthis->Name      = ajStrNew();
    pthis->Formatstr = ajStrNew();
    pthis->Format    = 0;
    pthis->Extension = ajStrNew();
    pthis->File      = NULL;
    pthis->Data      = ajListNew();

    return pthis;
}




/* @func ajAlignWrite *********************************************************
**
** Writes an alignment file
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignWrite(AjPAlign thys)
{
    ajDebug("ajAlignWrite\n");

    ajAlignTrace(thys);

    if(!thys->Format)
	if(!ajAlignFindFormat(thys->Formatstr, &thys->Format))
	    ajErr("unknown align format '%S'", thys->Formatstr);

    ajDebug("ajAlignWrite %d '%s'\n",
	    thys->Format, alignFormat[thys->Format].Name);

    ajAlignSetType(thys);

    /* Calling funclist alignFormat() */

    alignFormat[thys->Format].Write(thys);

    return;
}




/* @func ajAlignClose *********************************************************
**
** Closes an alignment
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignClose(AjPAlign thys)
{
    ajDebug("ajAlignClose '%F'\n", thys->File);

    if(!thys->SeqOnly)
	ajAlignWriteTail(thys);

    ajFileClose(&thys->File);

    return;
}




/* @func ajAlignWriteHeader ***************************************************
**
** Writes an alignment header
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignWriteHeader(AjPAlign thys)
{
    AjPFile outf;
    AjPStr tmpstr = NULL;
    AjBool doSingle;
    ajint i;

    outf     = thys->File;
    doSingle = ajFalse;	/* turned off for now - always multi format */    

    if(!thys->Count)
    {
	ajFmtPrintF(outf, "########################################\n");
	ajFmtPrintF(outf, "# Program:  %s\n", ajAcdProgram());
	ajFmtPrintF(outf, "# Rundate:  %D\n", ajTimeTodayRefF("log"));
	ajFmtPrintF(outf, "# Align_format: %S\n", thys->Formatstr);
	ajFmtPrintF(outf, "# Report_file: %F\n", outf);
	if(!doSingle || thys->Multi)
	    ajFmtPrintF(outf, "########################################\n");
	else
	    ajFmtPrintF(outf, "#\n");
    }
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#=======================================\n#\n");
    
    ajFmtPrintF(outf, "# Aligned_sequences: %d\n", thys->Nseqs);
    for(i=0; i < thys->Nseqs; i++)
    {
	ajStrAssC(&tmpstr, "");
	if(thys->Showacc)
	    ajFmtPrintAppS(&tmpstr, " (%S)",
			   ajSeqGetAcc(alignSeq(thys,i, 0)));
	if(thys->Showdes)
	    ajFmtPrintAppS(&tmpstr, " %S",
			   ajSeqGetDesc(alignSeq(thys,i, 0)));
	ajFmtPrintF(outf, "# %d: %S%S\n",
		    i+1, alignSeqName(thys, i), tmpstr);
    }

    if(ajStrLen(thys->Matrix))
	ajFmtPrintF(outf, "# Matrix: %S\n", thys->Matrix);
    
    if(ajStrLen(thys->GapPen))
	ajFmtPrintF(outf, "# Gap_penalty: %S\n", thys->GapPen);
    
    if(ajStrLen(thys->ExtPen))
	ajFmtPrintF(outf, "# Extend_penalty: %S\n", thys->ExtPen);
    
    if(ajStrLen(thys->Header))
    {
	ajStrAssS(&tmpstr, thys->Header);
	ajStrSubstituteCC(&tmpstr, "\n", "\1# ");
	ajStrSubstituteCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
    }
    
    if(ajStrLen(thys->SubHeader))
    {
	ajStrAssS(&tmpstr, thys->SubHeader);
	ajStrSubstituteCC(&tmpstr, "\n", "\1# ");
	ajStrSubstituteCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
	ajStrDel(&thys->SubHeader);
    }
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#=======================================\n\n");
    else
	ajFmtPrintF(outf, "########################################\n\n");
    
    ++thys->Count;
    
    ajStrDel(&tmpstr);
    
    return;
}




/* @func ajAlignWriteTail *****************************************************
**
** Writes an alignment tail
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignWriteTail(AjPAlign thys)
{
    AjPFile outf;
    AjPStr tmpstr = NULL;
    AjBool doSingle;

    outf     = thys->File;
    doSingle = ajFalse; /* turned off for now - always multi format */
    
    ajFmtPrintF(outf, "\n");
    
    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#---------------------------------------\n");
    else
	ajFmtPrintF(outf, "\n########################################\n");
    
    if(ajStrLen(thys->Tail))
    {
	ajStrAssS(&tmpstr, thys->Tail);
	ajStrSubstituteCC(&tmpstr, "\n", "\1# ");
	ajStrSubstituteCC(&tmpstr, "\1", "\n");
	ajFmtPrintF(outf, "#\n");
	ajFmtPrintF(outf, "# %S\n", tmpstr);
	ajFmtPrintF(outf, "#\n");
    }

    if(!doSingle || thys->Multi)
	ajFmtPrintF(outf, "#---------------------------------------\n");
    else
	ajFmtPrintF(outf, "########################################\n");

    ajStrDel(&tmpstr);

    return;
}




/* @func ajAlignSetHeader *****************************************************
**
** Defines an alignment header
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] header [const AjPStr] Align header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetHeader(AjPAlign thys, const AjPStr header)
{
    ajStrAssS(&thys->Header, header);

    ajDebug("ajAlignSetHeader len %d '%S'\n",
	    ajStrLen(thys->Header), header);

    return;
}




/* @func ajAlignSetHeaderC ****************************************************
**
** Defines an alignment header
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] header [const char*] Align header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetHeaderC(AjPAlign thys, const char* header)
{
    ajStrAssC(&thys->Header, header);

    ajDebug("ajAlignSetHeaderC len %d '%S'\n",
	    ajStrLen(thys->Header), header);

    return;
}




/* @func ajAlignSetHeaderApp **************************************************
**
** Appends to an alignment header
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] header [const AjPStr] Align header with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetHeaderApp(AjPAlign thys, const AjPStr header)
{
    if(ajStrLen(thys->Header) && ajStrChar(thys->Header, -1) != '\n')
	ajStrAppC(&thys->Header, "/n");

    ajStrApp(&thys->Header, header);

    ajDebug("ajAlignSetHeaderApp len %d '%S'\n",
	    ajStrLen(thys->Header), header);

    return;
}




/* @func ajAlignSetTail *******************************************************
**
** Defines an alignment tail
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] tail [const AjPStr] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetTail(AjPAlign thys, const AjPStr tail)
{
    ajStrAssS(&thys->Tail, tail);

    ajDebug("ajAlignSetTail len %d '%S'\n",
	    ajStrLen(thys->Tail), tail);

    return;
}




/* @func ajAlignSetTailC ******************************************************
**
** Defines an alignment tail
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] tail [const char*] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetTailC(AjPAlign thys, const char* tail)
{
    ajStrAssC(&thys->Tail, tail);

    ajDebug("ajAlignSetTailC len %d '%S'\n",
	    ajStrLen(thys->Tail), tail);

    return;
}




/* @func ajAlignSetTailApp ****************************************************
**
** Apopends to an alignment tail
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] tail [const AjPStr] Align tail with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetTailApp(AjPAlign thys, const AjPStr tail)
{
    if(ajStrLen(thys->Tail) && ajStrChar(thys->Tail, -1) != '\n')
	ajStrAppC(&thys->Tail, "/n");

    ajStrApp(&thys->Tail, tail);

    ajDebug("ajAlignSetTailApp len %d '%S'\n",
	    ajStrLen(thys->Tail), tail);

    return;
}




/* @func ajAlignSetSubHeader **************************************************
**
** Defines an alignment subheader (cleared after printing so it can
** be set again for the next alignment)
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] subheader [const AjPStr] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeader(AjPAlign thys, const AjPStr subheader)
{
  ajStrAssS(&thys->SubHeader, subheader);

  ajDebug("ajAlignSetSubHeader len %d '%S'\n",
	  ajStrLen(thys->SubHeader), subheader);

  return;
}




/* @func ajAlignSetSubHeaderC *************************************************
**
** Defines an alignment header (cleared after printing so it can
** be set again for the next alignment)
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] subheader [const char*] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeaderC(AjPAlign thys, const char* subheader)
{
    ajStrAssC(&thys->Header, subheader);

    ajDebug("ajAlignSetSubHeaderC len %d '%S'\n",
	    ajStrLen(thys->SubHeader), subheader);

    return;
}




/* @func ajAlignSetSubHeaderApp ***********************************************
**
** Appends to an alignment subheader (cleared after printing so it can
** be set again for the next alignment)
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] subheader [const AjPStr] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeaderApp(AjPAlign thys, const AjPStr subheader)
{
    if(ajStrLen(thys->SubHeader) && ajStrChar(thys->SubHeader, -1) != '\n')
	ajStrAppC(&thys->SubHeader, "/n");

    ajStrApp(&thys->SubHeader, subheader);

    ajDebug("ajAlignSetSubHeaderApp len %d '%S'\n",
	    ajStrLen(thys->SubHeader), subheader);

    return;
}




/* @func ajAlignSetSubHeaderPre ***********************************************
**
** Prepends to an alignment subheader (cleared after printing so it can
** be set again for the next alignment)
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] subheader [const AjPStr] Align subheader with embedded newlines
** @return [void]
** @@
******************************************************************************/

void ajAlignSetSubHeaderPre(AjPAlign thys, const AjPStr subheader)
{
    if(ajStrLen(thys->SubHeader) && ajStrChar(subheader,-1) != '\n')
	ajStrInsertC(&thys->SubHeader, 0, "/n");

    ajStrInsert(&thys->SubHeader, 0, subheader);

    ajDebug("ajAlignSetSubHeaderPre len %d '%S'\n",
	    ajStrLen(thys->SubHeader), subheader);

    return;
}




/* @func ajAlignSetMatrixNameC ************************************************
**
** Defines an alignment matrix
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] matrix [const char*] Matrix name
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixNameC(AjPAlign thys, const char* matrix)
{
    ajStrAssC(&thys->Matrix, matrix);

    return;
}




/* @func ajAlignSetMatrixName *************************************************
**
** Defines an alignment matrix
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] matrix [const AjPStr] Matrix name
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixName(AjPAlign thys, const AjPStr matrix)
{
    ajAlignSetMatrixNameC(thys, ajStrStr(matrix));

    return;
}




/* @func ajAlignSetMatrixInt **************************************************
**
** Defines an alignment matrix
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] matrix [const AjPMatrix] Matrix object
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixInt(AjPAlign thys, const AjPMatrix matrix)
{
    if(!thys->IMatrix)
    {
	thys->IMatrix = matrix;
	ajAlignSetMatrixName(thys, ajMatrixName(matrix));
    }

    if(thys->FMatrix)
	ajMatrixfDel(&thys->FMatrix);

    return;
}




/* @func ajAlignSetMatrixFloat ************************************************
**
** Defines an alignment matrix
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] matrix [const AjPMatrixf] Matrix (floating point version) object
** @return [void]
** @@
******************************************************************************/

void ajAlignSetMatrixFloat(AjPAlign thys, const AjPMatrixf matrix)
{
    if(!thys->FMatrix)
    {
	thys->FMatrix = matrix;
	ajAlignSetMatrixName(thys, ajMatrixfName(matrix));
    }

    if(thys->IMatrix)
	ajMatrixDel(&thys->IMatrix);

    return;
}




/* @func ajAlignSetGapI *******************************************************
**
** Defines alignment gap penalties
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] gappen [ajint] Gap penalty
** @param [r] extpen [ajint] Gap extension penalty
** @return [void]
** @@
******************************************************************************/

void ajAlignSetGapI(AjPAlign thys, ajint gappen, ajint extpen)
{
    static AjPStr tmpstr = NULL;

    ajFmtPrintS(&tmpstr, "%d", gappen);
    ajStrAssS(&thys->GapPen, tmpstr);

    ajFmtPrintS(&tmpstr, "%d", extpen);
    ajStrAssS(&thys->ExtPen, tmpstr);

    return;
}




/* @func ajAlignSetGapR *******************************************************
 **
 ** Defines alignment gap penalties
 **
 ** @param [r] thys [AjPAlign] Alignment object
 ** @param [r] gappen [float] Gap penalty
 ** @param [r] extpen [float] Gap extension penalty
 ** @return [void]
 ** @@
 *****************************************************************************/

void ajAlignSetGapR(AjPAlign thys, float gappen, float extpen)
{
    static AjPStr tmpstr = NULL;

    ajint precision = 3;
    ajint i;

    ajFmtPrintS(&tmpstr, "%.*f", precision, gappen);
    for(i=1; i<precision; i++)
    {
	if(ajStrChar(tmpstr, -1) != '0') break;
	ajStrTrim(&tmpstr, -1);
    }
    ajStrAssS(&thys->GapPen, tmpstr);

    ajFmtPrintS(&tmpstr, "%.*f", precision, extpen);
    for(i=1; i<precision; i++)
    {
	if(ajStrChar(tmpstr, -1) != '0') break;
	ajStrTrim(&tmpstr, -1);
    }
    ajStrAssS(&thys->ExtPen, tmpstr);

    return;
}




/* @func ajAlignSetScoreI *****************************************************
**
** Defines alignment score
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] score [ajint] score
** @return [void]
** @@
******************************************************************************/

void ajAlignSetScoreI(AjPAlign thys, ajint score)
{
    static AjPStr tmpstr = NULL;
    AlignPData data = NULL;

    ajListLast(thys->Data, (void**) &data);
    ajFmtPrintS(&tmpstr, "%d", score);
    ajStrAssS(&data->Score, tmpstr);

    ajDebug("ajAlignSetScoreI: %d '%S' %d\n", score, data->Score, data->Len);

    return;
}




/* @func ajAlignSetScoreR *****************************************************
**
** Defines alignment score
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] score [float] score
** @return [void]
** @@
******************************************************************************/

void ajAlignSetScoreR(AjPAlign thys, float score)
{
    static AjPStr tmpstr = NULL;

    ajint precision = 3;
    ajint i;
    AlignPData data = NULL;

    ajListLast(thys->Data, (void**) &data);
    ajFmtPrintS(&tmpstr, "%.*f", precision, score);
    for(i=1; i<precision; i++)
    {
	if(ajStrChar(tmpstr, -1) != '0') break;
	ajStrTrim(&tmpstr, -1);
    }
    ajStrAssS(&data->Score, tmpstr);

    return;
}




/* @func ajAlignSetStats ******************************************************
**
** Sets standard properties for an alignment subheader. These are:
** Length, Identity, Gaps, Similarity, Score
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @param [r] len [ajint] Alignment length
** @param [r] ident [ajint] Number of identities
** @param [r] sim [ajint] Number of similarities
** @param [r] gaps [ajint] Number of gaps
** @param [r] score [const AjPStr] Alignment score (as saved by
**                           ajAlignSetScoreI or ajAlignSetScoreR)
** @return [void]
******************************************************************************/

void ajAlignSetStats(AjPAlign thys, ajint iali, ajint len,
		     ajint ident, ajint sim, ajint gaps,
		     const AjPStr score)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;
    
    nali = ajListToArray(thys->Data, (void***) &pdata);
    if(iali < 0)
	data = pdata[nali-1];
    else
	data = pdata[iali];
    
    ajDebug("ajAlignSetStats iali:%d len:%d id:%d sim:%d gap:%d score:'%S'\n",
	    iali, len, ident, sim, gaps, score);
    
    data->Len = len;
    
    if(len > 0)
    {
	if(ident >= 0)
	    data->NumId = ident;
	else
	    data->NumId = -1;

	if(sim >= 0)
	    data->NumSim = sim;
	else
	    data->NumSim = -1;

	if(gaps >= 0)
	    data->NumGap = gaps;
	else
	    data->NumGap = -1;
    }

    if(ajStrLen(score))
	ajStrAssS(&data->Score, score);
    
    AJFREE(pdata);
    
    return;
}




/* @func ajAlignSetSubStandard ************************************************
**
** Sets standard subheader using the properties for an alignment.
** These are:
** Length, Identity, Gaps, Similarity, Score
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number (or -1 for the latest)
** @return [void]
******************************************************************************/

void ajAlignSetSubStandard(AjPAlign thys, ajint iali)
{
    AjPStr tmphdr = NULL;
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;
    float pct;

    nali = ajListToArray(thys->Data, (void***) &pdata);
    if(iali < 0)
	data = pdata[nali-1];
    else
	data = pdata[iali];

    ajFmtPrintAppS(&tmphdr, "Length: %d\n", data->Len);

    if(data->Len > 0)
    {
	if(data->NumId >= 0)
	{
	    pct = 100. * (float) data->NumId / (float) data->Len;
	    ajFmtPrintAppS(&tmphdr, "Identity:   %5d/%d (%4.1f%%)\n",
			   data->NumId, data->Len, pct);
	}

	if(data->NumSim >= 0)
	{
	    pct = 100. * (float) data->NumSim / (float) data->Len;
	    ajFmtPrintAppS(&tmphdr, "Similarity: %5d/%d (%4.1f%%)\n",
			   data->NumSim, data->Len, pct);
	}
	if(data->NumGap >= 0)
	{
	    pct = 100. * (float) data->NumGap / (float) data->Len;
	    ajFmtPrintAppS(&tmphdr, "Gaps:       %5d/%d (%4.1f%%)\n",
			   data->NumGap, data->Len, pct);
	}
    }

    if(ajStrLen(data->Score))
	ajFmtPrintAppS(&tmphdr, "Score: %S\n", data->Score);

    ajAlignSetSubHeaderPre(thys, tmphdr);

    ajStrDel(&tmphdr);
    AJFREE(pdata);

    return;
}




/* @funcstatic alignSeqs ******************************************************
**
** Returns the nth sequence for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [AjPSeq*] Pointer to the internal sequence array
******************************************************************************/

static AjPSeq* alignSeqs(const AjPAlign thys, ajint iali)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;

    nali = ajListToArray(thys->Data, (void***) &pdata);
    data = pdata[iali];

    AJFREE(pdata);

    return data->Seq;
}




/* @funcstatic alignSeq *******************************************************
**
** Returns the nth sequence for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iseq [ajint] Sequence number
** @param [r] iali [ajint] Alignment number
** @return [AjPSeq] Pointer to the internal sequence
******************************************************************************/

static AjPSeq alignSeq(const AjPAlign thys, ajint iseq, ajint iali)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;

    nali = ajListToArray(thys->Data, (void***) &pdata);
    data = pdata[iali];

    AJFREE(pdata);

    return data->Seq[iseq];
}




/* @funcstatic alignData ******************************************************
**
** Returns the nth data structure for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [AlignPData] Pointer to the internal alignment structure
******************************************************************************/

static AlignPData alignData(const AjPAlign thys, ajint iali)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;

    nali = ajListToArray(thys->Data, (void***) &pdata);
    data = pdata[iali];

    AJFREE(pdata);

    return data;
}




/* @funcstatic alignLen *******************************************************
**
** Returns the length of the nth sequence for an alignment
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [ajint] Length of the internal sequence
******************************************************************************/

static ajint alignLen(const AjPAlign thys, ajint iali)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;

    nali = ajListToArray(thys->Data, (void***) &pdata);
    data = pdata[iali];

    AJFREE(pdata);

    return data->Len;
}





/* @func ajAlignSetType *******************************************************
**
** Sets the align type (if it is not set already)
**
** @param [r] thys [AjPAlign] Alignment object
** @return [void]
** @@
******************************************************************************/

void ajAlignSetType(AjPAlign thys)
{
    AjPSeq seq;
    ajint i;

    ajDebug("ajAlignSetType '%S'\n",
	    thys->Type);

    if(ajStrLen(thys->Type))
	return;

    for(i=0; i < thys->Nseqs; i++)
    {
	seq = alignSeq(thys, i, 0);
	if(ajStrLen(seq->Type))
	{
	    ajStrAssS(&thys->Type, seq->Type);
	    return;
	}
    }

    return;
}




/* @func ajAlignSetExternal ***************************************************
**
** Sets the align object to use external sequence references, which are
** to be copied pointers rather than clones of the whole sequence.
**
** Intended for alignments of large sequences where there is no need to
** keep many copies. An example is the EMBOSS application wordmatch.
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] external [AjBool] If true, do not make copies of sequence data
**                              and do not delete internal sequence data
** @return [void]
** @@
******************************************************************************/

void ajAlignSetExternal(AjPAlign thys, AjBool external)
{
    ajDebug("ajAlignSetExternal old:%B new:%B\n",
	    thys->SeqExternal, external);

    thys->SeqExternal = external;

    return;
}




/* @func ajAlignSetRange ******************************************************
**
** Sets the alignment range in each sequence, but only for a
** pairwise alignment
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] start1 [ajint] Start in sequence 1
** @param [r] end1 [ajint] End in sequence 1
** @param [r] start2 [ajint] Start in sequence 2
** @param [r] end2 [ajint] End in sequence 2
** @return [AjBool] ajTrue on success. Failure also writes an error message.
** @@
******************************************************************************/

AjBool ajAlignSetRange(AjPAlign thys,
		       ajint start1, ajint end1,
		       ajint start2, ajint end2)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;

    ajDebug("ajAlignSetRange %d..%d %d..%d\n",
	    start1, end1, start2, end2);

    if(thys->Nseqs != 2)
    {
	ajErr("ajAlignSetRange requires alignment of 2 sequences only");
	return ajFalse;
    }

    nali = ajListToArray(thys->Data, (void***) &pdata);

    ajDebug("nali:%d set range %d\n", nali, nali-1);

    data = pdata[nali-1];
    data->Start[0]  = start1;
    data->End[0]    = end1;
    data->Offset[0] = 0;
    data->Rev[0]    = ajFalse;

    data->Start[1]  = start2;
    data->End[1]    = end2;
    data->Offset[1] = 0;
    data->Rev[1]    = ajFalse;

    if(thys->SeqExternal)
    {
	data->Len = (end1 - start1) + 1;
	if(data->Len < (end2 - start2 + 1))
	    data->Len = (end2 - start2) + 1;
	ajDebug("len:  %d\n", data->Len);
    }

    AJFREE(pdata);

    return ajTrue;
}




/* @func ajAlignSetSubRange ***************************************************
**
** Sets the alignment sub range in each sequence, but only for a
** pairwise alignment.
**
** This sets the SubOffset in addition to the Start and End values,
** for use where the alignment has a large sequence, but only part of
** it is to be reported.
**
** The usual example is where there are many matches in a long sequence,
** defined with SeqExternal so we use pointers to one original to avoid
** making multiple copies in memory while builing the AjPAlign and AlignPData
** structure.
**
** @param [r] thys [AjPAlign] Alignment object
** @param [r] substart1 [ajint] Subsequence offset in sequence 1
** @param [r] start1 [ajint] Subsequence start in sequence 1
** @param [r] end1 [ajint] Subsequence end in sequence 1
** @param [r] substart2 [ajint] Subsequence offset in sequence 2
** @param [r] start2 [ajint] Subsequence start in sequence 2
** @param [r] end2 [ajint] Subsequence end in sequence 2
** @return [AjBool] ajTrue on success. Failure also writes an error message.
** @@
******************************************************************************/

AjBool ajAlignSetSubRange(AjPAlign thys,
			  ajint substart1, ajint start1, ajint end1,
			  ajint substart2, ajint start2, ajint end2)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;

    ajDebug("ajAlignSetRange %d..%d %d..%d\n",
	    start1, end1, start2, end2);

    if(thys->Nseqs != 2)
    {
	ajErr("ajAlignSetRange requires alignment of 2 sequences only");
	return ajFalse;
    }

    nali = ajListToArray(thys->Data, (void***) &pdata);

    ajDebug("nali:%d set range %d\n", nali, nali-1);

    data = pdata[nali-1];
    data->SubOffset[0] = substart1;
    data->Start[0]     = start1;
    data->End[0]       = end1;
    data->Offset[0]    = 0;
    data->Rev[0]       = ajFalse;

    data->SubOffset[1] = substart2;
    data->Start[1] = start2;
    data->End[1] = end2;
    data->Offset[1] = 0;
    data->Rev[1] = ajFalse;

    if(thys->SeqExternal)
    {
	data->Len = (end1 - start1) + 1;
	if(data->Len < (end2 - start2 + 1))
	    data->Len = (end2 - start2) + 1;
	ajDebug("len:  %d\n", data->Len);
    }

    AJFREE(pdata);

    return ajTrue;
}




/* @funcstatic alignSeqName ***************************************************
**
** Returns the sequence name or USA depending on the setting in the
** Alignment object (derived from the ACD and command line -ausa option)
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] i [ajint] Sequence number in set
** @return [AjPStr] Sequence name for this alignment
******************************************************************************/

static AjPStr alignSeqName(const AjPAlign thys, ajint i)
{
    AjPSeq seq = NULL;

    seq = alignSeq(thys, i, 0);

    /*ajDebug("alignSeqName acc '%S' des '%S'\n",
      ajSeqGetAcc(seq), ajSeqGetDesc(seq));*/

    if(thys->Showusa)
	return ajSeqGetUsa(seq);

    return ajSeqGetName(seq);
}




/* @funcstatic alignDataDel ***************************************************
**
** Deletes an alignment data structure
**
** @param [d] pthys [AlignPData*] Alignment data structure
** @param [r] external [AjBool] Sequence is a pointer to an external
**                              object, do not delete.
** @return [void]
******************************************************************************/

static void alignDataDel(AlignPData* pthys, AjBool external)
{
    AlignPData thys;
    ajint i;

    thys = *pthys;

    AJFREE(thys->Start);
    AJFREE(thys->End);
    AJFREE(thys->Rev);
    AJFREE(thys->Offset);
    AJFREE(thys->SubOffset);

    ajStrDel(&thys->Score);
    if(!external)
	for(i=0;i<thys->Nseqs;++i)
	    ajSeqDel(&thys->Seq[i]);

    AJFREE(thys->Seq);
    AJFREE(*pthys);

    return;
}




/* @funcstatic alignDiff ******************************************************
**
** Blank out differences between two strings
**
** @param [w] pmark [AjPStr*] Mark string with spaces for differences
** @param [r] seq [const AjPStr] String (sequence) to compare
** @return [void]
******************************************************************************/

static void alignDiff(AjPStr* pmark, const AjPStr seq)
{
    ajint i;
    ajint ilen;
    char c;
    char d;
    char* s = " ";

    ilen = ajStrLen(seq);

    ajStrMod(pmark);

    if(ajStrLen(*pmark) < ilen)
	ilen = ajStrLen(*pmark);

    for(i=0; i < ilen; i++)
    {
	c = ajStrChar(*pmark, i);
	if(c == ' ') continue;
	d = ajStrChar(seq, i);
	if(toupper((int)c) == toupper((int)d)) continue;
	ajStrReplaceC(pmark, i, s, 1);
    }

    return;
}




/* @funcstatic alignSim *******************************************************
**
** Convert upper case (identical) positions to an identity character,
** and lower case (similar) positions to a similarity character
**
** @param [w] pmark [AjPStr*] Mark string with spaces for differences
** @param [r] idch [const char] Identity character
** @param [r] simch [const char] Similarity character
** @param [r] misch [const char] Mismatch character
** @param [r] gapch [const char] Gap character
** @return [void]
******************************************************************************/

static void alignSim(AjPStr* pmark, const char idch, const char simch,
		     const char misch, const char gapch)
{
    ajint i;
    ajint ilen;
    char c;

    ajDebug("alignSim '%S'\n", *pmark);

    ajStrMod(pmark);

    ilen = ajStrLen(*pmark);

    for(i=0; i < ilen; i++)
    {
	c = ajStrChar(*pmark, i);
	if(tolower((int)c) == 'x')
	    ajStrReplaceK(pmark, i, misch, 1);
	else if(isupper((int)c))
	    ajStrReplaceK(pmark, i, idch, 1);
	else if(islower((int)c))
	    ajStrReplaceK(pmark, i, simch, 1);
	else
	    ajStrReplaceK(pmark, i,gapch, 1);
    }

    ajDebug("  result '%S'\n", *pmark);

    return;
}




/* @funcstatic alignConsStats *************************************************
**
** Calculates alignment statistics (and a consensus).
**
** @param [u] thys [AjPAlign] Alignment object. Matrix will be defined
**                            if not already set.
** @param [r] iali [ajint] alignment number
** @param [w] cons [AjPStr*] the created consensus sequence
** @param [r] retident [ajint*] number of residues identical in all sequences
** @param [r] retsim   [ajint*] number of residues similar in all sequences
** @param [r] retgap   [ajint*] number of residues with a gap in 1 sequence
** @param [r] retlen [ajint*] length of the alignment
** @return [void]
******************************************************************************/

static void alignConsStats(AjPAlign thys, ajint iali, AjPStr *cons,
			   ajint* retident, ajint* retsim, ajint* retgap,
			   ajint* retlen)
{
    ajint   imat;     /* iterate over identical and matching arrays */
    ajint   iseq;	     /* iterate over sequences (outer loop) */
    ajint   jseq;	     /* iterate over sequences (inner loop) */
    
    
    ajint   **matrix  = NULL;
    float   **fmatrix = NULL;

    ajint   m1 = 0;
    ajint   m2 = 0;

    ajint   matsize;
    ajint   matchingmaxindex;
    ajint   identicalmaxindex;
    ajint   nseqs;
    ajint   mlen;
    
    float   max;
    float   contri = 0;
    float   contrj = 0;
    
    AjPSeqCvt cvt  = 0;
    
    AjPFloat  posScore = NULL; /* cumulative similarity scores by sequence */
    /* for matching all other sequences */
    float   *identical;	    /* cum. weight for each valid character */
    float   *matching;	 /* cum. weight for matching this character */
    ajint   highindex;	   /* position of highest score in posScore */
    ajint   kkpos;		/* alignment position loop variable */
    ajint   kipos;	   /* alignment position kkpos + iseq start */
    ajint   kjpos;	   /* alignment position kkpos + jseq start */
    ajint   khpos;		 /* alignment position in highindex */
    
    float himatch = 0.0;	/* highest match score (often used) */
    
    char **seqcharptr;
    char res;
    char nocon;
    char gapch;
    float fplural;
    float fplurality = 51.0;
    float setcase    = 0.0;
    float ident;
    AjBool isident;
    AjBool issim;
    AjBool isgap;
    AjPSeq* seqs;
    ajint numres;		 /* number of residues (not spaces) */
    AlignPData data = NULL;
    AjPStr debugstr1 = NULL;
    AjPStr debugstr2 = NULL;
    
    
    debugstr1=ajStrNew();
    debugstr2=ajStrNew();

    data = alignData(thys, iali);
    
    if(!thys->IMatrix && !thys->FMatrix)
    {
	if(ajSeqIsNuc(alignSeq(thys, 1, iali)))
	    ajStrAssC(&thys->Matrix, "EDNAFULL");
	else
	    ajStrAssC(&thys->Matrix, "EBLOSUM62");
	ajMatrixRead(&thys->IMatrix, thys->Matrix);
    }
    
    *retident=0;
    *retsim=0;
    *retgap=0;
    
    seqs    = alignSeqs(thys, iali);
    nseqs   = thys->Nseqs;
    mlen    = alignLen(thys, iali);
    fplural = alignTotweight(thys, 0) * fplurality / 100.;
    ident   = alignTotweight(thys, 0);
    setcase = ident;
    
    ajDebug("fplural:%.1f ident:%.1f setcase:%.1f mlen: %d\n",
	    fplural, ident, setcase, mlen);
    
    if(thys->IMatrix)
    {
	matrix  = ajMatrixArray(thys->IMatrix);
	cvt     = ajMatrixCvt(thys->IMatrix); /* return conversion table */
	matsize = ajMatrixSize(thys->IMatrix);
    }
    else
    {
	fmatrix = ajMatrixfArray(thys->FMatrix);
	cvt     = ajMatrixfCvt(thys->FMatrix); /* return conversion table */
	matsize = ajMatrixfSize(thys->FMatrix);
    }
    
    AJCNEW(seqcharptr,nseqs);
    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);
    
    posScore = ajFloatNew();
    
    gapch = '-';
    nocon = 'x';
    
    for(iseq=0;iseq<nseqs;iseq++)	/* get sequence as string */
	seqcharptr[iseq] =  ajSeqChar(alignSeq(thys, iseq, iali));
    
    /* For each position in the alignment, calculate consensus character */
    
    for(kkpos=0; kkpos< mlen; kkpos++)
    {
	res = gapch;
	
	isident = ajFalse;
	issim   = ajFalse;
	isgap   = ajFalse;
	
	/*
	 ** reset identities and +ve matches
	 */
	
	for(imat=0;imat<matsize;imat++)
	{
	    identical[imat] = 0.0; /* weights of all sequence chars in col. */
	    matching[imat]  = 0.0;
	}
	
	/*
	 ** reset the posScore array
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	    ajFloatPut(&posScore,iseq,0.);
	
	/*
	 ** generate scores (identical, posScore) for columns
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1 = ajSeqCvtK(cvt,seqcharptr[iseq][kipos]);
	    if(m1)
		identical[m1] += seqs[iseq]->Weight;
	    
	    for(jseq=iseq+1;jseq<nseqs;jseq++)
	    {
		kjpos = kkpos + data->SubOffset[jseq];
		m2 = ajSeqCvtK(cvt,seqcharptr[jseq][kjpos]);
		if(m1 && m2)
		{
		    if(matrix)
		    {
			contri = (float)matrix[m1][m2]*seqs[jseq]->Weight;
			+ajFloatGet(posScore,iseq);
			contrj = (float)matrix[m1][m2]*seqs[iseq]->Weight;
			+ajFloatGet(posScore,jseq);
		    }
		    else
		    {
			contri = fmatrix[m1][m2]*seqs[jseq]->Weight;
			+ajFloatGet(posScore,iseq);
			contrj = fmatrix[m1][m2]*seqs[iseq]->Weight;
			+ajFloatGet(posScore,jseq);
		    }
		    ajFloatPut(&posScore,iseq,contri);
		    ajFloatPut(&posScore,jseq,contrj);
		}
	    }
	}
	
	/*
	 ** highindex is the highest scoring position (seq no.) in posScore
	 ** for 2 sequences this appears to be usually 0
	 */
	
	highindex = -1;
	max       = -FLT_MAX;
	numres    = 0;
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    if(seqcharptr[iseq][kipos] != ' ' &&
	       seqcharptr[iseq][kipos] != '-')
		numres++;

	    if(ajFloatGet(posScore,iseq) > max)
	    {
		highindex = iseq;
		max       = ajFloatGet(posScore,iseq);
	    }
	}
	
	/* highindex is now set */
	
	/*
	 ** find +ve matches in the column
	 ** m1 is non-zero for a valid character in iseq
	 ** m2 is non-zero for a valid character in jseq
	 */
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1    = ajSeqCvtK(cvt, seqcharptr[iseq][kipos]);
	    if(!matching[m1]) /* first time we have met this character */
	    {
		for(jseq=0;jseq<nseqs;jseq++) /* all (other) sequences */
		{
		    kjpos = kkpos + data->SubOffset[jseq];
		    m2    = ajSeqCvtK(cvt, seqcharptr[jseq][kjpos]);
		    if(matrix)
		    {
			/* 'matching' if positive */
			if(m1 && m2 && matrix[m1][m2] > 0) 
			    matching[m1] += seqs[jseq]->Weight;
		    }
		    else
			if(m1 && m2 && fmatrix[m1][m2] > 0.0)
			    matching[m1] += seqs[jseq]->Weight;


		    /*
		       if( iseq != jseq)
		       /# skip the sequence we are on #/
		       {
		       m2 = ajSeqCvtK(cvt, seqcharptr[jseq][kjpos]);
		       if(matrix)
		       {
		       if(m1 && m2 && matrix[m1][m2] > 0)
		       /# 'matching' if positive #/
		       {
		       matching[m1] += seqs[jseq]->Weight;
		       }
		       }
		       else
		       {
		       if(m1 && m2 && fmatrix[m1][m2] > 0.0)
		       {
		       matching[m1] += seqs[jseq]->Weight;
		       }
		       }
		       }
		       */
		}
	    }
	}
	
	matchingmaxindex  = 0;	  /* get max matching and identical */
	identicalmaxindex = 0;
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1 = ajSeqCvtK(cvt,seqcharptr[iseq][kipos]);
	    if(identical[m1] > identical[identicalmaxindex])
		identicalmaxindex= m1;
	}
	
	for(iseq=0;iseq<nseqs;iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    m1 = ajSeqCvtK(cvt,seqcharptr[iseq][kipos]);
	    if(matching[m1] > matching[matchingmaxindex])
		matchingmaxindex= m1;
	    else if(matching[m1] ==  matching[matchingmaxindex])
	    {
		if(identical[m1] > identical[matchingmaxindex])
		    matchingmaxindex= m1;
	    }

	    if(seqcharptr[iseq][kipos] == '-' ||
	       seqcharptr[iseq][kipos] == ' ')
		isgap=ajTrue;
	}
	/*
	   ajDebug("index[%d] ident:%d matching:%d high:%d\n",
	   kpos,
	   identicalmaxindex,
	   matchingmaxindex, highindex);
	   */
	khpos   = kkpos + data->SubOffset[highindex];
	himatch = matching[ajSeqCvtK(cvt,seqcharptr[highindex][khpos])];
	
	if(thys->IMatrix)
	{
	    ajMatrixChar(thys->IMatrix, identicalmaxindex-1, &debugstr1);
	    ajMatrixChar(thys->IMatrix, matchingmaxindex-1, &debugstr2);
	    

	    ajDebug("index[%d] ident:%d '%S' %.1f matching:%d '%S' %.1f %.1f "
		    "high:%d '%c' %.1f\n",
		    kkpos,
		    identicalmaxindex,
		    debugstr1, 
		    identical[identicalmaxindex],
		    matchingmaxindex,
		    debugstr2,      
		    matching[matchingmaxindex],
		    himatch,
		    highindex, seqcharptr[highindex][khpos],
		    seqs[highindex]->Weight);
	}
	else
	{
	    ajMatrixfChar(thys->FMatrix, identicalmaxindex-1, &debugstr1);
	    ajMatrixfChar(thys->FMatrix, matchingmaxindex-1, &debugstr2);
	    ajDebug("index[%d] ident:%d '%S' %.1f matching:%d '%S' %.1f %.1f "
		    "high:%d '%c' %.1f\n",
		    kkpos,
		    identicalmaxindex,
		    debugstr1, 
		    identical[identicalmaxindex],
		    matchingmaxindex,
		    debugstr2, 
		    matching[matchingmaxindex],
		    himatch,
		    highindex, seqcharptr[highindex][khpos],
		    seqs[highindex]->Weight);
	}
		
	if(identical[identicalmaxindex] >= ident)
	    isident=ajTrue;
	if(matching[matchingmaxindex] >= fplural)
	    issim=ajTrue;
	
	/* plurality check */
	if(himatch >= fplural)
	    if(seqcharptr[highindex][khpos] != '-')
		res = toupper((int)seqcharptr[highindex][khpos]);


	/*
	   if(himatch < setcase)
	   res = tolower(res);
	   */

	if(himatch == seqs[highindex]->Weight)
	{
	    if(numres > 1)
		res = nocon;
	    else
		res = gapch;
	}
	
	if(issim && ! isident)
	    res = tolower((int)res);
	
	ajStrAppK(cons,res);
	if(isident)
	    ++*retident;
	if(issim)
	    ++*retsim;
	if(isgap)
	    ++*retgap;
	
	ajDebug("id:%b sim:%b gap:%b res:%c '", isident, issim, isgap, res);
	for(iseq=0; iseq<nseqs; iseq++)
	{
	    kipos = kkpos + data->SubOffset[iseq];
	    ajDebug("%c", seqcharptr[iseq][kipos]);
	}
	ajDebug("'\n");	
    }
    
    *retlen = alignLen(thys, iali);
    
    ajDebug("ret ident:%d sim:%d gap:%d len:%d\n",
	    *retident, *retsim, *retgap, *retlen);
    
    AJFREE(seqcharptr);
    AJFREE(matching);
    AJFREE(identical);
    ajFloatDel(&posScore);
    ajStrDel(&debugstr1);
    ajStrDel(&debugstr2);

    return;    
}




/* @funcstatic alignTotweight *************************************************
**
** Returns the total wight for all sequences in an alignment.
**
** @param [r] thys [const AjPAlign] Alignment object
** @param [r] iali [ajint] Alignment number
** @return [float] Total weight for all sequences
******************************************************************************/

static float alignTotweight(const AjPAlign thys, ajint iali)
{
    ajint i;
    AjPSeq seq = NULL;
    float ret  = 0.0;

    for(i=0; i < thys->Nseqs; i++)
    {
	seq = alignSeq(thys, i, iali);
	ret += seq->Weight;
    }

    return ret;
}




/* @func ajAlignTrace *********************************************************
**
** Reports an AjPAlign object to debug output
**
** @param [r] thys [const AjPAlign] alignment object
** @return [void]
******************************************************************************/

void ajAlignTrace(const AjPAlign thys)
{
    ajDebug("AjAlignTrace\n");
    ajDebug("============\n");
    ajDebug("Name: '%S'\n", thys->Name);
    ajDebug("Usa: '%S'\n", thys->Usa);
    ajDebug("Type: '%S'\n", thys->Type);
    ajDebug("Formatstr: '%S'\n", thys->Formatstr);
    ajDebug("Format: %d\n", thys->Format);
    ajDebug("Extension: '%S'\n", thys->Extension);
    ajDebug("File: '%F'\n", thys->File);
    ajDebug("Header: '%S'\n", thys->Header);
    ajDebug("SubHeader: '%S'\n", thys->SubHeader);
    ajDebug("Tail: '%S'\n", thys->Tail);
    ajDebug("Showusa: %B\n", thys->Showusa);
    ajDebug("Multi: %B\n", thys->Multi);
    ajDebug("Global: %B\n", thys->Global);

    alignTraceData(thys);

    ajDebug("Nseqs: %d\n", thys->Nseqs);
    ajDebug("WidthNmin: %d\n", thys->Nmin);
    ajDebug("Nmax: %d\n", thys->Nmax);
    ajDebug("Width: %d\n", thys->Width);
    ajDebug("Count: %d\n", thys->Count);

    ajDebug("IMatrix: %x\n", thys->IMatrix);
    ajDebug("FMatrix: %x\n", thys->FMatrix);
    ajDebug("Matrix: '%S'\n", thys->Matrix);
    ajDebug("GapPen: '%S'\n", thys->GapPen);
    ajDebug("ExtPen: '%S'\n", thys->ExtPen);
    ajDebug("SeqOnly: %B\n", thys->SeqOnly);
    ajDebug("SeqExternal: %B\n", thys->SeqExternal);

    return;
}




/* @funcstatic alignTraceData *************************************************
**
** Report alignment internal list data to debug output
**
** @param [r] thys [const AjPAlign] Alignment object
** @return [void]
******************************************************************************/

static void alignTraceData(const AjPAlign thys)
{
    AlignPData* pdata = NULL;
    AlignPData data = NULL;
    ajint nali;
    ajint iali;
    ajint i;
    ajint nseq;
    AjPStr tmpstr = NULL;
    
    nseq = thys->Nseqs;
    
    nali = ajListToArray(thys->Data, (void***) &pdata);
    ajDebug("Data list length: %d\n", nali);
    if(!nali)
	return;
    
    for(iali=0; iali<nali; iali++)
    {
	data = pdata[iali];
	
	ajDebug("%d Len: %d\n", iali, data->Len);
	ajDebug("%d NumId: %d\n", iali, data->NumId);
	ajDebug("%d NumSim: %d\n", iali, data->NumSim);
	ajDebug("%d NumGap: %d\n", iali, data->NumGap);
	ajDebug("%d Score: '%S'\n", iali, data->Score);
	
	ajDebug("%d Start: {", iali);
	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->Start[i]);

	ajDebug(" }\n");
	
	ajDebug("%d End: {", iali);
	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->End[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Offset: {", iali);
	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->Offset[i]);

	ajDebug(" }\n");
	
	ajDebug("%d SubOffset: {", iali);
	for(i=0; i < nseq; i++)
	    ajDebug(" %d", data->SubOffset[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Rev: {", iali);
	for(i=0; i < nseq; i++)
	    ajDebug(" %b", data->Rev[i]);

	ajDebug(" }\n");
	
	ajDebug("%d Seq: {\n", iali);
	for(i=0; i < nseq; i++)
	    if(ajSeqLen(data->Seq[i]) > 40)
	    {
		ajStrAssSub(&tmpstr, ajSeqStr(data->Seq[i]), -20, -1);
		ajDebug("  %d '%20.20S...%20S'\n", i,
			ajSeqStr(data->Seq[i]),
			tmpstr);
	    }
	    else
		ajDebug("  %d '%S'\n", i, ajSeqStr(data->Seq[i]));

	ajDebug("  }\n");	
    }
    AJFREE(pdata);
    ajStrDel(&tmpstr);
    
    return;
}




/* @func ajAlignPrintFormat ***************************************************
**
** Reports the internal data structures
**
** @param [r] outf [const AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajAlignPrintFormat(const AjPFile outf, AjBool full)
{
    ajint i = 0;

    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "# alignment output formats\n");
    ajFmtPrintF(outf, "# Name         Minseq Maxseq Nuc Pro\n");
    ajFmtPrintF(outf, "\n");
    ajFmtPrintF(outf, "AFormat {\n");
    for(i=0; alignFormat[i].Name; i++)
	ajFmtPrintF(outf, "  %-12s %6d %6d %3B %3B\n",
		    alignFormat[i].Name,
		    alignFormat[i].Minseq,
		    alignFormat[i].Maxseq,
		    alignFormat[i].Nuc,
		    alignFormat[i].Prot);

    ajFmtPrintF(outf, "}\n\n");

    return;
}

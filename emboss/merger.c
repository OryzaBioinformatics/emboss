/* @source merger application
**
** Merge two sequences after a global alignment
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @@
**
** Closely based on work by Alan Bleasby
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

void merge(AjPStr *merged, AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
                        int start1, int start2, float score, AjBool mark,
                        float **sub, AjPSeqCvt cvt, char *namea,
                        char *nameb, int begina, int beginb);

float quality (char * seq, int pos, int window);

AjBool bestquality (char * a, char *b, int apos, int bpos);

int main(int argc, char **argv)
{
    AjPSeq a;
    AjPSeq b;
    AjPSeqout seqout;

    AjPStr m;
    AjPStr n;

    AjPStr merged=NULL;
    
    AjPFile outf;
    
    int    lena;
    int    lenb;

    char   *p;
    char   *q;

    int start1=0;
    int start2=0;
    
    float  *path;
    int    *compass;
    
    AjPMatrixf matrix;
    AjPSeqCvt  cvt=0;
    float      **sub;

    float gapopen;
    float gapextend;
    int maxarr=1000;
    int len;			/* arbitrary. realloc'd if needed */

    float score;
    int begina;
    int beginb;
    
    (void) embInit("merger", argc, argv);

    a      = ajAcdGetSeq("seqa");
    b      = ajAcdGetSeq("seqb");
    seqout = ajAcdGetSeqout ("outseq");
    matrix    = ajAcdGetMatrixf("datafile");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    outf      = ajAcdGetOutfile("report");

    gapopen = ajRoundF(gapopen, 8);
    gapextend = ajRoundF(gapextend, 8);

    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);

/* make the two sequences lowercase so we can show which one we are
using in the merge by uppercasing it */

    (void) ajSeqToLower (a);
    (void) ajSeqToLower (b);

    m=ajStrNew();
    n=ajStrNew();
    
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    begina = ajSeqBegin(a);
    beginb = ajSeqBegin(b);

    lena = ajSeqLen(a);
    lenb = ajSeqLen(b);
    len = lena*lenb;
    if(len>maxarr)
    {
        AJCRESIZE(path,len);
        AJCRESIZE(compass,len);
        maxarr=len;
    }


    p = ajSeqChar(a);
    q = ajSeqChar(b);
    
    (void) ajStrAssC(&m,"");
    (void) ajStrAssC(&n,"");

    (void) embAlignPathCalc(p,q,lena,lenb,gapopen,gapextend,path,sub,cvt,
            compass, ajFalse);

    score = embAlignScoreNWMatrix(path,a,b,sub,cvt,lena,lenb,gapopen,compass,
                     gapextend,&start1,&start2);

    (void) embAlignWalkNWMatrix(path,a,b,&m,&n,lena,lenb,
				&start1,&start2,gapopen,
				gapextend,cvt,compass,sub);

/* now construct the merged sequence, uppercase the bits of the two
input sequences which are used in the merger */
    (void) merge(&merged, outf,p,q,m,n,start1,start2,score,1,sub,cvt,
                   ajSeqName(a),ajSeqName(b),begina,beginb);

/* print the alignment */
    (void) embAlignPrintGlobal(outf,p,q,m,n,start1,start2,score,1,sub,cvt,
               ajSeqName(a),ajSeqName(b),begina,beginb);

/* write the merged sequence */    
    (void) ajSeqReplace(a, merged);
    (void) ajSeqWrite (seqout, a);
    (void) ajSeqWriteClose (seqout);

    AJFREE(compass);
    AJFREE(path);

    (void) ajStrDel(&n);
    (void) ajStrDel(&m);

    (void) ajExit();
    return 0;
}

/* @func merge *******************************************
** 
** Print a global alignment
** Nucleotides or proteins as needed.   
** 
** @param [w] ms [AjPStr *] output merged sequence
** @param [w] outf [AjPFile] output stream
** @param [r] a [char *] complete first sequence
** @param [r] b [char *] complete second sequence
** @param [r] m [AjPStr] Walk alignment for first sequence
** @param [r] n [AjPStr] Walk alignment for second sequence
** @param [r] start1 [int] start of alignment in first sequence
** @param [r] start2 [int] start of alignment in second sequence
** @param [r] score [float] alignment score from AlignScoreX
** @param [r] mark [AjBool] mark matches and conservatives
** @param [r] sub [float **] substitution matrix
** @param [r] cvt [AjPSeqCvt] conversion table for matrix
** @param [r] namea [char *] name of first sequence
** @param [r] nameb [char *] name of second sequence
** @param [r] begina [int] first sequence offset
** @param [r] beginb [int] second sequence offset
**
** @return [void]
******************************************************************************/

void merge(AjPStr *ms, AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
                        int start1, int start2, float score, AjBool mark,
                        float **sub, AjPSeqCvt cvt, char *namea,
                        char *nameb, int begina, int beginb) {

  int apos, bpos;
  int i;                                                                        

  char *p = ajStrStr(m);
  char *q = ajStrStr(n);
  
  int olen = ajStrLen(m);	/* length of walk alignment */
  int alen, blen;	/* lengths of the sequences after the aligned region */

/* output the left hand side */
  if (start1 > start2) {
    for (i=0; i<start1; i++) {
      (void) ajStrAppK(ms, a[i]);
    }

    if (start2) {
      (void) ajFmtPrintF(outf, "WARNING: *********************************");
      (void) ajFmtPrintF(outf,
		  "The region of alignment only starts at position %d "
		  "of sequence %s", start2+1, nameb);
      (void) ajFmtPrintF(outf, "Only the sequence of %s is being used before this "
		  "point\n\n", namea);
    }
  } else if (start2 > start1) {
    for (i=0; i<start2; i++) {
      (void) ajStrAppK(ms, b[i]);
    }

    if (start1) {
      (void) ajFmtPrintF(outf, "WARNING: *********************************");
      (void) ajFmtPrintF(outf, "The region of alignment only starts at position %d "
		  "of sequence %s", start1+1, namea);
      (void) ajFmtPrintF(outf, "Only the sequence of %s is being used before this "
		  "point\n\n", nameb);
    }
  } else if (start1 && start2) {	/* both the same length and */
                                        /* > 1 before the aligned region */
     (void) ajFmtPrintF(outf, "WARNING: *********************************");
     (void) ajFmtPrintF(outf, "There is an equal amount of unaligned sequence (%d) "
		 "at the start of the sequences."
		 ,start1);
     (void) ajFmtPrintF(outf, "Sequence %s is being arbitrarily chosen for the "
		 "merged sequence\n\n", namea);

     for (i=0; i<start1; i++) {
       (void) ajStrAppK(ms, a[i]);
     }
  }

/* header */
(void) ajFmtPrintF(outf, "# %s position base\t\t%s position base\tUsing\n", namea, nameb);


/* make the merged sequence */
/* point to the start of the alignment in the complete unaligned sequences */
  apos = start1;
  bpos = start2;

  for(i=0; i<olen; i++) {
    if (p[i]=='.' || p[i]==' ' || q[i]=='.' || q[i]==' ') {	/* gap! */
      if (bestquality(a, b, apos, bpos)) {
      	p[i] = toupper((int)p[i]);
      	if (p[i] != '.' && p[i] != ' ') (void) ajStrAppK(ms, p[i]); 
        (void) ajFmtPrintF(outf, "\t%d\t'%c'\t\t%d\t'%c'\t\t'%c'\n",
		    apos+1, p[i],bpos+1, q[i], p[i]);
      } else {
      	q[i] = toupper((int)q[i]);
      	if (q[i] != '.' && q[i] != ' ') (void) ajStrAppK(ms, q[i]); 
        (void) ajFmtPrintF(outf, "\t%d\t'%c'\t\t%d\t'%c'\t\t'%c'\n",
		    apos+1, p[i],bpos+1, q[i], q[i]);
      }      	      	

    } else if (p[i]=='n' || p[i]=='N') {
      q[i] = toupper((int)q[i]);
      (void) ajStrAppK(ms, q[i]);
/*      (void) ajFmtPrintF(outf, "Sequence %s position %d is 'n' - using Sequence %s: %c\n\n", namea, apos+1, nameb, q[i]);
*/
    } else if (q[i]=='n' || q[i]=='N') {
      p[i] = toupper((int)p[i]);
      (void) ajStrAppK(ms, p[i]);
/*      (void) ajFmtPrintF(outf, "Sequence %s position %d is 'n' - using Sequence %s: %c\n\n", nameb, bpos+1, namea, p[i]);
*/
    } else if (p[i] != q[i]) {
/* get the sequence with the best quality and use the base from that one */   
      if (bestquality(a, b, apos, bpos)) {
        p[i] = toupper((int)p[i]);
        (void) ajStrAppK(ms, p[i]);   
        (void) ajFmtPrintF(outf, "\t%d\t'%c'\t\t%d\t'%c'\t\t'%c'\n",
		    apos+1, p[i],bpos+1, q[i], p[i]);
      } else {
        q[i] = toupper((int)q[i]);
        (void) ajStrAppK(ms, q[i]);   
        (void) ajFmtPrintF(outf, "\t%d\t'%c'\t\t%d\t'%c'\t\t'%c'\n",
		    apos+1, p[i],bpos+1, q[i], q[i]);
      }

    } else {  /* both the same */
      (void) ajStrAppK(ms, p[i]);
    }

/* update the positions in the unaligned complete sequences */    
    if (p[i] != '.' &&  p[i] != ' ') apos++;
    if (q[i] != '.' &&  q[i] != ' ') bpos++;
  }  

/* output the right hand side */
  alen=strlen(&a[apos]);
  blen=strlen(&b[bpos]);

  if (alen > blen) {
    (void) ajStrAppC(ms, &a[apos]);
    if (blen) {
      (void) ajFmtPrintF(outf, "WARNING: *********************************");
      (void) ajFmtPrintF(outf, "The region of alignment ends at position %d of "
		  "sequence %s", bpos+1, nameb);
      (void) ajFmtPrintF(outf, "Only the sequence of %s is being used after "
		  "this point\n\n", namea);
    }
    
  } if (blen > alen) {
    (void) ajStrAppC(ms, &b[bpos]);
    if (alen) {
      (void) ajFmtPrintF(outf, "WARNING: *********************************");
      (void) ajFmtPrintF(outf, "The region of alignment ends at position %d of "
		  "sequence %s", apos+1, namea);
      (void) ajFmtPrintF(outf, "Only the sequence of %s is being used after this "
		  "point\n\n", nameb);
    }
    

  } else if (alen && blen) {	/* both the same length and > 1 */
     (void) ajFmtPrintF(outf, "WARNING: *********************************");
     (void) ajFmtPrintF(outf, "There is an equal amount of unaligned sequence (%d) "
		 "at the end of the sequences.", alen);
     (void) ajFmtPrintF(outf, "Sequence %s is being arbitrarily chosen for the "
		 "merged sequence\n\n", namea);
     (void) ajStrAppC(ms, &a[apos]);
  }
}

/* @func bestquality *******************************************
**
** Return ajTrue if the first sequence has the best quality
** If both sequences have the same quality, pick the first
**
** @param [r] a [char*] First sequence
** @param [r] b [char*] Second sequence
** @param [r] apos [int] Position in first sequence
** @param [r] bpos [int] Position in second sequence
** @return [AjBool] ajTrue = first sequence is the best quality at this point
**
******************************************************************************/

AjBool bestquality (char * a, char *b, int apos, int bpos) {

  float qa, qb;

  qa = quality(a, apos, 5);
  qb = quality(b, bpos, 5);

  if (qa == qb) {	/* both have the same quality, use a larger window */
    qa = quality(a, apos, 10);
    qb = quality(b, bpos, 10);
  } 
  
  if (qa == qb) {	/* both have the same quality, use a larger window */
    qa = quality(a, apos, 20);
    qb = quality(b, bpos, 20);
  } 
  
  if (qa >= qb) {     /*  both have the same quality, use the first sequence */
    return ajTrue;
  } else {
    return ajFalse;
  }

}

/* @func quality *******************************************
**
** Calculate the quality of a window of a sequence
**
** quality = sequence value/length under window either side of a position
**
** sequence value = sum of points in that subsequence
**
** good bases = 2 points
**
** ambiguous bases = 1 point
**
** N's = 0 points
**
** off end of the sequence = 0 points
**
** THIS HEAVILY DISCRIMINATES AGAINST THE IFFY BITS AT THE END OF
** SEQUENCE READS
**
** @param [r] seq [char*] Sequence
** @param [r] pos [int] Position
** @param [r] window [int] Window size
** @return [float] quality of the window
**
******************************************************************************/

float quality (char * seq, int pos, int window) {

  int value=0;
  int i;	

  for (i=pos; i<pos+window && i < strlen(seq); i++) {
    if (strchr("aAcCgGtTuU", seq[i])) {
      value+=2;	/* good bases count for two points */
    } else if (strchr("mrwsykvhdbMRWSYKVHDB", seq[i])) {
      value++;	/* ambiguous bases count for only one point */
    } /* N's count for zero points */
  }

  for (i=pos-1; i>pos-window && i>=0; i--) {
    if (strchr("aAcCgGtTuU", seq[i])) {
      value+=2;	/* good bases count for two points */
    } else if (strchr("mrwsykvhdbMRWSYKVHDB", seq[i])) {
      value++;	/* ambiguous bases count for only one point */
    } /* N's count for zero points */
  }

  return (double)value/(double)(window*2+1);
}


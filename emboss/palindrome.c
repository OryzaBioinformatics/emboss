/* Written by Mark Faller 26 November 1998*/
/* The algorithm pulls out palindromes and inverted repeats*/
/* Mismatches are allowed but no gaps*/

#include "emboss.h"
#include <stdlib.h>
#include <stdio.h>

AjBool overlap;

typedef struct palindrome{
   int forwardStart;
   int forwardEnd;
   int revStart;
   int revEnd;
   struct palindrome *next;
} *Palindrome;

static Palindrome newPalindrome( int fstart, int fend, int rstart, int rend);
static AjBool palindromeAInB( Palindrome a, Palindrome b);
static AjBool palindromeAOverB( Palindrome a, Palindrome b);
static AjBool palindromeOver ( int astart, int aend, int bstart, int bend);
static void printPalindrome( AjPFile outfile, AjPStr seq, Palindrome pal);
static AjBool palindromeLonger( Palindrome a, Palindrome b );
static void palindromeSwap ( Palindrome a, Palindrome b );

static Palindrome newPalindrome( int fstart, int fend, int rstart, int rend) {

   Palindrome pal;

   AJNEW(pal);
   pal->forwardStart = fstart;
   pal->forwardEnd = fend;
   pal->revStart = rstart;
   pal->revEnd = rend;
   pal->next = NULL;
   return pal;
}

static AjBool palindromeAInB( Palindrome a, Palindrome b) {

  if ((a->forwardStart >= b->forwardStart) &&
      (a->forwardEnd <=b->forwardEnd)) {
    if ((a->revStart <= b->revStart) &&
	(a->revEnd >= b->revEnd)) {
      return AJTRUE;
    }
  }

  return AJFALSE;
}

static AjBool palindromeAOverB( Palindrome a, Palindrome b) {

  /*ajDebug ("overlap %d..%d %d..%d\n",
	  a->forwardStart, a->forwardEnd,
	   a->revStart, a->revEnd);
  ajDebug ("   with %d..%d %d..%d\n",
	   b->forwardStart, b->forwardEnd,
	   b->revStart, b->revEnd);*/
  if (palindromeOver(a->forwardStart, a->forwardEnd,
		     b->forwardStart, b->forwardEnd) &&
      palindromeOver(a->revEnd, a->revStart,
		     b->revEnd, b->revStart)) {
    return AJTRUE;
  }

  return AJFALSE;
}

static AjBool palindromeOver( int astart, int aend, int bstart, int bend) {
  if (astart >= bstart && astart <= bend) return ajTrue;
  if (bstart >= astart && bstart <= aend) return ajTrue;
  return ajFalse;
}

static AjBool palindromeLonger( Palindrome a, Palindrome b ) {
  if ((a->forwardEnd - a->forwardStart) >
      (b->forwardEnd - b->forwardStart))
    return ajTrue;
  return ajFalse;
}

static void palindromeSwap ( Palindrome a, Palindrome b ) {
  b->forwardStart =  a->forwardStart;
  b->forwardEnd =  a->forwardEnd;
  b->revStart =  a->revStart;
  b->revEnd =  a->revEnd;
  return;
}


static void printPalindrome( AjPFile outfile, AjPStr seq, Palindrome pal) {

   int i;

   ajFmtPrintF( outfile, "%-5d ", (pal->forwardStart+1));
   for (i = pal->forwardStart; i < pal->forwardEnd; i++) {
      ajFmtPrintF( outfile, "%c", ajStrChar( seq, i));
   }
   ajFmtPrintF(outfile, " %5d\n      ", pal->forwardEnd);
   for (i = pal->forwardStart; i < pal->forwardEnd; i++) {
      ajFmtPrintF( outfile, "|");
   }
   ajFmtPrintF( outfile, "\n%-5d ", (pal->revStart+1));
   for (i = pal->revStart; i > pal->revEnd; i--) {
      ajFmtPrintF( outfile, "%c", ajStrChar(seq, i));
   }
   ajFmtPrintF( outfile, " %5d\n\n", (pal->revEnd+2));

}

int main( int argc, char * argv[]) {

   AjPSeq sequence;
   AjPFile outfile;
   int minLen, maxLen, /*minGap,*/ maxGap, beginPos, endPos, maxmismatches;

   AjPStr seqstr = NULL;
   int current, rev, count, gap;

   int begin, end, mismatches, mismatchAtEnd;
   int istart, iend;
   int ic, ir;

   Palindrome pfirstpal = NULL;
   Palindrome plastpal = NULL;
   Palindrome ppal = NULL;
   Palindrome pnext = NULL;

   AjBool found = AJFALSE;

   embInit("palindrome", argc, argv);

   /*   minGap = 0;*/
   sequence = ajAcdGetSeq( "insequence");
   minLen = ajAcdGetInt( "minpallen");
   maxLen = ajAcdGetInt( "maxpallen");
   maxGap = ajAcdGetInt( "gaplimit");
   outfile = ajAcdGetOutfile( "outfile");
   beginPos = ajSeqBegin( sequence );
   endPos = ajSeqEnd( sequence );
   maxmismatches = ajAcdGetInt( "nummismatches");
   overlap = ajAcdGetBool("overlap");

/*write header to file*/

   ajFmtPrintF( outfile, "Palindromes of:  %s \n", ajSeqName( sequence));
   ajFmtPrintF( outfile, "Sequence length is: %d \n", ajSeqLen( sequence));
   ajFmtPrintF( outfile, "Start at position: %d\nEnd at position: %d\n",
                beginPos, endPos);
   ajFmtPrintF( outfile, "Minimum length of Palindromes is: %d \n", minLen);
   ajFmtPrintF( outfile, "Maximum length of Palindromes is: %d \n", maxLen);
   ajFmtPrintF( outfile, "Maximum gap between elements is: %d \n", maxGap);
   ajFmtPrintF( outfile, "Number of mismatches allowed in Palindrome: %d\n", 
                maxmismatches);
   ajFmtPrintF( outfile, "\n\n\n");
   ajFmtPrintF( outfile, "Palindromes:\n");

/*check sequence is of type nucleotide else return error*/
   if (!ajSeqIsNuc( sequence)) {
      ajFmtPrintF( outfile, "Error, sequence must be a nucleotide sequence");
      ajExit();
   }


/*set vars in readiness to enter loop*/
   seqstr = ajStrNewC(ajSeqChar( sequence));
   begin = beginPos - 1;
   end = endPos - 1;

/*loop to look for inverted repeats*/
   for (current = begin; current < end; current++) {
      iend = current + 2*(maxLen) + maxGap;
      if (iend > end) iend = end;
      istart = current + minLen;

      for (rev = iend; rev > istart; rev--) {
	 count = 0;
	 mismatches = 0;
	 mismatchAtEnd = 0;
	 ic = current;
	 ir = rev;
         if (ajStrChar(seqstr, ic) ==
	     ajSeqBaseComp(ajStrChar(seqstr, ir))) {
            while (mismatches <= maxmismatches && ic < ir) {
               if (ajStrChar(seqstr, ic++) ==
		   ajSeqBaseComp(ajStrChar(seqstr, ir--))) {
                  mismatchAtEnd = 0;
               } else {
                  mismatches++;
                  mismatchAtEnd++;
               }
               count++;
            }
         }
         count -=mismatchAtEnd;
	 gap = rev - current - count - count + 1;

/* Find out if we have found reverse repeat long enough*/
         if (count >= minLen && gap <= maxGap) {

/*create new palindrome struct to hold new palindrome data*/ 
            ppal = newPalindrome(current,(current+count),rev,(rev-count));

/*if it is our first palindrome find then save it as start of palindrome list*/
            if (pfirstpal == NULL) {
               pfirstpal = ppal;
               plastpal = ppal;
            }
	    else {

/*check this isn't a subset of a palindrome already found*/
               pnext = pfirstpal;
               found = AJFALSE;
               while (pnext != NULL) {
                  if (overlap && palindromeAInB( ppal, pnext)) {
                     found = AJTRUE;
                     break;
                  }
                  if (!overlap && palindromeAOverB( ppal, pnext)) {
		     if (palindromeLonger(ppal, pnext)) {
		        ajDebug("swap...\n");
		        palindromeSwap(ppal, pnext);
		     }
		     else {
		       ajDebug("keep...\n");
		     }
                     found = AJTRUE;
                     break;
                  }
                  pnext = pnext->next;
               }

/*if new palindrome add to end of list*/
               if (!found) {
                  plastpal->next = ppal;
                  plastpal = ppal;
               } else {
                  AJFREE (ppal);
               }
            }
         }
      }
   }



/*Print out palindromes*/
   ppal = pfirstpal;
   while (ppal != NULL) {
      printPalindrome( outfile, seqstr, ppal);
      ppal = ppal->next;
   }


/*free memory used for palindrome list*/
   ppal = pfirstpal;
   while (ppal != NULL) {
      ppal = ppal->next;
      AJFREE (ppal);
   }

   ajExit();
   return 0;
}

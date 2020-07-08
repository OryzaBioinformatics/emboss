/*  Last edited: Mar  1 17:23 2000 (pmr) */
#include "emboss.h"
#include <math.h>
#include <limits.h>

/*
* This set of routines was written primarily for the compseq program.
* Feel free to use it for anything you want.
* 
* compseq counts the frequency of n-mers (or words) in a sequence.
*
* The easiest way to do this was to make a big unsigned long int array 
* to hold the counts of each type of word. 
*
* I needed a way of converting a sequence word into an integer so that I 
* could increment the appropriate element of the array. 
* (embNmerNuc2int and embNmerProt2int)
* 
* I also needed a way of converting an integer back to a short sequence
* so that I could display the word. 
* (embNmerInt2nuc and embNmerInt2prot)
* 
* embNmerGetNoElements returns the number of elements required to store the
* results.
* In other words it is the number of possible words of size n.
* 
* Gary Williams
* 
*/


/* @func embNmerNuc2int ************************************************
**
** Encodes a word of a nucleic sequence into a unsigned long int number.
** For a 4-byte unsigned long int this will work if the word size
** is less than 16, otherwise there aren't enough bits to hold the number.
**
** @param [r] seq [char *] the sequence to use
** @param [r] wordsize [int] the size of word of sequence to convert to int
** @param [r] offset [int] the offset into the sequence of the start of
**                         the word
** @param [w] otherflag [AjBool *] set true if the sequence contains
**                                 anything except A,G,C or T
** 
** @return [unsigned long int] the encoded word
******************************************************************************/

unsigned long int embNmerNuc2int(char *seq, int wordsize, int offset,
				AjBool *otherflag) {

  int i;
  unsigned long int result = 0;
  char c;

  *otherflag = ajFalse;

  for (i=0; i<wordsize; i++) { 
    c = seq[offset+i];
    result <<= 2;
    
    if (c == 'A') {
      /* result |= 0; */
    } else if (c == 'C') {
      result |= 1;
    } else if (c == 'G') {
      result |= 2;
    } else if (c == 'T') {
      result |= 3;
    } else {
      *otherflag = ajTrue;
      return 0;
    }
  }
  return result;
}

/* @func embNmerInt2nuc ************************************************
**
** Decodes a unsigned long int number into a word of a nucleic sequence.
**
** @param [w] seq [AjPStr *] the returned sequence
** @param [r] wordsize [int] the size of word to produce
** @param [r] value [unsigned long int] the number to decode
** 
** @return [int] not used
******************************************************************************/

int embNmerInt2nuc(AjPStr *seq, int wordsize, unsigned long int value) {
char bases[] = "ACGT"; 
char store[2];
int i;

  store[1] = '\0';	/* make a one-character NULL-teminated char* */

  for (i=0; i<wordsize; i++) {
    store[0] = bases[value & 3];
    (void) ajStrInsertC(seq, 0, store);
    value >>= 2;
  }
return 1;
}

/* @func embNmerProt2int ************************************************
**
** Encodes a word of a protein sequence into a unsigned long int number.
** For a 4-byte unsigned long int this will work if the word size
** is less than 8, otherwise there aren't enough bits to hold the number.
**
** @param [r] seq [char *] the sequence to use
** @param [r] wordsize [int] the size of word of sequence to convert to int
** @param [r] offset [int] the offset into the sequence of the start of
**                         the word
** @param [w] otherflag [AjBool *] set true if the sequence contains
**                                 anything other than valid amino acid codes
** @param [r] ignorebz [AjBool] true if B and Z are to be treated as
**                              non-valid residues
** 
** @return [unsigned long int] the encoded word
******************************************************************************/

unsigned long int embNmerProt2int(char *seq, int wordsize, int offset,
				 AjBool *otherflag, AjBool ignorebz) {
  int i;
  unsigned long int result = 0;
/* MATHOG, it's used as an int, so let it be an int 
  signed char c;
*/
  int c;
  int aa;
  int noaa;
  int *table;
  
/*                  a   b  c  d  e  f  g  h  i   j  k  l  m   n    o  p   q   
  r   s   t   u   v   w    x  y    z */
  int table21[] = { 0, -1, 1, 2, 3, 4, 5, 6, 7, -1, 8, 9, 10, 11, -1, 12, 13, 
  14, 15, 16, 17, 18, 19, -1, 20, -1, -1, -1, -1, -1, -1};

/*                  a  b  c  d  e  f  g  h  i   j  k  l   m   n    o  p   q   
  r   s   t   u   v   w    x  y    z */
  int table23[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, -1, 9, 10, 11, 12, -1, 13, 14, 
  15, 16, 17, 18, 19, 20, -1, 21, 22, -1, -1, -1, -1, -1};

/* see whether we are including B and Z as acceptable amino acids or not */
  if (ignorebz) {
    table = table21;
    noaa = 21;
  } else {
    table = table23;
    noaa = 23;
  }

  *otherflag = ajFalse;
  for (i=0; i<wordsize; i++) { 
    c = (int) seq[offset+i];
    result *= noaa;
    c = c - (int) 'A';
    if (c < 0 || c > 31) {
      *otherflag = ajTrue;
      return 0;
    }
    aa = table[c];
    if (aa == -1) {
      *otherflag = ajTrue;
      return 0;
    }

    result += aa;

  }
  return result;
}


/* @func embNmerInt2prot ************************************************
**
** Decodes a unsigned long int number into a word of a protein sequence.
**
** @param [w] seq [AjPStr *] the returned sequence
** @param [r] wordsize [int] the size of word to produce
** @param [r] value [unsigned long int] the number to decode
** @param [r] ignorebz [AjBool] true if B and Z are to be treated
**                                as non-valid residues
** 
** @return [int] not used
******************************************************************************/

int embNmerInt2prot(AjPStr *seq, int wordsize, unsigned long int value,
		   AjBool ignorebz) {

char aas21[] = "ACDEFGHIKLMNPQRSTUVWY"; 
char aas23[] = "ABCDEFGHIKLMNPQRSTUVWYZ"; 
char store[2];
int i;
int noaa;
char *aas;

/* see whether we are including B and Z as acceptable amino acids or not */
  if (ignorebz) {
    aas = aas21;
    noaa = 21;
  } else {
    aas = aas23;
    noaa = 23;
  }

  store[1] = '\0';	/* make a one-character NULL-teminated char* */

  for (i=0; i<wordsize; i++) {
    store[0] = aas[value % noaa];
    (void) ajStrInsertC(seq, 0, store);
    value /= noaa;
  }
return 1;
}

/* @func embNmerGetNoElements ************************************************
**
** Calculates the maximum number required to encode a sequence of size 'word'.
**
** @param [w] no_elements [unsigned long int *] the returned number
** @param [r] word [int] the size of word to produce
** @param [r] seqisnuc [AjBool] True is the sequence is nucleic,
**                              False if protein
** @param [r] ignorebz [AjBool] true if B and Z are to be treated as
**                                non-valid residues
** 
** @return [AjBool] True if the word is small enough to be encoded
**                   in a unsigned long int
******************************************************************************/

AjBool embNmerGetNoElements(unsigned long int *no_elements, int word,
			   AjBool seqisnuc, AjBool ignorebz) {

  double result;
  float ccfix = (float) word; /* (il) cc on alphas screws up otherwise */
   static unsigned long int maxlong = UINT_MAX;

  /* we use UINT_MAX here because the gcc compiler defines ULONG_MAX
     as a strange expression including LONG_MAX which is signed
     and this makes the whole conversion use signed values, and
     maybe even int rather than long */

  ajDebug ("embNmerGetNoElements ( %d, %b, %b )\n", word, seqisnuc, ignorebz);
  if (seqisnuc) {
    result = pow(4.0, ccfix); 
  } else {
    if (ignorebz) {
      result =  pow(21.0, ccfix);
    } else {
      result =  pow(23.0, ccfix);
    }
  }

  ajDebug ("...result: %.3f max: %.3f\n", result, (double) maxlong);

/* if the resulting number is too long for a unsigned long int, return false */
  if (result-1 > (double)maxlong) return ajFalse;

  *no_elements = (unsigned long int)result;
  return ajTrue;

}




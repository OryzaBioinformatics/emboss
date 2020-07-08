/*  Last edited: Mar  1 17:23 2000 (pmr) */
#ifdef __cplusplus
extern "C"
{
#endif

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

#ifndef embnmer_h
#define embnmer_h


unsigned long int embNmerNuc2int (char *seq, int wordsize, int offset,
				  AjBool *otherflag);
int               embNmerInt2nuc (AjPStr *seq, int wordsize,
				  unsigned long int value);
unsigned long int embNmerProt2int (char *seq, int wordsize, int offset,
				   AjBool *otherflag, AjBool ignorebz);
int               embNmerInt2prot (AjPStr *seq, int wordsize,
				   unsigned long int value, AjBool ignorebz);
AjBool            embNmerGetNoElements (unsigned long int *no_elements,
					int word, AjBool seqisnuc,
					AjBool ignorebz);


#endif

#ifdef __cplusplus
}
#endif

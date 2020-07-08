#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtranslate_h
#define ajtranslate_h

#include "ajax.h"

/* @data AjPTrn *******************************************************
**
** Ajax Sequence translation object.
**
** Holds the Genetic Code specification and information needed to translate
** the sequence and find initiation sites.
**
** @new ajTrnNew Default constructor
** @new ajTrnNewI Default constructor
** @new ajTrnNewC Default constructor
** @new ajTrnNewPep Peptide object constructor
** @delete ajTrnDel Default destructor
** @set ajTrnReadFile Reads a Genetic Code file
** @use ajTrnCodon Translating a codon from a AjPStr
** @use ajTrnRevCodon Reverse complement translating a codon from a AjPStr
** @use ajTrnCodonC Translating a codon from a char *
** @use ajTrnRevCodonC Translating a codon from a AjPStr
** @use ajTrnCodonK Translating a codon from a char * to a char
** @use ajTrnRevCodonK Reverse complement translating a codon from a char * to a char
** @use ajTrnC Translating a sequence from a char *
** @use ajTrnRevC Reverse complement translating a sequence from a char *
** @use ajTrnStr Translating a sequence from a AjPStr
** @use ajTrnRevStr Reverse complement translating a sequence from a AjPStr
** @use ajTrnSeq Translating a sequence from a AjPSeq
** @use ajTrnRevSeq Reverse complement translating a sequence from a AjPSeq
** @use ajTrnCFrame Translating a sequence from a char * in a frame
** @use ajTrnStrFrame Translating a sequence from a AjPStr in a frame
** @use ajTrnSeqFrame Translating a sequence from a AjPSeq in a frame
** @use ajTrnSeqOrig Translating a sequence (DEPRECATED)
** @use ajTrnStrOrig Translating a sequence (DEPRECATED)
** @use ajTrnGetTitle Returns description of the translation table
** @use ajTrnGetFileName Returns file name the translation table was read from
** @use ajTrnStartStop Checks whether the input codon is a Start codon, a Stop codon or something else
** @use ajTrnStartStopC Checks whether a const char * codon is a Start codon, a Stop codon or something else
** @@
******************************************************************************/

typedef struct AjSTrn {
  AjPStr FileName;		/* name of file that held the data */
  AjPStr Title;			/* title of data read from file */
  char GC[15][15][15];		/* genetic codon table */
  char Starts[15][15][15];	/* initiation site table */
} AjOTrn, *AjPTrn;


/* table to convert character of base to translation array element value */
/*static int trnconv[] = {*/
/* characters less than 64 */
/*  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, */

/* @  A   B  C   D   E   F  G   H   I   J  K   L  M   N   O*/
/*  14, 0, 13, 1, 12, 14, 14, 2, 11, 14, 14, 9, 14, 4, 14, 14,*/

/* P   Q  R  S  T  U   V  W   X  Y   Z   [   \   ]   ^   _ */
/*  14, 14, 5, 7, 3, 3, 10, 6, 14, 8, 14, 14, 14, 14, 14, 14,*/

/* `  a   b  c   d   e   f  g   h   i   j  k   l  m   n   o */
/*  14, 0, 13, 1, 12, 14, 14, 2, 11, 14, 14, 9, 14, 4, 14, 14,*/

/* p   q  r  s  t  u   v  w   x  y   z   {   |   }   ~   del */
/*  14, 14, 5, 7, 3, 3, 10, 6, 14, 8, 14, 14, 14, 14, 14, 14
};*/





void ajTrnDel (AjPTrn* pthis);
AjPTrn ajTrnNew (AjPStr trnFileName);
AjPTrn ajTrnNewI (int trnFileNameInt);
AjPTrn ajTrnNewC (char *trnFileName);
void ajTrnReadFile (AjPTrn trnObj, AjPFile trnFile);
AjPSeq ajTrnNewPep(AjPSeq nucleicSeq, int frame);
AjPStr ajTrnCodon (AjPTrn trnObj, AjPStr codon);
AjPStr ajTrnRevCodon (AjPTrn trnObj, AjPStr codon);
AjPStr ajTrnCodonC (AjPTrn trnObj, char *codon);
AjPStr ajTrnRevCodonC (AjPTrn trnObj, char *codon);
char ajTrnCodonK (AjPTrn trnObj, char *codon);
char ajTrnRevCodonK (AjPTrn trnObj, char *codon);
void ajTrnC (AjPTrn trnObj, char *str, int len, AjPStr *pep);
void ajTrnRevC (AjPTrn trnObj, char *str, int len, AjPStr *pep);
void ajTrnStr (AjPTrn trnObj, AjPStr str, AjPStr *pep);
void ajTrnRevStr (AjPTrn trnObj, AjPStr str, AjPStr *pep);
void ajTrnSeq (AjPTrn trnObj, AjPSeq seq, AjPStr *pep);
void ajTrnRevSeq (AjPTrn trnObj, AjPSeq seq, AjPStr *pep);
void ajTrnCFrame (AjPTrn trnObj, char *seq, int len, int frame, AjPStr *pep);
void ajTrnStrFrame (AjPTrn trnObj, AjPStr seq, int frame, AjPStr *pep);
void ajTrnSeqFrame (AjPTrn trnObj, AjPSeq seq, int frame, AjPStr *pep);
AjPSeq ajTrnSeqOrig (AjPTrn trnObj, AjPSeq trnSeq, int frame);
AjPStr ajTrnStrOrig (AjPTrn trnObj, AjPStr trnSeq, int frame);
AjPStr ajTrnGetTitle (AjPTrn thys);
AjPStr ajTrnGetFileName (AjPTrn thys);
int ajTrnStartStop (AjPTrn trnObj, AjPStr codon, char *aa);
int ajTrnStartStopC (AjPTrn trnObj, char *codon, char *aa);
       	 
#endif

#ifdef __cplusplus
}
#endif

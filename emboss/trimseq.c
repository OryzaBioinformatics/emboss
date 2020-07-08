/* @source trimseq application
**
** Trim ambiguous bits off the ends of sequences
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

/*  Last edited: Aug 16 13:08 1999 (pmr) */


#include "emboss.h"
#include <ctype.h>	/* for tolower, toupper */

int trim (AjPSeq seq, int sense, AjBool isnuc, int window, float percent, AjBool strict, AjBool star);
void parole(AjBool *gang, char *good_guys);
void arrest(AjBool *gang, char *bad_guys);

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq = NULL;
  int window;
  AjBool left, right, strict, star;
  float percent;
  AjBool isnuc;
  AjPStr str=NULL; /* NULL definition is required for the ajStrAss() call */
  int start, end;


  (void) embInit ("trimseq", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  seqout = ajAcdGetSeqoutall ("outseq");
  window = ajAcdGetInt ("window");
  percent = ajAcdGetFloat ("percent");
  left = ajAcdGetBool ("left");
  right = ajAcdGetBool ("right");
  strict = ajAcdGetBool ("strict");
  star = ajAcdGetBool ("star");

  while (ajSeqallNext(seqall, &seq)) {



/* is this a protein or nucleic sequence? */
    isnuc = ajSeqIsNuc(seq);

/* find the left start */
    if (left) {
      start = trim(seq, 1, isnuc, window, percent, strict, star) + 1;
    } else {
      start = 0;
    }

/* find the right end */
    if (right) {
      end = trim(seq, 0, isnuc, window, percent, strict, star) - 1;
    } else {
      end = ajSeqLen(seq)-1;
    }


/* get a COPY of the sequence string */
    (void) ajStrAss (&str, ajSeqStr(seq));

    (void) ajStrSub(&str, start, end);

    (void) ajSeqReplace(seq, str);

    (void) ajSeqAllWrite (seqout, seq);
  }

  (void) ajSeqWriteClose (seqout);

  (void) ajExit ();
  return 0;

}

/***********************************************************************
 trim 

Parameters:

AjPSeq sequence
int sense	1 = trim from left, 0 = trim from right
AjBool isnuc
int window
float percent
AjBool strict	trim off all IUPAC ambiguity codes, not just X, N
AjBool star	trim off asterisks in proteins

Returns:
int position to trim to or -1 or ajSeqLen(seq) if no bad characters were found

*/

int trim (AjPSeq seq, int sense, AjBool isnuc, int window, float percent, AjBool strict, AjBool star) {
	
  int leroy_brown;	/* last bad character */
  int suspect;		/* possible last bad character */
  AjBool gang[256];	/* all the characters - true if a bad one to be removed */
  int i;
  int a;		/* start position of window */
  int z;		/* position to end moving window to */
  int inc;		/* increment value to move window (+-1) */
  int count;		/* count of bad characters in window */
  int look;		/* position in wind we are looking at */
  float pc;		/* percentage of bad characters in this window */
  char c;

/* set the characters to trim */
  for (i=0; i<256; i++) {	/* set them all to be bad initially */
    gang[i] = ajTrue;
  }
  if (isnuc) {
    (void) parole(gang, "acgtu.-~ ");	/* normal bases and gap characters are good guys */
    if (!strict) {
      (void) parole(gang, "mrwsykvhdb");	/* so are ambiguity codes if we are not strict */
    }
  } else {
/* protein */
    (void) parole(gang, "arndcqeghilkmfpstwyv.-~ ");	/* normal residues and gap characters are good guys */
    if (!strict) {
      (void) parole(gang, "bz");		/* so are ambiguity codes if we are not strict */
    }
    if (star) {
      (void) parole(gang, "*");		/* so is an asterisk if we want them */
    }
  }

/* start loop - see which way we are moving */
  if (sense) {
    a = 0;
    z = ajSeqLen(seq) - window;
    inc = 1;
    leroy_brown = -1;
    suspect = -1;
  } else {
    a = ajSeqLen(seq)-1;
    z = window;
    inc = -1;
    leroy_brown = ajSeqLen(seq);
    suspect = ajSeqLen(seq);
  }

/* do an initial trim of contiguous runs of bad characters from the ends */
/* we always trim gaps from the end */
  for (; a != z; a += inc) {
    c = (ajSeqChar(seq))[a];
    if (gang[(int)c] || c == '.' || c == '-' || c == '~' || c == ' ') {	/* trim if we have a bad character or a gap character at the end */
      leroy_brown = a;	/* want to trim down to here */
    } else {
      break;
    }
  }

/* do the window trim of the remainder of the sequence */
  for (; a != z; a += inc) {
/* look in the window */
    for (count = 0, look = 0; look < window && look > -window; look += inc) {
      c = (ajSeqChar(seq))[a+look];
      if (gang[(int)c]) {
/* count the bad characters */
        count++;
/* remember the last bad character position in this window */
        suspect = a+look;
      }
    }
/* what is the percentage of bad characters in this window */
    pc = 100.0 * (float)count/(float)window;
/* do we want to trim this window? */
    if (pc < percent) break;
    if (sense) {
      if (suspect > leroy_brown) leroy_brown = suspect;
    } else {
      if (suspect < leroy_brown) leroy_brown = suspect;
    }
  }
  
/* do a final tidy up of gap characters left at the new end of the sequence */
/* we always trim gaps from the end */
  for (a = leroy_brown+inc; a != z; a += inc) {
    c = (ajSeqChar(seq))[a];
    if (c == '.' || c == '-' || c == '~' || c == ' ') {	/* trim if we have a gap character at the end */
      leroy_brown = a;	/* want to trim down to here */
    } else {
      break;
    }
  }

  return leroy_brown;

}
/***********************************************************************
 parole
 sets the upper and lowercase characters in the array 'gang' to be ajFalse

Parameters:
AjBool *gang	- array of flags for whether a character is required or not
char *good_guys	- string of chars that are required

*/

void parole(AjBool *gang, char *good_guys) {
	
  int i;

  for (i=0; good_guys[i]; i++) {
    gang[tolower((int) good_guys[i])] = ajFalse;
    gang[toupper((int) good_guys[i])] = ajFalse;
  }
}
	
/***********************************************************************
 arrest
 resets the upper and lowercase characters in the array 'gang' to be ajTrue

Parameters:
AjBool *gang	- array of flags for whether a character is required or not
char *bad_guys	- string of chars that are not required

*/

void arrest(AjBool *gang, char *bad_guys) {
	
  int i;

  for (i=0; bad_guys[i]; i++) {
    gang[tolower((int) bad_guys[i])] = ajTrue;
    gang[toupper((int) bad_guys[i])] = ajTrue;
  }
}
	


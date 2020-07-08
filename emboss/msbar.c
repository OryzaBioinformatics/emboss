/* @source msbar application
**
** Mutate sequence beyond all recognition
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


/*  Last edited: 10 Aug 1999 (GWW) */


#include "emboss.h"
#include <ctype.h>	/* for tolower, toupper */

static void blockmutn(AjPStr *str, AjBool isnuc, AjPStr *blocklist, int min, int max, AjBool inframe);
static void codonmutn(AjPStr *str, AjBool isnuc, AjPStr *codonlist, AjBool inframe);
static void pointmutn(AjPStr *str, AjBool isnuc, AjPStr *pointlist);
static void Insert(AjPStr *str, AjBool isnuc, int start, int end);
static void Move(AjPStr *str, int start, int end, int destination);
static void Duplicate(AjPStr *str, int start, int end);

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq = NULL;

  AjBool isnuc;
  AjPStr str=NULL; /* NULL definition is required for the ajStrAss() call */

  AjPStr *pointlist;
  AjPStr *codonlist;
  AjPStr *blocklist;

  int i, count, min, max;
  AjBool inframe;

  (void) embInit ("msbar", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  seqout = ajAcdGetSeqoutall ("outseq");

  pointlist = ajAcdGetList ("point");
  codonlist = ajAcdGetList ("codon");
  blocklist = ajAcdGetList ("block");

  count = ajAcdGetInt ("count");
  inframe = ajAcdGetBool ("inframe");
  min = ajAcdGetInt ("minimum");
  max = ajAcdGetInt ("maximum");

  while (ajSeqallNext(seqall, &seq)) {

/* is this a protein or nucleic sequence? */
    isnuc = ajSeqIsNuc(seq);

/* get a copy of the sequence string */
    (void) ajStrAss (&str, ajSeqStr(seq));

/* seed the random number generator */
    (void) ajRandomSeed();

/* do the mutation operations */
    for (i=0; i<count; i++) {

      (void) blockmutn(&str, isnuc, blocklist, min, max, inframe);
      (void) codonmutn(&str, isnuc, codonlist, inframe);
      (void) pointmutn(&str, isnuc, pointlist);

    }

    (void) ajSeqReplace(seq, str);
    (void) ajSeqAllWrite (seqout, seq);
  }

  (void) ajSeqWriteClose (seqout);

  (void) ajExit ();
  return 0;

}

/*******************************************************************************/

static void blockmutn(AjPStr *str, AjBool isnuc, AjPStr *blocklist, int min, int max, AjBool inframe) {
	
int i=-1;
int rposstart, rposend, rpos2;
int opt;

  while (blocklist[++i]) {
    (void) ajDebug("Next block mutation operation = %S\n", blocklist[i]);

/* None */
    if (!ajStrCmpC(blocklist[i], "0")) return;
      
/* get the option value */
    (void) ajStrToInt(blocklist[i], &opt);

/* if we want 'Any', then choose which one */
    if (opt == 1) {
      opt = ajRandomNumberD() * 5;
      opt += 2;
    }

/* get random block start and end positions in the sequence (0 to ajStrLen - 1) */
    if (inframe) {
      if (min < 3) min = 3;
      rposstart = ajRandomNumberD() * (double)ajStrLen(*str)/3;
      rposend = rposstart + (min/3) + ajRandomNumberD() * (double)((max - min)/3);
      rposstart *= 3;
      rposend *= 3;
      rposend--;
    } else {
      rposstart = ajRandomNumberD() * (double)ajStrLen(*str);
      rposend = rposstart + min + ajRandomNumberD() * (double)(max - min);
    }

    
    if (opt == 2) {
/* Insert */
      (void) ajDebug("block insert from %d to %d\n", rposstart, rposend);
      (void) Insert(str, isnuc, rposstart, rposend);

    }
    
    if (opt == 3) {
/* Delete */
      (void) ajDebug("block deletion from %d to %d\n", rposstart, rposend);
      (void) ajStrCut(str, rposstart, rposend);

    }
    
    if (opt == 4) {
/* Change */
      (void) ajDebug("block change from %d to %d\n", rposstart, rposend);
      (void) ajStrCut(str, rposstart, rposend);
      (void) Insert(str, isnuc, rposstart, rposend);      

    }
    
    if (opt == 5) {
/* Duplication */
      (void) ajDebug("block duplication from %d to %d\n", rposstart, rposend);
      (void) Duplicate(str, rposstart, rposend);

    }
    
    if (opt == 6) {
/* Move */
      if (inframe) {
        rpos2 = ajRandomNumberD() * (double)(ajStrLen(*str)/3);
        rpos2 *= 3;
      } else {
        rpos2 = ajRandomNumberD() * (double)ajStrLen(*str);
      }
      (void) ajDebug("block move from %d to %d to position %d\n", rposstart, rposend, rpos2);
      (void) Move(str, rposstart, rposend, rpos2);
    }


  }


}

/*******************************************************************************/

static void codonmutn(AjPStr *str, AjBool isnuc, AjPStr *codonlist, AjBool inframe) {
	
int rpos, rpos2;
int i=-1;
int opt;

  while (codonlist[++i]) {
    (void) ajDebug("Next codon mutation operation = %S\n", codonlist[i]);

/* None */
    if (!ajStrCmpC(codonlist[i], "0")) return;
      
/* get the option value */
    (void) ajStrToInt(codonlist[i], &opt);

/* if we want 'Any', then choose which one */
    if (opt == 1) {
      opt = ajRandomNumberD() * 5;
      opt += 2;
    }


/* get a random position in the sequence (0 to ajStrLen - 1) */
    if (inframe) {
      rpos = ajRandomNumberD() * (double)(ajStrLen(*str)/3);
      rpos *= 3;
    } else {
      rpos = ajRandomNumberD() * (double)ajStrLen(*str);
    }

    if (opt == 2) {
/* Insert */
      (void) ajDebug("codon insert at %d\n", rpos);
      (void) Insert(str, isnuc, rpos, rpos+2);      

    }
    
    if (opt == 3) {
/* Delete */
      (void) ajDebug("codon deletion at %d\n", rpos);
      (void) ajStrCut(str, rpos, rpos+2);

    }

    if (opt == 4) {
/* Change */
      (void) ajDebug("codon change at %d\n", rpos);
      (void) ajStrCut(str, rpos, rpos+2);
      (void) Insert(str, isnuc, rpos, rpos+2);      

    } 
    
    if (opt == 5) {
/* Duplication */
      (void) ajDebug("codon duplication at %d\n", rpos);
      (void) Duplicate(str, rpos, rpos+2);

    } 
    
    if (opt == 6) {
/* Move */
      if (inframe) {
        rpos2 = ajRandomNumberD() * (double)(ajStrLen(*str)/3);
        rpos2 *= 3;
      } else {
        rpos2 = ajRandomNumberD() * (double)ajStrLen(*str);
      }
      (void) ajDebug("codon move from %d to %d\n", rpos, rpos2);
      (void) Move(str, rpos, rpos+2, rpos2);
    }


  }



}

/*******************************************************************************/

static void pointmutn(AjPStr *str, AjBool isnuc, AjPStr *pointlist) {
	
int i=-1;
int rpos, rpos2;
int opt;

  while (pointlist[++i]) {
    (void) ajDebug("Next point mutation operation = %S\n", pointlist[i]);
/* None */
    if (!ajStrCmpC(pointlist[i], "0")) return;
      
/* get the option value */
    (void) ajStrToInt(pointlist[i], &opt);

/* if we want 'Any', then choose which one */
    if (opt == 1) {
      opt = ajRandomNumberD() * 5;
      opt += 2;
    }

/* get a random position in the sequence (0 to ajStrLen - 1) */
    rpos = ajRandomNumberD() * (double)ajStrLen(*str);

    if (opt == 2) {
/* Insert */
      (void) ajDebug("Point insert at %d\n", rpos);
      (void) Insert(str, isnuc, rpos, rpos);      

    } 

    if (opt == 3) {
/* Delete */
      (void) ajDebug("Point deletion at %d\n", rpos);
      (void) ajStrCut(str, rpos, rpos);

    } 
    
    if (opt == 4) {
/* Change */
      (void) ajDebug("Point change at %d\n", rpos);
      (void) ajStrCut(str, rpos, rpos);
      (void) Insert(str, isnuc, rpos, rpos);      

    } 
    
    if (opt == 5) {
/* Duplication */
      (void) ajDebug("Point duplication at %d\n", rpos);
      (void) Duplicate(str, rpos, rpos);

    } 

    if (opt == 6) {
/* Move */
      rpos2 = ajRandomNumberD() * (double)ajStrLen(*str);
      (void) ajDebug("Point move from %d to %d\n", rpos, rpos2);
      (void) Move(str, rpos, rpos, rpos2);
    }
  }

}

/*******************************************************************************/

static void Insert(AjPStr *str, AjBool isnuc, int start, int end) {

char nuc[] = "ACGT";
char prot[] = "ARNDCQEGHILKMFPSTWYV";
AjPStr ins=NULL;
int count = end - start +1;
int r;

  while (count--) {
    if (isnuc) {
      r = ajRandomNumberD() * (double)strlen(nuc);
      (void) ajStrAppK(&ins, nuc[r]);
    } else {
      r = ajRandomNumberD() * (double)strlen(prot);
      (void) ajStrAppK(&ins, prot[r]);      
    }
  }
(void) ajDebug("Inserting %S at %d\n", ins, start);
  (void) ajStrInsert(str, start, ins);
  (void) ajStrDel(&ins);

}

/*******************************************************************************/

static void Move(AjPStr *str, int start, int end, int destination) {

AjPStr mov=NULL;

  (void) ajStrAss(&mov, *str);
  (void) ajStrSub(&mov, start, end);
  (void) ajStrInsert(str, destination, mov);
  (void) ajStrDel(&mov);
                 
}

/*******************************************************************************/

static void Duplicate(AjPStr *str, int start, int end) {

  (void) Move(str, start, end, start);
                 
}

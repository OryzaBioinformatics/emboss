/* @source revseq application
**
** Reverse and complement a sequence
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

/*  Last edited: 1 Feb 1999 (ajb) */

#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPSeq seq;
  AjPSeqout seqout;
  AjBool reverse, complement;

  (void) embInit ("revseq", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  seqout = ajAcdGetSeqoutall ("outseq");

  reverse = ajAcdGetBool("reverse");
  complement = ajAcdGetBool("complement");

  while (ajSeqallNext(seqall, &seq)) {

/* is this a nucleotide sequence */
    if (ajSeqIsProt(seq)) {
      (void) ajFatal ("%s", "Input sequence must be nucleic");
    }

/* see if we have both reverse and complement */
    if (reverse && complement) {
      (void) ajSeqReverse(seq);    /* reverses and complements */
    } else if (reverse) {
      (void) ajSeqRevOnly(seq);
    } else {
      (void) ajSeqCompOnly(seq);
    }
    (void) ajSeqAllWrite (seqout, seq);

  }
  
  (void) ajSeqWriteClose (seqout);

  (void) ajExit ();
  return 0;
}

/*  Last edited: Jan 12 14:34 2000 (gww) */
/* @source showseq application
**
** Display a sequence with translations, features and other bits
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 14 Sept 1999 - GWW - written
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

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq = NULL;
  int n;
  int count;
  
  embInit ("nthseq", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  n = ajAcdGetInt ("number");
  seqout = ajAcdGetSeqout ("outseq");

  count = 0;
  while (ajSeqallNext(seqall, &seq)) {
    if (++count == n) {
      ajSeqWrite (seqout, seq);
      ajSeqWriteClose (seqout);
      ajExit ();
      return 0;
    }
  }

  ajSeqWriteClose (seqout);
  (void) ajDie("No such sequence - only %d sequences were input.", count);
  return 1;

}

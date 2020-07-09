/* @source splitter application
**
** Split a sequence into (overlapping) smaller sequences
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

#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq;
  AjPSeq subseq = NULL;
  AjPStr str = NULL;
  AjPStr name = NULL;
  AjPStr value = NULL;
  int size, overlap;
  int from, to, start, end;
  int seqlen;
  
  (void) embInit ("splitter", argc, argv);

  seqout = ajAcdGetSeqoutall ("outseq");
  seqall = ajAcdGetSeqall ("sequence");
  size = ajAcdGetInt ("size");
  overlap = ajAcdGetInt ("overlap");

  while (ajSeqallNext(seqall, &seq)) {

    seqlen = ajSeqLen (seq)-1;
    if (seqlen > size) {

      subseq = ajSeqNew ();
      for (from = 0; from <= seqlen; from += size) {

        to = from + size-1;
	start = from - overlap;
        if (start < 0) {start = 0;}

	end = to + overlap;
        if (end > seqlen) {end = seqlen;}

        (void) ajStrAssSub(&str, ajSeqStr(seq), start, end);
        (void) ajSeqReplace(subseq, str);

/* create a nice name for the subsequence */
	(void) ajStrAss(&name, ajSeqGetName(seq));
	(void) ajStrAppC(&name, "_");
	(void) ajStrFromInt(&value, start+1);
	(void) ajStrApp(&name, value);
	(void) ajStrAppC(&name, "-");
	(void) ajStrFromInt(&value, end+1);	
	(void) ajStrApp(&name, value);
	(void) ajSeqAssName(subseq, name);

/* set the description of the subsequence */
	(void) ajSeqAssDesc(subseq, ajSeqGetDesc(seq));

/* set the type of the subsequence */
	(void) ajSeqType(subseq);

        (void) ajSeqAllWrite (seqout, subseq);
      }

      (void) ajSeqDel (&subseq);
    	
    } else {

      (void) ajSeqAllWrite (seqout, seq);
    	
    }
  }
  
  (void) ajSeqWriteClose (seqout);

  (void) ajExit ();
  return 0;
}

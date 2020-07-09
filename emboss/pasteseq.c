/* @source pasteseq application
**
** Insert one sequence into another
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

int main(int argc, char **argv)
{
  AjPSeq seq=NULL;
  AjPSeq insertseq=NULL;
  AjPSeqout seqout=NULL;
  ajint pos=0;	/* position to insert after - can be 0 to sequence.length */
  AjPStr str=NULL;

  AjPSeq newseq=NULL; /* the new sequence */
  

  (void) embInit ("pasteseq", argc, argv);

  seq = ajAcdGetSeq ("sequence");
  insertseq = ajAcdGetSeq ("insseq");
  pos = ajAcdGetInt ("pos");
  seqout = ajAcdGetSeqout ("outseq");


/* create the new sequence - set the size to the size of the two input
sequences plus 1 for the terminating NULL */
  newseq = ajSeqNewL(ajSeqLen(seq) + ajSeqLen(insertseq) + 1);

/* create a name for the sequence */
  (void) ajSeqAssName(newseq, ajSeqGetName(seq));
  
/* set the description of the sequence */
  (void) ajSeqAssDesc(newseq, ajSeqGetDesc(seq));
  
/* do the insertion here */
  str = ajSeqStr(newseq);
  (void) ajStrAssC(&str, ajSeqChar(seq));
  (void) ajStrInsert(&str, pos, ajSeqStr(insertseq));

/* set the type of the output sequence (is this required?) */
  (void) ajSeqType (newseq);

/* output the sequence */
  (void) ajSeqWrite (seqout, newseq);
  (void) ajSeqWriteClose (seqout);


  (void) ajExit ();
  return 0;
}




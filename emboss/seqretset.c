/*  Last edited: Nov 11 11:37 1998 (ajb) */

#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeqset seqset;
  AjPSeqout seqout;

  embInit ("seqretset", argc, argv);

  seqset = ajAcdGetSeqset ("sequence1");
  seqout = ajAcdGetSeqoutset ("outseq2");

  ajSeqsetWrite (seqout, seqset);

  ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}

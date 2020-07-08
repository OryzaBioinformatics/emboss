/*  Last edited: Nov 11 11:35 1998 (pmr) */


#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq = NULL;

  embInit ("seqretall", argc, argv);

  seqout = ajAcdGetSeqoutall ("outseq2");
  seqall = ajAcdGetSeqall ("sequence1");

  while (ajSeqallNext(seqall, &seq)) {
    ajSeqAllWrite (seqout, seq);
  }

  ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}

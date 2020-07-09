#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeqout seqout;
  AjPSeqall seqall;
  AjPSeq seq = NULL;

  embInit ("seqretallfeat", argc, argv);

  seqout = ajAcdGetSeqoutall ("outseq");
  seqall = ajAcdGetSeqall ("sequence");

  while (ajSeqallNext(seqall, &seq)) {
    ajSeqAllWrite (seqout, seq);
    ajSeqTrace (seq);
  }
  ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}

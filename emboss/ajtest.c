#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeq seq;
  AjPSeqall seqall;
  AjPSeqout seqout;

  embInit ("ajtest", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  seqout = ajAcdGetSeqout ("outseq");

  while (ajSeqallNext (seqall, &seq)) {
    ajSeqTrace (seq);
    ajUser ("<%S>", ajSeqGetUsa(seq));
    ajSeqAllWrite (seqout, seq);
  }

  ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}

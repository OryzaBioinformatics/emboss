#include "emboss.h"

int main(int argc, char **argv)
{

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq = NULL;
  AjBool firstonly;

  embInit ("seqretsplit", argc, argv);

  seqout = ajAcdGetSeqoutall ("outseq");
  seqall = ajAcdGetSeqall ("sequence");

  firstonly = ajAcdGetBool ("firstonly");
  while (ajSeqallNext(seqall, &seq)) {
    ajSeqAllWrite (seqout, seq);
    if (firstonly) break;
  }

  ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}

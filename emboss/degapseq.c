#include "emboss.h"

int main(int argc, char **argv)
{

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeq seq = NULL;
  AjBool firstonly;
  AjPStr str=NULL;

  embInit ("degapseq", argc, argv);

  seqout = ajAcdGetSeqoutall ("outseq");
  seqall = ajAcdGetSeqall ("sequence");

  while (ajSeqallNext(seqall, &seq)) {

/* get a COPY of the sequence string */
    str = ajStrNew();
    ajStrAss (&str, ajSeqStr(seq));

    ajStrDegap(&str);
    ajSeqReplace(seq, str);
    ajStrDel(&str);

    ajSeqAllWrite (seqout, seq);
  }

  ajSeqWriteClose (seqout);

  ajExit ();
  return 0;
}


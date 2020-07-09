#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeq seq;
  AjPSeqall seqall;
  AjPStr seqstr ;
  AjPFile outf;
  int len ;
  float pgc ;

  embInit ("geecee", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  outf = ajAcdGetOutfile ("outfile");

  ajFmtPrintF(outf, "#Sequence   GC content\n") ;
  while (ajSeqallNext(seqall, &seq)) {
    seqstr = ajSeqStr(seq) ;
    len    = ajSeqLen(seq) ;
    pgc    = ajMeltGC(&seqstr,len) ; /* forward strand for now... */

    ajFmtPrintF(outf, "%-12s %5.2f\n", ajSeqName(seq), pgc) ;
  }

  ajExit ();
  return 0 ;
}

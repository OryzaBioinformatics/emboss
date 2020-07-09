#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPFile outf;
  AjPSeq seq = NULL;
  AjPStr seqstr, seqdesc ;
  int len ;
  float pgc ;

  embInit ("seqinfo", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  outf = ajAcdGetOutfile ("outfile");

  while (ajSeqallNext(seqall, &seq)) {

    seqdesc = ajSeqGetDesc(seq) ;
    len     = ajSeqLen(seq) ;

    ajFmtPrintF(outf, "Sequence \'%s\'\n",     ajSeqName(seq)) ;
    if(!ajStrIsSpace(seqdesc))
      ajFmtPrintF(outf, "Description:\t%S\n", seqdesc ) ;
    if(ajSeqIsNuc(seq)) {
      ajFmtPrintF(outf, "Type:\t\tDNA\n") ;
      ajFmtPrintF(outf, "Length:\t\t%d basepairs\n", len) ;
      seqstr = ajSeqStr(seq) ;
      pgc = ajMeltGC(&seqstr,len) ;
      ajFmtPrintF(outf, "GC Content:\t%-8.4f%%\n", pgc*100.0) ;
    }
    else if(ajSeqIsProt(seq)) {
      ajFmtPrintF(outf, "Type:\t\tProtein\n") ;
      ajFmtPrintF(outf, "Length:\t\t%d residues\n", len) ;
    }
    else {
      ajFmtPrintF(outf, "Type:\t\tUnknown\n") ;
      ajFmtPrintF(outf, "Length:\t\t%d residues\n", len) ;
    }
  }

  ajExit ();
  return 0 ;
}

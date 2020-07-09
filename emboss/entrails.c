#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPFile outf;
  AjBool full;

  embInit ("entrails", argc, argv);

  outf = ajAcdGetOutfile ("outfile");
  full = ajAcdGetBool ("fullreport");

  ajAcdPrintType (outf, full);
  ajSeqPrintInFormat (outf, full);
  ajSeqPrintOutFormat (outf, full);

  ajGraphPrintType (outf, full);
  
  ajExit ();
  return 0;
}

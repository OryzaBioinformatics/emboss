#include "emboss.h"

int main(int argc, char **argv)
{

  AjPSeqall seqall;
  AjPFile outf;
  AjPRegexp patexp;
  AjPSeq seq = NULL;
  AjPStr str = NULL;
  AjPStr tmpstr = NULL;
  AjPStr substr = NULL;
  AjBool found;
  ajint ioff;
  ajint ipos;
  ajint ilen;

  embInit ("preg", argc, argv);

  outf = ajAcdGetOutfile ("outfile");
  seqall = ajAcdGetSeqall ("sequence");
  patexp = ajAcdGetRegexp ("pattern");

  ajFmtPrintF (outf, "preg search of %S with pattern %S\n", 
	       ajAcdValue("sequence"), ajAcdValue("pattern"));

  while (ajSeqallNext(seqall, &seq)) {
    found = ajFalse;
    ipos = 1;
    ajStrAssS (&str, ajSeqStr(seq));
    ajStrToUpper(&str);
    ajDebug ("Testing '%s' len: %d %d\n",
	     ajSeqName(seq), ajSeqLen(seq), ajStrLen(str));
    while (ajRegExec (patexp, str)) {
      if (!found) {
	ajFmtPrintF (outf, "Matches in %s\n", ajSeqName(seq));
	found = ajTrue;
      }
      ioff = ajRegOffset (patexp);
      ilen = ajRegLenI (patexp, 0);
      ajRegSubI (patexp, 0, &substr);
      ajRegPost (patexp, &tmpstr);
      ajStrAssS (&str, tmpstr);
      ipos += ioff;
      ajFmtPrintF (outf, "%15s %5d %S\n", ajSeqName(seq), ipos, substr);
      ipos += ilen;
    }
  }

  ajFileClose (&outf);

  ajExit ();
  return 0;
}

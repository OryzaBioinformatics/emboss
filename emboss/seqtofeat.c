#include "emboss.h"

int main (int argc, char * argv[]) {

  AjPSeq seq;
  AjPFeatTabOut outft;
  AjPFeatTable ftab = NULL;
  AjPStr pattern;
  AjPRegexp exp = NULL;
  AjPStr cpyseq = NULL;
  AjPStr match = NULL;
  AjPStr src = NULL;
  AjPStr ftname = NULL;

  int ioffset = 1;
  int icnt = 0;
  int istart, iend;

  embInit ("seqtofeat", argc, argv);

  seq = ajAcdGetSeq ("sequence");
  pattern = ajAcdGetString ("pattern");
  outft = ajAcdGetFeatout ("outfeat");

  exp = ajRegComp(pattern);

  cpyseq = ajSeqStrCopy (seq);
  ajUser ("using pattern '%S' cpyseq len %d", pattern, ajStrLen(cpyseq));

  ftab = ajFeatTabNewOut (ajSeqGetName(seq));

  src = ajStrNewC ("seqtofeat");
  ftname = ajStrNewC ("pattern");

  while (ajStrLen(cpyseq) && ajRegExec (exp, cpyseq)) {
    istart = ioffset + ajRegOffset(exp);
    iend = istart + ajRegLenI(exp, 0) - 1;
    ajRegSubI (exp, 0, &match);
    ajUser ("offset %d match %d to %d '%S'",
	    ioffset, istart, iend, match);
    ajRegPost (exp, &cpyseq);
    ajFeatureNew (ftab, src, ftname, istart, iend, NULL, '+', 0,
		  NULL, istart, iend);
    ioffset = iend + 1;
    icnt++;
  }

  ajFeaturesWrite(outft, ftab);

  ajExit ();
  return 0;
}

#include "emboss.h"

int main(int argc, char **argv)
{

  AjPSeqall seqall;
  AjPSeqout seqout;
  AjPSeqout junkout;
  AjPSeq seq = NULL;
  AjPStr pattern = NULL;
  AjPStr name=NULL;
  AjPStr acc=NULL;
  AjBool gotone=ajFalse;

  embInit ("notseq", argc, argv);

  seqout = ajAcdGetSeqoutall ("outseq");
  junkout = ajAcdGetSeqoutall ("junkout");
  seqall = ajAcdGetSeqall ("sequence");
  pattern = ajAcdGetString ("exclude");

  while (ajSeqallNext(seqall, &seq)) {
    (void) ajStrAss(&name, ajSeqGetName(seq));
    (void) ajStrAss(&acc, ajSeqGetAcc(seq));
      
    if (embMiscMatchPattern(name, pattern) ||
	embMiscMatchPattern(acc, pattern)) {
      ajSeqAllWrite (junkout, seq);    	
      gotone = ajTrue;
    } else {
/* no match, so not excluded */
      ajSeqAllWrite (seqout, seq);
    }
    ajStrClear(&name);
    ajStrClear(&acc);
  }

  ajSeqWriteClose (seqout);
  ajSeqWriteClose (junkout);

  if (gotone) {
    ajExit ();
  } else {
    ajWarn("No matches found.");
    ajExitBad ();
  }

  return 0;
}

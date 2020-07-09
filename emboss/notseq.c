#include "emboss.h"

static AjBool MatchPattern (AjPStr str, AjPStr pattern);

int main (int argc, char * argv[]) {

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
      
    if (MatchPattern(name, pattern) || MatchPattern(acc, pattern)) {
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


/** @funcstatic MatchPattern ***********************************************
**
** Does a simple OR'd test of matches to (possibly wildcarded) words.
** The words are tested one at a time until a match is found.
** Whitespace and , ; | characters can separate the words in the pattern.
**
**
** @param [r] str [AjPStr] string to test
** @param [r] pattern [AjPStr] pattern to match with
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

static AjBool MatchPattern (AjPStr str, AjPStr pattern) {

  char whiteSpace[] = " \t\n\r,;|";      /* skip whitespace and , ; | */
  AjPStrTok tokens;
  AjPStr key=NULL;
  AjBool val = ajFalse;         /* returned value */
    
  tokens = ajStrTokenInit(pattern, whiteSpace);
  while (ajStrToken( &key, &tokens, NULL)) {
    if (ajStrMatchWild(str, key)) {
      val = ajTrue;
      break;
    }
  }
  (void) ajStrTokenClear( &tokens);
  (void) ajStrDel(&key);
    
  return val;

}


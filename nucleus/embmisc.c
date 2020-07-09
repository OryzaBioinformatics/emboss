/*
**
** EMBOSS miscellaneous routines used by mnore than one application
**
*/

#include "emboss.h"

/** @func embMiscMatchPattern ***********************************************
**
** Does a simple OR'd test of matches to (possibly wildcarded) words.
** The words are tested one at a time until a match is found.
** Whitespace and , ; | characters can separate the words in the pattern.
**
** @param [r] str [AjPStr] string to test
** @param [r] pattern [AjPStr] pattern to match with
**
** @return [AjBool] ajTrue = found a match
** @@
******************************************************************************/

AjBool embMiscMatchPattern (AjPStr str, AjPStr pattern) {
    
  char whiteSpace[] = " \t\n\r,;|";      /* skip whitespace and , ; | */
  AjPStrTok tokens;
  AjPStr key=NULL;
  AjBool val = ajFalse;		/* returned value */
      
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

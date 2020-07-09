/******************************************************************************
** @source AJAX POSREG (ajax POSIX regular expression) functions
**
** Uses Henry Spencer's newer POSIX regular expression library.
** The newer version appears to have some performance problems,
** especially in extracting substrings which we may need to use
** quite extensively. Most regular expressions should use the
** older version in ajreg.c but all regular expressions should
** also work (perhaps more slowly) with this version.
**
** Some extra tricks are added, such as simply returning the matched
** string or the nth substring.
**
** Possible extensions include a case-insensitive regcomp ... which
** would be useful in many places. Also a regcomp for prefices,
** suffices and exact matches which adds "^" and "$" to the pattern
** string.
**
** @author Copyright (C) 1999 Peter Rice
** @version 1.0 
** @modified Feb 26 pmr First version
** @@
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"
#include "ajposreg.h"
#include "hsp_regex.h"

static AjPPosRegexp posregCompFlagsC (const char* exp, ajint cflags);

/* constructors */

/* @func ajPosRegComp ********************************************************
**
** Compiles a regular expression.
**
** @param [r] exp [AjPStr] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegComp (AjPStr exp) {
  return posregCompFlagsC (ajStrStr(exp), 0);
}

/* @func ajPosRegCompC ********************************************************
**
** Compiles a regular expression.
**
** @param [r] exp [const char*] Regular expression character string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompC (const char* exp) {
  return posregCompFlagsC (exp, 0);
}

/* @funcstatic posregCompFlagsC ***********************************************
**
** Compiles a regular expression.
**
** @param [r] exp [const char*] Regular expression character string.
** @param [r] cflags [ajint] Regular expression compilation bit flag(s).
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

static AjPPosRegexp posregCompFlagsC (const char* exp, ajint cflags) {
  AjPPosRegexp ret = NULL;
  ajint nsub;
  ajint rval;

  AJNEW0(ret);
  AJNEW0(ret->Regex);

  /* ajDebug ("posregCompFlagsC '%s' %x\n", exp, cflags); */
  rval = hsp_regcomp (ret->Regex, exp, cflags|REG_EXTENDED);

  if (cflags & REG_NOSUB)
    nsub = 1;
  else
    nsub = ret->Regex->re_nsub + 1;

  /* ajDebug ("    rval: %d nsub: %d\n", rval, ret->Regex->re_nsub); */
  switch (rval) {
  case 0:
    if (nsub) AJCNEW0(ret->Match, nsub);
    break;
  default:
    ajPosRegErr (ret, rval);
    return NULL;
  }

  return ret;
}

/* @func ajPosRegCompCase ****************************************************
**
** Compiles a regular expression ignoring upper/lower case.
**
** @param [r] exp [AjPStr] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompCase (AjPStr exp) {
  return posregCompFlagsC (ajStrStr(exp), REG_ICASE);
}

/* @func ajPosRegCompCaseC ****************************************************
**
** Compiles a regular expression ignoring upper/lower case.
**
** @param [r] exp [const char*] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompCaseC (const char* exp) {
  return posregCompFlagsC (exp, REG_ICASE);
}

/* @func ajPosRegCompNosub ****************************************************
**
** Compiles a regular expression with no substrings saved.
**
** @param [r] exp [AjPStr] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompNosub (AjPStr exp) {
  return posregCompFlagsC (ajStrStr(exp), REG_NOSUB);
}

/* @func ajPosRegCompNosubC ***************************************************
**
** Compiles a regular expression with no substrings saved.
**
** @param [r] exp [const char*] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompNosubC (const char* exp) {
  return posregCompFlagsC (exp, REG_NOSUB);
}

/* @func ajPosRegCompNewline **************************************************
**
** Compiles a regular expression with newlines recognized.
**
** @param [r] exp [AjPStr] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompNewline (AjPStr exp) {
  return posregCompFlagsC (ajStrStr(exp), REG_NEWLINE);
}

/* @func ajPosRegCompNewlineC *************************************************
**
** Compiles a regular expression with newlines recognized.
**
** @param [r] exp [const char*] Regular expression string.
** @return [AjPPosRegexp] Compiled POSIX regular expression.
** @@
******************************************************************************/

AjPPosRegexp ajPosRegCompNewlineC (const char* exp) {
  return posregCompFlagsC (exp, REG_NEWLINE);
}

/* execute expression match */

/* @func ajPosRegExec ********************************************************
**
** Execute a regular expression search.
** The expression must first have been compiled with ajPosRegComp or
** ajPosRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] str [AjPStr] String to be compared.
** @return [AjBool] ajTrue if a match was found.
** @@
******************************************************************************/

AjBool ajPosRegExec (AjPPosRegexp prog, AjPStr str) {

  return ajPosRegExecC (prog, ajStrStr(str));
}

/* @func ajPosRegExecC ********************************************************
**
** Execute a regular expression search.
** The expression must first have been compiled with ajPosRegComp or
** ajPosRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] str [const char*] String to be compared.
** @return [AjBool] ajTrue if a match was found.
** @@
******************************************************************************/

AjBool ajPosRegExecC (AjPPosRegexp prog, const char* str) {

  AjPPosRegmatch match = prog->Match;
  ajint nsub;
  ajint ret;

  /* ajDebug ("ajPosRegExecC '%s'\n", str); */

  nsub = prog->Regex->re_nsub+1;
  prog->Regex->orig = str;

  ret = hsp_regexec (prog->Regex, str, nsub, match, 0);

  /* ajDebug ("   result %d\n", ret); */

  switch (ret) {
  case REG_OKAY:
     return ajTrue;
  case REG_NOMATCH:
    return ajFalse;
  default:
    ajPosRegErr (prog, ret);
  }
  return ajFalse;
}

/* @func ajPosRegOffset ******************************************************
**
** After a successful regular expression match, uses the regular
** expression and the original string to calculate the offset
** of the match from the start of the string.
**
** This information is normally lost during processing.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @return [ajint] Offset of match from start of string. 
**               -1 if the string and the expression do not match.
** @@
******************************************************************************/

ajint ajPosRegOffset (AjPPosRegexp rp) {

  AjPPosRegmatch rm = rp->Match;

  return (rm[0].rm_so);
}

/* @func ajPosRegOffsetI *****************************************************
**
** After a successful regular expression match, returns the offset
** of a substring from the start of the string.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Offset of match from start of string. 
**               -1 if the string and the expression do not match.
** @@
******************************************************************************/

ajint ajPosRegOffsetI (AjPPosRegexp rp, ajint isub) {

  AjPPosRegmatch rm = rp->Match;

  return (rm[isub].rm_so);
}

/* @func ajPosRegOffsetC *****************************************************
**
** After a successful regular expression match, returns the offset
** of the match from the start of the string.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @return [ajint] Offset of match from start of string. 
**               -1 if the string and the expression do not match.
** @@
******************************************************************************/

ajint ajPosRegOffsetC (AjPPosRegexp rp) {

  AjPPosRegmatch rm = rp->Match;
  return (rm[0].rm_so);
}

/* @func ajPosRegOffsetIC *****************************************************
**
** After a successful regular expression match, returns the offset
** of a substring from the start of the string.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Offset of match from start of string. 
**               -1 if the string and the expression do not match.
** @@
******************************************************************************/

ajint ajPosRegOffsetIC (AjPPosRegexp rp, ajint isub) {

  AjPPosRegmatch rm = rp->Match;
  return (rm[isub].rm_so);
}

/* @func ajPosRegLenI ********************************************************
**
** After a successful comparison, returns the length of a substring.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Substring length, or 0 if not found.
** @@
******************************************************************************/

ajint ajPosRegLenI (AjPPosRegexp rp, ajint isub) {

  AjPPosRegmatch rm = rp->Match;
  if (rm[isub].rm_so < 0)
    return 0;

  return (rm[isub].rm_eo - rm[isub].rm_so);
}

/* @func ajPosRegPost ********************************************************
**
** After a successful match, returns the remainder of the string.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [w] post [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajPosRegPost (AjPPosRegexp rp, AjPStr* post) {

  AjPPosRegmatch rm = rp->Match;
  const char* orig = rp->Regex->orig;

  if (rm[0].rm_eo >= 0) {
    (void) ajStrAssC (post, &orig[rm[0].rm_eo]);
    return ajTrue;
  }

  ajStrDel(post);
  return ajFalse;
}

/* @func ajPosRegPostC ********************************************************
**
** After a successful match, returns the remainder of the string.
** Result is a character string, which is set to point to the internal
** string data. This in turn is part of the original string. If this
** changes then the results are undefined.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [w] post [const char**] Character string to hold the result.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajPosRegPostC (AjPPosRegexp rp, const char** post) {

  AjPPosRegmatch rm = rp->Match;
  const char* orig = rp->Regex->orig;

  if (rm[0].rm_eo >= 0) {
    *post = &orig[rm[0].rm_eo];
    return ajTrue;
  }

  *post = 0;
  return ajFalse;
}

/* @func ajPosRegSubI ********************************************************
**
** After a successful match, returns a substring.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] isub [ajint] Substring number.
** @param [w] dest [AjPStr*] String to hold the result.
** @return [void]
** @@
******************************************************************************/

void ajPosRegSubI (AjPPosRegexp rp, ajint isub, AjPStr* dest) {

  AjPPosRegmatch rm = rp->Match;
  ajint ilen;
  const char* orig;

  orig = rp->Regex->orig;

  if (rm[isub].rm_so < 0) {
    ajStrDel (dest);
    return;
  }
  if (isub > rp->Regex->re_nsub) {
    ajStrDel (dest);
    return;
  }
  ilen = rm[isub].rm_eo - rm[isub].rm_so;
  (void) ajStrAssCI (dest, &orig[rm[isub].rm_so], ilen);

  return;
}

/* substitute substrings */

/* @func ajPosRegSub ********************************************************
**
** Processes a source string including possible substring references
** escaped with backslashes. Results are written to another string.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] source [AjPStr] Source string.
** @param [w] dest [AjPStr*] Destination string.
** @return [void]
** @@
******************************************************************************/

void ajPosRegSub (AjPPosRegexp rp, AjPStr source, AjPStr* dest) {
  ajPosRegSubC (rp, ajStrStr(source), dest);
  return;
}

/* @func ajPosRegSubC ********************************************************
**
** Processes a source string including possible substring references
** escaped with backslashes. Results are written to another string.
**
** Based on ajPosRegSub with string replacing char* for output.
**
** @param [r] rp [AjPPosRegexp] Compiled POSIX regular expression.
** @param [r] source [const char*] Source string.
** @param [w] dest [AjPStr*] Destination string.
** @return [void]
** @@
******************************************************************************/

void ajPosRegSubC (AjPPosRegexp rp, const char* source, AjPStr* dest) {

  return;
}

/* destructor */

/* @func ajPosRegFree ********************************************************
**
** Clears and frees a compiled POSIX regular expression.
**
** @param [w] exp [AjPPosRegexp*] Compiled POSIX regular expression.
** @return [void]
** @@
******************************************************************************/

void ajPosRegFree (AjPPosRegexp* exp) {
  hsp_regfree ((*exp)->Regex);
  AJFREE ((*exp)->Match);	/* safe even if it is NULL still */
  AJFREE ((*exp)->Regex);
  
  AJFREE(*exp);
  
  *exp = NULL;
  return;
}

/* @func ajPosRegTrace ********************************************************
**
** Traces a compiled POSIX regular expression with debug calls.
**
** @param [r] exp [AjPPosRegexp] Compiled POSIX regular expression.
** @return [void]
** @@
******************************************************************************/

void ajPosRegTrace (AjPPosRegexp exp) {

  AjPPosRegmatch rm = exp->Match;
  ajint isub;
  ajint ilen;
  ajint ipos;
  const char* orig = exp->Regex->orig;
  static AjPStr str = NULL;

  /* ajDebug ("  REGEXP trace\n"); */
  for (isub=0; isub <= exp->Regex->re_nsub; isub++) {
    if (rm[isub].rm_so >= 0) {
      ilen = rm[isub].rm_eo - rm[isub].rm_so;
      (void) ajStrAssCI (&str, &orig[rm[isub].rm_so], ilen);
      if (!isub) {
	/* ajDebug ("    string match '%S'\n", str); */
      }
      else {
	ipos = rm[isub].rm_so - rm[0].rm_so;
	/* ajDebug ("    substring %2d '%S' at %d\n", isub, str, ipos); */
      }
    }
  }
  /* ajDebug ("\n"); */
  ajStrDel (&str);
  return;
}

/* @func ajPosRegErr *******************************************************
**
** Reports an error message from a POSIX 1003.2 regular expression operation
**
** @param [r] prog [AjPPosRegexp] POSIX regular expression
** @param [r] errcode [ajint] Internal error code.
** @return [void]
** @@
******************************************************************************/

void ajPosRegErr (AjPPosRegexp prog, ajint errcode) {

  static char msg[128];

  (void) hsp_regerror (errcode, prog->Regex, msg, 128);
  ajErr(msg);

  return;
}

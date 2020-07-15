/******************************************************************************
** @source AJAX string functions
**
** AjPStr objects are reference counted strings
** Any change will need a new string object if the use count
** is greater than 1, so the original ajStr provided so that it can
** be reallocated in any routine where string modification is possible.
**
** In many cases
** the text is always a copy, even of a constant original, so
** that it can be simply freed.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
** @modified Jun 25 pmr First version
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

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#ifndef HAVE_MEMMOVE
#include <sys/types.h>
/* @header memmove ************************************************************
**
******************************************************************************/
static void* memmove(void *dst, const void* src, size_t len)
{
    return (void *)bcopy(src, dst, len);
}
#endif


#include <math.h>
#include "ajax.h"
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <string.h>




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




#define STRSIZE  32
#define LONGSTR  512
#define NULL_USE 1
char charNULL[1] = "";

AjOStr strONULL = { 1, 0, NULL_USE, charNULL}; /* use set to avoid changes */
AjPStr strPNULL = &strONULL;


static char* strParseCp = NULL;


/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjPStr strNew(size_t size);
static void   strCloneL(AjPStr* pthis, size_t size);

static ajlong strAlloc     = 0;
static ajlong strFree      = 0;
static ajlong strFreeCount = 0;
static ajlong strCount     = 0;
static ajlong strTotal     = 0;




/* @filesection ajstr ********************************************************
**
** @nam1rule aj Function belongs to the AJAX library.
**
** @suffix Len [size_t] length
** @suffix C [char*] C character string
** @suffix S [AjPStr] string object
** @suffix K [char] single character
*/




/* @datasection [char*] C character string ***********************************
**
** Function is for manipulating C-type (char*) strings and
** takes at least one char* argument.
**
** @nam2rule Char
*/




/* @section constructors ******************************************************
**
** Functions for constructing C-type (char*) strings,
** possibly with a starting string. 
**
** @fdata [char*]
** @fnote     Same argrule as "String constructor functions"
**
** @nam3rule  New     Construct a new string.
** @nam4rule  NewRes  Minimum reserved size.
**
** @argrule   C       txt [const char*] C character string
** @argrule   S       str [const AjPStr] Text string
** @argrule   NewRes  size [size_t] Reserved size
** @argrule   Len     len [size_t] Length of string
**
** @valrule   *  [char*] New C-style string
** @fcategory new
**
******************************************************************************/


/* @func ajCharNewC ***********************************************************
**
** A text string constructor which allocates memory for a string
** and initialises it with the text string provided.
**
** @param [r] txt [const char*] Initial text, possibly shorter than the
**        space allocated.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewC(const char* txt)
{
    char* cp;
    ajint len;

    len = strlen(txt);
    cp = (char*) AJALLOC0(len+1);
    memmove(cp, txt, len+1);

    return cp;
}




/* @func ajCharNewS ***********************************************************
**
** A text string constructor which allocates memory for a string 
** and initialises it with the string provided.
**
** @param [r] str [const AjPStr] String object as initial value and size
**                          for the text.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewS(const AjPStr str)
{
    static char* cp;

    cp = (char*) AJALLOC0(str->Len+1);
    memmove(cp, str->Ptr, str->Len+1);

    return cp;
}



/* @obsolete ajCharNew
** @rename ajCharNewS
*/

char __deprecated *ajCharNew(const AjPStr str)
{
    return ajCharNewS(str);
}

/* @func ajCharNewRes *********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and intialises the contents as an empty string.
**
** @param [r] size [size_t] Length of the Cstring, excluding the trailing NULL.
** @return [char*] A new text string with no contents.
** @@
******************************************************************************/

char* ajCharNewRes(size_t size)
{
    static char* cp;

    cp = (char*) AJALLOC0(size+1);
    cp[0] = '\0';

    return cp;
}

/* @obsolete ajCharNewL
** @rename ajCharNewRes
*/

char __deprecated * ajCharNewL(size_t size) {
    return ajCharNewRes(size);
}

/* @func ajCharNewResC ********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the text string provided.
**
** @param [r] txt [const char*] String object as initial value and size
**                          for the text.
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewResC(const char* txt, size_t size)
{
    static char* cp;
    size_t isize;
    ajint ilen;

    isize = size;
    ilen = strlen(txt);

    if(ilen >= (ajint) isize)
	isize = ilen + 1;

    cp = (char*) AJALLOC0(isize);
    memmove(cp, txt, ilen+1);

    return cp;
}

/* @func ajCharNewResS ********************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the string provided.
**
** @param [r] str [const AjPStr] String object as initial value and size
**                          for the text.
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewResS(const AjPStr str, size_t size)
{
    static char* cp;
    size_t isize;

    isize = size;
    if(str->Len >= (ajint) isize)
	isize = str->Len + 1;

    cp = (char*) AJALLOC0(isize);
    memmove(cp, str->Ptr, str->Len+1);

    return cp;
}


/* @obsolete ajCharNewLS
** @replace ajStrNewResS (1,2/2,1)
*/

char __deprecated * ajCharNewLS(size_t size, const AjPStr thys) {
    return ajCharNewResS(thys, size);
}



/* @func ajCharNewResLenC *****************************************************
**
** A text string constructor which allocates memory for a string of the 
** specified length and initialises it with the text string provided.
**
** @param [r] txt [const char*] String object as initial value and size
**                          for the text.
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @param [r] len [size_t] Length of txt to save calculation time.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewResLenC(const char* txt, size_t size, size_t len)
{
    static char* cp;
    size_t isize;

    isize = size;

    if(len >= isize)
	isize = len + 1;

    cp = (char*) AJALLOC0(isize);
    memmove(cp, txt, len);

    return cp;
}

/* @section destructors
**
** Functions for destruction of C-type (char*) strings.
**
** @fdata      [char*]
** @fnote     None.
** @nam3rule  Del    Destroy (free) an existing string.
** @argrule   Del Ptxt [char**] Pointer to character string to be deleted
** @valrule   * [void]
** @fcategory delete
******************************************************************************/


/* @func ajCharDel ***********************************************************
**
** A text string destructor to free memory for a text string.
**
** @param [d] Ptxt [char**] Text string to be deallocated.
** @return [void]
** @ure The string is freed using free in the C RTL, so it
**      must have been allocated by malloc in the C RTL
** @@
******************************************************************************/

void ajCharDel(char** Ptxt)
{
    AJFREE(*Ptxt);

    return;
}

/* @obsolete ajCharFree
** @rename ajCharDel
*/

void __deprecated ajCharFree(char** txt)
{
    ajCharDel(txt);
}



/* @section formatting
**
** Functions for formatting C-type (char*) strings).
**
** @fdata      [char*]
** @fnote     Same namrule as "String formatting functions"
**
** @nam3rule  Fmt         Change the format of a string.
** @nam4rule  FmtLower    Convert to lower case.
** @nam4rule  FmtTitle    Convert to title case.
** @nam4rule  FmtUpper    Convert to upper case.
**
** @argrule   * txt [char*] Character string
** @valrule   * [AjBool]
** @fcategory modify
*/

/* @func ajCharFmtLower *******************************************************
**
** Converts a text string to lower case.
**
** @param [u] txt [char*] Text string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajCharFmtLower(char* txt)
{
    char* cp;

    cp = txt;

    while(*cp)
    {
	/*
	 *  AJB: function ajSysItoC was there as some really fussy compilers
	 *  complained about casting ajint to char. However, for conversion of
	 *  large databases it's too much of an overhead. Think about a macro
	 *  later. In the meantime revert to the standard system call
	 *    *cp = ajSysItoC(tolower((ajint) *cp));
	 */
	*cp = (char)tolower((ajint) *cp);
	cp++;
    }

    return ajTrue;
}



/* @obsolete ajCharToLower
** @rename ajCharFmtLower
*/

void __deprecated ajCharToLower(char* txt)
{
    ajCharFmtLower(txt);
}

/* @func ajCharFmtUpper *******************************************************
**
** Converts a text string to upper case.
**
** @param [u] txt [char*] Text string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajCharFmtUpper(char* txt)
{
    char* cp;

    cp = txt;

    while(*cp)
    {
	/*
	 *  AJB: function ajSysItoC was there as some really fussy compilers
	 *  complained about casting ajint to char. However, for conversion of
	 *  large databases it's too much of an overhead. Think about a macro
	 *  later. In the meantime revert to the standard system call
	 *    *cp = ajSysItoC(toupper((ajint) *cp));
	 */
	*cp = (char) toupper((ajint) *cp);
	cp++;
    }

    return ajTrue;
}



/* @obsolete ajCharToUpper
** @rename ajCharFmtUpper
*/

void __deprecated ajCharToUpper(char* txt)
{
    ajCharFmtUpper(txt);
}




/* @section comparison
**
** Functions for comparing C-type (char*) strings.
**
** @fdata      [char*]
** @fnote     Same namrule as "String comparison functions".
** @nam3rule  Match         Compare two complete strings.
** @nam3rule  Prefix        Compare start of string to given prefix.
** @nam3rule  Suffix        Compare end of string to given suffix.
** @nam4rule  MatchCase   Case-insensitive comparison.
** @nam4rule  MatchWild   Comparison using wildcard characters.
** @nam5rule  MatchWildNext   Comparison using wildcard characters to
**                            next word in a string
** @nam5rule  MatchWildWord   Comparison using wildcard characters to
**                          whole words within a string.
** @nam4rule  PrefixCase  Case-insensitive comparison.
** @nam4rule  SuffixCase  Case-insensitive comparison.
**
** @argrule   * txt [const char*] Character string
** @argrule   C txt2 [const char*] Character string to compare
** @argrule   S str [const AjPStr] Character string to compare
** @valrule   * [AjBool] True on success
** @fcategory use
*/


/* @func ajCharMatchC *********************************************************
**
** Simple test for matching two text strings.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if text completely matches the start of String
** @@
******************************************************************************/

AjBool ajCharMatchC(const char* txt, const char* txt2)
{
    if(!txt || !txt2)
	return ajFalse;

    if(!strcmp(txt, txt2))
	return ajTrue;

    return ajFalse;
}

/* @obsolete ajStrMatchCC
** @rename ajCharMatchC
*/

AjBool __deprecated ajStrMatchCC(const char* thys, const char* text)
{
    return ajCharMatchC(thys, text);
}



/* @func ajCharMatchCaseC *****************************************************
**
** Simple case-insensitive test for matching two text strings.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajCharMatchCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    cp = txt;
    cq = txt2;

    if(!*cp && !*cq)
	return ajTrue;

    if(!*cp || !*cq)
	return ajFalse;

    while(*cp && *cq)
	if(tolower((ajint) *cp++) != tolower((ajint) *cq++))
	    return ajFalse;

    if(*cp || *cq)
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrMatchCaseCC
** @rename ajCharMatchCaseC
*/

AjBool __deprecated ajStrMatchCaseCC(const char* thys, const char* text)
{
    return ajCharMatchCaseC(thys, text);
}

/* @func ajCharMatchWildC *****************************************************
**
** Simple case-insensitive test for matching two text strings using 
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajCharMatchWildC(const char* txt, const char* txt2)
{
    ajint i;

    i = ajCharCmpWild(txt, txt2);

/*
    ajDebug("ajCharMatchWildC' '%s' cmp: %d\n",
	    str, text, i);
*/

    if(i)
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrMatchWildCC
** @rename ajCharMatchWildC
*/

AjBool __deprecated ajStrMatchWildCC(const char* str, const char* text)
{
    return ajCharMatchWildC(str, text);
}

/* @func ajCharMatchWildS *****************************************************
**
** Simple case-insensitive test for matching a text string and a string using
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] str [const AjPStr] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajCharMatchWildS(const char* txt, const AjPStr str)
{
    return ajCharMatchWildC(txt, str->Ptr);
}

/* @obsolete ajStrMatchWildCO
** @rename ajCharMatchWildS
*/

AjBool __deprecated ajStrMatchWildCO(const char* str, const AjPStr wild)
{
    return ajCharMatchWildS(str, wild);
}


/* @func ajCharMatchWildNextC *************************************************
**
** Test for matching the next 'word' in two text strings using 
** wildcard characters.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajCharMatchWildNextC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    const char* savecp;
    char lastch = '\0';
    
    ajDebug("ajCharMatchWildNextC '%s' '%s'\n", txt, txt2);

    cp = txt2;
    cq = txt;
    
    if(!*cp && !*cq)
	return ajTrue; /* both empty */

    if(!*cp)
	return ajFalse;	/* no query text */
    
    while(*cp && !isspace((int) *cp))
    {
	if(!*cq && *cp != '*')
	    return ajFalse;

	switch(*cp)
	{
	case '?':		/* skip next character and continue */
	    lastch = *cq;
	    cp++;
	    cq++;
	    break;
	case '*':
	    cp++;		 /* recursive call to test the rest */
	    if(!*cp)
	    {
		ajDebug("...matches at end +%d '%s' +%d '%s'\n",
			 (cq - txt), cq, (cp - txt2), cp);
		return ajTrue;	 /* just match the rest */
	    }

	    if(!*cq)		 /* no more string to compare */
	    {
		savecp = cp;
		while(*cp == '*') {
		    savecp = cp++;	/* may be ***... savecp is last '*' */
		}
		if(!*cp) return ajTrue;
		return ajCharMatchWildNextC(cq,savecp);
	    }

	    while(*cq)
	    {		 /* wildcard in mid name, look for the rest */
		if(ajCharMatchWildNextC(cq, cp)) /* recursive + repeats */
		    return ajTrue;
		ajDebug("...'*' at +%d '%s' +%d '%s' continuing\n",
			 (cq - txt), cq, (cp - txt2), cp);
		cq++;
	    }

	    return ajFalse;	  /* if we're still here, it failed */

	    /* always returns once '*' is found */

	default:	 /* for all other characters, keep checking */
	    if(tolower((ajint) *cp) != tolower((ajint) *cq))
		return ajFalse;

	    cp++;
	    if(*cq)
	    {
		lastch = *cq;
		cq++;
	    }
	}
    }

    ajDebug("...done comparing at +%d '%s' +%d '%s' lastch '%c'\n",
	     (cq - txt), cq, (cp - txt2), cp, lastch);
    
    if(!isalnum((int) lastch))
    {
	ajDebug("not a word boundary at '%c'\n", lastch);
	return ajFalse;
    }
    
    if(*cp)
    {
	ajDebug("...incomplete cp, FAILED\n");
	return ajFalse ;
    }
    
    if(*cq)
    {
	if(isalnum((int) *cq))
	{
	    ajDebug("word continues, failed\n");
	    return ajFalse;
	}
	ajDebug("word end ... success\n");
	return ajTrue;
    }
    
    ajDebug("...all finished and matched\n");
    
    return ajTrue;
}



/* @func ajCharMatchWildWordC *************************************************
**
** Case-insensitive test for matching a text string 'word' against a 
** text string using wildcard characters.
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Wildcard word
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajCharMatchWildWordC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    AjBool word = ajFalse;

    ajDebug("ajCharMatchWildWordC('%s', '%s')\n", txt, txt2);

    cp = txt2;
    cq = txt;

    if(!*cp && !*cq)
	return ajTrue;

    if(!*cp)
	return ajFalse;

    ajDebug("something to test, continue...\n");

    /* unlike ajCharMatchWild, we step through the string (str) not the
       query (text) */

    while(*cq)
    {
	if(!word)
	{
	    if(isalnum((int) *cq))
	    {				/* start of word */
		word = ajTrue;
		if(ajCharMatchWildNextC(cq, txt2))
		    return ajTrue;
	    }
	}
	else
	{
	    if(!isalnum((int) *cq))
		word = ajFalse;
	}

	cq++;
    }

    return ajFalse;
}


/* @obsolete ajStrMatchWordCC
** @rename ajCharMatchWildWordS
*/

AjBool __deprecated ajStrMatchWordCC (const char* str, const char* txt)
{
    return ajCharMatchWildWordC(str, txt);
}

/* @func ajCharPrefixC ********************************************************
**
** Test for matching the start of a text string against a given prefix text 
** string.
**
** @param [r] txt [const char*]  Test string as text
** @param [r] txt2 [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixC(const char* txt, const char* txt2)
{
    ajint ilen;

    ilen = strlen(txt2);

    if(!ilen)				/* no prefix */
	return ajFalse;

    if(ilen > (ajint) strlen(txt))		/* pref longer */
	return ajFalse;

    if(strncmp(txt, txt2, ilen))    /* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrPrefixCC
** @rename ajCharPrefixC
*/

AjBool __deprecated ajStrPrefixCC(const char* str, const char* pref)
{
    return ajCharPrefixC(str, pref);
}



/* @func ajCharPrefixS ********************************************************
**
** Test for matching the start of a text string against a given prefix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Prefix as string
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixS(const char* txt, const AjPStr str)
{
    if(!txt)
	return ajFalse;

    if(str->Len > (ajint) strlen(txt))	/* pref longer */
	return ajFalse;

    if(strncmp(txt, str->Ptr, str->Len)) /* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}


/* @obsolete ajStrPrefixCO
** @rename ajCharPrefixS
*/

AjBool __deprecated ajStrPrefixCO(const char* str, const AjPStr thys)
{
    return ajCharPrefixS(str, thys);
}

/* @func ajCharPrefixCaseC ****************************************************
**
** Case-insensitive test for matching the start of a text string against a 
** given prefix text string.
**
** @param [r] txt [const char*] Text
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixCaseC(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    cp = txt;
    cq = txt2;

    /* ajDebug("ajCharPrefixCaseC '%s' '%s'\n", txt, txt2); */

    if(!*cq)
	return ajFalse;

    while(*cp && *cq)
    {
	if(tolower((ajint) *cp) != tolower((ajint) *cq)) return ajFalse;
	cp++;cq++;
    }

    if(*cq)
	return ajFalse;

    /* ajDebug("ajStrPrefixCaseCC ..TRUE..\n"); */
    return ajTrue;
}

/* @obsolete ajStrPrefixCaseCC
** @rename ajCharPrefixCaseC
*/

AjBool __deprecated ajStrPrefixCaseCC(const char* thys, const char* pref)
{
    return ajCharPrefixCaseC(thys, pref);
}

/* @func ajCharPrefixCaseS ****************************************************
**
** Case-insensitive test for matching start of a text string against a given 
** prefix string,
**
** @param [r] txt [const char*] Text
** @param [r] str [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajCharPrefixCaseS(const char* txt, const AjPStr str)
{
    return ajCharPrefixCaseC(txt, str->Ptr);
}


/* @obsolete ajStrPrefixCaseCO
** @rename ajCharPrefixCaseS
*/

AjBool __deprecated ajStrPrefixCaseCO(const char* thys, const AjPStr pref)
{
    return ajCharPrefixCaseS(thys, pref);
}



/* @func ajCharSuffixC ********************************************************
**
** Test for matching the end of a text string against a given suffix text 
** string.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixC(const char* txt, const char* txt2)
{
    ajint ilen;
    ajint jlen;
    ajint jstart;

    ilen   = strlen(txt2);
    jlen   = strlen(txt);
    jstart = jlen - ilen;

    if(ilen > jlen)			/* suff longer */
	return ajFalse;

    if(strncmp(&txt[jstart], txt2, ilen)) /* +1 or -1 for a
					      failed match */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrSuffixCC
** @rename ajCharSuffixC
*/

AjBool __deprecated ajStrSuffixCC(const char* str, const char* suff)
{
    return ajCharSuffixC(str, suff);
}

/* @func ajCharSuffixS ********************************************************
**
** Test for matching the end of a text string against a given suffix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Suffix as string
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixS(const char* txt, const AjPStr str)
{
    ajint jlen;
    ajint jstart;

    jlen   = strlen(txt);
    jstart = jlen - str->Len;

    if(str->Len > jlen)		/* suff longer */
	return ajFalse;

    if(strncmp(&txt[jstart], str->Ptr, str->Len)) /* +1 or -1 for a
							failed match */
	return ajFalse;

    return ajTrue;
}



/* @obsolete ajStrSuffixCO
** @rename ajCharSuffixS
*/

AjBool __deprecated ajStrSuffixCO(const char* txt, const AjPStr suff)
{
    return ajCharSuffixS(txt, suff);
}

/* @func ajCharSuffixCaseC ****************************************************
**
** Case-sensitive test for matching the end of a text string against a given 
** suffix text string.
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixCaseC(const char* txt, const char* txt2)
{
    ajint ilen;
    ajint jlen;
    ajint jstart;
    const char* cp;
    const char* cq;

    ilen   = strlen(txt2);
    jlen   = strlen(txt);
    jstart = jlen - ilen;

    if(ilen > jlen)			/* suff longer */
	return ajFalse;

    cp = &txt[jstart];
    cq = txt2;
    while (cp)
    {
	if(tolower((ajint)*cp) != tolower((ajint)*cq)) return ajFalse;
	cp++; cq++;
    }

    return ajTrue;
}

/* @func ajCharSuffixCaseS ****************************************************
**
** Case-sensitive test for matching the end of a text string against a given 
** suffix string.
**
** @param [r] txt [const char*] Test string as text
** @param [r] str [const AjPStr] Suffix as string
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajCharSuffixCaseS(const char* txt, const AjPStr str)
{
    ajint jlen;
    ajint jstart;
    const char* cp;
    const char* cq;
    jlen   = strlen(txt);
    jstart = jlen - str->Len;

    if(str->Len > jlen)		/* suff longer */
	return ajFalse;

    cp = &txt[jstart];
    cq = str->Ptr;
    while (cp)
    {
	if(tolower((ajint)*cp) != tolower((ajint)*cq)) return ajFalse;
	cp++; cq++;
    }

    return ajTrue;
}



/* @section comparison (sorting)
**
** Functions for sorting strings (including C-type char* strings).
**
** @fdata      [char*]
** @fnote     namrule same as "String comparison (sorting) functions".
**
** @nam3rule  Cmp           Compare two complete strings & return sort order.
** @nam4rule  CmpCase     Case-insensitive comparison.
** @nam4rule  CmpWild     Comparison using wildcard characters.
**
** @argrule   * txt [const char*] Character string
** @argrule   * txt2 [const char*] Comparison string
** @argrule   Len len [size_t] Number of characters to compare
** @valrule   * [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @fcategory use
*/


/* @func ajCharCmpCase ********************************************************
**
** Finds the sort order of two text strings. 
**
** @param [r] txt [const char*] Text string
** @param [r] txt2 [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpCase(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;

    for(cp = txt, cq = txt2; *cp && *cq; cp++, cq++)
    {
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}
    }

    if(*cp)
	return 1;

    if(*cq)
	return -1;

    return 0;
}


/* @obsolete ajStrCmpCaseCC
** @rename ajCharCmpCase
*/

int __deprecated ajStrCmpCaseCC(const char* str1, const char* str2)
{
    return ajCharCmpCase (str1, str2);
}

/* @func ajCharCmpCaseLen *****************************************************
**
** Finds the sort order of substrings from the start of two text strings.
**
** @param [r] txt [const char*] Text string
** @param [r] txt2 [const char*] Text string
** @param [r] len  [size_t] length
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpCaseLen(const char* txt, const char* txt2, size_t len)
{
    const char* cp;
    const char* cq;
    ajint i;

    for(cp=txt,cq=txt2,i=0;*cp && *cq && i<(ajint)len;++i,++cp,++cq)
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}

    if(i==len)
	return 0;

    if(*cp)
	return 1;

    return -1;
}


/* @obsolete ajStrNCmpCaseCC
** @rename ajCharCmpCaseLen
*/

int __deprecated ajStrNCmpCaseCC(const char* str1, const char* str2, ajint len)
{
    return ajCharCmpCaseLen(str1, str2, len);
}

/* @func ajCharCmpWild ********************************************************
**
** Finds the sort-order (case insensitive) of two text strings using
** wildcard characters. 
**
** @param [r] txt [const char*] String
** @param [r] txt2 [const char*] Text
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajCharCmpWild(const char* txt, const char* txt2)
{
    const char* cp;
    const char* cq;
    
    /*ajDebug("ajCharCmpWild('%s', '%s')\n", str, text);*/

    cp = txt2;
    cq = txt;
    
    if(!*cp && !*cq)
	return 0;

    if(!*cp)
	return -1;
    
    /*ajDebug("something to test, continue...\n");*/
    
    while(*cp)
    {
	if(!*cq && *cp != '*')
	    return 1;

	switch(*cp)
	{
	case '?':		/* skip next character and continue */
	    cp++;
	    cq++;
	    break;
	case '*':
	    cp++;
	    if(!*cp)
	    {
		/* ajDebug("...matches at end +%d '%s' +%d '%s'\n",
		   (cq - str), cq, (cp - text), cp);*/
		return 0;		/* just match the rest */
	    }

	    if(!*cq)
	    {
		/*ajDebug("...test match to null string just in case\n");*/
		return ajCharCmpWild(cq, cp);
	    }

	    while(*cq)
	    {		 /* wildcard in mid name, look for the rest */
		if(ajCharMatchWildC(cq, cp))
		    return 0; /* recursive + repeats */
		/* ajDebug("...'*' at +%d '%s' +%d '%s' continuing\n",
		   (cq - str), cq, (cp - text), cp);*/
		cq++;
	    }

	    return 1;

	    /* always returns once '*' is found */

	default:	 /* for all other characters, keep checking */
	    if(tolower((ajint) *cp) != tolower((ajint) *cq))
	    {
		if(tolower((ajint) *cp) > tolower((ajint) *cq))
		    return -1;
		else
		    return 1;
	    }
	    cp++;
	    if(*cq)
		cq++;
	}
    }

    /*ajDebug("...done comparing at +%d '%s' +%d '%s'\n",
      (cq - str), cq, (cp - text), cp);*/
    if(*cp)
    {
	/*ajDebug("...incomplete cp, FAILED\n");*/
	return -1 ;
    }

    if(*cq)
    {
	/*ajDebug("...incomplete cq, FAILED\n");*/
	return 1;
    }
    /*ajDebug("...all finished and matched\n");*/
    
    return 0;
}


/* @obsolete ajStrCmpWildCC
** @rename ajCharCmpWild
*/

ajint __deprecated ajStrCmpWildCC(const char* str, const char* text)
{
    return ajCharCmpWild (str, text);
}

/* @section parsing functions
**
** Simple token parsing of strings
**
** @fdata [char*]
**
** @nam3rule Parse Parsing
** @argrule * txt [const char*] Text to be parsed
** @argrule C txtdelim [const char*] Delimiter set
** @valrule * [AjPStr] String token result, now owned by calling function.
** @fcategory use
*/


/* @func ajCharParseC *********************************************************
**
** Simple token parsing from text string using a specified set of delimiters.
**
** @param [r] txt [const char*] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] txtdelim [const char*] Delimiter(s) to be used between tokens.
** @return [AjPStr] Token
** @error NULL if no further token is found.
** @@
******************************************************************************/

AjPStr ajCharParseC (const char* txt, const char* txtdelim)
{
    static AjPStr strp = 0; /* internal AjPStr - do not try to destroy */
    static char* cp    = NULL;

    if (!strp)
    {
	if (!txt)
	{
	    ajWarn("Error in ajCharParseC: NULL argument and not initialised");
	    return NULL;
	}
	strp = ajStrNew();
	AJFREE(strp->Ptr);
    }

    if (txt)
    {
	if (cp) ajCharDel(&cp);
	cp = ajCharNewC(txt);
	strp->Ptr = ajSysStrtok (cp, txtdelim);
    }
    else
    {
	strp->Ptr = ajSysStrtok (NULL, txtdelim);
    }

    if (strp->Ptr)
    {
	strp->Len = strlen(strp->Ptr);
	strp->Res = strp->Len + 1;
	return strp;
    }
    else
    {
	strp->Len=0;
    }

    return NULL;
}



/* @obsolete ajStrTokCC
** @rename ajCharParseC
*/

AjPStr __deprecated ajStrTokCC (const char* txt, const char* delim)
{
    return ajCharParseC(txt, delim);
}

/* @datasection [AjPStr] String ***********************************************
**
** Functions for manipulating AJAX (AjPStr) strings
**
** @nam2rule Str    Function is for manipulating strings
**
*/



/* @section constructors 
**
** Functions for constructing string objects, possibly with a starting string. 
**
** @fdata      [AjPStr]
** @fnote     Same namrule as "String constructor functions:
**            C-type (char*) strings"
** @nam3rule  New     Construct a new string.
** @nam4rule  NewRef  Construct making a reference counted copy of
**                    an existing string.
** @nam4rule  NewRes  Construct with reserved size.
** @argrule   C       txt [const char*] Text string
** @argrule   S       str [const AjPStr] Text string
** @argrule   NewRes  size [size_t] Reserved size
** @argrule   NewRef  refstr [AjPStr] Text string to be duplicated
** @argrule   Len     len [size_t] Length of string
** @valrule   *     [AjPStr] New string
** @fcategory new
*/


/* @func ajStrNew *************************************************************
**
** Default string constructor which allocates memory for a string.
** 
** The null string usage pointer is incremented.
**
** @return [AjPStr] Pointer to an empty string
** @@
******************************************************************************/

AjPStr ajStrNew(void)
{
    return ajStrNewRef(strPNULL);
}


/* @func ajStrNewC ************************************************************
**
** String constructor which allocates memory for a string and initialises it
** with the text string provided. 
** 
** The string size is set just large enough to hold the supplied text.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @return [AjPStr] Pointer to a string containing the supplied text
** @@
******************************************************************************/

AjPStr ajStrNewC(const char* txt)
{
    ajint i;
    ajint j;
    AjPStr thys;

    i = strlen(txt);
    j = ajRound(i + 1, STRSIZE);

    thys = ajStrNewResLenC(txt, j, i);

    return thys;
}




/* @func ajStrNewS ************************************************************
**
** String constructor which allocates memory for a string and initialises it
** with the string provided.
**
** @param [r] str [const AjPStr] String to be cloned
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewS(const AjPStr str)
{
    return ajStrNewResLenC(str->Ptr, str->Res, str->Len);
}




/* @func ajStrNewRef **********************************************************
**
** String referencing function which returns a pointer to the string passed in
** and increases its reference count.
**
** Not a true string copy, but if modified the new string will behave
** as a copy.
**
** @param [u] refstr [AjPStr] AJAX string object
** @return [AjPStr] Pointer to the string passed as an argument,
**         with its use count increased by 1.
** @@
******************************************************************************/

AjPStr ajStrNewRef(AjPStr refstr)
{
    if(!refstr)
	ajFatal("ajStrNewRef source string NULL");

    refstr->Use++;

    return refstr;
}


/* @obsolete ajStrDup
** @rename ajStrNewRef
*/

AjPStr __deprecated ajStrDup(AjPStr thys)
{
    return ajStrNewRef(thys);
}

/* @func ajStrNewRes **********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null).
**
** @param [r] size [size_t] Reserved size (including a possible null).
** @return [AjPStr] Pointer to an empty string of specified size.
** @@
******************************************************************************/

AjPStr ajStrNewRes(size_t size)
{
    AjPStr thys;

    thys = ajStrNewResLenC("", size, 0);

    return thys;
}

/* @obsolete ajStrNewL
** @rename ajStrNewRes
*/

AjPStr __deprecated ajStrNewL(size_t size)
{
    return ajStrNewRes(size);
}

/* @func ajStrNewResC *********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null) and initialises it with the text 
** string provided.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewResC(const char* txt, size_t size)
{
    ajint i;
    AjPStr thys;

    i = strlen(txt);

    thys = ajStrNewResLenC(txt, size, i);

    return thys;
}



/* @obsolete ajStrNewCL
** @rename ajStrNewResC
*/

AjPStr __deprecated ajStrNewCL(const char* txt, size_t size)
{
    return ajStrNewResC(txt, size);
}

/* @func ajStrNewResS *********************************************************
**
** String constructor which allocates memory for a string of an initial 
** reserved size (including a possible null) and initialises it with the string
** provided.
**
** @param [r] str [const AjPStr] String to initialise
**        the new string.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewResS(const AjPStr str, size_t size)
{
    AjPStr thys;

    thys = ajStrNewResLenC(str->Ptr, size, str->Len);

    return thys;
}


/* @func ajStrNewResLenC ******************************************************
**
** String constructor which allocates memory for a string of a given length 
** and of an initial reserved size (including a possible null) and initialises 
** it with the text string provided.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @param [r] len [size_t] Length of txt to save calculation time.
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewResLenC(const char* txt, size_t size, size_t len)
{
    AjPStr thys;
    ajint minlen;

    minlen = (ajint) size;

    if((size <= len) || (size == NPOS))
	minlen = len+1;

    thys = strNew(minlen);
    thys->Len = len;
    if(txt)
	memmove(thys->Ptr, txt, len+1);
    thys->Ptr[len] = '\0';

    return thys;
}

/* @obsolete ajStrNewCIL
** @rename ajStrNewResLenC
*/

AjPStr __deprecated ajStrNewCIL(const char* txt, ajint len, size_t size)
{
    return ajStrNewResLenC (txt, size, len);
}

/* @funcstatic strCloneL ******************************************************
**
** Makes a new clone of a string with a usage count of one and a minimum
** reserved size.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [void]
** @@
******************************************************************************/

static void strCloneL(AjPStr* Pstr, size_t size)
{
    AjPStr thys;
    AjPStr ret;

    thys = *Pstr;
    ret = ajStrNewResLenC(thys->Ptr, size, thys->Len);
    ajStrDel(Pstr);
    *Pstr = ret;

    return;
}





/* @funcstatic strNew ******************************************************
**
** Internal constructor for modifiable AJAX strings. Used by all the string
** parameterized contructors to allocate the space for the text string.
** The exception is ajStrNew which returns a clone of the null string.
**
** @param [rE] size [size_t] size of the reserved space, including the
**        terminating NULL character. Zero uses a default string size STRSIZE.
** @return [AjPStr] A pointer to an empty string
** @@
******************************************************************************/

static AjPStr strNew(size_t size)
{
    AjPStr ret;

    if(!size)
	size = STRSIZE;

    AJNEW0(ret);
    ret->Res = size;
    ret->Ptr = AJALLOC0(size);
    ret->Len = 0;
    ret->Use = 1;
    ret->Ptr[0] = '\0';

    strAlloc += size;
    strCount++;
    strTotal++;

    return ret;
}




/* @section destructors
**
** Functions for destruction of string objects.
**
** @fdata     [AjPStr]
** @fnote     Return type could be standardised.
**
** @nam3rule  Del       Destroy (free) an existing string.
** @nam3rule  Delarray  Destroy (free) an array of strings
** @nam4rule  DelStatic Destroy (clear) a string without freeing memory
**                      to save reallocation of static string variables
** 
** @argrule   Del Pstr [AjPStr*]
** @argrule   Delarray PPstr [AjPStr**]
** 
** @valrule   * [void]
** @valrule   *DelStatic [AjBool] True if the string still exists as empty
**                               False if the string was reference counted
**                               and this instance was deleted
** @fcategory delete
******************************************************************************/



/* @func ajStrDel *************************************************************
**
** Default string destructor which frees memory for a string.
** 
** Decrements the use count. When it reaches zero, the string is removed from
** memory.  If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [d] Pstr [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [void]
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
** @@
******************************************************************************/

void ajStrDel(AjPStr* Pstr)
{
    AjPStr thys;

    if(!Pstr)
	return;

    if(!*Pstr)
	return;

    thys = *Pstr;

    --thys->Use;
    if(!thys->Use)
    {					/* any other references? */
	if(thys == strPNULL)
	{
	    ajErr("Error - trying to delete the null string constant\n");
	    thys->Use = NULL_USE;     /* restore the original value */
	}
	else
	{
	    AJFREE(thys->Ptr);		/* free the string */

	    strFree += thys->Res;
	    strFreeCount++;
	    strCount--;

	    thys->Res = 0;	      /* in case of copied pointers */
	    thys->Len = 0;

	    AJFREE(*Pstr);		/* free the object */
	}
    }

    *Pstr = NULL;

    return;
}



/* @func ajStrDelStatic *******************************************************
**
** String dereferencing function which sets a string pointer to NULL and 
** decrements the use count of the string to which it referred. Strings with a
** use count of 1 are *not* freed to avoid freeing and reallocating memory when
** they are reused.
**
** Memory reserved for the string is never deleted and can always be
** reused by any remaining AjPStr that points to it even if this pointer
** is cleared.
**
** Use for more efficient memory management for static strings, to avoid
** reallocation when used in a subsequent call.
**
** If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [w] Pstr [AjPStr*] Pointer to the string to be deleted.
** @return [AjBool] True if the string exists and can be reused
**                  False if the string was deleted.
** @@
******************************************************************************/

AjBool ajStrDelStatic(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
	return ajFalse;

    thys = *Pstr;

    if(thys->Use == 1)
    {			       /* last reference - clear the string */
	*thys->Ptr = '\0';
	thys->Len  = 0;
	return ajTrue;
    }
    else
    {
	--thys->Use;
	*Pstr = NULL;
    }

    return ajFalse;
}




/* @obsolete ajStrDelReuse
** @rename ajStrDelStatic
*/

AjBool __deprecated ajStrDelReuse(AjPStr* pthis)
{
    ajStrDelStatic(pthis);
    if(*pthis) return ajTrue;
    return ajFalse;
}


/* @func ajStrDelarray ********************************************************
**
** Default string destructor which frees memory for an array of strings.
** 
** Decrements the use count. When it reaches zero, the string is removed from
** memory.  If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [d] PPstr [AjPStr**] Pointer to the string array to be deleted.
**         The last string is a NULL pointer
**         The pointer is always deleted.
** @return [void]
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
** @@
******************************************************************************/

void ajStrDelarray(AjPStr** PPstr)
{
    ajint i = 0;

    if(!PPstr)
	return;

    if(!*PPstr)
	return;

    for(i=0;(*PPstr)[i];i++)
	ajStrDel(&(*PPstr)[i]);
    AJFREE(*PPstr);

    return;
}


/* @section assignment
**
** Functions for assigning a string.
**
** @fdata      [AjPStr]
** @fnote     Memory for the string is allocated if necessary.
**
** @nam3rule  Assign      Assign a string (or substring) either by reference
**                        or duplication.
** @nam4rule  AssignRef   Copy by setting pointer reference only.
** @nam4rule  AssignSub   Copy a substring.
** @nam4rule  AssignRes   Copy into string of a reserved size (at least).
** @nam4rule  AssignMax   Copy up to a maximum number of characters only.
** @nam4rule  AssignEmpty Copy only if existing string is empty.
**
** @argrule   *       Pstr [AjPStr*] Modifiable string
** @argrule   Ref     refstr [AjPStr] Master string
** @argrule   Res     size [size_t] Reserved maximum size
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Len     len [size_t] Length of string
** @argrule   Sub     pos1 [ajint] Start position, negative value counts
**                                 from end
** @argrule   Sub     pos2 [ajint] End position, negative value counts from end
** @valrule   * [AjBool]
**
** @fcategory assign
*/


/* @func ajStrAssignC *********************************************************
**
** Copy a text string to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignC(AjPStr* Pstr, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajint i;

    if (!txt) {
	*Pstr = ajStrNewResLenC("", 1, 0);
	return ret;
    }

    i = strlen(txt);
    ret  = ajStrSetRes(Pstr, i+1);
    thys = *Pstr;
    thys->Len = i;

    if(i)
	memmove(thys->Ptr, txt, i+1);
    else
	thys->Ptr[0] = '\0';

    return ret;
}

/* @obsolete ajStrAssC
** @rename ajStrAssignC
*/

AjBool __deprecated ajStrAssC(AjPStr* pthis, const char* text)
{
    return ajStrAssignC (pthis, text);
}

/* @func ajStrAssignK *********************************************************
**
** Copy a single character to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] chr [char] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignK(AjPStr* Pstr, char chr)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    ret  = ajStrSetRes(Pstr, 2);
    thys = *Pstr;

    thys->Ptr[0] = chr;
    thys->Ptr[1] = '\0';
    thys->Len = 1;

    return ret;
}


/* @obsolete ajStrAssK
** @rename ajStrAssignK
*/

AjBool __deprecated ajStrAssK(AjPStr* pthis, const char text)
{
    return ajStrAssignK(pthis, text);
}

/* @func ajStrAssignS *********************************************************
**
** Copy a string to a string.
**
** This is a genuine copy (reference count isn't used).  Useful where both 
** strings will be separately overwritten later so that they can both remain
** modifiable.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [rN] str [const AjPStr] Source string.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    if(!str)
    {
	return ajStrAssignC(Pstr, "");
    }

    ret = ajStrSetRes(Pstr, str->Len+1); /* minimum reserved size, OR more */
    thys = *Pstr;

    thys->Len = str->Len;
    memmove(thys->Ptr, str->Ptr, str->Len+1);

    return ret;
}


/* @obsolete ajStrAssS
** @rename ajStrAssignS
*/

AjBool __deprecated ajStrAssS(AjPStr* pthis, const AjPStr str)
{
    return ajStrAssignS(pthis, str);
}

/* @obsolete ajStrAssI
** @replace ajStrAssignS (1,2,3/1,2)
*/

AjBool __deprecated ajStrAssI(AjPStr* Pstr, const AjPStr str, size_t i)
{
    return ajStrAssignS(Pstr, str);
}

/* @func ajStrAssignEmptyC ****************************************************
**
** Ensures a string is set (has a value). If the string is set it is left 
** alone, otherwise it is initialised with a text string.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignEmptyC(AjPStr* Pstr, const char* txt)
{
    AjBool ret = ajFalse;

    if(!*Pstr)
	ret = ajStrAssignC(Pstr, txt);
    else if(!(*Pstr)->Len)
	ret = ajStrAssignC(Pstr, txt);

    return ret;
}



/* @obsolete ajStrSetC
** @rename ajStrAssignEmptyC
*/

AjBool __deprecated ajStrSetC(AjPStr* pthis, const char* str)
{
    return ajStrAssignEmptyC(pthis, str);
}

/* @func ajStrAssignEmptyS ****************************************************
**
** Ensures a string is set (has a value). If the string is set it is left 
** alone, otherwise it is initialised with a string.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [r] str [const AjPStr] Source string object.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignEmptyS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;		/* true if ajStrDup is used */

    if(!*Pstr)
	ret = ajStrAssignS(Pstr, str);
    else if(!(*Pstr)->Len)
	ret = ajStrAssignS(Pstr, str);

    return ret;
}




/* @obsolete ajStrSet
** @rename ajStrAssignEmptyS
*/

AjBool __deprecated ajStrSet(AjPStr* pthis, const AjPStr str)
{
    AjBool ret = ajFalse;
    ajDebug("ajStrSet pthis:%x '%S' str:%x '%S'\n",
	    *pthis, *pthis, str, str);
    ret = ajStrAssignEmptyS(pthis, str);
    ajDebug("ajStrSet ret:%B pthis:%x '%S' str:%x '%S'\n",
	    ret, *pthis, *pthis, str, str);
    return ret;
}

/* @func ajStrAssignLenC ******************************************************
**
** Copy a text string of a given length to a string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [rN] txt [const char*] Source text.
** @param [r] len [size_t] Length of source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignLenC(AjPStr* Pstr, const char* txt, size_t  len)
{
    AjBool ret = ajFalse;
    AjPStr thys;

    if (!txt)
	ajFatal("ajStrAssignLenC source text NULL");

    ret = ajStrSetRes(Pstr, len+1);
    thys = *Pstr;

    thys->Len = len;
    if (len)
	memmove(thys->Ptr, txt, len);

    thys->Ptr[len] = '\0';

    return ret;
}


/* @obsolete ajStrAssCI
** @rename ajStrAssignLenC
*/

AjBool __deprecated ajStrAssCI(AjPStr* pthis, const char* txt, size_t ilen)
{
    return ajStrAssignLenC(pthis, txt, ilen);
}

/* @func ajStrAssignRef *******************************************************
**
** String referencing function which makes a string point to another.  
** Increases the reference count of the string pointed to.
**
** Not a true string copy, but if modified the new string will behave
** as a copy.  
** 
** Sets the destination string to NULL if the source string is NULL.
**
** @param [w] Pstr [AjPStr*] Target string which is overwritten.
** @param [u] refstr [AjPStr] Source string object
**        Copy by reference count so not const.
** @return [AjBool] ajTrue if string was reallocated
** @cre If both arguments point to the same string object, nothing happens.
** @@
******************************************************************************/

AjBool ajStrAssignRef(AjPStr* Pstr, AjPStr refstr)
{
    AjBool ret = ajTrue;		/* true if ajStrDup is used */

    ajStrDel(Pstr);

    if(!refstr)
	ajFatal("ajStrAssignRef source string NULL");

    *Pstr = ajStrNewRef(refstr);

    return ret;
}

/* @obsolete ajStrCopy
** @rename ajStrAssignRef
*/

AjBool __deprecated ajStrCopy(AjPStr* pthis, AjPStr str)
{
    return ajStrAssignRef(pthis, str);
}

/* @func ajStrAssignResC ******************************************************
**
** Copy a text string to a string with a minimum reserved size.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] size [size_t] Space to reserve.
** @param [r] txt [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignResC(AjPStr* Pstr, size_t size, const char* txt)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajint ilen;
    ajint isize;

    if (txt)
	ilen = strlen(txt);
    else
	ilen = 0;

    isize = (ajint) size;

    if(ilen >= isize)
	isize = ilen + 1;

    ret = ajStrSetRes(Pstr, isize);
    thys = *Pstr;

    thys->Len = ilen;

    if(ilen)
	memmove(thys->Ptr, txt, ilen);

    thys->Ptr[ilen] = '\0';

    return ret;
}

/* @obsolete ajStrAssCL
** @replace ajStrAssignResC (1,2,3/1,3,2)
*/

AjBool __deprecated ajStrAssCL(AjPStr* pthis, const char* txt, size_t i)
{
    return ajStrAssignResC(pthis, i, txt);
}




/* @func ajStrAssignResS ******************************************************
**
** Copy a string to a string with a minimum reserved size.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] size [size_t] Size of new string.
** @param [rN] str [const AjPStr] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignResS(AjPStr* Pstr, size_t size, const AjPStr str)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajint isize;

    isize = (ajint) size;

    if (str)
    {
	if(isize <= str->Len)
	    isize = str->Len+1;
    }

    ret = ajStrSetRes(Pstr, isize);
    thys = *Pstr;

    if (str)
    {
	thys->Len = str->Len;
	memmove(thys->Ptr, str->Ptr, str->Len);
	thys->Ptr[str->Len] = '\0';
    }
    else
    {
	thys->Len = 0;
	thys->Ptr[0] = '\0';
    }

    return ret;
}

/* @obsolete ajStrAssL
** @replace ajStrAssignResS (1,2,3/1,3,2)
*/

AjBool __deprecated ajStrAssL(AjPStr* pthis, const AjPStr str, size_t i)
{
    return ajStrAssignResS(pthis, i, str);
}

/* @func ajStrAssignSubC ******************************************************
**
** Copies a substring of a text string to a string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source text
** @param [r] pos1 [ajint] start position for substring
** @param [r] pos2 [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssignSubC(AjPStr* Pstr, const char* txt, ajint pos1, ajint pos2)
{
    ajint ilen;
    ajint ibegin;
    ajint iend;

    ibegin = pos1;
    iend   = pos2;

    if(ibegin < 0)
	ibegin = strlen(txt) + pos1;
    if(iend < 0)
	iend = strlen(txt) + pos2;

    ilen = iend - ibegin + 1;

    return ajStrAssignLenC(Pstr, &txt[ibegin], ilen);
}


/* @obsolete ajStrAssSubC
** @rename ajStrAssignSubC
*/

AjBool __deprecated ajStrAssSubC(AjPStr* pthis, const char* txt,
				 ajint begin, ajint end)
{
    return ajStrAssignSubC(pthis, txt, begin, end);
}

/* @func ajStrAssignSubS ******************************************************
**
** Copies a substring of a string to a string.
**
** The substring is defined from character positions beginpos to endpos.
**
** ajTrue is returned if target was
** (re)allocated, ajFalse is returned otherwise. 
**
** @short Copies a substring of a string to a string.
**
** @param [w] Pstr [AjPStr*] Target string
**          {memory will be automatically allocated if required}
** @param [r] str [const AjPStr] Source string
** @param [r] pos1 [ajint] Start position in src of substring
**              {negative values count from the end of the string
**               with -1 as the last position}
** @param [r] pos2 [ajint] End position in src of substring
**              {negative values count from the end of the string
**               with -1 as the last position}
** @return [AjBool] ajTrue if pthis was (re)allocated, ajFalse otherwise
** @release 1.0.0 
** @@
******************************************************************************/

AjBool ajStrAssignSubS(AjPStr* Pstr, const AjPStr str,
		      ajint pos1, ajint pos2)
{
    ajint ilen;
    ajint ibegin;
    ajint iend;

    ibegin = ajMathPos(str->Len, pos1);
    iend = ajMathPosI(str->Len, ibegin, pos2);
    if(iend == str->Len)
	iend--;

    ilen = iend - ibegin + 1;

    return ajStrAssignLenC(Pstr, &str->Ptr[ibegin], ilen);
}


/* @obsolete ajStrAssSub
** @rename ajStrAssignSubS
*/
AjBool __deprecated ajStrAssSub(AjPStr* pthis, const AjPStr src,
		   ajint beginpos, ajint endpos)
{
    return ajStrAssignSubS(pthis, src, beginpos, endpos);
}


/* @section combination functions.
**
** Functions for appending, inserting or overwriting a string
** or substring to another.
**
** @fdata      [AjPStr]
** @fnote     Memory for the object is allocated if necessary
**
** @nam3rule  Append         Append one string to another.
** @nam4rule  AppendSub    Append substring.
** @nam4rule  AppendCount    Append number of characters.
** @nam3rule  Join        Append one string to another after cutting strings. 
** @nam3rule  Insert      Insert one string into another.
** @nam3rule  Paste       Overwrite one string with another. 
** @nam4rule  PasteCount  Overwrite with a number of single characters
** @nam4rule  PasteMax    Length of overwrite string specified
** @nam3rule  Mask        Mask a range of characters. 
**
** @argrule   *        Pstr [AjPStr*] Modifiable string
** @argrule   Insert  pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   Paste   pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   Join   pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Count     num [size_t] Number of single characters to copy
** @argrule   Len     len [size_t] Number of characters to copy from string
** @argrule   Max     len [size_t] Length of string
** @argrule   Sub     pos1 [ajint] Start position, negative value counts
**                                 from end
** @argrule   Mask     pos1 [ajint] Start position, negative value counts
**                                 from end
** @argrule   Sub     pos2 [ajint] End position, negative value counts from end
** @argrule   Mask    pos2 [ajint] End position, negative value counts from end
** @argrule   Join    posb [ajint] Position in source string
**                                negative values count from the end
** @argrule   Mask    maskchr [char] Masking character
** 
** @valrule   * [AjBool] 
**
** @fcategory modify
** @@
*/



/* @func ajStrAppendC *********************************************************
**
** Appends a text string to the end of a string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] txt [const char*] Source text
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendC(AjPStr* Pstr, const char* txt)
{
    ajint i;

    i = strlen(txt);

    return ajStrAppendLenC(Pstr, txt, i);
}

/* @obsolete ajStrAppC
** @rename ajStrAppendC
*/

AjBool __deprecated ajStrAppC(AjPStr* pthis, const char* txt)
{
    return ajStrAppendC(pthis, txt);
}

/* @func ajStrAppendK *********************************************************
**
** Appends a character to the end of a string.
**
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [rN] chr [char] Source character
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendK(AjPStr* Pstr, char chr)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajint j;

    thys = *Pstr;

    if(thys)
	j = AJMAX(thys->Res, thys->Len+2);
    else
	j = 2;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    thys->Ptr[thys->Len] = chr;
    thys->Ptr[++thys->Len] = '\0';

    return ret;
}

/* @obsolete ajStrAppK
** @rename ajStrAppendK
*/

AjBool __deprecated ajStrAppK(AjPStr* pthis, const char chr)
{
    return ajStrAppendK(pthis, chr);
}

/* @func ajStrAppendS *********************************************************
**
** Appends a string to the end of another string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] str [const AjPStr] Source string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendS(AjPStr* Pstr, const AjPStr str)
{
    AjBool ret = ajFalse;

    AjPStr thys;
    ajint j;

    if(!str) {
	ajFatal("ajStrAppendS source string is NULL");
    }

    thys = *Pstr;

    if(thys)
    {
	j = AJMAX(thys->Res, thys->Len+str->Len+1);
	if(j > thys->Res && thys->Res > LONGSTR)
	    j = j + j/2;
    }
    else
	j = str->Len+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, str->Ptr, str->Len+1);	/* include the null */
    thys->Len += str->Len;

    return ret;
}


/* @obsolete ajStrApp
** @rename ajStrAppendS
*/

AjBool __deprecated ajStrApp(AjPStr* pthis, const AjPStr src)
{
    return ajStrAppendS(pthis, src);
}

/* @func ajStrAppendCountK ****************************************************
**
** Appends any number of a given character to the end of a string.
** 
** Equivalent to a repeat count for ajStrAppK.
**
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] chr [char] Source character
** @param [r] num [size_t] Repeat count
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendCountK(AjPStr* Pstr, char chr, size_t num)
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajint i;
    ajint j;
    char* cp;

    thys = *Pstr;

    if(thys)
	j = AJMAX(thys->Res, (ajint)(thys->Len+num+1));
    else
	j = num+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    cp = &thys->Ptr[thys->Len];
    for(i=0; i<(ajint)num; i++)
    {
	*cp = chr;
	cp++;
    }

    *cp = '\0';
    thys->Len += num;

    return ret;
}

/* @obsolete ajStrAppKI
** @rename ajStrAppendCountK
*/

AjBool __deprecated ajStrAppKI(AjPStr* pthis, const char chr, ajint number)
{
    return ajStrAppendCountK(pthis, chr, number);
}
/* @func ajStrAppendLenC ******************************************************
**
** Appends a text string of a given length to the end of a string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [rN] txt [const char*] Source text
** @param [r] len [size_t] String length
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendLenC(AjPStr* Pstr, const char* txt, size_t len)
{
    AjBool ret = ajFalse;

    AjPStr thys;
    ajint j;

    thys = *Pstr;

    if(!txt)
	ajFatal("ajStrAppendLenC source string is NULL");

    if(*Pstr)
	j = AJMAX(thys->Res, (ajint) (thys->Len+len+1));
    else
	j = len+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, txt, len+1);
    thys->Len += len;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}


/* @obsolete ajStrAppCI
** @rename ajStrAppendLenC
*/

AjBool __deprecated ajStrAppCI(AjPStr* pthis, const char* txt, size_t i)
{
    return ajStrAppendLenC(pthis, txt, i);
}

/* @func ajStrAppendSubS ******************************************************
**
** Appends a substring of a string to the end of another string.
** 
** Uses {ajStrSetRes} to make sure target string is modifiable.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] str [const AjPStr] Source string
** @param [r] pos1 [ajint] start position for substring
** @param [r] pos2 [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppendSubS(AjPStr* Pstr, const AjPStr str, ajint pos1, ajint pos2)
{
    ajint ilen;
    ajint ibegin;
    ajint iend;
    AjBool ret = ajFalse;

    AjPStr thys;
    ajint j;

    thys = *Pstr;

    ibegin = ajMathPos(str->Len, pos1);
    iend   = ajMathPosI(str->Len, ibegin, pos2);

    ilen = iend - ibegin + 1;

    if(thys)
    {
	j = AJMAX(thys->Res, thys->Len+ilen+1);
	if(j > thys->Res && thys->Res > LONGSTR)
	    j = j + j/2;
    }
    else
	j = ilen+1;

    ret = ajStrSetResRound(Pstr, j);
    thys = *Pstr;			/* possible new location */

    memmove(thys->Ptr+thys->Len, &str->Ptr[ibegin], ilen);
    thys->Len += ilen;

    thys->Ptr[thys->Len] = '\0';

    return ret;
}

/* @obsolete ajStrAppSub
** @rename ajStrAppendSubS
*/

AjBool __deprecated ajStrAppSub(AjPStr* pthis, const AjPStr src,
			       ajint begin, ajint end)
{
    return ajStrAppendSubS(pthis, src, begin, end);
}

/* @func ajStrInsertC *********************************************************
**
** Insert a text string into a string at a specified postion.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] txt [const char*] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrInsertC(AjPStr* Pstr, ajint pos, const char* txt )
{
    AjBool ret = ajFalse;
    AjPStr thys;
    ajint j = 0;
    ajint y = 0;
    ajint ibegin;
    char* ptr1;
    const char* ptr2;
    ajint len;

    len = strlen(txt);

    thys = *Pstr;
    if(!thys)
    {
	ajStrAssignResC(Pstr, len+1, "");
	thys = *Pstr;
    }

    /* can be at start or after end */
    ibegin = ajMathPosI(thys->Len+1, 0, pos);

    j = thys->Len+len+1;

    if(j > thys->Res)
    {
	ret = ajStrSetResRound(Pstr, j);
    }
    else
    {
	ajStrGetuniqueStr(Pstr);
	ret = ajTrue;
    }

    thys = *Pstr;			/* possible new location */

    /* move characters "i" places up to leave place for insertion */
    ptr1 =  &thys->Ptr[thys->Len+len];
    ptr2 =  &thys->Ptr[thys->Len];
    for(y=0; y<=thys->Len-ibegin ; y++)
    {
	*ptr1 = *ptr2;
	ptr1--;
	ptr2--;
    }

    thys->Len += len;			/* set the new length */
    thys->Ptr[thys->Len] = '\0';	/* ### was Len+1 ### add the
					   end character */

    /* add the new text */
    ptr1 = & thys->Ptr[ibegin];
    ptr2 = txt;

    for(y=0; y< len; y++)
    {
	*ptr1 = *ptr2;
	ptr1++;
	ptr2++;
    }

    return ret;
}




/* @func ajStrInsertK *********************************************************
**
** Inserts a character into a string at a specified postion.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] chr [char] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrInsertK(AjPStr* Pstr, ajint pos, char chr )
{
    char tmpstr[2] = "?";
    *tmpstr = chr;
    return ajStrInsertC(Pstr, pos, tmpstr);
}

/* @func ajStrInsertS *********************************************************
**
** Inserts a text string into a string at a specified postion.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Position where text is to be inserted.
**                        Negative position counts from the end
** @param [r] str [const AjPStr] String to be inserted
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @error ajFalse if the insert failed. Currently this happens if
**        pos is negative, but this could be reassigned to a position
**        from the end of the string in future.
** @@
******************************************************************************/
AjBool ajStrInsertS(AjPStr* Pstr, ajint pos, const AjPStr str )
{
    return ajStrInsertC(Pstr, pos, str->Ptr);
}

/* @obsolete ajStrInsert
** @rename ajStrInsertS
*/

AjBool __deprecated ajStrInsert(AjPStr* Pstr, ajint pos, const AjPStr str )
{
    return ajStrInsertS(Pstr, pos, str);
}


/* @func ajStrJoinC ***********************************************************
**
** Cut down string at pos1 and add string2 from position pos2.
**
** @param [u] Pstr [AjPStr*] Target string.
** @param [r] pos [ajint] Number of characters to keep in target string.
** @param [r] txt [const char*] Text to append.
** @param [r] posb [ajint] Position of first character to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrJoinC(AjPStr* Pstr, ajint pos, const char* txt,
		   ajint posb)
{
    AjPStr thys;
    ajint len;
    ajint ibegin;
    ajint ibegin2;
    ajint i = 0;
    ajint j = 0;
    ajint newlen = 0;

    len = strlen(txt);

    thys = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos);
    ibegin2 = ajMathPos(len, posb);

    if(thys->Len < ibegin || len < ibegin2)
	return ajFalse;

    newlen = ibegin + len - ibegin2 + 1;

    if(newlen > thys->Res)
    {
	ajStrSetResRound(Pstr, j);
	thys = *Pstr;
    }

    for(i=ibegin,j=ibegin2; j <= len; i++,j++)
	thys->Ptr[i] = txt[j];

    thys->Len = i-1;

    return ajTrue;
}




/* @func ajStrJoinS ***********************************************************
**
** Appends one string to another after cutting both strings.
** 
** Cuts down string at pos1 and add string2 from position pos2.
**
** @param [u] Pstr [AjPStr*] Target string.
** @param [r] pos [ajint] Start position in target string,
**                         negative numbers count from the end.
** @param [r] str [const AjPStr] String to append.
** @param [r] posb [ajint] Starts position to copy,
**                         negative numbers count from the end.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrJoinS(AjPStr* Pstr, ajint pos, const AjPStr str,
		  ajint posb)
{
    ajint ibegin1;
    ajint ibegin2;

    ibegin1 = ajMathPos((*Pstr)->Len, pos);
    ibegin2 = ajMathPos(str->Len, posb);

    return ajStrJoinC(Pstr, ibegin1, str->Ptr, ibegin2);
}



/* @func ajStrMask ************************************************************
**
** Masks out characters from a string over a specified range.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] pos1 [ajint] start position to be masked
** @param [r] pos2 [ajint] end position to be masked
** @param [r] maskchr [char] masking character
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
** @@
******************************************************************************/

AjBool ajStrMask(AjPStr* Pstr, ajint pos1, ajint pos2, char maskchr)
{
    AjPStr thys;
    ajint ibegin;
    ajint iend;
    ajint i;

    thys = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2);

/*
    ajDebug("ajStrMask %d %d len: %d ibegin: %d iend: %d char '%c'\n",
	     begin, end, thys->Len, ibegin, iend, maskchar);
*/

    if(iend < ibegin)
	return ajFalse;

    for(i=ibegin; i<=iend; i++)
	thys->Ptr[i] = maskchr;

    return ajTrue;
}




/* @func ajStrPasteS **********************************************************
**
** Overwrite one string with another. 
** 
** Replace string at pos1 with new string.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Number of characters of target string to keep.
** @param [r] str [const AjPStr] String to replace.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteS( AjPStr* Pstr, ajint pos, const AjPStr str)
{
    return ajStrPasteMaxC(Pstr, pos, str->Ptr, str->Len);
}


/* @obsolete ajStrReplaceS
** @rename ajStrPasteS
*/

AjBool __deprecated ajStrReplaceS( AjPStr* pthis, ajint begin,
				  const AjPStr overwrite)
{
    return ajStrPasteS(pthis, begin, overwrite);
}


/* @func ajStrPasteCountK *****************************************************
**
** Overwrites a string with a number of single characters.
** 
** Replace string at pos and add num copies of character chr.  Or to the end 
** of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Number of characters of target string to keep.
** @param [r] chr [char] Character to replace.
** @param [r] num [size_t] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteCountK( AjPStr* Pstr, ajint pos, char chr,
		      size_t num)
{
    AjPStr thys;
    ajint ibegin;
    ajint iend;
    char* ptr1 = 0;
    ajint i;

    thys = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos);
    iend   = ibegin + num;

    if(iend > thys->Len)		/* can't fit */
	iend = thys->Len - 1;

    ptr1 = &thys->Ptr[ibegin];

    for(i=ibegin;i<iend;i++)
	*ptr1++ = chr;

    return ajTrue;
}


/* @obsolete ajStrReplaceK
** @rename ajStrPasteCountK
*/

AjBool __deprecated ajStrReplaceK( AjPStr* pthis, ajint ibegin,
		      char overwrite, ajint ilen)
{
    return ajStrPasteCountK(pthis, ibegin, overwrite, ilen);
}


/* @func ajStrPasteMaxC *******************************************************
**
** Overwrite one string with a specifed number of characters from a text 
** string.
** 
** Replaces string at pos ands add len characters from text string txt.
** Or to the end of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Start position in target string.
** @param [r] txt [const char*] String to replace.
** @param [r] len [size_t] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteMaxC (AjPStr* Pstr, ajint pos, const char* txt,
		       size_t len)
{
    AjPStr thys;
    ajint ibegin;
    ajint iend;
    char* ptr1       = 0;
    const char* ptr2 = 0;
    ajint slen;
    ajint ilen;

    slen = strlen(txt);

    thys = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos);
    iend = ibegin + len;

    if((iend  > thys->Len) || ((ajint)len > slen) ) /* can't fit */
	return ajFalse;

    ptr1 = &thys->Ptr[ibegin];
    ptr2 = txt;

    for(ilen=len;ilen>0;ilen--)
	*ptr1++ = *ptr2++;

    return ajTrue;
}

/* @obsolete ajStrReplaceC
** @rename ajStrPasteMaxC
*/

AjBool __deprecated ajStrReplaceC( AjPStr* pthis, ajint begin,
				   const char* overwrite, ajint ilen)
{
    return ajStrPasteMaxC(pthis, begin, overwrite, ilen);
}

/* @func ajStrPasteMaxS *******************************************************
**
** Overwrite one string with a specifed number of characters from another 
** string. 

** Replaces string at pos and add len characters from string str.
** Or to the end of the existing string
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] pos [ajint] Start position in tagret string.
** @param [r] str [const AjPStr] Replacement string
** @param [r] len [size_t] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrPasteMaxS(AjPStr* Pstr, ajint pos, const AjPStr str,
		    size_t len)
{
    return ajStrPasteMaxC(Pstr, pos, str->Ptr, len);
}


/* @obsolete ajStrReplace
** @rename ajStrPasteMaxS
*/
AjBool __deprecated ajStrReplace( AjPStr* pthis, ajint begin,
				  const AjPStr overwrite, ajint ilen)
{
    return ajStrPasteMaxS(pthis, begin, overwrite, ilen);
}


/* @section cut
**
** Functions for removing characters or regions (substrings) from a string.
**
** @fdata      [AjPStr]
** @fnote     None.
** @nam3rule  Cut                Remove region(s) from a string. 
** @nam4rule  CutComments      Remove comment lines.
** @nam5rule  CutCommentsStart Remove comment lines beginning with '#' only.
** @nam4rule  CutEnd           Remove one end or another.
** @nam4rule  CutRange         Remove a substring.
** @nam4rule  CutStart         Remove one end or another.
** @nam4rule  CutWild          Remove end from first wildcard character.
** @nam4rule  CutHtml          Remove HTML (i.e. everything between
**                             '<' and '>').
** @nam3rule  Keep             Keep part of a string.
** @nam4rule  KeepRange        Keep range of character positions.
** @nam4rule  KeepSet          Keep only characters in a set.
** @nam5rule  KeepSetAlpha     Also remove non-alphabetic.
** @nam3rule  Quote            Editing quotes in qtrings
** @nam4rule  QuoteStrip       Removing quotes
** @nam5rule  QuoteStripAll    Removing internal and external quotes
** @nam3rule  Remove           Remove individual characters from a string.
** @nam4rule  RemoveGap        Remove non-sequence characters.
** @nam4rule  RemoveHtml       Remove HTML tags.
** @nam4rule  RemoveLast       Remove last character.
** @nam5rule  RemoveLastNewline   Remove last character if a newline only.
** @nam4rule  RemoveNonseq     Remove non-sequence characters
**                               (all chars except alphabetic & '*')
** @nam4rule  RemoveSet        Remove a set of characters.
** @nam4rule  RemoveWhite      Remove whitespace characters.
** @nam5rule  RemoveWhiteExcess  Remove excess whitespace only.
** @nam4rule  RemoveWild       Remove characters after a wildcard.
** @nam3rule  Trim               Remove region(s) of a given character
**                               composition only from start and / or end
**                               of a string.
** @nam4rule  TrimEnd          Trim from end only.
** @nam4rule  TrimStart        Trim from start only.
** @nam3rule  Truncate         Intuitively named end-cut function. 
** @nam4rule  TruncateLen      Truncate to a specified length
** @nam4rule  TruncatePos      Truncate after a specified position
** @nam4rule  TrimWhite        Trim whitespace only.
** @nam5rule  TrimWhiteEnd     Trim whitespace from end.
** @nam5rule  TrimWhiteStart   Trim whitespace from start.
**
** @argrule   *       Pstr [AjPStr*] Modifiable string
** @argrule   Pos     pos [ajint] Position in string to start inserting,
**                                negative values count from the end
** @argrule   C       txt [const char*] Text string
** @argrule   K       chr [char] Single character
** @argrule   S       str [const AjPStr] Text string
** @argrule   Len      len [size_t] Number of characters to copy
** @argrule   CutEnd   len [size_t] Number of characters to copy
** @argrule   CutStart len [size_t] Number of characters to copy
** @argrule   Range pos1 [ajint] Start position in string, negative
**                                  numbers count from end
** @argrule   Range pos2 [ajint] End position in string, negative
**                                  numbers count from end
** @valrule   * [AjBool]
**
** @fcategory modify
*/



/* @func ajStrCutComments *****************************************************
**
** Removes comments from a string.
** 
** A comment begins with a "#" character and may appear anywhere in the string.
** See ajStrCutCommentsStart for alternative definition of a comment.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @return [AjBool] ajTrue if there is some text remaining
** @@
******************************************************************************/

AjBool ajStrCutComments(AjPStr* Pstr)
{
    AjPStr thys;
    char *cp;

    thys = ajStrGetuniqueStr(Pstr);

    if(!thys->Len)		/* empty string */
	return ajFalse;

    cp = strchr(thys->Ptr, '#');
    if(cp)
    {				/* comment found and removed */
	*cp = '\0';
	thys->Len = cp - thys->Ptr;
    }

    if(!thys->Len)	      /* no text before the comment */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrUncomment
** @rename ajStrCutComments
*/

AjBool __deprecated ajStrUncomment(AjPStr* text)
{
    return ajStrCutComments(text);
}

/* @func ajStrCutCommentsStart ************************************************
**
** Removes comments from a string.
** 
** A comment is a blank line or any text starting with a "#" character.
**
** @param [u] Pstr [AjPStr*] Line of text from input file
** @return [AjBool] ajTrue if there is some text remaining
** @@
******************************************************************************/

AjBool ajStrCutCommentsStart(AjPStr* Pstr)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    if(!ajStrGetLen(thys))		/* empty string */
	return ajFalse;

    if(thys->Ptr[0] == '#')
    {					/* comment found */
	ajStrAssignC(Pstr, "");
	return ajFalse;
    }

    return ajTrue;
}


/* @obsolete ajStrUncommentStart
** @rename ajStrCutCommentsStart
*/

AjBool __deprecated ajStrUncommentStart(AjPStr* text)
{
    return ajStrCutCommentsStart(text);
}

/* @func ajStrCutEnd **********************************************************
**
** Removes a number of characters from the end of a string
**
** @param [u] Pstr [AjPStr*] string
** @param [r] len [size_t] Number of characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrCutEnd(AjPStr* Pstr, size_t len)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    if(!len)
	return ajTrue;

    if(len < 0)
	ajFatal("ajStrCutEnd called with negative length '%d' to be cut",
		len);

    if((ajint)len > thys->Len)
	thys->Len = 0;
    else
	thys->Len -= len;

    thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}


/* @obsolete ajStrTrim
** @replace ajStrCutStart (1,+/1,2)
** @replace ajStrCutEnd   (1,-/1,-2)
*/

AjBool __deprecated ajStrTrim(AjPStr* pthis, ajint num)
{
    AjBool ret;
    if(num >= 0)
	ret = ajStrCutStart(pthis, num);
    else
	ret = ajStrCutEnd(pthis, -num);
    return ret;
}

/* @func ajStrCutRange ********************************************************
**
** Removes a substring from a string.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] pos1 [ajint] start position to be cut
** @param [r] pos2 [ajint] end position to be cut
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
** @@
******************************************************************************/

AjBool ajStrCutRange(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjPStr thys;
    ajint ilen;
    ajint ibegin;
    ajint iend;
    ajint irest;

    thys = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2) + 1;
    ilen = iend - ibegin;

    /*ajDebug("ajStrCut %d %d len: %d ibegin: %d iend: %d\n",
	     begin, end, thys->Len, ibegin, iend);*/

    irest = thys->Len - iend + 1;
    if(irest > 0)
	memmove(&thys->Ptr[ibegin], &thys->Ptr[iend], irest);
    thys->Len -= ilen;
    thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}



/* @obsolete ajStrCut
** @rename ajStrCutRange
*/

AjBool __deprecated ajStrCut(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrCutRange(pthis, begin, end);
}


/* @func ajStrCutStart ********************************************************
**
** Removes a number of characters from the start of a string
**
** @param [u] Pstr [AjPStr*] string
** @param [r] len [size_t] Number of characters to delete from the start
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrCutStart(AjPStr* Pstr, size_t len)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    if(!len)
	return ajTrue;

    if(len < 0)
	ajFatal("ajStrCutStart called with negative length '%d' to be cut",
		len);

    if((ajint)len > thys->Len)
	thys->Len = 0;
    else
    {
	memmove(thys->Ptr, &thys->Ptr[len], thys->Len - len);
	thys->Len -= len;
	thys->Ptr[thys->Len] = '\0';
    }

    return ajTrue;
}


/* @func ajStrKeepRange *******************************************************
**
** Reduces target string to a substring of itself by deleting all except
** a range of character positions.
**
** The end is allowed to be before begin, in which case the output is an
** empty string.
**
** @param [w] Pstr [AjPStr*] Target string.
** @param [r] pos1 [ajint] Start position for substring.
** @param [r] pos2 [ajint] End position for substring.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrKeepRange(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjBool ret = ajFalse;
    AjPStr str;
    ajint ibegin;
    ajint ilen;
    ajint iend;

    str = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(str->Len, pos1);
    iend = ajMathPos(str->Len, pos2);

    if(iend == str->Len)
	iend--;

    if(iend < ibegin)
	ilen = 0;
    else
	ilen = iend - ibegin + 1;

    if(ilen)
    {
	if(ibegin)
	    ajMemMove(str->Ptr, &str->Ptr[ibegin], ilen);
	str->Len = ilen;
	str->Ptr[ilen] = '\0';
    }
    else
    {
	str->Len = 0;
	str->Ptr[0] = '\0';
    }

    return ret;
}


/* @obsolete ajStrSub
** @rename ajStrKeepRange
*/

AjBool __deprecated ajStrSub(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrKeepRange(pthis, begin, end);
}

/* @func ajStrKeepSetC ***************************************************
**
** Removes all characters from a string that are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] txt [const char*] Character set to keep
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrKeepSetC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    char *p;
    char *q;

    thys = ajStrGetuniqueStr(Pstr);

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(strchr(txt, *p))
	    *q++=*p;
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @obsolete ajStrKeepC
** @rename ajStrKeepSetC
*/

AjBool __deprecated ajStrKeepC(AjPStr* s, const char* charset)
{
    return ajStrKeepSetC (s, charset);
}

/* @func ajStrKeepSetAlphaC ***************************************************
**
** Removes all characters from a string that are not alphabetic and
** are not in a given set.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @param [r] txt [const char*] Non-alphabetic character set to keep
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrKeepSetAlphaC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    char *p;
    char *q;

    thys = ajStrGetuniqueStr(Pstr);

    p = thys->Ptr;
    q = thys->Ptr;

    while(*p)
    {
	if(isalpha((ajint)*p))
	    *q++=*p;
	else if(strchr(txt, *p))
	    *q++=*p;
	p++;
    }

    *q='\0';
    thys->Len = q - thys->Ptr;

    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @obsolete ajStrKeepAlphaC
** @rename ajStrKeepSetAlphaC
*/

AjBool __deprecated ajStrKeepAlphaC(AjPStr* s, const char* charset)
{
    return ajStrKeepSetAlphaC(s, charset);
}

/* @func ajStrQuoteStrip ******************************************************
**
** Removes any double quotes from a string.
**
** Internal escaped or double quotes are converted to single quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] ajTrue if string is not empty
** @@
******************************************************************************/
AjBool ajStrQuoteStrip(AjPStr* Pstr)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    if(ajStrGetCharLast(thys) == '"')
	ajStrCutEnd(Pstr, 1);

    if(ajStrGetCharFirst(thys) == '"')
	ajStrCutStart(Pstr, 1);

    ajStrExchangeCC(Pstr, "\"\"", "\""); /* doubled quotes to single */
    ajStrExchangeCC(Pstr, "\\\"", "\""); /* escaped quotes to single */

    if(!thys->Len) return ajFalse;

    return ajTrue;
}




/* @func ajStrQuoteStripAll ***************************************************
**
** Removes any single or double quotes from a string.
** 
** Internal escaped or doubled-up quotes are converted to single quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True on success
** @@
******************************************************************************/
AjBool ajStrQuoteStripAll(AjPStr* Pstr)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    if(ajStrGetCharLast(thys) == '"')
    {
	ajStrCutEnd(Pstr, 1);

	if(ajStrGetCharFirst(thys) == '"')
	    ajStrCutStart(Pstr, 1);

	ajStrExchangeCC(Pstr, "\"\"", "\""); /* doubled quotes to single */
	ajStrExchangeCC(Pstr, "\\\"", "\""); /* escaped quotes to single */
    }
    else if(ajStrGetCharLast(thys) == '\'')
    {
	ajStrCutEnd(Pstr, 1);

	if(ajStrGetCharFirst(thys) == '\'')
	    ajStrCutStart(Pstr, 1);

	ajStrExchangeCC(Pstr, "''", "'"); /* doubled quotes to single */
	ajStrExchangeCC(Pstr, "\\'", "'"); /* escaped quotes to single */
    }

    if (!thys->Len) return ajFalse;

    return ajTrue;
}




/* @func ajStrRemoveGap *******************************************************
**
** Removes non-sequence characters (all but alphabetic characters and asterisk)
** from a string.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True if string is not empty
** @@
******************************************************************************/

AjBool ajStrRemoveGap(AjPStr* Pstr)
{
    char *p;
    char *q;
    ajint  i;
    ajint  len;
    char c;
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    p = q = thys->Ptr;
    len = thys->Len;

    for(i=0;i<len;++i)
    {
	c = *(p++);
	if((c=='O') || (c=='o'))	/* O is a gap character for Phylip */
	    --thys->Len;
	else if((c>='A' && c<='Z') || (c>='a' && c<='z') || (c=='*'))
	    *(q++) = c;
	else
	    --thys->Len;
    }

    thys->Ptr[thys->Len] = '\0';

    if(!thys->Len) return ajFalse;

    return ajTrue;
}



/* @obsolete ajStrDegap
** @rename ajStrRemoveGap
*/

void __deprecated ajStrDegap(AjPStr* thys)
{
    ajStrRemoveGap(thys);
}

/* @func ajStrRemoveHtml ******************************************************
**
** Removes html from a string.  
**
** html is defined as all substrings between and including angle brackets.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrRemoveHtml(AjPStr* Pstr)
{
    char *p;
    char *q;
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    p = q = thys->Ptr;
    while(*p)
    {
	if(*p!='<')
	{
	    *q++=*p++;
	    continue;
	}

	while(*p)
	{
	    --thys->Len;
	    if(*p=='>')
	    {
		++p;
		break;
	    }
	    ++p;
	}
    }
    *q='\0';

    if(!thys->Len) return ajFalse;
    return ajTrue;
}




/* @func ajStrRemoveLastNewline ***********************************************
**
** Removes last character from a string if it is a newline character.
**
** @param [w] Pstr [AjPStr*] String
** @return [AjBool] True is string is not empty
** @@
******************************************************************************/

AjBool ajStrRemoveLastNewline(AjPStr* Pstr)
{
    AjPStr thys;

    thys = *Pstr;

    if(thys->Len)
	if(thys->Ptr[thys->Len-1]=='\n')
	{
	    thys->Ptr[thys->Len-1]='\0';
	    --thys->Len;
	}

    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @obsolete ajStrRemoveNewline
** @rename ajStrRemoveLastNewline
*/

void __deprecated ajStrRemoveNewline(AjPStr* thys)
{
    ajStrRemoveLastNewline(thys);
}

/* @func ajStrRemoveSetC ***************************************************
**
** Removes all of a given set of characters from a string.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] txt [const char*] characters to remove
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajStrRemoveSetC(AjPStr* Pstr, const char *txt)
{
    char *p = NULL;
    char *q = NULL;
    char *r = NULL;
    AjPStr thys = NULL;
    
    if(!(*Pstr))
	return ajFalse;
    if(!ajStrGetLen(*Pstr))
	return ajFalse;

    thys = *Pstr;
    
    p = thys->Ptr;
    q = p;

    while((r=strpbrk(p,txt)))
    {
	while(p!=r)
	    *(q++) = *(p++);
	++p;
	--thys->Len;
	*q = '\0';
    }
    if(!thys->Len) return ajFalse;
    return ajTrue;
}

/* @obsolete ajStrRemoveCharsC
** @rename ajStrRemoveSetC
*/

void __deprecated ajStrRemoveCharsC(AjPStr* pthis, const char *strng)
{
    ajStrRemoveSetC(pthis, strng);
}

/* @func ajStrRemoveWhite *****************************************************
**
** Removes excess whitespace characters from a string.
**
** Leading/trailing whitespace removed. Multiple spaces replaced by single
** spaces.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrRemoveWhite(AjPStr* Pstr)
{
    AjBool ret      = ajFalse;
    AjPStr thys;

    ajint i = 0;
    ajint j = 0;
    ajint len;
    char *p;

    /* $$$ need to clean up extra strlen calls here */

    thys = ajStrGetuniqueStr(Pstr);

    p = thys->Ptr;
    
    /* if string was already empty, no need to do anything */
    
    len = thys->Len;
    if(!len)
	return ajFalse;
    
    /* tabs converted to spaces */
    
    for(i=0;i<len;++i)
	if(p[i]=='\t')
	    p[i]=' ';
    
    /* remove leading spaces */
    
    i = 0;
    while(p[i])
    {
	if(p[i]!=' ')
	    break;
	++i;
	--len;
    }
    
    if(i)
    {
	len++;
	memmove(p,&p[i], len);
	len=strlen(p);
	if(!len)
	{			    /* if that emptied it, so be it */
	    thys->Len = 0;
	    return ajFalse;
	}
    }
    
    /* remove newline at the end (if any) */
    
    if(p[len-1]=='\n')
    {
	--len;
	p[len]='\0';
    }
    
    if(!len)
    {				    /* if that emptied it, so be it */
	thys->Len = 0;
	return ajFalse;
    }
    
    /* clean up any space at the end */
    
    for(i=len-1;i>-1;--i)
	if(p[i]!=' ')
	    break;

    p[i+1]='\0';
    
    len=strlen(p);
    
    for(i=j=0;i<len;++i)
    {
	if(p[i]!=' ')
	{
	    p[j++]=p[i];
	    continue;
	}
	p[j++]=' ';

	for(++i;;++i)
	    if(p[i]!=' ')
	    {
		p[j++]=p[i];
		break;
	    }
    }
    
    p[j]='\0';

    thys->Len = j;

    if(!ajStrGetLen(thys))
	return ajFalse;

    return ret;
}


/* @obsolete ajStrClean
** @rename ajStrRemoveWhite
*/

AjBool __deprecated ajStrClean(AjPStr* s)
{
    return ajStrRemoveWhite(s);
}

/* @func ajStrRemoveWhiteExcess ***********************************************
**
** Removes all whitespace characters from a string.
**
** @param [u] Pstr [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrRemoveWhiteExcess(AjPStr* Pstr)
{
    AjPStr thys;
    ajint i = 0;
    ajint j = 0;
    ajint len;
    char *p;

    thys = ajStrGetuniqueStr(Pstr);

    p   = thys->Ptr;
    len = thys->Len;

    for(i=0;i<len;++i)
	if(p[i]=='\t' || p[i]=='\n' || p[i]=='\r')
	    p[i]=' ';

    for(i=0;i<len;++i)
	if(p[i]!=' ')
	    p[j++]=p[i];
	else
	    --thys->Len;

    p[j]='\0';

    if(!thys->Len) return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrCleanWhite
** @rename ajStrRemoveWhiteExcess
*/

AjBool __deprecated ajStrCleanWhite(AjPStr* s)
{
    return ajStrRemoveWhiteExcess(s);
}

/* @func ajStrRemoveWild ******************************************************
**
** Removes all characters after the first wildcard character (if found). 
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if the string contained a wildcard and was
**                  truncated.
** @@
******************************************************************************/

AjBool ajStrRemoveWild(AjPStr* Pstr)
{
    char* cp;
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    cp = thys->Ptr;

    while(*cp)
    {
	switch(*cp)
	{
	case '?':
	case '*':
	    *cp = '\0';
	    thys->Len = cp - thys->Ptr;
	    return ajTrue;
	default:
	    cp++;
	}
    }

    return ajFalse;
}



/* @obsolete ajStrWildPrefix
** @rename ajStrRemoveWild
*/

AjBool __deprecated ajStrWildPrefix(AjPStr* str)
{
    return ajStrRemoveWild(str);
}

/* @func ajStrTrimC ***********************************************************
**
** Removes regions with a given character composition from start and end of a 
** string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from each end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    ajint i;

    thys = ajStrGetuniqueStr(Pstr);

    cp = thys->Ptr;
    i = strspn(cp, txt);
    if(i)
    {
	thys->Len -= i;
	if(thys->Len)
	    memmove(thys->Ptr, &thys->Ptr[i], thys->Len);
	else
	    *thys->Ptr = '\0';
    }

    if(i)
	thys->Ptr[thys->Len] = '\0';

    if(!thys->Len)
	return ajFalse;


    cp = &thys->Ptr[thys->Len-1];
    i = 0;
    while(thys->Len && strchr(txt, *cp))
    {
	i++;
	thys->Len--;
	if(thys->Len)
	    cp--;
    }

    if(i)
	thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}




/* @func ajStrTrimEndC ********************************************************
**
** Removes a region with a given character composition from end of a string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimEndC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    ajint i;

    thys = ajStrGetuniqueStr(Pstr);

    if(!thys->Len)
	return ajFalse;

    cp = &thys->Ptr[thys->Len-1];
    i = 0;
    while(thys->Len && strchr(txt, *cp))
    {
	i++;
	thys->Len--;
	if(thys->Len)
	    cp--;
    }

    if(i)
	thys->Ptr[thys->Len] = '\0';

    return ajTrue;
}




/* @func ajStrTrimStartC ******************************************************
**
** Removes a region with a given character composition from start of a string.
**
** @param [u] Pstr [AjPStr*] string
** @param [r] txt [const char*] Characters to delete from the end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimStartC(AjPStr* Pstr, const char* txt)
{
    AjPStr thys;
    const char* cp;
    ajint i;

    thys = ajStrGetuniqueStr(Pstr);

    if(!thys->Len)
	return ajFalse;

    cp = thys->Ptr;
    i = 0;
    while(thys->Len && strchr(txt, *cp))
    {
	i++;
	thys->Len--;
	cp++;
    }

    if(i)
	memmove(thys->Ptr, &thys->Ptr[i], thys->Len+1);

    return ajTrue;
}




/* @func ajStrTrimWhite *******************************************************
**
** Removes regions composed of white space characters only from the start and 
** end of a string.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimWhite(AjPStr* Pstr)
{
    return ajStrTrimC(Pstr, "\t \n\r");
}


/* @obsolete ajStrChomp
** @rename ajStrTrimWhite
*/

AjBool __deprecated ajStrChomp(AjPStr* pthis)
{
    return ajStrTrimWhite(pthis);
}


/* @obsolete ajStrChompC
** @rename ajStrTrimC
*/

AjBool __deprecated ajStrChompC(AjPStr* pthis, const char* delim)
{
    return ajStrTrimC(pthis, delim);
}

/* @obsolete ajStrChop
** @replace ajStrCutEnd (1/1,'1')
*/

AjBool __deprecated ajStrChop(AjPStr* pthis)
{
    return ajStrCutEnd(pthis, 1);
}



/* @func ajStrTrimWhiteEnd ****************************************************
**
** Removes regions composed of white space characters only from the end of a 
** string.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimWhiteEnd(AjPStr* Pstr)
{
    return ajStrTrimEndC(Pstr, "\t \n\r");
}

/* @obsolete ajStrChompEnd
** @rename ajStrTrimWhiteEnd
*/

AjBool __deprecated ajStrChompEnd(AjPStr* pthis)
{
    return ajStrTrimWhiteEnd(pthis);
}

/* @func ajStrTruncateLen *****************************************************
**
** Removes the end from a string reducing it to a defined length.
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] len [size_t] Length of required string.
**
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTruncateLen(AjPStr* Pstr, size_t len)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    if((ajint)len > thys->Len) return ajTrue;

    thys->Ptr[len] = '\0';
    thys->Len = len;

    return ajTrue;
}



/* @func ajStrTruncatePos *************************************************
**
** Removes the end from a string by cutting at a defined position.
**
** @param [u] Pstr [AjPStr*] target string
** @param [r] pos [ajint] First position to be deleted. Negative values
**                        count from the end
** @return [AjBool] True is string was feallocated
** @@
******************************************************************************/

AjBool ajStrTruncatePos(AjPStr* Pstr, ajint pos)
{
    AjPStr thys;
    ajint ibegin;

    thys = ajStrGetuniqueStr(Pstr);

    ibegin = 1 + ajMathPos(thys->Len, pos);
    thys->Ptr[ibegin] = '\0';
    thys->Len = ibegin;

    return ajTrue;
}


/* @obsolete ajStrTruncate
@rename ajStrTruncateLen
*/

AjBool __deprecated ajStrTruncate(AjPStr* Pstr, ajint pos)
{
    return ajStrTruncateLen(Pstr, pos);
}


/* @section substitution
**
** Functions for subsitutions of characters or regions (substrings)
** within a string.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  Exchange           Substitute substrings in a string with other
**                               strings.
** @nam3rule  Random             Randomly rearrange characters.
** @nam3rule  Reverse            Reverse order of characters.
** @nam4rule  ExchangeSet      Substitute character(s) in a string with
**                               other character(s).
**
** @argrule * Pstr [AjPStr*] String to be edited
** @arg1rule C txt [const char*] Text to be replaced
** @arg1rule K chr [char] Text to be replaced
** @arg1rule S str [const AjPStr] Text to be replaced
** @arg2rule C txtnew [const char*] New text
** @arg2rule K chrnew [char] New text
** @arg2rule S strnew [const AjPStr] New text
**
** @valrule * [AjBool] True if string was reallocated
**
** @fcategory modify
*/

/* @func ajStrExchangeCC ****************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] txt [const char*] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeCC(AjPStr* Pstr, const char* txt,
			 const char* txtnew)
{    
    AjPStr thys;
    AjBool cycle = ajTrue;
    ajint pos    = 0;
    ajint tlen = strlen(txt);

    thys = ajStrGetuniqueStr(Pstr);
    
    if(*txt)
    {
	while(cycle)
	{
	    pos = ajStrFindC(*Pstr, txt);
	    if(pos >= 0)
	    {
		ajStrCutRange(Pstr,pos,pos+tlen-1);
		ajStrInsertC(Pstr,pos,txtnew);
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}

/* @obsolete ajStrSubstituteCC
** @rename ajStrExchangeCC
*/

AjBool __deprecated ajStrSubstituteCC(AjPStr* pthis, const char* replace,
			 const char* putin)
{
    return ajStrExchangeCC(pthis, replace, putin);
}


/* @func ajStrExchangeCS ****************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] txt [const char*] string to replace.
** @param [r] strnew [const AjPStr] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeCS(AjPStr* Pstr, const char* txt,
		       const AjPStr strnew)
{    
    AjPStr thys;
    AjBool cycle = ajTrue;
    ajint pos    = 0;
    ajint tlen = strlen(txt);

    thys = ajStrGetuniqueStr(Pstr);
    
    if(*txt)
    {
	while(cycle)
	{
	    pos = ajStrFindC(thys, txt);
	    if(pos >= 0)
	    {
		ajStrCutRange(Pstr,pos,pos+tlen-1);
		ajStrInsertS(Pstr,pos,strnew);
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}

/* @func ajStrExchangeKK ******************************************************
**
** Replace all occurrences in a string of one character with another. 
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r]  chr [char] Character to replace.
** @param [r]  chrnew [char] Character to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeKK(AjPStr* Pstr, char chr, char chrnew)
{    
    AjBool ret = ajFalse;
    AjPStr thys;
    char* cp;
    
    thys  = ajStrGetuniqueStr(Pstr);

    cp   = thys->Ptr;
    
    while(*cp)
    {
	if(*cp == chr)
	    *cp = chrnew;
	cp++;
    }
    
    return ret;
}


/* @obsolete ajStrSubstituteKK
** @rename ajStrExchangeKK
*/

AjBool __deprecated ajStrSubstituteKK(AjPStr* pthis, char replace, char putin)
{    
    return ajStrExchangeKK(pthis, replace, putin);
}

/* @func ajStrExchangeSC ****************************************************
**
** Replace all occurrences in a string of one substring with another.
**
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] str [const AjPStr] string to replace.
** @param [r] txtnew [const char*] string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSC(AjPStr* Pstr, const AjPStr str,
		       const char* txtnew)
{    
    AjPStr thys;
    AjBool cycle = ajTrue;
    ajint pos    = 0;

    thys = ajStrGetuniqueStr(Pstr);
    
    if(str->Len)
    {
	while(cycle)
	{
	    pos = ajStrFindS(thys, str);
	    if(pos >= 0)
	    {
		ajStrCutRange(Pstr,pos,pos+str->Len-1);
		ajStrInsertC(Pstr,pos,txtnew);
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}

/* @func ajStrExchangeSS ******************************************************
**
** Replace all occurrences in a string of one substring with another.
**
** @param [u] Pstr [AjPStr*]  Target string.
** @param [r] str [const AjPStr] string to replace.
** @param [r] strnew [const AjPStr]   string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSS(AjPStr* Pstr, const AjPStr str, const AjPStr strnew)
{    
    AjPStr thys;
    AjBool cycle = ajTrue;
    ajint pos    = 0;
    
    thys = ajStrGetuniqueStr(Pstr);
    
    if(str->Len !=0)
    {
	while(cycle)
	{
	    pos = ajStrFindC(thys, str->Ptr);
	    if(pos >= 0)
	    {
		ajStrCutRange(Pstr,pos,pos+str->Len-1);
		ajStrInsertS(Pstr,pos,strnew);
	    }
	    else
		cycle = ajFalse;
	}
    }

    return ajTrue;
}


/* @obsolete ajStrSubstitute
** @rename ajStrExchangeSS
*/

AjBool ajStrSubstitute(AjPStr* pthis, const AjPStr replace, const AjPStr putin)
{    
    return ajStrExchangeSS(pthis, replace, putin);
}

/* @func ajStrExchangeSetCC ***************************************************
**
** Replace all occurrences in a string of one set of characters with another
** set. 
**
** @param [w] Pstr [AjPStr*] String
** @param [r] txt [const char*] Unwanted characters
** @param [r] txtnew [const char*] Replacement characters
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSetCC(AjPStr* Pstr, const char* txt, const char* txtnew)
{
    char filter[256] = {'\0'};		/* should make all zero */
    ajint i;

    const char *co;
    const char *cn;
    char* cp;
    AjPStr thys;

    co = txt;
    cn = txtnew;

    thys = ajStrGetuniqueStr(Pstr);

    i = strlen(txtnew);
    if((ajint)strlen(txt) > i)
    {
	ajErr("ajStrExchangeSetCC new char set '%s' shorter than old '%s'",
	       txt, txtnew);
    }

    while(*co)
    {
	if(!*cn)			/* oops, too short, use first */
	    filter[(ajint)*co++] = *txtnew;
	else
	    filter[(ajint)*co++] = *cn++;
    }

    for(cp = thys->Ptr; *cp; cp++)
	if(filter[(ajint)*cp])
	    *cp = filter[(ajint)*cp];

    return ajTrue;
}




/* @obsolete ajStrConvertCC
** @rename ajStrExchangeSetCC
*/

AjBool __deprecated ajStrConvertCC(AjPStr* pthis, const char* oldc,
				 const char* newc)
{
    return ajStrExchangeSetCC(pthis, oldc, newc);
}






/* @func ajStrExchangeSetSS ***************************************************
**
** Replace all occurrences in a string of one set of characters with another
** set. 
**
** @param [w] Pstr [AjPStr*] String
** @param [r] str [const AjPStr] Unwanted characters
** @param [r] strnew [const AjPStr] Replacement characters
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrExchangeSetSS(AjPStr* Pstr, const AjPStr str, const AjPStr strnew)
{
    return ajStrExchangeSetCC(Pstr, str->Ptr, strnew->Ptr);
}


/* @obsolete ajStrConvert
** @rename ajStrExchangeSetSS
*/

AjBool __deprecated ajStrConvert(AjPStr* pthis, const AjPStr oldc,
				 const AjPStr newc)
{
    return ajStrExchangeSetSS(pthis, oldc, newc);
}



/* @func ajStrRandom **********************************************************
**
** Randomly rearranges the characters in a string. 
**
** The application code must first call ajRandomSeed().
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True unless string is empty
** @@
******************************************************************************/
AjBool ajStrRandom(AjPStr* Pstr)
{
    AjPStr copy = NULL;
    AjPStr thys;
    const char *p;
    char *q;

    ajint *rn = NULL;
    ajint *na = NULL;

    ajint len;
    ajint i;

    thys = ajStrGetuniqueStr(Pstr);

    ajStrAssignS(&copy, thys);
    p=ajStrGetPtr(copy);
    q=thys->Ptr;

    len = thys->Len;
    AJCNEW(na, len);
    AJCNEW(rn, len);

    for(i=0;i<len;++i)
    {
	na[i] = i;
	rn[i] = ajRandomNumber();
    }
    ajSortIntIncI(rn,na,len);

    for(i=0;i<len;++i)
	q[i] = p[na[i]];

    AJFREE(na);
    AJFREE(rn);
    ajStrDel(&copy);


    if(!thys->Len) return ajFalse;
    return ajTrue;
}



/* @func ajStrReverse *********************************************************
**
** Reverses the order of characters in a string
**
** @param [w] Pstr [AjPStr*] Target string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrReverse(AjPStr* Pstr)
{
    AjBool ret = ajFalse;
    char *cp;
    char *cq;
    char tmp;
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    cp = thys->Ptr;
    cq = cp + thys->Len - 1;

    while(cp < cq)
    {
	tmp = *cp;
	*cp = *cq;
	*cq = tmp;
	cp++;
	cq--;
    }

    return ret;
}



/* @obsolete ajStrRev
** @rename ajStrReverse
*/

AjBool __deprecated ajStrRev(AjPStr* pthis)
{
    return ajStrReverse(pthis);
}

/* @section query
**
** Functions for querying the properties of strings.
**
** @fdata       [AjPStr]
** @fnote      None.
** @nam3rule   Calc   Calculate a value (not in categories above) derived from
**                    elements of a string object.
** @nam4rule   CalcCount Counts occurence of character(s) in string.
** @nam4rule   CalcWhole Tests whether range refers to whole string.
** @nam3rule   Has    Check whether string contains certain character(s).
** @nam4rule   HasParentheses Contains parentheses.
** @nam4rule   HasWild        Contains wildcard character(s).
** @nam3rule   Is     Check whether string value represents a datatype or is
**                    of a certain character composition.
** @nam4rule   IsBool         Represents Boolean value.
** @nam4rule   IsDouble       Represents double value.
** @nam4rule   IsFloat        Represents float value.
** @nam4rule   IsHex          Represents hex value.
** @nam4rule   IsInt          Represents integer value.
** @nam4rule   IsLong         Represents long value.
** @nam4rule   IsAlnum        Alphanumeric characters only.
** @nam4rule   IsAlpha        Alphabetic characters only.
** @nam4rule   IsLower        No uppercase alphabetic characters.
** @nam4rule   IsNum          Decimal digits only.
** @nam4rule   IsUpper        No lowercase alphabetic characters.
** @nam4rule   IsWhite        Whitespace characters only.
** @nam4rule   IsWild         One or more wildcard characters.
** @nam4rule   IsWord         Non-whitespace characters only. 
** @nam3rule   Whole          Specified range covers whole string. 
**
** @argrule * str [const AjPStr] String
** @argrule Whole pos1 [ajint] Start position, negative values count from end
** @argrule Whole pos2 [ajint] End position, negative values count from end
** @argrule CountC txt [const char*] Text to count
** @argrule CountK chr [char] Character to count
**
** @valrule * [AjBool] Result of query
** @valrule *Count [ajint] Number of occurrences
**
** @fcategory use
*/


/* @func ajStrCalcCountC ******************************************************
**
** Counts occurrences of set of characters in a string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt [const char*] Characters to count
** @return [ajint] Number of times characters were found in string
******************************************************************************/

ajint ajStrCalcCountC(const AjPStr str, const char* txt)
{
    ajint ret = 0;
    const char* cp;

    cp = txt;

    while(*cp)
    {
	ret += ajStrCalcCountK(str, *cp);
	cp++;
    }

    return ret;
}



/* @obsolete ajStrCountC
** @rename ajStrCalcCountC
*/

ajint ajStrCountC(const AjPStr str, const char* txt)
{
    return ajStrCalcCountC(str, txt);
}

/* @func ajStrCalcCountK ******************************************************
**
** Counts occurrences of a character in a string.
**
** @param [r] str [const AjPStr] String
** @param [r] chr [char] Character to count
** @return [ajint] Number of times character was found in string
******************************************************************************/

ajint ajStrCalcCountK(const AjPStr str, char chr)
{
    ajint ret = 0;
    const char* cp;

    cp = ajStrGetPtr(str);;

    while(*cp)
    {
	if(*cp == chr) ret++;
	cp++;
    }

    return ret;
}



/* @obsolete ajStrCountK
** @rename ajStrCalcCountK
*/

ajint __deprecated ajStrCountK(const AjPStr str, char ch)
{
    return ajStrCalcCountK(str, ch);
}

/* @func ajStrHasParentheses **************************************************
**
** Tests whether a string contains (possibly nested) pairs of parentheses.
** 
** Pairs of parentheses are difficult to test for with regular expressions.
**
** @param [r] str [const AjPStr] String to test
** @return [AjBool] ajTrue if string has zero or more () pairs
**                  with possibly other text
** @@
******************************************************************************/

AjBool ajStrHasParentheses(const AjPStr str)
{
    ajint left = 0;
    const char *cp;
    
    cp = ajStrGetPtr(str);
    
    /* if string was already empty, no need to do anything */
    
    while (*cp)
    {
	switch (*cp++)
	{
	case '(':
	    left++;
	    break;
	case ')':
	    if (!left) return ajFalse;
	    left--;
	    break;
	default:
	    break;
	}
    }
    if (left)
	return ajFalse;

    return ajTrue;
}


/* @obsolete ajStrParentheses
** @rename ajStrHasParentheses
*/

AjBool __deprecated ajStrParentheses(const AjPStr s)
{
    return ajStrHasParentheses(s);
}

/* @func ajStrIsAlnum *********************************************************
**
** Test whether a string contains alphanumeric characters only (no white 
** space).
**
** The test is defined by isalnum in the C RTL plus underscores.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphanumeric
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsAlnum(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    for(cp = ajStrGetPtr(str);*cp;cp++)
	if(*cp != '_' && !isalnum((ajint)*cp))
	    return ajFalse;

    return ajTrue;
}



/* @func ajStrIsAlpha *********************************************************
**
** Test whether a string contains alphabetic characters only (no white space).
** 
** Test is defined by isalpha in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphabetic
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsAlpha(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    while(*cp)
	if(!isalpha((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsBool **********************************************************
**
** Tests whether a string represents a valid Boolean value.
**
** The string must be a single character boolean value, or one of "yes, "no",
** "true" or "false".
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a boolean.
** @see ajStrToBool
** @@
******************************************************************************/

AjBool ajStrIsBool(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    if(!strchr("YyTt1NnFf0", *cp))
	return ajFalse;

    if(str->Len == 1)		/* one character only */
	return ajTrue;

/* longer strings - test they really match */

    if(ajStrMatchCaseC(str, "yes"))
	return ajTrue;

    if(ajStrMatchCaseC(str, "no"))
	return ajTrue;

    if(ajStrMatchCaseC(str, "true"))
	return ajTrue;

    if(ajStrMatchCaseC(str, "false"))
	return ajTrue;

    return ajFalse;
}




/* @func ajStrIsDouble ********************************************************
**
** Tests whether a string represents a valid double precision value,
**
** Test uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a double
**         precision number.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsDouble(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    errno = 0;
    strtod(cp, &ptr);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsFloat *********************************************************
**
** Tests whether a string represents a valid floating point value.
**
** Test uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a floating
**         point number.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsFloat(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;
    double d;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    errno = 0;
    d = strtod(cp, &ptr);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    if(d > FLT_MAX)
	return ajFalse;

    if(d < -FLT_MAX)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsHex ***********************************************************
**
** Tests whether a string represents a valid hexadecimal value.
**
** Test uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a hexadecimal value.
** @cre an empty string always returns false.
** @@
******************************************************************************/

AjBool ajStrIsHex(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    errno = 0;
    strtol(cp, &ptr, 16);
    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsInt ***********************************************************
**
** Tests whether a string represents a valid integer value.
**
** Test uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as an integer.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsInt(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    errno = 0;
    strtol(cp, &ptr, 10);
    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsLong **********************************************************
**
** Tests whether a string represents a valid ajlong integer value.
**
** Test uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as an integer.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsLong(const AjPStr str)
{
    const char* cp;
    char* ptr = NULL;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    errno = 0;
    strtol(cp, &ptr, 10);

    if(*ptr || errno == ERANGE)
	return ajFalse;

    return ajTrue;
}




/* @func ajStrIsLower *********************************************************
**
** Tests whether a string contains no upper case alphabetic characters.
**
** Test is defined by isupper in the C RTL..
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphabetic
** @cre an empty string returns ajTrue
** @@
******************************************************************************/

AjBool ajStrIsLower(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajTrue;

    if(!str->Len)
	return ajTrue;

    cp = ajStrGetPtr(str);

    while(*cp)
	if(isupper((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsNum ***********************************************************
**
** Test whether a string contains decimal digits only.
**
** Test is defined by isdigit in the C RTL..
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely numeric
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsNum(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    for(cp = ajStrGetPtr(str);*cp;cp++)
	if(!isdigit((ajint)*cp))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsUpper *********************************************************
**
** Test whether a string contains no lower case alphabetic characters.
**
** Test is defined by islower in the C RTL..
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string has no lower case characters.
** @cre an empty string returns ajTrue
** @@
******************************************************************************/

AjBool ajStrIsUpper(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    while(*cp)
	if(islower((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}




/* @func ajStrIsWhite ********************************************************
**
** Test whether a string contains whitespace characters only.
**
** The test is made using the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string is only white space (or empty).
** @@
******************************************************************************/

AjBool ajStrIsWhite(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajTrue;

    cp = ajStrGetPtr(str);

    while (*cp)
    {
	if (!isspace((int)*cp)) return ajFalse;
	cp++;
    }

    return ajTrue;
}




/* @func ajStrIsWild **********************************************************
**
** Tests whether a string contains the standard wildcard characters * or ?.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if string has wildcards.
** @@
******************************************************************************/

AjBool ajStrIsWild(const AjPStr str)
{
    static AjPRegexp wildexp = NULL;

    if(!wildexp)
	wildexp = ajRegCompC("([*?])");

    return ajRegExec(wildexp, str);
}





/* @func ajStrIsWord **********************************************************
**
** Test whether a string contains no white space characters. 
**
** Test is defined by isspace in the C RTL.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if the string has no white space
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsWord(const AjPStr str)
{
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);

    while(*cp)
	if(isspace((ajint)*cp++))
	    return ajFalse;

    return ajTrue;
}


/* @obsolete ajStrIsSpace
** @rename ajStrIsWhite
*/

AjBool __deprecated ajStrIsSpace(const AjPStr thys)
{
    return ajStrIsWhite(thys);
}





/* @func ajStrWhole ***********************************************************
**
** Tests whether a range refers to the whole string
**
** @param [r] str [const AjPStr] String
** @param [r] pos1 [ajint] Begin position (0 start, negative from the end)
** @param [r] pos2 [ajint] Begin position (0 start, negative from the end)
** @return [AjBool] ajTrue is range covers the whole string
******************************************************************************/

AjBool ajStrWhole(const AjPStr str, ajint pos1, ajint pos2)
{
    ajint ibeg;
    ajint iend;

    ibeg = ajMathPos(str->Len, pos1);
    if(!ibeg)
	return ajFalse;

    iend = ajMathPosI(str->Len, ibeg, pos2);
    if(iend != (str->Len - 1))
	return ajFalse;

    return ajTrue;
}




/* @section element retrieval
**
** Functions for returning elements of a string object.
**
** @fdata       [AjPStr]
** @fnote      The last word of the ajStrGet family of functions is the
**             element variable name.
** @fnote      Each ajXxxGet function should be provided as a macro.
**
** @nam3rule   Get         Retrieve an unmodified element of a string object. 
** @nam4rule   GetChar     Retrieve first single character
** @nam5rule   GetCharFirst     Retrieve first single character
** @nam5rule   GetCharLast     Retrieve last single character
** @nam5rule   GetCharPos    Retrieve a single character
** @nam4rule   GetLen     Retrieve length
** @nam4rule   GetPtr     Retrieve char* pointer
** @nam4rule   GetRes     Retrieve eserved memory size
** @nam4rule   GetPtr         Retrieve additional space available
** @nam4rule   GetRoom         Retrieve additional space available
** @nam4rule   GetUse     Retrieve use (reference) count
** @nam4rule   GetValid     Test string is valid
**
** @argrule * str [const AjPStr] String
** @argrule GetCharPos pos [ajint] Position, negative values count from the end
**
** @valrule GetChar [char] Single character
** @valrule GetLen [ajint] String length
** @valrule GetRes [ajint] Reserved size
** @valrule GetPtr [const char*] read-only C string pointer to internal string
** @valrule GetuniquePtr [char*] Modifiable C string pointer to internal string
** @valrule GetUse [ajint] Reference (use) count
** @valrule GetRoom [ajint] Remaining space to extend string
**                          without reallocating
** @valrule GetValid [AjBool] True if string is valid
**
** @fcategory use
*/

/* @func ajStrGetCharFirst ****************************************************
**
** Returns the first character from a string.
**
** @param [r] str [const AjPStr] String
** @return [char] First character or null character if empty.
** @@
******************************************************************************/

char ajStrGetCharFirst(const AjPStr str)
{
    return str->Ptr[0];
}


/* @func ajStrGetCharLast ****************************************************
**
** Returns the last character from a string.
**
** @param [r] str [const AjPStr] String
** @return [char] Last character or null character if empty.
** @@
******************************************************************************/

char ajStrGetCharLast(const AjPStr str)
{
    if(!str->Len)
	return '\0';

    return str->Ptr[str->Len - 1];
}


/* @func ajStrGetCharPos ******************************************************
**
** Returns a single character at a given position from a string.
**
** @param [r] str [const AjPStr] String
** @param [r] pos [ajint] Position in the string, negative values are
**        from the end of the string.
** @return [char] Character at position pos or null character if out of range.
** @@
******************************************************************************/

char ajStrGetCharPos(const AjPStr str, ajint pos)
{
    ajint ipos;

    if(!str)
	return '\0';

    if(pos < 0)
	ipos = str->Len + pos;
    else
	ipos = pos;

    if((ipos < 0) || (ipos > str->Len))
	return '\0';

    return str->Ptr[ipos];
}


/* @obsolete ajStrChar
** @replace ajStrGetCharFirst (1,'0'/1)
** @replace ajStrGetCharLast (1,'-1'/1)
** @replace ajStrGetCharPos (1,2/1,2)
*/

char __deprecated ajStrChar(const AjPStr thys, ajint pos)
{
    if(pos == 0)
	ajStrGetCharFirst(thys);
    if(pos == -1)
	ajStrGetCharLast(thys);
    
    return ajStrGetCharPos(thys, pos);
}

/* @func ajStrGetLen **********************************************************
**
** Returns the current length of the C (char *) string.
**
** The length will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string length
** @@
******************************************************************************/

ajint ajStrGetLen(const AjPStr str)
{
    if(!str)
	return 0;

    else
	return str->Len;
}


/* @macro MAJSTRGETLEN ********************************************************
**
** Returns the current length of the C (char *) string. 
**
** A macro version of {ajStrGetLen} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string length
** @@
******************************************************************************/




/* @obsolete ajStrLen
** @rename ajStrGetLen
*/

ajint __deprecated ajStrLen(const AjPStr thys)
{
    return ajStrGetLen(thys);
}

/* @obsolete MAJSTRLEN
** @rename MAJSTRGETLEN
*/

ajint __deprecated MAJSTRLEN(const AjPStr thys)
{
    return MAJSTRGETLEN(thys);
}

/* @func ajStrGetPtr **********************************************************
**
** Returns the current pointer to C (char *) string.
**
** The pointer will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [const char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/

const char* ajStrGetPtr(const AjPStr str)
{
    if(!str)
	return charNULL;

    return str->Ptr;
}



/* @macro MAJSTRGETPTR ********************************************************
**
** Returns the current pointer to C (char *) string.
**
** A macro version of {ajStrGetPtr} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [const char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/




/* @obsolete ajStrStr
** @rename ajStrGetPtr
*/

const char __deprecated * ajStrStr(const AjPStr thys)
{
    return ajStrGetPtr(thys);
}

/* @obsolete MAJSTRSTR
** @rename MAJSTRGETPTR
*/

const char __deprecated *MAJSTRSTR(const AjPStr thys)
{
    return MAJSTRGETPTR(thys);
}

/* @func ajStrGetRes **********************************************************
**
** Returns the current reserved size of the C (char *) string. 
**
** The reserved size will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string reserved size
** @@
******************************************************************************/

ajint ajStrGetRes(const AjPStr str)
{
    if(!str)
	return 0;

    return str->Res;
}



/* @macro MAJSTRGETRES ********************************************************
**
** Returns the current reserved size of the C (char *) string. 
** 
** A macro version of {ajStrGetRes} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string reserved size
** @@
******************************************************************************/




/* @obsolete ajStrSize
** @rename ajStrGetRes
*/
ajint __deprecated ajStrSize(const AjPStr thys)
{
    return ajStrGetRes(thys);
}

/* @obsolete MAJSTRSIZE
** @rename MAJSTRGETRES
*/
ajint __deprecated MAJSTRSIZE(const AjPStr thys)
{
    return MAJSTRGETRES(thys);
}

/* @func ajStrGetRoom *********************************************************
**
** Returns the additional space available in a string before it would require
** reallocating.
**
** @param [r] str [const AjPStr] String
** @return [ajint] Space available for additional characters.
** @@
******************************************************************************/

ajint ajStrGetRoom(const AjPStr str)
{
    return (str->Res - str->Len - 1);
}


/* @obsolete ajStrRoom
** @rename ajStrGetRoom
*/

ajint __deprecated ajStrRoom(const AjPStr thys)
{
    return ajStrGetRoom(thys);
}

/* @func ajStrGetUse **********************************************************
**
** Returns the current usage count of the C (char *) string. 
**
** The usage count will remain valid unless the string is resized or deleted.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string usage count
** @@
******************************************************************************/

ajint ajStrGetUse(const AjPStr str)
{
    if(!str)
	return 0;

    return str->Use;
}



/* @macro MAJSTRGETUSE ********************************************************
**
** Returns the current usage count of the C (char *) string. 
** 
** A macro version of {ajStrGetUse} available in case it is needed for speed.
**
** @param [r] str [const AjPStr] Source string
** @return [ajint] Current string usage count
** @@
******************************************************************************/




/* @obsolete ajStrRef
** @rename ajStrGetUse
*/

ajint __deprecated ajStrRef(const AjPStr thys)
{
    return ajStrGetUse(thys);
}

/* @obsolete MAJSTRREF
** @rename MAJSTRGETUSE
*/

ajint __deprecated MAJSTRREF(const AjPStr thys)
{
    return MAJSTRGETUSE(thys);
}

/* @func ajStrGetValid ********************************************************
**
** Checks a string object for consistency. 
**
** Intended for debugging and testing of these routines, but made publicly 
** available.
**
** @param [r] str [const AjPStr] String
** @return [AjBool] ajTrue if no errors were found.
** @@
******************************************************************************/

AjBool ajStrGetValid(const AjPStr str)
{
    AjBool ret = ajTrue;

    if(!str)
    {
	ajErr("ajStrGetValid: NULL string pointer");
	ret = ajFalse;
    }

    if(str->Use < 0)
    {
	ajErr("ajStrGetValid: Bad use value %d", str->Use);
	ret = ajFalse;
    }

    if(str->Res < 1)
    {
	ajErr("ajStrGetValid: Bad size value %d", str->Res);
	ret = ajFalse;
    }

    if(str->Len < 0)
    {
	ajErr("ajStrGetValid: Bad length value %d\n", str->Len);
	ret = ajFalse;
    }

    if(str->Len >= str->Res)
    {
	ajErr("ajStrGetValid: Size %d too small for length %d\n",
	       str->Res, str->Len);
	ret = ajFalse;
    }

    if(!str->Ptr)
    {
	ajErr("ajStrGetValid: NULL pointer\n");
	ret = ajFalse;
    }
    else
    {
	if(str->Len != strlen(str->Ptr))
	{
	    ajErr("ajStrGetValid: Len %d differs from strlen %d\n",
		   str->Len, strlen(str->Ptr));
	    ret = ajFalse;
	}
    }

    return ret;
}




/* @section modifiable string retrieval
**
** Functions for returning elements of a string object.
**
** @fdata       [AjPStr]
** @fnote      The last word of the ajStrGet family of functions is the
**             element variable name.
** @fnote      Each ajXxxGet function should be provided as a macro.
**
** @nam3rule   Getunique   Single copy strings
** @nam4rule   GetuniquePtr   Retrieve char* pointer as single copy
** @nam4rule   GetuniqueStr   Retrieve string pointer as single copy
**
** @argrule * Pstr [AjPStr*] String
**
** @valrule GetuniqueStr [AjPStr] Modifiable string pointer to internal string
** @valrule GetuniquePtr [char*] Modifiable C string pointer to internal string
**
** @fcategory modify
*/

/* @func ajStrGetuniquePtr ****************************************************
**
** Makes the string value unique by copying any string with a reference count
** of more than 1, and returns the current C (char *) pointer.
**
** As the input string is not const, the calling program must be able
** to modify it in order to call this function.
**
** If the length of the string is changed, it is the responsibility of
** the caller to reset it with ajStrSetValid
**
** @param [u] Pstr [AjPStr*] Source string
** @return [char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/

char* ajStrGetuniquePtr(AjPStr *Pstr)
{
    AjPStr thys;

    thys = ajStrGetuniqueStr(Pstr);

    return thys->Ptr;
}




/* @obsolete ajStrStrMod
** @rename ajStrGetuniquePtr
*/

char __deprecated * ajStrStrMod(AjPStr *pthis)
{
    return ajStrGetuniquePtr(pthis);
}

/* @func ajStrGetuniqueStr ****************************************************
**
** Make certain a string is modifiable by checking it has no other references, 
** or by making a new real copy of the string.
**
** Uses strCloneL to copy without copying the reference count.
**
** The target string is guaranteed to have a reference count of exactly 1.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjPStr] The new string pointer, or NULL for failure
** @@
******************************************************************************/

AjPStr ajStrGetuniqueStr(AjPStr* Pstr)
{
    AjPStr thys;

    if(!*Pstr)
    {
	*Pstr = ajStrNew();
	return *Pstr;
    }

    thys = *Pstr;
    if(thys->Use > 1)
    {
	strCloneL(Pstr, thys->Res);
    }

    return *Pstr;
}




/* @obsolete ajStrMod
** @rename ajStrGetUniqueStr
*/

AjBool ajStrMod(AjPStr* pthis)
{
    AjBool ret = ajTrue;
    if ((*pthis)->Use == 1)
	ret = ajFalse;
    ajStrGetuniqueStr(pthis);
    return ret;
}

/* @section element assignment
**
** Functions for assigning elements of a string object.
**
** @fdata       [AjPStr]
** @fnote      The last word of the ajStrGet family of functions is the
**             element variable name.
** @fnote      Each ajXXXSet function should be provided as a macro.
** @nam3rule   Set       Set elements (attributes)  
** @nam4rule   SetClear  Clear all elements (empty string, length zero)
** @nam4rule   SetLen  
** @nam4rule   SetRes    Set (increase) reserved memory
** @nam5rule   SetResRound  Maximum rounded up by a standard amount (32)
** @nam4rule   SetUnique    Set reference count to 1, making a copy if needed
** @nam4rule   SetValid   Sets the string elements to be valid,
**                    for example adjusting the string length after editing.
**
** @argrule * Pstr [AjPStr*] String
** @argrule Res size [size_t] Reserved size
** @argrule Len len [size_t] Correct string length
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/

/* @func ajStrSetClear ********************************************************
**
** Clears all elemements in a string object.
**
** If the string is already empty nothing happens. If the string has data, 
** it makes sure the string is modifiable and sets it to empty.
**
** @param  [w] Pstr [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrSetClear(AjPStr* Pstr)
{
    AjPStr thys;

    thys  = ajStrGetuniqueStr(Pstr);

    thys->Ptr[0] = '\0';
    thys->Len = 0;

    return ajTrue;
}


/* @obsolete ajStrClear
** @rename ajStrSetClear
*/

AjBool __deprecated ajStrClear(AjPStr* pthis)
{
    return ajStrSetClear(pthis);
}

/* @func ajStrSetRes **********************************************************
**
** Ensures a string is modifiable and big enough for its intended purpose.
**
** The target string is guaranteed to have a reference count of 1 and a 
** minimum reserved size.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
** @@
******************************************************************************/

AjBool ajStrSetRes(AjPStr* Pstr, size_t size)
{
    AjPStr thys;
    size_t savesize;

    savesize = size; /* often part of *Pstr, about to vanish */

    if(!*Pstr)
    {
	*Pstr = ajStrNewRes(savesize);
	return ajTrue;
    }

    thys = *Pstr;

    if((thys->Use > 1) || (thys->Res < (ajint)savesize))
    {
	strCloneL(Pstr, savesize);
	return ajTrue;
    }

    return ajFalse;
}



/* @obsolete ajStrModL
** @rename ajStrSetRes
*/
AjBool __deprecated ajStrModL(AjPStr* pthis, size_t size)
{
    return ajStrSetRes(pthis, size);
}

/* @func ajStrSetResRound *****************************************************
**
** Ensures a string is modifiable and big enough for its intended purpose. 
**
** The target string is guaranteed to have a reference count of 1,
** and a minimum reserved size.  The string size is rounded up.
**
** @param [w] Pstr [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
** @@
******************************************************************************/

AjBool ajStrSetResRound(AjPStr* Pstr, size_t size)
{
    AjPStr thys;
    size_t roundsize;

    if(!*Pstr)
    {
	roundsize = ajRound(size, STRSIZE);
	*Pstr = ajStrNewRes(roundsize);
	return ajTrue;
    }

    thys = *Pstr;
    if((thys->Use > 1 || thys->Res < (ajint)size))
    {
	roundsize = ajRound(size, STRSIZE);
	strCloneL(Pstr, roundsize);
	return ajTrue;
    }

    return ajFalse;
}



/* @obsolete ajStrModMinL
** @rename ajStrSetResRound
*/

AjBool __deprecated ajStrModMinL(AjPStr* pthis, size_t size)
{
    return ajStrSetResRound(pthis, size);
}

/* @func ajStrSetValid ********************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] Pstr [AjPStr*] String.
** @return [AjBool] True on success.
** @@
******************************************************************************/

AjBool ajStrSetValid(AjPStr *Pstr)
{
    AjBool ret = ajTrue;
    AjPStr thys = *Pstr;

    if(thys->Use > 1)
    {
	ajWarn("ajStrSetValid called for string in use %d times\n", thys->Use);
	ret = ajFalse;
    }

    thys->Len = strlen(thys->Ptr);

    if(thys->Len > (thys->Res - 1))	/* ouch someone went off the end */
    {
	ajWarn("ajStrSetValid found string with length %d in size %d",
	       thys->Len, thys->Res);
	ret = ajFalse;
	ajStrSetRes(Pstr, thys->Len+1);
    }

    return ret;
}



/* @obsolete ajStrFix
** @rename ajStrSetValid
*/

void __deprecated ajStrFix(AjPStr *pthis)
{
    ajStrSetValid(pthis);
    return;
}

/* @func ajStrSetValidLen *****************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] Pstr [AjPStr*] String
** @param [r] len [size_t] Length expected.
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajStrSetValidLen(AjPStr* Pstr, size_t len)
{
    AjBool ret = ajTrue;
    AjPStr thys = *Pstr;

    /* check 3 error conditions */

    if(thys->Use > 1)
    {
	ajWarn("ajStrFixI called for string in use %d times\n", thys->Use);
	ret = ajFalse;
    }

    if((ajint)len >= thys->Res)
    {
	ajWarn("ajStrFixI called with length %d for string with size %d\n",
	       len, thys->Res);
	thys->Ptr[thys->Res-1] = '\0';
	len = strlen(thys->Ptr);
	ret = ajFalse;
    }

    thys->Ptr[len] = '\0';

    thys->Len = len;

    return ret;
}



/* @obsolete ajStrFixI
** @rename ajStrSetValidLen
*/

void __deprecated ajStrFixI(AjPStr* pthis, ajint ilen)
{
    ajStrSetValidLen(pthis, ilen);
    return;
}

/* @section string to datatype conversion functions 
**
** Functions for converting strings to other datatypes.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  To            Convert string value to a different datatype.
** @nam4rule  ToBool        Convert to boolean
** @nam4rule  ToDouble        Convert to double
** @nam4rule  ToFloat        Convert to float
** @nam4rule  ToHex        Convert to hexadecimal
** @nam4rule  ToInt        Convert to integer
** @nam4rule  ToLong        Convert to long
**
** @argrule * str [const AjPStr] String
** @argrule ToBool Pval [AjBool*] Boolean return value
** @argrule ToDouble Pval [double*] Double return value
** @argrule ToFloat Pval [float*] Float return value
** @argrule ToHex Pval [ajint*] Integer return value
** @argrule ToInt Pval [ajint*] Integer return value
** @argrule ToLong Pval [ajlong*] Long integer return value
**
** @valrule * [AjBool] True on success
**
** @fcategory cast
*/

/* @func ajStrToBool **********************************************************
**
** Converts a string into a Boolean value.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [AjBool*] ajTrue if the string is "true" as a boolean.
** @return [AjBool] ajTrue if the string had a valid boolean value.
** @cre an empty string returns ajFalse.
** @see ajStrIsBool
** @@
******************************************************************************/

AjBool ajStrToBool(const AjPStr str, AjBool* Pval)
{
    const char* cp;
    ajint i;

    *Pval = ajFalse;

    if(!str)
	return ajFalse;

    if(str->Len < 1)
	return ajFalse;

    cp = ajStrGetPtr(str);

    if(strchr("YyTt1", *cp))
    {
	*Pval = ajTrue;
	if(str->Len == 1)
	    return ajTrue;

	if(ajStrMatchCaseC(str, "yes"))
	    return ajTrue;

	if(ajStrMatchCaseC(str, "true"))
	    return ajTrue;

	return ajFalse;
    }
    else if(strchr("NnFf", *cp))
    {
	*Pval = ajFalse;
	if(!ajStrGetCharPos(str, 1))
	    return ajTrue;

	if(ajStrMatchCaseC(str, "no"))
	    return ajTrue;

	if(ajStrMatchCaseC(str, "false"))
	    return ajTrue;

	return ajFalse;
    }
    else if(strchr("123456789", *cp))
    {
	*Pval = ajTrue;
	if(ajStrIsFloat(str))
	    return ajTrue;

	return ajFalse;
    }
    else if(strchr("0+-", *cp))
    {
	i = strcspn(cp, "123456789");	/* e.g. 0.1, 0007 */
	if(cp[i])
	    *Pval = ajTrue;
	else
	    *Pval = ajFalse;
	if(ajStrIsFloat(str))
	    return ajTrue;

	return ajFalse;
    }

    return ajFalse;
}




/* @func ajStrToDouble ********************************************************
**
** Converts a string into a double precision value.
**
** Uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [double*] String represented as a double precision number.
** @return [AjBool] ajTrue if the string had a valid double precision value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToDouble(const AjPStr str, double* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    double d;
    char* ptr = NULL;

    *Pval = 0.0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = ajStrGetPtr(str);

    errno = 0;
    d = strtod(cp, &ptr);

    if(!*ptr  && errno != ERANGE)
    {
	*Pval = d;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToFloat *********************************************************
**
** Converts a string into a floating point value.
**
** Uses the strtod call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [float*] String represented as a floating point number.
** @return [AjBool] ajTrue if the string had a valid floating point value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToFloat(const AjPStr str, float* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    double d;
    char* ptr = NULL;

    *Pval = 0.0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = ajStrGetPtr(str);

    errno = 0;
    d = strtod(cp, &ptr);

    if(!*ptr  && errno != ERANGE)
    {
	if(d > FLT_MAX)
	    return ajFalse;

	if(d < -FLT_MAX)
	    return ajFalse;

	*Pval = (float) d;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToHex ***********************************************************
**
** Converts a string from hexadecimal into an integer value.
**
** Uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajint*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid hexadecimal value.
** @cre an empty string returns ajFalse.
** @see ajStrIsHex
** @@
******************************************************************************/

AjBool ajStrToHex(const AjPStr str, ajint* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = ajStrGetPtr(str);
    errno = 0;
    l = strtol(cp, &ptr, 16);
    if(!*ptr && errno != ERANGE)
    {
	l = AJMAX(INT_MIN, l);
	l = AJMIN(INT_MAX, l);
	*Pval =(ajint) l;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToInt ***********************************************************
**
** Converts a string into an integer value.
**
** Uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajint*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToInt(const AjPStr str, ajint* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = ajStrGetPtr(str);

    errno = 0;
    l = strtol(cp, &ptr, 10);
    if(!*ptr && errno != ERANGE)
    {
	l = AJMAX(INT_MIN, l);
	l = AJMIN(INT_MAX, l);
	*Pval =(ajint) l;
	ret = ajTrue;
    }

    return ret;
}




/* @func ajStrToLong **********************************************************
**
** Converts a string into an integer value.
**
** Uses the strtol call in the C RTL.
**
** @param [r] str [const AjPStr] String
** @param [w] Pval [ajlong*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToLong(const AjPStr str, ajlong* Pval)
{
    AjBool ret = ajFalse;
    const char* cp;
    ajlong l;
    char* ptr;

    *Pval = 0;

    if(!str)
	return ret;

    if(!str->Len)
	return ret;

    cp = ajStrGetPtr(str);

    errno = 0;
    l = strtol(cp, &ptr, 10);

    if(!*ptr && errno != ERANGE)
    {
	*Pval = l;
	ret = ajTrue;
    }

    return ret;
}




/* @section datatype to string conversion functions 
**
** Functions for converting datatypes to strings.
**
** @fdata      [AjPStr]
** @fnote     See also "String to datatype conversion functions: C-type (char*)
**            strings"
** @nam3rule  From          Convert a datatype value to a string.
** @nam4rule  FromBool        Convert from boolean
** @nam4rule  FromDouble        Convert from double
** @nam5rule  FromDoubleExp        Convert from double in exponential form
** @nam4rule  FromFloat        Convert from float
** @nam4rule  FromInt        Convert from integer
** @nam4rule  FromLong        Convert from long
**
** @argrule * Pstr [AjPStr*] String result
** @argrule FromBool val [AjBool] boolean value
** @argrule FromDouble val [double] Double value
** @argrule FromFloat val [float] Float value
** @argrule FromInt val [ajint] Integer value
** @argrule FromLong val [ajlong] Long integer value
** @argrule FromDouble precision [ajint] Floating point precision
** @argrule FromFloat precision [ajint] Floating point precision
**
** @valrule * [AjBool] True on success
**
** @fcategory derive
*/

/* @func ajStrFromBool ********************************************************
**
** Converts a Boolean value into a 1-letter string. 
**
** Can be used to print boolean values, but the ajFmt library has better ways.
**
** @param [w] Pstr [AjPStr*] String to hold the result.
** @param [r] val [AjBool] Boolean value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromBool(AjPStr* Pstr, AjBool val)
{
    AjBool ret = ajFalse;
    static char bool_y[] = "Y";
    static char bool_n[] = "N";

    if(val)
	ret = ajStrAssignC(Pstr, bool_y);
    else
	ret = ajStrAssignC(Pstr, bool_n);

    return ret;
}


/* @func ajStrFromDouble ******************************************************
**
** Converts a double precision value into a string. 
**
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [double] Double precision value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromDouble(AjPStr* Pstr, double val, ajint precision)
{
    AjBool ret = ajFalse;
    long long i;
    char fmt[12];
    AjPStr thys;
    long long ival;

    ival = (long long) fabs(val);

    if(ival)
	i = precision + (ajint) log10((double)ival) + 4;
    else
	i = precision + 4;

    ret = ajStrSetRes(Pstr, (size_t)i);
    thys = *Pstr;

    sprintf(fmt, "%%.%df", precision);
    thys->Len = sprintf(thys->Ptr, fmt, val);

    return ret ;
}




/* @func ajStrFromDoubleExp ***************************************************
**
** Converts a double precision value into a string. 
**
** The string size is set to be just large enough to hold the value. Uses 
** exponential form.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [double] Double precision value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromDoubleExp(AjPStr* Pstr, double val, ajint precision)
{
    AjBool ret = ajFalse;
    long long i;
    char fmt[12];
    AjPStr thys;
    long long ival;

    ival = (long long) fabs(val);

    if(ival)
	i = precision + (ajint) log10((double)ival) + 8;
    else
	i = precision + 8;

    ret = ajStrSetRes(Pstr, (size_t) i);
    thys = *Pstr;

    sprintf(fmt, "%%.%de", precision);
    thys->Len = sprintf(thys->Ptr, fmt, val);

    return ret ;
}


/* @obsolete ajStrFromDoubleE
** @rename ajStrFromDoubleExp
*/

AjBool __deprecated ajStrFromDoubleE(AjPStr* Pstr, double val, ajint precision)
{
    return ajStrFromDoubleExp(Pstr, val, precision);
}

/* @func ajStrFromFloat *******************************************************
**
** Converts a floating point value into a string. 
**
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [float] Floating point value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromFloat(AjPStr* Pstr, float val, ajint precision)
{
    AjBool ret = ajFalse;
    ajint i;
    char fmt[12];
    AjPStr thys;
    ajint ival;

    ival = abs((ajint) val);

    if(ival)
	i = precision + (ajint) log10((double)ival) + 4;
    else
	i = precision + 4;

    ret = ajStrSetRes(Pstr, i);
    thys = *Pstr;

    sprintf(fmt, "%%.%df", precision);
    thys->Len = sprintf(thys->Ptr, fmt, val);

    return ret;
}




/* @func ajStrFromInt *********************************************************
**
** Converts an integer value into a string. 
** 
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [ajint] Integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromInt(AjPStr* Pstr, ajint val)
{
    AjBool ret = ajFalse;
    ajint i;
    AjPStr thys;

    if(val)
	i = (ajint) log10((double)abs(val)) + 2;
    else
	i = 2;

    if(val < 0)
	i++;

    ret = ajStrSetRes(Pstr, i);
    thys = *Pstr;

    thys->Len = sprintf(thys->Ptr, "%d", val);

    return ret ;
}




/* @func ajStrFromLong ********************************************************
**
** Converts an ajlong integer value into a string.
**
** The string size is set to be just large enough to hold the value.
**
** @param [w] Pstr [AjPStr*] Target string
** @param [r] val [ajlong] Long integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromLong(AjPStr* Pstr, ajlong val)
{
    AjBool ret = ajFalse;
    ajlong i;
    AjPStr thys;

    if(val)
	i = (ajlong) log10((double)abs(val)) + 2;
    else
	i = 2;

    if(val < 0)
	i++;

    ret = ajStrSetRes(Pstr, i);
    thys = *Pstr;

    thys->Len = sprintf(thys->Ptr, "%ld", (long)val);

    return ret;
}




/* @section formatting
**
** Functions for formatting strings.
**
** @fdata      [AjPStr]
** @fnote     Same namrule as "String formatting functions: C-type (char*)
**            strings".
** @nam3rule  Fmt           Change the format of a string.
** @nam4rule  FmtBlock    Format in blocks
** @nam4rule  FmtLower    Convert to lower case.
** @nam5rule  FmtLowerSub Convert sub-string.
** @nam4rule  FmtQuote    Enclose in double quotes
** @nam4rule  FmtTitle    Convert first character of string to uppercase. 
** @nam4rule  FmtUpper    Convert to upper case.
** @nam5rule  FmtUpperSub    Substring only
** @nam4rule  FmtWrap    Wrap with newlines
** @nam5rule  FmtWrapLeft    Wrap with newlines and left margin of spaces
**
** @argrule * Pstr [AjPStr*] String
** @argrule FmtBlock len [ajint] Block length
** @argrule Sub pos1 [ajint] Start position, negative value counts from end
** @argrule Sub pos2 [ajint] End position, negative value counts from end
** @argrule Wrap width [ajint] Line length
** @argrule WrapLeft margin [ajint] Left margin
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/

/* @func ajStrFmtBlock ********************************************************
**
** Splits a string into words (blocks) of a given size by inserting spaces.
**
** Mainly intended for sequence ouput formats
**
** @param [u] Pstr [AjPStr*] String.
** @param [r] len [ajint] Block size
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrFmtBlock(AjPStr* Pstr, ajint len)
{
    ajint i;
    char* cp;
    char* cq;
    AjPStr thys;
    ajint j;

    i = (*Pstr)->Len + ((*Pstr)->Len-1)/len;
    ajStrSetRes(Pstr, i+1);
    thys = *Pstr;

/*
    ajDebug("ajStrBlock len: %d blksize: %d i: %d\n", thys->Len, blksize, i);
*/

    cp = &thys->Ptr[thys->Len];
    cq = &thys->Ptr[i];
    for(j=thys->Len-1; j; j--)
    {
	*(--cq) = *(--cp);
	if(!(j%len))
	    *(--cq) = ' ';
    }
    thys->Ptr[i]='\0';
    thys->Len = i;

    ajStrTrace(thys);

/*
    ajDebug("result '%S'\n", thys);
*/

    return ajTrue;
}


/* @obsolete ajStrBlock
** @rename ajStrFmtBlock
*/

AjBool __deprecated ajStrBlock(AjPStr* pthis, ajint blksize)
{
    return ajStrFmtBlock(pthis, blksize);
}

/* @func ajStrFmtLower ********************************************************
**
** Converts a string to lower case. 
**
** If the string has multiple references, a new string is made first.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtLower(AjPStr* Pstr)
{
    AjPStr thys;

    thys  = ajStrGetuniqueStr(Pstr);

    ajCharFmtLower(thys->Ptr);

    return ajTrue;
}


/* @obsolete ajStrToLower
** @rename ajStrFmtLower
*/
AjBool __deprecated ajStrToLower(AjPStr* pthis)
{
    return ajStrFmtLower(pthis);
}

/* @func ajStrFmtLowerSub *****************************************************
**
** Converts a substring of a string to lower case.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] pos1 [ajint] start position for conversion
** @param [r] pos2 [ajint] end position for conversion
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtLowerSub(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjPStr thys;
    ajint ibegin;
    ajint iend;
    ajint i;

    thys  = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2);
    for (i=ibegin; i<=iend;i++)
	thys->Ptr[i] = (char)tolower((ajint) thys->Ptr[i]);

    return ajTrue;
}




/* @obsolete ajStrToLowerII
** @rename ajStrFmtLowerSub
*/
AjBool __deprecated ajStrToLowerII(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrFmtLowerSub(pthis, begin, end);
}

/* @func ajStrFmtQuote ********************************************************
**
** Ensures a string is quoted with double quotes.
**
** @param [u] Pstr [AjPStr *] string
**
** @return [AjBool] True if string is not empty.
**                  As it is at least quoted, it will always return true.
** @@
******************************************************************************/

AjBool ajStrFmtQuote(AjPStr* Pstr)
{
    if(ajStrGetCharFirst(*Pstr) != '"')
	ajStrInsertC(Pstr, 0, "\"");

    /* careful: could be an empty string that is now '"' */
    /* this is the only way to have a length of 1 */

    if((*Pstr)->Len == 1 || ajStrGetCharLast(*Pstr) != '"')
	ajStrAppendK(Pstr, '"');

    if(!(*Pstr)->Len) return ajFalse;

    return ajTrue;
}




/* @obsolete ajStrQuote
** @rename ajStrFmtQuote
*/

void __deprecated ajStrQuote(AjPStr* s)
{
    ajStrFmtQuote(s);
}


/* @func ajStrFmtTitle ********************************************************
**
** Converts the first character of a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtTitle(AjPStr* Pstr)
{
    AjPStr thys;
    char* cp;

    ajStrFmtLower(Pstr);

    thys  = ajStrGetuniqueStr(Pstr);

    cp = thys->Ptr;

    while(*cp) {
	if(isalpha(*cp))
	{
	    *cp = (char) toupper((int)*cp);
	    return ajTrue;
	}
	cp++;
    }

    return ajFalse;
}




/* @obsolete ajStrToTitle
** @rename ajStrFmtTitle
*/

AjBool __deprecated ajStrToTitle(AjPStr* pthis)
{
    return ajStrFmtTitle(pthis);
}



/* @func ajStrFmtUpper ********************************************************
**
** Converts a string to upper case. 
**
** If the string has multiple references, a new string is made first.
**
** @param [u] Pstr [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtUpper(AjPStr* Pstr)
{
    AjPStr thys;

    thys  = ajStrGetuniqueStr(Pstr);

    ajCharFmtUpper(thys->Ptr);

    return ajTrue;
}



/* @obsolete ajStrToUpper
** @rename ajStrFmtUpper
*/

AjBool __deprecated ajStrToUpper(AjPStr* pthis)
{
    return ajStrFmtUpper(pthis);
}

/* @func ajStrFmtUpperSub *****************************************************
**
** Converts a substring of a string to upper case.
**
** @param [u] Pstr [AjPStr*] String
** @param [r] pos1 [ajint] start position for conversion
** @param [r] pos2 [ajint] end position for conversion
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFmtUpperSub(AjPStr* Pstr, ajint pos1, ajint pos2)
{
    AjPStr thys;
    ajint ibegin;
    ajint iend;
    ajint i;

    thys  = ajStrGetuniqueStr(Pstr);

    ibegin = ajMathPos(thys->Len, pos1);
    iend = ajMathPosI(thys->Len, ibegin, pos2);
    for (i=ibegin; i<=iend;i++)
	thys->Ptr[i] = (char)toupper((ajint) thys->Ptr[i]);

    return ajTrue;
}


/* @obsolete ajStrToUpperII
** @rename ajStrFmtUpperII
*/

AjBool __deprecated ajStrToUpperII(AjPStr* pthis, ajint begin, ajint end)
{
    return ajStrFmtUpperSub(pthis, begin, end);
}

/* @func ajStrFmtWrap *********************************************************
**
** Formats a string so that it wraps when printed.  
**
** Newline characters are inserted, at white space if possible, 
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajint] Line width
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrFmtWrap(AjPStr* Pstr, ajint width )
{
    AjPStr thys;
    char* cp;
    char* cq;
    ajint i;

    if(width > (*Pstr)->Len)		/* already fits on one line */
	return ajTrue;

    thys = ajStrGetuniqueStr(Pstr);

    cq = thys->Ptr;
    for(i=width; i < thys->Len; i+=width)
    {
	cp = &thys->Ptr[i];
	while(cp > cq && !isspace((ajint)*cp))
	    cp--;

	if(cp == cq)
	{
	    ajStrInsertC(Pstr, i, "\n");
	    cp = &thys->Ptr[i+1];
	}
	else
	    *cp = '\n';
	cq = cp;
    }

    return ajTrue;
}



/* @obsolete ajStrWrap
** @rename ajStrFmtWrap
*/

AjBool __deprecated ajStrWrap(AjPStr* Pstr, ajint width )
{
    return ajStrFmtWrap(Pstr, width);
}

/* @func ajStrFmtWrapLeft *****************************************************
**
** Formats a string so that it wraps and has a margin of space characters when
** printed.  
**
** Newline characters are inserted, at white space if possible, 
**
** @param [u] Pstr [AjPStr*] Target string
** @param [r] width [ajint] Line width
** @param [r] margin [ajint] Left margin
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrFmtWrapLeft(AjPStr* Pstr, ajint width, ajint margin)
{
    AjPStr newstr = NULL;
    char* cp;
    ajint len;
    ajint i   = 0;
    ajint j;
    ajint isp = 0;
    
    /* ajDebug("ajStrFmtWrapLeft %d %d\n'%S'\n", width, margin, *Pstr); */

    len = 1 + (*Pstr)->Len + (margin + 1) * (*Pstr)->Len / width;
    ajStrAssignS(&newstr, *Pstr);
    ajStrAssignResC(Pstr, len, "");
    
    for(cp = newstr->Ptr; *cp; cp++)
    {
	switch(*cp)
	{
	case '\n':
	    ajStrAppendK(Pstr, '\n');
	    for(j=0; j<margin; j++)
		ajStrAppendK(Pstr, ' ');
	    i = 0;
	    isp = 0;
	    break;
	case ' ':
	case '\t':
	    isp = (*Pstr)->Len;
	    /* ajDebug("can split at %d\n", isp); */
	default:
	    if(++i >= width)
	    {	/* too wide, time to split */
		/* ajDebug("split at i: %d isp: %d\n'%S'\n",
		   i, isp, *Pstr); */
		if(isp)
		{
		    if(isp == (*Pstr)->Len)
			ajStrAppendK(Pstr, '\n');
		    else
			(*Pstr)->Ptr[isp] = '\n';
		}
		else
		{
		    ajStrAppendK(Pstr, *cp); /* keep going */
		    break;
		}

		for(j=0; j<margin; j++)
		{	  /* follow newline with left margin spaces */
		    isp++;
		    ajStrInsertC(Pstr, isp, " ");
		}
		i = (*Pstr)->Len - isp;
		isp = 0;

		if(!isspace((ajint)*cp))
		    ajStrAppendK(Pstr, *cp);
	    }
	    else
		ajStrAppendK(Pstr, *cp);

	    break;
	}
    }
    
    ajStrDel(&newstr);

    return ajTrue;
}


/* @obsolete ajStrWrapLeft
** @rename ajStrFmtWrapLeft
*/

AjBool __deprecated ajStrWrapLeft(AjPStr* pthis, ajint width, ajint left)
{
    return ajStrFmtWrapLeft(pthis, width, left);
}

/* @section comparison
**
** Functions for comparing strings 
**
** @fdata      [AjPStr]
**
** @nam3rule  Match          Compare two complete strings.
** @nam4rule  MatchCase      Case-insensitive comparison.
** @nam4rule  MatchWild      Comparison using wildcard characters.
** @nam5rule  MatchWildWord  Case-insensitive wildcard comparison of 
**                           first words within two strings.
** @nam4rule  MatchWord      Comparison using whole words.
** @nam5rule  MatchWordOne   Comparison using whole words matching any one.
** @nam5rule  MatchWordAll   Comparison using whole words matching every one.

** @nam3rule  Prefix         Compare start of string to given prefix.
** @nam4rule  PrefixCase     Case-insensitive comparison.
** @nam3rule  Suffix         Compare end of string to given suffix.
** @nam4rule  SuffixCase     Case-insensitive comparison.
**
** @argrule * str [const AjPStr] String
** @argrule C txt2 [const char*] String
** @argrule S str2 [const AjPStr] String
**
** @valrule * [AjBool] True on success
**
** @fcategory use
*/



/* @func ajStrMatchC **********************************************************
**
** Simple test for matching a string and a text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two complete strings are the same
** @@
******************************************************************************/

AjBool ajStrMatchC(const AjPStr str, const char* txt2)
{
    if(!str || !txt2)
	return ajFalse;

    if(!strcmp(str->Ptr, txt2))
	return ajTrue;

    return ajFalse;
}




/* @func ajStrMatchS **********************************************************
**
** Simple test for matching two strings. 
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Second String
** @return [AjBool] ajTrue if two complete strings are the same
** @@
******************************************************************************/

AjBool ajStrMatchS(const AjPStr str, const AjPStr str2)
{
    if(!str || !str2)
	return ajFalse;

    if(!strcmp(str->Ptr, str2->Ptr))
	return ajTrue;

    return ajFalse;
}


/* @obsolete ajStrMatch
** @rename ajStrMatchS
*/
AjBool __deprecated ajStrMatch(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchS(str, str2);
}

/* @func ajStrMatchCaseC ******************************************************
**
** Simple case-insensitive test for matching a string and a text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCaseC(const AjPStr str, const char* txt2)
{
    if(!str || !txt2)
	return ajFalse;

    return ajCharMatchCaseC(str->Ptr, txt2);
}




/* @func ajStrMatchCaseS ******************************************************
**
** Simple case-insensitive test for matching two strings.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Second String
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCaseS(const AjPStr str, const AjPStr str2)
{
    if(!str || !str2)
	return ajFalse;

    return ajCharMatchCaseC(str->Ptr, str2->Ptr);
}

/* @obsolete ajStrMatchCase
** @rename ajStrMatchCaseS
*/

AjBool __deprecated ajStrMatchCase(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchCaseS(str, str2);
}


/* @func ajStrMatchWildC ******************************************************
**
** Simple case-insensitive test for matching a string and a text string using
** wildcard characters.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildC(str->Ptr, txt2);
}




/* @func ajStrMatchWildS ******************************************************
**
** Simple case-insensitive test for matching two strings using wildcard 
** characters.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [AjBool] ajTrue if two strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildC(str->Ptr, str2->Ptr);
}


/* @obsolete ajStrMatchWild
** @rename ajStrMatchWildS
*/

AjBool __deprecated ajStrMatchWild(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchWildS(str, str2);
}




/* @func ajStrMatchWildWordC **************************************************
**
** Case-insensitive test for matching a text string "word" against a string
** using wildcard characters. 
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Text
** @return [AjBool] ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWildWordC(const AjPStr str, const char* txt2)
{
    return ajCharMatchWildWordC(str->Ptr, txt2);
}




/* @func ajStrMatchWildWordS **************************************************
**
** Case-insensitive test for matching a string "word" against a string
** using wildcard characters. 
**
** 'Word' is defined as starting and ending with an alphanumeric character
** (A-Z a-z 0-9) with no white space.
**
** The query text can use '*' or '?' as a wildcard.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWildWordS(const AjPStr str, const AjPStr str2)
{
    return ajCharMatchWildWordC(str->Ptr, str2->Ptr);
}



/* @obsolete ajStrMatchWord
** @rename ajStrMatchWildWordS
*/

AjBool __deprecated ajStrMatchWord(const AjPStr str, const AjPStr str2)
{
    return ajStrMatchWildWordS(str, str2);
}




/* @func ajStrMatchWordAllS **************************************************
**
** Test for matching all words within a string.
** Matches can be to parts of words in the original string.
**
** 'Word' is defined as no white space.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWordAllS(const AjPStr str, const AjPStr str2)
{
    AjBool ret = ajTrue;
    const AjPStr teststr = NULL;

    teststr = ajStrParseWhite(str2);

    if(!teststr)
	return ajFalse;

    while (teststr)
    {
	if(ajStrFindS(str, teststr) == -1)
	    ret = ajFalse;
	teststr = ajStrParseWhite(NULL);
    }
    return ret;
}



/* @func ajStrMatchWordOneS **************************************************
**
** Test for matching a word within a string.
** Matches can be to parts of words in the original string.
**
** 'Word' is defined as no white space.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Text
** @return [AjBool]  ajTrue if found
** @@
******************************************************************************/

AjBool ajStrMatchWordOneS(const AjPStr str, const AjPStr str2)
{
    AjBool ret = ajFalse;
    const AjPStr teststr = NULL;

    teststr = ajStrParseWhite(str2);

    if(!teststr)
	return ajFalse;

    while (teststr)
    {
	if(ajStrFindS(str, teststr) != -1)
	    ret = ajTrue;
	teststr = ajStrParseWhite(NULL);
    }
    return ret;
}



/* @func ajStrPrefixC *********************************************************
**
** Test for matching the start of a string against a given prefix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixC(const AjPStr str, const char* txt2)
{
    ajint ilen;

    if(!str)
	return ajFalse;

    ilen = strlen(txt2);

    if(!ilen)				/* no prefix */
	return ajFalse;

    if(ilen > ajStrGetLen(str))		/* pref longer */
	return ajFalse;

    if(strncmp(str->Ptr, txt2, ilen))	/* +1 or -1 for a failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajStrPrefixS *********************************************************
**
** Test for matching the start of a string against a given prefix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixS(const AjPStr str, const AjPStr str2)
{
    if(!str)
	return ajFalse;

    if(!str2)
	return ajFalse;

    if(!str2->Len)			/* no prefix */
	return ajFalse;

    if(str2->Len > ajStrGetLen(str))	/* pref longer */
	return ajFalse;

    if(strncmp(str->Ptr, str2->Ptr, str2->Len)) /* +1 or -1 for a
						     failed match */
	return ajFalse;

    return ajTrue;
}

/* @obsolete ajStrPrefix
** @rename ajStrPrefixS
*/

AjBool __deprecated ajStrPrefix(const AjPStr str, const AjPStr str2)
{
    return ajStrPrefixS(str, str2);
}



/* @func ajStrPrefixCaseC *****************************************************
**
** Case-insensitive test for matching the start of a string against a 
** given prefix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseC(const AjPStr str, const char* txt2)
{
    return ajCharPrefixCaseC(str->Ptr, txt2);
}




/* @func ajStrPrefixCaseS *****************************************************
**
** Case-insensitive test for matching the start of a string against a 
** given prefix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharPrefixCaseC(str->Ptr, str2->Ptr);
}


/* @obsolete ajStrPrefixCase
** @rename ajStrPrefixCaseS
*/

AjBool __deprecated ajStrPrefixCase(const AjPStr str, const AjPStr str2)
{
    return ajStrPrefixCaseS(str,str2);
}




/* @func ajStrSuffixC *********************************************************
**
** Test for matching the end of a string against a given suffix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixC(const AjPStr str, const char* txt2)
{
    ajint ilen;
    ajint istart;

    if(!str)
	return ajFalse;

    ilen   = strlen(txt2);
    istart = str->Len - ilen;

    if(ilen > ajStrGetLen(str))		/* suff longer */
	return ajFalse;

    if(strncmp(&str->Ptr[istart], txt2, ilen)) /* +1 or -1 for a
						    failed match */
	return ajFalse;

    return ajTrue;
}




/* @func ajStrSuffixS *********************************************************
**
** Test for matching the end of a string against a given suffix string.
** 
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Suffix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixS(const AjPStr str, const AjPStr str2)
{
    ajint ilen;
    ajint istart;

    if(!str)
	return ajFalse;

    ilen   = ajStrGetLen(str2);
    istart = str->Len - ilen;

    if(ilen > ajStrGetLen(str))		/* suffix longer */
	return ajFalse;

    if(strncmp(&str->Ptr[istart], str2->Ptr, ilen)) /* +1 or -1 for a
						     failed match */
	return ajFalse;

    return ajTrue;
}



/* @obsolete ajStrSuffix
** @rename ajStrSuffixS
*/

AjBool __deprecated ajStrSuffix(const AjPStr str, const AjPStr str2)
{
    return ajStrSuffixS(str, str2);
}

/* @func ajStrSuffixCaseC *****************************************************
**
** Case-insensitive test for matching the end of a string against a 
** given suffix text string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Prefix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixCaseC(const AjPStr str, const char* txt2)
{
    return ajCharSuffixCaseC(str->Ptr, txt2);
}




/* @func ajStrSuffixCaseS *****************************************************
**
** Case-insensitive test for matching the end of a string against a 
** given suffix string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixCaseS(const AjPStr str, const AjPStr str2)
{
    return ajCharSuffixCaseC(str->Ptr, str2->Ptr);
}


/* @section comparison (sorting)
**
** Functions for sorting strings.
**
** @fdata      [AjPStr]
** @fnote     namrule same as "String comparison (sorting) functions (C-type
**            char* strings)".
** @nam3rule  Cmp           Compare two complete strings & return sort order.
** @nam4rule  CmpCase     Case-insensitive comparison.
** @nam4rule  CmpWild     Comparison using wildcard characters.
** @nam3rule  Vcmp     Compare string using void arguments.
**
** @argrule Cmp str [const AjPStr] String to compare
** @argrule C txt2 [const char*] String to compare
** @argrule S str2 [const AjPStr] String to compare
** @argrule Len len [size_t] Number of characters to compare
** @argrule Vcmp str [const void*] String
** @argrule Vcmp str2 [const void*] String to compare
**
** @valrule * [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
**
** @fcategory use
*/



/* @func ajStrCmpC ************************************************************
**
** Finds the sort order of a string and a text string.
**
** @param [r] str [const AjPStr] String object
** @param [r] txt2 [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpC(const AjPStr str, const char* txt2)
{
    return strcmp(str->Ptr, txt2);
}



/* @func ajStrCmpLenC *********************************************************
**
** Finds the sort order of a string and a text string comparing the first 
** n characters only.
**
** @param [r] str [const AjPStr] String object
** @param [r] txt2 [const char*] Text string
** @param [r] len [size_t] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpLenC(const AjPStr str, const char* txt2, size_t len)
{
    return strncmp(str->Ptr, txt2, len);
}




/* @obsolete ajStrNCmpC
** @rename ajStrCmpLenC
*/

int __deprecated ajStrNCmpC(const AjPStr str, const char* txt, ajint len)
{
    return ajStrCmpLenC(str, txt, len);
}


/* @func ajStrCmpS ************************************************************
**
** Finds the sort order of two strings. 
**
** @param [r] str [const AjPStr] First string
** @param [r] str2 [const AjPStr] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpS(const AjPStr str, const AjPStr str2)
{
    return strcmp(str->Ptr, str2->Ptr);
}





/* @obsolete ajStrCmpO
** @rename ajStrCmpS
*/

int __deprecated ajStrCmpO(const AjPStr thys, const AjPStr anoth)
{
    return ajStrCmpS(thys, anoth);
}

/* @func ajStrCmpCaseS ********************************************************
**
** Finds the sort order of two strings using a case-insensitive comparison.
**
** @param [r] str [const AjPStr] text string
** @param [r] str2 [const AjPStr] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpCaseS(const AjPStr str, const AjPStr str2)
{
    const char* cp;
    const char* cq;

    for(cp = str->Ptr, cq = str2->Ptr; *cp && *cq; cp++, cq++)
	if(toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if(toupper((ajint) *cp) > toupper((ajint) *cq))
		return 1;
	    else
		return -1;
	}

    if(*cp)
	return 1;

    if(*cq)
	return -1;

    return 0;
}

/* @obsolete ajStrCmpCase
** @rename ajStrCmpCaseS
*/

int __deprecated ajStrCmpCase(const AjPStr str, const AjPStr str2)
{
    return ajStrCmpCaseS(str, str2);
}

/* @func ajStrCmpLenS *********************************************************
**
** Finds the sort order of two strings comparing the first
** n characters only.

** Compares the first n characters of two strings
**
** @param [r] str [const AjPStr] String object
** @param [r] str2 [const AjPStr] Second string object
** @param [r] len [size_t] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpLenS(const AjPStr str, const AjPStr str2, size_t len)
{
    return strncmp(str->Ptr, str2->Ptr, len);
}


/* @obsolete ajStrNCmpO
** @rename ajStrCmpLenS
*/

int __deprecated ajStrNCmpO(const AjPStr str, const AjPStr str2, ajint len)
{
    return ajStrCmpLenS(str, str2, len);
}


/* @func ajStrCmpWildC ********************************************************
**
** Finds the sort-order (case insensitive) of string and a text string using 
** wildcard characters. 
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] Wildcard text
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpWildC(const AjPStr str, const char* txt2)
{
    return ajCharCmpWild(str->Ptr, txt2);
}




/* @func ajStrCmpWildS ********************************************************
**
** Finds the sort-order (case insensitive) of two strings using wildcard 
** characters. 
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] Wildcard string
** @return [int]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpWildS(const AjPStr str, const AjPStr str2)
{
    return ajCharCmpWild(str->Ptr, str2->Ptr);
}

/* @obsolete ajStrCmpWild
** @rename ajStrCmpWildS
*/


int __deprecated ajStrCmpWild(const AjPStr str, const AjPStr str2)
{
    return ajStrCmpWildS(str, str2);
}



/* @func ajStrVcmp *********************************************************
**
** Finds the sort order of two strings cast as void.  
**
** For use with {ajListSort}, {ajListUnique} and other functions which 
** require a general function with void arguments.
**
** @param [r] str [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrVcmp(const void* str, const void* str2)
{
    return strcmp((*(AjPStr*)str)->Ptr, (*(AjPStr*)str2)->Ptr);
}


/* @obsolete ajStrCmp
** @rename ajStrVcmp
*/

int __deprecated ajStrCmp(const void* str, const void* str2)
{
    return ajStrVcmp(str, str2);
}



/* @section comparison (search) functions
**
** Functions for finding substrings or characters in strings.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule  Find          Locate first occurence of a string
**                          within another string. 
** @nam4rule  FindAny       Any in a set of characters (FindSet?)
** @nam4rule  FindCase      Case insensitive
** @nam3rule  Findlast      Locate last occurence of a string
**
** @argrule * str [const AjPStr] String
** @argrule C txt2 [const char*] Text to find
** @argrule K chr [char] Character
** @argrule S str2 [const AjPStr] Text to find
**
** @valrule * [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
**
** @fcategory use
*/



/* @func ajStrFindC ***********************************************************
**
** Finds the first occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindC(const AjPStr str, const char* txt2)
{
    const char* cp;

    cp = strstr(str->Ptr, txt2);
    if(!cp)
	return -1;

    return(cp - str->Ptr);
}




/* @func ajStrFindS ***********************************************************
**
** Finds the first occurrence in a string of a second string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindS(const AjPStr str, const AjPStr str2)
{
    const char* cp;

    cp = strstr(str->Ptr, str2->Ptr);
    if(!cp)
	return -1;

    return (cp - str->Ptr);
}



/* @obsolete ajStrFind
** @rename ajStrFindS
*/

ajint __deprecated ajStrFind(const AjPStr str, const AjPStr str2)
{
    return ajStrFindS(str, str2);
}

/* @func ajStrFindAnyC ********************************************************
**
** Finds the first occurrence in a string of any character in a second 
** (text) string.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindAnyC(const AjPStr str, const char* txt2)
{
    ajint i;

    i = strcspn(str->Ptr, txt2);
    if(i == str->Len)
	return -1;
    return i;
}




/* @func ajStrFindAnyK ********************************************************
**
** Finds the first occurrence in a string of a specified character.
**
** @param [r] str [const AjPStr] String
** @param [r] chr [char] character to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindAnyK(const AjPStr str, char chr)
{
    const char* cp;

    cp = strchr(str->Ptr, (ajint) chr);
    if(!cp)
	return -1;

    return(cp - str->Ptr);
}



/* @obsolete ajStrFindK
** @rename ajStrFindAnyK
*/

ajint __deprecated ajStrFindK(const AjPStr thys, const char chr)
{
    return ajStrFindAnyK(thys, chr);
}

/* @func ajStrFindAnyS ********************************************************
**
** Finds the first occurrence in a string of any character in a second 
** string.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                Or -1 for text not found.
** @@
******************************************************************************/

ajint ajStrFindAnyS(const AjPStr str, const AjPStr str2)
{
    ajint i;

    i = strcspn(str->Ptr, str2->Ptr);
    if(i == str->Len)
	return -1;
    return i;
}



/* @func ajStrFindCaseC *******************************************************
**
** Finds the first occurrence in a string of a second (text) string using a
** case-insensitive search.
**
** @param [r] str [const AjPStr] String
** @param [r] txt2 [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindCaseC(const AjPStr str, const char *txt2)
{
    AjPStr t1;
    AjPStr t2;
    ajint v;

    t1 = ajStrNewC(str->Ptr);
    t2 = ajStrNewC(txt2);
    ajStrFmtUpper(&t1);
    ajStrFmtUpper(&t2);

    v = ajStrFindS(t1,t2);
    ajStrDel(&t1);
    ajStrDel(&t2);

    return v;
}




/* @func ajStrFindCaseS *******************************************************
**
** Finds the first occurrence in a string of a second string using a 
** case-insensitive search.
**
** @param [r] str [const AjPStr] String
** @param [r] str2 [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
**                -1 if not found.
** @@
******************************************************************************/

ajint ajStrFindCaseS(const AjPStr str, const AjPStr str2)
{
    return ajStrFindCaseC(str,str2->Ptr);
}



/* @obsolete ajStrFindCase
** @rename ajStrFindCaseS
*/

ajint __deprecated ajStrFindCase(const AjPStr str, const AjPStr str2)
{
    return ajStrFindCaseS(str,str2);
}


/* @func ajStrFindlastC *******************************************************
**
** Finds the last occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String to search
** @param [r] txt2 [const char*] text to look for
** @return [ajint] Position of the text string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindlastC(const AjPStr str, const char* txt2)
{
    ajint i = 0;
    ajint j = 0;
    ajint len = 0;
    const char* ptr1 = 0;
    const char* ptr2 = 0;
    ajint found = ajTrue;

    len = strlen(txt2);

    for(i=str->Len-len;i>=0;i--)
    {
	ptr1 = &str->Ptr[i];
	ptr2 = txt2;
	found = ajTrue;
	for(j=0;j<len;j++)
	{
	    if(*ptr1 != *ptr2)
	    {
		found = ajFalse;
		break;
	    }
	    ptr2++;
	    ptr1++;
	}

	if(found)
	    return i;
    }

    return -1;
}


/* @obsolete ajStrRFindC
** @rename ajStrFindlastC
*/

ajint __deprecated ajStrRFindC(const AjPStr thys, const char* text)
{
    return ajStrFindlastC(thys, text);
}

/* @func ajStrFindlastS *******************************************************
**
** Finds the last occurrence in a string of a second (text) string.
**
** @param [r] str [const AjPStr] String to search
** @param [r] str2 [const AjPStr] text to look for
** @return [ajint] Position of the text string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindlastS(const AjPStr str, const AjPStr str2)
{
    ajint i = 0;
    ajint j = 0;
    const char* ptr1 = 0;
    const char* ptr2 = 0;
    ajint found = ajTrue;

    for(i=str->Len-str2->Len;i>=0;i--)
    {
	ptr1 = &str->Ptr[i];
	ptr2 = &str2->Ptr[i];
	found = ajTrue;
	for(j=0;j<str2->Len;j++)
	{
	    if(*ptr1 != *ptr2)
	    {
		found = ajFalse;
		break;
	    }
	    ptr2++;
	    ptr1++;
	}

	if(found)
	    return i;
    }

    return -1;
}


/* @section parsing functions
**
** Functions for parsing tokens from strings.
**
** @fdata      [AjPStr]
** @fnote     None.
**
** @nam3rule Extract        Return token(s) from a string and return
**                          the remainder
** @nam4rule ExtractFirst   Remove first word from a string, no leading spaces
** @nam4rule ExtractWord    Remove first word from a string, skipping spaces
** @nam3rule Parse          Parse tokens using strtok
** @nam4rule ParseCount     Count tokens using string token functions
** @nam5rule ParseCountMulti Multiple (reentrant) version (obsolete?)
** @nam4rule ParseSplit     Parse into array of strings
** @nam4rule ParseWhite     Count tokens using string token functions
**
** @argrule * str [const AjPStr] String
** @argrule C txtdelim [const char*] Text to find
** @argrule K chr [char] Character
** @argrule S strdelim [const AjPStr] Text to find
** @argrule Split PPstr [AjPStr**] String array of results
** @argrule Extract Prest [AjPStr*] Remainder of string
** @argrule First Pword [AjPStr*] First word of string
** @argrule Word Pword [AjPStr*] First word of string
**
** @valrule * [const AjPStr] Latest string parsed.
** @valrule *Count [ajint] Number of string matches.
** @valrule *Split [ajint] Number of string matches.
** @valrule *Extract [AjBool] True if a match was found
**
** @fcategory use
*/



/* @func ajStrExtractFirst ****************************************************
**
** Returns a word from the start of a string, and the remainder of the string
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [w] Prest [AjPStr*] Remainder of string
** @param [w] Pword [AjPStr*] First word of string
** @return [AjBool] True if parsing succeeded
** @@
******************************************************************************/

AjBool ajStrExtractFirst(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    ajint i=0;
    ajint j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);
    if(isspace(*cp++)) 	return ajFalse;

    while(*cp && !isspace(*cp))		/* look for end of first word */
    {
	cp++;
	i++;
    }

    j = i+1;
    while(*cp && isspace(*cp))
    {
	cp++;
	j++;
    }
    if(!*cp) return ajFalse;		/* nothing after whitespace */

    ajStrAssignSubS(Pword, str, 0, i);
    ajStrAssignSubS(Prest, str, j, str->Len);

    ajDebug("ajStrExtractFirst i:%d j:%d len:%d word '%S'\n",
	    i, j, str->Len, *Pword);
    return ajTrue;
}



/* @func ajStrExtractWord *****************************************************
**
** Returns a word from the start of a string, and the remainder of the string.
** Leading spaces are skipped.
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [w] Prest [AjPStr*] Remainder of string
** @param [w] Pword [AjPStr*] First word of string
** @return [AjBool] True if parsing succeeded
** @@
******************************************************************************/

AjBool ajStrExtractWord(const AjPStr str, AjPStr* Prest, AjPStr* Pword)
{
    ajint i=0;
    ajint istart=0;
    ajint j=0;
    const char* cp;

    if(!str)
	return ajFalse;

    if(!str->Len)
	return ajFalse;

    cp = ajStrGetPtr(str);
    while(isspace(*cp))
    {
	cp++;
	istart++;
    }

    if(!*cp)
	return ajFalse;

    i = istart-1;
    while(*cp && !isspace(*cp))		/* look for end of first word */
    {
	cp++;
	i++;
    }

    j = i+1;
    while(*cp && isspace(*cp))
    {
	cp++;
	j++;
    }
    if(!*cp) return ajFalse;		/* nothing after whitespace */

    ajStrAssignSubS(Pword, str, istart, i);
    ajStrAssignSubS(Prest, str, j, str->Len);

    ajDebug("ajStrExtractFirst i:%d j:%d len:%d word '%S'\n",
	    i, j, str->Len, *Pword);
    return ajTrue;
}



/* @func ajStrParseC **********************************************************
**
** Tokenise a string using a specified set of delimiters and return tokens
** from the string. 
**
** This function uses the C run time library strtok function. This makes it
** necessary to only call it from one string at a time as strtok and this
** function have internal memories of the last position.
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] txtdelim [const char*] Delimiter(s) to be used betwen tokens.
** @return [const AjPStr] Token returned, when all tokens are parsed
**                  a NULL is returned.
** @@
******************************************************************************/

const AjPStr ajStrParseC(const AjPStr str, const char* txtdelim)
{
    static AjOStr strParseStr = { 1, 0, 0, NULL};
    static AjPStr strp = &strParseStr;

    if(!strp->Ptr)
    {
	if(!str)
	{
	    ajWarn("Error in ajStrParseC: NULL argument and not initialised");
	    ajUtilCatch();
	    return NULL;
	}
    }

    if(str)
    {
	if(strParseCp) ajCharDel(&strParseCp);
	strParseCp = ajCharNewC(str->Ptr);
	strp->Ptr = ajSysStrtok(strParseCp, txtdelim);
    }
    else
	strp->Ptr = ajSysStrtok(NULL, txtdelim);

    if(strp->Ptr)
    {
	strp->Len = strlen(strp->Ptr);
	strp->Res = strp->Len + 1;
	return strp;
    }
    else {
	strp->Len=0;
	strp->Res=1;
	ajCharDel(&strParseCp);
    }

    return NULL;
}



/* @obsolete ajStrTokC
** @rename ajStrParseC
*/

AjPStr __deprecated ajStrTokC(const AjPStr thys, const char* delim)
{
    return (AjPStr) ajStrParseC(thys, delim);
}

/* @func ajStrParseCount ******************************************************
**
** Returns the number of tokens in a string.
**
** @param [r] str [const AjPStr] String to examine.
** @return [ajint] The number of tokens
** @@
******************************************************************************/

ajint ajStrParseCount(const AjPStr str)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;

    ajint count;

    count = 0;
    ajStrTokenAssign(&t, str);

    while(ajStrTokenNextParse(&t, &tmp))
	++count;

    ajStrTokenDel(&t);
    ajStrDel(&tmp);

    return count;
}


/* @obsolete ajStrTokenCount
** @rename ajStrParseCountC
*/

ajint __deprecated ajStrTokenCount(const AjPStr line, const char *delim)
{
    return ajStrParseCountC(line, delim);
}

/* @func ajStrParseCountC *****************************************************
**
** Returns the number of tokens in a string using a specified set of 
** delimiters held in a text string.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] txtdelim [const char *] String of delimiter characters.
** @return [ajint] The number of tokens
** @@
******************************************************************************/

ajint ajStrParseCountC(const AjPStr str, const char *txtdelim)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;

    ajint count;

    count = 0;
    ajStrTokenAssignC(&t, str, txtdelim);

    while(ajStrTokenNextParse(&t, &tmp))
	++count;

    ajStrTokenDel(&t);
    ajStrDel(&tmp);

    return count;
}


/* @func ajStrParseCountS *****************************************************
**
** Returns the number of tokens in a string using a specified set of 
** delimiters held in a string.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] strdelim [const AjPStr] String of delimiter characters.
** @return [ajint] The number of tokens
** @@
******************************************************************************/

ajint ajStrParseCountS(const AjPStr str, const AjPStr strdelim)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;

    ajint count;

    count = 0;
    ajStrTokenAssignS(&t, str, strdelim);

    while(ajStrTokenNextParse(&t, &tmp))
	++count;

    ajStrTokenDel(&t);
    ajStrDel(&tmp);

    return count;
}


/* @func ajStrParseCountMultiC ************************************************
**
** Returns the number of tokens in a string. A reentrant function.
**
** Uses {ajSysStrtokR} in place of the C RTL strtko function.
**
** @param [r] str [const AjPStr] String to examine.
** @param [r] txtdelim [const char *] String of delimiter characters.
** @return [ajint] The number of tokens
** @@
******************************************************************************/

ajint ajStrParseCountMultiC(const AjPStr str, const char *txtdelim)
{
    AjPStr buf  = NULL;
    ajint count;
    char  *p;
    char  *save = NULL;

    if(!str)
	return 0;

    buf = ajStrNew();

    p = ajSysStrtokR(ajStrGetPtr(str),txtdelim,&save,&buf);
    if(!p)
    {
	ajStrDel(&buf);
	return 0;
    }

    count = 1;
    while(ajSysStrtokR(NULL,txtdelim,&save,&buf))
	++count;

    ajStrDel(&buf);

    return count;
}


/* @obsolete ajStrTokenCountR
** @rename ajStrParseCountMultiC
*/

ajint __deprecated ajStrTokenCountR(const AjPStr line, const char *delim)
{
    return ajStrParseCountMultiC(line, delim);
}

/* @func ajStrParseSplit ******************************************************
**
** Splits a newline-separated multi-line string into an array of strings.
**
** @param [r] str [const AjPStr] String
** @param [w] PPstr [AjPStr**] pointer to array of AjPStrs
**
** @return [ajint] Number of array elements created
** @@
******************************************************************************/


ajint ajStrParseSplit(const AjPStr str, AjPStr **PPstr)
{
    ajint c;
    ajint len;
    ajint i;
    ajint n;
    const char *p = NULL;
    const char *q = NULL;

    if(!str->Len)
	return 0;


    p = q = ajStrGetPtr(str);

    len = str->Len;
    for(i=c=n=0;i<len;++i)
	if(*(p++)=='\n')
	    ++c;
    p=q;

    AJCNEW0(*PPstr,c);


    for(i=0;i<c;++i)
    {
	while(*q!='\n')	/* safe - we already counted c as # of newlines */
	    ++q;
	(*PPstr)[n] = ajStrNew();
	ajStrAssignSubC(&(*PPstr)[n++],p,0,q-p);
	p = ++q;
    }

    return c;
}

/* @obsolete ajStrListToArray
** @rename ajStrParseSplit
*/

ajint __deprecated ajStrListToArray(const AjPStr thys, AjPStr **array)
{
    return ajStrParseSplit(thys, array);
}

/* @func ajStrParseWhite ******************************************************
**
** Tokenise a string using whitespace and return tokens from the string. 
**
** @param [r] str [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @return [const AjPStr] Token
** @error NULL if no further token is found.
** @@
******************************************************************************/

const AjPStr ajStrParseWhite(const AjPStr str)
{
    return ajStrParseC(str, " \t\n\r");
}



/* @obsolete ajStrTok
** @rename ajStrParseWhite
*/

AjPStr __deprecated ajStrTok(const AjPStr str)
{
    return (AjPStr) ajStrParseWhite(str);
}

/* @section debug
**
** Functions for reporting of a string object.
**
** @fdata       [AjPStr]
** @nam3rule  Stat        Report string statistics to debug file 
** @nam3rule   Trace        Report string elements to debug file 
** @nam4rule   TraceFull        All characters in detail
** @nam4rule   TraceTitle        With title text
**
** @argrule Stat title [const char*] Report title
** @argrule Trace str [const AjPStr] String
** @argrule TraceTitle title [const char*] Report title
**
** @valrule * [void]
**
** @fcategory misc
*/

/* @func ajStrStat ************************************************************
**
** Prints a summary of string usage with debug calls.
**
** @param [r] title [const char*] Title for this summary
** @return [void]
** @@
******************************************************************************/

void ajStrStat(const char* title)
{
    static ajlong statAlloc     = 0;
    static ajlong statCount     = 0;
    static ajlong statFree      = 0;
    static ajlong statFreeCount = 0;
    static ajlong statTotal     = 0;

    ajDebug("String usage statistics since last call %s:\n", title);
    ajDebug("String usage (bytes): %Ld allocated, %Ld freed\n",
	    strAlloc - statAlloc, strFree - statFree);
    ajDebug("String usage (number): %Ld allocated, %Ld freed, %Ld in use\n",
	    strTotal - statTotal, strFreeCount - statFreeCount,
	    strCount - statCount);

    statAlloc     = strAlloc;
    statCount     = strCount;
    statFree      = strFree;
    statFreeCount = strFreeCount;
    statTotal     = strTotal;

    return;
}




/* @func ajStrTrace ***********************************************************
**
** Checks a string object for consistency and reports its contents.
**
** @param [r] str [const AjPStr] String
** @return [void]
** @@
******************************************************************************/

void ajStrTrace(const AjPStr str)
{
    if(!str)
    {
	ajDebug("String trace NULL\n");
	return;
    }

    if(!ajStrGetValid(str));
	ajDebug("ajStrGetValid problems: see warning messages\n");

    ajDebug("String trace use: %d size: %d len: %d string: ",
	    str->Use, str->Res, str->Len);

    if(str->Ptr)
    {
	if(str->Len <= 20)
	    ajDebug("<%s>\n", str->Ptr);
	else
	    ajDebug("<%10.10s>..<%s>\n",
		    str->Ptr, str->Ptr + str->Len-10);
    }
    else
	ajDebug("<NULL>\n");

    ajDebug("             ptr: %x    charptr: %x\n",
	    str, str->Ptr);

    return;
}




/* @func ajStrTraceFull ******************************************************
**
** Checks a string object for consistency and reports its contents character 
** by character.
**
** @param [r] str [const AjPStr] String
** @return [void]
** @@
******************************************************************************/

void ajStrTraceFull(const AjPStr str)
{
    int i;

    if(!str)
    {
	ajDebug("String tracechars NULL\n");
	return;
    }

    if(!ajStrGetValid(str));
	ajDebug("ajStrGetValid problems: see warning messages\n");

    ajDebug("String tracechars use: %d size: %d len: %d string: ",
	    str->Use, str->Res, str->Len);

    if(str->Len)
    {
        ajDebug("\n");
        for(i=0; i < str->Len; i++)
	    ajDebug(" '%c' %20x [%d]\n",
		     str->Ptr[i], (ajint) str->Ptr[i], i);
    }
    else
	ajDebug("<NULL>\n");

    ajDebug("             ptr: %x    charptr: %x\n",
	    str, str->Ptr);

    return;
}




/* @func ajStrTraceTitle ******************************************************
**
** Checks a string object for consistency and reports its contents using a 
** defined title for the report.
**
** @param [r] str [const AjPStr] String
** @param [r] title [const char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajStrTraceTitle(const AjPStr str, const char* title)
{
    ajDebug("%s\n",title);
    ajStrTrace(str);

    return;
}




/* @section exit
**
** Functions called on exit from the program by ajExit to do
** any necessary cleanup and to report internal statistics to the debug file
**
** @fdata      [AjPStr]
** @fnote     general exit functions, no arguments
**
** @nam3rule Exit Cleanup and report on exit
**
** @valrule * [void]
**
** @fcategory misc
*/

/* @func ajStrExit ************************************************************
**
** Prints a summary of string usage with debug calls.
**
** @return [void]
** @@
******************************************************************************/

void ajStrExit(void)
{
    ajDebug("String usage (bytes): %Ld allocated, %Ld freed, %Ld in use\n",
	    strAlloc, strFree,
	    (strAlloc - strFree));
    ajDebug("String usage (number): %Ld allocated, %Ld freed %Ld in use\n",
	    strTotal, strFreeCount, strCount);

    ajCharDel(&strParseCp);
    return;
}




/* @obsolete ajStrFill
** @replace ajStrAppendCountK (1,2,3/1,3,*)
*/

void __deprecated ajStrFill(AjPStr* pthys, ajint len, char fill)
{
    ajint icount;
    icount = len - (*pthys)->Len;
    ajStrAppendCountK(pthys, fill, icount);
    return;
}




/* @obsolete ajStrPos
** @replace ajMathPos (1,2/'ajStrGetLen[1]',2)
*/

ajint __deprecated ajStrPos(const AjPStr thys, ajint ipos)
{
    return ajMathPos(thys->Len, ipos);
}




/* @obsolete ajStrPosI
** @replace ajMathPosI (1,2,3/'ajStrGetLen[1]',2,3)
*/

ajint __deprecated ajStrPosI(const AjPStr thys, ajint imin, ajint ipos)
{
    return ajMathPosI(thys->Len, imin, ipos);
}




/* @obsolete ajStrPosII
** @rename ajMathPosI
*/

ajint __deprecated ajStrPosII(ajint ilen, ajint imin, ajint ipos)
{
    return ajMathPosI(ilen, imin, ipos);
}




/* @obsolete ajCharPos
** @replace ajMathPos (1,2/'strlen[1]',2)
*/

ajint __deprecated ajCharPos(const char* thys, ajint ipos)
{
    return ajMathPos(strlen(thys), ipos);
}



/* @datasection [AjIStr] String iterator *************************************
**
** String iterators point to successive characters in a string
**
** @nam2rule Str String
** @nam3rule Iter Iterator
**
******************************************************************************/

/* @section constructors
**
** @fdata [AjIStr]
**
** @nam4rule New      String iterator constructor.
** @nam5rule NewBack  String iterator reverse direction constructor.
**
** @argrule New str [const AjPStr] Origiial string
**
** @valrule * [AjIStr] String iterator
**
** @fcategory new
*/



/* @func ajStrIterNew *********************************************************
**
** String iterator constructor which allocates memory for a string iterator, 
** used to iterate over the characters in a string.
**
** @param [r] str [const AjPStr] Original string
**
** @return [AjIStr] String Iterator
** @@
******************************************************************************/

AjIStr ajStrIterNew(const AjPStr str)
{
    AjIStr iter;

    if(!str)
	ajFatal("ajStrIterNew source string NULL");

    AJNEW0(iter);
    iter->Start = iter->Ptr = str->Ptr;
    iter->End = iter->Start + ajStrGetLen(str) - 1;

    return iter;
}


/* @obsolete ajStrIter
** @rename ajStrIterNew
*/

AjIStr __deprecated ajStrIter(const AjPStr str)
{
    return ajStrIterNew(str);
}

/* @func ajStrIterNewBack *****************************************************
**
** String iterator constructor which allocates memory for a string iterator, 
** used to iterate over the characters in a string, from end to start.
**
** @param [r] str [const AjPStr] Original string
** @return [AjIStr] String Iterator
** @@
******************************************************************************/

AjIStr ajStrIterNewBack(const AjPStr str)
{
    AjIStr iter;

    if(!str)
	ajFatal("ajStrIterNewBack source string NULL");

    AJNEW0(iter);
    iter->Start = str->Ptr;
    iter->End = iter->Ptr = iter->Start + ajStrGetLen(str) - 1;

    return iter;
}



/* @obsolete ajStrIterBack
** @rename ajStrIterNewBack
*/

AjIStr __deprecated ajStrIterBack(const AjPStr str)
{
    return ajStrIterNewBack(str);
}

/* @section destructors
** @fdata [AjIStr]
**
** @nam4rule Del Destructor
**
** @argrule Del iter [AjIStr*] String iterator
**
** @valrule * [void]
**
** @fcategory delete
*/



/* @func ajStrIterDel ********************************************************
**
** String iterator destructor which frees memory for a string iterator.
**
** @param [d] iter [AjIStr*] String iterator
** @return [void]
** @@
******************************************************************************/

void ajStrIterDel(AjIStr* iter)
{
    AJFREE(*iter);

    return;
}


/* @obsolete ajStrIterFree
** @rename ajStrIterDel
*/

void __deprecated ajStrIterFree(AjIStr* iter)
{
    ajStrIterDel(iter);
}

/* @section tests
** @fdata [AjIStr]
** @nam4rule   Done      Check whether iteration has ended (no more
**                             characters).
** @nam5rule   DoneBack  Reverse iterator
**
** @argrule Done iter [const AjIStr] Iterator
**
** @valrule * [AjBool] Result of test
**
** @fcategory use
*/

/* @func ajStrIterDone ******************************************************
**
** Tests whether a string iterator has completed yet.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [AjBool] true if complete
** @@
******************************************************************************/

AjBool ajStrIterDone(const AjIStr iter)
{
    return (iter->Ptr > iter->End);
}

/* @func ajStrIterDoneBack ****************************************************
**
** Tests whether a string iterator (from end to start) has completed yet.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [AjBool] true if complete
** @@
******************************************************************************/

AjBool ajStrIterDoneBack(const AjIStr iter)
{
    return (iter->Ptr < iter->Start);
}



/* @obsolete ajStrIterBackDone
** @rename ajStrIterDoneBack
*/

AjBool __deprecated ajStrIterBackDone(AjIStr iter)
{
    return ajStrIterDoneBack(iter);
}

/* @section resets
** @fdata [AjIStr]
**
** @nam4rule   Begin     Check whether iterator is at start (first
**                             character).
** @nam4rule   End       Check whether iterator is at end (last
**                             character).
** @argrule * iter [AjIStr] String iterator
**
** @valrule * [void]
**
** @fcategory modify
*/

/* @func ajStrIterBegin *******************************************************
**
** Sets a string iterator to its start condition,
**
** @param [u] iter [AjIStr] String iterator.
** @return [void]
** @@
******************************************************************************/

void ajStrIterBegin(AjIStr iter)
{
    iter->Ptr = iter->Start;
    return;
}

/* @func ajStrIterEnd ********************************************************
**
** Sets a string iterator to its stop condition.
**
** @param [u] iter [AjIStr] String iterator.
** @return [void]
** @@
******************************************************************************/

void ajStrIterEnd(AjIStr iter)
{
    iter->Ptr = iter->End;
}

/* @section attributes
** @fdata [AjIStr]
**
** @nam4rule Get Return element
**
** @argrule Get iter [const AjIStr] Iterator
** @valrule GetC [const char*]
** @valrule GetK [char]
**
** @fcategory cast
*/

/* @func ajStrIterGetC *******************************************************
**
** Returns the remainder of the string at the current string iterator 
** position.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [const char*] Current text string within iterator
** @@
******************************************************************************/

const char* ajStrIterGetC(const AjIStr iter)
{
    return iter->Ptr;
}


/* @func ajStrIterGetK ********************************************************
**
** Returns the value (character) at the current string iterator position.
**
** @param [r] iter [const AjIStr] String iterator.
** @return [char] Current character within iterator
** @@
******************************************************************************/

char ajStrIterGetK(const AjIStr iter)
{
    return *(iter->Ptr);
}


/* @section modifiers
**
** @fdata [AjIStr]
**
** @nam4rule Put Replace an element
**
** @argrule * iter [AjIStr]
** @argrule K chr [char] Character to put at current position
** @valrule * [void]
**
** @fcategory modify
*/

/* @func ajStrIterPutK ********************************************************
**
** Replaces the character at the current string iterator position.
**
** @param [u] iter [AjIStr] String iterator.
** @param [r] chr [char] Character
** @return [void]
** @@
******************************************************************************/

void ajStrIterPutK(AjIStr iter, char chr)
{
    *iter->Ptr = chr;
    return;
}



/* @section stepping
**
** @fdata [AjIStr]
**
** @nam4rule   Next      Iterate to next character.
** @nam5rule   NextBack  Reverse iterator
**
** @argrule * iter [AjIStr] String iterator
**
** @valrule * [AjIStr] Updated iterator
**
** @fcategory modify
*/


/* @func ajStrIterNext ********************************************************
**
** Step to next character in string iterator.
**
** @param [u] iter [AjIStr] String iterator.
** @return [AjIStr] Updated iterator duplicated as return value.
** @@
******************************************************************************/

AjIStr ajStrIterNext(AjIStr iter)
{
    iter->Ptr++;
    if(iter->Ptr > iter->End)
	return NULL;

    return iter;
}




/* @func ajStrIterNextBack ****************************************************
**
** Step to previous character in string iterator.
**
** @param [u] iter [AjIStr] String iterator.
** @return [AjIStr] Updated iterator duplicated as return value.
** @@
******************************************************************************/

AjIStr ajStrIterNextBack(AjIStr iter)
{
    iter->Ptr--;

    if(iter->Ptr < iter->Start)
	return NULL;

    return iter;
}



/* @obsolete ajStrIterBackNext
** @rename ajStrIterNextBack
*/

AjIStr __deprecated ajStrIterBackNext(AjIStr iter)
{
    return ajStrIterNextBack(iter);
}


/* @obsolete ajStrIterMore
** @replace ajStrIterDone (1/!1)
*/

AjBool __deprecated ajStrIterMore(AjIStr iter)
{
    return (!ajStrIterDone(iter));
}




/* @obsolete ajStrIterMoreBack
** @rename ajStrIterDoneBack
*/

AjBool __deprecated ajStrIterMoreBack(AjIStr iter)
{
    return (!ajStrIterDoneBack(iter));
}


/* @datasection [AjPStrTok] String token parser *******************************
**
** @nam2rule Str String
** @nam3rule Token Token parser
**
******************************************************************************/

/* @section constructors
**
** @fdata [AjPStrTok]
**
** @nam4rule New Constructor
**
** @argrule New str [const AjPStr] string
** @argrule C txtdelim [const char*] Text delimiter
** @argrule S strdelim [const AjPStr] Text delimiter
** @valrule * [AjPStrTok] String token parser
**
** @fcategory new
*/


/* @func ajStrTokenNewC *******************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and a set of default delimiters defined in a 
** text string.
**
** @param [r] str [const AjPStr] Source string
** @param [r] txtdelim [const char*] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
** @@
******************************************************************************/

AjPStrTok ajStrTokenNewC(const AjPStr str, const char* txtdelim)
{
    AjPStrTok ret;

    AJNEW0(ret);

    ajStrAssignS(&ret->String, str);

    ret->Delim = ajStrNewC(txtdelim);
    ret->Pos   = 0;		  /* GFF parsing needed this change */

    return ret;
}


/* @obsolete ajStrTokenInit
** @rename ajStrTokenNewC
*/

AjPStrTok __deprecated ajStrTokenInit(const AjPStr thys, const char* delim)
{
    return ajStrTokenNewC(thys, delim);
}

/* @func ajStrTokenNewS *******************************************************
**
** String token parser constructor which allocates memory for a string token 
** parser object from a string and an optional set of default delimiters 
** defined in a text string.
**
** @param [r] str [const AjPStr] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
** @@
******************************************************************************/

AjPStrTok ajStrTokenNewS(const AjPStr str, const AjPStr strdelim)
{
    AjPStrTok ret;

    AJNEW0(ret);

    ajStrAssignS(&ret->String, str);

    ret->Delim = ajStrNewS(strdelim);
    ret->Pos   = 0;		  /* GFF parsing needed this change */

    return ret;
}


/* @section destructors
**
** @fdata [AjPStrTok]
**
** @nam4rule Del Destructor
**
** @argrule Del Ptoken [AjPStrTok*] String token parser
**
** @valrule * [void]
**
** @fcategory delete
*/

/* @func ajStrTokenDel ******************************************************
**
** String token parser destructor which frees memory for a string token 
** parser.
**
** @param [d] Ptoken [AjPStrTok*] Token parser
** @return [void]
** @@
******************************************************************************/

void ajStrTokenDel(AjPStrTok* Ptoken)
{
    if(!*Ptoken)
	return;

    ajStrDel(&(*Ptoken)->String);
    ajStrDel(&(*Ptoken)->Delim);
    AJFREE(*Ptoken);

    return;
}




/* @obsolete ajStrTokenClear
** @rename ajStrTokenDel
*/

void __deprecated ajStrTokenClear(AjPStrTok* token)
{
    ajStrTokenDel(token);
    return;
}


/* @section assignment
**
** @fdata [AjPStrTok]
**
** @nam4rule Assign Assignment of string and/or delimiter(s)
**
** @argrule Assign Ptoken [AjPStrTok*] Token parser
** @argrule Assign str [const AjPStr] String to be parsed
** @argrule C txtdelim [const char*] delimiter
** @argrule S strdelim [const AjPStr] delimiter
**
** @valrule * [AjBool] True on success
**
** @fcategory assign
*/

/* @func ajStrTokenAssign ****************************************************
**
** Generates a string token parser object from a string without a specific
** set of delimiters.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAssign(AjPStrTok* Ptoken, const AjPStr str)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignS(&tok->String, str);
    ajStrAssignC(&tok->Delim, "");
    tok->Pos = 0;

    return ajTrue;
}



/* @obsolete ajStrTokenAss
** @rename ajStrTokenAssignC
*/

AjBool __deprecated ajStrTokenAss(AjPStrTok* ptok, const AjPStr thys,
				  const char* delim)
{
    return ajStrTokenAssignC(ptok, thys, delim);
}

/* @func ajStrTokenAssignC ****************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters defined in a text string.
**
** The string token can be either an existing token to be overwritten
** or a NULL.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @param [r] txtdelim [const char*] Default delimiter(s)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAssignC(AjPStrTok* Ptoken, const AjPStr str,
			 const char* txtdelim)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignS(&tok->String, str);
    ajStrAssignC(&tok->Delim, txtdelim);
    tok->Pos = strspn(tok->String->Ptr, tok->Delim->Ptr);

    return ajTrue;
}



/* @func ajStrTokenAssignS ****************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters defined in a string.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @param [r] str [const AjPStr] Source string
** @param [r] strdelim [const AjPStr] Default delimiter(s)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAssignS(AjPStrTok* Ptoken, const AjPStr str,
			 const AjPStr strdelim)
{
    AjPStrTok tok;
    
    if(*Ptoken)
	tok = *Ptoken;
    else
    {
	AJNEW0(tok);
	*Ptoken = tok;
    }
    
    ajStrAssignS(&tok->String, str);
    ajStrAssignS(&tok->Delim, strdelim);
    tok->Pos = strspn(tok->String->Ptr, tok->Delim->Ptr);

    return ajTrue;
}

/* @section reset
**
** @fdata [AjPStrTok]
**
** @nam4rule Reset Reset the token parser internals
**
** @argrule * Ptoken [AjPStrTok*] String token parser
**
** @valrule * [void]
**
** @fcategory modify
*/

/* @func ajStrTokenReset ******************************************************
**
** Clears the strings from a string token parser object.
**
** @param [w] Ptoken [AjPStrTok*] String token object
** @return [void]
** @@
******************************************************************************/

void ajStrTokenReset(AjPStrTok* Ptoken)
{
    AjPStrTok tok;

    if(!Ptoken)
	return;

    if(!*Ptoken)
	return;

    tok = *Ptoken;

    ajStrDelStatic(&tok->String);
    ajStrDelStatic(&tok->Delim);

    return;
}




/* @section debug
**
** @fdata [AjPStrTok]
**
** @nam4rule Trace Write internals to debug file
**
** @argrule * token [const AjPStrTok] String token parser
**
** @valrule * [void]
**
** @fcategory misc
*/

/* @func ajStrTokenTrace ******************************************************
**
** Writes a debug trace of a string token parser object.
**
** @param [r] token [const AjPStrTok] String token object
** @return [void]
** @@
******************************************************************************/

void ajStrTokenTrace(const AjPStrTok token)
{
    ajDebug("ajStrTokenTrace %x\n", token);

    if(!token)
	return;

    ajDebug("... String:\n");
    ajStrTrace(token->String);
    ajDebug("... Delim:\n");
    ajStrTrace(token->Delim);

    return;
}




/* @section parsing
**
** @fdata [AjPStrTok]
**
** @nam4rule Next Return next token
** @nam5rule NextFind Use delimiter as a set of characters
** @nam5rule NextParse Use delimiter as a string
** @nam4rule Rest Return remainder of string
** @nam5rule RestParse Return remainder of string
**
** @argrule * Ptoken [AjPStrTok*] String token parser
** @argrule C txtdelim [const char*] Delimiter
** @argrule S strdelim [const AjPStr] Delimiter
** @argrule * Pstr [AjPStr*] String result
**
** @valrule * [AjBool] True on success
**
** @fcategory modify
*/

/* @func ajStrTokenNextFind *************************************************
**
** Parses tokens from a string using a string token parser.  Treats the 
** delimiter as a set of characters. 
**
** @param [u] Ptoken [AjPStrTok*] Token parser. Updated with the delimiter
**        string (if any) in delim.
** @param [w] Pstr [AjPStr*] Token found
**
** @return [AjBool] ajTrue if another token was found.
** @@
******************************************************************************/

AjBool ajStrTokenNextFind(AjPStrTok* Ptoken, AjPStr* Pstr)
{
    ajint ilen;
    AjPStrTok token;
    char* cp;
    char* cq;

    token = *Ptoken;

    if(!*Ptoken)
    {					/* token already cleared */
	ajStrAssignC(Pstr, "");
	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignC(Pstr, "");
	ajStrTokenDel(Ptoken);
	return ajFalse;
    }

    cp = &token->String->Ptr[token->Pos];
    cq = strstr(cp, token->Delim->Ptr);
    if(cq)
    {
	ilen = cq - cp;
	ajStrAssignSubS(Pstr, token->String, token->Pos,
			token->Pos + ilen - 1);
	token->Pos += ilen;
	token->Pos += token->Delim->Len;
	return ajTrue;
    }

    /* delimiter not found - return rest of string */

    ilen = token->String->Len - token->Pos;
    ajStrAssignLenC(Pstr, cp, ilen);
    token->Pos += ilen;

    return ajTrue;
}



/* @obsolete ajStrDelim
** @replace ajStrTokenNextFind (1,2,n/2,1)
** @replace ajStrTokenNextFindC (1,2,3/2,3,1)
*/

AjBool __deprecated ajStrDelim(AjPStr* pthis, AjPStrTok* ptoken,
			       const char* delim)
{
    if(delim)
	return ajStrTokenNextFindC(ptoken, delim, pthis);
    else
	return ajStrTokenNextFind(ptoken, pthis);
}

/* @func ajStrTokenNextFindC *************************************************
**
** Parses tokens from a string using a string token parser.  Uses characters 
** from a defined text string as a delimiter.
**
** @param [u] Ptoken [AjPStrTok*] Token parser. Updated with the delimiter
**        string in delim.
** @param [r] txtdelim [const char*] Delimiter string.
** @param [w] Pstr [AjPStr*] Token found
**
** @return [AjBool] ajTrue if another token was found.
** @@
******************************************************************************/

AjBool ajStrTokenNextFindC(AjPStrTok* Ptoken, const char* txtdelim,
			   AjPStr* Pstr)
{
    AjPStrTok token;

    token = *Ptoken;

    ajStrAssignC(&token->Delim, txtdelim);

    return ajStrTokenNextFind(Ptoken, Pstr);
}

/* @func ajStrTokenNextParse **************************************************
**
** Parses tokens from a string using a string token parser.  Treats the 
** delimiter as a string.
**
** The test uses the C function 'strcspn'.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenNextParse(AjPStrTok* Ptoken, AjPStr* Pstr)
{
    ajint ilen;
    AjPStrTok token;
    char* cp;

    token = *Ptoken;

    if(!*Ptoken)
    {					/* token already cleared */
	ajStrAssignC(Pstr, "");
	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignC(Pstr, "");
	ajStrTokenDel(Ptoken);
	return ajFalse;
    }

    cp = &token->String->Ptr[token->Pos];
    ilen = strcspn(cp, token->Delim->Ptr);

    if(ilen)
    {
	ajStrAssignSubS(Pstr, token->String,
		       token->Pos, token->Pos + ilen - 1);
    }
    else
    {
	ajStrAssignC(Pstr, "");
    }
    token->Pos += ilen;
    token->Pos += strspn(&token->String->Ptr[token->Pos], token->Delim->Ptr);

    return ajTrue;
}



/* @obsolete ajStrToken
** @replace ajStrTokenNextParse (1,2,n/2,1)
** @replace ajStrTokenNextParseC (1,2,3/2,3,1)
*/
AjBool __deprecated ajStrToken(AjPStr* pthis, AjPStrTok* ptoken,
			       const char* delim)
{
    if(delim)
	return ajStrTokenNextParseC(ptoken, delim, pthis);
    else
	return ajStrTokenNextParse(ptoken, pthis);
}

/* @func ajStrTokenNextParseC *************************************************
**
** Parses tokens from a string using a string token parser.  Uses characters 
** from a defined text string as a delimiter.
**
** Returns the next token parsed from a string token parsing object
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [r] txtdelim [const char*] Delimiter character set.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenNextParseC(AjPStrTok* Ptoken, const char* txtdelim,
			    AjPStr* Pstr)
{
    if(!*Ptoken) {
	ajStrAssignC(Pstr, "");
	return ajFalse;
    }
    ajStrAssignC(&(*Ptoken)->Delim, txtdelim);
    return ajStrTokenNextParse(Ptoken, Pstr);
}




/* @func ajStrTokenNextParseS *************************************************
**
** Parses tokens from a string using a string token parser.  Uses characters 
** from a defined string as a delimiter.
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test uses the C function 'strcspn'.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [r] strdelim [const AjPStr] Delimiter character set.
** @param [w] Pstr [AjPStr*] Next token returned, may be empty if the
**                           delimiter has changed.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenNextParseS(AjPStrTok* Ptoken, const AjPStr strdelim,
			    AjPStr* Pstr)
{
    if(!*Ptoken)
    {
	ajStrAssignC(Pstr, "");
	return ajFalse;
    }

    ajStrAssignS(&(*Ptoken)->Delim, strdelim);
    return ajStrTokenNextParse(Ptoken, Pstr);
}






/* @func ajStrTokenRestParse **************************************************
**
** Returns the remainder of a string that's been partially parsed using a 
** string token parser.
**
** @param [u] Ptoken [AjPStrTok*] String token parsing object.
** @param [w] Pstr [AjPStr*] Next token returned.
** @return [AjBool] True if successfully parsed.
**                  False (and string set to empty) if there is nothing
**                  more to parse.
** @@
******************************************************************************/

AjBool ajStrTokenRestParse(AjPStrTok* Ptoken, AjPStr* Pstr)
{
    AjPStrTok token;

    token = *Ptoken;

    if(!*Ptoken)
    {					/* token already cleared */
	ajStrAssignC(Pstr, "");
	return ajFalse;
    }

    if(token->Pos >= token->String->Len)
    {					/* all done */
	ajStrAssignC(Pstr, "");
	ajStrTokenDel(Ptoken);
	return ajFalse;
    }

    if(token->Pos < token->String->Len)
	ajStrAssignSubS(Pstr, token->String,
			    token->Pos, token->String->Len);
    else
	ajStrAssignC(Pstr, "");

    token->Pos = token->String->Len;

    return ajTrue;
}


/* @obsolete ajStrTokenRest
** @replace ajStrTokenRestParse (1,2/2,1)
*/

AjBool __deprecated ajStrTokenRest(AjPStr* pthis, AjPStrTok* ptoken)
{
    return ajStrTokenRestParse(ptoken, pthis);
}



/* @obsolete ajStrNull
** @remove Make a local empty string instead
*/

const AjPStr __deprecated ajStrNull(void)
{
    return strPNULL;
}





/* @obsolete ajStrArrayDel
** @remove Make a local copy if needed - no need for full set of array functions
*/

void __deprecated ajStrArrayDel(AjPStr** pthis)
{
    AjPStr* thys;
    ajint i;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    for (i=0; thys[i];i++)
    {
	ajStrDel(&thys[i]);
    }

    AJFREE(*pthis);
    return;
}





/* @obsolete ajStrAss
** @replace ajStrAssignC (1,n/1,'""')
** @replace ajStrAssignRef (1,2/1,2)
*/

AjBool __deprecated ajStrAss(AjPStr* pthis, AjPStr str)
{
    AjBool ret = ajTrue;		/* always true for now */

    ajStrDel(pthis);		/* we just use the ref count of str */

    if(str)
	ajStrAssignRef(pthis, str);
    else
	ajStrAssignC(pthis, "");

    return ret;
}




/* @obsolete ajStrCopyC
** @replace ajStrDel (1,n/1)
** @replace ajStrAssignC (1,2/1,2)
*/

AjBool __deprecated ajStrCopyC(AjPStr* pthis, const char* str)
{
    AjBool ret = ajTrue;		/* true if ajStrDup is used */

    if(!str)
	ajStrDel(pthis);
    else
	ajStrAssignC(pthis, str);

    return ret;
}




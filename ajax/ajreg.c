/******************************************************************************
** @source AJAX REG (ajax regular expression) functions
**
** Uses Henry Spencer's older regular expression library at present.
** The newer version appears to have some performance problems,
** especially in extracting substrings which we may need to use
** quite extensively.
**
** A major reason for adding this layer is to allow a future
** near-transparent switch to the POSIX version of the Henry Spencer
** library at some future date.
**
** Some extra tricks are added, such as simply returning the matched
** string or the nth substring.
**
** Possible extensions include a case-insensitive regcomp ... which
** would be useful in many places. Also a regcomp for prefices,
** suffices and exact matches which adds "^" and "$" to the pattern
** string.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
** @modified Jun 25 pmr First version
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
#include "ajreg.h"




static ajlong regAlloc     = 0;
static ajlong regFree      = 0;
static ajlong regFreeCount = 0;
static ajlong regCount     = 0;
static ajlong regTotal     = 0;




/* constructors */

/* @func ajRegComp ************************************************************
**
** Compiles a regular expression.
**
** @param [r] exp [AjPStr] Regular expression string.
** @return [AjPRegexp] Compiled regular expression.
** @@
******************************************************************************/

AjPRegexp ajRegComp(AjPStr exp)
{
    return ajRegCompC(ajStrStr(exp));
}




/* @func ajRegCompC ***********************************************************
**
** Compiles a regular expression.
**
** @param [r] exp [const char*] Regular expression character string.
** @return [AjPRegexp] Compiled regular expression.
** @@
******************************************************************************/

AjPRegexp ajRegCompC(const char* exp)
{
    AjPRegexp ret;
    int options = 0;
    int errpos  = 0;
    const char *errptr            = NULL;
    const unsigned char *tableptr = NULL;

    AJNEW0(ret);
    AJCNEW0(ret->ovector, AJREG_OVECSIZE);
    ret->ovecsize = AJREG_OVECSIZE/3;
    ret->pcre = pcre_compile(exp, options, &errptr, &errpos, tableptr);
    if(!ret->pcre)
    {
	ajErr("Failed to compile regular expression '%s' at position %d: %s",
	      exp, errpos, errptr);
	AJFREE(ret);
	return NULL;
    }

    regAlloc += sizeof(ret);
    regCount ++;
    regTotal ++;
    /*ajDebug("ajRegCompC %x size %d regexp '%s'\n",
      ret, (int) sizeof(ret), exp);*/

    return ret;
}



/* @func ajRegCompCase ********************************************************
**
** Compiles a case-insensitive regular expression.
**
** @param [r] exp [AjPStr] Regular expression string.
** @return [AjPRegexp] Compiled regular expression.
** @@
******************************************************************************/

AjPRegexp ajRegCompCase(AjPStr exp)
{
    return ajRegCompCaseC(ajStrStr(exp));
}





/* @func ajRegCompCaseC *******************************************************
**
** Compiles a case-insensitive regular expression.
**
** @param [r] exp [const char*] Regular expression character string.
** @return [AjPRegexp] Compiled regular expression.
** @@
******************************************************************************/

AjPRegexp ajRegCompCaseC(const char* exp)
{
    AjPRegexp ret;
    int options = PCRE_CASELESS;
    int errpos  = 0;
    const char *errptr            = NULL;
    const unsigned char *tableptr = NULL;

    AJNEW0(ret);
    AJCNEW0(ret->ovector, AJREG_OVECSIZE);
    ret->ovecsize = AJREG_OVECSIZE/3;
    ret->pcre = pcre_compile(exp, options, &errptr, &errpos, tableptr);
    if(!ret->pcre)
    {
	ajErr("Failed to compile regular expression '%s' at position %d: %s",
	      exp, errpos, errptr);
	AJFREE(ret);
	return NULL;
    }

    regAlloc += sizeof(ret);
    regCount ++;
    regTotal ++;
    /*ajDebug("ajRegCompCaseC %x size %d regexp '%s'\n",
      ret, (int) sizeof(ret), exp);*/

    return ret;
}




/* execute expression match */

/* @func ajRegExec ************************************************************
**
** Execute a regular expression search.
** The expression must first have been compiled with ajRegComp or ajRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPRegexp] Compiled regular expression.
** @param [r] str [const AjPStr] String to be compared.
** @return [AjBool] ajTrue if a match was found.
** @@
******************************************************************************/

AjBool ajRegExec(AjPRegexp prog, const AjPStr str)
{
    int startoffset = 0;
    int options     = 0;
    int status      = 0;
    char msgbuf[1024];

    status = pcre_exec(prog->pcre, prog->extra, ajStrStr(str), ajStrLen(str),
		       startoffset, options, prog->ovector, 3*prog->ovecsize);
    if(status >= 0)
    {
	prog->orig = ajStrStr(str);
	/* ajRegTrace(prog); */
	if(status == 0)
	    ajWarn("ajRegExec too many substrings");
	return ajTrue;
    }

    if(status < -1)
    {
	pcre_regerror(status, (const regex_t *)prog->pcre, msgbuf, 1024);
	ajWarn("ajRegExec problem status %d '%s'", status, msgbuf);
    }

    prog->orig = NULL;

    return ajFalse;
}




/* @func ajRegExecC ***********************************************************
**
** Execute a regular expression search.
** The expression must first have been compiled with ajRegComp or ajRegCompC.
**
** Internal data structures in the expression will be set to substrings
** which other functions can retrieve.
**
** @param [u] prog [AjPRegexp] Compiled regular expression.
** @param [r] str [const char*] String to be compared.
** @return [AjBool] ajTrue if a match was found.
** @@
******************************************************************************/

AjBool ajRegExecC(AjPRegexp prog, const char* str)
{
    int startoffset = 0;
    int options     = 0;
    int status      = 0;
    char msgbuf[1024];

    status = pcre_exec(prog->pcre, prog->extra, str, strlen(str),
		       startoffset, options, prog->ovector, 3*prog->ovecsize);
    if(status >= 0)
    {
	prog->orig = str;
	/* ajRegTrace(prog); */
	if(status == 0)
	    ajWarn("ajRegExecC too many substrings");
	return ajTrue;
    }

    if(status < -1)
    {
	pcre_regerror(status, (const regex_t *)prog->pcre, msgbuf, 1024);
	ajWarn("ajRegExecC problem status %d '%s'", status, msgbuf);
    }

    prog->orig = NULL;

    return ajFalse;
}




/* @func ajRegOffset **********************************************************
**
** After a successful regular expression match, uses the regular
** expression and the original string to calculate the offset
** of the match from the start of the string.
**
** This information is normally lost during processing.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @return [ajint] Offset of match from start of string.
**               -1 if the string and the expression do not match.
** @@
******************************************************************************/

ajint ajRegOffset(AjPRegexp rp)
{
    return (rp->ovector[0]);
}




/* @func ajRegOffsetI *********************************************************
**
** After a successful regular expression match, uses the regular
** expression and the original string to calculate the offset
** of a substring from the start of the string.
**
** This information is normally lost during processing.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Offset of match from start of string.
**               -1 if the string and the expression do not match.
** @@
******************************************************************************/

ajint ajRegOffsetI(AjPRegexp rp, ajint isub)
{
    if(isub < 1)
	ajErr("Invalid substring number %d", isub);;

    if(isub >= (rp->ovecsize))
	ajErr("Invalid substring number %d", isub);;

    return (rp->ovector[isub*2]);
}




/* @func ajRegLenI ************************************************************
**
** After a successful comparison, returns the length of a substring.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @param [r] isub [ajint] Substring number.
** @return [ajint] Substring length, or 0 if not found.
** @@
******************************************************************************/

ajint ajRegLenI(AjPRegexp rp, ajint isub)
{
    ajint istart;
    ajint iend;

    istart = 2*isub;
    iend   = istart+1;

    if(isub < 0)
	return 0;

    if(isub >= rp->ovecsize)
	return 0;

    if(rp->ovector[istart] < 0)
	return 0;

    return (rp->ovector[iend] - rp->ovector[istart]);
}




/* @func ajRegPost ************************************************************
**
** After a successful match, returns the remainder of the string.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @param [w] post [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajRegPost(AjPRegexp rp, AjPStr* post)
{
    if(rp->ovector[1])
    {
	ajStrAssC(post, &rp->orig[rp->ovector[1]]);
	return ajTrue;
    }

    ajStrDelReuse(post);

    return ajFalse;
}




/* @func ajRegPostC ***********************************************************
**
** After a successful match, returns the remainder of the string.
** Result is a character string, which is set to point to the internal
** string data. This in turn is part of the original string. If this
** changes then the results are undefined.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @param [w] post [const char**] Character string to hold the result.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajRegPostC(AjPRegexp rp, const char** post)
{
    if(rp->ovector[1])
    {
	*post = &rp->orig[rp->ovector[1]];
	return ajTrue;
    }

    *post = 0;

    return ajFalse;
}




/* @func ajRegPre ************************************************************
**
** After a successful match, returns the string before the match.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @param [w] post [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajRegPre(AjPRegexp rp, AjPStr* dest)
{
    ajint ilen;

    ilen = rp->ovector[0];
    ajStrModL(dest, ilen+1);
    if(ilen)
    {
	strncpy((*dest)->Ptr, rp->orig, ilen);
	(*dest)->Len = ilen;
	(*dest)->Ptr[ilen] = '\0';

	return ajTrue;
    }

    ajStrDelReuse(dest);

    return ajFalse;
}





/* @func ajRegSubI ************************************************************
**
** After a successful match, returns a substring.
**
** @param [r] rp [AjPRegexp] Compiled regular expression.
** @param [r] isub [ajint] Substring number.
** @param [w] dest [AjPStr*] String to hold the result.
** @return [AjBool] ajTrue if a substring was defined
**                  ajFalse if the substring is not matched
**                  ajFalse if isub is out of range
** @@
******************************************************************************/

AjBool ajRegSubI(AjPRegexp rp, ajint isub, AjPStr* dest)
{
    ajint ilen;
    ajint istart;
    ajint iend;

    istart = 2*isub;
    iend   = istart+1;

    if(isub < 0)
    {
	ajStrDelReuse(dest);
	return ajFalse;
    }

    if(isub >= rp->ovecsize)
    {
	ajStrDelReuse(dest);
	return ajFalse;
    }

    if(rp->ovector[istart] < 0)
    {
	ajStrDelReuse(dest);
	return ajFalse;
    }

    ilen = rp->ovector[iend] - rp->ovector[istart];
    ajStrModL(dest, ilen+1);
    if(ilen)
	strncpy((*dest)->Ptr, &rp->orig[rp->ovector[istart]], ilen);
    (*dest)->Len = ilen;
    (*dest)->Ptr[ilen] = '\0';

    return ajTrue;
}




/* destructor */

/* @func ajRegFree ************************************************************
**
** Clears and frees a compiled regular expression.
**
** @param [w] pexp [AjPRegexp*] Compiled regular expression.
** @return [void]
** @@
******************************************************************************/

void ajRegFree(AjPRegexp* pexp)
{
    AjPRegexp exp;

    if(!pexp)
	return;

    if(!*pexp)
	return;

    exp = *pexp;

    /*
       ajDebug("ajRegFree %x size regexp %d\n", exp,
       (ajint) sizeof(exp));
    */

    regFreeCount += 1;
    regFree += sizeof(*exp);
    if(exp->pcre)
	regFree += sizeof(exp->pcre);
    if(exp->extra)
	regFree += sizeof(exp->extra);
    regTotal --;

    AJFREE(exp->pcre);
    AJFREE(exp->extra);
    AJFREE(exp->ovector);
    AJFREE(*pexp);

    return;
}




/* @func ajRegTrace ***********************************************************
**
** Traces a compiled regular expression with debug calls.
**
** @param [r] exp [AjPRegexp] Compiled regular expression.
** @return [void]
** @@
******************************************************************************/

void ajRegTrace(AjPRegexp exp)
{
    ajint isub;
    ajint ilen;
    ajint ipos;
    ajint istart;
    ajint iend;
    static AjPStr str = NULL;

    ajDebug("  REGEXP trace\n");
    for(isub=0; isub < exp->ovecsize; isub++)
    {
	istart = 2*isub;
	iend   = istart+1;
	if(exp->ovector[iend] >= exp->ovector[istart])
	{
	    ilen = exp->ovector[iend] - exp->ovector[istart];
	    ajStrModL(&str, ilen+1);
	    strncpy(str->Ptr, &exp->orig[exp->ovector[istart]], ilen);
	    str->Len = ilen;
	    str->Ptr[ilen] = '\0';
	    if(!isub)
	    {
		ajDebug(" original string '%s'\n", exp->orig);
		ajDebug("    string match '%S'\n", str);
	    }
	    else
	    {
		ipos = exp->ovector[istart];
		ajDebug("    substring %2d '%S' at %d\n", isub, str, ipos);
	    }
	}
    }
    ajDebug("\n");

    ajStrDelReuse(&str);

    return;
}




/* @func ajRegExit ************************************************************
**
** Prints a summary of regular expression (AjPRegexp) usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajRegExit(void)
{
    ajDebug("Regexp usage (bytes): %ld allocated, %ld freed, %ld in use\n",
	     regAlloc, regFree, regAlloc - regFree);
    ajDebug("Regexp usage (number): %ld allocated, %ld freed %ld in use\n",
	     regTotal, regFreeCount, regCount);

    return;
}

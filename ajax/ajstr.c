/********************************************************************
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
********************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#ifndef HAVE_MEMMOVE
#include <sys/types.h>
static void* memmove (void *dst, const void* src, size_t len) {
  return (void *)bcopy (src, dst, len);
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

#define STRSIZE 32
#define LONGSTR 512
#define NULL_USE 5
char charNULL[1] = "";

AjOStr strONULL = { 1, 0, NULL_USE, charNULL}; /* use set to avoid changes */
AjPStr strPNULL = &strONULL;

/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */


static AjPStr strNewNew (size_t size);
static void strClone (AjPStr* pthis);
static void strCloneL (AjPStr* pthis, size_t size);


static ajlong strAlloc = 0;
static ajlong strFree = 0;
static ajlong strFreeCount = 0;
static ajlong strCount = 0;
static ajlong strTotal = 0;

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section String Constructors ***********************************************
**
** All constructors return a new string by pointer. It is the responsibility
** of the user to first destroy any previous string. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing string, see instead
** the {String Assignments} and {String Modifiers} functions.
**
** All string constructors eventually use an internal call to
** static function {strNewNew}.
**
** The range of constructors is provided to allow flexibility in how
** applications can define new strings.
**
******************************************************************************/

/* @func ajStrNew *************************************************************
**
** Default constructor for empty AJAX strings.
** The null string usage pointer is incremented.
**
** @return [AjPStr] Pointer to an empty string
** @@
******************************************************************************/

AjPStr ajStrNew (void) {

  return ajStrDup (strPNULL);
}

/* @func ajStrNewC ************************************************************
**
** Constructor given an initial text string. The string size is set
** just large enough to hold the supplied text.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @return [AjPStr] Pointer to a string containing the supplied text
** @@
******************************************************************************/

AjPStr ajStrNewC (const char* txt) { 

  ajint i, j;
  AjPStr thys;

  i = strlen(txt);
  j = ajRound (i + 1, STRSIZE);

  thys = ajStrNewCIL (txt, i, j);

  return thys;
}

/* @func ajStrNewL ************************************************************
**
** Constructor given an initial reserved size (including a possible null).
**
** @param [r] size [size_t] Reserved size (including a possible null).
** @return [AjPStr] Pointer to an empty string of specified size.
** @@
******************************************************************************/

AjPStr ajStrNewL (size_t size) { 

  AjPStr thys;

  thys = ajStrNewCIL ("", 0, size);
  return thys;
}

/* @func ajStrNewCL ***********************************************************
**
** Constructor given a text string and an initial reserved size
** (including a possible null) to allow for future expansion.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewCL (const char* txt, size_t size) { 

  ajint i;
  AjPStr thys;

  i = strlen(txt);

  thys = ajStrNewCIL (txt, i, size);
  return thys;
}

/* @func ajStrNewCIL **********************************************************
**
** Constructor given a text string, its length and an initial reserved size
** (including a possible null) to allow for future expansion.
**
** This is a general function used by the other constructors, which
** simply fill in the missing values and then call ajStrNewCIL.
**
** @param [r] txt [const char*] Null-terminated character string to initialise
**        the new string.
** @param [r] len [ajint] Length of txt to save calculation time.
** @param [r] size [size_t]  Reserved size (including a possible null).
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewCIL (const char* txt, ajint len, size_t size) { 

  AjPStr thys;

  ajint minlen = size;

  if ((size <= len) || (size == NPOS))
    minlen = len+1;

  thys = strNewNew(minlen);
  thys->Len = len;
  if (txt)
    (void) strncpy (thys->Ptr, txt, len+1);
  thys->Ptr[len] = '\0';

  return thys;
}

/* @func ajStrNewS **********************************************************
**
** Constructor given a text string, its length and an initial reserved size
** (including a possible null) to allow for future expansion.
**
** This is a general function used by the other constructors, which
** simply fill in the missing values and then call ajStrNewCIL.
**
** @param [r] str [AjPStr] String to be cloned
** @return [AjPStr] Pointer to a string of the specified size
**         containing the supplied text.
** @@
******************************************************************************/

AjPStr ajStrNewS (AjPStr str) { 

  return ajStrNewCIL (str->Ptr, str->Len, str->Res);
}

/* @func ajStrDup *************************************************************
**
** Constructor making a copy of a string object.
**
** @param [r] thys [const AjPStr] AJAX string object
** @return [AjPStr] Pointer to the string passed as an argument,
**         with its use count increased by 1.
** @@
** Modified: pmr 21-jan-00 return empty string if there is no original string
******************************************************************************/

AjPStr ajStrDup (const AjPStr thys) {	

  if (!thys)
    return ajStrNew();

  thys->Use++;

  return thys;
}

/* @func ajCharNew ************************************************************
**
** A text string constructor which allocates a string of the specified length
** and initialises it with the text string provided.
**
** @param [P] thys [const AjPStr] String object as initial value and size
**                          for the text.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNew (const AjPStr thys) {

  static char* cp;

  cp = (char*) AJALLOC(thys->Len+1);
  (void) strncpy (cp, thys->Ptr, thys->Len+1);

  return cp;
}

/* @func ajCharNewC ***********************************************************
**
** A text string constructor which allocates a string of the specified length
** and initialises it with the text string provided.
**
** @param [r] len [ajint] Length of the Cstring, excluding the trailing NULL.
** @param [r] txt [const char*] Initial text, possibly shorter than the
**        space allocated.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewC (ajint len, const char* txt) {

  static char* cp;

  cp = (char*) AJALLOC(len+1);
  (void) strncpy (cp, txt, len+1);

  return cp;
}

/* @func ajCharNewL ***********************************************************
**
** A text string constructor which allocates a string of the specified length
** but no contents.
**
** @param [r] len [ajint] Length of the Cstring, excluding the trailing NULL.
** @return [char*] A new text string with no contents.
** @@
******************************************************************************/

char* ajCharNewL (ajint len) {

  static char* cp;

  cp = (char*) AJALLOC(len+1);

  return cp;
}

/* @func ajCharNewLS **********************************************************
**
** A text string constructor which allocates a string of the specified length
** and initialises it with the text string provided.
**
** @param [r] size [size_t] Maximum string length, as returned by strlen
** @param [P] thys [const AjPStr] String object as initial value and size
**                          for the text.
** @return [char*] A new text string.
** @ure The text provided must fit in the specified length
** @@
******************************************************************************/

char* ajCharNewLS (size_t size, const AjPStr thys) {

  static char* cp;

  size_t isize = size;
  if (thys->Len >= isize) isize = thys->Len + 1;

  cp = (char*) AJALLOC(isize);
  (void) strncpy (cp, thys->Ptr, thys->Len+1);

  return cp;
}

/* #funcstatic ajCharNewBuffC ********************************************
**
** OBSOLETE: used in the EMBOSS DBI programs, replaced by ajNewCharC
**
** Constructor for a text string from an AjPStr, using a large buffer
** to reduce the number of mallocs.
**
** #param [r] str [char*] Text object
** #param [r] i [ajint] Length
** #return [char*] New text string.
******************************************************************************/

/*
static char* newcharCI (char* str, ajint i) {

  static char* buffer = NULL;
  static ajint ipos=0;
  static ajint imax=0;

  char* ret;

  if ((ipos+i) > imax) {
    AJCNEW(buffer, 1000000);
    ajDebug ("newchar need more memory ipos: %d i: %d  imax: %d buffer: %x\n",
	     ipos, i, imax, buffer);
    imax = 1000000;
    ipos = 0;
  }
  ret = &buffer[ipos];
  strncpy (ret, str, i);
  ipos += i;
  return ret;
}
*/

/* @funcstatic strNewNew ****************************************************
**
** Internal constructor for modifiable AJAX strings. Used by all the string
** parameterized contructors to allocate the space for the text string.
** The exception is ajStrNew which returns a clone of the null string.
**
** @param [r] size [size_t] size of the reserved space, including the
**        terminating NULL character.
** @return [AjPStr] A pointer to an empty string
** @@
******************************************************************************/

static AjPStr strNewNew (size_t size) {

  AjPStr ret;
  if (!size) size = STRSIZE;

  AJNEW0 (ret);
  ret->Res = size;
  ret->Ptr = AJALLOC(size);
  ret->Len = 0;
  ret->Use = 1;
  ret->Ptr[0] = '\0';

  strAlloc += size;
  strCount++;
  strTotal++;

  return ret;
}

/* @funcstatic strClone *****************************************************
**
** Makes a new clone of a string with a usage count of one.
** @param [wP] pthis [AjPStr*] String
** @return [void]
** @@
******************************************************************************/

static void strClone (AjPStr* pthis) {

  AjPStr thys = pthis ? *pthis : 0;
  AjPStr ret;

  ret = ajStrNewCIL (thys->Ptr, thys->Len, thys->Res);
  ajStrDel (pthis);
  *pthis = ret;

  return;
}

/* @funcstatic strCloneL ****************************************************
**
** Makes a new clone of a string with a usage count of one and a minimum
** reserved size.
**
** @param [wP] pthis [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [void]
** @@
******************************************************************************/

static void strCloneL (AjPStr* pthis, size_t size) {

  AjPStr thys = pthis ? *pthis : 0;
  AjPStr ret;

  ret = ajStrNewCIL (thys->Ptr, thys->Len, size);
  ajStrDel (pthis);
  *pthis = ret;

  return;
}

/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section String Destructors ********************************************
**
** Destruction is achieved by reducing the reference counter.
** When it reaches zero there are no further client pointers and
** the string is freed from memory.
**
** No other routine is allowed to change the Use value for a string.
**
** There is a special case where the string turns out to be the
** original null string object used to initialise every new string.
** This will not be deleted by a well-behaved program but there is
** no way to guarantee this in this library.
**
******************************************************************************/

/* @func ajStrDel *************************************************************
**
** Default destructor for AJAX strings.
** decrements the use count. When it reaches zero, the
** string is removed from memory.
**
** If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [wP] pthis [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [void]
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
** @@
******************************************************************************/

void ajStrDel (AjPStr* pthis) {

  AjPStr thys = pthis ? *pthis : 0;

  if (!pthis) return;
  if (!*pthis) return;

  --thys->Use;
  if (!thys->Use) {		/* any other references? */
    if (thys == strPNULL) {
      ajErr ("Error - trying to delete the null string constant\n");
      thys->Use = NULL_USE;		/* restore the original value */
    }
    else {

      AJFREE (thys->Ptr);		/* free the string */

      strFree += thys->Res;
      strFreeCount++;
      strCount--;

      thys->Res = 0;		/* in case of copied pointers */
      thys->Len = 0;

      AJFREE (*pthis);		/* free the object */
    }
  }

  *pthis = NULL;
  return;
}

/* @func ajStrDelReuse ********************************************************
**
** Destructor for AJAX strings. Strings with a use count of 1
** are kept to avoid freeing and reallocating memory when they are reused.
** The pointer is only deleted for duplicate strings. Memory
** reserved for the string is never deleted and can always be
** reused by the AjPStr that points to it even if this pointer is cleared.
**
** Use for more efficient memory management for static strings.
**
** If the given string is NULL, or a NULL pointer, simply returns.
**
** @param  [wP] pthis [AjPStr*] Pointer to the string to be deleted.
** @return [AjBool] ajTrue if the string still exists,
**                  ajFalse if if was deleted.
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
** @@
******************************************************************************/

AjBool ajStrDelReuse (AjPStr* pthis) {

  AjPStr thys = pthis ? *pthis : 0;

  if (!pthis) return ajFalse;
  if (!*pthis) return ajFalse;

  if (thys->Use == 1) {		/* last reference - clear the string */
    *thys->Ptr = '\0';
    thys->Len = 0;
    return ajTrue;
  }
  else {
    --thys->Use;
    *pthis = NULL;
  }

  return ajFalse;
}

/* @func ajCharFree ***********************************************************
**
** A text string destructor which deallocates a text string.
**
** @param [wP] txt [char*] Text string to be deallocated.
** @return [char*] A NULL pointer.
** @ure The string is freed using free in the C RTL, so it
**      must have been allocated by malloc in the C RTL
** @@
******************************************************************************/

char* ajCharFree (char* txt) {

  AJFREE (txt);
  return NULL;
}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section String Assignments ************************************************
**
** These functions overwrite the string provided as the first argument
** by calling one of the {String destructors} first. A NULL value
** is always acceptable so these functions are often used to
** create new strings by assignment.
**
******************************************************************************/

/* @func ajStrClear ***********************************************************
**
** Makes sure a string has no text. If the string is already empty
** nothing happens. If the string has data, makes sure the string
** is modifiable and sets it to empty.
**
** @param  [wP] pthis [AjPStr*] Pointer to the string to be deleted.
**         The pointer is always deleted.
** @return [AjBool] ajTrue if string was reallocated
** @cre    The default null string must not be deleted. Calling this
**         routine for copied pointers could cause this. An error message
**         is issued and the null string use count is restored.
**
******************************************************************************/

AjBool ajStrClear (AjPStr* pthis) {

  AjBool ret = ajFalse;

  AjPStr thys = pthis ? *pthis : 0;

  if (!pthis) return ret;
  if (!*pthis) return ret;

  if (!thys->Len) return ret;

  ret = ajStrMod(pthis);
  thys = *pthis;

  thys->Ptr[0] = '\0';
  thys->Len = 0;

  return ret;
}

/* @func ajStrSet *************************************************************
**
** Ensures a string is set. If is already has a value, it is left alone.
** Otherwise the default value is used.
**
** @param [wPN] pthis [AjPStr*] Target string which is overwritten.
** @param [rN] str [const AjPStr] Source string object
**        A NULL pointer makes the target NULL too.
** @return [AjBool] ajTrue if string was reallocated
** @cre If both arguments point to the same string object, nothing happens.
** @@
******************************************************************************/

AjBool ajStrSet (AjPStr* pthis, const AjPStr str) {	

  AjBool ret = ajTrue;		/* true if ajStrDup is used */

  if (!pthis)
    *pthis = ajStrDup (str);
  else if (!*pthis)
    *pthis = ajStrDup (str);
  else if (!(*pthis)->Len) {
    ret = ajStrAssS (pthis, str);
  }

  return ret;
}

/* @func ajStrSetC ************************************************************
**
** Ensures a string is set. If is already has a value, it is left alone.
** Otherwise the default value is used.
**
** @param [wPN] pthis [AjPStr*] Target string which is overwritten.
** @param [rN] str [const char*] Source text.
**        A NULL pointer makes the target NULL too.
** @return [AjBool] ajTrue if string was reallocated
** @cre If both arguments point to the same string object, nothing happens.
** @@
******************************************************************************/

AjBool ajStrSetC (AjPStr* pthis, const char* str) {	

  AjBool ret = ajFalse;

  if (!pthis)
    ret = ajStrAssC (pthis, str);
  else if (!*pthis)
    ret = ajStrAssC (pthis, str);
  else if (!(*pthis)->Len)
    ret = ajStrAssC (pthis, str);

  return ret;
}

/* @func ajStrAss *************************************************************
**
** Assignment constructor with copy of a string object
**
** @param [wPN] pthis [AjPStr*] Target string
** @param [rN] str [const AjPStr] Source string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAss (AjPStr* pthis, const AjPStr str) {	

  AjBool ret = ajTrue;		/* always true for now */

  ajStrDel(pthis);		/* we just use the ref count of str */
  if (str)
    *pthis = ajStrDup(str);
  else
    *pthis = ajStrNew();

  return ret;
}

/* @func ajStrAssC ************************************************************
**
** Copy a text string to a string object.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] text [const char*] Source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssC (AjPStr* pthis, const char* text) {

  AjBool ret = ajFalse;
  AjPStr thys = *pthis;

  ajint i = strlen(text);

  ret = ajStrModL (pthis, i+1);
  thys = *pthis;
  thys->Len = i;
  (void) strncpy (thys->Ptr, text, i+1);

  return ret;
}

/* @func ajStrAssS ************************************************************
**
** Copy a string to a string object without using reference count.
**
** Useful where both strings will be separately overwritten later
** so that they can both remain modifiable.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] str [const AjPStr] Source string.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssS (AjPStr* pthis, const AjPStr str) {

  AjBool ret = ajFalse;

  AjPStr thys = *pthis;

  if (!str) {			/* no source string */
    *pthis = ajStrNew();
    return ajTrue;
  }

  ret = ajStrModL (pthis, str->Len+1); /* minimum reserved size, may be more */

  thys = *pthis;
  thys->Len = str->Len;
  (void) strncpy (thys->Ptr, str->Ptr, str->Len+1);

  return ret;
}

/* @func ajStrAssI ************************************************************
**
** Copy a text string to a string object.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] str [const AjPStr] Source text.
** @param [r] i [size_t] Length of source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssI (AjPStr* pthis, const AjPStr str, size_t i) {

  AjBool ret = ajFalse;

  AjPStr thys = *pthis;

  if (i > str->Len)
    i = str->Len;

  ret = ajStrModL (pthis, i+1);
  thys = *pthis;
  thys->Len = i;
  (void) strncpy (thys->Ptr, str->Ptr, i);
  thys->Ptr[i] = '\0';

  return ret;
}

/* @func ajStrAssL ************************************************************
**
** Copy a text string to a string object.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] str [const AjPStr] Source text.
** @param [r] i [size_t] Size of new string.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssL (AjPStr* pthis, const AjPStr str, size_t i) {

  AjBool ret = ajFalse;

  AjPStr thys = *pthis;
  ajint isize = i;

  if (isize <= str->Len)
    isize = str->Len+1;

  ret = ajStrModL (pthis, isize);

  thys = *pthis;
  thys->Len = str->Len;
  (void) strncpy (thys->Ptr, str->Ptr, str->Len);
  thys->Ptr[str->Len] = '\0';

  return ret;
}

/* @func ajStrAssCI ***********************************************************
**
** Copy a text string to a string object.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] txt [const char*] Source text.
** @param [r] ilen [size_t] Length of source text.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssCI (AjPStr* pthis, const char* txt, size_t ilen) {

  AjBool ret = ajFalse;

  AjPStr thys = *pthis;

  ret = ajStrModL (pthis, ilen+1);
  thys = *pthis;
  thys->Len = ilen;
  (void) strncpy (thys->Ptr, txt, ilen);
  thys->Ptr[ilen] = '\0';

  return ret;
}

/* @func ajStrAssCL ***********************************************************
**
** Copy a text string to a string object.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] txt [const char*] Source text.
** @param [r] i [size_t] Space to reserve.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssCL (AjPStr* pthis, const char* txt, size_t i) {

  AjBool ret = ajFalse;

  AjPStr thys = *pthis;
  ajint ilen = strlen(txt);
  ajint isize = i;

  if (ilen >= isize)
    isize = ilen + 1;

  ret = ajStrModL (pthis, isize);
  thys = *pthis;
  thys->Len = ilen;
  if (ilen)
    (void) strncpy (thys->Ptr, txt, ilen);
  thys->Ptr[ilen] = '\0';

  return ret;
}

/* @func ajStrCopy ***********************************************************
**
** Copies a string, using the reference count.
**
** Sets the destination string to NULL if the source string is NULL.
**
** @param [wPN] pthis [AjPStr*] Target string which is overwritten.
** @param [rN] str [const AjPStr] Source string object
**        A NULL pointer makes the target NULL too.
** @return [AjBool] ajTrue if string was reallocated
** @cre If both arguments point to the same string object, nothing happens.
** @@
******************************************************************************/

AjBool ajStrCopy (AjPStr* pthis, const AjPStr str) {	

  AjBool ret = ajTrue;		/* true if ajStrDup is used */

  ajStrDel(pthis);

  if (!str)
    return ret;
  else 
    *pthis = ajStrDup (str);

  return ret;
}

/* @func ajStrCopyC ***********************************************************
**
** Copies a string.
**
** Sets the destination string to NULL if the source string is NULL.
**
** @param [wPN] pthis [AjPStr*] Target string which is overwritten.
** @param [rN] str [const char*] Source string object
**        A NULL pointer makes the target NULL too.
** @return [AjBool] ajTrue if string was reallocated
** @cre If both arguments point to the same string object, nothing happens.
** @@
******************************************************************************/

AjBool ajStrCopyC (AjPStr* pthis, const char* str) {	

  AjBool ret = ajTrue;		/* true if ajStrDup is used */

  if (!str)
    ajStrDel (pthis);
  else 
    ajStrAssC (pthis, str);

  return ret;
}

/* @func ajStrAssSub **********************************************************
**
** Copies a substring to a string object.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] str [const AjPStr] Source string
** @param [r] begin [ajint] start position for substring
** @param [r] end [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssSub (AjPStr* pthis, const AjPStr str, ajint begin, ajint end) {
  ajint ilen;
  ajint ibegin;
  ajint iend;

  ibegin = ajStrPos (str, begin);
  iend = ajStrPosI (str, ibegin, end);
  if(iend == str->Len)
    iend--;
  /*
  if (begin < 0)
    ibegin = str->Len + begin;
  if (end < 0)
    iend = str->Len + end;
  */

  ilen = iend - ibegin + 1;

  return ajStrAssCI (pthis, &str->Ptr[ibegin], ilen);
}

/* @func ajStrAssSubC *********************************************************
**
** Copies a substring to a string object.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] txt [const char*] Source text
** @param [r] begin [ajint] start position for substring
** @param [r] end [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAssSubC (AjPStr* pthis, const char* txt, ajint begin, ajint end) {

  ajint ilen;
  ajint ibegin=begin;
  ajint iend=end;

  if (begin < 0)
    ibegin = strlen(txt) + begin;
  if (end < 0)
    iend = strlen(txt) + end;

  ilen = iend - ibegin + 1;

  return ajStrAssCI (pthis, &txt[begin], ilen);
}

/* @func ajStrFromBool ********************************************************
**
** Converts a Boolean value into a 1-letter string. Can be used to print
** boolean values, but the ajFmt library has better ways.
**
** @param [P] pthis [AjPStr*] String to hold the result.
** @param [r] boule [AjBool] Boolean value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromBool(AjPStr* pthis, AjBool boule) {

  AjBool ret = ajFalse;
  static char bool_y[] = "Y";
  static char bool_n[] = "N";

  if (boule)
    ret = ajStrAssC(pthis,  bool_y);
  else
    ret = ajStrAssC(pthis, bool_n);
  return ret;
}

/* @func ajStrFromInt *********************************************************
**
** Converts an integer value into a string. The string size is set to be
** just large enough to hold the value.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] val [ajint] Integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromInt (AjPStr* pthis, ajint val) {

  AjBool ret = ajFalse;

  ajint i;
  AjPStr thys;

  if (val)
    i = (ajint) log10((double)abs(val)) + 2;
  else
    i = 2;
  if (val < 0) i++;

  ret = ajStrModL(pthis, i);
  thys = *pthis;

  thys->Len = sprintf (thys->Ptr, "%d", val);

  return ret ;
}

/* @func ajStrFromLong ********************************************************
**
** Converts a ajlong integer value into a string. The string size is set to be
** just large enough to hold the value.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] val [ajlong] Long integer value
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromLong (AjPStr* pthis, ajlong val) {

  AjBool ret = ajFalse;

  ajlong i;
  AjPStr thys;

  if (val)
    i = (ajlong) log10((double)abs(val)) + 2;
  else
    i = 2;
  if (val < 0) i++;

  ret = ajStrModL(pthis, i);
  thys = *pthis;

  thys->Len = sprintf (thys->Ptr, "%ld", (long)val);

  return ret;
}

/* @func ajStrFromFloat *******************************************************
**
** Converts a floating point value into a string. The string size is set to be
** just large enough to hold the value.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] val [float] Floating point value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromFloat (AjPStr* pthis, float val, ajint precision) {

  AjBool ret = ajFalse;

  ajint i;
  char fmt[12];
  AjPStr thys;

  ajint ival = abs((ajint) val);

  if (ival)
    i = precision + (ajint) log10((double)ival) + 4;
  else
    i = precision + 4;

  ret = ajStrModL(pthis, i);
  thys = *pthis;

  (void) sprintf (fmt, "%%.%df", precision);
  thys->Len = sprintf (thys->Ptr, fmt, val);

  return ret;
}

/* @func ajStrFromDouble ******************************************************
**
** Converts a double precision value into a string. The string size is set
** to be just large enough to hold the value.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] val [double] Double precision value
** @param [r] precision [ajint] Precision (number of decimal places) to use.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrFromDouble (AjPStr* pthis, double val, ajint precision) {

  AjBool ret = ajFalse;
  ajint i;
  char fmt[12];
  AjPStr thys;

  ajint ival = abs((ajint) val);

  if (ival)
    i = precision + (ajint) log10((double)ival) + 4;
  else
    i = precision + 4;

  ret = ajStrModL(pthis, i);
  thys = *pthis;

  (void) sprintf (fmt, "%%.%df", precision);
  thys->Len = sprintf (thys->Ptr, fmt, val);

  return ret ;
}

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section String Modifiers **************************************************
**
** These functions use the contents of a string and update them.
** If the string is changed the result is always a string with
** a reference count of 1, but in some cases the string will
** remain unchanged.
**
** To be sure a string has a reference count of 1, use a call to
** {ajStrMod} or {ajStrModL}.
**
******************************************************************************/

/* @func ajStrApp *************************************************************
**
** Appends a string object to the end of another string object.
** Uses {ajStrModL} to make sure target string is modifiable.
**
** @param [wPN] pthis [AjPStr*] Target string
** @param [rN] src [const AjPStr] Source string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrApp (AjPStr* pthis, const AjPStr src) {

  AjBool ret = ajFalse;
  AjPStr t;
  
  AjPStr thys = pthis ? *pthis : 0;
  ajint j;

  t = ajStrNewC(src->Ptr);

  if (pthis && *pthis) {
    j = AJMAX (thys->Res, thys->Len+src->Len+1);
    if (j > thys->Res && thys->Res > LONGSTR)
      j = j + j/2;
  }
  else
    j = src->Len+1;

  ret = ajStrModL (pthis, ajRound (j, STRSIZE));
  thys = *pthis;		/* possible new location */

  (void) strncpy (thys->Ptr+thys->Len, t->Ptr, src->Len+1);
  thys->Len += src->Len;

  ajStrDel(&t);
  return ret;
}

/* @func ajStrAppC ************************************************************
**
** Appends a character string to the end of a string object.
** Uses {ajStrModL} to make sure target string is modifiable.
**
** @param [wPN] pthis [AjPStr*] Target string
** @param [rN] txt [const char*] Source text
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppC (AjPStr* pthis, const char* txt) {

  AjBool ret = ajFalse;
  AjPStr t;
  
  AjPStr thys = pthis ? *pthis : 0;
  ajint i, j;

  t = ajStrNewC(txt);
  i = strlen(txt);
  if (pthis && *pthis)
    j = AJMAX (thys->Res, thys->Len+i+1);
  else
    j = i+1;

  ret = ajStrModL (pthis, ajRound (j, STRSIZE));
  thys = *pthis;		/* possible new location */

  (void) strncpy (thys->Ptr+thys->Len, t->Ptr, i+1);
  thys->Len += i;

  ajStrDel(&t);
  
  return ret;
}

/* @func ajStrAppK ************************************************************
**
** Appends a character to the end of a string object.
** Uses {ajStrModL} to make sure target string is modifiable.
**
** @param [wPN] pthis [AjPStr*] Target string
** @param [rN] chr [const char] Source character
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppK (AjPStr* pthis, const char chr) {

  AjBool ret = ajFalse;

  AjPStr thys = pthis ? *pthis : 0;
  ajint j;

  if (pthis && *pthis)
    j = AJMAX (thys->Res, thys->Len+2);
  else
    j = 2;

  ret = ajStrModL (pthis, ajRound (j, STRSIZE));
  thys = *pthis;		/* possible new location */

  *(thys->Ptr+thys->Len) = chr;
  *(thys->Ptr+thys->Len+1) = '\0';
  thys->Len++;

  return ret;
}

/* @func ajStrAppKI ***********************************************************
**
** Appends characters to the end of a string object.
** Equivalent to a repeat count for ajStrAppK.
**
** Uses {ajStrModL} to make sure target string is modifiable.
**
** @param [wPN] pthis [AjPStr*] Target string
** @param [rN] chr [const char] Source character
** @param [r] number [ajint] Repeat count
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppKI (AjPStr* pthis, const char chr, ajint number) {

  AjBool ret = ajFalse;

  AjPStr thys = pthis ? *pthis : 0;
  ajint i;
  ajint j;
  char* cp;

  if (pthis && *pthis)
    j = AJMAX (thys->Res, thys->Len+number+1);
  else
    j = number+1;

  ret = ajStrModL (pthis, ajRound (j, STRSIZE));
  thys = *pthis;		/* possible new location */

  cp = &thys->Ptr[thys->Len];
  for (i=0; i<number; i++) {
    *cp = chr;
    cp++;
  }

  *cp = '\0';
  thys->Len+= number;

  return ret;
}

/* @func ajStrAppSub **********************************************************
**
** Appends a substring to the end of another string object.
** Uses {ajStrModL} to make sure target string is modifiable.
**
** @param [wPN] pthis [AjPStr*] Target string
** @param [rN] src [const AjPStr] Source string
** @param [r] begin [ajint] start position for substring 
** @param [r] end [ajint] end position for substring
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrAppSub (AjPStr* pthis, const AjPStr src, ajint begin, ajint end) {

  ajint ilen;
  ajint ibegin;
  ajint iend;
  AjBool ret = ajFalse;
  AjPStr t;
  
  AjPStr thys = pthis ? *pthis : 0;
  ajint j;

  ibegin = ajStrPos (src, begin);
  iend = ajStrPosI (src, ibegin, end);
  if(iend == src->Len)
    iend--;
  /*
  if (begin < 0)
    ibegin = src->Len + begin;
  if (end < 0)
    iend = src->Len + end;
  */

  ilen = iend - ibegin + 1;

  t = ajStrNewC(src->Ptr);

  if (pthis && *pthis) {
    j = AJMAX (thys->Res, thys->Len+ilen+1);
    if (j > thys->Res && thys->Res > LONGSTR)
      j = j + j/2;
  }
  else
    j = ilen+1;

  ret = ajStrModL (pthis, ajRound (j, STRSIZE));
  thys = *pthis;		/* possible new location */

  (void) strncpy (thys->Ptr+thys->Len, &t->Ptr[ibegin], ilen);
  thys->Len += ilen;

  thys->Ptr[thys->Len] = '\0';

  ajStrDel(&t);
  return ret;
}

/* @func ajStrInsert **********************************************************
**
** Inserts text into a string object at a specified postion.
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] Position where text is to be inserted
** @param [r] insert [const AjPStr] String to be inserted
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @error ajFalse if the insert failed. Currently this happens if
**        pos is negative, but this could be reassigned to a position
**        from the end of the string in future.
** @@
******************************************************************************/
AjBool ajStrInsert(AjPStr* pthis, ajint begin, const AjPStr insert ) {

  return ajStrInsertC(pthis, begin, insert->Ptr);
}

/* @func ajStrInsertC *********************************************************
**
** Inserts text into a string object at a specified postion.
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] Position where text is to be inserted
** @param [r] insert [const char*] Text to be inserted
** @return [AjBool]  ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrInsertC(AjPStr* pthis, ajint begin, const char* insert ) {

  AjBool ret = ajFalse;
  AjPStr thys;
  ajint j = 0;
  ajint y = 0;
  ajint ibegin;
  char* ptr1;
  const char* ptr2;
  ajint len = strlen(insert);

  thys = *pthis;
  if (!thys) {
    (void) ajStrAssCL(pthis, "", len+1);
    thys = *pthis;
  }
  
  ibegin = ajStrPos (thys, begin);
  if( ibegin > thys->Len ) 
    return ret;

  j = thys->Len+len+1;

  if(j > thys->Res) {
    /*    ajDebug("ajStrInsertC res: %d len: %d +len: %d j: %d\n",
	  thys->Res, thys->Len, len, j);*/
    /*    ajDebug("expanding to res: %d\n", ajRound(j, STRSIZE));*/
    ret = ajStrModL(pthis, ajRound(j, STRSIZE));
    /*    ajDebug("expanded to res: %d len: %d\n", (*pthis)->Res, (*pthis)->Len);*/
  }
  else
    ret = ajStrMod (pthis);

  thys = *pthis;                         /* possible new location */

  /* move characters "i" places up to leave place for insertion */

  ptr1 =  &thys->Ptr[thys->Len+len];
  ptr2 =  &thys->Ptr[thys->Len];
  for(y=0; y<=thys->Len-ibegin ; y++) {
    *ptr1 = *ptr2;
    ptr1--;
    ptr2--;
  }

  thys->Len += len;                        /* set the new length */ 
  thys->Ptr[thys->Len] = '\0';         /* ### was Len+1 ### add the end character */

  /* add the new text */ 
  ptr1 = & thys->Ptr[ibegin];
  ptr2 = insert;

  for(y=0; y< len; y++) {
    *ptr1 = *ptr2;
    ptr1++;
    ptr2++;
  }

  return ret;
}


/* @func ajStrTruncate **********************************************************
** 
** Cut down string to N characters
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] Length of required string.
**
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTruncate (AjPStr* pthis, ajint begin) {

  AjBool ret = ajFalse;
  AjPStr thys;
  ajint ibegin;

  ret = ajStrMod (pthis);
  thys = *pthis;

  ibegin = ajStrPos (thys, begin);

  if(thys->Len < ibegin )     
    return ajFalse;

  thys->Ptr[ibegin] = '\0';
  thys->Len = ibegin;

  return ret;
}

/* @func ajStrReplace *********************************************************
** 
** Replace string at pos1 and add len characters from string overwrite.
** Or to the end of the existing string
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] Number of characters of target string to keep.
** @param [r] overwrite [const AjPStr] String to replace.
** @param [r] ilen [ajint] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrReplace( AjPStr* pthis, ajint begin, const AjPStr overwrite, ajint ilen) {

  return ajStrReplaceC(pthis, begin, overwrite->Ptr, ilen);
}

/* @func ajStrReplaceC ********************************************************
** 
** Replace string at pos1 and add len characters from string overwrite.
** Or to the end of the existing string
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] Number of characters of target string to keep.
** @param [r] overwrite [const char*] String to replace.
** @param [r] ilen [ajint] Number of characters to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrReplaceC ( AjPStr* pthis, ajint begin, const char* overwrite,
		       ajint ilen) {

  AjPStr thys;
  ajint ibegin;
  ajint iend;
  char* ptr1 = 0;
  const char* ptr2 = 0;
  ajint len = strlen(overwrite);

  (void) ajStrMod (pthis);
  thys = *pthis;

  ibegin = ajStrPos (thys, begin);
  iend = ibegin + ilen;

  if((iend  > thys->Len) || (ilen > len) ) /* can't fit */
    return ajFalse;

  ptr1 = &thys->Ptr[ibegin];
  ptr2 = overwrite;    
  
  for(;ilen>0;ilen--)
    *ptr1++ = *ptr2++;

  return ajTrue;
}

/* @func ajStrJoin ************************************************************
** 
** Cut down string at pos1 and add string2 from position pos2.
**
** @param [uP] pthis [AjPStr*] Target string.
** @param [r] begin [ajint] Number of characters to keep in target string.
** @param [r] addbit [const AjPStr] String to append.
** @param [r] begin2 [ajint] Position of first character to copy from text.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrJoin (AjPStr* pthis, ajint begin, const AjPStr addbit, ajint begin2) {
  ajint ibegin2;

  ibegin2 = ajStrPosI (addbit, 0, begin2);

  return ajStrJoinC(pthis, begin, addbit->Ptr, ibegin2);
}

/* @func ajStrJoinC ***********************************************************
** 
** Cut down string at pos1 and add string2 from position pos2.
**
** @param [uP] pthis [AjPStr*] Target string.
** @param [r] begin [ajint] Number of characters to keep in target string.
** @param [r] addbit [const char*] Text to append.
** @param [r] ibegin2 [ajint] Position of first character to copy from text.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrJoinC (AjPStr* pthis, ajint begin, const char* addbit,
		   ajint ibegin2) {

  AjPStr thys;
  ajint len = strlen(addbit);
  ajint ibegin;
  ajint i = 0;
  ajint j = 0;
  ajint newlen = 0;
  
  (void) ajStrMod (pthis);
  thys = *pthis;

  ibegin = ajStrPos (thys, begin);

  if(thys->Len < ibegin || len < ibegin2)     
    return ajFalse;

  newlen = ibegin + len - ibegin2 + 1;

  if(newlen > thys->Res) {
    (void) ajStrModL(pthis, ajRound(j, STRSIZE));
    thys = *pthis;
  }

  for(i=ibegin,j=ibegin2; j <= len; i++,j++)
    thys->Ptr[i] = addbit[j];

  thys->Len = i-1; 
  
  return ajTrue;
}

/* @func ajStrSubstitute ****************************************************
**
** Replace all occurrences of replace with putin in string pthis.
**
** @param [uP] pthis [AjPStr*]  Target string.
** @param [r]  replace [const AjPStr] string to replace.
** @param [r]  putin [const AjPStr]   string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrSubstitute (AjPStr *pthis,
			const AjPStr replace, const AjPStr putin){

  AjBool ret = ajFalse;
  AjPStr thys;
  AjBool cycle = ajTrue;
  ajint pos = 0;

  ret = ajStrMod (pthis);
  thys = *pthis;

  if(replace->Len !=0){
    while(cycle){
      pos = ajStrFindC(thys, replace->Ptr);
      if(pos >= 0){
	(void) ajStrCut(&thys,pos,pos+replace->Len-1); 
	(void) ajStrInsert(&thys,pos,putin);
      }
      else
	cycle = ajFalse;
    }
  }
  *pthis = thys;

  return ret;
}

/* @func ajStrSubstituteCC ****************************************************
**
** Replace all occurrences of replace with putin in string pthis.
**
** @param [uP] pthis [AjPStr*]  Target string.
** @param [r]  replace [const char*] string to replace.
** @param [r]  putin [const char*]   string to insert.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrSubstituteCC(AjPStr *pthis, const char* replace,
			 const char* putin){

  AjBool ret = ajFalse;
  AjPStr thys;
  AjBool cycle = ajTrue;
  ajint pos = 0;

  ret = ajStrMod (pthis);
  thys = *pthis;

  if(strlen(replace) !=0){
    while(cycle){
      pos = ajStrFindC(thys, replace);
      if(pos >= 0){
	(void) ajStrCut(&thys,pos,pos+strlen(replace)-1); 
	(void) ajStrInsertC(&thys,pos,putin);
      }
      else
	cycle = ajFalse;
    }
  }
  *pthis = thys;

  return ret;
}
  
/* @func ajStrChomp ***********************************************************
** 
** Remove start and end white space chars from the String.
**
** @param [uP] pthis [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrChomp (AjPStr* pthis){

  AjBool ret = ajFalse;
  static AjPStr spaces = NULL;

  if (!spaces)
    (void) ajStrAssC(&spaces,"\t \n");
    
  ret = ajStrMod (pthis);
  (void) ajStrTrimC(pthis, spaces->Ptr);

  return ret;
}

/* @func ajStrChompC **********************************************************
** 
** Remove start and end white space chars from the String.
**
** @param [uP] pthis [AjPStr*] String
** @param [r] delim [char*] delimiter(s)
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrChompC (AjPStr* pthis, char* delim){

  AjBool ret = ajFalse;

  ret = ajStrMod (pthis);
  (void) ajStrTrimC(pthis, delim);

  return ret;
}

/* @func ajStrChop ************************************************************
**
** Removes the last character from a string
**
** @param [uP] pthis [AjPStr*] string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrChop (AjPStr* pthis) {

  AjBool ret = ajFalse;
  AjPStr thys;

  ret = ajStrMod (pthis);
  thys = *pthis;

  if (thys->Len) {
    thys->Ptr[--thys->Len] = '\0';
  }

  return ret;
}

/* @func ajStrTrim ************************************************************
**
** Removes the characters from the start or end of a string
**
** @param [uP] pthis [AjPStr*] string
** @param [r] num [ajint] Number of characters to delete for the start (if
**            positive) or the end (if negative)
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrim (AjPStr* pthis, ajint num) {

  AjBool ret = ajFalse;
  AjPStr thys;

  ret = ajStrMod (pthis);
  thys = *pthis;

  if (num > 0) {
    if(num > thys->Len)
      return ajFalse; 
    (void) memmove (thys->Ptr, &thys->Ptr[num], thys->Len - num);
    thys->Len -= num;
    thys->Ptr[thys->Len] = '\0';
  }
  else if (num < 0) {
    if((-num) > thys->Len)
      return ajFalse; 
    thys->Len += num;
    thys->Ptr[thys->Len] = '\0';
  }

  return ret;
}

/* @func ajStrTrimC ***********************************************************
**
** Removes a set of characters from the start and end of a string
**
** @param [uP] pthis [AjPStr*] string
** @param [r] chars [const char*] Characters to delete from each end
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrTrimC (AjPStr* pthis, const char* chars) {

  AjBool ret = ajFalse;
  AjPStr thys;
  const char* cp;
  ajint i;

  if (!pthis) return ret;
  if (!*pthis) return ret;

  ret = ajStrMod (pthis);
  thys = *pthis;

  cp = thys->Ptr;
  i = strspn(cp, chars);
  if (i) {
    thys->Len -= i;
    if (thys->Len)
      (void) memmove (thys->Ptr, &thys->Ptr[i], thys->Len);
    else
      *thys->Ptr = '\0';
  }
  if (i)
    thys->Ptr[thys->Len] = '\0';

  if (!thys->Len) return ret;  /* changed to ! (il) 10/2/98 */
                           /* if there is still something there, then */
                           /* see if the end needs trimming */

  cp = &thys->Ptr[thys->Len-1];
  i = 0;
  while (strchr(chars, *cp)) {
    thys->Len--;
    cp--;
    i++;
  }
  if (i) thys->Ptr[thys->Len] = '\0';

  return ret;
}

/* @func ajStrCut *************************************************************
**
** Cut out a range of characters from a string.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] start position to be cut
** @param [r] end [ajint] end position to be cut
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
** @@
******************************************************************************/

AjBool ajStrCut (AjPStr* pthis, ajint begin, ajint end) {
  AjPStr thys;
  ajint ilen;
  ajint ibegin;
  ajint iend;

  (void) ajStrMod (pthis);
  thys = *pthis;

  ibegin = ajStrPos (thys, begin);
  iend = ajStrPosI (thys, ibegin, end+1);
  ilen = iend - ibegin;


  ajDebug ("ajStrCut %d %d len: %d ibegin: %d iend: %d\n",
	   begin, end, thys->Len, ibegin, iend);

  if (iend <= ibegin) return ajFalse;

  if (ibegin < thys->Len) {
    (void) memmove (&thys->Ptr[ibegin], &thys->Ptr[iend], thys->Len - iend);
    thys->Len -= ilen;
    thys->Ptr[thys->Len] = '\0';
    return ajTrue;
  }

  return ajFalse;
}

/* @func ajStrMask *********************************************************
**
** Mask out a range of characters from a string.
**
** @param [wP] pthis [AjPStr*] Target string
** @param [r] begin [ajint] start position to be masked
** @param [r] end [ajint] end position to be masked
** @param [r] maskchar [char] masking character
** @return [AjBool] ajTrue on success, ajFalse if begin is out of range
** @@
******************************************************************************/

AjBool ajStrMask (AjPStr* pthis, ajint begin, ajint end, char maskchar) {
  AjPStr thys;
  ajint ibegin;
  ajint iend;
  ajint i;

  (void) ajStrMod (pthis);
  thys = *pthis;

  ibegin = ajStrPos (thys, begin);
  iend = ajStrPosI (thys, ibegin, end+1);

  ajDebug ("ajStrMask %d %d len: %d ibegin: %d iend: %d char '%c'\n",
	   begin, end, thys->Len, ibegin, iend, maskchar);

  if (iend < ibegin) return ajFalse;

  for (i=ibegin; i<iend; i++)
    thys->Ptr[i] = maskchar;

  return ajTrue;
}

/* @func ajStrRev *************************************************************
**
** Reverses the order of characters in a string
**
** @param [wP] pthis [AjPStr*] Target string
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrRev (AjPStr* pthis) {

  AjBool ret = ajFalse;
  char *cp;
  char *cq;
  char tmp;

  ret = ajStrMod (pthis);

  cp = ajStrStr(*pthis);
  cq = cp + ajStrLen(*pthis) - 1;

  while (cp < cq) {
    tmp = *cp;
    *cp = *cq;
    *cq = tmp;
    cp++;
    cq--;
  }

  return ret;
}


/* @func ajStrRandom ************************************************************
**
** Returns randomised string in place
** NB: application should have already called ajRandomSeed() somewhere
**
** @param [rw] s [AjPStr *] string
**
** @return [void]
** @@
******************************************************************************/
void ajStrRandom(AjPStr *s)
{
    AjPStr copy=NULL;
    char *p;
    char *q;
    
    ajint *rn=NULL;
    ajint *na=NULL;

    ajint len;
    ajint i;

    ajStrAssS (&copy, *s);
    p=ajStrStr(copy);
    q=ajStrStr(*s);

    len = ajStrLen(*s);
    AJCNEW (na, len);
    AJCNEW (rn, len);

    for(i=0;i<len;++i)
    {
	na[i]=i;
	rn[i]=ajRandomNumber();
    }
    ajSortIntDecI(rn,na,len);
    
    for(i=0;i<len;++i)
	q[i]=p[na[i]];

    AJFREE (na);
    AJFREE (rn);
    ajStrDel(&copy);

    (void) ajStrMod(s);
    return;
}


/* @func ajStrSub *************************************************************
**
** Reduces target string to a substring of itself.
**
** The end is allowed to be before begin, in which case the output is an
** empty string.
**
** @param [wP] pthis [AjPStr*] Target string.
** @param [r] begin [ajint] Start position for substring.
** @param [r] end [ajint] End position for substring.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrSub (AjPStr* pthis, ajint begin, ajint end) {

  AjBool ret = ajFalse;
  AjPStr thys;
  ajint ibegin, ilen, iend;

  ret = ajStrMod (pthis);
  thys = *pthis;

  ibegin = ajStrPos (thys, begin);
  iend = ajStrPos (thys, end);

  if(iend == thys->Len)
    iend--;
  if (iend < ibegin) ilen = 0;
  else ilen = iend - ibegin + 1;

  if (ilen) {
    if (ibegin) {
      (void) ajMemMove (thys->Ptr, &thys->Ptr[ibegin], ilen);
    }
    thys->Len = ilen;
    thys->Ptr[ilen] = '\0';
  }
  else {
    thys->Len = 0;
    thys->Ptr[0] = '\0';
  }

  return ret;
}

/* @func ajStrMod *************************************************************
**
** Make certain a string is modifiable by checking it has no
** other references, or by making a new real copy of the string.
**
** Uses strClone to copy without copying the reference count.
** 
** The target string is guaranteed to have a reference count of exactly 1.
**
** @param [wP] pthis [AjPStr*] String
** @return [AjBool] ajTrue if the string was reallocated
** @@
******************************************************************************/

AjBool ajStrMod (AjPStr* pthis) { 

  AjBool ret = ajFalse;

  AjPStr thys = pthis ? *pthis : 0;
  if (!*pthis) {
    thys = *pthis = ajStrNew();
    ret = ajTrue;
  }

  if (thys->Use > 1) {
    strClone (pthis);
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrModL ************************************************************
**
** Make certain that a string is modifiable, and big enough for its
** intended purpose.
** 
** The target string is guaranteed to have a reference count of 1,
** and a minimum reserved size.
**
** @param [wP] pthis [AjPStr*] String
** @param [r] size [size_t] Minimum reserved size.
** @return [AjBool] ajTrue if the string was reallocated
** @@
******************************************************************************/

AjBool ajStrModL (AjPStr* pthis, size_t size) { 

  AjBool ret = ajFalse;

  AjPStr thys = pthis ? *pthis : 0;
  size_t savesize = size;	/* often part of *pthis, about to vanish */

  if (!thys) {
    thys = *pthis = ajStrNewL(savesize);
    ret = ajTrue;
  }

  if ((thys->Use > 1 || thys->Res < savesize)) {
    strCloneL (pthis, savesize);
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrConvert *********************************************************
**
** Replaces one set of characters with another set
**
** @param [wP] pthis [AjPStr*] String
** @param [r] oldc [const AjPStr] Unwanted characters
** @param [r] newc [const AjPStr] Replacement characters
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrConvert (AjPStr* pthis,
		     const AjPStr oldc, const AjPStr newc) {
  return ajStrConvertCC (pthis, oldc->Ptr, newc->Ptr);
}

/* @func ajStrConvertCC ********************************************
**
** Replaces one set of characters with another set
**
** @param [wP] pthis [AjPStr*] String
** @param [r] oldc [const char*] Unwanted characters
** @param [r] newc [const char*] Replacement characters
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrConvertCC (AjPStr* pthis, const char* oldc, const char* newc) {

  AjBool ret = ajFalse;
  char filter[256] = {'\0'};	/* should make all zero */
  ajint i;

  const char *co = oldc;
  const char *cn = newc;
  char* cp;

  ret = ajStrMod(pthis);

  i = strlen(newc);
  if(strlen(oldc) > i){
      ajErr ("ajStrConvertCC new character set '%s' shorter than old '%s'",
	     oldc, newc);
      ajErr ("Will only convert the first %d chars",i);
  }

  while (i) {
    if (!*cn){
      ajErr (" SHOULD NEVER SEE THIS "
	     "StrConvertCC new character set '%s' shorter than old '%s'",
	     oldc, newc);
    }
    else
      filter[(ajint)*co++] = *cn++;
    i--;
  }

  for (cp = (*pthis)->Ptr; *cp; cp++) {
    if (filter[(ajint)*cp])
      *cp = filter[(ajint)*cp];
  }
  return ret;
}

/* @func ajStrToLower *********************************************************
**
** Converts a string to lower case. If the string has multiple references,
** a new string is made first.
**
** @param [uP] pthis [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrToLower (AjPStr* pthis) {

  AjBool ret = ajFalse;
  AjPStr thys;

  ret = ajStrMod (pthis);
  thys = *pthis;
  ajCharToLower(thys->Ptr);

  return ret;
}

/* @func ajCharToLower ********************************************************
**
** Converts a text string to lower case.
**
** @param [uP] txt [char*] Text string
** @return [void]
** @@
******************************************************************************/

void ajCharToLower (char* txt)
{
  char* cp = txt;
  while (*cp)
  {
  /*
   *  AJB: The ajSysItoC function was there as some really fussy compilers
   *  complained about casting ajint to char. However, for conversion of
   *  large databases its too much of an overhead. Think about a macro
   *  later. In the meantime revert to the standard system call
   *    *cp = ajSysItoC(tolower((ajint) *cp));
   */
    *cp = (char)tolower((ajint) *cp);
    cp++;
  }

  return;
}

/* @func ajStrToUpper *********************************************************
**
** Converts a string to upper case. If the string has multiple references,
** a new string is made first.
**
** @param [uP] pthis [AjPStr*] String
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrToUpper (AjPStr* pthis) {

  AjBool ret = ajFalse;
  AjPStr thys;

  ret = ajStrMod (pthis);
  thys = *pthis;
  ajCharToUpper(thys->Ptr);

  return ret;
}

/* @func ajCharToUpper ********************************************************
**
** Converts a text string to upper case.
**
** @param [uP] txt [char*] Text string
** @return [void]
** @@
******************************************************************************/

void ajCharToUpper (char* txt)
{
  char* cp = txt;
  while (*cp)
  {
  /*
   *  AJB: The ajSysItoC function was there as some really fussy compilers
   *  complained about casting ajint to char. However, for conversion of
   *  large databases its too much of an overhead. Think about a macro
   *  later. In the meantime revert to the standard system call
   *    *cp = ajSysItoC(toupper((ajint) *cp));
   */
    *cp = (char) toupper((ajint) *cp);
    cp++;
  }

  return;
}

/* @func ajStrClean ***********************************************************
**
** Remove excess whitespace from a string
**
** Leading/trailing whitespace removed. Multiple spaces replaced by
** singles spaces.
**
** @param [rw] s [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrClean(AjPStr *s) {

  AjBool ret = ajFalse;
  static AjPStr t = NULL;
  ajint i=0;
  ajint j=0;
  ajint len;
  char *p;

  (void) ajStrAssS (&t,*s);		/* make a buffer in t */
    
  p=ajStrStr(t);
  len=strlen(p);

  for(i=0;i<len;++i) if(p[i]=='\t') p[i]=' ';

  i=0;
  while(1) {
    if(!p[i]) break;
    if(p[i]!=' ') break;
    ++i;
  }
  (void) strcpy(p,&p[i]);

  len=strlen(p);
  if(p[len-1]=='\n') {
    p[len-1]='\0';
    --len;
  }
    
  for(i=len-1;i>-1;--i)
    if(p[i]!=' ') break;
  p[i+1]='\0';

  len=strlen(p);
    
  for(i=j=0;i<len;++i) {
    if(p[i]!=' ') {
      p[j++]=p[i];
      continue;
    }
    p[j++]=' ';

    for(++i;;++i)
      if(p[i]!=' ') {
	p[j++]=p[i];
	break;
      }
  }
    
  p[j]='\0';

  ret = ajStrAssC(s,p);

  return ret;
}

/* @func ajStrCleanWhite *****************************************************
**
** Removes all whitespace from a string
**
** @param [rw] s [AjPStr *] String to clean.
** @return [AjBool] ajTrue if string was reallocated
** @@
******************************************************************************/

AjBool ajStrCleanWhite(AjPStr *s) {

  AjBool ret = ajFalse;
  static AjPStr t = NULL;
  ajint i=0;
  ajint j=0;
  ajint len;
  char *p;
  
  (void) ajStrAssS (&t,*s);		/* make a buffer in t */
    
  p=ajStrStr(t);
  len=ajStrLen(t);

  for(i=0;i<len;++i)
      if(p[i]=='\t' || p[i]=='\n')
	  p[i]=' ';
  for(i=0;i<len;++i)
  {
      if(p[i]!=' ')
      {
	  p[j++]=p[i];
      }
      else
	  --t->Len;
  }
  p[j]='\0';

  ret = ajStrAssC(s,p);

  return ret;
}

/* @func ajStrBlock ***********************************************************
**
** Splits string into words (blocks) of a given size.
**
** Mainly intended for sequence ouptut formats
**
** @param [u] pthis [AjPStr*] String.
** @param [r] blksize [ajint] Block size
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajStrBlock (AjPStr* pthis, ajint blksize) {
  ajint i;
  char* cp;
  char* cq;
  AjPStr thys;
  ajint j;

  i = (*pthis)->Len + ((*pthis)->Len-1)/blksize;
  (void) ajStrModL (pthis, i+1);
  thys = *pthis;

  ajDebug ("ajStrBlock len: %d blksize: %d i: %d\n", thys->Len, blksize, i); 

  cp = &thys->Ptr[thys->Len];
  cq = &thys->Ptr[i];
  for (j=thys->Len-1; j; j--) {
    *(--cq) = *(--cp);
    if (!(j%blksize)) *(--cq) = ' ';
  }
  thys->Ptr[i]='\0';
  thys->Len = i;

  ajStrTrace (thys);
  ajDebug ("result '%S'\n", thys);

  return ajTrue;
}

/* @func ajStrFill ************************************************************
**
** Pad a string with extra characters at the end
**
** @param [u] pthys [AjPStr*] String.
** @param [r] len [ajint] Final length
** @param [r] fill [char] Characters to use for padding
** @return [void]
** @@
******************************************************************************/
void ajStrFill (AjPStr* pthys, ajint len, char fill) {

  ajint i;
  AjPStr thys;

  ajStrModL (pthys, len+1);

  thys = *pthys;

  if (thys->Len >= len) return;

  for (i=thys->Len; i < len; i++)
    thys->Ptr[i] = fill;

  thys->Ptr[len] = '\0';
  thys->Len = len;

  return;
}

/* @func ajStrFix *************************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] thys [const AjPStr] String.
** @return [void]
** @@
******************************************************************************/

void ajStrFix (const AjPStr thys) {
  if (thys->Use > 1)
    ajWarn ("ajStrFix called for string in use %d times\n", thys->Use);
  thys->Len = strlen(thys->Ptr);

  return;
}

/* @func ajStrFixI ************************************************************
**
** Reset string length when some nasty caller may have edited it
**
** @param [u] thys [const AjPStr] String
** @param [r] ilen [ajint] Length expected.
** @return [void]
** @@
******************************************************************************/

void ajStrFixI (const AjPStr thys, ajint ilen) {
  if (thys->Use > 1) {
    ajDebug ("ajStrFixI called for string in use %d times\n'%*S'\n",
	     thys->Use, thys->Res, thys);
    ajWarn ("ajStrFixI called for string in use %d times\n", thys->Use);
  }

  if (ilen >= thys->Res) {
    ajWarn("ajStrFixI called with length %d for string with size %d\n",
	   ilen, thys->Res);
    thys->Ptr[thys->Res-1] = '\0';
    ajDebug("ajStrFixI called with length %d for string with size %d\n'%S'\n",
	   ilen, thys->Res, thys);
    ilen = strlen(thys->Ptr);
  }
  if (ilen < 0) {
    ajWarn("ajStrFixI called with negative length %d\n", ilen);
    thys->Ptr[thys->Res-1] = '\0';
    ilen = strlen(thys->Ptr);
  }

  thys->Ptr[ilen] = '\0';

  thys->Len = ilen;

  return;
}


/* @func ajStrFixTestI ********************************************************
**
** Reset string length when some nasty caller may have edited it
** and issue warnings if the length was not correct.
**
** @param [u] thys [const AjPStr] String
** @param [r] ilen [ajint] Length expected.
** @return [void]
** @@
******************************************************************************/

void ajStrFixTestI (const AjPStr thys, ajint ilen) {
  if (thys->Use > 1)
    ajWarn ("ajStrFixTestI called for string in use %d times\n", thys->Use);

  if (ilen > thys->Res) {
    ajWarn("ajStrFixTestI called with length %d for string with size %d\n",
	   ilen, thys->Res);
    ilen = strlen(thys->Ptr);
  }
  if (ilen < 0) {
    ajWarn("ajStrFixTestI called with negative length %d\n", ilen);
    ilen = strlen(thys->Ptr);
  }

  if (thys->Ptr[ilen]) {
    ajWarn("ajStrFixTestI called with length %d but this is not end of string\n",
	   ilen);
    ajWarn("... true length %d\n",
	   strlen(thys->Ptr));
    ajDebug("ajStrFixTestI called with length %d but this is not end of string\n",
	   ilen);
    ajDebug("... true length %d\n",
	   strlen(thys->Ptr));
    thys->Ptr[ilen] = '\0';
    ajStrTrace(thys);
  }

  thys->Len = ilen;

  return;
}


/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section String Operators ********************************************
**
** These functions use the contents of a string but do not make any changes.
**
** They include string comparisons. See also {String Casts}.
**
******************************************************************************/

/* @func ajStrFindC ***********************************************************
** 
** Locates the first occurrence in the string of the second string.
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindC (const AjPStr thys, const char* text) {

  const char* cp;
  cp = strstr (thys->Ptr, text);
  if (!cp) return -1;
  return (cp - thys->Ptr);
}

/* @func ajStrFind ***********************************************************
** 
** Locates the first occurrence in the string of the second string.
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFind (const AjPStr thys, const AjPStr text)
{
  const char* cp;
  cp = strstr (thys->Ptr, text->Ptr);
  if (!cp) return -1;
  return (cp - thys->Ptr);
}

/* @func ajStrFindCaseC *******************************************************
** 
** Locates the first occurrence in the string of the second string.
** Case insensitive
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const char*] text to find
** @return [ajint] Position of the start of text in string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindCaseC (const AjPStr thys, const char *text) {

    AjPStr t1;
    AjPStr t2;
    ajint v;
    
    t1 = ajStrNewC(thys->Ptr);
    t2 = ajStrNewC(text);
    ajStrToUpper(&t1);
    ajStrToUpper(&t2);

    v = ajStrFind(t1,t2);
    ajStrDel(&t1);
    ajStrDel(&t2);

    return v;
}

/* @func ajStrFindCase *******************************************************
** 
** Locates the first occurrence in the string of the second string.
** Case insensitive
**
** @param [r] thys [const AjPStr] String
** @param [r] text [AjPStr] text to find
** @return [ajint] Position of the start of text in string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrFindCase (const AjPStr thys, AjPStr text) {

    return ajStrFindCaseC(thys,text->Ptr);
}

/* @func ajStrRFindC **********************************************************
** 
** Locates the last occurrence in the string of the second text string.
**
** @param [r] thys [const AjPStr] String to search
** @param [r] text [const char*] text to look for
** @return [ajint] Position of the text string if found.
** @error -1 Text not found.
** @@
******************************************************************************/

ajint ajStrRFindC (const AjPStr thys, const char* text) {

  ajint i = 0;
  ajint j = 0;
  ajint len = 0;
  const char* ptr1 = 0;
  const char* ptr2 = 0;
  ajint found = ajTrue;

  len = strlen(text);

  for(i=thys->Len-len;i>=0;i--) {
    ptr1 = &thys->Ptr[i];
    ptr2 = text;
    found = ajTrue;
    for(j=0;j<len;j++) {
      if(*ptr1 != *ptr2) {
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
/* @func ajStrCmp ************************************************************
** 
** Compares the value of two strings for use in sorting (e.g. ajListSort)
**
** @param [r] str1 [const void*] First string
** @param [r] str2 [const void*] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmp (const void* str1, const void* str2) {

  return strcmp((*(AjPStr*)str1)->Ptr, (*(AjPStr*)str2)->Ptr);
}


/* @func ajStrCmpO ************************************************************
** 
** Compares the value of two strings for use in sorting (e.g. ajListSort)
**
** @param [r] thys [const AjPStr] First string
** @param [r] anoth [const AjPStr] Second string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpO (const AjPStr thys, const AjPStr anoth) {

  return strcmp(thys->Ptr, anoth->Ptr);
}

/* @func ajStrCmpC ************************************************************
** 
** Compares the value a string object and a character string
**
** @param [r] thys [const AjPStr] String object
** @param [r] text [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpC (const AjPStr thys, const char* text) {

  return strcmp(thys->Ptr, text);
}

/* @func ajStrNCmpO ***********************************************************
** 
** Compares the first n characters of two strings
**
** @param [r] thys [const AjPStr] String object
** @param [r] anoth [const AjPStr] Second string object
** @param [r] n [ajint] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrNCmpO (const AjPStr thys, const AjPStr anoth, ajint n) {

  return strncmp(thys->Ptr, anoth->Ptr, n);
}

/* @func ajStrNCmpC ***********************************************************
** 
** Compares the first n characters of a string object and a text string
**
** @param [r] thys [const AjPStr] String object
** @param [r] text [const char*] Text string
** @param [r] n [ajint] Length to compare
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrNCmpC (const AjPStr thys, const char* text, ajint n) {

  return strncmp(thys->Ptr, text, n);
}


/* @func ajStrNCmpCaseCC ******************************************************
** 
** Compares the value of a string and another
**
** @param [r] str1 [const char*] Text string
** @param [r] str2 [const char*] Text string
** @param [r] len  [ajint] length
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrNCmpCaseCC (const char* str1, const char* str2, ajint len)
{

    const char* cp;
    const char* cq;
    ajint i;
  
    for(cp=str1,cq=str2,i=0;*cp && *cq && i<len;++i,++cp,++cq)
	if (toupper((ajint) *cp) != toupper((ajint) *cq))
	{
	    if (toupper((ajint) *cp) > toupper((ajint) *cq)) return 1;
	    else return -1;
	}
    if(i==len) return 0;
    if (*cp) return 1;

    return -1;
}


/* @func ajStrCmpCase *********************************************************
** 
** Compares the value of two string objects
**
** @param [r] str1 [const AjPStr] text string
** @param [r] str2 [const AjPStr] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpCase (const AjPStr str1, const AjPStr str2) {

  const char* cp;
  const char* cq;

  for (cp = str1->Ptr, cq = str2->Ptr; *cp && *cq; cp++, cq++) {
    if (toupper((ajint) *cp) != toupper((ajint) *cq)) {
      if (toupper((ajint) *cp) > toupper((ajint) *cq)) return 1;
      else return -1;
    }
  }

  if (*cp) return 1;
  if (*cq) return -1;
  return 0;

}

/* @func ajStrCmpCaseCC *******************************************************
** 
** Compares the value a string object and a character string
**
** @param [r] str1 [const char*] Text string
** @param [r] str2 [const char*] Text string
** @return [int] -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

int ajStrCmpCaseCC (const char* str1, const char* str2) {

  const char* cp;
  const char* cq;

  for (cp = str1, cq = str2; *cp && *cq; cp++, cq++) {
    if (toupper((ajint) *cp) != toupper((ajint) *cq)) {
      if (toupper((ajint) *cp) > toupper((ajint) *cq)) return 1;
      else return -1;
    }
  }

  if (*cp) return 1;
  if (*cq) return -1;
  return 0;

}

/* @func ajStrMatch ***********************************************************
**
** Simple test for matching string and text.
**
** @param [r] thys [const AjPStr] String
** @param [r] str [const AjPStr] Second String
** @return [AjBool] ajTrue if two complete strings are the same
** @@
******************************************************************************/

AjBool ajStrMatch (const AjPStr thys, const AjPStr str) {

  if (!thys || !str)
    return ajFalse;

  if (!strcmp (thys->Ptr, str->Ptr))
    return ajTrue;

  return ajFalse;
}

/* @func ajStrMatchC **********************************************************
**
** Simple test for matching string and text.
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const char*] Text
** @return [AjBool] ajTrue if two complete strings are the same
** @@
******************************************************************************/

AjBool ajStrMatchC (const AjPStr thys, const char* text) {

  if (!thys || !text)
    return ajFalse;

  if (!strcmp (thys->Ptr, text))
    return ajTrue;

  return ajFalse;
}

/* @func ajStrMatchCC *********************************************************
**
** Simple test for matching two text values.
**
** @param [r] thys [const char*] String
** @param [r] text [const char*] Text
** @return [AjBool] ajTrue if Text completely matches the start of String
** @@
******************************************************************************/

AjBool ajStrMatchCC (const char* thys, const char* text) {
  if (!thys || !text)
    return ajFalse;

  if (!strcmp (thys, text))
    return ajTrue;

  return ajFalse;
}

/* @func ajStrMatchCase *******************************************************
**
** Simple case insensitive test for matching string and text.
**
** @param [r] thys [const AjPStr] String
** @param [r] str [const AjPStr] Second String
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCase (const AjPStr thys, const AjPStr str) {

  if (!thys || !str)
    return ajFalse;

  return ajStrMatchCaseCC (thys->Ptr, str->Ptr);
}

/* @func ajStrMatchCaseC ******************************************************
**
** Simple case insensitive test for matching string and text.
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCaseC (const AjPStr thys, const char* text) {

  if (!thys || !text)
    return ajFalse;

  return ajStrMatchCaseCC (thys->Ptr, text);
}

/* @func ajStrMatchCaseCC *****************************************************
**
** Simple case insensitive test for matching two text values.
**
** @param [r] thys [const char*] String
** @param [r] text [const char*] Text
** @return [AjBool] ajTrue if two strings are exactly the same excluding case
** @@
******************************************************************************/

AjBool ajStrMatchCaseCC (const char* thys, const char* text) {

  const char* cp = thys;
  const char* cq = text;

  if (!*cp || !*cq)
    return ajFalse;

  while (*cp && *cq) {
    if (tolower((ajint) *cp++) != tolower((ajint) *cq++)) return ajFalse;
  }
  if (*cp || *cq)
    return ajFalse;

  return ajTrue;
}

/* @func ajStrMatchWild *******************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] thys [const AjPStr] String
** @param [r] wild [const AjPStr] Wildcard string
** @return [AjBool] ajTrue if two strings match
** @@
******************************************************************************/

AjBool ajStrMatchWild (const AjPStr thys, const AjPStr wild) {

  return ajStrMatchWildCC (thys->Ptr, wild->Ptr);
}

/* @func ajStrMatchWildC ******************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const char*] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildC (const AjPStr thys, const char* text) {

  return ajStrMatchWildCC (thys->Ptr, text);
}

/* @func ajStrMatchWildCC *****************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] str [const char*] String
** @param [r] text [const char*] Text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildCC (const char* str, const char* text) {
  ajint i;

  i = ajStrCmpWildCC(str, text);
  if (i)
    return ajFalse;

  return ajTrue;
}

/* @func ajStrCmpWild *******************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] thys [const AjPStr] String
** @param [r] wild [const AjPStr] Wildcard string
** @return [ajint]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

ajint ajStrCmpWild (const AjPStr thys, const AjPStr wild) {

  return ajStrCmpWildCC (thys->Ptr, wild->Ptr);
}

/* @func ajStrCmpWildC ******************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] thys [const AjPStr] String
** @param [r] text [const char*] Wildcard text
** @return [ajint]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

ajint ajStrCmpWildC (const AjPStr thys, const char* text) {

  return ajStrCmpWildCC (thys->Ptr, text);
}

/* @func ajStrCmpWildCC *****************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] str [const char*] String
** @param [r] text [const char*] Text
** @return [ajint]  -1 if first string should sort before second, +1 if the
**         second string should sort first. 0 if they are identical
**         in length and content.
** @@
******************************************************************************/

ajint ajStrCmpWildCC (const char* str, const char* text) {

  const char* cp = text;
  const char* cq = str;

  /*ajDebug("ajStrCmpWildCC('%s', '%s')\n", str, text);*/

  if (!*cp && !*cq) return 0;
  if (!*cp) return -1;

  /*ajDebug("something to test, continue...\n");*/

  while (*cp) {
    if (!*cq && *cp != '*')
      return 1;

    switch (*cp) {
    case '?':			/* skip next character and continue */
      cp++;
      cq++;
      break;
    case '*':
      cp++;
      if (!*cp) {
	/* ajDebug ("...matches at end +%d '%s' +%d '%s'\n",
	   (cq - str), cq, (cp - text), cp);*/
	return 0;	/* just match the rest */
      }
      if (!*cq) {
	/*ajDebug ("...test match to null string just in case\n");*/
	return ajStrCmpWildCC (cq, cp);
      }
      while (*cq) {		/* wildcard in mid name, look for the rest */
        if (ajStrMatchWildCC (cq, cp)) return 0; /* recursive + repeats */
	/* ajDebug ("...'*' at +%d '%s' +%d '%s' continuing\n",
	   (cq - str), cq, (cp - text), cp);*/
	cq++;
      }
      return 1;

      /* always returns once '*' is found */

    default:			/* for all other characters, keep checking */
      if (tolower((ajint) *cp) != tolower((ajint) *cq)) {
	if (tolower((ajint) *cp) > tolower((ajint) *cq))
	  return -1;
	else
	  return 1;
      }
      cp++;
      if (*cq)
	cq++;
    }
  }
  /*ajDebug ("...done comparing at +%d '%s' +%d '%s'\n",
    (cq - str), cq, (cp - text), cp);*/
  if (*cp) {
    /*ajDebug ("...incomplete cp, FAILED\n");*/
    return -1 ;
  }
  if (*cq) {
    /*ajDebug ("...incomplete cq, FAILED\n");*/
    return 1;
  }
  /*ajDebug ("...all finished and matched\n");*/

  return 0;
}


/* @func ajStrMatchWildCO *****************************************************
**
** Simple case insensitive test for matching a wildcard value.
**
** @param [r] str [const char*] String
** @param [r] wild [const AjPStr] Wildcard text
** @return [AjBool] ajTrue if the strings match
** @@
******************************************************************************/

AjBool ajStrMatchWildCO (const char* str, const AjPStr wild) {

  return ajStrMatchWildCC (str, wild->Ptr);
}

/* @func ajStrWildPrefix *****************************************************
**
** Tests for wildcard characters and terminates the string at the
** first wild character (if any).
**
** @param [r] str [AjPStr*] String
** @return [AjBool] ajTrue if the string contained a wildcard and was
**                  truncated.
** @@
******************************************************************************/

AjBool ajStrWildPrefix (AjPStr* str) {

  char* cp;

  (void) ajStrMod(str);
  cp = ajStrStr(*str);

  while (*cp) {
    switch (*cp) {
    case '?':
    case '*':
      *cp = '\0';
      ajStrFix(*str);
      return ajTrue;
    default:
      cp++;
    }
  }

  return ajFalse;
}

/* @func ajStrIsWild **********************************************************
**
** Tests whether a string contains standard wildcard characters * or ?
**
** @param [r] thys [const AjPStr] String
** @return [AjBool] ajTrue if string has widlcards.
** @@
******************************************************************************/

AjBool ajStrIsWild (const AjPStr thys) {
  static AjPRegexp wildexp = NULL;
  if (!wildexp)
    wildexp = ajRegCompC ("([*?])");
  return ajRegExec(wildexp, thys);
}

/* @func ajStrCheck ***********************************************************
**
** Checks a string object for consistency. Intended for debugging and testing
** of these routines, but made publicly available.
**
** @param [r] thys [const AjPStr] String
** @return [AjBool] ajTrue if no errors were found.
** @@
******************************************************************************/

AjBool ajStrCheck (const AjPStr thys) {

  AjBool ret=ajTrue;

  if (!thys) {
    ajErr ("ajStrCheck: NULL string pointer");
    ret = ajFalse;
  }

  if (thys->Use < 0) {
    ajErr ("ajStrCheck: Bad use value %d", thys->Use);
    ret = ajFalse;
  }

  if (thys->Res < 1) {
    ajErr ("ajStrCheck: Bad size value %d", thys->Res);
    ret = ajFalse;
  }

  if (thys->Len < 0) {
    ajErr ("ajStrCheck: Bad length value %d\n", thys->Len);
    ret = ajFalse;
  }

  if (thys->Len >= thys->Res) {
    ajErr ("ajStrCheck: Size %d too small for length %d\n",
	   thys->Res, thys->Len);
    ret = ajFalse;
  }

  if (!thys->Ptr) {
    ajErr ("ajStrCheck: NULL pointer\n");
    ret = ajFalse;
  }
  else {
    if (thys->Len != strlen(thys->Ptr)) {
      ajErr ("ajStrCheck: Len %d differs from strlen %d\n",
	     thys->Len, strlen(thys->Ptr));
      ret = ajFalse;
    }
      }


  return ret;
}

/* @func ajStrTraceT **********************************************************
**
** Checks a string object and reports its contents. Title provided by caller
**
** @param [r] thys [const AjPStr] String
** @param [r] title [char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajStrTraceT (const AjPStr thys, char* title) {

  ajDebug("%s\n",title);
  ajStrTrace(thys);

  return;
}

/* @func ajStrTrace ***********************************************************
**
** Checks a string object and reports its contents.
**
** @param [r] thys [const AjPStr] String
** @return [void]
** @@
******************************************************************************/

void ajStrTrace (const AjPStr thys)
{

    if (!thys)
    {
	ajDebug("String trace NULL\n");
	return;
    }
  
    (void) ajStrCheck (thys);

    ajDebug("String trace use: %d size: %d len: %d string: ",
	    thys->Use, thys->Res, thys->Len);
  
    if (thys->Ptr)
    {
	if (thys->Len <= 20)
	    ajDebug("<%s>\n", thys->Ptr);
	else
	    ajDebug("<%10.10s>..<%s>\n",
		    thys->Ptr, thys->Ptr + thys->Len-10);
    }
    else
	ajDebug("<NULL>\n");

    ajDebug("             ptr: %x    charptr: %x\n",
	    thys, thys->Ptr);

    return;
}

/* @func ajStrStat ************************************************************
**
** Prints a summary of string usage with debug calls
**
** @param [r] title [char*] Title for this summary
** @return [void]
** @@
******************************************************************************/

void ajStrStat (char* title) {

  static ajlong statAlloc = 0;
  static ajlong statCount = 0;
  static ajlong statFree = 0;
  static ajlong statFreeCount = 0;
  static ajlong statTotal= 0;

  ajDebug ("String usage statistics since last call %s:\n", title);
  ajDebug ("String usage (bytes): %ld allocated, %ld freed\n",
	   strAlloc - statAlloc, strFree - statFree);
  ajDebug ("String usage (number): %ld allocated, %ld freed, %ld in use\n",
	   strTotal - statTotal, strFreeCount - statFreeCount,
	   strCount - statCount);

  statAlloc = strAlloc;
  statCount = strCount;
  statFree = strFree;
  statFreeCount = strFreeCount;
  statTotal = strTotal;

  return;
}


/* @func ajStrExit ************************************************************
**
** Prints a summary of string usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajStrExit (void) {

  ajDebug ("String usage (bytes): %ld allocated, %ld freed, %ld in use\n",
	   strAlloc, strFree, strAlloc - strFree);
  ajDebug ("String usage (number): %ld allocated, %ld freed %ld in use\n",
	   strTotal, strFreeCount, strCount);

  return;
}


/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section String Casts ********************************************
**
** These functions examine the contents of a string and return some
** derived information. Some of them provide access to the internal
** components of a string. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/


/* @macro MAJSTRSTR *******************************************************
**
** A macro version of {ajStrStr} available in case it is needed for speed.
**
** @param [r] thys [AjPStr] Source string
** @return [char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/

/* @func ajStrStr *************************************************************
**
** Returns the current Cstring pointer. This will remain valid unless
** the string is resized or deleted.
**
** @param [r] thys [const AjPStr] Source string
** @return [char*] Current string pointer, or a null string if undefined.
** @@
******************************************************************************/

char* ajStrStr (const AjPStr thys) {
  if (!thys)
    return charNULL;

  return thys->Ptr;
}

/* @macro MAJSTRLEN *******************************************************
**
** A macro version of {ajStrLen} available in case it is needed for speed.
**
** @param [r] thys [AjPStr] Source string
** @return [ajint] Current string length
** @@
******************************************************************************/

/* @func ajStrLen *************************************************************
**
** Returns the current Cstring length. This will remain valid unless
** the string is resized or deleted.
**
** @param [r] thys [const AjPStr] Source string
** @return [ajint] Current string length
** @@
******************************************************************************/

ajint ajStrLen (const AjPStr thys) {
  if (!thys)
    return 0;

  return thys->Len;
}

/* @macro MAJSTRSIZE *******************************************************
**
** A macro version of {ajStrSize} available in case it is needed for speed.
**
** @param [r] thys [AjPStr] Source string
** @return [ajint] Current string reserved size
** @@
******************************************************************************/

/* @func ajStrSize ************************************************************
**
** Returns the current Cstring reserved size. This will remain valid unless
** the string is resized or deleted.
**
** @param [r] thys [const AjPStr] Source string
** @return [ajint] Current string reserved size
** @@
******************************************************************************/

ajint ajStrSize (const AjPStr thys) {
  if (!thys)
    return 0;

  return thys->Res;
}

/* @macro MAJSTRREF *******************************************************
**
** A macro version of {ajStrRef} available in case it is needed for speed.
**
** @param [r] thys [AjPStr] Source string
** @return [ajint] Current string usage count
** @@
******************************************************************************/

/* @func ajStrRef *************************************************************
**
** Returns the current Cstring usage count. This will remain valid unless
** the string is resized or deleted.
**
** @param [r] thys [const AjPStr] Source string
** @return [ajint] Current string usage count
** @@
******************************************************************************/

ajint ajStrRef (const AjPStr thys) {
  if (!thys)
    return 0;

  return thys->Use;
}

/* @func ajStrBool ************************************************************
**
** Converts a Boolean value into a text string. Can be used to print
** boolean values, but the ajFmt library has better ways.
**
** @param [r] boule [AjBool] Boolean value
** @return [char*] "True" or "False"
** @@
******************************************************************************/

char* ajStrBool (AjBool boule) {
  static char bool_true[] = "True";
  static char bool_false[] = "False";

  if (boule)
    return bool_true;

  return bool_false;
}

/* @func ajStrYN **************************************************************
**
** Converts a Boolean value into a 1-letter string. Can be used to print
** boolean values, but the ajFmt library has better ways.
**
** @param [r] boule [AjBool] Boolean value
** @return [char*] "Y" or "N"
** @@
******************************************************************************/

char* ajStrYN (AjBool boule) {
  static char bool_y[] = "Y";
  static char bool_n[] = "N";

  if (boule)
    return bool_y;

  return bool_n;
}

/* @func ajStrTok *************************************************************
**
** Simple token parsing using white space.
** 
** @param [r] thys [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @return [AjPStr] Token
** @error NULL if no further token is found.
** @@
******************************************************************************/

AjPStr ajStrTok (const AjPStr thys) {

  return ajStrTokC (thys, " \t\n\r");

}

/* @func ajStrTokC ************************************************************
**
** Simple token parsing using specified set of delimiters.
** 
** @param [r] thys [const AjPStr] String to be parsed (first call) or
**        NULL for followup calls using the same string, as for the
**        C RTL function strtok which is eventually called.
** @param [r] delim [const char*] Delimiter(s) to be used betwen tokens.
** @return [AjPStr] Token
** @error NULL if no further token is found.
** @@
******************************************************************************/

AjPStr ajStrTokC (const AjPStr thys, const char* delim) {

  static AjPStr strp=0;		/* internal AjPStr - do not try to destroy */
  static char* cp = NULL;

  if (!strp) {
    if (!thys) {
      ajWarn("Error in ajStrTokC: NULL argument and not initialised");
      return NULL;
    }
    strp = ajStrNewL(thys->Res);
    AJFREE(strp->Ptr);
  }

  if (thys) {
    if (cp) (void) ajCharFree(cp);
    cp = ajCharNewC(thys->Len, thys->Ptr);
    strp->Ptr = ajSysStrtok (cp, delim);
  }
  else {
    strp->Ptr = ajSysStrtok (NULL, delim);
  }

  if (strp->Ptr) {
    strp->Len = strlen(strp->Ptr);
    strp->Res = strp->Len + 1;
    return strp;
  }
  else {
      strp->Len=0;
  }

  return NULL;
}

/* @func ajStrIsBool **********************************************************
**
** Simple test for a string having a valid Boolean value.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a boolean.
** @cre an empty string always returns false.
** @see ajStrToBool
** @@
******************************************************************************/

AjBool ajStrIsBool (const AjPStr thys) {

  char* cp = ajStrStr(thys);

  if (!thys->Len) return ajFalse;

  if (!strchr("YyTt1NnFf0", *cp)) return ajFalse;

  return ajTrue;
}

/* @func ajStrIsHex ***********************************************************
**
** Simple test for a string having a valid hexadecimal value, using the strtol
** call in the C RTL.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a hexadecimal value.
** @cre an empty string always returns false.
** @@
******************************************************************************/

AjBool ajStrIsHex (const AjPStr thys) {

  char* cp = ajStrStr(thys);
  char* ptr = NULL;

  if (!thys->Len) return ajFalse;

  errno = 0;
  (void) strtol (cp, &ptr, 16);
  if (*ptr || errno == ERANGE)
    return ajFalse;

  return ajTrue;
}

/* @func ajStrIsInt ***********************************************************
**
** Simple test for a string having a valid integer value, using the strtol
** call in the C RTL.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as an integer.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsInt (const AjPStr thys) {

  char* cp = ajStrStr(thys);
  char* ptr = NULL;

  if (!thys->Len) return ajFalse;

  errno = 0;
  (void) strtol (cp, &ptr, 10);
  if (*ptr || errno == ERANGE)
    return ajFalse;

  return ajTrue;
}

/* @func ajStrIsLong **********************************************************
**
** Simple test for a string having a valid ajlong integer value, using the strtol
** call in the C RTL.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as an integer.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsLong (const AjPStr thys) {

  char* cp = ajStrStr(thys);
  char* ptr = NULL;

  if (!thys->Len) return ajFalse;

  errno = 0;
  (void) strtol (cp, &ptr, 10);
  if (*ptr || errno == ERANGE)
    return ajFalse;

  return ajTrue;
}

/* @func ajStrIsFloat *********************************************************
**
** Simple test for a string having a valid floating point value,
** using the strtod call in the C RTL.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a floating
**         point number.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsFloat (const AjPStr thys) {

  char* cp = ajStrStr(thys);
  char* ptr = NULL;
  double d;

  if (!thys->Len) return ajFalse;

  errno = 0;
  d = strtod (cp, &ptr);
  if (*ptr || errno == ERANGE)
    return ajFalse;
  if (d > FLT_MAX) return ajFalse;
  if (d < -FLT_MAX) return ajFalse;

  return ajTrue;
}

/* @func ajStrIsDouble ********************************************************
**
** Simple test for a string having a valid double precision value,
** using the strtod call in the C RTL.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is acceptable as a double
**         precision number.
** @cre an empty string always returns false.
** @see ajStrTokIntBool
** @@
******************************************************************************/

AjBool ajStrIsDouble (const AjPStr thys) {

  char* cp = ajStrStr(thys);
  char* ptr = NULL;

  if (!thys->Len) return ajFalse;

  errno = 0;
  (void) strtod (cp, &ptr);
  if (*ptr || errno == ERANGE)
    return ajFalse;

  return ajTrue;
}

/* @func ajStrToBool **********************************************************
**
** Converts a string into a Boolean value.
**
** @param [r] thys [const AjPStr] String
** @param [w] result [AjBool*] ajTrue if the string is "true" as a boolean.
** @return [AjBool] ajTrue if the string had a valid boolean value.
** @cre an empty string returns ajFalse.
** @see ajStrIsBool
** @@
******************************************************************************/

AjBool ajStrToBool (const AjPStr thys, AjBool* result) {

  AjBool ret=ajFalse;
  char* cp = ajStrStr(thys);
  ajint i;

  *result = ajFalse;

  if (thys->Len < 1)
    return ajFalse;

  if (strchr("YyTt1", *cp)) {
    *result = ajTrue;
    ret = ajTrue;
  }
  else if (strchr("NnFf", *cp)) {
    *result = ajFalse;
    ret = ajTrue;
  }
  else if (strchr("0+", *cp)) {
    i = strcspn(cp, "123456789"); /* e.g. 0.1, 0007 */
    if (cp[i])
      *result = ajTrue;
    else
      *result = ajFalse;
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrToHex ***********************************************************
**
** Converts a string from hexadecimal into an integer value,
** using the strtol call in the C RTL.
**
** @param [r] thys [const AjPStr] String
** @param [w] result [ajint*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid hexadecimal value.
** @cre an empty string returns ajFalse.
** @see ajStrIsHex
** @@
******************************************************************************/

AjBool ajStrToHex (const AjPStr thys, ajint* result) {

  AjBool ret=ajFalse;
  char* cp = ajStrStr(thys);
  ajlong l;
  char* ptr;

  *result = 0;
  if (!thys->Len) return ret;

  errno = 0;
  l = strtol (cp, &ptr, 16);
  if (!*ptr && errno != ERANGE) {
    l = AJMAX(INT_MIN, l);
    l = AJMIN(INT_MAX, l);
    *result = (ajint) l;
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrToInt ***********************************************************
**
** Converts a string into an integer value, using the strtol call
** in the C RTL.
**
** @param [r] thys [const AjPStr] String
** @param [w] result [ajint*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToInt (const AjPStr thys, ajint* result) {

  AjBool ret=ajFalse;
  char* cp = ajStrStr(thys);
  ajlong l;
  char* ptr;

  *result = 0;
  if (!thys->Len) return ret;

  errno = 0;
  l = strtol (cp, &ptr, 10);
  if (!*ptr && errno != ERANGE) {
    l = AJMAX(INT_MIN, l);
    l = AJMIN(INT_MAX, l);
    *result = (ajint) l;
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrToLong **********************************************************
**
** Converts a string into an integer value, using the strtol call
** in the C RTL.
**
** @param [r] thys [const AjPStr] String
** @param [w] result [ajlong*] String represented as an integer.
** @return [AjBool] ajTrue if the string had a valid integer value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToLong (const AjPStr thys, ajlong* result) {

  AjBool ret=ajFalse;
  char* cp = ajStrStr(thys);
  ajlong l;
  char* ptr;

  *result = 0;
  if (!thys->Len) return ret;

  errno = 0;
  l = strtol (cp, &ptr, 10);
  if (!*ptr && errno != ERANGE) {
    *result = l;
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrToFloat *********************************************************
**
** Converts a string into a floating point value, using the strtod call
** in the C RTL.
**
** @param [r] thys [const AjPStr] String
** @param [w] result [float*] String represented as a floating point number.
** @return [AjBool] ajTrue if the string had a valid floating point value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToFloat (const AjPStr thys, float* result) {

  AjBool ret=ajFalse;
  char* cp = ajStrStr(thys);
  double d;
  char* ptr = NULL;

  *result = 0.0;
  if (!thys->Len)
    return ret;

  errno = 0;
  d = strtod (cp, &ptr);
  if (!*ptr  && errno != ERANGE) {
    if (d > FLT_MAX) return ajFalse;
    if (d < -FLT_MAX) return ajFalse;
    *result = (float) d;
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrToDouble ********************************************************
**
** Converts a string into a double precision value, using the strtod call
** in the C RTL.
**
** @param [r] thys [const AjPStr] String
** @param [w] result [double*] String represented as a double precision number.
** @return [AjBool] ajTrue if the string had a valid double precision value.
** @cre an empty string returns ajFalse.
** @see ajStrIsInt
** @@
******************************************************************************/

AjBool ajStrToDouble (const AjPStr thys, double* result) {

  AjBool ret=ajFalse;
  char* cp = ajStrStr(thys);
  double d;
  char* ptr = NULL;

  *result = 0.0;
  if (!thys->Len)
    return ret;

  errno = 0;
  d = strtod (cp, &ptr);
  if (!*ptr  && errno != ERANGE) {
    *result = d;
    ret = ajTrue;
  }

  return ret;
}

/* @func ajStrTokenCount ******************************************************
**
** Returns the number of tokens in a string
**
** @param [r] line [AjPStr*] String to examine.
** @param [r] delim [const char *] String of delimiter characters.
** @return [ajint] The number of tokens
** @@
******************************************************************************/

ajint ajStrTokenCount(AjPStr* line, const char *delim)
{
    static AjPStrTok t = NULL;
    static AjPStr tmp = NULL;
    
    ajint count;

    count=0;
    (void) ajStrTokenAss (&t, *line, delim);

    while(ajStrToken(&tmp, &t, delim)) ++count;
    
    return count;
}

/* @func ajStrRoom *******************************************************
**
** Returns the additional space available in a string.
**
** @param [r] thys [const AjPStr] String
** @return [ajint] Space available for additional characters.
** @@
******************************************************************************/

ajint ajStrRoom (const AjPStr thys) {

  return (thys->Res - thys->Len - 1);
}

/* @func ajStrPos *************************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** @param [wP] thys [const AjPStr] Target string.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 0 and length.
** @@
******************************************************************************/

ajint ajStrPos (const AjPStr thys, ajint ipos) {

  return ajStrPosI (thys, 0, ipos);
}

/* @func ajStrPosI ************************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [wP] thys [const AjPStr] Target string.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 0 and length.
** @@
******************************************************************************/

ajint ajStrPosI (const AjPStr thys, ajint imin, ajint ipos) {

  return ajStrPosII (thys->Len, imin, ipos);
}

/* @func ajStrPosII ***********************************************************
**
** Converts a position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [r] ilen [ajint] maximum length.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 0 and length.
** @@
******************************************************************************/

ajint ajStrPosII (ajint ilen, ajint imin, ajint ipos) {
  ajint jpos;

  if (ipos < 0)
    jpos = ilen + ipos;
  else
    jpos = ipos;

  if (jpos > ilen)
    jpos = ilen;

  if (jpos < imin)
    jpos = imin;

  return jpos;
}

/* @func ajCharPos ************************************************************
**
** converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [wP] thys [const char*] Target string.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 0 and length.
** @@
******************************************************************************/

ajint ajCharPos (const char* thys, ajint ipos) {
  ajint jpos;
  ajint len = strlen(thys);

  if (ipos < 0)
    jpos = len + ipos;
  else
    jpos = ipos;

  if (jpos > len)
    jpos = len;

  if (jpos < 0)
    jpos = 0;

  return jpos;
}

/* ==================================================================== */
/* ======================== Iterators ==================================*/
/* ==================================================================== */

/* @section String Iterators ********************************************
**
** Iterators provide a means for callers to loop through a string,
** returning successive matches until no more can be found.
**
** The string iterators are a test case for the design and development
** of more complex iterators such as for lists and tables.
**
******************************************************************************/

/* @func ajStrIter ****************************************************
**
** Creates an iterator over the characters in a string.
**
** @param [wP] thys [const AjPStr] Original string
** @return [AjIStr] String Iterator
** @@
******************************************************************************/

AjIStr ajStrIter (const AjPStr thys)
{
  AjIStr iter;
  
  AJNEW0(iter);
  iter->Start = iter->Ptr = ajStrStr(thys);
  iter->End = iter->Start + ajStrLen(thys) - 1;
  
  return iter;

}

/* @func ajStrIterBack *******************************************************
**
** Creates an iterator over the characters in a string set to end of string.
**
** @param [wP] thys [const AjPStr] Original string
** @return [AjIStr] String Iterator
** @@
******************************************************************************/

AjIStr ajStrIterBack (const AjPStr thys)
{
  AjIStr iter;
  
  AJNEW0(iter);
  iter->Start = ajStrStr(thys);
  iter->End = iter->Ptr = iter->Start + ajStrLen(thys) - 1;
  
  return iter;

}

/* @macro ajStrIterBegin ******************************************************
**
** Start condition for a string iterator.
**
** @param [P] iter [AjIStr] String iterator.
** @return [AjIStr] Begin
** @@
******************************************************************************/

/* @func ajStrIterNext ********************************************************
**
** Step to next character in string iterator.
**
** @param [P] iter [AjIStr] String iterator.
** @return [AjIStr] Updated iterator duplicated as return value.
** @@
******************************************************************************/

AjIStr ajStrIterNext (AjIStr iter)
{
    
    iter->Ptr++;
    if(iter->Ptr > iter->End)
	return NULL;

    return iter;
}

/* @func ajStrIterBackNext ****************************************************
**
** Step to previous character in string iterator.
**
** @param [P] iter [AjIStr] String iterator.
** @return [AjIStr] Updated iterator duplicated as return value.
** @@
******************************************************************************/

AjIStr ajStrIterBackNext (AjIStr iter)
{

    
    iter->Ptr--;

    if(iter->Ptr < iter->Start)
	return NULL;

    return iter;
}

/* @macro ajStrIterEnd ******************************************************
**
** Stop condition for a string iterator.
**
** @param [P] iter [AjIStr] String iterator.
** @return [AjIStr] End
** @@
******************************************************************************/

/* @macro ajStrIterGetC ******************************************************
**
** Current value for a string iterator.
**
** @param [P] iter [AjIStr] String iterator.
** @return [char*] Current text string within iterator
** @@
******************************************************************************/

/* @macro ajStrIterGetK ******************************************************
**
** Current value for a string iterator.
**
** @param [P] iter [AjIStr] String iterator.
** @return [char] Current character within iterator
** @@
******************************************************************************/

/* @func ajStrIterFree*********************************************************
**
** Deletes a string iterator
**
** @param [P] iter [AjIStr*] String iterator
** @return [void]
** @@
******************************************************************************/

void ajStrIterFree (AjIStr* iter)
{

    AJFREE(*iter);

    return;
}

/* ==================================================================== */
/* ======================== AjPStrTok ==================================*/
/* ==================================================================== */

/* ==================================================================== */
/* ======================== Constructors ===============================*/
/* ==================================================================== */

/* @section String Token Constructors ****************************************
**
** There is one constructor so far for string tokens. It already accepts
** a delimiter string of NULL for the default so there seems to be no need
** to add more functions.
**
******************************************************************************/


/* @func ajStrTokenInit *******************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters.
**
** The string is reference counted, so this copy takes up no extra memory.
** @param [r] thys [const AjPStr] Source string
** @param [r] delim [const char*] Default delimiter(s)
** @return [AjPStrTok] A new string token parser.
** @@
******************************************************************************/

AjPStrTok ajStrTokenInit (const AjPStr thys, const char* delim) {

  static AjPStrTok ret;

  AJNEW0(ret);

  ret->String = ajStrDup (thys);
  if (delim) {
    ret->Delim = ajStrNewC (delim);
    /*ret->Pos = strspn(ret->String->Ptr, ret->Delim->Ptr);*/
    ret->Pos = 0;		/* GFF parsing needed this change */
  }
  else {
    ret->Delim = ajStrNew();
    ret->Pos = 0;
  }

  return ret;
}

/* ==================================================================== */
/* ========================= Destructors ============================== */
/* ==================================================================== */

/* @section String Token Destructors ****************************************
**
** There is one destructor so far for string tokens.
**
******************************************************************************/

/* @func ajStrTokenClear ******************************************************
**
** Destructor for a string token parser.
**
** @param [wP] token [AjPStrTok*] Token parser
** @return [void]
** @@
******************************************************************************/

void ajStrTokenClear (AjPStrTok* token) {

  if (!*token) return;

  ajStrDel (&(*token)->String);
  ajStrDel (&(*token)->Delim);
  AJFREE (*token);

  return;
}

/* ==================================================================== */
/* =========================== Assignment ============================== */
/* ==================================================================== */

/* @section String Token Assignments ******************************************
**
** The functions in this section use a String Token to control the
** parsing of a string.
**
******************************************************************************/

/* @func ajStrTokenAss *******************************************************
**
** Generates a string token parser object from a string and an optional
** set of default delimiters.
**
** The string token can be either an existing token to be overwritten
** or a NULL.
**
** The string is reference counted, so this copy takes up no extra memory.
** @param [w] ptok [AjPStrTok*] String token object
** @param [r] thys [const AjPStr] Source string
** @param [r] delim [const char*] Default delimiter(s)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajStrTokenAss (AjPStrTok* ptok, const AjPStr thys, const char* delim) {

  AjPStrTok tok;

  if (*ptok)
    tok = *ptok;
  else {
    AJNEW0(tok);
    *ptok = tok;
  }

  (void) ajStrAss (&tok->String, thys);
  if (delim) {
    (void) ajStrAssC (&tok->Delim, delim);
    tok->Pos = strspn(tok->String->Ptr, tok->Delim->Ptr);
  }
  else {
    (void) ajStrAssC(&tok->Delim, "");
    tok->Pos = 0;
  }

  return ajTrue;
}

/* @func ajStrTokenReset ******************************************************
**
** Clears the strings from a string token object.
**
** The string token can be either an existing token to be overwritten
** or a NULL.
**
** The string is reference counted, so this copy takes up no extra memory.
** @param [w] ptok [AjPStrTok*] String token object
** @return [void]
** @@
******************************************************************************/

void ajStrTokenReset (AjPStrTok* ptok) {

  AjPStrTok tok;

  if (!ptok)
    return;
  if (!*ptok)
    return;

  tok = *ptok;

  (void) ajStrDelReuse(&tok->String);
  (void) ajStrDelReuse(&tok->Delim);

  return;
}

/* @func ajStrTokenTrace ******************************************************
**
** Writes a debug trace of a string token object.
**
** @param [r] tok [AjPStrTok] String token object
** @return [void]
** @@
******************************************************************************/

void ajStrTokenTrace (AjPStrTok tok) {

  ajDebug ("ajStrTokenTrace %x\n", tok);

  if (!tok)
    return;

  ajDebug ("... String:\n");
  ajStrTrace(tok->String);
  ajDebug ("... Delim:\n");
  ajStrTrace(tok->Delim);

  return;
}

/* ==================================================================== */
/* =========================== Operators ============================== */
/* ==================================================================== */

/* @section String Token Operators ********************************************
**
** The functions in this section use a String Token to control the
** parsing of a string.
**
******************************************************************************/

/* @func ajStrToken ***********************************************************
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test used the C function 'strcspn'.
**
** @param [P] pthis [AjPStr*] Next token returned.
** @param [P] ptoken [AjPStrTok*] String token parsing object.
** @param [P] delim [const char*] Delimiter character set.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajStrToken (AjPStr* pthis, AjPStrTok* ptoken, const char* delim) {

  ajint ilen;
  AjPStrTok token = *ptoken;
  char* cp;

  if (!*ptoken) {		/* token already cleared */
    (void) ajStrAssC (pthis, "");
    return ajFalse;
  }

  if (token->Pos >= token->String->Len) { /* all done */
    (void) ajStrAssC (pthis, "");
    ajStrTokenClear(ptoken);
    return ajFalse;
  }

  if (delim)
    (void) ajStrAssC (&token->Delim, delim);

  cp = &token->String->Ptr[token->Pos];
  ilen = strcspn(cp, token->Delim->Ptr);

  if (ilen)
    (void) ajStrAssSub (pthis, token->String,
			token->Pos, token->Pos + ilen - 1);
  else
    (void) ajStrAssC (pthis, "");

  token->Pos += ilen;
  token->Pos += strspn(&token->String->Ptr[token->Pos], token->Delim->Ptr);

  return ajTrue;
}

/* @func ajStrTokenRest *******************************************************
**
** Returns the remainder of a partially parsed string.
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** The test used the C function 'strcspn'.
**
** @param [P] pthis [AjPStr*] Next token returned.
** @param [P] ptoken [AjPStrTok*] String token parsing object.
** @return [AjBool] ajTrue if success.
** @@
******************************************************************************/

AjBool ajStrTokenRest (AjPStr* pthis, AjPStrTok* ptoken) {

  AjPStrTok token = *ptoken;

  if (!*ptoken) {		/* token already cleared */
    (void) ajStrAssC (pthis, "");
    return ajFalse;
  }

  if (token->Pos >= token->String->Len) { /* all done */
    (void) ajStrAssC (pthis, "");
    ajStrTokenClear(ptoken);
    return ajFalse;
  }

  if (token->Pos < token->String->Len)
    (void) ajStrAssSub (pthis, token->String, token->Pos, token->String->Len);
  else
    (void) ajStrAssC (pthis, "");

  token->Pos = token->String->Len;

  return ajTrue;
}

/* @func ajStrDelim ***********************************************************
**
** Treats "delim" as a string to find (unlike Token which treats it as
** a character set)
**
** Note: This can return "true" but an empty string in cases where the
** delimiter has changed since the previous call.
**
** @param [wP] pthis [AjPStr*] Token found
** @param [r] ptoken [AjPStrTok*] Token parser. Updated with the delimiter
**        string (if any) in delim.
** @param [rN] delim [const char*] Optional delimiter string. If NULL, the
**        delimiters in ptoken are used.
**
** @return [AjBool] ajTrue if another token was found.
** @@
******************************************************************************/

AjBool ajStrDelim (AjPStr* pthis, AjPStrTok* ptoken, const char* delim) {

  ajint ilen;
  AjPStrTok token = *ptoken;
  char* cp;
  char* cq;

  if (!*ptoken) {		/* token already cleared */
    (void) ajStrAssC (pthis, "");
    return ajFalse;
  }

  if (token->Pos >= token->String->Len) { /* all done */
    (void) ajStrAssC (pthis, "");
    ajStrTokenClear(ptoken);
    return ajFalse;
  }

  if (delim)
    (void) ajStrAssC (&token->Delim, delim);

  cp = &token->String->Ptr[token->Pos];
  cq = strstr(cp, token->Delim->Ptr);
  if (cq) {
    ilen = cq - cp;
    (void) ajStrAssSub (pthis, token->String, token->Pos,
			token->Pos + ilen - 1);
    token->Pos += ilen;
    token->Pos += token->Delim->Len;
    return ajTrue;
  }

  /* delimiter not found - return rest of string */

  ilen = token->String->Len - token->Pos;
  (void) ajStrAssCI (pthis, cp, ilen);
  token->Pos += ilen;

  return ajTrue;
}


/* ==================================================================== */
/* ========================= Reporters =================================*/
/* ==================================================================== */

/* @func ajStrPrefix **********************************************************
**
** Tests the start of a string against a given prefix string.
**
** @param [r] thys [const AjPStr] String
** @param [r] pref [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefix (const AjPStr thys, const AjPStr pref) {

  if (!pref->Len)		/* no prefix */
    return ajFalse;

  if (pref->Len > ajStrLen(thys))	/* pref longer */
      return ajFalse;

  if (strncmp(thys->Ptr, pref->Ptr, pref->Len)) /* +1 or -1 for a failed match */
    return ajFalse;

  return ajTrue;
}

/* @func ajStrPrefixC *********************************************************
**
** Tests the start of a string against a given prefix text.
**
** @param [r] thys [const AjPStr] String
** @param [r] pref [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixC (const AjPStr thys, const char* pref) {

  ajint ilen = strlen(pref);

  if (!ilen)		/* no prefix */
    return ajFalse;

  if (ilen > ajStrLen(thys))	/* pref longer */
      return ajFalse;
  if (strncmp(thys->Ptr, pref, ilen)) /* +1 or -1 for a failed match */
    return ajFalse;

  return ajTrue;
}


/* @func ajStrPrefixCC ********************************************************
**
** Tests the start of a string against a given prefix text.
**
** @param [r]str [const char*]  Test string as text
** @param [r] pref [const char*] Prefix as text
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCC (const char* str, const char* pref) {

  ajint ilen = strlen(pref);

  if (!ilen)		/* no prefix */
    return ajFalse;

  if (ilen > strlen(str))	/* pref longer */
      return ajFalse;
  if (strncmp(str, pref, ilen)) /* +1 or -1 for a failed match */
    return ajFalse;

  return ajTrue;
}


/* @func ajStrPrefixCO ********************************************************
**
** Tests the start of a text against a given prefix string.
**
** @param [r] str [const char*] Test string as text
** @param [r] thys [const AjPStr] Prefix as string
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCO (const char* str, const AjPStr thys) {

  if (thys->Len > strlen(str))	/* pref longer */
      return ajFalse;
  if (strncmp(str, thys->Ptr, thys->Len)) /* +1 or -1 for a failed match */
    return ajFalse;

  return ajTrue;
}

/* @func ajStrPrefixCase ****************************************************
**
** Tests the start of a string against a given prefix string,
** case insensitive.
**
** @param [r] thys [const AjPStr] String
** @param [r] pref [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCase (const AjPStr thys, const AjPStr pref) {

  return ajStrPrefixCaseCC(thys->Ptr, pref->Ptr);
}

/* @func ajStrPrefixCaseC ****************************************************
**
** Tests the start of a string against a given prefix string,
** case insensitive.
**
** @param [r] thys [const AjPStr] String
** @param [r] pref [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseC (const AjPStr thys, const char* pref) {

  return ajStrPrefixCaseCC(thys->Ptr, pref);
}

/* @func ajStrPrefixCaseCO ****************************************************
**
** Tests the start of a string against a given prefix string,
** case insensitive.
**
** @param [r] thys [const char*] Text
** @param [r] pref [const AjPStr] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseCO (const char* thys, const AjPStr pref) {

  return ajStrPrefixCaseCC(thys, pref->Ptr);
}

/* @func ajStrPrefixCaseCC ****************************************************
**
** Tests the start of a string against a given prefix string,
** case insensitive.
**
** @param [r] thys [const char*] Text
** @param [r] pref [const char*] Prefix
** @return [AjBool] ajTrue if the string begins with the prefix
** @@
******************************************************************************/

AjBool ajStrPrefixCaseCC (const char* thys, const char* pref) {

  const char* cp = thys;
  const char* cq = pref;

  /* ajDebug ("ajStrPrefixCaseCC '%s' '%s'\n", thys, pref); */

  if (!*cq)
    return ajFalse;

  while (*cp && *cq) {
    if (tolower((ajint) *cp) != tolower((ajint) *cq)) return ajFalse;
    cp++;cq++;
  }

  if (*cq)
    return ajFalse;

  /* ajDebug ("ajStrPrefixCaseCC ..TRUE..\n"); */
  return ajTrue;
}

/* @func ajStrSuffix **********************************************************
**
** Tests the end of a string against a given suffix string.
**
** @param [r] thys [const AjPStr] String
** @param [r] suff [const AjPStr] Suffix
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffix (const AjPStr thys, const AjPStr suff) {

  ajint ilen = ajStrLen(suff);
  ajint istart = thys->Len - ilen;

  if (ilen > ajStrLen(thys))	/* suffix longer */
      return ajFalse;
  if (strncmp(&thys->Ptr[istart], suff->Ptr, ilen)) /* +1 or -1 for a 
						       failed match */
    return ajFalse;

  return ajTrue;
}

/* @func ajStrSuffixC *********************************************************
**
** Tests the end of a string against a given suffix text.
**
** @param [r] thys [const AjPStr] String
** @param [r] suff [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixC (const AjPStr thys, const char* suff) {

  ajint ilen = strlen(suff);
  ajint istart = thys->Len - ilen;

  if (ilen > ajStrLen(thys))	/* suff longer */
      return ajFalse;
  if (strncmp(&thys->Ptr[istart], suff, ilen)) /* +1 or -1 for a
						  failed match */
    return ajFalse;

  return ajTrue;
}

/* @func ajStrSuffixCC ********************************************************
**
** Tests the end of a string against a given suffix text.
**
** @param [r] str [const char*] String
** @param [r] suff [const char*] Suffix as text
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixCC (const char* str, const char* suff) {

  ajint ilen = strlen(suff);
  ajint jlen = strlen(str);
  ajint jstart = jlen - ilen;

  if (ilen > jlen)	/* suff longer */
      return ajFalse;
  if (strncmp(&str[jstart], suff, ilen)) /* +1 or -1 for a
					    failed match */
    return ajFalse;

  return ajTrue;
}

/* @func ajStrSuffixCO ********************************************************
**
** Tests the end of a text against a given suffix string.
**
** @param [r] str [const char*] Test string as text
** @param [r] suff [const AjPStr] Suffix as string
** @return [AjBool] ajTrue if the string ends with the suffix
** @@
******************************************************************************/

AjBool ajStrSuffixCO (const char* str, const AjPStr suff) {

  ajint jlen = strlen(str);
  ajint jstart = jlen - suff->Len;

  if (suff->Len > jlen)	/* suff longer */
      return ajFalse;
  if (strncmp(&str[jstart], suff->Ptr, suff->Len)) /* +1 or -1 for a
						    failed match */
    return ajFalse;

  return ajTrue;
}

/* @func ajStrIsAlpha *********************************************************
**
** Simple test for a string having no white space and being only alphabetic
** as defined by isalpha in the C RTL..
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphabetic
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsAlpha (const AjPStr thys) {
  char* cp = ajStrStr(thys);

  if (!thys->Len) return ajFalse;

  while (*cp) {
    if (!isalpha((ajint)*cp++)) return ajFalse;
  }

  return ajTrue;
}

/* @func ajStrIsAlnum *********************************************************
**
** Simple test for a string having no white space and being only alphanumeric
** as defined by isalnum in the C RTL plus underscores ..
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely alphanumeric
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsAlnum (const AjPStr thys) {
  char* cp;

  if (!thys->Len) return ajFalse;

  for (cp = ajStrStr(thys);*cp;cp++) {
    if (*cp != '_' && !isalnum((ajint)*cp)) return ajFalse;
  }

  return ajTrue;
}

/* @func ajStrIsWord **********************************************************
**
** Simple test for a string having no white space
** as defined by isalnum in the C RTL.
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string has no wite space
** @cre an empty string returns ajFalse
** @@
******************************************************************************/

AjBool ajStrIsWord (const AjPStr thys) {

  char* cp = ajStrStr(thys);

  if (!thys->Len) return ajFalse;

  while (*cp) {
    if (isspace((ajint)*cp++)) return ajFalse;
  }
  return ajTrue;
}

/* @func ajStrIsSpace *********************************************************
**
** Simple test for a string having only white space
** as defined by isspace in the C RTL..
**
** @param [rE] thys [const AjPStr] String
** @return [AjBool] ajTrue if the string is entirely white space
** @cre an empty string returns ajTrue
** @@
******************************************************************************/

AjBool ajStrIsSpace (const AjPStr thys) {
  char* cp = ajStrStr(thys);

  if (!thys->Len) return ajTrue;

  while (*cp) {
    if (!isspace((ajint)*cp++)) return ajFalse;
  }

  return ajTrue;
}

/* @func ajStrWrap **********************************************************
**
** Inserts newlines into a ajlong string, at white space if possible,
** so that it wraps when printed.
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] width [ajint] Line width
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrWrap(AjPStr* pthis, ajint width ) {

  AjPStr thys;
  char* cp;
  char* cq;
  ajint i;
  if (width > (*pthis)->Len)	/* already fits on one line */
    return ajTrue;

  (void) ajStrMod (pthis);
  thys = *pthis;

  cq = thys->Ptr;
  for (i=width; i < thys->Len; i+=width) {
    cp = &thys->Ptr[i];
    while (cp > cq && !isspace((ajint)*cp))
      cp--;
    if (cp == cq) {
      (void) ajStrInsertC(pthis, i, "\n");
      cp = &thys->Ptr[i+1];
    }
    else
      *cp = '\n';
    cq = cp;
  }

  return ajTrue;
}

/* @func ajStrWrapLeft ******************************************************
**
** Inserts newlines into a ajlong string, at white space if possible,
** so that it wraps when printed.
**
** This version asks for a left margin of space characters.
**
** @param [uP] pthis [AjPStr*] Target string
** @param [r] width [ajint] Line width
** @param [r] left [ajint] Left margin
** @return [AjBool] ajTrue on successful completion else ajFalse;
** @@
******************************************************************************/

AjBool ajStrWrapLeft (AjPStr* pthis, ajint width, ajint left) {

  static AjPStr newstr = NULL;
  char* cp;
  ajint len;
  ajint i=0;
  ajint j;
  ajint isp = 0;

  /* ajDebug ("ajStrWrapLeft %d %d\n'%S'\n", width, left, *pthis); */

  len = 1 + (*pthis)->Len + (left + 1) * (*pthis)->Len / width;
  (void) ajStrAssS (&newstr, *pthis);
  (void) ajStrAssCL (pthis, "", len);

  for (cp = ajStrStr(newstr); *cp; cp++) {
    switch (*cp) {
    case '\n':
      (void) ajStrAppK (pthis, '\n');
      for (j=0; j<left; j++)
	(void) ajStrAppK (pthis, ' ');
      i = 0;
      isp = 0;
      break;
    case ' ':
    case '\t':
      isp = (*pthis)->Len;
      /* ajDebug ("can split at %d\n", isp); */
    default:
      if (++i >= width) {	/* too wide, time to split */
	/* ajDebug("split at i: %d isp: %d\n'%S'\n", i, isp, *pthis); */
	if (isp) {
	  if (isp == (*pthis)->Len)
	    (void) ajStrAppK (pthis, '\n');
	  else
	    (*pthis)->Ptr[isp] = '\n';
	}
	else {
	  (void) ajStrAppK (pthis, *cp); /* keep going */
	  break;
	}
	for (j=0; j<left; j++) { /* follow newline with left margin spaces */
	  isp++;
	  (void) ajStrInsertC (pthis, isp, " ");
	}
	i = (*pthis)->Len - isp;
	isp = 0;
	if (!isspace((ajint)*cp))
	  (void) ajStrAppK (pthis, *cp);
      }
      else {
	(void) ajStrAppK (pthis, *cp);
      }
      break;
    }
  }

  return ajTrue;
}

/* @func ajStrChar ************************************************************
**
** Returns a single character from a string at a given position
**
** @param [r] thys [const AjPStr] String
** @param [r] pos [ajint] Position in the string, negative values are
**        from the end of the string.
** @return [char] Character at position pos or null character if out of range.
** @@
******************************************************************************/

char ajStrChar (const AjPStr thys, ajint pos) {

  ajint ipos;

  if (!thys)
    return '\0';

  if (pos < 0)
    ipos = thys->Len + pos;
  else
    ipos = pos;

  if ((ipos < 0) || (ipos > thys->Len))
    return '\0';

  return thys->Ptr[ipos];
}

/* ==================================================================== */
/* ====================== Miscellaneous ================================*/
/* ==================================================================== */

/*Intro* String Miscellaneous
**
** These functions have no home as yet, but can usefully be considered as
** String functions for now.
**
*/


/* @func ajStrListToArray ****************************************************
**
** Splits a newline separated multi-line string into an array of AjPStrs
**
** @param [r] thys [AjPStr] String
** @param [w] array [AjPStr**] pointer to array of AjPStrs
**
** @return [ajint] Number of array elements created
** @@
******************************************************************************/

ajint ajStrListToArray(AjPStr thys, AjPStr **array)
{
    ajint c;
    ajint len;
    ajint i;
    ajint n;
    char *p=NULL;
    char *q=NULL;

    if(!thys->Len)
	return 0;


    p = q = ajStrStr(thys);

    len = thys->Len;
    for(i=c=n=0;i<len;++i)
	if(*(p++)=='\n')
	    ++c;
    p=q;
    
    AJCNEW0(*array,c);


    for(i=0;i<c;++i)
    {
	while(*q!='\n')
	    ++q;
	(*array)[n] = ajStrNew();
	ajStrAssSubC(&(*array)[n++],p,0,q-p);
	p = ++q;
    }
    
    return c;
}


/* @func ajStrDegap **********************************************************
**
** Removes all but alphabetic characters from a string
**
** @param [w] thys [AjPStr*] String
** @return [void]
** @@
******************************************************************************/

void ajStrDegap(AjPStr* thys)
{
    char *p;
    char *q;
    ajint  i;
    ajint  len;
    char c;
    
    p = q = (*thys)->Ptr;
    len = (*thys)->Len;
    
    for(i=0;i<len;++i)
    {
	c = *(p++);
	if((c>='A' && c<='Z') || (c>='a' && c<='z'))
	    *(q++) = c;
	else
	    --(*thys)->Len;
    }
    (*thys)->Ptr[(*thys)->Len] = '\0';

    return;
}

/* @func ajStrRemoveHtml *****************************************************
**
** Removes all strings between and including angle brackets
**
** @param [w] thys [AjPStr*] String
** @return [void]
** @@
******************************************************************************/

void ajStrRemoveHtml(AjPStr *thys)
{
    char *p;
    char *q;

    p = q = (*thys)->Ptr;
    while(*p)
    {
	if(*p!='<')
	{
	    *q++=*p++;
	    continue;
	}
	while(*p)
	{
	    if(*p=='>')
	    {
		++p;
		break;
	    }
	    --(*thys)->Len;
	    ++p;
	}
    }
    *q='\0';

    return;
}


/* @func ajStrRemoveNewline **************************************************
**
** Removes any trailing newline
**
** @param [w] thys [AjPStr*] String
** @return [void]
** @@
******************************************************************************/

void ajStrRemoveNewline(AjPStr *thys)
{
    char *p;
    AjPStr pthis=*thys;

    p = pthis->Ptr;

    if(pthis->Len)
      if(*(p+pthis->Len-1)=='\n')
      {
         *(p+pthis->Len-1)='\0';
         --pthis->Len;
      }

    return;
}

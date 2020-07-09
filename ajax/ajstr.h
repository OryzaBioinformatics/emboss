#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajstr_h
#define ajstr_h

#include "ajdefine.h"
#include "ajtable.h"

/* @data AjPStr *******************************************************
**
** Ajax string object.
**
** Holds a null terminated character string with additional data.
** The length is known and held internally.
** The reserved memory size is known and held internally.
** The reference count is known and held internally.
** New pointers can refer to the same string without needing
** to duplicate the character data.
**
** If a string has multiple references it cannot be changed. Any
** instance to be changed is first copied to a new string. This
** means that any function which can change the character data must
** pass a pointer to the string so that the string can be moved.
**
** A default null string is provided. New strings are by default
** implemented as pointers to this with increased reference counters.
**
** AjPStr is implemented as a pointer to a C data structure.
**
** @alias AjSStr
** @alias AjOStr
**
** @new ajStrNew Default constructor
** @new ajStrNewC Constructor with char* text
** @new ajStrNewL Constructor with reserved size
** @new ajStrNewCL Constructor with char* text and reserved size
** @new ajStrNewCIL Constructor with char* text, length and reserved size
** @new ajStrDup Duplicates an existing string (with increased reference count)
** @new ajCharNew    Constructor for a new C text string with a given string
** @new ajCharNewC   Constructor for a new C text string with a text value
** @new ajCharNewL   Constructor for a new empty C text string with
**                   a given length
** @new ajCharNewLS  Constructor for a new C text string with a given
**                   string and length.
**
** @delete ajStrDel Default destructor
** @delete ajCharFree Deletes a C text string
**
** @ass ajStrClear Clears all contents
** @ass ajStrSet   If empty, initialize with a copy of a string object
** @ass ajStrSetC  If empty, initialize with a copy of a char* text
** @ass ajStrAss   Assignment with a copy of a string object
** @ass ajStrAssC  Assignment with char* text.
** @ass ajStrAssI  Assignment with a copy of a string object and a maximum
**                 length.
** @ass ajStrAssL  Assignment with a copy of a string object and a minimum
**                 reserved size.
** @ass ajStrAssCI  Assignment with char* text and a maximum
**                 length.
** @ass ajStrAssCL  Assignment with a char* text and a minimum
**                 reserved size.
** @ass ajStrAssSub Assignment with a substring of a string object
** @ass ajStrAssSubC Assignment with a substring of a char* text
**
** @ass ajStrFromBool Creates a string representation of an AjBool
** @ass ajStrFromInt  Creates a string representation of an int
** @ass ajStrFromLong Creates a string representation of a ajlong int
** @ass ajStrFromFloat Creates a string representation of a float
** @ass ajStrFromDouble Creates a string representation of a double
**
** @mod ajStrApp   Appends a string object
** @mod ajStrAppC  Appends a char* text
** @mod ajStrAppK  Appends a char
** @mod ajStrAppSub Appends a substring
** @mod ajStrInsert  Inserts string object text within a string.
** @mod ajStrInsertC Inserts char* text within a string.
** @mod ajStrTruncate Removes characters beyond a given position.
** @mod ajStrReplace  Replace remainder of string with char* text.
** @mod ajStrReplaceC Replace remainder of string with string object text.
** @mod ajStrJoin  Replace remainder of string with remainder of second string.
** @mod ajStrJoinC Replace remainder of string with remainder of char* text.
** @mod ajStrSubstitute   Replace all occurances of one string with another in
**                        the string object.
** @mod ajStrSubstituteCC Replace all occurances of char *text  with
**                        another char* in the string object.
** @mod ajStrChomp Removes white space from front and end of string.
** @mod ajStrChompC Removes white space from front and end of string.
** @mod ajStrChop  Removes the last character from a string
** @mod ajStrTrim  Removes a number of characters from start of end of a string
** @mod ajStrTrimC Removes a set of characters from start of end of a string
** @mod ajStrCut Removes a range of character positions from a string
** @mod ajStrMask Masks out a range of characters from a string
** @mod ajStrRev Reverses the order of characters in a string
** @mod ajStrSub Reduces string to a substring of itself.
** @mod ajStrMod  Make certain a string is modifiable by checking it has no
**                other references, or by making a new real copy of the string.
** @mod ajStrModL Make certain a string is modifiable, and big enough for its
**                intended purpose.
** @mod ajStrConvert   Replaces one set of characters with another set,
**                     defined as two string objects
** @mod ajStrConvertCC Replaces one set of characters with another set,
**                     defined as two char* texts.
** @mod ajStrToLower  Converts a string to lower case.
** @mod ajCharToLower Converts a C text string to lower case.
** @mod ajStrToUpper  Converts a string to upper case.
** @mod ajCharToUpper Converts a C text string to upper case.
** @mod ajStrClean   Remove excess whitespace from a string
** @mod ajStrCleanWhite Remove all whitespace from a string
** @mod ajStrFix  Reset string length when some nasty caller may have edited it
** @mod ajStrFixI Reset string length when some nasty caller may have edited it
**
** @use ajStrFind   Find
** @use ajStrFindC  Find
** @use ajStrFindCase   Find
** @use ajStrFindCaseC  Find
** @use ajStrRFindC Reverse find.
** @use ajStrCmp  String compare
** @use ajStrCmpO  String compare
** @use ajStrCmpC  String compare
** @use ajStrNCmpO String compare
** @use ajStrNCmpC String compare
** @use ajStrCmpCase String case insensitive compare
** @use ajStrCmpCaseCC String case insensitive compare
**
** @use ajStrMatch       Test for matching strings
** @use ajStrMatchC      Test for matching strings
** @use ajStrMatchCC     Test for matching strings
** @use ajStrMatchCase   Test for matching strings
** @use ajStrMatchCaseC  Test for matching strings
** @use ajStrMatchCaseCC Test for matching strings
** @use ajStrMatchWild   Test for matching strings with wildcards
** @use ajStrMatchWildC  Test for matching strings with wildcards
** @use ajStrMatchWildCC Test for matching strings with wildcards
** @use ajStrMatchWildCO Test for matching strings with wildcards
** @use ajStrWildPrefix Tests for wildcard characters and terminates the
**                      string at the first wild character (if any).
** @use ajStrIsWild Tests whether a string contains standard wildcard
**                  characters '*' or '?'
** @use ajStrCheck Tests a string has a valid internal structure
** @use ajStrTrace Writes a debug report on a string
** @use ajStrExit Prints a summary of string usage with debug calls
**
** @cast ajStrStr Returns char* text.
**                Note: this should be treated as a constant as it is a
**                pointer to the actual data to avoid excessive memory
**                allocations.
** @cast ajStrLen Returns string length
** @cast ajStrSize Returns string reserved bytes (including trailing null)
** @cast ajStrRef Returns string reference count
** @cast ajStrPos String position
** @cast ajStrBool    Returns a static C text string with a Yes/No value
**                   from an AjBool
** @cast ajStrYN      Returns a static C text string with a Y/N 1 character
**                   value from an AjBool
** @cast ajStrTok  Returns a static next string from token parsing of a string
**                 with the previous delimiter set
** @cast ajStrTokC Returns a static next string from token parsing of a
**                 string with a new delimiter set
** @cast ajStrIsBool returns true if the string is a valid AjBool
** @cast ajStrIsInt returns true if the string is a valid int
** @cast ajStrIsLong returns true if the string is a valid ajlong int
** @cast ajStrIsFloat returns true if the string is a valid float
** @cast ajStrIsDouble returns true if the string is a valid double
** @cast ajStrToBool Converts a string to an AjBool
** @cast ajStrToInt Converts a string to an int
** @cast ajStrToLong Converts a string to a ajlong int
** @cast ajStrToFloat Converts a string to a float
** @cast ajStrToDouble Converts a string to a double
** @cast ajStrTokenCount Returns the number of tokens in a string
** @cast ajStrRoom Returns the additional space available in a string
** @cast ajStrPos Converts a string position into a true position
** @cast ajStrPosI Converts a string position into a true position
** @cast ajCharPos Converts a string position into a true position
**
** @other AjPSeq uses AjPStr to store most values
** @other AjPStrTok parses AjPStr with delimiters
** @@
******************************************************************************/

typedef struct AjSStr {
  ajint Res;
  ajint Len;
  ajint Use;
  char *Ptr;
} AjOStr, *AjPStr;

/* @data AjPStrIter *******************************************************
**
** String iterator, used to test iterator functionality.
**
** @new ajStrIter Creates and initializes an iterator for a string
**
** @delete ajStrIterFree Destructor for a string iterator
**
** @mod ajStrIterNext Steps to the next iteration
**
** @cast ajStrIterBegin returns result of first iteration
** @cast ajStrIterEnd   returns result of last iteration
** @cast ajStrIterDone  Tests whether iteration is complete
** @cast ajStrIterGetCC   returns the character from the iterator
** @cast ajStrIterGetC   returns the character* from the iterator
** @@
******************************************************************************/
typedef struct AjSStrIter {
  char *Start;
  char *End;
  char *Ptr;
} *AjIStr;


/* @data AjPStrTok *******************************************************
**
** String token parser object for the string parsing functions. These normally
** require a set of characters to be skipped, but some functions use a string
** delimiter instead.
**
** @new ajStrTokenInit Generates a string token parser object
** @delete ajStrTokenClear Destroys a string token parser
** @use ajStrToken Parse a string with a set of unwanted characters and
**                 return the next token.
**
**                 Parsing is initialized with ajStrTokenInit
** @use ajStrDelim Parse a string with a string delimiter and return the
**                 next token.
**
**                 Parsing is initialized with ajStrTokenInit
** @@
******************************************************************************/

typedef struct AjSStrTok {
  AjPStr String;
  AjPStr Delim;
  ajint Pos;
} AjOStTok, *AjPStrTok;


/* ========================================================================= */
/* =================== All functions in alphabetical order ================= */
/* ========================================================================= */

char*      ajCharFree (char* txt);
char*      ajCharNewC (ajint len, const char* txt);
char*      ajCharNew (const AjPStr thys);
char*      ajCharNewL (ajint len);
char*      ajCharNewLS (size_t size, const AjPStr thys);
ajint        ajCharPos (const char* thys, ajint ipos);
void       ajCharToLower (char *txt);
void       ajCharToUpper (char *txt);

AjBool     ajStrApp  (AjPStr* pthis, const AjPStr src);
AjBool     ajStrAppC (AjPStr* pthis, const char *txt);
AjBool     ajStrAppK (AjPStr* pthis, const char chr);
AjBool     ajStrAppKI (AjPStr* pthis, const char chr, ajint number);
AjBool     ajStrAppSub (AjPStr* pthis, const AjPStr src, ajint begin, ajint end);
AjBool     ajStrAss   (AjPStr* pthis, const AjPStr str);
AjBool     ajStrAssC  (AjPStr* pthis, const char* txt);
AjBool     ajStrAssCI (AjPStr* pthis, const char* txt, size_t i);
AjBool     ajStrAssCL (AjPStr* pthis, const char* txt, size_t i);
AjBool     ajStrAssI  (AjPStr* pthis, const AjPStr str, size_t i);
AjBool     ajStrAssL  (AjPStr* pthis, const AjPStr str, size_t i);
AjBool     ajStrAssS  (AjPStr* pthis, const AjPStr str);
AjBool     ajStrAssSub  (AjPStr* pthis, const AjPStr str, ajint begin, ajint end);
AjBool     ajStrAssSubC (AjPStr* pthis, const char* txt, ajint begin, ajint end);
AjBool     ajStrBlock (AjPStr* pthis, ajint blksize);
char*      ajStrBool (AjBool boule);
char       ajStrChar (const AjPStr thys, ajint pos);
AjBool     ajStrCheck (const AjPStr thys);
AjBool     ajStrChomp (AjPStr* pthis);
AjBool     ajStrChompC (AjPStr* pthis, char* delim);
AjBool     ajStrChop  (AjPStr* pthis);
AjBool     ajStrClean (AjPStr* s);
AjBool     ajStrCleanWhite (AjPStr* s);
AjBool     ajStrClear (AjPStr* pthis);
int        ajStrCmpC  (const AjPStr thys, const char *text);
int        ajStrCmpCase (const AjPStr str1, const AjPStr str2);
int        ajStrCmpCaseCC (const char* str1, const char* str2);
int        ajStrCmp  (const void* str1, const void* str2);
int        ajStrCmpO  (const AjPStr thys, const AjPStr anoth);
int        ajStrCmpWild (const AjPStr thys, const AjPStr str);
int        ajStrCmpWildC (const AjPStr thys, const char* text);
int        ajStrCmpWildCC (const char* str, const char* text);
AjBool     ajStrConvert   (AjPStr* pthis, const AjPStr oldc,
			   const AjPStr newc);
AjBool     ajStrConvertCC (AjPStr* pthis, const char* oldc, const char* newc);
AjBool     ajStrCut (AjPStr* pthis, ajint begin, ajint end);
void       ajStrDegap(AjPStr* thys);
void       ajStrDel (AjPStr *thys);
AjBool     ajStrDelim (AjPStr* pthis, AjPStrTok *ptoken, const char *delim);
AjBool     ajStrDelReuse (AjPStr *thys);
AjPStr     ajStrDup (const AjPStr thys);
void       ajStrExit (void);

ajint        ajStrFind (const AjPStr thys, const AjPStr text);
ajint        ajStrFindC  (const AjPStr thys, const char* txt);
ajint        ajStrFindCase (const AjPStr thys, const AjPStr text);
ajint        ajStrFindCaseC (const AjPStr thys, const char* txt);

void       ajStrFill (AjPStr* thys, ajint count, char fill);
void       ajStrFix (const AjPStr thys);
void       ajStrFixI (const AjPStr thys, ajint ilen);
void       ajStrFixTestI (const AjPStr thys, ajint ilen);
AjBool     ajStrFromBool (AjPStr *pthis, AjBool boule);
AjBool     ajStrFromDouble (AjPStr *pthis, double val, ajint precision);
AjBool     ajStrFromFloat (AjPStr *pthis, float val, ajint precision);
AjBool     ajStrFromInt (AjPStr *pthis, ajint val);
AjBool     ajStrFromLong (AjPStr *pthis, ajlong val);

AjBool     ajStrInsert  (AjPStr* pthis, ajint pos, const AjPStr str);
AjBool     ajStrInsertC (AjPStr* pthis, ajint pos, const char* str);
AjBool     ajStrIsAlnum (const AjPStr thys);
AjBool     ajStrIsAlpha (const AjPStr thys);
AjBool     ajStrIsBool (const AjPStr thys);
AjBool     ajStrIsDouble (const AjPStr thys);
AjBool     ajStrIsFloat (const AjPStr thys);
AjBool     ajStrIsInt (const AjPStr thys);
AjBool     ajStrIsLong (const AjPStr thys);
AjBool     ajStrIsSpace (const AjPStr thys);
AjBool     ajStrIsWild (const AjPStr thys);
AjBool     ajStrIsWord (const AjPStr thys);
AjIStr     ajStrIter (const AjPStr thys);
AjIStr     ajStrIterBack (const AjPStr thys);
#define    ajStrIterBackDone(iter) (iter->Ptr < iter->Start)
#define    ajStrIterBackMore(iter) (iter->Ptr > iter->Start)
AjIStr     ajStrIterBackNext (AjIStr iter);
#define    ajStrIterBegin(iter) (iter->Ptr = iter->Start)
#define    ajStrIterDone(iter) (iter->Ptr > iter->End)
#define    ajStrIterEnd(iter) (iter->Ptr = iter->End)
void       ajStrIterFree (AjIStr *iter);
#define    ajStrIterGetK(iter) (*iter->Ptr)
#define    ajStrIterGetC(iter) (iter->Ptr)
#define    ajStrIterMore(iter) (iter->Ptr < iter->End)
AjIStr     ajStrIterNext (AjIStr iter);
#define    ajStrIterPutK(iter,c) (*iter->Ptr = c) 
AjBool     ajStrJoin  (AjPStr* pthis, ajint pos1, const AjPStr addbit, ajint pos2);
AjBool     ajStrJoinC (AjPStr* pthis, ajint pos1, const char* addbit, ajint pos2);

ajint        ajStrLen(const AjPStr thys);

#define    AJSTRLEN(str) str->Len
ajint        ajStrListToArray(AjPStr str, AjPStr **array);
AjBool     ajStrMask(AjPStr* str, ajint begin, ajint end, char maskchar);
AjBool     ajStrMatch       (const AjPStr thys, const AjPStr str);
AjBool     ajStrMatchC      (const AjPStr thys, const char* txt);
AjBool     ajStrMatchCase   (const AjPStr thys, const AjPStr str);
AjBool     ajStrMatchCaseC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchCaseCC (const char* thys, const char* text);
AjBool     ajStrMatchCC     (const char* thys, const char* text);
AjBool     ajStrMatchWild   (const AjPStr thys, const AjPStr wild);
AjBool     ajStrMatchWildC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchWildCC (const char* str, const char* text);
AjBool     ajStrMatchWildCO (const char* str, const AjPStr wild);
AjBool     ajStrMod  (AjPStr* pthis);
AjBool     ajStrModL (AjPStr* pthis, size_t size);
ajint        ajStrNCmpC (const AjPStr thys, const char *text, ajint n);
ajint 	   ajStrNCmpCaseCC (const char* str1, const char* str2, ajint len);
ajint        ajStrNCmpO (const AjPStr thys, const AjPStr anoth, ajint n);
AjPStr     ajStrNew (void);
AjPStr     ajStrNewC (const char *txt);
AjPStr     ajStrNewCIL (const char *txt, ajint len, size_t size);
AjPStr     ajStrNewCL (const char *txt, size_t size);
AjPStr     ajStrNewL (size_t size);
AjPStr     ajStrNewS (AjPStr str);

ajint        ajStrPos  (const AjPStr thys, ajint ipos);
ajint        ajStrPosI (const AjPStr thys, ajint imin, ajint ipos);
ajint        ajStrPosII (ajint ilen, ajint imin, ajint ipos);
AjBool     ajStrPrefix (const AjPStr thys, const AjPStr pref);
AjBool     ajStrPrefixC (const AjPStr thys, const char* pref);
AjBool     ajStrPrefixCC (const char *str, const char* pref);
AjBool     ajStrPrefixCase (const AjPStr thys, const AjPStr pref);
AjBool     ajStrPrefixCaseC (const AjPStr thys, const char* pref);
AjBool     ajStrPrefixCaseCC (const char *str, const char* pref);
AjBool     ajStrPrefixCaseCO (const char* thys, const AjPStr pref);
AjBool     ajStrPrefixCO (const char *str, const AjPStr thys);
void       ajStrRandom(AjPStr *s);
AjBool     ajStrReplace  (AjPStr* pthis, ajint pos1,
			  const AjPStr overwrite, ajint len);
AjBool     ajStrReplaceC (AjPStr* pthis, ajint pos1,
			  const char* overwrite, ajint len);
ajint        ajStrRef(const AjPStr thys);
#define    AJSTRREF(str) str->Use
void       ajStrRemoveHtml(AjPStr *thys);
void       ajStrRemoveNewline(AjPStr *thys);
AjBool     ajStrRev (AjPStr* pthis);
ajint        ajStrRFindC (const AjPStr thys, const char *text);
ajint        ajStrRoom (const AjPStr thys);
AjBool     ajStrSet   (AjPStr* pthis, const AjPStr str);
AjBool     ajStrSetC  (AjPStr* pthis, const char* str);
ajint        ajStrSize (const AjPStr thys);
#define    AJSTRSIZE(str) str->Res
void       ajStrStat (char* title);
char*      ajStrStr (const AjPStr thys);
#define    AJSTRSTR(str) str->Ptr
AjBool     ajStrSub (AjPStr* pthis, ajint begin, ajint len);
AjBool     ajStrSubstitute   (AjPStr* pthis, const AjPStr replace,
			      const AjPStr putin);
AjBool     ajStrSubstituteCC (AjPStr* pthis, const char* replace,
			      const char* putin);
AjBool     ajStrSuffix (const AjPStr thys, const AjPStr suff);
AjBool     ajStrSuffixC (const AjPStr thys, const char* suff);
AjBool     ajStrSuffixCC (const char *str, const char* suff);
AjBool     ajStrSuffixCO (const char *str, const AjPStr suff);

AjBool     ajStrToBool (const AjPStr thys, AjBool* result);
AjBool     ajStrToDouble (const AjPStr thys, double* result);
AjBool     ajStrToFloat (const AjPStr thys, float* result);
AjBool     ajStrToInt (const AjPStr thys, ajint* result);
AjPStr     ajStrTok (const AjPStr thys);
AjPStr     ajStrTokC (const AjPStr thys, const char* delim);
AjBool     ajStrToken (AjPStr* pthis, AjPStrTok *ptoken, const char *delim);
AjBool     ajStrTokenAss (AjPStrTok *ptok, const AjPStr thys,
			  const char *delim);
void       ajStrTokenClear (AjPStrTok *token);
ajint        ajStrTokenCount(AjPStr *line, const char *delim);
AjPStrTok  ajStrTokenInit (const AjPStr thys, const char *delim);
void       ajStrTokenReset (AjPStrTok* ptok);
AjBool     ajStrTokenRest (AjPStr* pthis, AjPStrTok* ptoken);
void       ajStrTokenTrace (AjPStrTok tok);
AjBool     ajStrToLong (const AjPStr thys, ajlong* result);
AjBool     ajStrToLower (AjPStr* pthis);
AjBool     ajStrToUpper (AjPStr* pthis);
void       ajStrTrace (const AjPStr thys);
void       ajStrTraceT (const AjPStr thys, char* title);
AjBool     ajStrTrim  (AjPStr* pthis, ajint num);
AjBool     ajStrTrimC (AjPStr* pthis, const char* chars);
AjBool     ajStrTruncate (AjPStr* pthis, ajint pos);
AjBool     ajStrWildPrefix (AjPStr* str);
AjBool     ajStrWrap (AjPStr* pthis, ajint width);
AjBool     ajStrWrapLeft (AjPStr* pthis, ajint width, ajint left);
char*      ajStrYN (AjBool boule);

#endif

#ifdef __cplusplus
}
#endif

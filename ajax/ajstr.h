#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajstr_h
#define ajstr_h

#include "ajdefine.h"
#include "ajtable.h"




/* @data AjPStr ***************************************************************
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
** @alias AjPPStr
** @alias AjSStr
** @alias AjOStr
** @iterator AjIStr
**
** @attr Res [ajint] Reserved bytes (usable for expanding in place)
** @attr Len [ajint] Length of current string, excluding NULL at end
** @attr Use [ajint] Use count: 1 for single reference, more if several
**                   pointers share the same string.
**                   Must drop to 0 before deleting. Modifying means making
**                   a new string if not 1.
** @attr Ptr [char*] The string, as a NULL-terminated C string.
** @@
******************************************************************************/

typedef struct AjSStr {
  ajint Res;
  ajint Len;
  ajint Use;
  char *Ptr;
} AjOStr;
#define AjPStr AjOStr*
typedef AjPStr* AjPPStr;




/* @data AjIStr ***************************************************************
**
** String iterator, used to test iterator functionality.
**
** @new ajStrIter Creates and initializes an iterator for a string
**
** @delete ajStrIterFree Destructor for a string iterator
**
** @modify ajStrIterNext Steps to the next iteration
**
** @modify ajStrIterBegin returns result of first iteration
** @modify ajStrIterNext Steps to the next iteration
** @modify ajStrIterBackNext Step to previous character in string iterator.
** @modify ajStrIterEnd   returns result of last iteration
** @cast ajStrIterDone  Tests whether iteration is complete
** @cast ajStrIterGetC  returns the character* from the iterator
** @cast ajStrIterGetK  returns the character from the iterator
**
** @attr Start [char*] Starting string pointer
** @attr End [char*] Final string pointer (NULL character position)
** @attr Ptr [char*] Current string pointer
** @@
******************************************************************************/

typedef struct AjSStrIter {
  char *Start;
  char *End;
  char *Ptr;
} AjOStrIter;

#define AjIStr  AjOStrIter*




/* @data AjPStrTok ************************************************************
**
** String token parser object for the string parsing functions. These normally
** require a set of characters to be skipped, but some functions use a string
** delimiter instead.
**
** @attr String [AjPStr] String
** @attr Delim [AjPStr] Delimiter set for ajStrToken
** @attr Pos [ajint] Position in string
**
** @new ajStrTokenInit Generates a string token parser object
** @delete ajStrTokenClear Destroys a string token parser
** @@
******************************************************************************/

typedef struct AjSStrTok {
  AjPStr String;
  AjPStr Delim;
  ajint Pos;
} AjOStrTok;

#define AjPStrTok AjOStrTok*




/* ========================================================================= */
/* ========================= All functions by section ====================== */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* === C character string === */

/* constructors */

char*      ajCharNewC (const char* txt);
char*      ajCharNewS (const AjPStr thys);
char*      ajCharNewRes(size_t size);
char*      ajCharNewResC(const char* txt, size_t size);
char*      ajCharNewResS(const AjPStr str, size_t size);
char*      ajCharNewResLenC(const char* txt, size_t size, size_t len);

/* destructors */

void       ajCharDel(char** Ptxt);

/* formatting */

AjBool     ajCharFmtLower (char *txt);
AjBool     ajCharFmtUpper (char *txt);

/* comparison */

AjBool     ajCharMatchC(const char* txt1, const char* txt2);
AjBool     ajCharMatchCaseC(const char* txt1, const char* txt2);
AjBool     ajCharMatchWildC(const char* txt1, const char* txt2);
AjBool     ajCharMatchWildS(const char* txt, const AjPStr str);
AjBool     ajCharMatchWildNextC(const char* txt1, const char* txt2);
AjBool     ajCharMatchWildWordC(const char* str, const char* txt);
AjBool     ajCharPrefixC(const char* txt, const char* pref);
AjBool     ajCharPrefixS(const char* txt, const AjPStr pref);
AjBool     ajCharPrefixCaseC(const char* txt, const char* pref);
AjBool     ajCharPrefixCaseS(const char* txt, const AjPStr pref);
AjBool     ajCharSuffixC(const char* txt, const char* suff);
AjBool     ajCharSuffixS(const char* txt, const AjPStr suff);
AjBool     ajCharSuffixCaseC(const char* txt, const char* suff);
AjBool     ajCharSuffixCaseS(const char* txt, const AjPStr suff);

/* comparison (sorting) */


int        ajCharCmpCase(const char* txt1, const char* txt2);
int        ajCharCmpCaseLen(const char* txt1, const char* txt2, size_t len);
ajint      ajCharCmpWild(const char* txt1, const char* txt2);

/* parsing */

AjPStr     ajCharParseC (const char* txt, const char* delim);

/* === AjPStr string === */

/* constructors */

AjPStr     ajStrNew (void);
AjPStr     ajStrNewC (const char *txt);
AjPStr     ajStrNewS (const AjPStr str);
AjPStr     ajStrNewRef(AjPStr str);
AjPStr     ajStrNewRes(size_t size);
AjPStr     ajStrNewResC (const char *txt, size_t size);
AjPStr     ajStrNewResS (const AjPStr str, size_t size);
AjPStr     ajStrNewResLenC (const char *txt, size_t size, size_t len);

/* destructors */

void       ajStrDel (AjPStr* Pstr);
void       ajStrDelarray(AjPStr** PPstr);
AjBool     ajStrDelStatic(AjPStr* Pstr);

/* assignment */

AjBool     ajStrAssignC(AjPStr* Pstr, const char* txt);
AjBool     ajStrAssignK(AjPStr* Pstr, char chr);
AjBool     ajStrAssignS(AjPStr* Pstr, const AjPStr str);
AjBool     ajStrAssignEmptyC  (AjPStr* pthis, const char* str);
AjBool     ajStrAssignEmptyS   (AjPStr* pthis, const AjPStr str);
AjBool     ajStrAssignLenC(AjPStr* Pstr, const char* txt, size_t ilen);
AjBool     ajStrAssignRef(AjPStr* Pstr, AjPStr refstr);
AjBool     ajStrAssignResC(AjPStr* Pstr, size_t size, const char* txt);
AjBool     ajStrAssignResS(AjPStr* Pstr, size_t i, const AjPStr str);
AjBool     ajStrAssignSubC(AjPStr* Pstr, const char* txt,
			   ajint pos1, ajint pos2);
AjBool     ajStrAssignSubS(AjPStr* Pstr, const AjPStr str,
			  ajint pos1, ajint pos2);

/* combination */

AjBool     ajStrAppendC(AjPStr* Pstr, const char* txt);
AjBool     ajStrAppendK(AjPStr* Pstr, char chr);
AjBool     ajStrAppendS(AjPStr* Pstr, const AjPStr str);
AjBool     ajStrAppendCountK(AjPStr* Pstr, char chr, size_t num);
AjBool     ajStrAppendLenC(AjPStr* Pstr, const char* txt, size_t len);
AjBool     ajStrAppendSubS(AjPStr* Pstr, const AjPStr str,
			   ajint pos1, ajint pos2);

AjBool     ajStrInsertC (AjPStr* pthis, ajint pos, const char* str);
AjBool     ajStrInsertK (AjPStr* pthis, ajint begin, char insert);
AjBool     ajStrInsertS  (AjPStr* pthis, ajint pos, const AjPStr str);

AjBool     ajStrJoinC (AjPStr* Pstr, ajint pos1,
		       const char* txt, ajint pos2);
AjBool     ajStrJoinS (AjPStr* Pstr, ajint pos,
		       const AjPStr str, ajint posb);
AjBool     ajStrMask(AjPStr* str, ajint begin, ajint end,
				  char maskchar);
AjBool     ajStrPasteS( AjPStr* Pstr, ajint pos, const AjPStr str);
AjBool     ajStrPasteCountK(AjPStr* Pstr, ajint pos, char chr,
		      size_t num);
AjBool     ajStrPasteMaxC (AjPStr* Pstr, ajint pos, const char* txt,
		     size_t n);
AjBool     ajStrPasteMaxS( AjPStr* Pstr, ajint pos, const AjPStr str,
		       size_t n);

/* cut */

AjBool     ajStrCutComments(AjPStr* Pstr);
AjBool     ajStrCutCommentsStart(AjPStr* Pstr);
AjBool     ajStrCutEnd(AjPStr* Pstr, size_t len);
AjBool     ajStrCutRange(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrCutStart(AjPStr* Pstr, size_t len);
AjBool     ajStrKeepRange(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrKeepSetC(AjPStr* Pstr, const char* txt);
AjBool     ajStrKeepSetAlphaC(AjPStr* Pstr, const char* txt);
AjBool     ajStrQuoteStrip(AjPStr *Pstr);
AjBool     ajStrQuoteStripAll(AjPStr *Pstr);
AjBool     ajStrRemoveGap(AjPStr* thys);
AjBool     ajStrRemoveHtml(AjPStr* pthis);
AjBool     ajStrRemoveLastNewline(AjPStr* Pstr);
AjBool     ajStrRemoveSetC(AjPStr* Pstr, const char *txt);
AjBool     ajStrRemoveWhite(AjPStr* Pstr);
AjBool     ajStrRemoveWhiteExcess(AjPStr* Pstr);
AjBool     ajStrRemoveWild(AjPStr* Pstr);
AjBool     ajStrTrimC (AjPStr* pthis, const char* txt);
AjBool     ajStrTrimEndC (AjPStr* Pstr, const char* txt);
AjBool     ajStrTrimStartC (AjPStr* Pstr, const char* txt);
AjBool     ajStrTrimWhite(AjPStr* Pstr);
AjBool     ajStrTrimWhiteEnd(AjPStr* Pstr);
AjBool     ajStrTruncateLen(AjPStr* Pstr, size_t len);
AjBool     ajStrTruncatePos(AjPStr* Pstr, ajint pos);

/* substitution */

AjBool     ajStrExchangeCC(AjPStr* Pstr, const char* txt, const char* txtnew);
AjBool     ajStrExchangeCS(AjPStr* Pstr, const char* txt,
			   const AjPStr strnew);
AjBool     ajStrExchangeKK(AjPStr* Pstr, char chr, char chrnew);
AjBool     ajStrExchangeSC(AjPStr* Pstr, const AjPStr str,
			   const char* txtnew);
AjBool     ajStrExchangeSS(AjPStr* Pstr, const AjPStr str,
			   const AjPStr strnew);
AjBool     ajStrExchangeSetCC(AjPStr* Pstr, const char* oldc,
			      const char* newc);
AjBool     ajStrExchangeSetSS(AjPStr* Pstr, const AjPStr str,
			    const AjPStr strnew);
AjBool     ajStrRandom(AjPStr *s);
AjBool     ajStrReverse(AjPStr* Pstr);

/* query */

ajint      ajStrCalcCountC(const AjPStr str, const char* txt);
ajint      ajStrCalcCountK(const AjPStr str, char chr);
AjBool     ajStrHasParentheses(const AjPStr str);
AjBool     ajStrIsAlnum (const AjPStr str);
AjBool     ajStrIsAlpha (const AjPStr str);
AjBool     ajStrIsBool (const AjPStr str);
AjBool     ajStrIsDouble (const AjPStr str);
AjBool     ajStrIsFloat (const AjPStr str);
AjBool     ajStrIsHex (const AjPStr str);
AjBool     ajStrIsInt (const AjPStr str);
AjBool     ajStrIsLong (const AjPStr str);
AjBool     ajStrIsLower (const AjPStr str);
AjBool     ajStrIsNum (const AjPStr str);
AjBool     ajStrIsUpper (const AjPStr str);
AjBool     ajStrIsWild (const AjPStr str);
AjBool     ajStrIsWhite (const AjPStr str);
AjBool     ajStrIsWord (const AjPStr str);
AjBool     ajStrWhole (const AjPStr str, ajint pos1, ajint pos2);

/* element retrieval */

char       ajStrGetCharFirst(const AjPStr str);
char       ajStrGetCharLast(const AjPStr str);
char       ajStrGetCharPos(const AjPStr str, ajint pos);
ajint      ajStrGetLen(const AjPStr str);
#define    MAJSTRGETLEN(str) str->Len
const char* ajStrGetPtr(const AjPStr str);
#define    MAJSTRGETPTR(str) str->Ptr
ajint      ajStrGetRes(const AjPStr str);
#define    MAJSTRGETRES(str) str->Res
ajint      ajStrGetRoom(const AjPStr str);
ajint      ajStrGetUse(const AjPStr str);
#define    MAJSTRGETUSE(str) str->Use
AjBool     ajStrGetValid (const AjPStr thys);

/* modifiable string retrieval */

char*      ajStrGetuniquePtr(AjPStr *Pstr);
AjPStr     ajStrGetuniqueStr(AjPStr *Pstr);

/* element assignment */

AjBool     ajStrSetClear (AjPStr* pthis);
AjBool     ajStrSetRes(AjPStr* Pstr, size_t size);
AjBool     ajStrSetResRound(AjPStr* Pstr, size_t size);
AjBool     ajStrSetValid(AjPStr *Pstr);
AjBool     ajStrSetValidLen(AjPStr* Pstr, size_t len);

/* string to datatype conversion functions */

AjBool     ajStrToBool (const AjPStr str, AjBool* Pval);
AjBool     ajStrToDouble (const AjPStr str, double* Pval);
AjBool     ajStrToFloat (const AjPStr str, float* Pval);
AjBool     ajStrToHex (const AjPStr str, ajint* Pval);
AjBool     ajStrToInt (const AjPStr str, ajint* Pval);
AjBool     ajStrToLong (const AjPStr thys, ajlong* result);

/* datatype to string conversion functions */

AjBool     ajStrFromBool (AjPStr* Pstr, AjBool val);
AjBool     ajStrFromDouble (AjPStr* Pstr, double val, ajint precision);
AjBool     ajStrFromDoubleExp (AjPStr* Pstr, double val, ajint precision);
AjBool     ajStrFromFloat (AjPStr* Pstr, float val, ajint precision);
AjBool     ajStrFromInt (AjPStr* Pstr, ajint val);
AjBool     ajStrFromLong (AjPStr* Pstr, ajlong val);

/* formatting */

AjBool     ajStrFmtBlock(AjPStr* pthis, ajint blksize);
AjBool     ajStrFmtLower(AjPStr* Pstr);
AjBool     ajStrFmtLowerSub(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrFmtQuote(AjPStr* Pstr);
AjBool     ajStrFmtTitle(AjPStr* Pstr);
AjBool     ajStrFmtUpper(AjPStr* Pstr);
AjBool     ajStrFmtUpperSub(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrFmtWrap(AjPStr* Pstr, ajint width );
AjBool     ajStrFmtWrapLeft(AjPStr* Pstr, ajint width, ajint left);

/* comparison */

AjBool     ajStrMatchC      (const AjPStr thys, const char* txt);
AjBool     ajStrMatchS      (const AjPStr thys, const AjPStr str);
AjBool     ajStrMatchCaseC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchCaseS  (const AjPStr thys, const AjPStr str);
AjBool     ajStrMatchWildC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchWildS  (const AjPStr thys, const AjPStr wild);
AjBool     ajStrMatchWildWordC (const AjPStr str, const char* text);
AjBool     ajStrMatchWildWordS (const AjPStr str, const AjPStr text);
AjBool     ajStrMatchWordAllS(const AjPStr str, const AjPStr str2);
AjBool     ajStrMatchWordOneS(const AjPStr str, const AjPStr str2);
AjBool     ajStrPrefixC(const AjPStr str, const char* txt2);
AjBool     ajStrPrefixS(const AjPStr str, const AjPStr str2);
AjBool     ajStrPrefixCaseC (const AjPStr str, const char* pref);
AjBool     ajStrPrefixCaseS (const AjPStr str, const AjPStr pref);
AjBool     ajStrSuffixC (const AjPStr thys, const char* suff);
AjBool     ajStrSuffixS (const AjPStr thys, const AjPStr suff);
AjBool     ajStrSuffixCaseC (const AjPStr str, const char* pref);
AjBool     ajStrSuffixCaseS (const AjPStr str, const AjPStr pref);

/* comparison (sorting) */

int        ajStrCmpC  (const AjPStr thys, const char *text);
int        ajStrCmpS(const AjPStr str, const AjPStr str2);
int        ajStrCmpCaseS (const AjPStr str1, const AjPStr str2);
ajint      ajStrCmpLenC (const AjPStr thys, const char *text, size_t len);
int        ajStrCmpLenS(const AjPStr str, const AjPStr str2, size_t len);
int        ajStrCmpWildC (const AjPStr thys, const char* text);
int        ajStrCmpWildS (const AjPStr thys, const AjPStr str);
int        ajStrVcmp  (const void* str1, const void* str2);

/* comparison (search) */

ajint      ajStrFindC  (const AjPStr str, const char* txt);
ajint      ajStrFindK  (const AjPStr str, const char chr);
ajint      ajStrFindS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindAnyC  (const AjPStr str, const char* txt);
ajint      ajStrFindAnyK(const AjPStr str, char chr);
ajint      ajStrFindAnyS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindCaseC (const AjPStr str, const char* txt);
ajint      ajStrFindCaseS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindlastC(const AjPStr str, const char* txt);
ajint      ajStrFindlastS(const AjPStr str, const AjPStr str2);

/* parsing */

AjBool     ajStrExtractFirst(const AjPStr str, AjPStr* Prest, AjPStr* Pword);
AjBool     ajStrExtractWord(const AjPStr str, AjPStr* Prest, AjPStr* Pword);
const AjPStr ajStrParseC(const AjPStr str, const char* txtdelim);
ajint      ajStrParseCount(const AjPStr line);
ajint      ajStrParseCountC(const AjPStr line, const char *txtdelim);
ajint      ajStrParseCountS(const AjPStr line, const AjPStr strdelim);
ajint      ajStrParseCountMultiC(const AjPStr str, const char *txtdelim);
ajint      ajStrParseSplit(const AjPStr str, AjPStr **PPstr);
const AjPStr ajStrParseWhite(const AjPStr str);

/*( debug */

void       ajStrStat (const char* title);
void       ajStrTrace (const AjPStr thys);
void       ajStrTraceFull (const AjPStr thys);
void       ajStrTraceTitle (const AjPStr thys, const char* title);

/* exit */

void       ajStrExit (void);

/* === string iterator === */

/* constructors */

AjIStr     ajStrIterNew (const AjPStr thys);
AjIStr     ajStrIterNewBack (const AjPStr thys);

/* destructors */

void       ajStrIterDel (AjIStr *iter);

/* tests */

AjBool     ajStrIterDone(const AjIStr iter);
AjBool     ajStrIterDoneBack(const AjIStr iter);

/* resets */

void       ajStrIterBegin(AjIStr iter);
void       ajStrIterEnd(AjIStr iter);

/* attributes */

const char* ajStrIterGetC(const AjIStr iter);
char       ajStrIterGetK(const AjIStr iter);

/* modifiers */

void       ajStrIterPutK(AjIStr iter, char chr);

/* stepping */

AjIStr     ajStrIterNext (AjIStr iter);
AjIStr     ajStrIterNextBack (AjIStr iter);

/* === string token parser === */

/* constructors */

AjPStrTok  ajStrTokenNewC(const AjPStr str, const char* txtdelim);
AjPStrTok  ajStrTokenNewS(const AjPStr str, const AjPStr strdelim);

/* destructors */

void       ajStrTokenDel(AjPStrTok* Ptoken);

/* assignment */

AjBool     ajStrTokenAssign(AjPStrTok* Ptoken, const AjPStr str);
AjBool     ajStrTokenAssignC(AjPStrTok* Ptoken, const AjPStr str,
			     const char* txtdelim);
AjBool     ajStrTokenAssignS(AjPStrTok* Ptoken, const AjPStr str,
			     const AjPStr strdelim);

/* reset */

void       ajStrTokenReset(AjPStrTok* Ptoken);

/* debug */

void       ajStrTokenTrace(const AjPStrTok token);

/* parsing */

AjBool     ajStrTokenNextFind(AjPStrTok* Ptoken, AjPStr* Pstr);
AjBool     ajStrTokenNextFindC(AjPStrTok* Ptoken, const char* strdelim,
			       AjPStr* Pstr);
AjBool     ajStrTokenNextParse(AjPStrTok* Ptoken, AjPStr* Pstr);
AjBool     ajStrTokenNextParseC(AjPStrTok* Ptoken, const char* txtdelim,
				AjPStr* Pstr);
AjBool     ajStrTokenNextParseS(AjPStrTok* Ptoken, const AjPStr strdelim,
				AjPStr* Pstr);
AjBool     ajStrTokenRestParse(AjPStrTok* Ptoken, AjPStr* Pstr);




AjBool     ajStrTruncate (AjPStr* Pstr, ajint pos);

/* =====================================================================
** Deprecated functions - renamed or replaced
** The __deprecated tag is used by the gcc compiler to report calls
** for other compilers it is defined as an empty string (i.e. removed)
** ===================================================================== */

void       __deprecated ajCharFree (char** txt);
char       __deprecated *ajCharNew (const AjPStr thys);
char       __deprecated *ajCharNewL (size_t size);
char       __deprecated *ajCharNewLS (size_t size, const AjPStr thys);
ajint      __deprecated ajCharPos (const char* txt, ajint ipos);
void       __deprecated ajCharToLower (char *txt);
void       __deprecated ajCharToUpper (char *txt);

AjBool     __deprecated ajStrApp  (AjPStr* pthis, const AjPStr src);
AjBool     __deprecated ajStrAppC (AjPStr* pthis, const char *txt);
AjBool     __deprecated ajStrAppCI (AjPStr* pthis, const char *txt, size_t i);
AjBool     __deprecated ajStrAppK (AjPStr* pthis, const char chr);
AjBool     __deprecated ajStrAppKI (AjPStr* pthis, const char chr,
				    ajint number);
AjBool     __deprecated ajStrAppSub (AjPStr* pthis, const AjPStr src,
				     ajint begin, ajint end);
void       __deprecated ajStrArrayDel (AjPStr** pthis);
AjBool     __deprecated ajStrAss   (AjPStr* pthis, AjPStr str); /* not const */
AjBool     __deprecated ajStrAssC  (AjPStr* pthis, const char* txt);
AjBool     __deprecated ajStrAssCI (AjPStr* pthis, const char* txt, size_t i);
AjBool     __deprecated ajStrAssCL (AjPStr* pthis, const char* txt, size_t i);
AjBool     __deprecated ajStrAssI  (AjPStr* pthis, const AjPStr str, size_t i);
AjBool     __deprecated ajStrAssK  (AjPStr* pthis, const char text);
AjBool     __deprecated ajStrAssL  (AjPStr* pthis, const AjPStr str, size_t i);
AjBool     __deprecated ajStrAssS  (AjPStr* pthis, const AjPStr str);
AjBool     __deprecated ajStrAssSub  (AjPStr* pthis, const AjPStr str,
				      ajint begin, ajint end);
AjBool     __deprecated ajStrAssSubC (AjPStr* pthis, const char* txt,
				      ajint begin, ajint end);
AjBool     __deprecated ajStrBlock (AjPStr* pthis, ajint blksize);
char       __deprecated ajStrChar (const AjPStr thys, ajint pos);
AjBool     __deprecated ajStrChomp (AjPStr* pthis);
AjBool     __deprecated ajStrChompC (AjPStr* pthis, const char* delim);
AjBool     __deprecated ajStrChompEnd (AjPStr* pthis);
AjBool     __deprecated ajStrChop  (AjPStr* pthis);
AjBool     __deprecated ajStrClean (AjPStr* s);
AjBool     __deprecated ajStrCleanWhite (AjPStr* s);
AjBool     __deprecated ajStrClear (AjPStr* pthis);
int        __deprecated ajStrCmp(const void* str, const void* str2);
int        __deprecated ajStrCmpCase(const AjPStr str, const AjPStr str2);
int        __deprecated ajStrCmpCaseCC (const char* str1, const char* str2);
int        __deprecated ajStrCmpO  (const AjPStr thys, const AjPStr anoth);
int        __deprecated ajStrCmpWild(const AjPStr str, const AjPStr str2);
int        __deprecated ajStrCmpWildCC (const char* str, const char* text);
AjBool     __deprecated ajStrConvert   (AjPStr* pthis, const AjPStr oldc,
					const AjPStr newc);
AjBool     __deprecated ajStrConvertCC (AjPStr* pthis, const char* oldc,
					const char* newc);
AjBool     __deprecated ajStrCopy (AjPStr* pthis, AjPStr str); /* not const */
AjBool     __deprecated ajStrCopyC (AjPStr* pthis, const char* str);
ajint      __deprecated ajStrCountC (const AjPStr thys, const char* str);
ajint      __deprecated ajStrCountK (const AjPStr thys, char ch);
AjBool     __deprecated ajStrCut(AjPStr* pthis, ajint begin, ajint end);
void       __deprecated ajStrDegap(AjPStr* pthis);
AjBool     __deprecated ajStrDelReuse (AjPStr* pthis);
AjBool     __deprecated ajStrDelim (AjPStr* pthis, AjPStrTok *ptoken,
				    const char *delim);
AjPStr     __deprecated ajStrDup (AjPStr thys);
void       __deprecated ajStrFill (AjPStr* pthis, ajint count, char fill);
ajint      __deprecated ajStrFind(const AjPStr str, const AjPStr str2);
ajint      __deprecated ajStrFindCase(const AjPStr str, const AjPStr str2);
ajint      __deprecated ajStrFindK  (const AjPStr thys, const char chr);
void       __deprecated ajStrFix (AjPStr *pthys);
void       __deprecated ajStrFixI (AjPStr *pthys, ajint ilen);
AjBool     __deprecated ajStrFromDoubleE(AjPStr* Pstr, double val,
					 ajint precision);
AjBool     __deprecated ajStrInsert(AjPStr* Pstr, ajint pos,
				    const AjPStr str );
AjBool     __deprecated ajStrIsSpace (const AjPStr thys);
AjBool     __deprecated ajStrKeepAlphaC (AjPStr* pthis, const char* chars);
AjBool     __deprecated ajStrKeepC (AjPStr* pthis, const char* chars);
ajint       __deprecated ajStrLen (const AjPStr thys);
ajint      __deprecated ajStrListToArray(const AjPStr str, AjPStr **array);
AjBool     __deprecated ajStrMatchCaseCC (const char* thys, const char* text);
AjBool     __deprecated ajStrMatchCC     (const char* thys, const char* text);
AjBool     __deprecated ajStrMatchWildCC (const char* str, const char* text);
AjBool     __deprecated ajStrMatchWildCO (const char* str, const AjPStr wild);
AjBool     __deprecated ajStrMatchWord   (const AjPStr str, const AjPStr text);
AjBool     __deprecated ajStrMatchWordCC (const char* str, const char* text);
AjBool     __deprecated ajStrMod (AjPStr* pthis);
AjBool     __deprecated ajStrModL (AjPStr* pthis, size_t size);
AjBool     __deprecated ajStrModMinL (AjPStr* pthis, size_t size);
AjPStr     __deprecated ajStrNewL (size_t size);
AjBool     __deprecated ajStrMatch(const AjPStr str, const AjPStr str2);
AjBool     __deprecated ajStrMatchCase(const AjPStr str, const AjPStr str2);
AjBool     __deprecated ajStrMatchWild(const AjPStr str, const AjPStr str2);
int        __deprecated ajStrNCmpC(const AjPStr str, const char* txt,
				   ajint len);
ajint 	   __deprecated ajStrNCmpCaseCC (const char* str1, const char* str2,
					 ajint len);
ajint      __deprecated ajStrNCmpO (const AjPStr thys, const AjPStr anoth,
				    ajint n);
AjPStr     __deprecated ajStrNewCL (const char *txt, size_t size);
AjPStr     __deprecated ajStrNewCIL (const char *txt, ajint len, size_t size);
const AjPStr __deprecated ajStrNull(void);
AjBool     __deprecated ajStrParentheses(const AjPStr s);
ajint      __deprecated ajStrPos  (const AjPStr thys, ajint ipos);
ajint      __deprecated ajStrPosI (const AjPStr thys, ajint imin, ajint ipos);
ajint      __deprecated ajStrPosII (ajint ilen, ajint imin, ajint ipos);
AjBool     __deprecated ajStrPrefix(const AjPStr str, const AjPStr str2);
AjBool     __deprecated ajStrPrefixCase(const AjPStr str, const AjPStr str2);
AjBool     __deprecated ajStrPrefixCaseCC (const char *str, const char* pref);
AjBool     __deprecated ajStrPrefixCaseCO (const char* thys,
					   const AjPStr pref);
AjBool     __deprecated ajStrPrefixCC (const char *str, const char* pref);
AjBool     __deprecated ajStrPrefixCO (const char *str, const AjPStr thys);
void       __deprecated ajStrQuote(AjPStr *s);
AjBool     __deprecated ajStrReplace  (AjPStr* pthis, ajint pos1,
			  const AjPStr overwrite, ajint len);
AjBool     __deprecated ajStrReplaceC (AjPStr* pthis, ajint pos1,
			  const char* overwrite, ajint len);
AjBool     __deprecated ajStrReplaceK (AjPStr* pthis, ajint pos1,
			  char overwrite, ajint len);
AjBool     __deprecated ajStrReplaceS( AjPStr* pthis,
				       ajint begin, const AjPStr overwrite);
ajint      __deprecated ajStrRef(const AjPStr thys);
void       __deprecated ajStrRemoveCharsC(AjPStr* thys, const char *strng);
void       __deprecated ajStrRemoveNewline(AjPStr* pthis);
AjBool     __deprecated ajStrRev (AjPStr* pthis);
ajint      __deprecated ajStrRFindC (const AjPStr thys, const char *text);
ajint      __deprecated ajStrRoom(const AjPStr thys);
AjBool     __deprecated ajStrSet  (AjPStr* pthis, const AjPStr str);
AjBool     __deprecated ajStrSetC  (AjPStr* pthis, const char* str);
ajint      __deprecated ajStrSize (const AjPStr thys);
const char __deprecated *ajStrStr (const AjPStr thys);
char       __deprecated *ajStrStrMod (AjPStr* thys);
AjBool     __deprecated ajStrSub (AjPStr* pthis, ajint begin, ajint len);
AjBool     __deprecated ajStrSubstitute   (AjPStr* pthis, const AjPStr replace,
			      const AjPStr putin);
AjBool     __deprecated ajStrSubstituteCC (AjPStr* pthis, const char* replace,
			      const char* putin);
AjBool     __deprecated ajStrSubstituteKK (AjPStr* pthis, char replace,
			      char putin);
AjBool     __deprecated ajStrSuffix(const AjPStr str, const AjPStr str2);
AjBool     __deprecated ajStrSuffixCC (const char *str, const char* suff);
AjBool     __deprecated ajStrSuffixCO (const char *str, const AjPStr suff);
AjBool     __deprecated ajStrToLower (AjPStr* pthis);
AjBool     __deprecated ajStrToLowerII(AjPStr* pthis, ajint begin, ajint end);
AjBool     __deprecated ajStrToTitle (AjPStr* pthis);
AjBool     __deprecated ajStrToUpper (AjPStr* pthis);
AjBool     __deprecated ajStrToUpperII(AjPStr* pthis, ajint begin, ajint end);
AjBool     __deprecated ajStrTrim  (AjPStr* pthis, ajint num);
AjBool     __deprecated ajStrTruncate(AjPStr* Pstr, ajint pos);
AjBool     __deprecated ajStrUncomment (AjPStr* text);
AjBool     __deprecated ajStrUncommentStart (AjPStr* text);
AjBool     __deprecated ajStrWildPrefix (AjPStr* str);
AjBool     __deprecated ajStrWrap (AjPStr* pthis, ajint width);
AjBool     __deprecated ajStrWrapLeft (AjPStr* pthis, ajint width, ajint left);




AjIStr     __deprecated ajStrIter (const AjPStr thys);
AjIStr     __deprecated ajStrIterBack (const AjPStr thys);
AjBool     __deprecated ajStrIterBackDone(AjIStr iter);
AjIStr     __deprecated ajStrIterBackNext (AjIStr iter);
void       __deprecated ajStrIterFree (AjIStr *iter);
AjBool     __deprecated ajStrIterMore(AjIStr iter);
AjBool     __deprecated ajStrIterMoreBack(AjIStr iter);


AjPStr     __deprecated ajStrTok (const AjPStr thys);
AjPStr     __deprecated ajStrTokC (const AjPStr thys, const char* delim);
AjPStr     __deprecated ajStrTokCC (const char* thys, const char* delim);
AjBool     __deprecated ajStrToken (AjPStr* pthis, AjPStrTok *ptoken,
				    const char *delim);
AjBool     __deprecated ajStrTokenAss (AjPStrTok *ptok, const AjPStr thys,
			  const char *delim);
void       __deprecated ajStrTokenClear (AjPStrTok *token);
ajint      __deprecated ajStrTokenCount(const AjPStr line, const char *delim);
ajint      __deprecated ajStrTokenCountR(const AjPStr line, const char *delim);
AjPStrTok  __deprecated ajStrTokenInit (const AjPStr thys, const char *delim);
AjBool     __deprecated ajStrTokenRest (AjPStr* pthis, AjPStrTok* ptoken);
void       __deprecated ajStrTokenTrace (const AjPStrTok tok);

/*#define    MAJSTRLEN(str) str->Len*/
const char __deprecated *MAJSTRSTR(const AjPStr thys);
ajint __deprecated MAJSTRLEN(const AjPStr thys);
ajint __deprecated MAJSTRSIZE(const AjPStr thys);
ajint __deprecated MAJSTRREF(const AjPStr thys);

/*
** End of prototype definitions
*/


/*#define    MAJSTRREF(str) str->Use*/
/*#define    MAJSTRSIZE(str) str->Res*/
/*#define    MAJSTRSUBK(str,pos,c) str->Ptr[pos]=c*/
/*#define    MAJSTRSTR(str) str->Ptr*/

#endif

#ifdef __cplusplus
}
#endif

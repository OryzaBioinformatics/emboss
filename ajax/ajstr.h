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
** @attr Res [ajuint] Reserved bytes (usable for expanding in place)
** @attr Len [ajuint] Length of current string, excluding NULL at end
** @attr Ptr [char*] The string, as a NULL-terminated C string.
** @attr Use [ajuint] Use count: 1 for single reference, more if several
**                   pointers share the same string.
**                   Must drop to 0 before deleting. Modifying means making
**                   a new string if not 1.
** @attr Padding [ajint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSStr {
  ajuint Res;
  ajuint Len;
  char *Ptr;
  ajuint Use;
  ajint Padding;
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
** @attr Pos [ajuint] Position in string
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajStrTokenInit Generates a string token parser object
** @delete ajStrTokenClear Destroys a string token parser
** @@
******************************************************************************/

typedef struct AjSStrTok {
  AjPStr String;
  AjPStr Delim;
  ajuint Pos;
  char Padding[4];
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
char*      ajCharNewRes(ajuint size);
char*      ajCharNewResC(const char* txt, ajuint size);
char*      ajCharNewResS(const AjPStr str, ajuint size);
char*      ajCharNewResLenC(const char* txt, ajuint size, ajuint len);

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
AjBool     ajCharMatchWildCaseC(const char* txt1, const char* txt2);
AjBool     ajCharMatchWildCaseS(const char* txt, const AjPStr str);
AjBool     ajCharMatchWildNextC(const char* txt1, const char* txt2);
AjBool     ajCharMatchWildWordC(const char* str, const char* txt);
AjBool     ajCharMatchWildNextCaseC(const char* txt1, const char* txt2);
AjBool     ajCharMatchWildWordCaseC(const char* str, const char* txt);
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
int        ajCharCmpCaseLen(const char* txt1, const char* txt2, ajuint len);
ajint      ajCharCmpWild(const char* txt1, const char* txt2);
ajint      ajCharCmpWildCase(const char* txt1, const char* txt2);

/* parsing */

AjPStr     ajCharParseC (const char* txt, const char* delim);

/* === AjPStr string === */

/* constructors */

AjPStr     ajStrNew (void);
AjPStr     ajStrNewC (const char *txt);
AjPStr     ajStrNewS (const AjPStr str);
AjPStr     ajStrNewRef(AjPStr str);
AjPStr     ajStrNewRes(ajuint size);
AjPStr     ajStrNewResC (const char *txt, ajuint size);
AjPStr     ajStrNewResS (const AjPStr str, ajuint size);
AjPStr     ajStrNewResLenC (const char *txt, ajuint size, ajuint len);

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
AjBool     ajStrAssignLenC(AjPStr* Pstr, const char* txt, ajuint ilen);
AjBool     ajStrAssignRef(AjPStr* Pstr, AjPStr refstr);
AjBool     ajStrAssignResC(AjPStr* Pstr, ajuint size, const char* txt);
AjBool     ajStrAssignResS(AjPStr* Pstr, ajuint i, const AjPStr str);
AjBool     ajStrAssignSubC(AjPStr* Pstr, const char* txt,
			   ajint pos1, ajint pos2);
AjBool     ajStrAssignSubS(AjPStr* Pstr, const AjPStr str,
			  ajint pos1, ajint pos2);

/* combination */

AjBool     ajStrAppendC(AjPStr* Pstr, const char* txt);
AjBool     ajStrAppendK(AjPStr* Pstr, char chr);
AjBool     ajStrAppendS(AjPStr* Pstr, const AjPStr str);
AjBool     ajStrAppendCountK(AjPStr* Pstr, char chr, ajuint num);
AjBool     ajStrAppendLenC(AjPStr* Pstr, const char* txt, ajuint len);
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
		      ajuint num);
AjBool     ajStrPasteMaxC (AjPStr* Pstr, ajint pos, const char* txt,
		     ajuint n);
AjBool     ajStrPasteMaxS( AjPStr* Pstr, ajint pos, const AjPStr str,
		       ajuint n);

/* cut */

AjBool     ajStrCutComments(AjPStr* Pstr);
AjBool     ajStrCutCommentsStart(AjPStr* Pstr);
AjBool     ajStrCutEnd(AjPStr* Pstr, ajuint len);
AjBool     ajStrCutRange(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrCutStart(AjPStr* Pstr, ajuint len);
AjBool     ajStrKeepRange(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrKeepSetC(AjPStr* Pstr, const char* txt);
AjBool     ajStrKeepSetS(AjPStr* Pstr, const AjPStr str);
AjBool     ajStrKeepSetAlpha(AjPStr* Pstr);
AjBool     ajStrKeepSetAlphaC(AjPStr* Pstr, const char* txt);
AjBool     ajStrKeepSetAlphaS(AjPStr* Pstr, const AjPStr str);
AjBool     ajStrKeepSetAlphaRest(AjPStr* Pstr, AjPStr* Prest);
AjBool     ajStrKeepSetAlphaRestC(AjPStr* Pstr, const char* txt,
				  AjPStr* Prest);
AjBool     ajStrKeepSetAlphaRestS(AjPStr* Pstr, const AjPStr str,
				  AjPStr* Prest);
AjBool     ajStrQuoteStrip(AjPStr *Pstr);
AjBool     ajStrQuoteStripAll(AjPStr *Pstr);
AjBool     ajStrRemoveGap(AjPStr* thys);
AjBool     ajStrRemoveHtml(AjPStr* pthis);
AjBool     ajStrRemoveLastNewline(AjPStr* Pstr);
AjBool     ajStrRemoveSetC(AjPStr* Pstr, const char *txt);
AjBool     ajStrRemoveWhite(AjPStr* Pstr);
AjBool     ajStrRemoveWhiteExcess(AjPStr* Pstr);
AjBool     ajStrRemoveWhiteSpaces(AjPStr* Pstr);
AjBool     ajStrRemoveWild(AjPStr* Pstr);
AjBool     ajStrTrimC (AjPStr* pthis, const char* txt);
AjBool     ajStrTrimEndC (AjPStr* Pstr, const char* txt);
AjBool     ajStrTrimStartC (AjPStr* Pstr, const char* txt);
AjBool     ajStrTrimWhite(AjPStr* Pstr);
AjBool     ajStrTrimWhiteEnd(AjPStr* Pstr);
AjBool     ajStrTruncateLen(AjPStr* Pstr, ajuint len);
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
AjBool     ajStrExchangePosCC(AjPStr* Pstr, ajint ipos, const char* txt,
			      const char* txtnew);
AjBool     ajStrExchangeSetCC(AjPStr* Pstr, const char* txt,
			      const char* newc);
AjBool     ajStrExchangeSetSS(AjPStr* Pstr, const AjPStr str,
			    const AjPStr strnew);
AjBool     ajStrExchangeSetRestCK(AjPStr* Pstr, const char* txt,
				  char chr);
AjBool     ajStrExchangeSetRestSK(AjPStr* Pstr, const AjPStr str,
				  char chr);
AjBool     ajStrRandom(AjPStr *s);
AjBool     ajStrReverse(AjPStr* Pstr);

/* query */

ajuint     ajStrCalcCountC(const AjPStr str, const char* txt);
ajuint     ajStrCalcCountK(const AjPStr str, char chr);
AjBool     ajStrHasParentheses(const AjPStr str);
AjBool     ajStrIsAlnum (const AjPStr str);
AjBool     ajStrIsAlpha (const AjPStr str);
AjBool     ajStrIsBool (const AjPStr str);
AjBool     ajStrIsCharsetC(const AjPStr str, const char* txt);
AjBool     ajStrIsCharsetS(const AjPStr str, const AjPStr str2);
AjBool     ajStrIsCharsetCaseC(const AjPStr str, const char* txt);
AjBool     ajStrIsCharsetCaseS(const AjPStr str, const AjPStr str2);
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
ajuint     ajStrGetLen(const AjPStr str);
#define    MAJSTRGETLEN(str) str->Len
const char* ajStrGetPtr(const AjPStr str);
#define    MAJSTRGETPTR(str) str->Ptr
ajuint      ajStrGetRes(const AjPStr str);
#define    MAJSTRGETRES(str) str->Res
ajuint     ajStrGetRoom(const AjPStr str);
ajuint     ajStrGetUse(const AjPStr str);
#define    MAJSTRGETUSE(str) str->Use
AjBool     ajStrGetValid (const AjPStr thys);

/* modifiable string retrieval */

char*      ajStrGetuniquePtr(AjPStr *Pstr);
AjPStr     ajStrGetuniqueStr(AjPStr *Pstr);

/* element assignment */

AjBool     ajStrSetClear (AjPStr* pthis);
AjBool     ajStrSetRes(AjPStr* Pstr, ajuint size);
AjBool     ajStrSetResRound(AjPStr* Pstr, ajuint size);
AjBool     ajStrSetValid(AjPStr *Pstr);
AjBool     ajStrSetValidLen(AjPStr* Pstr, ajuint len);

/* string to datatype conversion functions */

AjBool     ajStrToBool (const AjPStr str, AjBool* Pval);
AjBool     ajStrToDouble (const AjPStr str, double* Pval);
AjBool     ajStrToFloat (const AjPStr str, float* Pval);
AjBool     ajStrToHex (const AjPStr str, ajint* Pval);
AjBool     ajStrToInt (const AjPStr str, ajint* Pval);
AjBool     ajStrToLong (const AjPStr thys, ajlong* result);
AjBool     ajStrToUint (const AjPStr str, ajuint* Pval);

/* datatype to string conversion functions */

AjBool     ajStrFromBool (AjPStr* Pstr, AjBool val);
AjBool     ajStrFromDouble (AjPStr* Pstr, double val, ajint precision);
AjBool     ajStrFromDoubleExp (AjPStr* Pstr, double val, ajint precision);
AjBool     ajStrFromFloat (AjPStr* Pstr, float val, ajint precision);
AjBool     ajStrFromInt (AjPStr* Pstr, ajint val);
AjBool     ajStrFromLong (AjPStr* Pstr, ajlong val);
AjBool     ajStrFromUint (AjPStr* Pstr, ajuint val);

/* formatting */

AjBool     ajStrFmtBlock(AjPStr* pthis, ajuint blksize);
AjBool     ajStrFmtLower(AjPStr* Pstr);
AjBool     ajStrFmtLowerSub(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrFmtQuote(AjPStr* Pstr);
AjBool     ajStrFmtTitle(AjPStr* Pstr);
AjBool     ajStrFmtUpper(AjPStr* Pstr);
AjBool     ajStrFmtUpperSub(AjPStr* Pstr, ajint pos1, ajint pos2);
AjBool     ajStrFmtWrap(AjPStr* Pstr, ajuint width );
AjBool     ajStrFmtWrapAt(AjPStr* Pstr, ajuint width, char ch);
AjBool     ajStrFmtWrapLeft(AjPStr* Pstr, ajuint width, ajuint left);

/* comparison */

AjBool     ajStrMatchC      (const AjPStr thys, const char* txt);
AjBool     ajStrMatchS      (const AjPStr thys, const AjPStr str);
AjBool     ajStrMatchCaseC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchCaseS  (const AjPStr thys, const AjPStr str);
AjBool     ajStrMatchWildC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchWildS  (const AjPStr thys, const AjPStr wild);
AjBool     ajStrMatchWildCaseC  (const AjPStr thys, const char* text);
AjBool     ajStrMatchWildCaseS  (const AjPStr thys, const AjPStr wild);
AjBool     ajStrMatchWildWordC (const AjPStr str, const char* text);
AjBool     ajStrMatchWildWordS (const AjPStr str, const AjPStr text);
AjBool     ajStrMatchWildWordCaseC (const AjPStr str, const char* text);
AjBool     ajStrMatchWildWordCaseS (const AjPStr str, const AjPStr text);
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

int        ajStrCmpC(const AjPStr thys, const char *text);
int        ajStrCmpS(const AjPStr str, const AjPStr str2);
int        ajStrCmpCaseS (const AjPStr str1, const AjPStr str2);
ajint      ajStrCmpLenC (const AjPStr thys, const char *text, ajuint len);
int        ajStrCmpLenS(const AjPStr str, const AjPStr str2, ajuint len);
int        ajStrCmpWildC (const AjPStr thys, const char* text);
int        ajStrCmpWildS (const AjPStr thys, const AjPStr str);
int        ajStrCmpWildCaseC (const AjPStr thys, const char* text);
int        ajStrCmpWildCaseS (const AjPStr thys, const AjPStr str);
int        ajStrVcmp  (const void* str1, const void* str2);

/* comparison (search) */

ajint      ajStrFindC  (const AjPStr str, const char* txt);
ajint      ajStrFindS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindAnyC  (const AjPStr str, const char* txt);
ajint      ajStrFindAnyK(const AjPStr str, char chr);
ajint      ajStrFindAnyS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindCaseC (const AjPStr str, const char* txt);
ajint      ajStrFindCaseS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindRestC (const AjPStr str, const char* txt);
ajint      ajStrFindRestS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindRestCaseC (const AjPStr str, const char* txt);
ajint      ajStrFindRestCaseS (const AjPStr str, const AjPStr str2);
ajint      ajStrFindlastC(const AjPStr str, const char* txt);
ajint      ajStrFindlastS(const AjPStr str, const AjPStr str2);

/* parsing */

AjBool     ajStrExtractFirst(const AjPStr str, AjPStr* Prest, AjPStr* Pword);
AjBool     ajStrExtractWord(const AjPStr str, AjPStr* Prest, AjPStr* Pword);
const AjPStr ajStrParseC(const AjPStr str, const char* txtdelim);
ajuint     ajStrParseCount(const AjPStr line);
ajuint     ajStrParseCountC(const AjPStr line, const char *txtdelim);
ajuint     ajStrParseCountS(const AjPStr line, const AjPStr strdelim);
ajuint     ajStrParseCountMultiC(const AjPStr str, const char *txtdelim);
ajuint     ajStrParseSplit(const AjPStr str, AjPStr **PPstr);
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




/* =====================================================================
** Deprecated functions - renamed or replaced
** __deprecated The  tag is used by the gcc compiler to report calls
** for other compilers it is defined as an empty string (i.e. removed)
** ===================================================================== */

__deprecated void        ajCharFree (char** txt);
__deprecated char        *ajCharNew (const AjPStr thys);
__deprecated char        *ajCharNewL (size_t size);
__deprecated char        *ajCharNewLS (size_t size, const AjPStr thys);
__deprecated ajint       ajCharPos (const char* txt, ajint ipos);
__deprecated void        ajCharToLower (char *txt);
__deprecated void        ajCharToUpper (char *txt);

__deprecated AjBool      ajStrApp  (AjPStr* pthis, const AjPStr src);
__deprecated AjBool      ajStrAppC (AjPStr* pthis, const char *txt);
__deprecated AjBool      ajStrAppCI (AjPStr* pthis, const char *txt, size_t i);
__deprecated AjBool      ajStrAppK (AjPStr* pthis, const char chr);
__deprecated AjBool      ajStrAppKI (AjPStr* pthis, const char chr,
				    ajint number);
__deprecated AjBool      ajStrAppSub (AjPStr* pthis, const AjPStr src,
				     ajint begin, ajint end);
__deprecated void        ajStrArrayDel (AjPStr** pthis);
__deprecated AjBool      ajStrAss   (AjPStr* pthis, AjPStr str); /* not const */
__deprecated AjBool      ajStrAssC  (AjPStr* pthis, const char* txt);
__deprecated AjBool      ajStrAssCI (AjPStr* pthis, const char* txt, size_t i);
__deprecated AjBool      ajStrAssCL (AjPStr* pthis, const char* txt, size_t i);
__deprecated AjBool      ajStrAssI  (AjPStr* pthis, const AjPStr str, size_t i);
__deprecated AjBool      ajStrAssK  (AjPStr* pthis, const char text);
__deprecated AjBool      ajStrAssL  (AjPStr* pthis, const AjPStr str, size_t i);
__deprecated AjBool      ajStrAssS  (AjPStr* pthis, const AjPStr str);
__deprecated AjBool      ajStrAssSub  (AjPStr* pthis, const AjPStr str,
				      ajint begin, ajint end);
__deprecated AjBool      ajStrAssSubC (AjPStr* pthis, const char* txt,
				      ajint begin, ajint end);
__deprecated AjBool      ajStrBlock (AjPStr* pthis, ajint blksize);
__deprecated char        ajStrChar (const AjPStr thys, ajint pos);
__deprecated AjBool      ajStrChomp (AjPStr* pthis);
__deprecated AjBool      ajStrChompC (AjPStr* pthis, const char* delim);
__deprecated AjBool      ajStrChompEnd (AjPStr* pthis);
__deprecated AjBool      ajStrChop  (AjPStr* pthis);
__deprecated AjBool      ajStrClean (AjPStr* s);
__deprecated AjBool      ajStrCleanWhite (AjPStr* s);
__deprecated AjBool      ajStrClear (AjPStr* pthis);
__deprecated int         ajStrCmp(const void* str, const void* str2);
__deprecated int         ajStrCmpCase(const AjPStr str, const AjPStr str2);
__deprecated int         ajStrCmpCaseCC (const char* str1, const char* str2);
__deprecated int         ajStrCmpO  (const AjPStr thys, const AjPStr anoth);
__deprecated int         ajStrCmpWild(const AjPStr str, const AjPStr str2);
__deprecated int         ajStrCmpWildCC (const char* str, const char* text);
__deprecated AjBool      ajStrConvert   (AjPStr* pthis, const AjPStr oldc,
					const AjPStr newc);
__deprecated AjBool      ajStrConvertCC (AjPStr* pthis, const char* oldc,
					const char* newc);
__deprecated AjBool      ajStrCopy (AjPStr* pthis, AjPStr str); /* not const */
__deprecated AjBool      ajStrCopyC (AjPStr* pthis, const char* str);
__deprecated ajint       ajStrCountC (const AjPStr thys, const char* str);
__deprecated ajint       ajStrCountK (const AjPStr thys, char ch);
__deprecated AjBool      ajStrCut(AjPStr* pthis, ajint begin, ajint end);
__deprecated void        ajStrDegap(AjPStr* pthis);
__deprecated AjBool      ajStrDelReuse (AjPStr* pthis);
__deprecated AjBool      ajStrDelim (AjPStr* pthis, AjPStrTok *ptoken,
				    const char *delim);
__deprecated AjPStr      ajStrDup (AjPStr thys);
__deprecated void        ajStrFill (AjPStr* pthis, ajint count, char fill);
__deprecated ajint       ajStrFind(const AjPStr str, const AjPStr str2);
__deprecated ajint       ajStrFindCase(const AjPStr str, const AjPStr str2);
__deprecated ajint       ajStrFindK  (const AjPStr thys, const char chr);
__deprecated void        ajStrFix (AjPStr *pthys);
__deprecated void        ajStrFixI (AjPStr *pthys, ajint ilen);
__deprecated AjBool      ajStrFromDoubleE(AjPStr* Pstr, double val,
					 ajint precision);
__deprecated AjBool      ajStrInsert(AjPStr* Pstr, ajint pos,
				    const AjPStr str );
__deprecated AjBool      ajStrIsSpace (const AjPStr thys);
__deprecated AjBool      ajStrKeepAlphaC (AjPStr* pthis, const char* chars);
__deprecated AjBool      ajStrKeepC (AjPStr* pthis, const char* chars);
__deprecated ajint        ajStrLen (const AjPStr thys);
__deprecated ajint       ajStrListToArray(const AjPStr str, AjPStr **array);
__deprecated AjBool      ajStrMatchCaseCC (const char* thys, const char* text);
__deprecated AjBool      ajStrMatchCC     (const char* thys, const char* text);
__deprecated AjBool      ajStrMatchWildCC (const char* str, const char* text);
__deprecated AjBool      ajStrMatchWildCO (const char* str, const AjPStr wild);
__deprecated AjBool      ajStrMatchWord   (const AjPStr str, const AjPStr text);
__deprecated AjBool      ajStrMatchWordCC (const char* str, const char* text);
__deprecated AjBool      ajStrMod (AjPStr* pthis);
__deprecated AjBool      ajStrModL (AjPStr* pthis, size_t size);
__deprecated AjBool      ajStrModMinL (AjPStr* pthis, ajuint size);
__deprecated AjPStr      ajStrNewL (size_t size);
__deprecated AjBool      ajStrMatch(const AjPStr str, const AjPStr str2);
__deprecated AjBool      ajStrMatchCase(const AjPStr str, const AjPStr str2);
__deprecated AjBool      ajStrMatchWild(const AjPStr str, const AjPStr str2);
__deprecated int         ajStrNCmpC(const AjPStr str, const char* txt,
				   ajint len);
__deprecated ajint 	    ajStrNCmpCaseCC (const char* str1, const char* str2,
					 ajint len);
__deprecated ajint       ajStrNCmpO (const AjPStr thys, const AjPStr anoth,
				    ajint n);
__deprecated AjPStr      ajStrNewCL (const char *txt, size_t size);
__deprecated AjPStr      ajStrNewCIL (const char *txt, ajint len, size_t size);
__deprecated const AjPStr  ajStrNull(void);
__deprecated AjBool      ajStrParentheses(const AjPStr s);
__deprecated ajint       ajStrPos  (const AjPStr thys, ajint ipos);
__deprecated ajint       ajStrPosI (const AjPStr thys, ajint imin, ajint ipos);
__deprecated ajint       ajStrPosII (ajint ilen, ajint imin, ajint ipos);
__deprecated AjBool      ajStrPrefix(const AjPStr str, const AjPStr str2);
__deprecated AjBool      ajStrPrefixCase(const AjPStr str, const AjPStr str2);
__deprecated AjBool      ajStrPrefixCaseCC (const char *str, const char* pref);
__deprecated AjBool      ajStrPrefixCaseCO (const char* thys,
					   const AjPStr pref);
__deprecated AjBool      ajStrPrefixCC (const char *str, const char* pref);
__deprecated AjBool      ajStrPrefixCO (const char *str, const AjPStr thys);
__deprecated void        ajStrQuote(AjPStr *s);
__deprecated AjBool      ajStrReplace  (AjPStr* pthis, ajint pos1,
			  const AjPStr overwrite, ajint len);
__deprecated AjBool      ajStrReplaceC (AjPStr* pthis, ajint pos1,
			  const char* overwrite, ajint len);
__deprecated AjBool      ajStrReplaceK (AjPStr* pthis, ajint pos1,
			  char overwrite, ajint len);
__deprecated AjBool      ajStrReplaceS( AjPStr* pthis,
				       ajint begin, const AjPStr overwrite);
__deprecated ajint       ajStrRef(const AjPStr thys);
__deprecated void        ajStrRemoveCharsC(AjPStr* thys, const char *strng);
__deprecated void        ajStrRemoveNewline(AjPStr* pthis);
__deprecated AjBool      ajStrRev (AjPStr* pthis);
__deprecated ajint       ajStrRFindC (const AjPStr thys, const char *text);
__deprecated ajint       ajStrRoom(const AjPStr thys);
__deprecated AjBool      ajStrSet  (AjPStr* pthis, const AjPStr str);
__deprecated AjBool      ajStrSetC  (AjPStr* pthis, const char* str);
__deprecated ajint       ajStrSize (const AjPStr thys);
__deprecated const char  *ajStrStr (const AjPStr thys);
__deprecated char        *ajStrStrMod (AjPStr* thys);
__deprecated AjBool      ajStrSub (AjPStr* pthis, ajint begin, ajint len);
__deprecated AjBool      ajStrSubstitute   (AjPStr* pthis, const AjPStr replace,
			      const AjPStr putin);
__deprecated AjBool      ajStrSubstituteCC (AjPStr* pthis, const char* replace,
			      const char* putin);
__deprecated AjBool      ajStrSubstituteKK (AjPStr* pthis, char replace,
			      char putin);
__deprecated AjBool      ajStrSuffix(const AjPStr str, const AjPStr str2);
__deprecated AjBool      ajStrSuffixCC (const char *str, const char* suff);
__deprecated AjBool      ajStrSuffixCO (const char *str, const AjPStr suff);
__deprecated AjBool      ajStrToLower (AjPStr* pthis);
__deprecated AjBool      ajStrToLowerII(AjPStr* pthis, ajint begin, ajint end);
__deprecated AjBool      ajStrToTitle (AjPStr* pthis);
__deprecated AjBool      ajStrToUpper (AjPStr* pthis);
__deprecated AjBool      ajStrToUpperII(AjPStr* pthis, ajint begin, ajint end);
__deprecated AjBool      ajStrTrim  (AjPStr* pthis, ajint num);
__deprecated AjBool      ajStrTruncate(AjPStr* Pstr, ajint pos);
__deprecated AjBool      ajStrUncomment (AjPStr* text);
__deprecated AjBool      ajStrUncommentStart (AjPStr* text);
__deprecated AjBool      ajStrWildPrefix (AjPStr* str);
__deprecated AjBool      ajStrWrap (AjPStr* pthis, ajint width);
__deprecated AjBool      ajStrWrapLeft (AjPStr* pthis, ajint width, ajint left);




__deprecated AjIStr      ajStrIter (const AjPStr thys);
__deprecated AjIStr      ajStrIterBack (const AjPStr thys);
__deprecated AjBool      ajStrIterBackDone(AjIStr iter);
__deprecated AjIStr      ajStrIterBackNext (AjIStr iter);
__deprecated void        ajStrIterFree (AjIStr *iter);
__deprecated AjBool      ajStrIterMore(AjIStr iter);
__deprecated AjBool      ajStrIterMoreBack(AjIStr iter);


__deprecated const AjPStr ajStrTok (const AjPStr thys);
__deprecated const AjPStr ajStrTokC (const AjPStr thys, const char* delim);
__deprecated const AjPStr ajStrTokCC (const char* thys, const char* delim);
__deprecated AjBool      ajStrToken (AjPStr* pthis, AjPStrTok *ptoken,
				    const char *delim);
__deprecated AjBool      ajStrTokenAss (AjPStrTok *ptok, const AjPStr thys,
			  const char *delim);
__deprecated void        ajStrTokenClear (AjPStrTok *token);
__deprecated ajint       ajStrTokenCount(const AjPStr line, const char *delim);
__deprecated ajint       ajStrTokenCountR(const AjPStr line, const char *delim);
__deprecated AjPStrTok   ajStrTokenInit (const AjPStr thys, const char *delim);
__deprecated AjBool      ajStrTokenRest (AjPStr* pthis, AjPStrTok* ptoken);

/*#define    MAJSTRLEN(str) str->Len*/
__deprecated const char  *MAJSTRSTR(const AjPStr thys);
__deprecated ajint  MAJSTRLEN(const AjPStr thys);
__deprecated ajint  MAJSTRSIZE(const AjPStr thys);
__deprecated ajint  MAJSTRREF(const AjPStr thys);

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

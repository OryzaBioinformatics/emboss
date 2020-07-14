#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseq_h
#define ajseq_h

#include "ajseqdata.h"
#include "ajax.h"

/* @data AjPSeqCvt ************************************************************
**
** Sequence conversion data. Used to convert a sequence to binary.
**
** @new ajSeqCvtNew Creates from a character string of valid bases.
** @new ajSeqCvtNewText Creates from a character string of valid bases.
** @new ajSeqCvtNewZero Creates from a character string of valid bases.
** @new ajSeqCvtNewZeroS Creates from an array of strings of valid bases.
** @output ajSeqCvtTrace Reports on contents for debugging
*
** @alias AjSSeqCvt
** @alias AjOSeqCvt
**
** @attr size [ajint] Number of characters in table, usually
**                    all possible characters.
** @attr len [ajint] Number of characters defined
** @attr missing [ajint] Index of the missing character value
** @attr bases [AjPStr] The bases which can be converted
** @attr labels [AjPStr*] Unused
** @attr table [char*] Binary character value for each character in bases
** @@
******************************************************************************/

typedef struct AjSSeqCvt {
  ajint size;
  ajint len;
  ajint missing;
  AjPStr bases;
  AjPStr* labels;
  char *table;
} AjOSeqCvt;

#define AjPSeqCvt AjOSeqCvt*

/* ======= prototypes ==================*/

AjBool       ajIsAccession (const AjPStr accnum);
AjPStr       ajIsSeqversion (const AjPStr sv);
AjPSelex     ajSelexNew(ajint n);
void         ajSelexDel(AjPSelex *thys);
AjPSelexdata ajSelexdataNew(void);
void         ajSelexdataDel(AjPSelexdata *thys);
void         ajSelexSQDel(AjPSelexSQ *thys);
AjPSelexSQ   ajSelexSQNew(void);
AjPStockholm ajStockholmNew(ajint i);
void         ajStockholmDel(AjPStockholm *thys);
void         ajStockholmdataDel(AjPStockholmdata *thys);


AjPStockholmdata ajStockholmdataNew(void);

ajint        ajSeqallBegin (const AjPSeqall seq);
void         ajSeqallDel(AjPSeqall *thys);
ajint        ajSeqallEnd (const AjPSeqall seq);
const AjPStr ajSeqallGetName (const AjPSeqall thys);
const AjPStr ajSeqallGetNameSeq (const AjPSeqall thys);
ajint        ajSeqallGetRange (const AjPSeqall thys, ajint* begin, ajint* end);
const AjPStr ajSeqallGetUsa (const AjPSeqall thys);
ajint        ajSeqallLen (const AjPSeqall seqall);
AjPSeqall    ajSeqallNew (void);
void         ajSeqallClear (AjPSeqall thys);
AjBool       ajSeqallNext (AjPSeqall seqall, AjPSeq* retseq);
void         ajSeqallReverse (AjPSeqall thys);
void         ajSeqallSetRange (AjPSeqall seq, ajint ibegin, ajint iend);
void         ajSeqallToLower (AjPSeqall seqall);
void         ajSeqallToUpper (AjPSeqall seqall);
void         ajSeqAssAcc (AjPSeq thys, const AjPStr str);
void         ajSeqAssAccC (AjPSeq thys, const char* text);
void         ajSeqAssDesc (AjPSeq thys, const AjPStr str);
void         ajSeqAssDescC (AjPSeq thys, const char* text);
void         ajSeqAssEntry (AjPSeq thys, const AjPStr str);
void         ajSeqAssEntryC (AjPSeq thys, const char* text);
void         ajSeqAssFile (AjPSeq thys, const AjPStr str);
void         ajSeqAssFull (AjPSeq thys, const AjPStr str);
void         ajSeqAssFullC (AjPSeq thys, const char* text);
void         ajSeqAssGi (AjPSeq thys, const AjPStr str);
void         ajSeqAssGiC (AjPSeq thys, const char* text);
void         ajSeqAssName (AjPSeq thys, const AjPStr str);
void         ajSeqAssNameC (AjPSeq thys, const char* text);
void         ajSeqAssSeq (AjPSeq thys, const AjPStr str);
void         ajSeqAssSeqC (AjPSeq thys, const char* text);
void         ajSeqAssSeqCI (AjPSeq thys, const char* text, ajint ilen);
void         ajSeqAssSv (AjPSeq thys, const AjPStr str);
void         ajSeqAssSvC (AjPSeq thys, const char* text);
void         ajSeqAssUfo (AjPSeq thys, const AjPStr str);
void         ajSeqAssUfoC (AjPSeq thys, const char* text);
void         ajSeqAssUsa (AjPSeq thys, const AjPStr str);
void         ajSeqAssUsaC (AjPSeq thys, const char* text);
char         ajSeqBaseComp (char base);
ajint        ajSeqBegin (const AjPSeq seq);
const char*  ajSeqChar (const AjPSeq thys);
char*        ajSeqCharCopy (const AjPSeq seq);
char*        ajSeqCharCopyL (const AjPSeq seq, size_t size);
ajint        ajSeqCheckGcg (const AjPSeq thys);
void         ajSeqClear (AjPSeq thys);
void         ajSeqCompOnly (AjPSeq thys);
void         ajSeqCompOnlyStr (AjPStr* thys);
AjPFeattable ajSeqCopyFeat (const AjPSeq thys);
void         ajSeqCount (const AjPStr thys, ajint *b);
ajuint       ajSeqCrc(const AjPStr seq );
void         ajSeqCvtDel (AjPSeqCvt* thys);
ajint        ajSeqCvtK (const AjPSeqCvt thys, char ch);
ajint        ajSeqCvtKS (const AjPSeqCvt thys, const AjPStr ch);
ajint        ajSeqCvtLen (const AjPSeqCvt thys);
AjPSeqCvt    ajSeqCvtNew (const char* bases);
AjPSeqCvt    ajSeqCvtNewText (const char* bases);
AjPSeqCvt    ajSeqCvtNewZero (const char* bases);
AjPSeqCvt    ajSeqCvtNewZeroS (const AjPPStr bases, int n);
ajint        ajSeqCvtSize (const AjPSeqCvt cvt);
void         ajSeqCvtTrace (const AjPSeqCvt cvt);
void         ajSeqDel (AjPSeq* pthis);
ajint        ajSeqEnd (const AjPSeq seq);
ajint        ajSeqFill (AjPSeq seq, ajint len);
ajint        ajSeqGapCount (const AjPSeq thys);
ajint        ajSeqGapCountS (const AjPStr str);
void         ajSeqGapStandard (AjPSeq thys, char gapch);
const AjPStr ajSeqGetAcc (const AjPSeq thys);
const AjPStr ajSeqGetDesc (const AjPSeq thys);
const AjPStr ajSeqGetEntry (const AjPSeq thys);
const AjPFeattable ajSeqGetFeat (const AjPSeq thys);
const AjPStr ajSeqGetGi (const AjPSeq thys);
const AjPStr ajSeqGetName (const AjPSeq thys);
ajint        ajSeqGetRange (const AjPSeq thys, ajint* begin, ajint* end);
AjBool       ajSeqGetReverse (const AjPSeq thys);
AjBool       ajSeqGetReversed (const AjPSeq thys);
const AjPStr ajSeqGetSv (const AjPSeq thys);
const AjPStr ajSeqGetTax (const AjPSeq thys);
const AjPStr ajSeqGetUsa (const AjPSeq thys);
void         ajSeqinTrace (const AjPSeqin thys);
AjBool       ajSeqIsNuc (const AjPSeq thys);
AjBool       ajSeqIsProt (const AjPSeq thys);
ajint        ajSeqLen (const AjPSeq seq);
void         ajSeqMakeUsa (AjPSeq thys, const AjPSeqin seqin);
void         ajSeqMakeUsaS(const AjPSeq thys, const AjPSeqin seqin,
			   AjPStr* usa);
void         ajSeqMod (AjPSeq thys);
float        ajSeqMW (const AjPStr seq);
const char*  ajSeqName (const AjPSeq seq);
AjPSeq       ajSeqNew (void);
AjPSeq       ajSeqNewC (const char* seq, const char* name);
AjPSeq       ajSeqNewL (size_t size);
AjPSeq       ajSeqNewS (const AjPSeq seq);
AjPSeq       ajSeqNewStr (const AjPStr seq);
AjBool       ajSeqNum (const AjPSeq thys, const AjPSeqCvt cvt,
		       AjPStr *numseq);
AjBool       ajSeqNumS (const AjPStr thys, const AjPSeqCvt cvt,
			AjPStr *numseq);
ajint        ajSeqOffset (const AjPSeq seq);
ajint        ajSeqOffend (const AjPSeq seq);
ajint        ajSeqPos (const AjPSeq thys, ajint ipos);
ajint        ajSeqPosI (const AjPSeq thys, ajint imin, ajint ipos);
ajint        ajSeqPosII (ajint ilen, ajint imin, ajint ipos);
void         ajSeqReplace (AjPSeq thys, const AjPStr seq);
void         ajSeqReplaceC (AjPSeq thys, const char* seq);
void         ajSeqReverse (AjPSeq thys);
void         ajSeqReverseForce (AjPSeq thys);
void         ajSeqReverseStr (AjPStr* thys);
void         ajSeqRevOnly (AjPSeq thys);
ajint        ajSeqsetBegin (const AjPSeqset seq);
void         ajSeqsetDel(AjPSeqset *thys);
ajint        ajSeqsetEnd (const AjPSeqset seq);
ajint        ajSeqsetFill (AjPSeqset seq);
const AjPStr ajSeqsetGetFormat (const AjPSeqset thys);
const AjPStr ajSeqsetGetName (const AjPSeqset thys);
ajint        ajSeqsetGetRange (const AjPSeqset thys, ajint* begin, ajint* end);
const AjPSeq ajSeqsetGetSeq (const AjPSeqset thys, ajint i);
AjPSeq*      ajSeqsetGetSeqArray(const AjPSeqset thys);
const AjPStr ajSeqsetGetUsa (const AjPSeqset thys);
AjBool       ajSeqsetIsDna (const AjPSeqset thys);
AjBool       ajSeqsetIsNuc (const AjPSeqset thys);
AjBool       ajSeqsetIsProt (const AjPSeqset thys);
AjBool       ajSeqsetIsRna (const AjPSeqset thys);
ajint        ajSeqsetLen (const AjPSeqset seq);
const AjPStr ajSeqsetName (const AjPSeqset seq, ajint i);
AjPSeqset    ajSeqsetNew (void);
void         ajSeqSetRange (AjPSeq seq, ajint ibegin, ajint iend);
void         ajSeqsetReverse (AjPSeqset thys);
const char*  ajSeqsetSeq (const AjPSeqset seq, ajint i);
void         ajSeqsetSetRange (AjPSeqset seq, ajint ibegin, ajint iend);
ajint        ajSeqsetSize (const AjPSeqset seq);
void         ajSeqsetToLower (AjPSeqset seq);
float        ajSeqsetTotweight (const AjPSeqset seq);
void         ajSeqsetToUpper (AjPSeqset seq);
void         ajSeqsetTrim(AjPSeqset thys);
float        ajSeqsetWeight (const AjPSeqset seq, ajint i) ;
const AjPStr ajSeqStr (const AjPSeq thys);
AjPStr       ajSeqStrCopy (const AjPSeq thys);
void         ajSeqToLower (AjPSeq thys);
void         ajSeqToUpper (AjPSeq thys);
void         ajSeqTrace (const AjPSeq seq);
AjBool       ajSeqFindOutFormat (const AjPStr format, ajint* iformat);
AjBool       ajSeqTrim(AjPSeq thys);
#endif

#ifdef __cplusplus
}
#endif

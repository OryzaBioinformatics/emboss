#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseq_h
#define ajseq_h

#include "ajseqdata.h"
#include "ajax.h"

/* @data AjPSeqCvt *******************************************************
**
** Sequence conversion data. Used to convert a sequence to binary.
**
** @new ajSeqCvtNew Creates from a character string of valid bases.
** @new ajSeqCvtNewText Creates from a character string of valid bases.
** @new ajSeqCvtNewZero Creates from a character string of valid bases.
** @use ajSeqNum Convert sequence to numbers
** @use ajSeqCvtTrace Reports on contents for debugging
** @@
******************************************************************************/

typedef struct AjSSeqCvt {
  ajint size;
  ajint len;
  ajint missing;
  AjPStr bases;
  char *table;
} AjOSeqCvt, *AjPSeqCvt;

/* ======= prototypes ==================*/

AjBool       ajIsAccession (AjPStr accnum);
AjPStr       ajIsSeqversion (AjPStr sv);
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

ajint        ajSeqallBegin (AjPSeqall seq);
void         ajSeqallDel(AjPSeqall *thys);
ajint        ajSeqallEnd (AjPSeqall seq);
AjPStr       ajSeqallGetName (AjPSeqall thys);
AjPStr       ajSeqallGetNameSeq (AjPSeqall thys);
ajint        ajSeqallGetRange (AjPSeqall thys, ajint* begin, ajint* end);
AjPStr       ajSeqallGetUsa (AjPSeqall thys);
ajint        ajSeqallLen (AjPSeqall seqall);
AjPSeqall    ajSeqallNew (void);
AjBool       ajSeqallNext (AjPSeqall seqall, AjPSeq* retseq);
void         ajSeqallReverse (AjPSeqall thys);
void         ajSeqallSetRange (AjPSeqall seq, ajint ibegin, ajint iend);
void         ajSeqallToLower (AjPSeqall seqall);
void         ajSeqallToUpper (AjPSeqall seqall);
void         ajSeqAssAcc (AjPSeq thys, AjPStr str);
void         ajSeqAssAccC (AjPSeq thys, char* text);
void         ajSeqAssDesc (AjPSeq thys, AjPStr str);
void         ajSeqAssDescC (AjPSeq thys, char* text);
void         ajSeqAssEntry (AjPSeq thys, AjPStr str);
void         ajSeqAssEntryC (AjPSeq thys, char* text);
void         ajSeqAssFile (AjPSeq thys, AjPStr str);
void         ajSeqAssFull (AjPSeq thys, AjPStr str);
void         ajSeqAssFullC (AjPSeq thys, char* text);
void         ajSeqAssGi (AjPSeq thys, AjPStr str);
void         ajSeqAssGiC (AjPSeq thys, char* text);
void         ajSeqAssName (AjPSeq thys, AjPStr str);
void         ajSeqAssNameC (AjPSeq thys, char* text);
void         ajSeqAssSeq (AjPSeq thys, AjPStr str);
void         ajSeqAssSeqC (AjPSeq thys, char* text);
void         ajSeqAssSeqCI (AjPSeq thys, char* text, ajint ilen);
void         ajSeqAssSv (AjPSeq thys, AjPStr str);
void         ajSeqAssSvC (AjPSeq thys, char* text);
void         ajSeqAssUfo (AjPSeq thys, AjPStr str);
void         ajSeqAssUfoC (AjPSeq thys, char* text);
void         ajSeqAssUsa (AjPSeq thys, AjPStr str);
void         ajSeqAssUsaC (AjPSeq thys, char* text);
char         ajSeqBaseComp (char base);
ajint        ajSeqBegin (AjPSeq seq);
char*        ajSeqChar (AjPSeq thys);
char*        ajSeqCharCopy (AjPSeq seq);
char*        ajSeqCharCopyL (AjPSeq seq, size_t size);
ajint        ajSeqCheckGcg (AjPSeq thys);
void         ajSeqClear (AjPSeq thys);
void         ajSeqCompOnly (AjPSeq thys);
void         ajSeqCompOnlyStr (AjPStr* thys);
AjPFeattable ajSeqCopyFeat (AjPSeq thys);
void         ajSeqCount (AjPStr thys, ajint *b);
ajuint       ajSeqCrc( AjPStr seq );
void         ajSeqCvtDel (AjPSeqCvt* thys);
ajint        ajSeqCvtK (AjPSeqCvt thys, char ch);
ajint        ajSeqCvtLen (AjPSeqCvt thys);
AjPSeqCvt    ajSeqCvtNew (char* bases);
AjPSeqCvt    ajSeqCvtNewText (char* bases);
AjPSeqCvt    ajSeqCvtNewZero (char* bases);
ajint        ajSeqCvtSize (AjPSeqCvt cvt);
void         ajSeqCvtTrace (AjPSeqCvt cvt);
void         ajSeqDel (AjPSeq* pthis);
ajint        ajSeqEnd (AjPSeq seq);
ajint        ajSeqFill (AjPSeq seq, ajint len);
ajint        ajSeqGapCount (AjPSeq thys);
ajint        ajSeqGapCountS (AjPStr str);
void         ajSeqGapStandard (AjPSeq thys, char gapch);
AjPStr       ajSeqGetAcc (AjPSeq thys);
AjPStr       ajSeqGetDesc (AjPSeq thys);
AjPStr       ajSeqGetEntry (AjPSeq thys);
AjPFeattable ajSeqGetFeat (AjPSeq thys);
AjPStr       ajSeqGetGi (AjPSeq thys);
AjPStr       ajSeqGetName (AjPSeq thys);
ajint        ajSeqGetRange (AjPSeq thys, ajint* begin, ajint* end);
AjBool       ajSeqGetReverse (AjPSeq thys);
AjPStr       ajSeqGetSv (AjPSeq thys);
AjPStr       ajSeqGetUsa (AjPSeq thys);
void         ajSeqinTrace (AjPSeqin thys);
AjBool       ajSeqIsNuc (AjPSeq thys);
AjBool       ajSeqIsProt (AjPSeq thys);
ajint        ajSeqLen (AjPSeq seq);
void         ajSeqMakeUsa (AjPSeq thys, AjPSeqin seqin);
void         ajSeqMod (AjPSeq thys);
float        ajSeqMW (AjPStr seq);
char*        ajSeqName (AjPSeq seq);
AjPSeq       ajSeqNew (void);
AjPSeq       ajSeqNewL (size_t size);
AjPSeq       ajSeqNewS (AjPSeq seq);
AjBool       ajSeqNum (AjPSeq thys, AjPSeqCvt cvt, AjPStr *numseq);
ajint        ajSeqOffset (AjPSeq seq);
ajint        ajSeqOffend (AjPSeq seq);
ajint        ajSeqPos (AjPSeq thys, ajint ipos);
ajint        ajSeqPosI (AjPSeq thys, ajint imin, ajint ipos);
ajint        ajSeqPosII (ajint ilen, ajint imin, ajint ipos);
void         ajSeqQueryClear (AjPSeqQuery thys);
void         ajSeqQueryDel (AjPSeqQuery *pthis);
AjBool       ajSeqQueryIs (AjPSeqQuery qry);
AjPSeqQuery  ajSeqQueryNew (void);
void         ajSeqQueryStarclear (AjPSeqQuery qry);
void         ajSeqQueryTrace (AjPSeqQuery qry);
AjBool       ajSeqQueryWild (AjPSeqQuery qry);
void         ajSeqReplace (AjPSeq thys, AjPStr seq);
void         ajSeqReplaceC (AjPSeq thys, char* seq);
void         ajSeqReverse (AjPSeq thys);
void         ajSeqReverseStr (AjPStr* thys);
void         ajSeqRevOnly (AjPSeq thys);
ajint        ajSeqsetBegin (AjPSeqset seq);
void         ajSeqsetDel(AjPSeqset *thys);
ajint        ajSeqsetEnd (AjPSeqset seq);
ajint        ajSeqsetFill (AjPSeqset seq);
AjPStr       ajSeqsetGetFormat (AjPSeqset thys);
AjPStr       ajSeqsetGetName (AjPSeqset thys);
ajint        ajSeqsetGetRange (AjPSeqset thys, ajint* begin, ajint* end);
AjPSeq       ajSeqsetGetSeq (AjPSeqset thys, ajint i);
AjPStr       ajSeqsetGetUsa (AjPSeqset thys);
AjBool       ajSeqsetIsDna (AjPSeqset thys);
AjBool       ajSeqsetIsNuc (AjPSeqset thys);
AjBool       ajSeqsetIsProt (AjPSeqset thys);
AjBool       ajSeqsetIsRna (AjPSeqset thys);
ajint        ajSeqsetLen (AjPSeqset seq);
AjPStr       ajSeqsetName (AjPSeqset seq, ajint i);
AjPSeqset    ajSeqsetNew (void);
void         ajSeqSetRange (AjPSeq seq, ajint ibegin, ajint iend);
void         ajSeqsetReverse (AjPSeqset thys);
char*        ajSeqsetSeq (AjPSeqset seq, ajint i);
void         ajSeqsetSetRange (AjPSeqset seq, ajint ibegin, ajint iend);
ajint        ajSeqsetSize (AjPSeqset seq);
void         ajSeqsetToLower (AjPSeqset seq);
float        ajSeqsetTotweight (AjPSeqset seq);
void         ajSeqsetToUpper (AjPSeqset seq);
float        ajSeqsetWeight (AjPSeqset seq, ajint i) ;
AjPStr       ajSeqStr (AjPSeq thys);
AjPStr       ajSeqStrCopy (AjPSeq thys);
void         ajSeqToLower (AjPSeq thys);
void         ajSeqToUpper (AjPSeq thys);
void         ajSeqTrace (AjPSeq seq);
AjBool       ajSeqFindOutFormat (AjPStr format, ajint* iformat);
AjBool       ajSeqTrim(AjPSeq thys);
#endif

#ifdef __cplusplus
}
#endif

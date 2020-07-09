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
  int size;
  int len;
  int missing;
  AjPStr bases;
  char *table;
} AjOSeqCvt, *AjPSeqCvt;

/* ======= prototypes ==================*/

AjBool       ajIsAccession (AjPStr accnum);
int          ajSeqallBegin (AjPSeqall seq);
void         ajSeqallDel(AjPSeqall *thys);
int          ajSeqallEnd (AjPSeqall seq);
AjPStr       ajSeqallGetName (AjPSeqall thys);
AjPStr       ajSeqallGetNameSeq (AjPSeqall thys);
int          ajSeqallGetRange (AjPSeqall thys, int* begin, int* end);
AjPStr       ajSeqallGetUsa (AjPSeqall thys);
int          ajSeqallLen (AjPSeqall seqall);
AjPSeqall    ajSeqallNew (void);
AjBool       ajSeqallNext (AjPSeqall seqall, AjPSeq* retseq);
void         ajSeqallReverse (AjPSeqall thys);
void         ajSeqallSetRange (AjPSeqall seq, int ibegin, int iend);
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
void         ajSeqAssName (AjPSeq thys, AjPStr str);
void         ajSeqAssNameC (AjPSeq thys, char* text);
void         ajSeqAssSeq (AjPSeq thys, AjPStr str);
void         ajSeqAssSeqC (AjPSeq thys, char* text);
void         ajSeqAssUfo (AjPSeq thys, AjPStr str);
void         ajSeqAssUfoC (AjPSeq thys, char* text);
void         ajSeqAssUsa (AjPSeq thys, AjPStr str);
void         ajSeqAssUsaC (AjPSeq thys, char* text);
char         ajSeqBaseComp (char base);
int          ajSeqBegin (AjPSeq seq);
char*        ajSeqChar (AjPSeq thys);
char*        ajSeqCharCopy (AjPSeq seq);
char*        ajSeqCharCopyL (AjPSeq seq, size_t size);
int          ajSeqCheckGcg (AjPSeq thys);
void         ajSeqClear (AjPSeq thys);
void         ajSeqCompOnly (AjPSeq thys);
void         ajSeqCompOnlyStr (AjPStr* thys);
void         ajSeqCount (AjPStr thys, int *b);
unsigned int ajSeqCrc( AjPStr seq );
void         ajSeqCvtDel (AjPSeqCvt* thys);
int          ajSeqCvtK (AjPSeqCvt thys, char ch);
int          ajSeqCvtLen (AjPSeqCvt thys);
AjPSeqCvt    ajSeqCvtNew (char* bases);
AjPSeqCvt    ajSeqCvtNewText (char* bases);
AjPSeqCvt    ajSeqCvtNewZero (char* bases);
int          ajSeqCvtSize (AjPSeqCvt cvt);
void         ajSeqCvtTrace (AjPSeqCvt cvt);
void         ajSeqDel (AjPSeq* pthis);
int          ajSeqEnd (AjPSeq seq);
AjPStr       ajSeqGetAcc (AjPSeq thys);
AjPStr       ajSeqGetDesc (AjPSeq thys);
AjPStr       ajSeqGetEntry (AjPSeq thys);
AjPFeatTable ajSeqGetFeat (AjPSeq thys);
AjPStr       ajSeqGetName (AjPSeq thys);
int          ajSeqGetRange (AjPSeq thys, int* begin, int* end);
AjPStr       ajSeqGetUsa (AjPSeq thys);
void         ajSeqinTrace (AjPSeqin thys);
AjBool       ajSeqIsNuc (AjPSeq thys);
AjBool       ajSeqIsProt (AjPSeq thys);
int          ajSeqLen (AjPSeq seq);
void         ajSeqMakeUsa (AjPSeq thys, AjPSeqin seqin);
void         ajSeqMod (AjPSeq thys);
float        ajSeqMW (AjPStr seq);
char*        ajSeqName (AjPSeq seq);
AjPSeq       ajSeqNew (void);
AjPSeq       ajSeqNewL (size_t size);
AjPSeq       ajSeqNewS (AjPSeq seq);
AjBool       ajSeqNum (AjPSeq thys, AjPSeqCvt cvt, AjPStr *numseq);
int          ajSeqOffset (AjPSeq seq);
int          ajSeqOffend (AjPSeq seq);
int          ajSeqPos (AjPSeq thys, int ipos);
int          ajSeqPosI (AjPSeq thys, int imin, int ipos);
int          ajSeqPosII (int ilen, int imin, int ipos);
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
int          ajSeqsetBegin (AjPSeqset seq);
int          ajSeqsetEnd (AjPSeqset seq);
int          ajSeqsetFill (AjPSeqset seq);
AjPStr       ajSeqsetGetName (AjPSeqset thys);
int          ajSeqsetGetRange (AjPSeqset thys, int* begin, int* end);
AjPSeq       ajSeqsetGetSeq (AjPSeqset thys, int i);
AjBool       ajSeqsetIsNuc (AjPSeqset thys);
AjBool       ajSeqsetIsProt (AjPSeqset thys);
int          ajSeqsetLen (AjPSeqset seq);
AjPStr       ajSeqsetName (AjPSeqset seq, int i);
AjPSeqset    ajSeqsetNew (void);
void         ajSeqSetRange (AjPSeq seq, int ibegin, int iend);
void         ajSeqsetReverse (AjPSeqset thys);
char*        ajSeqsetSeq (AjPSeqset seq, int i);
void         ajSeqsetSetRange (AjPSeqset seq, int ibegin, int iend);
int          ajSeqsetSize (AjPSeqset seq);
void         ajSeqsetToLower (AjPSeqset seq);
float        ajSeqsetTotweight (AjPSeqset seq);
void         ajSeqsetToUpper (AjPSeqset seq);
float        ajSeqsetWeight (AjPSeqset seq, int i) ;
AjPStr       ajSeqStr (AjPSeq thys);
AjPStr       ajSeqStrCopy (AjPSeq thys);
void         ajSeqToLower (AjPSeq thys);
void         ajSeqToUpper (AjPSeq thys);
void         ajSeqTrace (AjPSeq seq);
AjBool       ajSeqFindOutFormat (AjPStr format, int* iformat);
AjBool       ajSeqTrim(AjPSeq thys);
#endif

#ifdef __cplusplus
}
#endif

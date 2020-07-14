#ifdef __cplusplus
extern "C"
{
#endif

/*
**
** ajfeat.h - AJAX Sequence Feature include file
**            Version 2.0 - June 2001
**
*/

#ifndef ajfeat_h
#define ajfeat_h

#include <stdlib.h>
#include <stdio.h>

#include "ajdefine.h"
#include "ajexcept.h"
#include "ajmem.h"
#include "ajreg.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajtime.h"
#include "ajfmt.h"
#include "ajfeatdata.h"
#include "ajseqdata.h"

/* ========================================================================= */
/* ========== All functions in (more or less) alphabetical order =========== */
/* ========================================================================= */

AjPFeature    ajFeatAdd (AjPFeattable thys, const AjPStr type,
			 ajint start, ajint end, float score,
			 char strand, ajint frame, const AjPStr desc);
AjPFeature    ajFeatAddC (AjPFeattable thys, const char* type,
			  ajint start, ajint end, float score,
			  char strand, ajint frame, const AjPStr desc);
AjBool        ajFeatIsChild (const AjPFeature gf);
void *        ajFeatClearTag(AjPFeature thys, const AjPFeattable table,
			     const AjPStr tag) ;
AjPFeature    ajFeatCopy (const AjPFeature orig);
void          ajFeatDel(AjPFeature *pthis) ;
void          ajFeatExit (void);
ajint         ajFeatGetEnd (const AjPFeature thys);
AjBool        ajFeatGetForward (const AjPFeature thys);
ajint         ajFeatGetFrame (const AjPFeature thys);
ajint         ajFeatGetLocs(const AjPStr str, AjPStr **cds, const char *type);
AjBool        ajFeatGetNote (const AjPFeature thys, const AjPStr name,
			     AjPStr* val);
AjBool        ajFeatGetNoteI (const AjPFeature thys, const AjPStr name,
			      ajint count, AjPStr* val);
ajint         ajFeatGetStart (const AjPFeature thys);
AjBool        ajFeatGetTag (const AjPFeature thys, const AjPStr name,
			    ajint num, AjPStr* val);
ajint         ajFeatGetTrans(const AjPStr str, AjPStr **cds);
AjPStr        ajFeatGetType (const AjPFeature thys);
AjBool        ajFeatIsCompMult (const AjPFeature gf);
AjBool        ajFeatIsLocal (const AjPFeature gf);
AjBool        ajFeatIsLocalRange (const AjPFeature gf, ajint start, ajint end);
AjBool        ajFeatIsMultiple (const AjPFeature gf);
AjBool        ajFeatLocToSeq(const AjPStr seq, const AjPStr line, AjPStr *res,
			     const AjPStr usa);
AjPFeature    ajFeatNew (AjPFeattable thys,
			 const AjPStr source, const AjPStr type,
			 ajint Start, ajint End, float score,
			 char strand, ajint frame);
AjPFeature    ajFeatNewII (AjPFeattable thys,
			   ajint Start, ajint End);
AjPFeature    ajFeatNewIIRev (AjPFeattable thys,
			      ajint Start, ajint End);
AjPFeature    ajFeatNewProt (AjPFeattable thys,
			     const AjPStr source, const AjPStr type,
			     ajint Start, ajint End, float score);
AjBool        ajFeatOutFormatDefault (AjPStr* pformat);
AjPFeattable  ajFeatRead  (AjPFeattabIn ftin) ;
void          ajFeatReverse  (AjPFeature thys, ajint ilen) ;
AjBool        ajFeatSetDesc (AjPFeature thys, const AjPStr desc);
AjBool        ajFeatSetDescApp (AjPFeature thys, const AjPStr desc);
void          ajFeatSortByEnd (AjPFeattable Feattab);
void          ajFeatSortByStart (AjPFeattable Feattab);
void          ajFeatSortByType (AjPFeattable Feattab);
void          ajFeattableAdd (AjPFeattable thys, AjPFeature feature) ;
ajint         ajFeattableBegin (const AjPFeattable thys);
void          ajFeattableClear (AjPFeattable thys);
AjPFeattable  ajFeattableCopy (const AjPFeattable orig);
void          ajFeattableDel (AjPFeattable *pthis) ;
ajint         ajFeattableEnd (const AjPFeattable thys);
AjPStr        ajFeattableGetName (const AjPFeattable thys);
AjBool        ajFeattableIsNuc (const AjPFeattable thys);
AjBool        ajFeattableIsProt (const AjPFeattable thys);
ajint         ajFeattableLen (const AjPFeattable thys);
AjPFeattable  ajFeattableNew (const AjPStr name);
AjPFeattable  ajFeattableNewDna (const AjPStr name);
AjPFeattable  ajFeattableNewProt (const AjPStr name);
AjPFeattable  ajFeattableNewSeq (const AjPSeq seq);
ajint         ajFeattablePos (const AjPFeattable thys, ajint ipos);
ajint         ajFeattablePosI (const AjPFeattable thys,
			       ajint imin, ajint ipos);
ajint         ajFeattablePosII (ajint ilen, ajint imin, ajint ipos);
void          ajFeattableReverse  (AjPFeattable  thys) ;
void          ajFeattableSetDna (AjPFeattable thys);
void          ajFeattableSetProt (AjPFeattable thys);
void          ajFeattableSetRange  (AjPFeattable thys,
				     ajint fbegin, ajint fend) ;
ajint         ajFeattableSize (const AjPFeattable thys);
void          ajFeattableTrace (const AjPFeattable thys);
AjBool        ajFeattableTrimOff (AjPFeattable thys,
				  ajint ioffset, ajint ilen);
AjBool        ajFeattableWrite(AjPFeattable thys, const AjPStr ufo);
AjBool        ajFeattableWriteDdbj (const AjPFeattable features,
				    AjPFile file);
AjBool        ajFeattableWriteEmbl (const AjPFeattable features,
				    AjPFile file);
AjBool        ajFeattableWriteGenbank (const AjPFeattable features,
				       AjPFile file);
AjBool        ajFeattableWriteGff (const AjPFeattable features,
				   AjPFile file);
AjBool        ajFeattableWritePir (const AjPFeattable features,
				   AjPFile file);
AjBool        ajFeattableWriteSwiss (const AjPFeattable features,
				     AjPFile file);
void          ajFeattabInClear (AjPFeattabIn thys);
void          ajFeattabInDel (AjPFeattabIn* pthis);
AjPFeattabIn  ajFeattabInNew (void);
AjPFeattabIn  ajFeattabInNewSS (const AjPStr fmt, const AjPStr name,
				const char* type);
AjPFeattabIn  ajFeattabInNewSSF (const AjPStr fmt, const AjPStr name,
				 const char* type, AjPFileBuff buff);
AjBool        ajFeattabInSetType(AjPFeattabIn thys, const AjPStr type);
AjBool        ajFeattabInSetTypeC(AjPFeattabIn thys, const char* type);
void          ajFeattabOutDel (AjPFeattabOut* pthis);
AjPFile       ajFeattabOutFile (const AjPFeattabOut thys);
AjPStr        ajFeattabOutFilename (const AjPFeattabOut thys);
AjBool        ajFeattabOutIsLocal(const AjPFeattabOut thys);
AjBool        ajFeattabOutIsOpen (const AjPFeattabOut thys);
AjPFeattabOut ajFeattabOutNew (void);
AjPFeattabOut ajFeattabOutNewSSF (const AjPStr fmt, const AjPStr name,
				  const char* type, AjPFile buff);
AjBool        ajFeattabOutOpen (AjPFeattabOut thys, const AjPStr ufo);
AjBool        ajFeattabOutSet (AjPFeattabOut thys, const AjPStr ufo);
void          ajFeattabOutSetBasename (AjPFeattabOut thys,
				       const AjPStr basename);
AjBool        ajFeattabOutSetType(AjPFeattabOut thys, const AjPStr type);
AjBool        ajFeattabOutSetTypeC(AjPFeattabOut thys, const char* type);
AjPFeattable  ajFeattabRead (AjPFeattabIn ftin) ;
AjBool        ajFeatTagAdd (AjPFeature thys,
			    const AjPStr tag, const AjPStr value);
AjBool        ajFeatTagAddC (AjPFeature thys,
			     const char* tag, const AjPStr value);
AjBool        ajFeatTagAddCC (AjPFeature thys,
			      const char* tag, const char* value);
AjIList       ajFeatTagIter (const AjPFeature thys);
AjBool        ajFeatTagSet (AjPFeature thys,
			    const AjPStr tag, const AjPStr value);
AjBool        ajFeatTagSetC (AjPFeature thys,
			     const char* tag, const AjPStr value);
void          ajFeatTagTrace (const AjPFeature thys);
AjBool        ajFeatTagval (AjIList iter, AjPStr* tagnam,
			    AjPStr* tagval);
void          ajFeatTest (void);
void          ajFeatTrace (const AjPFeature thys);
void          ajFeatTraceOld (const AjPFeattable thys);
AjBool        ajFeatTrimOffRange (AjPFeature ft, ajint ioffset,
				  ajint begin, ajint end,
				  AjBool dobegin, AjBool doend);
AjPFeattable  ajFeatUfoRead (AjPFeattabIn tabin, const AjPStr Ufo);
AjBool        ajFeatUfoWrite (const AjPFeattable thys,
			      AjPFeattabOut tabout, const AjPStr Ufo);
AjBool        ajFeatWrite (AjPFeattabOut ftout, const AjPFeattable ft) ;



/*
//#define       MAJFEATOBJVERIFY(p,c) ajFeatObjAssert((p), (c), \
//      __FILE__, __LINE__)
//#define       MAJFEATSETSCORE(p,s) (((AjPFeature)(p))->Score=(s))
//#define       MAJFEATSCORE(p)    ((p)->Score)
//#define       MAJFEATSOURCE(p)   ((p)->Source)
//#define       MAJFEATTYPE(p)     ((p)->Type)
//#define       MAJFEATTABSETVERSION(p,v) ((p)->Version=(v))
//#define       MAJFEATTABSETDATE(p,d)    ((p)->Date=(d))
//#define       MAJFEATTABDEFFORMAT(p,f)  ((p)->DefFormat=(f))
//#define       MAJFEATTABFORMAT(p)       ((p)->Format)
//#define       MAJFEATTABVERSION(p)      ((p)->Version)
//#define       MAJFEATTABDATE(p)         ((p)->Date)
//#define       MAJFEATTABDICTIONARY(p)   ((p)->Dictionary)
//#define       MAJFEATVOCABREADONLY(p,f) ((p)?(p)->ReadOnly=(f):AjTrue)
*/

#endif /* ajfeat_h */

#ifdef __cplusplus
}
#endif

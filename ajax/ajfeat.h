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

AjPFeature    ajFeatAdd (AjPFeattable thys, AjPStr type,
			 ajint start, ajint end, float score,
			 char strand, ajint frame, AjPStr desc);
AjPFeature    ajFeatAddC (AjPFeattable thys, char* type,
			  ajint start, ajint end, float score,
			  char strand, ajint frame, AjPStr desc);
void *        ajFeatClearTag(AjPFeature thys, AjPFeattable table,
			     AjPStr tag) ;
void          ajFeatCopy (AjPFeature* pthys, AjPFeature orig);
void          ajFeatDel(AjPFeature *pthis) ;
void          ajFeatExit (void);
ajint         ajFeatGetEnd (AjPFeature thys);
AjBool        ajFeatGetForward (AjPFeature thys);
ajint         ajFeatGetFrame (AjPFeature thys);
ajint         ajFeatGetLocs(AjPStr str, AjPStr **cds, char *type);
AjBool        ajFeatGetNote (AjPFeature thys, AjPStr name, AjPStr* val);
AjBool        ajFeatGetNoteI (AjPFeature thys, AjPStr name, ajint count,
			      AjPStr* val);
ajint         ajFeatGetStart (AjPFeature thys);
AjBool        ajFeatGetTag (AjPFeature thys, AjPStr name, ajint num,
			    AjPStr* val);
AjPStr        ajFeatGetType (AjPFeature thys);
ajint         ajFeatGetTrans(AjPStr str, AjPStr **cds);
AjBool        ajFeatIsLocal (AjPFeature gf);
AjBool        ajFeatIsLocalRange (AjPFeature gf, ajint start, ajint end);
AjBool        ajFeatIsProt (AjPFeattable thys);
ajint         ajFeatLen (AjPFeattable thys);
AjBool        ajFeatLocToSeq(AjPStr seq, AjPStr line, AjPStr *res,
			     AjPStr usa);
AjPFeature    ajFeatNew (AjPFeattable thys,
			 AjPStr source, AjPStr type,
			 ajint Start, ajint End,  float score,
			 char strand, ajint frame);
AjPFeature    ajFeatNewII (AjPFeattable thys,
			 ajint Start, ajint End);
AjPFeature    ajFeatNewProt (AjPFeattable thys,
			     AjPStr source, AjPStr type,
			     ajint Start, ajint End,  float score);
AjPFeattable  ajFeatRead  ( AjPFeattabIn  ftin ) ;
AjBool        ajFeatSetDesc (AjPFeature thys, AjPStr desc);
AjBool        ajFeatSetDescApp (AjPFeature thys, AjPStr desc);
ajint         ajFeatSize (AjPFeattable thys);
void          ajFeatSortByEnd(AjPFeattable Feattab);
void          ajFeatSortByStart(AjPFeattable Feattab);
void          ajFeatSortByType(AjPFeattable Feattab);
void          ajFeattableAdd (AjPFeattable thys, AjPFeature feature) ;
void          ajFeattableClear ( AjPFeattable thys );
void          ajFeattableCopy (AjPFeattable* pthys, AjPFeattable orig);
void          ajFeattableDel (AjPFeattable *pthis) ;
AjPStr        ajFeattableGetName (AjPFeattable thys);
AjPFeattable  ajFeattableNew( AjPStr name );
AjPFeattable  ajFeattableNewDna( AjPStr name );
AjPFeattable  ajFeattableNewProt( AjPStr name );
AjPFeattable  ajFeattableNewSeq( AjPSeq seq );
void          ajFeattableSetDna (AjPFeattable thys);
void          ajFeattableSetProt (AjPFeattable thys);
void          ajFeattableTrace (AjPFeattable thys);
AjBool        ajFeattableWriteDdbj (AjPFeattable features,
				    AjPFile file);
AjBool        ajFeattableWriteEmbl (AjPFeattable features,
				    AjPFile file);
AjBool        ajFeattableWriteGenbank (AjPFeattable features,
				       AjPFile file);
AjBool        ajFeattableWriteGff (AjPFeattable features,
				   AjPFile file);
AjBool        ajFeattableWritePir (AjPFeattable features,
				     AjPFile file);
AjBool        ajFeattableWriteSwiss (AjPFeattable features,
				     AjPFile file);
void          ajFeattabInClear (AjPFeattabIn thys);
void          ajFeattabInDel( AjPFeattabIn* pthis);
AjPFeattabIn  ajFeattabInNew (void);
AjPFeattabIn  ajFeattabInNewSS (AjPStr fmt, AjPStr name, char* type);
AjPFeattabIn  ajFeattabInNewSSF (AjPStr fmt, AjPStr name, char* type,
				 AjPFileBuff buff);
void          ajFeattabOutDel( AjPFeattabOut* pthis);
AjPFile       ajFeattabOutFile (AjPFeattabOut thys);
AjPStr        ajFeattabOutFilename (AjPFeattabOut thys);
AjBool        ajFeattabOutIsOpen (AjPFeattabOut thys);
AjPFeattabOut ajFeattabOutNew (void);
AjPFeattabOut ajFeattabOutNewSSF (AjPStr fmt, AjPStr name, char* type,
				  AjPFile buff);
AjBool        ajFeattabOutOpen (AjPFeattabOut thys, AjPStr ufo);
AjBool        ajFeattabOutSet (AjPFeattabOut thys, AjPStr ufo);
AjPFeattable  ajFeattabRead  ( AjPFeattabIn  ftin ) ; 
void          ajFeatTagAdd (AjPFeature thys, AjPStr tag, AjPStr value);
void          ajFeatTagAddC (AjPFeature thys, const char* tag, AjPStr value);
void          ajFeatTagAddCC (AjPFeature thys, const char* tag,
			      const char* value);
AjIList       ajFeatTagIter (AjPFeature thys);
AjPStr        ajFeatTagSet (AjPFeature thys, AjPStr tag, AjPStr value);
AjPStr        ajFeatTagSetC (AjPFeature thys, char* tag, AjPStr value);
void          ajFeatTagTrace (AjPFeature thys);
AjBool        ajFeatTagval (AjIList iter, AjPStr* tagnam,
			    AjPStr* tagval);
void          ajFeatTest (void);
void          ajFeatTrace (AjPFeature thys);
void          ajFeatTraceOld (AjPFeattable thys);
AjBool        ajFeatUfoRead (AjPFeattable* pthis,
			     AjPFeattabIn tabin, AjPStr Ufo);
AjBool        ajFeatUfoWrite (AjPFeattable thys,
			      AjPFeattabOut tabout, AjPStr Ufo);
AjBool        ajFeatWrite ( AjPFeattabOut ftout, AjPFeattable ft) ; 




#define       MAJFEATOBJVERIFY(p,c) ajFeatObjAssert((p), (c), \
      __FILE__, __LINE__)
#define       MAJFEATSETSCORE(p,s) (((AjPFeature)(p))->Score=(s))
#define       MAJFEATSCORE(p)    ((p)->Score)
#define       MAJFEATSOURCE(p)   ((p)->Source)
#define       MAJFEATTYPE(p)     ((p)->Type)
#define       MAJFEATTABSETVERSION(p,v) ((p)->Version=(v))
#define       MAJFEATTABSETDATE(p,d)    ((p)->Date=(d))
#define       MAJFEATTABDEFFORMAT(p,f)  ((p)->DefFormat=(f))
#define       MAJFEATTABFORMAT(p)       ((p)->Format)
#define       MAJFEATTABVERSION(p)      ((p)->Version)
#define       MAJFEATTABDATE(p)         ((p)->Date)
#define       MAJFEATTABDICTIONARY(p)   ((p)->Dictionary)
#define       MAJFEATVOCABREADONLY(p,f) ((p)?(p)->ReadOnly=(f):AjTrue)


#endif /* ajfeat_h */

#ifdef __cplusplus
}
#endif

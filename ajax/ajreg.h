#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
**
** AJAX regular expression functions
**
******************************************************************************/

#ifndef ajreg_h
#define ajreg_h

#include "ajax.h"
#include "hsregexp.h"
#include "hsregmagic.h"

/* constructors */

AjPRegexp ajRegComp (AjPStr exp);
AjPRegexp ajRegCompC (const char* exp);

/* execute expression match */

AjBool ajRegExec (AjPRegexp prog, AjPStr str);
AjBool ajRegExecC (AjPRegexp prog, const char* str);

/* test substrings */

ajint ajRegLenI (AjPRegexp rp, ajint isub);
AjBool ajRegPost (AjPRegexp rp, AjPStr* post);
AjBool ajRegPostC (AjPRegexp rp, const char** post);

/* substitute substrings */

ajint ajRegOffset (AjPRegexp rp);
ajint ajRegOffsetI (AjPRegexp rp, ajint isub);
ajint ajRegOffsetC (AjPRegexp rp);
ajint ajRegOffsetIC (AjPRegexp rp, ajint isub);

void ajRegSub (AjPRegexp rp, AjPStr source, AjPStr* dest);
void ajRegSubI (AjPRegexp rp, ajint isub, AjPStr* dest);
void ajRegSubC (AjPRegexp rp, const char* source, AjPStr* dest);

/* destructor */

void ajRegFree (AjPRegexp* pexp);
void ajRegTrace (AjPRegexp exp);

#endif

#ifdef __cplusplus
}
#endif

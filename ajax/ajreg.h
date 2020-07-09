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

int ajRegLenI (AjPRegexp rp, int isub);
AjBool ajRegPost (AjPRegexp rp, AjPStr* post);
AjBool ajRegPostC (AjPRegexp rp, const char** post);

/* substitute substrings */

int ajRegOffset (AjPRegexp rp);
int ajRegOffsetI (AjPRegexp rp, int isub);
int ajRegOffsetC (AjPRegexp rp);
int ajRegOffsetIC (AjPRegexp rp, int isub);

void ajRegSub (AjPRegexp rp, AjPStr source, AjPStr* dest);
void ajRegSubI (AjPRegexp rp, int isub, AjPStr* dest);
void ajRegSubC (AjPRegexp rp, const char* source, AjPStr* dest);

/* destructor */

void ajRegFree (AjPRegexp* pexp);
void ajRegTrace (AjPRegexp exp);

#endif

#ifdef __cplusplus
}
#endif

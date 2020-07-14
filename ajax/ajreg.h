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
#include "pcre_internal.h"
#include "pcreposix.h"

#define AJREG_OVECSIZE 30

/* @data AjPRegexp ************************************************************
**
** PCRE expression internals, wrapped for AJAX calls
**
** @alias AjSRegexp
** @alias AjORegexp
**
** @attr pcre [real_pcre*] PCRE compiled expression
** @attr extra [pcre_extra*] PCRE study data (if available, else NULL)
** @attr ovecsize [int] Output vector size
** @attr ovector [int*] Output vector offsets
** @attr orig [const char*] Original string
******************************************************************************/

typedef struct AjSRegexp {
    real_pcre *pcre;
    pcre_extra *extra;
    int ovecsize;
    int *ovector;
    const char* orig;
} AjORegexp;

#define AjPRegexp AjORegexp*

/* constructors */

AjPRegexp ajRegComp (AjPStr exp);
AjPRegexp ajRegCompC (const char* exp);

AjPRegexp ajRegCompCase (AjPStr exp);
AjPRegexp ajRegCompCaseC (const char* exp);

/* execute expression match */

AjBool ajRegExec (AjPRegexp prog, const AjPStr str);
AjBool ajRegExecC (AjPRegexp prog, const char* str);

/* test substrings */

ajint  ajRegLenI (AjPRegexp rp, ajint isub);
ajint  ajRegOffset (AjPRegexp rp);
ajint  ajRegOffsetI (AjPRegexp rp, ajint isub);

/* get substrings */

AjBool ajRegPre (AjPRegexp rp, AjPStr* dest);
AjBool ajRegPost (AjPRegexp rp, AjPStr* post);
AjBool ajRegPostC (AjPRegexp rp, const char** post);
AjBool ajRegSubI (AjPRegexp rp, ajint isub, AjPStr* dest);

/* destructor */

void   ajRegFree (AjPRegexp* pexp);
void   ajRegTrace (AjPRegexp exp);

void   ajRegExit (void);
#endif

#ifdef __cplusplus
}
#endif

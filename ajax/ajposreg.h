#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
**
** AJAX regular expression functions
**
******************************************************************************/

#ifndef ajposreg_h
#define ajposreg_h

#include "ajax.h"
#include "hsp_regex.h"

/* constructors */

AjPPosRegexp ajPosRegComp (AjPStr exp);
AjPPosRegexp ajPosRegCompC (const char* exp);
AjPPosRegexp ajPosRegCompCase (AjPStr exp);
AjPPosRegexp ajPosRegCompCaseC (const char* exp);
AjPPosRegexp ajPosRegCompNosub (AjPStr exp);
AjPPosRegexp ajPosRegCompNosubC (const char* exp);
AjPPosRegexp ajPosRegCompNewline (AjPStr exp);
AjPPosRegexp ajPosRegCompNewlineC (const char* exp);

/* execute expression match */

AjBool ajPosRegExec (AjPPosRegexp prog, AjPStr str);
AjBool ajPosRegExecC (AjPPosRegexp prog, const char* str);

/* test substrings */

ajint ajPosRegLenI (AjPPosRegexp rp, ajint isub);
AjBool ajPosRegPost (AjPPosRegexp rp, AjPStr* post);
AjBool ajPosRegPostC (AjPPosRegexp rp, const char** post);

/* substitute substrings */

ajint ajPosRegOffset (AjPPosRegexp rp);
ajint ajPosRegOffsetI (AjPPosRegexp rp, ajint isub);
ajint ajPosRegOffsetC (AjPPosRegexp rp);
ajint ajPosRegOffsetIC (AjPPosRegexp rp, ajint isub);

void ajPosRegSub (AjPPosRegexp rp, AjPStr source, AjPStr* dest);
void ajPosRegSubI (AjPPosRegexp rp, ajint isub, AjPStr* dest);
void ajPosRegSubC (AjPPosRegexp rp, const char* source, AjPStr* dest);

/* destructor */

void ajPosRegFree (AjPPosRegexp* exp);
void ajPosRegTrace (AjPPosRegexp exp);

/* error messages */

void ajPosRegErr (AjPPosRegexp prog, ajint errcode);

#endif

#ifdef __cplusplus
}
#endif

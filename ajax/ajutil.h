#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajutil_h
#define ajutil_h




/*
** Prototype definitions
*/

__noreturn void   ajExit (void);
__noreturn void   ajExitAbort (void);
__noreturn void   ajExitBad (void);
void   ajLogInfo (void);
AjBool ajUtilBigendian (void);
void   ajUtilCatch (void);
void   ajUtilRev2 (short* i);
void   ajUtilRev4 (ajint* i);
void   ajUtilRev8 (ajlong* i);
void   ajUtilRevInt(ajint* sval);
void   ajUtilRevShort(short* ival);
void   ajUtilRevLong(ajlong* lval);
void   ajUtilRevUint(ajuint* ival);

AjBool ajUtilUid (AjPStr* dest);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

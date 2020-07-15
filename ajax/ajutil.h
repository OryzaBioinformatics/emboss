#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajutil_h
#define ajutil_h




/*
** Prototype definitions
*/

void   ajExit (void);
ajint  ajExitAbort (void);
ajint  ajExitBad (void);
void   ajLogInfo (void);
AjBool ajUtilBigendian (void);
void   ajUtilCatch (void);
void   ajUtilRev2 (short* i);
void   ajUtilRev4 (ajint* i);
void   ajUtilRev8 (ajlong* i);
void   ajUtilRevInt(ajint* sval);
void   ajUtilRevShort(short* ival);
void   ajUtilRevLong(ajlong* lval);
AjBool ajUtilUid (AjPStr* dest);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

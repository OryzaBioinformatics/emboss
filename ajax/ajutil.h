#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajutil_h
#define ajutil_h

void ajExit (void);
ajint ajExitBad (void);
void ajLogInfo (void);
AjBool ajUtilBigendian (void);
void ajUtilRev4 (ajint* i);
void ajUtilRev2 (short* i);
AjBool ajUtilUid (AjPStr* dest);
#endif

#ifdef __cplusplus
}
#endif

/*  Last edited: Feb  7 16:19 2000 (pmr) */
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajutil_h
#define ajutil_h

void ajExit (void);
int ajExitBad (void);
void ajLogInfo (void);
AjBool ajUtilBigendian (void);
void ajUtilRev4 (int* i);
void ajUtilRev2 (short* i);
AjBool ajUtilUid (AjPStr* dest);
#endif

#ifdef __cplusplus
}
#endif

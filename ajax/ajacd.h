#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajacd_h
#define ajacd_h

#include "ajax.h"

AjBool        ajAcdDebug (void);
AjBool        ajAcdDebugIsSet (void);
void          ajAcdExit (AjBool single);
AjBool        ajAcdFilter (void);
AjPAlign      ajAcdGetAlign (char *token);
AjPFloat      ajAcdGetArray (char *token);
AjBool        ajAcdGetBool (char *token);
AjPCod        ajAcdGetCodon (char *token);
AjPPdb        ajAcdGetCpdb (char *token);
AjPFile       ajAcdGetDatafile (char *token);
AjPList       ajAcdGetDirlist (char *token);
AjPFeattable  ajAcdGetFeat (char *token);
AjPFeattabOut ajAcdGetFeatout (char *token);
AjPList       ajAcdGetFilelist (char *token);
float         ajAcdGetFloat (char *token);
AjPGraph      ajAcdGetGraph (char *token);
AjPGraph      ajAcdGetGraphxy (char *token);
AjPFile       ajAcdGetInfile (char *token);
ajint           ajAcdGetInt (char *token);
AjPStr*       ajAcdGetList (char *token);
AjPStr        ajAcdGetListI (char *token, ajint num);
AjPMatrix     ajAcdGetMatrix (char *token);
AjPMatrixf    ajAcdGetMatrixf (char *token);
AjPFile       ajAcdGetOutfile (char *token);
AjPPdb        ajAcdGetCpdb (char *token);
AjPRange      ajAcdGetRange (char *token);
AjPRegexp     ajAcdGetRegexp (char *token);
AjPReport     ajAcdGetReport (char *token);
AjPScop       ajAcdGetScop (char *token);
AjPStr*       ajAcdGetSelect (char *token);
AjPStr        ajAcdGetSelectI (char *token, ajint num);
AjPSeq        ajAcdGetSeq (char *token);
AjPSeqall     ajAcdGetSeqall (char *token);
AjPSeqout     ajAcdGetSeqout (char *token);
AjPSeqout     ajAcdGetSeqoutall (char *token);
AjPSeqout     ajAcdGetSeqoutset (char *token);
AjPSeqset     ajAcdGetSeqset (char *token);
AjPStr        ajAcdGetString (char *token);
AjStatus      ajAcdInit (char *pgm, ajint argc, char *argv[]);
AjStatus      ajAcdInitP (char *pgm, ajint argc, char *argv[], char *package);
void          ajAcdPrintType (AjPFile outf, AjBool full);
char*         ajAcdProgram (void);
void          ajAcdProgramS (AjPStr* pgm);
AjBool        ajAcdStdout (void);
AjPStr        ajAcdValue (char* token);

#endif

#ifdef __cplusplus
}
#endif

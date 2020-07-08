#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajacd_h
#define ajacd_h

#include "ajax.h"

AjBool    ajAcdDebug (void);
AjBool    ajAcdDebugIsSet (void);
AjBool    ajAcdFilter (void);
AjPFloat  ajAcdGetArray (char *token);
AjBool    ajAcdGetBool (char *token);
AjPCod    ajAcdGetCodon (char *token);
AjPFile   ajAcdGetDatafile (char *token);
AjPFeatTable  ajAcdGetFeat (char *token);
AjPFeatTabOut ajAcdGetFeatout (char *token);
float     ajAcdGetFloat (char *token);
AjPGraph  ajAcdGetGraph (char *token);
AjPGraph  ajAcdGetGraphxy (char *token);
AjPFile   ajAcdGetInfile (char *token);
int       ajAcdGetInt (char *token);
AjPStr*   ajAcdGetList (char *token);
AjPStr    ajAcdGetListI (char *token, int num);
AjPMatrix ajAcdGetMatrix (char *token);
AjPMatrixf ajAcdGetMatrixf (char *token);
AjPFile   ajAcdGetOutfile (char *token);
AjPRange  ajAcdGetRange (char *token);
AjPRegexp ajAcdGetRegexp (char *token);
AjPStr*   ajAcdGetSelect (char *token);
AjPStr    ajAcdGetSelectI (char *token, int num);
AjPSeq    ajAcdGetSeq (char *token);
AjPSeqall ajAcdGetSeqall (char *token);
AjPSeqout ajAcdGetSeqout (char *token);
AjPSeqout ajAcdGetSeqoutall (char *token);
AjPSeqout ajAcdGetSeqoutset (char *token);
AjPSeqset ajAcdGetSeqset (char *token);
AjPStr    ajAcdGetString (char *token);
AjStatus  ajAcdInit (char *pgm, int argc, char *argv[]);
AjStatus  ajAcdInitP (char *pgm, int argc, char *argv[], char *package);
void      ajAcdPrintType (AjPFile outf, AjBool full);
char*     ajAcdProgram (void);
void      ajAcdProgramS (AjPStr* pgm);
AjBool    ajAcdStdout (void);
AjPStr    ajAcdValue (char* token);

#endif

#ifdef __cplusplus
}
#endif

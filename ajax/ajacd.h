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
AjPAlign      ajAcdGetAlign (const char *token);
AjPFloat      ajAcdGetArray (const char *token);
AjBool        ajAcdGetBool (const char *token);
AjPCod        ajAcdGetCodon (const char *token);
AjPPdb        ajAcdGetCpdb (const char *token);
AjPFile       ajAcdGetDatafile (const char *token);
AjPList       ajAcdGetDirlist (const char *token);
AjPPhyloState* ajAcdGetDiscretestates (const char *token);
AjPPhyloState ajAcdGetDiscretestatesI (const char *token, ajint num);
AjPPhyloDist  ajAcdGetDistances (const char *token);
AjPFeattable  ajAcdGetFeat (const char *token);
AjPFeattabOut ajAcdGetFeatout (const char *token);
AjPList       ajAcdGetFilelist (const char *token);
float         ajAcdGetFloat (const char *token);
AjPPhyloFreq  ajAcdGetFrequencies (const char *token);
AjPGraph      ajAcdGetGraph (const char *token);
AjPGraph      ajAcdGetGraphxy (const char *token);
AjPFile       ajAcdGetInfile (const char *token);
ajint         ajAcdGetInt (const char *token);
AjPStr*       ajAcdGetList (const char *token);
AjPStr        ajAcdGetListI (const char *token, ajint num);
AjPMatrix     ajAcdGetMatrix (const char *token);
AjPMatrixf    ajAcdGetMatrixf (const char *token);
AjPFile       ajAcdGetOutfile (const char *token);
AjPPdb        ajAcdGetCpdb (const char *token);
AjPPhyloProp  ajAcdGetProperties (const char *token);
AjPRange      ajAcdGetRange (const char *token);
AjPRegexp     ajAcdGetRegexp (const char *token);
AjPReport     ajAcdGetReport (const char *token);
AjPScop       ajAcdGetScop (const char *token);
AjPStr*       ajAcdGetSelect (const char *token);
AjPStr        ajAcdGetSelectI (const char *token, ajint num);
AjPSeq        ajAcdGetSeq (const char *token);
AjPSeqall     ajAcdGetSeqall (const char *token);
AjPSeqout     ajAcdGetSeqout (const char *token);
AjPSeqout     ajAcdGetSeqoutall (const char *token);
AjPSeqout     ajAcdGetSeqoutset (const char *token);
AjPSeqset     ajAcdGetSeqset (const char *token);
AjPSeqset*    ajAcdGetSeqsetall (const char *token);
AjPSeqset     ajAcdGetSeqsetallI (const char *token, ajint num);
AjPStr        ajAcdGetString (const char *token);
AjPPhyloTree* ajAcdGetTree (const char *token);
AjPPhyloTree  ajAcdGetTreeI (const char *token, ajint num);
AjStatus      ajAcdInit (const char *pgm, ajint argc, char *argv[]);
AjStatus      ajAcdInitP (const char *pgm, ajint argc, char *argv[],
			  const char *package);
void          ajAcdPrintType (AjPFile outf, AjBool full);
void          ajAcdPrintQual(AjPFile outf, AjBool full);
char*         ajAcdProgram (void);
void          ajAcdProgramS (AjPStr* pgm);
AjBool        ajAcdSetControl (const char* optionName);
AjBool        ajAcdStdout (void);
AjPStr        ajAcdValue (const char* token);

#endif

#ifdef __cplusplus
}
#endif

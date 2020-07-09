#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embiep_h
#define embiep_h

#define EMBIEPSIZE 28			/* Usual alpha plus two array */
#define EMBIEPAMINO 26			/* Amino array index          */
#define EMBIEPCARBOXYL 27		/* Carboxyl array index       */



void   embIepCalcK (double *K);
void   embIepComp (char *s, ajint amino, ajint *c);
double embIepGetCharge (ajint *c, double *pro, double *total);
void   embIepGetProto (double *K, ajint *c, ajint *op, double H, double *pro);
AjBool embIepIEP (char *s, ajint amino, double *iep, AjBool termini);
double embIepKToPk (double K);
double embIepPhConcToPh (double H);
double embIepPhConverge (ajint *c, double *K, ajint *op, double *pro);
void   embIepPkRead (void);
double embIepPhToHConc (double pH);
double embIepPkToK (double pK);

#endif

#ifdef __cplusplus
}
#endif

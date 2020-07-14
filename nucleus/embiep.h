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
void   embIepComp (const char *s, ajint amino, ajint *c);
double embIepGetCharge (const ajint *c, const double *pro, double *total);
void   embIepGetProto (const double *K, const ajint *c,
		       ajint *op, double H, double *pro);
AjBool embIepIEP (const char *s, ajint amino, double *iep, AjBool termini);
double embIepKToPk (double K);
double embIepPhConcToPh (double H);
double embIepPhConverge (const ajint *c, const double *K,
			 ajint *op, double *pro);
void   embIepPkRead (void);
double embIepPhToHConc (double pH);
double embIepPkToK (double pK);

#endif

#ifdef __cplusplus
}
#endif

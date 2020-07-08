#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdan_h
#define ajdan_h

#include "ajax.h"

typedef struct AjMelt AjMelt;
struct AjMelt
{
    AjPStr pair;
    float enthalpy;
    float entropy;
    float energy;
};


extern AjBool aj_melt_I;
extern int    aj_melt_savesize;
extern AjBool aj_melt_saveinit;

extern void ajMeltInit(AjPStr *type, int savesize);
extern float ajProbScore(AjPStr *seq1, AjPStr *seq2, int len);
extern float ajMeltEnergy(AjPStr *strand, int len, int shift, AjBool isDNA,
		   AjBool maySave, float *enthalpy, float *entropy);
extern float ajMeltEnergy2(char *strand, int pos, int len, AjBool isDNA,
			   float *enthalpy, float *entropy, float **saveentr,
			   float **saveenth, float **saveener);
extern float ajTm(AjPStr *strand, int len, int shift, float saltconc,
	   float DNAconc, AjBool isDNA);
float ajTm2(char *strand, int pos, int len, float saltconc,
	    float DNAconc, AjBool isDNA);
extern float ajMeltGC(AjPStr *strand, int len);
extern float ajProdTm(float gc, float saltconc, int len);
extern float ajAnneal(float tmprimer, float tmproduct);

#endif

#ifdef __cplusplus
}
#endif

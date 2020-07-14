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
extern ajint    aj_melt_savesize;
extern AjBool aj_melt_saveinit;

extern void ajMeltInit(const AjPStr type, ajint savesize);
extern float ajProbScore(const AjPStr seq1, const AjPStr seq2, ajint len);
extern float ajMeltEnergy(const AjPStr strand, ajint len,
			  ajint shift, AjBool isDNA,
			  AjBool maySave, float *enthalpy, float *entropy);
extern float ajMeltEnergy2(const char *strand, ajint pos, ajint len,
			   AjBool isDNA,
			   float *enthalpy, float *entropy, float **saveentr,
			   float **saveenth, float **saveener);
extern float ajTm(const AjPStr strand, ajint len, ajint shift, float saltconc,
	   float DNAconc, AjBool isDNA);
float ajTm2(const char *strand, ajint pos, ajint len, float saltconc,
	    float DNAconc, AjBool isDNA);
extern float ajMeltGC(const AjPStr strand, ajint len);
extern float ajProdTm(float gc, float saltconc, ajint len);
extern float ajAnneal(float tmprimer, float tmproduct);

#endif

#ifdef __cplusplus
}
#endif

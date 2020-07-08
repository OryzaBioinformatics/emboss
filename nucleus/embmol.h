#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embmol_h
#define embmol_h


#define EMBMOLPARDISP (double)1000000.0

typedef struct EmbSMolFrag
{
    int begin;
    int end;
    double mwt;
} EmbOMolFrag, *EmbPMolFrag;



int embMolGetFrags(AjPStr thys, int rno, AjPList *l);




#endif

#ifdef __cplusplus
}
#endif

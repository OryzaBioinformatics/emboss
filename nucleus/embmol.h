#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embmol_h
#define embmol_h


#define EMBMOLPARDISP (double)1000000.0

/* @data EmbPMolFrag **********************************************************
**
** Nucleus seqeunce molecular fragment object.
**
** @@
******************************************************************************/

typedef struct EmbSMolFrag
{
    ajint begin;
    ajint end;
    double mwt;
} EmbOMolFrag, *EmbPMolFrag;



ajint embMolGetFrags(AjPStr thys, ajint rno, AjPList *l);




#endif

#ifdef __cplusplus
}
#endif

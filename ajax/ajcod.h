#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajcod_h
#define ajcod_h


/* @data AjPCod *******************************************************
**
** Ajax codon object.
**
** Holds arrays describing codon usage
** The length is known and held internally.
**
** AjPCod is implemented as a pointer to a C data structure.
**
** @alias AjSCod
** @alias AjOCod
**
** @@
******************************************************************************/

typedef struct AjSCod
{
    AjPStr name;		/* Name of codon file                   */
    ajint *aa;			/* Amino acid represented by codon      */
    ajint *num;			/* Number of codons                     */
    double *tcount;		/* Codons per thousand                  */
    double *fraction;		/* Fraction of amino acids of this type */
    ajint *back;			/* Index of favoured amino acid         */
} AjOCod, *AjPCod;


void    ajCodBacktranslate(AjPStr *b, AjPStr a, AjPCod thys);
ajint   ajCodBase(ajint c);
double  ajCodCai(AjPCod cod, AjPStr str);
void    ajCodCalcGribskov(AjPCod *nrm, AjPStr s);
double  ajCodCalcCai(AjPCod *thys);
double  *ajCodCaiW(AjPCod cod);
double  ajCodCalcNc(AjPCod *thys);
void    ajCodCalculateUsage(AjPCod *thys, ajint c);
void    ajCodClear(AjPCod *thys);
void    ajCodComp(ajint *NA, ajint *NC, ajint *NG, ajint *NT, char *str);
void    ajCodCountTriplets(AjPCod *thys, AjPStr s, ajint *c);
void    ajCodDel (AjPCod *thys);
AjPCod  ajCodDup (AjPCod thys);
ajint     ajCodIndexC(char *codon);
ajint     ajCodIndex(AjPStr s);
AjPCod	ajCodNew(void);
AjBool  ajCodRead(AjPStr fn, AjPCod *thys);
void    ajCodSetBacktranslate(AjPCod *thys);
char*   ajCodTriplet(ajint idx);
void 	ajCodWrite(AjPFile outf, AjPCod thys);

#endif

#ifdef __cplusplus
}
#endif

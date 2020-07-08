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
    int *aa;			/* Amino acid represented by codon      */
    int *num;			/* Number of codons                     */
    double *tcount;		/* Codons per thousand                  */
    double *fraction;		/* Fraction of amino acids of this type */
    int *back;			/* Index of favoured amino acid         */
} AjOCod, *AjPCod;


void    ajCodBacktranslate(AjPStr *b, AjPStr a, AjPCod thys);
int     ajCodBase(int c);
void    ajCodCalcGribskov(AjPCod *nrm, AjPStr s);
double  ajCodCalcNc(AjPCod *thys);
void    ajCodCalculateUsage(AjPCod *thys, int c);
void    ajCodClear(AjPCod *thys);
void    ajCodComp(int *NA, int *NC, int *NG, int *NT, char *str);
void    ajCodCountTriplets(AjPCod *thys, AjPStr s, int *c);
void    ajCodDel (AjPCod *thys);
AjPCod  ajCodDup (AjPCod thys);
int     ajCodIndexC(char *codon);
int     ajCodIndex(AjPStr s);
AjPCod	ajCodNew(void);
AjBool  ajCodRead(AjPStr fn, AjPCod *thys);
void    ajCodSetBacktranslate(AjPCod *thys);
char*   ajCodTriplet(int idx);
void 	ajCodWrite(AjPFile outf, AjPCod thys);

#endif

#ifdef __cplusplus
}
#endif

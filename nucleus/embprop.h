#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embprop_h
#define embprop_h

#define EMBPROPSIZE 28
#define EMBPROPMOLWT     0
#define EMBEMBPROPTINY      1
#define EMBPROPSMALL     2
#define EMBPROPALIPHATIC 3
#define EMBPROPAROMATIC  4
#define EMBPROPNONPOLAR  5
#define EMBPROPPOLAR     6
#define EMBPROPCHARGE    7
#define EMBPROPPOSITIVE  8
#define EMBPROPNEGATIVE  9

  /* define monoisotopic masses for common N- and C- terminal 
     modifications */
#define EMBPROPMSTN_H       1.00782
#define EMBPROPMSTN_FORMYL 29.00274
#define EMBPROPMSTN_ACETYL 43.01839

#define EMBPROPMSTC_OH     17.00274
#define EMBPROPMSTC_AMIDE  16.01872



extern double *EmbPropTable[];



typedef struct EmbSPropFrag	/* Enzyme digestion structure */
{
    ajint     start;
    ajint     end;
    double  molwt;
} EmbOPropFrag, *EmbPPropFrag;



void    embPropAminoRead (AjPFile fp);
  /* void    embPropAminoRead (void); */
void 	embPropCalcFragments (char *s, ajint n, ajint begin,
			      AjPList *l, AjPList *pa,
			      AjBool unfavoured, AjBool overlap,
			      AjBool allpartials, ajint *ncomp, ajint *npart,
			      AjPStr *rname);
double  embPropCalcMolwt (char *s, ajint start, ajint end);
  /* new method for chemically modified ends */
double  embPropCalcMolwtMod (char *s, ajint start, ajint end, double nmass, double cmass);
char*   embPropCharToThree (char c);
char*   embPropIntToThree (ajint c);
AjPStr  embPropProtGaps (AjPSeq seq, ajint pad);
AjPStr  embPropProt1to3 (AjPSeq seq, ajint pad);
AjBool  embPropPurine (char base);
AjBool  embPropPyrimidine (char base);
AjBool  embPropTransversion (char base1, char base2);
AjBool  embPropTransition (char base1, char base2);

#endif

#ifdef __cplusplus
}
#endif

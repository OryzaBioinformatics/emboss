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


extern double *EmbPropTable[];



typedef struct EmbSPropFrag	/* Enzyme digestion structure */
{
    int     start;
    int     end;
    double  molwt;
} EmbOPropFrag, *EmbPPropFrag;



void    embPropAminoRead (void);
void 	embPropCalcFragments (char *s, int n, int begin,
			      AjPList *l, AjPList *pa,
			      AjBool unfavoured, AjBool overlap,
			      AjBool allpartials, int *ncomp, int *npart,
			      AjPStr *rname);
double  embPropCalcMolwt (char *s, int start, int end);
char*   embPropCharToThree (char c);
char*   embPropIntToThree (int c);
AjPStr  embPropProtGaps (AjPSeq seq, int pad);
AjPStr  embPropProt1to3 (AjPSeq seq, int pad);
AjBool  embPropPurine (char base);
AjBool  embPropPyrimidine (char base);
AjBool  embPropTransversion (char base1, char base2);
AjBool  embPropTransition (char base1, char base2);

#endif

#ifdef __cplusplus
}
#endif

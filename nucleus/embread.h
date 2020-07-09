#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embread_h
#define embread_h

#define AJREADAMINO 28

AjBool embReadAminoDataDoubleC (char *s, double **a, double fill);
AjBool embReadAminoDataFloatC  (char *s, float **a, float fill);
AjBool embReadAminoDataIntC    (char *s, ajint **a, ajint fill);

#endif

#ifdef __cplusplus
}
#endif

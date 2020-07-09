#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsort_h
#define ajsort_h

extern void ajSortFloatDecI(float *a, ajint *p, ajint n);
extern void ajSortIntDecI(ajint *a, ajint *p, ajint n);
extern void ajSortFloatIncI(float *a, ajint *p, ajint n);
extern void ajSortIntIncI(ajint *a, ajint *p, ajint n);
extern void ajSortFloatDec(float *a, ajint n);
extern void ajSortIntDec(ajint *a, ajint n);
extern void ajSortFloatInc(float *a, ajint n);
extern void ajSortIntInc(ajint *a, ajint n);

#endif

#ifdef __cplusplus
}
#endif

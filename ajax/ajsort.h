#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsort_h
#define ajsort_h

extern void ajSortFloatDecI(const float *a, ajint *p, ajint n);
extern void ajSortIntDecI(const ajint *a, ajint *p, ajint n);
extern void ajSortFloatIncI(const float *a, ajint *p, ajint n);
extern void ajSortTwoIntIncI(ajint *a, ajint *p, ajint n);
extern void ajSortIntIncI(const ajint *a, ajint *p, ajint n);
extern void ajSortFloatDec(float *a, ajint n);
extern void ajSortIntDec(ajint *a, ajint n);
extern void ajSortFloatInc(float *a, ajint n);
extern void ajSortIntInc(ajint *a, ajint n);

#endif

#ifdef __cplusplus
}
#endif

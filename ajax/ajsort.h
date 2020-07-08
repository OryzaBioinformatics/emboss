#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsort_h
#define ajsort_h

extern void ajSortFloatDecI(float *a, int *p, int n);
extern void ajSortIntDecI(int *a, int *p, int n);
extern void ajSortFloatIncI(float *a, int *p, int n);
extern void ajSortIntIncI(int *a, int *p, int n);
extern void ajSortFloatDec(float *a, int n);
extern void ajSortIntDec(int *a, int n);
extern void ajSortFloatInc(float *a, int n);
extern void ajSortIntInc(int *a, int n);

#endif

#ifdef __cplusplus
}
#endif

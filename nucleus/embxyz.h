#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embxyz_h
#define embxyz_h

AjBool embXyzSeqsetNR(AjPList input, AjPInt *keep, ajint *nset,
		      AjPMatrixf matrix, float gapopen, float gapextend,
		      float thresh);

AjBool embXyzSeqsetNRRange(AjPList input, AjPInt *keep, ajint *nset,
		      AjPMatrixf matrix, float gapopen, float gapextend,
		      float thresh1, float thresh2);

#endif

#ifdef __cplusplus
}
#endif

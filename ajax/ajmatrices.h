#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmatrices_h
#define ajmatrices_h

/* @data AjPMatrix ************************************************************
**
** AJAX data structure for sequence comparison matrices with integer values
**
** @alias AjSMatrix
** @alias AjOMatrix
******************************************************************************/

typedef struct AjSMatrix {
  ajint Size;
  AjPStr Name;
  AjPStr* Codes;
  ajint** Matrix;
  AjPSeqCvt Cvt;
} AjOMatrix, *AjPMatrix;

/* @data AjPMatrixf ***********************************************************
**
** AJAX data structure for sequence comparison matrices with floating
** point values
**
** @alias AjSMatrixf
** @alias AjOMatrixf
******************************************************************************/

typedef struct AjSMatrixf {
  ajint Size;
  AjPStr Name;
  AjPStr* Codes;
  float** Matrixf;
  AjPSeqCvt Cvt;
} AjOMatrixf, *AjPMatrixf;

ajint**    ajMatrixArray (AjPMatrix thys);
void       ajMatrixChar (AjPMatrix thys, ajint i, AjPStr *label);
AjPSeqCvt  ajMatrixCvt (AjPMatrix thys);
void       ajMatrixDel (AjPMatrix *thys);
AjPStr     ajMatrixName (AjPMatrix thys);
AjPMatrix  ajMatrixNew (AjPStr* codes, ajint n, AjPStr filename);
AjBool     ajMatrixSeqNum (AjPMatrix thys, AjPSeq seq, AjPStr* numseq);
AjBool     ajMatrixRead (AjPMatrix* pthis, AjPStr filename);
ajint      ajMatrixSize (AjPMatrix thys);

float**    ajMatrixfArray (AjPMatrixf thys);
void       ajMatrixfChar (AjPMatrixf thys, ajint i, AjPStr *label);
AjPSeqCvt  ajMatrixfCvt (AjPMatrixf thys);
void       ajMatrixfDel (AjPMatrixf *thys);
AjPStr     ajMatrixfName (AjPMatrixf thys);
AjPMatrixf ajMatrixfNew (AjPStr* codes, ajint n, AjPStr filename);
AjBool     ajMatrixfSeqNum (AjPMatrixf thys, AjPSeq seq, AjPStr* numseq);
AjBool     ajMatrixfRead (AjPMatrixf* pthis, AjPStr filename);
ajint      ajMatrixfSize (AjPMatrixf thys);

AjBool     ajSeqsetConsStats(AjPSeqset thys, AjPMatrix mymatrix, AjPStr *cons,
			     ajint* retident, ajint* retsim, ajint* retgap,
			     ajint* retlen);

#endif

#ifdef __cplusplus
}
#endif

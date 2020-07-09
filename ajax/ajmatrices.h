#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmatrices_h
#define ajmatrices_h

/* @data AjPMatrix *******************************************************
**
** AJAX data structure for sequence comparison matrices with integer values
**
** @alias AjSMatrix
******************************************************************************/

typedef struct AjSMatrix {
  ajint Size;
  AjPStr Name;
  AjPStr Codes;
  ajint** Matrix;
  AjPSeqCvt Cvt;
} AjOMatrix, *AjPMatrix;

/* @data AjPMatrixf *******************************************************
**
** AJAX data structure for sequence comparison matrices with floating
** point values
**
** @alias AjSMatrixf
******************************************************************************/

typedef struct AjSMatrixf {
  ajint Size;
  AjPStr Name;
  AjPStr Codes;
  float** Matrixf;
  AjPSeqCvt Cvt;
} AjOMatrixf, *AjPMatrixf;

ajint**    ajMatrixArray (AjPMatrix thys);
char       ajMatrixChar (AjPMatrix thys, ajint i);
AjPSeqCvt  ajMatrixCvt (AjPMatrix thys);
void       ajMatrixDel (AjPMatrix *thys);
AjPStr     ajMatrixName (AjPMatrix thys);
AjPMatrix  ajMatrixNew (char* codes, AjPStr filename);
AjBool     ajMatrixSeqNum (AjPMatrix thys, AjPSeq seq, AjPStr* numseq);
AjBool     ajMatrixRead (AjPMatrix* matrix, AjPStr filename) ;
ajint      ajMatrixSize (AjPMatrix thys);

float**    ajMatrixfArray (AjPMatrixf thys);
char       ajMatrixfChar (AjPMatrixf thys, ajint i);
AjPSeqCvt  ajMatrixfCvt (AjPMatrixf thys);
void       ajMatrixfDel (AjPMatrixf *thys);
AjPStr     ajMatrixfName (AjPMatrixf thys);
AjPMatrixf ajMatrixfNew (char* codes, AjPStr filename);
AjBool     ajMatrixfSeqNum (AjPMatrixf thys, AjPSeq seq, AjPStr* numseq);
AjBool     ajMatrixfRead (AjPMatrixf* matrix, AjPStr filename) ;
ajint      ajMatrixfSize (AjPMatrixf thys);


#endif

#ifdef __cplusplus
}
#endif

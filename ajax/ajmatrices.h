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
**
** @attr Size [ajint] Matrix size (size of 2D array)
** @attr Name [AjPStr] Matrix name
** @attr Codes [AjPStr*] Row/column codes
** @attr Matrix [AjIntArray*] Matrix as 2D array
** @attr Cvt [AjPSeqCvt] Conversion table
** @@
******************************************************************************/

typedef struct AjSMatrix {
  ajint Size;
  AjPStr Name;
  AjPStr* Codes;
  AjIntArray* Matrix;
  AjPSeqCvt Cvt;
} AjOMatrix;
#define AjPMatrix AjOMatrix*

/* @data AjPMatrixf ***********************************************************
**
** AJAX data structure for sequence comparison matrices with floating
** point values
**
** @alias AjSMatrixf
** @alias AjOMatrixf
**
** @attr Size [ajint] Matrix size (size of 2D array)
** @attr Name [AjPStr] Matrix name
** @attr Codes [AjPStr*] Row/column codes
** @attr Matrixf [AjFloatArray*] Matrix as 2D array
** @attr Cvt [AjPSeqCvt] Conversion table
** @@
******************************************************************************/

typedef struct AjSMatrixf {
  ajint Size;
  AjPStr Name;
  AjPStr* Codes;
  AjFloatArray* Matrixf;
  AjPSeqCvt Cvt;
} AjOMatrixf;
#define AjPMatrixf AjOMatrixf*

AjIntArray*   ajMatrixArray (const AjPMatrix thys);
void          ajMatrixChar (const AjPMatrix thys, ajint i, AjPStr *label);
AjPSeqCvt     ajMatrixCvt (const AjPMatrix thys);
void          ajMatrixDel (AjPMatrix *thys);
AjPStr        ajMatrixName (const AjPMatrix thys);
AjPMatrix     ajMatrixNew (const AjPPStr codes, ajint n,
			   const AjPStr filename);
AjBool        ajMatrixSeqNum (const AjPMatrix thys, const AjPSeq seq,
			      AjPStr* numseq);
AjBool        ajMatrixRead (AjPMatrix* pthis, const AjPStr filename);
ajint         ajMatrixSize (const AjPMatrix thys);

AjFloatArray* ajMatrixfArray (const AjPMatrixf thys);
void          ajMatrixfChar (const AjPMatrixf thys, ajint i, AjPStr *label);
AjPSeqCvt     ajMatrixfCvt (const AjPMatrixf thys);
void          ajMatrixfDel (AjPMatrixf *thys);
AjPStr        ajMatrixfName (const AjPMatrixf thys);
AjPMatrixf    ajMatrixfNew (const AjPPStr codes, ajint n,
			    const AjPStr filename);
AjBool        ajMatrixfSeqNum (const AjPMatrixf thys, const AjPSeq seq,
			       AjPStr* numseq);
AjBool        ajMatrixfRead (AjPMatrixf* pthis, const AjPStr filename);
ajint         ajMatrixfSize (const AjPMatrixf thys);

#endif

#ifdef __cplusplus
}
#endif

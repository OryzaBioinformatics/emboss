#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajmatrices_h
#define ajmatrices_h

typedef struct AjSMatrix {
  int Size;
  AjPStr Name;
  AjPStr Codes;
  int** Matrix;
  AjPSeqCvt Cvt;
} AjOMatrix, *AjPMatrix;

typedef struct AjSMatrixf {
  int Size;
  AjPStr Name;
  AjPStr Codes;
  float** Matrixf;
  AjPSeqCvt Cvt;
} AjOMatrixf, *AjPMatrixf;

int**     ajMatrixArray (AjPMatrix thys);
char      ajMatrixChar (AjPMatrix thys, int i);
AjPSeqCvt ajMatrixCvt (AjPMatrix thys);
AjPStr    ajMatrixName (AjPMatrix thys);
AjPMatrix ajMatrixNew (char* codes, AjPStr filename);
AjBool    ajMatrixSeqNum (AjPMatrix thys, AjPSeq seq, AjPStr* numseq);
AjBool    ajMatrixRead (AjPMatrix* matrix, AjPStr filename) ;
int       ajMatrixSize (AjPMatrix thys);

float**    ajMatrixfArray (AjPMatrixf thys);
char       ajMatrixfChar (AjPMatrixf thys, int i);
AjPSeqCvt  ajMatrixfCvt (AjPMatrixf thys);
AjPMatrixf ajMatrixfDel (AjPMatrixf *thys);
AjPStr     ajMatrixfName (AjPMatrixf thys);
AjPMatrixf ajMatrixfNew (char* codes, AjPStr filename);
AjBool     ajMatrixfSeqNum (AjPMatrixf thys, AjPSeq seq, AjPStr* numseq);
AjBool     ajMatrixfRead (AjPMatrixf* matrix, AjPStr filename) ;
int        ajMatrixfSize (AjPMatrixf thys);


#endif

#ifdef __cplusplus
}
#endif

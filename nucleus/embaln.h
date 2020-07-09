#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embaln_h
#define embaln_h

#define PAZ  26
#define PAZ1 27

void embAlignCalcSimilarity(AjPStr m, AjPStr n, float **sub, AjPSeqCvt cvt,
			    int lenm, int lenn, float *id, float *sim,
			    float *idx, float *simx);

void embAlignPathCalc(char *a, char *b, int lena, int lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     int *compass, AjBool show);


void embAlignPrintGlobal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			int start1, int start2, float score, AjBool mark,
			float **sub, AjPSeqCvt cvt, char *namea,
			char *nameb, int begina, int beginb);
void embAlignPrintLocal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
		       int start1, int start2, float score, AjBool mark,
		       float **sub, AjPSeqCvt cvt, char *namea,
		       char *nameb, int begina, int beginb);
void embAlignPrintProfile(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			 int start1, int start2, float score, AjBool mark,
			 float **fmatrix, char *namea,
			 char *nameb, int begina, int beginb);

void embAlignProfilePathCalc(char *a, int mlen, int slen, float gapopen,
			    float gapextend, float *path, float **fmatrix,
			    int *compass, AjBool show);

float embAlignScoreNWMatrix(float *path, AjPSeq a, AjPSeq b, float **fmatrix,
			   AjPSeqCvt cvt, int lena, int lenb, float gapopen, int *compass,
			   float gapextend, int *start1, int *start2);

float embAlignScoreProfileMatrix(float *path, int *compass, float gapopen,
				float gapextend, AjPStr b,
				int clen, int slen, float **fmatrix,
				int *start1, int *start2);
float embAlignScoreSWMatrix(float *path, int *compass, float gapopen,
			  float gapextend,  AjPSeq a, AjPSeq b,
			  int lena, int lenb, float **sub,
			   AjPSeqCvt cvt, int *start1, int *start2);

void embAlignWalkNWMatrix(float *path, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, int lena, int lenb, int *start1,
			 int *start2, float gapopen,
			 float gapextend, AjPSeqCvt cvt, int *compass, float **sub);
void embAlignWalkProfileMatrix(float *path, int *compass, float gapopen,
			      float gapextend, AjPStr cons, AjPStr b,
			      AjPStr *m, AjPStr *n, int clen, int slen,
			      float **fmatrix, int *start1, int *start2);
void embAlignWalkSWMatrix(float *path, int *compass, float gapopen,
			 float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, int lena, int lenb, float **sub,
			 AjPSeqCvt cvt, int *start1, int *start2);

void embAlignPathCalcFast(char *a, char *b, int lena, int lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     int *compass, AjBool show,int width);

float embAlignScoreSWMatrixFast(float *path, int *compass, float gapopen,
                          float gapextend,  AjPSeq a, AjPSeq b,
                          int lena, int lenb, float **sub,
                          AjPSeqCvt cvt, int *start1, int *start2,int width);

void embAlignWalkSWMatrixFast(float *path, int *compass, float gapopen,
			 float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, int lena, int lenb, float **sub,
			 AjPSeqCvt cvt, int *start1, int *start2,int width);

#endif

#ifdef __cplusplus
}
#endif

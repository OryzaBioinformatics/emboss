#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embaln_h
#define embaln_h

#define PAZ  26
#define PAZ1 27

void embAlignCalcSimilarity(AjPStr m, AjPStr n, float **sub, AjPSeqCvt cvt,
			    ajint lenm, ajint lenn, float *id, float *sim,
			    float *idx, float *simx);

void embAlignPathCalc(char *a, char *b, ajint lena, ajint lenb, float gapopen,
		     float gapextend, float *path, float **sub, AjPSeqCvt cvt,
		     ajint *compass, AjBool show);


void embAlignPrintGlobal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			ajint start1, ajint start2, float score, AjBool mark,
			float **sub, AjPSeqCvt cvt, char *namea,
			char *nameb, ajint begina, ajint beginb);
void embAlignPrintLocal(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
		       ajint start1, ajint start2, float score, AjBool mark,
		       float **sub, AjPSeqCvt cvt, char *namea,
		       char *nameb, ajint begina, ajint beginb);
void embAlignPrintProfile(AjPFile outf, char *a, char *b, AjPStr m, AjPStr n,
			 ajint start1, ajint start2, float score, AjBool mark,
			 float **fmatrix, char *namea,
			 char *nameb, ajint begina, ajint beginb);

void embAlignProfilePathCalc(char *a, ajint mlen, ajint slen, float gapopen,
			    float gapextend, float *path, float **fmatrix,
			    ajint *compass, AjBool show);

void embAlignReportGlobal (AjPAlign align, AjPSeq seqa, AjPSeq seqb,
			  AjPStr m, AjPStr n,
			  ajint start1, ajint start2,
			  float gapopen, float gapextend,
			  float score, AjPMatrixf matrix,
			   ajint offset1, ajint offset2);
void embAlignReportLocal (AjPAlign align, AjPSeq seqa, AjPSeq seqb,
			  AjPStr m, AjPStr n,
			  ajint start1, ajint start2,
			  float gapopen, float gapextend,
			  float score, AjPMatrixf matrix,
			  ajint offset1, ajint offset2);
void embAlignReportProfile(AjPAlign align, AjPSeqset seqset,
			   char *a, char *b, AjPStr m, AjPStr n,
			   ajint start1, ajint start2,
			   float score, AjBool mark,
			   float **fmatrix, char *namea,
			   char *nameb, ajint begina, ajint beginb);

float embAlignScoreNWMatrix(float *path, AjPSeq a, AjPSeq b, float **fmatrix,
			    AjPSeqCvt cvt, ajint lena, ajint lenb,
			    float gapopen, ajint *compass,
			    float gapextend, ajint *start1, ajint *start2);

float embAlignScoreProfileMatrix(float *path, ajint *compass, float gapopen,
				 float gapextend, AjPStr b,
				 ajint clen, ajint slen, float **fmatrix,
				 ajint *start1, ajint *start2);
float embAlignScoreSWMatrix(float *path, ajint *compass, float gapopen,
			    float gapextend,  AjPSeq a, AjPSeq b,
			    ajint lena, ajint lenb, float **sub,
			    AjPSeqCvt cvt, ajint *start1, ajint *start2);

void embAlignWalkNWMatrix(float *path, AjPSeq a, AjPSeq b, AjPStr *m,
			  AjPStr *n, ajint lena, ajint lenb, ajint *start1,
			  ajint *start2, float gapopen,
			  float gapextend, AjPSeqCvt cvt,
			  ajint *compass, float **sub);
void embAlignWalkProfileMatrix(float *path, ajint *compass, float gapopen,
			      float gapextend, AjPStr cons, AjPStr b,
			      AjPStr *m, AjPStr *n, ajint clen, ajint slen,
			      float **fmatrix, ajint *start1, ajint *start2);
void embAlignWalkSWMatrix(float *path, ajint *compass, float gapopen,
			 float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			 AjPStr *n, ajint lena, ajint lenb, float **sub,
			 AjPSeqCvt cvt, ajint *start1, ajint *start2);

void embAlignPathCalcFast(char *a, char *b, ajint lena, ajint lenb,
			  float gapopen, float gapextend, float *path,
			  float **sub, AjPSeqCvt cvt,
			  ajint *compass, AjBool show, ajint width);

float embAlignScoreSWMatrixFast(float *path, ajint *compass, float gapopen,
				float gapextend,  AjPSeq a, AjPSeq b,
				ajint lena, ajint lenb, float **sub,
				AjPSeqCvt cvt, ajint *start1, ajint *start2,
				ajint width);

void embAlignWalkSWMatrixFast(float *path, ajint *compass, float gapopen,
			      float gapextend, AjPSeq a, AjPSeq b, AjPStr *m,
			      AjPStr *n, ajint lena, ajint lenb, float **sub,
			      AjPSeqCvt cvt, ajint *start1, ajint *start2,
			      ajint width);

#endif

#ifdef __cplusplus
}
#endif

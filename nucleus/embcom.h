#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embcom_h
#define embcom_h


#define MAX_W  50


struct STR
 {
  char WORD[MAX_W];
 };

typedef struct STR STRING;


struct traccia {
    ajint ind;
    float pc;
   };

/*
** Gary Williams - 5 Aug 2000
** changed struct trace to struct comtrace
** to avoid conflict with 'trace' in ncurses.h
*/
typedef struct traccia comtrace;


struct ujwin{
  float *Ujwin;
 };

typedef struct ujwin UJWin;

struct ujsim{
  UJWin *Ujsim;
 };

typedef struct ujsim UJSim;

struct sqsim{
  char *Sqsim;
 };
 
typedef struct sqsim SEQSim;

void embComComplexity(char *seq,char *name,int len,int jmin,int jmax,
		      ajint l,int step, ajint sim,int freq,int omnia,
		      AjPFile fp,AjPFile pf,
		      ajint print,int num_seq, float *MedValue);
void embComWriteValueOfSeq(AjPFile fp,int n,char *name,int len,float MedValue);
void embComWriteFile(AjPFile fp,int jmin,int jmax,int lwin,int step,int sim);
void embComUnused (void);

#endif

#ifdef __cplusplus
}
#endif

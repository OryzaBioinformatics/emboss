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

void embComComplexity(const char *seq,const char *name,
		      ajint len, ajint jmin, ajint jmax,
		      ajint l, ajint step, ajint sim, ajint freq, ajint omnia,
		      AjPFile fp, AjPFile pf,
		      ajint print, ajint num_seq, float *MedValue);
void embComWriteValueOfSeq(AjPFile fp, ajint n,const char *name, ajint len,
			   float MedValue);
void embComWriteFile(AjPFile fp, ajint jmin, ajint jmax, ajint lwin,
		     ajint step, ajint sim);
void embComUnused (void);

#endif

#ifdef __cplusplus
}
#endif

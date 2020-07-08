#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embpat_h
#define embpat_h


/*
 *  Defines for string search algorithms
 */
#define AJALPHA  256			/* Alphabet			*/
#define AJMOD256 0xff
#define AJALPHA2 128			/* ASCII printable		*/
#define AJWORD   32			/* Size of a word		*/
#define AJBPS    1			/* Bits per state		*/



typedef struct EmbSPatMatch {
  int number;
  int *start;
  int *len;
} EmbOPatMatch, *EmbPPatMatch;



typedef struct EmbSPatRestrict
{
    AjPStr cod;				/* RE name                         */
    AjPStr pat;				/* Recognition site                */
    AjPStr bin;				/* Binary converted site           */
    int    len;				/* Pattern length                  */
    AjBool blunt;			/* Blunt true, sticky false        */
    int    ncuts;			/* Number of cuts                  */
    int    cut1;			/* First  3' cut                   */
    int    cut2;			/* First  5' cut                   */
    int    cut3;			/* Second 3' cut		   */
    int    cut4;			/* Second 5' cut                   */
    AjPStr org;				/* Organism                        */
    AjPStr iso;                         /* Isoschizomers                   */
    AjPStr meth;                        /* Methylation                     */
    AjPStr sou;                         /* Source                          */
    AjPStr sup;                         /* Suppliers                       */
} EmbOPatRestrict, *EmbPPatRestrict;




/*
 *   Node structure for Baeza-Yates & Perleberg algorithm
 */
typedef struct EmbSPatBYPNode
{
    int offset;
    struct EmbSPatBYPNode *next;
} EmbOPatBYPNode, *EmbPPatBYPNode;




void        embPatBMHInit (AjPStr *pat, int len, int *next);
int         embPatBMHSearch (AjPStr *str, AjPStr *pat, int slen, int plen,
			     int *skip, int start, AjBool left, AjBool right,
			     AjPList *l, AjPStr *name, int begin);

int         embPatBruteForce (AjPStr *seq, AjPStr *pat, AjBool amino,
			      AjBool carboxyl,
			      AjPList *l, int begin, int mm, AjPStr *name);

void        embPatBYGCInit (AjPStr *pat, int *m, unsigned int *table,
			    unsigned int *limit);
int         embPatBYGSearch (AjPStr *str, AjPStr *name, int begin, int plen,
			     unsigned int *table, unsigned int limit,
			     AjPList l, AjBool amino, AjBool carboxyl);

void        embPatBYPInit (AjPStr *pat, int len, EmbPPatBYPNode offset,
			   int *buf);
int         embPatBYPSearch (AjPStr *str, AjPStr *name, int begin, int slen,
			     int plen, int mm, EmbPPatBYPNode offset, int *buf,
			     AjPList l, AjBool amino, AjBool carboxyl,
			     AjPStr pat);

AjBool      embPatClassify (AjPStr *pat, AjBool *amino, AjBool *carboxyl,
			    AjBool *fclass, AjBool *ajcompl, AjBool *dontcare,
			    AjBool *range, AjBool protein);

void embPatCompile(int type, AjPStr pattern, AjPStr opattern, int* plen,
		   int** buf, EmbOPatBYPNode* off, unsigned int** sotable,
		   int* solimit, int* m, AjPStr* regexp, int*** skipm,
		   int mismatch);

void embPatFuzzSearch(int type, int begin, AjPStr pattern,
		      AjPStr opattern, AjPStr name, AjPStr text, AjPList *l,
		      int plen, int mismatch, AjBool left, AjBool right,
		      int *buf, EmbOPatBYPNode *off, unsigned int *sotable,
		      int solimit, AjPStr regexp, int **skipm,
		      int *hits, int m, void **tidy);

int embPatGetType(AjPStr *pattern, int mismatch, AjBool protein, int *m,
		  AjBool *left, AjBool *right);

void        embPatKMPInit (AjPStr *pat, int len, int *next);
int         embPatKMPSearch (AjPStr *str, AjPStr *pat, int slen, int plen,
			     int *next, int start);

void        embPatMatchDel (EmbPPatMatch* pthis);
EmbPPatMatch embPatMatchFind  (AjPStr regexp, AjPStr string);
EmbPPatMatch embPatMatchFindC (AjPStr regexp, char *sptr);
int         embPatMatchGetEnd (EmbPPatMatch data, int index);
int         embPatMatchGetLen (EmbPPatMatch data, int index);
int         embPatMatchGetNumber (EmbPPatMatch data);
int         embPatMatchGetStart (EmbPPatMatch data, int index);

EmbPPatMatch embPatPosMatchFind  (AjPStr regexp, AjPStr string);
EmbPPatMatch embPatPosMatchFindC (AjPStr regexp, char *sptr);
void        embPatPosMatchDel (EmbPPatMatch* pthis);
int         embPatPosMatchGetEnd (EmbPPatMatch data, int index);
int         embPatPosMatchGetNumber (EmbPPatMatch data);
int         embPatPosMatchGetLen (EmbPPatMatch data, int index);
int         embPatPosMatchGetStart (EmbPPatMatch data, int index);
AjPStr      embPatPosSeqCreateRegExp  (AjPStr thys, AjBool protein);
AjPStr      embPatPosSeqCreateRegExpC (char *ptr, AjBool protein);
EmbPPatMatch embPatPosSeqMatchFind  (AjPSeq seq, AjPStr reg);
EmbPPatMatch embPatPosSeqMatchFindC (AjPSeq seq, char *reg);

AjPStr      embPatPrositeToRegExp (AjPStr *s);

void        embPatPushHit (AjPList l, AjPStr *name, int pos, int plen,
			   int begin, int mm);

void        embPatRestrictDel (EmbPPatRestrict *thys);
EmbPPatRestrict embPatRestrictNew (void);
int         embPatRestrictMatch (AjPSeq seq, int begin, int end,
				 AjPFile enzfile, AjPStr enzymes,
				 int sitelen, AjBool plasmid,
				 AjBool ambiguity, int min, int max,
				 AjBool blunt, AjBool sticky,
				 AjBool commercial, AjPList *l);
AjBool      embPatRestrictReadEntry (EmbPPatRestrict *re, AjPFile *inf);

int         embPatRestrictRestrict (AjPList *l, int hits, AjBool isos,
				    AjBool alpha);
int         embPatRestrictScan (EmbPPatRestrict *enz, AjPStr *substr,
				AjPStr *binstr, AjPStr *revstr,
				AjPStr *binrev, int len,
				AjBool ambiguity, AjBool plasmid, int min,
				int max, int begin, AjPList *l);

EmbPPatMatch embPatSeqMatchFind  (AjPSeq seq, AjPStr reg);
EmbPPatMatch embPatSeqMatchFindC (AjPSeq seq, char *reg);
AjPStr      embPatSeqCreateRegExp  (AjPStr thys, AjBool protein);
AjPStr      embPatSeqCreateRegExpC (char *ptr, AjBool protein);

void        embPatSOInit (AjPStr *pat, unsigned int *table,
			  unsigned int *limit);
int         embPatSOSearch (AjPStr *str, AjPStr *name, unsigned int first,
			    int begin, int plen, unsigned int *table,
			    unsigned int limit, AjPList l,
			    AjBool amino, AjBool carboxyl);

void        embPatTUBInit (AjPStr *pat, int **skipm, int m, int k, int plen);
int         embPatTUBSearch (AjPStr *pat,AjPStr *text, int slen,
			     int **skipm, int m,
			     int k, int begin, AjPList l, AjBool amino,
			     AjBool carboxyl, AjPStr *name, int plen);

void        embPatTUInit (AjPStr *pat, int **skipm, int m, int k);
int         embPatTUSearch (AjPStr *pat,AjPStr *text, int slen,
			    int **skipm, int m,
			    int k, int begin, AjPList l, AjBool amino,
			    AjBool carboxyl, AjPStr *name);

int         embPatVariablePattern (AjPStr *pattern, AjPStr opattern,
				   AjPStr text,
				   AjPStr patname, AjPList l, int mode,
				   int mismatch, int begin);

#endif

#ifdef __cplusplus
}
#endif

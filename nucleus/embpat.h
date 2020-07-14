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


/* @data EmbPPatMatch *********************************************************
**
** NUCLEUS data structure for pattern matches
**
******************************************************************************/

typedef struct EmbSPatMatch {
  ajint number;
  ajint *start;
  ajint *len;
} EmbOPatMatch, *EmbPPatMatch;


/* @data EmbPPatRestrict ******************************************************
**
** NUCLEUS data structure for pattern matches
**
******************************************************************************/

typedef struct EmbSPatRestrict
{
    AjPStr cod;				/* RE name                         */
    AjPStr pat;				/* Recognition site                */
    AjPStr bin;				/* Binary converted site           */
    ajint    len;			/* Pattern length                  */
    AjBool blunt;			/* Blunt true, sticky false        */
    ajint    ncuts;			/* Number of cuts                  */
    ajint    cut1;			/* First  3' cut                   */
    ajint    cut2;			/* First  5' cut                   */
    ajint    cut3;			/* Second 3' cut		   */
    ajint    cut4;			/* Second 5' cut                   */
    AjPStr org;				/* Organism                        */
    AjPStr iso;                         /* Isoschizomers                   */
    AjPStr meth;                        /* Methylation                     */
    AjPStr sou;                         /* Source                          */
    AjPStr sup;                         /* Suppliers                       */
} EmbOPatRestrict, *EmbPPatRestrict;




/* @data EmbPPatBYPNode *******************************************************
**
** NUCLEUS data structure for nodes in Baeza-Yates & Perleberg algorithm
**
******************************************************************************/

typedef struct EmbSPatBYPNode
{
    ajint offset;
    struct EmbSPatBYPNode *next;
} EmbOPatBYPNode, *EmbPPatBYPNode;




void        embPatBMHInit (AjPStr *pat, ajint len, ajint *next);
ajint         embPatBMHSearch (AjPStr *str, AjPStr *pat,
			       ajint slen, ajint plen,
			     ajint *skip, ajint start,
			       AjBool left, AjBool right,
			     AjPList *l, AjPStr *name, ajint begin);

ajint         embPatBruteForce (AjPStr *seq, AjPStr *pat, AjBool amino,
			      AjBool carboxyl,
			      AjPList *l, ajint begin, ajint mm, AjPStr *name);

void        embPatBYGCInit (AjPStr *pat, ajint *m, ajuint *table,
			    ajuint *limit);
ajint         embPatBYGSearch (AjPStr *str, AjPStr *name,
			       ajint begin, ajint plen,
			     ajuint *table, ajuint limit,
			     AjPList l, AjBool amino, AjBool carboxyl);

void        embPatBYPInit (AjPStr *pat, ajint len, EmbPPatBYPNode offset,
			   ajint *buf);
ajint         embPatBYPSearch (AjPStr *str, AjPStr *name, ajint begin,
			       ajint slen, ajint plen, ajint mm,
			       EmbPPatBYPNode offset, ajint *buf,
			     AjPList l, AjBool amino, AjBool carboxyl,
			     AjPStr pat);

AjBool      embPatClassify (AjPStr *pat, AjBool *amino, AjBool *carboxyl,
			    AjBool *fclass, AjBool *ajcompl, AjBool *dontcare,
			    AjBool *range, AjBool protein);

void embPatCompile(ajint type, AjPStr pattern, AjPStr opattern, ajint* plen,
		   ajint** buf, EmbOPatBYPNode* off, ajuint** sotable,
		   ajuint* solimit, ajint* m, AjPStr* regexp, ajint*** skipm,
		   ajint mismatch);

void            embPatFuzzSearch(ajint type, ajint begin, AjPStr pattern,
				 AjPStr opattern, AjPStr name,
				 AjPStr text, AjPList *l,
				 ajint plen, ajint mismatch,
				 AjBool left, AjBool right,
				 ajint *buf, EmbOPatBYPNode *off,
				 ajuint *sotable,
				 ajint solimit, AjPStr regexp, ajint **skipm,
				 ajint *hits, ajint m, void **tidy);

ajint           embPatGetType(AjPStr *pattern, ajint mismatch, AjBool protein,
			      ajint *m, AjBool *left, AjBool *right);

void            embPatKMPInit (AjPStr *pat, ajint len, ajint *next);
ajint           embPatKMPSearch (AjPStr *str, AjPStr *pat,
			       ajint slen, ajint plen,
			     ajint *next, ajint start);

void            embPatMatchDel (EmbPPatMatch* pthis);
EmbPPatMatch    embPatMatchFind  (AjPStr regexp, AjPStr string);
EmbPPatMatch    embPatMatchFindC (AjPStr regexp, char *sptr);
ajint           embPatMatchGetEnd (EmbPPatMatch data, ajint index);
ajint           embPatMatchGetLen (EmbPPatMatch data, ajint index);
ajint           embPatMatchGetNumber (EmbPPatMatch data);
ajint           embPatMatchGetStart (EmbPPatMatch data, ajint index);

AjPStr          embPatPrositeToRegExp (AjPStr *s);

void            embPatPushHit (AjPList l, AjPStr *name, ajint pos, ajint plen,
			   ajint begin, ajint mm);

void            embPatRestrictDel (EmbPPatRestrict *thys);
EmbPPatRestrict embPatRestrictNew (void);
ajint           embPatRestrictMatch (AjPSeq seq, ajint begin, ajint end,
				 AjPFile enzfile, AjPStr enzymes,
				 ajint sitelen, AjBool plasmid,
				 AjBool ambiguity, ajint min, ajint max,
				 AjBool blunt, AjBool sticky,
				 AjBool commercial, AjPList *l);
void            embPatRestrictPreferred(AjPList l, AjPTable t);
AjBool          embPatRestrictReadEntry (EmbPPatRestrict *re, AjPFile *inf);

ajint           embPatRestrictRestrict (AjPList *l, ajint hits, AjBool isos,
				    AjBool alpha);
ajint           embPatRestrictScan (EmbPPatRestrict *enz, AjPStr *substr,
				    AjPStr *binstr, AjPStr *revstr,
				    AjPStr *binrev, ajint len,
				    AjBool ambiguity,
				    AjBool plasmid, ajint min,
				    ajint max, ajint begin, AjPList *l);

ajint           embPatRestrictCutCompare(const void *a, const void *b);
ajint           embPatRestrictNameCompare(const void *a, const void *b);
ajint           embPatRestrictStartCompare(const void *a, const void *b);
EmbPPatMatch    embPatSeqMatchFind  (AjPSeq seq, AjPStr reg);
EmbPPatMatch    embPatSeqMatchFindC (AjPSeq seq, char *reg);
AjPStr          embPatSeqCreateRegExp  (AjPStr thys, AjBool protein);
AjPStr          embPatSeqCreateRegExpC (char *ptr, AjBool protein);

void            embPatSOInit (AjPStr *pat, ajuint *table,
			      ajuint *limit);
ajint           embPatSOSearch (AjPStr *str, AjPStr *name, ajuint first,
				ajint begin, ajint plen, ajuint *table,
				ajuint limit, AjPList l,
				AjBool amino, AjBool carboxyl);

void            embPatTUBInit (AjPStr *pat, ajint **skipm,
			   ajint m, ajint k, ajint plen);
ajint           embPatTUBSearch (AjPStr *pat,AjPStr *text, ajint slen,
				 ajint **skipm, ajint m,
				 ajint k, ajint begin, AjPList l, AjBool amino,
				 AjBool carboxyl, AjPStr *name, ajint plen);

void            embPatTUInit (AjPStr *pat, ajint **skipm, ajint m, ajint k);
ajint           embPatTUSearch (AjPStr *pat,AjPStr *text, ajint slen,
				ajint **skipm, ajint m,
				ajint k, ajint begin, AjPList l, AjBool amino,
				AjBool carboxyl, AjPStr *name);

ajint           embPatVariablePattern (AjPStr *pattern, AjPStr opattern,
				       AjPStr text,
				       AjPStr patname, AjPList l, ajint mode,
				       ajint mismatch, ajint begin);

#endif

#ifdef __cplusplus
}
#endif

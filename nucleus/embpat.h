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
** @attr number [ajint] Number of matches
** @attr start [ajint*] Match start positions
** @attr len [ajint*] Match lengths
** @@
******************************************************************************/

typedef struct EmbSPatMatch {
  ajint number;
  ajint *start;
  ajint *len;
} EmbOPatMatch;
#define EmbPPatMatch EmbOPatMatch*


/* @data EmbPPatRestrict ******************************************************
**
** NUCLEUS data structure for pattern matches
**
** @attr cod [AjPStr] Restriction Enzyme name
** @attr pat [AjPStr] Recognition site
** @attr bin [AjPStr] Binary converted site
** @attr len [ajint] Pattern length
** @attr blunt [AjBool] Blunt true, sticky false
** @attr ncuts [ajint] Number of cuts
** @attr cut1 [ajint] First  3' cut
** @attr cut2 [ajint] First  5' cut
** @attr cut3 [ajint] Second 3' cut
** @attr cut4 [ajint] Second 5' cut
** @attr org [AjPStr] Organism
** @attr iso [AjPStr] Isoschizomers
** @attr meth [AjPStr] Methylation
** @attr sou [AjPStr] Source
** @attr sup [AjPStr] Suppliers
** @@
******************************************************************************/

typedef struct EmbSPatRestrict
{
    AjPStr cod;
    AjPStr pat;
    AjPStr bin;
    ajint    len;
    AjBool blunt;
    ajint    ncuts;
    ajint    cut1;
    ajint    cut2;
    ajint    cut3;
    ajint    cut4;
    AjPStr org;
    AjPStr iso;
    AjPStr meth;
    AjPStr sou;
    AjPStr sup;
} EmbOPatRestrict;
#define EmbPPatRestrict EmbOPatRestrict*




/* @data EmbPPatBYPNode *******************************************************
**
** NUCLEUS data structure for nodes in Baeza-Yates & Perleberg algorithm
**
** @attr offset [ajint] Offset
** @attr next [struct EmbSPatBYPNode*] Pointer to next node
** @@
******************************************************************************/

typedef struct EmbSPatBYPNode
{
    ajint offset;
    struct EmbSPatBYPNode *next;
} EmbOPatBYPNode;
#define EmbPPatBYPNode EmbOPatBYPNode*




void            embPatBMHInit (const AjPStr pat, ajint len, ajint *next);
ajint           embPatBMHSearch (const AjPStr str, const AjPStr pat,
				 ajint slen, ajint plen,
				 const ajint *skip, ajint start,
				 AjBool left, AjBool right,
				 AjPList l, const AjPStr name, ajint begin);

ajint           embPatBruteForce (const AjPStr seq, const AjPStr pat,
				  AjBool amino,
				  AjBool carboxyl,
				  AjPList l, ajint begin, ajint mm,
				  const AjPStr name);

void            embPatBYGCInit (const AjPStr pat, ajint *m, ajuint *table,
				ajuint *limit);
ajint           embPatBYGSearch (const AjPStr str, const AjPStr name,
				 ajint begin, ajint plen,
				 const ajuint *table, ajuint limit,
				 AjPList l, AjBool amino, AjBool carboxyl);

void            embPatBYPInit (const AjPStr pat, ajint len,
			       EmbPPatBYPNode offset, ajint *buf);
ajint           embPatBYPSearch (const AjPStr str, const AjPStr name,
				 ajint begin,
				 ajint slen, ajint plen, ajint mm,
				 EmbPPatBYPNode offset, ajint *buf,
				 AjPList l, AjBool amino, AjBool carboxyl,
				 const AjPStr pat);

AjBool          embPatClassify (const AjPStr pat, AjPStr *cleanpat,
				AjBool *amino, AjBool *carboxyl,
				AjBool *fclass, AjBool *ajcompl,
				AjBool *dontcare, AjBool *range,
				AjBool protein);

void            embPatCompile(ajint type, const AjPStr pattern,
			      ajint* plen, ajint** buf,
			      EmbPPatBYPNode off, ajuint** sotable,
			      ajuint* solimit, ajint* m, AjPStr* regexp,
			      ajint*** skipm,  ajint mismatch);

void            embPatFuzzSearch(ajint type, ajint begin, const AjPStr pattern,
				 const AjPStr name,
				 const AjPStr text, AjPList l,
				 ajint plen, ajint mismatch,
				 AjBool left, AjBool right,
				 ajint *buf, EmbPPatBYPNode off,
				 const ajuint *sotable,
				 ajint solimit, const AjPStr regexp,
				 ajint * const *skipm,
				 ajint *hits, ajint m, void **tidy);

ajint           embPatGetType(const AjPStr pattern, AjPStr *cleanpat,
			      ajint mismatch,
			      AjBool protein,
			      ajint *m, AjBool *left, AjBool *right);

void            embPatKMPInit (const AjPStr pat, ajint len, ajint *next);
ajint           embPatKMPSearch (const AjPStr str, const AjPStr pat,
				 ajint slen, ajint plen,
				 const ajint *next, ajint start);

void            embPatMatchDel (EmbPPatMatch* pthis);
EmbPPatMatch    embPatMatchFind  (const AjPStr regexp, const AjPStr string);
EmbPPatMatch    embPatMatchFindC (const AjPStr regexp, const char *sptr);
ajint           embPatMatchGetEnd (const EmbPPatMatch data, ajint index);
ajint           embPatMatchGetLen (const EmbPPatMatch data, ajint index);
ajint           embPatMatchGetNumber (const EmbPPatMatch data);
ajint           embPatMatchGetStart (const EmbPPatMatch data, ajint index);

AjPStr          embPatPrositeToRegExp (const AjPStr s);
AjPStr          embPatPrositeToRegExpEnds (const AjPStr s,
					   AjBool start, AjBool end);

void            embPatPushHit (AjPList l, const AjPStr name,
			       ajint pos, ajint plen,
			       ajint begin, ajint mm);

void            embPatRestrictDel (EmbPPatRestrict *thys);
EmbPPatRestrict embPatRestrictNew (void);
ajint           embPatRestrictMatch (const AjPSeq seq, ajint begin, ajint end,
				     AjPFile enzfile, const AjPStr enzymes,
				     ajint sitelen, AjBool plasmid,
				     AjBool ambiguity, ajint min, ajint max,
				     AjBool blunt, AjBool sticky,
				     AjBool commercial, AjPList l);
void            embPatRestrictPreferred(AjPList l, const AjPTable t);
AjBool          embPatRestrictReadEntry (EmbPPatRestrict re, AjPFile inf);

ajint           embPatRestrictRestrict (AjPList l, ajint hits, AjBool isos,
					AjBool alpha);
ajint           embPatRestrictScan (const EmbPPatRestrict enz,
				    const AjPStr substr,
				    const AjPStr binstr, const AjPStr revstr,
				    const AjPStr binrev, ajint len,
				    AjBool ambiguity,
				    AjBool plasmid, ajint min,
				    ajint max, ajint begin, AjPList l);

ajint           embPatRestrictCutCompare(const void *a, const void *b);
ajint           embPatRestrictNameCompare(const void *a, const void *b);
ajint           embPatRestrictStartCompare(const void *a, const void *b);
EmbPPatMatch    embPatSeqMatchFind  (const AjPSeq seq, const AjPStr reg);
EmbPPatMatch    embPatSeqMatchFindC (const AjPSeq seq, const char *reg);
AjPStr          embPatSeqCreateRegExp  (const AjPStr thys, AjBool protein);
AjPStr          embPatSeqCreateRegExpC (const char *ptr, AjBool protein);

void            embPatSOInit (const AjPStr pat, ajuint *table,
			      ajuint *limit);
ajint           embPatSOSearch (const AjPStr str, const AjPStr name,
				ajuint first,
				ajint begin, ajint plen, const ajuint *table,
				ajuint limit, AjPList l,
				AjBool amino, AjBool carboxyl);

void            embPatTUBInit (const AjPStr pat, ajint **skipm,
			       ajint m, ajint k, ajint plen);
ajint           embPatTUBSearch (const AjPStr pat,const AjPStr text,
				 ajint slen,
				 ajint * const *skipm, ajint m,
				 ajint k, ajint begin, AjPList l, AjBool amino,
				 AjBool carboxyl, const AjPStr name,
				 ajint plen);

void            embPatTUInit (const AjPStr pat,
			      ajint **skipm, ajint m, ajint k);
ajint           embPatTUSearch (const AjPStr pat,const AjPStr text, ajint slen,
				ajint * const *skipm, ajint m,
				ajint k, ajint begin, AjPList l, AjBool amino,
				AjBool carboxyl, const AjPStr name);

ajint           embPatVariablePattern (const AjPStr pattern,
				       const AjPStr text,
				       const AjPStr patname, AjPList l,
				       ajint mode,
				       ajint mismatch, ajint begin);

#endif

#ifdef __cplusplus
}
#endif

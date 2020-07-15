#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embpat_h
#define embpat_h






/* @data EmbPPatMatch *********************************************************
**
** NUCLEUS data structure for pattern matches
**
** @attr start [ajuint*] Match start positions
** @attr len [ajuint*] Match lengths
** @attr number [ajuint] Number of matches
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSPatMatch {
  ajuint *start;
  ajuint *len;
  ajuint number;
  char Padding[4];
} EmbOPatMatch;
#define EmbPPatMatch EmbOPatMatch*




/* @data EmbPPatRestrict ******************************************************
**
** NUCLEUS data structure for pattern matches
**
** @attr cod [AjPStr] Restriction Enzyme name
** @attr pat [AjPStr] Recognition site
** @attr bin [AjPStr] Binary converted site
** @attr len [ajuint] Pattern length
** @attr blunt [AjBool] Blunt true, sticky false
** @attr cut1 [ajint] First  3' cut
** @attr cut2 [ajint] First  5' cut
** @attr cut3 [ajint] Second 3' cut
** @attr cut4 [ajint] Second 5' cut
** @attr org [AjPStr] Organism
** @attr iso [AjPStr] Isoschizomers
** @attr meth [AjPStr] Methylation
** @attr sou [AjPStr] Source
** @attr sup [AjPStr] Suppliers
** @attr ncuts [ajuint] Number of cuts
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct EmbSPatRestrict
{
    AjPStr cod;
    AjPStr pat;
    AjPStr bin;
    ajuint  len;
    AjBool blunt;
    ajint  cut1;
    ajint  cut2;
    ajint  cut3;
    ajint  cut4;
    AjPStr org;
    AjPStr iso;
    AjPStr meth;
    AjPStr sou;
    AjPStr sup;
    ajuint  ncuts;
    char Padding[4];
} EmbOPatRestrict;
#define EmbPPatRestrict EmbOPatRestrict*




#define EmbPPatBYPNode AjOPatBYPNode*
#define EmbOPatBYPNode AjOPatBYPNode




/*
** Prototype definitions
*/

void            embPatBMHInit (const AjPStr pat, ajuint len, ajint *next);
ajuint          embPatBMHSearch (const AjPStr str, const AjPStr pat,
				 ajuint slen, ajuint plen,
				 const ajint *skip, ajuint start,
				 AjBool left, AjBool right,
				 AjPList l, const AjPStr name, ajuint begin);

ajuint          embPatBruteForce (const AjPStr seq, const AjPStr pat,
				  AjBool amino,
				  AjBool carboxyl,
				  AjPList l, ajuint begin, ajuint mm,
				  const AjPStr name);

void            embPatBYGCInit (const AjPStr pat, ajuint *m, ajuint *table,
				ajuint *limit);
ajuint          embPatBYGSearch (const AjPStr str, const AjPStr name,
				 ajuint begin, ajuint plen,
				 const ajuint *table, ajuint limit,
				 AjPList l, AjBool amino, AjBool carboxyl);

void            embPatBYPInit (const AjPStr pat, ajuint len,
			       EmbPPatBYPNode offset, ajint *buf);
ajuint          embPatBYPSearch (const AjPStr str, const AjPStr name,
				 ajuint begin,
				 ajuint slen, ajuint plen, ajuint mm,
				 EmbPPatBYPNode offset, ajint *buf,
				 AjPList l, AjBool amino, AjBool carboxyl,
				 const AjPStr pat);

AjBool          embPatClassify (const AjPStr pat, AjPStr *cleanpat,
				AjBool *amino, AjBool *carboxyl,
				AjBool *fclass, AjBool *ajcompl,
				AjBool *dontcare, AjBool *range,
				AjBool protein);

void            embPatCompile(ajuint type, const AjPStr pattern,
			      ajuint* plen, ajint** buf,
			      EmbPPatBYPNode off, ajuint** sotable,
			      ajuint* solimit, ajuint* m, AjPStr* regexp,
			      ajuint*** skipm,  ajuint mismatch);

void            embPatFuzzSearch(ajuint type, ajuint begin,
				 const AjPStr pattern,
				 const AjPStr name,
				 const AjPStr text, AjPList l,
				 ajuint plen, ajuint mismatch,
				 AjBool left, AjBool right,
				 ajint *buf, EmbPPatBYPNode off,
				 const ajuint *sotable,
				 ajuint solimit, const AjPStr regexp,
				 ajuint * const *skipm,
				 ajuint *hits, ajuint m, const void **tidy);

ajuint          embPatGetType(const AjPStr pattern, AjPStr *cleanpat,
			      ajuint mismatch,
			      AjBool protein,
			      ajuint *m, AjBool *left, AjBool *right);

void		embPatCompileII (AjPPatComp thys, ajuint mismatch);
void		embPatFuzzSearchII (AjPPatComp thys, ajuint begin,
				   const AjPStr name, const AjPStr text,
				   AjPList l, ajuint mismatch, ajuint *hits,
				   const void **tidy);
ajuint   	embPatGetTypeII (AjPPatComp thys, const AjPStr pattern,
				 ajuint mismatch, AjBool protein);

void            embPatKMPInit (const AjPStr pat, ajuint len, ajint *next);
ajuint          embPatKMPSearch (const AjPStr str, const AjPStr pat,
				 ajuint slen, ajuint plen,
				 const ajint *next, ajuint start);

void            embPatMatchDel (EmbPPatMatch* pthis);
EmbPPatMatch    embPatMatchFind  (const AjPStr regexp, const AjPStr strng,
				  AjBool left, AjBool right);
EmbPPatMatch    embPatMatchFindC (const AjPStr regexp, const char *sptr,
				  AjBool left, AjBool right);
ajuint          embPatMatchGetEnd (const EmbPPatMatch data, ajuint indexnum);
ajuint          embPatMatchGetLen (const EmbPPatMatch data, ajuint indexnum);
ajuint          embPatMatchGetNumber (const EmbPPatMatch data);
ajuint          embPatMatchGetStart (const EmbPPatMatch data, ajuint indexnum);

AjPStr          embPatPrositeToRegExp (const AjPStr s);
AjPStr          embPatPrositeToRegExpEnds (const AjPStr s,
					   AjBool start, AjBool end);

void            embPatPushHit (AjPList l, const AjPStr name,
			       ajuint pos, ajuint plen,
			       ajuint begin, ajuint mm);

void            embPatRestrictDel (EmbPPatRestrict *thys);
EmbPPatRestrict embPatRestrictNew (void);
ajuint          embPatRestrictMatch (const AjPSeq seq,
				     ajuint begin, ajuint end,
				     AjPFile enzfile, const AjPStr enzymes,
				     ajuint sitelen, AjBool plasmid,
				     AjBool ambiguity, ajuint min, ajuint max,
				     AjBool blunt, AjBool sticky,
				     AjBool commercial, AjPList l);
void            embPatRestrictPreferred(AjPList l, const AjPTable t);
AjBool          embPatRestrictReadEntry (EmbPPatRestrict re, AjPFile inf);

ajuint          embPatRestrictRestrict (AjPList l, ajuint hits, AjBool isos,
					AjBool alpha);
ajuint          embPatRestrictScan (const EmbPPatRestrict enz,
				    const AjPStr substr,
				    const AjPStr binstr, const AjPStr revstr,
				    const AjPStr binrev, ajuint len,
				    AjBool ambiguity,
				    AjBool plasmid, ajuint min,
				    ajuint max, ajuint begin, AjPList l);

ajint           embPatRestrictCutCompare(const void *a, const void *b);
ajint           embPatRestrictNameCompare(const void *a, const void *b);
ajint           embPatRestrictStartCompare(const void *a, const void *b);
EmbPPatMatch    embPatSeqMatchFind  (const AjPSeq seq, const AjPStr reg);
EmbPPatMatch    embPatSeqMatchFindC (const AjPSeq seq, const char *reg);
AjPStr          embPatSeqCreateRegExp  (const AjPStr thys, AjBool protein);
AjPStr          embPatSeqCreateRegExpC (const char *ptr, AjBool protein);

void            embPatSOInit (const AjPStr pat, ajuint *table,
			      ajuint *limit);
ajuint          embPatSOSearch (const AjPStr str, const AjPStr name,
				ajint first,
				ajuint begin, ajuint plen, const ajuint *table,
				ajuint limit, AjPList l,
				AjBool amino, AjBool carboxyl);

void            embPatTUBInit (const AjPStr pat, ajuint **skipm,
			       ajuint m, ajuint k, ajuint plen);
ajuint          embPatTUBSearch (const AjPStr pat,const AjPStr text,
				 ajuint slen,
				 ajuint * const *skipm, ajuint m,
				 ajuint k, ajuint begin,
				 AjPList l, AjBool amino,
				 AjBool carboxyl, const AjPStr name,
				 ajuint plen);

void            embPatTUInit (const AjPStr pat,
			      ajuint **skipm, ajuint m, ajuint k);
ajuint          embPatTUSearch (const AjPStr pat,
				const AjPStr text, ajuint slen,
				ajuint * const *skipm, ajuint m,
				ajuint k, ajuint begin,
				AjPList l, AjBool amino,
				AjBool carboxyl, const AjPStr name);

ajuint          embPatVariablePattern (const AjPStr pattern,
				       const AjPStr text,
				       const AjPStr patname, AjPList l,
				       ajuint mode,
				       ajuint mismatch, ajuint begin);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

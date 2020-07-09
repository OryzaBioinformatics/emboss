#ifdef __cplusplus
extern "C" {
#endif

#ifndef hsp_regcomp_h
#define hsp_regcomp_h

/* === regcomp.c === */

static void p_ere(register struct parse *p, ajint stop);
static void p_ere_exp(register struct parse *p);
static void p_str(register struct parse *p);
static void p_bre(register struct parse *p, register ajint end1,
		  register ajint end2);
static ajint p_simp_re(register struct parse *p, ajint starordinary);
static ajint p_count(register struct parse *p);
static void p_bracket(register struct parse *p);
static void p_b_term(register struct parse *p, register cset *cs);
static void p_b_cclass(register struct parse *p, register cset *cs);
static void p_b_eclass(register struct parse *p, register cset *cs);
static char p_b_symbol(register struct parse *p);
static char p_b_coll_elem(register struct parse *p, ajint endc);
static char othercase(ajint ch);
static void bothcases(register struct parse *p, ajint ch);
static void ordinary(register struct parse *p, register ajint ch);
static void nonnewline(register struct parse *p);
static void repeat(register struct parse *p, sopno start, ajint from, ajint to);
static ajint seterr(register struct parse *p, ajint e);
static cset *allocset(register struct parse *p);
static void freeset(register struct parse *p, register cset *cs);
static ajint freezeset(register struct parse *p, register cset *cs);
static ajint firstch(register struct parse *p, register cset *cs);
static ajint nch(register struct parse *p, register cset *cs);
static void mcadd(register struct parse *p, register cset *cs,
		  register char *cp);
static void mcsub(register cset *cs, register char *cp);
static ajint mcin(register cset *cs, register char *cp);
static char *mcfind(register cset *cs, register char *cp);
static void mcinvert(register struct parse *p, register cset *cs);
static void mccase(register struct parse *p, register cset *cs);
static ajint isinsets(register REGUTSSTRUCT *g, ajint c);
static ajint samesets(register REGUTSSTRUCT *g, ajint c1, ajint c2);
static void categorize(struct parse *p, register REGUTSSTRUCT *g);
static sopno dupl(register struct parse *p, sopno start, sopno finish);
static void doemit(register struct parse *p, sop op, size_t opnd);
static void doinsert(register struct parse *p, sop op, size_t opnd, sopno pos);
static void dofwd(register struct parse *p, sopno pos, sop value);
static void enlarge(register struct parse *p, sopno size);
static void stripsnug(register struct parse *p, register REGUTSSTRUCT *g);
static void findmust(register struct parse *p, register REGUTSSTRUCT *g);
static sopno pluscount(register struct parse *p, register REGUTSSTRUCT *g);

#endif

#ifdef __cplusplus
}
#endif

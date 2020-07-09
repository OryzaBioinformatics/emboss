#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "hsp_regex.h"

#include "hsp_utils.h"
#include "hsp_regex2.h"

#include "hsp_cclass.h"
#include "hsp_cname.h"

/*
 * parse structure, passed up and down to avoid global variables and
 * other clumsinesses
 */
struct parse {
	char *next;		/* next character in RE */
	char *end;		/* end of string (-> NUL normally) */
	ajint error;		/* has an error been seen? */
	sop *strip;		/* malloced strip */
	sopno ssize;		/* malloced strip size (allocated) */
	sopno slen;		/* malloced strip length (used) */
	ajint ncsalloc;		/* number of csets allocated */
	REGUTSSTRUCT *g;
#	define	NPAREN	10	/* we need to remember () 1-9 for back refs */
	sopno pbegin[NPAREN];	/* -> ( ([0] unused) */
	sopno pend[NPAREN];	/* -> ) ([0] unused) */
};

/* === regcomp.ih === */
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

/* ========= end regcomp.ih ========= */


static char nuls[10];		/* place to point scanner in event of error */

/*
 * macros for use with parse structure
 * BEWARE:  these know that the parse structure is named `p' !!!
 */
#define	PEEK()	(*p->next)
#define	PEEK2()	(*(p->next+1))
#define	MORE()	(p->next < p->end)
#define	MORE2()	(p->next+1 < p->end)
#define	SEE(c)	(MORE() && PEEK() == (c))
#define	SEETWO(a, b)	(MORE() && MORE2() && PEEK() == (a) && PEEK2() == (b))
#define	EAT(c)	((SEE(c)) ? (NEXT(), 1) : 0)
#define	EATTWO(a, b)	((SEETWO(a, b)) ? (NEXT2(), 1) : 0)
#define	NEXT()	(p->next++)
#define	NEXT2()	(p->next += 2)
#define	NEXTn(n)	(p->next += (n))
#define	GETNEXT()	(*p->next++)
#define	SETERROR(e)	seterr(p, (e))
#define	REQUIRE(co, e)	((co) || SETERROR(e))
#define	MUSTSEE(c, e)	(REQUIRE(MORE() && PEEK() == (c), e))
#define	MUSTEAT(c, e)	(REQUIRE(MORE() && GETNEXT() == (c), e))
#define	MUSTNOTSEE(c, e)	(REQUIRE(!MORE() || PEEK() != (c), e))
#define	EMIT(op, sopnd)	doemit(p, (sop)(op), (size_t)(sopnd))
#define	INSERT(op, pos)	doinsert(p, (sop)(op), HERE()-(pos)+1, pos)
#define	AHEAD(pos)		dofwd(p, pos, HERE()-(pos))
#define	ASTERN(sop, pos)	EMIT(sop, HERE()-pos)
#define	HERE()		(p->slen)
#define	THERE()		(p->slen - 1)
#define	THERETHERE()	(p->slen - 2)
#define	DROP(n)	(p->slen -= (n))

#ifndef NDEBUG
static ajint never = 0;		/* for use in asserts; shuts lint up */
#else
#define	never	0		/* some <assert.h>s have bugs too */
#endif

/* @func hsp_regcomp **********************************************************
**
** interface for parser and compilation
**
** @param [?] preg [regex_t*] Undocumented
** @param [?] pattern [const char*] Undocumented
** @param [?] cflags [ajint] Undocumented
** @return [ajint] 0 success, otherwise REG_something
** @@
******************************************************************************/

ajint hsp_regcomp(regex_t *preg, const char *pattern, ajint cflags)
{
    struct parse pa;
    register REGUTSSTRUCT *g;
    register struct parse *p = &pa;
    register ajint i;
    register size_t len;
#ifdef REDEBUG
#	define	GOODFLAGS(f)	(f)
#else
#	define	GOODFLAGS(f)	((f)&~REG_DUMP)
#endif

    cflags = GOODFLAGS(cflags);
    if ((cflags&REG_EXTENDED) && (cflags&REG_NOSPEC))
	return(REG_INVARG);

    if (cflags&REG_PEND)
    {
	if (preg->re_endp < pattern)
	    return(REG_INVARG);
	len = preg->re_endp - pattern;
    }
    else
	len = strlen((char *)pattern);

    /* do the mallocs early so failure handling is easy */
    g = (REGUTSSTRUCT *)malloc(sizeof(REGUTSSTRUCT) +
			       (NC-1)*sizeof(cat_t));
    if (g == NULL)
	return(REG_ESPACE);
    p->ssize = len/(size_t)2*(size_t)3 + (size_t)1; /* ugh */
    p->strip = (sop *)malloc(p->ssize * sizeof(sop));
    p->slen = 0;
    if (p->strip == NULL)
    {
	free((char *)g);
	return(REG_ESPACE);
    }

    /* set things up */
    p->g = g;
    p->next = (char *)pattern;		/* convenience; we do not modify it */
    p->end = p->next + len;
    p->error = 0;
    p->ncsalloc = 0;
    for (i = 0; i < NPAREN; i++)
    {
	p->pbegin[i] = 0;
	p->pend[i] = 0;
    }
    g->csetsize = NC;
    g->sets = NULL;
    g->setbits = NULL;
    g->ncsets = 0;
    g->cflags = cflags;
    g->iflags = 0;
    g->nbol = 0;
    g->neol = 0;
    g->must = NULL;
    g->mlen = 0;
    g->nsub = 0;
    g->ncategories = 1;			/* category 0 is "everything else" */
    g->categories = &g->catspace[-(CHAR_MIN)];
    (void) memset((char *)g->catspace, 0, NC*sizeof(cat_t));
    g->backrefs = 0;

    /* do it */
    EMIT(OEND, 0);
    g->firststate = THERE();
    if (cflags&REG_EXTENDED)
	p_ere(p, OUT);
    else if (cflags&REG_NOSPEC)
	p_str(p);
    else
	p_bre(p, OUT, OUT);
    EMIT(OEND, 0);
    g->laststate = THERE();

    /* tidy up loose ends and fill things in */
    categorize(p, g);
    stripsnug(p, g);
    findmust(p, g);
    g->nplus = pluscount(p, g);
    g->magic = MAGIC2;
    preg->re_nsub = g->nsub;
    preg->re_g = g;
    preg->re_magic = MAGIC1;
#ifndef REDEBUG
    /* not debugging, so can't rely on the assert() in regexec() */
    if (g->iflags&BAD) (void) SETERROR(REG_ASSERT);
#endif

    /* win or lose, we're done */
    if (p->error != 0)			/* lose */
	hsp_regfree(preg);
    return(p->error);
}




/* @funcstatic p_ere *********************************************************
**
** ERE parser top level, concatenation and alternation
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] stop [ajint] the character this ERE should end at
** @return [void]
** @@
******************************************************************************/

static void p_ere(register struct parse *p, ajint stop)
{
    register char c;
    register sopno prevback=0;
    register sopno prevfwd=0;
    register sopno conc;
    register ajint first = 1;		/* is this the first alternative? */

    for (;;)
    {
	/* do a bunch of concatenated expressions */
	conc = HERE();
	while (MORE() && (c = PEEK()) != '|' && c != stop)
	    p_ere_exp(p);
	(void)REQUIRE(HERE() != conc, REG_EMPTY); /* require nonempty */

	if (!EAT('|'))
	    break;			/* NOTE BREAK OUT */

	if (first)
	{
	    INSERT(OCH_, conc);		/* offset is wrong */
	    prevfwd = conc;
	    prevback = conc;
	    first = 0;
	}
	ASTERN(OOR1, prevback);
	prevback = THERE();
	AHEAD(prevfwd);			/* fix previous offset */
	prevfwd = HERE();
	EMIT(OOR2, 0);			/* offset is very wrong */
    }

    if (!first)
    {					/* tail-end fixups */
	AHEAD(prevfwd);
	ASTERN(O_CH, prevback);
    }

    assert(!MORE() || SEE(stop));
}





/* @funcstatic  p_ere_exp *****************************************************
**
** parse one subERE, an atom possibly followed by a repetition op
**
** @param [?] p [register struct parse*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void p_ere_exp(register struct parse *p)
{
    register char c;
    register sopno pos;
    register ajint count;
    register ajint count2;
    register sopno subno;
    ajint wascaret = 0;

    assert(MORE());	/* caller should have ensured this */
    c = GETNEXT();

    pos = HERE();
    switch (c)
    {
    case '(':
	(void)REQUIRE(MORE(), REG_EPAREN);
	p->g->nsub++;
	subno = p->g->nsub;
	if (subno < NPAREN)
	    p->pbegin[subno] = HERE();
	EMIT(OLPAREN, subno);
	if (!SEE(')'))
	    p_ere(p, ')');
	if (subno < NPAREN)
	{
	    p->pend[subno] = HERE();
	    assert(p->pend[subno] != 0);
	}
	EMIT(ORPAREN, subno);
	(void)MUSTEAT(')', REG_EPAREN);
	break;
#ifndef POSIX_MISTAKE
    case ')':		/* happens only if no current unmatched ( */
	/*
	 * You may ask, why the ifndef?  Because I didn't notice
	 * this until slightly too late for 1003.2, and none of the
	 * other 1003.2 regular-expression reviewers noticed it at
	 * all.  So an unmatched ) is legal POSIX, at least until
	 * we can get it fixed.
	 */
	(void) SETERROR(REG_EPAREN);
	break;
#endif
    case '^':
	EMIT(OBOL, 0);
	p->g->iflags |= USEBOL;
	p->g->nbol++;
	wascaret = 1;
	break;
    case '$':
	EMIT(OEOL, 0);
	p->g->iflags |= USEEOL;
	p->g->neol++;
	break;
    case '|':
	(void) SETERROR(REG_EMPTY);
	break;
    case '*':
    case '+':
    case '?':
	(void) SETERROR(REG_BADRPT);
	break;
    case '.':
	if (p->g->cflags&REG_NEWLINE)
	    nonnewline(p);
	else
	    EMIT(OANY, 0);
	break;
    case '[':
	p_bracket(p);
	break;
    case '\\':
	(void)REQUIRE(MORE(), REG_EESCAPE);
	c = GETNEXT();
	ordinary(p, c);
	break;
    case '{':		/* okay as ordinary except if digit follows */
	(void)REQUIRE(!MORE() || !isdigit((ajint)PEEK()), REG_BADRPT);
	/* FALLTHROUGH */
    default:
	ordinary(p, c);
	break;
    }

    if (!MORE())
	return;
    c = PEEK();
    /* we call { a repetition if followed by a digit */
    if (!( c == '*' || c == '+' || c == '?' ||
	  (c == '{' && MORE2() && isdigit((ajint)PEEK2())) ))
	return;				/* no repetition, we're done */
    NEXT();

    (void)REQUIRE(!wascaret, REG_BADRPT);
    switch (c)
    {
    case '*':				/* implemented as +? */
	/* this case does not require the (y|) trick, noKLUDGE */
	INSERT(OPLUS_, pos);
	ASTERN(O_PLUS, pos);
	INSERT(OQUEST_, pos);
	ASTERN(O_QUEST, pos);
	break;
    case '+':
	INSERT(OPLUS_, pos);
	ASTERN(O_PLUS, pos);
	break;
    case '?':
	/* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
	INSERT(OCH_, pos);		/* offset slightly wrong */
	ASTERN(OOR1, pos);		/* this one's right */
	AHEAD(pos);			/* fix the OCH_ */
	EMIT(OOR2, 0);			/* offset very wrong... */
	AHEAD(THERE());			/* ...so fix it */
	ASTERN(O_CH, THERETHERE());
	break;
    case '{':
	count = p_count(p);
	if (EAT(',')) {
	    if (isdigit((ajint)PEEK()))
	    {
		count2 = p_count(p);
		(void)REQUIRE(count <= count2, REG_BADBR);
	    }
	    else			/* single number with comma */
		count2 = INFINITY;
	}
	else				/* just a single number */
	    count2 = count;
	repeat(p, pos, count, count2);
	if (!EAT('}'))
	{				/* error heuristics */
	    while (MORE() && PEEK() != '}')
		NEXT();
	    (void)REQUIRE(MORE(), REG_EBRACE);
	    (void) SETERROR(REG_BADBR);
	}
	break;
    }

    if (!MORE())
	return;
    c = PEEK();
    if (!( c == '*' || c == '+' || c == '?' ||
	  (c == '{' && MORE2() && isdigit((ajint)PEEK2())) ) )
	return;
    (void) SETERROR(REG_BADRPT);
}





/* @funcstatic  p_str *********************************************************
**
** string (no metacharacters) "parser"
**
** @param [?] p [register struct parse*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void p_str(register struct parse *p)
{
    (void)REQUIRE(MORE(), REG_EMPTY);
    while (MORE())
	ordinary(p, GETNEXT());
}





/* @funcstatic  p_bre *********************************************************
**
** BRE parser top level, anchoring and concatenation
**
** This implementation is a bit of a kludge, in that a trailing $ is first
** taken as an ordinary character and then revised to be an anchor.  The
** only undesirable side effect is that '$' gets included as a character
** category in such cases.  This is fairly harmless; not worth fixing.
** The amount of lookahead needed to avoid this kludge is excessive.
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] end1 [register ajint] first terminating character
** @param [?] end2 [register ajint] second terminating character
** @return [void]
** @@
******************************************************************************/

static void p_bre( register struct parse *p, register ajint end1,
		  register ajint end2)
{
    register sopno start = HERE();
    register ajint first = 1;		/* first subexpression? */
    register ajint wasdollar = 0;

    if (EAT('^'))
    {
	EMIT(OBOL, 0);
	p->g->iflags |= USEBOL;
	p->g->nbol++;
    }
    while (MORE() && !SEETWO(end1, end2))
    {
	wasdollar = p_simp_re(p, first);
	first = 0;
    }
    if (wasdollar)
    {					/* oops, that was a trailing anchor */
	DROP(1);
	EMIT(OEOL, 0);
	p->g->iflags |= USEEOL;
	p->g->neol++;
    }

    (void)REQUIRE(HERE() != start, REG_EMPTY); /* require nonempty */
}





/* @funcstatic  p_simp_re *****************************************************
**
** parse a simple RE, an atom possibly followed by a repetition
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] starordinary [ajint] is a leading * an ordinary character?
** @return [ajint] was the simple RE an unbackslashed $? 
** @@
******************************************************************************/

static ajint p_simp_re(register struct parse *p, ajint starordinary)
{
    register ajint c;
    register ajint count;
    register ajint count2;
    register sopno pos;
    register ajint i;
    register sopno subno;
#	define	BACKSL	(1<<CHAR_BIT)

    pos = HERE();			/* repetion op, if any, covers from here */

    assert(MORE());			/* caller should have ensured this */
    c = GETNEXT();
    if (c == '\\')
    {
	(void)REQUIRE(MORE(), REG_EESCAPE);
	c = BACKSL | (unsigned char)GETNEXT();
    }
    switch (c)
    {
    case '.':
	if (p->g->cflags&REG_NEWLINE)
	    nonnewline(p);
	else
	    EMIT(OANY, 0);
	break;
    case '[':
	p_bracket(p);
	break;
    case BACKSL|'{':
	(void) SETERROR(REG_BADRPT);
	break;
    case BACKSL|'(':
	p->g->nsub++;
	subno = p->g->nsub;
	if (subno < NPAREN)
	    p->pbegin[subno] = HERE();
	EMIT(OLPAREN, subno);
	/* the MORE here is an error heuristic */
	if (MORE() && !SEETWO('\\', ')'))
	    p_bre(p, '\\', ')');
	if (subno < NPAREN)
	{
	    p->pend[subno] = HERE();
	    assert(p->pend[subno] != 0);
	}
	EMIT(ORPAREN, subno);
	(void)REQUIRE(EATTWO('\\', ')'), REG_EPAREN);
	break;
    case BACKSL|')':			/* should not get here -- must be user */
    case BACKSL|'}':
	(void) SETERROR(REG_EPAREN);
	break;
    case BACKSL|'1':
    case BACKSL|'2':
    case BACKSL|'3':
    case BACKSL|'4':
    case BACKSL|'5':
    case BACKSL|'6':
    case BACKSL|'7':
    case BACKSL|'8':
    case BACKSL|'9':
	i = (c&~BACKSL) - '0';
	assert(i < NPAREN);
	if (p->pend[i] != 0)
	{
	    assert(i <= p->g->nsub);
	    EMIT(OBACK_, i);
	    assert(p->pbegin[i] != 0);
	    assert(OP(p->strip[p->pbegin[i]]) == OLPAREN);
	    assert(OP(p->strip[p->pend[i]]) == ORPAREN);
	    (void) dupl(p, p->pbegin[i]+1, p->pend[i]);
	    EMIT(O_BACK, i);
	}
	else
	    (void) SETERROR(REG_ESUBREG);
	p->g->backrefs = 1;
	break;
    case '*':
	(void)REQUIRE(starordinary, REG_BADRPT);
	/* FALLTHROUGH */
    default:
	ordinary(p, c &~ BACKSL);
	break;
    }

    if (EAT('*'))
    {					/* implemented as +? */
	/* this case does not require the (y|) trick, noKLUDGE */
	INSERT(OPLUS_, pos);
	ASTERN(O_PLUS, pos);
	INSERT(OQUEST_, pos);
	ASTERN(O_QUEST, pos);
    }
    else if (EATTWO('\\', '{'))
    {
	count = p_count(p);
	if (EAT(','))
	{
	    if (MORE() && isdigit((ajint)PEEK()))
	    {
		count2 = p_count(p);
		(void)REQUIRE(count <= count2, REG_BADBR);
	    }
	    else			/* single number with comma */
		count2 = INFINITY;
	}
	else				/* just a single number */
	    count2 = count;
	repeat(p, pos, count, count2);
	if (!EATTWO('\\', '}'))
	{				/* error heuristics */
	    while (MORE() && !SEETWO('\\', '}'))
		NEXT();
	    (void)REQUIRE(MORE(), REG_EBRACE);
	    (void) SETERROR(REG_BADBR);
	}
    }
    else if (c == (unsigned char)'$')	/* $ (but not \$) ends it */
	return(1);

    return(0);
}





/* @funcstatic  p_count *******************************************************
**
** parse a repetition count
**
** @param [?] p [register struct parse*] Undocumented
** @return [ajint] the value
** @@
******************************************************************************/

static ajint p_count(register struct parse *p)
{
    register ajint count = 0;
    register ajint ndigits = 0;

    while (MORE() && isdigit((ajint)PEEK()) && count <= DUPMAX)
    {
	count = count*10 + (GETNEXT() - '0');
	ndigits++;
    }

    (void)REQUIRE(ndigits > 0 && count <= DUPMAX, REG_BADBR);
    return(count);
}





/* @funcstatic  p_bracket *****************************************************
**
** parse a bracketed character list
**
** Note a significant property of this code:  if the allocset() did SETERROR,
** no set operations are done.
**
** @param [?] p [register struct parse*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void p_bracket(register struct parse *p)
{
    register cset *cs = allocset(p);
    register ajint invert = 0;

    /* Dept of Truly Sickening Special-Case Kludges */
    if (p->next + 5 < p->end && strncmp(p->next, "[:<:]]", 6) == 0)
    {
	EMIT(OBOW, 0);
	NEXTn(6);
	return;
    }
    if (p->next + 5 < p->end && strncmp(p->next, "[:>:]]", 6) == 0)
    {
	EMIT(OEOW, 0);
	NEXTn(6);
	return;
    }

    if (EAT('^'))
	invert++;			/* make note to invert set at end */
    if (EAT(']'))
	CHadd(cs, ']');
    else if (EAT('-'))
	CHadd(cs, '-');
    while (MORE() && PEEK() != ']' && !SEETWO('-', ']'))
	p_b_term(p, cs);
    if (EAT('-'))
	CHadd(cs, '-');
    (void)MUSTEAT(']', REG_EBRACK);

    if (p->error != 0)			/* don't mess things up further */
	return;

    if (p->g->cflags&REG_ICASE)
    {
	register ajint i;
	register ajint ci;

	for (i = p->g->csetsize - 1; i >= 0; i--)
	    if (CHIN(cs, i) && isalpha(i))
	    {
		ci = othercase(i);
		if (ci != i)
		    CHadd(cs, ci);
	    }
	if (cs->multis != NULL)
	    mccase(p, cs);
    }
    if (invert)
    {
	register ajint i;

	for (i = p->g->csetsize - 1; i >= 0; i--)
	    if (CHIN(cs, i))
		CHsub(cs, i);
	    else
		CHadd(cs, i);
	if (p->g->cflags&REG_NEWLINE)
	    CHsub(cs, '\n');
	if (cs->multis != NULL)
	    mcinvert(p, cs);
    }

    assert(cs->multis == NULL);		/* xxx */

    if (nch(p, cs) == 1)
    {					/* optimize singleton sets */
	ordinary(p, firstch(p, cs));
	freeset(p, cs);
    }
    else
	EMIT(OANYOF, freezeset(p, cs));
}





/* @funcstatic  p_b_term ******************************************************
**
** parse one term of a bracketed character list
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void p_b_term(register struct parse *p, register cset *cs)
{
    register char c;
    register char start, finish;
    register ajint i;

    /* classify what we've got */
    switch ((MORE()) ? PEEK() : '\0')
    {
    case '[':
	c = (MORE2()) ? PEEK2() : '\0';
	break;
    case '-':
	(void) SETERROR(REG_ERANGE);
	return;				/* NOTE RETURN */
/*	break;*/
    default:
	c = '\0';
	break;
    }

    switch (c) {
    case ':':				/* character class */
	NEXT2();
	(void)REQUIRE(MORE(), REG_EBRACK);
	c = PEEK();
	(void)REQUIRE(c != '-' && c != ']', REG_ECTYPE);
	p_b_cclass(p, cs);
	(void)REQUIRE(MORE(), REG_EBRACK);
	(void)REQUIRE(EATTWO(':', ']'), REG_ECTYPE);
	break;
    case '=':				/* equivalence class */
	NEXT2();
	(void)REQUIRE(MORE(), REG_EBRACK);
	c = PEEK();
	(void)REQUIRE(c != '-' && c != ']', REG_ECOLLATE);
	p_b_eclass(p, cs);
	(void)REQUIRE(MORE(), REG_EBRACK);
	(void)REQUIRE(EATTWO('=', ']'), REG_ECOLLATE);
	break;
    default:	/* symbol, ordinary character, or range */
	        /* xxx revision needed for multichar stuff */
	start = p_b_symbol(p);
	if (SEE('-') && MORE2() && PEEK2() != ']')
	{
	    /* range */
	    NEXT();
	    if (EAT('-'))
		finish = '-';
	    else
		finish = p_b_symbol(p);
	}
	else
	    finish = start;
	/* xxx what about signed chars here... */
	(void)REQUIRE(start <= finish, REG_ERANGE);
	for (i = start; i <= finish; i++)
	    CHadd(cs, i);
	break;
    }
}





/* @funcstatic  p_b_cclass ****************************************************
**
** parse a character-class name and deal with it
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void p_b_cclass(register struct parse *p, register cset *cs)
{
    register char *sp = p->next;
    register struct cclass *cp;
    register size_t len;
    register char *u;
    register char c;

    while (MORE() && isalpha((ajint)PEEK()))
	NEXT();
    len = p->next - sp;
    for (cp = cclasses; cp->name != NULL; cp++)
	if (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
	    break;
    if (cp->name == NULL)
    {
	/* oops, didn't find it */
	(void) SETERROR(REG_ECTYPE);
	return;
    }

    u = cp->chars;
    while ((c = *u++) != '\0')
	CHadd(cs, c);
    for (u = cp->multis; *u != '\0'; u += strlen(u) + 1)
	MCadd(p, cs, u);
}





/* @funcstatic  p_b_eclass ****************************************************
**
** parse an equivalence-class name and deal with it
**
** This implementation is incomplete. xxx
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void p_b_eclass(register struct parse *p, register cset *cs)
{
    register char c;

    c = p_b_coll_elem(p, '=');
    CHadd(cs, c);
}





/* @funcstatic  p_b_symbol ****************************************************
**
** parse a character or [..]ed multicharacter collating symbol
**
** @param [?] p [register struct parse*] Undocumented
** @return [char] value of symbol
** @@
******************************************************************************/

static char p_b_symbol(register struct parse *p)
{
    register char value;

    (void)REQUIRE(MORE(), REG_EBRACK);
    if (!EATTWO('[', '.'))
	return(GETNEXT());

    /* collating symbol */
    value = p_b_coll_elem(p, '.');
    (void)REQUIRE(EATTWO('.', ']'), REG_ECOLLATE);
    return(value);
}





/* @funcstatic  p_b_coll_elem *************************************************
**
** parse a collating-element name and look it up
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] endc [ajint] name ended by endc,']'
** @return [char] value of collating element 
** @@
******************************************************************************/

static char p_b_coll_elem(register struct parse *p, ajint endc)
{
    register char *sp = p->next;
    register struct cname *cp;
    register ajint len;

    while (MORE() && !SEETWO(endc, ']'))
	NEXT();
    if (!MORE())
    {
	(void) SETERROR(REG_EBRACK);
	return(0);
    }
    len = p->next - sp;
    for (cp = cnames; cp->name != NULL; cp++)
	if (strncmp(cp->name, sp, len) == 0 && cp->name[len] == '\0')
	    return(cp->code);		/* known name */
    if (len == 1)
	return(*sp);			/* single character */
    (void) SETERROR(REG_ECOLLATE);	/* neither */
    return(0);
}





/* @funcstatic  othercase *****************************************************
**
** return the case counterpart of an alphabetic
**
** @param [?] ch [ajint] Undocumented
** @return [char] if no counterpart, return ch
** @@
******************************************************************************/

static char othercase(ajint ch)
{
    assert(isalpha(ch));
    if (isupper(ch))
	return(tolower(ch));
    else if (islower(ch))
	return(toupper(ch));
    else				/* peculiar, but could happen */
	return(ch);
}





/* @funcstatic  bothcases *****************************************************
**
** emit a dualcase version of a two-case character
**
** Boy, is this implementation ever a kludge...
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] ch [ajint] Undocumented
** @return [void]
** @@
******************************************************************************/

static void bothcases(register struct parse *p, ajint ch)
{
    register char *oldnext = p->next;
    register char *oldend = p->end;
    char bracket[3];

    assert(othercase(ch) != ch);	/* p_bracket() would recurse */
    p->next = bracket;
    p->end = bracket+2;
    bracket[0] = ch;
    bracket[1] = ']';
    bracket[2] = '\0';
    p_bracket(p);
    assert(p->next == bracket+2);
    p->next = oldnext;
    p->end = oldend;
}





/* @funcstatic  ordinary ******************************************************
**
** emit an ordinary character
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] ch [register ajint] Undocumented
** @return [void]
** @@
******************************************************************************/

static void ordinary(register struct parse *p, register ajint ch)
{
    register cat_t *cap = p->g->categories;

    if ((p->g->cflags&REG_ICASE) && isalpha(ch) && othercase(ch) != ch)
	bothcases(p, ch);
    else
    {
	EMIT(OCHAR, (unsigned char)ch);
	if (cap[ch] == 0)
	    cap[ch] = p->g->ncategories++;
    }
}





/* @funcstatic  nonnewline ****************************************************
**
** emit REG_NEWLINE version of OANY
**
** Boy, is this implementation ever a kludge...
**
** @param [?] p [register struct parse*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void nonnewline(register struct parse *p)
{
    register char *oldnext = p->next;
    register char *oldend = p->end;
    char bracket[4];

    p->next = bracket;
    p->end = bracket+3;
    bracket[0] = '^';
    bracket[1] = '\n';
    bracket[2] = ']';
    bracket[3] = '\0';
    p_bracket(p);
    assert(p->next == bracket+3);
    p->next = oldnext;
    p->end = oldend;
}





/* @funcstatic  repeat ********************************************************
**
** generate code for a bounded repetition, recursively if needed
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] start [sopno] operand from here to end of strip 
** @param [?] from [ajint] repeated from this number 
** @param [?] to [ajint] to this number of times (maybe INFINITY)
** @return [void]
** @@
******************************************************************************/

static void repeat( register struct parse *p, sopno start, ajint from, ajint to)
{
    register sopno finish = HERE();
#	define	N	2
#	define	INF	3
#	define	REP(f, t)	((f)*8 + (t))
#	define	MAP(n)	(((n) <= 1) ? (n) : ((n) == INFINITY) ? INF : N)
    register sopno copy;

    if (p->error != 0)			/* head off possible runaway recursion */
	return;

    assert(from <= to);

    switch (REP(MAP(from), MAP(to)))
    {
    case REP(0, 0):			/* must be user doing this */
	DROP(finish-start);		/* drop the operand */
	break;
    case REP(0, 1):			/* as x{1,1}? */
    case REP(0, N):			/* as x{1,n}? */
    case REP(0, INF):			/* as x{1,}? */
	/* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
	INSERT(OCH_, start);		/* offset is wrong... */
	repeat(p, start+1, 1, to);
	ASTERN(OOR1, start);
	AHEAD(start);			/* ... fix it */
	EMIT(OOR2, 0);
	AHEAD(THERE());
	ASTERN(O_CH, THERETHERE());
	break;
    case REP(1, 1):			/* trivial case */
	/* done */
	break;
    case REP(1, N):			/* as x?x{1,n-1} */
	/* KLUDGE: emit y? as (y|) until subtle bug gets fixed */
	INSERT(OCH_, start);
	ASTERN(OOR1, start);
	AHEAD(start);
	EMIT(OOR2, 0);			/* offset very wrong... */
	AHEAD(THERE());			/* ...so fix it */
	ASTERN(O_CH, THERETHERE());
	copy = dupl(p, start+1, finish+1);
	assert(copy == finish+4);
	repeat(p, copy, 1, to-1);
	break;
    case REP(1, INF):			/* as x+ */
	INSERT(OPLUS_, start);
	ASTERN(O_PLUS, start);
	break;
    case REP(N, N):			/* as xx{m-1,n-1} */
	copy = dupl(p, start, finish);
	repeat(p, copy, from-1, to-1);
	break;
    case REP(N, INF):			/* as xx{n-1,INF} */
	copy = dupl(p, start, finish);
	repeat(p, copy, from-1, to);
	break;
    default:				/* "can't happen" */
	(void) SETERROR(REG_ASSERT);	/* just in case */
	break;
    }
}





/* @funcstatic  seterr ********************************************************
**
** set an error condition
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] e [ajint] Undocumented
** @return [ajint] useless but makes type checking happy
** @@
******************************************************************************/

static ajint seterr(register struct parse *p, ajint e)
{
    if (p->error == 0)		/* keep earliest error condition */
	p->error = e;
    p->next = nuls;		/* try to bring things to a halt */
    p->end = nuls;
    return(0);			/* make the return value well-defined */
}





/* @funcstatic  allocset ******************************************************
**
** allocate a set of characters for []
**
** @param [?] p [register struct parse*] Undocumented
** @return [cset*] Undocumented
** @@
******************************************************************************/

static cset* allocset(register struct parse *p)
{
    register ajint no = p->g->ncsets++;
    register size_t nc;
    register size_t nbytes;
    register cset *cs;
    register size_t css = (size_t)p->g->csetsize;
    register ajint i;

    if (no >= p->ncsalloc)
    {					/* need another column of space */
	p->ncsalloc += CHAR_BIT;
	nc = p->ncsalloc;
	assert(nc % CHAR_BIT == 0);
	nbytes = nc / CHAR_BIT * css;
	if (p->g->sets == NULL)
	    p->g->sets = (cset *)malloc(nc * sizeof(cset));
	else
	    p->g->sets = (cset *)realloc((char *)p->g->sets,
					 nc * sizeof(cset));
	if (p->g->setbits == NULL)
	    p->g->setbits = (UCH *)malloc(nbytes);
	else
	{
	    p->g->setbits = (UCH *)realloc((char *)p->g->setbits,
					   nbytes);
	    /* xxx this isn't right if setbits is now NULL */
	    for (i = 0; i < no; i++)
		p->g->sets[i].ptr = p->g->setbits + css*(i/CHAR_BIT);
	}
	if (p->g->sets != NULL && p->g->setbits != NULL)
	    (void) memset((char *)p->g->setbits + (nbytes - css),
			  0, css);
	else
	{
	    no = 0;
	    (void) SETERROR(REG_ESPACE);
	    /* caller's responsibility not to do set ops */
	}
    }

    assert(p->g->sets != NULL);		/* xxx */
    cs = &p->g->sets[no];
    cs->ptr = p->g->setbits + css*((no)/CHAR_BIT);
    cs->mask = 1 << ((no) % CHAR_BIT);
    cs->hash = 0;
    cs->smultis = 0;
    cs->multis = NULL;

    return(cs);
}





/* @funcstatic  freeset *******************************************************
**
** free a now-unused set
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void freeset(register struct parse *p, register cset *cs)
{
    register ajint i;
    register cset *top = &p->g->sets[p->g->ncsets];
    register size_t css = (size_t)p->g->csetsize;

    for (i = 0; i < css; i++)
	CHsub(cs, i);
    if (cs == top-1)			/* recover only the easy case */
	p->g->ncsets--;
}





/* @funcstatic  freezeset *****************************************************
**
** final processing on a set of characters
**
** The main task here is merging identical sets.  This is usually a waste
** of time (although the hash code minimizes the overhead), but can win
** big if REG_ICASE is being used.  REG_ICASE, by the way, is why the hash
** is done using addition rather than xor -- all ASCII [aA] sets xor to
** the same value!
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [ajint] set number
** @@
******************************************************************************/

static ajint freezeset( register struct parse *p, register cset *cs)
{
    register UCH h = cs->hash;
    register ajint i;
    register cset *top = &p->g->sets[p->g->ncsets];
    register cset *cs2;
    register size_t css = (size_t)p->g->csetsize;

    /* look for an earlier one which is the same */
    for (cs2 = &p->g->sets[0]; cs2 < top; cs2++)
	if (cs2->hash == h && cs2 != cs) 
	{
	    /* maybe */
	    for (i = 0; i < css; i++)
		if (!!CHIN(cs2, i) != !!CHIN(cs, i))
		    break;		/* no */
	    if (i == css)
		break;			/* yes */
	}

    if (cs2 < top)
    {					/* found one */
	freeset(p, cs);
	cs = cs2;
    }

    return((ajint)(cs - p->g->sets));
}





/* @funcstatic  firstch *******************************************************
**
** return first character in a set (which must have at least one)
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [ajint] character; there is no "none" value 
** @@
******************************************************************************/

static ajint firstch(register struct parse *p, register cset *cs)
{
    register ajint i;
    register size_t css = (size_t)p->g->csetsize;

    for (i = 0; i < css; i++)
	if (CHIN(cs, i))
	    return((char)i);
    assert(never);
    return(0);				/* arbitrary */
}





/* @funcstatic  nch ***********************************************************
**
** number of characters in a set
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint nch(register struct parse *p, register cset *cs)
{
    register ajint i;
    register size_t css = (size_t)p->g->csetsize;
    register ajint n = 0;

    for (i = 0; i < css; i++)
	if (CHIN(cs, i))
	    n++;
    return(n);
}





/* @funcstatic  mcadd *********************************************************
**
** add a collating element to a cset
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @param [?] cp [register char*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void mcadd(register struct parse *p,register cset *cs,
		  register char *cp)
{
    register size_t oldend = cs->smultis;

    cs->smultis += strlen(cp) + 1;
    if (cs->multis == NULL)
	cs->multis = malloc(cs->smultis);
    else
	cs->multis = realloc(cs->multis, cs->smultis);
    if (cs->multis == NULL)
    {
	(void) SETERROR(REG_ESPACE);
	return;
    }

    (void) strcpy(cs->multis + oldend - 1, cp);
    cs->multis[cs->smultis - 1] = '\0';
}





/* @funcstatic  mcsub *********************************************************
**
** subtract a collating element from a cset
**
** @param [?] cs [register cset*] Undocumented
** @param [?] cp [register char*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void mcsub(register cset *cs, register char *cp)
{
    register char *fp = mcfind(cs, cp);
    register size_t len = strlen(fp);

    assert(fp != NULL);
    (void) memmove(fp, fp + len + 1,
		   cs->smultis - (fp + len + 1 - cs->multis));
    cs->smultis -= len;

    if (cs->smultis == 0)
    {
	free(cs->multis);
	cs->multis = NULL;
	return;
    }

    cs->multis = realloc(cs->multis, cs->smultis);
    assert(cs->multis != NULL);
}





/* @funcstatic  mcin **********************************************************
**
** is a collating element in a cset?
**
** @param [?] cs [register cset*] Undocumented
** @param [?] cp [register char*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint mcin(register cset *cs, register char *cp)
{
    return(mcfind(cs, cp) != NULL);
}





/* @funcstatic  mcfind ********************************************************
**
** find a collating element in a cset
**
** @param [?] cs [register cset*] Undocumented
** @param [?] cp [register char*] Undocumented
** @return [char*] Undocumented
** @@
******************************************************************************/

static char * mcfind(register cset *cs,register char *cp)
{
    register char *p;

    if (cs->multis == NULL)
	return(NULL);
    for (p = cs->multis; *p != '\0'; p += strlen(p) + 1)
	if (strcmp(cp, p) == 0)
	    return(p);
    return(NULL);
}





/* @funcstatic  mcinvert ******************************************************
**
** invert the list of collating elements in a cset
**
** This would have to know the set of possibilities.  Implementation
** is deferred.
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void mcinvert(register struct parse *p, register cset *cs)
{
    assert(cs->multis == NULL);		/* xxx */
}





/* @funcstatic  mccase ********************************************************
**
** add case counterparts of the list of collating elements in a cset
**
** This would have to know the set of possibilities.  Implementation
** is deferred.
**
* This would have to know the set of possibilities.  Implementation
** @param [?] p [register struct parse*] Undocumented
** @param [?] cs [register cset*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void mccase(register struct parse *p,register cset *cs)
{
    assert(cs->multis == NULL);		/* xxx */
}





/* @funcstatic  isinsets ******************************************************
**
** is this character in any sets?
**
** @param [?] g [register REGUTSSTRUCT*] Undocumented
** @param [?] c [ajint] Undocumented
** @return [ajint] predicate
** @@
******************************************************************************/

static ajint isinsets(register REGUTSSTRUCT *g, ajint c)
{
    register UCH *col;
    register ajint i;
    register ajint ncols = (g->ncsets+(CHAR_BIT-1)) / CHAR_BIT;
    register unsigned uc = (unsigned char)c;

    for (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
	if (col[uc] != 0)
	    return(1);
    return(0);
}





/* @funcstatic  samesets ******************************************************
**
** are these two characters in exactly the same sets?
**
** @param [?] g [register REGUTSSTRUCT*] Undocumented
** @param [?] c1 [ajint] Undocumented
** @param [?] c2 [ajint] Undocumented
** @return [ajint] predicate
** @@
******************************************************************************/

static ajint samesets(register REGUTSSTRUCT *g, ajint c1, ajint c2)
{
    register UCH *col;
    register ajint i;
    register ajint ncols = (g->ncsets+(CHAR_BIT-1)) / CHAR_BIT;
    register unsigned uc1 = (unsigned char)c1;
    register unsigned uc2 = (unsigned char)c2;

    for (i = 0, col = g->setbits; i < ncols; i++, col += g->csetsize)
	if (col[uc1] != col[uc2])
	    return(0);
    return(1);
}





/* @funcstatic  categorize ****************************************************
**
** sort out character categories
**
** @param [?] p [struct parse*] Undocumented
** @param [?] g [register REGUTSSTRUCT*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void categorize(struct parse *p,register REGUTSSTRUCT *g)
{
    register cat_t *cats = g->categories;
    register ajint c;
    register ajint c2;
    register cat_t cat;

    /* avoid making error situations worse */
    if (p->error != 0)
	return;

    for (c = CHAR_MIN; c <= CHAR_MAX; c++)
	if (cats[c] == 0 && isinsets(g, c))
	{
	    cat = g->ncategories++;
	    cats[c] = cat;
	    for (c2 = c+1; c2 <= CHAR_MAX; c2++)
		if (cats[c2] == 0 && samesets(g, c, c2))
		    cats[c2] = cat;
	}
}





/* @funcstatic  dupl **********************************************************
**
** emit a duplicate of a bunch of sops
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] start [sopno] from here 
** @param [?] finish [sopno] to this less one
** @return [sopno] start of duplicate
** @@
******************************************************************************/

static sopno dupl(register struct parse *p,sopno start,sopno finish)
{
    register sopno ret = HERE();
    register sopno len = finish - start;

    assert(finish >= start);
    if (len == 0)
	return(ret);
    enlarge(p, p->ssize + len);		/* this many unexpected additions */
    assert(p->ssize >= p->slen + len);
    (void) memcpy((char *)(p->strip + p->slen),
		  (char *)(p->strip + start), (size_t)len*sizeof(sop));
    p->slen += len;
    return(ret);
}




/* @funcstatic  doemit ********************************************************
**
** emit a strip operator
**
** It might seem better to implement this as a macro with a function as
** hard-case backup, but it's just too big and messy unless there are
** some changes to the data structures.  Maybe later.
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] op [sop] Undocumented
** @param [?] opnd [size_t] Undocumented
** @return [void]
** @@
******************************************************************************/

static void doemit(register struct parse *p,sop op,size_t opnd)
{
    /* avoid making error situations worse */
    if (p->error != 0)
	return;

    /* deal with oversize operands ("can't happen", more or less) */
    assert(opnd < 1<<OPSHIFT);

    /* deal with undersized strip */
    if (p->slen >= p->ssize)
	enlarge(p, (p->ssize+1) / 2 * 3); /* +50% */
    assert(p->slen < p->ssize);

    /* finally, it's all reduced to the easy case */
    p->strip[p->slen++] = SOP(op, opnd);
}





/* @funcstatic  doinsert ******************************************************
**
** insert a sop into the strip
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] op [sop] Undocumented
** @param [?] opnd [size_t] Undocumented
** @param [?] pos [sopno] Undocumented
** @return [void]
** @@
******************************************************************************/

static void doinsert(register struct parse *p,sop op,size_t opnd,sopno pos)
{
    register sopno sn;
    register sop s;
    register ajint i;

    /* avoid making error situations worse */
    if (p->error != 0)
	return;

    sn = HERE();
    EMIT(op, opnd);			/* do checks, ensure space */
    assert(HERE() == sn+1);
    s = p->strip[sn];

    /* adjust paren pointers */
    assert(pos > 0);
    for (i = 1; i < NPAREN; i++) {
	if (p->pbegin[i] >= pos)
	{
	    p->pbegin[i]++;
	}
	if (p->pend[i] >= pos)
	{
	    p->pend[i]++;
	}
    }

    (void) memmove((char *)&p->strip[pos+1], (char *)&p->strip[pos],
		   (HERE()-pos-1)*sizeof(sop));
    p->strip[pos] = s;
}





/* @funcstatic  dofwd *********************************************************
**
** complete a forward reference
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] pos [register sopno] Undocumented
** @param [?] value [sop] Undocumented
** @return [void]
** @@
******************************************************************************/

static void dofwd(register struct parse *p,register sopno pos,sop value)
{
    /* avoid making error situations worse */
    if (p->error != 0)
	return;

    assert(value < 1<<OPSHIFT);
    p->strip[pos] = OP(p->strip[pos]) | value;
}





/* @funcstatic  enlarge *******************************************************
**
** enlarge the strip
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] size [register sopno] Undocumented
** @return [void]
** @@
******************************************************************************/

static void enlarge(register struct parse *p, register sopno size)
{
    register sop *sp;

    if (p->ssize >= size)
	return;

    sp = (sop *)realloc(p->strip, size*sizeof(sop));
    if (sp == NULL) {
	(void) SETERROR(REG_ESPACE);
	return;
    }
    p->strip = sp;
    p->ssize = size;
}





/* @funcstatic stripsnug *****************************************************
**
** compact the strip
**
** @param [?] p [register struct parse*] Undocumented
** @param [?] g [register REGUTSSTRUCT*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void stripsnug(register struct parse *p,register REGUTSSTRUCT *g)
{
    g->nstates = p->slen;
    g->strip = (sop *)realloc((char *)p->strip, p->slen * sizeof(sop));
    if (g->strip == NULL)
    {
	(void) SETERROR(REG_ESPACE);
	g->strip = p->strip;
    }
}





/* @funcstatic findmust ******************************************************
**
** fill in must and mlen with longest mandatory literal string
**
** This algorithm could do fancy things like analyzing the operands of |
** for common subsequences.  Someday.  This code is simple and finds most
** of the interesting cases.
**
** Note that must and mlen got initialized during setup.
**
** @param [?] p [struct parse*] Undocumented
** @param [?] g [register REGUTSSTRUCT*] Undocumented
** @return [void]
** @@
******************************************************************************/

static void findmust(struct parse *p,register REGUTSSTRUCT *g)
{
    register sop *scan;
    sop *start=0;
    register sop *newstart=0;
    register sopno newlen;
    register sop s;
    register char *cp;
    register sopno i;

    /* avoid making error situations worse */
    if (p->error != 0)
	return;

    /* find the longest OCHAR sequence in strip */
    newlen = 0;
    scan = g->strip + 1;
    do
    {
	s = *scan++;
	switch (SWOP(s))
	{
	case AJ_OCHAR:			/* sequence member */
	    if (newlen == 0)		/* new sequence */
		newstart = scan - 1;
	    newlen++;
	    break;
	case AJ_OPLUS_:			/* things that don't break one */
	case AJ_OLPAREN:
	case AJ_ORPAREN:
	    break;
	case AJ_OQUEST_:		/* things that must be skipped */
	case AJ_OCH_:
	    scan--;
	    do
	    {
		scan += OPND(s);
		s = *scan;
		/* assert() interferes w debug printouts */
		if (OP(s) != O_QUEST && OP(s) != O_CH &&
		    OP(s) != OOR2)
		{
		    g->iflags |= BAD;
		    return;
		}
	    } while (OP(s) != O_QUEST && OP(s) != O_CH);
	    /* fallthrough */
	default:			/* things that break a sequence */
	    if (newlen > g->mlen)
	    {				/* ends one */
		start = newstart;
		g->mlen = newlen;
	    }
	    newlen = 0;
	    break;
	}
    } while (OP(s) != OEND);

    if (g->mlen == 0)			/* there isn't one */
	return;

    /* turn it into a character string */
    g->must = malloc((size_t)g->mlen + 1);
    if (g->must == NULL)
    {					/* argh; just forget it */
	g->mlen = 0;
	return;
    }
    cp = g->must;
    scan = start;
    for (i = g->mlen; i > 0; i--)
    {
	while (OP(s = *scan++) != OCHAR)
	    continue;
	assert(cp < g->must + g->mlen);
	*cp++ = (char)OPND(s);
    }
    assert(cp == g->must + g->mlen);
    *cp++ = '\0';			/* just on general principles */
}





/* @funcstatic  pluscount *****************************************************
**
** count + nesting
**
** @param [?] p [struct parse*] Undocumented
** @param [?] g [register REGUTSSTRUCT*] Undocumented
** @return [sopno] nesting depth
** @@
******************************************************************************/

static sopno pluscount(struct parse *p,register REGUTSSTRUCT *g)
{
    register sop *scan;
    register sop s;
    register sopno plusnest = 0;
    register sopno maxnest = 0;

    if (p->error != 0)
	return(0);			/* there may not be an OEND */

    scan = g->strip + 1;
    do
    {
	s = *scan++;
	switch (SWOP(s))
	{
	case AJ_OPLUS_:
	    plusnest++;
	    break;
	case AJ_O_PLUS:
	    if (plusnest > maxnest)
		maxnest = plusnest;
	    plusnest--;
	    break;
	}
    } while (OP(s) != OEND);
    if (plusnest != 0)
	g->iflags |= BAD;
    return(maxnest);
}






/* @func regcompUnused ********************************************************
**
** Unused functions to keep compiler happy
**
** @return [void]
** @@
******************************************************************************/

void regcompUnused(void)
{
    cset ucset;
    
    mcin(&ucset,"");
    mcsub(&ucset,"");
}

#ifdef __cplusplus
extern "C"
{
#endif

/* === engine.c === */

static ajint matcher(register REGUTSSTRUCT *g, char *string, size_t nmatch,
		   regmatch_t pmatch[], ajint eflags);
static char *dissect(register struct match *m, char *start, char *stop,
		     sopno startst, sopno stopst);
static char *backref(register struct match *m, char *start, char *stop,
		     sopno startst, sopno stopst, sopno lev);
static char *fast(register struct match *m, char *start, char *stop,
		  sopno startst, sopno stopst);
static char *slow(register struct match *m, char *start, char *stop,
		  sopno startst, sopno stopst);
static states step(register REGUTSSTRUCT *g, sopno start, sopno stop,
		   register states bef, ajint ch, register states aft);


#define	BOL	(OUT+1)
#define	EOL	(BOL+1)
#define	BOLEOL	(BOL+2)
#define	NOTHING	(BOL+3)
#define	BOW	(BOL+4)
#define	EOW	(BOL+5)
#define	CODEMAX	(BOL+5)		/* highest code used */
#define	NONCHAR(c)	((c) > CHAR_MAX)
#define	NNONCHAR	(CODEMAX-CHAR_MAX)

#ifdef REDEBUG
static void print(struct match *m, char *caption, states st, ajint ch, FILE *d);
#endif
#ifdef REDEBUG
static void at(struct match *m, char *title, char *start, char *stop,
	       sopno startst, sopno stopst);
#endif
#ifdef REDEBUG
static char *pchar(ajint ch);
#endif

#ifdef __cplusplus
}
#endif

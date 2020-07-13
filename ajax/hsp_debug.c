#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include "hsp_regex.h"
extern char ajSysItoC(ajint v);

#include "hsp_utils.h"
#include "hsp_regex2.h"

/* == include debug.ih pasted in here == */

static void s_print(register REGUTSSTRUCT *g, FILE *d);
static char *regchar(ajint ch);

/* == end debug.ih == */

/* @func hsp_regprint *********************************************************
**
** print a regexp for debugging
**
** N.B: Was....  void hsp_regprint(regex_t r, FILE *d){ but r has to
**    be a pointer. REALLY BAD to use "r" as a variable in a routine
**    of this length - it's a pity to find it in the code with a
**    search!
**
** @param [r] r [regex_t*] Undocumented
** @param [r] d [FILE*] Undocumented
** @return [void]
******************************************************************************/

void hsp_regprint(regex_t *r, FILE *d)
{
	register REGUTSSTRUCT *g = r->re_g;
	register ajint i;
	register ajint c;
	register ajint last;
	ajint nincat[NC];

	(void) fprintf(d, "%ld states, %d categories", (long)g->nstates,
							g->ncategories);
	(void) fprintf(d, ", first %ld last %ld", (long)g->firststate,
						(long)g->laststate);
	if (g->iflags&USEBOL)
		(void) fprintf(d, ", USEBOL");
	if (g->iflags&USEEOL)
		(void) fprintf(d, ", USEEOL");
	if (g->iflags&BAD)
		(void) fprintf(d, ", BAD");
	if (g->nsub > 0)
		(void) fprintf(d, ", nsub=%ld", (long)g->nsub);
	if (g->must != NULL)
		(void) fprintf(d, ", must(%ld) `%*s'", (long)g->mlen, (ajint)g->mlen,
			       g->must);
	if (g->backrefs)
	    (void) fprintf(d, ", backrefs");
	if (g->nplus > 0)
	    (void) fprintf(d, ", nplus %ld", (long)g->nplus);
	(void) fprintf(d, "\n");
	s_print(g, d);
	for (i = 0; i < g->ncategories; i++)
	{
	    nincat[i] = 0;
	    for (c = CHAR_MIN; c <= CHAR_MAX; c++)
		if ((ajint) g->categories[c] == i)
		    nincat[i]++;
	}
	(void) fprintf(d, "cc0#%d", nincat[0]);
	for (i = 1; i < g->ncategories; i++)
	    if (nincat[i] == 1)
	    {
		for (c = CHAR_MIN; c <= CHAR_MAX; c++)
		    if ((ajint)g->categories[c] == i)
			break;
		(void) fprintf(d, ", %d=%s", i, regchar(c));
	    }
	(void) fprintf(d, "\n");
	for (i = 1; i < g->ncategories; i++)
	    if (nincat[i] != 1) {
		(void) fprintf(d, "cc%d\t", i);
		last = -1;
		for (c = CHAR_MIN; c <= CHAR_MAX+1; c++) /* +1 does flush */
		    if (c <= CHAR_MAX && (ajint) g->categories[c] == i)
		    {
			if (last < 0)
			{
			    (void) fprintf(d, "%s", regchar(c));
			    last = c;
			}
		    }
		    else
		    {
			if (last >= 0)
			  {
			    if (last != c-1)
				(void) fprintf(d, "-%s",
					       regchar(c-1));
			    last = -1;
			}
		    }
		(void) fprintf(d, "\n");
	    }
    }

/* @funcstatic s_print ********************************************************
**
** print the strip for debugging
**
** @param [r] g [register REGUTSSTRUCT*] Undocumented
** @param [r] d [FILE*] Undocumented
** @return [void]
******************************************************************************/

static void s_print(register REGUTSSTRUCT *g, FILE *d)
{
    register sop *s;
    register cset *cs;
    register ajint i;
    register ajint done = 0;
    register sop opnd;
    register ajint col = 0;
    register ajint last;
    register sopno offset = 2;
#define	GAP()	{	if (offset % 5 == 0) { \
    if (col > 40) { \
			(void) fprintf(d, "\n\t"); \
			    col = 0; \
} else { \
	     (void) fprintf(d, " "); \
		 col++; \
} \
} else \
    col++; \
	offset++; \
}

    if (OP(g->strip[0]) != OEND)
	(void) fprintf(d, "missing initial OEND!\n");
    for (s = &g->strip[1]; !done; s++) {
	opnd = OPND(*s);
	switch ((ajint)SWOP(*s)) {
	case AJ_OEND:
	    (void) fprintf(d, "\n");
	    done = 1;
	    break;
	case AJ_OCHAR:
	    if (strchr("\\|()^$.[+*?{}!<> ", ajSysItoC(opnd)) != NULL)
		(void) fprintf(d, "\\%c", ajSysItoC(opnd));
	    else
		(void) fprintf(d, "%s", regchar(ajSysItoC(opnd)));
	    break;
	case AJ_OBOL:
	    (void) fprintf(d, "^");
	    break;
	case AJ_OEOL:
	    (void) fprintf(d, "$");
	    break;
	case AJ_OBOW:
	    (void) fprintf(d, "\\{");
	    break;
	case AJ_OEOW:
	    (void) fprintf(d, "\\}");
	    break;
	case AJ_OANY:
	    (void) fprintf(d, ".");
	    break;
	case AJ_OANYOF:
	    (void) fprintf(d, "[(%ld)", (long)opnd);
	    cs = &g->sets[opnd];
	    last = -1;
	    for (i = 0; i < g->csetsize+1; i++)	/* +1 flushes */
		if (CHIN(cs, i) && i < g->csetsize)
		{
		    if (last < 0)
		    {
			(void) fprintf(d, "%s", regchar(i));
			last = i;
		    }
		}
		else
		{
		    if (last >= 0)
		    {
			if (last != i-1)
			    (void) fprintf(d, "-%s",
					   regchar(i-1));
			last = -1;
		    }
		}
	    (void) fprintf(d, "]");
	    break;
	case AJ_OBACK_:
	    (void) fprintf(d, "(\\<%ld>", (long)opnd);
	    break;
	case AJ_O_BACK:
	    (void) fprintf(d, "<%ld>\\)", (long)opnd);
	    break;
	case AJ_OPLUS_:
	    (void) fprintf(d, "(+");
	    if (OP(*(s+opnd)) != O_PLUS)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    break;
	case AJ_O_PLUS:
	    if (OP(*(s-opnd)) != OPLUS_)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    (void) fprintf(d, "+)");
	    break;
	case AJ_OQUEST_:
	    (void) fprintf(d, "(?");
	    if (OP(*(s+opnd)) != O_QUEST)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    break;
	case AJ_O_QUEST:
	    if (OP(*(s-opnd)) != OQUEST_)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    (void) fprintf(d, "?)");
	    break;
	case AJ_OLPAREN:
	    (void) fprintf(d, "((<%ld>", (long)opnd);
	    break;
	case AJ_ORPAREN:
	    (void) fprintf(d, "<%ld>))", (long)opnd);
	    break;
	case AJ_OCH_:
	    (void) fprintf(d, "<");
	    if (OP(*(s+opnd)) != OOR2)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    break;
	case AJ_OOR1:
	    if (OP(*(s-opnd)) != OOR1 && OP(*(s-opnd)) != OCH_)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    (void) fprintf(d, "|");
	    break;
	case AJ_OOR2:
	    (void) fprintf(d, "|");
	    if (OP(*(s+opnd)) != OOR2 && OP(*(s+opnd)) != O_CH)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    break;
	case AJ_O_CH:
	    if (OP(*(s-opnd)) != OOR1)
		(void) fprintf(d, "<%ld>", (long)opnd);
	    (void) fprintf(d, ">");
	    break;
	default:
	    (void) fprintf(d, "!%d(%d)!", (ajint)OP(*s), (ajint)opnd);
	    break;
	}
	if (!done)
	    GAP();
    }
}

/* @funcstatic regchar ********************************************************
**
** make a character printable
**
** @param [r] ch [ajint] Undocumented
** @return [char*] Undocumented
******************************************************************************/

static char * regchar(ajint ch)
{
    static char buf[10];

    if (isprint(ch) || ch == ' ')
	(void) sprintf(buf, "%c", ch);
    else
	(void) sprintf(buf, "\\%o", ch);
    return(buf);
}

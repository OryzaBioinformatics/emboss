/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#ifndef hsregexp_h
#define hsregexp_h

#include "ajarch.h"

#define NSUBEXP  10

/* @data AjPRegexp ************************************************************
**
** Regular expression data structure, based on the Henry Spencer
** regexp library.
**
** Regular expressions must be compiled before they can be used.
**
** @new ajRegComp Compiles a regular expression from an AjPStr string.
** @new ajRegCompC Compiles a regular expression from a char* string.
** @delete ajRegFree Clears and frees a compiled regular expression.
** @set ajRegExec Compares a regular expression to an AjPStr string.
** @set ajRegExecC Compares a regular expression to a char* string.
** @use ajRegSub Parses a string and substitues matches and submatches.
** @use ajRegSubC Parses a string and substitues matches and submatches.
** @use ajRegTrace Traces a compiled and used regular expression.
** @cast ajRegOffset Returns the offset of a match to an AjPStr.
** @cast ajRegOffsetI Returns the offset of the nth substring match.
** @cast ajRegOffsetC Returns the offset of a match to char*.
** @cast ajRegOffsetIC Returns the offset of the nth substring match.
** @cast ajRegLenI Returns the length of the nth substring match.
** @cast ajRegPost Returns the remainder of the string.
** @cast ajRegPostC Returns the remainder of the string as char*.
** @cast ajRegSubI Returns the nth substring match.
** @@
******************************************************************************/

/* Note that char program[1] must be at the end of the structure
 * as it is a place marker for the start of a node!
 */

typedef struct regexp {
	char *startp[NSUBEXP];
	char *endp[NSUBEXP];
	const char *orig;	/* AJAX extension: start of original */
	char regstart;		/* Internal use only. */
	char reganch;		/* Internal use only. */
        char padding[2];        /* Some chumminess with compiler */
	char *regmust;		/* Internal use only. */
	ajint regmlen;		/* Internal use only. */
	char program[1];	/* more chumminess with compiler. */
} regexp, AjORegexp, *AjPRegexp;

extern regexp *hsregcomp(const char *re);
extern ajint hsregexec(regexp *rp, const char *s);
extern void hsregsub(const regexp *rp, const char *src, char *dst);
extern void hsregerror(const char *message);
extern void hsregerror2(const char* fmt, const char *str);

#ifndef HAVE_MEMMOVE
#ifndef EXIT_FAILURE
#  define EXIT_FAILURE   (1)             /* exit function failure       */
#endif
#endif

#endif

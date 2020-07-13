/*
 * hsregerror
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ajax.h"

/* @func hsregerror ***********************************************************
**
** Error message for the Henry Spencer (modified) regular expression routines.
**
** @param [r] s [const char*] Error message text
** @return [void]
** @@
******************************************************************************/

void hsregerror(const char *s)
{
#ifdef ERRAVAIL
    error("hsregexp: %s", s);
#else
    ajMessError("hsregexp: %s\n", s);
    ajMessCrash("Cannot continue");
    exit(EXIT_FAILURE);
#endif
    /* NOTREACHED */
}





/* @func hsregerror2 **********************************************************
**
** Error message for the Henry Spencer (modified) regular expression routines.
** This version reports the message and the regular expression which
** generated it.
**
** @param [r] exp [const char*] Regular expression
** @param [r] s [const char*] Message text
** @return [void]
** @@
******************************************************************************/

void hsregerror2(const char *exp, const char *s)
{
#ifdef ERRAVAIL
    error("hsregexp: %s", s);
    error("regexp: /%s/\n", exp);
#else
    ajMessError("hsregexp: %s\n", s);
    ajMessError("regexp: /%s/\n", exp);
    ajMessCrash("Cannot continue");
    exit(EXIT_FAILURE);
#endif
    /* NOTREACHED */
}

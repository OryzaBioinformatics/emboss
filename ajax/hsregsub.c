/*
 * regsub
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hsregexp.h"
#include "hsregmagic.h"

/*
 - regsub - perform substitutions after a regexp match
 */
void hsregsub(const regexp *rp, const char *source, char *dest)
{
    register regexp * const prog = (regexp *)rp;
    register char *src = (char *)source;
    register char *dst = dest;
    register char c;
    register int no;
    register size_t len;

    if (prog == NULL || source == NULL || dest == NULL)
    {
	hsregerror("NULL parameter to regsub");
	return;
    }
    if ((int)((unsigned char)*(prog->program)) !=  MAGIC)
    {
	hsregerror("damaged regexp");
	return;
    }

    while ((c = *src++) != '\0')
    {
	if (c == '&')
	    no = 0;
	else if (c == '\\' && isdigit((int)*src))
	    no = *src++ - '0';
	else
	    no = -1;

	if (no < 0) {			/* Ordinary character. */
	    if (c == '\\' && (*src == '\\' || *src == '&'))
		c = *src++;
	    *dst++ = c;
	}
	else if (prog->startp[no] != NULL && prog->endp[no] != NULL &&
		 prog->endp[no] > prog->startp[no])
	{
	    len = prog->endp[no] - prog->startp[no];
	    (void) strncpy(dst, prog->startp[no], len);
	    dst += len;
	    if (*(dst-1) == '\0')
	    {				/* strncpy hit NUL. */
		hsregerror("damaged match string");
		return;
	    }
	}
    }
    *dst++ = '\0';
}

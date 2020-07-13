#include <stdio.h>
#include <string.h>
#include "ajarch.h"
/*
 - split - divide a string into fields, like awk split()
 = ajint split(char *string, char *fields[], ajint nfields, char *sep);
 */
/* number of fields, including overflow
char *string;
char *fields[];			 list is not NULL-terminated
ajint nfields;			 number of entries available in fields[]
char *sep;			 "" white, "c" single char, "ab" [ab]+ */

/* @func split ****************************************************************
**
** Undocumented.
**
** @param [?] string [char*] Undocumented
** @param [?] fields [char* []] Undocumented
** @param [?] nfields [ajint] Undocumented
** @param [?] sep [char*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

ajint split(char *string, char *fields[], ajint nfields, char *sep)
{
    register char *p = string;
    register char c;			/* latest character */
    register char sepc = sep[0];
    register char sepc2;
    register ajint fn;
    register char **fp = fields;
    register char *sepp;
    register ajint trimtrail;

    /* white space */
    if (sepc == '\0')
    {
	while ((c = *p++) == ' ' || c == '\t')
	    continue;
	p--;
	trimtrail = 1;
	sep = " \t";			/* note, code below knows this is 2 ajlong */
	sepc = ' ';
    }
    else
	trimtrail = 0;
    sepc2 = sep[1];			/* now we can safely pick this up */

    /* catch empties */
    if (*p == '\0')
	return(0);

    /* single separator */
    if (sepc2 == '\0')
    {
	fn = nfields;
	for (;;)
	{
	    *fp++ = p;
	    fn--;
	    if (fn == 0)
		break;
	    while ((c = *p++) != sepc)
		if (c == '\0')
		    return(nfields - fn);
	    *(p-1) = '\0';
	}
	/* we have overflowed the fields vector -- just count them */
	fn = nfields;
	for (;;)
	{
	    while ((c = *p++) != sepc)
		if (c == '\0')
		    return(fn);
	    fn++;
	}
	/* not reached */
    }

    /* two separators */
    if (sep[2] == '\0')
    {
	fn = nfields;
	for (;;) {
	    *fp++ = p;
	    fn--;
	    while ((c = *p++) != sepc && c != sepc2)
		if (c == '\0')
		{
		    if (trimtrail && **(fp-1) == '\0')
			fn++;
		    return(nfields - fn);
		}
	    if (fn == 0)
		break;
	    *(p-1) = '\0';
	    while ((c = *p++) == sepc || c == sepc2)
		continue;
	    p--;
	}
	/* we have overflowed the fields vector -- just count them */
	fn = nfields;
	while (c != '\0')
	{
	    while ((c = *p++) == sepc || c == sepc2)
		continue;
	    p--;
	    fn++;
	    while ((c = *p++) != '\0' && c != sepc && c != sepc2)
		continue;
	}
	/* might have to trim trailing white space */
	if (trimtrail)
	{
	    p--;
	    while ((c = *--p) == sepc || c == sepc2)
		continue;
	    p++;
	    if (*p != '\0')
	    {
		if (fn == nfields+1)
		    *p = '\0';
		fn--;
	    }
	}
	return(fn);
    }

    /* n separators */
    fn = 0;
    for (;;)
    {
	if (fn < nfields)
	    *fp++ = p;
	fn++;
	for (;;)
	{
	    c = *p++;
	    if (c == '\0')
		return(fn);
	    sepp = sep;
	    while ((sepc = *sepp++) != '\0' && sepc != c)
		continue;
	    if (sepc != '\0')		/* it was a separator */
		break;
	}
	if (fn < nfields)
	    *(p-1) = '\0';
	for (;;)
	{
	    c = *p++;
	    sepp = sep;
	    while ((sepc = *sepp++) != '\0' && sepc != c)
		continue;
	    if (sepc == '\0')		/* it wasn't a separator */
		break;
	}
	p--;
    }

}

#ifdef TEST_SPLIT






/* @prog hsp_split ************************************************************
**
** test program
**
** pgm		runs regression<br>
** pgm sep	splits stdin lines by sep<br>
** pgm str sep	splits str by sep<br>
** pgm str sep n	splits str by sep n times
**
** @param [?] argc [ajint] Undocumented
** @param [?] argv [char**] Undocumented
** @@
******************************************************************************/

int main(ajint argc, char **argv)
{
    char buf[512];
    register ajint n;
#	define	MNF	10
    char *fields[MNF];

    if (argc > 4)
	for (n = atoi(argv[3]); n > 0; n--)
	{
	    (void) strcpy(buf, argv[1]);
	}
    else if (argc > 3)
	for (n = atoi(argv[3]); n > 0; n--)
	{
	    (void) strcpy(buf, argv[1]);
	    (void) split(buf, fields, MNF, argv[2]);
	}
    else if (argc > 2)
	(void) dosplit(argv[1], argv[2]);
    else if (argc > 1)
	while (fgets(buf, sizeof(buf), stdin) != NULL)
	{
	    buf[strlen(buf)-1] = '\0';	/* stomp newline */
	    (void) dosplit(buf, argv[1]);
	}
    else
	(void) regress();

    exit(0);
}





/* @func dosplit **************************************************************
**
** Undocumented.
**
** @param [?] string [char*] Undocumented
** @param [?] seps [char*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

ajint dosplit(char *string, char *seps)
{
#	define	NF	5
    char *fields[NF];
    register ajint nf;

    nf = split(string, fields, NF, seps);
    (void) hsp_split_print(nf, NF, fields);
}





/* @func hsp_split_print ******************************************************
**
** Undocumented
**
** @param [r] nf [ajint] Undocumented
** @param [r] nfp [ajint] Undocumented
** @param [r] fields [char* []] Undocumented
** @return [ajint] Undocumented
******************************************************************************/

ajint hsp_split_print(ajint nf, ajint nfp, char *fields[])
{
    register ajint fn;
    register ajint bound;

    bound = (nf > nfp) ? nfp : nf;
    (void) printf("%d:\t", nf);
    for (fn = 0; fn < bound; fn++)
	(void) printf("\"%s\"%s", fields[fn], (fn+1 < nf) ? ", " : "\n");
}





#define	RNF	5		/* some table entries know this */
struct {
	char *str;
	char *seps;
	ajint nf;
	char *fi[RNF];
} tests[] = {
	"",		" ",	0,	{ "" },
	" ",		" ",	2,	{ "", "" },
	"x",		" ",	1,	{ "x" },
	"xy",		" ",	1,	{ "xy" },
	"x y",		" ",	2,	{ "x", "y" },
	"abc def  g ",	" ",	5,	{ "abc", "def", "", "g", "" },
	"  a bcd",	" ",	4,	{ "", "", "a", "bcd" },
	"a b c d e f",	" ",	6,	{ "a", "b", "c", "d", "e f" },
	" a b c d ",	" ",	6,	{ "", "a", "b", "c", "d " },

	"",		" _",	0,	{ "" },
	" ",		" _",	2,	{ "", "" },
	"x",		" _",	1,	{ "x" },
	"x y",		" _",	2,	{ "x", "y" },
	"ab _ cd",	" _",	2,	{ "ab", "cd" },
	" a_b  c ",	" _",	5,	{ "", "a", "b", "c", "" },
	"a b c_d e f",	" _",	6,	{ "a", "b", "c", "d", "e f" },
	" a b c d ",	" _",	6,	{ "", "a", "b", "c", "d " },

	"",		" _~",	0,	{ "" },
	" ",		" _~",	2,	{ "", "" },
	"x",		" _~",	1,	{ "x" },
	"x y",		" _~",	2,	{ "x", "y" },
	"ab _~ cd",	" _~",	2,	{ "ab", "cd" },
	" a_b  c~",	" _~",	5,	{ "", "a", "b", "c", "" },
	"a b_c d~e f",	" _~",	6,	{ "a", "b", "c", "d", "e f" },
	"~a b c d ",	" _~",	6,	{ "", "a", "b", "c", "d " },

	"",		" _~-",	0,	{ "" },
	" ",		" _~-",	2,	{ "", "" },
	"x",		" _~-",	1,	{ "x" },
	"x y",		" _~-",	2,	{ "x", "y" },
	"ab _~- cd",	" _~-",	2,	{ "ab", "cd" },
	" a_b  c~",	" _~-",	5,	{ "", "a", "b", "c", "" },
	"a b_c-d~e f",	" _~-",	6,	{ "a", "b", "c", "d", "e f" },
	"~a-b c d ",	" _~-",	6,	{ "", "a", "b", "c", "d " },

	"",		"  ",	0,	{ "" },
	" ",		"  ",	2,	{ "", "" },
	"x",		"  ",	1,	{ "x" },
	"xy",		"  ",	1,	{ "xy" },
	"x y",		"  ",	2,	{ "x", "y" },
	"abc def  g ",	"  ",	4,	{ "abc", "def", "g", "" },
	"  a bcd",	"  ",	3,	{ "", "a", "bcd" },
	"a b c d e f",	"  ",	6,	{ "a", "b", "c", "d", "e f" },
	" a b c d ",	"  ",	6,	{ "", "a", "b", "c", "d " },

	"",		"",	0,	{ "" },
	" ",		"",	0,	{ "" },
	"x",		"",	1,	{ "x" },
	"xy",		"",	1,	{ "xy" },
	"x y",		"",	2,	{ "x", "y" },
	"abc def  g ",	"",	3,	{ "abc", "def", "g" },
	"\t a bcd",	"",	2,	{ "a", "bcd" },
	"  a \tb\t c ",	"",	3,	{ "a", "b", "c" },
	"a b c d e ",	"",	5,	{ "a", "b", "c", "d", "e" },
	"a b\tc d e f",	"",	6,	{ "a", "b", "c", "d", "e f" },
	" a b c d e f ",	"",	6,	{ "a", "b", "c", "d", "e f " },

	NULL,		NULL,	0,	{ NULL },
};





/* @func regress **************************************************************
**
** Undocumented.
**
** @return [ajint] Undocumented
** @@
******************************************************************************/

ajint regress()
{
    char buf[512];
    register ajint n;
    char *fields[RNF+1];
    register ajint nf;
    register ajint i;
    register ajint printit;
    register char *f;

    for (n = 0; tests[n].str != NULL; n++)
    {
	(void) strcpy(buf, tests[n].str);
	fields[RNF] = NULL;
	nf = split(buf, fields, RNF, tests[n].seps);
	printit = 0;
	if (nf != tests[n].nf)
	{
	    (void) printf("split `%s' by `%s' gave %d fields, not %d\n",
			  tests[n].str, tests[n].seps, nf, tests[n].nf);
	    printit = 1;
	}
	else if (fields[RNF] != NULL)
	{
	    (void) printf("split() went beyond array end\n");
	    printit = 1;
	}
	else
	{
	    for (i = 0; i < nf && i < RNF; i++)
	    {
		f = fields[i];
		if (f == NULL)
		    f = "(NULL)";
		if (strcmp(f, tests[n].fi[i]) != 0)
		{
		    (void)
		    printf("split `%s' by `%s', field %d is `%s', not `%s'\n",
				  tests[n].str, tests[n].seps,
				  i, fields[i], tests[n].fi[i]);
		    printit = 1;
		}
	    }
	}
	if (printit)
	    hsp_split_print(nf, RNF, fields);
    }
}
#endif

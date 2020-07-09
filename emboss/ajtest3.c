/*
**
** Test routine for hash functions from AJAX.
**
*/


/* the executable requires  file names for arguments or text from
** stdin.
*/
  
#include "emboss.h"
#include "ajmem.h"
#include "ajassert.h"
#include "ajtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

void wf(char *, FILE *);
ajint first(ajint c);
ajint rest (ajint c);
ajint compare(const void *x, const void *y);
 void vfree(const void *, void **, void *);

ajint getword(FILE *fp, char *buf, ajint size,
        ajint first(ajint c), ajint rest(ajint c)) {
        ajint i = 0, c;
        assert(fp && buf && size > 1 && first && rest);
        c = getc(fp);
        for ( ; c != EOF; c = getc(fp))
                if (first(c)) {
                        {
                                if (i < size - 1)
                                        buf[i++] = c;
                        }
                        c = getc(fp);
                        break;
                }
        for ( ; c != EOF && rest(c); c = getc(fp))
                {
                        if (i < size - 1)
                                buf[i++] = c;
                }
        if (i < size)
                buf[i] = 0;
        else
                buf[size-1] = 0;
        if (c != EOF)
                ungetc(c, fp);
        return i > 0;
}



int main(int argc, char **argv)
{
        ajint i;
	ajNamInit("emboss");
        for (i = 1; i < argc; i++) {
                FILE *fp = fopen(argv[i], "r");
                if (fp == NULL) {
#ifndef HAVE_MEMMOVE
                        ajFatal("%s: can't open '%s' (%s)\n",argv[0], argv[i], sys_errlist[errno]);
#else
                        ajFatal("%s: can't open '%s' (%s)\n",argv[0], argv[i], strerror(errno));
#endif
                } else {
                        wf(argv[i], fp);
                        fclose(fp);
                }
        }
        if (argc == 1) wf(NULL, stdin);
        ajExit();
	return 0;
}

void wf(char *name, FILE *fp) {
        AjPTable table = ajStrTableNew(50);
        char buf[128];
        while (getword(fp, buf, sizeof buf, first, rest)) {
                AjPStr word;
                ajint i, *count;
                for (i = 0; buf[i] != '\0'; i++)
                        buf[i] = tolower(buf[i]);
                word = ajStrNewC(buf);
                count = ajTableGet(table, word);
                if (count){
		  (*count)++;
		  ajStrDel(&word);
		}
                else {
                        AJNEW(count);
                        *count = 1;
                        ajTablePut(table, (const void *)word, (void *) count);
                }
        }
        if (name)
	  ajUser("%s:", name);
        { ajint i;
          void **array = ajTableToarray(table, NULL);
	  /*          qsort(array, ajTableLength(table), 2*sizeof (*array),
                compare);*/
          for (i = 0; array[i]; i += 2)
                ajFmtPrint("%d\t%S\n", *(ajint *)array[i+1],
                        (AjPStr) array[i]);
          AJFREE(array); }
	ajTableMap(table, vfree, NULL);
        ajTableFree(&table);
}

ajint first(ajint c) {
        return isalpha(c);
}

ajint rest(ajint c) {
        return isalpha(c) || c == '_';
}

void vfree(const void *key, void **count, void *cl) {
  AjPStr sy = (AjPStr) key;

  AJFREE(*count);
  ajStrDel(&sy);
}
ajint compare(const void *x, const void *y) {
  AjPStr sx = (AjPStr) x;
  AjPStr sy = (AjPStr) y;

  return ajStrCmpCase (sx, sy);
 }


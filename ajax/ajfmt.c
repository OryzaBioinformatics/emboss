/********************************************************************
** @Source AJAX format functions 
**
** String formatting routines. Similar to printf, fprintf, vprintf 
** etc but the set of conversion specifiers is not fixed, and cannot
** store more characters than it can hold.
**
** Special formatting provided here:
**   %B : AJAX boolean
**   %D : AJAX date
**   %S : AJAX string
**
** Other differences are:
**   %s : accepts null strings and prints null in angle brackets
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
**
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
********************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
#include "ajax.h"
#include "ajfmt.h"
#include "ajtime.h"
#include "ajexcept.h"
#include "ajstr.h"
#include "ajfile.h"

struct buf {
	char *buf;
	char *bp;
	int size;
};

#define pad(n,c) do { int nn = (n); \
                   while (nn-- > 0) \
                     (void) put((c), cl); } while (0)

/* @funcstatic cvt_s **********************************************************
**
** Conversion for %s to print char* text
**
** As for C RTL but prints null if given a null pointer.
**
** @param [r] code [int] Format code specified (usually s)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_s(int code, VALIST ap,
		  int put(int c, void* cl), void* cl,
		  unsigned int* flags, int width, int precision) {

  char *str = va_arg(VA_V(ap), char *);

  if (str)
    ajFmtPuts(str, strlen(str), put, cl, flags,
	      width, precision);
  else
    ajFmtPuts("<null>", 6, put, cl, flags,
	      width, precision);

  return;
}

/* @funcstatic cvt_d **********************************************************
**
** Conversion for %d to print integer
**
** @param [r] code [int] Format code specified (usually d)
** @param [r] ap [va_list*] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_d(int code, VALIST ap,
		  int put(int c, void* cl), void* cl,
		  unsigned int* flags, int width, int precision) {

  long val;
  unsigned long m;
  char buf[43];
  char *p = buf + sizeof buf;

  if (flags['l']) {
    val = (long) va_arg(VA_V(ap), long);
  }
  else if (flags['h']) {
    val = (long) va_arg(VA_V(ap), int); /* ANSI C converts short to int */
  }
  else {
    val = (long) va_arg(VA_V(ap), int);
  }

  if (val == INT_MIN)
    m =INT_MAX + 1U;
  else if (val < 0)
    m = -val;
  else
    m = val;

  do
    *--p = ajSysItoC(m%10 + '0');
  while ((m /= 10) > 0);

  if (val < 0)
    *--p = '-';

  ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	    width, precision);

  return;
}

/* @funcstatic cvt_u **********************************************************
**
** Conversion for %u to print unsigned integer
**
** @param [r] code [int] Format code specified (usually u)
** @param [r] app [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_u(int code, VALIST ap,
		  int put(int c, void* cl), void* cl,
		  unsigned int* flags, int width, int precision) {

  unsigned long m;

  char buf[43];
  char *p = buf + sizeof buf;

  if (flags['l'])
    m  = va_arg(VA_V(ap), unsigned long);
  else if (flags['h'])
    m  = va_arg(VA_V(ap), unsigned int); /* ANSI C converts short to int */
  else
    m  = va_arg(VA_V(ap), unsigned int);

  do
    *--p = ajSysItoC(m%10 + '0');
  while ((m /= 10) > 0);

  ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	    width, precision);

  return;
}

/* @funcstatic cvt_o **********************************************************
**
** Conversion for %o to print unsigned integer as octal
**
** @param [r] code [int] Format code specified (usually o)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_o(int code, VALIST ap,
		  int put(int c, void* cl), void* cl,
		  unsigned int* flags, int width, int precision) {

  unsigned long m;
  char buf[43];
  char *p = buf + sizeof buf;

  if (flags['l'])
    m = va_arg(VA_V(ap), unsigned long);
  if (flags['h'])
    m = va_arg(VA_V(ap), unsigned int); /* ANSI C converts short to int */
  else
    m = va_arg(VA_V(ap), unsigned int);

  do
    *--p = ajSysItoC((m&0x7) + '0');
  while ((m>>= 3) != 0);

  if (flags['#'])
    *--p = '0';

  ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	    width, precision);

  return;
}

/* @funcstatic cvt_x **********************************************************
*
** Conversion for %x to print unsigned integer as hexadecimal
**
** @param [r] code [int] Format code specified (usually x)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_x(int code, VALIST ap,
		  int put(int c, void* cl), void* cl,
		  unsigned int* flags, int width, int precision) {

  unsigned long m;
  char buf[43];
  char *p = buf + sizeof buf;

  if (flags['l'])
    m = va_arg(VA_V(ap), unsigned long);
  else if (flags['h'])
    m = va_arg(VA_V(ap), unsigned int); /* ANSI C converts short to int */
  else
    m = va_arg(VA_V(ap), unsigned int);

  if (code == 'X') {
    do
      *--p = "0123456789ABCDEF"[m&0xf];
    while ((m>>= 4) != 0);
  }
  else {
    do
      *--p = "0123456789abcdef"[m&0xf];
    while ((m>>= 4) != 0);
  }

  while (precision > buf+sizeof(buf)-p)
    *--p = '0';

  if (flags['#']) {
    *--p = 'x';
    *--p = '0';
  }

  ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	    width, precision);

  return;
}

/* @funcstatic cvt_p **********************************************************
**
** Conversion for %p to print pointers of type void* as hexadecimal
**
** @param [r] code [int] Format code specified (usually p)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_p(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
	unsigned long m = (unsigned long)va_arg(VA_V(ap), void*);
	char buf[43];
	char *p = buf + sizeof buf;
	precision = INT_MIN;
	do
		*--p = "0123456789abcdef"[m&0xf];
	while ((m>>= 4) != 0);
	ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
		width, precision);
}

/* @funcstatic cvt_c **********************************************************
**
** Conversion for %c to print an integer (or a character)
** as a single character.
**
** Arguments passed in the variable part of an argument list are promoted
** by default, so char is always promoted to int by the time it reaches here.
**
** @param [r] code [int] Format code specified (usually c)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_c(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
	if (width == INT_MIN)
		width = 0;
	if (width < 0) {
		flags['-'] = 1;
		width = -width;
	}
	if (!flags['-'])
		pad(width - 1, ' ');
	(void) put(ajSysItoUC(va_arg(VA_V(ap), int)), cl);
	if ( flags['-'])
		pad(width - 1, ' ');
}

/* @funcstatic cvt_f **********************************************************
**
** Conversion for %f to print a floating point number.
**
** Because it is generally faster than hand crafted code, the standard
** conversion in sprintf is used, and the resulting string is then
** written out.
**
** Precision is limited to 99 decimal places so it will fit in 2 characters
** of the format for sprintf.
**
** @param [r] code [int] Format code specified (usually f)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_f(int code, VALIST ap,
		  int put(int c, void* cl), void* cl,
		  unsigned int* flags, int width, int precision) {
  char buf[DBL_MAX_10_EXP+1+1+99+1];

  if (precision < 0) {
    if (code == 'f') precision = FLT_DIG;
    else precision = DBL_DIG;
  }
  if (code == 'g' && precision == 0)
    precision = 1;

  {				/* use sprintf to convert to string */
				/* using code and precision */
    static char fmt[12] = "%.dd";
    int i = 2;

    (void) assert(precision <= 99);
    fmt[i++] = ajSysItoC((precision/10)%10 + '0');
    fmt[i++] = ajSysItoC(precision%10 + '0');
    fmt[i++] = ajSysItoC(code);
    fmt[i] = '\0';

    (void) sprintf(buf, fmt, va_arg(VA_V(ap), double));
  }

				/* now write string and support width */
  ajFmtPutd(buf, strlen(buf), put, cl, flags,
	    width, precision);

  return;
}

/* @funcstatic cvt_S **********************************************************
**
** Conversion for %S to print a string
**
** @param [r] code [int] Format code specified (usually S)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_S(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
	AjPStr str1 = va_arg(VA_V(ap), AjPStr);
	if (str1) {
	  ajFmtPuts(str1->Ptr, str1->Len, put, cl, flags,
		    width, precision);
	}
	else {
	  ajFmtPuts("<null>", 6, put, cl, flags,
		    width, precision);
	}
}

/* @funcstatic cvt_b **********************************************************
**
** Conversion for %b to print a boolean as a 1 letter code (Y or N)
**
** @param [r] code [int] Format code specified (usually b)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_b(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
	AjBool bl = va_arg(VA_V(ap), AjBool);
	if (bl)
	  ajFmtPuts("Y", 1, put, cl, flags,
		width, precision);
	else
	  ajFmtPuts("N", 1, put, cl, flags,
		width, precision);

}

/* @funcstatic cvt_B **********************************************************
**
** Conversion for %B to print a boolean as text (Yes or No)
**
** @param [r] code [int] Format code specified (usually B)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_B(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
	AjBool bl = va_arg(VA_V(ap), AjBool);
	if (bl)
	  ajFmtPuts("Yes", 3, put, cl, flags,
		width, precision);
	else
	  ajFmtPuts("No", 2, put, cl, flags,
		width, precision);

}

/* @funcstatic cvt_D **********************************************************
**
** Conversion for %D to print a datetime value
**
** @param [r] code [int] Format code specified (usually D)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_D(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
        AJTIME *time =  va_arg(VA_V(ap), AJTIME *);
	struct tm *mytime = time->time;

	char buf[280];
	char yr[280];
	
	if(time->format){
	  (void) strftime(buf,280, time->format,mytime);
	}
	else{
	    /* AJB. Extra 3 lines to get round damned %y warning. Sigh */
	    (void) strftime(yr,280,"%Y",mytime);
	    (void) strcpy(yr,&yr[strlen(yr)-2]);
	    (void) strftime(buf,280, "%d/%m/", mytime);
	    (void) strcat(buf,yr);
	  /*	  (void) sprintf(buf,"%2.2d/%2.2d/%2.2d",   oops: Y2K bug :-)
		  mytime->tm_mday, mytime->tm_mon+1, mytime->tm_year);*/
	}
	ajFmtPuts(&buf[0], strlen(buf), put, cl, flags,
		width, precision);
}

/* @funcstatic cvt_F **********************************************************
**
** Conversion for %F to print a file object
**
** @param [r] code [int] Format code specified (usually F)
** @param [r] ap [va_list] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_F(int code, VALIST ap,
	int put(int c, void* cl), void* cl,
	unsigned int* flags, int width, int precision) {
	AjPFile fil = va_arg(VA_V(ap), AjPFile);
	if (fil && fil->Name) {
	  ajFmtPuts(fil->Name->Ptr, fil->Name->Len, put, cl, flags,
		    width, precision);
	}
	else {
	  ajFmtPuts("<null>", 6, put, cl, flags,
		    width, precision);
	}
}


static const Except_T Fmt_Overflow = { "Formatting Overflow" };

/* ****************************************************************************
**
** Conversion functions called for each conversion code.
**
** Usually, code "x" will call "cvt_x" but there are exceptions. For example,
** floating point conversions all use cvt_f which sends everything to
** the standard C library. Also, cvt_d is used by alternative codes.
**
******************************************************************************/

static Fmt_T cvt[256] = {
 /*   0-  7 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*   8- 15 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  16- 23 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  24- 31 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  32- 39 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  40- 47 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  48- 55 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  56- 63 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  64- 71 */      0,     0, cvt_B,     0, cvt_D,     0, cvt_F,     0,
 /*  72- 79 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  80- 87 */      0,     0,     0, cvt_S,     0,     0,     0,     0,
 /*  88- 95 */  cvt_x,     0,     0,     0,     0,     0,     0,     0,
 /*  96-103 */      0,     0, cvt_b, cvt_c, cvt_d, cvt_f, cvt_f, cvt_f,
 /* 104-111 */      0,     0,     0,     0,     0,     0, cvt_d, cvt_o,
 /* 112-119 */  cvt_p,     0,     0, cvt_s,     0, cvt_u,     0,     0,
 /* 120-127 */  cvt_x,     0,     0,     0,     0,     0,     0,     0
};

/* ****************************************************************************
**
** Legal flag characters for conversions:
**  '-' left justify value within field.
**  '+' always put a sign character '+' of '-' for a numeric value.
**  ' ' always put a sign character ' ' or '-' for a numeric value.
**  '0' pad width with zeroes rather than spaces
**  '#' alternative forms of the e,f,g,E,G formats
**      for C this also changes o,x,X but is not yet implemented here.
** 
******************************************************************************/

static char *Fmt_flags = "-+ 0#"; 

static int outc(int c, void* cl) {
	FILE *f = cl;
	return putc(c, f);
}

static int s_ajinsert(int c, void* cl) {
	struct buf *p = cl;
	if (p->bp >= p->buf + p->size)
		AJRAISE(Fmt_Overflow);
	*p->bp++ = ajSysItoC(c);
	return c;
}

static int s_ajappend(int c, void* cl) {
	struct buf *p = cl;
	if (p->bp >= p->buf + p->size) {
		AJRESIZE(p->buf, 2*p->size);
		p->bp = p->buf + p->size;
		p->size *= 2;
	}
	*p->bp++ = ajSysItoC(c);
	return c;
}

/* @func ajFmtPuts *******************************************************
**
** Format and emit the converted numeric (ajFmtPutd) or string
** (ajFmtPuts) in str[0..len-1] according to Fmt's defaults 
** and the values of flags,width and precision. It is a c.r.e
** for str=null, len less than 0 or flags=null.
**
** @param [w] str [const char*] Text to write.
** @param [r] len [int] Text length.
** @param [f] put [int function] Standard function.
** @param [r] cl [void*] Standard.
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @cre attempting to write over len chars to str
**
** @@
**************************************************************************/

void ajFmtPuts (const char* str, int len,
	       int put(int c, void* cl), void* cl,
	       unsigned int* flags, int width, int precision) {

  (void) assert(str);
  (void) assert(len >= 0);
  (void) assert(flags);

  if (width == INT_MIN)
    width = 0;
  if (width < 0) {
    flags['-'] = 1;
    width = -width;
  }
  if (precision >= 0)
    flags['0'] = 0;
  if (precision >= 0 && precision < len)
    len = precision;
  if (!flags['-'])
    pad(width - len, ' ');
  {
    int i;
    for (i = 0; i < len; i++)
      (void) put((unsigned char)*str++, cl);
  }
  if ( flags['-'])
    pad(width - len, ' ');

  return;
}

/* @func ajFmtFmt **********************************************************
**
** formats and emits the "..." arguments according to the format string fmt
**
** @param [f] put [int function] Standard function.
** @param [rP] cl [void*] Standard.
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
**
** @@
**************************************************************************/

void ajFmtFmt (int put(int c, void* cl), void* cl,
	const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ajFmtVfmt(put, cl, fmt, ap);
	va_end(ap);
}

/* @func  ajFmtPrint *****************************************************
**
** format and emit the "..." arguments according to fmt;writes to stdout.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtPrint (const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ajFmtVfmt(outc, stdout, fmt, ap);
	va_end(ap);
}

/* @func ajFmtVPrint *****************************************************
**
** format and emit the "..." arguments according to fmt;writes to stdout.
**
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtVPrint (const char* fmt, va_list ap) {
	ajFmtVfmt(outc, stdout, fmt, ap);
}

/* @func ajFmtError  *****************************************************
**
** format and emit the "..." arguments according to fmt;writes to stderr.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtError (const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ajFmtVfmt(outc, stderr, fmt, ap);
	va_end(ap);
}

/* @func ajFmtVError  *****************************************************
**
** format and emit the "..." arguments according to fmt;writes to stderr.
**
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtVError (const char* fmt, va_list ap) {
	ajFmtVfmt(outc, stderr, fmt, ap);
}

/* @func ajFmtPrintF  *****************************************************
**
** format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [r] file [AjPFile] Output file.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtPrintF (AjPFile file, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ajFmtVfmt(outc, file->fp, fmt, ap);
	va_end(ap);
}

/* @func ajFmtVPrintF ******************************************************
**
** format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [r] file [AjPFile] Output file.
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtVPrintF(AjPFile file, const char* fmt, va_list ap) {
	ajFmtVfmt(outc, file->fp, fmt, ap);
}

/* @func ajFmtVPrintFp ***************************************************
**
** Format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [r] stream [FILE*] Output file.
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtVPrintFp(FILE* stream, const char* fmt, va_list ap) {
	ajFmtVfmt(outc, stream, fmt, ap);
}

/* @func ajFmtPrintFp ***************************************************
**
** format and emit the "..." arguments according to fmt;writes to stream..
**
** @param [r] stream [FILE*] Output file.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtPrintFp(FILE* stream, const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	ajFmtVfmt(outc, stream, fmt, ap);
	va_end(ap);
}

/* @func ajFmtVPrintCL ****************************************************
**
** formats the "..." arguments into buf[1...size-1] according to fmt,
** appends a num character, and returns the length of buf. It is a
** c.r.e for size to be less than or equal to 0. Raises Fmt_Overflow
** if more than size-1 characters are emitted.
**
** @param [w] buf [char*] char string to be written too.
** @param [r] size [int] length of buffer
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list
** @return [int] number of characters written to buf.
** @@
**************************************************************************/

int ajFmtVPrintCL(char* buf, int size, const char* fmt, va_list ap) {
	int len;
	len = ajFmtVfmtCL(buf, size, fmt, ap);
	return len;
}


/* @func ajFmtPrintCL ***************************************************
**
** formats the "..." arguments into buf[1...size-1] according to fmt,
** appends a num character, and returns the length of buf. It is a
** c.r.e for size to be lass than or equal to 0. Raises Fmt_Overflow
** if more than size-1 characters are emitted.
**
** @param [w] buf [char*] char string to be written too.
** @param [r] size [int] length of buffer
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
**
** @return [] [int] number of characters written to buf.
**
** @@
**************************************************************************/

int ajFmtPrintCL(char* buf, int size, const char* fmt, ...) {
	va_list ap;
	int len;
	va_start(ap, fmt);
	len = ajFmtVfmtCL(buf, size, fmt, ap);
	va_end(ap);
	return len;
}

/* @func ajFmtStr ************************************************************
**
** Formats the "..." arguments into a New AjPStr according to fmt.
** It starts with an initial size of 20 then doubles until the
** fmt output fits.
**
** The caller is reponsible for deleting the AjPStr afterwards.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
**
** @return [AjPStr] fnew AjPStr with Ptr holding formatted chars
** @@ 
******************************************************************************/

AjPStr ajFmtStr (const char* fmt, ...) {
	va_list ap;
#if defined(__PPC__) && defined(_CALL_SYSV)
	va_list save_ap;
#endif
	volatile AjBool okay=ajFalse;
	int len =20;
	AjPStr fnew;

	fnew = ajStrNewL (len);
	va_start(ap, fmt);

#if defined(__PPC__) && defined(_CALL_SYSV)
	__va_copy(save_ap, ap);
#endif
	while(!okay){
	  AJTRY
	    len = ajFmtVfmtCL(fnew->Ptr, fnew->Res, fmt, ap);
	    fnew->Len = len;
	    okay = ajTrue;
       	  ELSE
	    len = fnew->Res *2;       /* double the memory and try again */
	    (void) ajStrModL(&fnew, len);
#if defined(__PPC__) && defined(_CALL_SYSV)
	    __va_copy(ap, save_ap);
#endif
	  END_TRY;
	}  
	va_end(ap);
	return fnew;
}

/* @func ajFmtPrintS *******************************************************
**
** Formats the "..." arguments into an AjPStr according to fmt.
** If AjPStr is not large enough then if it is the only one i.e
** Use = 1 then increase till it fits. Else return 0 if it does not
** fit. If it fits return the address of the new AjPStr.
**
** @param [u] pthis [AjPStr*] String to be written too.
** @param [r] fmt [const char*] Format for string.
** @param [v] [...] Variable length argument list
**
** @return [AjPStr] Output string
**
** @error on unsuccessful writing return 0
**
** @@
** NOTE: unsafe may be best to pass a pointer to the pointer new 
** as it passes back 0 if not able to be done
*****************************************************************************/ 

AjPStr ajFmtPrintS (AjPStr* pthis, const char* fmt, ...) {
        volatile AjPStr thys;
	va_list ap;
#if defined(__PPC__) && defined(_CALL_SYSV)
	va_list save_ap;
#endif
	volatile AjBool okay = ajFalse;
	int len ;

	va_start(ap, fmt);

	(void) ajStrModL(pthis, 32);
	thys = *pthis;
	len = thys->Res;
#if defined(__PPC__) && defined(_CALL_SYSV)
	__va_copy(save_ap, ap);
#endif
	while(!okay){
	  AJTRY
	    len = ajFmtVfmtCL(thys->Ptr, thys->Res, fmt, ap);
	    thys->Len = len;
	    okay = ajTrue;
	  ELSE
	    if(thys->Use == 1){
	      len = thys->Res *2;        /* double the memory and try again */
	      /* create new one with twice the memory */
	      (void) ajStrModL(pthis, len);
	      thys = *pthis;
#if defined(__PPC__) && defined(_CALL_SYSV)
	      __va_copy(ap, save_ap);
#endif
	    }
	    else{
	      thys= 0;
	      okay = ajTrue;
	      ajMessOutCode("BUFCPY");
	    }
	  END_TRY;
	}  
	va_end(ap);

	return thys;
}

/* @func ajFmtPrintAppS ****************************************************
**
** Formats the "..." arguments and appends to an AjPStr according to fmt.
** If AjPStr is not large enough then if it is the only one i.e
** Use = 1 then increase till it fits. Else return 0 if it does not
** fit. If it fits return the address of the new AjPStr.
**
** @param [u] pthis [AjPStr*] String to be written too.
** @param [r] fmt [const char*] Format for string.
** @param [v] [...] Variable length argument list
**
** @return [AjPStr] Output string.
**
** @error on unsuccessful writing return 0
**
** @@
** NOTE: unsafe may be best to pass a pointer to the pointer new 
** as it passes back 0 if not able to be done
*****************************************************************************/ 

AjPStr ajFmtPrintAppS(AjPStr* pthis, const char* fmt, ...) {
        volatile AjPStr thys;
	va_list ap;
#if defined(__PPC__) && defined(_CALL_SYSV)
	va_list save_ap;
#endif
	volatile AjBool okay = ajFalse;
	int len ;

	va_start(ap, fmt);

	(void) ajStrModL(pthis, 32);
	thys = *pthis;
	len = thys->Res;
#if defined(__PPC__) && defined(_CALL_SYSV)
	__va_copy(save_ap, ap);
#endif
	while(!okay){
	  AJTRY
	    len = ajFmtVfmtCL(&thys->Ptr[thys->Len], thys->Res-thys->Len,
			      fmt, ap);
	    thys->Len += len;
	    okay = ajTrue;
	  ELSE
	    if(thys->Use == 1){
	      len = thys->Res *2;        /* double the memory and try again */
	      /* create new one with twice the memory */
	      (void) ajStrModL(pthis, len);
	      thys = *pthis;
#if defined(__PPC__) && defined(_CALL_SYSV)
	      __va_copy(ap, save_ap);
#endif
	    }
	    else{
	      thys= 0;
	      okay = ajTrue;
	      ajMessOutCode("BUFCPY");
	    }
	  END_TRY;
	}  
	va_end(ap);

	return thys;
}

/* @func ajFmtVfmtCL **********************************************************
**
** Same as ajFmtPrintCL but takes arguments from the list ap.
**
** @param [w] buf [char*] char string to be written too.
** @param [r] size [int] length of buffer
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list.
**
** @return [] [int] number of characters written to buf.
**
** @@
**************************************************************************/

int ajFmtVfmtCL(char* buf, int size, const char* fmt,
	va_list ap) {
	struct buf cl;
	(void) assert(buf);
	(void) assert(size > 0);
	(void) assert(fmt);
	cl.buf = cl.bp = buf;
	cl.size = size;
	ajFmtVfmt(s_ajinsert, &cl, fmt, ap);
	(void) s_ajinsert(0, &cl);
	return cl.bp - cl.buf - 1;
}

/* @func ajFmtString *****************************************************
**
** formats the "..." arguments into a null-terminated string according to
** fmt and returns that string.
**
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
**
** @return [char*] Output string.
**
** @@ 
***************************************************************************/

char* ajFmtString(const char* fmt, ...) {
	char *str;
	va_list ap;
	(void) assert(fmt);
	va_start(ap, fmt);
	str = ajFmtVString(fmt, ap);
	va_end(ap);
	return str;
}

/* @func ajFmtVString *****************************************************
**
** as ajFmtString but takes arguments from the list ap.
**
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list.
**
** @return [char*] Output string.
**
** @@
***************************************************************************/

char* ajFmtVString(const char* fmt, va_list ap) {

  struct buf cl;

  (void) assert(fmt);

  cl.size = 256;
  cl.buf = cl.bp = AJALLOC(cl.size);
  ajFmtVfmt(s_ajappend, &cl, fmt, ap);
  (void) s_ajappend(0, &cl);

  return AJRESIZE(cl.buf, cl.bp - cl.buf);
}

/* @func ajFmtVfmt ***********************************************************
**
** as ajFmtPrint but takes arguments from the list ap.
**
** @param [f] put [int function] Standard function
** @param [rP] cl [void*] Where we are going to write the results
** @param [r] fmt [const char*] Format string
** @param [r] ap [va_list] Variable argument list
** @return [void]
** @@
******************************************************************************/

void ajFmtVfmt (int put(int c, void* cl), void* cl,
	       const char* fmt, va_list ap) {

  (void) assert(put);
  (void) assert(fmt);

  while (*fmt) {
    if (*fmt != '%' || *++fmt == '%') /* %% just outputs '%' */
      (void) put((unsigned char)*fmt++, cl);
    else {			/* we have a % - get working on the format */
      unsigned char c;
      int flags[256];
      int width = INT_MIN, precision = INT_MIN;
      (void) memset(flags, '\0', sizeof flags);
      if (Fmt_flags) {		/* look for any conversion flags */
	unsigned char c = *fmt;
	for ( ; (int)c && strchr(Fmt_flags, c); c = *++fmt) {
	  (void) assert(flags[(int)c] < 255);
	  flags[(int)c]++;
	}
      }
      if (*fmt == '*' || isdigit((int)*fmt)) { 
	int n;
	if (*fmt == '*') {	/* '*' width = int arg */
	  n = va_arg(ap, int);
	  (void) assert(n != INT_MIN);
	  fmt++;
	} else
	  for (n = 0; isdigit((int)*fmt); fmt++) {
	    int d = *fmt - '0';
	    (void) assert(n <= (INT_MAX - d)/10);
	    n = 10*n + d;
	  }
	width = n;
      }
      if (*fmt == '.' && (*++fmt == '*' || isdigit((int)*fmt))) {
	int n; 
	if (*fmt == '*') {	/* '*' precision = int arg */
	  n = va_arg(ap, int);
	  (void) assert(n != INT_MIN);
	  fmt++;
	} else
	  for (n = 0; isdigit((int)*fmt); fmt++) {
	    int d = *fmt - '0';
	    (void) assert(n <= (INT_MAX - d)/10);
	    n = 10*n + d;
	  }
	  precision = n;
      }
      if (*fmt == 'l' || *fmt == 'L'|| *fmt == 'h') { /* size modifiers */
	(void) assert(flags[(int)*fmt] < 255); /* store as flags - */
	                                          /* values do not clash */
	flags[(int)*fmt]++;
	fmt++;
      }
      c = *fmt++;		/* finally, next character is the code */
      (void) assert(cvt[(int)c]);		/* we need a defined routine */
      (*cvt[(int)c])(c, VA_P(ap), put, cl, (unsigned int *)flags, width, precision);
    }
  }
}

/* @func ajFmtRegister *************************************************
**
** Registers 'newcvt' as the conversion routine for format code 'code'
**
** @param [r] code [int] value of char to be replaced
** @param [f] newcvt [Fmt_T] new routine for conversion
**
** @return [Fmt_T] old value
** @@
************************************************************************/

Fmt_T ajFmtRegister(int code, Fmt_T newcvt) {
  Fmt_T old;
  (void) assert(0 < code
		&& code < (int)(sizeof (cvt)/sizeof (cvt[0])));
  old = cvt[code];
  cvt[code] = newcvt;
  return old;
}

/* @func ajFmtPutd **********************************************************
**
** Given a string containing a number in full, converts it using the width
** and precision values.
**
** @param [w] str [const char*] Text to write.
** @param [r] len [int] Text length.
** @param [f] put [int function] Standard function.
** @param [r] cl [void*] Standard.
** @param [r] flags [unsigned int*] Flags (after the %)
** @param [r] width [int] Width (before the dot)
** @param [r] precision [int] Precision (after the dot)
** @return [void]
** @@
*****************************************************************************/

void ajFmtPutd(const char* str, int len,
	      int put(int c, void* cl), void* cl,
	      unsigned int* flags, int width, int precision) {
  int sign;
  (void) assert(str);
  (void) assert(len >= 0);
  (void) assert(flags);

  if (width == INT_MIN)
    width = 0;
  if (width < 0) {
    flags['-'] = 1;
    width = -width;
  }

  /*
  if (precision >= 0)
    flags['0'] = 0;
  */

  if (len > 0 && (*str == '-' || *str == '+')) {
    sign = *str++;
    len--;
  } else if (flags['+'])
    sign = '+';
  else if (flags[' '])
    sign = ' ';
  else
    sign = 0;

  {
    int n;
    int j=0;
    if (precision < 0)
      precision = 1;
    if (len < precision)
      n = precision;
    else if (precision == 0 && len == 1 && str[0] == '0')
      n = 0;
    else
      n = len;
    if (sign)
      n++;

    if (flags['#'] && flags['0']) { /* make space for the padding */
      if (*str == '0' && *(str+1) == 'x') {
	(void) put((unsigned char)*str++, cl);
	(void) put((unsigned char)*str++, cl);
	j += 2;
      }
    }
    if (flags['-']) {
      if (sign)
	(void) put(sign, cl);
    }
    else if (flags['0']) {
      if (sign)
	(void) put(sign, cl);
      pad(width - n, '0');
    }
    else {
      pad(width - n, ' ');
      if (sign)
	(void) put(sign, cl);
    }
    pad(precision - len, '0');	/* pad after end */
    {
      int i;
      for (i = j; i < len; i++)
	(void) put((unsigned char)*str++, cl);
    }
    if (flags['-'])
      pad(width - n, ' ');
  }
}

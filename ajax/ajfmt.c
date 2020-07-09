/********************************************************************
** @Source AJAX format functions 
**
** String formatting routines. Similar to printf, fprintf, vprintf 
** etc but the set of conversion specifiers is not fixed, and cannot
** store more characters than it can hold.
** There is also ajFmtScanS which is an extended sscanf.
**
** Special formatting provided here:
**   %B : AJAX boolean
**   %D : AJAX date
**   %S : AJAX string
**   %z : Dynamic char* allocation in ajFmtScanS
**
** Other differences are:
**   %s : accepts null strings and prints null in angle brackets
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Copyright (C) 2001 Alan Bleasby. Added ajFmtScanS functs
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

typedef struct FmtSBuf {
  char *buf;			/* buffer to write */
  char *bp;			/* next position in buffer */
  ajint size;			/* size of buffer from malloc */
  AjBool fixed;			/* if ajTrue, cannot reallocate */
} FmtOBuf, *FmtPBuf;

#define pad(n,c) do { ajint nn = (n); \
                   while (nn-- > 0) \
                     (void) put((c), cl); } while (0)

static AjBool c_notin(ajint c, char *list);
static AjBool c_isin(ajint c, char *list);
static ajint ajFmtVscan(char *thys,const char *fmt,va_list ap);

static void scvt_uS(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_d(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_x(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_f(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_s(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_o(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_u(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_p(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_uB(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_c(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_b(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);
static void scvt_z(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok);


static void cvt_s(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_uB(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_uD(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_uF(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_uS(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_x(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_b(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_c(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_d(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_f(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_o(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_p(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);
static void cvt_u(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision);

#if defined(HAVE64)
static ajlong sc_long(const char *str);
static ajulong sc_ulong(const char *str);
static ajulong sc_hex(const char *str);
static ajulong sc_octal(const char *str);
#endif

/* @funcstatic c_isin ********************************************************
**
** Checks whether a character is in a set
**
** @param [r] c [ajint] character
** @param [r] list [char*] set of characters
** @return [AjBool] ajTrue if character is in the set
** @@
******************************************************************************/

static AjBool c_isin(ajint c, char *list)
{
    while(*list)
	if(c == (ajint)*(list++))
	    return ajTrue;

    return ajFalse;
}

/* @funcstatic c_notin ********************************************************
**
** Checks whether a character is not in a set
**
** @param [r] c [ajint] character
** @param [r] list [char*] set of characters
** @return [AjBool] ajTrue if character is not in the set
** @@
******************************************************************************/

static AjBool c_notin(ajint c, char *list)
{
    while(*list)
	if(c == (ajint)*(list++))
	    return ajFalse;

    return ajTrue;
}


/* @funcstatic cvt_s **********************************************************
**
** Conversion for %s to print char* text
**
** As for C RTL but prints null if given a null pointer.
**
** @param [r] code [ajint] Format code specified (usually s)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_s(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{

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
** @param [r] code [ajint] Format code specified (usually d)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_d(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{

    long val=0;
#if defined(HAVE64)
    ajlong hval=0;
#endif
    ajulong m=0;
  
    char buf[43];
    char *p = buf + sizeof buf;

    if (flags['l'])
    {
	val = (long) va_arg(VA_V(ap), long);
#if defined(HAVE64)
	hval = val;
#endif
    }
    else if(flags['L'])
    {
#if defined(HAVE64)
	hval = (ajlong) va_arg(VA_V(ap),ajlong);
	val = hval;
#else
	val = (long) va_arg(VA_V(ap), long);
	ajDebug("Warning: Use of %%Ld on a 32 bit model");
#endif
    }
    else if (flags['h'])
    {
	/* ANSI C converts short to ajint */
	val = (long) va_arg(VA_V(ap), int);
#if defined(HAVE64)
	hval = val;
#endif
    }
    else
    {
	val = (long) va_arg(VA_V(ap), int);
#if defined(HAVE64)
	hval = val;
#endif
    }
    

#if defined(HAVE64)
    if (hval == INT_MIN)
	m =INT_MAX + 1U;
    else if (hval < 0)
	m = -hval;
    else
	m = hval;

    do
	*--p = ajSysItoC(m%10 + '0');
    while ((m /= 10) > 0);

    if (hval < 0)
	*--p = '-';
#else
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
#endif

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}

/* @funcstatic cvt_u **********************************************************
**
** Conversion for %u to print unsigned integer
**
** @param [r] code [ajint] Format code specified (usually u)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_u(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    unsigned long m=0;
#if defined(HAVE64)
    ajulong  hm=0;
#endif
    char buf[43];
    char *p = buf + sizeof buf;

    if(flags['l'])
	m  = va_arg(VA_V(ap), unsigned long);
    else if(flags['h'])
	/* ANSI C converts short to ajint */
	m  = va_arg(VA_V(ap), unsigned int);
    else if(flags['L'])
    {
#if defined(HAVE64)
	hm = va_arg(VA_V(ap), ajulong);
#else
	m  = va_arg(VA_V(ap), unsigned long);
	ajDebug("Warning: Use of %%L on 32 bit model");
#endif
    }
    else
	m  = va_arg(VA_V(ap), unsigned int);

#if !defined(HAVE64)
    do
	*--p = ajSysItoC(m%10 + '0');
    while((m /= 10) > 0);
#else
    do
	*--p = ajSysItoC((int)(hm%(ajulong)10 + '0'));
    while((hm /= (ajulong)10) > 0);
#endif
    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}

/* @funcstatic cvt_o **********************************************************
**
** Conversion for %o to print unsigned integer as octal
**
** @param [r] code [ajint] Format code specified (usually o)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_o(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    unsigned long m=0;
    char buf[43];
    char *p = buf + sizeof buf;
#if defined(HAVE64)
    ajulong hm=0;
#endif

    if(flags['l'])
	m = va_arg(VA_V(ap), unsigned long);
    if(flags['h'])
	/* ANSI C converts short to ajint */
	m = va_arg(VA_V(ap), unsigned int);
    else if(flags['L'])
    {
#if defined(HAVE64)
	hm = (ajulong) va_arg(VA_V(ap), ajulong);
#else
	m = va_arg(VA_V(ap), unsigned long);
	ajDebug("Warning: Use of %%Lo on a 32 bit model");
#endif
    }
    else
	m = va_arg(VA_V(ap), unsigned int);

#if !defined(HAVE64)
    do
	*--p = ajSysItoC((m&0x7) + '0');
    while((m>>= 3) != 0);
#else
    do
	*--p = ajSysItoC((int)((hm&0x7) + '0'));
    while((hm>>= 3) != 0);
#endif

    if(flags['#'])
	*--p = '0';

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}

/* @funcstatic cvt_x **********************************************************
*
** Conversion for %x to print unsigned integer as hexadecimal
**
** @param [r] code [ajint] Format code specified (usually x)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_x(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    unsigned long m=0;
#if defined(HAVE64)
    ajulong hm=0;
#endif
    char buf[43];
    char *p = buf + sizeof buf;

    if(flags['l'])
	m = va_arg(VA_V(ap), unsigned long);
    else if(flags['h'])
	/* ANSI C converts short to int */
	m = va_arg(VA_V(ap), unsigned int);
    else if(flags['L'])
    {
#if defined(HAVE64)
	hm = va_arg(VA_V(ap), ajulong);
#else
	m = va_arg(VA_V(ap), unsigned long);
	ajDebug("Warning: Use of %%Lx on a 32 bit model");
#endif
    }
    else
	m = va_arg(VA_V(ap), unsigned int);

    if(code == 'X')
    {
	do
	    *--p = "0123456789ABCDEF"[m&0xf];
	while((m>>= 4) != 0);
    }
    else
    {
#if !defined(HAVE64)
	do
	    *--p = "0123456789abcdef"[m&0xf];
	while((m>>= 4) != 0);
#else
	do
	    *--p = "0123456789abcdef"[hm&0xf];
	while((hm>>= 4) != 0);
#endif
    }

    while(precision > buf+sizeof(buf)-p)
	*--p = '0';

    if(flags['#'])
    {
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
** @param [r] code [ajint] Format code specified (usually p)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_p(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    unsigned long m = (unsigned long)va_arg(VA_V(ap), void*);
    char buf[43];
    char *p = buf + sizeof buf;
    precision = INT_MIN;

    do
	*--p = "0123456789abcdef"[m&0xf];
    while((m>>= 4) != 0);

    ajFmtPutd(p, (buf + sizeof buf) - p, put, cl, flags,
	      width, precision);

    return;
}

/* @funcstatic cvt_c **********************************************************
**
** Conversion for %c to print an integer (or a character)
** as a single character.
**
** Arguments passed in the variable part of an argument list are promoted
** by default, so char is always promoted to ajint by the time it reaches here.
**
** @param [r] code [ajint] Format code specified (usually c)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_c(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    if(width == INT_MIN)
	width = 0;
    if(width < 0)
    {
	flags['-'] = 1;
	width = -width;
    }
    if(!flags['-'])
	pad(width - 1, ' ');
    (void) put(ajSysItoUC(va_arg(VA_V(ap), int)), cl);
    if(flags['-'])
	pad(width - 1, ' ');

    return;
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
** @param [r] code [ajint] Format code specified (usually f)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_f(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    char buf[DBL_MAX_10_EXP+1+1+99+1];

    if (precision < 0)
    {
	if (code == 'f') precision = FLT_DIG;
	else precision = DBL_DIG;
    }
    if (code == 'g' && precision == 0)
	precision = 1;

    {					/* use sprintf to convert to string */
	/* using code and precision */
	static char fmt[12] = "%.dd";
	ajint i = 2;

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

/* @funcstatic cvt_uS *********************************************************
**
** Conversion for %S to print a string
**
** @param [r] code [ajint] Format code specified (usually S)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uS(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    AjPStr str1 = va_arg(VA_V(ap), AjPStr);

    if (str1)
	ajFmtPuts(str1->Ptr, str1->Len, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("<null>", 6, put, cl, flags,
		  width, precision);

    return;
}

/* @funcstatic cvt_b **********************************************************
**
** Conversion for %b to print a boolean as a 1 letter code (Y or N)
**
** @param [r] code [ajint] Format code specified (usually b)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_b(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    AjBool bl = va_arg(VA_V(ap), AjBool);

    if(bl)
	ajFmtPuts("Y", 1, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("N", 1, put, cl, flags,
		  width, precision);

    return;
}

/* @funcstatic cvt_uB *********************************************************
**
** Conversion for %B to print a boolean as text (Yes or No)
**
** @param [r] code [ajint] Format code specified (usually B)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uB(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    AjBool bl = va_arg(VA_V(ap), AjBool);

    if(bl)
	ajFmtPuts("Yes", 3, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("No", 2, put, cl, flags,
		  width, precision);

    return;
}

/* @funcstatic cvt_uD *********************************************************
**
** Conversion for %D to print a datetime value
**
** @param [r] code [ajint] Format code specified (usually D)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uD(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    AjPTime time =  va_arg(VA_V(ap), AjPTime);
    struct tm *mytime = time->time;

    char buf[280];
    char yr[280];
	
    if(time->format)
	(void) strftime(buf,280, time->format,mytime);
    else
    {
	/* Long-winded but gets around some compilers' %y warnings */
	(void) strftime(yr,280,"%Y",mytime);
	(void) strcpy(yr,&yr[strlen(yr)-2]);
	(void) strftime(buf,280, "%d/%m/", mytime);
	(void) strcat(buf,yr);
    }
    ajFmtPuts(&buf[0], strlen(buf), put, cl, flags,
	      width, precision);
}

/* @funcstatic cvt_uF *********************************************************
**
** Conversion for %F to print a file object
**
** @param [r] code [ajint] Format code specified (usually F)
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] put [int function] Standard function
** @param [r] cl [void*] Standard
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
******************************************************************************/

static void cvt_uF(ajint code, VALIST ap, int put(int c, void* cl), void* cl,
		  ajuint* flags, ajint width, ajint precision)
{
    AjPFile fil = va_arg(VA_V(ap), AjPFile);

    if (fil && fil->Name)
	ajFmtPuts(fil->Name->Ptr, fil->Name->Len, put, cl, flags,
		  width, precision);
    else
	ajFmtPuts("<null>", 6, put, cl, flags,
		  width, precision);

    return;
}


static const Except_T Fmt_Overflow = { "Formatting Overflow" };

/* @funclist Fmt_T ************************************************************
**
** Conversion functions called for each conversion code.
**
** Usually, code "x" will call "cvt_x" but there are exceptions. For example,
** floating point conversions all use cvt_f which sends everything to
** the standard C library. Also, cvt_d is used by alternative codes.
**
** @return [void]
******************************************************************************/

static Fmt_T cvt[256] =
{
 /*   0-  7 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*   8- 15 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  16- 23 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  24- 31 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  32- 39 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  40- 47 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  48- 55 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  56- 63 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  64- 71 */      0,     0,cvt_uB,     0,cvt_uD,     0,cvt_uF,     0,
 /*  72- 79 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  80- 87 */      0,     0,     0,cvt_uS,     0,     0,     0,     0,
 /*  88- 95 */  cvt_x,     0,     0,     0,     0,     0,     0,     0,
 /*  96-103 */      0,     0, cvt_b, cvt_c, cvt_d, cvt_f, cvt_f, cvt_f,
 /* 104-111 */      0,     0,     0,     0,     0,     0, cvt_d, cvt_o,
 /* 112-119 */  cvt_p,     0,     0, cvt_s,     0, cvt_u,     0,     0,
 /* 120-127 */  cvt_x,     0,     0,     0,     0,     0,     0,     0
};


/* @funclist Fmt_S ************************************************************
**
** Conversion functions called for each scan conversion code.
**
** Usually, code "x" will call "cvt_x" but there are exceptions. For example,
** floating point conversions all use cvt_f which sends everything to
** the standard C library. Also, cvt_d is used by alternative codes.
**
** @return [void]
******************************************************************************/

static Fmt_S scvt[256] =
{
 /*   0-  7 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*   8- 15 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  16- 23 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  24- 31 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  32- 39 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  40- 47 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  48- 55 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  56- 63 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  64- 71 */      0,     0,scvt_uB,     0,     0,     0,     0,     0,
 /*  72- 79 */      0,     0,     0,     0,     0,     0,     0,     0,
 /*  80- 87 */      0,     0,     0, scvt_uS,    0,     0,     0,     0,
 /*  88- 95 */ scvt_x,     0,     0,     0,     0,     0,     0,     0,
 /*  96-103 */      0,     0,scvt_b,scvt_c,scvt_d,scvt_f,scvt_f,scvt_f,
 /* 104-111 */      0,     0,     0,     0,     0,     0,scvt_d,scvt_o,
 /* 112-119 */ scvt_p,     0,     0,scvt_s,     0,scvt_u,     0,     0,
 /* 120-127 */ scvt_x,     0,scvt_z,     0,     0,     0,     0,     0
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

/* @funcstatic fmtOutC *******************************************************
**
** General output function to print a single character to a file
**
** @param [R] c [int] Character to be written
** @param [R] cl [void*] Output file
** @return [ajint] 0 on success
******************************************************************************/

static ajint fmtOutC(int c, void* cl)
{
    FILE *f = cl;

    return putc(c, f);
}

/* @funcstatic fmtInsert ****************************************************
**
** Inserts a character in a buffer, raises a Fmt_Overflow exception if
** the buffer is too small.
**
** @param [R] c [int] Character to be written
** @param [R] cl [void*] Output file
** @return [ajint] 0 on success
******************************************************************************/

static ajint fmtInsert(int c, void* cl)
{
    FmtPBuf p = cl;

    if (p->bp >= p->buf + p->size)
    {
        if (p->fixed)
	  AJRAISE(Fmt_Overflow);

	AJRESIZE(p->buf, 2*p->size);
	p->bp = p->buf + p->size;
	p->size *= 2;
    }
    *p->bp++ = ajSysItoC(c);

    return c;
}

/* @funcstatic fmtAppend ****************************************************
**
** Appends a character to a buffer, resizing it if necessary
**
** @param [R] c [ajint] Character to be written
** @param [R] cl [void*] Output file
** @return [ajint] 0 on success
******************************************************************************/

static ajint fmtAppend(ajint c, void* cl)
{
    FmtPBuf p = cl;

    if(p->bp >= p->buf + p->size)
    {
        if (p->fixed)
	  AJRAISE(Fmt_Overflow);

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
** @param [r] len [ajint] Text length.
** @param [f] put [int function] Standard function.
** @param [r] cl [void*] Standard.
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @cre attempting to write over len chars to str
**
** @@
**************************************************************************/

void ajFmtPuts (const char* str, ajint len, int put(int c, void* cl), void* cl,
		ajuint* flags, ajint width, ajint precision)
{

    (void) assert(str);
    (void) assert(len >= 0);
    (void) assert(flags);

    if(width == INT_MIN)
	width = 0;

    if(width < 0)
    {
	flags['-'] = 1;
	width = -width;
    }

    if(precision >= 0)
	flags['0'] = 0;

    if(precision >= 0 && precision < len)
	len = precision;

    if(!flags['-'])
	pad(width - len, ' ');
    {
	ajint i;
	for(i = 0; i < len; i++)
	    (void) put((unsigned char)*str++, cl);
    }

    if(flags['-'])
	pad(width - len, ' ');

    return;
}

/* @func ajFmtFmt **********************************************************
**
** formats and emits the "..." arguments according to the format string fmt
**
** @param [f] put [ajint function] Standard function.
** @param [rP] cl [void*] Standard.
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
**
** @@
**************************************************************************/

void ajFmtFmt(ajint put(ajint c, void* cl), void* cl, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(put, cl, fmt, ap);
    va_end(ap);

    return;
}

/* @func ajFmtPrint *****************************************************
**
** format and emit the "..." arguments according to fmt;writes to stdout.
**
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [void]
** @@
**************************************************************************/

void ajFmtPrint (const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, stdout, fmt, ap);
    va_end(ap);

    return;
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

void ajFmtVPrint (const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, stdout, fmt, ap);

    return;
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

void ajFmtError (const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, stderr, fmt, ap);
    va_end(ap);

    return;
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

void ajFmtVError (const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, stderr, fmt, ap);

    return;
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

void ajFmtPrintF (AjPFile file, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, file->fp, fmt, ap);
    va_end(ap);

    return;
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

void ajFmtVPrintF(AjPFile file, const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, file->fp, fmt, ap);

    return;
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

void ajFmtVPrintFp(FILE* stream, const char* fmt, va_list ap)
{
    ajFmtVfmt(fmtOutC, stream, fmt, ap);

    return;
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

void ajFmtPrintFp(FILE* stream, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    ajFmtVfmt(fmtOutC, stream, fmt, ap);
    va_end(ap);

    return;
}

/* @func ajFmtVPrintCL ****************************************************
**
** formats the "..." arguments into buf[1...size-1] according to fmt,
** appends a num character, and returns the length of buf. It is a
** c.r.e for size to be less than or equal to 0. Raises Fmt_Overflow
** if more than size-1 characters are emitted.
**
** @param [w] buf [char*] char string to be written too.
** @param [r] size [ajint] length of buffer
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list
** @return [ajint] number of characters written to buf.
** @@
**************************************************************************/

ajint ajFmtVPrintCL(char* buf, ajint size, const char* fmt, va_list ap)
{
    ajint len;

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
** @param [r] size [ajint] length of buffer
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable length argument list
**
** @return [] [ajint] number of characters written to buf.
**
** @@
**************************************************************************/

ajint ajFmtPrintCL(char* buf, ajint size, const char* fmt, ...)
{
    va_list ap;
    ajint len;

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

AjPStr ajFmtStr(const char* fmt, ...)
{
    va_list ap;
#if defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif
    ajint len = 32;
    AjPStr fnew;

    fnew = ajStrNewL (len);
    va_start(ap, fmt);

#if defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap, ap);
#endif

    fnew->Len = ajFmtVfmtStrCL(&fnew->Ptr, 0, &fnew->Res, fmt, ap);

#if defined(__PPC__) && defined(_CALL_SYSV)
	__va_copy(ap, save_ap);
#endif

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

AjPStr ajFmtPrintS (AjPStr* pthis, const char* fmt, ...)
{
    volatile AjPStr thys;
    va_list ap;

#if defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif

    va_start(ap, fmt);

    (void) ajStrModL(pthis, 32);
    thys = *pthis;

#if defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap, ap);
#endif

    thys->Len = ajFmtVfmtStrCL(&thys->Ptr, 0, &thys->Res, fmt, ap);

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
*****************************************************************************/ 

AjPStr ajFmtPrintAppS(AjPStr* pthis, const char* fmt, ...)
{
    volatile AjPStr thys;
    va_list ap;
#if defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif
    ajint len;

    va_start(ap, fmt);

    (void) ajStrModL(pthis, 32);
    thys = *pthis;

#if defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap, ap);
#endif

    len = ajFmtVfmtStrCL(&thys->Ptr, thys->Len, &thys->Res,
			      fmt, ap);

    thys->Len += len;

    va_end(ap);

    return thys;
}

/* @func ajFmtVfmtStrCL ******************************************************
**
** Same as ajFmtPrintCL but takes arguments from the list ap.
**
** @param [w] pbuf [char**] char string to be written too.
** @param [r] pos [ajint] position in buffer to start writing
** @param [U] size [ajint*] allocated size of the buffer
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list.
**
** @return [] [ajint] number of characters written to buf.
**
** @@
**************************************************************************/

ajint ajFmtVfmtStrCL(char **pbuf, ajint pos, ajint* size,
const char* fmt, va_list ap)
{
    FmtOBuf cl;

    (void) assert(*pbuf);
    (void) assert(*size > 0);
    (void) assert(fmt);

    cl.buf = *pbuf;
    cl.bp = cl.buf + pos;
    cl.size = *size;
    cl.fixed = ajFalse;

    ajFmtVfmt(fmtAppend, &cl, fmt, ap);
    (void) fmtAppend(0, &cl);

    *size = cl.size;
    *pbuf = cl.buf;

    return cl.bp - cl.buf - 1 - pos;
}

/* @func ajFmtVfmtCL *******************************************************
**
** Same as ajFmtPrintCL but takes arguments from the list ap.
**
** @param [w] buf [char*] char string to be written too.
** @param [r] size [ajint] length of buffer
** @param [r] fmt [const char*] Format string.
** @param [r] ap [va_list] Variable length argument list.
**
** @return [] [ajint] number of characters written to buf.
**
** @@
**************************************************************************/

ajint ajFmtVfmtCL(char* buf, ajint size, const char* fmt, va_list ap)
{
    FmtOBuf cl;

    (void) assert(buf);
    (void) assert(size > 0);
    (void) assert(fmt);

    cl.buf = cl.bp = buf;
    cl.size = size;
    cl.fixed = ajTrue;

    ajFmtVfmt(fmtInsert, &cl, fmt, ap);
    (void) fmtInsert(0, &cl);

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

char* ajFmtString(const char* fmt, ...)
{
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

char* ajFmtVString(const char* fmt, va_list ap)
{
    FmtOBuf cl;

    (void) assert(fmt);

    cl.size = 256;
    cl.buf = cl.bp = AJALLOC(cl.size);
    cl.fixed = ajFalse;

    ajFmtVfmt(fmtAppend, &cl, fmt, ap);
    (void) fmtAppend(0, &cl);

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

void ajFmtVfmt (int put(int c, void* cl), void* cl, const char* fmt,
		va_list ap)
{

    (void) assert(put);
    (void) assert(fmt);
    (void) assert(cl);

    while(*fmt)
    {
	if(*fmt != '%' || *++fmt == '%') /* %% just outputs '%' */
	    (void) put((unsigned char)*fmt++, cl);
	else
	{				/* we have a % - get working on the format */
	    unsigned char c;
	    ajint flags[256];
	    ajint width = INT_MIN, precision = INT_MIN;

	    (void) memset(flags, '\0', sizeof flags);

	    if(Fmt_flags)
	    {				/* look for any conversion flags */
		unsigned char c = *fmt;
		for ( ; (int)c && strchr(Fmt_flags, c); c = *++fmt)
		{
		    (void) assert(flags[(int)c] < 255);
		    flags[(int)c]++;
		}
	    }

	    if(*fmt == '*' || isdigit((int)*fmt))
	    { 
		ajint n;

		if (*fmt == '*')
		{			/* '*' width = ajint arg */
		    n = va_arg(ap, int);
		    (void) assert(n != INT_MIN);
		    fmt++;
		}
		else
		    for(n = 0; isdigit((int)*fmt); fmt++)
		    {
			ajint d = *fmt - '0';
			(void) assert(n <= (INT_MAX - d)/10);
			n = 10*n + d;
		    }
		width = n;
	    }

	    if(*fmt == '.' && (*++fmt == '*' || isdigit((int)*fmt)))
	    {
		ajint n; 

		if (*fmt == '*')
		{			/* '*' precision = ajint arg */
		    n = va_arg(ap, int);
		    (void) assert(n != INT_MIN);
		    fmt++;
		}
		else
		    for(n = 0; isdigit((int)*fmt); fmt++)
		    {
			ajint d = *fmt - '0';
			(void) assert(n <= (INT_MAX - d)/10);
			n = 10*n + d;
		    }
		precision = n;
	    }

	    if (*fmt == 'l' || *fmt == 'L'|| *fmt == 'h')
	    {				/* size modifiers */
		(void) assert(flags[(int)*fmt] < 255); /* store as flags - */
		/* values do not clash */
		flags[(int)*fmt]++;
		fmt++;
	    }
	    c = *fmt++;		/* finally, next character is the code */

	    /* Calling funclist Fmt_T() */

	    (void) assert(cvt[(int)c]);	/* we need a defined routine */
	    (*cvt[(int)c])(c, VA_P(ap), put, cl, (ajuint *)flags, width,
			   precision);
	}
    }

    return;
}

/* @func ajFmtRegister *************************************************
**
** Registers 'newcvt' as the conversion routine for format code 'code'
**
** @param [r] code [ajint] value of char to be replaced
** @param [f] newcvt [Fmt_T] new routine for conversion
**
** @return [Fmt_T] old value
** @@
************************************************************************/

Fmt_T ajFmtRegister(ajint code, Fmt_T newcvt)
{
    Fmt_T old;

    (void) assert(0 < code
		  && code < (ajint)(sizeof (cvt)/sizeof (cvt[0])));
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
** @param [r] len [ajint] Text length.
** @param [f] put [int function] Standard function.
** @param [r] cl [void*] Standard.
** @param [r] flags [ajuint*] Flags (after the %)
** @param [r] width [ajint] Width (before the dot)
** @param [r] precision [ajint] Precision (after the dot)
** @return [void]
** @@
*****************************************************************************/

void ajFmtPutd(const char* str, ajint len, int put(int c, void* cl), void* cl,
	       ajuint* flags, ajint width, ajint precision)
{
    ajint sign;

    (void) assert(str);
    (void) assert(len >= 0);
    (void) assert(flags);

    if(width == INT_MIN)
	width = 0;

    if(width < 0)
    {
	flags['-'] = 1;
	width = -width;
    }

    /*
       if(precision >= 0)
       flags['0'] = 0;
       */

    if(len > 0 && (*str == '-' || *str == '+'))
    {
	sign = *str++;
	len--;
    }
    else if(flags['+'])
	sign = '+';
    else if(flags[' '])
	sign = ' ';
    else
	sign = 0;

    {
	ajint n;
	ajint j=0;

	if(precision < 0)
	    precision = 1;

	if(len < precision)
	    n = precision;
	else if(precision == 0 && len == 1 && str[0] == '0')
	    n = 0;
	else
	    n = len;

	if (sign)
	    n++;

	if (flags['#'] && flags['0'])
	{				/* make space for the padding */
	    if (*str == '0' && *(str+1) == 'x')
	    {
		(void) put((unsigned char)*str++, cl);
		(void) put((unsigned char)*str++, cl);
		j += 2;
	    }
	}

	if(flags['-'])
	{
	    if (sign)
		(void) put(sign, cl);
	}
	else if(flags['0'])
	{
	    if (sign)
		(void) put(sign, cl);
	    pad(width - n, '0');
	}
	else
	{
	    pad(width - n, ' ');
	    if (sign)
		(void) put(sign, cl);
	}

	pad(precision - len, '0');	/* pad after end */
	{
	    ajint i;

	    for (i = j; i < len; i++)
		(void) put((unsigned char)*str++, cl);
	}

	if (flags['-'])
	    pad(width - n, ' ');
    }

    return;
}

/* @func ajFmtPrintSplit *****************************************************
**
** Block and print a string. String is split at given delimiters
**
** @param [w] outf [AjPFile] output stream
** @param [r] str [AjPStr] text to write
** @param [r] prefix [char *] prefix string
** @param [r] len [ajint] maximum span
** @param [r] delim [char *] delimiter string
** @return [void]
** @@
*****************************************************************************/

void ajFmtPrintSplit(AjPFile outf, AjPStr str, char *prefix, ajint len,
		     char *delim)
{
    AjPStrTok handle=NULL;
    AjPStr token = NULL;
    AjPStr tmp   = NULL;
    AjPStr tmp2  = NULL;

    ajint    n = 0;
    ajint    l = 0;
    ajint    c = 0;

    token = ajStrNew();
    tmp   = ajStrNewC("");
    tmp2  = ajStrNew();

    handle = ajStrTokenInit(str,delim);

    while(ajStrToken(&token,&handle,NULL))
    {
	if(!c)
	    ajFmtPrintF(outf,"%s",prefix);

	if((l=n+ajStrLen(token)) < len)
	{
	    if(c++)
		ajStrAppC(&tmp," ");
	    ajStrApp(&tmp,token);
	    if(c!=1)
		n = ++l;
	    else
		n = l;
	}
	else
	{
	    ajFmtPrintF(outf,"%S\n",tmp);
	    ajStrAssS(&tmp,token);
	    ajStrAppC(&tmp," ");
	    n = ajStrLen(token) + 1;
	    c = 0;
	}
    }

    if(c)
	ajFmtPrintF(outf,"%S\n",tmp);
    else
    {
	n = ajStrLen(tmp);
	ajStrAssSub(&tmp2,tmp,0,n-2);
	ajFmtPrintF(outf,"%s%S\n",prefix,tmp2);
    }


    ajStrTokenClear(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);
    ajStrDel(&tmp2);

    return;
}


/* @func ajFmtScanS  *****************************************************
**
** Scan a string according to fmt and load the ... variable pointers
** Like C function sscanf.
**
** @param [r] thys [AjPStr] String.
** @param [r] fmt [const char*] Format string.
** @param [v] [...] Variable length argument list
** @return [ajint] number of successful conversions
** @@
**************************************************************************/

ajint ajFmtScanS(AjPStr thys, const char* fmt, ...)
{
    va_list ap;
    ajint   n;
#if defined(__PPC__) && defined(_CALL_SYSV)
    va_list save_ap;
#endif

    va_start(ap, fmt);

#if defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(save_ap,ap);
#endif

    n = ajFmtVscan(thys->Ptr,fmt,ap);
    
#if defined(__PPC__) && defined(_CALL_SYSV)
    __va_copy(ap,save_ap);
#endif


    va_end(ap);

    return n;
}


/* @funcstatic ajFmtVscan *****************************************************
**
** Scan a string according to fmt and load the va_list variable pointers
**
** @param [r] thys [char*] String.
** @param [r] fmt [const char*] Format string.
** @param [w] ap [va_list] Variable length argument list
** @return [ajint] number of successful conversions
** @@
**************************************************************************/
static ajint ajFmtVscan(char *thys,const char *fmt,va_list ap)
{
    ajint n;
    char *p;
    char *q;
    static char *wspace=" \n\t";
    AjBool convert=ajTrue;
    AjBool ok=ajTrue;
    ajint width=0;
    ajint v=0;
    ajint d=0;

    n=0;
    p = thys;
    q = (char *)fmt;
    
    while(*p && *q)
    {
	/* Ignore all whitespace */
	if(c_isin((ajint)*p,wspace))
	{
	    ++p;
	    continue;
	}
	if(c_isin((ajint)*q,wspace))
	{
	    ++q;
	    continue;
	}
	
	/* If *q isn't '%' then it must match *p */
	if(*q != '%')
	{
	    if(*q!=*p)
		break;
	    else
	    {
		++p;
		++q;
		continue;
	    }
	}
	
	/* Check for %% */
	if(*(++q)=='%')
	{
	    if(*p!='%')
		break;
	    else
	    {
		++p;
		++q;
		continue;
	    }
	}
	

	/*
	 *  *p now points to a string character to be matched
	 *  *q points to first character after fmt '%'
	 */

	/* Check for %* format */
	convert = ajTrue;
	if(*q=='*')
	{
	    ++q;
	    convert = ajFalse;
	}
	
	/* If *q is a numeral then calculate the width else set to INT_MIN */
	if(isdigit((int)*q))
	{
	    for(v=0;isdigit((int)*q);++q)
	    {
		d = *q - '0';
		v = 10*v + d;
	    }
	    width = v;
	}
	else
	    width = INT_MIN;

	/* Just ignore size modifier for now */
	if (*q== 'l' || *q== 'L'|| *q== 'h')
	    ++q;

	/* *q is the conversion function to call */
	ok = ajTrue;

	/* Calling funclist Fmt_S() */
	(void) assert(scvt[(int)*q]);
	(*scvt[(int)*q])(q,&p,VA_P(ap),width,convert,&ok);

	if(!ok)
	    break;
	
	if(convert)
	    ++n;

	/*
	 *  p will already have been incremented by the convert function
	 *  so just increment q
	 */
	++q;
	

    }


    return n;
}



/* @funcstatic scvt_uS ********************************************************
**
** Conversion for %S to load a string
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_uS(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    AjPStr *val = NULL;
    static char *wspace=" \n\t";
    ajint c=0;

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width;++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (AjPStr *) va_arg(VA_V(ap), AjPStr *);
	    ajStrAssSubC(val,p,0,q-p-1);
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_d ********************************************************
**
** Conversion for %d to load an integer
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_d(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    long *val=NULL;
    ajlong *hval=NULL;
    static char *wspace=" \n\t";
    static char *dig="+-0123456789";
    ajint c=0;
    static AjPStr t=NULL;
    long  n=0;
    ajlong hn=0;
    char  flag=*(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (long *) va_arg(VA_V(ap), long *);
	    else
		hval = (ajlong *) va_arg(VA_V(ap), ajlong *);
	    ajStrAssSubC(&t,p,0,q-p-1);
	    if(flag!='L')
		sscanf(ajStrStr(t),"%ld",&n);

	    else
	    {
#if defined(HAVE64)    
		hn = sc_long(ajStrStr(t));
#else
		val = hval;
		sscanf(ajStrStr(t),"%ld",&n);
		hn = n;
		ajDebug("Warning: Use of %%Ld on a 32 bit model");
#endif
	    }
	    if(flag=='h')
		*(short*)val = (short)n;
	    else if(flag=='l')
		*(long*)val = n;
	    else if(flag=='L')
		*(ajlong*)hval = hn;
	    else
		*(int*)val = (int)n;
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_x ********************************************************
**
** Conversion for %x to load an unsigned hexadecimal
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_x(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    unsigned long *val=NULL;
    ajulong *hval=NULL;
    static char *wspace=" \n\t";
    static char *dig="0123456789abcdefABCDEFx";
    ajint c=0;
    static AjPStr t=NULL;
    unsigned long  n=0;
    ajulong hn=0;
    char  flag=*(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (unsigned long *) va_arg(VA_V(ap), unsigned long *);
	    else
		hval = (ajulong *) va_arg(VA_V(ap), ajulong *);
	    ajStrAssSubC(&t,p,0,q-p-1);
	    if(flag!='L')
	    {
		if(sscanf(ajStrStr(t),"%lx",&n)!=1)
		    return;
	    }
	    else
	    {
#if defined(HAVE64)
		hn = sc_hex(ajStrStr(t));
#else
		val = hval;
		if(sscanf(ajStrStr(t),"%lx",&n)!=1)
		    return;
		hn = n;
		ajDebug("Warning: Use of %%Lx on a 32 bit model");
#endif
	    }

	    if(flag=='h')
		*(unsigned short*)val = (unsigned short)n;
	    else if(flag=='l')
		*(unsigned long*)val = n;
	    else if(flag=='L')
		*(ajulong*)hval = hn;
	    else
		*(unsigned int*)val = (unsigned int)n;
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_f ********************************************************
**
** Conversion for %f to load a float/double
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_f(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    double *val;
    float  *fval;
    static char *wspace=" \n\t";
    static char *dig="+-0123456789.eE";
    ajint c=0;
    static AjPStr t=NULL;
    double  n=(double)0.;
    float   fn = 0.;
    
    char  flag=*(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    ajStrAssSubC(&t,p,0,q-p-1);
	    if(flag=='l')
	    {
		val = (double*) va_arg(VA_V(ap), double *);
		if(sscanf(ajStrStr(t),"%lf",&n)!=1)
		    return;
		*(double *)val = n;
	    }
	    else
	    {
		fval = (float*) va_arg(VA_V(ap), float *);
		if(sscanf(ajStrStr(t),"%f",&fn)!=1)
		    return;
		*(float *)fval = fn;
	    }
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_s ********************************************************
**
** Conversion for %s to load a char *
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_s(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    char *val = NULL;
    static char *wspace=" \n\t";
    ajint c=0;
    static AjPStr t=NULL;

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width;++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (char *) va_arg(VA_V(ap), char *);
	    ajStrAssSubC(&t,p,0,q-p-1);
	    strcpy(val,ajStrStr(t));
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_o ********************************************************
**
** Conversion for %o to load an unsigned octal
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_o(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    unsigned long *val=NULL;
    ajulong  *hval=NULL;
    static char *wspace=" \n\t";
    static char *dig="01234567";
    ajint c=0;
    static AjPStr t=NULL;
    unsigned long  n=0;
    ajulong hn=0;
    char  flag=*(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (unsigned long *) va_arg(VA_V(ap), unsigned long *);
	    else
		hval = (ajulong *)  va_arg(VA_V(ap), ajulong *);
	    ajStrAssSubC(&t,p,0,q-p-1);

	    if(flag!='L')
	    {
		if(sscanf(ajStrStr(t),"%lo",&n)!=1)
		    return;
	    }
	    else
	    {
#if defined(HAVE64)
		hn = sc_octal(ajStrStr(t));
#else
		val = hval;
		if(sscanf(ajStrStr(t),"%lo",&n)!=1)
		    return;
		hn = n;
		ajDebug("Warning: Use of %%Lo on a 32 bit model");
#endif
	    }

	    if(flag=='h')
		*(unsigned short*)val = (unsigned short)n;
	    else if(flag=='l')
		*(unsigned long*)val = n;
	    else if(flag=='L')
		*(ajulong*)hval = hn;
	    else
		*(unsigned int*)val = (unsigned int)n;
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_u ********************************************************
**
** Conversion for %u to load an unsigned integer
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_u(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    unsigned long *val=NULL;
    ajulong *hval=NULL;
    static char *wspace=" \n\t";
    static char *dig="+0123456789";
    ajint c=0;
    static AjPStr t=NULL;
    unsigned long  n=0;
    ajulong hn=0;
    char  flag=*(fmt-1);

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    if(flag!='L')
		val = (unsigned long *) va_arg(VA_V(ap), unsigned long *);
	    else
		hval = (ajulong *)  va_arg(VA_V(ap), ajulong *);
	    ajStrAssSubC(&t,p,0,q-p-1);

	    if(flag!='L')
	    {
		if(sscanf(ajStrStr(t),"%lu",&n)!=1)
		    return;
	    }
	    else
	    {
#if defined(HAVE64)
		hn = sc_ulong(ajStrStr(t));
#else
		val = hval;
		if(sscanf(ajStrStr(t),"%lu",&n)!=1)
		    return;
		hn = n;
		ajDebug("Warning: Use of %%Lu on a 32 bit model");
#endif
	    }


	    if(flag=='h')
		*(unsigned short*)val = (unsigned short)n;
	    else if(flag=='l')
		*(unsigned long*)val = n;
	    else if(flag=='L')
		*(ajulong*)hval = hn;
	    else
		*(unsigned int*)val = (unsigned int)n;
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_p ********************************************************
**
** Conversion for %p to load a pointer of type void * as hexadecimal
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_p(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    void *val;
    static char *wspace=" \n\t";
    static char *dig="0123456789abcdefABCDEFx";
    ajint c=0;
    static AjPStr t=NULL;
    unsigned long  n=0;

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (void *) va_arg(VA_V(ap), void *);
	    ajStrAssSubC(&t,p,0,q-p-1);
	    if(sscanf(ajStrStr(t),"%lx",&n)!=1)
		return;
	    val = (void *)n;
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_uB *******************************************************
**
** Conversion for %B to load a boolean (integer or YyNnTtFf)
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_uB(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q = NULL;
    AjBool *val=NULL;
    static char *wspace=" \n\t";
    static char *dig="+-0123456789";
    static char *tr="YyTt";
    static char *fa="NnFf";
    ajint c=0;
    static AjPStr t=NULL;
    AjBool n=ajFalse;

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    q=p;

    if(!strncmp(q,"Yes",3))
    {
	*pos = q+3;
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);	    
	    *(AjBool*)val = ajTrue;
	}
	*ok = ajTrue;
	return;
    }

    if(!strncmp(q,"No",2))
    {
	*pos = q+2;
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);	    
	    *(AjBool*)val = ajFalse;
	}
	*ok = ajTrue;
	return;
    }
	

    if(c_isin((int)*q,tr) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajTrue;
	}
	*pos = ++q;
	*ok = ajTrue;
	return;
    }

    if(c_isin((int)*q,fa) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajFalse;
	}
	*pos = ++q;
	*ok = ajTrue;
	return;
    }
    

    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width &&
	    c_isin((int)*q,dig);++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace) && c_isin((int)*q,dig);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    ajStrAssSubC(&t,p,0,q-p-1);
	    sscanf(ajStrStr(t),"%d",&n);
	    if(n)
		*(AjBool*)val = ajTrue;
	    else
		*(AjBool*)val = ajFalse;
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


/* @funcstatic scvt_c ********************************************************
**
** Conversion for %c to load a character
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_c(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q = NULL;
    char *val=NULL;
    char n='\0';
    
    q=p;
    
    *ok = ajFalse;

    if(!(width==INT_MIN || width==1))
	return;

    if(convert)
    {
	n = *q;
	val = (char *) va_arg(VA_V(ap), char *);
	*val = n;
    }
    
	
    *pos = ++q;
    *ok  = ajTrue;

    return;
}


/* @funcstatic scvt_b ********************************************************
**
** Conversion for %B to load a boolean (YyNnTtFf)
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_b(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q = NULL;
    AjBool *val=NULL;
    static char *tr="YyTt";
    static char *fa="NnFf";

    *ok = ajFalse;
    
    q=p;

    if(c_isin((int)*q,tr) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajTrue;
	}
	*pos = ++q;
	*ok = ajTrue;
	return;
    }

    if(c_isin((int)*q,fa) && (width==INT_MIN || width==1))
    {
	if(convert)
	{
	    val = (AjBool *) va_arg(VA_V(ap), AjBool *);
	    *(AjBool*)val = ajFalse;
	}
	*pos = ++q;
	*ok = ajTrue;
    }
    
    return;
}


/* @funcstatic scvt_z ********************************************************
**
** Conversion for %z to load a char **
**
** @param [r] fmt [char*] Format string at conv char posn
** @param [w] pos [char**] Input string current position
** @param [r] ap [VALIST] Original arguments at current position
** @param [r] width [ajint] Width
** @param [r] convert [AjBool] ajFalse if %* was specified
** @param [w] ok [AjBool*] set for a successful conversion
** @return [void]
** @@
******************************************************************************/

static void scvt_z(char *fmt, char **pos, VALIST ap, ajint width,
		   AjBool convert, AjBool *ok)
{
    char *p = *pos;
    char *q;
    char **val = NULL;
    static char *wspace=" \n\t";
    ajint c=0;
    static AjPStr t=NULL;

    if(!t)
	t = ajStrNew();

    *ok = ajFalse;
    
    if(width!=INT_MIN)
	for(q=p;*q && c_notin((int)*q,wspace) && c<width;++q,++c);
    else
	for(q=p;*q && c_notin((int)*q,wspace);++q);

    if(q-p)
    {
	if(convert)
	{
	    val = (char **) va_arg(VA_V(ap), char **);
	    ajStrAssSubC(&t,p,0,q-p-1);
	    if(!*val)
		*val = ajCharNewL(ajStrLen(t)+1);
	    strcpy(*val,ajStrStr(t));
	}
	
	*pos = q;
	*ok = ajTrue;
    }

    return;
}


#if defined(HAVE64)
/* @funcstatic sc_long ***************************************************
**
** Load a 64 bit long from a char*
**
** @param [r] str [const char*] long number
** @return [ajlong] result
** @@
******************************************************************************/
static ajlong sc_long(const char *str)
{
    ajlong v=0;
    ajint d;
    char *p;
    char c;
    
    p = (char *)str;
    
    while((c=*(p++)))
    {
	d = c - '0';
	v = (ajlong)10*v + (ajlong)d;
    }
    
    return v;
}


/* @funcstatic sc_ulong ***************************************************
**
** Load a 64 bit unsigned long from a char*
**
** @param [r] str [const char*] long number
** @return [ajulong] result
** @@
******************************************************************************/
static ajulong sc_ulong(const char *str)
{
    ajulong v=0;
    ajint d;
    char *p;
    char c;
    
    p = (char *)str;
    
    while((c=*(p++)))
    {
	d = c - '0';
	v = (ajulong)10*v + (ajulong)d;
    }
    
    return v;
}

/* @funcstatic sc_hex ***************************************************
**
** Load a 64 bit unsigned long from a char* hexadecimal
**
** @param [r] str [const char*] long hex number
** @return [ajulong] result
** @@
******************************************************************************/

static ajulong sc_hex(const char *str)
{
    ajulong v=0;
    ajint d;
    char *p;
    char c;
    
    p = (char *)str+2;
    
    while((c=toupper((int)*(p++))))
    {
	if(c>='0' && c<='9')
	    d = c - '0';
	else
	    d = c - 'A' + 10;
	v = (ajulong)16*v + (ajulong)d;
    }
    
    return v;
}


/* @funcstatic sc_octal ***************************************************
**
** Load a 64 bit unsigned long from a char* octal
**
** @param [r] str [const char*] long hex number
** @return [ajulong] result
** @@
******************************************************************************/

static ajulong sc_octal(const char *str)
{
    ajulong v=0;
    ajint d;
    char *p;
    char c;
    
    p = (char *)str+1;
    
    while((c=toupper((int)*(p++))))
    {
	d = c - '0';
	v = (ajulong)8*v + (ajulong)d;
    }
    
    return v;
}
#endif

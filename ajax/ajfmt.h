#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfmt_h
#define ajfmt_h

#include <stdarg.h>
#include <stdio.h>
#include "ajexcept.h"

#if defined(__amd64__) || defined(__PPC__) && defined(_CALL_SYSV)
#define VALIST va_list
#define VA_P(x) (x)
#define VA_V(x) (x)
#else
#define VALIST va_list*
#define VA_P(x) (&x)
#define VA_V(x) (*x)
#endif

typedef void (*Fmt_T) (ajint code, VALIST ap,
		       int put(int c, void *cl), void *cl,
		       ajuint flags[256], ajint width, ajint precision);

typedef void (*Fmt_S) (const char *fmt, char **pos, VALIST ap, int width,
		       AjBool convert, AjBool *ok);

void   ajFmtFmt (int put(int c, void *cl), void *cl,
		 const char *fmt, ...);
void   ajFmtVfmt (int put(int c, void *cl), void *cl,
		  const char *fmt, va_list ap);
void   ajFmtError (const char *fmt, ...);
void   ajFmtVError (const char *fmt, va_list ap);
void   ajFmtPrint (const char *fmt, ...);
void   ajFmtVPrint (const char *fmt, va_list ap);
void   ajFmtPrintFp (FILE *stream,
		     const char *fmt, ...);
void   ajFmtVPrintFp (FILE *stream,
		      const char *fmt, va_list ap);
void   ajFmtPrintF (const AjPFile file,
		    const char *fmt, ...);
void   ajFmtVPrintF (const AjPFile file,
		     const char *fmt, va_list ap);
ajint  ajFmtPrintCL (char *buf, ajint size,
		     const char *fmt, ...);
ajint  ajFmtVPrintCL (char *buf, ajint size,
		      const char *fmt, va_list ap);
void   ajFmtPrintSplit(const AjPFile outf, const AjPStr str,
		       const char *prefix, ajint len,
		       const char *delim);
ajint  ajFmtVFmtS(char *buf, ajint size,
		  const char *fmt, va_list ap);
char*  ajFmtString (const char *fmt, ...);
char*  ajFmtVString(const char *fmt, va_list ap);
Fmt_T  ajFmtRegister(ajint code, Fmt_T cvt);
void   ajFmtPutd (const char *str, ajint len,
		  int put(int c, void *cl), void *cl,
		  ajuint flags[256], ajint width, ajint precision);
void   ajFmtPuts (const char *str, ajint len,
		  int put(int c, void *cl), void *cl,
		  ajuint flags[256], ajint width, ajint precision);
AjPStr ajFmtStr (const char *fmt, ...);
AjPStr ajFmtPrintS (AjPStr *pthis, const char *fmt, ...) ;
AjPStr ajFmtVPrintS (AjPStr *pthis, const char *fmt, va_list ap) ;
AjPStr ajFmtPrintAppS (AjPStr *pthis, const char *fmt, ...) ;
ajint  ajFmtVfmtCL (char* buf, ajint size, const char* fmt,
		    va_list ap);
ajint  ajFmtVfmtStrCL (char** buf, ajint pos, ajint *size,
		       const char* fmt, va_list ap);

ajint  ajFmtScanS (const AjPStr thys, const char* fmt, ...);
ajint  ajFmtScanC (const char* thys, const char* fmt, ...);

#endif

#ifdef __cplusplus
}
#endif

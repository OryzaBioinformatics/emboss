#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfmt_h
#define ajfmt_h

#include <stdarg.h>
#include <stdio.h>
#include "ajexcept.h"

#if defined(__PPC__) && defined(_CALL_SYSV)
#define VALIST va_list
#define VA_P(x) (x)
#define VA_V(x) (x)
#else
#define VALIST va_list*
#define VA_P(x) (&x)
#define VA_V(x) (*x)
#endif

typedef void (*Fmt_T)(ajint code, VALIST ap,
	int put(int c, void *cl), void *cl,
	ajuint flags[256], ajint width, ajint precision);

typedef void (*Fmt_S)(char *fmt, char **pos, VALIST ap, int width,
		      AjBool convert, AjBool *ok);

extern void ajFmtFmt (int put(int c, void *cl), void *cl,
	const char *fmt, ...);
extern void ajFmtVfmt(int put(int c, void *cl), void *cl,
	const char *fmt, va_list ap);
extern void ajFmtError (const char *fmt, ...);
extern void ajFmtVError (const char *fmt, va_list ap);
extern void ajFmtPrint (const char *fmt, ...);
extern void ajFmtVPrint (const char *fmt, va_list ap);
extern void ajFmtPrintFp(FILE *stream,
	const char *fmt, ...);
extern void ajFmtVPrintFp(FILE *stream,
	const char *fmt, va_list ap);
extern void ajFmtPrintF(AjPFile file,
	const char *fmt, ...);
extern void ajFmtVPrintF(AjPFile file,
	const char *fmt, va_list ap);
extern ajint ajFmtPrintCL (char *buf, ajint size,
	const char *fmt, ...);
extern ajint ajFmtVPrintCL (char *buf, ajint size,
	const char *fmt, va_list ap);
extern void ajFmtPrintSplit(AjPFile outf, AjPStr str, char *prefix, ajint len,
			    char *delim);
extern ajint ajFmtVFmtS(char *buf, ajint size,
	const char *fmt, va_list ap);
extern char *ajFmtString (const char *fmt, ...);
extern char *ajFmtVString(const char *fmt, va_list ap);
extern Fmt_T ajFmtRegister(ajint code, Fmt_T cvt);
extern void ajFmtPutd(const char *str, ajint len,
	int put(int c, void *cl), void *cl,
	ajuint flags[256], ajint width, ajint precision);
extern void ajFmtPuts(const char *str, ajint len,
	int put(int c, void *cl), void *cl,
	ajuint flags[256], ajint width, ajint precision);
extern AjPStr ajFmtStr(const char *fmt, ...);
extern AjPStr ajFmtPrintS(AjPStr *pthis, const char *fmt, ...) ;
extern AjPStr ajFmtPrintAppS(AjPStr *pthis, const char *fmt, ...) ;
ajint ajFmtVfmtCL(char* buf, ajint size, const char* fmt,
	va_list ap);

ajint ajFmtScanS(AjPStr thys, const char* fmt, ...);


#endif

#ifdef __cplusplus
}
#endif

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

typedef void (*Fmt_T)(int code, VALIST ap,
	int put(int c, void *cl), void *cl,
	unsigned int flags[256], int width, int precision);

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
extern int ajFmtPrintCL (char *buf, int size,
	const char *fmt, ...);
extern int ajFmtVPrintCL (char *buf, int size,
	const char *fmt, va_list ap);
extern int ajFmtVFmtS(char *buf, int size,
	const char *fmt, va_list ap);
extern char *ajFmtString (const char *fmt, ...);
extern char *ajFmtVString(const char *fmt, va_list ap);
extern Fmt_T ajFmtRegister(int code, Fmt_T cvt);
extern void ajFmtPutd(const char *str, int len,
	int put(int c, void *cl), void *cl,
	unsigned int flags[256], int width, int precision);
extern void ajFmtPuts(const char *str, int len,
	int put(int c, void *cl), void *cl,
	unsigned int flags[256], int width, int precision);
extern AjPStr ajFmtStr(const char *fmt, ...);
extern AjPStr ajFmtPrintS(AjPStr *pthis, const char *fmt, ...) ;
extern AjPStr ajFmtPrintAppS(AjPStr *pthis, const char *fmt, ...) ;
int ajFmtVfmtCL(char* buf, int size, const char* fmt,
	va_list ap);
#endif

#ifdef __cplusplus
}
#endif

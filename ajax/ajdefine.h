#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdefine_h
#define ajdefine_h

#include "ajarch.h"

#define NPOS (size_t) (-1)
/* const size_t NPOS = (size_t) (-1);*/ /* maximum size_t value */

typedef void fvoid_t(void);		/* void function type */

enum capacity {default_size, reserve};

/* @data AjBool ***************************************************************
**
** Boolean data type
**
** Used to store true (ajTrue) and false (ajFalse) values.
**
** ajFalse is defined as zero, and the data type is equivalent to "ajint".
**
** For definitions, macros AJTRUE and AJFALSE are also defined.
**
** On output, conversion code "%b" writes "Y" or "N"
** while conversion code "%B" writes "Yes" or "No".
**
** @@
******************************************************************************/

typedef ajint AjBool;

/* @data AjStatus *************************************************************
**
** Status code returned with bit fields.
**
** Intended as a general return code for functions, but so far only
** used by ajAcdInit because in most cases AjBool is enough.
**
** @@
******************************************************************************/

typedef ajint AjStatus;

typedef ajint AjEnum;
typedef ajint AjMask;

typedef ajint AjInt4;		/* 4 bytes integer */

#define AJAXLONGDOUBLE double

#define AJBOOL(b) (b ? "TRUE" : "FALSE")
static const ajint ajFltDig = 3;

static const ajint ajFalse = 0;
static const ajint ajTrue = 1;
static const ajint ajStatusOK = 0;
static const ajint ajStatusInfo = 1;
static const ajint ajStatusWarn = 2;
static const ajint ajStatusError = 4;
static const ajint ajStatusFatal = 8;

#define AJFALSE 0
#define AJTRUE 1

#ifdef commentedout
#define ajFalse 0
#define ajTrue 1
#define ajStatusOK 0
#define ajStatusInfo 1
#define ajStatusWarn 2
#define ajStatusError 4
#define ajStatusFatal 8
#endif

#define CASE2(a,b) ((a << 8) + b)
#define CASE3(a,b,c) ((a << 16) + (b << 8) + c)
#define CASE4(a,b,c,d) ((a << 24) + (b << 16) + (c << 8) + d)

#define STRCASE2(a) ((a[0] << 8) + a[1])

#endif

#ifdef __cplusplus
}
#endif

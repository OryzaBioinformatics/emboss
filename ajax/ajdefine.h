#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdefine_h
#define ajdefine_h

#define NPOS (size_t) (-1)
/* const size_t NPOS = (size_t) (-1);*/ /* maximum size_t value */

typedef void fvoid_t(void);		/* void function type */

enum capacity {default_size, reserve};

/* @data AjBool *******************************************************
**
** Boolean data type
**
** Used to store true (ajTrue) and false (ajFalse) values.
**
** ajFalse is defined as zero, and the data type is equivalent to "int".
**
** For definitions, macros AJTRUE and AJFALSE are also defined.
**
** On output, conversion code "%b" writes "Y" or "N"
** while conversion code "%B" writes "Yes" or "No".
**
** @@
******************************************************************************/

typedef int AjBool;

/* @data AjDate *******************************************************
**
** Date/time data type
**
** Used to store date/time values. Equivalent to "int".
**
** On output, conversion code "%D" writes the date.
** @@
******************************************************************************/

typedef int AjDate;

/* @data AjStatus *******************************************************
**
** Status code returned with bit fields.
**
** Intended as a general return code for functions, but so far only
** used by ajAcdInit because in most cases AjBool is enough.
**
** @@
******************************************************************************/

typedef int AjStatus;

typedef int AjEnum;
typedef int AjMask;

typedef int AjInt4;		/* 4 bytes integer */

#define AJAXLONGDOUBLE double

#define AJBOOL(b) (b ? "TRUE" : "FALSE")
static const int ajFltDig = 3;

static const int ajFalse = 0;
static const int ajTrue = 1;
static const int ajStatusOK = 0;
static const int ajStatusInfo = 1;
static const int ajStatusWarn = 2;
static const int ajStatusError = 4;
static const int ajStatusFatal = 8;

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

#endif

#ifdef __cplusplus
}
#endif

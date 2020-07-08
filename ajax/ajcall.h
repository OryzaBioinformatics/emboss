#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajcall_h
#define ajcall_h

#include "ajax.h"
typedef void* (*CallFunc)(char *name, va_list args);

void callRegister(char *name, CallFunc func);

void* call(char *name, ...);

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajcall_h
#define ajcall_h

#include "ajax.h"
typedef void* (*CallFunc)(const char *name, va_list args);

void callRegister(const char *name, CallFunc func);

void* call(const char *name, ...);

#endif

#ifdef __cplusplus
}
#endif

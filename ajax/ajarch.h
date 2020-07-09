#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajarch_h
#define ajarch_h


#include <sys/types.h>
#include <stdio.h>

#if defined(AJ_Linux64) || defined(AJ_Solaris64) || defined(AJ_IRIX64) \
   || defined(AJ_OSF164)
#define HAVE64
#endif

#if !defined(AJ_LinuxLF) && !defined(AJ_SolarisLF) && !defined(AJ_IRIXLF)
typedef int ajint;
typedef long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long ajulong;
#endif


#ifdef AJ_LinuxLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello64(a)
#define fseek(a,b,c) fseeko64(a,b,c)
#endif


#ifdef AJ_SolarisLF
#define HAVE64
typedef int ajint;
typedef long long ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long long ajulong;
#define ftell(a) ftello(a)
#define fseek(a,b,c) fseeko(a,b,c)
#endif

#ifdef AJ_IRIXLF
#define HAVE64
typedef int ajint;
typedef off64_t ajlong;
typedef unsigned int ajuint;
typedef short ajshort;
typedef unsigned short ajushort;
typedef unsigned long ajulong;
#define ftell(a) ftell64(a)
#define fseek(a,b,c) fseek64(a,b,c)
#endif



#endif

#ifdef __cplusplus
}
#endif

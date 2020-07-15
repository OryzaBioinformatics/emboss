#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajsys_h
#define ajsys_h

#include "ajax.h"
#include <sys/types.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef __VMS
#ifndef WIN32
#include <sys/param.h>
#endif
#endif
#include <sys/stat.h>




/*
** Prototype definitions
*/

AjBool        ajSysArglist (const AjPStr cmdline, char** pgm, char*** arglist);
void          ajSysArgListFree (char*** arglist);
void          ajSysBasename(AjPStr *filename);
void          ajSysCanon(AjBool state);
void          ajSysExit(void);
char          ajSysItoC(ajint v);
unsigned char ajSysItoUC(ajint v);
AjBool        ajSysIsDirectory(const char *s);
AjBool        ajSysIsRegular(const char *s);
FILE         *ajSysFdopen(ajint filedes, const char *mode);
void          ajSystem(const AjPStr cl);
void          ajSystemEnv(const AjPStr cl, char * const env[]);
char         *ajSysStrdup(const char *s);
AjBool        ajSysUnlink(const AjPStr s);
AjBool        ajSysWhich(AjPStr *exefile);
AjBool        ajSysWhichEnv(AjPStr *exefile, char * const env[]);
char         *ajSysStrtok(const char *s, const char *t);
char         *ajSysStrtokR(char *s, const char *t, char **ptrptr,
			    AjPStr *buf);
char         *ajSysFgets(char *buf, int size, FILE *fp);
FILE         *ajSysFopen(const char *name, const char *flags);

/*
** End of prototype definitions
*/


/*
** S_IFREG is non-ANSI therefore define it here
** At least keeps all the very dirty stuff in one place
*/

#ifndef S_IFREG
#define S_IFREG 0100000
#endif
#ifndef S_IFDIR
#define S_IFDIR 0x4000
#endif

#ifndef WIN32
#define AJ_FILE_REG S_IFREG
#define AJ_FILE_DIR S_IFDIR
#else
#define AJ_FILE_REG _S_IFREG
#define AJ_FILE_DIR _S_IFDIR
#endif

#endif

#ifdef __cplusplus
}
#endif

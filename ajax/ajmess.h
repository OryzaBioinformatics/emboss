#ifdef __cplusplus
extern "C"
{
#endif

/*  File: ajmess.h
 *  Author: Richard Durbin (rd@mrc-lmb.cam.ac.uk) and Ed Griffiths.
 *  as part of the ACEDB package (messubs.h)
 *  Modified: by I Longden for the EMBOSS package.
 */

#ifndef ajmess_h
#define ajmess_h

#include <stdarg.h>

extern AjBool acdDebugSet;
extern AjBool acdDebug;
extern AjPStr acdProgram;


#define SUBDIR_DELIMITER_STR "\\"

typedef void (*AjMessVoidRoutine)(void) ;
typedef void (*AjMessOutRoutine)(char*) ;

void ajMessDie (char *format, ...);

AjMessVoidRoutine ajMessBeepReg (AjMessVoidRoutine func) ;
AjMessOutRoutine  ajMessOutReg (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessDumpReg (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessErrorReg (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessExitReg (AjMessOutRoutine func) ;
AjMessOutRoutine  ajMessCrashReg (AjMessOutRoutine func) ;

void ajMessSetErr(char *filename, int line_num);
void ajMessCrashFL (char *format, ...);
void ajMessVCrashFL (char *format, va_list args);
void ajMessCrashCodeFL (char *code);

/* External Interface.                                                       */
/* Note that ajMesscrash is a macro and that it makes use of the ',' operator  */
/* in C. This means that the ajMesscrash macro will only produce a single C    */
/* statement and hence can be used within brackets etc. and will not break   */
/* existing code, e.g.                                                       */
/*                     funcblah(ajMesscrash("hello")) ;                        */
/* will become:                                                              */
/* funcblah(uMessSetErrorOrigin(__FILE__, __LINE__), uMessCrash("hello")) ;  */
/*                                                                           */

void ajMessErrorInit(char *progname) ; /* Record the applications name for
                                                      use in error messages. */

void ajMessBeep (void) ; /* make a beep */

AjBool ajMessErrorSetFile(char *errfile);   /* set file to read codes/messages from */
void ajMessCodesDelete(void);               /* Delete the code/message pairs */

void ajMessOut (char *format, ...) ;  /* simple message */
void ajMessOutCode(char *name);       /* as above but uses codes to get message */
void ajMessDump (char *format, ...) ; /* write to log file */
void ajMessError (char *format, ...) ; /* error message and write to log file */
void ajMessVError (char *format, va_list args) ; /* error message and write to log file */
void ajMessErrorCode(char *name);      /* as above but uses code to get message */
void ajMessExit(char *format, ...) ;  /* error message, write to log file & exit */
void ajMessWarning (char *format, ...); /* warning message */
void ajMessSetErr (char *filename, int line_num);

#define ajMessCrash   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajMessCrashCode ajMessSetErr(__FILE__, __LINE__), ajMessCrashCodeFL
#define ajFatal   ajMessSetErr(__FILE__, __LINE__), ajMessCrashFL
#define ajVFatal   ajMessSetErr(__FILE__, __LINE__), ajMessVCrashFL
                                                  /* abort - but see below */
#define ajWarn ajMessWarning
#define ajVWarn ajMessVWarning
#define ajErr ajMessError
#define ajVErr ajMessVError
#define ajUser ajMessOut
#define ajVUser ajMessVOut
#define ajDie ajMessDie
#define ajVDie ajMessDie

/*AjBool ajMessReadErrorFile();*/ /* TEST REMOVE LATER AS IT SHOULD BE STATIC */


AjBool ajMessQuery (char *text,...) ;   /* ask yes/no question */
AjBool ajMessPrompt (char *prompt, char *dfault, char *fmt) ;
        /* ask for data satisfying format get results via freecard() */

char* ajMessSysErrorText (void) ; 
        /* wrapped system error message for use in ajMesserror/crash() */

int ajMessErrorCount (void);
        /* return numbers of error so far */

/**** routines to catch crashes if necessary, e.g. when acedb dumping ****/

#include <setjmp.h>

jmp_buf*  ajMessCatchCrash (jmp_buf* ) ;
jmp_buf*  ajMessCatchError (jmp_buf* ) ;
char*     ajMessCaughtMessage (void) ;

  /* if a setjmp() stack context is set using ajMessCatch*() then rather than
     exiting or giving an error message, ajMessCrash() and messError() will
     longjmp() back to the context.
     ajMessCatch*() return the previous value. Use argument = 0 to reset.
     ajMessCaughtMessage() can be called from the jumped-to routine to get
     the error message that would have been printed.
  */

void        ajDebug (char *fmt, ...);
FILE*       ajDebugFile (void);
int         ajUserGet (AjPStr *pthis, char *fmt, ...);

#endif /* defined(DEF_REGULAR_H) */

#ifdef __cplusplus
}
#endif

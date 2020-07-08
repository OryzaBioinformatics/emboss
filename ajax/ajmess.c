/*  Last edited: Jun 14 15:58 2000 (pmr) */
/*  File: ajmess.c
 *  Author: Richard Durbin (rd@mrc-lmb.cam.ac.uk) and Ed Griffiths.
 *  as part of the ACEDB package (messubs.c)
 *  Modified: by I Longden for the EMBOSS package.
 */

#include <string.h>

#include "ajax.h"
#include <errno.h>

/* next three moved from acd for library splitting */
AjBool acdDebugSet = 0;
AjBool acdDebug = 0;
AjPStr acdProgram = NULL;
/***************************************************/



AjPTable errorTable = 0;

static jmp_buf *errorJmpBuf = 0 ;
static jmp_buf *crashJmpBuf = 0 ;

static int errorCount = 0 ;

static char *messErrorFile;

static AjBool fileDebug = 0;
static AjPFile fileDebugFile = NULL;
static AjPStr fileDebugName = NULL;

static char* messGetFilename(char *path);

/* @func ajMessInvokeDebugger *************************************************
**
** Used to trace in a debugger as a breakpoint
**
** @return [void]
** @@
******************************************************************************/

void ajMessInvokeDebugger (void) {
  static AjBool reentrant = AJFALSE ;

  if (!reentrant) {
    reentrant = AJTRUE ;
    /* messalloccheck() ;*/
    reentrant = AJFALSE ;
  }
}

/******************************************************************************
** Constraints on message buffer size.
**
** BUFSIZE:  size of message buffers (messbuf, a global buffer for general
**           message stuff and a private one in ajMessDump).
** PREFIX:   length of message prefix (used to report details such as the
**           file/line info. for where the error occurred.
** MAINTEXT: space left in buffer is the rest after the prefix and string
**           terminator (NULL) are subtracted.
******************************************************************************/

enum {BUFSIZE = 32768,
      PREFIXSIZE = 1024,
      MAINTEXTSIZE = BUFSIZE - PREFIXSIZE - 1} ;

/******************************************************************************
** This buffer is used by just about all of the below routines and has the
** obvious problem that strings can run over the end of it, we can only
** detect this after the event with vsprintf.
******************************************************************************/

static char messbuf[BUFSIZE] ;

/******************************************************************************
** Format strings using va_xx calls.
** Arguments to the macro must have the following types:
**   FORMAT_ARGS:   va_list used to get the variable argument list.
**        FORMAT:   char *  to a string containing the printf format string.
**    TARGET_PTR:   char *  the formatted string will be returned in this
**                          string pointer, N.B. do not put &TARGET_PTR
**        PREFIX:   char *  to a string to be used as a prefix to the rest
**                          of the string, or NULL.
******************************************************************************/

#define AJAXFORMATSTRING(FORMAT_ARGS, FORMAT, TARGET_PTR, PREFIX)       \
va_start(FORMAT_ARGS, FORMAT) ;                                        \
TARGET_PTR = messFormat(FORMAT_ARGS, FORMAT, PREFIX) ;                \
va_end(FORMAT_ARGS) ;

#define AJAXVFORMATSTRING(FORMAT_ARGS, FORMAT, TARGET_PTR, PREFIX)       \
TARGET_PTR = messFormat(FORMAT_ARGS, FORMAT, PREFIX) ;

static char *messFormat(va_list args, char *format, char *prefix) ;
static void messDump (char *message);

/* Some standard defines for titles/text for messages:                       */
/*                                                                           */

#define MESG_TITLE "EMBOSS"
#define ERROR_PREFIX "   ERROR: "
#define WARNING_PREFIX "   Warning: "
#define EXIT_PREFIX "   NON-EMBOSS ERROR: "
#define DIE_PREFIX "   FATAL ERROR: "
#define CRASH_PREFIX_FORMAT "\n   %s FATAL ERROR reported by %s at line %d:\n"
#define FULL_CRASH_PREFIX_FORMAT "\n   %s FATAL ERROR reported by program %s, in file %s, at line %d:\n"
#define SYSERR_FORMAT "system error %d - %s"

/******************************************************************************
** ajMessCrash now reports the file/line no. where ajMessCrash was issued
** as an aid to debugging. We do this using a static structure which holds
** the information and a macro version of ajMessCrash (see regular.h), the
** structure elements are retrieved using access functions.
******************************************************************************/

typedef struct _MessErrorInfo {
  char *progname ;		/* Name of executable reporting error. */
  char *filename ;		/* Filename where error reported */
  int line_num ;		/* line number of file where error
				   reported. */
} MessErrorInfo ;

static MessErrorInfo messageG = {NULL, NULL, 0} ;

static int messGetErrorLine(void) ;
static char *messGetErrorFile(void) ;
static char *messGetErrorProgram(void) ;

/***************************************************************/
/********* call backs and functions to register them ***********/

static AjMessVoidRoutine beepRoutine = 0 ;
static AjMessOutRoutine	 outRoutine = 0 ;
static AjMessOutRoutine	 dumpRoutine = 0 ;
static AjMessOutRoutine	 errorRoutine = 0 ;
static AjMessOutRoutine	 exitRoutine = 0 ;
static AjMessOutRoutine	 crashRoutine = 0 ;
static AjMessOutRoutine	 warningRoutine = 0 ;

/* @func ajMessBeepReg ********************************************
**
** Sets a function to process beeps
**
** @param [r] func [AjMessVoidRoutine] Function to be registered
** @return [AjMessVoidRoutine] Previously defined function
** @@
******************************************************************************/

AjMessVoidRoutine ajMessBeepReg (AjMessVoidRoutine func) {
  AjMessVoidRoutine old = beepRoutine ;
  beepRoutine = func ;
  return old;
}

/* @func ajMessOutReg ********************************************
**
** Sets a function to write messages
**
** @param [r] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessOutReg (AjMessOutRoutine func) {
  AjMessOutRoutine old = outRoutine ;
  outRoutine = func ;
  return old ;
}

/* @func ajMessDumpReg ********************************************
**
** Sets a function to dump data
**
** @param [r] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessDumpReg (AjMessOutRoutine func) {
  AjMessOutRoutine old = dumpRoutine ;
  dumpRoutine = func ;
  return old ;
}

/* @func ajMessErrorReg ********************************************
**
** Sets a function to report errors
**
** @param [r] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessErrorReg (AjMessOutRoutine func) {
  AjMessOutRoutine old = errorRoutine ;
  errorRoutine = func ;
  return old ;
}

/* @func ajMessExitReg ********************************************
**
** Sets a function to exit
**
** @param [r] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessExitReg (AjMessOutRoutine func) {
  AjMessOutRoutine old = exitRoutine ;
  exitRoutine = func ;
  return old ;
}

/* @func ajMessCrashReg ********************************************
**
** Sets a function to crash
**
** @param [r] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessCrashReg (AjMessOutRoutine func) {
  AjMessOutRoutine old = crashRoutine ;
  crashRoutine = func ;
  return old ;
}

/* @func ajMessWarningReg ********************************************
**
** Sets a function to print warnings
**
** @param [r] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessWarningReg (AjMessOutRoutine func) {
  AjMessOutRoutine old = warningRoutine ;
  warningRoutine = func ;
  return old ;
}

/* @func ajMessBeep *******************************************************
**
** Calls the defined beep function, if any. Otherwise prints ASCII 7 to
** standard output.
**
** @return [void]
** @@
******************************************************************************/

void ajMessBeep (void) {
  if (beepRoutine)
    (*beepRoutine)() ;
  else {
    (void) printf ("%c",0x07) ;  /* bell character, I hope */
    (void) fflush (stderr) ;	/* added by fw 02.Feb 1994 */
  }

  return;
}

/* @func ajMessOut *******************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard output.
**
** @param [r] format [char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessOut (char *format,...) {
  va_list args ;
  char *mesg_buf;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, NULL);

  if (outRoutine)
    (*outRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  return;
}

/* @func ajMessVOut *******************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard output.
**
** @param [r] format [char*] Format string
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessVOut (char *format, va_list args) {
  char *mesg_buf;

  /* Format the message string. */

  AJAXVFORMATSTRING(args, format, mesg_buf, NULL);

  if (outRoutine)
    (*outRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  return;
}

/* @func ajMessDump *******************************************************
**
** Formats a message. Calls the dump function (if any).
** Otherwise no further action.
**
** @param [r] format [char*] format string.
** @param [v] [...] Variable length argument list.
** @return [void]
** @@
******************************************************************************/

void ajMessDump (char *format,...) {
  static char dumpbuf[BUFSIZE] ; /* BEWARE limited buffer size. */
  char *mesg_buf = &dumpbuf[0] ;
  va_list args ;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, NULL);

  (void) strcat (mesg_buf, "\n") ;	/* assume we are writing to a file */

  if (dumpRoutine)
    (*dumpRoutine)(mesg_buf) ;

  return;
}

/* @funcstatic messDump ******************************************************
**
** Calls the dump function (if any) to dump text followed by a newline.
**
** @param [r] message [char*] Message text
** @return [void]
** @@
******************************************************************************/

static void messDump (char *message) {

  if (dumpRoutine) {
    (*dumpRoutine)(message) ;
    (*dumpRoutine)("\n") ;
  }

  return;
}

/* @func ajMessErrorCount *****************************************************
**
** Returns the number of times the error routines have been called.
**
** @return [int] Error function call count.
** @@
******************************************************************************/

int ajMessErrorCount (void) {
  return errorCount ;
}

/* @func ajMessError *******************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessError (char *format, ...) {
  char *prefix = ERROR_PREFIX ;
  char *mesg_buf = NULL ;
  va_list args ;

  ++errorCount ;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, prefix) ;

  if (errorJmpBuf)  /* throw back up to the function that registered it */
    longjmp (*errorJmpBuf, 1) ;

  messDump(mesg_buf) ;

  if (errorRoutine)
    (*errorRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger () ;
  return;
}

/* @func ajMessVError *******************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessVError (char *format, va_list args) {
  char *prefix = ERROR_PREFIX ;
  char *mesg_buf = NULL ;

  ++errorCount ;

  /* Format the message string. */

  AJAXVFORMATSTRING(args, format, mesg_buf, prefix) ;

  if (errorJmpBuf)  /* throw back up to the function that registered it */
    longjmp (*errorJmpBuf, 1) ;

  messDump(mesg_buf) ;

  if (errorRoutine)
    (*errorRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger () ;
  return;
}

/* @func ajMessDie *******************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
** Then kills the application.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessDie (char *format, ...) {
  char *prefix = DIE_PREFIX ;
  char *mesg_buf = NULL ;
  va_list args ;

  ++errorCount ;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, prefix) ;

  if (errorJmpBuf)  /* throw back up to the function that registered it */
    longjmp (*errorJmpBuf, 1) ;

  messDump(mesg_buf) ;

  if (errorRoutine)
    (*errorRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger () ;

  exit(EXIT_FAILURE) ;

  return;			/* Should never get here. */
}

/* @func ajMessVDie *******************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
** Then kills the application.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessVDie (char *format, va_list args) {
  char *prefix = DIE_PREFIX ;
  char *mesg_buf = NULL ;

  ++errorCount ;

  /* Format the message string. */

  AJAXVFORMATSTRING(args, format, mesg_buf, prefix) ;

  if (errorJmpBuf)  /* throw back up to the function that registered it */
    longjmp (*errorJmpBuf, 1) ;

  messDump(mesg_buf) ;

  if (errorRoutine)
    (*errorRoutine)(mesg_buf) ;
  else
    ajMessCrash(mesg_buf);

  ajMessInvokeDebugger () ;
  return;
}

/* @func ajMessWarning *******************************************************
**
** Formats a warning message. Calls the warning function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** @param [r] format [char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessWarning (char *format, ...) {
  char *prefix = WARNING_PREFIX ;
  char *mesg_buf = NULL ;
  va_list args ;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, prefix) ;

  if (errorJmpBuf)  /* throw back up to the function that registered it */
    longjmp (*errorJmpBuf, 1) ;
    
  messDump(mesg_buf) ;

  if (warningRoutine)
    (*warningRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger () ;

  return;
}

/* @func ajMessVWarning *******************************************************
**
** Formats a warning message. Calls the warning function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** @param [r] format [char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessVWarning (char *format, va_list args) {
  char *prefix = WARNING_PREFIX ;
  char *mesg_buf = NULL ;

  /* Format the message string. */

  AJAXVFORMATSTRING(args, format, mesg_buf, prefix) ;

  if (errorJmpBuf)  /* throw back up to the function that registered it */
    longjmp (*errorJmpBuf, 1) ;
    
  messDump(mesg_buf) ;

  if (warningRoutine)
    (*warningRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger () ;

  return;
}

/* @func ajMessExit ******************************************************
**
** Formats an exit message and calls the exit function (if any).
** Otherwise prints the message to standard error with a trailing newline
** and exist with code EXIT_FAILURE.
**
** Use this function for errors that while being unrecoverable are not a
** problem with the AJAX code.
**
** Note that there errors are logged but that this routine will exit without
** any chance to interrupt it (see the crash routine in ajMessCrashFL), this
** could be changed to allow the application to register an exit handler.
**
** @param [r] format [char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessExit(char *format, ...) {
  char *prefix = EXIT_PREFIX ;
  char *mesg_buf = NULL ;
  va_list args ;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, prefix) ;

  messDump(mesg_buf) ;

  if (exitRoutine)
    (*exitRoutine)(mesg_buf) ;
  else
    (void) fprintf (stderr, "%s\n", mesg_buf) ;

  exit(EXIT_FAILURE) ;

  return ;			/* Should never get here. */
}

/* @func ajMessCrashFL *******************************************************
**
** This is the routine called by the ajFatal macro and others.
**
** This routine may encounter errors itself, in which case it will attempt
** to call itself to report the error. To avoid infinite recursion we limit
** this to just one reporting of an internal error and then we abort.
**
** @param [r] format [char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessCrashFL (char *format, ...) {
  enum {MAXERRORS = 1} ;
  static int internalErrors = 0 ;
  static char prefix[1024] ;
  int rc ;
  char *mesg_buf = NULL ;
  va_list args ;

  /* Check for recursive calls and abort if necessary. */

  if (internalErrors > MAXERRORS)
    abort() ;
  else
    internalErrors++ ;

  /* Construct the message prefix, adding the program name if possible. */

  if (messGetErrorProgram() == NULL)
    rc = sprintf(prefix, CRASH_PREFIX_FORMAT, MESG_TITLE,
		 messGetErrorFile(), messGetErrorLine()) ;
  else
    rc = sprintf(prefix, FULL_CRASH_PREFIX_FORMAT, MESG_TITLE,
		 messGetErrorProgram(), messGetErrorFile(),
		 messGetErrorLine()) ;
  if (rc < 0)
    ajMessCrash("sprintf failed") ;

  /* Format the message string. */

  AJAXFORMATSTRING(args, format, mesg_buf, prefix) ;


  if (crashJmpBuf)	/* throw back up to the function that registered it */
    longjmp(*crashJmpBuf, 1) ;

  
  messDump(mesg_buf) ;

  if (crashRoutine)
    (*crashRoutine)(mesg_buf) ;
  else
    (void) fprintf(stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger() ;

  exit(EXIT_FAILURE) ;

  return ;			/* Should never get here. */
}

/* @func ajMessVCrashFL ******************************************************
**
** This is the routine called by the ajVFatal macro and others.
**
** This routine may encounter errors itself, in which case it will attempt
** to call itself to report the error. To avoid infinite recursion we limit
** this to just one reporting of an internal error and then we abort.
**
** @param [r] format [char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessVCrashFL (char *format, va_list args) {
  enum {MAXERRORS = 1} ;
  static int internalErrors = 0 ;
  static char prefix[1024] ;
  int rc ;
  char *mesg_buf = NULL ;

  /* Check for recursive calls and abort if necessary. */

  if (internalErrors > MAXERRORS)
    abort() ;
  else
    internalErrors++ ;

  /* Construct the message prefix, adding the program name if possible. */

  if (messGetErrorProgram() == NULL)
    rc = sprintf(prefix, CRASH_PREFIX_FORMAT, MESG_TITLE,
		 messGetErrorFile(), messGetErrorLine()) ;
  else
    rc = sprintf(prefix, FULL_CRASH_PREFIX_FORMAT, MESG_TITLE,
		 messGetErrorProgram(), messGetErrorFile(),
		 messGetErrorLine()) ;
  if (rc < 0)
    ajMessCrash("sprintf failed") ;

  /* Format the message string. */

  AJAXVFORMATSTRING(args, format, mesg_buf, prefix) ;


  if (crashJmpBuf)	/* throw back up to the function that registered it */
    longjmp(*crashJmpBuf, 1) ;

  
  messDump(mesg_buf) ;

  if (crashRoutine)
    (*crashRoutine)(mesg_buf) ;
  else
    (void) fprintf(stderr, "%s\n", mesg_buf) ;

  ajMessInvokeDebugger() ;

  exit(EXIT_FAILURE) ;

  return ;			/* Should never get here. */
}


/* @func ajMessCatchError *****************************************************
**
** Redirects error call
**
** If a setjmp() stack context is set using ajMessCatchError() then rather
** than exiting or giving an error message, ajMessError() will
** longjmp() back to the context.
**
** @param [r] fnew [jmp_buf*] Jump buffer new
** @return [jmp_buf*] Jump buffer old
** @@
******************************************************************************/

jmp_buf* ajMessCatchError (jmp_buf* fnew) {
  jmp_buf* old = errorJmpBuf ;
  errorJmpBuf = fnew ;

  return old ;
}

/* @func ajMessCatchCrash *****************************************************
**
** Redirects crash call
**
** If a setjmp() stack context is set using ajMessCatchCrash() then rather
** than exiting or giving an error message, ajMessCrash() will
** longjmp() back to the context.
**
** @param [r] fnew [jmp_buf*] Jump buffer new
** @return [jmp_buf*] Jump buffer old
** @@
******************************************************************************/

jmp_buf* ajMessCatchCrash (jmp_buf* fnew) {
  jmp_buf* old = crashJmpBuf ;
  crashJmpBuf = fnew ;

  return old ;
}

/* @func ajMessCaughtMessage **************************************************
**
** Returns the current message text.
**
** @return [char*] Message text
** @@
******************************************************************************/

char* ajMessCaughtMessage (void) {
  return messbuf ;
}

/* This function writes into the global messbuf which has a limited size,    */
/* see top of file: BUFSIZE                                                  */
/*                                                                           */
/*char *ajFmtString (char *format, ...)
 {
 char *mesg_buf ;
   va_list args ;

   ?? Format the message string.                                              ??
   AJAXFORMATSTRING(args, format, mesg_buf, NULL)

   return mesg_buf ;
   }
*/

/* Return the string for a given errno from the standard C library.          */
/*                                                                           */

/* @func ajMessSysErrorText ***************************************************
**
** Returns the system message text from 'strerror'
**
** @return [char*] System error message.
** @@
******************************************************************************/

char* ajMessSysErrorText (void) {

  static char* errmess = 0 ;
  char *mess ;

  mess = ajFmtString (SYSERR_FORMAT, errno, strerror(errno)) ;

  /* must make copy - will be used when mess* calls itself */
  if (errmess)
    AJFREE(errmess) ;
  errmess = ajSysStrdup (mess) ;

  return errmess ;
}

/************************* message formatting ********************************/
/* This routine does the formatting of the message string using vsprintf,    */
/* it copes with the format string accidentally being our internal buffer.   */
/*                                                                           */
/* This routine does its best to check that the vsprintf is successful, if   */
/* not the routine bombs out with an error message. Note that num_bytes is   */
/* the return value from vsprintf.                                           */
/* Failures trapped:                                                         */
/*      num_bytes less than 0  =  vsprintf failed, reason is reported.      */
/* num_bytes + 1 more than BUFSIZE  =  our internal buffer size was         */
/*                                      exceeded.                            */
/*                                 (vsprintf returns number of bytes written */
/*                                  _minus_ terminating NULL)                */
/*                                                                           */

/* @funcstatic messFormat *****************************************************
**
** Used by the AJAXFORMAT macros to format messages.
**
** This routine does the formatting of the message string using vsprintf,
** it copes with the format string accidentally being our internal buffer.
**
** This routine does its best to check that the vsprintf is successful, if
** not the routine bombs out with an error message. Note that num_bytes is 
** the return value from vsprintf. 
**
** Failures trapped: 
**  num_bytes less than 0  =  vsprintf failed, reason is reported. 
**  num_bytes + 1 more than BUFSIZE  =  our internal buffer size was exceeded.
**                                 (vsprintf returns number of bytes written
**                                  _minus_ terminating NULL) 
**
** @param [r] args [va_list] Variable length argument list
** @param [r] format [char*] Format string
** @param [r] prefix [char*] Message prefix
** @return [char*] Formatted message text
** @@
******************************************************************************/

static char* messFormat(va_list args, char *format, char *prefix) {

  static char *new_buf = NULL ;
  char *buf_ptr ;
  int num_bytes, prefix_len ;

  /* Check arguments. */

  if (format == NULL)
    ajMessCrash("invalid call, no format string.") ;

  if (prefix == NULL)
    prefix_len = 0 ;
  else {
    prefix_len = strlen(prefix) ;
    if ((prefix_len + 1) > PREFIXSIZE)
      ajMessCrash("prefix string is too long.") ;
  }

  /* If they supply our internal buffer as an argument, e.g. because they */
  /* used ajFmtString as an arg, then make a copy, otherwise use internal */
  /* buffer.                                                              */

  if (format == messbuf) {
    if (new_buf != NULL) AJFREE(new_buf) ;
    buf_ptr = new_buf = ajSysStrdup(format) ;
  }
  else
    buf_ptr = messbuf ;

  /* Add the prefix if there is one. */
  if (prefix != NULL) {
    if (!strcpy (buf_ptr, prefix))
      ajMessCrash("strcpy failed") ;
  }
  
  /* Do the format.                                                          */
  /* num_bytes = vsprintf((buf_ptr + prefix_len), format, args)+prefix_len+1;*/

  num_bytes = prefix_len + 1 ;
  num_bytes += ajFmtVPrintCL((buf_ptr + prefix_len),BUFSIZE, format, args);

  /* Check the result. This should never happen using the ajFmtVPrintCL routine
  instead of the vsprintf routine */

  if (num_bytes < 0)
    ajMessCrash("vsprintf failed: %s", ajMessSysErrorText()) ;
  else if (num_bytes > BUFSIZE)
    ajMessCrash("messubs internal buffer size (%d) exceeded, "
		"a total of %d bytes were written",
		BUFSIZE, num_bytes) ;  

  return(buf_ptr) ;
}

/* @funcstatic messGetFilename ******************************************
**
** Converts a filename into a base file name. Used for filenames passed
** by macros from __FILE__ which could include part or all of the path
** depending on how the source code was compiled.
**
** @param [r] path [char*] File name, possibly with full path.
** @return [char*] Base file name
** @@
******************************************************************************/

static char* messGetFilename(char *path) {
  static char *path_copy = NULL ;
  const char *path_delim = SUBDIR_DELIMITER_STR ;
  char *result = NULL, *tmp ;
    
  if (path != NULL) {
    if (strcmp((path + strlen(path) - 1), path_delim) != 0) {/* Last char = "/" ?? */
      if (path_copy != NULL)
	AJFREE (path_copy) ;
      path_copy = ajSysStrdup(path) ;
      
      tmp = ajSysStrtok(path_copy, path_delim) ;
          
      while (tmp != NULL) {
        result = tmp ;		/* Keep results of previous strtok */

        tmp = ajSysStrtok(NULL, path_delim) ;
      }
    }
  }
    
  return(result) ;
}

/*
** When AJAX needs to crash because there has been an unrecoverable
** error we want to output the file and line number of the code that
** detected the error. Here are the functions to do it.
**
** Applications can optionally initialise the error handling section of the
** message package, currently the program name can be set (argv[0] in the
** main routine) as there is no easy way to get at this at run time except
** from the main.
** */

/* @func ajMessErrorInit *****************************************************
**
** Initialises the stored program name.
**
** @param [r] progname [char*] Program name.
** @return [void]
** @@
******************************************************************************/

void ajMessErrorInit (char *progname) {

  if (progname != NULL)
    messageG.progname = ajSysStrdup(messGetFilename(progname)) ;

  return ;
}

/* @func ajMessSetErr *******************************************************
**
** Stores the source file name (converted to a base name)
** and the source line number to be
** reported by the crash routines.
**
** Invoked automatically by a macro (e.g. ajFatal) where needed.
**
** @param [r] filename [char*] source filename, __FILE__
** @param [r] line_num [int] source line number, __LINE__
** @return [void]
** @@
******************************************************************************/

void ajMessSetErr (char *filename, int line_num) {
  
  assert(filename != NULL && line_num != 0) ;
    
  /* We take the basename here because __FILE__ can be a path rather than    */
  /* just a filename, depending on how a module was compiled.                */

  messageG.filename = ajSysStrdup(messGetFilename(filename)) ;

  messageG.line_num = line_num ;

  return ;
}


/* Access functions for message error data.                                  */
/* @funcstatic messGetErrorProgram ********************************************
**
** Returns the stored program name.
**
** @return [char*] Program name
** @@
******************************************************************************/

static char* messGetErrorProgram(void) {
  return(messageG.progname) ;
}  

/* @funcstatic messGetErrorFile ***********************************************
**
** Returns the stored error file name.
**
** @return [char*] Error file name
** @@
******************************************************************************/

static char* messGetErrorFile(void) {
  return(messageG.filename) ;
}  

/* @funcstatic messGetErrorLine ***********************************************
**
** Returns the stored error source line number.
**
** @return [int] Original source code line number
** @@
******************************************************************************/

static int messGetErrorLine(void) {
  return(messageG.line_num) ;
}  

/* set a file to read for all the messages. NB if this is not set
Then a default one will be read */

/* @func ajMessErrorSetFile ***************************************************
**
** Opens a file and sets this to be the error file.
**
** @param [r] errfile [char*] Error file name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajMessErrorSetFile(char *errfile) {
  FILE *fp=0;

  if(errfile){
    if ((fp = fopen(errfile,"r"))) {
      messErrorFile = ajSysStrdup(errfile);
      (void) fclose(fp);
      return ajTrue;
    }
  }

  return ajFalse;
}

/* @funcstatic ajMessReadErrorFile ********************************************
**
** Reads the error message file (with a default of
** $EMBOSS_ROOT/messages/messages.english)
** and loads the results into an internal table.
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool ajMessReadErrorFile(void){
  char line[512];
  char name[12];
  char message[200];
  FILE *fp=0;
  char *mess,*cp;
  char *namestore,*messstore;

  if(messErrorFile) {
    fp = fopen(messErrorFile,"r");
  }
  if(!fp) {
    messErrorFile = ajFmtString("%s/messages/messages.english",
				  getenv("EMBOSS_ROOT"));
    fp = fopen(messErrorFile,"r");
  }    
  if(!fp)
    return ajFalse;
  
  errorTable = ajStrTableNewC(0);

  while(fgets(line, 512, fp)) {
    if(sscanf(line,"%s %s",name,message)!=2)
	ajFatal("Library sscanf1");
    cp = strchr(line,'"');
    cp++;
    mess = &message[0];
    while(*cp != '"') {
      *mess = *cp;
      cp++;
/*      *mess++; Looks wrong to me. Replaced by below. AJB */
      mess++;
    }
    *mess = '\0';
    namestore = ajFmtString("%s",name);
    messstore = ajFmtString("%s",message);
    mess = (char *) ajTableGet(errorTable, namestore);
    if(mess)
      ajMessError("%s is listed more than once in file %s",
		  name,messErrorFile);
    else
      (void) ajTablePut(errorTable, namestore, messstore);

  }
  return ajTrue;
}

/* @func ajMessOutCode *******************************************************
**
** Writes an output message for a given message code.
**
** @param [r] code [char*] Message code
** @return [void]
** @@
******************************************************************************/

void ajMessOutCode(char *code) {
  char *mess=0;

  if(errorTable) {
    mess = ajTableGet(errorTable, code);
    if(mess)
      ajMessOut(mess);
    else
      ajMessOut("could not find error code %s",code);
  }
  else {
    if (ajMessReadErrorFile()) {
      mess = ajTableGet(errorTable, code);
      if (mess)
	ajMessOut(mess);
      else
	ajMessOut("could not find error code %s",code);
    }
    else
      ajMessOut("Could not read the error file hence no reference to %s",
		code);
  }
  return;
}

/* @func ajMessErrorCode ******************************************************
**
** Writes an error message for a given message code.
**
** @param [r] code [char*] Error code
** @return [void]
** @@
******************************************************************************/

void ajMessErrorCode(char *code) {
  char *mess=0;

  if(errorTable) {
    mess = ajTableGet(errorTable, code);
    if(mess)
      ajMessError(mess);
    else
      ajMessError("could not find error code %s",code);
  }
  else {
    if(ajMessReadErrorFile()) {
      mess = ajTableGet(errorTable, code);
      if (mess)
	ajMessError(mess);
      else
	ajMessError("could not find error code %s",code);
    }
    else
      ajMessError("Could not read the error file, hence no reference to %s",
		  code);
  }
  return;
}

/* @func ajMessCrashCodeFL ****************************************************
**
** Writes an error message for a given message code and crashes.
**
** @param [r] code [char*] Error code
** @return [void]
** @@
******************************************************************************/

void ajMessCrashCodeFL (char *code) {
  char *mess=0;

  if(errorTable) {
    mess = ajTableGet(errorTable, code);
    if (mess)
      ajMessCrashFL(mess);
    else
      ajMessCrashFL("could not find error code %s",code);
  }
  else {
    if (ajMessReadErrorFile()) {
      mess = ajTableGet(errorTable, code);
      if (mess)
	ajMessCrashFL(mess);
      else
	ajMessCrashFL("could not find error code %s",code);
    }
    else
      ajMessCrashFL("Could not read the error file hence no reference to %s",
		 code);
  }
  return;
}

/* @func ajMessCodesDelete ****************************************************
**
** Deletes the message codes table.
**
** @return [void]
** @@
******************************************************************************/

void ajMessCodesDelete (void) {
  void **array;
  int i;
  
  if(!errorTable)
    return;

  array =  ajTableToarray(errorTable, NULL);

  for (i = 0; array[i]; i += 2){
    AJFREE (array[i+1]);
    AJFREE (array[i]);
  }
  AJFREE (array); 
  ajTableFree (&errorTable);
  errorTable = 0;
  return;
}
/* @func ajDebug*******************************************************
**
** Writes a debug message to the debug file if debugging is on.
** Typically, debugging is turned on by adding '-debug' to the command line
** or by defining a variable prefix_DEBUG
**
** Avoid using this call in any code which can be invoked before the command
** line processing is complete as it can be a problem to find a reasonable
** file name for debug output under these circumstances.
**
** @param [r] fmt [char*] Format.
** @param [v] [...] Variable argument list.
** @return [void]
** @@
******************************************************************************/

void ajDebug (char* fmt, ...) {
  va_list args ;
  static int debugset = 0;
  static int depth = 0;

  if (depth) {			/* recursive call, get out quick */
    if (fileDebugFile) {
      va_start (args, fmt) ;
      ajFmtVPrintF(fileDebugFile, fmt, args) ;
      va_end (args) ;
    }
    return;
  }

  depth++;
  if (!debugset && acdDebugSet) {
    fileDebug = acdDebug;
    if (fileDebug) {
      (void) ajFmtPrintS(&fileDebugName, "%s.dbg", ajStrStr(acdProgram));
      fileDebugFile = ajFileNewOut (fileDebugName);
      ajFileUnbuffer (fileDebugFile);
    }
    debugset = 1;
  }

  if (fileDebug) {
    va_start (args, fmt) ;
    ajFmtVPrintF(fileDebugFile, fmt, args) ;
    va_end (args) ;
  }

  depth--;
  return;
}

/* @func ajDebugFile ***************************************************
**
** Returns the file used for debug output, or NULL if no debug file is open.
**
** @return [FILE*] C runtime library file handle for debug output.
** @@
******************************************************************************/

FILE* ajDebugFile (void) {

  if (!fileDebugFile)
    return NULL;
  return ajFileFp(fileDebugFile);
}

/* @func ajUserGet *******************************************************
**
** Writes a prompt to the terminal and reads one line from the user.
**
** @param [w] pthis [AjPStr*] Buffer for the user response.
** @param [r] fmt [char*] Format string
** @param [v] [...] Variable argument list.
** @return [int] Length of response string.
** @@
******************************************************************************/

int ajUserGet (AjPStr* pthis, char* fmt, ...) {
  AjPStr thys;
  char *cp;
  va_list args ;

  va_start (args, fmt) ;
  ajFmtVError(fmt, args) ;
  va_end (args) ;

  /* Must be > 1, reserved for fgets!! */
  (void) ajStrModL (pthis,ajFileBuffSize());
  thys = pthis ? *pthis : 0;

  ajDebug ("ajUserGet buffer len: %d res: %d ptr: %x\n",
	   ajStrLen(thys), ajStrSize(thys), thys->Ptr);

  cp = fgets (thys->Ptr, thys->Res, stdin);

  if (!cp) {			/* EOF or error */
    if (feof(stdin))
      ajFatal ("END-OF-FILE reading from user\n");
    else
      ajFatal ("Error reading from user\n");
  }

  thys->Len = strlen(thys->Ptr);
  if (thys->Ptr[thys->Len-1] == '\n') {
    thys->Ptr[--thys->Len] = '\0';
  }
  else
    ajErr ("ajUserGet no newline seen\n");

  return thys->Len;
}


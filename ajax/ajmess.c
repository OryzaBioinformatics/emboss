/******************************************************************************
** @source AJAX message functions
**
** @author Richard Durbin and Ed Griffiths from ACEdb (messubs.c)
** @version 1.0
** @modified Ian Longden for EMBOSS
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

#include <string.h>

#include "ajax.h"
#include <errno.h>


#ifdef __CYGWIN__
#define fopen(a,b) ajSysFopen(a,b)
#endif


/* next three moved from acd for library splitting */

AjBool acdDebugSet    = 0;
AjBool acdDebugBuffer = 0;
AjBool acdDebug       = 0;
AjPStr acdProgram     = NULL;


AjOError AjErrorLevel =
{
    AJTRUE, AJTRUE, AJTRUE, AJTRUE
};

/***************************************************/



AjPTable errorTable = 0;

static ajint errorCount = 0;

static char *messErrorFile;

static AjBool fileDebug      = 0;
static AjPFile fileDebugFile = NULL;
static AjPStr fileDebugName  = NULL;

static char* messGetFilename(const char *path);




/*============================================================================
**======================== Macros ============================================
=============================================================================*/




/* @macro ajFatal *************************************************************
**
** Fatal error message to standard error.
** Includes filename and line number in the source code that invokes it.
** Newline is added automatically at the end of the format string.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

/* @macro ajMessCrash *********************************************************
**
** Crash error message to standard error.
** Includes filename and line number in the source code that invokes it.
** Newline is added automatically at the end of the format string.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/




/* @func ajMessInvokeDebugger *************************************************
**
** Used to trace in a debugger as a breakpoint
**
** @return [void]
** @@
******************************************************************************/

void ajMessInvokeDebugger(void)
{
    static AjBool reentrant = AJFALSE;

    if(!reentrant)
    {
	reentrant = AJTRUE;
	reentrant = AJFALSE;
    }

    return;
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
      MAINTEXTSIZE = BUFSIZE - PREFIXSIZE - 1};

/******************************************************************************
** This buffer is used by just about all of the below routines and has the
** obvious problem that strings can run over the end of it, we can only
** detect this after the event with vsprintf.
******************************************************************************/

static char messbuf[BUFSIZE];

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

#define AJAXFORMATSTRING(FORMAT_ARGS, FORMAT, TARGET_PTR, PREFIX)    \
va_start(FORMAT_ARGS, FORMAT);                                       \
TARGET_PTR = messFormat(FORMAT_ARGS, FORMAT, PREFIX);                \
va_end(FORMAT_ARGS);

#define AJAXVFORMATSTRING(FORMAT_ARGS, FORMAT, TARGET_PTR, PREFIX)   \
TARGET_PTR = messFormat(FORMAT_ARGS, FORMAT, PREFIX);

static char *messFormat(va_list args, const char *format, const char *prefix);
static void messDump(const char *message);

/* Some standard defines for titles/text for messages:                       */
/*                                                                           */

#define MESG_TITLE "EMBOSS"
#define ERROR_PREFIX "Error: "
#define WARNING_PREFIX "Warning: "
#define EXIT_PREFIX "   An error spotted (non-EMBOSS): "
#define DIE_PREFIX "Died: "
#define CRASH_PREFIX_FORMAT "\n   %s An error in %s at line %d:\n"
#define FULL_CRASH_PREFIX_FORMAT "\n   %s Program cannot continue " \
                                 "(%s, in file %s, at line %d):\n"
#define SYSERR_FORMAT "Something wrong with a system call (%d - %s)"
#define SYSERR_OK "Successful system call (%d - %s)"

/******************************************************************************
** ajMessCrash now reports the file/line no. where ajMessCrash was issued
** as an aid to debugging. We do this using a static structure which holds
** the information and a macro version of ajMessCrash (see regular.h), the
** structure elements are retrieved using access functions.
******************************************************************************/

/* @datastatic MessPErrorInfo *************************************************
**
** Message error information
**
** @alias MessSErrorInfo
** @alias MessOErrorInfo
**
** @attr progname [char*] Name of executable reporting error
** @attr filename [char*] Filename where error was reported
** @attr line_num [ajint] line number of file where error was reported.
** @@
******************************************************************************/

typedef struct MessSErrorInfo
{
    char* progname;
    char* filename;
    ajint line_num;
} MessOErrorInfo;

#define MessPErrorInfo MessOErrorInfo*

static MessOErrorInfo messageG = {NULL, NULL, 0};

static ajint messGetErrorLine(void);
static char *messGetErrorFile(void);
static char *messGetErrorProgram(void);

/***************************************************************/
/********* call backs and functions to register them ***********/

static AjMessVoidRoutine beepRoutine    = 0;
static AjMessOutRoutine	 outRoutine     = 0;
static AjMessOutRoutine	 dumpRoutine    = 0;
static AjMessOutRoutine	 errorRoutine   = 0;
static AjMessOutRoutine	 exitRoutine    = 0;
static AjMessOutRoutine	 crashRoutine   = 0;
static AjMessOutRoutine	 warningRoutine = 0;




/* @func ajMessRegBeep ********************************************************
**
** Sets a function to process beeps
**
** @param [f] func [AjMessVoidRoutine] Function to be registered
** @return [AjMessVoidRoutine] Previously defined function
** @@
******************************************************************************/

AjMessVoidRoutine ajMessRegBeep(AjMessVoidRoutine func)
{
    AjMessVoidRoutine old;

    old = beepRoutine;
    beepRoutine = func;

    return old;
}




/* @func ajMessRegOut *********************************************************
**
** Sets a function to write messages
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegOut(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = outRoutine;
    outRoutine = func;

    return old;
}




/* @func ajMessRegDump ********************************************************
**
** Sets a function to dump data
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegDump(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = dumpRoutine;
    dumpRoutine = func;

    return old;
}




/* @func ajMessRegErr *********************************************************
**
** Sets a function to report errors
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegErr(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = errorRoutine;
    errorRoutine = func;

    return old;
}




/* @func ajMessRegExit ********************************************************
**
** Sets a function to exit
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegExit(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = exitRoutine;
    exitRoutine = func;
    return old;
}




/* @func ajMessRegCrash *******************************************************
**
** Sets a function to crash
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegCrash(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = crashRoutine;
    crashRoutine = func;

    return old;
}




/* @func ajMessRegWarn ********************************************************
**
** Sets a function to print warnings
**
** @param [f] func [AjMessOutRoutine] Function to be registered
** @return [AjMessOutRoutine] Previously defined function
** @@
******************************************************************************/

AjMessOutRoutine ajMessRegWarn(AjMessOutRoutine func)
{
    AjMessOutRoutine old;

    old = warningRoutine;
    warningRoutine = func;

    return old;
}




/* @func ajMessBeep ***********************************************************
**
** Calls the defined beep function, if any. Otherwise prints ASCII 7 to
** standard error.
**
** @return [void]
** @@
******************************************************************************/

void ajMessBeep(void)
{
    if(beepRoutine)
	(*beepRoutine)();
    else
    {
	printf("%c",0x07);
	fflush(stdout);
    }

    return;
}




/* @func ajUser ***************************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard error with an extra newline.
**
** @param [r] format [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajUser(const char *format,...)
{
    va_list args;
    const char *mesg_buf;

    AJAXFORMATSTRING(args, format, mesg_buf, NULL);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    return;
}




/* @func ajMessOut ************************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard error with no newline.
**
** @param [r] format [const char*] Format string
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessOut(const char *format,...)
{
    va_list args;
    char *mesg_buf;

    AJAXFORMATSTRING(args, format, mesg_buf, NULL);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s", mesg_buf);

    return;
}




/* @func ajVUser **************************************************************
**
** Formats a message. Calls the defined output function (if any).
** Otherwise prints the message to standard error.
**
** @param [r] format [const char*] Format string
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVUser(const char *format, va_list args)
{
    char *mesg_buf;

    AJAXVFORMATSTRING(args, format, mesg_buf, NULL);

    if(outRoutine)
	(*outRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    return;
}




/* @func ajMessDump ***********************************************************
**
** Formats a message. Calls the dump function (if any).
** Otherwise no further action.
**
** @param [r] format [const char*] format string.
** @param [v] [...] Variable length argument list.
** @return [void]
** @@
******************************************************************************/

void ajMessDump(const char *format,...)
{
    static char dumpbuf[BUFSIZE];   /* BEWARE limited buffer size. */
    char *mesg_buf;
    va_list args;

    mesg_buf = &dumpbuf[0];

    AJAXFORMATSTRING(args, format, mesg_buf, NULL);

    strcat(mesg_buf, "\n"); /* assume we are writing to a file */

    if(dumpRoutine)
	(*dumpRoutine)(mesg_buf);

    return;
}




/* @funcstatic messDump *******************************************************
**
** Calls the dump function (if any) to dump text followed by a newline.
**
** @param [r] message [const char*] Message text
** @return [void]
** @@
******************************************************************************/

static void messDump(const char *message)
{
    if(dumpRoutine)
    {
	(*dumpRoutine)(message);
	(*dumpRoutine)("\n");
    }

    return;
}




/* @func ajMessErrorCount *****************************************************
**
** Returns the number of times the error routines have been called.
**
** @return [ajint] Error function call count.
** @@
******************************************************************************/

ajint ajMessErrorCount(void)
{
    return errorCount;
}




/* @func ajErr ****************************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajErr(const char *format, ...)
{
    char *prefix   = ERROR_PREFIX;
    char *mesg_buf = NULL;
    va_list args;

    ++errorCount;

    if(AjErrorLevel.error)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(errorRoutine)
	    (*errorRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }

    return;
}




/* @func ajVErr ***************************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVErr(const char *format, va_list args)
{
    char *prefix   = ERROR_PREFIX;
    char *mesg_buf = NULL;

    ++errorCount;

    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(errorRoutine)
	(*errorRoutine)(mesg_buf);
    else
    {
	if(AjErrorLevel.error)
	    fprintf(stderr, "%s\n", mesg_buf);
    }
    ajMessInvokeDebugger();
    return;
}




/* @func ajDie ****************************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
** Then kills the application.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajDie(const char *format, ...)
{
    const char *prefix   = DIE_PREFIX;
    const char *mesg_buf = NULL;
    va_list args;

    ++errorCount;

    if(AjErrorLevel.die)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(errorRoutine)
	    (*errorRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }


    exit(EXIT_FAILURE);

    return;
}




/* @func ajVDie ***********************************************************
**
** Formats an error message. Calls the error function (if any).
** Otherwise prints the message to standard error with a trailing newline.
** Then kills the application.
**
** The error message count is incremented by 1 for each call.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVDie(const char *format, va_list args)
{
    char *prefix   = DIE_PREFIX;
    char *mesg_buf = NULL;

    ++errorCount;

    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(errorRoutine)
	(*errorRoutine)(mesg_buf);
    else
	ajMessCrash(mesg_buf);

    ajMessInvokeDebugger();

    return;
}




/* @func ajWarn ***************************************************************
**
** Formats a warning message. Calls the warning function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajWarn(const char *format, ...)
{
    char *prefix   = WARNING_PREFIX;
    char *mesg_buf = NULL;
    va_list args;

    if(AjErrorLevel.warning)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(warningRoutine)
	    (*warningRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }

    return;
}




/* @func ajVWarn **************************************************************
**
** Formats a warning message. Calls the warning function (if any).
** Otherwise prints the message to standard error with a trailing newline.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajVWarn(const char *format, va_list args)
{
    char *prefix   = WARNING_PREFIX;
    char *mesg_buf = NULL;

    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(warningRoutine)
	(*warningRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    ajMessInvokeDebugger();

    return;
}




/* @func ajMessExitmsg ********************************************************
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
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessExitmsg(const char *format, ...)
{
    char *prefix   = EXIT_PREFIX;
    char *mesg_buf = NULL;
    va_list args;

    AJAXFORMATSTRING(args, format, mesg_buf, prefix);

    messDump(mesg_buf);

    if(exitRoutine)
	(*exitRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);

    exit(EXIT_FAILURE);

    return;
}




/* @func ajMessCrashFL ********************************************************
**
** This is the routine called by the ajFatal macro and others.
**
** This routine may encounter errors itself, in which case it will attempt
** to call itself to report the error. To avoid infinite recursion we limit
** this to just one reporting of an internal error and then we abort.
**
** @param [r] format [const char*] Format
** @param [v] [...] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessCrashFL(const char *format, ...)
{
    enum {MAXERRORS = 1};
    static ajint internalErrors = 0;
    static char prefix[1024];
    ajint rc;
    char *mesg_buf = NULL;
    va_list args;


    if(internalErrors > MAXERRORS)
	abort();
    else
	internalErrors++;

    /* Construct the message prefix, adding the program name if possible. */

    if(messGetErrorProgram() == NULL)
	rc = sprintf(prefix, CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorFile(), messGetErrorLine());
    else
	rc = sprintf(prefix, FULL_CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorProgram(), messGetErrorFile(),
		     messGetErrorLine());
    if(rc < 0)
	ajMessCrash("sprintf failed");

    if(AjErrorLevel.fatal)
    {
	AJAXFORMATSTRING(args, format, mesg_buf, prefix);

	messDump(mesg_buf);

	if(crashRoutine)
	    (*crashRoutine)(mesg_buf);
	else
	    fprintf(stderr, "%s\n", mesg_buf);

	ajMessInvokeDebugger();
    }


    exit(EXIT_FAILURE);

    return;
}




/* @func ajMessVCrashFL *******************************************************
**
** This is the routine called by the ajVFatal macro and others.
**
** This routine may encounter errors itself, in which case it will attempt
** to call itself to report the error. To avoid infinite recursion we limit
** this to just one reporting of an internal error and then we abort.
**
** @param [r] format [const char*] Format
** @param [v] args [va_list] Variable length argument list
** @return [void]
** @@
******************************************************************************/

void ajMessVCrashFL(const char *format, va_list args)
{
    enum {MAXERRORS = 1};
    static ajint internalErrors = 0;
    static char prefix[1024];
    ajint rc;
    char *mesg_buf = NULL;
    
    if(internalErrors > MAXERRORS)
	abort();
    else
	internalErrors++;
    
    /* Construct the message prefix, adding the program name if possible. */
    
    if(messGetErrorProgram() == NULL)
	rc = sprintf(prefix, CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorFile(), messGetErrorLine());
    else
	rc = sprintf(prefix, FULL_CRASH_PREFIX_FORMAT, MESG_TITLE,
		     messGetErrorProgram(), messGetErrorFile(),
		     messGetErrorLine());
    if(rc < 0)
	ajMessCrash("sprintf failed");
    
    
    AJAXVFORMATSTRING(args, format, mesg_buf, prefix);
    
    messDump(mesg_buf);
    
    if(crashRoutine)
	(*crashRoutine)(mesg_buf);
    else
	fprintf(stderr, "%s\n", mesg_buf);
    
    ajMessInvokeDebugger();
    
    exit(EXIT_FAILURE);

    return;
}



/* @func ajMessCaughtMessage **************************************************
**
** Returns the current message text.
**
** @return [char*] Message text
** @@
******************************************************************************/

char* ajMessCaughtMessage(void)
{
    return messbuf;
}




/* @func ajMessSysErrorText ***************************************************
**
** Returns the system message text from 'strerror' from the standard C
** library.
**
** @return [char*] System error message.
** @@
******************************************************************************/

char* ajMessSysErrorText(void)
{
    static char* errmess = 0;
    char *mess;

    if(errno)
	mess = ajFmtString(SYSERR_FORMAT, errno, strerror(errno));
    else
	mess = ajFmtString(SYSERR_OK, errno, strerror(errno));
      
    /* must make copy - will be used when mess* calls itself */
    if(errmess)
	AJFREE(errmess);
    errmess = ajSysStrdup(mess);

    return errmess;
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
** @param [v] args [va_list] Variable length argument list
** @param [r] format [const char*] Format string
** @param [r] prefix [const char*] Message prefix
** @return [char*] Formatted message text
** @@
******************************************************************************/

static char* messFormat(va_list args, const char *format, const char *prefix)
{
    static char *new_buf = NULL;
    char *buf_ptr;
    ajint num_bytes;
    ajint prefix_len;


    if(format == NULL)
	ajMessCrash("invalid call, no format string.");

    if(prefix == NULL)
	prefix_len = 0;
    else
    {
	prefix_len = strlen(prefix);
	if((prefix_len + 1) > PREFIXSIZE)
	    ajMessCrash("prefix string is too long.");
    }

    /* If they supply our internal buffer as an argument, e.g. because they */
    /* used ajFmtString as an arg, then make a copy, otherwise use internal */
    /* buffer.                                                              */

    if(format == messbuf)
    {
	if(new_buf != NULL)
	    AJFREE(new_buf);
	buf_ptr = new_buf = ajSysStrdup(format);
    }
    else
	buf_ptr = messbuf;

    /* Add the prefix if there is one. */
    if(prefix != NULL)
    {
	if(!strcpy(buf_ptr, prefix))
	    ajMessCrash("strcpy failed");
    }


    num_bytes = prefix_len + 1;
    num_bytes += ajFmtVPrintCL((buf_ptr + prefix_len),BUFSIZE, format, args);

    /*
    **  Check the result. This should never happen using the
    **  ajFmtVPrintCL routine instead of the vsprintf routine
    */

    if(num_bytes < 0)
	ajMessCrash("vsprintf failed: %s", ajMessSysErrorText());
    else if(num_bytes > BUFSIZE)
	ajMessCrash("messubs internal buffer size (%d) exceeded, "
		    "a total of %d bytes were written",
		    BUFSIZE, num_bytes);

    return(buf_ptr);
}




/* @funcstatic messGetFilename ************************************************
**
** Converts a filename into a base file name. Used for filenames passed
** by macros from __FILE__ which could include part or all of the path
** depending on how the source code was compiled.
**
** @param [r] path [const char*] File name, possibly with full path.
** @return [char*] Base file name
** @@
******************************************************************************/

static char* messGetFilename(const char *path)
{
    static char *path_copy = NULL;
    const char *path_delim = SUBDIR_DELIMITER_STR;
    char *result = NULL;
    char *tmp;

    if(path != NULL)
    {
	if(strcmp((path + strlen(path) - 1), path_delim) != 0)
	{				/* Last char = "/" ?? */
	    if(path_copy != NULL)
		AJFREE(path_copy);
	    path_copy = ajSysStrdup(path);

	    tmp = ajSysStrtok(path_copy, path_delim);

	    while(tmp != NULL)
	    {
		result = tmp;	 /* Keep results of previous strtok */

		tmp = ajSysStrtok(NULL, path_delim);
	    }
	}
    }

    return(result);
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




/* @func ajMessErrorInit ******************************************************
**
** Initialises the stored program name.
**
** @param [r] progname [const char*] Program name.
** @return [void]
** @@
******************************************************************************/

void ajMessErrorInit(const char *progname)
{
    if(progname != NULL)
	messageG.progname = ajSysStrdup(messGetFilename(progname));

    return;
}




/* @func ajMessSetErr *********************************************************
**
** Stores the source file name (converted to a base name)
** and the source line number to be
** reported by the crash routines.
**
** Invoked automatically by a macro (e.g. ajFatal) where needed.
**
** @param [r] filename [const char*] source filename, __FILE__
** @param [r] line_num [ajint] source line number, __LINE__
** @return [void]
** @@
******************************************************************************/

void ajMessSetErr(const char *filename, ajint line_num)
{
    assert(filename != NULL && line_num != 0);

    /*
    ** We take the basename here because __FILE__ can be a path rather
    ** than just a filename, depending on how a module was compiled.
    */

    messageG.filename = ajSysStrdup(messGetFilename(filename));

    messageG.line_num = line_num;

    return;
}




/* Access functions for message error data.                                  */
/* @funcstatic messGetErrorProgram ********************************************
**
** Returns the stored program name.
**
** @return [char*] Program name
** @@
******************************************************************************/

static char* messGetErrorProgram(void)
{
    return(messageG.progname);
}




/* @funcstatic messGetErrorFile ***********************************************
**
** Returns the stored error file name.
**
** @return [char*] Error file name
** @@
******************************************************************************/

static char* messGetErrorFile(void)
{
    return(messageG.filename);
}




/* @funcstatic messGetErrorLine ***********************************************
**
** Returns the stored error source line number.
**
** @return [ajint] Original source code line number
** @@
******************************************************************************/

static ajint messGetErrorLine(void)
{
    return(messageG.line_num);
}




/* set a file to read for all the messages. NB if this is not set
Then a default one will be read */

/* @func ajMessErrorSetFile ***************************************************
**
** Opens a file and sets this to be the error file.
**
** @param [r] errfile [const char*] Error file name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajMessErrorSetFile(const char *errfile)
{
    FILE *fp = 0;

    if(errfile)
    {
	if((fp = fopen(errfile,"r")))
	{
	    messErrorFile = ajSysStrdup(errfile);
	    fclose(fp);
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

static AjBool ajMessReadErrorFile(void)
{
    char line[512];
    char name[12];
    char message[200];
    FILE *fp=0;
    char *mess;
    char *cp;
    char *namestore;
    char *messstore;

    if(messErrorFile)
	fp = fopen(messErrorFile,"r");

    if(!fp)
    {
	messErrorFile = ajFmtString("%s/messages/messages.english",
				    getenv("EMBOSS_ROOT"));
	fp = fopen(messErrorFile,"r");
    }

    if(!fp)
	return ajFalse;

    errorTable = ajStrTableNewC(0);

    while(fgets(line, 512, fp))
    {
	if(sscanf(line,"%s %s",name,message)!=2)
	    ajFatal("Library sscanf1");
	cp = strchr(line,'"');
	cp++;
	mess = &message[0];
	while(*cp != '"')
	{
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
	    ajErr("%s is listed more than once in file %s",
			name,messErrorFile);
	else
	    ajTablePut(errorTable, namestore, messstore);
    }

    return ajTrue;
}




/* @func ajMessOutCode ********************************************************
**
** Writes an output message for a given message code.
**
** @param [r] code [const char*] Message code
** @return [void]
** @@
******************************************************************************/

void ajMessOutCode(const char *code)
{
    char *mess=0;

    if(errorTable)
    {
	mess = ajTableGet(errorTable, code);
	if(mess)
	    ajMessOut(mess);
	else
	    ajMessOut("could not find error code %s",code);
    }
    else
    {
	if(ajMessReadErrorFile())
	{
	    mess = ajTableGet(errorTable, code);
	    if(mess)
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
** @param [r] code [const char*] Error code
** @return [void]
** @@
******************************************************************************/

void ajMessErrorCode(const char *code)
{
    char *mess = 0;

    if(errorTable)
    {
	mess = ajTableGet(errorTable, code);
	if(mess)
	    ajErr(mess);
	else
	    ajErr("could not find error code %s",code);
    }
    else
    {
	if(ajMessReadErrorFile())
	{
	    mess = ajTableGet(errorTable, code);
	    if(mess)
		ajErr(mess);
	    else
		ajErr("could not find error code %s",code);
	}
	else
	    ajErr("Could not read the error file, "
		  "hence no reference to %s",
		  code);
    }

    return;
}




/* @func ajMessCrashCodeFL ****************************************************
**
** Writes an error message for a given message code and crashes.
**
** @param [r] code [const char*] Error code
** @return [void]
** @@
******************************************************************************/

void ajMessCrashCodeFL(const char *code)
{
    char *mess = 0;

    if(errorTable)
    {
	mess = ajTableGet(errorTable, code);
	if(mess)
	    ajMessCrashFL(mess);
	else
	    ajMessCrashFL("could not find error code %s",code);
    }
    else
    {
	if(ajMessReadErrorFile())
	{
	    mess = ajTableGet(errorTable, code);
	    if(mess)
		ajMessCrashFL(mess);
	    else
		ajMessCrashFL("could not find error code %s",code);
	}
	else
	    ajMessCrashFL("Could not read the error file "
			  "hence no reference to %s",
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

void ajMessCodesDelete(void)
{
    void **array;
    ajint i;

    if(!errorTable)
	return;

    array =  ajTableToarray(errorTable, NULL);

    for(i = 0; array[i]; i += 2)
    {
	AJFREE(array[i+1]);
	AJFREE(array[i]);
    }
    AJFREE(array);

    ajTableFree(&errorTable);
    errorTable = 0;

    return;
}




/* @func ajDebug **************************************************************
**
** Writes a debug message to the debug file if debugging is on.
** Typically, debugging is turned on by adding '-debug' to the command line
** or by defining a variable prefix_DEBUG
**
** Avoid using this call in any code which can be invoked before the command
** line processing is complete as it can be a problem to find a reasonable
** file name for debug output under these circumstances.
**
** @param [r] fmt [const char*] Format.
** @param [v] [...] Variable argument list.
** @return [void]
** @@
******************************************************************************/

void ajDebug(const char* fmt, ...)
{
    va_list args;
    static ajint debugset = 0;
    static ajint depth    = 0;
    AjPStr bufstr         = NULL;
    
    if(depth)
    {				   /* recursive call, get out quick */
	if(fileDebugFile)
	{
	    va_start(args, fmt);
	    ajFmtVPrintF(fileDebugFile, fmt, args);
	    va_end(args);
	}
	return;
    }

    depth++;
    if(!debugset && acdDebugSet)
    {
	fileDebug = acdDebug;
	if(fileDebug)
	{
	    ajFmtPrintS(&fileDebugName, "%s.dbg", ajStrStr(acdProgram));
	    fileDebugFile = ajFileNewOut(fileDebugName);
	    if(!fileDebugFile)
		ajFatal("Cannot open debug file %S",fileDebugName);
	    if(ajNamGetValueC("debugbuffer", &bufstr))
	    {
		ajStrToBool(bufstr, &acdDebugBuffer);
	    }
	    if(!acdDebugBuffer)
		ajFileUnbuffer(fileDebugFile);
	    ajFmtPrintF(fileDebugFile, "Debug file %F buffered:%B\n",
			 fileDebugFile, acdDebugBuffer);
	    ajStrDel(&bufstr);
	}
	debugset = 1;
    }

    if(fileDebug)
    {
	va_start(args, fmt);
	ajFmtVPrintF(fileDebugFile, fmt, args);
	va_end(args);
    }

    depth--;

    return;
}




/* @func ajDebugFile **********************************************************
**
** Returns the file used for debug output, or NULL if no debug file is open.
**
** @return [FILE*] C runtime library file handle for debug output.
** @@
******************************************************************************/

FILE* ajDebugFile(void)
{
    if(!fileDebugFile)
	return NULL;

    return ajFileFp(fileDebugFile);
}




/* @func ajUserGet ************************************************************
**
** Writes a prompt to the terminal and reads one line from the user.
**
** @param [w] pthis [AjPStr*] Buffer for the user response.
** @param [r] fmt [const char*] Format string
** @param [v] [...] Variable argument list.
** @return [ajint] Length of response string.
** @@
******************************************************************************/

ajint ajUserGet(AjPStr* pthis, const char* fmt, ...)
{
    AjPStr thys;
    char *cp;
    va_list args;

    va_start(args, fmt);
    ajFmtVError(fmt, args);
    va_end(args);

    /* Must be > 1, reserved for fgets!! */
    ajStrModL(pthis,ajFileBuffSize());
    thys = pthis ? *pthis : 0;

    ajDebug("ajUserGet buffer len: %d res: %d ptr: %x\n",
	     ajStrLen(thys), ajStrSize(thys), thys->Ptr);

    cp = fgets(thys->Ptr, thys->Res, stdin);

    if(!cp)
    {				/* EOF or error */
	if(feof(stdin))
	    ajFatal("END-OF-FILE reading from user\n");
	else
	    ajFatal("Error reading from user\n");
    }

    thys->Len = strlen(thys->Ptr);
    if(thys->Ptr[thys->Len-1] == '\n')
    {
	thys->Ptr[--thys->Len] = '\0';
    }
    else
	ajErr("ajUserGet no newline seen\n");

    return thys->Len;
}




/* @func ajMessExit ***********************************************************
**
** Delete any static initialised values
**
** @return [void]
** @@
******************************************************************************/

void ajMessExit(void)
{
    ajFileClose(&fileDebugFile);
    ajStrDel(&fileDebugName);

    return;
}

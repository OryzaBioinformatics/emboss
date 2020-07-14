/*
**
** AJAX library routines
**
*/

#include "ajax.h"
#include <stdarg.h>
#include <pwd.h>

static AjBool utilBigendian;
static ajint utilBigendCalled = 0;

/* @func ajExit ***************************************************************
**
** Calls 'exit' with a successful code (zero).
**
** But first it calls some cleanup routines which can report on resource
** usage etc.
**
** @return [void]
** @@
******************************************************************************/

void ajExit (void) {
  ajDebug("\nFinal Summary\n=============\n\n");
  ajLogInfo();
  ajStrExit();
  ajRegExit();
  ajFileExit();
  ajFeatExit();
  ajAcdExit(ajFalse);
  ajNamExit();
  ajMemExit();
  ajMessExit();	      /* clears data for ajDebug - do this last!!!  */
  exit (0);
  return;
}

/* @func ajExitBad ************************************************************
**
** Calls 'exit' with an unsuccessful code (EXIT_FAILURE defined in stdlib.h).
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [ajint] Exit code
** @@
******************************************************************************/

ajint ajExitBad (void) {
  exit (EXIT_FAILURE);
  return EXIT_FAILURE;
}

/* @func ajLogInfo ************************************************************
**
** If a log file is in use, writes run details to end of file.
**
** @return [void]
** @@
******************************************************************************/

void ajLogInfo (void) {
  static AjPFile logf;
  static AjPStr logfile=NULL;
  static AjPStr uids=NULL;

  if (ajNamGetValueC("logfile", &logfile)) {
    logf = ajFileNewApp(logfile);
    if (!logf) return;
    ajUtilUid(&uids),
    ajFmtPrintF (logf, "%s\t%S\t%D\n",
		 ajAcdProgram(),
		 uids,
		 ajTimeTodayF("log"));
    ajFileClose(&logf);
  }

  return;
}

/* @func ajUtilUid ************************************************************
**
** Returns the user's userid
**
** @param [r] dest [AjPStr*] String to return result
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajUtilUid (AjPStr* dest) {

  ajint uid;
  struct passwd* pwd;

  ajDebug ("ajUtilUid\n");

  uid = getuid();
  if (!uid) {
    ajStrAssC (dest, "");
    return ajFalse;
  }

  ajDebug("  uid: %d\n", uid);
  pwd = getpwuid(uid);
  if (!pwd) {
    ajStrAssC (dest, "");
    return ajFalse;
  }

  ajDebug("  pwd: '%s'\n", pwd->pw_name);

  ajStrAssC (dest, pwd->pw_name);
  return ajTrue;
}

/* @func ajUtilBigendian ******************************************************
**
** Tests whether the host system uses big endian byte order.
**
** @return [AjBool] ajTrue if host is big endian.
** @@
******************************************************************************/

AjBool ajUtilBigendian (void) {
  static union lbytes {
    char chars[sizeof(ajint)];
    ajint i;
  } data;

  if (!utilBigendCalled) {
    utilBigendCalled = 1;
    data.i = 0;
    data.chars[0] = '\1';
    if (data.i == 1)
      utilBigendian = ajFalse;
    else
      utilBigendian = ajTrue;
  }

  return utilBigendian;
}

/* @func ajUtilRev2 ***********************************************************
**
** Reverses the byte order in a 2 byte integer.
**
** @param [u] sval [short*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajUtilRev2 (short* sval) {
  union lbytes {
    char chars[2];
    short s;
  } data, revdata;
  char* cs;
  char* cd;
  ajint i;

  data.s = *sval;
  cs = data.chars;
  cd =&revdata.chars[1];
  for (i=0; i < 2; i++)
  {
      *cd = *cs++;
      --cd;
  }

  *sval = revdata.s;

  return;
}

/* @func ajUtilRev4 ***********************************************************
**
** Reverses the byte order in a 4 byte integer.
**
** @param [u] ival [ajint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajUtilRev4 (ajint* ival) {
  union lbytes {
    char chars[4];
    ajint i;
  } data, revdata;
  char* cs;
  char* cd;
  ajint i;

  data.i = *ival;
  cs = data.chars;
  cd =&revdata.chars[3];
  for (i=0; i < 4; i++)
  {
      *cd = *cs++;
      --cd;
  }

  *ival = revdata.i;

  return;
}

/* @func ajUtilRev8 ***********************************************************
**
** Reverses the byte order in an 8 byte long.
**
** @param [u] ival [ajlong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajUtilRev8 (ajlong* ival) {
  union lbytes {
    char chars[8];
    ajlong l;
  } data, revdata;
  char* cs;
  char* cd;
  ajint i;

  data.l = *ival;
  cs = data.chars;
  cd =&revdata.chars[7];
  for (i=0; i < 8; i++)
  {
      *cd = *cs++;
      --cd;
  }

  *ival = revdata.l;

  return;
}
/* @func ajUtilCatch **********************************************************
**
** Dummy function to be called in special cases so it can be used when
** debugging in GDB.
**
** To use, simply put a call to ajUtilCatch() into your code, and use
** "break ajUtilCatch" in gdb to get a traceback.
**
** @return [void]
** @@
******************************************************************************/

void ajUtilCatch (void) {

  static ajint calls=0;

  calls = calls + 1;

  return;
}

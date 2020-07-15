/******************************************************************************
** @source AJAX utility functions
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 1998 Peter Rice
** @version 1.0
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

#include "ajax.h"
#include <stdarg.h>
#ifdef WIN32
#include "win32.h"
#include <winsock2.h>
#include <lmcons.h> /* for UNLEN */
#else
#include <pwd.h>
#include <unistd.h>
#endif



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

void ajExit(void)
{
#ifdef WIN32
    WSACleanup();
#endif
    ajDebug("\nFinal Summary\n=============\n\n");
    ajLogInfo();
    ajTableExit();
    ajListExit();
    ajFileExit();
    ajFeatExit();
    ajSeqExit();
    ajAlignExit();
    ajReportExit();
    ajAcdExit(ajFalse);
    ajNamExit();
    ajSysExit();
    ajCallExit();
    ajBaseExit();
    ajCodExit();
    ajTrnExit();
    ajMeltExit();
    ajTimeExit();
    ajRegExit();
    ajArrExit();
    ajStrExit();
    ajMemExit();
    ajMessExit();     /* clears data for ajDebug - do this last!!!  */
    exit(0);

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

ajint ajExitBad(void)
{
    exit(EXIT_FAILURE);

    return EXIT_FAILURE;
}




/* @func ajExitAbort **********************************************************
**
** Exits without flushing any files. Needed for exit from, for example,
** a failed system call (ajFileNewInPipe, and so on) where the parent
** process has open output files, and the child priocess needs to exit
** without affecting them. Failure to exit this way can mean the output
** buffer is flushed twice.
**
** Calls '-exit' with an unsuccessful code (EXIT_FAILURE defined in stdlib.h).
**
** No cleanup or reporting routines are called. Simply crashes.
**
** @return [ajint] Exit code
** @@
******************************************************************************/

ajint ajExitAbort(void)
{
    _exit(EXIT_FAILURE);

    return EXIT_FAILURE;
}




/* @func ajLogInfo ************************************************************
**
** If a log file is in use, writes run details to end of file.
**
** @return [void]
** @@
******************************************************************************/

void ajLogInfo(void)
{
    AjPFile logf;
    AjPStr logfname = NULL;
    AjPStr uids    = NULL;
    AjPTime today = NULL;

    today = ajTimeTodayF("log");
    
    if(ajNamGetValueC("logfile", &logfname))
    {
	logf = ajFileNewApp(logfname);
	if(!logf)
	    return;

	ajUtilUid(&uids),
	ajFmtPrintF(logf, "%S\t%S\t%D\n",
		    ajAcdGetProgram(),
		    uids,
		    today);
	ajStrDel(&uids);
	ajStrDel(&logfname);
	ajFileClose(&logf);
    }


    AJFREE(today);

    return;
}




/* @func ajUtilUid ************************************************************
**
** Returns the user's userid
**
** @param [w] dest [AjPStr*] String to return result
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajUtilUid(AjPStr* dest)
{
#ifndef WIN32
    ajint uid;
    struct passwd* pwd;

    ajDebug("ajUtilUid\n");

    uid = getuid();
    if(!uid)
    {
	ajStrAssignC(dest, "");
	return ajFalse;
    }

    ajDebug("  uid: %d\n", uid);
    pwd = getpwuid(uid);
    if(!pwd)
    {
	ajStrAssignC(dest, "");
	return ajFalse;
    }

    ajDebug("  pwd: '%s'\n", pwd->pw_name);

    ajStrAssignC(dest, pwd->pw_name);

    return ajTrue;

#else	/* WIN32 */
    char nameBuf[UNLEN+1];
    DWORD nameLen = UNLEN+1;

    ajDebug("ajUtilUid\n");

    if (GetUserName(nameBuf, &nameLen))
    {
	ajDebug("  pwd: '%s'\n", nameBuf);
	ajStrAssC (dest, nameBuf);
	return ajTrue;
    }

    ajStrAssC (dest, "");

    return ajFalse;
#endif
}




/* @func ajUtilBigendian ******************************************************
**
** Tests whether the host system uses big endian byte order.
**
** @return [AjBool] ajTrue if host is big endian.
** @@
******************************************************************************/

AjBool ajUtilBigendian(void)
{
    static union lbytes
    {
	char chars[sizeof(ajint)];
	ajint i;
    } data;

    if(!utilBigendCalled)
    {
	utilBigendCalled = 1;
	data.i           = 0;
	data.chars[0]    = '\1';

	if(data.i == 1)
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

void ajUtilRev2(short* sval)
{
    union lbytes
    {
	char chars[2];
	short s;
    } data, revdata;
    char* cs;
    char* cd;
    ajint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[1];

    for(i=0; i < 2; i++)
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

void ajUtilRev4(ajint* ival)
{
    union lbytes
    {
	char chars[4];
	ajint i;
    } data, revdata;
    char* cs;
    char* cd;
    ajint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[3];
    for(i=0; i < 4; i++)
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

void ajUtilRev8(ajlong* ival)
{
    union lbytes
    {
	char chars[8];
	ajlong l;
    } data, revdata;
    char* cs;
    char* cd;
    ajint i;
    
    data.l = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[7];
    for(i=0; i < 8; i++)
    {
	*cd = *cs++;
	--cd;
    }
    
    *ival = revdata.l;
    
    return;
}




/* @func ajUtilRevShort ******************************************************
**
** Reverses the byte order in a short integer.
**
** @param [u] sval [short*] Short integer in wrong byte order.
**                          Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajUtilRevShort(short* sval)
{
    union lbytes
    {
	char chars[8];
	short s;
    } data, revdata;
    char* cs;
    char* cd;
    ajint i;

    data.s = *sval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(short)-1];

    for(i=0; i < sizeof(short); i++)
    {
	*cd = *cs++;
	--cd;
    }

    *sval = revdata.s;

    return;
}




/* @func ajUtilRevInt *********************************************************
**
** Reverses the byte order in an integer.
**
** @param [u] ival [ajint*] Integer in wrong byte order.
**                        Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajUtilRevInt(ajint* ival)
{
    union lbytes
    {
	char chars[8];
	ajint i;
    } data, revdata;
    char* cs;
    char* cd;
    ajint i;

    data.i = *ival;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajint)-1];
    for(i=0; i < sizeof(ajint); i++)
    {
	*cd = *cs++;
	--cd;
    }

    *ival = revdata.i;

    return;
}




/* @func ajUtilRevLong *******************************************************
**
** Reverses the byte order in a long.
**
** @param [u] lval [ajlong*] Integer in wrong byte order.
**                           Returned in correct order.
** @return [void]
** @@
******************************************************************************/

void ajUtilRevLong(ajlong* lval)
{
    union lbytes
    {
	char chars[8];
	ajlong l;
    } data, revdata;
    char* cs;
    char* cd;
    ajint i;
    
    data.l = *lval;
    cs     = data.chars;
    cd     = &revdata.chars[sizeof(ajlong)-1];
    for(i=0; i < sizeof(ajlong); i++)
    {
	*cd = *cs++;
	--cd;
    }
    
    *lval = revdata.l;
    
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

void ajUtilCatch(void)
{
    static ajint calls = 0;

    calls = calls + 1;

    return;
}

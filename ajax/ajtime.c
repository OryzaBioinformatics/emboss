/******************************************************************************
** @source AJAX time functions
**
** @author Copyright (C) 1998 Ian Longden
** @author Copyright (C) 2003 Jon Ison    
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
#include "ajtime.h"
#include <time.h>




/* @datastatic TimePFormat ****************************************************
**
** Internal structure for known Ajax time formats
**
** @alias TimeSFormat
** @alias TimeOFormat
**
** @attr Name [char*] format name
** @attr Format [char*] C run time library time format string
** @@
******************************************************************************/

typedef struct TimeSFormat
{
    char* Name;
    char* Format;
} TimeOFormat;
#define TimePFormat TimeOFormat*




static TimeOFormat timeFormat[] =  /* formats for strftime */
{
    {"GFF", "%Y-%m-%d"},
    {"yyyy-mm-dd", "%Y-%m-%d"},
    {"dd Mon yyyy", "%d %b %Y"},
    {"day", "%d-%b-%Y"},
    {"daytime", "%d-%b-%Y %H:%M"},
    {"log", "%a %b %d %H:%M:%S %Y"},
    { NULL, NULL}
};




/* @func ajTimeTodayRef *******************************************************
**
** AJAX function to return today's time as an AjPTime object reference
** @return [const AjPTime] Pointer to static time object containing
**                         today's date/time
** @@
******************************************************************************/

const AjPTime ajTimeTodayRef(void)
{
    static AjPTime thys = NULL;
    time_t tim;
    
    tim = time(0);

    if(!thys)
	AJNEW0(thys);

    if(!ajTimeLocal(tim,thys))
        return NULL;

    thys->format = NULL;

    return thys;
}




/* @func ajTimeToday **********************************************************
**
** AJAX function to return today's time as an AjPTime object
** @return [AjPTime] Pointer to time object containing today's date/time
** @exception  'Mem_Failed' from memory allocations
** @@
******************************************************************************/

AjPTime ajTimeToday(void)
{
    AjPTime thys = NULL;
    time_t tim;
    
    tim = time(0);

    if(!thys)
	AJNEW0(thys);

    if(!ajTimeLocal(tim,thys))
        return NULL;

    thys->format = NULL;

    return thys;
}




/* @funcstatic TimeFormat *****************************************************
**
** AJAX function to return the ANSI C format for an AJAX time string
**
** @param [r] timefmt [const char*] AJAX time format
** @return [char*] ANSI C time format, or NULL if none found
** @@
******************************************************************************/

static char* TimeFormat(const char *timefmt)
{
    ajint i;
    AjBool ok    = ajFalse;
    char *format = NULL ;

    for(i=0; timeFormat[i].Name; i++)
	if(ajStrMatchCaseCC(timefmt, timeFormat[i].Name))
	{
	    ok = ajTrue;
	    break;
	}

    if(ok)
	format = timeFormat[i].Format;
    else
	ajWarn("Unknown date/time format %s", timefmt);
  
    return format;
}




/* @func ajTimeTodayF *********************************************************
**
** AJAX function to return today's time as an AjPTime object
** with a specified output format
**
** @param [r] timefmt [const char*] A controlled vocabulary of time formats
**
** @return [] [AjPTime] Pointer to time object containing today's date/time
** @exception  'Mem_Failed' from memory allocations
** @@
**
******************************************************************************/

AjPTime ajTimeTodayF(const char* timefmt)
{
    AjPTime thys = NULL;
    time_t tim;
    
    tim = time(0);

    if(!thys)
	AJNEW0(thys);

    if(!ajTimeLocal(tim,thys))
        return NULL;

    thys->format = TimeFormat(timefmt);

    return thys;
}




/* @func ajTimeTodayRefF ******************************************************
**
** AJAX function to return today's time as a static AjPTime object
** with a specified output format
**
** @param [r] timefmt [const char*] A controlled vocabulary of time formats
**
** @return [] [const AjPTime] Pointer to static time object containing
**                            today's date/time
** @@
**
******************************************************************************/

const AjPTime ajTimeTodayRefF(const char* timefmt)
{
    static AjPTime thys = NULL;
    time_t tim;
    
    tim = time(0);

    if(!thys)
	AJNEW0(thys);

    if(!ajTimeLocal(tim,thys))
        return NULL;

    thys->format = TimeFormat(timefmt);

    return thys;
}




/* @func ajTimeTrace **********************************************************
**
** Debug report on the contents of an AjPTime object
**
** @param [r] thys [const AjPTime] Time object
** @return [void]
** @@
******************************************************************************/

void ajTimeTrace(const AjPTime thys) 
{
    ajDebug("Time value trace '%D'\n", thys);
    ajDebug("format: '%s'\n", thys->format);

    return;
}




/* @func ajTimeSet ************************************************************
**
** Constructor for user specification of an arbitrary AjPTime object.
** Except for 'timefmt', the arguments are based upon the UNIX
** 'tm' time structure defined in the time.h header file.
** The range validity of numbers given are not checked.
**
** @param  [rN] timefmt [const char*] Time format to use
** @param  [rN] mday    [ajint]   Day of the month [1-31]
** @param  [rN] mon     [ajint]   Month [1-12]
** @param  [rN] year    [ajint]   Four digit year
** @return [AjPTime] An AjPTime object
** @@
******************************************************************************/

AjPTime ajTimeSet( const char *timefmt, ajint mday, ajint mon, ajint year)
{
    AjPTime thys;

    thys = ajTimeTodayF(timefmt) ;

    thys->time.tm_mday  = mday ;
    thys->time.tm_mon   = mon-1;
    if(year > 1899) year = year-1900;
    thys->time.tm_year  = year ;

    mktime(&thys->time);

    return thys ;
}




/* @func ajTimeLocal ************************************************************ 
** A localtime()/localtime_r() replacement for AjPTime objects
**
** @param  [r] timer [const time_t] Time
** @param  [w] thys [AjPTime] Time object
**
** @return [AjBool] true if successful
** @@
******************************************************************************/

AjBool ajTimeLocal(const time_t timer, AjPTime thys)
{
    struct tm *result;

#ifdef __ppc__
    result = localtime(&timer);
    if(!result)
	return ajFalse;

    thys->time.tm_sec = result->tm_sec;
    thys->time.tm_min = result->tm_min;
    thys->time.tm_mday = result->tm_mday;
    thys->time.tm_hour = result->tm_hour;
    thys->time.tm_mon  = result->tm_mon;
    thys->time.tm_year = result->tm_year;
#else
    result = (struct tm *)localtime_r(&timer,&thys->time);
    if(!result)
	return ajFalse;
#endif

    return ajTrue;
}




/* @func ajTimeNew ************************************************************
**
** Constructor for AjPTime object.
**
** @return [AjPTime] An AjPTime object
** @@
******************************************************************************/

AjPTime ajTimeNew(void)
{
    AjPTime thys = NULL;

    AJNEW0(thys);

    return thys ;
}




/* @func ajTimeDel ************************************************************
**
** Destructor for AjPTime object.
**
** @param [w] thys [AjPTime*] Time object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajTimeDel(AjPTime *thys)
{
    /* Check arg's */
    if(*thys==NULL)
	return;

    AJFREE(*thys);
    *thys = NULL;
    
    return;
}

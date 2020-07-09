#include "ajax.h"
#include "ajtime.h"
#include <time.h>

typedef struct TimeSFormat {
  char* Name;
  char* Format;
} TimeOFormat, *TimePFormat;

static TimeOFormat timeFormat[] = { /* formats for strftime */
  {"GFF", "%Y-%m-%d"},
  {"yyyy-mm-dd", "%Y-%m-%d"},
  {"dd Mon yyyy", "%d %b %Y"},
  {"log", "%a %b %d %H:%M:%S %Y"},
  { NULL, NULL}
};

/*
static const Except_T Null_AjPTime_Pointer  = { "NULL AjPTime variable pointer encountered!" };
*/

/* @func ajTimeToday **********************************************
**
** AJAX function to return today's time as an AjPTime object
** @return [AjPTime] Pointer to time object containing today's date/time
** @exception  'Mem_Failed' from memory allocations
** @@
*******************************************************************/
AjPTime ajTimeToday (void) {

  static AjPTime thys = NULL;
  const time_t tim = time(0);

  if (!thys) AJNEW0(thys);
  thys->time = localtime(&tim);
  thys->format = NULL;

  return thys;
}

/* @funcstatic TimeFormat **********************************************
**
** AJAX function to return the ANSI C format for an AJAX time string
**
** @param [r] timefmt [char*] AJAX time format
** @return [char*] ANSI C time format, or NULL if none found
** @@
*******************************************************************/
static char* TimeFormat(char *timefmt)
{
  int i;
  AjBool ok    = ajFalse;
  char *format = NULL ;

  for (i=0; timeFormat[i].Name; i++) {
    if (ajStrMatchCaseCC(timefmt, timeFormat[i].Name)) {
      ok = ajTrue;
      break;
    }
  }
  if (ok)
    format = timeFormat[i].Format;
  else
    ajWarn ("Unknown date/time format %s", timefmt);

  return format ;
}

/* @func ajTimeTodayF **********************************************
**
** AJAX function to return today's time as an AjPTime object
** with a specified output format
**
** @param [r] timefmt [char*] A controlled vocabulary of time formats
**
** @return [] [AjPTime] Pointer to time object containing today's date/time
** @exception  'Mem_Failed' from memory allocations
** @@
**
*******************************************************************/
AjPTime ajTimeTodayF (char* timefmt) {

  static AjPTime thys = NULL;
  const time_t tim = time(0);

  if (!thys) AJNEW0(thys);
  thys->time = localtime(&tim);

  thys->format = TimeFormat(timefmt) ;

  return thys;
}

/* @func ajTimeTrace *******************************************************
**
** Debug report on the contents of an AjPTime object
**
** @param [r] thys [AjPTime] Time object
** @return [void]
** @@
******************************************************************************/

void ajTimeTrace (AjPTime thys) {
  ajDebug ("Time value trace '%D'\n", thys);
  ajDebug ("format: '%s'\n", thys->format);
}

/* @func ajTimeSet **********************************************
**
** Constructor for user specification of an arbitrary AjPTime object.
** Except for 'timefmt', the arguments are based upon the UNIX 
** 'tm' time structure defined in the time.h header file.
** The range validity of numbers given are not checked.
**
** @param  [rN] timefmt [char*] Time format to use
** @param  [rN] mday    [int]   Day of the month [1-31]
** @param  [rN] mon     [int]   Month [1-12]
** @param  [rN] year    [int]   Four digit year
** @return [AjPTime] An AjPTime object
** @@
*******************************************************************/
AjPTime ajTimeSet( char *timefmt, int mday, int mon, int year) {
   AjPTime thys = ajTimeTodayF (timefmt) ;

   thys->time->tm_mday  = mday ;
   thys->time->tm_mon   = mon-1; 
   if (year > 1899) year = year-1900;
   thys->time->tm_year  = year ;    

   (void) mktime (thys->time);

   return thys ;
}

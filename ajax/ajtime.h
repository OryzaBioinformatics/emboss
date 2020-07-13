#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtime_h
#define ajtime_h
#include <time.h>

/* @data AjPTime **************************************************************
**
** AJAX time and data data structure. The same structure is used for both.
**
** @alias AjPDate
******************************************************************************/

typedef struct AjSTime {
  struct tm *time;
  char *format;
} AjOTime, *AjPTime, *AjPDate;


AjPTime ajTimeToday (void);
AjPTime ajTimeTodayF (char* timefmt);
void ajTimeTrace (AjPTime thys);

AjPTime ajTimeSet( char *timefmt, ajint mday, ajint mon, ajint year) ;

#endif

#ifdef __cplusplus
}
#endif

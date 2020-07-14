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
  struct tm time;
  char *format;
} AjOTime, AjODate;

#define AjPTime AjOTime*
#define AjPDate AjODate*

AjBool  ajTimeLocal(const time_t timer, AjPTime thys);
AjPTime ajTimeToday (void);
AjPTime ajTimeTodayF (const char* timefmt);
const AjPTime ajTimeTodayRef (void);
const AjPTime ajTimeTodayRefF (const char* timefmt);
void    ajTimeTrace (const AjPTime thys);
AjPTime ajTimeSet( const char *timefmt, ajint mday, ajint mon, ajint year) ;
AjPTime ajTimeNew(void);
void    ajTimeDel(AjPTime *thys);


#endif

#ifdef __cplusplus
}
#endif

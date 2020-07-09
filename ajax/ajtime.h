#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajtime_h
#define ajtime_h
#include <time.h>
typedef struct ajtime {
  struct tm *time;
  char *format;
} AJTIME, *AjPTime, *AjPDate;


AjPTime ajTimeToday (void);
AjPTime ajTimeTodayF (char* timefmt);
void ajTimeTrace (AjPTime thys);

AjPTime ajTimeSet( char *timefmt, ajint mday, ajint mon, ajint year) ;

#endif

#ifdef __cplusplus
}
#endif

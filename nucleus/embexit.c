/*  Last edited: Feb  7 16:02 2000 (pmr) */
/*
**
** EMBOSS exit routine
**
*/

#include "emboss.h"

/* @func  embExit ******************************************************
**
** Cleans up as necessary, and calls ajExit
**
** @return [void]
** @@
******************************************************************************/
void embExit (void) {

  ajExit ();
  return;
}

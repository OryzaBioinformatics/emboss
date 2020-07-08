/* Added to help with debugging support.  Includes macros as well as a
 * static function pldebug, which has a stdarg (vargarg) based argument list.
 * Debugging output is only enabled if DEBUG is defined prior to inclusion of
 * this file and the debug stream variable is set (can be set by -debug command
 * line option).
 *
*/

/*
    pldebug.h

    Copyright (C) 1995 by Maurice J. LeBrun

    Debugging support for PLplot.

    This software may be freely copied, modified and redistributed without
    fee provided that this copyright notice is preserved intact on all
    copies and modified copies. 
 
    There is no warranty or other guarantee of fitness of this software.
    It is provided solely "as is". The author(s) disclaim(s) all
    responsibility and liability with respect to this software's usage or
    its effect upon hardware or computer systems. 
*/

#ifndef __PLDEBUG_H__
#define __PLDEBUG_H__
#undef DEBUG
#include <stdarg.h>


/* For the truly desperate debugging task */

#ifdef DEBUG_ENTER
#define dbug_enter(a) \
if (plsc->debug) \
    (void) fprintf(stderr, "Entered %s (%s)\n", a, __FILE__);

#else
#define dbug_enter(a)
#endif

/* If we're using a debugging malloc, include the header file here */

/*
#ifdef DEBUGGING_MALLOC
#include <malloc.h>
#endif
*/


#endif	/* __PLDEBUG_H__ */

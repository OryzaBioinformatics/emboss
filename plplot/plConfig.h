/*
    plConfig.h.in

    Maurice LeBrun
    IFS, University of Texas at Austin
    18-Jul-1994

    Contains macro definitions that determine miscellaneous PLplot library
    configuration defaults, such as macros for bin, font, lib, and tcl
    install directories, and various system dependencies.  On a Unix
    system, typically the configure script builds plConfig.h from
    plConfig.h.in.  Elsewhere, it's best to hand-configure a plConfig.h
    file and keep it with the system-specific files.
*/

#ifndef __PLCONFIG_H__
#define __PLCONFIG_H__


/* All these should now be set by automake so ignore */
/* But leaving empty justin case we have to back track il 15/9/99 */


/* Define if on a POSIX.1 compliant system.  */
/*#define _POSIX_SOURCE 1*/

/* Define HAVE_UNISTD_H if unistd.h is available. */
/*#define HAVE_UNISTD_H 1*/

/* Define if you have vfork.h.  */
/* #undef HAVE_VFORK_H */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define to `char *' if <sys/types.h> doesn't define.  */
/* #undef caddr_t */

/* Define as the return type of signal handlers (int or void).  */
/* #undef RETSIGTYPE */

/* Define if you have the ANSI C header files.  */
/*#define STDC_HEADERS 1*/

/* Define vfork as fork if vfork does not work.  */
/* #undef vfork */

/* Define if popen is available.  */
/*#define HAVE_POPEN 1*/

/* Define if we're using a debugging malloc */
/* #undef DEBUGGING_MALLOC */

/* If you don't know what this is for, you shouldn't be using it */
/* #undef NOBRAINDEAD */

/* Define if fgetpos/fsetpos is busted */
/* #undef USE_FSEEK */

/* Define if [incr Tcl] is available */
/* #undef HAVE_ITCL */

/* Define if [incr Tk] is available */
/* #undef HAVE_ITK */

/* Install directories. */
/*
#define LIB_DIR "\nERROR please setenv PLPLOT_LIB emboss_dir/PLPLOT/lib\n"
#define BIN_DIR "Not used?"
#define TCL_DIR "TCL NOT installed"
*/

#endif	/* __PLCONFIG_H__ */

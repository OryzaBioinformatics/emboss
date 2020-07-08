/*
    plDevs.h.in

    Maurice LeBrun
    IFS, University of Texas at Austin
    18-Jul-1994

    Contains macro definitions that determine what device drivers are
    compiled into the PLplot library.  On a Unix system, typically the
    configure script builds plDevs.h from plDevs.h.in.  Elsewhere, it's
    best to hand-configure a plDevs.h file and keep it with the
    system-specific files.
*/

#ifndef __PLDEVS_H__
#define __PLDEVS_H__

#define PLD_plmeta 1
#define PLD_null 1
#define PLD_xterm 1
#define PLD_tek4010 1
#define PLD_tek4107 1
#define PLD_mskermit 1
#define PLD_vlt 1
#define PLD_versaterm 1
#define PLD_conex 1
/* #undef PLD_linuxvga */
#define PLD_dg300 1
#define PLD_ps 1
#define PLD_xfig 1
#define PLD_ljii 1
#define PLD_lj_hpgl 1
#define PLD_hp7470 1
#define PLD_hp7580 1
#define PLD_imp 1
#define PLD_xwin 1
/* #undef PLD_tk */
/* #undef PLD_dp */
#define PLD_pbm 1

#endif	/* __PLDEVS_H__ */

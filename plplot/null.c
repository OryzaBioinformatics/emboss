/* All device drivers: enabling macro renamed to PLD_<driver>, where <driver>
 * is xwin, ps, etc.  See plDevs.h for more detail.
*/

/*	null.c

	PLplot Null device driver.
*/
#include "plDevs.h"

#ifdef PLD_null

#include "plplotP.h"
#include "drivers.h"

/*--------------------------------------------------------------------------*\
 * plD_init_null()
 *
 * Initialize device (terminal).
\*--------------------------------------------------------------------------*/

void
plD_init_null(PLStream *pls)
{
    int xmin = 0;
    int xmax = PIXELS_X - 1;
    int ymin = 0;
    int ymax = PIXELS_Y - 1;

    float pxlx = (double) PIXELS_X / (double) LPAGE_X;
    float pxly = (double) PIXELS_Y / (double) LPAGE_Y;

    (void) pls;

/* Set up device parameters */

    plP_setpxl(pxlx, pxly);
    plP_setphy(xmin, xmax, ymin, ymax);
}

/*--------------------------------------------------------------------------*\
 * The remaining driver functions are all null.
\*--------------------------------------------------------------------------*/

void
plD_line_null(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
   (void) pls;
   (void) x1a;
   (void) y1a;
   (void) x2a;
   (void) y2a;
}

void
plD_polyline_null(PLStream *pls, short *xa, short *ya, PLINT npts)
{
   (void) pls;
   (void) xa;
   (void) ya;
   (void) npts;
}

void
plD_eop_null(PLStream *pls)
{
   (void) pls;
}

void
plD_bop_null(PLStream *pls)
{
   (void) pls;
}

void
plD_tidy_null(PLStream *pls)
{
   (void) pls;
}

void 
plD_state_null(PLStream *pls, PLINT op)
{
   (void) pls;
   (void) op;
}

void
plD_esc_null(PLStream *pls, PLINT op, void *ptr)
{
   (void) pls;
   (void) op;
   (void) ptr;
}

#else
int 
pldummy_null(void)
{
    return 0;
}

#endif				/* PLD_nulldev */

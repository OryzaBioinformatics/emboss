/* All drivers: pls->width now more sensibly handled.  If the driver supports
 * multiple widths, it first checks to see if it has been initialized
 * already (e.g. from the command line) before initializing it.  For drivers
 * that don't support multiple widths, pls->width is ignored.
*/

/*	dg300.c

	PLplot dg300 device driver.
*/
#include "plDevs.h"

#ifdef PLD_dg300

#include "plplotP.h"
#include "drivers.h"

/* top level declarations */

#define  DGX    639
#define  DGY    239

struct termattr {
    unsigned char com[4];
    unsigned char rom[4];
    unsigned char ram[4];
    unsigned char con[5];
    unsigned char eor;
} termattr;

/*--------------------------------------------------------------------------*\
 * plD_init_dg()
 *
 * Initialize device.
\*--------------------------------------------------------------------------*/

void
plD_init_dg(PLStream *pls)
{
/* Request terminal configuration report */

    (void) printf("\n\036\107\051\n");
    (void) scanf("%s", (char *) &termattr);
    while (getchar() != '\n');
    if (!strncmp((char *) &termattr.ram[0], "0000", 4)) {
	(void)
	    printf("Please wait while graphics interpreter is downloaded.\n");

    /* Need to download graphics interpreter. */

	(void) system("cat  /usr/local/src/g300/g300gci110.tx");
    }

/* Clear screen, Set pen color to green, Absolute positioning */

    (void) printf("\036\107\063\060\n\036\107\155\061\n\036\107\151\060\n");
    (void) printf("\036\107\042\061\n");

    pls->termin = 1;		/* Is an interactive device */

    plP_setpxl((PLFLT) (3.316 * 16), (PLFLT) (1.655 * 16));
    plP_setphy(0, DGX * 16, 0, DGY * 16);
}

/*--------------------------------------------------------------------------*\
 * plD_line_dg()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_dg(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    int xx1 = x1a, yy1 = y1a, xx2 = x2a, yy2 = y2a;

    (void) pls;

    (void) printf("LINE %d %d %d %d\n",
		  xx1 >> 4, yy1 >> 3, xx2 >> 4, yy2 >> 3);
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_dg()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void
plD_polyline_dg(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_dg(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_dg()
 *
 * End of page.  User must hit a <CR> to continue.
\*--------------------------------------------------------------------------*/

void
plD_eop_dg(PLStream *pls)
{
    (void) pls;

    (void) putchar('\007');
    (void) fflush(stdout);
    while (getchar() != '\n');
    (void) printf("ERASE\n");
}

/*--------------------------------------------------------------------------*\
 * plD_bop_dg()
 *
 * Set up for the next page.
\*--------------------------------------------------------------------------*/

void
plD_bop_dg(PLStream *pls)
{
    pls->page++;
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_dg()
 *
 * Close graphics file
\*--------------------------------------------------------------------------*/

void
plD_tidy_dg(PLStream *pls)
{
    (void) pls;

    (void) printf("\036\107\042\060\n");
    (void) fflush(stdout);
}

/*--------------------------------------------------------------------------*\
 * plD_state_dg()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void 
plD_state_dg(PLStream *pls, PLINT op)
{
    (void) pls;
    (void) op;
}

/*--------------------------------------------------------------------------*\
 * plD_esc_dg()
 *
 * Escape function.
\*--------------------------------------------------------------------------*/

void
plD_esc_dg(PLStream *pls, PLINT op, void *ptr)
{
    (void) pls;
    (void) op;
    (void) ptr;
}

#else
int 
pldummy_dg300(void)
{
    return 0;
}

#endif				/* PLD_dg300 */

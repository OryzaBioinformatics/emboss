/*	xfig.c

	PLplot xfig device driver.
*/
#include "plDevs.h"

#ifdef PLD_xfig

#include "plplotP.h"
#include "drivers.h"

/* Function prototypes */

static void flushbuffer(PLStream *);

/* top level declarations */

#define FIGX	599
#define FIGY	599
#define DPI	80
#define BSIZE	25

static short *buffptr, bufflen;
static short count;
static int curwid = 1;
static int firstline = 1;

/*--------------------------------------------------------------------------*\
 * plD_init_xfig()
 *
 * Initialize device.
\*--------------------------------------------------------------------------*/

void
plD_init_xfig(PLStream *pls)
{
    PLDev *dev;

/* Initialize family file info */

    plFamInit(pls);

/* Prompt for a file name if not already set */

    plOpenFile(pls);

/* Allocate and initialize device-specific data */

    dev = plAllocDev(pls);

    dev->xold = UNDEFINED;
    dev->yold = UNDEFINED;
    dev->xmin = 0;
    dev->xmax = FIGX;
    dev->ymin = 0;
    dev->ymax = FIGY;

    plP_setpxl(3.1496, 3.1496);	/* 80 DPI */

    plP_setphy(0, FIGX, 0, FIGY);

/* Write out header */

    (void) fprintf(pls->OutFile, "#FIG 1.4X\n");
    (void) fprintf(pls->OutFile, "%d 2\n", DPI);

    bufflen = 2 * BSIZE;
    buffptr = (short *) malloc(sizeof(short) * bufflen);
    if (buffptr == NULL)
	plexit("Out of memory!");
}

/*--------------------------------------------------------------------------*\
 * plD_line_xfig()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*--------------------------------------------------------------------------*/

void
plD_line_xfig(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    PLDev *dev = (PLDev *) pls->dev;
    int xx1 = x1a, yy1 = y1a, xx2 = x2a, yy2 = y2a;
    short *tempptr;

/* If starting point of this line is the same as the ending point of */
/* the previous line then don't raise the pen. (This really speeds up */
/* plotting and reduces the size of the file. */

    if (firstline) {
	count = 0;
	*(buffptr + count++) = xx1;
	*(buffptr + count++) = yy1;
	*(buffptr + count++) = xx2;
	*(buffptr + count++) = yy2;
	firstline = 0;
    }
    else if (xx1 == dev->xold && yy1 == dev->yold) {
	if (count + 2 >= bufflen) {
	    bufflen += 2 * BSIZE;
	    tempptr = (short *)
		realloc((void *) buffptr, bufflen * sizeof(short));
	    if (tempptr == NULL) {
		free((void *) buffptr);
		plexit("Out of memory!");
	    }
	    buffptr = tempptr;
	}
	*(buffptr + count++) = xx2;
	*(buffptr + count++) = yy2;
    }
    else {
	flushbuffer(pls);
	*(buffptr + count++) = xx1;
	*(buffptr + count++) = yy1;
	*(buffptr + count++) = xx2;
	*(buffptr + count++) = yy2;
    }
    dev->xold = xx2;
    dev->yold = yy2;
}

/*--------------------------------------------------------------------------*\
 * plD_polyline_xfig()
 *
 * Draw a polyline in the current color.
\*--------------------------------------------------------------------------*/

void
plD_polyline_xfig(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_xfig(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*--------------------------------------------------------------------------*\
 * plD_eop_xfig()
 *
 * End of page.
\*--------------------------------------------------------------------------*/

void
plD_eop_xfig(PLStream *pls)
{
    if (!firstline)
	flushbuffer(pls);
}

/*--------------------------------------------------------------------------*\
 * plD_bop_xfig()
 *
 * Set up for the next page.
 * Advance to next family file if necessary (file output).
\*--------------------------------------------------------------------------*/

void
plD_bop_xfig(PLStream *pls)
{
    PLDev *dev = (PLDev *) pls->dev;

    dev->xold = UNDEFINED;
    dev->yold = UNDEFINED;
    firstline = 1;

    if (!pls->termin)
	plGetFam(pls);

    pls->page++;
}

/*--------------------------------------------------------------------------*\
 * plD_tidy_xfig()
 *
 * Close graphics file or otherwise clean up.
\*--------------------------------------------------------------------------*/

void
plD_tidy_xfig(PLStream *pls)
{
    flushbuffer(pls);
    free((void *) buffptr);
    (void) fclose(pls->OutFile);
}

/*--------------------------------------------------------------------------*\
 * plD_state_xfig()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*--------------------------------------------------------------------------*/

void 
plD_state_xfig(PLStream *pls, PLINT op)
{
    switch (op) {

    case PLSTATE_WIDTH:
	flushbuffer(pls);
	firstline = 1;

	if (pls->width <= 1)
	    curwid = 1;
	else if (pls->width >= 4)
	    curwid = 3;
	else
	    curwid = (int) pls->width;

	break;

    case PLSTATE_COLOR0:
	break;

    case PLSTATE_COLOR1:
	break;
    }
}

/*--------------------------------------------------------------------------*\
 * plD_esc_xfig()
 *
 * Escape function.
\*--------------------------------------------------------------------------*/

void
plD_esc_xfig(PLStream *pls, PLINT op, void *ptr)
{
    (void) pls;
    (void) op;
    (void) ptr;
}

/*--------------------------------------------------------------------------*\
 * Utility functions.
\*--------------------------------------------------------------------------*/

static void
flushbuffer(PLStream *pls)
{
    short i = 0;

    if (count == 0)
	return;

    (void) fprintf(pls->OutFile, "2 1 0 %d 0 0 0 0 0.000 0 0\n", curwid);
    while (i < count) {
	(void) fprintf(pls->OutFile, "%d %d ", *(buffptr + i),
		FIGY - *(buffptr + i + 1));
	i += 2;
    }
    (void) fprintf(pls->OutFile, "9999 9999\n");
    count = 0;
}

#else
int 
pldummy_xfig(void)
{
    return 0;
}

#endif				/* PLD_xfig */

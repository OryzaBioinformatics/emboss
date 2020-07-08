/* All drivers: pls->width now more sensibly handled.  If the driver supports
 * multiple widths, it first checks to see if it has been initialized
 * already (e.g. from the command line) before initializing it.  For drivers
 * that don't support multiple widths, pls->width is ignored.
*/

/*	next.c

	PLplot NeXT display driver.

	Attention: this driver needs a supporter.  It may not even work
	any more.  Also, there have been many improvements to ps.c that
	were not incorporated here.  If you use this please consider
	becoming the official maintainer.  Email mjl@dino.ph.utexas.edu
	for more info.
*/
#include "plDevs.h"

#ifdef PLD_next

#include "plplotP.h"
#include "drivers.h"

/* top level declarations */

#define LINELENGTH      70
#define COPIES          1
#define XSIZE           540	/* 7.5" x 7.5"  (72 points equal 1 inch) */
#define YSIZE           540
#define ENLARGE         5
#define XPSSIZE         ENLARGE*XSIZE
#define YPSSIZE         ENLARGE*YSIZE
#define XOFFSET         18
#define YOFFSET         18
#define XSCALE          100
#define YSCALE          100
#define LINESCALE       100
#define ANGLE           90
#define PSX             XPSSIZE-1
#define PSY             YPSSIZE-1
#define OF		pls->OutFile
#define MIN_WIDTH	1		/* Minimum pen width */
#define MAX_WIDTH	10		/* Maximum pen width */
#define DEF_WIDTH	3		/* Default pen width */

static char outbuf[128];
static int llx = XPSSIZE, lly = YPSSIZE, urx = 0, ury = 0, ptcnt;

/*------------------------------------------------------------------------*\
 * plD_init_nx()
 *
 * Initialize device.
\*------------------------------------------------------------------------*/

void
plD_init_nx(PLStream *pls)
{
    PLDev *dev;

/* Allocate and initialize device-specific data */

    dev = plAllocDev(pls);

    dev->xold = UNDEFINED;
    dev->yold = UNDEFINED;
    dev->xmin = 0;
    dev->xmax = PSX;
    dev->ymin = 0;
    dev->ymax = PSY;

    plP_setpxl((PLFLT) 11.81, (PLFLT) 11.81);	/* 300 dpi */

    plP_setphy(0, PSX, 0, PSY);
}

/*------------------------------------------------------------------------*\
 * plD_line_nx()
 *
 * Draw a line in the current color from (x1,y1) to (x2,y2).
\*------------------------------------------------------------------------*/

void
plD_line_nx(PLStream *pls, short x1a, short y1a, short x2a, short y2a)
{
    PLDev *dev = (PLDev *) pls->dev;
    int x1 = x1a, y1 = y1a, x2 = x2a, y2 = y2a;
    int ori;

    if (pls->linepos + 21 > LINELENGTH) {
	(void) putc('\n', OF);
	pls->linepos = 0;
    }
    else
	(void) putc(' ', OF);

    pls->bytecnt++;

    if (x1 == dev->xold && y1 == dev->yold && ptcnt < 40) {
	(void) sprintf(outbuf, "%d %d D", x2, y2);
	ptcnt++;
    }
    else {
	sprintf(outbuf, "Z %d %d M %d %d D", x1, y1, x2, y2);
	llx = MIN(llx, x1);
	lly = MIN(lly, y1);
	urx = MAX(urx, x1);
	ury = MAX(ury, y1);
	ptcnt = 1;
    }
    llx = MIN(llx, x2);
    lly = MIN(lly, y2);
    urx = MAX(urx, x2);
    ury = MAX(ury, y2);

    (void) fprintf(OF, "%s", outbuf);
    pls->bytecnt += strlen(outbuf);
    dev->xold = x2;
    dev->yold = y2;
    pls->linepos += 21;
}

/*------------------------------------------------------------------------*\
 * plD_polyline_nx()
 *
 * Draw a polyline in the current color.
\*------------------------------------------------------------------------*/

void
plD_polyline_nx(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
	plD_line_nx(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*------------------------------------------------------------------------*\
 * plD_eop_nx()
 *
 * End of page.
\*------------------------------------------------------------------------*/

void
plD_eop_nx(PLStream *pls)
{
    (void) fprintf(OF, " S\neop\n");

    (void) pclose(OF);
}

/*------------------------------------------------------------------------*\
 * plD_bop_nx()
 *
 * Set up for the next page.
\*------------------------------------------------------------------------*/

void
plD_bop_nx(PLStream *pls)
{
    PLDev *dev = (PLDev *) pls->dev;

    dev->xold = UNDEFINED;
    dev->yold = UNDEFINED;

/* Pipe output to Preview */

    OF = popen("open", "w");

/* Header comments */

    (void) fprintf(OF, "%%!PS-Adobe-2.0 EPSF-2.0\n");
    (void) fprintf(OF, "%%%%Title: PLplot Graph\n");
    (void) fprintf(OF, "%%%%Creator: PLplot Version %s\n", PLPLOT_VERSION);
    (void) fprintf(OF, "%%%%BoundingBox: 0 0 576 576\n");
    (void) fprintf(OF, "%%%%EndComments\n\n");

/* Definitions */

/* - eop -  -- end a page */

    (void) fprintf(OF, "/eop\n");
    (void) fprintf(OF, "   {\n");
    (void) fprintf(OF, "    showpage\n");
    (void) fprintf(OF, "   } def\n");

/* Set line parameters */

    (void) fprintf(OF, "/@line\n");
    (void) fprintf(OF, "   {0 setlinecap\n");
    (void) fprintf(OF, "    0 setlinejoin\n");
    (void) fprintf(OF, "    1 setmiterlimit\n");
    (void) fprintf(OF, "   } def\n");

/* d @hsize -  horizontal clipping dimension */

    (void) fprintf(OF, "/@hsize   {/hs exch def} def\n");
    (void) fprintf(OF, "/@vsize   {/vs exch def} def\n");

/* d @hoffset - shift for the plots */

    (void) fprintf(OF, "/@hoffset {/ho exch def} def\n");
    (void) fprintf(OF, "/@voffset {/vo exch def} def\n");

/* s @hscale - scale factors */

    (void) fprintf(OF, "/@hscale  {100 div /hsc exch def} def\n");
    (void0 fprintf(OF, "/@vscale  {100 div /vsc exch def} def\n");

/* Set line width */

    (void) fprintf(OF, "/lw %d def\n", (int) (
	(pls->width < MIN_WIDTH) ? DEF_WIDTH :
	(pls->width > MAX_WIDTH) ? MAX_WIDTH : pls->width));

/* Setup user specified offsets, scales, sizes for clipping */

    (void) fprintf(OF, "/@SetPlot\n");
    (void) fprintf(OF, "   {\n");
    (void) fprintf(OF, "    ho vo translate\n");
    (void) fprintf(OF, "    XScale YScale scale\n");
    (void) fprintf(OF, "    lw setlinewidth\n");
    (void) fprintf(OF, "   } def\n");

/* Setup x & y scales */

    (void) fprintf(OF, "/XScale\n");
    (void) fprintf(OF, "   {hsc hs mul %d div} def\n", YPSSIZE);
    (void) fprintf(OF, "/YScale\n");
    (void) fprintf(OF, "   {vsc vs mul %d div} def\n", XPSSIZE);

/* Macro definitions of common instructions, to keep output small */

    (void) fprintf(OF, "/M {moveto} def\n");
    (void) fprintf(OF, "/D {lineto} def\n");
    (void) fprintf(OF, "/S {stroke} def\n");
    (void) fprintf(OF, "/Z {stroke newpath} def\n");
    (void) fprintf(OF, "/F {fill} def\n");
    (void) fprintf(OF, "/C {setrgbcolor} def\n");
    (void) fprintf(OF, "/G {setgray} def\n");
    (void) fprintf(OF, "/W {setlinewidth} def\n");

/* Set up the plots */

    (void) fprintf(OF, "@line\n");
    (void) fprintf(OF, "%d @hsize\n", YSIZE);
    (void) fprintf(OF, "%d @vsize\n", XSIZE);
    (void) fprintf(OF, "%d @hoffset\n", YOFFSET);
    (void) fprintf(OF, "%d @voffset\n", XOFFSET);
    (void) fprintf(OF, "%d @hscale\n", YSCALE);
    (void) fprintf(OF, "%d @vscale\n", XSCALE);
    (void) fprintf(OF, "@SetPlot\n\n");
    pls->page++;
    pls->linepos = 0;
}

/*------------------------------------------------------------------------*\
 * plD_tidy_nx()
 *
 * Close graphics file or otherwise clean up.
\*------------------------------------------------------------------------*/

void
plD_tidy_nx(PLStream *pls)
{
}

/*------------------------------------------------------------------------*\
 * plD_state_nx()
 *
 * Handle change in PLStream state (color, pen width, fill attribute, etc).
\*------------------------------------------------------------------------*/

void 
plD_state_nx(PLStream *pls, PLINT op)
{
    switch (op) {

    case PLSTATE_WIDTH:{
	int width = 
	    (pls->width < MIN_WIDTH) ? DEF_WIDTH :
	    (pls->width > MAX_WIDTH) ? MAX_WIDTH : pls->width;

	(void) fprintf(OF, " S\n%d W", width);

	dev->xold = UNDEFINED;
	dev->yold = UNDEFINED;
	break;
    }
    case PLSTATE_COLOR0:
	break;

    case PLSTATE_COLOR1:
	break;
    }
}

/*------------------------------------------------------------------------*\
 * plD_esc_nx()
 *
 * Escape function.
\*------------------------------------------------------------------------*/

void
plD_esc_nx(PLStream *pls, PLINT op, void *ptr)
{
}

#else
int 
pldummy_next()
{
    return 0;
}

#endif			/* PLD_next */

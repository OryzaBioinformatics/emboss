/*
*
* Version 2 of my PLplot PNG driver using the PNG-only gd library.
* Not copyrighted (c) 1999 James R. Phillips.  No rights reserved
* under US and international copyright laws.  This lack of a
* copyright notice is not required to be present under penalty
* of law if you use this driver in any way.  You have not been warned.
*
* You may notice that using the latest gd library there is not
* much actual programming per se required to write this driver.
*
* Send any questions, comments, or ridicule to Randy at randy-san@zunzun.com.
*/


#ifdef PLD_png

#include "plDevs.h"
#include "plplotP.h"
#include "drivers.h"


/* the gd PNG library header */
#include "gd.h"

static void  fill_polygon	(PLStream *pls);

/*
* My idea is that these globally defined height and width parameters
* are easily changed elsewhere before making a PNG file by using
*
* extern int PNGWidth, PNGHeight;
*
* and then setting the height and width as needed.  Bad programming?
* Maybe, but it works for me...
*/


int PNGWidth = 640;
int PNGHeight  = 480;

/* 2* default size */

/* int PNGWidth = 1280; */
/* int PNGHeight  = 960; */


int PNGColor  = 16; /* you should not need to use this*/

/*  a gd PNG pointer for output */
gdImagePtr im_out;


void
plD_init_png(PLStream *pls)
{
    int colorcount, red, green, blue;

  /* 
     in case of PNG family problems, turn this on to get a trace
     whenever the PNG init function is called.

     Most likely trouble is the way init is followed by bop,
     and the PNG bop function calls plGetFam which then
     ends up calling init again
 */

  /*
       FILE* fp;
      char fname[256];
      static int i=1;

      sprintf (fname, "plinitpng.%d", i++);
      fp = fopen (fname, "w");
      plxtrace(fp);
      fclose (fp);
  */

    pls->termin = 0;		/* not an interactive terminal */
    pls->icol0 = 1;
    pls->color = 0;
    pls->width = 1;
    pls->bytecnt = 0;
    pls->page = 0;
    pls->dev_fill0 = 1;		/* Can do solid fills */
    pls->family = 1;            /* il  allow multiple pages. */

/* Initialize family file info */
    plFamInit(pls);

    /*  Prompt for a file name if not already set */
    plOpenFile(pls);

    plP_setpxl(2.0, 2.0); /*  1.0 and 1.0 works for me... BUT 2.0 BETTER FOR IL. */

    plP_setphy(0, PNGWidth-1, 0, PNGHeight-1);

    /*     make a blank PNG image... */
    im_out = gdImageCreate(PNGWidth, PNGHeight);

    /*
     * copy the plplot color table to the PNG color table.  The first
     * color in this table will be the PNG background color.
    */

    for (colorcount = 0; colorcount < 16; colorcount++){
        plgcol0(colorcount, &red, &green, &blue);
        gdImageColorAllocate(im_out, red, green, blue);
    }

}

/*----------------------------------------------------------------------*\
* Draw a line in the current color from (x1,y1) to (x2,y2).
\*----------------------------------------------------------------------*/

void
plD_line_png(PLStream *pls, short x1, short y1, short x2, short y2)
{
    gdImageLine(im_out, x1, PNGHeight-1 - y1, x2, PNGHeight-1 - y2, PNGColor);
}

/*----------------------------------------------------------------------*\
* Draw a polyline in the current color.
\*----------------------------------------------------------------------*/

void
plD_polyline_png(PLStream *pls, short *xa, short *ya, PLINT npts)
{
    PLINT i;

    for (i = 0; i < npts - 1; i++)
        plD_line_png(pls, xa[i], ya[i], xa[i + 1], ya[i + 1]);
}

/*----------------------------------------------------------------------*\
* End of page.
\*----------------------------------------------------------------------*/

void
plD_eop_png(PLStream *pls)
{
    /* Write PNG */
  
    gdImagePng(im_out, pls->OutFile);

    (void) fclose(pls->OutFile);
    pls->OutFile = NULL; /* just making sure ;-) */

    /*   blank the image... */
    gdImageFilledRectangle(im_out, 0, 0, PNGWidth-1, PNGHeight-1, 0);
}

/*----------------------------------------------------------------------*\
* Set up for the next page.
* Advance to next family file if necessary (file output).
\*----------------------------------------------------------------------*/

void
plD_bop_png(PLStream *pls)
{
  /* in case of PNG family problems, turn this on to get a trace
     whenever the PNG bop function is called. */

  /*
      FILE* fp;
      char fname[256];
      static int i=1;

      sprintf (fname, "plboppng.%d", i++);
      fp = fopen (fname, "w");
      plxtrace(fp);
      fclose (fp);
  */
    if(!pls->level)
      return;

    if (pls->family){
        if (!pls->OutFile)
	  pls->famadv = 1; /* force new file if family */
        plGetFam(pls);
    }
    pls->page++;
}

/*----------------------------------------------------------------------
* Close graphics file or otherwise clean up.
----------------------------------------------------------------------*/

void
plD_tidy_png(PLStream *pls)
{
    gdImageDestroy(im_out);
}


void plD_state_png(PLStream *pls,PLINT op)
{
    switch(op) {
      case PLSTATE_COLOR0:
        PNGColor = (int)pls->icol0; /* colors were made equal in plD_bop_PNG */
    }
}


/*----------------------------------------------------------------------
* Escape function.
----------------------------------------------------------------------*/

void
plD_esc_png(PLStream *pls, PLINT op, void *ptr)
{
    switch (op) {
      case PLESC_FILL:
			fill_polygon(pls);
	break;
    }
}

/*----------------------------------------------------------------------
 * fill_polygon()
 *
 * Fill polygon described in points pls->dev_x[] and pls->dev_y[].
 * Only solid color fill supported.
----------------------------------------------------------------------*/

static void
fill_polygon(PLStream *pls)
{

   int n;

	gdPoint *points = malloc(pls->dev_npts * sizeof(gdPoint));

	for (n = 0; n < pls->dev_npts; n++) {
		points[n].x = pls->dev_x[n];
		points[n].y = PNGHeight - pls->dev_y[n];
   }

	gdImageFilledPolygon(im_out, points, pls->dev_npts, PNGColor);

   free((void *)points);
}

int
pldummy_PNG()
{
    return 0;
}

#else
static int pngdummy()
{
    return 0;
}

#endif

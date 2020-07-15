/* Plots a simple stripchart.
 * Eventually I want a really cool demo here: slowly evolving plots of
 * say density and temperature.  These only need to get updated every so
 * often.  And then at the bottom some strip charts of energy or such
 * that are continually updated.
 */

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <string.h>

#include "plplotP.h"
#include "plplot.h"


/* Data declarations for stripcharts. */

#define PEN	4

typedef struct {
    PLFLT xmin, xmax, ymin, ymax, xjump, xlen;
    PLFLT wxmin, wxmax, wymin, wymax;	/* FIXME - some redundancy might exist */
    char *xspec, *yspec, *labx, *laby, *labtop;
    PLINT y_ascl, acc, colbox, collab;
    PLFLT xlpos, ylpos;
    PLFLT *x[PEN], *y[PEN];
    PLINT npts[PEN], nptsmax[PEN];
	PLINT colline[PEN], styline[PEN];
	char *legline[PEN];
} PLStrip;

static int sid;					/* strip id number */
static PLStrip *strip[100];		/* Array of pointers */
static PLStrip *stripc;			/* current strip chart */

/* Generates a complete stripchart plot.  */

static void
plstrip_gen(PLStrip *strip);

/* draw legend */

static void
plstrip_legend(PLStrip *strip, int flag);

/*char *strdup(const char *);*/
/*--------------------------------------------------------------------------*\
 * plstripc
 *
 * Create 1d stripchart.
\*--------------------------------------------------------------------------*/

void
plstripc(PLINT *id, char *xspec, char *yspec,
	PLFLT xmin, PLFLT xmax, PLFLT xjump, PLFLT ymin, PLFLT ymax,
	PLFLT xlpos, PLFLT ylpos,
	PLINT y_ascl, PLINT acc,
	PLINT colbox,  PLINT collab,
	PLINT *colline,  PLINT *styline, char *legline[],
	char *labx, char *laby, char *labtop)
{
    int i;

/* Get a free strip id and allocate it */

    for (i = 0; i < 100; i++)
		if (strip[i] == NULL)
		    break;

    if (i == 100) {
		plabort("plstripc: Cannot create new strip chart");
		*id = -1;
		return;
    }
    else {
		sid = *id = i;
		strip[sid] = (PLStrip *) calloc(1, (size_t) sizeof(PLStrip));
		if (strip[sid] == NULL) {
	    	plabort("plstripc: Out of memory.");
	    	*id = -1;
	    	return;
	    }
    }

/* Fill up the struct with all relevant info */

    stripc = strip[sid];

    for (i=0; i<PEN; i++) {
	    stripc->npts[i] =  0;
	    stripc->nptsmax[i] =  100;
	    stripc->colline[i] = colline[i];
	    stripc->styline[i] = styline[i];
            stripc->legline[i] = strdup(legline[i]); /* strdup() is needed because value must persist after call has finished */
	    stripc->x[i] = (PLFLT *) malloc((size_t) sizeof(PLFLT) * stripc->nptsmax[i]);;
	    stripc->y[i] = (PLFLT *) malloc((size_t) sizeof(PLFLT) * stripc->nptsmax[i]);;
	    if (stripc->x[i] == NULL || stripc->y[i] == NULL) {
			plabort("plstripc: Out of memory.");
			plstripd(sid);
			*id = -1;
			return;
		}
	}

    stripc->xlpos = xlpos;		/* legend position [0..1] */
    stripc->ylpos = ylpos;	
    stripc->xmin = xmin;		/* initial bounding box */
    stripc->xmax = xmax;
    stripc->ymin = ymin;
    stripc->ymax = ymax;
    stripc->xjump = xjump;		/* jump x step(%) when x attains xmax (xmax is then set to xmax+xjump) */
    stripc->xlen = xmax - xmin;	/* lenght of x scale */
    stripc->y_ascl = y_ascl;	/* autoscale y between x jump scale */
    stripc->acc = acc;			/* accumulate plot (not really stripchart) */
    stripc->xspec = strdup(xspec);	/* x axis specification */
    stripc->yspec = strdup(yspec);	/* strdup() is needed because value must persist after call has finished */
    stripc->labx = strdup(labx);		/* x label */
    stripc->laby = strdup(laby);
    stripc->labtop = strdup(labtop);	/* title */
    stripc->colbox = colbox;	/* box color */
    stripc->collab = collab;	/* label color */

/* Generate the plot */

    plstrip_gen(stripc);
	plstrip_legend(stripc,1);
}

void plstrip_legend(PLStrip *mystripc, int first)
{
	int i;
	PLFLT sc, dy;
	
    /* draw legend */
	
	plgchr(&sc, &dy);
	sc = dy = dy/100;
	plwind(-0.01, 1.01, -0.01, 1.01);
	for (i=0; i<PEN; i++) {
		if (mystripc->npts[i] || first) {
			plcol(mystripc->colline[i]);
			pllsty(mystripc->styline[i]);
			pljoin(mystripc->xlpos, mystripc->ylpos - sc,
			       mystripc->xlpos + 0.1, mystripc->ylpos - sc);
			plcol(mystripc->collab);
			plptex(mystripc->xlpos + 0.11, mystripc->ylpos - sc,
			       0., 0., 0, mystripc->legline[i]);sc += dy;
		}
	}
    plwind(mystripc->xmin, mystripc->xmax, mystripc->ymin, mystripc->ymax);
    plflush();
}

/*--------------------------------------------------------------------------*\
 * plstrip_gen
 *
 * Generates a complete stripchart plot.  Used either initially or
 * during rescaling.
\*--------------------------------------------------------------------------*/
PLFLT oxm,oxM, oym,oyM;
void
plstrip_gen(PLStrip *mystrip)
{
	int i;
	PLFLT x[]={0.,1.,1.,0.}, y[]={0.,0.,1.,1.};

/* Set up window */

/*	dont know how to clear only one subwindow. Any hints?) */

    /*
//	plbop();
//	pleop();
//	pladv(0);

//	what about this way? 
    */
		plvpor(0,1,0,1);
		plwind(0,1,0,1);
		plcol(0);plpsty(0);
		plfill(4, &x[0], &y[0]);
		plvsta();

/* Draw box and same window dimensions */
 mystrip->wxmin=mystrip->xmin; mystrip->wxmax=mystrip->xmax;
 mystrip->wymin=mystrip->ymin; mystrip->wymax=mystrip->ymax; /* FIXME - can exist some redundancy here */

    plwind(mystrip->xmin, mystrip->xmax, mystrip->ymin, mystrip->ymax);
    
	pllsty(1);
    plcol(mystrip->colbox);
    plbox(mystrip->xspec, 0.0, 0, mystrip->yspec, 0.0, 0);

    plcol(mystrip->collab);
    pllab(mystrip->labx, mystrip->laby, mystrip->labtop);
	 
    for (i=0; i<PEN; i++) {
	    if (mystrip->npts[i] > 0) {
			plcol(mystrip->colline[i]);pllsty(mystrip->styline[i]);
			plline(mystrip->npts[i], mystrip->x[i], mystrip->y[i]);
	    }
    }

	plstrip_legend(mystrip,0);
}

/*--------------------------------------------------------------------------*\
 * plstripa
 *
 * Add a point to a stripchart.  
 * Allocates memory and rescales as necessary.
\*--------------------------------------------------------------------------*/

void
c_plstripa(PLINT id, PLINT p, PLFLT x, PLFLT y)
{
    int j, yasc=0, istart;

    if (p >= PEN) {
    	plabort("Non existent pen");
    	return;
    }

    if (strip[id] == NULL) {
    	plabort("Non existent stripchart");
    	return;
    }

    stripc = strip[id];

/* Add new point, allocating memory if necessary */

    if (++stripc->npts[p] > stripc->nptsmax[p]) {
		stripc->nptsmax[p] += 32;
		stripc->x[p] = (PLFLT *) realloc((void *) stripc->x[p], sizeof(PLFLT)*stripc->nptsmax[p]);
		stripc->y[p] = (PLFLT *) realloc((void *) stripc->y[p], sizeof(PLFLT)*stripc->nptsmax[p]);
		if (stripc->x[p] == NULL || stripc->y[p] == NULL) {
			plabort("plstripc: Out of memory.");
			plstripd(id);
			return;
		}
    }
    
    stripc->x[p][stripc->npts[p]-1] = x;
    stripc->y[p][stripc->npts[p]-1] = y;

/*    if ( x > stripc->xmax) */
	    stripc->xmax = x;
	    
	if (stripc->y_ascl == 1 && (y > stripc->ymax || y < stripc->ymin))
		yasc=1;
/*		
    stripc->ymax = MAX(y, stripc->ymax);
    stripc->ymin = MIN(y, stripc->ymin);
*/
	if (y > stripc->ymax)
	  stripc->ymax = stripc->ymin + 1.1*(y - stripc->ymin);
	if (y < stripc->ymin)
	  stripc->ymin = stripc->ymax - 1.1*(stripc->ymax - y);

/* Now either plot new point or regenerate plot */

        if (stripc->xmax - stripc->xmin < stripc->xlen) {
            if( yasc == 0) {

            /* If user has changed subwindow, make shure we have the
               correct one */
                plvsta();
                plwind(stripc->wxmin, stripc->wxmax, stripc->wymin, stripc->wymax); /* FIXME - can exist some redundancy here */

		plcol(stripc->colline[p]); pllsty(stripc->styline[p]);
		plP_movwor(stripc->x[p][stripc->npts[p]-2], stripc->y[p][stripc->npts[p]-2]);
		plP_drawor(stripc->x[p][stripc->npts[p]-1], stripc->y[p][stripc->npts[p]-1]);
		plflush();
            }
            else {
		stripc->xmax = stripc->xmin + stripc->xlen;
                plstrip_gen(stripc);
	    }
	}
    else {
/* Regenerating plot */
	if (stripc->acc == 0) {
		for (j=0; j<PEN; j++) {
			if (stripc->npts[j] > 0) {
				istart = 0;
				while (stripc->x[j][istart] < stripc->xmin + stripc->xlen*stripc->xjump)
				    istart++;
			
				stripc->npts[j] = stripc->npts[j] - istart;
	/* make it faster
				for (i = 0; i < stripc->npts[j]; i++) {
				    stripc->x[j][i] = stripc->x[j][i+istart];
				    stripc->y[j][i] = stripc->y[j][i+istart];
				}
	*/
				memcpy( &stripc->x[j][0], &stripc->x[j][istart], (stripc->npts[j])*sizeof(PLFLT));
				memcpy( &stripc->y[j][0], &stripc->y[j][istart], (stripc->npts[j])*sizeof(PLFLT));
			}
		}
	} else
		stripc->xlen = stripc->xlen * (1 + stripc->xjump);

		stripc->xmin = stripc->x[p][0];
		stripc->xmax = stripc->xmax + stripc->xlen*stripc->xjump;

		plstrip_gen(stripc);
    }
}

/*--------------------------------------------------------------------------*\
 * plstripd
 *
 * Deletes and releases memory used by a stripchart.  
\*--------------------------------------------------------------------------*/

void
c_plstripd(PLINT id)
{
	int i;
    stripc = strip[id];

    if (stripc == NULL) {
    	plwarn("No such stripchart");
    	return;
    }
    
    for (i=0; i<PEN; i++) {
    	if (stripc->npts[i]) {
	    	free((void *) stripc->x[i]);
		    free((void *) stripc->y[i]);
		    free((void *)stripc->legline[i]);
	    }
    }

	free((void *)stripc->xspec);
	free((void *)stripc->yspec);
	free((void *)stripc->labx);
	free((void *)stripc->laby);
	free((void *)stripc->labtop);    
    free((void *) stripc);
    strip[id] = NULL;
}

/********************************************************************
** @source AJAX GRAPH (ajax graphics) functions
** @author Ian Longden
** These functions control all aspects of AJAX graphics.
**
** @version 1.0 
** @modified 1988-11-12 pmr First version
** @modified 1999 ajb ANSI
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
********************************************************************/

#define GRAPHMAIN 1     
#include "ajax.h"
#include "ajgraph.h"
#include "ajtime.h"
#include <math.h>
#include <limits.h>
#include <float.h>
#define AZ 28

enum ajGraphObjectTypes { RECTANGLE, RECTANGLEFILL, TEXT, LINE};

static char *colournum[] = { "BLACK", "RED", "YELLOW", "GREEN", "AQUAMARINE",
		"PINK", "WHEAT", "GREY", "BROWN", "BLUE", "BLUEVIOLET",
		"CYAN", "TURQUOISE", "MAGENTA", "SALMON", "WHITE"};


static void     GraphArray (int numofpoints, float *x, float *y);
static void     GraphArrayGaps(int numofpoints, float *x, float *y);
static void     GraphArrayGapsI(int numofpoints, int *x, int *y);
static void     GraphCharSize (float size);
static void     GraphCheckFlags (int flags);
static void     GraphCheckPoints (PLINT n, PLFLT *x, PLFLT *y);
static void     GraphClose (void);
static void     GraphDataObjDel (AjPGraphData graphs);
static void     GraphDataObjDraw (AjPGraphData graphs);
static void     GraphDataObjPrint (AjPGraphData graphs);
static void     GraphFill (int numofpoints, float *x, float *y);
static void     GraphFillPat (int pat);
static void     GraphInit (AjPGraph graphs);
static void     GraphLineStyle (int style);
static void     GraphListDevicesarg (char* name, va_list args);
static void     GraphObjDel (AjPGraph graphs);
static void     GraphObjDraw (AjPGraph graphs);
static void     GraphObjPrint (AjPGraph graphs);
static void     GraphOpenFile (AjPGraph graphs, char *ext);
static void     GraphOpenXwin (AjPGraph graphs, char *ext);
static void     GraphPen (int pen, int red, int green, int blue);
static void     GraphRegister (void);
static AjBool   GraphSet2 (AjPGraph thys, AjPStr type,AjBool *res);
static AjBool   GraphSetarg (char *name, va_list args);
static void     GraphSetName (AjPGraph thys, AjPStr txt, char *ext);
static void     GraphSetNumSubPage (int numofsubpages);
static void     GraphSetPen (int colour);
static void     GraphSetWin (float xmin, float xmax, float ymin, float ymax);
static void     GraphSetWin2 (float xmin, float xmax, float ymin, float ymax);
static void     GraphSubPage (int page);
static void     GraphSymbols (float *x1, float *y1, int numofdots,int symbol);
static void     GraphText (float x1, float y1, float x2, float y2,
			   float just,char *text);
static AjBool   GraphTracearg (char *name, va_list args);
static void     GraphWind (float xmin, float xmax, float ymin, float ymax);
static void     GraphxyDisplayToData (AjPGraph graphs, AjBool closeit,
				      char *ext);
static void     GraphxyDisplayToFile (AjPGraph graphs, AjBool closeit,
				      char *ext);
static void     GraphxyDisplayXwin (AjPGraph graphs, AjBool closeit,
				    char *ext);
static void     GraphxyGeneral (AjPGraph graphs, AjBool closeit);
static void     GraphxyInitData (AjPGraphData graph);
static AjPGraph GraphxyNewIarg (char *name, va_list args);
static AjBool   GraphxySet2 (AjPGraph thys, AjPStr type,AjBool *res);
static AjBool   GraphxySetarg (char *name, va_list args);
static AjBool   GraphxySetOutarg (char *name, va_list args);
static AjBool   GraphxySubtitlearg (char *name, va_list args);
static AjBool   GraphxyTitlearg (char *name, va_list args);
static AjBool   GraphxyXtitlearg (char *name, va_list args);
static AjBool   GraphxyYtitlearg (char *name, va_list args);


typedef struct GraphSType {
  char* Name;			/* Name recognized by AJAX calls */
  char* Device;			/* PLPLOT name */
  char* ext;			/* file extension */
  void (*XYDisplay) (AjPGraph thys, AjBool closeit, char *ext);
  void (*GOpen) (AjPGraph thys, char *ext);
} GraphOType, *GraphPType;

static GraphOType graphType[] = {
  {"postscript", "ps",      ".ps",   GraphxyDisplayToFile, GraphOpenFile},
  {"ps",         "ps",      ".ps",   GraphxyDisplayToFile, GraphOpenFile},
  {"hpgl",       "lj_hpgl", ".hpgl", GraphxyDisplayToFile, GraphOpenFile},
  {"hp7470",     "hp7470",  ".hpgl", GraphxyDisplayToFile, GraphOpenFile},
  {"hp7580",     "hp7580",  ".hpgl", GraphxyDisplayToFile, GraphOpenFile},
  {"meta",       "plmeta",  ".meta", GraphxyDisplayToFile, GraphOpenFile},
  {"colourps",   "psc",     ".ps",   GraphxyDisplayToFile, GraphOpenFile},
  {"cps",        "psc",     ".ps",   GraphxyDisplayToFile, GraphOpenFile},
#ifndef X_DISPLAY_MISSING /* X not available */
  {"xwindows",   "xwin",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"x11",        "xwin",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
#endif
  {"tektronics", "tekt",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"tekt",       "tekt",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"tek4107t",   "tek4107t","null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"tek",        "tek4107t","null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"none",       "null",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"null",       "null",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"text",       "null",    "null",  GraphxyDisplayXwin,   GraphOpenXwin},
  {"data",       "null",    ".dat",  GraphxyDisplayToData, NULL},
#ifndef X_DISPLAY_MISSING /* X not available */
  {"xterm",      "xterm",   "null",  GraphxyDisplayXwin, GraphOpenXwin},
#endif
#ifdef PLD_png            /* if png/gd/zlib libraries available for png driver */
  {"png",        "png",     ".png",  GraphxyDisplayToFile, GraphOpenFile},
#endif
  {NULL, NULL}
};


#ifndef NO_PLOT

static int currentfgcolour  = BLACK;     /* Use ajGraphSetFore to change */
static int currentbgwhite = 1;      /* By default use a white background */
static float currentcharsize  = 1.0; /* Use ajGraphSetCharSize to change */
static int currentlinestyle = 1;     /* Use ajGraphSetLineStyle to change*/
static int currentfillpattern = 1;     /* Use ajGraphSetLineStyle to change*/

/***************************************************************************
plplot calls should only be made once from now on so all calls to plplot
should be made via ajGraph calls. So why bother? This is to make it easier
to change to another graph package as all the plplot calls should only be
listed once hence only a few routines should need to be changed?? Well in
theory anyway.
     Gone a bit further than I first envisaged but all the plplot commands
are now first and none should exist outside this initial bit.
***************************************************************************/
/*****************************************
PLPLOT CALLS *START*
******************************************/


/* @funcstatic GraphDefCharSize ***********************************************
**
** Set the default char size in mm.
**
** @param [r] size [float] Character size in mm. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphDefCharSize(float size){
  plschr((PLFLT)size, 1.0);
}

/* @funcstatic GraphTextLength *************************************************
**
** Compute the length of a string in user coordinates.
**
** @param [r] x1 [float] Start of text box on x axis
** @param [r] y1 [float] Start of text box on y axis
** @param [r] x2 [float] End of text box on x axis
** @param [r] y2 [float] End of text box on y axis
** @param [r] text [char*] Text
**
** @return [float] The length of the string in user coordinates
** @@
******************************************************************************/

static float GraphTextLength(float x1, float y1, float x2, float y2, char *text){
  return plstrlW(x1, y1, x2-x1, y2-y1, text);
}

/* @funcstatic GraphTextHeight **************************************************
**
** Compute the height of a character in user coordinates.
**
** @param [r] x1 [float] Start of text box on x axis
** @param [r] x2 [float] End of text box on x axis
** @param [r] y1 [float] Start of text box on y axis
** @param [r] y2 [float] End of text box on y axis
**
** @return [float] The height of the character in user coordinates
** @@
******************************************************************************/

static float GraphTextHeight(float x1, float x2, float y1, float y2){
  return plgchrW(x1, y1, x2-x1, y2-y1);
}

/* @func ajGraphSetDevice *****************************************************
**
** Set graph device
**
** @param [r] thys [AjPGraph] Graph object with displaytype set
**
** @return [void]
** @@
******************************************************************************/

void ajGraphSetDevice(AjPGraph thys){
  ajDebug ("=g= plsdev ('%s') [graphType[%d].Device] ready: %B\n",
	   graphType[thys->displaytype].Device,
	   thys->displaytype, thys->ready);
  if (!thys->ready)
    plsdev(graphType[thys->displaytype].Device);
}

/* @func ajGraphSetName ***************************************************
**
** set BaseName and extension.
**
** @param [r] thys [AjPGraph] Graph object.
**
** @return [void]
** @@
******************************************************************************/
void ajGraphSetName(AjPGraph thys){

  if (!ajStrMatchCaseCC(graphType[thys->displaytype].ext, "null"))
    GraphSetName(thys, thys->outputfile,graphType[thys->displaytype].ext);

  return;
}

/* @funcstatic GraphSetName ***************************************************
**
** set BaseName and extension.
**
** @param [r] thys [AjPGraph] Graph object.
** @param [r] txt [AjPStr] base name for files
** @param [r] ext [char*] extension for files
**
** @return [void]
** @@
******************************************************************************/
static void GraphSetName(AjPGraph thys, AjPStr txt,char *ext){

  if (!thys->ready) {
    ajDebug ("=g= plxsfnam ('%S', '%s')\n", txt, ext);
    plxsfnam (ajStrStr(txt), ext);
  }
  return;
}

/* @funcstatic GraphInit ******************************************************
**
** calls plinit.
**
** @param [r] thys [AjPGraph] Graph object.
** @return [void]
** @@
******************************************************************************/

static void GraphInit(AjPGraph thys){
  float fold;

  if (!thys->ready) {
    ajDebug ("=g= plinit ()\n");
    plinit();
  }
  thys->ready = ajTrue;

  fold = ajGraphSetCharSize(0);          /* set the char size */
  fold = ajGraphSetCharSize(fold);
}  

/* @funcstatic GraphSubPage ***************************************************
**
** start new graph page.
**
** @param [r] page [int] Page number
**
** @return [void]
** @@
******************************************************************************/

static void GraphSubPage(int page){
  ajDebug ("=g= pladv (%d) [page]\n", page);
  pladv(page);
}

/* @funcstatic GraphSetNumSubPage *********************************************
**
** Sets the number of sub pages for a page.
**
** @param [r] numofsubpages [int] Number of subpages
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetNumSubPage(int numofsubpages){
  ajDebug ("=g= plssub (1, %d) [numofsubpages]\n", numofsubpages);
  plssub(1,numofsubpages);
}

/* @funcstatic GraphWind ******************************************************
**
** Calls plwind.
**
** @param [r] xmin [float] Minimum x axis value
** @param [r] xmax [float] Maximum x axis value
** @param [r] ymin [float] Minimum y axis value
** @param [r] ymax [float] Maximum y axis value
**
** @return [void]
** @@
******************************************************************************/

static void GraphWind(float xmin, float xmax, float ymin, float ymax){
  ajDebug ("=g= plwind (%.2f, %.2f, %.2f, %.2f) [xmin/max ymin/max]\n",
	   xmin, xmax, ymin, ymax);
  plwind(xmin, xmax, ymin, ymax);
}

/* @funcstatic GraphSetWin *************************************************
**
** Creates a window using the ranges.
**
** @param [r] xmin [float] Minimum x axis value
** @param [r] xmax [float] Maximum x axis value
** @param [r] ymin [float] Minimum y axis value
** @param [r] ymax [float] Maximum y axis value
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetWin(float xmin, float xmax, float ymin, float ymax){
  ajDebug("=g= plvpor (0.0,1.0,0.0,1.0) [whole screen]\n");
  plvpor(0.0,1.0,0.0,1.0);         /* use the whole screen */
  GraphWind(xmin, xmax, ymin, ymax);  /* User must add boundarys if */
				      /* they want them*/
                                      /* by modifying xmin xmax etc */
}

/* @funcstatic GraphSetWin2 ************************************************
**
** Creates a window using the ranges.
**
** @param [r] xmin [float] Minimum x axis value
** @param [r] xmax [float] Maximum x axis value
** @param [r] ymin [float] Minimum y axis value
** @param [r] ymax [float] Maximum y axis value
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetWin2(float xmin, float xmax, float ymin, float ymax){
  ajDebug("=g= plvsta ()\n");
  plvsta();         /* use the whole screen */
  GraphWind(xmin, xmax, ymin, ymax);  /* User must add boundarys if they */
				      /* want them*/
                                      /* by modifying xmin xmax etc */
}

/* @funcstatic GraphArray*********************************************
**
** Draw lines from the array of x and y values.
**
** @param [r] numofpoints [int] Number of data points
** @param [r] x [float*] Array of x axis values
** @param [r] y [float*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphArray(int numofpoints, float *x, float *y){
  ajDebug("=g= plline ( %d, %.2f .. %.2f, %.2f .. %.2f) [num x..x y..y]\n",
	  numofpoints, x[0], x[numofpoints-1], y[0], y[numofpoints-1] );
  plline(numofpoints, x,y);
} 

/* @funcstatic GraphArrayGaps *****************************************
**
** Draw lines for an array of floats with gaps.
** Gaps are declared by having values of FLT_MIN.
**
** @param [r] numofpoints [int] Number of data points
** @param [r] x [float*] Array of x axis values
** @param [r] y [float*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphArrayGaps(int numofpoints, float *x, float *y){
  int i;
  float *x1,*x2;
  float *y1,*y2;

  x1 = x2 = x;
  y1 = y2 = y;
  x2++;
  y2++;

  for(i=1;i<numofpoints;i++){
    if(*x2 != FLT_MIN && *x1 != FLT_MIN && *y2 != FLT_MIN && *y1 != FLT_MIN) {
      ajDebug("=g= pljoin (%.2f, %.2f, %.2f, %.2f) [ xy xy]\n",
	      *x1, *y1, *x2, *y2);
      pljoin(*x1,*y1,*x2,*y2);
    }
    x1++; y1++;
    x2++; y2++;
  }
} 

/* @funcstatic GraphArrayGapsI *****************************************
**
** Draw lines for an array of integers with gaps.
** Gaps are declared by having values of INT_MIN.
**
** @param [r] numofpoints [int] Number of data points
** @param [r] x [int*] Array of x axis values
** @param [r] y [int*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphArrayGapsI(int numofpoints, int *x, int *y){
  int i;
  int *x1,*x2;
  int *y1,*y2;

  x1 = x2 = x;
  y1 = y2 = y;
  x2++;
  y2++;

  for(i=0;i<numofpoints-1;i++){
    if(*x2 != INT_MIN && *x1 != INT_MIN && *y2 != INT_MIN && *y1 != INT_MIN) {
      ajDebug("=g= pljoin (%.2f, %.2f, %.2f, %.2f) [ xy xy] [int xy xy]\n",
	      (float)*x1, (float)*x2, (float)*y1, (float)*y2);
      pljoin((float)*x1,(float)*y1,(float)*x2,(float)*y2);
    }
    x1++; y1++;
    x2++; y2++;
  }
} 

/* @funcstatic GraphFill ******************************************************
**
** Polygon fill a set of points.
**
** @param [r] numofpoints [int] Number of data points
** @param [r] x [float*] Array of x axis values
** @param [r] y [float*] Array of y axis values
**
** @return [void]
** @@
******************************************************************************/

static void GraphFill(int numofpoints, float *x, float *y){

  ajDebug("=g= plfill ( %d, %.2f .. %.2f, %.2f .. %.2f) [num x..x y..y]\n",
	  numofpoints, x[0], x[numofpoints-1], y[0], y[numofpoints-1] );
  plfill(numofpoints, x, y);
}

/* @funcstatic GraphPen *******************************************
**
** Change the actual colour of a pen.
**
** @param [r] pen [int] Pen colour number
** @param [r] red [int] Red value (see PLPLOT)
** @param [r] green [int] Green value (see PLPLOT)
** @param [r] blue [int] Blue value (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

static void GraphPen(int pen, int red, int green, int blue){
  ajDebug ("=g= plscol0 (%d, %d, %d, %d) [pen RGB]\n",
	   pen, red, green, blue);
  plscol0(pen,red,green,blue);
}

/* @funcstatic GraphSymbols ***********************************************
**
** Draw a symbol from the font list.
**
** @param [r] x1 [float*] Array of x axis values
** @param [r] y1 [float*] Array of y axis values
** @param [r] numofdots [int] Number of data points
** @param [r] symbol [int] Symbol number (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

static void GraphSymbols(float *x1, float *y1, int numofdots,int symbol){
  ajDebug ("=g= plpoin (%d, %.2f .. %.2f, %.2f .. %.2f, %d) [size, x..x y..y sym ]\n",
	   numofdots,x1[0], x1[numofdots-1], y1[0], y1[numofdots-1], symbol);
  plpoin(numofdots,x1,y1, symbol);
}

/* @funcstatic GraphClose *****************************************************
**
** Close the graph with the plplot command.
**
** @return [void]
** @@
******************************************************************************/

static void GraphClose(void){
  ajDebug ("=g= plend ()\n");
  plend();
}  

/* @funcstatic GraphText ******************************************************
**
** Display text.
**
** @param [r] x1 [float] Start of text box on x axis
** @param [r] y1 [float] Start of text box on y axis
** @param [r] x2 [float] End of text box on x axis
** @param [r] y2 [float] End of text box on y axis
** @param [r] just [float] Justification (see PLPLOT)
** @param [r] text [char*] Text
**
** @return [void]
** @@
******************************************************************************/

static void GraphText(float x1, float y1, float x2, float y2,
		      float just,char *text){
  ajDebug ("=g= plptex (%.2f, %.2f, %.2f, %.2f, %.2f, '%s') [xy xy just text]\n",
	   x1, y1, x2, y2, just, text);
  plptex(x1,y1,x2,y2,just, text);
}

/* @funcstatic GraphLineStyle *************************************************
**
** Set the Line style. i.e. dots dashes unbroken. 
**
** @param [r] style [int] Line style. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphLineStyle(int style){
  ajDebug ("=g= pllsty (%d) [line style]\n", style); 
  pllsty((PLINT)style);
}

/* @funcstatic GraphFillPat ***************************************************
**
** Set the pattern to fill with.
**
** @param [r] pat [int] Pattern code. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphFillPat(int pat){
  ajDebug ("=g= plpsty (%d) [pattern style]\n", pat); 
  plpsty((PLINT)pat);
}


/* @funcstatic GraphCharSize **************************************************
**
** Set the char size.
**
** @param [r] size [float] Character size. See PLPLOT.
**
** @return [void]
** @@
******************************************************************************/

static void GraphCharSize(float size){
  ajDebug ("=g= plschr (0.0, %.2f) [0.0 charsize]\n", size); 
  plschr(0.0,(PLFLT)size);
}

/* @func ajGraphLabel ***********************************************
**
** Label current Plot.
**
** @param [r] x [char*]        text for x axis labelling.
** @param [r] y [char*]        text for y axis labelling.
** @param [r] title [char*]    text for title of plot.
** @param [r] subtitle [char*] text for subtitle of plot.
** @return [void]
** @@
*************************************************************************/

void ajGraphLabel(char *x, char *y, char *title, char *subtitle){
  float fold;

  ajDebug ("=g= plmtex ('t', 2.5, 0.5, 0.5, '%s') [title]\n", title); 
  plmtex("t", (PLFLT) 2.5, (PLFLT) 0.5, (PLFLT) 0.5, title);
  ajDebug ("=g= plmtex ('b', 3.2, 0.5, 0.5, '%s') [x-title]\n", x); 
  plmtex("b", (PLFLT) 3.2, (PLFLT) 0.5, (PLFLT) 0.5, x);
  ajDebug ("=g= plmtex ('l', 5.0, 0.5, 0.5, '%s') [y-title]\n", y); 
  plmtex("l", (PLFLT) 5.0, (PLFLT) 0.5, (PLFLT) 0.5, y);
  
  fold = ajGraphSetCharSize(0);
  (void) ajGraphSetCharSize(fold/(float)2.0);
  ajDebug ("=g= plmtex ('t', 1.5, 0.5, 0.5, '%s') [subtitle]\n", subtitle); 
  plmtex("t", (PLFLT) 1.5, (PLFLT) 0.5, (PLFLT) 0.5, subtitle);
  fold = ajGraphSetCharSize(fold);

    /*  pllab(x,y,title);*/
}

/* @funcstatic GraphSetPen ****************************************************
**
** Set the pen to the colour specified.
** 
** @param [r] colour [int] Pen colour (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

static void GraphSetPen(int colour){
  ajDebug("=g= plcol (%d) [colour]\n", colour);
  plcol((PLINT)colour);
}

/* @func ajGraphSetPenWidth **********************************************
**
** Set the current pen width.
**
** @param [r] width [float] width for the pen.
** @return [void] 
** @@
******************************************************************************/
void ajGraphSetPenWidth(float width){

  ajDebug("=g= plwid (%.2f) [width]\n", width);
  plwid(width);
}

/*****************************************
PLPLOT CALLS *END*
******************************************/

/* @func ajGraphOpenPlot ***********************************************
**
** Open a window.
**
** @param [r] thys [AjPGraph] Graph object.
** @param [r] numofsets [int] number of plots in set.
**
** @return [void]
** @@
**************************************************************************/

void ajGraphOpenPlot(AjPGraph thys, int numofsets) {

  ajGraphSetDevice(thys);
  GraphSetNumSubPage(numofsets);
  ajGraphColourBack();
  GraphInit(thys);
  ajGraphColourFore();
}


/* @func ajGraphOpenWin ***********************************************
**
** Open a window whose view is defined by x and y's min and max.
**
** @param [r] thys [AjPGraph] Graph object
** @param [r] xmin [float] minimum x value.(user coordinates)
** @param [r] xmax [float] maximum x value.
** @param [r] ymin [float] minimum y value.
** @param [r] ymax [float] maximum y value.
**
** @return [void]
** @@
**************************************************************************/

void ajGraphOpenWin (AjPGraph thys, float xmin, float xmax,
		  float ymin, float ymax)
{
  AJTIME ajtime;
  const time_t tim = time(0);      

  ajtime.time = localtime(&tim);
  ajtime.format = 0;

  ajGraphSetDevice(thys);
  graphType[thys->displaytype].GOpen(thys, graphType[thys->displaytype].ext);


  if( ajStrLen(thys->title) <=1)
    (void) ajFmtPrintAppS(&thys->title,"%s (%D)",ajAcdProgram(),&ajtime);

  ajGraphColourBack();
  GraphInit(thys);
  ajGraphColourFore();
  GraphSubPage(0);
  GraphSetWin(xmin, xmax, ymin, ymax);

}


/* @func ajGraphNewPage ***********************************************
**
** Clear Screen if (X) or new page if plotter/postscript. Also pass a boolean
** to state wether you want the current oen colour character sizes etc to
** be reset or stay the same for the next page.
**
** @param [r] resetdefaults [AjBool] reset page setting?
** @return [void]
** @@
**************************************************************************/
void ajGraphNewPage(AjBool resetdefaults){
  int old;
  float fold;

  GraphSubPage(0);
  if(resetdefaults){
    (void) ajGraphSetFore(BLACK);
    (void) ajGraphSetCharSize(1.0);
    (void) ajGraphSetLineStyle(0);
  }
  else{
    old = ajGraphSetFore(BLACK); /* pladv resets every thing so need */
				       /* to get the old copy */
    (void) ajGraphSetFore(old);         /* then set in again */
    fold = ajGraphSetCharSize(0);
    (void) ajGraphSetCharSize(fold);
    old = ajGraphSetLineStyle(0);
    (void) ajGraphSetLineStyle(old);
  } 
}


/* @func ajGraphCloseWin ***********************************************
**
** Close current window.
**
** @return [void]
** @@
**************************************************************************/
void ajGraphCloseWin(void){
  GraphClose();
}

/* @func ajGraphOpen ***********************************************
**
** Open a window whose view is defined by the x's and y's min and max
** values.
**
** @param [r] thys [AjPGraph] Graph object
** @param [r] xmin [PLFLT] minimum x value.(user coordinates)
** @param [r] xmax [PLFLT] maximum x value.
** @param [r] ymin [PLFLT] minimum y value.
** @param [r] ymax [PLFLT] maximum y value.
** @param [r] flags [int] flag bit settings
**
** @return [void]
** @@
**************************************************************************/

void ajGraphOpen (AjPGraph thys, PLFLT xmin, PLFLT xmax,
		  PLFLT ymin, PLFLT ymax, int flags) {
  AJTIME ajtime;
  const time_t tim = time(0);      

  ajtime.time = localtime(&tim);
  ajtime.format = 0;

  ajGraphSetDevice(thys);
  graphType[thys->displaytype].GOpen(thys, graphType[thys->displaytype].ext);

  if( ajStrLen(thys->title) <=1){
    (void) ajStrAppC(&thys->title,
		     ajFmtString("%s (%D)",ajAcdProgram(),&ajtime));
  }

  GraphSetName(thys, thys->outputfile,graphType[thys->displaytype].ext);
  ajGraphColourBack();
  GraphInit(thys);
  ajGraphColourFore();
  ajGraphPlenv(xmin, xmax , ymin, ymax , flags);
}

/* @func ajGraphLabelYRight *************************************
**
** Label the right hand y axis.
**
** @param [r] text [char*] text for label of right y axis.
** @return [void]
** @@
*************************************************************************/
void ajGraphLabelYRight(char *text){
  ajDebug ("=g= plmtex ('r', 2.0, 0.5, 0.5, '%s') [LabelYRight]\n", text); 
  plmtex("r",2.0,0.5,0.5,text);
}


/* @func ajGraphClose ***********************************************
**
** Close current Plot.
**
** @return [void]
** @@
**************************************************************************/
void ajGraphClose(void){
  GraphClose();
}


/* @func ajGraphSet *******************************************************
**
** Initialize options for a graph object
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] type [AjPStr] Graph type
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajGraphSet (AjPGraph thys, AjPStr type) {

  int i;

  for (i=0;graphType[i].Name;i++) {
    if (ajStrPrefixCaseCO(graphType[i].Name, type)) {
      if (!graphType[i].GOpen) {
	ajDebug("ajGraphSet type '%S' displaytype %d '%s' no GOpen function\n",
	      type, i, graphType[i].Name);
	return ajFalse;
      }
      thys->displaytype = i;
      ajDebug("ajGraphSet type '%S' displaytype %d '%s'\n",
	      type, i, graphType[i].Name);
      return ajTrue;
    }
  }

  thys->displaytype = i;
  ajDebug("ajGraphSet type '%S' displaytype not found, set to %d '%s'\n",
	  type, i, graphType[i].Name);
  return ajFalse;
}

/* @func ajGraphxySet *******************************************************
**
** Initialize options for a graphxy object
**
** @param [u] thys [AjPGraph] Graph object
** @param [r] type [AjPStr] Graph type
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajGraphxySet (AjPGraph thys, AjPStr type) {

  int i;

  for (i=0;graphType[i].Name;i++) {
    if (ajStrPrefixCaseCO(graphType[i].Name, type)) {
      if (!graphType[i].XYDisplay) {
	ajDebug("ajGraphxySet type '%S' displaytype %d '%s' no GOpen function\n",
		type, i, graphType[i].Name);
	return ajFalse;
      }
      thys->displaytype = i;
      ajDebug("ajGraphxySet type '%S' displaytype %d '%s'\n",
	      type, i, graphType[i].Name);
      return ajTrue;
    }
  }

  thys->displaytype = i;
  ajDebug("ajGraphxySet type '%S' displaytype not found, set to %d '%s'\n",
	  type, i, graphType[i].Name);
  return ajFalse;
}

/* @func ajGraphDumpDevices ***************************************************
**
** Dump device options for a graph object
**
** @return [void] 
** @@
******************************************************************************/

void ajGraphDumpDevices (void) {

  int i;

  ajUser("Devices allowed are:-");
  for (i=0;graphType[i].Name;i++) {
    ajUser("%s",graphType[i].Name);
  }
  
  
  return;
}

/* @funcstatic GraphListDevicesarg ********************************************
**
** Store device names for a graph object in a list
**
** @param [r] name [char*] Function name (always needed for these callbacks)
** @param [u] args [va_list] Arguments, in actual user this must
**                           be an AjPList object
** @return [void] 
** @@
******************************************************************************/

static void GraphListDevicesarg (char* name, va_list args) {

  AjPList list;
  int i;
  AjPStr devname;

  list = va_arg(args, AjPList);

  for (i=0;graphType[i].Name;i++) {
    devname = ajStrNewC(graphType[i].Name);
    ajListstrPushApp (list, devname);
  }

  return;
}


/* @func ajGraphTraceInt ****************************************************
**
** Writes debug messages to trace the device driver internals graph object.
**
** @param [r] thys [AjPGraph] Graph object
** @param [r] outf [FILE*] Output file
** @return [void]
** @@
*****************************************************************************/

void ajGraphTraceInt (AjPGraph thys, FILE* outf) {
  plxtrace (outf);
}

/* @func ajGraphTrace *******************************************************
**
** Writes debug messages to trace the contents of a graph object.
**
** @param [r] thys [AjPGraph] Graph object
** @return [void]
** @@
*****************************************************************************/

void ajGraphTrace (AjPGraph thys) {

  ajDebug("Graph trace\n");
  ajDebug("\n(a) True booleans\n");

  if (thys->minmaxcalc) ajDebug ("minmaxcalc %B\n", thys->minmaxcalc);


  ajDebug("\n(b) Strings with values\n");

  ajDebug ("Title '%S'\n", thys->title);
  ajDebug ("Subtitle '%S'\n", thys->subtitle);
  ajDebug ("Xaxis '%S'\n", thys->xaxis);
  ajDebug ("Yaxis '%S'\n", thys->yaxis);
  ajDebug ("outputfile '%S'\n", thys->outputfile);

  ajDebug("\n(c) Other values\n");
  ajDebug ("flags %x\n", thys->flags);
  ajDebug ("numofgraphs %d\n", thys->numofgraphs);
  ajDebug ("numofgraphsmax %d\n", thys->numofgraphsmax);
  ajDebug("minmaxcalc %B\n", thys->minmaxcalc);
  ajDebug("minX   %7.3f maxX   %7.3f\n", thys->minX, thys->maxX);
  ajDebug("minY   %7.3f maxY   %7.3f\n", thys->minY, thys->maxY);
  ajDebug("xstart %7.3f xend   %7.3f\n", thys->xstart, thys->xend);
  ajDebug("ystart %7.3f yend   %7.3f\n", thys->ystart, thys->yend);
  ajDebug("file %F\n", thys->file);
  ajDebug("displaytype %d '%s'\n", thys->displaytype,
	  graphType[thys->displaytype].Device);

  return;
}


/* @func ajGraphCircle **********************************************
**
** Draw a circle.
**
** @param  [r] xcentre [PLFLT] x coor for centre.
** @param  [r] ycentre [PLFLT] y coor for centre.
** @param  [r] radius  [float] radius of the circle.
** @return [void]
** @@
**
** NOTE: Due to x and y not the same length this produces an oval!!
**       This will have to do for now. But i am aware that the code
**       is slow and not quite right.
*********************************************************************/
void ajGraphCircle (PLFLT xcentre, PLFLT ycentre, float radius){
  PLFLT x[361],y[361];
  int i;
 
  for(i=0;i<360;i++){
    x[i] = xcentre + (radius * (float)cos(ajDegToRad(i)));
    y[i] = ycentre + (radius * (float)sin(ajDegToRad(i)));
  }
  x[360] = x[0];
  y[360] = y[0];

  GraphArray(361, x,y);
}

/* @func ajGraphPolyFill **************************************************
**
** Draw a polygon and fill it in.
**
** @param [r] n [int] number of points
** @param [r] x [PLFLT *] x coors of points
** @param [r] y [PLFLT *] y coors of points
** @return [void]
** @@
******************************************************************************/
void ajGraphPolyFill(int n, PLFLT *x, PLFLT *y){
  GraphFill(n, x, y);
}

/* @func ajGraphPoly **************************************************
**
** Draw a polygon.
**
** @param [r] n [int] number of points
** @param [r] x [PLFLT *] x coors of points
** @param [r] y [PLFLT *] y coors of points
** @return [void]
** @@
******************************************************************************/
void ajGraphPoly (int n, PLFLT *x, PLFLT *y){
  GraphArray(n, x, y);
}

/* @func ajGraphTriFill **************************************************
**
** Draw a Triangle and fill it in.
**
** @param [r] x1 [PLFLT] x1 coord of point 1.
** @param [r] y1 [PLFLT] y1 coord of point 1.
** @param [r] x2 [PLFLT] x2 coord of point 2.
** @param [r] y2 [PLFLT] y2 coord of point 2.
** @param [r] x3 [PLFLT] x3 coord of point 3.
** @param [r] y3 [PLFLT] y3 coord of point 3.
** @return [void]
** @@
******************************************************************************/

void ajGraphTriFill(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2,
			 PLFLT x3, PLFLT y3){

  PLFLT x[3], y[3];

  x[0]=x1; x[1]=x2; x[2]=x3;
  y[0]=y1; y[1]=y2; y[2]=y3;

  ajGraphPolyFill(3, x, y);
}

/* @func ajGraphTri **************************************************
**
** Draw a Triangle.
**
** @param [r] x1 [PLFLT] x1 coord of point 1.
** @param [r] y1 [PLFLT] y1 coord of point 1.
** @param [r] x2 [PLFLT] x2 coord of point 2.
** @param [r] y2 [PLFLT] y2 coord of point 2.
** @param [r] x3 [PLFLT] x3 coord of point 3.
** @param [r] y3 [PLFLT] y3 coord of point 3.
** @return [void]
** @@
******************************************************************************/

void ajGraphTri (PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2,
		 PLFLT x3, PLFLT y3){

  PLFLT x[4], y[4];

  x[0]=x1; x[1]=x2; x[2]=x3; x[3]=x1;
  y[0]=y1; y[1]=y2; y[2]=y3; y[3]=y1;

  ajGraphPoly(4, x, y);
}

/* @func ajGraphDiaFill **************************************************
**
** Draw a diamond to the plotter device at point x0,y0 size big and
** fill it in.
**
** @param [r] x0 [PLFLT] x position to draw the diamond.
** @param [r] y0 [PLFLT] y position to draw the diamond.
** @param [r] size [PLFLT]  how big to draw the diamond.
** @return [void]
** @@
******************************************************************************/

void ajGraphDiaFill (PLFLT x0, PLFLT y0, PLFLT size){

  PLFLT x[4], y[4];
  PLFLT incr = size*(float)0.5;

  x[0] = x0;
  y[0] = y0 + incr;
  x[1] = x0 + incr;
  y[1] = y0 + size;
  x[2] = x0 + size;
  y[2] = y0 + incr;
  x[3] = x0 + incr;
  y[3] = y0;

  GraphFill(4, x, y);

  return;
}

/* @func ajGraphDia *****************************************************
**
** Draw a diamond to the plotter device at point x0,y0 size big.
**
** @param [r] x0 [PLFLT] x position to draw the diamond.
** @param [r] y0 [PLFLT] y position to draw the diamond.
** @param [r] size [PLFLT]  how big to draw the diamond.
** @return [void]
** @@
******************************************************************************/

void ajGraphDia (PLFLT x0, PLFLT y0, PLFLT size){

  PLFLT x[5], y[5];
  PLFLT incr = size*(float)0.5;

  x[0] = x0;
  y[0] = y0 + incr;
  x[1] = x0 + incr;
  y[1] = y0 + size;
  x[2] = x0 + size;
  y[2] = y0 + incr;
  x[3] = x0 + incr;
  y[3] = y0;
  x[4] = x[0];
  y[4] = y[0];
  GraphArray(5, x, y);

  return;
}

/* @func ajGraphBoxFill *****************************************************
**
** Draw a box to the plotter device at point x0,y0 size big and
** fill it in.
**
** @param [r] x0 [PLFLT] x position to draw the box.
** @param [r] y0 [PLFLT] y position to draw the box.
** @param [r] size [PLFLT]  how big to draw the box.
** @return [void]
** @@
******************************************************************************/

void ajGraphBoxFill (PLFLT x0, PLFLT y0, PLFLT size) {

  PLFLT x[4], y[4];

  x[0] = x0;
  y[0] = y0;
  x[1] = x0;
  y[1] = y0 + size;
  x[2] = x0 + size;
  y[2] = y0 + size;
  x[3] = x0 + size;
  y[3] = y0;
  GraphFill(4, x, y);

  return;
}

/* @func ajGraphBox *****************************************************
**
** Draw a box to the plotter device at point x0,y0 size big.
**
** @param [r] x0 [PLFLT] x position to draw the box.
** @param [r] y0 [PLFLT] y position to draw the box.
** @param [r] size [PLFLT]  how big to draw the box.
** @return [void]
** @@
******************************************************************************/

void ajGraphBox (PLFLT x0, PLFLT y0,PLFLT size) {

  PLFLT x[5], y[5];

  x[0] = x0;
  y[0] = y0;
  x[1] = x0;
  y[1] = y0 + size;
  x[2] = x0 + size;
  y[2] = y0 + size;
  x[3] = x0 + size;
  y[3] = y0;
  x[4] = x[0];
  y[4] = y[0];
  GraphArray(5, x, y);

  return;
}
/* @func ajGraphRectFill ***********************************************
**
** Draw a rectangle and fill it with the current pen colour/style.
**
** @param [r] x0 [PLFLT] x0 coor.
** @param [r] y0 [PLFLT] y0 coor.
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphRectFill (PLFLT x0, PLFLT y0, PLFLT x1, PLFLT y1) {

  PLFLT x[4], y[4];

  x[0] = x0;
  y[0] = y0;
  x[1] = x0;
  y[1] = y1;
  x[2] = x1;
  y[2] = y1;
  x[3] = x1;
  y[3] = y0;
  GraphFill(4, x, y);

  return;
}

/* @func ajGraphRect ***********************************************
**
** Draw a rectangle with the current pen colour/style.
**
** @param [r] x0 [PLFLT] x0 coor.
** @param [r] y0 [PLFLT] y0 coor.
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphRect (PLFLT x0, PLFLT y0,PLFLT x1, PLFLT y1) {

  PLFLT x[5], y[5];

  x[0] = x0;
  y[0] = y0;
  x[1] = x0;
  y[1] = y1;
  x[2] = x1;
  y[2] = y1;
  x[3] = x1;
  y[3] = y0;
  x[4] = x[0];
  y[4] = y[0];
  GraphArray(5, x, y);

  return;
}

/* @func ajGraphSetBackWhite ********************************************
**
** Set the background colour to White.
**
** @return [void]
** @@
*************************************************************************/
void ajGraphSetBackWhite(void){

  ajDebug("ajGraphSetBackWhite currentbgwhite: %B\n", currentbgwhite);

  if(!currentbgwhite){    
    GraphPen(NCOLS-1, 0, 0, 0);
    GraphPen(0, 255, 255, 255);
    currentbgwhite = 1;
    ajDebug ("ajGraphSetBackWhite pen 0 WHITE pen %d BLACK\n", NCOLS-1);
  }
}

/* @func ajGraphSetBackBlack ********************************************
**
** Set the background colour to Black.
**
** @return [void]
** @@
*************************************************************************/
void ajGraphSetBackBlack(void){

  ajDebug("ajGraphSetBackBlack currentbgwhite: %B\n", currentbgwhite);

  if(currentbgwhite){    
    GraphPen(0, 0, 0, 0);
    GraphPen(NCOLS-1, 255, 255, 255);
    currentbgwhite = 0;
    ajDebug ("ajGraphSetBackBlack pen 0 BLACK pen %d WHITE\n", NCOLS-1);
  }
}

/* @func ajGraphColourBack ********************************************
**
** Set the background colour to either Black or white depending on the 
** current settings.
**
** @return [void]
** @@
*************************************************************************/
void ajGraphColourBack(void){
  
  ajDebug("ajGraphColourBack currentbgwhite: %B\n", currentbgwhite);

  if(currentbgwhite){    
    GraphPen(NCOLS-1, 0, 0, 0);
    GraphPen(0, 255, 255, 255);
    ajDebug ("ajGraphColourBack pens 0 WHITE and pen %d BLACK\n", NCOLS-1);
  }

}

/* @func ajGraphColourFore **********************************************
**
** Set the foreground plotting colour using current stored colour.
**
** @return [void]
** @@
******************************************************************************/
void ajGraphColourFore(void){
  int colour = currentfgcolour;

  if(currentbgwhite){          /* OKAY!! when we swap backgrounds we also */
    if(colour == 0)       /* swap pens. User does not know this so switch */
      colour = NCOLS-1;             /* for them */
    else if(colour == NCOLS-1)
      colour = 0;
  }
  GraphSetPen(colour);
  ajDebug ("ajGraphColourFore currentbgwhite: %B currentfgcolour: %d => %d\n",
	   currentbgwhite, currentfgcolour, colour);
}

/* @func ajGraphSetFore **********************************************
**
** Set the foreground plotting colour
**
** @param [r] colour [int]  colour to set drawing to.
** @return [int] the previous colour. 
** @@
******************************************************************************/
int ajGraphSetFore (int colour) {
  int oldcolour;
  int col = colour;

  if(col > NCOLS-1)       /* in case of looping through colours */
    while(col > NCOLS-1)  /* start at the begining once past end */
      col -=NCOLS;
  if(col < 0)
    ajDebug ("ajGraphSetFore give up and use currentfgcolour %d\n",
	     currentfgcolour);
  if(col < 0)
    return currentfgcolour;
  
  oldcolour = currentfgcolour;
  currentfgcolour = col;

  if(currentbgwhite){          /* OKAY!! when we swap backgrounds we also */
    if(col == 0)            /* swap pens. User does not know this so switch */
      col = NCOLS-1;             /* for them */
    else if(col == NCOLS-1)
      col = 0;
  }

  ajDebug ("ajGraphSetFore (%d) currentbgwhite: %B, currentfgcolour: %d\n",
	   colour, currentbgwhite, currentfgcolour);

  ajGraphColourFore();
  
  return oldcolour;
}

/* @func ajGraphCheckColour **********************************************
**
** Find if the colour is on the list
**
** @param [r] colour [AjPStr]  colour to set drawing to.
** @return [int] the colour number if found else -1. 
** @@
******************************************************************************/
int ajGraphCheckColour(AjPStr colour){
  int i;

  (void) ajStrToUpper(&colour);
  for(i=0;i<NCOLS;i++){
    if(!strcmp(ajStrStr(colour),colournum[i]))
      return i;
  }
  return -1;
}

/* @func ajGraphGetBaseColour *******************************************
**
** Initialize a base colours array for sequence characters
**
** @return [int*] Array of colours (see PLPLOT)
** @@
******************************************************************************/

int* ajGraphGetBaseColour(void){
  int *ret;

  ret = (int *) AJALLOC(sizeof(int)*AZ);

  ret[0] = BLACK; /* A */
  ret[1] = BLACK; /* B */
  ret[2] = YELLOW; /* C */
  ret[3] = RED; /* D */
  ret[4] = RED; /* E */
  ret[5] = WHEAT; /* F */
  ret[6] = GREY; /* G */
  ret[7] = BLUE; /* H */
  ret[8] = BLACK; /* I */
  ret[9] = BLACK; /* J */
  ret[10] = BLUE; /* K*/
  ret[11] = BLACK; /* L*/
  ret[12] = YELLOW; /* M*/
  ret[13] = GREEN; /* N*/
  ret[14] = BLACK; /* O*/
  ret[15] = BLUEVIOLET; /* P*/
  ret[16] = GREEN; /* Q*/
  ret[17] = BLUE; /* R*/
  ret[18] = CYAN; /* S*/
  ret[19] = CYAN; /* T*/
  ret[20] = BLACK; /* U*/
  ret[21] = BLACK; /* V*/
  ret[22] = WHEAT; /* W*/
  ret[23] = BLACK; /* X*/
  ret[24] = WHEAT; /* Y*/
  ret[25] = BLACK; /* Z*/
  ret[26] = BLACK; /* ? */
  ret[27] = BLACK; /* ?*/

  return ret;
}

/* @func ajGraphGetCharSize ***************************************************
**
** Get the char size.
**
** @param [rw] defheight [float *] where to store the default char height
** @param [rw] currentheight [float *] where to the current  char height
**
** @return [void]
** @@
******************************************************************************/
void ajGraphGetCharSize(float *defheight, float *currentheight){

  ajDebug ("=g= plgchr (&f &f) [&defht, &curht]\n"); 
  plgchr(defheight,currentheight);
}

/* @func ajGraphGetOut *****************************************
**
** Get the Output Device Parameters
**
** @param [rw] xp [float *] where to store the default char height
** @param [rw] yp [float *] where to the current  char height
** @param [rw] xleng [int *] where to store the default char height
** @param [rw] yleng [int *] where to the current  char height
** @param [rw] xoff [int *] where to store the default char height
** @param [rw] yoff [int *] where to the current  char height
**
** @return [void]
** @@
******************************************************************************/
void ajGraphGetOut(float *xp,float *yp,int *xleng,int *yleng,
		   int *xoff,int *yoff){

  ajDebug ("=g= plgpage (&f &f) [&xp, &yp, &xleng, &yleng, &xoff, &yoff]\n"); 
  plgpage(xp,yp,xleng,yleng,xoff,yoff);
}

/* @func ajGraphSetOri ***************************************************
**
** Set graph orientation
**
** @param [r] ori [int] orientation (landscape is zero, portrait is
**                      any other value).
**
** @return [void]
**@@
******************************************************************************/
void ajGraphSetOri(int ori){
  ajDebug ("=g= plsori (%d) [ori]\n", ori); 
  plsori(ori);
}

/* @func ajGraphPlenv ***************************************************
**
** Defines a plot environment. i.e. tells plplot wether the graph is boxed, 
** wether it has tick marks, wether it has labels etc. These should already
** be set in the flags.
**
** @param [r] xmin [float] X axis start
** @param [r] xmax [float] X axis end
** @param [r] ymin [float] Y axis start
** @param [r] ymax [float] Y axis end
** @param [r] flags [int] flag bit settings
** @return [void]
** @@
******************************************************************************/

void ajGraphPlenv (float xmin, float xmax, float ymin, float ymax,
		   int flags) {

  char xopt[10]=" ",yopt[10]=" ";
  int i=0;
  int j=0;


  ajDebug ("ajGraphPlenv (%.3f, %.3f, %.3f, %.3f, flags:%x)\n",
	   xmin, xmax, ymin, ymax, flags);
  GraphSubPage(0);
  /*  plvsta();*/
  /*  plwind(xmin,xmax,ymin,ymax);*/
  GraphSetWin2(xmin,xmax,ymin,ymax);

  if(flags & AJGRAPH_X_BOTTOM)
    xopt[i++] = 'b';

  if(flags & AJGRAPH_Y_LEFT)
    yopt[j++] = 'b';

  if(flags & AJGRAPH_X_TOP)
    xopt[i++] = 'c';

  if(flags & AJGRAPH_Y_RIGHT)
    yopt[j++] = 'c';

  if(flags & AJGRAPH_X_TICK){
    xopt[i++] = 't';  /* do ticks */
    xopt[i++] = 's';  /* do subticks */
    if(flags & AJGRAPH_X_INVERT_TICK)
      xopt[i++] = 'i';
    if(flags & AJGRAPH_X_NUMLABEL_ABOVE)
      xopt[i++] = 'm';
    else
      xopt[i++] = 'n';  /* write numeric labels */
  }

  if(flags & AJGRAPH_Y_TICK){
    yopt[j++] = 't';
    yopt[j++] = 's';
    if(flags & AJGRAPH_Y_INVERT_TICK)
      yopt[j++] = 'i';
    if(flags & AJGRAPH_Y_NUMLABEL_LEFT)
      yopt[j++] = 'm';
    else
      yopt[j++] = 'n';
  }

  if(flags & AJGRAPH_X_GRID)
    xopt[i++] = 'g';

  if(flags & AJGRAPH_Y_GRID)
    yopt[j++] = 'g';

  ajDebug ("=g= plbox ('%s', 0.0, 0, '%s', 0.0, 0) [xopt, 0.0, 0, yopt, 0.0, 0]\n", xopt, yopt); 
  plbox(xopt, 0.0, 0, yopt, 0.0, 0);

  return;
}

/* @funcstatic GraphCheckFlags ************************************************
**
** Prints the flags defined by bits in the input integer value.
**
** @param [r] flags [int] flag bits
** @return [void]
** @@
******************************************************************************/

static void GraphCheckFlags (int flags) {

  (void) printf("flag = %d\n",flags);
  if(flags & AJGRAPH_X_BOTTOM)
    (void) printf("AJGRAPH_X_BOTTOM \n");
  if(flags & AJGRAPH_Y_LEFT)
    (void) printf("AJGRAPH_Y_LEFT \n");
  if(flags & AJGRAPH_X_TOP)
    (void) printf("AJGRAPH_X_TOP \n");
  if(flags & AJGRAPH_Y_RIGHT)
    (void) printf(" AJGRAPH_Y_RIGHT\n");
  if(flags & AJGRAPH_X_TICK)
    (void) printf("AJGRAPH_X_TICK \n");
  if(flags & AJGRAPH_Y_TICK)
    (void) printf("AJGRAPH_Y_TICK \n");
  if(flags & AJGRAPH_X_LABEL)
    (void) printf("AJGRAPH_X_LABEL \n");
  if(flags & AJGRAPH_Y_LABEL)
    (void) printf("AJGRAPH_Y_LABEL \n");
  if(flags & AJGRAPH_TITLE)
   (void) printf("AJGRAPH_TITLE \n");
  if(flags & AJGRAPH_JOINPOINTS)
    (void) printf("AJGRAPH_JOINPOINTS \n");
  if(flags & AJGRAPH_OVERLAP)
    (void) printf("AJGRAPH_OVERLAP \n");
  if(flags & AJGRAPH_Y_NUMLABEL_LEFT)
    (void) printf("AJGRAPH_Y_NUMLABEL_LEFT");
  if(flags & AJGRAPH_Y_INVERT_TICK)
    (void) printf("AJGRAPH_Y_INVERT_TICK");
  if(flags & AJGRAPH_Y_GRID)
    (void) printf("AJGRAPH_Y_GRID");
  if(flags & AJGRAPH_X_NUMLABEL_ABOVE)
    (void) printf("AJGRAPH_X_NUMLABEL_ABOVE");
  if(flags & AJGRAPH_X_INVERT_TICK)
    (void) printf("AJGRAPH_X_INVERT_TICK");
  if(flags & AJGRAPH_X_GRID)
    (void) printf("AJGRAPH_X_GRID");

  return;
}

/* @func ajGraphGetColour **********************************************
**
** Return current foreground colour
**
** @return [int] colour.
** @@
******************************************************************************/

int ajGraphGetColour(void)
{
    return currentfgcolour;
}

/* @func ajGraphSetLineStyle **********************************************
**
** Set the line style.
**
** @param [r] style [int]  line style to set drawing to.
** @return [int] the previous line style. 
** @@
******************************************************************************/
int ajGraphSetLineStyle (int style) {
  int oldstyle;

  if(style < 1 || style > 8)
    style = 1;

  oldstyle = currentlinestyle;
  currentlinestyle = style;
  GraphLineStyle((PLINT)style);

  return oldstyle;
}

/* @func ajGraphSetFillPat **********************************************
**
** Set the Fill Pattern type.
**
** @param [r] style [int]  line style to set drawing to.
** @return [int] the previous line style. 
** @@
******************************************************************************/

int ajGraphSetFillPat (int style) {
  int oldstyle;

  if(style < 0 || style > 8)
    style = 1;

  oldstyle = currentfillpattern;
  currentfillpattern = style;
  GraphFillPat((PLINT)style);

  return oldstyle;
}

/* @func ajGraphSetCharSize**********************************************
**
** Set the character size.
**
** @param [r] size [float]  character size.
** @return [float] the previous character size. 
** @@
******************************************************************************/
float ajGraphSetCharSize (float size) {
  float oldsize;

  oldsize = currentcharsize;
  currentcharsize = size;
  GraphCharSize((PLFLT)size);

  return oldsize;
}


/* @funcstatic GraphCheckPoints *********************************************
**
** Prints a list of data points from two floating point arrays.
**
** @param [r] n [PLINT] Number of points to print
** @param [r] x [PLFLT*] X coordinates
** @param [r] y [PLFLT*] Y coordinates
** @return [void]
** @@
******************************************************************************/
static void GraphCheckPoints (PLINT n, PLFLT *x, PLFLT *y) {
  int i;

  for(i=0;i<n;i++)
    (void) printf("%d %f %f\n",i,x[i],y[i]);
}

/* @func ajGraphLine **********************************************
**
** Draw line between 2 points.
**
** @param [r] x1 [PLFLT]  x start position.
** @param [r] y1 [PLFLT]  y start position.
** @param [r] x2 [PLFLT]  x end position.
** @param [r] y2 [PLFLT]  y end position.
** @return [void]
** @@
******************************************************************************/

void ajGraphLine (PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2) {
  ajDebug ("=g= pljoin (%.2f, %.2f, %.2f, %.2f) [xy xy]\n",
	   x1, y1, x2, y2); 
  pljoin(x1,y1,x2,y2);
}

/* @funcstatic GraphDrawLines ******************************************
**
** Draw a number of lines. The points are stored in two array.
**
** @param [r] numoflines [int] number of line to plot.
** @param [r] x1         [PLFLT*] pointer to x coordinates.
** @param [r] y1         [PLFLT*] pointer to y coordinates.
** @return [void]
** @@
******************************************************************************/

static void GraphDrawLines( int numoflines,PLFLT *x1, PLFLT *y1){
  int i;
  PLFLT x0,y0;
  
  for(i=0;i<numoflines-1;i++){
    x0 = *x1; 
    y0 = *y1;
    x1++;
    y1++;
    ajGraphLine(x0,y0,*x1,*y1);
  }
}

/* @func ajGraphLines ***********************************************
**
** Draw a set of lines.
**
** @param [r] x1 [PLFLT*] x1 coors.
** @param [r] y1 [PLFLT*] y1 coors.
** @param [r] x2 [PLFLT*] x2 coors.
** @param [r] y2 [PLFLT*] y2 coors.
** @param [r] numoflines [int] The number of lines to be drawn.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphLines (PLFLT *x1, PLFLT *y1, PLFLT *x2, PLFLT *y2, int numoflines){
  int i=0;
      
  for(i=0;i<numoflines;i++){
    ajGraphLine(*x1,*y1,*x2,*y2);
    x1++; y1++; x2++; y2++;
  }
}

/* @func ajGraphDots ***********************************************
**
** Draw a set of dots.
**
** @param [r] x1 [PLFLT*] x1 coors.
** @param [r] y1 [PLFLT*] y1 coors.
** @param [r] numofdots [int] The number of dots to be drawn.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphDots(PLFLT *x1,PLFLT *y1, int numofdots){
  
    GraphSymbols(x1, y1, numofdots, 17);

  /* see x06c in examples for codes e.g. 17 is a dot*/
}

/* @func ajGraphSymbols ***********************************************
**
** Draw a set of dots.
**
** @param [r] numofdots [int] Number of coordinates in x1 and x2.
** @param [r] x1 [PLFLT*] x1 coors.
** @param [r] y1 [PLFLT*] y1 coors.
** @param [r] symbol [int] Symbol code.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphSymbols( int numofdots, PLFLT *x1,PLFLT *y1, int symbol){
  
    GraphSymbols(x1, y1, numofdots, symbol);

  /* see x06c in examples for codes e.g. 17 is a dot*/
}


/* @func ajGraphTextLine ***********************************************
**
** Draw text along a line.
**
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @param [r] x2 [PLFLT] x2 coor.
** @param [r] y2 [PLFLT] y2 coor.
** @param [r] text [char*] The text to be displayed.
** @param [r] just [PLFLT] justification of the string. (0=left,1=right,0.5=middle etc)
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphTextLine(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2,
		     char *text, PLFLT just){
  GraphText(x1,y1,x2-x1,y2-y1,just, text);
}


/* @func ajGraphText ************************************************
**
** Draw text, positioning with respect to (x1,y1) by just. 
**
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @param [r] text [char*] The text to be displayed.
** @param [r] just [PLFLT] justification of the string. (0=left,1=right,0.5=middle etc)
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphText(PLFLT x1, PLFLT y1, char *text, PLFLT just){
  GraphText(x1,y1,1.0,0.0,just, text);
}


/* @func ajGraphTextStart ************************************************
**
** Draw text starting at position (x1,y1)
**
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @param [r] text [char*] The text to be displayed.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphTextStart(PLFLT x1, PLFLT y1, char *text){
  GraphText(x1,y1,1.0,0.0,0.0, text);
}


/* @func ajGraphTextEnd ************************************************
**
** Draw text ending at position (x1,y1)
**
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @param [r] text [char*] The text to be displayed.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphTextEnd (PLFLT x1, PLFLT y1, char *text){
  GraphText(x1,y1,1.0,0.0,1.0, text);
}


/* @func ajGraphTextMid ************************************************
**
** Draw text with Mid point of text at (x1,y1).
**
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @param [r] text [char*] The text to be displayed.
** @return [void]
**
** @@ 
************************************************************************/
void ajGraphTextMid (PLFLT x1, PLFLT y1, char *text){
  GraphText(x1,y1,1.0,0.0,0.5, text);
}

/* @func ajGraphVertBars *******************************************
**
** Draw vertical Error Bars.
**
** @param [r] numofpoints [int] number of error bars to be drawn.
** @param [r] x [PLFLT*] x positions to draw at.
** @param [r] ymin [PLFLT*] y positions to start at.
** @param [r] ymax [PLFLT*] y positions to end at.
** @return [void]
**
** @@
*************************************************************************/
void ajGraphVertBars(int numofpoints, PLFLT *x, PLFLT *ymin, PLFLT *ymax){
  ajDebug ("=g= plerry (%d %.2f .. %.2f, %.2f, %.2f) [num, x..x, ymin, ymax]\n",
	   numofpoints, x[0], x[numofpoints-1], ymin, ymax); 
  plerry(numofpoints,x,ymin,ymax);
}


/* @func ajGraphHoriBars *******************************************
**
** Draw Horizontal Error Bars.
**
** @param [r] numofpoints [int] number of error bars to be drawn.
** @param [r] y [PLFLT*] y positions to draw at.
** @param [r] xmin [PLFLT*] x positions to start at.
** @param [r] xmax [PLFLT*] x positions to end at.
** @return [void]
**
** @@
*************************************************************************/
void ajGraphHoriBars(int numofpoints, PLFLT *y, PLFLT *xmin, PLFLT *xmax){
  ajDebug ("=g= plerrx (%d %.2f .. %.2f, %.2f, %.2f) [num, y..y, xmin, xmax]\n",
	   numofpoints, y[0], y[numofpoints-1], xmin, xmax); 
  plerrx(numofpoints,y,xmin,xmax);
}

/* @func ajGraphInitSeq **********************************************
**
** Creates a graph and define default values based on a sequence.
**
** Existing titles and other data are unchanged
**
** @param [r] thys [AjPGraph] Graph
** @param [r] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajGraphInitSeq (AjPGraph thys, AjPSeq seq){

  if (!ajStrLen(thys->title)) {
    (void) ajFmtPrintS(&thys->title, "%s of %s",
		       ajAcdProgram(), ajSeqName(seq));
  }

  (void) ajGraphxySetXRangeII (thys, 1, ajSeqLen(seq));

  return;
}

/* @funcstatic GraphOpenFile *******************************************
**
** A general routine for setting BaseName and extension in plplot. 
**
** @param [r] graphs [AjPGraph] Multiple graph pointer.
** @param [r] ext [char*] file extension
** @return [void]
** @@
******************************************************************************/
static void GraphOpenFile (AjPGraph graphs, char *ext) {
  /*  AjPFile outputfile;*/

  ajDebug ("GraphOpenFile '%S'\n", graphs->outputfile);

  /*  (void) ajStrAppC(&graphs->outputfile,ext);*/

  GraphSetName(graphs, graphs->outputfile,ext);
  
  return;
}

/* @funcstatic GraphOpenXwin *********************************************
**
** A general routine for drawing graphs to an xwin. Does nothing.
**
** @param [r] graphs [AjPGraph] Multiple graph pointer.
** @param [r] ext [char*] file extension
** @return [void]
** @@
******************************************************************************/
static void GraphOpenXwin (AjPGraph graphs, char *ext ) {

  return;
}

/* @funcstatic GraphxyDisplayToFile *******************************************
**
** A general routine for drawing graphs to a file. 
**
** @param [r] graphs [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [char*] file extension
** @return [void]
** @@
******************************************************************************/

static void GraphxyDisplayToFile (AjPGraph graphs, AjBool closeit, char *ext) {

  ajDebug ("ajGraphxyDisplayToFile '%S'\n", graphs->outputfile);

  GraphSetName(graphs, graphs->outputfile,ext);
  (void) ajGraphxyCheckMaxMin(graphs);
  GraphxyGeneral(graphs, closeit);

  return;
}

/* @funcstatic GraphxyDisplayToData *******************************************
**
** A general routine for drawing graphs to file as points. 
**
** @param [r] graphs [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [char*] file extension
** @return [void]
** @@
******************************************************************************/
static void GraphxyDisplayToData (AjPGraph graphs, AjBool closeit, char *ext) {
  AjPFile outf=NULL;
  AjPGraphData g=NULL;
  AjPGraphObj ptr=NULL;
  AjPStr temp;
  int i,j;
  float minxa=64000.;
  float minya=64000.;
  float maxxa=-64000.;
  float maxya=-64000.;
  int type=0;
  
  ajDebug ("ajGraphxyDisplayToData '%S'\n", graphs->outputfile);


  /* Calculate maxima and minima */
  for(i=0;i<graphs->numofgraphs;i++)
  {
      g = (graphs->graphs)[i];
      minxa = (minxa<g->minX) ? minxa : g->minX;
      minya = (minya<g->minY) ? minya : g->minY;
      maxxa = (maxxa>g->maxX) ? maxxa : g->maxX;
      maxya = (maxya>g->maxY) ? maxya : g->maxY;
  }



  for(i=0;i<graphs->numofgraphs;i++){
    g = (graphs->graphs)[i];
    
    /* open a file for dumping the data points */
    temp = ajFmtStr("%S%d%s",graphs->outputfile,i+1,ext);
    outf = ajFileNewOut(temp);
    if(!outf){
      ajMessError("Error could not open file %S\n",temp);
      return;
    }
    else
      ajMessOut("Writing graph %d data to %S\n",i+1,temp);

    
    (void) ajFmtPrintF(outf,"##%S\n",g->gtype);
    (void) ajFmtPrintF(outf,"##Title %S\n",graphs->title);
    (void) ajFmtPrintF(outf,"##Graphs %d\n",graphs->numofgraphs);
    (void) ajFmtPrintF(outf,"##Number %d\n",i+1);
    (void) ajFmtPrintF(outf,"##Points %d\n",g->numofpoints);


    (void) ajFmtPrintF(outf,"##XminA %f XmaxA %f YminA %f YmaxA %f\n",
		       minxa,maxxa,minya,maxya);
    (void) ajFmtPrintF(outf,"##Xmin %f Xmax %f Ymin %f Ymax %f\n",
		       g->tminX,g->tmaxX,g->tminY,g->tmaxY);
    (void) ajFmtPrintF(outf,"##ScaleXmin %f ScaleXmax %f ScaleYmin %f "
		       "ScaleYmax %f\n",g->minX,g->maxX,g->minY,g->maxY);

    (void) ajFmtPrintF(outf,"##Maintitle %S\n",g->title);

    if(graphs->numofgraphs == 1)
    {
	(void) ajFmtPrintF(outf,"##Xtitle %S\n",graphs->xaxis);
	(void) ajFmtPrintF(outf,"##Ytitle %S\n",graphs->yaxis);

    }
    else
    {
	(void) ajFmtPrintF(outf,"##Subtitle %S\n",g->subtitle);
	(void) ajFmtPrintF(outf,"##Xtitle %S\n",g->xaxis);
	(void) ajFmtPrintF(outf,"##Ytitle %S\n",g->yaxis);
    }

    /* Dump out the data points */
    for(j=0;j<g->numofpoints;j++)
      (void) ajFmtPrintF(outf,"%f\t%f\n",g->x[j],g->y[j]);


    /* Now for the Data graphobjs */
    ajFmtPrintF(outf,"##DataObjects\n##Number %d\n",g->numofobjects);
    
    if(g->numofobjects)
    {
	ptr = g->Obj;
	for(j=0;j<g->numofobjects;++j)
	{
	    type = ptr->type;
	    if(type==LINE || type==RECTANGLE || type==RECTANGLEFILL)
	    {
		if(type==LINE)
		    ajFmtPrintF(outf,"Line ");
		else if(type==RECTANGLE)
		    ajFmtPrintF(outf,"Rectangle ");
		else
		    ajFmtPrintF(outf,"Filled Rectangle ");
		ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d\n",
			    ptr->x1,ptr->y1,ptr->x2,ptr->y2,ptr->colour);
	    }
	    else if(type==TEXT)
	    {
		ajFmtPrintF(outf,"Textline ");
		ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d "
			    "size 1.0 %S\n",
			    ptr->x1,ptr->y1,ptr->x2,ptr->y2,ptr->colour,
			    ptr->text);
	    }
	    ptr = ptr->next;
	}
    }
    

    /* Now for the Graph graphobjs */
    ajFmtPrintF(outf,"##GraphObjects\n##Number %d\n",graphs->numofobjects);
    
    if(graphs->numofobjects)
    {
	ptr = graphs->Obj;
	for(j=0;j<graphs->numofobjects;++j)
	{
	    type = ptr->type;
	    if(type==LINE || type==RECTANGLE || type==RECTANGLEFILL)
	    {
		if(type==LINE)
		    ajFmtPrintF(outf,"Line ");
		else if(type==RECTANGLE)
		    ajFmtPrintF(outf,"Rectangle ");
		else
		    ajFmtPrintF(outf,"Filled Rectangle ");
		ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d\n",
			    ptr->x1,ptr->y1,ptr->x2,ptr->y2,ptr->colour);
	    }
	    else if(type==TEXT)
	    {
		ajFmtPrintF(outf,"Textline ");
		ajFmtPrintF(outf,"x1 %f y1 %f x2 %f y2 %f colour %d "
			    "size 1.0 %S\n",
			    ptr->x1,ptr->y1,ptr->x2,ptr->y2,ptr->colour,
			    ptr->text);
	    }
	    ptr = ptr->next;
	}
    }
    

    ajFileClose(&outf);
    ajStrDel(&temp);
    }      

  return;
}


/* @funcstatic GraphxyDisplayXwin *********************************************
**
** A general routine for drawing graphs to an xwin. 
**
** @param [r] graphs [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close file if true
** @param [r] ext [char*] file extension
** @return [void]
** @@
******************************************************************************/
static void GraphxyDisplayXwin (AjPGraph graphs, AjBool closeit, char *ext ) {

  ajGraphxyCheckMaxMin(graphs);
  GraphxyGeneral(graphs, closeit);
  return;
}



/* @func ajGraphxySetOut **********************************************
**
** Set the name of the output file. Only used later if the device
** plotter is capable of postscript output. ps and cps.
**
** @param [w] mult [AjPGraph] Graph structure to write file name too.
** @param [r] txt [AjPStr] Name of the file.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetOut (AjPGraph mult, AjPStr txt) {

  if(!ajStrLen(txt))
    return;

  (void) ajStrAssS(&mult->outputfile, txt);

  return;
}

/* @func ajGraphxySetOutC **********************************************
**
** Set the name of the output file. Only used later if the device
** plotter is capable of postscript output. ps and cps.
**
** @param [w] mult [AjPGraph] Graph structure to write file name too.
** @param [r] txt [char*] Name of the file.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetOutC (AjPGraph mult, char* txt) {

  if(!strlen(txt))
    return;

  (void) ajStrAssC(&mult->outputfile, txt);

  return;
}

/* @func ajGraphxySetLineType **********************************************
**
** Set the line type for this graph.
**
** @param [w] graph [AjPGraphData] Graph structure to store info in.
** @param [r] type [int] Set the line type. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetLineType(AjPGraphData graph, int type) {
  graph->lineType = type;
  return;
}

/* @func ajGraphxySetXStart **********************************************
**
** Set the start position for X in the graph.
** 
**
** @param [w] graphs [AjPGraph] Graph structure to store info in.
** @param [r] val  [float] The start value for x graph coors.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXStart (AjPGraph graphs, float val) {
  graphs->xstart = val;
  return;
}

/* @func ajGraphxySetXEnd **********************************************
**
** Set the end position for X in the graph.
**
** @param [w] graphs [AjPGraph] Graph structure to store info in.
** @param [r] val  [float]  The end value for x graph coors
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXEnd (AjPGraph graphs, float val) {
  graphs->xend = val;
  return;
}

/* @func ajGraphxySetYStart **********************************************
**
** Set the start position for Y in the graph.
**
** @param [w] graphs [AjPGraph] Graph structure to store info in.
** @param [r] val  [float] The end value for y graph coors.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetYStart (AjPGraph graphs, float val) {
  graphs->ystart = val;
  return;
}

/* @func ajGraphxySetYEnd **********************************************
**
** Set the end position for Y in the graph.
**
** @param [w] graphs [AjPGraph] Graph structure to store info in.
** @param [r] val  [float] The start value for y graph coors.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetYEnd (AjPGraph graphs, float val) {
  graphs->yend = val;
  return;
}

/* @func ajGraphxyYtitle **********************************************
**
** Set the title for the Y axis for multiple plot on one graph.
** 
** @param [w] graphs  [AjPGraph] Graph structure to store info in.
** @param [r] title [AjPStr] title for the y axis. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxyYtitle (AjPGraph graphs, AjPStr title){
  (void) ajStrAss(&graphs->yaxis, title);
}
/* @func ajGraphxySetColour **********************************************
**
** Set the colour for the plot on one graph.
** 
** @param [w] graph [AjPGraphData] Graph structure to store info in.
** @param [r] colour [int] colour for this plot. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetColour (AjPGraphData graph, int colour){
  graph->colour = colour;
}

/* @func ajGraphxyYtitleC **********************************************
**
** Set the title for the Y axis for multiple plot on one graph.
** 
** @param [w] graphs  [AjPGraph] Graph structure to store info in.
** @param [r] title [char*] title for the y axis. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxyYtitleC (AjPGraph graphs, char* title){
  (void) ajStrAssC(&graphs->yaxis, title);
  return;
}

/* @func ajGraphxyXtitle **********************************************
**
** Set the title for the X axis for multiple plot on one graph.
** 
** @param [w] graphs  [AjPGraph] Graph structure to store info in.
** @param [r] title [AjPStr] title for the x axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyXtitle (AjPGraph graphs, AjPStr title) {
  (void) ajStrAss(&graphs->xaxis,title);
  return;
}

/* @func ajGraphxyXtitleC **********************************************
**
** Set the title for the X axis for multiple plot on one graph.
** 
** @param [w] graphs  [AjPGraph] Graph structure to store info in.
** @param [r] title [char*] title for the x axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyXtitleC (AjPGraph graphs, char* title) {
  (void) ajStrAssC(&graphs->xaxis,title);
  return;
}

/* @func ajGraphxyTitle **********************************************
**
** Set the graph Title.
** 
** @param [w]  graphs [AjPGraph] Graph structure to store info in.
** @param [r] title [AjPStr]  title. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxyTitle (AjPGraph graphs, AjPStr title) {
  (void) ajStrAss(&graphs->title,title);
  return;
}

/* @func ajGraphxyTitleC **********************************************
**
** Set the graph Title.
** 
** @param [w]  graphs [AjPGraph] Graph structure to store info in.
** @param [r] title [char*]  title. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxyTitleC (AjPGraph graphs, char* title) {
  (void) ajStrAssC(&graphs->title,title);
  return;
}

/* @func ajGraphxySubtitle **********************************************
**
**  Set the title for the Y axis.
**
** @param [w] graphs  [AjPGraph] Graph structure to store info in.
** @param [r] title [AjPStr] title for the Y axis.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySubtitle (AjPGraph graphs, AjPStr title) {
  (void) ajStrAss(&graphs->subtitle,title);
  return;
}

/* @func ajGraphxySubtitleC **********************************************
**
** Set the graph Title.
** 
** @param [w]  graphs [AjPGraph] Graph structure to store info in.
** @param [r] title [char*]  subtitle. 
** @return [void]
** @@
******************************************************************************/

void ajGraphxySubtitleC (AjPGraph graphs, char* title) {
  (void) ajStrAssC(&graphs->subtitle,title);
  return;
}

/* @func ajGraphxyDataSetYtitle *******************************************
**
** Set the title for the Y axis.
**
** @param [w] graphdata  [AjPGraphData] Graph structure to store info in.
** @param [r] title [AjPStr] title for the Y axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetYtitle (AjPGraphData graphdata, AjPStr title) {
  (void) ajStrSet(&graphdata->yaxis,title);
  return;
}

/* @func ajGraphxyDataSetYtitleC ******************************************
**
** Set the title for the Y axis.
**
** @param [w] graphdata  [AjPGraphData] Graph structure to store info in.
** @param [r] title [char*] title for the Y axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetYtitleC (AjPGraphData graphdata, char* title) {
  (void) ajStrSetC(&graphdata->yaxis,title);
  return;
}

/* @func ajGraphxyDataSetXtitle *******************************************
**
** Set the title for the X axis.
**
** @param [w] graphdata  [AjPGraphData] Graph structure to store info in.
** @param [r] title [AjPStr] title for the X axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetXtitle (AjPGraphData graphdata, AjPStr title) {
  (void) ajStrSet(&graphdata->xaxis,title);
  return;
}

/* @func ajGraphxyDataSetXtitleC ******************************************
**
** Set the title for the X axis.
**
** @param [w] graphdata  [AjPGraphData] Graph structure to store info in.
** @param [r] title [char*] title for the X axis.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetXtitleC (AjPGraphData graphdata, char* title) {
  (void) ajStrSetC(&graphdata->xaxis,title);
  return;
}

/* @func ajGraphxyDel ******************************************************
**
** Destructor for a graph object
**
** @param [w] mult  [AjPGraph] Graph structure to store info in.
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDel (AjPGraph mult) {
  AjPGraphData graphdata;
  int i;

  for(i = 0 ; i < mult->numofgraphs ; i++) {
    graphdata = (mult->graphs)[i];
    if(!graphdata->xcalc)
      AJFREE (graphdata->x);
    if(!graphdata->ycalc)
      AJFREE (graphdata->y);
    if(!graphdata->gtype)
	ajStrDel(&graphdata->gtype);
    GraphDataObjDel(graphdata); 
    ajStrDel(&mult->title);
    ajStrDel(&mult->subtitle);
    ajStrDel(&mult->xaxis);
    ajStrDel(&mult->yaxis);
    ajStrDel(&mult->outputfile);
  }
  GraphObjDel(mult);
  
  AJFREE (mult->graphs);
  AJFREE (mult);

  return;
}


/* @funcstatic GraphxyInitData ************************************************
**
** Initialise the contents of a graph object
**
** @param [w] graph  [AjPGraphData] Graph structure to store info in.
** @return [void]
** @@
******************************************************************************/

static void GraphxyInitData (AjPGraphData graph) {

  ajDebug("GraphInitData title: %x subtitle: %x xaxis: %x yaxis: %x\n",
	  graph->title, graph->subtitle, graph->xaxis, graph->yaxis);

  graph->x = 0;
  graph->y = 0;
  graph->xcalc = ajTrue;
  graph->ycalc = ajTrue;
  (void) ajStrSetC(&graph->title,"");
  (void) ajStrSetC(&graph->subtitle,"");
  (void) ajStrSetC(&graph->xaxis,"");
  (void) ajStrSetC(&graph->yaxis,"");
  (void) ajStrSetC(&graph->gtype,"");
  graph->minX = graph->maxX = 0;
  graph->minY = graph->maxY = 0;
  graph->lineType = 1;
  graph->colour = BLACK;
  graph->Obj = 0;

  return;
}


/* @func ajGraphxyDataNew ***************************************************
**
** Creates a new empty graph
**
** @return [AjPGraphData] New empty graph
** @@
******************************************************************************/

AjPGraphData ajGraphxyDataNew (void) {
  static AjPGraphData graph;

  AJNEW0(graph);
  graph->numofpoints = 0;

  return graph;
}  

/* @func ajGraphxyAddDataPtrPtr *************************************************
**
** Adds (x,y) data points defined in two floating point arrays
**
** @param [r] graph [AjPGraphData] Graph object
** @param [r] x [float*] X coordinates
** @param [r] y [float*] Y coordinates
** @return [void]
** @@
******************************************************************************/

void ajGraphxyAddDataPtrPtr (AjPGraphData graph, float *x,float *y) {
  int i;

  for(i=0;i<graph->numofpoints; i++) {
    graph->x[i] = x[i];
    graph->y[i] = y[i];
  }

  return;
}

/* @func ajGraphxyAddDataCalcPtr ************************************************
**
** Adds (x,y) data points defined by an x-axis start and increment
** and a floating point array of y-axis values.
**
** @param [r] graph [AjPGraphData] Graph object
** @param [r] numofpoints [int] Number of points in array
** @param [r] start [float] Start position
** @param [r] incr [float] Increment
** @param [r] y [float*] Y coordinates
** @return [void]
** @@
******************************************************************************/

void ajGraphxyAddDataCalcPtr (AjPGraphData graph, int numofpoints,
			    float start, float incr, float* y) {
  int i;
  PLFLT x = 0.0;

  AJCNEW(graph->x, numofpoints);
  AJCNEW(graph->y, numofpoints);
 
  for(i=0;i<numofpoints; i++) {
    graph->x[i] = (start+x);
    graph->y[i] = y[i];
    x += incr;
  }

  graph->ycalc = ajTrue; /* i.e. OK to delete at the end as it 
		            is our own copy */

  graph->numofpoints = numofpoints;

  return;
}

/* @func ajGraphxySetXRangeII **********************************************
**
** Sets the X axis range with integers
**
** @param [r] thys [AjPGraph] Graph
** @param [r] start [int] start position
** @param [r] end [int] end position
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXRangeII (AjPGraph thys, int start, int end) {

  thys->xstart = (float) start;
  thys->xend = (float) end;

  return;
}

/* @func ajGraphxySetYRangeII **********************************************
**
** Sets the Y axis range with integers
**
** @param [r] thys [AjPGraph] Graph
** @param [r] start [int] start position
** @param [r] end [int] end position
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetYRangeII (AjPGraph thys, int start, int end) {

  thys->ystart = (float) start;
  thys->yend = (float) end;

  return;
}

/* @func ajGraphxyInitGraphCalcPtr ********************************************
**
** Creates a graph and
** adds (x,y) data points defined by an x-axis start and increment
** and a floating point array of y-axis values.
**
** @param [r] numofpoints [int] Number of points in array
** @param [r] start [float] Start position
** @param [r] incr [float] Increment
** @param [r] y [float*] Y coordinates
** @return [AjPGraph] New multi graph object
** @@
******************************************************************************/

AjPGraph ajGraphxyInitGraphCalcPtr (int numofpoints,
				      float start, float incr, float *y) {
  AjPGraph mult;
  AjPGraphData graph;
  
  mult = ajGraphxyNewI(1);

  graph =  ajGraphxyDataNew();
  GraphxyInitData(graph);
  ajGraphxyAddDataCalcPtr(graph,numofpoints,start,incr,y);

  (void) ajGraphxyAddGraph(mult,graph);

  ajGraphxyCheckMaxMin(mult);
  
  return mult;
}

/* @func ajGraphxyDataNewI **********************************************
**
** Create and initialise the data structure for the graph with a defined
** number of data points.
**
** @param [r] numofpoints [int] Number of points
** @return [AjPGraphData] Pointer to new graph structure.
** @@
******************************************************************************/

AjPGraphData ajGraphxyDataNewI (int numofpoints) {

  AjPGraphData graph;

  AJNEW0(graph);

  GraphxyInitData(graph);
  AJCNEW0(graph->x, numofpoints);
  AJCNEW0(graph->y, numofpoints);
  graph->numofpoints = numofpoints;
  return graph;
}

/* @func ajGraphxyAddGraph ************************************************
**
** add another graph structure to the multiple graph structure.
**
** @param [w] mult [AjPGraph] multple graph structure.
** @param [r] graphdata [AjPGraphData] graph to be added.
** @return [int] 1 if graph added successfully else 0;
** @@
******************************************************************************/

int ajGraphxyAddGraph(AjPGraph mult, AjPGraphData graphdata){

  if(mult->numofgraphs)
    if((mult->graphs)[0]->numofpoints != graphdata->numofpoints){
      ajMessError("ERROR only homogenous number of points allowed for multiple graphs\n");
      return 0;
    }
  
  if(mult->numofgraphs < mult->numofgraphsmax){
    (mult->graphs)[mult->numofgraphs++] = graphdata;
    return 1;
  }

  ajMessError("ERROR no space left more graphs in the multiple graph store\n");
  return 0;
}

/* @func ajGraphxyReplaceGraph ************************************************
**
** replace graph structure into the multiple graph structure.
**
** @param [w] mult [AjPGraph] multple graph structure.
** @param [r] graphdata [AjPGraphData] graph to be added.
** @return [int] 1 if graph added successfully else 0;
** @@
******************************************************************************/

int ajGraphxyReplaceGraph(AjPGraph mult, AjPGraphData graphdata){

  (mult->graphs)[0] = graphdata;
  mult->numofgraphs=1;

  mult->minmaxcalc = 0;
  
  return 1;
}

/* @func ajGraphNew ************************************************
**
** Create a structure to hold a general graph.
**
** @return [AjPGraph] multiple graph structure.
** @@
******************************************************************************/

AjPGraph ajGraphNew (void) {
  AjPGraph graph;

  AJNEW0(graph);
  AJCNEW0(graph->graphs,1);

  ajDebug ("ajGraphNew\n");

  graph->numofgraphs = 0;
  graph->numofgraphsmax = 1;
  graph->flags = GRAPH_XY;
  graph->minmaxcalc  = 0;
  (void) ajFmtPrintS(&graph->outputfile,"%s", ajAcdProgram());

  return graph;
}

/* @func ajGraphxyNewI ************************************************
**
** Create a structure to hold a number of graphs.
**
** @param [r] numsets [int] maximum number of graphs that can stored.
** @return [AjPGraph] multiple graph structure.
** @@
******************************************************************************/

AjPGraph ajGraphxyNewI (int numsets) {
  AjPGraph mult;

  AJNEW0(mult);
  AJCNEW0(mult->graphs,numsets);

  ajDebug ("ajGraphxyNewI numsets: %d\n", numsets);

  mult->numofgraphs = 0;
  mult->numofgraphsmax = numsets;
  mult->minX = mult->minY = 64000;
  mult->maxX = mult->maxY = -64000;
  mult->flags = GRAPH_XY;
  mult->minmaxcalc  = 0;
  mult->xstart = mult->xend = 0;
  mult->ystart = mult->yend = 0;
  mult->Obj = 0;
  (void) ajFmtPrintS(&mult->outputfile,"%s", ajAcdProgram());

  return mult;
}

/* @func ajGraphSetMulti ************************************************
**
** Create a structure to hold a number of graphs.
**
** @param [w] thys [AjPGraph] Graph structure to store info in.
** @param [r] numsets [int] maximum number of graphs that can stored.
** @return [void]
** @@
******************************************************************************/

void ajGraphSetMulti (AjPGraph thys, int numsets) {

  AJFREE(thys->graphs);
  AJCNEW0(thys->graphs,numsets);

  ajDebug ("ajGraphSetMulti numsets: %d\n", numsets);

  thys->numofgraphs = 0;
  thys->numofgraphsmax = numsets;
  thys->minmaxcalc  = 0;

  return;
}

/* @func ajGraphxyDataSetTitle **********************************************
**
**  Set the title.
** 
**
** @param [w] graph  [AjPGraphData] Graph structure to store info in.
** @param [r] title [AjPStr] Title
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetTitle (AjPGraphData graph, AjPStr title) {
  (void) ajStrSet(&graph->title,title);
  return;
}


/* @func ajGraphxyDataSetTitleC **********************************************
**
**  Set the title.
** 
**
** @param [w] graph  [AjPGraphData] Graph structure to store info in.
** @param [r] title [char*] Title
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetTitleC (AjPGraphData graph, char *title) {
  (void) ajStrSetC(&graph->title,title);
  return;
}

/* @func ajGraphxyDataSetSubtitle *********************************************
**
**  Set the Subtitle.
** 
**
** @param [w] graph  [AjPGraphData] Graph structure to store info in.
** @param [r] title [AjPStr] Sub Title
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetSubtitle (AjPGraphData graph, AjPStr title) {
  (void) ajStrSet(&graph->subtitle,title);
  return;
}


/* @func ajGraphxyDataSetSubtitleC ********************************************
**
**  Set the subtitle.
** 
**
** @param [w] graph  [AjPGraphData] Graph structure to store info in.
** @param [r] title [char*] Sub Title
** @return [void]
** @@
******************************************************************************/

void ajGraphxyDataSetSubtitleC (AjPGraphData graph, char *title) {
  (void) ajStrSetC(&graph->subtitle,title);
  return;
}


/* @func ajGraphxySetFlag **********************************************
**
** Set the flags for the xy graph to add or subract "flag" depending on istrue
**
** @param [w] graphs [AjPGraph] graph to have flags altered.
** @param [r] flag   [int]      flag to be set.
** @param [r] istrue [AjBool]   whether to set the flag or remove it. 
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetFlag(AjPGraph graphs,int flag, AjBool istrue){ 
  if(graphs->flags & flag){
    if(!istrue)
      graphs->flags -= flag;
  }
  else{
    if(istrue)
      graphs->flags += flag;
  }
}
/* @func ajGraphxySetOverLap *******************************************
**
** Set whether the graphs should lay on top of each other.
**
** @param [r] graphs [AjPGraph] Multiple graph object
** @param [r] overlap [AjBool] if true overlap else do not.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetOverLap(AjPGraph graphs, AjBool overlap) {
  if(graphs->flags & AJGRAPH_OVERLAP){
    if(!overlap)
      graphs->flags -= AJGRAPH_OVERLAP;
  }
  else{
    if(overlap)
      graphs->flags += AJGRAPH_OVERLAP;
  }
}

/* @func ajGraphxySetGaps *****************************************************
**
** Set whether the graphs should enable gaps.
**
** @param [r] graphs [AjPGraph] Multiple graph object
** @param [r] overlap [AjBool] if true allowgaps else do not.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetGaps(AjPGraph graphs, AjBool overlap) {
  if(graphs->flags & AJGRAPH_GAPS){
    if(!overlap)
      graphs->flags -= AJGRAPH_GAPS;
  }
  else{
    if(overlap)
      graphs->flags += AJGRAPH_GAPS;
  }
}

/* @func ajGraphxySetXBottom *********************************************
**
** Set whether the graph is to display a bottom x axis. 
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetXBottom(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_BOTTOM, set);
}


/* @func ajGraphxySetXTop *************************************************
**
** Set whether the graph is to display the left X axis at the top.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/

void ajGraphxySetXTop(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_TOP, set);
}


/* @func ajGraphxySetYRight ***********************************************
**
** Set the graph is to display a right hand Y axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYRight(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_RIGHT, set);
}


/* @func ajGraphxySetYLeft ************************************************
**
** Set whether the graph is to display the left Y axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYLeft(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_LEFT, set);
}


/* @func ajGraphxySetXTick ************************************************
**
** Set whether the graph is to display tick marks on the x axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetXTick(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_TICK, set);
}

/* @func ajGraphxySetYTick ************************************************
**
** Set the graph is to display tick marks on the y axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYTick(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_TICK, set);
}


/* @func ajGraphxySetXLabel ***************************************************
**
** Set whether the graph is to label the x axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetXLabel(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_LABEL, set);
}


/* @func ajGraphxySetYLabel ***************************************************
**
** Set whether the graph is to label the y axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYLabel(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_LABEL, set);
}


/* @func ajGraphxySetTitleDo *********************************************
**
** Set whether the graph is to display the title.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetTitleDo(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_TITLE, set);
}

/* @func ajGraphxySetSubtitleDo ******************************************
**
** Set whether the graph is to display the subtitle.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetSubtitleDo(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_SUBTITLE, set);
}


/* @func ajGraphxySetCirclePoints *********************************************
**
** Set the graph to draw circles at the points.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetCirclePoints(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_CIRCLEPOINTS, set);
}


/* @func ajGraphxySetJoinPoints ***********************************************
**
** Set the graph to draw lines between the points.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetJoinPoints(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_JOINPOINTS, set);
}


/* @func ajGraphxySetXLabelTop ********************************************
**
** Set whether the graph is to display the labels on the top x  axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetXLabelTop(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_NUMLABEL_ABOVE, set);
}


/* @func ajGraphxySetYLabelLeft *******************************************
**
** Set whether the graph is to display the labels on the left hand axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYLabelLeft(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_NUMLABEL_LEFT, set);
}


/* @func ajGraphxySetXInvTicks *********************************************
**
** Set whether the graph is to display the tick marks inside the plot on
** the x axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetXInvTicks(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_INVERT_TICK, set);
}


/* @func ajGraphxySetYInvTicks *********************************************
**
** Set whether the graph is to display the tick marks inside the plot
** on the y axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYInvTicks(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_INVERT_TICK, set);
}


/* @func ajGraphxySetXGrid ****************************************************
**
** Set whether the graph is to grid the tick marks on the x axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetXGrid(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_X_GRID, set);
}


/* @func ajGraphxySetYGrid ****************************************************
**
** Set whether the graph is to grid the tick marks on the x axis.
**
** @param [rw] graphs [AjPGraph] graph to have flag altered
** @param [r]  set    [AjBool]   whether to set or turn off.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetYGrid(AjPGraph graphs, AjBool set){
  ajGraphxySetFlag(graphs, AJGRAPH_Y_GRID, set);
}

/* @func ajGraphxySetMaxMin ***********************************************
**
** Set the max and min of the data points for all graphs. 
**
** @param [rw] graphs [AjPGraph] multple graph structure.
** @param [r] xmin [float]  x min.
** @param [r] xmax [float]  x max.
** @param [r] ymin [float]  y min.
** @param [r] ymax [float]  y max.
** @return [void]
** @@
******************************************************************************/
void ajGraphxySetMaxMin(AjPGraph graphs,float xmin,float xmax,
			float ymin,float ymax){
  AjPGraphData graphdata;
  int i;

  graphs->minX = xmin; 
  graphs->minY = ymin;
  graphs->maxX = xmax;
  graphs->maxY = ymax;
  for(i = 0 ; i < graphs->numofgraphs ; i++) {
    graphdata = (graphs->graphs)[i];
    graphdata->minX = xmin; 
    graphdata->minY = ymin;
    graphdata->maxX = xmax;
    graphdata->maxY = ymax;
  }
}

/* @func ajGraphDataxySetMaxMin *********************************************
**
** Set the max and min of the data points you wish to display. 
**
** @param [rw] graphdata [AjPGraphData] multple graph structure.
** @param [r] xmin [float]  x min.
** @param [r] xmax [float]  x max.
** @param [r] ymin [float]  y min.
** @param [r] ymax [float]  y max.
** @return [void]
** @@
******************************************************************************/
void ajGraphDataxySetMaxMin(AjPGraphData graphdata, float xmin, float xmax,
			    float ymin, float ymax){

    graphdata->minX = xmin; 
    graphdata->minY = ymin;
    graphdata->maxX = xmax;
    graphdata->maxY = ymax;

}

/* @func ajGraphDataxyMaxMin *********************************************
**
** Get the max and min of the data points you wish to display. 
**
** @param [r] array [float*] array
** @param [r] npoints
** @param [w] min [float]  min.
** @param [w] max [float]  max.
** @return [void]
** @@
******************************************************************************/
void ajGraphDataxyMaxMin(float *array, int npoints, float *min, float *max)
{
    int i;

    *min = 64000.;
    *max = -64000.;
    
    for(i=0;i<npoints;++i)
    {
	*min = (*min < array[i]) ? *min : array[i];
	*max = (*max > array[i]) ? *max : array[i];
    }

    return;
}


/* @func ajGraphDataxySetMaxima *********************************************
**
** Set the scale max and min of the data points you wish to display. 
**
** @param [rw] graphdata [AjPGraphData] multple graph structure.
** @param [r] xmin [float]  true x min.
** @param [r] xmax [float]  true x max.
** @param [r] ymin [float]  true y min.
** @param [r] ymax [float]  true y max.
** @return [void]
** @@
******************************************************************************/
void ajGraphDataxySetMaxima(AjPGraphData graphdata, float xmin, float xmax,
			   float ymin, float ymax){

    graphdata->tminX = xmin; 
    graphdata->tminY = ymin;
    graphdata->tmaxX = xmax;
    graphdata->tmaxY = ymax;

}


/* @func ajGraphDataxySetTypeC *********************************************
**
** Set the type of the graph for data output. 
**
** @param [rw] graphdata [AjPGraphData] multple graph structure.
** @param [r] type [char*]  Type e.g. "2D Plot", "Histogram".
** @return [void]
** @@
******************************************************************************/
void ajGraphDataxySetTypeC(AjPGraphData graphdata, char* type)
{

    (void) ajStrAssC(&graphdata->gtype,type);
    return;
}


/* @func ajGraphxyCheckMaxMin **********************************************
**
** Calculate the max and min of the data points and store them.
**
** @param [rw] graphs [AjPGraph] multiple graph structure.
** @return [void]
** @@
******************************************************************************/
void ajGraphxyCheckMaxMin (AjPGraph graphs) {
  AjPGraphData graphdata=NULL;
  int i,j;
  
  
  for(i = 0 ; i < graphs->numofgraphs ; i++) {
    graphdata = (graphs->graphs)[i];
    if(graphdata->minX == graphdata->maxX ||
       graphdata->minY == graphdata->maxY){

      graphdata->minX = graphdata->minY =  64000;
      graphdata->maxX = graphdata->maxY =  -64000;
      for( j = 0 ; j < graphdata->numofpoints; j++) {
	if(graphdata->maxX < (graphdata->x)[j])
	  graphdata->maxX = (graphdata->x)[j];
	if(graphdata->maxY < (graphdata->y)[j])
	  graphdata->maxY = (graphdata->y)[j];
	if(graphdata->minX > (graphdata->x)[j])
	  graphdata->minX = (graphdata->x)[j];
	if(graphdata->minY > (graphdata->y)[j])
	  graphdata->minY = (graphdata->y)[j];
      }
    }

    if (graphs->minX > graphdata->minX)
      graphs->minX = graphdata->minX; 
    if (graphs->minY > graphdata->minY)
      graphs->minY = graphdata->minY; 
    if (graphs->maxX < graphdata->maxX)
      graphs->maxX = graphdata->maxX; 
    if (graphs->maxY < graphdata->maxY)
      graphs->maxY = graphdata->maxY; 
  }

  if(!graphs->minmaxcalc){
    graphs->xstart = graphs->minX;
    graphs->xend = graphs->maxX;
    graphs->ystart = graphs->minY;
    graphs->yend = graphs->maxY;
    graphs->minmaxcalc = ajTrue;
  }  
}

/* @funcstatic GraphxyGeneral **********************************************
**
** A general routine for drawing graphs. 
**
** @param [r] graphs [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool] Close at end if true.
** @return [void]
** @@
******************************************************************************/

static void GraphxyGeneral (AjPGraph graphs, AjBool closeit) {

  AjPGraphData g;
  int i,old,old2;
  AJTIME ajtime;
  const time_t tim = time(0);      

  ajtime.time = localtime(&tim);
  ajtime.format = 0;

  ajGraphSetDevice(graphs);

  if(graphs->flags & AJGRAPH_OVERLAP){
    ajGraphColourBack();
    GraphInit(graphs);
    ajGraphColourFore();
    /*    GraphSubPage(0);         Done in ajGraphplenv*/

    g = (graphs->graphs)[0];    
    graphs->xstart = g->minX;
    graphs->xend   = g->maxX;
    graphs->ystart = g->minY;
    graphs->yend   = g->maxY;

    ajGraphPlenv(graphs->xstart, graphs->xend,
		 graphs->ystart, graphs->yend, graphs->flags);
    
    if((graphs->flags & AJGRAPH_TITLE) && ajStrLen(graphs->title) <=1){
      (void) ajStrAppC(&graphs->title,ajFmtString("%s (%D)",
						  ajAcdProgram(),&ajtime));
    }
    ajGraphLabel(((graphs->flags & AJGRAPH_X_LABEL) ?
		      ajStrStr(graphs->xaxis) : " "),
		     ((graphs->flags & AJGRAPH_Y_LABEL) ?
		      ajStrStr(graphs->yaxis) : " "), 
	  ((graphs->flags & AJGRAPH_TITLE) ?
	   ajStrStr(graphs->title) : " "),
	   (graphs->flags & AJGRAPH_SUBTITLE) ?
		     ajStrStr(graphs->subtitle) : " ");
      
    for(i=0;i<graphs->numofgraphs;i++){
      g = (graphs->graphs)[i];

      /* Draw the line through the data */
      old = ajGraphSetFore(g->colour);
      if(graphs->flags & AJGRAPH_CIRCLEPOINTS)
	ajGraphSymbols(g->numofpoints,(g->x), (g->y),  9);

      old2 = ajGraphSetLineStyle(g->lineType);
      if(graphs->flags & AJGRAPH_JOINPOINTS){
	if(graphs->flags & AJGRAPH_GAPS)
	  GraphArrayGaps(g->numofpoints, (g->x), (g->y));
	else
	  GraphArray(g->numofpoints, (g->x), (g->y));
      }
      (void) ajGraphSetLineStyle(old2);
      (void) ajGraphSetFore(old);
    }
    GraphObjDraw(graphs);
      
  }
  else{
    GraphSetNumSubPage(graphs->numofgraphs);
    ajGraphColourBack();
    GraphInit(graphs);
    ajGraphColourFore();
    for(i=0;i<graphs->numofgraphs;i++){
      g = (graphs->graphs)[i];
      ajGraphPlenv(g->minX, g->maxX,
		   g->minY, g->maxY, graphs->flags);
      ajGraphLabel(((graphs->flags & AJGRAPH_X_LABEL) ? ajStrStr(g->xaxis) : " "),
	    ((graphs->flags & AJGRAPH_Y_LABEL) ? ajStrStr(g->yaxis) : " "), 
	    ((graphs->flags & AJGRAPH_TITLE) ? ajStrStr(g->title) : " "),
	     (graphs->flags & AJGRAPH_SUBTITLE) ? ajStrStr(g->subtitle) :" ");
      old = ajGraphSetFore(g->colour);
      if(graphs->flags & AJGRAPH_CIRCLEPOINTS)
	ajGraphSymbols(g->numofpoints,(g->x), (g->y), 9);

      /* Draw the line through the data */
      /*    old = ajGraphSetFore(i+2);*/
      old2 = ajGraphSetLineStyle(g->lineType);
      if(graphs->flags & AJGRAPH_JOINPOINTS){
	if(graphs->flags & AJGRAPH_GAPS)
	  GraphArrayGaps(g->numofpoints, (g->x), (g->y));
	else
	  GraphArray(g->numofpoints, (g->x), (g->y));
      }
      (void) ajGraphSetLineStyle(old2);
      (void) ajGraphSetFore(old);
      GraphDataObjDraw(g);
    }
    
  }
  if(closeit)
    GraphClose();
}

/* @func ajGraphxyDisplay *************************************************
**
** A general routine for drawing graphs. 
**
** @param [r] graphs  [AjPGraph] Multiple graph pointer.
** @param [r] closeit [AjBool]   Whether to close graph at the end.
** @return [void]
** @@
*************************************************************************/
void ajGraphxyDisplay (AjPGraph graphs, AjBool closeit) {

  (graphType[graphs->displaytype].XYDisplay)
    (graphs, closeit, graphType[graphs->displaytype].ext);
  return;
}


/* @func ajGraphObjAddRect ****************************************
**
** Add a Rectangle to be drawn when the graph is plotted, fill states whether
** the rectangle should be filled in.
**
** @param [r] graphs [AjPGraph] Graph object
** @param [r] x1 [float] Start x coordinate
** @param [r] y1 [float] Start y coordinate
** @param [r] x2 [float] End x coordinate
** @param [r] y2 [float] End y coordinate
** @param [r] colour [int] Colour code (see PLPLOT)
** @param [r] fill [int] Fill code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphObjAddRect(AjPGraph graphs, float x1, float y1,
				   float x2, float y2, int colour, int fill){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    AJNEW((graphs->Obj));
    Obj = graphs->Obj; 
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj->next)
      Obj = Obj->next;
    AJNEW(Obj->next);
    Obj = Obj->next;
  }

  ++graphs->numofobjects;


  if(fill)
    Obj->type = RECTANGLEFILL;
  else
    Obj->type = RECTANGLE;
  Obj->text = 0;
  Obj->x1 = x1;
  Obj->x2 = x2;
  Obj->y1 = y1;
  Obj->y2 = y2;
  Obj->colour = colour;
  Obj->next = 0;
}

/* @func ajGraphObjAddText *********************************************
**
** Add text to be drawn when the graph is plotted.
**
** @param [r] graphs [AjPGraph] Graph object
** @param [r] x1 [float] Start x position
** @param [r] y1 [float] Start y position
** @param [r] colour [int] Colour code (see PLPLOT)
** @param [r] text [char*] Text
**
** @return [void]
** @@
******************************************************************************/

void ajGraphObjAddText(AjPGraph graphs, float x1, float y1,
			      int colour, char *text){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    AJNEW((graphs->Obj));
    Obj = graphs->Obj; 
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj->next)
      Obj = Obj->next;
    AJNEW(Obj->next);
    Obj = Obj->next;
  }

  ++graphs->numofobjects;


  Obj->type = TEXT;
  Obj->text = 0;
  (void) ajStrSetC(&Obj->text,text);
  Obj->x1 = x1;
  Obj->x2 = 0.0;
  Obj->y1 = y1;
  Obj->y2 = 0.0;
  Obj->colour = colour;
  Obj->next = 0;
}

/* @func ajGraphObjAddLine *********************************************
**
** Add a line to be drawn when the graph is plotted.
**
** @param [r] graphs [AjPGraph] Graph object
** @param [r] x1 [float] Start x position
** @param [r] y1 [float] Start y position
** @param [r] x2 [float] End x position
** @param [r] y2 [float] End y position
** @param [r] colour [int] Colour code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphObjAddLine(AjPGraph graphs, float x1, float y1,
			      float x2, float y2, int colour){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    AJNEW((graphs->Obj));
    Obj = graphs->Obj; 
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj->next)
      Obj = Obj->next;
    AJNEW(Obj->next);
    Obj = Obj->next;
  }

  ++graphs->numofobjects;
  
  Obj->type = LINE;
  Obj->text = 0;
  Obj->x1 = x1;
  Obj->x2 = x2;
  Obj->y1 = y1;
  Obj->y2 = y2;
  Obj->colour = colour;
  Obj->next = 0;
}


/* @func ajGraphDataObjDel ***************************************************
**
** Delete all objects from a graph data object.
**
** @param [w] thys [AjPGraphData*] Graph data object
**
** @return [void]
** @@
******************************************************************************/

void ajGraphDataObjDel(AjPGraphData *thys)
{
    AjPGraphObj here=NULL;
    AjPGraphObj p=NULL;

    here = p = (*thys)->Obj;
    while(p)
    {
	p = here->next;
	ajStrDel(&here->text);
	AJFREE(here);
	here = p;
    }

    (*thys)->numofobjects = 0;
    (*thys)->Obj = NULL;

    return;
}

/* @func ajGraphObjDel ***************************************************
**
** Delete all objects from a graph object.
**
** @param [w] thys [AjPGraph*] Graph object
**
** @return [void]
** @@
******************************************************************************/

void ajGraphObjDel(AjPGraph *thys)
{
    AjPGraphObj here=NULL;
    AjPGraphObj p=NULL;

    here = p = (*thys)->Obj;
    while(p)
    {
	p = here->next;
	ajStrDel(&here->text);
	AJFREE(here);
	here = p;
    }

    (*thys)->numofobjects = 0;
    (*thys)->Obj = NULL;

    return;
}

/* @func ajGraphDataDel ***************************************************
**
** Delete a graph data object.
**
** @param [w] thys [AjPGraphData*] Graph data object
**
** @return [void]
** @@
******************************************************************************/

void ajGraphDataDel(AjPGraphData *thys)
{
    AjPGraphData this = *thys;
    
    AJFREE(this->x);
    AJFREE(this->y);
    ajStrDel(&this->title);
    ajStrDel(&this->subtitle);
    ajStrDel(&this->xaxis);
    ajStrDel(&this->yaxis);
    ajStrDel(&this->gtype);

    ajGraphDataObjDel(thys);

    AJFREE(this);

    return;
}

/* @funcstatic GraphObjPrint *****************************************
**
** Print all the drawable objects in readable form.
**
** @param [r] graphs [AjPGraph] Graph object
**
** @return [void]
** @@
******************************************************************************/

static void GraphObjPrint(AjPGraph graphs){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    ajUser("No Objects");
    return;
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj){
      if(Obj->type == RECTANGLE )
	ajUser("type = RECTANGLE, %f %f %f %f col= %d",
	       Obj->x1, Obj->y1,
	       Obj->x2,Obj->y2,Obj->colour);
      else if(Obj->type == RECTANGLEFILL )
	ajUser("type = RECTANGLEFILL, %f %f %f %f col= %d",
	       Obj->x1, Obj->y1,
	       Obj->x2,Obj->y2,Obj->colour);
      else if(Obj->type == TEXT)
	ajUser("type = TEXT, %f %f col= %d %S",
	       Obj->x1, Obj->y1,Obj->colour,
	      Obj->text); 
      else if(Obj->type == LINE )
	ajUser("type = LINE, %f %f %f %f col= %d",
	       Obj->x1, Obj->y1,
	       Obj->x2,Obj->y2,Obj->colour);
     Obj = Obj->next;
    }
  }
}

/* @funcstatic GraphObjDraw ******************************************
**
** Display the drawable objects connected to this graph.
**
** @param [r] graphs [AjPGraph] Graph object
**
** @return [void]
** @@
******************************************************************************/

static void GraphObjDraw(AjPGraph graphs){
  AjPGraphObj Obj;
  int temp;

  if(!graphs->Obj){
    return;
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj){
      if(Obj->type == RECTANGLE){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphRect(Obj->x1, Obj->y1,Obj->x2,
			 Obj->y2);
	(void) ajGraphSetFore(temp);
      }
      else if(Obj->type == RECTANGLEFILL){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphRectFill(Obj->x1, Obj->y1,Obj->x2,
			     Obj->y2);
	(void) ajGraphSetFore(temp);
      }
      else if(Obj->type == TEXT ){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphTextStart(Obj->x1, Obj->y1,
			       ajStrStr(Obj->text));
	(void) ajGraphSetFore(temp);
      }
      else if(Obj->type == LINE){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphLine(Obj->x1, Obj->y1,Obj->x2,
			Obj->y2);
	(void) ajGraphSetFore(temp);
      }
      else
	ajUser("UNDEFINED OBJECT TYPE USED");
      Obj = Obj->next;
    }
  }
}

/* @funcstatic GraphObjDel ****************************************
**
**  Delete all the drawable objects connected to the graph object.
**
** @param [r] graphs [AjPGraph] Graph object
**
** @return [void]
** @@
******************************************************************************/

static void GraphObjDel(AjPGraph graphs){
  AjPGraphObj Obj,anoth;

  if(!graphs->Obj){
    return;
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj){
      	anoth = Obj->next;
	AJFREE(Obj);
	Obj = anoth;
      }
  }
  graphs->Obj = 0;
}

/* @func ajGraphDataObjAddRect ************************************
**
** Add a Rectangle to be drawn when the graph is plotted, fill states whether
** the rectangle should be filled in.
**
** @param [r] graphs [AjPGraphData] Graph data object
** @param [r] x1 [float] Start x position
** @param [r] y1 [float] Start y position
** @param [r] x2 [float] End x position
** @param [r] y2 [float] End y position
** @param [r] colour [int] Colour code (see PLPLOT)
** @param [r] fill [int] Fill code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphDataObjAddRect (AjPGraphData graphs,
				       float x1, float y1,
				       float x2, float y2,
				       int colour, int fill){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    AJNEW((graphs->Obj));
    Obj = graphs->Obj; 
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj->next)
      Obj = Obj->next;
    AJNEW(Obj->next);
    Obj = Obj->next;
  }

  ++graphs->numofobjects;


  if(fill)
    Obj->type = RECTANGLEFILL;
  else
    Obj->type = RECTANGLE;
  Obj->text = 0;
  Obj->x1 = x1;
  Obj->x2 = x2;
  Obj->y1 = y1;
  Obj->y2 = y2;
  Obj->colour = colour;
  Obj->next = 0;
}

/* @func ajGraphDataObjAddText *****************************************
**
** Add Text to be drawn when the graph is plotted.
**
** @param [r] graphs [AjPGraphData] Graph data object
** @param [r] x1 [float] Start x position
** @param [r] y1 [float] Start y position
** @param [r] colour [int] Colour code (see PLPLOT)
** @param [r] text [char*] Text to add
**
** @return [void]
** @@
******************************************************************************/

void ajGraphDataObjAddText(AjPGraphData graphs, float x1, float y1,
				  int colour, char *text){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    AJNEW((graphs->Obj));
    Obj = graphs->Obj; 
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj->next)
      Obj = Obj->next;
    AJNEW(Obj->next);
    Obj = Obj->next;
  }

  ++graphs->numofobjects;


  Obj->type = TEXT;
  Obj->text = 0;
  (void) ajStrSetC(&Obj->text,text);
  Obj->x1 = x1;
  Obj->x2 = 0.0;
  Obj->y1 = y1;
  Obj->y2 = 0.0;
  Obj->colour = colour;
  Obj->next = 0;
}

/* @func ajGraphDataObjAddLine *****************************************
**
** Add a line to be drawn when the graph is plotted.
**
** @param [r] graphs [AjPGraphData] Graph data object
** @param [r] x1 [float] Start x position
** @param [r] y1 [float] Start y position
** @param [r] x2 [float] End x position
** @param [r] y2 [float] End y position
** @param [r] colour [int] Colour code (see PLPLOT)
**
** @return [void]
** @@
******************************************************************************/

void ajGraphDataObjAddLine(AjPGraphData graphs, float x1, float y1,
				  float x2, float y2, int colour){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    AJNEW((graphs->Obj));
    Obj = graphs->Obj; 
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj->next)
      Obj = Obj->next;
    AJNEW(Obj->next);
    Obj = Obj->next;
  }

  ++graphs->numofobjects;

  Obj->type = LINE;
  Obj->text = 0;
  Obj->x1 = x1;
  Obj->x2 = x2;
  Obj->y1 = y1;
  Obj->y2 = y2;
  Obj->colour = colour;
  Obj->next = 0;
}

/* @funcstatic GraphDataObjPrint *************************************
**
** Print all the drawable objects in readable form.
**
** @param [r] graphs [AjPGraphData] Graph data object
**
** @return [void]
** @@
******************************************************************************/

static void GraphDataObjPrint(AjPGraphData graphs){
  AjPGraphObj Obj;

  if(!graphs->Obj){
    ajUser("No Objects");
    return;
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj){
      if(Obj->type == RECTANGLE )
	ajUser("type = RECTANGLE, %f %f %f %f col= %d",
	       Obj->x1,Obj->y1,
	       Obj->x2,Obj->y2,Obj->colour);
      else if(Obj->type == RECTANGLEFILL )
	ajUser("type = RECTANGLEFILL, %f %f %f %f col= %d",
	       Obj->x1, Obj->y1,
	       Obj->x2,Obj->y2,Obj->colour);
      else if(Obj->type == TEXT)
	ajUser("type = TEXT, %f %f col= %d %S",Obj->x1,
	       Obj->y1,Obj->colour,
	      Obj->text); 
      else if(Obj->type == LINE )
	ajUser("type = LINE, %f %f %f %f col= %d",Obj->x1,
	       Obj->y1,
	       Obj->x2,Obj->y2,Obj->colour);
     Obj = Obj->next;
    }
  }
}


/* @funcstatic GraphDataObjDraw **************************************
**
** Display the drawable objects connected to this graph.
**
** @param [R] graphs [AjPGraphData] Graph data object
**
** @return [void]
** @@
******************************************************************************/

static void GraphDataObjDraw(AjPGraphData graphs){
  AjPGraphObj Obj;
  int temp;

  if(!graphs->Obj){
    return;
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj){
      if(Obj->type == RECTANGLE){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphRect(Obj->x1, Obj->y1,
			 Obj->x2,Obj->y2);
	(void) ajGraphSetFore(temp);
      }
      else if(Obj->type == RECTANGLEFILL){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphRectFill(Obj->x1, Obj->y1,
			     Obj->x2,Obj->y2);
	(void) ajGraphSetFore(temp);
      }
      else if(Obj->type == TEXT ){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphTextStart(Obj->x1, Obj->y1,
			       ajStrStr(Obj->text));
	(void) ajGraphSetFore(temp);
      }
      else if(Obj->type == LINE){
	temp = ajGraphSetFore(Obj->colour);
	ajGraphLine(Obj->x1, Obj->y1,
			Obj->x2,Obj->y2);
	(void) ajGraphSetFore(temp);
      }
      else
	ajUser("UNDEFINED OBJECT TYPE USED");
      Obj = Obj->next;
    }
  }
}

/* @funcstatic GraphDataObjDel ************************************
**
** Delete all the drawable objects connected to the graphdata object.
**
** @param [r] graphs [AjPGraphData] Graph data object
**
** @return [void]
** @@
******************************************************************************/

static void GraphDataObjDel(AjPGraphData graphs){
  AjPGraphObj Obj,anoth;

  if(!graphs->Obj){
    return;
  }
  else { /* cycle through till NULL found */
    Obj = graphs->Obj;
    while(Obj){
      	anoth = Obj->next;
	AJFREE(Obj);
	Obj = anoth;
      }
  }
  graphs->Obj = 0;
}
/*****************************************************************************
Functions needed for callRegister routines.
*****************************************************************************/

/* @funcstatic GraphSet2 ***********************************************
**
** Calls ajGraphSet and saves the return value in a variable because
** callRegister loses return values
**
** @param [r] thys [AjPGraph] Graph object 
** @param [r] type [AjPStr] Graph type
** @param [w] res [AjBool*] Result of ajGraphSet
** @return [AjBool] always returns ajTrue
** @@
******************************************************************************/

static AjBool GraphSet2(AjPGraph thys, AjPStr type, AjBool *res){
  AjBool retval=AJTRUE;
  
  *res = ajGraphSet(thys, type);

  return retval;
}

/* @funcstatic GraphxySet2 ***********************************************
**
** Calls ajGraphxySet and saves the return value in a variable because
** callRegister loses return values
**
** @param [r] thys [AjPGraph] Graph object 
** @param [r] type [AjPStr] Graph type
** @param [w] res [AjBool*] Result of ajGraphxySet
** @return [AjBool] always returns ajTrue
** @@
******************************************************************************/

static AjBool GraphxySet2(AjPGraph thys, AjPStr type,AjBool *res){
  AjBool retval=AJTRUE;
  
  *res = ajGraphxySet(thys, type);

  return retval;
}

/* @funcstatic GraphSetarg ***********************************************
**
** Passes argument list to GraphSet2. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph,
**                           AjPStr, AjBool*)
** @return [AjBool] return value from GraphSet2
** @@
******************************************************************************/

static AjBool GraphSetarg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool *temp3 = NULL;
  AjBool retval;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  temp3 = va_arg(args, AjBool *);
  
  retval = GraphSet2(temp,temp2,temp3);

  return retval;
}
/* @funcstatic GraphxySetarg ***********************************************
**
** Passes argument list to GraphxySet2. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph,
**                           AjPStr, AjBool*)
** @return [AjBool] return value from ajGraphxySet2
** @@
******************************************************************************/

static AjBool GraphxySetarg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool *temp3 = NULL;
  AjBool retval;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  temp3 = va_arg(args, AjBool *);
  
  retval = GraphxySet2(temp,temp2,temp3);

  return retval;
}
/* @funcstatic GraphxyTitlearg **********************************************
**
** Passes argument list to GraphxyTitle. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph, AjPStr)
** @return [AjBool] always ajTrue.
** @@
******************************************************************************/

static AjBool GraphxyTitlearg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool retval = AJTRUE;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  
  ajGraphxyTitle(temp,temp2);

  return retval;
}
/* @funcstatic GraphxySubtitlearg *******************************************
**
** Passes argument list to ajGraphxySubtitle. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph, AjPStr)
** @return [AjBool] always ajTrue.
** @@
******************************************************************************/

static AjBool GraphxySubtitlearg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool retval = AJTRUE;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  
  ajGraphxySubtitle(temp,temp2);

  return retval;
}

/* @funcstatic GraphxyXtitlearg *****************************************
**
** Passes argument list to GraphxyXtitle. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph, AjPStr)
** @return [AjBool] always ajTrue.
** @@
******************************************************************************/

static AjBool GraphxyXtitlearg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool retval = AJTRUE;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  
  ajGraphxyXtitle(temp,temp2);

  return retval;
}
/* @funcstatic GraphxyYtitlearg *****************************************
**
** Passes argument list to ajGraphxyYtitle. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph, AjPStr)
** @return [AjBool] always ajTrue.
** @@
******************************************************************************/

static AjBool GraphxyYtitlearg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool retval = AJTRUE;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  
  ajGraphxyYtitle(temp,temp2);

  return retval;
}

/* @funcstatic GraphxySetOutarg **************************************
**
** Passes argument list to ajGraphxySetOut. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph, AjPStr)
** @return [AjBool] always ajTrue.
** @@
*****************************************************************************/

static AjBool GraphxySetOutarg(char *name, va_list args) {
  AjPGraph temp = NULL;
  AjPStr temp2 = NULL;
  AjBool retval = AJTRUE;

  temp = va_arg(args, AjPGraph);
  temp2 = va_arg(args, AjPStr);
  
  ajGraphxySetOut(temp,temp2);

  return retval;
}

/* @funcstatic GraphxyNewIarg ***********************************************
**
** Passes argument list to ajGraphxyNewI. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (int)
** @return [AjPGraph] New graph object
** @@
******************************************************************************/

static AjPGraph GraphxyNewIarg(char *name, va_list args){
  int temp = 0;
  AjPGraph retval;

  temp = va_arg(args, int);
  retval = ajGraphxyNewI(temp);

  return retval;
}

/* @funcstatic GraphTracearg ***********************************************
**
** Passes argument list to ajGraphTrace. Note that the callRegister
** method prevents any prototype checking on the call.
**
** @param [r] name [char*] Function name, required by callRegister but ignored.
** @param [r] args [va_list] Argument list, really must be (AjPGraph)
** @return [AjBool] always ajTrue.
** @@
******************************************************************************/

static AjBool GraphTracearg(char *name, va_list args){
  AjPGraph temp = NULL;
  AjBool retval = AJTRUE;

  temp = va_arg(args, AjPGraph);
  
  ajGraphTrace(temp);

  return retval;
}

/* @func ajGraphInit ******************************************************
**
** Initialises the graphics then everything else. Reads an ACD
** (AJAX Command Definition) file, 
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [int] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/

AjStatus ajGraphInit (char *pgm, int argc, char *argv[]) {

  ajNamInit("emboss");

  GraphRegister ();

  ajDebug ("=g= plxswin ('%s') [argv[0]]\n", argv[0]);
  plxswin(argv[0]);
  
  return ajAcdInit (pgm, argc, argv);

}

/* @func ajGraphInitP ******************************************************
**
** Initialises the graphics then everything else. Reads an ACD
** (AJAX Command Definition) file, 
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
** Must be called in each EMBOSS program first.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [int] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @param [r] package [char*] Package name, used to find the ACD file
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/

AjStatus ajGraphInitP (char *pgm, int argc, char *argv[], char *package) {

  ajNamInit("emboss");

  GraphRegister ();

  ajDebug ("=g= plxswin ('%s') [argv[0]]\n", argv[0]);
  plxswin(argv[0]);

  return ajAcdInitP (pgm, argc, argv, package);

}

/* @funcstatic GraphRegister **************************************************
**
** Register the graphics calls
**
** @return [void]
** @@
******************************************************************************/

static void GraphRegister (void) {

  callRegister("ajGraphNew",(CallFunc)ajGraphNew);
  callRegister("ajGraphSet",(CallFunc)GraphSetarg);
  callRegister("ajGraphxySet",(CallFunc)GraphxySetarg);
  callRegister("ajGraphDumpDevices",(CallFunc)ajGraphDumpDevices);
  callRegister("ajGraphListDevices",(CallFunc)GraphListDevicesarg);
  callRegister("ajGraphxyTitle",(CallFunc)GraphxyTitlearg);
  callRegister("ajGraphxySubtitle",(CallFunc)GraphxySubtitlearg);
  callRegister("ajGraphxyXtitle",(CallFunc)GraphxyXtitlearg);
  callRegister("ajGraphxyYtitle",(CallFunc)GraphxyYtitlearg);
  callRegister("ajGraphTrace",(CallFunc)GraphTracearg);
  callRegister("ajGraphxyNewI",(CallFunc)GraphxyNewIarg);
  callRegister("ajGraphxySetOutputFile",(CallFunc)GraphxySetOutarg);

  return;
}

/* @func ajGraphPrintType **************************************************
**
** Print graph types
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/
void ajGraphPrintType(AjPFile outf, AjBool full) {

  GraphPType gt;
  int i;

  ajFmtPrintF (outf, "\n");
  ajFmtPrintF (outf, "# Graphics Devices\n");
  ajFmtPrintF (outf, "# Name         Device       Extension\n");
  ajFmtPrintF (outf, "GraphType {\n");
  for (i=0; graphType[i].Name; i++) {
    gt = &graphType[i];
    ajFmtPrintF (outf, "  %-12s", gt->Name);
    ajFmtPrintF (outf, " %-12s", gt->Device);
    ajFmtPrintF (outf, " %s", gt->ext);
    ajFmtPrintF (outf, "\n");
  }
  ajFmtPrintF (outf, "}\n");

  return;
}


/* @funcstatic GraphDistPts *****************************************************
**
** Compute the distance between 2 points in user coordinates.
**
** @param [r] x1 [float] x coord of point 1
** @param [r] y1 [float] y coord of point 1
** @param [r] x2 [float] x coord of point 2
** @param [r] y2 [float] y coord of point 2
**
** @return [float] The distance between the 2 points in user coordinates
** @@
******************************************************************************/

static float GraphDistPts(float x1, float y1, float x2, float y2){
  PLFLT diag;

  diag = sqrt( (x2-x1) * (x2-x1) + (y2-y1) * (y2-y1) );

  return diag;
}

/* @func ajGraphTextLength *************************************************
**
** Compute the length of a string in user coordinates.
**
** @param [r] x1 [PLFLT] Start of text box on x axis
** @param [r] y1 [PLFLT] Start of text box on y axis
** @param [r] x2 [PLFLT] End of text box on x axis
** @param [r] y2 [PLFLT] End of text box on y axis
** @param [r] text [char*] Text
**
** @return [PLFLT] The length of the string in user coordinates.
** @@
******************************************************************************/

PLFLT ajGraphTextLength(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2, char *text){
  return GraphTextLength(x1, y1, x2, y2, text);
}

/* @func ajGraphTextHeight **************************************************
**
** Compute the height of a character in user coordinates.
**
** @param [r] x1 [PLFLT] Start of text box on x axis
** @param [r] x2 [PLFLT] End of text box on x axis
** @param [r] y1 [PLFLT] Start of text box on y axis
** @param [r] y2 [PLFLT] End of text box on y axis
**
** @return [PLFLT] The height of the character in user coordinates.
** @@
******************************************************************************/
PLFLT ajGraphTextHeight(PLFLT x1, PLFLT x2, PLFLT y1, PLFLT y2){
  return GraphTextHeight(x1, y1, x2, y2);
}

/* @func ajGraphDistPts *****************************************************
**
** Compute the distance between 2 points in user coordinates.
**
** @param [r] x1 [PLFLT] x coord of point 1
** @param [r] y1 [PLFLT] y coord of point 1
** @param [r] x2 [PLFLT] x coord of point 2
** @param [r] y2 [PLFLT] y coord of point 2
**
** @return [PLFLT] The distance between the 2 points in user coordinates.
** @@
******************************************************************************/
PLFLT ajGraphDistPts(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2){
  return GraphDistPts(x1, y1, x2, y2);
}


/* @func ajGraphSetDefCharSize*********************************************
**
** Set the default character size in mm.
**
** @param [r] size [float]  character size in mm.
** @return [float] the previous character size in mm. 
** @@
******************************************************************************/
float ajGraphSetDefCharSize (float size) {
  float oldsize, oldscale;

  ajGraphGetCharSize(&oldsize, &oldscale);
  GraphDefCharSize((PLFLT)size);

  return oldsize;
}
/* @func ajGraphFitTextOnLine **********************************************
**
** Computes the character size (in mm) needed to write a text string with specified 
** height and length (in user coord). The length of the string is the distance between 
** (x1,y1) and (x2,y2); its height is TextHeight. 
** If the default size is too large, characters are shrunk. If it is too small, 
** characters are enlarged.
**
** @param [r] x1 [PLFLT] x1 coor.
** @param [r] y1 [PLFLT] y1 coor.
** @param [r] x2 [PLFLT] x2 coor.
** @param [r] y2 [PLFLT] y2 coor.
** @param [r] text [char*] The text to be displayed.
** @param [r] TextHeight [PLFLT] The height of the text (in user coord).
** @return [PLFLT] The character size (in mm) that fits the specified height and length.
**
** @@ 
************************************************************************/
PLFLT ajGraphFitTextOnLine(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2, char *text, 
			   PLFLT TextHeight){
  
  PLFLT i, stringHeight, stringLength, distpts, oldcharsize, charsize;
  
  /* adjust character height */
  stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
  oldcharsize = ajGraphSetDefCharSize(0.0);
  
  if( stringHeight<TextHeight ) {
    for(i=oldcharsize; i>0.0; i+=0.1) {
      ajGraphSetDefCharSize(i);
      stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
      if( stringHeight>TextHeight ) {
        i-=0.1;
        break;
      }
    }
  }
  else {
    for(i=oldcharsize; i>0.0; i-=0.1) {
      ajGraphSetDefCharSize(i);
      stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
      if( stringHeight<TextHeight ) break;
    }
  }
  charsize = i;
  ajGraphSetDefCharSize(charsize);
  
  stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
  oldcharsize = ajGraphSetDefCharSize(0.0);
  
  if( stringHeight<TextHeight ) {
    for(i=oldcharsize; i>0.0; i+=0.1) {
      ajGraphSetDefCharSize(i);
      stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
      if( stringHeight>TextHeight ) {
        i-=0.1;
        break;
      }
    }
  }
  else {
    for(i=oldcharsize; i>0.0; i-=0.1) {
      ajGraphSetDefCharSize(i);
      stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
      if( stringHeight<TextHeight ) break;
    }
  }
  charsize = i;
  ajGraphSetDefCharSize(charsize);
  
  stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
  oldcharsize = ajGraphSetDefCharSize(0.0);
  
  if( stringHeight<TextHeight ) {
    for(i=oldcharsize; i>0.0; i+=0.1) {
      ajGraphSetDefCharSize(i);
      stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
      if( stringHeight>TextHeight ) {
        i-=0.1;
        break;
      }
    }
  }
  else {
    for(i=oldcharsize; i>0.0; i-=0.1) {
      ajGraphSetDefCharSize(i);
      stringHeight = ajGraphTextHeight(x1, y1, x2, y2);
      if( stringHeight<TextHeight ) break;
    }
  }
  charsize = i;
  ajGraphSetDefCharSize(charsize);
  
  /* adjust character width */
  distpts = ajGraphDistPts(x1, y1, x2, y2);
  stringLength = ajGraphTextLength(x1, y1, x2, y2, text);
  
  if( stringLength<distpts ) {
    for(i=charsize; i>0.0; i+=0.1) {
      ajGraphSetDefCharSize(i);
      stringLength = ajGraphTextLength(x1, y1, x2, y2, text);
      if( stringLength>distpts ) {
        i-=0.1;
        break;
      }
    }
  }
  else {
    for(i=charsize; i>0.0; i-=0.1) {
      ajGraphSetDefCharSize(i);
      stringLength = ajGraphTextLength(x1, y1, x2, y2, text);
      if( stringLength<distpts ) break;
    }
  }
  charsize = i;
  
  ajGraphSetDefCharSize(oldcharsize);
  
  return charsize;
}

/* @func ajGraphPartCircle **********************************************
**
** Draw a portion of a circle (an arc).
**
** @param  [r] xcentre [PLFLT] x coor for centre.
** @param  [r] ycentre [PLFLT] y coor for centre.
** @param  [r] Radius  [PLFLT] radius of the circle.
** @param  [r] StartAngle [PLFLT] angle of the start of the arc.
** @param  [r] EndAngle [PLFLT] angle of the end of the arc.
** @return [void]
** @@
**
** NOTE: Due to x and y not the same length this produces an oval!!
**       This will have to do for now. But i am aware that the code
**       is slow and not quite right.
*********************************************************************/
void ajGraphPartCircle(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT StartAngle, PLFLT EndAngle)
{
  PLFLT angle;
  int i;
  PLFLT x[361], y[361];
  int numofpoints;


  x[0]=xcentre + ( Radius*(float)cos(ajDegToRad(StartAngle)) );
  y[0]=ycentre + ( Radius*(float)sin(ajDegToRad(StartAngle)) );
  for(i=1, angle=StartAngle+1; angle<EndAngle; angle++, i++) {
    x[i]=xcentre + ( Radius*(float)cos(ajDegToRad(angle)) );
    y[i]=ycentre + ( Radius*(float)sin(ajDegToRad(angle)) );
  }
  x[i]=xcentre + ( Radius*(float)cos(ajDegToRad(EndAngle)) );
  y[i]=ycentre + ( Radius*(float)sin(ajDegToRad(EndAngle)) );
  numofpoints = i+1;
  
  GraphDrawLines(numofpoints, x, y);
}

/* @func ajComputeCoord ********************************************
**
** compute the coordinates of a point on a circle knowing the angle.
**
** @param  [r] xcentre [PLFLT] x coor for centre.
** @param  [r] ycentre [PLFLT] y coor for centre.
** @param  [r] Radius  [PLFLT] Radius of the circle.
** @param  [r] Angle [PLFLT] angle at which the point is.
** @return [PLFLT*] The x and y coordinates of the point.
** @@
*********************************************************************/
PLFLT* ajComputeCoord(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT Angle)
{
  PLFLT *xy;

  xy = (float *)AJALLOC( 2*sizeof(float) );
  xy[0] = xcentre + ( Radius*(float)cos(ajDegToRad(Angle)) );
  xy[1] = ycentre + ( Radius*(float)sin(ajDegToRad(Angle)) );
  
  return xy;
}

/* @funcstatic GraphDrawTextOnCurve *************************************
**
** Draw text along a curve (i.e., an arc of a circle).
** The text is written character by character.
**
** @param  [r] xcentre [PLFLT] x coor for centre.
** @param  [r] ycentre [PLFLT] y coor for centre.
** @param  [r] Radius  [PLFLT] Radius of the circle.
** @param  [r] Angle [PLFLT] angle at which a particular character will be written (in deg).
** @param  [r] pos [PLFLT] index for incrementing the angle for the next character in the text.
** @param  [r] Text [char*] The text to be displayed.
** @param  [r] just [PLFLT] justification of the string. (0=left,1=right,0.5=middle etc)
** @return [void]
** @@
*********************************************************************/
static void GraphDrawTextOnCurve(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT Angle, PLFLT pos, 
char *Text, PLFLT just){
int i, numchar;
PLFLT *xy1, *xy2;
char *text;

text = (char *)AJALLOC( 1000*sizeof(char) );
xy1 = (float *)AJALLOC( 2*sizeof(float) );
xy2 = (float *)AJALLOC( 2*sizeof(float) );

numchar = strlen(Text);
for(i=0; i<numchar; i++) {
  xy1 = ajComputeCoord(xcentre, ycentre, Radius, Angle+pos*i+0.5*pos);
  xy2 = ajComputeCoord(xcentre, ycentre, Radius, Angle+pos*i+1.5*pos);
  strcpy(text, Text);
  text[i+1] = '\0';
  ajGraphDrawTextOnLine(xy1[0], xy1[1], xy2[0], xy2[1], &text[i], just);
}
AJFREE(text);
AJFREE(xy1); 
AJFREE(xy2); 
}

/* @func ajGraphDrawTextOnCurve *************************************
**
** Draw text along a curve (i.e., an arc of a circle).
** The text is written character by character, forwards or backwards depending o
n the angle.
**
** @param  [r] xcentre [PLFLT] x coor for centre.
** @param  [r] ycentre [PLFLT] y coor for centre.
** @param  [r] Radius  [PLFLT] radius of the circle.
** @param  [r] StartAngle [PLFLT] angle of the start of the arc (in deg).
** @param  [r] EndAngle [PLFLT] angle of the end of the arc (in deg).
** @param  [r] Text [char*] The text to be displayed.
** @param  [r] just [PLFLT] justification of the string. (0=left,1=right,0.5=midd
le etc)
** @return [void]
** @@
*********************************************************************/
void ajGraphDrawTextOnCurve(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT StartAngle, 
PLFLT EndAngle, char *Text, PLFLT just)
{
  int numchar = strlen(Text);
  PLFLT pos = (EndAngle-StartAngle)/numchar;
  
  if( ((StartAngle>180.0 && StartAngle<=360.0) && 
       (EndAngle>180.0 && EndAngle<=360.0)) || ((StartAngle>540.0 && StartAngle<=720.0) 
						&& (EndAngle>540.0 && EndAngle<=720.0)) ) 
    GraphDrawTextOnCurve(xcentre, ycentre, Radius, StartAngle, +1*pos, Text, just);
  else GraphDrawTextOnCurve(xcentre, ycentre, Radius, EndAngle, -1*pos, Text, just);
}

/* @func ajGraphRectangleOnCurve ***********************************
**
** Draw a rectangle along a curve with the current pen colour/style.
**
** @param  [r] xcentre [PLFLT] x coor for centre.
** @param  [r] ycentre [PLFLT] y coor for centre.
** @param  [r] Radius  [PLFLT] radius of the circle.
** @param  [r] BoxHeight [PLFLT] The height of the rectangle in user coordinates.
** @param  [r] StartAngle [PLFLT] angle of the start of the rectangle.
** @param  [r] EndAngle [PLFLT] angle of the end of the rectangle.
** @return [void]
** @@
*********************************************************************/
void ajGraphRectangleOnCurve(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT BoxHeight, PLFLT StartAngle, PLFLT EndAngle)
{
  PLFLT *xy1, *xy2;
  PLFLT r1Blocks = Radius;
  PLFLT r2Blocks = r1Blocks+BoxHeight;
  
  ajGraphPartCircle(xcentre, ycentre, r1Blocks, StartAngle, EndAngle);
  ajGraphPartCircle(xcentre, ycentre, r2Blocks, StartAngle, EndAngle);
  
  xy1 = (float *)AJALLOC( 2*sizeof(float) );
  xy2 = (float *)AJALLOC( 2*sizeof(float) );
  
  xy1 = ajComputeCoord(xcentre, ycentre, r1Blocks, StartAngle);
  xy2 = ajComputeCoord(xcentre, ycentre, r2Blocks, StartAngle);
  ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
  xy1 = ajComputeCoord(xcentre, ycentre, r1Blocks, EndAngle);
  xy2 = ajComputeCoord(xcentre, ycentre, r2Blocks, EndAngle);
  ajGraphDrawLine( xy1[0], xy1[1], xy2[0], xy2[1] );
}




/* @func ajGraphUnused **************************************************
**
** Unused functions to avoid compiler warnings
**
** @return [void]
** @@
******************************************************************************/
void ajGraphUnused(void)
{
    float f=0.0;
    int i=0;
    AjPGraph graphs=NULL;
    AjPGraphData gd=NULL;
    
    GraphObjPrint(graphs);
    GraphDataObjPrint(gd);    
    GraphCheckPoints (0, &f, &f);
    GraphCheckFlags (0);
    GraphArrayGapsI (0, &i, &i);
}

#endif

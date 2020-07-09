#ifdef __cplusplus
extern "C"
{
#endif

/* @source ajGraph.h 
**
** General Plot/Printing routines.
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

/* temporary include file for graphics to define structure(s) */

#ifndef ajgraph_h
#define ajgraph_h

#include "plplot.h"
#include "ajgraphstruct.h"
#include "ajdefine.h"
#include "ajstr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ajGraphDrawLine ajGraphLine
#define ajGraphDrawTextOnLine ajGraphTextLine

#define GRAPH_XGAP      60
#define GRAPH_YGAP      60
#define GRAPH_TITLE_GAP 60

#define AJGRAPH_X_BOTTOM   0x0001  /* print xaxis bottom line */ 
#define AJGRAPH_Y_LEFT     0x0002  /* print yaxis left line */
#define AJGRAPH_X_TOP      0x0004  /* print xaxis top line */
#define AJGRAPH_Y_RIGHT    0x0008  /* printf y axis on the right*/
#define AJGRAPH_X_TICK     0x0010  /* add tick marks for x axis (bottom) */
#define AJGRAPH_Y_TICK     0x0020  /* add tick marks dor y axis (left) */
#define AJGRAPH_X_LABEL    0x0040  /* add x axis label */
#define AJGRAPH_Y_LABEL    0x0080  /* add y axis label */
#define AJGRAPH_TITLE      0x0100  /* add title */
#define AJGRAPH_JOINPOINTS 0x0200  /* join the point data by a line a line */
#define AJGRAPH_OVERLAP    0x0400  /* write plots on top of each other */
#define AJGRAPH_Y_NUMLABEL_LEFT   0x0800
#define AJGRAPH_Y_INVERT_TICK     0x1000
#define AJGRAPH_Y_GRID            0x2000
#define AJGRAPH_X_NUMLABEL_ABOVE  0x4000
#define AJGRAPH_X_INVERT_TICK     0x8000
#define AJGRAPH_X_GRID            0x10000
#define AJGRAPH_CIRCLEPOINTS      0x20000
#define AJGRAPH_SUBTITLE          0x40000
#define AJGRAPH_GAPS              0x80000

#define GRAPH_XY (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_X_TOP + \
		  AJGRAPH_Y_RIGHT + AJGRAPH_X_TICK + AJGRAPH_Y_TICK + \
		  AJGRAPH_Y_LABEL + AJGRAPH_JOINPOINTS + AJGRAPH_X_LABEL + \
		  AJGRAPH_TITLE + AJGRAPH_SUBTITLE + AJGRAPH_OVERLAP)

#define GRAPH_XY_MAIN (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT + \
		       AJGRAPH_X_TOP + AJGRAPH_Y_TICK + AJGRAPH_X_LABEL + \
		       AJGRAPH_Y_LABEL + AJGRAPH_JOINPOINTS + AJGRAPH_TITLE + \
		       AJGRAPH_SUBTITLE + AJGRAPH_OVERLAP)
  

void          ajGraphBox (PLFLT x0, PLFLT y0,PLFLT size);
void          ajGraphBoxFill (PLFLT x0, PLFLT y0, PLFLT size);
int           ajGraphCheckColour (AjPStr colour);
void          ajGraphCircle (PLFLT xcentre, PLFLT ycentre, float radius);
void          ajGraphClose (void);
void          ajGraphCloseWin (void);
void          ajGraphColourBack (void);
void          ajGraphColourFore (void);
void          ajGraphDataDel(AjPGraphData *thys);
void          ajGraphDataObjAddLine (AjPGraphData graphs, float x1, float y1,
				     float x2, float y2, int colour);
void          ajGraphDataObjAddRect (AjPGraphData graphs,
				     float x1, float y1,
				     float x2, float y2,
				     int colour, int fill);
void          ajGraphDataObjAddText (AjPGraphData graphs, float x1, float y1,
				    int colour, char *text);
void          ajGraphDataxySetMaxMin (AjPGraphData graphdata, float xmin, 
				      float xmax, float ymin, float ymax);
void          ajGraphDataxyMaxMin(float *array, int npoints, float *min,
				  float *max);
		  
void          ajGraphDataxySetMaxima(AjPGraphData graphdata, float xmin,
				     float xmax, float ymin, float ymax);
void          ajGraphDataxySetTypeC(AjPGraphData graphdata, char* type);
		  
void          ajGraphDia (PLFLT x0, PLFLT y0, PLFLT size);
void          ajGraphDiaFill (PLFLT x0, PLFLT y0, PLFLT size);
void          ajGraphDots (PLFLT *x1,PLFLT *y1, int numofdots);
void          ajGraphDumpDevices (void);
int*          ajGraphGetBaseColour (void);
void          ajGraphGetCharSize (float *defheight, float *currentheight);
int           ajGraphGetColour(void);
void          ajGraphGetOut (float *xp,float *yp,int
			     *xleng,int *yleng,int *xoff,int *yoff);
void          ajGraphGetOutputDeviceParams(float *xp,float *yp,
					   int *xleng,int *yleng,int *xoff,
					   int *yoff);
void          ajGraphHoriBars (int numofpoints, PLFLT *y,
				   PLFLT *xmin, PLFLT *xmax);
AjStatus      ajGraphInit (char *pgm, int argc, char *argv[]);
AjStatus      ajGraphInitP (char *pgm, int argc, char *argv[], char *package);
void          ajGraphInitSeq (AjPGraph thys, AjPSeq seq);
void          ajGraphLabel (char *x, char *y, char *title, char *subtitle);
void          ajGraphLabelYRight (char *text);
void          ajGraphLine (PLFLT x1,PLFLT y1,PLFLT x2,PLFLT y2);
void          ajGraphLines (PLFLT *x1,PLFLT *y1,PLFLT *x2,PLFLT *y2,
			       int numoflines);
AjPGraph      ajGraphNew (void);
void          ajGraphNewPage (AjBool resetdefaults);
void          ajGraphObjAddLine (AjPGraph graphs, float x1, float y1,
				 float x2, float y2, int colour);
void          ajGraphObjAddRect (AjPGraph graphs, float x1, float y1,
				 float x2, float y2, int colour, int fill);
void          ajGraphObjAddText (AjPGraph graphs, float x1, float y1,
				 int colour, char *text);
void          ajGraphObjDel(AjPGraph *thys);
void          ajGraphDataObjDel(AjPGraphData *thys);
void          ajGraphOpen (AjPGraph thys, PLFLT xmin, PLFLT xmax,
			   PLFLT ymin, PLFLT ymax, int flags);
void          ajGraphOpenPlot (AjPGraph thys, int numofsets);
void          ajGraphOpenWin  (AjPGraph thys, float xmin, float xmax,
				 float ymin, float ymax);
void          ajGraphPlenv (float xmin, float xmax, float ymin, float ymax,
			    int flags);
void          ajGraphPoly (int n, PLFLT *x, PLFLT *y);
void          ajGraphPolyFill (int n, PLFLT *x, PLFLT *y);
void          ajGraphPrintType(AjPFile outf, AjBool full);
void          ajGraphRect (PLFLT x0, PLFLT y0,PLFLT x1, PLFLT y1) ;
void          ajGraphRectFill (PLFLT x0, PLFLT y0, PLFLT x1, PLFLT y1) ;
AjBool        ajGraphSet (AjPGraph thys, AjPStr type);
void          ajGraphSetDevice(AjPGraph thys);
float         ajGraphSetCharSize (float size);
void          ajGraphSetBackBlack (void);
void          ajGraphSetBackWhite (void);
void          ajGraphSetPenWidth(float width);
int           ajGraphSetFillPat (int style);
int           ajGraphSetFore (int colour);
int           ajGraphSetLineStyle (int style);
void          ajGraphSetMulti (AjPGraph thys, int numsets);
void          ajGraphSetName (AjPGraph thys);
void          ajGraphSetOri(int ori);
void          ajGraphSymbols( int numofdots, PLFLT *x1,PLFLT *y1, int symbol);
void          ajGraphText (PLFLT x1, PLFLT y1, char *text, PLFLT just);
void          ajGraphTextEnd (PLFLT x1, PLFLT y1, char *text);
void          ajGraphTextMid (PLFLT x1, PLFLT y1, char *text);
void          ajGraphTextLine (PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2,
				     char *text, PLFLT just);
void          ajGraphTextStart (PLFLT x1, PLFLT y1, char *text);
void          ajGraphTrace (AjPGraph thys);
void          ajGraphTraceInt (AjPGraph thys, FILE* outf);
void          ajGraphTri (PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2,
			       PLFLT x3, PLFLT y3);
void          ajGraphTriFill (PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2,
				   PLFLT x3, PLFLT y3);
void          ajGraphUnused(void);
void          ajGraphVertBars (int numofpoints, PLFLT *x,
				    PLFLT *ymin, PLFLT *ymax);
void          ajGraphxyAddDataCalcPtr (AjPGraphData graph, int numofpoints,
				       float start, float incr, float* y);
void          ajGraphxyAddDataPtrPtr (AjPGraphData graph, float *x,float *y);
int           ajGraphxyAddGraph (AjPGraph mult, AjPGraphData graphdata);
int           ajGraphxyReplaceGraph(AjPGraph mult, AjPGraphData graphdata);
AjPGraphData  ajGraphxyDataNew (void);
AjPGraphData  ajGraphxyDataNewI (int numsets);
void          ajGraphxyDataSetSubtitle  (AjPGraphData graph,AjPStr title);
void          ajGraphxyDataSetSubtitleC (AjPGraphData graph,char *title);
void          ajGraphxyDataSetTitle  (AjPGraphData graph,AjPStr title);
void          ajGraphxyDataSetTitleC (AjPGraphData graph,char *title);
void          ajGraphxyDataSetXtitle  (AjPGraphData graphdata,
					   AjPStr title);
void          ajGraphxyDataSetXtitleC (AjPGraphData graphdata,
					   char *title);
void          ajGraphxyDataSetYtitle  (AjPGraphData graphdata,
					   AjPStr title);
void          ajGraphxyDataSetYtitleC (AjPGraphData graphdata,
					   char *title);
void          ajGraphxyDel (AjPGraph mult);
void          ajGraphxyDisplay (AjPGraph graphs, AjBool closeit );
AjPGraph      ajGraphxyInitGraphCalcPtr (int numofpoints, float start,
					 float incr, float *y);
AjPGraph      ajGraphxyNewI (int numofpoints);
void          ajGraphxyPrint (AjPGraph graphs) ;
void          ajGraphxyCheckMaxMin (AjPGraph graphs);
AjBool        ajGraphxySet (AjPGraph thys, AjPStr type);
void          ajGraphxySetCirclePoints (AjPGraph graphs, AjBool set);
void          ajGraphxySetColour (AjPGraphData graph, int colour);
void          ajGraphxySetFlag (AjPGraph graphs,int flag, AjBool istrue);
void          ajGraphxySetGaps(AjPGraph graphs, AjBool overlap);
void          ajGraphxySetJoinPoints (AjPGraph graphs, AjBool set);
void          ajGraphxySetLineType (AjPGraphData graph, int type);
void          ajGraphxySetMaxMin(AjPGraph graphs,float xmin,float xmax,
				 float ymin,float ymax);
void          ajGraphxySetOut (AjPGraph mult, AjPStr txt);
void          ajGraphxySetOutC (AjPGraph mult, char* txt);
void          ajGraphxySetOverLap (AjPGraph graphs, AjBool overlap);
void          ajGraphxySetSubtitleDo (AjPGraph graphs, AjBool set);
void          ajGraphxySetTitleDo (AjPGraph graphs, AjBool set);
void          ajGraphxySetXBottom (AjPGraph graphs, AjBool set);
void          ajGraphxySetXLabelTop (AjPGraph graphs, AjBool set);
void          ajGraphxySetXTick (AjPGraph graphs, AjBool set);
void          ajGraphxySetXTop (AjPGraph graphs, AjBool set);
void          ajGraphxySetXEnd (AjPGraph graphs, float val);
void          ajGraphxySetXGrid (AjPGraph graphs, AjBool set);
void          ajGraphxySetXInvTicks (AjPGraph graphs, AjBool set);
void          ajGraphxySetXLabel (AjPGraph graphs, AjBool set);
void          ajGraphxySetXRangeII (AjPGraph thys, int start, int end);
void          ajGraphxySetXStart (AjPGraph graphs, float val);
void          ajGraphxySetYLabelLeft (AjPGraph graphs, AjBool set);
void          ajGraphxySetYLeft (AjPGraph graphs, AjBool set);
void          ajGraphxySetYRight (AjPGraph graphs, AjBool set);
void          ajGraphxySetYTick (AjPGraph graphs, AjBool set);
void          ajGraphxySetYEnd (AjPGraph graphs, float val);
void          ajGraphxySetYGrid (AjPGraph graphs, AjBool set);
void          ajGraphxySetYInvTicks (AjPGraph graphs, AjBool set);
void          ajGraphxySetYLabel (AjPGraph graphs, AjBool set);
void          ajGraphxySetYRangeII (AjPGraph thys, int start, int end);
void          ajGraphxySetYStart (AjPGraph graphs, float val);
void          ajGraphxySubtitle  (AjPGraph graphs, AjPStr title);
void          ajGraphxySubtitleC (AjPGraph graphs, char *title);
void          ajGraphxyTitle     (AjPGraph graphs, AjPStr title);
void          ajGraphxyTitleC    (AjPGraph graphs, char *title);
void          ajGraphxyTrace      (AjPGraph thys);
void          ajGraphxyXtitle  (AjPGraph graphs, AjPStr title);
void          ajGraphxyXtitleC (AjPGraph graphs, char *title);
void          ajGraphxyYtitle  (AjPGraph graphs, AjPStr title);
void          ajGraphxyYtitleC (AjPGraph graphs, char *title);
void          ajGraphxySetGaps(AjPGraph graphs, AjBool overlap);
void          ajGraphPartCircle(PLFLT xcentre, PLFLT ycentre, float radius, PLFLT StartAngle, PLFLT EndAngle);
PLFLT         *ajComputeCoord(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT Angle);
void          ajGraphDrawTextOnCurve(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT StartAngle, 
				     PLFLT EndAngle, char *Text, PLFLT just);
void          ajGraphRectangleOnCurve(PLFLT xcentre, PLFLT ycentre, PLFLT Radius, PLFLT BoxHeight, 
				      PLFLT StartAngle, PLFLT EndAngle);
PLFLT         ajGraphTextLength(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2, char *text);
PLFLT         ajGraphTextHeight(PLFLT x1, PLFLT x2, PLFLT y1, PLFLT y2);
PLFLT         ajGraphDistPts(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2);
PLFLT          ajGraphFitTextOnLine(PLFLT x1, PLFLT y1, PLFLT x2, PLFLT y2, char *text, PLFLT TextHeight);
float         ajGraphSetCharSize (float size);
float         ajGraphSetDefCharSize (float size);

#endif /* ajgraph_h */

#ifdef __cplusplus
}
#endif

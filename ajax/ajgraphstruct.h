#ifndef ajgraphstruct_h
#define ajgraphstruct_h

#define MAX_STRING 180

#include "ajgraphxml.h"

/* @data AjPGraphPlpObj *******************************************************
**
** AJAX data structure for graph objects, contained as a substructure
** in AjPGraphPlpData
**
** @attr type [ajint] Object type in AjEGraphObjectTypes
** @attr colour [ajint] See AjEGraphColour for plplot colours
** @attr text [AjPStr] Text to plot
** @attr x1 [float] x start
** @attr x2 [float] x end
** @attr y1 [float] y start
** @attr y2 [float] y end
** @attr next [struct AjSGraphPlpObj*] link to next object in the list
** @@
******************************************************************************/

typedef struct AjSGraphPlpObj {
  ajint type;
  ajint colour;
  AjPStr text;
  float x1;
  float x2;
  float y1;
  float y2;
  struct AjSGraphPlpObj *next;
} AjOGraphPlpObj;
#define AjPGraphPlpObj AjOGraphPlpObj*

/* @data AjPGraphPlpData ******************************************************
**
** Graph data object. Substructure of AjPGraphPlp.
**
** @attr x [float*] x coordinates
** @attr xcalc [AjBool] if x calculated then delete after
** @attr y [float*] y coordinates
** @attr ycalc [AjBool] as with x. So we do not delete data if it was
**                      passed as a ptr
** @attr numofpoints [ajint] Number of points in x and y
** @attr numofobjects [ajint] Number of graph objects starting at Obj
** @attr minX [float] Lowest x value
** @attr maxX [float] Highest x value
** @attr minY [float] Lowest y value
** @attr maxY [float] Highest y value
** @attr tminX [float] First x value to plot
** @attr tmaxX [float] Last x value to plot
** @attr tminY [float] First y value to plot
** @attr tmaxY [float] Last y value to plot
** @attr title [AjPStr] Plot title
** @attr subtitle [AjPStr] Plot subtitle
** @attr xaxis [AjPStr] Plot x axis title
** @attr yaxis [AjPStr] Plot y axis title
** @attr gtype [AjPStr] Graph type: 2D, Tick etc
** @attr colour [ajint] See AjEGraphColour for plplot colours
** @attr lineType [ajint] Line type for plplot
** @attr Obj [AjPGraphPlpObj] First graph object - links to rest
** @@
******************************************************************************/

typedef struct AjSGraphPlpData {
  float *x;
  AjBool xcalc;
  float *y;
  AjBool ycalc;
  ajint numofpoints;
  ajint numofobjects;
  float minX;
  float maxX;
  float minY;
  float maxY;
  float tminX;
  float tmaxX;
  float tminY;
  float tmaxY;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  AjPStr gtype;
  ajint colour;
  ajint lineType;
  AjPGraphPlpObj Obj;
} AjOGraphPlpData;
#define AjPGraphPlpData AjOGraphPlpData*

/* @data AjPGraphPlp **********************************************************
**
** Graph plplot object.
**
** @attr numofgraphs [ajint] Number of graphs in graphs
** @attr numofobjects [ajint] Number of objects in Obj
** @attr numofgraphsmax [ajint] Maximum number of graphs expected
** @attr flags [ajint] over rides the EmbGraphData flags
** @attr minX [float] Lowest x value for all graphs
** @attr maxX [float] Highest x value for all graphs
** @attr minY [float] Lowest y value for all graphs
** @attr maxY [float] Highest y value for all graphs
** @attr xstart [float] First x value to plot
** @attr xend [float] Last x value to plot
** @attr ystart [float] First y value to plot
** @attr yend [float] Last y value to plot
** @attr ready [AjBool] Set by plplot device init
** @attr minmaxcalc [AjBool] Set true when (xy)start/end are set
** @attr isdata [AjBool] True for a data type graph
** @attr displaytype [ajint] Displaytype index to graphType
** @attr title [AjPStr] Plot title
** @attr subtitle [AjPStr] Plot subtitle
** @attr xaxis [AjPStr] Plot x axis title
** @attr yaxis [AjPStr] Plot y axis title
** @attr outputfile [AjPStr] Output filename
** @attr graphs [AjPGraphPlpData*] XY Data to plot for Graph(s)
** @attr Obj [AjPGraphPlpObj] Objects to plot for single graph
** @@
******************************************************************************/

typedef struct AjSGraphPlp {
  ajint numofgraphs;
  ajint numofobjects;
  ajint numofgraphsmax;
  ajint flags;
  float minX;
  float maxX;
  float minY;
  float maxY;
  float xstart;
  float xend;
  float ystart;
  float yend;
  AjBool ready;
  AjBool minmaxcalc;
  AjBool isdata;
  ajint displaytype;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  AjPStr outputfile;
  AjPGraphPlpData *graphs;
  AjPGraphPlpObj Obj;
} AjOGraphPlp;
#define AjPGraphPlp AjOGraphPlp*

/* @data AjPGraph *************************************************************
**
** Graph object.
**
** @attr numsets [ajint] Number of sets in a multiple graph
** @attr plplot [AjPGraphPlp] PlPlot graph object
** @attr xml [AjPGraphXml] XML graph object
** @@
******************************************************************************/

typedef struct AjSGraph {
    ajint numsets;
    AjPGraphPlp plplot;
    AjPGraphXml xml;
} AjOGraph;
#define AjPGraph AjOGraph*

enum AjEGraphColours {BLACK, RED, YELLOW, GREEN, AQUAMARINE,
		      PINK, WHEAT, GREY, BROWN, BLUE, BLUEVIOLET,
		      CYAN, TURQUOISE, MAGENTA, SALMON, WHITE};


#define NCOLS 16

#endif /* ajgraphstruct_h */

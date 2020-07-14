#ifndef ajgraphstruct_h
#define ajgraphstruct_h

#define MAX_STRING 180

/* @data AjPGraphObj **********************************************************
**
** AJAX data structure for graph objects, contained as a substructure
** in AjPGraphData
**
******************************************************************************/

typedef struct AjSGraphObj {
  ajint type;			/* Object type in AjEGraphObjectTypes */
  ajint colour;			/* See AjEGraphColour for plplot colours */
  AjPStr text;			/* Text to plot */
  float x1;			/* x start */
  float x2;			/* x end */
  float y1;			/* y start */
  float y2;			/* y end */
  struct AjSGraphObj *next;	/* link to next object in the list */
} AjOGraphObj, *AjPGraphObj;

/* @data AjPGraphData *********************************************************
**
** Graph data object. Substructure of AjPGraph.
**
** @@
******************************************************************************/

typedef struct AjSGraphData {
  float *x;			/* x coordinates */
  AjBool xcalc;			/* if x calculated then delete after */
  float *y;			/* y coordinates */
  AjBool ycalc;			/* as with x. So you do not delete data */
				/*   if it was passed as a ptr. */
  ajint numofpoints;		/* Number of points in x and y */
  ajint numofobjects;		/* Number of graph objects starting at Obj */
  float minX;			/* Lowest x value */
  float maxX;			/* Highest x value */
  float minY;			/* Lowest y value */
  float maxY;			/* Highest y value */
  float tminX;			/* First x value to plot */
  float tmaxX;			/* Last x value to plot */
  float tminY;			/* First y value to plot */
  float tmaxY;			/* Last y value to plot */
  AjPStr title;			/* Plot title */
  AjPStr subtitle;		/* Plot subtitle */
  AjPStr xaxis;			/* Plot x axis title */
  AjPStr yaxis;			/* Plot y axis title */
  AjPStr gtype;			/* 2D, Tick etc */
  ajint colour;			/* See AjEGraphColour for plplot colours */
  ajint lineType;		/* Line type for plplot */
  AjPGraphObj Obj;		/* First graph object - links to rest */
} AjOGraphData, *AjPGraphData;

/* @data AjPGraph *************************************************************
**
** Graph object.
**
** @@
******************************************************************************/

typedef struct AjSGraph {
  ajint numofgraphs;		/* Number of graphs in graphs*/
  ajint numofobjects;		/* Number of objects in Obj */
  ajint numofgraphsmax;		/* Maximum number of graphs expected */
  ajint flags;		       /* over rides the EmbGraphData flags */
  float minX;			/* Lowest x value for all graphs */
  float maxX;			/* Highest x value for all graphs */
  float minY;			/* Lowest y value for all graphs  */
  float maxY;			/* Highest y value for all graphs  */
  float xstart;			/* First x value to plot */
  float xend;			/* Last x value to plot */
  float ystart;			/* First y value to plot */
  float yend;			/* Last y value to plot */
  AjBool ready;			/* Set by plplot device init */
  AjBool minmaxcalc;		/* Set true when (xy)start/end are set */
  ajint displaytype;		/* Displaytype index to graphType */
  AjPFile file;			/* Output file */
  AjPStr title;			/* Plot title */
  AjPStr subtitle;		/* Plot subtitle */
  AjPStr xaxis;			/* Plot x axis title */
  AjPStr yaxis;			/* Plot y axis title */
  AjPStr outputfile;		/* Output filename */
  AjPGraphData *graphs;		/* XY Data to plot for Graph(s) */
  AjPGraphObj Obj;		/* Objects to plot for single graph */
} AjOGraph, *AjPGraph;

enum AjEGraphColours {BLACK, RED, YELLOW, GREEN, AQUAMARINE,
		      PINK, WHEAT, GREY, BROWN, BLUE, BLUEVIOLET,
		      CYAN, TURQUOISE, MAGENTA, SALMON, WHITE};


#define NCOLS 16

#endif /* ajgraphstruct_h */

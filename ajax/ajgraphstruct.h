#ifndef ajgraphstruct_h
#define ajgraphstruct_h

#define MAX_STRING 180

typedef struct AjSGraphObj {
  ajint type;
  ajint colour;
  AjPStr text;
  float x1,x2;
  float y1,y2;
  struct AjSGraphObj *next;
} AjOGraphObj, *AjPGraphObj;

/* @data AjPGraphData *******************************************************
**
** Graph data object. Substructure of AjPGraph. Still under development
** 
** @@ 
******************************************************************************/

typedef struct AjSGraphData {
  float *x;			/* x coords */
  AjBool xcalc;			/* if x calculated then delete after */
  float *y;			/* y coords */
  AjBool ycalc;			/* as with x. So you do not delete data */
				/*   if it was passed as a ptr. */
  ajint numofpoints;
  ajint numofobjects;
  float minX,maxX;
  float minY,maxY;
  float tminX,tmaxX;
  float tminY,tmaxY;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  AjPStr gtype;			/* 2D, Tick etc */
  ajint colour;         
  ajint lineType;
  AjPGraphObj Obj;
} AjOGraphData, *AjPGraphData;

/* @data AjPGraph *******************************************************
**
** Graph object. Still under development
**
** @@
******************************************************************************/

typedef struct AjSGraph {
  ajint numofgraphs;
  ajint numofobjects;
  ajint numofgraphsmax;
  ajint flags;           /* over rides the EmbGraphData flags */
  float minX,maxX;     /* min max over all graphs */ 
  float minY,maxY;     
  float xstart;
  float xend;   /* plot only between these points */
  float ystart;
  float yend;
  AjBool ready;
  AjBool minmaxcalc;
  ajint displaytype;
  AjPFile file;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  AjPStr outputfile;
  AjPGraphData *graphs;
  AjPGraphObj Obj;
} AjOGraph, *AjPGraph;

enum ajColours {BLACK, RED, YELLOW, GREEN, AQUAMARINE,
		PINK, WHEAT, GREY, BROWN, BLUE, BLUEVIOLET,
		CYAN, TURQUOISE, MAGENTA, SALMON, WHITE};


#define NCOLS 16

#endif /* ajgraphstruct_h */




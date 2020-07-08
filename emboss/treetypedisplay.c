/*  Last edited: Mar 23 12:27 2000 (pmr) */
#include "emboss.h"

int main(int argc, char *argv[])
{
  PLFLT x1[9]={2,1,2,1,1,2,3,3}; /* x1,y1  x2,y2  are lines */
  PLFLT y1[9]={4,2,3,1,1,3,2,2};
  PLFLT x2[9]={2,1,1,0,2,3,4,3};
  PLFLT y2[9]={3,1,2,0,0,2,2,1};
  PLFLT pt1[9]={2,1,3,4,1,3,0,2,2}; /* pt1,pt1    are pts */
  PLFLT pt2[9]={4,2,2,2,1,1,0,0,3};
  int i;
  char temp[7];
  float xmin=-1.0,xmax=5.0,ymin=-1.0,ymax=5.0;
  AjPGraph graph;

  ajGraphInit ("treetypedisplay", argc, argv);
  graph = ajAcdGetGraph("graph");

  ajGraphOpenWin(graph, xmin,xmax,ymin,ymax);
  ajGraphLines(x1,y1,x2,y2,8);
  ajGraphDots(pt1,pt2,9);
  i=0;
  for(i=0;i<8;i++){
    sprintf(temp,"line %d",i);
    ajGraphTextLine(x1[i], y1[i], x2[i], y2[i], temp, 1.0);
  }
  ajGraphCloseWin();
  ajExit();
  return 0;
}

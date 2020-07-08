/*  Last edited: Feb 21 12:51 2000 (pmr) */
/*emboss/histogramtest 3 10 10 1 20 30*/
#include "emboss.h"
#include "ajhist.h"

void ajGraphHistogram
(int numofdatapoints, int numofsets, PLFLT *data,int sidebyside, int xmin, int xmax,int bins);

int main(int argc, char *argv[])
{
  AjPHist hist=NULL;
  AjPGraph graph=NULL;
  PLFLT **data,*data2;
  int i,j,k=1;
  int sets,points;
  char temp[20];

  (void) ajGraphInit ("histogramtest", argc, argv);

  ajUser ("init done");

  graph = ajAcdGetGraphxy ("graph");
  sets = ajAcdGetInt ("sets");

  points = ajAcdGetInt ("points");

  ajUser("ajHistNewG");
  hist = ajHistNewG(sets,points, graph);

  hist->bins = ajAcdGetInt ("bins");

  /*  hist->displaytype = ajAcdGetInt ("sidebyside");*/

  hist->displaytype=HIST_ONTOP;

  hist->xmin = ajAcdGetInt ("xstart");

  hist->xmax = ajAcdGetInt ("xend");

  /*  ajUser("DEBUG: xstart = %f, xend = %f",hist->xmin,hist->xmax);*/


  ajUser("ajHistSetTitleC");
  ajHistSetTitleC(hist, "A demo of the Histogram suite");

  ajUser("ajHistSetXAxisC");
  ajHistSetXAxisC(hist, "X axis");
  
  ajUser("ajHistSetYAxisLeftC");
  ajHistSetYAxisLeftC(hist, "LEFT");
  
  ajUser("ajHistSetYAxisRightC");
  ajHistSetYAxisRightC(hist, "RIGHT");

  AJCNEW(data, sets);
  for(i=0;i<sets;i++){
    AJCNEW(data[i], points);
    data2 = data[i];
    for(j=0;j<points;j++){
      data2[j] = k++;
    }
  ajUser("ajHistSetPtrToData (%d)", i);
    ajHistSetPtrToData(hist, i, data2);
  }

  ajUser("DOING 1st");
  ajHistDisplay (hist);

  /* free all memory */
  for(i=0;i<sets;i++)
    AJFREE(data[i]);
  ajHistDelete(hist);

  /* now do again but copy the data */
  hist = ajHistNewG(sets,points, graph);

  hist->bins = ajAcdGetInt ("bins");

  /*  hist->displaytype = ajAcdGetInt ("sidebyside");*/
  hist->displaytype=HIST_SIDEBYSIDE;

  hist->xmin = ajAcdGetInt ("xstart");

  hist->xmax = ajAcdGetInt ("xend");

  ajHistSetTitleC(hist, "A demo of the Histogram suite");

  ajHistSetXAxisC(hist, "X axis");
  
  ajHistSetYAxisLeftC(hist, "LEFT");
  
  ajHistSetYAxisRightC(hist, "RIGHT");

  k=-10;
  for(i=0;i<sets;i++){
    AJCNEW(data2, points);
    for(j=0;j<points;j++){
      data2[j] = k++;
    }
    ajHistCopyData(hist, i, data2);
    AJFREE(data2);
  }
 
  /*  hist->displaytype = !hist->displaytype;*/
  for(i=0;i<sets;i++)
    ajHistSetPattern(hist, i, i);

  ajHistSetBlackandWhite(hist , AJTRUE);

  ajUser("DOING 2nd");
  ajHistDisplay (hist);

  ajUser("DOING 3rd");
  hist->displaytype=HIST_SEPARATE;

  for(i=0;i<sets;i++){
    sprintf(temp,"number %d",i);
    ajHistSetMultiTitleC(hist,i,temp);
    sprintf(temp,"sequence %d",i);
    ajHistSetMultiXTitleC(hist,i,temp);
    sprintf(temp,"y value for %d",i);
    ajHistSetMultiYTitleC(hist,i,temp);
  }
  ajHistSetBlackandWhite(hist , AJFALSE);
  ajHistDisplay (hist);

  

  ajHistDelete(hist);
   

 ajExit ();
  return 0;
}

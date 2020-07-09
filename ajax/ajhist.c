/********************************************************************
** @source AJAX GRAPH (ajax histogram) functions
**
** These functions control all aspects of AJAX histogram.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0 
** @modified 1988-11-12 pmr First version
** @modified 1999 ajb ANSIfication
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

#include "ajax.h"
#include "ajgraph.h"
#include "ajhist.h"
#include "limits.h"
#include "float.h"

ajint aj_hist_mark=GRAPH_HIST;


/* @func ajHistDisplay **********************************************
**
** Display the histogram.
**
** @param [r] hist [AjPHist] Histogram Structure.
** @return [void]
** @@
********************************************************************/

void ajHistDisplay (AjPHist hist){
  PLFLT *data=NULL,*totals=NULL,*totals2=NULL; 
  float ptsperbin,max=FLT_MIN,min=0.0;
  ajint i,j,ratioint,num,old;
  float bin_range,bar_width,offset,start,tot;
  float percent5;

  /* Sanity check */
  if(hist->numofdatapoints < 1 || hist->numofsets < 1 || hist->bins < 1){
    ajErr("points =%d, sets = %d, bins = %d !!! Must all be Greater than 1 ",
	  hist->numofdatapoints,hist->numofsets,hist->bins);
    return;
  }
  /* what multiple is the bins to numofdatasets */
  /* as i may have to take an average if not identical */
  ptsperbin = (float)hist->numofdatapoints/(float)hist->bins;
  
  if(ptsperbin < 1.0){
    ajErr("You cannot more have bins than datapoints!!");
    return;
  }
  /* is the ratio a whole number? */
  ratioint = (ajint)ptsperbin;
  if((ptsperbin - (float)ratioint) != 0.0){
    ajErr("number of data points needs to be a multiple of bins");
    return;
  }
  /* end Sanity check */

  /* Add spacing either side */
  percent5 = (hist->xmax-hist->xmin)*(float)0.025;

  /* check to see if data is okay */
  /*  for(i=0;i<hist->numofsets;i++){
    (void) printf("\n");
    data = hist->hists[i]->data;
    for(j=0;j<hist->numofdatapoints;j++){      
      (void) printf("%d %f\t",i+1,data[j]); 
    } 
    (void) printf("\n");
  } 
  */

  /* calculate max and min for each set */
  if(hist->numofsets != 1){
  
    /* if NOT side by side max and min as the sets added together */
    if(hist->displaytype == HIST_SIDEBYSIDE){
      /* find the max value */
      max = INT_MIN;
      min = 0;
      for(j=0;j<hist->numofsets;j++){
	data = hist->hists[j]->data;
	for(i=0;i<hist->numofdatapoints;i++){	  
	  if(data[i] > max)
	    max = data[i];
	  if(data[i] < min)
	    min = data[i];
	}
      }
    }
    else if(hist->displaytype == HIST_ONTOP){
      totals = AJALLOC(hist->numofdatapoints*(sizeof(PLFLT)));    
      /* set all memory to 0.0 */
      for(i=0;i<hist->numofdatapoints;i++){
	totals[i] = 0.0;
      }
      min = 0;
      max = 0;
      for(j=0;j<hist->numofsets;j++){
	data = hist->hists[j]->data;
	for(i=0;i<hist->numofdatapoints;i++){
	  totals[i] += data[i]; 
	  if(totals[i] > max)
	    max = totals[i];
	  if(totals[i] < min)
	    min = totals[i];	  
	  /*	  ajDebug("%d %d\t%f",j,i,totals[i]);*/
	}
      }
    }
    else if(hist->displaytype == HIST_SEPARATE){
      totals = AJALLOC(hist->numofsets*(sizeof(PLFLT)));    
      totals2 = AJALLOC(hist->numofsets*(sizeof(PLFLT)));    
      for(j=0;j<hist->numofsets;j++){
	data = hist->hists[j]->data;
	totals[j] = 0;
	totals2[j] = 0;
	for(i=0;i<hist->numofdatapoints;i++){
	  if(totals[j] < data[i])
	    totals[j] = data[i];
	  if(totals2[j] > data[i])
	    totals2[j] = data[i];
	}
      }
    }
  }
  else {
    data = hist->hists[0]->data;
    max = data[0];  
    min = 0;
    for(i=1;i<hist->numofdatapoints;i++){
      if(data[i] > max)
	max = data[i];
      if(data[i] < min)
	min = data[i];
    }
    if(hist->displaytype == HIST_ONTOP/*!hist->sidebyside*/){
      totals = AJALLOC(hist->numofdatapoints*(sizeof(PLFLT)));    
      /* set all memory to 0.0 */
      for(i=0;i<hist->numofdatapoints;i++){
	totals[i] = 0.0;
      }
    } 
    else if(hist->displaytype == HIST_SEPARATE){
      totals = AJALLOC((sizeof(PLFLT)));    
      totals[0]= max;
      totals2 = AJALLOC((sizeof(PLFLT)));    
      totals2[0]= min;
    }
  }

  bin_range = (hist->xmax - hist->xmin)/(float)hist->bins;

  if(hist->displaytype != HIST_SEPARATE){
    if (max <= 0.01) {
      if (max < 0.0)
	max = 0.0;
      else
	max = 1.0;
    }
    ajGraphOpenPlot (hist->graph, 1);
    ajGraphPlenv(hist->xmin-percent5, hist->xmax+percent5, min,
		 max*((float)1.025), aj_hist_mark);
    ajGraphLabel(ajStrStr(hist->xaxis) ,
		     ajStrStr(hist->yaxisleft) , 
		     ajStrStr(hist->title)," ");
    
    ajGraphLabelYRight(ajStrStr(hist->yaxisright));
  }
  else {
    ajGraphOpenPlot(hist->graph, hist->numofsets);
  }

  if(hist->displaytype == HIST_SIDEBYSIDE){
    bar_width = bin_range/hist->numofsets;
    for(i=0;i<hist->numofsets;i++){
      offset = i*bar_width;
      start = hist->xmin;
      num = 0;
      tot=0.0;
      data = hist->hists[i]->data;
      for(j=0;j<hist->numofdatapoints;j++){
	tot += data[j];
	num++;
	if(num >= ptsperbin){
	  tot = tot / (float)num;
	  if(hist->BaW)
	    old = ajGraphSetFillPat(hist->hists[i]->pattern);
	  else
	    old = ajGraphSetFore(hist->hists[i]->colour);
	  ajGraphRectFill(start+offset,0.0,start+offset+bar_width,tot);
	  if(hist->BaW)
	    (void) ajGraphSetFillPat(old);
	  else
	    (void) ajGraphSetFore(old);
	  ajGraphRect(start+offset,0.0,start+offset+bar_width,tot);
	  num = 0;
	  tot = 0;
	  start +=bin_range; 
	}
	 
      }
    }
  }
  else if(hist->displaytype == HIST_SEPARATE){
    bar_width = bin_range;
    for(i=0;i<hist->numofsets;i++){
      
      if (totals[i] <= 0.01) {	/* apparently the ymin value */
	if (totals[i] < 0.0)
	  totals[i] = 0.0;
	else
	  totals[i] = 1.0;
      }
      ajGraphPlenv(hist->xmin-percent5, hist->xmax+percent5,
		   totals2[i]*((float)1.025), totals[i]*((float)1.025),
		   aj_hist_mark);
      offset = /*bar_width*/0.0;
      start = hist->xmin;
      num = 0;
      tot=0.0;
      data = hist->hists[i]->data;
      ajGraphLabel(ajStrStr(hist->hists[i]->xaxis) ,
		       ajStrStr(hist->hists[i]->yaxis) , 
		       ajStrStr(hist->hists[i]->title)," ");

      for(j=0;j<hist->numofdatapoints;j++){
	tot += data[j];
	num++;
	if(num >= ptsperbin){
	  tot = tot / (float)num;
	  if(hist->BaW)
	    old = ajGraphSetFillPat(hist->hists[i]->pattern);
	  else
	    old = ajGraphSetFore(hist->hists[i]->colour);
	  ajGraphRectFill(start+offset,0.0,start+offset+bar_width,tot);
	  if(hist->BaW)
	    (void) ajGraphSetFillPat(old);
	  else
	    (void) ajGraphSetFore(old);
	  ajGraphRect(start+offset,0.0,start+offset+bar_width,tot);
	  num = 0;
	  tot = 0;
	  start +=bin_range; 
	}
	 
      }
    }
  }
  else if(hist->displaytype == HIST_ONTOP) {
    for(i=0;i<hist->numofdatapoints;i++)
      totals[i] = 0.0;
    for(i=0;i<hist->numofsets;i++){
      data = hist->hists[i]->data;
      start = hist->xmin;
      num = 0;
      tot=0.0;
      
      for(j=0;j<hist->numofdatapoints;j++){
	tot += data[j];
	num++;
	if(num >= ptsperbin){
	  tot = tot / (float)num;
	  if(hist->BaW)
	    old = ajGraphSetFillPat(hist->hists[i]->pattern);
	  else
	    old = ajGraphSetFore(hist->hists[i]->colour);
	  ajGraphRectFill(start,totals[j],start+bin_range,tot+totals[j]);
	  if(hist->BaW)
	    (void) ajGraphSetFillPat(old);
	  else
	    (void) ajGraphSetFore(old);
	  ajGraphRect(start,totals[j],start+bin_range,tot+totals[j]);
	  totals[j] += tot;
	  tot = 0;
	  /*	  ajDebug("num = %d",num);*/
	  num = 0;
	  start +=bin_range; 
	}	 
      }
    }
    AJFREE(totals);
    if(hist->displaytype == HIST_SEPARATE)
      AJFREE(totals2);
  }
}

/* @func ajHistClose **************************************************
**
** Closes the histograms window.
**
** @return [void]
**
** @@
***********************************************************************/
void ajHistClose (void) {
  ajGraphCloseWin();
}

/* @func ajHistDelete **************************************************
**
** Delete and free all memory associated with the histogram.
** Does not delete the graph.
**
** @param [rw] hist [AjPHist] Histogram to be deleted.
** @return [void]
**
** @@
***********************************************************************/

void ajHistDelete(AjPHist hist){
  ajint i;

  for(i=0;i<hist->numofsets; i++){
    if(hist->hists[i]->deletedata)
      AJFREE(hist->hists[i]->data);
    AJFREE((hist->hists[i]));
  }
  AJFREE(hist->hists);

  AJFREE(hist->title);
  AJFREE(hist->xaxis);
  AJFREE(hist->yaxisleft);
  AJFREE(hist->yaxisright);

  AJFREE(hist);
}

/* @func ajHistNew *****************************************************
**
** Create a histogram Object. Which can hold "numofsets" set of data of which
** all must have "numofpoints" data points in them.
**
** @param [r] numofsets [ajint] Number of sets of data.
** @param [r] numofpoints [ajint] Number of data points per set.
** @return [AjPHist] histogram structure.
** @@
**************************************************************************/
AjPHist ajHistNew(ajint numofsets, ajint numofpoints){
  static AjPHist hist=NULL;
  ajint i;
  
  AJNEW0(hist);

  hist->numofsets = 0;
  hist->numofsetsmax = numofsets;
  hist->numofdatapoints = numofpoints;
  hist->xmin = 0;
  hist->xmax = 0;
  hist->displaytype = HIST_SIDEBYSIDE;          /* default draw multiple histograms side by side */
  hist->bins = 0;                
  hist->BaW = AJFALSE;                
  (void) ajStrSetC(&hist->title,""); 
  (void) ajStrSetC(&hist->xaxis,"");
  (void) ajStrSetC(&hist->yaxisleft,""); 
  (void) ajStrSetC(&hist->yaxisright,""); 

  AJCNEW0(hist->hists,numofsets);
  for(i=0;i<numofsets; i++){
    AJNEW0((hist->hists[i]));
    (hist->hists)[i]->data = NULL;
    (hist->hists)[i]->deletedata = AJFALSE;
    (hist->hists)[i]->colour = i+2;
    (hist->hists)[i]->pattern = 0;
    (hist->hists)[i]->title = NULL;
    (hist->hists)[i]->xaxis = NULL;
    (hist->hists)[i]->yaxis = NULL;
    /*    (hist->hists)[i]->label = NULL;*/
  }
  
  return hist;

}
/* @func ajHistNewG *****************************************************
**
** Create a histogram Object which has the histogram data and graph data
** storage capacity.
**
** @param [r] numofsets [ajint] Number of sets of data.
** @param [r] numofpoints [ajint] Number of data points per set.
** @param [r] graph [AjPGraph] Graph object.
** @return [AjPHist] histogram structure.
** @@
**************************************************************************/
AjPHist ajHistNewG (ajint numofsets, ajint numofpoints, AjPGraph graph){

  AjPHist hist = ajHistNew (numofsets, numofpoints);
  hist->graph = graph;
  ajGraphSetDevice (graph);
  ajGraphSetMulti (graph, numofsets);
  ajGraphSetName (graph);
  return hist;
}

/* @func ajHistSetMultiTitle *********************************************
**
** Set ptr for title for index'th set..
**
** @param [rw] hist [AjPHist]   Histogram to have ptr set.
** @param [r] index [ajint]       Index for the set number.
** @param [r] title [AjPStr]    Title.
** @return [void]
** @@
*************************************************************************/
void ajHistSetMultiTitle(AjPHist hist, ajint index, AjPStr title){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  hist->hists[index]->title = title;
}

/* @func ajHistSetMultiTitleC *********************************************
**
** Store title for the index'th set.  
**
** @param [rw] hist [AjPHist]   Histogram to have ptr set.
** @param [r]  index [ajint]      Index for the set number.
** @param [r]  title  [char *]  Title.
** @return [void]
** @@
*************************************************************************/
void ajHistSetMultiTitleC(AjPHist hist, ajint index, char *title){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  (void) ajStrAssC(&hist->hists[index]->title,title);  
}

/* @func ajHistSetMultiXTitle *********************************************
**
** Set ptr for X axis title for index'th set..
**
** @param [rw] hist [AjPHist]   Histogram to have ptr set.
** @param [r] index [ajint]       Index for the set number.
** @param [r] title [AjPStr]    x Title.
** @return [void]
** @@
*************************************************************************/
void ajHistSetMultiXTitle(AjPHist hist, ajint index, AjPStr title){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  hist->hists[index]->xaxis = title;
}

/* @func ajHistSetMultiXTitleC *********************************************
**
** Store X axis title for the index'th set.  
**
** @param [rw] hist [AjPHist]   Histogram to have ptr set.
** @param [r] index [ajint]       Index for the set number.
** @param [r] title [char *]    x Title.
** @return [void]
** @@
*************************************************************************/
void ajHistSetMultiXTitleC(AjPHist hist, ajint index, char *title){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  (void) ajStrAssC(&hist->hists[index]->xaxis,title);  
}

/* @func ajHistSetMultiYTitle *********************************************
**
** Set ptr for Y axis title for index'th set..
**
** @param [rw] hist [AjPHist]   Histogram to have ptr set.
** @param [r] index [ajint]       Index for the set number.
** @param [r] title [AjPStr]    Y Title.
** @return [void]
** @@
*************************************************************************/
void ajHistSetMultiYTitle(AjPHist hist, ajint index, AjPStr title){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  hist->hists[index]->yaxis = title;
}

/* @func ajHistSetMultiYTitleC *********************************************
**
** Store Y axis title for the index'th set.  
**
** @param [rw] hist [AjPHist]   Histogram to have ptr set.
** @param [r] index [ajint]       Index for the set number.
** @param [r] title [char *]    Y Title.
** @return [void]
** @@
*************************************************************************/
void ajHistSetMultiYTitleC(AjPHist hist, ajint index, char *title){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  (void) ajStrAssC(&hist->hists[index]->yaxis,title);  
}

/* @func ajHistSetPtrToData *********************************************
**
** Set ptr to data for a set of data points for index'th set..
**
** @param [rw] hist [AjPHist] Histogram to have ptr set.
** @param [r] index [ajint]     Index for the set number.
** @param [r] data  [PLFLT*]  Ptr to the data.
** @return [void]
** @@
*************************************************************************/
void ajHistSetPtrToData(AjPHist hist, ajint index, PLFLT *data){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }
  if(!hist->hists[index]->data)
    hist->numofsets++;
  hist->hists[index]->data = data;
}

/* @func ajHistCopyData *************************************************
**
** Copy data from data ptr to histogram for index'th set.
**
** @param [rw] hist [AjPHist] Histogram to have ptr set.
** @param [r] index [ajint]     Index for the set number.
** @param [r] data  [PLFLT*]  Ptr to the data.
** @return [void]
** @@
*************************************************************************/
void ajHistCopyData(AjPHist hist, ajint index, PLFLT *data){
  ajint i;

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d", hist->numofdatapoints-1,index);
    return;
  }

  hist->hists[index]->data = AJALLOC(hist->numofdatapoints*sizeof(PLFLT));
  for(i=0;i<hist->numofdatapoints;i++)
    hist->hists[index]->data[i] = data[i];
 
  hist->hists[index]->deletedata = AJTRUE;
  hist->numofsets++;
  return;
}


/* @func ajHistSetTitleC ***********************************************
** 
** Copy Title for the histogram.
**
** @param [rw] hist [AjPHist] histogram to set string in.
** @param [r] string [char*] text to be copied.
** @return [void]
** @@
************************************************************************/
void ajHistSetTitleC(AjPHist hist, char* string){
  (void) ajStrAssC(&hist->title,string);
}


/* @func ajHistSetXAxisC ***********************************************
** 
** Store X axis label for the histogram
**
** @param [rw] hist [AjPHist] histogram to set string in.
** @param [r] string [char*] text to be copied.
** @return [void]
** @@
************************************************************************/
void ajHistSetXAxisC(AjPHist hist, char* string){
  (void) ajStrAssC(&hist->xaxis,string);
}


/* @func ajHistSetYAxisLeftC *******************************************
** 
** Store Y Axis Left Label for the histogram
**
** @param [rw] hist [AjPHist] histogram to set string in.
** @param [r] string [char*] text to be copied.
** @return [void]
** @@
************************************************************************/
void ajHistSetYAxisLeftC(AjPHist hist, char* string){
  (void) ajStrAssC(&hist->yaxisleft,string);
}


/* @func ajHistSetYAxisRightC*******************************************
** 
** Store Y Axis Right Label for the histogram
**
** @param [rw] hist [AjPHist] histogram to set string in.
** @param [r] string [char*] text to be copied.
** @return [void]
** @@
************************************************************************/
void ajHistSetYAxisRightC(AjPHist hist, char* string){
  (void) ajStrAssC(&hist->yaxisright,string);
}

/* @func  ajHistSetColour ************************************************
**
** Set colour for bars in histogram for index'th set.
**
** @param [rw] hist [AjPHist] Histogram to have ptr set.
** @param [r] index [ajint]     Index for the set number.
** @param [r] colour [ajint]    Colour for bar set.
** @return [void]
** @@
*************************************************************************/
void ajHistSetColour(AjPHist hist, ajint index, ajint colour){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }

  hist->hists[index]->colour = colour;
}


/* @func  ajHistSetPattern ************************************************
**
** Set colour for bars in histogram for index'th set.
**
** @param [rw] hist [AjPHist] Histogram to have ptr set.
** @param [r] index [ajint]     Index for the set number.
** @param [r] style [ajint]    Line style number for bar set.
** @return [void]
** @@
*************************************************************************/
void ajHistSetPattern(AjPHist hist, ajint index, ajint style){

  if(index >= hist->numofdatapoints || index < 0){
    ajErr("Histograms can only be allocated from 0 to %d. NOT %d",
	   hist->numofdatapoints-1,index);
    return;
  }

  hist->hists[index]->pattern = style;
}

/* @func  ajHistSetBlackandWhite *****************************************
**
** Set patterns instead of colours for printing to B/W printers etc.
**
** @param [rw] hist [AjPHist] Histogram to have ptr set.
** @param [r] set [AjBool]    Set to use patterns or colour for filling.
** @return [void]
** @@
*************************************************************************/
void ajHistSetBlackandWhite(AjPHist hist, AjBool set){
   hist->BaW = set;
}










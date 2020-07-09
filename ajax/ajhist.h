#ifdef __cplusplus
extern "C"
{
#endif

/* @source embGraph.h 
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

#ifndef ajhist_h
#define ajhist_h

#include "ajgraph.h"
#include "ajdefine.h"
#include "ajstr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define HIST_ONTOP      0
#define HIST_SIDEBYSIDE 1
#define HIST_SEPARATE   2

#define GRAPH_HIST (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT+ AJGRAPH_Y_INVERT_TICK + AJGRAPH_X_INVERT_TICK + AJGRAPH_Y_TICK + AJGRAPH_X_TICK + AJGRAPH_X_LABEL + AJGRAPH_Y_LABEL + AJGRAPH_TITLE )

extern int aj_hist_mark;    

typedef struct AjSHistData {
  float *data; /* y coors */
  AjBool deletedata;
  int colour;
  int pattern;
  /*  AjPStr label;*/
  AjPStr title;
  AjPStr xaxis;
  AjPStr yaxis;
  /*  AjPStr yaxisright;*/
} AjOHistData, *AjPHistData;

typedef struct AjSHist {
  int numofsets;        /* number of current sets */
  int numofsetsmax;     /* maximum number of sets */
  int numofdatapoints; 
  float xmin,xmax;
  int displaytype;
  int bins;
  AjBool BaW;           /* Black and White */
  AjPStr title;
  AjPStr xaxis;
  AjPStr yaxisleft;
  AjPStr yaxisright;
  AjPGraph graph;
  AjPHistData *hists;
} AjOHist, *AjPHist;

void    ajHistClose (void);
void    ajHistCopyData (AjPHist hist, int index, PLFLT *data);
void    ajHistDelete (AjPHist hist);
void    ajHistDisplay (AjPHist hist);
AjPHist ajHistNew (int numofsets, int numofpoints);
AjPHist ajHistNewG (int numofsets, int numofpoints, AjPGraph graph);
void    ajHistSetBlackandWhite (AjPHist hist, AjBool set);
void    ajHistSetColour(AjPHist hist, int index, int colour);
void    ajHistSetMultiTitle  (AjPHist hist,int index, AjPStr title);
void    ajHistSetMultiTitleC (AjPHist hist,int index, char *title);
void    ajHistSetMultiXTitle  (AjPHist hist,int index, AjPStr title);
void    ajHistSetMultiXTitleC (AjPHist hist,int index, char *title);
void    ajHistSetMultiYTitle  (AjPHist hist,int index, AjPStr title);
void    ajHistSetMultiYTitleC (AjPHist hist,int index, char *title);
void    ajHistSetPattern (AjPHist hist, int index, int style);
void    ajHistSetPtrToData (AjPHist hist,int index, PLFLT *data);
void    ajHistSetTitleC (AjPHist hist, char* string);
void    ajHistSetXAxisC (AjPHist hist, char* string);
void    ajHistSetYAxisLeftC (AjPHist hist, char* string);
void    ajHistSetYAxisRightC (AjPHist hist, char* string);

#endif /* ajhist_h */

#ifdef __cplusplus
}
#endif

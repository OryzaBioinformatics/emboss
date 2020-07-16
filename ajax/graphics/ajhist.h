#ifdef __cplusplus
extern "C"
{
#endif




/* @source ajhist.h
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

#define GRAPH_HIST (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT + \
AJGRAPH_Y_INVERT_TICK + AJGRAPH_X_INVERT_TICK + AJGRAPH_Y_TICK + \
AJGRAPH_X_TICK + AJGRAPH_X_LABEL + AJGRAPH_Y_LABEL + AJGRAPH_TITLE )

#ifndef WIN32
extern ajint aj_hist_mark;
#else
#ifdef AJAXGDLL_EXPORTS
__declspec(dllexport) ajint aj_hist_mark;
#else
__declspec(dllimport) ajint aj_hist_mark;
#endif
#endif




/* @data AjPHistdata **********************************************************
**
** AJAX data structure for histogram graph data
**
** @attr data [float*] y coordinates
** @attr deletedata [AjBool] if true, delete data
** @attr colour [ajint] colour code
** @attr title [AjPStr] graph title
** @attr subtitle [AjPStr] graph subtitle
** @attr xaxis [AjPStr] x axis title
** @attr yaxis [AjPStr] y axis title
** @attr pattern [ajint] pattern code
** @attr Padding [char[4]] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct AjSHistdata {
  float *data;
  AjBool deletedata;
  ajint colour;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxis;
  ajint pattern;
  char Padding[4];
} AjOHistdata;
#define AjPHistdata AjOHistdata*




/* @data AjPHist **************************************************************
**
** AJAX data structure for histogram graph objects
**
**
** @attr numofsets [ajuint] number of current sets
** @attr numofsetsmax [ajuint] maximum number of sets
** @attr numofdatapoints [ajuint] number of data points
** @attr xmin [float] Lowest x value
** @attr xmax [float] Highest x value
** @attr displaytype [ajuint] Display type index
** @attr bins [ajuint] Number of histogram bins
** @attr BaW [AjBool] Black and white rendering if ajTrue
** @attr title [AjPStr] Plot title
** @attr subtitle [AjPStr] Plot subtitle
** @attr xaxis [AjPStr] x axis title
** @attr yaxisleft [AjPStr] y axis title
** @attr yaxisright [AjPStr] second y axis title for right side
** @attr graph [AjPGraph] Graph
** @attr hists [AjPHistdata*] Histogram bin data
** @@
******************************************************************************/

typedef struct AjSHist {
  ajuint numofsets;
  ajuint numofsetsmax;
  ajuint numofdatapoints;
  float xmin;
  float xmax;
  ajuint displaytype;
  ajuint bins;
  AjBool BaW;
  AjPStr title;
  AjPStr subtitle;
  AjPStr xaxis;
  AjPStr yaxisleft;
  AjPStr yaxisright;
  AjPGraph graph;
  AjPHistdata *hists;
} AjOHist;
#define AjPHist AjOHist*




/*
** Prototype definitions
*/

void    ajHistogramClose (void);
void    ajHistDataCopy (AjPHist hist, ajuint indexnum, PLFLT const *data);
void    ajHistDel (AjPHist* hist);
void    ajHistDisplay (const AjPHist hist);
AjPHist ajHistNew (ajuint numofsets, ajuint numofpoints);
AjPHist ajHistNewG (ajuint numofsets, ajuint numofpoints, AjPGraph graph);
void    ajHistSetMono (AjPHist hist, AjBool set);
void    ajHistSetmultiColour(AjPHist hist, ajuint indexnum, ajint colour);
void    ajHistSetmultiTitleS (AjPHist hist, ajuint indexnum,
                              const AjPStr title);
void    ajHistSetmultiTitleC (AjPHist hist, ajuint indexnum, const char *title);
void    ajHistSetmultiXlabelS (AjPHist hist, ajuint indexnum,
                               const AjPStr title);
void    ajHistSetmultiXlabelC (AjPHist hist, ajuint indexnum,
                               const char *title);
void    ajHistSetmultiYlabelS (AjPHist hist, ajuint indexnum,
                               const AjPStr title);
void    ajHistSetmultiYlabelC (AjPHist hist, ajuint indexnum,
                               const char *title);
void    ajHistSetmultiPattern (AjPHist hist, ajuint indexnum, ajint style);
void    ajHistDataAdd (AjPHist hist, ajuint indexnum, PLFLT *data);
void    ajHistSetTitleC (AjPHist hist, const char* strng);
void    ajHistSetXlabelC (AjPHist hist, const char* strng);
void    ajHistSetYlabelC (AjPHist hist,const  char* strng);
void    ajHistSetRlabelC (AjPHist hist, const char* strng);

void    ajHistogramSetMark(ajint mark);

/*
** End of prototype definitions
*/

__deprecated void    ajHistSetColour(AjPHist hist, ajuint indexnum,
                                     ajint colour);
__deprecated void    ajHistSetPattern (AjPHist hist, ajuint indexnum,
                                       ajint style);
__deprecated void    ajHistSetBlackandWhite (AjPHist hist, AjBool set);
__deprecated void    ajHistSetXAxisC (AjPHist hist, const char* strng);
__deprecated void    ajHistSetYAxisLeftC (AjPHist hist,const  char* strng);
__deprecated void    ajHistSetYAxisRightC (AjPHist hist, const char* strng);
__deprecated void    ajHistCopyData (AjPHist hist, ajuint indexnum,
                                     const PLFLT *data);
__deprecated void    ajHistSetPtrToData (AjPHist hist,
                                         ajint indexnum, PLFLT *data);
__deprecated void    ajHistSetMark(ajint mark);
__deprecated void    ajHistClose (void);
__deprecated void    ajHistDelete (AjPHist* hist);
__deprecated void    ajHistSetMultiTitle  (AjPHist hist, ajint indexnum,
                                           const AjPStr title);
__deprecated void    ajHistSetMultiTitleC (AjPHist hist, ajint indexnum,
                                           const char *title);
__deprecated void    ajHistSetMultiXTitle  (AjPHist hist, ajint indexnum,
                                            const AjPStr title);
__deprecated void    ajHistSetMultiXTitleC (AjPHist hist, ajint indexnum,
                                            const char *title);
__deprecated void    ajHistSetMultiYTitle  (AjPHist hist, ajint indexnum,
                                            const AjPStr title);
__deprecated void    ajHistSetMultiYTitleC (AjPHist hist, ajint indexnum,
                                            const char *title);

#endif /* ajhist_h */

#ifdef __cplusplus
}
#endif

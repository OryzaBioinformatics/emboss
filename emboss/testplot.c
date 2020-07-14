/* @source testplot.c
**
** General test routine for graph plotting PLPLOT.
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

#ifndef NO_PLOT
#include "emboss.h"
#include "ajax.h"
#include "ajgraph.h"
#include <math.h>
#define numsets 3
#define numpoints 360

ajint ipoints;




/* @prog testplot *************************************************************
**
** Test of plotting
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPGraphData graphdata;
    ajint i;
    AjPGraph mult;
    AjBool overlap;

    ajGraphInit("testplot", argc, argv);

    mult    = ajAcdGetGraphxy("graph");
    ipoints = ajAcdGetInt("points");
    overlap = ajAcdGetBool("overlap");

    ajUser("Plotting sin, cos and  tan for %d degrees",ipoints);

    /* Create multiple graph store for a set of graphs */
    /* This is used for drawing several graphs in one window */

    /* create a new graph */
    graphdata = ajGraphxyDataNewI(ipoints);

    /* add graph to list in a multiple graph */
    ajGraphxyAddGraph(mult,graphdata);

    /* set overlap based on bool*/
    ajGraphxySetOverLap(mult, overlap);

    /* create the point values for this graph */
    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = sin(ajDegToRad(i));
    }

    /* embGraphSetData(graphdata,&array[0][0]);*/
    ajGraphxyDataSetYtitleC(graphdata,"SINE(degrees)");
    ajGraphxyDataSetXtitleC(graphdata,"degrees");
    ajGraphxyDataSetTitleC(graphdata,"hello");
    ajGraphxySetColour(graphdata,GREEN);

    if(!overlap)
    {
	ajGraphDataObjAddRect(graphdata,70.0,0.55,80.0,0.45,GREEN,1);
	ajGraphDataObjAddText(graphdata,82.0,0.5,GREEN,"Sine");
    }

    graphdata = ajGraphxyDataNewI(ipoints);

    ajGraphxyAddGraph(mult,graphdata);

    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = cos(ajDegToRad((float)i));
    }

    ajGraphxyDataSetXtitleC(graphdata,"degrees");
    ajGraphxyDataSetYtitleC(graphdata,"COS(degrees)");
    ajGraphxyDataSetTitleC(graphdata,"hello");
    ajGraphxySetColour(graphdata,RED);

    if(!overlap)
    {
	ajGraphDataObjAddLine(graphdata,5.0,0.1,15.0,0.1,RED);
	ajGraphDataObjAddText(graphdata,17.0,0.1,RED,"Cosine");
    }

    /* now set larger axis than needed */
    ajGraphDataxySetMaxMin(graphdata,0.0,(float)ipoints,-0.5,1.5);

    graphdata = ajGraphxyDataNewI(ipoints);
    ajGraphxySetLineType(graphdata, 3);
    ajGraphxyAddGraph(mult,graphdata);

    for(i=0;i<ipoints; i++)
    {
	graphdata->x[i] = (float)i;
	graphdata->y[i] = (tan(ajDegToRad(i))*0.2);
    }

    ajGraphxyDataSetXtitleC(graphdata,"degrees");
    ajGraphxyDataSetYtitleC(graphdata,"TAN(degrees)");
    ajGraphxyDataSetTitleC(graphdata,"hello");
    ajGraphxySetLineType(graphdata, 2);
    ajGraphxySetColour(graphdata,BLUE);

    if(!overlap)
    {
	ajGraphDataObjAddRect(graphdata,5.0,9.0,15.0,8.5,BLUE,0);
	ajGraphDataObjAddText(graphdata,17.0,8.75,BLUE,"Tangent");
    }

    ajGraphxyYtitleC(mult,"sin,cos,tan");
    ajGraphxyXtitleC(mult,"degrees");
    ajGraphxyTitleC(mult,"Trig functions");

    ajGraphxySetYStart(mult,0.0);
    ajGraphxySetYEnd(mult,2.0);

    if(overlap)
    {
	ajGraphObjAddRect(mult,5.0,9.0,15.0,8.5,BLUE,0);
	ajGraphObjAddRect(mult,5.0,8.0,15.0,7.5,GREEN,1);
	ajGraphObjAddLine(mult,5.0,6.75,15.0,6.75,RED);
	ajGraphObjAddText(mult,17.0,8.75,BLUE,"Tangent");
	ajGraphObjAddText(mult,17.0,7.75,GREEN,"Sine");
	ajGraphObjAddText(mult,17.0,6.75,RED,"Cosine");
    }

    ajGraphxyDisplay(mult,AJTRUE);

    ajExit();

    return 0;
}
#else
int main(int argc, char **argv)
{
    ajFatal("Sorry no PLplot was found on compilation hence NO graph\n");
    return 0;
}
#endif





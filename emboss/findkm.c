/* @source findkm.c
**
** @author: Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk),
** David Martin (david.martin@biotek.uio.no)
**
** Application to calculate the Michaelis Menton Constants (Km) of different
** enzymes and their substrates.
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

#include "emboss.h"
#include <math.h>
#include <stdlib.h>
#include <limits.h>

static float findkm_summation(float *arr, ajint number);
static float findkm_multisum (float *arr1, float *arr2, ajint number);
static float findkm_findmax(float *arr1, ajint number);
static float findkm_findmin(float *arr1, ajint number);


/*Func declarations */


/* @prog findkm ***************************************************************
**
** Find Km and Vmax for an enzyme reaction by a Hanes/Woolf plot
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile infile = NULL;
    AjPFile outfile = NULL;
    AjPStr line;
    AjPGraph graphLB =NULL;
    AjPGraphData xygraph=NULL;
    AjPGraphData xygraph2 =NULL;
    AjBool doplot;

    ajint N=0;

    float *xdata=NULL;
    float *ydata=NULL;
    float *V=NULL;
    float *S=NULL;

    float a;
    float b;
    float upperXlimit;
    float upperYlimit;

    float A;
    float B;
    float C;
    float D;
    float xmin;
    float xmax;
    float ymin;
    float ymax;
    float xmin2;
    float xmax2;
    float ymin2;
    float ymax2;

    float Vmax;
    float Km;
    float cutx;
    float cuty;

    float amin=0.;
    float amax=0.;
    float bmin=0.;
    float bmax=0.;


    (void)ajGraphInit("findkm", argc, argv);

    infile = ajAcdGetInfile("infile");
    outfile = ajAcdGetOutfile ("outfile");
    doplot =ajAcdGetBool("plot");
    graphLB = ajAcdGetGraphxy("graphLB");
    line = ajStrNew();


    /* Determine N by reading infile */

    while (ajFileReadLine(infile, &line))
    {
        if (ajStrLen(line) >0) N++;
    }


    /* only allocate memory to the arrays */

    AJCNEW (xdata, N);
    AJCNEW (ydata, N);
    AJCNEW (S, N);
    AJCNEW (V, N);

    (void)ajFileSeek(infile, 0L, 0);

    N=0;
    while (ajFileReadLine(infile, &line))
    {
	if (ajStrLen(line) > 0)
        {
            (void)sscanf(ajStrStr(line),"%f %f",&S[N],&V[N]);
            if (S[N] > 0.0 && V[N] > 0.0)
            {
                xdata[N] = S[N];
                ydata[N] = S[N]/V[N];
		N++;
            }
        }
    }


    /* find the max and min values for the graph parameters*/
    xmin = 0.5*findkm_findmin(xdata, N);
    xmax = 1.5*findkm_findmax(xdata, N);
    ymin = 0.5*findkm_findmin(ydata, N);
    ymax = 1.5*findkm_findmax(ydata, N);

    xmin2 = 0.5*findkm_findmin(S, N);
    xmax2 = 1.5*findkm_findmax(S, N);
    ymin2 = 0.5*findkm_findmin(V, N);
    ymax2 = 1.5*findkm_findmax(V, N);



    /* Incase the casted ints turn out to be same number on the axis,
       make the max number larger than the min so graph can be seen. */

    if ((ajint)xmax == (ajint)xmin)
        ++xmax;
    if ((ajint)ymax == (ajint)ymin)
        ++ymax;


    if ((ajint)xmax2 == (ajint)xmin2)
        ++xmax2;
    if ((ajint)ymax2 == (ajint)ymin2)
        ++ymax2;



    /* Gaussian Elimination for Best-fit curve plotting and
       calculating Km and Vmax */

    A = findkm_summation(xdata, N);
    B = findkm_summation(ydata, N);

    C = findkm_multisum(xdata, ydata, N);
    D = findkm_multisum(xdata, xdata, N);

    /*    c2 = (C -((A*B)/N) / (D - (D/N)));
	  c1 = ((B-A) * c2 /N);

	  Vmax = c1;

	  Km = (1/c1)/c2;*/



    /*To find the best fit line, Least Squares Fit:    y =ax +b;
      Two Simultaneous equations, REARRANGE FOR b

      findkm_summation(ydata, N) - findkm_summation(xdata,N)*a - N*b =0;
      b = (findkm_summation(ydata,N) - findkm_summation(xdata,N)*a) /  N;
      b = (B - A*a)/ N;

      C - D*a - A*((B - A*a)/ N) =0;
      C - D*a - A*B/N + A*A*a/N =0;
      C - A*B/N = D*a - A*A*a/N;*/

    /* REARRANGE FOR a */

    a = (N*C - A*B)/ (N*D - A*A);
    b = (B - A*a)/ N;

    /* Equation of Line - Lineweaver burk eqn*/
    /* 1/V = (Km/Vmax)*(1/S) + 1/Vmax;*/


    Vmax = 1/a;
    Km = b/a;

    cutx = -1/Km;
    cuty = Km/Vmax;

    /* set limits for last point on graph */

    upperXlimit = findkm_findmax(xdata,N)+3;
    upperYlimit = (upperXlimit)*a + b;

    (void)ajFmtPrintF(outfile, "---Hanes Woolf Plot Calculations---\n");
    (void)ajFmtPrintF(outfile, "Slope of best fit line is a = %.2f\n", a);
    (void)ajFmtPrintF(
		      outfile,"Coefficient in Eqn of line y = ma +b is b "
		      "= %.2f\n", b);

    (void)ajFmtPrintF(outfile, "Where line cuts x axis = (%.2f, 0)\n", cutx);
    (void)ajFmtPrintF(outfile, "Where line cuts y axis = (0, %.2f)\n", cuty);
    (void)ajFmtPrintF(outfile,
		      "Limit-point of graph for plot = (%.2f, %.2f)\n\n",
		      upperXlimit, upperYlimit);
    (void)ajFmtPrintF(outfile, "Vmax = %.2f, Km = %f\n",Vmax, Km);

    /* draw graphs */

    if(doplot)
    {
	xygraph = ajGraphxyDataNewI(N);
	ajGraphxyAddDataPtrPtr(xygraph, S, V);
	ajGraphxyAddGraph(graphLB, xygraph);
	ajGraphxyDataSetTitleC(xygraph, "Michaelis Menten Plot");
	ajGraphxyDataSetXtitleC(xygraph, "[S]");
	ajGraphxyDataSetYtitleC(xygraph, "V");

	ajGraphxySetXStart(graphLB, 0.0);
	ajGraphxySetXEnd(graphLB, xmax2);
	ajGraphxySetYStart(graphLB, 0.0);
	ajGraphxySetYEnd(graphLB, ymax2);
	ajGraphxySetXRangeII(graphLB, (ajint)0.0, (ajint)xmax2);
	ajGraphxySetYRangeII(graphLB, (ajint)0.0, (ajint)ymax2);
	ajGraphDataObjAddLine
	    (xygraph, 0.0, 0.0, S[0], V[0], (ajint)BLACK);
	ajGraphxySetCirclePoints(graphLB, ajTrue);
	ajGraphDataxySetMaxMin(xygraph,0.0,xmax2,0.0,ymax2);


	ajGraphDataxyMaxMin(S,N,&amin,&amax);
	ajGraphDataxyMaxMin(V,N,&bmin,&bmax);
	ajGraphDataxySetMaxima(xygraph,amin,amax,bmin,bmax);
	ajGraphDataxySetTypeC(xygraph,"2D Plot Float");

	xygraph2 = ajGraphxyDataNewI(N);
	ajGraphxyAddDataPtrPtr(xygraph2, xdata, ydata);
	ajGraphxyAddGraph(graphLB, xygraph2);

	ajGraphxyDataSetTitleC(xygraph2, "Hanes Woolf Plot");
	ajGraphxyDataSetXtitleC(xygraph2, "[S]");
	ajGraphxyDataSetYtitleC(xygraph2, "[S]/V");

	ajGraphxySetXStart(graphLB, cutx);
	ajGraphxySetXEnd(graphLB, upperXlimit);
	ajGraphxySetYStart(graphLB, 0.0);
	ajGraphxySetYEnd(graphLB, upperYlimit);
	ajGraphxySetXRangeII(graphLB, (ajint)cutx, (ajint)upperXlimit);
	ajGraphxySetYRangeII(graphLB, (ajint)0.0, (ajint)upperYlimit);
	/*    ajGraphDataObjAddLine
	      (xygraph2, cutx, 0.0, upperXlimit, upperYlimit, (ajint)RED);*/
	ajGraphxySetCirclePoints(graphLB, ajTrue);
	ajGraphDataxySetMaxMin(xygraph2, cutx,upperXlimit,0.0,upperYlimit);
	ajGraphDataxyMaxMin(xdata,N,&amin,&amax);
	ajGraphDataxyMaxMin(ydata,N,&bmin,&bmax);
	ajGraphDataxySetMaxima(xygraph2,amin,amax,bmin,bmax);
	ajGraphDataxySetTypeC(xygraph2,"2D Plot");



	ajGraphxyTitleC(graphLB,"FindKm");
	ajGraphxySetOverLap(graphLB,ajFalse);
	ajGraphxyDisplay(graphLB, ajTrue);
    }




    AJFREE (xdata);
    AJFREE (ydata);

    AJFREE (S);
    AJFREE (V);

    ajFileClose(&infile);

    ajExit();
    return 0;
}



/* @funcstatic findkm_summation ***********************************************
**
** Undocumented.
**
** @param [?] arr [float*] Undocumented
** @param [?] number [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/


static float findkm_summation(float *arr, ajint number)
{
    ajint i;
    float sum=0;

    for (i = 0; i < number; i++)
        sum += arr[i];

    return sum;
}


/* @funcstatic findkm_multisum ************************************************
**
** Undocumented.
**
** @param [?] arr1 [float*] Undocumented
** @param [?] arr2 [float*] Undocumented
** @param [?] number [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float findkm_multisum(float *arr1, float *arr2, ajint number)
{
    ajint i;
    float sum=0;

    for (i = 0; i < number; i++)
        sum += arr1[i]*arr2[i];

    return sum;
}




/* @funcstatic findkm_findmax *************************************************
**
** Undocumented.
**
** @param [?] arr [float*] Undocumented
** @param [?] number [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float findkm_findmax(float *arr, ajint number)
{
    ajint i;
    float max=arr[0];

    for (i=1; i<number; ++i)
        if (arr[i] > max)
            max = arr[i];

    return max;
}




/* @funcstatic findkm_findmin *************************************************
**
** Undocumented.
**
** @param [?] arr [float*] Undocumented
** @param [?] number [ajint] Undocumented
** @return [float] Undocumented
** @@
******************************************************************************/

static float findkm_findmin(float *arr, ajint number)
{
    ajint i;
    float min=arr[0];

    for (i=1; i<number; ++i)
        if (arr[i] < min)
            min = arr[i];

    return min;
}








    /* Gaussian Elimination -from 'Algorithms in C' .

       Best fit curves

       -Matrix multiplication
    (N      sumX)   (c1) =  sumY
    (sum X  sumX^2) (c2)    sumXY

    sim eqns:
    1   N*c1 +sumX*c2 = sumY
    2   sumX*c1 + sumX^2 * c2 = sumXY


    Subs for c2:

    c2  = (sumXY - sumX*c1)/ sumX^2
    c2  = sumXY - (sumY -c2*sumX / N) *sumX
    c2  = sumXY - (sumX*sumY + c2*sumX^2) / N
    c2  = sumXY - (sumX*sumY/N) + (c2 * (sumX^2)/ N)
    c2 - (c2*sumX^2)/ N = sumXY -sumX*sumY/ N
    c2(1 - sumX^2 / N) = sumXY - (sumX*sumY / N)
    c2 =  (sumXY - (sumX*sumY / N)) / sumX^2-(sumX^2 / N))



    Joining both together:


    c1  = sumY -sumX ((sumXY - (sumX*sumY / N)) / sumX^2 -(sumX^2 / N)) / N


    A = sumX
    B = sumY
    C = sumX*sumY
    D = sumX^2

    so sub'ing these values into the c1 & c2 eqns:

    c1 = (B-A ((C - A*B/N)/ (D - D/N)) )/N

    c2 = ((C - A*B/N)/ (D- D/N))

    */












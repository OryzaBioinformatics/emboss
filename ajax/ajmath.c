/*  Last edited: Mar  6 12:09 2000 (pmr) */
/*** general routines ***/

#include "ajmath.h"
#include <math.h>
#include <time.h>
#include <float.h>
#include <stdlib.h>


#define AjRandomXmod 1000009711.0
#define AjRandomYmod 33554432.0
#define AjRandomTiny 1.0e-17


static AjBool aj_rand_i = 0;
static int    aj_rand_index;
static double aj_rand_poly[101];
static double aj_rand_other;



/* @func ajRound *******************************************************
**
** Rounds an integer to be a multiple of a given number.
**
** @param [r] i [int] Integer to round.
** @param [r] round [int] Rounding multiple.
** @return [int] Result.
******************************************************************************/

int ajRound (int i, int round) {

  return round * ((int)(i+round-1)/round);
}


/* @func ajRoundF *******************************************************
**
** Rounds a floating point number to have bits free for cumulative addition
**
** @param [r] a [float] Float to round.
** @param [r] nbits [int] Number of bits to free.
** @return [float] Result.
******************************************************************************/

float ajRoundF (float a, int nbits) {

  double w, x, y, z, b, c;
  int i;
  int bitsused;

  bitsused = FLT_MANT_DIG - nbits; /* save 16 bits for cumulative error */
                                /* usually leave 8 bits */
  if (bitsused < 8) bitsused = 8;

  x = frexp (a, &i);            /* a is between 0.5 and 1.0 */
                                /* i is the power of two */

  /* multiply by 2**n, convert to an integer, divide again */
  /* so we only keep n (or whatever) bits */

  y = ldexp(x, bitsused);       /* multiply by 2**n */
  z = modf(y, &w);              /* change to an integer + remainder */
  if (z > 0.5) w += 1.0;        /* round up ?*/
  if (z < -0.5) w -= 1.0;       /* round down? */

  b = ldexp (w, -bitsused);     /* divide by 2**n */
  c = ldexp(b, i);              /* divide by the original power of two */

  /*  ajDebug ("\najRoundF (%.10e) c: %.10e bitsused: %d\n", a, c, bitsused);
      ajDebug ("       x: %f i: %d y: %f w: %.1f\n", x, i, y, w);*/
  
  return (float) c;
}


/* @func ajRecToPol  *******************************************************
**
** Converts cartesian co-ordinates to polar
**
** @param [r] x [float] X co-ordinate
** @param [r] y [float] Y co-ordinate
** @param [w] radius [float*] Radius
** @param [w] angle [float*] Angle
** @return [void]
******************************************************************************/
void ajRecToPol(float x, float y, float *radius, float *angle)
{
    *radius = (float) sqrt((double)(x*x+y*y));
    *angle  = (float) ajRadToDeg((float)atan2((double)y,(double)x));
}




/* @func ajPolToRec  *******************************************************
**
** Converts polar co-ordinates to cartesian
**
** @param [r] radius [float] Radius
** @param [r] angle [float] Angle
** @param [w] x [float*] X co-ordinate
** @param [w] y [float*] Y co-ordinate
** @return [void]
******************************************************************************/
void ajPolToRec(float radius, float angle, float *x, float *y)
{
    *x = radius*(float)cos((double)ajDegToRad(angle));
    *y = radius*(float)sin((double)ajDegToRad(angle));
}




/* @func ajDegToRad  *******************************************************
**
** Converts degrees to radians
**
** @param [r] degrees [float] Degrees
** @return [float] Radians
******************************************************************************/
float ajDegToRad(float degrees)
{
    return degrees*(float)(AJM_PI/180.0);
}



/* @func ajRadToDeg   *******************************************************
**
** Converts radians to degrees
**
** @param [r] radians [float] Radians
** @return [float] Degrees
******************************************************************************/
float ajRadToDeg(float radians)
{
    return radians*(float)(180.0/AJM_PI);
}



/* @func ajGaussProb   *******************************************************
**
** Returns a probability given a Gaussian distribution
**
** @param [r] mean [float] mean
** @param [r] sd [float] sd
** @param [r] score [float] score
** @return [double] probability
******************************************************************************/
double ajGaussProb(float mean, float sd, float score)
{
    return pow(AJM_E,(double)(-0.5*((score-mean)/sd)*((score-mean)/sd)))
	/ (sd * (float)2.0 * AJM_PI);
}




/* @func ajGeoMean   *******************************************************
**
** Calculate a geometric mean
**
** @param [r] s [float*] array of values
** @param [r] n [int] number of values
** @return [float] geometric mean
*****************************************************************************/
float ajGeoMean(float *s, int n)
{
    float x;
    int   i;

    for(i=0,x=1.0;i<n;++i) x*=s[i];
    return (float)pow((double)x,(double)(1.0/(float)n));
}




/* @func ajPosMod   *******************************************************
**
** Modulo always returning positive number
**
** @param [r] a [int] value1
** @param [r] b [int] value2
** @return [int] value1 modulo value2
*****************************************************************************/
int ajPosMod(int a, int b)
{
    int t;

    if(b<=0)
	ajFatal("ajPosMod given non-positive divisor");
    t=a%b;
    return (t<0) ? t+b : t;
}


/* @func ajRandomSeed *******************************************************
**
** Seed for the ajRandomNumberD routine
**
** Based on dprand and sdprand and used with the permission of the
** author....
** Copyright (C) 1992  N.M. Maclaren
** Copyright (C) 1992  The University of Cambridge
**
**  This software may be reproduced and used freely, provided that all
**  users of it agree that the copyright holders are not liable for any
**  damage or injury caused by use of this software and that this condition
**  is passed onto all subsequent recipients of the software, whether
**  modified or not.
**
** @return [void]
******************************************************************************/

void ajRandomSeed(void)
{
    int ix, iy, iz, i;
    double x=0.0;
    int seed;
    
    /*
     *  seed should be set to an integer between 0 and 9999 inclusive; a value
     *  of 0 will initialise the generator only if it has not already been
     *  done.
     */

    if (!aj_rand_i)
        aj_rand_i = 1;
    else
        return;

    seed = (time(0) % 9999)+1;

    /*
     *  aj_rand_index must be initialised to an integer between 1 and 101
     *  inclusive, aj_rand_poly[0...100] to integers between 0 and 1000009710
     *  inclusive (not all 0), and aj_rand_other to a non-negative proper
     *  fraction with denominator 33554432.  It uses the Wichmann-Hill
     *  generator to do this.
     */

    ix = (seed >= 0 ? seed : -seed) % 10000 + 1;
    iy = 2*ix+1;
    iz = 3*ix+1;
    for (i = -11; i < 101; ++i)
    {
        if (i >= 0) aj_rand_poly[i] = floor(AjRandomXmod*x);
        ix = (171*ix) % 30269;
        iy = (172*iy) % 30307;
        iz = (170*iz) % 30323;
        x = ((double)ix)/30269.0+((double)iy)/30307.0+((double)iz)/30323.0;
        x = x-floor(x);
    }

    aj_rand_other = floor(AjRandomYmod*x)/AjRandomYmod;
    aj_rand_index = 0;
}


/* @func ajRandomNumber *******************************************************
**
** Generate a pseudo-random number between 0-32767
**
** @return [int] Random number
******************************************************************************/

int ajRandomNumber(void)
{
    return (int) (floor(ajRandomNumberD()*32768.0));
}

/* @func ajRandomNumberD *****************************************************
**
** Generate a random number between 0-1.0
**
** Based on dprand and sdprand and used with the permission of the
** author....
** Copyright (C) 1992  N.M. Maclaren
** Copyright (C) 1992  The University of Cambridge
**
**  This software may be reproduced and used freely, provided that all
**  users of it agree that the copyright holders are not liable for any
**  damage or injury caused by use of this software and that this condition
**  is passed onto all subsequent recipients of the software, whether
**  modified or not.
**
** @return [double] Random number
******************************************************************************/

double ajRandomNumberD(void)
{
    static double offset = 1.0/AjRandomYmod;
    static double xmod2  = 2.0*AjRandomXmod;
    static double xmod4  = 4.0*AjRandomXmod;

    int n;
    double x, y;

    /*
     *  This returns a uniform (0,1) random number, with extremely good
     *  uniformity properties.  It assumes that double precision provides
     *  at least 33 bits of accuracy, and uses a power of two base.
     */

    if (!aj_rand_i) ajRandomSeed();

    /*
     *  See [Knuth] for why this implements the algorithm described in the
     *  paper.
     *  Note that this code is tuned for machines with fast double precision,
     *  but slow multiply and divide; many, many other options are possible.
     */

    if ((n = aj_rand_index-64) < 0)
	n += 101;
    x = aj_rand_poly[aj_rand_index]+aj_rand_poly[aj_rand_index];
    x = xmod4-aj_rand_poly[n]-aj_rand_poly[n]-x-x-aj_rand_poly[aj_rand_index];

    if (x <= 0.0)
    {
        if (x < -AjRandomXmod)
	    x += xmod2;
        if (x < 0.0)
	    x += AjRandomXmod;
    }
    else
    {
        if (x >= xmod2)
	{
            x = x-xmod2;
            if (x >= AjRandomXmod)
		x -= AjRandomXmod;
        }
        if (x >= AjRandomXmod)
	    x -= AjRandomXmod;
    }
    aj_rand_poly[aj_rand_index] = x;
    if (++aj_rand_index >= 101)
	aj_rand_index = 0;

    /*
     *  Add in the second generator modulo 1, and force to be non-zero.
     *  The restricted ranges largely cancel themselves out.
     */

    do
    {
        y = 37.0*aj_rand_other+offset;
        aj_rand_other = y-floor(y);
    }
    while (!aj_rand_other);

    if ((x = x/AjRandomXmod+aj_rand_other) >= 1.0)
	x -= 1.0;


    return x+AjRandomTiny;
}


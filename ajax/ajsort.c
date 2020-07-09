#include "ajax.h"


/* @func ajSortFloatDecI ********************************************************
**
** Based on an array of floats, sort an ajint element array.
**
** @param [r] a [float*] Array of floats used in sort tests
** @param [rw] p [ajint*] Array of ints to be sorted depending on floats.
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatDecI(float *a, ajint *p, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]>a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }
}


/* @func ajSortIntDecI ********************************************************
**
** Based on an array of ints, sort an ajint element array.
**
** @param [r] a [ajint*] Array of ints used in sort tests
** @param [rw] p [ajint*] Array of ints to be sorted depending on floats.
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntDecI(ajint *a, ajint *p, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]>a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }
}


/* @func ajSortFloatIncI ******************************************************
**
** Based on an array of floats, sort (ascending) an ajint element array.
**
** @param [r] a [float*] Array of floats used in sort tests
** @param [rw] p [ajint*] Array of ints to be sorted depending on floats.
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatIncI(float *a, ajint *p, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]<=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }
}


/* @func ajSortIntIncI ********************************************************
**
** Based on an array of ints, sort (ascending) an ajint element array.
**
** @param [r] a [ajint*] Array of ints used in sort tests
** @param [rw] p [ajint*] Array of ints to be sorted depending on floats.
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntIncI(ajint *a, ajint *p, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[p[j]]<=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }
}

/* @func ajSortFloatDec *******************************************************
**
** Sort a float array.
**
** @param [rw] a [float*] Array of floats to sort
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatDec(float *a, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    float t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }
}


/* @func ajSortIntDec  ********************************************************
**
** Sort an ajint array.
**
** @param [rw] a [ajint*] Array of ints to sort
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntDec(ajint *a, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }
}

/* @func ajSortFloatInc ******************************************************
**
** Sort a float array (ascending).
**
** @param [rw] a [float*] Array of floats to sort
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatInc(float *a, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    float t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }
}


/* @func ajSortIntInc  ********************************************************
**
** Sort an ajint array (ascending)
**
** @param [rw] a [ajint*] Array of ints to sort
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntInc(ajint *a, ajint n)
{
    ajint s;
    ajint i;
    ajint j;
    ajint t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }
}

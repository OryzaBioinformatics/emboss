#include "ajax.h"


/* @func ajSortFloatDecI ********************************************************
**
** Based on an array of floats, sort an int element array.
**
** @param [r] a [float*] Array of floats used in sort tests
** @param [rw] p [int*] Array of ints to be sorted depending on floats.
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatDecI(float *a, int *p, int n)
{
    int s;
    int i;
    int j;
    int t;

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
** Based on an array of ints, sort an int element array.
**
** @param [r] a [int*] Array of ints used in sort tests
** @param [rw] p [int*] Array of ints to be sorted depending on floats.
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntDecI(int *a, int *p, int n)
{
    int s;
    int i;
    int j;
    int t;

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
** Based on an array of floats, sort (ascending) an int element array.
**
** @param [r] a [float*] Array of floats used in sort tests
** @param [rw] p [int*] Array of ints to be sorted depending on floats.
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatIncI(float *a, int *p, int n)
{
    int s;
    int i;
    int j;
    int t;

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
** Based on an array of ints, sort (ascending) an int element array.
**
** @param [r] a [int*] Array of ints used in sort tests
** @param [rw] p [int*] Array of ints to be sorted depending on floats.
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntIncI(int *a, int *p, int n)
{
    int s;
    int i;
    int j;
    int t;

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
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatDec(float *a, int n)
{
    int s;
    int i;
    int j;
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
** Sort an int array.
**
** @param [rw] a [int*] Array of ints to sort
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntDec(int *a, int n)
{
    int s;
    int i;
    int j;
    int t;

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
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortFloatInc(float *a, int n)
{
    int s;
    int i;
    int j;
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
** Sort an int array (ascending)
**
** @param [rw] a [int*] Array of ints to sort
** @param [r] n [int] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/
void ajSortIntInc(int *a, int n)
{
    int s;
    int i;
    int j;
    int t;

    for(s=n/2; s>0; s /= 2)
	for(i=s; i<n; ++i)
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }
}

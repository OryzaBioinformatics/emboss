/******************************************************************************
** @source AJAX sort functions
**
** @author Copyright (C) 2001 Alan Bleasby
** @version 1.0
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
******************************************************************************/

#include "ajax.h"




/* @func ajSortFloatDecI ******************************************************
**
** Based on an array of floats, sort an ajint element array.
**
** @param [r] a [float*] Array of floats used in sort tests
** @param [u] p [ajint*] Array of ints to be sorted depending on floats.
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
	    for(j=i-s;j>=0 && a[p[j]]<a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortIntDecI ********************************************************
**
** Based on an array of ints, sort an ajint element array.
**
** @param [r] a [ajint*] Array of ints used in sort tests
** @param [u] p [ajint*] Array of ints to be sorted depending on floats.
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
	    for(j=i-s;j>=0 && a[p[j]]<a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortFloatIncI ******************************************************
**
** Based on an array of floats, sort (ascending) an ajint element array.
**
** @param [r] a [float*] Array of floats used in sort tests
** @param [u] p [ajint*] Array of ints to be sorted depending on floats.
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
	    for(j=i-s;j>=0 && a[p[j]]>=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortIntIncI ********************************************************
**
** Based on an array of ints, sort (ascending) an ajint element array.
**
** @param [r] a [ajint*] Array of ints used in sort tests
** @param [u] p [ajint*] Array of ints to be sorted depending on floats.
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
	    for(j=i-s;j>=0 && a[p[j]]>=a[p[j+s]]; j-=s)
	    {
		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}




/* @func ajSortTwoIntIncI *****************************************************
**
** Based on an array of ints, sort (ascending) two ajint element array.
**
** @param [r] a [ajint*] Array of ints used in sort tests
** @param [u] p [ajint*] Second array of ints to be sorted
** @param [r] n [ajint] Number of elements to sort
**
** @return [void]
** @@
******************************************************************************/

void ajSortTwoIntIncI(ajint *a, ajint *p, ajint n)
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

		t = p[j];
		p[j] = p[j+s];
		p[j+s] = t;
	    }

    return;
}





/* @func ajSortFloatDec *******************************************************
**
** Sort a float array.
**
** @param [u] a [float*] Array of floats to sort
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
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortIntDec  ********************************************************
**
** Sort an ajint array.
**
** @param [u] a [ajint*] Array of ints to sort
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
	    for(j=i-s;j>=0 && a[j]<a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortFloatInc *******************************************************
**
** Sort a float array (ascending).
**
** @param [u] a [float*] Array of floats to sort
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
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}




/* @func ajSortIntInc  ********************************************************
**
** Sort an ajint array (ascending)
**
** @param [u] a [ajint*] Array of ints to sort
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
	    for(j=i-s;j>=0 && a[j]>a[j+s]; j-=s)
	    {
		t = a[j];
		a[j] = a[j+s];
		a[j+s] = t;
	    }

    return;
}

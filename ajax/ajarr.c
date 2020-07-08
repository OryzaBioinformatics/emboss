/*  Last edited: Jun 14 16:02 2000 (pmr) */
/********************************************************************
** @source AJAX ARRAY functions
**
** These functions control all aspects of AJAX array utilities
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0 
** @modified Mar 12 1999 ajb First version
** @modified May 10 2000 ajb added dynamically allocated numeric arrays
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
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#ifndef HAVE_MEMMOVE
static void* memmove (void *dst, const void* src, size_t len) {
  return (void *)bcopy (src, dst, len);
}
#endif

#define RESERVED_SIZE 32


static AjBool ajIntResize(AjPInt *thys, int elem);
static AjBool ajInt2dResize(AjPInt2d *thys, int elem);
static AjBool ajInt3dResize(AjPInt3d *thys, int elem);
static AjBool ajFloatResize(AjPFloat *thys, int elem);
static AjBool ajFloat2dResize(AjPFloat2d *thys, int elem);
static AjBool ajFloat3dResize(AjPFloat3d *thys, int elem);
static AjBool ajDoubleResize(AjPDouble *thys, int elem);
static AjBool ajDouble2dResize(AjPDouble2d *thys, int elem);
static AjBool ajDouble3dResize(AjPDouble3d *thys, int elem);
static AjBool ajShortResize(AjPShort *thys, int elem);
static AjBool ajShort2dResize(AjPShort2d *thys, int elem);
static AjBool ajShort3dResize(AjPShort3d *thys, int elem);
static AjBool ajLongResize(AjPLong *thys, int elem);
static AjBool ajLong2dResize(AjPLong2d *thys, int elem);
static AjBool ajLong3dResize(AjPLong3d *thys, int elem);



/* @func ajIntNew *************************************************************
**
** Default constructor for empty AJAX integer arrays.
**
** @return [AjPInt] Pointer to an empty integer array structure
** @@
******************************************************************************/

AjPInt ajIntNew(void)
{
    AjPInt thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(int));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;
    
    return thys;
}



/* @func ajIntNewL ************************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size
** @return [AjPInt] Pointer to an empty integer array struct of specified size.
** @@
******************************************************************************/

AjPInt ajIntNewL(int size)
{
    AjPInt thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(int));
    thys->Len = 0;
    thys->Res = (int)size;
    
    return thys;
}


/* @func ajIntDel *************************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPInt*] Pointer to the int array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajIntDel(AjPInt *thys)
{

    if(!thys || !*thys)
	return;
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    
    return;
}


/* @func ajIntGet *************************************************************
**
** Retrieve an element from an AJAX integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPInt] Pointer to the int array.
** @param  [r] elem [int] array element.
**
** @return [int] contents of array element
** @@
******************************************************************************/

int ajIntGet(AjPInt thys, int elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad int array index %d\n",elem);
    
    return thys->Ptr[elem];
}


/* @func ajIntPut *************************************************************
**
** Load an integer array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the int array.
** @param  [r] elem [int] array element.
** @param  [r] v [int] value to load.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

AjBool ajIntPut(AjPInt *thys, int elem, int v)
{
    if(!thys || !*thys || elem<0)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    (void) ajIntResize(thys, elem);

    (*thys)->Ptr[elem] = v;
    
    return ajTrue;
}



/* @funcstatic ajIntResize ***************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajIntResize(AjPInt *thys, int size)
{
    AjPInt p=NULL;
    int    s;
    int    clen;
    int    limit;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    *thys = ajIntNewL(s);
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove((*thys)->Ptr,p->Ptr,limit*sizeof(int));

    (*thys)->Len = size+1;
    

    ajIntDel(&p);

    return ajTrue;
}



/* @func ajIntInt *************************************************************
**
** Returns the current int* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [AjPInt] Source array
** @return [int*] Current array pointer, or a null string if undefined.
** @@
******************************************************************************/
int* ajIntInt(AjPInt thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}



/* @func ajIntLen *************************************************************
**
** Get length of dynamic 1d int array
**
** @param [r] thys [AjPInt] Source array
** @return [int] length
** @@
******************************************************************************/

int ajIntLen(AjPInt thys)
{
    return thys->Len;
}



/* @func ajFloatNew *************************************************************
**
** Default constructor for empty AJAX float arrays.
**
** @return [AjPFloat] Pointer to an empty float array structure
** @@
******************************************************************************/

AjPFloat ajFloatNew(void)
{
    AjPFloat thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(float));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;
    
    return thys;
}



/* @func ajFloatNewL ************************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size
** @return [AjPFloat] Pointer to an empty float array struct of specified size.
** @@
******************************************************************************/

AjPFloat ajFloatNewL(int size)
{
    AjPFloat thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(float));
    thys->Len = 0;
    thys->Res = (int)size;
    
    return thys;
}


/* @func ajFloatDel *************************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPFloat*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFloatDel(AjPFloat *thys)
{

    if(!thys || !*thys)
	return;
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    
    return;
}


/* @func ajFloatGet *************************************************************
**
** Retrieve an element from an AJAX float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPFloat] Pointer to the float array.
** @param  [r] elem [int] array element.
**
** @return [float] contents of array element
** @@
******************************************************************************/

float ajFloatGet(AjPFloat thys, int elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad float array index %d\n",elem);
    
    return thys->Ptr[elem];
}


/* @func ajFloatPut *************************************************************
**
** Load a float array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat*] Pointer to the int array.
** @param  [r] elem [int] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

AjBool ajFloatPut(AjPFloat *thys, int elem, float v)
{
    if(!thys || !*thys || elem<0)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    (void) ajFloatResize(thys, elem);

    (*thys)->Ptr[elem] = v;
    
    return ajTrue;
}



/* @funcstatic ajFloatResize ***************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajFloatResize(AjPFloat *thys, int size)
{
    AjPFloat p=NULL;
    int    s;
    int    clen;
    int    limit;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    *thys = ajFloatNewL(s);
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove((*thys)->Ptr,p->Ptr,limit*sizeof(float));

    (*thys)->Len = size+1;
    

    ajFloatDel(&p);

    return ajTrue;
}



/* @func ajFloatFloat *************************************************************
**
** Returns the current float* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [AjPFloat] Source array
** @return [float*] Current array pointer, or a null string if undefined.
** @@
******************************************************************************/
float* ajFloatFloat(AjPFloat thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}



/* @func ajFloatLen *************************************************************
**
** Get length of dynamic 1d float array
**
** @param [r] thys [AjPFloat] Source array
** @return [int] length
** @@
******************************************************************************/

int ajFloatLen(AjPFloat thys)
{
    return thys->Len;
}



/* @func ajDoubleNew *************************************************************
**
** Default constructor for empty AJAX double arrays.
**
** @return [AjPDouble] Pointer to an empty double array structure
** @@
******************************************************************************/

AjPDouble ajDoubleNew(void)
{
    AjPDouble thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(double));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;
    
    return thys;
}



/* @func ajDoubleNewL ************************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size
** @return [AjPDouble] Pointer to an empty double array struct of specified size.
** @@
******************************************************************************/

AjPDouble ajDoubleNewL(int size)
{
    AjPDouble thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(double));
    thys->Len = 0;
    thys->Res = (int)size;
    
    return thys;
}


/* @func ajDoubleDel *************************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPDouble*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajDoubleDel(AjPDouble *thys)
{

    if(!thys || !*thys)
	return;
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    
    return;
}


/* @func ajDoubleGet *************************************************************
**
** Retrieve an element from an AJAX double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPDouble] Pointer to the double array.
** @param  [r] elem [int] array element.
**
** @return [double] contents of array element
** @@
******************************************************************************/

double ajDoubleGet(AjPDouble thys, int elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad double array index %d\n",elem);
    
    return thys->Ptr[elem];
}


/* @func ajDoublePut *************************************************************
**
** Load a double array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble*] Pointer to the int array.
** @param  [r] elem [int] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

AjBool ajDoublePut(AjPDouble *thys, int elem, double v)
{
    if(!thys || !*thys || elem<0)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    (void) ajDoubleResize(thys, elem);

    (*thys)->Ptr[elem] = v;
    
    return ajTrue;
}



/* @funcstatic ajDoubleResize ***************************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajDoubleResize(AjPDouble *thys, int size)
{
    AjPDouble p=NULL;
    int    s;
    int    clen;
    int    limit;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    *thys = ajDoubleNewL(s);
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove((*thys)->Ptr,p->Ptr,limit*sizeof(double));

    (*thys)->Len = size+1;
    

    ajDoubleDel(&p);

    return ajTrue;
}



/* @func ajDoubleDouble *************************************************************
**
** Returns the current double* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [AjPDouble] Source array
** @return [double*] Current array pointer, or a null string if undefined.
** @@
******************************************************************************/
double* ajDoubleDouble(AjPDouble thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}



/* @func ajDoubleLen *************************************************************
**
** Get length of dynamic 1d double array
**
** @param [r] thys [AjPDouble] Source array
** @return [int] length
** @@
******************************************************************************/

int ajDoubleLen(AjPDouble thys)
{
    return thys->Len;
}



/* @func ajShortNew *************************************************************
**
** Default constructor for empty AJAX short arrays.
**
** @return [AjPShort] Pointer to an empty short array structure
** @@
******************************************************************************/

AjPShort ajShortNew(void)
{
    AjPShort thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(short));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;
    
    return thys;
}



/* @func ajShortNewL ************************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size
** @return [AjPShort] Pointer to an empty short array struct of specified size.
** @@
******************************************************************************/

AjPShort ajShortNewL(int size)
{
    AjPShort thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(short));
    thys->Len = 0;
    thys->Res = (int)size;
    
    return thys;
}


/* @func ajShortDel *************************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPShort*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajShortDel(AjPShort *thys)
{

    if(!thys || !*thys)
	return;
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    
    return;
}


/* @func ajShortGet *************************************************************
**
** Retrieve an element from an AJAX short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPShort] Pointer to the short array.
** @param  [r] elem [int] array element.
**
** @return [short] contents of array element
** @@
******************************************************************************/

short ajShortGet(AjPShort thys, int elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad short array index %d\n",elem);
    
    return thys->Ptr[elem];
}


/* @func ajShortPut *************************************************************
**
** Load a short array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort*] Pointer to the int array.
** @param  [r] elem [int] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

AjBool ajShortPut(AjPShort *thys, int elem, short v)
{
    if(!thys || !*thys || elem<0)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    (void) ajShortResize(thys, elem);

    (*thys)->Ptr[elem] = v;
    
    return ajTrue;
}



/* @funcstatic ajShortResize ***************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajShortResize(AjPShort *thys, int size)
{
    AjPShort p=NULL;
    int    s;
    int    clen;
    int    limit;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    *thys = ajShortNewL(s);
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove((*thys)->Ptr,p->Ptr,limit*sizeof(short));

    (*thys)->Len = size+1;
    

    ajShortDel(&p);

    return ajTrue;
}



/* @func ajShortShort *************************************************************
**
** Returns the current short* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [AjPShort] Source array
** @return [short*] Current array pointer, or a null string if undefined.
** @@
******************************************************************************/
short* ajShortShort(AjPShort thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}



/* @func ajShortLen *************************************************************
**
** Get length of dynamic 1d short array
**
** @param [r] thys [AjPShort] Source array
** @return [int] length
** @@
******************************************************************************/

int ajShortLen(AjPShort thys)
{
    return thys->Len;
}



/* @func ajLongNew *************************************************************
**
** Default constructor for empty AJAX long arrays.
**
** @return [AjPLong] Pointer to an empty long array structure
** @@
******************************************************************************/

AjPLong ajLongNew(void)
{
    AjPLong thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(long));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;
    
    return thys;
}



/* @func ajLongNewL ************************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size
** @return [AjPLong] Pointer to an empty long array struct of specified size.
** @@
******************************************************************************/

AjPLong ajLongNewL(int size)
{
    AjPLong thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(long));
    thys->Len = 0;
    thys->Res = (int)size;
    
    return thys;
}


/* @func ajLongDel *************************************************************
**
** Default destructor for AJAX long arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPLong*] Pointer to the long array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajLongDel(AjPLong *thys)
{

    if(!thys || !*thys)
	return;
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    
    return;
}


/* @func ajLongGet *************************************************************
**
** Retrieve an element from an AJAX long array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPLong] Pointer to the long array.
** @param  [r] elem [int] array element.
**
** @return [long] contents of array element
** @@
******************************************************************************/

long ajLongGet(AjPLong thys, int elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad long array index %d\n",elem);
    
    return thys->Ptr[elem];
}


/* @func ajLongPut *************************************************************
**
** Load a long array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong*] Pointer to the int array.
** @param  [r] elem [int] array element.
** @param  [r] v [long] value to load.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

AjBool ajLongPut(AjPLong *thys, int elem, long v)
{
    if(!thys || !*thys || elem<0)
	ajErr("Attempt to write to illegal array value %d\n",elem);

    if(elem < (*thys)->Res)
    {
	if(elem>=(*thys)->Len)
	    (*thys)->Len = elem+1;
	(*thys)->Ptr[elem] = v;
	return ajFalse;
    }

    (void) ajLongResize(thys, elem);

    (*thys)->Ptr[elem] = v;
    
    return ajTrue;
}



/* @funcstatic ajLongResize ***************************************************
**
** Resize a long array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajLongResize(AjPLong *thys, int size)
{
    AjPLong p=NULL;
    int    s;
    int    clen;
    int    limit;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize long array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    *thys = ajLongNewL(s);
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove((*thys)->Ptr,p->Ptr,limit*sizeof(long));

    (*thys)->Len = size+1;
    

    ajLongDel(&p);

    return ajTrue;
}



/* @func ajLongLong *************************************************************
**
** Returns the current long* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [AjPLong] Source array
** @return [long*] Current array pointer, or a null string if undefined.
** @@
******************************************************************************/
long* ajLongLong(AjPLong thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}



/* @func ajLongLen *************************************************************
**
** Get length of dynamic 1d long array
**
** @param [r] thys [AjPLong] Source array
** @return [long] length
** @@
******************************************************************************/

long ajLongLen(AjPLong thys)
{
    return thys->Len;
}



/* @func ajFloatParse ********************************************************
**
** Parses a string into a floating point array.
**
** The array size is already set.
**
** @param [w] str [AjPStr] Input string 
** @param [r] array [AjPFloat*] Array 
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFloatParse (AjPStr str, AjPFloat* array) {

  static AjPRegexp numexp = NULL;
  int i=0;
  float t=0.0;
  
  static AjPStr tmpstr = NULL;
  static AjPStr numstr = NULL;

  if (!numexp)
    numexp = ajRegCompC ("[+-]?[0-9.]+");

  ajStrAssS (&tmpstr, str);

  while (ajRegExec (numexp, tmpstr)) {
    if (i >(*array)->Len) return ajFalse;
    ajRegSubI (numexp, 0, &numstr);
    ajRegPost (numexp, &tmpstr);
    ajDebug ("array [%d] '%S'\n", i, numstr);
    ajStrToFloat (numstr, &t);
    (void) ajFloatPut(array,i,t);
    i++;
  }

  if (i < (*array)->Len) return ajFalse;

  return ajTrue;
}

/* @func ajFloatStr **********************************************************
**
** Writes a floating point array as a string
**
** @param [w] str [AjPStr*] Output string 
** @param [r] array [AjPFloat] Array 
** @param [r] precision [int] floating point precision
** @return [void]
** @@
******************************************************************************/

void ajFloatStr (AjPStr* str, AjPFloat array, int precision) {

  int i;

  for (i=0; i < array->Len; i++) {
    if (i)
      ajStrAppK (str, ' ');
    ajFmtPrintAppS (str, "%.*f", precision, ajFloatGet(array,i));
  }
  return;
}

/* @func ajFloatTrace ********************************************************
**
** Writes a floating point array to the debug file
**
** @param [r] array [AjPFloat] Array 
** @param [r] precision [int] floating point precision
** @param [r] text [char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajFloatTrace (AjPFloat array, int precision, char* text) {

  int i;

  ajDebug ("%s\n", text);
  for (i=0; i < array->Len; i++) {
      ajDebug ("%3d: %.*f\n", i, precision, ajFloatGet(array,i));
  }
  ajDebug ("\n");

  return;
}


/* @func ajArrCommaList ******************************************************
**
** Creates an AjPStr array from a string of comma separated tokens
**
** @param [r] s [AjPStr] Line containing comma separated strings
** @param [w] a [AjPStr **] array pointer to create and load
**
** @return [int] number of array elements created
** @@
******************************************************************************/
int ajArrCommaList(AjPStr s, AjPStr **a)
{
    AjPStr    x;
    AjPStrTok t;
    int n;
    int i;

    
    n = ajStrTokenCount(&s,",\n");
    AJCNEW(*a, n);
    
    x = ajStrNew();
    t = ajStrTokenInit(s,",\n");

    for(i=0;i<n;++i)
    {
	(void) ajStrToken(&x,&t,",\n");
	(*a)[i] = ajStrNewC(ajStrStr(x));
    }

    ajStrDel(&x);
    ajStrTokenClear(&t);

    return n;
}



/* @func  ajArrDoubleLine ****************************************************
**
** Creates a double array from a string of columns
**
** @param [r] line [AjPStr*] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] cols [int] Number of tokens in the string
** @param [r] startcol [int] Start token (1 to n)
** @param [r] endcol [int] End token (1 to n)
** @return [double*] Allocated array of integers
** @@
******************************************************************************/

double* ajArrDoubleLine(AjPStr *line, const char *delim, int cols, 
			int startcol, int endcol) {

    AjPStrTok t=NULL;
    AjPStr tmp=NULL;
    static double *ret;
    int ncols;
    int i;
    
    
    t = ajStrTokenInit(*line, delim);
    tmp = ajStrNew();
    ncols = (endcol-startcol)+1;
    AJCNEW(ret, ncols);

    for(i=0;i<startcol-1;++i)
	if(!ajStrToken(&tmp,&t,delim))
	    ajFatal("Token missing");

    for(i=0;i<ncols;++i)
    {
	if(!ajStrToken(&tmp,&t,delim))
	    ajFatal("Token missing");
	
	if(!ajStrToDouble(tmp,&ret[i]))
	    ajFatal("Bad float conversion");
    }

    ajStrDel(&tmp);
    ajStrTokenClear(&t);

    return ret;
}



/* @func ajArrIntLine ******************************************************
**
** Creates an Int array from a string of columns
**
** @param [r] line [AjPStr*] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] cols [int] Number of tokens in the string
** @param [r] startcol [int] Start token (1 to n)
** @param [r] endcol [int] End token (1 to n)
** @return [int*] Allocated array of integers
** @@
******************************************************************************/

int* ajArrIntLine(AjPStr *line, const char *delim, int cols, 
		  int startcol, int endcol) {

    AjPStrTok t=NULL;
    AjPStr tmp=NULL;
    static int *ret;
    int ncols;
    int i;
    
    
    t = ajStrTokenInit(*line, delim);
    tmp = ajStrNew();
    ncols = (endcol-startcol)+1;
    AJCNEW(ret, ncols);

    for(i=0;i<startcol-1;++i)
	if(!ajStrToken(&tmp,&t,delim))
	    ajFatal("Token missing");

    for(i=0;i<ncols;++i)
    {
	if(!ajStrToken(&tmp,&t,delim))
	    ajFatal("Token missing");
	
	if(!ajStrToInt(tmp,&ret[i]))
	    ajFatal("Bad integer conversion");
    }

    ajStrDel(&tmp);
    ajStrTokenClear(&t);

    return ret;
}


/* @func  ajArrFloatLine ****************************************************
**
** Creates a Float array from a string of columns
**
** @param [r] line [AjPStr*] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] cols [int] Number of tokens in the string
** @param [r] startcol [int] Start token (1 to n)
** @param [r] endcol [int] End token (1 to n)
** @return [float*] Allocated array of integers
** @@
******************************************************************************/

float* ajArrFloatLine(AjPStr *line, const char *delim, int cols, 
		      int startcol, int endcol) {

    AjPStrTok t=NULL;
    AjPStr tmp=NULL;
    static float *ret;
    int ncols;
    int i;
    
    
    t = ajStrTokenInit(*line, delim);
    tmp = ajStrNew();
    ncols = (endcol-startcol)+1;
    AJCNEW(ret, ncols);

    for(i=0;i<startcol-1;++i)
	if(!ajStrToken(&tmp,&t,delim))
	    ajFatal("Token missing");

    for(i=0;i<ncols;++i)
    {
	if(!ajStrToken(&tmp,&t,delim))
	    ajFatal("Token missing");
	
	if(!ajStrToFloat(tmp,&ret[i]))
	    ajFatal("Bad float conversion");
    }

    ajStrDel(&tmp);
    ajStrTokenClear(&t);

    return ret;
}



/* @func ajInt2dNew **********************************************************
**
** Default constructor for empty AJAX 2D integer arrays.
**
** @return [AjPInt2d] Pointer to an empty integer array structure
** @@
******************************************************************************/

AjPInt2d ajInt2dNew(void)
{
    AjPInt2d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPInt*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajInt2dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPInt2d] Pointer to an empty integer 2d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPInt2d ajInt2dNewL(int size)
{
    AjPInt2d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajInt2dDel ***********************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPInt2d*] Pointer to the int array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajInt2dDel(AjPInt2d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajIntDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajInt2dGet *************************************************************
**
** Retrieve an element from an AJAX 2d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPInt2d] Pointer to the int array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
**
** @return [int] contents of array element
** @@
******************************************************************************/

int ajInt2dGet(AjPInt2d thys, int elem1, int elem2)
{
    AjPInt t;
    
    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad int array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);
    
    return ajIntGet(t,elem2);
}



/* @func ajInt2dPut **********************************************************
**
** Load an integer 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt2d*] Pointer to the int array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] v [int] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajInt2dPut(AjPInt2d *thys, int elem1, int elem2, int v)
{
    if(!thys || !*thys || elem1<0 || elem2<0)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajIntNew();
	return ajIntPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    (void) ajInt2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajIntNew();
    
    ajIntPut(&(*thys)->Ptr[elem1],elem2,v);
    
    return ajTrue;
}



/* @funcstatic ajInt2dResize ***************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt2d*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajInt2dResize(AjPInt2d *thys, int size)
{
    AjPInt2d nthys;
    AjPInt2d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPInt*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPInt*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajIntDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajInt2dLen ***************************************************
**
** Get lengths of 2d int array
**
**
** @param  [r] thys [AjPInt2d] Pointer to the int array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajInt2dLen(AjPInt2d thys, int* len1, int* len2)
{
    AjPInt t;
    int i;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}



/* @func ajInt2dInt ***************************************************
**
** Convert AjPInt2d to int**
**
** @param  [r] thys [AjPInt2d] Pointer to the int array.
**
** @return [int**] coverted value.
** @@
******************************************************************************/

int** ajInt2dInt(AjPInt2d thys)
{
    AjPInt t=NULL;
    int **array;
    int d1;
    int d2;
    int i;
    
    ajInt2dLen(thys,&d1,&d2);
    
    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    (void) memmove(array[i],t->Ptr,t->Len*sizeof(int));
    }

    return array;
}



/* @func ajInt3dNew **********************************************************
**
** Default constructor for empty AJAX 3D integer arrays.
**
** @return [AjPInt3d] Pointer to an empty integer array structure
** @@
******************************************************************************/

AjPInt3d ajInt3dNew(void)
{
    AjPInt3d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPInt2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajInt3dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPInt3d] Pointer to an empty integer 3d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPInt3d ajInt3dNewL(int size)
{
    AjPInt3d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt2d*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajInt3dDel ***********************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPInt3d*] Pointer to the int array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajInt3dDel(AjPInt3d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajInt2dDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajInt3dGet *************************************************************
**
** Retrieve an element from an AJAX 3d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPInt3d] Pointer to the int array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
**
** @return [int] contents of array element
** @@
******************************************************************************/

int ajInt3dGet(AjPInt3d thys, int elem1, int elem2, int elem3)
{
    AjPInt2d t;
    
    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad int array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);
    
    return ajInt2dGet(t,elem2,elem3);
}



/* @func ajInt3dPut **********************************************************
**
** Load an integer 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt3d*] Pointer to the int array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
** @param  [r] v [int] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajInt3dPut(AjPInt3d *thys, int elem1, int elem2, int elem3, int v)
{
    if(!thys || !*thys || elem1<0 || elem2<0 || elem3<0)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajInt2dNew();
	return ajInt2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    (void) ajInt3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajInt2dNew();
    
    ajInt2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    
    return ajTrue;
}



/* @funcstatic ajInt3dResize ***************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt3d*] Pointer to the int array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajInt3dResize(AjPInt3d *thys, int size)
{
    AjPInt3d nthys;
    AjPInt3d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPInt2d*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPInt2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajInt2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajInt3dLen ***************************************************
**
** Get lengths of 3d int array
**
**
** @param  [r] thys [AjPInt3d] Pointer to the int array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
** @param  [w] len3 [int*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajInt3dLen(AjPInt3d thys, int* len1, int* len2, int* len3)
{
    AjPInt2d t2;
    AjPInt   t1;
    int i;
    int j;
    int v;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3=0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}
    
    return;
}



/* @func ajInt3dInt ***************************************************
**
** Convert AjPInt3d to int***
**
** @param  [r] thys [AjPInt3d] Pointer to the int array.
**
** @return [int***] converted values.
** @@
******************************************************************************/

int*** ajInt3dInt(AjPInt3d thys)
{
    AjPInt2d t2=NULL;
    AjPInt   t1=NULL;
    int ***array;
    int d1;
    int d2;
    int d3;
    int i;
    int j;
    
    ajInt3dLen(thys,&d1,&d2,&d3);
    
    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];
	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len) continue;
		if((t1=t2->Ptr[j]))
		    (void) memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(int));
	    }
	}
    }

    return array;
}



/* @func ajFloat2dNew **********************************************************
**
** Default constructor for empty AJAX 2D float arrays.
**
** @return [AjPFloat2d] Pointer to an empty float array structure
** @@
******************************************************************************/

AjPFloat2d ajFloat2dNew(void)
{
    AjPFloat2d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPFloat*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajFloat2dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPFloat2d] Pointer to an empty float 2d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPFloat2d ajFloat2dNewL(int size)
{
    AjPFloat2d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPFloat*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajFloat2dDel ***********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFloat2dDel(AjPFloat2d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajFloatDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajFloat2dGet *************************************************************
**
** Retrieve an element from an AJAX 2d float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPFloat2d] Pointer to the float array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
**
** @return [float] contents of array element
** @@
******************************************************************************/

float ajFloat2dGet(AjPFloat2d thys, int elem1, int elem2)
{
    AjPFloat t;
    
    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad float array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);
    
    return ajFloatGet(t,elem2);
}



/* @func ajFloat2dPut **********************************************************
**
** Load a float 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajFloat2dPut(AjPFloat2d *thys, int elem1, int elem2, float v)
{
    if(!thys || !*thys || elem1<0 || elem2<0)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajFloatNew();
	return ajFloatPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    (void) ajFloat2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajFloatNew();
    
    ajFloatPut(&(*thys)->Ptr[elem1],elem2,v);
    
    return ajTrue;
}



/* @funcstatic ajFloat2dResize ***************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajFloat2dResize(AjPFloat2d *thys, int size)
{
    AjPFloat2d nthys;
    AjPFloat2d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPFloat*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPFloat*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajFloatDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajFloat2dLen ***************************************************
**
** Get lengths of 2d float array
**
**
** @param  [r] thys [AjPFloat2d] Pointer to the float array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
**
** @return [void] 
** @@
******************************************************************************/

void ajFloat2dLen(AjPFloat2d thys, int* len1, int* len2)
{
    AjPFloat t;
    int i;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}



/* @func ajFloat2dFloat ***************************************************
**
** Convert AjPFloat2d to float**
**
** @param  [r] thys [AjPFloat2d] Pointer to the float array.
**
** @return [float**] converted values.
** @@
******************************************************************************/

float** ajFloat2dFloat(AjPFloat2d thys)
{
    AjPFloat t=NULL;
    float **array;
    int d1;
    int d2;
    int i;
    
    ajFloat2dLen(thys,&d1,&d2);
    
    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    (void) memmove(array[i],t->Ptr,t->Len*sizeof(float));
    }

    return array;
}



/* @func ajFloat3dNew **********************************************************
**
** Default constructor for empty AJAX 3D float arrays.
**
** @return [AjPFloat3d] Pointer to an empty float array structure
** @@
******************************************************************************/

AjPFloat3d ajFloat3dNew(void)
{
    AjPFloat3d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPFloat2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajFloat3dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPFloat3d] Pointer to an empty float 3d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPFloat3d ajFloat3dNewL(int size)
{
    AjPFloat3d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPFloat2d*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajFloat3dDel ***********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFloat3dDel(AjPFloat3d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajFloat2dDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajFloat3dGet *************************************************************
**
** Retrieve an element from an AJAX 3d float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPFloat3d] Pointer to the float array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
**
** @return [float] contents of array element
** @@
******************************************************************************/

float ajFloat3dGet(AjPFloat3d thys, int elem1, int elem2, int elem3)
{
    AjPFloat2d t;
    
    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad float array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);
    
    return ajFloat2dGet(t,elem2,elem3);
}



/* @func ajFloat3dPut **********************************************************
**
** Load a float 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajFloat3dPut(AjPFloat3d *thys, int elem1, int elem2, int elem3, float v)
{
    if(!thys || !*thys || elem1<0 || elem2<0 || elem3<0)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajFloat2dNew();
	return ajFloat2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    (void) ajFloat3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajFloat2dNew();
    
    ajFloat2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    
    return ajTrue;
}



/* @funcstatic ajFloat3dResize ***************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajFloat3dResize(AjPFloat3d *thys, int size)
{
    AjPFloat3d nthys;
    AjPFloat3d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPFloat2d*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPFloat2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajFloat2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajFloat3dLen ***************************************************
**
** Get lengths of 3d float array
**
**
** @param  [r] thys [AjPFloat3d] Pointer to the float array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
** @param  [w] len3 [int*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajFloat3dLen(AjPFloat3d thys, int* len1, int* len2, int* len3)
{
    AjPFloat2d t2;
    AjPFloat   t1;
    int i;
    int j;
    int v;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3=0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}
    
    return;
}



/* @func ajFloat3dFloat ***************************************************
**
** Convert AjPFloat3d to float***
**
** @param  [r] thys [AjPFloat3d] Pointer to the float array.
**
** @return [float***] converted values.
** @@
******************************************************************************/

float*** ajFloat3dFloat(AjPFloat3d thys)
{
    AjPFloat2d t2=NULL;
    AjPFloat   t1=NULL;
    float ***array;
    int d1;
    int d2;
    int d3;
    int i;
    int j;
    
    ajFloat3dLen(thys,&d1,&d2,&d3);
    
    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];
	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len) continue;
		if((t1=t2->Ptr[j]))
		    (void) memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(float));
	    }
	}
    }

    return array;
}



/* @func ajDouble2dNew **********************************************************
**
** Default constructor for empty AJAX 2D double arrays.
**
** @return [AjPDouble2d] Pointer to an empty double array structure
** @@
******************************************************************************/

AjPDouble2d ajDouble2dNew(void)
{
    AjPDouble2d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPDouble*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajDouble2dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPDouble2d] Pointer to an empty double 2d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPDouble2d ajDouble2dNewL(int size)
{
    AjPDouble2d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPDouble*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajDouble2dDel ***********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajDouble2dDel(AjPDouble2d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajDoubleDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajDouble2dGet *************************************************************
**
** Retrieve an element from an AJAX 2d double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPDouble2d] Pointer to the double array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
**
** @return [double] contents of array element
** @@
******************************************************************************/

double ajDouble2dGet(AjPDouble2d thys, int elem1, int elem2)
{
    AjPDouble t;
    
    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad double array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);
    
    return ajDoubleGet(t,elem2);
}



/* @func ajDouble2dPut **********************************************************
**
** Load a double 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajDouble2dPut(AjPDouble2d *thys, int elem1, int elem2, double v)
{
    if(!thys || !*thys || elem1<0 || elem2<0)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajDoubleNew();
	return ajDoublePut(&(*thys)->Ptr[elem1],elem2,v);
    }

    (void) ajDouble2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajDoubleNew();
    
    ajDoublePut(&(*thys)->Ptr[elem1],elem2,v);
    
    return ajTrue;
}



/* @funcstatic ajDouble2dResize ***************************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajDouble2dResize(AjPDouble2d *thys, int size)
{
    AjPDouble2d nthys;
    AjPDouble2d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPDouble*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPDouble*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajDoubleDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajDouble2dLen ***************************************************
**
** Get lengths of 2d double array
**
**
** @param  [r] thys [AjPDouble2d] Pointer to the double array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajDouble2dLen(AjPDouble2d thys, int* len1, int* len2)
{
    AjPDouble t;
    int i;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}



/* @func ajDouble2dDouble ***************************************************
**
** Convert AjPDouble2d to double**
**
** @param  [r] thys [AjPDouble2d] Pointer to the double array.
**
** @return [double**] converted values.
** @@
******************************************************************************/

double** ajDouble2dDouble(AjPDouble2d thys)
{
    AjPDouble t=NULL;
    double **array;
    int d1;
    int d2;
    int i;
    
    ajDouble2dLen(thys,&d1,&d2);
    
    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    (void) memmove(array[i],t->Ptr,t->Len*sizeof(double));
    }

    return array;
}



/* @func ajDouble3dNew **********************************************************
**
** Default constructor for empty AJAX 3D double arrays.
**
** @return [AjPDouble3d] Pointer to an empty double array structure
** @@
******************************************************************************/

AjPDouble3d ajDouble3dNew(void)
{
    AjPDouble3d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPDouble2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajDouble3dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPDouble3d] Pointer to an empty double 3d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPDouble3d ajDouble3dNewL(int size)
{
    AjPDouble3d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPDouble2d*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajDouble3dDel ***********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajDouble3dDel(AjPDouble3d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajDouble2dDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajDouble3dGet *************************************************************
**
** Retrieve an element from an AJAX 3d double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPDouble3d] Pointer to the double array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
**
** @return [double] contents of array element
** @@
******************************************************************************/

double ajDouble3dGet(AjPDouble3d thys, int elem1, int elem2, int elem3)
{
    AjPDouble2d t;
    
    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad double array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);
    
    return ajDouble2dGet(t,elem2,elem3);
}



/* @func ajDouble3dPut **********************************************************
**
** Load a double 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajDouble3dPut(AjPDouble3d *thys, int elem1, int elem2, int elem3, double v)
{
    if(!thys || !*thys || elem1<0 || elem2<0 || elem3<0)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajDouble2dNew();
	return ajDouble2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    (void) ajDouble3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajDouble2dNew();
    
    ajDouble2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    
    return ajTrue;
}



/* @funcstatic ajDouble3dResize ***************************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajDouble3dResize(AjPDouble3d *thys, int size)
{
    AjPDouble3d nthys;
    AjPDouble3d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPDouble2d*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPDouble2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajDouble2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajDouble3dLen ***************************************************
**
** Get lengths of 3d double array
**
**
** @param  [r] thys [AjPDouble3d] Pointer to the double array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
** @param  [w] len3 [int*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajDouble3dLen(AjPDouble3d thys, int* len1, int* len2, int* len3)
{
    AjPDouble2d t2;
    AjPDouble   t1;
    int i;
    int j;
    int v;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3=0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}
    
    return;
}



/* @func ajDouble3dDouble ***************************************************
**
** Convert AjPDouble3d to double***
**
** @param  [r] thys [AjPDouble3d] Pointer to the double array.
**
** @return [double***] converted values.
** @@
******************************************************************************/

double*** ajDouble3dDouble(AjPDouble3d thys)
{
    AjPDouble2d t2=NULL;
    AjPDouble   t1=NULL;
    double ***array;
    int d1;
    int d2;
    int d3;
    int i;
    int j;
    
    ajDouble3dLen(thys,&d1,&d2,&d3);
    
    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];
	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len) continue;
		if((t1=t2->Ptr[j]))
		    (void) memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(double));
	    }
	}
    }

    return array;
}



/* @func ajShort2dNew **********************************************************
**
** Default constructor for empty AJAX 2D short arrays.
**
** @return [AjPShort2d] Pointer to an empty short array structure
** @@
******************************************************************************/

AjPShort2d ajShort2dNew(void)
{
    AjPShort2d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPShort*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajShort2dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPShort2d] Pointer to an empty short 2d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPShort2d ajShort2dNewL(int size)
{
    AjPShort2d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPShort*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajShort2dDel ***********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajShort2dDel(AjPShort2d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajShortDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajShort2dGet *************************************************************
**
** Retrieve an element from an AJAX 2d short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPShort2d] Pointer to the short array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
**
** @return [short] contents of array element
** @@
******************************************************************************/

short ajShort2dGet(AjPShort2d thys, int elem1, int elem2)
{
    AjPShort t;
    
    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad short array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);
    
    return ajShortGet(t,elem2);
}



/* @func ajShort2dPut **********************************************************
**
** Load a short 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajShort2dPut(AjPShort2d *thys, int elem1, int elem2, short v)
{
    if(!thys || !*thys || elem1<0 || elem2<0)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajShortNew();
	return ajShortPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    (void) ajShort2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajShortNew();
    
    ajShortPut(&(*thys)->Ptr[elem1],elem2,v);
    
    return ajTrue;
}



/* @funcstatic ajShort2dResize ***************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajShort2dResize(AjPShort2d *thys, int size)
{
    AjPShort2d nthys;
    AjPShort2d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPShort*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPShort*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajShortDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajShort2dLen ***************************************************
**
** Get lengths of 2d short array
**
**
** @param  [r] thys [AjPShort2d] Pointer to the short array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajShort2dLen(AjPShort2d thys, int* len1, int* len2)
{
    AjPShort t;
    int i;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}



/* @func ajShort2dShort ***************************************************
**
** Convert AjPShort2d to short**
**
** @param  [r] thys [AjPShort2d] Pointer to the short array.
**
** @return [short**] converted values
** @@
******************************************************************************/

short** ajShort2dShort(AjPShort2d thys)
{
    AjPShort t=NULL;
    short **array;
    int d1;
    int d2;
    int i;
    
    ajShort2dLen(thys,&d1,&d2);
    
    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    (void) memmove(array[i],t->Ptr,t->Len*sizeof(short));
    }

    return array;
}



/* @func ajShort3dNew **********************************************************
**
** Default constructor for empty AJAX 3D short arrays.
**
** @return [AjPShort3d] Pointer to an empty short array structure
** @@
******************************************************************************/

AjPShort3d ajShort3dNew(void)
{
    AjPShort3d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPShort2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajShort3dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPShort3d] Pointer to an empty short 3d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPShort3d ajShort3dNewL(int size)
{
    AjPShort3d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPShort2d*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajShort3dDel ***********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajShort3dDel(AjPShort3d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajShort2dDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajShort3dGet *************************************************************
**
** Retrieve an element from an AJAX 3d short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPShort3d] Pointer to the short array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
**
** @return [short] contents of array element
** @@
******************************************************************************/

short ajShort3dGet(AjPShort3d thys, int elem1, int elem2, int elem3)
{
    AjPShort2d t;
    
    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad short array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);
    
    return ajShort2dGet(t,elem2,elem3);
}



/* @func ajShort3dPut **********************************************************
**
** Load a short 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajShort3dPut(AjPShort3d *thys, int elem1, int elem2, int elem3, short v)
{
    if(!thys || !*thys || elem1<0 || elem2<0 || elem3<0)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajShort2dNew();
	return ajShort2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    (void) ajShort3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajShort2dNew();
    
    ajShort2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    
    return ajTrue;
}



/* @funcstatic ajShort3dResize ***************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajShort3dResize(AjPShort3d *thys, int size)
{
    AjPShort3d nthys;
    AjPShort3d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPShort2d*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPShort2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajShort2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajShort3dLen ***************************************************
**
** Get lengths of 3d short array
**
**
** @param  [r] thys [AjPShort3d] Pointer to the short array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
** @param  [w] len3 [int*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajShort3dLen(AjPShort3d thys, int* len1, int* len2, int* len3)
{
    AjPShort2d t2;
    AjPShort   t1;
    int i;
    int j;
    int v;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3=0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}
    
    return;
}



/* @func ajShort3dShort ***************************************************
**
** Convert AjPShort3d to short***
**
** @param  [r] thys [AjPShort3d] Pointer to the short array.
**
** @return [short***] converted values.
** @@
******************************************************************************/

short*** ajShort3dShort(AjPShort3d thys)
{
    AjPShort2d t2=NULL;
    AjPShort   t1=NULL;
    short ***array;
    int d1;
    int d2;
    int d3;
    int i;
    int j;
    
    ajShort3dLen(thys,&d1,&d2,&d3);
    
    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];
	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len) continue;
		if((t1=t2->Ptr[j]))
		    (void) memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(short));
	    }
	}
    }

    return array;
}



/* @func ajLong2dNew **********************************************************
**
** Default constructor for empty AJAX 2D long arrays.
**
** @return [AjPLong2d] Pointer to an empty long array structure
** @@
******************************************************************************/

AjPLong2d ajLong2dNew(void)
{
    AjPLong2d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPLong*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajLong2dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPLong2d] Pointer to an empty long 2d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPLong2d ajLong2dNewL(int size)
{
    AjPLong2d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPLong*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajLong2dDel ***********************************************************
**
** Default destructor for AJAX long arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPLong2d*] Pointer to the long array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajLong2dDel(AjPLong2d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajLongDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajLong2dGet *************************************************************
**
** Retrieve an element from an AJAX 2d long array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPLong2d] Pointer to the long array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
**
** @return [long] contents of array element
** @@
******************************************************************************/

long ajLong2dGet(AjPLong2d thys, int elem1, int elem2)
{
    AjPLong t;
    
    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad long array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);
    
    return ajLongGet(t,elem2);
}



/* @func ajLong2dPut **********************************************************
**
** Load a long 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong2d*] Pointer to the long array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] v [long] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajLong2dPut(AjPLong2d *thys, int elem1, int elem2, long v)
{
    if(!thys || !*thys || elem1<0 || elem2<0)
	ajErr("Attempt to write to illegal array value [%d][%d]\n",elem1,
	      elem2);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajLongNew();
	return ajLongPut(&(*thys)->Ptr[elem1],elem2,v);
    }

    (void) ajLong2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajLongNew();
    
    ajLongPut(&(*thys)->Ptr[elem1],elem2,v);
    
    return ajTrue;
}



/* @funcstatic ajLong2dResize ***************************************************
**
** Resize a long array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong2d*] Pointer to the long array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajLong2dResize(AjPLong2d *thys, int size)
{
    AjPLong2d nthys;
    AjPLong2d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize long array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPLong*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPLong*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajLongDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajLong2dLen ***************************************************
**
** Get lengths of 2d long array
**
**
** @param  [r] thys [AjPLong2d] Pointer to the long array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajLong2dLen(AjPLong2d thys, int* len1, int* len2)
{
    AjPLong t;
    int i;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}



/* @func ajLong2dLong ***************************************************
**
** Convert AjPLong2d to long**
**
** @param  [r] thys [AjPLong2d] Pointer to the long array.
**
** @return [long**] converted values.
** @@
******************************************************************************/

long** ajLong2dLong(AjPLong2d thys)
{
    AjPLong t=NULL;
    long **array;
    int d1;
    int d2;
    int i;
    
    ajLong2dLen(thys,&d1,&d2);
    
    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    (void) memmove(array[i],t->Ptr,t->Len*sizeof(long));
    }

    return array;
}



/* @func ajLong3dNew **********************************************************
**
** Default constructor for empty AJAX 3D long arrays.
**
** @return [AjPLong3d] Pointer to an empty long array structure
** @@
******************************************************************************/

AjPLong3d ajLong3dNew(void)
{
    AjPLong3d thys;
    int    i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPLong2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;
    
    return thys;
}



/* @func ajLong3dNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [int] Reserved size 1st dim
** @return [AjPLong3d] Pointer to an empty long 3d array struct of
**                    specified size.
** @@
******************************************************************************/

AjPLong3d ajLong3dNewL(int size)
{
    AjPLong3d thys;
    int i;
    

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPLong2d*));
    thys->Len = 0;
    thys->Res = (int)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}



/* @func ajLong3dDel ***********************************************************
**
** Default destructor for AJAX long arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [w] thys [AjPLong3d*] Pointer to the long array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajLong3dDel(AjPLong3d *thys)
{
    int i;

    if(!thys || !*thys)
	return;

    for(i=(*thys)->Res-1;i>-1;--i)
	if((*thys)->Ptr[i])
	    ajLong2dDel(&((*thys)->Ptr[i]));
    
    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;
    return;
}



/* @func ajLong3dGet *************************************************************
**
** Retrieve an element from an AJAX 3d long array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [AjPLong3d] Pointer to the long array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
**
** @return [long] contents of array element
** @@
******************************************************************************/

long ajLong3dGet(AjPLong3d thys, int elem1, int elem2, int elem3)
{
    AjPLong2d t;
    
    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad long array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);
    
    return ajLong2dGet(t,elem2,elem3);
}



/* @func ajLong3dPut **********************************************************
**
** Load a long 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong3d*] Pointer to the long array.
** @param  [r] elem1 [int] array element.
** @param  [r] elem2 [int] array element.
** @param  [r] elem3 [int] array element.
** @param  [r] v [long] value to load.
**
** @return [AjBool] true if any array was extended.
** @@
******************************************************************************/

AjBool ajLong3dPut(AjPLong3d *thys, int elem1, int elem2, int elem3, long v)
{
    if(!thys || !*thys || elem1<0 || elem2<0 || elem3<0)
	ajErr("Attempt to write to illegal array value [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    if(elem1 < (*thys)->Res)
    {
	if(elem1>=(*thys)->Len)
	    (*thys)->Len = elem1+1;
	if(!(*thys)->Ptr[elem1])
	    (*thys)->Ptr[elem1] = ajLong2dNew();
	return ajLong2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    }

    (void) ajLong3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajLong2dNew();
    
    ajLong2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);
    
    return ajTrue;
}



/* @funcstatic ajLong3dResize ***************************************************
**
** Resize a long array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong3d*] Pointer to the long array.
** @param  [r] size [int] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajLong3dResize(AjPLong3d *thys, int size)
{
    AjPLong3d nthys;
    AjPLong3d p=NULL;
    int    s;
    int    clen;
    int    limit;
    int    i;
    

    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize long array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;
    
    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPLong2d*));
    nthys->Res = s;
    
    if((int)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;
    
    (void) memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPLong2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajLong2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;
    
    return ajTrue;
}



/* @func ajLong3dLen ***************************************************
**
** Get lengths of 3d long array
**
**
** @param  [r] thys [AjPLong3d] Pointer to the long array.
** @param  [w] len1 [int*] Length of 1st dim
** @param  [w] len2 [int*] Length of 2nd dim
** @param  [w] len3 [int*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajLong3dLen(AjPLong3d thys, int* len1, int* len2, int* len3)
{
    AjPLong2d t2;
    AjPLong   t1;
    int i;
    int j;
    int v;
    
    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3=0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	{
	    v = t2->Len;
	    for(j=0;j<v;++j)
		if((t1=t2->Ptr[j]))
		    *len3 = (*len3 > t1->Len) ? *len3 : t1->Len;
	}
    
    return;
}



/* @func ajLong3dLong ***************************************************
**
** Convert AjPLong3d to long***
**
** @param  [r] thys [AjPLong3d] Pointer to the long array.
**
** @return [long***] converted values.
** @@
******************************************************************************/

long*** ajLong3dLong(AjPLong3d thys)
{
    AjPLong2d t2=NULL;
    AjPLong   t1=NULL;
    long ***array;
    int d1;
    int d2;
    int d3;
    int i;
    int j;
    
    ajLong3dLen(thys,&d1,&d2,&d3);
    
    AJCNEW0(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	t2 = thys->Ptr[i];
	for(j=0;j<d2;++j)
	{
	    AJCNEW0(array[i][j],d3);
	    if(t2)
	    {
		if(j>=t2->Len) continue;
		if((t1=t2->Ptr[j]))
		    (void) memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(long));
	    }
	}
    }

    return array;
}



/******************************************************************************
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
******************************************************************************/


#include "ajax.h"
#include <stdio.h>
#include <sys/types.h>
#include <string.h>

#ifndef HAVE_MEMMOVE
static void* memmove (void *dst, const void* src, size_t len)
{
    return (void *)bcopy (src, dst, len);
}
#endif

#define RESERVED_SIZE 32

static AjBool ajChararrResize(AjPChar *thys, ajint elem);
static AjBool ajIntResize(AjPInt *thys, ajint elem);
static AjBool ajInt2dResize(AjPInt2d *thys, ajint elem);
static AjBool ajInt3dResize(AjPInt3d *thys, ajint elem);
static AjBool ajFloatResize(AjPFloat *thys, ajint elem);
static AjBool ajFloat2dResize(AjPFloat2d *thys, ajint elem);
static AjBool ajFloat3dResize(AjPFloat3d *thys, ajint elem);
static AjBool ajDoubleResize(AjPDouble *thys, ajint elem);
static AjBool ajDouble2dResize(AjPDouble2d *thys, ajint elem);
static AjBool ajDouble3dResize(AjPDouble3d *thys, ajint elem);
static AjBool ajShortResize(AjPShort *thys, ajint elem);
static AjBool ajShort2dResize(AjPShort2d *thys, ajint elem);
static AjBool ajShort3dResize(AjPShort3d *thys, ajint elem);
static AjBool ajLongResize(AjPLong *thys, ajint elem);
static AjBool ajLong2dResize(AjPLong2d *thys, ajint elem);
static AjBool ajLong3dResize(AjPLong3d *thys, ajint elem);



/* @func ajChararrNew *********************************************************
**
** Default constructor for empty AJAX character arrays.
**
** @return [AjPChar] Pointer to an empty character array structure
** @category new [AjPChar] Default constructor
** @@
******************************************************************************/

AjPChar ajChararrNew(void)
{
    AjPChar thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(char));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    return thys;
}




/* @func ajChararrNewL ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size
** @return [AjPChar] Pointer to an empty character array struct
**                   of specified size.
** @category new [AjPChar] Constructor with reserved size
** @@
******************************************************************************/

AjPChar ajChararrNewL(ajint size)
{
    AjPChar thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(char));
    thys->Len = 0;
    thys->Res = (ajint)size;

    return thys;
}




/* @func ajChararrDel *********************************************************
**
** Default destructor for AJAX character arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPChar*] Pointer to the char array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPChar] Default destructor
** @@
******************************************************************************/

void ajChararrDel(AjPChar *thys)
{
    if(!thys || !*thys)
	return;

    AJFREE((*thys)->Ptr);
    AJFREE(*thys);

    *thys = NULL;

    return;
}




/* @func ajChararrGet *********************************************************
**
** Retrieve an element from an AJAX character array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPChar] Pointer to the char array.
** @param  [r] elem [ajint] array element.
**
** @return [char] contents of array element
** @category cast [AjPChar] Retrieve a character from an array
** @@
******************************************************************************/

char ajChararrGet(const AjPChar thys, ajint elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad char array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajChararrPut *********************************************************
**
** Load a character array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPChar*] Pointer to the char array.
** @param  [r] elem [ajint] array element.
** @param  [r] v [char] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPChar] Load a character array element
** @@
******************************************************************************/

AjBool ajChararrPut(AjPChar *thys, ajint elem, char v)
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

    ajChararrResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @func ajChararrChararr *****************************************************
**
** Returns the current char* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPChar] Source array
** @return [char*] Current array pointer, or a null string if undefined.
** @category cast [AjPChar] Retrieve internal pointer
** @@
******************************************************************************/

char* ajChararrChararr(const AjPChar thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajIntNew *************************************************************
**
** Default constructor for empty AJAX integer arrays.
**
** @return [AjPInt] Pointer to an empty integer array structure
** @category new [AjPInt] Default constructor
** @@
******************************************************************************/

AjPInt ajIntNew(void)
{
    AjPInt thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(ajint));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    return thys;
}




/* @func ajIntNewL ************************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size
** @return [AjPInt] Pointer to an empty integer array struct of specified size.
** @category new [AjPInt] Constructor with reserved size
** @@
******************************************************************************/

AjPInt ajIntNewL(ajint size)
{
    AjPInt thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(ajint));
    thys->Len = 0;
    thys->Res = (ajint)size;

    return thys;
}




/* @func ajIntDel *************************************************************
**
** Default destructor for AJAX integer arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPInt*] Pointer to the ajint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPInt] Default destructor
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
** @param  [r] thys [const AjPInt] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
**
** @return [ajint] contents of array element
** @category cast [AjPInt] Retrieve an integer from an array
** @@
******************************************************************************/

ajint ajIntGet(const AjPInt thys, ajint elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad ajint array index %d\n",elem);

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
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
** @param  [r] v [ajint] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPInt] Load an integer array element
** @@
******************************************************************************/

AjBool ajIntPut(AjPInt *thys, ajint elem, ajint v)
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

    ajIntResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @func ajIntInc *************************************************************
**
** Increment an integer array element.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
**
** @return [void]
** @@
******************************************************************************/

void ajIntInc(AjPInt *thys, ajint elem)
{
    if(!thys || !*thys || elem<0 || elem>(*thys)->Len)
	ajErr("Attempt to write to illegal array value %d\n",elem);


    ++(*thys)->Ptr[elem];

    return;
}




/* @func ajIntDec *************************************************************
**
** Deccrement an integer array element.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
**
** @return [void]
** @@
******************************************************************************/

void ajIntDec(AjPInt *thys, ajint elem)
{
    if(!thys || !*thys || elem<0 || elem>(*thys)->Len)
	ajErr("Attempt to write to illegal array value %d\n",elem);


    --(*thys)->Ptr[elem];

    return;
}




/* @funcstatic ajChararrResize ************************************************
**
** Resize a character array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPChar*] Pointer to the char array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajChararrResize(AjPChar *thys, ajint size)
{
    AjPChar p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize character array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    *thys = ajChararrNewL(s);

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(char));

    (*thys)->Len = size+1;


    ajChararrDel(&p);

    return ajTrue;
}




/* @funcstatic ajIntResize ****************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajIntResize(AjPInt *thys, ajint size)
{
    AjPInt p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    *thys = ajIntNewL(s);

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(ajint));

    (*thys)->Len = size+1;


    ajIntDel(&p);

    return ajTrue;
}




/* @func ajIntInt *************************************************************
**
** Returns the current ajint* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPInt] Source array
** @return [ajint*] Current array pointer, or a null string if undefined.
** @category cast [AjPInt] Retrieve internal pointer
** @@
******************************************************************************/

ajint* ajIntInt(const AjPInt thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajIntLen *************************************************************
**
** Get length of dynamic 1d ajint array
**
** @param [r] thys [const AjPInt] Source array
** @return [ajint] length
** @@
******************************************************************************/

ajint ajIntLen(const AjPInt thys)
{
    return thys->Len;
}




/* @func ajFloatNew ***********************************************************
**
** Default constructor for empty AJAX float arrays.
**
** @return [AjPFloat] Pointer to an empty float array structure
** @category new [AjPFloat] Default constructor
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




/* @func ajFloatNewL **********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size
** @return [AjPFloat] Pointer to an empty float array struct of specified size.
** @category new [AjPFloat] Constructor with reserved size
** @@
******************************************************************************/

AjPFloat ajFloatNewL(ajint size)
{
    AjPFloat thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(float));
    thys->Len = 0;
    thys->Res = (ajint)size;

    return thys;
}




/* @func ajFloatDel ***********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPFloat*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFloat] Default destructor
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




/* @func ajFloatGet ***********************************************************
**
** Retrieve an element from an AJAX float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPFloat] Pointer to the float array.
** @param  [r] elem [ajint] array element.
**
** @return [float] contents of array element
** @category cast [AjPFloat] Retrieve a float from an array
** @@
******************************************************************************/

float ajFloatGet(const AjPFloat thys, ajint elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad float array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajFloatPut ***********************************************************
**
** Load a float array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPFloat] Load a float array element
** @@
******************************************************************************/

AjBool ajFloatPut(AjPFloat *thys, ajint elem, float v)
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

    ajFloatResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}





/* @funcstatic ajFloatResize **************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajFloatResize(AjPFloat *thys, ajint size)
{
    AjPFloat p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    *thys = ajFloatNewL(s);

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(float));

    (*thys)->Len = size+1;


    ajFloatDel(&p);

    return ajTrue;
}




/* @func ajFloatFloat *********************************************************
**
** Returns the current float* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPFloat] Source array
** @return [float*] Current array pointer, or a null string if undefined.
** @category cast [AjPFloat] Retrieve internal pointer
** @@
******************************************************************************/

float* ajFloatFloat(const AjPFloat thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajFloatLen ***********************************************************
**
** Get length of dynamic 1d float array
**
** @param [r] thys [const AjPFloat] Source array
** @return [ajint] length
** @@
******************************************************************************/

ajint ajFloatLen(const AjPFloat thys)
{
    return thys->Len;
}




/* @func ajDoubleNew **********************************************************
**
** Default constructor for empty AJAX double arrays.
**
** @return [AjPDouble] Pointer to an empty double array structure
** @category new [AjPDouble] Default constructor
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




/* @func ajDoubleNewL *********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size
** @return [AjPDouble] Pointer to an empty double array struct
**                     of specified size.
** @category new [AjPDouble] Constructor with reserved size
** @@
******************************************************************************/

AjPDouble ajDoubleNewL(ajint size)
{
    AjPDouble thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(double));
    thys->Len = 0;
    thys->Res = (ajint)size;

    return thys;
}




/* @func ajDoubleDel **********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPDouble*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPDouble] Default destructor
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




/* @func ajDoubleGet **********************************************************
**
** Retrieve an element from an AJAX double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPDouble] Pointer to the double array.
** @param  [r] elem [ajint] array element.
**
** @return [double] contents of array element
** @category cast [AjPDouble] Retrieve a double from an array
** @@
******************************************************************************/

double ajDoubleGet(const AjPDouble thys, ajint elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad double array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajDoublePut **********************************************************
**
** Load a double array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPDouble] Load a double array element
** @@
******************************************************************************/

AjBool ajDoublePut(AjPDouble *thys, ajint elem, double v)
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

    ajDoubleResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @funcstatic ajDoubleResize *************************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajDoubleResize(AjPDouble *thys, ajint size)
{
    AjPDouble p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize double array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    *thys = ajDoubleNewL(s);

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(double));

    (*thys)->Len = size+1;


    ajDoubleDel(&p);

    return ajTrue;
}




/* @func ajDoubleDouble *******************************************************
**
** Returns the current double* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPDouble] Source array
** @return [double*] Current array pointer, or a null string if undefined.
** @category cast [AjPDouble] Retrieve internal pointer
** @@
******************************************************************************/

double* ajDoubleDouble(const AjPDouble thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajDoubleLen **********************************************************
**
** Get length of dynamic 1d double array
**
** @param [r] thys [const AjPDouble] Source array
** @return [ajint] length
** @@
******************************************************************************/

ajint ajDoubleLen(const AjPDouble thys)
{
    return thys->Len;
}




/* @func ajShortNew ***********************************************************
**
** Default constructor for empty AJAX short arrays.
**
** @return [AjPShort] Pointer to an empty short array structure
** @category new [AjPShort] Default constructor
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




/* @func ajShortNewL **********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size
** @return [AjPShort] Pointer to an empty short array struct of specified size.
** @category new [AjPShort] Constructor with reserved size
** @@
******************************************************************************/

AjPShort ajShortNewL(ajint size)
{
    AjPShort thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(short));
    thys->Len = 0;
    thys->Res = (ajint)size;

    return thys;
}




/* @func ajShortDel ***********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPShort*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPShort] Default destructor
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




/* @func ajShortGet ***********************************************************
**
** Retrieve an element from an AJAX short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPShort] Pointer to the short array.
** @param  [r] elem [ajint] array element.
**
** @return [short] contents of array element
** @category cast [AjPShort] Retrieve a short from an array
** @@
******************************************************************************/

short ajShortGet(const AjPShort thys, ajint elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad short array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajShortPut ***********************************************************
**
** Load a short array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPShort] Load a short array element
** @@
******************************************************************************/

AjBool ajShortPut(AjPShort *thys, ajint elem, short v)
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

    ajShortResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @funcstatic ajShortResize **************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajShortResize(AjPShort *thys, ajint size)
{
    AjPShort p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize short array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    *thys = ajShortNewL(s);

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(short));

    (*thys)->Len = size+1;


    ajShortDel(&p);

    return ajTrue;
}




/* @func ajShortShort *********************************************************
**
** Returns the current short* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPShort] Source array
** @return [short*] Current array pointer, or a null string if undefined.
** @category cast [AjPShort] Retrieve internal pointer
** @@
******************************************************************************/

short* ajShortShort(const AjPShort thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajShortLen ***********************************************************
**
** Get length of dynamic 1d short array
**
** @param [r] thys [const AjPShort] Source array
** @return [ajint] length
** @@
******************************************************************************/

ajint ajShortLen(const AjPShort thys)
{
    return thys->Len;
}




/* @func ajLongNew ************************************************************
**
** Default constructor for empty AJAX ajlong arrays.
**
** @return [AjPLong] Pointer to an empty ajlong array structure
** @category new [AjPLong] Default constructor
** @@
******************************************************************************/

AjPLong ajLongNew(void)
{
    AjPLong thys;

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(ajlong));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    return thys;
}




/* @func ajLongNewL ***********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size
** @return [AjPLong] Pointer to an empty ajlong array struct of specified size.
** @category new [AjPLong] Constructor with reserved size
** @@
******************************************************************************/

AjPLong ajLongNewL(ajint size)
{
    AjPLong thys;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(ajlong));
    thys->Len = 0;
    thys->Res = (ajint)size;

    return thys;
}




/* @func ajLongDel ************************************************************
**
** Default destructor for AJAX ajlong arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPLong*] Pointer to the ajlong array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPLong] Default destructor
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




/* @func ajLongGet ************************************************************
**
** Retrieve an element from an AJAX ajlong array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPLong] Pointer to the ajlong array.
** @param  [r] elem [ajint] array element.
**
** @return [ajlong] contents of array element
** @category cast [AjPLong] Retrieve a ajlong from an array
** @@
******************************************************************************/

ajlong ajLongGet(const AjPLong thys, ajint elem)
{
    if(elem<0 || !thys || elem>=thys->Len)
	ajErr("Attempt to access bad ajlong array index %d\n",elem);

    return thys->Ptr[elem];
}




/* @func ajLongPut ************************************************************
**
** Load a ajlong array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong*] Pointer to the ajint array.
** @param  [r] elem [ajint] array element.
** @param  [r] v [ajlong] value to load.
**
** @return [AjBool] true if the array was extended.
** @category modify [AjPLong] Load a ajlong array element
** @@
******************************************************************************/

AjBool ajLongPut(AjPLong *thys, ajint elem, ajlong v)
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

    ajLongResize(thys, elem);

    (*thys)->Ptr[elem] = v;

    return ajTrue;
}




/* @funcstatic ajLongResize ***************************************************
**
** Resize a ajlong array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajLongResize(AjPLong *thys, ajint size)
{
    AjPLong p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize ajlong array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    *thys = ajLongNewL(s);

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove((*thys)->Ptr,p->Ptr,limit*sizeof(ajlong));

    (*thys)->Len = size+1;


    ajLongDel(&p);

    return ajTrue;
}




/* @func ajLongLong ***********************************************************
**
** Returns the current ajlong* pointer. This will remain valid until
** the array is resized or deleted.
**
** @param [r] thys [const AjPLong] Source array
** @return [ajlong*] Current array pointer, or a null string if undefined.
** @category cast [AjPLong] Retrieve internal pointer
** @@
******************************************************************************/

ajlong* ajLongLong(const AjPLong thys)
{
    if(!thys || !thys->Len)
	return NULL;

    return thys->Ptr;
}




/* @func ajLongLen ************************************************************
**
** Get length of dynamic 1d ajlong array
**
** @param [r] thys [const AjPLong] Source array
** @return [ajlong] length
** @@
******************************************************************************/

ajlong ajLongLen(const AjPLong thys)
{
    return thys->Len;
}




/* @func ajFloatParse *********************************************************
**
** Parses a string into a floating point array.
**
** The array size is already set.
**
** @param [r] str [const AjPStr] Input string
** @param [w] array [AjPFloat*] Array
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFloatParse (const AjPStr str, AjPFloat* array)
{
    static AjPRegexp numexp = NULL;

    ajint i = 0;
    float t = 0.0;

    static AjPStr tmpstr = NULL;
    static AjPStr numstr = NULL;

    if (!numexp)
	numexp = ajRegCompC ("[+-]?[0-9.]+");

    ajStrAssS (&tmpstr, str);

    while (ajRegExec (numexp, tmpstr))
    {
	ajRegSubI (numexp, 0, &numstr);
	ajRegPost (numexp, &tmpstr);
	ajDebug ("array [%d] '%S'\n", i, numstr);
	ajStrToFloat (numstr, &t);
	ajFloatPut(array,i,t);
	i++;
    }

    if (!i)
	return ajFalse;

    return ajTrue;
}




/* @func ajFloatStr ***********************************************************
**
** Writes a floating point array as a string
**
** @param [r] array [const AjPFloat] Array
** @param [r] precision [ajint] floating point precision
** @param [w] str [AjPStr*] Output string
** @return [void]
** @@
******************************************************************************/

void ajFloatStr (const AjPFloat array, ajint precision, AjPStr* str)
{
    ajint i;

    for (i=0; i < array->Len; i++)
    {
	if (i)
	    ajStrAppK (str, ' ');
	ajFmtPrintAppS (str, "%.*f", precision, ajFloatGet(array,i));
    }
    return;
}




/* @func ajFloatTrace *********************************************************
**
** Writes a floating point array to the debug file
**
** @param [r] array [const AjPFloat] Array
** @param [r] precision [ajint] floating point precision
** @param [r] text [const char*] Report title
** @return [void]
** @@
******************************************************************************/

void ajFloatTrace (const AjPFloat array, ajint precision, const char* text)
{
    ajint i;

    ajDebug ("%s\n", text);
    for (i=0; i < array->Len; i++)
	ajDebug ("%3d: %.*f\n", i, precision, ajFloatGet(array,i));

    ajDebug ("\n");

    return;
}




/* @func ajArrCommaList *******************************************************
**
** Creates an AjPStr array from a string of comma separated tokens
**
** @param [r] s [const AjPStr] Line containing comma separated strings
** @param [w] a [AjPStr **] array pointer to create and load
**
** @return [ajint] number of array elements created
** @@
******************************************************************************/
ajint ajArrCommaList(const AjPStr s, AjPStr **a)
{
    AjPStr    x;
    AjPStrTok t;
    ajint n;
    ajint i;


    n = ajStrTokenCount(s,",\n");
    if(!n)
	return 0;

    AJCNEW(*a, n);

    x = ajStrNew();
    t = ajStrTokenInit(s,",\n");

    for(i=0;i<n;++i)
    {
	ajStrToken(&x,&t,",\n");
	(*a)[i] = ajStrNewC(ajStrStr(x));
    }

    ajStrDel(&x);
    ajStrTokenClear(&t);

    return n;
}




/* @func  ajArrDoubleLine *****************************************************
**
** Creates a double array from a string of columns
**
** @param [r] line [const AjPStr] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] cols [ajint] Number of tokens in the string
** @param [r] startcol [ajint] Start token (1 to n)
** @param [r] endcol [ajint] End token (1 to n)
** @return [double*] Allocated array of integers
** @@
******************************************************************************/

double* ajArrDoubleLine(const AjPStr line, const char *delim, ajint cols,
			ajint startcol, ajint endcol)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;
    static double *ret;
    ajint ncols;
    ajint i;


    t = ajStrTokenInit(line, delim);
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




/* @func ajArrIntLine *********************************************************
**
** Creates an Int array from a string of columns
**
** @param [r] line [const AjPStr] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] cols [ajint] Number of tokens in the string
** @param [r] startcol [ajint] Start token (1 to n)
** @param [r] endcol [ajint] End token (1 to n)
** @return [ajint*] Allocated array of integers
** @@
******************************************************************************/

ajint* ajArrIntLine(const AjPStr line, const char *delim, ajint cols,
		    ajint startcol, ajint endcol)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;
    static ajint *ret;
    ajint ncols;
    ajint i;


    t     = ajStrTokenInit(line, delim);
    tmp   = ajStrNew();
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




/* @func  ajArrFloatLine ******************************************************
**
** Creates a Float array from a string of columns
**
** @param [r] line [const AjPStr] Line containing numbers
** @param [r] delim [const char*]  Delimiter string for tokens
** @param [r] cols [ajint] Number of tokens in the string
** @param [r] startcol [ajint] Start token (1 to n)
** @param [r] endcol [ajint] End token (1 to n)
** @return [float*] Allocated array of integers
** @@
******************************************************************************/

float* ajArrFloatLine(const AjPStr line, const char *delim, ajint cols,
		      ajint startcol, ajint endcol)
{
    AjPStrTok t = NULL;
    AjPStr tmp  = NULL;
    static float *ret;
    ajint ncols;
    ajint i;
    AjPStr tmpline = NULL;

    tmpline = ajStrNew();
    ajStrAssS(&tmpline,line);
    
    ajStrClean(&tmpline);

    t     = ajStrTokenInit(tmpline, delim);
    tmp   = ajStrNew();
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
    ajStrDel(&tmpline);
    ajStrTokenClear(&t);

    return ret;
}




/* @func ajInt2dNew ***********************************************************
**
** Default constructor for empty AJAX 2D integer arrays.
**
** @return [AjPInt2d] Pointer to an empty integer array structure
** @category new [AjPInt2d] Default constructor
** @@
******************************************************************************/

AjPInt2d ajInt2dNew(void)
{
    AjPInt2d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPInt*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajInt2dNewL **********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPInt2d] Pointer to an empty integer 2d array struct of
**                    specified size.
** @category new [AjPInt2d] Constructor with reserved size
** @@
******************************************************************************/

AjPInt2d ajInt2dNewL(ajint size)
{
    AjPInt2d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt*));
    thys->Len = 0;
    thys->Res = (ajint)size;

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
** @param  [d] thys [AjPInt2d*] Pointer to the ajint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPInt2d] Default destructor
** @@
******************************************************************************/

void ajInt2dDel(AjPInt2d *thys)
{
    ajint i;

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




/* @func ajInt2dGet ***********************************************************
**
** Retrieve an element from an AJAX 2d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPInt2d] Pointer to the ajint array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
**
** @return [ajint] contents of array element
** @category cast [AjPInt2d] Retrieve an integer from an array
** @@
******************************************************************************/

ajint ajInt2dGet(const AjPInt2d thys, ajint elem1, ajint elem2)
{
    AjPInt t;

    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajint array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajIntGet(t,elem2);
}




/* @func ajInt2dPut ***********************************************************
**
** Load an integer 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt2d*] Pointer to the ajint array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] v [ajint] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPInt2d] Load an integer array element
** @@
******************************************************************************/

AjBool ajInt2dPut(AjPInt2d *thys, ajint elem1, ajint elem2, ajint v)
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

    ajInt2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajIntNew();

    ajIntPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic ajInt2dResize **************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt2d*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajInt2dResize(AjPInt2d *thys, ajint size)
{
    AjPInt2d nthys;
    AjPInt2d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize integer array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPInt*));
    nthys->Res = s;

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPInt*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajIntDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajInt2dLen ***********************************************************
**
** Get lengths of 2d ajint array
**
**
** @param  [r] thys [const AjPInt2d] Pointer to the ajint array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajInt2dLen(const AjPInt2d thys, ajint* len1, ajint* len2)
{
    AjPInt t;
    ajint i;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajInt2dInt ***********************************************************
**
** Convert AjPInt2d to ajint**
**
** @param  [r] thys [const AjPInt2d] Pointer to the ajint array.
**
** @return [ajint**] coverted value.
** @category cast [AjPInt2d] Retrieve internal pointer
** @@
******************************************************************************/

ajint** ajInt2dInt(const AjPInt2d thys)
{
    AjPInt t = NULL;
    ajint **array;
    ajint d1;
    ajint d2;
    ajint i;

    ajInt2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(ajint));
    }

    return array;
}




/* @func ajInt3dNew ***********************************************************
**
** Default constructor for empty AJAX 3D integer arrays.
**
** @return [AjPInt3d] Pointer to an empty integer array structure
** @category new [AjPInt3d] Default constructor
** @@
******************************************************************************/

AjPInt3d ajInt3dNew(void)
{
    AjPInt3d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPInt2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajInt3dNewL **********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPInt3d] Pointer to an empty integer 3d array struct of
**                    specified size.
** @category new [AjPInt3d] Constructor with reserved size
** @@
******************************************************************************/

AjPInt3d ajInt3dNewL(ajint size)
{
    AjPInt3d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPInt2d*));
    thys->Len = 0;
    thys->Res = (ajint)size;

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
** @param  [d] thys [AjPInt3d*] Pointer to the ajint array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPInt3d] Default destructor
** @@
******************************************************************************/

void ajInt3dDel(AjPInt3d *thys)
{
    ajint i;

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




/* @func ajInt3dGet ***********************************************************
**
** Retrieve an element from an AJAX 3d integer array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPInt3d] Pointer to the ajint array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
**
** @return [ajint] contents of array element
** @category cast [AjPInt3d] Retrieve an integer from an array
** @@
******************************************************************************/

ajint ajInt3dGet(const AjPInt3d thys, ajint elem1, ajint elem2, ajint elem3)
{
    AjPInt2d t;

    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajint array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajInt2dGet(t,elem2,elem3);
}




/* @func ajInt3dPut ***********************************************************
**
** Load an integer 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt3d*] Pointer to the ajint array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
** @param  [r] v [ajint] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPInt3d] Load an integer array element
** @@
******************************************************************************/

AjBool ajInt3dPut(AjPInt3d *thys,
		  ajint elem1, ajint elem2, ajint elem3, ajint v)
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

    ajInt3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajInt2dNew();

    ajInt2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic ajInt3dResize **************************************************
**
** Resize an integer array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPInt3d*] Pointer to the ajint array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajInt3dResize(AjPInt3d *thys, ajint size)
{
    AjPInt3d nthys;
    AjPInt3d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


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

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPInt2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajInt2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajInt3dLen ***********************************************************
**
** Get lengths of 3d ajint array
**
**
** @param  [r] thys [const AjPInt3d] Pointer to the ajint array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
** @param  [w] len3 [ajint*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajInt3dLen(const AjPInt3d thys, ajint* len1, ajint* len2, ajint* len3)
{
    AjPInt2d t2;
    AjPInt   t1;
    ajint i;
    ajint j;
    ajint v;

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




/* @func ajInt3dInt ***********************************************************
**
** Convert AjPInt3d to ajint***
**
** @param  [r] thys [const AjPInt3d] Pointer to the ajint array.
**
** @return [ajint***] converted values.
** @category cast [AjPInt3d] Retrieve internal pointer
** @@
******************************************************************************/

ajint*** ajInt3dInt(const AjPInt3d thys)
{
    AjPInt2d t2 = NULL;
    AjPInt   t1 = NULL;
    ajint ***array;
    ajint d1;
    ajint d2;
    ajint d3;
    ajint i;
    ajint j;

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
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(ajint));
	    }
	}
    }

    return array;
}




/* @func ajFloat2dNew *********************************************************
**
** Default constructor for empty AJAX 2D float arrays.
**
** @return [AjPFloat2d] Pointer to an empty float array structure
** @category new [AjPFloat2d] Default constructor
** @@
******************************************************************************/

AjPFloat2d ajFloat2dNew(void)
{
    AjPFloat2d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPFloat*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat2dNewL ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPFloat2d] Pointer to an empty float 2d array struct of
**                    specified size.
** @category new [AjPFloat2d] Constructor with reserved size
** @@
******************************************************************************/

AjPFloat2d ajFloat2dNewL(ajint size)
{
    AjPFloat2d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPFloat*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat2dDel *********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPFloat2d*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFloat2d] Default destructor
** @@
******************************************************************************/

void ajFloat2dDel(AjPFloat2d *thys)
{
    ajint i;

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




/* @func ajFloat2dGet *********************************************************
**
** Retrieve an element from an AJAX 2d float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPFloat2d] Pointer to the float array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
**
** @return [float] contents of array element
** @category cast [AjPFloat2d] Retrieve a float from an array
** @@
******************************************************************************/

float ajFloat2dGet(const AjPFloat2d thys, ajint elem1, ajint elem2)
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




/* @func ajFloat2dPut *********************************************************
**
** Load a float 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPFloat2d] Load a float array element
** @@
******************************************************************************/

AjBool ajFloat2dPut(AjPFloat2d *thys, ajint elem1, ajint elem2, float v)
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

    ajFloat2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajFloatNew();

    ajFloatPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic ajFloat2dResize ************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat2d*] Pointer to the float array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajFloat2dResize(AjPFloat2d *thys, ajint size)
{
    AjPFloat2d nthys;
    AjPFloat2d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize float array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s    = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPFloat*));
    nthys->Res = s;

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPFloat*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajFloatDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajFloat2dLen *********************************************************
**
** Get lengths of 2d float array
**
** @param  [r] thys [const AjPFloat2d] Pointer to the float array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajFloat2dLen(const AjPFloat2d thys, ajint* len1, ajint* len2)
{
    AjPFloat t;
    ajint i;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajFloat2dFloat *******************************************************
**
** Convert AjPFloat2d to float**
**
** @param  [r] thys [const AjPFloat2d] Pointer to the float array.
**
** @return [float**] converted values.
** @category cast [AjPFloat2d] Retrieve internal pointer
** @@
******************************************************************************/

float** ajFloat2dFloat(const AjPFloat2d thys)
{
    AjPFloat t = NULL;
    float **array;
    ajint d1;
    ajint d2;
    ajint i;

    ajFloat2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(float));
    }

    return array;
}




/* @func ajFloat3dNew *********************************************************
**
** Default constructor for empty AJAX 3D float arrays.
**
** @return [AjPFloat3d] Pointer to an empty float array structure
** @category new [AjPFloat3d] Default constructor
** @@
******************************************************************************/

AjPFloat3d ajFloat3dNew(void)
{
    AjPFloat3d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPFloat2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat3dNewL ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPFloat3d] Pointer to an empty float 3d array struct of
**                    specified size.
** @category new [AjPFloat3d] Constructor with reserved size
** @@
******************************************************************************/

AjPFloat3d ajFloat3dNewL(ajint size)
{
    AjPFloat3d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPFloat2d*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajFloat3dDel *********************************************************
**
** Default destructor for AJAX float arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPFloat3d*] Pointer to the float array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPFloat3d] Default destructor
** @@
******************************************************************************/

void ajFloat3dDel(AjPFloat3d *thys)
{
    ajint i;

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




/* @func ajFloat3dGet *********************************************************
**
** Retrieve an element from an AJAX 3d float array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPFloat3d] Pointer to the float array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
**
** @return [float] contents of array element
** @category cast [AjPFloat3d] Retrieve a float from an array
** @@
******************************************************************************/

float ajFloat3dGet(const AjPFloat3d thys, ajint elem1, ajint elem2, ajint elem3)
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




/* @func ajFloat3dPut *********************************************************
**
** Load a float 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
** @param  [r] v [float] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPFloat3d] Load a float array element
** @@
******************************************************************************/

AjBool ajFloat3dPut(AjPFloat3d *thys,
		    ajint elem1, ajint elem2, ajint elem3, float v)
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

    ajFloat3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajFloat2dNew();

    ajFloat2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic ajFloat3dResize ************************************************
**
** Resize a float array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPFloat3d*] Pointer to the float array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajFloat3dResize(AjPFloat3d *thys, ajint size)
{
    AjPFloat3d nthys;
    AjPFloat3d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


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

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPFloat2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajFloat2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajFloat3dLen *********************************************************
**
** Get lengths of 3d float array
**
** @param  [r] thys [const AjPFloat3d] Pointer to the float array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
** @param  [w] len3 [ajint*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajFloat3dLen(const AjPFloat3d thys, ajint* len1, ajint* len2, ajint* len3)
{
    AjPFloat2d t2;
    AjPFloat   t1;
    ajint i;
    ajint j;
    ajint v;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3 = 0;
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




/* @func ajFloat3dFloat *******************************************************
**
** Convert AjPFloat3d to float***
**
** @param  [r] thys [const AjPFloat3d] Pointer to the float array.
**
** @return [float***] converted values.
** @category cast [AjPFloat3d] Retrieve internal pointer
** @@
******************************************************************************/

float*** ajFloat3dFloat(const AjPFloat3d thys)
{
    AjPFloat2d t2 = NULL;
    AjPFloat   t1 = NULL;
    float ***array;
    ajint d1;
    ajint d2;
    ajint d3;
    ajint i;
    ajint j;

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
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(float));
	    }
	}
    }

    return array;
}




/* @func ajDouble2dNew ********************************************************
**
** Default constructor for empty AJAX 2D double arrays.
**
** @return [AjPDouble2d] Pointer to an empty double array structure
** @category new [AjPDouble2d] Default constructor
** @@
******************************************************************************/

AjPDouble2d ajDouble2dNew(void)
{
    AjPDouble2d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPDouble*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble2dNewL *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPDouble2d] Pointer to an empty double 2d array struct of
**                    specified size.
** @category new [AjPDouble2d] Constructor with reserved size
** @@
******************************************************************************/

AjPDouble2d ajDouble2dNewL(ajint size)
{
    AjPDouble2d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPDouble*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble2dDel ********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPDouble2d*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPDouble2d] Default destructor
** @@
******************************************************************************/

void ajDouble2dDel(AjPDouble2d *thys)
{
    ajint i;

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




/* @func ajDouble2dGet ********************************************************
**
** Retrieve an element from an AJAX 2d double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPDouble2d] Pointer to the double array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
**
** @return [double] contents of array element
** @category cast [AjPDouble2d] Retrieve a double from an array
** @@
******************************************************************************/

double ajDouble2dGet(const AjPDouble2d thys, ajint elem1, ajint elem2)
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




/* @func ajDouble2dPut ********************************************************
**
** Load a double 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPDouble2d] Load a double array element
** @@
******************************************************************************/

AjBool ajDouble2dPut(AjPDouble2d *thys, ajint elem1, ajint elem2, double v)
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

    ajDouble2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajDoubleNew();

    ajDoublePut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic ajDouble2dResize ***********************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble2d*] Pointer to the double array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajDouble2dResize(AjPDouble2d *thys, ajint size)
{
    AjPDouble2d nthys;
    AjPDouble2d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


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

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPDouble*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajDoubleDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajDouble2dLen ********************************************************
**
** Get lengths of 2d double array
**
** @param  [r] thys [const AjPDouble2d] Pointer to the double array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajDouble2dLen(const AjPDouble2d thys, ajint* len1, ajint* len2)
{
    AjPDouble t;
    ajint i;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajDouble2dDouble *****************************************************
**
** Convert AjPDouble2d to double**
**
** @param  [r] thys [const AjPDouble2d] Pointer to the double array.
**
** @return [double**] converted values.
** @category cast [AjPDouble2d] Retrieve internal pointer
** @@
******************************************************************************/

double** ajDouble2dDouble(const AjPDouble2d thys)
{
    AjPDouble t = NULL;
    double **array;
    ajint d1;
    ajint d2;
    ajint i;

    ajDouble2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(double));
    }

    return array;
}




/* @func ajDouble3dNew ********************************************************
**
** Default constructor for empty AJAX 3D double arrays.
**
** @return [AjPDouble3d] Pointer to an empty double array structure
** @category new [AjPDouble3d] Default constructor
** @@
******************************************************************************/

AjPDouble3d ajDouble3dNew(void)
{
    AjPDouble3d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPDouble2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble3dNewL *******************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPDouble3d] Pointer to an empty double 3d array struct of
**                    specified size.
** @category new [AjPDouble3d] Constructor with reserved size
** @@
******************************************************************************/

AjPDouble3d ajDouble3dNewL(ajint size)
{
    AjPDouble3d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPDouble2d*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajDouble3dDel ********************************************************
**
** Default destructor for AJAX double arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPDouble3d*] Pointer to the double array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPDouble3d] Default destructor
** @@
******************************************************************************/

void ajDouble3dDel(AjPDouble3d *thys)
{
    ajint i;

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




/* @func ajDouble3dGet ********************************************************
**
** Retrieve an element from an AJAX 3d double array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPDouble3d] Pointer to the double array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
**
** @return [double] contents of array element
** @category cast [AjPDouble3d] Retrieve a double from an array
** @@
******************************************************************************/

double ajDouble3dGet(const AjPDouble3d thys, ajint elem1, ajint elem2, ajint elem3)
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




/* @func ajDouble3dPut ********************************************************
**
** Load a double 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
** @param  [r] v [double] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPDouble3d] Load a double array element
** @@
******************************************************************************/

AjBool ajDouble3dPut(AjPDouble3d *thys,
		     ajint elem1, ajint elem2, ajint elem3, double v)
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

    ajDouble3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajDouble2dNew();

    ajDouble2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic ajDouble3dResize ***********************************************
**
** Resize a double array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPDouble3d*] Pointer to the double array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajDouble3dResize(AjPDouble3d *thys, ajint size)
{
    AjPDouble3d nthys;
    AjPDouble3d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


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

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPDouble2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajDouble2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajDouble3dLen ********************************************************
**
** Get lengths of 3d double array
**
** @param  [r] thys [const AjPDouble3d] Pointer to the double array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
** @param  [w] len3 [ajint*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajDouble3dLen(const AjPDouble3d thys, ajint* len1, ajint* len2, ajint* len3)
{
    AjPDouble2d t2;
    AjPDouble   t1;
    ajint i;
    ajint j;
    ajint v;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3 = 0;
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




/* @func ajDouble3dDouble *****************************************************
**
** Convert AjPDouble3d to double***
**
** @param  [r] thys [const AjPDouble3d] Pointer to the double array.
**
** @return [double***] converted values.
** @category cast [AjPDouble3d] Retrieve internal pointer
** @@
******************************************************************************/

double*** ajDouble3dDouble(const AjPDouble3d thys)
{
    AjPDouble2d t2 = NULL;
    AjPDouble   t1 = NULL;
    double ***array;
    ajint d1;
    ajint d2;
    ajint d3;
    ajint i;
    ajint j;

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
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(double));
	    }
	}
    }

    return array;
}




/* @func ajShort2dNew *********************************************************
**
** Default constructor for empty AJAX 2D short arrays.
**
** @return [AjPShort2d] Pointer to an empty short array structure
** @category new [AjPShort2d] Default constructor
** @@
******************************************************************************/

AjPShort2d ajShort2dNew(void)
{
    AjPShort2d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPShort*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort2dNewL ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPShort2d] Pointer to an empty short 2d array struct of
**                    specified size.
** @category new [AjPShort2d] Constructor with reserved size
** @@
******************************************************************************/

AjPShort2d ajShort2dNewL(ajint size)
{
    AjPShort2d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPShort*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort2dDel *********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPShort2d*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPShort2d] Default destructor
** @@
******************************************************************************/

void ajShort2dDel(AjPShort2d *thys)
{
    ajint i;

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




/* @func ajShort2dGet *********************************************************
**
** Retrieve an element from an AJAX 2d short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPShort2d] Pointer to the short array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
**
** @return [short] contents of array element
** @category cast [AjPShort2d] Retrieve a short from an array
** @@
******************************************************************************/

short ajShort2dGet(const AjPShort2d thys, ajint elem1, ajint elem2)
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




/* @func ajShort2dPut *********************************************************
**
** Load a short 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPShort2d] Load a short array element
** @@
******************************************************************************/

AjBool ajShort2dPut(AjPShort2d *thys, ajint elem1, ajint elem2, short v)
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

    ajShort2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajShortNew();

    ajShortPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic ajShort2dResize ************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort2d*] Pointer to the short array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajShort2dResize(AjPShort2d *thys, ajint size)
{
    AjPShort2d nthys;
    AjPShort2d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


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

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPShort*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajShortDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajShort2dLen *********************************************************
**
** Get lengths of 2d short array
**
** @param  [r] thys [const AjPShort2d] Pointer to the short array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajShort2dLen(const AjPShort2d thys, ajint* len1, ajint* len2)
{
    AjPShort t;
    ajint i;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajShort2dShort *******************************************************
**
** Convert AjPShort2d to short**
**
** @param  [r] thys [const AjPShort2d] Pointer to the short array.
**
** @return [short**] converted values
** @category cast [AjPShort2d] Retrieve internal pointer
** @@
******************************************************************************/

short** ajShort2dShort(const AjPShort2d thys)
{
    AjPShort t = NULL;
    short **array;
    ajint d1;
    ajint d2;
    ajint i;

    ajShort2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(short));
    }

    return array;
}




/* @func ajShort3dNew *********************************************************
**
** Default constructor for empty AJAX 3D short arrays.
**
** @return [AjPShort3d] Pointer to an empty short array structure
** @category new [AjPShort3d] Default constructor
** @@
******************************************************************************/

AjPShort3d ajShort3dNew(void)
{
    AjPShort3d thys;
    ajint    i;


    AJNEW0(thys);
    thys->Ptr = AJALLOC0(RESERVED_SIZE*sizeof(AjPShort2d*));
    thys->Len = 0;
    thys->Res = RESERVED_SIZE;

    for(i=0;i<RESERVED_SIZE;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort3dNewL ********************************************************
**
** Constructor given an initial reserved size.
**
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPShort3d] Pointer to an empty short 3d array struct of
**                    specified size.
** @category new [AjPShort3d] Constructor with reserved size
** @@
******************************************************************************/

AjPShort3d ajShort3dNewL(ajint size)
{
    AjPShort3d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPShort2d*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajShort3dDel *********************************************************
**
** Default destructor for AJAX short arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPShort3d*] Pointer to the short array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPShort3d] Default destructor
** @@
******************************************************************************/

void ajShort3dDel(AjPShort3d *thys)
{
    ajint i;

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




/* @func ajShort3dGet *********************************************************
**
** Retrieve an element from an AJAX 3d short array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPShort3d] Pointer to the short array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
**
** @return [short] contents of array element
** @category cast [AjPShort3d] Retrieve a short from an array
** @@
******************************************************************************/

short ajShort3dGet(const AjPShort3d thys, ajint elem1, ajint elem2, ajint elem3)
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




/* @func ajShort3dPut *********************************************************
**
** Load a short 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
** @param  [r] v [short] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPShort3d] Load a short array element
** @@
******************************************************************************/

AjBool ajShort3dPut(AjPShort3d *thys,
		    ajint elem1, ajint elem2, ajint elem3, short v)
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

    ajShort3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajShort2dNew();

    ajShort2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic ajShort3dResize ************************************************
**
** Resize a short array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPShort3d*] Pointer to the short array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajShort3dResize(AjPShort3d *thys, ajint size)
{
    AjPShort3d nthys;
    AjPShort3d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


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

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPShort2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajShort2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajShort3dLen *********************************************************
**
** Get lengths of 3d short array
**
** @param  [r] thys [const AjPShort3d] Pointer to the short array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
** @param  [w] len3 [ajint*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajShort3dLen(const AjPShort3d thys, ajint* len1, ajint* len2, ajint* len3)
{
    AjPShort2d t2;
    AjPShort   t1;
    ajint i;
    ajint j;
    ajint v;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3 = 0;
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




/* @func ajShort3dShort *******************************************************
**
** Convert AjPShort3d to short***
**
** @param  [r] thys [const AjPShort3d] Pointer to the short array.
**
** @return [short***] converted values.
** @category cast [AjPShort3d] Retrieve internal pointer
** @@
******************************************************************************/

short*** ajShort3dShort(const AjPShort3d thys)
{
    AjPShort2d t2 = NULL;
    AjPShort   t1 = NULL;
    short ***array;
    ajint d1;
    ajint d2;
    ajint d3;
    ajint i;
    ajint j;

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
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(short));
	    }
	}
    }

    return array;
}




/* @func ajLong2dNew **********************************************************
**
** Default constructor for empty AJAX 2D ajlong arrays.
**
** @return [AjPLong2d] Pointer to an empty ajlong array structure
** @category new [AjPLong2d] Default constructor
** @@
******************************************************************************/

AjPLong2d ajLong2dNew(void)
{
    AjPLong2d thys;
    ajint    i;


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
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPLong2d] Pointer to an empty ajlong 2d array struct of
**                    specified size.
** @category new [AjPLong2d] Constructor with reserved size
** @@
******************************************************************************/

AjPLong2d ajLong2dNewL(ajint size)
{
    AjPLong2d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPLong*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajLong2dDel **********************************************************
**
** Default destructor for AJAX ajlong arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPLong2d*] Pointer to the ajlong array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPLong2d] Default destructor
** @@
******************************************************************************/

void ajLong2dDel(AjPLong2d *thys)
{
    ajint i;

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




/* @func ajLong2dGet **********************************************************
**
** Retrieve an element from an AJAX 2d ajlong array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPLong2d] Pointer to the ajlong array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
**
** @return [ajlong] contents of array element
** @category cast [AjPLong2d] Retrieve a ajlong from an array
** @@
******************************************************************************/

ajlong ajLong2dGet(const AjPLong2d thys, ajint elem1, ajint elem2)
{
    AjPLong t;

    if(elem1<0 || elem2<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajlong array index [%d][%d]\n",elem1,
	      elem2);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][]\n",elem1);

    return ajLongGet(t,elem2);
}




/* @func ajLong2dPut **********************************************************
**
** Load a ajlong 2d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong2d*] Pointer to the ajlong array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] v [ajlong] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPLong2d] Load a ajlong array element
** @@
******************************************************************************/

AjBool ajLong2dPut(AjPLong2d *thys, ajint elem1, ajint elem2, ajlong v)
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

    ajLong2dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajLongNew();

    ajLongPut(&(*thys)->Ptr[elem1],elem2,v);

    return ajTrue;
}




/* @funcstatic ajLong2dResize *************************************************
**
** Resize a ajlong array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong2d*] Pointer to the ajlong array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajLong2dResize(AjPLong2d *thys, ajint size)
{
    AjPLong2d nthys;
    AjPLong2d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize ajlong array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPLong*));
    nthys->Res = s;

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPLong*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajLongDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajLong2dLen **********************************************************
**
** Get lengths of 2d ajlong array
**
** @param  [r] thys [const AjPLong2d] Pointer to the ajlong array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
**
** @return [void]
** @@
******************************************************************************/

void ajLong2dLen(const AjPLong2d thys, ajint* len1, ajint* len2)
{
    AjPLong t;
    ajint i;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t=thys->Ptr[i]))
	    *len2 = (*len2 > t->Len) ? *len2 : t->Len;

    return;
}




/* @func ajLong2dLong *********************************************************
**
** Convert AjPLong2d to ajlong**
**
** @param  [r] thys [const AjPLong2d] Pointer to the ajlong array.
**
** @return [ajlong**] converted values.
** @category cast [AjPLong2d] Retrieve internal pointer
** @@
******************************************************************************/

ajlong** ajLong2dLong(const AjPLong2d thys)
{
    AjPLong t = NULL;
    ajlong **array;
    ajint d1;
    ajint d2;
    ajint i;

    ajLong2dLen(thys,&d1,&d2);

    AJCNEW(array,d1);
    for(i=0;i<d1;++i)
    {
	AJCNEW0(array[i],d2);
	if((t=thys->Ptr[i]))
	    memmove(array[i],t->Ptr,t->Len*sizeof(ajlong));
    }

    return array;
}




/* @func ajLong3dNew **********************************************************
**
** Default constructor for empty AJAX 3D ajlong arrays.
**
** @return [AjPLong3d] Pointer to an empty ajlong array structure
** @category new [AjPLong3d] Default constructor
** @@
******************************************************************************/

AjPLong3d ajLong3dNew(void)
{
    AjPLong3d thys;
    ajint    i;


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
** @param [r] size [ajint] Reserved size 1st dim
** @return [AjPLong3d] Pointer to an empty ajlong 3d array struct of
**                    specified size.
** @category new [AjPLong3d] Constructor with reserved size
** @@
******************************************************************************/

AjPLong3d ajLong3dNewL(ajint size)
{
    AjPLong3d thys;
    ajint i;

    size = ajRound(size,RESERVED_SIZE);

    AJNEW0(thys);
    thys->Ptr = AJALLOC0(size*sizeof(AjPLong2d*));
    thys->Len = 0;
    thys->Res = (ajint)size;

    for(i=0;i<size;++i)
	thys->Ptr[i] = NULL;

    return thys;
}




/* @func ajLong3dDel **********************************************************
**
** Default destructor for AJAX ajlong arrays.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [d] thys [AjPLong3d*] Pointer to the ajlong array to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjPLong3d] Default destructor
** @@
******************************************************************************/

void ajLong3dDel(AjPLong3d *thys)
{
    ajint i;

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




/* @func ajLong3dGet **********************************************************
**
** Retrieve an element from an AJAX 3d ajlong array.
**
** If the given array is a NULL pointer, simply returns.
**
** @param  [r] thys [const AjPLong3d] Pointer to the ajlong array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
**
** @return [ajlong] contents of array element
** @category cast [AjPLong3d] Retrieve a ajlong from an array
** @@
******************************************************************************/

ajlong ajLong3dGet(const AjPLong3d thys, ajint elem1, ajint elem2, ajint elem3)
{
    AjPLong2d t;

    if(elem1<0 || elem2<0 || elem3<0 || !thys || elem1>=thys->Len)
	ajErr("Attempt to access bad ajlong array index [%d][%d][%d]\n",elem1,
	      elem2,elem3);

    t = thys->Ptr[elem1];
    if(!t)
	ajErr("Attempt to access bad 1st dimension [%d][][]\n",elem1);

    return ajLong2dGet(t,elem2,elem3);
}




/* @func ajLong3dPut **********************************************************
**
** Load a ajlong 3d array element.
**
** If the given array is a NULL pointer an error is generated.
** If the array is of insufficient size then the array is extended.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong3d*] Pointer to the ajlong array.
** @param  [r] elem1 [ajint] array element.
** @param  [r] elem2 [ajint] array element.
** @param  [r] elem3 [ajint] array element.
** @param  [r] v [ajlong] value to load.
**
** @return [AjBool] true if any array was extended.
** @category modify [AjPLong3d] Load a ajlong array element
** @@
******************************************************************************/

AjBool ajLong3dPut(AjPLong3d *thys,
		   ajint elem1, ajint elem2, ajint elem3, ajlong v)
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

    ajLong3dResize(thys, elem1);

    if(!(*thys)->Ptr[elem1])
	(*thys)->Ptr[elem1] = ajLong2dNew();

    ajLong2dPut(&(*thys)->Ptr[elem1],elem2,elem3,v);

    return ajTrue;
}




/* @funcstatic ajLong3dResize *************************************************
**
** Resize a ajlong array.
**
** If the given array is a NULL pointer an error is generated.
** Negative indices generate an error.
**
** @param  [w] thys [AjPLong3d*] Pointer to the ajlong array.
** @param  [r] size [ajint] new size.
**
** @return [AjBool] true if the array was extended.
** @@
******************************************************************************/

static AjBool ajLong3dResize(AjPLong3d *thys, ajint size)
{
    AjPLong3d nthys;
    AjPLong3d p = NULL;
    ajint    s;
    ajint    clen;
    ajint    limit;
    ajint    i;


    if(!thys || !*thys || size<0)
	ajErr("Illegal attempt to resize ajlong array");

    clen = ajRound((*thys)->Len-1,RESERVED_SIZE);
    s = ajRound(size+1,RESERVED_SIZE);
    if(s == clen)
	return ajFalse;

    p = *thys;

    AJNEW0(nthys);
    nthys->Ptr = AJALLOC0(s*sizeof(AjPLong2d*));
    nthys->Res = s;

    if((ajint)size < p->Len-1)
	limit = size+1;
    else
	limit = p->Len;

    memmove(nthys->Ptr,p->Ptr,limit*sizeof(AjPLong2d*));

    i = nthys->Len = size+1;


    for(;i<p->Res;++i)
	if(p->Ptr[i])
	    ajLong2dDel(&p->Ptr[i]);

    AJFREE(p->Ptr);
    AJFREE(p);

    *thys = nthys;

    return ajTrue;
}




/* @func ajLong3dLen **********************************************************
**
** Get lengths of 3d ajlong array
**
** @param  [r] thys [const AjPLong3d] Pointer to the ajlong array.
** @param  [w] len1 [ajint*] Length of 1st dim
** @param  [w] len2 [ajint*] Length of 2nd dim
** @param  [w] len3 [ajint*] Length of 3rd dim
**
** @return [void]
** @@
******************************************************************************/

void ajLong3dLen(const AjPLong3d thys, ajint* len1, ajint* len2, ajint* len3)
{
    AjPLong2d t2;
    AjPLong   t1;
    ajint i;
    ajint j;
    ajint v;

    *len1 = thys->Len;
    *len2 = 0;
    for(i=0;i<*len1;++i)
	if((t2=thys->Ptr[i]))
	    *len2 = (*len2 > t2->Len) ? *len2 : t2->Len;
    *len3 = 0;
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



/* @func ajLong3dLong *********************************************************
**
** Convert AjPLong3d to ajlong***
**
** @param  [r] thys [const AjPLong3d] Pointer to the ajlong array.
**
** @return [ajlong***] converted values.
** @category cast [AjPLong3d] Retrieve internal pointer
** @@
******************************************************************************/

ajlong*** ajLong3dLong(const AjPLong3d thys)
{
    AjPLong2d t2 = NULL;
    AjPLong   t1 = NULL;
    ajlong ***array;
    ajint d1;
    ajint d2;
    ajint d3;
    ajint i;
    ajint j;

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
		    memmove(array[i][j],t1->Ptr,
				   t1->Len*sizeof(ajlong));
	    }
	}
    }

    return array;
}

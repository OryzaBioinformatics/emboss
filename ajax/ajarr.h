#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajarr_h
#define ajarr_h


#include <sys/types.h>

/* @data AjPChar **************************************************************
**
** Ajax character object.
**
** Holds a character array with additional data.
** The length is known and held internally.
**
** AjPChar is implemented as a pointer to a C data structure.
**
** @alias AjSChar
** @alias AjOChar
**
** @new    ajCharNew Default constructor
** @new    ajCharNewL Constructor with reserved size
** @delete ajCharDel Default destructor
** @ass    ajCharGet Retrieve a character from an array
** @mod    ajCharPut Load a character array element
** @cast   ajCharChar Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSChar {
  ajint Res;
  ajint Len;
  char *Ptr;
} AjOChar, *AjPChar;



/* @data AjPInt ***************************************************************
**
** Ajax integer object.
**
** Holds an integer array with additional data.
** The length is known and held internally.
**
** AjPInt is implemented as a pointer to a C data structure.
**
** @alias AjSInt
** @alias AjOInt
**
** @new    ajIntNew Default constructor
** @new    ajIntNewL Constructor with reserved size
** @delete ajIntDel Default destructor
** @ass    ajIntGet Retrieve an integer from an array
** @mod    ajIntPut Load an integer array element
** @cast   ajIntInt Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSInt {
  ajint Res;
  ajint Len;
  ajint *Ptr;
} AjOInt, *AjPInt;



/* @data AjPInt2d *************************************************************
**
** Ajax 2d integer object.
**
** Holds an integer array with additional data.
** The length is known and held internally.
**
** AjPInt2d is implemented as a pointer to a C data structure.
**
** @alias AjSInt2d
** @alias AjOInt2d
**
** @new    ajInt2dNew Default constructor
** @new    ajInt2dNewL Constructor with reserved size
** @delete ajInt2dDel Default destructor
** @ass    ajInt2dGet Retrieve an integer from an array
** @mod    ajInt2dPut Load an integer array element
** @cast   ajInt2dInt Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSInt2d {
  ajint Res;
  ajint Len;
  AjPInt *Ptr;
} AjOInt2d, *AjPInt2d;



/* @data AjPInt3d *************************************************************
**
** Ajax 3d integer object.
**
** Holds an integer array with additional data.
** The length is known and held internally.
**
** AjPInt3d is implemented as a pointer to a C data structure.
**
** @alias AjSInt3d
** @alias AjOInt3d
**
** @new    ajInt3dNew Default constructor
** @new    ajInt3dNewL Constructor with reserved size
** @delete ajInt3dDel Default destructor
** @ass    ajInt3dGet Retrieve an integer from an array
** @mod    ajInt3dPut Load an integer array element
** @cast   ajInt3dInt Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSInt3d {
  ajint Res;
  ajint Len;
  AjPInt2d *Ptr;
} AjOInt3d, *AjPInt3d;



/* @data AjPFloat *************************************************************
**
** Ajax float object.
**
** Holds a float array with additional data.
** The length is known and held internally.
**
** AjPFloat is implemented as a pointer to a C data structure.
**
** @alias AjSFloat
** @alias AjOFloat
**
** @new    ajFloatNew Default constructor
** @new    ajFloatNewL Constructor with reserved size
** @delete ajFloatDel Default destructor
** @ass    ajFloatGet Retrieve a float from an array
** @mod    ajFloatPut Load a float array element
** @cast   ajFloatFloat Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSFloat {
  ajint Res;
  ajint Len;
  float *Ptr;
} AjOFloat, *AjPFloat;



/* @data AjPFloat2d ***********************************************************
**
** Ajax 2d float object.
**
** Holds a 2d float array with additional data.
** The length is known and held internally.
**
** AjPFloat2d is implemented as a pointer to a C data structure.
**
** @alias AjSFloat2d
** @alias AjOFloat2d
**
** @new    ajFloat2dNew Default constructor
** @new    ajFloat2dNewL Constructor with reserved size
** @delete ajFloat2dDel Default destructor
** @ass    ajFloat2dGet Retrieve a float from an array
** @mod    ajFloat2dPut Load a float array element
** @cast   ajFloat2dFloat Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSFloat2d {
  ajint Res;
  ajint Len;
  AjPFloat *Ptr;
} AjOFloat2d, *AjPFloat2d;



/* @data AjPFloat3d ***********************************************************
**
** Ajax 3d float object.
**
** Holds a 3d float array with additional data.
** The length is known and held internally.
**
** AjPFloat3d is implemented as a pointer to a C data structure.
**
** @alias AjSFloat3d
** @alias AjOFloat3d
**
** @new    ajFloat3dNew Default constructor
** @new    ajFloat3dNewL Constructor with reserved size
** @delete ajFloat3dDel Default destructor
** @ass    ajFloat3dGet Retrieve a float from an array
** @mod    ajFloat3dPut Load a float array element
** @cast   ajFloat3dFloat Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSFloat3d {
  ajint Res;
  ajint Len;
  AjPFloat2d  *Ptr;
} AjOFloat3d, *AjPFloat3d;



/* @data AjPDouble ************************************************************
**
** Ajax double object.
**
** Holds a double array with additional data.
** The length is known and held internally.
**
** AjPDouble is implemented as a pointer to a C data structure.
**
** @alias AjSDouble
** @alias AjODouble
**
** @new    ajDoubleNew Default constructor
** @new    ajDoubleNewL Constructor with reserved size
** @delete ajDoubleDel Default destructor
** @ass    ajDoubleGet Retrieve a double from an array
** @mod    ajDoublePut Load a double array element
** @cast   ajDoubleDouble Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSDouble {
  ajint Res;
  ajint Len;
  double *Ptr;
} AjODouble, *AjPDouble;



/* @data AjPDouble2d **********************************************************
**
** Ajax 2d double object.
**
** Holds a 2d double array with additional data.
** The length is known and held internally.
**
** AjPDouble2d is implemented as a pointer to a C data structure.
**
** @alias AjSDouble2d
** @alias AjODouble2d
**
** @new    ajDouble2dNew Default constructor
** @new    ajDouble2dNewL Constructor with reserved size
** @delete ajDouble2dDel Default destructor
** @ass    ajDouble2dGet Retrieve a double from an array
** @mod    ajDouble2dPut Load a double array element
** @cast   ajDouble2dDouble Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSDouble2d {
  ajint Res;
  ajint Len;
  AjPDouble *Ptr;
} AjODouble2d, *AjPDouble2d;



/* @data AjPDouble3d **********************************************************
**
** Ajax 3d double object.
**
** Holds a 3d double array with additional data.
** The length is known and held internally.
**
** AjPDouble3d is implemented as a pointer to a C data structure.
**
** @alias AjSDouble3d
** @alias AjODouble3d
**
** @new    ajDouble3dNew Default constructor
** @new    ajDouble3dNewL Constructor with reserved size
** @delete ajDouble3dDel Default destructor
** @ass    ajDouble3dGet Retrieve a double from an array
** @mod    ajDouble3dPut Load a double array element
** @cast   ajDouble3dDouble Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSDouble3d {
  ajint Res;
  ajint Len;
  AjPDouble2d  *Ptr;
} AjODouble3d, *AjPDouble3d;



/* @data AjPShort *************************************************************
**
** Ajax short object.
**
** Holds a short array with additional data.
** The length is known and held internally.
**
** AjPShort is implemented as a pointer to a C data structure.
**
** @alias AjSShort
** @alias AjOShort
**
** @new    ajShortNew Default constructor
** @new    ajShortNewL Constructor with reserved size
** @delete ajShortDel Default destructor
** @ass    ajShortGet Retrieve a short from an array
** @mod    ajShortPut Load a short array element
** @cast   ajShortShort Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSShort {
  ajint Res;
  ajint Len;
  short *Ptr;
} AjOShort, *AjPShort;



/* @data AjPShort2d ***********************************************************
**
** Ajax 2d short object.
**
** Holds a 2d short array with additional data.
** The length is known and held internally.
**
** AjPShort2d is implemented as a pointer to a C data structure.
**
** @alias AjSShort2d
** @alias AjOShort2d
**
** @new    ajShort2dNew Default constructor
** @new    ajShort2dNewL Constructor with reserved size
** @delete ajShort2dDel Default destructor
** @ass    ajShort2dGet Retrieve a short from an array
** @mod    ajShort2dPut Load a short array element
** @cast   ajShort2dShort Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSShort2d {
  ajint Res;
  ajint Len;
  AjPShort *Ptr;
} AjOShort2d, *AjPShort2d;



/* @data AjPShort3d ***********************************************************
**
** Ajax 3d short object.
**
** Holds a 3d short array with additional data.
** The length is known and held internally.
**
** AjPShort3d is implemented as a pointer to a C data structure.
**
** @alias AjSShort3d
** @alias AjOShort3d
**
** @new    ajShort3dNew Default constructor
** @new    ajShort3dNewL Constructor with reserved size
** @delete ajShort3dDel Default destructor
** @ass    ajShort3dGet Retrieve a short from an array
** @mod    ajShort3dPut Load a short array element
** @cast   ajShort3dShort Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSShort3d {
  ajint Res;
  ajint Len;
  AjPShort2d  *Ptr;
} AjOShort3d, *AjPShort3d;



/* @data AjPLong **************************************************************
**
** Ajax ajlong object.
**
** Holds a ajlong array with additional data.
** The length is known and held internally.
**
** AjPLong is implemented as a pointer to a C data structure.
**
** @alias AjSLong
** @alias AjOLong
**
** @new    ajLongNew Default constructor
** @new    ajLongNewL Constructor with reserved size
** @delete ajLongDel Default destructor
** @ass    ajLongGet Retrieve a ajlong from an array
** @mod    ajLongPut Load a ajlong array element
** @cast   ajLongLong Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSLong {
  ajint Res;
  ajint Len;
  ajlong *Ptr;
} AjOLong, *AjPLong;



/* @data AjPLong2d ************************************************************
**
** Ajax 2d ajlong object.
**
** Holds a 2d ajlong array with additional data.
** The length is known and held internally.
**
** AjPLong2d is implemented as a pointer to a C data structure.
**
** @alias AjSLong2d
** @alias AjOLong2d
**
** @new    ajLong2dNew Default constructor
** @new    ajLong2dNewL Constructor with reserved size
** @delete ajLong2dDel Default destructor
** @ass    ajLong2dGet Retrieve a ajlong from an array
** @mod    ajLong2dPut Load a ajlong array element
** @cast   ajLong2dLong Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSLong2d {
  ajint Res;
  ajint Len;
  AjPLong *Ptr;
} AjOLong2d, *AjPLong2d;



/* @data AjPLong3d ************************************************************
**
** Ajax 3d ajlong object.
**
** Holds a 3d ajlong array with additional data.
** The length is known and held internally.
**
** AjPLong3d is implemented as a pointer to a C data structure.
**
** @alias AjSLong3d
** @alias AjOLong3d
**
** @new    ajLong3dNew Default constructor
** @new    ajLong3dNewL Constructor with reserved size
** @delete ajLong3dDel Default destructor
** @ass    ajLong3dGet Retrieve a ajlong from an array
** @mod    ajLong3dPut Load a ajlong array element
** @cast   ajLong3dLong Retrieve internal pointer
** @@
******************************************************************************/

typedef struct AjSLong3d {
  ajint Res;
  ajint Len;
  AjPLong2d  *Ptr;
} AjOLong3d, *AjPLong3d;



/* ========================================================================= */
/* =================== All functions in alphabetical order ================= */
/* ========================================================================= */

void        ajDoubleDel(AjPDouble *thys);
double*     ajDoubleDouble(AjPDouble thys);
double      ajDoubleGet(AjPDouble thys, ajint elem);
ajint       ajDoubleLen(AjPDouble thys);
AjPDouble   ajDoubleNew(void);
AjPDouble   ajDoubleNewL(ajint size);
AjBool      ajDoublePut(AjPDouble *thys, ajint elem, double v);

void        ajDouble2dDel(AjPDouble2d *thys);
double      ajDouble2dGet(AjPDouble2d thys, ajint elem1, ajint elem2);
double**    ajDouble2dDouble(AjPDouble2d thys);
void        ajDouble2dLen(AjPDouble2d thys, ajint *len1, ajint *len2);
AjPDouble2d ajDouble2dNew(void);
AjPDouble2d ajDouble2dNewL(ajint size);
AjBool      ajDouble2dPut(AjPDouble2d *thys,
			  ajint elem1, ajint elem2, double v);

void        ajDouble3dDel(AjPDouble3d *thys);
double      ajDouble3dGet(AjPDouble3d thys,
			  ajint elem1, ajint elem2, ajint elem3);
double***   ajDouble3dDouble(AjPDouble3d thys);
void        ajDouble3dLen(AjPDouble3d thys,
			  ajint* len1, ajint* len2, ajint* len3);
AjPDouble3d ajDouble3dNew(void);
AjPDouble3d ajDouble3dNewL(ajint size);
AjBool      ajDouble3dPut(AjPDouble3d *thys,
			  ajint elem1, ajint elem2, ajint elem3,
			  double v);

void        ajFloatDel(AjPFloat *thys);
float*      ajFloatFloat(AjPFloat thys);
float       ajFloatGet(AjPFloat thys, ajint elem);
ajint       ajFloatLen(AjPFloat thys);
AjPFloat    ajFloatNew(void);
AjPFloat    ajFloatNewL(ajint size);
AjBool      ajFloatPut(AjPFloat *thys, ajint elem, float v);

void        ajFloat2dDel(AjPFloat2d *thys);
float       ajFloat2dGet(AjPFloat2d thys, ajint elem1, ajint elem2);
float**     ajFloat2dFloat(AjPFloat2d thys);
void        ajFloat2dLen(AjPFloat2d thys, ajint *len1, ajint *len2);
AjPFloat2d  ajFloat2dNew(void);
AjPFloat2d  ajFloat2dNewL(ajint size);
AjBool      ajFloat2dPut(AjPFloat2d *thys, ajint elem1, ajint elem2, float v);

void        ajFloat3dDel(AjPFloat3d *thys);
float       ajFloat3dGet(AjPFloat3d thys,
			 ajint elem1, ajint elem2, ajint elem3);
float***    ajFloat3dFloat(AjPFloat3d thys);
void        ajFloat3dLen(AjPFloat3d thys,
			 ajint* len1, ajint* len2, ajint* len3);
AjPFloat3d  ajFloat3dNew(void);
AjPFloat3d  ajFloat3dNewL(ajint size);
AjBool      ajFloat3dPut(AjPFloat3d *thys,
			 ajint elem1, ajint elem2, ajint elem3,
			 float v);

AjPChar     ajChararrNew(void);
AjPChar     ajChararrNewL(ajint size);
void        ajChararrDel(AjPChar *thys);
char        ajChararrGet(AjPChar thys, ajint elem);
AjBool      ajChararrPut(AjPChar *thys, ajint elem, char v);
char*       ajChararrChararr(AjPChar thys);


void        ajIntDel(AjPInt *thys);
void        ajIntDec(AjPInt *thys, ajint elem);
ajint       ajIntGet(AjPInt thys, ajint elem);
void        ajIntInc(AjPInt *thys, ajint elem);
ajint*      ajIntInt(AjPInt thys);
ajint       ajIntLen(AjPInt thys);
AjPInt      ajIntNew(void);
AjPInt      ajIntNewL(ajint size);
AjBool      ajIntPut(AjPInt *thys, ajint elem, ajint v);

void        ajInt2dDel(AjPInt2d *thys);
ajint       ajInt2dGet(AjPInt2d thys, ajint elem1, ajint elem2);
ajint**     ajInt2dInt(AjPInt2d thys);
void        ajInt2dLen(AjPInt2d thys, ajint *len1, ajint *len2);
AjPInt2d    ajInt2dNew(void);
AjPInt2d    ajInt2dNewL(ajint size);
AjBool      ajInt2dPut(AjPInt2d *thys, ajint elem1, ajint elem2, ajint v);

void        ajInt3dDel(AjPInt3d *thys);
ajint       ajInt3dGet(AjPInt3d thys, ajint elem1, ajint elem2, ajint elem3);
ajint***    ajInt3dInt(AjPInt3d thys);
void        ajInt3dLen(AjPInt3d thys, ajint* len1, ajint* len2, ajint* len3);
AjPInt3d    ajInt3dNew(void);
AjPInt3d    ajInt3dNewL(ajint size);
AjBool      ajInt3dPut(AjPInt3d *thys,
		       ajint elem1, ajint elem2, ajint elem3, ajint v);

void        ajLongDel(AjPLong *thys);
ajlong      ajLongGet(AjPLong thys, ajint elem);
ajlong      ajLongLen(AjPLong thys);
ajlong*     ajLongLong(AjPLong thys);
AjPLong     ajLongNew(void);
AjPLong     ajLongNewL(ajint size);
AjBool      ajLongPut(AjPLong *thys, ajint elem, ajlong v);

void        ajLong2dDel(AjPLong2d *thys);
ajlong      ajLong2dGet(AjPLong2d thys, ajint elem1, ajint elem2);
ajlong**    ajLong2dLong(AjPLong2d thys);
void        ajLong2dLen(AjPLong2d thys, ajint *len1, ajint *len2);
AjPLong2d   ajLong2dNew(void);
AjPLong2d   ajLong2dNewL(ajint size);
AjBool      ajLong2dPut(AjPLong2d *thys, ajint elem1, ajint elem2, ajlong v);

void        ajLong3dDel(AjPLong3d *thys);
ajlong      ajLong3dGet(AjPLong3d thys,
			ajint elem1, ajint elem2, ajint elem3);
void        ajLong3dLen(AjPLong3d thys, ajint* len1, ajint* len2, ajint* len3);
ajlong***     ajLong3dLong(AjPLong3d thys);
AjPLong3d   ajLong3dNew(void);
AjPLong3d   ajLong3dNewL(ajint size);
AjBool      ajLong3dPut(AjPLong3d *thys, ajint elem1, ajint elem2, ajint elem3,
			ajlong v);

void        ajShortDel(AjPShort *thys);
short       ajShortGet(AjPShort thys, ajint elem);
ajint       ajShortLen(AjPShort thys);
short*      ajShortShort(AjPShort thys);
AjPShort    ajShortNew(void);
AjPShort    ajShortNewL(ajint size);
AjBool      ajShortPut(AjPShort *thys, ajint elem, short v);

void        ajShort2dDel(AjPShort2d *thys);
short       ajShort2dGet(AjPShort2d thys, ajint elem1, ajint elem2);
short**     ajShort2dShort(AjPShort2d thys);
void        ajShort2dLen(AjPShort2d thys, ajint *len1, ajint *len2);
AjPShort2d  ajShort2dNew(void);
AjPShort2d  ajShort2dNewL(ajint size);
AjBool      ajShort2dPut(AjPShort2d *thys, ajint elem1, ajint elem2, short v);

void        ajShort3dDel(AjPShort3d *thys);
short       ajShort3dGet(AjPShort3d thys,
			 ajint elem1, ajint elem2, ajint elem3);
void        ajShort3dLen(AjPShort3d thys,
			 ajint* len1, ajint* len2, ajint* len3);
short***    ajShort3dShort(AjPShort3d thys);
AjPShort3d  ajShort3dNew(void);
AjPShort3d  ajShort3dNewL(ajint size);
AjBool      ajShort3dPut(AjPShort3d *thys,
			 ajint elem1, ajint elem2, ajint elem3,
			 short v);


AjBool      ajFloatParse (AjPStr str, AjPFloat *array);
void        ajFloatStr (AjPStr* str, AjPFloat array, ajint precision);
void        ajFloatTrace (AjPFloat array, ajint precision, char* text);

ajint       ajArrCommaList(AjPStr s, AjPStr **a);
double*     ajArrDoubleLine(AjPStr *line, const char *delim, ajint cols,
			    ajint startcol, ajint endcol);
ajint*      ajArrIntLine(AjPStr *line, const char *delim, ajint cols,
			 ajint startcol, ajint endcol);
float*      ajArrFloatLine(AjPStr *line, const char *delim, ajint cols,
			   ajint startcol, ajint endcol);
#endif

#ifdef __cplusplus
}
#endif

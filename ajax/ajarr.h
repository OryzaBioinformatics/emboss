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
** Saves on length calculation, and allows growth in reserved memory without
** changing the pointer in the calling routine.
**
** AjPChar is implemented as a pointer to a C data structure.
**
** @alias AjSChar
** @alias AjOChar
**
** @new    ajChararrNew Default constructor
** @new    ajChararrNewL Constructor with reserved size
** @delete ajChararrDel Default destructor
** @cast   ajChararrGet Retrieve a character from an array
** @modify ajChararrPut Load a character array element
** @cast   ajChararrChararr Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [char*] Array of characters
** @@
******************************************************************************/

typedef struct AjSChar {
  ajint Res;
  ajint Len;
  char *Ptr;
} AjOChar;
#define AjPChar AjOChar*



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
** @cast   ajIntGet Retrieve an integer from an array
** @modify ajIntPut Load an integer array element
** @cast   ajIntInt Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [ajint*] Array of integers
** @@
******************************************************************************/

typedef struct AjSInt {
  ajint Res;
  ajint Len;
  ajint *Ptr;
} AjOInt;
#define AjPInt AjOInt*



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
** @cast   ajInt2dGet Retrieve an integer from an array
** @modify ajInt2dPut Load an integer array element
** @cast   ajInt2dInt Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPInt*] Array of integer arrays
** @@
******************************************************************************/

typedef struct AjSInt2d {
  ajint Res;
  ajint Len;
  AjPInt *Ptr;
} AjOInt2d;
#define AjPInt2d AjOInt2d*



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
** @cast   ajInt3dGet Retrieve an integer from an array
** @modify ajInt3dPut Load an integer array element
** @cast   ajInt3dInt Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPInt2d*] Array of 2d integer arrays
** @@
******************************************************************************/

typedef struct AjSInt3d {
  ajint Res;
  ajint Len;
  AjPInt2d *Ptr;
} AjOInt3d;
#define AjPInt3d AjOInt3d*



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
** @cast   ajFloatGet Retrieve a float from an array
** @modify ajFloatPut Load a float array element
** @cast   ajFloatFloat Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [float*] Array of floats
** @@
******************************************************************************/

typedef struct AjSFloat {
  ajint Res;
  ajint Len;
  float *Ptr;
} AjOFloat;
#define AjPFloat AjOFloat*



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
** @cast   ajFloat2dGet Retrieve a float from an array
** @modify ajFloat2dPut Load a float array element
** @cast   ajFloat2dFloat Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPFloat*] Array of float arrays
** @@
******************************************************************************/

typedef struct AjSFloat2d {
  ajint Res;
  ajint Len;
  AjPFloat *Ptr;
} AjOFloat2d;
#define AjPFloat2d AjOFloat2d*



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
** @cast   ajFloat3dGet Retrieve a float from an array
** @modify ajFloat3dPut Load a float array element
** @cast   ajFloat3dFloat Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPFloat2d*] Array of 2d float arrays
** @@
******************************************************************************/

typedef struct AjSFloat3d {
  ajint Res;
  ajint Len;
  AjPFloat2d  *Ptr;
} AjOFloat3d;
#define AjPFloat3d AjOFloat3d*



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
** @cast   ajDoubleGet Retrieve a double from an array
** @modify ajDoublePut Load a double array element
** @cast   ajDoubleDouble Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [double*] Array of doubles
** @@
******************************************************************************/

typedef struct AjSDouble {
  ajint Res;
  ajint Len;
  double *Ptr;
} AjODouble;
#define AjPDouble AjODouble*



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
** @cast   ajDouble2dGet Retrieve a double from an array
** @modify ajDouble2dPut Load a double array element
** @cast   ajDouble2dDouble Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPDouble*] Array of double arrays
** @@
******************************************************************************/

typedef struct AjSDouble2d {
  ajint Res;
  ajint Len;
  AjPDouble *Ptr;
} AjODouble2d;
#define AjPDouble2d AjODouble2d*



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
** @cast ajDouble3dGet Retrieve a double from an array
** @modify ajDouble3dPut Load a double array element
** @cast   ajDouble3dDouble Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPDouble2d*] Array of 2d double arrays
** @@
******************************************************************************/

typedef struct AjSDouble3d {
  ajint Res;
  ajint Len;
  AjPDouble2d  *Ptr;
} AjODouble3d;
#define AjPDouble3d AjODouble3d*



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
** @cast   ajShortGet Retrieve a short from an array
** @modify ajShortPut Load a short array element
** @cast   ajShortShort Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [short*] Array of shorts
** @@
******************************************************************************/

typedef struct AjSShort {
  ajint Res;
  ajint Len;
  short *Ptr;
} AjOShort;
#define AjPShort AjOShort*



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
** @cast   ajShort2dGet Retrieve a short from an array
** @modify ajShort2dPut Load a short array element
** @cast   ajShort2dShort Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPShort*] Array of short arrays
** @@
******************************************************************************/

typedef struct AjSShort2d {
  ajint Res;
  ajint Len;
  AjPShort *Ptr;
} AjOShort2d;
#define AjPShort2d AjOShort2d*



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
** @cast   ajShort3dGet Retrieve a short from an array
** @modify ajShort3dPut Load a short array element
** @cast   ajShort3dShort Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPShort2d*] Array of 2d short arrays
** @@
******************************************************************************/

typedef struct AjSShort3d {
  ajint Res;
  ajint Len;
  AjPShort2d  *Ptr;
} AjOShort3d;
#define AjPShort3d AjOShort3d*



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
** @cast   ajLongGet Retrieve a ajlong from an array
** @modify ajLongPut Load a ajlong array element
** @cast   ajLongLong Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [ajlong*] Array of longs
** @@
******************************************************************************/

typedef struct AjSLong {
  ajint Res;
  ajint Len;
  ajlong *Ptr;
} AjOLong;
#define AjPLong AjOLong*



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
** @cast   ajLong2dGet Retrieve a ajlong from an array
** @modify ajLong2dPut Load a ajlong array element
** @cast   ajLong2dLong Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPLong*] Array of long arrays
** @@
******************************************************************************/

typedef struct AjSLong2d {
  ajint Res;
  ajint Len;
  AjPLong *Ptr;
} AjOLong2d;
#define AjPLong2d AjOLong2d*



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
** @cast   ajLong3dGet Retrieve a ajlong from an array
** @modify ajLong3dPut Load a ajlong array element
** @cast   ajLong3dLong Retrieve internal pointer
**
** @attr Res [ajint] Reserved space in case of extension
** @attr Len [ajint] Actual length used
** @attr Ptr [AjPLong2d*] Array of 2d long arrays
** @@
******************************************************************************/

typedef struct AjSLong3d {
  ajint Res;
  ajint Len;
  AjPLong2d  *Ptr;
} AjOLong3d;
#define AjPLong3d AjOLong3d*



/* ========================================================================= */
/* =================== All functions in alphabetical order ================= */
/* ========================================================================= */

void        ajDoubleDel(AjPDouble *thys);
double*     ajDoubleDouble(const AjPDouble thys);
double      ajDoubleGet(const AjPDouble thys, ajint elem);
ajint       ajDoubleLen(const AjPDouble thys);
AjPDouble   ajDoubleNew(void);
AjPDouble   ajDoubleNewL(ajint size);
AjBool      ajDoublePut(AjPDouble *thys, ajint elem, double v);

void        ajDouble2dDel(AjPDouble2d *thys);
double      ajDouble2dGet(const AjPDouble2d thys, ajint elem1, ajint elem2);
double**    ajDouble2dDouble(const AjPDouble2d thys);
void        ajDouble2dLen(const AjPDouble2d thys, ajint *len1, ajint *len2);
AjPDouble2d ajDouble2dNew(void);
AjPDouble2d ajDouble2dNewL(ajint size);
AjBool      ajDouble2dPut(AjPDouble2d *thys,
			  ajint elem1, ajint elem2, double v);

void        ajDouble3dDel(AjPDouble3d *thys);
double      ajDouble3dGet(const AjPDouble3d thys,
			  ajint elem1, ajint elem2, ajint elem3);
double***   ajDouble3dDouble(const AjPDouble3d thys);
void        ajDouble3dLen(const AjPDouble3d thys,
			  ajint* len1, ajint* len2, ajint* len3);
AjPDouble3d ajDouble3dNew(void);
AjPDouble3d ajDouble3dNewL(ajint size);
AjBool      ajDouble3dPut(AjPDouble3d *thys,
			  ajint elem1, ajint elem2, ajint elem3,
			  double v);

void        ajFloatDel(AjPFloat *thys);
float*      ajFloatFloat(const AjPFloat thys);
float       ajFloatGet(const AjPFloat thys, ajint elem);
ajint       ajFloatLen(const AjPFloat thys);
AjPFloat    ajFloatNew(void);
AjPFloat    ajFloatNewL(ajint size);
AjBool      ajFloatPut(AjPFloat *thys, ajint elem, float v);

void        ajFloat2dDel(AjPFloat2d *thys);
float       ajFloat2dGet(const AjPFloat2d thys, ajint elem1, ajint elem2);
float**     ajFloat2dFloat(const AjPFloat2d thys);
void        ajFloat2dLen(const AjPFloat2d thys, ajint *len1, ajint *len2);
AjPFloat2d  ajFloat2dNew(void);
AjPFloat2d  ajFloat2dNewL(ajint size);
AjBool      ajFloat2dPut(AjPFloat2d *thys, ajint elem1, ajint elem2, float v);

void        ajFloat3dDel(AjPFloat3d *thys);
float       ajFloat3dGet(const AjPFloat3d thys,
			 ajint elem1, ajint elem2, ajint elem3);
float***    ajFloat3dFloat(const AjPFloat3d thys);
void        ajFloat3dLen(const AjPFloat3d thys,
			 ajint* len1, ajint* len2, ajint* len3);
AjPFloat3d  ajFloat3dNew(void);
AjPFloat3d  ajFloat3dNewL(ajint size);
AjBool      ajFloat3dPut(AjPFloat3d *thys,
			 ajint elem1, ajint elem2, ajint elem3,
			 float v);

AjPChar     ajChararrNew(void);
AjPChar     ajChararrNewL(ajint size);
void        ajChararrDel(AjPChar *thys);
char        ajChararrGet(const AjPChar thys, ajint elem);
AjBool      ajChararrPut(AjPChar *thys, ajint elem, char v);
char*       ajChararrChararr(const AjPChar thys);


void        ajIntDel(AjPInt *thys);
void        ajIntDec(AjPInt *thys, ajint elem);
ajint       ajIntGet(const AjPInt thys, ajint elem);
void        ajIntInc(AjPInt *thys, ajint elem);
ajint*      ajIntInt(const AjPInt thys);
ajint       ajIntLen(const AjPInt thys);
AjPInt      ajIntNew(void);
AjPInt      ajIntNewL(ajint size);
AjBool      ajIntPut(AjPInt *thys, ajint elem, ajint v);

void        ajInt2dDel(AjPInt2d *thys);
ajint       ajInt2dGet(const AjPInt2d thys, ajint elem1, ajint elem2);
ajint**     ajInt2dInt(const AjPInt2d thys);
void        ajInt2dLen(const AjPInt2d thys, ajint *len1, ajint *len2);
AjPInt2d    ajInt2dNew(void);
AjPInt2d    ajInt2dNewL(ajint size);
AjBool      ajInt2dPut(AjPInt2d *thys, ajint elem1, ajint elem2, ajint v);

void        ajInt3dDel(AjPInt3d *thys);
ajint       ajInt3dGet(const AjPInt3d thys, ajint elem1, ajint elem2,
		       ajint elem3);
ajint***    ajInt3dInt(const AjPInt3d thys);
void        ajInt3dLen(const AjPInt3d thys,
		       ajint* len1, ajint* len2, ajint* len3);
AjPInt3d    ajInt3dNew(void);
AjPInt3d    ajInt3dNewL(ajint size);
AjBool      ajInt3dPut(AjPInt3d *thys,
		       ajint elem1, ajint elem2, ajint elem3, ajint v);

void        ajLongDel(AjPLong *thys);
ajlong      ajLongGet(const AjPLong thys, ajint elem);
ajlong      ajLongLen(const AjPLong thys);
ajlong*     ajLongLong(const AjPLong thys);
AjPLong     ajLongNew(void);
AjPLong     ajLongNewL(ajint size);
AjBool      ajLongPut(AjPLong *thys, ajint elem, ajlong v);

void        ajLong2dDel(AjPLong2d *thys);
ajlong      ajLong2dGet(const AjPLong2d thys, ajint elem1, ajint elem2);
ajlong**    ajLong2dLong(const AjPLong2d thys);
void        ajLong2dLen(const AjPLong2d thys, ajint *len1, ajint *len2);
AjPLong2d   ajLong2dNew(void);
AjPLong2d   ajLong2dNewL(ajint size);
AjBool      ajLong2dPut(AjPLong2d *thys, ajint elem1, ajint elem2, ajlong v);

void        ajLong3dDel(AjPLong3d *thys);
ajlong      ajLong3dGet(const AjPLong3d thys,
			ajint elem1, ajint elem2, ajint elem3);
void        ajLong3dLen(const AjPLong3d thys, 
			ajint* len1, ajint* len2, ajint* len3);
ajlong***   ajLong3dLong(const AjPLong3d thys);
AjPLong3d   ajLong3dNew(void);
AjPLong3d   ajLong3dNewL(ajint size);
AjBool      ajLong3dPut(AjPLong3d *thys, ajint elem1, ajint elem2, ajint elem3,
			ajlong v);

void        ajShortDel(AjPShort *thys);
short       ajShortGet(const AjPShort thys, ajint elem);
ajint       ajShortLen(const AjPShort thys);
short*      ajShortShort(const AjPShort thys);
AjPShort    ajShortNew(void);
AjPShort    ajShortNewL(ajint size);
AjBool      ajShortPut(AjPShort *thys, ajint elem, short v);

void        ajShort2dDel(AjPShort2d *thys);
short       ajShort2dGet(const AjPShort2d thys, ajint elem1, ajint elem2);
short**     ajShort2dShort(const AjPShort2d thys);
void        ajShort2dLen(const AjPShort2d thys, ajint *len1, ajint *len2);
AjPShort2d  ajShort2dNew(void);
AjPShort2d  ajShort2dNewL(ajint size);
AjBool      ajShort2dPut(AjPShort2d *thys, ajint elem1, ajint elem2, short v);

void        ajShort3dDel(AjPShort3d *thys);
short       ajShort3dGet(const AjPShort3d thys,
			 ajint elem1, ajint elem2, ajint elem3);
void        ajShort3dLen(const AjPShort3d thys,
			 ajint* len1, ajint* len2, ajint* len3);
short***    ajShort3dShort(const AjPShort3d thys);
AjPShort3d  ajShort3dNew(void);
AjPShort3d  ajShort3dNewL(ajint size);
AjBool      ajShort3dPut(AjPShort3d *thys,
			 ajint elem1, ajint elem2, ajint elem3,
			 short v);


AjBool      ajFloatParse (const AjPStr str, AjPFloat *array);
void        ajFloatStr (const AjPFloat array, ajint precision, AjPStr* str);
void        ajFloatTrace (const AjPFloat array, ajint precision,
			  const char* text);

ajint       ajArrCommaList(const  AjPStr s, AjPStr **a);
double*     ajArrDoubleLine(const AjPStr line, const char *delim, ajint cols,
			    ajint startcol, ajint endcol);
ajint*      ajArrIntLine(const AjPStr line, const char *delim, ajint cols,
			 ajint startcol, ajint endcol);
float*      ajArrFloatLine(const AjPStr line, const char *delim, ajint cols,
			   ajint startcol, ajint endcol);
#endif

#ifdef __cplusplus
}
#endif

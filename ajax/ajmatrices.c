/******************************************************************************
** @source AJAX matrices functions
**
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






/* @func ajMatrixNew **********************************************************
**
** Creates a new, zero matrix from a text string of defined sequence
** characters and a matrix name.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [char*] Valid sequence character codes
** @param [r] filename [AjPStr] Matrix filename
** @return [AjPMatrix] New matrix
** @@
******************************************************************************/

AjPMatrix ajMatrixNew (char* codes, AjPStr filename) {
  ajint isize;
  ajint i;

  AjPMatrix ret;

  AJNEW0(ret);

  (void) ajStrAssS(&ret->Name, filename);

  (void) ajStrAssC (&ret->Codes, codes);
  isize = 1+ajStrLen(ret->Codes); /* zero for invalid codes */
  ret->Size = isize;

  AJCNEW0(ret->Matrix, isize);
  for (i=0; i<isize; i++) {
    AJCNEW0(ret->Matrix[i], isize);
  }
  ret->Cvt = ajSeqCvtNewZero (codes);

  return ret;
}

/* @func ajMatrixfNew *********************************************************
**
** Creates a new, zero matrix from a text string of defined sequence
** characters and a matrix name.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [char*] Valid sequence character codes
** @param [r] filename [AjPStr] Matrix filename
** @return [AjPMatrixf] New matrix
** @@
******************************************************************************/

AjPMatrixf ajMatrixfNew (char* codes, AjPStr filename) {
  ajint isize;
  ajint i;

  AjPMatrixf ret;

  AJNEW0(ret);

  (void) ajStrAssS(&ret->Name, filename);

  (void) ajStrAssC (&ret->Codes, codes);
  isize = 1+ajStrLen(ret->Codes); /* zero for invalid codes */
  ret->Size = isize;

  AJCNEW0(ret->Matrixf, isize);
  for (i=0; i<isize; i++) {
    AJCNEW0(ret->Matrixf[i], isize);
  }
  ret->Cvt = ajSeqCvtNewZero (codes);

  return ret;
}

/* @func ajMatrixfDel *********************************************************
**
** Delete a float matrix
**
** @param [w] thys [AjPMatrixf*] Matrix to delete
** @return [void]
** @@
******************************************************************************/

void ajMatrixfDel (AjPMatrixf *thys)
{
  ajint isize;
  ajint i;

  if(!*thys || !thys)
      return;

  ajStrDel(&(*thys)->Codes);
  ajStrDel(&(*thys)->Name);
  isize = (*thys)->Size;
  for(i=0;i<isize;++i)
      AJFREE( (*thys)->Matrixf[i]);
  AJFREE((*thys)->Matrixf);

  ajSeqCvtDel(&(*thys)->Cvt);
  AJFREE(*thys);

  return;
}

/* @func ajMatrixDel **********************************************************
**
** Delete an integer matrix
**
** @param [w] thys [AjPMatrix*] Matrix to delete
** @return [void]
** @@
******************************************************************************/

void ajMatrixDel (AjPMatrix *thys)
{
  ajint isize;
  ajint i;

  if(!*thys || !thys)
      return;

  ajStrDel(&(*thys)->Codes);
  ajStrDel(&(*thys)->Name);
  isize = (*thys)->Size;
  for(i=0;i<isize;++i)
      AJFREE( (*thys)->Matrix[i]);
  AJFREE((*thys)->Matrix);

  ajSeqCvtDel(&(*thys)->Cvt);
  AJFREE(*thys);

  return;
}

/* @func ajMatrixArray ********************************************************
**
** Returns the comparison matrix as an array of integer arrays.
** Sequence characters are indexed in this array using the internal
** Sequence Conversion table in the matrix (see ajMatrixCvt)
**
** @param [r] thys [AjPMatrix] Matrix object
** @return [ajint**] array of integer arrays for comparison values.
** @@
******************************************************************************/

ajint** ajMatrixArray (AjPMatrix thys) {
  return thys->Matrix;
}

/* @func ajMatrixfArray *******************************************************
**
** Returns the comparison matrix as an array of float arrays.
** Sequence characters are indexed in this array using the internal
** Sequence Conversion table in the matrix (see ajMatrixCvt)
**
** @param [r] thys [AjPMatrixf] Float Matrix object
** @return [float**] array of float arrays for comparison values.
** @@
******************************************************************************/

float** ajMatrixfArray (AjPMatrixf thys) {
  return thys->Matrixf;
}

/* @func ajMatrixSize *********************************************************
**
** Returns the comparison matrix size.
**
** @param [r] thys [AjPMatrix] Matrix object
** @return [ajint] .
** @@
******************************************************************************/

ajint ajMatrixSize (AjPMatrix thys) {
  return thys->Size;
}

/* @func ajMatrixfSize ********************************************************
**
** Returns the comparison matrix size.
**
** @param [r] thys [AjPMatrixf] Matrix object
** @return [ajint] .
** @@
******************************************************************************/

ajint ajMatrixfSize (AjPMatrixf thys) {
  return thys->Size;
}

/* @func ajMatrixCvt **********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [AjPMatrix] Matrix object
** @return [AjPSeqCvt] sequence character conversion table
** @@
******************************************************************************/

AjPSeqCvt ajMatrixCvt (AjPMatrix thys) {
  return thys->Cvt;
}

/* @func ajMatrixfCvt *********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [AjPMatrixf] Float Matrix object
** @return [AjPSeqCvt] sequence character conversion table
** @@
******************************************************************************/

AjPSeqCvt ajMatrixfCvt (AjPMatrixf thys) {
  return thys->Cvt;
}

/* @func ajMatrixChar *********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [AjPMatrix] Matrix object
** @param [r] i [ajint] Character index
** @return [char] sequence character code
** @@
******************************************************************************/

char ajMatrixChar (AjPMatrix thys, ajint i) {
  if (i >= thys->Size) return '?';
  if (i < 0) return '?';
  return ajStrChar(thys->Codes,i);
}

/* @func ajMatrixfChar ********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [AjPMatrixf] Matrix object
** @param [r] i [ajint] Character index
** @return [char] sequence character code
** @@
******************************************************************************/

char ajMatrixfChar (AjPMatrixf thys, ajint i) {
  if (i >= thys->Size) return '?';
  if (i < 0) return '?';
  return ajStrChar(thys->Codes,i);
}

/* @func ajMatrixName *********************************************************
**
** Returns the name of a matrix object, usually the filename from
** which it was read.
**
** @param [r] thys [AjPMatrix] Matrix object
** @return [AjPStr] The name, a pointer to the internal name.
** @@
******************************************************************************/

AjPStr ajMatrixName (AjPMatrix thys) {
  return thys->Name;
}

/* @func ajMatrixfName ********************************************************
**
** Returns the name of a matrix object, usually the filename from
** which it was read.
**
** @param [r] thys [AjPMatrixf] Matrix object
** @return [AjPStr] The name, a pointer to the internal name.
** @@
******************************************************************************/

AjPStr ajMatrixfName (AjPMatrixf thys) {
  return thys->Name;
}

/* @func ajMatrixRead *********************************************************
**
** Constructs a comparison matrix from a given local data file
**
** @param [w] pthis [AjPMatrix*] New Matrix object.
** @param [r] filename [AjPStr] Input filename
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixRead (AjPMatrix* pthis, AjPStr filename) {

  AjPStr buffer= NULL;
  AjPStr delim = NULL;
  AjPFile file = NULL;
  char *ptr;
  ajint i = 0;
  ajint l=0,k;
  AjBool first = ajTrue;
  ajint cols=0;
  ajint *templine;
  char* orderstring=NULL;
  AjPMatrix thys=NULL;

  ajint** matrix=NULL;
  ajint minval=-1;

  delim = ajStrNewC(" :\t\n");

  ajFileDataNew(filename,&file);

  if(!file)
    return ajFalse;

  while (ajFileGets(file,&buffer)){
    ptr = ajStrStr(buffer);
    if(*ptr != '#' && *ptr != '\n'){ /* not a comment */
      if(first){
        cols = ajStrTokenCount(&buffer,ajStrStr(delim));
	orderstring = ajCharNewL(cols+1);
        first = ajFalse;
        while(*ptr){
          if(!isspace((ajint)*ptr)){
            orderstring[l++]= *ptr;
          }
          ptr++;
        }
        orderstring[l] = '\0';
	thys = *pthis = ajMatrixNew (orderstring, filename);
	matrix = thys->Matrix;
      }
      else{
        k = ajSeqCvtK(thys->Cvt, *ptr);
	/*	(void) printf("-%d-",k);*/
        templine = ajArrIntLine(&buffer,ajStrStr(delim),cols,2,cols);

        ptr = orderstring;
        for(i=0;i<cols-1;i++){
	  /*	  (void) printf(" %d",ajAZToInt(*ptr));*/
	  if (templine[i] < minval) minval=templine[i];
          matrix[k][ajSeqCvtK(thys->Cvt,*ptr)] = templine[i];
          ptr++;
        }
	/*	(void) printf("\n");*/
        AJFREE(templine);
      }
    }
  }
  ajDebug ("fill rest with minimum value %d\n", minval);
  for(i=0;i<cols-1;i++){
    matrix[0][i] = minval;
    matrix[i][0] = minval;
  }

  ajFileClose(&file);
  ajStrDel(&buffer);
  ajStrDel(&delim);
  (void) ajCharFree(orderstring);

  ajDebug("read matrix file %S\n", filename);

  return ajTrue;
}

/* @func ajMatrixfRead ********************************************************
**
** Constructs a comparison matrix from a given local data file
**
** @param [w] pthis [AjPMatrixf*] New Float Matrix object.
** @param [r] filename [AjPStr] Input filename
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixfRead (AjPMatrixf* pthis, AjPStr filename) {

  AjPStr buffer= NULL;
  AjPStr delim = NULL;
  AjPFile file = NULL;
  char *ptr;
  ajint i = 0;
  ajint l=0,k;
  AjBool first = ajTrue;
  ajint cols=0;
  float *templine;
  char* orderstring=NULL;
  AjPMatrixf thys=NULL;

  float** matrix=NULL;
  float minval=-1.0;

  delim = ajStrNewC(" :\t\n");

  ajFileDataNew(filename,&file);

  if(!file)
    return ajFalse;

  while (ajFileGets(file,&buffer)){
    ptr = ajStrStr(buffer);
    if(*ptr != '#' && *ptr != '\n'){ /* not a comment */
      if(first){
        cols = ajStrTokenCount(&buffer,ajStrStr(delim));
	orderstring = ajCharNewL(cols+1);
        first = ajFalse;
        while(*ptr){
          if(!isspace((ajint)*ptr)){
            orderstring[l++]= *ptr;
          }
          ptr++;
        }
        orderstring[l] = '\0';
	thys = *pthis = ajMatrixfNew (orderstring, filename);
	matrix = thys->Matrixf;
      }
      else{
        k = ajSeqCvtK(thys->Cvt, *ptr);
	/*	printf("-%d-",k);*/
        templine = ajArrFloatLine(&buffer,ajStrStr(delim),cols,2,cols);

        ptr = orderstring;
        for(i=0;i<cols-1;i++){
	  if (templine[i] < minval) minval=templine[i];
          matrix[k][ajSeqCvtK(thys->Cvt,*ptr)] = templine[i];
          ptr++;
        }
        AJFREE(templine);
      }
    }
  }
  ajDebug ("fill rest with minimum value %d\n", minval);
  for(i=0;i<cols-1;i++){
    matrix[0][i] = minval;
    matrix[i][0] = minval;
  }

  ajFileClose(&file);
  ajStrDel(&buffer);
  ajStrDel(&delim);
  (void) ajCharFree(orderstring);

  ajDebug("read matrix file %S\n", filename);

  return ajTrue;
}

/* @func ajMatrixSeqNum *******************************************************
**
** Converts a sequence top index numbers using the matrix's
** internal conversion table. Sequence characters not defined
** in the matrix are converted to zero.
**
** @param [r] thys [AjPMatrix] Matrix object
** @param [r] seq [AjPSeq] Sequence object
** @param [w] numseq [AjPStr*] Index code version of the sequence
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixSeqNum (AjPMatrix thys, AjPSeq seq, AjPStr* numseq) {

  return ajSeqNum (seq, thys->Cvt, numseq);
}

/* @func ajMatrixfSeqNum ******************************************************
**
** Converts a sequence to index numbers using the matrix's
** internal conversion table. Sequence characters not defined
** in the matrix are converted to zero.
**
** @param [r] thys [AjPMatrixf] Float Matrix object
** @param [r] seq [AjPSeq] Sequence object
** @param [w] numseq [AjPStr*] Index code version of the sequence
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajMatrixfSeqNum (AjPMatrixf thys, AjPSeq seq, AjPStr* numseq) {

  return ajSeqNum (seq, thys->Cvt, numseq);
}

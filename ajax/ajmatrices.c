/******************************************************************************
** @source AJAX matrices functions
**
** @version 1.0
** @author Copyright (C) 2003 Alan Bleasby
** @author Copyright (C) 2003 Peter Rice
** @@
** @modified Copyright (C) 2003 Jon Ison. Rewritten for string matrix labels
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
** Creates a new, zero matrix from an array of strings and a matrix name. If 
** the matrix is a residue substitution matrix then each string would be a 
** defined sequence character.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [AjPStr*] Matrix labels, e.g. valid sequence
** character codes
** @param [r] n [ajint] Number of labels
** @param [r] filename [AjPStr] Matrix filename
** @return [AjPMatrix] New matrix, or NULL if codes, n or filename are 0.
** @@
******************************************************************************/

AjPMatrix ajMatrixNew(AjPStr* codes, ajint n, AjPStr filename)
{
    ajint     i   = 0;
    AjPMatrix ret = NULL;
    ajint nsize;

    if((!n) || (!codes) || (!filename))
	return NULL;

    nsize = n + 1;

    AJNEW0(ret);

    ajStrAssS(&ret->Name, filename);

    AJCNEW0(ret->Codes, n);
    for(i=0; i<n; i++)
	ret->Codes[i] = ajStrNew();
    
    for(i=0; i<n; i++)
	ajStrAssS(&ret->Codes[i], codes[i]);

    ret->Size = nsize;

    AJCNEW0(ret->Matrix, nsize);
    for(i=0; i<nsize; i++)
	AJCNEW0(ret->Matrix[i], nsize);
    ret->Cvt = ajSeqCvtNewZeroS(codes, n);

    return ret;
}




/* @func ajMatrixfNew *********************************************************
**
** Creates a new, zero matrix from an array of strings and a matrix name. If 
** the matrix is a residue substitution matrix then each string would be a 
** defined sequence character.
**
** The matrix comparison value table Matrix is created and initialised
** with zeroes.
**
** @param [r] codes [AjPStr*] Matrix labels, e.g. valid sequence char codes
** @param [r] n [ajint] Number of labels
** @param [r] filename [AjPStr] Matrix filename
** @return [AjPMatrixf] New matrix, or NULL if codes, n or filename are 0.
** @@
******************************************************************************/

AjPMatrixf ajMatrixfNew(AjPStr* codes, ajint n, AjPStr filename)
{
    ajint i = 0;
    AjPMatrixf ret = 0;
    ajint nsize;
 
    if((!n) || (!codes) || (!filename))
	return NULL;

    nsize = n + 1;

    AJNEW0(ret);

    ajStrAssS(&ret->Name, filename);

    AJCNEW0(ret->Codes, n);
    for(i=0; i<n; i++)
	ret->Codes[i] = ajStrNew();

    for(i=0; i<n; i++)
	ajStrAssS(&ret->Codes[i], codes[i]);

    ret->Size = nsize;

    AJCNEW0(ret->Matrixf, nsize);
    for(i=0; i<nsize; i++)
	AJCNEW0(ret->Matrixf[i], nsize);
    ret->Cvt = ajSeqCvtNewZeroS(codes, n);

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

void ajMatrixfDel(AjPMatrixf *thys)
{
    ajint isize = 0;
    ajint jsize = 0;
    ajint i     = 0;


    if(!*thys || !thys)
	return;

    isize = (*thys)->Size;
    jsize = isize - 1;
    for(i=0; i<jsize; ++i)
	ajStrDel(&(*thys)->Codes[i]);
    AJFREE((*thys)->Codes);

    ajStrDel(&(*thys)->Name);
    for(i=0; i<isize; ++i)
	AJFREE((*thys)->Matrixf[i]);
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

void ajMatrixDel(AjPMatrix *thys)
{
    ajint isize = 0;
    ajint jsize = 0;
    ajint i     = 0;


    if(!*thys || !thys)
	return;

    isize = (*thys)->Size;
    jsize = isize - 1;
    for(i=0; i<jsize; ++i)
	ajStrDel(&(*thys)->Codes[i]);
    AJFREE((*thys)->Codes);

    ajStrDel(&(*thys)->Name);
    for(i=0; i<isize; ++i)
	AJFREE((*thys)->Matrix[i]);
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

ajint** ajMatrixArray(AjPMatrix thys)
{
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

float** ajMatrixfArray(AjPMatrixf thys)
{
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

ajint ajMatrixSize(AjPMatrix thys)
{
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

ajint ajMatrixfSize(AjPMatrixf thys)
{
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

AjPSeqCvt ajMatrixCvt(AjPMatrix thys)
{
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

AjPSeqCvt ajMatrixfCvt(AjPMatrixf thys)
{
    return thys->Cvt;
}




/* @func ajMatrixChar *********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any string defined in the matrix to a
** positive integer, and any other string is converted to zero.
**
** @param [r] thys [AjPMatrix] Matrix object
** @param [r] i [ajint] Character index
** @param [w] label [AjPStr *] Matrix label, e.g. sequence character code
** @return [void] 
** @@
******************************************************************************/

void ajMatrixChar(AjPMatrix thys, ajint i, AjPStr *label)
{
    if(i >= thys->Size)
    {
	ajStrAssC(label, "?");
	return;
    }

    if(i < 0) 
    {
	ajStrAssC(label, "?");
	return;
    }

    ajStrAssS(label, thys->Codes[i]);

    return;
}




/* @func ajMatrixfChar ********************************************************
**
** Returns the sequence character conversion table for a matrix.
** This table converts any character defined in the matrix to a
** positive integer, and any other character is converted to zero.
**
** @param [r] thys [AjPMatrixf] Matrix object
** @param [r] i [ajint] Character index
** @param [w] label [AjPStr *] Matrix label, e.g. sequence character code
** @return [void] 
** @@
******************************************************************************/

void ajMatrixfChar(AjPMatrixf thys, ajint i, AjPStr *label)
{
    if(i >= thys->Size) 
    {	
	ajStrAssC(label, "?");
	return;
    }
    
    if(i < 0)
    {
	ajStrAssC(label, "?");
	return;
    }

    ajStrAssS(label, thys->Codes[i]);

    return;
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

AjPStr ajMatrixName(AjPMatrix thys)
{
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

AjPStr ajMatrixfName(AjPMatrixf thys)
{
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

AjBool ajMatrixRead(AjPMatrix* pthis, AjPStr filename)
{
    AjPStr buffer = NULL;
    AjPStr delim  = NULL;
    AjPStr tok    = NULL;

    AjPStr firststring  = NULL;
    AjPStr *orderstring = NULL;

    AjPFile file    = NULL;
    AjBool first    = ajTrue;
    AjPMatrix  thys = NULL;
    char *ptr       = NULL;
    ajint **matrix  = NULL;

    ajint minval = -1;
    ajint i      = 0;
    ajint l      = 0;
    ajint k      = 0;
    ajint cols   = 0;

    ajint *templine = NULL;

    firststring = ajStrNew();
    
    delim = ajStrNewC(" :\t\n");
    
    ajFileDataNew(filename,&file);
    
    if(!file)
    {
	ajStrDel(&firststring);
	return ajFalse;
    }
    
    
    while(ajFileGets(file,&buffer))
    {
	ptr = ajStrStr(buffer);
	if(*ptr != '#' && *ptr != '\n')
	{				
	    if(first)
	    {
		cols = ajStrTokenCount(&buffer,ajStrStr(delim));
		AJCNEW0(orderstring, cols);
		for(i=0; i<cols; i++)   
		    orderstring[i] = ajStrNew();
		
		tok = ajStrTokC(buffer, " :\t\n");
		ajStrAssS(&orderstring[l++], tok);
		while((tok = ajStrTokC(NULL, " :\t\n")))
		    ajStrAssS(&orderstring[l++], tok);

		first = ajFalse;

		thys = *pthis = ajMatrixNew(orderstring, cols, filename);
		matrix = thys->Matrix;
	    }
	    else
	    {
		ajFmtScanC(ptr, "%S", &firststring);
		
		k = ajSeqCvtK(thys->Cvt, ajStrChar(firststring,0));
		/* 
		 ** cols+1 is used below because 2nd and subsequent lines have 
		 ** one more string in them (the residue label) 
		 */
		templine = ajArrIntLine(&buffer,ajStrStr(delim),cols+1,2,
					cols+1);
		
		for(i=0; i<cols; i++)   
		{
		    if(templine[i] < minval) 
			minval = templine[i];
		    matrix[k][ajSeqCvtK(thys->Cvt,
					ajStrChar(orderstring[i],0))] 
					    = templine[i];
		}
		AJFREE(templine);
	    }
	}
    }
    ajDebug("fill rest with minimum value %d\n", minval);
    

    ajFileClose(&file);
    ajStrDel(&buffer);
    ajStrDel(&delim);

    for(i=0; i<cols; i++)   
	ajStrDel(&orderstring[i]);
    AJFREE(orderstring);
        
    
    ajDebug("read matrix file %S\n", filename);
    
    ajStrDel(&firststring);    

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

AjBool ajMatrixfRead(AjPMatrixf* pthis, AjPStr filename)
{
    AjPStr *orderstring = NULL;
    AjPStr buffer       = NULL;
    AjPStr delim        = NULL;
    AjPStr firststring  = NULL;
    AjPStr reststring   = NULL;
    AjPStr tok          = NULL;

    ajint len  = 0;
    ajint i    = 0;
    ajint l    = 0;
    ajint k    = 0;
    ajint cols = 0;

    char *ptr = NULL;

    AjPMatrixf thys = NULL;
    AjPFile file    = NULL;
    AjBool  first   = ajTrue;

    float **matrix  = NULL;
    float *templine = NULL;
    float minval    = -1.0;

    
    firststring = ajStrNew();
    reststring  = ajStrNew();

    delim = ajStrNewC(" :\t\n");
    
    ajFileDataNew(filename,&file);
    
    if(!file)
    {
	ajStrDel(&firststring);
	ajStrDel(&reststring);
	return ajFalse;
    }
    
    while(ajFileGets(file,&buffer))
    {
	ptr = ajStrStr(buffer);
	if(*ptr != '#' && *ptr != '\n')
	{				
	    if(first)
	    {
		cols = ajStrTokenCount(&buffer,ajStrStr(delim));
		AJCNEW0(orderstring, cols);
		for(i=0; i<cols; i++)   
		    orderstring[i] = ajStrNew();

		tok = ajStrTokC(buffer, " :\t\n");
		ajStrAssS(&orderstring[l++], tok);
		while((tok = ajStrTokC(NULL, " :\t\n")))
		    ajStrAssS(&orderstring[l++], tok);

		first = ajFalse;

		thys = *pthis = ajMatrixfNew(orderstring, cols, filename);
		matrix = thys->Matrixf;
	    }
	    else
	    {
		ajFmtScanC(ptr, "%S", &firststring);
		k = ajSeqCvtK(thys->Cvt, ajStrChar(firststring,0));
		len = MAJSTRLEN(firststring);
		ajStrAssSubC(&reststring, ptr, len, -1);

		/* 
		** Must discard the first string (label) and use 
		** reststring otherwise ajArrFloatLine would fail (it 
		** cannot convert a string to a float)
		**   
		** Use cols,1,cols in below because although 2nd and 
		** subsequent lines have one more string in them (the
		** residue label in the 1st column) we've discard that
		** from the string that's passsed
		*/
		templine
		    =ajArrFloatLine(&reststring,ajStrStr(delim),cols,1,cols);
		
		for(i=0; i<cols; i++)  
		{
		    if(templine[i] < minval) 
			minval = templine[i];
		    matrix[k][ajSeqCvtK(thys->Cvt,
					ajStrChar(orderstring[i],0))] 
					    = templine[i];
		}
		AJFREE(templine);
	    }
	}
    }
    ajDebug("fill rest with minimum value %d\n", minval);
    

    ajFileClose(&file);
    ajStrDel(&buffer);
    ajStrDel(&delim);


    for(i=0; i<cols; i++)   
	ajStrDel(&orderstring[i]);
    AJFREE(orderstring);


    ajDebug("read matrix file %S\n", filename);
    
    ajStrDel(&firststring);
    ajStrDel(&reststring);

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

AjBool ajMatrixSeqNum(AjPMatrix thys, AjPSeq seq, AjPStr* numseq)
{
    return ajSeqNum(seq, thys->Cvt, numseq);
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

AjBool ajMatrixfSeqNum(AjPMatrixf thys, AjPSeq seq, AjPStr* numseq)
{
    return ajSeqNum(seq, thys->Cvt, numseq);
}

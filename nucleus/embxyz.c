/* @source embxyz.c
**
** Algorithms for protein structure
** Copyright (c) 2001 Jon Ison
** Copyright (c) 2001 Ranjeeva Ranasinghe
** Copyright (c) 2001 Alan Bleasby
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"


/* @func embXyzSeqsetNR ******************************************************
**
** Reads a list of AjPSeq's and writes an array describing the redundancy in 
** the list. Elements in the array correspond to sequences in the list, i.e. 
** the array[0] corresponds to the first sequence in the list.
** JCI - This should probably be an emb function
**
** @param [r] input  [AjPList]    List of ajPSeq's 
** @param [w] keep   [AjPInt*]    0: rejected (redundant) sequence, 1: the 
                                  sequence was retained
** @param [w] nset   [ajint*]     Number of sequences in nr set (no. of 1's in
**                                the keep array)
** @param [r] matrix    [AjPMatrixf] Residue substitution matrix
** @param [r] gapopen   [float]      Gap insertion penalty
** @param [r] gapextend [float]      Gap extension penalty
** @param [r] thresh    [float]      Threshold residue id. for "redundancy"
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/
AjBool embXyzSeqsetNR(AjPList input, AjPInt *keep, ajint *nset,
		      AjPMatrixf matrix, float gapopen, float gapextend,
		      float thresh)
{
    ajint         start1  =0;	/*Start of seq 1, passed as arg but not used*/
    ajint         start2  =0;	/*Start of seq 2, passed as arg but not used*/
    ajint         maxarr  =300;	/*Initial size for matrix*/
    ajint         len;
    ajint         x;		/*Counter for seq 1*/
    ajint         y;		/*Counter for seq 2*/ 
    ajint         nin;		/*Number of sequences in input list*/
    ajint        *compass;

    char       *p;
    char       *q;

    float     **sub;
    float       id       =0.;	/*Passed as arg but not used here*/
    float       sim      =0.;	
    float       idx      =0.;	/*Passed as arg but not used here*/
    float       simx     =0.;	/*Passed as arg but not used here*/
    float      *path;

    AjPStr      m = NULL;	/*Passed as arg but not used here*/
    AjPStr      n = NULL;	/*Passed as arg but not used here*/

    AjPSeq      *inseqs  =NULL;	/*Array containing input sequences*/
    AjPInt      lens     =NULL;	/*1: Lengths of sequences* in input list*/
    AjPFloat2d  scores   =NULL;
    AjPSeqCvt   cvt      =0;
    AjBool      show     =ajFalse;	/*Passed as arg but not used here*/





    /*Intitialise some variables*/
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    m = ajStrNew();    
    n = ajStrNew();    
    gapopen   = ajRoundF(gapopen,8);
    gapextend = ajRoundF(gapextend,8);
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);


    /*Convert the AjPList to an array of AjPseq*/
    if(!(nin=ajListToArray(input,(void ***)&inseqs)))
    {
	ajWarn("Zero sized list of sequences passed into SeqsetNR");
	AJFREE(compass);
	AJFREE(path);
	ajStrDel(&m);
	ajStrDel(&n);
	return ajFalse;
    }

    
    /*Create an ajint array to hold lengths of sequences*/
    lens = ajIntNewL(nin);     
    for(x=0; x<nin; x++)
	ajIntPut(&lens,x,ajSeqLen(inseqs[x]));


    /*Set the keep array elements to 1 */
    for(x=0;x<nin;x++)
	ajIntPut(keep,x,1);
       

    /*Create a 2d float array to hold the similarity scores*/
    scores = ajFloat2dNew();   






    /*Start of main application loop*/
    for(x=0; x<nin; x++)       
    { 
	for(y=x+1; y<nin; y++) 
	{
	    /*Process w/o alignment identical sequences*/	
	    if(ajStrMatch(inseqs[x]->Seq, inseqs[y]->Seq))
	    {
		ajFloat2dPut(&scores,x,y,(float)100.0);
		continue;
	    } 
	    

	    /* Intitialise variables for use by alignment functions*/	    
	    len = ajIntGet(lens,x)*ajIntGet(lens,y);

	    if(len>maxarr)
	    {
		AJCRESIZE(path,len);
		AJCRESIZE(compass,len);
		maxarr=len;
	    }
	    
	    p = ajSeqChar(inseqs[x]); 
	    q = ajSeqChar(inseqs[y]); 

	    ajStrAssC(&m,"");
	    ajStrAssC(&n,"");


	    /* Check that no sequence length is 0 */
	    if((ajIntGet(lens,x)==0)||(ajIntGet(lens,y)==0))
	    {
		ajWarn("Zero length sequence in SeqsetNR");
		AJFREE(compass);
		AJFREE(path);
		ajStrDel(&m);
		ajStrDel(&n);
		ajFloat2dDel(&scores);
		ajIntDel(&lens);
		AJFREE(inseqs);
		
		return ajFalse;
	    }


	    /* Call alignment functions */
	    embAlignPathCalc(p,q,ajIntGet(lens,x),ajIntGet(lens,y), gapopen,
			     gapextend,path,sub,cvt,compass,show);

	    embAlignScoreNWMatrix(path,inseqs[x],inseqs[y],sub,cvt,
				  ajIntGet(lens,x), ajIntGet(lens,y),gapopen,
				  compass,gapextend,&start1,&start2);

	    embAlignWalkNWMatrix(path,inseqs[x],inseqs[y],&m,&n,
				 ajIntGet(lens,x),ajIntGet(lens,y),
				 &start1,&start2,gapopen,gapextend,cvt,
				 compass,sub);

	    embAlignCalcSimilarity(m,n,sub,cvt,ajIntGet(lens,x),
				   ajIntGet(lens,y),&id,&sim,&idx, &simx);


	    /* Write array with score*/
	    ajFloat2dPut(&scores,x,y,sim);
	}
    }
    /* End of main application loop*/





    
    /* Write the keep array as appropriate*/
    for(x=0; x<nin; x++)
    {
	if(!ajIntGet(*keep,x))
	    continue;
	else
	    (*nset)++;

	for(y=x+1; y<nin; y++)	
	{
	    if(!ajIntGet(*keep,y))
		continue;

	    if(ajFloat2dGet(scores,x,y) >= thresh)
	    {
		if(ajIntGet(lens,x) < ajIntGet(lens,y))
		    ajIntPut(keep,x,0);
		
		else
		    ajIntPut(keep,y,0);  
	    }
	}
    }


    /* Tidy up */
    AJFREE(compass);
    AJFREE(path);
    ajStrDel(&m);
    ajStrDel(&n);
    ajFloat2dDel(&scores);
    ajIntDel(&lens);
    AJFREE(inseqs);
    
    
    /* Bye Bye */
    return ajTrue;
}    

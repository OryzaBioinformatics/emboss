/****************************************************************************
**
** @source embsig.c
**
** Data structures and algorithms for use with sparse sequence signatures.
** For use with the the Hit, Hitlist, Sigpos and Signature objects. 
** 
** Copyright (c) 2004 Jon Ison
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
****************************************************************************/

#include "emboss.h"
#include <math.h>




/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */

/* @datastatic EmbPHitidx *****************************************************
**
** Nucleus Hitidx object.
**
** Holds data for an indexing Hit and Hitlist objects
**
** EmbPHitidx is implemented as a pointer to a C data structure.
**
** @alias EmbSHitidx
** @alias EmbOHitidx
**
** @attr Id [AjPStr] Identifier
** @attr hptr [AjPHit] Pointer to AjPHit structure
** @attr lptr [AjPHitlist]Pointer to AjPHitlist structure 
** @@
****************************************************************************/
typedef struct EmbSHitidx
{  
    AjPStr      Id;
    AjPHit      hptr;
    AjPHitlist  lptr;
}EmbOHitidx;
#define EmbPHitidx EmbOHitidx*


/* @datastatic EmbPSigcell ****************************************************
**
** Nucleus Sigcell object.
**
** Holds data for a cell of a path matrix for a signature:sequence alignment.
**
** EmbPSigcell is implemented as a pointer to a C data structure.
**
** @alias EmbSSigcell
** @alias EmbOSigcell
**
** @attr val [float] Value for this cell
** @attr prev [ajint] Index in path matrix of prev. highest value
** @attr visited [AjBool] ajTrue if this cell has been visited
** @@
****************************************************************************/
typedef struct EmbSSigcell
{
    float  val;
    ajint  prev;
    AjBool visited;
} EmbOSigcell;
#define EmbPSigcell EmbOSigcell*





/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */

AjPSigdat    embSigdatNew(ajint nres, ajint ngap);
AjPSigpos    embSigposNew(ajint ngap);

void         embSigdatDel(AjPSigdat *pthis);
void         embSigposDel(AjPSigpos *thys);

EmbPHitidx   embHitidxNew(void);
void         embHitidxDel(EmbPHitidx *pthis);
ajint        embHitidxBinSearch(const AjPStr id,
				EmbPHitidx const *arr, ajint siz);
ajint        embHitidxMatchId(const void *hit1, const void *hit2);


AjBool       embHitlistReadFam(AjPFile scopf, const AjPStr fam,
			       const AjPStr sfam, 
			       const AjPStr fold, const AjPStr klass,
			       AjPList* list);
AjBool       embHitlistReadSfam(AjPFile scopf,
				const AjPStr fam, const AjPStr sfam, 
				const AjPStr fold, const AjPStr klass,
				AjPList* list);
AjBool       embHitlistReadFold(AjPFile scopf,
				const AjPStr fam, const AjPStr sfam, 
				const AjPStr fold, const AjPStr klass,
				AjPList* list);





/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */
/* @func embSigdatNew *******************************************************
**
** Sigdat object constructor. This is normally called by the 
** embSignatureReadNew function. Fore-knowledge of the number of empirical 
** residues and gaps is required.
** Important: Functions which manipulate the Sigdat object rely on data in 
** the gap arrays (gsiz and grfq) being filled in order of increasing gap 
** size.
**
** @param [r] nres [ajint]   Number of emprical residues.
** @param [r] ngap [ajint]   Number of emprical gaps.
** 
** @return [AjPSigdat] Pointer to a Sigdat object
** @@
****************************************************************************/

AjPSigdat embSigdatNew(ajint nres, ajint ngap)
{
    AjPSigdat ret = NULL;


    AJNEW0(ret);
    ret->nres = nres;
    ret->ngap = ngap;

    if(ngap)
    {
	ret->gsiz = ajIntNewL((ajint) ngap);
	ret->gfrq = ajIntNewL((ajint) ngap);
	ajIntPut(&ret->gsiz, ngap-1, (ajint)0);
	ajIntPut(&ret->gfrq, ngap-1, (ajint)0);
    }
    else
    {
	ret->gsiz = ajIntNew();
	ret->gfrq = ajIntNew();
	ajIntPut(&ret->gsiz, 0, (ajint)0);
	ajIntPut(&ret->gfrq, 0, (ajint)0);
    }

    if(nres)
    {
	ret->rids = ajChararrNewL((ajint) nres);
	ret->rfrq = ajIntNewL((ajint) nres);
        ajIntPut(&ret->rfrq, nres-1, (ajint)0);
	ajChararrPut(&ret->rids, nres-1, (char)' ');
    }
    else
    {
	ret->rids = ajChararrNew();
	ret->rfrq = ajIntNew();
	ajIntPut(&ret->rfrq, 0, (ajint)0);
	ajChararrPut(&ret->rids, 0, (char)' ');
    }
    
    return ret;
}




/* @func embSigposNew *******************************************************
**
** Sigpos object constructor. This is normally called by the
** embSignatureCompile function. Fore-knowledge of the number of permissible
** gaps is required.
**
** @param [r] ngap [ajint]   Number of permissible gaps.
** 
** @return [AjPSigpos] Pointer to a Sigpos object
** @@
****************************************************************************/

AjPSigpos embSigposNew(ajint ngap)
{
    AjPSigpos ret = NULL;
    
    AJNEW0(ret);
    ret->ngaps = ngap;
    
    /* Create arrays */
    AJCNEW0(ret->gsiz, ngap);
    AJCNEW0(ret->gpen, ngap);
    AJCNEW0(ret->subs, 26);
        
    return ret;
}




/* @func embSigposDel *******************************************************
**
** Destructor for Sigpos object.
**
** @param [w] pthis [AjPSigpos*] Sigpos object pointer
**
** @return [void]
** @@
****************************************************************************/

void embSigposDel(AjPSigpos *pthis)
{
    AJFREE((*pthis)->gsiz);
    AJFREE((*pthis)->gpen);
    AJFREE((*pthis)->subs);

    AJFREE(*pthis); 
    *pthis = NULL;
    
    return; 
}





/* @func embSigdatDel *******************************************************
**
** Destructor for Sigdat object.
**
** @param [w] pthis [AjPSigdat*] Sigdat object pointer
**
** @return [void]
** @@
****************************************************************************/

void embSigdatDel(AjPSigdat *pthis)
{

    ajIntDel(&(*pthis)->gsiz);
    ajIntDel(&(*pthis)->gfrq);
    ajIntDel(&(*pthis)->rfrq);
    ajChararrDel(&(*pthis)->rids);
    
    AJFREE(*pthis);    
    *pthis = NULL;

    return; 
}




/* @func embHitidxNew *******************************************************
**
** Hitidx object constructor. This is normally called by the 
** embHitlistClassify function.
**
** @return [EmbPHitidx] Pointer to a Hitidx object
** @@
****************************************************************************/

EmbPHitidx embHitidxNew(void)
{
    EmbPHitidx ret  =NULL;

    AJNEW0(ret);

    ret->Id         = ajStrNew();
    ret->hptr       = NULL;
    ret->lptr       = NULL;
    
    return ret;
}




/* @func embHitidxDel *******************************************************
**
** Destructor for Hitidx object.
**
** @param [w] pthis [EmbPHitidx*] Hitidx object pointer
**
** @return [void]
** @@
****************************************************************************/

void embHitidxDel(EmbPHitidx *pthis)
{
    ajStrDel(&(*pthis)->Id);

    AJFREE(*pthis);
    *pthis = NULL;
    
    return;
}




/* @func embHitidxBinSearch *************************************************
**
** Performs a binary search for an accession number over an array of Hitidx
** structures (which of course must first have been sorted). This is a 
** case-insensitive search.
**
** @param [r] id  [const AjPStr]       Search term
** @param [r] arr [EmbPHitidx const *]  Array of EmbOHitidx objects
** @param [r] siz [ajint]        Size of array
**
** @return [ajint] Index of first Hitidx object found with an Id element 
** matching id, or -1 if id is not found.
** @@
****************************************************************************/

ajint embHitidxBinSearch(const AjPStr id, EmbPHitidx const *arr, ajint siz)
{
    int l;
    int m;
    int h;
    int c;


    l = 0;
    h = siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;

        if((c=ajStrCmpCase(id, arr[m]->Id)) < 0) 
	    h = m-1;
        else if(c>0) 
	    l = m+1;
        else 
	    return m;
    }

    return -1;
}




/* @func embHitidxMatchId ***************************************************
**
** Function to sort Hitidx objects by Id element. Usually called by 
** embHitlistClassify.
**
** @param [r] hit1  [const void*] Pointer to Hitidx object 1
** @param [r] hit2  [const void*] Pointer to Hitidx object 2
**
** @return [ajint] -1 if Id1 should sort before Id2, +1 if the Id2 should sort 
** first. 0 if they are identical in length and content. 
** @@
****************************************************************************/

ajint embHitidxMatchId(const void *hit1, const void *hit2)
{
    EmbPHitidx p = NULL;
    EmbPHitidx q = NULL;

    p = (*(EmbPHitidx*)hit1);
    q = (*(EmbPHitidx*)hit2);
    
    return ajStrCmpO(p->Id, q->Id);

}




/* @func embHitlistReadFam **************************************************
**
** Reads a scop families file (see documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified family, 
** and create a list of Hitlist structures.  Only the first familiy in the 
** scop families file matching the specified classification is read (the file
** should not normally contain duplicate families).
**
** @param [u] scopf     [AjPFile]    The scop families file.
** @param [r] fam       [const AjPStr]     Family
** @param [r] sfam      [const AjPStr]     Superfamily
** @param [r] fold      [const AjPStr]     Fold
** @param [r] klass     [const AjPStr]     Class
** @param [w] list      [AjPList*]   A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

AjBool embHitlistReadFam(AjPFile scopf,
			 const AjPStr fam, const AjPStr sfam,
			 const AjPStr fold, 
			 const AjPStr klass, AjPList* list)
{
    AjPHitlist hitlist = NULL; 
    AjBool done        = ajFalse;
    const AjPStr class       = NULL;

    class = klass;

    /*
    ** if family is specified then the other fields also have to be
    ** specified. check that the other fields are populated
    */
    if(!fam || !sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadFam\n");
	return ajFalse;
    }
    

    while((hitlist=embHitlistRead(scopf)))
    {
	if(ajStrMatch(fam,hitlist->Family) &&
	   ajStrMatch(sfam,hitlist->Superfamily) &&
	   ajStrMatch(fold,hitlist->Fold) &&
	   ajStrMatch(class,hitlist->Class))
	{ 
	    ajListPushApp(*list,hitlist);
	    done=ajTrue;
	    break;
	}
	else
	    embHitlistDel(&hitlist);
    }
    
    if(done)
	return ajTrue;

    return ajFalse;
}





/* @func embHitlistReadSfam *************************************************
**
** Reads a scop families file (see documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified 
** superfamily, and create a list of Hitlist structures.
**
** @param [u] scopf     [AjPFile]      The scop families file.
** @param [r] fam       [const AjPStr]       Family
** @param [r] sfam      [const AjPStr]       Superfamily
** @param [r] fold      [const AjPStr]       Fold
** @param [r] klass     [const AjPStr]       Class
** @param [w] list      [AjPList*]     A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

AjBool embHitlistReadSfam(AjPFile scopf,
			  const AjPStr fam, const AjPStr sfam,
			  const AjPStr fold, const AjPStr klass, AjPList* list)
{
    AjPHitlist hitlist = NULL; 
    AjBool done  = ajFalse;
    const AjPStr class = NULL;

    class = klass;
    
    /*
    ** if family is specified then the other fields also have to be
    ** specified.  check that the other fields are populated
    */
    if(!sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadSfam\n");
	return ajFalse;
    }
    
    
    while((hitlist=embHitlistRead(scopf)))
	if(ajStrMatch(fam,hitlist->Superfamily) &&
	   ajStrMatch(fold,hitlist->Fold) &&
	   ajStrMatch(class,hitlist->Class))
	{
	    ajListPushApp(*list,hitlist);
	    done=ajTrue;
	}
	else
	    embHitlistDel(&hitlist);

    if(done)
	return ajTrue;

    return ajFalse;
}





/* @func embHitlistReadFold *************************************************
**
** Reads a scop families file (see documentation for the EMBASSY 
** DOMAINATRIX package), selects the entries with the specified 
** fold, and create a list of Hitlist structures.
**
** @param [u] scopf     [AjPFile]      The scop families file.
** @param [r] fam       [const AjPStr]       Family
** @param [r] sfam      [const AjPStr]       Superfamily
** @param [r] fold      [const AjPStr]       Fold
** @param [r] klass     [const AjPStr]       Class
** @param [w] list      [AjPList*]     A list of hitlist structures.
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

AjBool embHitlistReadFold(AjPFile scopf,
			  const AjPStr fam, const AjPStr sfam,
			  const AjPStr fold, const AjPStr klass, AjPList* list)
{
    AjPHitlist hitlist = NULL; 
    AjBool done  = ajFalse;
    const AjPStr class = NULL;

    class = klass;
    
    /*
    ** if family is specified then the other fields also have to be
    ** specified.  check that the other fields are populated
    */ 
    if(!fold || !class)
    {
	ajWarn("Bad arguments passed to embHitlistReadFold\n");
	return ajFalse;
    }
    
    while((hitlist = embHitlistRead(scopf)))
	if(ajStrMatch(fam,hitlist->Fold) &&
	   ajStrMatch(class,hitlist->Class))
	{
	    ajListPushApp(*list,hitlist);
	    done=ajTrue;
	}
	else
	    embHitlistDel(&hitlist);

    if(done)
	return ajTrue;

    return ajFalse;	
}








/* ======================================================================= */
/* =========================== constructors ============================== */
/* ======================================================================= */

/* @section Constructors ****************************************************
**
** All constructors return a pointer to a new instance. It is the 
** responsibility of the user to first destroy any previous instance. The 
** target pointer does not need to be initialised to NULL, but it is good 
** programming practice to do so anyway.
**
****************************************************************************/

/* @func embHitlistNew ******************************************************
**
** Hitlist object constructor. This is normally called by the embHitlistRead
** function. Fore-knowledge of the number of hits is required.
**
** @param [r] n [ajint] Number of hits
** 
** @return [AjPHitlist] Pointer to a hitlist object
** @@
****************************************************************************/

AjPHitlist embHitlistNew(ajint n)
{
    AjPHitlist ret = NULL;
    ajint i = 0;
    

    AJNEW0(ret);
    ret->Class       = ajStrNew();
    ret->Fold        = ajStrNew();
    ret->Superfamily = ajStrNew();
    ret->Family      = ajStrNew();
    ret->Model       = ajStrNew();
    ret->Priority    = ajFalse;
    
    ret->N=n;

    if(n)
    {
	AJCNEW0(ret->hits,n);
	for(i=0;i<n;++i)
	    ret->hits[i] = embHitNew();
    }	

    return ret;
}





/* @func embHitNew **********************************************************
**
** Hit object constructor. This is normally called by the embHitlistNew
** function.
**
** @return [AjPHit] Pointer to a hit object
** @@
****************************************************************************/

AjPHit embHitNew(void)
{
    AjPHit ret = NULL;

    AJNEW0(ret);

    ret->Seq       = ajStrNew();
    ret->Acc       = ajStrNew();
    ret->Spr       = ajStrNew();
    ret->Typeobj   = ajStrNew();
    ret->Typesbj   = ajStrNew();
    ret->Model     = ajStrNew();
    ret->Alg       = ajStrNew();
    ret->Group     = ajStrNew();
    ret->Start     = 0;
    ret->End       = 0;
    ret->Rank      = 0;
    ret->Score     = 0;    
    ret->Eval      = 0;
    ret->Pval      = 0;
    ret->Target    = ajFalse;
    ret->Target2   = ajFalse;
    ret->Priority  = ajFalse;

    return ret;
}







/* @func embSignatureNew ****************************************************
**
** Signature object constructor. This is normally called by the
** embSignatureReadNew function. Fore-knowledge of the number of signature 
** positions is required.
**
** @param [r] n [ajint]   Number of signature positions
** 
** @return [AjPSignature] Pointer to a Signature object
** @@
****************************************************************************/

AjPSignature embSignatureNew(ajint n)
{
    AjPSignature ret = NULL;


    AJNEW0(ret);
    ret->Class       = ajStrNew();
    ret->Fold        = ajStrNew();
    ret->Superfamily = ajStrNew();
    ret->Family      = ajStrNew();
    ret->npos = n;

    /* Create arrays of pointers to Sigdat & Sigpos structures */
    if(n)
    {
	ret->dat = AJCALLOC0(n, sizeof(AjPSigdat));
	ret->pos = AJCALLOC0(n, sizeof(AjPSigpos));
    }
    
    return ret;
}





/* ======================================================================= */
/* =========================== destructors =============================== */
/* ======================================================================= */

/* @section Structure Destructors *******************************************
**
** All destructor functions receive the address of the instance to be
** deleted.  The original pointer is set to NULL so is ready for re-use.
**
****************************************************************************/

/* @func embHitlistDel ******************************************************
**
** Destructor for hitlist object.
**
** @param [w] ptr [AjPHitlist*] Hitlist object pointer
**
** @return [void]
** @@
****************************************************************************/

void embHitlistDel(AjPHitlist *ptr)
{
    int x = 0;  /* Counter */

    if(!(*ptr))
    {
	ajWarn("Null pointer passed to embHitlistDel");
	return;
    }
    
    if((*ptr)->Class)
	ajStrDel(&(*ptr)->Class);
    if((*ptr)->Fold)
	ajStrDel(&(*ptr)->Fold);
    if((*ptr)->Superfamily)
	ajStrDel(&(*ptr)->Superfamily);
    if((*ptr)->Family)
	ajStrDel(&(*ptr)->Family);
    if((*ptr)->Model)
	ajStrDel(&(*ptr)->Model);
    
    for(x=0;x<(*ptr)->N; x++)
	if((*ptr)->hits[x])
	    embHitDel(&(*ptr)->hits[x]);

    if((*ptr)->hits)
	AJFREE((*ptr)->hits);
    
    if(*ptr)
	AJFREE(*ptr);
    
    *ptr = NULL;
    
    return;
}





/* @func embHitDel **********************************************************
**
** Destructor for hit object.
**
** @param [w] ptr [AjPHit*] Hit object pointer
**
** @return [void]
** @@
****************************************************************************/

void embHitDel(AjPHit *ptr)
{
    ajStrDel(&(*ptr)->Seq);
    ajStrDel(&(*ptr)->Acc);
    ajStrDel(&(*ptr)->Spr);
    ajStrDel(&(*ptr)->Typeobj);
    ajStrDel(&(*ptr)->Typesbj);
    ajStrDel(&(*ptr)->Model);
    ajStrDel(&(*ptr)->Alg);
    ajStrDel(&(*ptr)->Group);

    AJFREE(*ptr);
    *ptr = NULL;
    
    return;
}





/* @func embSignatureDel ****************************************************
**
** Destructor for Signature object.
**
** @param [w] ptr [AjPSignature*] Signature object pointer
**
** @return [void]
** @@
****************************************************************************/

void embSignatureDel(AjPSignature *ptr)
{
    ajint x = 0;
    
    if(!(*ptr))
	return;
    
    if((*ptr)->dat)
	for(x=0;x<(*ptr)->npos; ++x)
	    if((*ptr)->dat[x])
		embSigdatDel(&(*ptr)->dat[x]);

    if((*ptr)->pos)
	for(x=0;x<(*ptr)->npos; ++x)
	    if((*ptr)->pos[x])
		embSigposDel(&(*ptr)->pos[x]);

    if((*ptr)->Class)
	ajStrDel(&(*ptr)->Class);
    if((*ptr)->Fold)
	ajStrDel(&(*ptr)->Fold);
    if((*ptr)->Superfamily)
	ajStrDel(&(*ptr)->Superfamily);
    if((*ptr)->Family)
	ajStrDel(&(*ptr)->Family);

    if((*ptr)->dat)
	AJFREE((*ptr)->dat);

    if((*ptr)->pos)
	AJFREE((*ptr)->pos);

    AJFREE(*ptr);    
    *ptr = NULL;

    return;
}





/* ======================================================================= */
/* ============================ Assignments ============================== */
/* ======================================================================= */

/* @section Assignments *****************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
****************************************************************************/


/* @func embHitMerge ********************************************************
**
** Creates a new Hit object which corresponds to a merging of the two 
** sequences from the Hit objects hit1 and hit2. 
**
** The Typeobj of the merged hit is set.  The merged hit is classified 
** as follows :
** If hit1 or hit2 is a SEED, the merged hit is classified as a SEED.
** Otherwise, if hit1 or hit2 is HIT, the merged hit is clsasified as a HIT.
** If hit1 and hit2 are both OTHER, the merged hit remains classified as 
** OTHER.
** 
** @param [r] hit1     [const AjPHit]  Hit 1
** @param [r] hit2     [const AjPHit]  Hit 2
**
** @return [AjPHit] Pointer to Hit object.
** @@
****************************************************************************/

AjPHit embHitMerge(const AjPHit hit1, const AjPHit hit2)
{
    AjPHit ret;
    ajint start = 0;      /* Start of N-terminal-most sequence */
    ajint end   = 0;      /* End of N-terminal-most sequence */
    AjPStr temp = NULL;
    
    /* Check args */
    if(!hit1 || !hit2)
    {
	ajWarn("Bad arg's passed to AjPHitMerge");
	return NULL;
    }

    if(!ajStrMatch(hit1->Acc, hit2->Acc))
    {
	ajWarn("Merge attempted on 2 hits with different accession numbers");
	return NULL;
    }

    /* Allocate memory */
    ret  = embHitNew();
    temp = ajStrNew();
    

    ajStrAssS(&(ret->Acc), hit1->Acc);
    ajStrAssS(&(ret->Spr), hit1->Spr);
        


    /*
    ** Copy the N-terminal most sequence to our new sequence 
    ** and assign start point of new hit
    */
    if(hit1->Start <= hit2->Start)
    {
	ajStrAssS(&(ret->Seq), hit1->Seq);
	ret->Start = hit1->Start;
	end   = hit1->End;
	start = hit2->Start;
    }	
    else
    {
	ajStrAssS(&(ret->Seq), hit2->Seq);
    	ret->Start = hit2->Start;
	end   = hit2->End;
	start = hit1->Start;
    }
    

    /* Assign end point of new hit */
    if(hit1->End >= hit2->End)
	ret->End = hit1->End;
    else
    	ret->End = hit2->End;


    /* Assign the C-terminus of the sequence of the new hit     
       if necessary */
    if(hit2->End > end)
    {
	ajStrAssSub(&temp, hit2->Seq, end-start+1, -1);
	ajStrApp(&(ret->Seq),temp);
    }
    else if(hit1->End > end)
    {
	ajStrAssSub(&temp, hit1->Seq, end-start+1, -1);
	ajStrApp(&(ret->Seq),temp);
    }


    /* Classify the merged hit */
    if(ajStrMatchC(hit1->Typeobj, "SEED") ||
       ajStrMatchC(hit1->Typeobj, "SEED"))
	ajStrAssC(&(ret->Typeobj), "SEED");
    else if(ajStrMatchC(hit1->Typeobj, "HIT") ||
	    ajStrMatchC(hit1->Typeobj, "HIT"))
	ajStrAssC(&(ret->Typeobj), "HIT");
    else
	ajStrAssC(&(ret->Typeobj), "OTHER");


    if(ajStrMatch(hit1->Model, hit2->Model))
	ajStrAssS(&ret->Model, hit1->Model);
    

    
    /* All other elements of structure are left as NULL / o / ajFalse */
        
    ajStrDel(&temp);

    return ret;
}





/* ======================================================================= */
/* ============================= Modifiers =============================== */
/* ======================================================================= */

/* @section Modifiers *******************************************************
**
** These functions use the contents of an instance and update them.
**
****************************************************************************/





/* ======================================================================= */
/* ========================== Operators ===================================*/
/* ======================================================================= */

/* @section Operators *******************************************************
**
** These functions use the contents of an instance but do not make any 
** changes.
**
****************************************************************************/


/* @func embHitlistMatchFold ************************************************
**
** Function to sort Hitlist object by Fold element. 
**
** @param [r] hit1  [const void*] Pointer to Hitlist object 1
** @param [r] hit2  [const void*] Pointer to Hitlist object 2
**
** @return [ajint] -1 if Fold1 should sort before Fold2, +1 if the Fold2 
** should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint embHitlistMatchFold(const void *hit1, const void *hit2)
{
    AjPHitlist p = NULL;
    AjPHitlist q = NULL;

    p = (*(AjPHitlist*)hit1);
    q = (*(AjPHitlist*)hit2);
    
    return ajStrCmpO(p->Fold, q->Fold);
}





/* @func embMatchScore ******************************************************
**
** Function to sort Hit objects by score record. Usually called by 
** ajListSort.
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if score1<score2, 0 if score1==score2, else -1.
** @@
****************************************************************************/

ajint embMatchScore(const void *hit1, const void *hit2)
{
    AjPHit p = NULL;
    AjPHit q = NULL;

    p = (*(AjPHit*)hit1);
    q = (*(AjPHit*)hit2);
    
    if(p->Score < q->Score)
	return -1;
    else if (p->Score == q->Score)
	return 0;

    return 1;
}



/* @func embMatchinvScore ***************************************************
**
** Function to sort Hit objects by score record. Usually called by 
** ajListSort.  The sorting order is inverted - i.e. it returns -1 if score1 
** > score2 (as opposed to embMatchScore).
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if score1<score2, 0 if score1==score2, else -1.
** @@
****************************************************************************/

ajint embMatchinvScore(const void *hit1, const void *hit2)
{
    AjPHit p = NULL;
    AjPHit q = NULL;

    p = (*(AjPHit*)hit1);
    q = (*(AjPHit*)hit2);
    
    if(p->Score > q->Score)
	return -1;
    else if (p->Score == q->Score)
	return 0;

    return 1;
}






/* ======================================================================= */
/* ============================== Casts ===================================*/
/* ======================================================================= */

/* @section Casts ***********************************************************
**
** These functions examine the contents of an instance and return some
** derived information. Some of them provide access to the internal
** components of an instance. They are provided for programming convenience
** but should be used with caution.
**
****************************************************************************/





/* ======================================================================= */
/* =========================== Reporters ==================================*/
/* ======================================================================= */

/* @section Reporters *******************************************************
**
** These functions return the contents of an instance but do not make any 
** changes.
**
****************************************************************************/

/* @func embHitsOverlap *****************************************************
**
** Checks for overlap between two hits.
**
** @param [r] hit1  [const AjPHit]     Pointer to hit object 1
** @param [r] hit2  [const AjPHit]     Pointer to hit object 2
** @param [r] n     [ajint]      Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
****************************************************************************/

AjBool embHitsOverlap(const AjPHit hit1, const AjPHit hit2, ajint n)
{
    if((MAJSTRLEN(hit1->Seq)<n) || (MAJSTRLEN(hit2->Seq)<n))
    {
	ajWarn("Sequence length smaller than overlap limit in "
	       "embHitsOverlap ... checking for string match instead");

	if((ajStrFind(hit1->Seq, hit2->Seq)!=-1) ||
	   (ajStrFind(hit2->Seq, hit1->Seq)!=-1))
	    return ajTrue;
	else
	    return ajFalse;
    }

    if( (((hit1->End - hit2->Start + 1)>=n) && 
	 (hit2->Start >= hit1->Start)) ||
       (((hit2->End - hit1->Start + 1)>=n) && 
	(hit1->Start >= hit2->Start)))
	return ajTrue;

    return ajFalse;
}




/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/

/* @func embHitlistRead *****************************************************
**
** Read a hitlist object from a file in embl-like format (see documentation
** for the DOMAINATRIX "seqsearch" application). 
** 
** @param [u] inf      [AjPFile] Input file stream
**
** @return [AjPHitlist] Hitlist object
** @@
****************************************************************************/

AjPHitlist embHitlistRead(AjPFile inf) 
{
    AjPHitlist ret = NULL;
    
    AjPStr line   = NULL;   /* Line of text */
    AjPStr class  = NULL;
    AjPStr fold   = NULL;
    AjPStr super  = NULL;
    AjPStr family = NULL;
    AjBool   ok   = ajFalse;
    ajint    n    = 0;              /* Number of current sequence */
    ajint    nset = 0;              /* Number in set */
    ajint  Sunid_Family = 0;        /* SCOP sunid for family */



    /* Allocate strings */
    class   = ajStrNew();
    fold    = ajStrNew();
    super   = ajStrNew();
    family  = ajStrNew();
    line    = ajStrNew();
    

    
    /* Read first line */
    ok = ajFileReadLine(inf,&line);

    /* '//' is the delimiter for block of hits (in case the file 
       contains multiple hitlists). */

    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %d", &Sunid_Family);
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssC(&class,ajStrStr(line)+3);
	    ajStrClean(&class);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&fold,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssC(&super,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&super,ajStrStr(line)+3);
	    }
	    ajStrClean(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssC(&family,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"NS"))
	{
	    ajFmtScanS(line, "NS %d", &nset);


	    /* Create hitlist structure */
	    (ret)=embHitlistNew(nset);
	    (ret)->N=nset;
	    ajStrAssS(&(ret)->Class, class);
	    ajStrAssS(&(ret)->Fold, fold);
	    ajStrAssS(&(ret)->Superfamily, super);
	    ajStrAssS(&(ret)->Family, family);
	    (ret)->Sunid_Family = Sunid_Family;
	}
	else if(ajStrPrefixC(line,"NN"))
	{
	    /* Increment hit counter */
	    n++;
	    
	    /* Safety check */
	    if(n>nset)
		ajFatal("Dangerous error in input file caught "
			"in embHitlistRead.\n Email jison@hgmp.mrc.ac.uk");
	}
	else if(ajStrPrefixC(line,"SC"))
	{
	    ajFmtScanS(line, "%*s %f", &(ret)->hits[n-1]->Score);
	}
	else if(ajStrPrefixC(line,"AC"))
	{
	    ajStrAssC(&(ret)->hits[n-1]->Acc,ajStrStr(line)+3);
	    ajStrClean(&(ret)->hits[n-1]->Acc);
	}
	else if(ajStrPrefixC(line,"SP"))
	{
	    ajStrAssC(&(ret)->hits[n-1]->Spr,ajStrStr(line)+3);
	    ajStrClean(&(ret)->hits[n-1]->Spr);
	}
	else if(ajStrPrefixC(line,"TY"))
	{
	    ajStrAssC(&(ret)->hits[n-1]->Typeobj,ajStrStr(line)+3);	
	    ajStrClean(&(ret)->hits[n-1]->Typeobj);		
	}
	else if(ajStrPrefixC(line,"MO"))
	{
	    ajStrAssC(&(ret)->hits[n-1]->Model,ajStrStr(line)+3);	
	    ajStrClean(&(ret)->hits[n-1]->Model);		
	}
	else if(ajStrPrefixC(line,"RA"))
	    ajFmtScanS(line, "%*s %d %*s %d", &(ret)->hits[n-1]->Start,
		       &(ret)->hits[n-1]->End);
	else if(ajStrPrefixC(line,"GP"))
	    ajFmtScanS(line, "%*s %S", &(ret)->hits[n-1]->Group);
	else if(ajStrPrefixC(line,"SQ"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppC(&(ret)->hits[n-1]->Seq,ajStrStr(line));
	    ajStrCleanWhite(&(ret)->hits[n-1]->Seq);
	    continue;
	}
	
	ok = ajFileReadLine(inf,&line);
    }


    ajStrDel(&line);
    ajStrDel(&class);
    ajStrDel(&fold);
    ajStrDel(&super);
    ajStrDel(&family);
    
    if(!ok)
	return NULL;

    return ret;
}





/* @func embHitlistReadNode *************************************************
**
** Reads a scop families file (see documentation for the EMBASSY 
** DOMAINATRIX package) and writes a list of Hitlist objects containing 
** all domains matching the scop classification provided.
**
** @param [u] inf    [AjPFile]   File containing multiple Hitlist objects
** @param [r] fam    [const AjPStr]    Family.
** @param [r] sfam   [const AjPStr]    Superfamily.
** @param [r] fold   [const AjPStr]    Fold.
** @param [r] klass  [const AjPStr]    Class.
** 
** @return [AjPList] List of Hitlist objects or NULL.
** @@
****************************************************************************/

AjPList embHitlistReadNode(AjPFile inf,
			   const AjPStr fam, const AjPStr sfam, 
			   const AjPStr fold, const AjPStr klass)
{
    AjPList ret = NULL;
    const AjPStr class   = NULL;

    class = klass;

    if(!inf)
	ajFatal("NULL arg passed to embHitlistReadNode");

    /* Allocate the list if it does not already exist */

    (ret) = ajListNew();

    
    /*
    ** if family is specified then the other fields
    ** also have to be specified.
    */
    if(fam)
    {
	if(!sfam || !fold || !class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNode\n");
		ajListDel(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadFam(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListDel(&(ret));
		return NULL;
	    }
	}
    }
    /*
    ** if superfamily is specified then the other fields
    ** also have to be specified.
    */
    else if(sfam)
    {
	if(!fold || !class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNode\n");
		ajListDel(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadSfam(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListDel(&(ret));
		return NULL;
	    }
	}	   
    }
    /*
    ** if fold is specified then the other fields also have
    ** to be specified.
    */
    else if(fold)
    {
	if(!class)
	{
	    ajWarn("Bad arguments passed to embHitlistReadNode\n");
		ajListDel(&(ret));
	    return NULL;
	}
	else
	{
	    if((embHitlistReadFold(inf,fam,sfam,fold,class,&ret)))
		return ret;
	    else
	    {
		    ajListDel(&(ret));
		return NULL;
	    }
	}
    } 

    ajWarn("Bad arguments passed to embHitlistReadNode\n");
	ajListDel(&(ret));

    return ret;
}





/* @func embHitlistWrite ****************************************************
**
** Write contents of a Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Text for Class, Fold, Superfamily and Family is only written if the text
** is available.
** 
** @param [u] outf [AjPFile] Output file stream
** @param [r] obj [const AjPHitlist] Hitlist object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool embHitlistWrite(AjPFile outf, const AjPHitlist obj)
{
    ajint x = 0;  /* Counter */
    
    if(!obj)
	return ajFalse;

    if(MAJSTRLEN(obj->Class))
	ajFmtPrintF(outf,"CL   %S\n",obj->Class);

    if(MAJSTRLEN(obj->Fold))
	ajFmtPrintSplit(outf,obj->Fold,"XX\nFO   ",75," \t\n\r");

    if(MAJSTRLEN(obj->Superfamily))
	ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");

    if(MAJSTRLEN(obj->Family))
	ajFmtPrintSplit(outf,obj->Family,"XX\nFA   ",75," \t\n\r");

    if(MAJSTRLEN(obj->Family) && obj->Sunid_Family)
	ajFmtPrintF(outf,"XX\nSI   %d\n", obj->Sunid_Family);
    

    ajFmtPrintF(outf,"XX\nNS   %d\nXX\n",obj->N);

    for(x=0;x<obj->N;x++)
    {
	ajFmtPrintF(outf, "%-5s[%d]\nXX\n", "NN", x+1);
	if(MAJSTRLEN(obj->hits[x]->Model))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "MO", obj->hits[x]->Model);
	    ajFmtPrintF(outf, "XX\n");
	}
	
	if(MAJSTRLEN(obj->hits[x]->Typeobj))
	    ajFmtPrintF(outf, "%-5s%S\n", "TY", obj->hits[x]->Typeobj);
	ajFmtPrintF(outf, "XX\n");
	ajFmtPrintF(outf, "%-5s%.2f\n", "SC", obj->hits[x]->Score);
	ajFmtPrintF(outf, "XX\n");
	if(MAJSTRLEN(obj->hits[x]->Group))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "GP", obj->hits[x]->Group);
	    ajFmtPrintF(outf, "XX\n");
	}

	ajFmtPrintF(outf, "%-5s%S\n", "AC", obj->hits[x]->Acc);
	ajFmtPrintF(outf, "XX\n");
	if(MAJSTRLEN(obj->hits[x]->Spr))
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "SP", obj->hits[x]->Spr);
	    ajFmtPrintF(outf, "XX\n");
	}
	
	ajFmtPrintF(outf, "%-5s%d START; %d END;\n", "RA",
		    obj->hits[x]->Start, obj->hits[x]->End);
	ajFmtPrintF(outf, "XX\n");
	ajSeqWriteXyz(outf, obj->hits[x]->Seq, "SQ");
	ajFmtPrintF(outf, "XX\n");
    }
    ajFmtPrintF(outf, "//\n");

    return ajTrue;
}





/* @func embHitlistWriteSubset **********************************************
**
** Write contents of a Hitlist object to an output file in embl-like format
** (see documentation for the DOMAINATRIX "seqsearch" application).
** Only those hits are written for which a 1 is given in the corresponding
** position in array of integers.
** Text for Class, Fold, Superfamily and Family is only written if the text
** is available.
** 
** @param [u] outf  [AjPFile]    Output file stream
** @param [r] obj   [const AjPHitlist] Hitlist object
** @param [r] ok    [const AjPInt]     Whether hits are to be printed or not
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool embHitlistWriteSubset(AjPFile outf,
			     const AjPHitlist obj, const AjPInt ok)
{
    ajint x    = 0;  /* Counter */
    ajint y    = 0;  /* Counter */
    ajint nset = 0;  /* No. in set to be printed out */
    

    if(!obj)
	return ajFalse;

    if(MAJSTRLEN(obj->Class))
	ajFmtPrintF(outf,"CL   %S\n",obj->Class);

    if(MAJSTRLEN(obj->Fold))
	ajFmtPrintSplit(outf,obj->Fold,"XX\nFO   ",75," \t\n\r");

    if(MAJSTRLEN(obj->Superfamily))
	ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");

    if(MAJSTRLEN(obj->Family))
	ajFmtPrintSplit(outf,obj->Family,"XX\nFA   ",75," \t\n\r");

    if(MAJSTRLEN(obj->Family))
	ajFmtPrintF(outf,"XX\nSI   %d\n", obj->Sunid_Family);


    for(nset=0, x=0;x<obj->N;x++)
	if(ajIntGet(ok, x) == 1)
	    nset++;
	    
    ajFmtPrintF(outf,"XX\nNS   %d\nXX\n",nset);

    for(x=0;x<obj->N;x++)
    { 
	if(ajIntGet(ok, x) == 1)
	{
	    y++;

	    ajFmtPrintF(outf, "%-5s[%d]\nXX\n", "NN", y);
	    if(MAJSTRLEN(obj->hits[x]->Model))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "MO", obj->hits[x]->Model);
		ajFmtPrintF(outf, "XX\n");
	    }
	    
	    if(MAJSTRLEN(obj->hits[x]->Typeobj))
		ajFmtPrintF(outf, "%-5s%S\n", "TY", obj->hits[x]->Typeobj);
	    ajFmtPrintF(outf, "XX\n");
	    ajFmtPrintF(outf, "%-5s%.2f\n", "SC", obj->hits[x]->Score);
	    ajFmtPrintF(outf, "XX\n");
	    if(MAJSTRLEN(obj->hits[x]->Group))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "GP", obj->hits[x]->Group);
		ajFmtPrintF(outf, "XX\n");
	    }
	    
	    ajFmtPrintF(outf, "%-5s%S\n", "AC", obj->hits[x]->Acc);
	    ajFmtPrintF(outf, "XX\n");
	    if(MAJSTRLEN(obj->hits[x]->Spr))
	    {
		ajFmtPrintF(outf, "%-5s%S\n", "SP", obj->hits[x]->Spr);
		ajFmtPrintF(outf, "XX\n");
	    }

	    ajFmtPrintF(outf, "%-5s%d START; %d END;\n", "RA",
			obj->hits[x]->Start, obj->hits[x]->End);
	    ajFmtPrintF(outf, "XX\n");
	    ajSeqWriteXyz(outf, obj->hits[x]->Seq, "SQ");
	    ajFmtPrintF(outf, "XX\n");
	}
    }
    ajFmtPrintF(outf, "//\n");
	
    return ajTrue;
}





/* @func embSignatureReadNew ************************************************
**
** Read a Signature object from a file in embl-like format (see documentation
** for the DOMAINATRIX "sigscan" application).
**
** @param [u] inf [AjPFile] Input file stream
**
** @return [AjPSignature] Signature object
** @@
****************************************************************************/

AjPSignature embSignatureReadNew(AjPFile inf)
{
    AjPSignature ret = NULL;
    
    static AjPStr line   = NULL;
    static AjPStr class  = NULL;
    static AjPStr fold   = NULL;
    static AjPStr super  = NULL;
    static AjPStr family = NULL;
    ajint  Sunid_Family;        /* SCOP sunid for family */

    AjBool ok   = ajFalse;
    ajint  npos = 0;   /* No. signature positions */
    ajint  i    = 0;   /* Loop counter */
    ajint  n    = 0;   /* Counter of signature positions */
    ajint  nres = 0;   /* No. residues for a sig. position */
    ajint  ngap = 0;   /* No. gaps for a sig. position */
    ajint  wsiz = 0;   /* Windows size for a sig. position */
    ajint  v1   = 0;
    ajint  v2   = 0;
    char   c1   = '\0';
    
    
    /* CHECK ARG'S */
    if(!inf)
	return NULL;
    

    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
	class   = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	line    = ajStrNew();
    }


    /* Read first line */
    ok=ajFileReadLine(inf,&line);

    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %d", &Sunid_Family);
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssC(&class,ajStrStr(line)+3);
	    ajStrClean(&class);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&fold,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssC(&super,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&super,ajStrStr(line)+3);
	    }
	    ajStrClean(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssC(&family,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"NP"))
	{
	    ajFmtScanS(line, "NP%d", &npos);

	    /* Create signature structure */
	    (ret)=embSignatureNew(npos);
	    ajStrAssS(&(ret)->Class, class);
	    ajStrAssS(&(ret)->Fold, fold);
	    ajStrAssS(&(ret)->Superfamily, super);
	    ajStrAssS(&(ret)->Family, family);
	    (ret)->Sunid_Family = Sunid_Family;	
	}
	else if(ajStrPrefixC(line,"NN"))
	{
	    /* Increment position counter */
	    n++;

	    /* Safety check */
	    if(n>npos)
		ajFatal("Dangerous error in input file caught in "
			"embSignatureReadNew.\n Email jison@hgmp.mrc.ac.uk");
	}
	else if(ajStrPrefixC(line,"IN"))
	    {
		ajFmtScanS(line, "%*s %*s %d %*c %*s %d %*c %*s %d", 
			   &nres, &ngap, &wsiz);
	
		/* Create Sigdat structures and fill some elements */
		(ret)->dat[n-1]=embSigdatNew(nres, ngap);

		(ret)->dat[n-1]->wsiz=wsiz;

		/* Skip 'XX' line */
		if(!(ok = ajFileReadLine(inf,&line)))
		    break;

		/* Read in residue data */
		for(i=0; i<(ret)->dat[n-1]->nres; i++)
		{
		    if(!(ok = ajFileReadLine(inf,&line)))
			break;
		    ajFmtScanS(line, "%*s %c %*c %d", &c1,&v2);
		    ajChararrPut(&(ret)->dat[n-1]->rids,i,c1);
		    ajIntPut(&(ret)->dat[n-1]->rfrq,i,v2);
		}
		if(!ok)
		    break;
	       

		/* Skip 'XX' line */
		if(!(ok = ajFileReadLine(inf,&line)))
		    break;

		/* Read in gap data */
		for(i=0; i<(ret)->dat[n-1]->ngap; i++)
		{
		    if(!(ok = ajFileReadLine(inf,&line)))
			break;
		    ajFmtScanS(line, "%*s %d %*c %d", &v1,&v2);
		    ajIntPut(&(ret)->dat[n-1]->gsiz,i,v1);
		    ajIntPut(&(ret)->dat[n-1]->gfrq,i,v2);
		}
		if(!ok)
		    break;
	    }

	ok = ajFileReadLine(inf,&line);
    }
    
    if(!ok)
	return NULL;

    return ret;
}





/* @func embSignatureWrite **************************************************
**
** Write contents of a Signature object to an output file in embl-like 
** format (see documentation for the DOMAINATRIX "sigscan" application). 
**
** @param [w] outf [AjPFile]       Output file stream
** @param [r] obj  [const AjPSignature]  Signature object
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/
AjBool embSignatureWrite(AjPFile outf, const AjPSignature obj)
{ 
    ajint i;
    ajint j;

    if(!outf || !obj)
	return ajFalse;


    ajFmtPrintF(outf,"CL   %S",obj->Class);
    ajFmtPrintSplit(outf,obj->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,obj->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,obj->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nSI   %d\n", obj->Sunid_Family);
    ajFmtPrintF(outf,"XX\nNP   %d\n",obj->npos);


    for(i=0;i<obj->npos;++i)
    {
	ajFmtPrintF(outf,"XX\nNN   [%d]\n",i+1);
	ajFmtPrintF(outf,"XX\nIN   NRES %d ; NGAP %d ; WSIZ %d\nXX\n",
		    obj->dat[i]->nres, obj->dat[i]->ngap,
		    obj->dat[i]->wsiz);

	for(j=0;j<obj->dat[i]->nres;++j)
	    ajFmtPrintF(outf,"AA   %c ; %d\n",
			(char)  ajChararrGet(obj->dat[i]->rids, j),
			(ajint) ajIntGet(obj->dat[i]->rfrq, j));
	ajFmtPrintF(outf,"XX\n");
	for(j=0;j<obj->dat[i]->ngap;++j)
	    ajFmtPrintF(outf,"GA   %d ; %d\n",
			(ajint) ajIntGet(obj->dat[i]->gsiz, j),
			(ajint) ajIntGet(obj->dat[i]->gfrq, j));
    }
    ajFmtPrintF(outf,"//\n");
    
    return ajTrue;
}





/* @func embSignatureHitsRead ***********************************************
**
** Reads a signature hits file, allocates a Hitlist object and writes it 
** with hits from a signature hits file (see documentation for the 
** DOMAINATRIX "sigscan" application). In other words, this function reads
** the results of a scan of a signature against a protein sequence database.  
**
** @param [u] inf  [AjPFile]      Input file stream
**
** @return [AjPHitlist] Hitlist object that was allocated.
** @@
****************************************************************************/

AjPHitlist embSignatureHitsRead(AjPFile inf)
{
    AjPList list        = NULL;
    AjPHitlist ret      = NULL;
    ajint  Sunid_Family = 0;
    AjBool ok           = ajTrue;
    AjPHit tmphit       = NULL;
    

    AjPStr class  = NULL;
    AjPStr fold   = NULL;
    AjPStr super  = NULL;
    AjPStr family = NULL;
    AjPStr line   = NULL;
    
    if(!inf)
    {
	ajWarn("NULL file pointer passed to embSignatureHitsRead");
	return NULL;
    }
    

    list   = ajListNew();
    class  = ajStrNew();
    fold   = ajStrNew();
    super  = ajStrNew();
    family = ajStrNew();
    line   = ajStrNew();

    
    
    while(ok && ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %d", &Sunid_Family);
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssC(&class,ajStrStr(line)+3);
	    ajStrClean(&class);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&fold,ajStrStr(line)+3);
	    while((ok=ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssC(&super,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&super,ajStrStr(line)+3);
	    }
	    ajStrClean(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssC(&family,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"HI"))
	{
	    tmphit=embHitNew();
	    ajFmtScanS(line, "%*s %*d %S %d %d %S %S %S %f %f %f", 
		       &tmphit->Acc, 
		       &tmphit->Start, 
		       &tmphit->End, 
		       &tmphit->Group, 
		       &tmphit->Typeobj, 
		       &tmphit->Typesbj, 
		       &tmphit->Score, 
		       &tmphit->Pval, 
		       &tmphit->Eval);
	    ajListPush(list, (void *)tmphit);
	}
    }

    ret = embHitlistNew(ajListLength(list));
    ajStrAssS(&ret->Class, class);
    ajStrAssS(&ret->Fold, fold);
    ajStrAssS(&ret->Superfamily, super);
    ajStrAssS(&ret->Family, family);
    ret->Sunid_Family = Sunid_Family;
    
    ret->N=ajListToArray(list, (void ***)&(ret->hits));
    

    ajListDel(&list);
    ajStrDel(&class);
    ajStrDel(&fold);
    ajStrDel(&super);
    ajStrDel(&family);
    ajStrDel(&line);
    
    return ret;
}





/* @func embSignatureHitsWrite **********************************************
**
** Writes a list of AjOHit objects to an output file (see documentation
** for the DOMAINATRIX "sigscan" application). This is intended for 
** displaying the results from scans of a signature against a protein sequence
** database.  The Hitlist must have first been classified by a call to 
** embHitlistClassify.  Hits up to the first user-specified number of false
** hits are written.
**
** @param [u] outf    [AjPFile]      Output file stream
** @param [r] sig     [const AjPSignature] Signature object
** @param [r] hitlist [const AjPHitlist]   Hitlist objects with hits from scan
** @param [r] n       [ajint]        Max. no. false hits to output
**
** @return [AjBool] True if file was written
** @@
****************************************************************************/

AjBool embSignatureHitsWrite(AjPFile outf, const AjPSignature sig, 
			     const AjPHitlist hitlist, ajint n)
{
    ajint  x  = 0;
    ajint  nf = 0;
    
    
    /* Check args */
    if(!outf || !hitlist || !sig)
	return ajFalse;

    
    /* Print header info */
    ajFmtPrintF(outf, "DE   Results of signature search\nXX\n");


    /* Print SCOP classification records of signature */
    ajFmtPrintF(outf,"CL   %S",sig->Class);
    ajFmtPrintSplit(outf,sig->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,sig->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,sig->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nSI   %d\n", sig->Sunid_Family);
    ajFmtPrintF(outf,"XX\n");
    
    
    /* Loop through list and print out data */
    for(x=0;x<hitlist->N; x++)
    {
	if(ajStrMatchC(hitlist->hits[x]->Typeobj, "FALSE"))
	    nf++;
	if(nf>n)
	    break;
	if(MAJSTRLEN(hitlist->hits[x]->Acc))
	    ajFmtPrintF(outf, "HI  %-6d%-10S%-5d%-5d%-15S%-10S%-10S%-7.1f"
			"%-7.3f%-7.3f\n", 
			x+1, hitlist->hits[x]->Acc, 
			hitlist->hits[x]->Start+1, hitlist->hits[x]->End+1,
			hitlist->hits[x]->Group, 
			hitlist->hits[x]->Typeobj, hitlist->hits[x]->Typesbj, 
			hitlist->hits[x]->Score, hitlist->hits[x]->Pval,
			hitlist->hits[x]->Eval);
	else
	    ajFmtPrintF(outf, "HI  %-6d%-10S%-5d%-5d%-15S%-10S%-10S"
			"%-7.1f%-7.3f%-7.3f\n", 
			x+1, hitlist->hits[x]->Spr, 
			hitlist->hits[x]->Start+1, hitlist->hits[x]->End+1,
			hitlist->hits[x]->Group, 
			hitlist->hits[x]->Typeobj, hitlist->hits[x]->Typesbj, 
			hitlist->hits[x]->Score, hitlist->hits[x]->Pval,
			hitlist->hits[x]->Eval);
    }
    

    ajFmtPrintF(outf, "XX\n//\n");

    return ajTrue;
}
















/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */

/* @section Miscellaneous ***************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories. 
**
****************************************************************************/

/* @func embHitlistClassify *************************************************
**
** Classifies a list of signature-sequence hits (held in a Hitlist object) 
** according to list of target sequences (a list of Hitlist objects).
** 
** Writes the Group, Typeobj (primary classification) & Typesbj (secondary
** classification) elements depending on how the SCOP classification 
** records of the Hit object and target sequence in question compare.
** 
**
** The following classification of hits is taken from the documentation
** for the DOMAINATRIX "sigscan" application :
** Definition of classes of hit 
** The primary classification is an objective definition of the hit and has 
** one of the following values:
** SEED - the sequence was included in the original alignment from which the 
** signature was generated.
** HIT - A protein which was detected by psiblast  (see psiblasts.c) to 
** be a homologue to at least one of the proteins in the family from which 
** the signature was derived. Such proteins are identified by the 'HIT' 
** record in the scop families file.
** OTHER - A true member of the family but not a homologue as detected by 
** psi-blast. Such proteins may have been found from the literature and 
** manually added to the scop families file or may have been detected by the 
** EMBOSS program swissparse (see swissparse.c). They are identified in the 
** scop families file by the 'OTHER' record.
** CROSS - A protein which is homologous to a protein of the same fold,
** but differnt family, of the proteins from which the signature was
** derived.
** FALSE - A homologue to a protein with a different fold to the family
** of the signature.
** UNKNOWN - The protein is not known to be CROSS, FALSE or a true hit (a 
** SEED, HIT or OTHER).
** The secondary classification is provided for convenience and a value as 
** follows:
** Hits of SEED, HIT and OTHER classification are all listed as TRUE.
** Hits of CROSS, FALSE or UNKNOWN objective
** classification are listed as CROSS, 
** FALSE or UNKNOWN respectively.
**
** The Group element is copied from the target sequence for 'TRUE' objective
** hits, whereas 'NOT_APPLICABLE' is given for other types of hit.
**
** The subjective column allows for hand-annotation of the hits files so that 
** proteins of UNKNOWN objective classification can re-classified by a human 
** expert as TRUE, FALSE, CROSS or otherwise
** left as UNKNOWN for the purpose of 
** generating signature performance plots with the EMBOSS application sigplot.
**
**
** @param [r] hits    [AjPHitlist const *] Pointer to Hitlist object with hits
** @param [r] targets [const AjPList]   List of AjOHitlist objects with targets
** @param [r] thresh  [ajint]       Minimum length (residues) of overlap 
** required for two hits with the same code to be counted as the same hit.
**
** @return [AjBool] True on success, else False
** @@
****************************************************************************/

AjBool embHitlistClassify(AjPHitlist const *hits,
			  const AjPList targets, ajint thresh)
{  
    /*
    ** A list of Hitidx structures is derived from the list of AjOHitlist 
    ** objects to allow rapid searching for a given protein accession number
    */
    AjIList itert   = NULL;	/* List iterator for targets */
    AjPHitlist ptrt = NULL;	/* Pointer for targets (hitlist structure) */
    EmbPHitidx ptri  = NULL;	/* Pointer for index (Hitidx structure) */

    EmbPHitidx *idxarr = NULL;	/* Array of Hitidx structures */
    AjPList idxlist   = NULL;	/* List of Hitidx structures */

    ajint idxsiz = 0;		/* No.target sequences */
    ajint pos    = 0;		/* Position of a matching code in Hitidx 
					  structure */
    ajint tpos = 0;		/* Temp. position counter */
    ajint x    = 0;		/* Loop counter */

    AjPStr tmpstr = NULL;
    

    
    /* Check args */
    if(!(*hits) || (!targets))
    {
	ajWarn("NULL args passed to embHitlistClassify\n");
	return ajFalse;
    }
    


    /* Create list & list iterator & other memory */
    itert   = ajListIterRead(targets);
    idxlist = ajListNew();
    tmpstr  = ajStrNew();
    

    /* Loop through list of targets filling list of Hitidx structures */
    while((ptrt=(AjPHitlist)ajListIterNext(itert)))
    {
	/* Write Hitidx structure */
	for(x=0;x<ptrt->N;x++)
	{
	    ptri = embHitidxNew();
	    ptri->hptr=ptrt->hits[x];
	    ptri->lptr=ptrt;
	    if(MAJSTRLEN(ptrt->hits[x]->Acc))
		ajStrAssS(&ptri->Id, ptrt->hits[x]->Acc);
	    else
		ajStrAssS(&ptri->Id, ptrt->hits[x]->Spr);
	    
	    ajListPush(idxlist,(EmbPHitidx) ptri);
	}
    }

    
    /* Order the list of Hitidx structures by Id and transform 
       into an array */
    ajListSort(idxlist, embHitidxMatchId);
    idxsiz = ajListToArray(idxlist, (void ***) &idxarr);
        

    /* Loop through list of hits */
    for(x=0; x<(*hits)->N; x++)
    {
	if((MAJSTRLEN((*hits)->hits[x]->Acc)))
	    pos=embHitidxBinSearch((*hits)->hits[x]->Acc, idxarr, idxsiz);
	else
	    pos=embHitidxBinSearch((*hits)->hits[x]->Spr, idxarr, idxsiz);
	if(pos!=-1)
	{
	    /*
	    ** Id was found
	    ** The list may contain multiple entries for the same Id, so 
	    ** search the current position and then up the list for other 
	    ** matching strings
	    */
	    tpos=pos; 

	    if(MAJSTRLEN((*hits)->hits[x]->Acc))
		ajStrAssS(&tmpstr, (*hits)->hits[x]->Acc);
	    else
		ajStrAssS(&tmpstr, (*hits)->hits[x]->Spr);

	    while(ajStrMatchCase(idxarr[tpos]->Id, tmpstr))
	    {
		if(embHitsOverlap(idxarr[tpos]->hptr, 
				    (*hits)->hits[x], thresh))
		{	

		    if( (idxarr[tpos]->lptr)->Sunid_Family ==
		       (*hits)->Sunid_Family)
			/* SCOP family is identical */
		    {
			ajStrAssS(&(*hits)->hits[x]->Typeobj, 
				  (idxarr[tpos]->hptr)->Typeobj);

			ajStrAssC(&(*hits)->hits[x]->Typesbj, 
				  "TRUE");
			ajStrAssS(&(*hits)->hits[x]->Group, 
				  (idxarr[tpos]->hptr)->Group);
		    }
		    else if((ajStrMatchCase((idxarr[tpos]->lptr)->Fold, 
					   (*hits)->Fold)) &&
			    (ajStrMatchCase((idxarr[tpos]->lptr)->Class, 
					   (*hits)->Class)))
			/* SCOP folds are identical */
		    {
			ajStrAssC(&(*hits)->hits[x]->Typeobj, "CROSS");
			ajStrAssC(&(*hits)->hits[x]->Typesbj, "CROSS");

			ajStrAssC(&(*hits)->hits[x]->Group, "NOT_APPLICABLE");
		    }
		    else
			/* SCOP folds are different */
		    {
			ajStrAssC(&(*hits)->hits[x]->Typeobj, "FALSE");
			ajStrAssC(&(*hits)->hits[x]->Typesbj, "FALSE");
			ajStrAssC(&(*hits)->hits[x]->Group, "NOT_APPLICABLE");
		    }
		}
		else
		{
		    /*
		    ** Id was found but there was no overlap so set 
		    ** classification to UNKNOWN, but only if it has 
		    ** not already been set
		    */
		    if((!ajStrMatchC((*hits)->hits[x]->Typesbj, "TRUE")) &&
		       (!ajStrMatchC((*hits)->hits[x]->Typesbj, "CROSS")) &&
		       (!ajStrMatchC((*hits)->hits[x]->Typesbj, "FALSE")))
		    {
			ajStrAssC(&(*hits)->hits[x]->Typeobj, "UNKNOWN");
			ajStrAssC(&(*hits)->hits[x]->Typesbj, "UNKNOWN");
			ajStrAssC(&(*hits)->hits[x]->Group, "NOT_APPLICABLE");
		    }
		}
		tpos--;	
		if(tpos<0) 
		    break;
	    }	    
				    
	    /* Search down the list */
	    tpos = pos+1; 

	    if(MAJSTRLEN((*hits)->hits[x]->Acc))
		ajStrAssS(&tmpstr, (*hits)->hits[x]->Acc);
	    else
		ajStrAssS(&tmpstr, (*hits)->hits[x]->Spr);

	    if(tpos<idxsiz) 
		while(ajStrMatchCase(idxarr[tpos]->Id, tmpstr))
		{

		    if(embHitsOverlap(idxarr[tpos]->hptr, 
					(*hits)->hits[x], thresh))
		    {	
			if( (idxarr[tpos]->lptr)->Sunid_Family ==
			   (*hits)->Sunid_Family)

			    /* SCOP family is identical */
			{
			    ajStrAssS(&(*hits)->hits[x]->Typeobj, 
				     (idxarr[tpos]->hptr)->Typeobj);
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "TRUE");
			    ajStrAssS(&(*hits)->hits[x]->Group, 
				      (idxarr[tpos]->hptr)->Group);
			}
			else if((ajStrMatchCase((idxarr[tpos]->lptr)->Fold, 
					       (*hits)->Fold)) &&
				(ajStrMatchCase((idxarr[tpos]->lptr)->Class, 
					   (*hits)->Class)))
			    /* SCOP fold is identical */
			{	
			    ajStrAssC(&(*hits)->hits[x]->Typeobj, "CROSS");
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "CROSS");
			    ajStrAssC(&(*hits)->hits[x]->Group,
				      "NOT_APPLICABLE");
			}
			else
			    /* SCOP folds are different */
			{
			    ajStrAssC(&(*hits)->hits[x]->Typeobj, "FALSE");
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "FALSE");
			    ajStrAssC(&(*hits)->hits[x]->Group,
				      "NOT_APPLICABLE");
			}
		    }
  		    else
		    {
			/*
			** Id was found but there was no overlap so set 
			** classification to UNKNOWN, but only if it has 
			** not already been set
			*/
			if((!ajStrMatchC((*hits)->hits[x]->Typesbj, "TRUE")) &&
		       (!ajStrMatchC((*hits)->hits[x]->Typesbj, "CROSS")) &&
		       (!ajStrMatchC((*hits)->hits[x]->Typesbj, "FALSE")))
			{
			    ajStrAssC(&(*hits)->hits[x]->Typeobj, "UNKNOWN");
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "UNKNOWN");
			    ajStrAssC(&(*hits)->hits[x]->Group,
				      "NOT_APPLICABLE");
			}
		    }
		    tpos++;	
		    if(tpos==idxsiz) 
			break;
		}
	}
	else
	{
	    /* Id was NOT found so set classification to UNKNOWN */
	    ajStrAssC(&(*hits)->hits[x]->Typeobj, "UNKNOWN");
	    ajStrAssC(&(*hits)->hits[x]->Typesbj, "UNKNOWN");
	    ajStrAssC(&(*hits)->hits[x]->Group, "NOT_APPLICABLE");
	}
    }
    

    while(ajListPop(idxlist, (void **) &ptri))
	embHitidxDel(&ptri);	
    ajListDel(&idxlist);
    AJFREE(idxarr);
    ajListIterFree(&itert);
    ajStrDel(&tmpstr);

    return ajTrue;
}





/* @func embSignatureCompile ************************************************
**
** Function to compile a signature: calls embSigposNew to allocate an array
** of AjOSigpos objects within an AjOSignature object, and then writes this
** array. The signature must first have been allocated by using the 
** embSignatureNew function.
**
** @param [w] S      [AjPSignature*] Signature object
** @param [r] gapo   [float]         Gap opening penalty
** @param [r] gape   [float]         Gap extension penalty
** @param [r] matrix [const AjPMatrixf]    Residue substitution matrix
**
** @return [AjBool] True if array was written succesfully.
** @@
****************************************************************************/

AjBool embSignatureCompile(AjPSignature *S, float gapo, float gape, 
			   const AjPMatrixf matrix)
{
    AjPSeqCvt cvt   = NULL;   /* Conversion array for AjPMatrixf */
    float     **sub = NULL;   /* Substitution matrix from AjPMatrixf */
    ajint     x     = 0;      
    ajint     y     = 0;      
    ajint     z     = 0;      
    AjBool    *tgap = NULL;   /* Temporary array of gap sizes. A cell 
			         == True if a gap is permissible */
    float     *tpen = NULL;   /* Temporary array of gap penalties */
    ajint     dim   = 0;      /* Dimension of tgap & tpen arrays */
    float     pen   = 0.0;    /* Gap penalty */
    ajint     ngap  = 0;      /* No. of gaps */
    ajint     div   = 0;      /* Used in calculating residue match values */
     
    /* CHECK ARGS */
    if(!(*S) || !((*S)->dat) || !((*S)->pos) || !matrix)
	return ajFalse;
    

    /* INITIALISE SUBSTITUTION MATRIX */
    sub  = ajMatrixfArray(matrix);
    cvt  = ajMatrixfCvt(matrix);    

    

    /* LOOP FOR EACH SIGNATURE POSITION */
    for(x=0; x<(*S)->npos; x++)
    {
	/*
	** ALLOCATE TEMP. ARRAY OF GAP SIZES, OF SIZE == 
	** LARGEST GAP + WINDOW SIZE.  NOTE IT IS ESSENTIAL 
	** THAT THE GAP DATA IS GIVEN IN ORDER OF INCREASING
	** GAP SIZE IN THE STRUCTURE
	*/
	/*      (*S)->dat[x]->gsiz[(*S)->dat[x]->ngap - 1] */
	dim = (ajIntGet((*S)->dat[x]->gsiz, (*S)->dat[x]->ngap - 1))
	    + (*S)->dat[x]->wsiz + 1;
	AJCNEW0(tgap, dim);
	AJCNEW0(tpen, dim);
	
	/* FILL TEMP ARRAYS */
	for(y=0; y<(*S)->dat[x]->ngap; y++)
	{
	    /* GAP NOT EXTENDED BY WINDOW */
	    tgap[(ajIntGet((*S)->dat[x]->gsiz, y))]=ajTrue;
	    tpen[(ajIntGet((*S)->dat[x]->gsiz, y))]=0;
	    

	    /* GAP IS EXTENDED BY WINDOW */
	    for(z=1; z<=(*S)->dat[x]->wsiz; z++)
	    {
		pen=gapo+gape*(z-1);
		
		/* A penalty has been assigned for this gap distance before */
		if(tgap[(ajIntGet((*S)->dat[x]->gsiz, y))+z])
		{
		    /* Write the new penalty value if it is lower than the 
		       existing one */
		    if( pen < tpen[(ajIntGet((*S)->dat[x]->gsiz, y))+z])
			tpen[(ajIntGet((*S)->dat[x]->gsiz, y))+z]=pen;
		}
		/* We have not assigned a penalty to this gap distance before */
		else
		{
		    tpen[(ajIntGet((*S)->dat[x]->gsiz, y))+z]=pen;
		    tgap[(ajIntGet((*S)->dat[x]->gsiz, y))+z]=ajTrue;
		}
		
		
		/* A penalty has been assigned for this gap distance before */
		if( ajIntGet((*S)->dat[x]->gsiz, y)-z >= 0)
		{
		    /* Write the new penalty value if it is lower than the 
		       existing one */
		    if(tgap[ajIntGet((*S)->dat[x]->gsiz, y)-z])
		    {
			if(pen < tpen[(ajIntGet((*S)->dat[x]->gsiz, y))-z])
			    tpen[(ajIntGet((*S)->dat[x]->gsiz, y))-z]=pen;
		    }
		    /*
		    ** We have not assigned a penalty to this gap
		    ** distance before
		    */
		    else
		    { 
			tpen[(ajIntGet((*S)->dat[x]->gsiz, y))-z]=pen;
			tgap[(ajIntGet((*S)->dat[x]->gsiz, y))-z]=ajTrue;
		    }
		}
	    }
	}
	

	/* ALLOCATE ARRAY OF Sigpos OBJECTS */
	for(ngap=0, y=0; y<dim; y++)
	    if(tgap[y])
		ngap++;
	(*S)->pos[x] = embSigposNew(ngap);



	/*ASIGN THE GAP DATA  */
	for(ngap=0, y=0; y<dim; y++)
	    if(tgap[y])
	    {
		(*S)->pos[x]->gsiz[ngap]=y;
		(*S)->pos[x]->gpen[ngap]=tpen[y];
		ngap++;
	    }

	/* CALCULATE RESIDUE MATCH VALUES */
	for(z=0;z<26; z++)
	{
	    for(div=0, y=0; y<(*S)->dat[x]->nres; y++)
	    {
		div+=(ajIntGet((*S)->dat[x]->rfrq, y));
		
		(*S)->pos[x]->subs[z] += 
		    (ajIntGet((*S)->dat[x]->rfrq, y)) * 
			sub[ajSeqCvtK(cvt,(char)((ajint)'A'+z))]
			    [ajSeqCvtK(cvt, ajChararrGet((*S)->dat[x]->rids,
							 y))];
	    }
	    (*S)->pos[x]->subs[z] /= div;
	}
	
	/* FREE tgap & tpen ARRAYS */
	AJFREE(tgap);
	AJFREE(tpen);
    }
        
    return ajTrue;
}





/* @func embSignatureAlignSeq ***********************************************
**
** Performs an alignment of a signature to a protein sequence. The signature
** must have first been compiled by calling the embSignatureCompile 
** function.
** A Hit object is written.
**
** @param [r] S      [const AjPSignature] Signature object
** @param [r] seq    [const AjPSeq]       Protein sequence
** @param [w] hit    [AjPHit*]      Hit object pointer
** @param [r] nterm  [ajint]        N-terminal matching option
**
** @return [AjBool] True if a signature-sequence alignment was successful and 
** the Hit object was written.  Returns False if there was an internal error, 
** bad arg's etc. or in cases where a sequence is rejected because of 
** N-terminal matching options). 
** @@
****************************************************************************/

AjBool embSignatureAlignSeq(const AjPSignature S, const AjPSeq seq,
			    AjPHit *hit, 
			    ajint nterm)
{
    const AjPStr  P     = NULL; 
    ajint  gidx   = 0;	  /*Index into gap array */
    ajint  glast  = 0;	  /*Index of last gap to try */
    ajint  nres   = 0;	  /*No. of residues in protein */
    ajint  nresm1 = 0;	  /*== nres-1 */
    static EmbPSigcell path = NULL;  /*Path matrix as 1D array */

    ajint dim =0;         /*Dimension of 1D path matrix == nres 
				   * S->npos */
    static char *p = NULL;  /*Protein sequence */
    ajint  start   = 0;	    /* Index into path matrix of first position 
			    ** in the previous row to grow an alignment 
			    ** from
			    */
    ajint startp = 0;	  /*Index into protein sequence for this 
				    position */
    ajint stop  = 0;	  /*Index into path matrix of last position in 
				    previous row to grow an alignment from */
    ajint this  = 0;	  /*Index into path matrix for current row */
    ajint last  = 0;	  /*Index into path matrix for last row */
    ajint thisp = 0;	  /*Index into protein sequence for current row */
    ajint lastp = 0;	  /*Index into protein sequence for last row */
    ajint sidx  = 0;	  /*Index into signature */
    float val   = 0;	  /*Value for signature position:residue match */
    float mval  = 0;	  /*Max. value of matches of last signature 
		 	    position:protein sequence */
    ajint max  = 0;	  /*Index into path matrix for cell with mval */
    ajint maxp = 0;      /*Index into protein sequence for path matrix 
				    cell with mval */
    static char *alg = NULL;   /*String for alignment */
    ajint cnt;                 /*A loop counter */
    ajint mlen = 0;            /*Min. possible length of the alignment of the 
				    signature */
    float score=0;             /*Score for alignment */
    

    /* CHECK ARGS AND CREATE STRINGS */
    if(!S || !seq || !hit)
	return ajFalse;
    

    P = ajSeqStr(seq);
    /*Check protein sequence contains alphabetic characters only */
    if(!ajStrIsAlpha(P))
	return ajFalse;


    /* INITIALISE VARIABLES */
    nres   = ajStrLen(P);    /* No. columns in path matrix */
    nresm1 = nres-1;         /* Index of last column in path matrix */
    dim = nres * S->npos;


    /* ALLOCATE MEMORY */
    /*First time the function is called */
    if(!path)
    {
	/* CREATE PATH MATRIX */
	AJCNEW(path, dim);

	/* CREATE ALIGNMENT AND PROTEIN SEQUENCE STRINGS */
	alg = AJALLOC((nres*sizeof(char))+1);
	p = AJALLOC((nres*sizeof(char))+1);
    }	
    else 
    {
	/* CREATE PATH MATRIX */
	if(dim > (ajint) sizeof(path)/sizeof(EmbOSigcell))
	    AJCRESIZE(path, dim);

	/* CREATE ALIGNMENT AND PROTEIN SEQUENCE STRINGS */
	if((nres) > (ajint) sizeof(alg)/sizeof(char))
	{
	    AJCRESIZE(alg, nres+1);
	    AJCRESIZE(p, nres+1);
	}
    }



    /*
    ** INITIALISE PATH MATRIX
    ** Only necessary to initilise <try> element to ajFalse
    */
    for(cnt=0;cnt<dim;cnt++)
	path[cnt].visited = ajFalse;
    
    


    /* COPY SEQUENCE AND CONVERT TO UPPER CASE, OVERWRITE ALIGNMENT STRING */
    strcpy(p, ajStrStr(P));
    ajCharToUpper(p);
    for(cnt=0;cnt<nres;cnt++)
	alg[cnt] = '-';
    alg[cnt] = '\0';
    
    switch(nterm)
    {
    case 1:
	/*
	** The first position 
	** can be aligned anywhere in the protein sequence, so long
	** as there is sufficient space to align the rest of the 
	** signature (this is fast, but might not be ideal, e.g. for
        ** detection of fragments.).  Note that gap distance for 
	** first signature position is ignored. Note the function 
	** will return if the whole of the signature can not
	** be aligned
	** This is the RECOMMENDED option
	*/
	
	/*
	** Find last gap to try for first sig. position and return an 
	** error if first sig. position cannot be fitted
	*/
	mlen = 1;   /*For first signature position */
	for(sidx=1;sidx<S->npos;sidx++)
	    mlen+=(1+S->pos[sidx]->gsiz[0]);
	start = startp = 0;
	stop  = nres-mlen;
	if(stop<0)
	    return ajFalse;    
	
	/*
	** Assign path matrix for row 0. 'this' is index into both path
	** matrix and protein sequence in this case.  There is no gap 
	** penalty for the first position.
	** Assign indices into path matrix of start and stop positions for
	** row 0.  
	** Assign index into protein sequence for start position.
	*/
	for(this=0;this<=stop;this++)
	{
	    path[this].val = S->pos[0]->subs[(ajint) ((ajint)p[this] -
						      (ajint)'A')];
	    path[this].prev    = 0;
	    path[this].visited = ajTrue;
	}
	break;
	
    case 2:
	/*
	** The first position 
	** can be aligned anywhere in the protein sequence (this is
	** slower, but means that, e.g. high scoring alignments that
	** are lacking C-terminal signature positions, will not be 
	** discarded.
	*/
	
	for(this=0;this<nres;this++)
	{
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] -
						    (ajint)'A')];
	    path[this].prev=0;
	    path[this].visited=ajTrue;
	}
	start = startp = 0;
	stop  = nresm1;
	break;
	
    case 3:
	/*
	** Use empirical gaps only, rather than allowing the 
	** first signature positions to be aligned to anywhere
	** within the sequence
	*/
	
	for(glast=S->pos[0]->ngaps-1; glast>=0; glast--)
	    if(S->pos[0]->gsiz[glast]<nresm1)
		break;
	if(glast==-1)
	    return ajFalse;
	
	
	
	for(gidx=0; gidx<=glast; ++gidx)
	{	
	    this=S->pos[0]->gsiz[gidx];
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] -
						    (ajint)'A')];
	    path[this].prev    = 0;
	    path[this].visited = ajTrue;
	}
	startp = start = S->pos[0]->gsiz[0];
	stop=S->pos[0]->gsiz[gidx-1];
	break;
	
    default:
	ajFatal("Bad nterm value for embSignatureAlignSeq. "
		"This should never happen.\n");
	break;
    }
    
    
    /*
    ** Assign path matrix for other rows
    ** Loop for each signature position, beginning at row 1
    */
    for(sidx=1;sidx<S->npos;sidx++)
    {
	/*Loop for permissible region of previous row */
	for(last=start, lastp=startp; last<=stop; last++, lastp++)
	{
	    if(path[last].visited==ajFalse)
		continue;

	    /*Loop for each permissible gap in current row */
	    for(gidx=0;gidx<S->pos[sidx]->ngaps;gidx++)
	    {
		if((thisp=lastp+S->pos[sidx]->gsiz[gidx]+1)>nresm1)
		    break;

		this = last+nres+S->pos[sidx]->gsiz[gidx]+1;
		val  = path[last].val +
		    S->pos[sidx]->subs[(ajint) (p[thisp] - (ajint)'A')] -
			S->pos[sidx]->gpen[gidx];
		

		if((path[this].visited==ajTrue)&&(val > path[this].val))
		{
		    path[this].val  = val;
		    path[this].prev = last;
		    continue;
		}				
		/*The cell hasn't been visited before so give it a score */
		if(path[this].visited==ajFalse)
		{
		    path[this].val     = val;
		    path[this].prev    = last;
		    path[this].visited = ajTrue;
		    continue;
		}	
	    }
	}
    	
	/* We cannot accomodate the next position */
	if((startp+=(1+S->pos[sidx]->gsiz[0]))>=nresm1)
	    break;
	start += (nres+1+S->pos[sidx]->gsiz[0]);
	/*last gives (index into last position tried)+1  because
	  of loop increment.  */
	
	stop=this;
    }


    /*
    ** Find index into protein sequence and number of signature position 
    ** (row) corresponding to the last cell in the path matrix which was
    ** assigned
    */
    thisp = this - (ajint) ((sidx=(ajint)floor((double)(this/nres))) * nres);
    


    /*
    ** Find maximal value in this row ... give mval a silly value
    ** so it is assigned at least once
    */
    for(mval=-1000000 ; thisp>=0; this--, thisp--)
    {
	if(path[this].visited==ajFalse)
	    continue;
	if(path[this].val > mval)
	{
	    mval = path[this].val;
	    max  = this;
	    maxp = thisp;
	}
    }


    /*Assign score for alignment */
    score = mval; 
    score /= S->npos;


    /* Backtrack through matrix */
    alg[maxp] = '*';

    for(this=path[max].prev; sidx>0; this=path[this].prev)
    {
	thisp= this - (ajint) ((sidx=(ajint)floor((double)(this/nres))) 
			       * nres);
	alg[thisp] = '*';
    }
    

    /* Write hit structure */
    ajStrAssC(&(*hit)->Alg, alg);
    ajStrAssS(&(*hit)->Seq, P);
    (*hit)->Start=thisp;
    (*hit)->End=maxp;
    ajStrAssS(&(*hit)->Acc, ajSeqGetAcc(seq));
    if(!MAJSTRLEN((*hit)->Acc))
	ajStrAssS(&(*hit)->Acc, ajSeqGetName(seq));
    if(!MAJSTRLEN((*hit)->Acc))
	ajWarn("Could not find an accession number or name for a sequence"
	       " in embSignatureAlignSeq");
    (*hit)->Score=score;
    
    return ajTrue;
}





/* @func embSignatureAlignSeqall ********************************************
**
** Aligns a signature to a set of sequences and writes a Hitlist object with 
** the results. The top-scoring <n> hits are written. The signature must have 
** first been compiled by calling the embSignatureCompile function.
** Memory for an Hitlist object must be allocated beforehand by using the 
** Hitlist constructor with an arg. of 0.
**
** @param [r] sig      [const AjPSignature] Signature object
** @param [u] db       [AjPSeqall]    Protein sequences
** @param [r] n        [ajint]        Max. number of top-scoring hits to store
** @param [w] hitlist  [AjPHitlist*]  Hitlist object pointer
** @param [r] nterm    [ajint]        N-terminal matching option
**
** @return [AjBool] True if Hitlist object was written succesfully.
** @@
****************************************************************************/

AjBool embSignatureAlignSeqall(const AjPSignature sig, AjPSeqall db,
			       ajint n, 
			       AjPHitlist *hitlist, ajint nterm)
{
    ajint   nhits    = 0;        /* Number of hits written to Hitlist object*/
    ajint   hitcnt   = 0;        /* Counter of number of hits */
    AjPHit  hit      = NULL;	 /* The current hit */    
    AjPHit  ptr      = NULL;	 /* Temp. pointer to hit structure */    
    AjPSeq  seq      = NULL;     /* The current protein sequence from db */ 
    AjPList listhits = NULL;     /* Temp. list of hits */

    /* Check args */
    if(!sig || !db || !hitlist)
    {
	ajWarn("NULL arg passed to embSignatureAlignSeqall");
	return ajFalse;
    }
    

    /* Memory allocation */
    listhits = ajListNew();
    /*    seq = ajSeqNew();    */


    /* Initialise Hitlist object with SCOP records from Signature */
    ajStrAssS(&(*hitlist)->Class, sig->Class);
    ajStrAssS(&(*hitlist)->Fold, sig->Fold);
    ajStrAssS(&(*hitlist)->Superfamily, sig->Superfamily);
    ajStrAssS(&(*hitlist)->Family, sig->Family);

        
    /* Search the database */
    while(ajSeqallNext(db,&seq))
    {
	/* Allocate memory for hit */
	hit=embHitNew();
	

	if(!embSignatureAlignSeq(sig, seq, &hit, nterm))
	{	
	    embHitDel(&hit);
	    continue;
	}
	else
	    hitcnt++;
	

	/* Push hit onto list */
	ajListPush(listhits,(AjPHit) hit);
	

	if(hitcnt>n)
	{	
	    /* Sort list according to score, highest first */
	    ajListSort(listhits, embMatchinvScore);
	 

	    /* Pop the hit (lowest scoring) from the bottom of the list */
	    ajListPopEnd(listhits, (void *) &ptr);
	    embHitDel(&ptr);
	}
    }
    

    /* Sort list according to score, highest first */
    ajListSort(listhits, embMatchinvScore);


    /* Convert list to array within Hitlist object */
    nhits=ajListToArray(listhits, (void ***)  &(*hitlist)->hits);
    (*hitlist)->N = nhits;
    

    ajListDel(&listhits);
    ajSeqDel(&seq);

    return ajTrue;
}





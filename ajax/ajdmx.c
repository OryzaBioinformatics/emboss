/****************************************************************************
** 
** @source ajdmx.c 
**
** AJAX library code for some of the DOMAINATRIX EMBASSY applications. 
** For use with the Scophit and Scopalign objects.  The code is disparate 
** including low-level functions and algorithms.  The functionality will 
** eventually be subsumed by other AJAX and NUCLEUS libraries. 
** 
** @author: Copyright (C) 2004 Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author: Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
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
****************************************************************************/





/* ======================================================================= */
/* ============================ include files ============================ */
/* ======================================================================= */

#include <math.h>
#include "ajax.h"





/* ======================================================================= */
/* ============================ private data ============================= */
/* ======================================================================= */





/* ======================================================================= */
/* ================= Prototypes for private functions ==================== */
/* ======================================================================= */





/* ======================================================================= */
/* ========================== private functions ========================== */
/* ======================================================================= */





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

/* @func ajDmxScophitNew ****************************************************
**
** Scophit object constructor. 
**
** @return [AjPScophit] Pointer to a Scophit object
** @@
****************************************************************************/

AjPScophit ajDmxScophitNew(void)
{
    AjPScophit ret = NULL;

    AJNEW0(ret);

    ret->Class       = ajStrNew();
    ret->Fold        = ajStrNew();
    ret->Superfamily = ajStrNew();
    ret->Family      = ajStrNew();
    ret->Seq         = ajStrNew();
    ret->Acc         = ajStrNew();
    ret->Spr         = ajStrNew();
    ret->Typeobj     = ajStrNew();
    ret->Typesbj     = ajStrNew();
    ret->Model       = ajStrNew();
    ret->Alg         = ajStrNew();
    ret->Group       = ajStrNew();
    ret->Start        = 0;
    ret->End          = 0;
    ret->Rank         = 0;
    ret->Score        = 0;    
    ret->Sunid_Family = 0;
    ret->Eval         = 0;
    ret->Pval         = 0;
    ret->Target       = ajFalse;
    ret->Target2      = ajFalse;
    ret->Priority     = ajFalse;
    
    return ret;
}





/* @func ajDmxScopalgNew ****************************************************
**
** Scopalg object constructor. This is normally called by the ajDmxScopalgRead
** function. Fore-knowledge of the number of sequences is required.
**
** @param [r] n [ajint]   Number of sequences
** 
** @return [AjPScopalg] Pointer to a Scopalg object
** @@
****************************************************************************/

AjPScopalg ajDmxScopalgNew(ajint n)
{
    AjPScopalg ret = NULL;
    ajint i = 0;
    
    AJNEW0(ret);
    ret->Class        = ajStrNew();
    ret->Fold         = ajStrNew();
    ret->Superfamily  = ajStrNew();
    ret->Family       = ajStrNew();
    ret->Post_similar = ajStrNew();
    ret->Positions    = ajStrNew();
    ret->width = 0;
    ret->N = n;

    if(n)
    {
	AJCNEW0(ret->Codes,n);
	for(i=0;i<n;++i)
	    ret->Codes[i] = ajStrNew();

	AJCNEW0(ret->Seqs,n);
	for(i=0;i<n;++i)
	    ret->Seqs[i] = ajStrNew();
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

/* @func ajDmxScophitDel ****************************************************
**
** Destructor for Scophit object.
**
** @param [w] pthis [AjPScophit*] Scophit object pointer
**
** @return [void]
** @@
****************************************************************************/

void ajDmxScophitDel(AjPScophit *pthis)
{
    ajStrDel(&(*pthis)->Class);
    ajStrDel(&(*pthis)->Fold);
    ajStrDel(&(*pthis)->Superfamily);
    ajStrDel(&(*pthis)->Family);
    ajStrDel(&(*pthis)->Seq);
    ajStrDel(&(*pthis)->Acc);
    ajStrDel(&(*pthis)->Spr);
    ajStrDel(&(*pthis)->Typeobj);
    ajStrDel(&(*pthis)->Typesbj);
    ajStrDel(&(*pthis)->Model);
    ajStrDel(&(*pthis)->Alg);
    ajStrDel(&(*pthis)->Group);

    AJFREE(*pthis);
    *pthis = NULL;
    
    return;
}





/* @func ajDmxScophitDelWrap ************************************************
**
** Wrapper to destructor for Scophit object for use with generic functions.
**
** @param [d] ptr [const void **] Object pointer
**
** @return [void]
** @@
****************************************************************************/

void ajDmxScophitDelWrap(const void  **ptr)
{
    AjPScophit *del;

    del = (AjPScophit *) ptr;
    
    ajDmxScophitDel(del);
    
    return;
}





/* @func ajDmxScopalgDel ****************************************************
**
** Destructor for Scopalg object.
**
** @param [d] pthis [AjPScopalg*] Scopalg object pointer
**
** @return [void]
** @@
****************************************************************************/

void ajDmxScopalgDel(AjPScopalg *pthis)
{
    int x = 0;  /* Counter */
    
    ajStrDel(&(*pthis)->Class);
    ajStrDel(&(*pthis)->Fold);
    ajStrDel(&(*pthis)->Superfamily);
    ajStrDel(&(*pthis)->Family);
    ajStrDel(&(*pthis)->Post_similar);
    ajStrDel(&(*pthis)->Positions);

    for(x=0;x<(*pthis)->N; x++)
    {
	ajStrDel(&(*pthis)->Codes[x]);
	ajStrDel(&(*pthis)->Seqs[x]);
    }
    
    AJFREE((*pthis)->Codes);
    AJFREE((*pthis)->Seqs);
    
    AJFREE(*pthis);
    *pthis = NULL;
    
    return;
}





/* ======================================================================= */
/* ============================ Assignments ============================== */
/* ======================================================================= */

/* @section Assignments ****************************************************
**
** These functions overwrite the instance provided as the first argument
** A NULL value is always acceptable so these functions are often used to
** create a new instance by assignment.
**
****************************************************************************/

/* @func ajDmxScophitListCopy ***********************************************
**
** Read a list of Scophit structures and returns a pointer to a duplicate 
** of the list. 
** 
** @param [r] ptr [const AjPList]  List of Scophit objects
**
** @return [AjPList] True on success (list was duplicated ok)
** @@
**
** Should modify this eventually to fit "standard" method for assignment
** functions, i.e. pass in the pointer as the first argument
**
****************************************************************************/

AjPList ajDmxScophitListCopy(const AjPList ptr)
{
    AjPList ret    = NULL;
    AjIList iter   = NULL;
    AjPScophit hit = NULL;
    AjPScophit new = NULL;

    /* Check arg's */
    if(!ptr)
    {
	ajWarn("Bad arg's passed to ajDmxScophitListCopy\n");
	return NULL;
    }
    
    /* Allocate the new list */
    ret = ajListNew();
    
    /* Initialise the iterator */
    iter = ajListIterRead(ptr);
    
    /* Iterate through the list of Scophit objects */
    while((hit=(AjPScophit)ajListIterNext(iter)))
    {
	new = ajDmxScophitNew();
	
	ajDmxScophitCopy(&new, hit);

	/* Push scophit onto list */
	ajListPushApp(ret,new);
    }

    ajListIterFree(&iter);

    return ret;
}





/* @func ajDmxScophitCopy ***************************************************
**
** Copies the contents from one Scophit object to another.
**
** @param [w] to   [AjPScophit*] Scophit object pointer 
** @param [r] from [const AjPScophit]  Scophit object 
**
** @return [AjBool] True if copy was successful.
** @@
****************************************************************************/

AjBool ajDmxScophitCopy(AjPScophit *to, const AjPScophit from)
{
    /* Check args */
    if(!(*to) || !from)
	return ajFalse;

    ajStrAssS(&(*to)->Class, from->Class);
    ajStrAssS(&(*to)->Fold, from->Fold);
    ajStrAssS(&(*to)->Superfamily, from->Superfamily);
    ajStrAssS(&(*to)->Family, from->Family);
    ajStrAssS(&(*to)->Seq, from->Seq);
    ajStrAssS(&(*to)->Acc, from->Acc);
    ajStrAssS(&(*to)->Spr, from->Spr);
    ajStrAssS(&(*to)->Typeobj, from->Typeobj);
    ajStrAssS(&(*to)->Typesbj, from->Typesbj);
    ajStrAssS(&(*to)->Model, from->Model);
    ajStrAssS(&(*to)->Alg, from->Alg);
    ajStrAssS(&(*to)->Group, from->Group);
    (*to)->Start = from->Start;
    (*to)->End = from->End;
    (*to)->Rank = from->Rank;
    (*to)->Score = from->Score;
    (*to)->Eval = from->Eval;
    (*to)->Pval = from->Pval;
    (*to)->Target = from->Target;
    (*to)->Target2 = from->Target2;
    (*to)->Priority = from->Priority;
    (*to)->Sunid_Family = from->Sunid_Family;

    return ajTrue;
}





/* ======================================================================= */
/* ============================= Modifiers =============================== */
/* ======================================================================= */

/* @section Modifiers *******************************************************
**
** These functions use the contents of an instance and update them.
**
****************************************************************************/

/* @func ajDmxScophitTargetLowPriority **************************************
**
** Sets the Target element of a Scophit object to True if its Priority is low.
**
** @param [u] h  [AjPScophit *]     Pointer to Scophit object
**
** @return [AjBool] True on success. False otherwise.
** @@
****************************************************************************/

AjBool ajDmxScophitTargetLowPriority(AjPScophit *h)
{
    /* Check args */
    if(!(*h))
    {
	ajWarn("Bad arg's passed to ajDmxScophitTargetLowPriority\n"); 
	return ajFalse;
    }

    if((*h)->Priority==ajFalse)
	(*h)->Target = ajTrue;

    return ajTrue;
}





/* @func ajDmxScophitTarget2 ************************************************
**
** Sets the Target2 element of a Scophit object to True.
**
** @param [u] h  [AjPScophit *]     Pointer to Scophit object
**
** @return [AjBool] True on success. False otherwise.
** @@
****************************************************************************/

AjBool ajDmxScophitTarget2(AjPScophit *h)
{
    /* Check args */
    if(!(*h))
    {
	ajWarn("Bad arg's passed to ajDmxScophitTarget2\n");
	return ajFalse;
    }
    
    (*h)->Target2 = ajTrue;

    return ajTrue;
}





/* @func ajDmxScophitTarget *************************************************
**
** Sets the Target element of a Scophit object to True.
**
** @param [u] h  [AjPScophit *]     Pointer to Scophit object
**
** @return [AjBool] True on success. False otherwise.
** @@
****************************************************************************/
AjBool ajDmxScophitTarget(AjPScophit *h)
{
    /* Check args */
    if(!(*h))
    {
	ajWarn("Bad arg's passed to ajDmxScophitTarget\n");
	return ajFalse;
    }
    
    (*h)->Target = ajTrue;

    return ajTrue;
}





/* ======================================================================= */
/* ========================== Operators ===================================*/
/* ======================================================================= */

/* @section Operators *******************************************************
**
** These functions use the contents of an instance but do not make any 
** changes.
**
****************************************************************************/

/* @func ajDmxScophitCheckTarget ********************************************
**
** Checks to see if the Target element of a Scophit object == ajTrue.
**
** @param [r] ptr [const AjPScophit] Scophit object pointer
**
** @return [AjBool] Returns ajTrue if the Target element of the Scophit 
** object == ajTrue, returns ajFalse otherwise.
** @@
****************************************************************************/

AjBool ajDmxScophitCheckTarget(const AjPScophit ptr)
{
    return ptr->Target;
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





/* @func ajDmxScophitCompScore **********************************************
**
** Function to sort Scophit objects by Score element. Usually called by 
** ajListSort.  
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if score1<score2, 0 if score1==score2, else -1.
** @@
****************************************************************************/

ajint ajDmxScophitCompScore(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    if(p->Score < q->Score)
        return -1;
    else if (p->Score == q->Score)
        return 0;

    return 1;
}





/* @func ajDmxScophitCompPval ***********************************************
**
** Function to sort AjOScophit objects by Pval record. Usually called by 
** ajListSort.
**
** @param [r] hit1  [const void*] Pointer to Hit object 1
** @param [r] hit2  [const void*] Pointer to Hit object 2
**
** @return [ajint] 1 if Pval1>Pval2, 0 if Pval1==Pval2, else -1.
** @@
****************************************************************************/

ajint ajDmxScophitCompPval(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    if(p->Pval < q->Pval)
        return -1;
    else if (p->Pval == q->Pval)
        return 0;

    return 1;
}





/* @func ajDmxScophitCompAcc ************************************************
**
** Function to sort Scophit objects by Acc element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if Acc1 should sort before Acc2,
**                 +1 if the Acc2 should sort first. 
**                  0 if they are identical in length and content. 
** @@
****************************************************************************/

ajint ajDmxScophitCompAcc(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    return ajStrCmpO(p->Acc, q->Acc);
}





/* @func ajDmxScophitCompSunid **********************************************
**
** Function to sort Scophit object by Sunid_Family.
**
** @param [r] entry1  [const void*] Pointer to AjOScophit object 1
** @param [r] entry2  [const void*] Pointer to AjOScophit object 2
**
** @return [ajint] -1 if Sunid_Family1 < Sunid_Family2, +1 if the 
** Sunid_Family2 should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint ajDmxScophitCompSunid(const void *entry1, const void *entry2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)entry1);
    q = (*(AjPScophit*)entry2);
   

    if(p->Sunid_Family < q->Sunid_Family)
        return -1;
    else if(p->Sunid_Family == q->Sunid_Family)
        return 0;

    return 1;
}





/* @func ajDmxScophitCompSpr ************************************************
**
** Function to sort Scophit object by Spr element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if Spr1 should sort before Spr2,
**                 +1 if the Spr2 should sort first. 
**                  0 if they are identical in length and content. 
** @@
****************************************************************************/

ajint ajDmxScophitCompSpr(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    return ajStrCmpO(p->Spr, q->Spr);
}






/* @func ajDmxScophitCompEnd ************************************************
**
** Function to sort Scophit object by End element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if End1 should sort before End2, +1 if the End2 
** should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint ajDmxScophitCompEnd(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
   

    if(p->End < q->End)
	return -1;
    else if(p->End == q->End)
	return 0;

    return 1;
}





/* @func ajDmxScophitCompStart **********************************************
**
** Function to sort Scophit object by Start element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if Start1 should sort before Start2, +1 if the Start2 
** should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint ajDmxScophitCompStart(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
   

    if(p->Start < q->Start)
	return -1;
    else if(p->Start == q->Start)
	return 0;

    return 1;
}





/* @func ajDmxScophitCompFam ************************************************
**
** Function to sort Scophit object by Family element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if Family1 should sort before Family2, +1 if the 
** Family2 should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint ajDmxScophitCompFam(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    return ajStrCmpO(p->Family, q->Family);
}





/* @func ajDmxScophitCompSfam ***********************************************
**
** Function to sort Scophit object by Superfamily  element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if Superfamily1 should sort before Superfamily2, +1 if 
** the Superfamily2 should sort first. 0 if they are identical.
** @@
****************************************************************************/

ajint ajDmxScophitCompSfam(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    return ajStrCmpO(p->Superfamily, q->Superfamily);
}





/* @func ajDmxScophitCompFold ***********************************************
**
** Function to sort Scophit object by Fold element. 
**
** @param [r] hit1  [const void*] Pointer to Scophit object 1
** @param [r] hit2  [const void*] Pointer to Scophit object 2
**
** @return [ajint] -1 if Fold1 should sort before Fold2, +1 if the Fold2 
** should sort first. 0 if they are identical.
** @@
****************************************************************************/
ajint ajDmxScophitCompFold(const void *hit1, const void *hit2)
{
    AjPScophit p = NULL;
    AjPScophit q = NULL;

    p = (*(AjPScophit*)hit1);
    q = (*(AjPScophit*)hit2);
    
    return ajStrCmpO(p->Fold, q->Fold);
}





/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @func ajDmxScopalgGetseqs ************************************************
**
** Read a Scopalg object and writes an array of AjPStr containing the 
** sequences without gaps.
** 
** @param [r] thys     [const AjPScopalg]  Scopalg object
** @param [w] arr      [AjPStr **]   Array of AjPStr 
**
** @return [ajint] Number of sequences read
** @@
****************************************************************************/
ajint ajDmxScopalgGetseqs(const AjPScopalg thys, AjPStr **arr)
{
    ajint i;
        
    /* Check args */
    if(!thys)
    {
	ajWarn("Null args passed to ajDmxScopalgGetseqs");
	return 0;
    }
    
    
    *arr = (AjPStr *) AJCALLOC0(thys->N, sizeof(AjPStr));
    
    for(i=0;i<thys->N;++i)
    {
	(*arr)[i] = ajStrNew();

	ajStrAssS(&((*arr)[i]), thys->Seqs[i]);
	
	ajStrDegap(&((*arr)[i]));
	
    }

    return thys->N;
}




/* @func ajDmxScophitsWrite *************************************************
**
** Write contents of a list of Scophits to an output file in embl-like format
** (see scopalign.c documentation).
** Text for Class, Fold, Superfamily and Family is only written if the text
** is available.
** 
** @param [w] outf [AjPFile] Output file stream
** @param [r] list [const AjPList] List object
**
** @return [AjBool] True on success
** @@
****************************************************************************/

AjBool ajDmxScophitsWrite(AjPFile outf, const AjPList list)
{

    AjIList iter = NULL;
    
    AjPScophit thys = NULL;
    
    iter = ajListIterRead(list);
    

    while((thys = (AjPScophit)ajListIterNext(iter)))
    {
        
        if(!thys)
            return ajFalse;

        if(MAJSTRLEN(thys->Class))
        {
	    ajFmtPrintF(outf,"CL   %S\n",thys->Class);	    
	    ajFmtPrintF(outf, "XX\n");
	}
	
        if(MAJSTRLEN(thys->Fold))
        {
	    ajFmtPrintSplit(outf,thys->Fold,"XX\nFO   ",75," \t\n\r");
	    ajFmtPrintF(outf, "XX\n");
	}
	
        if(MAJSTRLEN(thys->Superfamily))
        {
	    ajFmtPrintSplit(outf,thys->Superfamily,"XX\nSF   ",75," \t\n\r");
	    ajFmtPrintF(outf, "XX\n");
	}
	
        if(MAJSTRLEN(thys->Family))
        {
	    ajFmtPrintSplit(outf,thys->Family,"XX\nFA   ",75," \t\n\r");
	    ajFmtPrintF(outf, "XX\n");
	}
	
        if(MAJSTRLEN(thys->Family))
        {
	    ajFmtPrintF(outf,"XX\nSI   %d\n", thys->Sunid_Family);
	    ajFmtPrintF(outf, "XX\n");
	}
	
	if(MAJSTRLEN(thys->Typeobj))
	    ajFmtPrintF(outf, "%-5s%S\n", "TY", thys->Typeobj);
        ajFmtPrintF(outf, "XX\n");
        ajFmtPrintF(outf, "%-5s%.5f\n", "SC", thys->Score);
        ajFmtPrintF(outf, "XX\n");
        ajFmtPrintF(outf, "%-5s%.5f\n", "EV", thys->Eval);
        ajFmtPrintF(outf, "XX\n");
        
        if(MAJSTRLEN(thys->Group))
        {
            ajFmtPrintF(outf, "%-5s%S\n", "GP", thys->Group);
            ajFmtPrintF(outf, "XX\n");
        }

        ajFmtPrintF(outf, "%-5s%S\n", "AC", thys->Acc);
        ajFmtPrintF(outf, "XX\n");
        ajFmtPrintF(outf, "%-5s%d START; %d END;\n", "RA", thys->Start,
		    thys->End);
        ajFmtPrintF(outf, "XX\n");
        ajSeqWriteXyz(outf, thys->Seq, "SQ");
        ajFmtPrintF(outf, "XX\n");
    
        ajFmtPrintF(outf, "//\n");
    }

    ajListIterFree(&iter);
    

    return ajTrue;
}





/* @func ajDmxScopalgRead ***************************************************
**
** Read a Scopalg object from a file (see scopalign.c documentation).
** 
** @param [u] inf      [AjPFile] Input file stream
** @param [w] thys     [AjPScopalg*]  Scopalg object
**
** @return [AjBool] True if the file contained any data, even an empty 
** alignment.
** @@
****************************************************************************/

AjBool ajDmxScopalgRead(AjPFile inf, AjPScopalg *thys)
{
    static AjPStr line    = NULL;     /* Line of text */
    static AjPStr class   = NULL;
    static AjPStr fold    = NULL;
    static AjPStr super   = NULL;
    static AjPStr family  = NULL;
    static AjPStr postsim = NULL;   /* Post-similar line */
    static AjPStr posttmp = NULL;   /* Temp. storage for post-similar line */

    static AjPStr posisim = NULL;   /* Positions line */
    static AjPStr positmp = NULL;   /* Temp. storage for Positions line */
    
    AjBool done_1st_blk   = ajFalse; /* Flag for whether we've read first
					block of sequences */

    ajint x     = 0;              /* Loop counter */
    ajint y     = 0;              /* Loop counter */
    ajint cnt   = 0;              /* Temp. counter of sequence */
    ajint nseq  = 0;              /* No. of sequences in alignment */
    ajint Sunid = 0;              /* SCOP Sunid for family */
    
    
    AjPList list_seqs  = NULL;     /* List of sequences */
    AjPList list_codes = NULL;     /* List of codes */
    AjPStr  *arr_seqs  = NULL;     /* Array of sequences */
    AjPStr  seq        = NULL;     
    AjPStr  code       = NULL;     /* Id code of sequence */
    AjPStr  codetmp    = NULL;     /* Id code of sequence */
    AjPStr  seq1       = NULL;


    /* Check args */	
    if(!inf)
	return ajFalse;
    

    /* Allocate strings */
    /* Only initialise strings if this is called for the first time */
    if(!line)
    {
	class   = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	line    = ajStrNew();
	postsim = ajStrNew();
	posttmp = ajStrNew();
	posisim = ajStrNew();
	positmp = ajStrNew();
	seq1    = ajStrNew();
	codetmp = ajStrNew();
    }

    
    /* Create new lists */
    list_seqs  = ajListstrNew();
    list_codes = ajListstrNew();


    /* Read the rest of the file */
    while(ajFileReadLine(inf,&line))
    {
	/* Ignore 'Number' lines */
	if((ajStrPrefixC(line,"Number")))
	    continue;
    	else if(ajStrPrefixC(line,"SI"))
	{
	    ajFmtScanS(line, "%*s %d", &Sunid);
	}
    	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssC(&class,ajStrStr(line)+3);
	    ajStrClean(&class);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&fold,ajStrStr(line)+3);
	    while((ajFileReadLine(inf,&line)))
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
	    while((ajFileReadLine(inf,&line)))
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
	    while((ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"XX"))
	    continue;
	else if (ajStrPrefixC(line,"Post_similar"))
	{
	    /* Parse post_similar line */
	    ajFmtScanS(line, "%*s%S", &posttmp);
	    if(done_1st_blk == ajTrue)
		ajStrApp(&postsim, posttmp);
	    else
		ajStrAssS(&postsim, posttmp);
	    
	    continue;
	}
	else if (ajStrPrefixC(line,"Positions"))
	{
	    /* Parse Positions line */
	    ajFmtScanS(line, "%*s%S", &positmp);
	    if(done_1st_blk == ajTrue)
		ajStrApp(&posisim, positmp);
	    else
		ajStrAssS(&posisim, positmp);
	    
	    continue;
	}
	else if(ajStrChar(line,1)=='\0')
	{ 
	    /* If we are on a blank line */
	    /* ajFileReadLine will trim the tailing \n */

	    done_1st_blk=ajTrue;
	    y++;

	    /* The first blank line */
	    if(y == 1)
		ajListstrToArray(list_seqs, &arr_seqs);

	    cnt = 0;
	    continue;
	}
	else
	{
	    /* Parse a line of sequence */
	    if(done_1st_blk == ajTrue)
	    {
		/* already read in the first block of sequences */
		ajFmtScanS(line, "%*s%S", &seq1);
		ajStrApp(&arr_seqs[cnt], seq1);
		cnt++;
		continue;
	    }	
	    else
	    {
		/* It is a sequence line from the first block */
		nseq++;
		seq = ajStrNew();		
		code = ajStrNew();		
		ajFmtScanS(line, "%S%S", &code, &seq);
		
		/* Push strings onto lists */
		ajListstrPushApp(list_seqs,seq);
		ajListstrPushApp(list_codes,code);
		continue;
	    }
	}	
    }
    

    /*
    ** Cope for cases where alignment is in one block only, 
    ** i.e. there were no empty lines 
    **
    ** XX
    ** Number               10        20        30        40        50
    ** d1bsna1      QDLDEARAMEAKRKAEEHISSSHGDVDYAQASAELAKAIAQLRVIELTKKAM
    ** d1e79h1      DMLDLGAAKANLEKAQSELLGAADEATRAEIQIRIEANEALVKAL-------
    ** Post_similar 111111111111111111111111111111111111111111111-------
    */
    if(!done_1st_blk && nseq)
	ajListstrToArray(list_seqs, &arr_seqs);

    ajStrDel(&seq1);
    
    if(!nseq)
	ajWarn("No sequences in alignment !\n");

    /* Allocate memory for Scopalg structure */
    (*thys) = ajDmxScopalgNew(nseq);



    /* Assign SCOP records */
    ajStrAssS(&(*thys)->Class,class);
    ajStrAssS(&(*thys)->Fold,fold);
    ajStrAssS(&(*thys)->Superfamily,super);
    ajStrAssS(&(*thys)->Family,family); 
    (*thys)->Sunid_Family = Sunid;
    


    if(nseq)
    {
	/* Assign sequences and free memory */
	for(x=0; x<nseq; x++)
	{
	    ajStrAssS(&(*thys)->Seqs[x],arr_seqs[x]); 
	    ajStrDel(&arr_seqs[x]);

	}
	AJFREE(arr_seqs);
	
	
	/* Assign width */
	(*thys)->width = ajStrLen((*thys)->Seqs[0]);
	
	
	for(x=0; ajListstrPop(list_codes,&codetmp); x++)
	{
	    ajStrAssS(&(*thys)->Codes[x],codetmp);
	    ajStrDel(&codetmp);
	}
	
	
	/* Assign Post_similar line */
	ajStrAssS(&(*thys)->Post_similar,postsim); 

	/* Assign Positions line */
	ajStrAssS(&(*thys)->Positions,posisim); 
    }
    
    ajListstrDel(&list_seqs); 
    ajListstrDel(&list_codes); 
    
    return ajTrue;
}





/* @func ajDmxScopalgWrite **************************************************
**
** Write a Scopalg object to file in clusta format annotated with Scop 
** classification as below:
**
**
**
** CL   Alpha and beta proteins (a+b)
** XX
** FO   Phospholipase D/nuclease
** XX
** SF   Phospholipase D/nuclease
** XX
** FA   Phospholipase D
** XX
** SI   64391
** XX
** d1f0ia1      AATPHLDAVEQTLRQVSPGLEGDVWERTSGNKLDGSAADPSDWLLQTP-GCWGDDKC
** d1f0ia2      -----------------------------NVPV---------IAVG-GLG---VGIK
** 
** d1f0ia1      A-------------------------------D-RVGTKRLLAKMTENIGNATRTVD
** d1f0ia2      DVDPKSTFRPDLPTASDTKCVVGLHDNTNADRDYDTV-NPEESALRALVASAKGHIE
**
**
** 
** @param [r] scop [const AjPScopalg]  Scopalg object
** @param [u] outf [AjPFile]     Output file stream
**
** @return [AjBool] True on success (an alignment was written)
** @@
****************************************************************************/

AjBool ajDmxScopalgWrite(const AjPScopalg scop, AjPFile outf)
{
    /* Could modify scopalign.c to use this function now it is done */

    ajint x = 0;    
    ajint y = 0;    
    ajint tmp_wid  = 0;     /* Temp. variable for width */
    ajint code_wid = 0;     /* Max. code width +1 */
    ajint seq_wid  = 0;     /* Width of alignment rounded up to nearest 60 */
    ajint nblk     = 0;     /* Number of blocks of alignment in output */

    AjPStr tmp_seq = NULL;  /* Temp. variable for sequence */
    ajint start    = 0;     /* Start position of sequence fragment wrt full
				 length alignment */
    ajint     end     =0;     /* End position of sequence fragment wrt full
				 length alignment */
    

    /* Write SCOP classification records to file */
    ajFmtPrintF(outf,"CL   %S",scop->Class);
    ajFmtPrintSplit(outf,scop->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,scop->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,scop->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\n");
    ajFmtPrintF(outf,"SI   %d\nXX",scop->Sunid_Family);
    

    /* Find max. width of code, and add 1 to it for 1 whitespace */
    for(x=0;x<scop->N;x++)
	if( (tmp_wid=MAJSTRLEN(scop->Codes[x]))>code_wid)
	    code_wid = tmp_wid;
    code_wid++;
    

    /* Calculate no. of blocks in alignment */
    seq_wid = ajRound(scop->width, 60);
    nblk    = (ajint) (seq_wid / 60);
    
    
    /* Print out sequence in blocks */
    for(x=0;x<nblk;x++)
    {
	start = x*60;
	end   = start + 59;
	if(end>=scop->width)
	    end = scop->width - 1;
	
	ajFmtPrintF(outf, "\n");
	for(y=0; y<scop->N; y++)
	{
	    ajStrAssSub(&tmp_seq, scop->Seqs[y], start, end);
	    ajFmtPrintF(outf, "%*S%-60S\n", code_wid, scop->Codes[y], tmp_seq);
	}
    }
    
    return ajTrue;
}






/* @func ajDmxScopalgWriteClustal *******************************************
**
** Writes a Scopalg object to a specified file in CLUSTAL format (just the 
** alignment without the SCOP classification information).
**
** @param [r] align      [const AjPScopalg]  Scopalg object
** @param [u] outf       [AjPFile]   Outfile file pointer
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

AjBool ajDmxScopalgWriteClustal(const AjPScopalg align, AjPFile outf)
{
    ajint i;
    
    /* Check args */
    if(!align)
    {
	ajWarn("Null args passed to ajDmxScopalgWriteClustal");
	return ajFalse;
    }
    
    /* remove i from the print statement before commiting */
    ajFmtPrintF(outf,"CLUSTALW\n\n");
    ajFmtPrintF(outf, "\n"); 

    for(i=0;i<align->N;++i)
    	ajFmtPrintF(outf,"%S_%d   %S\n",align->Codes[i],i,align->Seqs[i]);
    ajFmtPrintF(outf,"\n");
    ajFmtPrintF(outf,"\n"); 
    
    return ajTrue;
}	





/* @func ajDmxScopalgWriteClustal2 ******************************************
**
** Writes a Scopalg object to a specified file in CLUSTAL format (just the 
** alignment without the SCOP classification information).
**
** @param [r] align      [const AjPScopalg]  Scopalg object.
** @param [u] outf       [AjPFile]   Outfile file pointer.
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/

AjBool ajDmxScopalgWriteClustal2(const AjPScopalg align, AjPFile outf)
{
    ajint i;
    
    /* Check args */
    if(!align)
    {
	ajWarn("Null args passed to ajDmxScopalgWriteClustal2");
	return ajFalse;
    }
    
    /* remove i from the print statement before commiting */
    ajFmtPrintF(outf, "\n"); 

    for(i=0;i<align->N;++i)
    	ajFmtPrintF(outf,"%S_%d   %S\n",align->Codes[i],i,align->Seqs[i]);
    ajFmtPrintF(outf,"\n");
    
    return ajTrue;
}	





/* @func ajDmxScopalgWriteFasta ********************************************
**
** Writes a Scopalg object to a specified file in FASTA format (just the 
** alignment without the SCOP classification information).
**
** @param [r] align      [const AjPScopalg]  A list Hitlist structures.
** @param [u] outf       [AjPFile]     Outfile file pointer
** 
** @return [AjBool] True on success (a file has been written)
** @@
****************************************************************************/
AjBool ajDmxScopalgWriteFasta(const AjPScopalg align, AjPFile outf)
{
    ajint i;
    
    /*Check args*/
    if(!align)
    {
	ajWarn("Null args passed to ajDmxScopalgWriteFasta");
	return ajFalse;
    }
    
    /* remove i from the print statement before commiting
    ajFmtPrintF(*outf,"CLUSTALW\n\n");
    ajFmtPrintF(*outf, "\n");*/ 

    for(i=0;i<align->N;++i)
    	ajFmtPrintF(outf,">%S_%d\n%S\n",align->Codes[i],i,align->Seqs[i]);
    ajFmtPrintF(outf,"\n");
    ajFmtPrintF(outf,"\n"); 
    
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

/* @func ajDmxScopSeqFromSunid **********************************************
**
** Writes a sequence corresponding to a Scop domain given a Sunid for the 
** domain. The sequence is taken from one of a list of Scop objects that is
** provided.  The swissprot sequence is taken in priority over the pdb 
** sequence.
** 
** @param [r] id   [ajint]    Search term
** @param [w] seq  [AjPStr*]  Result sequence
** @param [r] list [const AjPList]  Sorted list of Scop objects
**
** @return [AjBool]  True if a swissprot identifier code was found for the
**                   Pdb code.
** @@
****************************************************************************/

AjBool ajDmxScopSeqFromSunid(ajint id, AjPStr *seq, const AjPList list)
{
    AjPScop *arr = NULL;  /* Array derived from list */
    ajint dim =0;         /* Size of array */
    ajint idx =0;         /* Index into array for the Pdb code */

    if(!id || !list)
    {
        ajWarn("Bad args passed to ajDmxScopSeqFromSunid");
        return ajFalse;
    }
      
    dim = ajListToArray(list, (void ***) &(arr));
    if(!dim)
    {
        ajWarn("Empty list passed to ajDmxScopSeqFromSunid");
        return ajFalse;
    }

    if((idx = ajScopArrFindSunid(arr, dim, id))==-1)
    {
        AJFREE(arr);
        return ajFalse;
    }

    /* swissprot sequence has priority */
    if((ajStrLen(arr[idx]->SeqSpr))==0)
	ajStrAssS(seq, arr[idx]->SeqPdb);
    else
	ajStrAssS(seq, arr[idx]->SeqSpr);
    
    AJFREE(arr);

    return ajTrue;
}




/* @func ajDmxDummyFunction ***************************************************
**
** Dummy function to catch all unused functions defined in the ajdmx
** source file.
**
** @return [void]
**
******************************************************************************/

void ajDmxDummyFunction(void)
{
    return;
}





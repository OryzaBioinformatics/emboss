/****************************************************************************
**
** @source embdmx.c
** 
** @source Algorithms for some of the DOMAINATRIX EMBASSY applications. 
** For use with the Scophit and Scopalign objects.  
** The functionality will eventually be subsumed by other AJAX and NUCLEUS 
** libraries. 
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

#include "emboss.h"





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

/* @func embDmxScophitsToHitlist *********************************************
**
** Reads from a list of Scophit objects and writes a Hitlist object 
** with the next block of hits with identical SCOP classification. If the 
** iterator passed in is NULL it will read from the start of the list, 
** otherwise it will read from the current position. Memory for the Hitlist
** will be allocated if necessary and must be freed by the user.
** 
** @param [r] in      [const AjPList]     List of pointers to Scophit objects
** @param [w] out     [AjPHitlist*] Pointer to Hitlist object
** @param [u] iter    [AjIList*]    Pointer to iterator for list.
**
** @return [AjBool] True on success (lists were processed ok)
** @@
****************************************************************************/

AjBool embDmxScophitsToHitlist(const AjPList in,
			       AjPHitlist *out, AjIList *iter)
{
    AjPScophit scoptmp = NULL;        /* Temp. pointer to Scophit object */
    AjPHit tmp      = NULL;           /* Temp. pointer to Hit object */
    AjPList list    = NULL;           /* Temp. list of Hit objects */
    AjBool do_fam   = ajFalse;
    AjBool do_sfam  = ajFalse;
    AjBool do_fold  = ajFalse;
    AjBool do_class = ajFalse;
    AjPStr fam      = NULL;
    AjPStr sfam     = NULL;
    AjPStr fold     = NULL;
    AjPStr class    = NULL;
    ajint Sunid_Family = 0;
    
    /* Check args and allocate memory */
    if(!in || !iter)
    {
	ajWarn("NULL arg passed to embDmxScophitsToHitlist");
	return ajFalse;
    }


    /*
    ** If the iterator passed in is NULL it will read from the start of the 
    ** list, otherwise it will read from the current position.
    */
    if(!(*iter))
	*iter=ajListIterRead(in);


    if(!((scoptmp=(AjPScophit)ajListIterNext(*iter))))
    {
	ajWarn("Empty list in embDmxScophitsToHitlist");
	ajListIterFree(iter);	
	return ajFalse;
    }

    if(!(*out))
	*out = embHitlistNew(0);
    
    fam   = ajStrNew();
    sfam  = ajStrNew();
    fold  = ajStrNew();
    class = ajStrNew();

    list = ajListNew();

    Sunid_Family=scoptmp->Sunid_Family;
    
    
    if(scoptmp->Class)
    {
	do_class = ajTrue;
	ajStrAssS(&class, scoptmp->Class);
    }

    if(scoptmp->Fold)
    {
	do_fold= ajTrue;
	ajStrAssS(&fold, scoptmp->Fold);
    }

    if(scoptmp->Superfamily)
    {
	do_sfam = ajTrue;
	ajStrAssS(&sfam, scoptmp->Superfamily);
    }

    if(scoptmp->Family)
    {
	do_fam = ajTrue;
	ajStrAssS(&fam, scoptmp->Family);
    }

    embDmxScophitToHit(&tmp, scoptmp);
    ajListPush(list, (AjPHit) tmp);
    tmp = NULL;
        

    while((scoptmp=(AjPScophit)ajListIterNext(*iter)))
    {
	/*
	** The ajListIterBackNext(*iter); return the
	** iterator to the correct position for the 
	** next read
	*/
	if(do_class)
	    if(!ajStrMatch(scoptmp->Class, class))
	    {
		ajListIterBackNext(*iter);
		break;
	    }
	
	if(do_fold)
	    if(!ajStrMatch(scoptmp->Fold, fold))
	    {
		ajListIterBackNext(*iter);
		break;
	    }
	
	if(do_sfam)
	    if(!ajStrMatch(scoptmp->Superfamily, sfam))
	    {
		ajListIterBackNext(*iter);
		break;
	    }
	
	if(do_fam)
	    if(!ajStrMatch(scoptmp->Family, fam))
	    {
		ajListIterBackNext(*iter);
		break;
	    }
	
	
	embDmxScophitToHit(&tmp, scoptmp);
	ajListPush(list, (AjPHit) tmp);
	tmp = NULL;
    }
    ajStrAssS(&(*out)->Class, class);
    ajStrAssS(&(*out)->Fold, fold);
    ajStrAssS(&(*out)->Superfamily, sfam);
    ajStrAssS(&(*out)->Family, fam);
    (*out)->Sunid_Family = Sunid_Family;
    

    /* Copy temp. list to Hitlist */
    (*out)->N = ajListToArray(list, (void ***) &((*out)->hits));

    ajStrDel(&fam);
    ajStrDel(&sfam);
    ajStrDel(&fold);
    ajStrDel(&class);
    ajListDel(&list);	    

    return ajTrue;
}





/* @func embDmxScophitToHit *************************************************
**
** Copies the contents from a Scophit to a Hit object. Creates the Hit object
** if necessary.
**
** @param [w] to   [AjPHit*] Hit object pointer 
** @param [r] from [const AjPScophit] Scophit object 
**
** @return [AjBool] True if copy was successful.
** @@
****************************************************************************/

AjBool embDmxScophitToHit(AjPHit *to, const AjPScophit from)
{
    if(!from)
    {
	ajWarn("NULL arg passed to embDmxScophitToHit");
	return ajFalse;
    }
    
    if(!(*to))
	*to = embHitNew();

    ajStrAssS(&(*to)->Seq, from->Seq);
    (*to)->Start = from->Start;
    (*to)->End   = from->End;
    ajStrAssS(&(*to)->Acc, from->Acc);
    ajStrAssS(&(*to)->Spr, from->Spr);
    ajStrAssS(&(*to)->Typeobj, from->Typeobj);
    ajStrAssS(&(*to)->Typesbj, from->Typesbj);
    ajStrAssS(&(*to)->Model, from->Model);
    ajStrAssS(&(*to)->Alg, from->Alg);
    ajStrAssS(&(*to)->Group, from->Group);
    (*to)->Rank  = from->Rank;
    (*to)->Score = from->Score;
    (*to)->Eval  = from->Eval;
    (*to)->Pval  = from->Pval;
    (*to)->Target   = from->Target;
    (*to)->Target2  = from->Target2;
    (*to)->Priority = from->Priority;

    return ajTrue;
}





/* @func embDmxScophitsAccToHitlist *****************************************
**
** Reads from a list of Scophit objects and writes a Hitlist object 
** with the next block of hits with identical SCOP classification. A Hit is 
** only written to the Hitlist if an accession number is given.  Also, only 
** one of any pair of duplicate hits (overlapping hits with
** identical accession) 
** will be written to the Hitlist. An 'overlap' is defined as a shared region 
** of 10 or more residues.
** To check for these the list is first be sorted by Accession number.
** 
** If the iterator passed in is NULL it will read from the start of the list, 
** otherwise it will read from the current position. Memory for the Hitlist
** will be allocated if necessary and must be freed by the user.
** 
** @param [r] in      [const AjPList]     List of pointers to Scophit objects
** @param [w] out     [AjPHitlist*] Pointer to Hitlist object
** @param [u] iter    [AjIList*]    Pointer to iterator for list.
**
** @return [AjBool] True on success (lists were processed ok)
** @@
****************************************************************************/

AjBool embDmxScophitsAccToHitlist(const AjPList in,
				  AjPHitlist *out, AjIList *iter)
{
    AjPScophit scoptmp = NULL;        /* Temp. pointer to Scophit object */

    AjPHit tmp   = NULL;            /* Temp. pointer to Hit object */
    AjPList list = NULL;           /* Temp. list of Hit objects */

    AjBool do_fam   = ajFalse;
    AjBool do_sfam  = ajFalse;
    AjBool do_fold  = ajFalse;
    AjBool do_class = ajFalse;

    AjPStr fam   = NULL;
    AjPStr sfam  = NULL;
    AjPStr fold  = NULL;
    AjPStr class = NULL;
    ajint Sunid_Family = 0;
    
    /* Check args and allocate memory */
    if(!in || !iter)
    {
	ajWarn("NULL arg passed to embDmxScophitsAccToHitlist");
	return ajFalse;
    }


    /*
    ** If the iterator passed in is NULL it will read from the start of the 
    ** list, otherwise it will read from the current position.
    */
    if(!(*iter))
	*iter=ajListIterRead(in);


    if(!((scoptmp=(AjPScophit)ajListIterNext(*iter))))
    {
	ajWarn("Empty list in embDmxScophitsToHitlist");
	ajListIterFree(iter);	
	return ajFalse;
    }

    /*
    ** Find the first Scophit which has an accession number 
    ** if necessary
    */
    if((ajStrMatchC(scoptmp->Acc,"Not_available")) ||
       (MAJSTRLEN(scoptmp->Acc)==0))
    {
	while((scoptmp=(AjPScophit)ajListIterNext(*iter)))
	{
	    if((ajStrMatchC(scoptmp->Acc,"Not_available") == ajFalse) &&
	       (MAJSTRLEN(scoptmp->Acc)!=0))
		break;
	}
	if(!scoptmp)
	{
	    ajWarn("List with no Scophits with Acc in "
		   "embDmxScophitsAccToHitlist");
	    ajListIterFree(iter);	
	    return ajFalse;
	}
    }
    

    if(!(*out))
	*out = embHitlistNew(0);
    
    fam   = ajStrNew();
    sfam  = ajStrNew();
    fold  = ajStrNew();
    class = ajStrNew();

    list = ajListNew();

    
    Sunid_Family=scoptmp->Sunid_Family;
    
    
    if(scoptmp->Class)
    {
	do_class = ajTrue;
	ajStrAssS(&class, scoptmp->Class);
    }

    if(scoptmp->Fold)
    {
	do_fold= ajTrue;
	ajStrAssS(&fold, scoptmp->Fold);
    }

    if(scoptmp->Superfamily)
    {
	do_sfam = ajTrue;
	ajStrAssS(&sfam, scoptmp->Superfamily);
    }

    if(scoptmp->Family)
    {
	do_fam = ajTrue;
	ajStrAssS(&fam, scoptmp->Family);
    }

    /* Only want to push the hit if it is not targetted */
    if(!scoptmp->Target2)
    {
	embDmxScophitToHit(&tmp, scoptmp);
	ajListPush(list, (AjPHit) tmp);
	tmp = NULL;
    }
    
        

    while((scoptmp=(AjPScophit)ajListIterNext(*iter)))
    {
	if(do_class)
	    if(!ajStrMatch(scoptmp->Class, class))
		break;

	if(do_fold)
	    if(!ajStrMatch(scoptmp->Fold, fold))
		break;

	if(do_sfam)
	    if(!ajStrMatch(scoptmp->Superfamily, sfam))
		break;

	if(do_fam)
	    if(!ajStrMatch(scoptmp->Family, fam))
		break;
	
	if((ajStrMatchC(scoptmp->Acc,"Not_available")) ||
	   (MAJSTRLEN(scoptmp->Acc)==0))
	    continue;

	if(scoptmp->Target2)
	    continue;
		
	embDmxScophitToHit(&tmp, scoptmp);
	ajListPush(list, (AjPHit) tmp);
	tmp=NULL;

	continue;
    }

    ajStrAssS(&(*out)->Class, class);
    ajStrAssS(&(*out)->Fold, fold);
    ajStrAssS(&(*out)->Superfamily, sfam);
    ajStrAssS(&(*out)->Family, fam);
    (*out)->Sunid_Family = Sunid_Family;
    

    /* Copy temp. list to Hitlist */
    (*out)->N = ajListToArray(list, (void ***) &((*out)->hits));

    /* Tidy up and return */
    ajStrDel(&fam);
    ajStrDel(&sfam);
    ajStrDel(&fold);
    ajStrDel(&class);
    ajListDel(&list);	    

    return ajTrue;
}





/* @func embDmxHitsWrite ****************************************************
 ** Writes a list of AjOHit objects to an output file. This is intended for 
 ** displaying the results from scans of a model against a protein sequence
 ** database. Output in a sigplot compatible format.
 **
 ** @param [u] outf    [AjPFile]     Output file stream
 ** @param [r] hits    [const AjPHitlist]  Hitlist objects with hits from scan
 ** @param [r] maxhits [ajint]       Max. hits to write.
 **
 ** @return [AjBool] True if file was written
 ** @@
 ***************************************************************************/
AjBool embDmxHitsWrite(AjPFile outf, const AjPHitlist hits, ajint maxhits)
{
    ajint  x  = 0;
    ajint cnt = 0;
    
    AjPList tmplist = NULL;
    AjPList outlist = NULL; /* rank-ordered list of hits for output */
    AjIList iter    = NULL;
    AjPScophit hit  = NULL;
    
    

    /*Check args*/
    if(!outf || !hits)
        return ajFalse;

    tmplist=ajListNew();
    outlist=ajListNew();
    
    
    /* Push hits onto tmplist */
    ajListPushApp(tmplist, hits);
    embDmxHitlistToScophits(tmplist, &outlist);
    ajListSort(outlist, ajDmxScophitCompPval);
    

    /*Print header info*/
    ajFmtPrintF(outf, "DE   Results of %S search\nXX\n",hits->Model);

    /*Print SCOP classification records of signature */
    ajFmtPrintF(outf,"CL   %S",hits->Class);
    ajFmtPrintSplit(outf,hits->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,hits->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,hits->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nSI   %d\n", hits->Sunid_Family);
    ajFmtPrintF(outf,"XX\n");
    

    iter=ajListIterRead(outlist);
    while((hit=(AjPScophit) ajListIterNext(iter)))
    {
	if(cnt==maxhits)
	    break;
	
	ajFmtPrintF(outf,"HI  %-6d%-10S%-5d%-5d%-15S%-10S%-10S%-10.2f"
		    "%-7.4f%-7.4f\n",
                    x+1, hit->Acc, 
                    hit->Start+1, hit->End+1,
                    hit->Group, 
                    hit->Typeobj, hit->Typesbj, 
                    hit->Score, hit->Pval, hit->Eval);
	ajDmxScophitDel(&hit);

/*CORRECTION*/	if(ajStrMatchC(hit->Typeobj, "FALSE"))
	    cnt++;
	x++;
    }
    
    ajListIterFree(&iter);
    ajListDel(&outlist);
    ajListDel(&tmplist);
    
    /*Print tail info*/
    ajFmtPrintF(outf, "XX\n//\n");   
    
    /*Clean up and return*/ 
    return ajTrue;
}





/* @func embDmxScopToScophit ************************************************
**
** Writes a Scophit structure with the common information in a Scop
** structure. The swissprot sequence is taken in preference to the pdb 
** sequence.
**
** @param [r] source  [const AjPScop]       The Scop object to convert
** @param [w] target  [AjPScophit*]   Destination of the the scophit 
**                                    structure to write to. 
**
** @return [AjBool] ajTrue on the success of creating a Scophit structure. 
** @@
****************************************************************************/

AjBool embDmxScopToScophit(const AjPScop source, AjPScophit* target)
{

    if(!source || !target)
    {
	ajWarn("bad args passed to embDmxScopToScophit\n");
	return ajFalse;
    }
    

    ajStrAssS(&(*target)->Class,source->Class);
    ajStrAssS(&(*target)->Fold,source->Fold);
    ajStrAssS(&(*target)->Superfamily,source->Superfamily);
    ajStrAssS(&(*target)->Family,source->Family);
    (*target)->Sunid_Family = source->Sunid_Family;
    
    /* The swissprot sequence was not available */
    if(ajStrLen(source->SeqSpr)==0)
    {
	ajStrAssS(&(*target)->Seq,source->SeqPdb);
	(*target)->Start = 0;
	(*target)->End   = 0;
	ajStrAssC(&(*target)->Acc,"Not_available");
	ajStrAssC(&(*target)->Spr,"Not_available");
    }
    else
    {
	ajStrAssS(&(*target)->Seq,source->SeqSpr);
	(*target)->Start = source->Startd;
	(*target)->End   = source->Endd;
	ajStrAssS(&(*target)->Acc,source->Acc);
	ajStrAssS(&(*target)->Spr,source->Spr);
    }
    
    return ajTrue;
}





/* @func embDmxScopalgToScop ************************************************
**
** Takes a Scopalg object (scop alignment) and an array of Scop objects
** taken from, e.g. a scop classification file.
** Extracts the scop domain codes from the alignment and compiles a list of 
** corresponding Scop objects from the scop classification file.
**
** @param [r] align     [const AjPScopalg]  Contains a seed alignment.
** @param [r] scop_arr  [AjPScop const *]    Array of AjPScop objects
** @param [r] scop_dim  [ajint]       Size of array
** @param [w] list      [AjPList*]    List of Scop objects.
** 
** @return [AjBool] A populated list has been returned
**                  (a file has been written)
** @@
****************************************************************************/

AjBool embDmxScopalgToScop(const AjPScopalg align, AjPScop const *scop_arr, 
			   ajint scop_dim, AjPList* list)
{
    AjPStr entry_up = NULL;  /* Current entry, upper case */
    ajint idx = 0;           /* Index into array for the Pdb code */
    ajint i   = 0;           /* Simple loop counter */


    entry_up  = ajStrNew();
    

    /* check for bad arguments */
    if(!align)
    {
        ajWarn("Bad args passed to embDmxScopalgToScop");
	ajStrDel(&entry_up);
        return ajFalse;
    }



    /*
    ** write to list the scop structures matching a particular
    ** family of domains
    */
    for(i=0;i<align->N;i++)
    {
	ajStrAssS(&entry_up, align->Codes[i]);
	ajStrToUpper(&entry_up);
	
	
        if((idx = ajScopArrFindScopid(scop_arr,scop_dim,entry_up))==-1)
        {
	    ajStrDel(&entry_up);
	    return ajFalse;
	}
	else
	{
	    /* DIAGNOSTICS
	    ajFmtPrint("Pushing %d (%S)\n", scop_arr[idx]->Sunid_Family, 
		       scop_arr[idx]->Acc); */
	    
            ajListPushApp(*list,scop_arr[idx]);
	}
	
    }
    

    ajStrDel(&entry_up);

    return ajTrue;
}





/* @func embDmxScophitsOverlapAcc *******************************************
**
** Checks for overlap and identical accession numbers between two hits.
**
** @param [r] h1  [const AjPScophit]  Pointer to hit object 1
** @param [r] h2  [const AjPScophit]  Pointer to hit object 2
** @param [r] n   [ajint]       Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
****************************************************************************/

AjBool embDmxScophitsOverlapAcc(const AjPScophit h1, const AjPScophit h2,
				ajint n)
{
    if((MAJSTRLEN(h1->Seq)<n) || (MAJSTRLEN(h2->Seq)<n))
    {
	ajWarn("Sequence length smaller than overlap limit in "
	       "embDmxScophitsOverlapAcc ... checking for string "
	       "match instead");

	if(((ajStrFind(h1->Seq, h2->Seq)!=-1) ||
	    (ajStrFind(h2->Seq, h1->Seq)!=-1)) &&
	   (ajStrMatch(h1->Acc, h2->Acc)))
	    return ajTrue;
	else
	    return ajFalse;
    }

    if( ((((h1->End - h2->Start + 1)>=n) && (h2->Start >= h1->Start)) ||
	 (((h2->End - h1->Start + 1)>=n) && (h1->Start >= h2->Start)))  &&
       (ajStrMatch(h1->Acc, h2->Acc)))
	return ajTrue;

    return ajFalse;
}





/* @func embDmxScophitsOverlap **********************************************
**
** Checks for overlap between two hits.
**
** @param [r] h1  [const AjPScophit]     Pointer to hit object 1
** @param [r] h2  [const AjPScophit]     Pointer to hit object 2
** @param [r] n   [ajint]          Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
****************************************************************************/

AjBool embDmxScophitsOverlap(const AjPScophit h1, const AjPScophit h2, ajint n)
{
    if((MAJSTRLEN(h1->Seq)<n) || (MAJSTRLEN(h2->Seq)<n))
    {
	ajWarn("Sequence length smaller than overlap limit in "
	       "embDmxScophitsOverlap ... checking for string match instead");

	if((ajStrFind(h1->Seq, h2->Seq)!=-1) ||
	   (ajStrFind(h2->Seq, h1->Seq)!=-1))
	    return ajTrue;
	else
	    return ajFalse;
    }
    
    if( (((h1->End - h2->Start + 1)>=n) && (h2->Start >= h1->Start)) ||
       (((h2->End - h1->Start + 1)>=n) && (h1->Start >= h2->Start)))
	return ajTrue;
    else 
	return ajFalse;
}





/* @func embDmxScophitMerge *************************************************
**
** Creates a new Scophit object which corresponds to a merging of the two 
** sequences from the Scophit objects hit1 and hit2. 
**
** The Typeobj of the merged hit is set.  The merged hit is classified 
** as follows :
** If hit1 or hit2 is a SEED, the merged hit is classified as a SEED.
** Otherwise, if hit1 or hit2 is HIT, the merged hit is clsasified as a HIT.
** If hit1 and hit2 are both OTHER, the merged hit remains classified as 
** OTHER.
** 
** @param [r] hit1     [const AjPScophit]  Scophit 1
** @param [r] hit2     [const AjPScophit]  Scophit 2
**
** @return [AjPScophit] Pointer to Scophit object.
** @@
****************************************************************************/

AjPScophit embDmxScophitMerge(const AjPScophit hit1, const AjPScophit hit2)
{
    AjPScophit ret;
    ajint start = 0;    /* Start of N-terminal-most sequence */
    ajint end   = 0;    /* End of N-terminal-most sequence */
    AjPStr temp = NULL;
    

    /* Check args */
    if(!hit1 || !hit2)
    {
	ajWarn("Bad arg's passed to AjPScophitMerge");
	return NULL;
    }

    if(!ajStrMatch(hit1->Acc, hit2->Acc))
    {
	ajWarn("Merge attempted on 2 hits with different accession numbers");
	return NULL;
    }

    /* Allocate memory */
    ret = ajDmxScophitNew();
    temp = ajStrNew();
    
    ajStrAssS(&(ret->Acc), hit1->Acc);
    ajStrAssS(&(ret->Spr), hit1->Spr);
        
    if(ajStrMatch(hit1->Class, hit2->Class))
    {
	ajStrAssS(&(ret->Class), hit1->Class);
	if(ajStrMatch(hit1->Fold, hit2->Fold))
	{
	    ajStrAssS(&(ret->Fold), hit1->Fold);
	    if(ajStrMatch(hit1->Superfamily, hit2->Superfamily))
	    {
		ajStrAssS(&(ret->Superfamily), hit1->Superfamily);
		if(ajStrMatch(hit1->Family, hit2->Family))
		    ajStrAssS(&(ret->Family), hit1->Family);
	    }
	}
    }
    

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


    /*
    ** Assign the C-terminus of the sequence of the new hit     
    ** if necessary
    */
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
    

    if(hit1->Sunid_Family == hit2->Sunid_Family)
	ret->Sunid_Family = hit1->Sunid_Family;
    
    /* All other elements of structure are left as NULL / o / ajFalse */
        
    ajStrDel(&temp);

    return ret;
}





/* @func embDmxScophitMergeInsertOther **************************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Appends the new Scophit onto a list. Target
** hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [u] list   [AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertOther(AjPList list, AjPScophit hit1,
				    AjPScophit hit2)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListPushApp(list, (void *) new);
    
    return ajTrue;
}





/* @func embDmxScophitMergeInsertOtherTarget ********************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Appends the new Scophit onto a list. Target
** hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [u] list   [AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertOtherTarget(AjPList list, AjPScophit hit1,
					   AjPScophit hit2)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListPushApp(list, (void *) new);
    
    return ajTrue;
}





/* @func embDmxScophitMergeInsertOtherTargetBoth ****************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Appends the new Scophit onto a list. Target
** hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [u] list   [AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertOtherTargetBoth(AjPList list, AjPScophit hit1,
					      AjPScophit hit2)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajDmxScophitTarget2(&new);
    ajDmxScophitTarget2(&hit1);
    ajDmxScophitTarget2(&hit2);
    ajListPushApp(list, (void *) new);
    
    return ajTrue;
}





/* @func embDmxScophitMergeInsertThis ***************************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Insert the new Scophit immediately after hit2. 
** Target hit1 and hit2 for removal (set the Target element to ajTrue).
** 
** @param [r] list   [const AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
** @param [u] iter   [AjIList]     List iterator
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertThis(const AjPList list, AjPScophit hit1, 
				   AjPScophit hit2,  AjIList iter)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list || !iter)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListInsert(iter, (void *) new);
    
    return ajTrue;
}





/* @func embDmxScophitMergeInsertThisTarget *********************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Insert the new Scophit immediately after hit2. 
** Target hit1, hit2 and the new Scophit for removal
** (set the Target element to ajTrue).
** 
** @param [r] list   [const AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
** @param [u] iter   [AjIList]     List iterator
**
** @return [AjBool] True on success.
** @@
****************************************************************************/

AjBool embDmxScophitMergeInsertThisTarget(const AjPList list,
					  AjPScophit hit1, 
					  AjPScophit hit2,  AjIList iter)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list || !iter)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajListInsert(iter, (void *) new);
    
    return ajTrue;
}





/* @func embDmxScophitMergeInsertThisTargetBoth  ****************************
**
** Creates a new Scophit object which corresponds to a merging of two Scophit
** objects hit1 and hit2. Insert the new Scophit immediately after hit2. 
** Target hit1, hit2 and the new Scophit for removal (both the Target and 
** Target2 elements are set to ajTrue).
** 
** 
** @param [r] list   [const AjPList]     List of Scophit objects
** @param [d] hit1   [AjPScophit]  Scophit object 1
** @param [d] hit2   [AjPScophit]  Scophit object 2
** @param [u] iter   [AjIList]     List iterator
**
** @return [AjBool] True on success.
** @@
****************************************************************************/
AjBool embDmxScophitMergeInsertThisTargetBoth(const AjPList list,
					      AjPScophit hit1, 
					      AjPScophit hit2,
					      AjIList iter)
{
    AjPScophit new;

    /* Check args */
    if(!hit1 || !hit2 || !list || !iter)
	return ajFalse;
    

    new = embDmxScophitMerge(hit1, hit2);
    ajDmxScophitTarget(&new);
    ajDmxScophitTarget(&hit1);
    ajDmxScophitTarget(&hit2);
    ajDmxScophitTarget2(&new);
    ajDmxScophitTarget2(&hit1);
    ajDmxScophitTarget2(&hit2);
    ajListInsert(iter, (void *) new);
    
    return ajTrue;
}





/* @func embDmxSeqNR ********************************************************
**
** Reads a list of AjPSeq's and writes an array describing the redundancy in
** the list. Elements in the array correspond to sequences in the list, i.e.
** the array[0] corresponds to the first sequence in the list.
**
** Sequences are classed as redundant (0 in the array, i.e. they are possibly
** to be discarded later) if they exceed a threshold (%) level of sequence
** similarity to any other in the set (the shortest sequence of the current
** pair will be discarded).
** 
** The set output will always contain at least 1 sequence.
** 
** @param [r] input  [const AjPList]    List of ajPSeq's 
** @param [w] keep   [AjPInt*]    0: rejected (redundant) sequence, 1: the 
                                  sequence was retained
** @param [w] nset   [ajint*]     Number of sequences in nr set (no. of 1's 
**                                in the keep array)
** @param [r] matrix    [const AjPMatrixf] Residue substitution matrix
** @param [r] gapopen   [float]      Gap insertion penalty
** @param [r] gapextend [float]      Gap extension penalty
** @param [r] thresh    [float]      Threshold residue id. for "redundancy"
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/

AjBool embDmxSeqNR(const AjPList input, AjPInt *keep, ajint *nset,
		   const AjPMatrixf matrix, float gapopen, float gapextend,
		   float thresh)
{
    ajint start1  = 0;	  /* Start of seq 1, passed as arg but not used */
    ajint start2  = 0;	  /* Start of seq 2, passed as arg but not used */
    ajint maxarr  = 300;  /* Initial size for matrix */
    ajint len;
    ajint x;		  /* Counter for seq 1 */
    ajint y;		  /* Counter for seq 2 */
    ajint nin;		  /* Number of sequences in input list */
    ajint *compass;

    const char  *p;
    const char  *q;

    AjFloatArray *sub;
    float id   = 0.;	  /* Passed as arg but not used here */
    float sim  = 0.;
    float idx  = 0.;	  /* Passed as arg but not used here */
    float simx = 0.;	  /* Passed as arg but not used here */
    float *path;

    AjPStr m = NULL;	  /* Passed as arg but not used here */
    AjPStr n = NULL;	  /* Passed as arg but not used here */

    AjPSeq      *inseqs = NULL;	 /* Array containing input sequences */
    AjPInt      lens    = NULL;	 /* 1: Lengths of sequences* in input list */
    AjPFloat2d  scores  = NULL;
    AjPSeqCvt   cvt     = 0;
    AjBool      show    = ajFalse; /* Passed as arg but not used here */


    /* Intitialise some variables */
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    m = ajStrNew();
    n = ajStrNew();
    gapopen   = ajRoundF(gapopen,8);
    gapextend = ajRoundF(gapextend,8);
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);

    /* Convert the AjPList to an array of AjPseq */
    if(!(nin=ajListToArray(input,(void ***)&inseqs)))
    {
	ajWarn("Zero sized list of sequences passed into SeqsetNR");
	AJFREE(compass);
	AJFREE(path);
	ajStrDel(&m);
	ajStrDel(&n);
	return ajFalse;
    }


    /* Create an ajint array to hold lengths of sequences */
    lens = ajIntNewL(nin);
    for(x=0; x<nin; x++)
	ajIntPut(&lens,x,ajSeqLen(inseqs[x]));


    /* Set the keep array elements to 1 */
    for(x=0;x<nin;x++)
	ajIntPut(keep,x,1);


    /* Create a 2d float array to hold the similarity scores */
    scores = ajFloat2dNew();

    /* Start of main application loop */
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)
	{
	    /* DIAGNOSTICS
	       ajFmtPrint("x=%d y=%d\nComparing\n%S\nto\n%S\n\n", 
		       x, y, inseqs[x]->Seq, inseqs[y]->Seq);
	    */
	    

	    /* Process w/o alignment identical sequences */
	    if(ajStrMatch(inseqs[x]->Seq, inseqs[y]->Seq))
	    {
/* DIAGNOSTICS		printf("Score=%f\n", 100.0); */
		
		ajFloat2dPut(&scores,x,y,(float)100.0);
		continue;
	    }


	    /* Intitialise variables for use by alignment functions */
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
	/* DIAGNOSTICS	    printf("Score=%f\n", sim); */
		    ajFloat2dPut(&scores,x,y,sim);
	}
    }


    /* DIAGNOSTIC 
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)	
	{
	    ajFmtPrint("%d:%d : %f\n", x+1, y+1, ajFloat2dGet(scores,x,y));
        }
    }
    */
    


    /* Write the keep array as appropriate */
    for(x=0; x<nin; x++)
    {
	if(!ajIntGet(*keep,x))
	    continue;

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

    for(x=0; x<nin; x++)
    	if(ajIntGet(*keep,x))
	    (*nset)++;

    /* Keep first sequence in case all have been processed out */
    if(*nset == 0)
    {
	ajIntPut(keep,0,1);
	*nset = 1;
    }
    

    AJFREE(compass);
    AJFREE(path);
    ajStrDel(&m);
    ajStrDel(&n);
    ajFloat2dDel(&scores);
    ajIntDel(&lens);
    AJFREE(inseqs);

    return ajTrue;
}





/* @func embDmxSeqNRRange****************************************************
**
** Reads a list of AjPSeq's and writes an array describing the redundancy in
** the list. Elements in the array correspond to sequences in the list, i.e.
** the array[0] corresponds to the first sequence in the list.
**
** Sequences are classed as redundant (0 in the array, i.e. they are possibly
** to be discarded later) if they lie outside a range of threshold (%) 
** sequence similarity to others in the set (the shortest sequence of the 
** current pair will be discarded).
**
** @param [r] input  [const AjPList]    List of ajPSeq's
** @param [w] keep   [AjPInt*]    0: rejected (redundant) sequence, 1: the
                                  sequence was retained
** @param [w] nset   [ajint*]     Number of sequences in nr set (no. of 1's
**                                in the keep array)
** @param [r] matrix    [const AjPMatrixf] Residue substitution matrix
** @param [r] gapopen   [float]      Gap insertion penalty
** @param [r] gapextend [float]      Gap extension penalty
** @param [r] thresh1    [float]    Threshold lower limit
** @param [r] thresh2    [float]    Threshold upper limit
**
** @return [AjBool] ajTrue on success
** @@
****************************************************************************/
AjBool embDmxSeqNRRange(const AjPList input, AjPInt *keep, ajint *nset,
			const AjPMatrixf matrix,
			float gapopen, float gapextend,
			float thresh1, float thresh2)
{
    ajint start1 = 0;	/* Start of seq 1, passed as arg but not used */
    ajint start2 = 0;	/* Start of seq 2, passed as arg but not used */
    ajint maxarr = 300;	/* Initial size for matrix */
    ajint len;
    ajint x;		/* Counter for seq 1 */
    ajint y;		/* Counter for seq 2 */
    ajint nin;		/* Number of sequences in input list */
    ajint *compass;

    const char  *p;
    const char  *q;

    float **sub;
    float id   = 0.;	/* Passed as arg but not used here */
    float sim  = 0.;
    float idx  = 0.;	/* Passed as arg but not used here */
    float simx = 0.;	/* Passed as arg but not used here */
    float *path;

    AjPStr m = NULL;	/* Passed as arg but not used here */
    AjPStr n = NULL;	/* Passed as arg but not used here */

    AjPSeq *inseqs = NULL;	/* Array containing input sequences */
    AjPInt lens    = NULL;	/* 1: Lengths of sequences* in input list */
    AjPFloat2d  scores = NULL;
    AjPSeqCvt cvt = 0;
    AjBool show = ajFalse;	/* Passed as arg but not used here */
    AjBool ok   = ajFalse;      /* True if the current sequence has
					  at least thresh1 % similarity to
					  at least one other sequence in the
					  set */


    /* Intitialise some variables */
    AJCNEW(path, maxarr);
    AJCNEW(compass, maxarr);
    m = ajStrNew();
    n = ajStrNew();
    gapopen   = ajRoundF(gapopen,8);
    gapextend = ajRoundF(gapextend,8);
    sub = ajMatrixfArray(matrix);
    cvt = ajMatrixfCvt(matrix);


    /* Convert the AjPList to an array of AjPseq */
    if(!(nin=ajListToArray(input,(void ***)&inseqs)))
    {
	ajWarn("Zero sized list of sequences passed into SeqsetNR");
	AJFREE(compass);
	AJFREE(path);
	ajStrDel(&m);
	ajStrDel(&n);
	return ajFalse;
    }


    /* Create an ajint array to hold lengths of sequences */
    lens = ajIntNewL(nin);
    for(x=0; x<nin; x++)
	ajIntPut(&lens,x,ajSeqLen(inseqs[x]));


    /* Set the keep array elements to 1 */
    for(x=0;x<nin;x++)
	ajIntPut(keep,x,1);


    /* Create a 2d float array to hold the similarity scores */
    scores = ajFloat2dNew();


    /* Start of main application loop */
    for(x=0; x<nin; x++)
    {
	for(y=x+1; y<nin; y++)
	{
	    /* Process w/o alignment identical sequences */
	    if(ajStrMatch(inseqs[x]->Seq, inseqs[y]->Seq))
	    {
		ajFloat2dPut(&scores,x,y,(float)100.0);
		continue;
	    }


	    /* Intitialise variables for use by alignment functions */
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


	    /* Write array with score */
	    ajFloat2dPut(&scores,x,y,sim);
	}
    }


    /* Write the keep array as appropriate, first check the upper limit */
    for(x=0; x<nin; x++)
    {
	if(!ajIntGet(*keep,x))
	    continue;

	for(y=x+1; y<nin; y++)
	{
	    if(!ajIntGet(*keep,y))
		continue;

	    if(ajFloat2dGet(scores,x,y) >= thresh2)
	    {
		if(ajIntGet(lens,x) < ajIntGet(lens,y))
		    ajIntPut(keep,x,0);

		else
		    ajIntPut(keep,y,0);
	    }
	}
    }


    /* Now check the lower limit */
    for(x=0; x<nin; x++)
    {
	if(!ajIntGet(*keep,x))
	    continue;

	ok = ajFalse;

	for(y=x+1; y<nin; y++)
	{
	    if(!ajIntGet(*keep,y))
		continue;

	    if(ajFloat2dGet(scores,x,y) >= thresh1)
	    {
		ok = ajTrue;
		break;
	    }
	}

	if(!ok)
	    ajIntPut(keep,x,0);
    }


    for(x=0; x<nin; x++)
    	if(ajIntGet(*keep,x))
	    (*nset)++;



    /* Keep first sequence in case all have been processed out */
    if(*nset == 0)
    {
	ajIntPut(keep,0,1);
	*nset = 1;
    }

    /* Tidy up */
    AJFREE(compass);
    AJFREE(path);
    ajStrDel(&m);
    ajStrDel(&n);
    ajFloat2dDel(&scores);
    ajIntDel(&lens);
    AJFREE(inseqs);

    return ajTrue;
}





/* @func embDmxHitlistToScophits ********************************************
**
** Read from a list of Hitlist structures and writes a list of Scophit 
** structures.
** 
** @param [r] in      [const AjPList]  List of pointers to Hitlist structures
** @param [w] out     [AjPList*] Pointer to list of Scophit structures
**
** @return [AjBool] True on success (lists were processed ok)
** @@
****************************************************************************/

AjBool embDmxHitlistToScophits(const AjPList in, AjPList *out)
{
    AjPScophit scophit = NULL;   /* Pointer to Scophit object */
    AjPHitlist hitlist = NULL;   /* Pointer to Hitlist object */
    AjIList iter = NULL;         /* List iterator */
    ajint x      = 0;            /* Loop counter */


    /* Check args */
    if(!in)
    {
	ajWarn("Null arg passed to embDmxHitlistToScophits");
	return ajFalse;
    }

    /* Create list iterator and new list */
    iter = ajListIterRead(in);	
    

    /* Iterate through the list of Hitlist pointers */
    while((hitlist=(AjPHitlist)ajListIterNext(iter)))
    {
	/* Loop for each hit in hitlist structure */
	for(x=0; x<hitlist->N; ++x)
	{
	    /* Create a new scophit structure */
	    scophit = ajDmxScophitNew();
	    

	    /* Assign scop classification records from hitlist structure */
	    ajStrAssS(&scophit->Class, hitlist->Class);
	    ajStrAssS(&scophit->Fold, hitlist->Fold);
	    ajStrAssS(&scophit->Superfamily, hitlist->Superfamily);
	    ajStrAssS(&scophit->Family, hitlist->Family);
	    scophit->Sunid_Family = hitlist->Sunid_Family;
	    scophit->Priority = hitlist->Priority;
	    
	    /* Assign records from hit structure */
	    ajStrAssS(&scophit->Seq, hitlist->hits[x]->Seq);
	    ajStrAssS(&scophit->Acc, hitlist->hits[x]->Acc);
	    ajStrAssS(&scophit->Spr, hitlist->hits[x]->Spr);
	    ajStrAssS(&scophit->Typeobj, hitlist->hits[x]->Typeobj);
	    ajStrAssS(&scophit->Typesbj, hitlist->hits[x]->Typesbj);
	    ajStrAssS(&scophit->Model, hitlist->hits[x]->Model);
	    ajStrAssS(&scophit->Alg, hitlist->hits[x]->Alg);
	    ajStrAssS(&scophit->Group, hitlist->hits[x]->Group);
	    scophit->Start = hitlist->hits[x]->Start;
	    scophit->End = hitlist->hits[x]->End;
	    scophit->Rank = hitlist->hits[x]->Rank;
	    scophit->Score = hitlist->hits[x]->Score;
	    scophit->Eval = hitlist->hits[x]->Eval;
	    scophit->Pval = hitlist->hits[x]->Pval;
	    
           	     
	    /* Push scophit onto list */
	    ajListPushApp(*out,scophit);
	}
    }	
    

    ajListIterFree(&iter);	

    return ajTrue;
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





/* ======================================================================= */
/* ========================== Input & Output ============================= */
/* ======================================================================= */

/* @section Input & output **************************************************
**
** These functions are used for formatted input and output to file.    
**
****************************************************************************/





/* ======================================================================= */
/* ======================== Miscellaneous =================================*/
/* ======================================================================= */
/* @section Miscellaneous ***************************************************
**
** These functions may have diverse functions that do not fit into the other
** categories. 
**
****************************************************************************/








/* @source seqsort application
**
** Removes ambiguities from a file of family members (hits)
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @@
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
** 02111-1307, USA.
**/

#include "emboss.h"

#define MODE_PSIBLAST    1
#define MODE_SWISSPARSE  2
#define MODE_MERGE       3
#define FAMILY           1
#define SUPERFAMILY      2
#define FOLD             3


static AjPList      seqsort_HitlistListRead(AjPStr path, AjPStr extn);
static AjBool       seqsort_PsiblastHitSort(AjPList* famlist,
						 AjPList* supfamlist,
						 AjPList* foldlist,
						 ajint sig_overlap,
						 AjPStr path,
						 AjPStr extn);
static AjBool       seqsort_SwissparseHitSort(AjPList* famlist,
						   AjPList* supfamlist,
						   AjPList* foldlist,
						   ajint sig_overlap,
						   AjPStr path, AjPStr extn);
static AjBool       seqsort_MergeHitSort(AjPList* famlist,
					      AjPList* supfamlist,
					      AjPList* foldlist,
					      ajint sig_overlap,
					      AjPStr file1, AjPStr file2);
static AjBool       seqsort_IdentifyMembers(AjPList* list, ajint node,
						 ajint sig_overlap);
static AjPList      seqsort_FileMerge(AjPFile* file1, AjPFile* file2);
static AjBool       seqsort_WriteOutputFile(AjPFile fptr, AjPList famlist,
					    AjPList supfamlist,
					    AjPList foldlist);

/* @prog seqsort *******************************************************
**
** Removes ambiguities from a set of hits resulting from a database search
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint  sig_overlap  = 0;        /* the minimum overlapping
                                       residues required for merging
                                       of two hits */
 
    AjPStr psipath      = NULL;     /* the name of the directory where
                                       psiblasts results are kept */

    AjPStr psiextn      = NULL;     /* the psiblasts file extension */

    AjPStr swisspath    = NULL;     /* the name of the directory where
                                       swissparse results are kept */

    AjPStr swissextn    = NULL;     /* the swissparse file extension */
   
    AjPList famlist     = NULL;     /* a list of family members */

    AjPList supfamlist  = NULL;     /* a list of superfamily members,
                                       which is initially empty */

    AjPList foldlist    = NULL;     /* a list of fold members, which
                                       is initiallu empty */
    
    AjPStr file1        = NULL;     /* input file 1 in mode 3 */
    AjPStr file2        = NULL;     /* input file 2 in mode 3 */

    AjPStr      *mode   =NULL;      /* Mode of operation from acd*/
/*  AjPStr      outfile = NULL; */  /* Name of the output file */
    AjPFile     outf    = NULL;     /* File pointer for output file */
    

    famlist    = ajListNew();
    supfamlist = ajListNew();
    foldlist   = ajListNew();
           
    embInit("seqsort",argc,argv);
    mode     = ajAcdGetList("mode");
    sig_overlap = ajAcdGetInt("overlap");
/*  outfile = ajAcdGetString("outfile"); */
    outf = ajAcdGetOutfile("outfile");


    if(ajStrChar(*mode, 0) == '1')
    {
      psipath = ajAcdGetString("psipath");
      psiextn = ajAcdGetString("psiextn");
      /* Process results of psiblast searches for scop families */ 
      seqsort_PsiblastHitSort(&famlist,&supfamlist,&foldlist,
			      sig_overlap,psipath,psiextn);

    }
    else if(ajStrChar(*mode, 0) == '2')
    {
      file1  = ajAcdGetString("psifile");
      file2  = ajAcdGetString("swissfile");
      /* Combine results of psiblast and swissparse searches */
      seqsort_MergeHitSort(&famlist,&supfamlist,&foldlist,sig_overlap,
			   file1,file2);
    }


    /* Write output file */
/*    if((outf=ajFileNewOut(outfile))==NULL)
	ajWarn("Could not open output file for writing");
    else	
    { */
	seqsort_WriteOutputFile(outf, famlist, supfamlist, foldlist);
	ajFileClose(&outf);
/*    } */
    
    
	    

  
    /* clean up */
    ajStrDel(&mode[0]);
    AJFREE(mode);
    ajListDel(&famlist);
    ajListDel(&supfamlist);
    ajListDel(&foldlist);
    ajStrDel(&psipath);
    ajStrDel(&psiextn);
    ajStrDel(&swisspath);
    ajStrDel(&swissextn);
    ajStrDel(&file1);
    ajStrDel(&file2);
/*    ajStrDel(&outfile); */
    
    ajExit();

    return 0;
}

/* @funcstatic seqsort_HitlistListRead  **********************************
**
** Looks in a specified directory for a given extension and create a list of
** hitlist structures of the hits in the file matching this extension.
**
** @param [r] path       [AjPStr]    File path of hits files.
** @param [r] extn       [AjPStr]    File extension of hits files.
** 
** @return [AjPList] tmplist, which is a list of hitlist structures.
** @@
******************************************************************************/

static AjPList seqsort_HitlistListRead(AjPStr path, AjPStr extn)
{
    AjPStr tmp         = NULL;	 /* temparary string */
    AjPStr filename    = NULL;	 /* the name of the file containing
                                    the results of a psiblasts run */
    AjPStr logf        = NULL;   /* log file pointer */
     
    AjPList list       = NULL;   /* a list to hold the file names */
    AjPList tmplist    = NULL;	 /* temparary list */

    AjPHitlist hitlist = NULL;   /* hitlist structure for reading a
                                    psiblasts or swissparse results
                                    file */
    
    AjPFile  inf       = NULL;   /* file containing the results of a
                                    psiblasts or swissparse run */
    
    
    tmp      = ajStrNew();
    filename = ajStrNew();
    logf     = ajStrNew();

    list     = ajListNew();
    tmplist  = ajListNew();
    
    inf      = ajFileNew();
    
    /* Check directories */
    if((!ajFileDir(&path)) || (!(extn)))
	ajFatal("Could not open psiblast results directory");    
    
    /* Create list of files in the path */
    ajStrAssC(&tmp, "*");  		/* assign a wildcard to tmp */
	
    if((ajStrChar(extn, 0)=='.')) 	/* checks if the file
                                           extension starts with "."  */
	ajStrApp(&tmp, extn);    	/* assign the acd input file
                                           extension to tmp */
 
    /* this picks up situations where the user has specified an
       extension without a "." */
    else
    {
	ajStrAppC(&tmp, ".");       	/* assign "." to tmp */  
	ajStrApp(&tmp, extn);       	/* append tmp with a user
                                           specified extension */
    }	

    /* all files containing hits will be in a list */
    ajFileScan(path, tmp, &list, ajFalse, ajFalse, NULL, NULL, ajFalse, NULL);
    
    /* read each psiblast file and create a list of Scophit structures */
    while(ajListPop(list,(void **)&filename))
    {
	if((inf = ajFileNewIn(filename)) == NULL)
	{
	  ajWarn("Could not open for reading\n");
	  ajFmtPrintS(&logf,"WARN  Could not open for reading %S\n",filename);
	  continue;	    
	}	
	
	ajXyzHitlistRead(inf,"//",&hitlist); /* read each psiblast file
//into a Hitlist structure */
	ajListPushApp(tmplist, hitlist);        /* create a temporary
                                                   list of hitlist
                                                   structures */
    }
    
    /* clean up */
    ajStrDel(&tmp);
    ajStrDel(&filename);
    ajStrDel(&logf);
    ajListDel(&list);
    ajFileClose(&inf);
    
    return tmplist;
}


/* @funcstatic seqsort_FileMerge *****************************************
**
** Looks at a specific directory for the given files(2) and creates a list
** of hitlist structures. The Hitlist object from file1 is set to high 
** priority and that from file2 is set to low priority.
**
** @param [r] file1       [AjPFile *]    Hits file 1.
** @param [r] file2       [AjPFile *]    Hits file 2.
**
** @return [AjPList] tmplist, which is a list of hitlist structures.
** @@
******************************************************************************/

static AjPList seqsort_FileMerge(AjPFile* file1, AjPFile* file2)
{
    AjPList tmplist    = NULL;  /* temparary list */
    AjPHitlist hitlist = NULL;  /* hit structure for reading a
                                   psiblasts or swissparse results
                                   file */
  
    tmplist  = ajListNew();
    
    /* read file1 and push it on to tmplist */
    ajXyzHitlistRead(*file1,"//",&hitlist);
    ajXyzHitlistPriorityHigh(&hitlist);
    ajListPushApp(tmplist,hitlist);

    /* read file2 and push it on to tmplist */
    ajXyzHitlistRead(*file2,"//",&hitlist);
    ajXyzHitlistPriorityLow(&hitlist);
    ajListPushApp(tmplist,hitlist);   

    return tmplist;
}


/* @funcstatic seqsort_PsiblastHitSort  **********************************
**
** Sorts the results of a psiblasts run. Add the hits to the relavent list 
** created in main.
**
** @param [w] famlist    [AjPList *] Families list
** @param [w] supfamlist [AjPList *] Superfamilies list
** @param [w] foldlist   [AjPList *] Folds list
** @param [r] sig_overlap [ajint]    The minimum overlaping residues required
**                                   for merging of two hits
** @param [r] path       [AjPStr]    File path of hits files.
** @param [r] extn       [AjPStr]    File extension of hits files.
** 
** @return [AjBool] ajTrue if the list has been sorted, ajFalse otherwise.
** @@
******************************************************************************/

static AjBool seqsort_PsiblastHitSort(AjPList* famlist,
					   AjPList* supfamlist,
					   AjPList* foldlist,
					   ajint sig_overlap, AjPStr path,
					   AjPStr extn)
{

    AjPList hitslist   = NULL;		/* a list of hitlist structures */
    AjIList iter       = NULL;		/* a list iterator for family list */
  
    AjPScophit  hit    = NULL;		/* a given hit in the list */
    AjPScophit  nexthit= NULL;		/* the next hit in the list */

   
    /* Check args */
    if((!(*famlist)) || (!(*supfamlist)) || (!(*foldlist)) || (!path)
       || (!extn))
	return ajFalse;
    

    hitslist    = ajListNew();

    /***********************************************************************/
    /** FIGURE A.2 Preparation of list for processing results of PSIBLAST **/
    /** searches for SCOP families.                                       **/
    /***********************************************************************/
 
    /* read the files containing the psiblasts hits and construct a
       list of hitlist structures */
    hitslist  =  seqsort_HitlistListRead(path,extn);

    /* Convert the hitslist to a single list of Scophit structures
       (families list)*/
    ajXyzHitlistToScophits(hitslist,famlist);

    /***********************************************************************/
    /** FIGURE A.3 Identify members of families (merge overlapping hits)  **/
    /***********************************************************************/

    iter=ajListIter(*famlist); 

    /* sort list, first by Family, then by Accession number, and
       finally by Start */
    ajListSort3(*famlist, ajXyzScophitCompFam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
        
    /* get the first node in the list, only once */
    hit = (AjPScophit)ajListIterNext(iter);
  
    /* Loop while we can get another hit */
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the families identical */
	    if(ajStrMatch(hit->Family,nexthit->Family))
	    {
		/* Replace 2 hits in list with a merged hit write back
                   to the same list*/
		ajXyzScophitMergeInsertThis(*famlist,hit,nexthit,iter);
		hit = (AjPScophit)ajListIterNext(iter);
	    }
	}	

	/* would mean that the two hits were distinct and should be
           left in the list */
	else
	{
	    /* move one node along */
	    hit = nexthit;
	}
    }
    ajListIterFree(iter);

    /* The end of the list been reached */
    /* Delete hits in the list that are targetted for removal */
    ajListGarbageCollect(*famlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
    
    /* sort list , first by accession number,then by start and finally
       by family */
    ajListSort3(*famlist, ajXyzScophitCompId, ajXyzScophitCompStart,
		ajXyzScophitCompFam);

    ajListIterFree(iter);

   
    /***********************************************************************
    ** FIGURE A.4 Identify members of families (remove superfamily and
    ** fold members from family list).
    ***********************************************************************/

    iter= ajListIter(*famlist);
    
    /* get the first node in the list, only once */
    hit = (AjPScophit)ajListIterNext(iter); 
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the superfamilies identical */
	    if(ajStrMatch(hit->Superfamily,nexthit->Superfamily)&&
	       !(ajStrMatch(hit->Family,nexthit->Family)))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the supfamlist */
		ajXyzScophitMergeInsertOther(*supfamlist,hit,nexthit);     
	    }	
	    /* are the folds identical */
	    else if(ajStrMatch(hit->Fold,nexthit->Fold)&&
		    !(ajStrMatch(hit->Superfamily,nexthit->Superfamily))&&
		    !(ajStrMatch(hit->Family,nexthit->Family)))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the foldlist */
		ajXyzScophitMergeInsertOther(*foldlist,hit,nexthit);    
	    }
	    else
	    {
		/* Target both hits for removal. */
		ajXyzScophitTarget(&hit);
		ajXyzScophitTarget(&nexthit);
	    }
	}
	else
	    /* move one node along */
	    hit=nexthit;
    }
    
    /* The end of the list been reached */
    /* Delete hits in the list that are targetted for removal */
    ajListGarbageCollect(*famlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);

    /* sort list, first by Family, then by accession number, and
       finally by Start */
    ajListSort3(*famlist, ajXyzScophitCompFam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    /* the iterator */
    ajListIterFree(iter);
    
    /***********************************************************************
    ** FIGURE A.5 Identify members of superfamilies (merge overlapping
    ** hits)
    ************************************************************************/

    iter= ajListIter(*supfamlist);
    
    /* sort list, first by superfamily, then by accession number and
       finally by the start */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    /* get the first node in the list, only once */ 
    hit = (AjPScophit)ajListIterNext(iter);	                
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {   	   
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the superfamilies identical */
	    if(ajStrMatch(hit->Superfamily,nexthit->Superfamily))
	    {
		/* merge the two hits and then write back to the same list */
		ajXyzScophitMergeInsertThis(*supfamlist,hit,nexthit, iter);
		hit = (AjPScophit)ajListIterNext(iter);
	    }	
	}
	/* would mean that the two hits were distinct and should be
           left in the list */
	else
	{
	    /* move one node along */
	    hit = nexthit;
	}
    }

    /* The end of the list been reached */
    /* Delete hits in the list that are targetted for removal */
    ajListGarbageCollect(*supfamlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);

    /* sort list , first by accession number,then by start and finally
       by family */
    ajListSort3(*supfamlist, ajXyzScophitCompId, ajXyzScophitCompStart,
		ajXyzScophitCompSfam);
    
    ajListIterFree(iter);
    
    /************************************************************************
    ** FIGURE A.6 Identify members of superfamilies (remove fold
    ** members from the superfamilies list.
    **********************************************************************/
    
    iter= ajListIter(*supfamlist);
    
    /* get the first node in the list, only once */
    hit = (AjPScophit)ajListIterNext(iter);                     
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the folds identical */
	    if(ajStrMatch(hit->Fold,nexthit->Fold)&&
	       !(ajStrMatch(hit->Superfamily,nexthit->Superfamily))&&
	       !(ajStrMatch(hit->Family,nexthit->Family)))
	    {
		/* target both hits for removal. Place a hit
		   corresponding to the merging of the two hits into
		   the folds list */
		ajXyzScophitMergeInsertOther(*foldlist,hit,nexthit);
	    }
	    else
	    {
		/* Target both hits for removal. */
		ajXyzScophitTarget(&hit);
		ajXyzScophitTarget(&nexthit);
	    }
	}
	else
	    /* move one node along */
	    hit = nexthit;
    }
    
    /* The end of the list has been reached */
    /* Delete hits in the list that are targeted for removal */
    ajListGarbageCollect(*supfamlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);

    /* sort list, first by Family, then by accession number, and
       finally by Start */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    ajListIterFree(iter);
    
    /************************************************************************
    ** FIGURE A.7 Identify members of folds (merge overlapping hits
    ** and remove hits of unknown classification).
    *********************************************************************/
    
    iter= ajListIter(*foldlist);
    
    /* sort list, first by fold, then by accession number and finally
       by the start */
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    /* get the first node in the list, only once */ 
    hit = (AjPScophit)ajListIterNext(iter);	
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the folds identical */
	    if(ajStrMatch(hit->Fold,nexthit->Fold))
	    {
		/* target both hits for removal. Place a hit
		   corresponding to the merging of the two hits into
		   the folds (same) list */
		ajXyzScophitMergeInsertThis(*foldlist,hit,nexthit, iter);
		hit = (AjPScophit)ajListIterNext(iter);
	    }
	    else
	    {
		/* Target both hits for removal. */
		ajXyzScophitTarget(&hit);
		ajXyzScophitTarget(&nexthit);
	    }
	}
	else
	    /* move one node along */
	    hit = nexthit;
    }	
    
    /* The end of the list has been reached */
    /* Delete hits in the list that are targeted for removal */
    ajListGarbageCollect(*foldlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
    
    ajListSort3(*foldlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    ajListIterFree(iter);
    
    /* clean up */
    ajListDel(&hitslist);
    
    return ajTrue;
}



/* @funcstatic seqsort_SwissparseHitSort  ********************************
**
** Sorts the results of a swissparse run. Add the hits to the relavent list 
** created in main.
**
** @param [w] famlist    [AjPList *] Families list
** @param [w] supfamlist [AjPList *] Superfamilies list
** @param [w] foldlist   [AjPList *] Folds list
** @param [r] sig_overlap [ajint]    The minimum overlapping residues
**                                   required for merging of two hits
** @param [r] path       [AjPStr]    File path of hits files.
** @param [r] extn       [AjPStr]    File extension of hits files.
** 
** @return [AjBool] ajTrue if the list has been sorted, ajFalse otherwise.
** @@
*****************************************************************************/

static AjBool seqsort_SwissparseHitSort(AjPList* famlist,
					     AjPList* supfamlist,
					     AjPList* foldlist,
					     ajint sig_overlap,
					     AjPStr path, AjPStr extn)
{
    AjPList hitslist       = NULL;	/* a list of hitlist structures */
    AjPList copiedfamlist  = NULL;	/* a copy of the family list */
    AjPList copiedsfamlist = NULL;	/* a copy of the superfamily list */
    
    AjIList iter           = NULL;	/* a list iterator */
    
    AjPScophit  hit        = NULL;	/* a given hit in the list */
    AjPScophit  nexthit    = NULL;	/* the next hit in the list */
    AjPScophit  tmp        = NULL;	/* a tmp Scophit variable */
    
    
    hitslist       = ajListNew();
    copiedfamlist  = ajListNew();
    copiedsfamlist = ajListNew();
    
    tmp     = ajXyzScophitNew();
    
    /* Check args */
    if((!(*famlist)) || (!(*supfamlist)) || (!(*foldlist)) || (!path) ||
       (!extn))
	return ajFalse;
    
    /*************************************************************************
    ** FIGURE B.2 Preparation of list for processing results of
    ** SWISSPARSE searches for SCOP families
    ************************************************************************/
    
    /* read the files containing the swissparse hits and construct a
       list of hitlist structure for each type of hit */
    hitslist  =  seqsort_HitlistListRead(path,extn);
    
    /* Convert the list of hitlist to three populated lists of Scophit
       structures */
    ajXyzHitlistToThreeScophits(hitslist,famlist,supfamlist,foldlist);
    
    /************************************************************************
    ** FIGURE B.3 Identify members of families (remove super-family
    ** and fold members from family list)
    ***********************************************************************/
    
    iter = ajListIter(*famlist);
    
    /* sort list, first by Family, then by Accession number and
       finally by the Start */
    ajListSort3(*famlist, ajXyzScophitCompFam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    /* get the first node in the list, only once */
    hit  = (AjPScophit)ajListIterNext(iter);
   
    /* Loop while we can get another hit */
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the superfamilies identical */
	    if(ajStrMatch(hit->Superfamily,nexthit->Superfamily) &&
	       !(ajStrMatch(hit->Family,nexthit->Family)))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the supfamlist */
		ajXyzScophitMergeInsertOther(*supfamlist,hit,nexthit);     
	    }
	    /* are the folds identical */
	    else if(ajStrMatch(hit->Fold,nexthit->Fold) &&
		    !(ajStrMatch(hit->Superfamily,nexthit->Superfamily)) &&
		    !(ajStrMatch(hit->Family,nexthit->Family)))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the supfamlist */
		ajXyzScophitMergeInsertOther(*foldlist,hit,nexthit);     
	    }
	    else
	    {
		/* Target both hits for removal. */
		ajXyzScophitTarget(&hit);
		ajXyzScophitTarget(&nexthit);
	    }
	}
	else	
	    /*move one node along */
	    hit = nexthit;
    }	
    
    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal */
    ajListGarbageCollect(*famlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
    
    /* Sort list again */
    ajListSort3(*famlist, ajXyzScophitCompFam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    ajListIterFree(iter);
    
    
    
    /************************************************************************
    ** FIGURE B.4 Identify members of superfamilies (remove duplicate
    ** hits and fold members from superfamily list)
    ****************************************************************/
    
    iter = ajListIter(*supfamlist);
    /* sort list, first by superfamily, then by accession number and
       finally by the start */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    
    hit  = (AjPScophit)ajListIterNext(iter);
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/*check if the accession numbers are the same and if there is
          significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the superfamilies identical */
	    if(ajStrMatch(hit->Superfamily,nexthit->Superfamily))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the supfamlist */
		ajXyzScophitMergeInsertThis(*supfamlist,hit,nexthit,iter);
		hit = (AjPScophit)ajListIterNext(iter);
	    }
	    /* are the folds identical */
	    else if(ajStrMatch(hit->Fold,nexthit->Fold) &&
		    !(ajStrMatch(hit->Superfamily,nexthit->Superfamily)) &&
		    !(ajStrMatch(hit->Family,nexthit->Family)))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the supfamlist */
		ajXyzScophitMergeInsertOther(*foldlist,hit,nexthit);
	    }
	    else
	    {
		/* Target both hits for removal. */
		ajXyzScophitTarget(&hit);
		ajXyzScophitTarget(&nexthit);
	    }
	}
	else	
	    /*move one node along */
	    hit = nexthit;
    }	
    
    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal */
    ajListGarbageCollect(*supfamlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
    
    /* Sort list again */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    ajListIterFree(iter);
    
    
    /************************************************************************
    ** FIGURE B.5 Identify members of folds (remove super-family and
    ** fold members from folds list)
    ************************************************************************/
    
    iter = ajListIter(*foldlist);
    /* sort list, first by fold, then by accession number and finally
       by the start */
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    
    hit  = (AjPScophit)ajListIterNext(iter);
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/*check if the accession numbers are the same and if there is
          significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* are the folds identical */
	    if(ajStrMatch(hit->Fold,nexthit->Fold))
	    {
		/* Target both hits for removal.  Place a hit
                   corresponding to the merging of the two hits into
                   the supfamlist */
		ajXyzScophitMergeInsertThis(*foldlist,hit,nexthit,iter);
	    }		
	    else	
	    {	
		/* Target both hits for removal. */
		ajXyzScophitTarget(&hit);
		ajXyzScophitTarget(&nexthit);
	    }		
	}
	else	
	    /*move one node along */
	    hit = nexthit;
    }
    
    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal */
    ajListGarbageCollect(*foldlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
    
    /* Sort list again */
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    ajListIterFree(iter);
    
    
    /**************************************************************************
    ** FIGURE B.6 Identify members of superfamilies (remove known
    ** family members from superfamilies list)
    ************************************************************************/
    
    /* make a copy of the families list */
    copiedfamlist = ajXyzScophitListCopy(*famlist);
    
    /* append the copied families list to the end of the superfamilies list */
    while(ajListPop(copiedfamlist,(void **)&tmp))
    {
	/* target each hit from the families list for removal */
	tmp->Target = ajTrue;
	ajListPushApp(*supfamlist,tmp);        
    }
    
    /* sort list, first by superfamily, then by accession number and
       finally by the start */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);;
    
    iter = ajListIter(*supfamlist);
    
    hit  = (AjPScophit)ajListIterNext(iter);
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {	
	/*check if the accession numbers are the same and if there is
          significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* Check that one of the hits is targetted for removal */
	    if(!hit->Target && !nexthit->Target)
		ajWarn("Neither hit targetted for removal ajXyzSwissparseHitSort.\n"
		       "Unexpected behaviour.");
	  
	    /* Identify the hit that is not targeted for removal
	       and target this hit for removal from the list */
	    if(!hit->Target)
		hit->Target=ajTrue;
	    else if(!nexthit->Target)
		nexthit->Target=ajTrue;
	  
	}
	else
	    /*move one node along */
	    hit = nexthit;
    }
    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal. */
    ajListGarbageCollect(*supfamlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
    
    /* Sort list again */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    ajListIterFree(iter);
    
    
    /*************************************************************************
    ** FIGURE B.7 Identify members of folds (remove known family and
    ** superfamily members from folds list)
    ************************************************************************/
    
    /* make a copy of the families list */
    copiedfamlist = ajXyzScophitListCopy(*famlist);

    /* make a copy of the superfamilies list */
    copiedsfamlist = ajXyzScophitListCopy(*supfamlist);
    
    /* append the copied families list tp the end of the folds list */
    while(ajListPop(copiedfamlist,(void **)&tmp))
    {
	/* target each hit from the families list for removal */
	tmp->Target=ajTrue;
	

	ajListPushApp(*foldlist,tmp);        
    }
   
    /* append the copied superfamilies list tp the end of the folds list */
    while(ajListPop(copiedsfamlist,(void **)&tmp))
    {
	/* target each hit from the families list for removal */
	tmp->Target=ajTrue;

	ajListPushApp(*foldlist, tmp);        

    }

    iter = ajListIter(*foldlist);

    /* sort list, first by superfamily, then by accession number and
       finally by the start */
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);

    hit  = (AjPScophit)ajListIterNext(iter);
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {	
	/*check if the accession numbers are the same and if there is
          significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* Check that one of the hits is targetted for removal */
	    if(!hit->Target && !nexthit->Target)
		ajWarn("Neither hit targetted for removal ajXyzSwissparseHitSort.\n"
		       "Unexpected behaviour.");

	    /* Identify the hit that is not targeted for removal
	       and target this hit for removal from the list */
	    if(!hit->Target)
		hit->Target=ajTrue;
	    else if(!nexthit->Target)
		nexthit->Target=ajTrue;
	}
	else
	    /*move one node along */
	    hit = nexthit;
    }

    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal. */
    ajListGarbageCollect(*foldlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);
	
    /* Sort list again */
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);

    ajListIterFree(iter);
   
    /* clean up */
    ajListDel(&hitslist);
    
    return ajTrue;
}


/* @funcstatic seqsort_IdentifyMembers ***********************************
**
** Identifies members of a scop node for a given list. Removes overlaping 
** low-priority hits from list.
**
** @param [r] list       [AjPList *] working list.
** @param [r] node       [ajint]     the scop node 
** @param [r] sig_overlap [ajint]    The minimum overlapping residues
**                                   required for merging of two hits
**
** @return [AjBool] ajTrue if the list has been processed, ajFalse otherwise.
** @@
******************************************************************************/
static AjBool seqsort_IdentifyMembers(AjPList* list, ajint node,
				      ajint sig_overlap)
{
    AjIList iter      = NULL;   /* a list iterator */
    
    AjPScophit hit    = NULL;   /* a given hit in the list */
    AjPScophit nexthit= NULL;   /* the next hit in the list */
    
    /* Check args */
    if(!(*list))
	return ajFalse;    

    /* sort list */
    if(node == FAMILY)
	ajListSort3(*list, ajXyzScophitCompFam, ajXyzScophitCompId,
		    ajXyzScophitCompStart);


    else if(node == SUPERFAMILY)
	ajListSort3(*list, ajXyzScophitCompSfam, ajXyzScophitCompId,
		    ajXyzScophitCompStart);


    else if(node  == FOLD)
	ajListSort3(*list, ajXyzScophitCompFold, ajXyzScophitCompId,
		    ajXyzScophitCompStart);
	

    iter = ajListIter(*list); 

    /* get the first node in the list, only once */
    hit = (AjPScophit)ajListIterNext(iter);  

    /* Loop while we can get another hit */
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {
	/* check if the accession numbers are the same and if there is
           significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    if(node == FAMILY && ajStrMatch(hit->Family,nexthit->Family))
	    {
		/* target low priority hit for removal from the
                   families list */
		ajXyzScophitTargetLowPriority(&hit);
		ajXyzScophitTargetLowPriority(&nexthit);
	    }   	    
	    else if(node == SUPERFAMILY &&
		    ajStrMatch(hit->Superfamily,nexthit->Superfamily))
	    {
		/* target low priority hit for removal from the
                   superfamilies list */
		ajXyzScophitTargetLowPriority(&hit);
		ajXyzScophitTargetLowPriority(&nexthit);
	    }
	    else if(node == FOLD && ajStrMatch(hit->Fold,nexthit->Fold))
	    {
		/* target low priority hit for removal from the folds list */
		ajXyzScophitTargetLowPriority(&hit);
		ajXyzScophitTargetLowPriority(&nexthit);
	    }
	    
	}
	/* move one node along */
	hit = nexthit;	
    }

    /* Remove hits that are targetted for removal */
    ajListGarbageCollect(*list, ajXyzScophitDelWrap, (const void *) ajXyzScophitCheckTarget);


    /* the end of the list has been reached */
    /* sort the list */
     if(node == FAMILY)
	 ajListSort3(*list, ajXyzScophitCompFam, ajXyzScophitCompId, ajXyzScophitCompStart);
	
    else if(node == SUPERFAMILY)
	 ajListSort3(*list, ajXyzScophitCompSfam, ajXyzScophitCompId, ajXyzScophitCompStart);

    else if(node == FOLD)
	ajListSort3(*list, ajXyzScophitCompFold, ajXyzScophitCompId, ajXyzScophitCompStart);

    else
	ajFatal("incompatible scop node\n");


    /* Tidy up and return */
    ajListIterFree(iter);
    ajXyzScophitDel(&hit);
    ajXyzScophitDel(&nexthit);
    
    return ajTrue;
}



/* @funcstatic seqsort_MergeHitSort **************************************
 **
 ** Combine results of SWISSPARSE and PSIBLAST searches.
 **
 ** @param [w] famlist    [AjPList *] Families list.
 ** @param [w] supfamlist [AjPList *] Superfamilies list.
 ** @param [w] foldlist   [AjPList *] Folds list.
 ** @param [r] sig_overlap [ajint]    The minimum overlapping residues
 **                                   required for merging of two hits
 ** @param [r] file1      [AjPStr]   name of hits file 1.
 ** @param [r] file2      [AjPStr]   name of hits file 2.
 ** 
 ** @return [AjBool] ajTrue if the list has been sorted, ajFalse otherwise.
 ** @@
 ****************************************************************************/

static AjBool seqsort_MergeHitSort(AjPList* famlist,
					AjPList* supfamlist,
					AjPList* foldlist,
					ajint sig_overlap,
					AjPStr file1, AjPStr file2)
{
    AjPList hitslist       = NULL;	/* a list of hitlist structures */
    AjPList copiedfamlist  = NULL;	/* a copy of the family list */
    AjPList copiedsfamlist = NULL;	/* a copy of the superfamily list */

    AjIList iter           = NULL;	/* a list iterator */ 

    AjPScophit hit         = NULL;	/* a given hit in the list */ 
    AjPScophit nexthit     = NULL;	/* the next hit in the list */
    AjPScophit tmp         = NULL;	/* temparary Scophit structure */

    AjPFile    inf1        = NULL;      /* file containing the hits from psiblasts */
    AjPFile    inf2        = NULL;      /* file containing the hits
                                           from swissparse */
    

    inf1 = ajFileNewIn(file1);
    inf2 = ajFileNewIn(file2);

    /***********************************************************************
    ** FIGURE C.2 Preparation and sorting of lists for merging two input
    ** files.
    ***********************************************************************/ 

    /* read the files containing the swissparse and psiblasts hits and
       construct a list of hitlist structures.  This call sets the
       Hitlist object from file1 is to high priority, that from file2
       is set to low priority. */
    hitslist = seqsort_FileMerge(&inf1,&inf2);

    /* converts the list of hitlist to three lists of Scophit
       structures i.e. famlist, supfamlist and foldlist. */
    ajXyzHitlistToThreeScophits(hitslist,famlist,supfamlist,foldlist);
    
    /*************************************************************************
    ** FIGURE C.3 Identify members of families (remove overlapping
    ** low-priority hits from families list)
    *************************************************************************/
    seqsort_IdentifyMembers(famlist,FAMILY,sig_overlap);


    /*************************************************************************
    ** FIGURE C.4 Identify members of superfamilies (remove
    ** overlapping low-priority hits from superfamilies list)
    *************************************************************************/
    seqsort_IdentifyMembers(supfamlist,SUPERFAMILY,sig_overlap);


    /*************************************************************************
    ** FIGURE C.5 Identify members of folds (remove overlapping
    ** low-priority hits from the folds list)
    *************************************************************************/
    seqsort_IdentifyMembers(foldlist,FOLD,sig_overlap);
    
    
    /*************************************************************************
    ** FIGURE C.6 Identify members of superfamilies (remove known
    ** family members from superfamilies list)
    *************************************************************************/
    
    /* make a copy of the families list */
    copiedfamlist = ajXyzScophitListCopy(*famlist);
    
    /* append the families list tp the end of the superfamilies list */
    while(ajListPop(copiedfamlist,(void **)&tmp))
    {
	/* target each hit from the families list for removal */
	tmp->Target=ajTrue;
	
	
	ajListPushApp(*supfamlist,tmp);        
    }	
    
    /* sort list, first by superfamily, then by accession number and
       finally by the start */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    
    
    iter = ajListIter(*supfamlist);
    hit  = (AjPScophit)ajListIterNext(iter);
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {	
	/*check if the accession numbers are the same and if there is
          significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* Check that one of the hits is targetted for removal */
	    if(!hit->Target && !nexthit->Target)
		ajWarn("Neither hit targetted for removal ajXyzSwissparseHitSort.\n"
		       "Unexpected behaviour.");

	    /* Identify the hit that is not targeted for removal
	       and target this hit for removal from the list */
	    if(!hit->Target)
		hit->Target=ajTrue;
	    else if(!nexthit->Target)
		nexthit->Target=ajTrue;
	}
	else
	    /*move one node along */
	    hit = nexthit;
    }

    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal. */
    ajListGarbageCollect(*supfamlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);

	
    /* Sort list again */
    ajListSort3(*supfamlist, ajXyzScophitCompSfam, ajXyzScophitCompId,
		ajXyzScophitCompStart);

    ajListIterFree(iter);
    
    
    /************************************************************************
    ** FIGURE C.7 Identify members of folds (remove known family and
    ** superfamily members from folds list)
    *************************************************************************/
    
    /* make a copy of the families list */
    copiedfamlist = ajXyzScophitListCopy(*famlist);

    /* make a copy of the superfamilies list */
    copiedsfamlist = ajXyzScophitListCopy(*supfamlist);
    
    /* append the families list to the end of the folds list */
    while(ajListPop(copiedfamlist,(void **)&tmp))
    {	
	/* target each hit from the families list for removal */
	tmp->Target = ajTrue;
	
	ajListPushApp(*foldlist,tmp);        
    }
    
    /* append the superfamilies list to the end of the folds list */
    while(ajListPop(copiedsfamlist,(void **)&tmp))
    {
	/* target each hit from the families list for removal */
	tmp->Target = ajTrue;

	ajListPushApp(*foldlist,tmp);        
    }   

    iter = ajListIter(*foldlist);
    /* sort list, first by superfamily, then by accession number and
       finally by the start */
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);

    hit  = (AjPScophit)ajListIterNext(iter);
    while((nexthit=(AjPScophit)ajListIterNext(iter)))
    {	
	/*check if the accession numbers are the same and if there is
          significant overlap */
	if(ajXyzScophitsOverlapAcc(hit,nexthit,sig_overlap))
	{
	    /* Check that one of the hits is targetted for removal */
	    if(!hit->Target && !nexthit->Target)
		ajWarn("Neither hit targetted for removal seqsort_SwissparseHitSort.\n"
		       "Unexpected behaviour.");

	    /* Identify the hit that is not targeted for removal
	       and target this hit for removal from the list */
	    if(!hit->Target)
		hit->Target=ajTrue;
	    else if(!nexthit->Target)
		nexthit->Target=ajTrue;
	}
	else
	    /*move one node along */
	    hit = nexthit;
    }

    /* the end of the list has been reached */
    /* delete hits in the list that are targeted for removal. */
    ajListGarbageCollect(*foldlist, ajXyzScophitDelWrap,
			 (const void *) ajXyzScophitCheckTarget);

	
    ajListSort3(*foldlist, ajXyzScophitCompFold, ajXyzScophitCompId,
		ajXyzScophitCompStart);
    ajListIterFree(iter);

      
    /* clean up */
    ajListDel(&hitslist);
    ajListDel(&copiedfamlist);
    ajListDel(&copiedsfamlist);
    ajXyzScophitDel(&hit);
    ajXyzScophitDel(&nexthit);
    ajXyzScophitDel(&tmp);
    ajFileClose(&inf1);
    ajFileClose(&inf2);
    
    return ajTrue;
       
}






/* @funcstatic seqsort_WriteOutputFile ****************************************
**
** Undocumented.
**
** @param [?] fptr [AjPFile] Undocumented
** @param [?] famlist [AjPList] Undocumented
** @param [?] supfamlist [AjPList] Undocumented
** @param [?] foldlist [AjPList] Undocumented
** @return [AjBool] Undocumented
** @@
******************************************************************************/

static AjBool seqsort_WriteOutputFile(AjPFile fptr, AjPList famlist, AjPList supfamlist, AjPList foldlist)
{
    AjPHitlist hitlist = NULL;
    AjIList iter       = NULL;

    
    /* Check args */
    if(!famlist && !supfamlist && !foldlist)
	return ajFalse;
    

    while((ajXyzScophitsToHitlist(foldlist, &hitlist, &iter)))
    {
	ajXyzHitlistWrite(fptr, hitlist);
	ajXyzHitlistDel(&hitlist);
	hitlist=NULL;
    }
    ajListIterFree(iter);
    iter=NULL;


    while((ajXyzScophitsToHitlist(supfamlist, &hitlist, &iter)))
    {
	ajXyzHitlistWrite(fptr, hitlist);
	ajXyzHitlistDel(&hitlist);
	hitlist=NULL;
    }

    ajListIterFree(iter);
    iter=NULL;

    while((ajXyzScophitsToHitlist(famlist, &hitlist, &iter)))
    {
	ajXyzHitlistWrite(fptr, hitlist);
	ajXyzHitlistDel(&hitlist);
	hitlist=NULL;
    }
    ajListIterFree(iter);
    iter=NULL;

    return ajTrue;
}


void seqsort_unused()
{
  seqsort_SwissparseHitSort(NULL,NULL,NULL,0,NULL,NULL);
  return;
}

/* @source alignwrap application
**
** Aligns single sequences to an existing seed alignment
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
******************************************************************************
** 
** 
** 
** 
** 
** 
** Operation
** 
** alignwrap parses a stamp alignment annotated with SCOP classification 
** records (generated by the EMBOSS applications scopalign) and a file of 
** database hits (generated by the EMBOSS application psiblasts or swissparse
** and possibly processed by seqsort), and extends the structure-based 
** alignment with sequences from the hits file one sequence at a time by 
** calling clustalw.  A multiple sequence alignment in clustal format is 
** generated.
**
** Notes
** Program requirment:- Requires a working version of clustalw. If you are 
** running this program from the HGMP, need to type "use clustal" at the 
** command line before using this program.
**/


#include "emboss.h"

AjBool ajXyzHitlistExtract(AjPFile *scopf, AjPList *list, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class);
AjBool ajXyzHitlistsToClustal(AjPList *list, AjPFile *outf);
AjBool ajXyzWriteClustalAlign(AjPScopalg align, AjPFile* outf);
AjBool ajXyzExtractFam(AjPFile *scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class, AjPList* list);
AjBool ajXyzExtractSfam(AjPFile *scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class,AjPList* list);
AjBool ajXyzExtractFold(AjPFile *scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class,AjPList* list);

int main(int argc, char **argv)
{
    AjPStr  syscmd      = NULL;  /* command line arguments */
    AjPStr  inpath      = NULL;  /* path to the directory that contain the alignment files*/
    AjPStr  outpath     = NULL;  /* the path to the output directory */
    AjPStr  extn        = NULL;  /* the file extention of the alignment files */
    AjPStr  outextn     = NULL;  /* the file extention of the extended alignment files */
    AjPStr  tmp_str     = NULL;  /* temparary string */
    AjPStr  tmp_name    = NULL;  /* string object for holding randomly generated file names */

    AjPStr  filename    = NULL;  /* the name of the present working file */
    AjPStr  outfilename = NULL;  /* the name of the current output file */
    AjPStr  clustinf1   = NULL;  /* the name of the input alignment file for clustalw */
    AjPStr  clustinf2   = NULL;  /* the name of the input sequence file for clustalw */
    AjPStr  treefile    = NULL;  /* the name of the .dnd file that will be created */
    
    AjPStr  scopfam     = NULL;  /* name of the scop families file */
    
    AjPStr fam          = NULL;   /* family name */
    AjPStr sfam         = NULL;   /* superfamily name */
    AjPStr fold         = NULL;   /* fold name */
    AjPStr class        = NULL;   /* class name */
    AjPStr msg          = NULL;   /* Message string */
    
   
    AjPFile alnf       	= NULL;   /* the seed alignment file with the scop info and the stamp info */
    AjPFile outf1       = NULL;   /* the outfile for the parsed database sequences in CLUSTAL format */
    AjPFile outf2       = NULL;   /* the outfile for the parsed seed alignment in CLUSTAL format */
    AjPFile scopf       = NULL;   /* the scop families file */
            
    AjPList list        = NULL;   /* a list of relavant file names */
    AjPList tmp_list    = NULL;   /* temp list */
    
    AjPScopalg align    = NULL;   /* a scop alignment structure */

    AjPHitlist temphl   = NULL;   /* Temp. pointer to Hitlist object */
    
    ajint   posdash     = 0;      /* Position of last '/' in filenames from 'list' */
    ajint   posdot      = 0;      /* Position of last '.' in filenames from 'list' */
    
    
    msg        = ajStrNew();
    outfilename= ajStrNew();
    syscmd     = ajStrNew();
    inpath     = ajStrNew();
    outpath    = ajStrNew();
    extn       = ajStrNew();
    outextn    = ajStrNew();
    tmp_str    = ajStrNew();
    tmp_name   = ajStrNew();
    clustinf1  = ajStrNew();
    clustinf2  = ajStrNew();
    treefile   = ajStrNew();
    
    fam        = ajStrNew();
    sfam       = ajStrNew();
    fold       = ajStrNew();
    class      = ajStrNew();
   
    list       = ajListNew();
    
    embInit("alignwrap",argc,argv);
    
    inpath    = ajAcdGetString("inpath");
    extn      = ajAcdGetString("extn");
    outextn   = ajAcdGetString("outextn");
    scopfam   = ajAcdGetString("scopfamilies");
    outpath   = ajAcdGetString("outpath");
    
    /* Check directories */
    if((!ajFileDir(&inpath)) || (!(extn)))
	ajFatal("Could not open seed alignment directory or file extension NULL");
    
    if((!ajFileDir(&outpath)) || (!(outextn)))
	ajFatal("Could not open extended alignment directory or file extension NULL");
    
    


    /* Add a '.' to outextn if one does not already exist */
    if((ajStrChar(outextn, 0)!='.')) 	/* checks if the file extension starts with "." */
	ajStrInsertC(&outextn, 0, ".");



    /* Create list of files in the path */
    ajStrAssC(&tmp_str, "*");  		/* assign a wildcard to tmp_str */
	
    if((ajStrChar(extn, 0)=='.')) 	/* checks if the file extension starts with "." */
	ajStrApp(&tmp_str, extn);    	/* assign the acd input file extension to tmp */
    /* this picks up situations where the user has specified an extension without a "." */
    else
    {
	ajStrAppC(&tmp_str, ".");       /* assign "." to tmp */  
	ajStrApp(&tmp_str, extn);       /* append tmp with a user specified extension */  
    }	

    /* all files containing seed alignments will be in a list */
    ajFileScan(inpath, tmp_str, &list, ajFalse, ajFalse, NULL, NULL, ajFalse, NULL);  


    /*Initialise random number generator for naming of tmp_name. files and create clustalw input files */
    ajRandomSeed();
    ajStrAssC(&tmp_name,ajFileTempName(NULL));


    /* read each psiblast file and create a list of Scophit structures */
    while(ajListPop(list,(void **)&filename))
    {
	/* open a scop seed alignment file */
	if((alnf = ajFileNewIn(filename)) == NULL)
	{
	    ajFmtPrintS(&msg, "Could not open seed alignment file %S", filename);
	    ajWarn(ajStrStr(msg));
	}
	else
	{
	    /* set up the random file for the input alignment for clustal */
	    ajStrAss(&clustinf1,tmp_name);
	    ajStrAppC(&clustinf1,".aln");

	    /* set up the random file for the input sequences for clustal */
	    ajStrAss(&clustinf2,tmp_name);
	    ajStrAppC(&clustinf2,".seqs");

	    /* setup the .dnd tree file to delete later */
	    ajStrAss(&treefile,tmp_name);
	    ajStrAppC(&treefile,".dnd");
	    
	    /* read the scop seed alignment file into a scopalg object */
	    ajXyzScopalgRead(alnf,&align);

	    /* extract the scop information */
	    ajStrAss(&fam,align->Family);
	    ajStrAss(&sfam,align->Superfamily);
	    ajStrAss(&fold,align->Fold);
	    ajStrAss(&class,align->Class);

	    /* write out the seed alignment (from the scopalg object) in CLUSTAL format */
	    outf1 = ajFileNewOut(clustinf1);
	    ajXyzWriteClustalAlign(align,&outf1);
	    ajFileClose(&outf1);

	    /* Delete Scopalg structure */
	    ajXyzScopalgDel(&align);
	    
	    /*open the scopfamilies file and pass a file pointer into each of the functions */
	    scopf = ajFileNewIn(scopfam);
	    /* extract the relavent family or superfamily or etc.. into a list of Hitlist objects. */
	    tmp_list = ajListNew();
	    ajXyzHitlistExtract(&scopf,&tmp_list, fam, sfam, fold, class);
	    ajFileClose(&scopf);
	    
	    /* write out the sequences in the list of Hitlist structures in CLUSTAL format. */
	    outf2 = ajFileNewOut(clustinf2);
	    ajXyzHitlistsToClustal(&tmp_list,&outf2);
	    ajFileClose(&alnf);

	    ajFileClose(&outf2);

	    while(ajListPop(tmp_list,(void **)&temphl))
		ajXyzHitlistDel(&temphl);
	    ajListDel(&tmp_list);


	    /* Create the name of the output file */
	    posdash = ajStrRFindC(filename, "/");
	    posdot  = ajStrRFindC(filename, ".");
	    if(posdash >= posdot)
		ajFatal("Could not create filename. Email rranasin@hgmp.mrc.ac.uk");
	    else
	    {
		ajStrAssSub(&outfilename, filename, posdash+1, posdot-1);
	    }
	    
	    
	    /* run clustalw in profile to sequence mode */
	    ajFmtPrintS(&syscmd,"clustalw -type=protein -profile1=%S -sequences -profile2=%S -outfile=%S%S%S\n",
			clustinf1,clustinf2,outpath, outfilename, outextn);
	    system(ajStrStr(syscmd));
	    
	    /* clean up directory */
	    ajSysUnlink(&clustinf1);
	    ajSysUnlink(&clustinf2);
	    ajSysUnlink(&treefile);
	}
	/* Free the nodes ! */
	ajStrDel(&filename);
    }
    

    /* clean up */
    ajStrDel(&msg);
    ajStrDel(&syscmd);
    ajStrDel(&fam);
    ajStrDel(&sfam);
    ajStrDel(&fold);
    ajStrDel(&class);
    ajStrDel(&inpath);
    ajStrDel(&outpath);
    ajStrDel(&extn);
    ajStrDel(&outextn);
    ajStrDel(&tmp_str);
    ajStrDel(&tmp_name);
    ajStrDel(&clustinf1);
    ajStrDel(&clustinf2);
    ajStrDel(&treefile);
    ajStrDel(&scopfam);
    ajStrDel(&outfilename);
    ajListDel(&list);
    
    ajExit();
    return 0;
    
}


/* @func ajXyzHitlistExtract ************************************************
**
** Reads a scop families file and writes a list of Hitlist objects containing 
** all domains matching the scop classification provided.
**
** @param [r] scopf     [AjPFile *]      The scop families file.
** @param [r] list      [AjPList *]      List of Hitlist objects.
** @param [r] fam       [AjPStr   ]      Family.
** @param [r] sfam      [AjPStr   ]      Superfamily.
** @param [r] fold      [AjPStr   ]      Fold.
** @param [r] class     [AjPStr   ]      Class
** 
** @return [AjBool] True on success (a list of hits was read)
** @@
******************************************************************************/

AjBool ajXyzHitlistExtract(AjPFile *scopf, AjPList *list, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class)
{
    AjBool donemem=ajFalse;   

    /* Allocate the list if it does not already exist */
    if(!(*list))
    {
	donemem=ajTrue;
	(*list)=ajListNew();
    }
    
    /* if family is specified then the other fields also have to be specified. */
    if(fam)
    {
	if(!sfam || !fold || !class)
	{
	    ajWarn("Bad arguments passed to ajXyzHitlistExtract\n");
	    if(donemem)
		ajListDel(&(*list));
	    return ajFalse;
	}
	else
	    ajXyzExtractFam(scopf,fam,sfam,fold,class,list);
    }

    /* if superfamily is specified then the other fields also have to be specified. */
    else if(sfam)
    {
	if(!fold || !class)
	{
	    ajWarn("Bad arguments passed to ajXyzHitlistExtract\n");
	    if(donemem)
		ajListDel(&(*list));
	    return ajFalse;
	}
	else
	    ajXyzExtractSfam(scopf,fam,sfam,fold,class,list);
    }
    
    /* if fold is specified then the other fields also have to be specified. */
    else if(fold)
    {
	if(!class)
	{
	    ajWarn("Bad arguments passed to ajXyzHitlistExtract\n");
	    if(donemem)
		ajListDel(&(*list));
	    return ajFalse;
	}
	else
	    ajXyzExtractFold(scopf,fam,sfam,fold,class,list);
    } 

    else
    {
	ajWarn("Bad arguments passed to ajXyzHitlistExtract\n");
	if(donemem)
	    ajListDel(&(*list));
	return ajFalse;
    }
    
    return ajTrue;
}



/* @func ajXyzExtractFam ********************************************************
**
** Reads a scopfamilies file into a hitlist structure and selects 
** the entries with the specified family and create a list of these structures.
**
** @param [r] scopf     [AjPFile *]       The scop families file.
** @param [r] fam       [AjPStr  *]       Family
** @param [r] sfam      [AjPStr  *]       Superfamily
** @param [r] fold      [AjPStr  *]       Fold
** @param [w] list      [AjPList *]       A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
********************************************************************************/

AjBool ajXyzExtractFam(AjPFile *scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class, AjPList* list)
{
    AjPHitlist hitlist = NULL; 

    /* if family is specified then the other fields also have to be specified. */
    /* check that the other fields are populated */ 
    if(!fam || !sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to ajXyzExtractFam\n");
	return ajFalse;
    }
    

    while(ajXyzHitlistRead(*scopf,"//",&hitlist))
    {
	if(ajStrMatch(fam,hitlist->Family) &&
	   ajStrMatch(sfam,hitlist->Superfamily) &&
	   ajStrMatch(fold,hitlist->Fold) &&
	   ajStrMatch(class,hitlist->Class))
	    ajListPushApp(*list,hitlist);
    }
    
    return ajTrue;
}


/* @func ajXyzExtractSfam ***************************************************
**
** Reads a scopfamilies file into a hitlist structure and selects 
** the entries with the specified superfamily and create a list of these 
** structures.
**
** @param [r] scopf     [AjPFile *]       The scop families file.
** @param [r] fam       [AjPStr  *]       Family
** @param [r] sfam      [AjPStr  *]       Superfamily
** @param [r] fold      [AjPStr  *]       Fold
** @param [w] list      [AjPList *]       A list of hitlist structures.
** 
** @return [AjBool] True on success (a file has been written)
** @@
******************************************************************************/

AjBool ajXyzExtractSfam(AjPFile *scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class, AjPList* list)
{
    AjPHitlist hitlist = NULL; 
    
    /* if family is specified then the other fields also have to be specified. */
    /* check that the other fields are populated */ 
    if(!sfam || !fold || !class)
    {
	ajWarn("Bad arguments passed to ajXyzExtractSfam\n");
	return ajFalse;
    }
    
    
    while(ajXyzHitlistRead(*scopf,"//",&hitlist))
    {
	if(ajStrMatch(fam,hitlist->Superfamily) &&
	   ajStrMatch(fold,hitlist->Fold) &&
	   ajStrMatch(class,hitlist->Class))
	    ajListPushApp(*list,hitlist);
    }
    
    return ajTrue;
}


/* @func ajXyzExtractFold ***************************************************
**
** Reads a scopfamilies file into a hitlist structure and selects 
** the entries with the specified fold and create a list of these structures.
**
** @param [r] scopf     [AjPFile *]       The scop families file.
** @param [r] fam       [AjPStr  *]       Family
** @param [r] sfam      [AjPStr  *]       Superfamily
** @param [r] fold      [AjPStr  *]       Fold
** @param [w] hitlist   [AjPHitlist *]    Hitlist object. 
** @param [w] list      [AjPList *]       A list of hitlist structures.
** @@
******************************************************************************/

AjBool ajXyzExtractFold(AjPFile *scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class,AjPList* list)
{
    AjPHitlist hitlist = NULL; 

    /* if family is specified then the other fields also have to be specified. */
    /* check that the other fields are populated */ 
    if(!fold || !class)
    {
	ajWarn("Bad arguments passed to ajXyzExtractFold\n");
	return ajFalse;
    }
    
    while(ajXyzHitlistRead(*scopf,"//",&hitlist))
    {
	if(ajStrMatch(fam,hitlist->Fold) &&
	   ajStrMatch(class,hitlist->Class))
	    ajListPushApp(*list,hitlist);
    }	
    
    return ajTrue;
}


/* @func ajXyzHitlistsToClustal **********************************************
**
** Takes a list of Hitlist structures, converts them into a list of 
** Scophit structures and then writes the sequences to a file in CLUSTAL 
** format.
**
** @param [r] list      [AjPList *]    A list Hitlist structures.
** @param [w] outf      [AjPFile *]    Outfile file pointer
** 
** @return [AjBool] True on success (a file has been written)
** @@
******************************************************************************/

AjBool ajXyzHitlistsToClustal(AjPList *list, AjPFile *outf)
{
    AjPList hitslist = NULL; /* list of populatd scophit objects */ 
    AjIList iter     = NULL; /* a list iterator for hitslist */
    AjPScophit hit   = NULL; /* a scophit object to hold a hit */
    
    /* check arguments */
    if(!(*list) || (!(*outf)))
    {
	ajWarn("Bad arguments\n");
	return ajFalse;
    }
    
    hitslist = ajListNew();
    
    if(ajXyzHitlistToScophits(*list,&hitslist))
    {
	iter = ajListIter(hitslist);
	/* iterate through the list and write out the accession number and sequence to outf in CLUSTAL format. */
	while((hit = (AjPScophit)ajListIterNext(iter)))
	{
	    /* print the accession number and sequence to outfile */
	    ajFmtPrintF(*outf,">%S_%d_%d\n",hit->Id,hit->Start,hit->End);
	    ajFmtPrintF(*outf,"%S\n",hit->Seq);
	}	
	ajListIterFree(iter);
    }		
    
    return ajTrue;
}


/* @func ajXyzWriteClustalAlign **********************************************
**
** Takes a Scopalg object and writes the alignment to a specified file in
** CLUSTAL format.
**
** @param [r] align      [AjPScopalg *]  A list Hitlist structures.
** @param [w] outf       [AjPFile *]     Outfile file pointer
** 
** @return [AjBool] True on success (a file has been written)
** @@
******************************************************************************/

AjBool ajXyzWriteClustalAlign(AjPScopalg align, AjPFile* outf)
{
    ajint i;
    
    /*Check args*/
    if(!align)
    {
	ajWarn("Null args passed to ajXyzWriteClustalAlign ");
	return ajFalse;
    }
    
    /* remove i from the print statement before commiting */
    ajFmtPrintF(*outf,"CLUSTALW\n\n");

    for(i=0;i<align->N;++i)
    	ajFmtPrintF(*outf,"%S_%d   %S\n",align->Codes[i],i,align->Seqs[i]);
    ajFmtPrintF(*outf,"\n");
    
    return ajTrue;
}	






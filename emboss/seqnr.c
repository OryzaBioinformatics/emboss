/* @source seqnr application
 **
 ** Converts redundant embl-format scophit file to non-redundant one
 ** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
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
 */


#include "emboss.h"

/* @prog seqnr ****************************************************************
**
** Converts redundant database results to a non-redundant set of hits
**
******************************************************************************/

int main(int argc, char **argv)
{
    float gapopen;		   /* Gap insertion penalty */
    float gapextend;	           /* Gap extension penalty */
    float thresh;		   /* Threshold for non-redundancy */
    AjPMatrixf matrix;		   /* Substitution matrix */
    
    AjPStr tmp           = NULL;   /* temparary string */
    AjPStr filename      = NULL;   /* the name of the input file that contains his to be processed */
    AjPStr outfilename   = NULL;   /* the name of the output file containing the processed hits */
    AjPStr logf          = NULL;   /* log file pointer */
    
    AjPList list         = NULL;   /* a list to hold the file names */
    AjPList seqlist      = NULL;   /* a list of sequences to be processed */
    AjPList hits         = NULL;   /* a list of scophits from each hitlist iteration that has not been processed */
    AjPList proslist     = NULL;   /* processed hits from which the redundancy has been removed */
    AjPList hitlistin    = NULL;   /* list of hitlists containing hits that are unprocessed */
    
    AjPHitlist tmphitlist= NULL;   /* temparary hitlist structure */
    AjPHitlist hitlistout= NULL;   /* hitlist containing processed hits */
    
    AjPFile inf          = NULL;   /* file containing the results of a psiblasts or swissparse run */
    AjPFile outf         = NULL;   /* the outfile for the processed families */
    
    AjPStr path          = NULL;   /* path to the directory that contain the hits file */
    AjPStr outpath       = NULL;   /* output path directory for the processed hits */
    AjPStr extn          = NULL;   /* extention of the hits file */
    AjPStr outextn       = NULL;   /* output extension for the processed hits */
    
    AjPSeq seq           = NULL;   /* A sequence object to hold the constructed sequence */
    
    AjIList iter         = NULL;   /* A list iterator */
    
    AjPScophit hit       = NULL;   /* temperary scophit */
   
    AjPInt keep          = NULL;   /* 1: This sequence was kept after redundancy removal,
				      0: it was discarded */
    ajint nsetnr         = 0;	   /* No. proteins in the non-redundant set */
    ajint i              = 0;	   /* loop counter */
    ajint posdash        = 0;      /* Position of last '/' in filenames from 'list' */
    ajint posdot         = 0;      /* Position of last '.' in filenames from 'list' */


    /* Read data from acd */
    embInit("seqnr",argc,argv);
    
    thresh    = ajAcdGetFloat("thresh");
    matrix    = ajAcdGetMatrixf("datafile");
    gapopen   = ajAcdGetFloat("gapopen");
    gapextend = ajAcdGetFloat("gapextend");
    path      = ajAcdGetString("path");
    outpath   = ajAcdGetString("outpath");
    extn      = ajAcdGetString("extn");
    outextn   = ajAcdGetString("outextn");
    
    tmp        = ajStrNew();
    filename   = ajStrNew();
    outfilename= ajStrNew();
    logf       = ajStrNew();
    list       = ajListNew();

    
    /* Check directories */
    if((!ajFileDir(&path)) || (!(extn)))
	ajFatal("Could not open psiblast results directory");    
    
    if((!ajFileDir(&outpath)) || (!(outextn)))
	ajFatal("Could not open extended alignment directory or file extension NULL");
    
    /* Create list of files in the path */
    ajStrAssC(&tmp, "*");  		/* assign a wildcard to tmp */
    
    if((ajStrChar(extn, 0)=='.')) 	/* checks if the file extension starts with "." */
	ajStrApp(&tmp, extn);    	/* assign the acd input file extension to tmp */
    
    /* this picks up situations where the user has specified an extension without a "." */
    else
    {
	ajStrAppC(&tmp, ".");       	/* assign "." to tmp */  
	ajStrApp(&tmp, extn);       	/* append tmp with a user specified extension */  
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
	
	/* Add a '.' to outextn if one does not already exist */
	if((ajStrChar(outextn, 0)!='.')) /* checks if the file extension starts with "." */
	    ajStrInsertC(&outextn, 0, ".");
	
	/* Create the name of the output file */
	posdash = ajStrRFindC(filename, "/");
	posdot  = ajStrRFindC(filename, ".");
	
	if(posdash >= posdot)
	    ajFatal("Could not create filename. Email rranasin@hgmp.mrc.ac.uk");
	else
	{
	    ajStrAssSub(&outfilename, filename, posdash+1, posdot-1);
	    ajStrApp(&outfilename,outextn);
	}
	
	/* create the output stream */
	outf = ajFileNewOut(outfilename);

	/* read in each entry delimited by a "//" */
	while(ajXyzHitlistRead(inf,"//",&tmphitlist)) 
	{
	    keep       = ajIntNew();
	    hitlistin  = ajListNew();
	    hits       = ajListNew();
	    seqlist    = ajListNew();
	    proslist   = ajListNew();

	    /* Create list of a single Hitlist from tmphitlist */
	    ajListPushApp(hitlistin,tmphitlist);

	    /* Convert this list to a list of of Scophit objects */
	    ajXyzHitlistToScophits(hitlistin,&hits); 

	    /* Delete original Hitlist and the derived list
	     We now just have a list of Scophit's called <hits> */
	    ajXyzHitlistDel(&tmphitlist);
	    ajListDel(&hitlistin);

	    iter      = ajListIter(hits);	
	    while((hit=(AjPScophit)ajListIterNext(iter)))
	    {
		seq = ajSeqNew();
		ajStrAss(&seq->Name,hit->Id);
		ajStrAss(&seq->Seq,hit->Seq);
		ajListPushApp(seqlist,seq);
	    }
	    ajListIterFree(iter);

	    /*remove the redundancy from the sequence set */
	    embXyzSeqsetNR(seqlist, &keep, &nsetnr, matrix, gapopen, gapextend, thresh);
	    
	    /* create a list of processed hits */
	    for(iter = ajListIter(hits), i = 0;(hit = (AjPScophit)ajListIterNext(iter));i++)
	    {
		if(ajIntGet(keep,i))
		    ajListPushApp(proslist,hit);
		else
		    ajXyzScophitDel(&hit);
	    }
	    ajListDel(&hits);
	    ajListIterFree(iter);
	    iter = NULL;
	    
	    /* write a hitlist of the processed scophits */
	    ajXyzScophitsToHitlist(proslist,&hitlistout,&iter);
	    
	    /* delete and clean up proslist  */
	    iter=ajListIter(proslist);
	    while((hit=(AjPScophit)ajListIterNext(iter)))
		ajXyzScophitDel(&hit);
	    ajListIterFree(iter);
	    ajListDel(&proslist);

	    /* write the processed hitlist in a file in EMBL format */
	    ajXyzHitlistWrite(outf,hitlistout);
	    ajXyzHitlistDel(&hitlistout);
	    hitlistout = NULL;

	    /*delete and clean up seqlist */
	    iter=ajListIter(seqlist);
	    while((seq=(AjPSeq)ajListIterNext(iter)))
		ajSeqDel(&seq);
	    ajListIterFree(iter);
	    ajListDel(&seqlist);


	    /* clean up */
	    ajIntDel(&keep);
	}
	
	ajFileClose(&inf);
	ajFileClose(&outf);
    }
    
    /* clean up */
    ajMatrixfDel(&matrix);
    ajStrDel(&path);
    ajStrDel(&outpath);
    ajStrDel(&extn);
    ajStrDel(&outextn);
    ajStrDel(&tmp);
    ajStrDel(&filename);
    ajStrDel(&outfilename);
    ajStrDel(&logf);
    ajListDel(&list);

    ajExit();
    return 0;

}













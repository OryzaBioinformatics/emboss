/* @source est2genome application
**
** Gribskov statistical plot of synonymous codon usage
**
** @author: Copyright (C) Peter Rice
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
******************************************************************************/
/*
 * Revision 1.7 1997/03/17   15:26:00  pmr
 * when EST is reversed, need to reverse the EST sequence positions
 * also cleaned up ajlong code lines

 * Revision 1.6  1997/02/10  14:07:54  rmott
 * fixed bug so that splice sites in REVERSE direction
 * (ie ct/ac rather than gt/ag) are found correctly.
 * Output modified so that splice direction is written
 *
 * Revision 1.4  1997/01/30  17:21:22  rmott
 * fixed bug, and now computes area limit better
 *
 * Revision 1.3  1997/01/30  17:03:45  rmott
 * debugged version with debug statements still in.
 * Fixed problem of not initialising edge properly
 *
 * Revision 1.2  1996/08/07  10:12:05  rmott
 
 * Linear-Space version
 *
   * Revision 1.1  1996/08/01  13:55:42  rmott
   * Initial revision
   *
   
   */

#include "emboss.h"
#include "embest.h"
#include <math.h>

extern ajint lsimmat[256][256];

#define BOTH 0
#define FORWARD_ONLY 1
#define REVERSE_ONLY 2

ajint verbose;
ajint debug;

static void  est2genome_make_output( AjPFile ofile, AjPSeq genome, AjPSeq est,
				    EmbPEstAlign ge, ajint match,
				    ajint mismatch, ajint gap_penalty,
				    ajint intron_penalty,
				    ajint splice_penalty, ajint minscore,
				    ajint align, ajint width, ajint reverse);



int main(int argc, char **argv)
{
    AjPSeq genome;
    AjPSeq splice_sites, reversed_splice_sites;
    AjPSeq est, reversed_est=NULL;
    EmbPEstAlign fge=NULL, rge=NULL, bge=NULL;
    ajint width=50;
    ajint match=1;
    ajint mismatch=1;
    ajint gap_penalty=2;
    ajint intron_penalty=40;
    ajint splice_penalty=20;
    ajint splice=1;
    ajint align=0;
    ajint reverse=0;
    ajint isreverse=0;
    ajint doreverse=0;		/* zero for first inclusion, set to 1 later */
    float megabytes=10.0;
    ajint minscore=30;
    ajint shuffles=0, max_score=0;
    ajint seed;
    ajint best=1;
    ajint search_mode;
    char *mode;
    AjPStr modestr;
    AjPFile outfile;
    AjPSeqall estset;
    
    /* the fasta input files */
    
    embInit ("est2genome", argc, argv);
    
    estset = ajAcdGetSeqall ("est");
    genome = ajAcdGetSeq ("genome");
    outfile = ajAcdGetOutfile ("outfile");
    
    /* the alignment penalties */
    
    match = ajAcdGetInt ("match");
    mismatch = ajAcdGetInt ("mismatch");
    gap_penalty  = ajAcdGetInt ("gappenalty");
    intron_penalty  = ajAcdGetInt ("intronpenalty");
    splice_penalty  = ajAcdGetInt ("splicepenalty");
    doreverse  = ajAcdGetBool ("reverse");
    
    /* the min score for an alignment to be output */
    minscore = ajAcdGetInt ("minscore");
    
    if (doreverse)
	isreverse = 1;
    
    splice = ajAcdGetBool ("splice");
    
    /* Print the alignemt */
    align = ajAcdGetBool ("align");
    width = ajAcdGetInt ("width");
    
    /* mode: This is complicated.
       
       "forward"   just search forward strands of both sequences
       "reverse"   just search forward of genomic vs reverse of est
       "both"      search forward strand of genomic against forward and
                   reverse 
       THEN: take the best of these two, and re-align assuming 
       a reversed gene so that the splice sites would be appear 
       as ct/ac. Only output the best alignment unless the
       flag -nobest is set.
       
       Thus THREE alignments are made.
       
       The output cordinates are such that the genomic sequence  
       is always in the forward direction.
       
       */
		   
    modestr = ajAcdGetString("mode");
    mode = ajStrStr(modestr);

    if ( !strcmp(mode,"both")  ) 
	search_mode = BOTH;
    else if (!strcmp(mode,"forward") )
	search_mode = FORWARD_ONLY;
    else if (!strcmp(mode,"reverse") )
	search_mode = REVERSE_ONLY;
    else
    {
	ajErr ("search mode  %s must be one of: "
	       "both, forward, reverse\n", mode );
	exit(1);
    }
    
    /* just print the best alignment ? */
    
    best = ajAcdGetBool("best");
    
    /* max space in megabytes */
    
    megabytes = ajAcdGetFloat("space");
    
    /* print debugging info */
    
    verbose = ajAcdGetBool ("verbose");
    debug = ajAcdGetBool ("debug");
    
    if (verbose) ajDebug ("debugging set to %d\n", debug);
    
    if (verbose)
	embEstSetVerbose();
    if (debug)
	embEstSetDebug();

    /* shuffle the sequences to test for statistical
       significance this many times */

    shuffles = ajAcdGetInt("shuffle");
    seed = ajAcdGetInt("seed");

    seed = ajAcdGetInt("seed");
    if (!seed)
	seed = embEstGetSeed();
    seed = -seed;

    if ( mismatch < 0 )
	mismatch = -mismatch;
    if ( gap_penalty < 0 )
	gap_penalty = -gap_penalty;
    if ( intron_penalty < 0 )
	intron_penalty = -intron_penalty;
    if ( splice_penalty < 0 )
	splice_penalty = -splice_penalty;
    
    embEstMatInit( match, mismatch, gap_penalty, 0, '-' );
    
    if ( ajSeqLen(genome) )
    {
	/* Make sure we have enough space to hold the genomic AjPSeq */
	
	if ( megabytes < ajSeqLen(genome)*1.5e-6 ) 
	{
	    ajWarn ("increasing space from %.3f to %.3f Mb\n",
		    megabytes, 1.5e-6*ajSeqLen(genome) );
	    megabytes = 1.5e-6*ajSeqLen(genome);
	}
	
	/* find the GT/AG splice sites */
	
	if ( splice )
	    splice_sites = embEstFindSpliceSites( genome, 1 );
	else
	    splice_sites = NULL;
	
	if ( search_mode == BOTH )
	    reversed_splice_sites = embEstFindSpliceSites( genome, 0 );
	else
	    reversed_splice_sites = NULL;
	
	/* process each est */
	
	while (ajSeqallNext(estset, &est))
	{

	    /* if required, make shuffled comparisons
	       to get statistical significance */

	    ajDebug("shuffles: %d\n", shuffles);
	    if ( shuffles > 0 )
	    {
		AjPSeq shuffled_est = ajSeqNewS(est);
		ajint n;
		ajint score;
		double mean=0, std=0;
		EmbPEstAlign sge;

		for(n=0;n<shuffles;n++)
		{
		    embEstShuffleSeq( shuffled_est, 1, &seed );
		    sge = embEstAlignNonRecursive( shuffled_est,
						  genome, match, mismatch,
						  gap_penalty,
						  intron_penalty,
						  splice_penalty,
						  splice_sites, 0, 0,
						  DIAGONAL );  
		    score = sge->score;
		    ajDebug("%30.30S\n", ajSeqStr(shuffled_est) );
		    ajDebug("%5d score %d seed %d\n", n, score, seed );
		    if ( score > max_score ) 
			max_score = score;
		    mean += score;
		    std += score*score;
		    embEstFreeAlign(&sge);
		}
	      
		mean /= shuffles;
		std = sqrt( (std = shuffles*mean*mean)/(shuffles-1.0) );
		ajDebug ("shuffles: %d max: %d mean: %.2f std dev: %.2f\n",
			 shuffles, max_score, mean, std );
		minscore = max_score+1; 
		ajSeqDel(&shuffled_est);
	    }
		  
	    if ( search_mode != REVERSE_ONLY ) { /* forward strand */
		fge = embEstAlignLinearSpace ( est, genome, match,
					      mismatch, gap_penalty,
					      intron_penalty, splice_penalty,
					      splice_sites, megabytes );
		if (!fge)
		    ajFatal ("forward strand alignment failed");
	    }
	    else 
		fge = NULL;

	    if ( search_mode != FORWARD_ONLY ) /* reverse strand */
	    {
		reversed_est = ajSeqNewS(est);
		ajSeqReverse(reversed_est);

		rge = embEstAlignLinearSpace ( reversed_est, genome,
					      match, mismatch, gap_penalty,
					      intron_penalty, splice_penalty,
					      splice_sites, megabytes );
		if (!rge)
		    ajFatal ("reverse strand alignment failed");
	    }
	    else
		rge = NULL;

	    if ( search_mode == BOTH )	/* search both strands */
	    {
		if ( fge->score > rge->score ) 
		{			/* redo forward search with
					   reversed splice sites */
		    bge = embEstAlignLinearSpace ( est, genome, match,
						  mismatch, gap_penalty,
						  intron_penalty,
						  splice_penalty,
						  reversed_splice_sites,
						  megabytes );
		    if (  bge->score > fge->score ) /* probably have a
						       reversed gene */
		    {
			ajFmtPrintF(outfile,
				    "Note Best alignment is between forward "
				    "est and forward genome, but splice  "
				    "sites imply REVERSED GENE\n");
			est2genome_make_output(outfile, genome, est, bge,
					       match, mismatch, gap_penalty,
					       intron_penalty, splice_penalty,
					       minscore, align, width,
					       reverse);
			if ( best == 0 ) /* print substandard alignment too */
			    est2genome_make_output(outfile, genome, est, fge,
						   match, mismatch,
						   gap_penalty, intron_penalty,
						   splice_penalty, minscore,
						   align, width, reverse);
		    }
		    else
		    {
			ajFmtPrintF(outfile,
				    "Note Best alignment is between forward "
				    "est and forward genome, and splice  "
				    "sites imply forward gene\n");
			est2genome_make_output(outfile, genome, est, fge,
					       match, mismatch,
					       gap_penalty, intron_penalty,
					       splice_penalty, minscore,
					       align, width, reverse);
			if ( best == 0 )
			    est2genome_make_output(outfile, genome, est, bge,
						   match, mismatch,
						   gap_penalty, intron_penalty,
						   splice_penalty, minscore,
						   align, width, reverse);
		    }
		}
		else 
		{
		    bge = embEstAlignLinearSpace ( reversed_est,genome,
						  match, mismatch,
						  gap_penalty,
						  intron_penalty,
						  splice_penalty,
						  reversed_splice_sites,
						  megabytes );
		    if (  bge->score > rge->score ) /* probably have a
						       reversed gene */
		    {
			ajFmtPrintF(outfile,
				    "Note Best alignment is between "
				    "reversed est and forward genome, but "
				    "splice sites imply REVERSED GENE\n");
			est2genome_make_output(outfile, genome, reversed_est,
					       bge, match, mismatch,
					       gap_penalty, intron_penalty,
					       splice_penalty, minscore,
					       align, width, isreverse);
			if ( best == 0 ) /* print substandard alignment too */
			    est2genome_make_output(outfile, genome,
						   reversed_est, rge, match,
						   mismatch, gap_penalty,
						   intron_penalty,
						   splice_penalty, minscore,
						   align, width, isreverse);
		    }
		    else
		    {
			ajFmtPrintF(outfile,
				    "Note Best alignment is between reversed"
				    " est and forward genome, and splice  "
				    "sites imply forward gene\n");
			est2genome_make_output(outfile, genome, reversed_est,
					       rge, match, mismatch,
					       gap_penalty, intron_penalty,
					       splice_penalty, minscore,
					       align, width, isreverse);
			if ( best == 0 )
			    est2genome_make_output(outfile, genome,
						   reversed_est, bge, match,
						   mismatch, gap_penalty,
						   intron_penalty,
						   splice_penalty, minscore,
						   align, width, isreverse);
		    }
		}
		
		
	    }
	    embEstFreeAlign(&bge);
	    embEstFreeAlign(&rge);
	    embEstFreeAlign(&fge);
	    
	    ajSeqDel(&est);
	    ajSeqDel(&reversed_est);
	}
	
	if ( splice_sites )
	    ajSeqDel( &splice_sites );
	
	if ( reversed_splice_sites ) 
	    ajSeqDel( &reversed_splice_sites );
	
	ajSeqDel(&genome); 
	return 0;
    }
    
    return 1;
}


/* @funcstatic est2genome_make_output *****************************************
**
** Undocumented.
**
** @param [?] ofile [AjPFile] Undocumented
** @param [?] genome [AjPSeq] Undocumented
** @param [?] est [AjPSeq] Undocumented
** @param [?] ge [EmbPEstAlign] Undocumented
** @param [?] match [ajint] Undocumented
** @param [?] mismatch [ajint] Undocumented
** @param [?] gap_penalty [ajint] Undocumented
** @param [?] intron_penalty [ajint] Undocumented
** @param [?] splice_penalty [ajint] Undocumented
** @param [?] minscore [ajint] Undocumented
** @param [?] align [ajint] Undocumented
** @param [?] width [ajint] Undocumented
** @param [?] reverse [ajint] Undocumented
** @@
******************************************************************************/


static void est2genome_make_output( AjPFile ofile, AjPSeq genome, AjPSeq est,
				   EmbPEstAlign ge, ajint match,
				   ajint mismatch, ajint gap_penalty,
				   ajint intron_penalty, ajint splice_penalty,
				   ajint minscore, ajint align, ajint width,
				   ajint reverse)
{

    if ( ge->score >= minscore )
    {
	embEstOutBlastStyle( ofile, genome, est, ge,
			    match, mismatch, gap_penalty,
			    intron_penalty, splice_penalty, 1, reverse );
	ajFmtPrintF( ofile, "\n");
	embEstOutBlastStyle ( ofile, genome, est, ge,
			     match, mismatch, gap_penalty,
			     intron_penalty, splice_penalty, 0, reverse );
	      
	if ( align )
	{
	    ajFmtPrintF(ofile, "\n\n%s vs %s:\n",
			ajSeqName(genome), ajSeqName(est) );
	    embEstPrintAlign( ofile, genome, est, ge, width );
	}
    }

    return;
}

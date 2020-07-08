/*  Last edited: Feb 25 14:16 2000 (pmr) */
/*
 * Revision 1.7 1997/03/17   15:26:00  pmr
 * when EST is reversed, need to reverse the EST sequence positions
 * also cleaned up long code lines

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

extern int lsimmat[256][256];

#define BOTH 0
#define FORWARD_ONLY 1
#define REVERSE_ONLY 2

extern int verbose;
extern int debug;
extern int indentation;

void 
make_output( AjPFile ofile, SEQUENCE *genome, SEQUENCE *est, ge_alignment *ge,
	    int match, int mismatch, int gap_penalty, int intron_penalty,
	    int splice_penalty, int minscore, int align, int width,
	    int reverse);

int 
main( int argc, char *argv[])
{
  SEQUENCE *genome;
  SEQUENCE *splice_sites, *reversed_splice_sites;
  SEQUENCE *est, *reversed_est=NULL;
  ge_alignment *fge=NULL, *rge=NULL, *bge=NULL;
  int width=50;
  int match=1;
  int mismatch=1;
  int gap_penalty=2;
  int intron_penalty=40;
  int splice_penalty=20;
  int splice=1;
  int align=0;
  int reverse=0;
  int isreverse=0;
  int doreverse=0;		/* zero for first inclusion, set to 1 later */
  float megabytes=10.0;
  int minscore=30;
  int shuffles=0, max_score=0;
  int seed;
  int best=1;
  int search_mode;
  char *mode;
  AjPStr modestr;
  AjPFile outfile;
  AjPSeqall estset;
  AjPSeq genomeseq;

  /* the fasta input files */

  embInit ("est2genome", argc, argv);

  estset = ajAcdGetSeqall ("est");
  genomeseq = ajAcdGetSeq ("genome");
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
     "both"      search forward strand of genomic against forward and reverse 
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

  /* shuffle the sequences to test for statistical
     significance this many times */

  shuffles = ajAcdGetInt("shuffle");
  seed = ajAcdGetInt("seed");

  getseed(&seed,argc,argv); /* random number seed */
  seed = -seed;

  if ( mismatch < 0 )
    mismatch = -mismatch;
  if ( gap_penalty < 0 )
    gap_penalty = -gap_penalty;
  if ( intron_penalty < 0 )
    intron_penalty = -intron_penalty;
  if ( splice_penalty < 0 )
    splice_penalty = -splice_penalty;

  matinit( match, mismatch, gap_penalty, 0, '-' );

  genome = seq_to_sequence(genomeseq);
  if ( genome )
    {
      /* Make sure we have enough space to hold the genomic sequence */

      if ( megabytes < genome->len*1.5e-6 ) 
	{
	  ajWarn ("increasing space from %.3f to %.3f Mb\n",
		  megabytes, 1.5e-6*genome->len );
	  megabytes = 1.5e-6*genome->len;
	}

      /* find the GT/AG splice sites */

      if ( splice )
	splice_sites = find_splice_sites( genome, 1 );
      else
	splice_sites = NULL;

      if ( search_mode == BOTH )
	reversed_splice_sites = find_splice_sites( genome, 0 );
      else
	reversed_splice_sites = NULL;

      /* process each est */
      
      while ( (est = get_next_sq( estset )) )
	{

	  /* if required, make shuffled comparisons
             to get statistical significance */

	  ajDebug("shuffles: %d\n", shuffles);
	  if ( shuffles > 0 )
	    {
	      SEQUENCE *shuffled_est = seqdup( est );
	      int n;
	      int score;
	      double mean=0, std=0;
	      ge_alignment *sge;

	      for(n=0;n<shuffles;n++)
		{
		  shuffle_seq( shuffled_est, 1, &seed );
		  sge = non_recursive_est_to_genome( shuffled_est,
						    genome, match, mismatch,
						    gap_penalty,
						    intron_penalty,
						    splice_penalty,
						    splice_sites, 0, 0,
						    DIAGONAL );  
		  score = sge->score;
		  ajDebug("%30.30s\n", shuffled_est->s );
		  ajDebug("%5d score %d seed %d\n", n, score, seed );
		  if ( score > max_score ) 
		    max_score = score;
		  mean += score;
		  std += score*score;
		  free_ge(sge);
		}
	      
	      mean /= shuffles;
	      std = sqrt( (std = shuffles*mean*mean)/(shuffles-1.0) );
	      ajDebug ("shuffles: %d max: %d mean: %.2f std dev: %.2f\n",
		      shuffles, max_score, mean, std );
	      minscore = max_score+1; 
	      free_seq(shuffled_est);
	    }
		  
	  if ( search_mode != REVERSE_ONLY ) { /* forward strand */
	    fge = linear_space_est_to_genome( est, genome, match,
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
	      reversed_est = seqdup(est);
	      complement_seq(reversed_est->s);

	      rge = linear_space_est_to_genome( reversed_est, genome,
					       match, mismatch, gap_penalty,
					       intron_penalty, splice_penalty,
					       splice_sites, megabytes );
	      if (!rge)
		ajFatal ("reverse strand alignment failed");
	    }
	  else
	    rge = NULL;

	  if ( search_mode == BOTH ) /* search both strands */
	    {
	      if ( fge->score > rge->score ) 
		{ 		  /* redo forward search with
				     reversed splice sites */
		  bge = linear_space_est_to_genome( est, genome, match,
						   mismatch, gap_penalty,
						   intron_penalty,
						   splice_penalty,
						   reversed_splice_sites,
						   megabytes );
		  if (  bge->score > fge->score ) /* probably have a
						     reversed gene */
		    {
		      ajFmtPrintF(outfile,
			      "Note Best alignment is between forward est "
			      "and forward genome, but splice  sites imply "
			      "REVERSED GENE\n");
		      make_output(outfile, genome, est, bge, match, mismatch,
				  gap_penalty, intron_penalty, splice_penalty,
				  minscore, align, width, reverse);
		      if ( best == 0 ) /* print substandard alignment too */
			make_output(outfile, genome, est, fge, match,
				    mismatch, gap_penalty, intron_penalty,
				    splice_penalty, minscore, align, width,
				    reverse);
		    }
		  else
		    {
		      ajFmtPrintF(outfile,
		             "Note Best alignment is between forward est and "
			     "forward genome, and splice  sites imply forward "
			     "gene\n");
		      make_output(outfile, genome, est, fge, match, mismatch,
				  gap_penalty, intron_penalty, splice_penalty,
				  minscore, align, width, reverse);
		      if ( best == 0 )
			make_output(outfile, genome, est, bge, match, mismatch,
				    gap_penalty, intron_penalty,
				    splice_penalty, minscore, align, width,
				    reverse);
		    }
		}
	      else 
		{
		  bge = linear_space_est_to_genome( reversed_est,genome,
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
		      make_output(outfile, genome, reversed_est, bge, match,
				  mismatch, gap_penalty, intron_penalty,
				  splice_penalty, minscore, align, width,
				  isreverse);
		      if ( best == 0 ) /* print substandard alignment too */
			make_output(outfile, genome, reversed_est, rge,
				    match, mismatch, gap_penalty,
				    intron_penalty, splice_penalty,
				    minscore, align, width, isreverse);
		    }
		  else
		    {
		      ajFmtPrintF(outfile,
		             "Note Best alignment is between reversed est "
			     "and forward genome, and splice  sites imply "
			     "forward gene\n");
		      make_output(outfile, genome, reversed_est, rge, match,
				  mismatch, gap_penalty, intron_penalty,
				  splice_penalty, minscore, align, width,
				  isreverse);
		      if ( best == 0 )
			make_output(outfile, genome, reversed_est, bge, match,
				    mismatch, gap_penalty, intron_penalty,
				    splice_penalty, minscore, align, width,
				    isreverse);
		    }
		}


	    }
	  if ( bge ) {
	    free_ge(bge);
	    bge = NULL;
	  }
	  if ( rge ) {
	    free_ge(rge);
	    rge = NULL;
	  }
	  if ( fge ) {
	    free_ge(fge);
	    fge = NULL;
	  }

	  free_seq_copy(est);
	  if (reversed_est) 
	    free_seq(reversed_est);
	}

      if ( splice_sites )
	free_seq( splice_sites );

      if ( reversed_splice_sites ) 
	free_seq( reversed_splice_sites );

      free_seq_copy(genome); 
      return 0;
    }
  return 1;
}



void 
make_output( AjPFile ofile, SEQUENCE *genome, SEQUENCE *est, ge_alignment *ge,
	    int match, int mismatch, int gap_penalty, int intron_penalty,
	    int splice_penalty, int minscore, int align, int width,
	    int reverse)
{
  if ( ge->score >= minscore )
    {
      blast_style_output( ofile, genome, est, ge, match, mismatch, gap_penalty,
			 intron_penalty, splice_penalty, 1, reverse );
      ajFmtPrintF( ofile, "\n");
      blast_style_output( ofile, genome, est, ge, match, mismatch, gap_penalty,
			 intron_penalty, splice_penalty, 0, reverse );
	      
      if ( align )
	{
	  ajFmtPrintF(ofile, "\n\n%s vs %s:\n", genome->name, est->name );
	  print_align( ofile, genome, est, ge, width );
	}
    }
}

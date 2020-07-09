/* @source primer3 application
** Picks PCR primers and hybridization oligos
** 
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 5 Nov 2001 - GWW - written
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
*****************************************************************************/

#include "emboss.h"

/* estimate size of a sequence's output table */
#define TABLEGUESS 150

/* @prog primer3 **************************************************************
**
** EMBOSS wrapper for the Whitehead's primer3 program
**
******************************************************************************/

/* stuff for fork/exec/pipe etc.*/
#include <unistd.h>
#include <stdio.h>

static FILE * primer3_start_write(int fd);
static void primer3_write(AjPStr str, FILE *stream);
static void primer3_end_write(FILE *stream);
static AjPStr primer3_read(int fd);
static void primer3_send_range(FILE * stream, char * tag, AjPRange value);
static void primer3_send_range2(FILE * stream, char * tag, AjPRange value);
static void primer3_send_int(FILE * stream, char * tag, ajint value);
static void primer3_send_float(FILE * stream, char * tag, float value);
static void primer3_send_bool(FILE * stream, char * tag, AjBool value);
static void primer3_send_string(FILE * stream, char * tag, AjPStr value);
static void primer3_send_stringC(FILE * stream, char * tag, const char * value);
static void primer3_send_end(FILE * stream);
static void primer3_report (AjPFile outfile, AjPStr output, ajint numreturn);
static void primer3_output_report(AjPFile outfile, AjPTable table, 
	ajint numreturn);
static AjPStr primer3_tableget(char * key1, ajint number, char *key2, 
	AjPTable table);
static void primer3_write_primer(AjPFile outfile, char *tag, AjPStr pos, 
	AjPStr tm, AjPStr gc, AjPStr seq, AjBool rev);

	
int main(int argc, char **argv, char **env)
{

/* Global details */
  AjBool	explain_flag;
  AjBool	file_flag;
  AjPStr *	task;
  ajint		num_return;
  ajint		first_base_index;

/* "Sequence" Input Tags */
  AjPSeqall	sequence;
  AjPRange	included_region;
  AjPRange	target;
  AjPRange	excluded_region;
  AjPStr	left_input;
  AjPStr	right_input;

/* Primer details */
  AjBool	pick_anyway;
  AjPFile	mispriming_library;
  float		max_mispriming;
  float		pair_max_mispriming;
  ajint		gc_clamp;
  ajint		opt_size;
  ajint		min_size;
  ajint		max_size;
  float		opt_tm;
  float		min_tm;
  float		max_tm;
  float		max_diff_tm;
  float		opt_gc_percent;
  float		min_gc;
  float		max_gc;
  float		salt_conc;
  float		dna_conc;
  ajint		num_ns_accepted;
  float		self_any;
  float		self_end;
  ajint		max_poly_x;
  
/* Sequence Quality */
/* these are not (yet) implemented */
/*
  AjPFile	sequence_quality;
  ajint		min_quality;
  ajint		min_end_quality;
  ajint		quality_range_min;
  ajint		quality_range_max;
*/

/* Product details */
  ajint		product_opt_size;
  AjPRange	product_size_range;
  float		product_opt_tm;
  float		product_min_tm;
  float		product_max_tm;

/* Objective Function Penalty Weights for Primers */
  float		max_end_stability;
/* these are not (yet) implemented */
/*
  float		inside_penalty;
  float		outside_penalty;
*/

/* Primer penalties */
/* these are not (yet) implemented */

/* Internal Oligo "Sequence" Input Tags */
  AjPRange	internal_oligo_excluded_region;

/* Internal Oligo "Global" Input Tags */
  AjPStr	internal_oligo_input;
  ajint		internal_oligo_opt_size;
  ajint		internal_oligo_min_size;
  ajint		internal_oligo_max_size;
  float		internal_oligo_opt_tm;
  float		internal_oligo_min_tm;
  float		internal_oligo_max_tm;
  float		internal_oligo_opt_gc_percent;
  float		internal_oligo_min_gc;
  float		internal_oligo_max_gc;
  float		internal_oligo_salt_conc;
  float		internal_oligo_dna_conc;
  float		internal_oligo_self_any;
  float		internal_oligo_self_end;
  ajint		internal_oligo_max_poly_x;
  AjPFile	internal_oligo_mishyb_library;
  float		internal_oligo_max_mishyb;
/*
  ajint		internal_oligo_min_quality;
*/  
  
/* Internal Oligo penalties */
/* these are not (yet) implemented */

/* EMBOSS-wrapper-specific stuff */
  AjPFile	outfile;
  

/* other variables */
  AjPStr    result = NULL;
  AjPStr    strand = NULL;
  AjPStr    substr = NULL;
  AjPSeq    seq = NULL;
  ajint     begin;
  ajint     end;
  FILE *    stream;
  AjPStr    taskstr = NULL;
  AjPStr    program  = NULL;

/* fork/pipe variables */
  pid_t nPid;
  int pipeto[2];      /* pipe to feed the exec'ed program input */
  int pipefrom[2];    /* pipe to get the exec'ed program output */



  embInit("primer3", argc, argv);


/* Global details */
  explain_flag        = ajAcdGetBool("explainflag");
  file_flag           = ajAcdGetBool("fileflag");
  task                = ajAcdGetList("task");
  num_return          = ajAcdGetInt("numreturn");
  first_base_index    = ajAcdGetInt("firstbaseindex");

/* "Sequence" Input Tags */
  sequence            = ajAcdGetSeqall("sequence");
  included_region     = ajAcdGetRange("includedregion");
  target              = ajAcdGetRange("target");
  excluded_region     = ajAcdGetRange("excludedregion");
  left_input          = ajAcdGetString("forwardinput");
  right_input         = ajAcdGetString("reverseinput");

/* Primer details */
  pick_anyway         = ajAcdGetBool("pickanyway");
  mispriming_library  = ajAcdGetInfile("mispriminglibrary");
  max_mispriming      = ajAcdGetFloat("maxmispriming");
  pair_max_mispriming = ajAcdGetFloat("pairmaxmispriming");
  gc_clamp            = ajAcdGetInt("gcclamp");
  opt_size            = ajAcdGetInt("osize");
  min_size            = ajAcdGetInt("minsize");
  max_size            = ajAcdGetInt("maxsize");
  opt_tm              = ajAcdGetFloat("otm");
  min_tm              = ajAcdGetFloat("mintm");
  max_tm              = ajAcdGetFloat("maxtm");
  max_diff_tm         = ajAcdGetFloat("maxdifftm");
  opt_gc_percent      = ajAcdGetFloat("ogcpercent");
  min_gc              = ajAcdGetFloat("mingc");
  max_gc              = ajAcdGetFloat("maxgc");
  salt_conc           = ajAcdGetFloat("saltconc");
  dna_conc            = ajAcdGetFloat("dnaconc");
  num_ns_accepted     = ajAcdGetInt("numnsaccepted");
  self_any            = ajAcdGetFloat("selfany");
  self_end            = ajAcdGetFloat("selfend");
  max_poly_x          = ajAcdGetInt("maxpolyx");
  
/* Sequence Quality */
/* these are not (yet) implemented */
/*
  sequence_quality    = ajAcdGetInfile("sequencequality");
  min_quality         = ajAcdGetInt("minquality");
  min_end_quality     = ajAcdGetInt("minendquality");
  quality_range_min   = ajAcdGetInt("qualityrangemin");
  quality_range_max   = ajAcdGetInt("qualityrangemax");
*/

/* Product details */
  product_opt_size    = ajAcdGetInt("productosize");
  product_size_range  = ajAcdGetRange("productsizerange");
  product_opt_tm      = ajAcdGetFloat("productotm");
  product_min_tm      = ajAcdGetFloat("productmintm");
  product_max_tm      = ajAcdGetFloat("productmaxtm");

/* Objective Function Penalty Weights for Primers */
  max_end_stability   = ajAcdGetFloat("maxendstability");
/* these are not (yet) implemented */
/*
  inside_penalty      = ajAcdGetFloat("insidepenalty");
  outside_penalty     = ajAcdGetFloat("outsidepenalty");
*/

/* Primer penalties */
/* these are not (yet) implemented */

/* Internal Oligo "Sequence" Input Tags */
  internal_oligo_excluded_region = ajAcdGetRange("oligoexcludedregion");
  internal_oligo_input          = ajAcdGetString("oligoinput");
  
/* Internal Oligo "Global" Input Tags */
  internal_oligo_opt_size       = ajAcdGetInt("oligoosize");
  internal_oligo_min_size       = ajAcdGetInt("oligominsize");
  internal_oligo_max_size       = ajAcdGetInt("oligomaxsize");
  internal_oligo_opt_tm         = ajAcdGetFloat("oligootm");
  internal_oligo_min_tm         = ajAcdGetFloat("oligomintm");
  internal_oligo_max_tm         = ajAcdGetFloat("oligomaxtm");
  internal_oligo_opt_gc_percent = ajAcdGetFloat("oligoogcpercent");
  internal_oligo_min_gc         = ajAcdGetFloat("oligomingc");
  internal_oligo_max_gc         = ajAcdGetFloat("oligomaxgc");
  internal_oligo_salt_conc      = ajAcdGetFloat("oligosaltconc");
  internal_oligo_dna_conc       = ajAcdGetFloat("oligodnaconc");
  internal_oligo_self_any       = ajAcdGetFloat("oligoselfany");
  internal_oligo_self_end       = ajAcdGetFloat("oligoselfend");
  internal_oligo_max_poly_x     = ajAcdGetInt("oligomaxpolyx");
  internal_oligo_mishyb_library = ajAcdGetInfile("oligomishyblibrary");
  internal_oligo_max_mishyb     = ajAcdGetFloat("oligomaxmishyb");
/*
  internal_oligo_min_quality    = ajAcdGetInt("oligominquality");
*/  
  
/* Internal Oligo penalties */
/* these are not (yet) implemented */

/* EMBOSS-wrapper-specific stuff */
  outfile                       = ajAcdGetOutfile("outfile");
  

    /* open the pipes to connect to primer3 */
    if ( pipe( pipeto ) != 0 )
        ajFatal ( "Couldn't open pipe() to" );
    if ( pipe( pipefrom ) != 0 )
        ajFatal( "Couldn't open pipe() from" );

    
    /* fork off the primer3 executable */
    nPid = fork();
    if ( nPid < 0 )
        ajFatal( "Failure of fork()" );
    else if ( nPid == 0 )
    {
        /* CHILD PROCESS */
        /* dup pipe read/write to stdin/stdout */
       /* 
       ** I'm not sure how standard these STDIN/OUT_FILENO macros are,
       ** so use fileno to get stdin/out file numbers
       */
/*
        dup2( pipeto[0], STDIN_FILENO );
        dup2( pipefrom[1], STDOUT_FILENO  );
*/
        dup2( pipeto[0],  fileno(stdin));
        dup2( pipefrom[1], fileno(stdout));
        /* close unnecessary pipe descriptors */
        close( pipeto[0] );
        close( pipeto[1] );
        close( pipefrom[0] );
        close( pipefrom[1] );
	
	program = ajStrNew();
	ajStrAssC(&program, "primer3_core");
	if (ajSysWhich(&program)) {
	    execlp( "primer3_core", "primer3_core", NULL );
	    ajFatal("There was a problem while executing primer3");
	} else {
	    ajFatal("The program 'primer3_core' must be on the path.\nIt is part of the 'primer3' package,\navailable from the Whitehead Institute.\nSee: http://www-genome.wi.mit.edu/");
	}
	ajStrDel(&program);
	
    } else {
        /* PARENT PROCESS */
        /* Close unused pipe ends. This is especially important for the
        * pipefrom[1] write descriptor, otherwise primer3_read will never 
        * get an EOF.
        */
        close( pipeto[0] );
        close( pipefrom[1] );

        stream = primer3_start_write(pipeto[1]);

/* send primer3 Primer "Global" parameters */
        primer3_send_bool(stream, "PRIMER_EXPLAIN_FLAG", explain_flag);
	primer3_send_bool(stream, "PRIMER_FILE_FLAG", file_flag);
	if (!ajStrCmpC(task[0], "0")) {
	    ajStrAssC(&taskstr, "pick_pcr_primers");
	} else if (!ajStrCmpC(task[0], "1")) {
	    ajStrAssC(&taskstr, "pick_pcr_primers_and_hyb_probe");
	} else if (!ajStrCmpC(task[0], "2")) {
	    ajStrAssC(&taskstr, "pick_left_only");
	} else if (!ajStrCmpC(task[0], "3")) {
	    ajStrAssC(&taskstr, "pick_right_only");
	} else if (!ajStrCmpC(task[0], "4")) {
	    ajStrAssC(&taskstr, "pick_hyb_probe_only");
	} else {
	    ajFatal("Unknown value for TASK");
	}
	primer3_send_string(stream, "PRIMER_TASK", taskstr);
	primer3_send_int(stream, "PRIMER_NUM_RETURN", num_return);
	primer3_send_int(stream, "PRIMER_FIRST_BASE_INDEX", first_base_index);
	primer3_send_bool(stream, "PRIMER_PICK_ANYWAY", pick_anyway);
        /* mispriming library may not have been specified */
        if (mispriming_library) {
          primer3_send_stringC(stream, "PRIMER_MISPRIMING_LIBRARY", 
          	ajFileName(mispriming_library));
        }
	primer3_send_float(stream, "PRIMER_MAX_MISPRIMING", max_mispriming);
	primer3_send_float(stream, "PRIMER_PAIR_MAX_MISPRIMING", pair_max_mispriming);
	primer3_send_int(stream, "PRIMER_GC_CLAMP", gc_clamp);
	primer3_send_int(stream, "PRIMER_OPT_SIZE", opt_size);
	primer3_send_int(stream, "PRIMER_MIN_SIZE", min_size);
	primer3_send_int(stream, "PRIMER_MAX_SIZE", max_size);
	primer3_send_float(stream, "PRIMER_OPT_TM", opt_tm);
	primer3_send_float(stream, "PRIMER_MIN_TM", min_tm);
	primer3_send_float(stream, "PRIMER_MAX_TM", max_tm);
	primer3_send_float(stream, "PRIMER_MAX_DIFF_TM", max_diff_tm);
	primer3_send_float(stream, "PRIMER_OPT_GC_PERCENT", opt_gc_percent);
	primer3_send_float(stream, "PRIMER_MIN_GC", min_gc);
	primer3_send_float(stream, "PRIMER_MAX_GC", max_gc);
	primer3_send_float(stream, "PRIMER_SALT_CONC", salt_conc);
	primer3_send_float(stream, "PRIMER_DNA_CONC", dna_conc);
	primer3_send_int(stream, "PRIMER_NUM_NS_ACCEPTED", num_ns_accepted);
	primer3_send_float(stream, "PRIMER_SELF_ANY", self_any);
	primer3_send_float(stream, "PRIMER_SELF_END", self_end);
	primer3_send_int(stream, "PRIMER_MAX_POLY_X", max_poly_x);
        primer3_send_int(stream, "PRIMER_PRODUCT_OPT_SIZE", product_opt_size);
	primer3_send_range2(stream, "PRIMER_PRODUCT_SIZE_RANGE", product_size_range);
        primer3_send_float(stream, "PRIMER_PRODUCT_OPT_TM", product_opt_tm);
	primer3_send_float(stream, "PRIMER_PRODUCT_MIN_TM", product_min_tm);
	primer3_send_float(stream, "PRIMER_PRODUCT_MAX_TM", product_max_tm);
	primer3_send_float(stream, "PRIMER_MAX_END_STABILITY", max_end_stability);
	 
/* send primer3 Internal Oligo "Global" parameters */	
	primer3_send_int(stream, "PRIMER_INTERNAL_OLIGO_OPT_SIZE", internal_oligo_opt_size);
	primer3_send_int(stream, "PRIMER_INTERNAL_OLIGO_MIN_SIZE", internal_oligo_min_size);
	primer3_send_int(stream, "PRIMER_INTERNAL_OLIGO_MAX_SIZE", internal_oligo_max_size);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_OPT_TM", internal_oligo_opt_tm);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_MIN_TM", internal_oligo_min_tm);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_MAX_TM", internal_oligo_max_tm);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_OPT_GC_PERCENT", internal_oligo_opt_gc_percent);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_MIN_GC", internal_oligo_min_gc);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_MAX_GC", internal_oligo_max_gc);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_SALT_CONC", internal_oligo_salt_conc);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_DNA_CONC", internal_oligo_dna_conc);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_SELF_ANY", internal_oligo_self_any);
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_SELF_END", internal_oligo_self_end);
	primer3_send_int(stream, "PRIMER_INTERNAL_OLIGO_MAX_POLY_X", internal_oligo_max_poly_x);
        /* internal oligo mishybridising library may not have been specified */
        if (internal_oligo_mishyb_library) {
          primer3_send_stringC(stream, "PRIMER_INTERNAL_OLIGO_MISHYB_LIBRARY", 
		ajFileName(internal_oligo_mishyb_library));
        }
	primer3_send_float(stream, "PRIMER_INTERNAL_OLIGO_MAX_MISHYB", internal_oligo_max_mishyb);


	while(ajSeqallNext(sequence, &seq))
	{
            begin = ajSeqallBegin(sequence);
            end   = ajSeqallEnd(sequence);

            strand = ajSeqStrCopy(seq);

            ajStrToUpper(&strand);
            ajStrAssSubC(&substr,ajStrStr(strand), begin-1, end-1);

/* send flags to turn on using optimal product size */
	   primer3_send_float(stream, "PRIMER_PAIR_WT_PRODUCT_SIZE_GT", 0.05);
	   primer3_send_float(stream, "PRIMER_PAIR_WT_PRODUCT_SIZE_LT", 0.05);

/* send primer3 Primer "Sequence" parameters */
            primer3_send_string(stream, "SEQUENCE", substr);
            primer3_send_string(stream, "PRIMER_SEQUENCE_ID", ajSeqGetName(seq));
            primer3_send_range(stream, "INCLUDED_REGION", included_region);
	    primer3_send_range(stream, "TARGET", target);
	    primer3_send_range(stream, "EXCLUDED_REGION", excluded_region);
	    primer3_send_string(stream, "PRIMER_LEFT_INPUT", left_input);
	    primer3_send_string(stream, "PRIMER_RIGHT_INPUT", right_input);

/* send primer3 Internal Oligo "Sequence" parameters */
            primer3_send_range(stream, "PRIMER_INTERNAL_OLIGO_EXCLUDED_REGION", internal_oligo_excluded_region);
	    primer3_send_string(stream, "PRIMER_INTERNAL_OLIGO_INPUT", internal_oligo_input);
            

/* end the primer3 input sequence record with a '=' */
            primer3_send_end(stream);
        }
	
 	primer3_end_write(stream);
	close( pipeto[1] );

/* read the primer3 output */

        result = primer3_read(pipefrom[0]);

       	primer3_report (outfile, result, num_return);
    }

/* tidy up */    
    ajStrDel(&result);
    ajSeqDel(&seq);
    ajStrDel(&strand);
    ajStrDel(&substr);
    ajFileClose(&outfile);
    ajStrDel(&taskstr);
  
    ajExit();
    return 0;
}


/* @funcstatic primer3_read *******************************************
**
** Reads the output from primer3_core into a returned AjPStr until EOF
**
** @param [r] fd [int] file descriptor
** @return [AjPStr] 
**          
*************************************************************************/

static AjPStr primer3_read(int fd)
{
    FILE *stream;
    int ch;
    AjPStr ret=ajStrNew();

    ajDebug( "reading primer3_core output\n" );
    
    if ( (stream = fdopen( fd, "r" )) == NULL )
        ajFatal( "fdopen() read error" );
	
    while ( (ch = getc( stream )) != EOF )
        ajStrAppK(&ret, ch);

    ajDebug( "primer3_core output\n%S\n", ret );


    fclose( stream );
    ajDebug( "reading done\n" );

    return ret;
}


/* @funcstatic primer3_send_end *******************************************
**
** Writes the end-of-input flag '=' to the input stream of primer3_core
**
** @param [r] stream [FILE *] File handle
** @return [void] 
**          
*************************************************************************/
static void primer3_send_end(FILE * stream)
{
  fputs( "=\n", stream );
}

/* display ranges as 'start,length start2,length2' */
/* @funcstatic primer3_send_range *******************************************
**
** Write range data to primer3_core as 'start,length start2,length2,etc'
**
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [AjPRange] Ranges to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_range(FILE * stream, char * tag, AjPRange value)
{

  AjPStr str=ajStrNew();
  ajint n;
  ajint start, end;
  
  if (ajRangeNumber(value)) {
      (void) ajFmtPrintS(&str, "%s=", tag);
      primer3_write(str, stream);
      ajStrClear(&str);
      for (n=0; n < ajRangeNumber(value); n++) {
          ajRangeValues(value, n, &start, &end);
          (void) ajFmtPrintS(&str, "%d,%d ", start, end-start+1);  
          primer3_write(str, stream);
	  ajStrClear(&str);
      }
      (void) ajFmtPrintS(&str, "\n");
      primer3_write(str, stream);
  }

  ajStrDel(&str);
}


/* @funcstatic primer3_send_range2 *******************************************
**
** Write alternate display of ranges as 'a-b c-d' to primer3_core
**
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [AjPRange] Ranges to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_range2(FILE * stream, char * tag, AjPRange value)
{

  AjPStr str=ajStrNew();
  ajint n;
  ajint start, end;
  
  if (ajRangeNumber(value)) {
      (void) ajFmtPrintS(&str, "%s=", tag);
      primer3_write(str, stream);
      ajStrClear(&str);
      for (n=0; n < ajRangeNumber(value); n++) {
          ajRangeValues(value, n, &start, &end);
          (void) ajFmtPrintS(&str, "%d-%d ", start, end);  
          primer3_write(str, stream);
	  ajStrClear(&str);
      }
      (void) ajFmtPrintS(&str, "\n");
      primer3_write(str, stream);
  }

  ajStrDel(&str);
}

/* @funcstatic primer3_send_int *******************************************
**
** Write integer to primer3_core
**
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [ajint] Integer value to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_int(FILE * stream, char * tag, ajint value)
{

  AjPStr str=ajStrNew();
 
  (void) ajFmtPrintS(&str, "%s=%d\n", tag, value);  
  primer3_write(str, stream);
 
  ajStrDel(&str);
}

/* @funcstatic primer3_send_float *******************************************
**
** Write float to primer3_core
**
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [float] Float value to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_float(FILE * stream, char * tag, float value)
{

  AjPStr str=ajStrNew();

  (void) ajFmtPrintS(&str, "%s=%f\n", tag, value);  
  primer3_write(str, stream);

  ajStrDel(&str);
}

/* @funcstatic primer3_send_bool *******************************************
**
** Write boolean to primer3_core
**
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [AjBool] Boolean value to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_bool(FILE * stream, char * tag, AjBool value)
{

  AjPStr str=ajStrNew();

  (void) ajFmtPrintS(&str, "%s=%d\n", tag, value?1:0);  
  primer3_write(str, stream);

  ajStrDel(&str);
}

/* @funcstatic primer3_send_string *******************************************
**
** Write string to primer3_core
**
** 
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [AjPStr] String value to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_string(FILE * stream, char * tag, AjPStr value)
{

  AjPStr str=ajStrNew();

  if (ajStrLen(value)) {
      (void) ajFmtPrintS(&str, "%s=%S\n", tag, value);  
      primer3_write(str, stream);
  }

  ajStrDel(&str);
}

/* @funcstatic primer3_send_stringC *******************************************
**
** Write char * to primer3_core
**
** @param [r] stream [FILE *] File handle
** @param [r] tag [char *] Tag of primer3 data type
** @param [r] value [char *] Char * value to write
** @return [void] 
**          
*************************************************************************/
static void primer3_send_stringC(FILE * stream, char * tag, const char * value)
{

  AjPStr str=ajStrNew();

  if (strlen(value)) {
      (void) ajFmtPrintS(&str, "%s=%s\n", tag, value);  
      primer3_write(str, stream);
  }

  ajStrDel(&str);
}

/* @funcstatic primer3_start_write *******************************************
**
** Open a file descriptor as a stream to pipe to primer3_core
**
** @param [r] fd [int] File descriptor
** @return [FILE *] File stream
**          
*************************************************************************/
static FILE * primer3_start_write(int fd)
{
  FILE *stream;

  ajDebug( "start writing\n" );
  if ( (stream = fdopen( fd, "w" )) == NULL )
      ajFatal( "Couldn't open pipe with fdopen()" );
  
  return stream;
}
/* @funcstatic primer3_write *******************************************
**
** Write a tag=value AjPStr to the primer3_core input stream
**
** @param [r] str [AjPStr] Input string
** @param [r] stream [FILE *] Stream piped to primer3_core
** @return [void] 
**          
*************************************************************************/
static void primer3_write(AjPStr str, FILE *stream)
{
 
  fputs( ajStrStr(str), stream );

}
/* @funcstatic primer3_end_write *******************************************
**
** Close the stream piping in to primer3_core
**
** @param [r] stream [FILE *] Stream
** @return [void] 
**          
*************************************************************************/
static void primer3_end_write(FILE *stream)
{
 
  fclose( stream );
  
}

/* @funcstatic primer3_report *******************************************
**
** Read output of primer3_core into a temporary table of tag/value results
**
** @param [r] outfile [AjPFile] Report outfile
** @param [r] output [AjPStr] Output from primer3_core
** @param [r] numreturn [ajint] Number of results to return for each sequence
** @return [void] 
**          
*************************************************************************/
static void primer3_report (AjPFile outfile, AjPStr output, ajint numreturn)
{

  AjPStr line = NULL;
  AjPStrTok linetokenhandle;
  char eol[] = "\n\r";
  AjPStrTok keytokenhandle;
  char equals[] = "=";
  AjPStr key = NULL;
  AjPStr value = NULL;
  AjBool gotsequenceid = AJFALSE;
  AjPTable table;

/* set up tokeniser for lines */
  linetokenhandle = ajStrTokenInit (output, eol);

/* get next line of relevant results */
  while (ajStrToken (&line, &linetokenhandle, eol)) {
    if (!gotsequenceid) {

/* 
** Are we at the start of another sequence's results?
** Start storing the results in the table.
*/

      if (ajStrNCmpC(line, "PRIMER_SEQUENCE_ID=", 19) == 0) {
        gotsequenceid = AJTRUE;
        table = ajStrTableNew(TABLEGUESS);

      } else {
        continue;
      }

    } else {

/* 
** Are we at the end of this sequence? - marked by a '=' in the primer3
** output - then output the results.
*/

      if (ajStrCmpC(line, "=") == 0) {
        gotsequenceid = AJFALSE;
        primer3_output_report(outfile, table, numreturn);
        ajStrTableFree(&table);	
        continue;
      }
    }	
    
/* store key and value in table and parse values when have all of the sequence
results in the table because the LEFT, RIGHT and INTERNAL results for each
resulting primer are interleaved */
    
    keytokenhandle = ajStrTokenInit (line, equals);

    key = ajStrNew();
    ajStrToken(&key, &keytokenhandle,NULL);

    value = ajStrNew();
    ajStrToken(&value, &keytokenhandle,NULL);

/* debug */
    (void) ajDebug("key=%S\tvalue=%S\n", key, value);

    ajTablePut(table, (const void *)key, (void *)value);

    (void) ajStrTokenClear(&keytokenhandle);
  }
 
/* tidy up */
  ajStrDel(&line);
  (void) ajStrTokenClear(&linetokenhandle);
  ajStrTableFree(&table);

  return;
}

/* @funcstatic primer3_output_report *******************************************
**
** Read the results out of the tag/value table and write to report
**
** @param [r] outfile [AjPFile] Report outfile
** @param [r] table [AjPTable] Table of tag/value result pairs
** @param [r] numreturn [ajint] Number of results to return for each sequence
** @return [void] 
**          
*************************************************************************/
static void primer3_output_report(AjPFile outfile, AjPTable table,
	ajint numreturn)
{
  AjPStr key = NULL;  
  AjPStr error = NULL;
  AjPStr explain = NULL;
  AjPStr seqid = NULL;
  ajint i;
  AjPStr size = NULL;
  AjPStr seq = NULL;
  AjPStr pos = NULL;
  AjPStr tm = NULL;
  AjPStr gc = NULL;

/* Typical primer3 output looks like:
PRIMER_SEQUENCE_ID=em-id:HSFAU
PRIMER_PAIR_PENALTY=0.1974
PRIMER_LEFT_PENALTY=0.007073
PRIMER_RIGHT_PENALTY=0.190341
PRIMER_INTERNAL_OLIGO_PENALTY=0.132570
PRIMER_LEFT_SEQUENCE=AGTCGCCAATATGCAGCTCT
PRIMER_RIGHT_SEQUENCE=GGAGCACGACTTGATCTTCC
PRIMER_INTERNAL_OLIGO_SEQUENCE=GGAGCTACACACCTTCGAGG
PRIMER_LEFT=47,20
PRIMER_RIGHT=183,20
PRIMER_INTERNAL_OLIGO=80,20
PRIMER_LEFT_TM=60.007
PRIMER_RIGHT_TM=59.810
PRIMER_INTERNAL_OLIGO_TM=59.867
PRIMER_LEFT_GC_PERCENT=50.000
PRIMER_RIGHT_GC_PERCENT=55.000
PRIMER_INTERNAL_OLIGO_GC_PERCENT=60.000
PRIMER_LEFT_SELF_ANY=4.00
PRIMER_RIGHT_SELF_ANY=5.00
PRIMER_INTERNAL_OLIGO_SELF_ANY=4.00
PRIMER_LEFT_SELF_END=2.00
PRIMER_RIGHT_SELF_END=1.00
PRIMER_INTERNAL_OLIGO_SELF_END=4.00
PRIMER_LEFT_END_STABILITY=7.9000
PRIMER_RIGHT_END_STABILITY=8.2000
PRIMER_PAIR_COMPL_ANY=5.00
PRIMER_PAIR_COMPL_END=1.00
PRIMER_PRODUCT_SIZE=137

*/

/* check for errors */
  ajStrAssC(&key, "PRIMER_ERROR");
  error = (AjPStr)ajTableGet(table, (const void*)key);
  if (error != NULL) {
    ajErr("%S", error);
  }
  ajStrAssC(&key, "PRIMER_WARNING");
  error = (AjPStr)ajTableGet(table, (const void*)key);
  if (error != NULL) {
    ajWarn("%S", error);
  }
  
/* get the sequence id */
  ajStrAssC(&key, "PRIMER_SEQUENCE_ID");
  seqid = (AjPStr)ajTableGet(table, (const void*)key);
  (void) ajFmtPrintF(outfile, "\n# PRIMER3 RESULTS FOR %S\n\n", seqid);

/* get information on the analysis */
  ajStrAssC(&key, "PRIMER_LEFT_EXPLAIN");
  explain = (AjPStr)ajTableGet(table, (const void*)key);
  if (explain != NULL) {
    ajStrSubstituteCC(&explain, ",", "\n#");
    (void) ajFmtPrintF(outfile, "# FORWARD PRIMER STATISTICS:\n# %S\n\n", explain);
  }
  ajStrAssC(&key, "PRIMER_RIGHT_EXPLAIN");
  explain = (AjPStr)ajTableGet(table, (const void*)key);
  if (explain != NULL) {
    ajStrSubstituteCC(&explain, ",", "\n#");
    (void) ajFmtPrintF(outfile, "# REVERSE PRIMER STATISTICS:\n# %S\n\n", explain);
  }
  ajStrAssC(&key, "PRIMER_PAIR_EXPLAIN");
  explain = (AjPStr)ajTableGet(table, (const void*)key);
  if (explain != NULL) {
    ajStrSubstituteCC(&explain, ",", "\n#");
    (void) ajFmtPrintF(outfile, "# PRIMER PAIR STATISTICS:\n# %S\n\n", explain);
  }
  ajStrAssC(&key, "PRIMER_INTERNAL_OLIGO_EXPLAIN");
  explain = (AjPStr)ajTableGet(table, (const void*)key);
  if (explain != NULL) {
    ajStrSubstituteCC(&explain, ",", "\n#");
   (void) ajFmtPrintF(outfile, "# INTERNAL OLIGO STATISTICS:\n# %S\n\n", explain);
  }
  
/* table header */
  (void) ajFmtPrintF(outfile, "#                      Start  Len   Tm     GC%%   Sequence\n\n");

/* get the results */
  for (i=0; i <= numreturn; i++) {

/* product data */
    size = primer3_tableget("PRIMER_PRODUCT_SIZE", i, "", table);
    if (size != NULL) {
      (void) ajFmtPrintF(outfile, "%4d PRODUCT SIZE: %S\n",
    	i+1, size);
    }

/* left primer data */
    tm = primer3_tableget("PRIMER_LEFT", i, "TM", table);
    gc = primer3_tableget("PRIMER_LEFT", i, "GC_PERCENT", table);
    pos = primer3_tableget("PRIMER_LEFT", i, "", table);
    seq = primer3_tableget("PRIMER_LEFT", i, "SEQUENCE", table);
    primer3_write_primer(outfile, "FORWARD PRIMER", pos, tm, gc, seq, AJFALSE);


/* right primer data */
    tm = primer3_tableget("PRIMER_RIGHT", i, "TM", table);
    gc = primer3_tableget("PRIMER_RIGHT", i, "GC_PERCENT", table);
    pos = primer3_tableget("PRIMER_RIGHT", i, "", table);
    seq = primer3_tableget("PRIMER_RIGHT", i, "SEQUENCE", table);
    primer3_write_primer(outfile, "REVERSE PRIMER", pos, tm, gc, seq, AJTRUE);


/* internal oligo data */

    tm = primer3_tableget("PRIMER_INTERNAL_OLIGO", i, "TM", table);
    gc = primer3_tableget("PRIMER_INTERNAL_OLIGO", i, "GC_PERCENT", table);
    pos = primer3_tableget("PRIMER_INTERNAL_OLIGO", i, "", table);
    seq = primer3_tableget("PRIMER_INTERNAL_OLIGO", i, "SEQUENCE", table);
    primer3_write_primer(outfile, "INTERNAL OLIGO", pos, tm, gc, seq, AJFALSE);

    (void) ajFmtPrintF(outfile, "\n");

  }

/* tidy up */
  ajStrDel(&key);

}


/* @funcstatic primer3_tableget *******************************************
**
** Read the results out of the tag/value table
**
** @param [r] key1 [char *] First half of table key base string
** @param [r] number [ajint] Table key numeric part
** @param [r] key1 [char *] Second half of table key base string (minus the '_')
** @param [r] table [AjPTable] Table of tag/value result pairs
** @return [AjPStr] Table value
**          
*************************************************************************/
static AjPStr primer3_tableget(char *key1, ajint number, char *key2, AjPTable table)
{
  AjPStr fullkey = NULL;
  AjPStr keynum = NULL;
  AjPStr value = NULL;

  ajStrAssC(&fullkey, key1);
  if (number > 0) {
    ajStrAppC(&fullkey, "_");
    ajStrFromInt(&keynum, number);
    ajStrApp(&fullkey, keynum);
  }
  if (strcmp(key2, "")) {
    ajStrAppC(&fullkey, "_");
  }
  ajStrAppC(&fullkey, key2);
  ajDebug("Constructed key=%S\n", fullkey);
  value = (AjPStr)ajTableGet(table, (const void*)fullkey);

/* tidy up */
  ajStrDel(&fullkey);
  ajStrDel(&keynum);

  return value;

}
/* @funcstatic primer3_write_primer *******************************************
**
** Write out one primer or oligo line to the output file
**
** @param [r] outfile [AjPFile] Report outfile
** @param [r] tag [char *] Tag on output line
** @param [r] pos [AjPStr] Start and length string
** @param [r] tm [AjPStr] Tm of primer
** @param [r] gc [AjPStr] GC% of primer
** @param [r] seq [AjPStr] Sequence of primer
** @param [r] rev [AjBool] Sequence is reverse-complement
** @return [void] 
**          
*************************************************************************/
static void primer3_write_primer(AjPFile outfile, char *tag, AjPStr pos, 
	AjPStr tm, AjPStr gc, AjPStr seq, AjBool rev)
{

  ajint startint;
  ajint lenint;
  float tmfloat;
  float gcfloat;
  AjPStr start = NULL;
  ajint comma;

  if (pos != NULL) {
    ajStrToFloat(tm, &tmfloat);
    ajStrToFloat(gc, &gcfloat);
    comma = ajStrFindC(pos, ",");
    ajStrAss(&start, pos);
    ajStrCut(&start, comma, ajStrLen(start));
    ajStrToInt(start, &startint);
    ajStrCut(&pos, 0, comma);
    ajStrToInt(pos, &lenint);
    if (rev) {
      startint = startint - lenint + 1;
    }
    (void) ajFmtPrintF(outfile, "     %s  %6d %4d  %2.2f  %2.2f  %S\n\n",
    	tag, startint, lenint, tmfloat, gcfloat, seq);
  }
  
/* tidy up */
  ajStrDel(&start);

}

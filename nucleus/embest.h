#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embest_h
#define embest_h

/* Definition of the padding-character in CAF */

#define padding_char '-'

/* @data hash_list ************************************************************
**
** NUCLEUS internal data structure for est2genome EMBOSS application
** to maintain internal hash lists.
**
** @attr name [char*] Name
** @attr offset [unsigned long] Offset
** @attr text_offset [unsigned long] Text offset
** @attr next [struct hash_list*] Next in list
** @@
******************************************************************************/

typedef struct hash_list
{
  char *name;
  unsigned long offset;
  unsigned long text_offset;
  struct hash_list *next;
}
HASH_LIST;


typedef enum { INTRON=0, DIAGONAL=1, DELETE_EST=2, DELETE_GENOME=3,
	       FORWARD_SPLICED_INTRON=-1, REVERSE_SPLICED_INTRON=-2
} directions;
typedef enum { NOT_A_SITE=1, DONOR=2, ACCEPTOR=4 } donor_acceptor;

/* @data EmbPEstAlign *********************************************************
**
** NUCLEUS data structure for EST alignments (originally for est2genome)
**
** @attr gstart [ajint] Genomic start
** @attr estart [ajint] EST start
** @attr gstop [ajint] Genomic stop
** @attr estop [ajint] EST stop
** @attr score [ajint] Score
** @attr len [ajint] Length
** @attr align_path [ajint*] Path
** @@
******************************************************************************/

typedef struct EmbSEstAlign
{
  ajint gstart;
  ajint estart;
  ajint gstop;
  ajint estop;
  ajint score;
  ajint len;
  ajint *align_path;
} EmbOEstAlign;
#define EmbPEstAlign EmbOEstAlign*



enum base_types /* just defines a, c, g, t as 0-3, for indexing purposes. */
{
  base_a, base_c, base_g, base_t, base_n, base_i, base_o, nucleotides, anybase
};
/* Definitions for nucleotides */

#define for_each_base(n) for(n=base_a;n<=base_t;n++) /* iterate over acgt */
#define for_any_base(n)  for(n=base_a;n<nucleotides;n++) /* iterate acgtn */
#define proper_base(n)  ( (n) >= base_a && (n) <= base_t ) /* test for acgt */


/* interconvert between char and ajint representations of nucleotides */

char c_base( ajint base ); /* ajint to char */
ajint i_base( char base ); /* char to ajint */


FILE *openfile( char *filename, char *mode );

     /* attempts to open filename, using mode to determine function
	(read/write/append), and stops the program on failure */

FILE *openfile_in_searchpath(char *basename, char *mode, char *searchpath,
			     char *fullname );
FILE *openfile_in_envpath(char *basename, char *mode, char *env,
			  char *fullname );


char *extension ( char *filename, char *ext);

     /* changes the extension of filename to ext.

	e.g.
	extension( "file.dat", "out" );

	produces file.out

	extension( "file", "out");

	produces file.out

	extension( "file", ".out");

	produces file.out

	extension( "file.out", "");

	produces file

	Note: filename MUST be long enough to cope !

        Returns the modified string */


char *base_name (char *filename);

     /* strips any directory from filename

	e.g.
	base_name("/gea/users/rmott/.cshrc");

	produces .cshrc

	base_name("/usr");

	produces usr

	*/

char *dirname( char *pathname );

/* strips off the filename from the path, leaving the directory */

char *directory( char *filename, char *dir );

     /* changes the directory part of filename to dir.

	e.g.
	directory( "/home/gea/fred/file.dat", "/usr/local/" );

	produces /usr/local/file.dat

	directory("/usr/lib", "var");

	produces var/lib, i.e. a directory slash is inserted if necessary.

	*/

char *rootname( char *filename );

     /* trims off the directory and the extension from filename */

char *make_legal( char *filename );

FILE *argfile( char *format, char *mode, ajint argc, char **argv,
	       char *filename );

     /* parses the command-line using format, attempts to get a
	filename and open it. If format does not match any argument
	then NULL is returned. If format matchs an argument but the
	file will not open then the program stops*/

FILE *argfile_force( char *format, char *mode,
		     ajint argc, char **argv, char *filename );

     /* parses the command-line using format, attempts to get a
	filename and open it. If format does not match any argument or
	if format matchs an argument but the file will not open then
	the program stops*/

ajint file_time( char *filename );

     /* returns the time that filename was last modified, or 0 if filename
	cannot be opened for reading or if the call to stat fails */

char *file_date( char *filename );

     /* similar to file_time() execpt that a pointer to the date (in
	English) */

ajint legal_string( char *string, char **strings, ajint size, ajint *value );

/* checks if string is a member os strings, and sets value to the
index in the array strings returns 1 on success and 0 on failure */

char *next_arg( char *format, ajint argc, char **argv );
char *cl_stub( char *format );

ajint add_commands_from_file( ajint argc, char **argv,
			      ajint *nargc, char ***nargv );
void print_usage( ajint argc, char **argv, ajint stop );
void append_usage( char *format, char *text, char *def, ajint overide );
void gethelp( ajint argc, char **argv );


char
**split_on_separator( char *string, char separator, ajint *items);

/* headers for simple cmp functions */

ajint icmp(ajint *a, ajint *b);  /* ajint*   */
ajint iicmp(ajint **a, ajint **b);   /* ajint** */
ajint fcmp(float *a,float *b);  /* float* */
ajint ifcmp(float **a,float **b);  /* float** */
ajint Rstrcmp(char *a,char *b); /* Reversed strcmp */
ajint istrcmp(char **a,char **b); /* for char** */
ajint SStrcmp(char ***a,char ***b); /* for char *** !!!! */

ajint uscmp(unsigned short *a,unsigned short *b); /* unsigned short */

ajint IUBunequal(char x,char y);
ajint IUBcmp(char *a,char *b);


#define MINUS_INFINITY -10000000

ajint matches_extension( char *name, char *ext );
ajint matches_suffix( char *name, char *suffix );
ajint hash_name( char *string );
char *has_extension( char *name );
ajint uncomment( char *string );
void indent( void );


void write_MSP( AjPFile ofile, ajint *matches, ajint *len, ajint *tsub,
		AjPSeq genome, ajint gsub, ajint gpos, AjPSeq est,
		ajint esub, ajint epos, ajint reverse, ajint gapped );





ajint do_not_forget( ajint col, ajint row );


void free_rpairs(void);

void rpair_init( ajint max );

#define PRIME_NUM 10007
#define MAX_BUF 10000


char *shuffle_s( char *s, ajint *seed );

unsigned long seekto(FILE *fp, char *text);
unsigned long find_next(FILE *fp, char *text, char *line);
char *downcase(char *s);
char *upcase(char *s);
char *complement_seq(char *seq);
char *clean_line(char *s);
char *seq_comment(AjPSeq seq, char *key);
char *embl_seq_comment(AjPSeq seq, char *key);
char *nbrf_seq_comment(AjPSeq seq, char *key);

char *iubtoregexp(char *iubstring);
char *iub2regexp(char *iubstring, char *regexp, ajint maxlen);
char *iub_regexp( char c);


FILE *openfile_in_seqpath(char *basename, char *mode, char *searchpath,
			  char *fullname);
FILE *openfile_in_seqpath_arg(char *basename, char *ext, char *mode,
			      char *fullname);

AjPSeq into_sequence( char *name, char *desc, char *s );
AjPSeq subseq( AjPSeq seq, ajint start, ajint stop );

EmbPEstAlign embEstAlignNonRecursive ( const AjPSeq est, const AjPSeq genome,
				       ajint match, ajint mismatch,
				       ajint gap_penalty, ajint intron_penalty,
				       ajint splice_penalty,
				       const AjPSeq splice_sites,
				       ajint backtrack, ajint needleman,
				       ajint init_path );

EmbPEstAlign embEstAlignLinearSpace ( const AjPSeq est, const AjPSeq genome,
				      ajint match, ajint mismatch,
				      ajint gap_penalty, ajint intron_penalty,
				      ajint splice_penalty,
				      const AjPSeq splice_sites,
				      float max_area );

AjPSeq       embEstFindSpliceSites( const AjPSeq genome, ajint direction );
void         embEstFreeAlign( EmbPEstAlign *ge );
ajint          embEstGetSeed (void);
void         embEstMatInit (ajint match, ajint mismatch, ajint gap,
			    ajint neutral, char pad_char);
void         embEstOutBlastStyle ( AjPFile ofile,
				  const AjPSeq genome, const AjPSeq est,
				  const EmbPEstAlign ge, ajint match,
				  ajint mismatch, ajint gap_penalty,
				  ajint intron_penalty,
				  ajint splice_penalty,
				  ajint gapped, ajint reverse  );

void         embEstPrintAlign( AjPFile ofile,
			      const AjPSeq genome, const AjPSeq est,
			      const EmbPEstAlign ge, ajint width );
void         embEstSetDebug (void);
void         embEstSetVerbose (void);
AjPSeq       embEstShuffleSeq( AjPSeq seq, ajint in_place, ajint *seed );

#endif

#ifdef __cplusplus
}
#endif

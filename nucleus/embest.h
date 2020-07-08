#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embest_h
#define embest_h

/* Definition of the padding-character in CAF */

#define padding_char '-'

typedef enum 
{ EMBL, NBRF, FASTA, SINGLE  } 
WHAT_KIND_OF_DATABASE;

typedef struct hash_list
{
  char *name;
  unsigned long offset, text_offset;
  struct hash_list *next;
}
HASH_LIST;

typedef struct
{
  char *database;
  WHAT_KIND_OF_DATABASE type;
  int sequences;
  unsigned long length;
  FILE *datafile;
  FILE *textfile;
  FILE *indexfile;
  HASH_LIST **index;
}
DATABASE;


typedef struct
{
  char *name;
  DATABASE *database;
  char *desc;
  int len;
  char *s;
}
SEQUENCE;

typedef enum { INTRON=0, DIAGONAL=1, DELETE_EST=2, DELETE_GENOME=3, FORWARD_SPLICED_INTRON=-1, REVERSE_SPLICED_INTRON=-2 } directions;
typedef enum { NOT_A_SITE=0, DONOR=2, ACCEPTOR=4 } donor_acceptor;

typedef struct 
{
  int gstart, estart;
  int gstop, estop;
  int score;
  int len;
  int *align_path;
}
ge_alignment;

typedef struct
{
  int left, right;
}
coords;

typedef struct remember_pair
{
  int col, row;
}
RPAIR;

enum base_types /* just defines a, c, g, t as 0-3, for indexing purposes. */
{
  base_a, base_c, base_g, base_t, base_n, base_i, base_o, nucleotides, anybase
};
/* Definitions for nucleotides */

#define for_each_base(n) for(n=base_a;n<=base_t;n++)     /* iterate over a c g t */
#define for_any_base(n)  for(n=base_a;n<nucleotides;n++) /* iterate over a c g t n i o */
#define proper_base(n)  ( (n) >= base_a && (n) <= base_t ) /* test if base is a c g t */


/* interconvert between char and int representations of nucleotides */

char c_base( int base ); /* int to char */
int i_base( char base ); /* char to int */


FILE *openfile( char *filename, char *mode );
     
     /* attempts to open filename, using mode to determine function
	(read/write/append), and stops the program on failure */

FILE *openfile_in_searchpath(char *basename, char *mode, char *searchpath, char *fullname );
FILE *openfile_in_envpath(char *basename, char *mode, char *env, char *fullname );


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

FILE *argfile( char *format, char *mode, int argc, char **argv, char *filename );
     
     /* parses the command-line using format, attempts to get a filename and open
	it. If format does not match any argument then NULL is returned. If format
	matchs an argument but the file will not open then the program stops*/
     
FILE *argfile_force( char *format, char *mode, int argc, char **argv, char *filename );
     
     /* parses the command-line using format, attempts to get a filename and open
	it. If format does not match any argument or if format matchs an argument
	but the file will not open then the program stops*/

int file_time( char *filename );
     
     /* returns the time that filename was last modified, or 0 if filename
	cannot be opened for reading or if the call to stat fails */
     
char *file_date( char *filename );
     
     /* similar to file_time() execpt that a pointer to the date (in English) */

int legal_string( char *string, char **strings, int size, int *value );

/* checks if string is a member os strings, and sets value to the index in the array strings
returns 1 on success and 0 on failure */

char *next_arg( char *format, int argc, char **argv );
char *cl_stub( char *format );

int add_commands_from_file( int argc, char **argv, int *nargc, char ***nargv );
void print_usage( int argc, char **argv, int stop );
void append_usage( char *format, char *text, char *def, int overide );
void gethelp( int argc, char **argv );


char 
**split_on_separator( char *string, char separator, int *items);

/* headers for simple cmp functions */

int icmp(int *a,int *b);  /* int   */
int iicmp(int **a,int **b);   /* int** */
int fcmp(float *a,float *b);  /* float* */
int ifcmp(float **a,float **b);  /* float** */
int Rstrcmp(char *a,char *b); /* Reversed strcmp */
int istrcmp(char **a,char **b); /* for char** */
int SStrcmp(char ***a,char ***b); /* for char *** !!!! */

int uscmp(unsigned short *a,unsigned short *b); /* unsigned short */

int IUBunequal(char x,char y);
int IUBcmp(char *a,char *b);


#define MINUS_INFINITY -10000000

int matches_extension( char *name, char *ext );
int matches_suffix( char *name, char *suffix );
int add_database ( DATABASE *db );
int hash_name( char *string );
char *has_extension( char *name );
int uncomment( char *string );
void indent( void );

void print_align( AjPFile ofile, SEQUENCE *genome, SEQUENCE *est,
		  ge_alignment *ge, int width );

void blast_style_output( AjPFile ofile, SEQUENCE *genome, SEQUENCE *est,
			 ge_alignment *ge, int match, int mismatch,
			 int gap_penalty, int intron_penalty,
			 int splice_penalty, int gapped, int reverse  );

void write_MSP( AjPFile ofile, int *matches, int *len, int *tsub,
		SEQUENCE *genome, int gsub, int gpos, SEQUENCE *est,
		int esub, int epos, int reverse, int gapped );

ge_alignment *linear_space_est_to_genome( SEQUENCE *est, SEQUENCE *genome,
					  int match, int mismatch,
					  int gap_penalty, int intron_penalty,
					  int splice_penalty,
					  SEQUENCE *splice_sites,
					  float max_area );

ge_alignment * recursive_est_to_genome( SEQUENCE *est, SEQUENCE *genome,
					int match, int mismatch,
					int gap_penalty, int intron_penalty,
					int splice_penalty,
					SEQUENCE *splice_sites,
					float max_area, int init_path );

ge_alignment *non_recursive_est_to_genome( SEQUENCE *est, SEQUENCE *genome,
					   int match, int mismatch,
					   int gap_penalty, int intron_penalty,
					   int splice_penalty,
					   SEQUENCE *splice_sites,
					   int backtrack, int needleman,
					   int init_path );

int midpt_est_to_genome( SEQUENCE *est, SEQUENCE *genome, int match,
			 int mismatch, int gap_penalty, int intron_penalty,
			 int splice_penalty, SEQUENCE *splice_sites,
			 int middle, int *gleft, int *gright );

SEQUENCE *find_splice_sites( SEQUENCE *genome, int direction );

void matinit(int match, int mismatch, int gap, int neutral, char pad_char);


int do_not_forget( int col, int row );

int remember( int col, int row );

void free_rpairs(void);

void rpair_init( int max );

#define PRIME_NUM 10007
#define MAX_BUF 10000
#define SEQPATH "SEQ_DATA_HOME"

SEQUENCE *get_next_sq (AjPSeqall all);
SEQUENCE *seq_to_sequence( AjPSeq ajseq );

SEQUENCE *read_sequence(char *name, DATABASE *database);
SEQUENCE *read_seq(char *name);
SEQUENCE *read_embl_sq(char *name, DATABASE *database, unsigned long offset);
SEQUENCE *read_nbrf_sq(char *name, DATABASE *database,
		       unsigned long offset, unsigned long text_offset);
SEQUENCE *read_fasta_sq(char *name, DATABASE *database, unsigned long offset);
SEQUENCE *next_seq_from_list_file(FILE *list_file);
SEQUENCE *next_seq_from_database_spec(char *spec);
SEQUENCE *next_seq(char *spec_or_file);
SEQUENCE *get_seq_from_file(char *filename);
SEQUENCE *seqdup(SEQUENCE *seq);
SEQUENCE *shuffle_seq( SEQUENCE *seq, int in_place, int *seed );

char *shuffle_s( char *s, int *seed );

DATABASE *open_database(char *name);
DATABASE *which_database(char *database_name);
DATABASE *is_sequence_spec(char *spec, char *wild);
DATABASE *open_embl_database(char *name);
DATABASE *open_nbrf_database(char *name);
DATABASE *open_fasta_database(char *name);

FILE *which_file_of_sequences(char *filename);

unsigned long get_offset(char *name, DATABASE *database,
			 unsigned long *text_offset);
unsigned long seekto(FILE *fp, char *text);
unsigned long find_next(FILE *fp, char *text, char *line);
char *downcase(char *s);
char *upcase(char *s);
char *complement_seq(char *seq);
char *clean_line(char *s);
char *seq_comment(SEQUENCE *seq, char *key);
char *embl_seq_comment(SEQUENCE *seq, char *key);
char *nbrf_seq_comment(SEQUENCE *seq, char *key);

char *iubtoregexp(char *iubstring);
char *iub2regexp(char *iubstring, char *regexp, int maxlen);
char *iub_regexp( char c);


FILE *openfile_in_seqpath(char *basename, char *mode, char *searchpath,
			  char *fullname);
FILE *openfile_in_seqpath_arg(char *basename, char *ext, char *mode,
			      char *fullname);

SEQUENCE *into_sequence( char *name, char *desc, char *s );
SEQUENCE *subseq( SEQUENCE *seq, int start, int stop );
void free_seq( SEQUENCE *seq );
void free_seq_copy( SEQUENCE *seq );

void make_embl_index( DATABASE *db );
void make_fasta_index( DATABASE *db );
void make_nbrf_index( DATABASE *db );
int getseed( int *seed, int argc, char **argv );
void free_ge( ge_alignment *ge );


#endif

#ifdef __cplusplus
}
#endif

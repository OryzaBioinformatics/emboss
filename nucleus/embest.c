/* COMMAND_LINE PARSING AND FILE UTILITY ROUTINES */
/*
   (C) Richard Mott, Genome Analysis Lab, ICRF
   */

#include "emboss.h"
#include "embest.h"
#include <dirent.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
static DATABASE *databases[100];
static ajint database_count;

ajint lsimmat[256][256];
ajint verbose;
ajint debug;
ajint indentation;

/*command line option to suppress all comments*/
ajint comment = 1;

/* @func openfile ************************************************************
**
** attempts to open filename, using mode to determine function
** (read/write/append), and stops the program on failure 
** 
** @param [r] filename [char*] File to be opened
** @param [r] mode     [char*] Mode to be opened in
**        
** @return [FILE*] file pointer.
** @@
******************************************************************************/

FILE* openfile (char *filename, char *mode) {

  FILE *fp;
  if ( ( fp = fopen( make_legal(filename), mode ) ) == NULL )
  {
    ajErr( "ERROR: file %s will not open for function %s\n\n", filename, mode );
    exit(1);
  }
  return fp;
}


/* @func getfloat ************************************************************
**
** gets a float from command line.
** format can be a string like "-a=%f", or "-a". In the former case is
** looks for entries line -a=10.3 AND -a 10.3, in the latter just for -a10.3 
**
** @param [r] format [char*] command line qualifier and format
** @param [w] variable [float*] float value.
** @param [r] argc [ajint] number of command line arguments
** @param [r] argv [char**] the argument list.
**        
** @return [ajint] return 1 if successful 0 otherwise
** @@
******************************************************************************/

ajint getfloat (char *format, float *variable, ajint argc, char **argv) {

  float t; char *s;
  char Format[256];

  (void) sprintf(Format,"%g",*variable);
  append_usage( format, "float", Format, 0 );

  if ( (s = next_arg( format, argc, argv ) ) && sscanf( s, "%f", &t ) == 1 )
    {
      *variable = t;
      return 1;
    }


  s = format;
  while(*s && *s != '=')
    s++;
  if ( *s == '=' )
    (void) sprintf(Format,"%s", format);
  else
    (void) sprintf(Format,"%s%s", format, "=%f");

  while ( --argc > 0 )
    {
      if ( sscanf( argv[argc], Format, &t ) == 1 ) 
	{
	  *variable = t;
	  return 1;
	}
    }

  return 0;
}

/* @func next_arg ************************************************************
**
** scan the command lines for argument matching stub, and return the
** NEXT arg
**
** eg if format == "-max=%d", then argv == "-max 10" returns "10" 
**
** @param [r] format [char*] command line qualifier and format
** @param [rw] argc [ajint] number of command line arguments
** @param [r] argv [char**] the argument list.
**        
** @return [char*] next argument 
** @@
******************************************************************************/

char* next_arg (char *format, ajint argc, char **argv) {

  char *s = cl_stub( format );

  argc--;
  while( --argc > 0 )
    {
      if ( ! strcmp( s, argv[argc] ) )
	return argv[argc+1];
    }
  return NULL;
}

/* @func cl_stub ************************************************************
**
**  extract stub from format, eg "-files=%s" gives "-files" 
**
** @param [r] format [char*] command line qualifier and format
**        
** @return [char*] the stub
** @@
******************************************************************************/

char* cl_stub (char *format ) {

  static char stub[256];
  char *s=stub;

  while ( *format && *format != '=' )
    *s++ = *format++;
  *s = 0;

  return stub;
}


/* @func extension ************************************************************
**
** changes the extension of filename to ext. 
**	
**	e.g.
**	extension( "file.dat", "out" );
**	
**	produces file.out
**	
**	extension( "file", "out");
**	
**	produces file.out
**	
**	extension( "file", ".out");
**	
**	produces file.out
**	
**	extension( "file.out", "");
**	
**	produces file
**	
**	Note: filename MUST be ajlong enough to cope ! 
**
**        Returns the modified string
**
** @param [r] filename [char*] The filename
** @param [r]  ext      [char*] New extension
**        
** @return [char*] modified string
** @@
******************************************************************************/

char* extension (char *filename, char *ext) {

  ajint n;

  if ( ! ext )
    return (char*)NULL;

  if ( ext[0] == '.' )
    ext++;

  n = strlen(filename);

  while ( filename[n] != '.' && n > 0 )
    n--;

  if ( filename[n] != '.' )
    {
      n = strlen(filename);
      filename[n] = '.';
    }

  n++;

  (void) strcpy( &filename[n], ext );

  if ( filename[n=strlen(filename)-1] == '.' )
    filename[n] = 0;

  return filename;
}

/* @func base_name ************************************************************
**
** strips any directory from filename 
**	e.g.
**	base_name("/gea/users/rmott/.cshrc");
**	
**	produces .cshrc
**	
**	base_name("/usr");
**	
**	produces usr
**
** @param [rw] filename [char*] The filename
**        
** @return [char*] base name.
** @@
******************************************************************************/

char* base_name (char *filename) {

  ajint n, m;

  n = strlen(filename);

  while( filename[n] != '/' && n > 0 )
    n--;

  if ( filename[n] == '/' )
    n++;

  m = 0;
  while ( filename[m] )
    filename[m++] = filename[n++];

  return filename;
}

/* @func dirname ************************************************************
**
** strips off the filename from the path, leaving the directory
**
** @param [rw] pathname [char*] The path/file name
**        
** @return [char*] pathname.
** @@
******************************************************************************/

char* dirname (char *pathname ) {

  char *s = &pathname[strlen(pathname)-1];

  while( *s && s > pathname && *s != '/' )
    s--;

  if ( s == pathname && *s != '/' )
    (void) strcpy(pathname, "./");
  else if ( s == pathname && *s == '/' )
    (void) strcpy(pathname,"/");
  else
    *s = 0;

  return pathname;
}

/* @func directory ************************************************************
**
** changes the directory part of filename to dir.
**
**	e.g.
**	directory( "/home/gea/fred/file.dat", "/usr/local/" );
**	
**	produces /usr/local/file.dat
**	
**	directory("/usr/lib", "var");
**	
**	produces var/lib, i.e. a directory slash is inserted if necessary.
**	
** @param [rw] filename [char*] The path/file name
** @param [r]  dir      [char*] new directory to be added.
**        
** @return [char*] pathname.
** @@
******************************************************************************/

char* directory (char *filename, char *dir ) {

  char name[512];
  
  (void) base_name( filename );
  
  if ( dir && *dir  )
    {
      if ( dir[strlen(dir)-1] != '/' )
	(void) sprintf( name, "%s/%s", dir, filename );
      else
	(void) sprintf( name, "%s%s", dir, filename );

      (void) strcpy( filename, name );
    }
  return filename;
}	

/* @func rootname ************************************************************
**
** trims off the directory and the extension from filename
**
** @param [rw] filename [char*] The path/file name
**        
** @return [char*] rootname.
** @@
******************************************************************************/

char* rootname (char *filename ) {

  return extension(base_name(filename),"");
}


/* use compiler switch -DVAX to use the form of make_legal which
   converts . to _ */

/* @func make_legal ***********************************************************
**
** converts files like file.in.out to file_in.out
**
** @param [rw] filename [char*] The path/file name
**        
** @return [char*] filename.
** @@
******************************************************************************/

char* make_legal (char *filename ) {

  char *s = filename, *t;
  ajint n=0;

  t = s;
  while( *s )
    {
      if ( *s == '.' )
	{
	  n++;
	  t = s;
	}
      s++;
    }

  if ( n > 1 )
    {
      t--;
      while ( t != filename )
	{
	  if ( *t == '.' )
	    *t = '_';
	  t--;
	}
    }
  return filename;
}


/* #else

char *
make_legal( char *filename )
{
  return filename;
}

  #endif
*/

/* @func file_time ************************************************************
**
** returns the time that filename was last modified, or 0 if filename
** cannot be opened for reading or if the call to stat fails
**
** @param [r] filename [char*] The filename
**        
** @return [ajint] time or 0 if fails.
** @@
******************************************************************************/

ajint file_time (char *filename ) {

  FILE *fp;
  struct stat buf;

  if ( ! (fp = fopen(filename,"r")) )return 0;
  (void) fclose(fp);
  if ( ! stat( filename, &buf ) )return (ajint)buf.st_mtime;
  return 0;
}

/* @func file_date ************************************************************
**
** returns the date that filename was last modified, or ? if filename
** cannot be opened for reading or if the call to stat fails
**
** @param [r] filename [char*] The filename
**        
** @return [char*] pointer to the date
** @@
******************************************************************************/

char* file_date (char *filename ) {

  FILE *fp;
  struct stat buf;
  char date[256], *t;

  (void) strcpy( date, "?");

  if ( (fp = fopen(filename,"r")) )
    {
      (void) fclose(fp);
      if ( ! stat( filename, &buf ) )
	{
	  (void) sprintf( date, "%s", ctime(&buf.st_mtime) );
	  t = date;
	  while ( *t )
	    {
	      if ( *t == '\n')
		*t = 0;
	      t++;
	    }

	}
    }
  t = date;
  return t;
}

typedef struct use_list
{
  char *format;
  char *text;
  char *def;
  struct use_list *next;
}
USE_LIST;

static USE_LIST *use_begin=NULL, *use_end=NULL;

/* @func append_usage *********************************************************
**
** 'Redundant' Append to usage list. Only used for floating point types now.
**
** @param [r] format [char*] command line qualifier and format
** @param [r] text [char*] text
** @param [r] def [char*] default value (as text)
** @param [r] override [ajint] Override existing value if non zero
**        
** @return [void] 
** @@
******************************************************************************/

void append_usage (char *format, char *text, char *def, ajint override ) {

  USE_LIST *use;

  if ( ! use_begin ) {
    AJNEW (use_begin);
    use_end = use_begin;
  }
  else 
    {
      use = use_begin;
      while( use )
	{
	  if  ( ! strcmp( use->format, format ) )
	    {
	      if ( override )
		break;
	      else
		return;
	    }
	  use = use->next;
	}

      if ( ! use ) {
	AJNEW(use_end->next);
	use_end = use_end->next;
      }
      else
	use_end = use;
    }

/*  (void) ajDebug("append %s %s %s\n", format, text, def ); */
  use_end->format = (char*)ajSysStrdup( format );  
  use_end->text = (char*)ajSysStrdup( text );
  use_end->def = (char*)ajSysStrdup( def );
}

#define COM "-comfile="
#define MAX_COM_DEPTH 10

/* @func legal_string *********************************************************
**
**  checks if string is a member of strings, and sets value to the
**  index in the array strings ** returns 1 on success and 0 on
**  failure
**
** @param [r] string [char*] string
** @param [r] strings [char**] list of strings
** @param [r] size [ajint] size of strings.
** @param [rw] value [int*] value of the index.
**        
** @return [ajint] returns 1 on success and 0 on failure
** @@
******************************************************************************/

ajint legal_string (char *string, char **strings, ajint size, ajint *value ) {

  ajint i;

  if ( string )
    for(i=0;i<size;i++)
      if ( ! strcmp( string, strings[i] ) )
	{
	  *value = i;
	  return 1;
	}

  return 0;
}

/* @func split_on_separator ***************************************************
**
** split a string into substrings with separator 
** Returns array of null-terminated strings, or null if none found
** items returns the number of strings;
**
** @param [r] string [char*] string.
** @param [r] separator [char] separator to be used.
** @param [r] items [int*] the argument list.
**        
** @return [char**] array of null terminated strings 
** @@
******************************************************************************/

char** split_on_separator (char *string, char separator, ajint *items) {

  char **item=NULL;
  char *s, *t;
  ajint len;

  *items = 0;

  if ( string )
    {
      *items = 1;
      s = string;
      while(*s)
	{
	  (*items) += ( *s == separator );
	  s++;
	}

      AJCNEW (item, *items);
      s = string;
      *items = 0;
      while( *s )
	{
	  t = s;
	  while( *s != separator && *s )
	    s++;
	  
	  if ( s != t )
	    {
	      len = s-t;
	      AJCNEW(item[*items], len+1);
	      (void) strncpy(item[*items],t,len);
	      (*items) ++;
	    }
	  if ( *s == separator )
	    s++;
	}
    }
  return item;
}

/* CMP contains standard comparison functions for use with qsort */

/* @func  icmp ***********************************************************
**
** Compare two integers
**
** @param [r] a [int*] First value
** @param [r] b [int*] Second value
**
** @return [ajint] difference.
** @@
******************************************************************************/
 
ajint icmp (ajint *a,int *b) { 
  return *a-*b;
}

/* @func  iicmp ************************************************************
**
** Compare two integers.
**
** @param [r] a [int**] First value
** @param [r] b [int**] Second value
**
** @return [ajint] difference
** @@
******************************************************************************/

ajint iicmp (ajint **a,int **b) {

  return **a - **b;
}

/* @func  fcmp ************************************************************
**
** float comparison
**
** @param [r] a [float*] First value
** @param [r] b [float*] Second value
**
** @return [ajint] -1 if diff is negative, +1 if positive else 0
** @@
******************************************************************************/

ajint fcmp (float *a,float *b) {

  float x = *a - *b;
  if ( x > (float) 0.0 )return 1;
  if ( x < (float) 0.0 )return -1;
  return 0;
}

/* @func ifcmp  ************************************************************
**
** Compare two floats.
**
** @param  [r] a [float**] First value
** @param  [r] b [float**] Second value
**
** @return [ajint]  -1 if diff negative, +1 if positive, else 0
** @@
******************************************************************************/

ajint ifcmp (float **a,float **b) {

  return fcmp(*a,*b);
}

/* @func Rstrcmp ************************************************************
**
** String comparison with strings reversed, case insensitive
**
** @param [r] a [char*] First value
** @param [r] b [char*] Second value
**
** @return [ajint] difference.
** @@
******************************************************************************/

ajint Rstrcmp (char *a,char *b) {

  ajint la = strlen(a)-1;
  ajint lb = strlen(b)-1;
  ajint n;
  while ( la && lb )
    {
      if ((n = ( ((ajint)a[la--]) - ((ajint)b[lb--])) ) )
	return n;
    }
  return la-lb;
}

/* @func istrcmp ************************************************************
**
** String comparison.
**
** @param [r] a [char**] First value
** @param [r] b [char**] Second value
**
** @return [ajint] difference
** @@
******************************************************************************/

ajint istrcmp (char **a,char **b) {

  return strcmp(*a,*b);
}


/* a version of strcmp which works with null strings */
	  
/* @func nStrcmp ************************************************************
**
** a version of strcmp which works with null string
**
** @param [r] a [char*] First value
** @param [r] b [char*] Second value
**
** @return [ajint] difference
** @@
******************************************************************************/

ajint nStrcmp (char *a, char *b ) {

  if ( a && b )return strcmp(a,b);
  return 0;
}

/* @func  SStrcmp ************************************************************
**
**  String comparison.
**
** @param [r] a [char***] First value
** @param [r] b [char***] Second value
**
** @return [ajint] difference
** @@
******************************************************************************/

ajint SStrcmp (char ***a,char ***b) {

  return istrcmp(*a,*b);
}

/* @func  uscmp ************************************************************
** 
** unsigned short compare
**
** @param [r] a [ajushort*] First value
** @param [r] b [ajushort*] Second value
**
** @return [ajint] difference
** @@
******************************************************************************/

ajint uscmp (ajushort *a,ajushort *b) {

  return ((ajint)*a - (ajint)*b);
}

/* @func next_file ************************************************************
**
** Get the next file that fits dir/name.ext 
**
** @param [r] dir [char*] directory
** @param [r] ext [char*] extension
** @param [r] name [char*] name of file
** @param [r] restart [ajint] restart from the begining.
**
** @return [FILE*] next file
** @@
******************************************************************************/

FILE* next_file (char *dir, char *ext, char *name, ajint restart ) {

  static DIR *current_dir=NULL;
  char buf[256];
  struct dirent *fil;
  FILE *fp;
  if ( ! current_dir )
    {
      if ( ! (current_dir = (DIR*)opendir(dir) ) )
	{
	  (void) ajFatal("directory %s cannot be opened\n", dir );
	}
    }

  if ( restart )
    rewinddir( current_dir );

  while( (fil = (struct dirent*)readdir( current_dir )) )
    {
      (void) sprintf( buf, "%s/%s", dir, fil->d_name );
      if ( matches_extension( fil->d_name, ext )  &&
	   (fp = (FILE*)fopen( buf, "r" ) ) )
	/* && use_this_probe(fil->d_name) */
	{
	  (void) strcpy(name,fil->d_name);
	  return fp;
	}
    }

  current_dir=NULL;
  return NULL;
}

/* @func next_suffix_file ****************************************************
**
** Get the next file.
**
** @param [r] dir [char*] directory
** @param [r] suffix [char*] extension
** @param [r] name [char*] name of file
** @param [r] restart [ajint] restart from the begining.
**
** @return [FILE*] the next file.
** @@
******************************************************************************/

FILE* next_suffix_file( char *dir, char *suffix, char *name, ajint restart ) {

  static DIR *current_dir=NULL;
  char buf[256];
  struct dirent *fil;
  FILE *fp;
  if ( ! current_dir )
    {
      if ( ! (current_dir = (DIR*)opendir(dir) ) )
	{
	  (void) ajFatal("ERROR: directory %s cannot be opened\n", dir );
	}
    }

  if ( restart )
    rewinddir( current_dir );

  while( (fil = (struct dirent*)readdir( current_dir )) )
    {
      (void) sprintf( buf, "%s/%s", dir, fil->d_name );
      if ( (matches_suffix( fil->d_name, suffix ))  && (fp = (FILE*)fopen( buf, "r" ) ) ) /* && use_this_probe(fil->d_name) */
	{
	  (void) strcpy(name,fil->d_name);
	  return fp;
	}
    }

  current_dir=NULL;
  return NULL;
}

/* @func matches_suffix *******************************************************
**
** Returns 1 if suffix matches name.
**
** @param [r] name [char*] name of file
** @param [r] suffix [char*] extension
**
** @return [ajint] 1 if match successful, else 0
** @@
******************************************************************************/

ajint matches_suffix( char *name, char *suffix ) {

  ajint pos = strlen(name)-strlen(suffix);
  if ( pos >= 0 )return ! strcmp( &name[pos], suffix );
  return 0;
}

/* @func  matches_extension ***************************************************
**
** compares the file's extension to the extension provided.
**
** @param [r] name [char*] name of file
** @param [r] ext [char*] extension of file
**
** @return [ajint] 1 if match succesfull else 0
** @@
******************************************************************************/

ajint matches_extension( char *name, char *ext ) {

  char *s = (char*)has_extension( name );

 if ( s && ! strcmp(s, ext ) )return 1;
 return 0;
}

/* @func has_extension  *******************************************************
**
** finds the extension and passes back a pointer to its postion.
**
** @param [r] name [char*] name of file
**
** @return [char*] null byte if not found, else pointer to the byte.
** @@
******************************************************************************/

char* has_extension( char *name ) {

  return (char*)strrchr( name, '.' );
}


/* support for sequence matching using IUB ambiguity codes */

/* @func  IUBunequal **********************************************************
**
** Compares UIB sequence codes
**
** @param [r] x [char] First base code
** @param [r] y [char] Second base code
**
** @return [ajint] Comparison score for bases X and Y
** @@
******************************************************************************/

ajint IUBunequal(char x,char y) {

  static char iub[256][256];
  static ajint initialised=0;

  if ( ! initialised )
    {
      char A = 'A';
      char C = 'C';
      char G = 'G';
      char T = 'T';
      char U = 'U';
      char Y = 'Y';
      char M = 'M';
      char K = 'K';
      char S = 'S';
      char W = 'W';
      char H = 'H';
      char B = 'B';
      char V = 'V';
      char D = 'D';
      char N = 'N';
      char R = 'R';
      ajint i, j;

      iub[(ajint)A][(ajint)A] = A;
      iub[(ajint)C][(ajint)C] = C;
      iub[(ajint)G][(ajint)G] = G;
      iub[(ajint)T][(ajint)T] = T;

      iub[(ajint)R][(ajint)R] = R;
      iub[(ajint)R][(ajint)A] = A;
      iub[(ajint)R][(ajint)G] = G;

      iub[(ajint)R][(ajint)Y] = 0;
      iub[(ajint)R][(ajint)M] = A;
      iub[(ajint)R][(ajint)K] = G;
      iub[(ajint)R][(ajint)S] = G;
      iub[(ajint)R][(ajint)W] = A;
      iub[(ajint)R][(ajint)H] = A;
      iub[(ajint)R][(ajint)B] = G;
      iub[(ajint)R][(ajint)V] = R;
      iub[(ajint)R][(ajint)D] = R;
      iub[(ajint)R][(ajint)N] = R;

      iub[(ajint)Y][(ajint)C] = C;
      iub[(ajint)Y][(ajint)T] = T;
      iub[(ajint)Y][(ajint)Y] = Y;

      iub[(ajint)Y][(ajint)U] = U;
      iub[(ajint)Y][(ajint)M] = C;
      iub[(ajint)Y][(ajint)K] = T;
      iub[(ajint)Y][(ajint)S] = C;
      iub[(ajint)Y][(ajint)W] = T;
      iub[(ajint)Y][(ajint)H] = C;
      iub[(ajint)Y][(ajint)B] = Y;
      iub[(ajint)Y][(ajint)V] = C;
      iub[(ajint)Y][(ajint)D] = T;
      iub[(ajint)Y][(ajint)N] = Y;

      iub[(ajint)M][(ajint)A] = A;
      iub[(ajint)M][(ajint)C] = C;
      iub[(ajint)M][(ajint)M] = M;

      iub[(ajint)M][(ajint)S] = C;
      iub[(ajint)M][(ajint)W] = A;
      iub[(ajint)M][(ajint)H] = M;
      iub[(ajint)M][(ajint)B] = C;
      iub[(ajint)M][(ajint)V] = M;
      iub[(ajint)M][(ajint)D] = A;
      iub[(ajint)M][(ajint)N] = M;
      
      iub[(ajint)K][(ajint)G] = G;
      iub[(ajint)K][(ajint)T] = T;
      iub[(ajint)K][(ajint)U] = U;

      iub[(ajint)S][(ajint)C] = C;
      iub[(ajint)S][(ajint)G] = G;
      iub[(ajint)S][(ajint)S] = S;

      iub[(ajint)S][(ajint)H] = C;
      iub[(ajint)S][(ajint)B] = S;
      iub[(ajint)S][(ajint)V] = S;
      iub[(ajint)S][(ajint)D] = G;
      iub[(ajint)S][(ajint)N] = S;
      
      iub[(ajint)W][(ajint)A] = A;
      iub[(ajint)W][(ajint)T] = T;
      iub[(ajint)W][(ajint)U] = U;

      iub[(ajint)W][(ajint)W] = W;
      iub[(ajint)W][(ajint)H] = W;
      iub[(ajint)W][(ajint)B] = T;
      iub[(ajint)W][(ajint)V] = A;
      iub[(ajint)W][(ajint)D] = W;
      iub[(ajint)W][(ajint)N] = W;

      iub[(ajint)B][(ajint)C] = C;
      iub[(ajint)B][(ajint)G] = G;
      iub[(ajint)B][(ajint)T] = T;
      iub[(ajint)B][(ajint)U] = U;

      iub[(ajint)H][(ajint)A] = A;
      iub[(ajint)H][(ajint)C] = C;
      iub[(ajint)H][(ajint)T] = T;
      iub[(ajint)H][(ajint)U] = U;

      iub[(ajint)H][(ajint)H] = H;
      iub[(ajint)H][(ajint)B] = Y;
      iub[(ajint)H][(ajint)V] = M;
      iub[(ajint)H][(ajint)D] = W;
      iub[(ajint)H][(ajint)N] = N;

      iub[(ajint)V][(ajint)A] = A;
      iub[(ajint)V][(ajint)C] = C;
      iub[(ajint)V][(ajint)G] = G;

      iub[(ajint)V][(ajint)V] = V;
      iub[(ajint)V][(ajint)D] = R;
      iub[(ajint)V][(ajint)N] = V;
      
      iub[(ajint)D][(ajint)A] = A;
      iub[(ajint)D][(ajint)G] = G;
      iub[(ajint)D][(ajint)T] = T;
      iub[(ajint)D][(ajint)U] = U;

      iub[(ajint)D][(ajint)D] = D;
      iub[(ajint)D][(ajint)N] = D;

      iub[(ajint)N][(ajint)A] = A;
      iub[(ajint)N][(ajint)C] = C;
      iub[(ajint)N][(ajint)G] = G;
      iub[(ajint)N][(ajint)T] = T;

      iub[(ajint)N][(ajint)N] = N;
      
      for(i=0;i<256;i++)
	for(j=0;j<i;j++)
	  {
	    if ( ! iub[i][j] )
	      iub[i][j] = iub[j][i];
	    else
	      iub[j][i] = iub[i][j];
	    if ( iub[i][j] )
	      (void) ajDebug("%c %c = %c\n", i, j, iub[i][j] );
	  }

      initialised = 1;
    }
  
  return iub[toupper((ajint) x)][toupper((ajint) y)];
}

/* @func IUBcmp ************************************************************
**
** Compare two IUB sequences
**
** @param [r] a [char*] First base code
** @param [r] b [char*] Second base code
**
** @return [ajint] difference
** @@
******************************************************************************/

ajint IUBcmp(char *a,char *b) {

  ajint n;

  while( *a && *b )
    {
      n=IUBunequal(*a++,*b++);
      if ( n )
	return n;
    }
  if ( *a )return 1;
  if ( *b )return -1;
  return 0;
}
      
	

/* @func  non_recursive_est_to_genome *****************************************
**
** Modified Smith-Waterman/Needleman to align an EST or mRNA to a Genomic
** sequence, allowing for introns.
**
** The recursion is
**     
**     {  S[gpos-1][epos]   - gap_penalty
**     
**     {  S[gpos-1][epos-1] + D[gpos][epos]
**     
**     S[gpos][epos] = max {  S[gpos][epos-1]   - gap_penalty
**     
**     {  C[epos]           - intron_penalty 
**     
**     {  0 (optional, only if ! needleman )
**     
**     C[epos] = max{ S[gpos][epos], C[epos] }
**     
**     S[gpos][epos] is the score of the best path to the cell gpos, epos 
**     C[epos] is the score of the best path to the column epos
**     
**     
** @param [r] est [SEQUENCE*] Sequence of EST
** @param [r] genome [SEQUENCE*] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [SEQUENCE*] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] backtrack [ajint] Boolean.
**     If backtrack is 0 then only the start and end points and the score
**     are computed, and no path matrix is allocated.
** @param [r] needleman [ajint] Boolean 1 = global alignment 0 = local alignment
** @param [r] init_path [ajint] Type of initialization for the path.
**     If init_path  is DIAGONAL then the boundary conditions are adjusted  
**     so that the optimal path enters the cell (0,0) diagonally. Otherwise
**     it enters from the left (ie as a deletion in the EST)
**
** @return [ge_alignment*] Resulting genomic EST alignment
** @@
******************************************************************************/

ge_alignment* non_recursive_est_to_genome( SEQUENCE *est, SEQUENCE *genome,
					   ajint match, ajint mismatch,
					   ajint gap_penalty, ajint intron_penalty,
					   ajint splice_penalty,
					   SEQUENCE *splice_sites,
					   ajint backtrack, ajint needleman,
					   ajint init_path ) {

  unsigned char **ppath=NULL, *path=NULL;
  ajint *score1, *score2;
  ajint *s1, *s2, *s3;
  ajint *best_intron_score, *best_intron_coord;
  ajint e_len_pack = est->len/4+1;
  ajint gpos, epos;
  ajint emax = -1, gmax = -1;
  ajint max_score = 0;
  ajint diagonal, delete_genome, delete_est, intron;
  char *gseq, *eseq, g;
  ajint max, total=0;
  ajint p, pos;
  ajint *temp_path=NULL;
  ajint is_acceptor;
  ge_alignment *ge;
  coords *start1=NULL, *start2=NULL, *t1=NULL, *t2=NULL, *t3;
  coords *best_intron_start=NULL, best_start;
  ajint splice_type=0;

  unsigned char direction;
  unsigned char diagonal_path[4] = { 1, 4, 16, 64 };
  unsigned char delete_est_path[4] = { 2, 8, 32, 128 };
  unsigned char delete_genome_path[4] = { 3, 12, 48, 192 };
  unsigned char mask[4] = { 3, 12, 48, 192 };

  /* path is encoded as 2 bits per cell:
     
     00 intron
     10 diagonal
     01 delete_est
     11 delete_genome
     
     */

  /* the backtrack path, packed 4 cells per byte */

  char dbgmsg[512] = "<undefined>\n";

  if (debug) {
    ajDebug ("non_recursive_est_to_genome\n");
    ajDebug ("   backtrack:%d needleman:%d, init_path:%d\n",
	    backtrack, needleman, init_path);
  }

  AJNEW0 (ge);

  if ( backtrack )
    {
      AJCNEW (ppath, genome->len);
      for(gpos=0;gpos<genome->len;gpos++)
	AJCNEW (ppath[gpos], e_len_pack);      
      AJCNEW (temp_path,  genome->len+est->len);
    }
  else
    {
      AJCNEW (start1, est->len+1);
      AJCNEW (start2, est->len+1);
      AJCNEW (best_intron_start, est->len);

      t1 = start1+1;
      t2 = start2+1;
    }

  AJCNEW (score1, est->len+1);
  AJCNEW (score2, est->len+1);
  
  s1 = score1+1;
  s2 = score2+1;

  AJCNEW (best_intron_coord, est->len+1);
  AJCNEW (best_intron_score, est->len+1);
    
  gseq = downcase(ajSysStrdup(genome->s));
  eseq = downcase(ajSysStrdup(est->s));
  
  if ( ! backtrack ) {/* initialise the boundaries for the start points */
    for(epos=0;epos<est->len;epos++)
      {
	t1[epos].left = 0;
	t1[epos].right = epos;
	t2[epos].left = 0;	/* try initializing t2 explicitly */
	t2[epos].right = epos;	/* otherwise it gets missed on first pass */
	best_intron_start[epos] = t1[epos];
      }
  }
  if ( needleman ) {
    for(epos=0;epos<est->len;epos++)
      {
	s1[epos] = MINUS_INFINITY;
	best_intron_score[epos] = MINUS_INFINITY;
      }
  }
  
  for(gpos=0;gpos<genome->len;gpos++) /* loop thru GENOME sequence */
    {
      s3 = s1; s1 = s2; s2 = s3;
      
      g = gseq[gpos];

      if ( backtrack )
	path = ppath[gpos];
      else
	{
	  t3 = t1; t1 = t2; t2 = t3;
	  t2[-1].left = gpos;	/* set start1[0] to (gpos,0) */
	  t1[-1].left = gpos;	/* set start1[0] to (gpos,0) */
	  t1[-1].right = 0;
	}

      if ( splice_sites && (splice_sites->s[gpos] & ACCEPTOR ) )
	is_acceptor = 1;	/* gpos is last base of putative intron */
      else
	is_acceptor = 0;

/* initialisation */

      if ( needleman )
	{
	  if ( init_path == DIAGONAL || gpos > 0  )
	    s1[-1] = MINUS_INFINITY;
	  else
	    s1[-1] = 0;
	}
      else
	s1[-1] = 0;
      
      for(epos=0;epos<est->len;epos++) /* loop thru EST sequence */
	{
	  /* align est and genome */

	  diagonal = s2[epos-1] + lsimmat[(ajint)g][(ajint)eseq[epos]];
	  
	  /* single deletion in est */

	  delete_est = s1[epos-1] - gap_penalty;

	  /* single deletion in genome */

	  delete_genome = s2[epos] - gap_penalty;

	  /* intron in genome, possibly modified by
	     donor-acceptor splice sites */

	  if ( is_acceptor &&
	      (splice_sites->s[best_intron_coord[epos]] & DONOR ) )
	    intron = best_intron_score[epos] - splice_penalty;
	  else
	    intron = best_intron_score[epos] - intron_penalty;
	    
	  if ( delete_est > delete_genome )
	    max = delete_est;
	  else
	    max = delete_genome;

	  if ( diagonal > max )
	    max = diagonal;
	  
	  if ( intron > max )
	    max = intron;

	  if ( needleman || max > 0 ) /* save this score */
	    {
	      if ( max == diagonal ) /* match extension */
		{
		  s1[epos] = diagonal;
		  if ( backtrack ) {
/*		    path[epos/4] |= diagonal_path[epos%4]; <mod> */
		    path[epos/4] =  ajSysItoUC((ajuint) path[epos/4] |
					       (ajuint) diagonal_path[epos%4]);
		  }
		  else
		    {
		      if ( t2[epos-1].left == -1 ) /* SW start */
			{
			  t1[epos].left = gpos;
			  t1[epos].right = epos;
			  if (debug && t1[epos].left == 10126)
			    (void) sprintf (dbgmsg,
				     "t1[%d].left = gpos:%d\n",
				     epos, gpos);
			}
		      else {	/* continue previous match */
			t1[epos] = t2[epos-1];
			if (debug && t1[epos].left == 10126)
			  (void) sprintf (dbgmsg,
				   "t1[%d] = t2[epos-1] left:%d right:%d gpos: %d(a)\n",
				   epos, t1[epos].left, t1[epos].right, gpos);
		      }
		    }
		}
	      else if ( max == delete_est ) /* (continue) gap in EST */
		{
		  s1[epos] = delete_est;
		  if ( backtrack ) { /* <mod> */
		    path[epos/4]  =  ajSysItoUC((ajuint) path[epos/4] |
						(ajuint) delete_est_path[epos%4]);
		  }
		  else {
		    t1[epos] = t1[epos-1];
		    if (debug && t1[epos].left == 10126)
		      (void) sprintf (dbgmsg,
			       "t1[%d] = t2[epos-1] left:%d (b)\n",
			       epos, t1[epos].left);
		  }
		}
	      else if ( max == delete_genome ) /* (continue) gap in GENOME */
		{
		  s1[epos] = delete_genome;
		  if ( backtrack ) { /* <mod> */
		    path[epos/4] = ajSysItoUC((ajuint) path[epos/4] |
					      (ajuint) delete_genome_path[epos%4]);
		  }
		  else {
		    t1[epos] = t2[epos];
		    if (debug && t1[epos].left == 10126)
		      (void) sprintf (dbgmsg,
			       "t1[%d] = t2[epos] left:%d\n",
			       epos, t1[epos].left);
		  }
		}
	      else		/* Intron */
		{
		  s1[epos] = intron;
		  if ( ! backtrack )
		    t1[epos] = best_intron_start[epos];
		}
	    }
	  else			/* not worth saving (SW with score < 0 ) */
	    {
	      s1[epos] = 0;
	      if ( ! backtrack )
		{
		  t1[epos].left = -1;
		  t1[epos].right = -1;
		}
	    }

	  if ( best_intron_score[epos] < s1[epos] )
	    {
	      /* if ( intron > 0 ) */ /* will only need to store
		                         if this path is positive */
		if ( backtrack )
		  if ( do_not_forget(epos,
				     best_intron_coord[epos]) == 0 )
				/* store the previous path just
				   in case we need it */

		    {		/* error - stack ran out of memory. Clean up
				   and return NULL */

		      ajErr ("stack ran out of memory, returning NULL");

		      AJFREE (score1);
		      AJFREE (score2);
		      AJFREE (eseq);
		      AJFREE (gseq);
		      AJFREE (best_intron_score);
		      AJFREE (best_intron_coord);
		      AJFREE (temp_path);
		      for(gpos=0;gpos<genome->len;gpos++)
			AJFREE (ppath[gpos]);
		      AJFREE (ppath);
		      free_rpairs();
		      AJFREE (ge);

		      return NULL;
		    }

	      best_intron_score[epos] = s1[epos];
	      best_intron_coord[epos] = gpos;
	      if ( ! backtrack )
		best_intron_start[epos] = t1[epos];
	    }

	  if ( ! needleman && max_score < s1[epos] )
	    {
	      max_score = s1[epos];
	      emax = epos;
	      gmax = gpos;
	      if ( ! backtrack ) {
		best_start = t1[epos];
		if (debug)
		  ajDebug ("max_score: %d best_start = t1[%d] left:%d right:%d\n",
			  max_score, epos, best_start.left, best_start.right);
		if (debug)
		  ajDebug ("t1 from :%s\n", dbgmsg);
	      }
	    }
	}
    }

  /* back track */

  if ( needleman )
    {
      ge->gstop = genome->len-1;
      ge->estop = est->len-1;
      ge->score = s1[ge->estop];
    }
  else
    {
      ge->gstop = gmax;
      ge->estop = emax;
      ge->score = max_score;
    }

  if ( backtrack )
    {
      pos = 0;
      
      epos = ge->estop;
      gpos = ge->gstop;
      total = 0;

      /* determine the type of spliced intron (forward or reversed) */
      
      if ( splice_sites ) {
	if ( ! strcmp( splice_sites->desc, "forward") )
	  splice_type = FORWARD_SPLICED_INTRON;
	else if ( ! strcmp( splice_sites->desc, "reverse") )
	  splice_type = REVERSE_SPLICED_INTRON;
	else 
	  splice_type = INTRON; /* This is really an error - splice_sites
				   MUST have a direction */
      }
	  
      while( ( needleman || total < max_score) && epos >= 0 && gpos >= 0 )
	{
	  direction = ajSysItoUC(( (ajuint)ppath[gpos][epos/4] & (ajuint)mask[epos%4] ) >> (2*(epos%4))); 
	  temp_path[pos++] = direction;
	  if ( (ajuint) direction == INTRON ) /* intron */
	    {
	      ajint gpos1;

	      if ( gpos-best_intron_coord[epos]  <= 0 )
		{
 		  if ( verbose ) 
		    (void) ajWarn("NEGATIVE intron gpos: %d %d\n",
				  gpos, gpos-best_intron_coord[epos] ); 
		  gpos1 = remember(epos, gpos ); 
		}
	      else
		{
		  gpos1 = best_intron_coord[epos];	      
		}

	      if ( splice_sites && (splice_sites->s[gpos] & ACCEPTOR ) &&
		  ( splice_sites->s[gpos1] & DONOR ) )
		{
		  total -= splice_penalty;
		  temp_path[pos-1] = splice_type; /* make note that this
						     is a proper intron */
		}
	      else
		{
		  total -= intron_penalty;
		}

	      temp_path[pos++] = gpos-gpos1; /* intron this far */
	      gpos = gpos1;
	    }
	  else if ( (ajuint) direction == DIAGONAL ) /* diagonal */
	    {
	      total += lsimmat[(ajint)gseq[gpos]][(ajint)eseq[epos]];
	      epos--;
	      gpos--;
	    }
	  else if ( (ajuint) direction == DELETE_EST ) /* delete_est */
	    {
	      total -= gap_penalty;
	      epos--;
	    }
	  else			/* delete_genome */
	    {
	      total -= gap_penalty;
	      gpos--;
	    }
	}
      
      gpos++;
      epos++;
      
      
      ge->gstart = gpos;
      ge->estart = epos;
      ge->len = pos;
      if (debug)
	ajDebug ("gstart = gpos (a) : %d\n", ge->gstart);
      
      AJCNEW (ge->align_path, ge->len);

      /* reverse the ge so it starts at the beginning of the sequences */

      for(p=0;p<ge->len;p++)
	{
	  if ( temp_path[p] > INTRON ) /* can be INTRON or
					  FORWARD_SPLICED_INTRON or
					  REVERSE_SPLICED_INTRON */
	    ge->align_path[pos-p-1] = temp_path[p];
	  else
	    {
	      ge->align_path[pos-p-2] = temp_path[p];
	      ge->align_path[pos-p-1] = temp_path[p+1];
	      p++;
	    }
	}
    }
  else
    {
      ge->gstart = best_start.left;
      ge->estart = best_start.right;
      if (debug)
	ajDebug ("gstart = best_start.left : %d\n", ge->gstart);
    }

  AJFREE (score1);
  AJFREE (score2);
  AJFREE (eseq);
  AJFREE (gseq);
  AJFREE (best_intron_score);
  AJFREE (best_intron_coord);

  if ( backtrack )
    {
      AJFREE (temp_path);
      
      for(gpos=0;gpos<genome->len;gpos++)
	AJFREE (ppath[gpos]);
      AJFREE (ppath);
      free_rpairs();
    }
  else
    {
      AJFREE (start1);
      AJFREE (start2);
      AJFREE (best_intron_start);
    }

  if ( verbose )
    {
      indent();
      (void) ajDebug("non-recursive score %d total: %d gstart %d estart %d "
		     "gstop %d estop %d\n",
		     ge->score, total, ge->gstart, ge->estart,
		     ge->gstop, ge->estop );
    }

  return ge;
}

/* @func free_ge ************************************************************
**
** Free a genomic EST alignment structure
**
** @param [r] ge [ge_alignment*] Genomic EST alignment data structure
**
** @return [void]
** @@
******************************************************************************/

void free_ge( ge_alignment *ge ) {

  if ( ge )
    {
      if ( ge->align_path )
	AJFREE ( ge->align_path );
      AJFREE (ge);
    }
}
     
/* @func  recursive_est_to_genome *********************************************
**
** Modified Smith-Waterman/Needleman to align an EST or mRNA to a Genomic
**     sequence, allowing for introns
**
** @param [r] est [SEQUENCE*] Sequence of EST
** @param [r] genome [SEQUENCE*] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [SEQUENCE*] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] max_area [float] Maximum memory available for alignment
**            by standard method (allowing 4 bases per byte).
**            Otherwise sequences are split and aligned recursively.
** @param [r] init_path [ajint] Type of initialization for the path.
**     If init_path  is DIAGONAL then the boundary conditions are adjusted  
**     so that the optimal path enters the cell (0,0) diagonally. Otherwise
**     it enters from the left (ie as a deletion in the EST)
**
** @return [ge_alignment*] Resulting genomic EST alignment
** @@
******************************************************************************/

ge_alignment* recursive_est_to_genome( SEQUENCE *est, SEQUENCE *genome,
				       ajint match, ajint mismatch,
				       ajint gap_penalty, ajint intron_penalty,
				       ajint splice_penalty,
				       SEQUENCE *splice_sites, float max_area,
				       ajint init_path ) {

  ajint middle, gleft, gright, score, i, j;
  SEQUENCE *left_splice=NULL, *right_splice=NULL;
  SEQUENCE *left_genome, *right_genome;
  SEQUENCE *left_est, *right_est;
  ge_alignment *left_ge, *right_ge, *ge;
  float area;
  ajint split_on_del;

  if (debug)
    ajDebug ("recursive_est_to_genome\n");

  area = ((float)genome->len+(float)1.0)*((float)est->len+(float)1.0)/(float)4; /* divide by 4 as
							      we pack 4 cells
							      per byte */

  indentation += 3;

  if ( area <= max_area )	/* sequences small enough to align
				   by standard methods */
    {
      if ( verbose ) {
	indent();
	(void) ajDebug("using non-recursive alignment %d %d   %g %g\n",
		      genome->len, est->len, area, max_area );
      }
      ge = non_recursive_est_to_genome( est, genome, match, mismatch,
				       gap_penalty, intron_penalty,
				       splice_penalty, splice_sites,
				       1, 1, DIAGONAL );

      if ( ge != NULL ) { /* success */
	if (debug)
	  ajDebug ("success returns ge gstart:%d estart:%d gstop:%d estop:%d\n",
		  ge->gstart, ge->estart, ge->gstop, ge->estop);
	return ge;
      }
      else /* failure because we ran out of memory */
	{
	  indentation -= 3;
	  if ( verbose ) {
	    indent();
	    (void) ajDebug("Stack memory overflow ... splitting\n");
	  }
	}
    }
  /* need to recursively split */

  if ( verbose ) {
    indent();
    (void) ajDebug("splitting genome and est\n");
  }

  middle = est->len/2;
  
  score = midpt_est_to_genome( est, genome, match, mismatch, gap_penalty,
			      intron_penalty, splice_penalty, splice_sites,
			      middle, &gleft, &gright );
  if ( verbose ) {
    indent();
    (void) ajDebug("score %d middle %d gleft %d gright %d\n",
		   score, middle, gleft, gright );
  }
  
  split_on_del =  ( gleft == gright );
  
  
  /* split genome */
  
  left_genome = subseq( genome, 0, gleft );
  right_genome = subseq( genome, gright, genome->len-1);
  if ( splice_sites )
    {
      left_splice = subseq( splice_sites, 0, gleft );
      right_splice = subseq( splice_sites, gright, genome->len-1);
    }
  /* split est */
  
  left_est = subseq( est, 0, middle );
  right_est = subseq( est, middle+1, est->len-1 );

  /* align left and right parts separately */

  if ( verbose ) {
    indent();
    (void) ajDebug ("LEFT\n");
  }
  left_ge = recursive_est_to_genome( left_est, left_genome, match, mismatch,
				    gap_penalty, intron_penalty,
				    splice_penalty, left_splice, max_area,
				    DIAGONAL );  

  if ( verbose ) {
    indent();
    (void) ajDebug ("RIGHT\n");
  }
  right_ge = recursive_est_to_genome( right_est, right_genome, match,
				     mismatch, gap_penalty, intron_penalty,
				     splice_penalty, right_splice, max_area,
				     DIAGONAL );  


      /* merge the alignments */

  AJNEW0 (ge);
  ge->score = left_ge->score + right_ge->score;
  ge->gstart = 0;
  ge->estart = 0;
  ge->gstop = genome->len-1;
  ge->estop = est->len-1;

  ge->len = left_ge->len+right_ge->len;
  AJCNEW (ge->align_path, ge->len);

  for(i=0,j=0;j<left_ge->len;i++,j++)
    ge->align_path[i] = left_ge->align_path[j];
	  
  if ( split_on_del ) /* merge on an est deletion */
    {
      indent();
      (void) ajDebug ("split_on_del\n");
      ge->align_path[i++] = DELETE_EST;
      for(j=1;j<right_ge->len;i++,j++) /* omit first symbol on
					  right-hand alignment */
	ge->align_path[i] = right_ge->align_path[j];
    }
  else
    for(j=0;j<right_ge->len;i++,j++)
      ge->align_path[i] = right_ge->align_path[j];
	    

  free_seq(left_est); free_seq(right_est);
  free_seq(left_genome); free_seq( right_genome);
  if ( splice_sites ) { free_seq(left_splice); free_seq( right_splice); }
  free_ge( left_ge); free_ge( right_ge );

  indentation -= 3;
  if (verbose)
    (void) ajDebug ("end returns ge gstart:%d estart:%d gstop:%d estop:%d\n",
		    ge->gstart, ge->estart, ge->gstop, ge->estop);
  return ge;
}


/* @func  linear_space_est_to_genome ******************************************
**
** Align EST sequence to genomic in linear space
**
** @param [r] est [SEQUENCE*] Sequence of EST
** @param [r] genome [SEQUENCE*] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [SEQUENCE*] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] megabytes [float] Maximum memory allowed in Mbytes for
**        alignment by standard methods.
**
** @return [ge_alignment*] Genomic EST alignment
** @@
******************************************************************************/

ge_alignment * linear_space_est_to_genome( SEQUENCE *est, SEQUENCE *genome,
					  ajint match, ajint mismatch,
					  ajint gap_penalty, ajint intron_penalty,
					  ajint splice_penalty,
					  SEQUENCE *splice_sites,
					  float megabytes ) {

  ge_alignment *ge, *rge;
  SEQUENCE *genome_subseq, *est_subseq, *splice_subseq;
  float area;
  float max_area = megabytes*(float)1.0e6;

  rpair_init( (ajint)((float)1.0e6*megabytes) );

  area = ((float)genome->len+(float)1.0)*((float)est->len+(float)1.0)/(float)4; /* divide by 4
							      as we pack 4
							      cells per byte */

  ajDebug("area %.6f max_area %.6f\n", area, max_area);
/* sequences small enough to align by standard methods ?*/

  if ( area <= max_area ) 
  {
    ajDebug("call non_recursive_est_to_genome\n");
     return non_recursive_est_to_genome( est, genome, match, mismatch,
					 gap_penalty, intron_penalty,
					 splice_penalty, splice_sites,
					 1, 0, DIAGONAL );
  }

/* need to recursively split */

  /* first do a Smith-Waterman without backtracking to find
     the start and end of the alignment */
      
  ge = non_recursive_est_to_genome( est, genome, match, mismatch,
                                     gap_penalty, intron_penalty,
                                     splice_penalty, splice_sites,
                                     0, 0, DIAGONAL );  

  /* extract subsequences corresponding to the aligned regions */

  if ( verbose ) {
     indent();
     (void) ajDebug ("sw alignment score %d gstart %d estart %d "
		     "gstop %d estop %d\n", ge->score, ge->gstart,
		     ge->estart, ge->gstop, ge->estop ); 
  }
  genome_subseq = subseq(genome, ge->gstart, ge->gstop );
  est_subseq = subseq(est, ge->estart, ge->estop );
  if ( splice_sites ) 
    splice_subseq = subseq(splice_sites, ge->gstart, ge->gstop );
  else
    splice_subseq = NULL;

  /* recursively do the alignment */

  rge = recursive_est_to_genome( est_subseq, genome_subseq, match,
                                 mismatch, gap_penalty, intron_penalty,
                                 splice_penalty, splice_subseq, max_area,
                                 DIAGONAL );  

  ge->len = rge->len;
  ge->align_path = rge->align_path;

  AJFREE (rge);
  free_seq(genome_subseq);
  free_seq(est_subseq);
  if ( splice_subseq ) 
  AJFREE (splice_subseq);
      
  return ge;
}
     

/* @func  midpt_est_to_genome *************************************************
**
** Modified Needleman-Wunsch to align an EST or mRNA to a Genomic
** sequence, allowing for introns. The recursion is
**
**     {  S[gpos-1][epos]   - gap_penalty
**     
**     {  S[gpos-1][epos-1] + D[gpos][epos]
**     
**     S[gpos][epos] = max {  S[gpos][epos-1]   - gap_penalty
**     
**     {  C[epos]           - intron_penalty 
**     
**     C[epos] = max{ S[gpos][epos], C[epos] }
**     
**     S[gpos][epos] is the score of the best path to the cell gpos, epos 
**     C[epos] is the score of the best path to the column epos
**     
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
**     
**     NB: IMPORTANT:
**     
**     The input sequences are assumed to be subsequences chosen so
**     that they align end-to end, with NO end gaps. Call
**     non_recursive_est_to_genome() to get the initial max scoring
**     segment and chop up the sequences appropriately.
**     
**     The return value is the alignment score.
**
**     If the alignment crosses middle by a est deletion (ie horizontally) then
**              gleft == gright
**     
** @param [r] est [SEQUENCE*] Sequence of EST
** @param [r] genome [SEQUENCE*] Sequence of genomic region
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty (positive)
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] splice_sites [SEQUENCE*] Marked splice sites.
**     The intron_penalty may be modified to splice_penalty if splice_sites is
**     non-null and there are DONOR and ACCEPTOR sites at the start and
**     end of the intron.
** @param [r] middle [ajint] Sequence mid point position.
**     This Function does not compute the path, instead it finds the
**     genome coordinates where the best path crosses epos=middle, so this
**     should be called recursively to generate the complete alignment in
**     linear space.
** @param [r] gleft [int*] genome left coordinate at the crossing point.
**     If the alignment crosses middle in a diagonal fashion then
**              gleft+1 == gright
** @param [r] gright [int*] genome right coordinate at the crossing point.
**     If the alignment crosses middle in a diagonal fashion then
**              gleft+1 == gright
**
** @return [ajint] alignment score
** @@
******************************************************************************/

ajint midpt_est_to_genome( SEQUENCE *est, SEQUENCE *genome, ajint match,
			ajint mismatch, ajint gap_penalty, ajint intron_penalty,
			ajint splice_penalty, SEQUENCE *splice_sites,
			ajint middle, ajint *gleft, ajint *gright ) {

  ajint *score1, *score2;
  ajint *s1, *s2, *s3;
  ajint *best_intron_score, *best_intron_coord;
  ajint gpos, epos;
  ajint score;
  ajint diagonal, delete_genome, delete_est, intron;
  char *gseq, *eseq, g;
  ajint max;
  ajint is_acceptor;
  coords *m1, *m2, *m3;
  coords *midpt1, *midpt2, *best_intron_midpt;

  AJCNEW (score1, est->len+1);
  AJCNEW (score2, est->len+1);
  
  s1 = score1+1;
  s2 = score2+1;

  AJCNEW (midpt1, est->len+1);
  AJCNEW (midpt2, est->len+1);
  
  m1 = midpt1+1;
  m2 = midpt2+1;

  AJCNEW (best_intron_coord, est->len+1);
  AJCNEW (best_intron_score, est->len+1);
  AJCNEW (best_intron_midpt, est->len+1);
    
  gseq = downcase(ajSysStrdup(genome->s));
  eseq = downcase(ajSysStrdup(est->s));

  middle++;


  /* initialise the boundary: We want the alignment to start at [0,0] */

  for(epos=0;epos<est->len;epos++)
    {
      s1[epos] = MINUS_INFINITY;
      best_intron_score[epos] = MINUS_INFINITY;
    }

  for(gpos=0;gpos<genome->len;gpos++)
    {
      s3 = s1; s1 = s2; s2 = s3;
      m3 = m1; m1 = m2; m2 = m3;

      g = gseq[gpos];

      if ( splice_sites && ( splice_sites->s[gpos] & ACCEPTOR ) )
	is_acceptor = 1;	/* gpos is last base of putative intron */
      else
	is_acceptor = 0;

      
/* boundary conditions */
      
      s1[-1] = MINUS_INFINITY;

/* the meat */

      for(epos=0;epos<est->len;epos++)
	{
	  /* align est and genome */

	  diagonal = s2[epos-1] + lsimmat[(ajint)g][(ajint)eseq[epos]];
	  
	  /* single deletion in est */

	  delete_est = s1[epos-1] - gap_penalty;

	  /* single deletion in genome */

	  delete_genome = s2[epos] - gap_penalty;

	  /* intron in genome, possibly modified by
	     donor-acceptor splice sites */

	  if ( is_acceptor &&
	      (splice_sites->s[best_intron_coord[epos]] & DONOR ) )
	    intron = best_intron_score[epos] - splice_penalty;
	  else
	    intron = best_intron_score[epos] - intron_penalty;
	    
	  if ( delete_est > delete_genome )
	    max = delete_est;
	  else
	    max = delete_genome;

	  if ( diagonal > max )
	    max = diagonal;
	  
	  if ( intron > max )
	    max = intron;

	  if ( max == diagonal )
	    {
	      s1[epos] = diagonal;
	      if ( epos == middle ) 
		{
		  m1[epos].left = gpos-1;
		  m1[epos].right = gpos;
		}
	      else
		m1[epos] = m2[epos-1];
	    }
	  else if ( max == delete_est )
	    {
	      s1[epos] = delete_est;
	      if ( epos == middle )
		{
		  m1[epos].left = gpos;
		  m1[epos].right = gpos;
		}
	      else
		m1[epos] = m1[epos-1];
	    }
	  else if ( max == delete_genome )
	    {
	      s1[epos] = delete_genome;
	      m1[epos] = m2[epos];
	    }
	  else			/* intron */
	    {
	      s1[epos] = intron;
	      m1[epos] = best_intron_midpt[epos];
	    }

	  if ( best_intron_score[epos] < s1[epos] )
	    {
	      best_intron_score[epos] = s1[epos];
	      best_intron_coord[epos] = gpos;
	      best_intron_midpt[epos] = m1[epos];
	    }
	}
    }

  *gleft = m1[est->len-1].left;
  *gright = m1[est->len-1].right;
  score = s1[est->len-1];

  if ( verbose ) {
    indent();
    (void) ajDebug ("midpt score %d middle %d gleft %d gright %d "
		    "est->len %d genome->len %d\n",
		    score, middle-1, *gleft, *gright, est->len,
		    genome->len );
  }

  AJFREE (score1);
  AJFREE (score2);
  AJFREE (midpt1);
  AJFREE (midpt2);
  AJFREE (eseq);
  AJFREE (gseq);
  AJFREE (best_intron_score);
  AJFREE (best_intron_coord);

  return score;
}

/* @func  blast_style_output *************************************************
**
** output in blast style.
**
** @param [r] blast [AjPFile] Output file
** @param [r] genome [SEQUENCE*] Genomic sequence
** @param [r] est [SEQUENCE*] EST sequence
** @param [r] ge [ge_alignment*] Genomic EST alignment
** @param [r] match [ajint] Match score
** @param [r] mismatch [ajint] Mismatch penalty
** @param [r] gap_penalty [ajint] Gap penalty
** @param [r] intron_penalty [ajint] Intron penalty
** @param [r] splice_penalty [ajint] Splice site penalty
** @param [r] gapped [ajint] Boolean. 1 = write a gapped alignment
** @param [r] reverse [ajint] Boolean. 1 = reverse alignment
**
** @return [void]
** @@
******************************************************************************/

void blast_style_output( AjPFile blast, SEQUENCE *genome, SEQUENCE *est,
			ge_alignment *ge, ajint match, ajint mismatch,
			ajint gap_penalty, ajint intron_penalty,
			ajint splice_penalty, ajint gapped, ajint reverse  ) {

  ajint gsub, gpos, esub, epos, tsub, p;
  ajint matches=0, len=0, m;
  ajint total_matches=0, total_len=0;
  float percent;

  if (verbose)
    ajDebug ("debugging set to %d\n", debug);

  gsub = gpos = ge->gstart;
  esub = epos = ge->estart;

  if (verbose) {
    ajDebug("blast_style_output\n");
    ajDebug("gsub %d esub %d\n", gsub, esub);
  }

  if ( blast )
    {
      tsub = 0;
      for(p=0;p<ge->len;p++)
	if ( ge->align_path[p] <= INTRON )
	  {
	    write_MSP( blast, &matches, &len, &tsub, genome, gsub, gpos,
		      est, esub, epos, reverse, gapped);
	    if ( gapped )
	      {
		if ( ge->align_path[p] == INTRON )
		  {
		      ajFmtPrintF( blast,
			      "?Intron  %5d %5.1f %5d %5d %-12s\n",
			      -intron_penalty, (float) 0.0, gpos+1,
			      gpos+ge->align_path[p+1], genome->name ); 
		  }
		else /* proper intron */
		  {
		    if ( ge->align_path[p] == FORWARD_SPLICED_INTRON )
		      ajFmtPrintF( blast,
			      "+Intron  %5d %5.1f %5d %5d %-12s\n",
			      -splice_penalty, (float) 0.0, gpos+1,
			      gpos+ge->align_path[p+1], genome->name ); 
		    else
		      ajFmtPrintF( blast,
			      "-Intron  %5d %5.1f %5d %5d %-12s\n",
			      -splice_penalty, (float) 0.0, gpos+1,
			      gpos+ge->align_path[p+1], genome->name ); 

		  }
	      }

	    gpos += ge->align_path[++p];
	    esub = epos;
	    gsub = gpos;
	  }
	else if ( ge->align_path[p] == DIAGONAL )
	  {
	    m = lsimmat[(ajint)genome->s[(ajint)gpos]][(ajint)est->s[(ajint)epos]];
	    tsub += m;
	    if ( m > 0 )
	      {
		matches++;
		total_matches++;
	      }
	    len++;
	    total_len++;
	    gpos++;
	    epos++;
	  }
	else if ( ge->align_path[p] == DELETE_EST )
	  {
	    if ( gapped )
	      {
		tsub -= gap_penalty;
		epos++;
		len++;
		total_len++;
	      }
	    else
	      {
		write_MSP( blast, &matches, &len, &tsub, genome, gsub,
			  gpos, est, esub, epos, reverse, gapped );
		epos++;
		esub = epos;
		gsub = gpos;
	      }
	  }
	else if ( ge->align_path[(ajint)p] == DELETE_GENOME )
	  {
	    if ( gapped )
	      {
		tsub -= gap_penalty;
		gpos++;
		total_len++;
		len++;
	      }
	    else
	      {
		write_MSP( blast, &matches, &len, &tsub, genome, gsub,
			   gpos, est, esub, epos, reverse, gapped );
		gpos++;
		esub = epos;
		gsub = gpos;
	      }
	  }
      write_MSP( blast, &matches, &len, &tsub, genome, gsub, gpos, est,
		 esub, epos, reverse, gapped );

      if ( gapped )
	{
	  if ( total_len > 0 )
	    percent = (total_matches/(float)(total_len))*(float)100.0;
	  else
	    percent = (float) 0.0;

	  if ( reverse )
	    ajFmtPrintF( blast,
		    "\nSpan     %5d %5.1f %5d %5d %-12s %5d %5d %-12s  %s\n",
		    ge->score, percent, ge->gstart+1, ge->gstop+1,
		    genome->name, est->len-ge->estart, est->len-ge->estop,
		    est->name, est->desc );
	  else
	    ajFmtPrintF( blast,
		    "\nSpan     %5d %5.1f %5d %5d %-12s %5d %5d %-12s  %s\n",
		    ge->score, percent, ge->gstart+1, ge->gstop+1,
		    genome->name, ge->estart+1, ge->estop+1,  est->name,
		    est->desc );
	}

    }
}
/* @func write_MSP ***********************************************************
**
** write out the MSP (maximally scoring pair).
**
** @param [r] ofile [AjPFile] Output file
** @param [r] matches [int*] Number of matches found
** @param [r] len [int*] Length of alignment
** @param [r] tsub [int*] Score
** @param [r] genome [SEQUENCE*] Genomic sequence
** @param [r] gsub [ajint] Genomic start position
** @param [r] gpos [ajint] Genomic end position
** @param [r] est [SEQUENCE*] EST sequence
** @param [r] esub [ajint] EST start position
** @param [r] epos [ajint] EST end position
** @param [r] reverse [ajint] Boolean 1=reverse the EST sequence
** @param [r] gapped [ajint] Boolean 1=full gapped alignment
**                         0=display ungapped segment
**
** @return [void]
** @@
******************************************************************************/

void write_MSP( AjPFile ofile, ajint *matches, ajint *len, ajint *tsub,
	       SEQUENCE *genome, ajint gsub, ajint gpos, SEQUENCE *est,
	       ajint esub, ajint epos, ajint reverse, ajint gapped) {

  float percent;

  if ( *len > 0 )
    percent = (*matches/(float)(*len))*(float)100.0;
  else
    percent = (float) 0.0;

  if ( percent > 0 )
    {
      if ( gapped )
	ajFmtPrintF( ofile, "Exon     " );
      else
	ajFmtPrintF( ofile, "Segment  " );
      if ( reverse )
	ajFmtPrintF(ofile, "%5d %5.1f %5d %5d %-12s %5d %5d %-12s  %s\n", 
		*tsub, percent, gsub+1, gpos, genome->name,
		est->len-esub,  est->len-epos+1, est->name, est->desc );
      else
	ajFmtPrintF(ofile, "%5d %5.1f %5d %5d %-12s %5d %5d %-12s  %s\n",
		*tsub, percent, gsub+1, gpos, genome->name,
		esub+1, epos, est->name, est->desc );
    }
  *matches = *len = *tsub = 0;
}

/* @func print_align *********************************************************
**
** Print the alignment
**
** @param [r] ofile [AjPFile] Output file
** @param [r] genome [SEQUENCE*] Genomic sequence
** @param [r] est [SEQUENCE*] EST sequence
** @param [r] ge [ge_alignment*] Genomic EST alignment
** @param [r] width [ajint] Output width (in bases)
**
** @return [void]
** @@
******************************************************************************/

void print_align(AjPFile ofile, SEQUENCE *genome, SEQUENCE *est,
		 ge_alignment *ge, ajint width ) {

  ajint gpos, epos, pos, len, i, j, max, m;
  char *gbuf;
  char *ebuf;
  char *sbuf;
  ajint *gcoord, *ecoord;
  ajint namelen = strlen(genome->name) > strlen(est->name) ?
    strlen(genome->name): strlen(est->name)  ;
  char format[256];
  (void) sprintf(format, "%%%ds %%6d ", namelen );
  if ( ofile )
    {
      ajFmtPrintF(ofile, "\n");
      len = genome->len + est->len + 1;
      
      AJCNEW (gbuf, len);
      AJCNEW (ebuf, len);
      AJCNEW (sbuf, len);
      
      AJCNEW (gcoord, len);
      AJCNEW (ecoord, len);
      
      gpos = ge->gstart;
      epos = ge->estart;
      len = 0;
      for(pos=0;pos<ge->len;pos++)
	{
	  ajint way = ge->align_path[pos];
	  if ( way == DIAGONAL  ) /* diagonal */
	    {
	      gcoord[len] = gpos;
	      ecoord[len] = epos;
	      gbuf[len] = genome->s[gpos++];
	      ebuf[len] = est->s[epos++];
	      m = lsimmat[(ajint)gbuf[len]][(ajint)ebuf[len]];
/* MATHOG, the triple form promotes char to arithmetic type, which 
generates warnings as it might be able overflow the char type.  This is
equivalent but doesn't trigger any compiler noise 
	      sbuf[len] = (char) ( m > 0 ? '|' : ' ' );
*/
	      if(m>0) sbuf[len] = '|';
	      else    sbuf[len] = ' ';
	      len++;
	    }
	  else if ( way == DELETE_EST )
	    {
	      gcoord[len] = gpos;
	      ecoord[len] = epos;
	      gbuf[len] = '-';
	      ebuf[len] = est->s[epos++];
	      sbuf[len] = ' ';
	      len++;
	    }
	  else if ( way == DELETE_GENOME )
	    {
	      gcoord[len] = gpos;
	      ecoord[len] = epos;
	      gbuf[len] = genome->s[gpos++];
	      ebuf[len] = '-';
	      sbuf[len] = ' ';
	      len++;
	    }
	  else if ( way <= INTRON )
	    {
	      /* want enough space to print the first 5 and last 5
		 bases of the intron, plus a string containing the
		 intron length */

	      ajint intron_width = ge->align_path[pos+1];
	      ajint half_width = intron_width > 10 ? 5 : intron_width/2;
	      ajint g = gpos-1;
	      char number[30];
	      ajint numlen;
	      (void) sprintf(number," %d ", intron_width );
	      numlen = strlen(number);

	      for(j=len;j<len+half_width;j++)
		{
		  g++;
		  gcoord[j] = gpos-1;
		  ecoord[j] = epos-1;
		  gbuf[j] = ajSysItoC(tolower((ajint) genome->s[g]));
		  ebuf[j] = '.';
		  if ( way == FORWARD_SPLICED_INTRON )
		    sbuf[j] = '>';
		  else if ( way == REVERSE_SPLICED_INTRON )
		    sbuf[j] = '<';
		  else 
		    sbuf[j] = '?';
		}
	      len = j;
	      for(j=len;j<len+numlen;j++)
		{
		  gcoord[j] = gpos-1;
		  ecoord[j] = epos-1;
		  gbuf[j] = '.';
		  ebuf[j] = '.';
		  sbuf[j] = number[j-len];
		}
	      len = j;
	      g = gpos + intron_width - half_width-1;
	      for(j=len;j<len+half_width;j++)
		{
		  g++;
		  gcoord[j] = gpos-1;
		  ecoord[j] = epos-1;
		  gbuf[j] = ajSysItoC(tolower((ajint) genome->s[g]));
		  ebuf[j] = '.';
		  if ( way == FORWARD_SPLICED_INTRON )
		    sbuf[j] = '>';
		  else if ( way == REVERSE_SPLICED_INTRON )
		    sbuf[j] = '<';
		  else 
		    sbuf[j] = '?';
		}

	      gpos += ge->align_path[++pos];
	      len = j;
	    }
	}
      
      for(i=0;i<len;i+=width)
	{
	  max = ( i+width > len ? len : i+width );

	  ajFmtPrintF(ofile, format, genome->name, gcoord[i]+1 );
	  for(j=i;j<max;j++)
	    ajFmtPrintF(ofile, "%c",  gbuf[j]);
	  ajFmtPrintF(ofile," %6d\n", gcoord[j-1]+1 );

	  for(j=0;j<namelen+8;j++)
	    ajFmtPrintF(ofile, " ");
	  for(j=i;j<max;j++)
	    ajFmtPrintF(ofile,"%c", sbuf[j]);
	  ajFmtPrintF(ofile,  "\n");

	  ajFmtPrintF(ofile, format, est->name, ecoord[i]+1 );
	  for(j=i;j<max;j++)
	    ajFmtPrintF(ofile, "%c", ebuf[j]);
	  ajFmtPrintF(ofile," %6d\n\n", ecoord[j-1]+1 );
	}

      ajFmtPrintF( ofile, "\nAlignment Score: %d\n", ge->score );

      AJFREE (gbuf);
      AJFREE (ebuf);
      AJFREE (sbuf);
      AJFREE (gcoord);
      AJFREE (ecoord);
    }
}  

/* @func find_splice_sites *************************************************
**
** Finds all putative DONOR and ACCEPTOR splice sites in the genomic sequence.
**
** Returns a sequence object whose "dna" should be interpreted as an
** array indicating what kind (if any) of splice site can be found at
** each sequence position.
**     
**     DONOR    sites are NNGTNN last position in exon
**
**     ACCEPTOR sites are NAGN last position in intron
**
**     if forward==1 then search fot GT/AG
**     else               search for CT/AC
**
** @param [r] genome [SEQUENCE*] Genomic sequence
** @param [r] forward [ajint] Boolean. 1 = forward direction
**
** @return [SEQUENCE*] Sequence of bitmask codes for splice sites.
** @@
******************************************************************************/

SEQUENCE* find_splice_sites (SEQUENCE *genome, ajint forward ) {

  SEQUENCE *sites = seqdup( genome );
  ajint pos;
  char *s = genome->s;

  for(pos=0;pos<sites->len;pos++)
    sites->s[pos] = NOT_A_SITE;

  if ( forward ) { /* gene is in forward direction 
		      -splice consensus is gt/ag */
    for(pos=1;pos<sites->len-2;pos++)
      {
	if ( tolower((ajint) s[pos]) == 'g' &&
	     tolower((ajint) s[pos+1]) == 't' ) /* donor */
	  sites->s[pos-1] = ajSysItoC((ajuint) sites->s[pos-1] | (ajuint) DONOR); /* last position in exon */
	if ( tolower((ajint) s[pos]) == 'a' &&
	     tolower((ajint) s[pos+1]) == 'g' ) /* acceptor */
	  sites->s[pos+1]  = ajSysItoC((ajuint) sites->s[pos+1] | (ajuint) ACCEPTOR); /* last position in intron */
      }
    (void) strcpy(sites->desc,"forward"); /* so that other functions know */
  }
  else { /* gene is on reverse strand so splice consensus looks like ct/ac */
    for(pos=1;pos<sites->len-2;pos++)
      {
	if ( tolower((ajint) s[pos]) == 'c' &&
	     tolower((ajint) s[pos+1]) == 't' ) /* donor */
	  sites->s[pos-1] = ajSysItoC((ajuint) sites->s[pos-1] | (ajuint) DONOR); /* last position in exon */
	if ( tolower((ajint) s[pos]) == 'a' &&
	     tolower((ajint) s[pos+1]) == 'c' ) /* acceptor */
	  sites->s[pos+1] = ajSysItoC((ajuint) sites->s[pos+1] | (ajuint) ACCEPTOR); /* last position in intron */
      }
    (void) strcpy(sites->desc,"reverse"); /* so that other functions know */
  }

  return sites;
}

/* @func matinit ************************************************************
**
** Comparison matrix initialisation.
**
** @param [r] match [ajint] Match code
** @param [r] mismatch [ajint] Mismatch penalty
** @param [r] gap [ajint] Gap penalty
** @param [r] neutral [ajint] Score for ambiguous base positions.
** @param [r] pad_char [char] Pad character for gaps in input sequences
**
** @return [void]
** @@
******************************************************************************/

void matinit(ajint match, ajint mismatch, ajint gap, ajint neutral, char pad_char) {

  ajint c1, c2;
  
  for(c1=0;c1<256;c1++)
    for(c2=0;c2<256;c2++)
      {
	if ( c1 == c2 )
	  {
	    if ( c1 != '*' && c1 != 'n' &&  c1 != 'N' && c1 != '-' )
	      lsimmat[c1][c2] = match;
	    else
	      lsimmat[c1][c2] = 0;
	  }
	else
	  {
	    if ( c1 == pad_char || c2 == pad_char )
	      lsimmat[c1][c2] = lsimmat[c2][c1] = -gap;
	    else if( c1 == 'n' || c2 == 'n' || c1 == 'N' || c2 == 'N' ) 
	      lsimmat[c1][c2] = lsimmat[c2][c1] = neutral;
	    else
	      lsimmat[c1][c2] = lsimmat[c2][c1] = -mismatch;
	  }
      }

  for(c1=0;c1<256;c1++)
    {
      c2 = tolower(c1);
      lsimmat[c1][c2] = lsimmat[c1][c1];
      lsimmat[c2][c1] = lsimmat[c1][c1];
    }
}

/* @func  indent ************************************************************
**
** Indent report by printing spaces to standard output.
**
** @return [void]
** @@
******************************************************************************/

void indent( void) {

  ajint n = indentation;

  while(n--)
    (void) fputc(' ',stdout);
}

/* stuff for handling pairs 

Need to store pairs [row, col] so that we can retrieve the nearest
pair less than a given input pair [Row, Col] st Col == col and Row > row

Needed for lin_align.c

*/

#define LIMIT_RPAIR_SIZE 10000

static RPAIR *rpair=NULL;
static ajint rpairs=0;
static ajint rpair_size=0;
static ajint rpairs_sorted=0;
static ajint limit_rpair_size=LIMIT_RPAIR_SIZE;

extern ajint verbose;

/* @func rpair_cmp ***********************************************************
**
** Compare two RPAIR values. Return the column difference, or if
** the columns are the same, return the row difference.
**
** A value of zero means the two RPAIRS are identical.
**
** @param [r] a [const void*] First value
** @param [r] b [const void*] Second value
**
** @return [ajint] difference.
** @@
******************************************************************************/

ajint rpair_cmp( const void *a, const void *b ) {

  RPAIR *A = (RPAIR*)a;
  RPAIR *B = (RPAIR*)b;
  ajint n = A->col - B->col;

  if ( n == 0 )
    n = A->row - B->row;

  return n;
}

/* @func rpair_init **********************************************************
**
** Initialise the rpair settings
**
** @param [r] max_bytes [ajint] Maximum memory size (bytes)
**
** @return [void]
** @@
******************************************************************************/

void rpair_init( ajint max_bytes ) {

  limit_rpair_size = max_bytes/sizeof(RPAIR);
  free_rpairs();
}

/* @func free_rpairs  *********************************************************
**
** Free the rpairs data structure
**
** @return [void]
** @@
******************************************************************************/

void free_rpairs(void) {

  (void) ajDebug("FORGET: rpairs: %d rpair: %x\n", rpairs, rpair);
  if ( rpair ) AJFREE (rpair);
  rpair = NULL;
  rpair_size = 0;
  rpairs = 0;
  rpairs_sorted = 0;
}

/* @func do_not_forget *******************************************************
**
** Saving rpairs row and column values.
**
** @param [r] col [ajint] Current column
** @param [r] row [ajint] Current row
**
** @return [ajint] o upon error.
** @@
******************************************************************************/

ajint do_not_forget( ajint col, ajint row ) {

  if ( rpairs >= limit_rpair_size ) {
    ajErr ("rpairs %d beyond maximum %d", rpairs+1, limit_rpair_size);
    ajErr ("increase space threshold to repeat this search");
    return 0; /* failure - ran out of memory */
  }
  if ( rpairs >= rpair_size ) {
    rpair_size = ( rpairs == 0 ? 10000 : 2*rpairs );

    if ( rpair_size > limit_rpair_size ) /* enforce the limit */
      rpair_size = limit_rpair_size;

    AJCRESIZE (rpair, rpair_size); 
    if (verbose) {
      (void) ajDebug ("rpairs %d allocated rpair_size %d rpair: %x\n",
	       rpairs, rpair_size, rpair);
      (void) ajDebug ("test rpair[0] %x col %d row %d\n",
		      &rpair[0], rpair[0].col, rpair[0].row);
      (void) ajDebug ("test rpair[%d] %x col %d row %d\n",
		      rpairs, &rpair[rpairs],
		      rpair[rpairs].col, rpair[rpairs].row);
      (void) ajDebug ("test rpair[%d] %x col %d row %d\n",
		      rpair_size-1, &rpair[rpair_size-1],
		      rpair[rpair_size-1].col, rpair[rpair_size-1].row);
    }
  }

  rpair[rpairs].col = col;
  rpair[rpairs].row = row;

  rpairs++;
  rpairs_sorted = 0;

  return 1; /* success */
}

/* @func remember ************************************************************
**
** Recall rpair values for row and column
**
** @param [r] col [ajint] Current column
** @param [r] row [ajint] Current row
**
** @return [ajint] Row number
** @@
******************************************************************************/

ajint remember( ajint col, ajint row ) {

  RPAIR rp;
  ajint left, right, middle, d;
  ajint bad;

  if ( ! rpairs_sorted ) {
    qsort( rpair, rpairs, sizeof(RPAIR), rpair_cmp );
    rpairs_sorted = 1;
  }

  rp.col = col;
  rp.row = row;

  left = 0;
  right = rpairs-1;

  ajDebug ("remember left: %d right: %d rp rp.col rp.row\n",
	   left, right, rp, rp.col, rp.row);
  
/* MATHOG, changed flow somewhat, added "bad" variable, inverted some
tests ( PLEASE CHECK THEM!  I did this because the original version
had two ways to drop into the failure code, but one of them was only
evident after careful reading of the code.  */

  if ( (rpair_cmp( &rpair[left],&rp ) > 0 ) ||
       (rpair_cmp(&rpair[right],&rp ) < 0 ) ) {
       bad = 1; /*MATHOG, preceding test's logic was inverted */
  }
  else {
    bad = 0;
    while( right-left > 1 ) {	/* binary check for row/col */
      middle = (left+right)/2;
      d = rpair_cmp( &rpair[middle], &rp );
      if ( d < 0 ) 
	left = middle;
      else if ( d >= 0 )
	right = middle;
    }
     ajDebug (
	      "col %d row %d found right: col %d row %d left: col %d row %d\n",
	      col, row, rpair[right].col, rpair[right].row, rpair[left].col,
	      rpair[left].row );

/* any of these fail indicates failure */
/*MATHOG, next test's logic was inverted */
    if ( rpair_cmp( &rpair[right], &rp ) < 0 ||
	 rpair[left].col != col ||
         rpair[left].row >= row ) {
      ajDebug("remember => bad2\n");
      ajDebug("rpair_cmp( %d+%d, %d+%d) %d\n",
	      rpair[right].col, rpair[right].row,
	      rp.col, rp.row,
	      rpair_cmp( &rpair[right], &rp ));
      ajDebug("rpair[left].col %d %d\n", rpair[left].col, col);
      ajDebug("rpair[left].row %d %d\n", rpair[left].row, row);
      bad = 2;
    }
  }

 /* error - this should never happen  */

  if(bad != 0){
    (void) ajFatal("ERROR in remember() left: %d (%d %d) right: %d (%d %d) "
		   "col: %d row: %d, bad: %d\n",
		   left, rpair[left].col, rpair[left].row, right,
		   rpair[right].col, rpair[right].row, col, row, bad);
  }

  return rpair[left].row;
}

#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC ((float)1.0/(float)MBIG)
/* Taken from Numerical recipes in C, Press et al (C) */
/* modified by RFM Feb 1991 to prevent occasional return of NEGATIVE number */

/* @func  rand3 ************************************************************
**
** Random number generator.
**
** @param [r] idum [int*] Seed
**
** @return [float] Random flaoting point number.
** @@
******************************************************************************/

float rand3 (ajint *idum) {

  static ajint inext,inextp;
  static ajlong ma[56];
  static ajint iff=0;
  ajlong mj,mk;
  ajint i,ii,k;
  float ZZ;
	
  if (*idum < 0 || iff == 0) {
    iff=1;
    mj=MSEED-(*idum < 0 ? -*idum : *idum);
    mj %= MBIG;
    ma[55]=mj;
    mk=1;
    for (i=1;i<=54;i++) {
      ii=(21*i) % 55;
      ma[ii]=mk;
      mk=mj-mk;
      if (mk < MZ) mk += MBIG;
      mj=ma[ii];
    }
    for (k=1;k<=4;k++)
      for (i=1;i<=55;i++) {
	ma[i] -= ma[1+(i+30) % 55];
	if (ma[i] < MZ) ma[i] += MBIG;
      }
    inext=0;
    inextp=31;
    *idum=1;
  }
  if (++inext == 56) inext=1;
  if (++inextp == 56) inextp=1;
  mj=ma[inext]-ma[inextp];
  if (mj < MZ) mj += MBIG;
  ma[inext]=mj;
/*	return mj*FAC; */
  ZZ = mj*FAC;
  ZZ = (ZZ < (float)0.0 ? -ZZ : ZZ );
  ZZ = (ZZ > (float)1.0 ? ZZ-(ajint)ZZ : ZZ);
  return(ZZ);
	
}

#undef MBIG
#undef MSEED
#undef MZ
#undef FAC

/* functions for reading in lines (C) Richard Mott, ICRF */

/* @func  read_line ***********************************************************
**
** read in a line.
**
** @param [r] file [FILE*] Input file
** @param [rw] string [char*] String buffer
**
** @return [ajint] number of chars read.
** @@
******************************************************************************/

ajint read_line(FILE *file,char *string) {

  ajint	c;
  ajint	i=0;
	
  while((c=getc(file)))  {
    if (!i && c==EOF)  return EOF;
    if (i && c==EOF ) return i;
    if (c=='\n') return i;
    string[i] = ajSysItoC(c);
    string[++i] = '\0';
  }
  return 1;
}


/* @func  next_line ***********************************************************
**
** go to next line of input file
**
** @param [r] file [FILE*] Input file
**
** @return [ajint] 1 if successful else 0
** @@
******************************************************************************/

ajint next_line(FILE *file) {

  ajint	c;
  while((c=getc(file))) {
    if (feof(file)) return 0;
    if (c=='\n') return 1;
  }
  return 1;
}

/* @func not_blank ************************************************************
**
** checks whether string is full of white space
**
** @param [r] string [char*] String
**
** @return [ajint] 1 if not all white space else 0.
** @@
******************************************************************************/

ajint not_blank( char *string ) {

  while ( *string != 0 ) {
    if ( ! isspace((ajint)*string) ) return 1;
    string++;
  }
    
  return 0;
}

/* @func  skip_comments *******************************************************
**
** reads in successive lines, truncating comments and skipping blank lines
**
** @param [r] fp [FILE*] Input file
** @param [rw] string [char*] String buffer
**
** @return [ajint] number of chars read.
** @@
******************************************************************************/

ajint skip_comments( FILE *fp, char *string ) {

  ajint n;

  *string = 0;
  while ( ( n = read_line( fp, string ) ) != EOF ) {
    (void) uncomment( string );
    if ( not_blank( string ) ) return n;
  }

  return n;
}

/* @func uncomment ************************************************************
**
** truncates string at the first '!' comment character
**
** @param [rw] string [char*] String
**
** @return [ajint] 1 for success
** @@
******************************************************************************/

ajint uncomment( char *string ) {

    while ( *string != '!'
          /*&& *string != '#'*/ &&
	    *string != 0 )
      string++;

    *string = 0;
    return 1;
}


/* some functions for opening a file in a file search path */
/* return value is the FILE pointer to the opened file, or NULL
   basename is the name of the file without directory
   mode is the file open mode cf fopen
   searchpath is the colon-separated search path
   fullname is the name of the file successfully opened, or NULL 
   env is an environment variable containing the search path
*/

/* @func openfile_in_searchpath **********************************************
**
** open a file on the search path.
**
** @param [r] basename [char*] the name of the file without directory
** @param [r] mode [char*] file open mode (see ANSI C fopen)
** @param [r] searchpath [char*] colon-separated search path
** @param [r] fullname [char*] name of the file successfully opened, or NULL 
**
** @return [FILE*] FILE pointer to the opened file, or NULL
** @@
******************************************************************************/

FILE* openfile_in_searchpath(char *basename, char *mode, char *searchpath,
			     char *fullname ) {

  FILE *fp=NULL;

  if ( searchpath ) /* if search path defined... */
    {
      char *s = ajSysStrdup(searchpath);
      char *p = ajSysStrtok(s,":");
      while ( p )
	{
	  (void) sprintf(fullname,"%s/%s", p, basename );
	  if ( (fp = fopen(fullname,mode)) )
	    break;
	  p = ajSysStrtok(NULL,":");
	}
      AJFREE (s);
    }

  if ( ! fp ) /* try to open in cwd */
    {
      (void) strcpy( fullname, basename );
      fp = fopen(basename,mode);
    }

  if ( ! fp )
    *fullname = '\0';

  return fp;
}

/* open a file in a search path specified by the environment variable env */

/* @func  openfile_in_envpath **********************************************
**
** open a file in a search path specified by the environment variable env.
**
** @param [r] basename [char*] the name of the file without directory
** @param [r] mode [char*] file open mode (see ANSI C fopen)
** @param [r] env [char*] colon-separated search path
** @param [r] fullname [char*] name of the file successfully opened, or NULL 
**
** @return [FILE*] FILE pointer to the opened file, or NULL
** @@
******************************************************************************/

FILE* openfile_in_envpath(char *basename, char *mode, char *env,
			  char *fullname ) {

  return openfile_in_searchpath( basename, mode, getenv(env), fullname );
}


/* GETSEED returns a seed for the random number generator. This is by
default determined by the system clock, or optionally by the
command-line option -s=65767 etc */
/* (C) Richard Mott, ICRF */


extern ajint comment;

/* @func  getseed  ***********************************************************
**
** Returns a seed for the random number generator.  This is by default
** determined by the system clock, or optionally by the command-line
** option -s=65767 etc
**
** @param [rw] seed [int*] Seed
** @param [r] argc [ajint] Number of command line arguments
** @param [r] argv [char**] Array of command line arguments
**
** @return [ajint] seed.
** @@
******************************************************************************/

ajint getseed( ajint *seed, ajint argc, char **argv ) {

    time_t *tloc;
    ajint s;

    tloc = NULL;

    s = ajAcdGetInt("seed");
    if (!s)
      s = *seed = ((ajint)time(tloc))% 100000; 

    *seed = s;

    (void) ajDebug("! seed = %d\n", *seed);

    return *seed;
}

/* @func Getdate ****************************************************
**
** returns a character string containing the current date and time.
**
** @return [char*] current data and time string.
** @@
******************************************************************************/

char* Getdate (void) {

  time_t *tloc;
  char *t;
  static char temp[256];

  AJNEW (tloc);
  (void) time(tloc);
  (void) sprintf( temp, "%s", ctime(tloc) );

  t = temp;
  while ( *t ) {
    if ( *t == '\n')
      *t = 0;
    t++;
  }

  return temp;
}


extern ajint verbose;

typedef struct
{
  float key;
  ajint value;
}
key_value;

ajint kv_cmp( const void *a, const void *b );


/* @func  openfile_in_seqpath *************************************************
**
** open a file in the sequence path
**
** @param [r] basename [char*] the name of the file without directory
**                              or extension
** @param [r] ext [char*] file extension
** @param [r] mode [char*] file open mode (see ANSI C fopen)
** @param [r] fullname [char*] name of the file successfully opened, or NULL 
**
** @return [FILE*] FILE pointer to the opened file, or NULL
** @@
******************************************************************************/

FILE* openfile_in_seqpath( char *basename, char *ext, char *mode,
			   char *fullname ) {

  char buf[256];

  (void) strcpy(buf,basename);
  (void) extension(buf,ext);

  return openfile_in_envpath( buf, mode, SEQPATH, fullname );
}
/*
char *seq_data_home(name)
char *name;
{
  char buf[256], *s;

  (void) strcpy(buf,name);
  s = buf;
  while(*s)
    {
      *s = toupper(*s);
      s++;
    }

  if ( ( s = (char*)getenv(buf) ) || (s = (char*)getenv("SEQ_DATA_HOME") ) )
    return s;
  else
    return (char*)ajSysStrdup("./");
}
*/

/* @func  open_database *******************************************************
**
** open a database.
**
** @param [r] name [char*] Database name
**
** @return [DATABASE*] Database pointer
** @@
** MATHOG, minor change in logic so that it could end with a return
******************************************************************************/

DATABASE* open_database( char *name ) {

  DATABASE *db;

           db = which_database(      name ); /* NULL or a pointer */
  if (!db) db = open_embl_database(  name );
  if (!db) db = open_nbrf_database(  name );
  if (!db) db = open_fasta_database( name );
  if (!db){
      ajFatal("could not open database %s", name );
  }
  return db;
}

/* @func  open_embl_database ************************************************
**
** open an embl database
**
** @param [r] name [char*] Database name
**
** @return [DATABASE*] pointer to database.
** @@
******************************************************************************/

DATABASE* open_embl_database( char *name ) {

  char buf[256];
  DATABASE *db;

  AJNEW0 (db);
  db->database = (char*)ajSysStrdup(name);
  db->datafile = openfile_in_seqpath(name, "dat", "r",buf);
  db->indexfile = openfile_in_seqpath(name, "index","r",buf);
  db->type = EMBL;

  if ( ! db->datafile || ! db->indexfile )
    {
      AJFREE ( db );
      return NULL;
    }

  make_embl_index( db );
  
  (void) ajDebug ("\n! embl format database %s opened: %d sequences\n",
		  name, db->sequences );

  (void) add_database(db);

  return db;
}

/* @func  acc_nos ************************************************************
**
** get the accession number
**
** @param [r] seq [SEQUENCE*] Sequece
**
** @return [char*] accession number.
** @@
******************************************************************************/

char* acc_nos( SEQUENCE *seq ) {

  if ( seq->database->type == EMBL )return embl_seq_comment( seq, "AC" );
  return nbrf_seq_comment( seq, "ACCESSION" );
}

/* @func seq_comment **********************************************************
**
** Get comment for as sequence.
**
** @param [r] seq [SEQUENCE*] Sequence
** @param [r] key [char*] Search query
**
** @return [char*] comment.
** @@
******************************************************************************/

char* seq_comment( SEQUENCE *seq, char *key ) {

  if ( seq->database->type == EMBL )return embl_seq_comment( seq, key );
  return nbrf_seq_comment( seq, key );
}

/* @func  embl_seq_comment ****************************************************
**
** get embl sequence comment
**
** @param [r] seq [SEQUENCE*] Sequence
** @param [r] key [char*] Search query
**
** @return [char*] comment.
** @@
******************************************************************************/

char* embl_seq_comment( SEQUENCE *seq, char *key ) {

  char line[256];
  ajulong text_offset;
  ajulong offset = get_offset( seq->name, seq->database, &text_offset );
  ajulong end_pos;
  static char buf[MAX_BUF];

  *buf = 0;
  (void) fseek(seq->database->datafile,offset,0);
  end_pos = find_next( seq->database->datafile, "//", line );
  (void) fseek(seq->database->datafile,offset,0);

  while (1)
    {
      (void) find_next( seq->database->datafile, key, line );
      if ( end_pos < ftell(seq->database->datafile ) )
	break;
      else
	{
	  if ( strlen(buf) + strlen(line) + 2 < MAX_BUF )
	    (void) strcat( buf, clean_line(line)+strlen(key) );
	  else
	    break;
	}
    }

  return buf;
}

/* @func nbrf_seq_comment *****************************************************
**
** Get nbrf comment for a sequence.
**
** @param [r] seq [SEQUENCE*] Sequence
** @param [r] key [char*] Search query
**
** @return [char*] comment.
** @@
******************************************************************************/

char* nbrf_seq_comment( SEQUENCE *seq, char *key) {

  char line[256];
  ajulong text_offset=0;
  ajulong end_pos;
  static char buf[MAX_BUF];

  *buf = 0;
  if ( seq->database->textfile ) {
    (void) fseek(seq->database->textfile,text_offset+4,0);
    end_pos = find_next( seq->database->textfile, ">>>>", line );
    (void) fseek(seq->database->textfile,text_offset,0);

    while (1) {
      (void) find_next( seq->database->textfile, key, line );
      if ( end_pos < ftell(seq->database->textfile ) )
	break;
      else {
	if ( strlen(buf) + strlen(line) + 2 < MAX_BUF )
	  (void) strcat( buf, clean_line(line)+strlen(key) );
	else
	  break;
      }
    }
  }

  return buf;
}


/* @func  open_fasta_database *************************************************
**
** Open a fasta database.
**
** @param [r] name [char*] Database name
**
** @return [DATABASE*] Database data structure
** @@
******************************************************************************/

DATABASE* open_fasta_database( char *name ) {

  char buf[256];
  DATABASE *db;

  AJNEW0 (db);
  db->database = (char*)ajSysStrdup(name);
  db->datafile = openfile_in_seqpath(name, "fasta", "r",buf);
  db->indexfile = openfile_in_seqpath(name, "index","r",buf);
  db->type = FASTA;

  if ( ! db->datafile || ! db->indexfile )
    {
      AJFREE ( db );
      return NULL;
    }

  make_fasta_index( db );
  
  (void) ajDebug("\n! fasta format database %s opened: %d sequences\n",
		 name, db->sequences );

  (void) add_database(db);

  return db;
}

/* @func open_nbrf_database ***************************************************
**
** Open a nbrf database.
**
** @param [r] name [char*] Database name
**
** @return [DATABASE*] Database data structure
** @@
******************************************************************************/

DATABASE* open_nbrf_database( char *name ) {

  char buf[256];
  DATABASE *db;

  AJNEW0 (db);
  db->database = (char*)ajSysStrdup(name);
  db->datafile = openfile_in_seqpath(name, "seq", "r",buf);
  db->textfile = openfile_in_seqpath(name, "ref", "r",buf);
  db->indexfile = openfile_in_seqpath(name, "index", "r",buf);
  db->type = NBRF;

  if ( ! db->datafile || ! db->textfile || ! db->indexfile )
    {
      AJFREE ( db );
      return NULL;
    }

  make_nbrf_index( db );
  
  (void) ajDebug ("\n! nbrf format database %s opened: %d sequences\n",
		  name, db->sequences );

  (void) add_database(db);

  return db;
}


/* @func which_database *******************************************************
**
** Get database pointer.
**
** @param [r] database_name [char*] Database name
**
** @return [DATABASE*] Database data structure
** @@
******************************************************************************/

DATABASE* which_database( char *database_name ) {

  ajint n;

  for(n=0;n<database_count;n++)
    if ( ! strcmp( database_name, databases[n]->database ) )
      return databases[n];
  return NULL;
}

/* @func  add_database ********************************************************
**
** add database to list.
**
** @param [r] db [DATABASE*] Database
**
** @return [ajint] 1 if successful.
** @@
******************************************************************************/

ajint add_database ( DATABASE *db ) {

  if ( database_count < 100 )
    databases[database_count++] = db;
  else
    {
      (void) ajFatal("\nERROR: too many databases!");
    }
  return 1;
}

/* @func make_embl_index ******************************************************
**
** make an embl index
**
** @param [rw] db [DATABASE*] Database pointer
**
** @return [void]
** @@
******************************************************************************/

void make_embl_index( DATABASE *db ) {

  ajint n;
  unsigned long offset;
  HASH_LIST *item;
  char name[256];

  AJCNEW (db->index, PRIME_NUM);

  while( fscanf( db->indexfile, "%s %lu\n", name, &offset ) == 2 )
    {
      n = hash_name(name);
      AJNEW0 (item);
      item->name = (char*)ajSysStrdup(name);
      item->offset = offset;
      db->sequences++;

      if ( ! db->index[n] )
	db->index[n] = item;
      else
	{
	  item->next = db->index[n];
	  db->index[n] = item;
	}
    }

  rewind(db->indexfile);
}

/* @func  make_fasta_index ****************************************************
**
** make a fasta index
**
** @param [rw] db [DATABASE*] Database pointer
**
** @return [void]
** @@
******************************************************************************/

void make_fasta_index( DATABASE *db ) {

  ajint n;
  unsigned long offset;
  HASH_LIST *item;
  char name[256];

  AJCNEW (db->index, PRIME_NUM);

  while( fscanf( db->indexfile, "%s %lu\n", name, &offset ) == 2 )
    {
      n = hash_name(name);
      AJNEW0 (item);
      item->name = (char*)ajSysStrdup(name);
      item->offset = offset;
      db->sequences++;

      if ( ! db->index[n] )
	db->index[n] = item;
      else
	{
	  item->next = db->index[n];
	  db->index[n] = item;
	}
    }

  rewind(db->indexfile);
}

/* @func  make_nbrf_index ****************************************************
**
** make a nbrf index
**
** @param  [rw] db [DATABASE*] Database pointer
**
** @return [void]
** @@
******************************************************************************/

void make_nbrf_index( DATABASE *db ) {

  ajint n;
  unsigned long offset1, offset2;
  HASH_LIST *item;
  char name[256];

  AJCNEW (db->index, PRIME_NUM);

  while(fscanf( db->indexfile, "%s %lu %lu\n",
		name, &offset1, &offset2) == 3 ) {
    n = hash_name(name);
    AJNEW0 (item);
    item->name = (char*)ajSysStrdup(name);
    item->offset = offset1;
    item->text_offset = offset2;
    db->sequences++;

    if ( ! db->index[n] )
      db->index[n] = item;
    else {
      item->next = db->index[n];
      db->index[n] = item;
    }
  }

  rewind(db->indexfile);
}


/* @func  read_seq ************************************************************
**
** Read a sequence.
**
** @param [r] name [char*] Sequence name
**
** @return [SEQUENCE*] Sequence data structure
** @@
******************************************************************************/

SEQUENCE* read_seq( char *name ) {

  char *db, *sname;
  DATABASE *d;

  sname = (char*)strchr( name, ':' )+1;
  *(sname-1) = 0;
  db = name;

  d = open_database( db );
  return read_sequence( sname, d );

}
  
/* @func  read_sequence *******************************************************
**
** Read in a sequence from the database.
**
** @param [r] name [char*] Sequence name
** @param [r] database [DATABASE*] Database pointer
**
** @return [SEQUENCE*] Sequence data structure
** @@
******************************************************************************/

SEQUENCE* read_sequence( char *name, DATABASE *database ) {

  ajulong text_offset;
  ajulong offset = get_offset( name, database, &text_offset );

  if ( database->type == EMBL  )return read_embl_sq( name, database, offset );
  if ( database->type == NBRF  )return read_nbrf_sq( name, database, offset, text_offset );
  if ( database->type == FASTA )return read_fasta_sq( name, database, offset);
  return NULL;
}

/* @func  get_offset **********************************************************
**
** get the offset.
**
** @param [r] name [char*] Sequence name
** @param [r] database [DATABASE*] Database pointer
** @param [r] text_offset [ajulong*] Offset
**
** @return [ajulong] offset
** @@
******************************************************************************/

ajulong get_offset( char *name, DATABASE *database,
		   ajulong *text_offset ) {

  ajint n;
  HASH_LIST *h;

  n = hash_name(name);
  h = database->index[n];

  *text_offset = 0;

  while( h )
    {
      if ( ! strcmp( name, h->name ) )
	{
	  *text_offset = h->text_offset;
	  return h->offset;
	}
      else
	h = h->next;
    }
  return 0;
}
  
/* @func  read_embl_sq ********************************************************
**
** Read an embl sequence.
**
** @param [r] name [char*] Sequence name
** @param [r] database [DATABASE*] Database pointer
** @param [r] offset [ajulong] Offset
**
** @return [SEQUENCE*] sequence
** @@
******************************************************************************/

SEQUENCE* read_embl_sq( char *name, DATABASE *database,
			ajulong offset ) {

  char sname[256], line[512];
  ajint c;
  SEQUENCE *seq;
  ajint len, n;

  AJNEW0 (seq);
  (void) fseek(database->datafile, offset, 0 );

  (void) find_next( database->datafile, "ID", line );

  *sname = 0;
  (void) sscanf( line, "ID  %s", sname );

  if ( strcmp( sname, name ) ) {
    (void) ajFatal(
	    "\n! ERROR on read_embl_sq: pointer offset for %s wrong! "
	    "(gets %s instead)", name, sname );
  }

  seq->name = (char*)ajSysStrdup(name);
  seq->database = database;

  (void) find_next( database->datafile, "DE", line );

  seq->desc = (char*)ajSysStrdup(&line[5]);

  (void) find_next( database->datafile, "SQ", line );

  (void) sscanf( line, "SQ   Sequence %d", &len );

  seq->len = len;
  AJCNEW (seq->s, len+1);
  n = 0;

  while ( (c = getc(database->datafile)) != '/' && c != EOF ) {
    if ( isalpha(c) && ! isspace(c) && n < len )
      seq->s[n++] = ajSysItoC(tolower(c));
  }
  return seq;
}

/* @func  get_next_sq *********************************************************
**
** get the next sequence.
**
** @param [r] all [AjPSeqall] AJAX sequence stream
**
** @return [SEQUENCE*] Sequence
** @@
******************************************************************************/

SEQUENCE* get_next_sq( AjPSeqall all ) {

  static AjPSeq seq = NULL;
  if (!ajSeqallNext(all, &seq))
    return NULL;

  return (seq_to_sequence(seq));
}

/* @func  seq_to_sequence **************************************************
**
** Convert form AjPSeq format to SEQUENCE format.
**
** @param [r] ajseq [AjPSeq] AJAX sequence
**
** @return [SEQUENCE*] Sequence
** @@
******************************************************************************/

SEQUENCE* seq_to_sequence( AjPSeq ajseq ) {

  SEQUENCE *seq=NULL;
  if (!ajseq)
    return NULL;

  AJNEW0 (seq);
  seq->len = ajSeqLen(ajseq);
  seq->desc = ajStrStr(ajSeqGetDesc(ajseq));
  seq->name = ajStrStr(ajSeqGetName(ajseq));
  seq->s = ajSeqChar(ajseq);

  return seq;
}

/* @func  get_fasta_sq *******************************************************
**
** read a fasta sequence in from a file. Can be used for multiple reads
**
** @param [r] fp [FILE*] Input file
**
** @return [SEQUENCE*] Sequence
** @@
******************************************************************************/

SEQUENCE* get_fasta_sq( FILE *fp ) {

  ajint c;
  SEQUENCE *seq=NULL;
  char name[256], desc[256];
  ajint i;
  static char *buffer=NULL;
  static ajint buflen=0;

  if ( buffer == NULL ) {
    buflen = 100000;
    AJCNEW (buffer, buflen);
  }

  while( (c=getc(fp)) != '>' && c != EOF ) /* start of FASTA sequence */
    ;

  if ( c == EOF )
    return NULL;

  AJNEW0 (seq);

  i = 0;
  while( ! isspace(c=getc(fp)) ) { /* sequence name */
    if ( i < 254 )
      name[i++] = ajSysItoC(c);
  }

  name[i] = 0;
  seq->name = ajSysStrdup(name);

  while( c != '\n' && c != EOF && isspace(c=getc(fp)) )
    ;

  i = 0;
  while( c != '\n' && c != EOF ) {
    if ( i < 254 ) 
      desc[i++] = ajSysItoC(c);
  }
  c = getc(fp);

  desc[i] = 0;
  seq->desc = ajSysStrdup(desc);

  seq->len=0;
  while( (c=getc(fp)) != '>' && c != EOF ) {
    if ( isalpha(c)||(c=='-') ) {
      if ( seq->len+1 >= buflen ) {
	buflen += 100000;
	AJRESIZE (buffer, buflen);
      }
      buffer[seq->len++] = ajSysItoC(c);
    }
  }

  buffer[seq->len] = 0;

  seq->s = (char*)ajSysStrdup(buffer);
  
  buffer[0] = 0;

  (void) ungetc(c,fp);

  return seq;
}

/* @func read_fasta_sq ********************************************************
**
** Read a fasta sequence
**
** @param [r] name [char*] Name
** @param [r] database [DATABASE*] Database pointer
** @param [r] offset [ajulong] Offset
**
** @return [SEQUENCE*] sequence
** @@
******************************************************************************/

SEQUENCE* read_fasta_sq( char *name, DATABASE *database,
			 ajulong offset ) {

  char sname[256], line[512];
  SEQUENCE *seq;
  ajint len, n, c;
  ajulong current;

  AJNEW0 (seq);
  (void) fseek(database->datafile, offset, 0 );

  (void) read_line(database->datafile,line);
  *sname = 0;
  (void) sscanf( line, ">%s", sname );

  if ( strcmp( sname, name ) ) {
    (void) ajFatal("\n! ERROR on read_fasta_sq: pointer offset for %s wrong!"
	    " (gets %s instead)", name, sname );
  }

  seq->name = (char*)ajSysStrdup(name);
  seq->database = database;

  seq->desc = (char*)ajSysStrdup("       ");

  current = ftell(database->datafile);

  len=0;
  while((c=getc(database->datafile)) != '>' && c != EOF )
    len += ( ! isspace(c) && c != '\n' );
  seq->len = len;
  AJCNEW (seq->s, len+1);
  n = 0;

  (void) fseek(database->datafile, current, 0 );

  while( (c=getc(database->datafile)) != '>' && c != EOF )
    if ( ! isspace(c) && c != '\n'  && n < len )
      seq->s[n++] = ajSysItoC(tolower(c));

  seq->len = n;

  return seq;
}
	
/* @func  read_nbrf_sq ********************************************************
**
** Read a nbrf sequence.
**
** @param [r] name [char*] Name
** @param [r] database [DATABASE*] Database pointer
** @param [r] offset [ajulong] Sequence file offset
** @param [r] text_offset [ajulong] Text offset
**
** @return [SEQUENCE*] sequence
** @@
******************************************************************************/

SEQUENCE* read_nbrf_sq( char *name, DATABASE *database,
		       ajulong offset, ajulong text_offset ) {

  char sname[256], line[512];
  SEQUENCE *seq;
  ajulong current;
  ajint len, c;

  AJNEW0 (seq);
  (void) fseek(database->datafile, offset, 0 );

  *sname = 0;
  (void) read_line(database->datafile,line);
  (void) sscanf( line, ">>>>%s", sname );

  if ( strcmp( sname, name ) ) {
    (void) ajFatal(
	    "\n! ERROR on read_nbrf_sq: pointer offset for %s wrong! "
	    "(gets %s instead)", name, sname );
  }

  seq->name = (char*)ajSysStrdup(name);
  seq->database = database;

  (void) read_line(database->datafile,line);
  seq->desc = (char*)ajSysStrdup(line);

  current = ftell(database->datafile);
  len = 0;

  while ( (c = getc(database->datafile)) != '>' && c != EOF ) {
    if ( isalpha(c) && ! isspace(c)  )
      len++;
  }

  seq->len = len;
  AJCNEW (seq->s, len+1);
  (void) fseek( database->datafile, current, 0 );
  len = 0;

  while ( (c = getc(database->datafile)) != '>' && c != EOF ) {
    if ( isalpha(c) && ! isspace(c)  )
      seq->s[len++] = ajSysItoC(tolower(c));
  }

  return seq;
}
	
/* @func  find_next ***********************************************************
**
** find next line with text in.
**
** @param [r] fp [FILE*] Input file
** @param [r] text [char*] Test to search for
** @param [r] line [char*] Input buffer
**
** @return [ajulong] Offset in file
** @@
******************************************************************************/

ajulong find_next( FILE *fp, char *text, char *line ) {

  while( read_line(fp,line) != EOF ) {
    (void) clean_line(line);
    if ( !strncmp( line, text, strlen(text) ) )
      return ftell(fp)-strlen(line);
  }
  return 0;
}


/* @func  hash_name *********************************************************
**
** computes a integer hash value between 0 and PRIME_NUM-1 for the
** character string (case-insensitive)
**
** @param [r] string [char*] String
**
** @return [ajint] hash value
** @@
******************************************************************************/

ajint hash_name( char *string ) {

  ajint n=0;

  while( *string )
    {
      n = (64*n + tolower((ajint) *string) ) % PRIME_NUM;
      string++;
    }

  return n;
}

/* @func  compile_embl_index **************************************************
**
** compile the embl index
**
** @param [r] name [char*] database name
**
** @return [ajint] 1 for success
** @@
******************************************************************************/

ajint compile_embl_index( char *name ) {

  char buf[256];
  FILE *datafile, *indexfile;
  char sname[256];
  ajint c;
  unsigned long n, offset, state=-1;

  datafile = openfile_in_seqpath( name, "dat", "r", buf);
  indexfile = openfile_in_seqpath( name, "index", "w", buf);

  (void) ajDebug ("compiling embl format index for database %s ...\n", name );

  state = -1;
  offset = 0;
  n = 0;
  while ( (c = fgetc( datafile ) ) != EOF )
    {
      if ( c == '\n' )
	{
	  state = 1;
	}
      else if ( (state == 1 || state == -1 ) && c == 'I' )
	{
	  state = 2;
	}
      else if ( state == 2 && c == 'D' )
	{
	  offset = ftell(datafile)-2;
/*	  while( isspace(c=fgetc(datafile)) );*/
	  (void) fscanf( datafile, "%s", sname ); 
	  n++;
	  (void) fprintf(indexfile, "%-10s %lu\n", sname, offset );
	  (void) ajDebug ( "%-10s %lu\n", sname, offset );
	  state = 0;
	}
      else
	state = 0;
    }
  (void) fclose(indexfile);
  (void) fclose(datafile);

  (void) ajDebug ("... done. %ld sequences indexed\n", n );
  return 1;
}

/* @func compile_fasta_index **************************************************
**
** compile the fasta index
**
** @param [r] name [char*] Database name
**
** @return [void] 
** @@
******************************************************************************/

void compile_fasta_index( char *name ) {

  char buf[256];
  FILE *datafile, *indexfile;
  char sname[256];
  ajint c;
  unsigned long n, offset, state=-1;

  datafile = openfile_in_seqpath( name, "fasta", "r", buf);
  indexfile = openfile_in_seqpath( name, "index", "w", buf);

  (void) ajDebug ("compiling fasta format index for database %s ...\n",
		  name );

  state = -1;
  offset = 0;
  n = 0;
  while ( (c = fgetc( datafile ) ) != EOF ) {
    if ( c == '\n' ) {
      state = 1;
    }
    else if ( (state == 1 || state == -1 ) && c == '>' ) {
      state = 2;
    }
    else
      state = 0;

    if ( state == 2 ) {
      offset = ftell(datafile)-1;
/*	  while( isspace(c=fgetc(datafile)) );*/
      (void) fscanf( datafile, "%s", sname ); 
      n++;
      (void) fprintf(indexfile, "%-10s %lu\n", sname, offset );
      (void) ajDebug ( "%-10s %lu\n", sname, offset );
      state = 0;
    }
  }
  (void) fclose(indexfile);
  (void) fclose(datafile);

  (void) ajDebug ("... done. %ld sequences indexed\n", n );
}

/* @func  compile_nbrf_index **************************************************
**
** compile the nbrf index
**
** @param [r] name [char*] Database name
**
** @return [void]
** @@
******************************************************************************/

void compile_nbrf_index( char *name ) {

  char buf[256];
  FILE *datafile, *textfile, *indexfile;
  char name1[256], name2[256];
  unsigned long c, d;
  unsigned long n;

  datafile = openfile_in_seqpath( name, "seq", "r",buf);
  textfile = openfile_in_seqpath( name, "ref", "r",buf);
  indexfile = openfile_in_seqpath( name, "index", "w",buf);

  (void) ajDebug ("compiling nbrf format index for database %s ...\n", name );

  n = 0;
  while (1)
    {
      c = seekto( datafile, ">>>>");
      d = seekto( textfile, ">>>>");

      if ( c == EOF || d == EOF )
	break;

      (void) fscanf( datafile, ">>>>%s", name1 );
      (void) fscanf( textfile, ">>>>%s", name2 );

      if ( strcmp( name1, name2 ) )
	{
	  (void) ajFatal("\n! ERROR when indexing: different names %s %s",
		   name1, name2 );
	}

      (void) fprintf( indexfile, "%-10s %10lu %10lu\n", name1, c, d );
      n++;
    }
  (void) fclose(indexfile);
  (void) fclose(datafile);
  (void) fclose(textfile);

  (void) ajDebug ("... done. %ld sequences indexed\n", n );
}

/* @func  seekto ************************************************************
**
** seek text in the a file.
**
** @param [r] fp [FILE*] Input file
** @param [r] text [char*] String to find
**
** @return [ajulong] Offset at start of line
** @@
******************************************************************************/

ajulong seekto( FILE *fp, char *text ) {

  ajulong p;
  ajint c;
  char *s;

  while(1) {
    c = getc(fp);
    if ( c == EOF )
      return EOF;
    else if ( c == *text ) {
      p = ftell( fp );
      s = text;
      while( *s ) {
	if ( c != *s )
	  break;
	else if ( c == EOF )
	  return EOF;
	else {
	  s++;
	  c = getc(fp);
	}
      }
      if (! *s ) {
	p--;
	(void) fseek(fp,p,0);
/*	return p;*/
	break;
      }
      else 
	(void) fseek(fp,p+1,0);
    }
  }
  return p;
}
	    
/* @func  free_seq ************************************************************
**
** Free memory for a seq.
**
** @param [rw] seq [SEQUENCE*] Sequence
**
** @return [void]
** @@
******************************************************************************/

void free_seq( SEQUENCE *seq ) {

  if ( seq )
    {
      if ( seq->name && *(seq->name)) AJFREE ( seq->name );
      if ( seq->s && *(seq->s) ) AJFREE ( seq->s );
      if ( seq->desc && *(seq->desc) ) AJFREE ( seq->desc );
      AJFREE ( seq );
    }
}

/* @func  free_seq_copy *****************************************************
**
** free the seq copy but don't free its char* pointers .
**
** @param [rw] seq [SEQUENCE*] Sequence
**
** @return [void]
** @@
******************************************************************************/

void free_seq_copy( SEQUENCE *seq ) {

  if ( seq ) {
    AJFREE ( seq );
  }
}

/* @func  which_file_of_sequences ********************************************
**
** Finds the file pointer for a file.
**
** @param [r] filename [char*] File name
**
** @return [FILE*] Open file pointer
** @@
******************************************************************************/

FILE* which_file_of_sequences( char *filename ) {

  static char *filenames[100];
  static FILE *fp[100];
  static ajint files;
  ajint n;

  if ( *filename != '@' )
    return NULL;
  else
    filename++;

  for(n=0;n<files;n++) {
    if ( ! strcmp( filenames[n],filename) ) {
      if ( fp[n] )
	return fp[n];
      else return fp[n]=openfile(filename,"r");
    }
  }

  if ( files == 100 ) {
    (void) ajFatal("too many files of sequences!");
  }

  filenames[files] = (char*)ajSysStrdup(filename);
  fp[files] = openfile(filename,"r");
  return fp[files++];
}

/* @func  next_seq_from_list_file ********************************************
**
** get the next sequence form the list file
**
** @param [r] list_file [FILE*] Input file
**
** @return [SEQUENCE*] sequence.
** @@
******************************************************************************/

SEQUENCE* next_seq_from_list_file( FILE *list_file ) {

  char line[256], seq[256];
  SEQUENCE *sq = NULL;

  *line = 0;

  while ( ! sq &&  skip_comments( list_file, line ) != EOF ) {
    (void) sscanf( line, "%s", seq );
    sq = read_seq( seq );
  }

  return sq;  
}

/* @func  next_seq_from_database_spec *****************************************
**
** get the next seq from database
**
** @param [r] spec [char*] Sequence name
**
** @return [SEQUENCE*] sequence 
** @@
******************************************************************************/

SEQUENCE* next_seq_from_database_spec( char *spec ) {

  char wild[256], line[256], name[256];
  DATABASE *db = is_sequence_spec( spec, wild );
  ajint len = strlen(wild);

  while( read_line( db->indexfile, line ) != EOF ) {
    (void) sscanf( line, "%s", name );
    if ( !strncmp( name, wild, len ) )
      return read_sequence( name, db );
  }

  return NULL;
}

/* @func is_sequence_spec ****************************************************
**
** Check for a valid sequence spec dbname:entryname
**
** @param [r] spec [char*] Sequence spec
** @param [r] wild [char*] Sequence stub up to wildcard or NULL
**
** @return [DATABASE*] Database pointer
** @@
******************************************************************************/

DATABASE* is_sequence_spec( char *spec, char *wild ) {

  static char *old_spec=NULL;
  static char *old_wild;
  static DATABASE *old_db;
  char buf[256], *sname, *db, *s;

  if ( ! old_spec || strcmp( spec, old_spec ) ) {
    old_spec = (char*)ajSysStrdup(spec);
    (void) strcpy(buf,spec);
    sname = (char*)strchr( buf, ':' );
    if ( ! sname ) {
      sname = (char*)ajSysStrdup("");
    }
    else {
      sname++;
      *(sname-1) = 0;
    }

    if ( (s = (char*)strchr( sname, '*')) )
      *s = 0;
    db = buf;
    old_wild = (char*)ajSysStrdup(sname);
    old_db = open_database(db);
  }

  (void) strcpy(wild,old_wild);
  return old_db;
}

/* @func rewind_spec *********************************************************
**
** rewind file for sequence specifier
**
** @param [r] spec [char*] Sequence specifier
**
** @return [void]
** @@
******************************************************************************/

void rewind_spec( char *spec ) {

  char buf[256];
  DATABASE *db = is_sequence_spec( spec, buf );
  FILE *fp;

  if ( db )
    {
      rewind(db->indexfile);
      rewind(db->datafile);
    }
  else if ( (fp = which_file_of_sequences( spec )) )
    rewind(fp);
}

  
/* @func next_seq ************************************************************
**
** get next seq.
**
** @param [r] spec_or_file [char*] Sequence specification or filename
**
** @return [SEQUENCE*] sequence.
** @@
******************************************************************************/

SEQUENCE* next_seq ( char *spec_or_file ) {

  char wild[256];
  DATABASE *db = is_sequence_spec(spec_or_file,wild);
  FILE *fp;

  if ( db ) 
    return next_seq_from_database_spec(spec_or_file);
  else {
    if ( *spec_or_file == '@' &&
	 (fp = which_file_of_sequences( spec_or_file ) ) )
      return next_seq_from_list_file(fp);
  }
  return NULL;
}

/*main(argc, argv)

ajint argc;
char **argv;
{
  DATABASE *db;
  SEQUENCE *seq;
  ajint n=2;
  char spec[256];

  while ( seq = next_seq_from_database_spec( argv[1] ) )
    {
      (void) ajDebug ("seq: %s len: %d\n%s\n%s\n",
                      seq->name, seq->len, seq->desc, seq->s );
      free_seq(seq);
    }
}*/

static char *complement_table;

/* @func complement_base ******************************************************
**
** complement the base.
**
** @param [r] c [char] Standard nucleotide base code
**
** @return [char] the complement.
** @@
******************************************************************************/

char complement_base( char c ) {

  if ( !complement_table) {
    ajint x;
    AJCNEW0 (complement_table, 256);

    complement_table += 127;	/* start in the middle :-) */

    for(x=-127;x<128;x++) {
      if ( x == 'a' )
	complement_table[x] = 't';
      else if ( x == 'c' )
	complement_table[x] = 'g';
      else if ( x == 'g' )
	complement_table[x] = 'c';
      else if ( x == 't' || x == 'u' )
	complement_table[x] = 'a';
      else if ( x == '[' )
	complement_table[x] = ']';
      else if ( x == ']' )
	complement_table[x] = '[';
      else if ( x == '*' )
	complement_table[x] = '*';
      else if ( x == '-' )
	complement_table[x] = '-';
      else
	complement_table[x] = 'n';
    }
  }

  if ( isupper((ajint) c) ) {
    return ajSysItoC(toupper((ajint) complement_table[tolower((ajint) c)]));
  }
  return complement_table[tolower((ajint) c)];
}

/* @func complement_seq ******************************************************
**
** complement the sequence.
**
** @param [r] seq [char*] Sequence as a string
**
** @return [char*] complemented sequence.
** @@
******************************************************************************/

char* complement_seq( char *seq ) {

  char *s = seq;
  char c, *t;

  while (*s) {
    *s = complement_base( *s);
	  
    s++;
  }

  t = seq;
  s--;

  while ( t < s ) {
    c = *s;
    *s = *t;
    *t = c;
    t++;
    s--;
  }

  return seq;
}

/* @func subseq ************************************************************
**
** Creates a SEQUENCE containing the subsequence start to stop inclusive
** If start greater than stop then returns the reverse complement
** If start and stop are out of range they are truncated
**
** @param [r] seq [SEQUENCE*] Original sequence
** @param [r] start [ajint] Start base
** @param [r] stop [ajint] End base
**
** @return [SEQUENCE*] sub sequence
** @@
******************************************************************************/

SEQUENCE* subseq( SEQUENCE *seq, ajint start, ajint stop ) {

  SEQUENCE *subs = seqdup(seq);
  static char *s;
  ajint i;
  ajint reverse;

  if ( start > stop ) {
    reverse = 1;
    i = start;
    start = stop;
    stop = i;
  }
  else
    reverse = 0;

  if ( start < 0 ) start = 0;
  if ( start > seq->len-1 ) return NULL;
  if ( stop > seq->len-1 ) stop = seq->len-1;
    
  subs->len = stop-start+1;
  if (verbose)
    (void) ajDebug("calloc starting for subs->len %d\n", subs->len);
  AJCNEW (s, subs->len+1);
  if (verbose)
    (void) ajDebug("calloc done for subs->len %d\n", subs->len);
  for(i=0;i<subs->len;i++)
    s[i] = seq->s[start+i];

  if ( reverse )
    (void) complement_seq(s);

  AJFREE (subs->s);
  subs->s = s;

  return subs;
}


/* @func downcase ************************************************************
**
** Convert string to lower case.
**
** @param [r] s [char*] String
**
** @return [char*] lower case version.
** @@
******************************************************************************/

char* downcase(char *s) {

  char *t = s;
  while (*s) {
    *s = ajSysItoC(tolower((ajint) *s));
    s++;
  }
  return t;
}

/* @func upcase ************************************************************
**
** Convert string to upper case.
**
** @param [r] s [char*] String
**
** @return  [char*] upper case version.
** @@
******************************************************************************/

char* upcase(char *s) {

  char *t = s;
  while (*s) {
    *s = ajSysItoC(toupper((ajint) *s));
    s++;
  }
  return t;
}

/* @func clean_line ***********************************************************
**
** cleans the line.
**
** @param [r] s [char*] String
**
** @return [char*] cleaned line.
** @@
******************************************************************************/

char* clean_line(char *s) {

  char *t = s;

  while(*s) {
    if ( ! isprint((ajint)*s) )
      *s = ' ';
    s++;
  }
  return t;
}

/* @func  iubtoregexp ********************************************************
**
** convert a sequence string with IUB codes into a regular expression.
**
** @param [r] iubstring [char*] Sequence string
**
** @return [char*] regexp.
** @@
******************************************************************************/

char* iubtoregexp( char *iubstring ) {

  ajint len=strlen(iubstring);
  char *s = iubstring;
  char *t, *r;

  while(*s) {
    if ( (t = iub_regexp(*s++)) )
      len += strlen(t);
  }

  AJCNEW (r, len+3);

  return iub2regexp( iubstring, r, len+1 );
  
}

/* @func iub2regexp ***********************************************************
**
** convert a sequence with IUB codes into a regular expression returns
** either regexp or NULL if the length of the regexp is greater that
** maxlen, which should be set to the max permitted size
**
** @param [r] iubstring [char*] Sequence string
** @param [w] regexp [char*] Regular expression
** @param [r] maxlen [ajint] Maximum regular expression length
**
** @return [char*]  converted string or NULL if it fails.
** @@
******************************************************************************/

char* iub2regexp( char *iubstring, char *regexp, ajint maxlen ) {

  char *s, c;
  ajint len=0;

  maxlen--;
  while(*iubstring && len < maxlen) {
    if ( (s = iub_regexp(c=*iubstring++)) ) {
      (void) strcat( regexp, s );
      len += strlen(s);
    }
    else {
      regexp[len++] = c;
    }
  }

  if ( len <= maxlen ) {
    regexp[len]= 0;
    return regexp;
  }
  return NULL;
}

/* @func iub_regexp  **********************************************************
**
** return a pointer to the regexp corresponding to an iub code, or
** NULL otherwise. Note that only ambiguity codes return a non-null
** result
** 
** @param [r] c [char] UIB sequence code
**
** @return [char*] Regular expression for all matching bases
** @@
******************************************************************************/

char* iub_regexp (char c ) {

  static char *iub[256];
  static ajint initialised;

  if ( ! initialised ) {
    iub['r']= "[ag]";
    iub['y']= "[ctu]";
    iub['m']= "[ac]";
    iub['k']= "[gtu]";
    iub['s']= "[cg]";
    iub['w']= "[atu]";
    iub['h']= "[actu]";
    iub['b']= "[cgtu]";
    iub['v']= "[acg]";
    iub['d']= "[agtu]";
    iub['n']= "[acgtu]";
    initialised = 1;
  }

  return iub[tolower((ajint) c)];
}

#define MAXSEQLEN 100000

/* @func  get_seq_from_file **************************************************
**
** read a sequence from file
**
** @param [r] filename [char*] Filename
**
** @return [SEQUENCE*] sequence
** @@
******************************************************************************/

SEQUENCE* get_seq_from_file (char *filename) {

  FILE *fp;
  SEQUENCE *seq;
  char name[256];
  char desc[256];
  char buf[MAXSEQLEN+1];
  char *s, *t;
  ajint len;

  if ( (fp=openfile( filename, "r" )) ) {
    AJNEW0 (seq);
    (void) read_line( fp, desc );
    (void) sscanf( desc, "%s", name );
    seq->name = (char*)ajSysStrdup(name);
    (void) read_line( fp, desc );
    seq->desc = (char*)ajSysStrdup(desc);
    len=fread(buf, sizeof(char), MAXSEQLEN, fp );
    buf[len] = 0;
    s = buf;
    t = buf;
    len = 0;
    while ( *s ) {
      if ( isalpha((ajint)*s) ) {
	*t++ = ajSysItoC(tolower((ajint) *s));
	len++;
      }
      s++;
    }
    *t = 0;
    seq->len = len;
    seq->s = (char*)ajSysStrdup(buf);
    (void) fclose(fp);
    return seq;
  }
  return NULL;
}

/* @func  seqdup ************************************************************
**
** duplicate a sequence
**
** @param [r] seq [SEQUENCE*] Original sequence
**
** @return [SEQUENCE*] duplicate sequence
** @@
******************************************************************************/

SEQUENCE* seqdup (SEQUENCE *seq) {

  SEQUENCE *dup;
  if ( seq )
    {
      AJNEW0(dup);
      *dup = *seq;
      if ( seq->name )
	dup->name = (char*)ajSysStrdup(seq->name);
      else
	dup->name = NULL;
      if ( seq->desc ) {
	if (strlen(seq->desc) > 256)
	  dup->desc = (char*)ajSysStrdup(seq->desc);
	else {
	  AJCNEW (dup->desc, 512);
	  (void) strcpy (dup->desc, seq->desc);
	}
      }
      else
	dup->desc = NULL;
      if ( seq->s )
	dup->s = (char*)ajSysStrdup(seq->s);
      else
	dup->s = NULL;


      return dup;
    }
  return NULL;
}

    
/* @func  into_sequence *****************************************************
**
** turn name, desc and sequence into a sequence data structure.
**
** @param [r] name [char*] Sequence name
** @param [r] desc [char*] Sequence description
** @param [r] s [char*] Sequence bases
**
** @return [SEQUENCE*] sequence
** @@
******************************************************************************/

SEQUENCE* into_sequence( char *name, char *desc, char *s ) {

  SEQUENCE *seq;

  AJNEW0 (seq);
  seq->name = name;
  seq->desc = desc;
  seq->s = s;
  seq->len = strlen(s);
  return seq;
}

/* @func  kv_cmp ************************************************************
**
** compare key values.
**
** @param [r] a [const void*] First value
** @param [r] b [const void*] Second value
**
** @return [ajint] comparison value.
** @@
******************************************************************************/

ajint kv_cmp ( const void *a, const void *b ) {

  key_value *ka, *kb;
  ka = (key_value *) a;
  kb = (key_value *) b;

  return fcmp( &(ka->key), &(kb->key) );
}


/* @func shuffle_seq **********************************************************
**
** Shuffle the sequence.
**
** @param [r] seq [SEQUENCE*] Original sequence
** @param [r] in_place [ajint] Boolean 1=shuffle in place
** @param [r] seed [int*] Random number seed.
**
** @return [SEQUENCE*] shuffled sequence.
** @@
******************************************************************************/

SEQUENCE* shuffle_seq( SEQUENCE *seq, ajint in_place, ajint *seed ) {

  SEQUENCE *shuffled;
  if ( ! in_place )
    shuffled = seqdup( seq );
  else
    shuffled = seq;

  (void) shuffle_s( shuffled->s,  seed );

  return shuffled;
}
/*
  AJCNEW (tmp, shuffled->len);

  for(n=0;n<shuffled->len;n++)
    {
      tmp[n].key = rand3(seed);
      tmp[n].value = shuffled->s[n];
    }

  qsort(tmp,shuffled->len,sizeof(key_value),kv_cmp);

  for(n=0;n<shuffled->len;n++)
    shuffled->s[n] = tmp[n].value;

  AJFREE (tmp);

  return shuffled;
}
*/

/* @func shuffle_s ************************************************************
**
** in-place shuffle of a string
**
** @param [r] s [char*] String
** @param [r] seed [int*] Seed
**
** @return [char*] shuffled string.
** @@
******************************************************************************/

char* shuffle_s( char *s, ajint *seed ) {

/* in-place shuffle of a string */

  key_value *tmp;
  ajint n;
  ajint len = strlen(s);

  AJCNEW (tmp, len);

  for(n=0;n<len;n++)
    {
      tmp[n].key = rand3(seed);
      tmp[n].value = s[n];
    }

  qsort(tmp,len,sizeof(key_value),kv_cmp);

  for(n=0;n<len;n++)
    s[n] = ajSysItoC(tmp[n].value);

  AJFREE (tmp);

  return s;
}

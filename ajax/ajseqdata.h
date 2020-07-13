#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdata_h
#define ajseqdata_h

#define NULLFPOS -1

enum AjEQryType {QRY_UNKNOWN, QRY_ENTRY, QRY_QUERY, QRY_ALL};

typedef struct SeqSAccess SeqSAccess;

/* @data AjPSeqQuery *******************************************************
**
** Ajax Sequence Query object.
**
** Holds data needed to interpret the entry specification part of a USA.
** This can refer to an entry name (or "id"), and accession number or
** other queriable items.
**
** ajpseqquery is created with the entry specification part of a USA
** (Uniform Sequence Address). The syntax is currently related to that
** used by SRS release 5.1.
**
** @new ajSeqQueryNew Default constructor
** @delete ajSeqQueryDel Default destructor
** @set ajSeqQueryClear Clears all contents
** @set seqQueryMatch Compares an AjPSeq to a query.
** @set ajSeqQueryStarclear Clears fully wild elements of a query because
**                          empty elements are the same.
** @cast seqQueryWild Tests whether a query includes wildcards
** @cast seqQueryIs Tests whether a query has been defined
** @@
******************************************************************************/

typedef struct AjSSeqQuery {
  AjPStr DbName;		/* Database name */
  AjPStr DbType;		/* Database type */ 
  AjPStr Id;			/* ID Wildcard */
  AjPStr Acc;			/* Accession Wildcard */
  AjPStr Des;			/* Description Wildcard */
  AjPStr Key;			/* Keyword Wildcard */
  AjPStr Org;			/* Taxonomy Wildcard */
  AjPStr Sv;			/* SeqVersion Wildcard */
  AjPStr Method;		/* Name of access method */
  AjPStr Formatstr;		/* Name of input sequence format */
  AjPStr IndexDir;		/* Index directory */
  AjPStr Directory;		/* Data directory */
  AjPStr Filename;		/* Individual filename */
  AjPStr Exclude;		/* File wildcards to exclude (spaced) */
  AjPStr DbFields;		/* Query fields (plus id and acc) */
  AjPStr DbProxy;		/* Proxy host */
  AjPStr DbHttpVer;		/* HTTP Version */ 
  AjPStr Field;			/* Query level */
  AjPStr QryString;		/* Query level */
  AjPStr Application;		/* External application command */
  enum AjEQryType Type;		/* Enum query type */
  ajlong Fpos;			/* File position from fseek */
  AjBool QryDone;		/* Has the query been done yet */
  SeqSAccess* Access;	        /* Access function : see ajseqread.h */
  void* QryData;		/* Private data for access function */
} AjOSeqQuery, *AjPSeqQuery;


/* @data AjPSelexSQ *******************************************************
**
** Ajax Selex object for #=SQ information.
**
** @new ajSelexSQNew Default constructor
** @delete ajSelexSQDel Default destructor
** @@
******************************************************************************/

typedef struct AjSSelexSQ
{
    AjPStr name;
    AjPStr source;
    AjPStr ac;
    AjPStr de;
    float  wt;
    ajint  start;
    ajint  stop;
    ajint  len;
} AjOSelexSQ, *AjPSelexSQ;

/* @data AjPSelex *******************************************************
**
** Ajax Selex object.
**
** @new ajSelexNew Default constructor
** @delete ajSelexDel Default destructor
** @@
******************************************************************************/

typedef struct AjSSelex
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr cs;
    AjPStr rf;
    AjPStr *name;
    AjPStr *str;
    AjPStr *ss;
    float  ga[2];
    float  tc[2];
    float  nc[2];
    AjPSelexSQ *sq;
    ajint  n;
    ajint  Count;
} AjOSelex,*AjPSelex;


/* @data AjPSelexdata *******************************************************
**
** Ajax Selex data object (individual sequences)
**
** @new ajSelexdataNew Default constructor
** @delete ajSelexdataDel Default destructor
** @@
******************************************************************************/

typedef struct AjSSelexdata
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr cs;
    AjPStr rf;
    AjPStr name;
    AjPStr str;
    AjPStr ss;
    float  ga[2];
    float  tc[2];
    float  nc[2];
    AjPSelexSQ sq;
} AjOSelexdata,*AjPSelexdata;


/* @data AjPStockholm ***********************************************
**
** Ajax Stockholm object.
**
** @new ajStockholmNew Default constructor
** @delete ajStockholmDel Default destructor
** @@
******************************************************************************/

typedef struct AjSStockholm
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr al;
    AjPStr tp;
    AjPStr se;
    ajint  ga[2];
    float  tc[2];
    float  nc[2];
    AjPStr bm;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr sacons;
    AjPStr sscons;
    AjPStr gs;
    AjPStr *name;
    AjPStr *str;
    ajint  n;
    ajint  Count;
} AjOStockholm,*AjPStockholm;


/* @data AjPStockholmdata *****************************************************
**
** Ajax Stockholm data object (individual sequences)
**
** @new ajStockholmdataNew Default constructor
** @delete ajStockholmdataDel Default destructor
** @@
******************************************************************************/

typedef struct AjSStockholmdata
{
    AjPStr id;
    AjPStr ac;
    AjPStr de;
    AjPStr au;
    AjPStr al;
    AjPStr tp;
    AjPStr se;
    AjPStr bm;
    AjPStr sscons;
    AjPStr sacons;
    AjPStr ref;
    AjPStr dc;
    AjPStr dr;
    AjPStr cc;
    AjPStr gs;
    float  ga[2];
    float  tc[2];
    float  nc[2];
} AjOStockholmdata,*AjPStockholmdata;


/* @data AjPSeqin *******************************************************
**
** Ajax Sequence Input object.
**
** Holds the sequence specification and information needed to read
** the sequence and possible further sequences.
**
** Also holds information on a selected sequence range and other
** options.
**
** @new ajSeqinNew Default constructor
** @delete ajSeqinDel Default destructor
** @set ajSeqinUsa Resets using a new USA
** @set ajSeqinClear Resets ready for reuse.
** @set ajSeqinSetRange Sets a sequence range for all input sequences
** @use ajSeqRead Reading a sequence.
** @use ajSeqsetRead Reading a sequence set.
** @use ajSeqAllRead Reading a sequence stream.
** @other AjPSeq Sequences
** @other AjPSeqset Sequence sets
** @other AjPSeqall Sequence streams
** @@
******************************************************************************/

typedef struct AjSSeqin {
  AjPStr Name;			/* Sequence name (replace) */
  AjPStr Acc;			/* Sequence accession number (replace) */
  AjPStr Inputtype;		/* Sequence type from ACD */
  AjPStr Type;			/* Sequence type N or P */
  AjPStr Db;			/* Database name (replace) */
  AjPStr Setdb;			/* Database name (replace) */
  AjPStr Full;			/* Full name */
  AjPStr Date;			/* Date */
  AjPStr Desc;			/* One-line description */
  AjPStr Doc;			/* Full text */
  AjPStr Inseq;			/* temporary input sequence holder */
  ajint Begin;			/* Start position */
  ajint End;			/* End position */
  AjBool Rev;			/* Reverse/complement if true */
  AjPList List;			/* List of USAs to be read */
  AjPStr Usa;			/* USA for the sequence */
  AjPStr Ufo;			/* UFO for features (if any) */
  AjPFeattable Fttable;		/* Input feature table (why in AjPSeqin?) */
  AjPFeattabIn Ftquery;		/* Feature table input spec */
  AjPStr Formatstr;		/* Sequence input format name */
  AjEnum Format;		/* Sequence input format enum */
  AjPStr Filename;		/* Original filename */
  AjPStr Entryname;		/* Entry name */
  AjPFileBuff Filebuff;		/* Input sequence buffered file */
  AjBool Search;		/* Search for more entries (always true?) */
  AjBool Single;		/* Read single entries */
  AjBool Features;		/* true: read features if any */
  AjBool IsNuc;			/* true: known to be nucleic */
  AjBool IsProt;		/* true: known to be protein */
  AjBool multi;			/* ???? see also Single */
  AjBool Lower;			/* true: convert to lower case -slower */
  AjBool Upper;			/* true: convert to upper case -supper */
  AjBool Text;			/* true: save full text of entry */
  ajint Count;			/* count of entries so far. Used
			           when ACD reads first sequence and
				   we need to reuse it in a Next loop */
  ajint Filecount;		/* Unused */
  ajlong Fpos;			/* File position (fseek) for building USA */
  AjPSeqQuery Query;		/* Query data - see AjPSeqQuery */
  AjPSelex Selex;
  AjPStockholm Stockholm;
  void *Data;			/* Format data for reuse,
				   e.g. multiple sequence input */
} AjOSeqin, *AjPSeqin;



/* @data AjPSeq *******************************************************
**
** Ajax Sequence object.
**
** Holds the sequence itself, plus associated information such as a
** sequence name, accession number, format, type.
**
** Also holds information on a selected sequence range and other
** options.
**
** Sequence features can also be stored, but for efficiency reasons
** features are turned off by default.
**
** @new ajSeqNew Default constructor
** @new ajSeqNewL Constructor with expected maximum size.
** @new ajSeqNewS Constructor with sequence object to be cloned.
** @delete ajSeqDel Default destructor
** @set ajSeqRead Master sequence input, calls specific functions for
**                file access type and sequence format.
** @set ajSeqAllRead Master sequence stream input, reads first sequence
**                   from an open input stream.
** @set ajSeqallNext Master sequence stream input, reads next sequence
**                   from an open input stream.
** @set ajSeqMod Sets a sequence as modifiable by making its sequence
**               into a unique AjPStr.
** @set ajSeqReplace Replaces a sequence with a string containing a modified
**                   version.
** @set ajSeqReplaceC Replaces a sequence with a char* containing a modified
**                   version.
** @set ajSeqSetRange Sets a sequence using specified start and end positions.
** @set ajSeqType Sets the sequence type
** @set ajSeqSetNuc Sets sequence to be nucleotide
** @set ajSeqSetProt Sets sequence to be protein
** @set ajSeqToLower Converts a sequence to lower case
** @set ajSeqToUpper Converts a sequence to upper case
** @set ajSeqReverse Reverse complements a nucleotide sequence
** @set ajSeqReverseStr Reverse complements a nucleotide sequence as an AjPStr
** @set ajSeqRevOnly Reverses a sequence (does not complement)
** @set ajSeqCompOnly Complements a nucleotide sequence (does not reverse)
** @set ajSeqCompOnlyStr Complements a nucleotide sequence AjPStr
**                       (does not reverse)
** @cast ajSeqChar Returns the actual char* holding the sequence.
** @cast ajSeqCharCopy Returns a copy of the sequence as char*.
** @cast ajSeqCharCopyL Returns a copy of the sequence as char* with
**                      a specified minimum reserved length.
** @cast ajSeqStr Returns the actual AjPStr holding the sequence.
** @cast ajSeqCopyStr Returns a copy of the AjPStr holding the sequence.
** @cast ajSeqName Returns the sequence name as char*
** @cast ajSeqLen Returns the sequence length
** @cast ajSeqBegin Returns the sequence start position
** @cast ajSeqEnd Returns the sequence end position
** @cast ajSeqCheckGcg Calculates the GCG checksum for a sequence.
** @use ajSeqWrite Writes a sequence out.
** @use ajSeqIsNuc tests whether a sequence is nucleotide
** @use ajSeqIsProt tests whether a sequence is protein
** @use ajSeqNum Converts a sequence to numbers
** @use ajSeqTrace Reports the contents of a sequence
** @other AjPSeqset Sequence sets
** @other AjPSeqall Sequence streams
** @@
******************************************************************************/
typedef struct AjSSeq {
  AjPStr Name;			/* Name (ID) */
  AjPStr Acc;			/* Accession number (primary only) */
  AjPStr Sv;			/* SeqVersion number */
  AjPStr Gi;			/* GI NCBI version number */
  AjPStr Tax;			/* Main taxonomy (species) */
  AjPStr Type;			/* Type N or P */
  AjEnum EType;			/* unused, obsolete */
  AjPStr Db;			/* Database name */
  AjPStr Setdb;			/* Database name from command line */
  AjPStr Full;			/* Full name */
  AjPStr Date;			/* Date */
  AjPStr Desc;			/* One-line description */
  AjPStr Doc;			/* Obsolete - see TextPtr */
  AjBool Rev;			/* true: reverse-complemented */
  ajint Begin;			/* start position (processed on reading) */
  ajint End;			/* end position (processed on reading)( */
  ajint Offset;			/* offset from start */
  ajint Offend;			/* offset from end */
  ajlong Fpos;			/* File position (fseek) for USA */
  AjPStr Usa;			/* USA fo re-reading */
  AjPStr Ufo;			/* UFO for re-reading */
  AjPFeattable Fttable;		/* Feature table */
  AjPStr Formatstr;		/* Input format name */
  AjEnum Format;		/* Input format enum */
  AjPStr Filename;		/* Original filename */
  AjPStr Entryname;		/* Entryname (ID) */
  AjPStr TextPtr;		/* Full text */
  float Weight;			/* Weight from multiple alignment */
  AjPList Acclist;		/* Secondary accessions */
  AjPList Keylist;		/* Keyword list */
  AjPList Taxlist;		/* Taxonomy list (just species for now) */
  AjPStr Seq;			/* The sequence */
  AjPSelexdata Selexdata;
  AjPStockholmdata Stock;
} AjOSeq, *AjPSeq;

/* @data AjPSeqset *******************************************************
**
** Ajax Sequence set object. A sequence set contains one or more
** sequences together in memory, for example as a sequence alignment.
**
** Holds the sequence set itself, plus associated information such as a
** sequence names, accession number, format, type.
**
** Also holds information on a selected sequence range and other
** options.
**
** Sequence set features can also be stored, but for efficiency reasons
** features are turned off by default.
**
** @new ajSeqsetNew Default constructor
** @delete none Typically left until the program ends so there
**              is no destructor used.
** @set ajSeqsetRead Master input routine for a sequence set
** @set ajSeqsetToLower Converts a sequence set to lower case
** @set ajSeqsetToUpper Converts a sequence set to upper case
** @cast ajSeqsetLen Returns the maximum length of a sequence set
** @cast ajSeqsetSize Returns the number of sequences in a sequence set
** @cast ajSeqsetName Returns the name of a sequence in a set
** @cast ajSeqsetSeq Returns the char* pointer to a sequence in a set
** @cast ajSeqsetIsNuc Tests whether the sequence set is nucleotide
** @cast ajSeqsetIsProt Tests whether the sequence set is protein
** @use ajSeqsetWrite Writes out all sequences in a set
** @other AjPSeq Sequences
** @other AjPSeqall Sequence streams
** @@
******************************************************************************/

typedef struct AjSSeqset {
  ajint Size;			/* Number of sequences */
  ajint Len;			/* Maximum sequence length */
  ajint Begin;			/* start position */
  ajint End;			/* end position */
  AjBool Rev;			/* true: reverse-complemented */
  float Totweight;		/* total weight (usually 1.0 * Size) */
  AjPStr Type;			/* Type N or P */
  AjEnum EType;			/* enum type obsolete */
  AjPStr Formatstr;		/* Input format name */
  AjEnum Format;		/* Input format enum */
  AjPStr Filename;		/* Original filename */
  AjPStr Full;			/* Full name */
  AjPStr Name;			/* Name */
  AjPStr Usa;			/* USA for re-reading */
  AjPStr Ufo;			/* UFO for re-reading */
  AjPSeq* Seq;			/* Sequence array (see Size) */
  float* Seqweight;		/* Sequence weights (see also AjPSeq) */
} AjOSeqset, *AjPSeqset;



/* @data AjPSeqall *******************************************************
**
** Ajax Sequence all (stream) object.
**
** Inherits an AjPSeq but allows more sequences to be read from the
** same input by also inheriting the AjPSeqin input object.
**
** @new ajSeqallNew Default constructor
** @delete none Typically left until the program ends so there
**              is no destructor used. 
** @set ajSeqallNext Master sequence stream input.
** @other AjPSeq Sequences
** @other AjPSeqin Sequence input
** @other AjPSeqset Sequence sets
** @@
******************************************************************************/

typedef struct AjSSeqall {
  AjPSeq Seq;			/* Current sequence */
  AjPSeqin Seqin;		/* Sequence input for reading next */
  ajint Count;			/* Count of sequences so far */
  ajint Begin;			/* start position */
  ajint End;			/* end position */
  AjBool Rev;			/* true: reverse-complement */
} AjOSeqall, *AjPSeqall;

#endif

#ifdef __cplusplus
}
#endif

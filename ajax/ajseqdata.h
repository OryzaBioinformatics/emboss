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
  AjPStr DbName;
  AjPStr DbType;
  AjPStr Id;
  AjPStr Acc;
  AjPStr Des;
  AjPStr Method;
  AjPStr Formatstr;
  AjPStr IndexDir;
  AjPStr Directory;
  AjPStr Filename;
  AjPStr Exclude;
  AjPStr Application;
  enum AjEQryType Type;
  long Fpos;
  SeqSAccess* Access;
  void* QryData;
} AjOSeqQuery, *AjPSeqQuery;

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
  AjPStr Name;
  AjPStr Acc;
  AjPStr Inputtype;
  AjPStr Type;
  AjPStr Db;
  AjPStr Setdb;
  AjPStr Full;
  AjPStr Date;
  AjPStr Desc;
  AjPStr Doc;
  AjPStr Inseq;			/* temporary input sequence holder */
  int Begin;
  int End;
  AjBool Rev;
  AjPList List;
  AjPStr Usa;
  AjPStr Ufo;
  AjPFeatTable Fttable;
  AjPFeatTabIn Ftquery;
  AjPStr Formatstr;
  AjEnum Format;
  AjPStr Filename;
  AjPStr Entryname;
  AjPFileBuff Filebuff;
  AjBool Search;
  AjBool Single;
  AjBool Features;
  AjBool IsNuc;
  AjBool IsProt;
  AjBool multi;
  int Count;
  int Filecount;
  long Fpos;
  AjPSeqQuery Query;
  void *Data;
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
  AjPStr Name;
  AjPStr Acc;
  AjPStr Type;
  AjEnum EType;
  AjPStr Db;
  AjPStr Setdb;
  AjPStr Full;
  AjPStr Date;
  AjPStr Desc;
  AjPStr Doc;
  AjBool Rev;
  int Begin;
  int End;
  int Offset;
  int Offend;
  long Fpos;
  AjPStr Usa;
  AjPStr Ufo;
  AjPFeatTable Fttable;
  AjPStr Formatstr;
  AjEnum Format;
  AjPStr Filename;
  AjPStr Entryname;
  float Weight;
  AjPList Acclist;
  AjPStr Seq;
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
  int Size;
  int Len;
  int Begin;
  int End;
  AjBool Rev;
  float Totweight;
  AjPStr Type;
  AjEnum EType;
  AjPStr Formatstr;
  AjEnum Format;
  AjPStr Filename;
  AjPStr Full;
  AjPStr Name;
  AjPStr Usa;
  AjPStr Ufo;
  AjPSeq* Seq;
  float* Seqweight;
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
  AjPSeq Seq;
  AjPSeqin Seqin;
  int Count;
  int Begin;
  int End;
  AjBool Rev;
} AjOSeqall, *AjPSeqall;

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqwrite_h
#define ajseqwrite_h




/* @data AjPSeqout ************************************************************
**
** Ajax Sequence Output object.
**
** Holds definition of sequence output.
**
** @attr Name [AjPStr] Name (ID)
** @attr Acc [AjPStr] Accession number (primary only)
** @attr Sv [AjPStr] SeqVersion number
** @attr Gi [AjPStr] GI NCBI version number
** @attr Tax [AjPStr] Main taxonomy (species)
** @attr Desc [AjPStr] One-line description
** @attr Type [AjPStr] Type N or P
** @attr Outputtype [AjPStr] Output sequence known type
** @attr Db [AjPStr] Database name from input name
** @attr Setdb [AjPStr] Database name from input command line
** @attr Setoutdb [AjPStr] Database name from command line
** @attr Full [AjPStr] Full name
** @attr Date [AjPStr] Date
** @attr Doc [AjPStr] Obsolete - see TextPtr
** @attr Rev [AjBool] true: to be reverse-complemented
** @attr Offset [ajint] offset from start
** @attr Usa [AjPStr] USA for re-reading
** @attr Ufo [AjPStr] UFO for re-reading
** @attr Fttable [AjPFeattable] Feature table
** @attr Ftquery [AjPFeattabOut] Feature table output
** @attr FtFormat [AjPStr] Feature output format (if not in UFO)
** @attr FtFilename [AjPStr] Feature output filename (if not in UFO)
** @attr Informatstr [AjPStr] Input format
** @attr Formatstr [AjPStr] Output format
** @attr EType [AjEnum] unused, obsolete
** @attr Format [AjEnum] Output format index
** @attr Filename [AjPStr] Output filename (if not in USA)
** @attr Directory [AjPStr] Output firectory
** @attr Entryname [AjPStr] Entry name
** @attr Acclist [AjPList] Secondary accessions
** @attr Keylist [AjPList] Keyword list
** @attr Taxlist [AjPList] Taxonomy list (just species for now)
** @attr Seq [AjPStr] The sequence
** @attr File [AjPFile] Output file
** @attr Knownfile [AjPFile] Already open output file (we don't close this one)
** @attr Single [AjBool] If true, single sequence in each file (-ossingle)
** @attr Features [AjBool] If true, save features with sequence or in file
** @attr Extension [AjPStr] File extension
** @attr Accuracy [ajint*] Accuracy values (one per base) from base calling
** @attr Savelist [AjPList] Previous sequences saved for later output
**                          (e.g. MSF format)
** @attr Count [ajint] Number of sequences
** @attr Padding [char[4]] Padding to alignment boundary
**
** @new ajSeqoutNew Default constructor
** @delete ajSeqoutDel Default destructor
** @modify ajSeqoutUsa Resets using a new USA
** @modify ajSeqoutClear Resets ready for reuse.
** @modify ajSeqoutOpen If the file is not yet open, calls seqoutUsaProcess
** @cast ajSeqoutCheckGcg Calculates the GCG checksum for a sequence set.
** @modify ajSeqWrite Master sequence output routine
** @modify ajSeqsetWrite Master sequence set output routine
** @modify ajSeqFileNewOut Opens an output file for sequence writing.
** @other AjPSeq Sequences
** @@
******************************************************************************/

typedef struct AjSSeqout {
  AjPStr Name;
  AjPStr Acc;
  AjPStr Sv;
  AjPStr Gi;
  AjPStr Tax;
  AjPStr Desc;
  AjPStr Type;
  AjPStr Outputtype;
  AjPStr Db;
  AjPStr Setdb;
  AjPStr Setoutdb;
  AjPStr Full;
  AjPStr Date;
  AjPStr Doc;
  AjBool Rev;
  ajint Offset;
  AjPStr Usa;
  AjPStr Ufo;
  AjPFeattable Fttable;
  AjPFeattabOut Ftquery;
  AjPStr FtFormat;
  AjPStr FtFilename;
  AjPStr Informatstr;
  AjPStr Formatstr;
  AjEnum EType;
  AjEnum Format;
  AjPStr Filename;
  AjPStr Directory;
  AjPStr Entryname;
  AjPList Acclist;
  AjPList Keylist;
  AjPList Taxlist;
  AjPStr Seq;
  AjPFile File;
  AjPFile Knownfile;
  AjBool Single;
  AjBool Features;
  AjPStr Extension;
  ajint* Accuracy;
  AjPList Savelist;
  ajint Count;
  char Padding[4];
} AjOSeqout;

#define AjPSeqout AjOSeqout*




/*
** Prototype definitions
*/

__deprecated void          ajSeqAllWrite (AjPSeqout outseq, const AjPSeq seq);
void         ajSeqoutWriteSeq (AjPSeqout outseq, const AjPSeq seq);
AjBool       ajSeqoutOpenFilename (AjPSeqout seqout, const AjPStr name);
__deprecated AjBool        ajSeqFileNewOut (AjPSeqout seqout,
					   const AjPStr name);
ajint        ajSeqoutCheckGcg (const AjPSeqout outseq);
void         ajSeqoutClear (AjPSeqout thys);
void         ajSeqoutCount(const AjPSeqout seqout, ajuint* b);
void         ajSeqoutDefName(AjPSeqout thys,
			     const AjPStr setname, AjBool multi);
void         ajSeqoutDel (AjPSeqout* thys);
AjBool       ajSeqOutFormatDefault (AjPStr* pformat);
AjBool       ajSeqOutFormatSingle (AjPStr format);
AjPSeqout    ajSeqoutNew (void);
AjPSeqout    ajSeqoutNewFile (AjPFile file);
__deprecated AjPSeqout     ajSeqoutNewF (AjPFile file);
AjBool       ajSeqoutOpen (AjPSeqout thys);
AjBool       ajSeqOutSetFormat (AjPSeqout thys, const AjPStr format);
AjBool       ajSeqOutSetFormatC (AjPSeqout thys, const char* format);
void         ajSeqoutTrace (const AjPSeqout seq);
void         ajSeqPrintOutFormat (AjPFile outf, AjBool full);
void         ajSeqoutUsa (AjPSeqout* pthis, const AjPStr Usa);
void         ajSeqoutWriteSet (AjPSeqout seqout, const AjPSeqset seq);
__deprecated void          ajSeqWrite (AjPSeqout seqout, const AjPSeq seq);
void         ajSeqoutClose (AjPSeqout outseq);
__deprecated void          ajSeqWriteClose (AjPSeqout outseq);
void         ajSeqWriteExit(void);
void         ajSeqWriteXyz(AjPFile outf, const AjPStr seq, const char *prefix);
void         ajSssWriteXyz(AjPFile outf, const AjPStr seq, const char *prefix);
__deprecated void         ajSeqsetWrite(AjPSeqout outseq, const AjPSeqset seq);
/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

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
** @new ajSeqoutNew Default constructor
** @delete ajSeqoutDel Default destructor
** @set ajSeqoutUsa Resets using a new USA
** @set ajSeqoutClear Resets ready for reuse.
** @set ajSeqoutOpen If the file is not yet open, calls seqoutUsaProcess
** @cast ajSeqoutCheckGcg Calculates the GCG checksum for a sequence set.
** @use ajSeqWrite Master sequence output routine
** @use ajSeqsetWrite Master sequence set output routine
** @use ajSeqFileNewOut Opens an output file for sequence writing.
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
  AjEnum EType;
  AjPStr Outputtype;
  AjPStr Db;
  AjPStr Setdb;
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
  AjEnum Format;
  AjPStr Filename;
  AjPStr Directory;
  AjPStr Entryname;
  AjPList Acclist;
  AjPList Keylist;
  AjPList Taxlist;
  AjPStr Seq;
  AjPFile File;
  AjPFile Knownfile;		/* File provided externally */
  AjBool Single;
  AjBool Features;
  AjPStr Extension;
  ajint Count;
  AjPList Savelist;
} AjOSeqout;

#define AjPSeqout AjOSeqout*

void         ajSeqAllWrite (AjPSeqout outseq, AjPSeq seq);
AjBool       ajSeqFileNewOut (AjPSeqout seqout, const AjPStr name);
ajint        ajSeqoutCheckGcg (const AjPSeqout outseq);
void         ajSeqoutClear (AjPSeqout thys);
void         ajSeqoutDel (AjPSeqout* thys);
AjBool       ajSeqOutFormatDefault (AjPStr* pformat);
AjBool       ajSeqOutFormatSingle (AjPStr format);
AjPSeqout    ajSeqoutNew (void);
AjPSeqout    ajSeqoutNewF (AjPFile file);
AjBool       ajSeqoutOpen (AjPSeqout thys);
AjBool       ajSeqOutSetFormat (AjPSeqout thys, const AjPStr format);
AjBool       ajSeqOutSetFormatC (AjPSeqout thys, const char* format);
void         ajSeqoutTrace (const AjPSeqout seq);
void         ajSeqPrintOutFormat (const AjPFile outf, AjBool full);
void         ajSeqoutUsa (AjPSeqout* pthis, const AjPStr Usa);
void         ajSeqsetWrite (AjPSeqout seqout, const AjPSeqset seq);
void         ajSeqWrite (AjPSeqout seqout, const AjPSeq seq);
void         ajSeqWriteClose (AjPSeqout outseq);
void         ajSeqWriteXyz(AjPFile outf, const AjPStr seq, const char *prefix);
void         ajSssWriteXyz(AjPFile outf, const AjPStr seq, const char *prefix);


#endif

#ifdef __cplusplus
}
#endif

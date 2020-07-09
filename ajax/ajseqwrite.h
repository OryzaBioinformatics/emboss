#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqwrite_h
#define ajseqwrite_h

/* @data AjPSeqout *******************************************************
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
  AjPStr Desc;
  AjPStr Type;
  AjEnum EType;
  AjPStr Db;
  AjPStr Setdb;
  AjPStr Full;
  AjPStr Date;
  AjPStr Doc;
  AjBool Rev;
  ajint Offset;
  AjPStr Usa;
  AjPStr Ufo;
  AjPFeatTable Fttable;
  AjPFeatTabOut Ftquery;
  AjPStr FtFormat;
  AjPStr FtFilename;
  AjPStr Informatstr;
  AjPStr Formatstr;
  AjEnum Format;
  AjPStr Filename;
  AjPStr Entryname;
  AjPStr Seq;
  AjPFile File;
  AjBool Single;
  AjBool Features;
  AjPStr Extension;
  ajint Count;
  AjPList Savelist;
} AjOSeqout, *AjPSeqout;

void         ajSeqAllWrite (AjPSeqout outseq, AjPSeq seq);
AjBool       ajSeqFileNewOut (AjPSeqout seqout, AjPStr name);
ajint          ajSeqoutCheckGcg (AjPSeqout outseq);
void         ajSeqoutClear (AjPSeqout thys);
void         ajSeqoutDel (AjPSeqout* thys);
AjBool       ajSeqOutFormatDefault (AjPStr* pformat);
AjBool       ajSeqOutFormatSingle (AjPStr format);
AjPSeqout    ajSeqoutNew (void);
AjBool       ajSeqoutOpen (AjPSeqout thys);
AjBool       ajSeqOutSetFormat (AjPSeqout thys, AjPStr format);
AjBool       ajSeqOutSetFormatC (AjPSeqout thys, char* format);
void         ajSeqoutTrace (AjPSeqout seq);
void         ajSeqPrintOutFormat (AjPFile outf, AjBool full);
void         ajSeqoutUsa (AjPSeqout* pthis, AjPStr Usa);
void         ajSeqsetWrite (AjPSeqout seqout, AjPSeqset seq);
void         ajSeqWrite (AjPSeqout seqout, AjPSeq seq);
void         ajSeqWriteClose (AjPSeqout outseq);
void         ajSeqWriteCdb (AjPFile outf, AjPStr seq);

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqread_h
#define ajseqread_h

/* @data SeqPAccess *****************************************************
**
** Ajax Sequence Access database reading object.
**
** Holds information needed to read a sequence from a database.
** Access methods are defined for each known database type.
**
** Sequences are read from the database using the defined
** database access function, which is usually a static function
** within ajseq.c
**
** This should be a static data object but is needed for the definition
** of AjPSeqin.
**
** @alias SeqSAccess
** @new seqMethod returns a copy of a known access method definition.
** @other AjPSeqin Sequence input
** @@
******************************************************************************/

typedef struct SeqSAccess {
  char *Name;			/* Access method name used in emboss.default */
  AjBool (*Access) (AjPSeqin seqin); /* Access function */
} SeqOAccess, *SeqPAccess;

AjPSeqall    ajSeqallFile (AjPStr usa);
AjBool       ajSeqAllRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqGetFromUsa (AjPStr thys, AjBool protein, AjPSeq *seq);
void         ajSeqinClear (AjPSeqin thys);
void         ajSeqinDel (AjPSeqin* pthis);
AjPSeqin     ajSeqinNew (void);
AjBool       ajSeqParseFasta(AjPStr str, AjPStr* id, AjPStr* acc, AjPStr* desc);
AjBool       ajSeqParseNcbi(AjPStr str, AjPStr* id, AjPStr* acc, AjPStr* desc);
void         ajSeqinSetNuc (AjPSeqin seqin);
void         ajSeqinSetProt (AjPSeqin seqin);
void         ajSeqinSetRange (AjPSeqin seqin, ajint ibegin, ajint iend);
AjBool       ajSeqReadSelex(AjPFile inf, AjPFile outf);
void         ajSeqinUsa (AjPSeqin* pthis, AjPStr Usa);
void         ajSeqPrintInFormat (AjPFile outf, AjBool full);
AjBool       ajSeqRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqsetRead (AjPSeqset thys, AjPSeqin seqin);

#endif

#ifdef __cplusplus
}
#endif

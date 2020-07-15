#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqread_h
#define ajseqread_h




/* @data SeqPAccess ***********************************************************
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
** @new ajSeqMethod returns a copy of a known access method definition.
** @other AjPSeqin Sequence input
**
** @attr Name [char*] Access method name used in emboss.default
** @attr Alias [AjBool] Alias for another name
** @attr Entry [AjBool] Supports retrieval of single entries
** @attr Query [AjBool] Supports retrieval of selected entries
** @attr All [AjBool] Supports retrieval of all entries
** @attr Access [(AjBool*)] Access function
** @attr AccessFree [(AjBool*)] Access cleanup function
** @attr Desc [char*] Description
** @@
******************************************************************************/

typedef struct SeqSAccess {
  char *Name;
  AjBool Alias;
  AjBool Entry;
  AjBool Query;
  AjBool All;
  AjBool (*Access) (AjPSeqin seqin);
  AjBool (*AccessFree) (void* qry);
  char* Desc;
} SeqOAccess;

#define SeqPAccess SeqOAccess*




/*
** Prototype definitions
*/

AjPSeqall    ajSeqallFile (const AjPStr usa);
AjBool       ajSeqAllRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqGetFromUsa (const AjPStr thys, AjBool protein, AjPSeq *seq);
AjBool       ajSeqFormatTest (const AjPStr format);
void         ajSeqinClear (AjPSeqin thys);
void         ajSeqinClearPos(AjPSeqin thys);
void         ajSeqinDel (AjPSeqin* pthis);
AjPSeqin     ajSeqinNew (void);
void         ajSeqinSetNuc (AjPSeqin seqin);
void         ajSeqinSetProt (AjPSeqin seqin);
void         ajSeqinSetRange (AjPSeqin seqin, ajint ibegin, ajint iend);
void         ajSeqinUsa (AjPSeqin* pthis, const AjPStr Usa);
void         ajSeqinTrace (const AjPSeqin thys);

AjBool       ajSeqParseFasta(const AjPStr str, AjPStr* id, AjPStr* acc,
			     AjPStr* sv, AjPStr* desc);
AjBool       ajSeqParseNcbi(const AjPStr str, AjPStr* id, AjPStr* acc,
			    AjPStr* sv, AjPStr* gi, AjPStr* desc);
void         ajSeqQueryClear (AjPSeqQuery thys);
void         ajSeqQueryDel (AjPSeqQuery *pthis);
AjBool       ajSeqQueryIs (const AjPSeqQuery qry);
AjPSeqQuery  ajSeqQueryNew (void);
void         ajSeqQueryStarclear (AjPSeqQuery qry);
void         ajSeqQueryTrace (const AjPSeqQuery qry);
AjBool       ajSeqQueryWild (AjPSeqQuery qry);
void         ajSeqPrintInFormat (AjPFile outf, AjBool full);
AjBool       ajSeqRead (AjPSeq thys, AjPSeqin seqin);
void         ajSeqReadExit(void);
ajint        ajSeqsetApp (AjPSeqset thys, const AjPSeq seq);
AjBool       ajSeqsetGetFromUsa(const AjPStr thys, AjPSeqset *seq);
AjBool       ajSeqsetFromList (AjPSeqset thys, const AjPList list);
ajint        ajSeqsetFromPair (AjPSeqset thys,
			       const AjPSeq seqa, const AjPSeq seqb);
AjBool       ajSeqsetallRead (AjPList thys, AjPSeqin seqin);
AjBool       ajSeqsetRead (AjPSeqset thys, AjPSeqin seqin);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

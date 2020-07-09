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
  char *Name;
  AjBool (*Access) (AjPSeqin seqin);
} SeqOAccess, *SeqPAccess;

AjPSeqall    ajSeqallFile (AjPStr usa);
AjBool       ajSeqAllRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqGetFromUsa (AjPStr thys, AjBool protein, AjPSeq *seq);
void         ajSeqinClear (AjPSeqin thys);
void         ajSeqinDel (AjPSeqin* pthis);
AjPSeqin     ajSeqinNew (void);
AjBool       ajSeqParseNcbi(AjPStr str, AjPStr* id, AjPStr* acc, AjPStr* desc);
void         ajSeqinSetNuc (AjPSeqin seqin);
void         ajSeqinSetProt (AjPSeqin seqin);
void         ajSeqinSetRange (AjPSeqin seqin, int ibegin, int iend);
void         ajSeqinUsa (AjPSeqin* pthis, AjPStr Usa);
void         ajSeqPrintInFormat (AjPFile outf, AjBool full);
AjBool       ajSeqRead (AjPSeq thys, AjPSeqin seqin);
AjBool       ajSeqsetRead (AjPSeqset thys, AjPSeqin seqin);
AjBool 	     ajSeqABITest(AjPFile fp);
AjBool 	     ajSeqABIReadSeq(AjPFile fp,long int baseO,long int numBases,
                             AjPStr* nseq);
AjBool 	     ajSeqABIMachineName(AjPFile fp,AjPStr* machine);
int 	     ajSeqABIGetNData(AjPFile fp);
int	     ajSeqABIGetNBase(AjPFile fp);
void 	     ajSeqABIGetData(AjPFile fp,long int *Offset,long int numPoints, 
                             AjPInt2d trace);
void 	     ajSeqABIGetBasePosition(AjPFile fp,long int numBases, 
                             AjPShort* basePositions);
void 	     ajSeqABIGetSignal(AjPFile fp,long int fwo_,
                    	       short int sigC,short int sigA,
                   	       short int sigG,short int sigT);
float 	     ajSeqABIGetBaseSpace(AjPFile fp);
int	     ajSeqABIGetBaseOffset(AjPFile fp);
int 	     ajSeqABIGetBasePosOffset(AjPFile fp);
int	     ajSeqABIGetFWO(AjPFile fp);
int 	     ajSeqABIGetPrimerOffset(AjPFile fp);
int	     ajSeqABIGetPrimerPosition(AjPFile fp);
AjBool 	     ajSeqABIGetTraceOffset(AjPFile fp,long int *Offset);



#endif

#ifdef __cplusplus
}
#endif

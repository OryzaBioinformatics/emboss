#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqtype_h
#define ajseqtype_h

void         ajSeqGap (AjPSeq thys, char gapc, char padc);
void         ajSeqGapLen (AjPSeq thys, char gapc, char padc, ajint ilen);
void         ajSeqGapS (AjPStr* seq, char gapc);
void         ajSeqPrintType (AjPFile outf, AjBool full);
void         ajSeqSetNuc (AjPSeq thys);
void         ajSeqSetProt (AjPSeq thys);
char         ajSeqTypeAny (AjPSeq thys);
char         ajSeqTypeDna (AjPSeq thys);
char         ajSeqTypeGapany (AjPSeq thys);
char         ajSeqTypeGapdna (AjPSeq thys);
char         ajSeqTypeGapnuc (AjPSeq thys);
char         ajSeqTypeAnyprot (AjPSeq thys);
char         ajSeqTypeGapprot (AjPSeq thys);
char         ajSeqTypeGaprna (AjPSeq thys);
char         ajSeqTypeNuc (AjPSeq thys);
char         ajSeqTypeProt (AjPSeq thys);
char         ajSeqTypePuredna (AjPSeq thys);
char         ajSeqTypePurenuc (AjPSeq thys);
char         ajSeqTypePureprot (AjPSeq thys);
char         ajSeqTypePurerna (AjPSeq thys);
char         ajSeqTypeRna (AjPSeq thys);
char         ajSeqTypeStopprot (AjPSeq thys);
void         ajSeqType (AjPSeq thys);
AjBool       ajSeqTypeCheck (AjPSeq thys, AjPSeqin seqin);

#endif

#ifdef __cplusplus
}
#endif

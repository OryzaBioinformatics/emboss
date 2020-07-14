#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h

AjBool     ajSeqAccessAsis (AjPSeqin seqin);
AjBool     ajSeqAccessFile (AjPSeqin seqin);
AjBool     ajSeqAccessOffset (AjPSeqin seqin);
SeqPAccess ajSeqMethod (const AjPStr method);
AjBool     ajSeqMethodTest (const AjPStr method);
void       ajSeqPrintAccess (AjPFile outf, AjBool full);

#endif

#ifdef __cplusplus
}
#endif

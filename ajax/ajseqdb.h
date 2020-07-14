#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h

AjBool     ajSeqAccessAsis (AjPSeqin seqin);
AjBool     ajSeqAccessFile (AjPSeqin seqin);
AjBool     ajSeqAccessOffset (AjPSeqin seqin);
AjBool     ajSeqMethod (const AjPStr method, SeqPAccess* access);
AjBool     ajSeqMethodTest (const AjPStr method);
void       ajSeqPrintAccess (const AjPFile outf, AjBool full);

#endif

#ifdef __cplusplus
}
#endif

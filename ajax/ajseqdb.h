#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h

AjBool     ajSeqAccessAsis (AjPSeqin seqin);
AjBool     ajSeqAccessFile (AjPSeqin seqin);
AjBool     ajSeqAccessOffset (AjPSeqin seqin);
AjBool     ajSeqMethod (AjPStr method, SeqPAccess* access);
void       ajSeqPrintAccess (AjPFile outf, AjBool full);

#endif

#ifdef __cplusplus
}
#endif

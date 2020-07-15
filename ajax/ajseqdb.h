#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqdb_h
#define ajseqdb_h




/*
** Prototype definitions
*/

AjBool     ajSeqAccessAsis (AjPSeqin seqin);
AjBool     ajSeqAccessFile (AjPSeqin seqin);
AjBool     ajSeqAccessOffset (AjPSeqin seqin);
void       ajSeqDbExit(void);
SeqPAccess ajSeqMethod (const AjPStr method);
AjBool     ajSeqMethodTest (const AjPStr method);
void       ajSeqPrintAccess (AjPFile outf, AjBool full);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

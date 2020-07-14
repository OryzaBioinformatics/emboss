#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embcons_h
#define embcons_h

void embConsCalc (const AjPSeqset seqset, const AjPMatrix cmpmatrix,
	ajint nseqs, ajint mlen,float fplural, float setcase,
	ajint identity, AjPStr *cons);


#endif

#ifdef __cplusplus
}
#endif

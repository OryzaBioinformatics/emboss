#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embindex_h
#define embindex_h

void   embBtreeEmblKW(const AjPStr kwline, AjPList kwlist);
void   embBtreeEmblDE(const AjPStr deline, AjPList kwlist);    
ajint  embBtreeReadDir(AjPStr **filelist, const AjPStr fdirectory,
		       const AjPStr files, const AjPStr exclude);
AjBool embBtreeWriteFileList(const AjPStr *filelist, ajint nfiles,
			     const AjPStr fdirectory, const AjPStr idirectory,
			     const AjPStr dbname);

#endif

#ifdef __cplusplus
}
#endif

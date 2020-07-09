#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embdbi_h
#define embdbi_h

typedef struct EmbSac {
  char* ac;
  char* entry;
  ajint nid;			/* entry number */
} EmbOac, *EmbPac;

typedef struct EmbSentry {
  ajint nac;
  ajint rpos;
  ajint spos;
  ajint filenum;
  char* entry;
  char** ac;
} EmbOentry, *EmbPentry;

EmbPac embDbiAcnumNew (void);
ajint embDbiCmpId (const void* a, const void* b);
ajint embDbiCmpAcId (const void* a, const void* b);
ajint embDbiCmpAcAc (const void* a, const void* b);
EmbPentry embDbiEntryNew (void);
AjPList embDbiFileList (AjPStr dir, AjPStr wildfile, AjPStr exclude);
AjBool embDbiFlatOpenlib(AjPStr lname, AjPFile* libr);
void embDbiRmFile (AjPStr dbname, const char* ext, ajint nfiles,
		   AjBool cleanup);
void embDbiRmFileI (AjPStr dbname, const char* ext, ajint ifile,
		    AjBool cleanup);
void embDbiSortFile (AjPStr dbname, const char* ext1, const char* ext2,
		     ajint nfiles, AjBool cleanup, AjPStr sortopt);
void embDbiSysCmd (AjPStr cmdstr);
#endif

#ifdef __cplusplus
}
#endif

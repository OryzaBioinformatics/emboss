#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embindex_h
#define embindex_h

#define BTREE_DEF_IDLEN     15
#define BTREE_DEF_ACLEN     15
#define BTREE_DEF_SVLEN     15
#define BTREE_DEF_KWLEN     15
#define BTREE_DEF_DELEN     15
#define BTREE_DEF_TXLEN     15
#define BTREE_DEF_KWLIMIT   15
#define BTREE_DEF_CACHESIZE 100
#define BTREE_DEF_PAGESIZE  2048

typedef struct EmbSBtreeEntry
{
    AjBool do_id;
    AjBool do_accession;
    AjBool do_sv;
    AjBool do_description;
    AjBool do_keyword;
    AjBool do_taxonomy;
    AjPStr dbname;
    AjPStr dbrs;
    AjPStr release;
    AjPStr date;
    AjPStr dbtype;

    AjPStr directory;
    AjPStr idirectory;

    AjPList files;
    AjPList reffiles;
    ajint   nfiles;
    ajint cachesize;
    ajint pagesize;

    ajint idlen;
    ajint aclen;
    ajint svlen;
    ajint kwlen;
    ajint delen;
    ajint txlen;

    ajint idorder;
    ajint idfill;
    ajint acorder;
    ajint acfill;
    ajint svorder;
    ajint svfill;

    ajint kworder;
    ajint kwfill;
    ajint kwsecorder;
    ajint kwsecfill;
    ajint deorder;
    ajint defill;
    ajint desecorder;
    ajint desecfill;
    ajint txorder;
    ajint txfill;
    ajint txsecorder;
    ajint txsecfill;

    AjPBtcache idcache;
    AjPBtcache accache;
    AjPBtcache svcache;
    AjPBtcache kwcache;
    AjPBtcache decache;
    AjPBtcache txcache;

    ajlong fpos;
    ajlong reffpos;
    
    AjPStr id;
    AjPList ac;
    AjPList sv;
    AjPList tx;
    AjPList kw;
    AjPList de;
} EmbOBtreeEntry;
#define EmbPBtreeEntry EmbOBtreeEntry*
 



void   embBtreeEmblAC(const AjPStr acline, AjPList aclist);
void   embBtreeEmblKW(const AjPStr kwline, AjPList kwlist, ajint maxlen);
void   embBtreeEmblDE(const AjPStr deline, AjPList delist, ajint maxlen);
void   embBtreeEmblTX(const AjPStr kwline, AjPList kwlist, ajint maxlen);
void   embBtreeGenBankAC(const AjPStr acline, AjPList aclist);
void   embBtreeGenBankKW(const AjPStr kwline, AjPList kwlist, ajint maxlen);
void   embBtreeGenBankDE(const AjPStr kwline, AjPList kwlist, ajint maxlen);
void   embBtreeGenBankTX(const AjPStr kwline, AjPList kwlist, ajint maxlen);

void   embBtreeFastaDE(const AjPStr kwline, AjPList kwlist, ajint maxlen);


ajint  embBtreeReadDir(AjPStr **filelist, const AjPStr fdirectory,
		       const AjPStr files, const AjPStr exclude);
EmbPBtreeEntry embBtreeEntryNew(void);
ajint          embBtreeSetFields(EmbPBtreeEntry entry, AjPStr const * fields);
void           embBtreeEntryDel(EmbPBtreeEntry *thys);
void           embBtreeSetDbInfo(EmbPBtreeEntry entry, const AjPStr name,
				 const AjPStr dbrs,
		                 const AjPStr date, const AjPStr release,
		                 const AjPStr type, const AjPStr directory,
		                 const AjPStr idirectory);
ajint          embBtreeGetFiles(EmbPBtreeEntry entry, const AjPStr fdirectory,
				const AjPStr files, const AjPStr exclude);
AjBool         embBtreeWriteEntryFile(const EmbPBtreeEntry entry);
void           embBtreeGetRsInfo(EmbPBtreeEntry entry);
AjBool         embBtreeOpenCaches(EmbPBtreeEntry entry);
AjBool         embBtreeCloseCaches(EmbPBtreeEntry entry);
AjBool         embBtreeDumpParameters(EmbPBtreeEntry entry);


#endif

#ifdef __cplusplus
}
#endif

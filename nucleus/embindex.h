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




/* @data EmbPBtreeEntry *******************************************************
**
** Index tree entries
**
** @alias EmbSBtreeEntry
**
** @attr do_id [AjBool] Undocumented
** @attr do_accession [AjBool] Undocumented
** @attr do_sv [AjBool] Undocumented
** @attr do_description [AjBool] Undocumented
** @attr do_keyword [AjBool] Undocumented
** @attr do_taxonomy [AjBool] Undocumented
** @attr dbname [AjPStr] Undocumented
** @attr dbrs [AjPStr] Undocumented
** @attr release [AjPStr] Undocumented
** @attr date [AjPStr] Undocumented
** @attr dbtype [AjPStr] Undocumented
** @attr directory [AjPStr] Undocumented
** @attr idirectory [AjPStr] Undocumented
** @attr files [AjPList] Undocumented
** @attr reffiles [AjPList] Undocumented
** @attr nfiles [ajint] Undocumented
** @attr cachesize [ajint] Undocumented
** @attr pagesize [ajint] Undocumented
** @attr idlen [ajint] Undocumented
** @attr aclen [ajint] Undocumented
** @attr svlen [ajint] Undocumented
** @attr kwlen [ajint] Undocumented
** @attr delen [ajint] Undocumented
** @attr txlen [ajint] Undocumented
** @attr idorder [ajint] Undocumented
** @attr idfill [ajint] Undocumented
** @attr idsecorder [ajint] Undocumented
** @attr idsecfill [ajint] Undocumented
** @attr acorder [ajint] Undocumented
** @attr acfill [ajint] Undocumented
** @attr acsecorder [ajint] Undocumented
** @attr acsecfill [ajint] Undocumented
** @attr svorder [ajint] Undocumented
** @attr svfill [ajint] Undocumented
** @attr svsecorder [ajint] Undocumented
** @attr svsecfill [ajint] Undocumented
** @attr kworder [ajint] Undocumented
** @attr kwfill [ajint] Undocumented
** @attr kwsecorder [ajint] Undocumented
** @attr kwsecfill [ajint] Undocumented
** @attr deorder [ajint] Undocumented
** @attr defill [ajint] Undocumented
** @attr desecorder [ajint] Undocumented
** @attr desecfill [ajint] Undocumented
** @attr txorder [ajint] Undocumented
** @attr txfill [ajint] Undocumented
** @attr txsecorder [ajint] Undocumented
** @attr txsecfill [ajint] Undocumented
** @attr idcache [AjPBtcache] Undocumented
** @attr accache [AjPBtcache] Undocumented
** @attr svcache [AjPBtcache] Undocumented
** @attr kwcache [AjPBtcache] Undocumented
** @attr decache [AjPBtcache] Undocumented
** @attr txcache [AjPBtcache] Undocumented
** @attr fpos [ajlong] Undocumented
** @attr reffpos [ajlong] Undocumented
** @attr id [AjPStr] Undocumented
** @attr ac [AjPList] Undocumented
** @attr sv [AjPList] Undocumented
** @attr tx [AjPList] Undocumented
** @attr kw [AjPList] Undocumented
** @attr de [AjPList] Undocumented
******************************************************************************/

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
    ajint idsecorder;
    ajint idsecfill;
    ajint acorder;
    ajint acfill;
    ajint acsecorder;
    ajint acsecfill;
    ajint svorder;
    ajint svfill;
    ajint svsecorder;
    ajint svsecfill;

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
 



/*
** Prototype definitions
*/

void   embBtreeEmblAC(const AjPStr acline, AjPList aclist);
void   embBtreeEmblKW(const AjPStr kwline, AjPList kwlist, ajint maxlen);
void   embBtreeEmblDE(const AjPStr deline, AjPList delist, ajint maxlen);
void   embBtreeEmblSV(const AjPStr idline, AjPList svlist);
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

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

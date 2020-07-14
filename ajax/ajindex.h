#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajindex_h
#define ajindex_h


#define BT_PAGESIZE BUFSIZ      /* Default cache page size       */
#define BT_MAXRETRIES	100	/* Maximum number of read/write attempts */

#define BT_LOCK  2
#define BT_DIRTY 1
#define BT_CLEAN 0

#define BT_READ  ajTrue
#define BT_WRITE ajFalse


#define BT_ROOT     1
#define BT_INTERNAL 2
#define BT_LEAF	    4
#define BT_BUCKET   8
#define BT_OVERFLOW 16
#define BT_PRIBUCKET 32
#define BT_SECBUCKET 64

#define BTNO_BALANCE 100L
#define BTNO_NODE    100L








/* @data AjPBtNode ***********************************************************
**
** Btree node
**
** @attr NodeType [ajint] Root, Internal or Leaf
** @attr BlockOffset [ajlong] Offset within mainindex
** @attr Nkeys [ajint] Number of keys filled
** @attr TotLen [ajint] Total length of keys
** @attr Left [ajlong] Left Sibling
** @attr Right [ajlong] Right Sibling
** @attr Overflow [ajlong] Offset to overflow block
** @attr PrevNode [ajlong] Previous node
*****************************************************************************/

typedef struct AjSBtNode
{
    ajint  NodeType;
    ajlong BlockOffset;
    ajint  Nkeys;
    ajint  TotLen;
    ajlong Left;
    ajlong Right;
    ajlong Overflow;
    ajlong PrevNode;
} AjOBtNode;
#define AjPBtNode AjOBtNode*

/* @data AjPBtId ***************************************************
**
** Btree ID
**
** @attr id [AjPStr] Unique ID
** @attr dbno [ajint] Database file number
** @attr dups [ajint] Duplicates
** @attr offset [ajlong] Offset within database file (ftello)
******************************************************************************/

typedef struct AjSBtId
{
    AjPStr id;
    ajint  dbno;
    ajint  dups;
    ajlong offset;
} AjOBtId;
#define AjPBtId AjOBtId*


/* @data AjPBtWild ***************************************************
**
** Btree wildcard
**
** @attr id [AjPStr] Wildcard ID
** @attr pageno [ajlong] Page number of leaf
** @attr first [AjBool] true for first search
** @attr list [AjPList] list of AjPBtIds
******************************************************************************/

typedef struct AjSBtWild
{
    AjPStr id;
    ajlong pageno;
    AjBool first;
    AjPList list;
} AjOBtWild;
#define AjPBtWild AjOBtWild*




/* @data AjPBucket ***************************************************
**
** Bucket structure on disc
**
** Key, filenumber, ftell ID, subkey page (char*, ajint, ajlong, ajlong)
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr keylen [ajint*] key lengths
** @attr Ids [AjPBtId*] Ids
******************************************************************************/

typedef struct AjSBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    AjPBtId *Ids;
} AjOBucket;
#define AjPBucket AjOBucket*




/* Database file name structure
**
** ajint        order			Order of B+tree
** ajint        m			Max entries per bucket
** ajint	NFiles			Number of Indexed files
** ajint        TotalLen                Total length if dir/name entries
** Directory/FileName pairs
*/

#if defined(LENDIAN)
#define BT_GETAJINT(p,v) (memcpy((void*)v,(void*)p,sizeof(ajint)))
#define BT_GETAJLONG(p,v) (memcpy((void*)v,(void*)p,sizeof(ajlong)))
#define BT_SETAJINT(p,v) (memcpy((void*)p,(void*)&v,sizeof(ajint)))
#define BT_SETAJLONG(p,v) (memcpy((void*)p,(void*)&v,sizeof(ajlong)))
#else
#define BT_GETAJINT(p,v) memcpy((void*)v,(void*)p,sizeof(ajint)); \
                         ajUtilRevInt(v)
#define BT_GETAJLONG(p,v) memcpy((void*)v,(void*)p,sizeof(ajlong)); \
                          ajUtilRevLong(v)
#define BT_SETAJINT(p,v)  ajUtilRevInt(&v); \
                          memcpy((void*)p,(void*)&v,sizeof(ajint))
#define BT_SETAJLONG(p,v) ajUtilRevLong(&v); \
                          memcpy((void*)p,(void*)&v,sizeof(ajlong))
#endif


#define BT_BUCKIDLEN(str) (ajStrLen(str) + 1 + sizeof(ajint) + \
			   sizeof(ajint) + sizeof(ajlong))


/*
** Macros to determine entry positions within a bucket
*/

#define PBT_BUCKNODETYPE(p) p
#define PBT_BUCKNENTRIES(p) (p + sizeof(ajint))
#define PBT_BUCKOVERFLOW(p) (p + sizeof(ajint) + sizeof(ajint))
#define PBT_BUCKKEYLEN(p) (p + sizeof(ajint) + sizeof(ajint) + sizeof(ajlong))
#define BT_BUCKPRILEN(str) (ajStrLen(str) + 1 + sizeof(ajlong))
#define BT_BUCKSECLEN(str) (ajStrLen(str) +1)

/*
** Macros to return a page entry value within a bucket
*/

#if defined(LENDIAN)
#define GBT_BUCKNODETYPE(p,v) (memcpy((void*)v,(void*)PBT_BUCKNODETYPE(p), \
				      sizeof(ajint)))
#define GBT_BUCKNENTRIES(p,v) (memcpy((void*)v,(void*)PBT_BUCKNENTRIES(p), \
				      sizeof(ajint)))
#define GBT_BUCKOVERFLOW(p,v) (memcpy((void*)v,(void*)PBT_BUCKOVERFLOW(p), \
				      sizeof(ajlong)))
#else
#define GBT_BUCKNODETYPE(p,v) memcpy((void*)v,(void*)PBT_BUCKNODETYPE(p), \
				      sizeof(ajint)); \
                              ajUtilRevInt(v)
#define GBT_BUCKNENTRIES(p,v) memcpy((void*)v,(void*)PBT_BUCKNENTRIES(p), \
                                      sizeof(ajint)); \
                              ajUtilRevInt(v)
#define GBT_BUCKOVERFLOW(p,v) memcpy((void*)v,(void*)PBT_BUCKOVERFLOW(p), \
				      sizeof(ajlong)); \
                              ajUtilRevLong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/

#if defined(LENDIAN)
#define SBT_BUCKNODETYPE(p,v) (memcpy((void*)PBT_BUCKNODETYPE(p), \
				      (const void*)&v,sizeof(ajint)))
#define SBT_BUCKNENTRIES(p,v) (memcpy((void*)PBT_BUCKNENTRIES(p), \
				      (const void*)&v,sizeof(ajint)))
#define SBT_BUCKOVERFLOW(p,v) (memcpy((void*)PBT_BUCKOVERFLOW(p), \
				      (const void*)&v,sizeof(ajlong)))
#else
#define SBT_BUCKNODETYPE(p,v) ajUtilRevInt(&v); \
                              memcpy((void*)PBT_BUCKNODETYPE(p), \
				      (const void*)&v,sizeof(ajint))
#define SBT_BUCKNENTRIES(p,v) ajUtilRevInt(&v); \
                              memcpy((void*)PBT_BUCKNENTRIES(p), \
				     (const void*)&v,sizeof(ajint))
#define SBT_BUCKOVERFLOW(p,v) ajUtilRevLong(&v); \
                              memcpy((void*)PBT_BUCKOVERFLOW(p), \
				     (const void*)&v,sizeof(ajlong))
#endif

/*
** Macros to determine entry positions within an internal/leaf node
*/
#define PBT_NODETYPE(p) p
#define PBT_BLOCKNUMBER(p) (p + sizeof(ajint))
#define PBT_NKEYS(p) (p + sizeof(ajint) + sizeof(ajlong))
#define PBT_TOTLEN(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint))
#define PBT_LEFT(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		     +sizeof(ajint))
#define PBT_RIGHT(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		      +sizeof(ajint)+sizeof(ajlong))
#define PBT_OVERFLOW(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		         +sizeof(ajint)+sizeof(ajlong)+sizeof(ajlong))
#define PBT_PREV(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		     +sizeof(ajint)+sizeof(ajlong)+sizeof(ajlong) \
		     +sizeof(ajlong))
#define PBT_KEYLEN(p) (p+sizeof(ajint)+sizeof(ajlong)+sizeof(ajint) \
		       +sizeof(ajint)+sizeof(ajlong)+sizeof(ajlong) \
		       +sizeof(ajlong)+sizeof(ajlong))

/*
** Macros to return a page entry value within an internal/leaf node
*/
#if defined(LENDIAN)
#define GBT_NODETYPE(p,v) (memcpy((void*)v,(void*)PBT_NODETYPE(p), \
				  sizeof(ajint)))
#define GBT_BLOCKNUMBER(p,v) (memcpy((void*)v,(void*)PBT_BLOCKNUMBER(p), \
				     sizeof(ajlong)))
#define GBT_NKEYS(p,v) (memcpy((void*)v,(void*)PBT_NKEYS(p), \
			       sizeof(ajint)))
#define GBT_TOTLEN(p,v) (memcpy((void*)v,(void*)PBT_TOTLEN(p), \
			       sizeof(ajint)))
#define GBT_LEFT(p,v) (memcpy((void*)v,(void*)PBT_LEFT(p), \
			      sizeof(ajlong)))
#define GBT_RIGHT(p,v) (memcpy((void*)v,(void*)PBT_RIGHT(p), \
			       sizeof(ajlong)))
#define GBT_PREV(p,v) (memcpy((void*)v,(void*)PBT_PREV(p), \
			      sizeof(ajlong)))
#define GBT_OVERFLOW(p,v) (memcpy((void*)v,(void*)PBT_OVERFLOW(p), \
				  sizeof(ajlong)))
#else
#define GBT_NODETYPE(p,v) memcpy((void*)v,(void*)PBT_NODETYPE(p), \
				 sizeof(ajint)); \
                          ajUtilRevInt(v)
#define GBT_BLOCKNUMBER(p,v) memcpy((void*)v,(void*)PBT_BLOCKNUMBER(p), \
				     sizeof(ajlong)); \
                             ajUtilRevLong(v)
#define GBT_NKEYS(p,v) memcpy((void*)v,(void*)PBT_NKEYS(p), \
			       sizeof(ajint)); \
                       ajUtilRevInt(v)
#define GBT_TOTLEN(p,v) memcpy((void*)v,(void*)PBT_TOTLEN(p), \
			       sizeof(ajint)); \
                        ajUtilRevInt(v)
#define GBT_LEFT(p,v) memcpy((void*)v,(void*)PBT_LEFT(p), \
                             sizeof(ajlong)); \
                      ajUtilRevLong(v)

#define GBT_RIGHT(p,v) memcpy((void*)v,(void*)PBT_RIGHT(p), \
			      sizeof(ajlong)); \
                       ajUtilRevLong(v)
#define GBT_PREV(p,v) memcpy((void*)v,(void*)PBT_PREV(p), \
			      sizeof(ajlong)); \
                      ajUtilRevLong(v)
#define GBT_OVERFLOW(p,v) memcpy((void*)v,(void*)PBT_OVERFLOW(p), \
				  sizeof(ajlong)); \
                          ajUtilRevLong(v)
#endif


/*
** Macros to set a page entry value within an internal/leaf node
*/
#if defined(LENDIAN)
#define SBT_NODETYPE(p,v) (memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
				  sizeof(ajint)))
#define SBT_BLOCKNUMBER(p,v) (memcpy((void*)PBT_BLOCKNUMBER(p), \
				     (const void*)&v,sizeof(ajlong)))
#define SBT_NKEYS(p,v) (memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
			       sizeof(ajint)))
#define SBT_TOTLEN(p,v) (memcpy((void*)PBT_TOTLEN(p),(const void*)&v, \
				sizeof(ajint)))
#define SBT_LEFT(p,v) (memcpy((void*)PBT_LEFT(p), \
			      (const void*)&v,sizeof(ajlong)))
#define SBT_RIGHT(p,v) (memcpy((void*)PBT_RIGHT(p), \
			       (const void*)&v,sizeof(ajlong)))
#define SBT_PREV(p,v) (memcpy((void*)PBT_PREV(p), \
			      (const void*)&v,sizeof(ajlong)))
#define SBT_OVERFLOW(p,v) (memcpy((void*)PBT_OVERFLOW(p), \
				  (const void*)&v,sizeof(ajlong)))
#else
#define SBT_NODETYPE(p,v) ajUtilRevInt(&v); \
                          memcpy((void*)PBT_NODETYPE(p),(const void*)&v, \
				  sizeof(ajint))
#define SBT_BLOCKNUMBER(p,v) ajUtilRevLong(&v); \
                             memcpy((void*)PBT_BLOCKNUMBER(p), \
				     (const void*)&v,sizeof(ajlong))
#define SBT_NKEYS(p,v) ajUtilRevInt(&v); \
                       memcpy((void*)PBT_NKEYS(p),(const void*)&v, \
			       sizeof(ajint))
#define SBT_TOTLEN(p,v) ajUtilRevInt(&v); \
                        memcpy((void*)PBT_TOTLEN(p),(const void*)&v, \
				sizeof(ajint))
#define SBT_LEFT(p,v) ajUtilRevLong(&v); \
                      memcpy((void*)PBT_LEFT(p), \
			      (const void*)&v,sizeof(ajlong))
#define SBT_RIGHT(p,v) ajUtilRevLong(&v); \
                       memcpy((void*)PBT_RIGHT(p), \
			       (const void*)&v,sizeof(ajlong))
#define SBT_PREV(p,v) ajUtilRevLong(&v); \
                      memcpy((void*)PBT_PREV(p), \
			      (const void*)&v,sizeof(ajlong))
#define SBT_OVERFLOW(p,v) ajUtilRevLong(&v); \
                          memcpy((void*)PBT_OVERFLOW(p), \
				 (const void*)&v,sizeof(ajlong))
#endif


/* @data AjPBtpage ***************************************************
**
** Btree page
**
** @attr pageno [ajlong] Page number
** @attr dirty [ajint] Undocumented
** @attr next [struct AjSBtpage*] Next page
** @attr prev [struct AjSBtpage*] Previous page
** @attr buf [unsigned char*] Buffer
******************************************************************************/

typedef struct AjSBtpage
{
    ajlong pageno;
    ajint  dirty;
    struct AjSBtpage *next;
    struct AjSBtpage *prev;
    unsigned char *buf;
} AjOBtpage;
#define AjPBtpage AjOBtpage*


/* @data AjPBtcache ***************************************************
**
** B tree cache
**
** @attr fp [FILE*] Undocumented
** @attr pagesize [ajint] Undocumented
** @attr totsize [ajlong] Undocumented
** @attr lru [AjPBtpage] Undocumented
** @attr mru [AjPBtpage] Undocumented
** @attr listLength [ajint] Undocumented
** @attr order [ajint] Undocumented
** @attr level [ajint] Undocumented
** @attr cachesize [ajint] Undocumented
** @attr nperbucket [ajint] Undocumented
** @attr replace [AjPStr] Undocumented
** @attr count [ajlong] Undocumented
** @attr deleted [AjBool] Undocumented
** @attr slevel [ajint] Undocumented
** @attr sorder [ajint] Undocumented
** @attr snperbucket [ajint] Undocumented
** @attr secrootblock [ajlong] secondary tree root block
******************************************************************************/

typedef struct AjSBtCache
{
    FILE *fp;
    ajint pagesize;
    ajlong totsize;
    AjPBtpage lru;
    AjPBtpage mru;
    ajint listLength;
    ajint order;
    ajint level;
    ajint cachesize;
    ajint nperbucket;
    AjPStr replace;
    ajlong count;
    AjBool deleted;
    ajint slevel;
    ajint sorder;
    ajint snperbucket;
    ajlong secrootblock;
} AjOBtcache;
#define AjPBtcache AjOBtcache*




/* @data AjPBtPri ***************************************************
**
** Btree primary keyword
**
** @attr keyword [AjPStr] Keyword
** @attr treeblock [ajlong] Disc block of secondary tree
** @attr id [AjPStr] Identifier
******************************************************************************/

typedef struct AjSBtPri
{
    AjPStr keyword;
    ajlong treeblock;
    AjPStr id;
} AjOBtPri;
#define AjPBtPri AjOBtPri*




/* @data AjPPriBucket ***************************************************
**
** Keyword primary bucket structure on disc
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr keylen [ajint*] Key lengths
** @attr codes [AjPBtPri*] Codes
******************************************************************************/

typedef struct AjSPriBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    AjPBtPri *codes;
} AjOPriBucket;
#define AjPPriBucket AjOPriBucket*




/* @data AjPSecBucket ***************************************************
**
** Keyword secondary bucket structure on disc
**
** @attr NodeType [ajint] Node type
** @attr Nentries [ajint] Number of entries
** @attr Overflow [ajlong] Offset to overflow block
** @attr keylen [ajint*] key lengths
** @attr ids [AjPStr*] Ids
******************************************************************************/

typedef struct AjSSecBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    AjPStr   *ids;
} AjOSecBucket;
#define AjPSecBucket AjOSecBucket*




AjPBtcache ajBtreeCacheNewC(const char *file, const char *ext,
			    const char *idir, const char *mode,
			    ajint pagesize, ajint order, ajint fill,
			    ajint level, ajint cachesize);
AjPBtpage  ajBtreeCacheRead(AjPBtcache cache, ajlong pageno);
AjPBtpage  ajBtreeCacheWrite(AjPBtcache cache, ajlong pageno);
void       ajBtreeCreateRootNode(AjPBtcache cache, ajlong rootpage);
AjPBtpage  ajBtreeFindInsert(AjPBtcache cache, const char *key);

void     ajBtreeCacheDel(AjPBtcache *thys);
void     ajBtreeInsertId(AjPBtcache cache, const AjPBtId id);
void     ajBtreeIdDel(AjPBtId *thys);
AjPBtId  ajBtreeIdNew(void);
AjPBtId  ajBtreeIdFromKey(AjPBtcache cache, const char *key);
void     ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
			    const char *ext, const char *idir);
void     ajBtreeReadParams(const char *fn, const char *ext,
			   const char *idir, ajint *order,
			   ajint *nperbucket, ajint *pagesize, ajint *level,
			   ajint *cachesize, ajint *sorder,
			   ajint *snperbucket, ajint *count);
void     ajBtreeCacheSync(AjPBtcache cache, ajlong rootpage);

AjBool   ajBtreeDeleteId(AjPBtcache cache, const AjPBtId id);

AjPBtWild  ajBtreeWildNew(AjPBtcache cache, const AjPStr wild);
void       ajBtreeWildDel(AjPBtWild *thys);
AjPBtpage  ajBtreeFindInsertW(AjPBtcache cache, const char *key);
AjPBtId    ajBtreeIdFromKeyW(AjPBtcache cache, AjPBtWild wild);
AjBool     ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid);

AjPStr*    ajBtreeReadEntries(const char *filename, const char *indexdir);
void       ajBtreeInsertDupId(AjPBtcache cache, AjPBtId id);
AjPList    ajBtreeDupFromKey(AjPBtcache cache, const char *key);




AjPBtPri   ajBtreePriNew(void);
void       ajBtreePriDel(AjPBtPri *thys);
AjPBtPri   ajBtreePriFromKeyword(AjPBtcache cache, const char *key);

AjPBtcache ajBtreeSecCacheNewC(const char *file, const char *ext,
			       const char *idir, const char *mode,
			       ajint pagesize, ajint order, ajint fill,
			       ajint level, ajint cachesize,
			       ajint sorder, ajint slevel, ajint sfill,
			       ajint count);
AjPBtpage  ajBtreeSecFindInsert(AjPBtcache cache, const char *key);
void       ajBtreeInsertSecId(AjPBtcache cache, const AjPStr id);
AjBool     ajBtreeSecFromId(AjPBtcache cache, const char *key);

AjPList    ajBtreeSecLeafList(AjPBtcache cache, ajlong rootblock);
AjBool     ajBtreeVerifyId(AjPBtcache cache, ajlong rootblock, const char *id);

void       ajBtreeInsertKeyword(AjPBtcache cache, const AjPBtPri pri);

void btreeLockTest(AjPBtcache cache);

#endif

#ifdef __cplusplus
}
#endif

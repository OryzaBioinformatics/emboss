#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embindex_h
#define embindex_h


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

#define BTNO_BALANCE 100L
#define BTNO_NODE    100L

/* Internal/Leaf node structure
**
** ajint	NodeType		Root, Internal or Leaf
** long long    BlockOffset		Offset within mainindex
** ajint	Nkeys			Number of keys filled
** ajint	TotLen			Total length of keys
** long long	Left			Left Sibling
** long long    Right			Right Sibling
** long long	Overflow		Offset to overflow block
** long long	PrevNode		Previous node
** ajint[nkeys] Key lengths (or keys/pointers in overflow pages)
** Keys/Pointers
** Final pointer
*/


typedef struct EmbSBtNode
{
    ajint  NodeType;
    ajlong BlockOffset;
    ajint  Nkeys;
    ajint  TotLen;
    ajlong Left;
    ajlong Right;
    ajlong Overflow;
    ajlong PrevNode;
} EmbOBtNode, *EmbPBtNode;



typedef struct EmbSBtId
{
    AjPStr id;		/* Unique ID */
    ajint  dbno;	/* Database file number */
    ajint  dups;
    ajlong offset;      /* Offset within database file (ftello) */
} EmbOBtId, *EmbPBtId;



typedef struct EmbSBtWild
{
    AjPStr id;		/* Wildcard ID            */
    ajlong pageno;	/* Page number of leaf    */
    AjBool first;	/* true for first search  */
    AjPList list;	/* list of EmbPBtIds      */
} EmbOBtWild, *EmbPBtWild;




/* Bucket structure on disc
**
** ajint	NodeType
** ajint	Nentries
** long long	Overflow
** ajint[Nentries] key lengths
** Key, filenumber, ftell ID, subkey page (char*, ajint, ajlong, ajlong)
*/
    

typedef struct EmbSBucket
{
    ajint    NodeType;
    ajint    Nentries;
    ajlong   Overflow;
    ajint    *keylen;
    EmbPBtId *Ids;
} EmbOBucket, *EmbPBucket;




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


typedef struct EmbSBtpage
{
    ajlong pageno;
    ajint  dirty;
    struct EmbSBtpage *next;
    struct EmbSBtpage *prev;
    unsigned char *buf;
} EmbOBtpage, *EmbPBtpage;


typedef struct EmbSBtCache
{
    FILE *fp;
    ajint pagesize;
    ajlong totsize;
    EmbPBtpage lru;
    EmbPBtpage mru;
    ajint listLength;
    ajint order;
    ajint level;
    ajint cachesize;
    ajint nperbucket;
    AjPStr replace;
    ajlong count;
    AjBool deleted;
} EmbOBtcache, *EmbPBtcache;




EmbPBtcache embBtreeCacheNewC(const char *file, const char *mode,
			      ajint pagesize, ajint order, ajint fill,
			      ajint level, ajint cachesize);
EmbPBtpage  embBtreeCacheRead(EmbPBtcache cache, ajlong pageno);
EmbPBtpage  embBtreeCacheWrite(EmbPBtcache cache, ajlong pageno);
void        embBtreeCreateRootNode(EmbPBtcache cache);
EmbPBtpage  embBtreeFindInsert(EmbPBtcache cache, char *key);

ajint embBtreeReadDir(AjPStr **filelist, AjPStr fdirectory, AjPStr files,
		      AjPStr exclude);
AjBool embBtreeWriteFileList(AjPStr *filelist, ajint nfiles,
			     AjPStr fdirectory, AjPStr idirectory,
			     AjPStr dbname);
void     embBtreeCacheDel(EmbPBtcache *thys);
void     embBtreeInsertId(EmbPBtcache cache, EmbPBtId id);
void     embBtreeIdDel(EmbPBtId *thys);
EmbPBtId embBtreeIdNew(void);
EmbPBtId embBtreeIdFromKey(EmbPBtcache cache, char *key);
void     embBtreeWriteParams(EmbPBtcache cache, char *fn);
void     embBtreeReadParams(char *fn, ajint *order, ajint *nperbucket,
			    ajint *pagesize, ajint *level, ajint *cachesize);
void     embBtreeCacheSync(EmbPBtcache cache);

AjBool   embBtreeDeleteId(EmbPBtcache cache, EmbPBtId id);
void     embBtreeJoinLeaves(EmbPBtcache cache);

EmbPBtWild embBtreeWildNew(EmbPBtcache cache, AjPStr wild);
void       embBtreeWildDel(EmbPBtWild *thys);
EmbPBtpage embBtreeFindInsertW(EmbPBtcache cache, char *key);
EmbPBtId   embBtreeIdFromKeyW(EmbPBtcache cache, EmbPBtWild wild);
AjBool     embBtreeReplaceId(EmbPBtcache cache, EmbPBtId rid);

AjPStr*    embBtreeReadEntries(char *filename);
void       embBtreeInsertDupId(EmbPBtcache cache, EmbPBtId id);
AjPList    embBtreeDupFromKey(EmbPBtcache cache, char *key);


#endif

#ifdef __cplusplus
}
#endif

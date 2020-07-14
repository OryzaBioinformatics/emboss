/* @source ajindex.c
**
** B+ Tree Indexing plus Disc Cache.
** Copyright (c) 2003 Alan Bleasby
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "ajax.h"




static AjPBtpage  btreeCacheLocate(const AjPBtcache cache, ajlong page);
static AjPBtpage  btreeCacheLruUnlink(AjPBtcache cache);
static void       btreeCacheUnlink(AjPBtcache cache, AjPBtpage cpage);
static void       btreeCacheDestage(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage  btreePageNew(AjPBtcache cache);
static void       btreeCacheFetch(AjPBtcache cache, AjPBtpage cpage,
				  ajlong pageno);
static void       btreeCacheMruAdd(AjPBtcache cache, AjPBtpage cpage);
static AjPBtpage  btreeCacheControl(AjPBtcache cache, ajlong pageno,
				    AjBool isread);
static AjPBtpage  btreeFindINode(AjPBtcache cache, AjPBtpage page,
				 const char *item);


static AjPBtpage  btreePageFromKey(AjPBtcache cache, unsigned char *buf,
				   const char *item);
static ajint      btreeNumInBucket(AjPBtcache cache, ajlong pageno);
static AjPBucket  btreeReadBucket(AjPBtcache cache, ajlong pageno);
static void       btreeWriteBucket(AjPBtcache cache, const AjPBucket bucket,
				   ajlong pageno);
static void       btreeAddToBucket(AjPBtcache cache, ajlong pageno,
				   const AjPBtId id);
static void 	  btreeBucketDel(AjPBucket *thys);
static AjBool     btreeReorderBuckets(AjPBtcache cache, AjPBtpage page);
static void       btreeGetKeys(AjPBtcache cache, unsigned char *buf,
			       AjPStr **keys, ajlong **ptrs);
static ajint      btreeIdCompare(const void *a, const void *b);
static AjPBucket  btreeBucketNew(ajint n);
static void       btreeWriteNode(AjPBtcache cache, AjPBtpage page,
				 AjPStr const *keys, const ajlong *ptrs,
				 ajint nkeys);
static AjBool     btreeNodeIsFull(const AjPBtcache cache, AjPBtpage page);
static void       btreeInsertNonFull(AjPBtcache cache, AjPBtpage page,
				     const AjPStr key, ajlong less,
				     ajlong greater);
static void       btreeSplitRoot(AjPBtcache cache);
static void       btreeInsertKey(AjPBtcache cache, AjPBtpage page,
				 const AjPStr key, ajlong less,
				 ajlong greater);
static ajlong     btreeFindBalance(AjPBtcache cache, ajlong thisNode,
				   ajlong leftNode, ajlong rightNode,
				   ajlong lAnchor, ajlong rAnchor,
				   const AjPBtId id);
static AjBool     btreeRemoveEntry(AjPBtcache cache,ajlong pageno,
				   const AjPBtId id);
static void       btreeAdjustBuckets(AjPBtcache cache, AjPBtpage leaf);
static ajlong     btreeCollapseRoot(AjPBtcache cache, ajlong pageno);
static ajlong     btreeRebalance(AjPBtcache cache, ajlong thisNode,
				 ajlong leftNode, ajlong rightNode,
				 ajlong lAnchor, ajlong rAnchor);
static ajlong     btreeShift(AjPBtcache cache, ajlong thisNode,
			     ajlong balanceNode, ajlong anchorNode);
static ajlong     btreeMerge(AjPBtcache cache, ajlong thisNode,
			     ajlong mergeNode, ajlong anchorNode);

static void       btreeFindMin(AjPBtcache cache, ajlong pageno,
			       const char *key);
static ajlong     btreeInsertShift(AjPBtcache cache, AjPBtpage *retpage,
				   const char *key);
static void       btreeKeyShift(AjPBtcache cache, AjPBtpage tpage);




#if 0
static AjPBtpage  btreeTraverseLeaves(AjPBtcache cache, AjPBtpage thys);
static void       btreeJoinLeaves(AjPBtcache cache);
#endif




static AjPBtpage  btreeFindINodeW(AjPBtcache cache, AjPBtpage page,
				  const char *item);
static AjPBtpage  btreePageFromKeyW(AjPBtcache cache, unsigned char *buf,
				    const char *key);
static void       btreeReadLeaf(AjPBtcache cache, AjPBtpage page,
				AjPList list);
static AjPBtpage  btreeSplitLeaf(AjPBtcache cache, AjPBtpage spage);
static AjPBtpage  btreeSplitPriLeaf(AjPBtcache cache, AjPBtpage spage);




static AjPPriBucket  btreePriBucketNew(ajint n);
static void          btreePriBucketDel(AjPPriBucket *thys);
static AjPPriBucket  btreeReadPriBucket(AjPBtcache cache, ajlong pageno);
static void          btreeWritePriBucket(AjPBtcache cache,
					 const AjPPriBucket bucket,
					 ajlong pageno);
static void          btreeAddToPriBucket(AjPBtcache cache, ajlong pageno,
					 const AjPBtPri pri);
static ajint         btreeNumInPriBucket(AjPBtcache cache, ajlong pageno);
static ajint         btreeKeywordCompare(const void *a, const void *b);
static AjBool        btreeReorderPriBuckets(AjPBtcache cache,
					    AjPBtpage leaf);




static void          btreeWriteSecBucket(AjPBtcache cache,
					 const AjPSecBucket bucket,
					 ajlong pageno);
static AjPSecBucket  btreeReadSecBucket(AjPBtcache cache, ajlong pageno);
static ajint         btreeKeywordIdCompare(const void *a, const void *b);
static AjPBtpage     btreeSplitSecLeaf(AjPBtcache cache, AjPBtpage spage);

static AjPSecBucket  btreeSecBucketNew(ajint n);
static void          btreeSecBucketDel(AjPSecBucket *thys);
static void          btreeAddToSecBucket(AjPBtcache cache, ajlong pageno,
					 const AjPStr id);
static AjBool        btreeReorderSecBuckets(AjPBtcache cache, AjPBtpage leaf);
#if 0
static               void btreeInsertIdOnly(AjPBtcache cache,
					    const AjPBtPri pri);
#endif
static void          btreeSplitRootSec(AjPBtcache cache);
static ajint         btreeNumInSecBucket(AjPBtcache cache, ajlong pageno);
static void          btreeInsertKeySec(AjPBtcache cache, AjPBtpage page,
			               const AjPStr key, ajlong less,
			               ajlong greater);
static ajlong        btreeInsertShiftSec(AjPBtcache cache, AjPBtpage *retpage,
				         const char *key);
static void          btreeKeyShiftSec(AjPBtcache cache, AjPBtpage tpage);
static void          btreeInsertNonFullSec(AjPBtcache cache, AjPBtpage page,
				           const AjPStr key, ajlong less,
				           ajlong greater);




/* @func ajBtreeCacheNewC **************************************************
**
** Open a b+tree index file and initialise a cache object
**
** @param [r] file [const char *] name of file
** @param [r] ext [const char *] extension of file
** @param [r] idir [const char *] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] pagesize [ajint] pagesize
** @param [r] order [ajint] Tree order
** @param [r] fill [ajint] Number of entries per bucket
** @param [r] level [ajint] level of tree
** @param [r] cachesize [ajint] size of cache
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeCacheNewC(const char *file, const char *ext,
			    const char *idir, const char *mode,
			    ajint pagesize, ajint order, ajint fill,
			    ajint level, ajint cachesize)
{
    FILE *fp;
    AjPBtcache cache = NULL;
#if defined (HAVE64) && !defined(_OSF_SOURCE) && !defined(_AIX) && !defined(__hpux) && !defined(__ppc__) && !defined(__FreeBSD__)
    struct stat64 buf;
#else
    struct stat buf;
#endif
    ajlong filelen = 0L;

    AjPStr fn = NULL;

    fn = ajStrNew();
    ajFmtPrintS(&fn,"%s/%s.%s",idir,file,ext);
    
    fp = fopen(fn->Ptr,mode);
    if(!fp)
	return NULL;


#if defined (HAVE64) && !defined(_OSF_SOURCE) && !defined(_AIX) && !defined(__hpux) && !defined(__ppc__) && !defined(__FreeBSD__)
    if(!stat64(file, &buf))
#else
    if(!stat(file, &buf))
#endif
	filelen = buf.st_size;

    AJNEW0(cache);

    cache->listLength = 0;

    cache->lru   = NULL;
    cache->mru   = NULL;
    cache->count = 0L;
    cache->fp    = fp;
    
    cache->replace = ajStrNew();

    if(pagesize>0)
	cache->pagesize = pagesize;
    else
	cache->pagesize = BT_PAGESIZE;

    cache->totsize   = 0L;
    cache->level     = level;

    cache->order      = order;
    cache->nperbucket = fill;
    cache->totsize    = filelen;
    cache->cachesize  = cachesize;

    /* Add slevel, sorder and snperbucket ???? */

    ajStrDel(&fn);
    
    return cache;
}




/* @funcstatic btreePageNew ***********************************************
**
** Construct a cache page object
**
** @param [u] cache [AjPBtcache] cache
**
** @return [AjPBtpage] initialised disc block cache structure
** @@
******************************************************************************/

static AjPBtpage btreePageNew(AjPBtcache cache)
{
    AjPBtpage thys = NULL;

    /* ajDebug("In btreePageNew\n"); */

    AJNEW0(thys);
    AJCNEW0(thys->buf,cache->pagesize);
    thys->next = NULL;
    thys->prev = NULL;
    
    ++cache->listLength;

    
    return thys;
}




/* @funcstatic btreeBucketNew *********************************************
**
** Construct a bucket object
**
** @param [r] n [ajint] Number of IDs
**
** @return [AjPBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPBucket btreeBucketNew(ajint n)
{
    AjPBucket bucket = NULL;
    ajint i;

    /* ajDebug("In btreeBucketNew\n"); */
    
    AJNEW0(bucket);

    if(n)
    {
	AJCNEW0(bucket->Ids,n);
	AJCNEW0(bucket->keylen,n);
    }
    
    for(i=0;i<n;++i)
	bucket->Ids[i] = ajBtreeIdNew();

    bucket->NodeType = BT_BUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeCacheLocate *******************************************
**
** Search for a page in the cache
**
** @param [r] cache [const AjPBtcache] cache structure
** @param [r] page [ajlong] page number to locate
**
** @return [AjPBtpage]	pointer to page or NULL if not found
** @@
******************************************************************************/

static AjPBtpage btreeCacheLocate(const AjPBtcache cache, ajlong page)
{
    AjPBtpage cpage = NULL;

    /* ajDebug("In btreeCacheLocate\n");*/
    
    for(cpage = cache->mru; cpage; cpage = cpage->prev)
	if(cpage->pageno == page)
	    return cpage;

    return NULL;
}




/* @funcstatic btreeCacheUnlink *******************************************
**
** Remove links to a cache page and return the address of the page
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheUnlink(AjPBtcache cache, AjPBtpage cpage)
{
    /* ajDebug("In btreeCacheUnlink\n"); */

    if(cache->mru == cpage)
    {
	cache->mru = cpage->prev;
	if(cpage->prev)
	    cpage->prev->next = NULL;
	if(cache->lru == cpage)
	    cache->lru = NULL;
    }
    else if(cache->lru == cpage)
    {
	cache->lru = cpage->next;
	if(cpage->next)
	    cpage->next->prev = NULL;
    }
    else
    {
	cpage->prev->next = cpage->next;
	cpage->next->prev = cpage->prev;
    }

    return;
}




/* @funcstatic btreeCacheMruAdd *******************************************
**
** Insert a cache page at the mru position
**
** @param [w] cache [AjPBtcache] cache structure
** @param [u] cpage [AjPBtpage] cache page
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheMruAdd(AjPBtcache cache, AjPBtpage cpage)
{
    /* ajDebug("In btreeCacheMruAdd\n"); */

    cpage->prev = cache->mru;
    cpage->next = NULL;
    if(cache->mru)
	cache->mru->next = cpage;
    if(!cache->lru)
	cache->lru = cpage;
    cache->mru = cpage;

    return;
}




/* @funcstatic btreeCacheLruUnlink ****************************************
**
** Remove links to an LRU cache page
**
** @param [w] cache [AjPBtcache] cache structure
**
** @return [AjPBtpage]	pointer to unlinked cache page
** @@
******************************************************************************/

static AjPBtpage btreeCacheLruUnlink(AjPBtcache cache)
{
    AjPBtpage ret;

    /* ajDebug("In btreeCacheLruUnlink\n"); */

    if(cache->lru->dirty != BT_LOCK)
    {
	if(!cache->lru)
	    ajFatal("btreeCacheLruUnlink: No pages nodes found");

	ret = cache->lru;
	ret->next->prev = NULL;
	cache->lru = ret->next;
	return ret;
    }
    
    for(ret=cache->lru; ret; ret=ret->next)
	if(ret->dirty != BT_LOCK)
	    break;

    if(!ret)
	ajFatal("Too many locked cache pages. Try increasing cachesize");

    btreeCacheUnlink(cache,ret);
    
    return ret;
}




/* @funcstatic btreeCacheDestage *****************************************
**
** Destage a cache page
**
** @param [u] cache [AjPBtcache] cache
** @param [u] cpage [AjPBtpage] cache papge 
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheDestage(AjPBtcache cache, AjPBtpage cpage)
{
    ajint written = 0;
    ajint retries = 0;

    /* ajDebug("In btreeCacheDestage\n");*/

    if(fseek(cache->fp,cpage->pageno,SEEK_SET)==-1)
	fseek(cache->fp,0L,SEEK_END);
    
    while(written != cache->pagesize && retries != BT_MAXRETRIES)
    {
	written += fwrite((void *)cpage->buf,1,cache->pagesize-written,
			  cache->fp);
	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%d) reached in btreeCacheDestage",
		BT_MAXRETRIES);

    cpage->dirty = BT_CLEAN;
    

    return;
}




/* @funcstatic btreeCacheFetch *****************************************
**
** Fetch a cache page from disc
**
** @param [u] cache [AjPBtcache] cache
** @param [w] cpage [AjPBtpage] cache page 
** @param [r] pageno [ajlong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeCacheFetch(AjPBtcache cache, AjPBtpage cpage,
			    ajlong pageno)
{
    ajint sum = 0;
    ajint retries = 0;

    /* ajDebug("In btreeCacheFetch\n"); */

    if(fseek(cache->fp,pageno,SEEK_SET))
	ajFatal("Seek error in ajBtreeCachefetch");
    
    while(sum != cache->pagesize && retries != BT_MAXRETRIES)
    {
	sum += fread((void *)cpage->buf,1,cache->pagesize-sum,
		     cache->fp);
	++retries;
    }
    
    if(retries == BT_MAXRETRIES)
	ajFatal("Maximum retries (%d) reached in btreeCacheFetch",
		BT_MAXRETRIES);

    cpage->pageno = pageno;
    
    return;
}




/* @func ajBtreeCacheDel **************************************************
**
** Close a b+tree cache
**
** @param [w] thys [AjPBtcache*] list of files to read
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeCacheDel(AjPBtcache *thys)
{
    AjPBtcache pthis = *thys;
    AjPBtpage  page  = NULL;
    AjPBtpage  temp  = NULL;

    /* ajDebug("In ajBtreeCacheDel\n"); */
    
    for(page=pthis->lru;page;page=temp)
    {
	temp = page->next;
	AJFREE(page->buf);
	AJFREE(page);
    }

    ajStrDel(&pthis->replace);
    
    fclose(pthis->fp);

    AJFREE(pthis);
    *thys = NULL;
    
    return;
}




/* @funcstatic btreeCacheControl ******************************************
**
** Master control function for cache read/write
**
** @param [w] cache [AjPBtcache] name of file
** @param [r] pageno [ajlong] page number
** @param [r] isread [AjBool] is this a read operation?
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

static AjPBtpage btreeCacheControl(AjPBtcache cache, ajlong pageno,
				    AjBool isread)
{
    AjPBtpage ret      = NULL;
    ajlong lendian     = 0L;
    unsigned char *buf = NULL;    

    /* ajDebug("In btreeCacheControl\n"); */
    
    ret = btreeCacheLocate(cache,pageno);
    
    if(ret)
	btreeCacheUnlink(cache,ret);
    else
    {
	if(cache->listLength == cache->cachesize)
	{
	    ret = btreeCacheLruUnlink(cache);

	    if(ret->dirty == BT_DIRTY)
		btreeCacheDestage(cache,ret);
	    if(isread || pageno!=cache->totsize)
		btreeCacheFetch(cache,ret,pageno);
	}
	else
	{
	    ret = btreePageNew(cache);
	    buf = ret->buf;
	    lendian = pageno;
	    SBT_BLOCKNUMBER(buf,lendian);
	    if(isread || pageno!=cache->totsize)
		btreeCacheFetch(cache,ret,pageno);
	}

	if(!isread)
	    ret->pageno = pageno;
	else
	    ret->dirty = BT_CLEAN;
    }

    btreeCacheMruAdd(cache,ret);

    return ret;
}




/* @func ajBtreeCacheRead ******************************************
**
** Get a pointer to a disc cache page
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheRead(AjPBtcache cache, ajlong pageno)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In ajBtreeCacheRead\n"); */

    ret = btreeCacheControl(cache,pageno,BT_READ);

    return ret;
}




/* @func ajBtreeCacheSync *********************************************
**
** Sync all dirty cache pages
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootpage [ajlong] root page
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeCacheSync(AjPBtcache cache, ajlong rootpage)
{
    AjPBtpage page = NULL;

    /* ajDebug("In ajBtreeCacheSync\n");*/

    for(page=cache->lru;page;page=page->next)
	if(page->dirty == BT_DIRTY || page->dirty == BT_LOCK)
	    btreeCacheDestage(cache,page);

    page = btreeCacheLocate(cache,rootpage);
    page->dirty = BT_LOCK;

    return;
}




/* @func ajBtreeCacheWrite ******************************************
**
** Get a pointer to a disc cache page for writing
**
** @param [w] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [AjPBtpage] disc cache page pointer
** @@
******************************************************************************/

AjPBtpage ajBtreeCacheWrite(AjPBtcache cache, ajlong pageno)
{
    AjPBtpage ret = NULL;

    /* ajDebug("In ajBtreeCacheWrite\n");*/

    ret = btreeCacheControl(cache,pageno,BT_WRITE);

    return ret;
}




/* @func ajBtreeCreateRootNode ***********************************************
**
** Create and write an empty root node. Set it as root, write it to
** disc and then lock the page in the disc cache.
** The root node is at block 0L
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootpage [ajlong] root block
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeCreateRootNode(AjPBtcache cache, ajlong rootpage)
{
    AjPBtpage page = NULL;
    unsigned char *p;
    ajint nodetype;
    ajlong blocknumber;
    ajint nkeys;
    ajint totlen;
    ajlong left;
    ajlong right;
    ajlong overflow;
    ajlong prev;

    /* ajDebug("In ajBtreeCreateRootNode\n"); */

    page = ajBtreeCacheWrite(cache,rootpage);

    page->pageno    = rootpage;
    cache->totsize += cache->pagesize;
    
    p = page->buf;

    nodetype    = BT_ROOT;
    blocknumber = rootpage;
    nkeys       = 0;
    totlen      = 0;
    left        = 0L;
    right       = 0L;
    prev        = 0L;
    overflow    = 0L;

    /* Don't reuse the variables. Endianness may be changed */ 
    SBT_NODETYPE(p,nodetype);
    SBT_BLOCKNUMBER(p,blocknumber);
    SBT_NKEYS(p,nkeys);
    SBT_TOTLEN(p,totlen);
    SBT_LEFT(p,left);
    SBT_RIGHT(p,right);
    SBT_PREV(p,prev);
    SBT_OVERFLOW(p,overflow);

    page->dirty = BT_DIRTY;
    ajBtreeCacheSync(cache,rootpage);
    page->dirty = BT_LOCK;

    return;
}




/* @func ajBtreeFindInsert ***********************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char*] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeFindInsert(AjPBtcache cache, const char *key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    ajDebug("In ajBtreeFindInsert\n");

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,0L);

    if(!root)
	ajFatal("Something has unlocked the PRI root cache page\n");
    
    if(!cache->level)
	return root;
    
    ret = btreeFindINode(cache,root,key);
    
    return ret;
}




/* @funcstatic btreeFindINode *************************************************
**
** Recursive search for insert node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const char*] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeFindINode(AjPBtcache cache, AjPBtpage page,
				 const char *item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;
    ajint status       = 0;
    ajint ival         = 0;

    ajDebug("In btreeFindINode\n");
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);
    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
	pg = btreePageFromKey(cache,buf,item);
	ret->dirty = status;
	ret = btreeFindINode(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreePageFromKey *******************************************
**
** Return next lower index page given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const char *] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreePageFromKey(AjPBtcache cache, unsigned char *buf,
				   const char *key)
{
    unsigned char *rootbuf = NULL;
    ajint nkeys = 0;
    ajint order = 0;
    ajint i;
    
    ajlong blockno = 0L;
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    AjPBtpage page = NULL;
    
    ajDebug("In btreePageFromKey\n");
    
    rootbuf = buf;


    GBT_NKEYS(rootbuf,&nkeys);
    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,rootbuf,&karray,&parray);
    i = 0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    page =  ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @func ajBtreeIdNew *********************************************
**
** Constructor for index bucket ID informationn
**
**
** @return [AjPBtId] Index ID object
** @@
******************************************************************************/

AjPBtId ajBtreeIdNew(void)
{
    AjPBtId Id = NULL;

    /* ajDebug("In ajBtreeIdNew\n"); */

    AJNEW0(Id);
    Id->id = ajStrNew();
    Id->dbno = 0;
    Id->dups = 0;
    Id->offset = 0L;

    return Id;
}




/* @func ajBtreeIdDel *********************************************
**
** Destructor for index bucket ID information
**
** @param [w] thys [AjPBtId*] index ID object
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeIdDel(AjPBtId *thys)
{
    AjPBtId Id = NULL;

    /* ajDebug("In ajBtreeIdDel\n"); */

    if(!thys || !*thys)
	return;
    Id = *thys;
    
    ajStrDel(&Id->id);
    AJFREE(Id);
    *thys = NULL;

    return;
}




/* @funcstatic btreeReadBucket *********************************************
**
** Constructor for index bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [AjPBucket] bucket
** @@
******************************************************************************/

static AjPBucket btreeReadBucket(AjPBtcache cache, ajlong pageno)
{
    AjPBucket bucket    = NULL;
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtId id          = NULL;
    
    unsigned char *idptr = NULL;
    
    ajint  nodetype  = 0;
    ajint  nentries  = 0;
    ajlong overflow  = 0L;
    ajint  dirtysave = 0;
    
    ajint  i;
    ajint  len = 0;
    
    /* ajDebug("In btreeReadBucket\n"); */
    
    if(!pageno)
	ajFatal("BucketRead: cannot read bucket from root page");

    page  = ajBtreeCacheRead(cache,pageno);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("ReadBucket: NodeType mismatch. Not bucket (%d)", nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("ReadBucket: Bucket too full\n");
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    AJNEW0(bucket);
    bucket->NodeType = nodetype;
    bucket->Nentries = nentries;
    bucket->Overflow = overflow;
    
    AJCNEW0(bucket->keylen, nentries+1);
    AJCNEW0(bucket->Ids, nentries+1);

    
    kptr  = PBT_BUCKKEYLEN(buf);
    idptr = kptr + (nentries * sizeof(ajint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJINT(kptr,&len);
	if((idptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    ajDebug("ReadBucket: Overflow\n");
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_BUCKET)
		ajFatal("ReadBucket: NodeType mismatch. Not bucket (%d)",
			nodetype);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    idptr = PBT_BUCKKEYLEN(buf);
	}

	bucket->Ids[i] = ajBtreeIdNew();
	id = bucket->Ids[i];

	/* Fill ID objects */
	ajStrAssC(&id->id,(const char *)idptr);
	idptr += (strlen((const char *)idptr) + 1);
	BT_GETAJINT(idptr,&id->dbno);
	idptr += sizeof(ajint);
	BT_GETAJINT(idptr,&id->dups);
	idptr += sizeof(ajint);	
	BT_GETAJLONG(idptr,&id->offset);
	idptr += sizeof(ajlong);

	kptr += sizeof(ajint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeWriteBucket *******************************************
**
** Write index bucket object to the cache given a disc page number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPBucket] bucket
** @param [r] pageno [ajlong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteBucket(AjPBtcache cache, const AjPBucket bucket,
			     ajlong pageno)
{
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    ajint   v   = 0;
    ajint   i   = 0;
    ajint   len = 0;
    ajlong lv   = 0L;
    AjPBtId id  = NULL;

    ajint  nentries = 0;
    ajlong overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;
    ajlong   pno = 0L;
    
    /* ajDebug("In btreeWriteBucket\n"); */

    if(pageno == cache->totsize)	/* Create a new page */
    {
	pno = pageno;
	page = ajBtreeCacheWrite(cache,pageno);
	page->pageno = cache->totsize;
	cache->totsize += cache->pagesize;
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pageno);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_BUCKET;
    SBT_BUCKNODETYPE(buf,v);

    lbuf = buf;
    page->dirty = BT_LOCK;
    lpage = page;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    /* Write out key lengths */
    keyptr = PBT_BUCKKEYLEN(lbuf);
    for(i=0;i<nentries;++i)
    {
	if((keyptr-lbuf+1)+sizeof(ajint) > cache->pagesize)
	    ajFatal("BucketWrite: Bucket cannot hold more than %d keys",
		    i-1);

	id = bucket->Ids[i];
	/* Need to alter this if bucket ID structure changes */
	len = BT_BUCKIDLEN(id->id);
        v = len;
	BT_SETAJINT(keyptr,v);
	keyptr += sizeof(ajint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;
    for(i=0;i<nentries;++i)
    {
	id = bucket->Ids[i];
	len = BT_BUCKIDLEN(id->id);
	if((lptr-buf+1)+len > cache->pagesize) /* overflow */
	{
    	    ajDebug("WriteBucket: Overflow\n");
	    if(!overflow)		/* No overflow buckets yet */
	    {
		pno = cache->totsize;
                lv = pno;
		SBT_BUCKOVERFLOW(buf,lv);
		page = ajBtreeCacheWrite(cache,pno);
		page->pageno = cache->totsize;
		cache->totsize += cache->pagesize;
		buf = page->buf;
		v = BT_BUCKET;
		SBT_BUCKNODETYPE(buf,v);
		v = 0;
		SBT_BUCKNENTRIES(buf,v);
		lv = 0L;
		SBT_BUCKOVERFLOW(buf,lv);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		buf  = page->buf;
		GBT_BUCKOVERFLOW(buf,&overflow);
	    }

	    page->dirty = BT_DIRTY;
	    lptr = PBT_BUCKKEYLEN(buf);	    
	}
	
	sprintf((char *)lptr,"%s",ajStrStr(id->id));
	lptr += (ajStrLen(id->id) + 1);
        v = id->dbno;
	BT_SETAJINT(lptr,v);
	lptr += sizeof(ajint);
        v = id->dups;
	BT_SETAJINT(lptr,v);
	lptr += sizeof(ajint);
        lv = id->offset;
	BT_SETAJLONG(lptr,lv);
	lptr += sizeof(ajlong);
	

    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);
    
    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeBucketDel *********************************************
**
** Delete a bucket object
**
** @param [w] thys [AjPBucket*] bucket
**
** @return [void] bucket
** @@
******************************************************************************/

static void btreeBucketDel(AjPBucket *thys)
{
    AjPBucket pthis = NULL;
    ajint n;
    ajint i;
    
    /* ajDebug("In btreeBucketDel\n"); */

    if(!thys || !*thys)
	return;

    pthis = *thys;
    n = pthis->Nentries;

    for(i=0;i<n;++i)
	ajBtreeIdDel(&pthis->Ids[i]);
    
    if(n)
    {
	AJFREE(pthis->keylen);
	AJFREE(pthis->Ids);
    }
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @funcstatic btreeAddToBucket *******************************************
**
** Add an ID to a bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number of bucket
** @param [r] id [const AjPBtId] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToBucket(AjPBtcache cache, ajlong pageno,
			     const AjPBtId id)
{
    AjPBucket bucket = NULL;
    AjPBtId   destid = NULL;
    
    ajint nentries;
    
    /* ajDebug("In btreeAddToBucket\n"); */

    bucket   = btreeReadBucket(cache,pageno);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */
    bucket->Ids[nentries] = ajBtreeIdNew();
    destid = bucket->Ids[nentries];

    ajStrAssS(&destid->id,id->id);
    destid->dbno   = id->dbno;
    destid->offset = id->offset;
    destid->dups   = id->dups;
    
    ++bucket->Nentries;

    btreeWriteBucket(cache,bucket,pageno);

    btreeBucketDel(&bucket);
    
    return;
}




/* @funcstatic btreeNumInBucket *******************************************
**
** Return number of entries in a bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [ajint] Number of entries in bucket
** @@
******************************************************************************/

static ajint btreeNumInBucket(AjPBtcache cache, ajlong pageno)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;
    ajint  nodetype    = 0;
    ajint  nentries    = 0;
    
    /* ajDebug("In btreeNumInBucket\n"); */
    
    if(!pageno)
	ajFatal("NumInBucket: Attempt to read bucket from root page\n");

    page  = ajBtreeCacheRead(cache,pageno);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_BUCKET)
	ajFatal("ReadBucket: NodeType mismatch. Not bucket (%d)", nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeReorderBuckets *****************************************
**
** Re-order leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/
static AjBool btreeReorderBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPBucket *buckets  = NULL;
    AjPStr *keys        = NULL;
    ajlong *ptrs        = NULL;
    ajlong *overflows   = NULL;
    
    ajint i = 0;
    ajint j = 0;
    
    ajint order;
    ajint bentries      = 0;
    ajint totalkeys     = 0;
    ajint nperbucket    = 0;
    ajint maxnperbucket = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    ajint keylimit      = 0;
    ajint bucketn       = 0;
    ajint bucketlimit   = 0;
    ajint nodetype      = 0;
    
    AjPList idlist    = NULL;
    ajint   dirtysave = 0;
    AjPBtId bid       = NULL;
    AjPBucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajint   v = 0;
    
    /* ajDebug("In btreeReorderBuckets\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    nperbucket = cache->nperbucket;
    

    /* Read keys/ptrs */
    AJCNEW0(keys,order);
    AJCNEW0(ptrs,order);
    AJCNEW0(overflows,order);
    
    for(i=0;i<order;++i)
	keys[i] = ajStrNew();
    btreeGetKeys(cache,lbuf,&keys,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("BucketReorder: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInBucket(cache,ptrs[i]);
    totalkeys += btreeNumInBucket(cache,ptrs[i]);

    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);
    if(totalkeys % maxnperbucket)
	++bucketn;
    
    if(bucketn > order)
    {
	for(i=0;i<order;++i)
	    ajStrDel(&keys[i]);
	AJFREE(keys);
	AJFREE(ptrs);
	AJFREE(overflows);
	
	leaf->dirty = dirtysave;
	return ajFalse;
    }
    

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->Ids[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->Ids);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeIdCompare);
    AJFREE(buckets);

    cbucket = btreeBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;
    
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;
	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[count];
	    ajStrAssS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;

	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreeIdDel(&bid);
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&keys[i],bid->id);

	totkeylen += ajStrLen(bid->id);

	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,ptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;
    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->Ids[count];
	ajStrAssS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;

	++cbucket->Nentries;
	++count;
	ajBtreeIdDel(&bid);
    }
    
    
    if(!ptrs[i])
	ptrs[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,ptrs[i]);

    cbucket->Nentries = maxnperbucket;
    btreeBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);

    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = BT_DIRTY;
    if(nodetype == BT_ROOT)
	leaf->dirty = BT_LOCK;
    
    for(i=0;i<order;++i)
	ajStrDel(&keys[i]);
    AJFREE(keys);
    AJFREE(ptrs);
    AJFREE(overflows);
    

    btreeBucketDel(&cbucket);
    ajListDel(&idlist);

    return ajTrue;
}




/* @funcstatic btreeNodeIsFull *****************************************
**
** Tests whether a node is full of keys
**
** @param [r] cache [const AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
**
** @return [AjBool] true if full
** @@
******************************************************************************/

static AjBool btreeNodeIsFull(const AjPBtcache cache, AjPBtpage page)
{
    unsigned char *buf = NULL;
    ajint nkeys = 0;

    /* ajDebug("In btreeNodeIsFull\n"); */

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);

    if(nkeys == cache->order - 1)
	return ajTrue;

    return ajFalse;
}




/* @funcstatic btreeInsertNonFull *****************************************
**
** Insert a key into a non-full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajlong] less-than pointer
** @param [r] greater [ajlong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertNonFull(AjPBtcache cache, AjPBtpage page,
			       const AjPStr key, ajlong less, ajlong greater)
{
    unsigned char *buf = NULL;
    AjPStr *karray     = NULL;
    ajlong *parray     = NULL;
    ajint nkeys  = 0;
    ajint order  = 0;
    ajint ipos   = 0;
    ajint i;
    ajint count  = 0;

    ajlong lv = 0L;
    ajint  v  = 0;
    

    AjPBtpage ppage = NULL;
    ajlong pageno   = 0L;

    ajint nodetype = 0;
    
    ajDebug("In btreeInsertNonFull\n");

    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);

    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i = 0;
    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr) >= 0)
	++i;

    ipos = i;

    count = nkeys - ipos;
    

    if(ipos == nkeys)
    {
	ajStrAssS(&karray[ipos],key);
	parray[ipos+1] = greater;
	parray[ipos]   = less;
    }
    else
    {
	parray[nkeys+1] = parray[nkeys];

	for(i=nkeys-1; count>0; --count, --i)
	{
	    ajStrAssS(&karray[i+1],karray[i]);
	    parray[i+1] = parray[i];
	}

	ajStrAssS(&karray[ipos],key);
	parray[ipos] = less;
	parray[ipos+1] = greater;
    }

    ++nkeys;
    v = nkeys;
    SBT_NKEYS(buf,v);

    btreeWriteNode(cache,page,karray,parray,nkeys);
    if(nodetype == BT_ROOT)
	page->dirty = BT_LOCK;

    pageno = page->pageno;
    ppage = ajBtreeCacheRead(cache,less);
    lv = pageno;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;
    ppage = ajBtreeCacheRead(cache,greater);
    lv = pageno;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;
    


    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    if(nodetype != BT_ROOT)
	btreeKeyShift(cache,page);

    return;
}




/* @funcstatic btreeInsertKey *****************************************
**
** Insert a key into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajlong] less-than pointer
** @param [r] greater [ajlong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertKey(AjPBtcache cache, AjPBtpage page,
			   const AjPStr key, ajlong less, ajlong greater)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;
    AjPStr *karray      = NULL;
    ajlong *parray      = NULL;
    AjPStr *tkarray     = NULL;
    ajlong *tparray     = NULL;
    ajint nkeys    = 0;
    ajint order    = 0;
    ajint keypos   = 0;
    ajint rkeyno   = 0;
    
    ajint i = 0;
    ajint n = 0;
    
    ajint nodetype  = 0;
    AjPBtpage ipage = NULL;
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;

    ajlong blockno  = 0L;
    ajlong rblockno = 0L;
    ajlong lblockno = 0L;
    AjPStr mediankey  = NULL;
    ajlong medianless = 0L;
    ajlong mediangtr  = 0L;
    ajlong overflow   = 0L;
    ajlong prev       = 0L;
    ajint  totlen     = 0;
    
    ajlong lv = 0L;
    ajint  v  = 0;
    
    /* ajDebug("In btreeInsertKey\n"); */

    if(!btreeNodeIsFull(cache,page))
    {
	btreeInsertNonFull(cache,page,key,less,greater);
	return;
    }
    
    order = cache->order;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);

    if(nodetype == BT_ROOT)
    {
	AJCNEW0(karray,order);
	AJCNEW0(parray,order);
	for(i=0;i<order;++i)
	    karray[i] = ajStrNew();
	btreeSplitRoot(cache);

	btreeGetKeys(cache,lbuf,&karray,&parray);

	if(strcmp(key->Ptr,karray[0]->Ptr)<0)
	    blockno = parray[0];
	else
	    blockno = parray[1];
	ipage = ajBtreeCacheRead(cache,blockno);
	btreeInsertNonFull(cache,ipage,key,less,greater);

	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	return;
    }


    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    AJCNEW0(tkarray,order);
    AJCNEW0(tparray,order);
    mediankey = ajStrNew();
    
    for(i=0;i<order;++i)
    {
	tkarray[i] = ajStrNew();
	karray[i]  = ajStrNew();
    }
    
    lpage = page;
    lbuf = lpage->buf;
    
    btreeGetKeys(cache,lbuf,&karray,&parray);

    GBT_BLOCKNUMBER(lbuf,&lblockno);
    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;
    rbuf = rpage->buf;
    lv = rblockno;
    SBT_BLOCKNUMBER(rbuf,lv);

    
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);

    nkeys = order - 1;
    keypos = nkeys / 2;
    if(!(nkeys % 2))
	--keypos;

    ajStrAssS(&mediankey,karray[keypos]);
    medianless = lblockno;
    mediangtr  = rblockno;


    GBT_NODETYPE(lbuf,&nodetype);
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);


    totlen = 0;
    for(i=0;i<keypos;++i)
    {
	ajStrAssS(&tkarray[i],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i] = parray[i];
    }
    tparray[i] = parray[i];
    v = totlen;
    SBT_TOTLEN(lbuf,v);
    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNode(cache,lpage,tkarray,tparray,i);



    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    totlen = 0;
    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }
    tparray[i-(keypos+1)] = parray[i];
    v = totlen;
    SBT_TOTLEN(rbuf,v);
    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);


    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ipage = rpage;
    if(strcmp(key->Ptr,mediankey->Ptr)<0)
	ipage = lpage;

    btreeInsertNonFull(cache,ipage,key,less,greater);


    for(i=0;i<order;++i)
    {
	ajStrDel(&karray[i]);
	ajStrDel(&tkarray[i]);
    }
    AJFREE(karray);
    AJFREE(tkarray);
    AJFREE(parray);
    AJFREE(tparray);

    ipage = ajBtreeCacheRead(cache,prev);

    btreeInsertKey(cache,ipage,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeSplitRoot *****************************************
**
** Split the root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreeSplitRoot(AjPBtcache cache)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage rpage    = NULL;
    AjPBtpage lpage    = NULL;
    AjPBtpage tpage    = NULL;

    AjPStr *karray     = NULL;
    AjPStr *tkarray    = NULL;
    ajlong *parray     = NULL;
    ajlong *tparray    = NULL;

    ajint order     = 0;
    ajint nkeys     = 0;
    ajint keypos    = 0;
    
    ajlong rblockno = 0L;
    ajlong lblockno = 0L;
    
    AjPStr key = NULL;
    ajint  i;

    unsigned char *rootbuf = NULL;
    unsigned char *rbuf    = NULL;
    unsigned char *lbuf    = NULL;
    unsigned char *tbuf    = NULL;
    
    ajint nodetype  = 0;
    ajlong overflow = 0L;
    ajlong zero     = 0L;
    ajint totlen    = 0;
    ajint rkeyno    = 0;
    ajint n         = 0;

    ajlong lv = 0L;
    ajint  v  = 0;
    
    
    /* ajDebug("In btreeSplitRoot\n"); */

    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    AJCNEW0(tkarray,order);
    AJCNEW0(tparray,order);
    key = ajStrNew();

    for(i=0;i<order;++i)
    {
	karray[i]  = ajStrNew();
	tkarray[i] = ajStrNew();
    }
    

    rootpage = btreeCacheLocate(cache,0L);
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;
    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;

    lblockno = cache->totsize;
    lpage = ajBtreeCacheWrite(cache,lblockno);
    lpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;

    lv = rblockno;
    SBT_BLOCKNUMBER(rpage->buf,lv);
    lv = lblockno;
    SBT_BLOCKNUMBER(lpage->buf,lv);

    if(!cache->level)
    {
	lv = zero;
	SBT_LEFT(lpage->buf,lv);
	lv = rblockno;
	SBT_RIGHT(lpage->buf,lv);
	lv = lblockno;
	SBT_LEFT(rpage->buf,lv);
	lv = zero;
	SBT_RIGHT(rpage->buf,lv);
    }

    btreeGetKeys(cache,rootbuf,&karray,&parray);

    /* Get key for root node and write new root node */
    ajStrAssS(&tkarray[0],karray[keypos]);
    tparray[0] = lblockno;
    tparray[1] = rblockno;
    

    n = 1;
    v = n;
    SBT_NKEYS(rootbuf,v);
    btreeWriteNode(cache,rootpage,tkarray,tparray,1);
    rootpage->dirty = BT_LOCK;

    rbuf = rpage->buf;
    lbuf = lpage->buf;
    
    if(cache->level)
	nodetype = BT_INTERNAL;
    else
	nodetype = BT_LEAF;

    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    lv = overflow;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(lbuf,lv);
    lv = overflow;
    SBT_PREV(lbuf,lv);

    totlen = 0;
    for(i=0;i<keypos;++i)
    {
	ajStrAssS(&tkarray[i],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i] = parray[i];
    }
    tparray[i] = parray[i];
    v = totlen;
    SBT_TOTLEN(lbuf,v);
    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNode(cache,lpage,tkarray,tparray,i);

    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }

    totlen = 0;
    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }
    tparray[i-(keypos+1)] = parray[i];
    v = totlen;
    SBT_TOTLEN(rbuf,v);
    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    for(i=0;i<order;++i)
    {
	ajStrDel(&karray[i]);
	ajStrDel(&tkarray[i]);
    }

    ++cache->level;

    AJFREE(tkarray);
    AJFREE(tparray);
    AJFREE(karray);
    AJFREE(parray);
    ajStrDel(&key);
    
    return;
}




/* @funcstatic btreeGetKeys *********************************************
**
** Get Keys and Pointers from an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [w] keys [AjPStr **] keys
** @param [w] ptrs [ajlong**] ptrs
**
** @return [void]
** @@
******************************************************************************/

static void btreeGetKeys(AjPBtcache cache, unsigned char *buf,
			 AjPStr **keys, ajlong **ptrs)
{
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint  m;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    unsigned char *tbuf   = NULL;

    ajint    i;
    ajint    ival = 0;
    
    ajint    len;
    ajint    pagesize = 0;
    ajlong   overflow = 0L;

    AjPBtpage page = NULL;

    ajDebug("In btreeGetKeys\n");

    karray = *keys;
    parray = *ptrs;
    
    tbuf    = buf;

    pagesize = cache->pagesize;
    
    GBT_NKEYS(tbuf,&m);
    if(!m)
	ajFatal("GetKeys: No keys in node");

    lenptr =  PBT_KEYLEN(tbuf);
    keyptr = lenptr + m * sizeof(ajint);


    for(i=0; i<m; ++i)
    {
	BT_GETAJINT(lenptr,&ival);
	len = ival+1;
	
	if((keyptr-tbuf+1) + len + sizeof(ajlong) > pagesize)
	{
    	    ajDebug("GetKeys: Overflow\n");
	    GBT_OVERFLOW(tbuf,&overflow);
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	    GBT_NODETYPE(tbuf,&ival);
	    if(ival != BT_OVERFLOW)
		ajFatal("Overflow node expected but not found");
	    /*
	     ** The length pointer is restricted to the initial page.
	     ** The keyptr in overflow pages starts at the Key Lengths
	     ** position!
	     */
	    keyptr = PBT_KEYLEN(tbuf);
	}

	ajStrAssC(&karray[i],(const char *)keyptr);
	keyptr += len;

	BT_GETAJLONG(keyptr,&parray[i]);
	keyptr += sizeof(ajlong);
	lenptr += sizeof(ajint);
    }
    

    if((keyptr-tbuf+1) + sizeof(ajlong) > pagesize)
    {
	ajDebug("GetKeys: Overflow\n");
	GBT_OVERFLOW(tbuf,&overflow);
	page = ajBtreeCacheRead(cache,overflow);
	tbuf = page->buf;
	GBT_NODETYPE(tbuf,&ival);
	if(ival != BT_OVERFLOW)
	    ajFatal("Overflow node expected but not found");
	/*
	 ** The length pointer is restricted to the initial page.
	 ** The keyptr in overflow pages starts at the Key Lengths
	 ** position!
	 */
	keyptr = PBT_KEYLEN(tbuf);
    }
    
    BT_GETAJLONG(keyptr,&parray[i]);

    return;
}




/* @funcstatic btreeIdCompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeIdCompare(const void *a, const void *b)
{
    return strcmp((*(AjPBtId*)a)->id->Ptr,
		  (*(AjPBtId*)b)->id->Ptr);
}




/* @funcstatic btreeWriteNode *******************************************
**
** Write an internal node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] buffer
** @param [r] keys [AjPStr const *] keys
** @param [r] ptrs [const ajlong*] page pointers
** @param [r] nkeys [ajint] number of keys

**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteNode(AjPBtcache cache, AjPBtpage spage,
			   AjPStr const *keys, const ajlong *ptrs, ajint nkeys)
{
    unsigned char *lbuf   = NULL;
    unsigned char *tbuf   = NULL;
    unsigned char *lenptr = NULL;
    unsigned char *keyptr = NULL;
    AjPBtpage page        = NULL;
    
    ajlong   overflow = 0L;
    ajlong   blockno  = 0L;

    ajint    i;
    ajint    len;
    ajint    v  = 0;
    ajlong   lv = 0L;

    /* ajDebug("In btreeWriteNode\n"); */

    lbuf = spage->buf;
    tbuf = lbuf;
    page = spage;

    v = nkeys;
    SBT_NKEYS(lbuf,v);
    
    lenptr = PBT_KEYLEN(lbuf);
    keyptr = lenptr + nkeys * sizeof(ajint);

    for(i=0;i<nkeys;++i)
    {
	if((lenptr-lbuf+1) > cache->pagesize)
	    ajFatal("WriteNode: Too many key lengths for available pagesize");
	len = ajStrLen(keys[i]);
	v = len;
	BT_SETAJINT(lenptr,v);
	lenptr += sizeof(ajint);
    }


    GBT_OVERFLOW(lbuf,&overflow);

    for(i=0;i<nkeys;++i)
    {
	len = ajStrLen(keys[i]) + 1;
	if((keyptr-tbuf+1) + len + sizeof(ajlong) > cache->pagesize)
	{
	    ajDebug("WriteNode: Overflow\n");
	    if(!overflow)		/* No overflow buckets yet */
	    {
		page->dirty = BT_DIRTY;
		blockno = cache->totsize;
		lv = blockno;
		SBT_OVERFLOW(tbuf,lv);
		page = ajBtreeCacheWrite(cache,blockno);
		page->pageno = cache->totsize;
		cache->totsize += cache->pagesize;
		tbuf = page->buf;
		v = BT_OVERFLOW;
		SBT_NODETYPE(tbuf,v);
		lv = 0L;
		SBT_OVERFLOW(tbuf,lv);
		lv = blockno;
		SBT_BLOCKNUMBER(tbuf,lv);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		tbuf = page->buf;
		GBT_OVERFLOW(tbuf,&overflow);
	    }
	    keyptr = PBT_KEYLEN(tbuf);
	    page->dirty = BT_DIRTY;
	}


	sprintf((char *)keyptr,"%s",ajStrStr(keys[i]));
	keyptr += len;
	lv = ptrs[i];
	BT_SETAJLONG(keyptr,lv);
	keyptr += sizeof(ajlong);
    }
    



    if((keyptr-tbuf+1) + sizeof(ajlong) > cache->pagesize)
    {
	ajDebug("WriteNode: Overflow\n");
	page->dirty = BT_DIRTY;
	if(!overflow)			/* No overflow buckets yet */
	{
	    blockno = cache->totsize;
	    lv = blockno;
	    SBT_OVERFLOW(tbuf,lv);
	    page = ajBtreeCacheWrite(cache,blockno);
	    page->pageno = cache->totsize;
	    cache->totsize += cache->pagesize;
	    tbuf = page->buf;
	    v = BT_OVERFLOW;
	    SBT_NODETYPE(tbuf,v);
	    lv = blockno;
	    SBT_BLOCKNUMBER(tbuf,lv);
	}
	else
	{
	    page = ajBtreeCacheRead(cache,overflow);
	    tbuf = page->buf;
	}
	keyptr = PBT_KEYLEN(tbuf);
    }
    
    page->dirty = BT_DIRTY;

    overflow = 0L;
    SBT_OVERFLOW(tbuf,overflow);

    lv = ptrs[i];
    BT_SETAJLONG(keyptr,lv);

    return;
}




/* @func ajBtreeInsertId *********************************************
**
** Insert an ID structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] id [const AjPBtId] Id object
**
** @return [void] pointer to a page
** @@
******************************************************************************/

void ajBtreeInsertId(AjPBtcache cache, const AjPBtId id)
{
    AjPBtpage spage   = NULL;
    AjPBtpage parent  = NULL;
    AjPStr key        = NULL;
    const char *ckey  = NULL;
    AjPBucket lbucket = NULL;
    AjPBucket rbucket = NULL;
    ajlong lblockno = 0L;
    ajlong rblockno = 0L;
    ajlong blockno  = 0L;
    ajlong shift    = 0L;

    ajint nkeys = 0;
    ajint order = 0;

    ajint nodetype = 0;
 
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint i;
    ajint n;
    
    unsigned char *buf = NULL;

    /* ajDebug("In ajBtreeInsertId\n"); */
    
    key = ajStrNew();
    

    ajStrAssS(&key,id->id);
    if(!ajStrLen(key))
    {
	ajStrDel(&key);
	return;
    }

    ckey = ajStrStr(key);
    spage = ajBtreeFindInsert(cache,ckey);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();
    
    if(!nkeys)
    {
	lbucket  = btreeBucketNew(0);
	rbucket  = btreeBucketNew(0);

	lblockno = cache->totsize;
	btreeWriteBucket(cache,lbucket,lblockno);

	rblockno = cache->totsize;
	btreeWriteBucket(cache,rbucket,rblockno);	

	parray[0] = lblockno;
	parray[1] = rblockno;
	ajStrAssS(karray,key);
	
	btreeWriteNode(cache,spage,karray,parray,1);

	btreeBucketDel(&lbucket);
	btreeBucketDel(&rbucket);

	btreeAddToBucket(cache,rblockno,id);

	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	ajStrDel(&key);
	return;
    }
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;
    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    if(nodetype != BT_ROOT)
	if((shift = btreeInsertShift(cache,&spage,key->Ptr)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInBucket(cache,blockno);

    if(n == cache->nperbucket)
    {
	if(btreeReorderBuckets(cache,spage))
	{
	    GBT_NKEYS(buf,&nkeys);	    
	    btreeGetKeys(cache,buf,&karray,&parray);

	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;

	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];
	}
	else
	{
	    parent = btreeSplitLeaf(cache,spage);
	    spage = ajBtreeFindInsert(cache,ckey);
	    buf = spage->buf;

	    btreeGetKeys(cache,buf,&karray,&parray);

	    GBT_NKEYS(buf,&nkeys);
	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;

	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];

/*
	    spage = ajBtreeCacheRead(cache,blockno);
	    buf = spage->buf;
	    btreeGetKeys(cache,buf,&karray,&parray);
	    GBT_NKEYS(buf,&nkeys);
	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;
	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];
*/
	}
    }


    btreeAddToBucket(cache,blockno,id);

    ++cache->count;

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);
    ajStrDel(&key);

    return;
}




/* @func ajBtreeIdFromKey ************************************************
**
** Get an ID structure from a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char *] key
**
** @return [AjPBtId] pointer to an ID structure or NULL if not found
** @@
******************************************************************************/

AjPBtId ajBtreeIdFromKey(AjPBtcache cache, const char *key)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPBtId   id     = NULL;
    AjPBtId   tid    = NULL;
    
    AjPStr *karray  = NULL;
    ajlong *parray  = NULL;
    
    unsigned char *buf = NULL;

    ajint nentries = 0;
    ajint nkeys    = 0;
    ajint order    = 0;
    
    ajint i;
    
    ajlong blockno = 0L;
    AjBool found   = ajFalse;


    page = ajBtreeFindInsert(cache,key);
    buf = page->buf;
    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,buf,&karray,&parray);

    GBT_NKEYS(buf,&nkeys);

    i=0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    bucket = btreeReadBucket(cache,blockno);
    
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
    {
	if(!strcmp(key,bucket->Ids[i]->id->Ptr))
	{
	    found = ajTrue;
	    break;
	}
    }

    if(found)
    {
	id  = ajBtreeIdNew();
	tid = bucket->Ids[i];
	ajStrAssS(&id->id,tid->id);
	id->dups = tid->dups;
	id->dbno = tid->dbno;
	id->offset = tid->offset;
    }

    btreeBucketDel(&bucket);
    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    if(!found)
	return NULL;

    return id;
}




/* @func ajBtreeWriteParams ************************************************
**
** Write B+ tree parameters to file
**
** @param [r] cache [const AjPBtcache] cache
** @param [r] fn [const char *] file name
** @param [r] ext [const char *] index file extension name
** @param [r] idir [const char *] index file directory
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeWriteParams(const AjPBtcache cache, const char *fn,
			const char *ext, const char *idir)
{
    AjPStr  fname = NULL;
    AjPFile outf  = NULL;

    fname = ajStrNew();
    ajFmtPrintS(&fname,"%s/%s.p%s",idir,fn,ext);

    if(!(outf = ajFileNewOut(fname)))
	ajFatal("Cannot open param file %S\n",fname);

    ajFmtPrintF(outf,"Order     %d\n",cache->order);
    ajFmtPrintF(outf,"Fill      %d\n",cache->nperbucket);
    ajFmtPrintF(outf,"Pagesize  %d\n",cache->pagesize);
    ajFmtPrintF(outf,"Level     %d\n",cache->level);
    ajFmtPrintF(outf,"Cachesize %d\n",cache->cachesize);
    ajFmtPrintF(outf,"Order2    %d\n",cache->sorder);
    ajFmtPrintF(outf,"Fill2     %d\n",cache->snperbucket);
    ajFmtPrintF(outf,"Count     %d\n",cache->count);

    ajFileClose(&outf);
    ajStrDel(&fname);

    return;
}




/* @func ajBtreeReadParams ************************************************
**
** Read B+ tree parameters from file
**
** @param [r] fn [const char *] file
** @param [r] ext [const char *] file extension
** @param [r] idir [const char *] index directory
** @param [w] order [ajint*] tree order
** @param [w] nperbucket [ajint*] bucket fill
** @param [w] pagesize [ajint*] size of pages
** @param [w] level [ajint*] depth of tree (0 = root leaf)
** @param [w] cachesize [ajint*] cachesize
** @param [w] sorder [ajint*] secondary tree order
** @param [w] snperbucket [ajint*] secondary bucket fill
** @param [w] count [ajint*] number of primary keywords in the index
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeReadParams(const char *fn, const char *ext,
		       const char *idir, ajint *order,
		       ajint *nperbucket, ajint *pagesize, ajint *level,
		       ajint *cachesize, ajint *sorder,
		       ajint *snperbucket, ajint *count)
{
    AjPStr fname = NULL;
    AjPStr line  = NULL;
    AjPFile inf  = NULL;
    
    line  = ajStrNew();

    fname = ajStrNew();
    ajFmtPrintS(&fname,"%s/%s.p%s",idir,fn,ext);

    if(!(inf = ajFileNewIn(fname)))
	ajFatal("Cannot open param file %S\n",fname);

    while(ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"Order2"))
	    ajFmtScanS(line,"%*s%d",sorder);
	if(ajStrPrefixC(line,"Fill2"))
	    ajFmtScanS(line,"%*s%d",snperbucket);
	if(ajStrPrefixC(line,"Order"))
	    ajFmtScanS(line,"%*s%d",order);
	if(ajStrPrefixC(line,"Fill"))
	    ajFmtScanS(line,"%*s%d",nperbucket);
	if(ajStrPrefixC(line,"Pagesize"))
	    ajFmtScanS(line,"%*s%d",pagesize);
	if(ajStrPrefixC(line,"Level"))
	    ajFmtScanS(line,"%*s%d",level);
	if(ajStrPrefixC(line,"Cachesize"))
	    ajFmtScanS(line,"%*s%d",cachesize);
	if(ajStrPrefixC(line,"Count"))
	    ajFmtScanS(line,"%*s%d",count);
    }

    ajFileClose(&inf);
    ajStrDel(&fname);
    ajStrDel(&line);
    
    return;
}




/* @funcstatic btreeSplitLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeSplitLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajint nkeys     = 0;
    ajint order     = 0;
    ajint totalkeys = 0;
    ajint bentries  = 0;
    ajint keylimit  = 0;
    ajint nodetype  = 0;

    ajint rootnodetype  = 0;
    
    ajint i;
    ajint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajlong mediangtr  = 0L;
    ajlong medianless = 0L;
    

    AjPBtId bid = NULL;
    AjPBtId cid = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPBucket *buckets = NULL;
    AjPBucket cbucket  = NULL;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint keypos = 0;
    ajint lno    = 0;
    ajint rno    = 0;

    ajint bucketlimit   = 0;
    ajint maxnperbucket = 0;
    ajint nperbucket    = 0;
    ajint bucketn       = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    
    ajlong lblockno = 0L;
    ajlong rblockno = 0L;
    ajlong prev     = 0L;
    ajlong overflow = 0L;

    ajlong zero = 0L;
    ajlong join = 0L;
    
    ajlong lv = 0L;
    ajint  v  = 0;
    
    /* ajDebug("In btreeSplitLeaf\n"); */

    order = cache->order;
    nperbucket = cache->nperbucket;

    mediankey = ajStrNew();
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    if(rootnodetype == BT_ROOT)
    {
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWrite(cache,lblockno);
	lpage->pageno = cache->totsize;
	cache->totsize += cache->pagesize;
	lbuf = lpage->buf;
	lv = prev;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pageno;
	lpage = spage;
    }


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;
    rbuf = rpage->buf;


    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }
    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetKeys(cache,buf,&karray,&parray);


    keylimit = nkeys+1;
    AJCNEW0(buckets,keylimit);
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadBucket(cache,parray[i]);

    idlist = ajListNew();
    for(i=0;i<keylimit;++i)
    {
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->Ids[j]);
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->Ids);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeIdCompare);
    AJFREE(buckets);


    totalkeys = ajListLength(idlist);

    keypos = totalkeys / 2;

    lno = keypos;
    rno = totalkeys - lno;

    maxnperbucket = nperbucket >> 1;
    if(!maxnperbucket)
	++maxnperbucket;

    cbucket = btreeBucketNew(maxnperbucket);

    bucketn = lno / maxnperbucket;
    if(lno % maxnperbucket)
	++bucketn;
    bucketlimit = bucketn - 1;


    totkeylen = 0;
    count = 0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<maxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[j];
	    ajStrAssS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;

	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id);
	    ++count;
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}
	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssS(&karray[i],bid->id);
	totkeylen += ajStrLen(bid->id);

	if(!parray[i])
	    parray[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,parray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;
    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);
	cid = cbucket->Ids[j];
	++j;
	++count;

	ajStrAssS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }

    if(!parray[i])
	parray[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,parray[i]);

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lpage->dirty = BT_DIRTY;
    btreeWriteNode(cache,lpage,karray,parray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    ajStrAssS(&mediankey,bid->id);

    totkeylen = 0;
    bucketn = rno / maxnperbucket;
    if(rno % maxnperbucket)
	++bucketn;
    bucketlimit = bucketn - 1;

    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<maxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[j];
	    ajStrAssS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;

	    cbucket->keylen[j] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ajBtreeIdDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&karray[i],bid->id);
	totkeylen += ajStrLen(bid->id);

	parray[i] = cache->totsize;
	btreeWriteBucket(cache,cbucket,parray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;
    while(ajListPop(idlist,(void**)&bid))
    {
	cid = cbucket->Ids[j];
	++j;

	ajStrAssS(&cid->id,bid->id);
	cid->dbno = bid->dbno;
	cid->dups = bid->dups;
	cid->offset = bid->offset;
	++cbucket->Nentries;
	ajBtreeIdDel(&bid);
    }
    
    parray[i] = cache->totsize;
    btreeWriteBucket(cache,cbucket,parray[i]);

    nkeys = bucketn - 1;

    v = nkeys;
    SBT_NKEYS(rbuf,v);
    v = totkeylen;
    SBT_TOTLEN(rbuf,v);
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNode(cache,rpage,karray,parray,nkeys);
    rpage->dirty = BT_DIRTY;

    cbucket->Nentries = maxnperbucket;
    btreeBucketDel(&cbucket);
    ajListDel(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;


    if(rootnodetype == BT_ROOT)
    {
	ajStrAssS(&karray[0],mediankey);
	parray[0]=lblockno;
	parray[1]=rblockno;
	nkeys = 1;
	btreeWriteNode(cache,spage,karray,parray,nkeys);	
	spage->dirty = BT_LOCK;
	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	ajStrDel(&mediankey);
	++cache->level;
	return spage;
    }


    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    page = ajBtreeCacheRead(cache,prev);
    btreeInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prev);

    return page;
}




/* @func ajBtreeDeleteId *********************************************
**
** Entry point for ID deletion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] id [const AjPBtId] page
**
** @return [AjBool] True if found and deleted
** @@
******************************************************************************/

AjBool ajBtreeDeleteId(AjPBtcache cache, const AjPBtId id)
{
    AjPBtpage rootpage = NULL;
    unsigned char *buf = NULL;
    ajlong balanceNode = 0L;
    ajlong blockno     = 0L;

    
    /* ajDebug("In ajBtreeDeleteId\n"); */
    
    rootpage = btreeCacheLocate(cache,0L);
    rootpage->dirty = BT_LOCK;
    
    buf = rootpage->buf;
    
    balanceNode = BTNO_BALANCE;

    blockno = btreeFindBalance(cache,0L,BTNO_NODE,BTNO_NODE,BTNO_NODE,
				  BTNO_NODE,id);

    
    if(!cache->deleted)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeFindBalance *******************************************
**
** Master routine for entry deletion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajlong] Current node
** @param [r] leftNode [ajlong] Node to left
** @param [r] rightNode [ajlong] Node to right
** @param [r] lAnchor [ajlong] Left anchor
** @param [r] rAnchor [ajlong] Right anchor
** @param [r] id [const AjPBtId] id
**
** @return [ajlong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajlong btreeFindBalance(AjPBtcache cache, ajlong thisNode,
			       ajlong leftNode, ajlong rightNode,
			       ajlong lAnchor, ajlong rAnchor,
			       const AjPBtId id)
{
    unsigned char *buf  = NULL;
    unsigned char *buf1 = NULL;
    
    ajlong nextNode   = BTNO_NODE;
    ajlong nextLeft   = BTNO_NODE;
    ajlong nextRight  = BTNO_NODE;
    ajlong nextAncL   = BTNO_NODE;
    ajlong nextAncR   = BTNO_NODE;
    ajlong done       = 0L;
    
    ajint  nkeys      = 0;
    ajint  order      = 0;
    ajint  minkeys    = 0;
    ajint  i;
    ajint  nodetype   = 0;

    ajint n1keys      = 0;
    
    AjPBtpage page  = NULL;
    AjPBtpage page1 = NULL;

    ajlong balanceNode = 0L;
    ajlong blockno     = 0L;
    ajlong ptrSave     = 0L;

    AjPStr *karray  = NULL;
    ajlong *parray  = NULL;
    AjPStr *k1array = NULL;
    ajlong *p1array = NULL;

    
    char *key = NULL;
    AjBool existed = ajFalse;
    
    /* ajDebug("In btreeFindBalance\n"); */

    if(thisNode)
	page = ajBtreeCacheRead(cache,thisNode);
    else
    {
	page = btreeCacheLocate(cache,thisNode);
	page->dirty = BT_LOCK;
    }

    cache->deleted = ajFalse;

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    
    order = cache->order;
    minkeys = (order-1) / 2;
    if((order-1)%2)
	++minkeys;

    if(nkeys >= minkeys)
	balanceNode = BTNO_BALANCE;
    else
	balanceNode = page->pageno;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    AJCNEW0(k1array,order);
    AJCNEW0(p1array,order);
    
    for(i=0;i<order;++i)
    {
	k1array[i] = ajStrNew();
	karray[i]  = ajStrNew();
    }

    key = id->id->Ptr;

    btreeGetKeys(cache,buf,&karray,&parray);
    i=0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    nextNode = blockno;
    ptrSave = i;

    GBT_NODETYPE(buf,&nodetype);
    if(!(nodetype == BT_LEAF) && !(nodetype == BT_ROOT && !cache->level))
    {
	if(nextNode == parray[0])
	{
	    if(leftNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,leftNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextLeft = p1array[n1keys];
	    }
	    else
		nextLeft = BTNO_NODE;
	    
	    if(!thisNode)
		nextAncL = thisNode;
	    else
		nextAncL = lAnchor;
	}
	else
	{
	    nextLeft = parray[ptrSave-1];
	    nextAncL = thisNode;
	}

	if(nextNode == parray[nkeys])
	{
	    if(rightNode != BTNO_NODE)
	    {
		page1 = ajBtreeCacheRead(cache,rightNode);
		buf1 = page1->buf;
		GBT_NKEYS(buf1,&n1keys);
		btreeGetKeys(cache,buf1,&k1array,&p1array);
		nextRight = p1array[0];
	    }
	    else
		nextRight = BTNO_NODE;

	    if(!thisNode)
		nextAncR = thisNode;
	    else
		nextAncR = rAnchor;
	}
	else
	{
	    nextRight = parray[ptrSave+1];
	    nextAncR  = thisNode;
	}



	/* Check to see whether key exists in an internal node */
	if(nodetype != BT_LEAF && cache->level)
	{
	    i=0;
	    while(i!=nkeys && strcmp(key,karray[i]->Ptr))
		++i;
	    if(i!=nkeys)
	    {
		btreeFindMin(cache,parray[i+1],key);
		ajStrAssS(&karray[i],cache->replace);
		btreeWriteNode(cache,page,karray,parray,nkeys);
	    }
	
	}
	
	btreeFindBalance(cache,nextNode,nextLeft,nextRight,
			    nextAncL,nextAncR,id);

	if(thisNode)
	    page = ajBtreeCacheRead(cache,thisNode);
	else
	{
	    page = btreeCacheLocate(cache,thisNode);
	    page->dirty = BT_LOCK;
	}
	buf = page->buf;

    }
    else
    {
	if(nodetype == BT_LEAF || (nodetype==BT_ROOT && !cache->level))
	{
	    existed = btreeRemoveEntry(cache,thisNode,id);
	    
	    if(existed)
		cache->deleted = ajTrue;
	    GBT_NKEYS(buf,&nkeys);
	    if(nkeys >= minkeys || (nodetype==BT_ROOT && !cache->level))
		balanceNode = BTNO_BALANCE;
	    else
		balanceNode = page->pageno;
	}
    }


    if(balanceNode == BTNO_BALANCE || thisNode == 0L)
	done = BTNO_NODE;
    else
	done = btreeRebalance(cache,thisNode,leftNode,rightNode,
				 lAnchor,rAnchor);
    

    for(i=0;i<order;++i)
    {
	ajStrDel(&k1array[i]);
	ajStrDel(&karray[i]);
    }
    AJFREE(k1array);
    AJFREE(karray);
    AJFREE(p1array);
    AJFREE(parray);

    return done;
}




/* @funcstatic btreeRemoveEntry *******************************************
**
** Find and delete an ID from a given leaf node if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] leaf node page
** @param [r] id [const AjPBtId] id
**
** @return [AjBool] True if found (and deleted)
** @@
******************************************************************************/

static AjBool btreeRemoveEntry(AjPBtcache cache,ajlong pageno,
			       const AjPBtId id)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    ajlong blockno = 0L;
    
    ajint order    = 0;
    ajint nkeys    = 0;
    ajint nentries = 0;
    ajint i;

    ajint dirtysave = 0;
    
    AjBool found = ajFalse;
    char   *key  = NULL;

    unsigned char *buf = NULL;

    /* ajDebug("In btreeRemoveEntry\n"); */

    page = ajBtreeCacheRead(cache,pageno);
    buf = page->buf;
    dirtysave = page->dirty;
    page->dirty = BT_LOCK;
    order = cache->order;
    
    GBT_NKEYS(buf,&nkeys);
    if(!nkeys)
	return ajFalse;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,buf,&karray,&parray);
    
    key = id->id->Ptr;

    i=0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];
    
    bucket = btreeReadBucket(cache,blockno);


    nentries = bucket->Nentries;
    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!strcmp(key,bucket->Ids[i]->id->Ptr))
	{
	    found = ajTrue;
	    break;
	}
    

    if(found)
    {
	/* Perform the deletion */
	if(nentries == 1)
	{
	    bucket->Nentries = 0;
	    ajBtreeIdDel(&bucket->Ids[0]);
	}
	else
	{
	    ajBtreeIdDel(&bucket->Ids[i]);
	    bucket->Ids[i] = bucket->Ids[nentries-1];
	    --bucket->Nentries;
	}

	btreeWriteBucket(cache,bucket,blockno);
	btreeAdjustBuckets(cache,page);
	page->dirty = BT_DIRTY;
    }
    else
	page->dirty = dirtysave;

    btreeBucketDel(&bucket);

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    
    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @funcstatic btreeAdjustBuckets *****************************************
**
** Re-order leaf buckets
** Can be called whatever the state of a leaf (used by deletion funcs)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [void]
** @@
******************************************************************************/

static void btreeAdjustBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajint nkeys = 0;
    unsigned char *lbuf = NULL;
    AjPBucket *buckets  = NULL;
    AjPStr *keys        = NULL;
    ajlong *ptrs        = NULL;
    ajlong *overflows   = NULL;
    
    ajint i = 0;
    ajint j = 0;
    
    ajint order;
    ajint bentries      = 0;
    ajint totalkeys     = 0;
    ajint nperbucket    = 0;
    ajint maxnperbucket = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    ajint keylimit      = 0;
    ajint bucketn       = 0;
    ajint bucketlimit   = 0;
    ajint nodetype      = 0;
    ajint nids          = 0;
    ajint totnids       = 0;
    
    AjPList idlist    = NULL;
    ajint   dirtysave = 0;
    AjPBtId bid       = NULL;
    AjPBucket cbucket = NULL;
    AjPBtId cid       = NULL;

    ajint v = 0;
    
    /* ajDebug("In btreeAdjustBuckets\n"); */

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    lbuf = leaf->buf;

    GBT_NKEYS(lbuf,&nkeys);
    if(!nkeys)
    {
	leaf->dirty = dirtysave;
	return;
    }


    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    nperbucket = cache->nperbucket;
    

    /* Read keys/ptrs */
    AJCNEW0(keys,order);
    AJCNEW0(ptrs,order);
    AJCNEW0(overflows,order);
    
    for(i=0;i<order;++i)
	keys[i] = ajStrNew();

    btreeGetKeys(cache,lbuf,&keys,&ptrs);


    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInBucket(cache,ptrs[i]);
    totalkeys += btreeNumInBucket(cache,ptrs[i]);


    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;
    if(!maxnperbucket)
	++maxnperbucket;

    if(!leaf->pageno)
	maxnperbucket = nperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);
    if(totalkeys % maxnperbucket)
	++bucketn;

    if(bucketn == 1)
	++bucketn;
    
    
    if(bucketn > order)
	ajFatal("AdjustBuckets: bucket number greater than order");
    

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;
    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing id) */
    idlist  = ajListNew();

    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->Ids[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->Ids);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeIdCompare);
    AJFREE(buckets);

    cbucket = btreeBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;

    totnids = 0;
    nids = ajListLength(idlist);


    if(!totalkeys)
    {
	v = totalkeys;
	SBT_NKEYS(lbuf,v);
	for(i=0;i<order;++i)
	    ajStrDel(&keys[i]);
	AJFREE(keys);
	AJFREE(ptrs);
	AJFREE(overflows);
	ajListDel(&idlist);
	leaf->dirty = BT_DIRTY;
	return;
    }
    
    if(nids <= maxnperbucket)
    {
	cbucket->Overflow = overflows[1];
	cbucket->Nentries = 0;
	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&keys[0],bid->id);

	count = 0;
	while(count!=maxnperbucket && totnids != nids)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->Ids[count];
	    ajStrAssS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;

	    cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
	    ++cbucket->Nentries;
	    ++count;
	    ++totnids;
	    ajBtreeIdDel(&bid);
	}


	totkeylen += ajStrLen(keys[0]);

	if(!ptrs[1])
	    ptrs[1] = cache->totsize;
	btreeWriteBucket(cache,cbucket,ptrs[1]);

	cbucket->Overflow = overflows[0];
	cbucket->Nentries = 0;
	if(!ptrs[0])
	    ptrs[0] = cache->totsize;
	btreeWriteBucket(cache,cbucket,ptrs[0]);
    }
    else
    {
	for(i=0;i<bucketlimit;++i)
	{
	    cbucket->Overflow = overflows[i];
	    cbucket->Nentries = 0;

	    count = 0;
	    while(count!=maxnperbucket && totnids != nids)
	    {
		ajListPop(idlist,(void **)&bid);

		cid = cbucket->Ids[count];
		ajStrAssS(&cid->id,bid->id);
		cid->dbno = bid->dbno;
		cid->dups = bid->dups;
		cid->offset = bid->offset;

		cbucket->keylen[count] = BT_BUCKIDLEN(bid->id);
		++cbucket->Nentries;
		++count;
		ajBtreeIdDel(&bid);
	    }


	    ajListPeek(idlist,(void **)&bid);
	    ajStrAssS(&keys[i],bid->id);


	    totkeylen += ajStrLen(bid->id);

	    if(!ptrs[i])
		ptrs[i] = cache->totsize;
	    btreeWriteBucket(cache,cbucket,ptrs[i]);
	}
	
	
	/* Deal with greater-than bucket */
	
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;
	
	
	
	count = 0;
	while(ajListPop(idlist,(void **)&bid))
	{
	    cid = cbucket->Ids[count];
	    ajStrAssS(&cid->id,bid->id);
	    cid->dbno = bid->dbno;
	    cid->dups = bid->dups;
	    cid->offset = bid->offset;

	    ++cbucket->Nentries;
	    ++count;
	    ajBtreeIdDel(&bid);
	}
	
	
	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	
	btreeWriteBucket(cache,cbucket,ptrs[i]);
    }
    

    cbucket->Nentries = maxnperbucket;
    btreeBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);

    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = dirtysave;
    if(nodetype == BT_ROOT)
	leaf->dirty = BT_LOCK;


    for(i=0;i<order;++i)
	ajStrDel(&keys[i]);
    AJFREE(keys);
    AJFREE(ptrs);
    AJFREE(overflows);
    

    btreeBucketDel(&cbucket);
    ajListDel(&idlist);

    return;
}




/* @funcstatic btreeCollapseRoot *******************************************
**
** Master routine for entry deletion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number to make new root
**
** @return [ajlong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajlong btreeCollapseRoot(AjPBtcache cache, ajlong pageno)
{
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    AjPStr *karray = NULL;
    ajlong *parray = NULL;

    AjPBtpage rootpage = NULL;
    AjPBtpage page     = NULL;
    
    ajint nodetype = 0;
    ajint nkeys    = 0;
    ajint order    = 0;
    ajint i;

    ajlong prev = 0L;
    
    /* ajDebug("In btreeCollapseRoot\n"); */
    
    if(!cache->level)
	return BTNO_NODE;

    rootpage = btreeCacheLocate(cache,0L);
    buf = rootpage->buf;
    page = ajBtreeCacheRead(cache,pageno);


    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    /*
    ** Swap pageno values to make root the child and child the root
    ** Update nodetypes and mark the original root as a clean page
    */

    /* At this point page->pageno could be added to a free list */

    rootpage->pageno = page->pageno;
    rootpage->dirty = BT_CLEAN;
    nodetype = BT_INTERNAL;
    SBT_NODETYPE(buf,nodetype);

    page->pageno = 0;
    page->dirty  = BT_LOCK;
    buf = page->buf;
    nodetype = BT_ROOT;
    SBT_NODETYPE(buf,nodetype);
    
    --cache->level;

    if(cache->level)
    {
	/*
	 ** Update the PREV pointers of the new root's children
	 */
	GBT_NKEYS(buf,&nkeys);
	btreeGetKeys(cache,buf,&karray,&parray);
	for(i=0;i<nkeys+1;++i)
	{
	    page = ajBtreeCacheRead(cache,parray[i]);
	    lbuf = page->buf;
	    SBT_PREV(lbuf,prev);
	    page->dirty = BT_DIRTY;
	}
    }


    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    
    return 0L;
}




/* @funcstatic btreeRebalance *******************************************
**
** Master routine for entry deletion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajlong] Node to rebalance
** @param [r] leftNode [ajlong] left node
** @param [r] rightNode [ajlong] right node
** @param [r] lAnchor [ajlong] left anchor
** @param [r] rAnchor [ajlong] right anchor
**
** @return [ajlong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajlong btreeRebalance(AjPBtcache cache, ajlong thisNode,
			     ajlong leftNode, ajlong rightNode,
			     ajlong lAnchor, ajlong rAnchor)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    ajlong anchorNode   = 0L;
    ajlong balanceNode  = 0L;
    ajlong mergeNode    = 0L;
    ajlong done         = 0L;
    ajlong parent       = 0L;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;
    
    ajint lnkeys  = 0;
    ajint rnkeys  = 0;
    ajint size    = 0;
    ajint order   = 0;
    ajint minsize = 0;

    AjBool leftok  = ajFalse;
    AjBool rightok = ajFalse;
    
    
    /* ajDebug("In btreeRebalance\n"); */

    if(leftNode!=BTNO_NODE && lAnchor!=BTNO_NODE)
	leftok = ajTrue;
    if(rightNode!=BTNO_NODE && rAnchor!=BTNO_NODE)
	rightok = ajTrue;

    if(!leftok && !rightok)
	return BTNO_NODE;
    

    if(leftok)
    {
	lpage = ajBtreeCacheRead(cache,leftNode);
	lbuf  = lpage->buf;
	GBT_NKEYS(lbuf,&lnkeys);
    }
    

    if(rightok)
    {
	rpage = ajBtreeCacheRead(cache,rightNode);
	rbuf  = rpage->buf;
	GBT_NKEYS(rbuf,&rnkeys);
    }
    


    if(leftok && rightok)
    {
	size = (lnkeys >= rnkeys) ? lnkeys : rnkeys;
	balanceNode = (lnkeys >= rnkeys) ? leftNode : rightNode;
    }
    else if(leftok)
    {
	size = lnkeys;
	balanceNode = leftNode;
    }
    else
    {
	size = rnkeys;
	balanceNode = rightNode;
    }

    
    order = cache->order;
    minsize = (order-1) / 2;
    if((order-1)%2)
	++minsize;

    if(size >= minsize)
    {
	if(leftok && rightok)
	    anchorNode = (lnkeys >= rnkeys) ? lAnchor : rAnchor;
	else if(leftok)
	    anchorNode = lAnchor;
	else
	    anchorNode = rAnchor;
	done = btreeShift(cache,thisNode,balanceNode,anchorNode);
    }
	    
    else
    {
	tpage = ajBtreeCacheRead(cache,thisNode);
	tbuf  = tpage->buf;
	GBT_PREV(tbuf,&parent);
	if(leftok && rightok)
	{
	    anchorNode = (parent == lAnchor) ? lAnchor : rAnchor;
	    mergeNode  = (anchorNode  == lAnchor) ? leftNode : rightNode;
	}
	else if(leftok)
	{
	    anchorNode = lAnchor;
	    mergeNode  = leftNode;
	}
	else
	{
	    anchorNode = rAnchor;
	    mergeNode  = rightNode;
	}
	done = btreeMerge(cache,thisNode,mergeNode,anchorNode);
    }

    return done;
}




/* @funcstatic btreeShift *************************************************
**
** Shift spare entries from one node to another
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajlong] master node
** @param [r] balanceNode [ajlong] balance node
** @param [r] anchorNode [ajlong] anchor node
**
** @return [ajlong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajlong btreeShift(AjPBtcache cache, ajlong thisNode,
			 ajlong balanceNode, ajlong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *bbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kBarray = NULL;
    ajlong *pTarray = NULL;
    ajlong *pAarray = NULL;
    ajlong *pBarray = NULL;
    
    ajint  nAkeys = 0;
    ajint  nBkeys = 0;
    ajint  nTkeys = 0;
    ajint  order  = 0;
    ajint  count  = 0;
    ajint  i;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageB = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajint  anchorPos   = 0;
    ajlong prev        = 0L;
    ajint  nodetype    = 0;

    ajlong lv = 0L;
    
    /* ajDebug("In btreeShift\n"); */

    order = cache->order;
    AJCNEW0(kTarray,order);
    AJCNEW0(kAarray,order);
    AJCNEW0(kBarray,order);
    AJCNEW0(pTarray,order);
    AJCNEW0(pAarray,order);
    AJCNEW0(pBarray,order);
    for(i=0;i<order;++i)
    {
	kTarray[i] = ajStrNew();
	kAarray[i] = ajStrNew();
	kBarray[i] = ajStrNew();
    }


    pageA = ajBtreeCacheRead(cache,anchorNode);
    pageA->dirty = BT_LOCK;
    abuf = pageA->buf;
    pageB = ajBtreeCacheRead(cache,balanceNode);
    pageB->dirty = BT_LOCK;
    bbuf = pageB->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    pageT->dirty = BT_LOCK;
    tbuf = pageT->buf;

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(bbuf,&nBkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,bbuf,&kBarray,&pBarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    
    if(strcmp(kTarray[nTkeys-1]->Ptr,kBarray[nBkeys-1]->Ptr)<0)
	leftpage = pageT;
    else
	leftpage = pageB;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;
	while(i!=nAkeys && strcmp(kTarray[nTkeys-1]->Ptr,kAarray[i]->Ptr)>=0)
	    ++i;
	anchorPos = i;

	/* Move down anchor key to thisNode */
	ajStrAssS(&kTarray[nTkeys],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	while(nTkeys < nBkeys)
	{
	    ajStrAssS(&kTarray[nTkeys],kBarray[0]);
	    pTarray[nTkeys] = pBarray[0];
	    ++nTkeys;
	    --nBkeys;

	    for(i=0;i<nBkeys;++i)
	    {
		ajStrAssS(&kBarray[i],kBarray[i+1]);
		pBarray[i] = pBarray[i+1];
	    }
	    pBarray[i] = pBarray[i+1];
	}

	/* Adjust anchor key */
	ajStrAssS(&kAarray[anchorPos],kTarray[nTkeys-1]);
	--nTkeys;
    }
    else	/* thisNode on the right */
    {
	/* Find anchor key position */
	i=0;
	while(i!=nAkeys && strcmp(kBarray[nBkeys-1]->Ptr,kAarray[i]->Ptr)>=0)
	    ++i;
	anchorPos = i;

	/* Move down anchor key to thisNode */
	pTarray[nTkeys+1] = pTarray[nTkeys];
	for(i=nTkeys-1;i>-1;--i)
	{
	    ajStrAssS(&kTarray[i+1],kTarray[i]);
	    pTarray[i+1] = pTarray[i];
	}
	ajStrAssS(&kTarray[0],kAarray[anchorPos]);
	++nTkeys;

	/* Shift extra */
	count = 0;
	while(nTkeys < nBkeys)
	{
	    pTarray[nTkeys+1] = pTarray[nTkeys];
	    for(i=nTkeys-1;i>-1;--i)
	    {
		ajStrAssS(&kTarray[i+1],kTarray[i]);
		pTarray[i+1] = pTarray[i];
	    }
	    ajStrAssS(&kTarray[0],kBarray[nBkeys-1]);
	    pTarray[1] = pBarray[nBkeys];
	    ++nTkeys;
	    --nBkeys;
	}


	/* Adjust anchor key */
	ajStrAssS(&kAarray[anchorPos],kTarray[0]);
	--nTkeys;
	for(i=0;i<nTkeys;++i)
	{
	    ajStrAssS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
    }
    

    /* Adjust PREV pointers for thisNode */
    prev = pageT->pageno;
    for(i=0;i<nTkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pTarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);
	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }



    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageB,kBarray,pBarray,nBkeys);
    btreeWriteNode(cache,pageT,kTarray,pTarray,nTkeys);

    if(!anchorNode)
	pageA->dirty = BT_LOCK;

    for(i=0;i<order;++i)
    {
	ajStrDel(&kTarray[i]);
	ajStrDel(&kAarray[i]);
	ajStrDel(&kBarray[i]);
    }
    AJFREE(kTarray);
    AJFREE(kAarray);
    AJFREE(kBarray);
    AJFREE(pTarray);
    AJFREE(pAarray);
    AJFREE(pBarray);


    return BTNO_NODE;
}




/* @funcstatic btreeMerge *************************************************
**
** Merge two nodes
**
** @param [u] cache [AjPBtcache] cache
** @param [r] thisNode [ajlong] master node
** @param [r] mergeNode [ajlong] merge node
** @param [r] anchorNode [ajlong] anchor node
**
** @return [ajlong] page number or BTNO_NODE
** @@
******************************************************************************/

static ajlong btreeMerge(AjPBtcache cache, ajlong thisNode,
			 ajlong mergeNode, ajlong anchorNode)
{
    unsigned char *tbuf = NULL;
    unsigned char *abuf = NULL;
    unsigned char *nbuf = NULL;
    unsigned char *buf  = NULL;
    
    AjPStr *kTarray = NULL;
    AjPStr *kAarray = NULL;
    AjPStr *kNarray = NULL;
    ajlong *pTarray = NULL;
    ajlong *pAarray = NULL;
    ajlong *pNarray = NULL;

    ajlong thisprev  = 0L;
    ajlong mergeprev = 0L;
    
    
    ajint  nAkeys = 0;
    ajint  nNkeys = 0;
    ajint  nTkeys = 0;
    ajint  order  = 0;
    ajint  count  = 0;
    ajint  i;
    ajint  nodetype = 0;
    
    ajint saveA = 0;
    ajint saveN = 0;
    ajint saveT = 0;
    
    AjPBtpage pageA = NULL;
    AjPBtpage pageN = NULL;
    AjPBtpage pageT = NULL;
    AjPBtpage page  = NULL;
    
    AjPBtpage leftpage = NULL;

    ajint  anchorPos = 0;
    ajlong prev      = 0L;

    ajlong lv = 0L;
    

    AjBool collapse = ajFalse;
    
    /* ajDebug("In btreeMerge\n"); */

    order = cache->order;


    pageA = ajBtreeCacheRead(cache,anchorNode);
    saveA = pageA->dirty;
    pageA->dirty = BT_LOCK;
    abuf = pageA->buf;
    pageN = ajBtreeCacheRead(cache,mergeNode);
    saveN = pageN->dirty;
    pageN->dirty = BT_LOCK;
    nbuf = pageN->buf;
    pageT = ajBtreeCacheRead(cache,thisNode);
    saveT = pageT->dirty;
    pageT->dirty = BT_LOCK;
    tbuf = pageT->buf;

    GBT_PREV(tbuf,&thisprev);
    GBT_PREV(nbuf,&mergeprev);

    GBT_NKEYS(abuf,&nAkeys);
    GBT_NKEYS(nbuf,&nNkeys);
    GBT_NKEYS(tbuf,&nTkeys);

    GBT_NODETYPE(nbuf,&nodetype);


    if(nAkeys == 1)
    {
	if(!anchorNode && !thisprev && !mergeprev)
	    collapse = ajTrue;
	else
	{
	    pageA->dirty = saveA;
	    pageN->dirty = saveN;
	    pageT->dirty = saveT;
	    return thisNode;
	}
    }

    AJCNEW0(kTarray,order);
    AJCNEW0(kAarray,order);
    AJCNEW0(kNarray,order);
    AJCNEW0(pTarray,order);
    AJCNEW0(pAarray,order);
    AJCNEW0(pNarray,order);
    for(i=0;i<order;++i)
    {
	kTarray[i] = ajStrNew();
	kAarray[i] = ajStrNew();
	kNarray[i] = ajStrNew();
    }

    btreeGetKeys(cache,abuf,&kAarray,&pAarray);
    btreeGetKeys(cache,nbuf,&kNarray,&pNarray);
    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);

    if(strcmp(kTarray[nTkeys-1]->Ptr,kNarray[nNkeys-1]->Ptr)<0)
	leftpage = pageT;
    else
	leftpage = pageN;


    if(leftpage == pageT)
    {
	/* Find anchor key position */
	i=0;
	while(i!=nAkeys && strcmp(kTarray[nTkeys-1]->Ptr,kAarray[i]->Ptr)>=0)
	    ++i;
	anchorPos = i;

	/* Move down anchor key to neighbour Node */
	pNarray[nNkeys+1] = pNarray[nNkeys];
	for(i=nNkeys-1;i>-1;--i)
	{
	    ajStrAssS(&kNarray[i+1],kNarray[i]);
	    pNarray[i+1] = pNarray[i];
	}
	ajStrAssS(&kNarray[0],kAarray[anchorPos]);
	++nNkeys;


	/* Adjust anchor node keys/ptrs */
	++anchorPos;
	if(anchorPos==nAkeys)
	    pAarray[nAkeys-1] = pAarray[nAkeys];
	else
	{
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssS(&kAarray[i-1],kAarray[i]);
		pAarray[i-1] = pAarray[i];
	    }
	    pAarray[i-1] = pAarray[i];
	}
	--nAkeys;
	

	/* Merge this to neighbour */

	while(nTkeys)
	{
	    pNarray[nNkeys+1] = pNarray[nNkeys];
	    for(i=nNkeys-1;i>-1;--i)
	    {
		ajStrAssS(&kNarray[i+1],kNarray[i]);
		pNarray[i+1] = pNarray[i];
	    }
	    ajStrAssS(&kNarray[0],kTarray[nTkeys-1]);
	    pNarray[1] = pTarray[nTkeys];
	    pNarray[0] = pTarray[nTkeys-1];
	    --nTkeys;
	    ++nNkeys;
	}

	/* At this point the 'this' node could be added to a freelist */
    }
    else
    {
	/* Find anchor key position */
	i=0;
	while(i!=nAkeys && strcmp(kNarray[nNkeys-1]->Ptr,kAarray[i]->Ptr)>=0)
	    ++i;
	anchorPos = i;

	/* Move down anchor key to neighbourNode */
	ajStrAssS(&kNarray[nNkeys],kAarray[anchorPos]);
	++nNkeys;

	/* Adjust anchor node keys/ptrs */
	++anchorPos;
	if(anchorPos!=nAkeys)
	    for(i=anchorPos;i<nAkeys;++i)
	    {
		ajStrAssS(&kAarray[i-1],kAarray[i]);
		pAarray[i] = pAarray[i+1];
	    }
	--nAkeys;

	/* merge extra */
	count = 0;
	while(nTkeys)
	{
	    ajStrAssS(&kNarray[nNkeys],kTarray[count]);
	    pNarray[nNkeys] = pTarray[count];
	    ++nNkeys;
	    ++count;
	    --nTkeys;
	    pNarray[nNkeys] = pTarray[count];
	
	}

	/* At this point the 'this' node could be added to a freelist */
    }
    
    
    /* Adjust PREV pointers for neighbour Node */
    prev = pageN->pageno;
    for(i=0;i<nNkeys+1;++i)
    {
	page = ajBtreeCacheRead(cache,pNarray[i]);
	buf = page->buf;
	GBT_NODETYPE(buf,&nodetype);
	if(nodetype != BT_BUCKET)
	{
	    lv = prev;
	    SBT_PREV(buf,lv);
	    page->dirty = BT_DIRTY;
	}
    }

    pageT->dirty = BT_CLEAN;
    btreeWriteNode(cache,pageA,kAarray,pAarray,nAkeys);
    btreeWriteNode(cache,pageN,kNarray,pNarray,nNkeys);

    if(!anchorNode)
	pageA->dirty = BT_LOCK;

    
    for(i=0;i<order;++i)
    {
	ajStrDel(&kTarray[i]);
	ajStrDel(&kAarray[i]);
	ajStrDel(&kNarray[i]);
    }
    AJFREE(kTarray);
    AJFREE(kAarray);
    AJFREE(kNarray);
    AJFREE(pTarray);
    AJFREE(pAarray);
    AJFREE(pNarray);

    if(collapse)
	btreeCollapseRoot(cache,mergeNode);

    return thisNode;
}




/* @funcstatic btreeFindMin ***********************************************
**
** Find minimum key in subtree and store in cache
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page
** @param [r] key [const char *] key
**
** @return [void]
** @@
******************************************************************************/

static void btreeFindMin(AjPBtcache cache, ajlong pageno, const char *key)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPStr *karray   = NULL;
    ajlong *parray   = NULL;

    ajint nkeys    = 0;
    ajint nodetype = 0;
    ajint order    = 0;
    ajint nentries = 0;
    ajint i;

    unsigned char *buf = NULL;

    /* ajDebug("In btreeFindMin\n"); */

    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();
    
    page = ajBtreeCacheRead(cache,pageno);
    buf  = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    GBT_NKEYS(buf,&nkeys);

    btreeGetKeys(cache,buf,&karray,&parray);

    if(nodetype == BT_LEAF)
    {
	bucket = btreeReadBucket(cache,parray[0]);
	nentries = bucket->Nentries;
	if(nentries<2)
	{
	    btreeBucketDel(&bucket);
	    bucket = btreeReadBucket(cache,parray[1]);
	    nentries = bucket->Nentries;
	}
	if(nentries<1)	
	    ajFatal("FindMin: Too few entries in bucket Nkeys=%d\n",nkeys);
	ajStrAssS(&cache->replace,bucket->Ids[0]->id);
	if(!strcmp(cache->replace->Ptr,key))
	    ajStrAssS(&cache->replace,bucket->Ids[1]->id);
	for(i=1;i<nentries;++i)
	    if(strcmp(bucket->Ids[i]->id->Ptr,cache->replace->Ptr)<0 &&
	       strcmp(bucket->Ids[i]->id->Ptr,key))
		ajStrAssS(&cache->replace,bucket->Ids[i]->id);
	btreeBucketDel(&bucket);
    }
    else
    {
	pageno = parray[0];
	(void) btreeFindMin(cache,pageno,key);
	
    }
    

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return;
}




/* @funcstatic btreeInsertShift ********************************************
**
** Rebalance buckets on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const char *] key
**
** @return [ajlong] bucket block or 0L if shift not posible 
** @@
******************************************************************************/

static ajlong btreeInsertShift(AjPBtcache cache, AjPBtpage *retpage,
			       const char *key)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage tpage = NULL;

    ajint tkeys = 0;
    ajint pkeys = 0;
    ajint skeys = 0;
    ajint order = 0;
    
    ajint i;
    ajint n;
    
    ajlong parent  = 0L;
    ajlong blockno = 0L;
    
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajlong *pTarray = NULL;
    ajlong *pParray = NULL;
    ajlong *pSarray = NULL;

    AjPStr *karray = NULL;
    ajlong *parray = NULL;

    ajint ppos    = 0;
    ajint pkeypos = 0;
    ajint minsize = 0;
    
    /* ajDebug("In btreeInsertShift\n"); */


    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->order;
    minsize = order / 2;
    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    
    


    AJCNEW0(kParray,order);
    AJCNEW0(pParray,order);
    AJCNEW0(kSarray,order);
    AJCNEW0(pSarray,order);
    AJCNEW0(kTarray,order);
    AJCNEW0(pTarray,order);
    
    for(i=0;i<order;++i)
    {
	kSarray[i] = ajStrNew();
	kParray[i] = ajStrNew();
	kTarray[i] = ajStrNew();
    }

    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;
    while(i!=pkeys && strcmp(key,kParray[i]->Ptr)>=0)
	++i;
    pkeypos = i;
    
    if(i==pkeys)
    {
	if(strcmp(key,kParray[i-1]->Ptr)<0)
	    ppos = i-1;
	else
	    ppos = i;
    }
    else
	ppos = i;

    
    if(ppos) /* There is another leaf to the left */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos-1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	/* ajDebug("Left shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;
	while(pParray[i] != tpage->pageno)
	    ++i;
	--i;

	pkeypos = i;

	ajStrAssS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssS(&kParray[pkeypos],kTarray[0]);
	for(i=0;i<tkeys;++i)
	{
	    ajStrAssS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	i = 0;
	while(i!=pkeys && strcmp(key,kParray[i]->Ptr)>=0)
	    ++i;
	if(i==pkeys)
	{
	    if(strcmp(key,kParray[i-1]->Ptr)<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pageno)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	

	i = 0;
	while(i!=n && strcmp(key,karray[i]->Ptr)>=0)
	    ++i;
	if(i==n)
	{
	    if(strcmp(key,karray[i-1]->Ptr)<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return blockno;
    }
    

    if(ppos != pkeys)	/* There is a right node */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    /* Space in the right leaf */
    if(ppos != pkeys && skeys != order-1)
    {
	/* ajDebug("Right shift\n");*/
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;
	while(pParray[i] != tpage->pageno)
	    ++i;
	pkeypos = i;
	
	pSarray[skeys+1] = pSarray[skeys];
	for(i=skeys-1;i>-1;--i)
	{
	    ajStrAssS(&kSarray[i+1],kSarray[i]);
	    pSarray[i+1] = pSarray[i];
	}
	ajStrAssS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	i = 0;
	while(i!=pkeys && strcmp(key,kParray[i]->Ptr)>=0)
	    ++i;
	if(i==pkeys)
	{
	    if(strcmp(key,kParray[i-1]->Ptr)<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pageno)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	
	i = 0;
	while(i!=n && strcmp(key,karray[i]->Ptr)>=0)
	    ++i;
	if(i==n)
	{
	    if(strcmp(key,karray[i-1]->Ptr)<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return blockno;
    }


    for(i=0;i<order;++i)
    {
	ajStrDel(&kTarray[i]);
	ajStrDel(&kParray[i]);
	ajStrDel(&kSarray[i]);
    }
    AJFREE(kTarray);
    AJFREE(kSarray);
    AJFREE(kParray);
    AJFREE(pTarray);
    AJFREE(pSarray);
    AJFREE(pParray);

    return 0L;
}




/* @funcstatic btreeKeyShift ********************************************
**
** Rebalance Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
** @@
******************************************************************************/

static void btreeKeyShift(AjPBtcache cache, AjPBtpage tpage)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;
    unsigned char *buf  = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage page  = NULL;

    ajint tkeys = 0;
    ajint pkeys = 0;
    ajint skeys = 0;
    ajint order = 0;
    
    ajint i;
    
    ajlong parent  = 0L;
    
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajlong *pTarray = NULL;
    ajlong *pParray = NULL;
    ajlong *pSarray = NULL;

    ajint pkeypos = 0;
    ajint minsize = 0;

    ajlong lv = 0L;
    
    /* ajDebug("In btreeKeyShift\n"); */
    
    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->order;
    minsize = order / 2;
    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    

    AJCNEW0(kParray,order);
    AJCNEW0(pParray,order);
    AJCNEW0(kSarray,order);
    AJCNEW0(pSarray,order);
    AJCNEW0(kTarray,order);
    AJCNEW0(pTarray,order);
    
    for(i=0;i<order;++i)
    {
	kSarray[i] = ajStrNew();
	kParray[i] = ajStrNew();
	kTarray[i] = ajStrNew();
    }

    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    GBT_NKEYS(tbuf,&tkeys);


    btreeGetKeys(cache,pbuf,&kParray,&pParray);
    i=0;
    while(pParray[i] != tpage->pageno)
	++i;

    if(i) /* There is another leaf to the left */
    {
	pkeypos = i-1;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
	
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	ajStrAssS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssS(&kParray[pkeypos],kTarray[0]);
	for(i=0;i<tkeys;++i)
	{
	    ajStrAssS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	page = ajBtreeCacheRead(cache,pSarray[skeys]);
	buf = page->buf;
	lv = spage->pageno;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;


	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return;
    }



    if(i != pkeys)	/* There is a right node */
    {
	pkeypos = i;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    if(i != pkeys && skeys != order-1) /* Space in the right node */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	pSarray[skeys+1] = pSarray[skeys];
	for(i=skeys-1;i>-1;--i)
	{
	    ajStrAssS(&kSarray[i+1],kSarray[i]);
	    pSarray[i+1] = pSarray[i];
	}
	ajStrAssS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	page = ajBtreeCacheRead(cache,pSarray[0]);
	buf = page->buf;
	lv = spage->pageno;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;

	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return;
    }


    for(i=0;i<order;++i)
    {
	ajStrDel(&kTarray[i]);
	ajStrDel(&kParray[i]);
	ajStrDel(&kSarray[i]);
    }
    AJFREE(kTarray);
    AJFREE(kSarray);
    AJFREE(kParray);
    AJFREE(pTarray);
    AJFREE(pSarray);
    AJFREE(pParray);

    return;
}




#if 0
/* @funcstatic btreeTraverseLeaves *****************************************
**
** Find the next leaf by traversing the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [u] thys [AjPBtpage] current leaf page
**
** @return [AjPBtpage] next leaf or NULL
** @@
******************************************************************************/

static AjPBtpage btreeTraverseLeaves(AjPBtcache cache, AjPBtpage thys)
{
    AjPBtpage page = NULL;
    
    ajlong pageno = 0L;
    ajlong prev   = 0L;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;

    ajint nodetype = 0;
    ajint nkeys    = 0;
    ajint apos     = 0;
    ajint order    = 0;
    ajint i;

    unsigned char *buf = NULL;    

    if(!cache->level)
	return NULL;

    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    pageno = thys->pageno;
    buf = thys->buf;
    GBT_PREV(buf,&prev);

    page = ajBtreeCacheRead(cache,prev);
    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    btreeGetKeys(cache,buf,&karray,&parray);
    apos = 0;
    while(parray[apos] != pageno)
	++apos;

    while(apos == nkeys)
    {
	if(nodetype == BT_ROOT)
	{
	    for(i=0;i<order;++i)
		ajStrDel(&karray[i]);
	    AJFREE(karray);
	    AJFREE(parray);
	    return NULL;
	}

	GBT_PREV(buf,&prev);
	pageno = page->pageno;
	page = ajBtreeCacheRead(cache,prev);
	buf = page->buf;
	GBT_NKEYS(buf,&nkeys);
	GBT_NODETYPE(buf,&nodetype);
	btreeGetKeys(cache,buf,&karray,&parray);
	apos = 0;
	while(parray[apos] != pageno)
	    ++apos;
    }

    page = ajBtreeCacheRead(cache,parray[apos+1]);
    buf = page->buf;
    GBT_NODETYPE(buf,&nodetype);
    btreeGetKeys(cache,buf,&karray,&parray);
    
    while(nodetype != BT_LEAF)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetKeys(cache,buf,&karray,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return page;
}
#endif




#if 0
/* @funcstatic btreeJoinLeaves ***********************************************
**
** Update all Left/Right Leaf Pointers
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void] next leaf or NULL
** @@
******************************************************************************/

static void btreeJoinLeaves(AjPBtcache cache)
{
    unsigned char *buf = NULL;
    AjPBtpage page     = NULL;
    AjPBtpage newpage  = NULL;
    
    ajint nodetype = 0;
    ajint order    = 0;
    ajint i;

    AjPStr *karray = NULL;
    ajlong *parray = NULL;

    ajlong left    = 0L;
    ajlong right   = 0L;

    ajlong lv = 0L;
    
    if(!cache->level)
	return;

    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    page = btreeCacheLocate(cache,0L);
    buf = page->buf;
    btreeGetKeys(cache,buf,&karray,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetKeys(cache,buf,&karray,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }
    
    lv = left;
    SBT_LEFT(buf,lv);
    
    while((newpage = btreeTraverseLeaves(cache,page)))
    {
	right = newpage->pageno;
	lv = right;
	SBT_RIGHT(buf,lv);
	page->dirty = BT_DIRTY;
	left = page->pageno;
	buf = newpage->buf;
	lv = left;
	SBT_LEFT(buf,lv);
	page = newpage;
    }

    right = 0L;
    SBT_RIGHT(buf,right);
    page->dirty = BT_DIRTY;

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return;
}
#endif




/* @func ajBtreeWildNew *********************************************
**
** Construct a wildcard search object
**
** @param [u] cache [AjPBtcache] cache
** @param [r] wild [const AjPStr] wildcard id prefix (without asterisk)
**
** @return [AjPBtWild] b+ tree wildcard object
** @@
******************************************************************************/

AjPBtWild ajBtreeWildNew(AjPBtcache cache, const AjPStr wild)
{
    AjPBtWild thys = NULL;
    
    AJNEW0(thys);

    thys->id   = ajStrNewC(wild->Ptr);
    thys->list = ajListNew();

    thys->first = ajTrue;

    return thys;
}




/* @func ajBtreeWildDel *********************************************
**
** Destroy a wildcard search object
**
** @param [u] thys [AjPBtWild*] b+ tree wildcard structure
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeWildDel(AjPBtWild *thys)
{
    AjPBtWild pthis = NULL;
    AjPBtId   id    = NULL;
    
    pthis = *thys;
    if(!thys || !pthis)
	return;

    ajStrDel(&pthis->id);

    while(ajListPop(pthis->list,(void **)&id))
	ajBtreeIdDel(&id);

    ajListDel(&pthis->list);

    *thys = NULL;
    AJFREE(pthis);

    return;
}




/* @func ajBtreeFindInsertW ***********************************************
**
** Find the node that should contain a key (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char*] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeFindInsertW(AjPBtcache cache, const char *key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    /* ajDebug("In ajBtreeFindInsertW\n"); */

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,0L);
    
    if(!cache->level)
	return root;
    
    ret = btreeFindINodeW(cache,root,key);

    return ret;
}




/* @funcstatic btreeFindINodeW ************************************************
**
** Recursive search for node (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [r] item [const char*] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

static AjPBtpage btreeFindINodeW(AjPBtcache cache, AjPBtpage page,
				  const char *item)
{
    AjPBtpage ret = NULL;
    AjPBtpage pg  = NULL;

    unsigned char *buf = NULL;

    ajint status = 0;
    ajint ival   = 0;

    /* ajDebug("In btreeFindINodeW\n"); */
    
    ret = page;
    buf = page->buf;
    GBT_NODETYPE(buf,&ival);
    if(ival != BT_LEAF)
    {
	status = ret->dirty;
	ret->dirty = BT_LOCK;	/* Lock in case of lots of overflow pages */
	pg = btreePageFromKeyW(cache,buf,item);
	ret->dirty = status;
	ret = btreeFindINodeW(cache,pg,item);
    }
    
    return ret;
}




/* @funcstatic btreePageFromKeyW *******************************************
**
** Return next lower index page given a key (wild)
**
** @param [u] cache [AjPBtcache] cache
** @param [u] buf [unsigned char *] page buffer
** @param [r] key [const char *] key to search for 
**
** @return [AjPBtpage] pointer to a page
** @@
******************************************************************************/

static AjPBtpage btreePageFromKeyW(AjPBtcache cache, unsigned char *buf,
				    const char *key)
{
    ajint nkeys  = 0;
    ajint keylen = 0;
    ajint order  = 0;
    ajint i;
    
    ajlong blockno = 0L;
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    AjPBtpage page = NULL;

    unsigned char *rootbuf = NULL;
    
    /* ajDebug("In btreePageFromKeyW\n"); */
    
    rootbuf = buf;

    GBT_NKEYS(rootbuf,&nkeys);
    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    keylen = strlen(key);

    btreeGetKeys(cache,rootbuf,&karray,&parray);
    i = 0;
    while(i!=nkeys && strncmp(key,karray[i]->Ptr,keylen)>0)
	++i;
    if(i==nkeys)
    {
	if(strncmp(key,karray[i-1]->Ptr,keylen)<=0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    page =  ajBtreeCacheRead(cache,blockno);

    return page;
}




/* @funcstatic btreeReadLeaf ***********************************************
**
** Read all leaf Ids into a list
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] page
** @param [w] list [AjPList] list
**
** @return [void]
** @@
******************************************************************************/

static void btreeReadLeaf(AjPBtcache cache, AjPBtpage page, AjPList list)
{
    unsigned char *buf = NULL;
    AjPStr *karray     = NULL;
    ajlong *parray     = NULL;

    ajint keylimit = 0;
    ajint order    = 0;
    ajint nkeys    = 0;
    ajint bentries = 0;
    
    ajint i;
    ajint j;
    
    AjPBucket *buckets = NULL;

    /* ajDebug("In ReadLeaf\n"); */
    
    buf = page->buf;
    order = cache->order;

    AJCNEW0(parray,order);
    AJCNEW0(karray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,buf,&karray,&parray);

    GBT_NKEYS(buf,&nkeys);
    
    keylimit = nkeys+1;
    AJCNEW0(buckets,keylimit);

    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadBucket(cache,parray[i]);
    
    for(i=0;i<keylimit;++i)
    {
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(list,(void *)buckets[i]->Ids[j]);
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->Ids);
	AJFREE(buckets[i]);
    }
    ajListSort(list,btreeIdCompare);
    AJFREE(buckets);

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return;
}




/* @func ajBtreeIdFromKeyW ********************************************
**
** Wildcard retrieval of entries
**
** @param [u] cache [AjPBtcache] cache
** @param [u] wild [AjPBtWild] Wildcard
**
** @return [AjPBtId] next matching Id or NULL
** @@
******************************************************************************/

AjPBtId ajBtreeIdFromKeyW(AjPBtcache cache, AjPBtWild wild)
{

    AjPBtId id     = NULL;
    AjPBtpage page = NULL;
    char *key      = NULL;
    AjPList list   = NULL;
    AjBool found   = ajFalse;

    ajlong pageno = 0L;
    ajint keylen  = 0;

    unsigned char *buf = NULL;    

    key  = wild->id->Ptr;
    list = wild->list;

    keylen = strlen(key);

    found = ajFalse;
    
    if(wild->first)
    {
	page = ajBtreeFindInsertW(cache,key);
	page->dirty = BT_LOCK;
	wild->pageno = page->pageno;
	
	btreeReadLeaf(cache,page,list);

	page->dirty = BT_CLEAN;
	
	if(!ajListLength(list))
	    return NULL;

	while(ajListPop(list,(void **)&id))
	{
	    if(!strncmp(id->id->Ptr,key,keylen))
	    {
		found = ajTrue;
		break;
	    }
	    ajBtreeIdDel(&id);
	}

	wild->first = ajFalse;


	if(!found)	/* Check the next leaf just in case key==internal */
	{
	    buf = page->buf;
	    GBT_RIGHT(buf,&pageno);
	    if(!pageno)
		return NULL;
	    page = ajBtreeCacheRead(cache,pageno);
	    wild->pageno = pageno;
	    page->dirty = BT_LOCK;
	    
	    btreeReadLeaf(cache,page,list);	
	    
	    page->dirty = BT_CLEAN;
	    
	    if(!ajListLength(list))
		return NULL;
	    
	    found = ajFalse;
	    while(ajListPop(list,(void **)&id))
	    {
		if(!strncmp(id->id->Ptr,key,keylen))
		{
		    found = ajTrue;
		    break;
		}
		ajBtreeIdDel(&id);
	    }
	}


	if(!found)
	    return NULL;

	return id;
    }


    if(!ajListLength(list))
    {
	page = ajBtreeCacheRead(cache,wild->pageno); 
	buf = page->buf;
	GBT_RIGHT(buf,&pageno);
	if(!pageno)
	    return NULL;
	page = ajBtreeCacheRead(cache,pageno);
	wild->pageno = pageno;
	page->dirty = BT_LOCK;

	btreeReadLeaf(cache,page,list);	

	page->dirty = BT_CLEAN;
	
	if(!ajListLength(list))
	    return NULL;
    }
    


    while(ajListPop(list,(void **)&id))
    {
	if(!strncmp(id->id->Ptr,key,keylen))
	{
	    found = ajTrue;
	    break;
	}
	ajBtreeIdDel(&id);
    }
    
    
    if(!found)
	return NULL;
    
    return id;
}




/* @func ajBtreeReplaceId ************************************************
**
** Replace an ID structure in a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rid [const AjPBtId] replacement id object
**
** @return [AjBool] true if success
** @@
******************************************************************************/

AjBool ajBtreeReplaceId(AjPBtcache cache, const AjPBtId rid)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPBtId   id     = NULL;
    char *key        = NULL;
    
    AjPStr *karray  = NULL;
    ajlong *parray  = NULL;
    
    unsigned char *buf = NULL;

    ajint nentries = 0;
    ajint nkeys    = 0;
    ajint order    = 0;
    
    ajint i;
    
    ajlong blockno = 0L;
    AjBool found   = ajFalse;


    key = rid->id->Ptr;

    page = ajBtreeFindInsert(cache,key);
    buf = page->buf;
    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,buf,&karray,&parray);

    GBT_NKEYS(buf,&nkeys);

    i=0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    bucket = btreeReadBucket(cache,blockno);
    
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
    {
	if(!strcmp(key,bucket->Ids[i]->id->Ptr))
	{
	    found = ajTrue;
	    break;
	}
    }

    if(found)
    {
	id->dbno = rid->dbno;
	id->dups = rid->dups;
	id->offset = rid->offset;
	btreeWriteBucket(cache,bucket,blockno);
    }

    btreeBucketDel(&bucket);
    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    if(!found)
	return ajFalse;

    return ajTrue;
}




/* @func ajBtreeReadEntries ************************************************
**
** Read B+ tree entries from file
**
** @param [r] filename [const char*] file name
** @param [r] indexdir [const char*] index file directory
**
** @return [AjPStr*] array of database filenames
** @@
******************************************************************************/

AjPStr* ajBtreeReadEntries(const char *filename, const char *indexdir)
{
    AjPStr line = NULL;
    AjPStr fn   = NULL;
    AjPStr str  = NULL;
    
    AjPStr *files = NULL;
    AjPList list;
    AjPFile inf   = NULL;
    char p;

    line = ajStrNew();
    list = ajListNew();

    fn = ajStrNew();
    ajFmtPrintS(&fn,"%s/%s",indexdir,filename);
    
    ajStrAppC(&fn,".ent");
    
    inf = ajFileNewIn(fn);
    if(!inf)
	ajFatal("Cannot open database entries file %S",fn);

    while(ajFileReadLine(inf, &line))
    {
	p = *(line->Ptr);
	if(p == '#' || !ajStrLen(line))
	    continue;
	str = ajStrNew();
	ajStrAssS(&str,line);
	ajListPushApp(list,(void *)str);
    }

    ajListToArray(list,(void ***)&files);
    
    ajListDel(&list);
    ajStrDel(&line);
    ajStrDel(&fn);
    ajFileClose(&inf);

    return files;
}




/* @func ajBtreeInsertDupId ************************************************
**
** Get an ID structure from a leaf node given a key
**
** @param [u] cache [AjPBtcache] cache
** @param [u] id [AjPBtId] potentially duplicate id
**
** @return [void]
** @@
******************************************************************************/

void ajBtreeInsertDupId(AjPBtcache cache, AjPBtId id)
{
    AjPBtpage page   = NULL;
    AjPBucket bucket = NULL;
    AjPBtId   tid    = NULL;
    
    AjPStr *karray  = NULL;
    ajlong *parray  = NULL;
    
    unsigned char *buf = NULL;

    ajint nentries = 0;
    ajint nkeys    = 0;
    ajint order    = 0;
    
    ajint i;
    
    ajlong blockno = 0L;
    AjBool found   = ajFalse;

    AjPStr oldkey = NULL;

    page = ajBtreeFindInsert(cache,id->id->Ptr);

    buf = page->buf;
    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    if(cache->count)
    {
	btreeGetKeys(cache,buf,&karray,&parray);

	GBT_NKEYS(buf,&nkeys);

	i=0;
	while(i!=nkeys && strcmp(id->id->Ptr,karray[i]->Ptr)>=0)
	    ++i;
    
	if(i==nkeys)
	{
	    if(strcmp(id->id->Ptr,karray[i-1]->Ptr)<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	bucket = btreeReadBucket(cache,blockno);
    
	nentries = bucket->Nentries;

	found = ajFalse;

	for(i=0;i<nentries;++i)
	{
	    if(!strcmp(id->id->Ptr,bucket->Ids[i]->id->Ptr))
	    {
		found = ajTrue;
		break;
	    }
	}

	if(found)
	{
	    oldkey = ajStrNewC(id->id->Ptr);
	    tid = bucket->Ids[i];
	    ++tid->dups;
	    btreeWriteBucket(cache,bucket,blockno);
	    ajWarn("Dealing with a duplicate ID (%s)\n",id->id->Ptr);
	    ajFmtPrintS(&id->id,"%S%c%d",oldkey,'\1',tid->dups);
	    ajStrDel(&oldkey);
	}

	btreeBucketDel(&bucket);
    }
    

    ajBtreeInsertId(cache,id);

    ++cache->count;

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return;
}




/* @func ajBtreeDupFromKey ************************************************
**
** Write B+ tree parameters to file
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char *] key
**
** @return [AjPList] list of matching AjPBtIds or NULL
** @@
******************************************************************************/

AjPList ajBtreeDupFromKey(AjPBtcache cache, const char *key)
{
    AjPList list = NULL;
    AjPBtId id   = NULL;
    ajint i;
    ajint dups;
    
    AjPStr dupkey = NULL;
    AjPStr okey   = NULL;
    

    if(!(id = ajBtreeIdFromKey(cache,key)))
	return NULL;

    dupkey = ajStrNew();
    okey   = ajStrNew();
    list   = ajListNew();
    ajListPush(list,(void *)id);


    if(id->dups)
    {
	ajStrAssS(&okey,id->id);
	dups = id->dups;
	for(i=0;i<dups;++i)
	{
	    ajFmtPrintS(&dupkey,"%S%c%d",okey,'\1',i+1);
	    id = ajBtreeIdFromKey(cache,dupkey->Ptr);
	    if(!id)
		ajFatal("DupFromKey: Id not found\n");
	    ajListPushApp(list,(void *)id);
	}
    }

    ajStrDel(&okey);
    ajStrDel(&dupkey);
    

    return list;
}



/* @funcstatic btreePriBucketNew *********************************************
**
** Construct a primary keyword bucket object
**
** @param [r] n [ajint] Number of IDs
**
** @return [AjPPriBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPPriBucket btreePriBucketNew(ajint n)
{
    AjPPriBucket bucket = NULL;
    ajint i;

    ajDebug("In btreePriBucketNew\n");
    
    AJNEW0(bucket);

    if(n)
    {
	AJCNEW0(bucket->codes,n);
	AJCNEW0(bucket->keylen,n);
    }
    
    for(i=0;i<n;++i)
	bucket->codes[i] = ajBtreePriNew();

    bucket->NodeType = BT_PRIBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreePriBucketDel *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPPriBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreePriBucketDel(AjPPriBucket *thys)
{
    AjPPriBucket pthis = NULL;
    ajint n;
    ajint i;
    
    ajDebug("In btreePriBucketDel\n");

    if(!thys || !*thys)
	return;

    pthis = *thys;
    n = pthis->Nentries;

    for(i=0;i<n;++i)
	ajBtreePriDel(&pthis->codes[i]);
    

    AJFREE(pthis->keylen);
    AJFREE(pthis->codes);
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajBtreePriNew *********************************************
**
** Constructor for index bucket keyword information
**
**
** @return [AjPBtPri] Index ID object
** @@
******************************************************************************/

AjPBtPri ajBtreePriNew(void)
{
    AjPBtPri pri = NULL;

    ajDebug("In ajBtreePriNew\n");

    AJNEW0(pri);
    pri->keyword   = ajStrNew();
    pri->treeblock = 0L;
    pri->id        = ajStrNew();

    return pri;
}




/* @func ajBtreePriDel *********************************************
**
** Destructor for keyword index primary bucket information
**
** @param [w] thys [AjPBtPri*] index keyword primary object
**
** @return [void]
** @@
******************************************************************************/

void ajBtreePriDel(AjPBtPri *thys)
{
    AjPBtPri pri = NULL;

    ajDebug("In ajBtreePriDel\n");

    if(!thys || !*thys)
	return;
    pri = *thys;
    
    ajStrDel(&pri->keyword);
    ajStrDel(&pri->id);
    AJFREE(pri);
    *thys = NULL;

    return;
}




/* @funcstatic btreeReadPriBucket *********************************************
**
** Constructor for keyword index primary bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [AjPPriBucket] bucket
** @@
******************************************************************************/

static AjPPriBucket btreeReadPriBucket(AjPBtcache cache, ajlong pageno)
{
    AjPPriBucket bucket = NULL;
    AjPBtpage page      = NULL;
    AjPBtpage lpage     = NULL;
    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    AjPBtPri pri        = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajint  nodetype  = 0;
    ajint  nentries  = 0;
    ajlong overflow  = 0L;
    ajint  dirtysave = 0;
    
    ajint  i;
    ajint  len  = 0;
    
    ajDebug("In btreeReadPriBucket\n");
    
    if(!pageno)
	ajFatal("PriBucketRead: cannot read bucket from root page");

    page  = ajBtreeCacheRead(cache,pageno);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriReadBucket: NodeType mismatch. Not primary bucket (%d)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->nperbucket)
	ajFatal("PriReadBucket: Bucket too full\n");
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    AJNEW0(bucket);
    bucket->NodeType = nodetype;
    bucket->Nentries = nentries;
    bucket->Overflow = overflow;
    
    AJCNEW0(bucket->keylen, nentries+1);
    AJCNEW0(bucket->codes, nentries+1);

    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJINT(kptr,&len);
	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    ajDebug("PriReadBucket: Overflow\n");
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_PRIBUCKET)
		ajFatal("PriReadBucket: NodeType mismatch. Not primary "
			"bucket (%d)",nodetype);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	bucket->codes[i] = ajBtreePriNew();
	pri = bucket->codes[i];

	/* Fill ID objects */
	ajStrAssC(&pri->keyword,(const char *)codeptr);
	codeptr += (strlen((const char *)codeptr) + 1);
	BT_GETAJLONG(codeptr,&pri->treeblock);
	codeptr += sizeof(ajlong);

	kptr += sizeof(ajint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeWritePriBucket *******************************************
**
** Write primary keyword index bucket object to the cache given a disc page
** number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPPriBucket] bucket
** @param [r] pageno [ajlong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWritePriBucket(AjPBtcache cache, const AjPPriBucket bucket,
				ajlong pageno)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    ajint  v   = 0;
    ajint  i   = 0;
    ajint  len = 0;
    ajlong lv  = 0L;
    ajlong pno = 0L;

    AjPBtPri pri    = NULL;
    ajint  nentries = 0;
    ajlong overflow = 0L;

    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;

    
    ajDebug("In btreeWritePriBucket\n");

    if(pageno == cache->totsize)	/* Create a new page */
    {
	pno = pageno;
	page = ajBtreeCacheWrite(cache,pageno);
	page->pageno = cache->totsize;
	cache->totsize += cache->pagesize;
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pageno);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_PRIBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    lbuf = buf;
    page->dirty = BT_LOCK;
    lpage = page;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    /* Write out key lengths */
    keyptr = PBT_BUCKKEYLEN(lbuf);
    for(i=0;i<nentries;++i)
    {
	if((keyptr-lbuf+1)+sizeof(ajint) > cache->pagesize)
	    ajFatal("PriBucketWrite: Bucket cannot hold more than %d keys",
		    i-1);

	pri = bucket->codes[i];
	/* Need to alter this if bucket primary keyword structure changes */
	len = BT_BUCKPRILEN(pri->keyword);
        v = len;
	BT_SETAJINT(keyptr,v);
	keyptr += sizeof(ajint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;
    for(i=0;i<nentries;++i)
    {
	pri = bucket->codes[i];
	len = BT_BUCKPRILEN(pri->keyword);
	if((lptr-buf+1)+len > cache->pagesize) /* overflow */
	{
    	    ajDebug("WritePriBucket: Overflow\n");
	    if(!overflow)		/* No overflow buckets yet */
	    {
		pno = cache->totsize;
                lv = pno;
		SBT_BUCKOVERFLOW(buf,lv);
		page = ajBtreeCacheWrite(cache,pno);
		page->pageno = cache->totsize;
		cache->totsize += cache->pagesize;
		buf = page->buf;
		v = BT_PRIBUCKET;
		SBT_BUCKNODETYPE(buf,v);
		v = 0;
		SBT_BUCKNENTRIES(buf,v);
		lv = 0L;
		SBT_BUCKOVERFLOW(buf,lv);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		buf  = page->buf;
		GBT_BUCKOVERFLOW(buf,&overflow);
	    }

	    page->dirty = BT_DIRTY;
	    lptr = PBT_BUCKKEYLEN(buf);	    
	}
	
	sprintf((char *)lptr,"%s",ajStrStr(pri->keyword));
	lptr += (ajStrLen(pri->keyword) + 1);
        lv = pri->treeblock;
	BT_SETAJLONG(lptr,lv);
	lptr += sizeof(ajlong);
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);
    
    lpage->dirty = BT_DIRTY;

    return;
}




/* @funcstatic btreeAddToPriBucket *******************************************
**
** Add a keyword ID to a primary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number of bucket
** @param [r] pri [const AjPBtPri] ID info
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToPriBucket(AjPBtcache cache, ajlong pageno,
				const AjPBtPri pri)
{
    AjPPriBucket bucket  = NULL;
    AjPBtPri     dest    = NULL;
    AjPBtpage    page    = NULL;
    AjPBtpage    pripage = NULL;
    
    ajint nentries;
    ajlong secrootpage = 0L;
    ajlong right       = 0L;

    unsigned char *buf;
    
    ajDebug("In btreeAddToPriBucket\n");

    bucket   = btreeReadPriBucket(cache,pageno);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */
    bucket->codes[nentries] = ajBtreePriNew();
    dest = bucket->codes[nentries];

    ajStrAssS(&dest->keyword,pri->keyword);

    /* Need to add code here to access secondary tree */
    dest->treeblock = pri->treeblock;


    /* See if secondary tree exists, if not then create it */
    if(!pri->treeblock)
    {
	secrootpage = cache->totsize;

	ajBtreeCreateRootNode(cache,secrootpage);
	cache->secrootblock = secrootpage;
	page = ajBtreeCacheWrite(cache,secrootpage);
	page->dirty = BT_DIRTY;
	ajBtreeCacheSync(cache,secrootpage);
	page->dirty = BT_LOCK;
	dest->treeblock = secrootpage;
	buf = page->buf;
	cache->slevel = 0;

	/* Relock the primary root page as a CacheSync unlocks all */ 
	pripage = btreeCacheLocate(cache,0L);
	pripage->dirty = BT_LOCK;

	ajDebug("Created secondary tree at block %d\n",(ajint)secrootpage);
    }
    else
    {
	cache->secrootblock = pri->treeblock;
	page = ajBtreeCacheWrite(cache,cache->secrootblock);
	page->dirty = BT_LOCK;
	buf = page->buf;
	GBT_RIGHT(buf, &right);
	cache->slevel = (ajint) right;
    }

    ajBtreeInsertSecId(cache, pri->id);

    right = (ajlong) cache->slevel;
    
    SBT_RIGHT(buf, right);
    page->dirty = BT_DIRTY;
    



    
    ++bucket->Nentries;

    btreeWritePriBucket(cache,bucket,pageno);

    btreePriBucketDel(&bucket);
    
    return;
}




/* @funcstatic btreeNumInPriBucket *******************************************
**
** Return number of entries in a primary keyword bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [ajint] Number of entries in bucket
** @@
******************************************************************************/

static ajint btreeNumInPriBucket(AjPBtcache cache, ajlong pageno)
{
    AjPBtpage page     = NULL;
    unsigned char *buf = NULL;

    ajint  nodetype = 0;
    ajint  nentries = 0;
    
    ajDebug("In btreeNumInPriBucket\n");
    
    if(!pageno)
	ajFatal("NumInPriBucket: Attempt to read bucket from root page\n");

    page  = ajBtreeCacheRead(cache,pageno);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_PRIBUCKET)
	ajFatal("PriReadBucket: NodeType mismatch. Not primary bucket (%d)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeKeywordCompare *******************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeKeywordCompare(const void *a, const void *b)
{
    return strcmp((*(AjPBtPri*)a)->keyword->Ptr,
		  (*(AjPBtPri*)b)->keyword->Ptr);
}




/* @funcstatic btreeReorderPriBuckets *****************************************
**
** Re-order primary keyword leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/
static AjBool btreeReorderPriBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajint nkeys = 0;
    AjPPriBucket *buckets = NULL;
    unsigned char *lbuf   = NULL;

    AjPStr *keys      = NULL;
    ajlong *ptrs      = NULL;
    ajlong *overflows = NULL;
    
    ajint i = 0;
    ajint j = 0;
    
    ajint order;
    ajint bentries      = 0;
    ajint totalkeys     = 0;
    ajint nperbucket    = 0;
    ajint maxnperbucket = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    ajint keylimit      = 0;
    ajint bucketn       = 0;
    ajint bucketlimit   = 0;
    ajint nodetype      = 0;
    ajint dirtysave     = 0;    

    AjPList idlist       = NULL;
    AjPBtPri bid         = NULL;
    AjPPriBucket cbucket = NULL;
    AjPBtPri cid         = NULL;

    ajint v = 0;
    
    ajDebug("In btreeReorderPriBuckets\n");

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->order;
    nperbucket = cache->nperbucket;
    

    /* Read keys/ptrs */
    AJCNEW0(keys,order);
    AJCNEW0(ptrs,order);
    AJCNEW0(overflows,order);
    
    for(i=0;i<order;++i)
	keys[i] = ajStrNew();
    btreeGetKeys(cache,lbuf,&keys,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("PriBucketReorder: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInPriBucket(cache,ptrs[i]);
    totalkeys += btreeNumInPriBucket(cache,ptrs[i]);

    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);
    if(totalkeys % maxnperbucket)
	++bucketn;
    
    if(bucketn > order)
    {
	for(i=0;i<order;++i)
	    ajStrDel(&keys[i]);
	AJFREE(keys);
	AJFREE(ptrs);
	AJFREE(overflows);
	
	leaf->dirty = dirtysave;
	return ajFalse;
    }
    

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;

    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadPriBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing kw) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->codes[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->codes);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeKeywordCompare);
    AJFREE(buckets);

    cbucket = btreePriBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;
    
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;
	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[count];
	    ajStrAssS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;

	    cbucket->keylen[count] = BT_BUCKPRILEN(bid->keyword);
	    ++cbucket->Nentries;
	    ++count;
	    ajBtreePriDel(&bid);
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&keys[i],bid->keyword);

	totkeylen += ajStrLen(bid->keyword);

	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,ptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;
    while(ajListPop(idlist,(void **)&bid))
    {
	cid = cbucket->codes[count];
	ajStrAssS(&cid->keyword,bid->keyword);
	cid->treeblock = bid->treeblock;

	++cbucket->Nentries;
	++count;
	ajBtreePriDel(&bid);
    }
    
    
    if(!ptrs[i])
	ptrs[i] = cache->totsize;
    btreeWritePriBucket(cache,cbucket,ptrs[i]);

    cbucket->Nentries = maxnperbucket;
    btreePriBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);

    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = BT_DIRTY;
    if(nodetype == BT_ROOT)
	leaf->dirty = BT_LOCK;
    
    for(i=0;i<order;++i)
	ajStrDel(&keys[i]);
    AJFREE(keys);
    AJFREE(ptrs);
    AJFREE(overflows);
    

    btreePriBucketDel(&cbucket);
    ajListDel(&idlist);

    return ajTrue;
}




/* @func ajBtreeInsertKeyword *********************************************
**
** Insert a keyword structure into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pri [const AjPBtPri] Id object
**
** @return [void] pointer to a page
** @@
******************************************************************************/

void ajBtreeInsertKeyword(AjPBtcache cache, const AjPBtPri pri)
{
    AjPBtpage spage   = NULL;
    AjPBtpage page    = NULL;
    AjPBtpage parent  = NULL;
    AjPStr key        = NULL;
    const char *ckey  = NULL;
    ajlong lblockno = 0L;
    ajlong rblockno = 0L;
    ajlong blockno  = 0L;
    ajlong shift    = 0L;

    ajint nkeys = 0;
    ajint order = 0;

    ajint nodetype = 0;
    ajlong right   = 0L;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint i;
    ajint n;

    AjPPriBucket lbucket = NULL;
    AjPPriBucket rbucket = NULL;
    unsigned char *buf   = NULL;

    AjPBtPri exists = NULL;

    
    ajDebug("In ajBtreeInsertKeyword %S %S\n",pri->keyword,pri->id);

    /* Only insert a primary key if that key doesn't exist */
    exists = ajBtreePriFromKeyword(cache,MAJSTRSTR(pri->keyword));
    if(exists)
    {
	cache->secrootblock = exists->treeblock;
	page = ajBtreeCacheWrite(cache,cache->secrootblock);
	page->dirty = BT_LOCK;
	buf = page->buf;
	GBT_RIGHT(buf,&right);
	cache->slevel = (ajint) right;
	ajBtreeInsertSecId(cache,pri->id);
	right = (ajlong) cache->slevel;
	SBT_RIGHT(buf,right);
	page->dirty = BT_DIRTY;
	
	ajBtreePriDel(&exists);

	return;
    }
    ajDebug("Keyword doesn't exist\n");
    
    key = ajStrNew();
    

    ajStrAssS(&key,pri->keyword);
    if(!ajStrLen(key))
    {
	ajStrDel(&key);
	return;
    }

    ckey = ajStrStr(key);
    spage = ajBtreeFindInsert(cache,ckey);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    order = cache->order;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();
    
    if(!nkeys)
    {
	lbucket  = btreePriBucketNew(0);
	rbucket  = btreePriBucketNew(0);

	lblockno = cache->totsize;
	btreeWritePriBucket(cache,lbucket,lblockno);

	rblockno = cache->totsize;
	btreeWritePriBucket(cache,rbucket,rblockno);	

	parray[0] = lblockno;
	parray[1] = rblockno;
	ajStrAssS(karray,key);
	
	btreeWriteNode(cache,spage,karray,parray,1);

	GBT_BLOCKNUMBER(buf,&blockno);
	if(!blockno)
	    spage->dirty = BT_LOCK;

	btreePriBucketDel(&lbucket);
	btreePriBucketDel(&rbucket);

	btreeAddToPriBucket(cache,rblockno,pri);
	++cache->count;

	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);

	AJFREE(karray);
	AJFREE(parray);
	ajStrDel(&key);
	return;
    }
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;
    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    if(nodetype != BT_ROOT)
	if((shift = btreeInsertShift(cache,&spage,key->Ptr)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInPriBucket(cache,blockno);

    if(n == cache->nperbucket)
    {
	if(btreeReorderPriBuckets(cache,spage))
	{
	    GBT_NKEYS(buf,&nkeys);	    
	    btreeGetKeys(cache,buf,&karray,&parray);

	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;

	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];
	}
	else
	{
	    parent = btreeSplitPriLeaf(cache,spage);
	    spage = ajBtreeFindInsert(cache,ckey);
	    buf = spage->buf;

	    btreeGetKeys(cache,buf,&karray,&parray);

	    GBT_NKEYS(buf,&nkeys);
	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;

	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];

/*
	    spage = ajBtreeCacheRead(cache,blockno);
	    buf = spage->buf;
	    btreeGetKeys(cache,buf,&karray,&parray);
	    GBT_NKEYS(buf,&nkeys);
	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;
	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];
*/
	}
    }

    btreeAddToPriBucket(cache,blockno,pri);
    ++cache->count;

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);

    AJFREE(karray);
    AJFREE(parray);

    ajStrDel(&key);

    return;
}




/* @func ajBtreePriFromKeyword **********************************************
**
** Get Pri structure matching a keyword
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char *] key
**
** @return [AjPBtPri] pointer to an Pri structure or NULL if not found
** @@
******************************************************************************/

AjPBtPri ajBtreePriFromKeyword(AjPBtcache cache, const char *key)
{
    AjPBtpage page      = NULL;
    AjPPriBucket bucket = NULL;
    AjPBtPri  pri       = NULL;
    AjPBtPri  tpri      = NULL;
    
    AjPStr *karray  = NULL;
    ajlong *parray  = NULL;
    
    unsigned char *buf = NULL;

    ajint nentries = 0;
    ajint nkeys    = 0;
    ajint order    = 0;
    
    ajint i;
    
    ajlong blockno = 0L;
    AjBool found   = ajFalse;


    ajDebug("ajBtreePriFromKeyword\n");
    

    if(!cache->count)
	return NULL;

    page = ajBtreeFindInsert(cache,key);

    buf = page->buf;
    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,buf,&karray,&parray);
    
    GBT_NKEYS(buf,&nkeys);

    i=0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    bucket = btreeReadPriBucket(cache,blockno);
    
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
    {
	if(!strcmp(key,bucket->codes[i]->keyword->Ptr))
	{
	    found = ajTrue;
	    break;
	}
    }

    if(found)
    {
	pri  = ajBtreePriNew();
	tpri = bucket->codes[i];
	ajStrAssS(&pri->keyword,tpri->keyword);
	pri->treeblock = tpri->treeblock;
    }

    btreePriBucketDel(&bucket);
    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    if(!found)
	return NULL;

    return pri;
}




/* @funcstatic btreeSplitPriLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeSplitPriLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajint nkeys     = 0;
    ajint order     = 0;
    ajint totalkeys = 0;
    ajint bentries  = 0;
    ajint keylimit  = 0;
    ajint nodetype  = 0;

    ajint rootnodetype = 0;
    
    ajint i;
    ajint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajlong mediangtr  = 0L;
    ajlong medianless = 0L;
    

    AjPBtPri bid       = NULL;
    AjPBtPri cid       = NULL;
    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPPriBucket *buckets = NULL;
    AjPPriBucket cbucket  = NULL;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint keypos = 0;
    ajint lno    = 0;
    ajint rno    = 0;

    ajint bucketlimit   = 0;
    ajint maxnperbucket = 0;
    ajint nperbucket    = 0;
    ajint bucketn       = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    
    ajlong lblockno = 0L;
    ajlong rblockno = 0L;
    ajlong prev     = 0L;
    ajlong overflow = 0L;

    ajlong zero = 0L;
    ajlong join = 0L;
    
    ajlong lv = 0L;
    ajint  v  = 0;
    
    ajDebug("In btreeSplitPriLeaf\n");

    order = cache->order;
    nperbucket = cache->nperbucket;

    mediankey = ajStrNew();
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    if(rootnodetype == BT_ROOT)
    {
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWrite(cache,lblockno);
	lpage->pageno = cache->totsize;
	cache->totsize += cache->pagesize;
	lbuf = lpage->buf;
	lv = prev;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pageno;
	lpage = spage;
    }


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;
    rbuf = rpage->buf;


    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }
    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetKeys(cache,buf,&karray,&parray);


    keylimit = nkeys+1;
    AJCNEW0(buckets,keylimit);
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadPriBucket(cache,parray[i]);

    idlist = ajListNew();
    for(i=0;i<keylimit;++i)
    {
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->codes[j]);
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->codes);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeKeywordCompare);
    AJFREE(buckets);


    totalkeys = ajListLength(idlist);

    keypos = totalkeys / 2;

    lno = keypos;
    rno = totalkeys - lno;

    maxnperbucket = nperbucket >> 1;
    if(!maxnperbucket)
	++maxnperbucket;

    cbucket = btreePriBucketNew(maxnperbucket);

    bucketn = lno / maxnperbucket;
    if(lno % maxnperbucket)
	++bucketn;
    bucketlimit = bucketn - 1;


    totkeylen = 0;
    count = 0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<maxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[j];
	    ajStrAssS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;

	    cbucket->keylen[j] = BT_BUCKPRILEN(bid->keyword);
	    ++count;
	    ++cbucket->Nentries;
	    ajBtreePriDel(&bid);
	}
	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssS(&karray[i],bid->keyword);
	totkeylen += ajStrLen(bid->keyword);

	if(!parray[i])
	    parray[i] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,parray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;
    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);
	cid = cbucket->codes[j];
	++j;
	++count;

	ajStrAssS(&cid->keyword,bid->keyword);
	cid->treeblock = bid->treeblock;
	++cbucket->Nentries;
	ajBtreePriDel(&bid);
    }

    if(!parray[i])
	parray[i] = cache->totsize;
    btreeWritePriBucket(cache,cbucket,parray[i]);

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lpage->dirty = BT_DIRTY;
    btreeWriteNode(cache,lpage,karray,parray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    ajStrAssS(&mediankey,bid->keyword);

    totkeylen = 0;
    bucketn = rno / maxnperbucket;
    if(rno % maxnperbucket)
	++bucketn;
    bucketlimit = bucketn - 1;

    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<maxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    cid = cbucket->codes[j];
	    ajStrAssS(&cid->keyword,bid->keyword);
	    cid->treeblock = bid->treeblock;

	    cbucket->keylen[j] = BT_BUCKPRILEN(bid->keyword);
	    ++cbucket->Nentries;
	    ajBtreePriDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&karray[i],bid->keyword);
	totkeylen += ajStrLen(bid->keyword);

	parray[i] = cache->totsize;
	btreeWritePriBucket(cache,cbucket,parray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;
    while(ajListPop(idlist,(void**)&bid))
    {
	cid = cbucket->codes[j];
	++j;

	ajStrAssS(&cid->keyword,bid->keyword);
	cid->treeblock = bid->treeblock;
	++cbucket->Nentries;
	ajBtreePriDel(&bid);
    }
    
    parray[i] = cache->totsize;
    btreeWritePriBucket(cache,cbucket,parray[i]);

    nkeys = bucketn - 1;

    v = nkeys;
    SBT_NKEYS(rbuf,v);
    v = totkeylen;
    SBT_TOTLEN(rbuf,v);
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNode(cache,rpage,karray,parray,nkeys);
    rpage->dirty = BT_DIRTY;

    cbucket->Nentries = maxnperbucket;
    btreePriBucketDel(&cbucket);
    ajListDel(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;


    if(rootnodetype == BT_ROOT)
    {
	ajStrAssS(&karray[0],mediankey);
	parray[0]=lblockno;
	parray[1]=rblockno;
	nkeys = 1;
	btreeWriteNode(cache,spage,karray,parray,nkeys);	
	spage->dirty = BT_LOCK;
	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	ajStrDel(&mediankey);
	++cache->level;
	return spage;
    }


    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    page = ajBtreeCacheRead(cache,prev);
    btreeInsertKey(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prev);

    return page;
}




/* @funcstatic btreeReadSecBucket *********************************************
**
** Constructor for keyword index secondary bucket given a disc page number
** Creates one empty key slot for possible addition
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [AjPSecBucket] bucket
** @@
******************************************************************************/

static AjPSecBucket btreeReadSecBucket(AjPBtcache cache, ajlong pageno)
{
    AjPSecBucket bucket = NULL;

    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *kptr = NULL;
    
    unsigned char *codeptr = NULL;
    
    ajint  nodetype = 0;
    ajint  nentries = 0;
    ajlong overflow = 0L;
    ajint  dirtysave = 0;
    
    ajint  i;
    ajint  len  = 0;
    
    ajDebug("In btreeReadSecBucket\n");

    /* Put in test here for secondary root page read instead of !pageno */
#if 0    
    if(!pageno)
	ajFatal("SecBucketRead: cannot read bucket from root page");
#endif

    page  = ajBtreeCacheRead(cache,pageno);
    lpage = page;
    dirtysave = lpage->dirty;
    lpage->dirty = BT_LOCK;

    buf = lpage->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecReadBucket: NodeType mismatch. Not secondary bucket (%d)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);
    if(nentries > cache->snperbucket)
	ajFatal("SecReadBucket: Bucket too full\n");
    

    GBT_BUCKOVERFLOW(buf,&overflow);

    AJNEW0(bucket);
    bucket->NodeType = nodetype;
    bucket->Nentries = nentries;
    bucket->Overflow = overflow;
    
    AJCNEW0(bucket->keylen, nentries+1);
    AJCNEW0(bucket->ids, nentries+1);

    
    kptr  = PBT_BUCKKEYLEN(buf);
    codeptr = kptr + (nentries * sizeof(ajint));

    for(i=0;i<nentries;++i)
    {
	BT_GETAJINT(kptr,&len);
	if((codeptr-buf+1) + len > cache->pagesize)	/* overflow */
	{
	    ajDebug("SecReadBucket: Overflow\n");
	    page  = ajBtreeCacheRead(cache,overflow);
	    buf = page->buf;
	    GBT_BUCKNODETYPE(buf,&nodetype);
	    if(nodetype != BT_SECBUCKET)
		ajFatal("SecReadBucket: NodeType mismatch. Not secondary "
			"bucket (%d)",nodetype);
	    GBT_BUCKOVERFLOW(buf,&overflow);
	    /* overflow bucket ids start at the keylen position */
	    codeptr = PBT_BUCKKEYLEN(buf);
	}

	bucket->ids[i] = ajStrNew();

	/* Fill ID objects */
	ajStrAssC(&bucket->ids[i],(const char *)codeptr);
	codeptr += (strlen((const char *)codeptr) + 1);

	kptr += sizeof(ajint);
    }

    lpage->dirty = dirtysave;
    
    return bucket;
}




/* @funcstatic btreeWriteSecBucket *******************************************
**
** Write primary keyword index bucket object to the cache given a disc page
** number
**
** @param [u] cache [AjPBtcache] cache
** @param [r] bucket [const AjPSecBucket] bucket
** @param [r] pageno [ajlong] page number
**
** @return [void]
** @@
******************************************************************************/

static void btreeWriteSecBucket(AjPBtcache cache, const AjPSecBucket bucket,
				ajlong pageno)
{
    AjPBtpage page  = NULL;
    AjPBtpage lpage = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;

    ajint  v   = 0;
    ajint  i   = 0;
    ajint  len = 0;
    ajlong lv  = 0L;

    AjPStr sec = NULL;
    ajint  nentries = 0;
    ajlong overflow = 0L;
    unsigned char *keyptr = NULL;
    unsigned char *lptr   = NULL;
    ajlong   pno = 0L;
    
    ajDebug("In btreeWriteSecBucket\n");

    if(pageno == cache->totsize)	/* Create a new page */
    {
	pno = pageno;
	page = ajBtreeCacheWrite(cache,pageno);
	page->pageno = cache->totsize;
	cache->totsize += cache->pagesize;
	buf = page->buf;
	overflow = 0L;
	lv = overflow;
	SBT_BUCKOVERFLOW(buf,lv);
    }
    else
    {
	page = ajBtreeCacheRead(cache,pageno);
	buf = page->buf;
	GBT_BUCKOVERFLOW(buf,&overflow);
    }

    v = BT_SECBUCKET;
    SBT_BUCKNODETYPE(buf,v);

    lbuf = buf;
    page->dirty = BT_LOCK;
    lpage = page;

    nentries = bucket->Nentries;
    v = nentries;
    SBT_BUCKNENTRIES(buf,v);

    /* Write out key lengths */
    keyptr = PBT_BUCKKEYLEN(lbuf);
    for(i=0;i<nentries;++i)
    {
	if((keyptr-lbuf+1)+sizeof(ajint) > cache->pagesize)
	    ajFatal("SecBucketWrite: Bucket cannot hold more than %d keys",
		    i-1);

	sec = bucket->ids[i];
	/* Need to alter this if bucket primary keyword structure changes */
	len = BT_BUCKSECLEN(sec);
        v = len;
	BT_SETAJINT(keyptr,v);
	keyptr += sizeof(ajint);
    }


    /* Write out IDs using overflow if necessary */
    lptr = keyptr;
    for(i=0;i<nentries;++i)
    {
	sec = bucket->ids[i];
	len = BT_BUCKSECLEN(sec);
	if((lptr-buf+1)+len > cache->pagesize) /* overflow */
	{
    	    ajDebug("WriteSecBucket: Overflow\n");
	    if(!overflow)		/* No overflow buckets yet */
	    {
		pno = cache->totsize;
                lv = pno;
		SBT_BUCKOVERFLOW(buf,lv);
		page = ajBtreeCacheWrite(cache,pno);
		page->pageno = cache->totsize;
		cache->totsize += cache->pagesize;
		buf = page->buf;
		v = BT_SECBUCKET;
		SBT_BUCKNODETYPE(buf,v);
		v = 0;
		SBT_BUCKNENTRIES(buf,v);
		lv = 0L;
		SBT_BUCKOVERFLOW(buf,lv);
	    }
	    else
	    {
		page = ajBtreeCacheRead(cache,overflow);
		buf  = page->buf;
		GBT_BUCKOVERFLOW(buf,&overflow);
	    }

	    page->dirty = BT_DIRTY;
	    lptr = PBT_BUCKKEYLEN(buf);	    
	}
	
	sprintf((char *)lptr,"%s",ajStrStr(sec));
	lptr += (ajStrLen(sec) + 1);
    }

    lv = 0L;
    SBT_BUCKOVERFLOW(buf,lv);
    
    lpage->dirty = BT_DIRTY;

    return;
}




/* @func ajBtreeSecCacheNewC ************************************************
**
** Open a b+tree index file and initialise a cache object for keyword index
**
** @param [r] file [const char *] name of file
** @param [r] ext [const char *] extension of file
** @param [r] idir [const char *] index file directory
** @param [r] mode [const char *] opening mode
** @param [r] pagesize [ajint] pagesize
** @param [r] order [ajint] Tree order
** @param [r] fill [ajint] Number of entries per bucket
** @param [r] level [ajint] level of tree
** @param [r] cachesize [ajint] size of cache
** @param [r] sorder [ajint] order of secondary tree
** @param [r] slevel [ajint] level of secondary tree
** @param [r] sfill [ajint] Number of entries per secondary bucket
** @param [r] count [ajint] Number of entries in the index
**
** @return [AjPBtcache] initialised disc block cache structure
** @@
******************************************************************************/

AjPBtcache ajBtreeSecCacheNewC(const char *file, const char *ext,
			       const char *idir, const char *mode,
			       ajint pagesize, ajint order, ajint fill,
			       ajint level, ajint cachesize,
			       ajint sorder, ajint slevel, ajint sfill,
			       ajint count)
{
    FILE *fp;
    AjPBtcache cache = NULL;
#if defined (HAVE64) && !defined(_OSF_SOURCE) && !defined(_AIX) && !defined(__hpux) && !defined(__ppc__) && !defined(__FreeBSD__)
    struct stat64 buf;
#else
    struct stat buf;
#endif
    ajlong filelen = 0L;

    AjPStr fn = NULL;

    fn = ajStrNew();
    ajFmtPrintS(&fn,"%s/%s.%s",idir,file,ext);
    
    fp = fopen(fn->Ptr,mode);
    if(!fp)
	return NULL;


#if defined (HAVE64) && !defined(_OSF_SOURCE) && !defined(_AIX) && !defined(__hpux) && !defined(__ppc__) && !defined(__FreeBSD__)
    if(!stat64(file, &buf))
#else
    if(!stat(file, &buf))
#endif
	filelen = buf.st_size;

    AJNEW0(cache);

    cache->listLength = 0;

    cache->lru   = NULL;
    cache->mru   = NULL;
    cache->count = 0L;
    cache->fp    = fp;
    
    cache->replace = ajStrNew();

    if(pagesize>0)
	cache->pagesize = pagesize;
    else
	cache->pagesize = BT_PAGESIZE;

    cache->totsize   = 0L;
    cache->level     = level;

    cache->order      = order;
    cache->nperbucket = fill;
    cache->totsize    = filelen;
    cache->cachesize  = cachesize;

    cache->sorder = sorder;
    cache->slevel = slevel;
    cache->snperbucket = sfill;
    cache->count = count;
    
    ajStrDel(&fn);
    
    return cache;
}




/* @funcstatic btreeSplitSecLeaf *********************************************
**
** Split a leaf and propagate up if necessary
**
** @param [u] cache [AjPBtcache] cache
** @param [u] spage [AjPBtpage] page
**
** @return [AjPBtpage] pointer to a parent page
** @@
******************************************************************************/

static AjPBtpage btreeSplitSecLeaf(AjPBtcache cache, AjPBtpage spage)
{
    ajint nkeys     = 0;
    ajint order     = 0;
    ajint totalkeys = 0;
    ajint bentries  = 0;
    ajint keylimit  = 0;
    ajint nodetype  = 0;

    ajint rootnodetype = 0;
    
    ajint i;
    ajint j;
    
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage page  = NULL;
    
    AjPStr mediankey  = NULL;
    ajlong mediangtr  = 0L;
    ajlong medianless = 0L;

    AjPStr bid = NULL;

    unsigned char *buf  = NULL;
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;

    AjPList idlist = NULL;

    AjPSecBucket *buckets = NULL;
    AjPSecBucket cbucket  = NULL;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint keypos = 0;
    ajint lno    = 0;
    ajint rno    = 0;

    ajint bucketlimit   = 0;
    ajint maxnperbucket = 0;
    ajint nperbucket    = 0;
    ajint bucketn       = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    
    ajlong lblockno = 0L;
    ajlong rblockno = 0L;
    ajlong prev     = 0L;
    ajlong overflow = 0L;

    ajlong zero = 0L;
    ajlong join = 0L;
    
    ajlong lv = 0L;
    ajint  v  = 0;
    
    ajDebug("In btreeSplitSecLeaf\n");

    order = cache->sorder;
    nperbucket = cache->snperbucket;

    mediankey = ajStrNew();
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    buf = spage->buf;
    lbuf = buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&rootnodetype);

    if(rootnodetype == BT_ROOT)
    {
	lblockno = cache->totsize;
	lpage = ajBtreeCacheWrite(cache,lblockno);
	lpage->pageno = cache->totsize;
	cache->totsize += cache->pagesize;
	lbuf = lpage->buf;
	lv = cache->secrootblock;
	SBT_PREV(lbuf,lv);
    }
    else
    {
	lblockno = spage->pageno;
	lpage = spage;
    }


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;
    rbuf = rpage->buf;


    if(rootnodetype == BT_ROOT)
    {
	lv = zero;
	SBT_RIGHT(rbuf,lv);
	lv = zero;
	SBT_LEFT(lbuf,lv);
    }
    else
    {
	GBT_RIGHT(lbuf,&join);
	lv = join;
	SBT_RIGHT(rbuf,lv);
    }
    lv = lblockno;
    SBT_LEFT(rbuf,lv);
    lv = rblockno;
    SBT_RIGHT(lbuf,lv);


    btreeGetKeys(cache,buf,&karray,&parray);


    keylimit = nkeys+1;
    AJCNEW0(buckets,keylimit);
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadSecBucket(cache,parray[i]);

    idlist = ajListNew();
    for(i=0;i<keylimit;++i)
    {
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->ids[j]);
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->ids);
	AJFREE(buckets[i]);
    }



    ajListSort(idlist,btreeKeywordIdCompare);
    AJFREE(buckets);


    totalkeys = ajListLength(idlist);

    keypos = totalkeys / 2;

    lno = keypos;
    rno = totalkeys - lno;

    maxnperbucket = nperbucket >> 1;
    if(!maxnperbucket)
	++maxnperbucket;

    cbucket = btreeSecBucketNew(maxnperbucket);

    bucketn = lno / maxnperbucket;
    if(lno % maxnperbucket)
	++bucketn;
    bucketlimit = bucketn - 1;


    totkeylen = 0;
    count = 0;
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<maxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    

	    ajStrAssS(&cbucket->ids[j],bid);

	    cbucket->keylen[j] = BT_BUCKSECLEN(bid);
	    ++count;
	    ++cbucket->Nentries;
	    ajStrDel(&bid);
	}
	ajListPeek(idlist,(void **)&bid);
	
	ajStrAssS(&karray[i],bid);
	totkeylen += ajStrLen(bid);

	if(!parray[i])
	    parray[i] = cache->totsize;
	btreeWriteSecBucket(cache,cbucket,parray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;
    while(count != lno)
    {
	ajListPop(idlist,(void **)&bid);

	ajStrAssS(&cbucket->ids[j],bid);
	++j;
	++count;


	++cbucket->Nentries;
	ajStrDel(&bid);
    }

    if(!parray[i])
	parray[i] = cache->totsize;
    btreeWriteSecBucket(cache,cbucket,parray[i]);

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lpage->dirty = BT_DIRTY;
    btreeWriteNode(cache,lpage,karray,parray,nkeys);

    ajListPeek(idlist,(void **)&bid);
    ajStrAssS(&mediankey,bid);

    totkeylen = 0;
    bucketn = rno / maxnperbucket;
    if(rno % maxnperbucket)
	++bucketn;
    bucketlimit = bucketn - 1;

    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Nentries = 0;
	for(j=0;j<maxnperbucket;++j)
	{
	    ajListPop(idlist,(void **)&bid);
	    

	    ajStrAssS(&cbucket->ids[j],bid);

	    cbucket->keylen[j] = BT_BUCKSECLEN(bid);
	    ++cbucket->Nentries;
	    ajStrDel(&bid);
	}

	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&karray[i],bid);
	totkeylen += ajStrLen(bid);

	parray[i] = cache->totsize;
	btreeWriteSecBucket(cache,cbucket,parray[i]);
    }

    cbucket->Nentries = 0;

    j = 0;
    while(ajListPop(idlist,(void**)&bid))
    {
	ajStrAssS(&cbucket->ids[j],bid);
	++j;


	++cbucket->Nentries;
	ajStrDel(&bid);
    }
    
    parray[i] = cache->totsize;
    btreeWriteSecBucket(cache,cbucket,parray[i]);

    nkeys = bucketn - 1;

    v = nkeys;
    SBT_NKEYS(rbuf,v);
    v = totkeylen;
    SBT_TOTLEN(rbuf,v);
    nodetype = BT_LEAF;
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    
    btreeWriteNode(cache,rpage,karray,parray,nkeys);
    rpage->dirty = BT_DIRTY;

    cbucket->Nentries = maxnperbucket;
    btreeSecBucketDel(&cbucket);
    ajListDel(&idlist);



    medianless = lblockno;
    mediangtr  = rblockno;


    if(rootnodetype == BT_ROOT)
    {
	ajStrAssS(&karray[0],mediankey);
	parray[0]=lblockno;
	parray[1]=rblockno;
	nkeys = 1;
	btreeWriteNode(cache,spage,karray,parray,nkeys);	

	++cache->slevel;
	lv = cache->slevel;
	SBT_RIGHT(buf,lv);
	spage->dirty = BT_LOCK;
	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	ajStrDel(&mediankey);
	return spage;
    }


    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    page = ajBtreeCacheRead(cache,prev);
    btreeInsertKeySec(cache,page,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    page = ajBtreeCacheRead(cache,prev);

    return page;
}




/* @funcstatic btreeKeywordIdCompare *****************************************
**
** Comparison function for ajListSort
**
** @param [r] a [const void*] ID 1
** @param [r] b [const void*] ID 2
**
** @return [ajint] 0 = bases match
** @@
******************************************************************************/

static ajint btreeKeywordIdCompare(const void *a, const void *b)
{
    return strcmp((*(AjPStr*)a)->Ptr,
		  (*(AjPStr*)b)->Ptr);
}




/* @funcstatic btreeSecBucketNew *********************************************
**
** Construct a primary keyword secondary bucket object
**
** @param [r] n [ajint] Number of IDs
**
** @return [AjPSecBucket] initialised disc block cache structure
** @@
******************************************************************************/

static AjPSecBucket btreeSecBucketNew(ajint n)
{
    AjPSecBucket bucket = NULL;
    ajint i;

    ajDebug("In btreeSecBucketNew\n");
    
    AJNEW0(bucket);

    if(n)
    {
	AJCNEW0(bucket->ids,n);
	AJCNEW0(bucket->keylen,n);
    }
    
    for(i=0;i<n;++i)
	bucket->ids[i] = ajStrNew();

    bucket->NodeType = BT_SECBUCKET;
    bucket->Nentries = n;
    bucket->Overflow = 0L;
    
    return bucket;
}




/* @funcstatic btreeSecBucketDel *********************************************
**
** Delete a keyword primary bucket object
**
** @param [w] thys [AjPSecBucket*] bucket
**
** @return [void]
** @@
******************************************************************************/

static void btreeSecBucketDel(AjPSecBucket *thys)
{
    AjPSecBucket pthis = NULL;
    ajint n;
    ajint i;
    
    ajDebug("In btreeSecBucketDel\n");

    if(!thys || !*thys)
	return;

    pthis = *thys;
    n = pthis->Nentries;

    for(i=0;i<n;++i)
	ajStrDel(&pthis->ids[i]);
    
    AJFREE(pthis->keylen);
    AJFREE(pthis->ids);
    
    AJFREE(pthis);

    *thys = NULL;

    return;
}




/* @func ajBtreeInsertSecId *********************************************
**
** Insert a secondary ID into the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] id [const AjPStr] Id
**
** @return [void] pointer to a page
** @@
******************************************************************************/

void ajBtreeInsertSecId(AjPBtcache cache, const AjPStr id)
{
    AjPBtpage spage  = NULL;
    AjPBtpage parent = NULL;
    AjPStr key       = NULL; /* Don't really need this ... use id instead */
    const char *ckey = NULL;

    AjPSecBucket lbucket = NULL;
    AjPSecBucket rbucket = NULL;

    ajlong lblockno = 0L;
    ajlong rblockno = 0L;
    ajlong blockno  = 0L;
    ajlong shift    = 0L;

    ajint nkeys = 0;
    ajint order = 0;

    ajint nodetype = 0;
    
    unsigned char *buf = NULL;
 
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    ajint i;
    ajint n;

    AjBool exists  = ajFalse;

    
    ajDebug("In ajBtreeInsertSecId\n");


    /* Only insert an ID if it doesn't exist */
    exists = ajBtreeSecFromId(cache,MAJSTRSTR(id));
    if(exists)
    {
	ajDebug("ID already in tree\n");
	return;
    }
    
    key = ajStrNew();
    

    ajStrAssS(&key,id);
    if(!ajStrLen(key))
    {
	ajDebug("ID is NULL\n");
	ajStrDel(&key);
	return;
    }

    ckey = ajStrStr(key);
    ajDebug("ckey = %s\n",ckey);
    spage = ajBtreeSecFindInsert(cache,ckey);
    buf = spage->buf;

    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    order = cache->sorder;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    ajDebug("NKEYS=%d\n",nkeys);
    
    if(!nkeys)
    {
	lbucket  = btreeSecBucketNew(0);
	rbucket  = btreeSecBucketNew(0);

	lblockno = cache->totsize;
	btreeWriteSecBucket(cache,lbucket,lblockno);

	rblockno = cache->totsize;
	btreeWriteSecBucket(cache,rbucket,rblockno);	

	parray[0] = lblockno;
	parray[1] = rblockno;
	ajStrAssS(karray,key);
	
	btreeWriteNode(cache,spage,karray,parray,1);

	btreeSecBucketDel(&lbucket);
	btreeSecBucketDel(&rbucket);

	btreeAddToSecBucket(cache,rblockno,key);

	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	ajStrDel(&key);
	return;
    }
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;
    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    if(nodetype != BT_ROOT)
	if((shift = btreeInsertShiftSec(cache,&spage,key->Ptr)))
	    blockno = shift;

    buf = spage->buf;

    n = btreeNumInSecBucket(cache,blockno);

    if(n == cache->snperbucket)
    {
	if(btreeReorderSecBuckets(cache,spage))
	{
	    GBT_NKEYS(buf,&nkeys);	    
	    btreeGetKeys(cache,buf,&karray,&parray);

	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;

	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];
	}
	else
	{
	    parent = btreeSplitSecLeaf(cache,spage);
	    
	    spage = ajBtreeSecFindInsert(cache,ckey);
	    buf = spage->buf;

	    btreeGetKeys(cache,buf,&karray,&parray);

	    GBT_NKEYS(buf,&nkeys);
	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;

	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];

/*
	    spage = ajBtreeCacheRead(cache,blockno);
	    buf = spage->buf;
	    btreeGetKeys(cache,buf,&karray,&parray);
	    GBT_NKEYS(buf,&nkeys);
	    i=0;
	    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr)>=0)
		++i;
	    if(i==nkeys)
	    {
		if(strcmp(key->Ptr,karray[i-1]->Ptr)<0)
		    blockno = parray[i-1];
		else
		    blockno = parray[i];
	    }
	    else
		blockno = parray[i];
*/
	}
    }


    btreeAddToSecBucket(cache,blockno,key);

    ++cache->count;

    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);
    ajStrDel(&key);

    return;
}




/* @func ajBtreeSecFromId **********************************************
**
** See whether ID already exists in the tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char *] key
**
** @return [AjBool] true if ID already added
** @@
******************************************************************************/

AjBool ajBtreeSecFromId(AjPBtcache cache, const char *key)
{
    AjPBtpage page      = NULL;
    AjPSecBucket bucket = NULL;
    
    AjPStr *karray = NULL;
    ajlong *parray = NULL;
    
    unsigned char *buf = NULL;

    ajint nentries = 0;
    ajint nkeys    = 0;
    ajint order    = 0;
    
    ajint i;
    
    ajlong blockno = 0L;
    AjBool found   = ajFalse;

    ajDebug("In ajBtreeSecFromId %d\n",cache->count);

    if(!cache->count)
	return ajFalse;
    
    page = ajBtreeSecFindInsert(cache,key);
    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    if(!nkeys)
	return ajFalse;
    
    order = cache->sorder;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    btreeGetKeys(cache,buf,&karray,&parray);

    i=0;
    while(i!=nkeys && strcmp(key,karray[i]->Ptr)>=0)
	++i;
    
    if(i==nkeys)
    {
	if(strcmp(key,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    bucket = btreeReadSecBucket(cache,blockno);
    
    nentries = bucket->Nentries;

    found = ajFalse;

    for(i=0;i<nentries;++i)
	if(!strcmp(key,bucket->ids[i]->Ptr))
	{
	    found = ajTrue;
	    break;
	}

    btreeSecBucketDel(&bucket);
    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return found;
}




/* @func ajBtreeSecFindInsert ***********************************************
**
** Find the node that should contain a new key for insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [r] key [const char*] key to search for 
**
** @return [AjPBtpage] leaf node where item should be inserted
** @@
******************************************************************************/

AjPBtpage ajBtreeSecFindInsert(AjPBtcache cache, const char *key)
{
    AjPBtpage root = NULL;
    AjPBtpage ret  = NULL;

    ajDebug("In ajBtreeSecFindInsert\n");

    /* The root node should always be in the cache (BT_LOCKed) */
    root = btreeCacheLocate(cache,cache->secrootblock);

    /* ajDebug("cache->slevel = %d root=%d\n",cache->slevel,(ajint)root); */
    

    if(!cache->slevel)
	return root;
    
    ret = btreeFindINode(cache,root,key);

    return ret;
}




/* @funcstatic btreeNumInSecBucket *******************************************
**
** Return number of entries in a secondary id bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number
**
** @return [ajint] Number of entries in bucket
** @@
******************************************************************************/

static ajint btreeNumInSecBucket(AjPBtcache cache, ajlong pageno)
{
    unsigned char *buf = NULL;

    AjPBtpage page = NULL;
    ajint nodetype = 0;
    ajint nentries = 0;
    
    ajDebug("In btreeNumInPriBucket\n");

        
    if(pageno == cache->secrootblock)
	ajFatal("NumInSecBucket: Attempt to read bucket from root page\n");


    page  = ajBtreeCacheRead(cache,pageno);

    buf = page->buf;

    GBT_BUCKNODETYPE(buf,&nodetype);
    if(nodetype != BT_SECBUCKET)
	ajFatal("SecReadBucket: NodeType mismatch. Not secondary bucket (%d)",
		nodetype);
    
    GBT_BUCKNENTRIES(buf,&nentries);

    return nentries;
}




/* @funcstatic btreeAddToSecBucket *******************************************
**
** Add a keyword ID to a primary bucket
** Only called if there is room in the bucket
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pageno [ajlong] page number of bucket
** @param [r] id [const AjPStr] ID
**
** @return [void]
** @@
******************************************************************************/

static void btreeAddToSecBucket(AjPBtcache cache, ajlong pageno,
				const AjPStr id)
{
    AjPSecBucket bucket = NULL;
    ajint nentries;
    
    ajDebug("In btreeAddToSecBucket\n");

    bucket   = btreeReadSecBucket(cache,pageno);
    nentries = bucket->Nentries;


    /* Reading a bucket always gives one extra ID position */
    bucket->ids[nentries] = ajStrNew();

    ajStrAssS(&bucket->ids[nentries],id);

    ++bucket->Nentries;

    btreeWriteSecBucket(cache,bucket,pageno);

    btreeSecBucketDel(&bucket);
    
    return;
}




/* @funcstatic btreeReorderSecBuckets *****************************************
**
** Re-order secondary id leaf buckets
** Must only be called if one of the buckets is full
**
** @param [u] cache [AjPBtcache] cache
** @param [u] leaf [AjPBtpage] leaf page
**
** @return [AjBool] true if reorder was successful i.e. leaf not full
** @@
******************************************************************************/
static AjBool btreeReorderSecBuckets(AjPBtcache cache, AjPBtpage leaf)
{
    ajint nkeys = 0;
    AjPSecBucket *buckets = NULL;
    AjPSecBucket cbucket  = NULL;
    unsigned char *lbuf   = NULL;

    AjPStr *keys = NULL;
    ajlong *ptrs = NULL;

    ajlong *overflows = NULL;
    
    ajint i = 0;
    ajint j = 0;
    
    ajint order;
    ajint bentries      = 0;
    ajint totalkeys     = 0;
    ajint nperbucket    = 0;
    ajint maxnperbucket = 0;
    ajint count         = 0;
    ajint totkeylen     = 0;
    ajint keylimit      = 0;
    ajint bucketn       = 0;
    ajint bucketlimit   = 0;
    ajint nodetype      = 0;
    
    AjPList idlist  = NULL;
    ajint dirtysave = 0;
    AjPStr bid      = NULL;


    ajint v = 0;
    
    ajDebug("In btreeReorderSecBuckets\n");

    dirtysave = leaf->dirty;

    leaf->dirty = BT_LOCK;
    lbuf = leaf->buf;

    GBT_NODETYPE(lbuf,&nodetype);

    order = cache->sorder;
    nperbucket = cache->snperbucket;
    

    /* Read keys/ptrs */
    AJCNEW0(keys,order);
    AJCNEW0(ptrs,order);
    AJCNEW0(overflows,order);
    
    for(i=0;i<order;++i)
	keys[i] = ajStrNew();
    btreeGetKeys(cache,lbuf,&keys,&ptrs);

    GBT_NKEYS(lbuf,&nkeys);


    if(!nkeys)
	ajFatal("SecBucketReorder: Attempt to reorder empty leaf");

    for(i=0;i<nkeys;++i)
	totalkeys += btreeNumInSecBucket(cache,ptrs[i]);
    totalkeys += btreeNumInSecBucket(cache,ptrs[i]);

    /* Set the number of entries per bucket to approximately half full */
    maxnperbucket = nperbucket >> 1;

    if(!maxnperbucket)
	++maxnperbucket;

    /* Work out the number of new buckets needed */
    bucketn = (totalkeys / maxnperbucket);
    if(totalkeys % maxnperbucket)
	++bucketn;
    
    if(bucketn > order)
    {
	for(i=0;i<order;++i)
	    ajStrDel(&keys[i]);
	AJFREE(keys);
	AJFREE(ptrs);
	AJFREE(overflows);
	
	leaf->dirty = dirtysave;
	return ajFalse;
    }
    

    /* Read buckets */
    AJCNEW0(buckets,nkeys+1);
    keylimit = nkeys + 1;

    
    for(i=0;i<keylimit;++i)
	buckets[i] = btreeReadSecBucket(cache,ptrs[i]);


    /* Read IDs from all buckets and push to list and sort (increasing kw) */
    idlist  = ajListNew();
    
    for(i=0;i<keylimit;++i)
    {
	overflows[i] = buckets[i]->Overflow;
	bentries = buckets[i]->Nentries;
	for(j=0;j<bentries;++j)
	    ajListPush(idlist,(void *)buckets[i]->ids[j]);
	
	AJFREE(buckets[i]->keylen);
	AJFREE(buckets[i]->ids);
	AJFREE(buckets[i]);
    }
    ajListSort(idlist,btreeKeywordIdCompare);
    AJFREE(buckets);

    cbucket = btreeSecBucketNew(maxnperbucket);
    bucketlimit = bucketn - 1;
    
    for(i=0;i<bucketlimit;++i)
    {
	cbucket->Overflow = overflows[i];
	cbucket->Nentries = 0;

	count = 0;
	while(count!=maxnperbucket)
	{
	    ajListPop(idlist,(void **)&bid);
	    
	    ajStrAssS(&cbucket->ids[count],bid);

	    cbucket->keylen[count] = BT_BUCKSECLEN(bid);
	    ++cbucket->Nentries;
	    ++count;
	    ajStrDel(&bid);
	}


	ajListPeek(idlist,(void **)&bid);
	ajStrAssS(&keys[i],bid);

	totkeylen += ajStrLen(bid);

	if(!ptrs[i])
	    ptrs[i] = cache->totsize;
	btreeWriteSecBucket(cache,cbucket,ptrs[i]);
    }


    /* Deal with greater-than bucket */

    cbucket->Overflow = overflows[i];
    cbucket->Nentries = 0;

    count = 0;
    while(ajListPop(idlist,(void **)&bid))
    {
	ajStrAssS(&cbucket->ids[count],bid);

	++cbucket->Nentries;
	++count;
	ajStrDel(&bid);
    }
    
    
    if(!ptrs[i])
	ptrs[i] = cache->totsize;
    btreeWriteSecBucket(cache,cbucket,ptrs[i]);

    cbucket->Nentries = maxnperbucket;
    btreeSecBucketDel(&cbucket);

    /* Now write out a modified leaf with new keys/ptrs */

    nkeys = bucketn - 1;
    v = nkeys;
    SBT_NKEYS(lbuf,v);
    v = totkeylen;
    SBT_TOTLEN(lbuf,v);

    btreeWriteNode(cache,leaf,keys,ptrs,nkeys);

    leaf->dirty = BT_DIRTY;
    if(nodetype == BT_ROOT)
	leaf->dirty = BT_LOCK;
    
    for(i=0;i<order;++i)
	ajStrDel(&keys[i]);
    AJFREE(keys);
    AJFREE(ptrs);
    AJFREE(overflows);
    

    btreeSecBucketDel(&cbucket);
    ajListDel(&idlist);

    return ajTrue;
}



#if 0
/* @funcstatic btreeInsertIdOnly *****************************************
**
** Add only a secondary ID: the primary keyword already exists
**
** @param [u] cache [AjPBtcache] cache
** @param [r] pri [const AjPBtPri] keyword/id
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertIdOnly(AjPBtcache cache, const AjPBtPri pri)
{
    unsigned char *buf;
    AjPBtpage page = NULL;

    ajlong right = 0L;

    ajDebug("In btreeInsertIdOnly\n");
    
    if(!pri->treeblock)
    {
	fprintf(stderr,"btreeInsertIdOnly: root page doesn't exist\n");
	exit(-1);
    }
    
    cache->secrootblock = pri->treeblock;
    page = ajBtreeCacheWrite(cache,cache->secrootblock);
    page->dirty = BT_LOCK;
    buf = page->buf;
    GBT_RIGHT(buf, &right);
    cache->slevel = (ajint) right;

    ajBtreeInsertSecId(cache, pri->id);

    right = (ajlong) cache->slevel;
    SBT_RIGHT(buf, right);
    page->dirty = BT_DIRTY;

    return;
}
#endif




/* @funcstatic btreeSplitRootSec *****************************************
**
** Split s secondary root node
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

static void btreeSplitRootSec(AjPBtcache cache)
{
    AjPBtpage rootpage = NULL;
    AjPBtpage rpage    = NULL;
    AjPBtpage lpage    = NULL;
    AjPBtpage tpage    = NULL;

    AjPStr *karray  = NULL;
    AjPStr *tkarray = NULL;
    ajlong *parray  = NULL;
    ajlong *tparray = NULL;

    ajint order  = 0;
    ajint nkeys  = 0;
    ajint keypos = 0;
    
    ajlong rblockno = 0L;
    ajlong lblockno = 0L;

    ajlong right;
    
    AjPStr key = NULL;
    ajint  i;

    unsigned char *rootbuf = NULL;
    unsigned char *rbuf    = NULL;
    unsigned char *lbuf    = NULL;
    unsigned char *tbuf    = NULL;
    
    ajint nodetype  = 0;
    ajlong overflow = 0L;
    ajlong zero     = 0L;
    ajint totlen    = 0;
    ajint rkeyno    = 0;
    ajint n         = 0;

    ajlong lv = 0L;
    ajint  v  = 0;
    
    
    ajDebug("In btreeSplitRootSec\n");

    order = cache->sorder;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    AJCNEW0(tkarray,order);
    AJCNEW0(tparray,order);
    key = ajStrNew();

    for(i=0;i<order;++i)
    {
	karray[i]  = ajStrNew();
	tkarray[i] = ajStrNew();
    }
    

    rootpage = btreeCacheLocate(cache,cache->secrootblock);
    rootbuf = rootpage->buf;

    nkeys = order - 1;

    keypos = nkeys / 2;
    if(!(nkeys % 2))
	--keypos;


    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;

    lblockno = cache->totsize;
    lpage = ajBtreeCacheWrite(cache,lblockno);
    lpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;

    lv = rblockno;
    SBT_BLOCKNUMBER(rpage->buf,lv);
    lv = lblockno;
    SBT_BLOCKNUMBER(lpage->buf,lv);

    if(!cache->slevel)
    {
	printf("Shouldn't get here \n");
	exit(0);
	lv = zero;
	SBT_LEFT(lpage->buf,lv);
	lv = rblockno;
	SBT_RIGHT(lpage->buf,lv);
	lv = lblockno;
	SBT_LEFT(rpage->buf,lv);
	lv = zero;
	SBT_RIGHT(rpage->buf,lv);
    }

    btreeGetKeys(cache,rootbuf,&karray,&parray);

    /* Get key for root node and write new root node */
    ajStrAssS(&tkarray[0],karray[keypos]);
    tparray[0] = lblockno;
    tparray[1] = rblockno;
    

    n = 1;
    v = n;
    SBT_NKEYS(rootbuf,v);
    btreeWriteNode(cache,rootpage,tkarray,tparray,1);
    right = (ajlong)(cache->slevel + 1);
    SBT_RIGHT(rootbuf,right);

    rootpage->dirty = BT_LOCK;

    rbuf = rpage->buf;
    lbuf = lpage->buf;
    
    if(cache->slevel)
	nodetype = BT_INTERNAL;
    else
	nodetype = BT_LEAF;

    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    v = nodetype;
    SBT_NODETYPE(lbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);
    lv = cache->secrootblock;
    SBT_PREV(rbuf,lv);
    lv = overflow;
    SBT_OVERFLOW(lbuf,lv);
    lv = cache->secrootblock;
    SBT_PREV(lbuf,lv);

    totlen = 0;
    for(i=0;i<keypos;++i)
    {
	ajStrAssS(&tkarray[i],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i] = parray[i];
    }
    tparray[i] = parray[i];
    v = totlen;
    SBT_TOTLEN(lbuf,v);
    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNode(cache,lpage,tkarray,tparray,i);

    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }

    totlen = 0;
    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }
    tparray[i-(keypos+1)] = parray[i];
    v = totlen;
    SBT_TOTLEN(rbuf,v);
    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);

    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ++cache->slevel;

    for(i=0;i<order;++i)
    {
	ajStrDel(&karray[i]);
	ajStrDel(&tkarray[i]);
    }


    AJFREE(tkarray);
    AJFREE(tparray);
    AJFREE(karray);
    AJFREE(parray);
    ajStrDel(&key);
    
    return;
}




/* @funcstatic btreeInsertKeySec *****************************************
**
** Insert a secpndary key into a potentially full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajlong] less-than pointer
** @param [r] greater [ajlong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertKeySec(AjPBtcache cache, AjPBtpage page,
			      const AjPStr key, ajlong less, ajlong greater)
{
    unsigned char *lbuf = NULL;
    unsigned char *rbuf = NULL;
    unsigned char *tbuf = NULL;

    AjPStr *karray  = NULL;
    ajlong *parray  = NULL;
    AjPStr *tkarray = NULL;
    ajlong *tparray = NULL;

    ajint nkeys  = 0;
    ajint order  = 0;
    ajint keypos = 0;
    ajint rkeyno = 0;
    
    ajint i = 0;
    ajint n = 0;
    
    ajint nodetype = 0;
    AjPBtpage ipage = NULL;
    AjPBtpage lpage = NULL;
    AjPBtpage rpage = NULL;
    AjPBtpage tpage = NULL;

    ajlong blockno  = 0L;
    ajlong rblockno = 0L;
    ajlong lblockno = 0L;

    AjPStr mediankey  = NULL;
    ajlong medianless = 0L;
    ajlong mediangtr  = 0L;
    ajlong overflow   = 0L;
    ajlong prev       = 0L;
    ajint  totlen     = 0;
    
    ajlong lv = 0L;
    ajint  v  = 0;
    
    ajDebug("In btreeInsertKeySec\n");

    if(!btreeNodeIsFull(cache,page))
    {
	btreeInsertNonFullSec(cache,page,key,less,greater);
	return;
    }
    
    order = cache->sorder;
    lbuf = page->buf;
    GBT_NODETYPE(lbuf,&nodetype);

    if(nodetype == BT_ROOT)
    {
	AJCNEW0(karray,order);
	AJCNEW0(parray,order);
	for(i=0;i<order;++i)
	    karray[i] = ajStrNew();
	btreeSplitRootSec(cache);

	btreeGetKeys(cache,lbuf,&karray,&parray);

	if(strcmp(key->Ptr,karray[0]->Ptr)<0)
	    blockno = parray[0];
	else
	    blockno = parray[1];
	ipage = ajBtreeCacheRead(cache,blockno);
	btreeInsertNonFullSec(cache,ipage,key,less,greater);

	for(i=0;i<order;++i)
	    ajStrDel(&karray[i]);
	AJFREE(karray);
	AJFREE(parray);
	return;
    }


    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    AJCNEW0(tkarray,order);
    AJCNEW0(tparray,order);
    mediankey = ajStrNew();
    
    for(i=0;i<order;++i)
    {
	tkarray[i] = ajStrNew();
	karray[i]  = ajStrNew();
    }
    
    lpage = page;
    lbuf = lpage->buf;
    
    btreeGetKeys(cache,lbuf,&karray,&parray);

    GBT_BLOCKNUMBER(lbuf,&lblockno);
    rblockno = cache->totsize;
    rpage = ajBtreeCacheWrite(cache,rblockno);
    rpage->pageno = cache->totsize;
    cache->totsize += cache->pagesize;
    rbuf = rpage->buf;
    lv = rblockno;
    SBT_BLOCKNUMBER(rbuf,lv);

    
    GBT_PREV(lbuf,&prev);
    lv = prev;
    SBT_PREV(rbuf,lv);

    nkeys = order - 1;
    keypos = nkeys / 2;
    if(!(nkeys % 2))
	--keypos;

    ajStrAssS(&mediankey,karray[keypos]);
    medianless = lblockno;
    mediangtr  = rblockno;


    GBT_NODETYPE(lbuf,&nodetype);
    v = nodetype;
    SBT_NODETYPE(rbuf,v);
    lv = overflow;
    SBT_OVERFLOW(rbuf,lv);


    totlen = 0;
    for(i=0;i<keypos;++i)
    {
	ajStrAssS(&tkarray[i],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i] = parray[i];
    }
    tparray[i] = parray[i];
    v = totlen;
    SBT_TOTLEN(lbuf,v);
    n = i;
    v = n;
    SBT_NKEYS(lbuf,v);
    btreeWriteNode(cache,lpage,tkarray,tparray,i);



    for(i=0;i<n+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = lblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    totlen = 0;
    for(i=keypos+1;i<nkeys;++i)
    {
	ajStrAssS(&tkarray[i-(keypos+1)],karray[i]);
	totlen += ajStrLen(karray[i]);
	tparray[i-(keypos+1)] = parray[i];
    }
    tparray[i-(keypos+1)] = parray[i];
    v = totlen;
    SBT_TOTLEN(rbuf,v);
    rkeyno = (nkeys-keypos) - 1;
    v = rkeyno;
    SBT_NKEYS(rbuf,v);
    btreeWriteNode(cache,rpage,tkarray,tparray,rkeyno);


    for(i=0;i<rkeyno+1;++i)
    {
	tpage = ajBtreeCacheRead(cache,tparray[i]);
	tbuf = tpage->buf;
	lv = rblockno;
	SBT_PREV(tbuf,lv);
	tpage->dirty = BT_DIRTY;
    }


    ipage = rpage;
    if(strcmp(key->Ptr,mediankey->Ptr)<0)
	ipage = lpage;

    btreeInsertNonFullSec(cache,ipage,key,less,greater);


    for(i=0;i<order;++i)
    {
	ajStrDel(&karray[i]);
	ajStrDel(&tkarray[i]);
    }
    AJFREE(karray);
    AJFREE(tkarray);
    AJFREE(parray);
    AJFREE(tparray);

    ipage = ajBtreeCacheRead(cache,prev);

    btreeInsertKeySec(cache,ipage,mediankey,medianless,mediangtr);
    ajStrDel(&mediankey);

    return;
}




/* @funcstatic btreeInsertShiftSec *****************************************
**
** Rebalance buckets on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] retpage [AjPBtpage*] page
** @param [r] key [const char *] key
**
** @return [ajlong] bucket block or 0L if shift not posible 
** @@
******************************************************************************/

static ajlong btreeInsertShiftSec(AjPBtcache cache, AjPBtpage *retpage,
				  const char *key)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage tpage = NULL;

    ajint tkeys = 0;
    ajint pkeys = 0;
    ajint skeys = 0;
    ajint order = 0;
    
    ajint i;
    ajint n;
    
    ajlong parent  = 0L;
    ajlong blockno = 0L;
    
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajlong *pTarray = NULL;
    ajlong *pParray = NULL;
    ajlong *pSarray = NULL;

    AjPStr *karray = NULL;
    ajlong *parray = NULL;

    ajint ppos    = 0;
    ajint pkeypos = 0;
    ajint minsize = 0;
    
    ajDebug("In btreeInsertShiftSec\n");


    tpage = *retpage;

    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->sorder;
    minsize = order / 2;
    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return 0L;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    
    AJCNEW0(kParray,order);
    AJCNEW0(pParray,order);
    AJCNEW0(kSarray,order);
    AJCNEW0(pSarray,order);
    AJCNEW0(kTarray,order);
    AJCNEW0(pTarray,order);
    
    for(i=0;i<order;++i)
    {
	kSarray[i] = ajStrNew();
	kParray[i] = ajStrNew();
	kTarray[i] = ajStrNew();
    }

    btreeGetKeys(cache,pbuf,&kParray,&pParray);

    i=0;
    while(i!=pkeys && strcmp(key,kParray[i]->Ptr)>=0)
	++i;
    pkeypos = i;
    
    if(i==pkeys)
    {
	if(strcmp(key,kParray[i-1]->Ptr)<0)
	    ppos = i-1;
	else
	    ppos = i;
    }
    else
	ppos = i;

    
    if(ppos) /* There is another leaf to the left */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos-1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	/* ajDebug("Left shift\n"); */
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;
	while(pParray[i] != tpage->pageno)
	    ++i;
	--i;

	pkeypos = i;

	ajStrAssS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssS(&kParray[pkeypos],kTarray[0]);
	for(i=0;i<tkeys;++i)
	{
	    ajStrAssS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	i = 0;
	while(i!=pkeys && strcmp(key,kParray[i]->Ptr)>=0)
	    ++i;
	if(i==pkeys)
	{
	    if(strcmp(key,kParray[i-1]->Ptr)<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pageno)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	

	i = 0;
	while(i!=n && strcmp(key,karray[i]->Ptr)>=0)
	    ++i;
	if(i==n)
	{
	    if(strcmp(key,karray[i-1]->Ptr)<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return blockno;
    }
    

    if(ppos != pkeys)	/* There is a right node */
    {
	spage = ajBtreeCacheRead(cache,pParray[ppos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    /* Space in the right leaf */
    if(ppos != pkeys && skeys != order-1)
    {
	/* ajDebug("Right shift\n");*/
	btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
	btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	i = 0;
	while(pParray[i] != tpage->pageno)
	    ++i;
	pkeypos = i;
	
	pSarray[skeys+1] = pSarray[skeys];
	for(i=skeys-1;i>-1;--i)
	{
	    ajStrAssS(&kSarray[i+1],kSarray[i]);
	    pSarray[i+1] = pSarray[i];
	}
	ajStrAssS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	i = 0;
	while(i!=pkeys && strcmp(key,kParray[i]->Ptr)>=0)
	    ++i;
	if(i==pkeys)
	{
	    if(strcmp(key,kParray[i-1]->Ptr)<0)
		blockno = pParray[i-1];
	    else
		blockno = pParray[i];
	}
	else
	    blockno = pParray[i];

	if(blockno == spage->pageno)
	{
	    *retpage = spage;
	    karray = kSarray;
	    parray = pSarray;
	    n = skeys;
	}
	else
	{
	    karray = kTarray;
	    parray = pTarray;
	    n = tkeys;
	}
	
	i = 0;
	while(i!=n && strcmp(key,karray[i]->Ptr)>=0)
	    ++i;
	if(i==n)
	{
	    if(strcmp(key,karray[i-1]->Ptr)<0)
		blockno = parray[i-1];
	    else
		blockno = parray[i];
	}
	else
	    blockno = parray[i];

	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return blockno;
    }


    for(i=0;i<order;++i)
    {
	ajStrDel(&kTarray[i]);
	ajStrDel(&kParray[i]);
	ajStrDel(&kSarray[i]);
    }
    AJFREE(kTarray);
    AJFREE(kSarray);
    AJFREE(kParray);
    AJFREE(pTarray);
    AJFREE(pSarray);
    AJFREE(pParray);

    return 0L;
}




/* @funcstatic btreeInsertNonFullSec *****************************************
**
** Insert a key into a non-full node
**
** @param [u] cache [AjPBtcache] cache
** @param [u] page [AjPBtpage] original page
** @param [r] key [const AjPStr] key to insert
** @param [r] less [ajlong] less-than pointer
** @param [r] greater [ajlong] greater-than pointer
**
** @return [void]
** @@
******************************************************************************/

static void btreeInsertNonFullSec(AjPBtcache cache, AjPBtpage page,
				  const AjPStr key, ajlong less,
				  ajlong greater)
{
    unsigned char *buf = NULL;

    AjPStr *karray = NULL;
    ajlong *parray = NULL;

    ajint nkeys = 0;
    ajint order = 0;
    ajint ipos  = 0;
    ajint i;
    ajint count = 0;

    ajlong lv = 0L;
    ajint  v  = 0;
    

    AjPBtpage ppage = NULL;
    ajlong pageno   = 0L;

    ajint nodetype = 0;
    
    ajDebug("In btreeInsertNonFullSec\n");

    order = cache->sorder;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);

    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    buf = page->buf;
    GBT_NKEYS(buf,&nkeys);
    GBT_NODETYPE(buf,&nodetype);
    
    btreeGetKeys(cache,buf,&karray,&parray);

    i = 0;
    while(i!=nkeys && strcmp(key->Ptr,karray[i]->Ptr) >= 0)
	++i;

    ipos = i;

    count = nkeys - ipos;
    

    if(ipos == nkeys)
    {
	ajStrAssS(&karray[ipos],key);
	parray[ipos+1] = greater;
	parray[ipos]   = less;
    }
    else
    {
	parray[nkeys+1] = parray[nkeys];

	for(i=nkeys-1; count>0; --count, --i)
	{
	    ajStrAssS(&karray[i+1],karray[i]);
	    parray[i+1] = parray[i];
	}

	ajStrAssS(&karray[ipos],key);
	parray[ipos] = less;
	parray[ipos+1] = greater;
    }

    ++nkeys;
    v = nkeys;
    SBT_NKEYS(buf,v);

    btreeWriteNode(cache,page,karray,parray,nkeys);
    if(nodetype == BT_ROOT)
	page->dirty = BT_LOCK;

    pageno = page->pageno;
    ppage = ajBtreeCacheRead(cache,less);
    lv = pageno;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;
    ppage = ajBtreeCacheRead(cache,greater);
    lv = pageno;
    SBT_PREV(ppage->buf,lv);
    ppage->dirty = BT_DIRTY;
    


    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    if(nodetype != BT_ROOT)
	btreeKeyShiftSec(cache,page);

    return;
}




/* @funcstatic btreeKeyShiftSec ********************************************
**
** Rebalance secondary Nodes on insertion
**
** @param [u] cache [AjPBtcache] cache
** @param [u] tpage [AjPBtpage] page
**
** @return [void]
** @@
******************************************************************************/

static void btreeKeyShiftSec(AjPBtcache cache, AjPBtpage tpage)
{
    unsigned char *tbuf = NULL;
    unsigned char *pbuf = NULL;
    unsigned char *sbuf = NULL;
    unsigned char *buf  = NULL;

    AjPBtpage ppage = NULL;
    AjPBtpage spage = NULL;
    AjPBtpage page  = NULL;

    ajint tkeys = 0;
    ajint pkeys = 0;
    ajint skeys = 0;
    ajint order = 0;
    
    ajint i;
    
    ajlong parent = 0L;
    
    AjPStr *kTarray = NULL;
    AjPStr *kParray = NULL;
    AjPStr *kSarray = NULL;
    ajlong *pTarray = NULL;
    ajlong *pParray = NULL;
    ajlong *pSarray = NULL;

    ajint pkeypos = 0;
    ajint minsize = 0;

    ajlong lv = 0L;
    
    ajDebug("In btreeKeyShiftSec\n");
    
    tbuf = tpage->buf;

    GBT_PREV(tbuf,&parent);
    GBT_NKEYS(tbuf,&tkeys);

    order = cache->sorder;
    minsize = order / 2;
    if(order % 2)
	++minsize;

    if(tkeys <= minsize)
	return;

    
    ppage = ajBtreeCacheRead(cache,parent);
    pbuf = ppage->buf;
    GBT_NKEYS(pbuf,&pkeys);
    

    AJCNEW0(kParray,order);
    AJCNEW0(pParray,order);
    AJCNEW0(kSarray,order);
    AJCNEW0(pSarray,order);
    AJCNEW0(kTarray,order);
    AJCNEW0(pTarray,order);
    
    for(i=0;i<order;++i)
    {
	kSarray[i] = ajStrNew();
	kParray[i] = ajStrNew();
	kTarray[i] = ajStrNew();
    }

    btreeGetKeys(cache,tbuf,&kTarray,&pTarray);
    GBT_NKEYS(tbuf,&tkeys);


    btreeGetKeys(cache,pbuf,&kParray,&pParray);
    i=0;
    while(pParray[i] != tpage->pageno)
	++i;

    if(i) /* There is another leaf to the left */
    {
	pkeypos = i-1;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
	
    }

    if(i && skeys != order-1) /* There is space in the left leaf */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	ajStrAssS(&kSarray[skeys],kParray[pkeypos]);
	pSarray[skeys+1] = pTarray[0];
	++skeys;
	--tkeys;
	ajStrAssS(&kParray[pkeypos],kTarray[0]);
	for(i=0;i<tkeys;++i)
	{
	    ajStrAssS(&kTarray[i],kTarray[i+1]);
	    pTarray[i] = pTarray[i+1];
	}
	pTarray[i] = pTarray[i+1];
	pTarray[i+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	page = ajBtreeCacheRead(cache,pSarray[skeys]);
	buf = page->buf;
	lv = spage->pageno;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;


	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return;
    }



    if(i != pkeys)	/* There is a right node */
    {
	pkeypos = i;
	spage = ajBtreeCacheRead(cache,pParray[pkeypos+1]);
	sbuf = spage->buf;
	GBT_NKEYS(sbuf,&skeys);
    }


    if(i != pkeys && skeys != order-1) /* Space in the right node */
    {
	if(skeys)
	    btreeGetKeys(cache,sbuf,&kSarray,&pSarray);

	pSarray[skeys+1] = pSarray[skeys];
	for(i=skeys-1;i>-1;--i)
	{
	    ajStrAssS(&kSarray[i+1],kSarray[i]);
	    pSarray[i+1] = pSarray[i];
	}
	ajStrAssS(&kSarray[0],kParray[pkeypos]);
	pSarray[0] = pTarray[tkeys];
	ajStrAssS(&kParray[pkeypos],kTarray[tkeys-1]);
	++skeys;
	--tkeys;
	pTarray[tkeys+1] = 0L;
	
	btreeWriteNode(cache,spage,kSarray,pSarray,skeys);
	btreeWriteNode(cache,tpage,kTarray,pTarray,tkeys);
	btreeWriteNode(cache,ppage,kParray,pParray,pkeys);
	if(!ppage->pageno)
	    ppage->dirty = BT_LOCK;

	page = ajBtreeCacheRead(cache,pSarray[0]);
	buf = page->buf;
	lv = spage->pageno;
	SBT_PREV(buf,lv);
	page->dirty = BT_DIRTY;

	for(i=0;i<order;++i)
	{
	    ajStrDel(&kTarray[i]);
	    ajStrDel(&kParray[i]);
	    ajStrDel(&kSarray[i]);
	}
	AJFREE(kTarray);
	AJFREE(kSarray);
	AJFREE(kParray);
	AJFREE(pTarray);
	AJFREE(pSarray);
	AJFREE(pParray);

	return;
    }


    for(i=0;i<order;++i)
    {
	ajStrDel(&kTarray[i]);
	ajStrDel(&kParray[i]);
	ajStrDel(&kSarray[i]);
    }
    AJFREE(kTarray);
    AJFREE(kSarray);
    AJFREE(kParray);
    AJFREE(pTarray);
    AJFREE(pSarray);
    AJFREE(pParray);

    return;
}




/* @func btreeLockTest ********************************************
**
** Test function: show if a primary tree root block is unlocked
**
** @param [u] cache [AjPBtcache] cache
**
** @return [void]
** @@
******************************************************************************/

void btreeLockTest(AjPBtcache cache)
{
    AjPBtpage page = NULL;
    
    page = btreeCacheLocate(cache,0L);
    if(page->dirty != BT_LOCK)
	ajFatal("Root page unlocked\n");

    return;
}




/* @func ajBtreeSecLeafList ********************************************
**
** Read the leaves of a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajlong] root page of secondary tree
**
** @return [AjPList] List of IDs matching a keyword
** @@
******************************************************************************/

AjPList ajBtreeSecLeafList(AjPBtcache cache, ajlong rootblock)
{
    AjPList list;
    ajint order;
    AjPStr *karray;
    ajlong *parray;
    AjPBtpage page;
    unsigned char *buf;
    ajint nodetype;
    ajint i;
    ajint j;
    ajint k;
    ajlong level = 0L;
    
    AjPSecBucket bucket;
    ajint nkeys;
    AjPStr id = NULL;
    ajlong right;

    list = ajListNew();

    order = cache->order;

    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();

    
    page = ajBtreeCacheRead(cache,rootblock);
    buf = page->buf;

    GBT_RIGHT(buf,&level);
    cache->slevel = (ajint)level;
    
    btreeGetKeys(cache,buf,&karray,&parray);
    GBT_NODETYPE(buf,&nodetype);

    while(nodetype != BT_LEAF && cache->slevel!=0)
    {
	page = ajBtreeCacheRead(cache,parray[0]);
	buf = page->buf;
	btreeGetKeys(cache,buf,&karray,&parray);
	GBT_NODETYPE(buf,&nodetype);
    }

    do
    {
	GBT_NKEYS(buf,&nkeys);
	for(j=0;j<nkeys+1;++j)
	{
	    bucket = btreeReadSecBucket(cache, parray[j]);
	    
	    for(k=0;k<bucket->Nentries;++k)
	    {
		id = ajStrNew();
		ajStrAssS(&id,bucket->ids[k]);
		ajListPush(list, (void *)id);
	    }
	    btreeSecBucketDel(&bucket);
	}

	right = 0L;
	if(cache->slevel)
	{
	    GBT_RIGHT(buf,&right);
	    if(right)
	    {
		page = ajBtreeCacheRead(cache,right);	    
		buf = page->buf;
		btreeGetKeys(cache,buf,&karray,&parray);
	    }
	}
	
    } while(right);



    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return list;
}




/* @func ajBtreeVerifyId ********************************************
**
** Test routine: test for ID within a secondary tree
**
** @param [u] cache [AjPBtcache] cache
** @param [r] rootblock [ajlong] root page of secondary tree
** @param [r] id [const char *] test ID
**
** @return [AjBool] true if ID found
** @@
******************************************************************************/

AjBool ajBtreeVerifyId(AjPBtcache cache, ajlong rootblock, const char *id)
{
    AjPBtpage page;
    AjPBtpage spage;
    AjPSecBucket bucket;
    AjPStr *karray;
    ajlong *parray;
    ajint order;
    ajint i;
    ajint nkeys;
    unsigned char *buf;
    ajlong blockno;
    AjBool found;
    ajlong right;

    cache->secrootblock = rootblock;
    page = ajBtreeCacheRead(cache,rootblock);
    page->dirty = BT_LOCK;
    buf = page->buf;
    GBT_RIGHT(buf,&right);
    cache->slevel = (ajint)right;
    
    spage = ajBtreeSecFindInsert(cache,id);
    if(!spage)
	return ajFalse;
    
    buf = spage->buf;
    
    order = cache->sorder;
    AJCNEW0(karray,order);
    AJCNEW0(parray,order);
    for(i=0;i<order;++i)
	karray[i] = ajStrNew();
    GBT_NKEYS(buf,&nkeys);


    btreeGetKeys(cache,buf,&karray,&parray);
    i=0;
    while(i!=nkeys && strcmp(id,karray[i]->Ptr)>=0)
	++i;
    if(i==nkeys)
    {
	if(strcmp(id,karray[i-1]->Ptr)<0)
	    blockno = parray[i-1];
	else
	    blockno = parray[i];
    }
    else
	blockno = parray[i];

    bucket = btreeReadSecBucket(cache,blockno);

    found = ajFalse;
    for(i=0;i<bucket->Nentries;++i)
	if(!strcmp(id,bucket->ids[i]->Ptr))
	{
	    found = ajTrue;
	    break;
	}

    btreeSecBucketDel(&bucket);
    for(i=0;i<order;++i)
	ajStrDel(&karray[i]);
    AJFREE(karray);
    AJFREE(parray);

    return found;
}

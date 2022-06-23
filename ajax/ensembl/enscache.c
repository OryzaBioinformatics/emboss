/* @source enscache ***********************************************************
**
** Ensembl Cache functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.40 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:40 $ by $Author: mks $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA  02110-1301,  USA.
**
******************************************************************************/

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "enscache.h"
#include "enstable.h"




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== global variables ============================ */
/* ========================================================================= */




/* ========================================================================= */
/* ============================= private data ============================== */
/* ========================================================================= */

/* @datastatic CachePNode *****************************************************
**
** Ensembl Cache Node.
**
** @alias CacheSNode
** @alias CacheONode
**
** @attr Key [void*] Key data address
** @attr Value [void*] Value data address
** @attr Bytes [size_t] Byte size of this node including key and value data
** @attr Dirty [AjBool] Flag to mark that value data has not been written back
** @attr Padding [ajuint] Padding to alignment boundary
** @@
******************************************************************************/

typedef struct CacheSNode
{
    void *Key;
    void *Value;
    size_t Bytes;
    AjBool Dirty;
    ajuint Padding;
} CacheONode;

#define CachePNode CacheONode*




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static CachePNode cacheNodeNew(const EnsPCache cache, void *key, void *value);

static void cacheNodeDel(const EnsPCache cache, CachePNode *Pnode);

static AjBool cacheNodeInsert(EnsPCache cache, CachePNode node);

static AjBool cacheNodeRemove(EnsPCache cache, const CachePNode node);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection enscache ******************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @funcstatic cacheNodeNew ***************************************************
**
** Default constructor for an Ensembl Cache Node.
**
** The size of the Cache Node will be estimated according to the Ensembl Cache
** type with the function already provided at the Cache initialisation stage.
** This fuction will also reference value data to increment an internal usage
** counter and prevent deletion of value data while in the cache.
**
** @param [r] cache [const EnsPCache] Ensembl Cache
** @param [r] key [void*] Key data address
** @param [r] value [void*] Value data address
**
** @return [CachePNode] Ensembl Cache Node or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

static CachePNode cacheNodeNew(const EnsPCache cache, void *key, void *value)
{
    ajuint *Puintkey = NULL;

    CachePNode node = NULL;

    if (!cache)
        return NULL;

    if (!key)
        return NULL;

    if (!value)
        return NULL;

    AJNEW0(node);

    /* Add the size of the Ensembl Cache Node itself. */

    node->Bytes = sizeof (CacheONode);

    switch (cache->Type)
    {
        case ensECacheTypeNumeric:

            /* Copy AJAX unsigned integer key data. */

            AJNEW0(Puintkey);

            *Puintkey = *((ajuint *) key);

            node->Key = (void *) Puintkey;

            /* Add the size of unsigned integer key data. */

            node->Bytes += sizeof (ajuint);

            break;

        case ensECacheTypeAlphaNumeric:

            /* Copy AJAX String key data. */

            node->Key = (void *) ajStrNewS((AjPStr) key);

            /* Add the size of AJAX String key data. */

            node->Bytes += sizeof (AjOStr);

            node->Bytes += ajStrGetRes((AjPStr) node->Key);

            break;

        default:

            ajWarn("cacheNodeNew got unexpected Cache type %d.\n",
                   cache->Type);
    }

    /* Reference the value data. */

    if (cache->Freference && value)
        node->Value = (*cache->Freference) (value);

    /* Calculate the size of the value data. */

    if (cache->Fsize && node->Value)
        node->Bytes += (*cache->Fsize) (node->Value);

    node->Dirty = ajFalse;

    return node;
}




/* @funcstatic cacheNodeDel ***************************************************
**
** Default destructor for an Ensembl Cache Node.
**
** @param [r] cache [const EnsPCache] Ensembl Cache
** @param [d] Pnode [CachePNode*] Ensembl Cache Node address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

static void cacheNodeDel(const EnsPCache cache, CachePNode *Pnode)
{
    CachePNode pthis = NULL;

    if (!cache)
        return;

    if (!Pnode)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("cacheNodeDel"))
    {
        ajDebug("cacheNodeDel\n"
                "  *Pnode %p\n",
                *Pnode);

        /* cacheNodeTrace(*Pnode, 1); */
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pnode))
        return;

    /* Delete key data. */

    switch (cache->Type)
    {
        case ensECacheTypeNumeric:

            /* Delete AJAX unsigned integer key data. */

            AJFREE(pthis->Key);

            break;

        case ensECacheTypeAlphaNumeric:

            /* Delete AJAX String key data. */

            ajStrDel((AjPStr *) &pthis->Key);

            break;

        default:

            ajWarn("cacheNodeDel got unexpected Cache type %d.\n",
                   cache->Type);
    }

    /* Delete value data. */

    if (cache->Fdelete && pthis->Value)
        (*cache->Fdelete) (&pthis->Value);

    ajMemFree((void **) Pnode);

    return;
}




/* @funcstatic cacheNodeInsert ************************************************
**
** Insert an Ensembl Cache Node into an Ensembl Cache.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [u] node [CachePNode] Ensembl Cache Node
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool cacheNodeInsert(EnsPCache cache, CachePNode node)
{
    CachePNode old = NULL;

    if (!cache)
        return ajFalse;

    if (!node)
        return ajFalse;

    if (cache->MaxSize && (node->Bytes > cache->MaxSize))
        return ajFalse;

    /* Insert the node into the AJAX List. */

    ajListPushAppend(cache->List, (void *) node);

    /* Insert the node into the AJAX Table. */

    ajTablePut(cache->Table, node->Key, (void *) node);

    /* Update the cache statistics. */

    cache->Bytes += node->Bytes;

    cache->Count++;

    cache->Stored++;

    /* If the cache is too big, remove the top node(s). */

    while ((cache->MaxBytes && (cache->Bytes > cache->MaxBytes)) ||
           (cache->MaxCount && (cache->Count > cache->MaxCount)))
    {
        /* Remove the top node from the AJAX List. */

        ajListPop(cache->List, (void **) &old);

        /* Remove the node also from the AJAX Table. */

        ajTableRemove(cache->Table, old->Key);

        /* Update the cache statistics. */

        cache->Bytes -= old->Bytes;

        cache->Count--;

        cache->Dropped++;

        /* Write changes of value data to disk if any. */

        if (cache->Fwrite && old->Value && old->Dirty)
            (*cache->Fwrite) (old->Value);

        /* Both, key and value data are deleted via cacheNodeDel. */

        cacheNodeDel(cache, &old);
    }

    return ajTrue;
}




/* @funcstatic cacheNodeRemove ************************************************
**
** Remove an Ensembl Cache Node from an Ensembl Cache.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] node [const CachePNode] Ensembl Cache Node
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

static AjBool cacheNodeRemove(EnsPCache cache, const CachePNode node)
{
    AjIList iter = NULL;

    CachePNode lnode = NULL;

    if (!cache)
        return ajFalse;

    if (!node)
        return ajFalse;

    /* Remove the node from the AJAX List. */

    iter = ajListIterNew(cache->List);

    while (!ajListIterDone(iter))
    {
        lnode = (CachePNode) ajListIterGet(iter);

        if (lnode == node)
        {
            ajListIterRemove(iter);

            break;
        }
    }

    ajListIterDel(&iter);

    /* Remove the node from the AJAX Table. */

    ajTableRemove(cache->Table, node->Key);

    /* Update the cache statistics. */

    cache->Bytes -= node->Bytes;

    cache->Count--;

    cache->Removed++;

    return ajTrue;
}




/* @datasection [EnsPCache] Ensembl Cache *************************************
**
** @nam2rule Cache Functions for manipulating Ensembl Cache objects
** @cc Bio::EnsEMBL::Utils::Cache
** @cc CVS Revision: 1.3
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** @fdata [EnsPCache]
**
** @nam3rule New Constructor
**
** @argrule New type [const EnsECacheType] Ensembl Cache type
** @argrule New maxbytes [size_t] Maximum number of bytes held in the cache
** @argrule New maxcount [ajuint] Maximum number of objects to be cached
** @argrule New maxsize [size_t] Maximum size of an object to be cached
** @argrule New Freference [void* function] Object-specific referencing
** function
** @argrule New Fdelete [void function] Object-specific deletion function
** @argrule New Fsize [size_t function] Object-specific memory sizing function
** @argrule New Fread [void* function] Object-specific reading function
** @argrule New Fwrite [AjBool function] Object-specific writing function
** @argrule New synchron [AjBool] ajTrue: Immediately write-back value data
** @argrule New label [const char*] Cache label for statistics output
**
** @valrule * [EnsPCache] Ensembl Cache or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensCacheNew **********************************************************
**
** Default constructor for an Ensembl Cache.
**
** @param [r] type [const EnsECacheType] Ensembl Cache type
**                 (ensECacheTypeNumeric or ensECacheTypeAlphaNumeric)
** @param [r] maxbytes [size_t] Maximum number of bytes held in the cache
** @param [r] maxcount [ajuint] Maximum number of objects to be cached
** @param [r] maxsize [size_t] Maximum size of an object to be cached
** @param [f] Freference [void* function] Object-specific referencing function
** @param [f] Fdelete [void function] Object-specific deletion function
** @param [f] Fsize [size_t function] Object-specific memory sizing function
** @param [f] Fread [void* function] Object-specific reading function
** @param [f] Fwrite [AjBool function] Object-specific writing function
** @param [r] synchron [AjBool] ajTrue: Immediately write-back value data
**                              ajFalse: Write-back value data later
** @param [r] label [const char*] Cache label for statistics output
**
** @return [EnsPCache] Ensembl Cache or NULL
**
** @release 6.2.0
** @@
** The maximum size parameter should prevent the cache from purging too many
** objects when very large objects are inserted. If not set it defaults to
** a tenth of the maximum cache size.
**
** Object-specific functions are required to reference objects held in the
** cache or delete objects once purged from the cache, as well as memory sizing
** functions and object-specific read and write back functions.
******************************************************************************/

EnsPCache ensCacheNew(const EnsECacheType type,
                      size_t maxbytes,
                      ajuint maxcount,
                      size_t maxsize,
                      void* (*Freference) (void *value),
                      void (*Fdelete) (void **Pvalue),
                      size_t (*Fsize) (const void *value),
                      void* (*Fread) (const void *key),
                      AjBool (*Fwrite) (const void *value),
                      AjBool synchron,
                      const char *label)
{
    AjBool debug = AJFALSE;

    EnsPCache cache = NULL;

    debug = ajDebugTest("ensCacheNew");

    if (debug)
        ajDebug("ensCacheNew\n"
                "  type %d\n"
                "  maxbytes %Lu\n"
                "  maxcount %u\n"
                "  maxsize %Lu\n"
                "  Freference %p\n"
                "  Fdelete %p\n"
                "  Fsize %p\n"
                "  Fread %p\n"
                "  Fwrite %p\n"
                "  synchron '%B'\n"
                "  label '%s'\n",
                type,
                maxbytes,
                maxcount,
                maxsize,
                Freference,
                Fdelete,
                Fsize,
                Fread,
                Fwrite,
                synchron,
                label);
    /* FIXME: size_t can be shorter than ajulong */

    if ((type < ensECacheTypeNumeric) || (type > ensECacheTypeAlphaNumeric))
        ajFatal("ensCacheNew requires a valid type.\n");

    if ((!maxbytes) && (!maxcount))
        ajFatal("ensCacheNew requires either a "
                "maximum bytes or maximum count limit.\n");

    if (!maxsize)
        maxsize = maxbytes ? maxbytes / 10 + 1 : 0;

    if (maxbytes && (!maxsize))
        ajFatal("ensCacheNew requires a maximum size limit, "
                "when a maximum bytes limit is set.");

    /* TODO: Find and set a sensible value here! */
    /* FIXME: size_t can be shorter than ajulong */
    if (debug)
        ajDebug("ensCacheNew maxbytes %Lu, maxcount %u, maxsize %Lu.\n",
                maxbytes, maxcount, maxsize);

    if (maxbytes && (maxbytes < 1000))
        ajFatal("ensCacheNew cannot set a maximum bytes limit (%Lu) under "
                "1000, as each Cache Node requires %Lu bytes alone.",
                maxbytes, sizeof (CacheONode));

    /* TODO: Find and set a sensible value here! */

    if (maxsize && (maxsize < 3))
        ajFatal("ensCacheNew cannot set a maximum size limit (%Lu) under "
                "3 bytes. maximum bytes %Lu maximum count %u.",
                maxsize, maxbytes, maxcount);

    /*
    ** Pointers to functions for automatic reading of data not yet in the
    ** cache and writing of data modified in cache are not mandatory.
    ** If not specified the cache will simply lack this functionality.
    ** However, the specification of a function deleting stale cache entries
    ** and a function calculating the size of value data are required.
    */

    if (!Freference)
        ajFatal("ensCacheNew requires a referencing function.");

    if (!Fdelete)
        ajFatal("ensCacheNew requires a deletion function.");

    if (maxsize && (!Fsize))
        ajFatal("ensCacheNew requires a memory sizing function "
                "when a maximum size limit has been defined.");

    if (!label)
        ajFatal("ensCacheNew requires a label.");

    AJNEW0(cache);

    cache->Label = ajStrNewC(label);
    cache->List  = ajListNew();

    switch (type)
    {
        case ensECacheTypeNumeric:

            cache->Table = ajTableuintNew(0U);

            break;

        case ensECacheTypeAlphaNumeric:

            cache->Table = ajTablestrNew(0U);

            break;

        default:

            ajWarn("ensCacheNew got unexpected Cache type %d.\n",
                   cache->Type);
    }

    /*
    ** Since the AJAX Table does not use real key or value data,
    ** both, keydel and valdel need setting to NULL.
    */

    ajTableSetDestroy(
        cache->Table,
        (void (*)(void **)) NULL,
        (void (*)(void **)) NULL);

    cache->Freference = Freference;
    cache->Fdelete    = Fdelete;
    cache->Fsize      = Fsize;
    cache->Fread      = Fread;
    cache->Fwrite     = Fwrite;
    cache->Type       = type;
    cache->Synchron   = synchron;
    cache->MaxBytes   = maxbytes;
    cache->MaxCount   = maxcount;
    cache->MaxSize    = maxsize;
    cache->Bytes      = 0;
    cache->Count      = 0;
    cache->Dropped    = 0;
    cache->Removed    = 0;
    cache->Stored     = 0;
    cache->Hit        = 0;
    cache->Miss       = 0;

    return cache;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Cache object.
**
** @fdata [EnsPCache]
**
** @nam3rule Del Destroy (free) an Ensembl Cache
**
** @argrule * Pcache [EnsPCache*] Ensembl Cache address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensCacheDel **********************************************************
**
** Default destructor for an Ensembl Cache.
**
** @param [u] Pcache [EnsPCache*] Ensembl Cache address
**
** @return [void]
**
** @release 6.2.0
** @@
** Value data in Cache Nodes that have not been synchronised are written-back.
** Cache flags are reset for value data before the value data is deleted.
** After deletion of all Cache Nodes a summary statistics is printed and the
** Ensembl Cache is destroyed.
******************************************************************************/

void ensCacheDel(EnsPCache *Pcache)
{
    AjBool debug = AJFALSE;

    EnsPCache pthis = NULL;

    if (!Pcache)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    debug = ajDebugTest("ensCacheDel");

    if (debug)
        ajDebug("ensCacheDel\n"
                "  *Pcache %p\n",
                *Pcache);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Pcache))
        return;

    ensCacheClear(pthis);

    if (debug)
        ensCacheTrace(pthis, 1);

    ajStrDel(&pthis->Label);

    ajListFree(&pthis->List);

    ajTableFree(&pthis->Table);

    ajMemFree((void **) Pcache);

    return;
}




/* @section clear *************************************************************
**
** Clear an Ensembl Cache.
**
** @fdata [EnsPCache]
**
** @nam3rule Clear Clear an Ensembl Cache
**
** @argrule * cache [EnsPCache] Ensembl Cache
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensCacheClear ********************************************************
**
** Clear an Ensembl Cache.
**
** @param [u] cache [EnsPCache] Ensembl Cache
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.5.0
** @@
** Value data in Cache Node objects that have not been synchronised are
** written-back. Cache flags are reset for value data before the value data
** is deleted.
******************************************************************************/

AjBool ensCacheClear(EnsPCache cache)
{
    CachePNode node = NULL;

    if (!cache)
        return ajFalse;

    /* Remove Cache Node objects from the AJAX List. */

    while (ajListPop(cache->List, (void **) &node))
    {
        /* Remove the same Cache Node object from the AJAX Table. */

        (void) ajTableRemove(cache->Table, node->Key);

        /* Update the cache statistics. */

        cache->Count--;

        cache->Bytes -= node->Bytes;

        /* Write changes of value data to disk if any. */

        if (cache->Fwrite && node->Value && node->Dirty)
            (*cache->Fwrite) (node->Value);

        /* Both, key and value data are deleted via cacheNodeDel. */

        cacheNodeDel(cache, &node);
    }

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Cache object.
**
** @fdata [EnsPCache]
**
** @nam3rule Trace Report Ensembl Cache members to debug file.
**
** @argrule Trace cache [const EnsPCache] Ensembl Cache
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensCacheTrace ********************************************************
**
** Writes debug messages to trace the contents of a cache.
**
** @param [r] cache [const EnsPCache] Ensembl Cache
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCacheTrace(const EnsPCache cache, ajuint level)
{
    double ratio = 0.0;

    AjPStr indent = NULL;

    if (!cache)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    if (cache->Hit || cache->Miss)
        ratio = (double) cache->Hit /
            ((double) cache->Hit + (double) cache->Miss);

    ajDebug("%SensCache trace %p\n"
            "%S  Label '%S'\n"
            "%S  List %p length: %Lu\n"
            "%S  Table %p length: %Lu\n"
            "%S  Type %d\n"
            "%S  Synchron '%B'\n"
            "%S  MaxBytes %Lu\n" /* FIXME: size_t can be shorter than ajulong */
            "%S  MaxCount %u\n"
            "%S  MaxSize %Lu\n" /* FIXME: size_t can be shorter than ajulong */
            "%S  Bytes %Lu\n" /* FIXME: size_t can be shorter than ajulong */
            "%S  Count %u\n"
            "%S  Dropped %u\n"
            "%S  Removed %u\n"
            "%S  Stored %u\n"
            "%S  Hit %u\n"
            "%S  Miss %u\n"
            "%S  Hit/(Hit + Miss) %f\n",
            indent, cache,
            indent, cache->Label,
            indent, cache->List, ajListGetLength(cache->List),
            indent, cache->Table, ajTableGetLength(cache->Table),
            indent, cache->Type,
            indent, cache->Synchron,
            indent, cache->MaxBytes,
            indent, cache->MaxCount,
            indent, cache->MaxSize,
            indent, cache->Bytes,
            indent, cache->Count,
            indent, cache->Dropped,
            indent, cache->Removed,
            indent, cache->Stored,
            indent, cache->Hit,
            indent, cache->Miss,
            indent, ratio);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section modify ************************************************************
**
** Update cache object values
**
** @fdata [EnsPCache]
**
** @nam3rule Fetch Fetch value data from an Ensembl Cache
** @nam3rule Remove Remove value data from an Ensembl Cache
** @nam3rule Store Insert a value into an Ensembl Cache
** @nam3rule Synchronise Synchronise an Ensembl Cache
**
** @argrule Fetch cache [EnsPCache] Ensembl Cache
** @argrule Fetch key [void*] Key data address
** @argrule Fetch Pvalue [void**] Value data address address
** @argrule Remove cache [EnsPCache] Ensembl Cache
** @argrule Remove key [const void*] Key data address
** @argrule Store cache [EnsPCache] Ensembl Cache
** @argrule Store key [void*] Key data address
** @argrule Store Pvalue [void**] Value data address adress
** @argrule Synchronise cache [EnsPCache] Ensembl Cache
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensCacheFetch ********************************************************
**
** Fetch a value from an Ensembl Cache via a key. If the value is not already
** in the cache it will be read by the function provided at the Cache
** initialisation stage.
**
** The caller is responsible for deleting the returned object.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] key [void*] Key data address
** @param [wP] Pvalue [void**] Value data address address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCacheFetch(EnsPCache cache, void *key, void **Pvalue)
{
    AjIList iter = NULL;

    CachePNode lnode = NULL;
    CachePNode tnode = NULL;

    if (!cache)
        return ajFalse;

    if (!key)
        return ajFalse;

    if (!Pvalue)
        return ajFalse;

    *Pvalue = NULL;

    tnode = (CachePNode) ajTableFetchmodV(cache->Table, key);

    if (tnode)
    {
        cache->Hit++;

        /* Move the Cache Node to the end of the AJAX List. */

        iter = ajListIterNew(cache->List);

        while (!ajListIterDone(iter))
        {
            lnode = (CachePNode) ajListIterGet(iter);

            if (lnode == tnode)
            {
                ajListIterRemove(iter);

                ajListPushAppend(cache->List, (void *) lnode);

                break;
            }
        }

        ajListIterDel(&iter);

        /*
        ** Reference the object when returned by the cache so that external
        ** code has to delete it irrespectively whether it was read from the
        ** cache or instantiated by the cache->Fread function.
        */

        if (cache->Freference && tnode->Value)
            *Pvalue = (*cache->Freference) (tnode->Value);
    }
    else
    {
        cache->Miss++;

        if (cache->Fread)
        {
            *Pvalue = (*cache->Fread) (key);

            if (*Pvalue)
            {
                tnode = cacheNodeNew(cache, key, *Pvalue);

                if (!cacheNodeInsert(cache, tnode))
                    cacheNodeDel(cache, &tnode);
            }
        }
    }

    return ajTrue;
}




/* @func ensCacheRemove *******************************************************
**
** Remove value data from an Ensembl Cache via key data.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [r] key [const void*] Key data address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCacheRemove(EnsPCache cache, const void *key)
{
    CachePNode node = NULL;

    if (!cache)
        return ajFalse;

    if (!key)
        return ajFalse;

    node = (CachePNode) ajTableFetchmodV(cache->Table, key);

    if (node)
    {
        cacheNodeRemove(cache, node);

        /* Both, key and value data are deleted via cacheNodeDel. */

        cacheNodeDel(cache, &node);
    }

    return ajTrue;
}




/* @func ensCacheStore ********************************************************
**
** Insert value data into an Ensembl Cache under key data.
**
** @param [u] cache [EnsPCache] Ensembl Cache
** @param [u] key [void*] Key data address
** @param [w] Pvalue [void**] Value data address address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCacheStore(EnsPCache cache, void *key, void **Pvalue)
{
    CachePNode node = NULL;

    if (!cache)
        return ajFalse;

    if (!key)
        return ajFalse;

    if (!Pvalue)
        return ajFalse;

    /* Is a node already cached under this key? */

    node = (CachePNode) ajTableFetchmodV(cache->Table, key);

    if (node)
    {
        /*
        ** Delete the Object passed in and increase the reference counter
        ** of the cached Object before assigning it.
        */

        (*cache->Fdelete) (Pvalue);

        *Pvalue = (*cache->Freference) (node->Value);
    }
    else
    {
        node = cacheNodeNew(cache, key, *Pvalue);

        if (cacheNodeInsert(cache, node))
        {
            if (cache->Synchron)
            {
                if (cache->Fwrite && node->Value)
                    (*cache->Fwrite) (node->Value);

                node->Dirty = ajFalse;
            }
            else
                node->Dirty = ajTrue;
        }
        else
        {
            if (cache->Fwrite && node->Value)
                (*cache->Fwrite) (node->Value);

            cacheNodeDel(cache, &node);
        }
    }

    return ajTrue;
}




/* @func ensCacheSynchronise **************************************************
**
** Synchronise an Ensembl Cache by writing-back all value data that have not
** been written before.
**
** @param [u] cache [EnsPCache] Ensembl Cache
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensCacheSynchronise(EnsPCache cache)
{
    AjIList iter = NULL;

    CachePNode node = NULL;

    if (!cache)
        return ajFalse;

    iter = ajListIterNew(cache->List);

    while (!ajListIterDone(iter))
    {
        node = (CachePNode) ajListIterGet(iter);

        if (cache->Fwrite && node->Value && node->Dirty)
        {
            (*cache->Fwrite) (node->Value);

            node->Dirty = ajFalse;
        }
    }

    ajListIterDel(&iter);

    return ajTrue;
}

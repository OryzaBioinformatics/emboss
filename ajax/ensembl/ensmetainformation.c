/* @source Ensembl Meta-Information functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.25 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensmetainformation.h"
#include "enstable.h"




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */




/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private constants ========================= */
/* ==================================================================== */

/* @conststatic metainformationNonSpeciesKeys *********************************
**
** The following Ensembl Meta-Information keys are not species-specific,
** i.e. they are not linked to a species identifier.
**
******************************************************************************/

static const char* metainformationNonSpeciesKeys[] =
{
    "patch",
    "schema_version",
    "schema_type",
    (const char*) NULL
};




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool metainformationkeyIsSpecieskey(const AjPStr key);

static AjBool metainformationadaptorFetchAllbyStatement(
    EnsPMetainformationadaptor mia,
    const AjPStr statement,
    AjPList mis);

static AjBool metainformationadaptorCacheInit(
    EnsPMetainformationadaptor mia);

static void metainformationadaptorClearIdentifierCache(void** key,
                                                       void** value,
                                                       void* cl);

static void metainformationadaptorClearKeyCache(void** key,
                                                void** value,
                                                void* cl);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensmetainformation ********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPMetainformation] Ensembl Meta-Information ****************
**
** @nam2rule Metainformation Functions for manipulating
** Ensembl Meta-Information objects
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Meta-Information by pointer.
** It is the responsibility of the user to first destroy any previous
** Meta-Information. The target pointer does not need to be initialised to
** NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMetainformation]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy mi [const EnsPMetainformation] Ensembl Meta-Information
** @argrule Ini mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini species [ajuint] Species identififer
** @argrule Ini key [AjPStr] Key
** @argrule Ini value [AjPStr] Value
** @argrule Ref mi [EnsPMetainformation] Ensembl Meta-Information
**
** @valrule * [EnsPMetainformation] Ensembl Meta-Information or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMetainformationNewCpy *********************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [EnsPMetainformation] Ensembl Meta-Information or NULL
** @@
******************************************************************************/

EnsPMetainformation ensMetainformationNewCpy(const EnsPMetainformation mi)
{
    EnsPMetainformation pthis = NULL;

    if(!mi)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = mi->Identifier;

    pthis->Adaptor = mi->Adaptor;

    if(mi->Key)
        pthis->Key = ajStrNewRef(mi->Key);

    if(mi->Value)
        pthis->Value = ajStrNewRef(mi->Value);

    pthis->Species = (mi->Species) ? mi->Species : 1;

    return pthis;
}




/* @func ensMetainformationNewIni *********************************************
**
** Constructor for an Ensembl Meta-Information object with initial values.
**
** @cc Bio::EnsEMBL::Storable
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::MetaContainer
** @param [r] species [ajuint] Species identififer
** @param [u] key [AjPStr] Key
** @param [u] value [AjPStr] Value
**
** @return [EnsPMetainformation] Ensembl Meta-Information or NULL
** @@
******************************************************************************/

EnsPMetainformation ensMetainformationNewIni(EnsPMetainformationadaptor mia,
                                             ajuint identifier,
                                             ajuint species,
                                             AjPStr key,
                                             AjPStr value)
{
    EnsPMetainformation mi = NULL;

    AJNEW0(mi);

    mi->Use = 1;

    mi->Identifier = identifier;

    mi->Adaptor = mia;

    if(key)
        mi->Key = ajStrNewRef(key);

    if(value)
        mi->Value = ajStrNewRef(value);

    mi->Species = (species) ? species : 1;

    return mi;
}




/* @func ensMetainformationNewRef *********************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
**
** @return [EnsPMetainformation] Ensembl Meta-Information or NULL
** @@
******************************************************************************/

EnsPMetainformation ensMetainformationNewRef(EnsPMetainformation mi)
{
    if(!mi)
        return NULL;

    mi->Use++;

    return mi;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Meta-Information object.
**
** @fdata [EnsPMetainformation]
**
** @nam3rule Del Destroy (free) an Ensembl Meta-Information object
**
** @argrule * Pmi [EnsPMetainformation*] Ensembl Meta-Information
**                                       object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMetainformationDel ************************************************
**
** Default destructor for an Ensembl Meta-Information object.
**
** @param [d] Pmi [EnsPMetainformation*] Ensembl Meta-Information
**                                       object address
**
** @return [void]
** @@
******************************************************************************/

void ensMetainformationDel(EnsPMetainformation* Pmi)
{
    EnsPMetainformation pthis = NULL;

    if(!Pmi)
        return;

    if(!*Pmi)
        return;

    pthis = *Pmi;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pmi = NULL;

        return;
    }

    ajStrDel(&pthis->Key);
    ajStrDel(&pthis->Value);

    AJFREE(pthis);

    *Pmi = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an Ensembl Meta-Information object.
**
** @fdata [EnsPMetainformation]
**
** @nam3rule Get Return Meta-Information attribute(s)
** @nam4rule Adaptor Return the Ensembl Meta-Information Adaptor
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Key Return the key
** @nam4rule Species Return the species identifier
** @nam4rule Value Return the value
**
** @argrule * mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @valrule Adaptor [EnsPMetainformationadaptor] Ensembl Meta-Information
**                                               Adaptor or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Key [AjPStr] Key or NULL
** @valrule Species [ajuint] Species identifier or 0
** @valrule Value [AjPStr] Value or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensMetainformationGetAdaptor *****************************************
**
** Get the Ensembl Meta-Information Adaptor element of an
** Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPMetainformationadaptor ensMetainformationGetAdaptor(
    const EnsPMetainformation mi)
{
    if(!mi)
        return NULL;

    return mi->Adaptor;
}




/* @func ensMetainformationGetIdentifier **************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensMetainformationGetIdentifier(const EnsPMetainformation mi)
{
    if(!mi)
        return 0;

    return mi->Identifier;
}




/* @func ensMetainformationGetKey *********************************************
**
** Get the key element of an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [AjPStr] Key or NULL
** @@
******************************************************************************/

AjPStr ensMetainformationGetKey(const EnsPMetainformation mi)
{
    if(!mi)
        return NULL;

    return mi->Key;
}




/* @func ensMetainformationGetSpecies *****************************************
**
** Get the species identifier element of an Ensembl Meta-Information object.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [ajuint] Species identifier or 0
** @@
******************************************************************************/

ajuint ensMetainformationGetSpecies(const EnsPMetainformation mi)
{
    if(!mi)
        return 0;

    return mi->Species;
}




/* @func ensMetainformationGetValue *******************************************
**
** Get the value element of an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [AjPStr] Value or NULL
** @@
******************************************************************************/

AjPStr ensMetainformationGetValue(const EnsPMetainformation mi)
{
    if(!mi)
        return NULL;

    return mi->Value;
}




/* @section modifiers *********************************************************
**
** Functions for assigning elements of an Ensembl Meta-Information object.
**
** @fdata [EnsPMetainformation]
**
** @nam3rule Set Set one element of a Meta-Information
** @nam4rule Adaptor Set the Ensembl Meta-Information Adaptor
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Key Set the key
** @nam4rule Species Set the species identifier
** @nam4rule Value Set the value
**
** @argrule * mi [EnsPMetainformation] Ensembl Meta-Information object
** @argrule Adaptor mia [EnsPMetainformationadaptor] Ensembl Meta-Information
** Adaptor
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Key key [AjPStr] Key
** @argrule Species species [ajuint] Species identifier
** @argrule Value value [AjPStr] Value
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensMetainformationSetAdaptor *****************************************
**
** Set the Ensembl Meta-Information Adaptor element of an
** Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetAdaptor(EnsPMetainformation mi,
                                    EnsPMetainformationadaptor mia)
{
    if(!mi)
        return ajFalse;

    mi->Adaptor = mia;

    return ajTrue;
}




/* @func ensMetainformationSetIdentifier **************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetIdentifier(EnsPMetainformation mi,
                                       ajuint identifier)
{
    if(!mi)
        return ajFalse;

    mi->Identifier = identifier;

    return ajTrue;
}




/* @func ensMetainformationSetKey *********************************************
**
** Set the key element of an Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [u] key [AjPStr] Key
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetKey(EnsPMetainformation mi, AjPStr key)
{
    if(!mi)
        return ajFalse;

    ajStrDel(&mi->Key);

    if(key)
        mi->Key = ajStrNewRef(key);

    return ajTrue;
}




/* @func ensMetainformationSetSpecies *****************************************
**
** Set the species identifier element of an Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [r] species [ajuint] Species identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetSpecies(EnsPMetainformation mi, ajuint species)
{
    if(!mi)
        return ajFalse;

    mi->Species = species;

    return ajTrue;
}




/* @func ensMetainformationSetValue *******************************************
**
** Set the value element of an Ensembl Meta-Information.
**
** @param [u] mi [EnsPMetainformation] Ensembl Meta-Information
** @param [u] value [AjPStr] Value
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationSetValue(EnsPMetainformation mi, AjPStr value)
{
    if(!mi)
        return ajFalse;

    ajStrDel(&mi->Value);

    if(value)
        mi->Value = ajStrNewRef(value);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Meta-Information.
**
** @fdata [EnsPMetainformation]
**
** @nam3rule Trace Report Ensembl Meta-Information elements to debug file
**
** @argrule Trace mi [const EnsPMetainformation] Ensembl Meta-Information
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensMetainformationTrace **********************************************
**
** Trace an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationTrace(const EnsPMetainformation mi, ajuint level)
{
    AjPStr indent = NULL;

    if(!mi)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensMetainformationTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Key '%S'\n"
            "%S  Value '%S'\n"
            "%S  Species %u\n",
            indent, mi,
            indent, mi->Use,
            indent, mi->Identifier,
            indent, mi->Adaptor,
            indent, mi->Key,
            indent, mi->Value,
            indent, mi->Species);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an Ensembl Meta-Information object.
**
** @fdata [EnsPMetainformation]
**
** @nam3rule Calculate Calculate Ensembl Meta-Information values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensMetainformationCalculateMemsize ***********************************
**
** Get the memory size in bytes of an Ensembl Meta-Information.
**
** @param [r] mi [const EnsPMetainformation] Ensembl Meta-Information
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensMetainformationCalculateMemsize(const EnsPMetainformation mi)
{
    size_t size = 0;

    if(!mi)
        return 0;

    size += sizeof (EnsOMetainformation);

    if(mi->Key)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mi->Key);
    }

    if(mi->Value)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(mi->Value);
    }

    return size;
}




/* @funcstatic metainformationkeyIsSpecieskey *********************************
**
** Check whether a Meta-Information key is species-specific.
**
** @param [r] key [const AjPStr] Key
**
** @return [AjBool] ajTrue for species-specific keys,
**                  ajFalse for unspecific keys
** @@
******************************************************************************/

static AjBool metainformationkeyIsSpecieskey(const AjPStr key)
{
    register ajuint i = 0;

    AjBool result = AJTRUE;

    if(!key)
        return ajFalse;

    for(i = 0; metainformationNonSpeciesKeys[i]; i++)
        if(ajStrMatchCaseC(key, metainformationNonSpeciesKeys[i]))
        {
            result = AJFALSE;

            break;
        }

    return result;
}




/* @datasection [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**
** @nam2rule Metainformationadaptor Functions for manipulating
** Ensembl Meta-Information Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::BaseMetaContainer
** @cc CVS Revision: 1.19
** @cc CVS Tag: branch-ensembl-62
**
** @cc Bio::EnsEMBL::DBSQL::MetaContainer
** @cc CVS Revision: 1.37
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic metainformationadaptorFetchAllbyStatement **********************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Meta-Information objects.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [u] mis [AjPList] AJAX List of Ensembl Meta-Information objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool metainformationadaptorFetchAllbyStatement(
    EnsPMetainformationadaptor mia,
    const AjPStr statement,
    AjPList mis)
{
    ajuint identifier = 0;
    ajuint species    = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr key   = NULL;
    AjPStr value = NULL;

    EnsPMetainformation mi = NULL;

    if(ajDebugTest("metainformationadaptorFetchAllbyStatement"))
        ajDebug("metainformationadaptorFetchAllbyStatement\n"
                "  mia %p\n"
                "  statement %p\n"
                "  mis %p\n",
                mia,
                statement,
                mis);

    if(!mia)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!mis)
        return ajFalse;

    sqls = ensDatabaseadaptorSqlstatementNew(mia->Adaptor, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;
        species    = 0;

        key   = ajStrNew();
        value = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToUint(sqlr, &species);
        ajSqlcolumnToStr(sqlr, &key);
        ajSqlcolumnToStr(sqlr, &value);

        mi = ensMetainformationNewIni(mia, identifier, species, key, value);

        ajListPushAppend(mis, (void*) mi);

        ajStrDel(&key);
        ajStrDel(&value);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(mia->Adaptor, &sqls);

    return ajTrue;
}




/* @funcstatic metainformationadaptorCacheInit ********************************
**
** Initialise the Ensembl Meta-Information Adaptor-internal
** Meta-Information cache.
**
** The cache contains all Meta-Information objects irrespective of their
** species identifier. Standard object retrieval functions accessing the cache
** are expected to select species-specific Meta-Information objects via the
** species identifier in the corresponding Ensembl Database Adaptor, as well
** non species-specific objects. Specialist functions can still select objects
** for all species identifiers.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool metainformationadaptorCacheInit(
    EnsPMetainformationadaptor mia)
{
    ajuint* Pidentifier = NULL;

    AjPList mis      = NULL;
    AjPList list     = NULL;
    AjPStr statement = NULL;

    EnsPMetainformation mi   = NULL;
    EnsPMetainformation temp = NULL;

    if(ajDebugTest("metainformationadaptorCacheInit"))
        ajDebug("metainformationadaptorCacheInit\n"
                "  mia %p\n",
                mia);

    if(!mia)
        return ajFalse;

    statement = ajStrNewC(
        "SELECT "
        "meta.meta_id, "
        "meta.species_id, "
        "meta.meta_key, "
        "meta.meta_value "
        "FROM "
        "meta");

    mis = ajListNew();

    metainformationadaptorFetchAllbyStatement(mia, statement, mis);

    while(ajListPop(mis, (void**) &mi))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = mi->Identifier;

        temp = (EnsPMetainformation) ajTablePut(mia->CacheByIdentifier,
                                                (void*) Pidentifier,
                                                (void*) mi);

        if(temp)
        {
            ajWarn("metainformationCacheInit got more than one "
                   "Ensembl Meta-Information with identifier %u.\n",
                   temp->Identifier);

            ensMetainformationDel(&temp);
        }

        /* Insert into the key cache. */

        list = (AjPList) ajTableFetchmodV(mia->CacheByKey,
                                          (const void*) mi->Key);

        if(!list)
        {
            list = ajListNew();

            ajTablePut(mia->CacheByKey,
                       (void*) ajStrNewS(mi->Key),
                       (void*) list);
        }

        ajListPushAppend(list, (void*) ensMetainformationNewRef(mi));
    }

    ajListFree(&mis);

    ajStrDel(&statement);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Meta-Information Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Meta-Information Adaptor. The target pointer does not need to be initialised
** to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPMetainformationadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensMetainformationadaptorNew *****************************************
**
** Default constructor for an Ensembl Meta-Information Adaptor.
**
** Ensembl Object Adaptors are singleton objects in the sense that a single
** instance of an Ensembl Object Adaptor connected to a particular database is
** sufficient to instantiate any number of Ensembl Objects from the database.
** Each Ensembl Object will have a weak reference to the Object Adaptor that
** instantiated it. Therefore, Ensembl Object Adaptors should not be
** instantiated directly, but rather obtained from the Ensembl Registry,
** which will in turn call this function if neccessary.
**
** @see ensRegistryGetDatabaseadaptor
** @see ensRegistryGetMetainformationadaptor
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
**                                      or NULL
** @@
******************************************************************************/

EnsPMetainformationadaptor ensMetainformationadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPMetainformationadaptor mia = NULL;

    if(ajDebugTest("ensMetainformationadaptorNew"))
        ajDebug("ensMetainformationadaptorNew\n"
                "  dba %p\n",
                dba);

    if(!dba)
        return NULL;

    AJNEW0(mia);

    mia->Adaptor = dba;

    mia->CacheByIdentifier = ensTableuintNewLen(0);

    mia->CacheByKey = ensTablestrNewLen(0);

    metainformationadaptorCacheInit(mia);

    return mia;
}




/* @funcstatic metainformationadaptorClearIdentifierCache *********************
**
** An ajTableMapDel "apply" function to clear the Ensembl Meta-Information
** Adaptor-internal Ensembl Meta-Information cache. This function deletes the
** unsigned integer identifier key and the Ensembl Meta-Information value data.
**
** @param [u] key [void**] AJAX unsigned integer key data address
** @param [u] value [void**] Ensembl Meta-Information value data address
** @param [r] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metainformationadaptorClearIdentifierCache(void** key,
                                                       void** value,
                                                       void* cl)
{
    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    AJFREE(*key);

    ensMetainformationDel((EnsPMetainformation*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @funcstatic metainformationadaptorClearKeyCache ****************************
**
** An ajTableMapDel "apply" function to clear the Ensembl Meta-Information
** Adaptor-internal Ensembl Meta-Information cache. This function deletes the
** AJAX String Meta-Information key data and the AJAX List value data, as well
** as the Ensembl Meta-Information data from the AJAX List.
**
** @param [u] key [void**] AJAX String key data address
** @param [u] value [void**] AJAX List value data address
** @param [r] cl [void*] Standard, passed in from ajTableMapDel
** @see ajTableMapDel
**
** @return [void]
** @@
******************************************************************************/

static void metainformationadaptorClearKeyCache(void** key,
                                                void** value,
                                                void* cl)
{
    EnsPMetainformation mi = NULL;

    if(!key)
        return;

    if(!*key)
        return;

    if(!value)
        return;

    if(!*value)
        return;

    (void) cl;

    ajStrDel((AjPStr*) key);

    while(ajListPop(*((AjPList*) value), (void**) &mi))
        ensMetainformationDel(&mi);

    ajListFree((AjPList*) value);

    *key   = NULL;
    *value = NULL;

    return;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Meta-Information Adaptor object.
**
** @fdata [EnsPMetainformationadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Meta-Information Adaptor object
**
** @argrule * Pmia [EnsPMetainformationadaptor*]
** Ensembl Meta-Information Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensMetainformationadaptorDel *****************************************
**
** Default destructor for an Ensembl Meta-Information Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pmia [EnsPMetainformationadaptor*]
** Ensembl Meta-Information Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensMetainformationadaptorDel(EnsPMetainformationadaptor* Pmia)
{
    EnsPMetainformationadaptor pthis = NULL;

    if(!Pmia)
        return;

    if(!*Pmia)
        return;

    pthis = *Pmia;

    /* Clear and delete the identifier cache. */

    ajTableMapDel(pthis->CacheByIdentifier,
                  metainformationadaptorClearIdentifierCache,
                  NULL);

    ajTableFree(&pthis->CacheByIdentifier);

    /* Clear and delete the key cache. */

    ajTableMapDel(pthis->CacheByKey,
                  metainformationadaptorClearKeyCache,
                  NULL);

    ajTableFree(&pthis->CacheByKey);

    AJFREE(pthis);

    *Pmia = NULL;

    return;
}




/* @section canonical object retrieval ****************************************
**
** Functions for fetching Ensembl Meta-Information objects from an
** Ensembl SQL database.
**
** @fdata [EnsPMetainformationadaptor]
**
** @nam3rule Fetch Fetch Ensembl Meta-Information object(s)
** @nam4rule All Fetch all Ensembl Meta-Information objects
** @nam4rule Allby Fetch all Ensembl Meta-Information objects matching a
** criterion
** @nam5rule Key Fetch all by an Ensembl Meta-Information key
** @nam4rule By Fetch one Ensembl Meta-Information object matching a criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
**
** @argrule * mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @argrule All mis [AjPList] AJAX List of Ensembl Meta-Information objects
** @argrule AllbyKey key [const AjPStr] Key
** @argrule Allby mis [AjPList] AJAX List of Ensembl Meta-Information objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pmi [EnsPMetainformation*] Ensembl Meta-Information
** address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMetainformationadaptorFetchAllbyKey *******************************
**
** Fetch all Ensembl Meta-Information objects by a key.
**
** This function returns only non species-specific or species-specific
** Meta-Information objects, which species identifier matches the one of the
** underlying Ensembl Database Adaptor.
**
** The caller is responsible for deleting the Ensembl Meta-Information objects
** before deleting the AJAX List.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [r] key [const AjPStr] Key
** @param [u] mis [AjPList] AJAX List of Ensembl Meta-Information objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorFetchAllbyKey(
    EnsPMetainformationadaptor mia,
    const AjPStr key,
    AjPList mis)
{
    AjBool specieskey = AJFALSE;

    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPMetainformation mi = NULL;

    if(!mia)
        return ajFalse;

    if(!key)
        return ajFalse;

    if(!mis)
        return ajFalse;

    list = (AjPList) ajTableFetchmodV(mia->CacheByKey, (const void*) key);

    if(!list)
        return ajTrue;

    specieskey = metainformationkeyIsSpecieskey(key);

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        mi = (EnsPMetainformation) ajListIterGet(iter);

        /*
        ** For species-specific Meta-Information keys the species identifiers
        ** in the Meta-Information object and in the Database Adaptor have
        ** to match.
        */

        if(specieskey &&
           (mi->Species != ensDatabaseadaptorGetIdentifier(mia->Adaptor)))
            continue;

        ajListPushAppend(mis, (void**) ensMetainformationNewRef(mi));
    }

    ajListIterDel(&iter);

    return ajTrue;
}




/* @section accessory object retrieval ****************************************
**
** Functions for fetching objects releated to Ensembl Meta-Information objects
** from an Ensembl SQL database.
**
** @fdata [EnsPMetainformationadaptor]
**
** @nam3rule Retrieve Retrieve Ensembl Meta-Information-releated object(s)
** @nam4rule All Retrieve all Ensembl Meta-Information-releated objects
** @nam5rule Speciesnames Retrieve all species names of multi-species databases
** @nam4rule Genebuildversion Fetch the genebuild version
** @nam4rule Species Retrieve species information
** @nam5rule Commonname Retrieve the common name
** @nam5rule Productionname Retrieve the prodiuction name
** @nam5rule Scientificname Retrieve the scientific name
** @nam5rule Shortname Retrieve the short name
** @nam4rule Schemaversion Fetch the Ensembl database schema version
** @nam4rule Taxonomyidentifier Fetch the NCBI Taxonomy identifier
** @nam4rule Value Fetch an Ensembl Meta-Information value
**
** @argrule * mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @argrule AllSpeciesnames names [AjPList] AJAX List of AJAX String
** (species name) objects
** @argrule Genebuildversion Pvalue [AjPStr*] Value address
** @argrule Schemaversion Pvalue [AjPStr*] Value address
** @argrule Species Pvalue [AjPStr*] Value address
** @argrule Taxonomyidentifier Pvalue [AjPStr*] Value address
** @argrule Value key [const AjPStr] Key
** @argrule Value Pvalue [AjPStr*] Value address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMetainformationadaptorRetrieveAllSpeciesnames *********************
**
** Fetch all species names of multi-species databases.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [u] names [AjPList] AJAX List of AJAX String (species name) objects
**
** @return [AjBool] ajTrue if the Meta Information exists, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveAllSpeciesnames(
    EnsPMetainformationadaptor mia,
    AjPList names)
{
    AjIList iter = NULL;
    AjPList list = NULL;

    AjPStr key = NULL;

    EnsPMetainformation mi = NULL;

    if(!mia)
        return ajFalse;

    if(!names)
        return ajFalse;

    key = ajStrNewC("species.db_name");

    list = (AjPList) ajTableFetchmodV(mia->CacheByKey, (const void*) key);

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        mi = (EnsPMetainformation) ajListIterGet(iter);

        ajListPushAppend(names, (void*) ajStrNewS(mi->Value));
    }

    ajListIterDel(&iter);

    ajStrDel(&key);

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveGenebuildversion ********************
**
** Retrieve the genebuild version.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveGenebuildversion(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("genebuild.start_date");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveSchemaversion ***********************
**
** Retrieve the Ensembl database schema version.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveSchemaversion(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("schema_version");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveSpeciesCommonname *******************
**
** Retrieve the common name for the species.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveSpeciesCommonname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("species.common_name");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveSpeciesProductionname ***************
**
** Retrieve the production name for the species.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveSpeciesProductionname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("species.production_name");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveSpeciesScientificname ***************
**
** Retrieve the scientific name for the species.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveSpeciesScientificname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("species.scientific_name");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveSpeciesShortname ********************
**
** Retrieve the short name for the species.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveSpeciesShortname(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("species.short_name");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveTaxonomyidentifier ******************
**
** Retrieve the NCBI Taxonomy identifier.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [wP] Pvalue [AjPStr*] Value String address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveTaxonomyidentifier(
    EnsPMetainformationadaptor mia,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPStr key = NULL;

    if(!mia)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    key = ajStrNewC("species.taxonomy_id");

    if(!ensMetainformationadaptorRetrieveValue(mia, key, Pvalue))
        errors++;

    ajStrDel(&key);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @func ensMetainformationadaptorRetrieveValue *******************************
**
** Retrieve a single Ensembl Meta-Information value by a key.
**
** The function warns in case there is more than one Meta-Information value to
** a particular key and will return the first value returned by the database
** engine.
**
** The caller is responsible for deleting the AJAX String.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [r] key [const AjPStr] Key
** @param [wP] Pvalue [AjPStr*] Value address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorRetrieveValue(
    EnsPMetainformationadaptor mia,
    const AjPStr key,
    AjPStr* Pvalue)
{
    ajint errors = 0;

    AjPList mis = NULL;

    EnsPMetainformation mi = NULL;

    if(!mia)
        return ajFalse;

    if(!key)
        return ajFalse;

    if(!Pvalue)
        return ajFalse;

    if(*Pvalue)
        ajStrAssignClear(Pvalue);
    else
        *Pvalue = ajStrNew();

    mis = ajListNew();

    ensMetainformationadaptorFetchAllbyKey(mia, key, mis);

    if(ajListGetLength(mis) > 1)
    {
        errors++;

        ajWarn("ensMetainformationadaptorRetrieveValue matched %d "
               "'meta.meta_value' rows via 'meta.meta_key' '%S', "
               "but only the first value was returned.\n",
               ajListGetLength(mis), key);
    }

    ajListPeekFirst(mis, (void**) &mi);

    ajStrAssignS(Pvalue, ensMetainformationGetValue(mi));

    while(ajListPop(mis, (void**) &mi))
        ensMetainformationDel(&mi);

    ajListFree(&mis);

    if(errors)
        return ajFalse;

    return ajTrue;
}




/* @section check *************************************************************
**
** Check Ensembl Meta-Information objects.
**
** @fdata [EnsPMetainformationadaptor]
**
** @nam3rule Check Check Meta-Information objects
**
** @argrule * mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @argrule * key [const AjPStr] Key string
** @argrule * value [const AjPStr] Value string
**
** @valrule * [AjBool] ajTrue if the Meta-Information exists, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensMetainformationadaptorCheck ***************************************
**
** Check whether an Ensembl Meta-Information object with a particular
** key and value pair has already been stored in the database.
**
** @param [u] mia [EnsPMetainformationadaptor] Ensembl Meta-Information Adaptor
** @param [r] key [const AjPStr] Key
** @param [r] value [const AjPStr] Value
**
** @return [AjBool] ajTrue if the Meta-Information exists, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensMetainformationadaptorCheck(
    EnsPMetainformationadaptor mia,
    const AjPStr key,
    const AjPStr value)
{
    AjBool match = AJFALSE;
    AjBool specieskey = AJFALSE;

    AjIList iter = NULL;
    AjPList list = NULL;

    EnsPMetainformation mi = NULL;

    if(!mia)
        return ajFalse;

    if(!key)
        return ajFalse;

    if(!value)
        return ajFalse;

    list = (AjPList) ajTableFetchmodV(mia->CacheByKey, (const void*) key);

    if(!list)
        return ajFalse;

    specieskey = metainformationkeyIsSpecieskey(key);

    iter = ajListIterNew(list);

    while(!ajListIterDone(iter))
    {
        mi = (EnsPMetainformation) ajListIterGet(iter);

        /*
        ** For species-specific Meta-Information keys the species identifiers
        ** in the Meta-Information object and in the Database Adaptor have
        ** to match.
        */

        if(specieskey &&
           (mi->Species != ensDatabaseadaptorGetIdentifier(mia->Adaptor)))
            continue;

        if(ajStrMatchS(mi->Value, value))
        {
            match = ajTrue;

            break;
        }
    }

    ajListIterDel(&iter);

    return match;
}

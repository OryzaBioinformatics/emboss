#ifdef __cplusplus
extern "C"
{
#endif

#ifndef enscoordsystem_h
#define enscoordsystem_h

#include "ensdatabaseadaptor.h"
#include "enstable.h"




/* @data EnsPCoordsystemadaptor ***********************************************
**
** Ensembl Coordinate System Adaptor
**
** @alias EnsSCoordsystemadaptor
** @alias EnsOCoordsystemadaptor
**
** @attr Adaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @attr CacheByIdentifier [AjPTable] Database identifier cache
** @attr CacheByName [AjPTable] Name cache
** @attr CacheByRank [AjPTable] Rank cache
** @attr CacheByDefault [AjPTable] Default Ensembl Coordinate Systems
** @attr MappingPaths [AjPTable] Mapping paths between coordinate systems
** @attr ExternalToInternal [AjPTable] External to internal Sequence Regions
** @attr InternalToExternal [AjPTable] Internal to external Sequence Regions
** @attr SeqLevel [void*] Sequence-level Ensembl Coordinate System
** @attr TopLevel [void*] Top-level Ensembl Coordinate System
** @@
******************************************************************************/

typedef struct EnsSCoordsystemadaptor
{
    EnsPDatabaseadaptor Adaptor;
    AjPTable CacheByIdentifier;
    AjPTable CacheByName;
    AjPTable CacheByRank;
    AjPTable CacheByDefault;
    AjPTable MappingPaths;
    AjPTable ExternalToInternal;
    AjPTable InternalToExternal;
    void *SeqLevel;
    void *TopLevel;
} EnsOCoordsystemadaptor;

#define EnsPCoordsystemadaptor EnsOCoordsystemadaptor*




/* @data EnsPCoordsystem ******************************************************
**
** Ensembl Coordinate System
**
** @alias EnsSCoordsystem
** @alias EnsOCoordsystem
**
** @attr Use [ajuint] Use counter
** @cc Bio::EnsEMBL::Storable
** @attr Identifier [ajuint] Internal SQL database identifier (primary key)
** @attr Adaptor [EnsPCoordsystemadaptor] Ensembl Coordinate System Adaptor
** @cc Bio::EnsEMBL::Coordsystem
** @attr Name [AjPStr] Coordinate System name
** @attr Version [AjPStr] Coordinate System version
** @attr Default [AjBool] Default Coordinate System version of this name
** @attr SequenceLevel [AjBool] Sequence-level attribute
** @attr TopLevel [AjBool] Top-level attribute
** @attr Rank [ajuint] Coordinate System rank
** @@
******************************************************************************/

typedef struct EnsSCoordsystem
{
    ajuint Use;
    ajuint Identifier;
    EnsPCoordsystemadaptor Adaptor;
    AjPStr Name;
    AjPStr Version;
    AjBool Default;
    AjBool SequenceLevel;
    AjBool TopLevel;
    ajuint Rank;
} EnsOCoordsystem;

#define EnsPCoordsystem EnsOCoordsystem*




/*
** Prototype definitions
*/

/* Ensembl Coordinate System */

EnsPCoordsystem ensCoordsystemNew(EnsPCoordsystemadaptor adaptor,
                                  ajuint identifier,
                                  AjPStr name,
                                  AjPStr version,
                                  ajuint rank,
                                  AjBool deflt,
                                  AjBool toplvl,
                                  AjBool seqlvl);

EnsPCoordsystem ensCoordsystemNewObj(EnsPCoordsystem object);

EnsPCoordsystem ensCoordsystemNewRef(EnsPCoordsystem cs);

void ensCoordsystemDel(EnsPCoordsystem* Pcs);

EnsPCoordsystemadaptor ensCoordsystemGetAdaptor(const EnsPCoordsystem cs);

ajuint ensCoordsystemGetIdentifier(const EnsPCoordsystem cs);

const AjPStr ensCoordsystemGetName(const EnsPCoordsystem cs);

const AjPStr ensCoordsystemGetVersion(const EnsPCoordsystem cs);

AjBool ensCoordsystemGetDefault(const EnsPCoordsystem cs);

#define ensCoordsystemIsDefault ensCoordsystemGetDefault

AjBool ensCoordsystemGetSeqLevel(const EnsPCoordsystem cs);

#define ensCoordsystemIsSeqLevel ensCoordsystemGetSeqLevel

AjBool ensCoordsystemGetTopLevel(const EnsPCoordsystem cs);

#define ensCoordsystemIsTopLevel ensCoordsystemGetTopLevel

ajuint ensCoordsystemGetRank(const EnsPCoordsystem cs);

AjBool ensCoordsystemSetAdaptor(EnsPCoordsystem cs,
                                EnsPCoordsystemadaptor csa);

AjBool ensCoordsystemSetIdentifier(EnsPCoordsystem cs, ajuint identifier);

AjBool ensCoordsystemTrace(const EnsPCoordsystem cs, ajuint level);

AjBool ensCoordsystemMappingPathTrace(const AjPList css, ajuint level);

AjBool ensCoordsystemMatch(const EnsPCoordsystem cs1,
                           const EnsPCoordsystem cs2);

ajuint ensCoordsystemGetMemSize(const EnsPCoordsystem cs);

AjPStr ensCoordsystemGetSpecies(EnsPCoordsystem cs);

/* Ensembl Coordinate System Adaptor */

EnsPCoordsystemadaptor ensCoordsystemadaptorNew(EnsPDatabaseadaptor dba);

void ensCoordsystemadaptorDel(EnsPCoordsystemadaptor* Pcsa);

EnsPDatabaseadaptor ensCoordsystemadaptorGetDatabaseadaptor(
    const EnsPCoordsystemadaptor adaptor);

AjBool ensCoordsystemadaptorFetchAll(
    const EnsPCoordsystemadaptor adaptor,
    AjPList cslist);

AjBool ensCoordsystemadaptorFetchAllByName(
    const EnsPCoordsystemadaptor adaptor,
    const AjPStr name,
    AjPList cslist);

AjBool ensCoordsystemadaptorFetchByIdentifier(
    const EnsPCoordsystemadaptor adaptor,
    ajuint identifier,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchByName(
    const EnsPCoordsystemadaptor adaptor,
    const AjPStr name,
    const AjPStr version,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchByRank(
    const EnsPCoordsystemadaptor adaptor,
    ajuint rank,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchSeqLevel(
    const EnsPCoordsystemadaptor adaptor,
    EnsPCoordsystem *Pcs);

AjBool ensCoordsystemadaptorFetchTopLevel(
    const EnsPCoordsystemadaptor adaptor,
    EnsPCoordsystem *Pcs);

const AjPList ensCoordsystemadaptorGetMappingPath(
    const EnsPCoordsystemadaptor adaptor,
    EnsPCoordsystem cs1,
    EnsPCoordsystem cs2);

ajuint ensCoordsystemadaptorGetExternalSeqregionIdentifier(
    const EnsPCoordsystemadaptor adaptor,
    ajuint srid);

ajuint ensCoordsystemadaptorGetInternalSeqregionIdentifier(
    const EnsPCoordsystemadaptor adaptor,
    ajuint srid);

/*
** End of prototype definitions
*/




#endif

#ifdef __cplusplus
}
#endif

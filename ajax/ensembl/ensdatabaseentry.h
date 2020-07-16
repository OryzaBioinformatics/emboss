
#ifndef ENSDATABASEENTRY_H
#define ENSDATABASEENTRY_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ensanalysis.h"
#include "ensexternaldatabase.h"

AJ_BEGIN_DECLS




/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */




/* @const EnsEOntologylinkageType *********************************************
**
** Ensembl Ontology Linkage Type enumeration
**
******************************************************************************/

typedef enum EnsOOntologylinkageType
{
    ensEOntologylinkageNULL,
    ensEOntologylinkageIC,  /* Inferred by Curator */
    ensEOntologylinkageIDA, /* Inferred from Direct Assay */
    ensEOntologylinkageIEA, /* Inferred from Electronic Annotation */
    ensEOntologylinkageIEP, /* Inferred from Expression Pattern */
    ensEOntologylinkageIGI, /* Inferred from Genetic Interaction */
    ensEOntologylinkageIMP, /* Inferred from Mutant Phenotype */
    ensEOntologylinkageIPI, /* Inferred from Physical Interaction */
    ensEOntologylinkageISS, /* Inferred from Sequence or Structural Similarity */
    ensEOntologylinkageNAS, /* Non-traceable Author Statement */
    ensEOntologylinkageND,  /* No biological Data available */
    ensEOntologylinkageTAS, /* Traceable Author Statement */
    ensEOntologylinkageNR,  /* Not Recorded */
    ensEOntologylinkageRCA, /* Inferred from Reviewed Computational Analysis */
    ensEOntologylinkageEXP, /* Inferred from Experiment */
    ensEOntologylinkageISO, /* Inferred from Sequence Orthology */
    ensEOntologylinkageISA, /* Inferred from Sequence Alignment */
    ensEOntologylinkageISM, /* Inferred from Sequence Model */
    ensEOntologylinkageIGC  /* Inferred from Genomic Context */
} EnsEOntologylinkageType;




/* ==================================================================== */
/* ========================== public data ============================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================= public functions =========================== */
/* ==================================================================== */

/*
** Prototype definitions
*/

/* Ensembl External Reference */

EnsPExternalreference ensExternalreferenceNewCpy(
    const EnsPExternalreference er);

EnsPExternalreference ensExternalreferenceNewIni(
    ajuint identifier,
    EnsPAnalysis analysis,
    EnsPExternaldatabase edb,
    AjPStr primaryid,
    AjPStr displayid,
    AjPStr version,
    AjPStr description,
    AjPStr linkageannotation,
    AjPStr infotext,
    EnsEExternalreferenceInfotype erit,
    EnsEExternalreferenceObjecttype erot,
    ajuint objectid);

EnsPExternalreference ensExternalreferenceNewRef(
    EnsPExternalreference er);

void ensExternalreferenceDel(
    EnsPExternalreference* Per);

EnsPAnalysis ensExternalreferenceGetAnalysis(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetDescription(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetDisplayidentifier(
    const EnsPExternalreference er);

EnsPExternaldatabase ensExternalreferenceGetExternaldatabase(
    const EnsPExternalreference er);

ajuint ensExternalreferenceGetIdentifier(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetInfotext(
    const EnsPExternalreference er);

EnsEExternalreferenceInfotype ensExternalreferenceGetInfotype(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetLinkageannotation(
    const EnsPExternalreference er);

ajuint ensExternalreferenceGetObjectidentifier(
    const EnsPExternalreference er);

EnsEExternalreferenceObjecttype ensExternalreferenceGetObjecttype(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetPrimaryidentifier(
    const EnsPExternalreference er);

AjPStr ensExternalreferenceGetVersion(
    const EnsPExternalreference er);

AjBool ensExternalreferenceSetAnalysis(EnsPExternalreference er,
                                       EnsPAnalysis analysis);

AjBool ensExternalreferenceSetDescription(EnsPExternalreference er,
                                          AjPStr description);

AjBool ensExternalreferenceSetDisplayidentifier(EnsPExternalreference er,
                                                AjPStr displayid);

AjBool ensExternalreferenceSetExternaldatabase(EnsPExternalreference er,
                                               EnsPExternaldatabase edb);

AjBool ensExternalreferenceSetIdentifier(EnsPExternalreference er,
                                         ajuint identifier);

AjBool ensExternalreferenceSetInfotext(EnsPExternalreference er,
                                       AjPStr infotext);

AjBool ensExternalreferenceSetInfotype(EnsPExternalreference er,
                                       EnsEExternalreferenceInfotype erit);

AjBool ensExternalreferenceSetLinkageannotation(EnsPExternalreference er,
                                                AjPStr linkageannotation);

AjBool ensExternalreferenceSetObjectidentifier(EnsPExternalreference er,
                                               ajuint objectid);

AjBool ensExternalreferenceSetObjecttype(EnsPExternalreference er,
                                         EnsEExternalreferenceObjecttype erot);

AjBool ensExternalreferenceSetPrimaryidentifier(EnsPExternalreference er,
                                                AjPStr primaryid);

AjBool ensExternalreferenceSetVersion(EnsPExternalreference er,
                                      AjPStr version);

AjBool ensExternalreferenceTrace(const EnsPExternalreference er, ajuint level);

size_t ensExternalreferenceCalculateMemsize(const EnsPExternalreference er);

EnsEExternalreferenceInfotype ensExternalreferenceInfotypeFromStr(
    const AjPStr infotype);

const char* ensExternalreferenceInfotypeToChar(
    EnsEExternalreferenceInfotype erit);

EnsEExternalreferenceObjecttype ensExternalreferenceObjecttypeFromStr(
    const AjPStr objecttype);

const char* ensExternalreferenceObjecttypeToChar(
    EnsEExternalreferenceObjecttype erot);

/* Ensembl Identity Reference */

EnsPIdentityreference ensIdentityreferenceNewCpy(
    const EnsPIdentityreference ir);

EnsPIdentityreference ensIdentityreferenceNewIni(AjPStr cigar,
                                                 ajint qstart,
                                                 ajint qend,
                                                 ajint qidentity,
                                                 ajint tstart,
                                                 ajint tend,
                                                 ajint tidentity,
                                                 double score,
                                                 double evalue);

EnsPIdentityreference ensIdentityreferenceNewRef(
    EnsPIdentityreference ir);

void ensIdentityreferenceDel(EnsPIdentityreference* Pir);

AjPStr ensIdentityreferenceGetCigar(const EnsPIdentityreference ir);

double ensIdentityreferenceGetEvalue(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetQueryEnd(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetQueryIdentity(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetQueryStart(const EnsPIdentityreference ir);

double ensIdentityreferenceGetScore(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetTargetEnd(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetTargetIdentity(const EnsPIdentityreference ir);

ajint ensIdentityreferenceGetTargetStart(const EnsPIdentityreference ir);

AjBool ensIdentityreferenceSetCigar(EnsPIdentityreference ir,
                                    AjPStr cigar);

AjBool ensIdentityreferenceSetEvalue(EnsPIdentityreference ir,
                                     double evalue);

AjBool ensIdentityreferenceSetQueryEnd(EnsPIdentityreference ir,
                                       ajint qend);

AjBool ensIdentityreferenceSetQueryIdentity(EnsPIdentityreference ir,
                                            ajint qidentity);

AjBool ensIdentityreferenceSetQueryStart(EnsPIdentityreference ir,
                                         ajint qstart);

AjBool ensIdentityreferenceSetScore(EnsPIdentityreference ir,
                                    double score);

AjBool ensIdentityreferenceSetTargetEnd(EnsPIdentityreference ir,
                                        ajint tend);

AjBool ensIdentityreferenceSetTargetIdentity(EnsPIdentityreference ir,
                                             ajint tidentity);

AjBool ensIdentityreferenceSetTargetStart(EnsPIdentityreference ir,
                                          ajint tstart);

AjBool ensIdentityreferenceTrace(const EnsPIdentityreference ir, ajuint level);

size_t ensIdentityreferenceCalculateMemsize(const EnsPIdentityreference ir);

/* Ensembl Ontology Linkage */

EnsPOntologylinkage ensOntologylinkageNewCpy(
    const EnsPOntologylinkage ol);

EnsPOntologylinkage ensOntologylinkageNewIni(
    AjPStr linkagetype,
    EnsPDatabaseentry source);

EnsPOntologylinkage ensOntologylinkageNewRef(
    EnsPOntologylinkage ol);

void ensOntologylinkageDel(EnsPOntologylinkage* Pol);

/* Ensembl Ontology Linkage Type */

EnsEOntologylinkageType ensOntologylinkageTypeFromStr(const AjPStr type);

const char* ensOntologylinkageTypeToChar(EnsEOntologylinkageType olt);

/* Ensembl Database Entry */

EnsPDatabaseentry ensDatabaseentryNewCpy(const EnsPDatabaseentry dbe);

EnsPDatabaseentry ensDatabaseentryNewIni(
    EnsPDatabaseentryadaptor dbea,
    ajuint identifier,
    EnsPAnalysis analysis,
    EnsPExternaldatabase edb,
    AjPStr primaryid,
    AjPStr displayid,
    AjPStr version,
    AjPStr description,
    AjPStr linkageannotation,
    AjPStr infotext,
    EnsEExternalreferenceInfotype erit,
    EnsEExternalreferenceObjecttype erot,
    ajuint objectid);

EnsPDatabaseentry ensDatabaseentryNewRef(EnsPDatabaseentry dbe);

void ensDatabaseentryDel(EnsPDatabaseentry* Pdbe);

EnsPDatabaseentryadaptor ensDatabaseentryGetAdaptor(
    const EnsPDatabaseentry dbe);

ajuint ensDatabaseentryGetIdentifier(
    const EnsPDatabaseentry dbe);

const AjPList ensDatabaseentryGetOntologylinkages(
    const EnsPDatabaseentry dbe);

AjPList ensDatabaseentryGetSynonyms(
    const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryTrace(const EnsPDatabaseentry dbe, ajuint level);

size_t ensDatabaseentryCalculateMemsize(const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbDisplayname(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbName(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDbRelease(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDescription(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetDisplayidentifier(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetInfotext(
    const EnsPDatabaseentry dbe);

EnsEExternalreferenceInfotype ensDatabaseentryGetInfotype(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetLinkageannotation(
    const EnsPDatabaseentry dbe);

ajuint ensDatabaseentryGetObjectidentifier(
    const EnsPDatabaseentry dbe);

EnsEExternalreferenceObjecttype ensDatabaseentryGetObjecttype(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetPrimaryidentifier(
    const EnsPDatabaseentry dbe);

ajint ensDatabaseentryGetPriority(
    const EnsPDatabaseentry dbe);

EnsEExternaldatabaseStatus ensDatabaseentryGetStatus(
    const EnsPDatabaseentry dbe);

EnsEExternaldatabaseType ensDatabaseentryGetType(
    const EnsPDatabaseentry dbe);

AjPStr ensDatabaseentryGetVersion(
    const EnsPDatabaseentry dbe);

AjBool ensDatabaseentryAddOntologylinkage(EnsPDatabaseentry dbe,
                                          AjPStr linkagetype,
                                          EnsPDatabaseentry source);

AjBool ensDatabaseentryClearOntologylinkages(EnsPDatabaseentry dbe);

AjBool ensDatabaseentryFetchAllDependents(
    EnsPDatabaseentry dbe,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllDependentsByGene(
    EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllDependentsByTranscript(
    EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllDependentsByTranslation(
    EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllLinkagetypes(const EnsPDatabaseentry dbe,
                                            AjPList types);

AjBool ensDatabaseentryFetchAllMasters(
    EnsPDatabaseentry dbe,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllMastersByGene(
    EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllMastersByTranscript(
    EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes);

AjBool ensDatabaseentryFetchAllMastersByTranslation(
    EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes);

AjBool ensTableDatabaseentryClear(AjPTable table);

AjBool ensTableDatabaseentryDelete(AjPTable* Ptable);

/* Ensembl Database Entry Adaptor */

EnsPDatabaseentryadaptor ensRegistryGetDatabaseentryadaptor(
    EnsPDatabaseadaptor dba);

EnsPDatabaseentryadaptor ensDatabaseentryadaptorNew(
    EnsPDatabaseadaptor dba);

void ensDatabaseentryadaptorDel(EnsPDatabaseentryadaptor* Pdbea);

EnsPDatabaseadaptor ensDatabaseentryadaptorGetDatabaseadaptor(
    const EnsPDatabaseentryadaptor dbea);

AjBool ensDatabaseentryadaptorFetchAllDependents(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllDependentsByGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllDependentsByTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllDependentsByTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllMasters(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllMastersByGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPGene gene,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllMastersByTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranscript transcript,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllMastersByTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPDatabaseentry dbe,
    const EnsPTranslation translation,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllbyDescription(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr description,
    const AjPStr dbname,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllbyGene(
    EnsPDatabaseentryadaptor dbea,
    const EnsPGene gene,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllbyObject(
    EnsPDatabaseentryadaptor dbea,
    ajuint objectidentifier,
    const AjPStr objecttype,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllbySource(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr source,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllbyTranscript(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranscript transcript,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchAllbyTranslation(
    EnsPDatabaseentryadaptor dbea,
    const EnsPTranslation translation,
    const AjPStr dbname,
    EnsEExternaldatabaseType dbtype,
    AjPList dbes);

AjBool ensDatabaseentryadaptorFetchByAccession(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    const AjPStr accession,
    EnsPDatabaseentry* Pdbe);

AjBool ensDatabaseentryadaptorFetchByIdentifier(
    EnsPDatabaseentryadaptor dbea,
    ajuint identifier,
    EnsPDatabaseentry* Pdbe);

AjBool ensDatabaseentryadaptorRetrieveAllGeneidentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorRetrieveAllGeneidentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorRetrieveAllTranscriptidentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternaldatabasename(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr dbname,
    AjPList idlist);

AjBool ensDatabaseentryadaptorRetrieveAllTranslationidentifiersByExternalname(
    EnsPDatabaseentryadaptor dbea,
    const AjPStr name,
    const AjPStr dbname,
    AjPList idlist);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSDATABASEENTRY_H */

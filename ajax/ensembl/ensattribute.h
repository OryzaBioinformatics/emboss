/* @include ensattribute ******************************************************
**
** Ensembl Attribute functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.15 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 10:29:03 $ by $Author: mks $
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

#ifndef ENSATTRIBUTE_H
#define ENSATTRIBUTE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensdata.h"

AJ_BEGIN_DECLS




/* ========================================================================= */
/* =============================== constants =============================== */
/* ========================================================================= */




/* ========================================================================= */
/* ============================== public data ============================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== public functions ============================ */
/* ========================================================================= */

/*
** Prototype definitions
*/

/* Ensembl Attribute */

EnsPAttribute ensAttributeNewCpy(const EnsPAttribute attribute);

EnsPAttribute ensAttributeNewIni(EnsPAttributetype at,
                                 AjPStr value);

EnsPAttribute ensAttributeNewRef(EnsPAttribute attribute);

void ensAttributeDel(EnsPAttribute *Pattribute);

EnsPAttributetype ensAttributeGetAttributetype(const EnsPAttribute attribute);

AjPStr ensAttributeGetValue(const EnsPAttribute attribute);

AjBool ensAttributeTrace(const EnsPAttribute attribute, ajuint level);

size_t ensAttributeCalculateMemsize(const EnsPAttribute attribute);

AjPStr ensAttributeGetCode(const EnsPAttribute attribute);

AjPStr ensAttributeGetDescription(const EnsPAttribute attribute);

AjPStr ensAttributeGetName(const EnsPAttribute attribute);

/* Ensembl Attribute Adaptor */

EnsPAttributeadaptor ensRegistryGetAttributeadaptor(
    EnsPDatabaseadaptor dba);

EnsPDatabaseadaptor ensAttributeadaptorGetDatabaseadaptor(
    EnsPAttributeadaptor ata);

AjBool ensAttributeadaptorFetchAllbyGene(
    EnsPAttributeadaptor ata,
    const EnsPGene gene,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbyOperon(
    EnsPAttributeadaptor ata,
    const EnsPOperon operon,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbyOperontranscript(
    EnsPAttributeadaptor ata,
    const EnsPOperontranscript ot,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbySeqregion(
    EnsPAttributeadaptor ata,
    const EnsPSeqregion sr,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbySlice(
    EnsPAttributeadaptor ata,
    const EnsPSlice slice,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbyTranscript(
    EnsPAttributeadaptor ata,
    const EnsPTranscript transcript,
    const AjPStr code,
    AjPList attributes);

AjBool ensAttributeadaptorFetchAllbyTranslation(
    EnsPAttributeadaptor ata,
    const EnsPTranslation translation,
    const AjPStr code,
    AjPList attributes);

/* Ensembl Attribute Type */

EnsPAttributetype ensAttributetypeNewCpy(const EnsPAttributetype at);

EnsPAttributetype ensAttributetypeNewIni(EnsPAttributetypeadaptor ata,
                                         ajuint identifier,
                                         AjPStr code,
                                         AjPStr name,
                                         AjPStr description);

EnsPAttributetype ensAttributetypeNewRef(EnsPAttributetype at);

void ensAttributetypeDel(EnsPAttributetype *Pat);

EnsPAttributetypeadaptor ensAttributetypeGetAdaptor(
    const EnsPAttributetype at);

AjPStr ensAttributetypeGetCode(
    const EnsPAttributetype at);

AjPStr ensAttributetypeGetDescription(
    const EnsPAttributetype at);

ajuint ensAttributetypeGetIdentifier(
    const EnsPAttributetype at);

AjPStr ensAttributetypeGetName(
    const EnsPAttributetype at);

AjBool ensAttributetypeSetAdaptor(EnsPAttributetype at,
                                  EnsPAttributetypeadaptor ata);

AjBool ensAttributetypeSetCode(EnsPAttributetype at,
                               AjPStr code);

AjBool ensAttributetypeSetDescription(EnsPAttributetype at,
                                      AjPStr description);

AjBool ensAttributetypeSetIdentifier(EnsPAttributetype at,
                                     ajuint identifier);

AjBool ensAttributetypeSetName(EnsPAttributetype at,
                               AjPStr name);

AjBool ensAttributetypeTrace(const EnsPAttributetype at, ajuint level);

size_t ensAttributetypeCalculateMemsize(const EnsPAttributetype at);

/* Ensembl Attribute Type Adaptor */

EnsPAttributetypeadaptor ensRegistryGetAttributetypeadaptor(
    EnsPDatabaseadaptor dba);

EnsPAttributetypeadaptor ensAttributetypeadaptorNew(
    EnsPDatabaseadaptor dba);

AjBool ensAttributetypeadaptorCacheClear(EnsPAttributetypeadaptor ata);

void ensAttributetypeadaptorDel(EnsPAttributetypeadaptor *Pata);

EnsPBaseadaptor ensAttributetypeadaptorGetBaseadaptor(
    EnsPAttributetypeadaptor ata);

EnsPDatabaseadaptor ensAttributetypeadaptorGetDatabaseadaptor(
    EnsPAttributetypeadaptor ata);

AjBool ensAttributetypeadaptorFetchAll(EnsPAttributetypeadaptor ata,
                                       AjPList ats);

AjBool ensAttributetypeadaptorFetchByCode(EnsPAttributetypeadaptor ata,
                                          const AjPStr code,
                                          EnsPAttributetype *Pat);

AjBool ensAttributetypeadaptorFetchByIdentifier(EnsPAttributetypeadaptor ata,
                                                ajuint identifier,
                                                EnsPAttributetype *Pat);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSATTRIBUTE_H */

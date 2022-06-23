/* @include ensgvsample *******************************************************
**
** Ensembl Genetic Variation Sample functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.18 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2012/08/05 10:59:46 $ by $Author: mks $
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

#ifndef ENSGVSAMPLE_H
#define ENSGVSAMPLE_H

/* ========================================================================= */
/* ============================= include files ============================= */
/* ========================================================================= */

#include "ensgvdata.h"

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

/* Ensembl Genetic Variation Sample */

EnsPGvsample ensGvsampleNewCpy(const EnsPGvsample gvs);

EnsPGvsample ensGvsampleNewIni(EnsPGvsampleadaptor gvsa,
                               ajuint identifier,
                               AjPStr name,
                               AjPStr description,
                               EnsEGvsampleDisplay display,
                               ajuint size);

EnsPGvsample ensGvsampleNewRef(EnsPGvsample gvs);

void ensGvsampleDel(EnsPGvsample *Pgvs);

EnsPGvsampleadaptor ensGvsampleGetAdaptor(const EnsPGvsample gvs);

AjPStr ensGvsampleGetDescription(const EnsPGvsample gvs);

EnsEGvsampleDisplay ensGvsampleGetDisplay(const EnsPGvsample gvs);

ajuint ensGvsampleGetIdentifier(const EnsPGvsample gvs);

AjPStr ensGvsampleGetName(const EnsPGvsample gvs);

ajuint ensGvsampleGetSize(const EnsPGvsample gvs);

AjBool ensGvsampleSetAdaptor(EnsPGvsample gvs, EnsPGvsampleadaptor adaptor);

AjBool ensGvsampleSetDescription(EnsPGvsample gvs, AjPStr description);

AjBool ensGvsampleSetDisplay(EnsPGvsample gvs, EnsEGvsampleDisplay display);

AjBool ensGvsampleSetIdentifier(EnsPGvsample gvs, ajuint identifier);

AjBool ensGvsampleSetName(EnsPGvsample gvs, AjPStr name);

AjBool ensGvsampleSetSize(EnsPGvsample gvs, ajuint size);

size_t ensGvsampleCalculateMemsize(const EnsPGvsample gvs);

AjBool ensGvsampleTrace(const EnsPGvsample gvs, ajuint level);

EnsEGvsampleDisplay ensGvsampleDisplayFromStr(const AjPStr display);

const char *ensGvsampleDisplayToChar(EnsEGvsampleDisplay gvsd);

/* Ensembl Genetic Variation Sample Adaptor */

EnsPGvsampleadaptor ensRegistryGetGvsampleadaptor(
    EnsPDatabaseadaptor dba);

EnsPGvsampleadaptor ensGvsampleadaptorNew(
    EnsPDatabaseadaptor dba);

void ensGvsampleadaptorDel(EnsPGvsampleadaptor *Pgvsa);

EnsPBaseadaptor ensGvsampleadaptorGetBaseadaptor(
    EnsPGvsampleadaptor gvsa);

EnsPDatabaseadaptor ensGvsampleadaptorGetDatabaseadaptor(
    EnsPGvsampleadaptor gvsa);

AjBool ensGvsampleadaptorFetchAllbyDisplay(
    EnsPGvsampleadaptor gvsa,
    EnsEGvsampleDisplay gvsd,
    AjPList gvss);

AjBool ensGvsampleadaptorFetchAllbyIdentifiers(
    EnsPGvsampleadaptor gvsa,
    AjPTable gvss);

AjBool ensGvsampleadaptorFetchByIdentifier(
    EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    EnsPGvsample *Pgvs);

AjBool ensGvsampleadaptorRetrieveAllIdentifiersBySynonym(
    EnsPGvsampleadaptor gvsa,
    const AjPStr synonym,
    const AjPStr source,
    AjPList identifiers);

AjBool ensGvsampleadaptorRetrieveAllSynonymsByIdentifier(
    EnsPGvsampleadaptor gvsa,
    ajuint identifier,
    const AjPStr source,
    AjPList synonyms);

/*
** End of prototype definitions
*/




AJ_END_DECLS

#endif /* !ENSGVSAMPLE_H */

/* @source Ensembl Genetic Variation Individual functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2011/07/06 21:50:28 $ by $Author: mks $
** @version $Revision: 1.29 $
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

#include "ensgvindividual.h"




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

/* @conststatic gvindividualGender ********************************************
**
** The Ensembl Genetic Variation Individual gender element is enumerated in
** both, the SQL table definition and the data structure. The following
** strings are used for conversion in database operations and correspond to
** EnsEGvindividualGender.
**
******************************************************************************/

static const char* gvindividualGender[] =
{
    "",
    "Male",
    "Female",
    "Unknown",
    (const char*) NULL
};




/* @conststatic gvindividualType **********************************************
**
** The Ensembl Genetic Variation Individual type element is enumerated in
** both, the SQL table definition and the data structure. The following
** strings are used for conversion in database operations and correspond to
** EnsEGvindividualType.
**
******************************************************************************/

static const char* gvindividualType[] =
{
    "",
    "Fully_inbred",
    "Partly_inbred",
    "Outbred",
    "Mutant",
    "Unknown",
    (const char*) NULL
};




/* @conststatic gvindividualadaptorTables *************************************
**
** Array of Ensembl Genetic Variation Individual Adaptor SQL table names
**
******************************************************************************/

static const char* gvindividualadaptorTables[] =
{
    "sample",
    "individual",
    "individual_type",
    (const char*) NULL
};




/* @conststatic gvindividualadaptorColumns ************************************
**
** Array of Ensembl Genetic Variation Individual Adaptor SQL column names
**
******************************************************************************/

static const char* gvindividualadaptorColumns[] =
{
    "sample.sample_id",
    "sample.name",
    "sample.size",
    "sample.description",
    "sample.display",
    "individual.gender",
    "individual.father_individual_sample_id",
    "individual.mother_individual_sample_id",
    "individual_type.name",
    "individual_type.description",
    (const char*) NULL
};




/* @conststatic gvindividualadaptorDefaultcondition ***************************
**
** Ensembl Genetic Variation Individual Adaptor SQL default condition
**
******************************************************************************/

static const char* gvindividualadaptorDefaultcondition =
    "sample.sample_id = individual.sample_id "
    "AND "
    "individual.individual_type_id = individual_type.individual_type_id";




/* ==================================================================== */
/* ======================== private variables ========================= */
/* ==================================================================== */




/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

static AjBool gvindividualadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvis);




/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */




/* @filesection ensgvindividual ***********************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPGvindividual] Ensembl Genetic Variation Individual *******
**
** @nam2rule Gvindividual Functions for manipulating
** Ensembl Genetic Variation Individual objects
**
** @cc Bio::EnsEMBL::Variation::Individual
** @cc CVS Revision: 1.10
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Individual by
** pointer.
** It is the responsibility of the user to first destroy any previous
** Genetic Variation Individual. The target pointer does not need to be
** initialised to NULL, but it is good programming practice to do so anyway.
**
** @fdata [EnsPGvindividual]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy gvi [const EnsPGvindividual] Ensembl Genetic Variation
**                                           Individual
** @argrule Ini gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Ini father [EnsPGvindividual] Father
** @argrule Ini mother [EnsPGvindividual] Mother
** @argrule Ini gender [EnsEGvindividualGender] Gender
** @argrule Ini type [EnsEGvindividualType] Type
** @argrule Ini description [AjPStr] Description
** @argrule Ref gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @valrule * [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @fcategory new
******************************************************************************/




/* @func ensGvindividualNewCpy ************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation
**                                         Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual or NULL
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualNewCpy(const EnsPGvindividual gvi)
{
    EnsPGvindividual pthis = NULL;

    if(!gvi)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1;

    pthis->Identifier = gvi->Identifier;

    pthis->Adaptor = gvi->Adaptor;

    pthis->Gvsample = ensGvsampleNewRef(gvi->Gvsample);

    pthis->Father = ensGvindividualNewRef(gvi->Father);

    pthis->Mother = ensGvindividualNewRef(gvi->Mother);

    pthis->Gender = gvi->Gender;

    pthis->Type = gvi->Type;

    if(gvi->Description)
        pthis->Description = ajStrNewRef(gvi->Description);

    return pthis;
}




/* @func ensGvindividualNewIni ************************************************
**
** Constructor for an Ensembl Genetic Variation Individual with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Variation::Sample::new
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @cc Bio::EnsEMBL::Variation::Individual::new
** @param [u] father [EnsPGvindividual] Father
** @param [u] mother [EnsPGvindividual] Mother
** @param [u] gender [EnsEGvindividualGender] Gender
** @param [u] type [EnsEGvindividualType] Type
** @param [u] description [AjPStr] Description
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual or NULL
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualNewIni(EnsPGvindividualadaptor gvia,
                                       ajuint identifier,
                                       EnsPGvsample gvs,
                                       EnsPGvindividual father,
                                       EnsPGvindividual mother,
                                       EnsEGvindividualGender gender,
                                       EnsEGvindividualType type,
                                       AjPStr description)
{
    EnsPGvindividual gvi = NULL;

    if(!gvs)
        return NULL;

    AJNEW0(gvi);

    gvi->Use = 1;

    gvi->Identifier = identifier;

    gvi->Adaptor = gvia;

    gvi->Gvsample = ensGvsampleNewRef(gvs);

    gvi->Father = ensGvindividualNewRef(father);

    gvi->Mother = ensGvindividualNewRef(mother);

    gvi->Gender = gender;

    gvi->Type = type;

    if(description)
        gvi->Description = ajStrNewRef(description);

    return gvi;
}




/* @func ensGvindividualNewRef ************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualNewRef(EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    gvi->Use++;

    return gvi;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
**
** @nam3rule Del Destroy (free) an Ensembl Genetic Variation Individual object
**
** @argrule * Pgvi [EnsPGvindividual*] Ensembl Genetic Variation Individual
**                                     object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvindividualDel ***************************************************
**
** Default destructor for an Ensembl Genetic Variation Individual.
**
** @param [d] Pgvi [EnsPGvindividual*] Ensembl Genetic Variation Individual
**                                     object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvindividualDel(EnsPGvindividual* Pgvi)
{
    EnsPGvindividual pthis = NULL;

    if(!Pgvi)
        return;

    if(!*Pgvi)
        return;

    if(ajDebugTest("ensGvindividualDel"))
    {
        ajDebug("ensGvindividualDel\n"
                "  *Pgvi %p\n",
                *Pgvi);

        ensGvindividualTrace(*Pgvi, 1);
    }

    pthis = *Pgvi;

    pthis->Use--;

    if(pthis->Use)
    {
        *Pgvi = NULL;

        return;
    }

    ensGvsampleDel(&pthis->Gvsample);

    ensGvindividualDel(&pthis->Father);

    ensGvindividualDel(&pthis->Mother);

    ajStrDel(&pthis->Description);

    AJFREE(pthis);

    *Pgvi = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
**
** @nam3rule Get Return Genetic Variation Individual attribute(s)
** @nam4rule Adaptor Return the Ensembl Genetic Variation Individual Adaptor
** @nam4rule Description Return the description
** @nam4rule Father Return the father Ensembl Genetic Variation Individual
** @nam4rule Gender Return the gender
** @nam4rule Gvsample Return the Ensembl Genetic Variation Sample
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Mother Return the mother Ensembl Genetic Variation Individual
** @nam4rule Type Return the type
**
** @argrule * gvi [const EnsPGvindividual] Genetic Variation Individual
**
** @valrule Adaptor [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                            Individual Adaptor or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Father [EnsPGvindividual] Ensembl Genetic Variation Individual
** or NULL
** @valrule Gender [EnsEGvindividualGender] Gender or
** ensEGvindividualGenderNULL
** @valrule Gvsample [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0
** @valrule Mother [EnsPGvindividual] Ensembl Genetic Variation Individual
** or NULL
** @valrule Type [EnsEGvindividualType] Type ensEGvindividualTypeNULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvindividualGetAdaptor ********************************************
**
** Get the Ensembl Genetic Variation Individual Adaptor element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                   Individual Adaptor or NULL
** @@
******************************************************************************/

EnsPGvindividualadaptor ensGvindividualGetAdaptor(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Adaptor;
}




/* @func ensGvindividualGetDescription ****************************************
**
** Get the description element of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [AjPStr] Description or NULL
** @@
******************************************************************************/

AjPStr ensGvindividualGetDescription(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Description;
}




/* @func ensGvindividualGetFather *********************************************
**
** Get the father Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual or NULL
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualGetFather(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Father;
}




/* @func ensGvindividualGetGender *********************************************
**
** Get the gender element of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsEGvindividualGender] Gender or ensEGvindividualGenderNULL
** @@
******************************************************************************/

EnsEGvindividualGender ensGvindividualGetGender(const EnsPGvindividual gvi)
{
    if(!gvi)
        return ensEGvindividualGenderNULL;

    return gvi->Gender;
}




/* @func ensGvindividualGetGvsample *******************************************
**
** Get the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvsample] Ensembl Genetic Variation Sample or NULL
** @@
******************************************************************************/

EnsPGvsample ensGvindividualGetGvsample(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Gvsample;
}




/* @func ensGvindividualGetIdentifier *****************************************
**
** Get the SQL database-internal identifier element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [ajuint] SQL database-internal identifier or 0
** @@
******************************************************************************/

ajuint ensGvindividualGetIdentifier(const EnsPGvindividual gvi)
{
    if(!gvi)
        return 0;

    return gvi->Identifier;
}




/* @func ensGvindividualGetMother *********************************************
**
** Get the mother Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsPGvindividual] Ensembl Genetic Variation Individual or NULL
** @@
******************************************************************************/

EnsPGvindividual ensGvindividualGetMother(const EnsPGvindividual gvi)
{
    if(!gvi)
        return NULL;

    return gvi->Mother;
}




/* @func ensGvindividualGetType ***********************************************
**
** Get the type element of an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [EnsEGvindividualType] Type or ensEGvindividualTypeNULL
** @@
******************************************************************************/

EnsEGvindividualType ensGvindividualGetType(const EnsPGvindividual gvi)
{
    if(!gvi)
        return ensEGvindividualTypeNULL;

    return gvi->Type;
}




/* @section element assignment ************************************************
**
** Functions for assigning elements of an
** Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
**
** @nam3rule Set Set one element of a Genetic Variation Population
** @nam4rule Adaptor Set the Ensembl Genetic Variation Population Adaptor
** @nam4rule Description Set the description
** @nam4rule Gender Set the gender
** @nam4rule Gvsample Set the Ensembl Geentic Variation Sample
** @nam4rule Father Set the Ensembl Genetic Variation Individual
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Mother Set the Ensembl Genetic Variation Individual
** @nam4rule Type Set the type
**
** @argrule * gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
**                                   object
** @argrule Adaptor gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
** @argrule Description description [AjPStr] Description
** @argrule Father father [EnsPGvindividual] Ensembl Genetic Variation
**                                           Individual
** @argrule Gender gender [EnsEGvindividualGender] Gender
** @argrule Gvsample gvs [EnsPGvsample] Ensembl Genetic Variation Sample
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Mother mother [EnsPGvindividual] Ensembl Genetic Variation
**                                           Individual
** @argrule Type type [EnsEGvindividualType] Type
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensGvindividualSetAdaptor ********************************************
**
** Set the Ensembl Genetic Variation Individual Adaptor element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetAdaptor(EnsPGvindividual gvi,
                                 EnsPGvindividualadaptor gvia)
{
    if(!gvi)
        return ajFalse;

    gvi->Adaptor = gvia;

    return ajTrue;
}




/* @func ensGvindividualSetDescription ****************************************
**
** Set the description element of an Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetDescription(EnsPGvindividual gvi,
                                     AjPStr description)
{
    if(!gvi)
        return ajFalse;

    ajStrDel(&gvi->Description);

    if(description)
        gvi->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensGvindividualSetFather *********************************************
**
** Set the father Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] father [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetFather(EnsPGvindividual gvi,
                                EnsPGvindividual father)
{
    if(!gvi)
        return ajFalse;

    ensGvindividualDel(&gvi->Father);

    gvi->Father = ensGvindividualNewRef(father);

    return ajTrue;
}




/* @func ensGvindividualSetGender *********************************************
**
** Set the gender element of an Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] gender [EnsEGvindividualGender] Gender
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetGender(EnsPGvindividual gvi,
                                EnsEGvindividualGender gender)
{
    if(!gvi)
        return ajFalse;

    gvi->Gender = gender;

    return ajTrue;
}




/* @func ensGvindividualSetGvsample *******************************************
**
** Set the Ensembl Genetic Variation Sample element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] gvs [EnsPGvsample] Ensembl Genetic Variation Sample
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetGvsample(EnsPGvindividual gvi,
                                  EnsPGvsample gvs)
{
    if(!gvi)
        return ajFalse;

    ensGvsampleDel(&gvi->Gvsample);

    gvi->Gvsample = ensGvsampleNewRef(gvs);

    return ajTrue;
}




/* @func ensGvindividualSetIdentifier *****************************************
**
** Set the SQL database-internal identifier element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetIdentifier(EnsPGvindividual gvi,
                                    ajuint identifier)
{
    if(!gvi)
        return ajFalse;

    gvi->Identifier = identifier;

    return ajTrue;
}




/* @func ensGvindividualSetMother *********************************************
**
** Set the mother Ensembl Genetic Variation Individual element of an
** Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] mother [EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetMother(EnsPGvindividual gvi,
                                EnsPGvindividual mother)
{
    if(!gvi)
        return ajFalse;

    ensGvindividualDel(&gvi->Mother);

    gvi->Mother = ensGvindividualNewRef(mother);

    return ajTrue;
}




/* @func ensGvindividualSetType ***********************************************
**
** Set the type element of an Ensembl Genetic Variation Individual.
**
** @param [u] gvi [EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [u] type [EnsEGvindividualType] Type
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualSetType(EnsPGvindividual gvi,
                              EnsEGvindividualType type)
{
    if(!gvi)
        return ajFalse;

    gvi->Type = type;

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
**
** @nam3rule Trace Report Ensembl Genetic Variation Individual elements to
**                 debug file
**
** @argrule Trace gvi [const EnsPGvindividual] Ensembl Genetic Variation
**                                             Individual
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensGvindividualTrace *************************************************
**
** Trace an Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualTrace(const EnsPGvindividual gvi, ajuint level)
{
    AjPStr indent = NULL;

    if(!gvi)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensGvindividualTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  Sample %p\n"
            "%S  Father %p\n"
            "%S  Mother %p\n"
            "%S  Gender '%s'\n"
            "%S  Type '%s'\n"
            "%S  Description '%S'\n",
            indent, gvi,
            indent, gvi->Use,
            indent, gvi->Identifier,
            indent, gvi->Adaptor,
            indent, gvi->Gvsample,
            indent, gvi->Father,
            indent, gvi->Mother,
            indent, ensGvindividualGenderToChar(gvi->Gender),
            indent, ensGvindividualTypeToChar(gvi->Type),
            indent, gvi->Description);

    ensGvsampleTrace(gvi->Gvsample, level + 1);

    ensGvindividualTrace(gvi->Father, level + 1);
    ensGvindividualTrace(gvi->Mother, level + 1);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating values of an
** Ensembl Genetic Variation Individual object.
**
** @fdata [EnsPGvindividual]
**
** @nam3rule Calculate Calculate Ensembl Genetic Variation Individual values
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensGvindividualCalculateMemsize **************************************
**
** Calculate the memory size in bytes of an
** Ensembl Genetic Variation Individual.
**
** @param [r] gvi [const EnsPGvindividual] Ensembl Genetic Variation Individual
**
** @return [size_t] Memory size in bytes or 0
** @@
******************************************************************************/

size_t ensGvindividualCalculateMemsize(const EnsPGvindividual gvi)
{
    size_t size = 0;

    if(!gvi)
        return 0;

    size += sizeof (EnsOGvindividual);

    size += ensGvsampleCalculateMemsize(gvi->Gvsample);

    size += ensGvindividualCalculateMemsize(gvi->Father);

    size += ensGvindividualCalculateMemsize(gvi->Mother);

    if(gvi->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(gvi->Description);
    }

    return size;
}




/* @datasection [EnsEGvindividualGender] Ensembl Genetic Variation Individual
** Gender enumeration
**
** @nam2rule Gvindividual Functions for manipulating
** Ensembl Genetic Variation Individual objects
** @nam3rule GvindividualGender Functions for manipulating
** Ensembl Genetic Variation Individual Gender enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Individual Gender
** enumeration.
**
** @fdata [EnsEGvindividualGender]
**
** @nam4rule From Ensembl Genetic Variation Individual Gender query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  gender  [const AjPStr] Gender string
**
** @valrule * [EnsEGvindividualGender] Ensembl Genetic Variation Individual
**                                     Gender enumeration or
**                                     ensEGvindividualGenderNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvindividualGenderFromStr *****************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Individual Gender
** enumeration.
**
** @param [r] gender [const AjPStr] Gender string
**
** @return [EnsEGvindividualGender] Ensembl Genetic Variation Individual gender
**                                  enumeration or ensEGvindividualGenderNULL
** @@
******************************************************************************/

EnsEGvindividualGender ensGvindividualGenderFromStr(const AjPStr gender)
{
    register EnsEGvindividualGender i = ensEGvindividualGenderNULL;

    EnsEGvindividualGender egender = ensEGvindividualGenderNULL;

    for(i = ensEGvindividualGenderNULL;
        gvindividualGender[i];
        i++)
        if(ajStrMatchC(gender, gvindividualGender[i]))
            egender = i;

    if(!egender)
        ajDebug("ensGvindividualGenderFromStr encountered "
                "unexpected string '%S'.\n", gender);

    return egender;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Individual Gender enumeration.
**
** @fdata [EnsEGvindividualGender]
**
** @nam4rule To   Return Ensembl Genetic Variation Individual Gender
**                       enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvig [EnsEGvindividualGender] Ensembl Genetic Variation
**                                           Individual Gender enumeration
**
** @valrule Char [const char*] Ensembl Genetic Variation Individual Gender
**                             C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvindividualGenderToChar ******************************************
**
** Convert an Ensembl Genetic Variation Individual Gender enumeration into a
** C-type (char*) string.
**
** @param [u] gvig [EnsEGvindividualGender] Ensembl Genetic Variation
**                                          Individual Gender enumeration
**
** @return [const char*] Ensembl Genetic Variation Individual Gender
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvindividualGenderToChar(EnsEGvindividualGender gvig)
{
    register EnsEGvindividualGender i = ensEGvindividualGenderNULL;

    for(i = ensEGvindividualGenderNULL;
        gvindividualGender[i] && (i < gvig);
        i++);

    if(!gvindividualGender[i])
        ajDebug("ensGvindividualGenderToChar encountered an "
                "out of boundary error on Ensembl "
                "Genetic Variation Individual Gender enumeration %d.\n",
                gvig);

    return gvindividualGender[i];
}




/* @datasection [EnsEGvindividualType] Ensembl Genetic Variation Individual
** Type enumeration
**
** @nam2rule Gvindividual Functions for manipulating
** Ensembl Genetic Variation Individual objects
** @nam3rule GvindividualType Functions for manipulating
** Ensembl Genetic Variation Individual Type enumerations
**
******************************************************************************/




/* @section Misc **************************************************************
**
** Functions for returning an Ensembl Genetic Variation Individual Type
** enumeration.
**
** @fdata [EnsEGvindividualType]
**
** @nam4rule From Ensembl Genetic Variation Individual Type query
** @nam5rule Str  AJAX String object query
**
** @argrule  Str  type  [const AjPStr] Type string
**
** @valrule * [EnsEGvindividualType] Ensembl Genetic Variation Individual
**                                   Type enumeration or
**                                   ensEGvindividualTypeNULL
**
** @fcategory misc
******************************************************************************/




/* @func ensGvindividualTypeFromStr *******************************************
**
** Convert an AJAX String into an Ensembl Genetic Variation Individual Type
** enumeration.
**
** @param [r] type [const AjPStr] Type string
**
** @return [EnsEGvindividualType] Ensembl Genetic Variation Individual Type
**                                enumeration or ensEGvindividualTypeNULL
** @@
******************************************************************************/

EnsEGvindividualType ensGvindividualTypeFromStr(const AjPStr type)
{
    register EnsEGvindividualType i = ensEGvindividualTypeNULL;

    EnsEGvindividualType etype = ensEGvindividualTypeNULL;

    for(i = ensEGvindividualTypeNULL;
        gvindividualType[i];
        i++)
        if(ajStrMatchC(type, gvindividualType[i]))
            etype = i;

    if(!etype)
        ajDebug("ensGvindividualTypeFromStr encountered "
                "unexpected string '%S'.\n", type);

    return etype;
}




/* @section Cast **************************************************************
**
** Functions for returning attributes of an
** Ensembl Genetic Variation Individual Type enumeration.
**
** @fdata [EnsEGvindividualType]
**
** @nam4rule To   Return Ensembl Genetic Variation Individual Type enumeration
** @nam5rule Char Return C character string value
**
** @argrule To gvit [EnsEGvindividualType] Ensembl Genetic Variation
**                                         Individual Type enumeration
**
** @valrule Char [const char*] Ensembl Genetic Variation Individual Type
**                             C-type (char*) string
**
** @fcategory cast
******************************************************************************/




/* @func ensGvindividualTypeToChar ********************************************
**
** Convert an Ensembl Genetic Variation Individual Type enumeration into a
** C-type (char*) string.
**
** @param [u] gvit [EnsEGvindividualType] Ensembl Genetic Variation
**                                        Individual Type enumeration
**
** @return [const char*] Ensembl Genetic Variation Individual Type
**                       C-type (char*) string
** @@
******************************************************************************/

const char* ensGvindividualTypeToChar(EnsEGvindividualType gvit)
{
    register EnsEGvindividualType i = ensEGvindividualTypeNULL;

    for(i = ensEGvindividualTypeNULL;
        gvindividualType[i] && (i < gvit);
        i++);

    if(!gvindividualType[i])
        ajDebug("ensGvindividualTypeToChar encountered an "
                "out of boundary error on "
                "Ensembl Genetic Variation Individual Type enumeration %d.\n",
                gvit);

    return gvindividualType[i];
}




/* @datasection [EnsPGvindividualadaptor] Ensembl Genetic Variation Individual
** Adaptor
**
** @nam2rule Gvindividualadaptor Functions for manipulating
** Ensembl Genetic Variation Individual Adaptor objects
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor
** @cc CVS Revision: 1.16
** @cc CVS Tag: branch-ensembl-62
**
******************************************************************************/




/* @funcstatic gvindividualadaptorFetchAllbyStatement *************************
**
** Fetch all Ensembl Genetic Variation Individual objects via an SQL statement.
**
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] gvis [AjPList]
** AJAX List of Ensembl Genetic Variation Individual objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

static AjBool gvindividualadaptorFetchAllbyStatement(
    EnsPDatabaseadaptor dba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList gvis)
{
    ajuint sidentifier = 0;
    ajuint fatherid    = 0;
    ajuint motherid    = 0;
    ajuint ssize       = 0;

    EnsEGvsampleDisplay esdisplay  = ensEGvsampleDisplayNULL;
    EnsEGvindividualGender egender = ensEGvindividualGenderNULL;
    EnsEGvindividualType etype     = ensEGvindividualTypeNULL;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr sname        = NULL;
    AjPStr sdescription = NULL;
    AjPStr sdisplay     = NULL;
    AjPStr gender       = NULL;
    AjPStr tname        = NULL;
    AjPStr tdescription = NULL;

    EnsPGvindividual gvi         = NULL;
    EnsPGvindividual father      = NULL;
    EnsPGvindividual mother      = NULL;
    EnsPGvindividualadaptor gvia = NULL;

    EnsPGvsample gvs         = NULL;
    EnsPGvsampleadaptor gvsa = NULL;

    if(ajDebugTest("gvindividualadaptorFetchAllbyStatement"))
        ajDebug("gvindividualadaptorFetchAllbyStatement\n"
                "  dba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  gvis %p\n",
                dba,
                statement,
                am,
                slice,
                gvis);

    if(!dba)
        return ajFalse;

    if(!statement)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    gvia = ensRegistryGetGvindividualadaptor(dba);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        sidentifier  = 0;
        sname        = ajStrNew();
        ssize        = 0;
        sdescription = ajStrNew();
        sdisplay     = ajStrNew();
        gender       = ajStrNew();
        fatherid     = 0;
        motherid     = 0;
        tname        = ajStrNew();
        tdescription = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &sidentifier);
        ajSqlcolumnToStr(sqlr, &sname);
        ajSqlcolumnToUint(sqlr, &ssize);
        ajSqlcolumnToStr(sqlr, &sdescription);
        ajSqlcolumnToStr(sqlr, &sdisplay);
        ajSqlcolumnToStr(sqlr, &gender);
        ajSqlcolumnToUint(sqlr, &fatherid);
        ajSqlcolumnToUint(sqlr, &motherid);
        ajSqlcolumnToStr(sqlr, &tname);
        ajSqlcolumnToStr(sqlr, &tdescription);

        esdisplay = ensGvsampleDisplayFromStr(sdisplay);
        egender   = ensGvindividualGenderFromStr(gender);
        etype     = ensGvindividualTypeFromStr(tname);

        gvs = ensGvsampleNewIni(gvsa,
                                sidentifier,
                                sname,
                                sdescription,
                                esdisplay,
                                ssize);

        ensGvindividualadaptorFetchByIdentifier(gvia, fatherid, &father);
        ensGvindividualadaptorFetchByIdentifier(gvia, motherid, &mother);

        gvi = ensGvindividualNewIni(gvia,
                                    sidentifier,
                                    gvs,
                                    father,
                                    mother,
                                    egender,
                                    etype,
                                    tdescription);

        /*
        ** FIXME: Since the individual_type table has only few rows linked to
        ** SQL tables with several rows, this information could be moved into
        ** a separate object.
        */

        ajListPushAppend(gvis, (void*) gvi);

        ensGvsampleDel(&gvs);

        ensGvindividualDel(&father);
        ensGvindividualDel(&mother);

        ajStrDel(&sname);
        ajStrDel(&sdescription);
        ajStrDel(&sdisplay);
        ajStrDel(&gender);
        ajStrDel(&tname);
        ajStrDel(&tdescription);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Genetic Variation Individual Adaptor
** by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Genetic Variation Individual Adaptor. The target pointer does not
** need to be initialised to NULL, but it is good programming practice to do
** so anyway.
**
** @fdata [EnsPGvindividualadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPGvindividualadaptor]
** Ensembl Genetic Variation Individual Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensGvindividualadaptorNew ********************************************
**
** Default constructor for an Ensembl Genetic Variation Individual Adaptor.
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
** @see ensRegistryGetGvindividualadaptor
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPGvindividualadaptor]
** Ensembl Genetic Variation Individual Adaptor or NULL
** @@
******************************************************************************/

EnsPGvindividualadaptor ensGvindividualadaptorNew(
    EnsPDatabaseadaptor dba)
{
    if(!dba)
        return NULL;

    return ensBaseadaptorNew(
        dba,
        gvindividualadaptorTables,
        gvindividualadaptorColumns,
        (EnsPBaseadaptorLeftjoin) NULL,
        gvindividualadaptorDefaultcondition,
        (const char*) NULL,
        gvindividualadaptorFetchAllbyStatement);
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for an Ensembl Genetic Variation Individual Adaptor object.
**
** @fdata [EnsPGvindividualadaptor]
**
** @nam3rule Del Destroy (free) an
** Ensembl Genetic Variation Individual Adaptor object
**
** @argrule * Pgvia [EnsPGvindividualadaptor*]
** Ensembl Genetic Variation Individual Adaptor object address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensGvindividualadaptorDel ********************************************
**
** Default destructor for an Ensembl Genetic Variation Individual Adaptor.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Pgvia [EnsPGvindividualadaptor*]
** Ensembl Genetic Variation Individual Adaptor object address
**
** @return [void]
** @@
******************************************************************************/

void ensGvindividualadaptorDel(EnsPGvindividualadaptor* Pgvia)
{
    if(!Pgvia)
        return;

    ensGvsampleadaptorDel(Pgvia);

    *Pgvia = NULL;

    return;
}




/* @section element retrieval *************************************************
**
** Functions for returning elements of an
** Ensembl Genetic Variation Individual Adaptor object.
**
** @fdata [EnsPGvindividualadaptor]
**
** @nam3rule Get Return Genetic Variation Individual Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * gvia [EnsPGvindividualadaptor] Genetic Variation
**                                           Individual Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
**                                                or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensGvindividualadaptorGetBaseadaptor *********************************
**
** Get the Ensembl Base Adaptor element of an
** Ensembl Genetic Variation Individual Adaptor.
**
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @@
******************************************************************************/

EnsPBaseadaptor ensGvindividualadaptorGetBaseadaptor(
    EnsPGvindividualadaptor gvia)
{
    if(!gvia)
        return NULL;

    return gvia;
}




/* @func ensGvindividualadaptorGetDatabaseadaptor *****************************
**
** Get the Ensembl Database Adaptor element of an
** Ensembl Genetic Variation Individual Adaptor.
**
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
** @@
******************************************************************************/

EnsPDatabaseadaptor ensGvindividualadaptorGetDatabaseadaptor(
    EnsPGvindividualadaptor gvia)
{
    if(!gvia)
        return NULL;

    return ensBaseadaptorGetDatabaseadaptor(gvia);
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Genetic Variation Individual objects from an
** Ensembl SQL database.
**
** @fdata [EnsPGvindividualadaptor]
**
** @nam3rule Fetch Fetch Ensembl Genetic Variation Individual object(s)
** @nam4rule All Fetch all Ensembl Genetic Variation Individual objects
** @nam5rule Coverage Fetch all with read coverage
** @nam5rule Default Fetch all default
** @nam5rule Displayable Fetch all displayable
** @nam5rule Strains Fetch all strains
** @nam4rule Allby Fetch all Ensembl Genetic Variation Individual objects
**                 matching a criterion
** @nam5rule Display Fetch all by an Ensembl Genetic Variation Sample Display
** enumeration
** @nam5rule Gvpopulation Fetch all by an Ensembl Genetic Variation Population
** @nam5rule Name Fetch all by a name
** @nam5rule Parent Fetch all by a parent Ensembl Genetic Variation Individual
** @nam5rule Synonym Fetch all by an Ensembl Genetic Variation Sample synonym
** @nam4rule By Fetch one Ensembl Genetic Variation Individual object
**              matching a criterion
** @nam5rule Identifier Fetch by a SQL database-internal identifier
** @nam4rule Reference Fetch all reference
**
** @argrule * gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
** Individual Adaptor
** @argrule All gvis [AjPList] AJAX List of Ensembl Genetic Variation
** Individual objects
** @argrule AllbyDisplay display [EnsEGvsampleDisplay] Ensembl Genetic
** Variation Sample Display enumeration
** @argrule AllbyDisplay gvis [AjPList] AJAX List of Ensembl Genetic Variation
** Individual objects
** @argrule AllbyGvpopulation gvp [const EnsPGvpopulation] Ensembl Genetic
** Variation Population
** @argrule AllbyGvpopulation gvis [AjPList] AJAX List of Ensembl Genetic
** Variation Individual objects
** @argrule AllbyName name [const AjPStr] Ensembl Genetic Variation Sample
** name
** @argrule AllbyName gvis [AjPList] AJAX List of Ensembl Genetic Variation
** Individual objects
** @argrule AllbyParent parent [const EnsPGvindividual] Ensembl Genetic
** Variation
** @argrule AllbyParent gvis [AjPList] AJAX List of Ensembl Genetic Variation
** Individual
** @argrule AllbySynonym synonym [const AjPStr] Ensembl Genetic Variation
** Sample synonym
** @argrule AllbySynonym source [const AjPStr] Source
** @argrule AllbySynonym gvis [AjPList] AJAX List of Ensembl Genetic Variation
** Individual objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Pgvi [EnsPGvindividual*] Ensembl Genetic Variation
** Individual object address
** @argrule Reference Pgvi [EnsPGvindividual*] Ensembl Genetic Variation
** Individual object address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @func ensGvindividualadaptorFetchAllCoverage *******************************
**
** Fetch all Ensembl Genetic Variation Individual objects for which
** read coverage information is available.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_all_strains_with_coverage
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllCoverage(
    EnsPGvindividualadaptor gvia,
    AjPList gvis)
{
    ajuint identifier = 0;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvindividual gvi = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    statement = ajStrNewC(
        "SELECT "
        "DISTINCT "
        "read_coverage.sample_id "
        "FROM "
        "read_coverage");

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while(!ajSqlrowiterDone(sqli))
    {
        identifier = 0;

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);

        ensGvindividualadaptorFetchByIdentifier(gvia, identifier, &gvi);

        if(gvi)
            ajListPushAppend(gvis, (void*) gvi);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllDefault ******************************
**
** Fetch all Ensembl Genetic Variation Individual objects that should be
** displayed by default i.e. that are associated with an Ensembl Genetic
** Variation Sample Display enumeration value ensEGvsampleDisplayDefault.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     get_default_strains
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllDefault(
    EnsPGvindividualadaptor gvia,
    AjPList gvis)
{
    if(!gvia)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    return ensGvindividualadaptorFetchAllbyDisplay(
        gvia,
        ensEGvsampleDisplayDefault,
        gvis);
}




/* @func ensGvindividualadaptorFetchAllDisplayable ****************************
**
** Fetch all displayable Ensembl Genetic Variation Individual objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     get_display_strains
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllDisplayable(
    EnsPGvindividualadaptor gvia,
    AjPList gvis)
{
    EnsPGvindividual gvi = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    /* Fetch the reference Ensembl Genetic Variation Individual. */

    ensGvindividualadaptorFetchReference(gvia, &gvi);

    ajListPushAppend(gvis, (void*) gvi);

    /* Fetch all default Ensembl Genetic Variation Individual objects. */

    ensGvindividualadaptorFetchAllDefault(gvia, gvis);

    /* Fetch all display Ensembl Genetic Variation Individual objects. */

    ensGvindividualadaptorFetchAllbyDisplay(gvia,
                                            ensEGvsampleDisplayDisplayable,
                                            gvis);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllStrains ********************************
**
** Fetch all Ensembl Genetic Variation Individual objects that are strains
** (i.e. individual_type.name = 'fully_inbred').
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::fetch_all_strains
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllStrains(
    EnsPGvindividualadaptor gvia,
    AjPList gvis)
{
    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    constraint = ajStrNewC("individual_type.name = 'fully_inbred'");

    ensBaseadaptorFetchAllbyConstraint(gvia,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvis);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllbyDisplay ******************************
**
** Fetch all Ensembl Genetic Variation Individual objects by an
** Ensembl Genetic Variation Sample Display enumeration.
**
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
** Individual Adaptor
** @param [u] display [EnsEGvsampleDisplay] Ensembl Genetic Variation Sample
** Display enumeration
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
** objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllbyDisplay(
    EnsPGvindividualadaptor gvia,
    EnsEGvsampleDisplay display,
    AjPList gvis)
{
    AjBool result = AJFALSE;

    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    constraint = ajFmtStr(
        "sample.display = '%s'",
        ensGvsampleDisplayToChar(display));

    result = ensBaseadaptorFetchAllbyConstraint(
        gvia,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        gvis);

    ajStrDel(&constraint);

    return result;
}




/* @func ensGvindividualadaptorFetchAllbyGvpopulation *************************
**
** Fetch all Ensembl Genetic Variation Individual objects by an
** Ensembl Genetic Variation Population.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_all_by_Population
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] gvp [const EnsPGvpopulation] Ensembl Genetic Variation Population
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllbyGvpopulation(
    EnsPGvindividualadaptor gvia,
    const EnsPGvpopulation gvp,
    AjPList gvis)
{
    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvia)
        return ajFalse;

    if(!gvp)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    if(!gvp->Identifier)
    {
        ajDebug("ensGvindividualadaptorFetchAllbyGvpopulation "
                "cannot fetch Population for Individual "
                "without identifier.\n");

        return ajFalse;
    }

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'individual_population' table is
    ** required.
    */

    statement = ajFmtStr(
        "SELECT "
        "sample.sample_id, "
        "sample.name, "
        "sample.size, "
        "sample.description, "
        "sample.display, "
        "individual.gender, "
        "individual.father_individual_sample_id, "
        "individual.mother_individual_sample_id, "
        "individual_type.name, "
        "individual_type.description "
        "FROM "
        "sample, "
        "individual, "
        "individual_type, "
        "individual_population "
        "WHERE "
        "sample.sample_id = individual.sample_id "
        "AND "
        "individual.individual_type_id = "
        "individual_type.individual_type_id "
        "AND "
        "individual.sample_id = "
        "individual_population.individual_sample_id "
        "AND "
        "individual_population.population_sample_id = %u",
        gvp->Identifier);

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    gvindividualadaptorFetchAllbyStatement(dba,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvis);

    ajStrDel(&statement);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllbyName *********************************
**
** Fetch all Ensembl Genetic Variation Individual objects by an
** Ensembl Genetic Variation Sample name.
**
** Individual names may be non-unique, therefore the function returns an
** AJAX List of Ensembl Genetic Variation Individual objects.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::fetch_all_by_name
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] name [const AjPStr] Ensembl Genetic Variation Sample name
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllbyName(
    EnsPGvindividualadaptor gvia,
    const AjPStr name,
    AjPList gvis)
{
    char* txtname = NULL;

    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!name)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    ensBaseadaptorEscapeC(gvia, &txtname, name);

    constraint = ajFmtStr("sample.name = '%s'", txtname);

    ajCharDel(&txtname);

    ensBaseadaptorFetchAllbyConstraint(gvia,
                                       constraint,
                                       (EnsPAssemblymapper) NULL,
                                       (EnsPSlice) NULL,
                                       gvis);

    ajStrDel(&constraint);

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllbyParent *******************************
**
** Fetch all Ensembl Genetic Variation Individual objects by a parent
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_all_by_parent_Individual
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] parent [const EnsPGvindividual] Ensembl Genetic Variation
**                                            Individual
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllbyParent(
    EnsPGvindividualadaptor gvia,
    const EnsPGvindividual parent,
    AjPList gvis)
{
    AjPStr constraint = NULL;

    if(!gvia)
        return ajFalse;

    if(!parent)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    if(!parent->Identifier)
    {
        ajDebug("ensGvindividualadaptorFetchAllbyParent cannot get Ensembl "
                "Genetic Variation Individual objects for parent Individual "
                "without identifier.\n");

        return ajFalse;
    }

    switch(parent->Gender)
    {
        case ensEGvindividualGenderMale:

            constraint = ajFmtStr(
                "individual.father_individual_sample_id = %u",
                parent->Identifier);

            ensBaseadaptorFetchAllbyConstraint(gvia,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               gvis);

            ajStrDel(&constraint);

            return ajTrue;

            break;

        case ensEGvindividualGenderFemale:

            constraint = ajFmtStr(
                "individual.mother_individual_sample_id = %u",
                parent->Identifier);

            ensBaseadaptorFetchAllbyConstraint(gvia,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               gvis);

            ajStrDel(&constraint);

            return ajTrue;

            break;

        default:

            /* The gender is unknown, so assume this is the father. */

            constraint = ajFmtStr(
                "individual.father_individual_sample_id = %u",
                parent->Identifier);

            ensBaseadaptorFetchAllbyConstraint(gvia,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               gvis);

            ajStrDel(&constraint);

            if(ajListGetLength(gvis))
                return ajTrue;

            /* No result for the father, so assume it is the mother. */

            constraint = ajFmtStr(
                "individual.mother_individual_sample_id = %u",
                parent->Identifier);

            ensBaseadaptorFetchAllbyConstraint(gvia,
                                               constraint,
                                               (EnsPAssemblymapper) NULL,
                                               (EnsPSlice) NULL,
                                               gvis);

            ajStrDel(&constraint);

            if(ajListGetLength(gvis))
                return ajTrue;
    }

    /* Return regardless of a result. */

    return ajTrue;
}




/* @func ensGvindividualadaptorFetchAllbySynonym ******************************
**
** Fetch all Ensembl Genetic Variation Individual objects by an
** Ensembl Genetic Variation Sample synonym.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     fetch_individual_by_synonym
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] synonym [const AjPStr] Ensembl Genetic Variation Sample synonym
** @param [rN] source [const AjPStr] Source
** @param [u] gvis [AjPList] AJAX List of Ensembl Genetic Variation Individual
**                           objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchAllbySynonym(
    EnsPGvindividualadaptor gvia,
    const AjPStr synonym,
    const AjPStr source,
    AjPList gvis)
{
    char* txtsource  = NULL;
    char* txtsynonym = NULL;

    AjPStr statement = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if(!gvia)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    /*
    ** NOTE: This function does not use the Ensembl Base Adaptor
    ** functionality, because an additional 'sample_synonym' table is
    ** required.
    */

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    ensDatabaseadaptorEscapeC(dba, &txtsynonym, synonym);

    if(source && ajStrGetLen(source))
    {
        ensDatabaseadaptorEscapeC(dba, &txtsource, source);

        statement = ajFmtStr(
            "SELECT "
            "sample.sample_id, "
            "sample.name, "
            "sample.size, "
            "sample.description, "
            "sample.display, "
            "individual.gender, "
            "individual.father_individual_sample_id, "
            "individual.mother_individual_sample_id, "
            "individual_type.name, "
            "individual_type.description "
            "FROM "
            "sample, "
            "individual, "
            "individual_type, "
            "sample_synonym, "
            "source "
            "WHERE "
            "sample.sample_id = individual.sample_id "
            "AND "
            "individual.individual_type_id = "
            "individual_type.individual_type_id "
            "AND "
            "individual.sample_id = "
            "sample_synonym.sample_id "
            "AND "
            "sample_synonym.name = '%s' "
            "AND "
            "sample_synonym.source_id = source.source_id "
            "AND "
            "source.name = '%s'",
            txtsynonym,
            txtsource);

        ajCharDel(&txtsource);
    }
    else
        statement = ajFmtStr(
            "SELECT "
            "sample.sample_id, "
            "sample.name, "
            "sample.size, "
            "sample.description, "
            "sample.display, "
            "individual.gender, "
            "individual.father_individual_sample_id, "
            "individual.mother_individual_sample_id, "
            "individual_type.name, "
            "individual_type.description "
            "FROM "
            "sample, "
            "individual, "
            "individual_type, "
            "sample_synonym "
            "WHERE "
            "sample.sample_id = individual.sample_id "
            "AND "
            "individual.individual_type_id = "
            "individual_type.individual_type_id "
            "AND "
            "individual.sample_id = "
            "sample_synonym.sample_id "
            "AND "
            "sample_synonym.name = '%s'",
            txtsynonym);

    ajCharDel(&txtsynonym);

    gvindividualadaptorFetchAllbyStatement(dba,
                                           statement,
                                           (EnsPAssemblymapper) NULL,
                                           (EnsPSlice) NULL,
                                           gvis);

    ajStrDel(&statement);

    return ajTrue;
}

#if AJFALSE
AjBool ensGvindividualadaptorFetchAllbySynonymOld(
    EnsPGvindividualadaptor gvia,
    const AjPStr synonym,
    const AjPStr source,
    AjPList gvis)
{
    ajuint* Pidentifier = 0;

    AjPList idlist = NULL;

    EnsPDatabaseadaptor dba = NULL;

    EnsPGvindividual gvi = NULL;

    EnsPGvsampleadaptor gvsa = NULL;

    if(!gvia)
        return ajFalse;

    if(!synonym)
        return ajFalse;

    if(!gvis)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(gvia);

    gvsa = ensRegistryGetGvsampleadaptor(dba);

    idlist = ajListNew();

    /*
    ** FIXME: Wouldn't it be more efficient if the sample_synonym table was
    ** joined in? See the ensGvindividualadaptorFetchAllbyGvpopulation function
    ** above. Alternatively, an ensGvindividualadaptorFetchAllbyIdentifiers
    ** function which acepts an AJAX Table and uses a SELECT IN () statement
    ** internally, could be used.
    */

    ensGvsampleadaptorRetrieveAllIdentifiersBySynonym(gvsa,
                                                      synonym,
                                                      source,
                                                      idlist);

    while(ajListPop(idlist, (void**) &Pidentifier))
    {
        ensGvindividualadaptorFetchByIdentifier(gvia,
                                                *Pidentifier,
                                                &gvi);

        ajListPushAppend(gvis, (void*) gvi);

        AJFREE(Pidentifier);
    }

    ajListFree(&idlist);

    return ajTrue;
}
#endif




/* @func ensGvindividualadaptorFetchByIdentifier ******************************
**
** Fetch an Ensembl Genetic Variation Individual via its
** SQL database-internal identifier.
** The caller is responsible for deleting the
** Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::fetch_by_dbID
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Pgvi [EnsPGvindividual*] Ensembl Genetic Variation
**                                      Individual address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchByIdentifier(
    EnsPGvindividualadaptor gvia,
    ajuint identifier,
    EnsPGvindividual* Pgvi)
{
    if(!gvia)
        return ajFalse;

    if(!identifier)
        return ajFalse;

    if(!Pgvi)
        return ajFalse;

    return ensBaseadaptorFetchByIdentifier(gvia, identifier, (void**) Pgvi);
}




/* @func ensGvindividualadaptorFetchReference *********************************
**
** Fetch the reference Ensembl Genetic Variation Individual.
**
** @cc Bio::EnsEMBL::Variation::DBSQL::IndividualAdaptor::
**     get_reference_strain_name
** @param [u] gvia [EnsPGvindividualadaptor] Ensembl Genetic Variation
**                                           Individual Adaptor
** @param [wP] Pgvi [EnsPGvindividual*] Ensembl Genetic Variation Individual
**                                      object address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
** @@
******************************************************************************/

AjBool ensGvindividualadaptorFetchReference(
    EnsPGvindividualadaptor gvia,
    EnsPGvindividual* Pgvi)
{
    AjPList gvis = NULL;

    EnsPGvindividual gvi = NULL;

    if(!gvia)
        return ajFalse;

    if(!Pgvi)
        return ajFalse;

    gvis = ajListNew();

    ensGvindividualadaptorFetchAllbyDisplay(gvia,
                                            ensEGvsampleDisplayReference,
                                            gvis);

    /* There should be only one reference. */

    ajListPop(gvis, (void**) Pgvi);

    while(ajListPop(gvis, (void**) &gvi))
        ensGvindividualDel(&gvi);

    ajListFree(&gvis);

    return ajTrue;
}

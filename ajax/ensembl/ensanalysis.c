/* @source ensanalysis ********************************************************
**
** Ensembl Analysis functions
**
** @author Copyright (C) 1999 Ensembl Developers
** @author Copyright (C) 2006 Michael K. Schuster
** @version $Revision: 1.59 $
** @modified 2009 by Alan Bleasby for incorporation into EMBOSS core
** @modified $Date: 2013/02/17 13:02:10 $ by $Author: mks $
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

#include "ensanalysis.h"
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




/* ========================================================================= */
/* =========================== private constants =========================== */
/* ========================================================================= */

/* @conststatic analysisadaptorKTablenames ************************************
**
** Array of Ensembl Analysis Adaptor SQL table names
**
******************************************************************************/

static const char *const analysisadaptorKTablenames[] =
{
    "analysis",
    "analysis_description",
    (const char *) NULL
};




/* @conststatic analysisadaptorKColumnnames ***********************************
**
** Array of Ensembl Analysis Adaptor SQL column names
**
******************************************************************************/

static const char *const analysisadaptorKColumnnames[] =
{
    "analysis.analysis_id",
    "analysis.created",
    "analysis.logic_name",
    "analysis.db",
    "analysis.db_version",
    "analysis.db_file",
    "analysis.program",
    "analysis.program_version",
    "analysis.program_file",
    "analysis.parameters",
    "analysis.module",
    "analysis.module_version",
    "analysis.gff_source",
    "analysis.gff_feature",
    "analysis_description.description",
    "analysis_description.display_label",
    "analysis_description.displayable",
    "analysis_description.web_data",
    (const char *) NULL
};




/* @conststatic analysisadaptorKLeftjoins *************************************
**
** Array of Ensembl Analysis Adaptor SQL LEFT JOIN conditions
**
******************************************************************************/

static const EnsOBaseadaptorLeftjoin analysisadaptorKLeftjoins[] =
{
    {
        "analysis_description",
        "analysis.analysis_id = analysis_description.analysis_id"
    },
    {(const char *) NULL, (const char *) NULL}
};




/* @conststatic analysisadaptorKFeatureClasses ********************************
**
** Correlation of Ensembl data object types and SQL table names
**
******************************************************************************/

static const char *const analysisadaptorKFeatureClasses[] =
{
    "AffyFeature", "affy_feature",
    "Densityfeature", "density_type", /* density_type.analysis_id */
    "Dnaalignfeature", "dna_align_feature",
    "Gene", "gene",
    "Markerfeature", "marker_feature",
    "Predictiontranscript", "prediction_transcript",
    "OligoFeature", "oligo_feature",
    "Proteinalignfeature", "protein_align_feature",
    "Proteinfeature", "protein_feature",
    "QtlFeature", "qtl_feature",
    "Repeatfeature", "repeat_feature",
    "Simplefeature", "simple_feature",
    (const char *) NULL, (const char *) NULL
};




/* ========================================================================= */
/* =========================== private variables =========================== */
/* ========================================================================= */




/* ========================================================================= */
/* =========================== private functions =========================== */
/* ========================================================================= */

static int listAnalysisCompareIdentifierAscending(
    const void *item1,
    const void *item2);

static int listAnalysisCompareIdentifierDescending(
    const void *item1,
    const void *item2);

static int listAnalysisCompareNameAscending(
    const void *item1,
    const void *item2);

static int listAnalysisCompareNameDescending(
    const void *item1,
    const void *item2);

static AjBool analysisadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList analyses);

static AjBool analysisadaptorCacheInit(
    EnsPAnalysisadaptor aa);

static AjBool analysisadaptorCacheInsert(
    EnsPAnalysisadaptor aa,
    EnsPAnalysis *Panalysis);

static void analysisadaptorFetchAll(const void *key,
                                    void **Pvalue,
                                    void *cl);




/* ========================================================================= */
/* ======================= All functions by section ======================== */
/* ========================================================================= */




/* @filesection ensanalysis ***************************************************
**
** @nam1rule ens Function belongs to the Ensembl library
**
******************************************************************************/




/* @datasection [EnsPAnalysis] Ensembl Analysis *******************************
**
** @nam2rule Analysis Functions for manipulating Ensembl Analysis objects
**
** @cc Bio::EnsEMBL::Analysis
** @cc CVS Revision: 1.33
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Analysis by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Analysis.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPAnalysis]
**
** @nam3rule New Constructor
** @nam4rule Cpy Constructor with existing object
** @nam4rule Ini Constructor with initial values
** @nam4rule Ref Constructor by incrementing the reference counter
**
** @argrule Cpy analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule Ini aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @argrule Ini identifier [ajuint] SQL database-internal identifier
** @argrule Ini cdate [AjPStr] Creation date
** @argrule Ini name [AjPStr] Name
** @argrule Ini databasename [AjPStr] Database name
** @argrule Ini databaseversion [AjPStr] Database version
** @argrule Ini databasefile [AjPStr] Database file
** @argrule Ini programname [AjPStr] Program name
** @argrule Ini programversion [AjPStr] Program version
** @argrule Ini programfile [AjPStr] Program file
** @argrule Ini parameters [AjPStr] Parameters
** @argrule Ini modulename [AjPStr] Module name
** @argrule Ini moduleversion [AjPStr] Module version
** @argrule Ini gffsource [AjPStr] GFF source
** @argrule Ini gfffeature [AjPStr] GFF feature
** @argrule Ini description [AjPStr] Description
** @argrule Ini displaylabel [AjPStr] Display label
** @argrule Ini webdata [AjPStr] Web data
** @argrule Ini displayable [AjBool] Displayable flag
** @argrule Ref analysis [EnsPAnalysis] Ensembl Analysis
**
** @valrule * [EnsPAnalysis] Ensembl Analysis or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAnalysisNewCpy ****************************************************
**
** Object-based constructor function, which returns an independent object.
**
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAnalysis ensAnalysisNewCpy(const EnsPAnalysis analysis)
{
    EnsPAnalysis pthis = NULL;

    if (!analysis)
        return NULL;

    AJNEW0(pthis);

    pthis->Use = 1U;

    pthis->Identifier = analysis->Identifier;

    pthis->Adaptor = analysis->Adaptor;

    if (analysis->DateCreation)
        pthis->DateCreation = ajStrNewRef(analysis->DateCreation);

    if (analysis->Name)
        pthis->Name = ajStrNewRef(analysis->Name);

    if (analysis->Databasename)
        pthis->Databasename = ajStrNewRef(analysis->Databasename);

    if (analysis->Databaseversion)
        pthis->Databaseversion = ajStrNewRef(analysis->Databaseversion);

    if (analysis->Databasefile)
        pthis->Databasefile = ajStrNewRef(analysis->Databasefile);

    if (analysis->Programname)
        pthis->Programname = ajStrNewRef(analysis->Programname);

    if (analysis->Programversion)
        pthis->Programversion = ajStrNewRef(analysis->Programversion);

    if (analysis->Programfile)
        pthis->Programfile = ajStrNewRef(analysis->Programfile);

    if (analysis->Parameters)
        pthis->Parameters = ajStrNewRef(analysis->Parameters);

    if (analysis->Modulename)
        pthis->Modulename = ajStrNewRef(analysis->Modulename);

    if (analysis->Moduleversion)
        pthis->Moduleversion = ajStrNewRef(analysis->Moduleversion);

    if (analysis->Gffsource)
        pthis->Gffsource = ajStrNewRef(analysis->Gffsource);

    if (analysis->Gfffeature)
        pthis->Gfffeature = ajStrNewRef(analysis->Gfffeature);

    if (analysis->Description)
        pthis->Description = ajStrNewRef(analysis->Description);

    if (analysis->Displaylabel)
        pthis->Displaylabel = ajStrNewRef(analysis->Displaylabel);

    if (analysis->Webdata)
        pthis->Webdata = ajStrNewRef(analysis->Webdata);

    pthis->Displayable = analysis->Displayable;

    return pthis;
}




/* @func ensAnalysisNewIni ****************************************************
**
** Constructor for an Ensembl Analysis object with initial values.
**
** @cc Bio::EnsEMBL::Storable::new
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @cc Bio::EnsEMBL::Analysis::new
** @param [u] cdate [AjPStr] Creation date
** @param [u] name [AjPStr] Name
** @param [u] databasename [AjPStr] Database name
** @param [u] databaseversion [AjPStr] Database version
** @param [u] databasefile [AjPStr] Database file
** @param [u] programname [AjPStr] Program name
** @param [u] programversion [AjPStr] Program version
** @param [u] programfile [AjPStr] Program file
** @param [u] parameters [AjPStr] Parameters
** @param [u] modulename [AjPStr] Module name
** @param [u] moduleversion [AjPStr] Module version
** @param [u] gffsource [AjPStr] GFF source
** @param [u] gfffeature [AjPStr] GFF feature
** @param [u] description [AjPStr] Description
** @param [u] displaylabel [AjPStr] Display label
** @param [u] webdata [AjPStr] Web data
** @param [r] displayable [AjBool] Displayable flag
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

EnsPAnalysis ensAnalysisNewIni(EnsPAnalysisadaptor aa,
                               ajuint identifier,
                               AjPStr cdate,
                               AjPStr name,
                               AjPStr databasename,
                               AjPStr databaseversion,
                               AjPStr databasefile,
                               AjPStr programname,
                               AjPStr programversion,
                               AjPStr programfile,
                               AjPStr parameters,
                               AjPStr modulename,
                               AjPStr moduleversion,
                               AjPStr gffsource,
                               AjPStr gfffeature,
                               AjPStr description,
                               AjPStr displaylabel,
                               AjPStr webdata,
                               AjBool displayable)
{
    EnsPAnalysis analysis = NULL;

    if (!name)
        return NULL;

    AJNEW0(analysis);

    analysis->Use = 1U;

    analysis->Identifier = identifier;

    analysis->Adaptor = aa;

    if (cdate)
        analysis->DateCreation = ajStrNewRef(cdate);

    if (name)
        analysis->Name = ajStrNewRef(name);

    if (databasename)
        analysis->Databasename = ajStrNewRef(databasename);

    if (databaseversion)
        analysis->Databaseversion = ajStrNewRef(databaseversion);

    if (databasefile)
        analysis->Databasefile = ajStrNewRef(databasefile);

    if (programname)
        analysis->Programname = ajStrNewRef(programname);

    if (programversion)
        analysis->Programversion = ajStrNewRef(programversion);

    if (programfile)
        analysis->Programfile = ajStrNewRef(programfile);

    if (parameters)
        analysis->Parameters = ajStrNewRef(parameters);

    if (modulename)
        analysis->Modulename = ajStrNewRef(modulename);

    if (moduleversion)
        analysis->Moduleversion = ajStrNewRef(moduleversion);

    if (gffsource)
        analysis->Gffsource = ajStrNewRef(gffsource);

    if (gfffeature)
        analysis->Gfffeature = ajStrNewRef(gfffeature);

    if (description)
        analysis->Description = ajStrNewRef(description);

    if (displaylabel)
        analysis->Displaylabel = ajStrNewRef(displaylabel);

    analysis->Displayable = displayable;

    if (webdata)
        analysis->Webdata = ajStrNewRef(webdata);

    return analysis;
}




/* @func ensAnalysisNewRef ****************************************************
**
** Ensembl Object referencing function, which returns a pointer to the
** Ensembl Object passed in and increases its reference count.
**
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPAnalysis] Ensembl Analysis or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysis ensAnalysisNewRef(EnsPAnalysis analysis)
{
    if (!analysis)
        return NULL;

    analysis->Use++;

    return analysis;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Del Destroy (free) an Ensembl Analysis
**
** @argrule * Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAnalysisDel *******************************************************
**
** Default destructor for an Ensembl Analysis.
**
** @param [d] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAnalysisDel(EnsPAnalysis *Panalysis)
{
    EnsPAnalysis pthis = NULL;

    if (!Panalysis)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAnalysisDel"))
    {
        ajDebug("ensAnalysisDel\n"
                "  *Panalysis %p\n",
                *Panalysis);

        ensAnalysisTrace(*Panalysis, 1);
    }
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Panalysis) || --pthis->Use)
    {
        *Panalysis = NULL;

        return;
    }

    ajStrDel(&pthis->DateCreation);
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Databasename);
    ajStrDel(&pthis->Databaseversion);
    ajStrDel(&pthis->Databasefile);
    ajStrDel(&pthis->Programname);
    ajStrDel(&pthis->Programversion);
    ajStrDel(&pthis->Programfile);
    ajStrDel(&pthis->Parameters);
    ajStrDel(&pthis->Modulename);
    ajStrDel(&pthis->Moduleversion);
    ajStrDel(&pthis->Gffsource);
    ajStrDel(&pthis->Gfffeature);
    ajStrDel(&pthis->Description);
    ajStrDel(&pthis->Displaylabel);
    ajStrDel(&pthis->Webdata);

    ajMemFree((void **) Panalysis);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Get Return Ensembl Analysis attribute(s)
** @nam4rule Adaptor Return the Ensembl Analysis Adaptor
** @nam4rule Databasefile Return the database file
** @nam4rule Databasename Return the database name
** @nam4rule Databaseversion Return the database version
** @nam4rule Date Return a date
** @nam5rule Creation Return the date of creation
** @nam4rule Description Return the description
** @nam4rule Displayable Return the displayable flag
** @nam4rule Displaylabel Return the display label
** @nam4rule Gfffeature Return the GFF feature
** @nam4rule Gffsource Return the GFF source
** @nam4rule Identifier Return the SQL database-internal identifier
** @nam4rule Modulename Return the module name
** @nam4rule Moduleversion Return the module version
** @nam4rule Name Return the name
** @nam4rule Parameters Return the parameters
** @nam4rule Programfile Return the program file
** @nam4rule Programname Return the program name
** @nam4rule Programversion Return the program version
** @nam4rule Webdata Return the web data
**
** @argrule * analysis [const EnsPAnalysis] Ensembl Analysis
**
** @valrule Adaptor [EnsPAnalysisadaptor] Ensembl Analysis Adaptor or NULL
** @valrule Databasefile [AjPStr] Database file or NULL
** @valrule Databasename [AjPStr] Database name or NULL
** @valrule Databaseversion [AjPStr] Database version or NULL
** @valrule DateCreation [AjPStr] Creation date or NULL
** @valrule Description [AjPStr] Description or NULL
** @valrule Displayable [AjBool] Displayable flag or ajFalse
** @valrule Displaylabel [AjPStr] Display label or NULL
** @valrule Gfffeature [AjPStr] GFF feature or NULL
** @valrule Gffsource [AjPStr] GFF source or NULL
** @valrule Identifier [ajuint] SQL database-internal identifier or 0U
** @valrule Modulename [AjPStr] Module name or NULL
** @valrule Moduleversion [AjPStr] Module version or NULL
** @valrule Name [AjPStr] Name or NULL
** @valrule Parameters [AjPStr] Parameters or NULL
** @valrule Programfile [AjPStr] Program file or NULL
** @valrule Programname [AjPStr] Program name or NULL
** @valrule Programversion [AjPStr] Program version or NULL
** @valrule Webdata [AjPStr] Web data or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisGetAdaptor ************************************************
**
** Get the Ensembl Analysis Adaptor member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [EnsPAnalysisadaptor] Ensembl Analysis Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysisadaptor ensAnalysisGetAdaptor(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Adaptor : NULL;
}




/* @func ensAnalysisGetDatabasefile *******************************************
**
** Get the database file member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_file
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Database file or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetDatabasefile(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Databasefile : NULL;
}




/* @func ensAnalysisGetDatabasename *******************************************
**
** Get the database name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Database name or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetDatabasename(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Databasename : NULL;
}




/* @func ensAnalysisGetDatabaseversion ****************************************
**
** Get the database version member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_version
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Database version or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetDatabaseversion(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Databaseversion : NULL;
}




/* @func ensAnalysisGetDateCreation *******************************************
**
** Get the date of creation member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::created
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Creation date or NULL
**
** @release 6.4.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetDateCreation(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->DateCreation : NULL;
}




/* @func ensAnalysisGetDescription ********************************************
**
** Get the description member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::description
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Description or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetDescription(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Description : NULL;
}




/* @func ensAnalysisGetDisplayable ********************************************
**
** Get the displayable member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::displayable
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] Displayable flag or ajFalse
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisGetDisplayable(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Displayable : ajFalse;
}




/* @func ensAnalysisGetDisplaylabel *******************************************
**
** Get the display label member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::display_label
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Display label or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetDisplaylabel(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Displaylabel : NULL;
}




/* @func ensAnalysisGetGfffeature *********************************************
**
** Get the GFF feature member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_feature
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] GFF feature or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetGfffeature(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Gfffeature : NULL;
}




/* @func ensAnalysisGetGffsource **********************************************
**
** Get the GFF source member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_source
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] GFF source or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetGffsource(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Gffsource : NULL;
}




/* @func ensAnalysisGetIdentifier *********************************************
**
** Get the SQL database-internal identifier member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [ajuint] SQL database-internal identifier or 0U
**
** @release 6.2.0
** @@
******************************************************************************/

ajuint ensAnalysisGetIdentifier(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Identifier : 0U;
}




/* @func ensAnalysisGetModulename *********************************************
**
** Get the module name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Module name or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetModulename(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Modulename : NULL;
}




/* @func ensAnalysisGetModuleversion ******************************************
**
** Get the module version member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module_version
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Module version or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetModuleversion(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Moduleversion : NULL;
}




/* @func ensAnalysisGetName ***************************************************
**
** Get the name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::logic_name
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Name or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetName(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Name : NULL;
}




/* @func ensAnalysisGetParameters *********************************************
**
** Get the parameters member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::parameters
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Parameters or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetParameters(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Parameters : NULL;
}




/* @func ensAnalysisGetProgramfile ********************************************
**
** Get the program file member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_file
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Program file or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetProgramfile(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Programfile : NULL;
}




/* @func ensAnalysisGetProgramname ********************************************
**
** Get the program name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Program name or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetProgramname(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Programname : NULL;
}




/* @func ensAnalysisGetProgramversion *****************************************
**
** Get the program version member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_version
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Program version or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetProgramversion(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Programversion : NULL;
}





/* @func ensAnalysisGetWebdata ************************************************
**
** Get the web data member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::web_data
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjPStr] Web data or NULL
**
** @release 6.3.0
** @@
******************************************************************************/

AjPStr ensAnalysisGetWebdata(const EnsPAnalysis analysis)
{
    return (analysis) ? analysis->Webdata : NULL;
}




/* @section member assignment *************************************************
**
** Functions for assigning members of an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Set Set one member of an Ensembl Analysis
** @nam4rule Adaptor Set the Ensembl Analysis Adaptor
** @nam4rule Databasefile Set the database file
** @nam4rule Databasename Set the database name
** @nam4rule Databaseversion Set the database version
** @nam4rule Date Set a date
** @nam5rule Creation Set the date of creation
** @nam4rule Description Set the description
** @nam4rule Displayable Set the displayable flag
** @nam4rule Displaylabel Set the display label
** @nam4rule Gfffeature Set the GFF feature
** @nam4rule Gffsource Set the GFF source
** @nam4rule Identifier Set the SQL database-internal identifier
** @nam4rule Modulename Set the module name
** @nam4rule Moduleversion Set the module version
** @nam4rule Name Set the name
** @nam4rule Parameters Set the parameters
** @nam4rule Programfile Set the program file
** @nam4rule Programname Set the program name
** @nam4rule Programversion Set the program version
** @nam4rule Webdata Set the web data
**
** @argrule * analysis [EnsPAnalysis] Ensembl Analysis object
** @argrule Adaptor aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @argrule Databasefile databasefile [AjPStr] Database file
** @argrule Databasename databasename [AjPStr] Database name
** @argrule Databaseversion databaseversion [AjPStr] Database version
** @argrule DateCreation cdate [AjPStr] Creation date
** @argrule Description description [AjPStr] Description
** @argrule Displayable displayable [AjBool] Displayable flag
** @argrule Displaylabel displaylabel [AjPStr] Display label
** @argrule Gfffeature gfffeature [AjPStr] GFF feature
** @argrule Gffsource gffsource [AjPStr] GFF source
** @argrule Identifier identifier [ajuint] SQL database-internal identifier
** @argrule Modulename modulename [AjPStr] Module name
** @argrule Moduleversion moduleversion [AjPStr] Module version
** @argrule Name name [AjPStr] Name
** @argrule Parameters parameters [AjPStr] Parameters
** @argrule Programfile programfile [AjPStr] Program file
** @argrule Programname programname [AjPStr] Program name
** @argrule Programversion programversion [AjPStr] Program version
** @argrule Webdata webdata [AjPStr] Web data
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory modify
******************************************************************************/




/* @func ensAnalysisSetAdaptor ************************************************
**
** Set the Ensembl Analysis Adaptor member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::adaptor
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisSetAdaptor(EnsPAnalysis analysis,
                             EnsPAnalysisadaptor aa)
{
    if (!analysis)
        return ajFalse;

    analysis->Adaptor = aa;

    return ajTrue;
}




/* @func ensAnalysisSetDatabasefile *******************************************
**
** Set the database file member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_file
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] databasefile [AjPStr] Database file
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDatabasefile(EnsPAnalysis analysis,
                                  AjPStr databasefile)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Databasefile);

    analysis->Databasefile = ajStrNewRef(databasefile);

    return ajTrue;
}




/* @func ensAnalysisSetDatabasename *******************************************
**
** Set the database name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] databasename [AjPStr] Database name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDatabasename(EnsPAnalysis analysis,
                                  AjPStr databasename)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Databasename);

    analysis->Databasename = ajStrNewRef(databasename);

    return ajTrue;
}




/* @func ensAnalysisSetDatabaseversion ****************************************
**
** Set the database version member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::db_version
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] databaseversion [AjPStr] Database version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDatabaseversion(EnsPAnalysis analysis,
                                     AjPStr databaseversion)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Databaseversion);

    analysis->Databaseversion = ajStrNewRef(databaseversion);

    return ajTrue;
}




/* @func ensAnalysisSetDateCreation *******************************************
**
** Set the date of creation member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::created
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] cdate [AjPStr] Creation date
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDateCreation(EnsPAnalysis analysis,
                                  AjPStr cdate)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->DateCreation);

    analysis->DateCreation = ajStrNewRef(cdate);

    return ajTrue;
}




/* @func ensAnalysisSetDescription ********************************************
**
** Set the description member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::description
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] description [AjPStr] Description
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDescription(EnsPAnalysis analysis,
                                 AjPStr description)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Description);

    analysis->Description = ajStrNewRef(description);

    return ajTrue;
}




/* @func ensAnalysisSetDisplayable ********************************************
**
** Set the displayable member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::displayable
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [r] displayable [AjBool] Displayable flag
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDisplayable(EnsPAnalysis analysis,
                                 AjBool displayable)
{
    if (!analysis)
        return ajFalse;

    analysis->Displayable = displayable;

    return ajTrue;
}




/* @func ensAnalysisSetDisplaylabel *******************************************
**
** Set the display label member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::display_label
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] displaylabel [AjPStr] Display label
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetDisplaylabel(EnsPAnalysis analysis,
                                  AjPStr displaylabel)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Displaylabel);

    analysis->Displaylabel = ajStrNewRef(displaylabel);

    return ajTrue;
}




/* @func ensAnalysisSetGfffeature *********************************************
**
** Set the GFF feature member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_feature
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] gfffeature [AjPStr] GFF feature
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetGfffeature(EnsPAnalysis analysis,
                                AjPStr gfffeature)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Gfffeature);

    analysis->Gfffeature = ajStrNewRef(gfffeature);

    return ajTrue;
}




/* @func ensAnalysisSetGffsource **********************************************
**
** Set the GFF source member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::gff_source
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] gffsource [AjPStr] GFF source
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetGffsource(EnsPAnalysis analysis,
                               AjPStr gffsource)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Gffsource);

    analysis->Gffsource = ajStrNewRef(gffsource);

    return ajTrue;
}




/* @func ensAnalysisSetIdentifier *********************************************
**
** Set the SQL database-internal identifier member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Storable::dbID
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [r] identifier [ajuint] SQL database-internal identifier
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisSetIdentifier(EnsPAnalysis analysis,
                                ajuint identifier)
{
    if (!analysis)
        return ajFalse;

    analysis->Identifier = identifier;

    return ajTrue;
}




/* @func ensAnalysisSetModulename *********************************************
**
** Set the module name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] modulename [AjPStr] Module name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetModulename(EnsPAnalysis analysis,
                                AjPStr modulename)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Modulename);

    analysis->Modulename = ajStrNewRef(modulename);

    return ajTrue;
}




/* @func ensAnalysisSetModuleversion ******************************************
**
** Set the module version member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::module_version
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] moduleversion [AjPStr] Module version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetModuleversion(EnsPAnalysis analysis,
                                   AjPStr moduleversion)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Moduleversion);

    analysis->Moduleversion = ajStrNewRef(moduleversion);

    return ajTrue;
}




/* @func ensAnalysisSetName ***************************************************
**
** Set the name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::logic_name
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] name [AjPStr] Name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisSetName(EnsPAnalysis analysis,
                          AjPStr name)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Name);

    analysis->Name = ajStrNewRef(name);

    return ajTrue;
}




/* @func ensAnalysisSetParameters *********************************************
**
** Set the parameters member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::parameters
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] parameters [AjPStr] Parameters
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisSetParameters(EnsPAnalysis analysis,
                                AjPStr parameters)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Parameters);

    analysis->Parameters = ajStrNewRef(parameters);

    return ajTrue;
}




/* @func ensAnalysisSetProgramfile ********************************************
**
** Set the program file member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_file
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] programfile [AjPStr] Program file
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetProgramfile(EnsPAnalysis analysis,
                                 AjPStr programfile)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Programfile);

    analysis->Programfile = ajStrNewRef(programfile);

    return ajTrue;
}




/* @func ensAnalysisSetProgramname ********************************************
**
** Set the program name member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] programname [AjPStr] Program name
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetProgramname(EnsPAnalysis analysis,
                                 AjPStr programname)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Programname);

    analysis->Programname = ajStrNewRef(programname);

    return ajTrue;
}




/* @func ensAnalysisSetProgramversion *****************************************
**
** Set the program version member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::program_version
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] programversion [AjPStr] Program version
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetProgramversion(EnsPAnalysis analysis,
                                    AjPStr programversion)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Programversion);

    analysis->Programversion = ajStrNewRef(programversion);

    return ajTrue;
}




/* @func ensAnalysisSetWebdata ************************************************
**
** Set the web data member of an Ensembl Analysis.
**
** @cc Bio::EnsEMBL::Analysis::web_data
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
** @param [u] webdata [AjPStr] Web data
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

AjBool ensAnalysisSetWebdata(EnsPAnalysis analysis,
                             AjPStr webdata)
{
    if (!analysis)
        return ajFalse;

    ajStrDel(&analysis->Webdata);

    analysis->Webdata = ajStrNewRef(webdata);

    return ajTrue;
}




/* @section debugging *********************************************************
**
** Functions for reporting of an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Trace Report Ensembl Analysis members to debug file.
**
** @argrule Trace analysis [const EnsPAnalysis] Ensembl Analysis
** @argrule Trace level [ajuint] Indentation level
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensAnalysisTrace *****************************************************
**
** Trace an Ensembl Analysis.
**
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
** @param [r] level [ajuint] Indentation level
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisTrace(const EnsPAnalysis analysis, ajuint level)
{
    AjPStr indent = NULL;

    if (!analysis)
        return ajFalse;

    indent = ajStrNew();

    ajStrAppendCountK(&indent, ' ', level * 2);

    ajDebug("%SensAnalysisTrace %p\n"
            "%S  Use %u\n"
            "%S  Identifier %u\n"
            "%S  Adaptor %p\n"
            "%S  DateCreation '%S'\n"
            "%S  Name '%S'\n"
            "%S  Databasename '%S'\n"
            "%S  Databaseversion '%S'\n"
            "%S  Databasefile '%S'\n"
            "%S  Programname '%S'\n"
            "%S  Programversion '%S'\n"
            "%S  Programfile '%S'\n"
            "%S  Parameters '%S'\n"
            "%S  Modulename '%S'\n"
            "%S  Moduleversion '%S'\n"
            "%S  Gffsource '%S'\n"
            "%S  Gfffeature '%S'\n"
            "%S  Description %p\n"
            "%S  Displaylabel '%S'\n"
            "%S  Displayable '%B'\n"
            "%S  Webdata %p\n",
            indent, analysis,
            indent, analysis->Use,
            indent, analysis->Identifier,
            indent, analysis->Adaptor,
            indent, analysis->DateCreation,
            indent, analysis->Name,
            indent, analysis->Databasename,
            indent, analysis->Databaseversion,
            indent, analysis->Databasefile,
            indent, analysis->Programname,
            indent, analysis->Programversion,
            indent, analysis->Programfile,
            indent, analysis->Parameters,
            indent, analysis->Modulename,
            indent, analysis->Moduleversion,
            indent, analysis->Gffsource,
            indent, analysis->Gfffeature,
            indent, analysis->Description,
            indent, analysis->Displaylabel,
            indent, analysis->Displayable,
            indent, analysis->Webdata);

    ajStrDel(&indent);

    return ajTrue;
}




/* @section calculate *********************************************************
**
** Functions for calculating information from an Ensembl Analysis object.
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Calculate Calculate Ensembl Analysis information
** @nam4rule Memsize Calculate the memory size in bytes
**
** @argrule * analysis [const EnsPAnalysis] Ensembl Analysis
**
** @valrule Memsize [size_t] Memory size in bytes or 0
**
** @fcategory misc
******************************************************************************/




/* @func ensAnalysisCalculateMemsize ******************************************
**
** Get the memory size in bytes of an Ensembl Analysis.
**
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [size_t] Memory size in bytes or 0
**
** @release 6.4.0
** @@
******************************************************************************/

size_t ensAnalysisCalculateMemsize(const EnsPAnalysis analysis)
{
    size_t size = 0;

    if (!analysis)
        return 0;

    size += sizeof (EnsOAnalysis);

    if (analysis->DateCreation)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->DateCreation);
    }

    if (analysis->Name)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Name);
    }

    if (analysis->Databasename)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Databasename);
    }

    if (analysis->Databaseversion)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Databaseversion);
    }

    if (analysis->Databasefile)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Databasefile);
    }

    if (analysis->Programname)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Programname);
    }

    if (analysis->Programversion)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Programversion);
    }

    if (analysis->Programfile)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Programfile);
    }

    if (analysis->Parameters)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Parameters);
    }

    if (analysis->Modulename)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Modulename);
    }

    if (analysis->Moduleversion)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Moduleversion);
    }

    if (analysis->Gffsource)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Gffsource);
    }

    if (analysis->Gfffeature)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Gfffeature);
    }

    if (analysis->Description)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Description);
    }

    if (analysis->Displaylabel)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Displaylabel);
    }

    if (analysis->Webdata)
    {
        size += sizeof (AjOStr);

        size += ajStrGetRes(analysis->Webdata);
    }

    return size;
}




/* @section testing properties ************************************************
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Is Test an Ensembl Analysis property
** @nam4rule Database Ensembl Analysis is based on a database
**
** @argrule Is analysis [const EnsPAnalysis] Ensembl Analysis
**
** @valrule * [AjBool] Ensembl Analysis boolean property
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisIsDatabase ************************************************
**
** Test whether an Ensembl Analysis is based on a database.
**
** @cc Bio::EnsEMBL::Analysis::has_database
** @param [r] analysis [const EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool]
** ajTrue if the Ensembl Analysis was defined and has a database attached
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisIsDatabase(const EnsPAnalysis analysis)
{
    if (!analysis)
        return ajFalse;

    if (analysis->Databasename && ajStrGetLen(analysis->Databasename))
        return ajTrue;
    else
        return ajFalse;
}




/* @section comparison ********************************************************
**
** Functions for comparing Ensembl Analysis objects
**
** @fdata [EnsPAnalysis]
**
** @nam3rule Match Compare two Ensembl Analysis objects
**
** @argrule * analysis1 [const EnsPAnalysis] Ensembl Analysis
** @argrule * analysis2 [const EnsPAnalysis] Ensembl Analysis
**
** @valrule * [AjBool] ajTrue if the Ensembl Analysis objects are equal
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisMatch *****************************************************
**
** Test for matching two Ensembl Analysis objects.
**
** @cc Bio::EnsEMBL::Analysis::compare
** @param [r] analysis1 [const EnsPAnalysis] First Ensembl Analysis
** @param [r] analysis2 [const EnsPAnalysis] Second Ensembl Analysis
**
** @return [AjBool] ajTrue if the Ensembl Analysis objects are equal
**
** @release 6.2.0
** @@
** The comparison is based on an initial pointer equality test and if that
** fails, a case-insensitive string comparison of the name and version members
** is performed.
******************************************************************************/

AjBool ensAnalysisMatch(const EnsPAnalysis analysis1,
                        const EnsPAnalysis analysis2)
{
    if (ajDebugTest("ensAnalysisMatch"))
        ajDebug("ensAnalysisMatch\n"
                "  analysis1 %p\n"
                "  analysis2 %p\n",
                analysis1,
                analysis2);

    if (!analysis1)
        return ajFalse;

    if (!analysis2)
        return ajFalse;

    if (analysis1 == analysis2)
        return ajTrue;

    if (!ajStrMatchCaseS(analysis1->Name,
                         analysis2->Name))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Databasename,
                         analysis2->Databasename))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Databaseversion,
                         analysis2->Databaseversion))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Databasefile,
                         analysis2->Databasefile))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Programname,
                         analysis2->Programname))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Programversion,
                         analysis2->Programversion))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Programfile,
                         analysis2->Programfile))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Parameters,
                         analysis2->Parameters))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Modulename,
                         analysis2->Modulename))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Moduleversion,
                         analysis2->Moduleversion))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Gffsource,
                         analysis2->Gffsource))
        return ajFalse;

    if (!ajStrMatchCaseS(analysis1->Gfffeature,
                         analysis2->Gfffeature))
        return ajFalse;

    return ajTrue;
}




/* @datasection [AjPList] AJAX List *******************************************
**
** @nam2rule List Functions for manipulating AJAX List objects
**
******************************************************************************/




/* @funcstatic listAnalysisCompareIdentifierAscending *************************
**
** AJAX List of Ensembl Analysis objects comparison function to sort by
** Ensembl Analysis identifier in ascending order.
**
** @param [r] item1 [const void*] Ensembl Analysis address 1
** @param [r] item2 [const void*] Ensembl Analysis address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listAnalysisCompareIdentifierAscending(
    const void *item1,
    const void *item2)
{
    EnsPAnalysis analysis1 = *(EnsOAnalysis *const *) item1;
    EnsPAnalysis analysis2 = *(EnsOAnalysis *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listAnalysisCompareIdentifierAscending"))
        ajDebug("listAnalysisCompareIdentifierAscending\n"
                "  analysis1 %p\n"
                "  analysis2 %p\n",
                analysis1,
                analysis2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (analysis1 && (!analysis2))
        return -1;

    if ((!analysis1) && (!analysis2))
        return 0;

    if ((!analysis1) && analysis2)
        return +1;

    if (analysis1->Identifier < analysis2->Identifier)
        return -1;

    if (analysis1->Identifier > analysis2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listAnalysisCompareIdentifierDescending ************************
**
** AJAX List of Ensembl Analysis objects comparison function to sort by
** Ensembl Analysis identifier in descending order.
**
** @param [r] item1 [const void*] Ensembl Analysis address 1
** @param [r] item2 [const void*] Ensembl Analysis address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listAnalysisCompareIdentifierDescending(
    const void *item1,
    const void *item2)
{
    EnsPAnalysis analysis1 = *(EnsOAnalysis *const *) item1;
    EnsPAnalysis analysis2 = *(EnsOAnalysis *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listAnalysisCompareIdentifierDescending"))
        ajDebug("listAnalysisCompareIdentifierDescending\n"
                "  analysis1 %p\n"
                "  analysis2 %p\n",
                analysis1,
                analysis2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (analysis1 && (!analysis2))
        return -1;

    if ((!analysis1) && (!analysis2))
        return 0;

    if ((!analysis1) && analysis2)
        return +1;

    if (analysis1->Identifier > analysis2->Identifier)
        return -1;

    if (analysis1->Identifier < analysis2->Identifier)
        return +1;

    return 0;
}




/* @funcstatic listAnalysisCompareNameAscending *******************************
**
** AJAX List of Ensembl Analysis objects comparison function to sort by
** Ensembl Analysis name in ascending order.
**
** @param [r] item1 [const void*] Ensembl Analysis address 1
** @param [r] item2 [const void*] Ensembl Analysis address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listAnalysisCompareNameAscending(
    const void *item1,
    const void *item2)
{
    EnsPAnalysis analysis1 = *(EnsOAnalysis *const *) item1;
    EnsPAnalysis analysis2 = *(EnsOAnalysis *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listAnalysisCompareNameAscending"))
        ajDebug("listAnalysisCompareNameAscending\n"
                "  analysis1 %p\n"
                "  analysis2 %p\n",
                analysis1,
                analysis2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (analysis1 && (!analysis2))
        return -1;

    if ((!analysis1) && (!analysis2))
        return 0;

    if ((!analysis1) && analysis2)
        return +1;

    return ajStrCmpS(analysis1->Name, analysis2->Name);
}




/* @funcstatic listAnalysisCompareNameDescending ******************************
**
** AJAX List of Ensembl Analysis objects comparison function to sort by
** Ensembl Analysis name in descending order.
**
** @param [r] item1 [const void*] Ensembl Analysis address 1
** @param [r] item2 [const void*] Ensembl Analysis address 2
** @see ajListSort
**
** @return [int] The comparison function returns an integer less than,
**               equal to, or greater than zero if the first argument is
**               considered to be respectively less than, equal to, or
**               greater than the second.
**
** @release 6.4.0
** @@
******************************************************************************/

static int listAnalysisCompareNameDescending(
    const void *item1,
    const void *item2)
{
    EnsPAnalysis analysis1 = *(EnsOAnalysis *const *) item1;
    EnsPAnalysis analysis2 = *(EnsOAnalysis *const *) item2;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 2
    if (ajDebugTest("listAnalysisCompareNameDescending"))
        ajDebug("listAnalysisCompareNameDescending\n"
                "  analysis1 %p\n"
                "  analysis2 %p\n",
                analysis1,
                analysis2);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 2 */

    /* Sort empty values towards the end of the AJAX List. */

    if (analysis1 && (!analysis2))
        return -1;

    if ((!analysis1) && (!analysis2))
        return 0;

    if ((!analysis1) && analysis2)
        return +1;

    return -1 * ajStrCmpS(analysis1->Name, analysis2->Name);
}




/* @section list **************************************************************
**
** Functions for manipulating AJAX List objects.
**
** @fdata [AjPList]
**
** @nam3rule Analysis Functions for manipulating AJAX List objects of
** Ensembl Analysis objects
** @nam4rule Sort Sort functions
** @nam5rule Identifier Sort by Ensembl Analysis identifier member
** @nam5rule Name Sort by Ensembl Analysis name member
** @nam6rule Ascending  Sort in ascending order
** @nam6rule Descending Sort in descending order
**
** @argrule Sort analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory misc
******************************************************************************/




/* @func ensListAnalysisSortIdentifierAscending *******************************
**
** Sort an AJAX List of Ensembl Analysis objects by their
** Ensembl Analysis identifier in ascending order.
**
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListAnalysisSortIdentifierAscending(AjPList analyses)
{
    if (!analyses)
        return ajFalse;

    ajListSort(analyses, &listAnalysisCompareIdentifierAscending);

    return ajTrue;
}




/* @func ensListAnalysisSortIdentifierDescending ******************************
**
** Sort an AJAX List of Ensembl Analysis objects by their
** Ensembl Analysis identifier in descending order.
**
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListAnalysisSortIdentifierDescending(AjPList analyses)
{
    if (!analyses)
        return ajFalse;

    ajListSort(analyses, &listAnalysisCompareIdentifierDescending);

    return ajTrue;
}




/* @func ensListAnalysisSortNameAscending *************************************
**
** Sort an AJAX List of Ensembl Analysis objects by their
** Ensembl Analysis name in ascending order.
**
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListAnalysisSortNameAscending(AjPList analyses)
{
    if (!analyses)
        return ajFalse;

    ajListSort(analyses, &listAnalysisCompareNameAscending);

    return ajTrue;
}




/* @func ensListAnalysisSortNameDescending ************************************
**
** Sort an AJAX List of Ensembl Analysis objects by their
** Ensembl Analysis name in descending order.
**
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensListAnalysisSortNameDescending(AjPList analyses)
{
    if (!analyses)
        return ajFalse;

    ajListSort(analyses, &listAnalysisCompareNameDescending);

    return ajTrue;
}




/* @datasection [EnsPAnalysisadaptor] Ensembl Analysis Adaptor ****************
**
** @nam2rule Analysisadaptor Functions for manipulating
** Ensembl Analysis Adaptor objects
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor
** @cc CVS Revision: 1.76
** @cc CVS Tag: branch-ensembl-68
**
******************************************************************************/




/* @funcstatic analysisadaptorFetchAllbyStatement *****************************
**
** Run a SQL statement against an Ensembl Database Adaptor and consolidate the
** results into an AJAX List of Ensembl Analysis objects.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::_objFromHashref
** @param [u] ba [EnsPBaseadaptor] Ensembl Base Adaptor
** @param [r] statement [const AjPStr] SQL statement
** @param [uN] am [EnsPAssemblymapper] Ensembl Assembly Mapper
** @param [uN] slice [EnsPSlice] Ensembl Slice
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

static AjBool analysisadaptorFetchAllbyStatement(
    EnsPBaseadaptor ba,
    const AjPStr statement,
    EnsPAssemblymapper am,
    EnsPSlice slice,
    AjPList analyses)
{
    ajuint identifier = 0U;

    AjBool displayable = AJFALSE;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr cdate           = NULL;
    AjPStr name            = NULL;
    AjPStr databasename    = NULL;
    AjPStr databaseversion = NULL;
    AjPStr databasefile    = NULL;
    AjPStr programname     = NULL;
    AjPStr programversion  = NULL;
    AjPStr programfile     = NULL;
    AjPStr parameters      = NULL;
    AjPStr modulename      = NULL;
    AjPStr moduleversion   = NULL;
    AjPStr gffsource       = NULL;
    AjPStr gfffeature      = NULL;
    AjPStr description     = NULL;
    AjPStr displaylabel    = NULL;
    AjPStr webdata         = NULL;

    EnsPAnalysis analysis  = NULL;
    EnsPAnalysisadaptor aa = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (ajDebugTest("analysisadaptorFetchAllbyStatement"))
        ajDebug("analysisadaptorFetchAllbyStatement\n"
                "  ba %p\n"
                "  statement %p\n"
                "  am %p\n"
                "  slice %p\n"
                "  analyses %p\n",
                ba,
                statement,
                am,
                slice,
                analyses);

    if (!ba)
        return ajFalse;

    if (!statement)
        return ajFalse;

    if (!analyses)
        return ajFalse;

    dba = ensBaseadaptorGetDatabaseadaptor(ba);

    aa = ensRegistryGetAnalysisadaptor(dba);

    sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

    sqli = ajSqlrowiterNew(sqls);

    while (!ajSqlrowiterDone(sqli))
    {
        identifier      = 0;
        cdate           = ajStrNew();
        name            = ajStrNew();
        databasename    = ajStrNew();
        databaseversion = ajStrNew();
        databasefile    = ajStrNew();
        programname     = ajStrNew();
        programversion  = ajStrNew();
        programfile     = ajStrNew();
        parameters      = ajStrNew();
        modulename      = ajStrNew();
        moduleversion   = ajStrNew();
        gffsource       = ajStrNew();
        gfffeature      = ajStrNew();
        description     = ajStrNew();
        displaylabel    = ajStrNew();
        displayable     = ajFalse;
        webdata         = ajStrNew();

        sqlr = ajSqlrowiterGet(sqli);

        ajSqlcolumnToUint(sqlr, &identifier);
        ajSqlcolumnToStr(sqlr, &cdate);
        ajSqlcolumnToStr(sqlr, &name);
        ajSqlcolumnToStr(sqlr, &databasename);
        ajSqlcolumnToStr(sqlr, &databaseversion);
        ajSqlcolumnToStr(sqlr, &databasefile);
        ajSqlcolumnToStr(sqlr, &programname);
        ajSqlcolumnToStr(sqlr, &programversion);
        ajSqlcolumnToStr(sqlr, &programfile);
        ajSqlcolumnToStr(sqlr, &parameters);
        ajSqlcolumnToStr(sqlr, &modulename);
        ajSqlcolumnToStr(sqlr, &moduleversion);
        ajSqlcolumnToStr(sqlr, &gffsource);
        ajSqlcolumnToStr(sqlr, &gfffeature);
        ajSqlcolumnToStr(sqlr, &description);
        ajSqlcolumnToStr(sqlr, &displaylabel);
        ajSqlcolumnToBool(sqlr, &displayable);
        ajSqlcolumnToStr(sqlr, &webdata);

        analysis = ensAnalysisNewIni(aa,
                                     identifier,
                                     cdate,
                                     name,
                                     databasename,
                                     databaseversion,
                                     databasefile,
                                     programname,
                                     programversion,
                                     programfile,
                                     parameters,
                                     modulename,
                                     moduleversion,
                                     gffsource,
                                     gfffeature,
                                     description,
                                     displaylabel,
                                     webdata,
                                     displayable);

        ajListPushAppend(analyses, (void *) analysis);

        ajStrDel(&cdate);
        ajStrDel(&name);
        ajStrDel(&databasename);
        ajStrDel(&databaseversion);
        ajStrDel(&databasefile);
        ajStrDel(&programname);
        ajStrDel(&programversion);
        ajStrDel(&programfile);
        ajStrDel(&parameters);
        ajStrDel(&modulename);
        ajStrDel(&moduleversion);
        ajStrDel(&gffsource);
        ajStrDel(&gfffeature);
        ajStrDel(&description);
        ajStrDel(&displaylabel);
        ajStrDel(&webdata);
    }

    ajSqlrowiterDel(&sqli);

    ensDatabaseadaptorSqlstatementDel(dba, &sqls);

    return ajTrue;
}




/* @section constructors ******************************************************
**
** All constructors return a new Ensembl Analysis Adaptor by pointer.
** It is the responsibility of the user to first destroy any previous
** Ensembl Analysis Adaptor.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** @fdata [EnsPAnalysisadaptor]
**
** @nam3rule New Constructor
**
** @argrule New dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @valrule * [EnsPAnalysisadaptor] Ensembl Analysis Adaptor or NULL
**
** @fcategory new
******************************************************************************/




/* @func ensAnalysisadaptorNew ************************************************
**
** Default constructor for an Ensembl Analysis Adaptor.
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
** @see ensRegistryGetAnalysisadaptor
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::new
** @param [u] dba [EnsPDatabaseadaptor] Ensembl Database Adaptor
**
** @return [EnsPAnalysisadaptor] Ensembl Analysis Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPAnalysisadaptor ensAnalysisadaptorNew(
    EnsPDatabaseadaptor dba)
{
    EnsPAnalysisadaptor aa = NULL;

    if (!dba)
        return NULL;

    if (ajDebugTest("ensAnalysisadaptorNew"))
        ajDebug("ensAnalysisadaptorNew\n"
                "  dba %p\n",
                dba);

    AJNEW0(aa);

    aa->Adaptor = ensBaseadaptorNew(
        dba,
        analysisadaptorKTablenames,
        analysisadaptorKColumnnames,
        analysisadaptorKLeftjoins,
        (const char *) NULL,
        (const char *) NULL,
        &analysisadaptorFetchAllbyStatement);

    /*
    ** NOTE: The cache cannot be initialised here because the
    ** analysisadaptorCacheInit function calls
    ** ensBaseadaptorFetchAllbyConstraint,
    ** which calls analysisadaptorFetchAllbyStatement, which calls
    ** ensRegistryGetAnalysisadaptor. At that point, however,
    ** the Ensembl Analysis Adaptor has not been stored in the Registry.
    ** Therefore, each ensAnalysisadaptorFetch function has to test the
    ** presence of the adaptor-internal cache and eventually initialise
    ** before accessing it.
    **
    ** analysisadaptorCacheInit(aa);
    */

    return aa;
}




/* @section cache *************************************************************
**
** Functions for maintaining the Ensembl Analysis Adaptor-internal cache of
** Ensembl Analysis objects.
**
** @fdata [EnsPAnalysisadaptor]
**
** @nam3rule Cache Process an Ensembl Analysis Adaptor-internal cache
** @nam4rule Clear Clear the Ensembl Analysis Adaptor-internal cache
**
** @argrule * aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @valrule * [AjBool] True on success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic analysisadaptorCacheInit ***************************************
**
** Initialise the internal Ensembl Analysis cache of an
** Ensembl Analysis Adaptor.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool analysisadaptorCacheInit(
    EnsPAnalysisadaptor aa)
{
    AjBool result = AJFALSE;

    AjPList analyses = NULL;

    EnsPAnalysis analysis = NULL;

    if (ajDebugTest("analysisadaptorCacheInit"))
        ajDebug("analysisadaptorCacheInit\n"
                "  aa %p\n",
                aa);

    if (!aa)
        return ajFalse;

    if (aa->CacheByIdentifier)
        return ajFalse;
    else
    {
        aa->CacheByIdentifier = ajTableuintNew(0);

        ajTableSetDestroyvalue(
            aa->CacheByIdentifier,
            (void (*)(void **)) &ensAnalysisDel);
    }

    if (aa->CacheByName)
        return ajFalse;
    else
    {
        aa->CacheByName = ajTablestrNew(0U);

        ajTableSetDestroyvalue(
            aa->CacheByName,
            (void (*)(void **)) &ensAnalysisDel);
    }

    analyses = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ensAnalysisadaptorGetBaseadaptor(aa),
        (const AjPStr) NULL,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        analyses);

    while (ajListPop(analyses, (void **) &analysis))
    {
        analysisadaptorCacheInsert(aa, &analysis);

        /*
        ** Both caches hold internal references to the
        ** Ensembl Analysis objects.
        */

        ensAnalysisDel(&analysis);
    }

    ajListFree(&analyses);

    return result;
}




/* @funcstatic analysisadaptorCacheInsert *************************************
**
** Insert an Ensembl Analysis into the Ensembl Analysis Adaptor-internal cache.
** If an Ensembl Analysis with the same name member is already present in the
** Ensembl Analysis Adaptor-internal cache, the Ensembl Analysis is deleted and
** a pointer to the cached Ensembl Analysis is returned.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [u] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool analysisadaptorCacheInsert(
    EnsPAnalysisadaptor aa,
    EnsPAnalysis *Panalysis)
{
    ajuint *Pidentifier = NULL;

    EnsPAnalysis analysis1 = NULL;
    EnsPAnalysis analysis2 = NULL;

    if (!aa)
        return ajFalse;

    if (!aa->CacheByIdentifier)
        return ajFalse;

    if (!aa->CacheByName)
        return ajFalse;

    if (!Panalysis)
        return ajFalse;

    if (!*Panalysis)
        return ajFalse;

    /* Search the identifer cache. */

    analysis1 = (EnsPAnalysis) ajTableFetchmodV(
        aa->CacheByIdentifier,
        (const void *) &((*Panalysis)->Identifier));

    /* Search the name cache. */

    analysis2 = (EnsPAnalysis) ajTableFetchmodS(
        aa->CacheByName,
        (*Panalysis)->Name);

    if ((!analysis1) && (!analysis2))
    {
        /* Insert into the identifier cache. */

        AJNEW0(Pidentifier);

        *Pidentifier = (*Panalysis)->Identifier;

        ajTablePut(aa->CacheByIdentifier,
                   (void *) Pidentifier,
                   (void *) ensAnalysisNewRef(*Panalysis));

        /* Insert into the name cache. */

        ajTablePut(aa->CacheByName,
                   (void *) ajStrNewS((*Panalysis)->Name),
                   (void *) ensAnalysisNewRef(*Panalysis));
    }

    if (analysis1 && analysis2 && (analysis1 == analysis2))
    {
        ajDebug("analysisadaptorCacheInsert replaced Ensembl Analysis %p "
                "with one already cached %p.\n",
                *Panalysis, analysis1);

        ensAnalysisDel(Panalysis);

        ensAnalysisNewRef(analysis1);

        Panalysis = &analysis1;
    }

    if (analysis1 && analysis2 && (analysis1 != analysis2))
        ajDebug("analysisadaptorCacheInsert detected Ensembl Analysis objects "
                "in the identifier and name cache with identical names "
                "('%S' and '%S') but different addresses (%p and %p).\n",
                analysis1->Name, analysis2->Name, analysis1, analysis2);

    if (analysis1 && (!analysis2))
        ajDebug("analysisadaptorCacheInsert detected an Ensembl Analysis "
                "in the identifier, but not in the name cache.\n");

    if ((!analysis1) && analysis2)
        ajDebug("analysisadaptorCacheInsert detected an Ensembl Analysis "
                "in the name, but not in the identifier cache.\n");

    return ajTrue;
}




#if AJFALSE
/* @funcstatic analysisadaptorCacheRemove *************************************
**
** Remove an Ensembl Analysis from the Ensembl Analysis Adaptor-internal cache.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [u] analysis [EnsPAnalysis] Ensembl Analysis
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.3.0
** @@
******************************************************************************/

static AjBool analysisadaptorCacheRemove(
    EnsPAnalysisadaptor aa,
    EnsPAnalysis analysis)
{
    EnsPAnalysis analysis1 = NULL;
    EnsPAnalysis analysis2 = NULL;

    if (!aa)
        return ajFalse;

    if (!analysis)
        return ajFalse;

    analysis1 = (EnsPAnalysis) ajTableRemove(
        aa->CacheByIdentifier,
        (const void *) &analysis->Identifier);

    analysis2 = (EnsPAnalysis) ajTableRemove(
        aa->CacheByName,
        (const void *) analysis->Name);

    if (analysis1 && (!analysis2))
        ajWarn("analysisadaptorCacheRemove could remove Ensembl Analysis with "
               "identifier %u and name '%S' only from the identifier cache.\n",
               analysis->Identifier,
               analysis->Name);

    if ((!analysis1) && analysis2)
        ajWarn("analysisadaptorCacheRemove could remove Ensembl Analysis with "
               "identifier %u and name '%S' only from the name cache.\n",
               analysis->Identifier,
               analysis->Name);

    ensAnalysisDel(&analysis1);
    ensAnalysisDel(&analysis2);

    return ajTrue;
}

#endif /* AJFALSE */




/* @func ensAnalysisadaptorCacheClear *****************************************
**
** Clear the Ensembl Analysis Adaptor-internal cache of
** Ensembl Analysis objects.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAnalysisadaptorCacheClear(
    EnsPAnalysisadaptor aa)
{
    if (!aa)
        return ajFalse;

    ajTableDel(&aa->CacheByIdentifier);
    ajTableDel(&aa->CacheByName);

    return ajTrue;
}




/* @section destructors *******************************************************
**
** Destruction destroys all internal data structures and frees the memory
** allocated for an Ensembl Analysis Adaptor object.
**
** @fdata [EnsPAnalysisadaptor]
**
** @nam3rule Del Destroy (free) an Ensembl Analysis Adaptor
**
** @argrule * Paa [EnsPAnalysisadaptor*] Ensembl Analysis Adaptor address
**
** @valrule * [void]
**
** @fcategory delete
******************************************************************************/




/* @func ensAnalysisadaptorDel ************************************************
**
** Default destructor for an Ensembl Analysis Adaptor.
**
** This function also clears the internal caches.
**
** Ensembl Object Adaptors are singleton objects that are registered in the
** Ensembl Registry and weakly referenced by Ensembl Objects that have been
** instantiated by it. Therefore, Ensembl Object Adaptors should never be
** destroyed directly. Upon exit, the Ensembl Registry will call this function
** if required.
**
** @param [d] Paa [EnsPAnalysisadaptor*] Ensembl Analysis Adaptor address
**
** @return [void]
**
** @release 6.2.0
** @@
******************************************************************************/

void ensAnalysisadaptorDel(
    EnsPAnalysisadaptor *Paa)
{
    EnsPAnalysisadaptor pthis = NULL;

    if (!Paa)
        return;

#if defined(AJ_DEBUG) && AJ_DEBUG >= 1
    if (ajDebugTest("ensAnalysisadaptorDel"))
        ajDebug("ensAnalysisadaptorDel\n"
                "  *Paa %p\n",
                *Paa);
#endif /* defined(AJ_DEBUG) && AJ_DEBUG >= 1 */

    if (!(pthis = *Paa))
        return;

    ensAnalysisadaptorCacheClear(pthis);

    ensBaseadaptorDel(&pthis->Adaptor);

    ajMemFree((void **) Paa);

    return;
}




/* @section member retrieval **************************************************
**
** Functions for returning members of an Ensembl Analysis Adaptor object.
**
** @fdata [EnsPAnalysisadaptor]
**
** @nam3rule Get Return Ensembl Analysis Adaptor attribute(s)
** @nam4rule Baseadaptor Return the Ensembl Base Adaptor
** @nam4rule Databaseadaptor Return the Ensembl Database Adaptor
**
** @argrule * aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @valrule Baseadaptor [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
** @valrule Databaseadaptor [EnsPDatabaseadaptor] Ensembl Database Adaptor
** or NULL
**
** @fcategory use
******************************************************************************/




/* @func ensAnalysisadaptorGetBaseadaptor *************************************
**
** Get the Ensembl Base Adaptor member of an Ensembl Analysis Adaptor.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [EnsPBaseadaptor] Ensembl Base Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPBaseadaptor ensAnalysisadaptorGetBaseadaptor(
    EnsPAnalysisadaptor aa)
{
    return (aa) ? aa->Adaptor : NULL;
}




/* @func ensAnalysisadaptorGetDatabaseadaptor *********************************
**
** Get the Ensembl Database Adaptor member of an Ensembl Analysis Adaptor.
**
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
**
** @return [EnsPDatabaseadaptor] Ensembl Database Adaptor or NULL
**
** @release 6.2.0
** @@
******************************************************************************/

EnsPDatabaseadaptor ensAnalysisadaptorGetDatabaseadaptor(
    EnsPAnalysisadaptor aa)
{
    return ensBaseadaptorGetDatabaseadaptor(
        ensAnalysisadaptorGetBaseadaptor(aa));
}




/* @section object retrieval **************************************************
**
** Functions for fetching Ensembl Analysis objects from an
** Ensembl SQL database.
**
** @fdata [EnsPAnalysisadaptor]
**
** @nam3rule Fetch Fetch Ensembl Analysis object(s)
** @nam4rule All Fetch all Ensembl Analysis objects
** @nam4rule Allby Fetch all Ensembl Analysis objects matching a criterion
** @nam5rule Featureclass Fetch all by Feature class
** @nam4rule By Fetch one Ensembl Analysis object matching a criterion
** @nam5rule Identifier Fetch by an SQL database internal identifier
** @nam5rule Name Fetch by a name
**
** @argrule * aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @argrule All analyses [AjPList] AJAX List of Ensembl Analysis objects
** @argrule AllbyFeatureclass class [const AjPStr] Ensembl Feature class
** @argrule AllbyFeatureclass analyses [AjPList]
** AJAX List of Ensembl Analysis objects
** @argrule ByIdentifier identifier [ajuint] SQL database-internal identifier
** @argrule ByIdentifier Panalysis [EnsPAnalysis*] Ensembl Analysis address
** @argrule ByName name [const AjPStr] Ensembl Analysis name
** @argrule ByName Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @valrule * [AjBool] ajTrue upon success, ajFalse otherwise
**
** @fcategory use
******************************************************************************/




/* @funcstatic analysisadaptorFetchAll ****************************************
**
** An ajTableMap "apply" function to return all Ensembl Analysis objects from
** the Ensembl Analysis Adaptor-internal cache.
**
** @param [u] key [const void*] AJAX unsigned integer key data address
** @param [u] Pvalue [void**] Ensembl Analysis value data address
** @param [u] cl [void*]
** AJAX List of Ensembl Analysis objects, passed in via ajTableMap
** @see ajTableMap
**
** @return [void]
**
** @release 6.3.0
** @@
******************************************************************************/

static void analysisadaptorFetchAll(const void *key,
                                    void **Pvalue,
                                    void *cl)
{
    if (!key)
        return;

    if (!Pvalue)
        return;

    if (!*Pvalue)
        return;

    if (!cl)
        return;

    ajListPushAppend((AjPList) cl,
                     (void *) ensAnalysisNewRef(*((EnsPAnalysis *) Pvalue)));

    return;
}




/* @func ensAnalysisadaptorFetchAll *******************************************
**
** Fetch all Ensembl Analysis objects.
**
** The caller is responsible for deleting the Ensembl Analysis objects before
** deleting the AJAX List object.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_all
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchAll(
    EnsPAnalysisadaptor aa,
    AjPList analyses)
{
    if (!aa)
        return ajFalse;

    if (!analyses)
        return ajFalse;

    if (!aa->CacheByIdentifier)
        analysisadaptorCacheInit(aa);

    ajTableMap(aa->CacheByIdentifier,
               &analysisadaptorFetchAll,
               (void *) analyses);

    return ajTrue;
}




/* @func ensAnalysisadaptorFetchAllbyFeatureclass *****************************
**
** Fetch all Ensembl Analysis objects referenced by an Ensembl Feature class.
**
** Please see the analysisadaptorFeatureClasses array for a list of valid
** Feature classes that reference Ensembl Analysis objects.
**
** The caller is responsible for deleting the Ensembl Analysis objects before
** deleting the AJAX List.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_all_by_feature_class
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] class [const AjPStr] Ensembl Feature class
** @param [u] analyses [AjPList] AJAX List of Ensembl Analysis objects
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.4.0
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchAllbyFeatureclass(
    EnsPAnalysisadaptor aa,
    const AjPStr class,
    AjPList analyses)
{
    register ajuint i = 0U;
    ajuint identifier = 0U;
    ajuint match      = 0U;

    AjPSqlstatement sqls = NULL;
    AjISqlrow sqli       = NULL;
    AjPSqlrow sqlr       = NULL;

    AjPStr statement = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPDatabaseadaptor dba = NULL;

    if (!aa)
        return ajFalse;

    if (!(class && ajStrGetLen(class)))
        return ajFalse;

    if (!analyses)
        return ajFalse;

    for (i = 0U; analysisadaptorKFeatureClasses[i]; i += 2U)
        if (ajStrMatchC(class, analysisadaptorKFeatureClasses[i]))
            match = i + 1U;

    if (match)
    {
        dba = ensAnalysisadaptorGetDatabaseadaptor(aa);

        statement = ajFmtStr(
            "SELECT DISTINCT %s.analysis_id FROM %s",
            analysisadaptorKFeatureClasses[match],
            analysisadaptorKFeatureClasses[match]);

        sqls = ensDatabaseadaptorSqlstatementNew(dba, statement);

        sqli = ajSqlrowiterNew(sqls);

        while (!ajSqlrowiterDone(sqli))
        {
            identifier = 0;

            sqlr = ajSqlrowiterGet(sqli);

            ajSqlcolumnToUint(sqlr, &identifier);

            ensAnalysisadaptorFetchByIdentifier(aa, identifier, &analysis);

            if (analysis)
                ajListPushAppend(analyses, (void *) analysis);
            else
                ajWarn("ensAnalysisadaptorFetchAllbyFeatureclass found "
                       "Ensembl Analysis identifier %u in the '%s' table, "
                       "which is not referenced in the 'analysis' table.\n",
                       identifier,
                       analysisadaptorKFeatureClasses[match]);
        }

        ajSqlrowiterDel(&sqli);

        ensDatabaseadaptorSqlstatementDel(dba, &sqls);

        ajStrDel(&statement);
    }
    else
    {
        ajDebug("ensAnalysisadaptorFetchAllbyFeatureclass got invalid "
                "Feature class '%S'\n",
                class);

        return ajFalse;
    }

    return ajTrue;
}




/* @func ensAnalysisadaptorFetchByIdentifier **********************************
**
** Fetch an Ensembl Analysis by its SQL database-internal identifier.
** The caller is responsible for deleting the Ensembl Analysis.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_by_dbID
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] identifier [ajuint] SQL database-internal identifier
** @param [wP] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchByIdentifier(
    EnsPAnalysisadaptor aa,
    ajuint identifier,
    EnsPAnalysis *Panalysis)
{
    AjBool result = AJFALSE;

    if (!aa)
        return ajFalse;

    if (!identifier)
        return ajFalse;

    if (!Panalysis)
        return ajFalse;

    /*
    ** Initially, search the identifier cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!aa->CacheByIdentifier)
        analysisadaptorCacheInit(aa);

    *Panalysis = (EnsPAnalysis) ajTableFetchmodV(aa->CacheByIdentifier,
                                                 (const void *) &identifier);

    if (*Panalysis)
    {
        ensAnalysisNewRef(*Panalysis);

        return ajTrue;
    }

    /* For a cache miss re-query the database. */

    result = ensBaseadaptorFetchByIdentifier(
        ensAnalysisadaptorGetBaseadaptor(aa),
        identifier,
        (void **) Panalysis);

    analysisadaptorCacheInsert(aa, Panalysis);

    return result;
}




/* @func ensAnalysisadaptorFetchByName ****************************************
**
** Fetch an Ensembl Analysis by its name.
** The caller is responsible for deleting the Ensembl Analysis.
**
** @cc Bio::EnsEMBL::DBSQL::AnalysisAdaptor::fetch_by_logic_name
** @param [u] aa [EnsPAnalysisadaptor] Ensembl Analysis Adaptor
** @param [r] name [const AjPStr] Ensembl Analysis name
** @param [wP] Panalysis [EnsPAnalysis*] Ensembl Analysis address
**
** @return [AjBool] ajTrue upon success, ajFalse otherwise
**
** @release 6.2.0
** @@
******************************************************************************/

AjBool ensAnalysisadaptorFetchByName(
    EnsPAnalysisadaptor aa,
    const AjPStr name,
    EnsPAnalysis *Panalysis)
{
    char *txtname = NULL;

    AjBool result = AJFALSE;

    AjPList analyses = NULL;

    AjPStr constraint = NULL;

    EnsPAnalysis analysis = NULL;

    EnsPBaseadaptor ba = NULL;

    if (!aa)
        return ajFalse;

    if (!(name && ajStrGetLen(name)))
        return ajFalse;

    if (!Panalysis)
        return ajFalse;

    /*
    ** Initially, search the name cache.
    ** For any object returned by the AJAX Table the reference counter needs
    ** to be incremented manually.
    */

    if (!aa->CacheByName)
        analysisadaptorCacheInit(aa);

    *Panalysis = (EnsPAnalysis) ajTableFetchmodS(aa->CacheByName, name);

    if (*Panalysis)
    {
        ensAnalysisNewRef(*Panalysis);

        return ajTrue;
    }

    /* In case of a cache miss, re-query the database. */

    ba = ensAnalysisadaptorGetBaseadaptor(aa);

    ensBaseadaptorEscapeC(ba, &txtname, name);

    constraint = ajFmtStr("analysis.logic_name = '%s'", txtname);

    ajCharDel(&txtname);

    analyses = ajListNew();

    result = ensBaseadaptorFetchAllbyConstraint(
        ba,
        constraint,
        (EnsPAssemblymapper) NULL,
        (EnsPSlice) NULL,
        analyses);

    if (ajListGetLength(analyses) > 1)
        ajWarn("ensAnalysisadaptorFetchByName got more than one "
               "Ensembl Analysis for (UNIQUE) name '%S'.\n",
               name);

    ajListPop(analyses, (void **) Panalysis);

    analysisadaptorCacheInsert(aa, Panalysis);

    while (ajListPop(analyses, (void **) &analysis))
    {
        analysisadaptorCacheInsert(aa, &analysis);

        ensAnalysisDel(&analysis);
    }

    ajListFree(&analyses);

    ajStrDel(&constraint);

    return result;
}

/*  Last edited: May 22 15:48 2000 (pmr) */
#ifdef __cplusplus
extern "C"
{
#endif

/*
**
** ajfeat.h - AJAX Sequence Feature include file
**            Version 1.0 - June 1999
**
*/

#ifndef ajfeat_h
#define ajfeat_h

#include <stdlib.h>
#include <stdio.h>

#include "ajdefine.h"
#include "ajexcept.h"
#include "ajmem.h"
#include "ajreg.h"
#include "ajstr.h"
#include "ajfile.h"
#include "ajtime.h"
#include "ajfmt.h"

/**********************************************************************************
**
** Longterm Concept: to derive an abstract OOP hierarchy 
**                   for 'mappable' genomic features, 
**                   starting with 'Sequence Features', and in particular,
**                   'DNA Sequence features' in the spirit of 
**                   the 'Gene Feature Finding' format
**
** Legend:
**    Aj*  == single object instance
**    AjP* == pointer reference to an object instance
**    ()   == Fixed n-tuple (C/C++ struct/union/class)
**    {}   == variable set (list or table)
**
** In the following, the lefthand most Aj* identifier is a class. A class
** hierarchy is defined by indentation, e.g. AjMapSequence 'isa' AjMap, so in
** AjFeatSequence, an AjPMap inherited from AjFeature points to an AjMapSequence.
**
** AjPosition
**    AjPoint
**    AjRange      -- (AjStart,AjEnd)
**    AjInterval   -- (AjPMapObject, AjPMapObject)
**    AjChromBand
**
** AjGenome        -- (AjSpecies,AjIdentity)  # 'Identity' is specific source: strain, individual, library, etc.
** AjChromosome    -- (AjName)               
**
** AjPFeatObject
**    AjFeatTable -- ({AjFeature})
**       AjMap -- (AjGenome,AjChromosome,AjPosition,{AjMapObject})
**          AjMapGenetic   -- {AjFeatGenetic}
**          AjMapPhysical  -- {AjFeatPhysical}
**             AjMapContig -- {AjClone}
**             AjMapRH     -- {AjMarker}
**             AjMapSTS    -- {AjMarker}
**          AjMapSequence  -- (AjSeq,{AjFeatSequence})
**
**    AjFeature -- (AjFeatTable,AjSource,AjType,AjPosition,AjScore,{AjTag})
**       AjMapObject     -- (AjAccessionId)
**          AjSGeneticLocus
**             AjMarker         -- {AjAllele}
**             AjGene           -- {AjAllele}
**          AjPhysicalLocus
**             AjMarker         -- {AjAllele}
**             AjClone          -- (AjSeq)
**          AjFeatSequence
**             AjFeatDNA        -- (AjStrand,AjFrame)
**             AjFeatRNA
**             AjFeatProtein
**
** External (database) formatted feature formats 
** such as EMBL, GFF, etc are recorded as AjDataFormat types
**
** Implementation Philosophy:
**
** Where a subclass is derived from a base class, the first data field
** defined in the subclass is an variable declaration of the base class type.
** In this way, assuming structure alignments and ordering of variables
** in memory by order of declaration, casting a pointer of a subclass
** to the base class gives access to the baseclass object data.
**
** Generally, subclass construction/initializations need to provide initial values
** for all base class object construction/initializations...
**
**********************************************************************************/


/* @enum AjEFeatClass *******************************************************
**
** Ajax Enumeration for AjSFeatObject types
** 
** @value  AjCFeatUnknown          [] Unknown feature class
** @value  AjCFeature              [] Generic feature
** @value     AjCMapObject         [] Mapped feature
** @value        AjCGeneticLocus   [] Genetic map locus feature
** @value        AjCPhysicalLocus  [] Physical map local feature
** @value        AjCFeatSequence   [] Sequence level feature
** @value           AjCFeatDNA     [] DNA sequence feature
** @value           AjCFeatRNA     [] RNA sequence feature
** @value           AjCFeatProtein [] Protein sequence feature
** @value  AjCFeatTable            [] Generic feature table class
** @value     AjCMap               [] Set of mapped features
** @value        AjCMapGenetic     [] Genetic map of features
** @value        AjCMapPhysical    [] Physical map of features
** @value        AjCMapSequence    [] Sequence feature set 
** @@
**
** Enumeration constant values are specially designed for class/subclass
** discrimination, but are not otherwise of special significance.
**
******************************************************************************/
typedef enum { AjCFeatUnknown             = 0x00,
               AjCFeature                 = 0x40,
                   AjCMapObject           = 0x60,
                       AjCGeneticLocus    = 0x6A, 
                       AjCPhysicalLocus   = 0x6C,
                       AjCFeatSequence    = 0x70,
                           AjCFeatDNA     = 0x71,
                           AjCFeatRNA     = 0x72, 
                           AjCFeatProtein = 0x74,
               AjCFeatTable               = 0x80, 
                   AjCMap                 = 0xA0, 
                       AjCMapGenetic      = 0xAA, 
                       AjCMapPhysical     = 0xAC,
                       AjCMapSequence     = 0xB0
} AjEFeatClass ;




/* @data AjPFeatObject *******************************************************
**
** Ajax superclass for AjFeature and AjFeatTable classes
**
** This object class is never created separately, but is initialized internally
** within ajfeat.c during feature object creation, and is a type cast for 
** the 'Class' information in all other feature object classes. The first
** structure data field of AjFeature and AjFeatTable is assumed to be
** a AjEFeatClass variable called 'Class', which contains this information.
**
** @alias AjSFeatObject
** @alias AjOFeatObject
**
** @cast   ajFeatObjClass    Returns the 'AjEFeatClass' of an object
** @cast   ajFeatObjTest     Predicate test for the 'AjFeatClass' of an object
** @cast   ajFeatObjVerify   Exception throwing version of ajFeatObjTest
** @new    ajFeaturesRead    Generic function to read in genome features from a file
** @set    ajFeaturesWrite   Generic function to dump genome features to a file
** @@
**
******************************************************************************/
typedef struct AjSFeatObject {
     AjEFeatClass  Class ;
} AjOFeatObject,  *AjPFeatObject ;


typedef struct AjSFeatQuery {
  AjPStr Ufo;
  AjPStr Filename;
  AjPStr Formatstr;
  int Format;
} AjOFeatQuery, *AjPFeatQuery;


/******************************************************************************
** Descriptive Classes  *******************************************************
******************************************************************************/

/* @data AjPFeatLexicon *******************************************************
**
** Ajax object for the symbol table defining the (restricted) AjFeature field  Lexicon
**
** @alias AjSFeatLexicon
** @alias AjOFeatLexicon
**
** @new    ajFeatVocNew      Constructor
** @delete ajFeatVocDel      Default destructor
** @assign ajFeatVocCopy     Copy constructor
** @set    ajFeatVocReadOnly Sets/Clears readonly status of AjFeatLexicon table
** @set    ajFeatVocAddTag   Returns the existing or newly created 'key' of a tag in the table
** @cast   ajFeatVocTagKey   Returns the 'key' for a tag in the table; returns NULL if tag is absent
** @mod    ajFeatVocClearTag Removes a specified tag from the symbol table
** @mod    ajFeatVocClearAll Removes all tags from the symbol table
** @@
******************************************************************************/
  /*typedef void *AjPFeatKey ;*/ /* is an index into a AjFeatLexicon  */


#define MAIN_FEATURE  0x0001
#define MAIN_TAG      0x0002
#define TAG_TEXT      0x0004
#define TAG_QTEXT     0x0008
#define TAG_SBI       0x0010
#define TAG_LIMITED   0x0020
#define TAG_VOID      0x0040
#define TAG_MANDATORY 0x0080  /* mandatory tag */
#define TAG_GFF       0x0100

  /* List of possible tag's in the dictionary */
typedef struct AjSFeatVocTag {
  AjPStr name;                 /* Name */
  int flags;                   /* Flags used for what type it is. i.e. TEXT QTEXT VOID etc */
  AjPList limitedValues;       /* Values allowed for LIMITED types */
} AjOFeatVocTag, *AjPFeatVocTag ;

  /* list of tags for a given feature */

typedef struct AjSFeatVocTagForFeat {
  AjPFeatVocTag VocTag;
  AjBool        mandatory;
} AjOFeatVocTagForFeat, *AjPFeatVocTagForFeat ;

  /* list of all features in the dictionary */
typedef struct AjSFeatVocFeat {
  AjPStr name;                 /* Name */
  int flags;                   /* used if feature has a Mandatory Tag */
  AjPList Tags;                /* List of Tags allowed (stored as the key to AjSFeatTag) */
} AjOFeatVocFeat, *AjPFeatVocFeat ;

typedef struct AjSFeatLexicon {
  AjBool    ReadOnly ;
  AjPTable  FeatVocTable ;                  /* FEATURES */
  AjPTable  TagVocTable ;                   /* Tags     */
  int       format;                         /* NO_DICT_FORMAT 0, EMBL_DICT_FORMAT 1, GFF_DICT_FORMAT 2 */ 
} AjOFeatLexicon,  *AjPFeatLexicon ;


/****************************************************************************
** Table Classes  ************************************************************K**********************
******************************************************************************/

/* @enum AjEDataFormat *******************************************************
**
** Ajax Enumeration for AjPFeature/AjFeatTable objects
** 
** @value  AjDF_Unknown   [0] Unknown format type
** @value  AjDF_ACEDB     []  ACEDB database format
** @value  AjDF_EMBL      []  EMBL database format
** @value  AjDF_SWISSPROT []  Swiss protein database format
** @value  AjDF_GENBANK   []  Genbank database format
** @value  AjDF_DDBJ      []  DDBJ database format
** @value  AjDF_GFF       []  'Gene Feature Finding' format
** @@
******************************************************************************/
typedef enum { AjDF_Unknown = 0,
               AjDF_ACEDB, 
               AjDF_EMBL, 
               AjDF_SWISSPROT, 
               AjDF_GENBANK, 
               AjDF_DDBJ, 
               AjDF_GFF 
} was_AjEDataFormat ;


typedef enum { AjEFeatIOUnknown = 0,
	       AjEFeatInput, AjEFeatOutput
} AjEFeatIOType ;  


/* @data AjPFeatTabIn *******************************************************
**
** Ajax class for feature table input
**
** @alias AjSFeatTabIn
** @alias AjOFeatTabIn
**
** @new    ajFeatUfoProcess Constructor based upon a 'UFO' and file mode (rwa)
** @delete ajFeatTabInClose Destructor, closes file and releases resources
** @@
******************************************************************************/
typedef struct AjSFeatTabIn {
   AjEFeatIOType Mode ;
   AjPStr        Ufo;
   AjPStr        Formatstr;
   AjEFeatClass  Type ; 
   int           Format;
   AjPStr        Filename;
   AjPStr        Entryname;
   AjPFileBuff   Handle ;
   AjPStr        Seqname ;  /* name of AjPSeq assoc. with feature table */
}  AjOFeatTabIn, *AjPFeatTabIn ;  

/* @data AjPFeatTabOut *******************************************************
**
** Ajax class for feature table output
**
** @alias AjSFeatTabOut
** @alias AjOFeatTabOut
**
** @new    ajFeatUfoProcess Constructor based upon a 'UFO' and file mode (rwa)
** @delete ajFeatTabOutClose Destructor, closes file and releases resources
** @@
******************************************************************************/

typedef struct AjSFeatTabOut {
   AjEFeatIOType Mode ;
   AjPStr        Ufo;
   AjPStr        Formatstr;
   AjEFeatClass  Type ; 
   int           Format;
   AjPStr        Filename;
   AjPStr        Entryname;
   AjPFile       Handle ;
   AjPStr        Seqname ;  /* name of AjPSeq assoc. with feature table */
}  AjOFeatTabOut, *AjPFeatTabOut ;  


/* @data AjPFeatTable *******************************************************
**
** Ajax Collection base class for AjPFeatures. The table may have a default
** data Format, a format 'Version' and a 'Data' timestamp associated with it.
** The table maintains a 'dictionary' of tags associated with the 'Source', 
** 'Type' and 'Tag' fields of its component AjPFeatures. The features themselves may
** be accessed via iteration methods.
**
** @alias AjSFeatTable
** @alias AjOFeatTable
**
** @new    ajFeatTabNew          Constructor
** @delete ajFeatTabDel          Default destructor
** @assign ajFeatTabCopy         Copy constructor
** @mod    ajFeatTabAdd          Adds an AjPFeature to a set
** @mod    ajFeatTabSetVersion   (Re)sets the current  AjPFeatTable data
**                               format version
** @mod    ajFeatTabSetDate      (Re)sets the current date of the 
**                               AjPFeatTable (file) dataset
** @mod    ajFeatTabDefFormat    Sets the original input ('source') data
**                               format for the table.
** @mod    ajFeatTabDefSource    Sets default 'Source' label for features
**                               created without source label specified
** @mod    ajFeatTabDefType      Sets default 'Type' label for features
**                               created without type label specified
** @cast   ajFeatTabFormat       Returns the current AjPFeatTable data
**                               format type
** @cast   ajFeatTabVersion      Returns the current AjPFeatTable data
**                               format version
** @cast   ajFeatTabDate         Returns the date of the AjPFeatTable
**                               (file) dataset
** @cast   ajFeatTabDictionary   Returns the AjFeatLexicon 'dictionary'
**                               of feature tags
** @cast   ajFeatTabFirst        Iterator: returns the first AjPFeature
**                               of a set
** @cast   ajFeatTabNext         Iterator: returns the next AjPeature of
**                               a set (ajFeatTabFirst method must be called
**                               first before using this method)
** @cast   ajFeatTabCount        Returns the number of features in the
**                               feature set
** @set    ajFeatTabMerge        Merges another AjPFeatTable to the current one
** @mod    ajFeatTabRead         Reads in a feature set in a specified format
** @set    ajFeatTabHeader       Dumps out the (Format specific) header
**                               describing a feature set
** @set    ajFeatTabDump         Dumps the feature set to a file, in a
**                               specified format
** @use    ajFeatTabDumpMatches  Dumps the feature set to a file, with
**                               information about features cross matched
**                               between different feature tables.
**
** Functions being contemplated but not yet implemented:
**
** #cast   ajFeatTabMember       Predicate to test AjFeature membership in
**                               a table
** #set    ajFeatTabUnion        Returns 'union set' of two feature tables
** #set    ajFeatTabIntersection Returns 'intersection set' of two feature
**                               tables
** #set    ajFeatTabDifference   Returns 'difference set' of one 
**                               feature table 'subtracted' again another
** #mod    ajFeatTabSort         Sorts a feature table based upon user
**                               defined criteria
** #set    ajFeatTabFilter       Returns a new feature table based 
**                               upon a filtered version of an existing one
** #mod    ajFeatTabRewrite      Method to systematically rewrite source,
**                               type and tag fields
**                               of all the AjFeature's in the table
** #mod    ajFeatTabRemap        Method to systematically offset the
**                               AjPosition of all the AjFeature's in the table
** #set    ajFeatTabCluster      Returns an array of feature tables each
**                               containing a group of features sharing some
**                               attribute (by pairwise comparison) 
** #set    ajFeatTabGroup        Returns an table of feature tables each
**                               containing a group of features sharing
**                               some fixed, named attribute 
** #set    ajFeatTabRange        Returns the features of a feature table 
**                               which lie within a given AjRange
** #set    ajFeatTabOverlap      Returns a feature table containing features
**                               with AjPosition 'overlaps', from two feature
**                               tables
** #set    ajFeatTabOverlapSelf  Returns a feature table filtered for
**                               'self overlaps'
** #cast   ajFeatTabOverlapStats Returns summary statistics about feature
**                               table matches
** #set    ajFeatTabStudy        Performs a general analysis of a feature
**                               table, for *Range, *Score cast's
** #cast   ajFeatTabMinRange     Returns the minimum start AjPosition of a
**                               feature table
** #cast   ajFeatTabMaxRange     Returns the maximum end AjPosition of a
**                               feature table
** #cast   ajFeatTabMinScore     Returns the minimum score of a feature table
** #cast   ajFeatTabMaxScore     Returns the maximum score of a feature table
** #cast   ajFeatTabAveScore     Returns the average score of a feature table
** @@
******************************************************************************/

typedef struct AjSFeatTable {
   AjEFeatClass      Class ;
   AjPStr            Name ;
   float             Version ;
   AjPTime           Date ;
   AjPFeatLexicon    Dictionary ;  /* Hash table for Source, Type and
                                      Tag identifiers in contained
                                      Features; keyed by
                                      AjOFeatureKey's */
   int               DefFormat ; /* Original input or 'source' format
				    of the feature table */
   AjPFeatVocFeat    DefSource ;
   AjPFeatVocFeat    DefType ;
   AjPList           Features ;	/* IsA List of AjPFeatures... */
   int               Start;	/* a.k.a. GFF region... */
   int               End;
}  AjOFeatTable, *AjPFeatTable ;  

/* @enum AjEFeatStrand *******************************************************
**
** Ajax Enumeration for DNA sequence strand identity
** 
** @value  AjStrandUnknown [0]    Unknown or not applicable strand 
** @value  AjStrandWatson  ['+']  Plus ('Watson') strand
** @value  AjStrandCrick   ['-']  Minus ('Crick') strand
** @@
******************************************************************************/

typedef enum { AjStrandUnknown = 0, 
               AjStrandWatson  = '+',    
               AjStrandCrick   = '-' 
} AjEFeatStrand ;

/* @enum AjEFeatFrame *******************************************************
**
** Ajax Enumeration for (coding) DNA sequence frame identity
** 
** @value  AjFrameUnknown [0] Unknown or not applicable frame
** @value  AjFrameZero    []  Frame 0
** @value  AjFrameOne     []  Frame 1
** @value  AjFrameTwo     []  Frame 2
** @@
******************************************************************************/

typedef enum { AjFrameUnknown = 0, 
               AjFrameZero, 
               AjFrameOne, 
               AjFrameTwo 
} AjEFeatFrame  ;


/******************************************************************************
** Feature Classes  ***********************************************************
******************************************************************************/


/* @data AjPFeature *******************************************************
**
** Ajax Biological Feature object superclass.
**
** Holds generic data describing a single genome feature.
**
** A genome feature (in AJAX program context) is a description of a
** genomic entity which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Primary_Tag'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Position' in the genome, and may have
** any arbitrary number of descriptor tags associated with it.
**
** Generally, all 'constructors' require specification of an 'owner'
** of the feature, which is an AjPFeatTable object (or subclass
** thereof) to which the new feature is automatically appended.
**
** A feature may also be composed of subfeatures (specified as an
** AjPFeatTable)
**
** This class of object serves as a base class for Genetic, Physical
** and Sequence level map object classes of the genome.
** The collection class for this object type is 'AjFeatTable'.
**
** @alias AjSFeature
** @alias AjOFeature
**
** @new    ajFeatNew             Constructor - must specify the associated
**                               (non-null) AjPFeatTable?
** @new    ajFeatNewFromTPS      Constructor: given type, position & score,
**                               assumes default Source
** @new    ajFeatNewFromPS       Constructor: given position & score,
**                               assumes default Source and Type 
** @delete ajFeatDel             Default destructor
** @assign ajFeatCopy            Copy constructor
** @set    ajFeatSetSource       Sets the name of the analysis which
**                               ascertained the feature
** @set    ajFeatSetType         Sets the type of feature (e.g. exon, etc.)
** @set    ajFeatSetPos          Sets the AjPosition for the feature
** @set    ajFeatOffset          Offsets to the AjPosition of a feature by
**                               a specified fixed amount. 
** @set    ajFeatSetScore        Sets the score associated with the feature,
**                               if any
** @set    ajFeatSetTagValue     Sets a specified tag for a feature and any
**                               (optional) value associated with it
**                               If a value is unspecified (NULL), then the
**                               current value associated with the tag
**                               if returned. If a new value is specified,
**                               the old value is returned.
** @set    ajFeatClearTags       Clears all tags (except Source and Type tag)
** @set    ajFeatAddSubFeature   Adds a subsidiary AjFeature to the feature
** @cast   ajFeatOwner           Returns the 'owner' AjPFeatTable for the
**                               feature
** @cast   ajFeatSource          Returns the name of the analysis which
**                               ascertained the feature
** @cast   ajFeatType            Returns the type of feature (e.g. exon, etc.) 
** @cast   ajFeatPosition        Returns the AjPosition for the feature
** @cast   ajFeatLength          Returns the 'length' (AjPosition end -
**                               start +1 ) of the feature
** @cast   ajFeatScore           Returns the score associated with the
**                               feature, if any
** @cast   ajFeatTag             Returns the value associated with a
**                               specified tag associated with the feature
** @cast   ajFeatTagSet          Returns a list of the tags associated
**                               with the feature, including the source and
**                               type tags
** @cast   ajFeatSubFeatures     Returns the AjPFeatTable of subfeatures of
**                               the feature
** @set    ajFeatConcat          Concatenates two AjFeatures (AjPosition
**                               ranges) if overlap is detected.
** @cast   ajFeatOverlap         Tests for an overlap match (in AjPosition)
**                               between two ajFeatures
** @mod    ajFeatFromLine        Parses in a string, in a specified feature
**                               format
** @use    ajFeatDumpString      Dumps the feature to a string, in a
**                               specified format
** @use    ajFeatDump            Dumps the feature to a file, in a specified
**                               format
** @@
******************************************************************************/
typedef struct AjSFeature {
  AjEFeatClass      Class ;
  AjPFeatTable      Owner ;
  AjPFeatVocFeat     Source ;
  AjPFeatVocFeat     Type ;
  int               Start ;
  int               End;
  int               Start2;
  int               End2;
  AjPStr            Score ;
  AjPList           Tags ; /* a.k.a. the [group] field tag-values of GFF2 */
  AjPStr            Comment ;
  AjEFeatStrand     Strand ;
  AjEFeatFrame      Frame ;
  AjPStr            desc ;
  int               Flags;
  int               Group;
  int               Exon;
} AjOFeature, *AjPFeature ;

typedef struct FeatTagValueStruct {
  AjPFeatVocTagForFeat Tag ;
  AjPStr     Value ;                   /* WHY NOT JUST AjPStr ??? */
} FeatTagValue, *LPFeatTagValue ;

/* ========================================================================= */
/* ========== All functions in (more or less) alphabetical order =========== */
/* ========================================================================= */

/*
 * Some global functions, not in order
 */

  /* Feature Table Creation */
AjBool               ajFeatTabOutOpen (AjPFeatTabOut thys, AjPStr ufo);
AjPFeatTabIn         ajFeatTabInNew (void);
AjPFeatTabIn         ajFeatTabInNewSSF (AjPStr fmt, AjPStr name,
				       AjPFileBuff buff);
AjPFeatTabOut        ajFeatTabOutNew (void);
AjPFeatTable         ajFeaturesRead  ( AjPFeatTabIn  ftin ) ; 
AjPFeatTable         ajFeatTabNew    ( AjPStr name, AjPFeatLexicon dictionary) ;
AjPFeatTable         ajFeatTabNewOut ( AjPStr name ) ;

  /* Feature Creation */
AjPFeature            ajFeatureNew(AjPFeatTable owner,
				   AjPStr source, AjPStr type,
				   int Start, int End,  AjPStr score,
				   AjEFeatStrand strand, AjEFeatFrame frame,
				   AjPStr desc , int Start2, int End2) ;

  /* Dictionary Creation */
AjPFeatLexicon ajFeatEmblDictionaryCreate();
AjPFeatLexicon ajFeatGffDictionaryCreate();

  /* Features Delete */
void                 ajFeatTabInDel( AjPFeatTabIn* pthis);
void                 ajFeatTabOutDel( AjPFeatTabOut* pthis);
void                 ajFeatDel(AjPFeature *pthis) ;
void *               ajFeatClearTag(AjPFeature thys, AjPStr tag) ;
void                 ajFeatTabDel (AjPFeatTable *pthis) ;
void                 ajFeatDictDel();
void                 ajFeatDeleteDict(AjPFeatLexicon dict);
  /* Feature Object Operators */
AjBool               ajFeaturesWrite ( AjPFeatTabOut ftout, AjPFeatTable ft) ; 

AjBool               ajFeatIsProt (AjPFeatTable thys);
int                  ajFeatLen (AjPFeatTable thys);
AjBool               ajFeatObjCheck(void *pObj, AjEFeatClass crass,
				    const char *file, int line) ;
void                 ajFeatObjAssert(void *pObj, AjEFeatClass crass,
				     const char *file, int line) ;

LPFeatTagValue       ajFeatSetTagValue(AjPFeature thys, AjPStr tag, AjPStr value,AjBool nomult) ;
void                 ajFeatTabAdd (AjPFeatTable thys, AjPFeature feature) ;
AjBool               ajFeatRead (AjPFeatTable* pthis,
				  AjPFeatTabIn tabin, AjPStr Ufo);
AjBool               ajFeatWrite (AjPFeatTable thys,
				   AjPFeatTabOut tabout, AjPStr Ufo);
void                 ajFeatTrace (AjPFeatTable thys);
AjPStr               ajFeatGetName (AjPFeatTable thys);
int                  ajFeatLen (AjPFeatTable thys);
int                  ajFeatSize (AjPFeatTable thys);
void                 ajFeatIgnoreFeat(AjPFeatTable FeatTab, AjPList featignore);
void                 ajFeatOnlyAllowFeat(AjPFeatTable FeatTab, AjPList featonlyallow);
void                 ajFeatOnlyAllowTag(AjPFeatTable FeatTab, AjPList list);
void                 ajFeatIgnoreTag(AjPFeatTable FeatTab, AjPList list);

void                 ajFeatSortByType(AjPFeatTable FeatTab);
void                 ajFeatSortByStart(AjPFeatTable FeatTab);
void                 ajFeatSortByEnd(AjPFeatTable FeatTab);

AjBool               ajFeatTableWriteDdbj (AjPFeatTable features, AjPFile file);
AjBool               ajFeatTableWriteEmbl (AjPFeatTable features, AjPFile file);
AjBool               ajFeatTableWriteGenbank (AjPFeatTable features, AjPFile file);
AjBool               ajFeatTableWriteGff (AjPFeatTable features, AjPFile file);
AjBool               ajFeatTableWriteSwiss (AjPFeatTable features, AjPFile file);

AjPFeatVocFeat       CheckDictForFeature(AjPFeatTable table, AjPStr feature);
AjPFeatVocTag        CheckDictForTag(AjPFeatTable table, AjPStr tag);
void                 ajFeatDickTracy(AjPFeatLexicon dictionary);




#define              ajFeatObjClass(p)  \
      ((p)?((AjPFeatObject)(p))->Class:AjCFeatUnknown)
#define              ajFeatObjTest(p,c)   ajFeatObjCheck ((p), (c), \
      __FILE__, __LINE__)
#define              ajFeatObjVerify(p,c) ajFeatObjAssert((p), (c), \
      __FILE__, __LINE__)
#define              ajFeatSetPos(p,v)   (ajFeatObjVerify((p),AjCFeature),\
         ((AjPFeature)(p))->Position=(v))
#define               ajFeatSetScore(p,s) (ajFeatObjVerify((p),AjCFeature),\
         ((AjPFeature)(p))->Score=(s))
#define               ajFeatOwner(p)    (ajFeatObjVerify((p),AjCFeature),\
         (p)->Owner)
#define               ajFeatSource(p)   (ajFeatObjVerify((p),AjCFeature),\
         (p)->Source)
#define               ajFeatType(p)     (ajFeatObjVerify((p),AjCFeature),\
         (p)->Type)
#define               ajFeatPosition(p) (ajFeatObjVerify((p),AjCFeature),\
         (p)->Position)
#define               ajFeatLength(p)   (ajFeatObjVerify((p),AjCFeature),\
         ajPosRangeLength(&(((AjPFeature)(p))->Position)))
#define               ajFeatScore(p)    (ajFeatObjVerify((p),AjCFeature),\
         (p)->Score)
#define               ajFeatTabSetVersion(p,v) (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->Version=(v))
#define               ajFeatTabSetDate(p,d)    (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->Date=(d))
#define               ajFeatTabDefFormat(p,f)  (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->DefFormat=(f))
#define               ajFeatTabFormat(p)       (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->Format)
#define               ajFeatTabVersion(p)      (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->Version)
#define               ajFeatTabDate(p)         (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->Date)
#define               ajFeatTabDictionary(p)   (ajFeatObjVerify((p),\
         AjCFeatTable),(p)->Dictionary)
#define               ajFeatVocReadOnly(p,f) ((p)?(p)->ReadOnly=(f):AjTrue)



#endif /* ajfeat_h */

#ifdef __cplusplus
}
#endif

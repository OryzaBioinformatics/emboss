#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajnexus_h
#define ajnexus_h

#include "ajfile.h"
#include "ajstr.h"

/* @data AjPNexusTaxa *****************************************************
**
** Ajax nexus data taxa block object.
**
** @alias AjSNexusTaxa
** @alias AjONexusTaxa
**
** @new nexusTaxaNew Default constructor
**
** @delete nexusTaxaDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusTaxa {
    ajint Ntax;
    AjPStr* TaxLabels;
} AjONexusTaxa;
#define AjPNexusTaxa AjONexusTaxa*



/* @data AjPNexusCharacters ***************************************************
**
** Ajax nexus data characters block object.
**
** @alias AjSNexusCharacters
** @alias AjONexusCharacters
**
** @new nexusCharactersNew Default constructor
**
** @delete nexusCharactersDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusCharacters {
    AjBool NewTaxa;
    ajint Ntax;
    ajint Nchar;
    AjPStr DataType;
    AjBool RespectCase;
    char Missing;
    char Gap;
    AjPStr Symbols;
    AjPStr Equate;
    char MatchChar;
    AjBool Labels;
    AjBool Transpose;
    AjBool Interleave;
    AjPStr Items;
    AjPStr StatesFormat;
    AjBool Tokens;
    AjPStr Eliminate;
    AjPStr* CharStateLabels;
    AjPStr* CharLabels;
    AjPStr* StateLabels;
    AjPStr* Matrix;
    AjPStr* Sequences;
} AjONexusCharacters;
#define AjPNexusCharacters AjONexusCharacters*



/* @data AjPNexusUnaligned ****************************************************
**
** Ajax nexus data unaligned block object.
**
** @alias AjSNexusUnaligned
** @alias AjONexusUnaligned
**
** @new nexusUnalignedNew Default constructor
**
** @delete nexusUnalignedDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusUnaligned {
    AjBool NewTaxa;
    ajint Ntax;
    AjPStr DataType;
    AjBool RespectCase;
    char Missing;
    AjPStr Symbols;
    AjPStr Equate;
    AjBool Labels;
    AjPStr* Matrix;
} AjONexusUnaligned;
#define AjPNexusUnaligned AjONexusUnaligned*



/* @data AjPNexusDistances ****************************************************
**
** Ajax nexus data distances block object.
**
** @alias AjSNexusDistances
** @alias AjONexusDistances
**
** @new nexusDistancesNew Default constructor
**
** @delete nexusDistancesDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusDistances {
    AjBool NewTaxa;
    ajint Ntax;
    ajint Nchar;
    AjPStr Triangle;
    AjBool Diagonal;
    AjBool Labels;
    char Missing;
    AjBool Interleave;
    AjPStr* Matrix;
} AjONexusDistances;
#define AjPNexusDistances AjONexusDistances*



/* @data AjPNexusSets *********************************************************
**
** Ajax nexus data sets block object.
**
** @alias AjSNexusSets
** @alias AjONexusSets
**
** @new nexusSetsNew Default constructor
**
** @delete nexusSetsDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusSets {
    AjPStr* CharSet;
    AjPStr* StateSet;
    AjPStr* ChangeSet;
    AjPStr* TaxSet;
    AjPStr* TreeSet;
    AjPStr* CharPartition;
    AjPStr* TaxPartition;
    AjPStr* TreePartition;
} AjONexusSets;
#define AjPNexusSets AjONexusSets*



/* @data AjPNexusAssumptions **************************************************
**
** Ajax nexus data sssumptions block object.
**
** @alias AjSNexusAssumptions
** @alias AjONexusAssumptions
**
** @new nexusAssumptionsNew Default constructor
**
** @delete nexusAssumptionsDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusAssumptions {
    AjPStr DefType;
    AjPStr PolyTCount;
    AjPStr GapMode;
    AjPStr* UserType;
    AjPStr* TypeSet;
    AjPStr* WtSet;
    AjPStr* ExSet;
    AjPStr* AncStates;
} AjONexusAssumptions;
#define AjPNexusAssumptions AjONexusAssumptions*



/* @data AjPNexusCodons *******************************************************
**
** Ajax nexus data codons block object.
**
** @alias AjSNexusCodons
** @alias AjONexusCodons
**
** @new nexusCodonsNew Default constructor
**
** @delete nexusCodonsDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusCodons {
    AjPStr* CodonPosSet;
    AjPStr* GeneticCode;
    AjPStr* CodeSet;
} AjONexusCodons;
#define AjPNexusCodons AjONexusCodons*



/* @data AjPNexusTrees ********************************************************
**
** Ajax nexus data trees block object.
**
** @alias AjSNexusTrees
** @alias AjONexusTrees
**
** @new nexusTreesNew Default constructor
**
** @delete nexusTreesDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusTrees {
    AjPStr* Translate;
    AjPStr* Tree;
} AjONexusTrees;
#define AjPNexusTrees AjONexusTrees*



/* @data AjPNexusNotes *****************************************************
**
** Ajax nexus data notes block object.
**
** @alias AjSNexusNotes
** @alias AjONexusNotes
**
** @new nexusNotesNew Default constructor
**
** @delete nexusNotesDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexusNotes {
    AjPStr* Text;
    AjPStr* Picture;
} AjONexusNotes;
#define AjPNexusNotes AjONexusNotes*



/* @data AjPNexus *************************************************************
**
** Ajax nexus data object.
**
** @alias AjSNexus
** @alias AjONexus
**
** @new ajNexusNew Default constructor
**
** @delete ajNexusDel Default destructor
**
** @@
******************************************************************************/

typedef struct AjSNexus {
    ajint Ntax;
    AjPNexusTaxa Taxa;
    AjPNexusCharacters Characters;
    AjPNexusUnaligned Unaligned;
    AjPNexusDistances Distances;
    AjPNexusSets Sets;
    AjPNexusAssumptions Assumptions;
    AjPNexusCodons Codons;
    AjPNexusTrees Trees;
    AjPNexusNotes Notes;
    char *Ptr;
} AjONexus;
#define AjPNexus AjONexus*

void     ajNexusDel(AjPNexus* pthys);
ajint    ajNexusGetNtaxa(AjPNexus thys);
AjPStr*  ajNexusGetTaxa(AjPNexus thys);
AjPStr*  ajNexusGetSequences(AjPNexus thys);
AjPNexus ajNexusNew(void);
AjPNexus ajNexusParse(AjPFileBuff buff);
void     ajNexusTrace(AjPNexus thys);
#endif

#ifdef __cplusplus
}
#endif

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
** @attr Ntax [ajint] Number of taxons
** @attr TaxLabels [AjPStr*] Taxon names
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
** @attr NewTaxa [AjBool] New taxa read from data block
** @attr Ntax [ajint] Number of taxons
** @attr Nchar [ajint] Number of characters
** @attr DataType [AjPStr] Data type
** @attr RespectCase [AjBool] Respect case if true
** @attr Missing [char] Missing character in input data
** @attr Gap [char] Gap character in input data
** @attr Symbols [AjPStr] Character symbols
** @attr Equate [AjPStr] Character equivalent names
** @attr MatchChar [char] Matching charater in input
** @attr Labels [AjBool] Labels if true
** @attr Transpose [AjBool] Transpose data if true
** @attr Interleave [AjBool] Interleaved input if true
** @attr Items [AjPStr] Character items
** @attr StatesFormat [AjPStr] Statesformat string
** @attr Tokens [AjBool] If true, tokens set
** @attr Eliminate [AjPStr] Elimioate string
** @attr CharStateLabels [AjPStr*] Character and state labels
** @attr CharLabels [AjPStr*] Character labels
** @attr StateLabels [AjPStr*] State labels
** @attr Matrix [AjPStr*] Matrix data
** @attr Sequences [AjPStr*] Sequence data
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
** Very similar to a character block and will be merged with AjPNexusCharacter
** in the near future
**
** @alias AjSNexusUnaligned
** @alias AjONexusUnaligned
**
** @new nexusUnalignedNew Default constructor
**
** @delete nexusUnalignedDel Default destructor
**
** @attr NewTaxa [AjBool] New taxa read from data block
** @attr Ntax [ajint] Number of taxons
** @attr DataType [AjPStr] Data type
** @attr RespectCase [AjBool] Respect case if true
** @attr Missing [char] Missing character in input data
** @attr Symbols [AjPStr] Character symbols
** @attr Equate [AjPStr] Character equivalent names
** @attr Labels [AjBool] Labels if true
** @attr Matrix [AjPStr*] Matrix data
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
** @attr NewTaxa [AjBool] New taxa read from data block
** @attr Ntax [ajint] Number of taxons
** @attr Nchar [ajint] Number of characters
** @attr Triangle [AjPStr] Triangular distances block type
** @attr Diagonal [AjBool] If true, expect to read diagonal of matrix
** @attr Missing [char] Missing character in input data
** @attr Labels [AjBool] Labels if true
** @attr Interleave [AjBool] Interleaved input if true
** @attr Matrix [AjPStr*] Matrix data
** @@
******************************************************************************/

typedef struct AjSNexusDistances {
    AjBool NewTaxa;
    ajint Ntax;
    ajint Nchar;
    AjPStr Triangle;
    AjBool Diagonal;
    char Missing;
    AjBool Labels;
    AjBool Interleave;
    AjPStr* Matrix;
} AjONexusDistances;
#define AjPNexusDistances AjONexusDistances*



/* @data AjPNexusSets *********************************************************
**
** Ajax nexus data sets block object.
**
** The data is generally the NEXUS command strings as in the original file.
**
** @alias AjSNexusSets
** @alias AjONexusSets
**
** @new nexusSetsNew Default constructor
**
** @delete nexusSetsDel Default destructor
**
** @attr CharSet [AjPStr*] Character set commands
** @attr StateSet [AjPStr*] State set commands
** @attr ChangeSet [AjPStr*] Change set commands
** @attr TaxSet [AjPStr*] Taxon set commands
** @attr TreeSet [AjPStr*] Tree set commands
** @attr CharPartition [AjPStr*] Character partition statements
** @attr TaxPartition [AjPStr*] Taxa partition statements
** @attr TreePartition [AjPStr*] Tree partition statements
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
** @attr DefType [AjPStr] Options deftype subcommand
** @attr PolyTCount [AjPStr] Options polytcount subcommand
** @attr GapMode [AjPStr] Options gapmode subcommand
** @attr UserType [AjPStr*] Usertype commands
** @attr TypeSet [AjPStr*] Typeset commands
** @attr WtSet [AjPStr*] Wtset commands
** @attr ExSet [AjPStr*] Exset commands
** @attr AncStates [AjPStr*] Ancstates commands
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
** @attr CodonPosSet [AjPStr*] CodonPosSetcommands
** @attr GeneticCode [AjPStr*] GeneticCodecommands
** @attr CodeSet [AjPStr*] CodeSetcommands
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
** @attr Translate [AjPStr*] Translate commands
** @attr Tree [AjPStr*] Tree commands
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
** @attr Text [AjPStr*] Text commands
** @attr Picture [AjPStr*] Picture commands
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
** @attr Ntax [ajint] Number of taxa (wherever they were defined)
** @attr Taxa [AjPNexusTaxa] Taxa
** @attr Characters [AjPNexusCharacters] Characters (or data)
** @attr Unaligned [AjPNexusUnaligned] Unaligned
** @attr Distances [AjPNexusDistances] Distances
** @attr Sets [AjPNexusSets] Sets
** @attr Assumptions [AjPNexusAssumptions] Assumptions
** @attr Codons [AjPNexusCodons] Codons
** @attr Trees [AjPNexusTrees] Trees
** @attr Notes [AjPNexusNotes] Notes
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
} AjONexus;
#define AjPNexus AjONexus*

void     ajNexusDel(AjPNexus* pthys);
ajint    ajNexusGetNtaxa(const AjPNexus thys);
AjPStr*  ajNexusGetTaxa(const AjPNexus thys);
AjPStr*  ajNexusGetSequences(AjPNexus thys);
AjPNexus ajNexusNew(void);
AjPNexus ajNexusParse(AjPFileBuff buff);
void     ajNexusTrace(const AjPNexus thys);
#endif

#ifdef __cplusplus
}
#endif

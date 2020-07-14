#ifdef __cplusplus
extern "C"
{
#endif

/*
**
** ajfeatdata.h - AJAX Sequence Feature include file
**            Version 1.0 - March 2001
**
*/

#ifndef ajfeatdata_h
#define ajfeatdata_h

/******************************************************************************
** Table Classes  *************************************************************
******************************************************************************/

/* @data AjPFeattabIn *********************************************************
**
** Ajax class for feature table input
**
** @alias AjSFeattabIn
** @alias AjOFeattabIn
**
** @new    ajFeattabInNew Constructor
** @new    ajFeattabInNewSS Constructor with format, name and type
** @new    ajFeattabInNewSSF Constructor with format, name, type and input file
** @delete ajFeattabInDel Destructor
**
** @attr Ufo [AjPStr] Original UFO
** @attr Formatstr [AjPStr] Input format name
** @attr Format [ajint] Input format index
** @attr Filename [AjPStr] Original filename
** @attr Seqid [AjPStr] Sequence entryname
** @attr Type [AjPStr] Type N or P
** @attr Handle [AjPFileBuff] Input buffered file
** @attr Seqname [AjPStr] name of AjPSeq assoc. with feature table
** @@
******************************************************************************/

typedef struct AjSFeattabIn {
  AjPStr        Ufo;
  AjPStr        Formatstr;
  ajint         Format;
  AjPStr        Filename;
  AjPStr        Seqid;
  AjPStr        Type;
  AjPFileBuff   Handle;
  AjPStr        Seqname;
}  AjOFeattabIn;

#define AjPFeattabIn AjOFeattabIn*

/* @data AjPFeattabOut ********************************************************
**
** Ajax class for feature table output
**
** @alias AjSFeattabOut
** @alias AjOFeattabOut
**
** @new    ajFeattabOutNew Constructor
** @new    ajFeattabOutNewSSF Constructor with format, name, type and
**                            output file
** @delete ajFeattabOutDel Destructor
**
** @attr Ufo [AjPStr] Original output UFO
** @attr Formatstr [AjPStr] Output format name
** @attr Format [ajint] Output format index
** @attr Filename [AjPStr] Output filename
** @attr Directory [AjPStr] Output directory
** @attr Seqid [AjPStr] Output entryname
** @attr Type [AjPStr] Type N or P
** @attr Handle [AjPFile] Output file
** @attr Seqname [AjPStr] AjPSeq assoc. with feature table
** @attr Basename [AjPStr] Basename for output file
** @attr Local [AjBool] Opened as a local file if ajTrue
** @@
******************************************************************************/

typedef struct AjSFeattabOut {
  AjPStr        Ufo;
  AjPStr        Formatstr;
  ajint         Format;
  AjPStr        Filename;
  AjPStr        Directory;
  AjPStr        Seqid;
  AjPStr        Type;
  AjPFile       Handle;
  AjPStr        Seqname;
  AjPStr        Basename;
  AjBool        Local;
}  AjOFeattabOut;

#define AjPFeattabOut AjOFeattabOut*


/* @data AjPFeattable *********************************************************
**
** Ajax data type for collecting AjPFeatures in a 'feature table'.
** The features themselves may be accessed via iteration methods.
**
** @alias AjSFeattable
** @alias AjOFeattable
**
** @new    ajFeattableNew        Constructor
** @delete ajFeattableDel        Default destructor
** @modify ajFeattableAdd        Adds an AjPFeature to a set
** @new    ajFeatRead            Reads in a feature set in a specified format
**
** @attr Seqid [AjPStr] Sequence name
** @attr Type [AjPStr] Sequence type: P or N
** @attr DefFormat [ajint] Original input or 'source' format of feature table
** @attr Features [AjPList] List of AjPFeatures...
** @attr Start [ajint] First position used (like sequence begin)
** @attr End [ajint] Last position used (like sequence end)
** @attr Len [ajint] Maximum length
** @attr Offset [ajint] Offset when trimmed
** @attr Groups [ajint] Number of current group being added
** @@
******************************************************************************/

typedef struct AjSFeattable {
  AjPStr            Seqid;
  AjPStr            Type;
  ajint             DefFormat;
  AjPList           Features;
  ajint             Start;
  ajint             End;
  ajint             Len;
  ajint             Offset;
  ajint             Groups;
}  AjOFeattable;

#define AjPFeattable AjOFeattable*


/******************************************************************************
** Feature Classes  ***********************************************************
******************************************************************************/


/* @data AjPFeature ***********************************************************
**
** Ajax Biological Feature object superclass.
**
** Holds generic data describing a single genome feature.
**
** A feature is a description of a
** sequence location which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Type'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Location' in the genome, and may have
** any arbitrary number of descriptor Tags and TagValues associated with it.
**
**
** @alias AjSFeature
** @alias AjOFeature
**
** @new    ajFeatNew             Constructor - must specify the associated
**                               (non-null) AjPFeattable
** @new    ajFeatNewII           Simple constructor with only start and end
**                               positions
** @new    ajFeatNewIIRev        Simple constructor with only start and end
**                               positions, sets feature to be
**                               on the reverse strand
** @new    ajFeatNewProt         Protein-specific constructor -
**                               must specify the associated
**                               (non-null) AjPFeattable
** @delete ajFeatDel             Default destructor
** @new    ajFeatCopy            Copy constructor
**
** @attr Protein [AjBool] true for a protein feature
** @attr Source [AjPStr] Source program name (or EMBL)
** @attr Type [AjPStr] Feature type (feature key) from internal list
**                    for protein or nucleotide
** @attr Start [ajint] Start position
** @attr End [ajint] End position
** @attr Start2 [ajint] Second start position - EMBL (a.b)
** @attr End2 [ajint] Second end position - EMBL ..(a.b)
** @attr Score [float] Score or 0.0 if none
** @attr Tags [AjPList] Tag-value list (qualifier list)
** @attr Strand [char] Strand +/- or NULL
** @attr Frame [ajint] Frame 1..3, -1..-3 or 0
** @attr Flags [ajint] Flag bit mask for EMBL location
** @attr Group [ajint] Group for join/order/one-of
** @attr Exon [ajint] Exon number
** @attr Remote [AjPStr] Remote ID - EMBL Remote:a.b
** @attr Label [AjPStr] Label name for location - EMBL legacy
** @@
******************************************************************************/

typedef struct AjSFeature {
  AjBool            Protein;
  AjPStr            Source;
  AjPStr            Type;
  ajint             Start;
  ajint             End;
  ajint             Start2;
  ajint             End2;
  float             Score;
  AjPList           Tags;
  char              Strand;
  ajint             Frame;
  ajint             Flags;
  ajint             Group;
  ajint             Exon;
  AjPStr            Remote;

  AjPStr            Label;
} AjOFeature;

#define AjPFeature AjOFeature*


#endif /* ajfeatdata_h */

#ifdef __cplusplus
}
#endif

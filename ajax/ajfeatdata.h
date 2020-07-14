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
** @new    ajFeatUfoProcess Constructor based upon a 'UFO' and file mode (rwa)
** @delete ajFeattabInClose Destructor, closes file and releases resources
** @@
******************************************************************************/

typedef struct AjSFeattabIn {
  AjPStr        Ufo;		/* Original UFO */
  AjPStr        Formatstr;	/* Input format name */
  ajint         Format;		/* Input format enum */
  AjPStr        Filename;	/* Original filename */
  AjPStr        Seqid;		/* Sequence entryname */
  AjPStr        Type;		/* Type N or P */
  AjPFileBuff   Handle ;	/* Input buffered file */
  AjPStr        Seqname ;	/* name of AjPSeq assoc. with feature table */
}  AjOFeattabIn;

#define AjPFeattabIn AjOFeattabIn*

/* @data AjPFeattabOut ********************************************************
**
** Ajax class for feature table output
**
** @alias AjSFeattabOut
** @alias AjOFeattabOut
**
** @new    ajFeatUfoProcess Constructor based upon a 'UFO' and file mode (rwa)
** @delete ajFeattabOutClose Destructor, closes file and releases resources
** @@
******************************************************************************/

typedef struct AjSFeattabOut {
  AjPStr        Ufo;			/* Original output UFO */
  AjPStr        Formatstr;		/* Output format name */
  ajint         Format;			/* Output format enum */
  AjPStr        Filename;		/* Output filename */
  AjPStr        Directory;		/* Output directory */
  AjPStr        Seqid;			/* Output entryname */
  AjPStr        Type;			/* Type N or P */
  AjPFile       Handle;			/* Output file */
  AjPStr        Seqname;	        /* AjPSeq assoc. with feature table */
  AjPStr        Basename;		/* Basename for output file */
  AjBool        Local;			/* Opened as a local file */
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
** @delete ajFeattabDel          Default destructor
** @mod    ajFeattabAdd          Adds an AjPFeature to a set
** @mod    ajFeaturesRead        Reads in a feature set in a specified format
** @@
******************************************************************************/

typedef struct AjSFeattable {
  AjPStr            Seqid ;	/* Sequence name */
  AjPStr            Type ;	/* Sequence type: P or N */
  ajint             DefFormat ; /* Original input or 'source' format
				   of the feature table */
  AjPList           Features ;	/* List of AjPFeatures... */
  ajint             Start;      /* First position used (like sequence begin) */
  ajint             End;        /* Last position used (like sequence end) */
  ajint             Len;        /* Maximum length */
  ajint             Offset;     /* Offset when trimmed */
  ajint             Groups;	/* Number of current group being added */
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
**                               (non-null) AjPFeattable?
** @new    ajFeatNewFromTPS      Constructor: given type, position & score,
**                               assumes default Source
** @new    ajFeatNewFromPS       Constructor: given position & score,
**                               assumes default Source and Type
** @delete ajFeatDel             Default destructor
** @assign ajFeatCopy            Copy constructor
** @set    ajFeatSetSource       Sets the name of the analysis which
**                               ascertained the feature
** @set    ajFeatSetType         Sets the type of feature (e.g. exon, etc.)
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
** @cast   ajFeatSource          Returns the name of the analysis which
**                               ascertained the feature
** @cast   ajFeatType            Returns the type of feature (e.g. exon, etc.)
** @cast   ajFeatScore           Returns the score associated with the
**                               feature, if any
** @cast   ajFeatTag             Returns the value associated with a
**                               specified tag associated with the feature
** @cast   ajFeatTagSet          Returns a list of the tags associated
**                               with the feature, including the source and
**                               type tags
** @cast   ajFeatSubFeatures     Returns the AjPFeattable of subfeatures of
**                               the feature
** @mod    ajFeatFromLine        Parses in a string, in a specified feature
**                               format
** @use    ajFeatDumpString      Dumps the feature to a string, in a
**                               specified format
** @use    ajFeatDump            Dumps the feature to a file, in a specified
**                               format
** @@
******************************************************************************/

typedef struct AjSFeature {
  AjBool            Protein ;	/* true for a protein feature */
  AjPStr            Source ;	/* Source program name (or EMBL) */
  AjPStr            Type ;	/* Feature type (feature key) */
  ajint             Start ;	/* Start position */
  ajint             End;	/* End position */
  ajint             Start2;	/* Second start position - EMBL (a.b)*/
  ajint             End2;	/* Second end position - EMBL ..(a.b) */
  float             Score ;	/* Score or 0.0 if none */
  AjPList           Tags ;	/* Tag-value list (qualifier list) */
  char              Strand ;	/* Strand +/- or NULL */
  ajint             Frame ;	/* Frame 1..3, -1..-3 or 0 */
  ajint             Flags;	/* Flag bit mask for EMBL location */
  ajint             Group;	/* Group for join/order/one-of */
  ajint             Exon;	/* Exon number */
  AjPStr            Remote ;	/* Remote ID - EMBL Remote:a.b */

  /* Label is obsolete - remove if not in databases */
  AjPStr            Label ;	/* Label name for location - EMBL legacy */

  /*AjPStr            Desc ;*/	/* One-line description obsolete */
  /*AjPStr            Comment ;*/	/* Comment - obsolete */

} AjOFeature;

#define AjPFeature AjOFeature*


#endif /* ajfeatdata_h */

#ifdef __cplusplus
}
#endif

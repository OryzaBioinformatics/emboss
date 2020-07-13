/******************************************************************************
**
** a genome feature (in AJAX program context) is a description of a
** genomic entity which was determined by some 'source' analysis
** (which may be of 'wet lab' experimental or 'in silico'
** computational nature), has a 'primary' descriptor ('Primary_Tag'),
** may have some 'score' asserting the level of analysis confidence in
** its identity (e.g. log likelihood relative to a null hypothesis or
** other similar entity), has a 'Position' in the genome, and may have
** any arbitrary number of descriptor tags associated with it.
**
** @author Copyright (C) 1999 Richard Bruskiewich
** modified by Ian Longden.
** modified by Peter Rice.
** @version 3.0
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
**
** Implementation Notes:
**
** see also the header file, ajfeat.h, for details on class structure.
**
******************************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"

/* @datastatic FeatPTagval ****************************************************
**
** Feature tag values data structure
**
** @alias FeatSTagval
** @alias FeatOTagval
**
** @attr Tag [AjPStr] Tag name usually from a controlled internal vocabulary
** @attr Value [AjPStr] Tag value
** @@
******************************************************************************/

typedef struct FeatSTagval {
  AjPStr Tag;
  AjPStr Value;
} FeatOTagval, *FeatPTagval;

#define FEATFLAG_START_BEFORE_SEQ 0x0001 /* <start */
#define FEATFLAG_END_AFTER_SEQ    0x0002 /* >end */
#define FEATFLAG_CHILD            0x0004 /* join() */
#define FEATFLAG_BETWEEN_SEQ      0x0008  /* x^y */
#define FEATFLAG_START_TWO        0x0010  /* x.y.. */
#define FEATFLAG_END_TWO          0x0020  /* ..x.y */
#define FEATFLAG_POINT            0x0040  /* x */
#define FEATFLAG_COMPLEMENT_MAIN  0x0080  /* complement around the join */
#define FEATFLAG_MULTIPLE         0x0100  /* part of a multiple i.e. join*/
#define FEATFLAG_GROUP            0x0200  /* group */
#define FEATFLAG_ORDER            0x0400  /* order */
#define FEATFLAG_ONEOF            0x0800  /* one_of */
#define FEATFLAG_REMOTEID         0x1000  /* AB012345.6: */
#define FEATFLAG_LABEL            0x2000  /* label as location */

static AjBool   FeatInitDone = AJFALSE;
static AjPTable FeatTypeTableDna = NULL;
static AjPTable FeatTagsTableDna = NULL;
static AjPTable FeatTypeTableProtein = NULL;
static AjPTable FeatTagsTableProtein = NULL;

static AjBool   FeatInitGff = AJFALSE;
static AjPTable FeatTypeTableGff = NULL;
static AjPTable FeatTagsTableGff = NULL;

static AjBool   FeatInitEmbl = AJFALSE;
static AjPTable FeatTypeTableEmbl = NULL;
static AjPTable FeatTagsTableEmbl = NULL;

static AjBool   FeatInitPir = AJFALSE;
static AjPTable FeatTypeTablePir = NULL;
static AjPTable FeatTagsTablePir = NULL;

static AjBool   FeatInitSwiss = AJFALSE;
static AjPTable FeatTypeTableSwiss = NULL;
static AjPTable FeatTagsTableSwiss = NULL;

static void         featClear ( AjPFeature thys );
static ajint        featCompByEnd(const void *a, const void *b);
static ajint        featCompByGroup(const void *a, const void *b);
static ajint        featCompByStart(const void *a, const void *b);
static ajint        featCompByType(const void *a, const void *b);
static AjBool       featDelRegEmbl();
static AjBool       featDelRegGff();
static AjBool       featDelRegPir();
static AjBool       featDelRegSwiss();
static void         featDumpEmbl (AjPFeature thys, AjPStr location,
				 AjPFile file, AjBool IsEmbl);
static void         featDumpGff (AjPFeature thys, AjPFeattable owner,
				AjPFile file);
static void         featDumpPir (AjPFeature thys, AjPStr location,
				AjPFile file);
static void         featDumpSwiss (AjPFeature thys, AjPFile file,
				  AjPFeature gftop);
static AjPFeature   featFeatNew (AjPFeattable thys,
				AjPStr       source,
				AjPStr       type,
				ajint        Start, ajint End,
				float        score,
				char         strand,
				ajint        frame,
				/* AjPStr       desc,*/
				ajint        exon,
				ajint        Start2, ajint End2,
				AjPStr       entryid, AjPStr label,
				ajint        flags );
static AjPFeature   featFeatNewProt (AjPFeattable thys,
				   AjPStr       source,
				   AjPStr       type,
				   ajint        Start,
				   ajint        End,
				   float        score,
				   ajint        flags );
static AjPFeature   featFeatureNew (void);
static AjBool       featFindInFormat (AjPStr format, ajint* iformat);
static AjBool       featFindOutFormat (AjPStr format, ajint* iformat);
static void         featFlagSet (AjPFeature gf, AjPStr flags);
static AjBool       featFormatSet (AjPFeattabIn featin);
static char         featFrame (ajint frame);
static AjBool       featGetUsaSection(AjPStr* tmp, AjPStr token, ajint* begin,
				     ajint* end, AjPStr usa);
static void         featGroupSet (AjPFeature gf, AjPFeattable table,
				 AjPStr grouptag);
static void         featInit (void);
static void         featLocEmblWrapC(AjPStr* pval, ajint width,
				    char* prefix, char* preftyp,
				    AjPStr* retstr);
static AjBool       featoutUfoProcess (AjPFeattabOut thys, AjPStr ufo);
static AjPFeature   featPirFromLine ( AjPFeattable thys,
				     AjPStr origline);
static AjBool       featReadUnknown  ( AjPFeattable thys, AjPFileBuff file) ;
static AjBool       featReadEmbl     ( AjPFeattable thys, AjPFileBuff file) ;
static AjBool       featReadGff      ( AjPFeattable thys, AjPFileBuff file) ;
static AjBool       featReadPir      ( AjPFeattable thys, AjPFileBuff file) ;
static AjBool       featReadSwiss    ( AjPFeattable thys, AjPFileBuff file) ;
static AjBool       featRegInitEmbl();
static AjBool       featRegInitGff();
static AjBool       featRegInitPir();
static AjBool       featRegInitSwiss();
static AjBool       featVocabInitEmbl();
static AjBool       featVocabInitGff();
static AjBool       featVocabInitPir();
static AjBool       featVocabInitSwiss();
static char         featStrand (ajint strand);
static AjPFeature   featSwissFromLine ( AjPFeattable thys, AjPStr line,
					AjPStr* savefeat, AjPStr* savefrom,
					AjPStr* saveto, AjPStr* saveline);
static AjPFeature   featSwissProcess  ( AjPFeattable thys, AjPStr feature,
					AjPStr fromstr, AjPStr tostr,
					AjPStr source,
					AjPStr tags);
static void         featTableInit ( AjPFeattable thys,
				  AjPStr name);
static AjPFeattable featTableNew (void);
static AjPFeattable featTableNewS (AjPStr name);
static AjPStr       featTableTag (AjPStr tag, AjPTable table);
static AjPStr       featTableTagC (char *tag, AjPTable table);
static AjPStr       featTableType (AjPStr type, AjPTable table);
static AjBool       feattableWriteEmbl (AjPFeattable Feattab, AjPFile file,
				       AjBool IsEmbl);
static AjBool       featTagAllLimit(AjPStr* pval, AjPStr values);
static AjPStr       featTagDna (AjPStr type);
static void         featTagEmblDefault(AjPStr* pout, AjPStr tag, AjPStr* pval);
static void         featTagEmblWrapC(AjPStr* pval, ajint width, char* prefix,
				    AjPStr* retstr);
static void         featTagFmt (AjPStr name, AjPTable table, AjPStr* retstr);
static void         featTagGffDefault(AjPStr* pout, AjPStr tag, AjPStr* pval);
static AjBool       featTagGffSpecial(AjPStr* pval, AjPStr tag);
static void         featTagLimit (AjPStr name, AjPTable table, AjPStr* retstr);
static AjPStr       featTagProt (AjPStr type);
static void         featTagQuoteEmbl(AjPStr* pval);
static void         featTagQuoteGff(AjPStr* pval);
static void         featTagSetDefault (AjPFeature thys,
				      AjPStr tag, AjPStr value,
				      AjPStr* pdeftag, AjPStr* pdefval);
static void         featTagSetDefaultDna (AjPStr tag, AjPStr value,
					 AjPStr* pdeftag, AjPStr* pdefval);
static void         featTagSetDefaultProt (AjPStr tag, AjPStr value,
					  AjPStr* pdeftag, AjPStr* pdefval);
static AjBool       featTagSpecial(AjPStr* pval, AjPStr tag);
static AjBool       featTagSpecialAllAnticodon(AjPStr* pval);
static AjBool       featTagSpecialAllCitation(AjPStr* pval);
static AjBool       featTagSpecialAllCodon(AjPStr* pval);
static AjBool       featTagSpecialAllConssplice(AjPStr* pval);
static AjBool       featTagSpecialAllRptunit(AjPStr* pval);
static AjBool       featTagSpecialAllTranslexcept(AjPStr* pval);
static AjBool       featTagSpecialAllDbxref(AjPStr* pval);
static AjBool       featTagSpecialAllProteinid(AjPStr* pval);
static AjBool       featTagSpecialAllReplace(AjPStr* pval);
static AjBool       featTagSpecialAllTranslation(AjPStr* pval);
static void         featTagSwissWrapC(AjPStr* pval, ajint width, char* prefix,
				     AjPStr* retstr);
static FeatPTagval  featTagval ( AjPFeature thys, AjPStr tag);
static FeatPTagval  featTagvalNew ( AjPFeature thys, AjPStr tag, AjPStr value);
static FeatPTagval  featTagvalNewDna ( AjPStr tag, AjPStr value);
static FeatPTagval  featTagvalNewProt ( AjPStr tag, AjPStr value);
static AjPStr       featTypeDna (AjPStr type);
static AjBool       featTypePirIn (AjPStr* type);
static AjBool       featTypePirOut (AjPStr* type);
static AjPStr       featTypeProt (AjPStr type);
static AjBool       featVocabRead (char *name, ajint typsize, ajint tagsize,
				  AjPTable* pTypeTable, AjPTable* pTagsTable);

/* @datastatic FeatPInFormat **************************************************
**
** Featue input format definition
**
** @alias FeatSInFormat
** @alias FeatOInFormat
**
** @attr Name [char*] INput format name
** @attr Dna [AjBool] True if suitable for nucleotide data
** @attr Prot [AjBool] True if suitable for protein data
** @attr Used [AjBool] True if already used (initialised)
** @attr Read [(AjBool*)] Function to read feature data
** @attr InitReg [(AjBool*)] Function to initialise regular expressions
** @attr DelReg [(AjBool*)] Function to clean up regular expressions
** @@
******************************************************************************/

typedef struct FeatSInFormat {
  char* Name;
  AjBool Dna;
  AjBool Prot;
  AjBool Used;
  AjBool (*Read)  (AjPFeattable thys, AjPFileBuff file);
  AjBool (*InitReg)();
  AjBool (*DelReg)();
} FeatOInFormat, *FeatPInFormat;

/* name             Dna   Protein
   input-function   init-regex-function del-regex-function */

/* @funclist featInFormatDef **************************************************
**
** Input feature formats
**
** Includes the read function (featRead), and initialising (featInitReg)
** and deletion (featDelReg) of parsing resular expression.
**
******************************************************************************/

static FeatOInFormat featInFormatDef[] = {
  {"unknown",       AJFALSE, AJFALSE, AJFALSE,
   featReadUnknown, NULL,               NULL},
  {"embl",          AJTRUE,  AJFALSE, AJFALSE,
   featReadEmbl,    featRegInitEmbl,    featDelRegEmbl},
  {"em",            AJTRUE,  AJFALSE, AJFALSE,
   featReadEmbl,    featRegInitEmbl,    featDelRegEmbl},
  {"genbank",       AJTRUE,  AJFALSE, AJFALSE,
   featReadEmbl,    featRegInitEmbl,    featDelRegEmbl},
  {"gb",            AJTRUE,  AJFALSE, AJFALSE,
   featReadEmbl,    featRegInitEmbl,    featDelRegEmbl},
  {"ddbj",          AJTRUE,  AJFALSE, AJFALSE,
   featReadEmbl,    featRegInitEmbl,    featDelRegEmbl},
  {"gff",           AJTRUE,  AJTRUE,  AJFALSE,
   featReadGff,     featRegInitGff,     featDelRegGff},
  {"swiss",         AJFALSE, AJTRUE,  AJFALSE,
   featReadSwiss,   featRegInitSwiss,   featDelRegSwiss},
  {"sw",            AJFALSE, AJTRUE,  AJFALSE,
   featReadSwiss,   featRegInitSwiss,   featDelRegSwiss},
  {"swissprot",     AJFALSE, AJTRUE,  AJFALSE,
   featReadSwiss,   featRegInitSwiss,   featDelRegSwiss},
  {"pir",           AJFALSE, AJTRUE,  AJFALSE,
   featReadPir,     featRegInitPir,     featDelRegPir},
  {"nbrf",           AJFALSE, AJTRUE,  AJFALSE,
   featReadPir,     featRegInitPir,     featDelRegPir},
  {NULL, AJFALSE, AJFALSE, AJFALSE, NULL, NULL, NULL}
};

static FeatPInFormat featInFormat = featInFormatDef;

/* @datastatic FeatPTypePir ***************************************************
**
** PIR feature types related to internal protein features used to
** support internal (swissprot) interconversion with PIR, used where
** the names in Swissprot (internal) and PIR are equivalent but different.
**
** @alias FeatSTypePir
** @alias FeatOTypePir
**
** @attr Pir [char*] PIR database feature name
** @attr Internal [char*] Internal (swissprot) feature name
** @@
******************************************************************************/

typedef struct FeatSTypePir {
  char* Pir;
  char* Internal;
} FeatOTypePir, *FeatPTypePir;

static FeatOTypePir FeatPirType[] = {

  /* internal names for PIR feature types */

  /* Start with these - we take the first match we find */

  {"Active_site",        "act_site"},
  {"Binding_site",       "binding"},
  {"Cleavage_site",      "cleavage_site"},
  {"Cross-link",         "cross-link"},
  {"Disulfide_bonds",    "disulfid"},
  {"Domain",             "domain"},
  {"Inhibitory_site",    "inhibitory_site"},
  {"Modified_site",      "mod_res"},
  {"Product",            "product"},
  {"Region",             "site"},

  /* Closest PIR matches for other internal feature types */

  {"Binding_site",       "ca_bind"},
  {"Binding_site",       "dna_bind"},
  {"Binding_site",       "metal"},
  {"Binding_site",       "np_bind"},
  {"Binding_site",       "zn_fing"},
  {"Modified_site",      "carbohyd"},
  {"Modified_site",      "lipid"},
  {"Modified_site",      "init_met"},
  {"Modified_site",      "se_cys"},
  {"Modified_site",      "thioeth"},
  {"Modified_site",      "thiolest"},
  {"Product",            "chain"},
  {"Product",            "peptide"},
  {"Product",            "propep"},
  {"Region",             "conflict"},
  {"Region",             "helix"},
  {"Region",             "strand"},
  {"Region",             "turn"},
  {"Region",             "mutagen"},
  {"Region",             "non_cons"},
  {"Region",             "non_ter"},
  {"Region",             "repeat"},
  {"Region",             "signal"},
  {"Region",             "transit"},
  {"Region",             "similar"},
  {"Region",             "transmem"},
  {"Region",             "unsure"},
  {"Region",             "variant"},
  {"Region",             "varsplic"},
  {NULL, NULL}
};

static AjBool      ajFeattableWriteUnknown (AjPFeattable features,
					    AjPFile file);

static AjPFeature  featEmblFromLine ( AjPFeattable thys, AjPStr line,
				      AjPStr* savefeat,
				      AjPStr* saveloc, AjPStr* saveline);
static AjPFeature  featEmblProcess  ( AjPFeattable thys, AjPStr feature,
				      AjPStr source,
				      AjPStr* loc, AjPStr* tags);
static AjPFeature  featGffFromLine ( AjPFeattable thys, AjPStr line,
				     float version);

static void        featGffProcessTagval (AjPFeature gf, AjPFeattable table,
					 AjPStr groupfield, float version);

/* @datastatic FeatPOutFormat *************************************************
**
** Feature output formats
**
** @alias FeatSOutFormat
** @alias FeatOOutFormat
**
** @attr Name [char*] Format name
** @attr VocInit [(AjBool*)] Function to initialise vocabulary
** @attr Write [(AjBool*)] Function to write data
** @@
******************************************************************************/

typedef struct FeatSOutFormat {
  char* Name;
  AjBool (*VocInit) ();
  AjBool (*Write) (AjPFeattable thys, AjPFile file);
} FeatOOutFormat, *FeatPOutFormat;

/* @funclist featOutFormatDef *************************************************
**
** Feature output formats
**
** Includes functions to initialise the internal type/tag tables
** (featVocabInit) - done automatically for input formats by the featInitReg
** functions) and to write the output file (ajFeattableWrite)
**
******************************************************************************/

static FeatOOutFormat featOutFormatDef[] = {
  {"unknown",   NULL,               ajFeattableWriteUnknown},
  {"embl",      featVocabInitEmbl,  ajFeattableWriteEmbl},
  {"genbank",   featVocabInitEmbl,  ajFeattableWriteGenbank},
  {"gb",        featVocabInitEmbl,  ajFeattableWriteGenbank},
  {"ddbj",      featVocabInitEmbl,  ajFeattableWriteDdbj},
  {"gff",       featVocabInitGff,   ajFeattableWriteGff},
  {"pir",       featVocabInitPir,   ajFeattableWritePir},
  {"swissprot", featVocabInitSwiss, ajFeattableWriteSwiss},
  {"swiss",     featVocabInitSwiss, ajFeattableWriteSwiss},
  {"sw",        featVocabInitSwiss, ajFeattableWriteSwiss},
  {NULL, NULL, NULL}
};

static FeatPOutFormat featOutFormat = featOutFormatDef;


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Set each of the regular expressions below, depending on feature format */

static AjPRegexp
  GffRegexNumeric   = NULL,
  GffRegexblankline = NULL,
  GffRegexversion   = NULL,
  GffRegexdate      = NULL,
  GffRegexregion    = NULL,
  GffRegexcomment   = NULL,

  GffRegexTvTagval   = NULL,

  EmblRegexNew         = NULL,
  EmblRegexNext        = NULL,
  EmblRegexTv          = NULL,
  EmblRegexTvTag       = NULL,
  EmblRegexTvTagQuote  = NULL,
  EmblRegexTvTagQuote2 = NULL,
  EmblRegexOperIn      = NULL,
  EmblRegexOperOut     = NULL,
  EmblRegexOperNone    = NULL,
  EmblRegexLocMulti    = NULL,
  EmblRegexLoc         = NULL,
  EmblRegexLocRange    = NULL,
  EmblRegexLocNum      = NULL,

  PirRegexAll          = NULL,
  PirRegexCom          = NULL,
  PirRegexLoc          = NULL,
  PirRegexPos          = NULL,

  SwRegexComment       = NULL,
  SwRegexFtid          = NULL,
  SwRegexNew           = NULL,
  SwRegexNext          = NULL,

  DummyRegExec     = NULL
;

/* ==================================================================== */
/* ======================== private methods ========================= */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Feature Object Constructors ***************************************
**
** All constructors return a new feature or feature table set by pointer.
** It is the responsibility of the user to first destroy any previous feature.
** The target pointer does not need to be initialised to NULL, but it is good
** programming practice to do so anyway.
**
** Generally, however, the  'table' class object for AjPFeatures must first
** be created, prior to creating any 'AjPFeature' objects.
**
** To replace or reuse an existing feature object see instead
** the {Feature Assignments} and {Feature Modifiers} methods.
**
******************************************************************************/

/* @func ajFeattabOutOpen *****************************************************
**
** Processes the specified UFO, and opens the resulting output file.
**
** @param [r] thys [AjPFeattabOut] Features table output object
** @param [r] ufo [AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutOpen (AjPFeattabOut thys, AjPStr ufo) {

  ajDebug("ajFeattabOutOpen UFO '%S'\n", ufo);
  if (thys->Handle)
    return ajTrue;

  if (!ajStrLen(thys->Filename)) {
    if (!featoutUfoProcess (thys, ufo))
      return ajFalse;
  }

  ajDebug("trying to open '%S'\n", thys->Filename);
  thys->Handle = ajFileNewOut(thys->Filename);
  if (!thys->Handle) return ajFalse;
  ajDebug("after opening '%S'\n", thys->Filename);

  return ajTrue;
}

/* @func ajFeattabOutFile *****************************************************
**
** Returns the name of a feature output file
**
** @param [r] thys [AjPFeattabOut] Features table output object
** @return [AjPFile] File object
** @@
******************************************************************************/

AjPFile ajFeattabOutFile (AjPFeattabOut thys) {

  ajDebug("ajFeattabOutFile\n");
  return thys->Handle;
}

/* @func ajFeattabOutFilename *************************************************
**
** Returns the name of a feature output file
**
** @param [r] thys [AjPFeattabOut] Features table output object
** @return [AjPStr] Filename
** @@
******************************************************************************/

AjPStr ajFeattabOutFilename (AjPFeattabOut thys) {

  ajDebug("ajFeattabOutFilename\n");
  if (ajStrLen(thys->Filename))
    return thys->Filename;

  return NULL;
}

/* @func ajFeattabOutIsOpen ***************************************************
**
** Checks whether feature output file has already been opened
**
** @param [r] thys [AjPFeattabOut] Features table output object
** @return [AjBool] ajTrue if file is open
** @@
******************************************************************************/

AjBool ajFeattabOutIsOpen (AjPFeattabOut thys) {

  ajDebug("ajFeattabOutIsOpen\n");
  if (thys->Handle)
    return ajTrue;

  return ajFalse;
}

/* @func ajFeattabOutSet ******************************************************
**
** Processes the specified UFO, and specifies the resulting output file.
**
** @param [r] thys [AjPFeattabOut] Features table output object
** @param [r] ufo [AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattabOutSet (AjPFeattabOut thys, AjPStr ufo) {

  ajDebug("ajFeattabOutSet UFO '%S'\n", ufo);
  if (thys->Handle)
    return ajTrue;

  if (!featoutUfoProcess (thys, ufo))
    return ajFalse;

  return ajTrue;
}

/* @func ajFeattabInNew *******************************************************
**
** Constructor for an empty feature table input object
**
** @return [AjPFeattabIn] Feature table input object
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNew (void) {
  AjPFeattabIn pthis;
  AJNEW0(pthis);

  ajDebug("ajFeatTabInNew %x\n", pthis);

  return pthis;
}

/* @func ajFeattabInNewSS *****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read.
**
** @param [r] fmt [AjPStr] feature format
** @param [r] name [AjPStr] sequence name
** @param [r] type [char*] feature type
** @return [AjPFeattabIn] Feature table input object
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNewSS (AjPStr fmt, AjPStr name, char* type) {
  AjPFeattabIn pthis;
  ajint iformat = 0;

  if (!featFindInFormat(fmt, &iformat)) return NULL;

  pthis = ajFeattabInNew ();
  ajStrAssC (&pthis->Formatstr, featInFormat[pthis->Format].Name);
  pthis->Format = iformat;
  ajStrAssC (&pthis->Type, type);
  ajStrAssS (&pthis->Seqname, name);
  pthis->Handle = ajFileBuffNew();

  ajDebug("ajFeatTabInNewSSF %x Handle %x\n", pthis, pthis->Handle);

  return pthis;
}

/* @func ajFeattabInNewSSF ****************************************************
**
** Constructor for an empty feature table input object. The format and
** name are read. The file buffer is moved to the feature table input
** object and should not be deleted by the calling program.
**
** @param [r] fmt [AjPStr] feature format
** @param [r] name [AjPStr] sequence name
** @param [r] type [char*] feature type
** @param [r] buff [AjPFileBuff] Buffer containing feature data
** @return [AjPFeattabIn] Feature table input object
** @@
******************************************************************************/

AjPFeattabIn ajFeattabInNewSSF (AjPStr fmt, AjPStr name, char* type,
			       AjPFileBuff buff) {
  AjPFeattabIn pthis;
  ajint iformat = 0;

  if (!featFindInFormat(fmt, &iformat)) return NULL;

  pthis = ajFeattabInNew ();
  ajStrAssC (&pthis->Formatstr, featInFormat[iformat].Name);
  pthis->Format = iformat;
  ajStrAssC (&pthis->Type, type);
  ajStrAssS (&pthis->Seqname, name);
  pthis->Handle = buff;

  ajDebug("ajFeatTabInNewSSF %x Handle %x\n", pthis, buff);

  return pthis;
}

/* @func ajFeattabOutNew ******************************************************
**
** Constructor for an empty feature table output object
**
** @return [AjPFeattabOut] Feature table input object
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNew (void) {
  AjPFeattabOut pthis;
  AJNEW0(pthis);

  ajDebug("ajFeatTabOutNew %x\n", pthis);

  return pthis;
}

/* @func ajFeattabOutNewSSF ***************************************************
**
** Constructor for an empty feature table output object
**
** @param [r] fmt [AjPStr] feature format
** @param [r] name [AjPStr] sequence name
** @param [r] type [char*] feature type
** @param [r] file [AjPFile] Output file
** @return [AjPFeattabOut] Feature table output object
** @@
******************************************************************************/

AjPFeattabOut ajFeattabOutNewSSF (AjPStr fmt, AjPStr name, char* type,
			       AjPFile file) {
  AjPFeattabOut pthis;
  ajint iformat = 0;

  if (!featFindOutFormat(fmt, &iformat)) return NULL;

  pthis = ajFeattabOutNew ();
  ajStrAssC (&pthis->Formatstr, featOutFormat[iformat].Name);
  pthis->Format = iformat;
  switch (*type) {
  case 'p':
  case 'P':
    ajStrAssC (&pthis->Type, "P");
    break;
  default:
    ajStrAssC (&pthis->Type, "N");
    break;
  }

  ajStrAssS (&pthis->Seqname, name);
  pthis->Handle = file;

  ajDebug("ajFeatTabOutNewSSF %x\n", pthis);

  return pthis;
}

/* @func ajFeatRead ***********************************************************
**
** Generic interface function for reading in features from a file
** given the file handle, class of map, data format of input
** and possibly other associated data.
**
** @param  [rC] ftin   [AjPFeattabIn]  Specifies the external source (file)
**                                     of the features to be read in
** @return [AjPFeattable] Pointer to a new feature table containing
** the features read in
** @cre 'file' argument must be a valid AjPFile
** @@
**
** Version 1.0, 7/6/99 ACD to ajfeat access function (reading features)
**
******************************************************************************/

AjPFeattable ajFeatRead  ( AjPFeattabIn  ftin )
{
   AjPFileBuff   file ;
   ajint           format ;
   AjPFeattable features = NULL ;
   AjBool       result   = ajFalse ;

   assert (ftin);

   file     = ftin->Handle ;
   assert (file);

   format   = ftin->Format ;

   if (!format)
     return NULL;

   ajDebug ("ajFeatRead format %d '%s' file %x\n",
	    format, featInFormat[format].Name, file);

   if(!featInFormat[format].Used) {
     /* Calling funclist featInFormatDef() */
     if(!featInFormat[format].InitReg()) {
       ajDebug("Initialisation failed for %s\n",featInFormat[format].Name);
       ajErr ("Initialisation failed for feature format %s",
	      featInFormat[format].Name);
     }
     featInFormat[format].Used = ajTrue;
   }

   features = ajFeattableNew (ftin->Seqname);

   /* Calling funclist featInFormatDef() */
   result = featInFormat[format].Read(features, file);

   if(result) {
     ajFeattableTrace (features);
      return features ;
   } else {
      ajFeattableDel(&(features)) ;
   }
   return NULL;
}

/* @func ajFeatNew ************************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [rC]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         owns the feature
** @param  [rENU] source   [AjPStr]      Analysis basis for feature
** @param  [rENU] type     [AjPStr]      Type of feature (e.g. exon)
** @param  [rNU]  Start    [ajint]  Start position of the feature
** @param  [rNU]  End      [ajint]  End position of the feature
** @param  [rENU] score    [float]      Analysis score for the feature
** @param  [rNU]  strand   [char]  Strand of the feature
** @param  [rNU]  frame    [ajint]   Frame of the feature
** @return [AjPFeature] newly allocated feature object
** @@
******************************************************************************/

AjPFeature ajFeatNew (AjPFeattable thys,
		      AjPStr       source,
		      AjPStr       type,
		      ajint Start, ajint End,
		      float        score,
		      char         strand,
		      ajint        frame
		      )
{
  ajint flags = 0;
  AjPFeature ret = NULL ;

  ret = featFeatNew(thys,source,type,Start,End,score,strand,frame,
		    0,0,0,NULL, NULL,flags);

  return ret;
}

/* @func ajFeatNewII **********************************************************
**
** Simple constructor with only start and end positions
**
** User must specify associated 'ajFeattable' to which the new feature
** is automatically added!
**
** @param  [rC]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         owns the feature
** @param  [rNU]  Start    [ajint]  Start position of the feature
** @param  [rNU]  End      [ajint]  End position of the feature
** @return [AjPFeature] newly allocated feature object
** @@
******************************************************************************/

AjPFeature ajFeatNewII (AjPFeattable thys,
		      ajint Start, ajint End)
{
  static AjPStr source = NULL;
  static AjPStr type = NULL;
  static float score=0.0;
  static char strand='.';
  static ajint frame=0;
  static ajint flags = 0;

  AjPFeature ret = NULL ;

  if (!type)
    type = ajStrNewC("misc_feature");

  ret = featFeatNew(thys,source,type,Start,End,score,strand,frame,
		    0,0,0,NULL, NULL,flags);

  return ret;
}

/* @func ajFeatNewProt ********************************************************
**
** Constructor - must specify associated 'ajFeattable'
**               to which the new feature is automatically added!
**
** @param  [rC]   thys    [AjPFeattable] Pointer to the ajFeattable which
**                         owns the feature
** @param  [rENU] source   [AjPStr]      Analysis basis for feature
** @param  [rENU] type     [AjPStr]      Type of feature (e.g. exon)
** @param  [rNU]  Start    [ajint]  Start position of the feature
** @param  [rNU]  End      [ajint]  End position of the feature
** @param  [rENU] score    [float]      Analysis score for the feature
** @return [AjPFeature] newly allocated feature object
** @cre 'table' argument must be a valid ajFeattable
** @exception 'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeature ajFeatNewProt (AjPFeattable thys,
			  AjPStr       source,
			  AjPStr       type,
			  ajint Start, ajint End,
			  float        score
			  )
{
  ajint flags = 0;
  AjPFeature ret = NULL ;

  ret = featFeatNewProt(thys,source,type,Start,End,score,flags);

  return ret;
}

/* @funcstatic featCompByStart ************************************************
**
** Compare two features by their start.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByStart(const void *a, const void *b) {

  AjPFeature gfa = *(AjPFeature *) a;
  AjPFeature gfb = *(AjPFeature *) b;

  ajint val=0;

  val = gfa->Start - gfb->Start;
  if(val){
    return val;
  }
  else{
    val = gfb->End - gfa->End;
    if(val)
      return val;
    else
      return 0;
  }
}
/* @funcstatic featCompByEnd **************************************************
**
** Compare two features by their end.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByEnd(const void *a, const void *b) {

  AjPFeature gfa = *(AjPFeature *) a;
  AjPFeature gfb = *(AjPFeature *) b;

  ajint val=0;

  val = gfa->End - gfb->End;
  if(val){
    return val;
  }
  else{
    val = gfa->Start - gfb->Start;
    if(val)
      return val;
    else
      return 0;
  }
}
/* @funcstatic featCompByGroup ************************************************
**
** Compare two features by their group and exon numbers
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByGroup(const void *a, const void *b) {

  AjPFeature gfa = *(AjPFeature *) a;
  AjPFeature gfb = *(AjPFeature *) b;

  ajint val=0;

  val = gfa->Group - gfb->Group;
  if(val){
    return val;
  }
  else{
    val = gfa->Exon - gfb->Exon;
      return val;
  }
}

/* @funcstatic featCompByType *************************************************
**
** Compare two features by their type (key).
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
**
** @return [ajint] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/

static ajint featCompByType(const void *a, const void *b) {

  AjPFeature gfa = *(AjPFeature *) a;
  AjPFeature gfb = *(AjPFeature *) b;

  ajint val=0;

  val = ajStrCmp(&gfa->Type,&gfb->Type);
  if(val){
    return val;
  }
  else{
    val = gfa->Start - gfb->Start;
    if(val){
      return val;
    }
    else{
      val = gfa->End - gfb->End;
      if(val){
	return val;
      }
      else
	return 0;
    }

  }
}

/******************************************************************************
**
** Utility classes...
**
******************************************************************************/


/* @funcstatic featFeatNew ****************************************************
**
** Constructor for a new DNA feature,
** automatically added to the specififed table.
**
** @param  [rC]   thys     [AjPFeattable] Pointer to the ajFeattable which
**                         owns the feature
** @param  [rENU] source   [AjPStr]      Analysis basis for feature
** @param  [rENU] type     [AjPStr]      Type of feature (e.g. exon)
** @param  [rNU]  Start    [ajint]  Start position of the feature
** @param  [rNU]  End      [ajint]  End position of the feature
** @param  [rENU] score    [float]      Analysis score for the feature
** @param  [rNU]  strand   [char]  Strand of the feature
** @param  [rNU]  frame    [ajint]   Frame of the feature
** @param  [rNU]  exon     [ajint]  exon number (0 for default value)
** @param  [rNU]  Start2   [ajint]  2nd Start position of the feature
** @param  [rNU]  End2     [ajint]  2nd End position of the feature
** @param  [rENU] entryid  [AjPStr] Entry ID for location in another entry
** @param  [rENU] label    [AjPStr] Label for location (non-numeric)
** @param  [rNU]  flags    [ajint]  flags.
** @return [AjPFeature] newly allocated feature object
** @@
******************************************************************************/

static AjPFeature featFeatNew (AjPFeattable thys,
				  AjPStr       source,
				  AjPStr       type,
				  ajint        Start,
				  ajint        End,
				  float        score,
				  char         strand,
				  ajint        frame,
				  ajint        exon,
				  ajint        Start2,
				  ajint        End2,
				  AjPStr       entryid,
				  AjPStr       label,
				  ajint        flags ) {
  AjPFeature ret = NULL ;
  static ajint maxexon = 0;
  static AjPStr defsource=NULL;

  if (!defsource)
    ajAcdProgramS (&defsource);

  ajDebug ("\nfeatFeatNew '%S' %d .. %d %x\n", type, Start, End, flags);

  if(!ajStrLen(type)){
    return NULL;
  }

  featInit();

  /* Allocate the object... */

  ret = featFeatureNew();

  if(flags & FEATFLAG_CHILD){
    ret->Group = thys->Groups;
    if (exon)
      ret->Exon  = exon;
    else
      ret->Exon = ++maxexon;
  }
  else{
    thys->Groups++;
    ret->Group = thys->Groups;
    ret->Exon = 0;
  }

  if (ajStrLen(source))
    ajStrAssS (&ret->Source, source);
  else
    ajStrAssS (&ret->Source, defsource);

  ajStrAssS (&ret->Type, featTypeDna(type));

  ret->Score = score;

  ret->Flags = flags;

  ret->Strand = strand ;

  ret->Frame  = frame ;
  ret->Start = Start;
  ret->End = End;
  ret->Start2 = Start2;
  ret->End2 = End2;

  if (ajStrLen(entryid))
    ajStrAssS (&ret->Remote, entryid);
  else {
    thys->Len = AJMAX (thys->Len, ret->Start);
    thys->Len = AJMAX (thys->Len, ret->End);
  }

  if (ajStrLen(label))
    ajStrAssS (&ret->Label, label);

  ajFeattableAdd(thys,ret) ;

  return ret ;
}

/* @funcstatic featFeatNewProt ************************************************
**
** Constructor for a new protein feature,
** automatically added to the specififed table.
**
** @param  [rC]   thys     [AjPFeattable] Pointer to the ajFeattable which
**                         owns the feature
** @param  [rENU] source   [AjPStr]      Analysis basis for feature
** @param  [rENU] type     [AjPStr]      Type of feature (e.g. exon)
** @param  [rNU]  Start    [ajint]  Start position of the feature
** @param  [rNU]  End      [ajint]  End position of the feature
** @param  [rENU] score    [float]      Analysis score for the feature
** @param  [rNU]  flags    [ajint]  flags.
** @return [AjPFeature] newly allocated feature object
** @@
**
******************************************************************************/

static AjPFeature featFeatNewProt (AjPFeattable thys,
				   AjPStr       source,
				   AjPStr       type,
				   ajint        Start,
				   ajint        End,
				   float        score,
				   ajint        flags ) {
  AjPFeature ret = NULL ;
  static ajint maxexon = 0;
  static AjPStr defsource=NULL;

  if (!defsource)
    ajAcdProgramS (&defsource);

  ajDebug ("\nfeatFeatNew '%S' %d .. %d %x\n", type, Start, End, flags);

  if(!ajStrLen(type)){
    return NULL;
  }

  featInit();

  /* Allocate the object... and a new Tags list */
  ret = featFeatureNew() ;

  if(flags & FEATFLAG_CHILD){
    ret->Group = thys->Groups;
    ret->Exon = ++maxexon;
  }
  else{
    thys->Groups++;
    ret->Group = thys->Groups;
    ret->Exon = 0;
  }

  if (ajStrLen(source))
    ajStrAssS (&ret->Source, source);
  else
    ajStrAssS (&ret->Source, defsource);

  ajStrAssS (&ret->Type, featTypeProt(type));

  ret->Score = score;

  ret->Flags = flags;

  ret->Strand = '\0' ;

  ret->Frame  = 0 ;
  ret->Start = Start;
  ret->End = End;
  ret->Start2 = 0;
  ret->End2 = 0;

  ret->Protein = ajTrue;

  thys->Len = AJMAX (thys->Len, ret->Start);
  thys->Len = AJMAX (thys->Len, ret->End);

  ajFeattableAdd(thys,ret) ;

  return ret ;
}


/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section Feature Object Destructors ****************************************
**
** (Simple minded) object destruction by release of memory.
**
** No reference counting (for now).
**
******************************************************************************/

/* @func ajFeattabInDel *******************************************************
**
** Destructor for a feature table input object
**
** @param [d] pthis [AjPFeattabIn*] Feature table input object
** @return [void]
** @@
******************************************************************************/

void ajFeattabInDel (AjPFeattabIn* pthis) {

  AjPFeattabIn thys = *pthis;

  if (!thys) return;

  ajDebug ("ajFeattabInDel %x Handle %x\n", thys, thys->Handle);

  ajFileBuffDel(&thys->Handle);
  ajStrDel(&thys->Ufo);
  ajStrDel(&thys->Formatstr);
  ajStrDel(&thys->Filename);
  ajStrDel(&thys->Entryname);
  ajStrDel(&thys->Seqname);
  ajStrDel(&thys->Type);
  AJFREE(*pthis);
  return;
}

/* @func ajFeattableDel *******************************************************
**
** Destructor for ajFeattable objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** @param  [wP] pthis [AjPFeattable*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFeattableDel(AjPFeattable *pthis)
{
  AjPFeattable thys;
  if (!pthis) return ;

  thys = *pthis;

  ajDebug ("ajFeattableDel %x\n", thys);

  if (!thys) return ;

  ajFeattableClear(thys) ;

  ajStrDel (&thys->Type);
  ajListDel(&thys->Features) ;

  AJFREE (*pthis) ; /* free the object */
  *pthis = NULL ;

  return;
}

/* @func ajFeatDel ************************************************************
**
** Destructor for AjPFeature objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** @param  [wPC] pthis [AjPFeature*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFeatDel(AjPFeature *pthis) {
  if (!pthis) return ;
  if (!*pthis) return ;

  featClear(*pthis) ;

  AJFREE (*pthis) ; /* free the object */
  *pthis = NULL ;

  return;
}

/* @funcstatic featClear ******************************************************
**
** Deletes all feature tag-value pairs from a feature line
**
** @param [r] thys [AjPFeature] Feature
** @return [void]
** @@
******************************************************************************/

static void featClear ( AjPFeature thys ) {
  AjIList        iter = NULL ;
  FeatPTagval    item = NULL ;

  if (!thys) return ;

  /* We need to delete the associated Tag data structures too!!!*/

  if (thys->Tags) {
    iter = ajListIter(thys->Tags) ;
    while(ajListIterMore(iter))
    {
      item = (FeatPTagval)ajListIterNext (iter) ;
      /* assuming a simple block memory free for now...*/
      ajStrDel(&item->Value) ;
      ajStrDel(&item->Tag) ;
      AJFREE(item);
      ajListRemove(iter) ;
    }
    ajListIterFree(iter) ;

  }
  ajListFree(&(thys->Tags));
  ajListDel(&(thys->Tags)) ;

  ajStrDel(&thys->Source);
  ajStrDel(&thys->Type);
  ajStrDel(&thys->Remote);
  ajStrDel(&thys->Label);

}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Feature Assignments ***********************************************
**
** This category of class methods have three forms:
**
** 1. Copy constructors ("aj*Copy()" methods): assignments overwrite
**    the target 'Feature' object provided as the first argument by
**    calling the appropriate {Feature destructors} first.  A NULL
**    value is always acceptable so these methods are often used to
**    create new objects by assignment. (As in all object
**    construction), the caller of the function is responsible for
**    calling the respective object constructor.
**
** 2. Initializers ("aj*Init()" methods): assume an empty object of
**    the specified type and dereferenced by the object pointer
**    provided as the first argument, is to be is to be initialized in
**    two ways: first, by initialization of any 'base class' data (by
**    recursive calls to 'super' class initializers) and second, for
**    the current object class, using arguments provided (if
**    any). These methods are *automatically* called by {Feature
**    constructors} for dynamically created objects, but can (and
**    should) be called explicitly for statically defined objects.
**
******************************************************************************/

/* @func ajFeattableAdd *******************************************************
**
** Method to add a new AjPFeature to a AjPFeattable
**
** @param  [rC] thys    [AjPFeattable] The feature table
** @cre 'thys' must be a valid AjPFeattable
** @param  [rC] feature [AjPFeature]        Feature to be added to the set
** @cre 'feature' must be a valid AjPFeature (non-null)
** @return [void]
** @exception  'Null_Feature_Object' or 'Not_a_Subclass' if 'thys' is invalid
** @exception  'Null_Feature_Object' if null feature argument seen
** @@
**
******************************************************************************/

void ajFeattableAdd ( AjPFeattable thys, AjPFeature feature )
{
  thys->Len = AJMAX (thys->Len, feature->Start);
  thys->Len = AJMAX (thys->Len, feature->End);
  ajListPushApp ( thys->Features, feature);  ;

  if(feature->Type)
    ajDebug ("ajFeattableAdd list size %d '%S' %d %d\n",
	     ajListLength(thys->Features), feature->Type,
	     feature->Start, feature->End);
  else
    ajDebug ("ajFeattableAdd list size %d '%S' %d %d\n",
	     ajListLength(thys->Features), NULL,
	     feature->Start, feature->End);

  return;
}


/* @func ajFeatUfoRead ********************************************************
**
** Parses a UFO, opens an input file, and reads a feature table
**
** @param [w] pthis [AjPFeattable*] Feature table created, (or NULL if failed)
** @param [r] featin [AjPFeattabIn] Feature input object
** @param [r] ufo [AjPStr] UFO feature spec
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatUfoRead (AjPFeattable* pthis, AjPFeattabIn featin, AjPStr ufo) {
  static AjPRegexp fmtexp = NULL;
  static AjPRegexp filexp = NULL;
  static AjPStr ufotest = NULL;

  AjBool fmtstat = ajFalse;	/* status returns from regex tests */
  AjBool filstat = ajFalse;	/* status returns from regex tests */
  AjBool ret = ajFalse;
  ajint i;

  if (!fmtexp)
    fmtexp = ajRegCompC ("^([A-Za-z0-9]+):+(.*)$");
				/* \1 format */
				/* \2 remainder */

  if (!filexp)
    filexp = ajRegCompC ("^([^:]+)$");

  ajDebug("ajFeatUfoRead UFO '%S'\n", ufo);

  (void) ajStrAssS (&ufotest, ufo);

  if (ajStrLen(ufo)) {
    fmtstat = ajRegExec (fmtexp, ufotest);
    ajDebug("feat format regexp: %B\n", fmtstat);
  }

  if (fmtstat) {
    ajRegSubI (fmtexp, 1, &featin->Formatstr);
    (void) ajStrSetC (&featin->Formatstr, featInFormat[0].Name); /* unknown */
    ajRegSubI (fmtexp, 2, &ufotest); /* trim off the format */
    ajDebug ("found feat format %S\n", featin->Formatstr);

    if (!featFindInFormat (featin->Formatstr, &featin->Format))
      ajErr ("unknown input feature table format '%S'\n"
	     " NO Features will be read in", featin->Formatstr);
  }
  else {
    ajDebug ("no format specified in UFO");
  }

  featFormatSet (featin);

  filstat = ajRegExec (filexp, ufotest);
  ajDebug("filexp: %B\n", filstat);
  if (filstat) {
    ajRegSubI (filexp, 1, &featin->Filename);
  }
  else {
    if (ajStrLen(featin->Seqname) && ajStrLen(featin->Formatstr)) {
      (void) ajFmtPrintS(&ufotest, "%S.%S",
			 featin->Seqname, featin->Formatstr);
      (void) ajStrSet (&featin->Filename, ufotest);
      ajDebug ("generate filename  '%S'\n", featin->Filename);
    }
    else {
      ajDebug ("unable to generate filename "
	       "Featin Seqname '%S' Formatstr '%S'\n",
	       featin->Seqname, featin->Formatstr);
      return ajFalse;
    }
  }

  /* Open the file so that we can try to read it */

  ajDebug("trying to open '%S'\n", featin->Filename);
  featin->Handle = ajFileBuffNewIn (featin->Filename);
  if (!featin->Handle) return ajFalse;
  ajDebug("after opening '%S'\n", featin->Filename);


 /* OKAY if we have a format specified try this and this ONLY */
  if(featin->Format){
    *pthis = ajFeatRead(featin);
  }
  /* else loop through all types and try to read features */
  else {
    for(i=1;featInFormat[i].Name;i++){
      featin->Format = i;

      *pthis = ajFeatRead(featin);

      /* Reset buffer to start */
      ajFileBuffReset(featin->Handle);

      if(*pthis){
	ajFileBuffDel(&featin->Handle);
	return ajTrue;
      }
    }
  }
  if (!*pthis)
    ret = ajFalse;
  else
    ret = ajTrue;

  ajFileBuffDel(&featin->Handle);

  return ret;
}

/* @funcstatic featFormatSet **************************************************
**
** Sets the input format for a feature table using the feature input
** object's defined format.
**
** @param [r] featin [AjPFeattabIn] Feature table input.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFormatSet (AjPFeattabIn featin)
{

    if (ajStrLen(featin->Formatstr))
    {
	ajDebug ("... input format value '%S'\n", featin->Formatstr);
	if (featFindInFormat (featin->Formatstr, &featin->Format))
	{

	    /* we may need to set feature table format too? */

	    /*
	    (void) ajStrAss (&thys->Formatstr, featin->Formatstr);
	    thys->Format = featin->Format;
	    ajDebug ("...format OK '%S' = %d\n", featin->Formatstr,
		     featin->Format);
	    */
	}
	else
	{
	    ajDebug ("...format unknown '%S'\n", featin->Formatstr);
	}
	return ajTrue;
    }
    else
    {
	ajDebug ("...input format not set\n");
    }

    return ajFalse;
}
/* @func ajFeatUfoWrite *******************************************************
**
** Parses a UFO, opens an output file, and writes a feature table to it.
**
** @param [w] thys [AjPFeattable] Feature table created
** @param [r] featout [AjPFeattabOut] Feature output object
** @param [r] ufo [AjPStr] UFO feature spec (ignored)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatUfoWrite (AjPFeattable thys, AjPFeattabOut featout, AjPStr ufo) {

  ajFeattabOutOpen (featout, ufo);

  return ajFeatWrite(featout, thys);
}

/* @func ajFeattableWrite *****************************************************
**
** Parses a UFO, opens an output file, and writes a feature table to it.
**
** @param [w] thys [AjPFeattable] Feature table created
** @param [r] ufo [AjPStr] UFO feature spec (ignored)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeattableWrite (AjPFeattable thys, AjPStr ufo) {
  AjPFeattabOut tabout=NULL;

  tabout= ajFeattabOutNew();
  featoutUfoProcess (tabout, ufo);

  return ajFeatWrite(tabout, thys);
}

/* @func ajFeatSortByType *****************************************************
**
** Sort Feature table by Type.
**
** @param [rw] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
** @@
******************************************************************************/
void ajFeatSortByType(AjPFeattable Feattab){
  ajListSort(Feattab->Features,*featCompByType);
}

/* @func ajFeatSortByStart ****************************************************
**
** Sort Feature table by Start position.
**
** @param [rw] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
** @@
******************************************************************************/
void ajFeatSortByStart(AjPFeattable Feattab){
  ajListSort(Feattab->Features,*featCompByStart);
}

/* @func ajFeatSortByEnd ******************************************************
**
** Sort Feature table by End position.
**
** @param [rw] Feattab [AjPFeattable] Feature table to be sorted.
**
** @return [void]
** @@
******************************************************************************/
void ajFeatSortByEnd(AjPFeattable Feattab){
  ajListSort(Feattab->Features,*featCompByEnd);
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Feature Object Operators ******************************************
**
** These functions use the contents of a feature object,
** but do not make any changes.
**
******************************************************************************/

/* @funcstatic featTableInit **************************************************
**
** Initialize the components of a previously allocated AjPFeattable object.
**
** @param [uC]   thys       [AjPFeattable]   Target feature table object
** @param [rNE]  name       [AjPStr]         Name of the table (e.g.
**                                           sequence name)
** @return [void]
** @cre 'thys' must be non-NULL and pointing to an AjPFeature (or subclass
**      thereof) object
**
** @@
**
******************************************************************************/

static void featTableInit ( AjPFeattable thys,
                          AjPStr name) {
  ajDebug ("featTableInit Entering...\n");

  ajDebug ("featTableInit initializing name: '%S'\n", name);
  (void) ajStrAssS(&thys->Name,name) ;
  thys->DefFormat = 0;

  return;
}

/* @func ajFeattableClear *****************************************************
**
** Clears a feature table of all features
**
** @param [r] thys [AjPFeattable] Feature table
** @return [void]
** @@
******************************************************************************/

void ajFeattableClear ( AjPFeattable thys )
{
  AjIList iter       = NULL ;
  AjPFeature feature = NULL ;

  if (!thys) return ;

  /* Format and Version are simple variables, non-allocated...*/
  /* Don't worry about the Date... probably static...*/

  ajStrDel(&thys->Name);

  /* I traverse the list of features and delete them first... */
  if (thys->Features) {
    iter = ajListIter(thys->Features) ;
    while(ajListIterMore(iter)) {
      feature = (AjPFeature)ajListIterNext (iter) ;
      ajFeatDel(&feature) ;
      ajListRemove(iter) ;
    }
    ajListIterFree(iter) ;
  }

  /* No - we need the list to exist by to be empty */
  /* ajListDel(&thys->Features) ; */

  return;
}

/* @funcstatic featoutUfoProcess **********************************************
**
** Converts a UFO Uniform Feature Object into an open output file.
**
** First tests for "format::" and sets thys if it is found
**
** @param [u] thys [AjPFeattabOut] Feature table to be written.
** @param [u] ufo [AjPStr] UFO.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featoutUfoProcess (AjPFeattabOut thys, AjPStr ufo) {

  static AjPRegexp fmtexp = NULL;
  static AjPRegexp filexp = NULL;

  static AjPStr ufotest = NULL;

  AjBool fmtstat = ajFalse;	/* status returns from regex tests */
  AjBool filstat = ajFalse;	/* status returns from regex tests */
  AjPFeattabOut featout = thys;

  if (!fmtexp)
    fmtexp = ajRegCompC ("^([A-Za-z0-9]*):+(.*)$");
				/* \1 format */
				/* \2 remainder */
  if (!filexp)
    filexp = ajRegCompC ("^([^:]+)$");

  ajDebug("featoutUfoProcess UFO '%S'\n", ufo);

  (void) ajStrAssS (&ufotest, ufo);

  if (ajStrLen(ufo)) {
    fmtstat = ajRegExec (fmtexp, ufotest);
    ajDebug("feat format regexp: %B\n", fmtstat);
  }

  if (fmtstat) {
    ajRegSubI (fmtexp, 1, &featout->Formatstr);
    (void) ajStrSetC (&featout->Formatstr,
		      featOutFormat[0].Name); /* unknown */
    ajRegSubI (fmtexp, 2, &ufotest); /* trim off the format */
    ajDebug ("found feat format %S\n", featout->Formatstr);
   }
  else {
    ajDebug("no feat format specified in UFO '%S' try '%S' or 'gff'\n",
	    ufo, featout->Formatstr);
    (void) ajStrSetC(&featout->Formatstr, "gff");
  }

  if (!featFindOutFormat (featout->Formatstr, &featout->Format)){
      ajErr ("unknown output feature format '%S' will write as gff instead\n",
	     featout->Formatstr );
  }
  /* now go for the filename */

  filstat = ajRegExec (filexp, ufotest);
  ajDebug("filexp: %B\n", filstat);
  if (filstat) {
    ajRegSubI (filexp, 1, &featout->Filename);
  }
  else {
    if (ajStrLen(featout->Seqname))
      (void) ajFmtPrintS(&ufotest, "%S.%S", featout->Seqname,
			 featout->Formatstr);
    else
      (void) ajFmtPrintS(&ufotest, "unknown.%S", featout->Formatstr);

    (void) ajStrSet (&featout->Filename, ufotest);
    ajDebug ("generate filename  '%S'\n", featout->Filename);
  }

  ajDebug ("\n");

  return ajTrue;
}

/* @funcstatic featFindInFormat ***********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [P] format [AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindInFormat (AjPStr format, ajint* iformat) {

  static AjPStr tmpformat = NULL;
  ajint i = 0;

  ajDebug ("featFindInFormat '%S'\n", format);
  if (!ajStrLen(format))
    return ajFalse;

  (void) ajStrAssS (&tmpformat, format);
  (void) ajStrToLower(&tmpformat);
  for (i=0; featInFormat[i].Name; i++) {
    ajDebug ("test %d '%s' \n", i, featInFormat[i].Name);
    if (!ajStrNCmpC(tmpformat, featInFormat[i].Name, ajStrLen(tmpformat) )) {
      *iformat = i;
      (void) ajStrDelReuse(&tmpformat);
      ajDebug ("found '%s' at %d\n", featInFormat[i].Name, i);
      return ajTrue;
    }
  }

  ajErr ("Unknown input feat format '%S'", format);

  (void) ajStrDelReuse(&tmpformat);
  return ajFalse;
}


/* @funcstatic featFindOutFormat **********************************************
**
** Looks for the specified format(s) in the internal definitions and
** returns the index.
**
** Given a single format, sets iformat.
**
** @param [P] format [AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindOutFormat (AjPStr format, ajint* iformat) {

  static AjPStr tmpformat = NULL;
  ajint i = 0;

  ajDebug ("featFindOutFormat '%S'\n", format);
  if (!ajStrLen(format))
    return ajFalse;

  (void) ajStrAssS (&tmpformat, format);
  (void) ajStrToLower(&tmpformat);
  for (i=0; featOutFormat[i].Name; i++) {
    ajDebug ("test %d '%s' len=%d\n",
	     i, featOutFormat[i].Name,ajStrLen(tmpformat));
    if (!ajStrNCmpC(tmpformat, featOutFormat[i].Name,ajStrLen(tmpformat))) {
      *iformat = i;
      (void) ajStrDelReuse(&tmpformat);
      ajDebug ("found '%s' at %d\n", featOutFormat[i].Name, i);
      return ajTrue;
    }
  }

  /*  ajErr ("Unknown output feat format '%S'", format);*/

  (void) ajStrDelReuse(&tmpformat);
  *iformat = 1;
  return ajFalse;
}

/* @func ajFeatWrite **********************************************************
**
** Generic interface function for reading in features from a file
** given the file handle, class of map, data format of input
** and possibly other associated data.
**
** @param  [rC] ftout   [AjPFeattabOut]  Specifies the external source
**                                       (file) of the features to be read in
** @cre 'ftout' argument must be a valid AjPFeattabOut object
** @param  [rC] features [AjPFeattable]  Feature set to be written out
** @return [AjBool]  Returns ajTrue if successful; ajFalse otherwise
** @@
**
** Version 1.0, 21/6/99 ACD to ajfeat access function (for writing features)
**
** Old function protocol:
**
** AjBool ajFeatWrite( AjPFile file,
**                         AjPFeattable features,
**                         ajint format )
**
******************************************************************************/

AjBool ajFeatWrite ( AjPFeattabOut ftout, AjPFeattable features )
{
  AjPFile       file ;
  ajint format ;
  AjBool result          = ajFalse ;

  if(features){
    ajDebug( "ajFeatWrite Validating arguments\n");

    assert (ftout);

    file    = ftout->Handle ;
    assert (file);

    format  = ftout->Format ;

    ajDebug( "ajFeatWrite format is %d OK\n",ftout->Format);

    featOutFormat[format].VocInit();
    result = featOutFormat[format].Write(features, file);

    return result ;
  }
  else{
    ajDebug( " NO Features to output\n");
    return AJTRUE;
  }
}

/* @funcstatic featReadEmbl ***************************************************
**
** Reads feature data in EMBL format
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadEmbl     ( AjPFeattable thys, AjPFileBuff file){
  static AjPStr line  = NULL ;
  AjBool found = ajFalse ;
  static AjPStr savefeat  = NULL;
  static AjPStr saveline = NULL;
  static AjPStr saveloc  = NULL;

  if(!line)
    line = ajStrNewL(100);

  ajFeattableSetDna (thys);

  while( ajFileBuffGet (file, &line) ) {

    /* if it's an EMBL feature do stuff */
    if(!ajStrNCmpC(line, "FT   ", 5)){
      (void) ajStrChompEnd(&line); /* remove newline */
      if(featEmblFromLine(thys, line, &savefeat, &saveloc, &saveline))
	found = ajTrue ;
    }

    /* if it's a GenBank feature do stuff */
    else if(!ajStrNCmpC(line, "     ", 5)) {
      (void) ajStrChompEnd(&line); /* remove newline */
      if(featEmblFromLine(thys, line, &savefeat, &saveloc, &saveline))
	found = ajTrue ;
    }
  }
  if (featEmblFromLine(thys, NULL, &savefeat,&saveloc, &saveline))
      found = ajTrue;

  ajStrDel (&saveloc);
  ajStrDel (&saveline);

  return found;
}

/* @funcstatic featReadUnknown ************************************************
**
** Reads feature data in Unknown format
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadUnknown    ( AjPFeattable thys, AjPFileBuff file) {
  return ajFalse;
}

/* @funcstatic featReadPir ****************************************************
**
** Reads feature data in PIR format
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadPir   ( AjPFeattable thys, AjPFileBuff file) {
  static AjPStr line  = NULL ;
  AjBool found = ajFalse;

  ajDebug("featReadPir..........\n");

  while( ajFileBuffGet (file, &line) ) {

    (void) ajStrChomp(&line) ;

    ajDebug ("++ line '%S'\n", line);

    if(ajStrPrefixC (line, "F;")) {
      if(featPirFromLine(thys, line))
	found = ajTrue ;
    }
  }
  return found ;
}

/* @funcstatic featPirFromLine ************************************************
**
** Read input file line in PIR format
**
** Format is :-
** F;position/type: note #comment
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] origline [AjPStr] Input line
** @return [AjPFeature] New feature.
** @@
******************************************************************************/

static AjPFeature featPirFromLine ( AjPFeattable thys,
				    AjPStr origline) {
  static AjPStr source   = NULL;
  static AjPStr temp     = NULL;
  static AjPFeature gf   = NULL ;    /* made static so that it's easy
					 to add second line of description */
  static AjPStr locstr   = NULL;
  static AjPStr typstr   = NULL;
  static AjPStr notestr   = NULL;
  static AjPStr comstr   = NULL;
  static AjPStr exonstr   = NULL;
  static AjPStr posstr   = NULL;
  static AjPStr tagnote=NULL;
  static AjPStr tagcomm=NULL;
  ajint i=0;
  AjBool mother=ajTrue;
  ajint Start=0;
  ajint End = 0;
  ajint Flags = 0;

  ajDebug("featPirFromLine..........\n'%S'\n", origline);

  if (!tagnote)
    ajStrAssC (&tagnote, "note");
  if (!tagcomm)
    ajStrAssC (&tagcomm, "comment");
  if(!source){
    source    = ajStrNewC("PIR");
  }

  if (!ajRegExec(PirRegexAll, origline))
    return NULL;

  ajRegSubI(PirRegexAll, 1, &locstr);
  ajRegSubI(PirRegexAll, 2, &typstr);
  ajRegSubI(PirRegexAll, 3, &notestr);

  /* remove spaces in feature type so we can look it up */

  ajStrSubstituteCC (&typstr, " ", "_");

  featTypePirIn(&typstr);
  ajStrClean(&notestr);

  /* decode the position(s) */

  while (ajRegExec(PirRegexLoc, locstr)) { /* split at ',' */
    ajRegSubI(PirRegexLoc, 1, &exonstr);
    ajRegPost(PirRegexLoc, &temp);
    ajStrAssS(&locstr, temp);
    i=0;
    while (ajRegExec(PirRegexPos, exonstr)) { /* split at '-' */
      ajRegSubI(PirRegexPos, 1, &posstr);
      if (!i++) {
	if (!ajStrToInt (posstr, &Start))
	  Start = 1;
      }
      ajRegPost(PirRegexPos, &temp);
      ajStrAssS(&exonstr, temp);
    }
    if (!ajStrToInt (posstr, &End))
      End = 1;

    gf = featFeatNewProt ( thys,
			   source,	/* source sequence */
			   typstr,
			   Start, End,
			   0.0,
			   Flags ) ;

    /* for the first feature, process the rest of the tags */

    if (mother) {

      if (ajStrLen(notestr)) {
	ajFeatTagAdd (gf, tagnote, notestr);
      }

      ajRegPost(PirRegexAll, &temp);
      while (ajRegExec(PirRegexCom, temp)) {
	ajRegSubI(PirRegexCom, 1, &comstr);
	ajStrClean(&comstr);
	ajFeatTagAdd (gf, tagcomm, comstr);
	ajRegPost(PirRegexCom, &temp);
      }
    }

    mother = ajFalse;
    Flags |= FEATFLAG_CHILD;
  }

  if (mother) {
    ajWarn("featPirFromLine: Did not understand location '%S'", locstr);
  }

  return gf;
}

/* @funcstatic featReadSwiss **************************************************
**
** Reads feature data in SwissProt format
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadSwiss    ( AjPFeattable thys, AjPFileBuff file) {
  static AjPStr line  = NULL ;
  AjBool found = ajFalse;
  static AjPStr savefeat = NULL;
  static AjPStr saveline = NULL;
  static AjPStr savefrom  = NULL;
  static AjPStr saveto  = NULL;

  ajDebug("featReadSwiss..........\n");

  while( ajFileBuffGet (file, &line) ) {

    (void) ajStrChomp(&line) ;

    ajDebug ("++ line '%S'\n", line);

    if(ajStrPrefixC (line, "FT   ")) {
      if(featSwissFromLine(thys, line,
			   &savefeat, &savefrom, &saveto, &saveline))
	found = ajTrue ;
    }
  }

  featSwissFromLine(thys, NULL, &savefeat, &savefrom, &saveto, &saveline);

  return found ;
}


/* ==================================================================== */
/* ======================== GFF Processing functions ================== */
/* =================================================================== */

/* ajfeat defaults to version 2 GFF only...*/
#define DEFAULT_GFF_VERSION 2


/* @funcstatic featFlagSet ****************************************************
**
** Sets the flags for a feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [r] flagstr  [AjPStr]      Flags as a hexadecimal value
** @return [void]
** @@
******************************************************************************/

static void featFlagSet (AjPFeature gf, AjPStr flagstr) {

  static AjPRegexp flagexp = NULL;
  static AjPRegexp moreexp = NULL;
  static AjPStr savstr = NULL;
  static AjPStr tmpstr = NULL;
  static AjPStr typstr = NULL;
  static AjPStr valstr = NULL;
  ajint flags = 0;
  ajint num = 0;

  if (!flagexp)
      flagexp = ajRegCompC ("[ \"]*(0x[0-9a-f]+)");
  if (!moreexp)
      moreexp = ajRegCompC ("[ \"]*([^:]+):([^: \"]+)");

  ajDebug ("featFlagSet '%S'\n", flagstr);
  ajStrAssS (&savstr, flagstr);

  if (ajRegExec(flagexp, savstr)) {
    ajRegSubI (flagexp, 1, &tmpstr);
    if (ajStrToHex(tmpstr, &flags))
	gf->Flags = flags;
    ajDebug("flags: %x", gf->Flags);
    ajRegPost (flagexp, &tmpstr);
    ajStrAssS (&savstr, tmpstr);
  }

  while (ajRegExec(moreexp, savstr)) {
    ajRegSubI (moreexp, 1, &typstr);
    ajRegSubI (moreexp, 2, &valstr);
    ajDebug ("flag type '%S' val '%S'\n", typstr, valstr);
    if (ajStrMatchCaseC(typstr, "start2")) {
      if (ajStrToInt(valstr, &num))
	gf->Start2 = num;
    }
    else if(ajStrMatchCaseC(typstr, "end2")) {
      if (ajStrToInt(valstr, &num))
	gf->End2 = num;
    }
    else if(ajStrMatchCaseC(typstr, "remoteid")) {
      ajStrAssS (&gf->Remote, valstr);
    }
    else if(ajStrMatchCaseC(typstr, "label")) {
      ajStrAssS (&gf->Label, valstr);
    }
    else {
	ajWarn("Unknown GFF FeatFlags type '%S:%S'", typstr, valstr);
    }

    ajRegPost (moreexp, &tmpstr);
    ajStrAssS (&savstr, tmpstr);
  }

  return;
}

/* @funcstatic featGroupSet ***************************************************
**
** Sets the group tag for a feature.
**
** @param [u] gf       [AjPFeature]  Feature
** @param [u] table    [AjPFeattable] Feature table
** @param [r] grouptag [AjPStr]      Group field identifier
** @return [void]
** @@
******************************************************************************/

static void featGroupSet (AjPFeature gf, AjPFeattable table,
			  AjPStr grouptag) {

  static AjPRegexp groupexp = NULL;
  static AjPStr namstr = NULL;
  static AjPStr grpstr = NULL;
  static AjPStr savgrpstr = NULL;
  static ajint saveexon = 0;
  static ajint savegroup = 0;
  ajint grpnum;

  if (!groupexp)
    groupexp = ajRegCompC("^\"(([^.]*)[.])?([0-9]+)");

  if (ajStrLen(grouptag) && ajStrMatchCase(grouptag, savgrpstr)) {
    gf->Group = savegroup;
    gf->Exon = ++saveexon;
    return;
  }


  if (ajStrLen(grouptag) && ajRegExec(groupexp, grouptag)) {
    ajStrAssS (&savgrpstr, grouptag);
    ajRegSubI (groupexp, 2, &namstr);
    ajRegSubI (groupexp, 3, &grpstr);
    ajDebug ("featGroupSet '%S' name: '%S' group: '%S'\n",
	     grouptag, namstr, grpstr);
    if (ajStrToInt (grpstr, &grpnum)) {/* true, if the regex worked */
      gf->Group = grpnum;
      savegroup = grpnum;
    }
    else
      gf->Group = ++(table->Groups);
    if (ajStrLen(namstr)) {
      if (!ajStrMatchCase (namstr, table->Name)) {
	ajDebug ("GFF group field '%S' table '%S'\n",
		 grouptag, table->Name);
	ajWarn ("GFF group field '%S' for table '%S'",
		 grouptag, table->Name);
      }
    }
  }
  else {			/* regex failed, make something up */
    ajStrAssS(&grpstr, grouptag);
    gf->Group = ++(table->Groups);
    savegroup = gf->Group;
    gf->Exon = 0;
    saveexon = 0;
  }

}

/* @funcstatic featGffProcessTagval *******************************************
**
** Parses and adds a tag-value set to the specified AjPFeature;
** looked at 'parse_group' method in GFF::GeneFeature.pm Perl module
** for inspiration
**
** @param [u] gf [AjPFeature] Feature
** @param [u] table [AjPFeattable] Feature
** @param [r] groupfield [AjPStr] Group field identifier
** @param [r] version [float] GFF version
** @return [void]
** @@
******************************************************************************/

static void featGffProcessTagval (AjPFeature gf, AjPFeattable table,
				  AjPStr groupfield, float version)
{
  static AjPStr  TvString  = NULL ;
  AjPStr  tmptag      = NULL ;
  AjPStr  tmpval      = NULL ;
  AjBool  grpset = ajFalse;

  ajDebug("featGffProcessTagval version %3.1f '%S'\n", version, groupfield);

  /* Validate arguments */
  if(!ajStrLen(groupfield))	/* no tags, must be new */
    return;

  if( version == 1.0 ) {
    (void) featGroupSet (gf, table, groupfield) ;
    ajDebug("V1.0 group: '%S'\n", groupfield);
    grpset = ajTrue;

    return;
  }

/*
 *     Version 2 or greater: parse groupfield for semicolon ';'
 *     delimited tag-value structures, taking special care about
 *     double quoted string context. rbskfirst version of code was
 *     adapted from GFF.pm (th/rbsk), itself inherited from AceParse.pm,
 *     courtesy of James Gilbert
*/

  (void) ajStrAssS( &TvString, groupfield) ;
  while (ajStrLen(TvString)) {
    if (ajRegExec(GffRegexTvTagval, TvString)) {
      ajRegSubI(GffRegexTvTagval, 1, &tmptag);
      ajRegSubI(GffRegexTvTagval, 2, &tmpval);
      (void) ajStrChomp(&tmpval) ;
      ajDebug("GffTv '%S' '%S'\n", tmptag, tmpval);
      ajRegPost (GffRegexTvTagval, &TvString);
      if (ajStrMatchC (tmptag, "Sequence")) {
	(void) featGroupSet (gf, table, tmpval) ;
	grpset = ajTrue;
      }
      else if (ajStrMatchC (tmptag, "FeatFlags"))
	(void) featFlagSet (gf, tmpval) ;
      else {
	ajDebug("Before QuoteStrip: '%S'\n", tmpval);
	ajStrQuoteStrip(&tmpval);
	ajDebug(" After QuoteStrip: '%S'\n", tmpval);
	(void) ajFeatTagAdd (gf,tmptag,tmpval) ;
      }
    }
    else {
      ajDebug("Choked on '%S'\n", TvString);
      ajWarn("GFF tag parsing failed on '%S' in\n'%S'",
	     TvString, groupfield);
      ajStrDel(&TvString);
    }
  }

  if (!grpset) {
    (void) featGroupSet (gf, table, NULL) ;
    grpset = ajTrue;
  }

  return;
}

/* @funcstatic featSwissFromLine **********************************************
**
** Read input file in Swiss format
**
** Format is :-
** 0-1    FT
** 5-12   Keyname
** 14-19  From
** 21-26  To
** 34-74  Descrition
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] origline [AjPStr] Input line
** @param [r] savefeat [AjPStr*] Stored feature type
** @param [r] savefrom [AjPStr*] Continued from position
** @param [r] saveto   [AjPStr*] Continued to position
** @param [r] saveline [AjPStr*] Continued tag-value pairs
** @return [AjPFeature] New feature.
** @@
******************************************************************************/

static AjPFeature featSwissFromLine ( AjPFeattable thys,
				      AjPStr origline,
				      AjPStr* savefeat,
				      AjPStr* savefrom,
				      AjPStr* saveto,
				      AjPStr* saveline)
{
  static AjPStr qryfrom  = NULL;
  static AjPStr qryto    = NULL;
  static AjPStr source   = NULL;
  static AjPStr temp     = NULL;
  static AjPFeature gf   = NULL ;    /* made static so that it's easy
					 to add second line of description */
  AjBool newft = ajFalse;

  ajDebug("featSwissFromLine..........\n'%S'\n", origline);

  if(!source){
    source    = ajStrNewC("SWISSPROT");
  }

  if (origline) {
    newft = ajRegExec(SwRegexNew, origline);
  }

  if (newft || !origline) { /* process the last feature */
    ajDebug ("++ feat+from+to '%S' '%S' '%S'\n+ saveline '%S'\n",
	     *savefeat, *savefrom, *saveto, *saveline);
    if (ajStrLen(*savefrom)) {	/* finish the current feature */
      gf = featSwissProcess (thys, *savefeat, *savefrom, *saveto,
			     source, *saveline);
    }
    else {			/*  maybe there were no features */
      gf = NULL;
    }

    ajStrDelReuse(savefeat);
    ajStrDelReuse(savefrom);
    ajStrDelReuse(saveto);
    ajStrDelReuse(saveline);
    ajStrDelReuse(&qryfrom);
    ajStrDelReuse(&qryto);
  }

  if (!origline)		/* we are only cleaning up */
    return gf;

  if (newft) {	/* if new feature initialise for it */
    ajRegSubI (SwRegexNew, 2, savefeat);
    ajRegSubI (SwRegexNew, 3, &qryfrom);
    ajRegSubI (SwRegexNew, 4, savefrom);
    ajRegSubI (SwRegexNew, 5, &qryto);
    ajRegSubI (SwRegexNew, 6, saveto);
    ajRegSubI (SwRegexNew, 7, saveline);
    ajStrChomp(savefeat);
    ajDebug (" newft type '%S' from '%S' to '%S' rest '%S'\n",
	     *savefeat, *savefrom, *saveto, *saveline);
    return gf;
  }

  else {             /* more tag-values */
    if (ajRegExec(SwRegexNext, origline)) {
      ajRegSubI(SwRegexNext, 1, &temp);
      ajStrAppC (saveline, " ");
      ajStrApp (saveline, temp);
    }
    else {
      ajWarn ("Bad SwissProt feature line:\n%S", origline);
    }
  }

  return gf;

}

/* @funcstatic featSwissProcess ***********************************************
**
** Processes one feature location and qualifier tags for SwissProt
**
** @param [r] thys [AjPFeattable] Feature table
** @param [u] feature [AjPStr] Feature type key
** @param [u] fromstr [AjPStr] Feature start
** @param [u] tostr [AjPStr] Feature end
** @param [u] source [AjPStr] Feature table source
** @param [r] tags [AjPStr] Feature qualifier tags string
** @return [AjPFeature] Feature as inserted into the feature table
** @@
******************************************************************************/

static AjPFeature featSwissProcess  ( AjPFeattable thys, AjPStr feature,
				      AjPStr fromstr, AjPStr tostr,
				      AjPStr source,
				      AjPStr tags) {

  AjPFeature ret;
  ajint Start=0;
  ajint End = 0;
  ajint flags = 0;
  static AjPStr note=NULL;
  static AjPStr comment = NULL;
  static AjPStr ftid=NULL;
  static AjPStr tagnote=NULL;
  static AjPStr tagftid=NULL;
  static AjPStr tagcomm=NULL;
  static AjPStr tagstr=NULL;

  if (!tagnote)
    ajStrAssC (&tagnote, "note");
  if (!tagcomm)
    ajStrAssC (&tagcomm, "comment");
  if (!tagftid)
    ajStrAssC (&tagftid, "ftid");


  if (!ajStrToInt (fromstr, &Start))
    Start = 0;
  if (!ajStrToInt (tostr, &End))
    End = 0;

  ret = featFeatNewProt ( thys,
			  source,	/* source sequence */
			  feature,
			  Start, End,
			  0.0,
			  flags ) ;

  ajStrAssS (&tagstr, tags);
  ajStrTrimC(&tagstr, " .");

  if (ajRegExec (SwRegexFtid, tagstr)) {
    ajRegSubI (SwRegexFtid, 1, &note);
    ajRegSubI (SwRegexFtid, 2, &ftid);
    ajDebug("Swiss ftid found\nftid: '%S'\n",
	    ftid);
    if (ajStrLen(ftid))
      ajFeatTagAdd (ret, tagftid, ftid);
    ajStrAssS(&tagstr, note);
    ajStrTrimC(&tagstr, " .");
  }

  if (ajRegExec (SwRegexComment, tagstr)) {
    ajRegSubI (SwRegexComment, 1, &note);
    ajRegSubI (SwRegexComment, 2, &comment);
    ajDebug("Swiss comment found\nNote:  '%S'\nComment: '%S'\n",
	    note, comment);
    ajStrTrimC(&note, " .");
    if (ajStrLen(note))
      ajFeatTagAdd (ret, tagnote, note);
    if (ajStrLen(comment))
    ajFeatTagAdd (ret, tagcomm, comment);
  }
  else {
    ajDebug("Simple swiss note: '%S'\n", tagstr);
    if (ajStrLen(tagstr))
      ajFeatTagAdd (ret, tagnote, tagstr);
  }

  return ret;
}
/* @funcstatic featEmblFromLine ***********************************************
**
** Converts an input EMBL format line into a feature
**
** @param [r] thys     [AjPFeattable] Feature table
** @param [r] origline [AjPStr] Input line
** @param [r] savefeat [AjPStr*] Stored feature type
** @param [r] saveloc  [AjPStr*] Continued location
** @param [r] saveline [AjPStr*] Continued tag-value pairs
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featEmblFromLine ( AjPFeattable thys,
				     AjPStr origline,
				     AjPStr* savefeat,
				     AjPStr* saveloc,
				     AjPStr* saveline)
{
  static AjPFeature gf    = NULL ;      /* so tag-values can be added LATER */
  static AjPStr source    = NULL;
  static AjPStr line      = NULL;
  static AjPStr temp=NULL;
  AjBool newft = ajFalse;

  if(!source)
    source = ajStrNewC("EMBL");

  ajDebug("featEmblFromLine '%S'\n", origline);

  ajStrAssS(&line,origline);              /* As BufferFile cannot be edited */

  if (origline)
    newft = ajRegExec(EmblRegexNew, line);

  /*
    ajDebug ("+ newft: %B\n+ line '%S'\n",
	   newft, line);
  */

  if (newft || !origline) { /* process the last feature */
    ajDebug ("++ saveloc '%S'\n+ saveline '%S'\n",
	     *saveloc, *saveline);
    if (ajStrLen(*saveloc)) {
      gf = featEmblProcess (thys, *savefeat, source, saveloc, saveline);
    }
    else {
      gf = NULL;
    }
    ajStrDelReuse(saveloc);
    ajStrDelReuse(saveline);
  }

  if (!origline)		/* we are only cleaning up */
    return gf;

  if (newft) {	/* if new feature initialise for it */
    ajRegSubI (EmblRegexNew, 1, savefeat);
    ajRegSubI (EmblRegexNew, 2, saveloc);
    ajRegSubI (EmblRegexNew, 3, saveline);
    ajStrChomp(savefeat);
    return gf;
  }
  else if(!ajStrLen(*saveline)) { /* more location? */
    if (ajRegExec(EmblRegexNext, line)) {
      ajRegSubI(EmblRegexNext, 1, &temp);
      if (temp)
	ajStrApp (saveloc, temp);
      ajRegSubI(EmblRegexNext, 2, &temp);
      if (temp)
	ajStrApp (saveline, temp);
    }
    else {
      ajDebug ("Bad EMBL feature line:\n%S\n", line);
      ajWarn ("Bad EMBL feature line:\n%S", line);
    }
  }
  else {			/* tag-values */
    if (ajRegExec(EmblRegexTv, line)) {
      ajRegSubI(EmblRegexTv, 1, &temp);
      ajStrApp (saveline, temp);
    }
    else {
      ajDebug ("Bad EMBL feature line:\n%S\n", line);
      ajWarn ("Bad EMBL feature line:\n%S", line);
    }
  }

  return gf;
}

/* @funcstatic featEmblProcess ************************************************
**
** Processes one feature location and qualifier tags for EMBL
**
** @param [r] thys [AjPFeattable] Feature table
** @param [u] feature [AjPStr] Feature type key
** @param [u] source [AjPStr] Feature table source
** @param [r] loc [AjPStr*] Feature location
** @param [r] tags [AjPStr*] Feature qualifier tags string
** @return [AjPFeature] Feature as inserted into the feature table
** @@
******************************************************************************/

static AjPFeature featEmblProcess  ( AjPFeattable thys, AjPStr feature,
				     AjPStr source,
				     AjPStr* loc, AjPStr* tags) {

  AjPFeature ret = NULL;
  AjPFeature gf = NULL;
  AjPStr tag=NULL;
  AjPStr val=NULL;
  AjPStr opnam=NULL;
  AjPStr opval=NULL;
  AjPStr tmpstr=NULL;
  AjBool Fwd = ajTrue;
  AjBool LocFwd = ajTrue;
  AjPStr begstr=NULL;
  AjPStr delstr=NULL;
  AjPStr endstr=NULL;
  AjPStr locstr=NULL;
  AjBool Simple=ajFalse;
  AjBool BegBound=ajFalse;
  AjBool EndBound=ajFalse;
  ajint BegNum=0;
  ajint EndNum=0;
  ajint Beg2=0;
  ajint End2=0;
  static AjPStr entryid=NULL;
  static AjPStr label=NULL;
  AjBool Between = ajFalse;
  AjBool Join = ajFalse;
  AjBool Group = ajFalse;
  AjBool Order = ajFalse;
  AjBool OneOf = ajFalse;
  ajint Flags;
  char Strand = '+';
  AjBool Mother = ajTrue;
  ajint Frame = 0;
  float Score = 0.0;
  AjBool HasOper = ajFalse;
  AjBool RemoteId = ajFalse;
  AjBool IsLabel = ajFalse;
  ajint Exon = 0;

  ajStrCleanWhite(loc);	/* no white space needed */
  ajStrClean(tags);		/* single spaces only */
  ajDebug("cleaned feat loc: '%S'\n            tags: '%S'\n",
	  *loc, *tags);


  ajDebug("Location '%S'\n", *loc);
  ajStrAssS (&opval, *loc);
  if (ajRegExec(EmblRegexLocMulti, opval)) {
    ajDebug("Multiple locations, test operator(s)\n");
     while (ajStrLen(opval) && ajRegExec (EmblRegexOperOut, opval)) {
      ajRegSubI(EmblRegexOperOut, 1, &opnam);
      ajRegSubI(EmblRegexOperOut, 2, &tmpstr);
      ajDebug("OperOut %S( '%S' )\n", opnam, tmpstr);
      if (ajStrMatchCaseC(opnam, "complement")) {
	Fwd = !Fwd;
      }
      if (ajStrMatchCaseC(opnam, "one_of")) {
	OneOf = ajTrue;
      }
      if (ajStrMatchCaseC(opnam, "join")) {
	Join = ajTrue;
      }
      if (ajStrMatchCaseC(opnam, "order")) {
	Order = ajTrue;
      }
      if (ajStrMatchCaseC(opnam, "group")) {
	Group = ajTrue;
      }
      ajStrAssS (&opval, tmpstr);
    }
  }

  while (ajStrLen(opval)) {
    LocFwd = Fwd;
    BegBound = ajFalse;
    EndBound = ajFalse;
    Simple = ajFalse;
    Between = ajFalse;
    BegNum = EndNum = Beg2 = End2 = 0;
    HasOper = ajFalse;
    RemoteId = ajFalse;
    IsLabel = ajFalse;
    ajStrDelReuse(&entryid);
    ajStrDelReuse(&label);
    if (ajRegExec (EmblRegexOperIn, opval)) {
      ajRegSubI(EmblRegexOperIn, 1, &opnam);
      ajRegSubI(EmblRegexOperIn, 2, &locstr);
      ajDebug("OperIn %S( '%S' )\n", opnam, locstr);
      if (ajStrMatchCaseC(opnam, "complement"))
	LocFwd = !LocFwd;
      ajRegPost (EmblRegexOperIn, &tmpstr);
      ajStrAssS (&opval, tmpstr);
      ajDebug("rest: '%S'\n", opval);
      HasOper = ajTrue;
    }
    else {
      ajStrAssS(&locstr, opval);
    }
    if  (ajRegExec (EmblRegexOperNone, locstr)) { /* one exon */
      ajRegSubI(EmblRegexOperNone, 2, &entryid); /* if any */
      ajRegSubI(EmblRegexOperNone, 3, &tmpstr);	/* position (or label) */
      ajDebug("OperNone '%S' \n", tmpstr);
      if (ajStrLen(entryid)) {
	ajDebug("External entryid '%S'\n", entryid);
	RemoteId = ajTrue;
      }
      if (ajRegExec(EmblRegexLoc, tmpstr)) { /* xxx..xxx or xxx^xxx */
	ajRegSubI(EmblRegexLoc, 1, &begstr);
	ajRegSubI(EmblRegexLoc, 2, &delstr);
	if (ajStrMatchC(delstr, "^"))
	  Between = ajTrue;
	ajRegSubI(EmblRegexLoc, 3, &endstr);
	ajDebug("Location: '%S' '%S' '%S'\n", begstr, delstr, endstr);
      }
      else {
	ajStrAssS(&begstr, tmpstr);
	ajStrAssS(&endstr, begstr);
	Simple = ajTrue;
	ajDebug("Location: label='%S'\n", begstr);
      }

      ajRegPost (EmblRegexOperNone, &tmpstr);
      if (!HasOper) ajStrAssS (&opval, tmpstr);

      if (ajRegExec(EmblRegexLocNum, begstr)) {
	ajRegSubI(EmblRegexLocNum, 1, &tmpstr);
	if (ajStrLen(tmpstr))
	  BegBound=ajTrue;
	ajRegSubI(EmblRegexLocNum, 2, &tmpstr);
	ajStrToInt(tmpstr, &BegNum);
	ajDebug ("Begin '%S' %d  Bound: %B\n", begstr, BegNum, BegBound);
      }
      else if (ajRegExec(EmblRegexLocRange, begstr)) {
	ajRegSubI(EmblRegexLocRange, 1, &tmpstr);
	ajStrToInt(tmpstr, &BegNum);
	ajRegSubI(EmblRegexLocRange, 2, &tmpstr);
	ajStrToInt(tmpstr, &Beg2);
	ajDebug ("Begin range (%d . %d)\n", BegNum, Beg2);
      }
      else {
	ajDebug("Begin is a label '%S'\n", begstr);
	IsLabel = ajTrue;
	Simple = ajTrue;
	ajStrAssS (&label, begstr);
      }

      if (ajRegExec(EmblRegexLocNum, endstr)) {
	ajRegSubI(EmblRegexLocNum, 1, &tmpstr);
	if (ajStrLen(tmpstr))
	  EndBound=ajTrue;
	ajRegSubI(EmblRegexLocNum, 2, &tmpstr);
	ajStrToInt(tmpstr, &EndNum);
	ajDebug ("  End '%S' %d  Bound: %B\n", endstr, EndNum, EndBound);
      }
      else if (ajRegExec(EmblRegexLocRange, endstr)) {
	ajRegSubI(EmblRegexLocRange, 1, &tmpstr);
	ajStrToInt(tmpstr, &End2);
	ajRegSubI(EmblRegexLocRange, 2, &tmpstr);
	ajStrToInt(tmpstr, &EndNum);
	ajDebug ("  End range (%d . %d)\n", End2, EndNum);
      }
      else {
	IsLabel = ajTrue;
	Simple = ajTrue;
	ajStrAssS (&label, endstr);
	ajDebug("  End is a label '%S'\n", endstr);
      }
    }
    else {
      ajErr ("Unable to parse location:\n'%S'", opval);
    }

    /* location has been read in, now store it */

    Flags = 0;
    if (LocFwd) Strand = '+';
    else Strand = '-';

    if (Mother) {
      if (!Fwd) Flags |= FEATFLAG_COMPLEMENT_MAIN;
    }
    else {
      Flags |= FEATFLAG_CHILD;
    }
    if (Join || Order || Group || OneOf) Flags |= FEATFLAG_MULTIPLE;
    if (Group) Flags |= FEATFLAG_GROUP;
    if (Order) Flags |= FEATFLAG_ORDER;
    if (OneOf) Flags |= FEATFLAG_ONEOF;

    if (Simple) Flags |= FEATFLAG_POINT;
    if (Between) Flags |= FEATFLAG_BETWEEN_SEQ;
    if (End2) Flags |= FEATFLAG_END_TWO;
    if (Beg2) Flags |= FEATFLAG_START_TWO;
    if (BegBound) Flags |= FEATFLAG_START_BEFORE_SEQ;
    if (EndBound) Flags |= FEATFLAG_END_AFTER_SEQ;
    if (RemoteId) Flags |= FEATFLAG_REMOTEID;
    if (IsLabel) Flags |= FEATFLAG_LABEL;

    ajDebug("Calling featFeatNew, Flags: %x\n", Flags);

    gf = featFeatNew ( thys,
			source,	/* source sequence */
			feature,
			BegNum, EndNum,
			Score,
			Strand,
			Frame,
			Exon, Beg2, End2, entryid, label, Flags ) ;
    if (Mother)
      ret = gf;
    Mother = ajFalse;
    /*if (OneOf) break;*/
  }

  while (ajStrLen(*tags) && ajRegExec (EmblRegexTvTag, *tags)) {

    /* first process quoted values, which can look like multiple values */
    /* watch for "" double internal quotes */

    if (ajRegExec (EmblRegexTvTagQuote, *tags)) { /* /tag="val" */
      ajRegSubI (EmblRegexTvTagQuote, 1, &tag);
      ajRegSubI (EmblRegexTvTagQuote, 2, &val);
      ajRegPost (EmblRegexTvTagQuote, &tmpstr);
      ajStrAssS (tags, tmpstr);

      /* internal quotes are "" - save and fix after */
      while (ajRegExec (EmblRegexTvTagQuote2, *tags)) { /* "quoted ""val""" */
       ajRegSubI (EmblRegexTvTagQuote2, 1, &tmpstr);
       ajStrApp (&val, tmpstr);
       ajRegPost (EmblRegexTvTagQuote2, &tmpstr);
       ajStrAssS (tags, tmpstr);
      }
      ajDebug("Before QuoteStrip: '%S'\n", val);
      ajStrQuoteStrip(&val);
      ajDebug(" After QuoteStrip: '%S'\n", val);
      ajDebug ("(a) /%S='%S'\n", tag, val);
    }
    else {
      ajRegSubI (EmblRegexTvTag, 1, &tag);
      ajRegSubI (EmblRegexTvTag, 3, &val);
      if (ajStrLen(val))
	ajDebug ("(b) /%S='%S'\n", tag, val);
      else
	ajDebug ("(c) /%S\n", tag);
      ajRegPost (EmblRegexTvTag, &tmpstr);
      ajStrAssS (tags, tmpstr);
    }
    ajFeatTagAdd (ret, tag, val);
  }

  ajStrDel (&tmpstr);
  ajStrDel (&val);
  ajStrDel (&tag);
  ajStrDel (&begstr);
  ajStrDel (&delstr);
  ajStrDel (&opnam);
  ajStrDel (&opval);
  ajStrDel (&locstr);
  ajStrDel (&endstr);

  return ret;
}

/* @funcstatic featGffFromLine ************************************************
**
** Converts an input GFF format line into a feature
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] line [AjPStr] Input line
** @param [r] version [float] GFF version (1.0 for old format behaviour)
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featGffFromLine ( AjPFeattable thys, AjPStr line,
				    float version)
{
    AjPFeature gf    = NULL ;
    static AjPStrTok split  = NULL  ;
    static AjPStr
      seqname   = NULL,
      source    = NULL,
      feature   = NULL,
      start     = NULL,
      end       = NULL,
      score     = NULL,
      strandstr = NULL,
      framestr  = NULL,
      tagvalue  = NULL ;
    ajint Start=0, End=0;
    float fscore = 0.0;
    static AjPStr entryid=NULL;
    static AjPStr label=NULL;

    if(!ajStrLen(line)) return NULL ;

    split = ajStrTokenInit (line, "\t") ;

    if( !ajStrToken (&seqname, &split, NULL)) {           /* seqname */
        goto Error;
    } else if( !ajStrToken (&source, &split, NULL)) {     /* source  */
        goto Error;
    } else if( !ajStrToken (&feature, &split, NULL)) {    /* feature */
        goto Error;
    } else if( !ajStrToken (&start, &split, NULL)) {      /* start   */
        goto Error;
    } else if( !ajStrToken (&end, &split, NULL)) {        /* end     */
        goto Error;
    } else if( !ajStrToken (&score, &split, NULL)) {      /* score   */
        goto Error;
    } else if( !ajStrToken (&strandstr, &split, NULL)) {  /* strand  */
        goto Error;
    } else if( !ajStrToken (&framestr, &split, NULL)) {   /* frame   */
        goto Error;
    } else {

      /* feature object construction
	 and group tag */

        char   strand ;
        ajint    frame ;
        AjPStr          groupfield = NULL ;

        if(!ajStrToInt (start, &Start))
           Start = 0 ;
        if(!ajStrToInt (end,   &End))
           End   = 0 ;
        if(!ajStrToFloat (score,   &fscore))
           fscore   = 0.0 ;

        if(!ajStrCmpC(strandstr,"+")) {
           strand = '+' ;
        } else if( !ajStrCmpC(strandstr,"-")) {
           strand = '-' ;
        } else {
           strand = '\0' ;	/* change to \0 later */
        }

        if(!ajStrCmpC(framestr,"0")) {
           frame = 1 ;
        } else if( !ajStrCmpC(framestr,"1")) {
           frame = 2 ;
        } else if( !ajStrCmpC(framestr,"2")) {
           frame = 3 ;
        } else {
           frame = 0 ;
        }

        gf = featFeatNew( thys,
                           source,
                           feature,
                           Start, End,
                           fscore,
                           strand,
                           frame,
			   0,0,0, entryid, label, 0 ) ;

        if( ajStrTokenRest(&groupfield, &split))
           featGffProcessTagval( gf, thys, groupfield, version) ;

	ajStrDel(&groupfield) ;
	ajStrTokenClear(&split) ;

	(void) ajStrDelReuse(&seqname) ;
	(void) ajStrDelReuse(&source) ;
	(void) ajStrDelReuse(&feature) ;
	(void) ajStrDelReuse(&start) ;
	(void) ajStrDelReuse(&end) ;
	(void) ajStrDelReuse(&score) ;
	(void) ajStrDelReuse(&strandstr) ;
	(void) ajStrDelReuse(&framestr) ;
	(void) ajStrDelReuse(&tagvalue) ;

	return gf ;
    }

Error:

    ajStrTokenClear(&split) ;

    (void) ajStrDelReuse(&seqname) ;
    (void) ajStrDelReuse(&source) ;
    (void) ajStrDelReuse(&feature) ;
    (void) ajStrDelReuse(&start) ;
    (void) ajStrDelReuse(&end) ;
    (void) ajStrDelReuse(&score) ;
    (void) ajStrDelReuse(&strandstr) ;
    (void) ajStrDelReuse(&framestr) ;
    (void) ajStrDelReuse(&tagvalue) ;

    return gf ;
}

/* @funcstatic featReadGff ****************************************************
**
** Read input file in GFF format
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFileBuff] Input buffered file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadGff ( AjPFeattable thys, AjPFileBuff file)
{
  static AjPStr line  = NULL ;
  AjBool found = ajFalse ;
  float version = 2.0;

  ajDebug("featReadGff..........\n");

  while( ajFileBuffGet (file, &line) ) {

    (void) ajStrChomp(&line) ;

    /* Header information */

    if(ajRegExec(GffRegexblankline, line)) {
      ; /* ignore */
    }
    else if(ajRegExec(GffRegexversion,line)) {
      AjPStr verstr = NULL ;
      ajRegSubI (GffRegexversion, 1, &verstr);
      (void) ajStrToFloat (verstr, &version);
      ajStrDel(&verstr);
    }
/*
//    else if(ajRegExec(GffRegexdate,line)) {
//      AjPStr year  = NULL ;
//      AjPStr month = NULL ;
//      AjPStr day   = NULL ;
//      ajint nYear, nMonth, nDay ;
//      ajRegSubI (GffRegexdate, 1, &year);
//      ajRegSubI (GffRegexdate, 2, &month);
//      ajRegSubI (GffRegexdate, 3, &day);
//      (void) ajStrToInt (year,  &nYear);
//      (void) ajStrToInt (month, &nMonth);
//      (void) ajStrToInt (day,   &nDay);
//      ajStrDel(&year);
//      ajStrDel(&month);
//      ajStrDel(&day);
//    }
*/
    else if(ajRegExec(GffRegexregion,line)) {
      AjPStr start = NULL ;
      AjPStr end   = NULL ;
      ajRegSubI (GffRegexregion, 2, &start);
      ajRegSubI (GffRegexregion, 3, &end);
      (void) ajStrToInt (start, &(thys->Start));
      (void) ajStrToInt (end,   &(thys->End));
      ajStrDel(&start);
      ajStrDel(&end);
    }
    else if(ajRegExec(GffRegexcomment,line)) {
      ; /* ignore for now... could store them in
	   ajFeattable for future reference though?...*/
    }

    /* the real feature stuff */

    else {			/* must be a real feature at last !! */
      if(featGffFromLine(thys, line, version))  /* does the ajFeattableAdd */
	found = ajTrue ;
    }

    ajStrDelReuse(&line);
  }
  return found ;
}

/* @func ajFeattableWriteGff **************************************************
**
** Write feature table in GFF format
**
** @param [r] Feattab [AjPFeattable] feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGff (AjPFeattable Feattab, AjPFile file)
{
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  /* Check arguments */
  ajDebug ("ajFeattableWriteGff Checking arguments\n");
  assert (file);

  /* Print header first */
  (void) ajFmtPrintF (file, "##gff-version 2.0\n") ;

  (void) ajFmtPrintF (file, "##date %D\n", ajTimeTodayF("GFF")) ;

  if (ajStrMatchC(Feattab->Type, "N"))
    (void) ajFmtPrintF (file, "##Type %s %S\n", "DNA", Feattab->Name) ;
  else if (ajStrMatchC(Feattab->Type, "P"))
    (void) ajFmtPrintF (file, "##Type %s %S\n", "Protein", Feattab->Name) ;
  else
    (void) ajFmtPrintF (file, "##Type unknown <%S> %S\n",
			Feattab->Type, Feattab->Name) ;

  /*
  (void) ajFmtPrintF (file, "##sequence-region %s %d %d\n",
		      ajStrStr(Feattab->Name),
		      Feattab->Start,
		      Feattab->End) ;
  */

  /* For all features... relatively simple because internal structures
     are deliberately styled on GFF */

  if (Feattab->Features) {
    iter = ajListIter(Feattab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      featDumpGff(gf, Feattab, file) ;
    }
    ajListIterFree(iter) ;
  }

  return ajTrue ;
}

/* @funcstatic featRegInitEmbl ************************************************
**
** Initialize regular expressions and data structures for
** EMBL/GenBank/DDBJ format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitEmbl (void) {

  if (FeatInitEmbl)
    return ajTrue;

  featInit();

  featVocabInitEmbl();

  ajDebug ("Tables embl Type: %x Tags: %x\n",
	   FeatTypeTableEmbl, FeatTagsTableEmbl);

  EmblRegexLoc  = ajRegCompC("^(.*)([.][.]|\\^)(.*)$") ; /* start+end */
  EmblRegexLocMulti  = ajRegCompC(",") ; /* multiple location */
  EmblRegexLocNum  = ajRegCompC("^([<>]?)([0-9]+)$") ; /* <n >n */
  EmblRegexLocRange  = ajRegCompC("^[(]([0-9]+)[.]([0-9]+)[)]$") ; /* (n.n) */
  EmblRegexNew = ajRegCompC("^..   ([^ ]+) +([^/]*)(/.*)?") ; /* start
								 of
								 new
								 feature */
  EmblRegexNext = ajRegCompC("^..    +([^/]*)(/.*)?") ; /* start of
							   new
							   feature */
  EmblRegexOperIn  = ajRegCompC("^([a-zA-Z_]+)[(]([^)]+)[)],?") ; /* oper()
								     internal*/
  EmblRegexOperNone  = ajRegCompC("^(([^:,]+):)?([^,]+),?") ; /* simple
								 location */
  EmblRegexOperOut = ajRegCompC("^([a-zA-Z_]+)[(]"         /* oper() outside*/
				"(([^[()]+([(][^()]+[)])?)+)[)]$") ;
  EmblRegexTv = ajRegCompC("^..    +( .*)") ; /* start of new feature */
  EmblRegexTvTag = ajRegCompC("^ */([^/= ]+)(=([^/ ]+))?") ; /* tag=val */
  /* quoted strings include the first quote, but not the last */
  EmblRegexTvTagQuote = ajRegCompC("^ */([^/\"= ]+)=(\"[^\"]*\")"); /* quoted
								     string */
  EmblRegexTvTagQuote2 = ajRegCompC("^(\"[^\"]*\")") ; /* more string */

  FeatInitEmbl = ajTrue;

  return ajTrue;
}

/* @funcstatic featRegInitSwiss ***********************************************
**
** Initialize regular expressions and data structures for ajFeat#
** SwissProt format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitSwiss (void)
{
  if (FeatInitSwiss)
    return ajTrue;

  featInit();

  featVocabInitSwiss();

  ajDebug ("Tables swiss Type: %x Tags: %x\n",
	   FeatTypeTableSwiss, FeatTagsTableSwiss);

  ajDebug ("featRegInitSwiss Compiling regexps\n");
  if (!SwRegexNew)
    SwRegexNew = ajRegCompC("^FT   (([^ ]+) +([?]?)([0-9]+) +"
			    "([?]?)([0-9]+) *)(.*)$") ;
  if (!SwRegexNext)
    SwRegexNext = ajRegCompC("^FT    +(.*)$") ;
  if (!SwRegexComment)
    SwRegexComment = ajRegCompC("^(.*)[(]([^)]+)[)]$") ;
  if (!SwRegexFtid)
    SwRegexFtid = ajRegCompC("^(.*)/FTId=([^ .]+)$") ;

  FeatInitSwiss = ajTrue;

  return ajTrue;
}

/* @funcstatic featRegInitPir *************************************************
**
** Initialize regular expressions and data structures for ajFeat#
** Pir format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitPir (void)
{
  if (FeatInitPir)
    return ajTrue;

  featInit();

  featVocabInitPir();

  ajDebug ("Tables pir Type: %x Tags: %x\n",
	   FeatTypeTablePir, FeatTagsTablePir);

  ajDebug ("featRegInitPir Compiling regexps\n");
  if (!PirRegexAll)
    PirRegexAll = ajRegCompC("^F;([^/]+)/([^:]+):([^#]*)") ;
  if (!PirRegexCom)
    PirRegexCom = ajRegCompC("^#([^#]*)") ;
  if (!PirRegexLoc)
     PirRegexLoc = ajRegCompC("^([^,]+),?") ;
  if (!PirRegexPos)
     PirRegexPos = ajRegCompC("^([^-]+)-?") ;

  FeatInitPir = ajTrue;

  return ajTrue;
}

/* @funcstatic featRegInitGff *************************************************
**
** Initialize regular expressions and data structures for ajFeat GFF format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitGff (void) {
    /* Setup any global static runtime resources here
       for example, regular expression compilation calls */

  if (FeatInitGff)
    return ajTrue;

  featInit();

  featVocabInitGff();

  ajDebug ("featRegInitGff Compiling regexps\n");

  GffRegexNumeric = ajRegCompC("^[\\+-]?[0-9]+\\.?[0-9]*$") ;
  GffRegexblankline = ajRegCompC("^[ ]*$") ;
  GffRegexversion   = ajRegCompC("^##gff-version[ ]+([0-9]+)") ;
  GffRegexdate      = ajRegCompC("^##date[ ]+([0-9][0-9][0-9][0-9])-"
				 "([0-9][0-9]?)-([0-9][0-9]?)") ;
  GffRegexregion    = ajRegCompC("^##sequence-region[ ]+([0-9a-zA-Z]+)"
				 "[ ]+([\\+-]?[0-9]+)[ ]+([\\+-]?[0-9]+)") ;
  GffRegexcomment   = ajRegCompC ("^#[ ]*(.*)") ;

  GffRegexTvTagval = ajRegCompC (" *([^ ]+) *((\"(\\.|[^\\\"])*\"|"
				 "[^;]+)*)(;|$)"); /* "tag name */

  FeatInitGff = ajTrue;

  return ajTrue;
}

/* @funcstatic featDelRegEmbl *************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegEmbl(void) {

  if (!FeatInitEmbl)
    return ajTrue;

  ajRegFree(&EmblRegexLoc);
  ajRegFree(&EmblRegexLocMulti);
  ajRegFree(&EmblRegexLocNum);
  ajRegFree(&EmblRegexLocRange);
  ajRegFree(&EmblRegexNew);
  ajRegFree(&EmblRegexNext);
  ajRegFree(&EmblRegexOperIn);
  ajRegFree(&EmblRegexOperNone);
  ajRegFree(&EmblRegexOperOut);
  ajRegFree(&EmblRegexTv);
  ajRegFree(&EmblRegexTvTag);
  ajRegFree(&EmblRegexTvTagQuote);
  ajRegFree(&EmblRegexTvTagQuote2);

  ajStrTableFree(&FeatTypeTableEmbl);
  ajStrTableFree(&FeatTagsTableEmbl);

  FeatInitEmbl = ajFalse;
  return ajTrue;
}

/* @funcstatic featDelRegPir **************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegPir(void) {

  if (!FeatInitPir)
    return ajTrue;

  ajRegFree(&PirRegexAll) ;
  ajRegFree(&PirRegexCom) ;
  ajRegFree(&PirRegexLoc) ;
  ajRegFree(&PirRegexPos) ;

  ajStrTableFree(&FeatTypeTablePir);
  ajStrTableFree(&FeatTagsTablePir);

  FeatInitPir = ajFalse;
  return ajTrue;
}

/* @funcstatic featDelRegSwiss ************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegSwiss(void) {

  if (!FeatInitSwiss)
    return ajTrue;

  ajRegFree(&SwRegexComment) ;
  ajRegFree(&SwRegexFtid) ;
  ajRegFree(&SwRegexNew) ;
  ajRegFree(&SwRegexNext) ;

  ajStrTableFree(&FeatTypeTableSwiss);
  ajStrTableFree(&FeatTagsTableSwiss);

  FeatInitSwiss = ajFalse;
  return ajTrue;
}

/* @funcstatic featDelRegGff **************************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegGff(void)
{

  if (!FeatInitGff)
    return ajTrue;

    /* Clean-up any global static runtime resources here
       for example, regular expression pattern variables */

  ajRegFree(&GffRegexNumeric) ;
  ajRegFree(&GffRegexblankline) ;
  ajRegFree(&GffRegexversion) ;
  ajRegFree(&GffRegexdate) ;
  ajRegFree(&GffRegexregion) ;
  ajRegFree(&GffRegexcomment) ;
  ajRegFree(&GffRegexTvTagval) ;

  ajStrTableFree(&FeatTypeTableGff);
  ajStrTableFree(&FeatTagsTableGff);

  FeatInitGff = ajFalse;

  return ajTrue;
}

/* @func ajFeattableWriteDdbj *************************************************
**
** Write a feature table in DDBJ format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteDdbj (AjPFeattable thys, AjPFile file) {
  return feattableWriteEmbl(thys,file,ajFalse);
}

/* @func ajFeattableWriteEmbl *************************************************
**
** Write a feature table in EMBL format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteEmbl (AjPFeattable thys, AjPFile file) {
  return feattableWriteEmbl(thys,file,ajTrue);
}

/* @func ajFeattableWriteGenbank **********************************************
**
** Write a feature table in GenBank format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteGenbank (AjPFeattable thys, AjPFile file) {
  return feattableWriteEmbl(thys,file,ajFalse);
}

/* @funcstatic feattableWriteEmbl *********************************************
**
** Write a feature table in EMBL format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @param [r] IsEmbl [AjBool] ajTrue for EMBL (different line prefix)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool feattableWriteEmbl (AjPFeattable thys, AjPFile file,
				  AjBool IsEmbl) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;
  AjPFeature gfprev   = NULL ;
  AjBool join=ajFalse;
  AjBool whole = ajFalse;           /* has "complement(" been added */
  AjPStr location = NULL;        /* location list as a string */
  AjPStr temp=NULL;
  AjPStr pos=NULL;
  ajint oldgroup = -1;

  /* Check arguments */

  ajDebug ("feattableWriteEmbl Checking arguments\n");
  assert (file);

  ajFeattableSetDna (thys);

  /* feature table heading */

  if (IsEmbl) {
    ajFmtPrintF (file, "FH   Key             Location/Qualifiers\n");
    ajFmtPrintF (file, "FH\n");
  }
  else {
    ajFmtPrintF (file, "FEATURES             Location/Qualifiers\n");
  }

  location = ajStrNewL(80);
  temp = ajStrNewL(80);
  pos = ajStrNewL(80);

  /* For all features... we need to process a group at a time */

  ajListSort(thys->Features,*featCompByGroup);

  if (thys->Features) {
    iter = ajListIter(thys->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;

      if((oldgroup != gf->Group) && gfprev){ /* previous location ready */
	if(join) {
	  ajStrAppC(&location,")");      /* close bracket for join */
	  ajDebug("join: closing ')' appended\n");
	}
	if(whole){
	  ajStrInsertC(&location,0,"complement(");
	  ajStrAppC(&location,")");
	  ajDebug("wrap with complement(), reset whole %b to N\n", whole);
	  whole = ajFalse;
	}

	ajDebug("calling featDumpEmbl for gfprev\n");
	ajDebug("location: '%S'\n", location);
	featDumpEmbl(gfprev,location, file, IsEmbl) ; /* gfprev has tag data */

	/* reset the values from previous */
	ajDebug("reset location\n");
	ajStrClear(&location);
        ajDebug("reset join  %b to N\n", join);
	join = ajFalse;
      }

      oldgroup = gf->Group;

      /* process the new gf */

      ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	      gf->Type, gf->Group,gf->Exon, gf->Flags, ajListLength(gf->Tags));

      if(gf->Flags & FEATFLAG_COMPLEMENT_MAIN) {
	ajDebug("set2 whole %b to Y\n", whole);
	whole =ajTrue;
      }

      if(ajStrLen(location)) { /* one location already there */
	if(!join){
          ajDebug("insert 'join(', set join Y\n");
	  if (gf->Flags & FEATFLAG_GROUP)
	    ajStrInsertC(&location,0,"group(");
	  else if (gf->Flags & FEATFLAG_ORDER)
	    ajStrInsertC(&location,0,"order(");
	  else if (gf->Flags & FEATFLAG_ONEOF)
	    ajStrInsertC(&location,0,"one_of(");
	  else
	    ajStrInsertC(&location,0,"join(");
	  join = ajTrue;
	}
	ajStrAppC(&location,",");
        ajDebug("append ','\n");
      }

      ajStrClear(&temp);
      ajStrClear(&pos);

      if(gf->Flags & FEATFLAG_REMOTEID){
	ajFmtPrintAppS(&pos,"%S:",gf->Remote);
        ajDebug("remote: %S\n", gf->Remote);
      }

      if(gf->Flags & FEATFLAG_LABEL){
	ajFmtPrintAppS(&pos,"%S",gf->Label);
        ajDebug("label: %S\n", gf->Label);
       }
      else if(gf->Flags & FEATFLAG_START_BEFORE_SEQ){
	ajFmtPrintAppS(&pos,"<%d",gf->Start);
        ajDebug("<start\n");
      }
      else if(gf->Flags & FEATFLAG_START_TWO){
	ajFmtPrintAppS(&pos,"(%d.%d)",gf->Start,gf->Start2);
        ajDebug("start2 (%d.%d)\n", gf->Start, gf->Start2);
     }
      else{
	ajFmtPrintAppS(&pos,"%d",gf->Start);
        ajDebug("start\n");
      }

      if(!(gf->Flags & FEATFLAG_POINT)){
	if(gf->Flags & FEATFLAG_BETWEEN_SEQ){
	  ajFmtPrintAppS(&pos,"^%d",gf->End);
          ajDebug("between ^end\n");
	}
	else if(gf->Flags & FEATFLAG_END_AFTER_SEQ){
	  ajFmtPrintAppS(&pos,"..>%d",gf->End);
	  ajDebug(">end\n");
	}
	else if(gf->Flags & FEATFLAG_END_TWO){
	  ajFmtPrintAppS(&pos,"..(%d.%d)",gf->End2,gf->End);
          ajDebug("end2 (%d.%d)\n", gf->End2, gf->End);
	}
	else{
	  ajFmtPrintAppS(&pos,"..%d",gf->End);
	  ajDebug(".. end\n");
	}
      }

      if(gf->Strand == '-' && !whole){
	ajStrAssC(&temp,"complement(");
	ajStrApp(&temp,pos);
	ajStrAppC(&temp,")");
	ajDebug("strand '-', wrap exon with complement()\n");
      }
      else {
	ajStrAss(&temp,pos);
	ajDebug("simple exon\n");
      }
      ajStrClear(&pos);
      ajStrApp(&location,temp);
      if (!(gf->Flags & FEATFLAG_CHILD)) /* this is the parent/only feature */
	gfprev=gf;
    }

    ajListIterFree(iter);

    /* Don't forget the last one !!! */
    if(join) {
      ajStrAppC(&location,")");      /* close bracket for join */
      ajDebug("last: join: closing ')' appended\n");
    }
    if(whole){
      ajStrInsertC(&location,0,"complement(");
      ajStrAppC(&location,")");
      ajDebug("lest: wrap with complement(), reset whole %b to N\n", whole);
      whole = ajFalse;
    }

    ajDebug("last: calling featDumpEmbl for gfprev\n");
    ajDebug("location: '%S'\n", location);

    featDumpEmbl(gfprev,location, file, IsEmbl) ; /* gfprev has tag data */
    ajStrDel(&location);
    ajStrDel(&pos);
    ajStrDel(&temp);
  }

  ajDebug ("ajFeattableWriteEmbl Done\n");

  return ajTrue;
}

/* @funcstatic ajFeattableWriteUnknown ****************************************
**
** Write a feature table in 'unknown' format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool ajFeattableWriteUnknown (AjPFeattable thys, AjPFile file) {
  assert (file);
  (void) ajFmtPrintF (file, "Unknown feature format hence no output."
		      "Except this line!!\n");

  return ajFalse;
}

/* @func ajFeattableWriteSwiss ************************************************
**
** Write a feature table in SwissProt format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWriteSwiss (AjPFeattable thys, AjPFile file) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;
  AjPFeature gftop = NULL;
  ajint oldgroup = -1;

  /* Check arguments */
  ajDebug ("ajFeattableWriteSwiss Checking arguments\n");
  assert (file);

  ajFeattableSetProt (thys);

  /* no FH header in SwissProt */

  /* For all features... */

  if (thys->Features) {
    iter = ajListIter(thys->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      if (oldgroup != gf->Group) {
	oldgroup = gf->Group;
	gftop = gf;
      }
      else {
      if (!(gf->Flags & FEATFLAG_CHILD)) /* this is the parent/only feature */
	gftop = gf;
      }
      featDumpSwiss(gf, file, gftop) ;
    }
    ajListIterFree(iter) ;
  }

  return ajTrue ;
}

/* @func ajFeattableWritePir **************************************************
**
** Write a feature table in PIR format.
**
** @param [r] thys [AjPFeattable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeattableWritePir (AjPFeattable thys, AjPFile file) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;
  AjPFeature gfprev   = NULL ;
  AjPStr location = NULL;        /* location list as a string */
  AjPStr temp=NULL;
  AjPStr pos=NULL;
  ajint oldgroup = -1;

  /* Check arguments */

  ajDebug ("ajFeattableWritePir Checking arguments\n");
  assert (file);

  ajFeattableSetProt (thys);

  location = ajStrNewL(80);
  temp = ajStrNewL(80);
  pos = ajStrNewL(80);

  /* For all features... we need to process a group at a time */

  ajListSort(thys->Features,*featCompByGroup);

  if (thys->Features) {
    iter = ajListIter(thys->Features) ;

    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;

      if((oldgroup != gf->Group) && gfprev){ /* previous location ready */
	ajDebug("calling featDumpPir for gfprev\n");
	ajDebug("location: '%S'\n", location);
	featDumpPir(gfprev, location, file) ; /* gfprev has tag data */

	/* reset the values from previous */
	ajDebug("reset location\n");
	ajStrClear(&location);
      }

      oldgroup = gf->Group;

      /* process the new gf */

      ajDebug("\n'%S' group: %d exon: %d flags:%x tags: %d\n",
	      gf->Type, gf->Group,gf->Exon, gf->Flags, ajListLength(gf->Tags));

      if(ajStrLen(location)) { /* one location already there */
	ajStrAppC(&location,",");
        ajDebug("append ','\n");
      }

      ajStrClear(&temp);
      ajStrClear(&pos);

      ajFmtPrintAppS(&pos,"%d",gf->Start);
      ajDebug("start\n");

      if (gf->End != gf->Start) {
	ajFmtPrintAppS(&pos,"-%d",gf->End);
      }

      ajStrAss(&temp,pos);

      ajStrClear(&pos);
      ajStrApp(&location,temp);
      if (!(gf->Flags & FEATFLAG_CHILD)) /* this is the parent/only feature */
	gfprev=gf;
    }

    ajListIterFree(iter);

    /* Don't forget the last one !!! */

    ajDebug("last: calling featDumpPir for gfprev\n");
    ajDebug("location: '%S'\n", location);

    featDumpPir(gfprev,location, file) ; /* gfprev has tag data */
    ajStrDel(&location);
    ajStrDel(&pos);
    ajStrDel(&temp);
  }

  ajDebug ("ajFeattableWritePir Done\n");

  return ajTrue;
}

/* @func ajFeattableGetName ***************************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table
** and is not to be destroyed.
**
** @param [r] thys [AjPFeattable] Feature table
** @return [AjPStr] Feature table name.
** @@
******************************************************************************/

AjPStr ajFeattableGetName (AjPFeattable thys) {
  return thys->Name;
}


/* @funcstatic featFrame ******************************************************
**
** Converts a frame number in the range 0 to 3 to a GFF frame character
**
** @param [r] frame [ajint] Feature frame number
** @return [char] character for this frame in GFF
******************************************************************************/

static char featFrame (ajint frame) {

  static char framestr[] = ".012";

  if (frame < 0) return '.';
  if (frame > 3) return '.';

  return framestr[frame];
}

/* @funcstatic featStrand *****************************************************
**
** Converts a strand number to a GFF strand character. NULL characters
** are converted to '+' All other values are simply cast to character.
**
** @param [r] strand [ajint] Strand
** @return [char] GFF character for this strand.
** @@
******************************************************************************/

static char featStrand (ajint strand) {

  if (ajSysItoC(strand) != '-') return '+';

  return '-';
}

/* @func ajFeattableIsNuc *****************************************************
**
** Returns ajTrue if a feature table is knucleotide protein
**
** @param [r] thys [AjPFeattable] Feature table
** @return [AjBool] ajTrue for a protein feature table
** @@
******************************************************************************/

AjBool ajFeattableIsNuc (AjPFeattable thys) {

    if (ajStrMatchC(thys->Type, "N"))
	return ajTrue;

    if (ajFeattableIsProt(thys))
	return ajFalse;

    return ajFalse;
}

/* @func ajFeattableIsProt ****************************************************
**
** Returns ajTrue if a feature table is protein
**
** @param [r] thys [AjPFeattable] Feature table
** @return [AjBool] ajTrue for a protein feature table
** @@
******************************************************************************/

AjBool ajFeattableIsProt (AjPFeattable thys) {

    if (ajStrMatchC(thys->Type, "P"))
	return ajTrue;

    if (ajFeattableIsNuc(thys))
	return ajFalse;

    return ajFalse;
}

/* @func ajFeattableBegin *****************************************************
**
** Returns the feature table start position, or 1 if no start has been set.
**
** @param [P] thys [AjPFeattable] feature table object
** @return [ajint] Start position.
** @@
******************************************************************************/

ajint ajFeattableBegin (AjPFeattable thys)
{
    if (!thys->Start)
	return 1;

    return ajFeattablePos(thys, thys->Start);
}

/* @func ajFeattableEnd *******************************************************
**
** Returns the features table end position, or the feature table length if
** no end has been set.
**
** @param [P] thys [AjPFeattable] feature table object
** @return [ajint] End position.
** @@
******************************************************************************/

ajint ajFeattableEnd (AjPFeattable thys)
{
    if (!thys->End)
	return (ajFeattableLen(thys));

    return ajFeattablePosI(thys, ajFeattableBegin(thys), thys->End);
}

/* @func ajFeattableLen *******************************************************
**
** Returns the sequence length of a feature table
**
** @param [r] thys [AjPFeattable] Feature table
** @return [ajint] Length in bases or residues
** @@
******************************************************************************/

ajint ajFeattableLen (AjPFeattable thys) {
  if (!thys) return 0;
  return (thys->Len);
}

/* @func ajFeattableSize ******************************************************
**
** Returns the number of features in a feature table
**
** @param [r] thys [AjPFeattable] Feature table
** @return [ajint] Number of features
** @@
******************************************************************************/

ajint ajFeattableSize (AjPFeattable thys) {
  if (!thys) return 0;
  return ajListLength (thys->Features);
}

/* @func ajFeattabInClear *****************************************************
**
** Clears a Tabin input object back to "as new" condition, except
** for the USA list which must be preserved.
**
** @param [P] thys [AjPFeattabIn] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajFeattabInClear (AjPFeattabIn thys)
{

  ajDebug ("ajFeattabInClear called\n");

  (void) ajStrClear(&thys->Ufo);
  (void) ajStrClear(&thys->Seqname);
  (void) ajStrClear(&thys->Formatstr);
  (void) ajStrClear(&thys->Filename);
  (void) ajStrClear(&thys->Entryname);
  (void) ajStrClear(&thys->Type);
  ajFileBuffDel(&thys->Handle);
  if (thys->Handle)
    ajFatal("ajFeattabInClear did not delete Handle");

  return;
}


/* @func ajFeatLocToSeq *******************************************************
**
** Returns a sequence entry from a feature location
**
** @param [r] seq [AjPStr] sequence
** @param [r] line [AjPStr] location
** @param [w] res [AjPStr*] sequence construct
** @param [r] usa [AjPStr] usa of query
** @return [AjBool] true on success
** @@
******************************************************************************/

AjBool ajFeatLocToSeq(AjPStr seq, AjPStr line, AjPStr *res, AjPStr usa)
{
    static AjPStr str=NULL;
    char *p;
    char *q;

    ajint len;
    ajint i;
    ajint off;
    static AjPStr token=NULL;
    static AjPStr tmp=NULL;
    static AjPStr ent=NULL;

    AjPRegexp exp_ndotn = NULL;
    AjPRegexp exp_brnbr = NULL;
    AjPRegexp exp_compbrndashnbr = NULL;
    AjPRegexp exp_joinbr = NULL;
    AjPStrTok handle = NULL;

    AjBool isglobcomp=ajFalse;
    AjBool docomp=ajFalse;
    AjBool dbentry=ajFalse;

    ajint begin=0;
    ajint end=0;

    if(!str)
    {
	token = ajStrNew();
	str   = ajStrNew();
	tmp   = ajStrNew();
	ent   = ajStrNew();
    }

    ajStrAssS(&str,line);


    /* Remove chevrons */
    p=ajStrStr(str);
    len = ajStrLen(str);
    for(i=0;i<len;++i)
    {
	if(*p=='<' || *p=='>')
	    *p=' ';
	++p;
    }

    ajStrCleanWhite(&str);

    /* Replace sites by a single location */
    p=ajStrStr(str);
    len = ajStrLen(str);
    while(*p)
    {
	if(*p=='^')
	{
	    *(p++)=' ';
	    while(*p>='0' && *p<='9')
		*(p++) = ' ';
	}
	else
	    ++p;
    }
    ajStrCleanWhite(&str);


    /* Replace any x.y with x */
    exp_ndotn = ajRegCompC("([0-9]+)[.]([0-9]+)");
    p = ajStrStr(str);
    while(ajRegExec(exp_ndotn,str))
    {
	off = ajRegOffset(exp_ndotn);
	while(p[off]!='.')
	    ++off;
	p[off++]=' ';
	while(p[off]>='0' && p[off]<='9')
	    p[off++]=' ';
    }
    ajRegFree(&exp_ndotn);
    ajStrCleanWhite(&str);

    /* Replace any (n) with n */
    exp_brnbr = ajRegCompC("[(]([0-9]+)[)]");
    p = ajStrStr(str);
    while(ajRegExec(exp_brnbr,str))
    {
	off = ajRegOffset(exp_brnbr);
	p[off++]=' ';
	while(p[off]!=')')
	    ++off;
	p[off++]=' ';
    }
    ajRegFree(&exp_brnbr);
    ajStrCleanWhite(&str);

    /* See if its a global complement and remove complement enclosure */
    if(ajStrPrefixC(str,"complement("))
    {
	p=ajStrStr(str);
	len=ajStrLen(str);
	ajStrAssSub(&str,str,11,len-2);
	isglobcomp=ajTrue;
    }

    /* Replace .. with - */
    p = ajStrStr(str);
    while(*p)
    {
	if(*p=='.' && *(p+1)=='.')
	{
	    *p='-';
	    *(p+1)=' ';
	}
	++p;
    }

    ajStrCleanWhite(&str);


    /* Replace complement(n-n) with ^n-n */
    exp_compbrndashnbr = ajRegCompC("complement[(]([A-Za-z0-9:.]+)"
				    "[-]([0-9]+)[)]");

    p = ajStrStr(str);
    while(ajRegExec(exp_compbrndashnbr,str))
    {
	off = ajRegOffset(exp_compbrndashnbr);
	for(i=0;i<10;++i)
	    p[off++]=' ';
	p[off]='^';
	while(p[off]!=')')
	    ++off;
	p[off++]=' ';
    }
    ajStrCleanWhite(&str);
    ajRegFree(&exp_compbrndashnbr);


    /* Check for only one "join" */
    p = ajStrStr(str);
    exp_joinbr = ajRegCompC("join[(]");
    i=0;
    while(ajRegExec(exp_joinbr,str))
    {
	off = ajRegOffset(exp_joinbr);
	++i;
	if(off)
	{
	    ajWarn("Too many joins");
	    return ajFalse;
	}
	p=ajStrStr(str);
	len=ajStrLen(str);
	ajStrAssSub(&str,str,5,len-2);
    }
    ajRegFree(&exp_joinbr);


    /* Construct the sequence */
    ajStrAssC(res,"");
    handle = ajStrTokenInit(str,",");
    while(ajStrToken(&token,&handle,NULL))
    {
	p = ajStrStr(token);
	if(*p=='^')
	{
	    ++p;
	    docomp=ajTrue;
	}
	else
	    docomp=ajFalse;

	q=p;
	dbentry = ajFalse;
	while(*q)
	    if(*(q++)==':')
	    {
		dbentry = ajTrue;
		break;
	    }

	if(dbentry)
	{
	    if(*ajStrStr(token)=='^')
		ajStrAssC(&token,ajStrStr(token)+1);
	    if(!featGetUsaSection(&ent,token,&begin,&end,usa))
	    {
		ajWarn("Couldn't find embedded entry %S\n",token);
		return ajFalse;
	    }
	    ajStrAssSubC(&tmp,ajStrStr(ent),--begin,--end);
	}
	else
	{
	    if(sscanf(p,"%d-%d",&begin,&end)!=2)
	    {
		if(*p>='0' && *p<='9')
		{
		    if(sscanf(p,"%d",&begin)==1)
			end=begin;
		    else
		    {
			ajWarn("LocToSeq: Unpaired range");
			return ajFalse;
		    }
		}
	    }
	    ajStrAssSubC(&tmp,ajStrStr(seq),--begin,--end);
       }


	if(docomp)
	    ajSeqReverseStr(&tmp);
	ajStrAppC(res,ajStrStr(tmp));
    }

    if(isglobcomp)
	ajSeqReverseStr(res);

    return ajTrue;
}

/* @func ajFeatGetLocs ********************************************************
**
** Returns location information from catenated sequence entry
**
** @param [r] str [AjPStr] catenated (seq->TextPtr) entry
** @param [w] cds [AjPStr**] array of locations
** @param [r] type [char*] type (e.g. CDS/mrna)

** @return [ajint] number of location lines
** @@
******************************************************************************/

ajint ajFeatGetLocs(AjPStr str, AjPStr **cds, char *type)
{
    AjPStr *entry=NULL;
    ajint nlines=0;
    ajint i=0;
    ajint ncds=0;
    ajint nc=0;
    char *p=NULL;
    AjPStr test=NULL;

    test = ajStrNew();
    ajFmtPrintS(&test,"     %s",type);

    nlines = ajStrListToArray(str, &entry);

    for(i=0;i<nlines;++i)
    {
	if(ajStrPrefixC(entry[i],"FT "))
	{
	    p = ajStrStr(entry[i]);
	    *p = *(p+1) = ' ';
	}

	if(ajStrPrefix(entry[i],test))
	    ++ncds;
    }


    if(ncds)
    {
	AJCNEW0(*cds,ncds);
	for(i=0;i<ncds;++i)
	    (*cds)[i] = ajStrNew();
    }


    for(nc=i=0;nc<ncds;++nc)
    {
	if(ajStrPrefixC(entry[i],"FT "))
	{
	    p = ajStrStr(entry[i]);
	    *p = *(p+1) = ' ';
	}

	while(!ajStrPrefix(entry[i],test))
	    ++i;

	ajStrAssC(&(*cds)[nc],ajStrStr(entry[i++])+21);
	while( *(p=ajStrStr(entry[i]))==' ')
	{
	    if(*(p+21)=='/' || *(p+5)!=' ')
		break;
	    ajStrAppC(&(*cds)[nc],p+21);
	    ++i;
	}
	ajStrCleanWhite(&(*cds)[nc]);
    }


    for(i=0;i<nlines;++i)
	ajStrDel(&entry[i]);
    AJFREE(entry);

    ajStrDel(&test);

    return ncds;
}

/* @func ajFeatGetNote ********************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [AjPFeature] Feature object
** @param [r] name [AjPStr] Tag name
** @param [r] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetNote (AjPFeature thys, AjPStr name, AjPStr* val) {

  return ajFeatGetNoteI (thys, name, 0, val);

}

/* @func ajFeatGetNoteI *******************************************************
**
** Finds a named note tag (with a * prefix)
**
** @param [r] thys [AjPFeature] Feature object
** @param [r] name [AjPStr] Tag name
** @param [r] count [ajint] Tag count: zero for any, 1 for first, 2 for second
** @param [r] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetNoteI (AjPFeature thys, AjPStr name, ajint count,
		       AjPStr* val) {

  AjIList iter = NULL;
  FeatPTagval    item = NULL ;
  ajint icount=0;

  ajDebug("ajFeatGetNote '%S'\n", name);
  if (thys->Tags) {
    iter = ajListIter(thys->Tags);
    while (ajListIterMore(iter)) {

      item = (FeatPTagval)ajListIterNext (iter);
      ajDebug("  try /%S=\"%S\"\n", item->Tag, item->Value);
      if (ajStrMatchCaseC(item->Tag, "note")) {
	if (ajStrChar(item->Value, 0) == '*') {
	  ajDebug("  testing *name\n");
	  if (ajStrPrefixCaseCO(ajStrStr(item->Value)+1, name)) {
	    icount++;
	    ajDebug("  found [%d] '%S'\n", icount, name);
	    if (icount >= count) {
	      if (ajStrLen(item->Value) > (ajStrLen(name)+1)) {
		ajStrAssC (val, ajStrStr(item->Value)+ajStrLen(name)+2);
	      }
	      else {		/* no value */
		ajStrAssC (val, "");
	      }
	      ajListIterFree(iter);
	      return ajTrue;
	    }
	  }
	}
      }
    }
  }

  ajStrDel(val);
  ajListIterFree(iter);
  return ajFalse;

}



/* @func ajFeatGetTag *********************************************************
**
** Returns the nth value of a named feature tag.
**
** @param [r] thys [AjPFeature] Feature object
** @param [r] name [AjPStr] Tag name
** @param [r] num [ajint] Tag number
** @param [r] val [AjPStr*] Tag value (if found)
**
** @return [AjBool] ajTrue on success (feature tag found)
** @@
******************************************************************************/

AjBool ajFeatGetTag (AjPFeature thys, AjPStr name, ajint num, AjPStr* val) {

  AjIList iter = NULL;
  FeatPTagval    item = NULL ;
  ajint inum=0;

  if (thys->Tags) {
    iter = ajListIter(thys->Tags);
    while (ajListIterMore(iter)) {

      item = (FeatPTagval)ajListIterNext (iter);
      if (ajStrMatchCase(item->Tag, name)) {
	inum++;
	if (num == inum) {
	  ajStrAssS (val, item->Value);
	  ajListIterFree(iter);
	  return ajTrue;
	}
      }
    }
  }

  ajStrDel(val);
  ajListIterFree(iter);
  return ajFalse;

}

/* @func ajFeatGetType ********************************************************
**
** Returns the type (key) of a feature object. This is a copy of the
** pointer to the type, and is still owned by the feature
** and is not to be destroyed.
**
** @param [r] thys [AjPFeature] Feature object
**
** @return [AjPStr] Feature type, read only
** @@
******************************************************************************/

AjPStr ajFeatGetType (AjPFeature thys) {
  return thys->Type;
}

/* @func ajFeatGetStart *******************************************************
**
** Returns the start position of a feature object.
**
** @param [r] thys [AjPFeature] Feature object
**
** @return [ajint] Feature start position
** @@
******************************************************************************/

ajint ajFeatGetStart (AjPFeature thys) {
  return thys->Start;
}

/* @func ajFeatGetEnd *********************************************************
**
** Returns the end position of a feature object.
**
** @param [r] thys [AjPFeature] Feature object
**
** @return [ajint] Feature end position
** @@
******************************************************************************/

ajint ajFeatGetEnd (AjPFeature thys) {
  return thys->End;
}

/* @func ajFeatGetForward *****************************************************
**
** Returns the direction of a feature object.
**
** @param [r] thys [AjPFeature] Feature object
**
** @return [AjBool] ajTrue for a forward direction, ajFalse for reverse
** @@
******************************************************************************/

AjBool ajFeatGetForward (AjPFeature thys) {
  if (ajSysItoC(thys->Strand) != '-') return ajTrue;

  return ajFalse;
}

/* @func ajFeatGetFrame *******************************************************
**
** Returns the reading frame of a feature object.
**
** @param [r] thys [AjPFeature] Feature object
**
** @return [ajint] Feature reading frame (zero for undefined)
** @@
******************************************************************************/

ajint ajFeatGetFrame (AjPFeature thys) {
  return thys->Frame;
}

/* @func ajFeatGetTrans *******************************************************
**
** Returns translation information from catenated sequence entry
**
** @param [r] str [AjPStr] catenated (seq->TextPtr) entry
** @param [w] cds [AjPStr**] array of translations
**
** @return [ajint] number of location lines
** @@
******************************************************************************/

ajint ajFeatGetTrans(AjPStr str, AjPStr **cds)
{
    AjPStr *entry=NULL;
    ajint nlines=0;
    ajint i=0;
    ajint ncds=0;
    ajint nc=0;
    char *p=NULL;
    static AjPRegexp exp_tr=NULL;


    nlines = ajStrListToArray(str, &entry);

    exp_tr = ajRegCompC("/translation=");

    for(i=0;i<nlines;++i)
    {
	if(ajStrPrefixC(entry[i],"FT "))
	{
	    p = ajStrStr(entry[i]);
	    *p = *(p+1) = ' ';
	}

	if(ajRegExec(exp_tr,entry[i]))
	    ++ncds;
    }


    if(ncds)
    {
	AJCNEW0(*cds,ncds);
	for(i=0;i<ncds;++i)
	    (*cds)[i] = ajStrNew();
    }


    for(nc=i=0;nc<ncds;++nc)
    {
	if(ajStrPrefixC(entry[i],"FT "))
	{
	    p = ajStrStr(entry[i]);
	    *p = *(p+1) = ' ';
	}

	while(!ajRegExec(exp_tr,entry[i]))
	    ++i;

	ajStrAssC(&(*cds)[nc],ajStrStr(entry[i++])+35);
	while( *(p=ajStrStr(entry[i]))==' ')
	{
	    if(*(p+21)=='/' || *(p+5)!=' ')
		break;
	    ajStrAppC(&(*cds)[nc],p+21);
	    ++i;
	}
	p = ajStrStr((*cds)[nc]);
	p[ajStrLen((*cds)[nc])-2] = ' ';
	ajStrCleanWhite(&(*cds)[nc]);
    }


    for(i=0;i<nlines;++i)
	ajStrDel(&entry[i]);
    AJFREE(entry);

    return ncds;
}

/* @funcstatic featGetUsaSection **********************************************
**
** Returns the sequence for a USA
**
** What is this doing in ajfeat ???
**
** @param [R] thys [AjPStr*] Result
** @param [R] token [AjPStr] Token
** @param [R] begin [ajint*] Start position
** @param [R] end [ajint*] End position
** @param [R] usa [AjPStr] Usa to be processed
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featGetUsaSection(AjPStr* thys, AjPStr token, ajint* begin,
				ajint* end, AjPStr usa)
{
    AjPStrTok handle=NULL;
    AjPStrTok hand2=NULL;

    AjPStr db=NULL;
    AjPStr entry=NULL;
    AjPStr entry2=NULL;
    AjPStr numbers=NULL;
    AjBool ok=ajTrue;
    char   *p=NULL;
    AjPSeq seq=NULL;

    db      = ajStrNew();
    entry   = ajStrNew();
    entry2  = ajStrNew();
    numbers = ajStrNew();
    seq     = ajSeqNew();

    handle = ajStrTokenInit(usa,":");
    ajStrToken(&db,&handle,NULL);
    ajStrTokenClear(&handle);
    ajStrAppC(&db,":");

    handle = ajStrTokenInit(token,":");
    ajStrToken(&entry,&handle,NULL);

    hand2 = ajStrTokenInit(entry,".");
    ajStrToken(&entry2,&hand2,NULL);
    ajStrTokenClear(&hand2);

    ajStrToken(&numbers,&handle,NULL);
    ajStrTokenClear(&handle);

    p = ajStrStr(numbers);
    if(sscanf(p,"%d-%d",begin,end)!=2)
    {
	if(sscanf(p,"%d",begin)==1)
	    *end=*begin;
	else
	    ok = ajFalse;
    }

    ajStrApp(&db,entry2);

    if(!ajSeqGetFromUsa(db,0,&seq))
	ok = ajFalse;

    ajStrAssC(thys,ajSeqChar(seq));

    ajStrDel(&db);
    ajStrDel(&entry);
    ajStrDel(&entry2);
    ajStrDel(&numbers);
    ajSeqDel(&seq);

    return ok;
}

/*========================================================================
======================= NEW FUNCTIONS ====================================
========================================================================*/

/* @func ajFeatTest ***********************************************************
**
** Temporary testing function for feature handling
** to be deleted when all is working happily
**
** @return [void]
** @@
******************************************************************************/

void ajFeatTest (void) {
  AjPFeattable table=NULL;
  AjPStr desc=NULL;
  AjPStr source=NULL;
  AjPStr type=NULL;
  AjPFeature ft=NULL;

  featInit();
  table = ajFeattableNew(NULL);

  ajStrAssC(&source, "testft");
  ajStrAssC(&type, "misc_feature");
  ajStrAssC(&desc, "Just testing");

  ft = ajFeatNew(table, source, type, 5, 7, 1.23, '+', 0);
  ajFeatSetDesc(ft, desc);

  ajStrAssC(&desc, "Testing again");
  ft = ajFeatNew(table, source, type, 9, 19, 4.56, '-', 3);
  ajFeatSetDesc(ft, desc);

  ajFeattableTrace(table);

  ajFeattableDel(&table);
  ajStrDel (&desc);
  ajStrDel (&source);
  ajStrDel (&type);

  return;
}

/* @funcstatic featInit *******************************************************
**
** Initialises everything needed for feature handling
**
** @return [void]
** @@
******************************************************************************/

static void featInit (void) {

  if (FeatInitDone)
    return;

  FeatInitDone = ajTrue;

  featVocabRead ("emboss", 200, 200,
		 &FeatTypeTableDna, &FeatTagsTableDna);
  featVocabRead ("protein", 200, 200,
		 &FeatTypeTableProtein, &FeatTagsTableProtein);

  ajDebug ("Tables internal (Dna, Prot) Type: %x %x Tags: %x %x\n",
	   FeatTypeTableDna, FeatTypeTableProtein,
	   FeatTagsTableDna, FeatTagsTableProtein);


  return;
}

/* @funcstatic featVocabRead **************************************************
**
** Reads the possible feature types (keys) and tags (qualifiers)
** from files.
**
** @param [R] name [char*] Feature format
** @param [R] typsize [ajint] Estimated number of types
** @param [R] tagsize [ajint] Estimated number of tags
** @param [R] pTypeTable [AjPTable*] Feature type table
** @param [R] pTagsTable [AjPTable*] Feature tags table
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featVocabRead (char* name, ajint typsize, ajint tagsize,
			     AjPTable* pTypeTable, AjPTable* pTagsTable) {

  AjPFile TagsFile=NULL;
  AjPFile TypeFile=NULL;
  AjPStr line = NULL;
  AjPStr tagname = NULL;
  AjPStr tagtype = NULL;
  AjPStr tag = NULL;
  AjPStr req = NULL;
  AjPStr type=NULL;
  AjPStr tmpstr=NULL;
  AjPStr token=NULL;
  AjPStr savetype = NULL;
  AjPStr tagstr = NULL;
  AjPStr typtagstr = NULL;
  AjPStr defname = NULL;

  AjPStr TagsFName=NULL;
  AjPStr TypeFName=NULL;
  static AjPRegexp ValExp =  NULL;
  static AjPRegexp TagExp = NULL;
  static AjPRegexp VocabExp = NULL;

  ajint numtype = -1;
  ajint typecount = 0;
  ajint tagscount = 0;
  ajint linecount = 0;
  ajint i;
  ajint itypsize = 200;
  ajint itagsize = 200;

  char* TagType[] = {
    "QTEXT",			/* quoted text */
    "BTEXT",			/* bracketed text */
    "TEXT",			/* unquoted, no white space */
    "VOID",			/* no value */
    "LIMITED",			/* limited vocabulary */
    "QLIMITED",			/* limited vocabulary, with quotes */
    "SPECIAL",			/* special formatting */
    "QSPECIAL",			/* special formatting, with quotes */
    "TAGVAL",			/* /type=value for swissprot */
    NULL
  };

  if (*pTypeTable && *pTagsTable) return ajTrue;

  if (!ValExp)
    ValExp = ajRegCompC("([^ \t]+) +([^ \t]+)") ;
  if (!TagExp)
    TagExp = ajRegCompC("(([mM])?/([^ \t]+))|([^/ \t]+)") ;
  if (!VocabExp)
    VocabExp = ajRegCompC("([^\", \t]+)") ;

  if (typsize > 0)
    itypsize = typsize;
  if (tagsize > 0)
    itagsize = tagsize;

  if (!*pTypeTable)
    *pTypeTable = ajStrTableNewCase(itypsize);
  if (!*pTagsTable)
    *pTagsTable = ajStrTableNewCase(itagsize);

  /* First read in the list of all possible tags */

  ajFmtPrintS (&TagsFName, "Etags.%s", name);
  ajDebug("featVocabRead '%S'\n", TagsFName);

  ajFileDataNew (TagsFName, &TagsFile);
  if(!TagsFile){
    ajErr("Unable to read data file '%S'\n", TagsFName);
    return ajFalse;
  }

  tagscount =0;
  linecount=0;
  while(ajFileReadLine(TagsFile,&line)){
    linecount++;

    if(ajStrLen(line) && ajStrNCmpC(line,"#",1)) { /* skip comments */
      if (ajRegExec(ValExp,line)){
	numtype=-1;
	tagname = NULL;		/* create a new tag */
	ajRegSubI (ValExp, 1, &tagname) ;
	ajRegSubI (ValExp, 2, &tagtype) ;

	for (i=0; TagType[i]; i++) {

	  if(!ajStrCmpC(tagtype,TagType[i])) {
	    numtype = i;
	    break;
	  }
	}
	if (numtype < 0) {
	  ajDebug ("Bad feature tag type '%S' in %F line %d\n",
		 tagtype, TagsFile, linecount);
	  ajErr ("Bad feature tag type '%S' in %F line %d",
		 tagtype, TagsFile, linecount);
	  break;
	}
	ajStrDel(&tagtype);

	tagscount++;
	if (tagscount == 1) {	/* save first tag as the default */
	  tagstr = NULL;
	  ajStrAssC(&defname, "");
	  ajStrAssS(&tagstr, tagname);
	  if (ajTablePut (*pTagsTable, defname, tagstr))
	    ajErr("Etags.%s duplicate tag for '%S'", name, defname);
	  tagstr = NULL;
	  defname = NULL;
	}
	tagstr = NULL;
	ajFmtPrintS (&tagstr, "%s;", TagType[numtype]);

	/*
	** Controlled vocabulary :
	** read the list of valid values
	*/

	if(ajStrMatchCaseCC(TagType[numtype], "LIMITED") ||
	   ajStrMatchCaseCC(TagType[numtype], "QLIMITED")){
	  (void) ajRegPost(ValExp,&tmpstr);

	  while(ajRegExec(VocabExp, tmpstr)){
	    ajRegSubI (VocabExp, 1, &token) ;
	    ajFmtPrintAppS(&tagstr, "%S;", token);
	    (void) ajRegPost(VocabExp,&tmpstr);
	  }
	  ajStrDelReuse(&tmpstr);
	}

	if (ajTablePut (*pTagsTable, tagname, tagstr))
	  ajErr("Etags.%s duplicate tag for '%S'", name, tagname);
	tagstr = NULL;
	tagname = NULL;
      }
      else {
	ajDebug ("** line format bad **\n%S", line);
      }
    }
  }
  ajFileClose(&TagsFile);


  /* Efeatures file
  ** format: featuretype
  **            M/mandatorytag
  **             /tag
  **             /tag
  **
  ** All tags must be defined in the Etags file (read earlier into *pTagsTable)
  */

  ajFmtPrintS (&TypeFName, "Efeatures.%s", name);
  ajDebug("Trying to open %S...\n",TypeFName);
  ajFileDataNew(TypeFName,&TypeFile);
  if(!TypeFile){
    ajErr("Unable to read data file '%S'\n", TagsFName);
    ajRegFree(&ValExp);
    ajRegFree(&TagExp);
    ajRegFree(&VocabExp);
    return ajFalse;
  }

  typecount =0;
  while(ajFileReadLine(TypeFile,&line)){
    if(ajStrNCmpC(line,"#",1)){ /* if a comment skip it */
      if(ajRegExec(TagExp,line)){
	ajRegSubI (TagExp, 2, &req) ;    /* get the mandatory code */
	ajRegSubI (TagExp, 3, &tag) ;    /* and get the tag ... */
	ajRegSubI (TagExp, 4, &type) ;    /* ... or, get the type */

	if (ajStrLen(type)) {	/* new feature type */
	  typecount++;
	  if (typecount == 1) {	/* first type saved as "" default */
	    defname = NULL;
	    typtagstr = NULL;
	    ajStrAssC(&defname, "");
	    ajStrAssS(&typtagstr, type);
	    if (ajTablePut (*pTypeTable, defname, typtagstr))
	      ajErr("Efeatures.%s duplicate tag for '%S'", name, defname);
	    typtagstr = NULL;
	  }
	  else {		/* save the previous feature type */
	    if (ajTablePut (*pTypeTable, savetype, typtagstr))
	      ajErr("Efeatures.%s duplicate tag for '%S'", name, savetype);
	    typtagstr = NULL;
	  }

	  /*
	  ** set up new feature type and type-tag strings
	  ** ready to save the details
	  */

	  typtagstr = ajStrNewCL(";", 256);
	  savetype = type;
	  type = NULL;
	}
	else {			/* tag name */
	  if (!ajTableGet(*pTagsTable, tag)) {
	    ajWarn ("%S: tag %S (feature %S) not in Etags file",
		    TypeFName, tag, savetype);
	  }
	  if (ajStrLen(req))
	    ajFmtPrintAppS (&typtagstr, "*");
	  ajFmtPrintAppS (&typtagstr, "%S;", tag);
	}

      }
    }
  }

  if (typecount > 0) {		/* save the last feature type */
    if (ajTablePut (*pTypeTable, savetype, typtagstr))
      ajErr("Efeatures.%s duplicate tag for '%S'", name, savetype);
    typtagstr = NULL;
    savetype = NULL;
  }

  ajFileClose(&TypeFile);

  ajStrDel(&line);
  ajStrDel(&token);

  line = (AjPStr) ajTableGet (*pTypeTable, ajStrNew());
  ajDebug ("Default type: '%S'\n", line);

  line = (AjPStr) ajTableGet (*pTagsTable, ajStrNew());
  ajDebug ("Default tag:  '%S'\n", line);

  /*
  ajTableTrace (*pTypeTable);
  ajTableTrace (*pTagsTable);
  ajStrTablePrint (*pTypeTable);
  ajStrTablePrint (*pTagsTable);
  */

  ajStrDel (&tmpstr);
  ajStrDel (&TypeFName);
  ajStrDel (&TagsFName);
  ajStrDel (&req);
  ajStrDel (&tag);

  return ajTrue;

}

/* @funcstatic featVocabInitEmbl **********************************************
**
** Initialises feature table internals for EMBL format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitEmbl (void) {

  return featVocabRead ("embl", 0, 0,
			&FeatTypeTableEmbl, &FeatTagsTableEmbl);

}

/* @funcstatic featVocabInitGff ***********************************************
**
** Initialises feature table internals for GFF format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitGff (void) {

  return featVocabRead ("gff", 200, 200,
			&FeatTypeTableGff, &FeatTagsTableGff);

}

/* @funcstatic featVocabInitPir ***********************************************
**
** Initialises feature table internals for PIR format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitPir (void) {

  return featVocabRead ("pir", 20, 5, &FeatTypeTablePir, &FeatTagsTablePir);

}

/* @funcstatic featVocabInitSwiss *********************************************
**
** Initialises feature table internals for SwissProt format
**
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool featVocabInitSwiss (void) {

  return featVocabRead ("swiss", 50, 5,
			&FeatTypeTableSwiss, &FeatTagsTableSwiss);

}

/* @func ajFeatSetDescApp *****************************************************
**
** Sets the description for a feature
**
** @param [r] thys [AjPFeature] Feature
** @param [r] desc [AjPStr] Feature description (simple text)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatSetDescApp (AjPFeature thys, AjPStr desc) {

  FeatPTagval tv = NULL;
  static AjPStr tagnote = NULL;

  if (!tagnote)
    ajStrAssC (&tagnote, "note");

  tv = featTagval(thys, tagnote);
  if (tv) {
    ajStrAppC (&tv->Value, ", ");
    ajStrApp (&tv->Value, desc);
  }
  else {
    ajFeatTagSet (thys, tagnote, desc);
  }

  return ajTrue;
}

/* @func ajFeatSetDesc ********************************************************
**
** Sets the description for a feature
**
** @param [r] thys [AjPFeature] Feature
** @param [r] desc [AjPStr] Feature description (simple text)
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatSetDesc (AjPFeature thys, AjPStr desc) {

  ajFeatTagSetC (thys, "note", desc);

  return ajTrue;
}

/* @func ajFeatTagSetC ********************************************************
**
** Sets a feature tag value
**
** @param [r] thys [AjPFeature] Feature
** @param [r] tag [char*] Feature tag
** @param [r] value [AjPStr] Feature tag value
** @return [AjPStr] Previous tag value
** @@
******************************************************************************/

AjPStr ajFeatTagSetC (AjPFeature thys, char* tag, AjPStr value) {

  static AjPStr tmptag = NULL;
  ajStrAssC (&tmptag, tag);
  return ajFeatTagSet (thys, tmptag, value);
}

/* @func ajFeatTagSet *********************************************************
**
** Sets a feature tag value
**
** @param [r] thys [AjPFeature] Feature
** @param [r] tag [AjPStr] Feature tag
** @param [r] value [AjPStr] Feature tag value
** @return [AjPStr] Previous tag value,
** @@
******************************************************************************/

AjPStr ajFeatTagSet (AjPFeature thys, AjPStr tag, AjPStr value) {

  FeatPTagval tv = NULL;
  static AjPStr oldvalue = NULL;
  AjPStr tmptag = NULL;		/* this comes from AjPTable */
				/* so please, please don't delete */
  static AjPStr tmpfmt = NULL;
  static AjPStr tmpval = NULL;
  static AjPStr outtag = NULL;
  char* cp;

  ajDebug ("ajFeatTagSet '%S' '%S' Prot: %B\n", tag, value, thys->Protein);

  featInit();

  if (thys->Protein) {
    tmptag = featTableTag (tag, FeatTagsTableProtein);
    featTagFmt (tmptag,  FeatTagsTableProtein, &tmpfmt);
  }
  else {
    tmptag = featTableTag (tag, FeatTagsTableDna);
    featTagFmt (tmptag,  FeatTagsTableDna, &tmpfmt);
  }

  ajStrAssS (&tmpval, value);
  ajStrAssS (&outtag, tmptag);

  cp = ajStrStr(tmpfmt);
  switch (CASE2(cp[0], cp[1])) {
  case CASE2('L','I') :
      ajDebug ("case limited\n"); /* limited */
      break;
  case CASE2('Q', 'L') :	/* limited, escape quotes */
      ajDebug ("case qlimited\n");
      break;
  case CASE2('Q', 'S') :	/* special regexp, quoted */
      ajDebug ("case qspecial\n");
      if (!featTagSpecial(&tmpval, tmptag)) {
	featTagSetDefault (thys, tmptag, value, &outtag, &tmpval);
      }
      break;
  case CASE2('S','P') :	/* special regexp */
      ajDebug ("case special\n");
      if (!featTagSpecial(&tmpval, tmptag)) {
	featTagSetDefault (thys, tmptag, value, &outtag, &tmpval);
      }
      break;
  case CASE2('T','E') :	/* no space, no quotes, wrap at margin */
      ajDebug ("case text\n");
      break;
  case CASE2('V','O') :	/* no value, so an error here */
      ajDebug ("case void\n");
      break;
  case CASE2('Q','T') :	/* escape quotes, wrap at space */
      ajDebug ("case qtext\n");
      break;
  default:
      ajWarn("Unknown internal feature tag type '%S' for '%S'",
	     tmpfmt, tmptag);
  }

  tv = featTagval (thys, outtag);
  if (tv) {			/* replace current value */
    ajStrAssS (&oldvalue, tv->Value);
    ajStrAssS (&tv->Value, tmpval);
    ajDebug ("...replaced old value '%S'\n", oldvalue);
    return oldvalue;
  }
  else {			/* new tag-value */
    tv = featTagvalNew (thys, outtag, tmpval);
    ajListPushApp (thys->Tags, tv);
    ajDebug("...new tag-value\n");
    return NULL;
  }

  return NULL;
}

/* @func ajFeatTagAddCC *******************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [r] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [const char*] Feature tag value
** @return [void]
** @@
******************************************************************************/

void ajFeatTagAddCC (AjPFeature thys, const char* tag, const char* value) {

  static AjPStr tagstr = NULL;
  static AjPStr valstr = NULL;

  ajStrAssC (&tagstr, tag);
  ajStrAssC (&valstr, tag);
  ajFeatTagAdd (thys, tagstr, valstr);

  return;
}

/* @func ajFeatTagAddC ********************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [r] thys [AjPFeature] Feature
** @param [r] tag [const char*] Feature tag
** @param [r] value [AjPStr] Feature tag value
** @return [void]
** @@
******************************************************************************/

void ajFeatTagAddC (AjPFeature thys, const char* tag, AjPStr value) {

  static AjPStr tagstr = NULL;

  ajStrAssC (&tagstr, tag);
  ajFeatTagAdd (thys, tagstr, value);

  return;
}

/* @func ajFeatTagAdd *********************************************************
**
** Sets a feature tag value, creating a new feature tag even if one
** already exists.
**
** @param [r] thys [AjPFeature] Feature
** @param [r] tag [AjPStr] Feature tag
** @param [r] value [AjPStr] Feature tag value
** @return [void]
** @@
******************************************************************************/

void ajFeatTagAdd (AjPFeature thys, AjPStr tag, AjPStr value) {

  FeatPTagval tv = NULL;
  AjPStr tmptag = NULL;		/* this comes from AjPTable */
				/* so please, please don't delete */
  static AjPStr tmpfmt = NULL;
  static AjPStr tmpval = NULL;
  static AjPStr outtag = NULL;
  char* cp;

  ajDebug ("ajFeatTagAdd '%S' '%S' Prot: %B\n", tag, value, thys->Protein);

  featInit();

  if (thys->Protein) {
    tmptag = featTableTag (tag, FeatTagsTableProtein);
    featTagFmt (tmptag,  FeatTagsTableProtein, &tmpfmt);
  }
  else {
    tmptag = featTableTag (tag, FeatTagsTableDna);
    featTagFmt (tmptag,  FeatTagsTableDna, &tmpfmt);
  }

  ajDebug("tag: '%S' format: '%S'\n", tmptag, tmpfmt);
  ajStrAssS (&tmpval, value);
  ajStrAssS (&outtag, tmptag);

  cp = ajStrStr(tmpfmt);
  switch (CASE2(cp[0], cp[1])) {
  case CASE2('L','I') :
      ajDebug ("case limited\n"); /* limited */
      break;
  case CASE2('Q', 'L') :	/* limited, escape quotes */
      ajDebug ("case qlimited\n");
      break;
  case CASE2('Q', 'S') :	/* special regexp, quoted */
      ajDebug ("case qspecial\n");
      if (!featTagSpecial(&tmpval, tmptag)) {
	featTagSetDefault (thys, tmptag, value, &outtag, &tmpval);
      }
      break;
  case CASE2('S','P') :	/* special regexp */
      ajDebug ("case special\n");
      if (!featTagSpecial(&tmpval, tmptag)) {
	featTagSetDefault (thys, tmptag, value, &outtag, &tmpval);
      }
      break;
  case CASE2('T','E') :	/* no space, no quotes, wrap at margin */
      ajDebug ("case text\n");
      break;
  case CASE2('V','O') :	/* no value, so an error here */
      ajDebug ("case void\n");
      break;
  case CASE2('Q','T') :	/* escape quotes, wrap at space */
      ajDebug ("case qtext\n");
      break;
  default:
      ajWarn("Unknown internal feature tag type '%S' for '%S'",
	     tmpfmt, tmptag);
  }

  tv = featTagvalNew (thys, outtag, tmpval);
  ajListPushApp (thys->Tags, tv);
  ajDebug("...new tag-value\n");

  return;
}

/* @funcstatic featTagSetDefault **********************************************
**
** Sets a feature tag value, using the default feature tag
**
** @param [r] thys [AjPFeature] Feature
** @param [r] tag [AjPStr] Feature tag
** @param [r] value [AjPStr] feature tag value
** @param [r] pdeftag [AjPStr*] Default tag
** @param [r] pdefval [AjPStr*] Default tag value as "*tag: value"
** @return [void]
** @@
******************************************************************************/

static void featTagSetDefault (AjPFeature thys, AjPStr tag, AjPStr value,
			       AjPStr* pdeftag, AjPStr* pdefval) {

  featInit();

  if (thys->Protein)
    featTagSetDefaultDna (tag, value, pdeftag, pdefval);
  else
    featTagSetDefaultProt (tag, value, pdeftag, pdefval);

  return;
}

/* @funcstatic featTagSetDefaultDna *******************************************
**
** Sets a feature tag value, using the default DNA feature tag
**
** @param [r] tag [AjPStr] Feature tag
** @param [r] value [AjPStr] feature tag value
** @param [r] pdeftag [AjPStr*] Default tag
** @param [r] pdefval [AjPStr*] Default tag value as "*tag: value"
** @return [void]
** @@
******************************************************************************/

static void featTagSetDefaultDna (AjPStr tag, AjPStr value,
				  AjPStr* pdeftag, AjPStr* pdefval) {

  featInit();

  ajStrAssS (pdeftag, (AjPStr) ajTableGet (FeatTagsTableDna, ajStrNew()));
  ajFmtPrintS (pdefval, "*%S: %S", tag, value);

  return;
}

/* @funcstatic featTagSetDefaultProt ******************************************
**
** Sets a feature tag value, using the default protein feature tag
**
** @param [r] tag [AjPStr] Feature tag
** @param [r] value [AjPStr] feature tag value
** @param [r] pdeftag [AjPStr*] Default tag
** @param [r] pdefval [AjPStr*] Default tag value as "*tag: value"
** @return [void]
** @@
******************************************************************************/

static void featTagSetDefaultProt (AjPStr tag, AjPStr value,
				   AjPStr* pdeftag, AjPStr* pdefval) {

  featInit();

  ajStrAssS (pdeftag, (AjPStr) ajTableGet (FeatTagsTableProtein, ajStrNew()));
  ajFmtPrintS (pdefval, "*%S: %S", tag, value);

  return;
}

/* @func ajFeattableNew *******************************************************
**
** Constructor for a new (generic) feature table.
** Does not define the feature table type.
**
** @param [R] name [AjPStr] Name for new feature table (or NULL for unnamed)
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNew( AjPStr name )
{
  AjPFeattable thys = NULL ;

  /* Allocate the object... */
  thys = featTableNewS(name) ;

  ajDebug("ajFeattableNew %x\n", thys);

  return thys ;
}

/* @func ajFeattableSetDna ****************************************************
**
** Sets the type of a feature table as DNA
**
** @param [U] thys [AjPFeattable] Feature table object
** @return [void]
** @@
**
******************************************************************************/

void ajFeattableSetDna (AjPFeattable thys)
{
  ajStrSetC (&thys->Type, "N");

  return;
}

/* @func ajFeattableSetProt ***************************************************
**
** Sets the type of a feature table as Protein
**
** @param [U] thys [AjPFeattable] Feature table object
** @return [void]
** @@
**
******************************************************************************/

void ajFeattableSetProt ( AjPFeattable thys)
{
  ajStrSetC (&thys->Type, "P");

  return;
}

/* @func ajFeattableReverse ***************************************************
**
** Reverse the features in a feature table by iterating through and
** reversing all positions and strands.
**
** @param [r] thys [AjPFeattable] Feature table object
** @return [void]
******************************************************************************/

void ajFeattableReverse  ( AjPFeattable  thys ) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  if (ajFeattableIsProt (thys)) return;

  iter = ajListIter(thys->Features) ;

  while(ajListIterMore(iter)) {
    gf = ajListIterNext (iter) ;
    ajFeatReverse(gf) ;
  }

  ajListIterFree(iter) ;

  return;
}

/* @func ajFeatReverse ********************************************************
**
** Reverse one feature by reversing all positions and strand.
**
** @param [r] thys [AjPFeature] Feature object
** @return [void]
******************************************************************************/

void ajFeatReverse  ( AjPFeature  thys ) {

  ajint itmp;

  if (thys->Strand == '-')
    thys->Strand = '+';
  else
    thys->Strand = '-';

  itmp = thys->Start;
  thys->Start = thys->End;
  thys->End = itmp;

  itmp = thys->Start2;
  thys->Start2 = thys->End2;
  thys->End2 = itmp;

  /* thys->Frame is rather hard to guess ... leave alone for now */

  thys->Frame = 0;		/* set to unknown */

  return;

}

/* @func ajFeattableSetRange **************************************************
**
** Set the begin and end range for a feature table
**
** @param [r] thys [AjPFeattable] Feature table object
** @param [r] fbegin [ajint] Begin position
** @param [r] fend   [ajint] End position
** @return [void]
******************************************************************************/

void ajFeattableSetRange  ( AjPFeattable thys,
				     ajint fbegin, ajint fend ) {

  thys->Start = ajFeattablePosI (thys, 1, fbegin);
  thys->End = ajFeattablePosII (thys->Len, thys->Start, fend);
}

/* @func ajFeattableNewDna ****************************************************
**
** Constructor for a new DNA feature table
**
** @param [R] name [AjPStr] Name for new feature table (or NULL for unnamed)
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNewDna( AjPStr name )
{
  AjPFeattable thys = NULL ;

  /* Allocate the object... */
  thys = featTableNewS(name);

  ajStrAssC (&thys->Type, "N");

  ajDebug("ajFeattableNewDna %x\n", thys);

  return thys ;
}

/* @func ajFeattableNewSeq ****************************************************
**
** Constructor for a new feature table for an existing sequence.
** The feature table type is determined by the sequence type.
**
** @param [R] seq [AjPSeq] Sequence object to provide the name and type
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNewSeq( AjPSeq seq )
{
  AjPFeattable thys = NULL ;

  /* Allocate the object... */
  /*  AJNEW0(thys) ;  deleted by AJB */

  if (ajSeqIsProt(seq))
    thys = ajFeattableNewProt(ajSeqGetName(seq)) ;
  else
    thys = ajFeattableNewDna(ajSeqGetName(seq)) ;

  return thys ;
}

/* @func ajFeattableNewProt ***************************************************
**
** Constructor for a new protein feature table
**
** @param [R] name [AjPStr] Name for new feature table (or NULL for unnamed)
** @return [AjPFeattable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
**
******************************************************************************/

AjPFeattable ajFeattableNewProt ( AjPStr name )
{
  AjPFeattable thys = NULL ;

  /* Allocate the object... */
  thys = featTableNewS(name) ;

  ajStrAssC (&thys->Type, "P");

  ajDebug("ajFeattableNewProt %x\n", thys);

  return thys ;
}

/* @funcstatic featTagvalNew **************************************************
**
** Constructor for a feature tag-value pair
**
** @param [r]   thys   [AjPFeature]   Feature
** @param [r]   tag    [AjPStr]   Tag name
** @param [r]   value  [AjPStr]   Tag value
** @return [FeatPTagval] New tag-value pair object
** @@
******************************************************************************/

static FeatPTagval featTagvalNew ( AjPFeature thys,
				   AjPStr tag, AjPStr value) {
  FeatPTagval ret;

  if (thys->Protein)
    ret = featTagvalNewProt (tag, value);
  else
    ret = featTagvalNewDna (tag, value);

  return ret;
}

/* @funcstatic featTagvalNewDna ***********************************************
**
** Constructor for a feature tag-value pair
**
** @param [r]   tag    [AjPStr]   Tag name
** @param [r]   value  [AjPStr]   Tag value
** @return [FeatPTagval] New tag-value pair object
** @@
******************************************************************************/

static FeatPTagval featTagvalNewDna ( AjPStr tag, AjPStr value) {

  FeatPTagval ret;
  AjPStr tmptag = NULL;		/* from AjPTable, don't delete */

  featInit();

  AJNEW0(ret);

  tmptag = featTableTag (tag, FeatTagsTableDna);

  ajStrAssS (&ret->Tag, tmptag);
  ajStrAssS (&ret->Value, value);

  return ret;
}

/* @funcstatic featTagvalNewProt **********************************************
**
** Constructor for a protein feature tag-value pair
**
** @param [r]   tag    [AjPStr]   Tag name
** @param [r]   value  [AjPStr]   Tag value
** @return [FeatPTagval] New tag-value pair object
** @@
******************************************************************************/

static FeatPTagval featTagvalNewProt ( AjPStr tag, AjPStr value) {

  FeatPTagval ret;
  AjPStr tmptag = NULL;		/* from AjPTable, don't delete */

  featInit();

  AJNEW0(ret);

  tmptag = featTableTag (tag, FeatTagsTableProtein);

  ajStrAssS (&ret->Tag, tmptag);
  ajStrAssS (&ret->Value, value);

  return ret;
}

/* @funcstatic featTagval *****************************************************
**
** Checks for the existence of a defined tag for a feature.
**
** @param [r]   thys [AjPFeature]  Feature object
** @param [r]   tag  [AjPStr]      Tag name
** @return [FeatPTagval] Returns the tag-value pair if found,
**                       NULL if not found.
** @@
******************************************************************************/

static FeatPTagval featTagval ( AjPFeature thys, AjPStr tag) {

  AjIList iter = NULL;
  FeatPTagval ret = NULL;
  FeatPTagval tv = NULL;

  iter = ajListIter( thys->Tags );
  while (ajListIterMore(iter)) {
    tv = ajListIterNext(iter);
    if (ajStrMatchCase (tv->Tag, tag)) {
      ajDebug ("featTagval '%S' found value '%S'\n", tag, tv->Value);
      ret = tv;
      break;
    }
  }

  ajListIterFree( iter);

  /*
    if (!ret)
      ajDebug ("featTagval '%S' not found\n", tag);
  */


  return ret;
}

/* @func ajFeattableCopy ******************************************************
**
** Makes a copy of a feature table.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   pthys  [AjPFeattable*]  Feature table copy of the original
** @param [r]   orig  [AjPFeattable]  Original feature table
** @return [void]
** @@
******************************************************************************/

void ajFeattableCopy (AjPFeattable* pthys, AjPFeattable orig) {

  AjPFeattable thys;
  AjIList iter;
  AjPFeature featorig;
  AjPFeature feat = NULL;

  ajFeattableDel(pthys);

  if (!orig) return;

  *pthys = featTableNew();

  thys = *pthys;

  ajStrAssS (&thys->Name, orig->Name);
  ajStrAssS (&thys->Type, orig->Type);
  thys->DefFormat = orig->DefFormat;
  thys->Start = orig->Start;
  thys->End = orig->End;
  thys->Len = orig->Len;
  thys->Groups = orig->Groups;

  iter = ajListIter(orig->Features);

  while (ajListIterMore(iter)) {
    featorig = ajListIterNext(iter);
    feat = NULL;
    ajFeatCopy (&feat, featorig);
    ajFeattableAdd (thys, feat);
  }
  ajListIterFree (iter);

  return;
}

/* @func ajFeatCopy ***********************************************************
**
** Makes a copy of a feature.
**
** For cases where we need a copy we can safely change and/or delete.
**
** @param [r]   pthys  [AjPFeature*]  Feature  copy of the original
** @param [r]   orig  [AjPFeature]  Original feature
** @return [void]
** @@
******************************************************************************/

void ajFeatCopy (AjPFeature* pthys, AjPFeature orig) {

  AjPFeature thys;
  AjIList iter;
  FeatPTagval tvorig;

  ajFeatDel(pthys);
  *pthys = featFeatureNew();

  thys = *pthys;

  ajStrAssS (&thys->Source, orig->Source);
  ajStrAssS (&thys->Type, orig->Type);
  ajStrAssS (&thys->Remote, orig->Remote);
  ajStrAssS (&thys->Label, orig->Label);

  thys->Protein = orig->Protein;
  thys->Start = orig->Start;
  thys->End = orig->End;
  thys->Start2 = orig->Start2;
  thys->End2 = orig->End2;
  thys->Score = orig->Score;
  thys->Strand = orig->Strand;
  thys->Frame = orig->Frame;
  thys->Flags = orig->Flags;
  thys->Group = orig->Group;
  thys->Exon = orig->Exon;

  iter = ajListIter(orig->Tags);

  while (ajListIterMore(iter)) {
    tvorig = ajListIterNext(iter);
    ajFeatTagAdd(thys, tvorig->Tag, tvorig->Value);
  }
  ajListIterFree (iter);

  return;
}

/* @func ajFeatTrace **********************************************************
**
** Traces (to the debug file) a feature object
**
** @param [r]   thys  [AjPFeature]  Feature
** @return [void]
** @@
******************************************************************************/

void ajFeatTrace (AjPFeature thys) {

  ajDebug ("  Source: '%S'\n", thys->Source);
  ajDebug ("  Type: '%S'\n", thys->Type);
  ajDebug ("  Location: %d..%d\n", thys->Start, thys->End);
  ajDebug ("  Strand: '%c'\n", thys->Strand);
  ajDebug ("  Frame: '%d'\n", thys->Frame);
  ajDebug ("  Flags: '%x'\n", thys->Flags);
  ajDebug ("  Start2: '%d'\n", thys->Start2);
  ajDebug ("  End2: '%d'\n", thys->Start2);
  ajDebug ("  RemoteId: '%S'\n", thys->Remote);
  ajDebug ("  Label: '%S'\n", thys->Label);

  ajFeatTagTrace (thys);
  return;
}

/* @func ajFeatTagTrace *******************************************************
**
** Traces (to the debug file) the tag-value pairs of a feature object
**
** @param [r]   thys  [AjPFeature]  Feature
** @return [void]
** @@
******************************************************************************/

void ajFeatTagTrace (AjPFeature thys) {

  AjIList iter;
  ajint i=0;
  FeatPTagval tv = NULL;

  iter = ajListIter (thys->Tags);
  while (ajListIterMore(iter)) {
    tv = ajListIterNext(iter);
    ajDebug (" %3d  %S : '%S'\n", ++i, tv->Tag, tv->Value);
  }
  ajListIterFree (iter);

  return;
}

/* @func ajFeatTagIter ********************************************************
**
** Returns an iterator over all feature tag-value pairs
**
** @param [r]   thys  [AjPFeature]  Feature
** @return [AjIList] List iterator
** @@
******************************************************************************/

AjIList ajFeatTagIter (AjPFeature thys) {
  return ajListIter( thys->Tags);
}

/* @func ajFeatTagval *********************************************************
**
** Traces (to the debug file) the tag-value pairs of a feature object
**
** @param [r]  iter  [AjIList] List iterator from ajFeatTagIter
** @param [W] tagnam [AjPStr*] Tag name
** @param [W] tagval [AjPStr*] Tag val
** @return [AjBool] ajTrue if another tag-value pair was returned
** @@
******************************************************************************/

AjBool ajFeatTagval (AjIList iter, AjPStr* tagnam, AjPStr* tagval) {

  FeatPTagval tv = NULL;

  tv = ajListIterNext(iter);
  if (!tv)
    return ajFalse;
  ajStrAssS(tagnam, tv->Tag);
  ajStrAssS (tagval, tv->Value);

  return ajTrue;
}

/* @func ajFeattableTrace *****************************************************
**
** Traces (to the debug file) a complete feature table
**
** @param [r]   thys  [AjPFeattable]  Feature table
** @return [void]
** @@
******************************************************************************/

void ajFeattableTrace (AjPFeattable thys) {

  AjIList iter = NULL;
  AjPFeature ft = NULL;
  ajint i=0;

  ajDebug ("== ajFeattableTrace Start ==\n");

  if (!thys) {
    ajDebug("NULL table\n");
    return;
  }

  ajDebug("  Name: '%S'\n", thys->Name);

  iter = ajListIter (thys->Features);
  while (ajListIterMore(iter)) {
    ft = ajListIterNext(iter);
    ajDebug("Features[%d]\n", ++i);
    ajFeatTrace (ft);
  }
  ajListIterFree (iter);

  ajDebug ("== ajFeattableTrace Done ==\n");
  return;
}

/* @funcstatic featTypeDna ****************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal DNA feature table
**
** @param [r]   type  [AjPStr] Type name
** @return [AjPStr] Valid feature type
** @@
******************************************************************************/

static AjPStr featTypeDna (AjPStr type) {

  featInit();

  return featTableType (type, FeatTypeTableDna);
}

/* @funcstatic featTypeProt ***************************************************
**
** Given a feature type name,
** returns the valid feature type for the internal protein feature table
**
** @param [r]   type  [AjPStr] Type name
** @return [AjPStr] Valid feature type
** @@
******************************************************************************/

static AjPStr featTypeProt (AjPStr type) {

  featInit();

  return featTableType (type, FeatTypeTableProtein);
}

/* @funcstatic featTagDna *****************************************************
**
** Given a feature tag name,
** returns the valid feature tag for the internal DNA feature table
**
** @param [r]   thys  [AjPStr] Tag name
** @return [AjPStr] Valid feature tag name
** @@
******************************************************************************/

static AjPStr featTagDna (AjPStr thys) {

  featInit();

  return featTableTag (thys, FeatTagsTableDna);
}

/* @funcstatic featTagProt ****************************************************
**
** Given a feature tag name,
** returns the valid feature tag for the internal protein feature table
**
** @param [r]   thys  [AjPStr] Tag name
** @return [AjPStr] Valid feature tag name
** @@
******************************************************************************/

static AjPStr featTagProt (AjPStr thys) {

  featInit();

  return featTableTag (thys, FeatTagsTableProtein);
}

/* @funcstatic featTableType **************************************************
**
** Given a feature type name,
** returns the valid feature type for a feature table
**
** @param [r]   type  [AjPStr] Type name
** @param [r]   table [AjPTable]  Feature table
** @return [AjPStr] Valid feature type
** @@
******************************************************************************/

static AjPStr featTableType (AjPStr type, AjPTable table) {

  static AjPStr ret = NULL;

  ret = (AjPStr) ajTableKey (table, type);
  if (ret) {
    /*
      ajDebug ("featTableType '%S' found in internal table as '%S'\n",
	     type, ret);
    */
    return ret;
  }
  else {
    ret = (AjPStr) ajTableGet (table, ajStrNew());
    ajDebug ("featTableType '%S' not in internal table %x, default to '%S'\n",
	     type, table, ret);
    /* ajStrTableTrace (table); */
  }
  return ret;
}

/* @funcstatic featTableTag ***************************************************
**
** Given a feature tag name,
** returns the valid feature tag name for a feature table
**
** @param [r]   tag  [AjPStr] Type name
** @param [r]   table [AjPTable]  Feature table
** @return [AjPStr] Valid feature tag name
** @@
******************************************************************************/

static AjPStr featTableTag (AjPStr tag, AjPTable table) {

  static AjPStr ret = NULL;
  static AjPStr emptytag = NULL;

  if (!emptytag)
    emptytag = ajStrNewC("");

  if (tag) {
    ret = (AjPStr) ajTableKey (table, tag);
    if (ret) {

      ajDebug ("featTag '%S' found in internal table as '%S'\n",
	       tag, ret);

      return ret;
    }
    else  {
      ret = (AjPStr) ajTableGet (table, emptytag);
      ajDebug ("featTag '%S' not in internal table %x, default to '%S'\n",
	       tag, table, ret);
      /* ajStrTableTrace (table); */
    }
  }
  else {
    ret = (AjPStr) ajTableGet (table, emptytag);
    ajDebug ("featTag '%S' use default '%S'\n",
	     tag, ret);
  }

  return ret;
}

/* @funcstatic featTableTagC **************************************************
**
** Given a feature tag name,
** returns the valid feature tag name for a feature table
**
** @param [r]   tag  [char*] Type name
** @param [r]   table [AjPTable]  Feature table
** @return [AjPStr] Valid feature tag name
** @@
******************************************************************************/

static AjPStr featTableTagC (char* tag, AjPTable table) {

  static AjPStr ret = NULL;
  static AjPStr tmptag = NULL;

  ajStrAssC (&tmptag, tag);

  ret = (AjPStr) ajTableKey (table, tmptag);
  if (ret) {
    /*
      ajDebug ("featTag '%S' found in internal table as '%S'\n",
	     tag, ret);
    */
    return ret;
  }
  else  {
    ret = (AjPStr) ajTableGet (table, ajStrNew());
    ajDebug ("featTag '%S' not in internal table %x, default to '%S'\n",
	     tag, table, ret);
    /* ajStrTableTrace (table); */
  }
  return ret;
}

/* @funcstatic featTagSpecialAllAnticodon *************************************
**
** Tests a string as a valid internal (EMBL) feature /anticodon tag
**
** The format is  (pos:<base_range>,aa:<amino_acid>)
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllAnticodon (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr begstr = NULL;
  static AjPStr endstr = NULL;
  static AjPStr aastr  = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^[(]pos:([0-9]+)[.][.]([0-9]+),aa:([^)]+)[)]$");

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &begstr);
    ajRegSubI (exp, 2, &endstr);
    ajRegSubI (exp, 3, &aastr);
  }

  if (!ret) {
    ajDebug("bad /anticodon value '%S'\n", *pval);
    ajWarn ("bad /anticodon value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllCitation **************************************
**
** Tests a string as a valid internal (EMBL) feature /citation tag
**
** The format is [1] where the number is a citation in an EMBL entry.
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllCitation (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr numstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^\\[([0-9]+)\\]$");

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &numstr);
  }

  if (!ret) {
    ajDebug("bad /citation value '%S'\n", *pval);
    ajWarn ("bad /citation value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllCodon *****************************************
**
** Tests a string as a valid internal (EMBL) feature /codon tag
**
** The format is (seq:"ttt",aa:Leu)
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllCodon (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr seqstr = NULL;
  static AjPStr aastr  = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^[(]seq:\"([acgt][acgt][acgt])\",aa:([^)]+)[)]$");

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &seqstr);
    ajRegSubI (exp, 2, &aastr);
  }

  if (!ret) {
    ajDebug("bad /codon value '%S'\n", *pval);
    ajWarn ("bad /codon value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllConssplice ************************************
**
** Tests a string as a valid internal (EMBL) feature /cons_splice tag
**
** The format is (5'site:YES,3'site:NO) where the booleans can
** be YES, NO, or ABSENT (intended for use where one site is NO and
** the other is missing)
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllConssplice (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr begstr = NULL;
  static AjPStr endstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^[(]5'site:([A-Z]+),3'site:([A-Z]+)[)]$");

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &begstr);
    ajRegSubI (exp, 2, &endstr);
  }

  if (!ret) {
    ajDebug("bad /cons_splice value '%S'\n", *pval);
    ajWarn ("bad /cons_splice value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllRptunit ***************************************
**
** Tests a string as a valid internal (EMBL) feature /rpt_unit tag
**
** The format is 123..789
** Labels are also allowed which should be feature tags in the entry.
** Genbank (NCBI) appear to be putting the sequence consensus in. Usually
** this is a valid "label" - except of course that the label does not exist.
** One horror (July 2002) was: /rpt_unit=TCCTCACGTAG(T/C)
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllRptunit (AjPStr* pval) {

  static AjPRegexp exp = NULL;
  static AjPRegexp labexp = NULL;

  static AjPStr begstr = NULL;
  static AjPStr endstr = NULL;
  static AjPStr labstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^([0-9]+)[.][.]([0-9]+)$");
  if (!labexp)
    labexp = ajRegCompC("^([acgtACGT]+)$");

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &begstr);
    ajRegSubI (exp, 2, &endstr);
  }

  if (ajRegExec(labexp, *pval)) {
    ajRegSubI (exp, 1, &labstr);
    ret = ajTrue;
  }

  if (!ret) {
    ajDebug("bad /rpt_unit value '%S'\n", *pval);
    ajWarn ("bad /rpt_unit value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllTranslexcept **********************************
**
** Tests a string as a valid internal (EMBL) feature /transl_except tag
**
** The format is (pos:213..215,aa:Trp)
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllTranslexcept (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr begstr = NULL;
  static AjPStr endstr = NULL;
  static AjPStr aastr  = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^[(]pos:([0-9]+)[.][.]([0-9]+),aa:([^)]+)[)]$");

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &begstr);
    ajRegSubI (exp, 2, &endstr);
    ajRegSubI (exp, 3, &aastr);
  }

  if (!ret) {
    ajDebug("bad /transl_except value '%S'\n", *pval);
    ajWarn ("bad /transl_except value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllDbxref ****************************************
**
** Tests a string as a valid internal (EMBL) feature /db_xref tag
**
** The format is <database>:<identifier>
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllDbxref (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr dbstr = NULL;
  static AjPStr idstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^([^:]+):(.+)$");

  /*  if (!exp)
      exp = ajRegCompC("^\"([^:]+):(.+)\"$");*/

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &dbstr);
    ajRegSubI (exp, 2, &idstr);
  }

  if (!ret) {
    ajDebug("bad /db_xref value '%S'\n", *pval);
    ajWarn ("bad /db_xref value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllProteinid *************************************
**
** Tests a string as a valid internal (EMBL) feature /protein_id tag
**
** The format is AAA12345.1
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllProteinid (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr idstr = NULL;
  static AjPStr preidstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^(([A-Z0-9]+)[.][0-9]+)$");

  /* if (!exp)
     exp = ajRegCompC("^\"(([A-Z0-9]+)[.][0-9]+)\"$");*/

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &idstr);
    ajRegSubI (exp, 2, &preidstr);
  }

  if (!ret) {
    ajDebug("bad /protein_id value '%S'\n", *pval);
    ajWarn ("bad /protein_id value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagSpecialAllReplace ***************************************
**
** Tests a string as a valid internal (EMBL) feature /replace tag
**
** The format is "<sequence>"
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllReplace (AjPStr* pval) {

  static AjPRegexp exp = NULL;
  static AjPStr seqstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^([acgtn]*)$"); /* n is used in old_sequence */
				/* and in misc_difference features */

  /* if (!exp)
     exp = ajRegCompC("^\"([acgt]*)\"$");*/

  /* no need to add quotes here - we will add them if needed on output */

  /*
  ajDebug("Before quote '%S' %c %c\n", *pval,
	  ajStrChar(*pval, 0), ajStrChar(*pval, -1));
  ajStrQuote (pval);
  ajDebug(" After quote '%S' %c %c\n", *pval,
	  ajStrChar(*pval, 0), ajStrChar(*pval, -1));
  */

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &seqstr);
  }

  if (!ret) {
    ajDebug("bad /replace value '%S'\n", *pval);
    ajWarn ("bad /replace value '%S'",   *pval);
  }

  return ret;
}


/* @funcstatic featTagSpecialAllTranslation ***********************************
**
** Tests a string as a valid internal (EMBL) feature /translation tag
**
** The format is valid amino acid codes, no white space
**
** @param  [r] pval [AjPStr*] parameter value
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagSpecialAllTranslation (AjPStr* pval) {

  static AjPRegexp exp = NULL;

  static AjPStr seqstr = NULL;
  AjBool ret = ajFalse;

  if (!exp)
    exp = ajRegCompC("^([ACDEFGHIKLMNPQRSTVWYX]+)$");

  /* if (!exp)
     exp = ajRegCompC("^\"([ACDEFGHIKLMNPQRSTVWYX]+)\"$"); */

  ajStrCleanWhite(pval);

  if (ajRegExec(exp, *pval)) {
    ret = ajTrue;
    ajRegSubI (exp, 1, &seqstr);
  }

  if (!ret) {
    ajDebug("bad /translation value '%S'\n", *pval);
    ajWarn ("bad /translation value '%S'",   *pval);
  }

  return ret;
}

/* @funcstatic featTagQuoteEmbl ***********************************************
**
** Internal quotes converted to two double quotes
** for EMBL feature tag values
**
** @param  [r] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteEmbl (AjPStr* pval) {

  static AjPRegexp quoteexp = NULL;
  static AjPStr substr = NULL;
  static AjPStr valcopy = NULL;
  static AjPStr tmpstr = NULL;

  if (!quoteexp)
    quoteexp = ajRegCompC("([^\"]*)\"");

  ajDebug("featTagQuoteEmbl '%S'\n", *pval);

  ajStrAssS (&valcopy, *pval);
  ajStrDelReuse (pval);
  while (ajRegExec(quoteexp, valcopy)) {
    ajRegSubI(quoteexp, 1, &substr);
    ajDebug("part '%S'\n", substr);
    ajStrApp(pval, substr);
    ajStrAppC(pval, "\"\"");
    ajRegPost (quoteexp, &tmpstr);
    ajStrAssS (&valcopy, tmpstr);
  }
  ajDebug("rest '%S'\n", valcopy);
  ajStrApp(pval, valcopy);
  ajStrQuote (pval);

  return;
}

/* @funcstatic featTagQuoteGff ************************************************
**
** Internal quotes converted to escaped quotes
** for EMBL feature tag values
**
** @param  [r] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagQuoteGff (AjPStr* pval) {

  static AjPRegexp quoteexp = NULL;
  static AjPStr substr = NULL;
  static AjPStr valcopy = NULL;
  static AjPStr tmpstr = NULL;

  if (!quoteexp)
    quoteexp = ajRegCompC("([^\"]*)\"");

  ajDebug("featTagQuoteGff '%S'\n", *pval);

  ajStrAssS (&valcopy, *pval);
  ajStrDelReuse (pval);
  while (ajRegExec(quoteexp, valcopy)) {
    ajRegSubI(quoteexp, 1, &substr);
    ajDebug("part '%S'\n", substr);
    ajStrApp(pval, substr);
    ajStrAppC(pval, "\\\"");
    ajRegPost (quoteexp, &tmpstr);
    ajStrAssS (&valcopy, tmpstr);
  }
  ajDebug("rest '%S'\n", valcopy);
  ajStrApp(pval, valcopy);
  ajStrQuote (pval);

  return;
}

/* @funcstatic featLocEmblWrapC ***********************************************
**
** Splits EMBL feature location at the last possible comma
** and adds the appropriate prefix (e.g. the EMBL FT line type)
**
** @param  [r] ploc [AjPStr*] location as a string
** @param  [r] margin [ajint] Right margin
** @param  [r] prefix [char*] Left margin prefix string
** @param  [r] preftyp [char*] Left margin prefix string for first line
**                            (includes the feature key)
** @param [W] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featLocEmblWrapC (AjPStr *ploc, ajint margin,
				char* prefix, char* preftyp,
				AjPStr* retstr) {

  ajint left = 0;
  ajint width = 0;
  ajint len = 0;
  ajint i, j, k;
  static AjPStr tmpstr = NULL;
  ajint last;

  left = strlen(prefix);
  width = margin - left;	/* available width for printing */

  ajStrCleanWhite(ploc);	/* no white space in locations */
  len = ajStrLen(*ploc);

  ajDebug("featLocEmblWrapC %d <%d> '%S'\n", len, width, *ploc);
  for (i=0; i < len; i+= k) {
    last = i + width - 1;

    ajDebug("try %d to %d (len %d)\n", i, last, len);

    if ((last+1) >= len) { /* no need to split */
      ajStrAssSub(&tmpstr, *ploc, i, len-1);
      ajDebug ("last %d >= len %d\n", last, len);
      j = 0;
    }
    else {
      ajStrAssSub(&tmpstr, *ploc, i, last); /* save max string */
      j = ajStrRFindC(tmpstr, ","); /* last comma in tmpstr */
      ajDebug ("comma at %d\n", j);
    }
    if (j < 1) {		/* no comma found */
      j = ajStrLen(tmpstr);
      k = j;
    }
    else {			/* print up to last comma */
      j++;
      k = j;			/* start after the comma */
    }
    ajDebug ("%d +%d '%.*S'\n", i, j, j, tmpstr);
    if (!i)
      ajFmtPrintAppS(retstr, "%s%.*S\n", preftyp,j, tmpstr);
    else
      ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,j, tmpstr);
  }

  return;
}

/* @funcstatic featTagEmblWrapC ***********************************************
**
** Splits feature table output at the last possible space (or
** the last column if there are no spaces) and adds the appropriate
** prefix (e.g. the EMBL FT line type)
**
** @param  [r] pval [AjPStr*] parameter value
** @param  [r] margin [ajint] Right margin
** @param  [r] prefix [char*] Left margin prefix string
** @param  [W] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagEmblWrapC (AjPStr *pval, ajint margin, char* prefix,
				AjPStr* retstr) {

  ajint left = 0;
  ajint width = 0;
  ajint len = 0;
  ajint i, j, k;
  static AjPStr tmpstr = NULL;
  ajint last;

  left = strlen(prefix);
  width = margin - left;	/* available width for printing */

  ajStrClean(pval);		/* single spaces only */
  len = ajStrLen(*pval);

  ajDebug("featTagEmblWrapC %d <%d> '%S'\n", len, width, *pval);
  for (i=0; i < len; i+= k) {
    last = i + width - 1;

    ajDebug("try %d to %d (len %d)\n", i, last, len);

    if ((last+1) >= len) { /* no need to split */
      ajStrAssSub(&tmpstr, *pval, i, len-1);
      ajDebug ("last %d >= len %d\n", last, len);
      j = 0;
    }
    else if (ajStrChar(*pval, (last+1)) == ' ') { /* split at max width */
      ajStrAssSub(&tmpstr, *pval, i, last);
      j = last + 1;
    }
    else {
      ajStrAssSub(&tmpstr, *pval, i, last); /* save max string */
      j = ajStrRFindC(tmpstr, " "); /* last space in tmpstr */
      ajDebug ("space at %d\n", j);
    }
    if (j < 1) {		/* no space found */
      j = ajStrLen(tmpstr);
      k = j;
    }
    else {			/* print up to last space */
      k = j + 1;		/* start after the space */
    }
    ajDebug ("%d +%d '%.*S'\n", i, j, j, tmpstr);
    ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,j, tmpstr);
  }

  return;
}

/* @funcstatic featTagSwissWrapC **********************************************
**
** Splits feature table output at the last possible space (or
** the last column if there are no spaces) and adds the appropriate
** prefix (e.g. the SwissProt FT line type)
**
** @param  [r] pval [AjPStr*] parameter value
** @param  [r] margin [ajint] Right margin
** @param  [r] prefix [char*] Left margin prefix string
** @param  [W] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagSwissWrapC (AjPStr *pval, ajint margin, char* prefix,
			       AjPStr* retstr) {

  ajint left = 0;
  ajint width = 0;
  ajint len = 0;
  ajint i, j, k;
  static AjPStr tmpstr = NULL;
  static AjPStr valstr = NULL;
  ajint last;

  left = strlen(prefix);
  width = margin - left;	/* available width for printing */

  k = width; /* will be reset in the loop */

  ajDebug("featTagSwissWrapC %d <%d> '%S'\n", ajStrLen(*pval), width, *pval);

  if (ajStrLen(*pval) <= margin) {		/* no need to wrap */
    ajStrAssS(retstr, *pval);
    ajStrAppK(retstr, '\n');
    ajDebug ("simple '%S'\n", *retstr);
    return;
  }

  ajStrAssSub(retstr, *pval, 0, left-1);
  ajStrAssSub(&valstr, *pval, left, -1);
  len = ajStrLen(valstr);
  ajDebug ("rest '%S'\n", valstr);

  for (i=0; i < len; i+= k) {
    last = i + width - 1;

    ajDebug("try %d to %d (len %d)\n", i, last, len);

    if ((last+1) >= len) { /* no need to split */
      ajStrAssSub(&tmpstr, valstr, i, len-1);
      ajDebug ("last %d >= len %d\n", last, len);
      j = 0;
    }
    else if (ajStrChar(valstr, (last+1)) == ' ') { /* split at max width */
      ajStrAssSub(&tmpstr, valstr, i, last);
      j = last + 1;
    }
    else {
      ajStrAssSub(&tmpstr, valstr, i, last); /* save max string */
      j = ajStrRFindC(tmpstr, " "); /* last space in tmpstr */
      ajDebug ("space at %d\n", j);
    }
    if (j < 1) {		/* no space found */
      j = ajStrLen(tmpstr);
      k = j;
    }
    else {			/* print up to last space */
      k = j + 1;		/* start after the space */
    }
    ajDebug ("%d +%d '%.*S'\n", i, j, j, tmpstr);
    if (i)
      ajFmtPrintAppS(retstr, "%s%.*S\n", prefix,j, tmpstr);
    else
      ajFmtPrintAppS(retstr, "%.*S\n", j, tmpstr);
  }

  return;
}

/* @funcstatic featTagAllLimit ************************************************
**
** Tests a string as a valid feature value, given a
** list of possible values.
**
** @param  [r] pval [AjPStr*] parameter value
** @param  [r] values [AjPStr] comma delimited list of values
** @return [AjBool] ajTrue for a valid value, possibly corrected
**                  ajFalse if invalid, to be converted to default (note) type
** @@
******************************************************************************/

static AjBool featTagAllLimit (AjPStr* pval, AjPStr values) {

  static AjPRegexp limitexp = NULL;
  static AjPStr limstr = NULL;
  static AjPStr valcopy = NULL;
  static AjPStr tmpstr = NULL;
  AjBool ret = ajFalse;

  if (!limitexp)
    limitexp = ajRegCompC("([^,]+),?");

  ajDebug("featTagAllLimit '%S' '%S'\n", *pval, values);

  ajStrAssS (&valcopy, values);

  while (ajRegExec(limitexp, valcopy)) {
    ajRegSubI(limitexp, 1, &limstr);
    ajDebug("test '%S'\n", limstr);
    if (ajStrMatchCase(*pval, limstr)) {
      if (!ajStrMatch(*pval, limstr)) {
	ajStrAssS(pval, limstr);
      }
      ret = ajTrue;
      break;
    }
    ajRegPost (limitexp, &tmpstr);
    ajStrAssS (&valcopy, tmpstr);
  }

  return ret;
}

/* @funcstatic featTagEmblDefault *********************************************
**
** Give up, and generate a default feature tag
**
** @param  [r] pout [AjPStr*] Output string
** @param  [r] tag [AjPStr] original tag name
** @param  [r] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagEmblDefault (AjPStr* pout, AjPStr tag, AjPStr* pval) {

  ajDebug("featTagEmblDefault '%S' '%S'\n", tag, *pval);

  featTagQuoteEmbl(pval);
  ajFmtPrintS(pout, "/note=\"%S: %S\"", tag, *pval);
  return;
}

/* @funcstatic featTagGffDefault **********************************************
**
** Give up, and generate a default feature tag
**
** @param  [r] pout [AjPStr*] Output string
** @param  [r] tag [AjPStr] original tag name
** @param  [r] pval [AjPStr*] parameter value
** @return [void]
** @@
******************************************************************************/

static void featTagGffDefault (AjPStr* pout, AjPStr tag, AjPStr* pval) {

  ajDebug("featTagGffDefault '%S' '%S'\n", tag, *pval);

  featTagQuoteGff(pval);
  ajFmtPrintS(pout, "note \"%S: %S\"", tag, *pval);
  return;
}

/* @funcstatic featTagSpecial *************************************************
**
** Special processing for known internal (EMBL) tags
**
** @param  [u] pval [AjPStr*] parameter value
** @param  [r] tag [AjPStr] original tag name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featTagSpecial (AjPStr* pval, AjPStr tag) {

  ajDebug("featTagSpecial '%S'\n", tag);

  if (ajStrMatchC(tag, "anticodon"))
    return featTagSpecialAllAnticodon(pval);

  if (ajStrMatchC(tag, "citation"))
    return featTagSpecialAllCitation(pval);

  if (ajStrMatchC(tag, "codon"))
    return featTagSpecialAllCodon(pval);

  if (ajStrMatchC(tag, "cons_splice"))
    return featTagSpecialAllConssplice(pval);

  if (ajStrMatchC(tag, "rpt_unit"))
    return featTagSpecialAllRptunit(pval);

  if (ajStrMatchC(tag, "transl_except"))
    return featTagSpecialAllTranslexcept(pval);

  if (ajStrMatchC(tag, "db_xref"))
    return featTagSpecialAllDbxref(pval);

  if (ajStrMatchC(tag, "protein_id"))
    return featTagSpecialAllProteinid(pval);

  if (ajStrMatchC(tag, "replace"))
    return featTagSpecialAllReplace(pval);

  if (ajStrMatchC(tag, "translation"))
    return featTagSpecialAllTranslation(pval);

  ajDebug ("Unrecognised special EMBL feature tag '%S'\n", tag);
  ajWarn  ("Unrecognised special EMBL feature tag '%S'",   tag);

  return ajFalse;
}

/* @funcstatic featTagGffSpecial **********************************************
**
** Special processing for known GFF tags
**
** This function will be very similar to featTagSpecial, with scope
** for future GFF-specific extensions
**
** @param  [u] pval [AjPStr*] tag value
** @param  [r] tag [AjPStr] original tag name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featTagGffSpecial (AjPStr* pval, AjPStr tag) {

  ajDebug("featTagGffSpecial '%S' '%S'\n", tag, *pval);

  if (ajStrMatchC(tag, "anticodon"))
    return featTagSpecialAllAnticodon(pval);

  if (ajStrMatchC(tag, "citation"))
    return featTagSpecialAllCitation(pval);

  if (ajStrMatchC(tag, "codon"))
    return featTagSpecialAllCodon(pval);

  if (ajStrMatchC(tag, "cons_splice"))
    return featTagSpecialAllConssplice(pval);

  if (ajStrMatchC(tag, "rpt_unit"))
    return featTagSpecialAllRptunit(pval);

  if (ajStrMatchC(tag, "transl_except"))
    return featTagSpecialAllTranslexcept(pval);

  if (ajStrMatchC(tag, "db_xref"))
    return featTagSpecialAllDbxref(pval);

  if (ajStrMatchC(tag, "protein_id"))
    return featTagSpecialAllProteinid(pval);

  if (ajStrMatchC(tag, "replace"))
    return featTagSpecialAllReplace(pval);

  if (ajStrMatchC(tag, "translation"))
    return featTagSpecialAllTranslation(pval);

  ajDebug ("Unrecognised special GFF feature tag '%S'\n", tag);
  ajWarn  ("Unrecognised special GFF feature tag '%S'",   tag);

  return ajFalse;
}

/* @funcstatic featDumpEmbl ***************************************************
**
** Write details of single feature to file in EMBL/GenBank/DDBJ format
**
** @param [r] feat     [AjPFeature] Feature
** @param [r] location [AjPStr] location list
** @param [r] file     [AjPFile] Output file
** @param [r] IsEmbl   [AjBool] ajTrue if writing EMBL format (FT prefix)
** @return [void]
** @@
******************************************************************************/

static void featDumpEmbl (AjPFeature feat, AjPStr location, AjPFile file,
			  AjBool IsEmbl) {

  AjIList iter = NULL;
  ajint i=0;
  FeatPTagval tv = NULL;
  AjPStr tmptyp=NULL;		/* these come from AjPTable */
  AjPStr tmptag=NULL;		/* so please, please */
				/* don't delete them */
  static AjPStr tmpfmt=NULL;
  static AjPStr outstr = NULL;
  static AjPStr tmpval=NULL;
  static AjPStr tmplim = NULL;
  static AjPStr deftag = NULL;
  char* cp;
  AjPStr wrapstr = NULL;
  static AjPStr preftyploc = NULL;
  static AjPStr preftyptag = NULL;
  static AjPStr tmploc = NULL;

  ajStrAssC (&deftag, "note");

  ajDebug ("featDumpEmbl Start\n");

 /* print the location */

  ajStrAssS (&tmploc, location);
  tmptyp = featTableType (feat->Type, FeatTypeTableEmbl);

  if (IsEmbl) {
    ajFmtPrintS (&preftyploc, "%s   %-15.15S ", "FT", tmptyp);
    ajFmtPrintS (&preftyptag, "%s                   ", "FT");
  }
  else {
    ajFmtPrintS (&preftyploc, "%s   %-15.15S ", "  ", tmptyp);
    ajFmtPrintS (&preftyptag, "%s                   ", "  ");
  }

  featLocEmblWrapC(&tmploc, 72,
		   ajStrStr(preftyptag),
		   ajStrStr(preftyploc), &wrapstr);
  ajFmtPrintF (file, "%S", wrapstr);
  ajStrDel (&wrapstr);

  /* print the qualifiers */

  iter = ajListIter (feat->Tags);
  while (ajListIterMore(iter)) {
    tv = ajListIterNext(iter);
    ++i;
    tmptag = featTableTag (tv->Tag, FeatTagsTableEmbl);
    featTagFmt (tmptag, FeatTagsTableEmbl, &tmpfmt);
    /* ajDebug (" %3d  %S value: '%S'\n", i, tv->Tag, tv->Value); */
    ajDebug (" %3d  %S format: '%S'\n", i, tmptag, tmpfmt);
    ajFmtPrintS (&outstr, "/%S", tmptag);
    if (tv->Value) {
      ajStrAssS(&tmpval, tv->Value);
      cp = ajStrStr(tmpfmt);
      switch (CASE2(cp[0], cp[1])) {
      case CASE2('L','I') :
	ajDebug ("case limited\n"); /* limited */
	featTagLimit (tmptag, FeatTagsTableEmbl, &tmplim);
	featTagAllLimit (&tmpval, tmplim);
	ajFmtPrintAppS (&outstr, "=%S\n", tmpval);
	ajStrDel(&tmplim);
	break;
      case CASE2('Q', 'L') :	/* limited, escape quotes */
	ajDebug ("case qlimited\n");
	featTagLimit (tmptag, FeatTagsTableEmbl, &tmplim);
	featTagAllLimit (&tmpval, tmplim);
	featTagQuoteEmbl (&tmpval);
	ajFmtPrintAppS (&outstr, "=%S\n", tmpval);
	ajStrDel(&tmplim);
	break;
      case CASE2('Q', 'S') :	/* special regexp, quoted */
	ajDebug ("case qspecial\n");
	if (!featTagSpecial(&tmpval, tmptag)) {
	  featTagEmblDefault(&outstr, tmptag, &tmpval);
	}
	else {
	  featTagQuoteEmbl(&tmpval);
	  ajFmtPrintAppS (&outstr, "=%S\n", tmpval);
	}
	break;
      case CASE2('S','P') :	/* special regexp */
	ajDebug ("case special\n");
	if (!featTagSpecial(&tmpval, tmptag)) {
	  featTagEmblDefault(&outstr, tmptag, &tmpval);
	}
	else {
	  ajFmtPrintAppS (&outstr, "=%S\n", tmpval);
	}
	break;
      case CASE2('T','E') :	/* no space, no quotes, wrap at margin */
	ajDebug ("case text\n");
	ajStrCleanWhite(&tmpval);
	ajFmtPrintAppS (&outstr, "=%S\n", tmpval);
	break;
      case CASE2('V','O') :	/* no value, so an error here */
	ajDebug ("case void\n");
	break;
      case CASE2('Q','T') :	/* escape quotes, wrap at space */
	ajDebug ("case qtext\n");
	featTagQuoteEmbl(&tmpval);
	ajFmtPrintAppS (&outstr, "=%S\n", tmpval);
	break;
      default:
	ajWarn("Unknown EMBL feature tag type '%S' for '%S'",
	       tmpfmt, tmptag);
      }
    }
    else {
      ajDebug ("no value, hope it is void: '%S'\n", tmpfmt);
    }
    featTagEmblWrapC(&outstr, 80, ajStrStr(preftyptag), &wrapstr);
    ajFmtPrintF (file, "%S", wrapstr);
    ajStrDel (&wrapstr);
  }

  ajDebug ("featDumpEmbl Done %d tags\n", i);

  ajListIterFree (iter);
  return;
}

/* @funcstatic featDumpPir ****************************************************
**
** Write details of single feature to output file
**
** @param [r] thys [AjPFeature] Feature
** @param [r] location [AjPStr] Location as a string
** @param [r] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpPir (AjPFeature thys, AjPStr location, AjPFile file) {
  AjIList iter = NULL;
  AjPStr outtyp = NULL;		/* these come from AjPTable */
  AjPStr outtag = NULL;		/* so please, please */
				/* don't delete them */
  static AjPStr tmptyp = NULL;
  static AjPStr outcomm = NULL;
  static AjPStr outfmt = NULL;
  static AjPStr outstr = NULL;
  static AjPStr tmpval=NULL;
  AjPStr outval = NULL;
  FeatPTagval tv = NULL;
  char* cp;
  AjBool typmod;
  ajStrAssC (&outcomm, "");

  ajStrAssS (&tmptyp, thys->Type);
  typmod = featTypePirOut (&tmptyp);	/* try to pick the best type if any */

  if (typmod)
    ajFeatSetDescApp (thys, thys->Type);

  outtyp = featTableType (tmptyp, FeatTypeTablePir); /* make sure it's PIR */

  ajStrSubstituteCC (&outtyp, "_", " ");

  ajFmtPrintF (file, "F;%S/%S:", location, outtyp);

  /* For all tag-values... */

  iter = ajFeatTagIter (thys);

  while (ajListIterMore(iter)) {
    tv = ajListIterNext(iter);
    outtag = featTableTag (tv->Tag, FeatTagsTablePir);
    featTagFmt (outtag, FeatTagsTablePir, &outfmt);
    ajDebug("Tag '%S' => '%S' %S '%S'\n",
	    tv->Tag, outtag, outfmt, tv->Value);

    if (tv->Value) {
      ajStrAssS(&tmpval, tv->Value);
      if (ajStrMatchCaseC(outtag, "comment")) {
	ajFmtPrintAppS (&outcomm, " #%S", tmpval);
	continue;
      }
      cp = ajStrStr(outfmt);
      switch (CASE2(cp[0], cp[1])) {
      default:
	ajFmtPrintAppS (&outstr, " %S", tmpval);
      }
    }
    else {
      ajDebug ("no value, hope it is void: '%S'\n", outfmt);
    }

    ajFmtPrintF (file, "%S", outstr);
    ajStrDelReuse(&outstr);
    ajStrDel(&outval);
  }

  ajListIterFree(iter);

  ajFmtPrintF (file, "%S\n", outcomm);

  return;
}

/* @funcstatic featDumpSwiss **************************************************
**
** Write details of single feature to output file
**
** @param [r] thys [AjPFeature] Feature
** @param [r] file [AjPFile] Output file
** @param [r] gftop [AjPFeature] Parent feature
** @return [void]
** @@
******************************************************************************/

static void featDumpSwiss (AjPFeature thys, AjPFile file,
			   AjPFeature gftop) {

  AjIList iter = NULL;
  AjPStr outtyp = NULL;		/* these come from AjPTable */
  AjPStr outtag = NULL;		/* so please, please */
				/* don't delete them */
  static AjPStr outfmt = NULL;
  static AjPStr outstr = NULL;
  static AjPStr tmpval=NULL;
  static AjPStr tmplim = NULL;
  AjPStr outval = NULL;
  FeatPTagval tv = NULL;
  ajint i=0;
  char* cp;
  AjPStr wrapstr = NULL;

  outtyp = featTableType (thys->Type, FeatTypeTableSwiss);

  ajFmtPrintS (&outstr, "FT   %-8.8S %6d %6d",
	       outtyp, thys->Start, thys->End);

  /* For all tag-values... from gftop which could be the same as thys */

  iter = ajFeatTagIter (gftop);

  while (ajListIterMore(iter)) {
    tv = ajListIterNext(iter);
    outtag = featTableTag (tv->Tag, FeatTagsTableSwiss);
    featTagFmt (outtag, FeatTagsTableSwiss, &outfmt);
    ajDebug("Tag '%S' => '%S' %S '%S'\n",
	    tv->Tag, outtag, outfmt, tv->Value);
    if (i++)
      (void) ajFmtPrintAppS (&outstr, " ") ;
    else
      (void) ajFmtPrintAppS (&outstr, "       ") ;

    /* ajFmtPrintAppS (&outstr, "%S", outtag); */ /* tag type is silent */

    if (tv->Value) {
      ajStrAssS(&tmpval, tv->Value);
      cp = ajStrStr(outfmt);
      switch (CASE2(cp[0], cp[1])) {
      case CASE2('L','I') :	/* limited */
      case CASE2('Q', 'L') :	/* limited, escape quotes */
	ajDebug ("case limited\n");
	featTagLimit (outtag, FeatTagsTableSwiss, &tmplim);
	featTagAllLimit (&tmpval, tmplim);
	ajFmtPrintAppS (&outstr, "%S", tmpval);
	ajStrDel(&tmplim);
	break;
      case CASE2('T','A') :	/* tag=text */
	ajDebug ("case tagval\n");
	if (ajStrMatchCaseC(outtag, "ftid")) /* fix case for tag */
	  ajFmtPrintAppS (&outstr, "/FTId=%S",tmpval);
	else			/* lower case is fine */
	  ajFmtPrintAppS (&outstr, "/%S=%S",outtag,  tmpval);
	break;
      case CASE2('T','E') :	/* simple test, wrap at space */
	ajDebug ("case text\n");
	ajFmtPrintAppS (&outstr, "%S", tmpval);
	break;
      case CASE2('B','T') :	/* bracketed, wrap at space */
	ajDebug ("case qtext\n");
	ajFmtPrintAppS (&outstr, "(%S)", tmpval);
	break;
      default:
	ajWarn("Unknown SWISS feature tag type '%S' for '%S'",
	       outfmt, outtag);
      }
    }
    else {
      ajDebug ("no value, hope it is void: '%S'\n", outfmt);
    }

    ajStrDel(&outval);
  }

  ajListIterFree(iter);

  if (i)
    (void) ajFmtPrintAppS (&outstr, ".");

  featTagSwissWrapC(&outstr, 80, "FT                                ",
		    &wrapstr);
  ajFmtPrintF (file, "%S", wrapstr);
  ajStrDelReuse(&outstr);
  ajStrDel (&wrapstr);


  return;
}

/* @funcstatic featDumpGff ****************************************************
**
** Write details of single feature to GFF output file
**
** @param [r] thys [AjPFeature] Feature
** @param [r] owner [AjPFeattable] Feature table (used for the sequence name)
** @param [r] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGff (AjPFeature thys, AjPFeattable owner, AjPFile file) {

  AjIList iter = NULL;
  AjPStr outtyp = NULL;		/* these come from AjPTable */
  AjPStr outtag = NULL;		/* so please, please */
				/* don't delete them */
  static AjPStr outfmt = NULL;
  static AjPStr outstr = NULL;
  static AjPStr tmpval=NULL;
  static AjPStr tmplim = NULL;
  AjPStr outval = NULL;
  FeatPTagval tv = NULL;
  ajint i=0;
  char* cp;
  AjPStr flagdata = NULL;

  /* header done by calling routine */

  ajDebug ("featDumpGff...\n");

  /* simple line-by line with Gff tags */

  outtyp = featTableType (thys->Type, FeatTypeTableGff);

  ajDebug("Type '%S' => '%S'\n", thys->Type, outtyp);

  (void) ajFmtPrintF (file,
	       "%S\t%S\t%S\t%d\t%d\t%.3f\t%c\t%c\t",
	       owner->Name,
	       thys->Source,
	       thys->Type,
	       thys->Start,
	       thys->End,
	       thys->Score,
	       featStrand(thys->Strand),
	       featFrame(thys->Frame) ) ;

  if(thys->Flags) {
    ajFmtPrintS(&flagdata, "0x%x", thys->Flags);
  }
  if (thys->Start2) {
    if (ajStrLen(flagdata)) ajStrAppC (&flagdata, " ");
    ajFmtPrintAppS (&flagdata, "start2:%d", thys->Start2);
  }
  if (thys->End2) {
    if (ajStrLen(flagdata)) ajStrAppC (&flagdata, " ");
    ajFmtPrintAppS (&flagdata, "end2:%d", thys->End2);
  }
  if (ajStrLen(thys->Remote)) {
    if (ajStrLen(flagdata)) ajStrAppC (&flagdata, " ");
    ajFmtPrintAppS (&flagdata, "remoteid:%S", thys->Remote);
  }
  if (ajStrLen(thys->Label)) {
    if (ajStrLen(flagdata)) ajStrAppC (&flagdata, " ");
    ajFmtPrintAppS (&flagdata, "label:%S", thys->Label);
  }

  /* group and flags */

  (void) ajFmtPrintF (file, "Sequence \"%S.%d\"",
		      owner->Name, thys->Group) ;
  i++;

  if(ajStrLen(flagdata)) {
/*
** Move this code up to run for all features - to preserve the order
** when rewriting in EMBL format
//    if ( FEATFLAG_MULTIPLE){
//      (void) ajFmtPrintF (file, "Sequence \"%S.%d\" ; ",
//			  owner->Name, thys->Group) ;
//      i++;
//    }
*/
    if (i++)
      (void) ajFmtPrintF (file, " ; ") ;
    (void) ajFmtPrintF (file, "FeatFlags \"%S\"", flagdata) ;
  }

  /* For all tag-values... */

  iter = ajFeatTagIter (thys);

  while (ajListIterMore(iter)) {
    tv = ajListIterNext(iter);
    outtag = featTableTag (tv->Tag, FeatTagsTableGff);
    featTagFmt (outtag, FeatTagsTableGff, &outfmt);
    ajDebug("Tag '%S' => '%S' %S '%S'\n",
	    tv->Tag, outtag, outfmt, tv->Value);
    if (i++)
      (void) ajFmtPrintF (file, " ; ") ;
    ajFmtPrintAppS (&outstr, "%S", outtag);

    if (tv->Value) {
      ajStrAssS(&tmpval, tv->Value);
      cp = ajStrStr(outfmt);
      switch (CASE2(cp[0], cp[1])) {
      case CASE2('L','I') :	/* limited */
	ajDebug ("case limited\n");
	featTagLimit (outtag, FeatTagsTableGff, &tmplim);
	featTagAllLimit (&tmpval, tmplim);
	ajFmtPrintAppS (&outstr, " %S", tmpval);
	ajStrDel(&tmplim);
	break;
      case CASE2('Q', 'L') :	/* limited, escape quotes */
	ajDebug ("case qlimited\n");
	featTagLimit (outtag, FeatTagsTableGff, &tmplim);
	featTagAllLimit (&tmpval, tmplim);
	featTagQuoteGff(&tmpval);
	ajFmtPrintAppS (&outstr, " %S", tmpval);
	ajStrDel(&tmplim);
	break;
      case CASE2('T','E') :	/* no space, no quotes, wrap at margin */
	ajDebug ("case text\n");
	ajStrCleanWhite(&tmpval);
	ajFmtPrintAppS (&outstr, " %S", tmpval);
	break;
      case CASE2('Q','T') :	/* escape quotes, wrap at space */
	ajDebug ("case qtext\n");
	featTagQuoteGff(&tmpval);
	ajFmtPrintAppS (&outstr, " %S", tmpval);
	break;
      case CASE2('Q', 'S') :	/* special regexp, quoted */
	ajDebug ("case qspecial\n");
	if (!featTagGffSpecial(&tmpval, outtag)) {
	  featTagGffDefault(&outstr, outtag, &tmpval);
	}
	else {
	  featTagQuoteGff(&tmpval);
	  ajFmtPrintAppS (&outstr, " %S", tmpval);
	}
	break;
      case CASE2('S','P') :	/* special regexp */
	ajDebug ("case special\n");
	if (!featTagGffSpecial(&tmpval, outtag)) {
	  featTagGffDefault(&outstr, outtag, &tmpval);
	}
	else {
	  ajFmtPrintAppS (&outstr, " %S", tmpval);
	}
	break;
      case CASE2('V','O') :	/* no value, so an error here */
	ajDebug ("case void\n");
	break;
      default:
	ajWarn("Unknown GFF feature tag type '%S' for '%S'",
	       outfmt, outtag);
      }
    }
    else {
      ajDebug ("no value, hope it is void: '%S'\n", outfmt);
    }

    ajFmtPrintF (file, "%S", outstr);
    ajStrDelReuse(&outstr);
    ajStrDel(&outval);
  }

  ajListIterFree(iter);

  (void) ajFmtPrintF (file, "\n") ;

  ajStrDel (&flagdata);

  return;
}

/* @funcstatic featTypePirIn **************************************************
**
** Converts a PIR feature type into the corresponding internal type,
** because internal types are based on SwissProt.
**
** @param [W] type [AjPStr*] PIR feature type in, returned as internal type
** @return [AjBool] ajTrue if the type name was found and changed
******************************************************************************/

static AjBool featTypePirIn (AjPStr* type) {
  ajint i = 0;

  while (FeatPirType[i].Pir) {
    if (ajStrMatchCaseC(*type, FeatPirType[i].Pir)) {
      ajStrAssC (type, FeatPirType[i].Internal);
      return ajTrue;
    }
    i++;
  }
  return ajFalse;
}

/* @funcstatic featTypePirOut *************************************************
**
** Converts an internal feature type into the corresponding PIR type,
** because internal types are based on SwissProt.
**
** @param [W] type [AjPStr*] PIR feature type in, returned as internal type
** @return [AjBool] ajTrue if the type name was found and changed
******************************************************************************/

static AjBool featTypePirOut (AjPStr* type) {
  ajint i = 0;

  while (FeatPirType[i].Pir) {
    if (ajStrMatchCaseC(*type, FeatPirType[i].Internal)) {
      ajStrAssC (type, FeatPirType[i].Pir);
      return ajTrue;
    }
    i++;
  }
  return ajFalse;
}

/* @funcstatic featTagFmt *****************************************************
**
** Converts a feature tag value into the correct format, after
** checking it is an acceptable value
**
** @param [R] name  [AjPStr] Tag name
** @param [R] table [AjPTable] Tag table
** @param [W] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagFmt (AjPStr name, AjPTable table, AjPStr* retstr) {

  static AjPStr valtype = NULL;

  static AjPStr tagstr = NULL;
  static AjPRegexp ExpTag = NULL;

  if (!ExpTag)
    ExpTag = ajRegCompC ("([^;]*);");

  tagstr = (AjPStr) ajTableGet (table, name);

  ajRegExec (ExpTag, tagstr);
  ajRegSubI (ExpTag, 1, &valtype);

  ajDebug ("featTagFmt '%S' type '%S' (%S)\n",
	   name, valtype, tagstr);

  ajStrAssS (retstr, valtype);

  return;
}

/* @funcstatic featTagLimit ***************************************************
**
** Returns the controlled vocabulary list for a limited value.
**
** @param [R] name  [AjPStr] Tag name
** @param [R] table [AjPTable] Tag table
** @param [W] retstr [AjPStr*] string with formatted value.
** @return [void]
** @@
******************************************************************************/

static void featTagLimit (AjPStr name, AjPTable table, AjPStr* retstr) {

  static AjPStr vallist = NULL;

  static AjPStr tagstr = NULL;
  static AjPRegexp ExpTag = NULL;

  if (!ExpTag)
    ExpTag = ajRegCompC ("[^;]*;(.*)");

  tagstr = (AjPStr) ajTableGet (table, name);

  ajRegExec (ExpTag, tagstr);
  ajRegSubI (ExpTag, 1, &vallist);

  ajDebug ("featTagLimit '%S' list '%S' (%S)\n",
	   name, vallist, tagstr);

  ajStrAssS (retstr, vallist);

  return;
}

/* @func ajFeatExit ***********************************************************
**
** Prints a summary of file usage with debug calls
**
** Cleans up feature table internal memory
**
** @return [void]
** @@
******************************************************************************/

void ajFeatExit (void) {
  ajint i;

  for(i=1;featInFormat[i].Name;i++){
    if (featInFormat[i].Used) {
      /* Calling funclist featInFormatDef() */
      if (!featInFormat[i].DelReg()) {
	ajDebug("No DelReg yet for %s\n",featInFormat[i].Name);
	ajErr ("No DelReg yet for %s\n",featInFormat[i].Name);
      }
    }
  }

  ajStrTableFree(&FeatTypeTableEmbl);
  ajStrTableFree(&FeatTagsTableEmbl);

  ajStrTableFree(&FeatTypeTableGff);
  ajStrTableFree(&FeatTagsTableGff);

  ajStrTableFree(&FeatTypeTableSwiss);
  ajStrTableFree(&FeatTagsTableSwiss);

  ajStrTableFree(&FeatTypeTableDna);
  ajStrTableFree(&FeatTagsTableDna);

  ajStrTableFree(&FeatTypeTableProtein);
  ajStrTableFree(&FeatTagsTableProtein);

  return;
}

/* @func ajFeatUnused *********************************************************
**
** Dummy function to prevent compiler warnings
**
** @return [void]
******************************************************************************/

void ajFeatUnused (void) {
  if (!DummyRegExec)
    DummyRegExec = ajRegCompC(".*");

  (void) featTagDna (NULL);
  (void) featTagProt (NULL);
  (void) featTableTagC (NULL, NULL);
}

/* @funcstatic featFeatureNew *************************************************
**
** Constructor for a feature
**
** @return [AjPFeature] New empty feature
******************************************************************************/

static AjPFeature featFeatureNew (void) {

  AjPFeature ret;

  AJNEW0(ret);

  ret->Tags = ajListNew() ;        /* Assume empty until otherwise needed */

  return ret;
}

/* @funcstatic featTableNew ***************************************************
**
** Constructor for a feature table object.
**
** The type is left uninitialised
**
** @return [AjPFeattable] New empty feature table
******************************************************************************/

static AjPFeattable featTableNew (void) {

  AjPFeattable ret;

  AJNEW0(ret);

  ret->Features = ajListNew() ;	/* assume empty until otherwise needed */

  return ret;
}

/* @funcstatic featTableNewS **************************************************
**
** Constructor for a feature table object with a defined name
**
** The type is left uninitialised
**
** @param [R] name [AjPStr] Name for new feature table (or NULL for unnamed)
** @return [AjPFeattable] New empty feature table
******************************************************************************/

static AjPFeattable featTableNewS (AjPStr name) {

  AjPFeattable ret;

  ret = featTableNew();
  featTableInit(ret, name);

  return ret;
}

/* @func ajFeatIsLocal ********************************************************
**
** Tests whether the feature is local to the sequence.
** Returns AJTRUE if it is local, AJFALSE if remote.
**
** @param [r] gf       [AjPFeature]  Feature
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/
AjBool ajFeatIsLocal (AjPFeature gf)
{
    return !(gf->Flags & FEATFLAG_REMOTEID);
}


/* @func ajFeatIsLocalRange ***************************************************
**
** Tests whether the feature is local and in the specified range of the
** sequence.
** Returns AJTRUE if it is local and within the range.
** (Any label location is assumed to be outside the range.)
**
** @param [r] gf       [AjPFeature]  Feature
** @param [r] start    [ajint]  start of range
** @param [r] end      [ajint]  end of range
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/
AjBool ajFeatIsLocalRange (AjPFeature gf, ajint start, ajint end)
{

    if (gf->Flags & FEATFLAG_REMOTEID)
	return AJFALSE;
    if (gf->Flags & FEATFLAG_LABEL)
	return AJFALSE;
    if (gf->End < start || gf->Start > end)
	return AJFALSE;

    return AJTRUE;
}

/* @func ajFeatIsChild ********************************************************
**
** Tests whether the feature is a child member of a join
** The parent (first) feature of a join gives:
** ajFeatIsChild == ajFalse && ajFeatIsMultiple == ajTrue
**
** @param [r] gf       [AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a child,
**                  AJFALSE if it is not a child
** @@
******************************************************************************/
AjBool ajFeatIsChild (AjPFeature gf)
{
    return (gf->Flags & FEATFLAG_CHILD);
}


/* @func ajFeatIsMultiple *****************************************************
**
** Tests whether the feature is a member of a join, group order or one_of
**
** @param [r] gf       [AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a member
** @@
******************************************************************************/
AjBool ajFeatIsMultiple (AjPFeature gf)
{
    return (gf->Flags & FEATFLAG_MULTIPLE);
}



/* @func ajFeatIsCompMult *****************************************************
**
** Tests whether the feature is a member of a complement around a
** multiple (join, etc.)
**
** @param [r] gf       [AjPFeature]  Feature
** @return [AjBool] Returns AJTRUE if it is a complemented multiple
** @@
******************************************************************************/
AjBool ajFeatIsCompMult (AjPFeature gf)
{
    return (gf->Flags & FEATFLAG_COMPLEMENT_MAIN);
}



/* @func ajFeattabOutDel ******************************************************
**
** Destructor for a feature table output object
**
** @param [w] thys [AjPFeattabOut *] feature format
** @return [void] Feature table output object
** @@
******************************************************************************/

void ajFeattabOutDel (AjPFeattabOut *thys)
{
    AjPFeattabOut pthis = *thys;

    ajStrDel(&pthis->Ufo);
    ajStrDel(&pthis->Formatstr);
    ajStrDel(&pthis->Filename);
    ajStrDel(&pthis->Entryname);
    ajStrDel(&pthis->Type);
    ajStrDel(&pthis->Seqname);

    AJFREE(pthis);

    return;
}

/* @func ajFeattablePos *******************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [wP] thys [AjPFeattable] Target feature table.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajFeattablePos (AjPFeattable thys, ajint ipos)
{
    return ajFeattablePosII (ajFeattableLen(thys), 1, ipos);
}

/* @func ajFeattablePosI ******************************************************
**
** Converts a string position into a true position. If ipos is negative,
** it is counted from the end of the string rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** @param [wP] thys [AjPFeattable] Target feature table.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajFeattablePosI (AjPFeattable thys, ajint imin, ajint ipos)
{
    return ajFeattablePosII (ajFeattableLen(thys), imin, ipos);
}

/* @func ajFeattablePosII *****************************************************
**
** Converts a position into a true position. If ipos is negative,
** it is counted from the end of the sequence rather than the beginning.
**
** imin is a minimum relative position, also counted from the end
** if negative. Usually this is the start position when the end of a range
** is being tested.
**
** For strings, the result can go off the end to the terminating NULL.
** For sequences the maximum is the last base.
**
** @param [r] ilen [ajint] maximum length.
** @param [r] imin [ajint] Start position.
** @param [r] ipos [ajint] Position.
** @return [ajint] string position between 1 and length.
** @@
******************************************************************************/

ajint ajFeattablePosII (ajint ilen, ajint imin, ajint ipos)
{
    ajint jpos;

    if (ipos < 0)
	jpos = ilen + ipos + 1;
    else
    {
	if (ipos)
	    jpos = ipos;
	else
	    jpos = 1;
    }

    if (jpos > ilen)
	jpos = ilen;

    if (jpos < imin)
	jpos = imin;

    ajDebug("ajFeattablePosII ilen: %d imin: %d ipos: %d) = %d\n",
	    ilen, imin, ipos, jpos);

    return jpos;
}

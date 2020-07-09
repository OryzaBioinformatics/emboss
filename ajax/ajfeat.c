/*******************************************************************
** @source AJAX genome feature module implementation
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
** @author Copyright (C) 1999 Richard Bruskiewich
** modified by Ian Longden.
** @version 2.0
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
** Dictionarys and there uses. (IL)
**
** Various Dictionarys are allowed and at present two exist :-
** EMBL and GFF. For reading from files the dictionary is used to only
** features and tags that are in the dictionary for that format.
**     For output a dictionary is again used as new features/tags may have
** been added, or even the output type may not be the same as the input
** and hence all features/tags have to be processed against the new 
** output dictionary.
**     All Dictionarys can be held at the same time as reading of one
** does not overwrite the next one (as it used to be the case). The
** GFF dictionary is no longer just the EMBL + GFF specials to be
** more felxible.
**
**
** starting simple mindedly here, without 'reference counts' for objects
** but might be good to use such reference counting for AjAccessionID's,
** AjGenome, AjChromosome and even, AjFeature/AjFeatTable objects.
** (for example, in AjPosition.Pos.Interval's, one dereferences AjMapObjects
**  which might better be handled with reference counts?).
**
********************************************************************/

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"

static AjPFeatLexicon FEATURE_DICTIONARY[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static AjPFeatLexicon OUTPUT_DICTIONARY = NULL;

#define EMBL_FILE "Efeatures.embl"
#define GFF_FILE "Efeatures.gff"
#define TAGS_FILE "Etags.embl"
#define TAGSG_FILE "Etags.gff"

#define NO_DICT_FORMAT   0
#define EMBL_DICT_FORMAT 1
#define GFF_DICT_FORMAT  2

#define FEATURE_START_BEFORE_SEQ 0x0001
#define FEATURE_END_AFTER_SEQ    0x0002
#define FEATURE_MOTHER           0x0004
#define FEATURE_BETWEEN_SEQ      0x0008  /* x^y */
#define FEATURE_START_TWO        0x0010  /* x.y.. */
#define FEATURE_END_TWO          0x0020  /* ..x.y */
#define FEATURE_POINT            0x0040  /* x */
#define FEATURE_COMPLEMENT_MAIN  0x0080  /* complement around the join */
#define FEATURE_MULTIPLE         0x0100  /* part of a multiple i.e. join*/

static char featFrame (int frame);
static char featStrand (int strand);

static void ajFeatSetFlag(int *flags, int val);

static AjPFeature featSwissFromLine ( AjPFeatTable thys, AjPStr line);
static AjPFeature featGenbankFromLine ( AjPFeatTable thys, AjPStr line, int genbank);

static AjBool featReadUnknown  ( AjPFeatTable thys, AjPFileBuff file) ;
static AjBool featReadAcedb    ( AjPFeatTable thys, AjPFileBuff file) ;
static AjBool featReadEmbl     ( AjPFeatTable thys, AjPFileBuff file) ;
static AjBool featReadGenbank  ( AjPFeatTable thys, AjPFileBuff file) ;
static AjBool featReadDdbj     ( AjPFeatTable thys, AjPFileBuff file) ;
static AjBool featReadGff      ( AjPFeatTable thys, AjPFileBuff file) ;
static AjBool featReadSwiss    ( AjPFeatTable thys, AjPFileBuff file) ;

static AjBool featRegInitAcedb();
static AjBool featRegInitEmbl();
static AjBool featRegInitGenBank();
static AjBool featRegInitGff();
static AjBool featRegInitSwiss();

static AjBool featDelRegAcedb();
/*static AjBool featDelRegDdbj();      Now done by Genbank */
static AjBool featDelRegEmbl();
static AjBool featDelRegGenBank();
static AjBool featDelRegGff();
static AjBool featDelRegSwiss();
static AjPFeatLexicon EMBL_Dictionary ();
static AjPFeatLexicon GFF_Dictionary ();
static AjPFeatLexicon dummyDict ();


static AjPFeatLexicon ajFeatVocNew(void) ;
static AjPFeatVocFeat ajFeatVocAddFeat(AjPFeatLexicon thys, AjPStr tag, int flag) ;
static AjPFeatVocTag ajFeatVocAddTag(AjPFeatLexicon thys, AjPStr tag, int flag) ;
static AjPFeatVocTag ajFeatVocTagKey( AjPFeatLexicon thys, AjPStr tag ) ;
static AjPFeatVocFeat ajFeatVocFeatKey( AjPFeatLexicon thys, AjPStr tag ) ;
static AjPFeature FeatureNew(AjPFeatTable owner,
			AjPStr       source, 
			AjPStr       type,
			int Start, int End,
			AjPStr       score,
			AjEFeatStrand   strand,
			AjEFeatFrame    frame, 
			AjPStr    desc,
			int Start2, int End2,
			int flags );

typedef struct FeatSInFormat {
  char *Name;
  AjBool (*Read)  (AjPFeatTable thys, AjPFileBuff file);
  AjBool (*InitReg)();
  AjBool (*DelReg)();
  AjPFeatLexicon (*ReadDict)(int format);
} FeatOInFormat, *FeatPInFormat;

#define EMBL_ORDER 1
#define GENBANK_ORDER 2
#define GB_ORDER 3
#define DBDJ_ORDER 4
#define GFF_ORDER 5
#define ACEDB_ORDER 6
#define SWISS_ORDER 7
#define SW_ORDER 8

/* name         input-function   init-regex-function del-regex-function
   feature-dictionary */

static FeatOInFormat featInFormatDef[] = {
  {"unknown",   featReadUnknown, NULL,               NULL,
   dummyDict},
  {"embl",      featReadEmbl,    featRegInitEmbl,    featDelRegEmbl,
   EMBL_Dictionary},
  {"genbank",   featReadGenbank, featRegInitGenBank, featDelRegGenBank,
   EMBL_Dictionary},
  {"gb",        featReadGenbank, featRegInitGenBank, featDelRegGenBank,
   EMBL_Dictionary},
  {"ddbj",      featReadDdbj,    featRegInitGenBank, featDelRegGenBank,
   EMBL_Dictionary},
  {"gff",       featReadGff,     featRegInitGff,     featDelRegGff,
   GFF_Dictionary},
  {"acedb",     featReadAcedb,   featRegInitAcedb,   featDelRegAcedb,
   GFF_Dictionary},
  {"swissprot", featReadSwiss,   featRegInitSwiss,   featDelRegSwiss,
   dummyDict},
  {"sw",        featReadSwiss,   featRegInitSwiss,   featDelRegSwiss,
   dummyDict},
  {NULL, NULL}
};

static FeatPInFormat featInFormat = featInFormatDef;

static AjBool ajFeatTableWriteUnknown (AjPFeatTable features, AjPFile file);
static AjBool ajFeatTableWriteAcedb (AjPFeatTable features, AjPFile file);

static AjPFeature featEmblFromLine ( AjPFeatTable thys, AjPStr line );

static void FeatVocDel (const void *key, void **value, void *cl);

typedef struct FeatSOutFormat {
  char *Name;
  AjBool (*Write) (AjPFeatTable thys, AjPFile file);
} FeatOOutFormat, *FeatPOutFormat;

static FeatOOutFormat featOutFormatDef[] = {
  {"unknown",   ajFeatTableWriteUnknown},
  {"embl",      ajFeatTableWriteEmbl},
  {"genbank",   ajFeatTableWriteGenbank},
  {"gb",        ajFeatTableWriteGenbank},
  {"ddbj",      ajFeatTableWriteDdbj},
  {"gff",       ajFeatTableWriteGff},
  {"acedb",     ajFeatTableWriteAcedb},
  {"swissprot", ajFeatTableWriteSwiss},
  {"sw",        ajFeatTableWriteSwiss},
  {NULL, NULL}
};

static FeatPOutFormat featOutFormat = featOutFormatDef;


/* ==================================================================== */
/* ========================== private datatypes ======================= */
/* ==================================================================== */

typedef enum {ajFeatKeyNULL = 0} FeatKeyTags ;

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

static const Except_T Null_Feature_Object     = { "NULL AjPFeature object encountered!" };
static const Except_T Not_a_Subclass          = { "Invalid AjPFeature object/class encountered!" };
static const Except_T Null_Feature_Tag        = { "NULL AjPFeature tag encountered!" };
static const Except_T Null_Feature_Lexicon    = { "NULL AjPFeatLexicon encountered!" };
static const Except_T Dictionary_Not_Readonly = { "ReadOnly dictionary expected here!" };
static const Except_T Null_IO_Handle          = { "NULL feature I/O handle encountered!" };

/* Different from original here as i will set each of the below, depending on
   the feature format */

static AjPRegexp 
   FEAT_Regex_Feature   = NULL,
   FEAT_Regex_Numeric   = NULL,
   FEAT_Regex_blankline = NULL,
   FEAT_Regex_version   = NULL,
   FEAT_Regex_date      = NULL,
   FEAT_Regex_region    = NULL,
   FEAT_Regex_comment   = NULL,
   FEAT_TV_Regex        = NULL,
   FEAT_TV_Regex1       = NULL,
   FEAT_TV_Regex2       = NULL,
   FEAT_TV_Regex3       = NULL,
   FEAT_TV_Regex4       = NULL,
   FEAT_TV_Regex5       = NULL,
   FEAT_TV_Regex6       = NULL,
   FEAT_TV_Regex7       = NULL,
   FEAT_TV_Regex8       = NULL,
   EMBL_Regex_New         = NULL,
   EMBL_Regex_Complement  = NULL,
   EMBL_Regex_Complement2 = NULL,
   EMBL_Regex_Join        = NULL,
   EMBL_Regex_Join2       = NULL,
   EMBL_Regex_Location    = NULL,
   EMBL_Regex_Location2   = NULL,
   EMBL_Regex_Location3   = NULL,
   EMBL_Regex_Location4   = NULL,
   EMBL_Regex_Location5   = NULL,
   EMBL_Regex_SourcePoint = NULL,
   EMBL_Regex_SourceLine  = NULL;

/* ==================================================================== */
/* ======================== private methods ========================= */
/* ==================================================================== */

typedef enum { AjFeatNoDebug = 0, 
               AjFeatDebug   = 1, 
               AjFeatTrace   = 2, 
               AjFeatVerbose = 3 } AjEFeatDebug ;


#define featObjInit(p,c) ((p)?((AjPFeatObject)(p))->Class=(c):AjCFeatUnknown)

static AjBool featFindInFormat (AjPStr format, int* iformat);
static AjBool featFindOutFormat (AjPStr format, int* iformat);

static void featClear    ( AjPFeature       thys ) ;

/* Return standard dictionaries, if defined? */
static AjPFeatLexicon EMBL_Dictionary(int format) ; 
static AjPFeatLexicon GFF_Dictionary(int format) ; 

static void featTabInit( AjPFeatTable    thys, 
                          AjPStr         name,
                          AjPFeatLexicon dictionary) ;

static AjBool featoutUfoProcess (AjPFeatTabOut thys, AjPStr ufo);
static void featTabClear  ( AjPFeatTable thys ) ;

static void   featDumpGff (AjPFeature thys, AjPFile file) ;

static void TagVocTrace (const void *key, void **value, void *cl);
static void FeatVocTrace (const void *key, void **value, void *cl);
static void ajFeatAddTagToFeatList(AjPFeatVocFeat feature,void *tag, int flag);
static  AjPFeatVocTagForFeat ajFeatFindTagInFeatlist(AjPFeature thys, AjPFeatVocTag key);
static int typeMatch(AjPFeatVocFeat feat,AjPList list);
static void ajFeatIgnoreTag2(AjPFeature Feat, AjPList list);
static void ajFeatOnlyAllowTag2(AjPFeature Feat, AjPList list);


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
** The constructors for the utility ('property') classes like AjPosition
** may be invoked by themselves (with the proper arguments).
**
** Generally, however, the  'table' class object for AjFeatures must first be created,
** prior to creating any 'AjFeature' objects. This order of events
** is enforced by requiring that the constructors for table classes be given
** the associated 'table' (or 'map') object.
**
** To replace or reuse an existing feature object see instead
** the {Feature Assignments} and {Feature Modifiers} methods.
**
******************************************************************************/

/* @func ajFeatTabOutOpen ********************************************
**
** Processes the specified UFO, and opens the resulting output file.
**
** @param [r] thys [AjPFeatTabOut] Features table output object
** @param [r] ufo [AjPStr] UFO feature output specifier
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTabOutOpen (AjPFeatTabOut thys, AjPStr ufo) {

  if (!featoutUfoProcess (thys, ufo))
    return ajFalse;

  ajDebug("trying to open '%S'\n", thys->Filename);
  thys->Handle = ajFileNewOut(thys->Filename);
  if (!thys->Handle) return ajFalse;
  ajDebug("after opening '%S'\n", thys->Filename);
  
  return ajTrue;
}

/* @func ajFeatTabInNew *****************************************************
**
** Constructor for an empty feature table input object
**
** @return [AjPFeatTabIn] Feature table input object
** @@
******************************************************************************/

AjPFeatTabIn ajFeatTabInNew (void) {
  AjPFeatTabIn pthis;
  AJNEW0(pthis);

  return pthis;
}

/* @func ajFeatTabInNewSSF ****************************************************
**
** Constructor for an empty feature table input object
**
** @param [r] fmt [AjPStr] feature format
** @param [r] name [AjPStr] sequence name
** @param [r] buff [AjPFileBuff] Buffer containing feature data
** @return [AjPFeatTabIn] Feature table input object
** @@
******************************************************************************/

AjPFeatTabIn ajFeatTabInNewSSF (AjPStr fmt, AjPStr name,
			       AjPFileBuff buff) {
  AjPFeatTabIn pthis;
  int iformat = 0;

  if (!featFindInFormat(fmt, &iformat)) return NULL;

  pthis = ajFeatTabInNew ();
  ajStrAssC (&pthis->Formatstr, featInFormat[pthis->Format].Name);
  pthis->Format = iformat;
  ajStrAssS (&pthis->Seqname, name);
  pthis->Handle = buff;

  return pthis;
}

/* @func ajFeatTabOutNew *****************************************************
**
** Constructor for an empty feature table output object
**
** @return [AjPFeatTabOut] Feature table input object
** @@
******************************************************************************/

AjPFeatTabOut ajFeatTabOutNew (void) {
  AjPFeatTabOut pthis;
  AJNEW0(pthis);
  return pthis;
}

/* @func ajFeaturesRead **********************************************
**
** Generic interface function for reading in features from a file
** given the file handle, class of map, data format of input
** and possibly other associated data.
**
** @param  [rC] ftin   [AjPFeatTabIn]  Specifies the external source (file)
**                                     of the features to be read in
** @return [AjPFeatTable] Pointer to a new feature table containing
** the features read in
** @cre 'file' argument must be a valid AjPFile
** @exception  'Null_IO_Handle' if ftin or its embedded file handle is invalid
** @@
**
** old protocol:
**
** AjPFeatTable ajFeaturesRead( AjPFile       file, 
**                              AjEFeatClass  type, 
**                              int           format, 
**                              void         *data)
**
** Version 1.0, 7/6/99 ACD to ajfeat access function (reading features)
**
*******************************************************************/

AjPFeatTable ajFeaturesRead  ( AjPFeatTabIn  ftin ) 
{
   AjPFileBuff   file ;
   int           format ;
   AjPFeatTable features = NULL ;
   AjBool       result   = ajFalse ;

   if(ftin  == NULL) AJRAISE(Null_IO_Handle) ;
   file     = ftin->Handle ;
   if(file  == NULL) AJRAISE(Null_IO_Handle) ;
   format   = ftin->Format ;

   if (!format)
     return NULL;

   ajDebug ("ajFeaturesRead format %d '%s'\n",
	    format, featInFormat[format].Name);

   if(!featInFormat[format].InitReg())
     ajDebug("No InitReg yet for %s\n",featInFormat[format].Name);

   FEATURE_DICTIONARY[format] = featInFormat[format].ReadDict(format);
   if(FEATURE_DICTIONARY[format])
     ajDebug("Dictionary LOADED for %s \n",featInFormat[format].Name);
   else
     ajDebug("Dictionary *NOT* LOADED for %s\n",featInFormat[format].Name);

   features = ajFeatTabNew (ftin->Seqname, FEATURE_DICTIONARY[format]);
   result = featInFormat[format].Read(features, file);

   if(!featInFormat[format].DelReg())
     ajDebug("No DelReg yet for %s\n",featInFormat[format].Name);

   if(result) {
      return features ;
   } else {
      ajFeatTabDel(&(features)) ;
   }
   return NULL;
}

/* @func ajFeatTabNew **********************************************
**
** Constructor for a new (generic) feature table
**
** @param  [rNE] name       [AjPStr] Name of the table (e.g. sequence name)
** @param  [rN]  dictionary [AjPFeatLexicon] Table tag dictionary
** @return [] [AjPFeatTable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
** 
*******************************************************************/

AjPFeatTable ajFeatTabNew( AjPStr name, 
                            AjPFeatLexicon dictionary)
{
  AjPFeatTable thys = NULL ;

  /* Allocate the object... */
  AJNEW0(thys) ;

  /* ..then initialize it */
  (void) featObjInit(thys, AjCFeatTable) ;
  featTabInit(thys, name, dictionary) ;

  return thys ;
}

/* @func ajFeatTabNewOut **********************************************
**
** Constructor for a new (generic) feature table using definitions in
** a sequence object
**
** @param  [r] name [AjPStr] Name
** @return [AjPFeatTable] Pointer to a new (empty) feature table
** @exception  'Mem_Failed' from memory allocation
** @@
** 
*******************************************************************/

AjPFeatTable ajFeatTabNewOut ( AjPStr name) {

  AjPFeatTable thys = NULL ;

  /* Allocate the object... */
  AJNEW0(thys) ;

  /* ..then initialize it */
  (void) featObjInit(thys, AjCFeatTable) ;
  featTabInit(thys, name, NULL) ;

  return thys ;
}


/* @func ajFeatureNew **********************************************
**
** Constructor - must specify associated 'AjFeatTable'
**               to which the new feature is automatically added!
**
** @param  [rC]   owner    [AjPFeatTable] Pointer to the AjFeatTable which
**                         owns the feature
** @cre 'owner' argument must be a valid AjFeatTable
** @param  [rENU] source   [AjPStr]      Analysis basis for feature
** @param  [rENU] type     [AjPStr]      Type of feature (e.g. exon)
** @param  [rNU]  Start    [int]  Start position of the feature
** @param  [rNU]  End      [int]  End position of the feature
** @param  [rENU] score    [AjPStr]      Analysis score for the feature
** @param  [rNU]  strand   [AjEFeatStrand]  Strand of the feature
** @param  [rNU]  frame    [AjEFeatFrame]   Frame of the feature
** @param  [rENU] desc     [AjPStr]      desc of feature feature
** @param  [rNU]  Start2   [int]  2nd Start position of the feature
** @param  [rNU]  End2     [int]  2nd End position of the feature
** @return [AjPFeature] newly allocated feature object
** @exception 'Mem_Failed' from memory allocation
** @@
** 
*******************************************************************/

AjPFeature ajFeatureNew(AjPFeatTable owner,
			AjPStr       source, 
			AjPStr       type,
			int Start, int End,
			AjPStr       score,
			AjEFeatStrand   strand,
			AjEFeatFrame    frame, 
			AjPStr    desc,
			int Start2, int End2) 
{
  int flags=FEATURE_MOTHER;
  AjPFeature thys = NULL ; 
 
  thys = FeatureNew(owner,source,type,Start,End,score,strand,frame,desc,
		    Start2,End2,flags);

  return thys;
}

/* @func ajFeatGffDictionaryCreate ****************************************
**
** Returns a copy of the GFF standard dictionary.
**
** @return [AjPFeatLexicon] Pointer to dictionary
** @@
** 
*******************************************************************/

AjPFeatLexicon ajFeatGffDictionaryCreate(){
  return GFF_Dictionary(GFF_ORDER);
}

/* @func ajFeatEmblDictionaryCreate ****************************************
**
** Returns a copy of the EMBL standard dictionary.
**
** @return [AjPFeatLexicon] Pointer to dictionary
** @@
** 
*******************************************************************/

AjPFeatLexicon ajFeatEmblDictionaryCreate(){
  return EMBL_Dictionary(EMBL_ORDER);
}




/* @funcstatic featcompbystart *******************************************
**
** Compare two features by their start.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
** 
** @return [int] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/
static int featcompbystart(const void *a, const void *b){
  AjPFeature *gfa = (AjPFeature *) a;  
  AjPFeature *gfb = (AjPFeature *) b;  
  int val=0;

  val = (*gfa)->Start-(*gfb)->Start;
  if(val){
    return val;
  }
  else{
    val = (*gfb)->End-(*gfa)->End;
    if(val)
      return val;
    else
      return 0;
  }
}
/* @funcstatic featcompbyend *******************************************
**
** Compare two features by their end.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
** 
** @return [int] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/
static int featcompbyend(const void *a, const void *b){
  AjPFeature *gfa = (AjPFeature *) a;  
  AjPFeature *gfb = (AjPFeature *) b;  
  int val=0;

  val = (*gfa)->End-(*gfb)->End;
  if(val){
    return val;
  }
  else{
    val = (*gfa)->Start-(*gfb)->Start;
    if(val)
      return val;
    else
      return 0;
  }
}
/* @funcstatic featcompbygroup *******************************************
**
** Compare two features by their start.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
** 
** @return [int] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/
static int featcompbygroup(const void *a, const void *b){
  AjPFeature *gfa = (AjPFeature *) a;  
  AjPFeature *gfb = (AjPFeature *) b;  
  int val=0;

  val = (*gfa)->Group-(*gfb)->Group;
  if(val){
    return val;
  }
  else{
    val = (*gfa)->Start-(*gfb)->Start;
      return val;
  }
}

/* @funcstatic featcompbytype *******************************************
**
** Compare two features by their start.
**
** @param [r] a [const void *] feature
** @param [r] b [const void *] another feature
** 
** @return [int] -1 if a is less than b, 0 if a is equal to b else +1.
** @@
******************************************************************************/
static int featcompbytype(const void *a, const void *b){
  AjPFeature *gfa = (AjPFeature *) a;  
  AjPFeature *gfb = (AjPFeature *) b;  
  int val=0;

  val = ajStrCmp(&(*gfa)->Type->name,&(*gfb)->Type->name);
  if(val){
    return val;
  }
  else{
    val = (*gfa)->Start-(*gfb)->Start;
    if(val){
      return val;
    }
    else{
      val = (*gfa)->End-(*gfb)->End;
      if(val){
	return val;
      }
      else
	    return 0;
    }
    
  }
}


/* @funcstatic featgetpos *******************************************
**
** Convert a string to a postion.
**
** @param [r] pos [AjPStr *] String to be converted.
** @param [w] ipos [int *]   integer value to be returned.
**
** @return [int] 0 if okay. 1 if first char removed (usually ',')
** @@
******************************************************************************/

static int featgetpos(AjPStr *pos,int *ipos){
  

    if(!ajStrToInt(*pos,ipos))
    {
	
	ajStrCut(pos,0,0);
	if(ajStrToInt(*pos, ipos))
	    return 1;
	else
	{
	    *ipos=0;
	    ajDebug("Error getting start (set to 0 now) for line\n");
	    return -1;
	}
    }
    return 0;
}


/****************************************************************
**
** Utility classes...
** 
******************************************************************/


/* @funcstatic ajFeatVocNew **********************************************
**
** Constructor for an empty AjFeatLexicon object
**
** @return [AjPFeatLexicon] Pointer to a newly allocated AjFeatLexicon object
** @exception 'Mem_Failed' from memory allocation
** @@
** 
*******************************************************************/

static AjPFeatLexicon ajFeatVocNew(void) 
{
  AjPFeatLexicon thys = NULL ;

  ajDebug ("ajFeatVocNew Allocating AjPFeatLexicon\n");
  /* Allocate the object... */
  AJNEW0(thys) ;
  assert(thys != NULL) ;

  ajDebug ("ajFeatVocNew Allocating symbol table\n");
  thys->TagVocTable = ajStrTableNewCase(500) ; /* not likely to have this many tags? */
  thys->FeatVocTable = ajStrTableNewCase(500) ; /* not likely to have this many tags? */

  return thys ;
}


/* @func ajFeatTableDict **********************************************
**
** Return lexicon entry of a feature table
**
** @param [r] thys [AjPFeatTable] pointer to feature table
** @return [AjPFeatLexicon] Pointer to lexicon
** @@
** 
*******************************************************************/

AjPFeatLexicon ajFeatTableDict(AjPFeatTable thys)
{
    return thys->Dictionary;
}


/* @funcstatic FeatureNew **********************************************
**
** Constructor - must specify associated 'AjFeatTable'
**               to which the new feature is automatically added!
**
** @param  [rC]   owner    [AjPFeatTable] Pointer to the AjFeatTable which
**                         owns the feature
** @cre 'owner' argument must be a valid AjFeatTable
** @param  [rENU] source   [AjPStr]      Analysis basis for feature
** @param  [rENU] type     [AjPStr]      Type of feature (e.g. exon)
** @param  [rNU]  Start    [int]  Start position of the feature
** @param  [rNU]  End      [int]  End position of the feature
** @param  [rENU] score    [AjPStr]      Analysis score for the feature
** @param  [rNU]  strand   [AjEFeatStrand]  Strand of the feature
** @param  [rNU]  frame    [AjEFeatFrame]   Frame of the feature
** @param  [rENU] desc     [AjPStr]      desc of feature feature
** @param  [rNU]  Start2   [int]  2nd Start position of the feature
** @param  [rNU]  End2     [int]  2nd End position of the feature
** @param  [rNU]  flags    [int]  flags.
** @return [AjPFeature] newly allocated feature object
** @exception 'Mem_Failed' from memory allocation
** @@
** 
*******************************************************************/

static AjPFeature FeatureNew(AjPFeatTable owner,
			AjPStr       source, 
			AjPStr       type,
			int Start, int End,
			AjPStr       score,
			AjEFeatStrand   strand,
			AjEFeatFrame    frame, 
			AjPStr    desc,
			int Start2, int End2,
			int flags ) 
{
  AjPFeature thys = NULL ;
  static AjPFeature mother = NULL;
  static int group=0,exon=1;

  if(!type || !type->Ptr){
    return thys;
  }
  ajDebug ("FeatureNew '%S' %d .. %d\n", type, Start, End);

  /* Allocate the object... */
  AJNEW0(thys) ;

  /* ..then initialize it */
  (void) featObjInit(thys, AjCFeature) ;

  if(flags & FEATURE_MOTHER){
    mother = thys;
    group++;
    thys->Group = group;
    thys->Exon = 0;
    exon = 1;
  }
  else{
    if(mother->Group == 0){
      group++;
      mother->Group= group;
      mother->Exon = 0;
    }
    thys->Group = group;
    thys->Exon = exon++;
  }

  thys->Owner = owner ;

  if(ajStrLen(source))
     thys->Source = ajFeatVocAddFeat(ajFeatTabDictionary(owner), source,0)  ;
  else 
     thys->Source = (AjPFeatVocFeat) ajFeatKeyNULL ;

  if(ajStrLen(type)){
    if(owner->Dictionary->ReadOnly){
      thys->Type = ajFeatVocFeatKey(ajFeatTabDictionary(owner), type)  ;
      if(!thys->Type)
	ajWarn("%S not in Feature dictionary",type);
    }
    else
      thys->Type = ajFeatVocAddFeat(ajFeatTabDictionary(owner), type, 0)  ;
  }
  else 
    thys->Type = (AjPFeatVocFeat)ajFeatKeyNULL ;
  
  
  thys->Score = NULL ;
  
  if(score && ajStrLen(score))
    (void) ajStrAssS(&(thys->Score), score) ;
  else
    (void) ajStrAssC(&(thys->Score), ".") ;

  thys->Flags = flags;
  
  thys->desc = NULL ;
  if(desc)
    if(ajStrLen(desc))
      (void) ajStrAssS(&(thys->desc), desc) ;

  thys->Tags = NULL ;        /* Assume empty until otherwise needed */

  thys->Comment = NULL ;
  thys->Strand = strand ;
    
  thys->Frame  = frame ;
  thys->Start = Start;
  thys->End = End;
  thys->Start2 = Start2;
  thys->End2 = End2;

  ajFeatTabAdd(thys->Owner,thys) ;

  return thys ;
}





/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section Feature Object Destructors ***************************************
**
** (Simple minded) object destruction by release of memory.
**
** No reference counting (for now). 
**
******************************************************************************/

/* @funcstatic TagVocDel *******************************************
**
** Delete all the valid tag's.
**
** @param [r] key   [const void *] not used.
** @param [r] value [void **]      (AjPFeatVocTag *) to be dumped out.
** @param [r] cl    [void *]       not used.
**
** @return [void]
** @@
*********************************************************************/ 
static void TagVocDel (const void *key, void **value, void *cl){
  AjPFeatVocTag *a= (AjPFeatVocTag *) value; 
  AjPStr limited;
  AjIList       iter = NULL ;

  ajStrDel(&(*a)->name);

  if((*a)->limitedValues){
    iter = ajListIter((*a)->limitedValues) ;
    while(ajListIterMore(iter)) {
      limited = (AjPStr) ajListIterNext (iter) ;
      ajStrDel(&limited);
    }
    ajListIterFree(iter) ;   
    ajListFree(&(*a)->limitedValues);
  }
  AJFREE((*a));
  return;
}


/* @func ajFeatTabInDel ********************************************
**
** Destructor for a feature table input object
**
** @param [d] pthis [AjPFeatTabIn*] Feature table input object
** @return [void]
** @@
******************************************************************************/

void ajFeatTabInDel (AjPFeatTabIn* pthis) {
  
  AjPFeatTabIn thys = *pthis;

  if (!thys) return;

  ajFileBuffDel(&thys->Handle);
  ajStrDel(&thys->Ufo);
  ajStrDel(&thys->Formatstr);
  ajStrDel(&thys->Filename);
  ajStrDel(&thys->Entryname);
  ajStrDel(&thys->Seqname);
  AJFREE(*pthis);
  return;
}


/* @func ajFeatDel ***********************************************************
**
** Destructor for AjFeature objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** The destructor does not delete the 'owner' AjFeatTable.
**
** @param  [wPC] pthis [AjPFeature*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFeatDel(AjPFeature *pthis) {
  if (!pthis) return ;
  if (!*pthis) return ;

  ajFeatObjVerify(*pthis,AjCFeature) ;

  featClear(*pthis) ;


/*  ajListDel(&(*pthis)->Type->Tags);*/
  


  AJFREE (*pthis) ; /* free the object */
  *pthis = NULL ;

  return;
}

/* @func ajFeatTabDel ********************************************************
**
** Destructor for AjFeatTable objects.
** If the given object (pointer) is NULL, or a NULL pointer, simply returns.
**
** @param  [wP] pthis [AjPFeatTable*] Pointer to the object to be deleted.
**         The pointer is always deleted.
** @return [void]
** @@
******************************************************************************/

void ajFeatTabDel(AjPFeatTable *pthis)
{
  if (!pthis) return ;
  if (!*pthis) return ;

  ajFeatObjVerify(*pthis,AjCFeatTable) ;

  featTabClear(*pthis) ;
  AJFREE (*pthis) ; /* free the object */
  *pthis = NULL ;

  return;
}
/* @func ajFeatClearTag **********************************************
**
** Method to delete a specified tag from a feature. If available, any 
** tag associated value is returned (caller must delete if necessary)
**
** @param  [rC] thys  [AjPFeature] The feature whose tag (and value)
**                                 is to be set
** @param  [rC] tag   [AjPStr]     Tag to be set
** @return [void*] Previous value of tag (if any - see above)
** @cre 'thys' must be a valid AjPFeature
** @cre 'tag' must be a valid AjPStr (non-null)
** @exception  'Null_Feature_Object' or 'Not_a_Subclass' if 'thys' is invalid
** @exception  'Null_Feature_Tag' if null tag passed
** @@
**
*******************************************************************/
  
void* ajFeatClearTag (AjPFeature thys, AjPStr tag)
{
  AjPFeatVocTag key = NULL ;
  AjIList iter   = NULL ;
  AjBool  found  = ajFalse ;
  LPFeatTagValue item = NULL ;
  void *oldvalue = NULL ;

  ajFeatObjVerify(thys,AjCFeature) ;
  if(!tag) AJRAISE(Null_Feature_Tag) ;

  /* Retrieve the tag's key from the 'Owner' map's dictionary... */
  key = ajFeatVocTagKey(ajFeatTabDictionary(ajFeatOwner(thys)), tag)  ; 
  if(!key) AJRAISE(Null_Feature_Tag) ;

  /* Then remove the tag, if found, from the AjPFeature,
     along with any associated value... */
  if (thys->Tags) {
    iter = ajListIter(thys->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue)ajListIterNext (iter) ;
      if(item->Tag->VocTag == key) {
        found = ajTrue ;
      }
    }
  }

  /* Delete only if found...*/
  if(found) {
     oldvalue = item->Value ;
     ajListRemove(iter) ;
     item->Tag = NULL ;
     AJFREE(item) ;
  }
  ajListIterFree(iter) ;

  return oldvalue ; /* return any old tag value to user, for possible deletion...*/
}

/* @func  ajFeatDictDel **********************************************
**
** Delete the Feature Dictionarys for each format (if they exist).
**
** @return [void]
** @@
** 
*******************************************************************/
void ajFeatDictDel(){
int i,j;

  for(i=0;i< 7; i++){
    if(FEATURE_DICTIONARY[i]){

      if(FEATURE_DICTIONARY[i]->TagVocTable){
	ajTableMap(FEATURE_DICTIONARY[i]->TagVocTable, TagVocDel,NULL) ;
	ajTableMap(FEATURE_DICTIONARY[i]->FeatVocTable, FeatVocDel,NULL) ;
	
	ajTableFree(&FEATURE_DICTIONARY[i]->TagVocTable);
	ajTableFree(&FEATURE_DICTIONARY[i]->FeatVocTable);
	
	for(j=i+1;j<7;j++){      /* some use the same dictionary. ONLY delete once */
	  if(FEATURE_DICTIONARY[i] == FEATURE_DICTIONARY[j])
	    FEATURE_DICTIONARY[j] = NULL;
	}

	AJFREE(FEATURE_DICTIONARY[i]);
      }
      FEATURE_DICTIONARY[i] = NULL;
    }
  }

  return;
}
/* @func  ajFeatDeleteDict **********************************************
**
** Delete a Dictionary.
**
** @param [rw] dict [AjPFeatLexicon] dictionary to delete.
** @return [void]
** @@
** 
*******************************************************************/
void ajFeatDeleteDict(AjPFeatLexicon dict){

  if(dict){
    
    if(dict->TagVocTable){
      ajTableMap(dict->TagVocTable, TagVocDel,NULL) ;
      ajTableMap(dict->FeatVocTable, FeatVocDel,NULL) ;
      
      ajTableFree(&dict->TagVocTable);
      ajTableFree(&dict->FeatVocTable);
      
      AJFREE(dict);
    }
  }
  
  return;
}
  

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Feature Assignments **********************************************
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


/* @func ajFeatSetTagValue **********************************************
**
** Method to sets the value for a specified tag for a feature. 
** The specified tag is created if necessary.  A NULL value deletes the 
** value associated with a tag, but the tag remains in the feature.
** If available, the previous value associated with the tag is returned.
**
** @param  [rC] thys  [AjPFeature] The feature whose tag (and value)
**                                 is to be set
** @param  [rC] tag   [AjPStr]     Tag to be set
** @param  [rC] value [AjPStr]     (Optional) value of tag
** @param  [rC] nomult  [AjBool]     if false add to previous tag if exists.
** @return [LPFeatTagValue] tag 
** @cre 'thys' must be a valid AjPFeature
** @cre 'tag' must be a valid AjPStr (non-null)
** @exception  'Null_Feature_Object' or 'Not_a_Subclass' if thys invalid
** @exception  'Null_Feature_Tag' if null tag passed
** @@
**
*******************************************************************/

LPFeatTagValue ajFeatSetTagValue (AjPFeature thys, AjPStr tag, AjPStr value, AjBool nomult)
{
  AjPFeatVocTagForFeat key2 = NULL;  
  AjPFeatVocTag key   = NULL ;
  AjIList iter        = NULL ;
  AjBool  found       = ajFalse ;
  LPFeatTagValue item = NULL ;
  AjIList       iter2 = NULL ;
  AjPStr      limited = NULL;

  ajFeatObjVerify(thys, AjCFeature) ;
  if(!tag) AJRAISE(Null_Feature_Tag) ;
  
  /* Retrieve the tag's key from the 'Owner' map's dictionary... */
  if(thys->Owner->Dictionary->ReadOnly){
    key = ajFeatVocTagKey(ajFeatTabDictionary(ajFeatOwner(thys)), tag);
    if(!key){
      ajWarn("%S not in TAG dictionary",tag);
      return item;
    }
    key2 = ajFeatFindTagInFeatlist(thys,key);
    if(!key2){
      if(thys->Type)
	ajWarn("TAG %S not valid for feature %S",tag,thys->Type->name);
      else
	ajWarn("TAG %S not added due to feature not being valid",tag);
      
      return item;   
    }
    if(key->flags & TAG_LIMITED){
      found= ajFalse;
      iter2 = ajListIter(key->limitedValues) ;
      while(ajListIterMore(iter2)) {
	limited = (AjPStr) ajListIterNext (iter2) ;
/*
** Gary Williams - 4 Sept 2000
** 
** This routine wasn't recognising features in uppercase in EMBL faeture tables
** The EMBL feature table document says:
** http://www.ebi.ac.uk/embl/Documentation/FT_definitions/feature_table.html
> 3.3.3.2 Controlled vocabulary or enumerated values
>
> Some qualifiers require values from a controlled vocabulary and are
> entered without quotation marks.  For example, the '/direction'
> qualifier has only three values: 'left', 'right' or 'both'.  Qualifier
> value controlled vocabularies, like feature table component names, must
> be treated as completely case insensitive: they may be entered and
> displayed in any combination of upper and lower case ('/direction=Left'
> '/direction=left' and '/direction=LEFT' are all legal and all convey the
> same meaning).  The database staffs reserve the right to regularize the
> case of qualifier values in the interest of readability, unlike the case
> of feature labels where the databases will maintain the case as
> originally entered (see Section 3.4.2).
**
** so this line changed from:
**     if(!ajStrCmp(&limited,&value))
** to
**     if(!ajStrCmpCase(limited, value))
**
*/

	if(!ajStrCmpCase(limited, value))
	  found= ajTrue;
      }
      ajListIterFree(iter2) ;   
      if(!found){
	ajWarn("%S not a recognised Limited Value",value);
	ajWarn("\thence tag %S not input.",key->name);
	ajWarn("\tvalid values are:-",key->name);
	iter2 = ajListIter(key->limitedValues) ;
	while(ajListIterMore(iter2)) {
	  limited = (AjPStr) ajListIterNext (iter2) ;
	  ajWarn("\t\t%S",limited);
	}
	ajListIterFree(iter2) ;   
	return item;
      }
      
    }
  }
  else {
    AjPFeatVocFeat featkey;

    key = ajFeatVocAddTag(ajFeatTabDictionary(ajFeatOwner(thys)), tag, 0)  ; 
    featkey = ajFeatVocAddFeat(ajFeatTabDictionary(ajFeatOwner(thys)),thys->Type->name,0);
    ajFeatAddTagToFeatList((AjPFeatVocFeat) featkey, key, 0);
    key2 = ajFeatFindTagInFeatlist(thys,key);
    if(!key2){
      ajDebug("ERROR: tag=%S val=%S\n",tag,value);
      ajDebug("ERROR: feature %S\n",featkey->name);
      ajDebug("ERROR: tag %S\n",key->name);
    }
  }

  /* Initialized Tags list if required */
  if(!thys->Tags) thys->Tags = ajListNew() ;
  
  /* Then add the tag to the AjPFeature, with any associated value... */
  found =ajFalse;
  if (thys->Tags) {
    iter = ajListIter(thys->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue)ajListIterNext (iter) ;
      if(item->Tag == key2) {
	found = ajTrue ;
 	break;
      }
    }
  }
  ajListIterFree(iter) ; 
  
  if(found && nomult) {
    if(item){
      ajStrAppC(&item->Value," ");
      ajStrApp(&item->Value,value);
    }
    ajStrDel(&value);
  } else { /* new item */
    AJNEW0(item) ;
    item->Tag   = key2 ;
    item->Value = value ;
    ajListPushApp(thys->Tags,item) ;
  }
  
  return item ;
}


/* @func ajFeatTabAdd **********************************************
**
** Method to add a new AjPFeature to a AjPFeatTable
**
** @param  [rC] thys    [AjPFeatTable] The feature table
** @cre 'thys' must be a valid AjPFeatTable
** @param  [rC] feature [AjPFeature]        Feature to be added to the set
** @cre 'feature' must be a valid AjPFeature (non-null)
** @return [void]
** @exception  'Null_Feature_Object' or 'Not_a_Subclass' if 'thys' is invalid
** @exception  'Null_Feature_Object' if null feature argument seen
** @@
**
*******************************************************************/

void ajFeatTabAdd ( AjPFeatTable thys, AjPFeature feature )
{
  ajFeatObjVerify(thys, AjCFeatTable) ;
  ajFeatObjVerify(feature, AjCFeature) ;

  ajListPushApp ( thys->Features, feature);  ;

  if(feature->Type)
    ajDebug ("ajFeatTabAdd list size %d '%S' %d %d\n",
	     ajListLength(thys->Features), feature->Type->name,
	     feature->Start, feature->End);
  else
    ajDebug ("ajFeatTabAdd list size %d '%S' %d %d\n",
	     ajListLength(thys->Features), NULL,
	     feature->Start, feature->End);

  return;
}


/* @func ajFeatRead *******************************************************
**
** Parses a UFO, opens an input file, and reads a feature table
**
** @param [w] pthis [AjPFeatTable*] Feature table created, (or NULL if failed)
** @param [r] tabin [AjPFeatTabIn] Feature input object
** @param [r] ufo [AjPStr] UFO feature spec
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatRead (AjPFeatTable* pthis, AjPFeatTabIn tabin, AjPStr ufo) {
  static AjPRegexp fmtexp = NULL;
  static AjPRegexp filexp = NULL;
  static AjPStr ufotest = NULL;

  AjBool fmtstat = ajFalse;	/* status returns from regex tests */
  AjBool filstat = ajFalse;	/* status returns from regex tests */
  AjBool ret = ajFalse;
  AjPFeatTabIn featio = tabin;
  int i;

  if (!fmtexp)
    fmtexp = ajRegCompC ("^([A-Za-z0-9]+):+(.*)$");
				/* \1 format */
				/* \2 remainder */

  if (!filexp)
    filexp = ajRegCompC ("^([^:]+)$");

  ajDebug("ajFeatRead UFO '%S'\n", ufo);

  (void) ajStrAssS (&ufotest, ufo);

  if (ajStrLen(ufo)) {
    fmtstat = ajRegExec (fmtexp, ufotest);
    ajDebug("feat format regexp: %B\n", fmtstat);
  }

  if (fmtstat) {
    ajRegSubI (fmtexp, 1, &featio->Formatstr);
    (void) ajStrSetC (&featio->Formatstr, featInFormat[0].Name); /* def. unknown */
    ajRegSubI (fmtexp, 2, &ufotest); /* trim off the format */
    ajDebug ("found feat format %S\n", featio->Formatstr);

    if (!featFindInFormat (featio->Formatstr, &featio->Format))
      ajErr ("unknown input feature table format '%S'\n NO Features will be read in", featio->Formatstr);
  }

  filstat = ajRegExec (filexp, ufotest);
  ajDebug("filexp: %B\n", filstat);
  if (filstat) {
    ajRegSubI (filexp, 1, &featio->Filename);
  }
  else {
    (void) ajFmtPrintS(&ufotest, "%S.%S", featio->Seqname, featio->Formatstr);
    (void) ajStrSet (&featio->Filename, ufotest);
    ajDebug ("generate filename  '%S'", featio->Filename);
  }

  /* Open the file so that we can try to read it */

  ajDebug("trying to open '%S'\n", tabin->Filename);
  tabin->Handle = ajFileBuffNewIn (tabin->Filename);
  if (!tabin->Handle) return ajFalse;
  ajDebug("after opening '%S'\n", tabin->Filename);


 /* OKAY if we have a format specified try this and this ONLY */
  if (fmtstat) {
    if(tabin->Format){
      *pthis = ajFeaturesRead(tabin);
    }
  }
  /* else loop through all types and try to read gff's */
  else {
    for(i=1;featInFormat[i].Name;i++){
      tabin->Format = i;

      *pthis = ajFeaturesRead(tabin);
 

      /* Reset buffer to start */
      ajFileBuffReset(tabin->Handle);

      if(*pthis){
	ajFileBuffDel(&tabin->Handle);
	return ajTrue;
      }
    }
  }
  if (!*pthis) 
    ret = ajFalse;
  else
    ret = ajTrue;

  ajFileBuffDel(&tabin->Handle);

  return ret;
}

/* @func ajFeatWrite *******************************************************
**
** Parses a UFO, opens an output file, and writes a feature table to it.
**
** @param [w] thys [AjPFeatTable] Feature table created
** @param [r] tabout [AjPFeatTabOut] Feature output object
** @param [r] ufo [AjPStr] UFO feature spec (ignored)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatWrite (AjPFeatTable thys, AjPFeatTabOut tabout, AjPStr ufo) {
  static AjPRegexp fmtexp = NULL;
  static AjPRegexp filexp = NULL;


  if(ufo){
    if (!fmtexp)
      fmtexp = ajRegCompC ("^([A-Za-z0-9]+):+(.*)$");


    if (!filexp)
      filexp = ajRegCompC ("^([^:]+)$");
  }
    
  ajDebug("***************ajFeatWrite UFO '%S'\n", ufo);

  return ajFeaturesWrite(tabout, thys);
}

/* @func ajFeatTableWrite *******************************************************
**
** Parses a UFO, opens an output file, and writes a feature table to it.
**
** @param [w] thys [AjPFeatTable] Feature table created
** @param [r] ufo [AjPStr] UFO feature spec (ignored)
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFeatTableWrite (AjPFeatTable thys, AjPStr ufo) {
  AjPFeatTabOut tabout=NULL;
    
  tabout= ajFeatTabOutNew();
  featoutUfoProcess (tabout, ufo);

  return ajFeaturesWrite(tabout, thys);
}

/* @func ajFeatIgnoreFeat *********************************************
**
** Remove all features that of the types in the list.
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to modify.
** @param [r] list [AjPList] List of Features to remove.
**
** @return [void]
** @@
**********************************************************************/  
void ajFeatIgnoreFeat(AjPFeatTable FeatTab, AjPList list){
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;
  if(FeatTab->Features){
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      if(typeMatch(gf->Type,list)){
	ajFeatDel(&gf);
	ajListRemove(iter);
      }
    }
    ajListIterFree(iter) ;
  }
} 

/* @func ajFeatOnlyAllowFeat *********************************************
**
** Remove all features NOT of the types in the list.
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to modify.
** @param [r] list [AjPList] List of Features NOT to remove.
**
** @return [void]
** @@
**********************************************************************/  
void  ajFeatOnlyAllowFeat(AjPFeatTable FeatTab, AjPList list){
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;
  if(FeatTab->Features){
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      if(!typeMatch(gf->Type,list)){
	ajFeatDel(&gf);
	ajListRemove(iter);
      }
    }
    ajListIterFree(iter) ;
  }
}

/* @func ajFeatIgnoreTag *********************************************
**
** Remove all tags that of the types in the list.
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to modify.
** @param [r] list [AjPList] List of Tags to remove.
**
** @return [void]
** @@
**********************************************************************/  
void ajFeatIgnoreTag(AjPFeatTable FeatTab, AjPList list){
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;
  if(FeatTab->Features){
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      ajFeatIgnoreTag2(gf,list);
    }
    ajListIterFree(iter) ;
  }
}


/* @func ajFeatOnlyAllowTag *********************************************
**
** Remove all tags NOT of the types in the list.
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to modify.
** @param [r] list [AjPList] List of Tags NOT to remove.
**
** @return [void]
** @@
**********************************************************************/  
void ajFeatOnlyAllowTag(AjPFeatTable FeatTab, AjPList list){
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;
  if(FeatTab->Features){
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      ajFeatOnlyAllowTag2(gf,list);
    }
    ajListIterFree(iter) ;
  }
}
/* @func ajFeatSortByType **********************************************
**
** Sort Feature table by Type.
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to be sorted.
**
** @return [void]
** @@
********************************************************************/
void ajFeatSortByType(AjPFeatTable FeatTab){
  ajListSort(FeatTab->Features,*featcompbytype);
}

/* @func ajFeatSortByStart **********************************************
**
** Sort Feature table by Start position.
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to be sorted.
**
** @return [void]
** @@
********************************************************************/
void ajFeatSortByStart(AjPFeatTable FeatTab){
  ajListSort(FeatTab->Features,*featcompbystart);
}

/* @func ajFeatSortByEnd **********************************************
**
** Sort Feature table by End position. 
**
** @param [rw] FeatTab [AjPFeatTable] Feature table to be sorted.
**
** @return [void]
** @@
********************************************************************/
void ajFeatSortByEnd(AjPFeatTable FeatTab){
  ajListSort(FeatTab->Features,*featcompbyend);
}
  
/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Feature Object Operators *********************************************
**
** These functions use the contents of a feature object, but do not make any changes.
**
******************************************************************************/



/* @funcstatic EMBL_Dictionary **********************************************
**
** Returns a copy of the EMBL standard dictionary.
**
** @param [r] format [int] for storage.
** @return [AjPFeatLexicon] Pointer to an existing standard tag dictionary
** @@
** 
*******************************************************************/

static AjPFeatLexicon EMBL_Dictionary (int format) {
  AjPFeatLexicon EMBL=NULL;
  AjPFile fileptr=NULL;
  AjPStr line = NULL;
  AjPStr tag = NULL,type=NULL,feature=NULL,limited=NULL,new=NULL;
  AjPRegexp TAG_VAL =  NULL;
  AjPRegexp FEAT_TAG = NULL;
  AjPRegexp MAN_FEAT_TAG = NULL;
  AjPRegexp LIMITED_VALS = NULL;
  AjPFeatVocFeat featkey  = NULL;
  AjPFeatVocTag key = NULL ;
  int count; 
  AjBool found=ajFalse,man=ajFalse;
  static int alreadyread = 0;


  
  if(alreadyread){
    return FEATURE_DICTIONARY[alreadyread];
  }
  else{
    alreadyread = format;
    TAG_VAL = ajRegCompC("([^ ]*) *([^ ]*)") ;
    FEAT_TAG = ajRegCompC("/(.+)") ;
    MAN_FEAT_TAG = ajRegCompC("M/(.+)") ;
    LIMITED_VALS = ajRegCompC("([^\",]+)") ;
    ajDebug("No Dictionary loaded yet so readit\n");
  }

  /* First read in the list of all possible tags */
  ajDebug("Trying to open %s...",TAGS_FILE);
  ajFileDataNewC(TAGS_FILE,&fileptr);
  if(!fileptr){
    ajDebug("FAILED\n");
    ajRegFree(&TAG_VAL);
    ajRegFree(&FEAT_TAG);
    ajRegFree(&MAN_FEAT_TAG);
    ajRegFree(&LIMITED_VALS);
    return   EMBL;
  }
  else
    ajDebug("OKAY\n");
    
  EMBL = ajFeatVocNew();     /* Create new dictionary */

  while(ajFileReadLine(fileptr,&line)){
    if(ajStrNCmpC(line,"#",1)){ /* if a comment skip it */
    if (ajRegExec(TAG_VAL,line)){
      int numtype=TAG_TEXT;

      ajRegSubI (TAG_VAL, 1, &tag) ;
      ajRegSubI (TAG_VAL, 2, &type) ;

      if(!ajStrCmpC(type,"TEXT"))
	numtype = TAG_TEXT;
      else if(!ajStrCmpC(type,"QTEXT"))
	numtype = TAG_QTEXT;
      else if(!ajStrCmpC(type,"SBI"))
	numtype = TAG_SBI;
      else if(!ajStrCmpC(type,"LIMITED")){
	numtype = TAG_LIMITED;
      }
      else if(!ajStrCmpC(type,"VOID"))
	numtype = TAG_VOID;

      ajStrDel(&type);
      key = ajFeatVocAddTag(EMBL,tag,numtype);

      if(numtype == TAG_LIMITED){
	(void) ajRegPost(TAG_VAL,&type);
	ajStrChomp(&type);
	/*	ajDebug("Limited string = *%S*\n",type);*/
	while(ajRegExec(LIMITED_VALS,type)){
	  ajRegSubI (LIMITED_VALS, 1, &limited) ;
	  (void) ajRegPost(LIMITED_VALS,&type);
	  if(!key->limitedValues)
	    key->limitedValues = ajListNew();
	  new = ajStrNewS(limited);
	  ajStrDel(&limited);
	  ajListPushApp(key->limitedValues,new);	  
	}
	ajStrDel(&type);
      }
    }
    }
  }
  ajFileClose(&fileptr);
 

  ajDebug("Trying to open %s...",EMBL_FILE);
  ajFileDataNewC(EMBL_FILE,&fileptr);
  if(!fileptr){
    ajDebug("FAILED\n");
    ajRegFree(&TAG_VAL);
    ajRegFree(&FEAT_TAG);
    ajRegFree(&MAN_FEAT_TAG);
    ajRegFree(&LIMITED_VALS);
    return NULL;
  }
  else{
    ajDebug("OKAY\n");
  }

  count =0;
  while(ajFileReadLine(fileptr,&line)){
    count++;
   if(ajStrNCmpC(line,"#",1)){ /* if a comment skip it */
     found=ajFalse;
    man=ajFalse;
    ajStrChomp(&line);
    if(ajRegExec(MAN_FEAT_TAG,line)){
      ajRegSubI (MAN_FEAT_TAG, 1, &tag) ;    /* get the mandatory feature tag */
      found=ajTrue;
      man = ajTrue;
    }
    else if(ajRegExec(FEAT_TAG,line)){
      ajRegSubI (FEAT_TAG, 1, &tag) ;    /* get the mandatory feature tag */
      found=ajTrue;
    }
    if(found){
      /*      if(man)
	      ajDebug("\t%S\n",tag);*/

      found = ajFalse;
      key = ajFeatVocTagKey(EMBL,tag);
      if(!key)
	ajWarn("tag %S not found in %s for feature %S at line %d\n",tag,
	       TAGS_FILE,feature,count);
      else {
	featkey = ajFeatVocFeatKey(EMBL,feature);
	if(!featkey)
	  ajWarn("SERIOUS VOODOO HERE!!!!\n");
	else{
	    ajFeatAddTagToFeatList((AjPFeatVocFeat) featkey, key, man?TAG_MANDATORY:0);
	}
      }
      ajStrDel(&tag);
    }
    else{            /* FEATURE rather than tag */
      /*      ajDebug("FEATURE=\"%S\"\n",line);*/
      ajFeatVocAddFeat(EMBL,line,0);
      ajStrAssS(&feature,line);
    }
   }
  }
  ajFileClose(&fileptr);
  ajStrDel(&line);
  ajStrDel(&feature);
  ajRegFree(&TAG_VAL);
  ajRegFree(&FEAT_TAG);
  ajRegFree(&MAN_FEAT_TAG);
  ajRegFree(&LIMITED_VALS);
  

  EMBL->ReadOnly = ajTrue;

  /*  ajFeatDickTracy(EMBL);*/

  EMBL->format = EMBL_DICT_FORMAT;
  return EMBL;
}
/* @funcstatic GFF_Dictionary **********************************************
**
** Returns a copy of the GFF standard dictionary.
** @param [r] format [int] store the format. 
**
** @return [AjPFeatLexicon] Pointer to an existing standard tag dictionary
** @@
** 
*******************************************************************/

static AjPFeatLexicon GFF_Dictionary (int format) {
  AjPFeatLexicon GFF=NULL;
  AjPFile fileptr=NULL;
  AjPStr line = NULL;
  AjPStr tag = NULL,type=NULL,feature=NULL,limited=NULL,new=NULL;
  AjPRegexp TAG_VAL = NULL ;
  AjPRegexp FEAT_TAG =  NULL ;
  AjPRegexp MAN_FEAT_TAG = NULL ;
  AjPRegexp LIMITED_VALS = NULL ;
  AjPFeatVocFeat featkey  = NULL;
  AjPFeatVocTag key = NULL ;
  int count; 
  AjBool found=ajFalse,man=ajFalse;
  static int alreadyread = 0;

  /* Read in the standard EMBL features and tags */
  if(alreadyread){
    return FEATURE_DICTIONARY[alreadyread];
  }
  else {
    alreadyread = format;
    TAG_VAL = ajRegCompC("([^ ]*) *([^ ]*)") ;
    FEAT_TAG = ajRegCompC("/(.+)") ;
    MAN_FEAT_TAG = ajRegCompC("M/(.+)") ;
    LIMITED_VALS = ajRegCompC("([^\",]+)") ;
    ajDebug("No Dictionary loaded yet so readit\n");
  }


  GFF = ajFeatVocNew();     /* Create new dictionary */
  if(GFF){
    GFF->ReadOnly = ajTrue;
    
    /* Now add the non-standard GFF ones */
    ajDebug("Trying to open %s...",TAGSG_FILE);
    ajFileDataNewC(TAGSG_FILE,&fileptr);
    if(!fileptr){
      ajDebug("FAILED\n");
      ajRegFree(&TAG_VAL);
      ajRegFree(&FEAT_TAG);
      ajRegFree(&MAN_FEAT_TAG);
      ajRegFree(&LIMITED_VALS);
      return   GFF;
    }
    else
      ajDebug("OKAY\n");
    
    while(ajFileReadLine(fileptr,&line)){
      if(ajStrNCmpC(line,"#",1)){ /* if a comment skip it */
       if (ajRegExec(TAG_VAL,line)){
	int numtype=TAG_TEXT;
	ajRegSubI (TAG_VAL, 1, &tag) ;
	ajRegSubI (TAG_VAL, 2, &type) ;
	/*	ajDebug("TAG_VAL=%S %S\n",tag,type);*/
	if(!ajStrCmpC(type,"TEXT"))
	  numtype = TAG_TEXT;
	else if(!ajStrCmpC(type,"QTEXT"))
	  numtype = TAG_QTEXT;
	else if(!ajStrCmpC(type,"SBI"))
	  numtype = TAG_SBI;
	else if(!ajStrCmpC(type,"LIMITED")){
	  numtype = TAG_LIMITED;
	}
	else if(!ajStrCmpC(type,"VOID"))
	  numtype = TAG_VOID;
	
	/*	numtype += TAG_GFF;*/
     	
	key = ajFeatVocAddTag(GFF,tag,numtype);
	
	if(numtype == TAG_LIMITED){
	  (void) ajRegPost(TAG_VAL,&type);
	  ajStrChomp(&type);
	  /*	  ajDebug("Limited string = *%S*\n",type);*/
	  while(ajRegExec(LIMITED_VALS,type)){
	    ajRegSubI (LIMITED_VALS, 1, &limited) ;
	    (void) ajRegPost(LIMITED_VALS,&type);
	    /*	    ajDebug("\t%S\n",limited);*/
	    if(!key->limitedValues)
	      key->limitedValues = ajListNew();
	    new = ajStrNewS(limited);
	    ajStrDel(&limited); limited = NULL;
	    ajListPushApp(key->limitedValues,new);	  
	  }
	}
       }
      }
    }
    ajFileClose(&fileptr);
    ajStrDel(&type);
    
    ajDebug("Trying to open %s...",GFF_FILE);
    ajFileDataNewC(GFF_FILE,&fileptr);
    if(!fileptr){
      ajDebug("FAILED\n");
      ajRegFree(&TAG_VAL);
      ajRegFree(&FEAT_TAG);
      ajRegFree(&MAN_FEAT_TAG);
      ajRegFree(&LIMITED_VALS);
      return GFF;
    }
    else{
      ajDebug("OKAY\n");
    }

    count =0;
    while(ajFileReadLine(fileptr,&line)){
      count++;
     if(ajStrNCmpC(line,"#",1)){ /* if a comment skip it */
       found=ajFalse;
      man=ajFalse;
      ajStrChomp(&line);
      if(ajRegExec(MAN_FEAT_TAG,line)){
	ajRegSubI (MAN_FEAT_TAG, 1, &tag) ;    /* get the mandatory feature tag */
	found=ajTrue;
	man = ajTrue;
      }
      else if(ajRegExec(FEAT_TAG,line)){
	ajRegSubI (FEAT_TAG, 1, &tag) ;    /* get the mandatory feature tag */
	found=ajTrue;
      }
      if(found){
	/*	if(man)
		ajDebug("\t%S\n",tag);*/
	
	found = ajFalse;
	key = ajFeatVocTagKey(GFF,tag);
	if(!key)
	  ajWarn("tag %S not found in tags for feature %S at line %d\n",tag,feature,count);
	else {
	  featkey = ajFeatVocFeatKey(GFF,feature);
	  if(!featkey)
	    ajWarn("SERIOUS VOODOO HERE!!!!\n");
	  else{
	    ajFeatAddTagToFeatList((AjPFeatVocFeat) featkey, key, man?TAG_MANDATORY:0);
	  }
	}
	ajStrDel(&tag);
      }
      else{            /* FEATURE rather than tag */
	/*	ajDebug("FEATURE=\"%S\" TAG_GFF=%d\n",line,TAG_GFF);*/
	ajFeatVocAddFeat(GFF,line,TAG_GFF);
	ajStrAssS(&feature,line);
      }
     }
    }
    ajStrDel(&feature);
    ajFileClose(&fileptr);
    ajStrDel(&line);
    ajRegFree(&TAG_VAL);
    ajRegFree(&FEAT_TAG);
    ajRegFree(&MAN_FEAT_TAG);
    ajRegFree(&LIMITED_VALS);
    

    GFF->ReadOnly = ajTrue;
    
  }    
  GFF->format = GFF_DICT_FORMAT;
  return GFF;
}

/* @funcstatic featTabInit **********************************************
**
** Initialize the components of a previously allocated AjPFeatTable object.
** An empty feature table tag dictionary is 
** allocated if NULL dictionary is given as an argument.
**
** @param [uC]   thys       [AjPFeatTable]   Target feature table object
** @param [rNE]  name       [AjPStr]         Name of the table (e.g.
**                                           sequence name)
** @param [rCN]  dictionary [AjPFeatLexicon] Pointer to an existing tag
**                                           dictionary; must be 'ReadOnly'
** @return [void]
** @cre 'thys' must be non-NULL and pointing to an AjPFeature (or subclass
**      thereof) object
** @exception  Invalid 'thys' object pointers trigger either a
**             'Null_Feature_Object' or a 'Not_a_Subclass' exception.
** @exception  'Dictionary_Not_Readonly' if non-NULL dictionary is not
**             'ReadOnly'
** 
** @@
** 
*******************************************************************/

static void featTabInit ( AjPFeatTable thys, 
                          AjPStr name,
                          AjPFeatLexicon dictionary) { 
  ajDebug ("featTabInit Entering...\n");
  ajFeatObjVerify(thys, AjCFeatTable) ;

  ajDebug ("featTabInit initializing name: '%S'\n", name);
  (void) ajStrAssS(&thys->Name,name) ;
  thys->DefFormat = 0;
  thys->Version   = 0 ;
  thys->Date      = ajTimeTodayF("GFF") ;
  ajTimeTrace (thys->Date);
  thys->DefSource = NULL ;
  thys->DefType   = NULL ;

  if(dictionary) {
    ajDebug ("featTabInit Defined dictionary\n");
    if(!dictionary->ReadOnly) AJRAISE(Dictionary_Not_Readonly) ; /* FOR SAFETY? */
    thys->Dictionary = dictionary ;
  }
  else {
    ajDebug ("featTabInit Empty dictionary\n");
    thys->Dictionary = ajFeatVocNew() ;
  }
  thys->Features = ajListNew() ;
}

/* @funcstatic featTabClear ***************************************************
**
** Clears a feature table of all features and lexicon
**
** @param [r] thys [AjPFeatTable] Feature table
** @return [void]
** @@
******************************************************************************/

static void featTabClear ( AjPFeatTable thys )
{
  AjIList iter       = NULL ;
  AjPFeature feature = NULL ;

  if (!thys) return ;

  /* Class, Format and Version are simple variables, non-allocated...*/
  /* Don't worry about the Date... probably static...*/

  /* hopefully, a defined dictionary should be readonly,
     hence not destroyed unintentionally */


  /*  ajFeatVocDel(&(thys->Dictionary)) ;  */
  /* ONLY DELETE DICTIONARY IF ALL SEQUENCES HAVE BEEN DELETED!!! */

  ajStrDel(&thys->Name);
  

  /* I traverse the list of features and delete them first... */
  if (thys->Features) {
    iter = ajListIter(thys->Features) ;
    while(ajListIterMore(iter)) {
      feature = (AjPFeature)ajListIterNext (iter) ;
      ajFeatDel(&feature) ; 
      ajListRemove(iter) ;
    }
  }
  ajListIterFree(iter) ;
  ajListDel(&(thys->Features)) ;
}


/* @func ajFeatDickTracy **********************************************
**
** Print out debug information about the EMBL Dictionary.
**
** @param [r] EMBL [AjPFeatLexicon] Pointer to an existing standard tag dictionary
**
** @return [void]
** @@
** 
*******************************************************************/

void ajFeatDickTracy(AjPFeatLexicon EMBL){

  /* Dump out Tags with TYPES */
  ajDebug("TAG\tTYPE\n");
  ajTableMap(EMBL->TagVocTable, TagVocTrace,NULL) ;

  ajTableMap(EMBL->FeatVocTable, FeatVocTrace,NULL) ;

  return;
}


/* @funcstatic featoutUfoProcess **********************************************
**
** Converts a UFO Uniform Feature Object into an open output file.
**
** First tests for "format::" and sets thys if it is found
**
** @param [u] thys [AjPFeatTabOut] Feature table to be written.
** @param [u] ufo [AjPStr] UFO.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featoutUfoProcess (AjPFeatTabOut thys, AjPStr ufo) {

  static AjPRegexp fmtexp = NULL;
  static AjPRegexp filexp = NULL;

  static AjPStr ufotest = NULL;

  AjBool fmtstat = ajFalse;	/* status returns from regex tests */
  AjBool filstat = ajFalse;	/* status returns from regex tests */
  AjPFeatTabOut featio = thys;
 
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
    ajRegSubI (fmtexp, 1, &featio->Formatstr);
    (void) ajStrSetC (&featio->Formatstr, featOutFormat[0].Name); /* def. unknown */
    ajRegSubI (fmtexp, 2, &ufotest); /* trim off the format */
    ajDebug ("found feat format %S\n", featio->Formatstr);
   }
  else {
    ajDebug("no feat format specified in UFO '%S' try '%S' or 'gff'\n",
	    ufo, featio->Formatstr);
    (void) ajStrSetC(&featio->Formatstr, "gff");
  }

  if (!featFindOutFormat (featio->Formatstr, &featio->Format)){
      ajErr ("unknown output feature format '%S' will dump out as gff instead\n", featio->Formatstr );
  }
  /* now go for the filename */

  filstat = ajRegExec (filexp, ufotest);
  ajDebug("filexp: %B\n", filstat);
  if (filstat) {
    ajRegSubI (filexp, 1, &featio->Filename);
  }
  else {
    if (ajStrLen(featio->Seqname))
      (void) ajFmtPrintS(&ufotest, "%S.%S", featio->Seqname,
			 featio->Formatstr);
    else
      (void) ajFmtPrintS(&ufotest, "unknown.%S", featio->Formatstr);
	
    (void) ajStrSet (&featio->Filename, ufotest);
    ajDebug ("generate filename  '%S'", featio->Filename);
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
** @param [w] iformat [int*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindInFormat (AjPStr format, int* iformat) {

  static AjPStr tmpformat = NULL;
  int i = 0;

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
** @param [w] iformat [int*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool featFindOutFormat (AjPStr format, int* iformat) {

  static AjPStr tmpformat = NULL;
  int i = 0;

  ajDebug ("featFindOutFormat '%S'\n", format);
  if (!ajStrLen(format))
    return ajFalse;

  (void) ajStrAssS (&tmpformat, format);
  (void) ajStrToLower(&tmpformat);
  for (i=0; featOutFormat[i].Name; i++) {
    ajDebug ("test %d '%s' len=%d\n", i, featOutFormat[i].Name,ajStrLen(tmpformat));
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


/* @funcstatic ajFeatVocAddFeat **********************************************
**
** Returns the existing or newly created 'key' of a tag in the table
**
** @param [uC] thys [AjPFeatLexicon] Target feature object
** @param [rC] tag  [AjPStr]         Name of the AjPfeatTable
**                                   which owns the feature
** @param [r]  flag [int]            information. 
** @exception  NULL 'thys' triggers 'Null_Feature_Lexicon'
** @exception  NULL 'tag'  triggers 'Null_Feature_Tag'
** @return [AjPFeatVocFeat] Key for specified tag
** @@
**
** I'm not currently checking the voc for 'ReadOnly' status...
** 
*******************************************************************/
static AjPFeatVocFeat ajFeatVocAddFeat(AjPFeatLexicon thys, AjPStr tag, int flag) 
{
  AjPFeatVocFeat key  = NULL ;
  AjPFeatVocFeat new  = NULL ;

  if(!thys) AJRAISE(Null_Feature_Lexicon) ;
  if(!tag)  AJRAISE(Null_Feature_Tag) ;
  
  key = ajFeatVocFeatKey(thys,tag) ;

  if(!key) {
    AJNEW0(new) ;
    new->name=NULL;
    (void) ajStrAss(&(new->name),tag) ;
    new->flags = flag;
    new->Tags = NULL;
    key = new ;
    (void) ajTablePut(thys->FeatVocTable, tag, key); 
  }

  return key ;
}

/* @funcstatic ajFeatVocAddTag **********************************************
**
** Returns the existing or newly created 'key' of a tag in the table
**
** @param [uC] thys [AjPFeatLexicon] Target feature object
** @param [rC] tag  [AjPStr]         Name of the AjPfeatTable
**                                   which owns the feature
** @param [r]  flag [int]            information. 
** @exception  NULL 'thys' triggers 'Null_Feature_Lexicon'
** @exception  NULL 'tag'  triggers 'Null_Feature_Tag'
** @return [AjPFeatVocTag] Key for specified tag
** @@
**
** I'm not currently checking the voc for 'ReadOnly' status...
** 
*******************************************************************/
static AjPFeatVocTag ajFeatVocAddTag(AjPFeatLexicon thys, AjPStr tag, int flag) 
{
  AjPFeatVocTag key  = NULL ;
  AjPFeatVocTag new  = NULL ;

  if(!thys) AJRAISE(Null_Feature_Lexicon) ;
  if(!tag)  AJRAISE(Null_Feature_Tag) ;
  
  key = ajFeatVocTagKey(thys,tag) ;

  if(!key) {
    AJNEW0(new) ;
    new->name=NULL;
    (void) ajStrAss(&(new)->name,tag) ;
    new->flags = flag;
    key = new ;
    (void) ajTablePut(thys->TagVocTable, tag, new); 
  }
  return key ;
}

/* @funcstatic ajFeatVocTagKey **********************************************
**
** Returns the 'key' for a tag in the table; returns NULL if tag is absent
**
** @param [uC] thys [AjPFeatLexicon] Symbol table containing the tag
** @param [rC] tag  [AjPStr]            The tag to be retrieved
** @exception  NULL 'thys' triggers 'Null_Feature_Lexicon'
** @exception  NULL 'tag'  triggers 'Null_Feature_Tag'
** @return [AjPFeatVocTag] Key for specified tag
** @@
*******************************************************************/

static AjPFeatVocTag ajFeatVocTagKey( AjPFeatLexicon thys, AjPStr tag ) {
  if(!thys) AJRAISE(Null_Feature_Lexicon) ;
  if(!tag)  AJRAISE(Null_Feature_Tag) ;
  
  return (AjPFeatVocTag) ajTableGet(thys->TagVocTable, tag) ; /* may be NULL if absent? */
}
/* @funcstatic ajFeatVocFeatKey **********************************************
**
** Returns the 'key' for a tag in the table; returns NULL if tag is absent
**
** @param [uC] thys [AjPFeatLexicon] Symbol table containing the tag
** @param [rC] tag  [AjPStr]            The tag to be retrieved
** @exception  NULL 'thys' triggers 'Null_Feature_Lexicon'
** @exception  NULL 'tag'  triggers 'Null_Feature_Tag'
** @return [AjPFeatVocFeat] Key for specified tag
** @@
*******************************************************************/

static AjPFeatVocFeat ajFeatVocFeatKey( AjPFeatLexicon thys, AjPStr tag ) {
  if(!thys) AJRAISE(Null_Feature_Lexicon) ;
  if(!tag)  AJRAISE(Null_Feature_Tag) ;
  
  return (AjPFeatVocFeat) ajTableGet(thys->FeatVocTable, tag) ; /* may be NULL if absent? */
}




/* @func ajFeaturesWrite **********************************************
**
** Generic interface function for reading in features from a file
** given the file handle, class of map, data format of input
** and possibly other associated data.
**
** @param  [rC] ftout   [AjPFeatTabOut]  Specifies the external source (file) of the features to be read in
** @cre 'ftout' argument must be a valid AjPFeatTabOut object
** @param  [rC] features [AjPFeatTable]  Feature set to be written out
** @return [AjBool]  Returns ajTrue if successful; ajFalse otherwise
** @exception  'Null_IO_Handle' if 'ftout' or embedded file file handle invalid
** @exception  'Null_Feature_Object' or 'Not_a_Subclass' if 'features' is invalid
** @exception  'Format_Not_Supported' or 'Unknown_Format' if 'format' is invalid
** @@
**
** Version 1.0, 21/6/99 ACD to ajfeat access function (for writing features)
**
** Old function protocol:
**
** AjBool ajFeaturesWrite( AjPFile file, 
**                         AjPFeatTable features, 
**                         int format ) 
**
*******************************************************************/

AjBool ajFeaturesWrite ( AjPFeatTabOut ftout, AjPFeatTable features )  
{
  AjPFile       file ;
  int format ;
  AjBool result          = ajFalse ;

  if(features){
    ajDebug( "ajFeaturesWrite Validating arguments\n");
    if(ftout == NULL) AJRAISE(Null_IO_Handle) ;
    file    = ftout->Handle ;
    if(file  == NULL) AJRAISE(Null_IO_Handle) ;
    format  = ftout->Format ;
    
    ajFeatObjVerify(features, AjCFeatTable) ;
    ajDebug( "ajFeaturesWrite format is %d OK\n",ftout->Format);
    
    if(!FEATURE_DICTIONARY[format]) /* Read in dictionary if it is not already there */
      FEATURE_DICTIONARY[format] = featInFormat[format].ReadDict(format);

    OUTPUT_DICTIONARY = FEATURE_DICTIONARY[format]; 

    featInFormat[format].InitReg();
    result = featOutFormat[format].Write(features, file);
    featInFormat[format].DelReg();

    return result ;
  }
  else{
    ajDebug( " NO Features to output\n");
    return AJTRUE;
  }
}


/* @func ajFeatObjCheck ********************************************
**
** Check a feature object.
**
** @param [r] pObj [void*] Object
** @param [r] crass [AjEFeatClass] class code
** @param [r] file [const char*] source file name
** @param [r] line [int] source file line
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatObjCheck(void* pObj, AjEFeatClass crass, const char* file,
		      int line) {
  AjPFeatObject pFeatObj = (AjPFeatObject)pObj ;

  if(!pFeatObj)  
    ajExceptRaise(&Null_Feature_Object, file, line) ;

  /* mask out common bits and comparing for precise bits set */ 
  if( (((int)pFeatObj->Class & (int)crass)^ (int)crass) ) return ajFalse ; 

  return ajTrue;
}

/* @func ajFeatObjAssert ********************************************
**
** Exception throwing variant of ajFeatObjCheck
**
** @param [r] pObj [void*] Object
** @param [r] crass [AjEFeatClass] class code
** @param [r] file [const char*] source file name
** @param [r] line [int] source file line
** @return [void]
** @@
******************************************************************************/

void ajFeatObjAssert(void* pObj, AjEFeatClass crass,
		     const char* file, int line)  {
  AjPFeatObject pFeatObj = (AjPFeatObject)pObj ;

  if(!pFeatObj)  
    ajExceptRaise(&Null_Feature_Object, file, line) ;

  /* mask out common bits and comparing for precise bits set */ 
  if( (((int)pFeatObj->Class & (int)crass)^ (int)crass) ) 
    ajExceptRaise(&Not_a_Subclass, file, line) ; 

  return;
}
 

/* @funcstatic featReadAcedb ********************************************
**
** Reads feature data in ACEDB format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadAcedb    ( AjPFeatTable thys, AjPFileBuff file){
  ajDebug("featReadAcedb NOT IMPLEMENTED YET\n");
  return ajFalse;
}

/* @funcstatic featReadEmbl ********************************************
**
** Reads feature data in EMBL format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadEmbl     ( AjPFeatTable thys, AjPFileBuff file){
  static AjPStr line  = NULL ;
  AjBool found = ajFalse ;

  if(!line)
    line = ajStrNewL(100);
  
  while( ajFileBuffGet (file, &line) ) {

    (void) ajStrChomp(&line) ;
    
    if(ajRegExec(FEAT_Regex_Feature, line)){  /* if it's a feature do stuff */
      if(featEmblFromLine(thys, line)) 
	found = ajTrue ;
    }
  }
  return found;
}

/* @funcstatic featReadGenbank ********************************************
**
** Reads feature data in GenBank format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadGenbank  ( AjPFeatTable thys, AjPFileBuff file){
  static AjPStr line  = NULL ;
  AjBool found = ajFalse ;
  AjPRegexp FEAT_START_LINE = ajRegCompC("^FEATURE");
  AjPRegexp FEAT_END_LINE   = ajRegCompC("^[^ ]+");
  AjBool featStarted = ajFalse;

  if(!line)
    line = ajStrNewL(100);
  
  while( ajFileBuffGet (file, &line) ) {

    (void) ajStrChompC(&line, "\n") ;	/* just remove the last character */

    ajDebug ("featReadGenbank '%S'\n", line);

    if(featStarted && ajRegExec(FEAT_END_LINE, line)){
      ajRegFree(&FEAT_START_LINE);
      ajRegFree(&FEAT_END_LINE);
      return found;
    }
    if(featStarted || ajRegExec(FEAT_START_LINE, line)) {
      featStarted = ajTrue;
      if(featGenbankFromLine(thys, line, 1))
	found = ajTrue ;
    }
    ajDebug ("found %B\n", found);
  }
  ajRegFree(&FEAT_START_LINE);
  ajRegFree(&FEAT_END_LINE);
  return found;

}

/* @funcstatic featReadDdbj ********************************************
**
** Reads feature data in DDBJ format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadDdbj     ( AjPFeatTable thys, AjPFileBuff file){
  static AjPStr line  = NULL ;
  AjBool found = ajFalse ;
  AjPRegexp FEAT_START_LINE = ajRegCompC("^FEATURE");
  AjPRegexp FEAT_END_LINE   = ajRegCompC("^[^ ]+");
  AjBool featStarted = ajFalse;

  if(!line)
    line = ajStrNewL(100);
  
  while( ajFileBuffGet (file, &line) ) {

    /*    (void) ajStrChomp(&line) ;*/
    if(featStarted && ajRegExec(FEAT_END_LINE, line)){
      ajRegFree(&FEAT_START_LINE);
      ajRegFree(&FEAT_END_LINE);
      return found;
    }
    if(featStarted || ajRegExec(FEAT_START_LINE, line)) {
      featStarted = ajTrue;
      if(featGenbankFromLine(thys, line, 0))
	found = ajTrue ;
    }
  }
  ajRegFree(&FEAT_START_LINE);
  ajRegFree(&FEAT_END_LINE);
  return found;

}

/* @funcstatic featReadUnknown ********************************************
**
** Reads feature data in Unknown format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadUnknown    ( AjPFeatTable thys, AjPFileBuff file) {
  return ajFalse;
}

/* @funcstatic featReadSwiss ********************************************
**
** Reads feature data in SwissProt format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Buffered input file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadSwiss    ( AjPFeatTable thys, AjPFileBuff file) {
  static AjPStr line  = NULL ;
  AjBool found = ajFalse;
  ajDebug("featReadSwiss..........\n");
  
  while( ajFileBuffGet (file, &line) ) {
    
    (void) ajStrChomp(&line) ;
    
    if(ajRegExec(FEAT_Regex_Feature, line)) {
      if(featSwissFromLine(thys, line))  /* does start with FT but what else?? */
	found = ajTrue ;
    }
  }
  return found ;
}


/* ==================================================================== */
/* ======================== GFF Processing functions ================== */
/* =================================================================== */

/* ajfeat defaults to version 2 GFF only...*/
#define DEFAULT_GFF_VERSION 2

/* @funcstatic EMBLProcessTagValues *******************************************
**
** Parses and adds a tag-value set to the specified AjPFeature;
** looked at 'parse_group method in GFF::GeneFeature.pm Perl module
** for inspiration
**
** @param [u] gf [AjPFeature] Feature
** @param [r] line [AjPStr] of the form FT       .... /tag="value"
** @return [int] wether value has finished of not.
** @@
******************************************************************************/

static int EMBLProcessTagValues (AjPFeature gf, AjPStr line) {
  AjPStr value = NULL ;       /* Element to add to tags array */
  AjPStr tag   = NULL ;		/* used in list data */
  static LPFeatTagValue item = NULL ;

  /*  static AjBool tagopened = 0;*/  /* tag value continues on another line set to true */

  ajDebug("*********  EMBLProcessTagValues ********************\n");
  
  if (ajRegExec(FEAT_TV_Regex1,line)){     /*     /tag="value" */
    ajRegSubI (FEAT_TV_Regex1, 1, &tag) ;                     /* gets the tag */
    ajRegSubI (FEAT_TV_Regex1, 2, &value) ;/* gets the value */
    item = (LPFeatTagValue ) ajFeatSetTagValue(gf,tag,value,ajFalse);
    if(!item)
      ajStrDel(&value);
  }
  else if (ajRegExec(FEAT_TV_Regex2,line)){ /*     /tag=value */
    ajRegSubI (FEAT_TV_Regex2, 1, &tag) ;  
    ajRegSubI (FEAT_TV_Regex2, 2, &value) ; 
    item = (LPFeatTagValue ) ajFeatSetTagValue(gf,tag,value,ajFalse);
    if(!item)
      ajStrDel(&value);
  }
  else if (ajRegExec(FEAT_TV_Regex3,line)){ /* FT  /tag="value */
    ajRegSubI (FEAT_TV_Regex3, 1, &tag) ;  
    ajRegSubI (FEAT_TV_Regex3, 2, &value) ; 
    item = (LPFeatTagValue )  ajFeatSetTagValue(gf,tag,value,ajFalse);
    if(!item)
      ajStrDel(&value);
  }
  else if (ajRegExec(FEAT_TV_Regex6,line)){ /* FT  /tag*/
    ajRegSubI (FEAT_TV_Regex6, 1, &tag) ;  
    item = (LPFeatTagValue )  ajFeatSetTagValue(gf,tag,value,ajFalse);
    if(!item)
      ajStrDel(&value);
  }
  else if (ajRegExec(FEAT_TV_Regex4,line)){ /* FT valuecont */
    ajRegSubI (FEAT_TV_Regex4, 1, &value) ;
    if(item)
      item = (LPFeatTagValue )  ajFeatSetTagValue(gf,item->Tag->VocTag->name,value,ajTrue);
  }
  else if (ajRegExec(FEAT_TV_Regex5,line)){ /* FT valueend" */
    ajRegSubI (FEAT_TV_Regex5, 1, &value) ;  
    if(item)
      item = (LPFeatTagValue )  ajFeatSetTagValue(gf,item->Tag->VocTag->name,value,ajTrue);
  }
  else {
    ajDebug("NOTHING ?? %S",line);
  }

  if(tag)
    ajStrDel(&tag);

  return 1;

}


/* @funcstatic GFFProcessTagValues ********************************************
**
** Parses and adds a tag-value set to the specified AjPFeature;
** looked at 'parse_group method in GFF::GeneFeature.pm Perl module
** for inspiration
**
** @param [u] gf [AjPFeature] Feature
** @param [r] groupfield [AjPStr] Group field identifier
** @return [void]
** @@
******************************************************************************/

static void GFFProcessTagValues (AjPFeature gf, AjPStr groupfield) 
{
  static AjPStr field = NULL ;       /* Element to add to tags array */
  AjPStr tag   = NULL ;		/* used in list data */
  AjPStr value = NULL ;
  static AjPStr  G_String  = NULL ;


  ajDebug("*********  GFFProcessTagValues ********************\n");
  /*  if (!FeatModInitDone) ajFeatModInit();*/

    /* Validate arguments */
    if(!groupfield) return ;
    ajFeatObjVerify( gf, AjCFeature ) ;

    if( gf->Owner->Version == 1.0 ) {
      AjPStr group= NULL; /*(AjPStr *)AJALLOC0(2*sizeof(AjPStr)) ;*/
      ajStrAssS(&group,groupfield);
      /*       group[0] = groupfield ;*/
       /*       group[1] = '\0' ;*/
       (void) ajFeatSetTagValue(gf, ajStrNewC("Group"), group,ajFalse) ;
    } else { 

/*
 *     Version 2 or greater: parse groupfield for semicolon ';'
 *     delimited tag-value structures, taking special care about
 *     double quoted string context Code adapted from GFF.pm
 *     (th/rbsk), itself inherited from AceParse.pm, courtesy of James
 *     Gilbert 
*/

       AjBool  end_tag   = ajFalse ;
       AjPStr  sub1      = NULL ;

       (void) ajStrAssS( &G_String, groupfield) ;

       while (ajStrLen(G_String) > 0) {
	  AjBool q     = ajFalse ; /* Set inside quoted text */
	  for (;;) {
	    /* Are inside single double quote ? */
	    if ( q ) {
                /* Are at end of quoted text? */
                if (ajRegExec(FEAT_TV_Regex1,G_String)) {
                    q = ajFalse ;
                    ajRegSubI (FEAT_TV_Regex1, 1, &sub1) ;  /* gets the first matched subexpression */
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex1, &G_String) ;
		    if(ajStrLen(G_String) == 0 || !ajStrCmpC(sub1,";") ) {
			end_tag = ajTrue ; /* EOL simulated end of tag-value */
		    }
		    ajStrDel(&sub1);
                    break ;
                /* Save double quote from double quote at start of string */
		} else if (ajRegExec(FEAT_TV_Regex2,G_String) ) {
		    ajRegSubI (FEAT_TV_Regex2, 1, &sub1); 
		    (void) ajStrApp(&field, sub1);
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex2, &G_String) ;
                /* Or everything until next double quote (could be empty string?) */
                } else if (ajStrLen(G_String) && ajRegExec(FEAT_TV_Regex3,G_String) ) {
                    ajRegSubI (FEAT_TV_Regex3, 1, &sub1); 
		    (void) ajStrApp(&field, sub1);
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex3, &G_String) ;
                /* Or something is wrong */
                } else {
	            ajWarn("Unbalanced double quote in group field:\n\t%s\n\n", groupfield) ;
                    return ;
                }
		if(sub1)
		  ajStrDel(&sub1);
	    } else {
		/* Are at start of new quoted string */
		if ( ajRegExec(FEAT_TV_Regex4,G_String)) {
		    q = ajTrue ;
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex4, &G_String) ;
                /* Or see a tag-value delimiter... clear the tag? */
		} else if (ajRegExec(FEAT_TV_Regex5,G_String) ) {
		    end_tag = ajTrue ; 
                    (void) ajStrDelReuse(&field) ;
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex5, &G_String) ;
		    break ;
		/* or have the start of a '#' */
		} else if ( ajRegExec(FEAT_Regex_comment,G_String) ) {
		    ajRegSubI (FEAT_Regex_comment, 1, &(gf->Comment)); 
		    end_tag = ajTrue ; /* EOL simulated end of tag-value */
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_Regex_comment, &G_String) ;
		    break ;
                /* Or have an unquoted string */
		} else if ( ajRegExec(FEAT_TV_Regex6,G_String) ) {
                    ajRegSubI (FEAT_TV_Regex6, 1, &field); 
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex6, &G_String) ;
		    if(ajStrLen(G_String) == 0) {
			end_tag = ajTrue ;   /* EOL simulated end of tag-value */
		    }
		    break ;
                /* Or have spaces on start of line */
		} else {
                    (void) ajRegExec(FEAT_TV_Regex7,G_String) ;
		    /* effectively deletes the prefix matched text */
                    (void) ajRegPost (FEAT_TV_Regex7, &G_String) ;
                    /* Exit infinite loop if nothing left */
	            if(ajStrLen(G_String) == 0) break ;
		}
	     }
	 } /* end for(;;) */

	 if (field) { 
            if( tag ) {
	      if(value){
		ajStrAppC(&value," ");
	      }
	      (void) ajStrApp(&value,field) ;
            } else {
               (void) ajStrAssS(&tag,field) ;
            }
	    (void) ajStrDelReuse(&field) ;
         }
         if(end_tag) {
	     if( tag ) {
		(void) ajFeatSetTagValue(gf,tag,value,ajFalse) ;
	     }
	     end_tag = ajFalse ; 
	     ajStrDel(&tag) ;
	     value = NULL;
	 }
      } /* end while(ajStrLen(G_String)) */

   } /* else Version 2 */
}

/* @funcstatic GFFromLine ********************************************
**
** Converts an input GFF format line into a feature
**
** @param [r] seqmap [AjPFeatTable] Feature table
** @param [r] line [AjPStr] Input line
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature GFFromLine ( AjPFeatTable seqmap, AjPStr line )
{
    AjPFeature gf    = NULL ;
    AjPStrTok split  = NULL  ;
    static AjPStr
      seqname   = NULL,
      start     = NULL,
      end       = NULL,
      score     = NULL,
      strandstr = NULL,
      framestr  = NULL,
      tagvalue  = NULL ;
    AjPStr			/* used in tables */
      source  = NULL,
      feature = NULL;
    int Start;
    int End;

    if(!line) return NULL ;

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
    } else {  /* optional group && 
                 feature object construction */
        AjEFeatStrand   strand ;
        AjEFeatFrame    frame ;
        AjPStr          groupfield = NULL ;

        if(!ajStrToInt (start, &Start))
           Start = 0 ;
        if(!ajStrToInt (end,   &End))
           End   = 0 ;

        if(!ajStrCmpC(strandstr,"+")) {
           strand = AjStrandWatson ;
        } else if( !ajStrCmpC(strandstr,"-")) {
           strand = AjStrandCrick ;
        } else {
           strand = AjStrandUnknown ;
        }

        if(!ajStrCmpC(framestr,"0")) {
           frame = AjFrameZero ;
        } else if( !ajStrCmpC(framestr,"1")) {
           frame = AjFrameOne ;
        } else if( !ajStrCmpC(framestr,"2")) {
           frame = AjFrameTwo ;
        } else {
           frame = AjFrameUnknown ;
        }

        gf = FeatureNew( seqmap,
                           source, 
                           feature,
                           Start, End,
                           score,
                           strand,
                           frame, NULL,0,0 ,FEATURE_MOTHER) ;
        if( ajStrTokenRest(&groupfield, &split))
           GFFProcessTagValues( (AjPFeature)gf, groupfield) ;
    }
    
Error:
    ajStrTokenClear(&split) ;

    ajStrDel(&source) ;
    ajStrDel(&feature) ;

    (void) ajStrDelReuse(&seqname) ;
    (void) ajStrDelReuse(&start) ;
    (void) ajStrDelReuse(&end) ;
    (void) ajStrDelReuse(&score) ;
    (void) ajStrDelReuse(&strandstr) ;
    (void) ajStrDelReuse(&framestr) ;
    (void) ajStrDelReuse(&tagvalue) ;

    return gf ;
}
/* @funcstatic featGenbankFromLine ********************************************
**
** Read input file in Genbank format
**
** Format is :-
** 5-12   Keyname
** 14-19  From
** 21-26  To
** 34-74  Descrition
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] origline [AjPStr] Input buffered file
** @param [r] genbank [int] genbank or Ddbj (1 or 0)
** @return [AjPFeature] NULL if error.
** @@
******************************************************************************/

static AjPFeature featGenbankFromLine ( AjPFeatTable thys, AjPStr origline, int genbank)
{
  static AjPFeature gf    = NULL ;      /* made static so that tag-values can be added LATER */
  static AjPStr
    source    = NULL,
    feature   = NULL,
    start     = NULL,
    end       = NULL,
    start2    = NULL,
    end2      = NULL,
    line      = NULL;
  AjEFeatStrand   strand =AjStrandUnknown;
  int Start=0, End=0;
  int Start2=0, End2=0;
  AjBool okay=ajFalse;
  AjBool mother=ajFalse;       /* is it the first feature of a set */
  static AjBool join=0;         /* is it part of a join, multiple set data */
  static AjBool lastwasafeature = 1,complement=0;
  AjPStr temp=NULL;
  int val=0,flags=0,startpos=0;

  if(!source){
    if(genbank)
      source = ajStrNewC("Genbank");
    else
      source = ajStrNewC("Ddbj");
  }
  
  ajStrAssS(&line,origline);              /* Need to copy as BufferFile cannot be edited */

  if(ajRegExec(EMBL_Regex_New, line)) {             /* if new feature initialise for it */
    ajRegSubI (EMBL_Regex_New, 1, &feature);
    ajStrChomp(&feature);
    lastwasafeature = ajTrue;
    complement=ajFalse;
    mother = ajTrue;
    join = ajFalse;
  }
  else if(ajRegExec(FEAT_TV_Regex, line)){          
    lastwasafeature = ajFalse;
  }
  if(lastwasafeature){
    if(ajRegExec(EMBL_Regex_Complement, line)){ /* remove ^     FEAT   complement*/
      complement = ajTrue;
      ajRegPost(EMBL_Regex_Complement,&line);
    }
    if(ajRegExec(EMBL_Regex_Join, line)){              /* remove ^   FEAT join */
      ajRegPost(EMBL_Regex_Join,&line);
      join = ajTrue;
    }
    if(ajRegExec(EMBL_Regex_Join2, line)){              /* join ? */
      join = ajTrue;
    }
    if (ajRegExec(EMBL_Regex_Complement2, line)){      /* complement(x..y */
      do{
	ajRegSubI (EMBL_Regex_Complement2, 1, &start);
	ajRegSubI (EMBL_Regex_Complement2, 2, &end);

	startpos = ajRegOffset(EMBL_Regex_Complement2);       /* remove complement(x..y from the line */
	ajRegPost(EMBL_Regex_Complement2,&temp);              /* as this has now been processed */
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}
	val = featgetpos(&start,&Start);                      /* convert to an integer value */
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);                          /* convert to an integer value */
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(complement)                                        /* if its complement( complement( */
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;


	if(mother)                                            /* Mother-> start of feature region */
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);

	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL,0,0, flags ) ;
	mother=ajFalse;
	flags=0;
	
      } while (ajRegExec(EMBL_Regex_Complement2, line));
    }
    if(ajRegExec(EMBL_Regex_Location2, line)){               /* SEE ABOVE for most documentation */
      do{
	ajRegSubI (EMBL_Regex_Location2, 1, &start);
	ajRegSubI (EMBL_Regex_Location2, 2, &start2);
	ajRegSubI (EMBL_Regex_Location2, 3, &end);
	ajRegSubI (EMBL_Regex_Location2, 4, &end2);
	ajFeatSetFlag(&flags,FEATURE_START_TWO);
	ajFeatSetFlag(&flags,FEATURE_END_TWO);


	startpos = ajRegOffset(EMBL_Regex_Location2);
	ajRegPost(EMBL_Regex_Location2,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}
	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

	val = featgetpos(&start2,&Start2);
	if(val==-1)
	  ajDebug("Error getting start2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end2,&End2);
	if(val==-1)
	  ajDebug("Error getting end2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);


        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);

	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, Start2, End2, flags ) ;
	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location2, line));
    }
    if(ajRegExec(EMBL_Regex_Location, line)){
      do{
	ajRegSubI (EMBL_Regex_Location, 1, &start);
	ajRegSubI (EMBL_Regex_Location, 2, &end);

	startpos = ajRegOffset(EMBL_Regex_Location);
	ajRegPost(EMBL_Regex_Location,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;
	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);
	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL,0,0, flags ) ;
	mother=ajFalse;
	flags=0;
      } while (ajRegExec(EMBL_Regex_Location, line));
    }
    if(ajRegExec(EMBL_Regex_Location3, line)){
      do{
	ajRegSubI (EMBL_Regex_Location3, 1, &start);
	ajRegSubI (EMBL_Regex_Location3, 2, &start2);
	ajRegSubI (EMBL_Regex_Location3, 3, &end);
	ajFeatSetFlag(&flags,FEATURE_START_TWO);

	startpos = ajRegOffset(EMBL_Regex_Location3);
	ajRegPost(EMBL_Regex_Location3,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

	val = featgetpos(&start2,&Start2);
	if(val==-1)
	  ajDebug("Error getting start2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, Start2, 0, flags ) ;

	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location3, line));
    }
    if(ajRegExec(EMBL_Regex_Location4, line)){
      do{
	ajRegSubI (EMBL_Regex_Location4, 1, &start);
	ajRegSubI (EMBL_Regex_Location4, 2, &end);
	ajRegSubI (EMBL_Regex_Location4, 3, &end2);
	ajFeatSetFlag(&flags,FEATURE_END_TWO);

	startpos = ajRegOffset(EMBL_Regex_Location4);
	ajRegPost(EMBL_Regex_Location4,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);


	val = featgetpos(&end2,&End2);
	if(val==-1)
	  ajDebug("Error getting end2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, 0, End2, flags ) ;

	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location4, line));
    }
    if(ajRegExec(EMBL_Regex_Location5, line)){
      do{
	ajRegSubI (EMBL_Regex_Location5, 1, &start);
	ajRegSubI (EMBL_Regex_Location5, 2, &end);
	ajFeatSetFlag(&flags,FEATURE_BETWEEN_SEQ);

	startpos = ajRegOffset(EMBL_Regex_Location5);
	ajRegPost(EMBL_Regex_Location5,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting end (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);

	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, 0,0, flags ) ;

	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location5, line));
    }

    if(ajRegExec(EMBL_Regex_Complement, line)){
      ajRegSubI (EMBL_Regex_Complement, 1, &feature);
      ajStrChomp(&feature);
      ajRegSubI (EMBL_Regex_Complement, 2, &start); 
      ajRegSubI (EMBL_Regex_Complement, 3, &end); 
      if(complement) 
	strand = AjStrandWatson ;
      else 
	strand = AjStrandCrick ;
      okay =ajTrue;
    }
    if(ajRegExec(EMBL_Regex_SourceLine, line)){
      ajRegSubI (EMBL_Regex_SourceLine, 2, &start); 
      ajRegSubI (EMBL_Regex_SourceLine, 3, &end); 
      okay = ajTrue;
    }
    if(ajRegExec(EMBL_Regex_SourcePoint, line)){
      ajRegSubI (EMBL_Regex_SourcePoint, 2, &start);
      ajStrAss(&end,start);
      ajFeatSetFlag(&flags,FEATURE_POINT);
      okay = ajTrue;
    }
    if(okay){

      val = featgetpos(&start,&Start);
      if(val==-1)
	ajDebug("Error getting start (set to 0) for %S\n",line) ;
      else if(val)
	ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);
      
      val = featgetpos(&end,&End);
      if(val==-1)
	ajDebug("Error getting start (set to 0) for %S\n",line) ;
      else if(val)
	ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);
      

      if(mother)
	ajFeatSetFlag(&flags,FEATURE_MOTHER);
      if(join)
	ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

      gf = FeatureNew( thys,
			 source, 
			 feature,
			 Start, End,
			 NULL,
			 strand,
			 AjFrameUnknown,
			 NULL, 0,0, flags ) ;

      mother=ajFalse;
      flags=0;

    }
    ajStrDelReuse(&line);
    ajStrDelReuse(&start);
    ajStrDelReuse(&end);
    ajStrDelReuse(&start2);
    ajStrDelReuse(&end2);
  return gf;
  }
  else { /* tag value */
    
    EMBLProcessTagValues( (AjPFeature)gf, line) ;
    ajStrDelReuse(&line);
  }
  return gf;

}

/* @funcstatic featSwissFromLine ********************************************
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
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] line [AjPStr] Input buffered file
** @return [AjPFeature] NULL if error.
** @@
******************************************************************************/

static AjPFeature featSwissFromLine ( AjPFeatTable thys, AjPStr line)
{
  static AjPStr
    source    = NULL,
    feature   = NULL,
    start     = NULL,
    end       = NULL,
    score     = NULL,
    desc      = NULL;
  AjEFeatFrame    frame = AjFrameUnknown ;
  AjEFeatStrand   strand  = AjStrandUnknown ;
  int flags = 0;
  static AjPFeature gf    = NULL ;           /* made static so that it's easy
						to add second line of description */
  int Start, End, val;

  if(!source){
    source    = ajStrNewC("SWISSPROT");
    feature   = ajStrNewL(10);
    start     = ajStrNewL(8);
    end       = ajStrNewL(8);
    desc      = ajStrNewL(81);
  }
  
  /* reg exp has already checked that line starts with FT */
  
  /*STR MUST BE AT least 22 chars long (must has a to)*/
  if(ajStrLen(line) < 22)
    return gf;
  
  /* Get Keyname ---> feature in gff */
  ajStrAssSub(&feature,line,5,12);
  ajStrChomp(&feature); 
  if(!ajStrLen(feature)){
    /*
      Actually test for description. If it exist then add to
      static gf's already existing description.
    */
    if(!gf)
      return NULL;
    ajStrAssC(&desc,"");
    ajStrAssSub(&desc,line,34,74);
    ajStrChomp(&desc); 
    if(ajStrLen(desc)){
      ajStrApp(&(gf->desc),desc);
      ajStrDel(&desc);
      return NULL; /* entry added already */
    }
    else {
      ajStrDel(&desc);
      return NULL;
    }
  }
  
  /* Get the start point */
  ajStrAssSub(&start,line,14,19);
  ajStrChomp(&start); 
  if(!ajStrLen(start)){
    return NULL;
  }

  /* Get the end point */
  ajStrAssSub(&end,line,21,26);
  ajStrChomp(&end); 
  if(!ajStrLen(end)){
    return NULL;
  }

  /* Get the description THIS IS OPTIONAL */
  ajStrAssC(&desc,"");
  ajStrAssSub(&desc,line,34,74);
  ajStrChomp(&desc); 

  val = featgetpos(&start,&Start);
  if(val==-1)
    return NULL;
  else if(val)
    ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

  val = featgetpos(&end,   &End);
  if(val==-1)
    return NULL;
  else if (val)
    ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

  ajFeatSetFlag(&flags,FEATURE_MOTHER);

  ajDebug("flags = %d\n",flags);

  gf = FeatureNew( thys,
		     source, 
		     feature,
		     Start, End,
		     score,
		     strand,
		     frame,
		     desc,0,0, flags ) ;
  

  return gf;

}
/* @funcstatic featEmblFromLine ********************************************
**
** Converts an input EMBL format line into a feature
**
** @param [r] thys     [AjPFeatTable] Feature table
** @param [r] origline [AjPStr] Input line
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featEmblFromLine ( AjPFeatTable thys, AjPStr origline )
{
  static AjPFeature gf    = NULL ;      /* made static so that tag-values can be added LATER */
  static AjPStr
    /*    seqname   = NULL,*/
    source    = NULL,
    feature   = NULL,
    start     = NULL,
    end       = NULL,
    start2    = NULL,
    end2      = NULL,
    line      = NULL;
  AjEFeatStrand   strand =AjStrandUnknown;
  int Start=0, End=0;
  int Start2=0, End2=0;
  AjBool okay=ajFalse;
  AjBool mother=ajFalse;       /* is it the first feature of a set */
  static AjBool join=0;         /* is it part of a join, multiple set data */
  static AjBool lastwasafeature = 1,complement=0;
  AjPStr temp=NULL;
  int val=0,flags=0,startpos=0;

  if(!source)
    source = ajStrNewC("EMBL");

  ajStrAssS(&line,origline);              /* Need to copy as BufferFile cannot be edited */

  if(ajRegExec(EMBL_Regex_New, line)) {             /* if new feature initialise for it */
    ajRegSubI (EMBL_Regex_New, 1, &feature);
    ajStrChomp(&feature);
    lastwasafeature = ajTrue;
    complement=ajFalse;
    mother = ajTrue;
    join = ajFalse;
  }
  else if(ajRegExec(FEAT_TV_Regex, line)){          
    lastwasafeature = ajFalse;
  }
  if(lastwasafeature){
    if(ajRegExec(EMBL_Regex_Complement, line)){ /* remove ^FT   FEAT   complement*/
      complement = ajTrue;
      ajFeatSetFlag(&flags,FEATURE_COMPLEMENT_MAIN);
      ajRegPost(EMBL_Regex_Complement,&line);
    }
    if(ajRegExec(EMBL_Regex_Join, line)){              /* remove ^FT FEAT join */
      ajRegPost(EMBL_Regex_Join,&line);
      join = ajTrue;
    }
    if(ajRegExec(EMBL_Regex_Join2, line)){              /* join ? */
      join = ajTrue;
    }
    if (ajRegExec(EMBL_Regex_Complement2, line)){      /* complement(x..y */
      do{
	ajRegSubI (EMBL_Regex_Complement2, 1, &start);
	ajRegSubI (EMBL_Regex_Complement2, 2, &end);

	startpos = ajRegOffset(EMBL_Regex_Complement2);       /* remove complement(x..y from the line */
	ajRegPost(EMBL_Regex_Complement2,&temp);              /* as this has now been processed */
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}
	val = featgetpos(&start,&Start);                      /* convert to an integer value */
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);                          /* convert to an integer value */
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(complement)                                        /* if its complement( complement( */
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;


	if(mother)                                            /* Mother-> start of feature region */
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);

	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL,0,0, flags ) ;
	mother=ajFalse;
	flags=0;
	
      } while (ajRegExec(EMBL_Regex_Complement2, line));
    }
    if(ajRegExec(EMBL_Regex_Location2, line)){               /* SEE ABOVE for most documentation */
      do{
	ajRegSubI (EMBL_Regex_Location2, 1, &start);
	ajRegSubI (EMBL_Regex_Location2, 2, &start2);
	ajRegSubI (EMBL_Regex_Location2, 3, &end);
	ajRegSubI (EMBL_Regex_Location2, 4, &end2);
	ajFeatSetFlag(&flags,FEATURE_START_TWO);
	ajFeatSetFlag(&flags,FEATURE_END_TWO);


	startpos = ajRegOffset(EMBL_Regex_Location2);
	ajRegPost(EMBL_Regex_Location2,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}
	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

	val = featgetpos(&start2,&Start2);
	if(val==-1)
	  ajDebug("Error getting start2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end2,&End2);
	if(val==-1)
	  ajDebug("Error getting end2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);


        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, Start2, End2, flags ) ;
	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location2, line));
    }
    if(ajRegExec(EMBL_Regex_Location, line)){
      do{
	ajRegSubI (EMBL_Regex_Location, 1, &start);
	ajRegSubI (EMBL_Regex_Location, 2, &end);

	startpos = ajRegOffset(EMBL_Regex_Location);
	ajRegPost(EMBL_Regex_Location,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;
	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);
	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL,0,0, flags ) ;
	mother=ajFalse;
	flags=0;
      } while (ajRegExec(EMBL_Regex_Location, line));
    }
    if(ajRegExec(EMBL_Regex_Location3, line)){
      do{
	ajRegSubI (EMBL_Regex_Location3, 1, &start);
	ajRegSubI (EMBL_Regex_Location3, 2, &start2);
	ajRegSubI (EMBL_Regex_Location3, 3, &end);
	ajFeatSetFlag(&flags,FEATURE_START_TWO);

	startpos = ajRegOffset(EMBL_Regex_Location3);
	ajRegPost(EMBL_Regex_Location3,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

	val = featgetpos(&start2,&Start2);
	if(val==-1)
	  ajDebug("Error getting start2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, Start2, 0, flags ) ;

	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location3, line));
    }
    if(ajRegExec(EMBL_Regex_Location4, line)){
      do{
	ajRegSubI (EMBL_Regex_Location4, 1, &start);
	ajRegSubI (EMBL_Regex_Location4, 2, &end);
	ajRegSubI (EMBL_Regex_Location4, 3, &end2);
	ajFeatSetFlag(&flags,FEATURE_END_TWO);

	startpos = ajRegOffset(EMBL_Regex_Location4);
	ajRegPost(EMBL_Regex_Location4,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);


	val = featgetpos(&end2,&End2);
	if(val==-1)
	  ajDebug("Error getting end2 (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, 0, End2, flags ) ;

	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location4, line));
    }
    if(ajRegExec(EMBL_Regex_Location5, line)){
      do{
	ajRegSubI (EMBL_Regex_Location5, 1, &start);
	ajRegSubI (EMBL_Regex_Location5, 2, &end);
	ajFeatSetFlag(&flags,FEATURE_BETWEEN_SEQ);

	startpos = ajRegOffset(EMBL_Regex_Location5);
	ajRegPost(EMBL_Regex_Location5,&temp);
	ajStrTruncate(&line,startpos);
	if(temp){
	  ajStrApp(&line,temp);
	  ajStrDel(&temp);
	}

	val = featgetpos(&start,&Start);
	if(val==-1)
	  ajDebug("Error getting start (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);

	val = featgetpos(&end,&End);
	if(val==-1)
	  ajDebug("Error getting end (set to 0) for %S\n",line) ;
	else if(val)
	  ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);

        if(!complement) 
           strand = AjStrandWatson ;
        else 
           strand = AjStrandCrick ;

	if(mother)
	  ajFeatSetFlag(&flags,FEATURE_MOTHER);
	if(join)
	  ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

	gf = FeatureNew( thys,
			   source, 
			   feature,
			   Start, End,
			   NULL,
			   strand,
			   AjFrameUnknown,
			   NULL, 0,0, flags ) ;

	mother=ajFalse;
	flags=0;

      } while (ajRegExec(EMBL_Regex_Location5, line));
    }

    if(ajRegExec(EMBL_Regex_Complement, line)){
      ajRegSubI (EMBL_Regex_Complement, 1, &feature);
      ajStrChomp(&feature);
      ajRegSubI (EMBL_Regex_Complement, 2, &start); 
      ajRegSubI (EMBL_Regex_Complement, 3, &end); 
      if(complement) 
	strand = AjStrandWatson ;
      else 
	strand = AjStrandCrick ;
      okay =ajTrue;
    }
    if(ajRegExec(EMBL_Regex_SourceLine, line)){
      ajRegSubI (EMBL_Regex_SourceLine, 2, &start); 
      ajRegSubI (EMBL_Regex_SourceLine, 3, &end); 
      okay = ajTrue;
    }
    if(ajRegExec(EMBL_Regex_SourcePoint, line)){
      ajRegSubI (EMBL_Regex_SourcePoint, 2, &start);
      ajStrAss(&end,start);
      ajFeatSetFlag(&flags,FEATURE_POINT);
      okay = ajTrue;
    }
    if(okay){

      val = featgetpos(&start,&Start);
      if(val==-1)
	ajDebug("Error getting start (set to 0) for %S\n",line) ;
      else if(val)
	ajFeatSetFlag(&flags,FEATURE_START_BEFORE_SEQ);
      
      val = featgetpos(&end,&End);
      if(val==-1)
	ajDebug("Error getting start (set to 0) for %S\n",line) ;
      else if(val)
	ajFeatSetFlag(&flags,FEATURE_END_AFTER_SEQ);
      

      if(mother)
	ajFeatSetFlag(&flags,FEATURE_MOTHER);
      if(join)
	ajFeatSetFlag(&flags,FEATURE_MULTIPLE);

      gf = FeatureNew( thys,
			 source, 
			 feature,
			 Start, End,
			 NULL,
			 strand,
			 AjFrameUnknown,
			 NULL, 0,0, flags ) ;

      mother=ajFalse;
      flags=0;

    }
    ajStrDelReuse(&line);
    ajStrDelReuse(&start);
    ajStrDelReuse(&end);
    ajStrDelReuse(&start2);
    ajStrDelReuse(&end2);
  return gf;
  }
  else { /* tag value */
    
    EMBLProcessTagValues( (AjPFeature)gf, line) ;
    ajStrDelReuse(&line);
  }
  return gf;
  
}

/* @funcstatic featGffFromLine ********************************************
**
** Converts an input GFF format line into a feature
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] line [AjPStr] Input line
** @return [AjPFeature] New feature
** @@
******************************************************************************/

static AjPFeature featGffFromLine ( AjPFeatTable thys, AjPStr line )
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
    int Start=0, End=0;

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
    } else {  /* optional group && 
                 feature object construction */
        AjEFeatStrand   strand ;
        AjEFeatFrame    frame ;
        AjPStr          groupfield = NULL ;

        if(!ajStrToInt (start, &Start))
           Start = 0 ;
        if(!ajStrToInt (end,   &End))
           End   = 0 ;


        if(!ajStrCmpC(strandstr,"+")) {
           strand = AjStrandWatson ;
        } else if( !ajStrCmpC(strandstr,"-")) {
           strand = AjStrandCrick ;
        } else {
           strand = AjStrandUnknown ;
        }

        if(!ajStrCmpC(framestr,"0")) {
           frame = AjFrameZero ;
        } else if( !ajStrCmpC(framestr,"1")) {
           frame = AjFrameOne ;
        } else if( !ajStrCmpC(framestr,"2")) {
           frame = AjFrameTwo ;
        } else {
           frame = AjFrameUnknown ;
        }

        gf = FeatureNew( thys,
                           source, 
                           feature,
                           Start, End,
                           score,
                           strand,
                           frame,
			   NULL,0,0, FEATURE_MOTHER ) ;
        if( ajStrTokenRest(&groupfield, &split))
           GFFProcessTagValues( (AjPFeature)gf, groupfield) ;

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

/* @funcstatic featReadGff ********************************************
**
** Read input file in GFF format
**
** @param [r] thys [AjPFeatTable] Feature table
** @param [r] file [AjPFileBuff] Input buffered file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool featReadGff ( AjPFeatTable thys, AjPFileBuff file)
{
  static AjPStr line  = NULL ;
  AjBool found = ajFalse ;
  thys->Version = DEFAULT_GFF_VERSION ;



  ajDebug("featReadGff..........\n");
  while( ajFileBuffGet (file, &line) ) {

    (void) ajStrChomp(&line) ;

    if(ajRegExec(FEAT_Regex_blankline, line)) {
      ; /* ignore */
    }
    else if(ajRegExec(FEAT_Regex_version,line)) {
      AjPStr version = NULL ;
      ajRegSubI (FEAT_Regex_version, 1, &version); 
      (void) ajStrToFloat (version, &(thys->Version));
      ajStrDel(&version);
    }
    else if(ajRegExec(FEAT_Regex_date,line)) {
      AjPStr year  = NULL ;
      AjPStr month = NULL ;
      AjPStr day   = NULL ;
      int nYear, nMonth, nDay ;
      ajRegSubI (FEAT_Regex_date, 1, &year); 
      ajRegSubI (FEAT_Regex_date, 2, &month); 
      ajRegSubI (FEAT_Regex_date, 3, &day);
      (void) ajStrToInt (year,  &nYear);
      (void) ajStrToInt (month, &nMonth);
      (void) ajStrToInt (day,   &nDay);
      thys->Date = ajTimeSet("GFF",nDay,nMonth,nYear) ;
      ajTimeTrace (thys->Date);
      ajStrDel(&year);
      ajStrDel(&month);
      ajStrDel(&day);
    }
    else if(ajRegExec(FEAT_Regex_region,line)) {
      AjPStr start = NULL ;
      AjPStr end   = NULL ;
      ajRegSubI (FEAT_Regex_region, 2, &start); 
      ajRegSubI (FEAT_Regex_region, 3, &end);
      (void) ajStrToInt (start, &(thys->Start));
      (void) ajStrToInt (end,   &(thys->End));
      ajStrDel(&start);
      ajStrDel(&end);
    }
    else if(ajRegExec(FEAT_Regex_comment,line)) {
      ; /* ignore for now... could store them in 
	   AjFeatTable for future reference though?...*/
    }
    else {
      if(featGffFromLine(thys, line))  /* does the ajFeatTabAdd */
	found = ajTrue ;
    }
    ajStrDelReuse(&line);
  }
  return found ;
}

/* @funcstatic featDumpEmbl ********************************************
**
** Write details of single feature to debug file
**
** @param [r] feat     [AjPFeature] Feature
** @param [r] location [AjPStr] location list
** @param [r] file     [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpEmbl (AjPFeature feat, AjPStr location, AjPFile file){
  static AjPStr new=NULL;
  AjPStr temp = NULL,limited=NULL;
  int i=0,last=0,val=0;
  AjBool okay = ajTrue,first = ajTrue;
  AjIList           iter      = NULL, iter2 = NULL ;
  int space=0;
  LPFeatTagValue item = NULL;  
  AjPFeatVocTagForFeat item2 = NULL;
  AjPFeatVocTag man=NULL;
  AjBool found;
  AjPFeatVocFeat outType;
  AjPFeatVocTag outVocTag;

  if(!feat->Type)        /* feature type invalid so do not dump out */
    return;

  if(OUTPUT_DICTIONARY)
    outType = ajFeatVocFeatKey(OUTPUT_DICTIONARY,feat->Type->name);
  else
    outType = feat->Type;

  if(!outType){
    ajWarn("%S not found in output dictionary",feat->Type->name);
    return;
  }

  /* If a feature requires a mandatory value check to see that it is there
     before starting to print everything out */

  if(outType->flags & TAG_MANDATORY){ /* but at the moment we do not know which is mandatory */
    if (outType->Tags) {
      iter = ajListIter(outType->Tags) ;
      while(ajListIterMore(iter)) {
	item2 = (AjPFeatVocTagForFeat)ajListIterNext (iter) ;
	if(item2->mandatory){
	  man = item2->VocTag;
	  break;
	}
      }
    }
    if(!man){
      ajDebug("ERROR: Feature flags mandatory but none found in list for %S\n",feat->Type->name);
    }
    else{
      okay = ajFalse;
      iter = ajListIter(feat->Tags) ;
      while(ajListIterMore(iter)) {
	item = (LPFeatTagValue) ajListIterNext (iter) ;
	if(item->Tag->VocTag == man)
	  okay = ajTrue;
      }
    }
    if(!okay){
      ajWarn("Mandatory tag %S missing for Feature %S so WHOLE feature ignored!!!!\n",man->name,feat->Type->name);
      return;
    }
    else
      ajDebug("Mandatory tag %S for Feature %S found :-)\n",man->name,feat->Type->name);
  }
  
  if(!new){
    new = ajStrNewL(81);
  }
  if(ajStrLen(location) < 58){
    ajStrAssC(&new,"FT   ");

    ajStrApp(&new,outType->name);

    for(i=ajStrLen(new); i< 21;i++)         /* EMBL location start at char 22 */
      ajStrAppC(&new," ");
    (void) ajFmtPrintF (file, "%S%S\n",new,location);
  }
  else{ /* need to split location onto multiple lines as it is too big */
    temp = ajStrNewL(60);
    last = 0;
    okay = ajTrue;
    while(okay){
      ajStrAssSub(&temp,location,last,last+58); /* copy it first */
      ajStrChomp(&temp);
      val = ajStrRFindC(temp,",");     /* find the last , in this to cut at */
      
      /* now split location */
      if(ajStrLen(temp) < 58){ /* if all fits then this is the last one */
	okay = ajFalse;
      }
      else if ( val != -1)
	ajStrTruncate(&temp,val+1);
      if(first){
	ajStrAssC(&new,"FT   ");

	ajStrApp(&new,outType->name);

	for(i=ajStrLen(new); i< 21;i++)         /* EMBL location start at char 22 */
	  ajStrAppC(&new," ");
	first = ajFalse;
      }
      else{
	ajStrAssC(&new,"FT                   ");
      }
      (void) ajFmtPrintF (file, "%S%S\n",new,temp);
      last += val+1;
    }
    ajStrDel(&temp);
  }

  if (feat->Tags) {
    iter = ajListIter(feat->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue) ajListIterNext (iter) ;

      if(OUTPUT_DICTIONARY)
	outVocTag = ajFeatVocTagKey(OUTPUT_DICTIONARY,item->Tag->VocTag->name);
      else
	outVocTag = item->Tag->VocTag;

      if(!outVocTag){
	ajWarn("Tag %S not found in Dictionary hence ignored",item->Tag->VocTag->name);
	break;
      }
      /* check if limited and if so make sure it matches one of the available then */
      if(outVocTag->flags & TAG_LIMITED){
	/*      if(item->Tag->VocTag->flags & TAG_LIMITED){*/
	found= ajFalse;
	iter2 = ajListIter(outVocTag->limitedValues) ;
	while(ajListIterMore(iter2)) {
	  limited = (AjPStr) ajListIterNext (iter2) ;
	  if(!ajStrCmp(&limited,&item->Value))
	    found= ajTrue;
	}
	ajListIterFree(iter2) ;   
	if(!found){
	  ajWarn("%S not a recognised Limited Value",item->Value);
	  ajWarn("\thence tag %S not output.",outVocTag->name);
	  ajWarn("\tvalid values are:-",outVocTag->name);
	  iter2 = ajListIter(outVocTag->limitedValues) ;
	  while(ajListIterMore(iter2)) {
	    limited = (AjPStr) ajListIterNext (iter2) ;
	    ajWarn("\t\t%S",limited);
	  }
	  ajListIterFree(iter2) ;   
	  continue;
	}
      }

      ajStrAssC(&new,"FT                   /");
      if(!outVocTag)         
	ajStrAppC(&new,"note");
      else
	ajStrApp(&new,outVocTag->name);

      if(outVocTag->flags & TAG_QTEXT)
	ajStrAppC(&new,"=\"");
      else
	ajStrAppC(&new,"=");

      if(item->Value && !(outVocTag->flags & TAG_VOID)) {
        AjPStr s = item->Value ;
	last = 0;
	first = ajTrue;
	if(79-ajStrLen(new) < ajStrLen(s)){ /* multiple tag lines */
	  space = 79-ajStrLen(new);
	  okay = ajTrue;
	  while(okay){
	    ajStrAssSub(&temp,s,last,last+space); /* copy it first */
	    /*	    ajStrChomp(&temp);*/
	    if(ajStrLen(temp) < space) /* if all fits then this is the last one */
	      okay = ajFalse;
	    if(first){
	      (void) ajFmtPrintF (file,"%S%S",new,temp);
	      ajStrDel(&temp);
	      first = ajFalse;
	    }
	    else {
	      (void) ajFmtPrintF (file,"\n%S%S",new,temp);
	      ajStrDel(&temp);
	    }
	    ajStrAssC(&new,"FT                   ");
	    last += space+1;
	    space = 58;
	  }
	  (void) ajFmtPrintF (file,"\"\n");	  
	}
	else{
	  ajStrApp(&new,s);
	  if(outVocTag->flags & TAG_QTEXT)
	    ajStrAppC(&new,"\"");

	  ajStrAppC(&new,"\n");
	  (void) ajFmtPrintF (file,"%S",new);
	}
      }
      else{
	ajStrChop(&new);
	ajStrChop(&new);
	(void) ajFmtPrintF (file,"%S\n",new);
      }
    }
    ajListIterFree(iter) ;   
  }


  ajStrDelReuse(&new);
   
}

/* @funcstatic featDumpGenbank ********************************************
**
** Write details of single feature to debug file
**
** @param [r] feat     [AjPFeature] Feature
** @param [r] location [AjPStr] location list
** @param [r] file     [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGenbank (AjPFeature feat, AjPStr location, AjPFile file){
  static AjPStr new=NULL;
  AjPStr temp = NULL,limited=NULL;
  int i=0,last=0,val=0;
  AjBool okay = ajTrue,first = ajTrue;
  AjIList           iter      = NULL, iter2 = NULL ;
  int space=0;
  LPFeatTagValue item = NULL;  
  AjPFeatVocTagForFeat item2 = NULL;
  AjPFeatVocTag man=NULL;
  AjBool found;
  AjPFeatVocFeat outType;
  AjPFeatVocTag outVocTag;

  if(!feat->Type)        /* feature type invalid so do not dump out */
    return;

  if(OUTPUT_DICTIONARY)
    outType = ajFeatVocFeatKey(OUTPUT_DICTIONARY,feat->Type->name);
  else
    outType = feat->Type;

  if(!outType){
    ajWarn("%S not found in output dictionary",feat->Type->name);
    return;
  }

  /* If a feature requires a mandatory value check to see that it is there
     before starting to print everything out */

  if(outType->flags & TAG_MANDATORY){ /* but at the moment we do not know which is mandatory */
    if (outType->Tags) {
      iter = ajListIter(outType->Tags) ;
      while(ajListIterMore(iter)) {
	item2 = (AjPFeatVocTagForFeat)ajListIterNext (iter) ;
	if(item2->mandatory){
	  man = item2->VocTag;
	  break;
	}
      }
    }
    if(!man){
      ajDebug("ERROR: Feature flags mandatory but none found in list for %S\n",feat->Type->name);
    }
    else{
      okay = ajFalse;
      iter = ajListIter(feat->Tags) ;
      while(ajListIterMore(iter)) {
	item = (LPFeatTagValue) ajListIterNext (iter) ;
	if(item->Tag->VocTag == man)
	  okay = ajTrue;
      }
    }
    if(!okay){
      ajWarn("Mandatory tag %S missing for Feature %S so WHOLE feature ignored!!!!\n",man->name,feat->Type->name);
      return;
    }
    else
      ajDebug("Mandatory tag %S for Feature %S found :-)\n",man->name,feat->Type->name);
  }
  
  if(!new){
    new = ajStrNewL(81);
  }
  if(ajStrLen(location) < 58){
    ajStrAssC(&new,"     ");
    ajStrApp(&new,outType->name);

    for(i=ajStrLen(new); i< 21;i++)         /* EMBL location start at char 22 */
      ajStrAppC(&new," ");
    (void) ajFmtPrintF (file, "%S%S\n",new,location);
  }
  else{ /* need to split location onto multiple lines as it is too big */
    temp = ajStrNewL(60);
    last = 0;
    okay = ajTrue;
    while(okay){
      ajStrAssSub(&temp,location,last,last+58); /* copy it first */
      ajStrChomp(&temp);
      val = ajStrRFindC(temp,",");     /* find the last , in this to cut at */
      
      /* now split location */
      if(ajStrLen(temp) < 58){ /* if all fits then this is the last one */
	okay = ajFalse;
      }
      else if ( val != -1)
	ajStrTruncate(&temp,val+1);
      if(first){
	ajStrAssC(&new,"     ");
	ajStrApp(&new,outType->name);
	for(i=ajStrLen(new); i< 21;i++)         /* EMBL location start at char 22 */
	  ajStrAppC(&new," ");
	first = ajFalse;
      }
      else{
	ajStrAssC(&new,"                     ");
      }
      (void) ajFmtPrintF (file, "%S%S\n",new,temp);
      last += val+1;
    }
    ajStrDel(&temp);
  }

  if (feat->Tags) {
    iter = ajListIter(feat->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue) ajListIterNext (iter) ;

      if(OUTPUT_DICTIONARY)
	outVocTag = ajFeatVocTagKey(OUTPUT_DICTIONARY,item->Tag->VocTag->name);
      else
	outVocTag = item->Tag->VocTag;

      if(!outVocTag){
	ajWarn("Tag %S not found in Dictionary hence ignored",item->Tag->VocTag->name);
	break;
      }
      /* check if limited and if so make sure it matches one of the available then */
      if(outVocTag->flags & TAG_LIMITED){
	found= ajFalse;
	iter2 = ajListIter(outVocTag->limitedValues) ;
	while(ajListIterMore(iter2)) {
	  limited = (AjPStr) ajListIterNext (iter2) ;
	  if(!ajStrCmp(&limited,&item->Value))
	    found= ajTrue;
	}
	ajListIterFree(iter2) ;   
	if(!found){
	  ajWarn("%S not a recognised Limited Value",item->Value);
	  ajWarn("\thence tag %S not output.",outVocTag->name);
	  ajWarn("\tvalid values are:-",outVocTag->name);
	  iter2 = ajListIter(outVocTag->limitedValues) ;
	  while(ajListIterMore(iter2)) {
	    limited = (AjPStr) ajListIterNext (iter2) ;
	    ajWarn("\t\t%S",limited);
	  }
	  ajListIterFree(iter2) ;   
	  continue;
	}
      }

      ajStrAssC(&new,"                     /");
      if(!outVocTag)         
	ajStrAppC(&new,"note");
      else
	ajStrApp(&new,outVocTag->name);

      if(outVocTag->flags & TAG_QTEXT)
	ajStrAppC(&new,"=\"");
      else
	ajStrAppC(&new,"=");

      if(item->Value && !(outVocTag->flags & TAG_VOID)) {
        AjPStr s = item->Value ;
	last = 0;
	first = ajTrue;
	if(79-ajStrLen(new) < ajStrLen(s)){ /* multiple tag lines */
	  space = 79-ajStrLen(new);
	  okay = ajTrue;
	  while(okay){
	    ajStrAssSub(&temp,s,last,last+space); /* copy it first */
	    ajStrChomp(&temp);
	    if(ajStrLen(temp) < space) /* if all fits then this is the last one */
	      okay = ajFalse;
	    if(first){
	      (void) ajFmtPrintF (file,"%S%S",new,temp);
	      first = ajFalse;
	    }
	    else {
	      (void) ajFmtPrintF (file,"\n%S%S",new,temp);
	    }
	    ajStrAssC(&new,"                     ");
	    last += space+1;
	    space = 58;
	  }
	  (void) ajFmtPrintF (file,"\"\n");	  
	}
	else{
	  ajStrApp(&new,s);
	  if(outVocTag->flags & TAG_QTEXT)
	    ajStrAppC(&new,"\"");

	  ajStrAppC(&new,"\n");
	  (void) ajFmtPrintF (file,"%S",new);
	}
      }
      else{
	ajStrChop(&new);
	ajStrChop(&new);
	(void) ajFmtPrintF (file,"%S\n",new);
      }
    }
    ajListIterFree(iter) ;   
  }


  ajStrDelReuse(&new);
   
}




/* @funcstatic featDumpSwiss ********************************************
**
** Write details of single feature to debug file
**
** @param [r] feat [AjPFeature] Feature
** @param [r] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpSwiss (AjPFeature feat, AjPFile file)
{
  AjPStr new=ajStrNewC("FT                                ");
  char intval[7];
  AjBool extra= ajFalse;
  AjPFeatVocFeat outType;
  /*  AjPFeatVocTag outVocTag;*/

  if(!feat->Type)        /* feature type invalid so do not dump out */
    return;

  if(OUTPUT_DICTIONARY)
    outType = ajFeatVocFeatKey(OUTPUT_DICTIONARY,feat->Type->name);
  else
    outType = feat->Type;

  if(!outType){
    ajWarn("%S not found in output dictionary",feat->Type->name);
    return;
  }

  if(file == NULL) AJRAISE(Null_IO_Handle) ;
  
  ajStrReplace(&new,5,outType->name,ajStrLen(outType->name)); /* add feature */

  if(feat->Flags & FEATURE_START_BEFORE_SEQ)
    sprintf(intval,"<%d",feat->Start);
  else
    sprintf(intval,"%d",feat->Start);

  ajStrReplaceC(&new,14+(6-strlen(intval)),intval,strlen(intval)); /* add feature */

  if(feat->Flags & FEATURE_END_AFTER_SEQ)
    sprintf(intval,">%d",feat->End);
  else
    sprintf(intval,"%d",feat->End);
  ajStrReplaceC(&new,21+(6-strlen(intval)),intval,strlen(intval)); /* add feature */
  
  if(feat->desc){
    if(ajStrLen(feat->desc) > 41){
      extra= ajTrue;
      ajStrApp(&new,feat->desc);
      ajStrTruncate(&new,70);
    }
    else
      ajStrApp(&new,feat->desc);
  }

  (void) ajFmtPrintF (file, "%S\n",new);

  if(extra){
    ajStrClear(&new);
    ajStrSetC(&new,"FT                                ");
    ajStrJoin(&new,34,feat->desc,36); 
    (void) ajFmtPrintF (file, "%S\n",new);
  }
}

   

/* @funcstatic featDumpGff ********************************************
**
** Write details of single feature to debug file
**
** @param [r] thys [AjPFeature] Feature
** @param [r] file [AjPFile] Output file
** @return [void]
** @@
******************************************************************************/

static void featDumpGff (AjPFeature thys, AjPFile file)
{
   AjPStr            seqname   = NULL ;
   AjPFeatTable      owner     = NULL ;
   AjPFeature        feat      = NULL ;
   AjIList           iter      = NULL ;
   int               nTags     = 0 ;
   int end;
   LPFeatTagValue    item    = NULL ;
   AjPFeatVocFeat outType;
   AjPFeatVocTag outVocTag;

   if(file == NULL) AJRAISE(Null_IO_Handle) ;
   feat    = thys ;
   
   owner = feat->Owner ;
   
   if (owner) {
     (void) ajStrAssS(&seqname, owner->Name);
   }
   else {
     (void) ajStrAssCL(&seqname, "", 12);
   }
   
   end = feat->End;
   if(feat->Flags & FEATURE_END_TWO)
     end = feat->End2;
   
  if(OUTPUT_DICTIONARY)
    outType = ajFeatVocFeatKey(OUTPUT_DICTIONARY,feat->Type->name);
  else
    outType = feat->Type;

  if(outType){
   (void) ajFmtPrintF (file, "%S\t%S\t%S\t%d\t%d\t%S\t%c\t%c\t",
		       seqname,
		       feat->Source?feat->Source->name:NULL,
		       feat->Type?feat->Type->name:NULL,
		       feat->Start,
		       end,
		       feat->Score, 
		       featStrand(thys->Strand),
		       featFrame(thys->Frame) ) ;
  }
  else
    return;
   

   /* For all tag-values... */
   if(feat->Flags & FEATURE_MULTIPLE){
     (void) ajFmtPrintF (file, "Sequence \"%S.%d\"", seqname,feat->Group) ;
     nTags++;
   }
   ajStrDel(&seqname);
   
   if (feat->Tags) {
    iter = ajListIter(feat->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue)ajListIterNext (iter) ;

      if(OUTPUT_DICTIONARY)
	outVocTag = ajFeatVocTagKey(OUTPUT_DICTIONARY,item->Tag->VocTag->name);
      else
	outVocTag = item->Tag->VocTag;

      if(outVocTag){
	if(nTags++) (void) ajFmtPrintF (file,"; ") ;
	if(item->Tag)
	  (void) ajFmtPrintF (file, "%S", outVocTag->name) ;
	else
	  (void) ajFmtPrintF (file, "<NULL>??") ;
	
	if(item->Value) {
	  AjPStr s = item->Value ;
	  if(s) {  
	    if(ajRegExec(FEAT_TV_Regex8,s)){
	      AjPStr temp=NULL;
	      ajRegSubI(FEAT_TV_Regex8,1,&temp);
	      (void) ajFmtPrintF (file," \"%S\"", temp) ;
	      ajRegSubI(FEAT_TV_Regex8,2,&temp);
	      (void) ajFmtPrintF (file," %S", temp) ;
	      ajRegSubI(FEAT_TV_Regex8,3,&temp);
	      (void) ajFmtPrintF (file," %S", temp) ;
	      ajStrDel(&temp);
	    }
	    else if(ajRegExec(FEAT_Regex_Numeric,s)) {
	      (void) ajFmtPrintF (file," %S", s) ;
	    } else {
	      (void) ajFmtPrintF (file," \"%S\"", s) ;
	    }
	  }
	}
      }
      else
	ajWarn("Tag %S ignored as not found in dictionary",item->Tag->VocTag->name);
    }
  }
  if(feat->Flags & FEATURE_START_TWO){
      if(nTags++) (void) ajFmtPrintF (file,"; ") ;
      (void) ajFmtPrintF (file, "%s", "posfrom ") ;
      (void) ajFmtPrintF (file, "\"(%d.%d)\"", feat->Start,feat->Start2) ;
  }    
  if(feat->Flags & FEATURE_END_TWO){
      if(nTags++) (void) ajFmtPrintF (file,"; ") ;
      (void) ajFmtPrintF (file, "%s", "posto ") ;
      (void) ajFmtPrintF (file, "\"(%d.%d)\"", feat->End,feat->End2) ;
  }    
  if(feat->Flags & FEATURE_BETWEEN_SEQ){
      if(nTags++) (void) ajFmtPrintF (file,"; ") ;
      (void) ajFmtPrintF (file, "%s", "possite ") ;
      (void) ajFmtPrintF (file, "\"^\"") ;
  }    
  ajListIterFree(iter) ;

  (void) ajFmtPrintF (file,"\n") ;   
}

/* @func ajFeatTableWriteGff ********************************************
**
** Write feature table in GFF format
**
** @param [r] FeatTab [AjPFeatTable] feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTableWriteGff (AjPFeatTable FeatTab, AjPFile file)
{
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  /* Check arguments */
  ajDebug ("ajFeatTableWriteGff Checking arguments\n");
  if(file == NULL) AJRAISE(Null_IO_Handle) ;
  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;
  
  /* Print header first */
  (void) ajFmtPrintF (file, "##gff-version %3.1f\n",
		      FeatTab->Version) ;

  (void) ajFmtPrintF (file, "##date %D\n", FeatTab->Date) ;

  (void) ajFmtPrintF (file, "##sequence-region %s %d %d\n",
		      ajStrStr(FeatTab->Name),
		      FeatTab->Start,
		      FeatTab->End) ;

  /* For all features... */

  if (FeatTab->Features) {
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      featDumpGff(gf, file) ;
    }
  }
  ajListIterFree(iter) ;
   
  return ajTrue ;
}

/* @funcstatic featRegInitAcedb ********************************************
**
** Initialize regular expressions and data structures for ajFeat  format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitAcedb (void) 
{
  ajDebug("Hello Acedb\n");
  return ajFalse;
}
/* @funcstatic featRegInitEmbl ********************************************
**
** Initialize regular expressions and data structures for ajFeat  format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitEmbl (void) 
{
  FEAT_Regex_Feature = ajRegCompC("^FT(.*)$") ;                                     /* start with FT */
  FEAT_Regex_blankline = ajRegCompC("^[^F][^T]");
  EMBL_Regex_New = ajRegCompC("^FT   ([^ ]+) .*") ;                                       /* start of new feature */
  EMBL_Regex_SourceLine = ajRegCompC("^FT   (.+) (<?[0-9]+)[.][.](>?[0-9]+)");
  EMBL_Regex_SourcePoint = ajRegCompC("^FT   (.+) (<?[0-9]+)");
  EMBL_Regex_Complement = ajRegCompC("^FT   ([^ ]+) *complement");                      /* FT  x      complement" */
  EMBL_Regex_Complement2 = ajRegCompC("complement.([0-9]+)[.][.]([0-9]+)");         /* complement(x..y" */
  EMBL_Regex_Join = ajRegCompC("^FT   (.*) join");                                  /* FT join line" */
  EMBL_Regex_Join2 = ajRegCompC("join");                                            /* FT join line" */

  EMBL_Regex_Location = ajRegCompC("([<]?[0-9]+)[.][.]([>]?[0-9]+)");                        /* FT location line x..y " */
  EMBL_Regex_Location2 = ajRegCompC("[(]([0-9]+)[.]([0-9]+)[)]..[(]([0-9]+)[.]([0-9]+)[)]"); /* FT location line (x.y)..(a.b)  */
  EMBL_Regex_Location3 = ajRegCompC("[(]([0-9]+)[.]([0-9]+)[)][.][.]([>]?[0-9]+)");          /* FT location line (x.y)..a  */
  EMBL_Regex_Location4 = ajRegCompC("([>]?[0-9]+)[.][.][(]([0-9]+)[.]([0-9]+)[)]");          /* FT location line a..(x.y)  */
  EMBL_Regex_Location5 = ajRegCompC("([>]?[0-9]+)\\^([>]?[0-9]+)");                          /* FT location line a^b  */

  /* Tag Value pairs */
  FEAT_TV_Regex1 = ajRegCompC("/(.*)=\"(.*)\"");         /*     /tag="value" */
  FEAT_TV_Regex2 = ajRegCompC("/(.*)=([^\"]*)$");        /*     /tag=value */
  FEAT_TV_Regex3 = ajRegCompC("/(.*)=\"([^\"].*)$");     /* FT  /tag="value */
  FEAT_TV_Regex4 = ajRegCompC("^FT *([^\"]*)$");         /* FT valuecont */
  FEAT_TV_Regex5 = ajRegCompC("^FT *(.*)\"");            /* FT valueend" */
  FEAT_TV_Regex6 = ajRegCompC("^FT                   /(.+)");     /* FT            /tag*/
  FEAT_TV_Regex = ajRegCompC("^FT */");                  /* tag value general */

  return ajTrue;
}

/* @funcstatic featRegInitGenBank ********************************************
**
** Initialize regular expressions and data structures for ajFeat  format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitGenBank (void) 
{
  FEAT_Regex_Feature = ajRegCompC("^FT(.*)$") ;                                     /* start with FT */
  FEAT_Regex_blankline = ajRegCompC("^[^F][^T]");
  EMBL_Regex_New = ajRegCompC("^     ([^ ]+) .*") ;                                       /* start of new feature */
  EMBL_Regex_SourceLine = ajRegCompC("^     (.+) (<?[0-9]+)[.][.](>?[0-9]+)");
  EMBL_Regex_SourcePoint = ajRegCompC("^     (.+) (<?[0-9]+)");
  EMBL_Regex_Complement = ajRegCompC("^     ([^ ]+) *complement");                      /*     x      complement" */
  EMBL_Regex_Complement2 = ajRegCompC("complement.([0-9]+)[.][.]([0-9]+)");         /* complement(x..y" */
  EMBL_Regex_Join = ajRegCompC("^     (.*) join");                                  /*    join line" */
  EMBL_Regex_Join2 = ajRegCompC("join");                                            /* FT join line" */

  EMBL_Regex_Location = ajRegCompC("([<]?[0-9]+)[.][.]([>]?[0-9]+)");                        /*    location line x..y " */
  EMBL_Regex_Location2 = ajRegCompC("[(]([0-9]+)[.]([0-9]+)[)]..[(]([0-9]+)[.]([0-9]+)[)]"); /*    location line (x.y)..(a.b)  */
  EMBL_Regex_Location3 = ajRegCompC("[(]([0-9]+)[.]([0-9]+)[)][.][.]([>]?[0-9]+)");          /*    location line (x.y)..a  */
  EMBL_Regex_Location4 = ajRegCompC("([>]?[0-9]+)[.][.][(]([0-9]+)[.]([0-9]+)[)]");          /*    location line a..(x.y)  */
  EMBL_Regex_Location5 = ajRegCompC("([>]?[0-9]+)\\^([>]?[0-9]+)");                          /*    location line a^b  */

  /* Tag Value pairs */
  FEAT_TV_Regex1 = ajRegCompC("/(.*)=\"(.*)\"");         /*     /tag="value" */
  FEAT_TV_Regex2 = ajRegCompC("/(.*)=([^\"]*)$");        /*     /tag=value */
  FEAT_TV_Regex3 = ajRegCompC("/(.*)=\"([^\"].*)$");     /* FT  /tag="value */
  FEAT_TV_Regex4 = ajRegCompC("^   *([^\"]*)$");         /* FT valuecont */
  FEAT_TV_Regex5 = ajRegCompC("^   *(.*)\"");            /* FT valueend" */
  FEAT_TV_Regex6 = ajRegCompC("^                     /(.+)");     /* FT            /tag*/
  FEAT_TV_Regex = ajRegCompC("^   */");                  /* tag value general */

  return ajTrue;
}

/* @funcstatic featRegInitSwiss ********************************************
**
** Initialize regular expressions and data structures for ajFeat SwissProt format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitSwiss (void) 
{
  ajDebug ("featRegInitSwiss Compiling featDumpSwiss() regexps     ??????? NO IDEA WHY\n");
  FEAT_Regex_Feature = ajRegCompC("^FT(.*)$") ;
  
  return ajTrue;
}

/* @funcstatic featRegInitGff ********************************************
**
** Initialize regular expressions and data structures for ajFeat GFF format
**
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool featRegInitGff (void) 
{
    /* Setup any global static runtime resources here
       for example, regular expression compilation calls */


  /*  if (FeatModInitDone) return;*/

    ajDebug ("featRegInitGff Compiling featDumpGff() regexps\n");
    FEAT_Regex_Numeric = ajRegCompC("^[\\+-]?[0-9]+\\.?[0-9]*$") ;

    ajDebug ("featRegInitGff  Compiling featReadGff() regexps\n");
    FEAT_Regex_blankline = ajRegCompC("^[ ]*$") ;
    FEAT_Regex_version   = ajRegCompC("^##gff-version[ ]+([0-9]+)") ;
    FEAT_Regex_date      = ajRegCompC("^##date[ ]+([0-9][0-9][0-9][0-9])-([0-9][0-9]?)-([0-9][0-9]?)") ;
    FEAT_Regex_region    = ajRegCompC("^##sequence-region[ ]+([0-9a-zA-Z]+)[ ]+([\\+-]?[0-9]+)[ ]+([\\+-]?[0-9]+)") ;
    FEAT_Regex_comment   = ajRegCompC ("^#[ ]*(.*)") ;

    ajDebug ("featRegInitGff  Compiling GFFProcessTagValues() regexps\n");
    FEAT_TV_Regex1 = ajRegCompC ("^\"+([ ]+|;|$)"); 
    FEAT_TV_Regex2 = ajRegCompC ("^\\\\(\")"); /* escaped double quote embedded in a string...*/
    FEAT_TV_Regex3 = ajRegCompC ("^([^\"]*)"); 
    FEAT_TV_Regex4 = ajRegCompC ("^\""); 
    FEAT_TV_Regex5 = ajRegCompC ("^;"); 
    FEAT_TV_Regex6 = ajRegCompC ("^([^; ]+)[ ]*"); 
    FEAT_TV_Regex7 = ajRegCompC ("^[ ]+"); 
    FEAT_TV_Regex8 = ajRegCompC ("^(.+) ([0-9]+) ([0-9]+)$"); 

    /*    FeatModInitDone = ajTrue;*/

    return ajTrue;
}

/* @funcstatic featDelRegAcedb ********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegAcedb(void)
{
return ajFalse;
}

/* @funcstatic featDelRegEmbl ********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegEmbl(void)
{
  ajRegFree(&FEAT_Regex_Feature);
  ajRegFree(&FEAT_Regex_blankline);
  ajRegFree(&EMBL_Regex_New);
  ajRegFree(&EMBL_Regex_SourceLine);
  ajRegFree(&EMBL_Regex_SourcePoint);
  ajRegFree(&EMBL_Regex_Complement);
  ajRegFree(&EMBL_Regex_Complement2);
  ajRegFree(&EMBL_Regex_Join);
  ajRegFree(&EMBL_Regex_Join2);
  ajRegFree(&EMBL_Regex_Location);
  ajRegFree(&EMBL_Regex_Location2);
  ajRegFree(&EMBL_Regex_Location3);
  ajRegFree(&EMBL_Regex_Location4);
  ajRegFree(&EMBL_Regex_Location5);
  ajRegFree(&FEAT_TV_Regex);
  ajRegFree(&FEAT_TV_Regex1);
  ajRegFree(&FEAT_TV_Regex2);
  ajRegFree(&FEAT_TV_Regex3);
  ajRegFree(&FEAT_TV_Regex4);
  ajRegFree(&FEAT_TV_Regex5);
  ajRegFree(&FEAT_TV_Regex6);
  ajRegFree(&FEAT_TV_Regex7);
  ajRegFree(&FEAT_TV_Regex8);

return ajTrue;
}

/* @funcstatic featDelRegGenBank ********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegGenBank(void)
{
return featDelRegEmbl();
}

/* @funcstatic featDelRegSwiss ********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegSwiss(void)
{
  ajRegFree(&FEAT_Regex_Feature) ;
  return ajTrue;
}

/* @funcstatic featDelRegGff ********************************************
**
** Cleanup and exit routines. Free and destroy regular expressions
**
** @return [AjBool] ajFalse if unsuccesful
** @@
******************************************************************************/
static AjBool featDelRegGff(void)
{
    /* Clean-up any global static runtime resources here
       for example, regular expression pattern variables */

    ajDebug ("ajFeatModInit Freeing featDumpGff() regexps\n");
    ajRegFree(&FEAT_Regex_Numeric) ;

    ajDebug ("ajFeatModInit Freeing featReadGff() regexps\n");
    ajRegFree(&FEAT_Regex_blankline) ;
    ajRegFree(&FEAT_Regex_version) ;
    ajRegFree(&FEAT_Regex_date) ;
    ajRegFree(&FEAT_Regex_region) ;
    ajRegFree(&FEAT_Regex_comment) ;

    ajDebug ("ajFeatModInit Freeing GFFProcessTagValues() regexps\n");
    ajRegFree(&FEAT_TV_Regex1) ;
    ajRegFree(&FEAT_TV_Regex2) ;
    ajRegFree(&FEAT_TV_Regex3) ;
    ajRegFree(&FEAT_TV_Regex4) ;
    ajRegFree(&FEAT_TV_Regex5) ;
    ajRegFree(&FEAT_TV_Regex6) ;
    ajRegFree(&FEAT_TV_Regex7) ;
    ajRegFree(&FEAT_TV_Regex8) ;

    return ajTrue;
}

/* @func ajFeatTrace ********************************************
**
** Debug trace for a feature table
**
** @param [r] thys [AjPFeatTable] Feature table
** @return [void]
** @@
******************************************************************************/

void ajFeatTrace (AjPFeatTable thys) {

  AjIList iter;
  AjPFeature ft;
  AjIList tagiter;
  LPFeatTagValue tag;
  AjPStr valarray;
  int i = 0;
  int j = 0;
  /*  int k = 0;*/

  if(!thys){
    ajDebug ("ajFeatTrace feature empty\n");
    return;
  }

  ajDebug ("ajFeatTrace\n");

  ajDebug ("  Class: %x\n", thys->Class);
  ajDebug ("  Name: '%S'\n", thys->Name);
  ajDebug ("  Version: %.3f\n", thys->Version);
  ajDebug ("  Date: %D\n", thys->Date);
  ajDebug ("  Dictionary: %x\n", thys->Dictionary);
  /*  if(thys->Dictionary)
      ajFeatDickTracy(thys->Dictionary);*/
  ajDebug ("  DefFormat: %d '%s'\n",
	   thys->DefFormat, featInFormat[thys->DefFormat].Name);
  ajDebug ("  DefSource: %x\n", thys->DefSource);
  ajDebug ("  DefType: %x\n", thys->DefType);
  ajDebug ("  Extent...\n");
  ajDebug ("    Start: %d\n", thys->Start);
  ajDebug ("    End: %d\n", thys->End);
  ajDebug ("  Features...\n");

  iter = ajListIter(thys->Features) ;

  for (i=1;ajListIterMore(iter);i++) {
    ft = ajListIterNext(iter);
    ajDebug ("%5d: %x\n", i, ft);
    ajDebug ("    Class: %x\n", ft->Class);
    ajDebug ("    Owner: '%x'", ft->Owner); /* AjPFeatTable */
    if (ft->Owner)
      ajDebug ("  '%S'", ft->Owner->Name);
    ajDebug ("\n");
    if(ft->Source)
      ajDebug ("    Source: '%S'\n", ft->Source->name);
    if( ft->Type)
      ajDebug ("    Type: '%S'\n", ft->Type->name);
    ajDebug ("      Start:%8d\n", ft->Start);
    ajDebug ("      End:  %8d\n", ft->End);
    ajDebug ("    Score: '%S'\n", ft->Score);
    ajDebug ("    Comment: '%S'\n", ft->Comment);
    ajDebug ("    Strand: %c '%c'\n", ft->Strand, featStrand(ft->Strand));
    ajDebug ("    Frame: %d '%c'\n", ft->Frame, featFrame(ft->Frame));
    ajDebug ("    Tags: %x\n", ft->Tags);
    if (ft->Tags) {
      tagiter = ajListIter(ft->Tags) ;
      for (j=1;ajListIterMore(tagiter);j++) {
	tag = ajListIterNext(tagiter);
	if(tag->Tag)
	  ajDebug ("      Tag %d.%d '%S'", i, j, (AjPStr) tag->Tag->VocTag->name); /* void* */
	else
	  ajDebug ("      Tag %d.%d <NULL>??", i, j); /* void* */

	valarray = (AjPStr) tag->Value;
	ajDebug (" '%S'\n", valarray);
      }
      ajListIterFree(tagiter) ;
    }
  }
  ajListIterFree(iter) ;

    
  return;
}

/* @funcstatic ajFeatTableWriteAcedb ********************************************
**
** Write a feature table in ACEDB format.
**
** @param [r] features [AjPFeatTable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool ajFeatTableWriteAcedb (AjPFeatTable features, AjPFile file) {
  ajDebug("ajFeatTableWriteAcedb NOT IMPLEMENTED YET\n");
return ajFalse;
}

/* @func ajFeatTableWriteDdbj ********************************************
**
** Write a feature table in DDBJ format.
**
** @param [r] features [AjPFeatTable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTableWriteDdbj (AjPFeatTable features, AjPFile file) {
  return ajFeatTableWriteGenbank(features,file); /* as it's identical. */
}

/* @func ajFeatTableWriteEmbl ********************************************
**
** Write a feature table in EMBL format.
**
** @param [r] FeatTab [AjPFeatTable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTableWriteEmbl (AjPFeatTable FeatTab, AjPFile file) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;
  AjPFeature gfprev   = NULL ;
  AjBool join=ajFalse,whole = ajFalse;           /* has "join(" been added */
  AjPStr location = NULL;        /* location list as a string */
  char crap[80];
  AjPStr temp=NULL,pos=NULL;
  int oldgroup = -1;
  /* Check arguments */

  ajDebug ("ajFeatTableWriteEmbl Checking arguments\n");
  if(file == NULL) AJRAISE(Null_IO_Handle) ;
  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;


  location = ajStrNewL(80);
  temp = ajStrNewL(80);
  pos = ajStrNewL(80);

  /* For all features... */
  
  ajListSort(FeatTab->Features,*featcompbygroup); 
  /* embl can only dump in the order read in*/
  /* other option is to dump seperately ie. CDS join(1..4,7..8)*/
  /* would become CDS 1..4 and       */
  /*  CDS 7..8      on seperate lines */ 
  if (FeatTab->Features) {
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      ajDebug("g= %d, e=%d\n",gf->Group,gf->Exon);
      if((oldgroup != gf->Group) && gfprev){ /* all data for the previous feature is now ready */
	
	if(join)
	  ajStrAppC(&location,")");      /* close bracket for join */
	if(whole){
	  ajStrInsertC(&location,0,"complement(");
	  ajStrAppC(&location,")");
	  whole = ajFalse;
	}
	  
	featDumpEmbl(gfprev,location, file) ; /* REMEMBER gfprev has tag data */
	
	/* reset the values from previous */
	ajStrClear(&location);

	if(gf->Flags & FEATURE_COMPLEMENT_MAIN)
	  whole =ajTrue;

	join = ajFalse;  
      }
      else if(gf->Flags & FEATURE_COMPLEMENT_MAIN)
	whole =ajTrue;
      oldgroup = gf->Group;
      /* process the new gf */
      if(ajStrLen(location)){ /* one location already there */
	if(!join){
	  ajStrInsertC(&location,0,"join(");
	  join = ajTrue;
	}
	ajStrAppC(&location,",");
      }
      
      ajStrClear(&temp);
      ajStrClear(&pos);
      if(gf->Flags & FEATURE_START_BEFORE_SEQ){
	sprintf(crap,"<%d",gf->Start);
	ajStrAssC(&pos,crap);
      }
      else if(gf->Flags & FEATURE_START_TWO){
	sprintf(crap,"(%d.%d)",gf->Start,gf->Start2);
	ajStrAppC(&pos,crap);
      }
      else{
	sprintf(crap,"%d",gf->Start);
	ajStrAppC(&pos,crap);
      }
      
      if(!(gf->Flags & FEATURE_POINT)){
	if(gf->Flags & FEATURE_BETWEEN_SEQ){
	  sprintf(crap,"^%d",gf->End);
	  ajStrAppC(&pos,crap);
	}
	else if(gf->Flags & FEATURE_END_AFTER_SEQ){
	  sprintf(crap,"..>%d",gf->End);
	  ajStrAppC(&pos,crap);
	}
	else if(gf->Flags & FEATURE_END_TWO){
	  sprintf(crap,"..(%d.%d)",gf->End,gf->End2);
	  ajStrAppC(&pos,crap);
	}
	else{
	  sprintf(crap,"..%d",gf->End);
	  ajStrAppC(&pos,crap);
	}
      }
      
      if(gf->Strand == AjStrandCrick && !whole){
	ajStrAssC(&temp,"complement(");
	ajStrApp(&temp,pos);
	ajStrAppC(&temp,")");
      }
      else {
	ajStrAss(&temp,pos);
      }
      ajStrClear(&pos);
      ajStrApp(&location,temp);
      gfprev=gf;
    }
    /* Don't forget the last one !!! */
    if(join)
      ajStrAppC(&location,")");      /* close bracket for join */
    if(whole){
      ajStrInsertC(&location,0,"complement(");
      ajStrAppC(&location,")");
      whole = ajFalse;
    }
    
    featDumpEmbl(gfprev,location, file) ; /* REMEMBER gfprev has tag data */
    ajStrDel(&location);
    ajStrDel(&pos);
    ajStrDel(&temp);
  }
  ajListIterFree(iter) ;


return ajTrue;
}

/* @func ajFeatTableWriteGenbank ********************************************
**
** Write a feature table in GenBank format.
**
** @param [r] FeatTab [AjPFeatTable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTableWriteGenbank (AjPFeatTable FeatTab, AjPFile file) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;
  AjPFeature gfprev   = NULL ;
  AjBool join=ajFalse;           /* has "join(" been added */
  AjPStr location = NULL;        /* location list as a string */
  char crap[80];
  AjPStr temp=NULL,pos=NULL;

  /* Check arguments */

  ajDebug ("ajFeatTableWriteEmbl Checking arguments\n");
  if(file == NULL) AJRAISE(Null_IO_Handle) ;
  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;


  location = ajStrNewL(80);
  temp = ajStrNewL(80);
  pos = ajStrNewL(80);

  /* For all features... */

  if (FeatTab->Features) {
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      if((gf->Flags & FEATURE_MOTHER) && gfprev){ /* all data for the previous feature is now ready */

	if(join)
	  ajStrAppC(&location,")");      /* close bracket for join */

	featDumpGenbank(gfprev,location, file) ; /* REMEMBER gfprev has tag data */

	/* reset the values from previous */
	ajStrClear(&location);
	join = ajFalse;  
      }
      /* process the new gf */
      if(ajStrLen(location)){ /* one location already there */
	if(!join){
	  ajStrInsertC(&location,0,"join(");
	  join = ajTrue;
	}
	ajStrAppC(&location,",");
      }

      ajStrClear(&temp);
      ajStrClear(&pos);
      if(gf->Flags & FEATURE_START_BEFORE_SEQ){
	sprintf(crap,"<%d",gf->Start);
	ajStrAssC(&pos,crap);
      }
      else if(gf->Flags & FEATURE_START_TWO){
	sprintf(crap,"(%d.%d)",gf->Start,gf->Start2);
	ajStrAppC(&pos,crap);
      }
      else{
	sprintf(crap,"%d",gf->Start);
	ajStrAppC(&pos,crap);
      }
      
      if(!(gf->Flags & FEATURE_POINT)){
	if(gf->Flags & FEATURE_BETWEEN_SEQ){
	  sprintf(crap,"^%d",gf->End);
	  ajStrAppC(&pos,crap);
	}
	else if(gf->Flags & FEATURE_END_AFTER_SEQ){
	  sprintf(crap,"..>%d",gf->End);
	  ajStrAppC(&pos,crap);
	}
	else if(gf->Flags & FEATURE_END_TWO){
	  sprintf(crap,"..(%d.%d)",gf->End,gf->End2);
	  ajStrAppC(&pos,crap);
	}
	else{
	  sprintf(crap,"..%d",gf->End);
	  ajStrAppC(&pos,crap);
	}
      }

      if(gf->Strand == AjStrandCrick){
	ajStrAssC(&temp,"complement(");
	ajStrApp(&temp,pos);
	ajStrAppC(&temp,")");
      }
      else {
	ajStrAss(&temp,pos);
      }
      ajStrApp(&location,temp);
      gfprev=gf;
    }
    /* Don't forget the last one !!! */
    if(join)
      ajStrAppC(&location,")");      /* close bracket for join */
    
    featDumpGenbank(gfprev,location, file) ; /* REMEMBER gfprev has tag data */

  }
  ajListIterFree(iter) ;


return ajTrue;
}

/* @funcstatic ajFeatTableWriteUnknown ********************************************
**
** Write a feature table in SwissProt format.
**
** @param [r] FeatTab [AjPFeatTable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool ajFeatTableWriteUnknown (AjPFeatTable FeatTab, AjPFile file) {
  if(file == NULL) AJRAISE(Null_IO_Handle) ;
  (void) ajFmtPrintF (file, "Unknown feature format hence no outout.Except this line!!\n");

  return ajFalse;
}

/* @func ajFeatTableWriteSwiss ********************************************
**
** Write a feature table in SwissProt format.
**
** @param [r] FeatTab [AjPFeatTable] Feature table
** @param [r] file [AjPFile] Output file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFeatTableWriteSwiss (AjPFeatTable FeatTab, AjPFile file) {
  AjIList    iter = NULL ;
  AjPFeature gf   = NULL ;

  /* Check arguments */
  ajDebug ("ajFeatTableWriteSwiss Checking arguments\n");
  if(file == NULL) AJRAISE(Null_IO_Handle) ;
  ajFeatObjVerify(FeatTab, AjCFeatTable ) ;

  /* For all features... */

  if (FeatTab->Features) {
    iter = ajListIter(FeatTab->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
      featDumpSwiss(gf, file) ;
    }
  }
  ajListIterFree(iter) ;
   
  return ajTrue ;
}

/* @func ajFeatGetName ********************************************
**
** Returns the name of a feature table object. This is a copy of the
** pointer to the name, and is still owned by the feature table** and
** is not to be destroyed.
**
** @param [r] thys [AjPFeatTable] Feature table
** @return [AjPStr] Feature table name.
** @@
******************************************************************************/

AjPStr ajFeatGetName (AjPFeatTable thys) {
  return thys->Name;
}


/* @funcstatic featFrame ********************************************
**
** Converts a frame number in the range 0 to 3 to a GFF frame character
**
** @param [r] frame [int] Feature frame number
** @return [char] character for this frame in GFF
******************************************************************************/

static char featFrame (int frame) {

  static char framestr[] = ".012";

  if (frame < 0) return '.';
  if (frame > 3) return '.';
		  
  return framestr[frame];
}

/* @funcstatic featStrand ********************************************
**
** Converts a strand number to a GFF strand character. NULL characters
** are converted to '.' All other values are simply cast to character.
**
** @param [r] strand [int] Strand
** @return [char] GFF character for this strand.
** @@
******************************************************************************/

static char featStrand (int strand) {

  if (!strand) return '.';

  return ajSysItoC(strand);
}

/* @func ajFeatIsProt ********************************************
**
** Returns ajTrue if a feature table is protein
**
** @param [r] thys [AjPFeatTable] Feature table
** @return [AjBool] ajTrue for a protein feature table
** @@
******************************************************************************/

AjBool ajFeatIsProt (AjPFeatTable thys) {
  ajDebug("ajFeatIsProt NOT IMPLEMENTED YET\n");
  return ajFalse;
}

/* @func ajFeatLen ********************************************
**
** Returns the sequence length of a feature table
**
** @param [r] thys [AjPFeatTable] Feature table
** @return [int] Length in bases or residues
** @@
******************************************************************************/

int ajFeatLen (AjPFeatTable thys) {
  if (!thys) return 0;
  return (thys->End);
}

/* @func ajFeatSize ********************************************
**
** Returns the number of features in a feature table
**
** @param [r] thys [AjPFeatTable] Feature table
** @return [int] Number of features
** @@
******************************************************************************/

int ajFeatSize (AjPFeatTable thys) {
  if (!thys) return 0;
  return ajListLength (thys->Features);
}

/* @funcstatic typeMatch **********************************************
**
** Match routine for feature with list.
**
** @param [r] feat [AjPFeatVocFeat] Dictionary feature value
** @param [r] list [AjPList]  list to compare too.
**
** @return [int] 1 if matches else 0
** @@
******************************************************************************/

static int typeMatch(AjPFeatVocFeat feat,AjPList list){
  AjIList    iter = NULL ;
  AjPFeatVocFeat gf;

  if(!list)
    return 0 ;
  iter = ajListIter(list) ;
  while(ajListIterMore(iter)) {
    gf = ajListIterNext (iter) ;
    if(gf == feat){
      ajListIterFree(iter) ;
      return 1;
    }
  }
  ajListIterFree(iter) ;
  return 0;  
}

/* @funcstatic typeMatchTag **********************************************
**
** Match routine for Tag with list.
**
** @param [r] feat [AjPFeatVocTagForFeat] Dictionary feature value
** @param [r] list [AjPList]  list to compare too.
**
** @return [int] 1 if matches else 0
** @@
******************************************************************************/
static int typeMatchTag(AjPFeatVocTagForFeat feat,AjPList list){
  AjIList    iter = NULL ;
  AjPFeatVocTag gf;

  if(!list)
    return 0 ;

  iter = ajListIter(list) ;
  while(ajListIterMore(iter)) {
    gf = ajListIterNext (iter) ;
    if(gf == feat->VocTag){
      ajListIterFree(iter) ;
      return 1;
    }
  }
  ajListIterFree(iter) ;
  return 0;  
}

/* @func CheckDictForFeature *******************************************
**
** Search for Feature in the Dictionary.
**
** @param [r] table [AjPFeatTable] table to search.
** @param [r] feature [AjPStr] string to search for in table.
**
** @return [AjPFeatVocFeat] Feature Dictionary member or NULL if not there.
** @@
************************************************************************/
AjPFeatVocFeat CheckDictForFeature(AjPFeatTable table, AjPStr feature){
  return ajFeatVocFeatKey(ajFeatTabDictionary(table), feature)  ;
}

/* @func CheckDictForTag  **********************************************
**
** Search for Taa in the Dictionary.
**
** @param [r] table [AjPFeatTable] table to search.
** @param [r] tag [AjPStr] string to search for in table.
**
** @return [AjPFeatVocTag] Tag Dictionary ptr or NULL if not there.
** @@
************************************************************************/
AjPFeatVocTag CheckDictForTag(AjPFeatTable table, AjPStr tag){
  return ajFeatVocTagKey(ajFeatTabDictionary(table), tag)  ;
}


/* @funcstatic ajFeatIgnoreTag2 *********************************************
**
** Remove all tags that of the type in the list from the feature.
**
** @param [rw] Feat [AjPFeature] Feature table to modify.
** @param [r] list [AjPList] List of Tags to remove.
**
** @return [void]
** @@
**********************************************************************/  
static void ajFeatIgnoreTag2(AjPFeature Feat, AjPList list){
  AjIList    iter = NULL ;
  LPFeatTagValue item = NULL;  

  if(Feat && Feat->Tags){
    iter = ajListIter(Feat->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue) ajListIterNext (iter) ;
      if(typeMatchTag(item->Tag,list)){
	ajStrDel(&item->Value);
	AJFREE(item);
	ajListRemove(iter);
      }
    }
    ajListIterFree(iter) ;
  }
} 

/* @funcstatic ajFeatOnlyAllowTag2 *********************************************
**
** Remove all tags NOT of the type in the list from the feature.
**
** @param [rw] Feat [AjPFeature] Feature table to modify.
** @param [r] list [AjPList] List of Tags NOT to remove.
**
** @return [void]
** @@
**********************************************************************/  
static void ajFeatOnlyAllowTag2(AjPFeature Feat, AjPList list){
  AjIList    iter = NULL ;
  LPFeatTagValue item = NULL;  

  if(Feat && Feat->Tags){
    iter = ajListIter(Feat->Tags) ;
    while(ajListIterMore(iter)) {
      item = (LPFeatTagValue) ajListIterNext (iter) ;
      if(!typeMatchTag(item->Tag,list)){
	ajStrDel(&item->Value);
	AJFREE(item);
	ajListRemove(iter);
      }
    }
    ajListIterFree(iter) ;
  }
} 




/* @funcstatic featClear ******************************************************
**
** The implementation strategy for ajfeat object destruction is to 
** recursively clear the object of all base class data fields which
** involved fresh allocation of memory. A the moment, only featClear
** carries a burden of this activity... Other *Clear's could, for
** efficiency sake, become #define's (to avoid cost of function calls...)
**
** @param [r] thys [AjPFeature] Feature
** @return [void]
** @@
******************************************************************************/

static void featClear( AjPFeature thys )
{ 
  AjIList        iter = NULL ;
  LPFeatTagValue item = NULL ;
  
  if (!thys) return ;
  
  /* 'Owner' FeatTable will be deleted by its creator...*/
  /* Source & Type are just simple tag keys -- 
     eventually deleted by the 'Owner' FeatTable */
  
  ajStrDel(&(thys->Score)) ;
  
  /* I need to delete the associated Tag data structures too!!!*/
  if (thys->Tags)
  {
    iter = ajListIter(thys->Tags) ;
    while(ajListIterMore(iter))
    {
      item = (LPFeatTagValue)ajListIterNext (iter) ;
      /* assuming a simple block memory free for now...*/
      ajStrDel(&item->Value) ;
      AJFREE(item);
      ajListRemove(iter) ;
    }
      
  }
  ajListIterFree(iter) ;
  ajListFree(&(thys->Tags));
  ajListDel(&(thys->Tags)) ;
  
}

/* @funcstatic dummyDict ***************************************
**
** Dummy read dict for when no dictionary is used.
**
** @param [r] format [int] for storage.
** @return [AjPFeatLexicon] NULL always!!
** @@
**
******************************************************************************/
static AjPFeatLexicon dummyDict (int format) {

  return NULL;
}

/* @funcstatic ajFeatFindTagInFeatlist ***************************************
**
** Find and return Tag if it is in the feature list.
**
** @param [r] thys [AjPFeature] Feature
** @param [r] key  [AjPFeatVocTag] key to tag
**
** @return [AjPFeatVocTagForFeat] NULL if not found.
** @@
******************************************************************************/

static  AjPFeatVocTagForFeat ajFeatFindTagInFeatlist(AjPFeature thys, AjPFeatVocTag key){
  AjBool ret= ajFalse;
  AjIList iter        = NULL ;
  AjPFeatVocTagForFeat old= NULL;
  
  if(thys->Type){
    iter = ajListIter(thys->Type->Tags);
    while(ajListIterMore(iter)) {
      old =ajListIterNext (iter);
      if(key == old->VocTag){
	ret = ajTrue;
	break;
      }
    }
  }
  ajListIterFree(iter) ;
  if(ret)
    return old;
  else
    return NULL;
}

/* @funcstatic ajFeatAddTagToFeatList  *******************************************
**
** Add Tag to the Feature's list of tags.
**
** @param [rw] feature [AjPFeatVocFeat] Feature to add valid tag to.
** @param [r]  tag     [void*]          key to tag to be added.
** @param [r]  flag    [int]            Mandatory or not.
**
** @return [void]
** @@
******************************************************************************/

static void ajFeatAddTagToFeatList(AjPFeatVocFeat feature,void *tag, int flag){
  AjPFeatVocTagForFeat new = NULL,key2;  
  AjPFeatVocTag key = (AjPFeatVocTag) tag;
  AjIList iter        = NULL ;

  if(!feature->Tags){
    feature->Tags= ajListNew();
    /*feature->flags=0;*/
  }

  iter = ajListIter(feature->Tags) ;
  while(ajListIterMore(iter)) {
    key2 = (AjPFeatVocTagForFeat) ajListIterNext (iter);
    if(key2->VocTag == key)
    {
	ajListIterFree(iter);
	return;
    }
    
  }
  ajListIterFree(iter) ;

  AJNEW0(new);
  new->VocTag=tag;
  if(flag)
    new->mandatory=ajTrue;
  else
    new->mandatory = ajFalse;

  ajListPushApp(feature->Tags,(void *)new); /* Not deleted at end !!! */
  if(flag){
    if(!(feature->flags & TAG_MANDATORY))
      feature->flags += TAG_MANDATORY;
  }
  return;
}

/* @funcstatic FeatVocTrace *******************************************
**
** Trace out the valid tag's for a feature.
**
** @param [r] key   [const void *] not used.
** @param [r] value [void **]      (AjPFeatVocFeat *) to be dumped out.
** @param [r] cl    [void *]       not used.
**
** @return [void]
** @@
*********************************************************************/ 
static void FeatVocTrace (const void *key, void **value, void *cl){
  AjPFeatVocFeat *a= (AjPFeatVocFeat *) value; 
  AjPFeatVocTagForFeat tag=NULL;
  AjIList iter;
  int i;

  ajDebug("Feature *%S* ",(*a)->name);
  if((*a)->flags & TAG_GFF)
    ajDebug(" GFF SPECIFIC \n");
  else
    ajDebug("%d \n",(*a)->flags);
  iter = ajListIter((*a)->Tags) ;
  for (i=1;ajListIterMore(iter);i++) {
    tag = ajListIterNext(iter);
    if(tag->mandatory)
      ajDebug("tag=\t%S\t MANDATORY\n",tag->VocTag->name);
    else
      ajDebug("tag=\t%S\n",tag->VocTag->name);
  }
  ajListIterFree(iter) ;

}
/* @funcstatic FeatVocDel *******************************************
**
** Delete tag's for a feature.
**
** @param [r] key   [const void *] not used.
** @param [r] value [void **]      (AjPFeatVocFeat *) to be dumped out.
** @param [r] cl    [void *]       not used.
**
** @return [void]
** @@
*********************************************************************/ 
static void FeatVocDel (const void *key, void **value, void *cl){
  AjPFeatVocFeat *a= (AjPFeatVocFeat *) value; 
  AjPFeatVocTagForFeat tag=NULL;
  AjIList iter;
  int i;

  ajStrDel(&(*a)->name);

  iter = ajListIter((*a)->Tags) ;
  for (i=1;ajListIterMore(iter);i++) {
    tag = ajListIterNext(iter);
    AJFREE(tag);
  }
  ajListIterFree(iter) ;

  ajListFree(&(*a)->Tags);
  ajListDel(&(*a)->Tags);  /* no need to free contents */
                           /*as these are merely pointers */

  AJFREE((*a));
}

/* @funcstatic TagVocTrace *******************************************
**
** Trace out all the valid tag's.
**
** @param [r] key   [const void *] not used.
** @param [r] value [void **]      (AjPFeatVocTag *) to be dumped out.
** @param [r] cl    [void *]       not used.
**
** @return [void]
** @@
*********************************************************************/ 
static void TagVocTrace (const void *key, void **value, void *cl){
  AjPFeatVocTag *a= (AjPFeatVocTag *) value; 

  if((*a)->flags && TAG_TEXT)
    ajDebug("%S\tTEXT\n",(*a)->name);
  else if((*a)->flags && TAG_QTEXT)
    ajDebug("%S\tQTEXT\n",(*a)->name);
  else if((*a)->flags && TAG_SBI)
    ajDebug("%S\tSBI\n",(*a)->name);
  else if((*a)->flags && TAG_LIMITED)
    ajDebug("%S\tLIMITED\n",(*a)->name);
  else if((*a)->flags && TAG_VOID)
    ajDebug("%S\tVOID\n",(*a)->name);
  else
    ajDebug("%S\tUNKNOWN??????\n",(*a)->name);

  return;
}

/* @func ajFeatUnused ********************************************
**
** Dummy just to use any untested functions
**
** @return [void]
** @@
******************************************************************************/

void ajFeatUnused(void)
{
    AjPFeatTable seqmap=NULL;
    AjPStr line=NULL;
    
    (void) GFFromLine (seqmap,line );
    (void) EMBL_Dictionary (0);
}

/* @funcstatic ajFeatSetFlag ********************************************
**
** Set flag.
**
** @param [rw] flags [int*] int to add flag to.
** @param [r]  val   [int] flag to be set.
**
** @return [void]
** @@
******************************************************************************/
static void ajFeatSetFlag(int *flags, int val){
  if(*flags & val)
    return;
  else
    *flags +=val;
  return;
}

/* @func ajFeatTabInClear ****************************************************
**
** Clears a Tabin input object back to "as new" condition, except
** for the USA list which must be preserved.
**
** @param [P] thys [AjPFeatTabIn] Sequence input
** @return [void]
** @@
******************************************************************************/

void ajFeatTabInClear (AjPFeatTabIn thys)
{

  ajDebug ("ajFeatTabInClear called\n");

  (void) ajStrClear(&thys->Ufo);
  (void) ajStrClear(&thys->Seqname);
  (void) ajStrClear(&thys->Formatstr);
  (void) ajStrClear(&thys->Filename);
  (void) ajStrClear(&thys->Entryname);
  if (thys->Handle)
    ajFileBuffDel(&thys->Handle);
  if (thys->Handle)
    ajFatal("ajFeatTabInClear did not delete Handle");

  return;
}

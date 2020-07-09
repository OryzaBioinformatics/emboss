/********************************************************************
** @source AJAX ACD (ajax command definition) functions
**
** These functions control all aspects of AJAX command definition
** syntax, command line handling and prompting of the user.
**
** The only major functions visible to callers are the initialisation
** function ajAcdInit and a series of retrieval functions ajGetAcdObject
** for each defined object type (integer, sequence, and so on).
**
** Future extensions are planned, including the ability to write out
** the ACD internal structures in a number of other interface formats.
**
** @author Copyright (C) 1998 Peter Rice
** @version 1.0 
** @modified Jun 25 pmr First version
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
********************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "ajax.h"


#define DEFCODON  "Ehum.cut"
#define DEFDLIST  "."
#define DEFBLOSUM "EBLOSUM62"
#define DEFDNA    "EDNAFULL"
#define DEFCPDB   "1azu"
#define DEFSCOP   "d3sdha"

#define ACD_SEQ_BEGIN 0
#define	ACD_SEQ_END 1
#define	ACD_SEQ_LENGTH 2
#define	ACD_SEQ_PROTEIN 3
#define	ACD_SEQ_NUCLEIC 4
#define	ACD_SEQ_NAME 5
#define	ACD_SEQ_WEIGHT 6
#define	ACD_SEQ_COUNT 7

/*static AjBool acdDebug = 0;*/
/*static AjBool acdDebugSet = 0;*/
/*static AjPStr acdProgram = NULL;*/
static AjBool acdDoLog = 0;
static AjBool acdDoPretty = 0;
static AjBool acdDoHelp = 0;
static AjBool acdVerbose = 0;
static AjBool acdTable = 0;
static AjBool acdAuto = 0;
static AjBool acdFilter = 0;
static AjBool acdOptions = 0;
static AjBool acdStdout = 0;
static AjBool acdCodeSet = 0;
static AjPTable acdCodeTable = 0;
static ajint acdInFile = 0;
static ajint acdOutFile = 0;
static ajint acdPromptTry = 2;
static AjPStr acdInFName = NULL;
/*static AjPStr acdOutFName = NULL;*/
static AjPStr acdExpTmpstr = NULL;

static AjPStr acdLogFName = NULL;
static AjPFile acdLogFile = NULL;

static AjPStr acdPrettyFName = NULL;
static AjPFile acdPrettyFile = NULL;

/* Levels as defined in the ACD structure */

enum AcdELevel { ACD_APPL,		/* application definition */
		 ACD_PARAM,		/* parameter */
		 ACD_QUAL,		/* qualifier */
		 ACD_VAR,		/* variable */
		 ACD_IF,                /* if test */
		 ACD_SEC,	        /* start new section */
		 ACD_ENDSEC		/* end section */
};

/* Levels as text, only for use in logging report */

static char* acdLevel[] = { "APPL", "PARAM", "QUAL", "VAR", "IF", "SEC", "ENDSEC" };

/* Attribute value types */

enum AcdEValtype {VT_APPL, VT_STR, VT_WORD,
		      VT_BOOL, VT_INT, VT_FLOAT,
		      VT_NULL};

/* Attribute value types as text for use in logging report */

static char* acdValNames[] = {"application", "string", "word",
			      "bool", "integer", "float",
			      NULL};

/* attribute structure */

typedef struct AcdSAttr
{	
  char *Name;
  enum AcdEValtype Type;
} AcdOAttr, *AcdPAttr;


/* qualifier structure */

typedef struct AcdSQual
{
  char *Name;
  char *Default;
  char *Type;
  char *Help;
} AcdOQual, *AcdPQual;

typedef struct AcdSTableItem
{
  AjPStr Qual;
  AjPStr Help;
  AjPStr Valid;
  AjPStr Expect;
} AcdOTableItem, *AcdPTableItem;

/* @data AcdPAcd **************************************************************
**
** AJAX Command Definition component
**
** ACDs are built as an ACD file is parsed, and are processed as a
** list in sequential order.
**
** ACDs contain type information. Some functions will only work on certain
** types of ACDs.
**
** There is currently no destructor. No need for one at present.
**
** @new acdNewQual Creates a qualifier ACD
** @new acdNewQualQual Creates an associated qualifier ACD
** @new acdNewAppl Creates an ACD application
** @new acdNewVar Creates an ACD variable
** @new acdNewAcd General constructor
** @@
******************************************************************************/

typedef struct AcdSAcd
{
  struct AcdSAcd* Next;
  AjPStr Name;            
  AjPStr Token;
  ajint PNum;
  enum AcdELevel Level;		/* qual/param var appl */
  ajint Type;			/* acdType or acdKeywords index */
  ajint NAttr;
  AjPStr* AttrStr;		/* type attribs: qual/param only */
  ajint SAttr;			/* specially set for some types */
  AcdPAttr SetAttr;
  AjPStr* SetStr;
  AjPStr* DefStr;		/* default attrib values */
  AjBool Defined;		/* set when defined by user */
  AjBool Assoc;
  struct AcdSAcd* AssocQuals;
  AjPStr OrigStr;
  AjPStr ValStr;
  void* Value;
} AcdOAcd, *AcdPAcd;


/* general type structure */

typedef struct AcdSType
{
  char* Name;			/* type name */
  AcdPAttr Attr;		/* type-specific attributes */
  void (*Set)(AcdPAcd thys);	/* function to set value and prompt user */
  AcdPQual Quals;		/* type-specific associated qualifiers */
  char* Valid;			/* Valid data help message */
} AcdOType, *AcdPType;



static AjBool* acdParamSet;

static AcdPAcd acdApplCurr = NULL;
static AcdPAcd acdQualCurr = NULL;
static AcdPAcd acdVarCurr = NULL;
static AcdPAcd acdSecCurr = NULL;
static AcdPAcd acdEndsecCurr = NULL;
static AcdPAcd acdMasterQual = NULL;

/*
AcdOAcd acdList = {NULL, NULL, NULL, 0, ACD_APPL, 0, 0, NULL, 0, NULL,
		   NULL, NULL, 0, 0, NULL, NULL, NULL};
*/
static AcdPAcd acdList = NULL;
static AcdPAcd acdListLast = NULL;
static AcdPAcd acdListCurr = NULL;

static ajint acderr=0;
static ajint acdNParam=0;
static AjPStr acdAppl,  acdApplType, acdApplToken;
static AjPStr acdQual,  acdQualType, acdQualToken;
static AjPStr acdVar;
static AjPStr acdSec;
static AjPStr acdEndsec;


/* keywords (other than qualifier types) */

typedef enum
{
  BAD_STAGE, APPL_STAGE, QUAL_STAGE, VAR_STAGE, SEC_STAGE, ENDSEC_STAGE
} AcdEStage ;

static AcdEStage acdCurrentStage;

static AcdEStage acdStage (AjPStr token);
static void acdParse (AjPStr text);
static void acdArgsScan (ajint argc, char *argv[]);
static void acdHelp (void);
static void acdHelpAssoc (AcdPAcd thys, AjPStr *str, char *name);
static void acdHelpAppend (AcdPAcd thys, AjPStr* str, char flag);
static void acdHelpShow (AjPStr str, char* title);
static void acdHelpAssocTable (AcdPAcd thys, AjPList tablist, char flag);
static void acdHelpTable (AcdPAcd thys, AjPList tablist, char flag);
static void acdHelpTableShow (AjPList tablist, char* title);
static void acdHelpText (AcdPAcd thys, AjPStr* msg);
static void acdHelpValid (AcdPAcd thys, AjPStr *str);
static void acdArgsParse (ajint argc, char *argv[]);
static AjBool acdIsParam (char* arg, AjPStr* param, ajint* iparam,
                          AcdPAcd* acd);
static ajint acdIsQual (char* arg, char* arg2, ajint *iparam, AjPStr *pqual,
                      AjPStr *pvalue, ajint* number, AcdPAcd* acd);
static AjBool acdValIsBool (char* arg);
static void acdQualParse (AjPStr* pqual, AjPStr* pqmaster, ajint* number);
static void acdTokenToLower (char *token, ajint* number);
static void acdNoComment (AjPStr* text);
static AjPStr acdParseValue (AjPStrTok* tokenhandle, char* delim);
static AjPStr acdParseValueRB (AjPStrTok* tokenhandle, char* delim);
static AcdPAcd acdNewQual (AjPStr name, AjPStr token, AjPStr* type,
			   ajint pnum);
static AcdPAcd acdNewQualQual (AjPStr name, AjPStr* type);
static AcdPAcd acdNewAppl (AjPStr name);
static AcdPAcd acdNewVar (AjPStr name);
static AcdPAcd acdNewAcd (AjPStr name, AjPStr token, ajint itype);
static AcdPAcd acdNewAcdKey (AjPStr name, AjPStr token, ajint ikey);
static AcdPAcd acdNewSec (AjPStr name);
static AcdPAcd acdNewEndsec (AjPStr name);
static void acdTestUnknown (AjPStr name, AjPStr token, ajint pnum);
static void acdTestAssoc (AjPStr name);
static AjBool acdTestQualC (char *name);
static AcdPAcd acdFindAcd (AjPStr name, AjPStr token, ajint pnum);
static AcdPAcd acdFindAssoc (AcdPAcd thys, AjPStr name);
static ajint acdAttrCount (ajint itype);
static ajint acdAttrKeyCount (ajint ikey);
static ajint acdAttrListCount (AcdPAttr attr);
static AjBool acdIsLeftB (AjPStr *pstr);
static AjBool acdIsRightB (AjPStr *pstr);
static AjBool acdGetAttr (AjPStr* result, AjPStr name, AjPStr attrib);
static ajint acdFindType (AjPStr type);
static ajint acdFindTypeC (char* type);
/*static ajint acdFindKey (AjPStr key);*/
static ajint acdFindKeyC (char* key);
static void* acdGetValue (char *token, char* type);
static void* acdGetValueNum (char *token, char* type, ajint pnum);
static AjPStr acdGetValStr (char *token);
static AjBool acdGetValueAssoc (AcdPAcd thys, char *token, AjPStr *result);
static void acdBadRetry (AcdPAcd thys);
static void acdBadVal (AcdPAcd thys, AjBool required, char *fmt, ...);
static void acdListReport (char *title);
static void acdListAttr (AcdPAttr attr, AjPStr* valstr, ajint nattr);
static ajint acdFindAttr (AcdPAttr attr, AjPStr attrib);
static ajint acdFindAttrC (AcdPAttr attr, char* attrib);
static void acdSetAll (void);
static void acdProcess (void);
static AjBool acdCodeGet (AjPStr code, AjPStr *msg);
static AjBool acdCodeDef (AcdPAcd thys, AjPStr *msg);
static AjBool acdHelpCodeDef (AcdPAcd thys, AjPStr *msg);
static void acdCodeInit (void);
static AjBool acdSetQualAppl (AcdPAcd thys, AjBool val);
static AjBool acdAttrResolve (AcdPAcd thys, char *attr, AjPStr *result);
static AjBool acdAttrToBool (AcdPAcd thys,
			     char *attr, AjBool defval, AjBool *result);
static AjBool acdAttrToFloat (AcdPAcd thys,
			      char *attr, float defval, float *result);
static AjBool acdAttrToInt (AcdPAcd thys,
 			    char *attr, ajint defval, ajint *result);
static AjBool acdAttrToStr (AcdPAcd thys,
			    char *attr, char* defval, AjPStr *result);
static AjBool acdQualToBool (AcdPAcd thys, char *qual, 
			     AjBool defval, AjBool *result, AjPStr* valstr);
static AjBool acdQualToFloat (AcdPAcd thys, char *qual,
			      float defval, ajint precision,
			      float *result, AjPStr* valstr);
static AjBool acdQualToInt (AcdPAcd thys, char *qual,
			      ajint defval, ajint *result, AjPStr* valstr);
static AjBool acdQualToSeqbegin (AcdPAcd thys, char *qual,
				 ajint defval, ajint *result, AjPStr* valstr);
static AjBool acdQualToSeqend   (AcdPAcd thys, char *qual,
				 ajint defval, ajint *result, AjPStr* valstr);
static AjBool acdVarResolve (AjPStr* str);
static AjBool acdHelpVarResolve (AjPStr* str, AjPStr src);
static AjBool acdFunResolve (AjPStr* result, AjPStr str);
static AjBool acdVarSplit (AjPStr var, AjPStr* name, AjPStr* attrname);
static AjPStr acdAttrValue (AcdPAcd thys, char *attrib);
static AjBool acdAttrValueStr (AcdPAcd thys, char *attrib, char* def,
			       AjPStr *str);
static AcdPAcd acdFindItem (AjPStr item, ajint number);
static AcdPAcd acdFindQual (AjPStr qual, AjPStr master,
			    ajint PNum, ajint *iparam);
static AcdPAcd acdFindQualMaster (AjPStr qual, AjPStr master,
				  ajint PNum);
static AcdPAcd acdFindQualAssoc (AcdPAcd pa, AjPStr qual, ajint pnum);
static AcdPAcd acdFindParam (ajint PNum);
static ajint acdNextParam (ajint pnum);
static AjBool acdIsParamValue (AjPStr pval);
static AjBool acdIsRequired (AcdPAcd thys);

static AjBool acdReplyInit (AcdPAcd thys, char *defval, AjPStr* reply);
static AjBool acdUserGet (AcdPAcd thys, AjPStr* reply);
static AjBool acdUserGetPrompt (char* prompt, AjPStr* reply);

static AjBool acdDef (AcdPAcd thys, AjPStr value);
static AjBool acdSet (AcdPAcd thys, AjPStr* attrib, AjPStr value);
static AjBool acdSetDef (AcdPAcd thys, AjPStr value);
static AjBool acdSetDefC (AcdPAcd thys, char* value);
static AjBool acdSetQualDefBool (AcdPAcd thys, char* name, AjBool value);
static AjBool acdSetQualDefInt (AcdPAcd thys, char* name, ajint value);
static AjBool acdSetKey (AcdPAcd thys, AjPStr* attrib, AjPStr value);
static AjBool acdSetVarDef (AcdPAcd thys, AjPStr value);
static void acdPromptCodon (AcdPAcd thys);
static void acdPromptCpdb (AcdPAcd thys);
static void acdPromptDirlist (AcdPAcd thys);
static void acdPromptFeat (AcdPAcd thys);
static void acdPromptFeatout (AcdPAcd thys);
static void acdPromptGraph (AcdPAcd thys);
static void acdPromptReport (AcdPAcd thys);
static void acdPromptScop (AcdPAcd thys);
static void acdPromptSeq (AcdPAcd thys);
static void acdPromptSeqout (AcdPAcd thys);
static void acdPromptOutfile (AcdPAcd thys);
static void acdPromptInfile (AcdPAcd thys);
static void acdListPrompt (AcdPAcd thys);
static void acdSelectPrompt (AcdPAcd thys);
static AjPStr* acdListValue (AcdPAcd thys, ajint min, ajint max, AjPStr reply);
static AjPStr* acdSelectValue (AcdPAcd thys, ajint min, ajint max, AjPStr reply);
static void acdAmbigApp (AjPStr* pambiglist, AjPStr str);
static void acdAmbigAppC (AjPStr* pambiglist, char* txt);
static AjBool acdDataFilename (AjPStr* datafname, AjPStr name, AjPStr ext);
static AjBool acdInFilename (AjPStr* infname);
static AjBool acdOutFilename (AjPStr* outfname, AjPStr name, AjPStr ext);
static AjBool acdInFileSave (AjPStr infname);
static AjPStr acdStrDiff (AjPStr str1, AjPStr str2);
static void acdLog (char *fmt, ...);
static void acdPretty (char *fmt, ...);
static AjBool acdIsQtype (AcdPAcd thys);
static AjBool acdTextFormat (AjPStr* text);
AjBool acdVocabCheck (AjPStr str, char** vocab);

/* expression processing */

static AjBool acdExpPlus (AjPStr* result, AjPStr str);
static AjBool acdExpMinus (AjPStr* result, AjPStr str);
static AjBool acdExpStar (AjPStr* result, AjPStr str);
static AjBool acdExpDiv (AjPStr* result, AjPStr str);
static AjBool acdExpNot (AjPStr* result, AjPStr str);
static AjBool acdExpEqual (AjPStr* result, AjPStr str);
static AjBool acdExpNotEqual (AjPStr* result, AjPStr str);
static AjBool acdExpGreater (AjPStr* result, AjPStr str);
static AjBool acdExpLesser (AjPStr* result, AjPStr str);
static AjBool acdExpAnd (AjPStr* result, AjPStr str);
static AjBool acdExpOr (AjPStr* result, AjPStr str);
static AjBool acdExpCond (AjPStr* result, AjPStr str);
static AjBool acdExpCase (AjPStr* result, AjPStr str);
static AjBool acdExpFilename (AjPStr* result, AjPStr str);
static AjBool acdExpExists (AjPStr* result, AjPStr str);

typedef struct AcdSExpList {
  char* Name;
  AjBool (*Func) (AjPStr *result, AjPStr str);
} AcdOExpList;

/* @funclist explist **********************************************************
**
** Functions for processing expressions in ACD dependencies
**
******************************************************************************/

static AcdOExpList explist[] = {
    {"plus", acdExpPlus},
    {"minus", acdExpMinus},
    {"star", acdExpStar},
    {"div", acdExpDiv},
    {"not", acdExpNot},
    {"equal", acdExpEqual},
    {"notequal", acdExpNotEqual},
    {"greater", acdExpGreater},
    {"lesser", acdExpLesser},
    {"or", acdExpOr},
    {"and", acdExpAnd},
    {"cond", acdExpCond},
    {"case", acdExpCase},
    {"filename", acdExpFilename},
    {"exists", acdExpExists},
    {NULL, NULL}
};

/* Dummy model routine for new data types - but these must not be static
   and wil be defined in ajacd.h instead */

/*static void*  ajAcdGetXxxx (char *token);*/

static void acdHelpValidCodon (AcdPAcd thys, AjPStr* str);
static void acdHelpValidDirlist (AcdPAcd thys, AjPStr* str);
static void acdHelpValidData  (AcdPAcd thys, AjPStr* str);
static void acdHelpValidFeatout (AcdPAcd thys, AjPStr* str);
static void acdHelpValidFloat (AcdPAcd thys, AjPStr* str);
static void acdHelpValidGraph (AcdPAcd thys, AjPStr* str);
static void acdHelpValidIn (AcdPAcd thys, AjPStr* str);
static void acdHelpValidInt (AcdPAcd thys, AjPStr* str);
static void acdHelpValidList (AcdPAcd thys, AjPStr* str);
static void acdHelpValidMatrix (AcdPAcd thys, AjPStr* str);
static void acdHelpValidOut (AcdPAcd thys, AjPStr* str);
static void acdHelpValidCpdb (AcdPAcd thys, AjPStr* str);
static void acdHelpValidScop (AcdPAcd thys, AjPStr* str);
static void acdHelpValidRange (AcdPAcd thys, AjPStr* str);
static void acdHelpValidRegexp (AcdPAcd thys, AjPStr* str);
static void acdHelpValidSelect (AcdPAcd thys, AjPStr* str);
static void acdHelpValidSeq (AcdPAcd thys, AjPStr* str);
static void acdHelpValidSeqout (AcdPAcd thys, AjPStr* str);
static void acdHelpValidString (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectCodon (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectDirlist (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectData (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectFeatout (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectFloat (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectGraph (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectIn (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectInt (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectMatrix (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectOut (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectCpdb (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectScop (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectRange (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectRegexp (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectSeq (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectSeqout (AcdPAcd thys, AjPStr* str);
static void acdHelpExpectString (AcdPAcd thys, AjPStr* str);

/* Type-specific routines to prompt user and set the value.  each new
** type requires one of these routines */

static void acdSetXxxx (AcdPAcd thys);
static void acdSetAppl (AcdPAcd thys);
static void acdSetEndsec (AcdPAcd thys);
static void acdSetSec (AcdPAcd thys);
static void acdSetVar (AcdPAcd thys);
static void acdSetArray (AcdPAcd thys);
static void acdSetBool (AcdPAcd thys);
static void acdSetCodon (AcdPAcd thys);
static void acdSetDirlist (AcdPAcd thys);
static void acdSetDatafile (AcdPAcd thys);
static void acdSetDirectory (AcdPAcd thys);
static void acdSetFeat (AcdPAcd thys);
static void acdSetFeatout (AcdPAcd thys);
static void acdSetFloat (AcdPAcd thys);
static void acdSetGraph (AcdPAcd thys);
static void acdSetGraphxy (AcdPAcd thys);
static void acdSetInt (AcdPAcd thys);
static void acdSetInfile (AcdPAcd thys);
static void acdSetList (AcdPAcd thys);
static void acdSetMatrix (AcdPAcd thys);
static void acdSetMatrixf (AcdPAcd thys);
static void acdSetOutfile (AcdPAcd thys);
static void acdSetCpdb (AcdPAcd thys);
static void acdSetScop (AcdPAcd thys);
static void acdSetRange (AcdPAcd thys);
static void acdSetRegexp (AcdPAcd thys);
static void acdSetReport (AcdPAcd thys);
/*static void acdSetRegions (AcdPAcd thys);*/
static void acdSetSelect (AcdPAcd thys);
static void acdSetSeq (AcdPAcd thys);
static void acdSetSeqset (AcdPAcd thys);
static void acdSetSeqall (AcdPAcd thys);
static void acdSetSeqout (AcdPAcd thys);
static void acdSetSeqoutset (AcdPAcd thys);
static void acdSetSeqoutall (AcdPAcd thys);
static void acdSetString (AcdPAcd thys);

/*
** Known item types
**
** Each has 1 functions, used below in the definition of "types"
** The first is global, and passes the results to the application.
** The other is static and are used in command line and user management
**
** The argument definiion is the same for each.
** Items have a type and undefined pointers to the actual data.
** Each function knows the structure they must use, and the validations
** needed.
**
** Type   ajAcdGetType : globally available. returns the value
**
** ajBool acdSetType   : sets the value using the default value (if any)
**
*/

/* Default attributes available for all types */

static ajint nDefAttr = 15;

enum AcdEDef { DEF_DEFAULT,
	       DEF_PROMPT,
	       DEF_INFO,
	       DEF_CODE,
	       DEF_HELP,
	       DEF_REQUIRED,
	       DEF_OPTIONAL,
	       DEF_MISSING,
	       DEF_PARAMETER,
	       DEF_VALID,
	       DEF_EXPECTED,
	       DEF_COMMENT,
	       DEF_CORBA,
	       DEF_STYLE,
	       DEF_NEEDED
};

AcdOAttr acdAttrDef[] = { {"default", VT_STR}, /* default value */
			  {"prompt", VT_STR}, /* prompt */
			  {"information", VT_STR}, /* info for menus etc. */
			  {"code", VT_STR}, /* code for prompt */
			  {"help", VT_STR}, /* text for help+docs */
			  {"required", VT_BOOL}, /* prompt if missing? */
			  {"optional", VT_BOOL}, /* required with -options */
			  {"missing", VT_BOOL}, /* allow with no value? */
			  {"parameter", VT_BOOL}, /* accept as a parameter */
			  {"valid", VT_STR}, /* help: allowed values  */
			  {"expected", VT_STR}, /* help: expected value  */
			  {"comment", VT_STR}, /* comment for AppLab  */
			  {"corba", VT_STR}, /* corba spec for AppLab  */
			  {"style", VT_STR}, /* style for AppLab  */
			  {"needed", VT_BOOL}, /* include in GUI form */
			  {NULL, VT_NULL}
};

/* Type-specific attributes
** each must end with "NULL, VT_NULL" to define the end of the list
*/

AcdOAttr acdAttrXxxx[] = {
			 {NULL, VT_NULL} };

AcdOAttr acdAttrAppl[] = { {"documentation", VT_STR},
			   {"groups", VT_STR},
			   {"comment", VT_STR},
			   {NULL, VT_NULL} };

AcdOAttr acdAttrArray[] = { {"minimum", VT_FLOAT},
			    {"maximum", VT_FLOAT},
			    {"increment", VT_FLOAT},
			    {"precision", VT_INT},
			    {"warnrange", VT_BOOL},
			    {"size", VT_INT},
			    {"sum", VT_FLOAT},
			    {"tolerance", VT_FLOAT},
			  {NULL, VT_NULL} };

AcdOAttr acdAttrBool[] = {
			  {NULL, VT_NULL} };

AcdOAttr acdAttrCodon[] = { {"name", VT_STR},
			    {NULL, VT_NULL} };


AcdOAttr acdAttrDirectory[] = { {"fullpath", VT_BOOL},
				{"nullok", VT_BOOL},
				{NULL, VT_NULL} };

AcdOAttr acdAttrDirlist[] = { {"fullpath", VT_BOOL},
			      {"nullok", VT_BOOL},
			      {NULL, VT_NULL} };

AcdOAttr acdAttrEndsec[] = {
			   {NULL, VT_NULL} };

AcdOAttr acdAttrFeat[] = { {"name", VT_STR},
			   {"extension", VT_STR},
			   {NULL, VT_NULL} };

AcdOAttr acdAttrFeatout[] = { {"name", VT_STR},
			      {"extension", VT_STR},
			      {NULL, VT_NULL} };

AcdOAttr acdAttrFloat[] = { {"minimum", VT_FLOAT},
			    {"maximum", VT_FLOAT},
			    {"increment", VT_FLOAT},
			    {"precision", VT_INT},
			    {"warnrange", VT_BOOL},
			    {NULL, VT_NULL} };

AcdOAttr acdAttrGraph[] = { {"type", VT_STR},
			    {"gtitle",VT_STR},
			    {"gsubtitle",VT_STR},
			    {"gxtitle",VT_STR}, 
			    {"gytitle",VT_STR}, 
			    {"goutfile",VT_STR},
			    {NULL, VT_NULL} };

AcdOAttr acdAttrGraphxy[] = { {"type", VT_STR},
			      {"gtitle",VT_STR},
			      {"gsubtitle",VT_STR},
			      {"gxtitle",VT_STR},
			      {"gytitle",VT_STR}, 
			      {"goutfile",VT_STR},
			      {"multiple", VT_INT},
			      {NULL, VT_NULL} };

AcdOAttr acdAttrInt[] = { {"minimum", VT_INT},
			  {"maximum", VT_INT},
			  {"increment", VT_INT},
			  {"warnrange", VT_BOOL},
			  {NULL, VT_NULL} };

AcdOAttr acdAttrDatafile[] = { {"name", VT_STR},
			       {"extension", VT_STR},
			       {"type", VT_STR},
			       {"nullok", VT_BOOL},
			       {NULL, VT_NULL} };

AcdOAttr acdAttrInfile[] = { {"name", VT_STR},
			     {"extension", VT_STR},
			     {"type", VT_STR},
			     {"nullok", VT_BOOL},
			     {NULL, VT_NULL} };

AcdOAttr acdAttrList[] = { {"minimum", VT_INT},
			   {"maximum", VT_INT},
			   {"button", VT_BOOL},
			   {"casesensitive", VT_BOOL},
			   {"header", VT_STR},
			   {"delimiter", VT_STR},
			   {"codedelimiter", VT_STR},
			   {"values", VT_STR},
			   {NULL, VT_NULL} };

AcdOAttr acdAttrMatrix[] = { {"pname", VT_STR},
			   {"nname", VT_STR},
			   {"protein", VT_BOOL},
			   {NULL, VT_NULL} };

AcdOAttr acdAttrMatrixf[] = { {"pname", VT_STR},
			    {"nname", VT_STR},
			    {"protein", VT_BOOL},
			    {NULL, VT_NULL} };

AcdOAttr acdAttrOutfile[] = { {"name", VT_STR},
			      {"extension", VT_STR},
			      {"type", VT_STR},
			      {"nullok", VT_BOOL},
			      {NULL, VT_NULL} };

AcdOAttr acdAttrCpdb[] = { {"name", VT_STR},
			    {NULL, VT_NULL} };

AcdOAttr acdAttrScop[] = { {"name", VT_STR},
			    {NULL, VT_NULL} };


AcdOAttr acdAttrRange[] = { 
			    {NULL, VT_NULL} };


AcdOAttr acdAttrRegexp[] = { {"minlength", VT_INT},
			     {"maxlength", VT_INT},
			     {"upper", VT_BOOL},
			     {"lower", VT_BOOL},
			     {NULL, VT_NULL} };

AcdOAttr acdAttrReport[] = { {"name", VT_STR},
			      {"extension", VT_STR},
			      {NULL, VT_NULL} };

AcdOAttr acdAttrSec[] = { {"info", VT_STR},
			   {"type", VT_STR}, /* list in acdSetSec */
			   {"comment", VT_STR},
			   {"border", VT_INT},
			   {"side", VT_STR},    /* list in acdSetSec */
			   {"folder", VT_STR},
			   {NULL, VT_NULL} };

AcdOAttr acdAttrSelect[] = { {"minimum", VT_INT},
			     {"maximum", VT_INT},
			     {"button", VT_BOOL},
			     {"casesensitive", VT_BOOL},
			     {"header", VT_STR},
			     {"delimiter", VT_STR},
			     {"values", VT_STR},
			     {NULL, VT_NULL} };

AcdOAttr acdAttrSeqout[] = { {"name", VT_STR},
			     {"extension", VT_STR},
			     {"features", VT_BOOL},
			     {NULL, VT_NULL} };

AcdOAttr acdAttrSeqoutset[] = { {"name", VT_STR},
				{"extension", VT_STR},
				{"features", VT_BOOL},
				{NULL, VT_NULL} };

AcdOAttr acdAttrSeqoutall[] = { {"name", VT_STR},
				{"extension", VT_STR},
				{"features", VT_BOOL},
				{NULL, VT_NULL} };

AcdOAttr acdAttrSeq[] = { {"type", VT_STR},
			  {"features", VT_BOOL},
			  {"entry", VT_BOOL},
			  {NULL, VT_NULL} };

AcdOAttr acdAttrSeqset[] = { {"type", VT_STR},
			     {"features", VT_BOOL},
			     {NULL, VT_NULL} };

AcdOAttr acdAttrSeqall[] = { {"type", VT_STR},
			     {"features", VT_BOOL},
			     {"entry", VT_BOOL},
			     {NULL, VT_NULL} };

AcdOAttr acdAttrString[] = { {"minlength", VT_INT},
			     {"maxlength", VT_INT},
			     {"pattern", VT_STR},
       			     {"upper", VT_BOOL},
			     {"lower", VT_BOOL},
		     {NULL, VT_NULL} };

AcdOAttr acdAttrVar[] = {
                          {NULL, VT_NULL} };

typedef struct AcdSKey
{
  char *Name;
  AcdEStage Stage;
  AcdPAttr Attr;		/* type-specific attributes */
  void (*Set)(AcdPAcd thys);	/* function to set value and prompt user */
} AcdOKey, *AcdPKey;

/* @funclist acdKeywords ******************************************************
**
** Processing predefined ACD keywords (application, variable, section,
** endsection)
**
******************************************************************************/

AcdOKey acdKeywords[] =
{
  {"application", APPL_STAGE,   acdAttrAppl,   acdSetAppl},
  {"variable",    VAR_STAGE,    acdAttrVar,    acdSetVar},
  {"section",     SEC_STAGE,    acdAttrSec,    acdSetSec},
  {"endsection",  ENDSEC_STAGE, acdAttrEndsec, acdSetEndsec},
  {NULL, BAD_STAGE} };

/* Type-specific associated qualifiers which can be used positionally
** or numbered if tied to a parameter */

/* "qualifier"  "default" "type" */

AcdOQual acdQualAppl[] =	/* careful: index numbers used in*/
				/* acdSetQualAppl */
{
  {"auto",       "N",      "bool", "turn off prompts"},
  {"stdout",     "N",      "bool", "write standard output"},
  {"filter",     "N",      "bool", "read standard input, write standard output"},
                                  /* after auto and stdout so it can replace */
  {"options",    "N",      "bool", "prompt for required and optional values"},
  {"debug",      "N",      "bool", "write debug output to program.dbg"},
  {"acdlog",     "N",      "bool", "write ACD processing log to program.acdlog"},
  {"acdpretty",  "N",      "bool", "rewrite ACD file as program.acdpretty"},
  {"acdtable",   "N",      "bool", "write HTML table of options"},
  {"help",       "N",      "bool", "report command line options. More information on associated and general qualifiers can be found with -help -verbose"},
  {"verbose",    "N",      "bool", "report some/full command line options"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualFeat[] =
{
  {"fformat",    "",       "string",  "features format"},
  {"fopenfile",  "",       "string",  "features file name"},
  {"fask",       "",       "bool",    "prompt for begin/end/reverse"},
  {"fbegin",     "",       "integer", "first base used"},
  {"fend",       "",       "integer", "last base used, def=max length"},
  {"freverse",   "",       "bool",    "reverse (if DNA)"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualFeatout[] =
{
  {"offormat",    "",       "string", "output feature format"},
  {"ofopenfile",  "",       "string", "features file name"},
  {"ofextension", "",       "string", "file name extension"},
  {"ofname",      "",       "string", "base file name"},
  {"ofsingle",    "",       "bool",   "separate file for each entry"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualReport[] =
{
  {"rformat",    "",       "string", "report format"},
  {"ropenfile",  "",       "string", "report file name"},
  {"rextension", "",       "string", "file name extension"},
  {"rname",      "",       "string", "base file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualSeq[] =
{
  {"sbegin",     "", "integer", "first base used"},
  {"send",       "", "integer", "last base used, def=seq length"},
  {"sreverse",   "", "bool",    "reverse (if DNA)"},
  /*  {"sprompt",    "", "bool",    "prompt for begin/end/reverse"}, */
  {"sask",       "", "bool",    "ask for begin/end/reverse"},
  {"snucleotide", "", "bool",    "sequence is nucleotide"},
  {"sprotein",   "", "bool",    "sequence is protein"},
  {"slower",     "", "bool",    "make lower case"},
  {"supper",     "", "bool",    "make upper case"},
  {"sformat",    "", "string",  "input sequence format"},
  {"sopenfile",  "", "string",  "input filename"},
  {"sdbname",    "", "string",  "database name"},
  {"sid",        "", "string",  "entryname"},
  {"ufo",        "", "string",  "UFO features"},
  {"fformat",    "", "string",  "features format"},
  {"fopenfile",  "", "string",  "features file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualSeqset[] =
{
  {"sbegin",     "", "integer", "first base used"},
  {"send",       "", "integer", "last base used, def=seq length"},
  {"sreverse",   "", "bool",    "reverse (if DNA)"},
  /*  {"sprompt",    "", "bool",    "prompt for begin/end/reverse"}, */
  {"sask",       "", "bool",    "ask for begin/end/reverse"},
  {"snucleotide", "", "bool",    "sequence is nucleotide"},
  {"sprotein",   "", "bool",    "sequence is protein"},
  {"slower",     "", "bool",    "make lower case"},
  {"supper",     "", "bool",    "make upper case"},
  {"sformat",    "", "string",  "input sequence format"},
  {"sopenfile",  "", "string",  "input filename"},
  {"sdbname",    "", "string",  "database name"},
  {"sid",        "", "string",  "entryname"},
  {"ufo",        "", "string",  "UFO features"},
  {"fformat",    "", "string",  "features format"},
  {"fopenfile",  "", "string",  "features file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualSeqall[] =
{
  {"sbegin",     "", "integer", "first base used"},
  {"send",       "", "integer", "last base used, def=seq length"},
  {"sreverse",   "", "bool",    "reverse (if DNA)"},
  /*  {"sprompt",    "", "bool",    "prompt for begin/end/reverse"}, */
  {"sask",       "", "bool",    "ask for begin/end/reverse"},
  {"snucleotide", "", "bool",    "sequence is nucleotide"},
  {"sprotein",   "", "bool",    "sequence is protein"},
  {"slower",     "", "bool",    "make lower case"},
  {"supper",     "", "bool",    "make upper case"},
  {"sformat",    "", "string",  "input sequence format"},
  {"sopenfile",  "", "string",  "input filename"},
  {"sdbname",    "", "string",  "database name"},
  {"sid",        "", "string",  "entryname"},
  {"ufo",        "", "string",  "UFO features"},
  {"fformat",    "", "string",  "features format"},
  {"fopenfile",  "", "string",  "features file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualSeqout[] =
{
  {"osformat",   "",       "string",  "output seq format"},
  {"osextension", "",      "string",  "file name extension"},
  {"osname",     "",       "string",  "base file name"},
  {"osdbname",   "",       "string",  "database name to add"},
  {"ossingle",   "",       "bool",    "separate file for each entry"},
  {"oufo",       "",       "string",  "UFO features"},
  {"offormat",   "",       "string",  "features format"},
  {"ofname",     "",       "string",  "features file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualSeqoutset[] =
{
  {"osformat",   "",       "string",  "output seq format"},
  {"osextension", "",      "string",  "file name extension"},
  {"osname",     "",       "string",  "base file name"},
  {"osdbname",   "",       "string",  "database name to add"},
  {"ossingle",   "",       "bool",    "separate file for each entry"},
  {"oufo",       "",       "string",  "UFO features"},
  {"offormat",   "",       "string",  "features format"},
  {"ofname",     "",       "string",  "features file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualSeqoutall[] =
{
  {"osformat",   "",       "string",  "output seq format"},
  {"osextension", "",      "string",  "file name extension"},
  {"osname",     "",       "string",  "base file name"},
  {"osdbname",   "",       "string",  "database name to add"},
  {"ossingle",   "",       "bool",    "separate file for each entry"},
  {"oufo",       "",       "string",  "UFO features"},
  {"offormat",   "",       "string",  "features format"},
  {"ofname",     "",       "string",  "features file name"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualGraph[] =
{
  {"gprompt",   "", "bool",    "graph prompting"},
  {"gtitle",    "", "string",  "graph title"},
  {"gsubtitle", "", "string",  "graph subtitle"},
  {"gxtitle",   "", "string",  "graph x axis title"},
  {"gytitle",   "", "string",  "graph y axis title"},
  {"grtitle",   "", "string",  "graph right axis title"},
  {"gpages",    "", "integer", "number of pages"},
  {"goutfile",  "", "string",  "output file for non interactive displays"},
  {NULL, NULL, NULL, NULL} };

AcdOQual acdQualGraphxy[] =
{
  {"gprompt",   "", "bool",    "graph prompting"},
  {"gtitle",    "", "string",  "graph title"},
  {"gsubtitle", "", "string",  "graph subtitle"},
  {"gxtitle",   "", "string",  "graph x axis title"},
  {"gytitle",   "", "string",  "graph y axis title"},
  {"grtitle",   "", "string",  "graph right axis title"},
  {"gpages",    "", "integer", "number of pages"},
  {"gsets",     "", "integer", "number of sets"},
  {"goutfile",  "", "string",  "output file for non interactive displays"},
  {NULL, NULL, NULL, NULL} };

/* Type definitions - must be after attributes and functions are defined
** Add new types here as needed
** Create attribute list acdAttrType
*/

/*"Type"         Attributes        Function
  Qualifiers         "Help Text" */

/* @funclist acdType **********************************************************
**
** Processing for ACD types
**
** Includes the acdSet functions for each ACD type
**
******************************************************************************/

AcdOType acdType[] =
{
  {"array",        acdAttrArray,    acdSetArray,
   NULL,             "List of numbers" },
  {"bool",        acdAttrBool,      acdSetBool,
   NULL,             "Yes/No" },
  {"codon",	  acdAttrCodon,     acdSetCodon,
   NULL,             "Codon usage file in EMBOSS data path" },
  {"datafile",    acdAttrDatafile,  acdSetDatafile,
   NULL,             "Data file" },
  {"directory",   acdAttrDirectory, acdSetDirectory,
   NULL,             "Directory" },
  {"dirlist",	  acdAttrDirlist,     acdSetDirlist,
   NULL,             "Directory with files" },
  {"features",    acdAttrFeat,      acdSetFeat,
   acdQualFeat,      "Readable feature table" },
  {"featout",     acdAttrFeatout,   acdSetFeatout,
   acdQualFeatout,   "Writeable feature table" },
  {"float",       acdAttrFloat,     acdSetFloat,
   NULL,             "Floating point number" },
  {"graph",       acdAttrGraph,     acdSetGraph,
   acdQualGraph,     "Graph device" },
  {"xygraph",     acdAttrGraphxy,   acdSetGraphxy,
   acdQualGraphxy,   "Graph device" },
  {"infile",      acdAttrInfile,    acdSetInfile,
   NULL,             "Input file" },
  {"integer",     acdAttrInt,       acdSetInt,
   NULL,             "Integer" },
  {"list",        acdAttrList,      acdSetList,
   NULL,             "Selection from list of values" },
  {"matrix",      acdAttrMatrix,    acdSetMatrix,
   NULL,             "Comparison matrix file in EMBOSS data path" },
  {"matrixf",     acdAttrMatrixf,   acdSetMatrixf,
   NULL,             "Comparison matrix file in EMBOSS data path" },
  {"outfile",     acdAttrOutfile,   acdSetOutfile,
   NULL,             "Output file" },
  {"cpdb",	  acdAttrCpdb,     acdSetCpdb,
   NULL,             "Cleaned PDB file in EMBOSS data path" },
  {"scop",	  acdAttrScop,     acdSetScop,
   NULL,             "Scop entry in EMBOSS data path" },
  {"range",	  acdAttrRange,     acdSetRange,
   NULL,             "Sequence range" },
  {"regexp",	  acdAttrRegexp,     acdSetRegexp,
   NULL,             "Regular expression pattern" },
  {"report",      acdAttrReport,     acdSetReport,
   acdQualReport,    "Report file" },
  {"select",      acdAttrSelect,    acdSetSelect,
   NULL,             "Selection from list of values" },
  {"sequence",    acdAttrSeq,       acdSetSeq,
   acdQualSeq,       "Readable sequence" },
  {"seqset",      acdAttrSeqset,    acdSetSeqset,
   acdQualSeqset,    "Readable sequences" },
  {"seqall",      acdAttrSeqall,    acdSetSeqall,
   acdQualSeqall,    "Readable sequence(s)" },
  {"seqout",      acdAttrSeqout,    acdSetSeqout,
   acdQualSeqout,    "Writeable sequence" },
  {"seqoutset",   acdAttrSeqoutset, acdSetSeqoutset,
   acdQualSeqoutset, "Writeable sequences" },
  {"seqoutall",   acdAttrSeqoutall, acdSetSeqoutall,
   acdQualSeqoutall, "Writeable sequence(s)" },
  {"string",      acdAttrString,    acdSetString,
   NULL,             "String value" },
  {NULL, NULL, NULL, NULL, NULL}
};

typedef struct AcdSValid
{
  char* Name;
  void (*Valid) (AcdPAcd thys, AjPStr* str);
  void (*Expect) (AcdPAcd thys, AjPStr* str);
} AcdOValid, *AcdPValid;

/* @funclist acdValid *********************************************************
**
** ACD type help processing, includes functions to describe valid
** values and expected values in -help output and -acdtable output
**
******************************************************************************/

AcdOValid acdValid[] =
{
  {"sequence",  acdHelpValidSeq,     acdHelpExpectSeq},
  {"seqset",    acdHelpValidSeq,     acdHelpExpectSeq},
  {"seqall",    acdHelpValidSeq,     acdHelpExpectSeq},
  {"seqout",    acdHelpValidSeqout,  acdHelpExpectSeqout},
  {"seqoutset", acdHelpValidSeqout,  acdHelpExpectSeqout},
  {"seqoutall", acdHelpValidSeqout,  acdHelpExpectSeqout},
  {"outfile",   acdHelpValidOut,     acdHelpExpectOut},
  {"infile",    acdHelpValidIn,      acdHelpExpectIn},
  {"datafile",  acdHelpValidData,    acdHelpExpectData},
  {"codon",     acdHelpValidCodon,   acdHelpExpectCodon},
  {"dirlist",   acdHelpValidDirlist, acdHelpExpectDirlist},
  {"list",      acdHelpValidList,    NULL},
  {"cpdb",      acdHelpValidCpdb,    acdHelpExpectCpdb},
  {"scop",      acdHelpValidScop,    acdHelpExpectScop},
  {"select",    acdHelpValidSelect,  NULL},
  {"graph",     acdHelpValidGraph,   acdHelpExpectGraph},
  {"xygraph",   acdHelpValidGraph,   acdHelpExpectGraph},
  {"regexp",    acdHelpValidRegexp,  acdHelpExpectRegexp},
  {"string",    acdHelpValidString,  acdHelpExpectString},
  {"integer",   acdHelpValidInt,     acdHelpExpectInt},
  {"float",     acdHelpValidFloat,   acdHelpExpectFloat},
  {"matrix",    acdHelpValidMatrix,  acdHelpExpectMatrix},
  {"matrixf",   acdHelpValidMatrix,  acdHelpExpectMatrix},
  {"range",     acdHelpValidRange,   acdHelpExpectRange},
  {"featout",   acdHelpValidFeatout, acdHelpExpectFeatout},
  {NULL,        NULL}
};

/*** command line retrieval routines ***/

/* @func ajAcdInitP ***********************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @param [r] package [char*] Package name, used to find the ACD file
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/

AjStatus ajAcdInitP (char *pgm, ajint argc, char *argv[], char *package)
{

    static AjPFile acdFile = NULL;
    static AjPStr acdLine = NULL;
    static AjPStr acdFName = NULL;
    static AjPStr acdRoot = NULL;
    static AjPStr acdRootInst = NULL;
    static AjPStr acdPack = NULL;
    static AjPStr acdText = NULL;
    static AjPStr tmpstr = NULL;
    ajint i;
   
    acdProgram = ajStrNewC (pgm);
  
    ajDebug ("testing acdprompts");
    if (ajNamGetValueC ("acdprompts", &tmpstr))
    {
	ajDebug ("acdprompts '%S'", tmpstr);
	if (ajStrToInt(tmpstr, &i))
	    acdPromptTry = i;
	if (acdPromptTry < 1) acdPromptTry = 1;
	ajDebug ("acdPromptTry %d", acdPromptTry);
    }
    (void) ajStrDelReuse(&tmpstr);
  
    /* pre-parse the command line for special options */
  
    (void) ajStrStat ("before acdArgsScan");
    acdArgsScan (argc, argv);
    (void) ajStrStat ("after acdArgsScan");
  
    /* open the command definition file */
   
    (void) ajNamRootPack (&acdPack);
    (void) ajNamRootInstall (&acdRootInst);
    (void) ajFileDirFix (&acdRootInst);

    if (ajNamGetValueC ("acdroot", &acdRoot))
    {					/* _acdroot variable defined */
	(void) ajFileDirFix (&acdRoot);
	ajFmtPrintS (&acdFName, "%S%s.acd", acdRoot, pgm);
	acdFile = ajFileNewIn (acdFName);
    }
    else if (*package)
    {					/* separate package */
/*	ajFmtPrintS (&acdFName, "%Sshare/%S/acd/%s.acd",
		     acdRootInst, acdPack, pgm);*/
	ajFmtPrintS (&acdFName, "%Sshare/EMBOSS/acd/%s.acd",
		     acdRootInst, pgm);
	acdFile = ajFileNewIn (acdFName);
	if (!acdFile)
	{
	    ajDebug ("acdfile '%S' not opened\n", acdFName);
	    ajStrAssC (&acdPack, package); /* package name passed to acdInitP */
	    ajStrToLower (&acdPack);	/* temporary - old sources used capitals */
	    /* but try to pass correctly and remove this */

	    (void) ajNamRoot (&acdRoot);
	    (void) ajFileDirUp (&acdRoot);
	    ajFmtPrintS (&acdFName, "%Sembassy/%S/emboss_acd/%s.acd",
			 acdRoot, acdPack, pgm);
	    acdFile = ajFileNewIn (acdFName);
	}
    }
    else
    {					/* main package */
	ajFmtPrintS (&acdFName, "%Sshare/%S/acd/%s.acd",
		     acdRootInst, acdPack, pgm);
	acdFile = ajFileNewIn (acdFName);
	if (!acdFile)
	{
	    ajDebug ("acdfile '%S' not opened\n", acdFName);
	    (void) ajNamRoot (&acdRoot);
	    (void) ajFileDirFix (&acdRoot);
	    ajFmtPrintS (&acdFName, "%Sacd/%s.acd", acdRoot, pgm);
	    acdFile = ajFileNewIn (acdFName);
	}
    }
  
  
    if (!acdFile)
	ajFatal ("acdfile '%S' not opened\n", acdFName);

    (void) ajStrDelReuse (&acdFName);
  
    /* read the whole file into a string [change to use a list later] */
  
    acdText = ajStrNew();
  
    while (ajFileReadLine (acdFile, &acdLine))
    {
	acdNoComment(&acdLine);
	if (ajStrLen(acdLine))
	{
	    (void) ajStrApp (&acdText, acdLine);
	    (void) ajStrAppC (&acdText, " ");
	}
    }
    ajFileClose (&acdFile);
    (void) ajStrDelReuse (&acdLine);
    (void) ajStrDelReuse (&acdFName);
  
    /* Parse the input to set the initial definitions */
  
    (void) ajStrStat ("before acdParse");
    acdParse (acdText);
    (void) ajStrStat ("after acdParse");
    (void) ajStrDelReuse (&acdText);
  
    /* Fill in incomplete information like parameter numbers */
  
    acdProcess ();
    (void) ajStrStat ("after acdProcess");
  
    AJCNEW0(acdParamSet, acdNParam+1);

    /* report on what we have so far */
  
    acdListReport("Definitions in ACD file");
    (void) ajStrStat ("after acdListReport");
  
    /* parse the command line and update the values */
  
    acdArgsParse (argc, argv);
    (void) ajStrStat ("after acdArgsParse");
  
    /* report on what we have so far */
  
    acdListReport("Results of parsing command line arguments");
    (void) ajStrStat ("after acdListReport2");
  
    /* set the true values and prompt for missing required values */
  
    acdSetAll();
    (void) ajStrStat ("after acdSetAll");
  
    /* report on what we have now */
  
    acdListReport("Final results after setting values and prompting the user");
    (void) ajStrStat ("after acdListReport3");
  
    /* all done */

    /* debugging the defaults files and their contents
      ajNamDebugOrigin();
      ajNamDebugDatabases();
      ajNamDebugEnvironmentals();
    */

    return ajStatusOK;
}

/* @func ajAcdInit ************************************************************
**
** Initialises everything. Reads an ACD (AJAX Command Definition) file
** prompts the user for any missing information, reads all sequences
** and other input into local structures which applications can request.
**
** @param [r] pgm [char*] Application name, used as the name of the ACD file
** @param [r] argc [ajint] Number of arguments provided on the command line,
**        usually passsed as-is by the calling application.
** @param [r] argv [char* []] Actual arguments as an array of text.
** @return [AjStatus] Always returns ajStatusOK or aborts.
** @@
******************************************************************************/

AjStatus ajAcdInit (char *pgm, ajint argc, char *argv[]) {

  return ajAcdInitP (pgm, argc, argv, "");
}

/*===========================================================================*/
/*========================= ACD File Parsing ================================*/
/*===========================================================================*/

/* @funcstatic acdStage **************************************************
**
** Tests next token to set the next parsing stage.
**
** @param [r] token [AjPStr] Current token
** @return [AcdEStage] Stage enumerated code
** @@
******************************************************************************/

static AcdEStage acdStage (AjPStr token)
{

    ajint i;
    ajint ifound=0;
    AcdEStage j=BAD_STAGE;

    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    if (!ajStrLen(token))
	return BAD_STAGE;

    i = 0;
    while (acdKeywords[i].Name)
    {
	if (ajStrMatchC (token, acdKeywords[i].Name))
	    return acdKeywords[i].Stage;
	if (ajStrPrefixCO (acdKeywords[i].Name, token))
	{
	    ifound++;
	    j = acdKeywords[i].Stage;
	    acdAmbigAppC (&ambigList, acdKeywords[i].Name);
	}
	i++;
    }

    i =  0;
    while (acdType[i].Name)
    {
	if (ajStrMatchC (token, acdType[i].Name))
	    return QUAL_STAGE;
	if (ajStrPrefixCO (acdType[i].Name, token))
	{
	    ifound++;
	    j = QUAL_STAGE;
	    acdAmbigAppC (&ambigList, acdType[i].Name);
	}
	i++;
    }
    if (ifound == 1)
	return j;
    if (ifound > 1)
    {
	ajWarn ("ambiguous acd stage %S (%S)", token, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }

    return BAD_STAGE;
}

/* @funcstatic acdParse *******************************************************
**
** Parse the command line definition and build data structures from it.
**
** @param [r] text [AjPStr] ACD file converted to a string delimited by spaces
** @return [void]
** @see acdNewAppl
** @see acdNewQual
** @see acdNewVar
** @@
******************************************************************************/

static void acdParse (AjPStr text)
{
    static AjPStr acdtmp=NULL;
    static AjPStr acdvalue=NULL;
    static AjPStr acdmodvalue=NULL;
    static AjPStr acdtoken = NULL;
    AjPStrTok tokenhandle;
    char white[] = " \t\n\r";
    char whiteplus[] = " \t\n\r:=";
    AjBool done=ajFalse;

    tokenhandle = ajStrTokenInit (text, whiteplus);
  
    (void) ajStrToken (&acdtmp, &tokenhandle, NULL);
    (void) ajStrToLower (&acdtmp);

    acdCurrentStage = acdStage (acdtmp);
    if (acdCurrentStage != APPL_STAGE)
	ajFatal("The application definition must be at the start of the file");


    while (ajStrLen(acdtmp))
    {
	acdCurrentStage = acdStage (acdtmp);

	switch (acdCurrentStage)
	{

	    /* catch-all for failed parsing */

	case BAD_STAGE:
	    ajFatal ("Unrecognized token '%S'\n", acdtmp);
	    break;

	    /* appl: application_name */

	case APPL_STAGE:		
	    (void) ajStrAssS(&acdApplType, acdtmp); /* we have the type */
	    (void) ajStrToken (&acdAppl, &tokenhandle, white);
	    /* then the appl name */
	    (void) ajStrToLower (&acdAppl);
	    /* may be [token:= */
	    (void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);
		
	    (void) ajStrToLower (&acdtmp);
	    acdApplCurr = acdNewAppl (acdAppl);
	    if (acdIsLeftB(&acdtmp))
		(void) ajStrAssS (&acdApplToken, acdAppl);
	    else
	    {	/* some day, look for a new token here */
		(void) ajStrAssS (&acdApplToken, acdAppl);
		ajWarn("'[ doc: doc_def]' not found in .acd file for %S: %S\n",
			acdApplType, acdAppl);
		continue;		/* back to top of loop */
	    }
	    (void) ajStrToLower (&acdApplToken);
	    acdPretty ("%S: %S [\n", acdApplType,
		       acdAppl);

	    /* continue parsing until we reach a true closing ']' character */

	    if (!ajStrLen(acdtmp))	/* that was a lonely '[' we trimmed */
		(void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);

	    (void) ajStrToLower (&acdtmp);
	    done = acdIsRightB(&acdtmp);

	    while (!done)
	    {
		(void) ajStrAssS (&acdtoken, acdtmp);
		(void) ajStrAssS (&acdvalue, acdParseValueRB (&tokenhandle,
							      white));
		done = acdIsRightB(&acdvalue);
		(void) ajStrAssS (&acdmodvalue, acdvalue);
		(void) acdTextFormat(&acdmodvalue);
		(void) acdSetKey (acdApplCurr, &acdtoken, acdmodvalue);
		acdPretty ("  %S: \"%S\"\n", acdtoken, acdvalue);
		if (!done)
		{
		    (void) ajStrToken(&acdtmp, &tokenhandle, whiteplus);
		    (void) ajStrToLower (&acdtmp);
		    done = acdIsRightB(&acdtmp);
		}
	    }
	    acdPretty ("]\n");

	    (void) ajStrAssC (&acdVar, "today"); /* we have the type */
	    (void) ajFmtPrintS (&acdvalue, "%D", ajTimeToday());
	    acdVarCurr = acdNewVar (acdVar);
	    (void) acdSetVarDef (acdVarCurr, acdvalue);
	    break;

	    /* type: qualname [ attr: value ]
	     **
	     ** The token name is optional (defaults to the qualifier name)
	     ** The [] are required so the token can be detected.  Attributes
	     ** are defined for each "type", as are associated
	     ** qualifiers. There is no distinction between them here.  The
	     ** difference is that the qualifier values are defaults which
	     ** can be overridden on the ocmmand line
	     */

	case QUAL_STAGE:		
	    (void) ajStrAssS(&acdQualType, acdtmp); /* we have the type */
	    (void) ajStrToken (&acdQual, &tokenhandle, white); /* then the 
								  qualifier */
	    (void) ajStrToLower (&acdQual);
	    (void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);
	    /* may be [token:= */
	    (void) ajStrToLower (&acdtmp);
	    if (acdIsLeftB(&acdtmp))
		(void) ajStrAssS (&acdQualToken, acdQual);
	    else
	    {
		(void) ajStrAssS (&acdQualToken, acdtmp);
		(void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);
		if (!acdIsLeftB(&acdtmp))
		    ajFatal ("'[' not found for %S: %S\n", acdQualType,
			     acdQual);
	    }
	    (void) ajStrToLower (&acdQualToken);
	    acdQualCurr = acdNewQual (acdQual, acdQualToken, &acdQualType, 0);
	    acdPretty ("\n%S: %S %S [\n", acdQualType,
		       acdQual, acdStrDiff(acdQual, acdQualToken));

	    /* continue parsing until we reach a true closing ']' character */

	    if (!ajStrLen(acdtmp))	/* that was a lonely '[' we trimmed */
		(void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);

	    (void) ajStrToLower (&acdtmp);
	    done = acdIsRightB(&acdtmp);

	    while (!done)
	    {
		(void) ajStrAssS (&acdtoken, acdtmp);
		(void) ajStrAssS (&acdvalue, acdParseValueRB (&tokenhandle,
							      white));
		done = acdIsRightB(&acdvalue);
		(void) ajStrAssS (&acdmodvalue, acdvalue);
		(void) acdTextFormat(&acdmodvalue);
		(void) acdSet (acdQualCurr, &acdtoken, acdmodvalue);
		acdPretty ("  %S: \"%S\"\n", acdtoken, acdvalue);
		if (!done)
		{
		    (void) ajStrToken(&acdtmp, &tokenhandle, whiteplus);
		    (void) ajStrToLower (&acdtmp);
		    done = acdIsRightB(&acdtmp);
		}
	    }
	    acdPretty ("]\n");

	    break;

	case VAR_STAGE:		
	    /* then the variable name  and the value */
	    (void) ajStrToken (&acdVar, &tokenhandle, white);
	    (void) ajStrToLower (&acdVar);
	    acdVarCurr = acdNewVar (acdVar);
	    ajStrAssS(&acdvalue, acdParseValue(&tokenhandle, white));
	    acdPretty ("\nvariable:  %S \"%S\"\n", acdVar, acdvalue);
	    (void) acdSetVarDef (acdVarCurr, acdvalue);
	    break;

	case SEC_STAGE:		
	    /* new section - append to list */
	    (void) ajStrToken (&acdSec, &tokenhandle, white);
	    (void) ajStrToLower (&acdSec);

	    (void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);
	    if (!acdIsLeftB(&acdtmp))
		ajFatal ("'[' not found for section: %S\n", acdSec);

	    acdSecCurr = acdNewSec (acdSec);
	    acdPretty ("\nsection: %S  [\n", acdSec);

	    /* continue parsing until we reach a true closing ']' character */

	    if (!ajStrLen(acdtmp))	/* that was a lonely '[' we trimmed */
	      (void) ajStrToken (&acdtmp, &tokenhandle, whiteplus);

	    (void) ajStrToLower (&acdtmp);
	    done = acdIsRightB(&acdtmp); /* can be just [tok:val] */

	    while (!done)
	    {
		(void) ajStrAssS (&acdtoken, acdtmp);
		(void) ajStrAssS (&acdvalue, acdParseValueRB (&tokenhandle,
							      white));
		done = acdIsRightB(&acdvalue);
		(void) ajStrAssS (&acdmodvalue, acdvalue);
		(void) acdTextFormat(&acdmodvalue);
		(void) acdSetKey (acdSecCurr, &acdtoken, acdmodvalue);
		acdPretty ("  %S: \"%S\"\n", acdtoken, acdvalue);
		if (!done)
		{
		    (void) ajStrToken(&acdtmp, &tokenhandle, whiteplus);
		    (void) ajStrToLower (&acdtmp);
		    done = acdIsRightB(&acdtmp);
		}
	    }
	    acdPretty ("]\n");

	    break;

	case ENDSEC_STAGE:		
	    /* end section, remove from list. Only have the name here */
	    (void) ajStrToken (&acdEndsec, &tokenhandle, white);
	    (void) ajStrToLower (&acdEndsec);
	    acdEndsecCurr = acdNewEndsec (acdEndsec);
	    /* ajStrAssS(&acdvalue, acdParseValue(&tokenhandle, white));*/
	    acdPretty ("\nendsection:  %S\n", acdEndsec);
	    break;

	default:
	    ajFatal ("*** Help - unknown stage ***\n");
	    break;
	}

	(void) ajStrToken(&acdtmp, &tokenhandle, whiteplus);
	(void) ajStrToLower (&acdtmp);
    }
    acdLog ("-- All Done --\n");

    if (acderr) ajErr ("\n*** %d ERRORS FOUND ***\n", acderr);

    (void) ajStrDelReuse (&acdtmp);
    (void) ajStrDelReuse (&acdvalue);
    (void) ajStrDelReuse (&acdtoken);
    (void) ajStrTokenClear (&tokenhandle);

    return;
}


/* @funcstatic acdNoComment ***************************************************
**
** Strips comments from a character string (a line from an ACD file).
** Comments are blank lines or any text following a "#" character.
**
** @param [u] text [AjPStr*] Line of text from input file
** @return [void]
** @@
******************************************************************************/

static void acdNoComment (AjPStr* text)
{
    ajint i;
    char *cp;

    i = ajStrLen (*text);

    if (!i)				/* empty string */
	return;

    cp = strchr(ajStrStr(*text), '#');
    if (cp)
    {					/* comment found */
	*cp = '\0';
	(void) ajStrFix (*text);
    }

    return;
}

/* @funcstatic acdParseValue **************************************************
**
** Uses ajStrTok to complete a (possibly) quoted value.
** Note that ajStrTok has a stored internal copy of the text string
** which is set up at the start of acdParse and is being used here.
**
** Quotes can be single or double, or any kind of parentheses,
** depending on the first character of the next token examined.
**
** @param [u] tokenhandle [AjPStrTok*] Current parsing handle for input text
** @param [r] delim [char*] Delimiter string
** @return [AjPStr] String containing next value using acdStrTok
** @@
******************************************************************************/


static AjPStr acdParseValue (AjPStrTok* tokenhandle, char* delim)
{
    static AjPStr strp=NULL;
    static AjPStr tmpstrp=NULL;
    char  endq[]=" ";
    ajint iquote;
    char *cq;
    AjBool done = ajFalse;

    char *quotes = "\"'{(<";
    char *endquotes = "\"'})>";

    if(!ajStrToken (&strp, tokenhandle, delim))
	return NULL;

    cq = strchr(quotes, ajStrChar(strp, 0));
    if (!cq)				/* no quotes, simple return */
	return strp;


    /* quote found: parse up to closing quote then strip white space */

    (void) ajStrDelReuse (&tmpstrp);

    iquote = cq - quotes;
    endq[0] = endquotes[iquote];
    (void) ajStrTrim (&strp, 1);

    while(!done)
    {
	if (ajStrSuffixC(strp, endq))
	{				/* check for trailing quotes */
	    (void) ajStrTrim(&strp, -1);
	    done = ajTrue;
	}

	if (ajStrLen(strp))
	{
	    if (ajStrLen(tmpstrp))
	    {
		(void) ajStrAppC(&tmpstrp, " ");
		(void) ajStrApp (&tmpstrp, strp);
	    }
	    else
		(void) ajStrAssS(&tmpstrp, strp);
	}
	if (!done)
	    if (!ajStrToken (&strp, tokenhandle, delim))
		return NULL;
    }

    return tmpstrp;
}

/* @funcstatic acdParseValueRB ************************************************
**
** Uses ajStrTok to complete a (possibly) quoted value.
** Note that ajStrTok has a stored internal copy of the text string
** which is set up at the start of acdParse and is being used here.
**
** Quotes can be single or double, or any kind of parentheses,
** depending on the first character of the next token examined.
**
** @param [u] tokenhandle [AjPStrTok*] Current parsing handle for input text
** @param [r] delim [char*] Delimiter string
** @return [AjPStr] String containing next value using acdStrTok
** @@
******************************************************************************/

static AjPStr acdParseValueRB (AjPStrTok* tokenhandle, char* delim)
{
    static AjPStr strp=NULL;
    static AjPStr tmpstrp=NULL;
    char  endq[]=" ";
    char  endqbr[]=" ]";
    ajint iquote;
    char *cq;
    AjBool done = ajFalse;
    AjBool rightb = ajFalse;

    char *quotes = "\"'{(<";
    char *endquotes = "\"'})>";

    if (!ajStrToken (&strp, tokenhandle, delim))
	return NULL;

    cq = strchr(quotes, ajStrChar(strp, 0));
    if (!cq)				/* no quotes, simple return */
	return strp;


    /* quote found: parse up to closing quote then strip white space */

    (void) ajStrDelReuse (&tmpstrp);

    iquote = cq - quotes;
    endq[0] = endqbr[0] = endquotes[iquote];
    (void) ajStrTrim (&strp, 1);

    while (!done)
    {
	if (ajStrSuffixC(strp, endq))
	{
	    (void) ajStrTrim(&strp, -1);
	    done = ajTrue;
	}

	if (ajStrSuffixC(strp, endqbr))
	{
	    (void) ajStrTrim(&strp, -2);
	    rightb = ajTrue;
	    done = ajTrue;
	}
	if (ajStrLen(strp))
	{
	    if (ajStrLen(tmpstrp))
	    {
		(void) ajStrAppC(&tmpstrp, " ");
		(void) ajStrApp (&tmpstrp, strp);
	    }
	    else
		(void) ajStrAssS(&tmpstrp, strp);
	}
	if (!done)
	    if (!ajStrToken (&strp, tokenhandle, delim))
		return NULL;
    }

    if (rightb)
	(void) ajStrAppC(&tmpstrp, "]");

    return tmpstrp;
}

/* @funcstatic acdIsLeftB *****************************************************
**
** Tests a string against '[' to look for descent to the next level of parsing.
**
** @param [uP] pstr [AjPStr*] String which has a leading '[' removed if found
** @return [AjBool] ajTrue if start of string matches '['
** @@
******************************************************************************/

static AjBool acdIsLeftB (AjPStr* pstr)
{

    if (ajStrChar(*pstr, 0) != '[')
	return ajFalse;

    (void) ajStrTrim (pstr, 1);

    return ajTrue;
}

/* @funcstatic acdIsRightB ****************************************************
**
** Tests a string against ']' to look for ascent to a higher level of parsing.
**
** @param [uP] pstr [AjPStr*] String which has a trailing ']' removed if found
** @return [AjBool] ajTrue if end of string matches ']'
** @@
******************************************************************************/

static AjBool acdIsRightB (AjPStr* pstr)
{

    char ch = ajStrChar(*pstr, -1);

    if (ch != ']')
	return ajFalse;

    (void) ajStrTrim (pstr, -1);

    return ajTrue;
}

/* @funcstatic acdNewAppl *****************************************************
**
** Constructor front end for an application ACD object.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD application object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewAppl (AjPStr name)
{
    AcdPAcd acd;
    AcdPAcd qacd;
    AcdPAcd saveqacd = NULL;
    AcdPQual quals;
    static AjPStr qname = NULL;
    static AjPStr qtype = NULL;
    static ajint firstcall = 1;
    static ajint ikey;
    ajint i;

    if (firstcall)
    {
	ikey = acdFindKeyC("application");
	firstcall = 0;
    }

    i = 0;
    quals = acdQualAppl;
    if (quals)
    {
	while (quals[i].Name)
	{
	    (void) ajStrAssC (&qname, quals[i].Name);
	    (void) ajStrAssC (&qtype, quals[i].Type);
	    /*   qacd = acdNewQual (qname, qname, &qtype, o);*/
	    qacd = acdNewQualQual (qname, &qtype);
	    if (*quals[i].Default)
		(void) acdSetDefC (qacd, quals[i].Default);

	    if (!i)
		saveqacd = qacd;  /* save the location of the first one */
	    i++;
	}
    }

    acd = acdNewAcdKey (name, name, ikey);
    acd->Level = ACD_APPL;
    if (saveqacd)
	acd->AssocQuals = saveqacd;

    (void) ajStrDelReuse (&qname);
    (void) ajStrDelReuse (&qtype);

    return acd;
}

/* @funcstatic acdNewVar *****************************************************
**
** Constructor front end for a variable ACD object.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewVar (AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;

    if (firstcall)
    {
	ikey = acdFindKeyC ("variable");
	firstcall = 0;
    }

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_VAR;

    return acd;
}

/* @funcstatic acdNewSec *****************************************************
**
** Constructor front end for a section ACD object.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewSec (AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;

    if (firstcall)
    {
	ikey = acdFindKeyC ("section");
	firstcall = 0;
    }

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_SEC;

    return acd;
}

/* @funcstatic acdNewEndsec *****************************************************
**
** Constructor front end for an end of section ACD object.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD variable object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewEndsec (AjPStr name)
{
    AcdPAcd acd;
    static ajint firstcall = 1;
    static ajint ikey;

    if (firstcall)
    {
	ikey = acdFindKeyC ("endsection");
	firstcall = 0;
    }

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_ENDSEC;

    return acd;
}

/* @funcstatic acdNewQual *****************************************************
**
** Constructor front end for a qualifier ACD object.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] token [AjPStr] Qualifier name to be used on command line
** @param [r] type [AjPStr*] Type of value to be defined
** @param [r] pnum [ajint] Parameter number (zero for general qualifiers)
** @return [AcdPAcd] ACD parameter object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewQual (AjPStr name, AjPStr token, AjPStr* type,
			   ajint pnum)
{
    AcdPAcd acd;
    AcdPAcd qacd;
    AcdPAcd saveqacd = NULL;
    AcdPQual quals;
    static AjPStr qname = NULL;
    static AjPStr qtype = NULL;
    ajint itype;
    ajint i;

    if (!ajStrIsAlpha(name))
	ajWarn("ACD qualifier '%S' has invalid character(s)", name);
    if (!ajStrIsAlpha(token))
    {
	if (!ajStrMatch(name, token))
	    ajWarn("ACD token '%S' has invalid character(s)", token);
    }

    itype = acdFindType (*type);
    (void) ajStrAssC (type, acdType[itype].Name);

    /* do any associated qualifiers first so they are already complete
       when we come to the parameter later in processing */

    i = 0;
    quals = acdType[itype].Quals;	/* any associated qualifiers for itype?  */
    if (quals)
    {
	while (quals[i].Name)
	{
	    (void) ajStrAssC (&qname, quals[i].Name);
	    (void) ajStrAssC (&qtype, quals[i].Type);
	    qacd = acdNewQualQual (qname, &qtype);
	    if (*quals[i].Default)
		(void) acdSetDefC (qacd, quals[i].Default);
	    if (!i)
		saveqacd = qacd;	/* save the location of the first one */
	    i++;
	}
    }

    /* now set up the new parameter, and link in the list of qualifiers
       (if any) from earlier */

    acdTestUnknown (name, token, pnum);
    acd = acdNewAcd(name, token, itype);
    acd->Level = ACD_QUAL;
    if (saveqacd)
	acd->AssocQuals = saveqacd;

    (void) ajStrDelReuse (&qname);
    (void) ajStrDelReuse (&qtype);

    return acd;
}

/* @funcstatic acdNewQualQual *************************************************
**
** Constructor front end for an associated qualifier ACD object.
**
** @param [r] name [AjPStr] Qualifier name to be used on command line
** @param [r] type [AjPStr*] Type of value to be defined
** @return [AcdPAcd] ACD parameter object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewQualQual (AjPStr name, AjPStr* type)
{
    AcdPAcd acd;
    ajint itype;

    itype = acdFindType (*type);
    (void) ajStrAssC (type, acdType[itype].Name);

    acdTestAssoc (name);
    acd = acdNewAcd(name, name, itype);
    acd->Level = ACD_QUAL;
    acd->Assoc = ajTrue;

    return acd;
}

/* @funcstatic acdNewAcd ******************************************************
**
** General constructor for a new ACD qualifier object. Initialises all values
** in the ACD structure as appropriate.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] token [AjPStr] Qualifier name to be used on command line
** @param [r] itype [ajint] Integer type of value to be defined
**        as defined in acdFindType
** @return [AcdPAcd] ACD parameter object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewAcd (AjPStr name, AjPStr token, ajint itype)
{
    ajint i;

    if (acdListLast)
	acdListLast = AJNEW0(acdListLast->Next);
    else {
	acdListLast = AJNEW0(acdList);
    }
    (void) ajStrAssS(&acdListLast->Name, name);
    (void) ajStrAssS(&acdListLast->Token, token);
    acdListLast->Type = itype;

    /* we do NAttr and AttrStr explicitly for clarity, */
    /* though they are 0 and NULL from the AJNEW0 */

    switch (acdCurrentStage)
    {
    case QUAL_STAGE:
	acdListLast->NAttr = acdAttrCount(itype);
	break;
    default:
	acdListLast->NAttr = 0;
	break;
    }

    if (acdListLast->NAttr)
    {
	acdListLast->AttrStr = AJCALLOC (acdListLast->NAttr, sizeof (AjPStr));
	for (i = 0; i < acdListLast->NAttr; i++)
	    acdListLast->AttrStr[i] = ajStrNew();
    }
    else
	acdListLast->AttrStr = NULL;

    acdListLast->DefStr = AJCALLOC (nDefAttr, sizeof (AjPStr));
    for (i = 0; i < nDefAttr; i++)
	acdListLast->DefStr[i] = ajStrNew();

    acdListLast->Defined = ajFalse;
    acdListLast->Assoc = ajFalse;
    acdListLast->ValStr = ajStrNew();

    return acdListLast;
}

/* @funcstatic acdNewAcdKey ***************************************************
**
** General constructor for a new ACD general object. Initialises all values
** in the ACD structure as appropriate.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] token [AjPStr] Qualifier name to be used on command line
** @param [r] ikey [ajint] Integer type of value to be defined
**        as defined in acdFindKey
** @return [AcdPAcd] ACD parameter object for name.
** @@
******************************************************************************/

static AcdPAcd acdNewAcdKey (AjPStr name, AjPStr token, ajint ikey)
{
    ajint i;

    if (acdListLast)
	acdListLast = AJNEW0(acdListLast->Next);
    else {
	acdListLast = AJNEW0(acdList);
    }
    acdListLast->Next = NULL;
    (void) ajStrAssS (&acdListLast->Name, name);
    (void) ajStrAssS (&acdListLast->Token, token);
    acdListLast->PNum = 0;
    acdListLast->Level = ACD_APPL;
    acdListLast->Type = ikey;
    switch (acdCurrentStage)
    {
    case APPL_STAGE:
	acdListLast->NAttr = acdAttrKeyCount(ikey);
	break;
    case VAR_STAGE:
	acdListLast->NAttr = acdAttrKeyCount(ikey);
	break;
    case SEC_STAGE:
	acdListLast->NAttr = acdAttrKeyCount(ikey);
	break;
    default:
	acdListLast->NAttr = 0;
	break;
    }

    if (acdListLast->NAttr)
    {
	acdListLast->AttrStr = AJCALLOC (acdListLast->NAttr, sizeof (AjPStr));
	for (i = 0; i < acdListLast->NAttr; i++)
	    acdListLast->AttrStr[i] = ajStrNew();
    }
    else
	acdListLast->AttrStr = NULL;

    acdListLast->DefStr = NULL;

    acdListLast->SAttr = 0;
    acdListLast->SetAttr = NULL;
    acdListLast->SetStr = NULL;

    acdListLast->Defined = ajFalse;
    acdListLast->Assoc = ajFalse;
    acdListLast->AssocQuals = NULL;
    acdListLast->ValStr = ajStrNew();
    acdListLast->Value = NULL;

    return acdListLast;
}

/* @funcstatic acdTestUnknown *************************************************
**
** Makes sure that a name, token and pnum do not match any
** current ACD object.
**
** Aborts the program with a fatal error in case of problems.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] token [AjPStr] Qualifier name to be used on command line
** @param [r] pnum [ajint] Parameter number (zero for general qualifiers)
** @return [void]
** @@
******************************************************************************/

static void acdTestUnknown (AjPStr name, AjPStr token, ajint pnum)
{
    AcdPAcd pa;

    pa = acdFindAcd (name, token, pnum);
    if (pa)
    {
	if (pnum)
	    ajFatal ("Name/Token %S%d/%S%d not unique\n",
		     name, pnum, token, pnum);
	else
	    ajFatal ("Name/Token %S/%S not unique\n",
		     name, token);
    }

    return;
}

/* @funcstatic acdTestAssoc ***************************************************
**
** Makes sure that a name does not match any known ACD object name or token
** for all associated qualifiers.
**
** Aborts the program with a fatal error in case of problems.
**
** @param [r] name [AjPStr] Name or token name
** @return [void]
** @@
******************************************************************************/

static void acdTestAssoc (AjPStr name)
{
    AcdPAcd pa;

    for (pa=acdList; pa; pa=pa->Next)
    {
	if (!pa->Assoc && ajStrMatch(pa->Name, name) &&
	    ajStrMatch(pa->Token, name))
	{
	    ajFatal("Associated qualifier %S clashes with %S/%S in ACD file\n",
		    name, pa->Name, pa->Token);
	    break;
	}
    }

    return;
}

/* @funcstatic acdFindAcd *****************************************************
**
** Locates an ACD object by name, token and parameter number.
**
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] token [AjPStr] Qualifier name to be used on command line
** @param [r] pnum [ajint] Parameter number (zero for general qualifiers)
** @return [AcdPAcd] ACD object or NULL if not found
** @@
******************************************************************************/

static AcdPAcd acdFindAcd (AjPStr name, AjPStr token, ajint pnum)
{
    AcdPAcd pa;

    acdLog ("acdFindAcd ('%S', '%S', %d)\n", name, token, pnum);
    for (pa=acdList; pa; pa=pa->Next)
    {
	if (ajStrMatch(pa->Name, name) && ajStrMatch(pa->Token, token))
	    if (!pa->PNum || !pnum || (pa->PNum == pnum))
	    {
		acdLog ("..found '%S' %d\n", pa->Name, pa->PNum);
		return pa;
	    }
    }

    return NULL;
}

/* @funcstatic acdFindAssoc ***************************************************
**
** Locates an ACD object for an associated qualifier by name.
**
** Aborts the program with a fatal error in case of problems.
**
** @param [r] thys [AcdPAcd] ACD object for the parameter
** @param [r] name [AjPStr] Token name to be used by applications
** @return [AcdPAcd] ACD object for the selected qualifier
** @@
******************************************************************************/

static AcdPAcd acdFindAssoc (AcdPAcd thys, AjPStr name)
{
    AcdPAcd pa;
    ajint ifound=0;
    AcdPAcd ret=NULL;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (pa=thys->AssocQuals; pa->Assoc; pa=pa->Next)
    {
	if (ajStrPrefix(pa->Name, name))
	{
	    if (ajStrMatch(pa->Name, name))
		return pa;
	    ifound++;
	    ret = pa;
	    acdAmbigApp (&ambigList, pa->Name);
	}
    }

    if (ifound == 1)
	return ret;
    if (ifound > 1)
    {
	ajWarn ("ambiguous name/token %S (%S)", name, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }

    ajFatal ("ACD Parsing: Attribute or qualifier %S unknown\n",
	     name);
  
    return NULL;
}

/* @funcstatic acdTestQualC ***************************************************
**
** Tests whether "name" is a valid qualifier name.
** To be valid, it must begin with "-".
** If not, it can be taken as a value for the previous qualifier
**
** @param [r] name [char*] Qualifier name
** @return [AjBool] ajTrue if
** @@
******************************************************************************/

static AjBool acdTestQualC (char *name)
{
    static AjPStr  qstr = NULL;
    static AjPStr  qmaster = NULL;
    AcdPAcd pa;
    AcdPAcd qa;
    AcdPAcd savepa=NULL;
    ajint qnum = 0;
    ajint i;
    ajint ifound=0;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    ajDebug ("acdTestQualC '%s'\n", name);

    if (*name != '-')
	return ajFalse;

    (void) ajStrAssC (&qstr, name+1);

    i = ajStrFindC(qstr, "=");		/* qualifier with value */
    if (i > 0)
	(void) ajStrSub(&qstr, 0, i-1);	/* strip any value and keep testing */

    acdQualParse (&qstr, &qmaster, &qnum);

    if (ajStrLen(qmaster))
    {
	for (pa=acdList; pa; pa=pa->Next)
	{
	    if (ajStrMatch (pa->Name, qmaster))
	    {
		ajDebug ("  *master matched* '%S'\n", pa->Name);
		qa = acdFindAssoc(pa, qstr);
		if (qa)
		    return ajTrue;
		else
		    return ajFalse;
	    }
	}
	if (ajStrPrefix (pa->Name, qstr))
	{
	    ifound++;
	    savepa = pa;
	    acdAmbigApp (&ambigList, pa->Name);
	}
	ajDebug ("   ifound: %d\n", ifound);

	if (ifound == 1)
	{
	    qa = acdFindAssoc(savepa, qstr);
	    if (qa)
		return ajTrue;
	    else
		return ajFalse;
	}

	if (ifound > 1)
	{
	    ajWarn ("ambiguous master qualifier %s (%S)", name, ambigList);
	    (void) ajStrDelReuse(&ambigList);
	}
    }
    else
    {
	for (pa=acdList; pa; pa=pa->Next)
	{
	    if (ajStrMatch (pa->Name, qstr))
	    {
		ajDebug ("   *matched* '%S'\n", pa->Name);
		return ajTrue;
	    }
	    if (ajStrPrefix (pa->Name, qstr))
	    {
		ifound++;
		acdAmbigApp (&ambigList, pa->Name);
	    }
	}

	ajDebug ("   ifound: %d\n", ifound);

	if (ifound == 1)
	    return ajTrue;

	if (ifound > 1)
	{
	    ajWarn ("ambiguous qualifier %s (%S)", name, ambigList);
	    (void) ajStrDelReuse(&ambigList);
	}
    }

    return ajFalse;
}

/* @funcstatic acdFindType ****************************************************
**
** Looks for a Type by name, and returns the number in acdType
**
** @param [r] type [AjPStr] String containing the type name
** @return [ajint] Integer representing the type (if know). Can be
**         used as position in the acdType array.
** @error If not found, the return value points to the maximum position in
**        acdType which is set to NULL throughout.
** @@
******************************************************************************/

static ajint acdFindType (AjPStr type)
{
    ajint i;
    ajint ifound=0;
    ajint j=0;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (i=0; acdType[i].Name; i++)
    {
	if (ajStrMatchC (type, acdType[i].Name))
	    return i;
	if (ajStrPrefixCO (acdType[i].Name, type))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC (&ambigList, acdType[i].Name);
	}
    }

    if (ifound > 1)
    {
	ajWarn ("ambiguous type %S (%S)", type, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound != 1)
	ajFatal ("unknown type: '%S'\n", type);

    return j;
}

/* @funcstatic acdFindTypeC ***************************************************
**
** Looks for a Type by name, and returns the number in acdType
**
** @param [r] type [char*] Text string containing the type name
** @return [ajint] Integer representing the type (if know). Can be
**         used as position in the acdType array.
** @error If not found, the return value points to the maximum position in
**        acdType which is set to NULL throughout.
** @@
******************************************************************************/

static ajint acdFindTypeC (char* type)
{
    ajint i;
    ajint ifound=0;
    ajint j=0;
    ajint ilen = strlen(type);
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (i=0; acdType[i].Name; i++)
    {
	if (!strcmp (type, acdType[i].Name))
	    return i;
	if (strncmp (acdType[i].Name, type, ilen))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC (&ambigList, acdType[i].Name);
	}
    }

    if (ifound > 1)
    {
	ajWarn ("ambiguous type %s (%S)", type, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound != 1)
	ajFatal ("unknown type: '%s'\n", type);

    return j;
}

/* #funcstatic acdFindKey *****************************************************
**
** Looks for a Keyword by name, and returns the number in acdKeywords
**
** #param [r] key [AjPStr] String containing the keyword name
** #return [ajint] Integer representing the keyword (if known). Can be
**         used as position in the acdKeywords array.
** #error If not found, the return value points to the maximum position in
**        acdKeywords which is set to NULL throughout.
** @@
******************************************************************************/

/*static ajint acdFindKey (AjPStr key) {

  ajint i;
  ajint ifound=0;
  ajint j;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  for (i=0; acdKeywords[i].Name; i++) {
    if (ajStrMatchC (key, acdKeywords[i].Name))
      return i;
    if (ajStrPrefixCO (acdKeywords[i].Name, key)) {
      ifound++;
      j = i;
      acdAmbigAppC (&ambigList, acdKeywords[i].Name);
    }
  }

  if (ifound > 1) {
    ajWarn ("ambiguous keyword %S (%S)", key, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }
  if (ifound != 1)
    ajFatal ("unknown keyword: '%S'\n", key);

  return j;
}*/

/* @funcstatic acdFindKeyC ****************************************************
**
** Looks for a Keyword by name, and returns the number in acdKeywords
**
** @param [r] key [char*] Text string containing the keyword name
** @return [ajint] Integer representing the keyword (if known). Can be
**         used as position in the acdKeywords array.
** @error If not found, the return value points to the maximum position in
**        acdKeywords which is set to NULL throughout.
** @@
******************************************************************************/

static ajint acdFindKeyC (char* key)
{
    ajint i;
    ajint ifound=0;
    ajint j=0;
    ajint ilen = strlen(key);
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (i=0; acdKeywords[i].Name; i++)
    {
	if (!strcmp (key, acdKeywords[i].Name))
	    return i;
	if (strncmp (acdKeywords[i].Name, key, ilen))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC (&ambigList, acdKeywords[i].Name);
	}
    }

    if (ifound > 1)
    {
	ajWarn ("ambiguous keyword %s (%S)",key, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound != 1)
	ajFatal ("unknown keyword: '%s'\n", key);

    return j;
}

/*===========================================================================*/
/*======================== Talking to the User ==============================*/
/*===========================================================================*/

/* @funcstatic acdReplyInit ***************************************************
**
** Builds a default value for the reply first time around. Uses a default
** specially set in the ACD, or (if none) uses the default string passed in
** parameter "defval" and also sets this as the default in the ACD.
**
** @param [r] thys [AcdPAcd] ACD object for current item.
** @param [r] defval [char*] Default value, as a string
** @param [wP] reply [AjPStr*] String containing default reply
** @return [AjBool] ajTrue if a value in the ACD was used.
** @@
******************************************************************************/

static AjBool acdReplyInit (AcdPAcd thys, char *defval, AjPStr* reply)
{
    AjPStr def;

    if (thys->DefStr)
    {
	def = thys->DefStr[DEF_DEFAULT];
	if (ajStrLen(def))
	{
	    (void) ajStrAssS(reply, def);
	    (void) acdVarResolve(reply);
	    return ajTrue;
	}
    }

    (void) ajStrAssC (reply, defval);
    (void) ajStrAssC (&thys->DefStr[DEF_DEFAULT], defval);

    return ajFalse;
}

/* @funcstatic acdUserGet *****************************************************
**
** Given an ACD containing a defined prompt, a help string,
** or a default (string) value, prompts
** the user for a string value and returns it.
**
** If prompt is set, it is used. Otherwise, info can be used.
** If neither are defined, the item name and type are used to make a
** meaningful prompt.
**
** The default value is offered if it is set.
**
** The user response is returned in "reply"
**
** If -auto is in effect, fails if there is no value. 
**
** @param [r] thys [AcdPAcd] ACD object for current item.
** @param [wP] reply [AjPStr*] The user response, or
**        the default value if accepted.
** @return [AjBool] ajTrue if reply contains any text.
** @@
******************************************************************************/

static AjBool acdUserGet (AcdPAcd thys, AjPStr* reply)
{
    static AjPStr msg = NULL;
    AjBool ret = ajFalse;

    AjPStr prompt;
    AjPStr info;
    AjPStr code;
    AjPStr help;

    static AjPStr defreply=NULL;

    ajDebug ("acdUserGet '%S' reply '%S'\n", thys->Name, *reply);

    if (thys->DefStr && !acdAuto)
    {
	prompt = thys->DefStr[DEF_PROMPT];
	info = thys->DefStr[DEF_INFO];
	code = thys->DefStr[DEF_CODE];
	help = thys->DefStr[DEF_HELP];

	(void) ajStrAssS(&defreply, *reply);

	if (ajStrLen(code))
	    (void) acdCodeGet (code, &msg);

	else if (ajStrLen(prompt))
	    (void) ajStrAssS (&msg, prompt);

	else if (ajStrLen(info))
	    (void) ajStrAssS (&msg, info);

	else if (ajStrLen(help))
	    (void) ajStrAssS (&msg, help);

	else
	{
	    if (!acdCodeDef (thys, &msg))
	    {
		(void) ajStrAssCL(&msg, "", 512);
		(void) ajFmtPrintS (&msg, "-%S : enter %s value",
				    thys->Name, acdType[thys->Type].Name);
	    }
	}

	(void) acdVarResolve(&msg);

	(void) ajDebug ("acdUserGet '%S' defreply '%S' msg '%S'\n",
			thys->Name, defreply, msg);

	if (ajStrLen(defreply))
	    ret = ajUserGet (reply, "%S [%S]: ", msg, defreply);
	else
	    ret = ajUserGet (reply, "%S: ", msg);
	if (!ret)
	    (void) ajStrAssS (reply, defreply);
    }
  
    if (ajStrLen(*reply))
	ret = ajTrue;

    (void) ajStrDelReuse (&msg);
    (void) ajStrDelReuse (&defreply);

    return ret;
}

/* @funcstatic acdUserGetPrompt ***********************************************
**
** Given a defined prompt, prompts
** the user for a string value and returns it.
**
** The default value is offered if it is set.
**
** The user response is returned in "reply"
**
** If -auto is in effect, fails if there is no value. 
**
** @param [r] prompt [char*] prompt string
** @param [wP] reply [AjPStr*] The user response, or
**        the default value if accepted.
** @return [AjBool] ajTrue if reply contains any text.
** @@
******************************************************************************/

static AjBool acdUserGetPrompt (char* prompt, AjPStr* reply)
{
    AjBool ret = ajFalse;

    static AjPStr defreply=NULL;

    if (!acdAuto)
    {
	(void) ajStrAssS(&defreply, *reply);
	if (ajStrLen(defreply))
	    ret = ajUserGet (reply, "    %s [%S]: ", prompt, defreply);
	else
	    ret = ajUserGet (reply, "    %s: ", prompt);
	if (!ret)
	    (void) ajStrAssS (reply, defreply);
    }

    if (ajStrLen(*reply))
	ret = ajTrue;

    (void) ajStrDelReuse (&defreply);

    return ret;
}

/* @funcstatic acdBadRetry ***************************************************
**
** Writes a message to stderr, and kills the application.
**
** @param [r] thys [AcdPAcd] ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdBadRetry (AcdPAcd thys)
{
    ajDie ("%S terminated: Bad value for '-%S' and no more retries\n",
	   acdProgram, thys->Name);
}

/* @funcstatic acdBadVal ******************************************************
**
** Writes a message to stderr, returns only if this is a required value
** and we are prompting for values.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] required [AjBool] If true, value was required and
**        failure was fatal.
** @param [r] fmt [char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
** @@
******************************************************************************/

static void acdBadVal (AcdPAcd thys, AjBool required, char *fmt, ...)
{
    va_list args;
    static AjPStr msg = NULL;
    static AjPStr name = NULL;

    ajStrAssC(&name,ajStrStr(thys->Name));
  
    (void) ajFmtPrintS(&msg, "option -%S: %s", name, fmt);
    va_start (args, fmt) ;
    ajVErr (ajStrStr(msg), args);
    va_end (args) ;

    if (!required)
	ajDie ("%S terminated: Bad value for option and no prompt\n",
	       acdProgram);

    if (acdAuto)
	ajDie ("%S terminated: Bad value with -auto defined.\n",
	       acdProgram);

    return;
}

/* @funcstatic acdSetXxxx *****************************************************
**
** Dummy function to handle prompting and validation for an ACD item type.
** A similar routine is needed for any new item type. See the other acdSet
** functions for ideas on what to include.
**
** This functions knows all the attributes, defaults, associated qualifiers
** and validation rules for type Xxxx. If any attributes (etc) are added,
** this is where the processing will be done.
**
** The final value (of type Xxxx) is set in thys as Value
** and a string equivalent for easy printing is set in thys as Valstr
**
** The function does not need to return a value. It either succeeds in filling
** in all values, or aborts with a suitable error messages.
**
** @param [r] thys [AcdPAcd] ACD for current item (which is
**        always of type Xxxx)
** @return [void]
** @@
******************************************************************************/

static void acdSetXxxx (AcdPAcd thys)
{
    AjPStr val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    AjPStr reply = NULL;
    ajint itry;

    /* create storage for val if needed, e.g. with AJNEW0(val) */

    val = NULL;				/* set a default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "", &reply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {

	if (required)			/* need to prompt? */
	    (void) acdUserGet (thys, &reply);

	ok = ajTrue;			/* test the value somehow */
    }
    if (!ok)
	acdBadRetry (thys);

    (void) ajStrAssS(&val, reply);	/* use the validated reply */
    thys->Value = val;			/* set the value */
    (void) ajStrAssS (&thys->ValStr, val); /* set the string value */

    return;
}

/* @funcstatic acdSetAppl *****************************************************
**
** Makes sure all application ACD item values have been set.
**
** Called when an "appl" type ACD item is checked. Should not be called
** for any other item. The "appl" item may not be the first, as there
** are some 'standard' qualifiers set in advance for debugging, help and
** so on.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a report of what the program does which would appear
** before any user prompts.
**
** @param [r] thys [AcdPAcd] ACD for the application item.
** @return [void]
** @@
******************************************************************************/

static void acdSetAppl (AcdPAcd thys)
{

    static AjPStr appldoc = NULL;

    (void) acdAttrResolve (thys, "documentation", &appldoc);

    if (!acdAuto && ajStrLen(appldoc))
    {
	(void) ajStrWrapLeft (&appldoc, 75, 0);
	ajUser("%S", appldoc);
    }

    return;
}

/* @funcstatic acdSetEndsec *****************************************************
**
** Ends the current ACD section
**
** Called when an "endsection" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a blank line at the end of a section where something was prompted for.
**
** @param [r] thys [AcdPAcd] ACD for the endsection item.
** @return [void]
** @@
******************************************************************************/

static void acdSetEndsec (AcdPAcd thys)
{

    if (thys->DefStr)
	(void) ajStrAssS (&thys->ValStr, thys->DefStr[DEF_DEFAULT]);

    (void) acdVarResolve (&thys->ValStr);

    return;
}

/* @funcstatic acdSetSec *****************************************************
**
** Starts a new ACD section.
**
** Called when a "section" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a prompt issued before the first prompt for the section.
** This would be stored (the 'info' attribute) and used in the standard
** prompting functions.
**
** @param [r] thys [AcdPAcd] ACD for the section item.
** @return [void]
** @@
******************************************************************************/

static void acdSetSec (AcdPAcd thys)
{
    ajint border = 1;
    AjPStr comment = NULL;
    AjPStr folder  = NULL;
    AjPStr info    = NULL;
    AjPStr side    = NULL;
    AjPStr type    = NULL;
    AjPStr tmpstr  = NULL;

    char* sideVal[] = {"top", "bottom", "left", "right", NULL};
    char* typeVal[] = {"frame", "page", NULL};

    if (acdAttrToStr(thys, "type", "", &type)) {
      if (!acdVocabCheck(type, typeVal))
	(void) ajFatal("section %S, bad attribute value type: %S",
		       thys->Name, type);
    }

    if (acdAttrToInt (thys, "border", 1, &border)) {
	if (!ajStrMatchCaseC(type, "frame"))
	  (void) ajWarn("section %S, border only used by type: frame",
			thys->Name);
	if (border < 1) {
	    (void) acdAttrToStr(thys, "border", "", &tmpstr);
	    (void) ajFatal ("section %S, bad attribute value type: %S",
			    tmpstr);
	}
    }

    (void) acdAttrToStr(thys, "comment", "", &comment);
    if (acdAttrToStr(thys, "folder", "", &folder)) {
	if (!ajStrMatchCaseC(type, "page"))
	    ajWarn("section %S, folder only used by type: page",
		   thys->Name);
    }

    (void) acdAttrToStr(thys, "info", "", &info);
    if (acdAttrToStr(thys, "side", "", &side)) {
	if (!acdVocabCheck(side, sideVal))
	    ajFatal("section %S, bad attribute value side: %S",
		    thys->Name, side);
	if (!ajStrMatchCaseC(type, "frame"))
	    ajWarn("section %S, side only used by type: frame",
		   thys->Name);
    }

    ajStrDel(&comment);
    ajStrDel(&folder);
    ajStrDel(&info);
    ajStrDel(&side);
    ajStrDel(&type);
    ajStrDel(&tmpstr);
    return;
}

/* @funcstatic acdSetVar *****************************************************
**
** Defines an ACD variable.
**
** Called when a "variable" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a report of what the program does which would appear
** before any user prompts.
**
** @param [r] thys [AcdPAcd] ACD for the application item.
** @return [void]
** @@
******************************************************************************/

static void acdSetVar (AcdPAcd thys)
{

    if (thys->DefStr)
	(void) ajStrAssS (&thys->ValStr, thys->DefStr[DEF_DEFAULT]);

    (void) acdVarResolve (&thys->ValStr);

    return;
}

/* @func ajAcdGetArray ********************************************************
**
** Returns an item of type array as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPFloat] Floating point array object
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPFloat ajAcdGetArray (char *token)
{
    return acdGetValue (token, "array");
}

/* @funcstatic acdSetArray ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD floating point array item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "0.0" with a size of 1.
**
** Min and max limits, if set, are applied without comment.
** Precision is provided for logging purposes but otherwise not (yet) used.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToFloat
** @@
******************************************************************************/

static void acdSetArray (AcdPAcd thys)
{
    AjPFloat val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool warnrange;

    float fmin;
    float fmax;
    ajint precision;
    ajint size;
    float sum;
    float tolerance;
    float fdef;
    float ftol;
    float ftot;
    static AjPStr deflist = NULL;
    ajint i;
    float* array;

    (void) acdAttrToFloat (thys, "minimum", -FLT_MAX, &fmin);
    acdLog ("minimum: %e\n", fmin);

    (void) acdAttrToFloat (thys, "maximum", FLT_MAX, &fmax);
    acdLog ("maximum: %e\n", fmax);

    (void) acdAttrToFloat (thys, "sum", 1.0, &sum);
    acdLog ("sum: %e\n", sum);

    (void) acdAttrToFloat (thys, "tolerance", 0.01, &tolerance);
    acdLog ("tolerance: %e\n", tolerance);

    (void) acdAttrToInt (thys, "precision", 3, &precision);
    acdLog ("precision: %d\n", precision);

    (void) acdAttrToBool (thys, "warnrange", ajTrue, &warnrange);
    acdLog ("warnrange: %B\n", warnrange);

    (void) acdAttrToInt (thys, "size", 1, &size);
    acdLog ("size: %d\n", size);
    if (size < 1)
	ajFatal ("Array size %d less than 1", size);

    fdef = sum / ((float) size);

    for (i=0; i < size; i++)
    {
	if (i)
	    ajStrAppK (&deflist, ' ');
	ajFmtPrintAppS (&deflist, "%.*f", precision, fdef);
    }

    val = ajFloatNew();			/* create storage for the result */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, ajStrStr(deflist), &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	ok = ajFloatParse(reply, &val);
	if (!ok)
	    acdBadVal (thys, required,
		       "Invalid array value '%S', please try again",
		       reply);

	array = ajFloatFloat(val);
	ftot = 0.0;
	for (i=0; i< size; i++)
	{
	    if (array[i] < fmin)
	    {				/* reset within limits */
		if (warnrange)
		    ajWarn("floating point value [%d] out of range %.*f "
			   "less than %.*f\n",
			   i+1, precision, array[i], precision, fmin);
		array[i] = fmin;
	    }

	    if (array[i] > fmax)
	    {
		if (warnrange)
		    ajWarn("floating point value [%d] out of range %.*f "
			   "more than %.*f\n",
			   i+1, precision, array[i], precision, fmax);
		array[i] = fmax;
	    }
	    ftot += array[i];
	}

	ftol = (float) fabs (ftot -sum);
	if (ftol > tolerance)
	{
	    ajWarn ("Bad total %.*f, required total is %.*f with "
		    "tolerance %.*f",
		    precision, ftot, precision, sum,precision, tolerance);
	    acdBadVal (thys, required,
		       "Invalid array value '%S', please try again",
		       reply);
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajFloatStr(&thys->ValStr, val, precision);

    return;
}

/* @func ajAcdGetBool *********************************************************
**
** Returns an item of type Bool as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjBool] Boolean value from ACD item
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjBool ajAcdGetBool (char *token)
{
    AjBool* val;

    val = acdGetValue (token, "bool");
    return *val;
}

/* @funcstatic acdSetBool *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD boolean item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "N" for ajFalse.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToBool
** @@
******************************************************************************/

static void acdSetBool (AcdPAcd thys)
{
    AjBool* val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;

    AJNEW0(val);			/* create storage for the result */

    *val = ajFalse;			/* set the default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "N", &defreply);

    ajDebug("acdSetBool -%S def: %S\n", thys->Name, defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);
    
	ok = ajStrToBool(reply, val);
	if (!ok)
	    acdBadVal (thys, required, "Invalid Y/N value '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssC(&thys->ValStr, ajStrBool(*val));

    (void) acdSetQualAppl (thys, *val);	/* check special application
					   booleans */

    ajDebug("acdSetBool -%S val: %B\n", thys->Name, *val);

    if (ajStrMatchC(thys->Name, "help"))
	acdHelp();

    if (ajStrMatchC(thys->Name, "acdpretty") && *val)
	ajExit();

    return;
}


/* @func ajAcdGetCodon *******************************************************
**
** Returns an item of type Codon as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPCod] Codon object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPCod ajAcdGetCodon (char *token)
{
    return acdGetValue (token, "codon");
}


/* @funcstatic acdSetCodon ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if filtering is on) is "stdin", but then
** prompting is turned off.
**
** Otherwise there is no default value unles the ACD file has one.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetCodon (AcdPAcd thys)
{
    AjPCod val;

    AjPStr name=NULL;
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;

    val = ajCodNew();			/* set the default value */
    (void) acdAttrResolve (thys, "name", &name);
    if(!ajStrLen(name))
	(void) ajStrAssC(&name,DEFCODON);
  
    required = acdIsRequired(thys);

    (void) acdReplyInit (thys, ajStrStr(name), &defreply);
    acdPromptCodon (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {

	ok = ajTrue;	/* accept the default if nothing changes */

	(void) ajStrAss (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    if (!ajCodRead(reply,&val))
	    {
		acdBadVal (thys, required,
			   "Unable to read codon usage '%S'", reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    acdBadVal (thys, required, "Codon file is required");
	    ok = ajFalse;
	}
    }

    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    ajStrDel(&name);
    return;
}


/* @func ajAcdGetDirlist *****************************************************
**
** Returns a list of files in a given directory.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPList] List of files.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPList ajAcdGetDirlist (char *token)
{
    return acdGetValue (token, "dirlist");
}


/* @funcstatic acdSetDirlist ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD directory item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "." the current directory.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetDirlist (AcdPAcd thys)
{
    AjPList val;
    AjPStr  t;
    AjPStr  v;
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool nullok=ajFalse;
    AjBool dopath;
    ajint n;
    ajint i;
    
    val = NULL;

    (void) acdAttrToBool (thys, "fullpath", ajFalse, &dopath);
    acdLog ("nullok: %B\n", nullok);
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, ".", &defreply);
    acdPromptDirlist(thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    if (dopath)
		ok = ajFileDirPath(&reply);
	    else
		ok = ajFileDir(&reply);
	    if (!ok)
		acdBadVal (thys, required,
			   "Unable to open file '%S' for input", reply);
	}
	else
	{
	    if (!nullok)
		acdBadVal (thys, required, "Input file is required");
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);

    val = ajListstrNew();			/* set the default value */
    t   = ajStrNewC("*");
    ajFileScan(reply,t,&val,ajFalse,ajFalse,NULL,NULL,ajFalse,NULL);

    n = ajListLength(val);
    for(i=0;i<n;++i)
    {
/*	ajFmtPrintS(&t,"%S/",reply);*/
	ajFmtPrintS(&t,"");
	ajListPop(val,(void **)&v);
	ajStrApp(&t,v);
	ajStrAssC(&v,ajStrStr(t));
	ajListPushApp(val,(void *)v);
    }

    ajStrDel(&t);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}


/* @func ajAcdGetDatafile *****************************************************
**
** Returns an item of type Datafile as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPFile] File object. The file was already opened by
**         ajFileDataNew so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPFile ajAcdGetDatafile (char *token)
{
    return acdGetValue (token, "datafile");
}


/* @funcstatic acdSetDatafile *************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD datafile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "programname.dat"
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetDatafile (AcdPAcd thys)
{
    AjPFile val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool nullok;
    static AjPStr name = NULL;
    static AjPStr ext = NULL;

    static AjPStr datafname = NULL;

    val = NULL;				/* set the default value */

    (void) acdAttrResolve (thys, "name", &name);
    (void) acdAttrResolve (thys, "extension", &ext);

    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);

    required = acdIsRequired(thys);
    (void) acdDataFilename (&datafname, name, ext);
    (void) acdReplyInit (thys, ajStrStr(datafname), &defreply);
    acdPromptInfile (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {

	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    ajFileDataNew(reply, &val);
	    if (!val)
	    {
		acdBadVal (thys, required,
			   "Unable to open data file '%S' for input", reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    if (!nullok)
	    {
		acdBadVal (thys, required, "Input file is required");
		ok = ajFalse;
	    }
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetDirectory ****************************************************
**
** Returns an item of type AjPStr which has been validated as a
** directory.
**
** Optionally can be forced to have a fully qualified path when returned.
**
** @param [r] token [char*] Text token name
** @return [AjPStr] String containing a directory name
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr ajAcdGetDirectory (char *token)
{
    return acdGetValue (token, "directory");
}

/* @funcstatic acdSetDirectory ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD directory item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value is "." the current directory.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetDirectory (AcdPAcd thys)
{
    AjPStr val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool nullok=ajFalse;
    AjBool dopath;

    val = NULL;				/* set the default value */

    (void) acdAttrToBool (thys, "fullpath", ajFalse, &dopath);
    acdLog ("nullok: %B\n", nullok);
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, ".", &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    if (dopath)
		ok = ajFileDirPath(&reply);
	    else
		ok = ajFileDir(&reply);
	    if (!ok)
		acdBadVal (thys, required,
			   "Unable to open file '%S' for input", reply);
	}
	else
	{
	    if (!nullok)
		acdBadVal (thys, required, "Input file is required");
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);

    ajStrAssS (&val, reply);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetFeat *********************************************************
**
** Returns an item of type Features as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPFeattable] Feature Table object. The table was already loaded by
**         acdSetFeat so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPFeattable ajAcdGetFeat (char *token)
{
    return acdGetValue (token, "features");
}

/* @funcstatic acdSetFeat *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD feature table item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-fformat", "-fopenfile"
** are applied to the UFO before reading the sequence.
**
** Associated qualifiers "-fbegin", "-fend" and "-freverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the feature table,
** and the resulting table is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/

static void acdSetFeat (AcdPAcd thys)
{
    AjPFeattable val = NULL;
    AjPFeattabIn tabin = NULL;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;

    static AjPStr infname = NULL;

    static AcdOAttr setattr[] =
    {
	{"begin", VT_INT},
	{"end", VT_INT},
	{"length", VT_INT},
	{"protein", VT_BOOL},
	{"nucleic", VT_BOOL},
	{"name", VT_STR},
	{"size", VT_STR},
	{NULL, VT_NULL} };

    ajint fbegin=0;
    ajint fend=0;
    AjBool freverse=ajFalse;
    AjBool fprompt=ajFalse;
    ajint iattr;

    tabin = ajFeattabInNew();		/* set the default value */

    required = acdIsRequired(thys);
    (void) acdQualToBool (thys, "fask", ajFalse, &fprompt, &defreply);

    (void) acdInFilename (&infname);
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
    acdPromptFeat (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	(void) acdGetValueAssoc (thys, "fformat", &tabin->Formatstr);
	(void) acdGetValueAssoc (thys, "fopenfile", &tabin->Filename);

	ok = ajFeatUfoRead(&val, tabin, reply);
	if (!ok)
	    acdBadVal (thys, required,
		       "Unable to read feature table '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    (void) acdInFileSave(ajFeatGetName(val)); /* save the sequence name */

    /* now process the begin, end and reverse options */

    ok = acdQualToInt (thys, "fbegin", 1, &fbegin, &defreply);
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);
	if (fprompt)
	    (void) acdUserGetPrompt (" Begin at position", &reply);
	ok = ajStrToInt(reply, &fbegin);
	if (!ok)
	    acdBadVal (thys, ajTrue,
		       "Invalid integer value '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    ok = acdQualToInt (thys, "fend", ajFeatLen(val), &fend, &defreply);
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);
	if (fprompt)
	    (void) acdUserGetPrompt ("   End at position", &reply);
	ok = ajStrToInt(reply, &fend);
	if (!ok)
	    acdBadVal (thys, ajTrue,
		       "Invalid integer value '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    ok = acdQualToBool (thys, "freverse", ajFalse, &freverse, &defreply);
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);
	if (fprompt)
	    (void) acdUserGetPrompt ("    Reverse strand", &reply);
	ok = ajStrToBool(reply, &freverse);
	if (!ok)
	    acdBadVal (thys, ajTrue,
		       "Invalid Y/N value '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    acdLog ("sbegin: %d, send: %d, freverse: %B\n",
	    fbegin, fend, freverse);
  
    /*
       if (freverse)
       ajFeatReverse (val);
       
       ajFeattabSetRange(val, fbegin, fend);
       ajFeattabInSetRange(tabin, fbegin, fend);
       */

    ajFeattabInDel (&tabin);

    /* features tables have special set attributes */

    thys->SAttr = acdAttrListCount (setattr);
    thys->SetAttr = &setattr[0];
    thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

    iattr = 0;
    (void) ajStrFromInt (&thys->SetStr[iattr++], fbegin);
    (void) ajStrFromInt (&thys->SetStr[iattr++], fend);
    (void) ajStrFromInt (&thys->SetStr[iattr++], ajFeatLen(val));
    (void) ajStrFromBool (&thys->SetStr[iattr++], ajFeatIsProt(val));
    (void) ajStrAssS (&thys->SetStr[iattr++], val->Name);
    (void) ajStrFromInt (&thys->SetStr[iattr++], ajFeatSize(val));

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetFeatout ******************************************************
**
** Returns an item of type FeatOut as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPFeattabOut] Feature Table output object. Already opened
**                      by acdSetFeatout so this just returns the object
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPFeattabOut ajAcdGetFeatout (char *token)
{
    return acdGetValue (token, "featout");
}

/* @funcstatic acdSetFeatout **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD feature table item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-fformat", "-fopenfile"
** are applied to the UFO before reading the sequence.
**
** Associated qualifiers "-fbegin", "-fend" and "-freverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the feature table,
** and the resulting table is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/

static void acdSetFeatout (AcdPAcd thys)
{
    AjPFeattabOut val = NULL;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;

    static AjPStr name = NULL;
    static AjPStr ext = NULL;
    static AjPStr outfname = NULL;

    static AcdOAttr setattr[] =
    {
	{"begin", VT_INT},
	{"end", VT_INT},
	{"length", VT_INT},
	{"protein", VT_BOOL},
	{"nucleic", VT_BOOL},
	{"name", VT_STR},
	{NULL, VT_NULL} };

    required = acdIsRequired(thys);
    val = ajFeattabOutNew();

    acdAttrResolve (thys, "name", &name);
    if (!acdGetValueAssoc (thys, "offormat", &val->Formatstr))
	(void) acdAttrResolve (thys, "extension", &ext);

    (void) acdOutFilename (&outfname, name, val->Formatstr);
    (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    acdPromptFeatout (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	(void) acdGetValueAssoc (thys, "ofopenfile", &val->Filename);
	ok = ajFeattabOutOpen (val, reply);
	if (!ok)
	    acdBadVal (thys, required,
		       "Unable to read sequence '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    /* features tables have special set attributes */

    thys->SAttr = acdAttrListCount (setattr);
    thys->SetAttr = &setattr[0];
    thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

    /*
       (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_BEGIN], ajSeqBegin(val));
       (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_END], ajSeqEnd(val));
       (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_LENGTH], ajSeqLen(val));
       (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqIsProt(val));
       (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqIsNuc(val));
       (void) ajStrAssS (&thys->SetStr[ACD_SEQ_NAME], val->Name);
       */

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetFloat ********************************************************
**
** Returns an item of type Float as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [float] Floating point value from ACD item
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

float ajAcdGetFloat (char *token)
{
    float* val;

    val = acdGetValue (token, "float");
    return *val;
}

/* @funcstatic acdSetFloat ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD floating point item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "0.0".
**
** Min and max limits, if set, are applied without comment.
** Precision is provided for logging purposes but otherwise not (yet) used.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToFloat
** @@
******************************************************************************/

static void acdSetFloat (AcdPAcd thys)
{
    float* val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool warnrange;

    float fmin;
    float fmax;
    ajint precision;

    (void) acdAttrToFloat (thys, "minimum", -FLT_MAX, &fmin);
    acdLog ("minimum: %e\n", fmin);

    (void) acdAttrToFloat (thys, "maximum", FLT_MAX, &fmax);
    acdLog ("maximum: %e\n", fmax);

    (void) acdAttrToInt (thys, "precision", 3, &precision);
    acdLog ("precision: %d\n", precision);

    (void) acdAttrToBool (thys, "warnrange", ajTrue, &warnrange);
    acdLog ("warnrange: %B\n", warnrange);

    AJNEW0(val);			/* create storage for the result */

    *val = 0.0;				/* set the default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "0.0", &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	ok = ajStrToFloat(reply, val);
	if (!ok)
	    acdBadVal (thys, required,
		       "Invalid decimal value '%S', please try again",
		       reply);
    }
    if (!ok)
	acdBadRetry (thys);

    if (*val < fmin)
    {					/* reset within limits */
	if (warnrange)
	    ajWarn("floating point value out of range %.*f less than %.*f\n",
		   precision, *val, precision, fmin);
	*val = fmin;
    }

    if (*val > fmax)
    {
	if (warnrange)
	    ajWarn("floating point value out of range %.*f more than %.*f\n",
		   precision, *val, precision, fmax);
	*val = fmax;
    }

    thys->Value = val;
    (void) ajStrFromFloat(&thys->ValStr, *val, precision);

    return;
}

/* @func ajAcdGetGraph ******************************************************
**
** Returns a graph object which hold user graphics options.
**
** @param [r] token [char*] Text token name
** @return [AjPGraph] Graph object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPGraph ajAcdGetGraph (char *token)
{
    return acdGetValue (token, "graph");
}

/* @funcstatic acdSetGraph **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD graph item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetGraph (AcdPAcd thys)
{
    AjPGraph val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    static AjPStr title = NULL;
    AjPStr name = ajStrNewC("GRAPHICS");
    static AjPStr gdev = NULL;

    /*   ajint multi;
	 
	 (void) acdAttrToInt (thys, "multi", 1, &multi);
	 if (multi < 1) multi = 1;
	 acdLog ("multi: %d\n", multi);
	 */

    required = acdIsRequired(thys);

    if(ajNamGetValueC(ajStrStr(name),&gdev))
	(void) acdReplyInit (thys, ajStrStr(gdev), &defreply);
    else
	(void) acdReplyInit (thys, "x11", &defreply);

    acdPromptGraph (thys);

    val = call("ajGraphNew");
  
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	(void) call("ajGraphSet",val, reply,&ok);
	if (!ok)
	{
	    acdBadVal (thys, required,
		       "Invalid graph value '%S'", reply);
	    (void) call("ajGraphDumpDevices");
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssC (&thys->ValStr, "graph definition");

    if (!acdGetValueAssoc (thys, "gtitle", &title))
	(void) acdAttrResolve (thys, "gtitle", &title);
    (void) call("ajGraphxyTitle",val,title);

    if (!acdGetValueAssoc (thys, "gsubtitle", &title))
	(void) acdAttrResolve (thys, "gsubtitle", &title);
    (void) call("ajGraphxySubtitle",val,title);

    if (!acdGetValueAssoc (thys, "gxtitle", &title))
	(void) acdAttrResolve (thys, "gxtitle", &title);
    (void) call("ajGraphxyXtitle",val,title);

    if (!acdGetValueAssoc (thys, "gytitle", &title))
	(void) acdAttrResolve (thys, "gytitle", &title);
    (void) call("ajGraphxyYtitle",val,title);

    if (!acdGetValueAssoc (thys, "goutfile", &title))
	(void) acdAttrResolve (thys, "goutfile", &title);
    (void) call("ajGraphxySetOutputFile",val,title);

    (void) call("ajGraphTrace",val);

    ajStrDel(&name);
  
    return;
}

/* @func ajAcdGetGraphxy ******************************************************
**
** Returns a graph object which hold user graphics options.
**
** @param [r] token [char*] Text token name
** @return [AjPGraph] Graph object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPGraph ajAcdGetGraphxy (char *token)
{
    return acdGetValue (token, "xygraph");
}

/* @funcstatic acdSetGraphxy **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD XY graph item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetGraphxy (AcdPAcd thys)
{
    AjPGraph val;
    AjPStr name = ajStrNewC("GRAPHICS");
    static AjPStr gdev = NULL;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    ajint multi;
    static AjPStr title = NULL;

    (void) acdAttrToInt (thys, "multi", 1, &multi);
    if (multi < 1) multi = 1;
    acdLog ("multi: %d\n", multi);

    required = acdIsRequired(thys);
    /*
       (void) acdReplyInit (thys, "xwindows", &defreply);
       if(ajNamGetValueC(ajStrStr(name),&value)){
       (void) ajStrAssS(&defreply,value);
       ajStrDel(&value);
       }
       */

    if(ajNamGetValueC(ajStrStr(name),&gdev))
	(void) acdReplyInit (thys, ajStrStr(gdev), &defreply);
    else
	(void) acdReplyInit (thys, "x11", &defreply);

    acdPromptGraph (thys);

    val = call("ajGraphxyNewI",multi);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	(void) call("ajGraphxySet",val, reply, &ok);
	if (!ok){
	    acdBadVal (thys, required,
		       "Invalid XY graph value '%S'", reply);
	    (void) call("ajGraphDumpDevices");
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssC (&thys->ValStr, "XY graph definition");

    if (!acdGetValueAssoc (thys, "gtitle", &title))
	(void) acdAttrResolve (thys, "gtitle", &title);
    (void) call("ajGraphxyTitle",val,title);

    if (!acdGetValueAssoc (thys, "gsubtitle", &title))
	(void) acdAttrResolve (thys, "gsubtitle", &title);
    (void) call("ajGraphxySubtitle",val,title);

    if (!acdGetValueAssoc (thys, "gxtitle", &title))
	(void) acdAttrResolve (thys, "gxtitle", &title);
    (void) call("ajGraphxyXtitle",val,title);

    if (!acdGetValueAssoc (thys, "gytitle", &title))
	(void) acdAttrResolve (thys, "gytitle", &title);
    (void) call("ajGraphxyYtitle",val,title);

    if (!acdGetValueAssoc (thys, "goutfile", &title))
	(void) acdAttrResolve (thys, "goutfile", &title);
    (void) call("ajGraphxySetOutputFile",val,title);

    (void) call("ajGraphTrace",val);

    ajStrDel(&name);
    return;
}

/* @func ajAcdGetInt **********************************************************
**
** Returns an item of type ajint as defined in a named ACD item. Called by the
** application after all ACD values have been set, and simply returns
** what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [ajint] Integer value from ACD item
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

ajint ajAcdGetInt (char *token)
{
    ajint* val;

    val = acdGetValue (token, "integer");
    return *val;
}

/* @funcstatic acdSetInt ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD integer item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is "0".
**
** Min and max limits, if set, are applied without comment.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajStrToInt
** @@
******************************************************************************/

static void acdSetInt (AcdPAcd thys)
{
    ajint* val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool warnrange;

    ajint imin;
    ajint imax;

    (void) acdAttrToInt (thys, "minimum", INT_MIN, &imin);
    acdLog ("minimum: %d\n", imin);

    (void) acdAttrToInt (thys, "maximum", INT_MAX, &imax);
    acdLog ("maximum: %d\n", imax);

    (void) acdAttrToBool (thys, "warnrange", ajTrue, &warnrange);
    acdLog ("warnrange: %B\n", warnrange);

    AJNEW0(val);			/* create storage for the result */

    *val = 0;				/* set the default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "0", &defreply);

    acdLog ("acdSetInt %S default '%S' Required: %B\n",
	    thys->Name, defreply, required);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	acdLog (" reply: '%S' \n", reply);
	if (ajStrMatchC(reply, "default"))
	    (void) ajStrAssC(&reply, "0");
	ok = ajStrToInt(reply, val);
	acdLog (" modified reply: '%S' val: %d ok: %B\n", reply, *val, ok);
	if (!ok)
	    acdBadVal (thys, required,
		       "Invalid integer value '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    if (*val < imin)
    {					/* reset within limits */
	if (warnrange)
	    ajWarn("integer value out of range %d less than %d\n", *val, imin);
	*val = imin;
    }
    if (*val > imax)
    {
	if (warnrange)
	    ajWarn("integer value out of range %d more than %d\n", *val, imax);
	*val = imax;
    }

    thys->Value = val;
    (void) ajStrFromInt (&thys->ValStr, *val);

    return;
}

/* @func ajAcdGetInfile *******************************************************
**
** Returns an item of type Outfile as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPFile] File object. The file was already opened by
**         acdSetOutfile so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPFile ajAcdGetInfile (char *token)
{
    return acdGetValue (token, "infile");
}

/* @funcstatic acdSetInfile ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD infile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if filtering is on) is "stdin", but then
** prompting is turned off.
**
** Otherwise there is no default value unless the ACD file has one.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewIn
** @@
******************************************************************************/

static void acdSetInfile (AcdPAcd thys)
{
    AjPFile val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool nullok;

    static AjPStr infname = NULL;

    val = NULL;				/* set the default value */

    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);

    required = acdIsRequired(thys);
    (void) acdInFilename (&infname);
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
    acdPromptInfile (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    val = ajFileNewIn(reply);
	    if (!val)
	    {
		acdBadVal (thys, required,
			   "Unable to open file '%S' for input", reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    if (!nullok)
	    {
		acdBadVal (thys, required, "Input file is required");
		ok = ajFalse;
	    }
	}
    }
    if (!ok)
	acdBadRetry (thys);

    (void) acdInFileSave(reply);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetList *******************************************************
**
** Returns an item of type List as defined in a named ACD item,
** which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPStr*] String array of values with NULL for last element.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr* ajAcdGetList (char *token)
{
    return acdGetValue (token, "list");
}

/* @func ajAcdGetListI *******************************************************
**
** Returns one item from an array of type List as defined in a named
** ACD item, which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set, and
** simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @param [r] num [ajint] Token number (1 for the first)
** @return [AjPStr] String array of values with NULL for last element.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr ajAcdGetListI (char *token, ajint num)
{
    AjPStr* val;
    ajint i;

    val = acdGetValue (token, "list");
    for (i=1; i<num; i++)
	if (!val[i])
	    ajWarn ("value %d not found for %s, last value was %d\n",
		    num, token, i-1);

    return val[num-1];
}

/* @funcstatic acdSetList *****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** If a value is required and not yet given, prints out a header
** and the list of options, then asks for a selection or (if max is
** more than 1) a list of selections.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetList (AcdPAcd thys)
{
    AjPStr* val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    ajint i;

    ajint min, max;

    val = NULL;				/* set the default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "", &defreply);

    (void) acdAttrToInt (thys, "minimum", 1, &min);
    (void) acdAttrToInt (thys, "maximum", 1, &max);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	{
	    acdListPrompt(thys);
	    (void) acdUserGet (thys, &reply);
	}

	val = acdListValue(thys, min, max, reply);
	if (!val)
	{
	    acdBadVal (thys, required, "Bad list option '%S'", reply);
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    for (i=0; val[i]; i++)
    {
	ajDebug("Storing val[%d] '%S'\n", i,val[i]);
	if (i)
	    (void) ajStrAppC (&thys->ValStr, ";");
	(void) ajStrApp (&thys->ValStr, val[i]);
    }

    return;
}

/* @func ajAcdGetMatrix *******************************************************
**
** Returns an item of type Matrix as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPMatrix] Matrix object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPMatrix ajAcdGetMatrix (char *token)
{
    return acdGetValue (token, "matrix");
}

/* @func ajAcdGetMatrixf ******************************************************
**
** Returns an item of type Matrix as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPMatrixf] Float Matrix object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPMatrixf ajAcdGetMatrixf (char *token)
{
    return acdGetValue (token, "matrixf");
}

/* @funcstatic acdSetMatrix ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD matrix item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajMatrixRead
** @@
******************************************************************************/

static void acdSetMatrix (AcdPAcd thys)
{
    AjPMatrix val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool isprot;

    static AjPStr infname = NULL;

    val = NULL;				/* set the default value */

    required = acdIsRequired(thys);
    (void) acdAttrToBool (thys, "protein", ajTrue, &isprot);
    if (isprot)
    {
	(void) acdAttrResolve (thys, "pname", &infname);
	if (!ajStrLen(infname))
	    (void) ajStrAssC(&infname, DEFBLOSUM);
    }
    else
    {
	(void) acdAttrResolve (thys, "nname", &infname);
	if (!ajStrLen(infname))
	    (void) ajStrAssC(&infname, DEFDNA);
    }
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAss (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    if (!ajMatrixRead(&val, reply))
	    {
		acdBadVal (thys, required,
			   "Unable to read matrix '%S'", reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    acdBadVal (thys, required, "Matrix is required");
	    ok = ajFalse;
	}
    }

    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAss (&thys->ValStr, reply);

    return;
}

/* @funcstatic acdSetMatrixf **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD matrix item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajMatrixfRead
** @@
******************************************************************************/

static void acdSetMatrixf (AcdPAcd thys)
{
    AjPMatrixf val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool isprot;

    static AjPStr infname = NULL;

    val = NULL;				/* set the default value */

    required = acdIsRequired(thys);
    (void) acdAttrToBool (thys, "protein", ajTrue, &isprot);
    if (isprot)
    {
	(void) acdAttrResolve (thys, "pname", &infname);
	if (!ajStrLen(infname))
	    (void) ajStrAssC(&infname, DEFBLOSUM);
    }
    else
    {
	(void) acdAttrResolve (thys, "nname", &infname);
	if (!ajStrLen(infname))
	    (void) ajStrAssC(&infname, DEFDNA);
    }
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {

	ok = ajTrue;	/* accept the default if nothing changes */

	(void) ajStrAss (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    if (!ajMatrixfRead(&val, reply))
	    {
		acdBadVal (thys, required,
			   "Unable to read matrix '%S'", reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    acdBadVal (thys, required, "Matrix is required");
	    ok = ajFalse;
	}
    }

    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetOutfile ******************************************************
**
** Returns an item of type Outfile as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPFile] File object. The file was already opened by
**         acdSetOutfile so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPFile ajAcdGetOutfile (char *token)
{
    return acdGetValue (token, "outfile");
}

/* @funcstatic acdSetOutfile **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value, if stdout or filtering is on is "stdout" for the
** first file.
**
** Otherwise an output file name is constructed.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetOutfile (AcdPAcd thys)
{
    AjPFile val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool nullok;

    static AjPStr name = NULL;
    static AjPStr ext = NULL;
    static AjPStr outfname = NULL;

    val = NULL;				/* set the default value */

    (void) acdAttrResolve (thys, "name", &name);
    (void) acdAttrResolve (thys, "extension", &ext);

    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);

    required = acdIsRequired(thys);
    (void) acdOutFilename (&outfname, name, ext);
    (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    acdPromptOutfile (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;			/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    val = ajFileNewOut(reply);
	    if (!val)
	    {
		acdBadVal (thys, required,
			   "Unable to open file '%S' for output", reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    if (!nullok)
	    {
		acdBadVal (thys, required, "Output file is required");
		ok = ajFalse;
	    }
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetCpdb *******************************************************
**
** Returns an item of type Pdb as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPPdb] Cpdb object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPPdb ajAcdGetCpdb (char *token)
{
    return acdGetValue (token, "cpdb");
}


/* @funcstatic acdSetCpdb ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if filtering is on) is "stdin", but then
** prompting is turned off.
**
** Otherwise there is no default value unles the ACD file has one.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetCpdb (AcdPAcd thys)
{
    AjPPdb val;
    AjPFile inf=NULL;
    AjPStr name=NULL;
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;


    (void) acdAttrResolve (thys, "name", &name);
    if(!ajStrLen(name))
	(void) ajStrAssC(&name,DEFCPDB);
  
    required = acdIsRequired(thys);

    (void) acdReplyInit (thys, ajStrStr(name), &defreply);
    acdPromptCpdb(thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {

	ok = ajTrue;	/* accept the default if nothing changes */

	(void) ajStrAss (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    if((inf=ajFileNewIn(reply)))
	    {
		if (ajXyzCpdbRead(inf,&val))
		    ajFileClose(&inf);
		else
		{
		    acdBadVal (thys, required,
			       "Unable to read clean PDB file '%S'", reply);
		    ok = ajFalse;
		}
	    }
	    
	}
	else
	{
	    acdBadVal (thys, required, "Cleaned PDB file is required");
	    ok = ajFalse;
	}
    }

    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    ajStrDel(&name);
    return;
}

/* @func ajAcdGetScop *******************************************************
**
** Returns an item of type Scop as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPScop] Scop object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPScop ajAcdGetScop (char *token)
{
    return acdGetValue (token, "scop");
}


/* @funcstatic acdSetScop ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if filtering is on) is "stdin", but then
** prompting is turned off.
**
** Otherwise there is no default value unles the ACD file has one.
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetScop (AcdPAcd thys)
{
    AjPScop val;

    AjPFile inf=NULL;
    AjPStr name=NULL;
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;


    (void) acdAttrResolve (thys, "name", &name);
    if(!ajStrLen(name))
	(void) ajStrAssC(&name,DEFSCOP);
  
    required = acdIsRequired(thys);

    (void) acdReplyInit (thys, ajStrStr(name), &defreply);
    acdPromptScop(thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {

	ok = ajTrue;	/* accept the default if nothing changes */

	(void) ajStrAss (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    ajFileDataNewC(ajESCOP,&inf);
	    if(!inf)
		ok = ajFalse;
	    else if (!ajXyzScopRead(inf,reply,&val))
	    {
		acdBadVal (thys, required,
			   "Unable to read clean PDB file '%S'", reply);
		ok = ajFalse;
		ajFileClose(&inf);
	    }
	}
	else
	{
	    acdBadVal (thys, required, "Scop entry is required");
	    ok = ajFalse;
	}
    }

    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    ajStrDel(&name);
    return;
}

/* @func ajAcdGetRange *******************************************************
**
** Returns an item of type Range as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPRange] Range object.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPRange ajAcdGetRange (char *token)
{
    return acdGetValue (token, "range");
}


/* @funcstatic acdSetRange ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD outfile item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if filtering is on) is "stdin", but then
** prompting is turned off.
**
** The default value (if no other is available) is an empty string
** (ie. no range).
**
** Various file naming options are defined, but not yet implemented here.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetRange (AcdPAcd thys)
{
    AjPRange val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "", &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	/* accept the default if nothing changes */

	(void) ajStrAss (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (!ajRangeGet(&val,reply))
	{
	    acdBadVal (thys, required,
		       "Bad range specification '%S'", reply);
	    ok = ajFalse;
	}
    }

    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

  
    return;
}


/* @func ajAcdGetRegexp ******************************************************
**
** Returns an item of type Regexp as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPRegexp] Compiled regular expression.
**                     The original pattern string is available
**                     through a call to ajAcdGetValue
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPRegexp ajAcdGetRegexp (char *token)
{
    return acdGetValue (token, "regexp");
}

/* @funcstatic acdSetRegexp ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD regular expression item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is NULL.
**
** Attributes for minimum and maximum length are applied with error
** messages if exceeded.
** 
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @@
******************************************************************************/

static void acdSetRegexp (AcdPAcd thys) {

  AjPRegexp val = NULL;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  AjBool upper;
  AjBool lower;
  ajint itry;

  static AcdOAttr setattr[] = {
    {"length", VT_INT},
    {NULL, VT_NULL} };

  ajint minlen;
  ajint maxlen;
  ajint len;

  (void) acdAttrToInt (thys, "minlength", 1, &minlen);

  (void) acdAttrToInt (thys, "maxlength", INT_MAX, &maxlen);
  (void) acdAttrToBool (thys, "upper", ajFalse, &upper);
  (void) acdAttrToBool (thys, "lower", ajFalse, &lower);
 
  required = acdIsRequired(thys);
  (void) acdReplyInit (thys, "", &defreply);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */
    if (val)
      ajRegFree (&val);

    (void) ajStrAssS (&reply, defreply);

    if (required)
      (void) acdUserGet (thys, &reply);

    len = ajStrLen(reply);

    if (len < minlen) {
      acdBadVal (thys, required,
		 "Too short - minimum length is %d characters",
		 minlen);
      ok = ajFalse;
    }
    if (len > maxlen) {
      acdBadVal (thys, required,
		 "Too long - maximum length is %d characters",
		 maxlen);
      ok = ajFalse;
    }
    if (upper)
      ajStrToUpper (&reply);

    if (lower)
      ajStrToLower (&reply);
    if (ok)
      val = ajRegComp (reply);
    if (ok && !val) {
      acdBadVal (thys, required,
		 "Bad regular expression pattern:\n   '%S'",
		 reply);
      ok = ajFalse;
    }

  }
  if (!ok)
    acdBadRetry (thys);

  /* regexps have special set attributes the same as strings */

  thys->SAttr = acdAttrListCount (setattr);
  thys->SetAttr = &setattr[0];
  thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

  (void) ajStrFromInt (&thys->SetStr[0], ajStrLen(reply));

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);

  return;
}

/* @func ajAcdGetReport ******************************************************
**
** Returns an item of type Report as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPReport] Report output object. Already opened
**                      by acdSetFeatout so this just returns the object
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPReport ajAcdGetReport (char *token)
{
    return acdGetValue (token, "report");
}

/* @funcstatic acdSetReport **************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD report item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-rformat", "-ropenfile"
** are applied to the UFO before reading the sequence.
**
** Associated qualifiers "-rbegin", "-rend" and "-rreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the feature table,
** and the resulting table is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/

static void acdSetReport (AcdPAcd thys)
{
    AjPReport val = NULL;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;

    static AjPStr name = NULL;
    static AjPStr ext = NULL;
    static AjPStr outfname = NULL;

    static AcdOAttr setattr[] =
    {
	{"begin", VT_INT},
	{"end", VT_INT},
	{"length", VT_INT},
	{"protein", VT_BOOL},
	{"nucleic", VT_BOOL},
	{"name", VT_STR},
	{NULL, VT_NULL} };

    required = acdIsRequired(thys);
    val = ajReportNew();

    acdAttrResolve (thys, "name", &name);
    if (!acdGetValueAssoc (thys, "rformat", &val->Formatstr))
	(void) acdAttrResolve (thys, "rextension", &ext);

    (void) acdOutFilename (&outfname, name, val->Formatstr);
    (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    acdPromptReport (thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	(void) acdGetValueAssoc (thys, "ropenfile", &val->Filename);
	ok = ajReportOpen (val, reply);
	if (!ok)
	    acdBadVal (thys, required,
		       "Unable to read sequence '%S'", reply);
    }
    if (!ok)
	acdBadRetry (thys);

    /* reports have special set attributes */

    thys->SAttr = acdAttrListCount (setattr);
    thys->SetAttr = &setattr[0];
    thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

    /*
       (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_BEGIN], ajSeqBegin(val));
       (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_END], ajSeqEnd(val));
       (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_LENGTH], ajSeqLen(val));
       (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqIsProt(val));
       (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqIsNuc(val));
       (void) ajStrAssS (&thys->SetStr[ACD_SEQ_NAME], val->Name);
       */

    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);

    return;
}

/* @func ajAcdGetSelect *******************************************************
**
** Returns an item of type Select as defined in a named ACD item,
** which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPStr*] String array of values with NULL as last element.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr* ajAcdGetSelect (char *token) {

  return acdGetValue (token, "select");
}

/* @func ajAcdGetSelectI ******************************************************
**
** Returns one item from an array of type Select as defined in a named
** ACD item, which is an array of strings terminated by a null value.
** Called by the application after all ACD values have been set, and
** simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @param [r] num [ajint] Token number (1 for the first)
** @return [AjPStr] String array of values with NULL as last element.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr ajAcdGetSelectI (char *token, ajint num) {

  AjPStr* val;
  ajint i;

  val =  acdGetValue (token, "select");

  for (i=1; i<num; i++) {
    if (!val[i]) {
      ajWarn ("value %d not found for %s, last value was %d\n",
	      num, token, i-1);
      return NULL;
    }
  }
  return val[num-1];
}

/* @funcstatic acdSetSelect ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD select menu item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetSelect (AcdPAcd thys) {

  AjPStr* val;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  ajint itry;
  ajint i;

  ajint min=0, max=5;

  val = NULL;		/* set the default value */

  required = acdIsRequired(thys);
  (void) acdReplyInit (thys, "", &defreply);

  (void) acdAttrToInt (thys, "minimum", 1, &min);
  (void) acdAttrToInt (thys, "maximum", 1, &max);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required) {
      acdSelectPrompt (thys);
      (void) acdUserGet (thys, &reply);
    }

    /*
    AJCNEW0(val, 2);
    (void) ajStrAssS (&val[0], reply);
    */
    val = acdSelectValue(thys, min, max, reply);
    if (!val) {
      acdBadVal (thys, required, "Bad select option '%S'", reply);
      ok = ajFalse;
    }
  }
  if (!ok)
    acdBadRetry (thys);

  thys->Value = val;
  for (i=0; val[i]; i++) {
    if (i)
      (void) ajStrAppC (&thys->ValStr, ";");
    (void) ajStrApp (&thys->ValStr, val[i]);
  }

  return;
}

/* @func ajAcdGetSeq **********************************************************
**
** Returns an item of type Seq as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPSeq] Sequence object. The sequence was already loaded by
**         acdSetSeq so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPSeq ajAcdGetSeq (char *token) {

  return acdGetValue (token, "sequence");
}

/* @funcstatic acdSetSeq ******************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifiers "-supper", "-slower" and "-sask" are applied
** after reading.
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/

static void acdSetSeq (AcdPAcd thys) {

  AjPSeq val;
  AjPSeqin seqin;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  static AjPStr promptreply = NULL;
  ajint itry;
  ajint i;

  static AjPStr infname = NULL;

  static AcdOAttr setattr[] = {
    {"begin", VT_INT},
    {"end", VT_INT},
    {"length", VT_INT},
    {"protein", VT_BOOL},
    {"nucleic", VT_BOOL},
    {"name", VT_STR},
    {NULL, VT_NULL} };

  ajint sbegin=0;
  ajint send=0;
  AjBool sreverse=ajFalse;
  AjBool sprompt=ajFalse;
  AjBool snuc=ajFalse;
  AjBool sprot=ajFalse;
  AjBool slower=ajFalse;
  AjBool supper=ajFalse;
  
  val = ajSeqNew();		/* set the default value */
  seqin = ajSeqinNew();		/* set the default value */

  /* seqin->multi = ajFalse; */ /* pmr: moved to ajSeqinNew */
  
  (void) acdQualToBool (thys, "snucleotide", ajFalse, &snuc, &defreply);
  (void) acdQualToBool (thys, "sprotein", ajFalse, &sprot, &defreply);

  if (snuc)
    ajSeqinSetNuc (seqin);

  if (sprot)
    ajSeqinSetProt (seqin);

  required = acdIsRequired(thys);
  (void) acdInFilename (&infname);
  (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
  acdPromptSeq (thys);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
     (void) acdUserGet (thys, &reply);

    ajSeqinUsa (&seqin, reply);

    (void) acdGetValueAssoc (thys, "sformat", &seqin->Formatstr);
    (void) acdGetValueAssoc (thys, "sdbname", &seqin->Db);
    (void) acdGetValueAssoc (thys, "sopenfile", &seqin->Filename);
    (void) acdGetValueAssoc (thys, "sid", &seqin->Entryname);

    (void) acdGetValueAssoc (thys, "ufo", &seqin->Ufo);

    (void) acdGetValueAssoc (thys, "fformat", &seqin->Ftquery->Formatstr);
    (void) acdGetValueAssoc (thys, "fopenfile", &seqin->Ftquery->Filename);

    (void) acdAttrToStr(thys, "type", "", &seqin->Inputtype);
    (void) acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
    (void) acdAttrToBool(thys, "entry", ajFalse, &seqin->Text);

    i = ajStrLen(seqin->Ufo) + ajStrLen(seqin->Ftquery->Formatstr)
      + ajStrLen(seqin->Ftquery->Filename);

    if (i && !seqin->Features)
      ajWarn ("Feature table ignored");

    if (seqin->Features)
      ajDebug ("acdSetSeq with features UFO '%S'\n", seqin->Ufo);

    (void) ajStrStat ("before ajSeqRead");
    ok = ajSeqRead(val, seqin);
    (void) ajStrStat ("after ajSeqRead");
    if (!ok) {
      acdBadVal (thys, required,
		 "Unable to read sequence '%S'", reply);
    }
  }
  
  if (!ok)
    acdBadRetry (thys);

  (void) acdInFileSave(ajSeqGetName(val)); /* save the sequence name */

  /* some standard options using associated qualifiers */

  (void) acdQualToBool (thys, "supper", ajFalse, &supper, &defreply);
  (void) acdQualToBool (thys, "slower", ajFalse, &slower, &defreply);
  (void) acdQualToBool (thys, "sask", ajFalse, &sprompt, &defreply);
  
  /* now process the begin, end and reverse options */

  ok = acdQualToSeqbegin (thys, "sbegin", 0, &sbegin, &defreply);
  for (itry=acdPromptTry; itry && !ok; itry--) {
    (void) ajStrAssS (&promptreply, defreply);
    if (sprompt)
      (void) acdUserGetPrompt (" Begin at position", &promptreply);
    if (ajStrMatchCaseC(promptreply, "start"))
      (void) ajStrAssC(&promptreply, "0");
    ok = ajStrToInt(promptreply, &sbegin);
    if (!ok)
      acdBadVal (thys, sprompt,
		 "Invalid sequence position '%S'", promptreply);
  }
  if (!ok)
    acdBadRetry (thys);
  (void) acdSetQualDefInt(thys, "sbegin", sbegin);

  ok = acdQualToSeqend (thys, "send", 0, &send, &defreply);
  for (itry=acdPromptTry; itry && !ok; itry--) {
    (void) ajStrAssS (&promptreply, defreply);
    if (sprompt)
      (void) acdUserGetPrompt ("   End at position", &promptreply);
    if (ajStrMatchCaseC(promptreply, "end"))
      (void) ajStrAssC(&promptreply, "0");
    ok = ajStrToInt(promptreply, &send);
    if (!ok)
      acdBadVal (thys, sprompt,
		 "Invalid sequence position '%S'", promptreply);
  }
  if (!ok)
    acdBadRetry (thys);
  (void) acdSetQualDefInt(thys, "send", send);

  if (ajSeqIsNuc(val)) {
    ok = acdQualToBool (thys, "sreverse", ajFalse, &sreverse, &defreply);
    for (itry=acdPromptTry; itry && !ok; itry--) {
      (void) ajStrAssS (&promptreply, defreply);
      if (sprompt)
	(void) acdUserGetPrompt ("    Reverse strand", &promptreply);
      ok = ajStrToBool(promptreply, &sreverse);
      if (!ok)
	acdBadVal (thys, sprompt,
		   "Invalid Y/N value '%S'", promptreply);
    }
    if (!ok)
      acdBadRetry (thys);
    (void) acdSetQualDefBool(thys, "sreverse", sreverse);
  }

  acdLog ("sbegin: %d, send: %d, sreverse: %s\n",
		 sbegin, send, ajStrBool(sreverse));
  
  ajSeqSetRange(val, sbegin, send);
  ajSeqinSetRange(seqin, sbegin, send);

  if (slower)
    (void) ajStrToLower (&val->Seq);
  if (supper)
    (void) ajStrToUpper (&val->Seq);
  if (sreverse)
    ajSeqReverse (val);

  ajSeqinDel (&seqin);

  /* sequences have special set attributes */

  thys->SAttr = acdAttrListCount (setattr);
  thys->SetAttr = &setattr[0];
  thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_BEGIN], ajSeqBegin(val));
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_END], ajSeqEnd(val));
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_LENGTH], ajSeqLen(val));
  (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqIsProt(val));
  (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqIsNuc(val));
  (void) ajStrAssS (&thys->SetStr[ACD_SEQ_NAME], val->Name);

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);

  return;
}

/* @func ajAcdGetSeqset *******************************************************
**
** Returns an item of type Seqset as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPSeqset] Sequence set object. The sequence was already loaded by
**         acdSetSeqset so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPSeqset ajAcdGetSeqset (char *token) {

  return acdGetValue (token, "seqset");
}

/* @funcstatic acdSetSeqset ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifiers "-supper", "-slower" and "-sask" are applied
** after reading.
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/


static void acdSetSeqset (AcdPAcd thys) {

  AjPSeqset val;
  AjPSeqin seqin;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  static AjPStr promptreply = NULL;
  ajint itry;

  static AjPStr infname = NULL;

  static AcdOAttr setattr[] = {
    {"begin", VT_INT},
    {"end", VT_INT},
    {"length", VT_INT},
    {"protein", VT_BOOL},
    {"nucleic", VT_BOOL},
    {"name", VT_STR},
    {"totweight", VT_FLOAT},
    {"count", VT_INT},
    {NULL, VT_NULL} };

  ajint sbegin=0;
  ajint send=0;
  AjBool sreverse=ajFalse;
  AjBool sprompt=ajFalse;
  AjBool snuc=ajFalse;
  AjBool sprot=ajFalse;
  AjBool slower=ajFalse;
  AjBool supper=ajFalse;

  val = ajSeqsetNew();		/* set the default value */
  seqin = ajSeqinNew();		/* set the default value */

  seqin->multi = ajTrue;  /* pmr: moved to ajSeqinNew */ /* ajb added back */

  (void) acdQualToBool (thys, "snucleotide", ajFalse, &snuc, &defreply);
  (void) acdQualToBool (thys, "sprotein", ajFalse, &sprot, &defreply);

  if (snuc)
    ajSeqinSetNuc (seqin);

  if (sprot)
    ajSeqinSetProt (seqin);

  required = acdIsRequired(thys);
  (void) acdInFilename (&infname);
  (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
  acdPromptSeq (thys);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
     (void) acdUserGet (thys, &reply);

    ajSeqinUsa (&seqin, reply);
    (void) acdGetValueAssoc (thys, "sformat", &seqin->Formatstr);
    (void) acdGetValueAssoc (thys, "sdbname", &seqin->Db);
    (void) acdGetValueAssoc (thys, "sopenfile", &seqin->Filename);
    (void) acdGetValueAssoc (thys, "sid", &seqin->Entryname);
    (void) acdGetValueAssoc (thys, "ufo", &seqin->Ufo);

    (void) acdGetValueAssoc (thys, "fformat", &seqin->Ftquery->Formatstr);
    (void) acdGetValueAssoc (thys, "fopenfile", &seqin->Ftquery->Filename);

    (void) acdAttrToStr(thys, "type", "", &seqin->Inputtype);
    (void) acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
    if (ajStrLen(seqin->Ufo))
      seqin->Features = ajTrue;

    ok = ajSeqsetRead(val, seqin);
    if (!ok) {
      acdBadVal (thys, required,
		 "Unable to read sequence '%S'", reply);
    }
  }
  if (!ok)
    acdBadRetry (thys);

  ajSeqinDel (&seqin);

  (void) acdInFileSave(ajSeqsetGetName(val)); /* save the sequence name */

  (void) acdQualToBool (thys, "supper", ajFalse, &supper, &defreply);
  (void) acdQualToBool (thys, "slower", ajFalse, &slower, &defreply);
  (void) acdQualToBool (thys, "sask", ajFalse, &sprompt, &defreply);

  /* now process the begin, end and reverse options */

  ok = acdQualToSeqbegin (thys, "sbegin", 0, &sbegin, &defreply);
  for (itry=acdPromptTry; itry && !ok; itry--) {
    (void) ajStrAssS (&promptreply, defreply);
    if (sprompt)
      (void) acdUserGetPrompt (" Begin at position", &promptreply);
    if (ajStrMatchCaseC(promptreply, "start"))
      (void) ajStrAssC(&promptreply, "0");
    ok = ajStrToInt(promptreply, &sbegin);
    if (!ok)
      acdBadVal (thys, sprompt,
		 "Invalid integer value '%S'", promptreply);
  }
  if (!ok)
    acdBadRetry (thys);


  ok = acdQualToSeqend (thys, "send", 0, &send, &defreply);
  for (itry=acdPromptTry; itry && !ok; itry--) {
    (void) ajStrAssS (&promptreply, defreply);
    if (sprompt)
      (void) acdUserGetPrompt ("   End at position", &promptreply);
    if (ajStrMatchCaseC(promptreply, "end"))
      (void) ajStrAssC(&promptreply, "0");
    ok = ajStrToInt(promptreply, &send);
    if (!ok)
      acdBadVal (thys, sprompt,
		 "Invalid integer value '%S'", promptreply);
  }
  if (!ok)
    acdBadRetry (thys);

  if (ajSeqsetIsNuc(val)) {
    ok = acdQualToBool (thys, "sreverse", ajFalse, &sreverse, &defreply);
    for (itry=acdPromptTry; itry && !ok; itry--) {
      (void) ajStrAssS (&promptreply, defreply);
      if (sprompt)
	(void) acdUserGetPrompt ("    Reverse strand", &promptreply);
      ok = ajStrToBool(promptreply, &sreverse);
      if (!ok)
	acdBadVal (thys, sprompt,
		   "Invalid Y/N value '%S'", promptreply);
    }
    if (!ok)
      acdBadRetry (thys);
  }

  acdLog ("sbegin: %d, send: %d, sreverse: %s\n",
		 sbegin, send, ajStrBool(sreverse));

  ajSeqsetSetRange (val, sbegin, send);
  if (slower)
    ajSeqsetToLower (val);
  if (supper)
    ajSeqsetToUpper (val);
  if (sreverse)
    ajSeqsetReverse (val);

  /* sequences have special set attributes */

  thys->SAttr = acdAttrListCount (setattr);
  thys->SetAttr = &setattr[0];
  thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_BEGIN], ajSeqsetBegin(val));
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_END], ajSeqsetEnd(val));
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_LENGTH], ajSeqsetLen(val));
  (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqsetIsProt(val));
  (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqsetIsNuc(val));
  (void) ajStrAssS (&thys->SetStr[ACD_SEQ_NAME], val->Name);
  (void) ajStrFromFloat (&thys->SetStr[ACD_SEQ_WEIGHT],
			 ajSeqsetTotweight(val), 3);
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_COUNT], ajSeqsetSize(val));

  (void) acdInFileSave(ajSeqsetGetName(val));

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);

  return;
}

/* @func ajAcdGetSeqall *******************************************************
**
** Returns an item of type Seq as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPSeqall] Sequence stream object. The sequence was already
**         loaded by acdSetSeqall so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPSeqall ajAcdGetSeqall (char *token) {

  return acdGetValue (token, "seqall");
}

/* @funcstatic acdSetSeqall ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-sformat", "-sdbname", "-sopenfile", "-sid"
** are applied to the USA before reading the sequence.
**
** Associated qualifier "-sask" is applied
** after reading.
**
** Associated qualifiers "-supper", "-slower" are applied
** globally
**
** Associated qualifiers "-sbegin", "-send" and "-sreverse"
** are applied as appropriate, with prompting for values,
** after the sequence has been read. They are applied to the sequence,
** and the resulting sequence is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/

static void acdSetSeqall (AcdPAcd thys) {

  AjPSeqall val;
  AjPSeqin seqin;
  AjPSeq seq;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  static AjPStr promptreply = NULL;
  ajint itry;

  static AjPStr infname = NULL;

  static AcdOAttr setattr[] = {
    {"begin", VT_INT},
    {"end", VT_INT},
    {"length", VT_INT},
    {"protein", VT_BOOL},
    {"nucleic", VT_BOOL},
    {"name", VT_STR},
    {NULL, VT_NULL} };

  ajint sbegin=0;
  ajint send=0;
  AjBool sreverse=ajFalse;
  AjBool sprompt=ajFalse;
  AjBool snuc=ajFalse;
  AjBool sprot=ajFalse;
  AjBool slower=ajFalse;
  AjBool supper=ajFalse;
  
  val = ajSeqallNew();		/* set the default value */
  seqin = val->Seqin;
  seqin->multi = ajTrue;
  seq = val->Seq;

  (void) acdQualToBool (thys, "snucleotide", ajFalse, &snuc, &defreply);
  (void) acdQualToBool (thys, "sprotein", ajFalse, &sprot, &defreply);
  (void) acdQualToBool (thys, "supper", ajFalse, &supper, &defreply);
  (void) acdQualToBool (thys, "slower", ajFalse, &slower, &defreply);

  if(slower)
      seqin->Lower = ajTrue;
  if(supper)
      seqin->Upper = ajTrue;

  if (snuc)
    ajSeqinSetNuc (seqin);

  if (sprot)
    ajSeqinSetProt (seqin);

  required = acdIsRequired(thys);
  (void) acdInFilename (&infname);
  (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
  acdPromptSeq (thys);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
     (void) acdUserGet (thys, &reply);

    ajSeqinUsa (&seqin, reply);
    (void) acdGetValueAssoc (thys, "sformat", &seqin->Formatstr);
    (void) acdGetValueAssoc (thys, "sdbname", &seqin->Db);
    (void) acdGetValueAssoc (thys, "sopenfile", &seqin->Filename);
    (void) acdGetValueAssoc (thys, "sid", &seqin->Entryname);
    (void) acdGetValueAssoc (thys, "ufo", &seqin->Ufo);

    (void) acdGetValueAssoc (thys, "fformat", &seqin->Ftquery->Formatstr);
    (void) acdGetValueAssoc (thys, "fopenfile", &seqin->Ftquery->Filename);

    (void) acdAttrToStr(thys, "type", "", &seqin->Inputtype);
    (void) acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
    (void) acdAttrToBool(thys, "entry", ajFalse, &seqin->Text);

    if (ajStrLen(seqin->Ufo))
      seqin->Features = ajTrue;

    ok = ajSeqAllRead(seq, seqin);
    if (!ok) {
      acdBadVal (thys, required,
		 "Unable to read sequence '%S'", reply);
    }
  }
  if (!ok)
    acdBadRetry (thys);

/*  ajSeqinDel (&seqin);*/

  (void) acdInFileSave(ajSeqallGetName(val)); /* save the sequence name */

  (void) acdQualToBool (thys, "sask", ajFalse, &sprompt, &defreply);

  /* now process the begin, end and reverse options */

  ok = acdQualToSeqbegin (thys, "sbegin", 0, &sbegin, &defreply);
  for (itry=acdPromptTry; itry && !ok; itry--) {
    (void) ajStrAssS (&promptreply, defreply);
    if (sprompt)
      (void) acdUserGetPrompt (" Begin at position", &promptreply);
    if (ajStrMatchCaseC(promptreply, "start"))
      (void) ajStrAssC(&promptreply, "0");
    ok = ajStrToInt(promptreply, &sbegin);
    if (!ok)
      acdBadVal (thys, sprompt,
		 "Invalid integer value '%S'", promptreply);
  }
  if (!ok)
    acdBadRetry (thys);

  ok = acdQualToSeqend (thys, "send", 0, &send, &defreply);
  for (itry=acdPromptTry; itry && !ok; itry--) {
    (void) ajStrAssS (&promptreply, defreply);
    if (sprompt)
      (void) acdUserGetPrompt ("   End at position", &promptreply);
    if (ajStrMatchCaseC(promptreply, "end"))
      (void) ajStrAssC(&promptreply, "0");
    ok = ajStrToInt(promptreply, &send);
    if (!ok)
      acdBadVal (thys, sprompt,
		 "Invalid integer value '%S'", promptreply);
  }
  if (!ok)
    acdBadRetry (thys);

  if (ajSeqIsNuc(seq)) {
    ok = acdQualToBool (thys, "sreverse", ajFalse, &sreverse, &defreply);
    for (itry=acdPromptTry; itry && !ok; itry--) {
      (void) ajStrAssS (&promptreply, defreply);
      if (sprompt)
	(void) acdUserGetPrompt ("    Reverse strand", &promptreply);
      ok = ajStrToBool(promptreply, &sreverse);
      if (!ok)
	acdBadVal (thys, sprompt,
		   "Invalid Y/N value '%S'", promptreply);
    }
    if (!ok)
      acdBadRetry (thys);
  }

  acdLog ("sbegin: %d, send: %d, sreverse: %s\n",
		 sbegin, send, ajStrBool(sreverse));
  
  ajSeqallSetRange(val, sbegin, send);

  if (sreverse)
    ajSeqallReverse (val);
  

  /* sequences have special set attributes */

  thys->SAttr = acdAttrListCount (setattr);
  thys->SetAttr = &setattr[0];
  thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_BEGIN], ajSeqallBegin(val));
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_END], ajSeqallEnd(val));
  (void) ajStrFromInt (&thys->SetStr[ACD_SEQ_LENGTH], ajSeqLen(seq));
  (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_PROTEIN], ajSeqIsProt(seq));
  (void) ajStrFromBool (&thys->SetStr[ACD_SEQ_NUCLEIC], ajSeqIsNuc(seq));
  (void) ajStrAssS (&thys->SetStr[ACD_SEQ_NAME], seq->Name);

  (void) acdInFileSave (ajSeqallGetNameSeq(val));
  /*
  (void) acdInFileSave (ajSeqallGetName(val));
  */
  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);

  return;
}

/* @func ajAcdGetSeqout *******************************************************
**
** Returns an item of type Seqout as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPSeqout] Sequence output object. The file was already opened by
**         acdSetSeqout so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPSeqout ajAcdGetSeqout (char *token) {

  return acdGetValue (token, "seqout");
}

/* @funcstatic acdSetSeqout ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is "stdout".
**
** Associated qualifier "-osformat"
** is applied to the USA before opening the output file.
**
** Associated qualifiers are defined but not yet implemented.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetSeqout (AcdPAcd thys) {

  AjPSeqout val;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  ajint itry;

  static AjPStr name = NULL;
  static AjPStr ext = NULL;
  static AjPStr fmt = NULL;
  static AjPStr sing = NULL;
  static AjPStr outfname = NULL;
  AjBool osfeat;

  val = ajSeqoutNew();		/* set the default value */

  if (!acdGetValueAssoc(thys, "osname", &name))
    (void) acdAttrResolve (thys, "name", &name);

  if (!acdGetValueAssoc(thys, "osextension", &ext))
    (void) acdAttrResolve (thys, "extension", &ext);

  (void) acdGetValueAssoc(thys, "osformat", &fmt);
  (void) ajStrSet(&ext, fmt);
  if (!ajStrLen(ext))
    (void) ajSeqOutFormatDefault(&ext);

  (void) acdOutFilename (&outfname, name, ext);

  (void) acdGetValueAssoc (thys, "osdbname", &val->Setdb);

  (void) acdAttrToBool(thys, "features", ajFalse, &osfeat);
  val->Features = osfeat;

  ajDebug ("acdSetSeqout features: %B\n", val->Features);
  required = acdIsRequired(thys);
  (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
  acdPromptSeqout (thys);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
      (void) acdUserGet (thys, &reply);

    ajSeqoutUsa (&val, reply);
    val->Features = osfeat;

    (void) ajStrSet(&val->Formatstr, fmt);
    if (!ajStrLen(val->Formatstr))
      (void) ajSeqOutFormatDefault(&val->Formatstr);

    (void) ajStrSet(&val->Extension, ext);
    (void) ajStrSet(&val->Extension, val->Formatstr);

    (void) acdGetValueAssoc (thys, "oufo", &val->Ufo);
    (void) acdGetValueAssoc (thys, "offormat", &val->Ftquery->Formatstr);
    (void) acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename);

    (void) acdQualToBool (thys, "ossingle",
			  ajFalse,
			  &val->Single, &sing);

    if (!ajSeqoutOpen(val)) {
      acdBadVal (thys, required,
		 "Unable to write sequence to '%S'", reply);
      ok = ajFalse;
    }
  }
  if (!ok)
    acdBadRetry (thys);

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);
  ajDebug ("acdSetSeqout features: %B\n", val->Features);
  if (val->Features)
    ajDebug ("acdSetSeqout with features UFO '%S'\n", val->Ufo);


  return;
}

/* @func ajAcdGetSeqoutset ****************************************************
**
** Returns an item of type Seqoutset as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPSeqout] Sequence output object. The file was already 
**        opened by acdSetSeqoutset so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPSeqout ajAcdGetSeqoutset (char *token) {

  return acdGetValue (token, "seqoutset");
}

/* @funcstatic acdSetSeqoutset ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is "stdout".
**
** Associated qualifier "-osformat"
** is applied to the USA before opening the output file.
**
** Associated qualifiers are defined but not yet implemented.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetSeqoutset (AcdPAcd thys) {

  AjPSeqout val;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  ajint itry;

  static AjPStr name = NULL;
  static AjPStr ext = NULL;
  static AjPStr fmt = NULL;
  static AjPStr sing = NULL;
  static AjPStr outfname = NULL;
  AjBool osfeat;

  val = ajSeqoutNew();		/* set the default value */

  if (!acdGetValueAssoc(thys, "osname", &name))
    (void) acdAttrResolve (thys, "name", &name);

  if (!acdGetValueAssoc(thys, "osextension", &ext))
    (void) acdAttrResolve (thys, "extension", &ext);

  (void) acdGetValueAssoc(thys, "osformat", &fmt);
  (void) ajStrSet(&ext, fmt);
  if (!ajStrLen(ext))
    (void) ajSeqOutFormatDefault(&ext);

  (void) acdOutFilename (&outfname, name, ext);

  (void) acdAttrToBool(thys, "features", ajFalse, &osfeat);
  val->Features = osfeat;

  (void) acdGetValueAssoc (thys, "osdbname", &val->Setdb);

  required = acdIsRequired(thys);
  (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
  acdPromptSeqout (thys);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
      (void) acdUserGet (thys, &reply);

    ajSeqoutUsa (&val, reply);
    val->Features = osfeat;

    (void) ajStrSet(&val->Formatstr, fmt);
    if (!ajStrLen(val->Formatstr))
      (void) ajSeqOutFormatDefault(&val->Formatstr);

    (void) ajStrSet(&val->Extension, ext);
    (void) ajStrSet(&val->Extension, val->Formatstr);

    (void) acdGetValueAssoc (thys, "oufo", &val->Ufo);
    (void) acdGetValueAssoc (thys, "offormat", &val->Ftquery->Formatstr);
    (void) acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename);

    (void) acdQualToBool (thys, "ossingle",
			  ajSeqOutFormatSingle(val->Formatstr),
			  &val->Single, &sing);

    if (!ajSeqoutOpen(val)) {
      acdBadVal (thys, required,
		 "Unable to write sequence to '%S'", reply);
      ok = ajFalse;
    }
  }
  if (!ok)
    acdBadRetry (thys);

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);

  return;
}

/* @func ajAcdGetSeqoutall ****************************************************
**
** Returns an item of type Seqoutall as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPSeqout] Sequence output object. The file was already
**         opened by acdSetSeqoutall so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPSeqout ajAcdGetSeqoutall (char *token) {

  return acdGetValue (token, "seqoutall");
}

/* @funcstatic acdSetSeqoutall ************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD sequence output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is "stdout".
**
** Associated qualifier "-osformat"
** is applied to the USA before opening the output file.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetSeqoutall (AcdPAcd thys) {

  AjPSeqout val;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  ajint itry;

  static AjPStr name = NULL;
  static AjPStr ext = NULL;
  static AjPStr fmt = NULL;
  static AjPStr sing = NULL;
  static AjPStr outfname = NULL;
  AjBool osfeat;

  val = ajSeqoutNew();		/* set the default value */

  if (!acdGetValueAssoc(thys, "osname", &name))
    (void) acdAttrResolve (thys, "name", &name);

  if (!acdGetValueAssoc(thys, "osextension", &ext))
    (void) acdAttrResolve (thys, "extension", &ext);

  (void) acdGetValueAssoc(thys, "osformat", &fmt);
  (void) ajStrSet(&ext, fmt);
  if (!ajStrLen(ext))
    (void) ajSeqOutFormatDefault(&ext);

  (void) acdOutFilename (&outfname, name, ext);

  (void) acdGetValueAssoc (thys, "osdbname", &val->Setdb);

  (void) acdAttrToBool(thys, "features", ajFalse, &osfeat);
  val->Features = osfeat;

  required = acdIsRequired(thys);
  (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
  acdPromptSeqout (thys);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
      (void) acdUserGet (thys, &reply);

    ajSeqoutUsa (&val, reply);
    val->Features = osfeat;

    (void) ajStrSet(&val->Formatstr, fmt);
    if (!ajStrLen(val->Formatstr))
      (void) ajSeqOutFormatDefault(&val->Formatstr);

    (void) ajStrSet(&val->Extension, ext);
    (void) ajStrSet(&val->Extension, val->Formatstr);

    (void) acdGetValueAssoc (thys, "oufo", &val->Ufo);
    (void) acdGetValueAssoc (thys, "offormat", &val->Ftquery->Formatstr);
    (void) acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename);

    (void) acdLog ("acdSetSeqoutall ossingle default: %B\n",
		    ajSeqOutFormatSingle(val->Formatstr));

    (void) acdQualToBool (thys, "ossingle",
			  ajSeqOutFormatSingle(val->Formatstr),
			  &val->Single, &sing);

    (void) acdLog ("acdSetSeqoutall ossingle value %B '%S'\n",
		    val->Single, sing);

    if (!ajSeqoutOpen(val)) {
      acdBadVal (thys, required,
		 "Unable to write sequence to '%S'", reply);
      ok = ajFalse;
    }
  }
  if (!ok)
    acdBadRetry (thys);

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, reply);

  return;
}

/* @func ajAcdGetString *******************************************************
**
** Returns an item of type String as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPStr] String object. The string was already set by
**         acdSetString so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr ajAcdGetString (char *token) {

  return acdGetValue (token, "string");
}

/* @funcstatic acdSetString ***************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD string item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other is available) is an empty string.
**
** Attributes for minimum and maximum length are applied with error
** messages if exceeded.
** 
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFileNewOut
** @@
******************************************************************************/

static void acdSetString (AcdPAcd thys) {

  AjPStr val;

  AjBool required = ajFalse;
  AjBool ok = ajFalse;
  static AjPStr defreply = NULL;
  static AjPStr reply = NULL;
  static AjPStr pattern = NULL;
  AjBool upper;
  AjBool lower;
  ajint itry;

  AjPRegexp patexp = NULL;

  static AcdOAttr setattr[] = {
    {"length", VT_INT},
    {NULL, VT_NULL} };

  ajint minlen;
  ajint maxlen;
  ajint len;

  val = ajStrNew();		/* set the default value */

  (void) acdAttrToInt (thys, "minlength", 0, &minlen);

  (void) acdAttrToInt (thys, "maxlength", INT_MAX, &maxlen);
  (void) acdAttrToStr(thys, "pattern", "", &pattern);
  (void) acdAttrToBool (thys, "upper", ajFalse, &upper);
  (void) acdAttrToBool (thys, "lower", ajFalse, &lower);

  if (ajStrLen(pattern))
    patexp = ajRegComp(pattern);

  required = acdIsRequired(thys);
  (void) acdReplyInit (thys, "", &defreply);

  for (itry=acdPromptTry; itry && !ok; itry--) {

    ok = ajTrue;		/* accept the default if nothing changes */

    (void) ajStrAssS (&reply, defreply);

    if (required)
      (void) acdUserGet (thys, &reply);

    len = ajStrLen(reply);

    if (len < minlen) {
      acdBadVal (thys, required,
		 "Too short - minimum length is %d characters",
		 minlen);
      ok = ajFalse;
    }
    if (len > maxlen) {
      acdBadVal (thys, required,
		 "Too long - maximum length is %d characters",
		 maxlen);
      ok = ajFalse;
    }
    if (patexp && !ajRegExec (patexp, reply)) {
      acdBadVal (thys, required,
		 "String does no match pattern '%S'",
		 pattern);
      ok = ajFalse;
    }

  }
  if (!ok)
    acdBadRetry (thys);

  if (patexp)
    ajRegFree (&patexp);

  if (upper)
    ajStrToUpper (&val);

  if (lower)
    ajStrToLower (&val);

  /* strings have special set attributes */

  thys->SAttr = acdAttrListCount (setattr);
  thys->SetAttr = &setattr[0];
  thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));

  (void) ajStrFromInt (&thys->SetStr[0], ajStrLen(reply));

  (void) ajStrAssS(&val, reply);

  thys->Value = val;
  (void) ajStrAssS (&thys->ValStr, val);

  return;
}

/* @func ajAcdValue *******************************************************
**
** Returns the string value of any ACD item
**
** @param [r] token [char*] Text token name
** @return [AjPStr] String object. The string was already set by
**         acdSetString so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr ajAcdValue (char *token) {

  return acdGetValStr (token);
}

/* @funcstatic acdAttrCount ***************************************************
**
** Simply counts all attributes for a numbered ACD type,
**
** @param [r] itype [ajint] Numbered type as returned by acdFindType
** @return [ajint] number of attributes defined.
** @@
******************************************************************************/

static ajint acdAttrCount (ajint itype) {

  AcdPAttr attr = acdType[itype].Attr;

  return acdAttrListCount(attr);
}

/* @funcstatic acdAttrKeyCount ************************************************
**
** Simply counts all attributes for a numbered ACD keyword,
**
** @param [r] ikey [ajint] Numbered type as returned by acdFindKey
** @return [ajint] number of attributes defined.
** @@
******************************************************************************/

static ajint acdAttrKeyCount (ajint ikey) {

  AcdPAttr attr = acdKeywords[ikey].Attr;

  return acdAttrListCount(attr);
}

/* @funcstatic acdAttrListCount ***********************************************
**
** Simply counts all attributes for an attribute list.
**
** @param [r] attr [AcdPAttr] Attribute list
** @return [ajint] number of attributes defined.
** @@
******************************************************************************/

static ajint acdAttrListCount (AcdPAttr attr) {

  static ajint i;

  i = 0;
  while (attr[i].Name)
    i++;

  return i;
}

/* @funcstatic acdGetValue ****************************************************
**
** Picks up a token by name and tests the type.
** The value is returned as type "void*", to be cast by the calling
** routine which is supposed to know what to expect. For example, only
** ajAcdGetInt should call with a type of "integer".
**
** @param [r] token [char*] Token name, optionally including a numeric suffix.
** @param [r] type [char*] Type.
** @return [void*] Value.
**
******************************************************************************/

static void* acdGetValue (char *token, char* type) {

  void* ret;
  ajint pnum = 0;                 /* need to get from end of token */

  char *cp = ajCharNewC(strlen(token), token);

  acdLog ("acdGetValue '%s' (%s)\n", token, type);

  acdTokenToLower (cp, &pnum);

  ret = acdGetValueNum (cp, type, pnum);

  (void) ajCharFree (cp);
  return ret;
}

/* @funcstatic acdGetValStr ***************************************************
**
** Picks up a token by name and tests the type.
** The string value is returned for any data type.
**
** @param [r] token [char*] Token name, optionally including a numeric suffix.
** @return [AjPStr] String.
**
******************************************************************************/

static AjPStr acdGetValStr (char *token) {

  AcdPAcd acd;
  ajint pnum = 0;                 /* need to get from end of token */
  static AjPStr tokstr = NULL;

  char *cp = ajCharNewC(strlen(token), token);

  acdLog ("acdGetValStr '%s' (%s)\n", token);

  acdTokenToLower (cp, &pnum);
  ajStrAssC (&tokstr, cp);
  /* AJB addition of free */
  AJFREE(cp);
  acd = acdFindAcd (tokstr, tokstr, pnum);
  if (!acd) return NULL;

  return acd->ValStr;
}

/* @funcstatic acdGetValueAssoc ***********************************************
**
** Picks up the value for an associated qualifier as a string.
**
** @param [r] thys [AcdPAcd] ACD item for the master parameter/qualifier
** @param [r] token [char*] Token name, optionally including a numeric suffix.
** @param [wP] result [AjPStr*] String for the resulting value.
** @return [AjBool] ajTrue if found.
** @cre Aborts if not found.
**
******************************************************************************/

static AjBool acdGetValueAssoc (AcdPAcd thys, char *token, AjPStr *result) {

  ajint pnum = 0;                 /* need to get from end of token */
  AcdPAcd pa;
  char *cp = ajCharNewC(strlen(token), token);

  acdLog ("acdGetValueAssoc '%s' (%S)\n", token, thys->Name);

  acdTokenToLower (cp, &pnum);
  (void) ajCharFree(cp);

  if (pnum)
    ajFatal("associated token '%s' is numbered - not allowed\n");

  for (pa=thys->AssocQuals; pa->Assoc; pa=pa->Next) {
    if (ajStrMatchC(pa->Token, token)) {
      (void) ajStrAssS(result, pa->ValStr);
      return pa->Defined;
    }
  }

  ajFatal("Token '%s' not found\n", token);

  return ajFalse;
}

/* @funcstatic acdGetValueNum *************************************************
**
** Picks up the value by name, type and number.
**
** @param [r] token [char*] Token name
** @param [r] type [char*] ACD type
** @param [r] pnum [ajint] parameter number, or 0 for a general qualifier.
** @return [void*] Value of unknown type.
** @cre Aborts if not found.
**
******************************************************************************/

static void* acdGetValueNum (char *token, char* type, ajint pnum) {

  AcdPAcd pa;
  AcdPAcd ret = NULL;
  ajint itype = 0;
  ajint ifound = 0;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  if (type)
    itype = acdFindTypeC(type);

  for (pa=acdList; pa; pa=pa->Next) {
    if (ajStrMatchC(pa->Token, token)) {
      if (pa->PNum == pnum) {
	if (itype && (pa->Type != itype))
	  ajFatal ("Token %s is not of type %s\n", token, type);
	acdLog ("found %S [%d] '%S'\n",
		pa->Name, pa->PNum, pa->ValStr);
	return pa->Value;
      }
      else if (!pnum) {		/* matches any if unique, so count them */
	ifound++;
	ret = pa;
	acdAmbigApp (&ambigList, pa->Token);
      }
    }
  }

  if (ifound > 1) {
    ajWarn ("ambiguous token %s (%S)", token, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }
  if (ifound == 1) {
    if (itype && (ret->Type != itype))
      ajFatal ("Token %s is not of type %s\n", token, type);
    acdLog ("found %S [%d] '%S'\n",
		   ret->Name, ret->PNum, ret->ValStr);
    return ret->Value;
  }

  ajFatal("Token '%s' not found\n", token);

  return NULL;
}

/* @funcstatic acdHelp ********************************************************
**
** Reports on program options if acdDoHelp is set, either by
** -help on the command line or by the prefix_HELP variable..
**
** @return [void]
** @@
******************************************************************************/

static void acdHelp (void) {

  AcdPAcd pa;
  static AjPStr helpReq = NULL;
  static AjPStr helpOpt = NULL;
  static AjPStr helpAdv = NULL;
  static AjPStr helpAss = NULL;
  static AjPStr helpGen = NULL;
  static AjPStr helpStr = NULL;
  enum { HELP_UNK, HELP_APP, HELP_REQ, HELP_OPT,
	 HELP_ADV, HELP_ASS, HELP_GEN} helpType;
  AjPStr* def;
  AjBool tmpBool;
  char hlpFlag;
  AjBool flagReq = ajFalse;
  AjBool flagOpt = ajFalse;
  AjPList reqlist = NULL;
  AjPList optlist = NULL;
  AjPList advlist = NULL;
  AjPList genlist = NULL;
  AjPList asslist = NULL;

  ajDebug("acdHelp %B\n", acdDoHelp);

  if (!acdDoHelp) return;

  if (acdTable) {
    reqlist = ajListNew();
    optlist = ajListNew();
    advlist = ajListNew();
    genlist = ajListNew();
    if (acdVerbose) asslist = ajListNew();
    ajUser ("<table border cellspacing=0 cellpadding=3 bgcolor=\"#f5f5ff\">");
  }

  acdLog ("++ acdHelp\n");
  for (pa=acdList; pa; pa=pa->Next) {
    hlpFlag = ' ';
    acdLog ("++ Name %S Level %d Assoc %B AssocQuals %x\n",
	    pa->Name, pa->Level, pa->Assoc, pa->AssocQuals);
    helpType = HELP_ADV;
    if (pa->Level == ACD_APPL)
      helpType = HELP_APP;
    else {
      if (!acdIsQtype(pa)) continue;
    }

    def = pa->DefStr;

    if (def && ajStrLen(def[DEF_OPTIONAL])) {
      if (acdHelpVarResolve(&helpStr, def[DEF_OPTIONAL])) {
	if (!ajStrToBool(helpStr, &tmpBool))
	  ajFatal ("%S: Bad optional flag %S\n",
		   pa->Name, def[DEF_OPTIONAL]);
      }
      else {
	tmpBool = ajTrue;
        hlpFlag = '*';
	flagOpt = ajTrue;
      }
      if (tmpBool) helpType = HELP_OPT;
    }

    if (def && ajStrLen(def[DEF_REQUIRED])) {
      if (acdHelpVarResolve(&helpStr, def[DEF_REQUIRED])) {
	if (!ajStrToBool( helpStr, &tmpBool))
	  ajFatal ("%S: Bad required flag %S\n",
		   pa->Name, def[DEF_REQUIRED]);
      }
      else {
	tmpBool = ajTrue;
        hlpFlag = '*';
	flagReq = ajTrue;
      }
      if (tmpBool) helpType = HELP_REQ;
    }

    if (pa->Assoc) helpType = HELP_ASS;

    acdLog ("++ helpType %d\n", helpType);

    switch (helpType) {
    case HELP_APP:		/* application, do nothing */
      break;
    case HELP_REQ:
      acdHelpAppend (pa, &helpReq, hlpFlag);
      acdHelpTable  (pa, reqlist, hlpFlag);
      break;
    case HELP_OPT:
      acdHelpAppend (pa, &helpOpt, hlpFlag);
      acdHelpTable  (pa, optlist, hlpFlag);
      break;
    case HELP_ADV:
      acdHelpAppend (pa, &helpAdv, hlpFlag);
      acdHelpTable  (pa, advlist, hlpFlag);
      break;
    case HELP_ASS:		/* associated - process after the master */
      break;
    case HELP_GEN:		/* associated - process after the app */
      break;
    default:
      ajFatal ("unknown qualifier type %d in acdHelp", helpType);
    }

    if (pa->AssocQuals) {
      if (helpType == HELP_APP) {
	if (acdVerbose) {
	  acdHelpAssoc (pa, &helpGen, NULL);
	  acdHelpAssocTable  (pa, genlist, hlpFlag);
	}
	else {
	  acdHelpAssoc (pa, &helpGen, "help");
	  acdHelpAssocTable  (pa, genlist, hlpFlag);
	}
      }
      else {
	if (acdVerbose) {
	  acdHelpAssoc (pa, &helpAss, NULL);
	  acdHelpAssocTable  (pa, asslist, hlpFlag);
	}
      }
    }
  }

  if (flagReq)
    acdHelpShow (helpReq, "Mandatory qualifiers (* if not always prompted)");
  else
    acdHelpShow (helpReq, "Mandatory qualifiers");
  acdHelpTableShow (reqlist, "Mandatory qualifiers");
  if (flagOpt)
    acdHelpShow (helpOpt, "Optional qualifiers (* if not always prompted)");
  else
    acdHelpShow (helpOpt, "Optional qualifiers");
  acdHelpTableShow (optlist, "Optional qualifiers");
  acdHelpShow (helpAdv, "Advanced qualifiers");
  acdHelpTableShow (advlist, "Advanced qualifiers");
  if (acdVerbose) acdHelpShow
		    (helpAss, "Associated qualifiers");
  acdHelpShow (helpGen, "General qualifiers");
  if (acdVerbose && acdTable)
    acdHelpTableShow (asslist, "Associated qualifiers");
  if (acdVerbose && acdTable)
    acdHelpTableShow (genlist, "General qualifiers");

  if (acdTable) {
    ajUser ("</table>");
  }

  ajExit();
}

/* @funcstatic acdHelpAssoc **************************************************
**
** Processes all associated qualifiers for a qualifier or for the application.
**
** If a qualifier name is given (e.g. "help") then only that qualifier
** is processed.
**
** @param [r] thys [AcdPAcd]  ACD object
** @param [r] str [AjPStr*] Help text being built
** @param [r] name [char*] Single name to process
** @return [void]
** @@
******************************************************************************/

static void acdHelpAssoc (AcdPAcd thys, AjPStr *str, char* name) {

  static AjPStr line = NULL;
  static AjPStr qname = NULL;
  static AjPStr qtype = NULL;
  static AjPStr text = NULL;
  AcdPQual quals;
  ajint i;

  acdLog ("++ acdHelpAssoc %S\n", thys->Name);

  if (thys->Level == ACD_APPL) {
    quals = acdQualAppl;
  }
  else {
    (void) ajFmtPrintS (&line, "  \"-%S\" related qualifiers\n", thys->Name);
    (void) ajStrApp (str, line);
    quals = acdType[thys->Type].Quals;
  }

  acdLog ("++ type %d quals %x\n", thys->Type, quals);

  if (quals) {
    for (i=0; quals[i].Name; i++) {
      acdLog ("++ quals[%d].Name %s\n", i, quals[i].Name);
      if (name && strcmp(name, quals[i].Name)) continue;
      if (thys->PNum)
	(void) ajFmtPrintS (&qname, "-%s%d", quals[i].Name, thys->PNum);
      else
	(void) ajFmtPrintS (&qname, "-%s", quals[i].Name);
      (void) ajStrAssC (&qtype, quals[i].Type);
      (void) ajFmtPrintS (&line, "  %-20S %-10S ",
			  qname,  qtype);
      (void) ajStrAssC (&text, quals[i].Help);
      (void) acdTextFormat(&text);
      (void) ajStrWrapLeft (&text, 45, 34);
      (void) ajStrApp (&line, text);
      (void) ajStrAppC (&line, "\n");
      (void) ajStrApp (str, line);
    }
  }
  return;
}

/* @funcstatic acdHelpAppend **************************************************
**
** Appends a qualifier and its help text to a help category string.
**
** @param [r] thys [AcdPAcd]  ACD object
** @param [r] str [AjPStr*] Help text being built
** @param [r] flag [char] Flag character. Usually blank, but an asterisk
**                         is used if the status (optional/required) is
**                         uncertain.
** @return [void]
** @@
******************************************************************************/

static void acdHelpAppend (AcdPAcd thys, AjPStr *str, char flag) {
  static AjPStr name = NULL;
  static AjPStr valstr = NULL;
  static AjPStr nostr = NULL;
  static AjPStr nullstr = NULL;
  static AjPStr text = NULL;
  static AjPStr line = NULL;
  static AjPStr type = NULL;
  AjBool boolval;

  AjPStr defstr;

  if (!nullstr) nullstr = ajStrNew();

  (void) ajStrAssC (&type, acdType[thys->Type].Name);

  if (thys->DefStr)
    defstr = thys->OrigStr;
  else
    defstr = nullstr;

  (void) ajStrAssC (&nostr, "");
  if (acdIsQtype(thys) && ajStrMatchCC("bool", acdType[thys->Type].Name)) {
    if (ajStrToBool(defstr, &boolval) && boolval)
      (void) ajStrAssC (&nostr, "[no]");
    defstr = nullstr;
  }

  (void) ajStrAssS (&valstr, defstr);

  /* warning - don't try acdVarResolve here because we have not yet
     read in the data and things like calculated attributes do not exist */

  if (thys->Level == ACD_PARAM)
    (void) ajFmtPrintS (&name, "[-%S%S]", nostr, thys->Name);
  else
    (void) ajFmtPrintS (&name, " -%S%S", nostr, thys->Name);

  (void) ajFmtPrintS  (&line, "%c %-20S %-10S ", flag, name, type);
  acdHelpText (thys, &text);
  (void) acdTextFormat(&text);
  (void) ajStrWrapLeft (&text, 45, 34);
  (void) ajStrApp (&line, text);
  (void) ajStrAppC (&line, "\n");
  (void) ajStrApp (str, line);
  return;
}

/* @funcstatic acdHelpValidSeq ***********************************************
**
** Generates valid description for an input sequence type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidSeq (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidSeqout *********************************************
**
** Generates valid description for an output sequence type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidSeqout (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidOut ************************************************
**
** Generates valid description for an outfile type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidOut (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidIn *************************************************
**
** Generates valid description for an infile type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidIn (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidData ***********************************************
**
** Generates valid description for a datafile type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidData (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidInt ************************************************
**
** Generates valid description for an integer type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidInt (AcdPAcd thys, AjPStr* str) {
  ajint imin;
  ajint imax;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "minimum", "$", &tmpstr);
  if (!ajStrToInt(tmpstr, &imin))
    imin = INT_MIN;

  acdAttrValueStr (thys, "maximum", "$", &tmpstr);
  if (!ajStrToInt(tmpstr, &imax))
    imax = INT_MAX;


  if (imax != INT_MAX) {
    if (imin != INT_MIN)
      ajFmtPrintS (str, "Integer from %d to %d", imin, imax);
    else
      ajFmtPrintS (str, "Integer up to %d", imax);
  }
  else {
    if (imin != INT_MIN)
      ajFmtPrintS (str, "Integer %d or more", imin);
    else
      ajFmtPrintS (str, "Any integer value");
  }

  return;
}

/* @funcstatic acdHelpValidFloat **********************************************
**
** Generates valid description for a floating point type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidFloat (AcdPAcd thys, AjPStr* str) {

  float fmin;
  float fmax;
  ajint iprec;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "minimum", "$", &tmpstr);
  if (!ajStrToFloat(tmpstr, &fmin))
    fmin = -FLT_MAX;

  acdAttrValueStr (thys, "maximum", "$", &tmpstr);
  if (!ajStrToFloat(tmpstr, &fmax))
    fmax = FLT_MAX;

  acdAttrValueStr(thys, "precision", "$", &tmpstr);
  if (!ajStrToInt(tmpstr, &iprec))
    iprec = 3;

  if (fmax != FLT_MAX) {
    if (fmin != -FLT_MAX)
      ajFmtPrintS (str, "Number from %.*f to %.*f", iprec, fmin, iprec, fmax);
    else
      ajFmtPrintS (str, "Number up to %.*f", iprec, fmax);
  }
  else {
    if (fmin != -FLT_MAX)
      ajFmtPrintS (str, "Number %.*f or more", iprec, fmin);
    else
      ajFmtPrintS (str, "Any integer value");
  }

  return;
}

/* @funcstatic acdHelpValidCodon **********************************************
**
** Generates valid description for a codon usage table type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidCodon (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidDirlist *******************************************
**
** Generates valid description for a dirlist type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidDirlist (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidMatrix *********************************************
**
** Generates valid description for a comparison matrix type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidMatrix (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidFeatout ********************************************
**
** Generates valid description for a feature output type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidFeatout (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidCpdb **********************************************
**
** Generates valid description for a clean pdb type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidCpdb (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidScop **********************************************
**
** Generates valid description for a scop type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidScop (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidRange *********************************************
**
** Generates valid description for a sequence range.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidRange (AcdPAcd thys, AjPStr* str) {
  return;
}

/* @funcstatic acdHelpValidGraph **********************************************
**
** Generates valid description for a graphics device type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidGraph (AcdPAcd thys, AjPStr* str) {
  AjPList list;
  AjPStr name = NULL;
  ajint i = 0;
  list = ajListstrNew();

  call ("ajGraphListDevices", list);

  ajFmtPrintS (str, "EMBOSS has a list of known devices, including ");

  while (ajListstrPop (list, &name)) {
    if (i)
      ajFmtPrintAppS (str, ", ");
    ajFmtPrintAppS (str, "%S", name);
    ajStrDel (&name);
    i++;
  }

  ajListDel (&list);
  return;
}

/* @funcstatic acdHelpValidString *********************************************
**
** Generates valid description for a string type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidString (AcdPAcd thys, AjPStr* str) {

  ajint minlen;
  ajint maxlen;
  static AjPStr patstr = NULL;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "min", "0", &tmpstr);
  if (!ajStrToInt(tmpstr, &minlen))
    minlen = 0;
  acdAttrValueStr (thys, "max", "0", &tmpstr);
  if (!ajStrToInt(tmpstr, &maxlen))
    maxlen = 0;

  if (maxlen > 0) {
    if (minlen > 0)
      ajFmtPrintS (str, "A string from %d to %d characters", minlen, maxlen);
    else
      ajFmtPrintS (str, "A string up to %d characters", maxlen);
  }
  else {
    if (minlen > 0)
      ajFmtPrintS (str, "A string of at least %d characters", minlen);
    else
      ajStrAssC (str, "Any string is accepted");
  }

  acdAttrValueStr (thys, "pattern", "", &patstr);

  if (ajStrLen(patstr))
    ajFmtPrintAppS (str, ", matching regular expression /%S/", patstr);

  ajStrDelReuse(&tmpstr);
  ajStrDelReuse(&patstr);

  return;
}

/* @funcstatic acdHelpValidRegexp *********************************************
**
** Generates valid description for a regular expression type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidRegexp (AcdPAcd thys, AjPStr* str) {

  ajint minlen;
  ajint maxlen;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "min", "0", &tmpstr);
  if (!ajStrToInt(tmpstr, &minlen))
    minlen = 0;
  acdAttrValueStr (thys, "max", "0", &tmpstr);
  if (!ajStrToInt(tmpstr, &maxlen))
    maxlen = 0;

  if (maxlen > 0) {
    if (minlen > 0)
      ajFmtPrintS (str,
		   "A regular epression pattern from %d to %d characters",
		   minlen, maxlen);
    else
      ajFmtPrintS (str,
		   "A regular epression pattern up to %d characters",
		   maxlen);
  }
  else {
    if (minlen > 0)
      ajFmtPrintS (str,
		   "A regular epression pattern of at least %d characters",
		   minlen);
    else
      ajStrAssC (str,
		 "Any regular epression pattern is accepted");
  }

  ajStrDelReuse(&tmpstr);

  return;
}

/* @funcstatic acdHelpValidList ***********************************************
**
** Generates valid description for a list type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidList (AcdPAcd thys, AjPStr* str) {

  AjPStr codedelim = NULL;
  AjPStr delim = NULL;
  AjPStr value = NULL;
  AjPStrTok handle;
  AjPStrTok codehandle;
  static AjPStr code = NULL;
  static AjPStr desc = NULL;
  static AjPStr line = NULL;
  static char* white = " \t\n\r";

  acdAttrValueStr (thys, "delimiter", ";", &delim);

  acdAttrValueStr (thys, "value", "", &value);

  handle = ajStrTokenInit (value, ajStrStr(delim));

  ajFmtPrintS (str, "<table>");
  acdAttrValueStr (thys, "codedelimiter", ":", &codedelim);
  while (ajStrDelim (&line, &handle, NULL)) {
    codehandle = ajStrTokenInit (line, ajStrStr(codedelim));
    (void) ajStrToken (&code, &codehandle, NULL);
    (void) ajStrToken (&desc, &codehandle, ajStrStr(delim));
    (void) ajStrTrimC (&code, white);
    (void) ajStrTrimC (&desc, white);
    ajFmtPrintAppS (str, "<tr><td>%S</td> <td><i>(%S)</i></td></tr>",
		    code, desc);
    (void) ajStrTokenClear (&codehandle);
  }
  ajFmtPrintAppS (str, "</table>");

  (void) ajStrTokenClear (&handle);
  return;
}

/* @funcstatic acdHelpValidSelect *********************************************
**
** Generates valid description for a select type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidSelect (AcdPAcd thys, AjPStr* str) {

  AjPStr delim = NULL;
  AjPStr value = NULL;
  AjPStrTok handle;
  static AjPStr desc = NULL;
  static char* white = " \t\n\r";

  acdAttrValueStr (thys, "delimiter", ";", &delim);

  acdAttrValueStr (thys, "value", "", &value);

  handle = ajStrTokenInit (value, ajStrStr(delim));

  while (ajStrDelim (&desc, &handle, NULL)) {
    (void) ajStrTrimC (&desc, white);
    ajFmtPrintAppS (str, "%S<br>", desc);
  }

  (void) ajStrTokenClear (&handle);
  return;
}

/* @funcstatic acdHelpValid **************************************************
**
** Generates help text for an ACD object using the help, info, prompt
** and code settings.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValid (AcdPAcd thys, AjPStr* str) {

  ajint i;

  if (ajStrLen(*str)) return;

  
  if (acdAttrValueStr(thys, "valid", "", str))
    return;

  /* special processing for sequences, outseq, outfile */

  for (i=0; acdValid[i].Name; i++) {
    if (ajStrMatchCC (acdType[thys->Type].Name, acdValid[i].Name)) {
      if (acdValid[i].Valid) acdValid[i].Valid(thys, str);
      break;
    }
  }

  if (ajStrLen(*str)) return;

  ajStrAssC (str, acdType[thys->Type].Valid);

  return;
}

/* @funcstatic acdHelpExpectSeq ***********************************************
**
** Generates expected value description for an input sequence type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectSeq (AcdPAcd thys, AjPStr* str) {

  ajFmtPrintS(str, "<b>Required</b>");

  return;
}

/* @funcstatic acdHelpExpectSeqout ********************************************
**
** Generates expected value description for an output sequence type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectSeqout (AcdPAcd thys, AjPStr* str) {

  static ajint icall = 0;

  if (!icall++)
    ajFmtPrintS (str, "<i>&lt;sequence&gt;</i>.<i>format</i>");

  return;
}

/* @funcstatic acdHelpExpectOut ***********************************************
**
** Generates expected value description for an outfile type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectOut (AcdPAcd thys, AjPStr* str) {

  static ajint icall = 0;

  if (!icall++)
    ajFmtPrintS (str, "<i>&lt;sequence&gt;</i>.%S", acdProgram);

  return;
}

/* @funcstatic acdHelpExpectInt ***********************************************
**
** Generates expected value description for an integer type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectInt (AcdPAcd thys, AjPStr* str) {

  ajint i;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "default", "0", &tmpstr);
  if (ajStrToInt(tmpstr, &i))
    ajFmtPrintS(str, "%d", i);
  else
    ajFmtPrintS(str, "<i>calculated value</i>");

  return;
}

/* @funcstatic acdHelpExpectFloat *********************************************
**
** Generates expected value description for a floating point type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectFloat (AcdPAcd thys, AjPStr* str) {

  float f;
  ajint iprec;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "default", "0.0", &tmpstr);
  if (!ajStrToFloat(tmpstr, &f))
    f = 0.0;

  acdAttrValueStr   (thys, "precision", "3", &tmpstr);
  if (!ajStrToInt(tmpstr, &iprec))
    iprec = 3;

  ajFmtPrintS(str, "%.*f", iprec, f);

  return;
}

/* @funcstatic acdHelpExpectIn ************************************************
**
** Generates expected value description for an infile type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectIn (AcdPAcd thys, AjPStr* str) {

  ajFmtPrintS(str, "<b>Required</b>");

  return;
}

/* @funcstatic acdHelpExpectData **********************************************
**
** Generates expected value description for a datafile type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectData (AcdPAcd thys, AjPStr* str) {

  ajFmtPrintS(str, "<i>File in the data file path</i>");

  return;
}

/* @funcstatic acdHelpExpectCodon *********************************************
**
** Generates expected value description for a codon usage table type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectCodon (AcdPAcd thys, AjPStr* str) {

  (void) acdAttrResolve (thys, "name", str);
  if (ajStrLen(*str)) return;

  ajStrAssC (str, DEFCODON);

  return;
}


/* @funcstatic acdHelpExpectDirlist ******************************************
**
** Generates expected value description for a dirlist type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectDirlist (AcdPAcd thys, AjPStr* str) {

  (void) acdAttrResolve (thys, "name", str);
  if (ajStrLen(*str)) return;

  ajStrAssC (str, DEFDLIST);

  return;
}

/* @funcstatic acdHelpExpectCpdb *********************************************
**
** Generates expected value description for a clean pdb file type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectCpdb (AcdPAcd thys, AjPStr* str) {

  (void) acdAttrResolve (thys, "name", str);
  if (ajStrLen(*str)) return;

  ajStrAssC (str, DEFCPDB);

  return;
}

/* @funcstatic acdHelpExpectScop *********************************************
**
** Generates expected value description for a scop type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectScop (AcdPAcd thys, AjPStr* str) {

  (void) acdAttrResolve (thys, "name", str);
  if (ajStrLen(*str)) return;

  ajStrAssC (str, DEFSCOP);

  return;
}

/* @funcstatic acdHelpExpectMatrix ********************************************
**
** Generates expected value description for a comparison matrix type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectMatrix (AcdPAcd thys, AjPStr* str) {

  ajStrAssC (str, "EBLOSUM62 for protein<br>EDNAFULL for DNA");

  return;
}

/* @funcstatic acdHelpExpectFeatout *******************************************
**
** Generates expected value description for a feature output type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectFeatout (AcdPAcd thys, AjPStr* str) {

  ajStrAssC (str, "<i>unknown.gff</i>");

  return;
}

/* @funcstatic acdHelpExpectRange ********************************************
**
** Generates expected value description for a sequence range type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectRange (AcdPAcd thys, AjPStr* str) {

  ajStrAssC (str, "<i>full sequence</i>");

  return;
}

/* @funcstatic acdHelpExpectGraph *********************************************
**
** Generates expected value description for a graphics device type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectGraph (AcdPAcd thys, AjPStr* str) {

  ajStrAssC (str, "<i>EMBOSS_GRAPHICS</i> value, or x11");

  return;
}

/* @funcstatic acdHelpExpectRegexp ********************************************
**
** Generates expected value description for a regular expression type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectRegexp (AcdPAcd thys, AjPStr* str) {

  ajint minlen;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "min", "1", &tmpstr);
  if (!ajStrToInt(tmpstr, &minlen))
    minlen = 0;

  if (minlen > 0)
    ajStrAssC (str, "<b>Required</b>");
  else
    ajStrAssC (str, "<i>An empty regular expression is accepted</i>");
  return;
}

/* @funcstatic acdHelpExpectString ********************************************
**
** Generates expected value description for a string type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectString (AcdPAcd thys, AjPStr* str) {

  ajint minlen;
  static AjPStr tmpstr = NULL;

  acdAttrValueStr (thys, "min", "0", &tmpstr);
  if (!ajStrToInt(tmpstr, &minlen))
    minlen = 0;

  if (minlen > 0)
    ajStrAssC (str, "<b>Required</b>");
  else
    ajStrAssC (str, "<i>An empty string is accepted</i>");
  return;
}

/* @funcstatic acdHelpExpect **************************************************
**
** Generates expected value text for an ACD objectcode settings.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpect (AcdPAcd thys, AjPStr* str) {

  ajint i;
  if (ajStrLen(*str)) return;

  if (!thys->AssocQuals) {
    if (acdAttrValueStr(thys, "expected", "", str))
      return;
  }

  if (acdAttrValueStr(thys, "default", "", str))
    return;

  /* special processing for sequences, outseq, outfile */

  for (i=0; acdValid[i].Name; i++) {
    if (ajStrMatchCC (acdType[thys->Type].Name, acdValid[i].Name)) {
      if (acdValid[i].Expect) acdValid[i].Expect(thys, str);
      break;
    }
  }
  if (ajStrLen(*str)) return;

  ajStrAssS (str, thys->DefStr[DEF_DEFAULT]);

  if (ajStrLen(*str)) return;

  (void) ajStrAssC (str, "&nbsp;");

  return;
}

/* @funcstatic acdHelpText **************************************************
**
** Generates help text for an ACD object using the help, info, prompt
** and code settings.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpText (AcdPAcd thys, AjPStr* str) {

  AjPStr prompt;
  AjPStr info;
  AjPStr code;
  AjPStr help;
  static AjPStr msg = NULL;

  if (thys->DefStr) {

    prompt = thys->DefStr[DEF_PROMPT];
    info = thys->DefStr[DEF_INFO];
    code = thys->DefStr[DEF_CODE];
    help = thys->DefStr[DEF_HELP];

    if (ajStrLen(help))
      (void) ajStrAssS(&msg, help);

    else if (ajStrLen(code))
      (void) acdCodeGet (code, &msg);

    else if (ajStrLen(info))
      (void) ajStrAssS(&msg, info);

    else if (ajStrLen(prompt))
      (void) ajStrAssS(&msg, prompt);

    else {
      if (!acdHelpCodeDef (thys, &msg)) {
	(void) ajStrAssCL(&msg, "", 512);
	if (thys->Assoc)
	  (void) ajFmtPrintS (&msg, "%s value",
			      acdType[thys->Type].Name);
	else
	  (void) ajFmtPrintS (&msg, "(no help text) %s value",
			      acdType[thys->Type].Name);
      }
    }

    (void) acdVarResolve(&msg);
    (void) ajStrAssS (str, msg);
    (void) ajStrDelReuse(&msg);
  }
  return;
}

/* @funcstatic acdHelpShow **************************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] str [AjPStr] Help text (if any)
** @param [r] title [char*] Title line for this call
** @return [void]
** @@
******************************************************************************/

static void acdHelpShow (AjPStr str, char* title) {

  if (acdTable) return;

  if (!ajStrLen(str)) {
    ajUser("   %s: (none)", title);
    return;
  }

  ajUser("   %s:", title);
  ajUser("%S", str);

  return;
}


/* @funcstatic acdHelpTableShow ***********************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] tablist [AjPList] Help text (if any)
** @param [r] title [char*] Title line for this call
** @return [void]
** @@
******************************************************************************/

static void acdHelpTableShow (AjPList tablist, char* title) {

  AcdPTableItem item;
  AjIList iter = NULL;

  if (!acdTable) return;

  ajUser("<tr bgcolor=\"#FFFFD0\">");
  ajUser("<th align=\"left\" colspan=2>%s</th>", title);
  ajUser("<th align=\"left\">Allowed values</th>");
  ajUser("<th align=\"left\">Default</th>");
  ajUser("</tr>\n");

  if (!ajListLength(tablist)) {
    ajUser("<tr>");
    ajUser("<td colspan=4>(none)</td>");
    ajUser("</tr>\n");
  }
  else {
    iter=ajListIter(tablist);
    while ((item = ajListIterNext(iter))) {
      ajUser("<tr>");
      ajUser("<td>%S</td>", item->Qual);
      ajUser("<td>%S</td>", item->Help);
      ajUser("<td>%S</td>", item->Valid);
      ajUser("<td>%S</td>", item->Expect);
      ajUser("</tr>\n");
    }
  }

  ajListIterFree (iter);

  return;
}


/* @funcstatic acdHelpAssocTable **********************************************
**
** Appends an associated qualifier and its help text to the table list.
**
** @param [r] thys [AcdPAcd]  ACD object
** @param [r] tablist [AjPList] Help text list being built
** @param [r] flag [char] Flag character. Usually blank, but an asterisk
**                         is used if the status (optional/required) is
**                         uncertain.
** @return [void]
** @@
******************************************************************************/

static void acdHelpAssocTable (AcdPAcd thys, AjPList tablist, char flag) {

  AcdPTableItem item;

  static AjPStr line = NULL;
  static AjPStr qname = NULL;
  static AjPStr qtype = NULL;
  AcdPQual quals;
  ajint i;
  AcdPAcd pa;

  if (!acdTable) return;

  acdLog ("++ acdHelpAssoc %S\n", thys->Name);

  if (thys->Level == ACD_APPL) {
    quals = acdQualAppl;
  }
  else {
    (void) ajFmtPrintS (&line, "  \"-%S\" related qualifiers\n", thys->Name);
    /*(void) ajStrApp (str, line);*/
    quals = acdType[thys->Type].Quals;
  }

  acdLog ("++ type %d quals %x\n", thys->Type, quals);

  i=0;
  for (pa=thys->AssocQuals; pa->Assoc; pa=pa->Next) {
      acdLog ("++ assoc[%d].Name %S\n", i, pa->Name);
      AJNEW0 (item);
      if (thys->PNum)
	(void) ajFmtPrintS (&item->Qual, "-%S%d", pa->Name, pa->PNum);
      else
	(void) ajFmtPrintS (&item->Qual, "-%S", pa->Name);
      (void) ajFmtPrintS (&line, "  %-20S %-10S ",
			  qname,  qtype);
      acdHelpText (pa, &item->Help);
      acdHelpValid (pa, &item->Valid);
      acdHelpExpect (pa, &item->Expect);
      ajListPushApp (tablist, item);
  }


  return;
}

/* @funcstatic acdHelpTable **************************************************
**
** Appends a qualifier and its help text to the table list.
**
** @param [r] thys [AcdPAcd]  ACD object
** @param [r] tablist [AjPList] Help text list being built
** @param [r] flag [char] Flag character. Usually blank, but an asterisk
**                         is used if the status (optional/required) is
**                         uncertain.
** @return [void]
** @@
******************************************************************************/

static void acdHelpTable (AcdPAcd thys, AjPList tablist, char flag) {

  AcdPTableItem item;

  static AjPStr name = NULL;
  static AjPStr nostr = NULL;
  static AjPStr nullstr = NULL;
  static AjPStr type = NULL;
  AjBool boolval;

  AjPStr defstr;

  if (!acdTable) return;

  AJNEW0 (item);

  if (!nullstr) nullstr = ajStrNew();

  if (thys->DefStr)
    defstr = thys->OrigStr;
  else
    defstr = nullstr;

  (void) ajStrAssC (&nostr, "");
  if (acdIsQtype(thys) && ajStrMatchCC("bool", acdType[thys->Type].Name)) {
    if (ajStrToBool(defstr, &boolval)) {
      if (boolval)
	(void) ajStrAssC (&nostr, "[no]");
      ajFmtPrintS (&item->Expect, "%B", boolval);
    }
    else {
      if (!ajStrLen(defstr))
	ajFmtPrintS (&item->Expect, "%B", ajFalse);
    }
    defstr = nullstr;
  }

  if (thys->Level == ACD_PARAM)
    (void) ajFmtPrintS (&item->Qual, "[-%S%S]<br>(Parameter %d)",
			nostr, thys->Name, thys->PNum);
  else
    (void) ajFmtPrintS (&item->Qual, "-%S%S", nostr, thys->Name);

  (void) ajStrAssC (&type, acdType[thys->Type].Name);

  acdHelpExpect (thys, &item->Expect);

  /* warning - don't try acdVarResolve here because we have not yet
     read in the data and things like calculated attributes do not exist */

  if (thys->Level == ACD_PARAM)
    (void) ajFmtPrintS (&name, "[-%S%S]", nostr, thys->Name);
  else
    (void) ajFmtPrintS (&name, "-%S%S", nostr, thys->Name);

  acdHelpValid (thys, &item->Valid);
  acdHelpText (thys, &item->Help);

  ajListPushApp (tablist, item);

  return;
}
/* @funcstatic acdListReport **************************************************
**
** Reports the current status of the ACD internal structures, converting
** values to a printable form as appropriate.
**
** @param [r] title [char*] Title line for this call
** @return [void]
** @@
******************************************************************************/

static void acdListReport (char* title) {

  AcdPAcd pa;

  ajint i = 0;
  ajint j = 0;
  char underline[256];


  if (!acdDoLog) return;

  j = strlen(title);
  if (j > 255)
    j = 255;

  (void) memset (underline, '=', j);
  underline[j] = '\0';

  acdLog ("\n");
  acdLog ("%s\n", title);
  acdLog ("%s\n", underline);
  acdLog ("\n");

  for (pa=acdList; pa; pa=pa->Next) {
    acdLog ("ACD %d\n", i);
    if (pa->PNum) {
      acdLog ("       Name: '%S[%d]'\n", pa->Name, pa->PNum);
      acdLog ("      Token: '%S[%d]'\n", pa->Token, pa->PNum);
    }
    else {
      acdLog ("       Name: '%S'\n", pa->Name);
      acdLog ("      Token: '%S'\n", pa->Token);
    }
    acdLog ("      Param: %d\n", pa->PNum);
    acdLog ("      Level: %d   (%s)\n", pa->Level, acdLevel[pa->Level]);
    if (acdIsQtype(pa))
      acdLog ("  Qual Type: %d   (%s)\n", pa->Type,
		     acdType[pa->Type].Name);
    else
      acdLog ("   Key Type: %d   (%s)\n", pa->Type,
		     acdKeywords[pa->Type].Name);
    acdLog ("      NAttr: %d\n", pa->NAttr);
    acdLog ("      Assoc: %s\n", ajStrBool(pa->Assoc));
    if (pa->AssocQuals)
      acdLog (" AssocQuals: %S\n", pa->AssocQuals->Name);
    else
      acdLog (" AssocQuals: <undefined>\n");
    acdLog ("    Defined: %s\n", ajStrBool(pa->Defined));
    acdLog ("Orig. Value: '%S'\n", pa->OrigStr);
    if (pa->ValStr)
      acdLog ("      Value: '%S'\n", pa->ValStr);
    else
      acdLog ("      Value: <undefined>\n");
    acdLog ("\n");
    if (pa->DefStr) {
      acdLog ("      Default Attributes:\n");
      acdListAttr (acdAttrDef, pa->DefStr, nDefAttr);
      acdLog ("\n");
    }
    acdLog ("      Attributes:\n");
    if (acdIsQtype(pa))
      acdListAttr (acdType[pa->Type].Attr, pa->AttrStr, pa->NAttr);
    else
      acdListAttr (acdKeywords[pa->Type].Attr, pa->AttrStr, pa->NAttr);
    acdLog ("\n");
    i++;
  }

  return;
}

/* @funcstatic acdListAttr ****************************************************
**
** Reports all attributes for an ACD attribute list.
**
** @param [r] attr [AcdPAttr] Attribute list
** @param [r] valstr [AjPStr*] Array of string attribute values
** @param [r] nattr [ajint] Number of attributes in list
** @return [void]
** @@
******************************************************************************/

static void acdListAttr (AcdPAttr attr, AjPStr* valstr, ajint nattr) {

  ajint i;

  if (!valstr)
    return;

  for (i=0; i < nattr; i++) {
    if (valstr[i])
      acdLog (" %15.15s: '%S'\n", attr[i].Name, valstr[i] );
    else
      acdLog (" %15.15s: <null>\n", attr[i].Name);

  }

  return;
}

/* @funcstatic acdSet *********************************************************
**
** Sets an attribute or associated qualifier value for an ACD item.
**
** All attributes, of whatever type, are treated as strings at this stage.
**
** @param [r] thys [AcdPAcd] ACD item.
** @param [r] attrib [AjPStr*] Attribute name
** @param [r] value [AjPStr] Attribute value
** @return [AjBool] ajTrue if attribute is valid.
** @cre Aborts if attribute is not found.
** @@
******************************************************************************/

static AjBool acdSet (AcdPAcd thys, AjPStr* attrib, AjPStr value) {

  ajint i;

  AcdPAttr attr = acdType[thys->Type].Attr;
  AjPStr* attrstr = thys->AttrStr;
  AcdPAcd aqual;
/*  static AjPStr defattrib=NULL; */

  ajDebug("acdSet attr '%S' val '%S' type '%s'\n", thys->Name, *attrib, value);

  i = acdFindAttr (attr, *attrib);

  if (thys->DefStr && i < 0) {	/* try again with default attributes */
    i = acdFindAttr (acdAttrDef, *attrib);
    attr = acdAttrDef;
    attrstr = thys->DefStr;
  }

  if (i >= 0) {			/* success */
    (void) ajStrAssS (&attrstr[i], value);
    (void) ajStrAssC (attrib, attr[i].Name);
  }
  else {			/* recursion with associated qualifiers */
    aqual = NULL;
    if (thys->AssocQuals)
      aqual = acdFindAssoc(thys, *attrib);
    if (!aqual)
      ajFatal ("attribute %S unknown\n", *attrib );
    /*(void) ajStrAssC(&defattrib, "default");
    return acdSet (aqual, &defattrib, value);*/
    return acdDef (aqual, value);
  }

  return ajTrue;
}

/* @funcstatic acdSetKey ******************************************************
**
** Sets an attribute for an ACD key item.
**
** All attributes, of whatever type, are treated as strings at this stage.
**
** @param [r] thys [AcdPAcd] ACD item.
** @param [r] attrib [AjPStr*] Attribute name
** @param [r] value [AjPStr] Attribute value
** @return [AjBool] ajTrue if attribute is valid.
** @cre Aborts if attribute is not found.
** @@
******************************************************************************/

static AjBool acdSetKey (AcdPAcd thys, AjPStr* attrib, AjPStr value) {

  ajint i;

  AcdPAttr attr = acdKeywords[thys->Type].Attr;
  AjPStr* attrstr = thys->AttrStr;
  AcdPAcd aqual;
  static AjPStr defattrib=NULL;

  i = acdFindAttr (attr, *attrib);

  if (thys->DefStr && i < 0) {	/* try again with default attributes */
    i = acdFindAttr (acdAttrDef, *attrib);
    attr = acdAttrDef;
    attrstr = thys->DefStr;
  }

  if (i >= 0) {			/* success */
    (void) ajStrAssS (&attrstr[i], value);
    (void) ajStrAssC (attrib, attr[i].Name);
  }
  else {			/* recursion with associated qualifiers */
    aqual = NULL;
    if (thys->AssocQuals)
      aqual = acdFindAssoc(thys, *attrib);
    if (!aqual)
      ajFatal ("attribute %S unknown\n", *attrib );
    (void) ajStrAssC(&defattrib, "default");
    return acdSet (aqual, &defattrib, value);
  }

  return ajTrue;
}

/* @funcstatic acdDef *********************************************************
**
** Sets the default value for an ACD item, and flags this in thys as Defined.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [AjPStr] Default value
** @return [AjBool] ajTrue always.
** @see acdSetDef
** @@
******************************************************************************/

static AjBool acdDef (AcdPAcd thys, AjPStr value) {

  AjPStr* attrstr = thys->DefStr;

  ajDebug ("acdDef %S '%S' %x\n", thys->Name, value, attrstr);

  (void) acdSetDef (thys, value);
  thys->Defined = ajTrue;

  return ajTrue;
}

/* @funcstatic acdSetDef ******************************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [AjPStr] Default value
** @return [AjBool] ajTrue always.
** @@
******************************************************************************/

static AjBool acdSetDef (AcdPAcd thys, AjPStr value) {

  AjPStr* attrstr = thys->DefStr;

  ajDebug ("acdSetDef %S '%S' %x\n", thys->Name, value, attrstr);

  if (!thys->DefStr)
    return ajFalse;

  (void) ajStrAssS (&attrstr[DEF_DEFAULT], value);

  return ajTrue;
}

/* @funcstatic acdSetDefC *****************************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [char *] Default value
** @return [AjBool] ajTrue always.
** @@
******************************************************************************/

static AjBool acdSetDefC (AcdPAcd thys, char* value) {

  AjPStr* attrstr = thys->DefStr;

  ajDebug ("acdSetDefC %S '%s' %x\n", thys->Name, value, attrstr);

  if (!thys->DefStr)
    return ajFalse;

  (void) ajStrAssC (&attrstr[DEF_DEFAULT], value);

  return ajTrue;
}

/* @funcstatic acdSetQualDefBool **********************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] name [char *] Qualifier name
** @param [r] value [AjBool] Default value
** @return [AjBool] ajTrue always.
** @@
******************************************************************************/

static AjBool acdSetQualDefBool (AcdPAcd thys, char* name, AjBool value) {

  AjPStr* attrstr;
  static AjPStr qname = NULL;
  AcdPAcd acd;

  (void) ajStrAssC (&qname, name);
  acd = acdFindQualAssoc (thys, qname, 0);
  if (!acd) return ajFalse;

  attrstr = acd->DefStr;

  ajDebug ("acdSetQualDefBool %S [%d] '%s' %B\n",
	   thys->Name, thys->PNum, name, value);

  if (!thys->DefStr)
    return ajFalse;

  (void) ajFmtPrintS (&attrstr[DEF_DEFAULT], "%b", value);

  return ajTrue;
}

/* @funcstatic acdSetQualDefInt **********************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] name [char *] Qualifier name
** @param [r] value [ajint] Default value
** @return [AjBool] ajTrue always.
** @@
******************************************************************************/

static AjBool acdSetQualDefInt (AcdPAcd thys, char* name, ajint value) {

  AjPStr* attrstr;
  static AjPStr qname = NULL;
  AcdPAcd acd;

  (void) ajStrAssC (&qname, name);
  acd = acdFindQualAssoc (thys, qname, 0);
  if (!acd) return ajFalse;

  attrstr = acd->DefStr;

  ajDebug ("acdSetQualDefInt %S [%d] '%s' %S [%d] %d\n",
	   thys->Name, thys->PNum, name, acd->Name, acd->PNum, value);

  if (!thys->DefStr)
    return ajFalse;

  (void) ajFmtPrintS (&attrstr[DEF_DEFAULT], "%d", value);

  return ajTrue;
}

/* @funcstatic acdSetVarDef ***************************************************
**
** Sets the default value for a variable ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [AjPStr] Default value
** @return [AjBool] ajTrue always.
** @@
**
** Note: we have to set the ValStr directly as variables have no defstr.
** Variable references are resolved later by acdSetVar
******************************************************************************/

static AjBool acdSetVarDef (AcdPAcd thys, AjPStr value) {

  ajDebug ("acdSetVarDef %S '%S' %x\n", thys->Name, value, thys->ValStr);

  (void) ajStrAssS (&thys->ValStr, value);

  return ajTrue;
}

/* @funcstatic acdFindAttr ****************************************************
**
** Locates an attribute by name in an attribute list.
**
** @param [r] attr [AcdPAttr] Attribute list
** @param [r] attrib [AjPStr] Attribute name to be found
** @return [ajint] offset in "attr" if found
** @error -1 if not found.
** @@
******************************************************************************/

static ajint acdFindAttr (AcdPAttr attr, AjPStr attrib) {

  static ajint i;
  static ajint j;
  ajint ifound=0;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  for (i=0; attr[i].Name; i++) {
    if (ajStrMatchC (attrib, attr[i].Name))
      return i;
    if (ajStrPrefixCO (attr[i].Name, attrib)) {
      ifound++;
      j = i;
      acdAmbigAppC (&ambigList, attr[i].Name);
    }
  }
  if (ifound == 1)
    return j;
  if (ifound > 1) {
    ajWarn ("ambiguous attribute %S (%S)", attrib, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }

  return -1;
}

/* @funcstatic acdFindAttrC ***************************************************
**
** Locates an attribute by name in an attribute list.
**
** @param [r] attr [AcdPAttr] Attribute list
** @param [r] attrib [char*] Attribute name to be found
** @return [ajint] offset in "attr" if found
** @error -1 if not found.
** @@
******************************************************************************/

static ajint acdFindAttrC (AcdPAttr attr, char* attrib) {

  static ajint i;
  static ajint j;
  ajint k = strlen(attrib);
  ajint ifound=0;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  for (i=0; attr[i].Name; i++) {
    if (!strncmp (attr[i].Name, attrib, k)) {
      if (!strcmp (attr[i].Name, attrib))
	return i;
      ifound++;
      j = i;
      acdAmbigAppC (&ambigList, attr[i].Name);
    }
  }
  if (ifound == 1)
    return j;
  if (ifound > 1) {
    ajWarn ("ambiguous attribute %s (%S)", attrib, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }

  return -1;
}

/* @funcstatic acdProcess *****************************************************
**
** Steps through all the ACD items, filling in missing information.
** Parameters are definied in the default attributes. The parameter
** number is generated here in the order they are found.
**
** @return [void]
** @@
******************************************************************************/

static void acdProcess (void) {

  AcdPAcd pa;
  AcdPAcd qa = NULL;
  static AjPStr reqstr = NULL;
  static AjPStr yesstr = NULL;
  AjBool isreq;
  AjBool isparam;

  if (!reqstr) {
    (void) ajStrAssC(&reqstr, "required");
    (void) ajStrAssC(&yesstr, "Y");
  }

  for (pa=acdList; pa; pa=pa->Next) {
    if (pa->DefStr)
      (void) ajStrAssS (&pa->OrigStr, pa->DefStr[DEF_DEFAULT]);

    if (pa->DefStr && acdAttrToBool(pa, "parameter", ajFalse, &isparam)) {
      if (isparam) {
	acdNParam++;
	pa->PNum = acdNParam;
	pa->Level = ACD_PARAM;
	if (!acdAttrToBool(pa, "required", ajFalse, &isreq)) {
	  (void) acdSet(pa, &reqstr, yesstr);
	}
	qa = pa->AssocQuals;
	if (qa) {
	  while (qa->Assoc) {
	    qa->PNum = acdNParam;
	    qa = qa->Next;
	  }
	}
      }
    }
  }

  
  return;
   
}

/* @funcstatic acdSetAll ******************************************************
**
** Steps through all the ACD items, calling the acdSet... function
** for each in turn to prompt the user for any missing values and
** to check that all is OK.
**
** @return [void]
** @@
******************************************************************************/

static void acdSetAll (void) {
  AcdPAcd pa;

  ajint i = 0;

  for (pa=acdList; pa; pa=pa->Next) {
    if (acdIsQtype(pa))
      acdType[pa->Type].Set (pa);
    else
      acdKeywords[pa->Type].Set (pa);
    i++;
  }

  return;
}

/* @funcstatic acdQualToBool **************************************************
**
** Converts an associated qualifier value into a boolean.
** Any variable references are resolved at this stage.
**
** @param [r] thys [AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [char*]Qualifier name
** @param [r] defval [AjBool] default value
** @param [w] result [AjBool*] Resulting value.
** @param [w] valstr [AjPStr*] Resulting value as a string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdQualToBool (AcdPAcd thys, char *qual,
			     AjBool defval, AjBool *result, AjPStr* valstr) {

  AjBool ret;

  ret = acdGetValueAssoc (thys, qual, valstr);
  acdLog ("acdQualToBool item: %S qual: %s defval: %B str: '%S', ret: %B\n",
	  thys->Name, qual, defval, *valstr, ret);
  if (ret) {
    (void) acdVarResolve (valstr);
    acdLog ("resolved to: '%S'\n", *valstr);

    if (ajStrLen(*valstr)) {
      if (!ajStrToBool(*valstr, result)) {
	ajFatal ("%S: Bad associated qualifier boolean value -%s = %S\n",
		 thys->Name , qual, *valstr) ;
      }
      return ajTrue;
    }
  }

  *result = defval;
  (void) ajFmtPrintS(valstr, "%b", defval);
  return ajFalse;
}

/* @funcstatic acdQualToFloat *************************************************
**
** Converts an associated qualifier value into a floating point number.
** Any variable references are resolved at this stage.
**
** @param [r] thys [AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [char*]Qualifier name
** @param [r] defval [float] default value
** @param [r] precision [ajint] floating point precision
** @param [w] result [float*] Resulting value.
** @param [w] valstr [AjPStr*] Resulting value as a string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdQualToFloat (AcdPAcd thys, char *qual,
			      float defval, ajint precision,
			      float *result, AjPStr* valstr) {

  AjBool ret;

  ret = acdGetValueAssoc (thys, qual, valstr);
  acdLog ("acdQualToFloat item: %S qual: %s defval: %.3f str: '%S' ret: %B\n",
	  thys->Name, qual, defval, *valstr, ret);
  if (ret) {
    (void) acdVarResolve (valstr);
    acdLog ("resolved to: '%S'\n", *valstr);


    if (ajStrLen(*valstr)) {
      if (!ajStrToFloat(*valstr, result)) {
	ajFatal ("%S: Bad associated qualifier float value -%s = %S\n",
		 thys->Name, qual, *valstr) ;
      }
      return ajTrue;
    }
  }

  *result = defval;
  (void) ajStrFromFloat(valstr, defval, precision);
  return ajFalse;
}


/* @funcstatic acdQualToInt ***************************************************
**
** Converts an associated qualifier value into an integer.
** Any variable references are resolved at this stage.
**
** @param [r] thys [AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [char*] Qualifier name
** @param [r] defval [ajint] default value
** @param [w] result [ajint*] Resulting value.
** @param [wP] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdQualToInt (AcdPAcd thys, char *qual,
			    ajint defval, ajint *result, AjPStr* valstr) {


  AjBool ret;

  ret = acdGetValueAssoc (thys, qual, valstr);
  acdLog ("acdQualToInt item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	  thys->Name, qual, defval, *valstr, ret);
  if (ret) {
    (void) acdVarResolve (valstr);
    acdLog ("resolved to: '%S'\n", *valstr);

    if (ajStrLen(*valstr)) {
      if (ajStrMatchC(*valstr, "default"))
	(void) ajStrAssC(valstr, "0");
      if (!ajStrToInt(*valstr, result)) {
	ajFatal ("%S: Bad associated qualifier integer value -%s = %S\n",
		 thys->Name, qual, *valstr);
      }
      return ajTrue;
    }
  }

  *result = defval;
  (void) ajStrFromInt(valstr, defval);
  return ajFalse;
}

/* @funcstatic acdQualToSeqbegin **********************************************
**
** Converts an associated qualifier value into an integer, or the
** string "begin".
**
** Any variable references are resolved at this stage.
**
** @param [r] thys [AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [char*] Qualifier name
** @param [r] defval [ajint] default value
** @param [w] result [ajint*] Resulting value.
** @param [wP] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdQualToSeqbegin (AcdPAcd thys, char *qual,
			       ajint defval, ajint *result, AjPStr* valstr) {


  AjBool ret;

  ret = acdGetValueAssoc (thys, qual, valstr);
  acdLog ("acdQualToSeqpos item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	  thys->Name, qual, defval, *valstr, ret);
  if (ret) {
    (void) acdVarResolve (valstr);
    acdLog ("resolved to: '%S'\n", *valstr);

    if (ajStrLen(*valstr)) {
      if (!ajStrMatchCaseC(*valstr, "default")) {
	if (!ajStrToInt(*valstr, result)) {
	  ajFatal ("%S: Bad associated qualifier integer value -%s = %S\n",
		   thys->Name, qual, *valstr);
	}
      }
      acdLog ("return value %B '%S'\n", ajTrue, *valstr);
      return ajTrue;
    }
  }

  *result = defval;
  if (!defval) {
    (void) ajStrAssC (valstr, "start");
  }
  else {
    (void) ajStrFromInt(valstr, defval);
  }

  acdLog ("return default %B '%S'\n", ajFalse, *valstr);
  return ajFalse;
}

/* @funcstatic acdQualToSeqend ************************************************
**
** Converts an associated qualifier value into an integer, or the
** string "end".
**
** Any variable references are resolved at this stage.
**
** @param [r] thys [AcdPAcd] ACD item of master parameter or qualifier.
** @param [r] qual [char*] Qualifier name
** @param [r] defval [ajint] default value
** @param [w] result [ajint*] Resulting value.
** @param [wP] valstr [AjPStr*] Qualifier value as a string
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdQualToSeqend (AcdPAcd thys, char *qual,
			       ajint defval, ajint *result, AjPStr* valstr) {


  AjBool ret;

  ret = acdGetValueAssoc (thys, qual, valstr);
  acdLog ("acdQualToSeqpos item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	  thys->Name, qual, defval, *valstr, ret);
  if (ret) {
    (void) acdVarResolve (valstr);
    acdLog ("resolved to: '%S'\n", *valstr);

    if (ajStrLen(*valstr)) {
      if (!ajStrMatchCaseC(*valstr, "default")) {
	if (!ajStrToInt(*valstr, result)) {
	  ajFatal ("%S: Bad associated qualifier integer value -%s = %S\n",
		   thys->Name, qual, *valstr);
	}
      }
      acdLog ("return value %B '%S'\n", ajTrue, *valstr);
      return ajTrue;
    }
  }

  *result = defval;
  if (!defval) {
    (void) ajStrAssC (valstr, "end");
  }
  else {
    (void) ajStrFromInt(valstr, defval);
  }

  acdLog ("return default %B '%S'\n", ajFalse, *valstr);
  return ajFalse;
}

/* @funcstatic acdAttrToBool **************************************************
**
** Resolves and tests an attribute string. If it has a boolean value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attr [char*] Attribute name
** @param [r] defval [AjBool] Default value
** @param [w] result [AjBool*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

static AjBool acdAttrToBool (AcdPAcd thys,
  char *attr, AjBool defval, AjBool *result) {
  ajint i;
  static AjPStr str = NULL;

  (void) acdAttrResolve (thys, attr, &str);

  if (ajStrLen(str)) {
    if (ajStrToBool(str, result)) {
      (void) ajStrDelReuse(&str);
      return ajTrue;
    }
    if (ajStrToInt(str, &i)) {
      if (i) *result = ajTrue;
      else *result = ajFalse;
      (void) ajStrDelReuse(&str);
      return ajTrue;
    }
    else {
      ajFatal ("%S: Bad attribute boolean value %s = %S\n",
	       thys->Name, attr, str);
    }
  }

  *result = defval;
  (void) ajStrDelReuse(&str);

  return ajFalse;
}

/* @funcstatic acdAttrToFloat *************************************************
**
** Resolves and tests an attribute string. If it has a float value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attr [char*] Attribute name
** @param [r] defval [float] Default value
** @param [w] result [float*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

static AjBool acdAttrToFloat (AcdPAcd thys,
  char *attr, float defval, float *result) {

  static AjPStr str= NULL;

  (void) acdAttrResolve (thys, attr, &str);

  if (ajStrLen(str)) {
    if (ajStrToFloat(str, result)) {
      (void) ajStrDelReuse(&str);
      return ajTrue;
    }
    else {
      ajFatal ("%S: Bad attribute float value %s = %S\n",
	       thys->Name, attr, str);
    }
  }

  *result = defval;
  (void) ajStrDelReuse(&str);

  return ajFalse;
}

/* @funcstatic acdAttrToInt ***************************************************
**
** Resolves and tests an attribute string. If it has an integer value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attr [char*] Attribute name
** @param [r] defval [ajint] Default value
** @param [w] result [ajint*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

static AjBool acdAttrToInt (AcdPAcd thys,
  char *attr, ajint defval, ajint *result) {

  static AjPStr str = NULL;

  (void) acdAttrResolve (thys, attr, &str);

  if (ajStrLen(str)) {
    if (ajStrToInt(str, result)) {
      (void) ajStrDelReuse(&str);
      return ajTrue;
    }
    else {
      ajFatal ("%S: Bad attribute integer value %s = %S\n",
	       thys->Name, attr, str);
    }
  }

  *result = defval;
  (void) ajStrDelReuse(&str);

  return ajFalse;
}

/* @funcstatic acdAttrToStr ***************************************************
**
** Resolves an attribute to a string with translation of variable name(s).
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attr [char*] Attribute name
** @param [r] defval [char*] Default value
** @param [w] result [AjPStr*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

static AjBool acdAttrToStr (AcdPAcd thys,
  char *attr, char* defval, AjPStr *result) {

  if (acdAttrResolve(thys, attr, result))
    return ajTrue;

  (void) ajStrAssC (result, defval);

  return ajFalse;
}

/* @funcstatic acdAttrResolve *************************************************
**
** Resolves an attribute to a string with translation of variable name(s).
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attr [char*] Attribute name
** @param [w] result [AjPStr*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

static AjBool acdAttrResolve (AcdPAcd thys, char *attr, AjPStr *result) {

  (void) ajStrAssS(result, acdAttrValue (thys, attr));
  (void) acdVarResolve (result);

  if (ajStrLen(*result))
    return ajTrue;

  return ajFalse;
}

/* #funcstatic acdDefattrToInt ************************************************
**
** Resolves and tests an attribute string. If it has an integer value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** #param [r] thys [AcdPAcd] ACD item
** #param [r] attr [char*] Attribute name
** #param [r] defval [ajint] Default value
** #param [w] result [ajint*] Resulting value.
** #return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

/*static AjBool acdDefattrToInt (AcdPAcd thys,
  char *attr, ajint defval, ajint *result) {

  static AjPStr str = NULL;

  (void) acdAttrResolve (thys, attr, &str);

  if (ajStrLen(str)) {
    if (ajStrToInt(str, result)) {
      (void) ajStrDelReuse(&str);
      return ajTrue;
    }
    else {
      ajFatal ("%S: Bad attribute integer value %s = %S\n",
	       thys->Name, attr, str);
    }
  }

  *result = defval;
  (void) ajStrDelReuse(&str);

  return ajFalse;
}*/

/* @funcstatic acdVarResolve **************************************************
**
** Resolves any variable or function references in a string.
**
** First resolves variables in the form $(name) or $(name.attribute).
** This cunningly resolves internal () pairs.
**
** Then looks for function references and resolves them.
**
** @param [uP] var [AjPStr*] String value
** @return [AjBool] Always ajTrue so far
** @@
******************************************************************************/

static AjBool acdVarResolve (AjPStr* var) {

  static AjPStr varname=NULL;
  static AjPStr attrname=NULL;
  static AjPStr result = NULL;
  static AjPStr token = NULL;
  ajint ivar=0;
  ajint ifun=0;
  static AjPRegexp varexp = NULL;
  static AjPRegexp funexp = NULL;
  static AjPStr newvar = NULL;
  static AjPStr restvar = NULL;
  static AjPStr savein = NULL;

  if (!varexp) varexp = ajRegCompC("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");
  if (!funexp) funexp = ajRegCompC("^(.*)\\@\\(([^()]+)\\)");

  /* resolve variable references first to resolve internal parentheses */
  if (!var) {
    (void) ajStrAssC(var, "");
    return ajTrue;
  }

  (void) ajStrAssS(&savein, *var);

  while (ajRegExec (varexp, *var)) {
    ivar++;
    ajRegSubI(varexp, 2, &token); /* variable name */
    (void) acdVarSplit (token, &varname, &attrname);
    (void) acdGetAttr (&result, varname, attrname);

    ajRegSubI(varexp, 1, &newvar);
    (void) ajStrApp (&newvar, result);
    if (ajRegPost(varexp, &restvar)) /* any more? */
      (void) ajStrApp (&newvar, restvar);
    (void) ajStrAssS (var, newvar);
  }

  /* now resolve any function */

  while (ajRegExec (funexp, *var) ) {
    ifun++;
    ajRegSubI(funexp, 2, &token); /* function statement */
    (void) acdFunResolve (&result, token);
    ajRegSubI(funexp, 1, &newvar);
    (void) ajStrApp (&newvar, result);
    if (ajRegPost(funexp, &restvar)) /* any more? */
      (void) ajStrApp (&newvar, restvar);
    (void) ajStrAssS (var, newvar);
  }

  if (ivar > 1)
    ajDebug("Recursive variables in '%S'\n", savein);

  if (ifun > 1)
    ajDebug("Recursive expressions in '%S'\n", savein);

  (void) ajStrDelReuse(&savein);
  (void) ajStrDelReuse(&token);
  (void) ajStrDelReuse(&result);
  (void) ajStrDelReuse(&attrname);
  (void) ajStrDelReuse(&varname);
  (void) ajStrDelReuse(&newvar);
  (void) ajStrDelReuse(&restvar);

  return ajFalse;
}

/* @funcstatic acdHelpVarResolve **********************************************
**
** Resolves any variable or function references in a string if clearly
** defined, otherwise returns ajFalse and sets the string to "". For
** use with strings that cannot be resolved in help processing
** because of functions, variable dependencies etc.
**
** @param [uP] str [AjPStr*] String value
** @param [uP] var [AjPStr] Source string value
** @return [AjBool] ajTrue if it could be resolved cleanly
** @@
******************************************************************************/

static AjBool acdHelpVarResolve (AjPStr* str, AjPStr var) {

  static AjPRegexp varexp = NULL;
  static AjPRegexp funexp = NULL;

  if (!varexp) varexp = ajRegCompC("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");
  if (!funexp) funexp = ajRegCompC("^(.*)\\@\\(([^()]+)\\)");

  if (!var) {
    (void) ajStrAssC(str, "");
    return ajTrue;
  }

   /* reject variable references first to resolve internal parentheses */
  if (ajRegExec (varexp, var)) {
    (void) ajStrAssC (str, "");
    return ajFalse;
  }

  /* reject any function */
  if (ajRegExec (funexp, var)) {
    (void) ajStrAssC (str, "");
    return ajFalse;
  }

  (void) ajStrAssS (str, var);
  return ajTrue;
}

/* @funcstatic acdFunResolve **************************************************
**
** Resolves a function reference.
**
** Has a list of all accepted function syntax.
**
** @param [uP] result [AjPStr*] Result returned
** @param [P] str [AjPStr] Function statement input
** @return [AjBool] Always ajTrue so far
** @@
******************************************************************************/

static AjBool acdFunResolve (AjPStr* result, AjPStr str) {
  ajint i;

  ajDebug ("acdFunResolve '%S'\n", str);

  for (i = 0; explist[i].Name; i++) {
    /* ajDebug ("try using '%s'\n", explist[i].Name); */
    if (explist[i].Func (result, str)) {
      ajDebug ("resolved '%S' using '%s'\n", str, explist[i].Name);
      ajDebug ("  result '%S'\n", *result);
      return ajTrue;
    }
  }

  ajWarn("ACD expression invalid @(%S)\n", str);
  acdLog ("@(%S) *failed**\n", str);

  (void) ajStrAssS(result, str);

  return ajFalse;
}

/* @funcstatic acdExpPlus ****************************************************
**
** Looks for and resolves an expression @( num + num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpPlus (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;

  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[+][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    (void) ajFmtPrintS (result, "%d", ia+ib);
    acdLog ("ia: %d + ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[+][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    (void) ajFmtPrintS (result, "%f", da+db);
    acdLog ("da: %f + db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }
  return ajFalse;
}

/* @funcstatic acdExpMinus ***************************************************
**
** Looks for and resolves an expression @( num - num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpMinus (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;

  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[-][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    (void) ajFmtPrintS (result, "%d", ia-ib);
    acdLog ("ia: %d - ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[-][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    (void) ajFmtPrintS (result, "%f", da-db);
    acdLog ("da: %f - db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }
  return ajFalse;
}

/* @funcstatic acdExpStar ****************************************************
**
** Looks for and resolves an expression @( num * num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpStar (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;

  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[*][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    (void) ajFmtPrintS (result, "%d", ia*ib);
    acdLog ("ia: %d * ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[*][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    (void) ajFmtPrintS (result, "%f", da*db);
    acdLog ("da: %f * db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }
  return ajFalse;
}

/* @funcstatic acdExpDiv ****************************************************
**
** Looks for and resolves an expression @( num / num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpDiv (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;

  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[/][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    (void) ajFmtPrintS (result, "%d", ia/ib);
    acdLog ("ia: %d / ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[/][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    (void) ajFmtPrintS (result, "%f", da/db);
    acdLog ("da: %f / db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }
  return ajFalse;
}

/* @funcstatic acdExpNot ****************************************************
**
** Looks for and resolves an expression @(! bool ) or @(NOT bool)
** or @(not bool). An invalid bool value is treated as false,
** so it will return a true value.
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpNot (AjPStr* result, AjPStr str) {

  AjBool ba;
  static AjPRegexp nexp = NULL;

  if (!nexp)			/* ajint + ajint */
    nexp = ajRegCompC("^[ \t]*(!|[Nn][Oo][Tt])[ \t]*([A-Za-z0-9]+)[ \t]*$");

  if (ajRegExec (nexp, str) ) {
    ajDebug("nexp matched  '%S'\n", str);
    ajRegSubI(nexp, 2, &acdExpTmpstr);
    if (!ajStrToBool(acdExpTmpstr, &ba)) {
      acdLog ("invalid bool value '%S' in acdExpNot\n", acdExpTmpstr);
      ba = ajFalse;
    }
    if (ba)
      (void) ajFmtPrintS (result, "%b", ajFalse);
    else
      (void) ajFmtPrintS (result, "%b", ajTrue);
    acdLog ("ta: ! '%S' = '%S'\n", acdExpTmpstr, *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpEqual ****************************************************
**
** Looks for and resolves an expression @( num == num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpEqual (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;
  static AjPStr tmpstr2 = NULL;
  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;
  static AjPRegexp texp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[=][=][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    if (ia == ib)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ia: %d == ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float == float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[=][=][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    if (da == db)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("da: %f == db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }

  if (!texp)			/* string == string */
    texp = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[=][=][ \t]*"
			  "([^ \t]+)[ \t]*$");

  if (ajRegExec (texp, str) ) {
    ajDebug("texp matched  '%S'\n", str);
    ajRegSubI(texp, 1, &acdExpTmpstr);
    ajRegSubI(texp, 2, &tmpstr2);
    if (ajStrMatchCase(acdExpTmpstr, tmpstr2))
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ta: '%S' == tb: '%S' = '%S'\n", acdExpTmpstr, tmpstr2,
		   *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpNotEqual *************************************************
**
** Looks for and resolves an expression @( num != num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpNotEqual (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;
  static AjPStr tmpstr2 = NULL;
  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;
  static AjPRegexp texp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[!][=][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    if (ia != ib)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ia: %d != ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[!][=][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    if (da != db)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("da: %f != db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }

  if (!texp)			/* float + float */
    texp = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[!][=][ \t]*"
			  "([^ \t]+)[ \t]*$");

  if (ajRegExec (texp, str) ) {
    ajDebug("texp matched  '%S'\n", str);
    ajRegSubI(texp, 1, &acdExpTmpstr);
    ajRegSubI(texp, 2, &tmpstr2);
    if (!ajStrMatchCase(acdExpTmpstr, tmpstr2))
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ta: '%S' != tb: '%S' = '%S'\n", acdExpTmpstr, tmpstr2, *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpGreater **************************************************
**
** Looks for and resolves an expression @( num > num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpGreater (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;
  static AjPStr tmpstr2 = NULL;
  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;
  static AjPRegexp texp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[>][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    if (ia > ib)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ia: %d > ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[>][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    if (da > db)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("da: %f > db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }

  if (!texp)			/* float + float */
    texp = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[>][ \t]*"
			  "([^ \t]+)[ \t]*$");

  if (ajRegExec (texp, str) ) {
    ajDebug("texp matched  '%S'\n", str);
    ajRegSubI(texp, 1, &acdExpTmpstr);
    ajRegSubI(texp, 2, &tmpstr2);
    if (0 > ajStrCmpCase(tmpstr2, acdExpTmpstr))
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ta: '%S' > tb: '%S' = '%S'\n", acdExpTmpstr, tmpstr2,
		   *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpLesser ***************************************************
**
** Looks for and resolves an expression @( num < num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpLesser (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;
  static AjPStr tmpstr2 = NULL;
  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;
  static AjPRegexp texp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[<][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    if (ia < ib)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ia: %d < ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[<][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    if (da < db)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("da: %f < db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }

  if (!texp)			/* float + float */
    texp = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[<][ \t]*"
			  "([^ \t]+)[ \t]*$");

  if (ajRegExec (texp, str) ) {
    ajDebug("texp matched  '%S'\n", str);
    ajRegSubI(texp, 1, &acdExpTmpstr);
    ajRegSubI(texp, 2, &tmpstr2);
    if (0 < ajStrCmpCase(tmpstr2, acdExpTmpstr))
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ta: '%S' < tb: '%S' = '%S'\n", acdExpTmpstr, tmpstr2,
		   *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpOr ***************************************************
**
** Looks for and resolves an expression @( num | num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpOr (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;
  AjBool ba, bb;
  static AjPStr tmpstr2 = NULL;
  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;
  static AjPRegexp texp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[|][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    if (ia || ib)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ia: %d | ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[|][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    if (da || db)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("da: %f | db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }

  if (!texp)			/* char + char */
    texp = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[|][ \t]*"
			  "([^ \t]+)[ \t]*$");

  if (ajRegExec (texp, str) ) {
    ajDebug("texp matched  '%S'\n", str);
    ajRegSubI(texp, 1, &acdExpTmpstr);
    ajRegSubI(texp, 2, &tmpstr2);
    (void) ajStrToBool(tmpstr2,&ba);
    (void) ajStrToBool(acdExpTmpstr, &bb);
    if ( ba || bb )
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ta: '%S' | tb: '%S' = '%S'\n", acdExpTmpstr, tmpstr2,
		   *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpAnd ***************************************************
**
** Looks for and resolves an expression @( num & num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpAnd (AjPStr* result, AjPStr str) {

  ajint ia, ib;
  double da, db;
  AjBool ba, bb;
  static AjPStr tmpstr2 = NULL;
  static AjPRegexp iexp = NULL;
  static AjPRegexp dexp = NULL;
  static AjPRegexp texp = NULL;

  if (!iexp)			/* ajint + ajint */
    iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[&][ \t]*"
			 "([0-9+-]+)[ \t]*$");

  if (ajRegExec (iexp, str) ) {
    ajDebug("iexp matched  '%S'\n", str);
    ajRegSubI(iexp, 1, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ia);
    ajRegSubI(iexp, 2, &acdExpTmpstr);
    (void) ajStrToInt(acdExpTmpstr, &ib);
    if (ia && ib)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ia: %d & ib: %d = '%S'\n", ia, ib, *result);

    return ajTrue;
  }

  if (!dexp)			/* float + float */
    dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[&][ \t]*"
			  "([0-9.+-]+)[ \t]*$");
  if (ajRegExec (dexp, str) ) {
    ajDebug("dexp matched  '%S'\n", str);
    ajRegSubI(dexp, 1, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &da);
    ajRegSubI(dexp, 2, &acdExpTmpstr);
    (void) ajStrToDouble(acdExpTmpstr, &db);
    if (da && db)
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("da: %f & db: %f = '%S'\n", da, db, *result);

    return ajTrue;
  }

  if (!texp)			/* char + char */
    texp = ajRegCompC("^[ \t]*([^ \t]+)[ \t]*[&][ \t]*"
			  "([^ \t]+)[ \t]*$");

  if (ajRegExec (texp, str) ) {
    ajDebug("texp matched  '%S'\n", str);
    ajRegSubI(texp, 1, &acdExpTmpstr);
    ajRegSubI(texp, 2, &tmpstr2);
    (void) ajStrToBool(tmpstr2,&ba);
    (void) ajStrToBool(acdExpTmpstr, &bb);
    if ( ba && bb )
      (void) ajFmtPrintS (result, "%b", ajTrue);
    else
      (void) ajFmtPrintS (result, "%b", ajFalse);
    acdLog ("ta: '%S' & tb: '%S' = '%S'\n", acdExpTmpstr, tmpstr2,
		   *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpCond ****************************************************
**
** Looks for and resolves an expression @( bool ? trueval : falseval )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpCond (AjPStr* result, AjPStr str) {

  AjBool ba;
  static AjPRegexp condexp = NULL;

  if (!condexp)			/* bool ? iftrue : iffalse */
    condexp = ajRegCompC("^[ \t]*([.A-Za-z0-9+-]*)[ \t]*[?]"
			 "[ \t]*([^: \t]+)[ \t]*[:]"
			 "[ \t]*([^: \t]+)[ \t]*$");

  if (ajRegExec (condexp, str) ) {
    ajRegSubI(condexp, 1, &acdExpTmpstr);
    (void) ajStrToBool(acdExpTmpstr, &ba);
    if (ba)
      ajRegSubI(condexp, 2, result);
    else
      ajRegSubI(condexp, 3, result);

    acdLog ("ba: %B = '%S'\n", ba, *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpCase ****************************************************
**
** Looks for and resolves an expression as a switch/case statement
** @( var = casea : vala, caseb: valb else: val )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpCase (AjPStr* result, AjPStr str) {

  ajint ifound;
  AjBool todo;

  static AjPStr testvar = NULL;
  static AjPStr restvar = NULL;
  static AjPStr elsevar = NULL;
  static AjPRegexp caseexp = NULL;
  static AjPRegexp listexp = NULL;

  if (!caseexp)			/* value = (case : value,  ...) */
    caseexp = ajRegCompC("^[ \t]*([A-Za-z0-9+-]+)[ \t]*[=]");
  if (!listexp)			/* case : value */
    listexp = ajRegCompC("^[ \t]*([^: \t]+)[ \t]*[:]+"
			 "[ \t]*([^: \t,]+)[ \t,]*");

  if (ajRegExec (caseexp, str) ) {
    ajRegSubI(caseexp, 1, &testvar);

    if (!ajRegPost(caseexp, &restvar)) /* any more? */
      return ajFalse;

    (void) ajStrAssC (&elsevar, "");
    todo = ajTrue;
    ifound = 0;
    while (todo && ajRegExec (listexp, restvar)) {
      ajRegSubI(listexp, 1, &acdExpTmpstr);
      if (ajStrMatchC (acdExpTmpstr, "else"))  /* default */
	ajRegSubI(listexp, 2, &elsevar);
      if (ajStrMatch (acdExpTmpstr, testvar)) { /* match, take the value */
	ajRegSubI(listexp, 2, result);
	acdLog ("%S == %S : '%S'\n", testvar, acdExpTmpstr, *result);
	return ajTrue;
      }
      if (ajStrPrefix (testvar, acdExpTmpstr)) {
	ifound++;
	ajRegSubI(listexp, 2, result);
      }
      todo = ajRegPost(listexp, &restvar);
    }

    if (ifound) {		/* let ambiguous matches through */
      if (ifound > 1) {
	acdLog ("@(=) ambiguous match, last match accepted %S\n",
		       testvar);
	ajDebug ("@(=) ambiguous match, last match accepted %S\n", testvar);
      }
      acdLog ("%S ~= %S : '%S'\n", testvar, acdExpTmpstr, *result);
      return ajTrue;
    }
    if (ifound == 0) {
      (void) ajStrAssS (result, elsevar);
      acdLog ("%S != else : '%S'\n", testvar, *result);
      return ajTrue;
    }
  }

  return ajFalse;
}

/* @funcstatic acdExpFilename *************************************************
**
** Looks for an expression @(filename string) and returns a trimmed
** lower case file name prefix or suffix.
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpFilename (AjPStr* result, AjPStr str) {

  static AjPRegexp filexp = NULL;

  if (!filexp)			/* file: name */
    filexp = ajRegCompC("^[ \t]*[Ff][Ii][Ll][Ee]:[ \t]*([^ \t]+)[ \t]*$");

  if (ajRegExec (filexp, str) ) {
    ajDebug("filexp matched  '%S'\n", str);
    ajRegSubI(filexp, 1, &acdExpTmpstr);
    (void) ajStrAssS (result, acdExpTmpstr);
    (void) ajFileNameShorten (result);
    (void) ajStrToLower(result);
    acdLog ("file: %S = '%S'\n", acdExpTmpstr, *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdExpExists *************************************************
**
** Looks for an expression @(is string) and returns ajTrue
** if there is a value, and ajFalse if there is none
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpExists (AjPStr* result, AjPStr str) {

  static AjPRegexp filexp = NULL;
  AjBool test;
  if (!filexp)			/* file: name */
    filexp = ajRegCompC("^[ \t]*[Ii][Ss]:[ \t]*([^ \t]*)[ \t]*$");

  if (ajRegExec (filexp, str) ) {
    ajDebug("filexp matched  '%S'\n", str);
    ajRegSubI(filexp, 1, &acdExpTmpstr);
    if (ajStrLen(acdExpTmpstr))
      test = ajTrue;
    else
      test = ajFalse;
    (void) ajFmtPrintS(result, "%b", test);
    acdLog ("test: '%S' = '%S'\n", acdExpTmpstr, *result);

    return ajTrue;
  }

  return ajFalse;
}

/* @funcstatic acdVarSplit ****************************************************
**
** Splits a variable reference into name and attribute.
** Attribute is "default" if not specified
**
** @param [r] var [AjPStr] Variable reference
** @param [wP] name [AjPStr*] Variable name
** @param [wP] attrname [AjPStr*] Attribute name, or "default" if not set.
** @return [AjBool] ajTrue if successfully split
** @@
******************************************************************************/

static AjBool acdVarSplit (AjPStr var, AjPStr* name, AjPStr* attrname) {

  ajint i;

  (void) ajStrAssS (name, var);
  i = ajStrFindC(*name, ".");	/* qualifier with value */
  if (i > 0) {
    (void) ajStrAssS(attrname, var);
    (void) ajStrSub(name, 0, i-1);	/* strip any value and keep testing */
    (void) ajStrTrim(attrname, i+1);
  }
  else {
    (void) ajStrDelReuse(attrname);
  }
  
  return ajTrue;
}

/* @funcstatic acdAttrValue ***************************************************
**
** Returns the string value for a named attribute
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attrib [char*] Attribute name
** @return [AjPStr] Attribute value.
** @cre Aborts if attribute is not found.
** @@
******************************************************************************/

static AjPStr acdAttrValue (AcdPAcd thys, char *attrib) {

  AcdPAttr attr;
  AjPStr* attrstr = thys->AttrStr;
  AcdPAttr defattr = acdAttrDef;
  AjPStr* defstr = thys->DefStr;
  ajint i;

  if (acdIsQtype(thys))
    attr = acdType[thys->Type].Attr;
  else
    attr = acdKeywords[thys->Type].Attr;
      
  i = acdFindAttrC (attr, attrib);
  if (i >= 0)
    return attrstr[i];

  if (thys->DefStr) {
    i = acdFindAttrC (defattr, attrib);
    if (i >= 0)
      return defstr[i];
  }
  if (i < 0)
    ajFatal ("unknown attribute %s\n", attrib);
  return NULL;
}

/* @funcstatic acdAttrValueStr ************************************************
**
** Returns the string value for a named attribute
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attrib [char*] Attribute name
** @param [r] def [char*] Default value
** @param [w] str [AjPStr*] Attribute value
** @return [AjBool] ajTrue if success.
** @cre Aborts if attribute is not found.
** @@
******************************************************************************/

static AjBool acdAttrValueStr (AcdPAcd thys, char *attrib, char* def,
			       AjPStr *str) {

  AcdPAttr attr;
  AjPStr* attrstr = thys->AttrStr;
  AcdPAttr defattr = acdAttrDef;
  AjPStr* defstr = thys->DefStr;
  ajint i;

  if (acdIsQtype(thys))
    attr = acdType[thys->Type].Attr;
  else
    attr = acdKeywords[thys->Type].Attr;
      
  i = acdFindAttrC (attr, attrib);
  if (i >= 0) {
    ajStrAssS (str, attrstr[i]);
    if (ajStrLen(*str))
      return ajTrue;
    ajStrAssC (str, def);
    return ajFalse;
  }

  if (thys->DefStr) {
    i = acdFindAttrC (defattr, attrib);
    if (i >= 0) {
      ajStrAssS (str, defstr[i]);
      if (ajStrLen(*str))
	return ajTrue;
      ajStrAssC (str, def);
      return ajFalse;
    }
  }
  if (i < 0)
    ajFatal ("unknown attribute %s\n", attrib);
  return ajFalse;
}

/* @funcstatic acdArgsScan ****************************************************
**
** Steps through the command line and checks for special qualifiers.
** Sets special internal variables to reflect their presence.
**
** Currently these are "-debug", "-acdlog", "-stdout", "-filter",
** "-help" and "-auto".
**
** @param [r] argc [ajint] Number of arguments
** @param [r] argv [char* []] Actual arguments as a text array.
** @return [void]
** @@
******************************************************************************/

static void acdArgsScan (ajint argc, char *argv[]) {

  ajint i;

  for (i=0; i < argc; i++) {
    if (!strcmp(argv[i], "-debug")) {
      acdDebug = ajTrue;
      acdDebugSet = ajTrue;
    }
    if (!strcmp(argv[i], "-nodebug")) {
      acdDebug = ajFalse;
      acdDebugSet = ajTrue;
    }
    if (!strcmp(argv[i], "-stdout"))   acdStdout = ajTrue;
    if (!strcmp(argv[i], "-filter"))   acdFilter = ajTrue;
    if (!strcmp(argv[i], "-options"))  acdOptions = ajTrue;
    if (!strcmp(argv[i], "-verbose"))  acdVerbose = ajTrue;
    if (!strcmp(argv[i], "-help"))     acdDoHelp = ajTrue;
    if (!strcmp(argv[i], "-auto"))     acdAuto = ajTrue;
    if (!strcmp(argv[i], "-acdlog"))   acdDoLog = ajTrue;
    if (!strcmp(argv[i], "-acdpretty"))  acdDoPretty = ajTrue;
    if (!strcmp(argv[i], "-acdtable")) acdTable = ajTrue;

    if (!strcmp(argv[i], "-help")) {
	ajDebug("acdArgsScan -help argv[%d]\n", i);
    }
  }
  ajDebug ("acdArgsScan acdDebug %B acdDoHelp %B\n", acdDebug, acdDoHelp);

  return;
}
/* @funcstatic acdArgsParse ***************************************************
**
** Steps through the command line and compares to the stored command structure.
** Capable of cunning tricks such as matching values to qualifiers if they fit,
** and otherwise treating them as parameters.
**
** @param [r] argc [ajint] Number of arguments
** @param [r] argv [char* []] Actual arguments as a text array.
** @return [void]
** @@
******************************************************************************/

static void acdArgsParse (ajint argc, char *argv[]) {

  ajint i, j, number;
  ajint iparam=0;			/* expected next param */
  ajint jparam=0;			/* param found */
  ajint itestparam;
  ajint jtestparam=0;
  AcdPAcd acd;

  char *cp, *cq;
  static AjPStr qual = NULL;
  static AjPStr value = NULL;
  static AjPStr param = NULL;
  static AjPStr token = NULL;

  acdLog ("ArgsParse\n=========\n");

  acdLog ("\n");

  acdMasterQual = NULL;

  i = 1;
  while (i < argc) {
    acdLog ("%s ", argv[i]);
    i++;
  }
  acdLog ("\n");
  acdLog ("\n");

  i = 1;                        /* skip the program name */

  while (i < argc) {
    cp = argv[i];
    if ((i+1) < argc) cq = argv[i+1];
    else cq = NULL;

    acdLog ("\n");
    acdLog ("argv[%d] <%s>", i, cp);
    if (cq) acdLog (" + argv[%d] <%s>", i+1, cq);
    acdLog ("\n");
    jparam = 0;
    if ((j = acdIsQual(cp, cq, &jparam, &qual, &value, &number, &acd))) {
      if (jparam) {
	acdLog ("Parameter (%d) ", jparam);
	acdParamSet[jparam-1] = ajTrue;
	if (iparam == (jparam-1)) {
	  iparam = acdNextParam(iparam);
	  acdLog ("reset iparam = %d\n", iparam);
	}
	else
	  acdLog ("keep iparam = %d\n", iparam);
      }
      else
	acdLog ("Qualifier ");
      acdLog ("-%S ", acd->Name);
      if (number)
        acdLog ("[%d] ", number);
      if (ajStrLen(value))
        acdLog ("= '%S'", value);
      acdLog ("\n");

      if (acd) {		/* we found something */
        (void) acdDef (acd, value);
        acdLog ("set qualifier -%S[%d] (param %d) = %S\n",
		      acd->Name, acd->PNum, jparam, value);
        /* loop over any associated qualifiers for the rest */
	acdLog ("number: %d jparam: %d acd->PNum: %d acdNParam: %d\n",
		      number, jparam, acd->PNum, acdNParam);
        if (!number && !jparam && acd->PNum) {
          for (itestparam = acd->PNum+1; itestparam <= acdNParam;
	       itestparam++) {
	    acdLog ("test [%d] '%S'\n", itestparam, qual);
            acd = acdFindQual (qual, NULL, itestparam, &jtestparam);
            if (acd) {
              (void) acdDef (acd, value);
              acdLog ("set next qualifier -%S[%d] (param %d) = %S\n",
			    acd->Name, acd->PNum, jparam, value);
            }
	    else
	      acdLog ("no -%S[%d]\n", qual, itestparam);
          }
        }
      }
      else {
        acdLog ("unable to set qualifier -%S %d %d\n",
		      qual, number, iparam);
        ajFatal("unable to set qualifier -%S %d %d\n",
               qual, number, iparam);
      }
      if (j == 2) i++;
    }
    else {
      (void) acdIsParam(cp, &param, &iparam, &acd);
      if (acd) {
	if (acdIsParamValue(param)) {
	  acdLog ("Parameter %d: %S = %S\n",
			 iparam, acd->Name, param);
	  (void) acdDef (acd, param);
	  acdParamSet[iparam-1] = ajTrue;
	}
	else {
	  acdLog ("Parameter %d: %S = '%S' ** missing value **\n",
			 iparam, acd->Name, param);
	}
      }
      else {
	if (strpbrk(cp, "=") || strchr("-/", *cp))
	  ajFatal ("Bad qualifier or parameter '%s'\n", cp);
	else
	  ajFatal("Parameter %d '%s' not defined\n", iparam, cp);
      }
    }
    i++;
  }

  (void) ajStrDelReuse (&qual);
  (void) ajStrDelReuse (&value);
  (void) ajStrDelReuse (&param);
  (void) ajStrDelReuse (&token);

  return;
}

/* @funcstatic acdIsParamValue ************************************************
**
** Tests whether a parameter value is 'missing', in which case
** it wil be ignored for now.
**
** @param [r] pval [AjPStr] Parameter value
**
** @return [AjBool] ajFalse for a missing value.
** @@
******************************************************************************/

static AjBool acdIsParamValue (AjPStr pval) {

  if (ajStrMatchC(pval, ".")) return ajFalse;
  if (!ajStrLen(pval)) return ajFalse;

  return ajTrue;
}

/* @funcstatic acdNextParam ***************************************************
**
** Returns the next unknown parameter. Used for cases where parameters
** are specified by qualifier before their turn on the command line
**
** @param [r] pnum [ajint] Current parameter number
**
** @return [ajint] next undefined parameter
** @@
******************************************************************************/

static ajint acdNextParam (ajint pnum) {

  ajint i;

  if (pnum > acdNParam) return pnum+1; /* all done */

  for (i=pnum;i<acdNParam;i++)
    if (!acdParamSet[i]) return i; /* next free parameter */

  return acdNParam+1;		/* all done */
}

/* @funcstatic acdIsParam *****************************************************
**
** Tests an argument to see whether it could be a parameter.
** Qualifiers start with "-" or "/", or are built as qual=value where
** "qual" is a known qualifier.
** Parameters are any other text.
**
** @param [r] arg [char*] Argument
** @param [wP] param [AjPStr*] Parameter text copied on success
** @param [w] iparam [ajint*] Parameter number incremented on success
** @param [wP] acd [AcdPAcd*] ACD item for the current parameter
** @return [AjBool] ajTrue if "arg" could be a parameter
**         ajFalse if it appears to be a qualifier (starts with "-" and
**         ends with a name)
** @@
******************************************************************************/

static AjBool acdIsParam (char* arg, AjPStr* param, ajint* iparam,
                          AcdPAcd* acd) {

  char *cp = arg;

  acdLog ("acdIsParam arg: '%s' param: '%S' iparam: %d\n",
		arg, *param, *iparam);

  if (*iparam >= acdNParam) {
    ajFatal ("Argument '%s'\nToo many parameters %d/%d\n",
	     arg, (*iparam + 1), acdNParam);
  }


  (*iparam)++;
  *acd = acdFindParam (*iparam);

  if (!strcmp(cp, ".")) {	/* missing value */
    (void) ajStrAssC (param, ""); /* clear the parameter */
    return ajTrue;
  }

  (void) ajStrAssC(param, arg);	/* copy the argument value */
  if (*acd) {

    if ((*acd)->AssocQuals) {
      acdLog ("acdMasterQual [param] set to -%S\n", (*acd)->Name);
      acdMasterQual = *acd;
    }
    else if (acdMasterQual) {
      acdLog ("acdMasterQual cleared, was -%S\n", acdMasterQual->Name);
      acdMasterQual = NULL;
    }
    return ajTrue; 
  }

  return ajFalse; 
}

/* @funcstatic acdIsQual ******************************************************
**
** Tests an argument to see whether it is a qualifier
** Qualifiers start with "-".
** Qualifiers are assumed to take a value, which is either
** delimited by an "=" sign or is the next argument.
** Qualifiers can also have numbered suffix if matching one of the parameters
**
** @param [r] arg [char*] Argument
** @param [r] arg2 [char*] Next argument
** @param [w] iparam [ajint*] Parameter number
** @param [wP] pqual [AjPStr*] Qualifier name copied on success
** @param [wP] pvalue [AjPStr*] Qualifier value copied on success
** @param [w] number [ajint*] Qualifier number
** @param [wP] acd [AcdPAcd*] Qualifier data
** @return [ajint] Number of arguments consumed
** @@
******************************************************************************/

static ajint acdIsQual (char* arg, char* arg2, ajint *iparam, AjPStr *pqual,
                      AjPStr *pvalue, ajint* number, AcdPAcd* acd) {

  ajint ret=0;
  char *cp = arg;
  ajint i;
  AjBool gotvalue = ajFalse;
  AjBool ismissing = ajFalse;
  AjBool qstart = ajFalse;
  static AjPStr qmaster = NULL;

  *number = 0;

  if (!strcmp(cp, "-"))		/* stdin or stdout parameter */
    return 0;
  if (!strcmp(cp, "."))		/* dummy parameter */
    return 0;

  if (strchr("-/", *cp)) {	/* first character vs. qualifier starts */
    cp++;
    qstart = ajTrue;
  }

  if (!*cp)
    return 0;

  /* qualifier: now play hunt the value */

  ret = 1;
  (void) ajStrAssC (pqual, cp);	/* qualifier with '-' or '/' removed */

  /*
  ** qualifiers starting 'no' are boolean 'no'
  */

  if (qstart && ajStrPrefixC(*pqual, "no")) { /* boolean prefix */
    acdQualParse (pqual, &qmaster, number);
    (void) ajStrSub(pqual, 2, -1);
    (void) ajStrAssC(pvalue, "N");
    gotvalue = ajTrue;
  }

  else {
    i = ajStrFindC(*pqual, "=");
    if (i >= 0) {
      (void) ajStrAssSub(pvalue, *pqual, (i+1), -1);

      ajDebug("qualifier value '%S' '%S' %d .. %d\n",
	      *pvalue, *pqual, (i+1), -1);
      (void) ajStrSub (pqual, 0, (i-1));
      gotvalue = ajTrue;
    }
    else {
      if (!qstart)		/* no start, no "=" */
	return 0;
      if (!ajStrIsAlnum(*pqual))
	return 0;
    }
    acdQualParse (pqual, &qmaster, number);
  }

  if (ajStrLen(qmaster))	/* specific master, turn off auto processing */
    acdMasterQual = NULL;

  if (acdMasterQual) {
    acdLog ("(a) master, try associated with acdFindQualAssoc\n");
    *acd = acdFindQualAssoc (acdMasterQual, *pqual, *number);
    if (!*acd) {
      acdLog ("acdMasterQual cleared, was -%S\n", acdMasterQual->Name);
      acdMasterQual = NULL;
    }
    else {
      *number = acdMasterQual->PNum;
      acdLog ("Qualifier -%S associated with -%S\n",
		     *pqual, acdMasterQual->Name);
    }
  }

  if (!acdMasterQual) {
    acdLog ("(b) no master, general test with acdFindQual\n");
    *acd = acdFindQual (*pqual, qmaster, *number, iparam);
  }

  if (!*acd)
    ajFatal ("unknown qualifier %s\n", arg);

  if ((*acd)->AssocQuals) {
    acdLog ("acdMasterQual set to -%S\n", (*acd)->Name);
    acdMasterQual = *acd;
  }


  if (!gotvalue) {
    acdLog ("testing for a value\n");

    /*
    ** Bool qualifiers can have no value
    ** or can be followed by a valid Bool value
    */

    if (!strcmp(acdType[acdListCurr->Type].Name, "bool")) {
      if (acdValIsBool(arg2)) {	/* bool value, accept */
	ajDebug("acdValIsBool -%s '%s'\n", arg, arg2); 
	gotvalue = ajTrue;
	ret = 2;
	(void) ajStrAssC(pvalue, arg2);
      }
      else {			/* we must mean true */
	(void) ajStrAssC(pvalue, "Y");
      }
    }
    else {
      if (!arg2) {
	(void) ajStrToBool ((*acd)->DefStr[DEF_MISSING], &ismissing);
	if (!ismissing) {
	  ajFatal ("value required for %s\n", arg);
	}
      }
      /* test for known qualifiers */
      else {
	if (*arg2 == '-') {
	  if (!acdTestQualC (arg2)) { /* not known qualifier - must be value */
	    gotvalue = ajTrue;
	  }
	  else {
	    (void) ajStrToBool ((*acd)->DefStr[DEF_MISSING], &ismissing);
	    if (!ismissing) {
	      ajFatal ("value required for %s\n", arg);
	    }
	  }
	}
	else {
	  gotvalue = ajTrue;
	}
      }
      if (gotvalue) {
	ret = 2;
	(void) ajStrAssC (pvalue, arg2);
      }
      else
	(void) ajStrAssC (pvalue, "");
    }
  }

  return ret;
}

/* @funcstatic acdValIsBool ***************************************************
**
** Tests whether a value on the command line is a valid Boolean value
**
** @param [r] arg [char*] COmmand live argument value
** @return [AjBool] ajTrue if the value is boolean,
**                  but not whether it is true or false.
******************************************************************************/

static AjBool acdValIsBool (char* arg) {

  if (!arg) return ajFalse;
  switch (*arg) {
  case 'n':
  case 'N':
    if (!arg[1]) return ajTrue;
    return ajStrMatchCaseCC (arg, "no");
    break;

  case 'y':
  case 'Y':
    if (!arg[1]) return ajTrue;
    return ajStrMatchCaseCC (arg, "yes");
    break;

  case 't':
  case 'T':
    if (!arg[1]) return ajTrue;
    return ajStrMatchCaseCC (arg, "true");
    break;

  case 'f':
  case 'F':
    if (!arg[1]) return ajTrue;
    return ajStrMatchCaseCC (arg, "false");
    break;

  case '0':
    if (!arg[1]) return ajTrue;
    return ajStrMatchCaseCC (arg, "0");
    break;

  case '1':
    if (!arg[1]) return ajTrue;
    return ajStrMatchCaseCC (arg, "1");
    break;

  default:
    return ajFalse;
  }
}

/* @funcstatic acdFindItem ****************************************************
**
** Returns the ACD definition for a named item and
** (optionally) a given qualifier number. If the qualifier number
** is given, it is checked. If not, the first hit is used.
**
** @param [r] item [AjPStr] Item name
** @param [r] number [ajint] Item number (zero if a general item)
** @return [AcdPAcd] ACD item required
** @@
******************************************************************************/

static AcdPAcd acdFindItem (AjPStr item, ajint number) {

  AcdPAcd ret=NULL;
  AcdPAcd pa;
  AjBool found = ajFalse;
  ajint ifound=0;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  for (pa=acdList; pa; pa=pa->Next) {
    found = ajFalse;
    if (ajStrPrefix (pa->Name, item)) {
      if (!number || number == pa->PNum)
	found = ajTrue;
    }
    if (found) {
      if (ajStrMatch (pa->Name, item)) {
	return pa;
      }
      ifound++;
      ret = pa;
      acdAmbigApp (&ambigList, pa->Name);
    }
  }

  if (ifound == 1)
    return ret;
  if (ifound > 1) {
    ajWarn ("ambiguous item %S (%S)", item, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }

  return NULL;
}

/* @funcstatic acdFindQual ****************************************************
**
** Returns the parameter definition for a named qualifier and
** (optionally) a given qualifier number. If the qualifier number
** is given, it is checked. If not, the current parameter number is checked.
** General qualifiers have no specified number and can match at any time.
**
** @param [r] qual [AjPStr] Qualifier name
** @param [rN] master [AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @param [u] iparam [ajint*]  Current parameter number
** @return [AcdPAcd] ACD item for qualifier
** @@
******************************************************************************/

static AcdPAcd acdFindQual (AjPStr qual, AjPStr master,
			    ajint PNum, ajint *iparam) {

  /* test for match of parameter number and type */

  /* PNum : number encoded in qualifier name ==> forced match */
  /* iparam : current parameter number ==> possible match */
  /* when both are zero, could be a generic match, like "-begin" for
     all sequences. Just return the first and let caller find the rest */

  AcdPAcd ret=NULL;
  AcdPAcd pa;
  AjBool found = ajFalse;
  AjBool isparam = ajFalse;
  ajint ifound=0;
  static AjPStr ambigList = NULL;

  if (ajStrLen(master)) {
    *iparam = 0;
    return acdFindQualMaster (qual, master, PNum);
  }

  (void) ajStrAssC(&ambigList, "");

  acdLog ("acdFindQual '%S' PNum: %d iparam: %d\n",
     qual, PNum, *iparam);

  for (pa=acdList; pa; pa=pa->Next) {
    found = ajFalse;
    if (pa->Level == ACD_QUAL) {
      if (ajStrPrefix (pa->Name, qual)) {
	acdLog ("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
	if (PNum) {               /* -begin2 forces match to #2 */
	  if (PNum == pa->PNum) {
	    acdLog ("..matched PNum '%S' [%d]\n",
		  pa->Name, pa->PNum);
	    found = ajTrue;
	  }
	}
	else if (pa->PNum) {	/* defined for parameter pa->PNum */
	  acdLog ("..hit PNum '%S' [%d] (ambigList '%S')\n",
		  pa->Name, pa->PNum, ambigList);
	  if (!ifound  || !ajStrMatch(pa->Name, ambigList)) {
	    found = ajTrue;
	  }
	}
	else {	/* general match */
          found = ajTrue;
	}
	if (found) {
	  if (ajStrMatch (pa->Name, qual)) {
	    acdListCurr = pa;
	    return pa;
	  }
	  acdAmbigApp (&ambigList, pa->Name);
	  ifound++;
	  ret = pa;
	  acdLog ("..prefix only '%S', ifound %d\n", pa->Name, ifound);
	}
      }
    }
    else if (pa->Level == ACD_PARAM) {
      if (ajStrPrefix (pa->Name, qual)) {
	acdLog ("..matched param '%S' [%d]\n", pa->Name, pa->PNum);
	if (ajStrMatch (pa->Name, qual)) {
	  acdListCurr = pa;
	  *iparam = pa->PNum;
	  return pa;
	}
	acdAmbigApp (&ambigList, pa->Name);
	ifound++;
	isparam = ajTrue;
	ret = pa;
	acdLog ("..prefix only '%S', ifound %d\n", pa->Name, ifound);
      }
    }
  }

  if (ifound == 1) {
    acdListCurr = ret;
    if (isparam)
      *iparam = ret->PNum;
    return ret;
  }
  if (ifound > 1) {
    ajWarn ("ambiguous qualifier %S (%S)", qual, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }

  return NULL;
}

/* @funcstatic acdFindQualMaster **********************************************
**
** Returns the parameter definition for a named qualifier and
** (optionally) a given qualifier number. If the qualifier number
** is given, it is checked. If not, the current parameter number is checked.
** General qualifiers have no specified number and can match at any time.
**
** @param [r] qual [AjPStr] Qualifier name
** @param [rN] master [AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @return [AcdPAcd] ACD item for qualifier
** @@
******************************************************************************/

static AcdPAcd acdFindQualMaster (AjPStr qual, AjPStr master,
				  ajint PNum) {

  /* test for match of parameter number and type */

  /* PNum : number encoded in qualifier name ==> forced match */
  /* iparam : current parameter number ==> possible match */
  /* when both are zero, could be a generic match, like "-begin" for
     all sequences. Just return the first and let caller find the rest */

  AcdPAcd ret=NULL;
  AcdPAcd pa;
  AjBool found = ajFalse;
  ajint ifound=0;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  acdLog ("acdFindQualMaster '%S_%S' PNum: %d\n",
     qual, master, PNum);

  for (pa=acdList; pa; pa=pa->Next) {
    found = ajFalse;
    if (pa->Level == ACD_QUAL) {
      if (ajStrPrefix (pa->Name, master)) {
	acdLog ("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
	if (PNum) {               /* -begin2 forces match to #2 */
	  if (PNum == pa->PNum) {
	    acdLog ("..matched PNum '%S' [%d]\n",
		  pa->Name, pa->PNum);
	    found = ajTrue;
	  }
	}
	else if (pa->PNum) {	/* defined for parameter pa->PNum */
	  acdLog ("..hit PNum '%S' [%d] (ambigList '%S')\n",
		  pa->Name, pa->PNum, ambigList);
	  if (!ifound  || !ajStrMatch(pa->Name, ambigList)) {
	    found = ajTrue;
	  }
	}
	else {	/* general match */
          found = ajTrue;
	}
	if (found) {
	  if (ajStrMatch (pa->Name, master)) {
	    ret = pa;
	    ifound = 1;
	    break;
	  }
	  acdAmbigApp (&ambigList, pa->Name);
	  ifound++;
	  ret = pa;
	  acdLog ("..prefix only, ifound %d\n", ifound);
	}
      }
    }
    else if (pa->Level == ACD_PARAM) {
      if (ajStrPrefix (pa->Name, master)) {
	acdLog ("..matched param '%S' [%d]\n", pa->Name, pa->PNum);
	if (ajStrMatch (pa->Name, master)) {
	  ret = pa;
	  ifound = 1;
	  break;
	}
	ifound++;
	ret = pa;
	acdLog ("..prefix only, ifound %d\n", ifound);
      }
    }
  }

  if (ifound > 1) {
    acdLog ("..ambiguous master qualifier for %S_%S (%S)",
	    qual, master, ambigList);
    ajWarn ("ambiguous master qualifier for %S_%S (%S)",
	    qual, master, ambigList);
    (void) ajStrDelReuse(&ambigList);
    return NULL;
  }
  if (!ifound){
    acdLog ("..master qualifier for %S_%S not found\n", qual, master);
    return NULL;
  }

  acdLog ("..master qualifier found '%S' %d\n", ret->Name, ret->PNum);

  ifound = 0;
  for (pa=ret->AssocQuals; pa->Assoc; pa=pa->Next) {
    found = ajFalse;
    if (ajStrPrefix (pa->Name, qual)) {
      acdLog ("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
      if (PNum) {               /* -begin2 forces match to #2 */
	if (PNum == pa->PNum) {
	  acdLog ("..matched PNum '%S' [%d]\n",
		  pa->Name, pa->PNum);
	  found = ajTrue;
	}
      }
      else if (pa->PNum) {	/* defined for parameter pa->PNum */
	acdLog ("..hit PNum '%S' [%d] (ambigList '%S')\n",
		pa->Name, pa->PNum, ambigList);
	if (!ifound  || !ajStrMatch(pa->Name, ambigList)) {
	  found = ajTrue;
	}
      }
      else {	/* general match */
	found = ajTrue;
      }
      if (found) {
	if (ajStrMatch (pa->Name, qual)) {
	  acdListCurr = pa;
	  return pa;
	}
	acdAmbigApp (&ambigList, pa->Name);
	ifound++;
	ret = pa;
	acdLog ("..prefix only, ifound %d\n", ifound);
      }
    }
  }
  if (ifound == 1) {
    acdListCurr = ret;
    return ret;
  }
  if (ifound > 1) {
    acdLog ("..ambiguous associated qualifier %S_%S (%S)",
	    qual, master, ambigList);
    ajWarn ("ambiguous associated qualifier %S_%S (%S)",
	    qual, master, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }

  acdLog ("..associated qualifier %S_%S not found", qual, master);

  return NULL;
}



/* @funcstatic acdFindQualAssoc ***********************************************
**
** Returns the definition for a named associated qualifier.
** If the qualifier number
** is given, it is checked. If not, the current parameter number is checked.
** General qualifiers have no specified number and can match at any time.
**
** @param [r] thys [AcdPAcd] Master ACD item
** @param [r] qual [AjPStr] Qualifier name
** @param [r] pnum [ajint] Qualifier number (zero if a general qualifier)
** @return [AcdPAcd] ACD item for associated qualifier
** @error NULL returned if not found.
** @@
******************************************************************************/

static AcdPAcd acdFindQualAssoc (AcdPAcd thys, AjPStr qual, ajint pnum) {

  /* test for match of parameter number and type */

  /* PNum : number encoded in qualifier name ==> forced match */
  /* iparam : current parameter number ==> possible match */
  /* when both are zero, could be a generic match, like "-sbegin" for
     all sequences. Just return the first and let caller find the rest */

  AcdPAcd pa=thys->AssocQuals;
  ajint ifound=0;
  AcdPAcd ret=NULL;
  static AjPStr ambigList = NULL;

  (void) ajStrAssC(&ambigList, "");

  /* ajDebug ("acdFindQualAssoc '%S' pnum: %d\n", qual, pnum); */

  if (pnum  && (pa->PNum != pnum)) /* must be for same number (if any) */
    return NULL;

  for (; pa->Assoc; pa=pa->Next) {
    if (ajStrPrefix (pa->Name, qual)) {
      if (ajStrMatch (pa->Name, qual)) {
	/* ajDebug ("   *matched* '%S'\n", pa->Name); */
	acdListCurr = pa;
	return acdListCurr;
      }
      ifound++;
      ret = pa;
      acdAmbigApp (&ambigList, pa->Name);
    }
  }

  /* ajDebug ("   ifound: %d\n", ifound); */

  if (ifound == 1) {
    acdListCurr = ret;
    return acdListCurr;
  }
  if (ifound > 1) {
    ajWarn ("ambiguous qualifier %S (%S)", qual, ambigList);
    (void) ajStrDelReuse(&ambigList);
  }

  return NULL;
}

/* @funcstatic acdFindParam ***************************************************
**
** Returns the paremeter definition for a given parameter number
**
** @param [r] PNum [ajint] Parameter number
** @return [AcdPAcd] ACD item for parameter number PNum
** @error NULL if not found
** @@
******************************************************************************/

static AcdPAcd acdFindParam (ajint PNum) {

  /* test for match of parameter number and type */

  AcdPAcd pa ;

  for (pa=acdList; pa; pa=pa->Next) {
    if ((pa->Level == ACD_PARAM) && (pa->PNum == PNum)) {
      acdListCurr = pa;
      return pa;
    }
  }

  return NULL;
}

/* @funcstatic acdGetAttr *****************************************************
**
** Pick up a defined attribute for variable handling.
** The ACD item must be defined (already processed).
** Attributes include and specially set up by the acdSet function
** for that type.
**
** @param [uP] result [AjPStr*] Resulting attribute value
** @param [r] name [AjPStr] ACD item name
** @param [r] attrib [AjPStr] attribute name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdGetAttr (AjPStr* result, AjPStr name, AjPStr attrib) {

  char *cp, *cq;
  ajint ilen;
  ajint number=0;
  static AjPStr tempstr=NULL;
  AcdPAcd pa=NULL;
  AcdPAttr attr=NULL;
  ajint i;

  (void) ajStrDelReuse (result);

  (void) ajStrAssS (&tempstr, name);
  (void) ajStrToLower(&tempstr);
  cp = ajStrStr(tempstr);
  cq = &cp[ajStrLen(tempstr)];
  if (isdigit((ajint)*--cq)) {
    while (isdigit((ajint)*--cq));
    ++cq;

    number = (ajint) strtol (cq, NULL, 0);
    ilen = cq - cp - 1;
    (void) ajStrSub (&tempstr, 0, ilen);
  }

  pa = acdFindItem (tempstr, number);
  if (!pa)
    ajFatal ("Failed to resolve variable '%S'\n", name);

  if (!pa->ValStr)
    ajFatal ("variable '%S' not yet defined\n", name);

  if (!ajStrLen(attrib)) {	/* just use valstr */
    (void) ajStrAssS (result, pa->ValStr);
    (void) ajStrDelReuse (&tempstr);
    ajDebug ("no attribute name, use valstr for %S '%S'\n",
       pa->Name, *result);
    return ajTrue;
  }

  if (pa->DefStr) {
    attr = acdAttrDef;
    i = acdFindAttr (attr, attrib);
    if (i >= 0) {
      (void) ajStrAssS (result, pa->DefStr[i]);
      (void) ajStrDelReuse (&tempstr);
      ajDebug ("default attribute %S found for %S '%S'\n",
	       attrib, pa->Name, *result);
      return ajTrue;
    }
  }

  if (pa->NAttr) {
    attr = acdType[pa->Type].Attr;
    i = acdFindAttr (attr, attrib);
    if (i >= 0) {
      (void) ajStrAssS (result, pa->AttrStr[i]);
      (void) ajStrDelReuse (&tempstr);
      ajDebug ("type attribute %S found for %S '%S'\n",
	       attrib, pa->Name, *result);
      return ajTrue;
    }
  }
  
  if (pa->SAttr) {
    attr = pa->SetAttr;
    i = acdFindAttr (attr, attrib);
    if (i >= 0) {
      (void) ajStrAssS (result, pa->SetStr[i]);
      (void) ajStrDelReuse (&tempstr);
      ajDebug ("calculated attribute %S found for %S '%S'\n",
	       attrib, pa->Name, *result);
      return ajTrue;
    }
  }

  (void) ajStrDelReuse (&tempstr);

  ajDebug ("*attribute %S not found for %S*\n", attrib, pa->Name);
   return ajFalse;

}

/* @funcstatic acdQualParse *************************************************
**
** Converts a qualifier name to lower case and looks for a
** master qualifier name and a trailing number.
**
** @param [uP] pqual [AjPStr*] Qualifier name set to lower case
**        with number suffix removed
** @param [uP] pqmaster [AjPStr*] Master name for associated qualifier
** @param [w] number [ajint*] Qualifier number suffix if any
** @return [void]
** @@
******************************************************************************/

static void acdQualParse (AjPStr* pqual, AjPStr* pqmaster, ajint* number) {

  static AjPRegexp qualexp = NULL;
  static AjPStr tmpqual = NULL;
  static AjPStr tmpnum = NULL;

  if (!qualexp)
    qualexp = ajRegCompC ("^([a-z]+)(_([a-z]+))?([0-9]+)?$");

  acdLog ("acdQualParse ('%S')\n", *pqual);
  (void) ajStrToLower(pqual);
  acdLog ("lower: '%S'\n", *pqual);
  (void) ajStrAssS (&tmpqual, *pqual);

  (void) ajRegExec (qualexp, tmpqual);
  ajRegSubI(qualexp, 1, pqual);
  ajRegSubI(qualexp, 3, pqmaster);
  ajRegSubI(qualexp, 4, &tmpnum);
  if (ajStrLen(tmpnum))
      (void) ajStrToInt(tmpnum, number);

  acdLog ("pqual '%S' pqmaster '%S' tmpnum '%S' number %d\n",
	  *pqual, *pqmaster, tmpnum, *number);

  return;
}

/* @funcstatic acdTokenToLower ************************************************
**
** Converts a token name to lower case and looks for a trailing number.
**
** @param [u] token [char*] Qualifier name set to lower case
**        with number suffix removed
** @param [w] number [ajint*] Qualifier number suffix if any.
** @return [void]
** @@
******************************************************************************/

static void acdTokenToLower (char *token, ajint* number) {
  char *cp, *cq;
  ajint ilen;

  ajCharToLower(token);
  cp = token;
  cq = &cp[strlen(token)];
  if (!isdigit((ajint)*--cq)) {
    *number = 0;
    return;
  }

  while (isdigit((ajint)*--cq));
  ++cq;

  *number = (ajint) strtol (cq, NULL, 0);
  ilen = cq - cp;
  token[ilen] = '\0';
  return;

}

/* @funcstatic acdIsRequired **************************************************
**
** Returns true if an ACD item is required but not yet defined.
** Required means the required attribute is set (which it is by
** default for a parameter),
** or the optional flag is set and -options was specified.
**
** @param [r] thys [AcdPAcd] ACD item
** @return [AjBool] ajTrue if "thys" is required but no value set yet.
** @@
******************************************************************************/

static AjBool acdIsRequired (AcdPAcd thys) {

  AjPStr* def = thys->DefStr;
  AjBool required = ajFalse;

  if (thys->Defined)
    return ajFalse;
  if (!thys->DefStr)
    return ajFalse;

  if (ajStrLen(def[DEF_REQUIRED])) {
    (void) acdVarResolve(&def[DEF_REQUIRED]);
    if (!ajStrToBool(def[DEF_REQUIRED], &required))
      ajFatal ("%S: Bad required flag %S\n",
	       thys->Name, def[DEF_REQUIRED]);
    return required;
  }

  if (acdOptions && ajStrLen(def[DEF_OPTIONAL])) {
    (void) acdVarResolve(&def[DEF_OPTIONAL]);
    if (!ajStrToBool(def[DEF_OPTIONAL], &required))
      ajFatal ("%S: Bad optional flag %S\n",
	       thys->Name, def[DEF_OPTIONAL]);
    return required;
  }

  return ajFalse;
}

/* @func ajAcdDebug *******************************************************
**
** Tests whether debug messages are required by checking
** internal variable 'acdDebug'
**
** @return [AjBool] Debugging status.
** @@
******************************************************************************/

AjBool ajAcdDebug (void) {
  ajDebug("ajAcdDebug returning %B", acdDebug);
  return acdDebug;
}

/* @func ajAcdDebugIsSet *****************************************************
**
** Tests whether the command line switch for debug messages has been set
** by testing internal variable 'acdDebugSet'
**
** @return [AjBool] Debugging status.
** @@
******************************************************************************/

AjBool ajAcdDebugIsSet (void) {
  ajDebug("ajAcdDebugIsSet returning %B", acdDebugSet);
  return acdDebugSet;
}

/* @func ajAcdFilter *******************************************************
**
** Tests whether input and output use stdin and stdout as a filter
** by returning internal variable 'acdFilter'
**
** @return [AjBool] Filter status.
** @@
******************************************************************************/

AjBool ajAcdFilter (void) {
  return acdFilter;
}

/* @func ajAcdStdout *******************************************************
**
** Tests whether output uses stdout for output by default
** by returning internal variable 'acdStdout'
**
** @return [AjBool] Stdout status.
** @@
******************************************************************************/

AjBool ajAcdStdout (void) {
  return acdStdout;
}

/* @func ajAcdProgramS ******************************************************
**
** Returns the application (program) name from the ACD definition.
**
** @param [w] pgm [AjPStr*] returns the program name.
** @return [void]
** @@
******************************************************************************/

void ajAcdProgramS (AjPStr* pgm) {
  (void) ajStrAssS (pgm, acdProgram);
  return;
}

/* @func ajAcdProgram *******************************************************
**
** Returns the application (program) name from the ACD definition.
**
** @return [char*] Program name
** @@
******************************************************************************/

char* ajAcdProgram (void) {
  return ajStrStr(acdProgram);
}


/* @funcstatic acdPromptCodon ***********************************************
**
** Sets the default prompt for this ACD object to be a codon usage file
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptCodon (AcdPAcd thys) {
  AjPStr* prompt;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  (void) ajFmtPrintS (prompt, "Codon usage file");
  return;
}


/* @funcstatic acdPromptDirlist *********************************************
**
** Sets the default prompt for this ACD object to be a dirlist
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptDirlist (AcdPAcd thys) {
  AjPStr* prompt;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  (void) ajFmtPrintS (prompt, "Directory with files");
  return;
}

/* @funcstatic acdPromptFeat **************************************************
**
** Sets the default prompt for this ACD object to be a feature table
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptFeat (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  count++;
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Input features"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second features"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third features"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth features", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst features", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd features", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd features", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth features", count); break;
    }
    break;
  }
  return;
}

/* @funcstatic acdPromptCpdb ***********************************************
**
** Sets the default prompt for this ACD object to be a clean PDB file
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptCpdb (AcdPAcd thys) {
  AjPStr* prompt;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  (void) ajFmtPrintS (prompt, "Clean PDB file");
  return;
}

/* @funcstatic acdPromptScop ***********************************************
**
** Sets the default prompt for this ACD object to be a scop entry
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptScop (AcdPAcd thys) {
  AjPStr* prompt;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  (void) ajFmtPrintS (prompt, "Scop entry");
  return;
}

/* @funcstatic acdPromptSeq ***************************************************
**
** Sets the default prompt for this ACD object to be a sequence
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptSeq (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  count++;
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Input sequence"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second sequence"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third sequence"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth sequence", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst sequence", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd sequence", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd sequence", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth sequence", count); break;
    }
    break;
  }

  if (ajStrMatchCC(acdType[thys->Type].Name, "seqset"))
    ajStrAppC (prompt, " set");
  if (ajStrMatchCC(acdType[thys->Type].Name, "seqall"))
    ajStrAppC (prompt, "(s)");

  return;
}

/* @funcstatic acdPromptGraph ***********************************************
**
** Sets the default prompt for this ACD object to be a sequence
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptGraph (AcdPAcd thys) {
  AjPStr* prompt;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  (void) ajFmtPrintS (prompt, "Graph type");
  return;
}

/* @funcstatic acdPromptFeatout ***********************************************
**
** Sets the default prompt for this ACD object to be a features output
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptFeatout (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  count++;
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Output features"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second output features"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third output features"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth output features", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst output features", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd output features", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd output features", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth output features", count); break;
    }
    break;
  }
  return;
}

/* @funcstatic acdPromptReport ************************************************
**
** Sets the default prompt for this ACD object to be a report output
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptReport (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  count++;
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Output report"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second output report"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third output report"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth output report", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst output report", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd output report", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd output report", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth output report", count); break;
    }
    break;
  }
  return;
}

/* @funcstatic acdPromptSeqout ************************************************
**
** Sets the default prompt for this ACD object to be a sequence output
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptSeqout (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  count++;
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Output sequence"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second output sequence"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third output sequence"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth output sequence", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst output sequence", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd output sequence", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd output sequence", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth output sequence", count); break;
    }
    break;
  }
  return;
}

/* @funcstatic acdPromptOutfile ***********************************************
**
** Sets the default prompt for this ACD object to be an output
** file with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptOutfile (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr) {
    ajDebug("acdPromptOutfile thys->DefStr NULL\n");
    return;
  }

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt)) {
    ajDebug("acdPromptOutfile found thys->DefStr[DEF_PROMPT] '%S'\n", *prompt);
    /*(void) ajStrTrace (*prompt);*/
    /*(void) ajStrTrace (thys->DefStr[DEF_PROMPT]);*/
    return;
  }

  count++;
  ajDebug("acdPromptOutfile count %d\n", count);
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Output file"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second output file"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third output file"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth output file", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst output file", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd output file", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd output file", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth output file", count); break;
    }
    break;
  }
  /*(void) ajStrTrace (*prompt);*/
  /*(void) ajStrTrace (thys->DefStr[DEF_PROMPT]);*/
  ajDebug("acdPromptOutfile set thys->DefStr[DEF_PROMPT] '%S' '%S'\n",
	  *prompt, thys->DefStr[DEF_PROMPT]);
  return;
}

/* @funcstatic acdPromptInfile ************************************************
**
** Sets the default prompt for this ACD object to be an input
** file with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptInfile (AcdPAcd thys) {
  AjPStr* prompt;
  static ajint count=0;

  if (!thys->DefStr)
    return;

  prompt = &thys->DefStr[DEF_PROMPT];
  if (ajStrLen(*prompt))
    return;

  count++;
  switch (count) {
  case 1: (void) ajFmtPrintS (prompt, "Input file"); break;
  case 2: (void) ajFmtPrintS (prompt, "Second input file"); break;
  case 3: (void) ajFmtPrintS (prompt, "Third input file"); break;
  case 11:
  case 12:
  case 13:
    (void) ajFmtPrintS (prompt, "%dth input file", count); break;
  default:
    switch (count % 10) {
    case 1: (void) ajFmtPrintS (prompt, "%dst input file", count); break;
    case 2: (void) ajFmtPrintS (prompt, "%dnd input file", count); break;
    case 3: (void) ajFmtPrintS (prompt, "%drd input file", count); break;
    default: (void) ajFmtPrintS (prompt, "%dth input file", count); break;
    }
    break;
  }
  return;
}

/* @funcstatic acdCodeGet ****************************************************
**
** Translates a code into a message text using the code table
** for the current language.
**
** @param [r] code [AjPStr] Code name
** @param [w] msg [AjPStr*] Message text for this code in current language
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdCodeGet (AjPStr code, AjPStr *msg) {

  AjPStr value;			/* not static - copy of a table text */
  static AjPStr tmpcode = NULL;
  AjBool ret = ajFalse;

  ajDebug ("acdCodeGet ('%S')\n", code);

  if (!acdCodeSet)
    acdCodeInit();

  (void) ajStrAssS(&tmpcode, code);
  (void) ajStrToLower (&tmpcode);

  value = ajTableGet (acdCodeTable, tmpcode);
  if (value) {
    (void) ajStrAssS (msg, value);
    ajDebug ("%S value '%S'\n", code, *msg);
    ret = ajTrue;
  }

  (void) ajStrDelReuse(&tmpcode);

  return ret;
}

/* @funcstatic acdCodeDef *****************************************************
**
** Generates a default code name of 'def' + qualifier type.
** Translates into a message text using the code table
** for the current language.
**
** @param [r] thys [AcdPAcd] Current ACD object
** @param [w] msg [AjPStr*] Message text for default message
**                          in current language
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdCodeDef (AcdPAcd thys, AjPStr *msg) {

  static AjPStr code = NULL;
  static AjPStr value = NULL;
  AjBool ret = ajFalse;

  ajDebug ("acdCodeDef '%s'\n", acdType[thys->Type].Name);
  if (!acdCodeSet)
    acdCodeInit();

  code = ajStrNewC("def");
  (void) ajStrAppC (&code, acdType[thys->Type].Name);
  (void) ajStrToLower(&code);
  ajDebug ("look for defcode '%S'\n", code);

  if (acdCodeGet (code, &value)) {
    (void) ajFmtPrintS (msg, "-%S : %S",
		 thys->Name, value);
    (void) ajStrDelReuse (&value);
    ret = ajTrue;
  }
  else {
    ajDebug ("defcode not found '%S'\n", code);
  }

  (void) ajStrDelReuse (&code);

  return ret;
}

/* @funcstatic acdHelpCodeDef *****************************************************
**
** Generates a default code name of 'help' + qualifier type.
** Translates into a message text using the code table
** for the current language.
**
** @param [r] thys [AcdPAcd] Current ACD object
** @param [w] msg [AjPStr*] Help text in current language
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdHelpCodeDef (AcdPAcd thys, AjPStr *msg) {

  AjPStr code = NULL;
  static AjPStr value = NULL;
  AjBool ret = ajFalse;

  ajDebug ("acdHelpCodeDef '%s'\n", acdType[thys->Type].Name);
  if (!acdCodeSet)
    acdCodeInit();

  code = ajStrNewC("help");
  (void) ajStrAppC (&code, acdType[thys->Type].Name);
  (void) ajStrToLower(&code);
  ajDebug ("look for helpcode '%S'\n", code);

  if (acdCodeGet (code, &value)) {
    (void) ajFmtPrintS (msg, "%S", value);
    (void) ajStrDelReuse (&value);
    ret = ajTrue;
  }
  else {
    ajDebug ("helpcode not found '%S'\n", code);
  }

  ajStrDel(&code);

  return ret;
}


/* @funcstatic acdCodeInit ****************************************************
**
** Sets up the code file data for the current language when needed
**
** @return [void]
** @@
******************************************************************************/

static void acdCodeInit (void) {

  AjPFile codeFile = NULL;
  static AjPStr codeFName = NULL;
  static AjPStr codeRoot = NULL;
  static AjPStr codeRootInst = NULL;
  static AjPStr codePack = NULL;
  static AjPStr codeCode = NULL;
  static AjPStr codeValue = NULL;
  static AjPStr codeLine = NULL;
  static AjPStr codeText = NULL;
  AjPRegexp codexp = NULL;

  if (acdCodeSet) return;

  (void) ajNamRootPack (&codePack);
  (void) ajNamRootInstall (&codeRootInst);
  (void) ajFileDirFix (&codeRootInst);

  if (ajNamGetValueC ("acdroot", &codeRoot)) {
    (void) ajFileDirFix (&codeRoot);
    ajFmtPrintS (&codeFName, "%Scodes.english", codeRoot);
    codeFile = ajFileNewIn (codeFName);
  }
  else {
    ajFmtPrintS (&codeFName, "%Sshare/%S/acd/codes.english",
		 codeRootInst, codePack);
    codeFile = ajFileNewIn (codeFName);
    if (!codeFile) {
      ajDebug ("codefile '%S' not opened\n", codeFName);
      (void) ajNamRoot (&codeRoot);
      (void) ajFileDirFix (&codeRoot);
      ajFmtPrintS (&codeFName, "%Sacd/codes.english", codeRoot);
      codeFile = ajFileNewIn (codeFName);
    }
  }

  if (!codeFile)
    ajWarn ("Code file %S not found", codeFName);


  ajDebug ("Code file %F used\n", codeFile);
  codeText = ajStrNew();

  /* fix by Nicolas Joly <njoly@pasteur.fr> */

  while (codeFile && ajFileReadLine(codeFile, &codeLine)) {
    acdNoComment(&codeLine);
    if (ajStrLen(codeLine)) {
      (void) ajStrApp (&codeText, codeLine);
      (void) ajStrAppC (&codeText, " ");
    }
  }
  ajFileClose (&codeFile);
  
  (void) ajStrDelReuse (&codeLine);

  acdCodeTable = ajStrTableNew(0);

  codexp = ajRegCompC("^ *([^ ]+) +\"([^\"]*)\"");
  while (ajRegExec (codexp, codeText)) {
    codeCode = codeValue = NULL; /* need to save in table each time */
    ajRegSubI (codexp, 1, &codeCode);
    ajRegSubI (codexp, 2, &codeValue);
    (void) ajStrToLower (&codeCode);
    (void) ajTablePut (acdCodeTable, codeCode, codeValue);
    ajDebug ("add to table %S '%S'\n", codeCode, codeValue);
    (void) ajRegPost (codexp, &codeText);
  }
  if (!ajStrIsSpace(codeText))
    ajFatal ("Bad format in codes file %S", codeFName);

  ajRegFree (&codexp);
  (void) ajStrDelReuse (&codeText);
  (void) ajStrDelReuse (&codeFName);

  acdCodeSet = ajTrue;
  return;
}

/* @funcstatic acdSetQualAppl ************************************************
**
** Sets internal variables for the application booleans -debug etc.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] val [AjBool] Value
** @return [AjBool] ajTrue if this was an application-wide variable.
** @@
******************************************************************************/

static AjBool acdSetQualAppl (AcdPAcd thys, AjBool val) {
  ajint i=0;
  AjBool setval;
  static AjPStr setstr = NULL;
  static AjPStr valstr = NULL;

  ajDebug("acdSetQualAppl '%S'\n", thys->Name);

  for(i=0; acdQualAppl[i].Name; i++) {
    if (ajStrMatchC(thys->Name, acdQualAppl[i].Name)) {
      if (thys->Defined) {	/* User put it on the command line */
	setval = val;
	acdLog ("Appl qualifier defined %S = %b\n", thys->Name, setval);
      }
      else {			/* look for a variable */
	(void) ajFmtPrintS (&setstr, "%S", thys->Name);
	if (ajNamGetValue(setstr, &valstr)) {
	  (void) ajStrToBool(valstr, &setval);
	  acdLog ("Appl qualifier variable %S = %b\n", setstr, setval);
	}
	else			/* nothing found, use the default value */
	  setval = val;
      }
      switch (i) {		/* see acdQualAppl for the correct order */
      case 0: acdAuto     = setval; break;
      case 1: acdStdout   = setval; break;
      case 2: acdFilter   = setval;
	if (acdFilter) {
	  acdAuto   = ajTrue;
	  acdStdout = ajTrue;
	}
	break;
      case 3: acdOptions  = setval; break;
      case 4:
	acdDebug = setval;
	/* ajDebug ("acdSetQualAppl acdDebug %B\n", acdDebug); */
	acdDebugSet = ajTrue;
	break;
      case 5: acdDoLog    = setval; break;
      case 6: acdDoPretty = setval; break;
      case 7: acdTable    = setval; break;
      case 8: acdDoHelp   = setval; break;
      case 9: acdVerbose  = setval; break;
      }
      return ajTrue;
    }
  }

  return ajFalse;
}

/* @funcstatic acdSelectPrompt ************************************************
**
** Present the options as a simple numbered list
**
** @param [r] thys [AcdPAcd] ACD object
** @return [void]
** @@
******************************************************************************/

static void acdSelectPrompt (AcdPAcd thys) {
  AjPStr hdrstr;
  AjPStr delim;
  AjPStr value;
  AjPStrTok handle;
  static AjPStr line = NULL;
  static char* white = " \t\n\r";
  ajint i = 0;

  if (acdAuto) return;

  hdrstr = acdAttrValue (thys, "header");
  if (ajStrLen(hdrstr))
    ajUser ("%S", hdrstr);

  delim = acdAttrValue (thys, "delimiter");
  if (!ajStrLen(delim))
    (void) ajStrAssC(&delim, ";");
  value = acdAttrValue (thys, "value");
  handle = ajStrTokenInit (value, ajStrStr(delim));
  while (ajStrDelim (&line, &handle, NULL)) {
    (void) ajStrTrimC (&line, white);
    ajUser("  %5d : %S", ++i, line);
  }

  (void) ajStrTokenClear (&handle);
  (void) ajStrDelReuse (&line);

  return;
}

/* @funcstatic acdListPrompt **************************************************
**
** Present the options as a list with option codes selectable by the user
**
** @param [r] thys [AcdPAcd] ACD object
** @return [void]
** @@
******************************************************************************/

static void acdListPrompt (AcdPAcd thys) {
  AjPStr hdrstr;
  AjPStr codedelim;
  AjPStr delim;
  AjPStr value;
  AjPStrTok handle;
  AjPStrTok codehandle;
  static AjPStr line = NULL;
  static AjPStr code = NULL;
  static AjPStr desc = NULL;
  static char* white = " \t\n\r";

  if (acdAuto) return;

  hdrstr = acdAttrValue (thys, "header");
  if (ajStrLen(hdrstr))
    ajUser ("%S", hdrstr);

  delim = acdAttrValue (thys, "delimiter");
  if (!ajStrLen(delim))
    (void) ajStrAssC(&delim, ";");
  codedelim = acdAttrValue (thys, "codedelimiter");
  if (!ajStrLen(codedelim))
    (void) ajStrAssC(&codedelim, ":");

  value = acdAttrValue (thys, "value");
  handle = ajStrTokenInit (value, ajStrStr(delim));
  while (ajStrDelim (&line, &handle, NULL)) {
    codehandle = ajStrTokenInit (line, ajStrStr(codedelim));
    (void) ajStrToken (&code, &codehandle, NULL);
    (void) ajStrToken (&desc, &codehandle, ajStrStr(delim));
    (void) ajStrTrimC (&code, white);
    (void) ajStrTrimC (&desc, white);
    ajUser("  %8S : %S", code, desc);
    (void) ajStrTokenClear (&codehandle);
  }

  (void) ajStrTokenClear (&handle);
  (void) ajStrDelReuse (&line);
  (void) ajStrDelReuse (&code);
  (void) ajStrDelReuse (&desc);

  return;
}

/* @funcstatic acdListValue **************************************************
**
** Checks the user setting against the list of codes and descriptions.
**
** An unambiguous match to the codes counts as valid.
** If this fails, an unambiguous match to the descriptions counts.
** 
** @param [r] thys [AcdPAcd] ACD Object
** @param [r] min [ajint] Minimum number of values required
** @param [r] max [ajint] Maximum number of values required
** @param [r] reply [AjPStr] Default value
** @return [AjPStr*] Array of accepted matches, ending with a NULL.
** @@
******************************************************************************/

static AjPStr* acdListValue (AcdPAcd thys, ajint min, ajint max, AjPStr reply) {

  AjPStr* val = NULL;

  static AjPStr codedelim = NULL;
  static AjPStr delim = NULL;
  AjPStr value;
  AjBool exactcase;
  AjPStrTok handle;
  AjPStrTok rephandle;
  AjPStrTok codehandle;
  static AjPStr line = NULL;
  static AjPStr code = NULL;
  static AjPStr desc = NULL;
  AjPList list = NULL;
  static AjPStr repstr = NULL;
  static AjPStr hitstr = NULL;
  AjPStr hitstr1 = NULL;
  AjPStr hitstr2 = NULL;
  static AjPStr repdelim = NULL;
  static char* white = " \t\n\r";
  ajint k = 0;

  ajint ifound = 0;
  ajint jfound = 0;
  ajint ilen;

  AjBool ok = ajTrue;

  list = ajListstrNew();

  (void) acdAttrToBool(thys, "casesensitive", ajFalse, &exactcase);

  acdAttrValueStr (thys, "delimiter", ";", &delim);
  acdAttrValueStr (thys, "codedelimiter", ":", &codedelim);

  if (!repdelim) {
    repdelim = ajStrNewL(10);
    (void) ajStrAssC (&repdelim, ",");
    (void) ajStrAppC (&repdelim, white);
  }

  value = acdAttrValue (thys, "value");

  /* ajDebug ("reply: '%S' delim '%S'", reply, repdelim); */

  rephandle = ajStrTokenInit (reply, ajStrStr(repdelim));
  while (ajStrToken (&repstr, &rephandle, NULL)) {

    ajDebug("testing '%S'\n", repstr);
    handle = ajStrTokenInit (value, ajStrStr(delim));
    ifound = jfound = 0;
    while (ajStrDelim (&line, &handle, NULL)) {
      codehandle = ajStrTokenInit (line, ajStrStr(codedelim));
      (void) ajStrToken (&code, &codehandle, NULL);
      (void) ajStrToken (&desc, &codehandle, ajStrStr(delim));
      (void) ajStrTrimC (&code, white);
      /* ajDebug("code:  '%S'\n", code); */
      (void) ajStrTrimC (&desc, white);
      /* ajDebug ("desc:  '%S'\n", desc); */
      /* ajDebug ("test '%S' code: '%S' desc: %S'\n", repstr, code, desc); */

      if (ajStrMatch(code, repstr) ||
	  (!exactcase && ajStrMatch(code, repstr))) {
	ifound = 1;
	(void) ajStrAssS (&hitstr1, code);
	/* ajDebug ("code match '%S'\n", code); */
	break;
      }
      if (ajStrMatch(desc, repstr) ||
	  (!exactcase && ajStrMatchCase(desc, repstr))) {
	jfound = 1;
	(void) ajStrAssS (&hitstr2, code);
	/* ajDebug ("desc match '%S'\n", desc); */
	break;
      }
      if (ajStrPrefix(code, repstr) ||
	  (!exactcase && ajStrPrefixCase(code, repstr))) {
	ifound++;
	(void) ajStrAssS (&hitstr1, code);
	/* ajDebug ("code match '%S'\n", code); */
      }
      if (ajStrPrefix(desc, repstr) ||
	  (!exactcase && ajStrPrefixCase(desc, repstr))) {
	jfound++;
	(void) ajStrAssS (&hitstr2, code);
	/* ajDebug ("desc match '%S'\n", desc); */
      }

      (void) ajStrTokenClear (&codehandle);
      codehandle = NULL;
    } /* end of while */

    (void) ajStrTokenClear(&codehandle);

    if (ifound == 1) {
      hitstr = ajStrDup(hitstr1); /* don't assign - needed for list */
      ajListstrPushApp (list, hitstr);
    }
    else if (jfound == 1) {
      hitstr = ajStrDup(hitstr2); /* don't assign - needed for list */
      ajListstrPushApp (list, hitstr);
    }
    else {
      if (ifound || jfound)
	ajErr("'%S' is ambiguous", repstr);
      else
	ajErr("'%S' is not a valid list option", repstr);
      ok = ajFalse;
      break;
    }
    (void) ajStrTokenClear (&handle);
  }
  (void) ajStrTokenClear (&rephandle);
  (void) ajStrDelReuse (&repstr);

  ilen = ajListstrLength(list);
  ajDebug ("Found %d matches OK: %b min: %d max: %d\n",
	   ilen, ok, min, max);
  if (ok && ilen >= min && ilen <= max) {
    AJCNEW0(val, ilen+1);
    for (k = 0; k < ilen; k++) {
      (void) ajListstrPop (list, &val[k]);
      ajDebug ("Accept: '%S'\n", val[k]);
    }
  }

  ajDebug ("Found %d matches\n", ilen);
  ajDebug ("List length now %d\n", ajListstrLength(list));
  if (ok) ajDebug ("before return val[0] '%S'\n", val[0]);

  /* do not delete hitstr - it is copied as the last list item stored in val */
  ajListstrDel (&list);
  (void) ajStrDelReuse (&delim);
  (void) ajStrDelReuse (&codedelim);
  (void) ajStrDelReuse (&line);
  (void) ajStrDelReuse (&code);
  (void) ajStrDelReuse (&desc);
  ajStrDel(&hitstr1);
  ajStrDel(&hitstr2);
  
  if (ok) ajDebug ("before return val[0] '%S'\n", val[0]);
  if (!ok)
    return NULL;

  return val;
}

/* @funcstatic acdSelectValue ************************************************
**
** Checks the user setting against the select set of codes
**
** An unambiguous match to the codes counts as valid.
** If this fails, an unambiguous match to the descriptions counts.
** 
** @param [r] thys [AcdPAcd] ACD Object
** @param [r] min [ajint] Minimum number of values required
** @param [r] max [ajint] Maximum number of values required
** @param [r] reply [AjPStr] Default value
** @return [AjPStr*] Array of accepted matches, ending with a NULL.
** @@
******************************************************************************/

static AjPStr* acdSelectValue (AcdPAcd thys, ajint min, ajint max, AjPStr reply) {

  AjPStr* val = NULL;

  AjPStr delim;
  AjPStr value;
  AjBool exactcase;
  AjPStrTok handle;
  AjPStrTok rephandle;
  static AjPStr line = NULL;
  static AjPStr code = NULL;
  static AjPStr desc = NULL;
  AjPList list = NULL;
  static AjPStr repstr = NULL;
  static AjPStr hitstr = NULL;
  AjPStr hitstr2 = NULL;
  static AjPStr repdelim = NULL;
  static char* white = " \t\n\r";
  ajint i = 0;
  ajint k = 0;

  ajint jfound = 0;
  ajint icnt = 0;
  ajint ilen;

  AjBool ok = ajTrue;

  list = ajListstrNew();

  (void) acdAttrToBool(thys, "casesensitive", ajFalse, &exactcase);

  delim = acdAttrValue (thys, "delimiter");
  if (!ajStrLen(delim))
    (void) ajStrAssC(&delim, ";");

  if (!repdelim) {
    repdelim = ajStrNewL(10);
    (void) ajStrAssC (&repdelim, ",");
    (void) ajStrAppC (&repdelim, white);
  }

  value = acdAttrValue (thys, "value");

  /* ajDebug ("reply: '%S' delim '%S'", reply, repdelim); */

  rephandle = ajStrTokenInit (reply, ajStrStr(repdelim));
  while (ajStrToken (&repstr, &rephandle, NULL)) {

    ajDebug("testing '%S'\n", repstr);
    handle = ajStrTokenInit (value, ajStrStr(delim));
    i = jfound = 0;
    for (icnt=1; ajStrDelim (&desc, &handle, NULL); icnt++) {
      (void) ajStrTrimC (&desc, white);
      /* ajDebug ("desc:  '%S'\n", desc); */
      /* ajDebug ("test '%S' desc: %S'\n", repstr, desc); */
 
      if (ajStrMatch(desc, repstr) ||
	  (!exactcase && ajStrMatchCase(desc, repstr))) {
	jfound = 1;
	(void) ajStrAssS (&hitstr2, desc);
	/* ajDebug ("desc matches '%S'\n", desc); */
	break;
      }
      if (ajStrPrefix(desc, repstr) ||
	  (!exactcase && ajStrPrefixCase(desc,repstr))) {
	jfound++;
	(void) ajStrAssS (&hitstr2, desc);
	/* ajDebug ("desc prefix '%S' matches '%S'\n", repstr, desc); */
      }
      if (ajStrToInt(repstr, &i) && i == icnt) {
	jfound++;
	(void) ajStrAssS (&hitstr2, desc);
	/* ajDebug ("desc number '%S' (%d) matches '%S'\n", repstr, i, desc); */
      }
    } /* end of while */

    if (jfound == 1) {
      hitstr = ajStrDup(hitstr2); /* don't assign - needed for list */
      ajListstrPushApp (list, hitstr);
    }
    else {
      if (jfound)
	ajErr("'%S' is ambiguous", repstr);
      else
	ajErr("'%S' is not a valid list option", repstr);
      ok = ajFalse;
      break;
    }
    (void) ajStrTokenClear (&handle);
  }
  (void) ajStrTokenClear (&rephandle);
  (void) ajStrDelReuse (&repstr);

  ilen = ajListstrLength(list);
  if (ok && ilen >= min && ilen <= max) {
    AJCNEW0(val, ilen+1);
    for (k = 0; k < ilen; k++) {
      (void) ajListstrPop (list, &val[k]);
      /* ajDebug ("Accept: '%S'\n", val[k]); */
    }
  }

  ajDebug ("Found %d matches\n", ilen);

  /* do not delete hitstr - it is copied as the last list item stored in val */
  ajListstrDel (&list);
  (void) ajStrDelReuse (&line);
  (void) ajStrDelReuse (&code);
  (void) ajStrDelReuse (&desc);
  (void) ajStrDelReuse (&hitstr2);

  if (!ok)
    return NULL;

  return val;
}

/* @funcstatic acdAmbigApp ****************************************************
**
** Appends a token to a list, with commas as delimiters. Used to
** build a list of ambiguous matches for messages.
**
** @param [w] pambigList [AjPStr*] List of tokens with ',' delimiter
** @param [r] str [AjPStr] Latest token to add
** @return [void]
** @@
******************************************************************************/

static void acdAmbigApp (AjPStr* pambigList, AjPStr str) {
  if (ajStrLen(*pambigList)) (void) ajStrAppC (pambigList, ",");
  (void) ajStrApp (pambigList, str);
}

/* @funcstatic acdAmbigAppC ***************************************************
**
** Appends a token to a list, with commas as delimiters. Used to
** build a list of ambiguous matches for messages.
**
** @param [w] pambigList [AjPStr*] List of tokens with ',' delimiter
** @param [r] txt [char*] Latest token to add
** @return [void]
** @@
******************************************************************************/

static void acdAmbigAppC (AjPStr* pambigList, char* txt) {
  if (ajStrLen(*pambigList)) (void) ajStrAppC (pambigList, ",");
  (void) ajStrAppC (pambigList, txt);
}

/* @funcstatic acdDataFilename ***********************************************
**
** Sets a default data file name. If no values are provided, it will be
** programname.dat
**
** @param [wP] infname [AjPStr*] Resulting file name
** @param [r] name [AjPStr] Ffile name
** @param [r] ext [AjPStr] File extension
** @return [AjBool] ajTrue if a name was successfully set
** @@
******************************************************************************/

static AjBool acdDataFilename (AjPStr* infname, AjPStr name, AjPStr ext) {

  AjBool ret = ajFalse;

  if (ajStrLen(name))
    ajStrAssS (infname, name);
  else
    ajStrAssS (infname, acdProgram);

  if (ajStrLen(ext))
    ajFileNameExt (infname, ext);
  else
    ajFileNameExtC (infname, "dat");

  return ret;
}
/* @funcstatic acdInFilename *************************************************
**
** Sets a default input file name. If filtering is on, this will be stdin.
** Otherwise it is blank.
**
** @param [wP] infname [AjPStr*] Input file name
** @return [AjBool] ajTrue if a name was successfully set
** @@
******************************************************************************/

static AjBool acdInFilename (AjPStr* infname) {

  AjBool ret = ajFalse;

  if (!acdInFile && acdFilter) {
    (void) ajStrAssC (infname, "stdin");
    ret = ajTrue;
  }
  else
    (void) ajStrAssC (infname, "");

  acdInFile++;

  return ret;
}
/* @funcstatic acdOutFilename *************************************************
**
** Sets a default output file name. If stdout or filtering are on,
** this will be stdout for the first output file.
** Otherwise it is built from the defaults provided.
**
** The base file name is usually specified in the ACD file as name:
** and passed in by the calling acdSet function. The default will
** be the base file name saved from the first input file, or "outfile"
**
** The extension is usually specified in the ACD file as extension:
** and passed in by the calling acdSet function. The default will
** be the program name for the first file, and "out2", "out3" and so
** on for later files.
**
** The recommendation is that the extension should always be provided
** in the ACD file, but that the base file name should be taken from
** the input file in most cases.
**
** @param [wP] outfname [AjPStr*] Input file name
** @param [P] name [AjPStr] Specified base file name
** @param [P] ext [AjPStr] Specified extension
** @return [AjBool] ajTrue if a name was successfully set
** @@
******************************************************************************/

static AjBool acdOutFilename (AjPStr* outfname, AjPStr name, AjPStr ext) {

  AjBool ret = ajFalse;
  static AjPStr myname = NULL;
  static AjPStr myext = NULL;

  ajDebug ("acdOutFilename ('%S', '%S', '%S')\n", *outfname, name, ext);

  if (!acdOutFile && acdStdout){ /* first outfile, running as a filter */
    (void) ajStrAssC (outfname, "stdout");
    ajDebug ("outfile <first> '%S'\n", *outfname);
    acdOutFile++;
    return ajTrue;
  }

  (void) ajStrSet(&myname, name);	 /* use name if given */
  (void) ajStrSet(&myname, acdInFName); /* else use saved name */
  (void) ajStrSetC(&myname, "outfile"); /* if all else fails, use "outfile" */

  (void) ajStrSet(&myext, ext);	/* use extension if given */
  if (!acdOutFile)
    (void) ajStrSet(&myext, acdProgram);
  /* else try program name for first file */
  if (!ajStrLen(myext))		/* if all else fails, use out2 etc. */
    (void) ajFmtPrintS (&myext, "out%d", acdOutFile+1);

  ajDebug (". . . myname '%S', myext '%S'\n", myname, myext);

  if (ajStrLen(myext))
    (void) ajFmtPrintS(outfname, "%S.%S", myname, myext);
  else
    (void) ajStrAssS (outfname, myname);

  acdOutFile++;

  ajDebug ("outfile %d %S.%S\n", acdOutFile, myname, myext);

  (void) ajStrDelReuse(&myname);
  (void) ajStrDelReuse(&myext);

  return ret;
}

/* @funcstatic acdInFileSave *************************************************
**
** For the first call, saves the input filename for use in building output
** file name(s).
**
** @param [P] infname [AjPStr] Input file name
** @return [AjBool] ajTrue if a name was successfully set
** @@
******************************************************************************/

static AjBool acdInFileSave (AjPStr infname) {

  if (acdInFile != 1)
    return ajFalse;

  ajDebug ("acdInFileSave (%S)\n",
	   infname);

  if (!ajStrLen(infname)) return ajFalse;

  (void) ajStrAssS (&acdInFName, infname);
  (void) ajFileNameShorten (&acdInFName);
  (void) ajStrToLower(&acdInFName);

  ajDebug ("acdInFileSave (%S) input file set to '%S'\n",
	   infname, acdInFName);

  return ajTrue;
}

/* @funcstatic acdStrDiff *****************************************************
**
** Returns the second string if different from the first,
** otherwise returns an empty string
** 
** @param [r] str1 [AjPStr] First string
** @param [r] str2 [AjPStr] Second string
** @return [AjPStr] Selected string returned
** @@
******************************************************************************/

static AjPStr acdStrDiff (AjPStr str1, AjPStr str2) {

  static AjPStr emptyStr = NULL;

  if (!emptyStr)
    (void) ajStrAssC(&emptyStr, "");

  if (ajStrMatch (str1, str2))
    return emptyStr;

  return str2;
}

/* @funcstatic acdLog *********************************************************
**
** Writes a message to the .acdlog file
**
** @param [r] fmt [char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
** @@
******************************************************************************/

static void acdLog (char *fmt, ...) {

  va_list args ;

  if (!acdDoLog)
    return;

  if (!acdLogFName) {
    (void) ajFmtPrintS (&acdLogFName, "%S.acdlog", acdProgram);
    acdLogFile = ajFileNewOut (acdLogFName);
    ajFileUnbuffer(acdLogFile);
  }
  
  va_start (args, fmt) ;
  ajFmtVPrintF (acdLogFile, fmt, args);
  va_end (args) ;

  return;
}

/* @funcstatic acdPretty ******************************************************
**
** Writes a pretty formatted version of the .acd syntax
** message to the .acdpretty file
**
** @param [r] fmt [char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
** @@
******************************************************************************/

static void acdPretty (char *fmt, ...) {

  va_list args ;

  if (!acdDoPretty)
    return;

  if (!acdPrettyFName) {
    (void) ajFmtPrintS (&acdPrettyFName, "%S.acdpretty", acdProgram);
    acdPrettyFile = ajFileNewOut (acdPrettyFName);
    ajFileUnbuffer(acdPrettyFile);
  }
  
  va_start (args, fmt) ;
  ajFmtVPrintF (acdPrettyFile, fmt, args);
  va_end (args) ;

  return;
}

/* @funcstatic acdIsQtype *****************************************************
**
** Tests whether an ACD object is a qualifier or parameter type. If not we
** assume it is a keyword type.
**
** @param [r] thys [AcdPAcd] ACD object
** @return [AjBool] ajTrue if the object is a qualifier or parameter
** @@
******************************************************************************/

static AjBool acdIsQtype (AcdPAcd thys) {

  if ((thys->Level == ACD_QUAL) || (thys->Level == ACD_PARAM))
    return ajTrue;

  return ajFalse;
}

/* @funcstatic acdTextFormat ************************************************
**
** Converts backslash codes in a string into special characters
**
** @param [r] text [AjPStr*] Text with backslash codes
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool acdTextFormat (AjPStr* text) {

  (void) ajStrSubstituteCC (text, " \\ ", "\n");

  return ajTrue;
}


/* @func ajAcdDummyFunction ********************************************
**
** Dummy function to catch all unused functions defined in the ajacd
** source file.
**
** @return [void]
**
******************************************************************************/

void ajAcdDummyFunction(void)
{
    AjPStr ajpstr=NULL;
    AcdPAcd acdpacd=NULL;
    float f=0.0;

    acdSetXxxx(acdpacd);	/* template function for acdSet */
    (void) acdQualToFloat(acdpacd, "", 0.0, 0, &f, &ajpstr);
}

/* @func ajAcdPrintType *******************************************************
**
** Report details of all known ACD types
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report
** @return [void]
**
******************************************************************************/

void ajAcdPrintType (AjPFile outf, AjBool full) {

  AcdPType pat;
  AcdPAttr attr;
  AcdPQual qual;
  ajint i;

  ajFmtPrintF (outf, "\n");
  ajFmtPrintF (outf, "# ACD Types\n");
  ajFmtPrintF (outf, "# Name\n");
  ajFmtPrintF (outf, "#     Attribute    Type\n");
  ajFmtPrintF (outf, "#     Qualifier    Type       Default Helptext\n");
  ajFmtPrintF (outf, "AcdType {\n");

  for (i=0; acdType[i].Name; i++)  {
    pat = &acdType[i];
    ajFmtPrintF (outf, "  %-15s", pat->Name);
    ajFmtPrintF (outf, "\n");
    if (full && pat->Attr) {
      ajFmtPrintF (outf, "    attributes {\n");
      for (attr=pat->Attr; attr->Name; attr++) {
	ajFmtPrintF (outf, "      %-12s", attr->Name);
	ajFmtPrintF (outf, " %-10s", acdValNames[attr->Type]);
	ajFmtPrintF (outf, "\n");
       }
     ajFmtPrintF (outf, "    }\n");
    }
    if (pat->Quals) {
      ajFmtPrintF (outf, "    qualifiers {\n");
      for (qual=pat->Quals; qual->Name; qual++) {
	ajFmtPrintF (outf, "      %-12s", qual->Name);
	ajFmtPrintF (outf, " %-10s", qual->Type);
	ajFmtPrintF (outf, " \"%s\"", qual->Default);
	ajFmtPrintF (outf, " \"%s\"", qual->Help);
	ajFmtPrintF (outf, "\n");
      }
      ajFmtPrintF (outf, "    }\n");
    }
  }
  ajFmtPrintF (outf, "}\n");
  
  return;
}

/* @funcstatic acdVocabCheck **************************************************
**
** Checks for a string in a controlled vocabulary of character strings,
** ended with a NULL.
**
** @param [R] str [AjPStr] Test string
** @param [R] vocab [char**] Controlled vocabulary
** @return [AjBool] ajTrue if the string matched on of the words
******************************************************************************/

AjBool acdVocabCheck (AjPStr str, char** vocab) {

  ajint i=0;
  while (vocab[i]) {
    if (ajStrMatchCaseC(str, vocab[i]))
        return ajTrue;
    i++;
  }

  return ajFalse;
}

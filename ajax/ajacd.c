/******************************************************************************
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
******************************************************************************/

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

#define USED_GET 1
#define USED_ACD 2

/*static AjBool acdDebug = 0;*/
/*static AjBool acdDebugSet = 0;*/
/*static AjPStr acdProgram = NULL;*/
static AjBool acdDoHelp = 0;
static AjBool acdDoLog = 0;
static AjBool acdDoPretty = 0;
static AjBool acdDoTable = 0;
static AjBool acdDoValid = 0;
static AjBool acdVerbose = 0;
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
static AjPStr acdInTypeFeatName = NULL;
static AjPStr acdInTypeSeqName = NULL;
/*static AjPStr acdOutFName = NULL;*/
static AjPStr acdExpTmpstr = NULL;

static AjPStr acdLogFName = NULL;
static AjPFile acdLogFile = NULL;
static AjPList acdSecList = NULL;

static AjPStr acdPrettyFName = NULL;
static AjPFile acdPrettyFile = NULL;
static ajint acdPrettyMargin = 0;
static ajint acdPrettyIndent = 2;

static AjPStr acdFName = NULL;
static ajint acdLineNum = 0;
static ajint acdWordNum = 0;
static ajint acdErrorCount = 0;

static AjPStr acdStrName = NULL;

/*
static ajint acdLineCount = 0;
static AjPList acdListCount = NULL;
*/

/* keywords (other than qualifier types) */

typedef enum
{
    QUAL_STAGE, APPL_STAGE, VAR_STAGE, SEC_STAGE, ENDSEC_STAGE, BAD_STAGE
} AcdEStage ;

static AcdEStage acdCurrentStage;

/* Levels as defined in the ACD structure */

enum AcdELevel
{
    ACD_APPL,				/* application definition */
    ACD_PARAM,				/* parameter */
    ACD_QUAL,				/* qualifier */
    ACD_VAR,				/* variable */
    ACD_IF,				/* if test */
    ACD_SEC,				/* start new section */
    ACD_ENDSEC				/* end section */
};

/* Levels as text, only for use in logging report */

static char* acdLevel[] =
{
    "APPL", "PARAM", "QUAL", "VAR",
    "IF", "SEC", "ENDSEC"
};

/* Attribute value types */

enum AcdEValtype
{
    VT_APPL, VT_STR, VT_WORD,
    VT_BOOL, VT_INT, VT_FLOAT,
    VT_NULL
};

/* Attribute value types as text for use in logging report */

static char* acdValNames[] =
{
    "application", "string", "word",
    "boolean", "integer", "float",
    NULL
};

/* @datastatic AcdPAttr *******************************************************
**
** ACD attribute definition structure
**
** @alias AcdPAttr
** @alias AcdOAttr
**
** @attr Name [char*] Attribute name
** @attr Type [enum AcdEValtype] Type code
** @attr Help [char*] Descriptive short text for documentation
** @@
******************************************************************************/

typedef struct AcdSAttr
{
    char* Name;
    enum AcdEValtype Type;
    char* Help;
} AcdOAttr, *AcdPAttr;


/* @datastatic AcdPQual *******************************************************
**
** ACD qualifier definition structure
**
** @alias AcdSQual
** @alias AcdOQual
**
** @attr Name [char*] Qualifier name
** @attr Default [char*] Default value as a string
** @attr Type [char*] Type boolean, integer, float or string
** @attr Help [char*] Help text for documentation and for -help output
** @@
******************************************************************************/

typedef struct AcdSQual
{
    char* Name;
    char* Default;
    char* Type;
    char* Help;
} AcdOQual, *AcdPQual;

/* @datastatic AcdPTableItem **************************************************
**
** Help table structure
**
** @alias AcdSTableItem
** @alias AcdOTableItem
**
** @attr Qual [AjPStr] Qualifier name
** @attr Help [AjPStr] Help text
** @attr Valid [AjPStr] Valid input
** @attr Expect [AjPStr] Expected value(s)
** @@
******************************************************************************/

typedef struct AcdSTableItem
{
    AjPStr Qual;
    AjPStr Help;
    AjPStr Valid;
    AjPStr Expect;
} AcdOTableItem, *AcdPTableItem;

/* @datastatic AcdPAcd ********************************************************
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
    ajint Type;			    /* acdType or acdKeywords index */
    ajint NAttr;
    AjPStr* AttrStr;		   /* type attribs: qual/param only */
    ajint SAttr;		    /* specially set for some types */
    AcdPAttr SetAttr;
    AjPStr* SetStr;
    AjPStr* DefStr;			/* default attrib values */
    AjBool Defined;			/* set when defined by user */
    ajint Used;
    AjBool Assoc;
    ajint LineNum;
    struct AcdSAcd* AssocQuals;
    AjPStr StdPrompt;			/* Standard prompt if none is given */
    AjPStr OrigStr;
    AjPStr ValStr;
    void* Value;
} AcdOAcd, *AcdPAcd;


/* @datastatic AcdPSection ****************************************************
**
** ACD section definition
**
** @alias AcdSSection
** @alias AcdOSection
**
** @attr Name [char*] Section name
** @attr Description [char*] Section description
** @attr Type [char*] Section type "page"
** @@
******************************************************************************/

typedef struct AcdSSection
{
    char* Name;
    char* Description;
    char* Type;
} AcdOSection, *AcdPSection;

AcdOSection acdSecInput[] =
{
    {"input", "input Section", "page"},
    {NULL, NULL, NULL}
};
AcdOSection acdSecOutput[] =
{
    {"output", "output Section", "page"},
    {NULL, NULL, NULL}
};
AcdOSection acdSecRequired[] = 
{
    {"required", "required Section", "page"},
    {NULL, NULL, NULL}
};
AcdOSection acdSecAdvanced[] = 
{
    {"advanced", "advanced Section", "page"},
    {NULL, NULL, NULL}
};

/* @datastatic AcdPType *******************************************************
**
** ACD data type structure
**
** @alias AcdSType
** @alias AcdOType
**
** @attr Name [char*] Attribute type
** @attr Group [char*] Attribute group
** @attr Section [AcdPSection] Expected section name
** @attr Attr [AcdPAttr] Type-specific attributes
** @attr TypeSet [(void*)] function to set value and prompt user
** @attr Quals [AcdPQual] Type-specific qualifiers
** @attr Valid [char*] Valid data help message and description for
**                     documentation
** @@
******************************************************************************/

typedef struct AcdSType
{
    char* Name;				/* type name */
    char* Group;			/* group name */
    AcdPSection Section;		/* section */
    AcdPAttr Attr;			/* type-specific attributes */
    void (*TypeSet)(AcdPAcd thys); /* function to set value and prompt user */
    AcdPQual Quals;	           /* type-specific associated qualifiers */
    AjBool Stdprompt;			/* Expect a standard prompt */
    char* Valid;			/* Valid data help message */
} AcdOType, *AcdPType;			/*  */

static AjBool* acdParamSet;

static AcdPAcd acdNewCurr = NULL;
static AcdPAcd acdMasterQual = NULL;

/*
AcdOAcd acdList = {NULL, NULL, NULL, 0, ACD_APPL, 0, 0, NULL, 0, NULL,
		   NULL, NULL, 0, 0, NULL, NULL, NULL};
*/
static AcdPAcd acdList = NULL;
static AcdPAcd acdListLast = NULL;
static AcdPAcd acdListCurr = NULL;
static AcdPAcd acdProcCurr = NULL;
static AcdPAcd acdSetCurr = NULL;

static ajint acdNParam=0;

static void      acdAmbigApp (AjPStr* pambiglist, AjPStr str);
static void      acdAmbigAppC (AjPStr* pambiglist, char* txt);
static void      acdArgsParse (ajint argc, char *argv[]);
static void      acdArgsScan (ajint argc, char *argv[]);
static ajint     acdAttrCount (ajint itype);
static ajint     acdAttrKeyCount (ajint ikey);
static ajint     acdAttrListCount (AcdPAttr attr);
static AjBool    acdAttrResolve (AcdPAcd thys, char *attr, AjPStr *result);
static AjBool    acdAttrToBool (AcdPAcd thys,
				char *attr, AjBool defval, AjBool *result);
static AjBool    acdAttrToBoolTest (AcdPAcd thys,
				char *attr, AjBool defval, AjBool *result);
static AjBool    acdAttrToFloat (AcdPAcd thys,
				 char *attr, float defval, float *result);
static AjBool    acdAttrTest (AcdPAcd thys, char *attrib);
static AjBool    acdAttrToInt (AcdPAcd thys,
			       char *attr, ajint defval, ajint *result);
static AjBool    acdAttrToStr (AcdPAcd thys,
			       char *attr, char* defval, AjPStr *result);
static AjPStr    acdAttrValue (AcdPAcd thys, char *attrib);
static AjBool    acdAttrValueStr (AcdPAcd thys, char *attrib, char* def,
				  AjPStr *str);
static void      acdBadRetry (AcdPAcd thys);
static void      acdBadVal (AcdPAcd thys, AjBool required, char *fmt, ...);
static AjBool    acdCodeDef (AcdPAcd thys, AjPStr *msg);
static AjBool    acdCodeGet (AjPStr code, AjPStr *msg);
static void      acdCodeInit (void);
static AjBool    acdDataFilename (AjPStr* datafname, AjPStr name, AjPStr ext);
static AjBool    acdDef (AcdPAcd thys, AjPStr value);
static void      acdError (char* fmt, ...);
static void      acdErrorAcd (AcdPAcd thys, char* fmt, ...);

/* expression processing */

static AjBool    acdExpPlus (AjPStr* result, AjPStr str);
static AjBool    acdExpMinus (AjPStr* result, AjPStr str);
static AjBool    acdExpStar (AjPStr* result, AjPStr str);
static AjBool    acdExpDiv (AjPStr* result, AjPStr str);
static AjBool    acdExpNot (AjPStr* result, AjPStr str);
static AjBool    acdExpEqual (AjPStr* result, AjPStr str);
static AjBool    acdExpNotEqual (AjPStr* result, AjPStr str);
static AjBool    acdExpGreater (AjPStr* result, AjPStr str);
static AjBool    acdExpLesser (AjPStr* result, AjPStr str);
static AjBool    acdExpAnd (AjPStr* result, AjPStr str);
static AjBool    acdExpOr (AjPStr* result, AjPStr str);
static AjBool    acdExpCond (AjPStr* result, AjPStr str);
static AjBool    acdExpCase (AjPStr* result, AjPStr str);
static AjBool    acdExpFilename (AjPStr* result, AjPStr str);
static AjBool    acdExpExists (AjPStr* result, AjPStr str);

static AcdPAcd   acdFindAcd (AjPStr name, AjPStr token, ajint pnum);
static AcdPAcd   acdFindAssoc (AcdPAcd thys, AjPStr name, AjPStr altname);
static ajint     acdFindAttr (AcdPAttr attr, AjPStr attrib);
static ajint     acdFindAttrC (AcdPAttr attr, char* attrib);
static AcdPAcd   acdFindItem (AjPStr item, ajint number);
static ajint     acdFindKeyC (char* key);
static AcdPAcd   acdFindParam (ajint PNum);
static AcdPAcd   acdFindQual (AjPStr qual, AjPStr noqual, AjPStr master,
			      ajint PNum, ajint *iparam);
static AcdPAcd   acdFindQualAssoc (AcdPAcd pa, AjPStr qual, AjPStr noqual,
				   ajint PNum);
static AcdPAcd   acdFindQualMaster (AjPStr qual, AjPStr noqual, AjPStr master,
				    ajint PNum);
static ajint     acdFindType (AjPStr type);
static ajint     acdFindTypeC (char* type);
static AjBool    acdFunResolve (AjPStr* result, AjPStr str);
static AjBool    acdGetAttr (AjPStr* result, AjPStr name, AjPStr attrib);
static void*     acdGetValue (char *token, char* type);
static AjBool    acdGetValueAssoc (AcdPAcd thys, char *token, AjPStr *result);
static void*     acdGetValueNum (char *token, char* type, ajint pnum);
static AjPStr    acdGetValStr (char *token);
static void      acdHelp (void);
static void      acdHelpAppend (AcdPAcd thys, AjPStr* str, char flag);
static void      acdHelpAssoc (AcdPAcd thys, AjPStr *str, char *name);
static void      acdHelpAssocTable (AcdPAcd thys, AjPList tablist, char flag);
static AjBool    acdHelpCodeDef (AcdPAcd thys, AjPStr *msg);
static void      acdHelpShow (AjPStr str, char* title);
static void      acdHelpTable (AcdPAcd thys, AjPList tablist, char flag);
static void      acdHelpTableShow (AjPList tablist, char* title);
static void      acdHelpText (AcdPAcd thys, AjPStr* msg);
static void      acdHelpValid (AcdPAcd thys, AjPStr *str);
static AjBool    acdHelpVarResolve (AjPStr* str, AjPStr src);
static AjBool    acdInFilename (AjPStr* infname);
static AjBool    acdInFileSave (AjPStr infname);
static AjBool    acdInTypeFeat (AjPStr* intype);
static AjBool    acdInTypeFeatSave (AjPStr intype);
static AjBool    acdInTypeFeatSaveC (char* intype);
static AjBool    acdInTypeSeq (AjPStr* intype);
static AjBool    acdInTypeSeqSave (AjPStr intype);
static AjBool    acdIsLeftB (AjPList listwords);
static AjBool    acdIsParam (char* arg, AjPStr* param, ajint* iparam,
			     AcdPAcd* acd);
static AjBool    acdIsParamValue (AjPStr pval);
static ajint     acdIsQual (char* arg, char* arg2, ajint *iparam,
			    AjPStr *pqual, AjPStr *pvalue,
			    ajint* number, AcdPAcd* acd);
static AjBool    acdIsQtype (AcdPAcd thys);
static AjBool    acdIsRequired (AcdPAcd thys);
static AjBool    acdIsRightB (AjPStr* pstr, AjPList listwords);
static AjBool    acdIsStype (AcdPAcd thys);
static void      acdListAttr (AcdPAttr attr, AjPStr* valstr, ajint nattr);
static void      acdListPrompt (AcdPAcd thys);
static void      acdListReport (char *title);
static AjPStr*   acdListValue (AcdPAcd thys, ajint min, ajint max,
			       AjPStr reply);
static void      acdLog (char *fmt, ...);
static AcdPAcd   acdNewAcd (AjPStr name, AjPStr token, ajint itype);
static AcdPAcd   acdNewAcdKey (AjPStr name, AjPStr token, ajint ikey);
static AcdPAcd   acdNewAppl (AjPStr name);
static AcdPAcd   acdNewEndsec (AjPStr name);
static AcdPAcd   acdNewQual (AjPStr name, AjPStr token, AjPStr* type,
			     ajint pnum);
static AcdPAcd   acdNewQualQual (AjPStr name, AjPStr* type);
static AcdPAcd   acdNewSec (AjPStr name);
static AcdPAcd   acdNewVar (AjPStr name);
static ajint     acdNextParam (ajint pnum);
static AjBool    acdNotLeftB (AjPList listwords);
static AjBool    acdOutDirectory (AjPStr* outdir);
static AjBool    acdOutFilename (AjPStr* outfname, AjPStr name, AjPStr ext);
static void      acdParse (AjPList listwords, AjPList listcount);
static void      acdParseAlpha (AjPList listwords, AjPStr* pword);
static void      acdParseAttributes (AcdPAcd acd, AjPList listwords);
static void      acdParseName (AjPList listwords, AjPStr* pword);
static AjPStr    acdParseValue (AjPList listwords);
static void      acdPretty (char *fmt, ...);
static void      acdPrettyShift ();
static void      acdPrettyWrap (ajint left, char *fmt, ...);
static void      acdPrettyUnShift ();
static void      acdPrintCalcAttr (AjPFile outf, AjBool full,
				   char* acdtype, AcdOAttr calcattr[]);
static void      acdProcess (void);
static void      acdPromptAlign (AcdPAcd thys);
static void      acdPromptCodon (AcdPAcd thys);
static void      acdPromptCpdb (AcdPAcd thys);
static void      acdPromptDirlist (AcdPAcd thys);
static void      acdPromptFeat (AcdPAcd thys);
static void      acdPromptFeatout (AcdPAcd thys);
static void      acdPromptFilelist (AcdPAcd thys);
static void      acdPromptGraph (AcdPAcd thys);
static void      acdPromptInfile (AcdPAcd thys);
static void      acdPromptOutfile (AcdPAcd thys);
static void      acdPromptReport (AcdPAcd thys);
static void      acdPromptScop (AcdPAcd thys);
static void      acdPromptSeq (AcdPAcd thys);
static void      acdPromptSeqout (AcdPAcd thys);
static void      acdQualParse (AjPStr* pqual, AjPStr* pnoqual,
			       AjPStr* pqmaster, ajint* number);
static AjBool    acdQualToBool (AcdPAcd thys, char *qual,
				AjBool defval, AjBool *result, AjPStr* valstr);
static AjBool    acdQualToFloat (AcdPAcd thys, char *qual,
			      float defval, ajint precision,
			      float *result, AjPStr* valstr);
static AjBool    acdQualToInt (AcdPAcd thys, char *qual,
			       ajint defval, ajint *result, AjPStr* valstr);
static AjBool    acdQualToSeqbegin (AcdPAcd thys, char *qual,
				    ajint defval, ajint *result,
				    AjPStr* valstr);
static AjBool    acdQualToSeqend   (AcdPAcd thys, char *qual,
				    ajint defval, ajint *result,
				    AjPStr* valstr);
static AjPTable  acdReadGroups (void);
static void      acdReadSections (AjPTable* typetable, AjPTable* infotable);
static AjBool    acdReplyInit (AcdPAcd thys, char *defval, AjPStr* reply);
static AjBool    acdSet (AcdPAcd thys, AjPStr* attrib, AjPStr value);
static void      acdSetAll (void);
static AjBool    acdSetDef (AcdPAcd thys, AjPStr value);
static AjBool    acdSetDefC (AcdPAcd thys, char* value);
static AjBool    acdSetQualAppl (AcdPAcd thys, AjBool val);
static AjBool    acdSetQualDefBool (AcdPAcd thys, char* name, AjBool value);
static AjBool    acdSetQualDefInt (AcdPAcd thys, char* name, ajint value);
static AjBool    acdSetKey (AcdPAcd thys, AjPStr* attrib, AjPStr value);
static AjBool    acdSetVarDef (AcdPAcd thys, AjPStr value);
static void      acdSelectPrompt (AcdPAcd thys);
static AjPStr*   acdSelectValue (AcdPAcd thys, ajint min, ajint max,
				 AjPStr reply);
static AcdEStage acdStage (AjPStr token);
static AcdPAcd   acdTestAssoc (AcdPAcd thys, AjPStr name, AjPStr altname);
static void      acdTestAssocUnknown (AjPStr name);
static AjBool    acdTestQualC (char *name);
static void      acdTestUnknown (AjPStr name, AjPStr alias, ajint pnum);
static AjBool    acdTextFormat (AjPStr* text);
static void      acdTokenToLower (char *token, ajint* number);
static AjBool    acdUserGet (AcdPAcd thys, AjPStr* reply);
static AjBool    acdUserGetPrompt (char* prompt, AjPStr* reply);
static void      acdValidAppl (AcdPAcd thys);
static void      acdValidApplGroup (AjPStr groups);
static void      acdValidSection (AcdPAcd thys);
static void      acdValidQual (AcdPAcd thys);
static AjBool    acdValIsBool (char* arg);
static AjBool    acdVarResolve (AjPStr* str);
static AjBool    acdVarSplit (AjPStr var, AjPStr* name, AjPStr* attrname);
static AjBool    acdVarTest (AjPStr var);
static AjBool    acdVocabCheck (AjPStr str, char** vocab);
static void      acdWarn (char* fmt, ...);
static AjBool    acdWordNext (AjPList listwords, AjPStr* pword);
static AjBool    acdWordNextLower (AjPList listwords, AjPStr* pword);
static AjBool    acdWordNextName (AjPList listwords, AjPStr* pword);

/* @datastatic AcdPExpList ****************************************************
**
** Exression list structure for named expressions @plus etc.
**
** @alias AcdSExpList
** @alias AcdOExpList
**
** @attr Name [char*] Expression name
** @attr Func [(AjBool*)] Function to evaluate expression
** @@
******************************************************************************/

typedef struct AcdSExpList
{
    char* Name;
    AjBool (*Func) (AjPStr *result, AjPStr str);
} AcdOExpList, *AcdPExpList;

/* @funclist acdExpList *******************************************************
**
** Functions for processing expressions in ACD dependencies
**
******************************************************************************/

static AcdOExpList explist[] =
{
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
static void acdHelpValidFilelist (AcdPAcd thys, AjPStr* str);
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
static void acdHelpExpectFilelist (AcdPAcd thys, AjPStr* str);
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
static void acdSetAlign (AcdPAcd thys);
static void acdSetArray (AcdPAcd thys);
static void acdSetBool (AcdPAcd thys);
static void acdSetCodon (AcdPAcd thys);
static void acdSetDirlist (AcdPAcd thys);
static void acdSetDatafile (AcdPAcd thys);
static void acdSetDirectory (AcdPAcd thys);
static void acdSetFeat (AcdPAcd thys);
static void acdSetFeatout (AcdPAcd thys);
static void acdSetFilelist (AcdPAcd thys);
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

enum AcdEDef
{
    DEF_DEFAULT,
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

AcdOAttr acdAttrDef[] =
{
    {"default", VT_STR, "Default value"},
    {"prompt", VT_STR, "Prompt"},
    {"information", VT_STR, "Info for menus etc."},
    {"code", VT_STR, "Code for prompt"},
    {"help", VT_STR, "Text for help+docs"},
    {"required", VT_BOOL, "Prompt if missing?"},
    {"optional", VT_BOOL, "Required with -options"},
    {"missing", VT_BOOL, "Allow with no value?"},
    {"parameter", VT_BOOL, "Accept as a parameter"},
    {"valid", VT_STR, "Help: allowed values"},
    {"expected", VT_STR, "Help: expected value"},
    {"comment", VT_STR, "Comment for AppLab"},
    {"corba", VT_STR, "CORBA spec for AppLab"},
    {"style", VT_STR, "Style for AppLab"},
    {"needed", VT_BOOL, "Include in GUI form"},
    {NULL, VT_NULL, NULL}
};

/* Type-specific attributes
** each must end with "NULL, VT_NULL" to define the end of the list
*/

AcdOAttr acdAttrXxxx[] =
{
    {NULL, VT_NULL}
};

AcdOAttr acdAttrAppl[] =
{
    {"documentation", VT_STR, "Short overview of the application function"},
    {"groups", VT_STR, "(Not used by ACD) Groups for use by EMBOSS wossname"},
    {"gui", VT_STR, "(Not used by ACD) Suitability for launching in a GUI"},
    {"batch", VT_STR, "(Not used by ACD) Suitability for running in batch"},
    {"external", VT_STR, "(Not used by ACD) Third party tool(s) required"},
    {"cpu", VT_STR, "(Not used by ACD) Estimated maximum CPU usage"},
    {"comment", VT_STR, "(Not used by ACD) Comment"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrAlign[] =
{
    {"type", VT_STR, "[P]rotein or [N]ucleotide"},
    {"taglist", VT_STR, "Extra tags to report"},
    {"minseqs", VT_INT, "Minimum number of extra tags default:0"},
    {"maxseqs", VT_INT, "Maximum number of extra tags default:0"},
    {"multiple", VT_BOOL, "More than one alignment in one file default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrArray[] =
{
    {"minimum", VT_FLOAT, "Minimum value default:-FLT_MAX"},
    {"maximum", VT_FLOAT, "Maximum value default:FLT_MAX"},
    {"increment", VT_FLOAT, "(Not used by ACD) Increment for GUIs"},
    {"precision", VT_INT, "(Not used by ACD) Floating precision for GUIs"},
    {"warnrange", VT_BOOL, "Warning if values are out of range default:Y"},
    {"size", VT_INT, "Number of values required default:1"},
    {"sum", VT_FLOAT, "Total for all values default:1.0"},
    {"tolerance", VT_FLOAT, "Tolerance (sum +/- tolerance) of the total "
	 "default:0.01"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrBool[] =
{
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrCodon[] =
{
    {"name", VT_STR, "Codon table name default:Ehum.cut"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrCpdb[] =
{
    {"name", VT_STR, "Default cleaned PDB entry default:1azu"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrDatafile[] =
{
    {"name", VT_STR, "Default file base name"},
    {"extension", VT_STR, "Default file extension"},
    {"directory", VT_STR, "Default installed data directory"},
    {"standardtype", VT_STR, "Standard named file type"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrDirectory[] =
{
    {"fullpath", VT_BOOL, "Require full path in value default:N"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrDirlist[] =
{
    {"fullpath", VT_BOOL, "Require full path in value default:N"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrEndsec[] =
{
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrFeat[] =
{
    {"type", VT_STR, "Feature type (protein, nucleotide, etc.)"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrFeatout[] =
{
    {"name", VT_STR, "Default base file name (-ofname preferred)"},
    {"extension", VT_STR, "Default file extension (-offormat preferred)"},
    {"type", VT_STR, "Feature type (protein, nucleotide, etc.)"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrFilelist[] =
{
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {"standardtype", VT_STR, "Standard named file type"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrFloat[] =
{
    {"minimum", VT_FLOAT, "Minimum value default:-FLT_MAX"},
    {"maximum", VT_FLOAT, "Maximum value default:FLT_MAX"},
    {"increment", VT_FLOAT, "(Not used by ACD) Increment for GUIs"},
    {"precision", VT_INT, "Precision for printing values default:3"},
    {"warnrange", VT_BOOL, "Warning if values are out of range default:Y"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrGraph[] =
{
    {"standardtype", VT_STR, "(Not used by ACD) Standard named graph type"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrGraphxy[] =
{
    {"standardtype", VT_STR, "(Not used by ACD) Standard named graph type"},
    {"multiple", VT_INT, "Number of graphs default:1"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrInt[] =
{
    {"minimum", VT_INT, "Minimum value default:INT_MIN"},
    {"maximum", VT_INT, "Maximum value default:INT_MAX"},
    {"increment", VT_INT, "(Not used by ACD) Increment for GUIs"},
    {"warnrange", VT_BOOL, "Warning if values are out of range default:Y"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrInfile[] =
{
    {"standardtype", VT_STR, "(Not used by ACD) Standard named file type"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrList[] =
{
    {"minimum", VT_INT, "Minimum number of selections default:1"},
    {"maximum", VT_INT, "Maximum number of selections default:1"},
    {"button", VT_BOOL, "(Not used by ACD) Prefer checkboxes in GUI"},
    {"casesensitive", VT_BOOL, "Case sensitive default:N"},
    {"header", VT_STR, "Header description for list"},
    {"delimiter", VT_STR, "Delimiter for parsing values default:';'"},
    {"codedelimiter", VT_STR, "Delimiter for parsing codes default:':'"},
    {"values", VT_STR, "Codes and values with delimiters"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrMatrix[] =
{
    {"pname", VT_STR, "Default name for protein matrix default:EBLOSUM62"},
    {"nname", VT_STR, "Default name for nucleotide matrix default:EDNAFULL"},
    {"protein", VT_BOOL, "Protein matrix default:Y"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrMatrixf[] =
{
    {"pname", VT_STR, "Default name for protein matrix default:EBLOSUM62"},
    {"nname", VT_STR, "Default name for nucleotide matrix default:EDNAFULL"},
    {"protein", VT_BOOL, "Protein matrix default:Y"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrOutfile[] =
{
    {"name", VT_STR, "Default file name"},
    {"extension", VT_STR, "Default file extension"},
    {"standardtype", VT_STR, "(Not used by ACD) Standard named file type"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {"append", VT_BOOL, "Append to an existing file default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrRange[] =
{
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrRegexp[] =
{
    {"minlength", VT_INT, "Minimum pattern length default:1"},
    {"maxlength", VT_INT, "Maximum pattern length default:INT_MAX"},
    {"upper", VT_BOOL, "Convert to upper case default:N"},
    {"lower", VT_BOOL, "Convert to lower case default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrReport[] =
{
    {"type", VT_STR, "[P]rotein or [N]ucleotide"},
    {"taglist", VT_STR, "Extra tag names to report "},
    {"mintags", VT_INT, "Minimum number extra tags default:0"},
    {"multiple", VT_BOOL, "Multiple sequences in one report"},
    {"precision", VT_INT, "Score precision default:3"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrScop[] =
{
    {"name", VT_STR, "SCOP entry name default:d3sdha"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSec[] =
{
    {"information", VT_STR, "(Not used by ACD) Section description"},
    {"type", VT_STR, "(Not used by ACD) Type (frame, page)"},
    {"comment", VT_STR, "(Not used by ACD) Free text comment"},
    {"border", VT_INT, "(Not used by ACD) Border width default:1"},
    {"side", VT_STR, "(Not used by ACD) Side (top, bottom, left, right) "
	 "for type:frame"},
    {"folder", VT_STR, "(Not used by ACD) Folder name for type:page"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSelect[] =
{
    {"minimum", VT_INT, "Minimum number of selections default:1"},
    {"maximum", VT_INT, "Maximum number of selections default:1"},
    {"button", VT_BOOL, "(Not used by ACD) Prefer radiobuttons in GUI"},
    {"casesensitive", VT_BOOL, "Case sensitive matching default:N"},
    {"header", VT_STR, "Header description for selection list"},
    {"delimiter", VT_STR, "Delimiter for parsing values default:':'"},
    {"values", VT_STR, "Values with delimiters"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSeq[] =
{
    {"type", VT_STR, "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, "Read features if any default:N"},
    {"entry", VT_BOOL, "Read whole entry text default:N"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSeqall[] =
{
    {"type", VT_STR, "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, "Read features if any default:N"},
    {"entry", VT_BOOL, "Read whole entry text default:N"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSeqout[] =
{
    {"name", VT_STR, "Output base name (-osname preferred)"},
    {"extension", VT_STR, "Output extension (-osextension preferred)"},
    {"features", VT_BOOL, "Write features if any default:N"},
    {"type", VT_STR, "Output sequence type (protein, gapprotein, etc.)"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSeqoutall[] =
{
    {"name", VT_STR, "Output base name (-osname preferred)"},
    {"extension", VT_STR, "Output extension (-osextension preferred)"},
    {"features", VT_BOOL, "Writee features if any default:N"},
    {"type", VT_STR, "Output sequence type (protein, gapprotein, etc.)"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSeqoutset[] =
{
    {"name", VT_STR, "Output base name (-osname preferred)"},
    {"extension", VT_STR, "Output extension (-osextension preferred)"},
    {"features", VT_BOOL, "Write features if any default:N"},
    {"type", VT_STR, "Output sequence type (protein, gapprotein, etc.)"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrSeqset[] =
{
    {"type", VT_STR, "Input sequence type (protein, gapprotein, etc.)"},
    {"features", VT_BOOL, "Read features if any default:N"},
    {"nullok", VT_BOOL, "Can accept a null filename as 'no file' default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrString[] =
{
    {"minlength", VT_INT, "Minimum length default:0"},
    {"maxlength", VT_INT, "Maximum length default:INT_MAX"},
    {"pattern", VT_STR, "Regular expression for validation default:''"},
    {"upper", VT_BOOL, "Convert to upper case default:N"},
    {"lower", VT_BOOL, "Convert to lower case default:N"},
    {NULL, VT_NULL, NULL}
};

AcdOAttr acdAttrVar[] =
{
    {NULL, VT_NULL, NULL}
};

/* Calculated attributes */

static AcdOAttr acdCalcFeat[] =
{
    {"fbegin", VT_INT, "The beginning of the selection of the feature table"},
    {"fend", VT_INT, "The end of the selection of the feature table"},
    {"flength", VT_INT, "Total length of sequence (fsize is features count)"},
    {"fprotein", VT_BOOL, "Boolean, indicates if feature table is protein"},
    {"fnucleic", VT_BOOL, "Boolean, indicates if feature table is DNA"},
    {"fname", VT_STR, "The name of the feature table"},
    {"fsize", VT_STR, "Integer, number of features"},
    {NULL, VT_NULL, NULL}
};

static AcdOAttr acdCalcRegexp[] =
{
    {"length", VT_INT, "The length of the regular expression"},
    {NULL, VT_NULL, NULL}
};

static AcdOAttr acdCalcSeq[] =
{
    {"begin", VT_INT, "The beginning of the selection of the sequence"},
    {"end", VT_INT, "The end of the selection of the sequence"},
    {"length", VT_INT, "The total length of the sequence"},
    {"protein", VT_BOOL, "Boolean, indicates if sequence is protein"},
    {"nucleic", VT_BOOL, "Boolean, indicates if sequence is DNA"},
    {"name", VT_STR, "The name/ID/accession # of the sequence"},
    {NULL, VT_NULL, NULL}
};

static AcdOAttr acdCalcSeqall[] =
{
    {"begin", VT_INT, "The beginning of the selection of the sequence"},
    {"end", VT_INT, "The end of the selection of the sequence"},
    {"length", VT_INT, "The total length of the sequence"},
    {"protein", VT_BOOL, "Boolean, indicates if sequence is protein"},
    {"nucleic", VT_BOOL, "Boolean, indicates if sequence is DNA"},
    {"name", VT_STR, "The name/ID/accession # of the sequence"},
    {NULL, VT_NULL, NULL}
};

static AcdOAttr acdCalcSeqset[] =
{
    {"begin", VT_INT, "The beginning of the selection of the sequence"},
    {"end", VT_INT, "The end of the selection of the sequence"},
    {"length", VT_INT, "The maximum length of the sequence set"},
    {"protein", VT_BOOL, "Boolean, indicates if sequence set is protein"},
    {"nucleic", VT_BOOL, "Boolean, indicates if sequence set is DNA"},
    {"name", VT_STR, "The name of the sequence set"},
    {"totweight", VT_FLOAT, "Float, total sequence weight for a set"},
    {"count", VT_INT, "Integer, number of sequences in the set"},
    {NULL, VT_NULL, NULL}
};

static AcdOAttr acdCalcString[] =
{
    {"length", VT_INT, "The length of the string"},
    {NULL, VT_NULL, NULL}
};

/* @datastatic AcdPKey ********************************************************
**
** Keywords data structure for non-ACD types (application, variable, section,
** endsection) data
**
** @alias AcdSKey
** @alias AcdOKey
**
** @attr Name [char*] Keyword name
** @attr Stage [AcdEStage] Enumerated stage 
** @attr Attr [AcdPAttr] Type-specifric attributes 
** @attr KetSet [(void*)] function to set value and prompt user
** @@
******************************************************************************/

typedef struct AcdSKey
{
    char* Name;
    AcdEStage Stage;
    AcdPAttr Attr;			/* type-specific attributes */
    void (*KeySet)(AcdPAcd thys); /* function to set value and prompt user */
} AcdOKey, *AcdPKey;

/* @funclist acdKeywords ******************************************************
**
** Processing predefined ACD keywords (application, variable, section,
** endsection)
**
******************************************************************************/

AcdOKey acdKeywords[] =
{
    {"qualifier",   QUAL_STAGE,   NULL,          NULL},
    {"application", APPL_STAGE,   acdAttrAppl,   acdSetAppl},
    {"variable",    VAR_STAGE,    acdAttrVar,    acdSetVar},
    {"section",     SEC_STAGE,    acdAttrSec,    acdSetSec},
    {"endsection",  ENDSEC_STAGE, acdAttrEndsec, acdSetEndsec},
    {NULL, BAD_STAGE, NULL, NULL}
};


/* Type-specific associated qualifiers which can be used positionally
** or numbered if tied to a parameter */

/* "qualifier"  "default" "type" */

AcdOQual acdQualAppl[] =	/* careful: index numbers used in*/
/* acdSetQualAppl */
{
    {"auto",       "N", "boolean", "Turn off prompts"},
    {"stdout",     "N", "boolean", "Write standard output"},
    {"filter",     "N", "boolean", "Read standard input, "
	                           "write standard output"},
    /* after auto and stdout so it can replace */
    {"options",    "N", "boolean", "Prompt for required and optional values"},
    {"debug",      "N", "boolean", "Write debug output to program.dbg"},
    /* deprecated - to be removed in a future version */
    {"acdlog",     "N", "boolean", "Write ACD processing log "
                                   "to program.acdlog"},
    {"acdpretty",  "N", "boolean", "Rewrite ACD file as program.acdpretty"},
    {"acdtable",   "N", "boolean", "Write HTML table of options"},
    /* end of deprecated set */
    {"verbose",    "N", "boolean", "Report some/full command line options"},
    {"help",       "N", "boolean", "Report command line options. "
	                           "More information on associated and "
				   "general qualifiers "
				   "can be found with -help -verbose"},
    {"warning",    "Y", "boolean", "Report warnings"},
    {"error",      "Y", "boolean", "Report errors"},
    {"fatal",      "Y", "boolean", "Report fatal errors"},
    {"die",        "Y", "boolean", "Report deaths"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualAlign[] =
{
    {"aformat",    "",  "string",  "Alignment format"},
    {"aopenfile",  "",  "string",  "Alignment file name"},
    {"aextension", "",  "string",  "File name extension"},
    {"adirectory", "",  "string",  "Output directory"},
    {"aname",      "",  "string",  "Base file name"},
    {"awidth",     "0", "integer", "Alignment width"},
    {"aaccshow",   "N", "boolean", "Show accession number in the header"},
    {"adesshow",   "N", "boolean", "Show description in the header"},
    {"ausashow",   "N", "boolean", "Show the full USA in the alignment"},
    {"aglobal",    "N", "boolean", "Show the full sequence in alignment"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualFeat[] =
{
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {"fask",       "N", "boolean", "Prompt for begin/end/reverse"},
    {"fbegin",     "0", "integer", "First base used"},
    {"fend",       "0", "integer", "Last base used, def=max length"},
    {"freverse",   "N", "boolean", "Reverse (if DNA)"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualFeatout[] =
{
    {"offormat",   "",  "string",  "Output feature format"},
    {"ofopenfile", "",  "string",  "Features file name"},
    {"ofextension","",  "string",  "File name extension"},
    {"ofdirectory","",  "string",  "Output directory"},
    {"ofname",     "",  "string",  "Base file name"},
    {"ofsingle",   "N", "boolean", "Separate file for each entry"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualGraph[] =
{
    {"gprompt",   "N", "boolean", "Graph prompting"},
    {"gtitle",    "",  "string",  "Graph title"},
    {"gsubtitle", "",  "string",  "Graph subtitle"},
    {"gxtitle",   "",  "string",  "Graph x axis title"},
    {"gytitle",   "",  "string",  "Graph y axis title"},
    {"goutfile",  "",  "string",  "Output file for non interactive displays"},
    {"gdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualGraphxy[] =
{
    {"gprompt",   "N", "boolean", "Graph prompting"},
    {"gtitle",    "",  "string",  "Graph title"},
    {"gsubtitle", "",  "string",  "Graph subtitle"},
    {"gxtitle",   "",  "string",  "Graph x axis title"},
    {"gytitle",   "",  "string",  "Graph y axis title"},
    {"goutfile",  "",  "string",  "Output file for non interactive displays"},
    {"gdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualOutfile[] =
{
    {"odirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualReport[] =
{
    {"rformat",    "",  "string",  "Report format"},
    {"rname",      "",  "string",  "Base file name"},
    {"rextension", "",  "string",  "File name extension"},
    {"rdirectory", "",  "string",  "Output directory"},
    {"raccshow",   "N", "boolean", "Show accession number in the report"},
    {"rdesshow",   "N", "boolean", "Show description in the report"},
    {"rscoreshow", "Y", "boolean", "Show the score in the report"},
    {"rusashow",   "N", "boolean", "Show the full USA in the report"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualSeq[] =
{
    {"sbegin",     "0", "integer", "First base used"},
    {"send",       "0", "integer", "Last base used, def=seq length"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"sformat",    "", "string",   "Input sequence format"},
    {"sopenfile",  "", "string",   "Input filename"},
    {"sdbname",    "", "string",   "Database name"},
    {"sid",        "", "string",   "Entryname"},
    {"ufo",        "", "string",   "UFO features"},
    {"fformat",    "", "string",   "Features format"},
    {"fopenfile",  "", "string",   "Features file name"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualSeqset[] =
{
    {"sbegin",     "0", "integer", "First base used"},
    {"send",       "0", "integer", "Last base used, def=seq length"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"sformat",    "",  "string",  "Input sequence format"},
    {"sopenfile",  "",  "string",  "Input filename"},
    {"sdbname",    "",  "string",  "Database name"},
    {"sid",        "",  "string",  "Entryname"},
    {"ufo",        "",  "string",  "UFO features"},
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualSeqall[] =
{
    {"sbegin",     "0", "integer", "First base used"},
    {"send",       "0", "integer", "Last base used, def=seq length"},
    {"sreverse",   "N", "boolean", "Reverse (if DNA)"},
    {"sask",       "N", "boolean", "Ask for begin/end/reverse"},
    {"snucleotide","N", "boolean", "Sequence is nucleotide"},
    {"sprotein",   "N", "boolean", "Sequence is protein"},
    {"slower",     "N", "boolean", "Make lower case"},
    {"supper",     "N", "boolean", "Make upper case"},
    {"sformat",    "",  "string",  "Input sequence format"},
    {"sopenfile",  "",  "string",  "Input filename"},
    {"sdbname",    "",  "string",  "Database name"},
    {"sid",        "",  "string",  "Entryname"},
    {"ufo",        "",  "string",  "UFO features"},
    {"fformat",    "",  "string",  "Features format"},
    {"fopenfile",  "",  "string",  "Features file name"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualSeqout[] =
{
    {"osformat",   "",  "string",  "Output seq format"},
    {"osextension","",  "string",  "File name extension"},
    {"osname",     "",  "string",  "Base file name"},
    {"osdirectory","",  "string",  "Output directory"},
    {"osdbname",   "",  "string",  "Database name to add"},
    {"ossingle",   "N", "boolean", "Separate file for each entry"},
    {"oufo",       "",  "string",  "UFO features"},
    {"offormat",   "",  "string",  "Features format"},
    {"ofname",     "",  "string",  "Features file name"},
    {"ofdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualSeqoutset[] =
{
    {"osformat",   "",  "string",  "Output seq format"},
    {"osextension","",  "string",  "File name extension"},
    {"osname",     "",  "string",  "Base file name"},
    {"osdirectory","",  "string",  "Output directory"},
    {"osdbname",   "",  "string",  "Database name to add"},
    {"ossingle",   "N", "boolean", "Separate file for each entry"},
    {"oufo",       "",  "string",  "UFO features"},
    {"offormat",   "",  "string",  "Features format"},
    {"ofname",     "",  "string",  "Features file name"},
    {"ofdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

AcdOQual acdQualSeqoutall[] =
{
    {"osformat",   "",  "string",  "Output seq format"},
    {"osextension","",  "string",  "File name extension"},
    {"osname",     "",  "string",  "Base file name"},
    {"osdirectory","",  "string",  "Output directory"},
    {"osdbname",   "",  "string",  "Database name to add"},
    {"ossingle",   "N", "boolean", "Separate file for each entry"},
    {"oufo",       "",  "string",  "UFO features"},
    {"offormat",   "",  "string",  "Features format"},
    {"ofname",     "",  "string",  "Features file name"},
    {"ofdirectory","",  "string",  "Output directory"},
    {NULL, NULL, NULL, NULL}
};

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
    {"align",              "output",           acdSecOutput,
	 acdAttrAlign,     acdSetAlign,        acdQualAlign,
	 AJTRUE,  "Alignment output file" },
    {"array",              "simple",           NULL,
	 acdAttrArray,     acdSetArray,        NULL,
	 AJFALSE, "List of floating point numbers" },
    {"boolean",            "simple",           NULL,
	 acdAttrBool,      acdSetBool,         NULL,
	 AJFALSE, "Boolean value Yes/No" },
    {"codon",	           "input",            acdSecInput,
	 acdAttrCodon,     acdSetCodon,        NULL,
	 AJTRUE,  "Codon usage file in EMBOSS data path" },
    {"cpdb",	           "input",            acdSecInput,
	 acdAttrCpdb,      acdSetCpdb,         NULL,
	 AJFALSE, "Cleaned PDB file in EMBOSS data path" },
    {"datafile",           "input",            NULL,
	 acdAttrDatafile,  acdSetDatafile,     NULL,
	 AJFALSE, "Data file" },
    {"directory",          "input",            acdSecInput,
	 acdAttrDirectory, acdSetDirectory,    NULL,
	 AJFALSE, "Directory" },
    {"dirlist",	           "input",            acdSecInput,
	 acdAttrDirlist,   acdSetDirlist,      NULL,
	 AJFALSE, "Directory with files" },
    {"features",           "input",            acdSecInput,
	 acdAttrFeat,      acdSetFeat,         acdQualFeat,
	 AJTRUE,  "Readable feature table" },
    {"featout",            "output",           acdSecOutput,
	 acdAttrFeatout,   acdSetFeatout,      acdQualFeatout,
	 AJTRUE,  "Writeable feature table" },
    {"filelist",	   "input",            NULL,
	 acdAttrFilelist,  acdSetFilelist,     NULL,
	 AJFALSE, "Comma-separated file list" },
    {"float",              "simple",           NULL,
	 acdAttrFloat,     acdSetFloat,        NULL,
	 AJFALSE, "Floating point number" },
    {"graph",              "graph",            acdSecOutput,
	 acdAttrGraph,     acdSetGraph,        acdQualGraph,
	 AJTRUE,  "Graph device for a general graph" },
    {"infile",      "      input",             acdSecInput,
	 acdAttrInfile,    acdSetInfile,       NULL,
	 AJFALSE, "Input file" },
    {"integer",            "simple",           NULL,
	 acdAttrInt,       acdSetInt,          NULL,
	 AJFALSE, "Integer" },
    {"list",               "selection",        NULL,
	 acdAttrList,      acdSetList,         NULL,
	 AJFALSE, "Choose from menu list of values" },
    {"matrix",             "input",            acdSecInput,
	 acdAttrMatrix,    acdSetMatrix,       NULL,
	 AJFALSE, "Comparison matrix file in EMBOSS data path" },
    {"matrixf",            "input",            acdSecInput,
	 acdAttrMatrixf,   acdSetMatrixf,      NULL,
	 AJFALSE, "Comparison matrix file in EMBOSS data path" },
    {"outfile",            "output",           acdSecOutput,
	 acdAttrOutfile,   acdSetOutfile,   acdQualOutfile,
	 AJTRUE,  "Output file" },
    {"range",	           "simple",           NULL,
	 acdAttrRange,     acdSetRange,        NULL,
	 AJFALSE, "Sequence range" },
    {"regexp",	           "input",            acdSecInput,
	 acdAttrRegexp,    acdSetRegexp,       NULL,
	 AJFALSE, "Regular expression pattern" },
    {"report",             "output",           acdSecOutput,
	 acdAttrReport,    acdSetReport,       acdQualReport,
	 AJTRUE,  "Report output file" },
    {"scop",	           "input",            acdSecInput,
	 acdAttrScop,      acdSetScop,         NULL,
	 AJFALSE, "Scop entry in EMBOSS data path" },
    {"selection",          "selection",        NULL,
	 acdAttrSelect,    acdSetSelect,       NULL,
	 AJFALSE, "Choose from selection list of values" },
    {"sequence",           "input",            acdSecInput,
	 acdAttrSeq,       acdSetSeq,          acdQualSeq,
	 AJTRUE,  "Readable sequence" },
    {"seqall",             "input",            acdSecInput,
	 acdAttrSeqall,    acdSetSeqall,       acdQualSeqall,
	 AJTRUE,  "Readable sequence(s)" },
    {"seqout",             "output",           acdSecOutput,
	 acdAttrSeqout,    acdSetSeqout,       acdQualSeqout,
	 AJTRUE,  "Writeable sequence" },
    {"seqoutall",          "output",           acdSecOutput,
	 acdAttrSeqoutall, acdSetSeqoutall,    acdQualSeqoutall,
	 AJTRUE,  "Writeable sequence(s)" },
    {"seqoutset",          "output",           acdSecOutput,
	 acdAttrSeqoutset, acdSetSeqoutset,    acdQualSeqoutset,
	 AJTRUE,  "Writeable sequences" },
    {"seqset",             "input",            acdSecInput,
	 acdAttrSeqset,    acdSetSeqset,       acdQualSeqset,
	 AJTRUE,  "Readable sequences" },
    {"string",             "simple",           NULL,
	 acdAttrString,    acdSetString,       NULL,
	 AJFALSE, "String value" },
    {"xygraph",            "graph",            acdSecOutput,
	 acdAttrGraphxy,   acdSetGraphxy,      acdQualGraphxy,
	 AJTRUE,  "Graph device for a 2D graph" },
    {NULL, NULL, NULL, NULL, NULL, NULL, AJFALSE, NULL}
};

/* @datastatic AcdPValid ******************************************************
**
** ACD valid input structure. For each ACD type, stores functions
** that reports on valid input and expected values
**
** @alias AcdSValid
** @alias AcdOValid
**
** @attr Name [char*] ACD type name
** @attr Valid [(void*)] Function to report valid values in general
** @attr Expect [(void*)] Function to report expected values
**                        for this definition
** @@
******************************************************************************/

typedef struct AcdSValid
{
    char* Name;
    void (*Valid) (AcdPAcd thys, AjPStr* str);
    void (*Expect) (AcdPAcd thys, AjPStr* str);
} AcdOValid, *AcdPValid;

/* @funclist acdValue *********************************************************
**
** ACD type help processing, includes functions to describe valid
** values and expected values in -help output and -acdtable output
**
******************************************************************************/

AcdOValid acdValue[] =
{
    {"sequence",  acdHelpValidSeq,      acdHelpExpectSeq},
    {"seqset",    acdHelpValidSeq,      acdHelpExpectSeq},
    {"seqall",    acdHelpValidSeq,      acdHelpExpectSeq},
    {"seqout",    acdHelpValidSeqout,   acdHelpExpectSeqout},
    {"seqoutset", acdHelpValidSeqout,   acdHelpExpectSeqout},
    {"seqoutall", acdHelpValidSeqout,   acdHelpExpectSeqout},
    {"outfile",   acdHelpValidOut,      acdHelpExpectOut},
    {"infile",    acdHelpValidIn,       acdHelpExpectIn},
    {"datafile",  acdHelpValidData,     acdHelpExpectData},
    {"codon",     acdHelpValidCodon,    acdHelpExpectCodon},
    {"dirlist",   acdHelpValidDirlist,  acdHelpExpectDirlist},
    {"filelist",  acdHelpValidFilelist, acdHelpExpectFilelist},
    {"list",      acdHelpValidList,     NULL},
    {"cpdb",      acdHelpValidCpdb,     acdHelpExpectCpdb},
    {"scop",      acdHelpValidScop,     acdHelpExpectScop},
    {"select",    acdHelpValidSelect,   NULL},
    {"graph",     acdHelpValidGraph,    acdHelpExpectGraph},
    {"xygraph",   acdHelpValidGraph,    acdHelpExpectGraph},
    {"regexp",    acdHelpValidRegexp,   acdHelpExpectRegexp},
    {"string",    acdHelpValidString,   acdHelpExpectString},
    {"integer",   acdHelpValidInt,      acdHelpExpectInt},
    {"float",     acdHelpValidFloat,    acdHelpExpectFloat},
    {"matrix",    acdHelpValidMatrix,   acdHelpExpectMatrix},
    {"matrixf",   acdHelpValidMatrix,   acdHelpExpectMatrix},
    {"range",     acdHelpValidRange,    acdHelpExpectRange},
    {"featout",   acdHelpValidFeatout,  acdHelpExpectFeatout},
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
    static AjPStr acdRoot = NULL;
    static AjPStr acdRootInst = NULL;
    static AjPStr acdPack = NULL;
    static AjPStr tmpstr = NULL;
    AjPStrTok tokenhandle = NULL;
    char white[] = " \t\n\r";
    AjPList acdListWords = NULL;
    AjPList acdListCount = NULL;
    AjPStr tmpword = NULL;	/* words to add to acdListWords */
    ajint i;
    ajint *k = NULL;
    
    acdProgram = ajStrNewC (pgm);
    acdSecList = ajListstrNew();
    
    acdLog ("testing acdprompts");
    if (ajNamGetValueC ("acdprompts", &tmpstr))
    {
	acdLog ("acdprompts '%S'", tmpstr);
	if (ajStrToInt(tmpstr, &i))
	    acdPromptTry = i;
	if (acdPromptTry < 1) acdPromptTry = 1;
	acdLog ("acdPromptTry %d", acdPromptTry);
    }
    ajStrDel(&tmpstr);
    
    /* pre-parse the command line for special options */
    
    acdArgsScan (argc, argv);
    
    /* open the command definition file */
    
    (void) ajNamRootPack (&acdPack);
    (void) ajNamRootInstall (&acdRootInst);
    (void) ajFileDirFix (&acdRootInst);
    
    if (ajNamGetValueC ("acdroot", &acdRoot))
    {				       /* _acdroot variable defined */
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
	    acdLog ("acdfile '%S' not opened\n", acdFName);
	    ajStrAssC (&acdPack, package); /* package name for acdInitP */
	    ajStrToLower (&acdPack);

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
	    acdLog ("acdfile '%S' not opened\n", acdFName);
	    (void) ajNamRoot (&acdRoot);
	    (void) ajFileDirFix (&acdRoot);
	    ajFmtPrintS (&acdFName, "%Sacd/%s.acd", acdRoot, pgm);
	    acdFile = ajFileNewIn (acdFName);
	}
    }
    
    
    if (!acdFile)		/* test by nofile.acd */
	acdError ("ACD file not opened\n");
    
    /* read the whole file into a string [change to use a list later] */
    
    acdListWords = ajListstrNew();
    acdListCount = ajListNew();
    
    while (ajFileReadLine (acdFile, &acdLine))
    {
	AJNEW0(k);
	*k = ajListLength(acdListWords);
	ajListPushApp(acdListCount, k);
      
	if (ajStrUncomment(&acdLine))
	{
	    tokenhandle = ajStrTokenInit (acdLine, white);
	  
	    while (ajStrToken (&tmpword, &tokenhandle, NULL))
	    {
		if (ajStrLen(tmpword)) /* nothing before first whitespace */
		{
		    acdLog ("++ add to list %x '%S'\n", tmpword, tmpword);
		    ajListstrPushApp (acdListWords, tmpword);
		    tmpword = NULL;
		}
		else
		{
		    ajStrDel (&tmpword);
		}
	    }
	    (void) ajStrTokenClear (&tokenhandle);
	    ajStrDel (&tmpword); 	/* empty token at the end */
	}
    }
    ajFileClose (&acdFile);
    ajStrDel (&acdLine);
    
    AJNEW0(k);
    *k = ajListLength(acdListWords);
    ajListPushApp(acdListCount, k);
    
    /* Parse the input to set the initial definitions */
    
    acdParse (acdListWords, acdListCount);
    
    ajListstrDel (&acdListWords);
    ajListDel (&acdListCount);
    
    if (acdDoPretty || acdDoValid)
	ajExit ();
    
    /* Fill in incomplete information like parameter numbers */
    
    acdProcess ();
    
    AJCNEW0(acdParamSet, acdNParam+1);
    
    /* report on what we have so far */
    
    acdListReport("Definitions in ACD file");
    
    /* parse the command line and update the values */
    
    acdArgsParse (argc, argv);
    
    /* report on what we have so far */
    
    acdListReport("Results of parsing command line arguments");
    
    /* set the true values and prompt for missing required values */
    
    acdSetAll();
    
    /* report on what we have now */
    
    acdListReport("Final results after setting values and prompting the user");
    
    /* all done */
    
    ajStrDel(&acdRoot);
    ajStrDel(&acdRootInst);
    ajStrDel(&acdPack);
    ajStrDel(&acdFName);
    
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

AjStatus ajAcdInit (char *pgm, ajint argc, char *argv[])
{
    return ajAcdInitP (pgm, argc, argv, "");
}

/*===========================================================================*/
/*========================= ACD File Parsing ================================*/
/*===========================================================================*/

/* @funcstatic acdStage *******************************************************
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
    
    i = QUAL_STAGE + 1;
    while (acdKeywords[i].Name)	/* ACD keywords */
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
    while (acdType[i].Name)	/* ACD types as qualifiers */
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
    {					/* test ambigtype.acd */
	acdError ("ambiguous acd type %S (%S)", token, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    
    return BAD_STAGE;
}

/* @funcstatic acdParse *******************************************************
**
** Parse the command line definition and build data structures from it.
**
** @param [r] listwords [AjPList] List of words (as strings) from ACD file
** @param [r] listcount [AjPList] List of word count before each line
** @return [void]
** @see acdNewAppl
** @see acdNewQual
** @see acdNewVar
** @@
******************************************************************************/

static void acdParse (AjPList listwords, AjPList listcount)
{
    AjPStr acdStrType = NULL;
    AjPStr acdStrAlias = NULL;
    AjPStr acdStrValue = NULL;
    AjPStr secname = NULL;
    ajint linecount = 0;
    ajint lineword = 0;
    ajint *iword = NULL;
    acdLineNum = 0;
    
    while (ajListLength (listcount) && (!lineword))
    {
	ajListPeek (listcount, (void**) &iword);
	if (*iword)
	    lineword = *iword;
	else
	{
	    ajListPop (listcount, (void**) &iword);
	    linecount++;
	    acdLineNum = linecount - 1;
	    AJFREE (iword);
	}
    }
    lineword = 0;
    acdWordNum = 0;
    
    while (ajListLength(listwords))
    {
 	acdWordNextName (listwords, &acdStrType);
	
	while (ajListLength (listcount) && (lineword < acdWordNum))
	{
	    ajListPop (listcount, (void**) &iword);
	    lineword = *iword;
	    linecount++;
	    acdLineNum = linecount - 1;
	    AJFREE (iword);
	}
	
	acdCurrentStage = acdStage (acdStrType);
	if (acdWordNum == 1)
        {
	    if (acdCurrentStage != APPL_STAGE) /* test noappl.acd */
	    {
		acdError ("Application definition required at start");
	    }
	}
	
	switch (acdCurrentStage)
	{
	    
	case APPL_STAGE:
	    if (acdWordNum != 1)
	    {
		acdError ("Application definition allowed only at start");
	    }

	    /* application: then the appl name */
	    acdParseName (listwords, &acdStrName);
	    
	    acdNewCurr = acdNewAppl (acdStrName);
	    
	    acdPretty ("%s: %S [\n",
		       acdKeywords[acdCurrentStage].Name, acdStrName);
	    
	    acdParseAttributes (acdNewCurr, listwords);

	    acdValidAppl (acdNewCurr);
	    
	    /* automatic $(today) variable */
	    (void) ajStrAssC (&acdStrName, "today");
	    (void) ajFmtPrintS (&acdStrValue, "%D", ajTimeToday());
	    acdNewCurr = acdNewVar (acdStrName);
	    (void) acdSetVarDef (acdNewCurr, acdStrValue);
	    break;
	    
	    /* type: qualname alias[ attr: value ]
	     **
	     ** The alias name is optional (defaults to the qualifier name)
	     ** The [] are required so the token can be detected.  Attributes
	     ** are defined for each "type", as are associated
	     ** qualifiers. There is no distinction between them here.  The
	     ** difference is that the qualifier values are defaults which
	     ** can be overridden on the command line
	     */
	    
	case QUAL_STAGE:
	    acdParseAlpha (listwords, &acdStrName);
	    
	    if (acdNotLeftB(listwords))
	    {				/* test badalias.acd */
		if (!acdWordNextLower (listwords, &acdStrAlias) ||
		    !ajStrIsAlpha(acdStrAlias))
		    acdError ("Bad syntax qualifier alias name '%S'",
			      acdStrAlias);
	    }
	    else	/* we have an alternate name before the '[' */
	    {
		(void) ajStrAssS (&acdStrAlias, acdStrName);
	    }
	    
	    acdNewCurr = acdNewQual (acdStrName, acdStrAlias, &acdStrType, 0);
	    
	    if (!ajStrMatch(acdStrName, acdStrAlias))
		acdPretty ("\n%S: %S %S [\n", acdStrType,
		       acdStrName, acdStrAlias);
	    else
		acdPretty ("\n%S: %S [\n", acdStrType,
			   acdStrName);
	    
	    acdParseAttributes (acdNewCurr, listwords);
	    
	    acdValidQual (acdNewCurr);

	    break;
	    
	case SEC_STAGE:
	    /* section: name [ attrlist ] */
	    acdParseName (listwords, &acdStrName);
	    
	    acdNewCurr = acdNewSec (acdStrName);
	    
	    acdPretty ("\n%s: %S [\n",
		       acdKeywords[acdCurrentStage].Name, acdStrName);
	    
	    acdParseAttributes (acdNewCurr, listwords);
	    
	    acdValidSection (acdNewCurr);

	    acdPrettyShift ();
	    
	    break;
	    
	case ENDSEC_STAGE:
	    /* endsection: name */
	    /* remove from list of current sections */
	    acdParseName (listwords, &acdStrName);
	    acdNewCurr = acdNewEndsec (acdStrName);
	    acdValidSection (acdNewCurr);
	    acdPrettyUnShift ();
	    acdPretty ("\n%s: %S\n",
		       acdKeywords[acdCurrentStage].Name, acdStrName);
	    break;
	    
	    /* catch-all for failed parsing */
	    
	case VAR_STAGE:
	    /* then the variable name and the value */
	    acdParseName (listwords, &acdStrName);
	    ajStrAssS(&acdStrValue, acdParseValue(listwords));
	    acdNewCurr = acdNewVar (acdStrName);
	    (void) acdSetVarDef (acdNewCurr, acdStrValue);
	    acdPretty ("\n%s: %S \"%S\"\n",
		       acdKeywords[acdCurrentStage].Name,
		       acdStrName, acdStrValue);
	    break;
	    
	case BAD_STAGE:		/* test badstage.acd */
	default:		/* Fatal - should never happen */
	    acdError ("Unrecognized token '%S'\n", acdStrType);
	    break;
	}
    }
    acdLog ("-- All Done --\n");
    
    acdLog("-- All Done : acdSecList length %d\n",
	   ajListstrLength(acdSecList));
    
    acdLineNum = linecount;
    
    if (ajListstrLength(acdSecList)) /* fatal error, unclosed section(s) */
    {
	while (ajListstrPop(acdSecList, &secname))
	{
	    ajDebug("Section '%S' has no endsection\n", secname);
	    ajErr("Section '%S' has no endsection", secname); /* fails below */
	    ajStrDel(&secname);
	}
	acdLog("Unclosed sections in ACD file\n");
	acdError ("Unclosed sections in ACD file"); /* test noendsec.acd */
    }
    
    (void) ajStrDel (&acdStrName); /* the global string ... no longer needed */
    
    (void) ajStrDel (&acdStrAlias);
    (void) ajStrDel (&acdStrType);
    (void) ajStrDel (&acdStrValue);
    (void) ajListstrDel (&acdSecList);
    
    acdLineNum = 0;
    
    while (ajListLength (listcount))
    {
	ajListPop (listcount, (void**) &iword);
	AJFREE (iword);
    }
    return;
}


/* @funcstatic acdParseValue **************************************************
**
** Uses ajStrTok to complete a (possibly) quoted value.
** Note that ajStrTok has a stored internal copy of the text string
** which is set up at the start of acdParse and is being used here.
**
** Quotes can be single or double.
**
** The early versions also allowed any kind of parentheses,
** depending on the first character of the next token examined.
** This is now obsolete, to simplify the syntax and to allow
** future reuse of parentheses.
**
** @param [u] listwords [AjPList] List of strings for each word to be parsed
** @return [AjPStr] String containing next value from list
** @@
******************************************************************************/


static AjPStr acdParseValue (AjPList listwords)
{
    AjPStr strp=NULL;
    static AjPStr tmpstrp=NULL;
    char  endq[]=" ";
    ajint iquote;
    char *cq;
    AjBool done = ajFalse;
    
    char *quotes = "\"'";
    char *endquotes = "\"'";
    
    if (!acdWordNext (listwords, &strp)) /* test: novalue.acd */
	acdErrorAcd (acdNewCurr,
		     "Unexpected end of file, attribute value not found\n");
    
    cq = strchr(quotes, ajStrChar(strp, 0));
    if (!cq)				/* no quotes, simple return */
    {
	ajStrAssS(&tmpstrp, strp);
	ajStrDel(&strp);
	return tmpstrp;
    }
    
    /* quote found: parse up to closing quote then strip white space */
    
    iquote = cq - quotes;
    endq[0] = endquotes[iquote];
    
    (void) ajStrTrim (&strp, 1);
    (void) ajStrDelReuse (&tmpstrp);
    
    while(!done)
    {
	if (ajStrSuffixC(strp, endq))
	{			       /* check for trailing quotes */
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
	{
	    if (!acdWordNext (listwords, &strp)) /* test noquote.acd */
		acdErrorAcd (acdNewCurr,
			     "Unexpected end of file, no closing quote\n");
	}
    }
    
    ajStrDel (&strp);
    return tmpstrp;
}

/* @funcstatic acdWordNext ****************************************************
**
** Returns the next word from a list
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @param [r] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdWordNext (AjPList listwords, AjPStr* pword)
{
    ajStrDel (pword);
    if (ajListstrPop(listwords, pword))
    {
	acdWordNum++;
	return ajTrue;
    }

    ajStrAssC (pword, "");
    return ajFalse;
}

/* @funcstatic acdWordNextLower ***********************************************
**
** Returns the next word from a list, in lower case.
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @param [r] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdWordNextLower (AjPList listwords, AjPStr* pword)
{
    if (acdWordNext (listwords, pword))
    {
	if (!ajStrIsLower(*pword))
	{
	    acdWarn ("Automatically converting '%S' to lower case", *pword);
	    ajStrToLower(pword);
	}
	return ajTrue;
    }

    return ajFalse;
}

/* @funcstatic acdWordNextName ***********************************************
**
** Returns the next word from a list, in lower case
**
** This must be an ACD name (type or attribute) alphabetic only,
** with a trailing ':'
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @param [r] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdWordNextName (AjPList listwords, AjPStr* pword)
{
    if (acdWordNext(listwords, pword))
    {
	if (ajStrChar(*pword, -1) != ':')
	{				/* test nocolon.acd */
	    acdError ("Expected ':' not found after '%S'", *pword);
	    return ajFalse;
	}
	ajStrTrim (pword, -1);
	if (!ajStrIsAlpha(*pword))
	    return ajFalse;
	ajStrToLower(pword);
	return ajTrue;
    }

    return ajFalse;
}

/* @funcstatic acdParseName ***********************************************
**
** Returns the next word from a list, in lower case.
**
** This must be an ACD name (alphanumeric allowed as this is not used
** for qualifier names)
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @param [r] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static void acdParseName (AjPList listwords, AjPStr* pword)
{
    if (acdWordNextLower (listwords, pword))
    {
	if (ajStrIsAlnum (*pword))
	    return;
    }

    /* test noapplname.acd badapplname.acd */
    acdError ("Bad or missing %s name '%S'",
	      acdKeywords[acdCurrentStage].Name, *pword);

    return;
}

/* @funcstatic acdParseAlpha **************************************************
**
** Returns the next word from a list, in lower case.
**
** This must be an alphabetic word, no numbers or underscores allowed.
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @param [r] pword [AjPStr*] Next word from the list
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static void acdParseAlpha (AjPList listwords, AjPStr* pword)
{
    if (acdWordNextLower (listwords, pword))
    {
	if (ajStrIsAlpha(*pword))
	    return;
    }

    /* test noqualname.acd badqualname.acd */
    acdError ("Bad or missing %s alphabetic name '%S'",
	      acdKeywords[acdCurrentStage].Name, *pword);

    return;
}

/* @funcstatic acdNotLeftB ****************************************************
**
** Tests the start of the next word in the list for '[' at the start
**
** Does not remove the '['
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @return [AjBool] ajTrue if start of string does not match '['
** @@
******************************************************************************/

static AjBool acdNotLeftB (AjPList listwords)
{
    char ch;
    AjPStr pstr = NULL;
 
    if (!ajListstrPeek (listwords, &pstr))
	return ajFalse;

    ch = ajStrChar(pstr, 0);

    if (ch == '[')
    {
	return ajFalse;
    }
    /* do not delete pstr - we only peeked */

    return ajTrue;
}

/* @funcstatic acdIsLeftB *****************************************************
**
** Tests the start of the next word in the list for '[' at the start
**
** @param [r] listwords [AjPList] List of words parsed from ACD file
** @return [AjBool] ajTrue if start of string matches '['
** @@
******************************************************************************/

static AjBool acdIsLeftB (AjPList listwords)
{
    char ch;
    AjPStr teststr = NULL;
    AjPStr pstr = NULL;
 
    if (!ajListstrPeek (listwords, &teststr))
	return ajFalse;

    ch = ajStrChar(teststr, 0);

    if (ch == '[')
    {
	(void) ajStrTrim (&teststr, 1);	/* trim the leading '[' in the list */
	if (!ajStrLen(teststr))
	{		    /* only the '[' so remove from the list */
	    (void) acdWordNext (listwords, &pstr); /*  must succeed - Peeked */
	    ajStrDel (&pstr);	     /* empty - ignored - so delete */
	    teststr = NULL;
	}
	return ajTrue;
    }
    /* do not delete teststr - we only peeked  - deleted as pstr */

    return ajFalse;
}

/* @funcstatic acdIsRightB ****************************************************
**
** Tests for ']' to look for ascent to a higher level of parsing.
**
** Tests the end of the current string
** If that fails, tests the start of the next word in the list.
**
** Afterwards, the value of pstr is the last word with any ']' removed
**
** @param [uP] pstr [AjPStr*] String which has a trailing ']' removed if found
** @param [r] listwords [AjPList] List of remaining words to be parsed
** @return [AjBool] ajTrue if end of string matches ']'
** @@
******************************************************************************/

static AjBool acdIsRightB (AjPStr* pstr, AjPList listwords)
{
    AjPStr teststr = NULL;
    AjPStr tmpstr = NULL;
    char ch;

    if (*pstr)
    {
	ch = ajStrChar(*pstr, -1);

	if (ch == ']')	    /* test input pstr value for ']' at end */
	{
	    (void) ajStrTrim (pstr, -1);
	    return ajTrue;
	}
    }

    /* go on to the next word in the list */

    if (!ajListstrPeek (listwords, &teststr)) /* leftend.acd valend.acd */
	acdErrorAcd (acdNewCurr, "End of file looking for ']'");

    ch = ajStrChar(teststr, 0);

    if (ch == ']')		       /* next word starts with ']' */
    {
	(void) ajStrTrim (&teststr, 1);	/* trim the word - in the list */
	if (!ajStrLen(teststr))		/*  only "]" so delete it */
	{
	    acdWordNext (listwords, &tmpstr); /* works - used ajListstrPeek */
	    ajStrDel (&tmpstr);
	    teststr = NULL;
	}
	return ajTrue;
    }

    return ajFalse;
}

/* @funcstatic acdParseAttributes *********************************************
**
** Parse the attribute list for an ACD type
**
** @param [r] acd [AcdPAcd] Acd object
** @param [r] listwords [AjPList] List of parsed words
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static void acdParseAttributes (AcdPAcd acd, AjPList listwords)
{
    AjPStr strAttr = NULL;
    AjPStr strValue = NULL;
    AjPStr strFixValue = NULL;
    AjBool done=ajFalse;

    if (!acdIsLeftB(listwords))	/* test noleftappl.acd noleftsec.acd */
	/* noleftq.acd */
	acdErrorAcd (acdNewCurr, "Failed to find '[' for %s %S\n",
		     acdKeywords[acdCurrentStage].Name, acdStrName);

    acdPrettyShift ();

    done = acdIsRightB(&strAttr, listwords); /* could be [ ] */

    /* continue parsing until we reach a true closing ']' character */

    while (!done)
    {
	if (!acdWordNextName (listwords, &strAttr)) /* test: noattname.acd */
	    acdErrorAcd (acdNewCurr, "Bad or missing attribute name '%S'",
			 strAttr);

	(void) ajStrAssS (&strValue, acdParseValue (listwords));
	done = acdIsRightB(&strValue, listwords); /* will this be last pair? */

	(void) ajStrAssS (&strFixValue, strValue);
	(void) acdTextFormat(&strFixValue);
	if (acdCurrentStage == QUAL_STAGE)
	    (void) acdSet (acd, &strAttr, strFixValue);
	else
	    (void) acdSetKey (acd, &strAttr, strFixValue);
	acdPrettyWrap (ajStrLen(strAttr)+3, "%S: \"%S\"",
		       strAttr, strValue);
    }

    acdPrettyUnShift ();
    acdPretty ("]\n");

    ajStrDel (&strAttr);
    ajStrDel (&strValue);
    ajStrDel (&strFixValue);

    return;
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
		saveqacd = qacd; /* save the location of the first one */
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

/* @funcstatic acdNewVar ******************************************************
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

/* @funcstatic acdNewSec ******************************************************
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
    AjPStr secname=NULL;

    if (firstcall)
    {
	ikey = acdFindKeyC ("section");
	firstcall = 0;
    }

    acdLog("acdNewSec '%S' acdSecList length %d\n",
	   name, ajListstrLength(acdSecList));

    acd = acdNewAcdKey(name, name, ikey);
    acd->Level = ACD_SEC;

    ajStrAssS(&secname, name);
    ajListstrPush (acdSecList, secname);

    acdLog("acdNewSec acdSecList push '%S' new length %d\n",
	   secname, ajListstrLength(acdSecList));

    return acd;
}

/* @funcstatic acdNewEndsec ***************************************************
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
    AjPStr secname=NULL;

    if (firstcall)
    {
	ikey = acdFindKeyC ("endsection");
	firstcall = 0;
    }

    acdLog("acdNewEndsec '%S' acdSecList length %d\n",
	   name, ajListstrLength(acdSecList));

    if(!ajListstrLength (acdSecList))	/* test endsecextra.acd */
    {
	acdLog ("Bad endsection '%S', not in a section\n", name);
	acdError ("Bad endsection '%S', not in a section", name);
    }

    else
    {
	ajListstrPop (acdSecList, &secname);
	acdLog("Pop from acdSecList '%S' new length %d\n",
	       secname, ajListstrLength(acdSecList));

	if (!ajStrMatch(name, secname)) /* test badendsec.acd */
	{
	    acdLog ("Bad endsection '%S', current section is '%S\n'",
		    name, secname);
	    acdError ("Bad endsection '%S', current section is '%S'",
		      name, secname);
	}
	ajStrDel(&secname);
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
    
    itype = acdFindType (*type);
    (void) ajStrAssC (type, acdType[itype].Name);
    
    /* do any associated qualifiers first so they are already complete
       when we come to the parameter later in processing */
    
    i = 0;
    quals = acdType[itype].Quals; /* any associated qualifiers for itype?  */
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
		saveqacd = qacd; /* save the location of the first one */
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

    acdTestAssocUnknown (name);
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
    else
    {
	acdListLast = AJNEW0(acdList);
    }
    acdListLast->LineNum = acdLineNum;
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
    acdListLast->ValStr = NULL;

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
    else
    {
	acdListLast = AJNEW0(acdList);
    }
    acdListLast->Next = NULL;
    acdListLast->LineNum = acdLineNum;
    
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
    acdListLast->ValStr = NULL;
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
** @param [r] alias [AjPStr] Qualifier name to be used on command line
** @param [r] pnum [ajint] Parameter number (zero for general qualifiers)
** @return [void]
** @@
******************************************************************************/

static void acdTestUnknown (AjPStr name, AjPStr alias, ajint pnum)
{
    AcdPAcd pa;
    
    pa = acdFindAcd (name, alias, pnum);
    if (pa)
    {
	if (ajStrMatch(name, alias))
	{
	    if (pnum)			/* never used? */
		acdErrorAcd (pa, "Name '%S%d' not unique\n",
			     name, pnum);
	    else			/* test: dupname.acd */
		acdErrorAcd (pa, "Name '%S' not unique\n",
			     name);
	}
	else
	{
	    if (pnum)			/* never used? */
		acdErrorAcd (pa,
			     "Name/Alias '%S%d'/'%S%d' not unique\n",
			     name, pnum, alias, pnum);
	    else			/* test: dupalias.acd */
		acdErrorAcd (pa,
			     "Name/Alias '%S'/'%S' not unique\n",
			     name, alias);
	}
    }

    return;
}

/* @funcstatic acdTestAssocUnknown ********************************************
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

static void acdTestAssocUnknown (AjPStr name)
{
    AcdPAcd pa;

    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	if (!pa->Assoc && (ajStrMatch(pa->Name, name) ||
			   ajStrMatch(pa->Token, name)))
	{
	    if (ajStrMatch(pa->Name, pa->Token)) /* test: dupassoc.acd */
		acdErrorAcd (pa,
			     "Associated qualifier '%S' clashes with '%S' in ACD file\n",
			     name, pa->Name);
	    else
		acdErrorAcd (pa,	/* test: dupassoc2.acd */
			     "Associated qualifier '%S' clashes with '%S'/'%S' in ACD file\n",
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
        if (acdIsStype(pa)) continue;
	if (ajStrMatch(pa->Name, name) || ajStrMatch(pa->Token, token))
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
** Used in defining ACD objects and in processing the commandline.
**
** @param [r] thys [AcdPAcd] ACD object for the parameter
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] altname [AjPStr] Alternative token name (e.g. qualifier
**            with "no" prefix removed)
** @return [AcdPAcd] ACD object for the selected qualifier
** @@
******************************************************************************/

static AcdPAcd acdFindAssoc (AcdPAcd thys, AjPStr name, AjPStr noname)
{
    AcdPAcd pa;
    ajint ifound=0;
    AcdPAcd ret=NULL;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
    {
	if (ajStrPrefix(pa->Name, name) ||
	    ajStrPrefix(pa->Name, noname))
	{
	    if (ajStrMatch(pa->Name, name) ||
		ajStrMatch(pa->Name, noname))
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
	ajWarn ("Ambiguous name/token '%S' (%S)", name, ambigList);
	(void) ajStrDelReuse(&ambigList);
	acdErrorAcd (thys,
		     "Attribute or qualifier '%S' ambiguous\n",
		     name);
    }

    return NULL;
}

/* @funcstatic acdTestAssoc ***************************************************
**
** Locates an ACD object for an associated qualifier by name.
**
** Only tests silently for a possible qualifier.
**
** If this fails, we check properly later.
**
** Used in defining ACD objects and in processing the commandline.
**
** @param [r] thys [AcdPAcd] ACD object for the parameter
** @param [r] name [AjPStr] Token name to be used by applications
** @param [r] altname [AjPStr] Alternative token name (e.g. qualifier
**            with "no" prefix removed)
** @return [AcdPAcd] ACD object for the selected qualifier
** @@
******************************************************************************/

static AcdPAcd acdTestAssoc (AcdPAcd thys, AjPStr name, AjPStr noname)
{
    AcdPAcd pa;
    ajint ifound=0;
    AcdPAcd ret=NULL;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
    {
	if (ajStrPrefix(pa->Name, name) ||
	    ajStrPrefix(pa->Name, noname))
	{
	    if (ajStrMatch(pa->Name, name) ||
		ajStrMatch(pa->Name, noname))
		return pa;
	    ifound++;
	    ret = pa;
	}
    }

    if (ifound == 1)
	return ret;

    return NULL;
}

/* @funcstatic acdTestQualC ***************************************************
**
** Tests whether "name" is a valid qualifier name.
** To be valid, it must begin with "-" or '/'.
** If not, it can be taken as a value for the previous qualifier
**
** Used after a boolean option to check for a possible value
**
** Should run silently - if not valid, we will test it next anyway
**
** @param [r] name [char*] Qualifier name
** @return [AjBool] ajTrue if
** @@
******************************************************************************/

static AjBool acdTestQualC (char *name)
{
    static AjPStr  qstr = NULL;
    static AjPStr  qnostr = NULL;
    static AjPStr  qmaster = NULL;
    AcdPAcd pa;
    AcdPAcd qa;
    AcdPAcd savepa=NULL;
    ajint qnum = 0;
    ajint i;
    ajint ifound=0;
    static AjPStr ambigList = NULL;
    
    (void) ajStrAssC(&ambigList, "");
    
    acdLog ("acdTestQualC '%s'\n", name);
    
    if (*name != '-' && *name != '/' && !strstr(name, "="))
	return ajFalse;		/* not a qualifier name */
    
    (void) ajStrAssC (&qstr, name+1); /* lose the - or / prefix */
    
    i = ajStrFindC(qstr, "=");		/* qualifier with value */
    if (i > 0)
	(void) ajStrSub(&qstr, 0, i-1);	/* strip any value and keep testing */
    
    if (ajStrPrefixC(qstr, "no")) /* check for -no qualifiers */
	ajStrAssSub(&qnostr, qstr, 2, -1);
    else
	ajStrAssC (&qnostr, "");
    
    acdQualParse (&qstr, &qnostr, &qmaster, &qnum);
    
    if (ajStrLen(qmaster))	/* master specified as -qstr_qmaster */
    {
	for (pa=acdList; pa; pa=pa->Next)
	{
	    if (acdIsStype(pa)) continue;
	    if (ajStrMatch (pa->Name, qmaster))
	    {
		acdLog ("  *master matched* '%S'\n", pa->Name);
		qa = acdTestAssoc (pa, qstr, qnostr);
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
	acdLog ("   ifound: %d\n", ifound);

	if (ifound == 1)
	{
	    qa = acdTestAssoc (savepa, qstr, qnostr);
	    if (qa)
		return ajTrue;
	    else
		return ajFalse;
	}

	if (ifound > 1)		/* master should be checked earlier */
	{
	    /* ajWarn ("Ambiguous associated qualifier '%s' (%S)",
	       name, ambigList);
	       (void) ajStrDelReuse(&ambigList); */
	    return ajFalse;
	}
    }
    else			/* just qualifier name -qstr */
    {
	for (pa=acdList; pa; pa=pa->Next)
	{
	    if (acdIsStype(pa)) continue;
	    if (ajStrMatch (pa->Name, qstr))
	    {
		acdLog ("   *matched* '%S'\n", pa->Name);
		return ajTrue;
	    }
	    if (ajStrPrefix (pa->Name, qstr))
	    {
		ifound++;
		acdAmbigApp (&ambigList, pa->Name);
	    }
	}

	acdLog ("   ifound: %d\n", ifound);

	if (ifound == 1)
	    return ajTrue;

	if (ifound > 1)
	{
	    /* ajWarn ("Ambiguous qualifier '%s' (%S)", name, ambigList);
	       (void) ajStrDelReuse(&ambigList); */
	    return ajFalse;
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
    {			      /* warn now with the list, fail below */
	ajWarn ("ambiguous type %S (%S)", type, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound != 1)	       /* Fatal: but covered by other tests */
	acdError ("unknown type: '%S'\n", type);

    return j;
}

/* @funcstatic acdFindTypeC ***************************************************
**
** Looks for a Type by name, and returns the number in acdType
**
** @param [r] type [char*] Text string containing the type name
** @return [ajint] Integer representing the type (if known). Can be
**         used as position in the acdType array.
** @error If not found, the return value points to the maximum position in
**        acdType which is set to NULL throughout.
** @@
******************************************************************************/

static ajint acdFindTypeC (char* type)
{
    ajint i;
    ajint ifound=0;
    ajint j=-1;
    ajint ilen = strlen(type);
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (i=0; acdType[i].Name; i++)
    {
	if (!strcmp (type, acdType[i].Name))
	    return i;
	if (!strncmp (acdType[i].Name, type, ilen))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC (&ambigList, acdType[i].Name);
	}
    }

    if (ifound > 1)
    {			      /* warn now with the list, fail below */
	ajWarn ("ambiguous type %s (%S)", type, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound != 1)	       /* Fatal: but covered by other tests */
	acdError ("unknown type: '%s'\n", type);

    return j;
}

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

    for (i=QUAL_STAGE+1; acdKeywords[i].Name; i++)
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
	ajWarn ("ambiguous keyword %s (%S)", key, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound != 1)		/* Fatal: but strings are hardcoded */
	acdError ("unknown keyword: '%s'\n", key);

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
    
    acdLog ("acdUserGet '%S' reply '%S'\n", thys->Name, *reply);
    
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
	
	else if (ajStrLen(thys->StdPrompt))
	    (void) ajStrAssS (&msg, thys->StdPrompt);
	
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
	
	(void) acdLog ("acdUserGet '%S' defreply '%S' msg '%S'\n",
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

/* @funcstatic acdBadRetry ****************************************************
**
** Writes a message to stderr, and kills the application.
**
** @param [r] thys [AcdPAcd] ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdBadRetry (AcdPAcd thys)
{
    /* test acdc-retry */
    ajDie ("%S terminated: Bad value for '-%S' and no more retries",
	   acdProgram, thys->Name);
}

/* @funcstatic acdBadVal ******************************************************
**
** Writes a message to stderr, returns only if this is a required value
** and we are prompting for values.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] required [AjBool] If true, prompting for the value
**                              was possible.
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

    /*
     * replaced line below with following 2 to make msg more obvious to
     * the user
     */

    /*    (void) ajFmtPrintS(&msg, "option -%S: %s", name, fmt); */
    acdLog("Failure for option '%S'\n",name);
    (void) ajFmtPrintS(&msg, "%s", fmt);

    va_start (args, fmt) ;
    ajVErr (ajStrStr(msg), args);
    va_end (args) ;

    if (!required && !acdAuto)		/* test acdc-badadvanced */
	ajDie ("%S terminated: Bad value for '-%S' and no prompt",
	       acdProgram, name);

    if (acdAuto)			/* test acdc-badauto */
	ajDie ("%S terminated: Bad value for '-%S' with -auto defined",
	       acdProgram, name);

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
	(void) ajStrWrap (&appldoc, 75);
	ajUser("%S", appldoc);
    }

    return;
}

/* @funcstatic acdSetEndsec ***************************************************
**
** Ends the current ACD section
**
** Called when an "endsection" type ACD item is checked. Should not be called
** for any other item.
**
** At present there is nothing to prompt for here, though there could
** be, for example, a blank line at the end of a section where
** something was prompted for.
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

/* @funcstatic acdSetSec ******************************************************
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
    
    if (acdAttrToStr(thys, "type", "", &type))
    {
	if (!acdVocabCheck(type, typeVal))
	    acdErrorAcd (thys,
			 "section %S, bad attribute value type: %S",
			 thys->Name, type);
    }
    
    if (acdAttrToInt (thys, "border", 1, &border))
    {
	if (!ajStrMatchCaseC(type, "frame"))
	    (void) ajWarn("section %S, border only used by type: frame",
			  thys->Name);
	if (border < 1)
	{
	    (void) acdAttrToStr(thys, "border", "", &tmpstr);
	    acdErrorAcd (thys,
			 "section %S, bad attribute value type: %S",
			 tmpstr);
	}
    }
    
    (void) acdAttrToStr(thys, "comment", "", &comment);
    
    if (acdAttrToStr(thys, "folder", "", &folder))
    {
	if (!ajStrMatchCaseC(type, "page"))
	    ajWarn("section %S, folder only used by type: page",
		   thys->Name);
    }
    
    (void) acdAttrToStr(thys, "information", "", &info);
    
    if (acdAttrToStr(thys, "side", "", &side))
    {
	if (!acdVocabCheck(side, sideVal))
	    acdErrorAcd (thys,
			 "section %S, bad attribute value side: %S",
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

/* @funcstatic acdSetVar ******************************************************
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

/* @func ajAcdGetAlign ********************************************************
**
** Returns an item of type Align as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPAlign] Alignment output object. Already opened
**                      by ajAlignOpent so this just returns the object
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPAlign ajAcdGetAlign (char *token)
{
    return acdGetValue (token, "align");
}

/* @funcstatic acdSetAlign ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD alignment output item.
**
** Understands all attributes and associated qualifiers for this item type.
**
** The default value (if no other available) is a null string, which
** is invalid.
**
** Associated qualifiers "-aformat", "-aopenfile"
** are applied to the EURO before reading the sequence.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajSeqRead
** @@
******************************************************************************/

static void acdSetAlign (AcdPAcd thys)
{
    AjPAlign val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    
    static AjPStr name = NULL;
    static AjPStr ext = NULL;
    static AjPStr dir = NULL;
    static AjPStr outfname = NULL;
    static AjPStr fullfname = NULL;
    
    required = acdIsRequired(thys);
    val = ajAlignNew();
    
    (void) acdAttrToStr (thys, "type", "", &val->Type);
    (void) acdAttrToInt (thys, "minseqs", 0, &val->Nmin);
    (void) acdAttrToInt (thys, "maxseqs", 0, &val->Nmax);
    (void) acdAttrToBool (thys, "multiple", ajFalse, &val->Multi);
    
    (void) acdGetValueAssoc (thys, "aformat", &val->Formatstr);
    (void) acdGetValueAssoc (thys, "aextension", &ext);
    (void) acdGetValueAssoc (thys, "aname", &name);
    (void) acdGetValueAssoc (thys, "adirectory", &dir);
    (void) acdQualToInt (thys, "awidth", 50, &val->Width, &defreply);
    (void) acdQualToBool (thys, "aglobal", ajFalse, &val->Global, &defreply);
    (void) acdQualToBool (thys, "aaccshow", ajFalse, &val->Showacc, &defreply);
    (void) acdQualToBool (thys, "adesshow", ajFalse, &val->Showdes, &defreply);
    (void) acdQualToBool (thys, "ausashow", ajFalse, &val->Showusa, &defreply);
    
    if (!ajAlignValid (val))
    {					/* test acdc-alignbadformat */
	ajDie("Alignment option -%S: Validation failed",
	      thys->Name);
    }
    
    (void) acdOutDirectory (&dir);
    (void) acdOutFilename (&outfname, name, ext);
    (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    acdPromptAlign(thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	ajStrAssS(&fullfname, reply);
	ajFileSetDir (&fullfname, dir);
	ok = ajAlignOpen (val, fullfname);
	if (!ok)
	    acdBadVal (thys, required,
		       "Unable to open alignment file '%S'", fullfname);
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, fullfname);
    
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
    {
	acdErrorAcd (thys, "Array attribute size: %d less than 1", size);
    }
    
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

    val = acdGetValue (token, "boolean");
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

    AJNEW0(val);		   /* create storage for the result */

    *val = ajFalse;			/* set the default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "N", &defreply);

    acdLog("acdSetBool -%S def: %S\n", thys->Name, defreply);

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

    acdLog("acdSetBool -%S val: %B\n", thys->Name, *val);

    if (ajStrMatchC(thys->Name, "help"))
	acdHelp ();

    return;
}


/* @func ajAcdGetCodon ********************************************************
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


/* @funcstatic acdSetCodon ****************************************************
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

	ok = ajTrue;	   /* accept the default if nothing changes */

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

/* @func ajAcdGetCpdb *********************************************************
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


/* @funcstatic acdSetCpdb *****************************************************
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

	ok = ajTrue;	   /* accept the default if nothing changes */

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
    static AjPStr dir = NULL;
    
    static AjPStr datafname = NULL;
    
    val = NULL;				/* set the default value */
    
    (void) acdAttrResolve (thys, "name", &name);
    (void) acdAttrResolve (thys, "extension", &ext);
    (void) acdAttrResolve (thys, "directory", &dir);
    
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);
    
    required = acdIsRequired(thys);
    (void) acdDataFilename (&datafname, name, ext);
    (void) acdReplyInit (thys, ajStrStr(datafname), &defreply);
    acdPromptInfile (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	if (ajStrLen(reply))
	{
	    ajFileDataDirNew(reply, dir, &val);
	    if (!val)
	    {
		acdBadVal (thys, required,
			   "Unable to open data file '%S' for input",
			   reply);
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
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, ".", &defreply);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

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
			   "Unable to open file '%S' for input",
			   reply);
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

/* @func ajAcdGetDirlist ******************************************************
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


/* @funcstatic acdSetDirlist **************************************************
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
	ok = ajTrue;	   /* accept the default if nothing changes */

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
			   "Unable to open file '%S' for input",
			   reply);
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
** are applied to the UFO before reading the feature table.
**
** Associated qualifiers "-fbegin", "-fend" and "-freverse"
** are applied as appropriate, with prompting for values,
** after the feature table has been read.
** They are applied to the feature table,
** and the resulting table is what is set in the ACD item.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
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
    static AjPStr type = NULL;
    
    ajint fbegin=0;
    ajint fend=0;
    AjBool freverse=ajFalse;
    AjBool fprompt=ajFalse;
    ajint iattr;
    
    tabin = ajFeattabInNew();		/* set the default value */
    
    required = acdIsRequired(thys);
    (void) acdQualToBool (thys, "fask", ajFalse, &fprompt, &defreply);
    
    if (acdAttrToStr(thys, "type", "", &type))
    {
	if (!ajFeattabInSetType(tabin, type))
	    acdError ("Invalid type for feature input");
	acdInTypeFeatSave(type);
    }
    else
    {
	acdInTypeFeatSave(NULL);
    }

    (void) acdInFilename (&infname);
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
    acdPromptFeat (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

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
    
    (void) acdInFileSave(ajFeattableGetName(val)); /* save the sequence name */
    
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
    
    ok = acdQualToInt (thys, "fend", ajFeattableLen(val), &fend, &defreply);
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
    
    if (freverse)
	ajFeattableReverse (val);
    
    ajFeattableSetRange(val, fbegin, fend);
    
    ajFeattabInDel (&tabin);
    
    /* features tables have special set attributes */
    
    thys->SAttr = acdAttrListCount (acdCalcFeat);
    thys->SetAttr = &acdCalcFeat[0];
    thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));
    
    iattr = 0;
    (void) ajStrFromInt (&thys->SetStr[iattr++], ajFeattableBegin(val));
    (void) ajStrFromInt (&thys->SetStr[iattr++], ajFeattableEnd(val));
    (void) ajStrFromInt (&thys->SetStr[iattr++], ajFeattableLen(val));
    (void) ajStrFromBool (&thys->SetStr[iattr++], ajFeattableIsProt(val));
    (void) ajStrFromBool (&thys->SetStr[iattr++], ajFeattableIsNuc(val));
    (void) ajStrAssS (&thys->SetStr[iattr++], ajFeattableGetName(val));
    (void) ajStrFromInt (&thys->SetStr[iattr++], ajFeattableSize(val));
    
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
** Associated qualifiers "-offormat", "-ofopenfile"
** are applied to the UFO before opening the output file.
**
** @param [u] thys [AcdPAcd] ACD item.
** @return [void]
** @see ajFeatTabOutOpen
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
    static AjPStr type = NULL;
   
    required = acdIsRequired(thys);
    val = ajFeattabOutNew();
    
    if (!acdGetValueAssoc (thys, "ofname", &name))
	acdAttrResolve (thys, "name", &name);
    if (acdGetValueAssoc (thys, "offormat", &val->Formatstr))
	ajStrAssS (&ext, val->Formatstr);
    else
	(void) acdAttrResolve (thys, "extension", &ext);
    if (!ajStrLen(ext))
	(void) ajFeatOutFormatDefault(&ext);

    ajDebug("acdSetFeatout checking type\n");

    if (!acdAttrToStr(thys, "type", "", &type))
    {
	ajDebug("no type, try '%S'\n", type);
 	if (!acdInTypeFeat(&type))
	    ajWarn("No output type specified for '%S'", thys->Name);
    }

    ajDebug("Type '%S' try ajFeattabOutSetType\n", type);

    if (!ajFeattabOutSetType(val, type))
	acdError ("Invalid type for feature output");

    (void) acdOutDirectory (&val->Directory);
    (void) acdOutFilename (&outfname, name, ext);
    (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    acdPromptFeatout (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	(void) acdGetValueAssoc (thys, "ofopenfile", &val->Filename);
	(void) acdGetValueAssoc (thys, "ofdirectory", &val->Directory);
	(void) acdOutDirectory (&val->Directory);
	
	ok = ajFeattabOutOpen (val, reply);
	if (!ok)
	{
	    if (ajStrLen(val->Directory))
		acdBadVal (thys, required,
			   "Unable to open features output '%S%S'",
			   val->Directory, reply);
	    else
		acdBadVal (thys, required,
			   "Unable to open features output '%S'",
			   reply);
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);
    
    return;
}

/* @func ajAcdGetFilelist *****************************************************
**
** Returns a list of files given a comma-separated list.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPList] List of files.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPList ajAcdGetFilelist (char *token)
{
    return acdGetValue (token, "filelist");
}


/* @funcstatic acdSetFilelist *************************************************
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

static void acdSetFilelist (AcdPAcd thys)
{
    AjPList val;
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    AjBool nullok=ajFalse;

    val = NULL;

    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "", &defreply);
    acdPromptFilelist(thys);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (!ajStrLen(reply))
	{
	    if (!nullok)
	    {
		acdBadVal (thys, required, "File list is required");
		ok = ajFalse;
	    }
	}
    }
    if (!ok)
	acdBadRetry (thys);

    val = ajFileFileList(reply);

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

/* @func ajAcdGetGraph ********************************************************
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

/* @funcstatic acdSetGraph ****************************************************
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
#ifndef GROUT
    AjPGraph val;
    
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    static AjPStr title = NULL;
    static AjPStr gdev = NULL;
    
    /*   ajint multi;
	 
	 (void) acdAttrToInt (thys, "multi", 1, &multi);
	 if (multi < 1) multi = 1;
	 acdLog ("multi: %d\n", multi);
	 */
    
    required = acdIsRequired(thys);
    
    if(ajNamGetValueC ("GRAPHICS",&gdev))
	(void) acdReplyInit (thys, ajStrStr(gdev), &defreply);
    else
	(void) acdReplyInit (thys, "x11", &defreply);
    
    acdPromptGraph (thys);
    
    val = call ("ajGraphNew");
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	(void) call ("ajGraphSet",val, reply,&ok);
	if (!ok)
	{
	    (void) call ("ajGraphDumpDevices");
	    acdBadVal (thys, required,
		       "Invalid graph value '%S'", reply);
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssC (&thys->ValStr, "graph definition");
    
    if (acdGetValueAssoc (thys, "gtitle", &title))
	(void) call ("ajGraphxyTitle",val,title);
    
    if (acdGetValueAssoc (thys, "gsubtitle", &title))
	(void) call ("ajGraphxySubtitle",val,title);
    
    if (acdGetValueAssoc (thys, "gxtitle", &title))
	(void) call ("ajGraphxyXtitle",val,title);
    
    if (acdGetValueAssoc (thys, "gytitle", &title))
	(void) call ("ajGraphxyYtitle",val,title);
    
    if (acdGetValueAssoc (thys, "goutfile", &title))
	(void) call ("ajGraphxySetOutputFile",val,title);
    
    if (acdGetValueAssoc (thys, "gdirectory", &title))
	(void) call ("ajGraphxySetOutputDir",val,title);
    else
    {
	ajStrAssC(&title, "");
	if (acdOutDirectory(&title))
	    (void) call ("ajGraphxySetOutputDir",val,title);
    }
    
    
    (void) call ("ajGraphTrace",val);
    
#endif
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
#ifndef GROUT
    AjPGraph val;
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
       if(ajNamGetValueC(ajStrStr(name),&value))
       {
	   (void) ajStrAssS(&defreply,value);
	   ajStrDel(&value);
       }
       */
    
    if(ajNamGetValueC("GRAPHICS",&gdev))
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
	if (!ok)
	{
	    (void) call("ajGraphDumpDevices");
	    acdBadVal (thys, required,
		       "Invalid XY graph value '%S'", reply);
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssC (&thys->ValStr, "XY graph definition");
    
    if (acdGetValueAssoc (thys, "gtitle", &title))
	(void) call("ajGraphxyTitle",val,title);
    
    if (acdGetValueAssoc (thys, "gsubtitle", &title))
	(void) call("ajGraphxySubtitle",val,title);
    
    if (acdGetValueAssoc (thys, "gxtitle", &title))
	(void) call("ajGraphxyXtitle",val,title);
    
    if (acdGetValueAssoc (thys, "gytitle", &title))
	(void) call("ajGraphxyYtitle",val,title);
    
    if (acdGetValueAssoc (thys, "goutfile", &title))
	(void) call("ajGraphxySetOutputFile",val,title);
    
    if (acdGetValueAssoc (thys, "gdirectory", &title))
	(void) call("ajGraphxySetOutputDir",val,title);
    else
    {
	ajStrAssC(&title, "");
	if (acdOutDirectory(&title))
	    (void) call("ajGraphxySetOutputDir",val,title);
    }
    
    (void) call("ajGraphTrace",val);
    
#endif
    
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
	ok = ajTrue;	   /* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	if (ajStrLen(reply))
	{
	    val = ajFileNewIn(reply);
	    if (!val)
	    {
		acdBadVal (thys, required,
			   "Unable to open file '%S' for input",
			   reply);
		ok = ajFalse;
	    }
	}
	else
	{
	    if (!nullok)
	    {
		acdBadVal (thys, required,
			   "Input file is required");
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

/* @func ajAcdGetList *********************************************************
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

/* @func ajAcdGetListI ********************************************************
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
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	{
	    acdListPrompt(thys);
	    (void) acdUserGet (thys, &reply);
	}
	
	val = acdListValue(thys, min, max, reply);
	if (!val)
	{
	    acdBadVal (thys, required, "Bad menu option '%S'", reply);
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    for (i=0; val[i]; i++)
    {
	acdLog("Storing val[%d] '%S'\n", i,val[i]);
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
	ok = ajTrue;	   /* accept the default if nothing changes */
	
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

	ok = ajTrue;	   /* accept the default if nothing changes */

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
    AjBool append;
    
    static AjPStr name = NULL;
    static AjPStr ext = NULL;
    static AjPStr dir = NULL;
    static AjPStr outfname = NULL;
    static AjPStr fullfname = NULL;
    
    val = NULL;				/* set the default value */
    
    (void) acdAttrResolve (thys, "name", &name);
    (void) acdAttrResolve (thys, "extension", &ext);
    (void) acdGetValueAssoc(thys, "odirectory", &dir);
    
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    acdLog ("nullok: %B\n", nullok);
    (void) acdAttrToBool (thys, "append", ajFalse, &append);
    acdLog ("append: %B\n", append);
    
    required = acdIsRequired(thys);
    if (nullok)
    {
	(void) acdReplyInit (thys, "", &defreply);
    }
    else
    {
	(void) acdOutDirectory (&dir);
	(void) acdOutFilename (&outfname, name, ext);
	(void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    }
    acdPromptOutfile (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	if (ajStrLen(reply))
	{
	    ajStrAssS(&fullfname, reply);
	    ajFileSetDir (&fullfname, dir);
	    if (append)
	    {
		val = ajFileNewApp(fullfname);
	    }
	    else
	    {
		val = ajFileNewOut(fullfname);
	    }
	    if (!val)
	    {
		acdBadVal (thys, required,
			   "Unable to open file '%S' for output",
			   fullfname);
		ok = ajFalse;
	    }
	}
	else
	{
	    if (!nullok)
	    {
		acdBadVal (thys, required,
			   "Output file is required");
		ok = ajFalse;
	    }
	}
    }
    
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, fullfname);
    
    return;
}

/* @func ajAcdGetRange ********************************************************
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


/* @funcstatic acdSetRange ****************************************************
**
** Using the definition in the ACD file, and any values for the
** item or its associated qualifiers provided on the command line,
** prompts the user if necessary (and possible) and
** sets the actual value for an ACD range item.
**
** Understands all attributes and associated qualifiers for this item type.
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
	ok = ajTrue;	   /* accept the default if nothing changes */

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


/* @func ajAcdGetRegexp *******************************************************
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

static void acdSetRegexp (AcdPAcd thys)
{
    
    AjPRegexp val = NULL;
    
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    AjBool upper;
    AjBool lower;
    ajint itry;
    
    ajint minlen;
    ajint maxlen;
    ajint len;
    
    (void) acdAttrToInt (thys, "minlength", 1, &minlen);
    
    (void) acdAttrToInt (thys, "maxlength", INT_MAX, &maxlen);
    (void) acdAttrToBool (thys, "upper", ajFalse, &upper);
    (void) acdAttrToBool (thys, "lower", ajFalse, &lower);
    
    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "", &defreply);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	if (val)
	    ajRegFree (&val);
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	len = ajStrLen(reply);
	
	if (len < minlen)
	{
	    acdBadVal (thys, required,
		       "Too short (%S) - minimum length is %d characters",
		       thys->Name, minlen);
	    ok = ajFalse;
	}
	if (len > maxlen)
	{
	    acdBadVal (thys, required,
		       "Too long (%S) - maximum length is %d characters",
		       thys->Name,maxlen);
	    ok = ajFalse;
	}
	if (upper)
	    ajStrToUpper (&reply);
	
	if (lower)
	    ajStrToLower (&reply);
	if (ok)
	    val = ajRegComp (reply);
	if (ok && !val)
	{
	    acdBadVal (thys, required,
		       "Bad regular expression pattern:\n   '%S'",
		       reply);
	    ok = ajFalse;
	}
	
    }
    if (!ok)
	acdBadRetry (thys);
    
    /* regexps have special set attributes the same as strings */
    
    thys->SAttr = acdAttrListCount (acdCalcRegexp);
    thys->SetAttr = &acdCalcRegexp[0];
    thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));
    
    (void) ajStrFromInt (&thys->SetStr[0], ajStrLen(reply));
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);
    
    return;
}

/* @func ajAcdGetReport *******************************************************
**
** Returns an item of type Report as defined in a named ACD item.
** Called by the application after all ACD values have been set,
** and simply returns what the ACD item already has.
**
** @param [r] token [char*] Text token name
** @return [AjPReport] Report output object. Already opened
**                      by ajReportOpen so this just returns the object
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPReport ajAcdGetReport (char *token)
{
    return acdGetValue (token, "report");
}

/* @funcstatic acdSetReport ***************************************************
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
** are applied to the EURO before opening the output file.
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
    static AjPStr dir = NULL;
    static AjPStr outfname = NULL;
    static AjPStr fullfname = NULL;
    static AjPStr taglist = NULL;
    ajint mintags = 0;
    
    required = acdIsRequired(thys);
    val = ajReportNew();
    
    (void) acdAttrToStr (thys, "type", "", &val->Type);
    (void) acdAttrToInt (thys, "mintags", 0, &val->Mintags);
    (void) acdAttrToStr (thys, "taglist", "", &taglist);
    (void) acdAttrToBool (thys, "multiple", ajFalse, &val->Multi);
    (void) acdAttrToInt (thys, "precision", 3, &val->Precision);
    
    (void) acdGetValueAssoc (thys, "rdirectory", &dir);
    (void) acdGetValueAssoc (thys, "rextension", &ext);
    (void) acdGetValueAssoc (thys, "rname", &name);
    (void) acdGetValueAssoc (thys, "rformat", &val->Formatstr);
    (void) acdQualToBool (thys, "raccshow", ajFalse,
			  &val->Showacc, &defreply);
    (void) acdQualToBool (thys, "rdesshow", ajFalse,
			  &val->Showdes, &defreply);
    (void) acdQualToBool (thys, "rscoreshow", ajTrue,
			  &val->Showscore, &defreply);
    (void) acdQualToBool (thys, "rusashow", ajFalse,
			  &val->Showusa, &defreply);
    
    if (!ajReportSetTags (val, taglist, mintags))
    {				      /* test acdc-reportbadtaglist */
	acdErrorAcd (thys, "Bad tag list for report");
    }

    if (!ajReportValid (val))
    {					/* test acdc-reportbadtags */
	ajDie ("Report option -%S: Validation failed",
	       thys->Name);
    }
    
    (void) acdOutDirectory (&dir);
    (void) acdOutFilename (&outfname, name, ext);
    (void) acdReplyInit (thys, ajStrStr(outfname), &defreply);
    acdPromptReport (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	ajStrAssS(&fullfname, reply);
	ajFileSetDir (&fullfname, dir);
	ok = ajReportOpen (val, fullfname);
	if (!ok)
	    acdBadVal (thys, required,
		       "Unable to open report file '%S'",
		       fullfname);
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, fullfname);
    
    return;
}

/* @func ajAcdGetScop *********************************************************
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


/* @funcstatic acdSetScop *****************************************************
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

	ok = ajTrue;	   /* accept the default if nothing changes */

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

AjPStr* ajAcdGetSelect (char *token)
{
    return acdGetValue (token, "selection");
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

AjPStr ajAcdGetSelectI (char *token, ajint num)
{
    AjPStr* val;
    ajint i;

    val =  acdGetValue (token, "select");

    for (i=1; i<num; i++)
    {
	if (!val[i])
	{
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

static void acdSetSelect (AcdPAcd thys)
{

    AjPStr* val;

    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    ajint itry;
    ajint i;

    ajint min=0, max=5;

    val = NULL;				/* set the default value */

    required = acdIsRequired(thys);
    (void) acdReplyInit (thys, "", &defreply);

    (void) acdAttrToInt (thys, "minimum", 1, &min);
    (void) acdAttrToInt (thys, "maximum", 1, &max);

    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	{
	    acdSelectPrompt (thys);
	    (void) acdUserGet (thys, &reply);
	}

	/*
	   AJCNEW0(val, 2);
	   (void) ajStrAssS (&val[0], reply);
	   */
	val = acdSelectValue(thys, min, max, reply);
	if (!val)
	{
	    acdBadVal (thys, required, "Bad select option '%S'", reply);
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);

    thys->Value = val;
    for (i=0; val[i]; i++)
    {
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

AjPSeq ajAcdGetSeq (char *token)
{
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

static void acdSetSeq (AcdPAcd thys)
{
    AjPSeq val;
    AjPSeqin seqin;
    
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    AjBool okbeg = ajFalse;
    AjBool okend = ajFalse;
    AjBool okrev = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    static AjPStr promptreply = NULL;
    static AjPStr tmpstr = NULL;
    ajint itry;
    ajint i;
    
    static AjPStr infname = NULL;
    
    ajint sbegin=0;
    ajint send=0;
    AjBool sreverse=ajFalse;
    AjBool sprompt=ajFalse;
    AjBool snuc=ajFalse;
    AjBool sprot=ajFalse;
    AjBool nullok=ajFalse;
    
    
    val = ajSeqNew();		/* set the default value */
    seqin = ajSeqinNew();		/* set the default value */
    
    /* seqin->multi = ajFalse; */ /* pmr: moved to ajSeqinNew */
    
    (void) acdQualToBool (thys, "snucleotide", ajFalse, &snuc, &defreply);
    (void) acdQualToBool (thys, "sprotein", ajFalse, &sprot, &defreply);
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    
    required = acdIsRequired(thys);
    (void) acdInFilename (&infname);
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
    acdPromptSeq (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	if(!ajStrLen(reply))
	    if(nullok)
		continue;
	
	ajSeqinUsa (&seqin, reply);
	
	if (acdAttrToStr(thys, "type", "", &seqin->Inputtype))
	    acdInTypeSeqSave(seqin->Inputtype);
	else
	    acdInTypeSeqSave(NULL);
	
	(void) acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	(void) acdAttrToBool(thys, "entry", ajFalse, &seqin->Text);
	
	(void) acdGetValueAssoc (thys, "sformat", &seqin->Formatstr);
	(void) acdGetValueAssoc (thys, "sdbname", &seqin->Db);
	(void) acdGetValueAssoc (thys, "sopenfile", &seqin->Filename);
	(void) acdGetValueAssoc (thys, "sid", &seqin->Entryname);
	
	(void) acdGetValueAssoc (thys, "ufo", &seqin->Ufo);
	
	(void) acdGetValueAssoc (thys, "fformat", &seqin->Ftquery->Formatstr);
	(void) acdGetValueAssoc (thys, "fopenfile", &seqin->Ftquery->Filename);
	
	(void) acdQualToBool (thys, "supper", ajFalse, &seqin->Upper, &tmpstr);
	(void) acdQualToBool (thys, "slower", ajFalse, &seqin->Lower, &tmpstr);
	okbeg = acdQualToSeqbegin (thys, "sbegin", 0, &sbegin, &tmpstr);
	okend = acdQualToSeqend (thys, "send", 0, &send, &tmpstr);
	okrev = acdQualToBool (thys, "sreverse", ajFalse, &sreverse, &tmpstr);
	
	if (snuc)
	    ajSeqinSetNuc (seqin);
	if (sprot)
	    ajSeqinSetProt (seqin);
	
	i = ajStrLen(seqin->Ufo) + ajStrLen(seqin->Ftquery->Formatstr)
	    + ajStrLen(seqin->Ftquery->Filename);
	
	if (i && !seqin->Features)
	    ajWarn ("Feature table ignored");
	
	if (seqin->Features)
	    acdLog ("acdSetSeq with features UFO '%S'\n", seqin->Ufo);
	
	/* (try to) read the sequence */
	
	ok = ajSeqRead(val, seqin);
	if (!ok)
	{
	    acdBadVal (thys, required,
		       "Unable to read sequence '%S'", reply);
	}
    }
    
    if (!ok)
	acdBadRetry (thys);
    
    (void) acdInFileSave(ajSeqGetName(val)); /* save the sequence name */
    
    /* some standard options using associated qualifiers */
    
    (void) acdQualToBool (thys, "sask", ajFalse, &sprompt, &defreply);
    
    /* now process the begin, end and reverse options */
    
    if (seqin->Begin)
    {
	okbeg = ajTrue;
    }
    
    for (itry=acdPromptTry; itry && !okbeg; itry--)
    {
	(void) ajStrAssC (&promptreply, "start");
	if (sprompt)
	    (void) acdUserGetPrompt (" Begin at position", &promptreply);
	if (ajStrMatchCaseC(promptreply, "start"))
	    (void) ajStrAssC(&promptreply, "0");
	okbeg = ajStrToInt(promptreply, &sbegin);
	if (!okbeg)
	    acdBadVal (thys, sprompt,
		       "Invalid sequence position '%S'", promptreply);
    }
    if (!okbeg)
	acdBadRetry (thys);
    
    if (sbegin)
    {
	seqin->Begin = sbegin;
	val->Begin = sbegin;
	(void) acdSetQualDefInt(thys, "sbegin", sbegin);
    }
    
    if (seqin->End)
    {
	okend = ajTrue;
    }
    
    for (itry=acdPromptTry; itry && !okend; itry--)
    {
	(void) ajStrAssC (&promptreply, "end");
	if (sprompt)
	    (void) acdUserGetPrompt ("   End at position", &promptreply);
	if (ajStrMatchCaseC(promptreply, "end"))
	    (void) ajStrAssC(&promptreply, "0");
	okend = ajStrToInt(promptreply, &send);
	if (!okend)
	    acdBadVal (thys, sprompt,
		       "Invalid sequence position '%S'", promptreply);
    }
    if (!okend)
	acdBadRetry (thys);
    
    if (send)
    {
	seqin->End = send;
	val->End = send;
	(void) acdSetQualDefInt(thys, "send", send);
    }
    
    if (ajSeqIsNuc(val))
    {
	if (seqin->Rev)
	{
	    okrev = ajTrue;
	}
	for (itry=acdPromptTry; itry && !okrev; itry--)
	{
	    (void) ajStrAssC (&promptreply, "N");
	    if (sprompt)
		(void) acdUserGetPrompt ("    Reverse strand", &promptreply);
	    okrev = ajStrToBool(promptreply, &sreverse);
	    if (!okrev)
		acdBadVal (thys, sprompt,
			   "Invalid Y/N value '%S'", promptreply);
	}
	if (!okrev)
	    acdBadRetry (thys);
	if (sreverse)
	{
	    seqin->Rev = sreverse;
	    val->Rev = sreverse;
	    (void) acdSetQualDefBool(thys, "sreverse", sreverse);
	}
    }
    
    acdLog ("sbegin: %d, send: %d, sreverse: %s\n",
	    sbegin, send, ajStrBool(sreverse));
    
    if (val->Rev)
	ajSeqReverse (val);
    
    ajSeqinDel (&seqin);
    
    /* sequences have special set attributes */
    
    thys->SAttr = acdAttrListCount (acdCalcSeq);
    thys->SetAttr = &acdCalcSeq[0];
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

AjPSeqset ajAcdGetSeqset (char *token)
{
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


static void acdSetSeqset (AcdPAcd thys)
{
    AjPSeqset val;
    AjPSeqin seqin;
    
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    AjBool okbeg = ajFalse;
    AjBool okend = ajFalse;
    AjBool okrev = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    static AjPStr promptreply = NULL;
    static AjPStr tmpstr = NULL;
    ajint itry;
    
    static AjPStr infname = NULL;
    
    ajint sbegin=0;
    ajint send=0;
    AjBool sreverse=ajFalse;
    AjBool sprompt=ajFalse;
    AjBool snuc=ajFalse;
    AjBool sprot=ajFalse;
    AjBool nullok=ajFalse;
    
    val = ajSeqsetNew();		/* set the default value */
    seqin = ajSeqinNew();		/* set the default value */
    
    seqin->multi = ajTrue;  /* pmr: moved to ajSeqinNew */ /* ajb added back */
    
    (void) acdQualToBool (thys, "snucleotide", ajFalse, &snuc, &defreply);
    (void) acdQualToBool (thys, "sprotein", ajFalse, &sprot, &defreply);
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    
    required = acdIsRequired(thys);
    (void) acdInFilename (&infname);
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
    acdPromptSeq (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	
	if(!ajStrLen(reply))
	    if(nullok)
		continue;

	ajSeqinUsa (&seqin, reply);
	
	if (acdAttrToStr(thys, "type", "", &seqin->Inputtype))
	    acdInTypeSeqSave(seqin->Inputtype);
	else
	    acdInTypeSeqSave(NULL);
	
	(void) acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	
	(void) acdGetValueAssoc (thys, "sformat", &seqin->Formatstr);
	(void) acdGetValueAssoc (thys, "sdbname", &seqin->Db);
	(void) acdGetValueAssoc (thys, "sopenfile", &seqin->Filename);
	(void) acdGetValueAssoc (thys, "sid", &seqin->Entryname);
	
	(void) acdGetValueAssoc (thys, "ufo", &seqin->Ufo);
	(void) acdGetValueAssoc (thys, "fformat", &seqin->Ftquery->Formatstr);
	(void) acdGetValueAssoc (thys, "fopenfile", &seqin->Ftquery->Filename);
	
	(void) acdQualToBool (thys, "supper", ajFalse, &seqin->Upper, &tmpstr);
	(void) acdQualToBool (thys, "slower", ajFalse, &seqin->Lower, &tmpstr);
	okbeg = acdQualToSeqbegin (thys, "sbegin", 0, &seqin->Begin, &tmpstr);
	okend = acdQualToSeqend (thys, "send", 0, &seqin->End, &tmpstr);
	okrev = acdQualToBool (thys, "sreverse",
			       ajFalse, &seqin->Rev, &tmpstr);
	
	if (snuc)
	    ajSeqinSetNuc (seqin);
	
	if (sprot)
	    ajSeqinSetProt (seqin);
	
	if (ajStrLen(seqin->Ufo))
	    seqin->Features = ajTrue;
	
	ok = ajSeqsetRead(val, seqin);
	if (!ok)
	{
	    acdBadVal (thys, required,
		       "Unable to read sequence '%S'", reply);
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    (void) acdInFileSave(ajSeqsetGetName(val)); /* save the sequence name */
    
    (void) acdQualToBool (thys, "sask", ajFalse, &sprompt, &defreply);
    
    /* now process the begin, end and reverse options */
    
    if (seqin->Begin)
    {
	okbeg = ajTrue;
	val->Begin = seqin->Begin;
    }
    
    for (itry=acdPromptTry; itry && !okbeg; itry--)
    {
	(void) ajStrAssC (&promptreply, "start");
	if (sprompt)
	    (void) acdUserGetPrompt (" Begin at position", &promptreply);
	if (ajStrMatchCaseC(promptreply, "start"))
	    (void) ajStrAssC(&promptreply, "0");
	okbeg = ajStrToInt(promptreply, &sbegin);
	if (!okbeg)
	    acdBadVal (thys, sprompt,
		       "Invalid integer value '%S'", promptreply);
    }
    if (!okbeg)
	acdBadRetry (thys);
    
    if (sbegin)
    {
	seqin->Begin = sbegin;
	val->Begin = sbegin;
	(void) acdSetQualDefInt(thys, "sbegin", sbegin);
    }
    
    if (seqin->End)
    {
	okend = ajTrue;
	val->End = seqin->End;
    }
    
    for (itry=acdPromptTry; itry && !okend; itry--)
    {
	(void) ajStrAssC (&promptreply, "end");
	if (sprompt)
	    (void) acdUserGetPrompt ("   End at position", &promptreply);
	if (ajStrMatchCaseC(promptreply, "end"))
	    (void) ajStrAssC(&promptreply, "0");
	okend = ajStrToInt(promptreply, &send);
	if (!okend)
	    acdBadVal (thys, sprompt,
		       "Invalid integer value '%S'", promptreply);
    }
    if (!okend)
	acdBadRetry (thys);
    
    if (send)
    {
	seqin->End = send;
	val->End = send;
	(void) acdSetQualDefInt(thys, "send", send);
    }

    if (ajSeqsetIsNuc(val))
    {
	if (seqin->Rev)
	{
	    okrev = ajTrue;
	    val->Rev = seqin->Rev;
	}
	for (itry=acdPromptTry; itry && !okrev; itry--)
	{
	    (void) ajStrAssC (&promptreply, "N");
	    if (sprompt)
		(void) acdUserGetPrompt ("    Reverse strand", &promptreply);
	    okrev = ajStrToBool(promptreply, &sreverse);
	    if (!okrev)
		acdBadVal (thys, sprompt,
			   "Invalid Y/N value '%S'", promptreply);
	}
	if (!okrev)
	    acdBadRetry (thys);
	if (sreverse)
	{
	    seqin->Rev = sreverse;
	    val->Rev = sreverse;
	    (void) acdSetQualDefBool(thys, "sreverse", sreverse);
	}
    }
    
    acdLog ("sbegin: %d, send: %d, sreverse: %s\n",
	    sbegin, send, ajStrBool(sreverse));
    
    if (val->Rev)
	ajSeqsetReverse (val);
    
    ajSeqinDel (&seqin);
    
    /* sequences have special set attributes */
    
    thys->SAttr = acdAttrListCount (acdCalcSeqset);
    thys->SetAttr = &acdCalcSeqset[0];
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

AjPSeqall ajAcdGetSeqall (char *token)
{
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

static void acdSetSeqall (AcdPAcd thys)
{
    AjPSeqall val;
    AjPSeqin seqin;
    AjPSeq seq;
    
    AjBool required = ajFalse;
    AjBool ok = ajFalse;
    AjBool okbeg = ajFalse;
    AjBool okend = ajFalse;
    AjBool okrev = ajFalse;
    static AjPStr defreply = NULL;
    static AjPStr reply = NULL;
    static AjPStr promptreply = NULL;
    static AjPStr tmpstr = NULL;
    ajint itry;
    AjBool nullok=ajFalse;
    
    static AjPStr infname = NULL;
    
    ajint sbegin=0;
    ajint send=0;
    AjBool sreverse=ajFalse;
    AjBool sprompt=ajFalse;
    AjBool snuc=ajFalse;
    AjBool sprot=ajFalse;
    
    val = ajSeqallNew();		/* set the default value */
    seqin = val->Seqin;
    seqin->multi = ajTrue;
    seq = val->Seq;
    
    (void) acdQualToBool (thys, "snucleotide", ajFalse, &snuc, &defreply);
    (void) acdQualToBool (thys, "sprotein", ajFalse, &sprot, &defreply);
    (void) acdAttrToBool (thys, "nullok", ajFalse, &nullok);
    
    required = acdIsRequired(thys);
    (void) acdInFilename (&infname);
    (void) acdReplyInit (thys, ajStrStr(infname), &defreply);
    acdPromptSeq (thys);
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	if(!ajStrLen(reply))
	    if(nullok)
		continue;
	
	ajSeqinUsa (&seqin, reply);
	
	if (acdAttrToStr(thys, "type", "", &seqin->Inputtype))
	    acdInTypeSeqSave(seqin->Inputtype);
	else
	    acdInTypeSeqSave(NULL);
	
	(void) acdAttrToBool(thys, "features", ajFalse, &seqin->Features);
	(void) acdAttrToBool(thys, "entry", ajFalse, &seqin->Text);
	
	(void) acdGetValueAssoc (thys, "sformat", &seqin->Formatstr);
	(void) acdGetValueAssoc (thys, "sdbname", &seqin->Db);
	(void) acdGetValueAssoc (thys, "sopenfile", &seqin->Filename);
	(void) acdGetValueAssoc (thys, "sid", &seqin->Entryname);
	
	(void) acdGetValueAssoc (thys, "ufo", &seqin->Ufo);
	(void) acdGetValueAssoc (thys, "fformat", &seqin->Ftquery->Formatstr);
	(void) acdGetValueAssoc (thys, "fopenfile", &seqin->Ftquery->Filename);
	
	(void) acdQualToBool (thys, "supper", ajFalse, &seqin->Upper, &tmpstr);
	(void) acdQualToBool (thys, "slower", ajFalse, &seqin->Lower, &tmpstr);
	okbeg = acdQualToSeqbegin (thys, "sbegin", 0, &seqin->Begin, &tmpstr);
	okend = acdQualToSeqend (thys, "send", 0, &seqin->End, &tmpstr);
	okrev = acdQualToBool (thys, "sreverse",
			       ajFalse, &seqin->Rev, &tmpstr);
	
	if (snuc)
	    ajSeqinSetNuc (seqin);
	
	if (sprot)
	    ajSeqinSetProt (seqin);
	
	if (ajStrLen(seqin->Ufo))
	    seqin->Features = ajTrue;
	
	ok = ajSeqAllRead(seq, seqin);
	if (!ok)
	{
	    acdBadVal (thys, required,
		       "Unable to read sequence '%S'", reply);
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    /*  commentedout__ajSeqinDel (&seqin);*/
    
    (void) acdInFileSave(ajSeqallGetName(val)); /* save the sequence name */
    
    (void) acdQualToBool (thys, "sask", ajFalse, &sprompt, &defreply);
    
    /* now process the begin, end and reverse options */
    
    if (seqin->Begin)
    {
	okbeg = ajTrue;
	val->Begin = seqin->Begin;
    }
    
    for (itry=acdPromptTry; itry && !okbeg; itry--)
    {
	(void) ajStrAssC (&promptreply, "start");
	if (sprompt)
	    (void) acdUserGetPrompt (" Begin at position", &promptreply);
	if (ajStrMatchCaseC(promptreply, "start"))
	    (void) ajStrAssC(&promptreply, "0");
	okbeg = ajStrToInt(promptreply, &sbegin);
	if (!okbeg)
	    acdBadVal (thys, sprompt,
		       "Invalid integer value '%S'", promptreply);
    }
    if (!okbeg)
	acdBadRetry (thys);
    
    if (sbegin)
    {
	seqin->Begin = sbegin;
	seq->Begin = sbegin;
	val->Begin = sbegin;
	(void) acdSetQualDefInt(thys, "sbegin", sbegin);
    }
    
    if (seqin->End)
    {
	okend = ajTrue;
	val->End = seqin->End;
    }
    
    for (itry=acdPromptTry; itry && !okend; itry--)
    {
	(void) ajStrAssC (&promptreply, "end");
	if (sprompt)
	    (void) acdUserGetPrompt ("   End at position", &promptreply);
	if (ajStrMatchCaseC(promptreply, "end"))
	    (void) ajStrAssC(&promptreply, "0");
	okend = ajStrToInt(promptreply, &send);
	if (!okend)
	    acdBadVal (thys, sprompt,
		       "Invalid integer value '%S'", promptreply);
    }
    if (!okend)
	acdBadRetry (thys);
    
    if (send)
    {
	seqin->End = send;
	seq->End = send;
	val->End = send;
	(void) acdSetQualDefInt(thys, "send", send);
    }

    if (ajSeqIsNuc(seq))
    {
	if (seqin->Rev)
	{
	    okrev = ajTrue;
	    val->Rev = seqin->Rev;
	}
	for (itry=acdPromptTry; itry && !okrev; itry--)
	{
	    (void) ajStrAssC (&promptreply, "N");
	    if (sprompt)
		(void) acdUserGetPrompt ("    Reverse strand", &promptreply);
	    okrev = ajStrToBool(promptreply, &sreverse);
	    if (!okrev)
		acdBadVal (thys, sprompt,
			   "Invalid Y/N value '%S'", promptreply);
	}
	if (!okrev)
	    acdBadRetry (thys);
	if (sreverse)
	{
	    seqin->Rev = sreverse;
	    seq->Rev = sreverse;
	    val->Rev = sreverse;
	    (void) acdSetQualDefBool(thys, "sreverse", sreverse);
	}
    }
    
    /*
       //  ajUser ("ajAcdSetSeqall s %d .. %d %b  seqin %d .. %d %b  "
		   //"val %d .. %d %b  seq %d .. %d %b\n",
		   //	  sbegin, send, sreverse,
		   //	  seqin->Begin, seqin->End, seqin->Rev,
		   //	  val->Begin, val->End, val->Rev,
		   //	  seq->Begin, seq->End, seq->Rev);
		   */
    
    if (val->Rev)
	ajSeqallReverse (val);
    
    acdLog ("sbegin: %d, send: %d, sreverse: %s\n",
	    sbegin, send, ajStrBool(sreverse));
    
    /* sequences have special set attributes */
    
    thys->SAttr = acdAttrListCount (acdCalcSeqall);
    thys->SetAttr = &acdCalcSeqall[0];
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

AjPSeqout ajAcdGetSeqout (char *token)
{
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

static void acdSetSeqout (AcdPAcd thys)
{
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
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	ajSeqoutUsa (&val, reply);	/* resets the AjPSeqout */
	val->Features = osfeat;
	(void) acdGetValueAssoc(thys, "osdirectory", &val->Directory);
	(void) acdOutDirectory (&val->Directory);
	acdLog ("acdSetSeqout features: %B dir '%S'\n",
		val->Features, val->Directory);
	
	(void) ajStrSet(&val->Formatstr, fmt);
	if (!ajStrLen(val->Formatstr))
	    (void) ajSeqOutFormatDefault(&val->Formatstr);
	
	(void) ajStrSet(&val->Extension, ext);
	(void) ajStrSet(&val->Extension, val->Formatstr);
	
	(void) acdGetValueAssoc (thys, "oufo", &val->Ufo);
	(void) acdGetValueAssoc (thys, "offormat", &val->Ftquery->Formatstr);
	if (!ajStrLen(val->Ftquery->Formatstr))
	    (void) ajFeatOutFormatDefault(&val->Ftquery->Formatstr);
	(void) acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename);
	(void) acdGetValueAssoc (thys, "ofdirectory",
				 &val->Ftquery->Directory);
	(void) acdOutDirectory (&val->Ftquery->Directory);
	
	(void) acdQualToBool (thys, "ossingle",
			      ajFalse,
			      &val->Single, &sing);
	
	if (!acdAttrToStr(thys, "type", "", &val->Outputtype))
	{
	    if (!acdInTypeSeq(&val->Outputtype))
		ajWarn("No output type specified for '%S'", thys->Name);
	}
	
	if (!ajSeqoutOpen(val))
	{
	    if (ajStrLen(val->Directory))
		acdBadVal (thys, required,
			   "Unable to write sequence to '%S%S'",
			   val->Directory, reply);
	    else
		acdBadVal (thys, required,
			   "Unable to write sequence to '%S'",
			   reply);
	    ok = ajFalse;
	}
    }
    if (!ok)
	acdBadRetry (thys);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, reply);
    acdLog ("acdSetSeqout features: %B\n", val->Features);
    if (val->Features)
	acdLog ("acdSetSeqout with features UFO '%S'\n", val->Ufo);
    
    
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

AjPSeqout ajAcdGetSeqoutset (char *token)
{
    return acdGetValue (token, "seqoutset");
}

/* @funcstatic acdSetSeqoutset ************************************************
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

static void acdSetSeqoutset (AcdPAcd thys)
{
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
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	ajSeqoutUsa (&val, reply);
	val->Features = osfeat;
	(void) acdGetValueAssoc(thys, "osdirectory", &val->Directory);
	(void) acdOutDirectory (&val->Directory);
	acdLog ("acdSetSeqoutset features: %B\n", val->Features);
	
	(void) ajStrSet(&val->Formatstr, fmt);
	if (!ajStrLen(val->Formatstr))
	    (void) ajSeqOutFormatDefault(&val->Formatstr);
	
	(void) ajStrSet(&val->Extension, ext);
	(void) ajStrSet(&val->Extension, val->Formatstr);
	
	(void) acdGetValueAssoc (thys, "oufo", &val->Ufo);
	(void) acdGetValueAssoc (thys, "offormat", &val->Ftquery->Formatstr);
	if (!ajStrLen(val->Ftquery->Formatstr))
	    (void) ajFeatOutFormatDefault(&val->Ftquery->Formatstr);
	(void) acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename);
	(void) acdGetValueAssoc (thys, "ofdirectory",
				 &val->Ftquery->Directory);
	(void) acdOutDirectory (&val->Ftquery->Directory);
	
	(void) acdQualToBool (thys, "ossingle",
			      ajSeqOutFormatSingle(val->Formatstr),
			      &val->Single, &sing);
	
	if (!acdAttrToStr(thys, "type", "", &val->Outputtype))
	{
	    if (!acdInTypeSeq(&val->Outputtype))
		ajWarn("No output type specified for '%S'", thys->Name);
	}
	
	if (!ajSeqoutOpen(val))
	{
	    if (ajStrLen(val->Directory))
		acdBadVal (thys, required,
			   "Unable to write sequence to '%S%S'",
			   val->Directory, reply);
	    else
		acdBadVal (thys, required,
			   "Unable to write sequence to '%S'",
			   reply);
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

AjPSeqout ajAcdGetSeqoutall (char *token)
{
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

static void acdSetSeqoutall (AcdPAcd thys)
{
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
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;		/* accept the default if nothing changes */
	
	(void) ajStrAssS (&reply, defreply);
	
	if (required)
	    (void) acdUserGet (thys, &reply);
	
	ajSeqoutUsa (&val, reply);
	val->Features = osfeat;
	(void) acdGetValueAssoc(thys, "osdirectory", &val->Directory);
	(void) acdOutDirectory (&val->Directory);
	acdLog ("acdSetSeqoutall features: %B\n", val->Features);
	
	(void) ajStrSet(&val->Formatstr, fmt);
	if (!ajStrLen(val->Formatstr))
	    (void) ajSeqOutFormatDefault(&val->Formatstr);
	
	(void) ajStrSet(&val->Extension, ext);
	(void) ajStrSet(&val->Extension, val->Formatstr);
	
	(void) acdGetValueAssoc (thys, "oufo", &val->Ufo);
	(void) acdGetValueAssoc (thys, "offormat", &val->Ftquery->Formatstr);
	if (!ajStrLen(val->Ftquery->Formatstr))
	    (void) ajFeatOutFormatDefault(&val->Ftquery->Formatstr);
	/*
	   if (!acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename))
	   (void) acdOutFilename (&val->Ftquery->Filename,
				  val->Ftquery->Filename, val->Ftquery->Formatstr);
				  */
	
	acdGetValueAssoc (thys, "ofname", &val->Ftquery->Filename);
	
	(void) acdGetValueAssoc (thys, "ofdirectory", &val->Ftquery->Directory);
	(void) acdOutDirectory (&val->Ftquery->Directory);
	
	(void) acdLog ("acdSetSeqoutall ossingle default: %B\n",
		       ajSeqOutFormatSingle(val->Formatstr));
	
	(void) acdQualToBool (thys, "ossingle",
			      ajSeqOutFormatSingle(val->Formatstr),
			      &val->Single, &sing);
	
	(void) acdLog ("acdSetSeqoutall ossingle value %B '%S'\n",
		       val->Single, sing);
	
	if (!acdAttrToStr(thys, "type", "", &val->Outputtype))
	{
	    if (!acdInTypeSeq(&val->Outputtype))
		ajWarn("No output type specified for '%S'", thys->Name);
	}
	
	if (!ajSeqoutOpen(val))
	{
	    if (ajStrLen(val->Directory))
		acdBadVal (thys, required,
			   "Unable to write sequence to '%S%S'",
			   val->Directory, reply);
	    else
		acdBadVal (thys, required,
			   "Unable to write sequence to '%S'",
			   reply);
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

AjPStr ajAcdGetString (char *token)
{
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

static void acdSetString (AcdPAcd thys)
{
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
    
    for (itry=acdPromptTry; itry && !ok; itry--)
    {
	ok = ajTrue;	   /* accept the default if nothing changes */

	(void) ajStrAssS (&reply, defreply);

	if (required)
	    (void) acdUserGet (thys, &reply);

	len = ajStrLen(reply);

	if (len < minlen)
	{
	    acdBadVal (thys, required,
		       "Too short (%S) - minimum length is %d characters",
		       thys->Name, minlen);
	    ok = ajFalse;
	}
	if (len > maxlen)
	{
	    acdBadVal (thys, required,
		       "Too long (%S) - maximum length is %d characters",
		       thys->Name, maxlen);
	    ok = ajFalse;
	}
	if (patexp && !ajRegExec (patexp, reply))
	{
	    acdBadVal (thys, required,
		       "String does not match pattern '%S'",
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
    
    thys->SAttr = acdAttrListCount (acdCalcString);
    thys->SetAttr = &acdCalcString[0];
    thys->SetStr = AJCALLOC0 (thys->SAttr, sizeof (AjPStr));
    
    (void) ajStrFromInt (&thys->SetStr[0], ajStrLen(reply));
    
    (void) ajStrAssS(&val, reply);
    
    thys->Value = val;
    (void) ajStrAssS (&thys->ValStr, val);
    
    return;
}

/* @func ajAcdValue ***********************************************************
**
** Returns the string value of any ACD item
**
** @param [r] token [char*] Text token name
** @return [AjPStr] String object. The string was already set by
**         acdSetString so this just returns the pointer.
** @cre failure to find an item with the right name and type aborts.
** @@
******************************************************************************/

AjPStr ajAcdValue (char *token)
{
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

static ajint acdAttrCount (ajint itype)
{
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

static ajint acdAttrKeyCount (ajint ikey)
{
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

static ajint acdAttrListCount (AcdPAttr attr)
{
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

static void* acdGetValue (char *token, char* type)
{
    void* ret;
    ajint pnum = 0;		   /* need to get from end of token */

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

static AjPStr acdGetValStr (char *token)
{
    AcdPAcd acd;
    ajint pnum = 0;		   /* need to get from end of token */
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

static AjBool acdGetValueAssoc (AcdPAcd thys, char *token, AjPStr *result)
{
    ajint pnum = 0;		   /* need to get from end of token */
    AcdPAcd pa;
    char *cp = ajCharNewC(strlen(token), token);

    acdLog ("acdGetValueAssoc '%s' (%S)\n", token, thys->Name);

    acdTokenToLower (cp, &pnum);
    (void) ajCharFree(cp);

    if (pnum)
	acdErrorAcd (thys,
		     "associated token '%s' is numbered - not allowed\n",
		     token);

    for (pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
    {
	if (ajStrMatchC(pa->Token, token))
	{
	    (void) ajStrAssS(result, pa->ValStr);
	    return pa->Defined;
	}
    }

    acdErrorAcd (thys,
		 "Token '%s' not found\n",
		 token);

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

static void* acdGetValueNum (char *token, char* type, ajint pnum)
{
    AcdPAcd pa;
    AcdPAcd ret = NULL;
    ajint itype = 0;
    ajint ifound = 0;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    if (type)
	itype = acdFindTypeC(type);

    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	if (ajStrMatchC(pa->Token, token))
	{
	    if ((itype>=0) && (pa->Type != itype)) /* program source error */
	    {
		ajDie ("Value for '-%S' is not of type %s\n", pa->Token, type);
	    }
	    if (pa->PNum == pnum)
	    {
		acdLog ("found %S [%d] '%S'\n",
			pa->Name, pa->PNum, pa->ValStr);
		if (pa->Used & USED_GET)
		    ajWarn("Value for '-%S' used more than once", pa->Token);
		pa->Used |= USED_GET;
		return pa->Value;
	    }
	    else if (!pnum) /* matches any if no number, so count them */
	    {
		ifound++;
		ret = pa;
		acdAmbigApp (&ambigList, pa->Token);
	    }
	}
    }

    if (ifound > 1)
    {
	ajWarn ("Ambiguous qualifier '-%s' (%S)", token, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }
    if (ifound == 1)
    {
	acdLog ("found %S [%d] '%S'\n",
		ret->Name, ret->PNum, ret->ValStr);
	ret->Used |= USED_GET;
	return ret->Value;
    }

    /* program source error */
    ajDie ("Qualifier '-%s' not found\n", token);

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

static void acdHelp (void)
{
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
    
    acdLog("acdHelp %B\n", acdDoHelp);
    
    if (!acdDoHelp) return;
    
    if (acdDoTable)
    {
	reqlist = ajListNew();
	optlist = ajListNew();
	advlist = ajListNew();
	genlist = ajListNew();
	if (acdVerbose)
	    asslist = ajListNew();
      ajUser("<table border cellspacing=0 cellpadding=3 bgcolor=\"#ccccff\">");
	/* was #f5f5ff */
    }
    
    acdLog ("++ acdHelp\n");
    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	hlpFlag = ' ';
	acdLog ("++ Name %S Level %d Assoc %B AssocQuals %x\n",
		pa->Name, pa->Level, pa->Assoc, pa->AssocQuals);
	helpType = HELP_ADV;
	if (pa->Level == ACD_APPL)
	    helpType = HELP_APP;
	else
	{
	    if (!acdIsQtype(pa)) continue;
	}
	
	def = pa->DefStr;
	
	if (def && ajStrLen(def[DEF_OPTIONAL]))
	{
	    if (acdHelpVarResolve(&helpStr, def[DEF_OPTIONAL]))
	    {
		if (!ajStrToBool(helpStr, &tmpBool))
		    acdErrorAcd (pa, "Bad optional flag %S\n",
				 def[DEF_OPTIONAL]);
	    }
	    else
	    {
		tmpBool = ajTrue;
		hlpFlag = '*';
		flagOpt = ajTrue;
	    }
	    if (tmpBool) helpType = HELP_OPT;
	}
	
	if (def && ajStrLen(def[DEF_REQUIRED]))
	{
	    if (acdHelpVarResolve(&helpStr, def[DEF_REQUIRED]))
	    {
		if (!ajStrToBool( helpStr, &tmpBool))
		    acdErrorAcd (pa, "Bad required flag %S\n",
				 def[DEF_REQUIRED]);
	    }
	    else
	    {
		tmpBool = ajTrue;
		hlpFlag = '*';
		flagReq = ajTrue;
	    }
	    if (tmpBool) helpType = HELP_REQ;
	}
	
	if (pa->Assoc) helpType = HELP_ASS;

	acdLog ("++ helpType %d\n", helpType);
	
	switch (helpType)
	{
	case HELP_APP:			/* application, do nothing */
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
	case HELP_ASS:	   /* associated - process after the master */
	    break;
	case HELP_GEN:	      /* associated - process after the app */
	    break;
	default:
	    acdErrorAcd (pa, "unknown qualifier type %d in acdHelp", helpType);
	}
	
	if (pa->AssocQuals)
	{
	    if (helpType == HELP_APP)
	    {
		if (acdVerbose)
		{
		    acdHelpAssoc (pa, &helpGen, NULL);
		    acdHelpAssocTable  (pa, genlist, hlpFlag);
		}
		else
		{
		    acdHelpAssoc (pa, &helpGen, "help");
		    acdHelpAssocTable  (pa, genlist, hlpFlag);
		}
	    }
	    else
	    {
		if (acdVerbose)
		{
		    acdHelpAssoc (pa, &helpAss, NULL);
		    acdHelpAssocTable  (pa, asslist, hlpFlag);
		}
	    }
	}
    }

    if (flagReq)
	acdHelpShow (helpReq,
		     "Mandatory qualifiers (* if not always prompted)");
    else
	acdHelpShow (helpReq, "Mandatory qualifiers");
    acdHelpTableShow (reqlist, "Mandatory qualifiers");
    if (flagOpt)
	acdHelpShow (helpOpt,
		     "Optional qualifiers (* if not always prompted)");
    else
	acdHelpShow (helpOpt, "Optional qualifiers");
    acdHelpTableShow (optlist, "Optional qualifiers");
    acdHelpShow (helpAdv, "Advanced qualifiers");
    acdHelpTableShow (advlist, "Advanced qualifiers");
    if (acdVerbose) acdHelpShow
	(helpAss, "Associated qualifiers");
    acdHelpShow (helpGen, "General qualifiers");
    if (acdVerbose && acdDoTable)
	acdHelpTableShow (asslist, "Associated qualifiers");
    if (acdVerbose && acdDoTable)
	acdHelpTableShow (genlist, "General qualifiers");
    
    if (acdDoTable)
    {
	ajUser ("</table>");
    }
    
    ajExit();
}

/* @funcstatic acdHelpAssoc ***************************************************
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

static void acdHelpAssoc (AcdPAcd thys, AjPStr *str, char* name)
{
    static AjPStr line = NULL;
    static AjPStr qname = NULL;
    static AjPStr qtype = NULL;
    static AjPStr text = NULL;
    AcdPQual quals;
    ajint i;

    acdLog ("++ acdHelpAssoc %S\n", thys->Name);

    if (thys->Level == ACD_APPL)
    {
	quals = acdQualAppl;
    }
    else
    {
	(void) ajFmtPrintS (&line, "  \"-%S\" related qualifiers\n",
			    thys->Name);
	(void) ajStrApp (str, line);
	quals = acdType[thys->Type].Quals;
    }

    acdLog ("++ type %d quals %x\n", thys->Type, quals);

    if (quals)
    {
	for (i=0; quals[i].Name; i++)
	{
	    acdLog ("++ quals[%d].Name %s\n", i, quals[i].Name);
	    if (name && strcmp(name, quals[i].Name)) continue;
	    if (thys->PNum)
		(void) ajFmtPrintS (&qname, "-%s%d",
				    quals[i].Name, thys->PNum);
	    else
		(void) ajFmtPrintS (&qname, "-%s", quals[i].Name);
	    (void) ajStrAssC (&qtype, quals[i].Type);
	    (void) ajFmtPrintS (&line, "   %-20S %-10S ",
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

static void acdHelpAppend (AcdPAcd thys, AjPStr *str, char flag)
{
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
    
    if (ajStrMatchCC("list", acdType[thys->Type].Name))
    {
	(void) ajStrAssC (&type, "menu");
    }
    else
    {
	(void) ajStrAssC (&type, acdType[thys->Type].Name);
    }
    
    if (thys->DefStr)
	defstr = thys->OrigStr;
    else
	defstr = nullstr;
    
    (void) ajStrAssC (&nostr, "");
    if (acdIsQtype(thys) && ajStrMatchCC("boolean", acdType[thys->Type].Name))
    {
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

/* @funcstatic acdHelpValidSeq ************************************************
**
** Generates valid description for an input sequence type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidSeq (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidSeqout (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidOut (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidIn (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidData (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidInt (AcdPAcd thys, AjPStr* str)
{
    ajint imin;
    ajint imax;
    static AjPStr tmpstr = NULL;
    
    acdAttrValueStr (thys, "minimum", "$", &tmpstr);
    if (!ajStrToInt(tmpstr, &imin))
	imin = INT_MIN;
    
    acdAttrValueStr (thys, "maximum", "$", &tmpstr);
    if (!ajStrToInt(tmpstr, &imax))
	imax = INT_MAX;
    
    
    if (imax != INT_MAX)
    {
	if (imin != INT_MIN)
	    ajFmtPrintS (str, "Integer from %d to %d", imin, imax);
	else
	    ajFmtPrintS (str, "Integer up to %d", imax);
    }
    else
    {
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

static void acdHelpValidFloat (AcdPAcd thys, AjPStr* str)
{

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

    if (fmax != FLT_MAX)
    {
	if (fmin != -FLT_MAX)
	    ajFmtPrintS (str, "Number from %.*f to %.*f",
			 iprec, fmin, iprec, fmax);
	else
	    ajFmtPrintS (str, "Number up to %.*f", iprec, fmax);
    }
    else
    {
	if (fmin != -FLT_MAX)
	    ajFmtPrintS (str, "Number %.*f or more", iprec, fmin);
	else
	    ajFmtPrintS (str, "Any numeric value");
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

static void acdHelpValidCodon (AcdPAcd thys, AjPStr* str)
{
    return;
}

/* @funcstatic acdHelpValidDirlist ********************************************
**
** Generates valid description for a dirlist type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidDirlist (AcdPAcd thys, AjPStr* str)
{
    return;
}

/* @funcstatic acdHelpValidFilelist *******************************************
**
** Generates valid description for a filelist type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidFilelist (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidMatrix (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidFeatout (AcdPAcd thys, AjPStr* str)
{
    return;
}

/* @funcstatic acdHelpValidCpdb ***********************************************
**
** Generates valid description for a clean pdb type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidCpdb (AcdPAcd thys, AjPStr* str)
{
    return;
}

/* @funcstatic acdHelpValidScop ***********************************************
**
** Generates valid description for a scop type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidScop (AcdPAcd thys, AjPStr* str)
{
    return;
}

/* @funcstatic acdHelpValidRange **********************************************
**
** Generates valid description for a sequence range.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValidRange (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpValidGraph (AcdPAcd thys, AjPStr* str)
{
    AjPList list;
    AjPStr name = NULL;
    ajint i = 0;
    list = ajListstrNew();

    call ("ajGraphListDevices", list);

    ajFmtPrintS (str, "EMBOSS has a list of known devices, including ");

    while (ajListstrPop (list, &name))
    {
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

static void acdHelpValidString (AcdPAcd thys, AjPStr* str)
{

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

    if (maxlen > 0)
    {
	if (minlen > 0)
	    ajFmtPrintS (str, "A string from %d to %d characters",
			 minlen, maxlen);
	else
	    ajFmtPrintS (str, "A string up to %d characters", maxlen);
    }
    else
    {
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

static void acdHelpValidRegexp (AcdPAcd thys, AjPStr* str)
{
    ajint minlen;
    ajint maxlen;
    static AjPStr tmpstr = NULL;

    acdAttrValueStr (thys, "min", "0", &tmpstr);
    if (!ajStrToInt(tmpstr, &minlen))
	minlen = 0;
    acdAttrValueStr (thys, "max", "0", &tmpstr);
    if (!ajStrToInt(tmpstr, &maxlen))
	maxlen = 0;

    if (maxlen > 0)
    {
	if (minlen > 0)
	    ajFmtPrintS (str,
			 "A regular epression pattern from %d to "
			 "%d characters",
			 minlen, maxlen);
	else
	    ajFmtPrintS (str,
			 "A regular epression pattern up to %d characters",
			 maxlen);
    }
    else
    {
	if (minlen > 0)
	    ajFmtPrintS (str,
			 "A regular epression pattern of at least "
			 "%d characters",
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

static void acdHelpValidList (AcdPAcd thys, AjPStr* str)
{
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
    while (ajStrDelim (&line, &handle, NULL))
    {
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

static void acdHelpValidSelect (AcdPAcd thys, AjPStr* str)
{
    AjPStr delim = NULL;
    AjPStr value = NULL;
    AjPStrTok handle;
    static AjPStr desc = NULL;
    static char* white = " \t\n\r";

    acdAttrValueStr (thys, "delimiter", ";", &delim);

    acdAttrValueStr (thys, "value", "", &value);

    handle = ajStrTokenInit (value, ajStrStr(delim));

    while (ajStrDelim (&desc, &handle, NULL))
    {
	(void) ajStrTrimC (&desc, white);
	ajFmtPrintAppS (str, "%S<br>", desc);
    }

    (void) ajStrTokenClear (&handle);
    return;
}

/* @funcstatic acdHelpValid ***************************************************
**
** Generates help text for an ACD object using the help, info, prompt
** and code settings.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpValid (AcdPAcd thys, AjPStr* str)
{
    ajint i;

    if (ajStrLen(*str)) return;


    if (acdAttrValueStr(thys, "valid", "", str))
	return;

    /* special processing for sequences, outseq, outfile */

    for (i=0; acdValue[i].Name; i++)
    {
	if (ajStrMatchCC (acdType[thys->Type].Name, acdValue[i].Name))
	{
	    /* Calling funclist acdValue() */
	    if (acdValue[i].Valid) acdValue[i].Valid(thys, str);
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

static void acdHelpExpectSeq (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectSeqout (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectOut (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectInt (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectFloat (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectIn (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectData (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectCodon (AcdPAcd thys, AjPStr* str)
{
    (void) acdAttrResolve (thys, "name", str);
    if (ajStrLen(*str)) return;

    ajStrAssC (str, DEFCODON);

    return;
}


/* @funcstatic acdHelpExpectDirlist *******************************************
**
** Generates expected value description for a dirlist type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectDirlist (AcdPAcd thys, AjPStr* str)
{
    (void) acdAttrResolve (thys, "name", str);
    if (ajStrLen(*str)) return;

    ajStrAssC (str, DEFDLIST);

    return;
}

/* @funcstatic acdHelpExpectFilelist ******************************************
**
** Generates expected value description for a filelist type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectFilelist (AcdPAcd thys, AjPStr* str)
{
    ajStrAssC (str, "<i>comma-separated file list</i>");

    return;
}

/* @funcstatic acdHelpExpectCpdb **********************************************
**
** Generates expected value description for a clean pdb file type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectCpdb (AcdPAcd thys, AjPStr* str)
{
    (void) acdAttrResolve (thys, "name", str);
    if (ajStrLen(*str)) return;

    ajStrAssC (str, DEFCPDB);

    return;
}

/* @funcstatic acdHelpExpectScop **********************************************
**
** Generates expected value description for a scop type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectScop (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectMatrix (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectFeatout (AcdPAcd thys, AjPStr* str)
{
    ajStrAssC (str, "<i>unknown.gff</i>");

    return;
}

/* @funcstatic acdHelpExpectRange *********************************************
**
** Generates expected value description for a sequence range type.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpectRange (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectGraph (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectRegexp (AcdPAcd thys, AjPStr* str)
{
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

static void acdHelpExpectString (AcdPAcd thys, AjPStr* str)
{
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
** Generates expected value text for an ACD object code settings.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpExpect (AcdPAcd thys, AjPStr* str)
{
    ajint i;
    if (ajStrLen(*str)) return;
    
    if (!thys->AssocQuals)
    {
	if (acdAttrValueStr(thys, "expected", "", str))
	    return;
    }
    
    if (acdAttrValueStr(thys, "default", "", str))
	return;
    
    /* special processing for sequences, outseq, outfile */
    
    for (i=0; acdValue[i].Name; i++)
    {
	if (ajStrMatchCC (acdType[thys->Type].Name, acdValue[i].Name))
	{
	    /* Calling funclist acdValue() */
	    if (acdValue[i].Expect) acdValue[i].Expect(thys, str);
	    break;
	}
    }
    if (ajStrLen(*str)) return;
    
    ajStrAssS (str, thys->DefStr[DEF_DEFAULT]);
    
    if (ajStrLen(*str)) return;
    
    (void) ajStrAssC (str, "&nbsp;");
    
    return;
}

/* @funcstatic acdHelpText ****************************************************
**
** Generates help text for an ACD object using the help, info, prompt
** and code settings.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] str [AjPStr*] Help text (if any) generated
** @return [void]
** @@
******************************************************************************/

static void acdHelpText (AcdPAcd thys, AjPStr* str)
{
    AjPStr prompt;
    AjPStr info;
    AjPStr code;
    AjPStr help;
    static AjPStr msg = NULL;
    
    if (thys->DefStr)
    {
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
	
	else
	{
	    if (!acdHelpCodeDef (thys, &msg))
	    {
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

/* @funcstatic acdHelpShow ****************************************************
**
** Prints the qualifier category and the help for any
** qualifiers in that category (or "(none)" if there are none).
**
** @param [r] str [AjPStr] Help text (if any)
** @param [r] title [char*] Title line for this call
** @return [void]
** @@
******************************************************************************/

static void acdHelpShow (AjPStr str, char* title)
{
    if (acdDoTable) return;

    if (!ajStrLen(str))
    {
	ajUser ("   %s: (none)", title);
	return;
    }

    ajUser ("   %s:", title);
    ajUser ("%S", str);

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

static void acdHelpTableShow (AjPList tablist, char* title)
{
    AcdPTableItem item;
    AjIList iter = NULL;

    if (!acdDoTable) return;

    ajUser ("<tr bgcolor=\"#FFFFCC\">"); /* was #FFFFD0 */
    ajUser ("<th align=\"left\" colspan=2>%s</th>", title);
    ajUser ("<th align=\"left\">Allowed values</th>");
    ajUser ("<th align=\"left\">Default</th>");
    ajUser ("</tr>\n");

    if (!ajListLength(tablist))
    {
	ajUser ("<tr>");
	ajUser ("<td colspan=4>(none)</td>");
	ajUser ("</tr>\n");
    }
    else
    {
	iter=ajListIter(tablist);
	while ((item = ajListIterNext(iter)))
	{
	    ajUser ("<tr>");
	    ajUser ("<td>%S</td>", item->Qual);
	    ajUser ("<td>%S</td>", item->Help);
	    ajUser ("<td>%S</td>", item->Valid);
	    ajUser ("<td>%S</td>", item->Expect);
	    ajUser ("</tr>\n");
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

static void acdHelpAssocTable (AcdPAcd thys, AjPList tablist, char flag)
{

    AcdPTableItem item;

    static AjPStr line = NULL;
    static AjPStr qname = NULL;
    static AjPStr qtype = NULL;
    AcdPQual quals;
    ajint i;
    AcdPAcd pa;

    if (!acdDoTable) return;

    acdLog ("++ acdHelpAssoc %S\n", thys->Name);

    if (thys->Level == ACD_APPL)
    {
	quals = acdQualAppl;
    }
    else
    {
	(void) ajFmtPrintS (&line, "  \"-%S\" related qualifiers\n",
			    thys->Name);
	/*(void) ajStrApp (str, line);*/
	quals = acdType[thys->Type].Quals;
    }

    acdLog ("++ type %d quals %x\n", thys->Type, quals);

    i=0;
    for (pa=thys->AssocQuals; pa && pa->Assoc; pa=pa->Next)
    {
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

/* @funcstatic acdHelpTable ***************************************************
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

static void acdHelpTable (AcdPAcd thys, AjPList tablist, char flag)
{
    
    AcdPTableItem item;
    
    static AjPStr name = NULL;
    static AjPStr nostr = NULL;
    static AjPStr nullstr = NULL;
    static AjPStr type = NULL;
    AjBool boolval;
    
    AjPStr defstr;
    
    if (!acdDoTable) return;
    
    AJNEW0 (item);
    
    if (!nullstr) nullstr = ajStrNew();
    
    if (thys->DefStr)
	defstr = thys->OrigStr;
    else
	defstr = nullstr;
    
    (void) ajStrAssC (&nostr, "");
    if (acdIsQtype(thys) && ajStrMatchCC("boolean", acdType[thys->Type].Name))
    {
	if (ajStrToBool(defstr, &boolval))
	{
	    if (boolval)
		(void) ajStrAssC (&nostr, "[no]");
	    ajFmtPrintS (&item->Expect, "%B", boolval);
	}
	else
	{
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

static void acdListReport (char* title)
{
    
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
    
    for (pa=acdList; pa; pa=pa->Next)
    {
	acdLog ("ACD %d\n", i);
	if (pa->PNum)
	{
	    acdLog ("       Name: '%S[%d]'\n", pa->Name, pa->PNum);
	    acdLog ("      Token: '%S[%d]'\n", pa->Token, pa->PNum);
	}
	else
	{
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
	if (pa->DefStr)
	{
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

static void acdListAttr (AcdPAttr attr, AjPStr* valstr, ajint nattr)
{
    ajint i;

    if (!valstr)
	return;

    for (i=0; i < nattr; i++)
    {
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

static AjBool acdSet (AcdPAcd thys, AjPStr* attrib, AjPStr value)
{
    ajint iattr = -1;
    ajint idef = -1;
    
    AcdPAttr attr = acdType[thys->Type].Attr;
    AjPStr* attrstr = thys->AttrStr;
    AcdPAcd aqual;
    
    acdLog("acdSet attr '%S' val '%S' type '%S'\n",
	   thys->Name, *attrib, value);
    
    /* recursion with associated qualifiers */
    aqual = NULL;
    if (thys->AssocQuals)
	aqual = acdFindAssoc(thys, *attrib, NULL);
    
    iattr = acdFindAttr (attr, *attrib);
    
    if (thys->DefStr)	       /* try again with default attributes */
    {
	idef = acdFindAttr (acdAttrDef, *attrib);
    }
    
    if (iattr >= 0 && idef >= 0)	/* should never happen */
	acdErrorAcd (thys, "Duplicate type and default attribute '%S'",
		     *attrib);
    
    if (aqual)
    {
	if (iattr >= 0)
	    acdErrorAcd (thys,		/* no known case */
			 "'%S' matches attribute '%s' and "
			 "associated qualifier '%S'",
			 *attrib, attr[iattr].Name, aqual->Name);
	if (idef >= 0)
	    acdErrorAcd (thys,		/* ambigdefattr.acd */
			 "'%S' matches attribute '%s' and "
			 "associated qualifier '%S'",
			 *attrib, acdAttrDef[idef].Name, aqual->Name);
    }
    
    if (iattr >= 0)
    {
	(void) ajStrAssC (attrib, attr[iattr].Name);
	(void) ajStrAssS (&attrstr[iattr], value);
	return ajTrue;
    }
    
    if (idef >= 0)
    {
	(void) ajStrAssC (attrib, acdAttrDef[idef].Name);
	(void) ajStrAssS (&thys->DefStr[idef], value);
	return ajTrue;
    }
    
    if (aqual)
	return acdDef(aqual, value);

    /* test: wrongattr.acd */
    acdErrorAcd (thys, "Attribute '%S' unknown\n", *attrib );
    return ajFalse;
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

static AjBool acdSetKey (AcdPAcd thys, AjPStr* attrib, AjPStr value)
{
    ajint iattr = -1;

    AcdPAttr attr = acdKeywords[thys->Type].Attr;
    AjPStr* attrstr = thys->AttrStr;
    AcdPAcd aqual;

    /* recursion with associated qualifiers - e.g.for application */
    aqual = NULL;
    if (thys->AssocQuals)
	aqual = acdFindAssoc(thys, *attrib, NULL);

    iattr = acdFindAttr (attr, *attrib);

    if (aqual)
    {
	if (iattr >= 0)
	    acdErrorAcd (thys,		/* no known case */
			 "'%S' matches attribute '%s' and qualifier '%S'",
			 *attrib, attr[iattr].Name, aqual->Name);
    }

    if (iattr >= 0)
    {
	(void) ajStrAssC (attrib, attr[iattr].Name);
	(void) ajStrAssS (&attrstr[iattr], value);
	return ajTrue;
    }
  
    if (aqual)
	return acdDef (aqual, value);
  
    /* test: wrongattr.acd */
    acdErrorAcd (thys, "Attribute '%S' unknown\n", *attrib );
    return ajFalse;
}

/* @funcstatic acdDef *********************************************************
**
** Sets the default value for an ACD item, and flags in thys as Defined.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] value [AjPStr] Default value
** @return [AjBool] ajTrue always.
** @see acdSetDef
** @@
******************************************************************************/

static AjBool acdDef (AcdPAcd thys, AjPStr value)
{

    AjPStr* attrstr = thys->DefStr;

    acdLog ("acdDef %S '%S' %x\n", thys->Name, value, attrstr);

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

static AjBool acdSetDef (AcdPAcd thys, AjPStr value)
{
    AjPStr* attrstr = thys->DefStr;

    acdLog ("acdSetDef %S '%S' %x\n", thys->Name, value, attrstr);

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

static AjBool acdSetDefC (AcdPAcd thys, char* value)
{
    AjPStr* attrstr = thys->DefStr;

    acdLog ("acdSetDefC %S '%s' %x\n", thys->Name, value, attrstr);

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

static AjBool acdSetQualDefBool (AcdPAcd thys, char* name, AjBool value)
{
    AjPStr* attrstr;
    static AjPStr qname = NULL;
    AcdPAcd acd;

    (void) ajStrAssC (&qname, name);
    acd = acdFindQualAssoc (thys, qname, NULL, 0);
    if (!acd) return ajFalse;

    attrstr = acd->DefStr;

    acdLog ("acdSetQualDefBool %S [%d] '%s' %B\n",
	    thys->Name, thys->PNum, name, value);

    if (!thys->DefStr)
	return ajFalse;

    (void) ajFmtPrintS (&attrstr[DEF_DEFAULT], "%b", value);

    return ajTrue;
}

/* @funcstatic acdSetQualDefInt ***********************************************
**
** Sets the default value for an ACD item.
**
** @param [u] thys [AcdPAcd] ACD item
** @param [r] name [char *] Qualifier name
** @param [r] value [ajint] Default value
** @return [AjBool] ajTrue always.
** @@
******************************************************************************/

static AjBool acdSetQualDefInt (AcdPAcd thys, char* name, ajint value)
{
    AjPStr* attrstr;
    static AjPStr qname = NULL;
    AcdPAcd acd;

    (void) ajStrAssC (&qname, name);
    acd = acdFindQualAssoc (thys, qname, NULL, 0);
    if (!acd) return ajFalse;

    attrstr = acd->DefStr;

    acdLog ("acdSetQualDefInt %S [%d] '%s' %S [%d] %d\n",
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

static AjBool acdSetVarDef (AcdPAcd thys, AjPStr value)
{
    acdLog ("acdSetVarDef %S '%S' %x\n", thys->Name, value, thys->ValStr);

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

static ajint acdFindAttr (AcdPAttr attr, AjPStr attrib)
{
    static ajint i;
    static ajint j;
    ajint ifound=0;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (i=0; attr[i].Name; i++)
    {
	if (ajStrMatchC (attrib, attr[i].Name))
	    return i;
	if (ajStrPrefixCO (attr[i].Name, attrib))
	{
	    ifound++;
	    j = i;
	    acdAmbigAppC (&ambigList, attr[i].Name);
	}
    }
    if (ifound == 1)
	return j;
    if (ifound > 1)
    {
	ajWarn ("Ambiguous attribute %S (%S)", attrib, ambigList);
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

static ajint acdFindAttrC (AcdPAttr attr, char* attrib)
{
    static ajint i;
    static ajint j;
    ajint k = strlen(attrib);
    ajint ifound=0;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (i=0; attr[i].Name; i++)
    {
	if (!strncmp (attr[i].Name, attrib, k))
	{
	    if (!strcmp (attr[i].Name, attrib))
		return i;
	    ifound++;
	    j = i;
	    acdAmbigAppC (&ambigList, attr[i].Name);
	}
    }
    if (ifound == 1)
	return j;
    if (ifound > 1)
    {
	ajWarn ("ambiguous attribute %s (%S)", attrib, ambigList);
	(void) ajStrDelReuse(&ambigList);
    }

    return -1;
}

/* @funcstatic acdProcess *****************************************************
**
** Steps through all the ACD items, filling in missing information.
** Parameters are defined in the default attributes. The parameter
** number is generated here in the order they are found.
**
** Associated qualifiers (if any) also get a copy of the parameter number.
**
** @return [void]
** @@
******************************************************************************/

static void acdProcess (void)
{
    AcdPAcd pa;
    AcdPAcd qa = NULL;
    static AjPStr reqstr = NULL;
    static AjPStr yesstr = NULL;
    AjBool isreq;
    AjBool isparam;

    if (!reqstr)
    {
	(void) ajStrAssC(&reqstr, "required");
	(void) ajStrAssC(&yesstr, "Y");
    }

    for (acdProcCurr=acdList; acdProcCurr; acdProcCurr=acdProcCurr->Next)
    {
	pa = acdProcCurr;
	if (pa->DefStr)
	    (void) ajStrAssS (&pa->OrigStr, pa->DefStr[DEF_DEFAULT]);

	acdLog ("acdProcess '%S' defstr '%x' test parameter\n",
		pa->Name, pa->DefStr);
	if (pa->DefStr && acdAttrToBoolTest(pa, "parameter",
					    ajFalse, &isparam))
	{
	    if (isparam)
	    {
		acdNParam++;
		pa->PNum = acdNParam;
		pa->Level = ACD_PARAM;
		acdLog ("acdProcess '%S' required\n", pa->Name);
		if (!acdVarTest(acdAttrValue(pa, "required")))
		{		       /* no unresolvable variables */
		    if (!acdAttrToBool (pa, "required", ajFalse, &isreq))
			(void) acdSet(pa, &reqstr, yesstr);
		}
		qa = pa->AssocQuals;
		if (qa)
		{
		    while (qa->Assoc)
		    {
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

static void acdSetAll (void)
{
    AcdPAcd pa;

    ajint i = 0;


    for (acdSetCurr=acdList; acdSetCurr; acdSetCurr = acdSetCurr->Next)
    {
	pa = acdSetCurr;
	if (acdIsStype(pa)) continue;
	if (acdIsQtype(pa))
	{
	    /* Calling funclist acdType() */
	    acdType[pa->Type].TypeSet (pa);
	}
	else
	{
	    /* Calling funclist acdKeywords() */
	    acdKeywords[pa->Type].KeySet (pa);
	}
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
			     AjBool defval, AjBool *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc (thys, qual, valstr);
    acdLog ("acdQualToBool item: %S qual: %s defval: %B str: '%S', ret: %B\n",
	    thys->Name, qual, defval, *valstr, ret);
    if (ret)
    {
	(void) acdVarResolve (valstr);
	acdLog ("resolved to: '%S'\n", *valstr);

	if (ajStrLen(*valstr))
	{
	    if (!ajStrToBool(*valstr, result))
	    {
		acdErrorAcd (thys,
			     "Bad associated qualifier "
			     "boolean value -%s = %S\n",
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
			      float *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc (thys, qual, valstr);
    acdLog ("acdQualToFloat item: %S qual: %s defval: %.3f "
	    "str: '%S' ret: %B\n",
	    thys->Name, qual, defval, *valstr, ret);
    if (ret)
    {
	(void) acdVarResolve (valstr);
	acdLog ("resolved to: '%S'\n", *valstr);


	if (ajStrLen(*valstr))
	{
	    if (!ajStrToFloat(*valstr, result))
	    {
		acdErrorAcd (thys,
			     "%S: Bad associated qualifier "
			     "float value -%s = %S\n",
			     qual, *valstr) ;
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
			    ajint defval, ajint *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc (thys, qual, valstr);
    acdLog ("acdQualToInt item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	    thys->Name, qual, defval, *valstr, ret);
    if (ret)
    {
	(void) acdVarResolve (valstr);
	acdLog ("resolved to: '%S'\n", *valstr);

	if (ajStrLen(*valstr))
	{
	    if (ajStrMatchC(*valstr, "default"))
		(void) ajStrAssC(valstr, "0");
	    if (!ajStrToInt(*valstr, result))
	    {
		acdErrorAcd (thys,
			     "%S: Bad associated qualifier "
			     "integer value -%s = %S\n",
			     qual, *valstr);
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
** string "start".
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
				 ajint defval, ajint *result, AjPStr* valstr)
{
    AjBool ret;

    ret = acdGetValueAssoc (thys, qual, valstr);
    acdLog ("acdQualToSeqpos item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	    thys->Name, qual, defval, *valstr, ret);
    if (ret)
    {
	(void) acdVarResolve (valstr);
	acdLog ("resolved to: '%S'\n", *valstr);

	if (ajStrLen(*valstr))
	{
	    if (!ajStrMatchCaseC(*valstr, "default"))
	    {
		if (!ajStrToInt(*valstr, result))
		{
		    acdErrorAcd (thys,
				 "Bad associated qualifier "
				 "integer value -%s = %S\n",
				 qual, *valstr);
		}
	    }
	    acdLog ("return value %B '%S'\n", ajTrue, *valstr);
	    return ajTrue;
	}
    }

    *result = defval;
    if (!defval)
    {
	(void) ajStrAssC (valstr, "start");
    }
    else
    {
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
			       ajint defval, ajint *result, AjPStr* valstr)
{
    
    
    AjBool ret;
    
    ret = acdGetValueAssoc (thys, qual, valstr);
    acdLog ("acdQualToSeqpos item: %S qual: %s defval: %d str: '%S' ret: %B\n",
	    thys->Name, qual, defval, *valstr, ret);
    if (ret)
    {
	(void) acdVarResolve (valstr);
	acdLog ("resolved to: '%S'\n", *valstr);

	if (ajStrLen(*valstr))
	{
	    if (!ajStrMatchCaseC(*valstr, "default"))
	    {
		if (!ajStrToInt(*valstr, result))
		{
		    acdErrorAcd (thys,
				 "Bad associated qualifier "
				 "integer value -%s = %S\n",
				 qual, *valstr);
		}
	    }
	    acdLog ("return value %B '%S'\n", ajTrue, *valstr);
	    return ajTrue;
	}
    }
    
    *result = defval;
    if (!defval)
    {
	(void) ajStrAssC (valstr, "end");
    }
    else
    {
	(void) ajStrFromInt(valstr, defval);
    }
    
    acdLog ("return default %B '%S'\n", ajFalse, *valstr);
    return ajFalse;
}

/* @funcstatic acdAttrToBoolTest **********************************************
**
** Resolves and tests an attribute string. If it has a boolean value, returns
** true and sets the value. Otherwise returns false and the default value.
**
** Has to take care to test for variables, as their values are not yet
** set when this is called.
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attr [char*] Attribute name
** @param [r] defval [AjBool] Default value
** @param [w] result [AjBool*] Resulting value.
** @return [AjBool] ajTrue if a value was defined, ajFalse if the
**         default value was used.
** @@
******************************************************************************/

static AjBool acdAttrToBoolTest (AcdPAcd thys,
				 char *attr, AjBool defval, AjBool *result)
{

    if (acdVarTest(acdAttrValue (thys, attr))) /* test calcparam.acd */
	acdErrorAcd (thys,
		     "'%s' attribute cannot use a variable. "
		     "It is used to define "
		     "the command line before values have been set",
		     attr);
 
    return acdAttrToBool (thys, attr, defval, result);
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
			     char *attr, AjBool defval, AjBool *result)
{
    ajint i;
    static AjPStr str = NULL;
    
    (void) acdAttrResolve (thys, attr, &str);
    
    if (ajStrLen(str))
    {
	if (ajStrToBool(str, result))
	{
	    (void) ajStrDelReuse(&str);
	    return ajTrue;
	}
	if (ajStrToInt(str, &i))
	{
	    if (i) *result = ajTrue;
	    else *result = ajFalse;
	    (void) ajStrDelReuse(&str);
	    return ajTrue;
	}
	else
	{
	    acdErrorAcd (thys, "%S: Bad attribute boolean value %s = %S\n",
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
			      char *attr, float defval, float *result)
{

    static AjPStr str= NULL;

    (void) acdAttrResolve (thys, attr, &str);

    if (ajStrLen(str))
    {
	if (ajStrToFloat(str, result))
	{
	    (void) ajStrDelReuse(&str);
	    return ajTrue;
	}
	else
	{
	    acdErrorAcd (thys,
			 "Bad attribute float value %s = %S\n",
			 attr, str);
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
			    char *attr, ajint defval, ajint *result)
{
    static AjPStr str = NULL;

    (void) acdAttrResolve (thys, attr, &str);

    if (ajStrLen(str))
    {
	if (ajStrToInt(str, result))
	{
	    (void) ajStrDelReuse(&str);
	    return ajTrue;
	}
	else
	{
	    acdErrorAcd (thys, "Bad attribute integer value %s = %S\n",
			 attr, str);
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
			    char *attr, char* defval, AjPStr *result)
{
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

static AjBool acdAttrResolve (AcdPAcd thys, char *attr, AjPStr *result)
{
    (void) ajStrAssS(result, acdAttrValue (thys, attr));
    (void) acdVarResolve (result);

    if (ajStrLen(*result))
	return ajTrue;

    return ajFalse;
}

/* @funcstatic acdVarTest *****************************************************
**
** tests for any variable (but not function) references in a string.
**
** Used to check whether variables might be used before we have set
** their values..
**
** @param [uP] var [AjPStr] String value
** @return [AjBool] ajTrue if a variable was found
** @@
******************************************************************************/

static AjBool acdVarTest (AjPStr var)
{
    static AjPRegexp varexp = NULL;

    if (!varexp) varexp = ajRegCompC("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");

    if (ajRegExec (varexp, var))
	return ajTrue;
  
    return ajFalse;
}

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

static AjBool acdVarResolve (AjPStr* var)
{
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
    if (!var)
    {
	(void) ajStrAssC(var, "");
	return ajTrue;
    }
    
    if (!ajStrLen(*var))
	return ajTrue;
    
    (void) ajStrAssS(&savein, *var);
    acdLog("acdVarResolve '%S'\n", savein);
    
    while (ajRegExec (varexp, *var))
    {
	ivar++;
	ajRegSubI(varexp, 2, &token);	/* variable name */
	(void) acdVarSplit (token, &varname, &attrname);
	if (!acdGetAttr (&result, varname, attrname))
	    ajStrAssC(&result, "");

	ajRegSubI(varexp, 1, &newvar);
	(void) ajStrApp (&newvar, result);
	if (ajRegPost(varexp, &restvar)) /* any more? */
	    (void) ajStrApp (&newvar, restvar);
	(void) ajStrAssS (var, newvar);
	acdLog("... name %S resolved to '%S'\n", varname, newvar);
    }
    
    /* now resolve any function */
    
    while (ajRegExec(funexp, *var))
    {
	ifun++;
	ajRegSubI(funexp, 2, &token);	/* function statement */
	(void) acdFunResolve (&result, token);
	ajRegSubI(funexp, 1, &newvar);
	(void) ajStrApp (&newvar, result);
	if (ajRegPost(funexp, &restvar)) /* any more? */
	    (void) ajStrApp (&newvar, restvar);
	(void) ajStrAssS (var, newvar);
	acdLog("... function %S resolved to '%S'\n", token, newvar);
    }
    
    if (ivar > 1)
	acdLog("Recursive variables in '%S'\n", savein);
    
    if (ifun > 1)
	acdLog("Recursive expressions in '%S'\n", savein);
    
    (void) ajStrDelReuse(&savein);
    (void) ajStrDelReuse(&token);
    (void) ajStrDelReuse(&result);
    (void) ajStrDelReuse(&attrname);
    (void) ajStrDelReuse(&varname);
    (void) ajStrDelReuse(&newvar);
    (void) ajStrDelReuse(&restvar);
    
    return ajTrue;
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

static AjBool acdHelpVarResolve (AjPStr* str, AjPStr var)
{
    static AjPRegexp varexp = NULL;
    static AjPRegexp funexp = NULL;

    if (!varexp) varexp = ajRegCompC("^(.*)\\$\\(([a-zA-Z0-9_.]+)\\)");
    if (!funexp) funexp = ajRegCompC("^(.*)\\@\\(([^()]+)\\)");

    if (!var)
    {
	(void) ajStrAssC(str, "");
	return ajTrue;
    }

    /* reject variable references first to resolve internal parentheses */
    if (ajRegExec (varexp, var))
    {
	(void) ajStrAssC (str, "");
	return ajFalse;
    }

    /* reject any function */
    if (ajRegExec (funexp, var))
    {
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

static AjBool acdFunResolve (AjPStr* result, AjPStr str)
{
    ajint i;

    acdLog ("acdFunResolve '%S'\n", str);

    for (i = 0; explist[i].Name; i++)
    {
	/* ajDebug ("try using '%s'\n", explist[i].Name); */
	/* Calling funclist acdexplist() */

	if (explist[i].Func (result, str))
	{
	    acdLog ("resolved '%S' using '%s'\n", str, explist[i].Name);
	    acdLog ("  result '%S'\n", *result);
	    return ajTrue;
	}
    }

    ajWarn("ACD expression invalid @(%S)\n", str);
    acdLog ("@(%S) *failed**\n", str);

    (void) ajStrAssS(result, str);

    return ajFalse;
}

/* @funcstatic acdExpPlus *****************************************************
**
** Looks for and resolves an expression @( num + num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpPlus (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;

    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;

    if (!iexp)				/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[+][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
	ajRegSubI(iexp, 1, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ia);
	ajRegSubI(iexp, 2, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ib);
	(void) ajFmtPrintS (result, "%d", ia+ib);
	acdLog ("ia: %d + ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if (!dexp)				/* float + float */
	dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[+][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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

/* @funcstatic acdExpMinus ****************************************************
**
** Looks for and resolves an expression @( num - num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpMinus (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;

    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;

    if (!iexp)				/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[-][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
	ajRegSubI(iexp, 1, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ia);
	ajRegSubI(iexp, 2, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ib);
	(void) ajFmtPrintS (result, "%d", ia-ib);
	acdLog ("ia: %d - ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if (!dexp)				/* float + float */
	dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[-][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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

/* @funcstatic acdExpStar *****************************************************
**
** Looks for and resolves an expression @( num * num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpStar (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;

    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;

    if (!iexp)				/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[*][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
	ajRegSubI(iexp, 1, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ia);
	ajRegSubI(iexp, 2, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ib);
	(void) ajFmtPrintS (result, "%d", ia*ib);
	acdLog ("ia: %d * ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if (!dexp)				/* float + float */
	dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[*][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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

/* @funcstatic acdExpDiv ******************************************************
**
** Looks for and resolves an expression @( num / num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpDiv (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;

    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;

    if (!iexp)				/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[/][ \t]*"
			  "([0-9+-]+)[ \t]*$");

    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
	ajRegSubI(iexp, 1, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ia);
	ajRegSubI(iexp, 2, &acdExpTmpstr);
	(void) ajStrToInt(acdExpTmpstr, &ib);
	(void) ajFmtPrintS (result, "%d", ia/ib);
	acdLog ("ia: %d / ib: %d = '%S'\n", ia, ib, *result);

	return ajTrue;
    }

    if (!dexp)				/* float + float */
	dexp = ajRegCompC("^[ \t]*([0-9.+-]+)[ \t]*[/][ \t]*"
			  "([0-9.+-]+)[ \t]*$");

    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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

/* @funcstatic acdExpNot ******************************************************
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

static AjBool acdExpNot (AjPStr* result, AjPStr str)
{
    AjBool ba;
    static AjPRegexp nexp = NULL;

    if (!nexp)				/* ajint + ajint */
       nexp = ajRegCompC("^[ \t]*(!|[Nn][Oo][Tt])[ \t]*([A-Za-z0-9]+)[ \t]*$");

    if (ajRegExec(nexp, str))
    {
	acdLog("nexp matched  '%S'\n", str);
	ajRegSubI(nexp, 2, &acdExpTmpstr);
	if (!ajStrToBool(acdExpTmpstr, &ba))
	{
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

static AjBool acdExpEqual (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;
    static AjPStr tmpstr2 = NULL;
    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;
    static AjPRegexp texp = NULL;
    
    if (!iexp)			/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[=][=][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
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
    
    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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
    
    if (ajRegExec(texp, str))
    {
	acdLog("texp matched  '%S'\n", str);
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

static AjBool acdExpNotEqual (AjPStr* result, AjPStr str)
{
    
    ajint ia, ib;
    double da, db;
    static AjPStr tmpstr2 = NULL;
    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;
    static AjPRegexp texp = NULL;
    
    if (!iexp)			/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[!][=][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
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
    
    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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
    
    if (ajRegExec(texp, str))
    {
	acdLog("texp matched  '%S'\n", str);
	ajRegSubI(texp, 1, &acdExpTmpstr);
	ajRegSubI(texp, 2, &tmpstr2);
	if (!ajStrMatchCase(acdExpTmpstr, tmpstr2))
	    (void) ajFmtPrintS (result, "%b", ajTrue);
	else
	    (void) ajFmtPrintS (result, "%b", ajFalse);
	acdLog ("ta: '%S' != tb: '%S' = '%S'\n",
		acdExpTmpstr, tmpstr2, *result);

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

static AjBool acdExpGreater (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;
    static AjPStr tmpstr2 = NULL;
    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;
    static AjPRegexp texp = NULL;
    
    if (!iexp)			/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[>][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
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
    
    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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
    
    if (ajRegExec(texp, str))
    {
	acdLog("texp matched  '%S'\n", str);
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

static AjBool acdExpLesser (AjPStr* result, AjPStr str)
{
    ajint ia, ib;
    double da, db;
    static AjPStr tmpstr2 = NULL;
    static AjPRegexp iexp = NULL;
    static AjPRegexp dexp = NULL;
    static AjPRegexp texp = NULL;
    
    if (!iexp)			/* ajint + ajint */
	iexp = ajRegCompC("^[ \t]*([0-9+-]+)[ \t]*[<][ \t]*"
			  "([0-9+-]+)[ \t]*$");
    
    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
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
    
    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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
    
    if (ajRegExec(texp, str))
    {
	acdLog("texp matched  '%S'\n", str);
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

/* @funcstatic acdExpOr *******************************************************
**
** Looks for and resolves an expression @( num | num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpOr (AjPStr* result, AjPStr str)
{
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
    
    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
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
    
    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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
    
    if (ajRegExec(texp, str))
    {
	acdLog("texp matched  '%S'\n", str);
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

/* @funcstatic acdExpAnd ******************************************************
**
** Looks for and resolves an expression @( num & num )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpAnd (AjPStr* result, AjPStr str)
{
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
    
    if (ajRegExec(iexp, str))
    {
	acdLog("iexp matched  '%S'\n", str);
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
    if (ajRegExec(dexp, str))
    {
	acdLog("dexp matched  '%S'\n", str);
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
    
    if (ajRegExec(texp, str))
    {
	acdLog("texp matched  '%S'\n", str);
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

/* @funcstatic acdExpCond *****************************************************
**
** Looks for and resolves an expression @( bool ? trueval : falseval )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpCond (AjPStr* result, AjPStr str)
{
    AjBool ba;
    static AjPRegexp condexp = NULL;

    if (!condexp)			/* bool ? iftrue : iffalse */
	condexp = ajRegCompC("^[ \t]*([.A-Za-z0-9+-]*)[ \t]*[?]"
			     "[ \t]*([^: \t]+)[ \t]*[:]"
			     "[ \t]*([^: \t]+)[ \t]*$");

    if (ajRegExec(condexp, str))
    {
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

/* @funcstatic acdExpCase *****************************************************
**
** Looks for and resolves an expression as a switch/case statement
** @( var = casea : vala, caseb: valb else: val )
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpCase (AjPStr* result, AjPStr str)
{
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
    
    if (ajRegExec(caseexp, str))
    {
	ajRegSubI(caseexp, 1, &testvar);
	
	if (!ajRegPost(caseexp, &restvar)) /* any more? */
	    return ajFalse;
	
	(void) ajStrAssC (&elsevar, "");
	todo = ajTrue;
	ifound = 0;
	while (todo && ajRegExec (listexp, restvar))
	{
	    ajRegSubI(listexp, 1, &acdExpTmpstr);
	    if (ajStrMatchC (acdExpTmpstr, "else")) /* default */
		ajRegSubI(listexp, 2, &elsevar);
	    if (ajStrMatch (acdExpTmpstr, testvar)) /* match, take the value */
	    {
		ajRegSubI(listexp, 2, result);
		acdLog ("%S == %S : '%S'\n", testvar, acdExpTmpstr, *result);
		return ajTrue;
	    }
	    if (ajStrPrefix(testvar, acdExpTmpstr))
	    {
		ifound++;
		ajRegSubI(listexp, 2, result);
	    }
	    todo = ajRegPost(listexp, &restvar);
	}
	
	if (ifound)			   /* let ambiguous matches through */
	{
	    if (ifound > 1)
	    {
		acdLog ("@(=) ambiguous match, last match accepted %S\n",
			testvar);
		acdLog ("@(=) ambiguous match, last match accepted %S\n",
			testvar);
	    }
	    acdLog ("%S ~= %S : '%S'\n", testvar, acdExpTmpstr, *result);
	    return ajTrue;
	}
	if (ifound == 0)
	{
	    (void) ajStrAssS (result, elsevar);
	    acdLog ("%S != else : '%S'\n", testvar, *result);
	    return ajTrue;
	}
    }
    
    return ajFalse;
}

/* @funcstatic acdExpFilename *************************************************
**
** Looks for an expression @(file: string) and returns a trimmed
** lower case file name prefix or suffix.
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpFilename (AjPStr* result, AjPStr str)
{
    static AjPRegexp filexp = NULL;

    if (!filexp)			/* file: name */
	filexp = ajRegCompC("^[ \t]*[Ff][Ii][Ll][Ee]:[ \t]*([^ \t]+)[ \t]*$");

    if (ajRegExec(filexp, str))
    {
	acdLog("filexp matched  '%S'\n", str);
	ajRegSubI(filexp, 1, &acdExpTmpstr);
	(void) ajStrAssS (result, acdExpTmpstr);
	(void) ajFileNameShorten (result);
	(void) ajStrToLower(result);
	acdLog ("file: %S = '%S'\n", acdExpTmpstr, *result);

	return ajTrue;
    }

    return ajFalse;
}

/* @funcstatic acdExpExists ***************************************************
**
** Looks for an expression @(is string) and returns ajTrue
** if there is a value, and ajFalse if there is none
**
** @param [r] result [AjPStr*] Expression result
** @param [r] str [AjPStr] String with possible expression
** @return [AjBool] ajTrue if successfully resolved
** @@
******************************************************************************/

static AjBool acdExpExists (AjPStr* result, AjPStr str)
{
    static AjPRegexp filexp = NULL;
    AjBool test;
    if (!filexp)			/* file: name */
	filexp = ajRegCompC("^[ \t]*[Ii][Ss]:[ \t]*([^ \t]*)[ \t]*$");

    if (ajRegExec(filexp, str))
    {
	acdLog("filexp matched  '%S'\n", str);
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

static AjBool acdVarSplit (AjPStr var, AjPStr* name, AjPStr* attrname)
{
    ajint i;

    (void) ajStrAssS (name, var);
    i = ajStrFindC(*name, ".");		/* qualifier with value */
    if (i > 0)
    {
	(void) ajStrAssS(attrname, var);
	(void) ajStrSub(name, 0, i-1); /* strip any value and keep testing */
	(void) ajStrTrim(attrname, i+1);
    }
    else
    {
	(void) ajStrDelReuse(attrname);
    }

    return ajTrue;
}

/* @funcstatic acdAttrTest ***************************************************
**
** Tests for the existence of a named attribute
**
** @param [r] thys [AcdPAcd] ACD item
** @param [r] attrib [char*] Attribute name
** @return [AjBool] ajTrue if the named attribute exists
** @@
******************************************************************************/

static AjBool acdAttrTest (AcdPAcd thys, char *attrib)
{
    AcdPAttr attr;
    AcdPAttr defattr = acdAttrDef;
    ajint i;

    if (acdIsQtype(thys))
	attr = acdType[thys->Type].Attr;
    else
	attr = acdKeywords[thys->Type].Attr;

    i = acdFindAttrC (attr, attrib);
    if (i >= 0)
	return ajTrue;

    if (thys->DefStr)
    {
	i = acdFindAttrC (defattr, attrib);
	if (i >= 0)
	    return ajTrue;
    }

    return ajFalse;
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

static AjPStr acdAttrValue (AcdPAcd thys, char *attrib)
{
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

    if (thys->DefStr)
    {
	i = acdFindAttrC (defattr, attrib);
	if (i >= 0)
	    return defstr[i];
    }
    if (i < 0)
	acdErrorAcd (thys, "Unknown attribute '%s'\n", attrib);
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
			       AjPStr *str)
{
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
    {
	ajStrAssS (str, attrstr[i]);
	if (ajStrLen(*str))
	    return ajTrue;
	ajStrAssC (str, def);
	return ajFalse;
    }

    if (thys->DefStr)
    {
	i = acdFindAttrC (defattr, attrib);
	if (i >= 0)
	{
	    ajStrAssS (str, defstr[i]);
	    if (ajStrLen(*str))
		return ajTrue;
	    ajStrAssC (str, def);
	    return ajFalse;
	}
    }
    if (i < 0)
	acdErrorAcd (thys, "Unknown attribute %s\n", attrib);
    return ajFalse;
}

/* @func ajAcdSetControl ******************************************************
**
** Sets special qualifiers which were originally provided via the
** command line.
**
** Sets special internal variables to reflect their presence.
**
** Currently these are "acdlog", "acdpretty", "acdtable", "acdvalid"
**
** @param [r] optionName [char*] option name
** @return [AjBool] ajTrue if option was recognised
** @@
******************************************************************************/

AjBool ajAcdSetControl (char* optionName)
{

    if (!ajStrCmpCaseCC(optionName, "acdlog"))
    {
	acdDoLog = ajTrue;
	return ajTrue;
    }
    if (!ajStrCmpCaseCC(optionName, "acdpretty"))
    {
	acdDoPretty = ajTrue;
	return ajTrue;
    }
    if (!ajStrCmpCaseCC(optionName, "acdtable"))
    {
	acdDoTable = ajTrue;
	return ajTrue;
    }

    if (!ajStrCmpCaseCC(optionName, "acdvalid"))
    {
	acdDoValid = ajTrue;
	return ajTrue;
    }

    if (!ajStrCmpCaseCC(optionName, "acdhelp"))
    {
	acdDoHelp = ajTrue;
	return ajTrue;
    }

    if (!ajStrCmpCaseCC(optionName, "acdverbose"))
    {
	acdVerbose = ajTrue;
	return ajTrue;
    }

    /* program source error */
    ajDie("Unknown ajAcdSetControl control option '%s'", optionName);

    return ajFalse;
}

/* @funcstatic acdArgsScan ****************************************************
**
** Steps through the command line and checks for special qualifiers.
** Sets special internal variables to reflect their presence.
**
** Currently these are "-debug", "-stdout", "-filter", "-options"
** "-help" and "-auto", plus the message controls
** "-warning", "-error", "-fatal", "-die"
**
** @param [r] argc [ajint] Number of arguments
** @param [r] argv [char* []] Actual arguments as a text array.
** @return [void]
** @@
******************************************************************************/

static void acdArgsScan (ajint argc, char *argv[])
{
    ajint i;

    for (i=0; i < argc; i++)
    {
	if (!strcmp(argv[i], "-debug"))
	{
	    acdDebug = ajTrue;
	    acdDebugSet = ajTrue;
	}
	if (!strcmp(argv[i], "-nodebug"))
	{
	    acdDebug = ajFalse;
	    acdDebugSet = ajTrue;
	}
	if (!strcmp(argv[i], "-stdout"))   acdStdout = ajTrue;
	if (!strcmp(argv[i], "-filter"))   acdFilter = ajTrue;
	if (!strcmp(argv[i], "-options"))  acdOptions = ajTrue;
	if (!strcmp(argv[i], "-verbose"))  acdVerbose = ajTrue;
	if (!strcmp(argv[i], "-help"))     acdDoHelp = ajTrue;
	if (!strcmp(argv[i], "-auto"))     acdAuto = ajTrue;

	/* deprecated - to be removed in a future version */
	if (!strcmp(argv[i], "-acdlog"))   acdDoLog = ajTrue;
	if (!strcmp(argv[i], "-acdpretty"))  acdDoPretty = ajTrue;
	if (!strcmp(argv[i], "-acdtable")) acdDoTable = ajTrue;
	/* end of deprecated set */

	if (!strcmp(argv[i], "-warning"))   AjErrorLevel.warning = ajTrue;
	if (!strcmp(argv[i], "-nowarning")) AjErrorLevel.warning = ajFalse;
	if (!strcmp(argv[i], "-error"))     AjErrorLevel.error = ajTrue;
	if (!strcmp(argv[i], "-noerror"))   AjErrorLevel.error = ajFalse;
	if (!strcmp(argv[i], "-fatal"))     AjErrorLevel.fatal = ajTrue;
	if (!strcmp(argv[i], "-nofatal"))   AjErrorLevel.fatal = ajFalse;
	if (!strcmp(argv[i], "-die"))       AjErrorLevel.die = ajTrue;
	if (!strcmp(argv[i], "-nodie"))     AjErrorLevel.die = ajFalse;

	if (!strcmp(argv[i], "-help"))
	{
	    acdLog("acdArgsScan -help argv[%d]\n", i);
	}
    }
    acdLog ("acdArgsScan acdDebug %B acdDoHelp %B\n", acdDebug, acdDoHelp);

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

static void acdArgsParse (ajint argc, char *argv[])
{
    
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
    while (i < argc)
    {
	acdLog ("%s ", argv[i]);
	i++;
    }
    acdLog ("\n");
    acdLog ("\n");
    
    i = 1;                        /* skip the program name */
    
    while (i < argc)
    {
	cp = argv[i];
	if ((i+1) < argc) cq = argv[i+1];
	else cq = NULL;
	
	acdLog ("\n");
	acdLog ("argv[%d] <%s>", i, cp);
	if (cq) acdLog (" + argv[%d] <%s>", i+1, cq);
	acdLog ("\n");
	jparam = 0;
	if ((j = acdIsQual(cp, cq, &jparam, &qual, &value, &number, &acd)))
	{
	    if (jparam)
	    {
		acdLog ("Parameter (%d) ", jparam);
		acdParamSet[jparam-1] = ajTrue;
		if (iparam == (jparam-1))
		{
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
	    
	    /* acdFindQual dies (Unknown qualifier) if acd is not set,
	       so we are safe here */
	    
	    (void) acdDef (acd, value);
	    acdLog ("set qualifier -%S[%d] (param %d) = %S\n",
		    acd->Name, acd->PNum, jparam, value);
	    /* loop over any associated qualifiers for the rest */
	    acdLog ("number: %d jparam: %d acd->PNum: %d acdNParam: %d\n",
		    number, jparam, acd->PNum, acdNParam);
	    
	    if (!number && !jparam && acd->PNum)
	    {
		for (itestparam = acd->PNum+1; itestparam <= acdNParam;
		     itestparam++)
		{
		    acdLog ("test [%d] '%S'\n", itestparam, qual);
		    acd = acdFindQual (qual, NULL, NULL,
				       itestparam, &jtestparam);
		    if (acd)
		    {
			(void) acdDef (acd, value);
			acdLog ("set next qualifier -%S[%d] (param %d) = %S\n",
				acd->Name, acd->PNum, jparam, value);
		    }
		    else
			acdLog ("no -%S[%d]\n", qual, itestparam);
		}
	    }
	    
	    if (j == 2) i++;
	}
	else			/* not a qualifier - assume a parameter */
	{
	    iparam = acdNextParam(0);	/* first free parameter */
	    (void) acdIsParam(cp, &param, &iparam, &acd); /* die if too many */
	    if (acdIsParamValue(param))
	    {
		acdLog ("Parameter %d: %S = %S\n",
			iparam, acd->Name, param);
		(void) acdDef (acd, param);
		acdParamSet[iparam-1] = ajTrue;
	    }
	    else		 /* missing value "." or "" ignored */
	    {
		acdLog ("Parameter %d: %S = '%S' ** missing value **\n",
			iparam, acd->Name, param);
		ajStrAssC (&param, "");
		(void) acdDef (acd, param);
		acdParamSet[iparam-1] = ajTrue;
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
** it will be ignored for now.
**
** @param [r] pval [AjPStr] Parameter value
**
** @return [AjBool] ajFalse for a missing value.
** @@
******************************************************************************/

static AjBool acdIsParamValue (AjPStr pval)
{
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

static ajint acdNextParam (ajint pnum)
{
    ajint i;

    if (pnum > acdNParam) return pnum+1; /* all done */

    for (i=pnum;i<acdNParam;i++)
	if (!acdParamSet[i]) return i;	/* next free parameter */

    return acdNParam+1;			/* all done */
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
                          AcdPAcd* acd)
{
    char *cp = arg;

    acdLog ("acdIsParam arg: '%s' param: '%S' iparam: %d\n",
	    arg, *param, *iparam);

    if (*iparam >= acdNParam)		/* test acdc-toomanyparam */
    {
	ajErr ("Argument '%s' : Too many parameters %d/%d\n",
	       arg, (*iparam), acdNParam);
	ajExitBad ();
    }

    (*iparam)++;
    *acd = acdFindParam (*iparam);

    if (!strcmp(cp, "."))		/* missing value */
    {
	(void) ajStrAssC (param, "");	/* clear the parameter */
	return ajTrue;
    }

    (void) ajStrAssC(param, arg);	/* copy the argument value */
    if (*acd)
    {
	if ((*acd)->AssocQuals)
	{
	    acdLog ("acdMasterQual [param] set to -%S\n", (*acd)->Name);
	    acdMasterQual = *acd;
	}
	else if (acdMasterQual)
	{
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
			AjPStr *pvalue, ajint* number, AcdPAcd* acd)
{
    ajint ret=0;
    char *cp = arg;
    ajint i;
    AjBool gotvalue = ajFalse;
    AjBool ismissing = ajFalse;
    AjBool qstart = ajFalse;
    AjBool nullok = ajFalse;
    AjBool attrok = ajFalse;
    static AjPStr qmaster = NULL;
    static AjPStr noqual = NULL;
    
    *number = 0;
    
    if (!strcmp(cp, "-"))		/* stdin or stdout parameter */
	return 0;
    if (!strcmp(cp, "."))		/* dummy parameter */
	return 0;
    
    if (strchr("-/", *cp))	/* first character vs. qualifier starts */
    {
	cp++;
	qstart = ajTrue;
    }
    
    if (!*cp)
	return 0;
    
    /* qualifier: now play hunt the value */
    
    ret = 1;
    (void) ajStrAssC (pqual, cp);	/* qualifier with '-' or '/' removed */
    
    /*
     ** pqual could be
     ** qualname (unless boolean, look for next arg as the value)
     ** qualname=value (value as part of arg)
     ** noqualname (boolean negative, or nullok set to empty string
     */
    
    /*
     ** First check whether we have a value (set gotvalue) in the first arg
     */
    
    i = ajStrFindC(*pqual, "=");
    if (i >= 0)
    {
	(void) ajStrAssSub(pvalue, *pqual, (i+1), -1);
	
	acdLog("qualifier value '%S' '%S' %d .. %d\n",
	       *pvalue, *pqual, (i+1), -1);
	(void) ajStrSub (pqual, 0, (i-1));
	gotvalue = ajTrue;
    }
    else
    {
	if (!qstart)	/* no start, no "=" assume it's a parameter */
	    return 0;
	if (!ajStrIsAlnum(*pqual))	/* funny characters, fail */
	    return 0;
    }
    
    acdQualParse (pqual, &noqual, &qmaster, number);
    
    if (ajStrLen(qmaster))	/* specific master, turn off auto processing */
	acdMasterQual = NULL;	/* qmaster resets this in acdFindQual below */
    
    if (acdMasterQual)		/* we are still working with a master */
    {
	acdLog ("(a) master, try associated with acdFindQualAssoc\n");
	*acd = acdFindQualAssoc (acdMasterQual, *pqual, noqual, *number);
	if (!*acd)
	{
	    acdLog ("acdMasterQual cleared, was -%S\n", acdMasterQual->Name);
	    acdMasterQual = NULL;
	}
	else
	{
	    *number = acdMasterQual->PNum;
	    acdLog ("Qualifier -%S associated with -%S\n",
		    *pqual, acdMasterQual->Name);
	}
    }
    
    if (!acdMasterQual)
    {
	acdLog ("(b) no master, general test with acdFindQual\n");
	*acd = acdFindQual (*pqual, noqual, qmaster, *number, iparam);
    }
    
    if (!*acd)			/* test acdc-badqual */
	ajDie ("Unknown qualifier %s", arg);
    
    if ((*acd)->AssocQuals)	/* this one is a new master */
    {
	acdLog ("acdMasterQual set to -%S\n", (*acd)->Name);
	acdMasterQual = *acd;
    }
    
    if (gotvalue)
    {
	if (ajStrPrefix ((*acd)->Name, noqual))
	{				/* test acdc-noprefixvalue */
	    ajDie ("'no' prefix used with value for '%s'", arg);
	}
    }
    else
    {
	acdLog ("testing for a value\n");
	
	/*
	 ** Bool qualifiers can have no value
	 ** or can be followed by a valid Bool value
	 */
	
	if (ajStrPrefix ((*acd)->Name, noqual))
	{			       /* we have a -noqual matched */
	    acdLog ("we matched with -no\n");
	    if (!strcmp(acdType[acdListCurr->Type].Name,
			"boolean")) /* -nobool */
	    {
		acdLog ("-no%S=N boolean accepted\n", noqual);
		gotvalue = ajTrue;
		ret = 1;
		ajStrAssC(pvalue, "N");
		return ret;
	    }
	    if (acdAttrTest (*acd, "nullok"))
	    {
		attrok = acdAttrToBool(*acd, "nullok", /* -no for null value */
				       ajFalse, &nullok);
		acdLog ("check for nullok, found:%B value:%B\n",
			attrok, nullok);
	    }
	    else
	    {
		nullok = ajFalse;
	    }
	    if (nullok)
	    {
		acdLog ("-no%S='' nullOK accepted\n", noqual);
		gotvalue = ajTrue;
		ret = 1;
		ajStrAssC(pvalue, "");
		return ret;
	    }
	    else	 /* test acdc-noprefixbad acdc-noprefixbad2 */
	    {
		ajDie ("'no' prefix invalid for '%s'", arg);
	    }
	}
	
	if (!strcmp(acdType[acdListCurr->Type].Name, "boolean"))
	{
	    if (acdValIsBool(arg2))	/* bool value, accept */
	    {
		acdLog("acdValIsBool -%s '%s'\n", arg, arg2);
		gotvalue = ajTrue;
		ret = 2;
		(void) ajStrAssC(pvalue, arg2);
	    }
	    else			/* we must mean true */
	    {
		(void) ajStrAssC(pvalue, "Y");
	    }
	}
	else
	{
	    if (!arg2)
	    {
		(void) ajStrToBool ((*acd)->DefStr[DEF_MISSING], &ismissing);
		if (!ismissing)		/* test acdc-novalue */
		{
		    ajDie ("Value required for '%s'", arg);
		}
	    }
	    /* test for known qualifiers */
	    else
	    {
		if (*arg2 == '-')
		{
		    if (!acdTestQualC (arg2)) /* not known qualifier */
					      /* must be value */
		    {
			gotvalue = ajTrue;
		    }
		    else
		    {
			(void) ajStrToBool ((*acd)->DefStr[DEF_MISSING],
					    &ismissing);
			if (!ismissing)	/* test acdc-novalue2 */
			{
			    ajDie ("Value required for '%s' before '%s'",
				   arg, arg2);
			}
		    }
		}
		else
		{
		    gotvalue = ajTrue;
		}
	    }
	    if (gotvalue)
	    {
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

static AjBool acdValIsBool (char* arg)
{
    if (!arg) return ajFalse;
    switch (*arg)
    {
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
** Section and Endsection do not count
**
** @param [r] item [AjPStr] Item name
** @param [r] number [ajint] Item number (zero if a general item)
** @return [AcdPAcd] ACD item required
** @@
******************************************************************************/

static AcdPAcd acdFindItem (AjPStr item, ajint number)
{
    AcdPAcd ret=NULL;
    AcdPAcd pa;
    AjBool found = ajFalse;
    ajint ifound=0;
    static AjPStr ambigList = NULL;

    (void) ajStrAssC(&ambigList, "");

    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	found = ajFalse;
	if (ajStrPrefix (pa->Name, item))
	{
	    if (!number || number == pa->PNum)
		found = ajTrue;
	}
	if (found)
	{
	    if (ajStrMatch (pa->Name, item))
	    {
		return pa;
	    }
	    ifound++;
	    ret = pa;
	    acdAmbigApp (&ambigList, pa->Name);
	}
    }

    if (ifound == 1)
	return ret;
    if (ifound > 1)
    {
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
** @param [r] noqual [AjPStr] Alternative qualifier name
**        (qual with "no" prefix removed, or empty, or NULL)
** @param [rN] master [AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @param [u] iparam [ajint*]  Current parameter number
** @return [AcdPAcd] ACD item for qualifier
** @@
******************************************************************************/

static AcdPAcd acdFindQual (AjPStr qual, AjPStr noqual, AjPStr master,
			    ajint PNum, ajint *iparam)
{
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
    
    if (ajStrLen(master))
    {
	*iparam = 0;
	return acdFindQualMaster (qual, noqual, master, PNum);
    }
    
    (void) ajStrAssC(&ambigList, "");
    
    acdLog ("acdFindQual '%S' (%S) PNum: %d iparam: %d\n",
	    qual, noqual, PNum, *iparam);
    
    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	found = ajFalse;
	if (pa->Level == ACD_QUAL)
	{
	    if (ajStrPrefix (pa->Name, qual) ||
		ajStrPrefix (pa->Name, noqual))
	    {
		acdLog ("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
		if (PNum)               /* -begin2 forces match to #2 */
		{
		    if (PNum == pa->PNum)
		    {
			acdLog ("..matched PNum '%S' [%d]\n",
				pa->Name, pa->PNum);
			found = ajTrue;
		    }
		}
		else if (pa->PNum)	/* defined for parameter pa->PNum */
		{
		    acdLog ("..hit PNum '%S' [%d] (ambigList '%S')\n",
			    pa->Name, pa->PNum, ambigList);
		    if (!ifound  || !ajStrMatch(pa->Name, ambigList))
		    {
			found = ajTrue;
		    }
		}
		else	/* general match */
		{
		    found = ajTrue;
		}
		if (found)
		{
		    if (ajStrMatch (pa->Name, qual) ||
			ajStrMatch (pa->Name, noqual))
		    {
			acdListCurr = pa;
			return pa;
		    }
		    acdAmbigApp (&ambigList, pa->Name);
		    ifound++;
		    ret = pa;
		    acdLog ("..prefix only '%S', ifound %d\n",
			    pa->Name, ifound);
		}
	    }
	}
	else if (pa->Level == ACD_PARAM)
	{
	    if (ajStrPrefix (pa->Name, qual) ||
		ajStrPrefix (pa->Name, noqual))
	    {
		acdLog ("..matched param '%S' [%d]\n", pa->Name, pa->PNum);
		if (ajStrMatch (pa->Name, qual) ||
		    ajStrMatch (pa->Name, noqual))
		{
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
    
    if (ifound == 1)
    {
	acdListCurr = ret;
	if (isparam)
	    *iparam = ret->PNum;
	return ret;
    }
    if (ifound > 1)
    {
	ajWarn ("ambiguous qualifier '%S' (%S)", qual, ambigList);
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
** @param [r] noqual [AjPStr] Alternative qualifier name
**        (qual with "no" prefix removed, or empty, or NULL)
** @param [rN] master [AjPStr] Master qualifier name
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @return [AcdPAcd] ACD item for qualifier
** @@
******************************************************************************/

static AcdPAcd acdFindQualMaster (AjPStr qual, AjPStr noqual, AjPStr master,
				  ajint PNum)
{
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
    
    acdLog ("acdFindQualMaster '%S_%S' (%S) PNum: %d\n",
	    qual, master, noqual, PNum);
    
    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	found = ajFalse;
	if (pa->Level == ACD_QUAL)
	{
	    if (ajStrPrefix (pa->Name, master))
	    {
		acdLog ("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
		if (PNum)               /* -begin2 forces match to #2 */
		{
		    if (PNum == pa->PNum)
		    {
			acdLog ("..matched PNum '%S' [%d]\n",
				pa->Name, pa->PNum);
			found = ajTrue;
		    }
		}
		else if (pa->PNum)	/* defined for parameter pa->PNum */
		{
		    acdLog ("..hit PNum '%S' [%d] (ambigList '%S')\n",
			    pa->Name, pa->PNum, ambigList);
		    if (!ifound  || !ajStrMatch(pa->Name, ambigList))
		    {
			found = ajTrue;
		    }
		}
		else 	/* general match */
		{
		    found = ajTrue;
		}
		if (found)
		{
		    if (ajStrMatch (pa->Name, master))
		    {
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
	else if (pa->Level == ACD_PARAM)
	{
	    if (ajStrPrefix (pa->Name, master))
	    {
		acdLog ("..matched param '%S' [%d]\n", pa->Name, pa->PNum);
		if (ajStrMatch (pa->Name, master))
		{
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
    
    if (ifound > 1)
    {
	acdLog ("..ambiguous master qualifier for %S_%S (%S)",
		qual, master, ambigList);
	ajWarn ("Ambiguous master qualifier '%S' in %S_%S (%S)",
		master, qual, master, ambigList);
	(void) ajStrDelReuse(&ambigList);
	return NULL;
    }
    if (!ifound)
    {
	acdLog ("..master qualifier for %S_%S not found\n", qual, master);
	return NULL;
    }
    
    acdLog ("..master qualifier found '%S' %d\n", ret->Name, ret->PNum);
    
    ifound = 0;
    for (pa=ret->AssocQuals; pa && pa->Assoc; pa=pa->Next)
    {
	found = ajFalse;
	if (ajStrPrefix (pa->Name, qual) ||
	    ajStrPrefix (pa->Name, noqual))
	{
	    acdLog ("..matched qualifier '%S' [%d]\n", pa->Name, pa->PNum);
	    if (PNum)		      /* -begin2 forces match to #2 */
	    {
		if (PNum == pa->PNum)
		{
		    acdLog ("..matched PNum '%S' [%d]\n",
			    pa->Name, pa->PNum);
		    found = ajTrue;
		}
	    }
	    else if (pa->PNum)	  /* defined for parameter pa->PNum */
	    {
		acdLog ("..hit PNum '%S' [%d] (ambigList '%S')\n",
			pa->Name, pa->PNum, ambigList);
		if (!ifound  || !ajStrMatch(pa->Name, ambigList))
		{
		    found = ajTrue;
		}
	    }
	    else			/* general match */
	    {
		found = ajTrue;
	    }
	    if (found)
	    {
		if (ajStrMatch (pa->Name, qual) ||
		    ajStrMatch (pa->Name, noqual))
		{
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
    if (ifound == 1)
    {
	acdListCurr = ret;
	return ret;
    }
    if (ifound > 1)
    {
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
** @param [r] noqual [AjPStr] Alternative qualifier name
**        (qual with "no" prefix removed, or empty, or NULL)
** @param [r] PNum [ajint] Qualifier number (zero if a general qualifier)
** @return [AcdPAcd] ACD item for associated qualifier
** @error NULL returned if not found.
** @@
******************************************************************************/

static AcdPAcd acdFindQualAssoc (AcdPAcd thys, AjPStr qual, AjPStr noqual,
				 ajint PNum)
{
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

    /* acdLog ("acdFindQualAssoc '%S' pnum: %d\n", qual, pnum); */

    if (PNum  && (pa->PNum != PNum)) /* must be for same number (if any) */
	return NULL;

    for (; pa && pa->Assoc; pa=pa->Next)
    {
	if (ajStrPrefix (pa->Name, qual) ||
	    ajStrPrefix (pa->Name, noqual))
	{
	    if (ajStrMatch (pa->Name, qual) ||
		ajStrMatch (pa->Name, noqual))
	    {
		/* acdLog ("   *matched* '%S'\n", pa->Name); */
		acdListCurr = pa;
		return acdListCurr;
	    }
	    ifound++;
	    ret = pa;
	    acdAmbigApp (&ambigList, pa->Name);
	}
    }

    /* acdLog ("   ifound: %d\n", ifound); */

    if (ifound == 1)
    {
	acdListCurr = ret;
	return acdListCurr;
    }
    if (ifound > 1)
    {
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

static AcdPAcd acdFindParam (ajint PNum)
{
    /* test for match of parameter number and type */

    AcdPAcd pa ;

    for (pa=acdList; pa; pa=pa->Next)
    {
	if (acdIsStype(pa)) continue;
	if ((pa->Level == ACD_PARAM) && (pa->PNum == PNum))
	{
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
** Attributes include any specially calculated by the acdSet function
** for that type.
**
** @param [uP] result [AjPStr*] Resulting attribute value
** @param [r] name [AjPStr] ACD item name
** @param [r] attrib [AjPStr] attribute name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdGetAttr (AjPStr* result, AjPStr name, AjPStr attrib)
{
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
    if (isdigit((ajint)*--cq))
    {
	while (isdigit((ajint)*--cq));
	++cq;

	number = (ajint) strtol (cq, NULL, 0);
	ilen = cq - cp - 1;
	(void) ajStrSub (&tempstr, 0, ilen);
    }
    
    pa = acdFindItem (tempstr, number);
    if (!pa)			/* test ambigvar.acd */
	acdError ("Failed to resolve variable '%S'\n", name);
    
    if (!pa->ValStr)
    {
	if (!acdDoHelp)			/* test undefvar.acd */
	    acdError ("Variable '%S' not yet defined\n", name);
	ajStrAssC (result, "...");   /* only for help - patch it up */
	return ajTrue;
    }
    
    if (!ajStrLen(attrib))	/* just use valstr */
    {
	(void) ajStrAssS (result, pa->ValStr);
	(void) ajStrDelReuse (&tempstr);
	acdLog ("no attribute name, use valstr for %S '%S'\n",
		pa->Name, *result);
	pa->Used |= USED_ACD;
	return ajTrue;
    }
    
    if (pa->DefStr)
    {
	attr = acdAttrDef;
	i = acdFindAttr (attr, attrib);
	if (i >= 0)
	{
	    (void) ajStrAssS (result, pa->DefStr[i]);
	    (void) ajStrDelReuse (&tempstr);
	    acdLog ("default attribute %S found for %S '%S'\n",
		    attrib, pa->Name, *result);
	    return ajTrue;
	}
    }
    
    if (pa->NAttr)
    {
	attr = acdType[pa->Type].Attr;
	i = acdFindAttr (attr, attrib);
	if (i >= 0)
	{
	    (void) ajStrAssS (result, pa->AttrStr[i]);
	    (void) ajStrDelReuse (&tempstr);
	    acdLog ("type attribute %S found for %S '%S'\n",
		    attrib, pa->Name, *result);
	    return ajTrue;
	}
    }
    
    if (pa->SAttr)
    {
	attr = pa->SetAttr;
	i = acdFindAttr (attr, attrib);
	if (i >= 0)
	{
	    (void) ajStrAssS (result, pa->SetStr[i]);
	    (void) ajStrDelReuse (&tempstr);
	    acdLog ("calculated attribute %S found for %S '%S'\n",
		    attrib, pa->Name, *result);
	    return ajTrue;
	}
    }
    
    (void) ajStrDelReuse (&tempstr);
    
    acdLog ("*attribute %S not found for %S*\n", attrib, pa->Name);
    return ajFalse;
}

/* @funcstatic acdQualParse ***************************************************
**
** Converts a qualifier name to lower case and looks for a
** master qualifier name and a trailing number.
**
** @param [uP] pqual [AjPStr*] Qualifier name set to lower case
**        with number suffix removed
** @param [uP] pnoqual [AjPStr*] Qualifier name as pqual, with "no" prefix
**        removed, or empty string id pqual doesn't start with "no"
** @param [uP] pqmaster [AjPStr*] Master name for associated qualifier
** @param [w] number [ajint*] Qualifier number suffix if any
** @return [void]
** @@
******************************************************************************/

static void acdQualParse (AjPStr* pqual, AjPStr* pnoqual, AjPStr* pqmaster,
			  ajint* number)
{

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
    if (ajStrPrefixC(*pqual, "no"))
	ajStrAssSub(pnoqual, *pqual, 2, -1);
    else
	ajStrAssC (pnoqual, "");

    if (ajStrLen(tmpnum))
	(void) ajStrToInt(tmpnum, number);

    acdLog ("pqual '%S' pnoqual '%S' pqmaster '%S' tmpnum '%S' number %d\n",
	    *pqual, *pnoqual, *pqmaster, tmpnum, *number);

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

static void acdTokenToLower (char *token, ajint* number)
{
    char *cp, *cq;
    ajint ilen;

    ajCharToLower(token);
    cp = token;
    cq = &cp[strlen(token)];
    if (!isdigit((ajint)*--cq))
    {
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

static AjBool acdIsRequired (AcdPAcd thys)
{
    AjPStr* def = thys->DefStr;
    AjBool required = ajFalse;

    if (thys->Defined)
	return ajFalse;
    if (!thys->DefStr)
	return ajFalse;

    if (ajStrLen(def[DEF_REQUIRED]))
    {
	(void) acdVarResolve(&def[DEF_REQUIRED]);
	if (!ajStrToBool(def[DEF_REQUIRED], &required))
	    acdErrorAcd (thys, "Bad required flag %S\n",
			 def[DEF_REQUIRED]);
	return required;
    }

    if (acdOptions && ajStrLen(def[DEF_OPTIONAL]))
    {
	(void) acdVarResolve(&def[DEF_OPTIONAL]);
	if (!ajStrToBool(def[DEF_OPTIONAL], &required))
	    acdErrorAcd (thys, "Bad optional flag %S\n",
			 def[DEF_OPTIONAL]);
	return required;
    }

    return ajFalse;
}

/* @func ajAcdDebug ***********************************************************
**
** Tests whether debug messages are required by checking
** internal variable 'acdDebug'
**
** @return [AjBool] Debugging status.
** @@
******************************************************************************/

AjBool ajAcdDebug (void)
{
    ajDebug("ajAcdDebug returning %B", acdDebug);
    return acdDebug;
}

/* @func ajAcdDebugIsSet ******************************************************
**
** Tests whether the command line switch for debug messages has been set
** by testing internal variable 'acdDebugSet'
**
** @return [AjBool] Debugging status.
** @@
******************************************************************************/

AjBool ajAcdDebugIsSet (void)
{
    ajDebug("ajAcdDebugIsSet returning %B", acdDebugSet);
    return acdDebugSet;
}

/* @func ajAcdFilter **********************************************************
**
** Tests whether input and output use stdin and stdout as a filter
** by returning internal variable 'acdFilter'
**
** @return [AjBool] Filter status.
** @@
******************************************************************************/

AjBool ajAcdFilter (void)
{
    return acdFilter;
}

/* @func ajAcdStdout **********************************************************
**
** Tests whether output uses stdout for output by default
** by returning internal variable 'acdStdout'
**
** @return [AjBool] Stdout status.
** @@
******************************************************************************/

AjBool ajAcdStdout (void)
{
    return acdStdout;
}

/* @func ajAcdProgramS ********************************************************
**
** Returns the application (program) name from the ACD definition.
**
** @param [w] pgm [AjPStr*] returns the program name.
** @return [void]
** @@
******************************************************************************/

void ajAcdProgramS (AjPStr* pgm)
{
    (void) ajStrAssS (pgm, acdProgram);
    return;
}

/* @func ajAcdProgram *********************************************************
**
** Returns the application (program) name from the ACD definition.
**
** @return [char*] Program name
** @@
******************************************************************************/

char* ajAcdProgram (void)
{
    return ajStrStr(acdProgram);
}


/* @funcstatic acdPromptCodon *************************************************
**
** Sets the default prompt for this ACD object to be a codon usage file
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptCodon (AcdPAcd thys)
{
    AjPStr* prompt;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    (void) ajFmtPrintS (&thys->StdPrompt, "Codon usage file");
    return;
}


/* @funcstatic acdPromptDirlist ***********************************************
**
** Sets the default prompt for this ACD object to be a dirlist
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptDirlist (AcdPAcd thys)
{
    AjPStr* prompt;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    (void) ajFmtPrintS (&thys->StdPrompt, "Directory with files");
    return;
}

/* @funcstatic acdPromptFilelist **********************************************
**
** Sets the default prompt for this ACD object to be a filelist
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptFilelist (AcdPAcd thys)
{
    AjPStr* prompt;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    (void) ajFmtPrintS (&thys->StdPrompt, "Comma-separated file list");
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

static void acdPromptFeat (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt, "Input features"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt, "Second features"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt, "Third features"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt, "%dth features", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst features", count); break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd features", count); break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd features", count); break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth features", count); break;
	}
	break;
    }
    return;
}

/* @funcstatic acdPromptCpdb **************************************************
**
** Sets the default prompt for this ACD object to be a clean PDB file
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptCpdb (AcdPAcd thys)
{
    AjPStr* prompt;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    (void) ajFmtPrintS (&thys->StdPrompt, "Clean PDB file");
    return;
}

/* @funcstatic acdPromptScop **************************************************
**
** Sets the default prompt for this ACD object to be a scop entry
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptScop (AcdPAcd thys)
{
    AjPStr* prompt;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    (void) ajFmtPrintS (&thys->StdPrompt, "Scop entry");
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

static void acdPromptSeq (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt, "Input sequence"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt, "Second sequence"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt, "Third sequence"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt, "%dth sequence", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst sequence", count); break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd sequence", count); break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd sequence", count); break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth sequence", count); break;
	}
	break;
    }

    if (ajStrMatchCC(acdType[thys->Type].Name, "seqset"))
	ajStrAppC (&thys->StdPrompt, " set");
    if (ajStrMatchCC(acdType[thys->Type].Name, "seqall"))
	ajStrAppC (&thys->StdPrompt, "(s)");

    return;
}

/* @funcstatic acdPromptGraph *************************************************
**
** Sets the default prompt for this ACD object to be a sequence
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptGraph (AcdPAcd thys)
{
    AjPStr* prompt;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    (void) ajFmtPrintS (&thys->StdPrompt, "Graph type");
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

static void acdPromptFeatout (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				"Output features"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				"Second output features"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				"Third output features"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt,
			    "%dth output features", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst output features", count);
	    break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd output features", count);
	    break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd output features", count);
	    break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth output features", count);
	    break;
	}
	break;
    }
    return;
}

/* @funcstatic acdPromptAlign *************************************************
**
** Sets the default prompt for this ACD object to be a report output
** prompt with "first", "second" etc. added.
**
** @param [r] thys [AcdPAcd] Current ACD object.
** @return [void]
** @@
******************************************************************************/

static void acdPromptAlign (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				"Output alignment"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				"Second output alignment"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				"Third output alignment"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt,
			    "%dth output alignment", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst output alignment", count);
	    break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd output alignment", count);
	    break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd output alignment", count);
	    break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth output alignment", count);
	    break;
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

static void acdPromptReport (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				"Output report"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				"Second output report"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				"Third output report"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt,
			    "%dth output report", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst output report", count);
	    break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd output report", count);
	    break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd output report", count);
	    break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth output report", count);
	    break;
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

static void acdPromptSeqout (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				"Output sequence"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				"Second output sequence"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				"Third output sequence"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt,
			    "%dth output sequence", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst output sequence", count);
	    break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd output sequence", count);
	    break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd output sequence", count);
	    break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth output sequence", count);
	    break;
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

static void acdPromptOutfile (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
    {
	acdLog("acdPromptOutfile thys->DefStr NULL\n");
	return;
    }

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
    {
	acdLog("acdPromptOutfile found thys->DefStr[DEF_PROMPT] '%S'\n",
	       *prompt);
	/*(void) ajStrTrace (*prompt);*/
	/*(void) ajStrTrace (thys->DefStr[DEF_PROMPT]);*/
	return;
    }

    count++;
    acdLog("acdPromptOutfile count %d\n", count);
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				"Output file"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				"Second output file"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				"Third output file"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt,
			    "%dth output file", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst output file", count); break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd output file", count); break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd output file", count); break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth output file", count); break;
	}
	break;
    }
    /*(void) ajStrTrace (*prompt);*/
    /*(void) ajStrTrace (thys->DefStr[DEF_PROMPT]);*/
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

static void acdPromptInfile (AcdPAcd thys)
{
    AjPStr* prompt;
    static ajint count=0;

    if (!thys->DefStr)
	return;

    prompt = &thys->DefStr[DEF_PROMPT];
    if (ajStrLen(*prompt))
	return;

    count++;
    switch (count)
    {
    case 1: (void) ajFmtPrintS (&thys->StdPrompt, "Input file"); break;
    case 2: (void) ajFmtPrintS (&thys->StdPrompt, "Second input file"); break;
    case 3: (void) ajFmtPrintS (&thys->StdPrompt, "Third input file"); break;
    case 11:
    case 12:
    case 13:
	(void) ajFmtPrintS (&thys->StdPrompt, "%dth input file", count); break;
    default:
	switch (count % 10)
	{
	case 1: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dst input file", count); break;
	case 2: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%dnd input file", count); break;
	case 3: (void) ajFmtPrintS (&thys->StdPrompt,
				    "%drd input file", count); break;
	default: (void) ajFmtPrintS (&thys->StdPrompt,
				     "%dth input file", count); break;
	}
	break;
    }
    return;
}

/* @funcstatic acdCodeGet *****************************************************
**
** Translates a code into a message text using the code table
** for the current language.
**
** @param [r] code [AjPStr] Code name
** @param [w] msg [AjPStr*] Message text for this code in current language
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool acdCodeGet (AjPStr code, AjPStr *msg)
{
    AjPStr value;	       /* not static - copy of a table text */
    static AjPStr tmpcode = NULL;
    AjBool ret = ajFalse;

    acdLog ("acdCodeGet ('%S')\n", code);

    if (!acdCodeSet)
	acdCodeInit();

    (void) ajStrAssS(&tmpcode, code);
    (void) ajStrToLower (&tmpcode);

    value = ajTableGet (acdCodeTable, tmpcode);
    if (value)
    {
	(void) ajStrAssS (msg, value);
	acdLog ("%S value '%S'\n", code, *msg);
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

static AjBool acdCodeDef (AcdPAcd thys, AjPStr *msg)
{
    static AjPStr code = NULL;
    static AjPStr value = NULL;
    AjBool ret = ajFalse;

    acdLog ("acdCodeDef '%s'\n", acdType[thys->Type].Name);
    if (!acdCodeSet)
	acdCodeInit();

    code = ajStrNewC("def");
    (void) ajStrAppC (&code, acdType[thys->Type].Name);
    (void) ajStrToLower(&code);
    acdLog ("look for defcode '%S'\n", code);

    if (acdCodeGet (code, &value))
    {
	(void) ajFmtPrintS (msg, "-%S : %S",
			    thys->Name, value);
	(void) ajStrDelReuse (&value);
	ret = ajTrue;
    }
    else
    {
	acdLog ("defcode not found '%S'\n", code);
    }

    (void) ajStrDelReuse (&code);

    return ret;
}

/* @funcstatic acdHelpCodeDef *************************************************
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

static AjBool acdHelpCodeDef (AcdPAcd thys, AjPStr *msg)
{
    AjPStr code = NULL;
    static AjPStr value = NULL;
    AjBool ret = ajFalse;

    acdLog ("acdHelpCodeDef '%s'\n", acdType[thys->Type].Name);
    if (!acdCodeSet)
	acdCodeInit();

    code = ajStrNewC("help");
    (void) ajStrAppC (&code, acdType[thys->Type].Name);
    (void) ajStrToLower(&code);
    acdLog ("look for helpcode '%S'\n", code);

    if (acdCodeGet (code, &value))
    {
	(void) ajFmtPrintS (msg, "%S", value);
	(void) ajStrDelReuse (&value);
	ret = ajTrue;
    }
    else
    {
	acdLog ("helpcode not found '%S'\n", code);
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

static void acdCodeInit (void)
{
    AjPFile codeFile = NULL;
    static AjPStr codeFName = NULL;
    static AjPStr codeRoot = NULL;
    static AjPStr codeRootInst = NULL;
    static AjPStr codePack = NULL;
    static AjPStr codeCode = NULL;
    static AjPStr codeValue = NULL;
    static AjPStr codeLine = NULL;
    static AjPStr codeText = NULL;
    static AjPStr codeLanguage = NULL;
    AjPRegexp codexp = NULL;
    
    if (acdCodeSet) return;
    
    (void) ajNamRootPack (&codePack);
    (void) ajNamRootInstall (&codeRootInst);
    (void) ajFileDirFix (&codeRootInst);
    
    if (!ajNamGetValueC ("language", &codeLanguage))
	ajStrAssC (&codeLanguage, "english");
    
    if (ajNamGetValueC ("acdroot", &codeRoot))
    {
	(void) ajFileDirFix (&codeRoot);
	ajFmtPrintS (&codeFName, "%Scodes.%S", codeRoot, codeLanguage);
	codeFile = ajFileNewIn (codeFName);
	acdLog ("Code file in acdroot: '%S'\n", codeFName);
    }
    else
    {
	ajFmtPrintS (&codeFName, "%Sshare/%S/acd/codes.%S",
		     codeRootInst, codePack, codeLanguage);
	acdLog ("Code file installed: '%S'\n", codeFName);
	codeFile = ajFileNewIn (codeFName);
	if (!codeFile)
	{
	    acdLog ("Code file '%S' not opened\n", codeFName);
	    (void) ajNamRoot (&codeRoot);
	    (void) ajFileDirFix (&codeRoot);
	    ajFmtPrintS (&codeFName, "%Sacd/codes.%S", codeRoot, codeLanguage);
	    acdLog ("Code file from source dir: '%S'\n", codeFName);
	    codeFile = ajFileNewIn (codeFName);
	}
    }
    
    if (!codeFile)		/* test acdc-codemissing */
	ajWarn ("Code file %S not found", codeFName);
    else
	acdLog ("Code file %F used\n", codeFile);
    
    codeText = ajStrNew();
    
    /* fix by Nicolas Joly <njoly@pasteur.fr> */
    
    while (codeFile && ajFileReadLine(codeFile, &codeLine))
    {
	if (ajStrUncomment(&codeLine))
	{
	    (void) ajStrApp (&codeText, codeLine);
	    (void) ajStrAppC (&codeText, " ");
	}
    }
    ajFileClose (&codeFile);
    
    (void) ajStrDelReuse (&codeLine);
    
    acdCodeTable = ajStrTableNew(0);
    
    codexp = ajRegCompC("^ *([^ ]+) +\"([^\"]*)\"");
    while (ajRegExec (codexp, codeText))
    {
	codeCode = codeValue = NULL; /* need to save in table each time */
	ajRegSubI (codexp, 1, &codeCode);
	ajRegSubI (codexp, 2, &codeValue);
	(void) ajStrToLower (&codeCode);
	(void) ajTablePut (acdCodeTable, codeCode, codeValue);
	acdLog ("add to table %S '%S'\n", codeCode, codeValue);
	(void) ajRegPost (codexp, &codeText);
    }
    if (!ajStrIsSpace(codeText))	/* test acdc-codebad */
	ajDie ("Bad format in codes file %S after '%S \"%S\"'",
	       codeFName, codeCode, codeValue);
    
    ajRegFree (&codexp);
    (void) ajStrDelReuse (&codeText);
    (void) ajStrDelReuse (&codeFName);
    
    acdCodeSet = ajTrue;
    return;
}

/* @funcstatic acdSetQualAppl *************************************************
**
** Sets internal variables for the application booleans -debug etc.
**
** @param [r] thys [AcdPAcd] ACD object
** @param [r] val [AjBool] Value
** @return [AjBool] ajTrue if this was an application-wide variable.
** @@
******************************************************************************/

static AjBool acdSetQualAppl (AcdPAcd thys, AjBool val)
{
    ajint i=0;
    AjBool setval;
    static AjPStr setstr = NULL;
    static AjPStr valstr = NULL;
    static AjPStr bufstr = NULL;
    
    acdLog("acdSetQualAppl '%S'\n", thys->Name);
    
    for(i=0; acdQualAppl[i].Name; i++)
    {
	if (ajStrMatchC(thys->Name, acdQualAppl[i].Name))
	{
	    if (thys->Defined)	/* User put it on the command line */
	    {
		setval = val;
		acdLog ("Appl qualifier defined %S = %b\n",
			thys->Name, setval);
	    }
	    else			/* look for a variable */
	    {
		(void) ajFmtPrintS (&setstr, "%S", thys->Name);
		if (ajNamGetValue(setstr, &valstr))
		{
		    (void) ajStrToBool(valstr, &setval);
		    acdLog ("Appl qualifier variable %S = %b\n",
			    setstr, setval);
		}
		else	    /* nothing found, use the default value */
		    setval = val;
	    }
	    switch (i)		/* see acdQualAppl for the correct order */
	    {
	    case 0: acdAuto     = setval; break;
	    case 1: acdStdout   = setval; break;
	    case 2: acdFilter   = setval;
		if (acdFilter)
		{
		    acdAuto   = ajTrue;
		    acdStdout = ajTrue;
		}
		break;
	    case 3: acdOptions  = setval; break;
	    case 4:
		acdDebug = setval;
		/* acdLog ("acdSetQualAppl acdDebug %B\n", acdDebug); */
		acdDebugSet = ajTrue;
		if (ajNamGetValueC("debugbuffer", &bufstr))
		{
		    (void) ajStrToBool(bufstr, &acdDebugBuffer);
		}
		break;
		
		/* deprecated - to be removed in a future version */
	    case 5: acdDoLog    = setval; break;
	    case 6: acdDoPretty = setval; break;
	    case 7: acdDoTable    = setval; break;
		/* end of deprecated set */
		
	    case 8: acdVerbose  = setval; break;
	    case 9: acdDoHelp   = setval; break;
	    case 10: AjErrorLevel.warning = setval; break;
	    case 11: AjErrorLevel.error   = setval; break;
	    case 12: AjErrorLevel.fatal   = setval; break;
	    case 13: AjErrorLevel.die     = setval; break;
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

static void acdSelectPrompt (AcdPAcd thys)
{
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
    while (ajStrDelim (&line, &handle, NULL))
    {
	(void) ajStrTrimC (&line, white);
	ajUser ("  %5d : %S", ++i, line);
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

static void acdListPrompt (AcdPAcd thys)
{
    AjPStr hdrstr;
    AjPStr codedelim=NULL;
    AjPStr delim=NULL;
    AjPStr value;
    AjPStrTok handle;
    AjPStrTok codehandle;
    static AjPStr line = NULL;
    static AjPStr code = NULL;
    static AjPStr desc = NULL;
    static char* white = " \t\n\r";
    AjBool ddelim=ajFalse;
    AjBool cdelim=ajFalse;

    if (acdAuto) return;

    hdrstr = acdAttrValue (thys, "header");
    if (ajStrLen(hdrstr))
	ajUser ("%S", hdrstr);

    delim = acdAttrValue (thys, "delimiter");
    if (!ajStrLen(delim))
    {
	ddelim = ajTrue;
	(void) ajStrAssC(&delim, ";");
    }

    codedelim = acdAttrValue (thys, "codedelimiter");
    if (!ajStrLen(codedelim))
    {
	cdelim = ajTrue;
	(void) ajStrAssC(&codedelim, ":");
    }

    value = acdAttrValue (thys, "value");
    handle = ajStrTokenInit (value, ajStrStr(delim));
    while (ajStrDelim (&line, &handle, NULL))
    {
	codehandle = ajStrTokenInit (line, ajStrStr(codedelim));
	(void) ajStrToken (&code, &codehandle, NULL);
	(void) ajStrToken (&desc, &codehandle, ajStrStr(delim));
	(void) ajStrTrimC (&code, white);
	(void) ajStrTrimC (&desc, white);
	ajUser ("  %8S : %S", code, desc);
	(void) ajStrTokenClear (&codehandle);
    }

    (void) ajStrTokenClear (&handle);
    (void) ajStrDelReuse (&line);
    (void) ajStrDelReuse (&code);
    (void) ajStrDelReuse (&desc);
    if(ddelim)
	ajStrDel(&delim);
    if(cdelim)
	ajStrDel(&codedelim);

    return;
}

/* @funcstatic acdListValue ***************************************************
**
** Checks the user setting against the menu list of codes and descriptions.
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

static AjPStr* acdListValue (AcdPAcd thys, ajint min, ajint max,
			     AjPStr reply)
{
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
    static AjPStr validstr = NULL;
    AjPStr hitstr1 = NULL;
    AjPStr hitstr2 = NULL;
    static AjPStr ambigList = NULL;
    static AjPStr repdelim = NULL;
    static char* white = " \t\n\r";
    ajint k = 0;
    
    ajint ifound = 0;
    ajint jfound = 0;
    ajint ilen;
    ajint itoken=0;
    
    AjBool ok = ajTrue;
    
    list = ajListstrNew();
    
    (void) acdAttrToBool(thys, "casesensitive", ajFalse, &exactcase);
    
    acdAttrValueStr (thys, "delimiter", ";", &delim);
    acdAttrValueStr (thys, "codedelimiter", ":", &codedelim);
    
    if (!repdelim)
    {
	repdelim = ajStrNewL(10);
	(void) ajStrAssC (&repdelim, ",");
    }
    
    value = acdAttrValue (thys, "value");
    
    (void) ajStrAssC(&ambigList, "");
    ajStrAssC(&validstr, "");
    rephandle = ajStrTokenInit (reply, ajStrStr(repdelim));
    while (ajStrToken (&repstr, &rephandle, NULL))
    {
	itoken++;
	acdLog("testing '%S'\n", repstr);
	handle = ajStrTokenInit (value, ajStrStr(delim));
	ifound = jfound = 0;
	ajStrAssC(&validstr, "");
	while (ajStrDelim (&line, &handle, NULL))
	{
	    codehandle = ajStrTokenInit (line, ajStrStr(codedelim));
	    (void) ajStrToken (&code, &codehandle, NULL);
	    (void) ajStrToken (&desc, &codehandle, ajStrStr(delim));
	    (void) ajStrTrimC (&code, white);
	    (void) ajStrTrimC (&desc, white);
 
	    if (ajStrLen(validstr))
		ajStrAppK(&validstr, ',');
	    ajStrApp(&validstr, code);

	    if (ajStrMatch(code, repstr) ||
		(!exactcase && ajStrMatchCase(code, repstr)))
	    {
		ifound = 1;
		(void) ajStrAssS (&hitstr1, code);
		break;
	    }
	    if (ajStrMatch(desc, repstr) ||
		(!exactcase && ajStrMatchCase(desc, repstr)))
	    {
		jfound = 1;
		(void) ajStrAssS (&hitstr2, code);
		break;
	    }
	    if (ajStrPrefix(code, repstr) ||
		(!exactcase && ajStrPrefixCase(code, repstr)))
	    {
		ifound++;
		(void) ajStrAssS (&hitstr1, code);
		acdAmbigApp (&ambigList, code);
	    }
	    if (ajStrPrefix(desc, repstr) ||
		(!exactcase && ajStrPrefixCase(desc, repstr)))
	    {
		jfound++;
		(void) ajStrAssS (&hitstr2, code);
		acdAmbigApp (&ambigList, desc);
	    }

	    (void) ajStrTokenClear (&codehandle);
	    codehandle = NULL;
	} /* end of while */
	
	(void) ajStrTokenClear(&codehandle);
	
	if (ifound == 1)
	{
	    hitstr = ajStrDup(hitstr1);	/* don't assign - needed for list */
	    ajListstrPushApp (list, hitstr);
	}
	else if (jfound == 1)
	{
	    hitstr = ajStrDup(hitstr2);	/* don't assign - needed for list */
	    ajListstrPushApp (list, hitstr);
	}
	else
	{
	    if (ifound || jfound) /* test acdc-listambig1 acdc-listambig2 */
		ajErr ("'%S' is ambiguous (%S)", repstr, ambigList);
	    else			/* test acdc-listbad */
		ajErr ("'%S' is not a valid menu option\n"
		       "Accepted short codes are: %S",
		       repstr, validstr);
	    ok = ajFalse;
	    break;
	}
	(void) ajStrTokenClear (&handle);
    }
    (void) ajStrTokenClear (&rephandle);
    (void) ajStrDelReuse (&repstr);
    
    ilen = ajListstrLength(list);
    acdLog ("Found %d matches OK: %b min: %d max: %d\n",
	    ilen, ok, min, max);
    if (ok)
    {
	if (ilen < min)
	{				/* test acdc-listmin */
	    if (min <= 1)
		ajErr ("Menu needs %d value", min);
	    else
		ajErr ("Menu needs %d values", min);
	    ok = ajFalse;
	}
	if (ilen > max)			/* test acdc-listmax */
	{
	    if (max <= 1)
		ajErr ("Menu allows no more than %d value", max);
	    else
		ajErr ("Menu allows no more than %d values", max);
	    ok = ajFalse;
	}
    }
    
    if (ok)
    {
	AJCNEW0(val, ilen+1);
	for (k = 0; k < ilen; k++)
	{
	    (void) ajListstrPop (list, &val[k]);
	    acdLog ("Accept: '%S'\n", val[k]);
	}
    }
    
    acdLog ("Found %d matches\n", ilen);
    acdLog ("Menu length now %d\n", ajListstrLength(list));
    if (ok)
	acdLog ("Before return val[0] '%S'\n", val[0]);
    
    /* do not delete hitstr
     ** it is copied as the last list item stored in val
     */
    ajListstrDel (&list);
    (void) ajStrDelReuse (&delim);
    (void) ajStrDelReuse (&codedelim);
    (void) ajStrDelReuse (&line);
    (void) ajStrDelReuse (&code);
    (void) ajStrDelReuse (&desc);
    (void) ajStrDelReuse (&ambigList);
    ajStrDel(&hitstr1);
    ajStrDel(&hitstr2);
    
    if (ok)
	acdLog ("Before return val[0] '%S'\n", val[0]);
    
    if (!ok)
	return NULL;
    
    return val;
}

/* @funcstatic acdSelectValue *************************************************
**
** Checks the user setting against the selection list set of codes
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

static AjPStr* acdSelectValue (AcdPAcd thys, ajint min, ajint max,
			       AjPStr reply)
{
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
    static AjPStr validstr = NULL;
    AjPStr hitstr2 = NULL;
    static AjPStr ambigList = NULL;
    static AjPStr repdelim = NULL;
    static char* white = " \t\n\r";
    ajint i = 0;
    ajint k = 0;
    
    ajint jfound = 0;
    ajint icnt = 0;
    ajint ilen;
    ajint itoken=0;
    
    AjBool ok = ajTrue;
    
    list = ajListstrNew();
    
    (void) acdAttrToBool(thys, "casesensitive", ajFalse, &exactcase);
    
    delim = acdAttrValue (thys, "delimiter");
    if (!ajStrLen(delim))
	(void) ajStrAssC(&delim, ";");
    
    if (!repdelim)
    {
	repdelim = ajStrNewL(10);
	(void) ajStrAssC (&repdelim, ",");
	/*(void) ajStrAppC (&repdelim, white);*/ /* sorry no white space */
    }
    
    value = acdAttrValue (thys, "value");
    
    (void) ajStrAssC(&ambigList, "");
    ajStrAssC(&validstr, "");
    rephandle = ajStrTokenInit (reply, ajStrStr(repdelim));
    while (ajStrToken (&repstr, &rephandle, NULL))
    {
	itoken++;
	
	acdLog("testing '%S'\n", repstr);
	handle = ajStrTokenInit (value, ajStrStr(delim));
	i = jfound = 0;
	for (icnt=1; ajStrDelim (&desc, &handle, NULL); icnt++)
	{
	    (void) ajStrTrimC (&desc, white);
 
	    if (itoken == 1)
	    {
		if (ajStrLen(validstr))
		    ajStrAppK(&validstr, ',');
		ajStrApp(&validstr, desc);
	    }

	    if (ajStrMatch(desc, repstr) ||
		(!exactcase && ajStrMatchCase(desc, repstr)))
	    {
		jfound = 1;
		(void) ajStrAssS (&hitstr2, desc);
		break;
	    }
	    if (ajStrPrefix(desc, repstr) ||
		(!exactcase && ajStrPrefixCase(desc,repstr)))
	    {
		jfound++;
		(void) ajStrAssS (&hitstr2, desc);
		acdAmbigApp (&ambigList, desc);
	    }
	    if (ajStrToInt(repstr, &i) && i == icnt)
	    {
		jfound++;
		(void) ajStrAssS (&hitstr2, desc);
		acdAmbigApp (&ambigList, repstr);

	    }
	} /* end of while */
	
	if (jfound == 1)
	{
	    hitstr = ajStrDup(hitstr2);	/* don't assign - needed for list */
	    ajListstrPushApp (list, hitstr);
	}
	else
	{
	    if (jfound)			/* test acdc-selectambig */
		ajErr ("'%S' is ambiguous (%S)", repstr, ambigList);
	    else			/* test acdc-selectbad */
		ajErr ("'%S' is not a valid selection list option\n"
		       "Accepted values are: %S",
		       repstr, validstr);
	    ok = ajFalse;
	    break;
	}
	(void) ajStrTokenClear (&handle);
    }
    (void) ajStrTokenClear (&rephandle);
    (void) ajStrDelReuse (&repstr);
    
    ilen = ajListstrLength(list);
    
    if (ok)
    {
	if (ilen < min)			/* test acdc-selectmin */
	{
	    if (min <= 1)
		ajErr ("Selection list needs %d value", min);
	    else
		ajErr ("Selection list needs %d values", min);
	    ok = ajFalse;
	}
	if (ilen > max)			/* test acdc-selectmax */
	{
	    if (max <= 1)
		ajErr ("Selection list allows no more than %d value", max);
	    else
		ajErr ("Selection list allows no more than %d values", max);
	    ok = ajFalse;
	}
    }
    
    if (ok)
    {
	AJCNEW0(val, ilen+1);
	for (k = 0; k < ilen; k++)
	{
	    (void) ajListstrPop (list, &val[k]);
	}
    }
    
    acdLog ("Found %d matches\n", ilen);
    
    /* do not delete hitstr
     ** it is copied as the last list item stored in val
     */
    ajListstrDel (&list);
    (void) ajStrDelReuse (&line);
    (void) ajStrDelReuse (&code);
    (void) ajStrDelReuse (&desc);
    (void) ajStrDelReuse (&ambigList);
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

static void acdAmbigApp (AjPStr* pambigList, AjPStr str)
{
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

static void acdAmbigAppC (AjPStr* pambigList, char* txt)
{
    if (ajStrLen(*pambigList)) (void) ajStrAppC (pambigList, ",");
    (void) ajStrAppC (pambigList, txt);
}

/* @funcstatic acdDataFilename ************************************************
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

static AjBool acdDataFilename (AjPStr* infname, AjPStr name, AjPStr ext)
{
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

/* @funcstatic acdInFilename **************************************************
**
** Sets a default input file name. If filtering is on, this will be stdin.
** Otherwise it is blank.
**
** @param [wP] infname [AjPStr*] Input file name
** @return [AjBool] ajTrue if a name was successfully set
** @@
******************************************************************************/

static AjBool acdInFilename (AjPStr* infname)
{
    AjBool ret = ajFalse;

    if (!acdInFile && acdFilter)
    {
	(void) ajStrAssC (infname, "stdin");
	ret = ajTrue;
    }
    else
	(void) ajStrAssC (infname, "");

    acdInFile++;

    return ret;
}

/* @funcstatic acdOutDirectory ************************************************
**
** Sets a default output file directory. Uses the _OUTDIRECTORY variable
** as a default value, but any input string overrides it.
**
** The recommendation is that the directory should always be provided
** in the emboss.defaults file or by an environment variable.
**
** As for all associated qualifiers, it is also possible to set a value
** in the ACD file.
**
** @param [wP] dir [AjPStr*] Specified directory
** @return [AjBool] ajTrue if a directory was successfully set
** @@
******************************************************************************/

static AjBool acdOutDirectory (AjPStr* dir)
{
    AjBool ret = ajFalse;
    static AjPStr defdir = NULL;
    static AjPStr mydir = NULL;

    acdLog ("acdOutDirectory ('%S')\n",
	    *dir);

    if (!defdir)
    {
	if (!ajNamGetValueC ("outdirectory", &defdir))
	    ajStrAssC(&defdir, "");
    }

    if (dir && ajStrLen(*dir))
	ajStrAssS (&mydir, *dir);
    else
	ajStrAssS (&mydir, defdir);

    if (ajStrLen(mydir))
    {
	ajFileDirFix(&mydir);
	ajStrAssS(dir, mydir);
	ret = ajTrue;
    }
    else
    {
	ajStrAssC(dir, "");
	ret = ajFalse;
    }

    acdLog (". . . dir '%S' ret: %B\n",
	    *dir, ret);

    (void) ajStrDelReuse(&mydir);
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

static AjBool acdOutFilename (AjPStr* outfname, AjPStr name, AjPStr ext)
{
    AjBool ret = ajFalse;
    static AjPStr myname = NULL;
    static AjPStr myext = NULL;

    acdLog ("acdOutFilename ('%S', '%S', '%S')\n",
	    *outfname, name, ext);

    if (!acdOutFile && acdStdout) /* first outfile, running as a filter */
    {
	(void) ajStrAssC (outfname, "stdout");
	acdLog ("outfile <first> '%S'\n", *outfname);
	acdOutFile++;
	return ajTrue;
    }

    (void) ajStrSet(&myname, name);	/* use name if given */
    (void) ajStrSet(&myname, acdInFName); /* else use saved name */
    (void) ajStrSetC(&myname, "outfile"); /* all else fails, use "outfile" */

    (void) ajStrSet(&myext, ext);	/* use extension if given */
    if (!acdOutFile)
	(void) ajStrSet(&myext, acdProgram);

    /* else try program name for first file */
    if (!ajStrLen(myext))	/* if all else fails, use out2 etc. */
	(void) ajFmtPrintS (&myext, "out%d", acdOutFile+1);

    acdLog (". . . myname '%S', myext '%S'\n",
	    myname, myext);

    if (ext && ajStrLen(myext))	 /* NULL ext means add no extension */
	(void) ajFmtPrintS(outfname, "%S.%S", myname, myext);
    else
	(void) ajStrApp (outfname, myname);

    acdOutFile++;

    acdLog ("outfile %d %S.%S\n", acdOutFile, myname, myext);

    (void) ajStrDelReuse(&myname);
    (void) ajStrDelReuse(&myext);

    return ret;
}

/* @funcstatic acdInFileSave **************************************************
**
** For the first call, saves the input filename for use in building output
** file name(s).
**
** @param [P] infname [AjPStr] Input file name
** @return [AjBool] ajTrue if a name was successfully set
** @@
******************************************************************************/

static AjBool acdInFileSave (AjPStr infname)
{
    if (acdInFile != 1)
	return ajFalse;

    acdLog ("acdInFileSave (%S)\n",
	    infname);

    if (!ajStrLen(infname)) return ajFalse;

    (void) ajStrAssS (&acdInFName, infname);
    (void) ajFileNameShorten (&acdInFName);
    (void) ajStrToLower(&acdInFName);

    acdLog ("acdInFileSave (%S) input file set to '%S'\n",
	    infname, acdInFName);

    return ajTrue;
}

/* @funcstatic acdInTypeSeqSave ***********************************************
**
** For the first call, saves the input filename for use in building output
** file name(s).
**
** @param [P] intype [AjPStr] Input sequence type
** @return [AjBool] ajTrue if a type was successfully set
** @@
******************************************************************************/

static AjBool acdInTypeSeqSave (AjPStr intype)
{
    if (acdInTypeSeqName)
	return ajFalse;

    acdLog ("acdInTypeSeqSave (%S)\n",
	    intype);

    if (!ajStrLen(intype))
    {
	ajStrAssC (&acdInTypeSeqName, "");
	acdLog("Input sequence type defaults to ''\n", acdInTypeSeqName);
	acdInTypeFeatSave(NULL);
    }
    else
    {
	(void) ajStrAssS (&acdInTypeSeqName, intype);
	(void) ajStrToLower(&acdInTypeSeqName);
	if (ajSeqTypeIsAny(intype))
	    acdInTypeFeatSaveC("");
	else if (ajSeqTypeIsProt(intype))
	    acdInTypeFeatSaveC("protein");
	else if (ajSeqTypeIsNuc(intype))
	    acdInTypeFeatSaveC("nucleotide");
	else
	    acdInTypeFeatSaveC("");
   }
 
    acdLog ("acdInTypeSeqSave (%S) input type set to '%S'\n",
	    intype, acdInTypeSeqName);

    return ajTrue;
}

/* @funcstatic acdInTypeSeq **************************************************
**
** Returns the input sequence type (if known)
**
** @param [wP] infname [AjPStr*] Input sequence type
** @return [AjBool] ajTrue if a type was successfully set
** @@
******************************************************************************/

static AjBool acdInTypeSeq (AjPStr* typename)
{
    AjBool ret = ajFalse;

    acdLog("acdInTypeSeq saved acdInTypeSeqName '%S'\n", acdInTypeSeqName);

    if (acdInTypeSeqName)			/* could be an empty string */
    {
	(void) ajStrAssS (typename, acdInTypeSeqName);
	ret = ajTrue;
    }
    else
    {
	(void) ajStrAssC (typename, ""); /* allow anything, return ajFalse */
	ret = ajFalse;
    }

    return ret;
}

/* @funcstatic acdInTypeFeatSave **********************************************
**
** Saves the input feature type for use in setting the default output type
**
** @param [P] intype [AjPStr] Input feature type
** @return [AjBool] ajTrue if a type was successfully set
** @@
******************************************************************************/

static AjBool acdInTypeFeatSave (AjPStr intype)
{
    if (acdInTypeFeatName)
	return ajFalse;

    acdLog ("acdInTypeFeatSave (%S)\n",
	    intype);

    if (!ajStrLen(intype))
    {
	ajStrAssC (&acdInTypeFeatName, "");
	acdLog("Input feature type defaults to '%S'\n", acdInTypeFeatName);
    }
    else
	(void) ajStrAssS (&acdInTypeFeatName, intype);

    (void) ajStrToLower(&acdInTypeFeatName);

    acdLog ("acdInTypeFeatSave (%S) input feature type set to '%S'\n",
	    intype, acdInTypeFeatName);

    return ajTrue;
}

/* @funcstatic acdInTypeFeatSaveC *********************************************
**
** Saves the input feature type for use in setting the default output type
**
** @param [P] intype [char*] Input feature type
** @return [AjBool] ajTrue if a type was successfully set
** @@
******************************************************************************/

static AjBool acdInTypeFeatSaveC (char* intype)
{
    AjBool ret;
    AjPStr tmpstr = NULL;

    tmpstr = ajStrNewC(intype);
    ret = acdInTypeFeatSave(tmpstr);
    ajStrDel(&tmpstr);

    return ret;
}

/* @funcstatic acdInTypeFeat **************************************************
**
** Returns the input feature type (if known)
**
** @param [wP] typename [AjPStr*] Input feature type
** @return [AjBool] ajTrue if a type was successfully set
** @@
******************************************************************************/

static AjBool acdInTypeFeat (AjPStr* typename)
{
    AjBool ret = ajFalse;

    acdLog("acdInTypeFeat saved acdInTypeFeatName '%S'\n", acdInTypeFeatName);

    if (acdInTypeFeatName)		/* could be an empty string */
    {
	(void) ajStrAssS (typename, acdInTypeFeatName);
	ret = ajTrue;
    }
    else
    {
	(void) ajStrAssC (typename, ""); /* allow anything, return ajFalse */
	ret = ajFalse;
    }

    return ret;
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

static void acdLog (char *fmt, ...)
{
    va_list args ;

    if (!acdDoLog)
	return;

    if (!acdLogFName)
    {
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

static void acdPretty (char *fmt, ...)
{
    va_list args ;
    static AjPStr tmpstr = NULL;

    if (!acdDoPretty)
	return;

    if (!acdPrettyFName)
    {
	if (acdStdout)
	{
	    acdPrettyFile = ajFileNewF (stdout);
	}
	else
	{
	    (void) ajFmtPrintS (&acdPrettyFName, "%S.acdpretty", acdProgram);
	    acdPrettyFile = ajFileNewOut (acdPrettyFName);
	    ajFileUnbuffer(acdPrettyFile);
	}
    }


    va_start (args, fmt);
    ajFmtVPrintS (&tmpstr, fmt, args);
    va_end (args);

    while (ajStrChar(tmpstr, 0) == '\n')
    {
	ajFmtPrintF (acdPrettyFile, "\n");
	ajStrTrim (&tmpstr, 1);
    }

    if (acdPrettyMargin)
	ajFmtPrintF(acdPrettyFile, "%.*s", acdPrettyMargin,
		    "                                                       ");
    ajFmtPrintF (acdPrettyFile, "%S", tmpstr);

    return;
}

/* @funcstatic acdPrettyWrap **************************************************
**
** Writes a pretty formatted version of the .acd syntax
** lien to the .acdpretty file
**
** @param [r] left [ajint] Extra left margin for follow-on lines
** @param [r] fmt [char*] Format with ajFmt extensions
** @param [v] [...] Optional arguments
** @return [void]
** @@
******************************************************************************/

static void acdPrettyWrap (ajint left, char *fmt, ...)
{
    va_list args ;
    static AjPStr tmpstr = NULL;
    ajint leftmargin = left + acdPrettyMargin;
    ajint width = 78 - leftmargin;

    if (!acdDoPretty)
	return;

    if (!acdPrettyFName)
    {
	if (acdStdout)
	{
	    acdPrettyFile = ajFileNewF (stdout);
	}
	else
	{
	    (void) ajFmtPrintS (&acdPrettyFName, "%S.acdpretty", acdProgram);
	    acdPrettyFile = ajFileNewOut (acdPrettyFName);
	    ajFileUnbuffer(acdPrettyFile);
	}
    }

    va_start (args, fmt);
    ajFmtVPrintS (&tmpstr, fmt, args);
    va_end (args);

    ajStrSubstituteCC (&tmpstr, " \\ ", " \\\n"); /* force newlines at '\' */

    ajStrWrapLeft (&tmpstr, width, leftmargin);
    if (acdPrettyMargin)
	ajFmtPrintF(acdPrettyFile, "%.*s", acdPrettyMargin,
		    "                                                       ");
    ajFmtPrintF (acdPrettyFile, "%S\n", tmpstr);

    return;
}

/* @funcstatic acdPrettyShift *************************************************
**
** Right shifts (indents) acdpretty printing
**
** @return [void]
** @@
******************************************************************************/

static void acdPrettyShift ()
{
    acdPrettyMargin += acdPrettyIndent;

    return;
}

/* @funcstatic acdPrettyUnShift ***********************************************
**
** Left shifts acdpretty printing
**
** @return [void]
** @@
******************************************************************************/

static void acdPrettyUnShift ()
{
    acdPrettyMargin -= acdPrettyIndent;
    if (acdPrettyMargin < 0)
    {
	ajWarn ("acdpretty printing indent error - too many left shifts");
	acdPrettyMargin = 0;
    }

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

static AjBool acdIsQtype (AcdPAcd thys)
{
    if ((thys->Level == ACD_QUAL) || (thys->Level == ACD_PARAM))
	return ajTrue;

    return ajFalse;
}

/* @funcstatic acdIsStype *****************************************************
**
** Tests whether an ACD object is a section or endsection type.
**
** @param [r] thys [AcdPAcd] ACD object
** @return [AjBool] ajTrue if the object is a qualifier or parameter
** @@
******************************************************************************/

static AjBool acdIsStype (AcdPAcd thys)
{
    if ((thys->Level == ACD_SEC) || (thys->Level == ACD_ENDSEC))
	return ajTrue;

    return ajFalse;
}

/* @funcstatic acdTextFormat **************************************************
**
** Converts backslash codes in a string into special characters
**
** @param [r] text [AjPStr*] Text with backslash codes
** @return [AjBool] ajTrue if successful
** @@
******************************************************************************/

static AjBool acdTextFormat (AjPStr* text)
{
    (void) ajStrSubstituteCC (text, " \\ ", "\n");

    return ajTrue;
}


/* @func ajAcdDummyFunction ***************************************************
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

    acdSetXxxx(acdpacd);	    /* template function for acdSet */
    (void) acdQualToFloat(acdpacd, "", 0.0, 0, &f, &ajpstr);
}

/* @func ajAcdPrintType *******************************************************
**
** Report details of all known ACD types.
** For use by EMBOSS entrails.
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report
** @return [void]
**
******************************************************************************/

void ajAcdPrintType (AjPFile outf, AjBool full)
{
    AcdPType pat;
    AcdPAttr attr;
    AcdPQual qual;
    ajint i;
    
    ajFmtPrintF (outf, "\n");
    ajFmtPrintF (outf, "# ACD Types\n");
    ajFmtPrintF (outf, "# Name\n");
    ajFmtPrintF (outf, "#     Attribute    Type       Comment\n");
    ajFmtPrintF (outf, "#     Qualifier    Type       Default Helptext\n");
    ajFmtPrintF (outf, "AcdType {\n");
    
    for (i=0; acdType[i].Name; i++) 
    {
	pat = &acdType[i];
	ajFmtPrintF (outf, "  %-15s", pat->Name);
	ajFmtPrintF (outf, "  %-10s", pat->Group);
	ajFmtPrintF (outf, " \"%s\"", pat->Valid);
	ajFmtPrintF (outf, "\n");
	if (full && pat->Attr)
	{
	    ajFmtPrintF (outf, "    attributes {\n");
	    for (attr=pat->Attr; attr->Name; attr++)
	    {
		ajFmtPrintF (outf, "      %-12s", attr->Name);
		ajFmtPrintF (outf, " %-10s", acdValNames[attr->Type]);
		ajFmtPrintF (outf, " \"%s\"", attr->Help);
		ajFmtPrintF (outf, "\n");
	    }
	    ajFmtPrintF (outf, "    }\n");
	}
	if (pat->Quals)
	{
	    ajFmtPrintF (outf, "    qualifiers {\n");
	    for (qual=pat->Quals; qual->Name; qual++)
	    {
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
    
    ajFmtPrintF (outf, "# ACD Default attributes\n");
    ajFmtPrintF (outf, "# Name   Type   Comment\n");
    for (i=0; acdAttrDef[i].Name; i++)  {
	ajFmtPrintF (outf, "  %-15s", acdAttrDef[i].Name);
	ajFmtPrintF (outf, "  %-10s", acdValNames[acdAttrDef[i].Type]);
	ajFmtPrintF (outf, " \"%s\"", acdAttrDef[i].Help);
	ajFmtPrintF (outf, "\n");
    }
    
    ajFmtPrintF (outf, "# ACD Calculated attributes\n");
    ajFmtPrintF (outf, "# Name\n");
    ajFmtPrintF (outf, "#     Attribute    Type       Comment\n");
    
    acdPrintCalcAttr (outf, full, "features", acdCalcFeat);
    acdPrintCalcAttr (outf, full, "regexp", acdCalcRegexp);
    acdPrintCalcAttr (outf, full, "sequence", acdCalcSeq);
    acdPrintCalcAttr (outf, full, "seqall", acdCalcSeqall);
    acdPrintCalcAttr (outf, full, "seqset", acdCalcSeqset);
    acdPrintCalcAttr (outf, full, "string", acdCalcString);
    
    return;
}

/* @funcstatic acdPrintCalcAttr ***********************************************
**
** Report calculated attributes set
** For use by EMBOSS entrails.
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report
** @param [r] acdtype [char*] ACD type name
** @param [r] calcattr [AcdOAttr[]] Acd calculated attributes
** @return [void]
**
******************************************************************************/

static void acdPrintCalcAttr (AjPFile outf, AjBool full,
			      char* acdtype, AcdOAttr calcattr[])
{
    ajint i;

    ajFmtPrintF (outf, "  %-15s",acdtype);
    ajFmtPrintF (outf, "\n");
    if (full && calcattr[0].Name)
    {
	ajFmtPrintF (outf, "    attributes {\n");
	for (i=0; calcattr[i].Name; i++)
	{
	    ajFmtPrintF (outf, "      %-12s", calcattr[i].Name);
	    ajFmtPrintF (outf, " %-10s", acdValNames[calcattr[i].Type]);
	    ajFmtPrintF (outf, " \"%s\"", calcattr[i].Help);
	    ajFmtPrintF (outf, "\n");
	}
	ajFmtPrintF (outf, "    }\n");
    }
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

static AjBool acdVocabCheck (AjPStr str, char** vocab)
{
    ajint i=0;
    while (vocab[i])
    {
	if (ajStrMatchCaseC(str, vocab[i]))
	    return ajTrue;
	i++;
    }

    return ajFalse;
}

/* @funcstatic acdError *******************************************************
**
** Formatted write as an error message, then exits with ajExitBad
**
** @param [P] fmt [char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void acdError (char* fmt, ...)
{
    va_list args ;
    AjPStr errstr=NULL;
    ajint linenum;
    acdErrorCount++;

    va_start (args, fmt) ;
    ajFmtVPrintS (&errstr, fmt, args);
    va_end (args) ;

    if (acdLineNum > 0)
	linenum = acdLineNum;
    else if (acdSetCurr)
	linenum = acdSetCurr->LineNum;
    else if (acdProcCurr)
	linenum = acdProcCurr->LineNum;
    else if (acdListCurr)
	linenum = acdListCurr->LineNum;
    else
	linenum = 0;

    ajErr ("File %S line %d: %S", acdFName, linenum, errstr);
    ajStrDel(&errstr);
    ajExitBad ();

    return;
}

/* @funcstatic acdErrorValid **************************************************
**
** Formatted write as an error message, then continues (acdError exits)
**
** @param [P] fmt [char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void acdErrorValid (char* fmt, ...)
{
    va_list args ;
    AjPStr errstr=NULL;
    ajint linenum;
    acdErrorCount++;

    va_start (args, fmt) ;
    ajFmtVPrintS (&errstr, fmt, args);
    va_end (args) ;

    if (acdLineNum > 0)
	linenum = acdLineNum;
    else if (acdSetCurr)
	linenum = acdSetCurr->LineNum;
    else if (acdProcCurr)
	linenum = acdProcCurr->LineNum;
    else if (acdListCurr)
	linenum = acdListCurr->LineNum;
    else
	linenum = 0;

    ajErr ("File %S line %d: %S", acdFName, linenum, errstr);
    ajStrDel(&errstr);

    return;
}

/* @funcstatic acdWarn *******************************************************
**
** Formatted write as an error message.
**
** @param [P] fmt [char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void acdWarn (char* fmt, ...)
{
    va_list args ;
    AjPStr errstr=NULL;
    ajint linenum;
    acdErrorCount++;

    va_start (args, fmt) ;
    ajFmtVPrintS (&errstr, fmt, args);
    va_end (args) ;

    if (acdLineNum > 0)
	linenum = acdLineNum;
    else if (acdSetCurr)
	linenum = acdSetCurr->LineNum;
    else if (acdProcCurr)
	linenum = acdProcCurr->LineNum;
    else if (acdListCurr)
	linenum = acdListCurr->LineNum;
    else
	linenum = 0;

    ajWarn ("File %S line %d: %S", acdFName, linenum, errstr);
    ajStrDel(&errstr);

    return;
}

/* @funcstatic acdErrorAcd ****************************************************
**
** Formatted write as an error message, for a specified ACD object
**
** @param [R] thys [AcdPAcd] ACD object
** @param [P] fmt [char*] Format string
** @param [v] [...] Format arguments.
** @return [void]
** @@
******************************************************************************/

static void acdErrorAcd (AcdPAcd thys, char* fmt, ...)
{
    va_list args ;
    AjPStr errstr=NULL;
    acdErrorCount++;

    va_start (args, fmt) ;
    ajFmtVPrintS (&errstr, fmt, args);
    va_end (args) ;

    ajErr ("File %S line %d: (%S) %S",
	   acdFName, thys->LineNum, thys->Name, errstr);
    ajStrDel(&errstr);
    ajExitBad ();

    return;
}

/* @func ajAcdExit ************************************************************
**
** Reports any unused ACD values
**
** Cleans up feature table internal memory
**
** @param [R] silent [AjBool] Turn off messages (used when some messages
**                            are expected but can be ignored).
** @return [void]
** @@
******************************************************************************/

void ajAcdExit (AjBool silent)
{

    AcdPAcd pa;
    static AjBool staySilent=AJFALSE;

    if (silent) staySilent = ajTrue;
    if (acdDoHelp) staySilent = ajTrue;
    if (acdDoPretty) staySilent = ajTrue;

    if (!staySilent)
    {
	/* turn off the warnings for now ... comment out this line to
	 ** enable them. 
	 ** the problem is that some programs have conditionals around the
	 ** ajAcdGet calls so they can sometimes fail to use values, even where
	 ** the test cases say they are fine.
	 ** also some ACD files (showdb for example) use values,
	 ** but in some cases (showdb with all booleans on the command line)
	 ** the value is not needed
	 */

	/*
	   if (!staySilent) return;
	   */

	for (pa=acdList; pa; pa=pa->Next)
	{
	    if (pa->Assoc)
		continue;
	    if (pa->Level != ACD_PARAM && pa->Level != ACD_QUAL)
		continue;
	    if (!pa->Used)
	    {
		acdLog ("ACD qualifier never used: %S = '%S' (assoc %B)",
			pa->Name, pa->ValStr, pa->Assoc);
	    }

	}
    }

    return;
}

/*
 *  In case people want -errorlevel=warning,noerror,fatal
 *
//static void acdParseErrorLevel(AjPStr str)
//{
//    AjPStr *args;
//    ajint  nargs;
//    ajint  i;
//    ajint j;
//
//    AjPStr token = NULL;
//    AjBool tval = ajTrue;
//    static char **errors =
//    {
//	"warning", "error", "fatal", NULL
//    };
//    ajint ecount;
//    ajint idx = -1;
//
//    token = ajStrNew();
//
//    nargs = ajArrCommaList(str,&args);
//    for(i=0;i<nargs;++i)
//    {
//	tval = ajTrue;
//	ajStrToLower(&args[i]);
//	if(ajStrPrefixC(args[i],"no"))
//	{
//	    tval = ajFalse;
//	    ajStrAssC(&token,ajStrStr(args[i])+2);
//	}
//	else
//	    ajStrAssS(&token,args[i]);
//
//
//	for(j=0;errors[j],++j)
//	{
//	    if(!strncmp(errors[j],ajStrStr(token),ajStrLen(token)))
//	    {
//		++ecount;
//		idx = j;
//	    }
//	}
//
//	if(ecount > 1)
//	    fprintf(stderr,"Ambiguous error level value [%s]\n",
//		    ajStrStr(args[i]));
//	if(idx == -1)
//	    fprintf(stderr,"Unrecognised error level value [%s]\n",
//		    ajStrStr(args[i]));
//
//
//	switch(idx)
//	{
//	case 0:
//	    AjErrorLevel.warning = tval;
//	    break;
//	case 1:
//	    AjErrorLevel.error   = tval;
//	    break;
//	case 2:
//	    AjErrorLevel.fatal   = tval;
//	    break;
//	default:
//	    fprintf(stderr,"acdParseErrorLevel switch error\n");
//	    exit(0);
//	}
//    }
//
//
//    for(i=0;i<nargs;++i)
//	ajStrDel(&args[i]);
//    AJFREE(args);
//    ajStrDel(&token);
//
//    return;
//}
*/

/* @funcstatic acdValidAppl ***************************************************
**
** Validation for an application definition
**
** @param [R] thys [AcdPAcd] ACD object
** @return [void]
** @@
******************************************************************************/

static void acdValidAppl (AcdPAcd thys)
{
    ajint i;

    if (!acdDoValid)
	return;

    /* must have a documentation attribute */

    i = acdFindAttrC (acdAttrAppl, "documentation");
    if (!ajStrLen(thys->AttrStr[i]))
	acdErrorValid ("Application has no documentation defined");

    /* must have a group attribute */

    i = acdFindAttrC (acdAttrAppl, "groups");
    if (!ajStrLen(thys->AttrStr[i]))
	acdErrorValid ("Application has no groups defined");

    /* group must be a known group (and subgroup) */

    acdValidApplGroup(thys->AttrStr[i]);

    return;
}

/* @funcstatic acdValidSection ************************************************
**
** Validation for a section definition
**
** @param [R] thys [AcdPAcd] ACD object
** @return [void]
** @@
******************************************************************************/

static void acdValidSection (AcdPAcd thys)
{
    AjPTable typeTable = NULL;
    AjPTable infoTable = NULL;
    static ajint sectLevel=0;
    AjPStr sectType = NULL;
    AjPStr sectInfo = NULL;
    static AjPStr sectNameTop;
    AjPStr tmpstr = NULL;

    if (!acdDoValid)
	return;

    if (!typeTable)
	acdReadSections(&typeTable, &infoTable);

    if (ajStrMatchCC(acdKeywords[thys->Type].Name, "endsection"))
    {
	--sectLevel;
	if (sectLevel < 0)
	    acdErrorValid("too many endsections");
	return;
    }

    ++sectLevel;

    if (sectLevel == 1) {
	ajStrAssS (&sectNameTop, thys->Name);
    }

   /* should have a known name */

    sectType = ajTableGet(typeTable, thys->Name);
    tmpstr = acdAttrValue(thys, "type");

    if (!sectType)
    {
	if (sectLevel == 1)
	    acdErrorValid("Section '%S' not defined in sections.standard file",
		thys->Name);
	else
	    acdWarn("Sub level section '%S' not defined in "
		    "sections.standard file",
		    thys->Name);
	/* if unknown, must be distinctive - check for common words? */

	return;
    }

    else
    {

    /* if known, must have a standard type */

	if (sectLevel == 1)
	{
	    if (!ajStrMatchC(tmpstr, "page"))
		acdErrorValid("Top level section '%S' not of type 'page'",
			      thys->Name);

	    else if (!ajStrMatchC(sectType, "page"))
		acdErrorValid("Top level section '%S' defined as "
			      "sub type of '%S' in sections.standard",
			      thys->Name, sectType);
	}
	else
	{
	    if (ajStrMatchC(tmpstr, "page"))
		acdErrorValid("Sub level section '%S' not of type 'frame'",
			      thys->Name);
	    else if (ajStrPrefixC(sectType, "page"))
		acdErrorValid("Sub level section '%S' not defined "
			      "as type 'frame' in sections.standard",
			      thys->Name);
	    else if (!ajStrMatch(sectType, sectNameTop))
		acdErrorValid("Sub level section '%S' should be under '%S'",
			      thys->Name, sectType);
	}
    }

    sectInfo = ajTableGet(infoTable, thys->Name);
    tmpstr = acdAttrValue(thys, "information");

    /* must have an information attribute */

    if (!ajStrLen(tmpstr))
	acdWarn("Section has no information string");

     /* if known, must have a standard info string */

   else
    {
	if (!ajStrMatch(sectInfo, tmpstr))
	{
	    if (ajStrMatchCase(sectInfo, tmpstr))
		acdWarn("Section info '%S' expected, case mismatch",
			sectInfo);
	    else
		acdErrorValid("Section info '%S' expected, found '%S'",
			      sectInfo, tmpstr);
	}
    }


}

/* @funcstatic acdValidQual ************************************************
**
** Validation for a qualifier definition
**
** @param [R] thys [AcdPAcd] ACD object
** @return [void]
** @@
******************************************************************************/

static void acdValidQual (AcdPAcd thys)
{
    ajint itype;
    AjBool isparam = ajFalse;
    AjBool boolval;
    static AjPStr tmpstr = NULL;
    static AjPStr tmpinfo = NULL;
    static AjPStr tmpprompt = NULL;
    static ajint qualCountSeq = 0;
 /*   static ajint qualCountSeqall = 0; */
 /*   static ajint qualCountSeqset = 0; */
    static ajint qualCountSeqout = 0;
/*    static ajint qualCountSeqoutall = 0; */
 /*   static ajint qualCountSeqoutset = 0; */
    static ajint qualCountInfile = 0;
    static ajint qualCountOutfile = 0;
    static AjPStr qualName = NULL;
    static AjPStr qualSeqFirst = NULL;
    static AjPStr qualSeqoutFirst = NULL;
    static AjPStr seqTypeIn = NULL;
    static AjBool seqMulti = AJFALSE;
    static AjBool inMulti = AJFALSE;
    static AjBool outMulti = AJFALSE;
    static AjBool seqoutMulti = AJFALSE;

    if (!acdDoValid)
	return;

    /* parameter, required, optional only once, with 'Y' */

    itype = 0;

    tmpstr = acdAttrValue(thys, "parameter");
    if (ajStrLen(tmpstr))
    {
	itype++;
	if (acdVarTest(tmpstr))
	{
	    acdErrorValid ("Calculated parameter value");
	}
	else
	{
	    if (ajStrToBool(tmpstr,&isparam))
	    {
		if (!isparam)
		{
		    acdErrorValid ("parameter defined as false '%S'", tmpstr);
		    itype--;
		}
	    }
	}
    }

    tmpstr = acdAttrValue(thys, "required");
    if (ajStrLen(tmpstr))
    {
	itype++;
	if (acdVarTest(tmpstr))
	{
	    acdWarn ("Calculated required value");
	}
	else
	{
	    if (ajStrToBool(tmpstr,&boolval))
	    {
		if (!boolval)
		{
		    acdErrorValid ("required defined as false '%S'", tmpstr);
		    itype--;
		}
	    }
	}
    }

    tmpstr = acdAttrValue(thys, "optional");
    if (ajStrLen(tmpstr))
    {
	itype++;
	if (acdVarTest(tmpstr))
	{
	    acdWarn ("Calculated optional value");
	}
	else
	{
	    if (ajStrToBool(tmpstr,&boolval))
	    {
		if (!boolval)
		{
		    acdErrorValid ("optional defined as false '%S'", tmpstr);
		    itype--;
		}
	    }
	}
    }

    if (itype > 1)
    {
	acdErrorValid("Multiple definition of parameter/required/optional");
    }

    tmpinfo = acdAttrValue(thys, "information");
    tmpprompt = acdAttrValue(thys, "prompt");
    if (ajStrLen(tmpprompt) && !ajStrLen(tmpinfo))
    {
	acdErrorValid("prompt specified but no information");
    }

    /* if known, must have a standard info string */

    if (acdType[thys->Type].Stdprompt &&
	(ajStrLen(tmpinfo) || ajStrLen(tmpprompt)))
    {
	acdErrorValid("Unexpected information value for type '%s'",
	       acdType[thys->Type].Name);
    }

    /* else it must have an info attribute */

    if (!acdType[thys->Type].Stdprompt &&
	!ajStrLen(tmpinfo) && !ajStrLen(tmpprompt))
    {
	acdErrorValid("Missing information value for type '%s'",
	       acdType[thys->Type].Name);
    }

    /* if known, must have a standard type */

    /* expected types should have a known name */
/* sequence seqset seqall see below - need to do seqout, infile, outfile */

    if (ajStrMatchCC(acdType[thys->Type].Name, "sequence") ||
	ajStrMatchCC(acdType[thys->Type].Name, "seqall") ||
	ajStrMatchCC(acdType[thys->Type].Name, "seqset"))
    {
	if (!isparam)
	    acdWarn("sequence input is not a parameter");
	qualCountSeq++;
	if (qualCountSeq == 1)
	    ajStrAssS (&qualSeqFirst, thys->Token);
	ajFmtPrintS(&qualName, "%csequence",
		    (char) ('a' - 1 + qualCountSeq));
	if (!(ajStrSuffixC(thys->Token, "sequence") ||
	      (ajStrMatchCC(acdType[thys->Type].Name, "seqall") &&
	       ajStrSuffixC(thys->Token, "seqall"))))
	{
	    acdWarn("sequence qualifier '%S' is not 'sequence' or '*sequence' "
		    "or 'seqall'",
		    thys->Token);
	}
	else
	{
	    if ((qualCountSeq > 1) ||
		!(ajStrMatchC(thys->Token, "sequence") ||
		  (ajStrMatchCC(acdType[thys->Type].Name, "seqall") &&
		   ajStrSuffixC(thys->Token, "seqall")) ))
		seqMulti = ajTrue;
	    if (seqMulti)
	    {
		if (!ajStrMatch(thys->Token, qualName))
		    acdWarn("Expected sequence qualifier is '%S' found '%S'",
			    qualName, thys->Token);
	    }
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "type");
	if (!ajStrLen(tmpstr))
	    acdErrorValid ("No type specified for input sequence");
	else
	    if (qualCountSeq == 1)
		ajStrAssS(&seqTypeIn, tmpstr);

   }

    /* infile - assume parameter  -infile */
    /* check for type */

    if (ajStrMatchCC(acdType[thys->Type].Name, "infile") ||
	ajStrMatchCC(acdType[thys->Type].Name, "filelist"))
    {
	if (!isparam)
	    acdWarn("input file is not a parameter");
	qualCountInfile++;
	if (ajStrMatchCC(acdType[thys->Type].Name, "infile"))
	{
	    if (!ajStrSuffixC(thys->Token, "file"))
		acdWarn("infile qualifier '%S' is not 'infile' or '*file'",
			thys->Token);
	}
	else if (ajStrMatchCC(acdType[thys->Type].Name, "filelist"))
	{
	    if (!ajStrSuffixC(thys->Token, "files"))
		acdWarn("filelist qualifier '%S' is not '*files'",
			thys->Token);
	}
	else
	{
	    if ((qualCountInfile > 1) ||
		!ajStrMatchC(thys->Token, "infile"))
		inMulti = ajTrue;
	    if ((qualCountInfile == 1) &&
		!ajStrMatchC(thys->Token, "infile"))
		acdWarn ("First input file qualifier is not 'infile'");
	}

	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "standardtype");
	if (!ajStrLen(tmpstr))
	    acdWarn ("No standardtype specified for input file");

   }
    /* outfile - assume parameter unless default is stdout -outfile */
    /* check for type */

    if (ajStrMatchCC(acdType[thys->Type].Name, "outfile"))
    {
	if (!isparam)
	    acdWarn("output file is not a parameter");
	qualCountOutfile++;
	if (!ajStrSuffixC(thys->Token, "file"))
	    acdWarn("outfile qualifier '%S' is not 'outfile' or '*file'",
		    thys->Token);
	else
	{
	    if ((qualCountOutfile > 1) ||
		!ajStrMatchC(thys->Token, "outfile"))
		outMulti = ajTrue;
	    if ((qualCountOutfile == 1) &&
		!ajStrMatchC(thys->Token, "outfile"))
		acdWarn ("First output file qualifier is not 'outfile'");
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "standardtype");
	if (!ajStrLen(tmpstr))
	    acdWarn ("No standardtype specified for output file");
   }

    /* align - as for outfile? */

     if (ajStrMatchCC(acdType[thys->Type].Name, "align"))
    {
	if (!isparam)
	    acdWarn("alignment file is not a parameter");
	qualCountOutfile++;
	if (!ajStrSuffixC(thys->Token, "file"))
	    acdWarn("align qualifier '%S' is not 'outfile' or '*file'",
		    thys->Token);
	else
	{
	    if ((qualCountOutfile > 1) ||
		!ajStrMatchC(thys->Token, "outfile"))
		outMulti = ajTrue;
	    if ((qualCountOutfile == 1) &&
		!ajStrMatchC(thys->Token, "outfile"))
		acdWarn ("First alignment file qualifier is not 'outfile'");
	}
   }

   /* report - as for outfile? */

    if (ajStrMatchCC(acdType[thys->Type].Name, "report"))
    {
	if (!isparam)
	    acdWarn("report file is not a parameter");
	qualCountOutfile++;
	if (!ajStrSuffixC(thys->Token, "file"))
	    acdWarn("report qualifier '%S' is not 'outfile' or '*file'",
		    thys->Token);
	else
	{
	    if ((qualCountOutfile > 1) ||
		!ajStrMatchC(thys->Token, "outfile"))
		outMulti = ajTrue;
	    if ((qualCountOutfile == 1) &&
		!ajStrMatchC(thys->Token, "outfile"))
		acdWarn ("First report file qualifier is not 'outfile'");
	}	
   }

    /* seqout* - assume parameter - what names? -outseq? */
    /* type only if there is no sequence input */

    if (ajStrMatchCC(acdType[thys->Type].Name, "seqout") ||
	ajStrMatchCC(acdType[thys->Type].Name, "seqoutall") ||
	ajStrMatchCC(acdType[thys->Type].Name, "seqoutset"))
    {
	if (!isparam)
	    acdWarn("sequence output is not a parameter");
	qualCountSeqout++;
	if (qualCountSeqout == 1)
	    ajStrAssS (&qualSeqoutFirst, thys->Token);
	ajFmtPrintS(&qualName, "%coutseq",
		    (char) ('a' - 1 + qualCountSeq));

	if (!ajStrSuffixC(thys->Token, "outseq") &&
	    !ajStrSuffixC(thys->Token, "outfile"))
	    acdWarn("sequence output qualifier '%S' is not 'outseq' "
		    "or '*outseq' or 'outfile'",
		    thys->Token);

	else
	{
	    if ((qualCountSeqout > 1) ||
		(!ajStrMatchC(thys->Token, "outseq") &&
		 !ajStrMatchC(thys->Token, "outfile")))
		seqoutMulti = ajTrue;
	    if (seqoutMulti)
	    {
		if (!ajStrMatch(thys->Token, qualName))
		    acdWarn("Expected sequence output qualifier is '%S' "
			    "found '%S'",
			    qualName, thys->Token);
	    }
	}
	
	/* still to do - check for type */

	tmpstr = acdAttrValue(thys, "type");
	if (!ajStrLen(tmpstr) && !ajStrLen(seqTypeIn))
	    acdErrorValid ("No type specified for output sequence, "
			   "and no input sequence type as a default");

   }

    return;
}

/* @funcstatic acdValidApplGroup **********************************************
**
** Validation for application groups
**
** @param [R] groups [AjPStr] Group name(s)
** @return [void]
** @@
******************************************************************************/

static void acdValidApplGroup (AjPStr groups)
{
    static AjPTable grpTable = NULL;
    AjPRegexp grpexp = NULL;
    AjPStr tmpGroups = NULL;
    AjPStr grpName = NULL;
    AjPStr grpDesc = NULL;

    if (!acdDoValid) return;

    grpTable = acdReadGroups();

    ajStrAssS (&tmpGroups, groups);

    /* step through each group */
    grpexp = ajRegCompC("([^,]+),?");

    while (ajRegExec(grpexp, tmpGroups))
    {
	ajRegSubI(grpexp, 1, &grpName);
	ajStrClean(&grpName);
	grpDesc = ajTableGet(grpTable, grpName);
	if (!grpDesc)
	    acdErrorValid("Unknown group '%S' for application", grpName);
	ajRegPost(grpexp, &tmpGroups);
    }

    return;
}

/* @funcstatic acdReadGroups **************************************************
**
** Read standard table of application groups
**
** @return [AjPTable] String table of group names and descriptions
** @@
******************************************************************************/

static AjPTable acdReadGroups (void)
{
    AjPTable ret = ajStrTableNewCase(50);

    AjPFile grpFile = NULL;
    AjPStr grpFName = NULL;
    AjPStr grpRoot = NULL;
    AjPStr grpRootInst = NULL;
    AjPStr grpPack = NULL;
    AjPStr grpLine = NULL;
    AjPRegexp grpxp = NULL;
    AjPStr grpName = NULL;
    AjPStr grpDesc = NULL;

    (void) ajNamRootPack (&grpPack);
    (void) ajNamRootInstall (&grpRootInst);
    (void) ajFileDirFix (&grpRootInst);
    
    if (ajNamGetValueC ("acdroot", &grpRoot))
    {
	(void) ajFileDirFix (&grpRoot);
	ajFmtPrintS (&grpFName, "%Sgroups.standard", grpRoot);
	grpFile = ajFileNewIn (grpFName);
	acdLog ("Group file in acdroot: '%S'\n", grpFName);
    }
    else
    {
	ajFmtPrintS (&grpFName, "%Sshare/%S/acd/groups.standard",
		     grpRootInst, grpPack);
	acdLog ("Group file installed: '%S'\n", grpFName);
	grpFile = ajFileNewIn (grpFName);
	if (!grpFile)
	{
	    acdLog ("Grp file '%S' not opened\n", grpFName);
	    (void) ajNamRoot (&grpRoot);
	    (void) ajFileDirFix (&grpRoot);
	    ajFmtPrintS (&grpFName, "%Sacd/groups.standard", grpRoot);
	    acdLog ("Grp file from source dir: '%S'\n", grpFName);
	    grpFile = ajFileNewIn (grpFName);
	}
    }
    
    if (!grpFile)		/* test acdc-grpmissing */
	ajDie ("Group file %S not found", grpFName);
    else
	acdLog ("Group file %F used\n", grpFile);
    
    grpxp = ajRegCompC ("([^ ]+) +([^ ].*)");
    while (grpFile && ajFileReadLine(grpFile, &grpLine))
    {
	if (ajStrUncomment(&grpLine))
	{
	    ajStrClean (&grpLine);

	    if (ajRegExec(grpxp, grpLine))
	    {
		ajRegSubI (grpxp, 1, &grpName);
		ajRegSubI (grpxp, 2, &grpDesc);
		ajStrSubstituteKK(&grpName, '_', ' ');
		if (ajTablePut (ret, grpName, grpDesc))
		    ajWarn ("Duplicate group name in file %S",
			    grpFName);
		grpName = NULL;
		grpDesc = NULL;
	    }
	    else
		ajErr ("Bad record in file %S:\n%S",
		       grpFName, grpLine);
	}
    }
    ajFileClose (&grpFile);

    return ret;
}
    
/* @funcstatic acdReadGroupSections *******************************************
**
** Read standard table of ACD sections
**
** @param [C] typetable [AjPTable] String table of section names and types
** @param [C] infotable [AjPTable] String table of section names and
**                                 descriptions
** @return [void]
** @@
******************************************************************************/

static void acdReadSections (AjPTable* typetable, AjPTable* infotable)
{

    AjPFile sectFile = NULL;
    AjPStr sectFName = NULL;
    AjPStr sectRoot = NULL;
    AjPStr sectRootInst = NULL;
    AjPStr sectPack = NULL;
    AjPStr sectLine = NULL;
    AjPRegexp sectxp = NULL;
    AjPStr sectName = NULL;
    AjPStr sectType = NULL;
    AjPStr sectDesc = NULL;

    (void) ajNamRootPack (&sectPack);
    (void) ajNamRootInstall (&sectRootInst);
    (void) ajFileDirFix (&sectRootInst);
    
    *typetable = ajStrTableNewCase(50);
    *infotable = ajStrTableNewCase(50);

    if (ajNamGetValueC ("acdroot", &sectRoot))
    {
	(void) ajFileDirFix (&sectRoot);
	ajFmtPrintS (&sectFName, "%Ssections.standard", sectRoot);
	sectFile = ajFileNewIn (sectFName);
	acdLog ("Section file in acdroot: '%S'\n", sectFName);
    }
    else
    {
	ajFmtPrintS (&sectFName, "%Sshare/%S/acd/sections.standard",
		     sectRootInst, sectPack);
	acdLog ("Section file installed: '%S'\n", sectFName);
	sectFile = ajFileNewIn (sectFName);
	if (!sectFile)
	{
	    acdLog ("Sect file '%S' not opened\n", sectFName);
	    (void) ajNamRoot (&sectRoot);
	    (void) ajFileDirFix (&sectRoot);
	    ajFmtPrintS (&sectFName, "%Sacd/sections.standard", sectRoot);
	    acdLog ("Sect file from source dir: '%S'\n", sectFName);
	    sectFile = ajFileNewIn (sectFName);
	}
    }
    
    if (!sectFile)		/* test acdc-sectmissing */
	ajDie ("Section file %S not found", sectFName);
    else
	acdLog ("Section file %F used\n", sectFile);
    
    sectxp = ajRegCompC ("([^ ]+) +([^ ]+) +([^ ].*)");
    while (sectFile && ajFileReadLine(sectFile, &sectLine))
    {
	if (ajStrUncomment(&sectLine))
	{
	    ajStrClean (&sectLine);

	    if (ajRegExec(sectxp, sectLine))
	    {
		ajRegSubI (sectxp, 1, &sectName);
		ajRegSubI (sectxp, 2, &sectType);
		ajRegSubI (sectxp, 3, &sectDesc);
		ajStrSubstituteKK(&sectName, '_', ' ');
		if (ajTablePut (*typetable, sectName, sectType))
		    ajWarn ("Duplicate section name in file %S",
			    sectFName);
		if (ajTablePut (*infotable, sectName, sectDesc))
		    ajWarn ("Duplicate section name in file %S",
			    sectFName);
		sectName = NULL;
		sectType = NULL;
		sectDesc = NULL;
	    }
	    else
		ajErr ("Bad record in file %S:\n%S",
		       sectFName, sectLine);
	}
    }
    ajFileClose (&sectFile);

    return;
}
    

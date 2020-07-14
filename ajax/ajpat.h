#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajpat_h
#define ajpat_h

/*
 *  Defines for string search algorithms
 */
#define AJALPHA  256			/* Alphabet			*/
#define AJMOD256 0xff
#define AJALPHA2 128			/* ASCII printable		*/
#define AJWORD   32			/* Size of a word		*/
#define AJBPS    1			/* Bits per state		*/

/* @data AjPPatBYPNode *******************************************************
**
** AJAX data structure for nodes in Baeza-Yates & Perleberg algorithm
**
** @attr offset [ajint] Offset
** @attr next [struct AjSPatBYPNode*] Pointer to next node
** @@
******************************************************************************/

typedef struct AjSPatBYPNode
{
    ajint offset;
    struct AjSPatBYPNode *next;
} AjOPatBYPNode;
#define AjPPatBYPNode AjOPatBYPNode*

/* @data AjPPatComp **********************************************************
**
** NUCLEUS data structure that holds all needed datas for compiling and
** searching. Not including mismatch number.
**
** @alias AjSPatComp
** @alias AjOPatComp
**
** @attr pattern [AjPStr] Prosite pattern string
** @attr type [ajint] Prosite pattern compile type
** @attr plen [ajint] Prosite pattern length
** @attr buf [ajint*] Buffer for BMH search
** @attr off [AjOPatBYPNode[AJALPHA]] Offset buffer for B-Y/P search
** @attr sotable [ajuint*] Buffer for SHIFT-OR
** @attr solimit [ajuint] Limit for BMH search
** @attr m [ajint] Real length of pattern (from embPatGetType)
** @attr regex [AjPStr] PCRE regexp string
** @attr skipm [ajint**] Skip buffer for Tarhio-Ukkonen
** @attr amino [AjBool] Must match left begin
** @attr carboxyl [AjBool] Must match right
**
** @@
******************************************************************************/

typedef struct AjSPatComp
{
    AjPStr pattern;
    ajint type;
    ajint plen;
    ajint* buf;
    AjOPatBYPNode off[AJALPHA];
    ajuint* sotable;
    ajuint solimit;
    ajint m;
    AjPStr regex;
    ajint** skipm;
    AjBool amino;
    AjBool carboxyl;
} AjOPatComp;
#define AjPPatComp AjOPatComp*

/* @data AjPPatternSeq ********************************************************
**
** Ajax sequence pattern object.
**
** Holds definition of feature pattern. Regular expression patterns ignore
** mismatch value.
**
** @alias AjSPattern
** @alias AjOPattern
**
** @attr Name [AjPStr] Name.
** @attr Pattern [AjPStr] Pattern in string format.
** @attr Compiled [void*] Compiled version of the pattern.
** @attr Protein [AjBool] True if protein pattern
** @attr Mismatch [ajint] Mismatch value.
**
** @new ajPatternSeqNew Default constructor
** @delete ajPatternSeqDel Default destructor
** @use ajPatternSeqGetName Returns name.
** @use ajPatternSeqGetPattern Returns pattern in string format.
** @use ajPatternSeqGetCompiled Returns pointer to compiled pattern.
** @use ajPatternSeqGetType Returns type as integer value.
** @use ajPatternSeqGetMismatch Return mismatch value.
** @@
******************************************************************************/

typedef struct AjSPatternSeq {
  AjPStr Name;
  AjPStr Pattern;
  void* Compiled;
  AjBool Protein;
  ajint Mismatch;
} AjOPatternSeq;

#define AjPPatternSeq AjOPatternSeq*

/* @data AjPPatternRegex ******************************************************
**
** Ajax regular expression pattern object.
**
** Holds definition of feature pattern. Regular expression patterns ignore
** mismatch value.
**
** @alias AjSPattern
** @alias AjOPattern
**
** @attr Name [AjPStr] Name.
** @attr Pattern [AjPStr] Pattern in string format.
** @attr Compiled [AjPRegexp] Compiled version of the pattern.
** @attr Type [ajint] Type.
**
** @new ajPatternRegexNew Default constructor
** @delete ajPatternRegexDel Default destructor
** @use ajPatternRegexGetName Returns name.
** @use ajPatternRegexGetPattern Returns pattern in string format.
** @use ajPatternRegexGetCompiled Returns pointer to compiled pattern.
** @use ajPatternRegexGetType Returns type as integer value.
** @@
******************************************************************************/

typedef struct AjSPatternRegex {
  AjPStr    Name;
  AjPStr    Pattern;
  AjPRegexp Compiled;
  ajint     Type;
} AjOPatternRegex;

#define AjPPatternRegex AjOPatternRegex*

/*
** type can be 0: string, 1: prosite (protein) 2: prosite like (nucleotide)
*/
#define AJ_PAT_TYPE_STRING 0
#define AJ_PAT_TYPE_PRO 1
#define AJ_PAT_TYPE_NUCL 2

/* @data AjPPatlistSeq ********************************************************
**
** Ajax Pattern List object.
**
** Holds list of feature patterns and general information of them.
**
** @alias AjSPatlist
** @alias AjOPatlist
**
** @attr Patlist [AjPList] List for patterns.
** @attr Iter [AjIList] List iterator.
** @attr Protein [AjBool] True if protein
**
** @new ajPatlistNew Default constructor.
** @delete ajPatlistDel Default destructor.
** @modify ajPatlistRegexRead Reads the pattern file and fills the list.
** @modify ajPatlistSeqRead Reads the pattern file and fills the list.
** @modify ajPatlistRewind Restarts the iteration loop.
** @modify ajPatlistAdd Adds new pattern into list.
** @use ajPatlistGetNext Gets the next pattern from file and returns true if
**      available and false if not.
** @@
******************************************************************************/

typedef struct AjSPatlistSeq {
  AjPList Patlist;
  AjIList Iter;
  AjBool Protein;
} AjOPatlistSeq;

#define AjPPatlistSeq AjOPatlistSeq*

/* @data AjPPatlistRegex ******************************************************
**
** Ajax Pattern List object.
**
** Holds list of feature patterns and general information of them.
**
** @alias AjSPatlist
** @alias AjOPatlist
**
** @attr Patlist [AjPList] List for patterns.
** @attr Iter [AjIList] List iterator.
** @attr Type [ajint] Type of expression
**
** @new ajPatlistNew Default constructor.
** @delete ajPatlistDel Default destructor.
** @modify ajPatlistRegexRead Reads the pattern file and fills the list.
** @modify ajPatlistSeqRead Reads the pattern file and fills the list.
** @modify ajPatlistRewind Restarts the iteration loop.
** @modify ajPatlistAdd Adds new pattern into list.
** @use ajPatlistGetNext Gets the next pattern from file and returns true if
**      available and false if not.
** @@
******************************************************************************/

typedef struct AjSPatlistRegex {
  AjPList Patlist;
  AjIList Iter;
  ajint Type;
} AjOPatlistRegex;

#define AjPPatlistRegex AjOPatlistRegex*

/*
** Prototype definitions
*/

AjPPatternSeq ajPatternSeqNewList (AjPPatlistSeq plist, const AjPStr name,
				   const AjPStr pat, ajint mismatch);
void ajPatternSeqDel (AjPPatternSeq* pthys);
const AjPStr ajPatternSeqGetName (const AjPPatternSeq thys);
const AjPStr ajPatternSeqGetPattern (const AjPPatternSeq thys);
AjPPatComp ajPatternSeqGetCompiled (const AjPPatternSeq thys);
AjBool ajPatternSeqGetProtein (const AjPPatternSeq thys);
ajint ajPatternSeqGetMismatch (const AjPPatternSeq thys);
void ajPatternSeqSetCompiled (AjPPatternSeq thys, void* pat);
void ajPatternSeqDebug (const AjPPatternSeq pat);

AjPPatternRegex ajPatternRegexNewList (AjPPatlistRegex plist,
				       const AjPStr name,
				       const AjPStr pat);
void ajPatternRegexDel (AjPPatternRegex* pthys);
const AjPStr ajPatternRegexGetName (const AjPPatternRegex thys);
const AjPStr ajPatternRegexGetPattern (const AjPPatternRegex thys);
AjPRegexp ajPatternRegexGetCompiled (const AjPPatternRegex thys);
ajint ajPatternRegexGetType (const AjPPatternRegex thys);
void ajPatternRegexSetCompiled (AjPPatternRegex thys, AjPRegexp pat);
void ajPatternRegexDebug (const AjPPatternRegex pat);

/* Patlist handling functions */
AjPPatlistSeq ajPatlistSeqNewType (AjBool Protein);
AjPPatlistRegex ajPatlistRegexNewType (ajint type);
AjPPatlistRegex ajPatlistRegexNew (void);
AjPPatlistSeq ajPatlistSeqNew (void);
void ajPatlistSeqDel (AjPPatlistSeq* pthys);
void ajPatlistRegexDel (AjPPatlistRegex* pthys);
AjPPatlistRegex ajPatlistRegexRead(const AjPStr patspec,
				   const AjPStr patname,
				   ajint type, AjBool upper, AjBool lower);
AjPPatlistSeq ajPatlistSeqRead(const AjPStr patspec,
			       const AjPStr patname,
			       AjBool protein, ajint mismatches);
AjBool ajPatlistRegexGetNext (AjPPatlistRegex thys,
			      AjPPatternRegex* pattern);
AjBool ajPatlistSeqGetNext (AjPPatlistSeq thys,
			    AjPPatternSeq* pattern);
void ajPatlistSeqRewind (AjPPatlistSeq thys);
void ajPatlistRegexRewind (AjPPatlistRegex thys);
void ajPatlistSeqRemoveCurrent (AjPPatlistSeq thys);
void ajPatlistRegexRemoveCurrent (AjPPatlistRegex thys);
void ajPatlistAddRegex (AjPPatlistRegex thys, AjPPatternRegex pat);
void ajPatlistAddSeq (AjPPatlistSeq thys, AjPPatternSeq pat);
ajint ajPatlistSeqGetSize(const AjPPatlistSeq plist);
ajint ajPatlistRegexGetSize(const AjPPatlistRegex plist);
ajint ajPatlistRegexDoc(AjPPatlistRegex thys, AjPStr* pdoc);
ajint ajPatlistSeqDoc(AjPPatlistSeq thys, AjPStr* pdoc);

AjPPatComp	ajPPatCompNew (void);
void		ajPPatCompDel (AjPPatComp* pthys);
ajint ajPatternRegexType(const AjPStr type);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

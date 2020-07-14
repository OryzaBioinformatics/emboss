#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embWord_h
#define embWord_h

/* @data EmbPWordMatch ********************************************************
**
** NUCLEUS data structure for word matches
**
** @attr seq1start [ajint] match start point in original sequence
** @attr seq2start [ajint] match start point in comparison sequence
** @attr length [ajint] length of match
** @attr sequence [const AjPSeq] need in case we build multiple matches here
**                         so we know which one the match belongs to
** @@
******************************************************************************/

typedef struct EmbSWordMatch {
  ajint seq1start;
  ajint seq2start;
  ajint length;
  const AjPSeq sequence;
} EmbOWordMatch;
#define EmbPWordMatch EmbOWordMatch*

/* @data EmbPWord *************************************************************
**
** NUCLEUS data structure for words
**
** @attr count [ajint] Size of list
** @attr fword [const char*] Original word
** @attr list [AjPList] List of words
** @@
******************************************************************************/

typedef struct EmbSWord {
  ajint count;
  const char *fword;
  AjPList list;
} EmbOWord;
#define EmbPWord EmbOWord*

/* @data EmbPWord2 ************************************************************
**
** NUCLEUS data structure for words (part 2)
**
** @attr name [char*] Name
** @attr fword [EmbPWord] Word structure
** @@
******************************************************************************/

typedef struct EmbSWord2 {
  char *name;
  EmbPWord fword;
} EmbOWord2;
#define EmbPWord2 EmbOWord2*



AjPList embWordBuildMatchTable (AjPTable *seq1MatchTable,
				const AjPSeq seq2, ajint orderit);
void    embWordClear (void);
void    embWordFreeTable(AjPTable *table);
ajint   embWordGetTable (AjPTable *table, const AjPSeq seq);
void    embWordLength (ajint wordlen);
AjBool  embWordMatchIter (AjIList iter, ajint* start1, ajint* start2,
			  ajint* len);
void    embWordMatchListDelete (AjPList* plist);
void    embWordMatchListPrint (AjPFile file, const AjPList list);
void    embWordPrintTable  (const AjPTable table);
void    embWordPrintTableF (const AjPTable table, AjPFile outf);
void    embWordMatchListConvToFeat(const AjPList list,
				   AjPFeattable *tab1, AjPFeattable *tab2,
				   const AjPSeq seq1, const AjPSeq seq2);

void    embWordMatchMin(AjPList matchlist, ajint seq1length, int
        			seq2length);
#endif

#ifdef __cplusplus
}
#endif



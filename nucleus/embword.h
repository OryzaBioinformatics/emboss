/*  Last edited: Mar  2 15:41 2000 (pmr) */
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embWord_h
#define embWord_h

typedef struct EmbSWordMatch {
  int seq1start;            /* match start point in original sequence */
  int seq2start;            /* match start point in comparison sequence */
  int length;               /* length of match */
  AjPSeq sequence;          /* need in case we build multiple matches here */
			    /* so we know which one the match belongs to */
} EmbOWordMatch, *EmbPWordMatch;

typedef struct EmbSWord {
  int count;
  char *fword;
  AjPList list;
} EmbOWord, *EmbPWord;

typedef struct EmbSWord2 {
  char *name;
  EmbPWord fword;
} EmbOWord2, *EmbPWord2;



AjPList embWordBuildMatchTable (AjPTable *seq1MatchTable,
				AjPSeq seq2, int orderit);
void    embWordClear (void);
void    embWordFreeTable( AjPTable table);
int     embWordGetTable (AjPTable *table, AjPSeq seq);
void    embWordLength (int wordlen);
void    embWordMatchListDelete (AjPList list);
void    embWordMatchListPrint (AjPFile file, AjPList list);
void    embWordPrintTable  (AjPTable table);
void    embWordPrintTableF (AjPTable table, AjPFile outf);
void    embWordMatchListConvToFeat(AjPList list, AjPFeatTable *tab1, AjPFeatTable *tab2,
				AjPStr seq1name, AjPStr seq2name);

void    embWordMatchMin(AjPList matchlist, int seq1length, int
        			seq2length);
#endif

#ifdef __cplusplus
}
#endif



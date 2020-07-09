#include "emboss.h"
#include "math.h"	/* for sqrt() */

/* current wordlength - this is an easily accessible copy of the value
in the first node of wordLengthList */
static int wordLength = 0; 

/* list of wordlengths with current one at top of list */
static AjPList wordLengthList = NULL;	

static void orderPosMatchTable(AjPList unorderedList);
static int wordMatchCmpPos (const void* v1, const void* v2);

static int wordCmpStr(const void *x, const void *y);
static unsigned wordStrHash(const void *key, unsigned hashsize);
static void wordNewListTrace (int i, AjPList newlist);
static void wordCurListTrace (AjPList curlist);
static void wordCurIterTrace (AjIList curiter);
static int wordMatchCmp (const void* v1,
			 const void* v2);
static int deadZone(EmbPWordMatch match, int deadx1, int deady1, int
	deadx2, int deady2, int minlength);




/* @funcstatic wordCmpStr *********************************************
**
** Compare two words for first n chars. n set by embWordLength.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [int] difference 
** @@
******************************************************************************/
static int wordCmpStr(const void *x, const void *y) {
  return ajStrNCmpCaseCC((char *)x, (char *)y, wordLength);
}

/* @funcstatic wordStrHash *********************************************
**
** Create hash value from key.
**
** @param [r] key [const void *] key. 
** @param [r] hashsize [unsigned] Hash size
** @return [unsigned] hash value
** @@
******************************************************************************/
static unsigned wordStrHash(const void *key, unsigned hashsize)
{
  unsigned hashval;
  char *s = (char *) key;

  int i;
  for(i=0, hashval = 0; i < wordLength; i++, s++)
    hashval = toupper((int)*s) + 31 *hashval;
  return hashval % hashsize;
}

/* @funcstatic compare *********************************************
**
** Compare two words.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [int] count difference for words.
** @@
******************************************************************************/
static int compare(const void *x, const void *y) {
  EmbPWord x1 = ((EmbPWord2)x)->fword;
  EmbPWord y1 = ((EmbPWord2)y)->fword;
  
  return (y1->count - x1->count);
}

/* @func embWordLength ********************************************************
**
** Sets the word length for all functions. Must be called first.
** Creates the word length list if not yet done.
** Pushes the latest word length value on the list.
**
** @param [r] wordlen [int] Word length
** @return [void]
** @@
******************************************************************************/

void embWordLength (int wordlen) {

  int *pint;

  if (!wordLengthList) {
	wordLengthList = ajListNew();
  }

/* store the wordlength in case we do recursive word stuff */
  AJNEW0(pint);
  *pint = wordlen;
  ajListPush(wordLengthList, pint);

/* set the current wordlength as an easily accessible static int */
  wordLength = wordlen;

  return;
}

/* @func embWordClear *********************************************************
**
** Clears the word length for all functions. To be called when all is done.
** Pops the last word length from the list and frees it.
** If there is nothing else on the list, it frees the list.
**
** @return [void]
** @@
******************************************************************************/

void embWordClear (void) {
/* was  wordLength = 0; */

  int *pint;

/* pop the previous word length from the list in case we are doing
recursive word stuff */
  ajListPop(wordLengthList, (void **)&pint);
  AJFREE(pint);
  
  if (!ajListLength(wordLengthList)) {
    ajListDel(&wordLengthList);
    wordLengthList = NULL;	/* no valid word length set */
    wordLength = 0;		/* no valid word length set */
  } else {
/* set the current wordlength as an easily accessible static int */
    ajListFirst(wordLengthList, (void **)&pint);
    wordLength = *pint;
  }
}

/* @func embWordPrintTable ***********************************************
**
** Print the words found with their frequencies.
**
** @param [Pw] table [AjPTable] table to be created or updated.
** @return [void]
** @@
******************************************************************************/

void embWordPrintTable(AjPTable table) {
  void **array = ajTableToarray(table, NULL);
  EmbPWord ajnew;
  int i;

  qsort(array, ajTableLength(table), 2*sizeof (*array),compare);
  for (i = 0; array[i]; i += 2) {
    ajnew = (EmbPWord) array[i+1];
    (void) ajUser("%.*s\t%d", wordLength, ajnew->fword,ajnew->count);
  }
  AJFREE(array); 
}

/* @func embWordPrintTableF ***********************************************
**
** Print the words found with their frequencies.
**
** @param [Pw] table [AjPTable] table to be created or updated.
** @param [r] outf [AjPFile] Output file.
** @return [void]
** @@
******************************************************************************/

void embWordPrintTableF(AjPTable table, AjPFile outf) {
  void **array = ajTableToarray(table, NULL);
  EmbPWord ajnew;
  int i;

  qsort(array, ajTableLength(table), 2*sizeof (*array),compare);
  for (i = 0; array[i]; i += 2) {
    ajnew = (EmbPWord) array[i+1];
    (void) ajFmtPrintF(outf, "%.*s\t%d\n",
		       wordLength, ajnew->fword,ajnew->count);
  }
  AJFREE(array); 
}

/* @funcstatic positionListDelete *********************************************
**
** deletes entries in a list of positions.
**
** @param [wP] x [void**] Data values as void**
** @param [P] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void positionListDelete(void **x,void *cl) {
  int *p = (int *)*x;

  AJFREE(p);
}

/* @funcstatic vfree *********************************************
**
** free the elements in a list of positons
**
** @param [P] key [const void*] key for a table item
** @param [P] count [void**] Data values as void**
** @param [P] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void vfree(const void *key, void **count, void *cl) {

  /* free the elements in the list of the positons */
  ajListMap(((EmbPWord)*count)->list,positionListDelete, NULL);

  /* free the list structure for the positions. */
  ajListFree(&((EmbPWord)*count)->list);

  /* free the word associated to element */
  /* AJFREE((*count)->fword);*/

  /* free the word structure */
  AJFREE(*count);
}

/* @func embWordFreeTable ***********************************************
**
** delete the word table and free the memory.
**
** @param [wP] table [AjPTable] table to be created or updated.
** @return [void]
** @@
******************************************************************************/

void embWordFreeTable(AjPTable table) {

  ajTableMap(table, vfree, NULL);
  ajTableFree(&table);
  table = 0;
}

/* @funcstatic matchListDelete ***********************************************
**
** deletes entries in a list of matches.
**
** @param [wP] x [void**] Data values as void**
** @param [P] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void matchListDelete(void **x,void *cl) {
  EmbPWordMatch p = (EmbPWordMatch)*x;

  AJFREE(p);
}

/* @func embWordMatchListDelete ***********************************************
**
** delete the word table.
**
** @param [Pw] list [AjPList] list to be deleted.
** @return [void]
** @@
******************************************************************************/

void embWordMatchListDelete(AjPList list){
  ajListMap(list,matchListDelete, NULL);
  ajListFree(&list);
}

/* @funcstatic  matchListPrint ***********************************************
**
** print the word table.
**
** @param [r] x [void**] List item (EmbPWordMatch*)
** @param [r] cl [void*] Output file AjPFile
** @return [void]
** @@
******************************************************************************/

static void matchListPrint(void **x,void *cl) {
  EmbPWordMatch p = (EmbPWordMatch)*x;
  AjPFile file = (AjPFile) cl;

  (void) ajFmtPrintF(file, "%10d  %10d %10d\n",
		     (*p).seq1start+1,
		     (*p).seq2start+1,
		     (*p).length);
}

/* @func embWordMatchListPrint ***********************************************
**
** print the word table.
**
** @param [Pr] file [AjPFile] Output file
** @param [Pr] list [AjPList] list to be printed.
** @return [void]
** @@
******************************************************************************/

void embWordMatchListPrint(AjPFile file, AjPList list) {
  ajListMap(list,matchListPrint, file);
  return;
}

/* @func embWordMatchListConvToFeat ***********************************************
**
** convert the word table to feature tables.
**
** @param [Pr] list [AjPList] list to be printed.
** @param [rw] tab1 [AjPFeatTable*] feature table for sequence 1
** @param [rw] tab2 [AjPFeatTable*] feature table for sequence 2
** @param [r] seq1name [AjPStr] sequence name
** @param [r] seq2name [AjPStr] secondsequence name
** @return [void]
** @@
******************************************************************************/

void embWordMatchListConvToFeat(AjPList list, AjPFeatTable *tab1, AjPFeatTable *tab2,AjPStr seq1name, AjPStr seq2name) {
  AjEFeatStrand strand=AjStrandWatson;
  AjEFeatFrame frame=AjFrameUnknown;
  AjPStr score=NULL,source=NULL,type=NULL,tag=NULL;
  AjPFeature feature;
  AjIList iter=NULL;
  AjPFeatLexicon dict=NULL;

  dict = ajFeatGffDictionaryCreate(); 
  if(!*tab1)
    *tab1 = ajFeatTabNew(seq1name,dict);
  if(!*tab2)
    *tab2 = ajFeatTabNew(seq2name,dict);
  
  ajStrAssC(&source,"wordmatch");
  ajStrAssC(&type,"misc_feature");
  ajStrAssC(&score,"1.0");
  ajStrAssC(&tag,"note");
  
  iter = ajListIter(list);
  while(ajListIterMore(iter)) {
    EmbPWordMatch p = (EmbPWordMatch) ajListIterNext (iter) ;
    feature = ajFeatureNew(*tab1, source, type,
			   p->seq1start+1,p->seq1start+p->length , score, 
			   strand, frame, NULL , 0, 0) ;

    ajFeatSetTagValue(feature, tag, seq2name, 0);

    feature = ajFeatureNew(*tab2, source, type,
			   p->seq2start+1,p->seq2start+p->length , score, 
			   strand, frame, NULL , 0, 0) ;    

    ajFeatSetTagValue(feature, tag, seq1name, 0);
  }
  /* delete the iterator */
  ajListIterFree(iter);
  ajStrDel(&source);
  ajStrDel(&type);
  ajStrDel(&score);
  ajStrDel(&tag);

  return;
}

/* @func embWordGetTable *****************************************************
**
** Builds a table of all words in a sequence.
**
** The word length must be defined by a call to embWordLength.
**
** @param [wP] table [AjPTable*] table to be created or updated.
** @param [P] seq [AjPSeq] Sequence to be "worded"
** @return [int] 1 if successful 0 if not.
** @@
******************************************************************************/

int embWordGetTable(AjPTable *table, AjPSeq seq) {
  char * startptr;
  int i;
  int ilast;
  int *k;
  EmbPWord rec;

  int iwatch[] = {48, 5509, 2328, 2127, 5249, 2647, 5287, 5571, -1};
  int iw;
  AjBool dowatch;

  assert (wordLength > 0);

  ajDebug ("embWordGetTable seq.len %d wordlength %d\n",
	   ajSeqLen(seq), wordLength);

  if(ajSeqLen(seq) < wordLength) {
    ajErr("wordsize = %d, sequence length = %d",
	  wordLength, ajSeqLen(seq));
    return ajFalse;
  }

  if(!*table) {
    *table = ajTableNew(0, wordCmpStr, wordStrHash);
    ajDebug ("make new table\n");
  }

  startptr = ajSeqChar(seq);    /* initialise ptr to start of seq string*/

  i=0;
  ilast = ajSeqLen(seq) - wordLength;

  while(i <= ilast) {
    /*    struct position *position;*/

    dowatch = ajFalse;
    iw = 0;
    while (iwatch[iw] >= 0) {
      if (iwatch[iw++] == i) dowatch = ajTrue;
    }

    rec = (EmbPWord) ajTableGet(*table, startptr);
    /* does it exist already */
    if(rec) {                               /* if yes increment count */
      rec->count++;
      if (dowatch)
	ajDebug ("   %.*s exists %d times\n",
		 wordLength, startptr, rec->count);
    }
    else {                                 /* else create a new word */
      AJNEW0(rec);
      rec->count= 1;
      rec->fword = startptr;
      rec->list = ajListNew();
      (void) ajTablePut(*table, startptr, rec);
      if (dowatch)
	ajDebug ("   %.*s first time\n", wordLength, startptr);
    }

    AJNEW0(k);
    *k = i;
    ajListPushApp(rec->list, k);

    startptr ++;
    i++;

  }
  ajDebug ("table done, size %d\n", ajTableLength (*table));
  return ajTrue;
}

/* @funcstatic findWordAtPos ********************************************
**
** Looks for a word at a given position
**
** @param [r] word [char*] Word to find
** @param [P] seq1MatchTable [AjPTable] Match table
** @param [r] nextpos [int] Not used
** @return [int] Position found (1 is the start) or 0 if none.
** @@
******************************************************************************/

static int findWordAtPos (char *word, AjPTable seq1MatchTable, int nextpos) {
  EmbPWord wordmatch;
  int *k = &nextpos;
  
  wordmatch = ajTableGet(seq1MatchTable, word);
  if(wordmatch){
    int *pos;
    AjIList iter = ajListIter(wordmatch->list);
   
    while((pos = (int *) ajListIterNext(iter))){
      if(*pos == *k){
	ajListIterFree(iter);
	return *pos +1;
      }
    }
    ajListIterFree(iter);
  }
  return 0;
}
  
/* @funcstatic getWholeMatch ********************************************
**
** Looks for a word length match.
**
** @param [P] match [EmbPWordMatch] match structure
** @param [P] seq1MatchTable [AjPTable] match table
** @return [int] Match position
** @@
******************************************************************************/

static int getWholeMatch (EmbPWordMatch match,
			  AjPTable seq1MatchTable) {

  AjPSeq seq2 = match->sequence;
  char *startptr;
  int i=0;
  int ilast;
  int nextpos=0; 

  assert (wordLength > 0);

  startptr = &(ajSeqChar(seq2)[match->seq2start+1]);

  i = match->seq2start;

  nextpos = match->seq1start + 1 ;

  ilast = ajSeqLen(seq2) - wordLength;
  while (i <= ilast) {
 
    /* find if it matches */
    if (!findWordAtPos(startptr, seq1MatchTable, nextpos)) break;

    match->length++;
    nextpos++;
    i++;
    startptr++;
  }
  return (nextpos+wordLength) - (match->seq1start +1);
}

/* @funcstatic orderMatchTable ************************************************
**
** Sort the hits by length then seq1 start then by seq2 start
**
** @param [P] unorderedList [AjPList] Unsorted list
** @return [void]
** @@
******************************************************************************/

static void orderMatchTable(AjPList unorderedList) {
  ajListSort (unorderedList, wordMatchCmp);
  return;
}

/* @funcstatic wordMatchCmp ********************************************
**
** Compares two sequence matches so the result can be used in sorting.
** The comparison is done by size and if the size is equal, by seq1
** start position.  If the seq1 start positions are equal they are
** sorted by seq2 start position. 
**
** @param [P] v1 [const void*] First word
** @param [P] v2 [const void*] Comparison word
** @return [int] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
** @@
******************************************************************************/

static int wordMatchCmp (const void* v1,
			 const void* v2) {
  EmbPWordMatch* x1;
  EmbPWordMatch* x2;
  EmbPWordMatch m1;
  EmbPWordMatch m2;

  x1 = (EmbPWordMatch*) v1;
  x2 = (EmbPWordMatch*) v2;
  m1 = *x1;
  m2 = *x2;

  /*
  ajDebug ("m1 %x %5d %5d %5d\n",
           m1, m1->length, m1->seq2start, m1->seq1start);
  ajDebug ("m2 %x %5d %5d %5d\n",
           m2, m2->length, m2->seq2start, m2->seq1start);
  */

  if (m1->length != m2->length) {
    if (m1->length < m2->length) {
      /*ajDebug("return 1\n");*/
      return 1;
    }
    else return -1;
  }
  if (m1->seq1start != m2->seq1start) {
    if (m1->seq1start > m2->seq1start) {
      /*ajDebug("return 1\n");*/
      return 1;
    }
    else return -1;
  }
  if (m1->seq2start != m2->seq2start) {
    if (m1->seq2start > m2->seq2start) {
      /*ajDebug("return 1\n");*/
      return 1;}
    else return -1;
  }

  return 0;
}


/* @funcstatic orderPosMatchTable ************************************************
**
** Sort the hits by seq1 start then by seq2 start
**
** @param [P] unorderedList [AjPList] Unsorted list
** @return [void]
** @@
******************************************************************************/

static void orderPosMatchTable(AjPList unorderedList) {
  ajListSort (unorderedList, wordMatchCmpPos);
  return;
}

/* @funcstatic wordMatchCmpPos ********************************************
**
** Compares two sequence matches so the result can be used in sorting.
** The comparison is done by seq1
** start position.  If the seq1 start positions are equal they are
** sorted by seq2 start position. 
**
** @param [P] v1 [const void*] First word
** @param [P] v2 [const void*] Comparison word
** @return [int] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
** @@
******************************************************************************/

static int wordMatchCmpPos (const void* v1,
			 const void* v2) {
  EmbPWordMatch* x1;
  EmbPWordMatch* x2;
  EmbPWordMatch m1;
  EmbPWordMatch m2;

  x1 = (EmbPWordMatch*) v1;
  x2 = (EmbPWordMatch*) v2;
  m1 = *x1;
  m2 = *x2;

  /*
  ajDebug ("m1 %x %5d %5d %5d\n",
           m1, m1->length, m1->seq2start, m1->seq1start);
  ajDebug ("m2 %x %5d %5d %5d\n",
           m2, m2->length, m2->seq2start, m2->seq1start);
  */

  if (m1->seq1start != m2->seq1start) {
    if (m1->seq1start > m2->seq1start) {
      /*ajDebug("return 1\n");*/
      return 1;
    }
    else return -1;
  }
  if (m1->seq2start != m2->seq2start) {
    if (m1->seq2start > m2->seq2start) {
      /*ajDebug("return 1\n");*/
      return 1;}
    else return -1;
  }

  return 0;
}


/* @func embWordBuildMatchTable ***********************************************
**
** Create a linked list of all the matches and order them by the
** second sequence.
**
** We need three lists:
**   (a) all hits, added in positional order
**   (b) ongoing hits, where we have not reached the end yet
**                 which is a list of items in "all hits" being updated
**   (c) new hits, found in the word table from the other sequence.
**
** @param [Pw] seq1MatchTable [AjPTable*] Match table
** @param [r] seq2 [AjPSeq] Second sequence
** @param [r] orderit [int] 1 to sort results at end, else 0.
** @return [AjPList] List of matches.
** @error NULL table was not built due to an error.
** @@
******************************************************************************/

AjPList embWordBuildMatchTable (AjPTable *seq1MatchTable,  AjPSeq seq2,
				int orderit) {
  int i = 0;
  int ilast;
  AjPList hitlist=NULL;
  static AjPList curlist=NULL;
  AjPList newlist=NULL;
  char *startptr;
  EmbPWord wordmatch;
  EmbPWordMatch match;
  EmbPWordMatch match2;
  EmbPWordMatch curmatch=NULL;
  AjIList newiter;
  AjIList curiter;
  void *ptr=NULL;
  
  int *k = 0;
  int kcur=0, knew=0;

  /*  int iwatch[] = {0, 194, 211, 306, 393, 546, 624, 775, -1};*/
  /*  int iw;*/

  assert (wordLength > 0);
  AJNEW0(match);

  match->sequence = seq2;  

  hitlist = ajListNew();

  if(!curlist)
      curlist = ajListNew();

  if(ajSeqLen(seq2) < wordLength) {
    ajErr("ERROR: Sequence length = %d and word length = %d.\n",
	   ajSeqLen(seq2), wordLength);
    ajErr("sequence length must be larger than word length");
    return NULL;
  }
  startptr = ajSeqChar(seq2);
  ilast = ajSeqLen(seq2) - wordLength;

  /*ajDebug ("ilast: %d\n", ilast);*/

  while (i <= ilast) {

    /*    iw = 0;*/
    /*    while (iwatch[iw] >= 0) {
      if (iwatch[iw++] == i) dowatch = ajTrue;
      }*/
    /* if (dowatch)
      ajDebug ("watching at %d %.*s\n", i, wordLength, startptr);
    */

    if ((wordmatch = ajTableGet(*seq1MatchTable, startptr))) {

      /* match found so create EmbSWordMatch structure and fill it 
	 in. Then set next pos accordingly */
      /* BUT this could match several places so need to do for each position */

      /* we have a match between the two sequences */
      /* this could extend an existing match or start a new one */


      newlist = wordmatch->list;

      if(!ajListLength(newlist))
	ajErr("ERROR: newlist is empty??????\n");

      wordNewListTrace(i, newlist);

      /*ajDebug ("\nIterate at %d list size %d %.*s\n",
	i, ajListLength(newlist), wordLength, startptr);*/

      newiter = ajListIter(newlist);

      /* this is the list of matches for the current word and position */

      if (ajListLength(curlist)) {
	/*wordCurListTrace(curlist);*/
	curiter = ajListIter(curlist);
	/* ajDebug ("CurList size %d\n",ajListLength(curlist));*/
	curmatch = ajListIterNext(curiter);
	kcur = curmatch->seq1start + curmatch->length - wordLength + 1;
      }
      else
	curiter = 0;

      while (ajListIterMore(newiter) ) {
	k = (int*) ajListIterNext(newiter);
	knew = *k;

	/* compare to current hits to test for extending */

	while (curiter && (kcur < knew)) {
	  /* ajDebug ("..skip knew: %d kcur: %d start1: %d start2: %d len: %d\n",
		   knew, kcur, curmatch->seq1start, curmatch->seq2start,
		   curmatch->length);*/
	  ajListRemove(curiter);
	  curmatch = ajListIterNext(curiter);
	  if (curmatch) {
	    kcur = curmatch->seq1start + curmatch->length - wordLength + 1;
	    /* ajDebug ("curiter next kcur: %d\n", kcur);*/
	  }
	  else {
	    /* ajDebug ("curiter finished - free it\n");*/
	    ajListIterFree(curiter);
	    curiter = 0;
	  }
	}
	if (kcur == knew) {	/* check continued matches */
	  while (curiter && (kcur == knew)) {
	    /* ajDebug ("**match knew: %d kcur: %d start1: %d start2: %d len: %d\n",
		     knew, kcur, curmatch->seq1start, curmatch->seq2start,
		     curmatch->length);*/
	    curmatch->length++;
	    curmatch = ajListIterNext(curiter);
	    if (curmatch) {
	      kcur = curmatch->seq1start + curmatch->length - wordLength + 1;
	      /* ajDebug ("curiter next kcur: %d\n", kcur); */
	    }
	    else {
	      /* ajDebug ("curiter finished - free it\n");*/
	      ajListIterFree(curiter);
	      curiter = 0;
	    }
	  }
	}
	else {			/* new current match */
	  AJNEW0(match2);
	  match2->sequence = seq2;  
	  match2->seq1start = knew;
	  match2->seq2start = i;
	  match2->length = wordLength;
	  /* ajDebug ("save start1: %d start2: %d len: %d\n",
		   match2->seq1start, match2->seq2start,
		   match2->length);*/
	  ajListPushApp(hitlist, match2); /* add to hitlist */
	  if (curiter) {	/* add to curlist */
	    /*ajDebug("ajListInsert using curiter\n");*/
	    ajListInsert(curiter, match2);
	    /*wordCurListTrace(curlist);*/
	    /*wordCurIterTrace(curiter);*/
	  }
	  else {
	    /* ajDebug("ajListPushApp\n");*/
	    ajListPushApp(curlist, match2);
	    /*wordCurListTrace(curlist);*/
	  }
	}
	/*ajDebug ("k: %d i: %d\n", *k, i);*/
      }
      ajListIterFree(newiter);

      while (curiter) {
	/* ajDebug ("..ignore knew: %d kcur: %d start1: %d start2: %d len: %d\n",
		 knew, kcur, curmatch->seq1start, curmatch->seq2start,
		 curmatch->length);*/
	ajListRemove(curiter);
	curmatch = ajListIterNext(curiter);
	if (curmatch) {
	  kcur = curmatch->seq1start + curmatch->length - wordLength + 1;
	  /* ajDebug ("curiter next kcur: %d\n", kcur);*/
	}
	else {
	  /* ajDebug ("curiter finished - free it\n");*/
	  ajListIterFree(curiter);
	  curiter = 0;
	}
      }

    }

    /* no match, so all existing matches are completed */

    i++;
    startptr++;
  }

  /*wordCurListTrace(hitlist);*/
  if (orderit)
    orderMatchTable(hitlist);

  /*wordCurListTrace(hitlist);*/

  AJFREE (match);

  while(ajListPop(curlist,(void **)&ptr));
  
  return hitlist;
}

/* @funcstatic wordNewListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [r] i [int] Offset
** @param [P] newlist [AjPList] word list.
** @return [void]
** @@
******************************************************************************/

static void wordNewListTrace (int i, AjPList newlist) {
  /* int *k;*/

  AjIList iter = ajListIter(newlist);
  /*
  ajDebug ("\nnewlist...\n");
  while (ajListIterMore(iter)) {
    k = (int*) ajListIterNext(iter);
    ajDebug("%6d %6d %6d %6d %6d\n",
	    (*k)+wordLength, i+wordLength, (*k)+1, i+1, wordLength);
  }
  */
  ajListIterFree(iter);
}

/* @funcstatic wordCurListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [P] curlist [AjPList] word list.
** @return [void]
** @@
******************************************************************************/

static void wordCurListTrace (AjPList curlist) {
  /*EmbPWordMatch match;*/
  /*int i, j, ilen;*/
  AjIList iter = ajListIter(curlist);
  /*

  ajDebug ("\ncurlist...\n");
  while (ajListIterMore(iter)) {
    match = ajListIterNext(iter);
    i = match->seq1start + 1;
    j = match->seq2start + 1;
    ilen = match->length;
    ajDebug("%6d %6d %6d %6d %6d\n",
	    i+ilen, j+ilen, i, j, ilen);
  }
  */
  ajListIterFree(iter);
}

/* @funcstatic wordCurIterTrace ***********************************************
**
** Reports contents of a current word list iterator
**
** @param [P] curiter [AjIList] List iterator for the current word list
** @return [void]
** @@
******************************************************************************/

static void wordCurIterTrace (AjIList curiter) {
  /*AjPListNode node;*/
  /*EmbPWordMatch match;*/
  /*int i, j, ilen;*/

  /*
  ajDebug ("curiter ...\n");
  if (curiter->PCurr) {
    node = *curiter->PCurr;
    match = node->Item;
    i = match->seq1start + 1;
    j = match->seq2start + 1;
    ilen = match->length;
    ajDebug(" PCurr: %6d %6d %6d %6d %6d\n",
	    i+ilen, j+ilen, i, j, ilen);
  }
  else
    ajDebug (" PCurr: NULL\n");

  node = *curiter->PPrev;
  match = node->Item;
  i = match->seq1start + 1;
  j = match->seq2start + 1;
  ilen = match->length;
  ajDebug(" PPrev: %6d %6d %6d %6d %6d\n",
	  i+ilen, j+ilen, i, j, ilen);

  */

  return;
}


/* @funcstatic deadZone ******************************************************
** Determines if a match is within the region which is not overlapped by the 
** match starting at position (deadx1, deady1) or ending at position
** (deadx2, deady2). If it is in this region
** (the 'live zone') then 0 is returned, if it is partially in this region
** then it is truncated to lie inside it and 2 is returned, else 1 is returned.
** 
** What is the 'live zone' and 'dead zone'?
** When an initial large match has been found we wish to remove any other
** (smaller) matches that overlap with it. The region in which other matches
** overlap with the first match are called here the 'dead zones'. The regions
** in which they don't overlap are called the 'live zones'. Other matches are
** OK if they are in live zones - they can co-exist with this match.
** 
** 
**                   deadx2   
**       |              .
**       |              .   live
**       |              .   zone 2
**       |     dead     ............deady2
**       |     zone     /
**  seq2 |             /
**       |            /match
**       |           /
**   deady1..........     
**       |          .     dead
**       |          .     zone
**       |live      .
**       |zone 1  deadx1
**       -------------------------
**                 seq1
** 
** @param [P] match [EmbPWordMatch] match to investigate
** @param [r] deadx1 [int] x position of end of live zone 1
** @param [r] deady1 [int] y position of end of live zone 1
** @param [r] deadx2 [int] x position of end of live zone 2
** @param [r] deady2 [int] y position of end of live zone 2
** @return [int] 0=in live zone, 1=in dead zone, 2=truncated
** @@
******************************************************************************/

static int deadZone(EmbPWordMatch match, int deadx1, int deady1, int deadx2, int deady2, int minlength) {

  int startx = match->seq1start;
  int starty = match->seq2start;
  int endx = match->seq1start + match->length -1;
  int endy = match->seq2start + match->length -1;

/* is it in the top right live zone ? */
  if (startx > deadx2 && starty > deady2) return 0;

/* is it in the bottom right live zone ? */
  if (endx < deadx1 && endy < deady1) return 0;

/* is it in the top left dead zone? */
  if (starty >=  deady1 && endx <= deadx2) return 1;

/* is it in the bottom right dead zone? */
  if (endy <= deady2 && startx >= deadx1) return 1;

/* it must be partially in a dead zone - truncate it */

  if (endy < deady2) {
    if (startx - starty < deadx1 - deady1) {
/* crosses deady1 */
      match->length = deady1 - starty;
      
    } else if (startx - starty > deadx1 - deady1) {
/* crosses deadx1 */
      match->length = deadx1 - startx;
    	
    } else {
      ajFatal("Found a match where match is on the same diagonal \
      startx=%d, starty=%d, endx=%d, endy=%d \
      deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n", 
      startx, starty, endx, endy, deadx1, deady1, deadx2, deady2);    	
    }

  } else if (starty > deady1) {
    if (startx - starty < deadx1 - deady1) {
/* crosses deadx2 */
      match->length = endx - deadx2;
      match->seq1start = deadx2 +1;
      match->seq2start += deadx2 - startx +1;
    
    } else if (startx - starty > deadx1 - deady1) {
/* crosses deady2 */
      match->length = endy - deady2;
      match->seq1start += deady2 - starty +1;
      match->seq2start = deady2 +1;
    
    } else {
      ajFatal("Found a match where match is on the same diagonal \
      startx=%d, starty=%d, endx=%d, endy=%d \
      deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n", 
      startx, starty, endx, endy, deadx1, deady1, deadx2, deady2);    	
    }  	
  } else {
    ajFatal("Found a match that was not caught by any case \
    startx=%d, starty=%d, endx=%d, endy=%d \
    deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n", 
    startx, starty, endx, endy, deadx1, deady1, deadx2, deady2);
  }  	
  
/* is the truncated match shorter than our allowed minimum length?
If so it should be dead */
  if (match->length < minlength) return 1;

  return 2;
}


/* func embWordMatchMin ******************************************************
** Given a list of matches, reduce it to the minimal set of best
** non-overlapping matches.
**
** @param [P] matchlist [AjPList] list of matches to reduce to non-overlaping set
** @param [r] seq1length [int] length of sequence1 being considered
** @param [r] seq2length [int] length of sequence2 being considered
** @return [void] 
** @@
******************************************************************************/

void embWordMatchMin(AjPList matchlist, int seq1length, int seq2length) {


  AjIList iter = NULL;
  EmbPWordMatch match, thismatch;
  AjPList minlist = ajListNew();	/* list of matches in min set */
  int deadx1, deady1, deadx2, deady2;	/* positions of the dead zones */
  AjBool truncated;	
  int result;
             
/* order the matches by size - largest first */
  orderMatchTable(matchlist);

/* remove all other matches in the overlapping dead-zone, truncating
those that extend into the live-zone */

/* repeat until there are no more matches to process */
  while (ajListLength(matchlist)) {
/* get next longest match and append to list of minimal matches */
    ajListPop(matchlist, (void **)&thismatch);

    ajListPushApp(minlist, thismatch);
/* get the positions of the dead zones */
    deadx1 = thismatch->seq1start;	/* first pos of match */
    deady1 = thismatch->seq2start;	/* first pos of match */
    deadx2 = thismatch->seq1start + thismatch->length -1;	/* last pos of match */
    deady2 = thismatch->seq2start + thismatch->length -1;	/* last pos of match */

/* we haven't truncated any matches yet */
    truncated = ajFalse;
    
/* look at all remaining matches in matchlist */
    iter = ajListIter(matchlist);
    while (ajListIterMore(iter)) {
      match = ajListIterNext(iter);

/* want to remove this match if it is in the dead zone */      
      result = deadZone(match, deadx1, deady1, deadx2, deady2, wordLength);
      if (result == 1) {	/* it is in the dead zone - remove it */
/* need to free up the match structure and remove the current node of the list */
        matchListDelete((void **)&match, NULL);
      	ajListRemove(iter);
      } else if (result == 2) {	/* it is partially in the dead zone - now truncated */
        truncated = ajTrue;
      }
    }
    ajListIterFree(iter);

/* if we have done some truncating we need to sort the matchlist again by size */
    if (truncated) {
      orderMatchTable(matchlist);
    }
  }


/* sort by x start position */
  orderPosMatchTable(minlist);

/* debug */
/*
  ajDebug ("\noutput list...\n\n");
  iter = ajListIter(minlist);
  while (ajListIterMore(iter)) {
    match = ajListIterNext(iter);
    i = match->seq1start;
    j = match->seq2start;
    ilen = match->length;
    ajDebug("startx=%6d endx=%6d starty=%6d endy=%6d length=%6d\n",
            i+1, i+ilen, j+1, j+ilen, ilen);
  }
  ajListIterFree(iter);
*/

/* matchlist is now reduced to the minimal non-overlapping list */
  ajListPushList(matchlist, &minlist);
}




/*********************************************************/
/* Unused functions. Here to keep compiler warnings away */
/*********************************************************/
void wordUnused(void){
    EmbOWordMatch match;
    AjPTable ajptable=NULL;
    int *x,*y=NULL;
    
    (void) getWholeMatch (&match,ajptable);
    positionListDelete((void *)&x,(void *)y);
}


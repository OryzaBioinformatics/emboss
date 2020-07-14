/* @source embword.c
**
** Wordmatch routines
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"
#include "math.h"

/*
** current wordlength - this is an easily accessible copy of the value
** in the first node of wordLengthList
*/
static ajint wordLength = 0;

/* list of wordlengths with current one at top of list */
static AjPList wordLengthList = NULL;




static ajint    wordCmpStr(const void *x, const void *y);
static ajint    wordCompare(const void *x, const void *y);
static void     wordCurIterTrace(const AjIList curiter);
static void     wordCurListTrace(const AjPList curlist);
static ajint    wordDeadZone(EmbPWordMatch match,
			     ajint deadx1, ajint deady1,
			     int deadx2, ajint deady2, ajint minlength);
static ajint    wordFindWordAtPos(const char *word,
				  const AjPTable seq1MatchTable,
				  ajint nextpos);
static ajint    wordGetWholeMatch(EmbPWordMatch match,
				  const AjPTable seq1MatchTable);
static void     wordListInsertNodeOld(AjPListNode* pnode, void* x);
static void     wordListInsertOld(AjIList iter, void* x);
static ajint    wordMatchCmp(const void* v1, const void* v2);
static ajint    wordMatchCmpPos(const void* v1, const void* v2);
static void     wordNewListTrace(ajint i, const AjPList newlist);
static void     wordOrderPosMatchTable(AjPList unorderedList);

static unsigned wordStrHash(const void *key, unsigned hashsize);

static void     wordVFree(const void *key, void **count, void *cl);


/* @funcstatic wordCmpStr *****************************************************
**
** Compare two words for first n chars. n set by embWordLength.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] difference
** @@
******************************************************************************/

static ajint wordCmpStr(const void *x, const void *y)
{
    return ajStrNCmpCaseCC((char *)x, (char *)y, wordLength);
}




/* @funcstatic wordStrHash ****************************************************
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
    char *s;

    ajint i;

    s = (char *) key;

    for(i=0, hashval = 0; i < wordLength; i++, s++)
	hashval = toupper((ajint)*s) + 31 *hashval;

    return hashval % hashsize;
}




/* @funcstatic wordCompare ****************************************************
**
** Compare two words.
**
** @param [r] x [const void *] First word
** @param [r] y [const void *] Second word
** @return [ajint] count difference for words.
** @@
******************************************************************************/

static ajint wordCompare(const void *x, const void *y)
{
    EmbPWord x1;
    EmbPWord y1;

    x1 = ((EmbPWord2)x)->fword;
    y1 = ((EmbPWord2)y)->fword;

    return (y1->count - x1->count);
}




/* @func embWordLength ********************************************************
**
** Sets the word length for all functions. Must be called first.
** Creates the word length list if not yet done.
** Pushes the latest word length value on the list.
**
** @param [r] wordlen [ajint] Word length
** @return [void]
** @@
******************************************************************************/

void embWordLength(ajint wordlen)
{
    ajint *pint;

    if(!wordLengthList)
	wordLengthList = ajListNew();

    /* store the wordlength in case we do recursive word stuff */
    AJNEW0(pint);
    *pint = wordlen;
    ajListPush(wordLengthList, pint);

    /* set the current wordlength as an easily accessible static ajint */
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

void embWordClear(void)
{
    ajint *pint;

    /*
    ** pop the previous word length from the list in case there's
    ** recursive word stuff
    */
    ajListPop(wordLengthList, (void **)&pint);
    AJFREE(pint);

    if(!ajListLength(wordLengthList))
    {
	ajListDel(&wordLengthList);
	wordLengthList = NULL;		/* no valid word length set */
	wordLength = 0;			/* no valid word length set */
    }
    else
    {
	/* set the current wordlength as an easily accessible static ajint */
	ajListFirst(wordLengthList, (void **)&pint);
	wordLength = *pint;
    }

    return;
}




/* @func embWordPrintTable ****************************************************
**
** Print the words found with their frequencies.
**
** @param [r] table [const AjPTable] table to be printed
** @return [void]
** @@
******************************************************************************/

void embWordPrintTable(const AjPTable table)
{
    void **array;
    EmbPWord ajnew;
    ajint i;

    array = ajTableToarray(table, NULL);

    qsort(array, ajTableLength(table), 2*sizeof (*array),wordCompare);
    for(i = 0; array[i]; i += 2)
    {
	ajnew = (EmbPWord) array[i+1];
	ajUser("%.*s\t%d", wordLength, ajnew->fword,ajnew->count);
    }

    AJFREE(array);

    return;
}




/* @func embWordPrintTableF ***************************************************
**
** Print the words found with their frequencies.
**
** @param [r] table [const AjPTable] table to be printed
** @param [u] outf [AjPFile] Output file.
** @return [void]
** @@
******************************************************************************/

void embWordPrintTableF(const AjPTable table, AjPFile outf)
{
    void **array;
    EmbPWord ajnew;
    ajint i;

    array = ajTableToarray(table, NULL);

    qsort(array, ajTableLength(table), 2*sizeof (*array),wordCompare);
    for(i = 0; array[i]; i += 2)
    {
	ajnew = (EmbPWord) array[i+1];
	ajFmtPrintF(outf, "%.*s\t%d\n",
			   wordLength, ajnew->fword,ajnew->count);
    }

    AJFREE(array);

    return;
}




/* @funcstatic wordPositionListDelete *****************************************
**
** deletes entries in a list of positions.
**
** @param [d] x [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void wordPositionListDelete(void **x,void *cl)
{
    ajint *p;

    p = (ajint *)*x;

    AJFREE(p);

    return;
}




/* @funcstatic wordVFree ******************************************************
**
** free the elements in a list of positons
**
** @param [r] key [const void*] key for a table item
** @param [d] count [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void wordVFree(const void *key, void **count, void *cl)
{
    /* free the elements in the list of the positons */
    ajListMap(((EmbPWord)*count)->list,wordPositionListDelete, NULL);

    /* free the list structure for the positions. */
    ajListFree(&((EmbPWord)*count)->list);

    /* free the word structure */
    AJFREE(*count);

    return;
}




/* @func embWordFreeTable *****************************************************
**
** delete the word table and free the memory.
**
** @param [d] table [AjPTable*] table to be deleted
** @return [void]
** @@
******************************************************************************/

void embWordFreeTable(AjPTable *table)
{
    ajTableMap(*table, wordVFree, NULL);
    ajTableFree(table);
    table = 0;

    return;
}




/* @funcstatic wordMatchListDelete ********************************************
**
** deletes entries in a list of matches.
**
** @param [d] x [void**] Data values as void**
** @param [r] cl [void*] Ignored user data, usually NULL.
** @return [void]
** @@
******************************************************************************/

static void wordMatchListDelete(void **x,void *cl)
{
    EmbPWordMatch p;

    p = (EmbPWordMatch)*x;

    AJFREE(p);

    return;
}




/* @func embWordMatchListDelete ***********************************************
**
** delete the word table.
**
** @param [u] plist [AjPList*] list to be deleted.
** @return [void]
** @@
******************************************************************************/

void embWordMatchListDelete(AjPList* plist)
{
    if(!*plist)
	return;

    ajListMap(*plist,wordMatchListDelete, NULL);
    ajListFree(plist);

    return;
}




/* @funcstatic wordMatchListPrint *********************************************
**
** print the word table.
**
** @param [r] x [void*] List item (EmbPWordMatch*)
** @param [r] cl [void*] Output file AjPFile
** @return [void]
** @@
******************************************************************************/

static void wordMatchListPrint(void *x,void *cl)
{
    EmbPWordMatch p;
    AjPFile file;

    p    = (EmbPWordMatch)x;
    file = (AjPFile) cl;

    ajFmtPrintF(file, "%10d  %10d %10d\n",
		       p->seq1start+1,
		       p->seq2start+1,
		       p->length);

    return;
}




/* @func embWordMatchListPrint ************************************************
**
** print the word table.
**
** @param [u] file [AjPFile] Output file
** @param [r] list [const AjPList] list to be printed.
** @return [void]
** @@
******************************************************************************/

void embWordMatchListPrint(AjPFile file, const AjPList list)
{
    ajListMapRead(list,wordMatchListPrint, file);

    return;
}




/* @func embWordMatchListConvToFeat *******************************************
**
** convert the word table to feature tables.
**
** @param [r] list [const AjPList] list to be printed.
** @param [u] tab1 [AjPFeattable*] feature table for sequence 1
** @param [u] tab2 [AjPFeattable*] feature table for sequence 2
** @param [r] seq1 [const AjPSeq] sequence
** @param [r] seq2 [const AjPSeq] second sequence
** @return [void]
** @@
******************************************************************************/

void embWordMatchListConvToFeat(const AjPList list,
				AjPFeattable *tab1, AjPFeattable *tab2,
				const AjPSeq seq1, const AjPSeq seq2)
{
    char strand = '+';
    ajint frame = 0;
    AjPStr source = NULL;
    AjPStr type   = NULL;
    AjPStr tag    = NULL;
    AjPFeature feature;
    AjIList iter  = NULL;
    float score   = 1.0;
    
    if(!*tab1)
	*tab1 = ajFeattableNewSeq(seq1);

    if(!*tab2)
	*tab2 = ajFeattableNewSeq(seq2);
    
    ajStrAssC(&source,"wordmatch");
    ajStrAssC(&type,"misc_feature");
    score = 1.0;
    ajStrAssC(&tag,"note");
    
    iter = ajListIterRead(list);
    while(ajListIterMore(iter))
    {
	EmbPWordMatch p = (EmbPWordMatch) ajListIterNext(iter);
	feature = ajFeatNew(*tab1, source, type,
			    p->seq1start+1,p->seq1start+p->length , score,
			    strand, frame) ;
	
	ajFeatTagSet(feature, tag, ajSeqGetName(seq2));
	
	feature = ajFeatNew(*tab2, source, type,
			    p->seq2start+1,p->seq2start+p->length , score,
			    strand, frame) ;
	
	ajFeatTagSet(feature, tag, ajSeqGetName(seq1));
    }
    
    ajListIterFree(&iter);
    ajStrDel(&source);
    ajStrDel(&type);
    ajStrDel(&tag);
    
    return;
}




/* @func embWordGetTable ******************************************************
**
** Builds a table of all words in a sequence.
**
** The word length must be defined by a call to embWordLength.
**
** @param [u] table [AjPTable*] table to be created or updated.
** @param [r] seq [const AjPSeq] Sequence to be "worded"
** @return [ajint] 1 if successful 0 if not.
** @@
******************************************************************************/

ajint embWordGetTable(AjPTable *table, const AjPSeq seq)
{
    const char * startptr;
    ajint i;
    ajint ilast;
    ajint *k;
    EmbPWord rec;

    ajint iwatch[] = {-1};
    ajint iw;
    AjBool dowatch;

    assert(wordLength > 0);

    ajDebug("embWordGetTable seq.len %d wordlength %d\n",
	     ajSeqLen(seq), wordLength);

    if(ajSeqLen(seq) < wordLength)
    {
	ajErr("wordsize = %d, sequence length = %d",
	      wordLength, ajSeqLen(seq));
	return ajFalse;
    }

    if(!*table)
    {
	*table = ajTableNewL(ajSeqLen(seq), wordCmpStr, wordStrHash);
	ajDebug("make new table\n");
    }

    /* initialise ptr to start of seq string */
    startptr = ajSeqChar(seq);

    i = 0;
    ilast = ajSeqLen(seq) - wordLength;

    while(i <= ilast)
    {
	dowatch = ajFalse;
	iw = 0;
	while(iwatch[iw] >= 0)
	    if(iwatch[iw++] == i)
		dowatch = ajTrue;

	rec = (EmbPWord) ajTableGet(*table, startptr);

	/* does it exist already */
	if(rec)
	{
	    /* if yes increment count */
	    rec->count++;
	    if(dowatch)
		ajDebug("   %.*s exists %d times\n",
			 wordLength, startptr, rec->count);
	}
	else
	{
	    /* else create a new word */
	    AJNEW0(rec);
	    rec->count= 1;
	    rec->fword = startptr;
	    rec->list = ajListNew();
	    ajTablePut(*table, startptr, rec);
	}

	AJNEW0(k);
	*k = i;
	ajListPushApp(rec->list, k);

	startptr ++;
	i++;

    }

    ajDebug("table done, size %d\n", ajTableLength(*table));

    return ajTrue;
}




/* @funcstatic wordFindWordAtPos **********************************************
**
** Looks for a word at a given position
**
** @param [r] word [const char*] Word to find
** @param [r] seq1MatchTable [const AjPTable] Match table
** @param [r] nextpos [ajint] Not used
** @return [ajint] Position found (1 is the start) or 0 if none.
** @@
******************************************************************************/

static ajint wordFindWordAtPos(const char *word, const AjPTable seq1MatchTable,
			       ajint nextpos)
{
    EmbPWord wordmatch;
    ajint *k;
    ajint *pos;
    AjIList iter;

    k = &nextpos;

    wordmatch = ajTableGet(seq1MatchTable, word);
    if(wordmatch)
    {
	iter = ajListIterRead(wordmatch->list);

	while((pos = (ajint *) ajListIterNext(iter)))
	    if(*pos == *k)
	    {
		ajListIterFree(&iter);
		return *pos +1;
	    }

	ajListIterFree(&iter);
    }

    return 0;
}




/* @funcstatic wordGetWholeMatch **********************************************
**
** Looks for a word length match.
**
** @param [u] match [EmbPWordMatch] match structure
** @param [r] seq1MatchTable [const AjPTable] match table
** @return [ajint] Match position
** @@
******************************************************************************/

static ajint wordGetWholeMatch(EmbPWordMatch match,
			       const AjPTable seq1MatchTable)
{
    const AjPSeq seq2;
    const char *startptr;
    ajint i = 0;
    ajint ilast;
    ajint nextpos = 0;

    assert(wordLength > 0);

    seq2 = match->sequence;

    startptr = &(ajSeqChar(seq2)[match->seq2start+1]);

    i = match->seq2start;

    nextpos = match->seq1start + 1;

    ilast = ajSeqLen(seq2) - wordLength;
    while(i <= ilast)
    {
	/* find if it matches */
	if(!wordFindWordAtPos(startptr, seq1MatchTable, nextpos)) break;

	match->length++;
	nextpos++;
	i++;
	startptr++;
    }

    return (nextpos+wordLength) - (match->seq1start +1);
}




/* @funcstatic wordOrderMatchTable ********************************************
**
** Sort the hits by length then seq1 start then by seq2 start
**
** @param [u] unorderedList [AjPList] Unsorted list
** @return [void]
** @@
******************************************************************************/

static void wordOrderMatchTable(AjPList unorderedList)
{
    ajDebug("wordOrderMatchTable size %d\n", ajListLength(unorderedList));
    ajListSort(unorderedList, wordMatchCmp);

    return;
}




/* @funcstatic wordMatchCmp ***************************************************
**
** Compares two sequence matches so the result can be used in sorting.
** The comparison is done by size and if the size is equal, by seq1
** start position.  If the seq1 start positions are equal they are
** sorted by seq2 start position.
**
** @param [r] v1 [const void*] First word
** @param [r] v2 [const void*] Comparison word
** @return [ajint] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
** @@
******************************************************************************/

static ajint wordMatchCmp(const void* v1, const void* v2)
{
    EmbPWordMatch* x1;
    EmbPWordMatch* x2;
    EmbPWordMatch m1;
    EmbPWordMatch m2;

    x1 = (EmbPWordMatch*) v1;
    x2 = (EmbPWordMatch*) v2;
    m1 = *x1;
    m2 = *x2;

    /*
       ajDebug("m1 %x %5d %5d %5d\n",
       m1, m1->length, m1->seq2start, m1->seq1start);
       ajDebug("m2 %x %5d %5d %5d\n",
       m2, m2->length, m2->seq2start, m2->seq1start);
       */

    if(m1->length != m2->length)
    {
	if(m1->length < m2->length)
	{
	    /*ajDebug("return 1\n");*/
	    return 1;
	}
	else
	    return -1;
    }

    if(m1->seq1start != m2->seq1start)
    {
	if(m1->seq1start > m2->seq1start)
	    return 1;
	else
	    return -1;
    }

    if(m1->seq2start != m2->seq2start)
    {
	if(m1->seq2start > m2->seq2start)
	    return 1;
	else
	    return -1;
    }

    return 0;
}




/* @funcstatic wordOrderPosMatchTable *****************************************
**
** Sort the hits by seq1 start then by seq2 start
**
** @param [u] unorderedList [AjPList] Unsorted list
** @return [void]
** @@
******************************************************************************/

static void wordOrderPosMatchTable(AjPList unorderedList)
{
    ajListSort(unorderedList, wordMatchCmpPos);

    return;
}




/* @funcstatic wordMatchCmpPos ************************************************
**
** Compares two sequence matches so the result can be used in sorting.
** The comparison is done by seq1
** start position.  If the seq1 start positions are equal they are
** sorted by seq2 start position.
**
** @param [r] v1 [const void*] First word
** @param [r] v2 [const void*] Comparison word
** @return [ajint] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
** @@
******************************************************************************/

static ajint wordMatchCmpPos(const void* v1, const void* v2)
{
    EmbPWordMatch* x1;
    EmbPWordMatch* x2;
    EmbPWordMatch m1;
    EmbPWordMatch m2;

    x1 = (EmbPWordMatch*) v1;
    x2 = (EmbPWordMatch*) v2;
    m1 = *x1;
    m2 = *x2;

    /*
       ajDebug("m1 %x %5d %5d %5d\n",
       m1, m1->length, m1->seq2start, m1->seq1start);
       ajDebug("m2 %x %5d %5d %5d\n",
       m2, m2->length, m2->seq2start, m2->seq1start);
       */

    if(m1->seq1start != m2->seq1start)
    {
	if(m1->seq1start > m2->seq1start)
	{
	    /*ajDebug("return 1\n");*/
	    return 1;
	}
	else return -1;
    }

    if(m1->seq2start != m2->seq2start)
    {
	if(m1->seq2start > m2->seq2start)
	{
	    /*ajDebug("return 1\n");*/
	    return 1;
	}
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
** @param [u] seq1MatchTable [AjPTable*] Match table
** @param [r] seq2 [const AjPSeq] Second sequence
** @param [r] orderit [ajint] 1 to sort results at end, else 0.
** @return [AjPList] List of matches.
** @error NULL table was not built due to an error.
** @@
******************************************************************************/

AjPList embWordBuildMatchTable(AjPTable *seq1MatchTable,  const AjPSeq seq2,
				ajint orderit)
{
    ajint i = 0;
    ajint ilast;
    AjPList hitlist = NULL;
    static AjPList curlist = NULL;
    const AjPList newlist = NULL;
    const char *startptr;
    EmbPWord wordmatch;
    EmbPWordMatch match;
    EmbPWordMatch match2;
    EmbPWordMatch curmatch = NULL;
    AjIList newiter;
    AjIList curiter;
    void *ptr = NULL;

    ajint *k = 0;
    ajint kcur = 0;
    ajint knew = 0;

    assert(wordLength > 0);
    AJNEW0(match);

    match->sequence = seq2;

    hitlist = ajListNew();

    if(!curlist)
	curlist = ajListNew();

    if(ajSeqLen(seq2) < wordLength)
    {
	ajErr("ERROR: Sequence length = %d and word length = %d.\n",
	      ajSeqLen(seq2), wordLength);
	ajErr("sequence length must be larger than word length");
	return NULL;
    }
    startptr = ajSeqChar(seq2);
    ilast    = ajSeqLen(seq2) - wordLength;

    /*ajDebug("ilast: %d\n", ilast);*/

    while(i <= ilast)
    {
	if((wordmatch = ajTableGet(*seq1MatchTable, startptr)))
	{
	    /* match found so create EmbSWordMatch structure and fill it
	    ** in. Then set next pos accordingly
	    ** BUT this could match several places so need to do for each
            ** position
            **
	    ** there is a match between the two sequences
	    ** this could extend an existing match or start a new one
	    */

	    newlist = wordmatch->list;

	    if(!ajListLength(newlist))
		ajErr("ERROR: newlist is empty??????\n");

	    newiter = ajListIterRead(newlist);

	    /* this is the list of matches for the current word and position */

	    if(ajListLength(curlist))
	    {
	      /* ajDebug("CurList size %d\n",ajListLength(curlist)); */
		curiter = ajListIter(curlist);

		curmatch = ajListIterNext(curiter);
		kcur = curmatch->seq1start + curmatch->length - wordLength + 1;
	    }
	    else
		curiter = 0;

	    while(ajListIterMore(newiter) )
	    {
		k = (ajint*) ajListIterNext(newiter);
		knew = *k;

		/* compare to current hits to test for extending */

		while(curiter && (kcur < knew))
		{
		  /*ajDebug("..skip knew: %d kcur: %d start1: %d start2: %d "
			     "len: %d\n",
			     knew, kcur, curmatch->seq1start,
			     curmatch->seq2start,curmatch->length);*/
		    ajListRemove(curiter);

		    curmatch = ajListIterNext(curiter);
		    if(curmatch)
		    {
			kcur = curmatch->seq1start + curmatch->length -
			    wordLength + 1;
			/*ajDebug("curiter next kcur: %d\n", kcur);*/
		    }
		    else
		    {
			ajListIterFree(&curiter);
		    }
		}

		/* ajDebug("kcur: %d knew: %d\n", kcur, knew); */
		if(kcur && kcur == knew)
		{			/* check continued matches */
		    while(curiter && (kcur == knew))
		    {
		      /* ajDebug("**match knew: %d kcur: %d start1: %d "
				 "start2: %d len: %d\n",
				 knew, kcur, curmatch->seq1start,
				 curmatch->seq2start,curmatch->length); */
			curmatch->length++;
			curmatch = ajListIterNext(curiter);
			if(curmatch)
			    kcur = curmatch->seq1start + curmatch->length -
				wordLength + 1;
			else
			{
			    ajListIterFree(&curiter);
			}
		    }
		}
		else
		{			/* new current match */
		    AJNEW0(match2);
		    match2->sequence  = seq2;
		    match2->seq1start = knew;
		    match2->seq2start = i;
		    match2->length = wordLength;
		    /* ajDebug("save start1: %d start2: %d len: %d\n",
			     match2->seq1start, match2->seq2start,
			     match2->length);*/
		    /* ajDebug("Pushapp to hitlist\n"); */
		    ajListPushApp(hitlist, match2); /* add to hitlist */
		    if(curiter)
		    {			/* add to curlist */
		      /* ajDebug("ajListInsert using curiter\n"); */
			wordListInsertOld(curiter, match2);
			/*wordCurListTrace(curlist);*/
			/*wordCurIterTrace(curiter);*/
		    }
		    else
		    {
		      /* ajDebug("ajListPushApp to curlist\n"); */
			ajListPushApp(curlist, match2);
			/* wordCurListTrace(curlist); */
		    }
		}
		/* ajDebug("k: %d i: %d\n", *k, i); */
	    }
	    ajListIterFree(&newiter);

	    while(curiter)
	    {
	      /* ajDebug("..ignore knew: %d kcur: %d start1: %d "
			 "start2: %d len: %d\n",
			 knew, kcur, curmatch->seq1start, curmatch->seq2start,
			 curmatch->length); */
		ajListRemove(curiter);

		curmatch = ajListIterNext(curiter);
		if(curmatch)
		{
		    kcur = curmatch->seq1start +
			curmatch->length - wordLength + 1;
		    /* ajDebug("curiter next kcur: %d\n", kcur); */
		}
		else
		{
		  /* ajDebug("curiter finished - free it\n"); */
		    ajListIterFree(&curiter);
		}
	    }

	}

	/* no match, so all existing matches are completed */

	i++;
	startptr++;
    }

    /* wordCurListTrace(hitlist); */
    if(orderit)
	wordOrderMatchTable(hitlist);

    /* wordCurListTrace(hitlist); */

    AJFREE(match);

    while(ajListPop(curlist,(void **)&ptr));

    return hitlist;
}




/* @funcstatic wordNewListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [r] i [ajint] Offset
** @param [r] newlist [const AjPList] word list.
** @return [void]
** @@
******************************************************************************/

static void wordNewListTrace(ajint i, const AjPList newlist)
{
    ajint *k;
    AjIList iter;

    iter = ajListIterRead(newlist);

    ajDebug("\n++newlist... %d \n", i);
    ajDebug("++  k+len  i+len    k+1    i+1    len\n");
    while(ajListIterMore(iter))
    {
	k = (ajint*) ajListIterNext(iter);
	ajDebug("++ %6d %6d %6d %6d %6d\n",
		(*k)+wordLength, i+wordLength, (*k)+1, i+1, wordLength);
    }
    ajListIterFree(&iter);

    return;
}




/* @funcstatic wordCurListTrace ***********************************************
**
** Reports contents of a word list.
**
** @param [r] curlist [const AjPList] word list.
** @return [void]
** @@
******************************************************************************/

static void wordCurListTrace(const AjPList curlist)
{
/*
    EmbPWordMatch match;
    ajint i;
    ajint j;
    ajint ilen;
*/    
    AjIList iter = ajListIterRead(curlist);

    /*
       ajDebug("\ncurlist...\n");
       while(ajListIterMore(iter))
       {
       match = ajListIterNext(iter);
       i = match->seq1start + 1;
       j = match->seq2start + 1;
       ilen = match->length;
       ajDebug("%6d %6d %6d %6d %6d\n",
       i+ilen, j+ilen, i, j, ilen);
       }
    */

    ajListIterFree(&iter);

    return;
}




/* @funcstatic wordCurIterTrace ***********************************************
**
** Reports contents of a current word list iterator
**
** @param [r] curiter [const AjIList] List iterator for the current word list
** @return [void]
** @@
******************************************************************************/

static void wordCurIterTrace(const AjIList curiter)
{
    /*AjPListNode node;*/
    /*EmbPWordMatch match;*/
    /*ajint i, j, ilen;*/

    /*
       ajDebug("curiter ...\n");
       if(curiter->PCurr)
       {
       node = *curiter->PCurr;
       match = node->Item;
       i = match->seq1start + 1;
       j = match->seq2start + 1;
       ilen = match->length;
       ajDebug(" PCurr: %6d %6d %6d %6d %6d\n",
       i+ilen, j+ilen, i, j, ilen);
       }
       else
       ajDebug(" PCurr: NULL\n");

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




/* @funcstatic wordDeadZone ***************************************************
**
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
** @param [u] match [EmbPWordMatch] match to investigate
** @param [r] deadx1 [ajint] x position of end of live zone 1
** @param [r] deady1 [ajint] y position of end of live zone 1
** @param [r] deadx2 [ajint] x position of end of live zone 2
** @param [r] deady2 [ajint] y position of end of live zone 2
** @param [r] minlength [ajint] minimum length of match
** @return [ajint] 0=in live zone, 1=in dead zone, 2=truncated
** @@
******************************************************************************/

static ajint wordDeadZone(EmbPWordMatch match,
			  ajint deadx1, ajint deady1,
			  ajint deadx2, ajint deady2, ajint minlength)
{
    ajint startx;
    ajint starty;
    ajint endx;
    ajint endy;

    startx = match->seq1start;
    starty = match->seq2start;

    endx = match->seq1start + match->length -1;
    endy = match->seq2start + match->length -1;


    /* is it in the top right live zone ? */
    if(startx > deadx2 && starty > deady2)
	return 0;

    /* is it in the bottom right live zone ? */
    if(endx < deadx1 && endy < deady1)
	return 0;

    /* is it in the top left dead zone? */
    if(starty >=  deady1 && endx <= deadx2)
	return 1;

    /* is it in the bottom right dead zone? */
    if(endy <= deady2 && startx >= deadx1)
	return 1;

    /* it must be partially in a dead zone - truncate it */

    if(endy < deady2)
    {
	if(startx - starty < deadx1 - deady1)	        /* crosses deady1 */
	    match->length = deady1 - starty;
	else if(startx - starty > deadx1 - deady1)	/* crosses deadx1 */
	    match->length = deadx1 - startx;
	else
	    ajFatal("Found a match where match is on the same diagonal \n"
		    "startx=%d, starty=%d, endx=%d, endy=%d \n"
		    "deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n",
		    startx, starty, endx, endy, deadx1, deady1, deadx2,
		    deady2);
    }
    else if(starty > deady1)
    {
	if(startx - starty < deadx1 - deady1)
	{
	    /* crosses deadx2 */
	    match->length = endx - deadx2;
	    match->seq1start = deadx2 +1;
	    match->seq2start += deadx2 - startx +1;

	}
	else if(startx - starty > deadx1 - deady1)
	{
	    /* crosses deady2 */
	    match->length = endy - deady2;
	    match->seq1start += deady2 - starty +1;
	    match->seq2start = deady2 +1;

	}
	else
	    ajFatal("Found a match where match is on the same diagonal \n"
		    "startx=%d, starty=%d, endx=%d, endy=%d \n"
		    "deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n",
		    startx, starty, endx, endy, deadx1, deady1, deadx2,
		    deady2);
    }
    else
	ajFatal("Found a match that was not caught by any case \n"
		"startx=%d, starty=%d, endx=%d, endy=%d \n"
		"deadx1=%d, deady1=%d, deadx2=%d, deady2=%d\n",
		startx, starty, endx, endy, deadx1, deady1, deadx2, deady2);

    /*
    **  is the truncated match shorter than our allowed minimum length?
    **  If so it should be dead
    */
    if(match->length < minlength)
	return 1;

    return 2;
}




/* @func embWordMatchMin ******************************************************
** Given a list of matches, reduce it to the minimal set of best
** non-overlapping matches.
**
** @param [u] matchlist [AjPList] list of matches to reduce to
**                                non-overlapping set
** @param [r] seq1length [ajint]  length of sequence1 being considered
** @param [r] seq2length [ajint]  length of sequence2 being considered
** @return [void]
** @@
******************************************************************************/

void embWordMatchMin(AjPList matchlist, ajint seq1length, ajint seq2length)
{
    AjIList iter = NULL;
    EmbPWordMatch match;
    EmbPWordMatch thismatch;
    AjPList minlist;			/* list of matches in min set */
    ajint deadx1;			/* positions of the dead zones */
    ajint deady1;
    ajint deadx2;
    ajint deady2;
    AjBool truncated;
    ajint result;

    minlist = ajListNew();

    /* order the matches by size - largest first */
    wordOrderMatchTable(matchlist);

    /*
    **  remove all other matches in the overlapping dead-zone, truncating
    **  those that extend into the live-zone
    */

    /* repeat until there are no more matches to process */
    while(ajListLength(matchlist))
    {
	/* get next longest match and append to list of minimal matches */
	ajListPop(matchlist, (void **)&thismatch);

	ajListPushApp(minlist, thismatch);

	/* get the positions of the dead zones */
	deadx1 = thismatch->seq1start;	/* first pos of match */
	deady1 = thismatch->seq2start;	/* first pos of match */

	/* last pos of match */
	deadx2 = thismatch->seq1start + thismatch->length -1;

	/* last pos of match */
	deady2 = thismatch->seq2start + thismatch->length -1;

	/* haven't truncated any matches yet */
	truncated = ajFalse;

	/* look at all remaining matches in matchlist */
	iter = ajListIter(matchlist);
	while(ajListIterMore(iter))
	{
	    match = ajListIterNext(iter);

	    /* want to remove this match if it is in the dead zone */
	    result = wordDeadZone(match, deadx1, deady1, deadx2, deady2,
				  wordLength);
	    if(result == 1)
	    {
		/*
		**  it is in the dead zone - remove it
		**  Need to free up the match structure and remove the
		**  current node of the list
		*/
		wordMatchListDelete((void **)&match, NULL);
		ajListRemove(iter);
	    }
	    else if(result == 2)
	    {
		/* it is partially in the dead zone - now truncated */
		truncated = ajTrue;
	    }
	}
	ajListIterFree(&iter);

	/*
        **  if some truncating done then need to sort the matchlist
        **  again by size
        */
	if(truncated)
	    wordOrderMatchTable(matchlist);
    }


    /* sort by x start position */
    wordOrderPosMatchTable(minlist);


    /* matchlist is now reduced to the minimal non-overlapping list */
    ajListPushList(matchlist, &minlist);

    return;
}




/* @func embWordMatchIter  ****************************************************
**
** Return the start positions and length for the next match.
** The caller iterates over the list, which is a standard AjPList
**
** @param [u] iter [AjIList] List iterator
** @param [w] start1 [ajint*] Start in first sequence
** @param [w] start2 [ajint*] Start in second sequence
** @param [w] len [ajint*] Length of match
** @return [AjBool] ajFalse if the iterator was exhausted
**
******************************************************************************/

AjBool embWordMatchIter(AjIList iter, ajint* start1, ajint* start2,
			ajint* len)
{
    EmbPWordMatch p;

    if(!ajListIterMore(iter))
	return ajFalse;

    p = (EmbPWordMatch) ajListIterNext(iter);
    *start1 = p->seq1start;
    *start2 = p->seq2start;
    *len = p->length;

    return ajTrue;
}




/* @funcstatic wordListInsertOld **********************************************
**
** Obsolete ajListInsert version emulation
** Insert an item in a list, using an iterator (if not null)
** to show which position to insert. Otherwise, simply push.
**
** @param [u] iter [AjIList] List iterator.
** @param [r] x [void*] Data item to insert.
** @return [void]
** @@
******************************************************************************/

static void wordListInsertOld(AjIList iter, void* x)
{
    AjPList list;
    AjPListNode p;

    list = iter->Head;
    p    = iter->Here;

    if(!p->Prev)
    {
	ajListPush(list,(void *)x);
	return;
    }

    if(p == iter->Orig)
    {
	if(!p->Prev->Prev)
	    wordListInsertNodeOld(&list->First->Next,x);
	else
	    wordListInsertNodeOld(&p->Prev->Next,x);
    }
    else
    {
	if(!p->Prev->Prev)
	    wordListInsertNodeOld(&list->First,x);
	else
	    wordListInsertNodeOld(&p->Prev->Prev->Next,x);
    }

    list->Count++;

    return;
}




/* @funcstatic wordListInsertNodeOld ******************************************
**
** Inserts a new node in a list at the current node position.
**
** @param [u] pnode [AjPListNode*] Current node.
** @param [r] x [void*] Data item to insert.
** @return [void]
** @@
******************************************************************************/

static void wordListInsertNodeOld(AjPListNode* pnode, void* x)
{
    AjPListNode p;

    AJNEW0(p);
    p->Item = x;
    p->Next = (*pnode);
    p->Prev = (*pnode)->Prev;
    p->Next->Prev = p;
    *pnode = p;

    return;
}




/* @func embWordUnused ********************************************************
**
** Unused functions. Here to keep compiler warnings away
**
** @return [void]
******************************************************************************/

void embWordUnused(void)
{
    EmbOWordMatch match;
    AjPTable ajptable = NULL;

    wordGetWholeMatch(&match,ajptable);
    wordCurListTrace(NULL);	/* comment out in embWordBuildMatchTable */
    wordCurIterTrace(NULL);	/* comment out in embWordBuildMatchTable */
    wordNewListTrace(0, NULL);	/* comment out in embWordBuildMatchTable */

    return;
}

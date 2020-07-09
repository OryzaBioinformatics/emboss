#include "ajax.h"
#include "emboss.h"
#include "ajtable.h"
#define DEBUG 0 /* set to 1 for debug information */
#define THEONE 5

AjPFile outfile;

struct record{
  int score;
  float ident;
  char *name1; 
  int start1;
  int end1;
  char *name2;
  int start2;
  int end2;
  int offset; /* offset from the start of the "consensus" */
  int added;
};

struct tableTop{
  AjPTable hashTable;
  AjPList matches;
};

struct homol {
  int start;
  int end;
  int offset;
  int dup;
  int score;
  char *name;
  char *seq;
};

static int compare(const void *x, const void *y) {
  EmbPWord x1 = ((EmbPWord2)x)->fword;
  EmbPWord y1 = ((EmbPWord2)y)->fword;
  
  return (y1->count-x1->count);
}

void tempTable(AjPTable table, int wordlen){
  void **array = ajTableToarray(table, NULL);
  EmbPWord new;
  int i,*pos;

  qsort(array, ajTableLength(table), 2*sizeof (*array),compare);
  for (i = 0; array[i]; i += 2) {
    new = (EmbPWord) array[i+1];
    if(new->count > 1){
      AjIList iter = ajListIter(new->list);

      ajFmtPrintF (outfile,
		   "// %.*s\t%d", wordlen, new->fword,new->count);
      while(ajListIterMore(iter)){
	pos = (int *) ajListIterNext(iter);
	ajFmtPrintF (outfile, "\t%d",*pos);
      }
      ajFmtPrintF (outfile, "\n");
      ajListIterFree(iter);
    }
  }
  AJFREE(array); 
}

AjPSeq efetchSeq(char *name)
{
  char *cp;
  char fetchstr[256];
  FILE *pipe;
  int   len=0, d;
  AjPSeq seq;
  
  sprintf(fetchstr, "efetch -q '%s'", name);

  len = 0;

  if (!(pipe = (FILE*)popen(fetchstr, "r")))
    return NULL;
    
  while (!feof(pipe))
    if (isalpha(d=fgetc(pipe))) {
      /*	*cp++ = c;*/
      len++;
    }
  pclose(pipe);

  /*make sequence with size len*/
  
  seq = ajSeqNewL(len+1);
  cp = seq->Seq->Ptr;
  len = 0;
  
 if (!(pipe = (FILE*)popen(fetchstr, "r")))
    return NULL;

  while (!feof(pipe))
    if (isalpha(d=fgetc(pipe))) {
      *cp++ = d;
      len++;
    }
  pclose(pipe);
    
  seq->Seq->Len = len;
  
  return seq;
}

char * getsseq(AjPList sequencePartsList, int size, int verbose)
{
  AjIList iter = ajListIter(sequencePartsList);
  char fetchstr[256];
  char *cp;
  FILE *pipe;
  int   len=0, d;
  int total_len=0;
  char * auxseqcon;
  int j = 0,k,i;
  int *a;
  int *c;
  int *g;
  int *t;

  AJCNEW0(a, size+1);
  AJCNEW0(c, size+1);
  AJCNEW0(g, size+1);
  AJCNEW0(t, size+1);
  AJCNEW(auxseqcon, size+1);

  cp = &auxseqcon[0];

  while(len<size){
    *cp++ = '-';
    len++;
  }
  len = 0;

  /* FOR EACH LIST MEMBER */
  while(ajListIterMore(iter)){         /* for each cluster */
    struct homol *subseq = (struct homol*) ajListIterNext(iter);
    AJCNEW(subseq->seq, size+1);

    cp = &subseq->seq[0];

    while(len<=size){ /* fill sequence with -'s */
      *cp++ = '-';
      len++;
    }
    
    if (verbose)
      ajDebug ("//Efetching **%s**\n", subseq->name);
    sprintf(fetchstr, "efetch -q -s %d -e %d '%s'",
	    subseq->start,subseq->end, subseq->name);
    
    cp = &subseq->seq[subseq->offset];

    len = 0;
    if (!(pipe = (FILE*)popen(fetchstr, "r"))){
      ajErr ("Failed to open pipe %s\n", fetchstr);
      ajErr (" aborting program\n");
      exit(0);
    }
    while (!feof(pipe)) /* get the sequence information for this homol */
      if (isalpha(d=fgetc(pipe)) && len <= size) {
	  *cp++ = d;
	len++;
      }
    pclose(pipe);

    total_len +=len;
    if(len != subseq->end-subseq->start+1){
      ajErr ("ERROR getting all of subsequence %s len = %d got = %d",
	     subseq->name,subseq->end-subseq->start,len);
      ajErr ("efetch command = %s",fetchstr);
    }
  }
  ajListIterFree(iter);


  iter = ajListIter(sequencePartsList);
  while(ajListIterMore(iter)){         /* for each cluster */
    struct homol *subseq = (struct homol*) ajListIterNext(iter);

    k = subseq->offset + (subseq->end-subseq->start);
    for(i = subseq->offset; i <= k; i++){             /* over the whole "consensus" calculate the mode for the bases */
      switch (subseq->seq[i]) {
      case 'a' : a[i]++;
	break;
      case 'c' : c[i]++;
	break;
      case 't' : g[i]++;
	break;
      case 'g' : t[i]++;
	break;
      }
    }
  }
  ajListIterFree(iter);


  /* fill "consensus" with the base that occurs the most else - */
  for(k=0;k<size;k++){
    if(a[k] > c[k] && a[k] > t[k] && a[k] > g[k])
      auxseqcon[k] = 'a';
    else if (c[k] > a[k] && c[k] > t[k] && c[k] > g[k])
      auxseqcon[k] = 'c';
    else if (t[k] > a[k] && t[k] > g[k] && t[k] > c[k])
      auxseqcon[k] = 't';
    else if (g[k] > a[k] && g[k] > c[k] && g[k] > t[k])
      auxseqcon[k] = 'g';
    else
      auxseqcon[k] = '-';
  }


  /* calculate the % identity for each homol and use this as the new score */
  iter = ajListIter(sequencePartsList);
  while(ajListIterMore(iter)){         /* for each cluster */
    struct homol *subseq = (struct homol*) ajListIterNext(iter);
    k = subseq->offset + (subseq->end-subseq->start);
    j=0;
    for(i = subseq->offset; i <= k; i++){
      if(subseq->seq[i] == auxseqcon[i])
	j++;
    }
    subseq->score = (int)((j*100)/(subseq->end-subseq->start));
    AJFREE(subseq->seq);
  }
  ajListIterFree(iter);


  AJFREE(a);  
  AJFREE(c);
  AJFREE(t);
  AJFREE(g);

  return auxseqcon;

}

void cleanup_Cluster(AjPList list){
  AjIList iter = ajListIter(list);

  while(ajListIterMore(iter)){       
    struct tableTop *pos = (struct tableTop*) ajListIterNext(iter);
    if(pos->hashTable)
      ajTableFree(&pos->hashTable);
    if(pos->matches){
      AjIList iter2 = ajListIter(pos->matches);
      while(ajListIterMore(iter2)){      
	struct record *match = (struct record*) ajListIterNext(iter2);
	AJFREE(match->name1);
	AJFREE(match->name2);
	AJFREE(match);
      }
      ajListIterFree(iter2);
      ajListFree(&pos->matches);
    }
    AJFREE(pos);
  }
  ajListIterFree(iter);
  ajListFree(&list);
}

void consensus_output(AjPList unorderedlist, int addblanks,int number) {

  void **array = NULL;
  struct homol *homol;
  struct homol *homolptr[1800];
  struct homol *temp;

  int homolmax = 0;
  int i,j,found1,found2,notallsorted,num,min,k;
  struct record *match;
  struct record *match2;
  static int count=0;
  int start,max;
  char * dnaoutput;
  char * cp;
  AjPList sublist;
  /*  AjIList iter;
  int lastpos;
  char *lastname;*/
  int index;
  AjPTable hashTable;

  struct strange{
    int min;
    int max;
    int total;
    int maxmatch;
    int indexofmaxmatch;
    char *name;
  }*s1;

  int first = 1;
  char *name =0;
  int ia;
  int wordlen = 10;

  AJCNEW(homol, (ajListLength(unorderedlist)+1)*2);

  ia = ajListToArray(unorderedlist, &array);
 
  if(ia > 1) {
    /*ONLY if array[1] exists */
    /* Create hash table to control strange*/
    /* which will contain the min max and len */
    /* off all the homol for ONE seq */
    
    hashTable = ajStrTableNewC(0);
    match = (struct record *) array[0];
    
    AJNEW(s1);
    s1->min = match->start1;
    s1->max = match->end1;
    s1->total = match->end1-match->start1;
    s1->maxmatch = match->end1-match->start1;
    s1->indexofmaxmatch = 0;
    s1->name = match->name1;
    ajTablePut(hashTable, match->name1, s1);
    
    s1 = 0;
    AJNEW(s1);
    s1->min = match->start2;
    s1->max = match->end2;
    s1->total = match->end2-match->start2;
    s1->maxmatch = match->end2-match->start2;
    s1->indexofmaxmatch = 0;
    s1->name = match->name2;
    ajTablePut(hashTable, match->name2, s1);
    
    for (i = 1; i < ia; i++){  
      match = (struct record *) array[i];
      if (!(s1 = ajTableGet(hashTable, match->name1))) {
	s1 = 0;
	AJNEW(s1);
	s1->min = match->start1;
	s1->max = match->end1;
	s1->total = match->end1-match->start1;
	s1->maxmatch = match->end1-match->start1;
	s1->indexofmaxmatch = i;
	s1->name = match->name1;
	ajTablePut(hashTable, match->name1, s1);
      }	
      else {
	if(s1->min >  match->start1)
	  s1->min = match->start1;
	if(s1->max <  match->end1)
	  s1->max = match->end1;
	if(s1->maxmatch < match->end1-match->start1) {
	  s1->maxmatch = match->end1-match->start1;
	  s1->indexofmaxmatch = i;
	}
	s1->total += match->end1-match->start1;
      }
      
      if (!(s1 = ajTableGet(hashTable, match->name2))) {
	s1 = 0;
	AJNEW(s1);
	s1->min = match->start2;
	s1->max = match->end2;
	s1->total = match->end2-match->start2;
	s1->maxmatch = match->end2-match->start2;
	s1->indexofmaxmatch = i;
	s1->name = match->name2;
	ajTablePut(hashTable, match->name2, s1);
      }
      else {
	if (s1->min >  match->start2)
	  s1->min = match->start2;
	if (s1->max <  match->end2)
	  s1->max = match->end2;
	if (s1->maxmatch < match->end2-match->start2) {
	  s1->maxmatch = match->end2-match->start2;
	  s1->indexofmaxmatch = i;
	}
	s1->total += match->end2-match->start2;
      }
    }

    /* the one with greatest length */
    /* add all matches use min of this*/
    /* as the offset 0 */
    {
      void **array2 = ajTableToarray(hashTable, NULL);
      s1 = (struct strange * )array2[1];
      max = s1->max-s1->min;
      index = 1;
    
      for (i = 0; array2[i]; i+=2) {  
	s1 = (struct strange * ) array2[i+1];
	if (DEBUG && count == THEONE)
	  ajDebug ("//DEBUG strange(%d)  %s len = %d min= %d max=%d\n",
		 i+1,s1->name,s1->total,s1->min,s1->max);
	if (s1->max-s1->min > max) {
	  max = s1->max-s1->min;
	  index = i+1;
	}
      }
    
      s1 = (struct strange * )array2[index]; /* the one with the most matches length */
    
      match = (struct record *) array[s1->indexofmaxmatch];
      
      if(DEBUG)
	ajDebug ("//DEBUG maxtotal result = %s, "
	       "max for this is between %s and %s with a length of %d\n",
	       s1->name,match->name1,match->name2,match->end1-match->start1);
    }

    if (!strcmp(s1->name,match->name1)) {
      homol[0].start = match->start1;
      homol[0].end   = match->end1;
      homol[0].name = match->name1;
      homol[1].start = match->start2;
      homol[1].end   = match->end2;
      homol[1].name = match->name2;
    }
    else if (!strcmp(s1->name,match->name2)) {
      homol[1].start = match->start1;
      homol[1].end   = match->end1;
      homol[1].name = match->name1;
      homol[0].start = match->start2;
      homol[0].end   = match->end2;
      homol[0].name = match->name2;
    }    
    else{
      ajFatal ("FATAL ERROR matchname1 || matchname2 != s1->name");
      exit(0);
    }

    match->added = 1;
    homolmax = 1;
    homol[0].score =0;
    homol[1].score =0;
    homol[0].offset = 0;
    homol[1].offset = 0;
    homol[0].dup = 0;
    homol[1].dup = 0;
    homol[0].seq = 0;
    homol[1].seq = 0;

    for (i = 0; i < ia; i++) {  
      match = (struct record *) array[i];
      
      if(!strcmp(match->name1,s1->name)) {
	homolmax++;
	homol[homolmax].name = match->name2;
	homol[homolmax].offset = (match->start1 - homol[0].start);
	homol[homolmax].start = match->start2;
	homol[homolmax].end   = match->end2;
	homol[homolmax].score = match->score;
	homol[homolmax].dup = 0;
	homolmax++;
	homol[homolmax].name = match->name1;
	homol[homolmax].offset = homol[homolmax-1].offset;
	homol[homolmax].start = match->start1;
	homol[homolmax].end   = match->end1;
	homol[homolmax].score = match->score;
	homol[homolmax].dup = 0;
	match->added = 1;
	if(DEBUG && count == THEONE)
	  ajDebug ("//DEBUG(max) %s %d %d %s %d %d\n",
		 match->name1,match->start1,match->end1,
		 match->name2,match->start2,match->end2);
      }
      else if (!strcmp(match->name2,s1->name)) { 
	homolmax++;
	homol[homolmax].name = match->name1;
	homol[homolmax].offset = (match->start2 - homol[0].start) + homol[0].offset ;
	homol[homolmax].start = match->start1;
	homol[homolmax].end   = match->end1;
	homol[homolmax].score = match->score;
	homol[homolmax].dup = 0;
	homolmax++;
	homol[homolmax].name = match->name2;
	homol[homolmax].offset = homol[homolmax-1].offset;
	homol[homolmax].start = match->start2;
	homol[homolmax].end   = match->end2;
	homol[homolmax].score = match->score;
	homol[homolmax].dup = 0;
	match->added = 1;
	if(DEBUG && count == THEONE)
	  ajDebug ("//DEBUG(max) %s %d %d %s %d %d\n",
		 match->name1,match->start1,match->end1,
		 match->name2,match->start2,match->end2);
      }
    }
  }

  if(DEBUG && THEONE == count){
    for(i=0;i<=homolmax;i++){
      ajDebug ("//DEBUG(mid) %s %d %d offset = %d\n",
	     homol[i].name,homol[i].start,homol[i].end,
	     homol[i].offset);
    }
  }
    
  /* then carry on as normal */
  /*except for initial homol[0] etc */

  /* do long sort */

  for (i = 0; i < (ia-1); i++){  
    match = (struct record *) array[i];
    for (j= 0; j < (ia-1); j++){  
      match = (struct record *) array[j];
      match2 = (struct record *) array[j+1];
      if((match->end1-match->start1) > (match2->end1-match2->start1)){
        array[j+1] = match;
         array[j] = match2;
       }
    }
  }
  
  if (ia == 1) {
    match = (struct record *) array[0];
    match->added = 1;
    homol[0].start = match->start1;
    homol[0].end   = match->end1;
    homol[0].offset = 0;
    homol[0].name = match->name1;
    homol[0].score = match->score;
    homol[0].seq = 0;
    homol[1].start = match->start2;
    homol[1].end   = match->end2;
    homol[1].offset = 0;
    homol[1].name = match->name2;
    homol[1].score = match->score;
    homol[1].seq = 0;
    homolmax = 1;
    homol[0].dup = 0;
    homol[1].dup = 0;
  }
  if(DEBUG &&  count== THEONE && !(array[1])){
    ajDebug ("//DEBUG debug\n");
    ajDebug ("//DEBUG %s %d %d %s %d %d\n",
	   match->name1,match->start1,match->end1,
	   match->name2,match->start2,match->end2);
  }

  /* add the other matches and create the homols */
  /* this is looped as you cannot add a match until at least one of */
  /* the matches has already been added otherwise it has no position */
  /* with respect to the others. Hence loop until all added */

  if (ia > 1) {
    int diff1=0,diff2=0;
    notallsorted = 1;
    while(notallsorted){
      for( i = 0; i < ia; i++){
	match = (struct record *) array[i];
	if(!match->added){
	  found1 = 0;
	  for( j = 0; j <= homolmax ; j++){
	    if(!strcmp(match->name1,homol[j].name)) {
	      if(!found1){
		found1 = j+1;
		diff1 = abs(match->start1-homol[j].start);
	      }
	      else if(diff1 > abs(match->start1-homol[j].start)){
		diff1 = abs(match->start1-homol[j].start);
		found1 = j+1;
	      }
	    }
	  }
	  found2 =0;
	  for( j = 0; j <= homolmax; j++){
	    if(!strcmp(match->name2,homol[j].name)){
	      if(!found2){
		found2 = j+1;
		diff2 = abs(match->start2-homol[j].start);
	      }
	      else if(diff2 > abs(match->start2-homol[j].start)){
		diff2 = abs(match->start2-homol[j].start);
		found2 = j+1;
	      }
	    }
	  }
	  if(found1 || found2){
	    match->added = 1;
	    if(DEBUG && count == THEONE)
	      ajDebug ("//DEBUG %s %d %d %s %d %d\n",
		     match->name1,match->start1,match->end1,
		     match->name2,match->start2,match->end2);
	    homolmax++;
	    homol[homolmax].name = match->name1;
	    if(found1) {
	      homol[homolmax].name = match->name2;
	      homol[homolmax].offset = (match->start1 - homol[found1-1].start) + homol[found1-1].offset ;
	      homol[homolmax].start = match->start2;
	      homol[homolmax].end   = match->end2;
	      homol[homolmax].score = match->score;
	      homol[homolmax].dup = 0;
	      homolmax++;
	      homol[homolmax].name = match->name1;
	      homol[homolmax].offset = homol[homolmax-1].offset;
	      homol[homolmax].start = match->start1;
	      homol[homolmax].end   = match->end1;
	      homol[homolmax].score = match->score;
	      homol[homolmax].dup = 0;
	    }
	    else {
	      homol[homolmax].name = match->name1;
	      homol[homolmax].offset = (match->start2 - homol[found2-1].start) + homol[found2-1].offset ;
	      homol[homolmax].start = match->start1;
	      homol[homolmax].end   = match->end1;
	      homol[homolmax].score = match->score;
	      homol[homolmax].dup = 0;
 	      homolmax++;
	      homol[homolmax].name = match->name2;
	      homol[homolmax].offset = homol[homolmax-1].offset;
	      homol[homolmax].start = match->start2;
	      homol[homolmax].end   = match->end2;
	      homol[homolmax].score = match->score;
	      homol[homolmax].dup = 0;
	    }	      
	  }
	}
      }
      notallsorted = 0;                         /* have they all now been added */
      for(i = 0;i < ia; i++){
	match = (struct record *) array[i];
	if(!match->added)
	  notallsorted = 1;
      }
    }
  }

  /* Now sort them so that duplicates can be deleted */

  for (i=0;i<=homolmax;i++) {
    homolptr[i] = &homol[i];
  } 

  if(DEBUG && count == THEONE){
    for(i=0;i<=homolmax;i++){
      if(!homolptr[i]->dup)
	ajDebug ("//DEBUG DNA_homol\t \"%s\" \"ASSEMBL\" %d.0 %d %d %d %d\n",
	       homolptr[i]->name,homolptr[i]->score,homolptr[i]->offset+1,
	       homolptr[i]->offset +(homolptr[i]->end-homolptr[i]->start)+1,
	       homolptr[i]->start-1, homolptr[i]->end-1);
    }
  }
  if(DEBUG && count == (THEONE +1))
    exit(0);


  for( i = 0; i <= homolmax; i++){
    for( j = 0; j < homolmax; j++){
      num = strcmp(homolptr[j]->name,homolptr[j+1]->name);
      if(!num){
	if(homolptr[j]->start > homolptr[j+1]->start){
	  temp = homolptr[j];
	  homolptr[j] = homolptr[j+1];
	  homolptr[j+1] = temp;
	}
      }
      else if (num < 0){
	temp = homolptr[j];
	homolptr[j] = homolptr[j+1];
	homolptr[j+1] = temp;
      }
    }
  } 

  min = homolptr[0]->offset;
  for( j=0; j <= homolmax;j++){
    if(homolptr[j]->offset < min)
      min = homolptr[j]->offset;
  }

  for( j=0; j <= homolmax;){
    start = j;
    while(j<=homolmax && !strcmp(homolptr[start]->name,homolptr[j]->name))
      j++;
    for(i= start; i < j; i++){
      for( k=start; k< j; k++){
	if(i != k && !homolptr[k]->dup && !homolptr[i]->dup){
	  if(homolptr[k]->start == homolptr[i]->start &&
	     homolptr[k]->end == homolptr[i]->end && 
	     !strcmp(homolptr[k]->name,homolptr[i]->name))
	    homolptr[k]->dup = 1;
	  /*	  else if(homolptr[k]->start <= homolptr[i]->start &&        ?? if sub bit remove ??
		   homolptr[k]->end >= homolptr[i]->end &&                    Sub bit may match at another place therefore leave 
		   !strcmp(homolptr[k]->name,homolptr[i]->name))
	    homolptr[i]->dup = 1;*/
	}
      }
    }
  }


  i = 0; /* remove duplicates */
  for( j=0; j <= homolmax;j++){
    if( !homolptr[j]->dup)
      homolptr[i++]=homolptr[j];
  }
  homolmax = i-1;

  ajFmtPrintF (outfile, "\n");

  if(addblanks){ /* order by the consensus positions */
    for( j=0; j <= homolmax;){
      start = j;
      while(j<=homolmax && !strcmp(homolptr[start]->name,homolptr[j]->name))
	j++;
      for(i= start; i < j; i++){
	for( k=start; k< j; k++){
	  if(i > k && !homolptr[i]->dup && !homolptr[k]->dup){
	    if(homolptr[i]->offset < homolptr[k]->offset){
	      temp = homolptr[i];
	      homolptr[i] =  homolptr[k];
	      homolptr[k] = temp;
	    }
	  }
	}
      }
    }
    for( j=0; j <= homolmax;){ /* remove sub bits and merge overlaps */
      start = j;
      while(j<=homolmax && !strcmp(homolptr[start]->name,homolptr[j]->name))
	j++;
      for(i= start; i < j; i++){
	for( k=start; k< j; k++){
	  if( k != i && !homolptr[i]->dup && !homolptr[k]->dup){
	    if(homolptr[i]->start <= homolptr[k]->start &&        /* if sub bit remove */
	       homolptr[i]->end >= homolptr[k]->end && 
	       homolptr[i]->offset ==  homolptr[k]->offset &&
	       !strcmp(homolptr[k]->name,homolptr[i]->name))
	      homolptr[k]->dup = 1;
	    else if(homolptr[i]->start > homolptr[k]->start && 
		    homolptr[i]->start < homolptr[k]->end &&
		    homolptr[i]->end > homolptr[k]->end){
	      if ((homolptr[k]->offset - homolptr[k]->start) == 
		  (homolptr[i]->offset - homolptr[i]->start)){
		homolptr[k]->end =  homolptr[i]->end;
		homolptr[i]->dup = 1;
	      }
	      else if (DEBUG && ((name && strcmp(name,homolptr[i]->name)) ||
				 !name)){
		AjPSeq seq = efetchSeq(homolptr[i]->name);
		AjPTable table =0 ;
		first = 0;
		name = homolptr[i]->name;

		ajDebug ("//DEBUG FLAG cluster %d %s incorrect overlap "
		       "(offset start) (%d %d) (%d %d)\n",
		       count, homolptr[i]->name,homolptr[k]->offset,
		       homolptr[k]->start,
		       homolptr[i]->offset, homolptr[i]->start);

		if(embWordGetTable(&table, seq)) /* get table of words */
		  { 
		    tempTable(table,wordlen); /* print table of words */
		    embWordFreeTable(table); /* free table of words */
		  }
		
	      }
	      else if(DEBUG)
		ajDebug ("//DEBUG FLAG cluster %d %s incorrect overlap "
		       "(offset start) (%d %d) (%d %d)\n",
		       count, homolptr[i]->name,homolptr[k]->offset,
		       homolptr[k]->start,
		       homolptr[i]->offset, homolptr[i]->start);
	
	      
	    }
	    else if(homolptr[i]->start >= homolptr[k]->start && 
		    homolptr[i]->end <= homolptr[k]->end &&
		    ((homolptr[k]->offset - homolptr[k]->start) == 
		     (homolptr[i]->offset - homolptr[i]->start))){
	      homolptr[i]->dup = 1;
	      
	    }
	  }
	}
      }
      
    }
  }

  if(min)
    for( i=0; i <= homolmax; i++)
      homolptr[i]->offset = homolptr[i]->offset - (min);


 if(homolmax == 1){

   sublist = ajListNewArgs(homolptr[0],NULL);
   dnaoutput = getsseq(sublist,(homolptr[0]->end-homolptr[0]->start)+1,0);
   max = (homolptr[0]->end-homolptr[0]->start)+1;

   ajListFree(&sublist);
 }
 else{
   max = 0;

   sublist = 0;
   for(i=0;i<=homolmax;i++){
     if(!homolptr[i]->dup){
       if(homolptr[i]->offset +  (homolptr[i]->end - homolptr[i]->start) > max)
	 max = homolptr[i]->offset +  (homolptr[i]->end - homolptr[i]->start);

       if(sublist)
	 ajListAppend(sublist,ajListNodesNew(homolptr[i],NULL));
       else
	 sublist = ajListNewArgs(homolptr[i],NULL);
     }	 
   }
   dnaoutput = getsseq(sublist,max,0);

   ajListFree(&sublist);
 } 
 ajFmtPrintF (outfile, "DNA : \"consensus.irat%d\"",count);
 cp = dnaoutput;
 j=1;
 ajFmtPrintF (outfile, "\n         -");
 for(i=0;i<max;i++){
   if(j==50){
     ajFmtPrintF (outfile, "\n         ");
     j=0;
   }
   ajFmtPrintF (outfile, "%c",*cp);
/*   *cp++; Doesn't look right. Changed below. AJB */
   ++cp;
   j++;
 }
 ajFmtPrintF (outfile, "\n");

  ajFmtPrintF (outfile, "\nSequence : \"consensus.irat%d\"\n",count);
  ajFmtPrintF (outfile, "DNA      \"consensus.irat%d\" %d\n",count,max);
  ajFmtPrintF (outfile, "In_Cluster\t \"IRat_%d\"\n",count++);

  /*  lastpos =1;
  lastname = homolptr[0]->name;*/
  first = 0;
  for(i=0;i<=homolmax;i++){
    if(!homolptr[i]->dup){
      /*      if(addblanks){
	      if(strcmp(lastname,homolptr[i]->name)){
	      if(lastpos+5 < max)
	      ajFmtPrintF (outfile,
                           "DNA_homol\t \"%s\" \"ASSEMBL\" %d.0 %d %d %d %d\n",
	      homolptr[i]->name, 0, lastpos, max, 1, 3);
	      lastpos = 1;
	      lastname = homolptr[i]->name;
	      }
	      if(homolptr[i]->offset > lastpos+5)
	      ajFmtPrintF (outfile,
                           "DNA_homol\t \"%s\" \"ASSEMBL\" %d.0 %d %d %d %d\n",
	      homolptr[i]->name, 0, lastpos+5, homolptr[i]->offset-4, 1, 3);
	      
	      lastpos = homolptr[i]->offset +(homolptr[i]->end-homolptr[i]->start)+1;
	      }
	      */
      if(DEBUG && (homolptr[i]->score < 90 ))
	 ajDebug("//DEBUG FLAG lcuster %d  %s score = %d\n",
		count-1,homolptr[i]->name,homolptr[i]->score);
      if(DEBUG && (homolptr[i]->score < 90 ) && !first){
	for(j=0;j<=homolmax;j++){
	  if(!homolptr[i]->dup)
	    first++;
	}
	 ajDebug("//DEBUG ERROR cluster %d number in cluster = %d  "
		"num of homols = %d\n",
		count-1,number,first);
      }
      ajFmtPrintF (outfile,
		   "DNA_homol\t \"%s\" \"EMBOSS\" %d.0 %d %d %d %d\n",
	     homolptr[i]->name,
	     homolptr[i]->score, homolptr[i]->offset+1,
	     homolptr[i]->offset +(homolptr[i]->end-homolptr[i]->start)+1,
	     homolptr[i]->start-1, homolptr[i]->end-1);
    }
  }
  AJFREE(homol);

  if(dnaoutput)
    AJFREE(dnaoutput);
  AJFREE(array);
}

/*
void match_Print(void **x,void *cl){ 
  struct record *match = (struct record *)*x; 

  ajDebug ("%d %f %d %d %s %d %d %s\n",             ?? print out the data ??
	 match->score,match->ident, 
	 match->start1,match->end1,match->name1, 
	 match->start2,match->end2,match->name2); 
} 
*/ 
void cluster_Print_Ace(struct tableTop *cluster){
  AjIList iter = ajListIter(cluster->matches);
  AjPTable hashTable;
  int junk = 9;
  int *count = &junk;

  /* create hash table to make sure seq only printed out once */
  hashTable = ajStrTableNewC(0); 
  
  while(ajListIterMore(iter)){         /* for each cluster */
    struct record *match = (struct record*) ajListIterNext(iter);
    
    if(!ajTableGet(hashTable, match->name1)){
      ajFmtPrintF (outfile,
		   "Seq_Cluster\t\"%s\"\n",match->name1);
      ajTablePut(hashTable, match->name1, count);
    }	

    if(!ajTableGet(hashTable, match->name2)){
      ajFmtPrintF (outfile,
		   "Seq_cluster\t\"%s\"\n",match->name2);
      ajTablePut(hashTable, match->name2, count);
    }

  }
  ajListIterFree(iter);
  ajTableFree(&hashTable);
}

void mergeTwoClusters(AjPList list, struct tableTop *cluster1,
		      struct tableTop *cluster2, struct record *match) {
  struct record *match2;
  AjIList iter = ajListIter(cluster2->matches);
  void **array = ajTableToarray(cluster2->hashTable, NULL);
  int i;
  int *count = 0;


    /* add the matches */
  while(ajListIterMore(iter)){         /* for each cluster */
    match2 = (struct record *) ajListIterNext(iter);
    ajListAppend(cluster1->matches, ajListNodesNew(match2,NULL));
  }
  ajListIterFree(iter);
  
  /* add the names from the hash table */
  for( i = 0; array[i]; i+=2) 
    ajTablePut(cluster1->hashTable,(char *)array[i], count);        /* and add name2 to the hash table */
  
  
  /* add new match to match list */
  ajListAppend(cluster1->matches, ajListNodesNew(match,NULL));
  
  ajListFree(&cluster2->matches);
  cluster2->matches = 0; 
  ajTableFree(&cluster2->hashTable);
  cluster2->hashTable = 0; 
  AJFREE(array);

}

int isTheReverse(struct record *match, struct tableTop *cluster1){
  AjIList iter = ajListIter(cluster1->matches);

    while(ajListIterMore(iter)){         /* for each cluster */
      struct record *match2 = (struct record*) ajListIterNext(iter);
  
      if(match2->score == match->score && match2->ident == match->ident &&
	 match2->start1 == match->start2 && match2->end1 == match->end2 &&
	 match2->start2 == match->start1 && match2->end2 == match->end1 &&
	 !strcmp(match2->name1,match->name2) &&
	 !strcmp(match2->name2,match->name1)) {
	  ajListIterFree(iter);
	  return 1;
      }
    }
    ajListIterFree(iter);
    return 0;
}



int main(int argc, char * argv[]){
  AjPFile matchfile;

  AjPStr line = NULL;
  struct record *match;
  struct tableTop *clustertop=0;
  struct tableTop *cluster1=0;
  struct tableTop *cluster2=0;
  AjPList list =0;
  int junk = 9;
  int *count = &junk;
  int num = 0;
  int i =0;
  AjBool consensus;

  embInit("cluster", argc, argv);

  matchfile = ajAcdGetInfile("infile");
  outfile = ajAcdGetOutfile("outfile");

  consensus = ajAcdGetBool("consensus");

  while(ajFileGets(matchfile, &line)){
    AJNEW0(match);
    AJCNEW(match->name1, 40);
    AJCNEW(match->name2, 40);
    match->added = 0;
    num = sscanf(ajStrStr(line),
		 "%d %f %d %d %s %d %d %s",             /* read in the data */
		 &match->score,&match->ident,
		 &match->start1,&match->end1,match->name1,
		 &match->start2,&match->end2,match->name2);
    if(num == 8){
      match->offset = 0;
      cluster1 = cluster2 = 0;
      if(list){
	AjIList iter = ajListIter(list);
	struct tableTop *pos =0;
	  
	if(match->score == 3929)
	  ajDebug("// debug line\n");

	while(ajListIterMore(iter) && !( cluster1 && cluster2)){         /* for each cluster */
	  pos = (struct tableTop*) ajListIterNext(iter);
	  if (pos) { /* due to some may be removed after merging clusters */
	    if(pos->hashTable) { /* due to some may be removed after merging clusters */
	      if(ajTableGet(pos->hashTable, match->name1))
		cluster1 = pos;                                     /* check to see if the cluster
									 contains the name */
	      if(ajTableGet(pos->hashTable, match->name2))
		cluster2 = pos;                                     /* check to see if the cluster
									 contains the name */
	    }
	  }
	}
	ajListIterFree(iter);
	if(cluster1 && cluster2) {                                     /* if both names found */
	    if(cluster1 == cluster2){                                    /* if found it the same cluster */
	      if(!isTheReverse(match,cluster1))                          /* if it is not just a reverse of what has */
		ajListAppend(cluster1->matches, ajListNodesNew(match,NULL)); /* NOT been added then add to list     */
	      else{
		AJFREE(match->name1);
		AJFREE(match->name2);
		AJFREE(match);
	      }
	    }
	    else                                                         /* the two clusers need to be merged as each name */
	      mergeTwoClusters(list,cluster1,cluster2,match);     /* is in a different cluster */
	}
	else if(cluster1) {                                            /* if name1 found only */
	  ajListAppend(cluster1->matches, ajListNodesNew(match,NULL));     /* so add the match to it's list */
	  ajTablePut(cluster1->hashTable, match->name2, count);        /* and add name2 to the hash table */
	}	    
	else if(cluster2) {                                            /* vice versa if name2 found only */
	  ajListAppend(cluster2->matches, ajListNodesNew(match,NULL));        
	  ajTablePut(cluster2->hashTable, match->name1, count);
	}
	else {                                                         /* else start a new cluster */
	  AJNEW(clustertop);                                      
	  clustertop->hashTable = ajStrTableNewC(0); /* make hash table for this new cluster */
	  ajTablePut(clustertop->hashTable, match->name1, count);      /* add name1 to the hash table */
	  ajTablePut(clustertop->hashTable, match->name2, count);      /* add name2 to the hash table */
	  clustertop->matches = ajListNewArgs(match,NULL);                /* add match to list of matches for this 
									    cluster */
	  ajListAppend(list,ajListNodesNew(clustertop, NULL));
	}
      }
      else {
	AJNEW0(clustertop);                                      
	clustertop->hashTable = ajStrTableNewC(0);   /* make hash table for this new cluster */
	ajTablePut(clustertop->hashTable, match->name1, count);       /* add name1 to the hash table */
	ajTablePut(clustertop->hashTable, match->name2, count);       /* add name2 to the hash table */
	clustertop->matches = ajListNewArgs(match,NULL);                 /* add match to list of matches for this 
									   cluster */

	list = ajListNewArgs(clustertop,NULL);                           /* make list of clusters */
      }
    }
    else
      ajErr ("error reading file!!!!!!! %d",num);
  }

  i=0;
  if(list){
    AjIList iter = ajListIter(list);
    struct tableTop *pos =0;
    ajFmtPrintF (outfile,
		 "//*****************************************"
		 "**********************\n");
    
    while(ajListIterMore(iter)){         /* for each cluster */
      pos = (struct tableTop*) ajListIterNext(iter);
      if(pos){
	ajFmtPrintF (outfile, "\n");
	if(pos->matches){
	  ajFmtPrintF (outfile, "Cluster_id : \"IRat_%d\"\n",i);
	  cluster_Print_Ace(pos);
	  ajFmtPrintF (outfile, "Seq_cluster      \"consensus.irat%d\"",i++);
	  ajFmtPrintF (outfile, "\n");
	}
      }
    }
    ajListIterFree(iter);
  }
  if(list && consensus){
    AjIList iter = ajListIter(list);
    struct tableTop *pos =0;
    embWordLength(10);
    while(ajListIterMore(iter)){         /* for each cluster */
      pos = (struct tableTop*) ajListIterNext(iter);
      if(pos){
	if(pos->matches)
	  consensus_output(pos->matches,1,ajTableLength(pos->hashTable));
      }
    }
    ajListIterFree(iter);
  }
  cleanup_Cluster(list);

  ajExit();
  return 0;
} 


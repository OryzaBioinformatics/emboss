#include "ajax.h"

typedef struct SeqSOutFormat {
  char *Name;
  AjBool Single;
  AjBool Save;
  void (*Write) (AjPSeqout outseq);
} SeqOOutFormat, *SeqPOutFormat;

typedef struct SeqSSeqFormat {
  int namewidth;
  int numwidth;
  int spacer;
  int width;
  int tab;
  int linepos;
  AjBool nameright;
  AjBool nameleft;
  AjBool numright;
  AjBool numleft;
  int numline;
  AjBool skipbefore;
  AjBool skipafter;
  AjBool isactive;
  AjBool baseonlynum;
  AjBool numtop;
  AjBool numbot;
  AjBool nametop;
  AjBool numjust;

  AjBool noleaves;
  AjBool domatch;
  AjBool degap;
  AjBool pretty;
  char endstr[20];
  char gapchar;
  char matchchar;
  char padding[2];	/* Be friendly with the compiler: AJB */
  int interline;

} SeqOSeqFormat, *SeqPSeqFormat;

static int seqSpaceAll = -9;


static void       seqAllClone (AjPSeqout outseq, AjPSeq seq);
static void       seqClone (AjPSeqout outseq, AjPSeq seq);
static void       seqDbName (AjPStr* name, AjPStr db);
static void       seqDeclone (AjPSeqout outseq);
static void       seqDefName (AjPStr* name, AjBool multi);
static AjBool     seqFileReopen (AjPSeqout outseq);
/*static AjBool     seqFindOutFormat (AjPStr format, int *iformat);*/
static AjBool     seqoutUsaProcess (AjPSeqout thys);
static void       seqsetClone (AjPSeqout outseq, AjPSeqset seq, int i);

static void       seqSeqFormat (int seqlen, SeqPSeqFormat* psf);
static void       seqWriteAcedb (AjPSeqout outseq);
static void       seqWriteAsn1 (AjPSeqout outseq);
static void       seqWriteClustal (AjPSeqout outseq);
static void       seqWriteCodata (AjPSeqout outseq);
static void       seqWriteDebug (AjPSeqout outseq);
static void       seqWriteEmbl (AjPSeqout outseq);
static void       seqWriteFasta (AjPSeqout outseq);
static void       seqWriteFitch (AjPSeqout outseq);
static void       seqWriteGcg (AjPSeqout outseq);
static void       seqWriteGenbank (AjPSeqout outseq);
static void       seqWriteHennig86 (AjPSeqout outseq);
static void       seqWriteIg (AjPSeqout outseq);
static void       seqWriteJackknifer (AjPSeqout outseq);
static void       seqWriteJackknifernon (AjPSeqout outseq);
static void       seqWriteListAppend (AjPSeqout outseq, AjPSeq seq);
static void       seqWriteMega (AjPSeqout outseq);
static void       seqWriteMeganon (AjPSeqout outseq);
static void       seqWriteMsf (AjPSeqout outseq);
static void       seqWriteNbrf (AjPSeqout outseq);
static void       seqWriteNcbi (AjPSeqout outseq);
static void       seqWriteNexus (AjPSeqout outseq);
static void       seqWriteNexusnon (AjPSeqout outseq);
static void       seqWritePhylip (AjPSeqout outseq);
static void       seqWritePhylip3 (AjPSeqout outseq);
static void       seqWriteSeq (AjPSeqout outseq, SeqPSeqFormat sf);
static void       seqWriteStaden (AjPSeqout outseq);
static void       seqWriteStrider (AjPSeqout outseq);
static void       seqWriteSwiss (AjPSeqout outseq);
static void       seqWriteText (AjPSeqout outseq);
static void       seqWriteTreecon (AjPSeqout outseq);

static SeqOOutFormat seqOutFormat[] = { /* AJFALSE = write one file */
  {"unknown",    AJFALSE, AJFALSE, seqWriteFasta}, /* internal default writes FASTA */
                                 /* set 'fasta' in ajSeqOutFormatDefault */
  {"gcg",        AJTRUE,  AJFALSE, seqWriteGcg},
  {"gcg8",       AJTRUE,  AJFALSE, seqWriteGcg},
  {"embl",       AJFALSE, AJFALSE, seqWriteEmbl},
  {"em",         AJFALSE, AJFALSE, seqWriteEmbl},
  {"swiss",      AJFALSE, AJFALSE, seqWriteSwiss},
  {"sw",         AJFALSE, AJFALSE, seqWriteSwiss},
  {"fasta",      AJFALSE, AJFALSE, seqWriteFasta},
  {"pearson",    AJFALSE, AJFALSE, seqWriteFasta}, /* Pearson = Fasta */
  {"ncbi",       AJFALSE, AJFALSE, seqWriteNcbi},
  {"nbrf",       AJFALSE, AJFALSE, seqWriteNbrf},
  {"pir",        AJFALSE, AJFALSE, seqWriteNbrf},
  {"genbank",    AJFALSE, AJFALSE, seqWriteGenbank},
  {"gb",         AJFALSE, AJFALSE, seqWriteGenbank},
  {"ig",         AJFALSE, AJFALSE, seqWriteIg},
  {"codata",     AJFALSE, AJFALSE, seqWriteCodata},
  {"strider",    AJFALSE, AJFALSE, seqWriteStrider},
  {"acedb",      AJFALSE, AJFALSE, seqWriteAcedb},
  {"experiment", AJTRUE,  AJFALSE, seqWriteStaden},
  {"staden",     AJTRUE,  AJFALSE, seqWriteStaden},
  {"text",       AJTRUE,  AJFALSE, seqWriteText},
  {"plain",      AJTRUE,  AJFALSE, seqWriteText},
  {"raw",        AJTRUE,  AJFALSE, seqWriteText},
  {"fitch",      AJFALSE, AJFALSE, seqWriteFitch},
  {"msf",        AJFALSE, AJTRUE,  seqWriteMsf},
  {"clustal",    AJFALSE, AJTRUE,  seqWriteClustal},
  {"aln",        AJFALSE, AJTRUE,  seqWriteClustal},
  {"phylip",     AJFALSE, AJTRUE,  seqWritePhylip},
  {"phylip3",    AJFALSE, AJTRUE,  seqWritePhylip3},
  {"asn1",       AJFALSE, AJFALSE, seqWriteAsn1},
  {"hennig86",   AJFALSE, AJTRUE,  seqWriteHennig86},
  {"mega",       AJFALSE, AJTRUE,  seqWriteMega},
  {"meganon",    AJFALSE, AJTRUE,  seqWriteMeganon},
  {"nexus",      AJFALSE, AJTRUE,  seqWriteNexus},
  {"nexusnon",   AJFALSE, AJTRUE,  seqWriteNexusnon},
  {"paup",       AJFALSE, AJTRUE,  seqWriteNexus},
  {"paupnon",    AJFALSE, AJTRUE,  seqWriteNexusnon},
  {"jackknifer", AJFALSE, AJTRUE,  seqWriteJackknifer},
  {"jackknifernon", AJFALSE, AJTRUE,  seqWriteJackknifernon},
  {"treecon",    AJFALSE, AJTRUE,  seqWriteTreecon},
  {"debug",      AJFALSE, AJFALSE, seqWriteDebug},
  {NULL, 0, 0, NULL} };


/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Stream Operators ****************************************
**
** These functions use the contents of a sequence stream object but do
** not make any changes.
**
******************************************************************************/

/* @func ajSeqAllWrite ********************************************************
**
** Write next sequence out - continue until done.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

void ajSeqAllWrite (AjPSeqout outseq, AjPSeq seq) {

  ajDebug ("ajSeqAllWrite '%s' len: %d\n", ajSeqName(seq), ajSeqLen(seq));

  if (!outseq->Format) {
    if (!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format)) {
      ajErr ("unknown output format '%S'", outseq->Formatstr);
    }
  }

  ajDebug ("ajSeqAllWrite %d '%s' single: %B feat: %B Save: %B\n",
	   outseq->Format,
	   seqOutFormat[outseq->Format].Name,
	   seqOutFormat[outseq->Format].Single,
	   outseq->Features,
	   seqOutFormat[outseq->Format].Save);


  if (seqOutFormat[outseq->Format].Save) {
    seqWriteListAppend (outseq, seq);
    outseq->Count++;
    return;
  }

  seqAllClone (outseq, seq);

  if (outseq->Single)
    (void) seqFileReopen(outseq);

  if (outseq->Features)
    ajWarn("ajSeqAllWrite Features not yet implemented");

  seqOutFormat[outseq->Format].Write (outseq);
  outseq->Count++;
  seqDeclone (outseq);

  return;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Sequence Set Operators ********************************************
**
** These functions use the contents of a sequence set object but do
** not make any changes.
**
******************************************************************************/

/* @func ajSeqsetWrite ********************************************************
**
** Write a set of sequences out.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeqset] Sequence set.
** @return [void]
** @@
******************************************************************************/

void ajSeqsetWrite (AjPSeqout outseq, AjPSeqset seq) {

  int i = 0;

  ajDebug("ajSeqsetWrite\n");

  if (!outseq->Format) {
    if (!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format)) {
      ajErr ("unknown output format '%S'", outseq->Formatstr);
    }
  }

  ajDebug ("ajSeqSetWrite %d '%s' single: %B feat: %B Save: %B\n",
	   outseq->Format,
	   seqOutFormat[outseq->Format].Name,
	   seqOutFormat[outseq->Format].Single,
	   outseq->Features,
	   seqOutFormat[outseq->Format].Save);

  for (i=0; i < seq->Size; i++) {

    if (seqOutFormat[outseq->Format].Save) {
      seqWriteListAppend (outseq, seq->Seq[i]);
      outseq->Count++;
      continue;
    }

    seqsetClone (outseq, seq, i);
    if (outseq->Single)
      (void) seqFileReopen(outseq);

    if (outseq->Features)
      ajWarn("ajSeqsetWrite Features not yet implemented");

    seqOutFormat[outseq->Format].Write (outseq);
    outseq->Count++;
    seqDeclone (outseq);
  }

  return;
}

/* @funcstatic seqWriteListAppend ********************************************
**
** Add the latest sequence to the output list. If we are in single
** sequence mode, also write it out now though it does not seem
** a great idea in most cases to ask for this.
**
** @param [P] outseq [AjPSeqout] Sequence output
** @param [P] seq [AjPSeq] Sequence to be appended
** @return [void]
** @@
******************************************************************************/

static void seqWriteListAppend (AjPSeqout outseq, AjPSeq seq) {

  AjPSeq listseq;

  ajDebug ("seqWriteListAppend '%F' %S\n", outseq->File, ajSeqGetName(seq));

  if (!outseq->Savelist)
    outseq->Savelist = ajListNew();


  listseq = ajSeqNewS(seq);
  seqDefName (&listseq->Name, !outseq->Single);

  ajListPushApp(outseq->Savelist, listseq);
  
  if (outseq->Single) {
    ajDebug("single sequence mode: write immediately\n");
    seqDefName (&outseq->Name, !outseq->Single);
    seqOutFormat[outseq->Format].Write (outseq);
  }

  if (outseq->Features)
    ajWarn ("seqWriteListAppend features output not yet implemented");

  return;
}

/* @func ajSeqWriteClose ******************************************************
**
** Close a sequence output file. For formats that save everything up
** and write at the end, call the Write function first.
**
** @param [P] outseq [AjPSeqout] Sequence output
** @return [void]
** @@
******************************************************************************/

void ajSeqWriteClose (AjPSeqout outseq) {

  ajDebug ("ajSeqWriteClose '%F'\n", outseq->File);

  if (seqOutFormat[outseq->Format].Save)
    seqOutFormat[outseq->Format].Write (outseq);

  ajFileClose (&outseq->File);
  return;
}

/* @func ajSeqWrite ***********************************************************
**
** Write a sequence out. For formats that save everything up
** and write at the end, just append to the output list.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @param [P] seq [AjPSeq] Sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqWrite (AjPSeqout outseq, AjPSeq seq) {

  if (!outseq->Format) {
    if (!ajSeqFindOutFormat(outseq->Formatstr, &outseq->Format)) {
      ajErr ("unknown output format '%S'", outseq->Formatstr);
    }
  }

  ajDebug ("ajSeqWrite %d '%s' single: %B feat: %B Save: %B\n",
	   outseq->Format,
	   seqOutFormat[outseq->Format].Name,
	   seqOutFormat[outseq->Format].Single,
	   outseq->Features,
	   seqOutFormat[outseq->Format].Save);

  if (seqOutFormat[outseq->Format].Save) {
    seqWriteListAppend (outseq, seq);
    outseq->Count++;
    return;
  }

  seqClone (outseq, seq);

  seqDefName (&outseq->Name, !outseq->Single);

  if (outseq->Single)
    (void) seqFileReopen(outseq);

  seqOutFormat[outseq->Format].Write (outseq);

  if (outseq->Features) {
    (void) ajStrSet (&outseq->Ftquery->Seqname, seq->Name);
    ajFeatTrace(outseq->Fttable);
    if (!ajFeatWrite (outseq->Fttable, outseq->Ftquery, outseq->Ufo)) {
      ajWarn ("seqWrite features output failed UFO: '%S'",
	      outseq->Ufo);
      return;
    }
  }
    
  outseq->Count++;
  seqDeclone (outseq);

  return;
}

/* @funcstatic seqWriteFasta **************************************************
**
** Writes a sequence in FASTA format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteFasta (AjPSeqout outseq) {

  int i;
  int ilen;
  static AjPStr seq = NULL;
  int linelen = 60;

  seqDbName (&outseq->Name, outseq->Setdb);

  (void) ajFmtPrintF (outseq->File, ">%S", outseq->Name);
  if (ajStrLen(outseq->Acc))
    (void) ajFmtPrintF (outseq->File, " %S", outseq->Acc);
  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, " %S", outseq->Desc);
  (void) ajFmtPrintF (outseq->File, "\n");

  ilen = ajStrLen(outseq->Seq);
  for (i=0; i < ilen; i += linelen) {
    (void) ajStrAssSub (&seq, outseq->Seq, i, i+linelen-1);
    (void) ajFmtPrintF (outseq->File, "%S\n", seq);
  }

  return;
}

/* @funcstatic seqWriteNcbi ***************************************************
**
** Writes a sequence in NCBI format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNcbi (AjPSeqout outseq) {

  int i;
  int ilen;
  static AjPStr seq = NULL;
  int linelen = 60;

  (void) ajFmtPrintF (outseq->File, ">gnl|0|");
  if (ajStrLen(outseq->Db))
    (void) ajFmtPrintF (outseq->File, "%S", outseq->Db);
  else
    (void) ajFmtPrintF (outseq->File, "unk");
  (void) ajFmtPrintF (outseq->File, "|%S", outseq->Acc);
  (void) ajFmtPrintF (outseq->File, "|%S", outseq->Name);
  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, " %S", outseq->Desc);
  (void) ajFmtPrintF (outseq->File, "\n");

  ilen = ajStrLen(outseq->Seq);
  for (i=0; i < ilen; i += linelen) {
    (void) ajStrAssSub (&seq, outseq->Seq, i, i+linelen-1);
    (void) ajFmtPrintF (outseq->File, "%S\n", seq);
  }

  return;
}

/* @funcstatic seqWriteGcg ****************************************************
**
** Writes a sequence in GCG format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGcg (AjPSeqout outseq) {

  int ilen = ajStrLen(outseq->Seq);
  char ctype = 'N';
  int check;
  SeqPSeqFormat sf = NULL;

  if (!outseq->Type)
    (void) ajFmtPrintF (outseq->File, "!!NA_SEQUENCE 1.0\n\n");
  else if (ajStrMatchC(outseq->Type, "P")) {
    (void) ajFmtPrintF (outseq->File, "!!AA_SEQUENCE 1.0\n\n");
    ctype = 'P';
  }
  else 
    (void) ajFmtPrintF (outseq->File, "!!NA_SEQUENCE 1.0\n\n");

  ajSeqGapS (&outseq->Seq, '.');
  check = ajSeqoutCheckGcg (outseq);

  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, "%S\n\n", outseq->Desc);

  (void) ajFmtPrintF (outseq->File, "%S  Length: %d  Type: %c  Check: %4d ..\n",
	   outseq->Name, ilen, ctype, check);

  if (sf)
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  else {
    seqSeqFormat(ajStrLen(outseq->Seq), &sf);
    sf->spacer = 11;
    sf->numleft = ajTrue;
    sf->skipbefore = ajTrue;
    (void) strcpy (sf->endstr, "\n");	/* to help with misreads at EOF */
  }

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteStaden *************************************************
**
** Writes a sequence in Staden format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteStaden (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  ajFmtPrintF (outseq->File, "<%S---->\n", outseq->Name);
  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  
  sf->width = 60;
  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteText ***************************************************
**
** Writes a sequence in plain Text format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteText (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  
  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteHennig86 ************************************************
**
** Writes a sequence in Hennig86 format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteHennig86 (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  char* cp;

  ajDebug ("seqWriteHennig86 list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "xread\n");
  
  (void) ajFmtPrintF (outseq->File, /* title text */
		      "' Written by EMBOSS %D '\n", ajTimeToday());
  
  (void) ajFmtPrintF (outseq->File, /* length, count */
		      "%d %d\n", ilen, isize);
  
  for (i=0; i < isize; i++) {	/* loop over sequences */
    seq = seqarr[i];
    (void) ajStrAss(&sseq, seq->Seq);
    
    cp = ajStrStr(sseq);
    while (*cp) {
      switch (*cp) {
      case 'A':
      case 'a':
	*cp = '0';
	break;
      case 'T':
      case 't':
      case 'U':
      case 'u':
	*cp = '1';
	break;
      case 'G':
      case 'g':
	*cp = '2';
	break;
      case 'C':
      case 'c':
	*cp = '3';
	break;
      default:
	*cp = '?';
	break;
      }
      cp++;
    }
    (void) ajFmtPrintF (outseq->File,
			"%S\n%S\n",
			seq->Name, sseq);
  }  

  (void) ajFmtPrintF (outseq->File, /* terminate with ';' */
		      ";\n", ilen, isize);
  
  return;
}

/* @funcstatic seqWriteMega ************************************************
**
** Writes a sequence in Mega format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMega (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;
  int wid = 50;

  ajDebug ("seqWriteMega list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "#mega\n");
  (void) ajFmtPrintF (outseq->File, /* dummy title */
		      "TITLE: Written by EMBOSS %D\n", ajTimeToday());
  
  for (ipos=1; ipos <= ilen; ipos += wid) { /* interleaved */
    iend = ipos + wid -1;
    if (iend > ilen)
      iend = ilen;

    (void) ajFmtPrintF (outseq->File, /* blank space for comments */
			"\n");
    for (i=0; i < isize; i++) {	/* loop over sequences */
      seq = seqarr[i];
      (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
      ajSeqGapS(&sseq, '-');
      (void) ajFmtPrintF (outseq->File,
			  "#%-20.20S %S\n",
			  seq->Name, sseq);
    }
  }  

  return;
}

/* @funcstatic seqWriteMeganon ************************************************
**
** Writes a sequence in Mega non-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMeganon (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;

  ajDebug ("seqWriteMeganon list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "#mega\n");
  (void) ajFmtPrintF (outseq->File, /* dummy title */
		      "TITLE: Written by EMBOSS %D\n", ajTimeToday());
  (void) ajFmtPrintF (outseq->File, /* blank space for comments */
		      "\n");
  
  for (i=0; i < isize; i++) {	/* loop over sequences */
    seq = seqarr[i];
    (void) ajStrAss(&sseq, seq->Seq);
    ajSeqGapS(&sseq, '-');
    (void) ajFmtPrintF (outseq->File,
			"#%-20.20S\n%S\n",
			seq->Name, sseq);
  }  

  return;
}

/* @funcstatic seqWriteNexus ************************************************
**
** Writes a sequence in Nexus interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNexus (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;
  int wid = 50;

  ajDebug ("seqWriteNexus list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "#NEXUS\n");
  (void) ajFmtPrintF (outseq->File, /* dummy title */
		      "[TITLE: Written by EMBOSS %D]\n\n", ajTimeToday());
  (void) ajFmtPrintF (outseq->File,
		      "begin data;\n");
  (void) ajFmtPrintF (outseq->File, /* count, length */
		      "dimensions ntax=%d nchar=%d;\n", isize, ilen);
  (void) ajFmtPrintF (outseq->File,
		      "format interleave datatype=DNA missing=N gap=-;\n");
  (void) ajFmtPrintF (outseq->File, "\n");

  (void) ajFmtPrintF (outseq->File,
		      "matrix\n");
  for (ipos=1; ipos <= ilen; ipos += wid) { /* interleaved */
    iend = ipos +wid -1;
    if (iend > ilen)
      iend = ilen;

    if (ipos > 1)
      (void) ajFmtPrintF (outseq->File, "\n");

    for (i=0; i < isize; i++) {	/* loop over sequences */
      seq = seqarr[i];
      (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
      ajSeqGapS(&sseq, '-');
      (void) ajFmtPrintF (outseq->File,
		   "%-20.20S %S\n",
		   seq->Name, sseq);
    }
  }  

  (void) ajFmtPrintF (outseq->File,
		      ";\n\n");
  (void) ajFmtPrintF (outseq->File,
		      "endblock;\n");
  (void) ajFmtPrintF (outseq->File,
		      "begin assumptions;\n");
  (void) ajFmtPrintF (outseq->File,
		      "options deftype=unord;\n");
  return;
}

/* @funcstatic seqWriteNexusnon **********************************************
**
** Writes a sequence in Nexus non-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNexusnon (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;

  ajDebug ("seqWriteNexusnon list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "#NEXUS\n");
  (void) ajFmtPrintF (outseq->File, /* dummy title */
		      "[TITLE: Written by EMBOSS %D]\n\n", ajTimeToday());
  (void) ajFmtPrintF (outseq->File,
		      "begin data;\n");
  (void) ajFmtPrintF (outseq->File, /* count, length */
		      "dimensions ntax=%d nchar=%d;\n", isize, ilen);
  (void) ajFmtPrintF (outseq->File,
		      "format datatype=DNA missing=N gap=-;\n");
  (void) ajFmtPrintF (outseq->File, "\n");

  (void) ajFmtPrintF (outseq->File,
		      "matrix\n");
  for (i=0; i < isize; i++) {	/* loop over sequences */
    seq = seqarr[i];
    (void) ajStrAss(&sseq, seq->Seq);
    ajSeqGapS(&sseq, '-');
    (void) ajFmtPrintF (outseq->File,
			"%S\n%S\n",
			seq->Name, sseq);
  }  

  (void) ajFmtPrintF (outseq->File,
		      ";\n\n");
  (void) ajFmtPrintF (outseq->File,
		      "endblock;\n");
  (void) ajFmtPrintF (outseq->File,
		      "begin assumptions;\n");
  (void) ajFmtPrintF (outseq->File,
		      "options deftype=unord;\n");
  return;
}

/* @funcstatic seqWriteJackknifer *********************************************
**
** Writes a sequence in Jackknifer format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteJackknifer (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;
  int wid = 50;
  static AjPStr tmpid = NULL;

  ajDebug ("seqWriteJackknifer list size %d\n",
	   ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "' Written by EMBOSS %D \n", ajTimeToday());
  
  for (ipos=1; ipos <= ilen; ipos += wid) { /* interleaved */
    iend = ipos +wid -1;
    if (iend > ilen)
      iend = ilen;

    for (i=0; i < isize; i++) {	/* loop over sequences */
      seq = seqarr[i];
      (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
      ajSeqGapS(&sseq, '-');
      ajFmtPrintS (&tmpid, "(%S)", seq->Name);
      (void) ajFmtPrintF (outseq->File,
		   "%-20.20S %S\n",
		   tmpid, sseq);
    }
  }  

  (void) ajFmtPrintF (outseq->File, ";\n");

  return;
}

/* @funcstatic seqWriteJackknifernon ******************************************
**
** Writes a sequence in Jackknifer on-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteJackknifernon (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;
  int wid = 50;
  static AjPStr tmpid = NULL;

  ajDebug ("seqWriteJackknifernon list size %d\n",
	   ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* header text */
		      "' Written by EMBOSS %D \n", ajTimeToday());
  
  for (i=0; i < isize; i++) {	/* loop over sequences */
    seq = seqarr[i];
    for (ipos=1; ipos <= ilen; ipos += wid) { /* interleaved */
      iend = ipos +wid -1;
      if (iend > ilen)
	iend = ilen;

      (void) ajStrAssSub (&sseq, seq->Seq, ipos-1, iend-1);
      ajSeqGapS(&sseq, '-');
      if (ipos == 1) {
	ajFmtPrintS (&tmpid, "(%S)", seq->Name);
	(void) ajFmtPrintF (outseq->File,
			    "%-20.20S %S\n",
			    tmpid, sseq);
      }
      else
	(void) ajFmtPrintF (outseq->File,
			    "%S\n",
			    sseq);
    }
  }  

  (void) ajFmtPrintF (outseq->File, ";\n");

  return;
}

/* @funcstatic seqWriteTreecon ************************************************
**
** Writes a sequence in Treecon format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteTreecon (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;

  ajDebug ("seqWriteTreecon list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, /* count */
		      "%d\n", ilen);
  
  for (i=0; i < isize; i++) {	/* loop over sequences */
    seq = seqarr[i];
    (void) ajStrAss(&sseq, seq->Seq);
    ajSeqGapS(&sseq, '-');
    (void) ajFmtPrintF (outseq->File,
			"%S\n%S\n",
			seq->Name, sseq);
  }  

  return;
}

/* @funcstatic seqWriteClustal ************************************************
**
** Writes a sequence in Clustal (ALN) format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteClustal (AjPSeqout outseq) {

/*  static SeqPSeqFormat sf=NULL;*/

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;

  ajDebug ("seqWriteClustal list size %d\n", ajListLength(outseq->Savelist));


  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File,
		      "CLUSTAL W(1.4) multiple sequence alignment\n");
  
  (void) ajFmtPrintF (outseq->File, "\n\n");

  for (ipos=1; ipos <= ilen; ipos += 50) {
    iend = ipos + 50 -1;
    if (iend > ilen)
      iend = ilen;

    for (i=0; i < isize; i++) {
      seq = seqarr[i];
      (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
      ajSeqGapS(&sseq, '-');
      (void) ajStrBlock (&sseq, 10);
      (void) ajFmtPrintF (outseq->File,
		   "%-15.15S %S\n",
		   seq->Name, sseq);
    }
    (void) ajFmtPrintF (outseq->File, "%-15.15s %54.54s\n", "", ""); /* *. conserved line */
    (void) ajFmtPrintF (outseq->File, "\n"); /* blank line */
  }  

  return;
}

/* @funcstatic seqWriteMsf ***************************************************
**
** Writes a sequence in GCG Multiple Sequence File format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteMsf (AjPSeqout outseq) {

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int checktot = 0;
  int check;
  int itest;
  static AjPStr sbeg = NULL;
  static AjPStr send = NULL;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;
  int igap;

  ajDebug ("seqWriteMsf list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);

  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    ajSeqGapLen (seq, '.', '~', ilen); /* need to pad if any are shorter */
    check = ajSeqCheckGcg(seq);
    ajDebug (" '%S' len: %d checksum: %d\n",
	     ajSeqGetName(seq), ajSeqLen(seq), check);
    checktot += check;
    checktot = checktot % 10000;
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  ajDebug ("checksum %d\n", checktot);
  ajDebug ("outseq->Type '%S'\n", outseq->Type);

  if (!ajStrLen(outseq->Type)) {
    ajSeqType (seqarr[0]);
    (void) ajStrSet(&outseq->Type, seqarr[0]->Type);
  }
  ajDebug ("outseq->Type '%S'\n", outseq->Type);

  if (ajStrMatchC(outseq->Type, "P")) {
    (void) ajFmtPrintF (outseq->File, "!!AA_MULTIPLE_ALIGNMENT 1.0\n\n");
    (void) ajFmtPrintF (outseq->File,
			"  %F MSF:  %d Type: P %D CompCheck: %4d ..\n\n",
			outseq->File, ilen, ajTimeToday(), checktot);
  }
  else {
    (void) ajFmtPrintF (outseq->File, "!!NA_MULTIPLE_ALIGNMENT 1.0\n\n");
    (void) ajFmtPrintF (outseq->File,
			"  %F MSF: %d Type: N %D CompCheck: %4d ..\n\n",
			outseq->File, ilen, ajTimeToday(), checktot);
  }

  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    check = ajSeqCheckGcg(seq);
    (void) ajFmtPrintF (outseq->File,
			"  Name: %S Len: %d  Check: %4d Weight: %.2f\n",
			seq->Name, ajStrLen(seq->Seq),
			check, seq->Weight);
  }  
  
  (void) ajFmtPrintF (outseq->File, "\n//\n\n");

  for (ipos=1; ipos <= ilen; ipos += 50) {
    iend = ipos + 50 -1;
    if (iend > ilen)
      iend = ilen;
    (void) ajFmtPrintS (&sbeg, "%d", ipos);
    (void) ajFmtPrintS (&send, "%d", iend);
    if (iend == ilen) {
      igap = iend - ipos - ajStrLen(sbeg);
      ajDebug ("sbeg: %S send: %S ipos: %d iend: %d igap: %d len: %d\n",
	       sbeg, send, ipos, iend, igap, ajStrLen(send));
      if (igap >= ajStrLen(send))
	(void) ajFmtPrintF (outseq->File,
			    "           %S %*S\n", sbeg, igap, send);
      else
	(void) ajFmtPrintF (outseq->File, "           %S\n", sbeg);
    }
    else
      (void) ajFmtPrintF (outseq->File, "           %-25S%25S\n", sbeg, send);
    for (i=0; i < isize; i++) {
      seq = seqarr[i];
      check = ajSeqCheckGcg(seq);
      (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
      (void) ajFmtPrintF (outseq->File,
		   "%-10S %S\n",
		   seq->Name, sseq);
    }
    (void) ajFmtPrintF (outseq->File, "\n");
  }  

  /*
  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  
  seqWriteSeq (outseq, sf);
  */
  ajListDel(&outseq->Savelist);

  return;
}

/* @funcstatic seqWriteCodata *************************************************
**
** Writes a sequence in Codata format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteCodata (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;
  int j;

  (void) ajFmtPrintF (outseq->File, "ENTRY           %S \n", outseq->Name);
  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, "TITLE           %S, %d bases\n",
	     outseq->Desc, ajStrLen(outseq->Seq));
  if (ajStrLen(outseq->Acc))
    (void) ajFmtPrintF (outseq->File, "ACCESSION       %S\n",
	     outseq->Acc);
  (void) ajFmtPrintF (outseq->File, "SEQUENCE        \n");

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  sf->numwidth = 7;
  sf->width = 30;
  sf->numleft = ajTrue;
  sf->spacer = seqSpaceAll;
  (void) strcpy(sf->endstr, "\n///");

  for (j = 0; j <= sf->numwidth; j++)
    (void) ajFmtPrintF(outseq->File, " ");
  for (j = 5; j <= sf->width; j+=5)
    (void) ajFmtPrintF(outseq->File, "%10d", j);
  (void) ajFmtPrintF(outseq->File, "\n");

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteNbrf ***************************************************
**
** Writes a sequence in NBRF format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteNbrf (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  if (!outseq->Type)
    (void) ajFmtPrintF (outseq->File, ">D1;%S\n", outseq->Name);
  else if (ajStrMatchC(outseq->Type, "P")) {
    (void) ajFmtPrintF (outseq->File, ">P1;%S\n", outseq->Name);
  }
  else
    (void) ajFmtPrintF (outseq->File, ">D1;%S\n", outseq->Name);

  (void) ajFmtPrintF (outseq->File, "%S, %d bases\n",
	   outseq->Desc, ajStrLen(outseq->Seq));

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  sf->spacer = 11;
  (void) strcpy(sf->endstr, "*\n");
  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteEmbl ***************************************************
**
** Writes a sequence in EMBL format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteEmbl (AjPSeqout outseq) {

  static SeqPSeqFormat sf = NULL;
  int b[5];

  if (ajStrMatchC(outseq->Type, "P")) {
      seqWriteSwiss (outseq);
      return;
  }

  (void) ajFmtPrintF (outseq->File,
	       "ID   %-10.10S standard; DNA; UNC; %d BP.\n",
	       outseq->Name, ajStrLen(outseq->Seq));
  if (ajStrLen(outseq->Acc))
    (void) ajFmtPrintF (outseq->File, "AC   %S;\n", outseq->Acc);
  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, "DE   %S\n", outseq->Desc);
  ajSeqCount (outseq->Seq, b);
  (void) ajFmtPrintF (outseq->File,
	       "SQ   Sequence %d BP; %d A; %d C; %d G; %d T; %d other;\n",
	       ajStrLen(outseq->Seq), b[0], b[1], b[2], b[3], b[4]);

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  (void) strcpy (sf->endstr, "\n//");
  sf->tab = 4;
  sf->spacer = 11;
  sf->width = 60;
  sf->numright = ajTrue;
  sf->numwidth = 9;
  sf->numjust = ajTrue;

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteSwiss **************************************************
**
** Writes a sequence in SWISSPROT format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSwiss (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;
  int mw;
  unsigned int crc;

  if (ajStrMatchC(outseq->Type, "N")) {
      seqWriteEmbl (outseq);
      return;
  }

  (void) ajFmtPrintF (outseq->File,
	       "ID   %-10.10S     STANDARD;      PRT; %5d AA.\n",
	       outseq->Name, ajStrLen(outseq->Seq));
  if (ajStrLen(outseq->Acc))
    (void) ajFmtPrintF (outseq->File, "AC   %S;\n", outseq->Acc);
  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, "DE   %S\n", outseq->Desc);
  crc = ajSeqCrc (outseq->Seq);
  mw = (int) (0.5+ajSeqMW (outseq->Seq));
  (void) ajFmtPrintF (outseq->File,
	       "SQ   SEQUENCE %5d AA; %6d MW;  %08X CRC32;\n",
	       ajStrLen(outseq->Seq), mw, crc);

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  (void) strcpy (sf->endstr, "\n//");
  sf->tab = 4;
  sf->spacer = 11;
  sf->width = 60;

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteGenbank ************************************************
**
** Writes a sequence in GENBANK format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteGenbank (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;
  int b[5];

  (void) ajFmtPrintF (outseq->File, "LOCUS       %S\n", outseq->Name);
  if (ajStrLen(outseq->Acc))
    (void) ajFmtPrintF (outseq->File, "ACCESSION   %S\n", outseq->Acc);
  if (ajStrLen(outseq->Desc))
    (void) ajFmtPrintF (outseq->File, "DEFINITION  %S\n", outseq->Desc);
  ajSeqCount (outseq->Seq, b);
  if (b[4])
    (void) ajFmtPrintF (outseq->File,
		 "BASE COUNT   %6d a %6d c %6d g %6d t %6d others\n",
		 b[0], b[1], b[2], b[3], b[4]);
  else
    (void) ajFmtPrintF (outseq->File,
		 "BASE COUNT   %6d a %6d c %6d g %6d t\n",
		 b[0], b[1], b[2], b[3]);
  (void) ajFmtPrintF (outseq->File, "ORIGIN\n");

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  (void) strcpy (sf->endstr, "\n//");
  sf->tab = 1;
  sf->spacer = 11;
  sf->width = 60;
  sf->numleft = ajTrue;
  sf->numwidth = 8;

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteStrider ************************************************
**
** Writes a sequence in DNA STRIDER format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteStrider (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  (void) ajFmtPrintF (outseq->File, "; ### from DNA Strider ;-)\n");
  (void) ajFmtPrintF (outseq->File, "; DNA sequence  %S, %d bases\n;\n",
	   outseq->Name, ajStrLen(outseq->Seq));

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  (void) strcpy(sf->endstr, "\n//");

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteFitch **************************************************
**
** Writes a sequence in FITCH format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteFitch (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  (void) ajFmtPrintF (outseq->File, "%S, %d bases\n",
	   outseq->Name, ajStrLen(outseq->Seq));

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  sf->spacer = 4;
  sf->width = 60;

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWritePhylip *************************************************
**
** Writes a sequence in PHYLIP interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWritePhylip (AjPSeqout outseq) {

  int isize;
  int ilen=0;
  int i = 0;
  void** seqs;
  AjPSeq seq;
  AjPSeq* seqarr;
  int itest;
  static AjPStr sseq = NULL;
  int ipos;
  int iend;

  ajDebug ("seqWritePhylip list size %d\n", ajListLength(outseq->Savelist));

  isize = ajListLength(outseq->Savelist);
  if (!isize) return;

  itest = ajListToArray (outseq->Savelist, (void***) &seqs);
  ajDebug ("ajListToArray listed %d items\n", itest);
  seqarr = (AjPSeq*) seqs;
  for (i=0; i < isize; i++) {
    seq = seqarr[i];
    if (ilen < ajSeqLen(seq))
      ilen = ajSeqLen(seq);
  }  

  (void) ajFmtPrintF (outseq->File, " %d %d\n", isize, ilen);
  
  for (ipos=1; ipos <= ilen; ipos += 50) {
    iend = ipos + 50 -1;
    if (iend > ilen)
      iend = ilen;

    for (i=0; i < isize; i++) {
      seq = seqarr[i];
      (void) ajStrAssSub(&sseq, seq->Seq, ipos-1, iend-1);
      ajSeqGapS(&sseq, '-');
      (void) ajStrBlock (&sseq, 10);
      if (ipos == 1) {
	(void) ajFmtPrintF (outseq->File,
		   "%-12.12S%S\n",
		   seq->Name, sseq);
      }
      else {
	(void) ajFmtPrintF (outseq->File,
		   "%12s%S\n",
		   " ", sseq);
      }
    }
    if (iend < ilen)
      (void) ajFmtPrintF (outseq->File, "\n");
  }  

  return;
}

/* @funcstatic seqWritePhylip3 ************************************************
**
** Writes a sequence in PHYLIP non-interleaved format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWritePhylip3 (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  (void) ajFmtPrintF (outseq->File, " 1 %d YF\n", /* 1 is # sequences */
	   ajStrLen(outseq->Seq)); 
  (void) ajFmtPrintF (outseq->File, "%-10S  ", outseq->Name);

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  sf->linepos = -1;
  sf->tab = 12;
  sf->spacer = 11;
  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteAsn1 ***************************************************
**
** Writes a sequence in ASN.1 format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteAsn1 (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  (void) ajFmtPrintF (outseq->File, "  seq {\n");
  (void) ajFmtPrintF (outseq->File, "    id { local id 1 },\n");
  (void) ajFmtPrintF (outseq->File, "    descr { title \"%S\" },\n",
	   outseq->Desc);
  (void) ajFmtPrintF (outseq->File, "    inst {\n");

  if (!outseq->Type)
    (void) ajFmtPrintF (outseq->File,
	     "      repr raw, mol dna, length %d, topology linear,\n {\n",
	     ajStrLen(outseq->Seq));
  else if (ajStrMatchC(outseq->Type, "P")) {
    (void) ajFmtPrintF (outseq->File,
	     "      repr raw, mol aa, length %d, topology linear,\n {\n",
	     ajStrLen(outseq->Seq));
  }
  else
    (void) ajFmtPrintF (outseq->File,
	     "      repr raw, mol dna, length %d, topology linear,\n",
	     ajStrLen(outseq->Seq));

  (void) ajFmtPrintF (outseq->File, "      seq-data\n");
  if (ajStrMatchC(outseq->Type, "P"))
    (void) ajFmtPrintF (outseq->File, "        iupacaa \"");
  else
    (void) ajFmtPrintF (outseq->File, "        iupacna \"");

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  sf->linepos = 17;
  sf->spacer = 0;
  sf->width = 78;
  sf->tab = 0;
  (void) strcpy (sf->endstr, "\"\n      } } ,");
  
  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteIg *****************************************************
**
** Writes a sequence in INTELLIGENETICS format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteIg (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  (void) ajFmtPrintF (outseq->File, ";%S, %d bases\n",
	   outseq->Desc, ajStrLen(outseq->Seq));
  (void) ajFmtPrintF (outseq->File, "%S\n", outseq->Name);

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  (void) strcpy (sf->endstr, "1");	/* linear (DNA) */
  
  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteAcedb **************************************************
**
** Writes a sequence in ACEDB format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteAcedb (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  if (ajStrMatchC(outseq->Type, "P"))
    (void) ajFmtPrintF (outseq->File, "Peptide : \"%S\"\n", outseq->Name);
  else
    (void) ajFmtPrintF (outseq->File, "DNA : \"%S\"\n", outseq->Name);

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  (void) strcpy (sf->endstr, "\n");

  seqWriteSeq (outseq, sf);

  return;
}

/* @funcstatic seqWriteDebug **************************************************
**
** Writes a sequence in debug report format.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

static void seqWriteDebug (AjPSeqout outseq) {

  static SeqPSeqFormat sf=NULL;

  (void) ajFmtPrintF (outseq->File, "Sequence output trace\n");
  (void) ajFmtPrintF (outseq->File, "=====================\n\n");
  (void) ajFmtPrintF (outseq->File, "  Name: '%S'\n", outseq->Name);
  (void) ajFmtPrintF (outseq->File, "  Accession: '%S'\n", outseq->Acc);
  (void) ajFmtPrintF (outseq->File, "  Description: '%S'\n", outseq->Desc);
  (void) ajFmtPrintF (outseq->File, "  Type: '%S'\n", outseq->Type);
  (void) ajFmtPrintF (outseq->File, "  Database: '%S'\n", outseq->Db);
  (void) ajFmtPrintF (outseq->File, "  Full name: '%S'\n", outseq->Full);
  (void) ajFmtPrintF (outseq->File, "  Date: '%S'\n", outseq->Date);
  (void) ajFmtPrintF (outseq->File, "  Usa: '%S'\n", outseq->Usa);
  (void) ajFmtPrintF (outseq->File, "  Ufo: '%S'\n", outseq->Ufo);
  (void) ajFmtPrintF (outseq->File, "  Input format: '%S'\n", outseq->Informatstr);
  (void) ajFmtPrintF (outseq->File, "  Output format: '%S'\n", outseq->Formatstr);
  (void) ajFmtPrintF (outseq->File, "  Filename: '%S'\n", outseq->Filename);
  (void) ajFmtPrintF (outseq->File, "  Entryname: '%S'\n", outseq->Entryname);
  (void) ajFmtPrintF (outseq->File, "  File name: '%S'\n", outseq->File->Name);
  (void) ajFmtPrintF (outseq->File, "  Extension: '%S'\n", outseq->Extension);
  (void) ajFmtPrintF (outseq->File, "  Single: '%B'\n", outseq->Single);
  (void) ajFmtPrintF (outseq->File, "  Features: '%B'\n", outseq->Features);
  (void) ajFmtPrintF (outseq->File, "  Count: '%B'\n", outseq->Count);
  (void) ajFmtPrintF (outseq->File, "  Documentation:...\n%S\n", outseq->Doc);

  seqSeqFormat(ajStrLen(outseq->Seq), &sf);
  sf->numright = ajTrue;
  sf->numleft = ajTrue;
  sf->numjust = ajTrue;
  sf->tab = 1;
  sf->spacer = 11;
  sf->width = 50;
  
  seqWriteSeq (outseq, sf);

  return;
}

/* @func ajSeqFileNewOut ******************************************************
**
** Opens an output file for sequence writing. 'stdout' and 'stderr' are
** special cases using standard output and standard error respectively.
**
** @param [u] seqout [AjPSeqout] Sequence output object.
** @param [r] name [AjPStr] Output filename.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqFileNewOut (AjPSeqout seqout, AjPStr name) {

  AjBool single = seqout->Single;
  AjBool features = seqout->Features;

  if (ajStrMatchCaseC(name, "stdout"))
    single = ajFalse;
  if (ajStrMatchCaseC(name, "stderr"))
    single = ajFalse;

  if (single) {			/* OK, but nothing to open yet */
    (void) ajStrSet (&seqout->Extension, seqout->Formatstr);
    return ajTrue;
  }
  else {
    seqout->File = ajFileNewOut(name);
    if (seqout->File)
      return ajTrue;
  }

  if (features)
    ajWarn ("ajSeqFileNewOut features not yet implemented");

  return ajFalse;
}

/* @funcstatic seqoutUsaProcess ***********************************************
**
** Converts a USA Universal Sequence Address into an open output file.
**
** First tests for format:: and sets this if it is found
**
** Then looks for file:id and opens the file.
** In this case the file position is not known and sequence reading
** will have to scan for the entry/entries we need.
**
** @param [u] thys [AjPSeqout] Sequence output definition.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

static AjBool seqoutUsaProcess (AjPSeqout thys) {
  static AjPRegexp fmtexp = NULL;
  static AjPRegexp idexp = NULL;

  AjBool fmtstat;
  AjBool regstat;

  static AjPStr usatest = NULL;

  ajDebug("seqoutUsaProcess\n");
  if (!fmtexp)
    fmtexp = ajRegCompC ("^([A-Za-z0-9]*)::?(.*)$");
				/* \1 format */
				/* \2 remainder */

  if (!idexp)			/* \1 is filename \3 is the qryid */
    idexp = ajRegCompC ("^(.*)$");

  (void) ajStrAss (&usatest, thys->Usa);
  ajDebug("output USA to test: '%S'\n\n", usatest);

  fmtstat = ajRegExec (fmtexp, usatest);
  ajDebug("format regexp: %B\n", fmtstat);

  if (fmtstat) {
    ajRegSubI (fmtexp, 1, &thys->Formatstr);
    (void) ajStrSetC (&thys->Formatstr, seqOutFormat[0].Name); /* default unknown */
    ajRegSubI (fmtexp, 2, &usatest);
    ajDebug ("found format %S\n", thys->Formatstr);
    if (!ajSeqFindOutFormat (thys->Formatstr, &thys->Format)) {
      ajDebug ("unknown format '%S'\n", thys->Formatstr);
      return ajFalse;
    }
  }
  else {
    ajDebug("no format specified in USA\n");
  }
  ajDebug ("\n");

  regstat = ajRegExec (idexp, usatest);
  ajDebug ("file:id regexp: %B\n", regstat);

  if (regstat) {
    ajRegSubI (idexp, 1, &thys->Filename);
    ajDebug ("found filename %S single: %B\n",
	     thys->Filename, thys->Single);
    if (thys->Single) {
      ajDebug ("single output file per sequence, open later\n");
    }
    else {
      thys->File = ajFileNewOut (thys->Filename);
      if (!thys->File) {
	ajErr ("failed to open filename %S", thys->Filename);
	return ajFalse;
      }
    }
  }
  else {
    ajDebug ("no filename specified\n");
  }
  ajDebug ("\n");

return ajTrue;
}

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Sequence Output Modifiers *****************************************
**
** These functions use the contents of a sequence output object and
** update them.
**
******************************************************************************/

/* @func ajSeqoutOpen *********************************************************
**
** If the file is not yet open, calls seqoutUsaProcess to convert the USA into
** an open output file stream.
**
** Returns the results in the AjPSeqout object.
**
** @param [w] thys [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqoutOpen (AjPSeqout thys) {

  AjBool ret;

  ret = seqoutUsaProcess (thys);

  if (!ret) return ajFalse;

  if (!thys->Features)
    return ret;

  (void) ajStrSet (&thys->Ftquery->Seqname, thys->Name);
  ret = ajFeatTabOutOpen (thys->Ftquery, thys->Ufo);
  return ret;
}


/* @func ajSeqOutFormatSingle *************************************************
**
** Checks whether an output format should go to single files, rather than
** all sequences being written to one file. Some formats do not work when
** more than one sequence is writte to a file. Obvious examples are plain
** text and GCG formats.
**
** @param [P] format [AjPStr] Output format required.
** @return [AjBool] ajTrue if separate file is needed for each sequence.
** @@
******************************************************************************/

AjBool ajSeqOutFormatSingle (AjPStr format) {
  int iformat;
  if (!ajSeqFindOutFormat (format, &iformat)) {
    ajErr ("Unknown output format %S'", format);
    return ajFalse;
  }

  return seqOutFormat[iformat].Single;
}

/* @func ajSeqOutSetFormat ************************************************
**
** Sets the output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [wP] thys [AjPSeqout] Sequence output object.
** @param [r] format [AjPStr] Output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutSetFormat (AjPSeqout thys, AjPStr format) {

  static AjPStr fmt = NULL;

  ajDebug ("ajSeqOutSetFormat '%S'\n", format);
  (void) ajStrAssS (&fmt, format);
  (void) ajSeqOutFormatDefault(&fmt);

  (void) ajStrSet(&thys->Formatstr, fmt);
  ajDebug ("... output format set to '%S'\n", fmt);

  return ajTrue;
}

/* @func ajSeqOutFormatDefault ************************************************
**
** Sets the default output format. Currently hard coded but will be replaced
** in future by a variable.
**
** @param [wP] pformat [AjPStr*] Default output format.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqOutFormatDefault (AjPStr* pformat) {

  if (ajStrLen(*pformat)) {
    ajDebug ("... output format '%S'\n", *pformat);
  }
  else {
    /* ajStrSetC (pformat, seqOutFormat[0].Name);*/
    (void) ajStrSetC (pformat, "fasta"); /* use the real name */
    ajDebug ("... output format not set, default to '%S'\n", *pformat);
  }

  return ajTrue;
}

/* @func ajSeqPrintOutFormat ************************************************
**
** Reports the internal data structures
**
** @param [r] outf [AjPFile] Output file
** @param [r] full [AjBool] Full report (usually ajFalse)
** @return [void]
** @@
******************************************************************************/

void ajSeqPrintOutFormat (AjPFile outf, AjBool full) {
    
  int i=0;

  ajFmtPrintF (outf, "\n");
  ajFmtPrintF (outf, "# sequence output formats\n");
  ajFmtPrintF (outf, "# Name         Try (if true, try)\n");
  ajFmtPrintF (outf, "\n");
  ajFmtPrintF (outf, "OutFormat {\n");
  for (i=0; seqOutFormat[i].Name; i++) {
    ajFmtPrintF (outf, "  %-12s %B\n",
		 seqOutFormat[i].Name, seqOutFormat[i].Single);
  }
  ajFmtPrintF (outf, "}\n\n");

  return;
}
/* @func ajSeqFindOutFormat ***********************************************
**
** Looks for the specified output format in the internal definitions and
** returns the index.
**
** @param [P] format [AjPStr] Format required.
** @param [w] iformat [int*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSeqFindOutFormat (AjPStr format, int* iformat) {

  AjPStr tmpformat = NULL;
  int i = 0;

  if (!ajStrLen(format))
    return ajFalse;

  (void) ajStrAss (&tmpformat, format);
  (void) ajStrToLower(&tmpformat);

  while (seqOutFormat[i].Name) {
    if (ajStrMatchCaseC(tmpformat, seqOutFormat[i].Name)) {
      *iformat = i;
      ajStrDel(&tmpformat);
      return ajTrue;
    }
    i++;
  }

  ajStrDel(&tmpformat);
  return ajFalse;
}

/* @funcstatic seqSeqFormat ***************************************************
**
** Initialises sequence output formatting parameters.
**
** @param [r] seqlen [int] Sequence length
** @param [uP] psf [SeqPSeqFormat*] Sequence format object
** @return [void]
** @@
******************************************************************************/

static void seqSeqFormat (int seqlen, SeqPSeqFormat* psf) {

  char numform[20];
  SeqPSeqFormat sf;
  int i;
  int j;

  j = 1;

  for (i = seqlen; i; i /= 10)
    j++;

  (void) sprintf(numform, "%d", seqlen);
  ajDebug ("seqSeqFormat numwidth old: %d new: %d\n", strlen(numform)+1, j);

  if (!*psf) {
    sf = AJNEW0(*psf);
    sf->namewidth = 8;
    sf->spacer = 0;
    sf->width = 50;
    sf->tab = 0;
    sf->numleft = ajFalse;
    sf->numright = sf->numleft = sf->numjust = ajFalse;
    sf->nameright = sf->nameleft = ajFalse;
    sf->numline = 0;
    sf->linepos = 0;

    sf->skipbefore = ajFalse;
    sf->skipafter = ajFalse;
    sf->isactive = ajFalse;
    sf->baseonlynum = ajFalse;
    sf->gapchar = '-';
    sf->matchchar = '.';
    sf->noleaves = sf->domatch = sf->degap = ajFalse;
    sf->interline = 1;
    sf->pretty = ajFalse;
    (void) strcpy (sf->endstr, "");
  }
  else {
    sf = *psf;
  }

  sf->numwidth = j; /* or 8 as a reasonable minimum */

  return;
}

/* @func ajSeqoutCheckGcg *****************************************************
**
** Calculates a GCG checksum for an output sequence.
**
** @param [r] outseq [AjPSeqout] Output sequence.
** @return [int] GCG checksum.
** @@
******************************************************************************/

int ajSeqoutCheckGcg (AjPSeqout outseq) {
  register long  i, check = 0, count = 0;
  char *cp = ajStrStr(outseq->Seq);
  int ilen = ajStrLen(outseq->Seq);

  for (i = 0; i < ilen; i++) {
    count++;
    check += count * toupper((int) cp[i]);
    if (count == 57)
      count = 0;
  }
  check %= 10000;

  return check;
}

/* @funcstatic seqWriteSeq ****************************************************
**
** Writes an output sequence. The format and all other information is
** already stored in the output sequence object and the formatting structure.
**
** @param [r] outseq [AjPSeqout] Output sequence.
** @param [w] sf [SeqPSeqFormat] Output formatting structure.
** @return [void]
** @@
******************************************************************************/

static void seqWriteSeq (AjPSeqout outseq, SeqPSeqFormat sf) {

  /* code adapted from what readseq did */

  static int maxSeqWidth = 250;
  static char* defNocountSymbols = "_.-?";

  int i=0, j=0, l=0, ibase=0;
  int linesout = 0;
  int seqlen = ajStrLen(outseq->Seq);
  char *seq = ajStrStr(outseq->Seq);
  char *idword;
  char *cp;
  char s[1024];			/* the output line */

  char nameform[20];
  char numform[20];
  char nocountsymbols[20];

  int namewidth = sf->namewidth;
  int numwidth = sf->numwidth;
  int spacer = sf->spacer;
  int width = sf->width;
  int tab = sf->tab;
  int l1 = sf->linepos;
  AjBool nameleft = sf->nameleft;
  AjBool nameright = sf->nameright;
  AjBool numleft = sf->numleft;
  AjBool numright = sf->numright;
  int numline = 0;
  AjBool numjust = sf->numjust;
  AjBool skipbefore = sf->skipbefore;
  AjBool skipafter = sf->skipafter;
  AjBool baseonlynum = sf->baseonlynum;
  AjBool pretty = sf->pretty;
  char *endstr = sf->endstr;
  AjPFile file = outseq->File;
  FILE* outf = ajFileFp(file);

  ajDebug("seqWriteSeq\n");
  if (sf->numline) numline = 1;

  if (nameleft || nameright)
    (void) sprintf(nameform, "%%%d.%ds ",namewidth,namewidth);
  if (numline)
    (void) sprintf(numform, "%%%ds ",numwidth);
  else
    (void) sprintf(numform, "%%%dd",numwidth);

  (void) strcpy( nocountsymbols, defNocountSymbols);
  if (baseonlynum) {		/* add gap character to skips */
    if (strchr(nocountsymbols,sf->gapchar)==NULL) {
      (void) strcat(nocountsymbols," ");
      nocountsymbols[strlen(nocountsymbols)-1]= sf->gapchar;
    }
    if (sf->domatch &&		/* remove gap character from skips */
	(cp=strchr(nocountsymbols,sf->matchchar))!=NULL) {
      *cp= ' ';
    }
  }

  if (numline)
    idword= "";
  else
    idword = ajStrStr(outseq->Name);

  width = AJMIN(width,maxSeqWidth);

  i=0;				/* seqpos position in seq[]*/
  l=0;				/* linepos position in output line s[] */
  
  ibase = 1;			/* base count */

  while (i < seqlen) {

    if (l1 < 0)
      l1 = 0;
    else if (l1 == 0) {		/* start of a new line */
      if (skipbefore) {
	(void) fprintf(outf, "\n");	/* blank line before writing */
	linesout++;
      }
      if (nameleft)
	(void) fprintf(outf, nameform, idword);
      if (numleft) {
	if (numline)
	  (void) fprintf(outf, numform, "");
	else
	  (void) fprintf(outf, numform, ibase);
      }
      for (j=0; j<tab; j++)
	(void) fputc(' ',outf);
    }

    l1++;                 /* don't count spaces for width*/
    if (numline) {
      if (spacer==seqSpaceAll || (spacer != 0 && (l+1) % spacer == 1)) {
        if (numline==1) (void) fputc(' ',outf);
        s[l++] = ' ';
      }
      if (l1 % 10 == 1 || l1 == width) {
        if (numline==1) (void) fprintf(outf,"%-9d ",i+1);
        s[l++]= '|'; /* == put a number here */
        }
      else s[l++]= ' ';
      i++;
    }

    else {
      if (spacer==seqSpaceAll || (spacer != 0 && (l+1) % spacer == 1))
        s[l++] = ' ';
      if (!baseonlynum)
	ibase++;
      else if (0==strchr(nocountsymbols,seq[i]))
	ibase++;
      s[l++] = seq[i++];
    }

    if (l1 == width || i == seqlen) {
      if (pretty || numjust) {
	for ( ; l1<width; l1++) {
	  if (spacer==seqSpaceAll || (spacer != 0 && (l+1) % spacer == 1))
	    s[l++] = ' ';
	  s[l++]=' '; /* pad with blanks */
        }
      }
      s[l] = '\0';
      l = 0; l1 = 0;

      if (numline) {
        if (numline==2)
	  (void) fprintf(outf,"%s",s); /* finish numberline ! and | */
      }
      else {
	(void) fprintf(outf,"%s",s);
        if (numright || nameright)
	  (void) fputc(' ',outf);
        if (numright)
	  (void) fprintf(outf,numform, ibase-1);
        if (nameright)
	  (void) fprintf(outf, nameform,idword);
        if (i == seqlen)
	  (void) fprintf(outf,"%s",endstr);
      }
      (void) fputc('\n',outf);
      linesout++;
      if (skipafter) {
	(void) fprintf(outf, "\n");
	linesout++;
      }
    }
  }

  return;
}

/* ==================================================================== */
/* ============================ Casts ================================= */
/* ==================================================================== */

/* @section Sequence Output Casts ********************************************
**
** These functions examine the contents of a sequence output object
** and return some derived information. Some of them provide access to
** the internal components of a sequence output object. They are
** provided for programming convenience but should be used with
** caution.
**
******************************************************************************/

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Sequence Output Assignments ***************************************
**
** These functions overwrite the sequence output object provided as
** the first argument.
**
******************************************************************************/

/* @funcstatic seqClone *******************************************************
**
** Copies data from a sequence into a sequence output object.
** Used before writing the sequence.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

static void seqClone (AjPSeqout outseq, AjPSeq seq) {

  int ibegin = 1;
  int iend = ajStrLen(seq->Seq);

  if (seq->Begin) {
    ibegin = ajSeqBegin(seq);
    ajDebug ("seqClone begin: %d\n", ibegin);
  }

  if (seq->End) {
    iend = ajSeqEnd(seq);
    ajDebug ("seqClone begin: %d\n", ibegin);
  }

  (void) ajStrSet (&outseq->Name, seq->Name);
  (void) ajStrSet (&outseq->Acc, seq->Acc);
  (void) ajStrSet (&outseq->Desc, seq->Desc);
  (void) ajStrSet (&outseq->Type, seq->Type);
  (void) ajStrSet (&outseq->Informatstr, seq->Formatstr);
  (void) ajStrSet (&outseq->Entryname, seq->Entryname);
  (void) ajStrSet (&outseq->Db, seq->Db);

  outseq->Fttable = seq->Fttable;
  outseq->Offset = ibegin;
  (void) ajStrAssSub (&outseq->Seq, seq->Seq, ibegin-1, iend-1);

  ajDebug ("seqClone %d .. %d %d .. %d len; %d\n",
	   seq->Begin, seq->End, ibegin, iend, ajStrLen(outseq->Seq));
  ajDebug ("  Db: '%S' Name: '%S' Entryname: '%S'\n",
	   outseq->Db, outseq->Name, outseq->Entryname);

  return;
}

/* @funcstatic seqAllClone ****************************************************
**
** Copies data from a sequence into a sequence output object.
** Used before writing the sequence. This version works with sequence streams.
** The difference is that the output object must be overwritten.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seq [AjPSeq] Sequence.
** @return [void]
** @@
******************************************************************************/

static void seqAllClone (AjPSeqout outseq, AjPSeq seq) {

  int ibegin = 1;
  int iend = ajStrLen(seq->Seq);

  if (seq->Begin) {
    ibegin = ajSeqBegin(seq);
    ajDebug ("seqAllClone begin: %d\n", ibegin);
  }

  if (seq->End) {
    iend = ajSeqEnd(seq);
    ajDebug ("seqAllClone end: %d\n", iend);
  }

  (void) ajStrAssS (&outseq->Db, seq->Db);
  (void) ajStrAssS (&outseq->Name, seq->Name);
  (void) ajStrAssS (&outseq->Acc, seq->Acc);
  (void) ajStrAssS (&outseq->Desc, seq->Desc);
  (void) ajStrAssS (&outseq->Type, seq->Type);
  (void) ajStrAssS (&outseq->Informatstr, seq->Formatstr);
  (void) ajStrAssS (&outseq->Entryname, seq->Entryname);

  (void) ajStrAssSub (&outseq->Seq, seq->Seq, ibegin-1, iend-1);

  return;
}

/* @funcstatic seqsetClone ****************************************************
**
** Clones one sequence from a set ready for output.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @param [P] seqset [AjPSeqset] Sequence set.
** @param [r] i [int] Sequence number, zero for the first sequence.
** @return [void]
** @@
******************************************************************************/

static void seqsetClone (AjPSeqout outseq, AjPSeqset seqset, int i) {

  /* intended to clone ith sequence in the set */

  AjPSeq seq = seqset->Seq[i];

  seqAllClone (outseq, seq);

  return;
}

/* @funcstatic seqDeclone *****************************************************
**
** Clears clones data in a sequence output object.
**
** @param [P] outseq [AjPSeqout] Sequence output.
** @return [void]
** @@
******************************************************************************/

static void seqDeclone (AjPSeqout outseq) {

  (void) ajStrClear (&outseq->Db);
  (void) ajStrClear (&outseq->Name);
  (void) ajStrClear (&outseq->Acc);
  (void) ajStrClear (&outseq->Desc);
  (void) ajStrClear (&outseq->Type);
  (void) ajStrClear (&outseq->Informatstr);
  (void) ajStrClear (&outseq->Entryname);

  (void) ajStrClear (&outseq->Seq);

  return;
}

/* @funcstatic seqFileReopen **************************************************
**
** Reopen a sequence output file. Used after the file name has been changed
** when writing a set of sequences one to each file.
**
** @param [P] outseq [AjPSeqout] Sequence output object.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool seqFileReopen (AjPSeqout outseq) {

  AjPStr name = NULL;

  if (outseq->File)
    ajFileClose (&outseq->File);

  (void) ajFmtPrintS(&name, "%S.%S", outseq->Name, outseq->Extension);
  (void) ajStrToLower (&name);
  outseq->File = ajFileNewOut(name);
  ajDebug("seqFileReopen single: %B file '%S'\n", outseq->Single, name);
  ajStrDel(&name);

  if (!outseq->File)
    return ajFalse;

  return ajTrue;
}

/* @func ajSeqoutUsa **********************************************************
**
** Creates or resets a sequence output object using a new Universal
** Sequence Address
**
** @param [uP] pthis [AjPSeqout*] Sequence output object.
** @param [P] Usa [AjPStr] USA
** @return [void]
** @@
******************************************************************************/

void ajSeqoutUsa (AjPSeqout* pthis, AjPStr Usa) {

  AjPSeqout thys;

  if (!*pthis) {
    thys = *pthis = ajSeqoutNew();
  }
  else {
    thys = *pthis;
    ajSeqoutClear(thys);
  }

  (void) ajStrAss (&thys->Usa, Usa);

  return;
}

/* @func ajSeqoutClear ********************************************************
**
** Clears a Sequence output object back to "as new" condition
**
** @param [P] thys [AjPSeqout] Sequence output object
** @return [void]
** @@
******************************************************************************/

void ajSeqoutClear (AjPSeqout thys) {

  ajDebug ("ajSeqInClear called\n");

  (void) ajStrClear(&thys->Name);
  (void) ajStrClear(&thys->Acc);
  (void) ajStrClear(&thys->Desc);
  (void) ajStrClear(&thys->Type);
  (void) ajStrClear(&thys->Full);
  (void) ajStrClear(&thys->Date);
  (void) ajStrClear(&thys->Doc);
  (void) ajStrClear(&thys->Usa);
  (void) ajStrClear(&thys->Ufo);
  (void) ajStrClear(&thys->Informatstr);
  (void) ajStrClear(&thys->Formatstr);
  (void) ajStrClear(&thys->Filename);
  (void) ajStrClear(&thys->Entryname);
  (void) ajStrClear(&thys->Extension);
  (void) ajStrClear(&thys->Seq);
  thys->EType = 0;
  thys->Rev = ajFalse;
  thys->Format = 0;
  if (thys->File)
    ajFileClose(&thys->File);
  thys->Count = 0;
  thys->Single = ajFalse;
  thys->Features = ajFalse;

  return;
}

/* @funcstatic seqDbName *****************************************************
**
** Adds the database name (if any) to the name provided.
**
** @param [w] name [AjPStr*] Derived name.
** @param [r] db [AjPStr] Database name (if any)
** @return [void]
** @@
******************************************************************************/

static void seqDbName (AjPStr* name, AjPStr db) {

  static AjPStr tmpname = 0;

  if (!ajStrLen(db))
    return;

  (void) ajStrAssS (&tmpname, *name);
  (void) ajFmtPrintS (name, "%S:%S", db, tmpname);

  return;
}

/* @funcstatic seqDefName *****************************************************
**
** Provides a unique (for this program run) name for a sequence.
**
** @param [w] name [AjPStr*] Derived name.
** @param [r] multi [AjBool] If true, appends a number to the name.
** @return [void]
** @@
******************************************************************************/

static void seqDefName (AjPStr* name, AjBool multi) {

  static int count=0;

  if (ajStrLen(*name)) {
    ajDebug("seqDefName already have '%S'\n", *name);
    return;
  }

  if (multi)
    (void) ajFmtPrintS(name, "EMBOSS_%3.3d", ++count);
  else
    (void) ajStrAssC (name, "EMBOSS");

  ajDebug("seqDefName set to  '%S'\n", *name);

  return;
}

/* @func ajSeqoutTrace ********************************************************
**
** Debug calls to trace the data in a sequence object.
**
** @param [r] seq [AjPSeqout] Sequence output object.
** @return [void]
** @@
******************************************************************************/

void ajSeqoutTrace (AjPSeqout seq) {

  ajDebug ("\n\n\nSequence Out trace\n");
  ajDebug ( "==============\n\n");
  ajDebug ( "  Name: '%S'\n", seq->Name);
  if (ajStrLen(seq->Acc))
    ajDebug ( "  Accession: '%S'\n", seq->Acc);
  if (ajStrLen(seq->Desc))
    ajDebug ( "  Description: '%S'\n", seq->Desc);
  if (ajStrSize(seq->Seq))
    ajDebug ( "  Reserved: %d\n", ajStrSize(seq->Seq));
  if (ajStrLen(seq->Type))
    ajDebug ( "  Type: '%S'\n", seq->Type);
  if (ajStrLen(seq->Db))
    ajDebug ( "  Database: '%S'\n", seq->Db);
  if (ajStrLen(seq->Full))
    ajDebug ( "  Full name: '%S'\n", seq->Full);
  if (ajStrLen(seq->Date))
    ajDebug ( "  Date: '%S'\n", seq->Date);
  if (ajStrLen(seq->Usa))
    ajDebug ( "  Usa: '%S'\n", seq->Usa);
  if (ajStrLen(seq->Ufo))
    ajDebug ( "  Ufo: '%S'\n", seq->Ufo);
  if (ajStrLen(seq->Formatstr))
    ajDebug ( "  Output format: '%S'\n", seq->Formatstr);
  if (ajStrLen(seq->Filename))
    ajDebug ( "  Filename: '%S'\n", seq->Filename);
  if (ajStrLen(seq->Entryname))
    ajDebug ( "  Entryname: '%S'\n", seq->Entryname);
  if (ajStrLen(seq->Doc))
    ajDebug ( "  Documentation:...\n%S\n", seq->Doc);
  if(seq->Fttable)
    ajFeatTrace(seq->Fttable);
  else
    ajDebug( "  No Feature table present\n");
  if(seq->Features)
    ajDebug( "  Features ON\n");
  else
    ajDebug( "  Features OFF\n");
    
}

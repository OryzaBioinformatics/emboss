#include "emboss.h"

typedef struct primers {
  AjPStr Name;
  AjPRegexp Prima;
  AjPRegexp Primb;
  AjPStr Oligoa;
  AjPStr Oligob;
} *Primer;

AjPFile out = NULL;
AjPSeq seq = NULL;
AjPStr seqstr = NULL;
AjPStr revstr = NULL;
int nprimers = 0;
int ntests = 0;

static void primTest (void **x,void *cl);

int main (int argc, char * argv[]) {

  AjPSeqall seqall;
  AjPFile primfile;
  AjPStr rdline = NULL;

  Primer primdata;
  AjPStrTok handle = NULL;

  AjPList primList=NULL;

  embInit ("stssearch", argc, argv);

  primfile = ajAcdGetInfile ("primers");
  out = ajAcdGetOutfile ("out");
  seqall = ajAcdGetSeqall ("sequences");

  while (ajFileReadLine (primfile, &rdline)) {
    if (ajStrChar(rdline, 0) == '#')
      continue;
    if (ajStrSuffixC(rdline, ".."))
      continue;

    AJNEW(primdata);
    primdata->Name = NULL;
    primdata->Oligoa = NULL;
    primdata->Oligob = NULL;

    handle = ajStrTokenInit (rdline, " \t");
    ajStrToken (&primdata->Name, &handle, NULL);
    if (!(nprimers % 1000))
	  ajDebug ("Name [%d]: '%S'\n", nprimers, primdata->Name);

    ajStrToken (&primdata->Oligoa, &handle, NULL);
    ajStrToUpper(&primdata->Oligoa);
    primdata->Prima = ajRegComp(primdata->Oligoa);

    ajStrToken (&primdata->Oligob, &handle, NULL);
    ajStrToUpper(&primdata->Oligob);
    primdata->Primb = ajRegComp(primdata->Oligob);
    ajStrTokenClear (&handle);

    if (!nprimers)
      primList = ajListNew();

    ajListPushApp (primList, primdata);
    nprimers++;
  }

  if (!nprimers)
    ajFatal ("No primers read\n");

  ajDebug ("%d primers read\n", nprimers);

  while (ajSeqallNext(seqall, &seq)) {
    ajSeqToUpper(seq);
    ajStrAss(&seqstr, ajSeqStr(seq));
    ajStrAss(&revstr, ajSeqStr(seq));
    ajSeqReverseStr(&revstr);
    ajDebug ("Testing: %s\n", ajSeqName(seq));
    ntests = 0;
    ajListMap (primList, primTest, NULL);
  }

  ajFileClose (&out);

  ajExit ();
  return 0;
}

static void primTest (void **x,void *cl) {
  Primer* p;
  Primer primdata;

  static int calls = 0;

  AjBool testa, testb;
  AjBool testc, testd;
  int ioff;

  p = (Primer*) x;
  primdata = *p;

  ntests++;

  if (!(ntests % 1000))
    ajDebug("completed tests: %d\n", ntests);

  if (!calls) {
  }
  calls = 1;

  testa = ajRegExec (primdata->Prima, seqstr);
  if (testa) {
    ioff = ajRegOffset (primdata->Prima);
    ajDebug ("%s: %S PrimerA matched at %d\n",
	     ajSeqName(seq), primdata->Name, ioff);
    ajFmtPrintF (out, "%s: %S PrimerA matched at %d\n",
		 ajSeqName(seq), primdata->Name, ioff);
    ajRegTrace (primdata->Prima);
  }
  testb = ajRegExec (primdata->Primb, seqstr);
  if (testb) {
    ioff = ajRegOffset (primdata->Primb);
      ajDebug ("%s: %S PrimerB matched at %d\n",
	       ajSeqName(seq), primdata->Name, ioff);
    ajFmtPrintF (out, "%s: %S PrimerB matched at %d\n",
		 ajSeqName(seq), primdata->Name, ioff);
    ajRegTrace (primdata->Primb);
  }
  testc = ajRegExec (primdata->Prima, revstr);
  if (testc) {
    ioff = ajStrLen(seqstr) - ajRegOffset (primdata->Prima);
    ajDebug ("%s: (rev) %S PrimerA matched at %d\n",
	     ajSeqName(seq), primdata->Name, ioff);
    ajFmtPrintF (out, "%s: (rev) %S PrimerA matched at %d\n",
		 ajSeqName(seq), primdata->Name, ioff);
    ajRegTrace (primdata->Prima);
  }
  testd = ajRegExec (primdata->Primb, revstr);
  if (testd) {
    ioff = ajStrLen(seqstr) - ajRegOffset (primdata->Primb);
    ajDebug ("%s: (rev) %S PrimerB matched at %d\n",
	     ajSeqName(seq), primdata->Name, ioff);
    ajFmtPrintF (out, "%s: (rev) %S PrimerB matched at %d\n",
		 ajSeqName(seq), primdata->Name, ioff);
    ajRegTrace (primdata->Primb);
  }

  return;
}

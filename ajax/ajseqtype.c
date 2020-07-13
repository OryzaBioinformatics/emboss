#include "ajax.h"

typedef struct SeqSType {
  char *Name;
  AjBool Gaps;
  ajint Type;
  char (*Test) (AjPSeq thys);  
  char *Desc;
} SeqOType, *SeqPType;

enum ProtNuc {ISANY=0, ISNUC=1, ISPROT=2};

/* gaps only allowed if it says so
** gap conversion is a separate attribute, along with case convserion
*/

/* @funclist seqType **********************************************************
**
** Functions to test each sequence type
**
******************************************************************************/

static SeqOType seqType[] = {
  {"any",            AJFALSE, ISANY, ajSeqTypeAny,
                     "any valid sequence"}, /* reset type */
  {"dna",            AJFALSE, ISNUC, ajSeqTypeDna,      
                     "DNA sequence"},
  {"rna",            AJFALSE, ISNUC, ajSeqTypeRna,      
                     "RNA sequence"},
  {"puredna",        AJFALSE, ISNUC, ajSeqTypePuredna,  
                     "DNA, bases ACGT only"},
  {"purerna",        AJFALSE, ISNUC, ajSeqTypePurerna,  
                     "RNA, bases ACGU only"},
  {"nucleotide",     AJFALSE, ISNUC, ajSeqTypeNuc,      
                     "nucleotide sequence"},
  {"purenucleotide", AJFALSE, ISNUC, ajSeqTypePurenuc,  
                     "nucleotide, bases ACGTU only"},
  {"gapnucleotide",  AJFALSE, ISNUC, ajSeqTypeGapnuc,   
                     "nucleotide, bases ACGTU with gaps"},
  {"gapdna",         AJTRUE,  ISNUC, ajSeqTypeGapdna,   
                     "DNA sequence with gaps"},
  {"gaprna",         AJTRUE,  ISNUC, ajSeqTypeGaprna,   
                     "RNA sequence with gaps"},
  {"protein",        AJFALSE, ISPROT,  ajSeqTypeProt,     
                     "protein sequence"},
  {"gapprotein",     AJTRUE,  ISPROT,  ajSeqTypeGapprot,  
                     "protein sequence with gaps"},
  {"pureprotein",    AJFALSE, ISPROT,  ajSeqTypePureprot, 
                     "protein sequence without BZ or X"},
  {"stopprotein",    AJFALSE, ISPROT,  ajSeqTypeStopprot, 
                     "protein sequence with a possible stop"},
  {"gapany",         AJTRUE,  ISANY, ajSeqTypeGapany,   
                     "any valid sequence with gaps"}, /* reset type */
  {NULL,             AJFALSE, ISANY, NULL,            
                     NULL}
};

static void       seqGapSL (AjPStr* seq, char gapc, char padc, ajint ilen);
static AjBool     seqTypeStopTrim (AjPSeq thys);
static void       seqTypeSet (AjPSeq thys, AjPStr Type);

/* gap characters known are:
**
** . GCG and most others
** - Phylip and some alignment output
** ~ GCG for gaps at ends
** * Staden for DNA but stop for protein (fix on input?)
** O Phylip (fix on input?)
*/
char seqCharProt[] = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwyBXZbxz*";
char seqCharProtPure[] = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwy";
char seqCharProtAmbig[] = "BXZbxz";
char seqCharProtStop[] = "*";
char seqCharNuc[] = "ACGTUacgtuBDHKMNRSVWXYbdhkmnrsvwxy";
char seqCharNucPure[] = "ACGTUacgtu";
char seqCharNucAmbig[] = "BDHKMNRSVWXYbdhkmnrsvwxy";
char seqCharGap[] = ".-~Oo";	/* phylip uses O */
char seqCharNucDNA[] = "Tt";
char seqCharNucRNA[] = "Uu";
char seqCharGapany[] = ".-~Oo";	/* phylip uses O */
char seqCharGapdash[] = "-";
char seqCharGapdot[] = "..";
char seqGap = '-';		/* the (only) EMBOSS gap character */

/* @func ajSeqTypeTest *******************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used only for testing, so never writes any error message
**
** @param [P] thys [AjPSeq] Sequence object
** @param [P] Type [AjPStr] Sequence type
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeTest (AjPSeq thys, AjPStr Type) {

  ajint i = 0;
  char ret;

  ajDebug ("testing sequence '%s' type '%S'\n", ajSeqName(thys), Type);

  if (!ajStrLen(Type)) {		/* nothing given - anything goes */
    ajSeqGap (thys, seqGap, 0);
    return ajTrue;
  }

  for (i = 0; seqType[i].Name; i++) {
    if (!ajStrMatchCaseC(Type, seqType[i].Name)) continue;
    ajDebug ("type '%s' found (%s)\n", seqType[i].Name, seqType[i].Desc);
    if (seqType[i].Type == ISPROT && !ajSeqIsProt(thys)) {
      ajDebug("Sequence is not a protein\n");
      return ajFalse;
    }
    if (seqType[i].Type == ISNUC && !ajSeqIsNuc(thys)) {
      ajDebug("Sequence is not nucleic\n");
      return ajFalse;
    }

    /* Calling funclist seqType() */
    ret = seqType[i].Test (thys);
    if (ret) {
      ajDebug ("Sequence %s must be %s,\n found bad character '%c'",
	     ajSeqName(thys), seqType[i].Desc, ret);
      return ajFalse;
    }
    else {
      if (seqType[i].Gaps)
	ajSeqGap (thys, seqGap, 0);
      return ajTrue;
    }
  }

  ajErr ("Sequence type '%S' unknown", Type);
  return ajFalse;
}

/* @funcstatic seqTypeSet *****************************************************
**
** Sets the sequence type. Uses the first character of the type
** which can be N or P
**
** @param [P] thys [AjPSeq] Sequence object
** @param [P] Type [AjPStr] Sequence type
** @return [void]
** @@
******************************************************************************/

static void seqTypeSet (AjPSeq thys, AjPStr Type) {

  char* cp = ajStrStr(Type);

  ajDebug ("seqTypeSet '%S'\n", Type);

  switch (*cp) {
  case 'P':
  case 'p':
    ajSeqSetProt(thys);
    break;
  case 'N':
  case 'n':
    ajSeqSetNuc(thys);
    break;
  case '\0':
    break;
  default:
    ajErr("Unknown sequence type '%c'", *cp);
  }

  return;
}

/* @func ajSeqTypeCheck *******************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used for input validation - writes error message if the type check fails
**
** @param [P] thys [AjPSeq] Sequence object
** @param [P] seqin [AjPSeqin] Sequence input object
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeCheck (AjPSeq thys, AjPSeqin seqin) {

  ajint i = 0;
  char ret;

  AjPStr Type = seqin->Inputtype; /* ACD file had a predefined seq type */

  ajDebug ("testing sequence '%s' type '%S' IsNuc %B IsProt %B\n",
	   ajSeqName(thys), seqin->Inputtype, seqin->IsNuc, seqin->IsProt);

  if (seqin->IsNuc)
    ajSeqSetNuc(thys);

  if (seqin->IsProt)
    ajSeqSetProt(thys);

  if (seqin->Query && ajStrLen(seqin->Query->DbType)) {
    seqTypeSet(thys, seqin->Query->DbType);
  }

  if (!ajStrLen(Type)) {		/* nothing given - anything goes */
    ajSeqGap (thys, seqGap, 0);
    return ajTrue;
  }

  for (i = 0; seqType[i].Name; i++) {
    if (!ajStrMatchCaseC(Type, seqType[i].Name)) continue;
    ajDebug ("type '%s' found (%s)\n", seqType[i].Name, seqType[i].Desc);
    if (seqType[i].Type == ISPROT && !ajSeqIsProt(thys)) {
      ajErr("Sequence is not a protein\n");
      return ajFalse;
    }
    if (seqType[i].Type == ISNUC && !ajSeqIsNuc(thys)) {
      ajErr("Sequence is not nucleic\n");
      return ajFalse;
    }
    /* Calling funclist seqType() */
    ret = seqType[i].Test (thys);
    if (ret) {
      ajErr ("Sequence %s must be %s,\n found bad character '%c'",
	     ajSeqName(thys), seqType[i].Desc, ret);
      return ajFalse;
    }
    else {
      if (seqType[i].Gaps)
	ajSeqGap (thys, seqGap, 0);
      return ajTrue;
    }
  }

  ajErr ("Sequence type '%S' unknown", Type);
  return ajFalse;
}

/* @func ajSeqTypeAny ***********************************************
**
** Checks sequence type for any valid sequence character (but no gaps)
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] bad character if found, or null.
** @@
******************************************************************************/

char ajSeqTypeAny (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy (seqchars, seqCharProtPure);
    (void) strcat (seqchars, seqCharProtAmbig);
    (void) strcat (seqchars, seqCharProtStop);
    (void) strcat (seqchars, seqCharNucPure);
    (void) strcat (seqchars, seqCharNucAmbig);
  }
  if (!ajSeqLen(thys)) return ajTrue;

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  return 0;
}

/* @func ajSeqTypeDna ***********************************************
**
** Checks sequence type for DNA.
** 
** RNA codes are accepted but are converted to DNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeDna (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
  }

  ajDebug ("seqTypeDna test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  /* ajDebug ("convert '%S'\n", thys->Seq); */
  (void) ajStrConvertCC (&thys->Seq, seqCharNucRNA, seqCharNucDNA);
  /* ajDebug (" result '%S'\n", thys->Seq); */

  return '\0';
}

/* @func ajSeqTypeRna ***********************************************
**
** Checks sequence type for RNA.
** 
** DNA codes are accepted but are converted to RNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeRna (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
  }

  ajDebug ("seqTypeRna test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  /* ajDebug ("convert '%S'\n", thys->Seq); */
  (void) ajStrConvertCC (&thys->Seq, seqCharNucDNA, seqCharNucRNA);
  /* ajDebug (" result '%S'\n", thys->Seq); */

  return '\0';
}

/* @func ajSeqTypePuredna ***********************************************
**
** Checks sequence type for pure (unambiguous) DNA.
** 
** RNA codes are accepted but are converted to DNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePuredna (AjPSeq thys) {
  ajint i;
  static char* seqchars = seqCharNucPure;

  ajDebug ("seqTypePureDna test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  /* ajDebug ("convert '%S'\n", thys->Seq); */
  (void) ajStrConvertCC (&thys->Seq, seqCharNucRNA, seqCharNucDNA);
  /* ajDebug (" result '%S'\n", thys->Seq); */

  return '\0';
}

/* @func ajSeqTypePurerna ***********************************************
**
** Checks sequence type for pure (unambiguous) RNA.
** 
** DNA codes are accepted but are converted to RNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePurerna (AjPSeq thys) {
  ajint i;
  static char* seqchars = seqCharNucPure;

  ajDebug ("seqTypePureRna test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);
  /* ajDebug ("convert '%S'\n", thys->Seq); */
  (void) ajStrConvertCC (&thys->Seq, seqCharNucDNA, seqCharNucRNA);
  /* ajDebug (" result '%S'\n", thys->Seq); */

  return '\0';
}

/* @func ajSeqTypeNuc ***********************************************
**
** Checks sequence type for nucleotide.
** 
** RNA and DNA codes are accepted as is.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeNuc (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
  }

  ajDebug ("seqTypeNuc test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  return '\0';
}

/* @func ajSeqTypePurenuc ***********************************************
**
** Checks sequence type for pure (unambiguous) nucleotide.
** 
** RNA and DNA codes are accepted as is.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePurenuc (AjPSeq thys) {
  ajint i;
  static char* seqchars = seqCharNucPure;

  ajDebug ("seqTypePureNuc test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  return '\0';
}

/* @func ajSeqTypeGapnuc ***********************************************
**
** Checks sequence type for nucleotide with gaps.
** 
** RNA and DNA codes are accepted as is.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapnuc (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
    (void) strcat(seqchars, seqCharGap);
  }

  ajDebug ("seqTypeGapnuc test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  return '\0';
}


/* @func ajSeqTypeGapdna ***********************************************
**
** Checks sequence type for DNA with gaps.
** 
** RNA codes are accepted an converted to DNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapdna (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
    (void) strcat(seqchars, seqCharGap);
  }

  ajDebug ("seqTypeGapdna test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);


  /* ajDebug ("convert '%S'\n", thys->Seq); */
  (void) ajStrConvertCC (&thys->Seq, seqCharNucRNA, seqCharNucDNA);
  /* ajDebug (" result '%S'\n", thys->Seq); */

  return '\0';
}

/* @func ajSeqTypeGaprna ***********************************************
**
** Checks sequence type for RNA with gaps.
** 
** DNA codes are accepted an converted to RNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGaprna (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
    (void) strcat(seqchars, seqCharGap);
  }

  ajDebug ("seqTypeGaprna test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  /* ajDebug ("convert '%S'\n", thys->Seq); */
  (void) ajStrConvertCC (&thys->Seq, seqCharNucDNA, seqCharNucRNA);
  /* ajDebug (" result '%S'\n", thys->Seq); */

  return '\0';
}

/* @func ajSeqTypeProt ***********************************************
**
** Checks sequence type for protein.
** 
** A stop at the end is allowed (but is removed).
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeProt (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharProtPure);
    (void) strcat(seqchars, seqCharProtAmbig);
  }

  ajDebug ("seqTypeProt test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';
  (void) seqTypeStopTrim(thys);

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys)) {
    return ajStrChar(thys->Seq, i);
  }

  return '\0';
}

/* @func ajSeqTypePureprot ***********************************************
**
** Checks sequence type for (unambiguous) protein.
**
** A stop at the end is allowed (but is removed).
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePureprot (AjPSeq thys) {
  ajint i;
  static char* seqchars = seqCharProtPure;

  ajDebug ("seqTypePureprot test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  (void) seqTypeStopTrim(thys);
  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys)) {
    return ajStrChar(thys->Seq, i);
  }

  return '\0';
}

/* @func ajSeqTypeAnyprot ***********************************************
**
** Checks sequence type for anything that can be in a protein sequence
** 
** Stop codes are replaced with gaps.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeAnyprot (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharProtPure);
    (void) strcat(seqchars, seqCharProtAmbig);
    (void) strcat(seqchars, seqCharGap);
    (void) strcat(seqchars, seqCharProtStop);
  }

  ajDebug ("seqTypeAnyprot test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  return '\0';
}

/* @func ajSeqTypeGapprot ***********************************************
**
** Checks sequence type for protein with gaps.
** 
** Stop codes are replaced with gaps.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapprot (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharProtPure);
    (void) strcat(seqchars, seqCharProtAmbig);
    (void) strcat(seqchars, seqCharGap);
  }

  ajDebug ("seqTypeGapprot test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';
  (void) ajStrConvertCC (&thys->Seq, seqCharProtStop, "-");

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys))
    return ajStrChar(thys->Seq, i);

  return '\0';
}

/* @func ajSeqTypeStopprot ***********************************************
**
** Checks sequence type for protein.
** 
** Stops ('*') are allowed so this could be a 3 frame translation of DNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeStopprot (AjPSeq thys) {
  ajint i;
  static char seqchars[256] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharProtPure);
    (void) strcat(seqchars, seqCharProtAmbig);
    (void) strcat(seqchars, seqCharProtStop);
  }

  ajDebug ("seqTypeStopprot test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys)) {
    ajDebug ("bad character %d '%c'\n", i, ajStrChar(thys->Seq, i));
    ajSeqTrace(thys);
    ajStrTrace(thys->Seq);
    return ajStrChar(thys->Seq, i);
  }

  return '\0';
}

/* @func ajSeqTypeGapany ***********************************************
**
** Checks sequence type for any sequence with gaps.
** 
** Stops ('*') are allowed so this could be a 3 frame translation of DNA.
**
** @param [r] thys [AjPSeq] Sequence
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapany (AjPSeq thys) {
  ajint i;
  static char seqchars[512] = "";

  if (!*seqchars) {
    (void) strcpy(seqchars, seqCharProtPure);
    (void) strcat(seqchars, seqCharProtAmbig);
    (void) strcat(seqchars, seqCharProtStop);
    (void) strcat(seqchars, seqCharNucPure);
    (void) strcat(seqchars, seqCharNucAmbig);
    (void) strcat(seqchars, seqCharGap);
  }

  ajDebug ("seqTypeGapany test for '%s'\n", seqchars);

  if (!ajSeqLen(thys)) return '\0';

  i = strspn(ajStrStr(thys->Seq), seqchars);
  if (i < ajSeqLen(thys)) {
    return ajStrChar(thys->Seq, i);
  }

  return '\0';
}

/* @func ajSeqGap ********************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] thys [AjPSeq] Sequence
** @param [r] gapc [char] Standard gap character
** @param [r] padc [char] Gap character for ends of sequence
** @return [void]
** @@
******************************************************************************/

void ajSeqGap (AjPSeq thys, char gapc, char padc) {
  seqGapSL (&thys->Seq, gapc, padc, 0);
}

/* @func ajSeqGapLen ********************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] thys [AjPSeq] Sequence
** @param [r] gapc [char] Standard gap character
** @param [r] padc [char] Gap character for ends of sequence
** @param [r] ilen [ajint] Sequence length. Expanded if longer than
**                       current length
** @return [void]
** @@
******************************************************************************/

void ajSeqGapLen (AjPSeq thys, char gapc, char padc, ajint ilen) {
  seqGapSL (&thys->Seq, gapc, padc, ilen);
}

/* @func ajSeqGapS ********************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] seq [AjPStr*] Sequence
** @param [r] gapc [char] Standard gap character
** @return [void]
** @@
******************************************************************************/

void ajSeqGapS (AjPStr* seq, char gapc) {
  seqGapSL (seq, gapc, 0, 0);
}

/* @funcstatic seqGapSL *******************************************************
**
** Sets non-sequence characters in a string to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] seq [AjPStr*] String of sequence characters
** @param [r] gapc [char] Standard gap character
** @param [r] padc [char] Gap character for ends of sequence
** @param [r] ilen [ajint] Sequence length. Expanded if longer than
**                       current length
** @return [void]
** @@
******************************************************************************/

static void seqGapSL (AjPStr* seq, char gapc, char padc, ajint ilen) {

  ajint i;
  static char* newgap;
  static ajint igap;
  char* cp;
  char endc = gapc;

  igap = strlen(seqCharGap);
  if (!newgap){
    newgap = ajCharNewL(igap);
    newgap[0] = '\0';
  }
  
  if (*newgap != gapc) {
    for (i=0; i < igap; i++)
      newgap[i] = gapc;
    newgap[i] = '\0';
  }

  if (ilen)
    (void) ajStrModL (seq, ilen+1);
  else
    (void) ajStrMod(seq);

  (void) ajStrConvertCC (seq, seqCharGap, newgap);

  if (padc) {			/* start and end characters updated */
    endc = padc;
    for (cp = ajStrStr(*seq);
	 strchr(seqCharGap, *cp); cp++) /* pad start */
      *cp = padc;
    cp = ajStrStr(*seq);
    for (i=ajStrLen(*seq) - 1; i && strchr(seqCharGap, cp[i]);  i--)
      cp[i] = padc;
  }

  if (ajStrLen(*seq) < ilen) {	/* ilen can be zero to skip this */
    cp = ajStrStr(*seq);
    for (i=ajStrLen(*seq); i < ilen; i++)
      cp[i] = endc;
    cp[ilen] = '\0';
    ajStrFix(*seq);
  }

  return;
}

/* @funcstatic seqTypeStopTrim ************************************************
**
** Removes a trailing stop (asterisk) from a protein sequence
**
** @param [P] thys [AjPSeq] Sequence object
** @return [AjBool] ajTrue if a stop was removed.
** @@
******************************************************************************/

static AjBool seqTypeStopTrim (AjPSeq thys) {

  if (strchr(seqCharProtStop,ajStrChar(thys->Seq, -1))) {
    ajDebug("Trailing stop removed %c\n", ajStrChar(thys->Seq, -1));
    (void) ajStrTrim(&thys->Seq, -1);
    return ajTrue;
  }
  return ajFalse;
}

/* @func ajSeqSetNuc **********************************************************
**
** Sets a sequence type to "nucleotide"
**
** @param [P] thys [AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajSeqSetNuc (AjPSeq thys) {
    (void) ajStrAssC (&thys->Type, "N");
}

/* @func ajSeqSetProt ******************************************************
**
** Sets a sequence type to "protein"
**
** @param [P] thys [AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajSeqSetProt (AjPSeq thys) {
    (void) ajStrAssC (&thys->Type, "P");
}

/* @func ajSeqType ************************************************************
**
** Sets the type of a sequence if it has not yet been defined.
**
** @param [P] thys [AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajSeqType (AjPSeq thys) {

  ajDebug ("ajSeqType current: %S\n", thys->Type);

  if (ajStrLen(thys->Type))
    return;

  if (ajSeqIsNuc (thys)) {
    ajSeqSetNuc(thys);
    ajDebug ("ajSeqType nucleotide: %S\n", thys->Type);
    return;
  }
  if (ajSeqIsProt (thys)) {
    ajSeqSetProt(thys);
    ajDebug ("ajSeqType protein: %S\n", thys->Type);
    return;
  }

  ajDebug ("ajSeqType unknown: %S\n", thys->Type);
  return;
}

/* @func ajSeqPrintType *******************************************************
**
** Prints the seqType definitions.
** For EMBOSS entrails output
**
** @param [R] outf [AjPFile] Output file
** @param [R] full [AjBool] Full output
** @return [void]
******************************************************************************/

void ajSeqPrintType (AjPFile outf, AjBool full) {
  ajint i;

  char* typeName[] = {"ANY", "NUC", "PRO"};

  ajFmtPrintF (outf, "\n#Sequence Types\n");
  ajFmtPrintF (outf, "# Name            Gap N/P Desciption\n");
  for (i=0; seqType[i].Name; i++) {
    ajFmtPrintF (outf, "  %-15s %3B %s \"%s\"\n",
		 seqType[i].Name, seqType[i].Gaps,
		 typeName[seqType[i].Type], seqType[i].Desc);
    
  }
}

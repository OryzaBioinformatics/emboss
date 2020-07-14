#include "ajax.h"

/* @datastatic SeqPType *******************************************************
**
** Sequence types data structure, used to test input sequence against
** a defined sequence type
**
** @alias SeqSType
** @alias SeqOType
**
** @attr Name [char*] sequence type name
** @attr Gaps [AjBool] allow gap characters
** @attr Type [ajint] enumerated ISANY=0 ISNUC=1 ISPROT=2 
** @attr Test [(char*)] Test function
** @attr Desc [char*] Description for documentation purposes
** @@
******************************************************************************/

typedef struct SeqSType
{
    char *Name;
    AjBool Gaps;
    AjBool Ambig;
    ajint Type;
    char *ConvertFrom;
    char *ConvertTo;
    AjPRegexp (*Badchars) (void);
    char *Desc;
} SeqOType, *SeqPType;

enum ProtNuc {ISANY=0, ISNUC=1, ISPROT=2};

/*
** gaps only allowed if it says so
** gap conversion is a separate attribute, along with case convserion
*/

static AjBool     seqFindType (AjPStr typename, ajint* typenum);
static void       seqGapSL (AjPStr* seq, char gapc, char padc, ajint ilen);
static AjBool     seqTypeFix (AjPSeq thys, ajint itype);
static AjBool     seqTypeFixReg (AjPSeq thys, ajint itype, char fixchar);
static void       seqTypeSet (AjPSeq thys, AjPStr Type);
static AjBool     seqTypeStopTrimS (AjPStr* pthys);
static char       seqTypeTest (AjPStr thys, AjPRegexp badchars);
static AjBool     seqTypeTestI (AjPSeq thys, ajint itype);
static AjPRegexp  seqTypeCharAny (void);
static AjPRegexp  seqTypeCharAnyGap (void);
static AjPRegexp  seqTypeCharNuc (void);
static AjPRegexp  seqTypeCharNucGap (void);
static AjPRegexp  seqTypeCharNucPure (void);
static AjPRegexp  seqTypeCharProt (void);
static AjPRegexp  seqTypeCharProtAny (void);
static AjPRegexp  seqTypeCharProtGap (void);
static AjPRegexp  seqTypeCharProtPure (void);
static AjPRegexp  seqTypeCharProtStop (void);

static AjPRegexp seqtypeRegAny = NULL;
static AjPRegexp seqtypeRegAnyGap = NULL;
static AjPRegexp seqtypeRegNuc = NULL;
static AjPRegexp seqtypeRegNucGap = NULL;
static AjPRegexp seqtypeRegNucPure = NULL;
static AjPRegexp seqtypeRegProt = NULL;
static AjPRegexp seqtypeRegProtAny = NULL;
static AjPRegexp seqtypeRegProtGap = NULL;
static AjPRegexp seqtypeRegProtPure = NULL;
static AjPRegexp seqtypeRegProtStop = NULL;

/*
** gap characters known are:
**
** . GCG and most others
** - Phylip and some alignment output
** ~ GCG for gaps at ends
** * Staden for DNA but stop for protein (fix on input?)
** O Phylip (fix on input?)
*/

char seqCharProt[] = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwyBUXZbuxz*";
char seqCharProtPure[] = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwy";
char seqCharProtAmbig[] = "BUXZbuxz";
char seqCharProtStop[] = "*";
char seqCharProtU[] = "Uu";
char seqCharProtX[] = "Xx";
char seqCharNuc[] = "ACGTUacgtuBDHKMNRSVWXYbdhkmnrsvwxy";
char seqCharNucPure[] = "ACGTUacgtu";
char seqCharNucAmbig[] = "BDHKMNRSVWXYbdhkmnrsvwxy";
char seqCharGap[] = ".~Oo-";	/* phylip uses O */
char seqCharNucDNA[] = "Tt";
char seqCharNucRNA[] = "Uu";
char seqCharGapany[] = ".~Oo-";	/* phylip uses O */
char seqCharGapdash[] = "-";
char seqCharGapdot[] = ".";
char seqGap = '-';		/* the (only) EMBOSS gap character */
char seqCharGapTest[] = " .~Oo-";   /* phylip uses O - don't forget space */

/* @funclist seqType **********************************************************
**
** Functions to test each sequence type
**
******************************************************************************/

static SeqOType seqType[] =
{
/*   "name"            Gaps     Ambig    Type    CvtFrom CvtTo
         BadcharsFunction Description */
    {"any",            AJFALSE, AJTRUE,  ISANY,  NULL,          NULL,
	 seqTypeCharAny,
	 "any valid sequence"},		/* reset type */
    {"dna",            AJFALSE, AJTRUE,  ISNUC,  "Uu", "Tt",
	 seqTypeCharAny,
	 "DNA sequence"},
    {"rna",            AJFALSE, AJTRUE,  ISNUC,  "Tt", "Uu",
	 seqTypeCharAny,
	 "RNA sequence"},
    {"puredna",        AJFALSE, AJFALSE, ISNUC,  "Uu", "Tt",
	 seqTypeCharNucPure,
	 "DNA, bases ACGT only"},
    {"purerna",        AJFALSE, AJFALSE, ISNUC,  "Tt", "Uu",
	 seqTypeCharNucPure,
	 "RNA, bases ACGU only"},
    {"nucleotide",     AJFALSE, AJTRUE,  ISNUC,  NULL,          NULL,
	 seqTypeCharNuc,
	 "nucleotide sequence"},
    {"purenucleotide", AJFALSE, AJFALSE, ISNUC,  NULL,          NULL,
	 seqTypeCharNucPure,
	 "nucleotide, bases ACGTU only"},
    {"gapnucleotide",  AJTRUE,  AJTRUE,  ISNUC,  NULL,          NULL,
	 seqTypeCharNucGap,
	 "nucleotide, bases ACGTU with gaps"},
    {"gapdna",         AJTRUE,  AJTRUE,  ISNUC,  "Uu",   "Tt",
	 seqTypeCharNucGap,
	 "DNA sequence with gaps"},
    {"gaprna",         AJTRUE,  AJTRUE,  ISNUC,  "Tt", "Uu",
	 seqTypeCharNucGap,
	 "RNA sequence with gaps"},
    {"protein",        AJFALSE, AJTRUE,  ISPROT, "*",           "X",
	 seqTypeCharProt,
	 "protein sequence"},
    {"proteinstandard",AJFALSE, AJTRUE,  ISPROT, "Uu",  "Xx",
	 seqTypeCharProt,
	 "protein sequence with no selenocysteine"},
    {"gapprotein",     AJTRUE,  AJTRUE,  ISPROT, "*",           "X",
	 seqTypeCharProtGap,
	 "protein sequence with gaps"},
    {"gapproteinstandard", AJTRUE,  AJTRUE, ISPROT, "*Uu", "XXx",
	 seqTypeCharProtGap,
	 "protein sequence with gaps but no selenocysteine"},
    {"pureprotein",    AJFALSE, AJFALSE, ISPROT, NULL,          NULL,
	 seqTypeCharProtPure,
	 "protein sequence without BZ U or X"},
    {"stopprotein",    AJFALSE, AJTRUE,  ISPROT, NULL,          NULL,
	 seqTypeCharProtStop,
	 "protein sequence with a possible stop"},
    {"stopproteinstandard",AJFALSE, AJTRUE, ISPROT, "Uu", "Xx",
	 seqTypeCharProtStop,
	 "protein sequence with a possible stop but no selenocysteine"},
    {"gapany",         AJTRUE,  AJTRUE,  ISANY,  NULL,          NULL,
	 seqTypeCharAnyGap,
	 "any valid sequence with gaps"}, /* reset type */
    {NULL,             AJFALSE, AJTRUE,  ISANY,  NULL,          NULL,
	 NULL,
	 NULL}
};

/* @func ajSeqTypeTest ********************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used only for testing, so never writes any error message
**
** @param [P] pthys [AjPStr*] Sequence string
** @param [P] Type [AjPStr] Sequence type
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeTest (AjPSeq thys, AjPStr Type)
{
    ajint itype = -1;

    ajDebug ("testing sequence '%s' type '%S'\n", thys, Type);

    if (!ajStrLen(Type)) 	   /* nothing given - anything goes */
    {
	ajSeqGap (thys, seqGap, 0);
	return ajTrue;
    }

    if (!seqFindType (Type, &itype))
    {
	ajErr ("Sequence type '%S' unknown", Type);
	return ajFalse;
    }

    ajDebug ("ajSeqTypeTest type '%s' found (%s)\n",
	     seqType[itype].Name, seqType[itype].Desc);

    if (seqTypeTestI (thys, itype))
	return ajTrue;

    return seqTypeFix(thys, itype);
}

/* @funcstatic ajSeqTypeTestI *************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used only for testing, so never writes any error message
**
** @param [P] pthys [AjPStr*] Sequence string
** @param [R] itype [ajint] Sequence type index
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

static AjBool seqTypeTestI (AjPSeq thys, ajint itype)
{
    AjPStr tmpstr = NULL;
    AjPRegexp badchars;

    /*
     ** We have a known type, not we need to either show the sequence
     ** matches it, or fix it so it does (or, of course, give up)
     */

    /*
     ** First we test the type - predefined by a database,
     ** or by checking the sequence characters
     */

    if (seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGap (thys, seqGap, 0);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrDegap (&thys->Seq);
    }

    if (seqType[itype].Type == ISPROT && !ajSeqIsProt(thys))
    {
	ajDebug("Sequence is not a protein\n");
	return ajFalse;
    }
    if (seqType[itype].Type == ISNUC && !ajSeqIsNuc(thys))
    {
	ajDebug("Sequence is not nucleic\n");
	return ajFalse;
    }

    /* Calling funclist seqType() */
    badchars = seqType[itype].Badchars();
    if (!ajRegExec(badchars, thys->Seq))
    {
	if (seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    (void) ajStrConvertCC (&thys->Seq,
				   seqType[itype].ConvertFrom,
				   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

    ajRegSubI (badchars, 1, &tmpstr);
    ajDebug ("Sequence must be %s,\n found bad character '%c'",
	     seqType[itype].Desc, ajStrChar(tmpstr, 0));
    ajStrDel(&tmpstr);

    return ajFalse;
}

/* @funcstatic seqTypeFix *****************************************************
**
** Fixes (if possible) unacceptable sequence characters by removing gaps
** (if no gaps are allowed) and by setting ambiguity codes (if they
** are allowed).
**
** @param [P] thys [AjPSeq] Sequence object
** @param [R] itype [ajint] Sequence type index
** @return [AjBool] ajTrue if the type can be fixed
** @@
******************************************************************************/

static AjBool seqTypeFix (AjPSeq thys, ajint itype)
{
    AjBool ret = ajFalse;

    ajDebug ("seqTypeFix '%s'\n", seqType[itype].Name);

/*
** if ungapped, remove any gap characters
*/

    if (!seqType[itype].Gaps)
	ajStrDegap(&thys->Seq);

    if (seqType[itype].Ambig)
    {
	/*
	 ** list the bad characters, change to 'X' or 'N'
	 */
	switch (itype)
	{
	case ISPROT:
	    ret = seqTypeFixReg(thys, itype, 'X');
	    break;
	case ISNUC:
	    ret = seqTypeFixReg(thys, itype, 'N');
	    break;
	case ISANY:
	    if (ajSeqIsNuc(thys))
		ret = seqTypeFixReg(thys, itype, 'N');
	    else
		ret = seqTypeFixReg(thys, itype, 'X');
	    break;
	default:
	    ajErr("Unknown sequence type code for '%c'", seqType[itype].Name);
	    return ajFalse;
	}
    }

    return seqTypeTestI (thys, itype);
}

/* @funcstatic seqTypeFixReg **************************************************
**
** Fixes (if possible) unacceptable sequence characters by removing gaps
** (if no gaps are allowed) and by setting ambiguity codes (if they
** are allowed).
**
** @param [P] thys [AjPSeq] Sequence object
** @param [R] itype [ajint] Sequence type index
** @param [R] fixchar [char] Character to replace with
** @return [AjBool] ajTrue if the type can be fixed
** @@
******************************************************************************/

static AjBool seqTypeFixReg (AjPSeq thys, ajint itype, char fixchar)
{
    AjBool ret = ajFalse;
    AjPRegexp badchar;
    ajint ioff = 0;
    ajint lastioff = -1;
    ajint ilen = 0;
    ajint i;

    ajDebug ("seqTypeFixReg '%s'\n", seqType[itype].Name);
    ajDebug ("Seq old '%S'\n", thys->Seq);
    badchar = seqType[itype].Badchars();

    while (ajRegExec(badchar, thys->Seq))
    {
	ilen = ajRegLenI(badchar, 0);
	ioff = ajRegOffset(badchar);
	lastioff = ioff;
	if (lastioff >= ioff)
	    ajFatal("failed to fix sequence type - problem at position %d\n",
		    lastioff);
	ajDebug("Fix string at %d len %d\n", ioff, ilen);
	for (i=0;i<ilen;i++)
	    ajStrReplaceK(&thys->Seq, ++ioff, fixchar, 1);
    }
    ajDebug ("Seq new '%S'\n", thys->Seq);
    ret = ajTrue;
    return ret;
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

static void seqTypeSet (AjPSeq thys, AjPStr Type)
{

    char* cp = ajStrStr(Type);

    ajDebug ("seqTypeSet '%S'\n", Type);

    switch (*cp)
    {
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

/* @func ajSeqTypeCheckS ******************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used for input validation - writes error message if the type check fails
**
** @param [P] pthys [AjPStr*] Sequence string
** @param [P] type [AjPStr] Sequence type
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeCheckS (AjPStr* pthys, AjPStr type)
{

/*    AjPStr tmpstr = NULL; */
    AjPRegexp badchars;
    ajint itype = -1;

    /* ajDebug ("ajSeqTypeCheckS type '%S' seq '%S'\n", type, *pthys); */

    if (!ajStrLen(type))	   /* nothing given - anything goes */
    {
	ajSeqGapS (pthys, seqGap);
	return ajTrue;
    }

     if (!seqFindType (type, &itype))
    {
	ajErr ("Sequence type '%S' unknown", type);
	return ajFalse;
    }

    ajDebug ("ajSeqTypeCheckS type '%s' found (%s)\n",
	     seqType[itype].Name, seqType[itype].Desc);

    if (seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGapS (pthys, seqGap);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrDegap (pthys);
    }

    /* no need to test sequence type, we will test every character below */

    /* Calling funclist seqType() */
    badchars = seqType[itype].Badchars();
    if (!ajRegExec(badchars, *pthys))
    {
	if (seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    (void) ajStrConvertCC (pthys,
				   seqType[itype].ConvertFrom,
				   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

/*
    if (seqTypeFix(*pthys, itype))
	return ajTrue;

    if (!ajRegExec(badchars, (*pthys)->Seq))
    {
	ajRegSubI (badchars, 1, &tmpstr);
	ajErr ("Sequence must be %s,\n found bad character '%c'",
	       seqType[itype].Desc, ajStrChar(tmpstr, 0));
	ajStrDel(&tmpstr);
	return ajFalse;
    }
*/
    return ajTrue;
}

/* @func ajSeqTypeCheckIn *****************************************************
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

AjBool ajSeqTypeCheckIn (AjPSeq thys, AjPSeqin seqin)
{
    
    ajint itype = -1;
    AjPRegexp badchars;
    AjPStr tmpstr = NULL;
    
    AjPStr Type = seqin->Inputtype; /* ACD file had a predefined seq type */
    
    ajDebug ("testing sequence '%s' type '%S' IsNuc %B IsProt %B\n",
	     ajSeqName(thys), seqin->Inputtype, seqin->IsNuc, seqin->IsProt);
    
    if (seqin->IsNuc)
	ajSeqSetNuc(thys);
    
    if (seqin->IsProt)
	ajSeqSetProt(thys);
    
    if (seqin->Query && ajStrLen(seqin->Query->DbType))
    {
	seqTypeSet(thys, seqin->Query->DbType);
    }
    
    if (!ajStrLen(Type)) 		/* nothing given - anything goes */
    {
	ajSeqGap (thys, seqGap, 0);
	return ajTrue;
    }
    
    if (!seqFindType (Type, &itype))
    {
	ajErr ("Sequence type '%S' unknown", Type);
	return ajFalse;
    }

    ajDebug ("ajSeqTypeCheckIn type '%s' found (%s)\n",
	     seqType[itype].Name, seqType[itype].Desc);

    if (seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGap (thys, seqGap, 0);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrDegap (&thys->Seq);
    }

    if (seqType[itype].Type == ISPROT && !ajSeqIsProt(thys))
    {
	ajErr("Sequence is not a protein\n");
	return ajFalse;
    }
    if (seqType[itype].Type == ISNUC && !ajSeqIsNuc(thys))
    {
	ajErr("Sequence is not nucleic\n");
	return ajFalse;
    }

    /* Calling funclist seqType() */
    badchars = seqType[itype].Badchars();
    if (!ajRegExec(badchars, thys->Seq))
    {
	if (seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    (void) ajStrConvertCC (&thys->Seq,
				   seqType[itype].ConvertFrom,
				   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

    if (seqTypeFix(thys, itype))	/* this will reuse badchars */
	return ajTrue;

    if (!ajRegExec(badchars, thys->Seq)) /* must check again */
    {
	ajRegSubI (badchars, 1, &tmpstr);
	ajErr ("Sequence must be %s,\n found bad character '%c'",
	       seqType[itype].Desc, ajStrChar(tmpstr, 0));
	ajStrDel(&tmpstr);
	return ajFalse;
    }
    return ajTrue;
}

/* @func ajSeqTypeAnyS ********************************************************
**
** Checks sequence type for any valid sequence character (but no gaps)
**
** @param [r] pthys [AjPStr*] Sequence string (not changed at present)
** @return [char] bad character if found, or null.
** @@
******************************************************************************/

char ajSeqTypeAnyS (AjPStr* pthys)
{
    ajDebug ("seqTypeAnyS test\n");

    return seqTypeTest (*pthys, seqTypeCharAny());
}

/* @func ajSeqTypeDnaS ********************************************************
**
** Checks sequence type for DNA.
**
** RNA codes are accepted but are converted to DNA.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeDnaS (AjPStr* pthys)
{
   ajDebug ("seqTypeDnaS test\n");

    (void) ajStrConvertCC (pthys, seqCharNucRNA, seqCharNucDNA);

    return seqTypeTest (*pthys, seqTypeCharNuc());
}

/* @func ajSeqTypeRnaS ********************************************************
**
** Checks sequence type for RNA.
**
** DNA codes are accepted but are converted to RNA.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeRnaS (AjPStr* pthys)
{
    ajDebug ("seqTypeRnaS test\n");

    (void) ajStrConvertCC (pthys, seqCharNucDNA, seqCharNucRNA);

    return seqTypeTest (*pthys, seqTypeCharNuc());
}

/* @func ajSeqTypePurednaS ****************************************************
**
** Checks sequence type for pure (unambiguous) DNA.
**
** RNA codes are accepted but are converted to DNA.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePurednaS (AjPStr *pthys)
{
    ajDebug ("seqTypePurednaS test\n");

    (void) ajStrConvertCC (pthys, seqCharNucRNA, seqCharNucDNA);

    return seqTypeTest (*pthys, seqTypeCharNucPure());
}

/* @func ajSeqTypePurernaS ****************************************************
**
** Checks sequence type for pure (unambiguous) RNA.
**
** DNA codes are accepted but are converted to RNA.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePurernaS (AjPStr* pthys)
{
    ajDebug ("seqTypePurernaS test\n");

    (void) ajStrConvertCC (pthys, seqCharNucDNA, seqCharNucRNA);

    return seqTypeTest (*pthys, seqTypeCharNucPure());
}

/* @func ajSeqTypeNucS ********************************************************
**
** Checks sequence type for nucleotide.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] pthys [AjPStr*] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeNucS (AjPStr* pthys)
{
    static char seqchars[256] = "";

    if (!*seqchars)
    {
    }

    ajDebug ("seqTypeNuc test\n");

    return seqTypeTest (*pthys, seqTypeCharNuc());
}

/* @func ajSeqTypePurenucS ****************************************************
**
** Checks sequence type for pure (unambiguous) nucleotide.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] pthys [AjPStr*] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePurenucS (AjPStr* pthys)
{
    ajDebug ("seqTypePurenucS test\n");

    return seqTypeTest (*pthys, seqTypeCharNucPure());
}

/* @func ajSeqTypeGapnucS *****************************************************
**
** Checks sequence type for nucleotide with gaps.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] pthys [AjPStr*] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapnucS (AjPStr* pthys)
{
    ajDebug ("seqTypeGapnucS test\n");

    return seqTypeTest (*pthys, seqTypeCharNucGap());
}


/* @func ajSeqTypeGapdnaS *****************************************************
**
** Checks sequence type for DNA with gaps.
**
** RNA codes are accepted an converted to DNA.
**
** @param [r] pthys [AjPStr*] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapdnaS (AjPStr* pthys)
{
    ajDebug ("seqTypeGapdnaS test\n");

    (void) ajStrConvertCC (pthys, seqCharNucRNA, seqCharNucDNA);

    return seqTypeTest (*pthys, seqTypeCharNucGap());
}

/* @func ajSeqTypeGaprnaS *****************************************************
**
** Checks sequence type for RNA with gaps.
**
** DNA codes are accepted an converted to RNA.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGaprnaS (AjPStr* pthys)
{
    ajDebug ("seqTypeGaprnaS test\n");

    (void) ajStrConvertCC (pthys, seqCharNucDNA, seqCharNucRNA);

    return seqTypeTest (*pthys, seqTypeCharNucGap());
}

/* @func ajSeqTypeProtS *******************************************************
**
** Checks sequence type for protein.
**
** A stop at the end is allowed (but is removed).
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeProtS (AjPStr* pthys)
{
    ajDebug ("seqTypeProtS test\n");

    (void) seqTypeStopTrimS(pthys);

    return seqTypeTest (*pthys, seqTypeCharProt());
}

/* @func ajSeqTypePureprotS ***************************************************
**
** Checks sequence type for (unambiguous) protein.
**
** A stop at the end is allowed (but is removed).
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypePureprotS (AjPStr* pthys)
{
    ajDebug ("seqTypePureprotS test\n");

    (void) seqTypeStopTrimS(pthys);

    return seqTypeTest (*pthys, seqTypeCharProtPure());
}

/* @func ajSeqTypeAnyprotS ****************************************************
**
** Checks sequence type for anything that can be in a protein sequence
**
** Stop codes are replaced with gaps.
**
** @param [r] pthys [AjPStr*] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeAnyprotS (AjPStr* pthys)
{
    ajDebug ("seqTypeAnyprotS test\n");

    return seqTypeTest (*pthys, seqTypeCharProtAny());
}

/* @func ajSeqTypeGapprotS ****************************************************
**
** Checks sequence type for protein with gaps.
**
** Stop codes are replaced with gaps.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapprotS (AjPStr *pthys)
{
    ajDebug ("seqTypeGapprotS test\n");

    (void) ajStrConvertCC (pthys, seqCharProtStop, "-");

    return seqTypeTest (*pthys, seqTypeCharProtGap());
}

/* @func ajSeqTypeStopprotS ***************************************************
**
** Checks sequence type for protein.
**
** Stops ('*') are allowed so this could be a 3 frame translation of DNA.
**
** @param [r] pthys [AjPStr*] Sequence string
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeStopprotS (AjPStr* pthys)
{
    ajDebug ("seqTypeStopprotS test\n");

    return seqTypeTest (*pthys, seqTypeCharProtStop());
}

/* @func ajSeqTypeGapanyS *****************************************************
**
** Checks sequence type for any sequence with gaps.
**
** Stops ('*') are allowed so this could be a 3 frame translation of DNA.
**
** @param [r] pthys [AjPStr*] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapanyS (AjPStr* pthys)
{
    ajDebug ("seqTypeGapanyS test\n");

    return seqTypeTest (*pthys, seqTypeCharAnyGap());
}

/* @func ajSeqGap *************************************************************
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

void ajSeqGap (AjPSeq thys, char gapc, char padc)
{
    seqGapSL (&thys->Seq, gapc, padc, 0);
}

/* @func ajSeqGapLen **********************************************************
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

void ajSeqGapLen (AjPSeq thys, char gapc, char padc, ajint ilen)
{
    seqGapSL (&thys->Seq, gapc, padc, ilen);
}

/* @func ajSeqGapS ************************************************************
**
** Sets non-sequence characters to valid gap characters,
** and pads with extra gaps if necessary to a specified length
**
** @param [u] seq [AjPStr*] Sequence
** @param [r] gapc [char] Standard gap character
** @return [void]
** @@
******************************************************************************/

void ajSeqGapS (AjPStr* seq, char gapc)
{
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

static void seqGapSL (AjPStr* seq, char gapc, char padc, ajint ilen)
{
    ajint i;
    static char* newgap=NULL;
    static ajint igap;
    char* cp;
    char endc = gapc;
    
    igap = strlen(seqCharGapTest);
    if (!newgap){
	newgap = ajCharNewL(igap);
	newgap[0] = '\0';
    }
    
    /* Set the newgap string to match gapc */
    
    if (*newgap != gapc)
    {
	for (i=0; i < igap; i++)
	    newgap[i] = gapc;
	newgap[i] = '\0';
    }
    
    /*
       //  ajDebug("seqGapSL gapc '%c' padc '%c' len %d seqCharGapTest '%s'\n",
		   //	  gapc, padc, ilen, seqCharGapTest);
		   //  ajDebug("seqGapSL before '%S'\n", *seq);
		   */
    
    if (ilen)
	(void) ajStrModL (seq, ilen+1);
    else
	(void) ajStrMod(seq);
    
    (void) ajStrConvertCC (seq, seqCharGapTest, newgap);
    
    if (padc)
    {				/* start and end characters updated */
	endc = padc;
	for (cp = ajStrStr(*seq);
	     strchr(seqCharGapTest, *cp); cp++) /* pad start */
	{
	    *cp = padc;
	}
	cp = ajStrStr(*seq);
	for (i=ajStrLen(*seq) - 1; i && strchr(seqCharGapTest, cp[i]);  i--)
	    cp[i] = padc;
    }
    
    if (ajStrLen(*seq) < ilen) 	/* ilen can be zero to skip this */
    {
	cp = ajStrStr(*seq);
	for (i=ajStrLen(*seq); i < ilen; i++)
	    cp[i] = endc;
	cp[ilen] = '\0';
	ajStrFix(*seq);
    }
    
    /*  ajDebug("seqGapSL after  '%S'\n", *seq); */
    
    return;
}

/* @funcstatic seqTypeStopTrimS ***********************************************
**
** Removes a trailing stop (asterisk) from a protein sequence
**
** @param [P] pthys [AjPStr*] Sequence string
** @return [AjBool] ajTrue if a stop was removed.
** @@
******************************************************************************/

static AjBool seqTypeStopTrimS (AjPStr* pthys)
{
    if (strchr(seqCharProtStop,ajStrChar(*pthys, -1)))
    {
	ajDebug("Trailing stop removed %c\n", ajStrChar(*pthys, -1));
	(void) ajStrTrim(pthys, -1);
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

void ajSeqSetNuc (AjPSeq thys)
{
    (void) ajStrAssC (&thys->Type, "N");
}

/* @func ajSeqSetProt *********************************************************
**
** Sets a sequence type to "protein"
**
** @param [P] thys [AjPSeq] Sequence object
** @return [void]
** @@
******************************************************************************/

void ajSeqSetProt (AjPSeq thys)
{
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

void ajSeqType (AjPSeq thys)
{
    ajDebug ("ajSeqType current: %S\n", thys->Type);

    if (ajStrLen(thys->Type))
	return;

    if (ajSeqIsNuc (thys))
    {
	ajSeqSetNuc(thys);
	ajDebug ("ajSeqType nucleotide: %S\n", thys->Type);
	return;
    }
    if (ajSeqIsProt (thys))
    {
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

void ajSeqPrintType (AjPFile outf, AjBool full)
{
    ajint i;

    char* typeName[] = {"ANY", "NUC", "PRO"};

    ajFmtPrintF (outf, "\n#Sequence Types\n");
    ajFmtPrintF (outf, "# Name            Gap N/P Desciption\n");
    ajFmtPrintF (outf, "seqType {\n");
    for (i=0; seqType[i].Name; i++)
    {
	ajFmtPrintF (outf, "  %-15s %3B %s \"%s\"\n",
		     seqType[i].Name, seqType[i].Gaps,
		     typeName[seqType[i].Type], seqType[i].Desc);

    }
    ajFmtPrintF (outf, "}\n");
}

/* @funcstatic seqTypeTest ****************************************************
**
** Checks sequence contains only expected characters.
**
** Returns an invalid character for failure, or a null character for success.
**
** @param [R] thys [AjPStr] Sequence string
** @param [R] badchars [AjPRegexp] Regular expression for
**                                 sequence characters disallowed
** @return [char] invalid character if any.
******************************************************************************/

static char seqTypeTest (AjPStr thys, AjPRegexp badchars)
{
    AjPStr tmpstr = NULL;
    char ret = '\0';

    if (!ajStrLen(thys))
	return ret;

    ajDebug ("seqTypeTest, Sequence '%S'\n",
	     thys);
    if (!ajRegExec(badchars, thys))
	return ret;

    ajRegSubI (badchars, 1, &tmpstr);
    ret = ajStrChar(tmpstr, 0);
    ajDebug ("seqTypeTest, Sequence had bad character '%c' (%x) at %d of %d/%d\n '%S'",
	     ret, ret,
	     ajRegOffset(badchars),
	     ajStrLen(thys), strlen(ajStrStr(thys)), tmpstr);
    ajStrDel(&tmpstr);

   return ret;
}

/* @funcstatic seqTypeCharAny *************************************************
**
** Returns regular expression to test for type Any
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharAny (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegAny)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharNucPure,
		    seqCharNucAmbig);
	seqtypeRegAny = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegAny;
}

/* @funcstatic seqTypeCharAnyGap **********************************************
**
** Returns regular expression to test for type Any with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharAnyGap (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegAnyGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);
	seqtypeRegAnyGap = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegAnyGap;
}

/* @funcstatic seqTypeCharNuc *************************************************
**
** Returns regular expression to test for nucleotide bases
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNuc (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegNuc)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s]+)",
		    seqCharNucPure,
		    seqCharNucAmbig);
	seqtypeRegNuc = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegNuc;
}

/* @funcstatic seqTypeCharNucGap **********************************************
**
** Returns regular expression to test for nucleotide bases with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucGap (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegNucGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);
	seqtypeRegNucGap = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegNucGap;
}

/* @funcstatic seqTypeCharNucPure *********************************************
**
** Returns regular expression to test for nucleotide bases
** with no ambiguity
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucPure (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegNucPure)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s]+)",
		    seqCharNucPure);
	seqtypeRegNucPure = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegNucPure;
}

/* @funcstatic seqTypeCharProt ************************************************
**
** Returns regular expression to test for protein residues
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProt (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegProt)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig);
	seqtypeRegProt = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegProt;
}

/* @funcstatic seqTypeCharProtAny *********************************************
**
** Returns regular expression to test for protein residues or gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtAny (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegProtAny)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharGap);
	seqtypeRegProtAny = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegProtAny;
}

/* @funcstatic seqTypeCharProtGap *********************************************
**
** Returns regular expression to test for protein residues or gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtGap (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegProtGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharGap);
	seqtypeRegProtGap = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegProtGap;
}

/* @funcstatic seqTypeCharProtPure ********************************************
**
** Returns regular expression to test for protein residues
** with no ambiguity
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtPure (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegProtPure)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s]+)",
		    seqCharProtPure);
	seqtypeRegProtPure = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegProtPure;
}

/* @funcstatic seqTypeCharProtStop ********************************************
**
** Returns regular expression to test for protein residues or stop codons
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtStop (void)
{
    AjPStr regstr = NULL;

    if (!seqtypeRegProtStop)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop);
	seqtypeRegProtStop = ajRegComp (regstr);
	ajStrDel (&regstr);
    }
    return seqtypeRegProtStop;
}

/* @funcstatic seqFindType ****************************************************
**
** Returns sequence type index and ajTrue if type was found
**
** @param [R] typename [AjPStr] Sequence type
** @param [W] typenum [ajint*] Sequence type index
** @return [AjBool] ajTrue if sequence type was found
**
******************************************************************************/

static AjBool seqFindType (AjPStr typename, ajint* typenum)
{
    ajint i;
    ajint itype = -1;

    for (i = 0; seqType[i].Name; i++)
    {
	if (ajStrMatchCaseC(typename, seqType[i].Name))
	{
	    itype = i;
	    break;
	}
    }

    if (itype <0)
    {
	*typenum = i;
	return ajFalse;
    }

    *typenum = itype;
    return ajTrue;
}

/* @func ajSeqTypeIsProt ******************************************************
**
** Returns ajTrue is sequence type can be a protein (or 'any')
**
** @param [R] typename [AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be protein
**
******************************************************************************/

AjBool ajSeqTypeIsProt (AjPStr typename)
{
    ajint itype;
    if (seqFindType(typename, &itype))
    {
	switch (seqType[itype].Type)
	{
	case ISNUC:
	    return ajFalse; break;
	default:
	    return ajTrue; break;
	}
    }
    return ajFalse;
}

/* @func ajSeqTypeIsNuc *******************************************************
**
** Returns ajTrue is sequence type can be a nucleotide (or 'any')
**
** @param [R] typename [AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be nucleotide
**
******************************************************************************/

AjBool ajSeqTypeIsNuc (AjPStr typename)
{
    ajint itype;
    if (seqFindType(typename, &itype))
    {
	switch (seqType[itype].Type)
	{
	case ISPROT:
	    return ajFalse; break;
	default:
	    return ajTrue; break;
	}
    }
    return ajFalse;
}

/* @func ajSeqTypeIsAny *******************************************************
**
** Returns ajTrue is sequence type can be a protein or nucleotide
**
** @param [R] typename [AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be protein or nucleotide
**
******************************************************************************/

AjBool ajSeqTypeIsAny (AjPStr typename)
{
    ajint itype;
    if (seqFindType(typename, &itype))
    {
	switch (seqType[itype].Type)
	{
	case ISNUC:
	    return ajFalse; break;
	case ISPROT:
	    return ajFalse; break;
	default:
	    return ajTrue; break;
	}
    }
    return ajFalse;
}


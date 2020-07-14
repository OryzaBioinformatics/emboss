/******************************************************************************
** @source AJAX seqtype functions
**
** @author Copyright (C) 2002 Peter Rice
** @version 1.0
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/

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
** @attr Ambig [AjBool] True if ambiguity codes are allowed
** @attr Type [ajint] enumerated ISANY=0 ISNUC=1 ISPROT=2 
** @attr ConvertFrom [char*] Convert each of these characters to the
**                           ConvertTo equivalent
** @attr ConvertTo [char*] Equivalent for each sequence character in
**                         ConvertFrom
** @attr Badchars [(AjPRegexp*)] Test function
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
} SeqOType;

#define SeqPType SeqOType*




enum ProtNuc {ISANY=0, ISNUC=1, ISPROT=2};




/*
** gaps only allowed if it says so
** gap conversion is a separate attribute, along with case convserion
*/

static AjBool     seqFindType(const AjPStr type_name, ajint* typenum);
static void       seqGapSL(AjPStr* seq, char gapc, char padc, ajint ilen);
static AjBool     seqTypeFix(AjPSeq thys, ajint itype);
static AjBool     seqTypeFixReg(AjPSeq thys, ajint itype, char fixchar);
static void       seqTypeSet(AjPSeq thys, const AjPStr Type);
static AjBool     seqTypeStopTrimS(AjPStr* pthys);
static char       seqTypeTest(const AjPStr thys, AjPRegexp badchars);
static AjBool     seqTypeTestI(AjPSeq thys, ajint itype);
static AjPRegexp  seqTypeCharAny(void);
static AjPRegexp  seqTypeCharAnyGap(void);
static AjPRegexp  seqTypeCharDnaGap(void);
static AjPRegexp  seqTypeCharNuc(void);
static AjPRegexp  seqTypeCharNucGap(void);
static AjPRegexp  seqTypeCharNucGapPhylo(void);
static AjPRegexp  seqTypeCharNucPure(void);
static AjPRegexp  seqTypeCharProt(void);
static AjPRegexp  seqTypeCharProtAny(void);
static AjPRegexp  seqTypeCharProtGap(void);
static AjPRegexp  seqTypeCharProtGapPhylo(void);
static AjPRegexp  seqTypeCharProtPure(void);
static AjPRegexp  seqTypeCharProtStop(void);
static AjPRegexp  seqTypeCharRnaGap(void);

static AjPRegexp seqtypeRegAny      = NULL;
static AjPRegexp seqtypeRegAnyGap   = NULL;
static AjPRegexp seqtypeRegDnaGap   = NULL;
static AjPRegexp seqtypeRegNuc      = NULL;
static AjPRegexp seqtypeRegNucGap   = NULL;
static AjPRegexp seqtypeRegNucPure  = NULL;
static AjPRegexp seqtypeRegProt     = NULL;
static AjPRegexp seqtypeRegProtAny  = NULL;
static AjPRegexp seqtypeRegProtGap  = NULL;
static AjPRegexp seqtypeRegProtPure = NULL;
static AjPRegexp seqtypeRegProtStop = NULL;
static AjPRegexp seqtypeRegRnaGap   = NULL;




/*
** gap characters known are:
**
** . GCG and most others
** - Phylip and some alignment output
** ~ GCG for gaps at ends
** * Staden for DNA but stop for protein (fix on input?)
** O Phylip (fix on input?)
*/




char seqCharProt[]      = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwyBUXZbuxz*?";
char seqCharProtPure[]  = "ACDEFGHIKLMNPQRSTVWYacdefghiklmnpqrstvwy";
char seqCharProtAmbig[] = "BUXZbuxz?";
char seqCharProtStop[]  = "*";
char seqCharProtU[]     = "Uu";
char seqCharProtX[]     = "Xx";
char seqCharNuc[]       = "ACGTUacgtuBDHKMNRSVWXYbdhkmnrsvwxy?";
char seqCharNucPure[]   = "ACGTUacgtu";
char seqCharNucAmbig[]  = "BDHKMNRSVWXYbdhkmnrsvwxy?";
char seqCharGap[]       = ".~Oo-";	/* phylip uses O */
char seqCharNucDna[]    = "ACGTacgtBDHKMNRSVWXYbdhkmnrsvwxy?";
char seqCharNucRna[]    = "ACGUacguBDHKMNRSVWXYbdhkmnrsvwxy?";
char seqCharGapany[]    = ".~Oo-";	/* phylip uses O */
char seqCharGapdash[]   = "-";
char seqCharGapdot[]    = ".";
char seqGap = '-';		/* the (only) EMBOSS gap character */
char seqCharGapTest[]   = " .~Oo-";   /* phylip uses O - don't forget space */
char seqCharPhylo[]       = "?";	/* phylip uses ? for unknown or gap */




/* @funclist seqType **********************************************************
**
** Functions to test each sequence type
**
******************************************************************************/

static SeqOType seqType[] =
{
/*   "name"            Gaps     Ambig    Type    CvtFrom CvtTo
         BadcharsFunction Description */
    {"any",            AJFALSE, AJTRUE,  ISANY,  "?",    "X",
	 seqTypeCharAny,
	 "any valid sequence"},		/* reset type */
    {"gapany",         AJTRUE,  AJTRUE,  ISANY,  "?",    "X",
	 seqTypeCharAnyGap,
	 "any valid sequence with gaps"}, /* reset type */
    {"dna",            AJFALSE, AJTRUE,  ISNUC,  "?XxUu", "NNNTt",
	 seqTypeCharNuc,
	 "DNA sequence"},
    {"puredna",        AJFALSE, AJFALSE, ISNUC,  "Uu", "Tt",
	 seqTypeCharNucPure,
	 "DNA sequence, bases ACGT only"},
    {"gapdna",         AJTRUE,  AJTRUE,  ISNUC,  "?XxUu", "NNNTt",
	 seqTypeCharNucGap,
	 "DNA sequence with gaps"},
    {"gapdnaphylo",     AJTRUE,  AJTRUE,  ISNUC, "Uu",  "Tt",
	 seqTypeCharNucGapPhylo,
	 "DNA sequence with gaps and queries"},
    {"rna",            AJFALSE, AJTRUE,  ISNUC,  "?XxTt", "NNNUu",
	 seqTypeCharAny,
	 "RNA sequence"},
    {"purerna",        AJFALSE, AJFALSE, ISNUC,  "Tt", "Uu",
	 seqTypeCharNucPure,
	 "RNA sequence, bases ACGU only"},
    {"gaprna",         AJTRUE,  AJTRUE,  ISNUC,  "?XxTt", "NNNUu",
	 seqTypeCharNucGap,
	 "RNA sequence with gaps"},
    {"gaprnaphylo",     AJTRUE,  AJTRUE,  ISNUC, "Tt",  "Uu",
	 seqTypeCharNucGapPhylo,
	 "RNA sequence with gaps and queries"},
    {"nucleotide",     AJFALSE, AJTRUE,  ISNUC,  "?Xx",   "NNN",
	 seqTypeCharNuc,
	 "nucleotide sequence"},
    {"purenucleotide", AJFALSE, AJFALSE, ISNUC,  NULL,  NULL,
	 seqTypeCharNucPure,
	 "nucleotide sequence, bases ACGTU only"},
    {"gapnucleotide",  AJTRUE,  AJTRUE,  ISNUC,  "?Xx",   "NNN",
	 seqTypeCharNucGap,
	 "nucleotide sequence with gaps"},
    {"gapnucleotidephylo",  AJTRUE,  AJTRUE,  ISNUC,  "",   "",
	 seqTypeCharNucGapPhylo,
	 "nucleotide sequence with gaps and queries"},
    {"protein",        AJFALSE, AJTRUE,  ISPROT, "?*",  "XX",
	 seqTypeCharProt,
	 "protein sequence"},
    {"pureprotein",    AJFALSE, AJFALSE, ISPROT, NULL,  NULL,
	 seqTypeCharProtPure,
	 "protein sequence without BZ U X or *"},
    {"stopprotein",    AJFALSE, AJTRUE,  ISPROT, "?",   "X",
	 seqTypeCharProtStop,
	 "protein sequence with a possible stop"},
    {"gapprotein",     AJTRUE,  AJTRUE,  ISPROT, "?*",  "XX",
	 seqTypeCharProtGap,
	 "protein sequence with gaps"},
    {"gapproteinphylo",     AJTRUE,  AJTRUE,  ISPROT, "",  "",
	 seqTypeCharProtGapPhylo,
	 "protein sequence with gaps, stops and queries"},
    {"proteinstandard",AJFALSE, AJTRUE,  ISPROT, "?*Uu", "XXXx",
	 seqTypeCharProt,
	 "protein sequence with no selenocysteine"},
    {"stopproteinstandard",AJFALSE, AJTRUE, ISPROT, "?Uu", "XXx",
	 seqTypeCharProtStop,
	 "protein sequence with a possible stop but no selenocysteine"},
    {"gapproteinstandard", AJTRUE,  AJTRUE, ISPROT, "?*Uu", "XXXx",
	 seqTypeCharProtGap,
	 "protein sequence with gaps but no selenocysteine"},
    {NULL,             AJFALSE, AJTRUE,  ISANY,  NULL,  NULL,
	 NULL,
	 NULL}
};




/* @funcstatic seqTypeTestI ***************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used only for testing, so never writes any error message
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] itype [ajint] Sequence type index
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

static AjBool seqTypeTestI(AjPSeq thys, ajint itype)
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

    if(seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGap(thys, seqGap, 0);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrDegap(&thys->Seq);
    }

    if(seqType[itype].Type == ISPROT && !ajSeqIsProt(thys))
    {
	ajDebug("Sequence is not a protein\n");
	return ajFalse;
    }

    if(seqType[itype].Type == ISNUC && !ajSeqIsNuc(thys))
    {
	ajDebug("Sequence is not nucleic\n");
	return ajFalse;
    }

    /* Calling funclist seqType() */
    badchars = seqType[itype].Badchars();
    if(!ajRegExec(badchars, thys->Seq))
    {
	if(seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    ajStrConvertCC(&thys->Seq,
			   seqType[itype].ConvertFrom,
			   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

    ajRegSubI(badchars, 1, &tmpstr);
    ajDebug("seqTypeTestI: Sequence must be %s: found bad character '%c'\n",
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
** @param [u] thys [AjPSeq] Sequence object
** @param [r] itype [ajint] Sequence type index
** @return [AjBool] ajTrue if the type can be fixed
** @@
******************************************************************************/

static AjBool seqTypeFix(AjPSeq thys, ajint itype)
{
    AjBool ret = ajFalse;

    ajDebug("seqTypeFix '%s'\n", seqType[itype].Name);

    /*
     ** if ungapped, remove any gap characters
     */

    if(!seqType[itype].Gaps)
	ajStrDegap(&thys->Seq);

    if(seqType[itype].Ambig)
    {
	/*
	 ** list the bad characters, change to 'X' or 'N'
	 */
	switch(itype)
	{
	case ISPROT:
	    ret = seqTypeFixReg(thys, itype, 'X');
	    break;
	case ISNUC:
	    ret = seqTypeFixReg(thys, itype, 'N');
	    break;
	case ISANY:
	    if(ajSeqIsNuc(thys))
		ret = seqTypeFixReg(thys, itype, 'N');
	    else
		ret = seqTypeFixReg(thys, itype, 'X');
	    break;
	default:
	    ajDie("Unknown sequence type code for '%c'", seqType[itype].Name);
	    return ajFalse;
	}
    }

    if (ajStrMatchCC(seqType[itype].Name, "pureprotein"))
	    seqTypeStopTrimS(&thys->Seq);

    return seqTypeTestI(thys, itype);
}




/* @funcstatic seqTypeFixReg **************************************************
**
** Fixes (if possible) unacceptable sequence characters by removing gaps
** (if no gaps are allowed) and by setting ambiguity codes (if they
** are allowed).
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] itype [ajint] Sequence type index
** @param [r] fixchar [char] Character to replace with
** @return [AjBool] ajTrue if the type can be fixed
** @@
******************************************************************************/

static AjBool seqTypeFixReg(AjPSeq thys, ajint itype, char fixchar)
{
    AjBool ret = ajFalse;
    AjPRegexp badchar;
    ajint ioff     = 0;
    ajint lastioff = -1;
    ajint ilen     = 0;
    ajint i;

    ajDebug("seqTypeFixReg '%s'\n", seqType[itype].Name);
    /*ajDebug("Seq old '%S'\n", thys->Seq);*/
    badchar = seqType[itype].Badchars();

    while(ajRegExec(badchar, thys->Seq))
    {
	ilen = ajRegLenI(badchar, 0);
	ioff = ajRegOffset(badchar);
	lastioff = ioff;
	if(lastioff >= ioff)
	    ajFatal("failed to fix sequence type - problem at position %d\n",
		    lastioff);
	ajDebug("Fix string at %d len %d\n", ioff, ilen);
	for(i=0;i<ilen;i++)
	    ajStrReplaceK(&thys->Seq, ++ioff, fixchar, 1);
    }
    /*ajDebug("Seq new '%S'\n", thys->Seq);*/
    ret = ajTrue;

    return ret;
}




/* @funcstatic seqTypeSet *****************************************************
**
** Sets the sequence type. Uses the first character of the type
** which can be N or P
**
** @param [u] thys [AjPSeq] Sequence object
** @param [r] Type [const AjPStr] Sequence type
** @return [void]
** @@
******************************************************************************/

static void seqTypeSet(AjPSeq thys, const AjPStr Type)
{
    const char* cp;

    ajDebug("seqTypeSet '%S'\n", Type);

    cp = ajStrStr(Type);

    switch(*cp)
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
	ajDie("Unknown sequence type '%c'", *cp);
    }

    return;
}




/* @func ajSeqTypeCheckS ******************************************************
**
** Tests the type of a sequence is compatible with a defined type.
** If the type can have gaps, also tests for gap characters.
** Used for input validation - writes error message if the type check fails
**
** @param [u] pthys [AjPStr*] Sequence string
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeCheckS(AjPStr* pthys, const AjPStr type_name)
{
    /*    AjPStr tmpstr = NULL; */
    AjPRegexp badchars;
    ajint itype = -1;

    /* ajDebug("ajSeqTypeCheckS type '%S' seq '%S'\n", type_name, *pthys); */

    if(!ajStrLen(type_name))	   /* nothing given - anything goes */
    {
	ajSeqGapS(pthys, seqGap);
	return ajTrue;
    }

    if(!seqFindType(type_name, &itype))
    {
	ajDie("Sequence type '%S' unknown", type_name);
	return ajFalse;
    }

    ajDebug("ajSeqTypeCheckS type '%s' found (%s)\n",
	    seqType[itype].Name, seqType[itype].Desc);

    if(seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGapS(pthys, seqGap);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrDegap(pthys);
    }

    /* no need to test sequence type, we will test every character below */

    /* Calling funclist seqType() */
    badchars = seqType[itype].Badchars();
    if(!ajRegExec(badchars, *pthys))
    {
	if(seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    ajStrConvertCC(pthys,
			   seqType[itype].ConvertFrom,
			   seqType[itype].ConvertTo);
	}
	return ajTrue;
    }

    /*
       if(seqTypeFix(*pthys, itype))
       return ajTrue;
       
       if(!ajRegExec(badchars,(*pthys)->Seq))
       {
       ajRegSubI(badchars, 1, &tmpstr);
       ajErr("ajSeqTypeCheckS: Sequence must be %s: found bad character '%c'",
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
** @param [u] thys [AjPSeq] Sequence object
** @param [r] seqin [const AjPSeqin] Sequence input object
** @return [AjBool] ajTrue if compatible.
** @@
******************************************************************************/

AjBool ajSeqTypeCheckIn(AjPSeq thys, const AjPSeqin seqin)
{    
    ajint itype = -1;
    AjPRegexp badchars;
    AjPStr tmpstr = NULL;
    
    AjPStr Type;
    
    ajDebug("testing sequence '%s' '%S' type '%S' IsNuc %B IsProt %B\n",
	    ajSeqName(thys), thys->Seq,
	    seqin->Inputtype, seqin->IsNuc, seqin->IsProt);

    Type = seqin->Inputtype; /* ACD file had a predefined seq type */
    
    if(seqin->IsNuc)
	ajSeqSetNuc(thys);
    
    if(seqin->IsProt)
	ajSeqSetProt(thys);
    
    if(seqin->Query && ajStrLen(seqin->Query->DbType))
	seqTypeSet(thys, seqin->Query->DbType);

    
    if(!ajStrLen(Type))		   /* nothing given - anything goes */
    {
	ajSeqGap(thys, seqGap, 0);
	ajDebug("ajSeqTypeCheckIn: OK - no type, gaps converted to '-'\n");
	return ajTrue;
    }
    
    if(!seqFindType(Type, &itype))
    {
	ajDebug("ajSeqTypeCheckIn: rejected - unknown type\n");
	ajDie("Sequence type '%S' unknown", Type);
	return ajFalse;
    }

    ajDebug("ajSeqTypeCheckIn type '%s' found (%s)\n",
	    seqType[itype].Name, seqType[itype].Desc);

    if(seqType[itype].Gaps)
    {
	ajDebug("Convert gaps to '-'\n");
	ajSeqGap(thys, seqGap, 0);
    }
    else
    {
	ajDebug("Remove all gaps\n");
	ajStrDegap(&thys->Seq);
    }

    if(seqType[itype].Type == ISPROT)
    {
	if (ajSeqIsProt(thys))
	{
	    ajSeqSetProt(thys);
	}
	else
	{
	    ajErr("Sequence is not a protein\n");
	    ajDebug("ajSeqTypeCheckIn: rejected - not a protein\n");
	    return ajFalse;
	}
    }

    if(seqType[itype].Type == ISNUC)
    {
	if (ajSeqIsNuc(thys))
	{
	    ajSeqSetNuc(thys);
	}
	else
	{
	    ajErr("Sequence is not nucleic\n");
	    ajDebug("ajSeqTypeCheckIn: rejected - not nucleic\n");
	    return ajFalse;
	}
    }

    /* Calling funclist seqType() */
    badchars = seqType[itype].Badchars();
    if(!ajRegExec(badchars, thys->Seq))
    {
	ajDebug("ajSeqTypeCheckIn: bad characters test passed, convert\n");
	if(seqType[itype].ConvertFrom)
	{
	    ajDebug("Convert '%s' to '%s'\n",
		    seqType[itype].ConvertFrom,
		    seqType[itype].ConvertTo);
	    ajStrConvertCC(&thys->Seq,
			   seqType[itype].ConvertFrom,
			   seqType[itype].ConvertTo);
	}
	ajDebug("ajSeqTypeCheckIn: OK - no badchars\n");
	return ajTrue;
    }

    if(seqTypeFix(thys, itype))		/* this will reuse badchars */
    {
	ajDebug("ajSeqTypeCheckIn: OK - type fixed\n");
	return ajTrue;
    }
    if(ajRegExec(badchars, thys->Seq)) /* must check again */
    {
	ajRegSubI(badchars, 1, &tmpstr);
	ajErr("ajSeqTypeCheckIn: Sequence must be %s: "
	      "found bad character '%c'",
	      seqType[itype].Desc, ajStrChar(tmpstr, 0));
	ajStrDel(&tmpstr);
	ajDebug("ajSeqTypeCheckIn: rejected - still had badchars\n");
	return ajFalse;
    }

    ajDebug("ajSeqTypeCheckIn: OK - fixed finally\n");
    ajDebug("Final sequence '%S' type '%S' IsNuc %B IsProt %B\n",
	    thys->Seq, seqin->Inputtype, seqin->IsNuc, seqin->IsProt);
    return ajTrue;
}





/* @func ajSeqTypeDnaS *****************************************************
**
** Checks sequence type for DNA without gaps.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeDnaS(const AjPStr pthys)
{
    char ret;
    ajDebug("seqTypeDnaS test\n");

    ret = seqTypeTest(pthys, seqTypeCharNuc());
    if (ret)
	return ret;

    return seqTypeTest(pthys, seqTypeCharDnaGap());
}




/* @func ajSeqTypeRnaS *****************************************************
**
** Checks sequence type for Rna without gaps
**
** RNA codes are accepted as is.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeRnaS(const AjPStr pthys)
{
    char ret;
    ajDebug("seqTypeRnaS test\n");

    ret = seqTypeTest(pthys, seqTypeCharNuc());
    if (ret)
	return ret;

    return seqTypeTest(pthys, seqTypeCharRnaGap());
}




/* @func ajSeqTypeGapdnaS *****************************************************
**
** Checks sequence type for Dna with gaps
**
** DNA codes are accepted as is.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapdnaS(const AjPStr pthys)
{
    char ret;
    ajDebug("seqTypeGapdnaS test\n");

    ret = seqTypeTest(pthys, seqTypeCharNucGap());
    if (ret)
	return ret;

    return seqTypeTest(pthys, seqTypeCharDnaGap());
}




/* @func ajSeqTypeGaprnaS *****************************************************
**
** Checks sequence type for Rna with gaps
**
** RNA codes are accepted as is.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGaprnaS(const AjPStr pthys)
{
    char ret;
    ajDebug("seqTypeGaprnaS test\n");

    ret = seqTypeTest(pthys, seqTypeCharNucGap());
    if (ret)
	return ret;

    return seqTypeTest(pthys, seqTypeCharRnaGap());
}




/* @func ajSeqTypeGapnucS *****************************************************
**
** Checks sequence type for nucleotide with gaps.
**
** RNA and DNA codes are accepted as is.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapnucS(const AjPStr pthys)
{
    ajDebug("seqTypeGapnucS test\n");

    return seqTypeTest(pthys, seqTypeCharNucGap());
}




/* @func ajSeqTypeAnyprotS ****************************************************
**
** Checks sequence type for anything that can be in a protein sequence
**
** Stop codes are replaced with gaps.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeAnyprotS(const AjPStr pthys)
{
    ajDebug("seqTypeAnyprotS test\n");

    return seqTypeTest(pthys, seqTypeCharProtAny());
}




/* @func ajSeqTypeGapanyS *****************************************************
**
** Checks sequence type for any sequence with gaps.
**
** Stops ('*') are allowed so this could be a 3 frame translation of DNA.
**
** @param [r] pthys [const AjPStr] Sequence string (unchanged at present)
** @return [char] invalid character if any.
** @@
******************************************************************************/

char ajSeqTypeGapanyS(const AjPStr pthys)
{
    ajDebug("seqTypeGapanyS test\n");

    return seqTypeTest(pthys, seqTypeCharAnyGap());
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

void ajSeqGap(AjPSeq thys, char gapc, char padc)
{
    seqGapSL(&thys->Seq, gapc, padc, 0);

    return;
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

void ajSeqGapLen(AjPSeq thys, char gapc, char padc, ajint ilen)
{
    seqGapSL(&thys->Seq, gapc, padc, ilen);

    return;
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

void ajSeqGapS(AjPStr* seq, char gapc)
{
    seqGapSL(seq, gapc, 0, 0);

    return;
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

static void seqGapSL(AjPStr* seq, char gapc, char padc, ajint ilen)
{
    ajint i;
    static char* newgap = NULL;
    static ajint igap;
    char* cp;
    char endc = gapc;
    
    igap = strlen(seqCharGapTest);
    if(!newgap)
    {
	newgap = ajCharNewL(igap);
	newgap[0] = '\0';
    }
    
    /* Set the newgap string to match gapc */
    
    if(*newgap != gapc)
    {
	for(i=0; i < igap; i++)
	    newgap[i] = gapc;
	newgap[i] = '\0';
    }
    
    
    if(ilen)
	ajStrModL(seq, ilen+1);
    else
	ajStrMod(seq);
    
    ajStrConvertCC(seq, seqCharGapTest, newgap);
    
    if(padc)
    {				/* start and end characters updated */
	endc = padc;
	/* pad start */
	for(cp = ajStrStrMod(seq); strchr(seqCharGapTest, *cp); cp++)
	    *cp = padc;

	cp = ajStrStrMod(seq);
	for(i=ajStrLen(*seq) - 1; i && strchr(seqCharGapTest, cp[i]);  i--)
	    cp[i] = padc;
    }
    
    if(ajStrLen(*seq) < ilen)	   /* ilen can be zero to skip this */
    {
	cp = ajStrStrMod(seq);
	for(i=ajStrLen(*seq); i < ilen; i++)
	    cp[i] = endc;
	cp[ilen] = '\0';
	ajStrFix(seq);
    }
    
    /*  ajDebug("seqGapSL after  '%S'\n", *seq); */

    return;
}




/* @funcstatic seqTypeStopTrimS ***********************************************
**
** Removes a trailing stop (asterisk) from a protein sequence
**
** @param [u] pthys [AjPStr*] Sequence string
** @return [AjBool] ajTrue if a stop was removed.
** @@
******************************************************************************/

static AjBool seqTypeStopTrimS(AjPStr* pthys)
{
    if(strchr(seqCharProtStop,ajStrChar(*pthys, -1)))
    {
	ajDebug("Trailing stop removed %c\n", ajStrChar(*pthys, -1));
	ajStrTrim(pthys, -1);
	return ajTrue;
    }

    return ajFalse;
}




/* @func ajSeqSetNuc **********************************************************
**
** Sets a sequence type to "nucleotide"
**
** @param [u] thys [AjPSeq] Sequence object
** @return [void]
 ** @category modify [AjPSeq] Sets sequence to be nucleotide
** @@
******************************************************************************/

void ajSeqSetNuc(AjPSeq thys)
{
    ajStrAssC(&thys->Type, "N");

    return;
}




/* @func ajSeqSetProt *********************************************************
**
** Sets a sequence type to "protein"
**
** @param [u] thys [AjPSeq] Sequence object
** @return [void]
** @category modify [AjPSeq] Sets sequence to be protein
** @@
******************************************************************************/

void ajSeqSetProt(AjPSeq thys)
{
    ajStrAssC(&thys->Type, "P");

    return;
}




/* @func ajSeqsetSetNuc *******************************************************
**
** Sets a sequence set type to "nucleotide"
**
** @param [u] thys [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetNuc(AjPSeqset thys)
{
    ajStrAssC(&thys->Type, "N");

    return;
}




/* @func ajSeqsetSetProt ******************************************************
**
** Sets a sequence set type to "protein"
**
** @param [u] thys [AjPSeqset] Sequence set object
** @return [void]
** @@
******************************************************************************/

void ajSeqsetSetProt(AjPSeqset thys)
{
    ajStrAssC(&thys->Type, "P");

    return;
}




/* @func ajSeqType ************************************************************
**
** Sets the type of a sequence if it has not yet been defined.
**
** @param [u] thys [AjPSeq] Sequence object
** @return [void]
** @category modify [AjPSeq] Sets the sequence type
** @@
******************************************************************************/

void ajSeqType(AjPSeq thys)
{
    ajDebug("ajSeqType current: %S\n", thys->Type);

    if(ajStrLen(thys->Type))
	return;

    if(ajSeqIsNuc(thys))
    {
	ajSeqSetNuc(thys);
	ajDebug("ajSeqType nucleotide: %S\n", thys->Type);
	return;
    }

    if(ajSeqIsProt(thys))
    {
	ajSeqSetProt(thys);
	ajDebug("ajSeqType protein: %S\n", thys->Type);
	return;
    }

    ajDebug("ajSeqType unknown: %S\n", thys->Type);

    return;
}




/* @func ajSeqPrintType *******************************************************
**
** Prints the seqType definitions.
** For EMBOSS entrails output
**
** @param [u] outf [AjPFile] Output file
** @param [r] full [AjBool] Full output
** @return [void]
******************************************************************************/

void ajSeqPrintType(AjPFile outf, AjBool full)
{
    ajint i;

    char* typeName[] = {"ANY", "NUC", "PRO"};

    ajFmtPrintF(outf, "\n# Sequence Types\n");
    ajFmtPrintF(outf, "# Name                 Gap Ambig N/P From To Desciption\n");
    ajFmtPrintF(outf, "seqType {\n");
    for(i=0; seqType[i].Name; i++)
    {
	if (seqType[i].ConvertFrom)
	    ajFmtPrintF(outf, "  %-20s %3B   %3B %s \"%s\" \"%s\" \"%s\"\n",
			seqType[i].Name, seqType[i].Gaps,
			seqType[i].Ambig, typeName[seqType[i].Type],
			seqType[i].ConvertFrom, seqType[i].ConvertTo,
			seqType[i].Desc);
	else
	    ajFmtPrintF(outf, "  %-20s %3B   %3B %s \"\" \"\" \"%s\"\n",
			seqType[i].Name, seqType[i].Gaps,
			seqType[i].Ambig, typeName[seqType[i].Type],
			seqType[i].Desc);
    }
    ajFmtPrintF(outf, "}\n");

    return;
}




/* @funcstatic seqTypeTest ****************************************************
**
** Checks sequence contains only expected characters.
**
** Returns an invalid character for failure, or a null character for success.
**
** @param [r] thys [const AjPStr] Sequence string
** @param [u] badchars [AjPRegexp] Regular expression for
**                                 sequence characters disallowed
** @return [char] invalid character if any.
******************************************************************************/

static char seqTypeTest(const AjPStr thys, AjPRegexp badchars)
{
    AjPStr tmpstr = NULL;
    char ret = '\0';

    if(!ajStrLen(thys))
	return ret;

    ajDebug("seqTypeTest, Sequence '%S'\n", thys);
    if(!ajRegExec(badchars, thys))
	return ret;

    ajRegSubI(badchars, 1, &tmpstr);
    ret = ajStrChar(tmpstr, 0);
    ajDebug(
     "seqTypeTest, Sequence had bad character '%c' (%x) at %d of %d/%d\n '%S'",
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

static AjPRegexp seqTypeCharAny(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegAny)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharNucPure,
		    seqCharNucAmbig);
	seqtypeRegAny = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegAny;
}




/* @funcstatic seqTypeCharAnyGap **********************************************
**
** Returns regular expression to test for type Any with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharAnyGap(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegAnyGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);
	seqtypeRegAnyGap = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegAnyGap;
}




/* @funcstatic seqTypeCharNuc *************************************************
**
** Returns regular expression to test for nucleotide bases
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNuc(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegNuc)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s]+)",
		    seqCharNucPure,
		    seqCharNucAmbig);
	seqtypeRegNuc = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegNuc;
}




/* @funcstatic seqTypeCharNucGap **********************************************
**
** Returns regular expression to test for nucleotide bases with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucGap(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegNucGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharGap);
	seqtypeRegNucGap = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegNucGap;
}




/* @funcstatic seqTypeCharNucGapPhylo *****************************************
**
** Returns regular expression to test for nucleotide bases with gaps
** and queries
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharNucGapPhylo(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegNucGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s]+)",
		    seqCharNucPure,
		    seqCharNucAmbig,
		    seqCharPhylo,
		    seqCharGap);
	seqtypeRegNucGap = ajRegComp(regstr);
	ajStrDel(&regstr);
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

static AjPRegexp seqTypeCharNucPure(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegNucPure)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s]+)",
		    seqCharNucPure);
	seqtypeRegNucPure = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegNucPure;
}




/* @funcstatic seqTypeCharDnaGap **********************************************
**
** Returns regular expression to test for DNA bases with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharDnaGap(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegDnaGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharNucDna,
		    seqCharGap);
	seqtypeRegDnaGap = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegDnaGap;
}




/* @funcstatic seqTypeCharRnaGap **********************************************
**
** Returns regular expression to test for RNA bases with gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharRnaGap(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegRnaGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharNucRna,
		    seqCharGap);
	seqtypeRegRnaGap = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegRnaGap;
}




/* @funcstatic seqTypeCharProt ************************************************
**
** Returns regular expression to test for protein residues
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProt(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegProt)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig);
	seqtypeRegProt = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegProt;
}




/* @funcstatic seqTypeCharProtAny *********************************************
**
** Returns regular expression to test for protein residues or gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtAny(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegProtAny)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop,
		    seqCharPhylo,
		    seqCharGap);
	seqtypeRegProtAny = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegProtAny;
}




/* @funcstatic seqTypeCharProtGap *********************************************
**
** Returns regular expression to test for protein residues or gaps
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtGap(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegProtGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharGap);
	seqtypeRegProtGap = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegProtGap;
}




/* @funcstatic seqTypeCharProtGapPhylo ****************************************
**
** Returns regular expression to test for protein residues or gaps
** stops and queries
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtGapPhylo(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegProtGap)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtStop,
		    seqCharProtAmbig,
		    seqCharPhylo,
		    seqCharGap);
	seqtypeRegProtGap = ajRegComp(regstr);
	ajStrDel(&regstr);
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

static AjPRegexp seqTypeCharProtPure(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegProtPure)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s]+)",
		    seqCharProtPure);
	seqtypeRegProtPure = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegProtPure;
}




/* @funcstatic seqTypeCharProtStop ********************************************
**
** Returns regular expression to test for protein residues or stop codons
**
** @return [AjPRegexp] valid characters
******************************************************************************/

static AjPRegexp seqTypeCharProtStop(void)
{
    AjPStr regstr = NULL;

    if(!seqtypeRegProtStop)
    {
	regstr = ajStrNewL(256);
	ajFmtPrintS(&regstr, "([^%s%s%s]+)",
		    seqCharProtPure,
		    seqCharProtAmbig,
		    seqCharProtStop);
	seqtypeRegProtStop = ajRegComp(regstr);
	ajStrDel(&regstr);
    }

    return seqtypeRegProtStop;
}




/* @funcstatic seqFindType ****************************************************
**
** Returns sequence type index and ajTrue if type was found
**
** @param [r] type_name [const AjPStr] Sequence type
** @param [w] typenum [ajint*] Sequence type index
** @return [AjBool] ajTrue if sequence type was found
**
******************************************************************************/

static AjBool seqFindType(const AjPStr type_name, ajint* typenum)
{
    ajint i;
    ajint itype = -1;

    for(i = 0; seqType[i].Name; i++)
	if(ajStrMatchCaseC(type_name, seqType[i].Name))
	{
	    itype = i;
	    break;
	}

    if(itype <0)
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
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be protein
**
******************************************************************************/

AjBool ajSeqTypeIsProt(const AjPStr type_name)
{
    ajint itype;
    if(seqFindType(type_name, &itype))
	switch(seqType[itype].Type)
	{
	case ISNUC:
	    return ajFalse;
	default:
	    return ajTrue;
	}

    return ajFalse;
}




/* @func ajSeqTypeIsNuc *******************************************************
**
** Returns ajTrue is sequence type can be a nucleotide (or 'any')
**
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be nucleotide
**
******************************************************************************/

AjBool ajSeqTypeIsNuc(const AjPStr type_name)
{
    ajint itype;

    if(seqFindType(type_name, &itype))
	switch(seqType[itype].Type)
	{
	case ISPROT:
	    return ajFalse;
	default:
	    return ajTrue;
	}

    return ajFalse;
}




/* @func ajSeqTypeIsAny *******************************************************
**
** Returns ajTrue is sequence type can be a protein or nucleotide
**
** @param [r] type_name [const AjPStr] Sequence type
** @return [AjBool] ajTrue if sequence can be protein or nucleotide
**
******************************************************************************/

AjBool ajSeqTypeIsAny(const AjPStr type_name)
{
    ajint itype;

    if(seqFindType(type_name, &itype))
	switch(seqType[itype].Type)
	{
	case ISNUC:
	    return ajFalse;
	case ISPROT:
	    return ajFalse;
	default:
	    return ajTrue;
	}

    return ajFalse;
}

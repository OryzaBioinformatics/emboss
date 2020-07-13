/********************************************************************
** @source pestfind application
** Finds PEST motifs as potential proteolytic cleavage sites.
** @author Copyright (C) Michael K. Schuster
** Department of Medical Biochemistry University Vienna
** @author Copyright (C) Martin Grabner 
** (martin.grabner@univie.ac.at) EMBnet Austria
** @version 1.0
** @@
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
********************************************************************/

#include "emboss.h"

#define PSTPOT 1
#define PSTWEA 2
#define PSTINV 3

/* @data PestfindPData ************************************************
** Pestfind data object.
** Holds results for application pestfind.
** PestfindPData is implemented as a pointer to a C data structure.
** @alias PestfindSData
** @alias PestfindOData
** @@
********************************************************************/
typedef struct PestfindSData
{
    /* Type = 1 for potential, 2 for poor and 3 for invalid motifs */
    ajint Type;
    ajint Begin;		    /* Start position of PEST motif */
    ajint End;			    /* End position of PEST motif */
    ajint Length;		    /* Length of PEST motif */
    double Pscore;		    /* PEST score */
    double Hydind;		    /* Hydrophobicity index */
    double Pstpct;		    /* Mass percent (w/w) of DEPST */
} PestfindOData, *PestfindPData;

/* @macro ajStrIterIsBegin ******************************************
** Start point condition for a string iterator.
** @param [r] iter [AjIStr] String iterator.
** @return [AjBool] returns true for the first iteration
** and false for all subsequent.
** @@
** This macro tests for the first iteration which means, that 
** it tests whether the string iterator is at its start point.
********************************************************************/
#define ajStrIterIsBegin(iter) (iter->Ptr == iter->Start)


/* @macro ajStrIterIsEnd ********************************************
** End point condition for a string iterator.
** @param [r] iter [AjIStr] String iterator.
** @return [AjBool] returns AjTrue for the last iteration
** and AjFalse for all preceeding.
** @@
** This macro tests for the last iteration which means, that 
** it tests whether the string iterator is at its end point.
********************************************************************/
#define ajStrIterIsEnd(iter) (iter->Ptr == iter->End)

/* @macro ajStrIterPos **********************************************
** Position of a string iterator.
** @param [r] iter [AjIStr] String iterator.
** @return [ajint] returns the current position of a string iterator 
** within the target string.
** @@
** This macro returns an integer value of an iterator's current
** position within a string object.
** Useful for (sub-)string assignments according to iterator positions.
********************************************************************/
#define ajStrIterPos(iter) (iter->Ptr - iter->Start)

/* @macro ajStrAssSubItrBeg *****************************************
** Assigns a substring from begin to current point of iteration.
** @param [w] substr [AjPStr] Target string.
** @param [r] str [AjPStr] Source string.
** @param [r] iter [AjIStr] String iterator.
** @return [AjBool] ajTrue if string was reallocated. 
** @@
** This macro returns a pointer to a substring assigned from the
** begin of a target string to the curent point of interation.
********************************************************************/
#define ajStrAssSubItrBeg(substr, str, iter) \
	ajStrAssSub (&substr, str, 0, (iter->Ptr - iter->Start))

/* @macro ajStrAssSubItrEnd *****************************************
** Assigns substring from the current point of iteration until the end.
** @param [w] substr [AjPStr] Target string.
** @param [r] str [AjPStr] Source string.
** @param [r] iter [AjIStr] String iterator.
** @return [AjBool] ajTrue if string was reallocated. 
** @@
** This macro returns a pointer to a substring assigned from the
** current point of iteration to the end of a target string.
********************************************************************/
#define ajStrAssSubItrEnd(substr, str, iter) \
	ajStrAssSub (&substr, str, (iter->Ptr - iter->Start), (iter->End - iter->Start))

/* @funcstatic ajStrIterPosCopy *************************************
** Copies the current position of iteration between two
** string iterators over the same source string object.
** @param [r] itrbeg [const AjIStr] First string iterator.
** @param [w] itrend [AjIStr] Second string iterator.
** @return [ajint] current position of both iterators.
** @@
** This function returns an integer representing the current position 
** of iteration within a target string object.
********************************************************************/
static ajint ajStrIterPosCopy (const AjIStr itrbeg, AjIStr itrend)
{
    if (itrbeg->Start != itrend->Start)
    {
	ajFatal ("ajStrIterCopyPos: Iterators (AjIStr objects) belong"
		 " to different AjPStr objects!\n");
    }

    return (itrend->Ptr - itrend->Start);
}



/* @funcstatic ajStrAssSubItrItr ************************************
** Assigns a substring from two string iterators over the same 
** source string object.
** @param [w] substr [AjPStr] Target string.
** @param [r] str [const AjPStr] Source string.
** @param [r] itrbeg [const AjIStr] First string iterator.
** @param [r] itrend [const AjIStr] Second string iterator.
** @return [AjBool] ajTrue if string was reallocated.
** @@
** This function assigns a substring between two points of 
** string iteration within the same target string.
********************************************************************/
/*
static AjBool ajStrAssSubItrItr (AjPStr substr, const AjPStr str,
				 const AjIStr itrbeg, const AjIStr itrend)
{
    if ((itrbeg->Ptr - itrbeg->Start) <= (itrend->Ptr - itrend->Start))
	return ajStrAssSub (&substr, str, (itrbeg->Ptr - itrbeg->Start),
			    (itrend->Ptr - itrend->Start));

    return ajStrAssSub (&substr, str, (itrend->Ptr - itrend->Start),
			(itrbeg->Ptr - itrbeg->Start));
}
*/



/* @funcstatic pestfind_compare_position ****************************
** Helper function to sort the list of pestfind results by position.
** @param [rP] one [const void*] Pointer to first list element.
** @param [rP] two [const void*] Pointer to second list element.
** @return [ajint] returns -ve if one sorts before two,
** 0 if one is equal two and +ve if two sorts before one.
** @@
** This private pestfind function compares PEST results
** according to their start position. 
*******************************************************************/
static ajint pestfind_compare_position (const void *one, const void *two)
{
    return (*(PestfindPData*)one)->Begin - (*(PestfindPData*)two)->Begin;
}



/* @funcstatic pestfind_compare_length ******************************
** Helper function to sort the list of pestfind results by length.
** @param [rP] one [const void*] Pointer to first list element.
** @param [rP] two [const void*] Pointer to second list element.
** @return [ajint] returns -ve if one sorts before two,
** 0 if one is equal two and +ve if two sorts before one.
** @@
** This private pestfind function compares PEST results
** according to their length. 
*******************************************************************/
static ajint pestfind_compare_length (const void *one, const void *two)
{
    return (*(PestfindPData*)two)->Length - (*(PestfindPData*)one)->Length;
}




/* @funcstatic pestfind_compare_score *******************************
** Helper function to sort the list of pestfind results by score.
** @param [rP] one [const void*] Pointer to first list element.
** @param [rP] two [const void*] Pointer to second list element.
** @return [ajint] returns -ve if one sorts before two,
** 0 if one is equal two and +ve if two sorts before one.
** @@
** This private pestfind function compares PEST results
** according to their PEST score. Since a score is not
** defined for invalid PEST motifs those are sorted 
** at the end of the list according to their position.
*******************************************************************/
static ajint pestfind_compare_score (const void *one, const void *two)
{
    /* one = potential or weak PEST motif */
    if ((*(PestfindPData*)one)->Type == PSTPOT ||
	(*(PestfindPData*)one)->Type == PSTWEA)
    {
	/* two = potential or weak PEST motif */
	if ((*(PestfindPData*)two)->Type == PSTPOT ||
	    (*(PestfindPData*)two)->Type == PSTWEA)
	    return (ajint) (((*(PestfindPData*)two)->Pscore) * 1000) - 
		(ajint) (((*(PestfindPData*)one)->Pscore) * 1000);

	/* two = invalid PEST motif */
	if ((*(PestfindPData*)two)->Type == PSTINV)
	    return -1;
    }


    /* Remaining invalid PEST motifs are sorted by position */
    /* one = invalid PEST motif */
    if ((*(PestfindPData*)one)->Type == PSTINV)
    {
	/* two = potential or weak PEST motif */
	if ((*(PestfindPData*)two)->Type == PSTPOT ||
	    (*(PestfindPData*)two)->Type == PSTWEA)
	    return +1;

	/* two = invalid PEST motif */
	if ((*(PestfindPData*)two)->Type == PSTINV)
	    return (*(PestfindPData*)one)->Begin - 
		(*(PestfindPData*)two)->Begin;
    }

    return 0;
}

#ifdef PLD_png

extern int PNGWidth;
extern int PNGHeight;

#endif

/* @prog pestfind ***************************************************
** Finds PEST motifs as potential proteolytic cleavage sites.
********************************************************************/
int main (int argc, char **argv)
{
    AjPFile outf   = NULL;     /* output file */
    AjPFile mfptr  = NULL;     /* data file */
    AjPList reslst = NULL;     /* result list */
    AjPSeq  seq    = NULL;     /* sequence */
    AjPStr  map    = NULL;     /* string for motif map */
    AjPStr  str    = NULL;     /* sequence string */
    AjPStr  substr = NULL;     /* sequence sub-string */
    AjPStr  datafn = NULL;     /* data filename */
    AjPStr  sorder = NULL;     /* sort order */
    AjIList itrlst = NULL;     /* iterator list */
    AjIStr  itrbeg = NULL;     /* iterator begin of motif */
    AjIStr  itrend = NULL;     /* iterator end of motif */
    AjBool  dsppot = AJTRUE;   /* display potential mofifs */
    AjBool  dspwea = AJTRUE;   /* display weak motifs */
    AjBool  dspinv = AJTRUE;   /* display invalid motifs */
    AjBool  dspmap = AJTRUE;   /* display map of motifs */
    AjPGraph graph = NULL;     /* graphics object */
    AjPGraphData plot = NULL;  /* sub set of graphics object */
    
    PestfindPData pstdat = NULL; /* PEST find data object */
    /********************************************************************
     ** Array of linear transformed Kyte-Doolittle hydropathy indices (ltkdhi)
     ** in alphabetical order A-M and N-Z as well as N-terminus and C-terminus.
     ** Linear transformation was ltkdhi = 10 * kdhi + 45
     ** Values range from Argine R = 0 to Isoleucine I = 90
     ** B=(N|D)=10 since N=10 and D=10
     ** Z=(Q|E)=10 since Q=10 and E=10
     ** X=10*0+45=45
     ********************************************************************/
    const ajint ltkdhi[28] =
    {
	63, 10, 70, 10, 10, 72, 41, 13, 90,  0,  6, 82, 64,
	10,  0, 29, 10,  0, 36, 38,  0, 87, 36, 45, 58, 10,
	0,  0
    };
    
    ajint *aac;           /* array of pointers to amino acid counts (aac) from
			     function embIepComp() */
    ajint i     = 0;      /* general iterator i */
    ajint begin = 0;      /* sequence begin */
    ajint end   = 0;      /* sequence end */
    ajint win   = 0;      /* window size */
    ajint cnt   = 0;      /* number of amino acids between positively charged
			     ones */
    ajint seqlen  = 0;    /* sequence length */
    ajint sublen  = 0;    /* sub-sequence lenght */
    ajint posbeg  = 0;    /* position of outer iteration -> begin */
    ajint posend  = 0;    /* position of inner iteration -> end */
    double pstsum = 0;    /* sum over equivalents DEPST minus one equivalent
			     EPT */
    double hydind = 0;    /* hydrophobicity index */
    double momass = 0;    /* molecular mass */
    double pstpct = 0;    /* corrected mass% of DEPST minus one equivalent
			     EPT */
    double pscore = 0;    /* PEST-find score */
    double trshld = 0.0;  /* PEST-find score threshold value to discriminate
			     poor and potential */
    char   symbol = '\0'; /* single character as result of string iterations */
    float  ymax   = +60.0;/* Maximum PEST-find score (graphics y-axis) */
    float  ymin   = -60.0;/* Minimum PEST-find score (graphics y-axis) */
    
#ifdef PLD_png
    
    /*
       PNGWidth = 1280;
       PNGHeight = 960;
       */
    
    PNGWidth = 960;
    PNGHeight = 960;
    
#endif

    (void) ajGraphInit ("pestfind", argc, argv);
    
    seq    = ajAcdGetSeq ("sequence");
    win    = ajAcdGetInt ("window");
    dsppot = ajAcdGetBool ("potential");
    dspwea = ajAcdGetBool ("poor");
    dspinv = ajAcdGetBool ("invalid");
    dspmap = ajAcdGetBool ("map");
    trshld = ajAcdGetFloat ("threshold");
    outf   = ajAcdGetOutfile ("outfile");
    datafn = ajAcdGetString ("aadata");
    sorder = ajAcdGetSelectI ("order", 1);
    graph  = ajAcdGetGraphxy ("graph");
    begin  = ajSeqBegin (seq);
    end    = ajSeqEnd (seq);
    map    = ajStrNew ();
    str    = ajStrNew ();
    substr = ajStrNew ();
    reslst = ajListNew ();
    
    ajFileDataNew (datafn, &mfptr); /* Open the amino acid datafile. */
    if (!mfptr)
	ajFatal ("Amino acid datafile %S not found\n", datafn);
    
    embPropAminoRead (mfptr);
    ajFileClose (&mfptr); /* Close the amino acid datafile. */
    
    AJCNEW0(aac, EMBIEPSIZE);
    
    (void) ajStrAssSubC (&str, ajSeqChar (seq), (begin - 1), (end - 1));
    (void) ajStrToUpper (&str);
    seqlen = ajStrLen (str);
    itrbeg = ajStrIter (str);
    itrend = ajStrIter (str);
    while (!ajStrIterDone (itrbeg))
    {
	symbol = ajStrIterGetK (itrbeg);
	if (
	    ajStrIterIsBegin(itrbeg) ||
	    symbol == 'R' ||
	    symbol == 'H' ||
	    symbol == 'K'
	    )
	{
	    cnt = 0;
	    (void) ajStrIterPosCopy (itrbeg, itrend);
	    (void) ajStrIterNext (itrend);
	    while (!ajStrIterDone (itrend))
	    {
		symbol = ajStrIterGetK (itrend);
		if (
		    symbol == 'R' ||
		    symbol == 'H' ||
		    symbol == 'K' ||
		    ajStrIterIsEnd(itrend)
		    )
		{
		    /*
		     * PEST sequences smaller than the window size are
		     * rejected
		     */
		    if (cnt < win)
			break;
		    posbeg = ajStrIterPos (itrbeg);
		    posend = ajStrIterPos (itrend);
		    (void) ajStrAssSub (&substr, str, (posbeg + 1), (posend - 1));
		    sublen = ajStrLen (substr);
		    embIepComp (ajStrStr (substr), 1, aac);
		    /* Valid PEST motifs must contain D or E, P and S or T. */
		    if (
			aac[ajAZToInt ('D')] + aac[ajAZToInt ('E')] == 0 ||
			aac[ajAZToInt ('P')] == 0 ||
			aac[ajAZToInt ('S')] + aac[ajAZToInt ('T')] == 0
			)
		    {
			/* invalid PEST motifs */
			if (dspinv)
			{
			    AJNEW0(pstdat);
			    /* Type is PSTINV (3) for invalid PEST motifs */
			    (pstdat->Type)   = PSTINV;
			    (pstdat->Begin)  = posbeg;
			    (pstdat->End)    = posend;
			    (pstdat->Length) = sublen;
			    ajListPushApp (reslst, (PestfindPData) pstdat);
			}
			break;
		    }
		    momass  = embPropCalcMolwt (ajStrStr (substr), 0,
						(sublen - 1));
		    pstsum  = 0;
		    pstsum += EmbPropTable[ajAZToInt ('D')][EMBPROPMOLWT] *
			aac[ajAZToInt ('D')];
		    pstsum += EmbPropTable[ajAZToInt ('E')][EMBPROPMOLWT] *
			aac[ajAZToInt ('E')];
		    pstsum += EmbPropTable[ajAZToInt ('P')][EMBPROPMOLWT] *
			aac[ajAZToInt ('P')];
		    pstsum += EmbPropTable[ajAZToInt ('S')][EMBPROPMOLWT] *
			aac[ajAZToInt ('S')];
		    pstsum += EmbPropTable[ajAZToInt ('T')][EMBPROPMOLWT] *
			aac[ajAZToInt ('T')];
		    pstsum -= EmbPropTable[ajAZToInt ('E')][EMBPROPMOLWT];
		    pstsum -= EmbPropTable[ajAZToInt ('P')][EMBPROPMOLWT];
		    pstsum -= EmbPropTable[ajAZToInt ('T')][EMBPROPMOLWT];
		    pstpct  = pstsum / momass * 100;
		    hydind  = 0;
		    for (i = 0; i < 26; i++)
			hydind += EmbPropTable[i][EMBPROPMOLWT] *
			    (double) aac[i] * (double) ltkdhi[i] / momass;

		    pscore = 0.55 * pstpct - 0.5 * hydind;
		    /* valid PEST motifs */
		    if (pscore >= trshld)
		    {
			/* potential PEST motifs */
			if (dsppot)
			{
			    AJNEW0(pstdat);
			    /* Type is PSTPOT (1) for potential PEST motifs */
			    (pstdat->Type)   = PSTPOT;
			    (pstdat->Begin)  = posbeg;
			    (pstdat->End)    = posend;
			    (pstdat->Length) = sublen;
			    (pstdat->Pscore) = pscore;
			    (pstdat->Hydind) = hydind;
			    (pstdat->Pstpct) = pstpct;
			    ajListPushApp (reslst, (PestfindPData) pstdat);
			}
			break;
		    }
		    else
		    {
			/* poor PEST motifs */
			if (dspwea)
			{
			    AJNEW0(pstdat);
			    /* Type is PSTWEA (2) for poor PEST motifs */
			    (pstdat->Type)   = PSTWEA;
			    (pstdat->Begin)  = posbeg;
			    (pstdat->End)    = posend;
			    (pstdat->Length) = sublen;
			    (pstdat->Pscore) = pscore;
			    (pstdat->Hydind) = hydind;
			    (pstdat->Pstpct) = pstpct;
			    ajListPushApp (reslst, (PestfindPData) pstdat);
			}
			break;
		    }
		}
		else
		{
		    (void) ajStrIterNext (itrend);
		    cnt++;
		}
	    }
	    (void) ajStrIterNext (itrbeg);
	}
	else
	    (void) ajStrIterNext (itrbeg);
    }

    /* Sort list according to qualifier "order". */
    if (ajStrMatchC (sorder, "length"))
	ajListSort (reslst, pestfind_compare_length);
    
    if (ajStrMatchC (sorder, "position"))
	ajListSort (reslst, pestfind_compare_position);
    
    if (ajStrMatchC (sorder, "score"))
	ajListSort (reslst, pestfind_compare_score);
    
    /* Formatted list of results. */
    ajFmtPrintF (outf,
  "PEST-find: Finds PEST motifs as potential proteolytic cleavage sites.\n\n");
    
    if (ajListLength (reslst) == 0)
	ajFmtPrintF (outf,
		"    No PEST motif was identified in %s from %d to %d.\n\n",
		ajSeqName (seq), begin, end);

    
    if (ajListLength (reslst) == 1)
	ajFmtPrintF (outf,
		  "     1 PEST motif was identified in %s from %d to %d.\n\n",
		  ajSeqName (seq), begin, end);

    
    if (ajListLength (reslst) > 1)
    {
	ajFmtPrintF (outf, "%6d PEST motifs were identified in %s\n",
		     ajListLength (reslst), ajSeqName (seq));
	ajFmtPrintF (outf,
	      "       from positions %d to %d and sorted by %S.\n\n", begin,
	      end, sorder);
    }
    itrlst = ajListIter (reslst);
    while (ajListIterMore (itrlst))
    {
	pstdat = (PestfindPData) ajListIterNext (itrlst);

	if ((pstdat->Type) == PSTPOT)
	    ajFmtPrintF (outf, "Potential ");
	if ((pstdat->Type) == PSTWEA)
	    ajFmtPrintF (outf, "Poor ");
	if ((pstdat->Type) == PSTINV)
	    ajFmtPrintF (outf, "Invalid ");

	ajFmtPrintF (outf,
	       "PEST motif with %d amino acids between position %d and %d.\n",
		(pstdat->Length), ((pstdat->Begin) + begin),
		((pstdat->End) + begin));

	(void) ajStrAssSub (&substr, str, (pstdat->Begin), (pstdat->End));
	i = 1;				/* line counter */
	while ((i * 60) <= (pstdat->Length))
	{
	    /* 12 characters per line inserted */
	    (void) ajStrInsertC (&substr, (i*60+(i-1)*12), " ...\n   ... ");
	    i++;
	}
	ajFmtPrintF (outf, "%6d %S %d\n", ((pstdat->Begin) + begin), substr,
		     ((pstdat->End) + begin));
	if ((pstdat->Type) == PSTPOT)
	{
	    ajFmtPrintF (outf, "       DEPST: %.2f %% (w/w)\n",
			 (pstdat->Pstpct));
	    ajFmtPrintF (outf, "       Hydrophobicity index: %.2f\n",
			 (pstdat->Hydind));
	    ajFmtPrintF (outf, "       PEST score: %.2f  \n\n",
			 (pstdat->Pscore));
	}
	if ((pstdat->Type) == PSTWEA)
	    ajFmtPrintF (outf, "       PEST score: %.2f  \n\n",
			 (pstdat->Pscore));

	if ((pstdat->Type) == PSTINV)
	    ajFmtPrintF (outf, "\n");
    }
    
    /* Display map. */
    if (dspmap)
    {
	ajFmtPrintF (outf, "\n       ");
	ajFmtPrintF (outf, "---------+---------+");
	ajFmtPrintF (outf, "---------+---------+");
	ajFmtPrintF (outf, "---------+---------+");
	ajFmtPrintF (outf, "\n\n");
	ajListSort (reslst, pestfind_compare_position);
	itrlst = ajListIter (reslst);
	i = 0;
	while (i <= seqlen)
	{
	    while (ajListIterMore (itrlst))
	    {
		pstdat = (PestfindPData) ajListIterNext (itrlst);
		while (i <= (pstdat->Begin))
		{
		    (void) ajStrAppK (&map, ' ');
		    i++;
		}
		while (i <= ((pstdat->End) - 1))
		{
		    if ((pstdat->Type) == PSTPOT)
			(void) ajStrAppK (&map, '+');
		    if ((pstdat->Type) == PSTWEA)
			(void) ajStrAppK (&map, 'O');
		    if ((pstdat->Type) == PSTINV)
			(void) ajStrAppK (&map, '-');
		    i++;
		}
	    }
	    (void) ajStrAppK (&map, ' ');
	    i++;
	}

	/* print sequence and map lines */
	i = 1;				/* line counter */
	while ((i * 60) <= seqlen)
	{
	    (void) ajStrAssSub (&substr, str, ((i - 1) * 60), (i * 60 - 1));
	    ajFmtPrintF (outf, "%6d %S %d\n", ((i - 1) * 60 + begin), substr,
			 (i * 60 - 1 + begin));
	    (void) ajStrAssSub (&substr, map, ((i - 1) * 60), (i * 60 - 1));
	    ajFmtPrintF (outf, "       %S\n\n", substr);
	    i++;
	}

	if (((i - 1) * 60) < seqlen)
	{
	    (void) ajStrAssSub (&substr, str, ((i - 1) * 60), (seqlen - 1));
	    ajFmtPrintF (outf, "%6d %S %d\n", ((i - 1) * 60 + begin), substr,
			 (seqlen - 1 + begin));
	    (void) ajStrAssSub (&substr, map, ((i - 1) * 60), (seqlen - 1));
	    ajFmtPrintF (outf, "       %S\n\n", substr);
	}

	/* print legend */
	if (dsppot || dspwea || dspinv)
	    ajFmtPrintF (outf, "       Symbols   PEST motifs\n");
	if (dsppot)
	    ajFmtPrintF (outf, "       +++++++   potential\n");
	if (dspwea)
	    ajFmtPrintF (outf, "       OOOOOOO   poor     \n");
	if (dspinv)
	    ajFmtPrintF (outf, "       -------   invalid  \n");
	if (dsppot || dspwea || dspinv)
	    ajFmtPrintF (outf, "\n");
    }
    ajFileClose (&outf);     /* Close the output file. */
    
    /* Display graphics. */
    plot = ajGraphxyDataNewI (seqlen);
    ajGraphxySetOverLap (graph, ajFalse);
    ajGraphDataxySetTypeC (plot, "2D Plot");
    ajGraphxyDataSetTitleC (plot, "PEST-find");
    (void) ajFmtPrintS (&map, "Sequence %s from %d to %d", ajSeqName (seq),
			begin, end);
    ajGraphxyDataSetXtitle (plot, map);
    ajGraphxyDataSetYtitleC (plot, "PEST score");
    ajGraphDataxySetMaxMin (plot, (float) 1, (float) seqlen, ymin, ymax);
    ajGraphDataxySetMaxima (plot, (float) 1, (float) seqlen, ymin, ymax);
    /* threshold line */
    ajGraphDataObjAddLine (plot, (float) 0, (float) trshld, (float) seqlen,
			   (float) trshld, AQUAMARINE);
    (void) ajFmtPrintS (&map, "threshold %+.2f", trshld);
    ajGraphDataObjAddText (plot, (float) 0 + 2, (float) trshld + 2,
			   AQUAMARINE, ajStrStr(map));
    ajListSort (reslst, pestfind_compare_position);
    itrlst = ajListIter (reslst);
    while (ajListIterMore (itrlst))
    {
	pstdat = (PestfindPData) ajListIterNext (itrlst);
	if ((pstdat->Type) == PSTPOT)
	{
	    ajGraphDataObjAddLine (plot, (float) (pstdat->Begin),
				   (float) (pstdat->Pscore),
				   (float) (pstdat->End),
				   (float) (pstdat->Pscore), GREEN);
	    (void) ajFmtPrintS (&map, "%+.2f", (pstdat->Pscore));
	    ajGraphDataObjAddText (plot, (float) (pstdat->Begin) + 2,
				   (float) (pstdat->Pscore) + 2, GREEN,
				   ajStrStr(map));
	}
	if ((pstdat->Type) == PSTWEA)
	{
	    ajGraphDataObjAddLine (plot, (float) (pstdat->Begin),
				   (float) (pstdat->Pscore),
				   (float) (pstdat->End),
				   (float) (pstdat->Pscore), RED);
	    (void) ajFmtPrintS (&map, "%+.2f", (pstdat->Pscore));
	    ajGraphDataObjAddText (plot, (float) (pstdat->Begin) + 2,
				   (float) (pstdat->Pscore) + 2, RED,
				   ajStrStr(map));
	}
	if ((pstdat->Type) == PSTINV)
	{
	    ajGraphDataObjAddLine (plot, (float) (pstdat->Begin),
				   (float) (pstdat->Pscore),
				   (float) (pstdat->End),
				   (float) (pstdat->Pscore), BROWN);
	    ajGraphDataObjAddText (plot, (float) (pstdat->Begin) + 2,
				   (float) (pstdat->Pscore) + 2, BROWN,
				   "inv.");
	}
    }

    (void) ajGraphxyAddGraph (graph, plot);
    ajGraphSetCharSize (0.50);
    ajGraphxyTitleC (graph, "PEST-find");
    ajGraphxyDisplay (graph, AJTRUE);
    ajGraphCloseWin ();
    ajGraphxyDel (graph);
    
    /* clean-up and destruction */
    itrlst = ajListIter (reslst);
    while (ajListIterMore (itrlst))
    {
	pstdat = (PestfindPData) ajListIterNext (itrlst);
	AJFREE (pstdat);
    }
    
    AJFREE (aac);
    /* Delete the list of PEST data and all objects within. */
    ajListFree (&reslst);
    ajStrDel (&map);         /* Delete the map string. */
    ajStrDel (&str);         /* Delete the sequence string. */
    ajStrDel (&substr);      /* Delete the sequence sub-string. */
    ajStrDel (&datafn);      /* Delete the data file name string. */
    ajStrDel (&sorder);      /* Delete the sort order string. */
    ajListIterFree (itrlst); /* Delete the result list iterator. */
    ajStrIterFree (&itrbeg); /* Delete the iterator of the outer loop. */
    ajStrIterFree (&itrend); /* Delete the iterator of the inner loop. */
    
    ajExit ();
    return 0;
}

/* undefine macros to avoid clashes with Ajax library functions */
#undef ajStrIterIsBegin
#undef ajStrIterIsEnd
#undef ajStrIterPos
#undef ajStrAssSubItrBeg
#undef ajStrAssSubItrEnd
#undef PSTPOT
#undef PSTWEA
#undef PSTINV

/********************************************************************
** usage:
** pestfind [-window=10] [-order="length"|"position"|"score"]
** [-outfile=sequence.pestfind] [-aadata=Eamino.dat] [-threshold=5.0]
** [-[no]potential] [-[no]poor] [-[no]invalid] [-[no]map] sequence
********************************************************************/

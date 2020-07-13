/* @source seqwords application
**
** Generate file of hits for scop families by searching swissprot with
** keywords.
**
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
** @author: Copyright (C) Matt Blades (mblades@hgmp.mrc.ac.uk)
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
*******************************************************************************
**
**
**
**
**
*******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
*******************************************************************************
**
** Mon May 20 11:43:39 BST 2002
**
** The following documentation is out-of-date and should be disregarded.  It
** will be updated shortly.
**
*******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
*******************************************************************************
**
**
** Operation
**
** seqwords parses a file of keywords and the swissprot database and writes
** a file of sequences whose swissprot entries contains at least one of the
** keywords.  If an entry contains a keyword in a domain record of the feature
** table, then the sequence of the domain is written to the output file,
** otherwise the entire sequence is written.
**
**
** The keywords file (Figure 1) contains lists of keywords specific to a
** number of SCOP families and superfamilies.  Each list of keywords is given
** after a block of SCOP classification records; for family-specific search
** terms, the block must contain a CL, FO, SF and an FA record (see below).
** For superfamily-specific terms, clearly only the CL, FO and SF should be
** specified. A single keyword must be given per line after the record 'TE'.
** Each block of SCOP classification records and search terms must be
** delimited by the record '//' (the file should end with this record).
** It is possible to provide fold and class-specific search terms by using the
** CL and FO records only as appropriate, however, text searches of swissprot
** for members of scop folds and classes are unlikely to produce meaningful
** results.
**
**
** The format of the output file (Figure 2) is the same as the hits file
** written by the EMBOSS application psiblasts.  The file uses the following
** records:
** (1) DE - bibliographic information. The text 'Results of swissprot search'
** is always given.
** Four SCOP classification records may be given:
** (2)  CL - Domain class.  It is identical to the text given after 'Class' in
** the scop classification file (see documentation for the EMBOSS application
** scope).
** (3)  FO - Domain fold.  It is identical to the text given after 'Fold' in
** the scop classification file (see scope documentation).
** (4)  SF - Domain superfamily.  It is identical to the text given after
** 'Superfamily' in the scop classification file (see scope documentation).
** (5)  FA - Domain family. It is identical to the text given after 'Family'
** in the scop classification file (see scope documentation).
** (6)  NS - Number in set. The number of sequences retrieved by the search
** for this family or superfamily. The file will have a section containing an
** NN, AC, CL, RA and SQ records (see below) for each sequence in the set for
** each family / superfamily.
** (7) NN - Sequence number.  The number given in brackets after this record
** indicates the start of the data for the relevent sequence in the current
** set.
** (8) AC - Accession number of the hit.
** (9) TY - Classification of hit.  Always has the value 'OTHER' (the values
** SEED or HIT) are used for psiblasts output (see psiblasts documentation).
** (10) RA - Sequence range. The numbers before START and END give the start
** and end positions respectively of the domain relative to the full length
** sequence in the swissprot database.  A '1' and the length (amino acids) of
** the sequence are given in cases where an entire protein sequence is given.
** (11) SQ - protein sequence. The number of residues is given before AA on the
** first line. The protein sequence is given on subsequent lines.
** (12) XX - used for spacing.
** (13) // - used to delimit data for search of swissprot.
**
**
**
** Figure 1  Excerpt from seqwords input file
**
**  CL   All beta proteins
**  XX
**  FO   Lipocalins
**  XX
**  SF   Lipocalins
**  XX
**  TE   Lipocalin
**  TE   Calycin
**  //
**  CL   All beta proteins
**  XX
**  FO   Lipocalins
**  XX
**  SF   Lipocalins
**  XX
**  FA   Retinol binding protein-like
**  XX
**  TE   RBP
**  TE   Retinol binding
**  TE   Retinol-binding
**  //
**  CL   All beta proteins
**  XX
**  FO   Lipocalins
**  XX
**  SF   Lipocalins
**  XX
**  FA   Fatty acid binding protein-like
**  XX
**  TE   FABP
**  TE   Fatty acid binding
**  TE   Fatty acid-binding
**  //
**
**
**
**
** Figure 2  Excerpt from seqwords output file
**
**  DE   Members of scop families
**  XX
**  CL   All alpha proteins
**  XX
**  FO   Globin-like
**  XX
**  SF   Globin-like
**  XX
**  FA   Globins
**  XX
**  NS   2
**  XX
**  NN   [1]
**  XX
**  AC   P67983
**  XX
**  TY   OTHER
**  XX
**  RA   2 START; 79 END;
**  XX
**  SQ   SEQUENCE   141 AA;  15127 MW;  5EC7DB1E CRC32;
**       VLSPADKTNV KAAWGKVGAH AGEYGAEALE RMFLSFPTTK TYFPHFDLSH GSAQVKGHGK
**       KVADALTNAV AHVDDMPNAL SALSDLHAHK LRVDPVNFKL LSHCLLVTLA AHLPAEFTPA
**       VHASLDKFLA SVSTVLTSKY R
**  XX
**  NN   [2]
**  XX
**  AC   P673383
**  XX
**  TY   OTHER
**  XX
**  RA   2 START; 79 END;
**  XX
**  SQ   SEQUENCE   141 AA;  15127 MW;  5EC7DB1E CRC32;
**       VLSPADKTNV KAAWGKVGAH AGEYGAEALE RMFLSFPTTK TYFPHFDLSH GSAQVKGHGK
**       KVADALTNAV AHVDDMPNAL SALSDLHAHK LRVDPVNFKL LSHCLLVTLA AHLPAEFTPA
**       VHASLDKFLA SVSTVLTSKY R
**  XX
**  //
**  CL   All alpha proteins
**  XX
**  FO   Globin-like
**  XX
**  SF   Globin-like
**  XX
**  FA   Phycocyanins
**  XX
**
**
** Notes
** This is slow - swissprot is read multiple times (once for each list of
** terms).  Changing it to do a single file read would require modifying
** keysearch to take an array of hitlist and terms structures.
**
******************************************************************************/








#include "emboss.h"





typedef struct AjSTerms
{
    AjPStr Class;
    AjPStr Fold;
    AjPStr Superfamily;
    AjPStr Family;
    ajint  N;				/* No. of keywords */
    AjPStr *Keywords;			/* Array of keywords */
} AjOTerms,*AjPTerms;




static AjPTerms seqwords_TermsNew(void);

static AjBool   seqwords_TermsRead(AjPFile inf, AjPTerms *thys);
static void     seqwords_TermsDel(AjPTerms *pthis);
static AjBool   seqwords_keysearch(AjPFile inf, AjPTerms terms,
				     AjPHitlist *hits);

/* @prog seqwords *************************************************************
**
** Retrieves sequences from swissprot using keyword search
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile     key_inf=NULL;		/* File pointer for keywords file */
    AjPFile     sp_inf =NULL;		/* File pointer for swissprot
                                           database */
    AjPFile     outf   =NULL;		/* File pointer for output file */
    AjPTerms    keyptr =NULL;		/* Pointer to terms structure */
    AjPHitlist  hitptr =NULL;		/* Pointer to hitlist structure */





    /* Read data from acd */
    embInit("seqwords",argc,argv);
    key_inf  = ajAcdGetInfile("keyfile");
    sp_inf  = ajAcdGetInfile("spfile");
    outf =  ajAcdGetOutfile("outfile");


    /* Print DE field to output file */
    ajFmtPrintF(outf,"DE   Members of scop families\nXX\n");


    /* Start of main application loop
       Read next list of terms from input file */
    while((seqwords_TermsRead(key_inf, &keyptr)))
    {
	/* Rewind swissprot file pointer to the top */
	ajFileSeek(sp_inf, 0, 0);


	/* Allocate memory for hitlist */
	AJNEW0(hitptr);


	/* Do search of swissprot */
	seqwords_keysearch(sp_inf, keyptr, &hitptr);


	/* Copy scop records from terms to hitlist structure */
	ajStrAss(&hitptr->Class, keyptr->Class);
	ajStrAss(&hitptr->Fold, keyptr->Fold);
	ajStrAss(&hitptr->Superfamily, keyptr->Superfamily);
	ajStrAss(&hitptr->Family, keyptr->Family);


	/* Write output file */
	ajXyzHitlistWrite(outf, hitptr);


	/* Free memory for hitlist & keyptr*/
	ajXyzHitlistDel(&hitptr);
	seqwords_TermsDel(&keyptr);
    }
    seqwords_TermsDel(&keyptr);

    /* Tidy up*/
    ajFileClose(&key_inf);
    ajFileClose(&sp_inf);
    ajFileClose(&outf);


    return 0;
}





/* @funcstatic seqwords_TermsNew **********************************************
**
** Terms object constructor. This is normally called by the TermsRead
**  function.
**
** @return [AjPTerms] Pointer to a Terms object
** @@
******************************************************************************/
static AjPTerms seqwords_TermsNew(void)
{
    AjPTerms  ret =NULL;		/* Pointer to terms structure */


    /* Create an AjSTerms object */
    AJNEW0(ret);
    ret->Class=ajStrNew();
    ret->Fold=ajStrNew();
    ret->Superfamily=ajStrNew();
    ret->Family=ajStrNew();
    ret->N=0;
    ret->Keywords=NULL;

    return ret;
}





/* @funcstatic seqwords_TermsDel **********************************************
**
** Destructor for terms object.
**
** @param [w] pthis [AjPTerms*] Terms object pointer
**
** @return [void]
** @@
******************************************************************************/
static void seqwords_TermsDel(AjPTerms *pthis)
{
    int x=0;				/* Counter */
    AjPTerms thys = *pthis;

    ajStrDel(&thys->Class);
    ajStrDel(&thys->Fold);
    ajStrDel(&thys->Superfamily);
    ajStrDel(&thys->Family);


    for(x=0;x<thys->N; x++)
	ajStrDel(&thys->Keywords[x]);
    AJFREE(thys->Keywords);

    AJFREE(thys);

    return;
}





/* @funcstatic seqwords_TermsRead *********************************************
**
** Read the next Terms object from a file in embl-like format. The search
** terms are modified with a leading and trailing space.
**
** @param [r] inf [AjPFile] Input file stream
** @param [w] thys [AjPTerms*] Terms object
**
** @return [AjBool] True on success
** @@
******************************************************************************/
static AjBool seqwords_TermsRead(AjPFile inf, AjPTerms *thys)
{
    AjPStr   line           =NULL;	/* Line of text */
    AjPStr   temp           =NULL;
    AjPList  list_terms     =NULL;	/* List of keywords for a scop node*/
    AjBool   ok             =ajFalse;


    /* Create Terms structure */
    (*thys)=seqwords_TermsNew();


    /* Create list & allocate strings */
    list_terms = ajListstrNew();
    line       = ajStrNew();


    /* Read first line */
    ok = ajFileReadLine(inf,&line);


    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssC(&(*thys)->Class,ajStrStr(line)+3);
	    ajStrClean(&(*thys)->Class);
	}
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&(*thys)->Fold,ajStrStr(line)+3);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&(*thys)->Fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&(*thys)->Fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssC(&(*thys)->Superfamily,ajStrStr(line)+3);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&(*thys)->Superfamily,ajStrStr(line)+3);
	    }
	    ajStrClean(&(*thys)->Superfamily);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssC(&(*thys)->Family,ajStrStr(line)+3);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&(*thys)->Family,ajStrStr(line)+3);
	    }
	    ajStrClean(&(*thys)->Family);
	}
	else if(ajStrPrefixC(line,"TE"))
	{
	    /* Copy and clean up term */
	    temp    = ajStrNew();
	    ajStrAssC(&temp,ajStrStr(line)+3);
	    ajStrClean(&temp);


	    /* Append a leading and trailing space to search term*/
	    ajStrAppK(&temp, ' ');
	    ajStrInsertC(&temp, 0, " ");


	    /* Add the current term to the list */
	    ajListstrPush(list_terms,temp);
	}

	ok = ajFileReadLine(inf,&line);
    }
    if(!ok)
    {
	/* Clean up */
	ajListstrFree(&list_terms);
	ajStrDel(&line);


	/* Return */
	return ajFalse;
    }


    /*Convert the AjPList of terms to array of AjPSeq's*/
    if(!((*thys)->N=ajListstrToArray((AjPList)list_terms,&(*thys)->Keywords)))
    {
	ajWarn("Zero sized list of terms passed into seqwords_TermsRead");
    }


    /* Clean up.  Free the list (not the nodes!) */
    ajListstrDel(&list_terms);
    ajStrDel(&line);

    return ajTrue;
}





/* @funcstatic seqwords_keysearch *********************************************
**
** Search swissprot with terms structure and writes a hitlist structure
**
** @param [r] inf   [AjPFile]     File pointer to swissprot database
** @param [r] terms [AjPTerms]    Terms object pointer
** @param [w] hits  [AjPHitlist*] Hitlist object pointer
**
** @return [AjBool] True on success
** @@
******************************************************************************/
static AjBool seqwords_keysearch(AjPFile inf, AjPTerms terms,
				   AjPHitlist *hits)
{
    AjPStr   line           =NULL;	/* Line of text */
    AjPStr   id             =NULL;	/* Line of text */
    AjPStr   temp           =NULL;
    ajint    s              =0;         /* Temp. start of hit value */
    ajint    e              =0;         /* Temp. end of hit value */
    AjPInt   start          =NULL;      /* Array of start of hit(s) */
    AjPInt   end            =NULL;      /* Array of end of hit(s) */
    ajint    nhits          =0;         /* Number of hits */
    ajint    x              =0;
    AjBool   foundkw        =ajFalse;
    AjBool   foundft        =ajFalse;


    /* Check for valid args */
    if(!inf)
	return ajFalse;




    /* allocate strings and arrays */
    line       = ajStrNew();
    id         = ajStrNew();
    start      = ajIntNew();
    end        = ajIntNew();



    /* Start of main loop */
    while((ajFileReadLine(inf,&line)))
    {
	/* Parse the AC line */
	if(ajStrPrefixC(line,"AC"))
	{
	    /* Copy accesion number and remove the ';' from the end*/
	    ajFmtScanS(line, "%*s %S", &id);
	    ajStrSubstituteCC(&id, ";", "\0");


	    /* Reset flags & no. hits*/
	    foundkw=ajFalse;
	    foundft=ajFalse;
	    nhits=0;
	}


	/* Search the description and keyword lines with search terms */
	else if((ajStrPrefixC(line,"DE") || (ajStrPrefixC(line,"KW"))))
	{
	    /* Search terms have a leading and trailing space to prevent
	       them being found as substrings within other words.  To
	       catch cases where a DE or KW line begins with a search
	       term, we must add a leading and trailing space to line.
	       We must first remove punctation from the line to be parsed.*/
	    ajStrConvertCC(&line, ".,;:", "    ");
	    ajStrAppK(&line, ' ');
	    ajStrInsertC(&line, 0, " ");


	    for (x = 0; x < terms->N; x++)
		/* Search term is found */
		if((ajStrFindCase(line, terms->Keywords[x])!=-1))
		{
		    foundkw=ajTrue;
		    break;
		}
	}


	/* Search the feature table line with search terms */
	else if((ajStrPrefixC(line,"FT   DOMAIN")))
	{
	    /* Search terms have a leading and trailing space to prevent
	       them being found as substrings within other words.  To
	       catch cases where a FT line ends with a search
	       term, we must add a  trailing space to line
	       We must first remove punctation from the line to be parsed.*/
	    ajStrConvertCC(&line, ".,;:", "    ");
	    ajStrAppK(&line, ' ');


	    for (x = 0; x < terms->N; x++)
		if((ajStrFindCase(line, terms->Keywords[x])!=-1))
		{
		    /* Search term is found */
		    foundft = ajTrue;
		    nhits++;

		    /* Assign start and end of hit */
		    ajFmtScanS(line, "%*s %*s %d %d", &s, &e);


		    ajIntPut(&start, nhits-1, s);
		    ajIntPut(&end, nhits-1, e);
		    break;
		}
	}


	/* Parse the sequence */
	else if((ajStrPrefixC(line,"SQ") && ((foundkw == ajTrue) ||
					     (foundft == ajTrue))))
	{
	    /* Allocate memory for temp. sequence */
	    temp       = ajStrNew();


	    /* Read the sequence into hitlist structure */
	    while((ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"//"))
		/* Read sequence line into temp */
		ajStrAppC(&temp,ajStrStr(line)+3);


	    /* Clean up temp. sequence */
	    ajStrCleanWhite(&temp);


	    /*Priority is given to domain (rather than full length) sequence */
	    if(foundft)
	    {
		for(x=0;x<nhits;x++)
		{
		    /* Increment counter of hits for subsequent hits*/
		    (*hits)->N++;


		    /*
		     * Reallocate memory for array of hits in hitlist
		     * structure
		     */
		    AJCRESIZE((*hits)->hits, (*hits)->N);
		    (*hits)->hits[(*hits)->N-1]=ajXyzHitNew();


		    /* Assign start and end of hit */
		    (*hits)->hits[(*hits)->N-1]->Start = ajIntGet(start, x);
		    (*hits)->hits[(*hits)->N-1]->End = ajIntGet(end, x);


		    /* Extract sequence within specified range */
		    ajStrAssSub(&(*hits)->hits[(*hits)->N - 1]->Seq, temp,
				(*hits)->hits[(*hits)->N - 1]->Start - 1,
				(*hits)->hits[(*hits)->N - 1]->End - 1);


	    /* Put id into structure */
	    ajStrAss(&(*hits)->hits[(*hits)->N - 1]->Acc, id);


	    /* In seqwords the Type field will always be set to OTHER */
	    ajStrAssC(&(*hits)->hits[(*hits)->N - 1]->Typeobj, "OTHER");

		}
	    }
	    else
	    {
		/* Increment counter of hits */
		(*hits)->N++;


		/* Reallocate memory for array of hits in hitlist structure */
		AJCRESIZE((*hits)->hits, (*hits)->N);
		(*hits)->hits[(*hits)->N-1]=ajXyzHitNew();


		/* Extract whole sequence */
		ajStrAss(&(*hits)->hits[(*hits)->N - 1]->Seq, temp);
		(*hits)->hits[(*hits)->N - 1]->Start = 1;
		(*hits)->hits[(*hits)->N - 1]->End =
		    ajStrLen((*hits)->hits[(*hits)->N - 1]->Seq);


	    /* Put id into structure */
	    ajStrAss(&(*hits)->hits[(*hits)->N - 1]->Acc, id);


	    /* In seqwords the Type field will always be set to OTHER */
	    ajStrAssC(&(*hits)->hits[(*hits)->N - 1]->Typeobj, "OTHER");
	    }


	    /* Free temp. sequence */
	    ajStrDel(&temp);


	}
    }


    /* Clean up */
    ajStrDel(&line);
    ajStrDel(&id);
    ajIntDel(&start);
    ajIntDel(&end);

    return ajTrue;
}

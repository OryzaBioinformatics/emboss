/* @source hetparse application
**
** Converts raw dictionary of heterogen groups to a file in embl-like format.
**
** @author: Copyright (C) Waqas Awan (wawan@hgmp.mrc.ac.uk)
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
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
** Parse dictionary of heterogen groups available at
** http://pdb.rutgers.edu/het_dictionary.txt (excerpt shown in Figure 1) and
** write a file containing the group names, synonyms and 3-letter codes in
** swissprot-style format (Figure 2).
**
** Optionally, hetparse will search a directory of pdb files and will
** count the number of files that each heterogen appears in.
**
** The records used in the output file are as follows:
** (1) ID - 3-character abbreviation of heterogen
** (2) DE - full description
** (3) SY - synonym
** (4) NN - no. of files which this heterogen appears in
**
**
** Figure 1
** Excerpt from heterogen dictionary
**
** RESIDUE   061     58
** CONECT      N1     2 N2   C5
** CONECT      N2     2 N1   N3
** CONECT      N3     2 N2   N4
** CONECT      N4     3 N3   C5   HN4
** CONECT      C5     3 N1   N4   C6
** CONECT      C6     3 C5   C7   C11
** ... data ommitted for clarity ...
** END
** HET    061             58
** HETSYN     061 L-159,061
** HETNAM     061 2-BUTYL-6-HYDROXY-3-[2'-(1H-TETRAZOL-5-YL)-BIPHENYL-4-
** HETNAM   2 061 YLMETHYL]-3H-QUINAZOLIN-4-ONE
** FORMUL      061    C26 H24 N6 O2
**
** RESIDUE   072     90
** CONECT      S1B    2 C1B  C2A
** CONECT      C1A    3 C1B  O1A  N3A
** CONECT      C1B    4 S1B  C1A  C1C  H1B
** CONECT      O1A    1 C1A
** CONECT      C1C    4 C1B  C1D 1H1C 2H1C
** ... data ommitted for clarity ...
** END
** HET    072             90
** HETSYN     072 THIAZOLIDINONE; GW0072
** HETNAM     072 (+/-)(2S,5S)-3-(4-(4-CARBOXYPHENYL)BUTYL)-2-HEPTYL-4-
** HETNAM   2 072 OXO-5-THIAZOLIDINE
** FORMUL      072    C37 H46 N2 O4 S1
**
** RESIDUE   074     58
** CONECT      C1     4 C2  1H1  2H1  3H1
** CONECT      C2     4 C1   C3  1H2  2H2
** CONECT      C3     4 C2   N1  1H3  2H3
** CONECT      N1     3 C3   C4  1HN1
** CONECT      C4     3 N1   O1   C5
** CONECT      O1     1 C4
** ... data ommitted for clarity ...
** END
** HET    074             58
** HETSYN     074 CA-074; [N-(L-3-TRANS-PROPYLCARBAMOYL-OXIRANE-2-
** HETSYN   2 074 CARBONYL)-L-ISOLEUCYL-L-PROLINE]
** HETNAM     074 [PROPYLAMINO-3-HYDROXY-BUTAN-1,4-DIONYL]-ISOLEUCYL-
** HETNAM   2 074 PROLINE
** FORMUL      074    C18 H31 N3 O6
** ... data ommitted for clarity ...
**
**
**
** Figure 2 Example of hetparse output file
**
** ID  061
** DE  2-BUTYL-6-HYDROXY-3-[2'-(1H-TETRAZOL-5-YL)-BIPHENYL-4-YLMETHYL]-3H-
** DE  QUINAZOLIN-4-ONE
** SY  L-159,061
** NN  2
** //
** ID  072
** DE  (+/-)(2S,5S)-3-(4-(4-CARBOXYPHENYL)BUTYL)-2-HEPTYL-4-OXO-5-THIAZOLIDINE
** SY  THIAZOLIDINONE; GW0072
** NN  10
** //
** ID  074
** DE  [PROPYLAMINO-3-HYDROXY-BUTAN-1,4-DIONYL]-ISOLEUCYL-PROLINE
** SY  CA-074; [N-(L-3-TRANS-PROPYLCARBAMOYL-OXIRANE-2-CARBONYL)-L-ISOLEUCYL-L-
** SY  PROLINE]
** NN  1
** //
**
**
** Notes
**
******************************************************************************/






#include "emboss.h"

static AjBool        hetparse_HetScan(AjPStr path, AjPStr extn, AjPHet ptr);



/* @prog hetparse *************************************************************
**
**  Parse dictionary of heterogen groups
**
******************************************************************************/
int main(int argc, char **argv)
{
    AjPFile   fin=NULL;
    AjPFile   fout=NULL;
    AjPHet dic=NULL;
    AjPStr    path=NULL;
    AjPStr    extn=NULL;
    AjBool    dogrep=ajFalse;



    /* Allocate string */
    path=ajStrNew();
    extn=ajStrNew();


    embInit("hetparse", argc, argv);

    /* Get values from acd */
    fin    = ajAcdGetInfile("inf");
    fout   = ajAcdGetOutfile("outf");
    dogrep = ajAcdGetBool("dogrep");
    path   = ajAcdGetString("path");
    extn   = ajAcdGetString("extn");




    /* Parse raw file */
    if(!ajXyzHetRawRead(fin, &dic))
	ajFatal("ajXyzHetRawRead failed\n");


    /* Search pdb files for heterogens if appropriate */
    if(dogrep)
	hetparse_HetScan(path, extn, dic);


    /* Write output file */
    if(!ajXyzHetWrite(fout, dic, dogrep))
	ajFatal("ajXyzHetWrite failed\n");


    /* Tidy up and return */
    ajXyzHetDel(&dic);
    ajFileClose(&fin);
    ajFileClose(&fout);
    ajStrDel(&path);
    ajStrDel(&extn);


    ajExit();
    return 0;
}






/* @funcstatic hetparse_HetScan ***********************************************
**
** Search a directory of pdb files and count the number of files that each
** heterogen (from a Het object) appears in.
**
** @param [r] path [AjPStr]    Path of pdb files
** @param [r] extn [AjPStr]    Extension of pdb files
** @param [w] ptr  [AjPHet]    Het object
**
** @return [AjBool] True on success
** @@
******************************************************************************/
static AjBool        hetparse_HetScan(AjPStr path, AjPStr extn, AjPHet ptr)
{
    AjPList     listfiles=NULL;   /* List of files in  directory */
    AjPList     listhet=NULL;     /* List of names of different heterogens
				     in the current file */
    AjIList     iter=NULL;        /* Iterator for listhet */
    AjPStr      search_term=NULL; /* Temp. string to match against files
				     in directory */
    AjPStr      fname =NULL;      /* Temp pointer */
    AjPFile     fptr=NULL;        /* File pointer */
    AjPStr      line=NULL;        /* A line from the file */
    AjPStr      het=NULL;         /* 3-character code of heterogen */
    AjPStr      hetcopy=NULL;     /* Copy of 3-character code of heterogen
				     for list*/
    AjPStr      hettemp=NULL;     /* Temp. pointer to 3-character code of
				     heterogen */
    AjBool      foundhet=ajFalse; /* True if current heterogen was found
				     in listhet */
    ajint       i=0;              /* Counter */



    /* Check args */
    if(!path || !extn || !ptr)
    {
	ajWarn("Bad arg's passed to hetparse_HetScan");
	return ajFalse;
    }



    /* Allocate memory */
    search_term = ajStrNew();
    listfiles = ajListNew();
    line = ajStrNew();
    het = ajStrNew();


    /* Create list of files in alignments directory */

    ajStrAssC(&search_term, "*");
    if((ajStrChar(extn, 0)=='.'))
        ajStrApp(&search_term, extn);
    else
    {
        ajStrAppC(&search_term, ".");
        ajStrApp(&search_term, extn);
    }
    ajFileScan(path, search_term, &listfiles, ajFalse, ajFalse,
               NULL, NULL, ajFalse, NULL);



    /*Loop for reading each file in directory*/
    while(ajListPop(listfiles,(void **)&fname))
    {
        /* Open pdb file */
        if((fptr=ajFileNewIn(fname))==NULL)
	{
	    ajWarn("Could not open file in hetparse_HetScan\n");
	    continue;
	}
	else
	    ajFmtPrint("Processing %S\n", fname);



	/* Create list of heterogens */
	listhet = ajListstrNew();

	/* PARSE FILE & POPULATE LIST OF HETATM CODES */
	while(ajFileReadLine(fptr, &line))
	{
	    if(ajStrPrefixC(line,"HETATM"))
	    {
		/* Copy heterogen code from pdb file into <het>*/
		ajStrAssSub(&het, line, 17, 19);


		/* Initialise iterator to iterate through the list
		   <listhet> */
		iter=ajListIter(listhet);

		foundhet=ajFalse;

		/* Iterate through the list, make <hettemp> point to
		   the current node */
		while((hettemp=(AjPStr)ajListIterNext(iter)))
		{
		    /* If <het> matches the current node, break */
		    if(ajStrMatch(hettemp, het))
		    {
			foundhet=ajTrue;
			break;
		    }
		}

		/* <het> was not found in the list ... so add it */
		if(!foundhet)
		{
		    hetcopy=ajStrNew();
		    ajStrAss(&hetcopy, het);
		    ajListstrPush(listhet, hetcopy);
		}


		/* Free the list iterator */
		ajListIterFree(iter);
	    }
	}




	/* Compare list of heterogens from this file to our dictionary */
	/* Initialise iterator to iterate through the list <listhet> */
	iter=ajListIter(listhet);

	/* Iterate through the list, make <hettemp> point to the current node */
	while((hettemp=(AjPStr)ajListIterNext(iter)))
	{
	    /* Run through each heterogen in our dictionary and
	       increment the counter if we have a match*/
	    for(i=0; i< ptr->n; i++)
	    {
		if(ajStrMatch(hettemp, ptr->entries[i]->abv))
		{
		    ptr->entries[i]->cnt++;
		    break;
		}
	    }
	}


	/* Close file and tidy up */
	ajFileClose(&fptr);
	ajStrDel(&fname);
	ajListIterFree(iter);

	/* Free the list and its contents */
	iter=ajListIter(listhet);

	while((hettemp=(AjPStr)ajListIterNext(iter)))
	    ajStrDel(&hettemp);

	ajListstrDel(&listhet);

	/* Free the list iterator */
	ajListIterFree(iter);
    }



    /* Tidy up and return */
    ajStrDel(&search_term);
    ajListDel(&listfiles);
    ajStrDel(&line);
    ajStrDel(&het);

    return ajTrue;
}






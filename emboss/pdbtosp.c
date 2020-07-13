/* @source pdbtosp application
**
** Convert raw swissprot:pdb equivalence file to embl-like format.
**
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
**
** Operation
**
** pdbtosp parses the swissprot:pdb equivalence table (Figure 1) available
** at URL (1)
**
** (1) http://www.expasy.ch/cgi-bin/lists?pdbtosp.txt and writes the
** data out in embl-like format file (Figure 2). The raw (input) file
** can be obtained by doing a "save as ... text format" from the web
** page (1),
**
** No changes are made to the data other than changing the format in which it
** is held. This EMBL-like format data file is used by several other EMBOSS
** programs. The reason why the raw text is changed to an EMBL-like
** format before being used used by other EMBOSS programs is that it is an
** easier format to work with than the raw format. The records used to describe
** an entry are given below.
**
**
** Figure 1  Excerpt from the swissprot:pdb equivalence table
**
**   ------------------------------------------------------------------------
**   ExPASy Home page   Site Map    Search ExPASy   Contact us    SWISS-PROT
**
** Hosted by SIB       Mirror                                        USA[new]
** Switzerland         sites:      AustraliaCanada China Korea Taiwan
**  ------------------------------------------------------------------------
**
** < Other bibliographic information ommitted for clarity >
**
** PDB   Last revision
** code  date           SWISS-PROT entry name(s)
** ____  ___________    __________________________________________
** 101M  (08-APR-98)  : MYG_PHYCA   (P02185)
** 102L  (31-OCT-93)  : LYCV_BPT4   (P00720)
** 102M  (08-APR-98)  : MYG_PHYCA   (P02185)
** 103L  (31-OCT-93)  : LYCV_BPT4   (P00720)
**
** < Data ommitted for clarity >
**
** 7EST  (30-APR-94)  : EL1_PIG     (P00772)
** 7FAB  (31-JAN-94)  : HV2G_HUMAN  (P01825), LAC_HUMAN   (P01842),
**                      LV1E_HUMAN  (P01703)
** 9WGA  (15-OCT-90)  : AGI2_WHEAT  (P02876)
** 9XIA  (15-JUL-92)  : XYLA_STRRU  (P24300)
** 9XIM  (15-JUL-93)  : XYLA_ACTMI  (P12851)
**
** ----------------------------------------------------------------------------
** SWISS-PROT is copyright.  It is produced through a collaboration between the
** Swiss Institute  of  Bioinformatics   and the EMBL Outstation - the European
**
** < Other bibliographic information ommitted for clarity >
**
**
**
** Figure 2  Excerpt from embl-like format swissprot:pdb equivalence file
**
** EN   3SDH
** XX
** NE   2
** XX
** IN   LEU3_THETH ID; P00351 ACC;
** IN   LEU4_THEFF ID; P02351 ACC;
** XX
** //
** EN   2SDH
** XX
** NE   1
** XX
** IN   LEU1_FDFTH ID; P11351 ACC;
** XX
** //
**
**
** Important notes
** pdbtosp relies on finding a line beginning with '____  _' in the input file
** (all lines up to and including this one are ignored). Lines of code data
** are then parsed, up until the first blank line.
**
******************************************************************************/





#include "emboss.h"







/* @prog pdbtosp **************************************************************
**
** Convert raw swissprot:pdb equivalence file to embl-like format
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile    inf1    =NULL;
    AjPFile    outf    =NULL;
    AjPStr     pdb     =NULL;   /* PDB identifier */
    AjPStr     spr     =NULL;   /* Swissprot identifier */
    AjPStr     acc     =NULL;   /* Accession number */
    AjPStr     pspr    =NULL;   /* Swissprot identifier pointer */
    AjPStr     pacc    =NULL;   /* Accession number pointer */
    AjPStr     line    =NULL;   /* Line from file */
    AjPStr     token   =NULL;   /* Token from line */
    AjPStr     subtoken=NULL;   /* Token from line */
    AjPList    acclist =NULL;   /* List of accession numbers */
    AjPList    sprlist =NULL;   /* List of swissprot identifiers */
    ajint      n       =0;      /* No. of accession numbers for
				   current pdb code */
    AjBool     ok      =ajFalse;/* True if "____ _" has been found and
				  we can start parsing */
    AjBool     done_1st=ajFalse;/* True if the first line of data has
				   been parsed*/






    /* Read data from acd*/
    embInit("pdbtosp", argc, argv);
    inf1  =  ajAcdGetInfile("infilea");
    outf  =  ajAcdGetOutfile("outfile");


    /* Memory allocation */
    line    = ajStrNew();
    token   = ajStrNew();
    subtoken= ajStrNew();
    pdb     = ajStrNew();
    acclist = ajListstrNew();
    sprlist = ajListstrNew();


    /* Read lines from file */
    while(ajFileReadLine(inf1, &line))
    {
	if(ajStrPrefixC(line, "____  _"))
	{
	    ok=ajTrue;
	    continue;
	}


	if(!ok)
	    continue;

	if(ajStrMatchC(line, ""))
	    break;



	/* Read in pdb code first.  Then tokenise by ':', discard the
	   first token, then tokenise the second token by ',', parsing
	   out the swisssprot codes and accession numbers from the subtokens*/


	/* Make sure this is a line containing the pdb code */
	if((ajStrFindC(line, ":")!=-1))
	{
	    if(done_1st)
	    {
		/* Print data for last pdb code to file */
		ajFmtPrintF(outf, "%-5s%S\nXX\n%-5s%d\nXX\n",
			    "EN", pdb, "NE", n);

		while(ajListstrPop(acclist, &pacc))
		{
		    ajListstrPop(sprlist, &pspr);

		    ajFmtPrintF(outf, "%-5s%S ID; %S ACC;\n",
				"IN", pspr, pacc);

		    ajStrDel(&pspr);
		    ajStrDel(&pacc);
		}
		ajFmtPrintF(outf, "XX\n//\n");

		n=0;
	    }

	    ajFmtScanS(line, "%S", &pdb);

	    ajStrTokC(line, ":");
	    ajStrAssS(&token, ajStrTokC(NULL, ":"));

	    done_1st=ajTrue;
	}
	else
	{
	    ajStrAssS(&token, line);
	}


	spr  = ajStrNew();
	acc  = ajStrNew();
	ajFmtScanS(token, "%S (%S", &spr, &acc);

	if(ajStrSuffixC(acc, "),"))
	{
	    ajStrChop(&acc);
	    ajStrChop(&acc);
	}
	else
       	    ajStrChop(&acc);


	ajListstrPushApp(acclist, acc);
	ajListstrPushApp(sprlist, spr);
	n++;

	ajStrTokC(token, ",");
	while((subtoken=ajStrTokC(NULL, ",")))
	{
	    spr  = ajStrNew();
	    acc  = ajStrNew();

	    ajFmtScanS(subtoken, "%S (%S", &spr, &acc);

	    if(ajStrSuffixC(acc, "),"))
	    {
		ajStrChop(&acc);
		ajStrChop(&acc);
	    }
	    else
		ajStrChop(&acc);


	    ajListstrPushApp(acclist, acc);
	    ajListstrPushApp(sprlist, spr);
	    n++;
	}
    }


    /* Print data for last pdb code to file */
    ajFmtPrintF(outf, "%-5s%S\nXX\n%-5s%d\nXX\n",
		"EN", pdb, "NE", n);

    while(ajListstrPop(acclist, &pacc))
    {
	ajListstrPop(sprlist, &pspr);

	ajFmtPrintF(outf, "%-5s%S ID; %S ACC;\n",
		    "IN", pspr, pacc);

	ajStrDel(&pspr);
	ajStrDel(&pacc);
    }
    ajFmtPrintF(outf, "XX\n//\n");



    /* Tidy up */
    ajFileClose(&inf1);
    ajFileClose(&outf);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&subtoken);
    ajStrDel(&pdb);
    ajListstrDel(&acclist);
    ajListstrDel(&sprlist);

    ajExit();
    return 0;
}

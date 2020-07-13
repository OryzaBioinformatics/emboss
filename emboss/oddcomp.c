/* @source oddcomp application
**
** Identifies sequences with a region with a high composition of specific
** words
**
** @author: Copyright (C) David Martin (david.martin@biotek.uio.no) based on
** compseq by Gary Williams
** @@
**
** Last modified 8 November 1999 David Martin.
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
******************************************************************************/


#include "emboss.h"


static ajint oddcomp_readexpfreq(AjPTable *exptable, AjPFile compdata,
				 ajint *size);
static ajint oddcomp_makebigarray(ajlong no_elements, ajlong **bigarray);




/* @prog oddcomp **************************************************************
**
** Finds protein sequence regions with a biased composition
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq seq;
    ajint word =2;
    /*  AjBool inwindow;*/
    AjPFile outfile;
    AjPFile compdata;
    ajint window;
    ajint pos;
    char *s;
    ajlong result;
    ajlong *bigarray;
    ajlong *windowbuffer;		/* ring buffer for sliding window */
    ajulong no_elements;
    AjBool first_time_round = ajTrue;
    AjBool ignorebz = ajTrue;
    ajulong count;
    AjPStr dispseq=NULL;
    AjPStr ajb=NULL;
    ajulong total=0;
    ajulong other=0;
    AjBool otherflag;
    AjBool seqisnuc=ajFalse;
    ajint increment = 1;
    ajint ringsize;
    ajlong steps = 0;
    /*  ajint count_of_sequence_names = 0;*/
    AjPTable exptable = NULL;		/* table of expected frequencies */


    ajlong exp_freq;
    (void) embInit ("oddcomp", argc, argv);

    seqall = ajAcdGetSeqall ("sequence");
    window = ajAcdGetInt ("window");
    outfile = ajAcdGetOutfile ("outfile");
    compdata = ajAcdGetInfile ("compdata");
    ignorebz = ajAcdGetBool ("ignorebz");

    /* number of overlapping words in a window */
    ringsize = window - word + 1;

    /* Output some documentation to the results file */
    (void) ajFmtPrintF(outfile, "#\n"
		       "# Output from 'oddcomp'\n"
		       "#\n");
    (void) ajFmtPrintF(outfile,
		       "# The Expected frequencies are taken from the "
		       "file: %s\n", ajFileName(compdata));

    /* read the required frequencies into a table */
    (void) oddcomp_readexpfreq(&exptable, compdata, &word);


    /* more notes */
    (void) ajFmtPrintF(outfile, "#\n#\tWord size: %d\n",word);



    while (ajSeqallNext(seqall, &seq))
    {
	seqisnuc = ajSeqIsNuc(seq);

	/* not interested in nucleotide sequences so ignore any that get in */
	if (seqisnuc)
	    continue;

	/* ignore sequences shorter than the window of interest */
	if (ajSeqLen(seq)<window)
	    continue;


	/*
	 *  we first of all need to make a store for the results in a
	 *  nice big array
	 */
	if (first_time_round) {

	    if (!embNmerGetNoElements(&no_elements, word, seqisnuc, ignorebz))
		(void) ajFatal ("The word size is too large for the data "
				"structure available.");


	    (void) oddcomp_makebigarray(no_elements, &bigarray);

	    (void) oddcomp_makebigarray(ringsize, &windowbuffer); /* create
								     ring
								     buffer */

	    first_time_round = ajFalse;
	}

	(void) ajSeqToUpper(seq);
	s = ajSeqChar(seq);

	/*  initialise the results buffer for this sequence.
	 *  each word will require a certain number of counts to get to the
	 *  necessary frequency. Set the number of counts to negative this so
	 *  it is only necessary to check for counts >0. Also set the steps
	 *  variable to go the number of steps needed before a new check needs
	 *  to be made (count minimum number of words required before the state
	 *  can change.)
	 */
	for (count=0; count< no_elements;count++)
	{
	    (void) ajStrClear(&dispseq);
	    /*
	     *  need to clear the string as embNmerInt2Prot will prepend
	     *  to it
	     */
	    (void) embNmerInt2prot(&dispseq, word, count, ignorebz);
	    ajb=ajTableGet (exptable, dispseq);
	    if (ajb)
		(void) ajStrToLong( ajb, &exp_freq);
	    else
		exp_freq=0;

	    if ( exp_freq>0)
		/*
		 *  set bigarray count to negative the count needed to
		 *  exceed the frequency
		 */
		bigarray[count] = - exp_freq;
	    else
		bigarray[count]=0;
	}


	/*
	 *  Start at the first position, and fill the ring buffer by
	 *  sliding one step at a time.
	 */
	for (pos=1;pos<= ringsize; pos += increment)
	{
	    result = embNmerProt2int(s, word, pos, &otherflag,ignorebz);
	    if (otherflag)
		windowbuffer[pos%ringsize]=-1;
	    else
	    {
		windowbuffer[pos%ringsize]=result;
		bigarray[result]++;
	    }
	}

	/*
	 *  ringbuffer now full. calculate the number of steps to get a
	 *  change in  state by working out the sum of negative values
	 */

	for (count=0; count<no_elements;count++)
	    if (bigarray[count]<0)
		steps -= bigarray[count];

	for (pos=ringsize+1; pos <= ajSeqLen(seq)-word; pos += increment)
	{
	    /*
	     *  have we got to check to see whether or not we have the
	     *  necessary composition
	     */
	    if (steps==0)
	    {
		for (count=0; count<no_elements;count++)
		    if (bigarray[count]<0)
			steps -= bigarray[count];

		/*now check to see if the composition is a hit. */
		if (steps==0)
		{
		    ajFmtPrintF(outfile, "\t%s\n", ajSeqName(seq));
		    total++;
		    break;
		}
	    }
	    else
		steps--;

	    result = embNmerProt2int(s, word, pos, &otherflag,ignorebz);

	    /* uncount the word just leaving the window if it wasn't 'other'*/
	    if (windowbuffer[pos%ringsize] >=0)
		bigarray[windowbuffer[pos%ringsize]]--;
	    else
		other--;

	    /* count this word */


	    if (!otherflag)
	    {
		windowbuffer[pos%ringsize] = result;
		bigarray[result]++;
	    }
	    else
	    {
		windowbuffer[pos%ringsize] = -1;
		other++;
	    }
	}
    }


    (void) ajFmtPrintF(outfile, "\n#\tEND\t#\n");

    (void) ajFileClose(&outfile);

    /* tidy up */

    AJFREE (bigarray);

    (void) ajStrTableFree(&exptable);

    ajExit ();
    return 0;
}

/* @funcstatic oddcomp_makebigarray *******************************************
**
** Undocumented.
**
** @param [?] no_elements [ajlong] Undocumented
** @param [?] bigarray [ajlong**] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint oddcomp_makebigarray(ajlong no_elements, ajlong **bigarray)
{

    AJCNEW(*bigarray, no_elements);

    return 0;
}

/* @funcstatic oddcomp_readexpfreq ********************************************
**
** Undocumented.
**
** @param [?] exptable [AjPTable*] Undocumented
** @param [?] compdata [AjPFile] Undocumented
** @param [?] size [ajint*] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint oddcomp_readexpfreq(AjPTable *exptable, AjPFile compdata,
				 ajint *size)
{
    AjPStr line = NULL;
    char whiteSpace[] = " \t\n\r";
    AjPStrTok tokens;
    AjPStr sizestr=NULL;
    ajint thissize;
    AjPStr key;
    AjPStr value;

    /* initialise the hash table - use case-insensitive comparison */

    *exptable = ajStrTableNewCase(350);



    /* read the file */
    while (ajFileReadLine(compdata, &line))
    {
	/* skip comment and blank lines */
	if (!ajStrFindC(line, "#"))
	    continue;
	if (!ajStrLen(line))
	    continue;

	/* look for the word size */
	if (!ajStrFindC(line, "Word size"))
	{
	    (void) ajStrAssSub(&sizestr, line, 10, ajStrLen(line));
	    (void) ajStrChomp(&sizestr);
	    (void) ajStrToInt(sizestr, &thissize);

	    *size = thissize;
	    break;
	}
	else
	    (void) ajFatal ("The 'Word size' line was not found, "
			    "instead found:\n%S\n",line);
    }

    /* read the file */
    while (ajFileReadLine(compdata, &line))
    {
	/* skip comment and blank lines */
	if (!ajStrFindC(line, "#"))
	    continue;
	if (!ajStrLen(line))
	    continue;

	/*
	 *  look for the total number of counts - anything after this is
	 *  our data
	 */
	if (!ajStrFindC(line, "Total"))
	    break;
    }

    /* read in the observed frequencies as a string */
    while (ajFileReadLine(compdata, &line))
    {
	/* skip comment and blank lines */
	if (!ajStrFindC(line, "#"))
	    continue;
	if (!ajStrLen(line))
	    continue;

	tokens = ajStrTokenInit(line, whiteSpace);

	/* get the word as the key */
	key = ajStrNew();
	(void) ajStrToken( &key, &tokens, NULL);

	/*
	 *  get the observed count as the value - we'll use this as the
	 *  expected frequency
	 */
	value = ajStrNew();
	(void) ajStrToken( &value, &tokens, NULL);
	(void) ajTablePut( *exptable, key, value);
	(void) ajStrTokenClear( &tokens);
    }

    /* tidy up */
    ajStrDel(&line);
    ajStrDel(&sizestr);

    return 0;
}

/* @source compseq application
**
** Counts the composition of dimer/trimer/etc words in a sequence
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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
******************************************************************************/

#include "emboss.h"




static ajint compseq_readexpfreq(AjPTable *exptable, AjPFile infile,
				 ajint size);
static ajint compseq_makebigarray(ajulong no_elements, ajulong **bigarray);




/* @prog compseq **************************************************************
**
** Counts the composition of dimer/trimer/etc words in a sequence
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq seq;
    ajint word;
    AjBool zerocount;
    AjBool ignorebz;
    ajint frame;
    AjPFile outfile;
    AjPFile infile;
    AjBool reverse;
    AjPStr ajb=NULL;

    ajint pos;
    char *s;
    ajulong result;
    ajulong *bigarray;
    ajulong no_elements;
    AjBool first_time_round = ajTrue;
    ajulong count;
    AjPStr dispseq = NULL;
    ajulong total  = 0;
    ajulong other  = 0;
    AjBool otherflag;
    AjBool seqisnuc=ajFalse;
    ajint increment;
    ajint count_of_sequence_names = 0;
    AjPTable exptable = NULL;		/* table of expected frequencies */

    /* holds the string of "Other" for looking in the hash table 'exptable' */
    AjPStr strother = NULL;


    /* Don't have a file of expected frequencies yet */
    AjBool have_exp_freq = ajFalse;
    double default_exp_freq;
    double obs_freq = 0.0;
    double exp_freq;
    double obs_exp;

    embInit("compseq", argc, argv);

    seqall    = ajAcdGetSeqall("sequence");
    word      = ajAcdGetInt("word");
    outfile   = ajAcdGetOutfile("outfile");
    infile    = ajAcdGetInfile("infile");
    zerocount = ajAcdGetBool("zerocount");
    ignorebz  = ajAcdGetBool("ignorebz");
    frame     = ajAcdGetInt("frame");
    reverse   = ajAcdGetBool("reverse");

    /* Output some documentation to the results file */
    ajFmtPrintF(outfile, "#\n# Output from 'compseq'\n#\n");

    if(!zerocount)
	ajFmtPrintF(outfile,"# Words with a frequency of zero are "
		    "not reported.\n");


    if(!ignorebz)
	ajFmtPrintF(outfile,"# The amino acid codes 'B' and 'Z' will "
		    "be counted,\n# rather than treated as 'Other'.\n");
    
    
    if(frame)
	ajFmtPrintF(outfile,"# Only words in frame %d will be "
		    "counted.\n", frame);
    
    if(infile == NULL)
	ajFmtPrintF(outfile,"# The Expected frequencies are "
		    "calculated on the (false) assumption that every\n"
		    "# word has equal frequency.\n");
    else
    {
	ajFmtPrintF(outfile,"# The Expected frequencies are taken "
		    "from the file: %s\n",ajFileName(infile));
	compseq_readexpfreq(&exptable, infile, word);
	have_exp_freq = ajTrue;
    }
    ajFmtPrintF(outfile, "#\n");
    ajFmtPrintF(outfile, "# The input sequences are:\n");
    
    
    /*
    **  see if using a sliding window or only looking at a word-sized
    **  single frame
    */
    if(frame)
	increment = word;
    else
	increment = 1;
    
    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);

	/* note the name of the sequence */
	if(count_of_sequence_names++ < 10)
	    ajFmtPrintF(outfile, "#\t%s\n", ajSeqName(seq));
	else if(count_of_sequence_names++ == 11)
	    ajFmtPrintF(outfile, "# ... et al.\n");


	/* first of all need to make a store for the results */
	if(first_time_round)
	{
	    seqisnuc = ajSeqIsNuc(seq);
	    if(!embNmerGetNoElements(&no_elements, word, seqisnuc, ignorebz))
		ajFatal("The word size is too large for the data "
			"structure available.");

	    compseq_makebigarray(no_elements, &bigarray);
	    first_time_round = ajFalse;
	}

	ajSeqToUpper(seq);
	s = ajSeqChar(seq);

	/*
	**  Start at the first position, or at the frame, if it has been
	**  specified and then look at each word in a sliding window if
	**  no frame is specified, or at each increment of the word-size
	**  if frame is specified. Stop when less than a word-length from
	**  the end of the sequence.
	*/
	for(pos=frame; pos <= ajSeqLen(seq)-word; pos += increment)
	{
	    if(seqisnuc)
		result = embNmerNuc2int(s, word, pos, &otherflag);
	    else
		result = embNmerProt2int(s, word, pos, &otherflag, ignorebz);


	    /* count this word */
	    if(!otherflag)
		bigarray[result]++;
	    else
		other++;

	    total++;
	}

	if(seqisnuc && reverse)
	{
	    /* Do it again on the reverse strand */
	    ajSeqReverse(seq);
	    s = ajSeqChar(seq);

	    for(pos=frame; pos <= ajSeqLen(seq)-word; pos += increment)
	    {
		if(seqisnuc)
		    result = embNmerNuc2int(s, word, pos, &otherflag);
		else
		    result = embNmerProt2int(s, word, pos, &otherflag,
					     ignorebz);

		/* count this word */
		if(!otherflag)
		    bigarray[result]++;
		else
		    other++;

		total++;
	    }
	}
    }
    
    /* Give the word size used */
    ajFmtPrintF(outfile,"\n\nWord size\t%d\n", word);
    
    /* Now output the Total count */
    ajFmtPrintF(outfile,"Total count\t%Lu\n\n", total);
    
    /* we have now counted the frequency of the words in the sequences */
    ajFmtPrintF(outfile,
		"#\n# Word\tObs Count\tObs Frequency\tExp Frequency\t"
		"Obs/Exp Frequency\n#\n");
    
    /*
    **  if there's no file of expected frequencies, then make one
    **  by giving each word an equal frequency
    */
    default_exp_freq = 1/(double)no_elements;
    
    for(count=0; count<no_elements; count++)
    {
	if(!zerocount && bigarray[count] == 0)
	    continue;

	ajStrClear(&dispseq);

	if(seqisnuc)
	    embNmerInt2nuc(&dispseq, word, count);
	else
	    embNmerInt2prot(&dispseq, word, count, ignorebz);

	if(have_exp_freq)
	{
	    if((ajb=ajTableGet(exptable,dispseq)))
		ajStrToDouble(ajb, &exp_freq);
	    else
		exp_freq = default_exp_freq;
	}
	else
	    exp_freq = default_exp_freq;

	if(exp_freq == 0.0)
	    /* display a big number rather than a divide by zero error */
	    obs_exp = 10000000000.0;
	else
	{
	    obs_freq = (double)bigarray[count]/(double)total;
	    obs_exp = obs_freq/exp_freq;
	}

	ajFmtPrintF(outfile, "%S\t%Lu\t\t%.7f\t%.7f\t%.7f\n", dispseq,
		    bigarray[count], obs_freq, exp_freq, obs_exp);
    }
    
    /* now output the Other count */
    if(have_exp_freq)
    {
	ajStrAssC(&strother, "Other");
	ajStrToDouble(ajTableGet(exptable, strother), &exp_freq);
    }
    else
	obs_freq = (double)other/(double)total;
    
    
    if(!have_exp_freq || exp_freq==0.0)
    {
	exp_freq = 0.0;
	/* display a big number rather than a divide by zero error */
	obs_exp = 10000000000.0;
    }
    else
	obs_exp = obs_freq/exp_freq;
    
    
    ajFmtPrintF(outfile, "\nOther\t%Lu\t\t%.7f\t%.7f\t%.7f\n", other,
		obs_freq, exp_freq, obs_exp);
    
    ajFileClose(&outfile);
    

    AJFREE(bigarray);
    
    if(have_exp_freq)
	ajTableFree(&exptable);
    
    ajExit();

    return 0;
}




/* @funcstatic compseq_makebigarray *******************************************
**
** Undocumented.
**
** @param [?] no_elements [ajulong] Undocumented
** @param [?] bigarray [ajulong**] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint compseq_makebigarray(ajulong no_elements, ajulong **bigarray)
{

    ajDebug("makebigarray for %ld elements\n", no_elements);
    AJCNEW(*bigarray, no_elements);

    return 0;
}




/* @funcstatic compseq_readexpfreq ********************************************
**
** Undocumented.
**
** @param [?] exptable [AjPTable*] Undocumented
** @param [?] infile [AjPFile] Undocumented
** @param [?] size [ajint] Undocumented
** @return [ajint] Undocumented
** @@
******************************************************************************/

static ajint compseq_readexpfreq(AjPTable *exptable, AjPFile infile,
				 ajint size)
{

    AjPStr line = NULL;
    char whiteSpace[] = " \t\n\r";
    AjPStrTok tokens;
    AjPStr sizestr = NULL;
    ajint thissize;
    AjPStr key;
    AjPStr value;

    /* initialise the hash table - use case-insensitive comparison */
    *exptable = ajStrTableNewCase(350);


    /* read the file */
    while(ajFileReadLine(infile, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrLen(line))
	    continue;

	/* look for the word size */
	if(!ajStrFindC(line, "Word size"))
	{
	    ajStrAssSub(&sizestr, line, 10, ajStrLen(line)-1);
	    ajStrChomp(&sizestr);
	    ajStrToInt(sizestr, &thissize);

	    if(size == thissize)
		break;

	    ajFatal("The word size you are counting (%d) is different "
			    "to the word\nsize in the file of expected "
			    "frequencies (%d).", size, thissize);

	}
	else
	    ajFatal("The 'Word size' line was not found, instead "
			    "found:\n%S\n", line);
    }

    /* read the file */
    while(ajFileReadLine(infile, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrLen(line))
	    continue;

	/*
	**  look for the total number of counts - anything after this
	**  is the data
	*/
	if(!ajStrFindC(line, "Total"))
	    break;
    }

    /* read in the observed frequencies as a string */
    while(ajFileReadLine(infile, &line))
    {
	/* skip comment and blank lines */
	if(!ajStrFindC(line, "#"))
	    continue;

	if(!ajStrLen(line))
	    continue;

	tokens = ajStrTokenInit(line, whiteSpace);

	/* get the word as the key */
	key = ajStrNew();
	ajStrToken( &key, &tokens, NULL);

	/*
	**  get the observed frequency as the value - we'll use this as
	**  the expected frequency
	*/
	value = ajStrNew();

	/* skip the observed count column */
	ajStrToken(&value, &tokens, NULL);
	ajStrToken(&value, &tokens, NULL);

	ajTablePut(*exptable, key, value);

    }


    ajStrDel(&line);
    ajStrTokenClear(&tokens);

    return 0;
}

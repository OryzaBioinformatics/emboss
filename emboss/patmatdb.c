/* @source patmatDB.c
** @author: Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk)
** @@
** Application for pattern matching, one Prosite motif against a sequence database.
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


static void patmatdb_spaces(AjPFile *outf, ajint length);




/* @prog patmatdb *************************************************************
**
** Search a protein sequence with a motif
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPFile outf 		=NULL;	
    AjPFeattable tab            =NULL;
    AjPReport report            =NULL;

    AjPSeqall seqall 		=NULL;
    AjPSeq seq			=NULL;
    AjPStr str 			=NULL;	
    AjPStr regexp 		=NULL;
    AjPStr motif		=NULL;
    AjPStr temp = NULL;
    
    AjPStr regexpdata	=NULL;
    EmbPPatMatch match 	=NULL;
	
	
    ajint i;
    ajint number;
    ajint start;
    ajint end;
    ajint length;
    ajint zstart;
    ajint zend;    
    ajint seqlength;
    ajint j;
    AjPStr         tmpstr = NULL;
    AjPFeature gf;
    AjPStr fthit = NULL;

    embInit ("patmatdb", argc, argv);
	
    seqall	= ajAcdGetSeqall ("sequence");
    motif 	= ajAcdGetString ("motif");
    /* outf        = ajAcdGetOutfile("outfile"); */
    report = ajAcdGetReport ("outfile");

    temp=ajStrNew();
    ajStrToUpper(&motif);
    ajStrAssC (&fthit, "hit");

    /*converting the Prosite motif to a reg exps */		
    regexp =embPatPrositeToRegExp(&motif);
   
    ajFmtPrintAppS (&tmpstr, "Motif: %S\n", motif);
    ajReportSetHeader (report, tmpstr);

    while (ajSeqallNext(seqall, &seq))
    {
	str = ajSeqStr(seq); 
	ajStrToUpper(&str);
	
	/* comparing the reg exps to sequence for matches. */
	match 	= embPatPosMatchFind(regexp, str);

	/*returns the number of posix matches in the structure. */
	number 	= embPatPosMatchGetNumber(match);
	if(number && outf)
	    ajFmtPrintF(outf,"\nNumber of matches found in %s = %d\n",
			ajSeqName(seq), number);

	if (number)
	  tab = ajFeattableNewSeq(seq);

	for (i=0; i<number; i++)
	{
	    seqlength = ajStrLen(str);
	    if (outf)
	      ajFmtPrintF(outf, "Length of the sequence = %d basepairs\n",
			  seqlength);

	    /*returns length from pattern match for index'th item. */
	    length = embPatPosMatchGetLen(match, i);
	    if (outf)
	      ajFmtPrintF(outf, "Length of match = %d\n", length);

	    /*
	     * returns the start position from the pattern match for the
             * index'th item.
	     */
	    start = 1+embPatPosMatchGetStart(match, i);
	    if (outf)
	      ajFmtPrintF(outf, "Start of match = position %d of sequence\n",
			  start);			
	
	    /* returns the end point for the pattern match for the
	     * index'th item.
	     */
	    end	= 1+embPatPosMatchGetEnd(match, i);
	    if (outf)
	      ajFmtPrintF(outf, "End of match = position %d of sequence\n\n",
			  end);

	    gf = ajFeatNew (tab, NULL, fthit, start, end,
			    (float) length, ' ', 0);

	    if (outf)
	      ajFmtPrintF(outf,
			  "patmatDB of %s from %d to %d using pattern %s\n\n",
			  ajSeqName(seq), start, end, ajStrStr(motif));
		

	    if(start-5<0)
	    {
	        if (outf)
	        {
		    for(j=0;j<5-start;++j) ajFmtPrintF(outf," ");
	        }
		zstart=0;
	    }
	    else zstart=start-6;
	    
	    if (end+4> seqlength)
		zend = end;
	    else zend = end+4; 


				
	    ajStrAssSub(&temp, str, zstart, zend);
	    if (outf)
	    {
	      ajFmtPrintF(outf, "%s\n", ajStrStr(temp));
		
	      ajFmtPrintF(outf, "     |");
	      patmatdb_spaces(&outf, length);
	      ajFmtPrintF(outf, "|\n");

	      ajFmtPrintF(outf, "%6d", start);
	      patmatdb_spaces(&outf, length);
	      ajFmtPrintF(outf, "%-d\n\n\n", end);
	    }
	}
	if (number)
        {
	    ajReportWrite (report, tab, seq);
	    ajFeattableDel(&tab);
	}
	embPatMatchDel(&match);
    }	


    ajStrDel(&regexp);
    ajStrDel(&temp);    
    ajStrDel(&motif);
    ajStrDel(&str);
    ajStrDel(&regexpdata);

    ajReportClose(report);
    ajReportDel (&report);

    ajExit();
    return 0;
}

/* @funcstatic patmatdb_spaces ************************************************
**
** add spaces under sequence between the count bars
**
** @param [w] outf [AjPFile*] outfile
** @param [r] length [ajint] length+2
** @@
******************************************************************************/

static void patmatdb_spaces(AjPFile *outf, ajint length)
{
    ajint i;
    if (!outf) return;


    for (i=0; i < length-2; ++i)
	ajFmtPrintF(*outf, " ");
}

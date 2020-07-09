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


void spaces(AjPFile *outf, int length);



int main (int argc, char *argv[] )
{
    AjPFile outf 		=NULL;	

    AjPSeqall seqall 		=NULL;
    AjPSeq seq			=NULL;
    AjPStr str 			=NULL;	
    AjPStr regexp 		=NULL;
    AjPStr motif		=NULL;
    AjPStr temp = NULL;
    
    AjPStr regexpdata	=NULL;
    EmbPPatMatch match 	=NULL;
	
	
    int i;
    int number;
    int start;
    int end;
    int length;
    int zstart;
    int zend;    
    int seqlength;
    int j;
	
    embInit ("patmatdb", argc, argv);
	
    seqall	= ajAcdGetSeqall ("sequence");
    motif 	= ajAcdGetString ("motif");
    outf        = ajAcdGetOutfile("outfile");

    temp=ajStrNew();
    

    /*converting the Prosite motif to a reg exps */		
    regexp =embPatPrositeToRegExp(&motif);
   


    while (ajSeqallNext(seqall, &seq))
    {
	str = ajSeqStr(seq); 
	
	/* comparing the reg exps to sequence for matches. */
	match 	= embPatPosMatchFind(regexp, str);

	/*returns the number of posix matches in the structure. */
	number 	= embPatPosMatchGetNumber(match);
	if(number)
	    ajFmtPrintF(outf,"\nNumber of matches found in %s = %d\n",
			ajSeqName(seq), number);

	for (i=0; i<number; i++)
	{
	    seqlength = ajStrLen(str);
	    ajFmtPrintF(outf, "Length of the sequence = %d basepairs\n",
			seqlength);

	    /*returns length from pattern match for index'th item. */
	    length = embPatPosMatchGetLen(match, i);
	    ajFmtPrintF(outf, "Length of match = %d\n", length);

	    /*
	     * returns the start position from the pattern match for the
             * index'th item.
	     */
	    start = embPatPosMatchGetStart(match, i);
	    ajFmtPrintF(outf, "Start of match = position %d of sequence\n",
			start);			
	
	    /* returns the end point for the pattern match for the
	     * index'th item.
	     */
	    end	= embPatPosMatchGetEnd(match, i);
	    ajFmtPrintF(outf, "End of match = position %d of sequence\n\n", end);
				
	    
	    ajFmtPrintF(outf,
			"patmatDB of %s from %d to %d using pattern %s\n\n",
			ajSeqName(seq), start, end, ajStrStr(motif));
		

	    if(start-5<0)
	    {
		for(j=0;j<5-start;++j) ajFmtPrintF(outf," ");
		zstart=0;
	    }
	    else zstart=start-5;
	    
	    if (end+5> seqlength)
		zend = end;
	    else zend = end+5; 


				
	    ajStrAssSub(&temp, str, zstart, zend);
	    ajFmtPrintF(outf, "%s\n", ajStrStr(temp));
		
		
	    ajFmtPrintF(outf, "     |");
	    spaces(&outf, length);
	    ajFmtPrintF(outf, "|\n");


	    ajFmtPrintF(outf, "%6d", start);
	    spaces(&outf, length);
	    ajFmtPrintF(outf, "%-d\n\n\n", end);
	}

    }	


    ajStrDel(&regexp);
    ajStrDel(&temp);    
    ajStrDel(&motif);
    ajStrDel(&str);
    ajStrDel(&regexpdata);
    ajExit();
    return 0;
}



/* spaces - add spaces under sequence between the count bars. */

void spaces(AjPFile *outf, int length)
{
    int i;

    for (i=0; i < length-2; ++i)
	ajFmtPrintF(*outf, " ");
}

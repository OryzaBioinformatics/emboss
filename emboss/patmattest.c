/* @source patmattest.c
**
** General test routine for pattern matching.
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

static char *testset[] =
{
    "GAN","GAATTC","CCSGG","GANTC","GABNNNNNVTC","GA", "GANN","TC"
};

/* @prog patmettest *******************************************************
**
** Testing
**
******************************************************************************/
 
int main(int argc, char **argv)
{
    AjPStr cut = NULL;
    AjPStr new = NULL;
    AjPStr test = NULL;
    AjPStr regexp = NULL;
    EmbPPatMatch results = NULL;
    AjPSeq seq;
    ajint i;
    ajint j;

    embInit ("patmattest", argc, argv);

    ajStrAssC (&test,"GAATTCCCGGAGATTCCGACTC");


    for(i=0;i<8;i++)
    {
	ajStrAssC (&cut,testset[i]);

	/* Create the regular expression form the plain text */
	regexp = embPatSeqCreateRegExp(cut,0);    

	/* find the matches */
	results = embPatMatchFind(regexp,test);

    
	ajUser("01234567890123456789012345");
	ajUser("%S",test);
	ajUser("%S %S",cut,regexp);
	ajUser("%d matches found",results->number);
	for(j=0;j<results->number;j++)
	    ajUser("start = %d len = %d",results->start[j],results->len[j]);
	ajUser(" ");
	embPatMatchDel(&results);
	ajStrDel(&regexp);
	ajStrDel(&cut);
    }
    ajStrDel(&test);

    seq = ajAcdGetSeq ("sequence1");

    cut = ajAcdGetString("expression");

    results = embPatSeqMatchFind(seq, cut);
    ajUser("%S",cut);
    ajUser("%d matches found",results->number);
    for(j=0;j < embPatMatchGetNumber(results) ;j++)
    {
	ajUser("start = %d len = %d",embPatMatchGetStart(results,j),
	       embPatMatchGetLen(results,j));
	/*get a copy off the string */
	new = ajStrNewL(results->len[j]);
	ajStrAssSub(&new,ajSeqStr(seq),embPatMatchGetStart(results,j),
		    embPatMatchGetEnd(results,j));
	ajUser("%S",new);
	ajStrDel(&new);
    }

    ajUser(" ");
    embPatMatchDel(&results);
    ajStrDel(&cut);
    ajSeqDel(&seq);

    ajExit();
    return 0;
}

#include "emboss.h"
#include "ajtable.h"



int main(int argc, char **argv)
{

  AjPSeq seq;
  AjPTable table =0 ;
  AjPFile outf;
  ajint wordsize;

  embInit("wordcount", argc, argv);

  seq = ajAcdGetSeq ("sequence1");
  
  wordsize = ajAcdGetInt ("wordsize");
  outf = ajAcdGetOutfile ("outfile");

  embWordLength (wordsize);

  if(embWordGetTable(&table, seq)) /* get table of words */
     { 
       embWordPrintTableF(table, outf);              /* print table of words */
       /*test if you can add to table        if(getWordTable(&table, seq, wordcount)) ?? get table of words ??
	 { 
	   printWordTable(table);              ?? print table of words ??
	 }  */
       embWordFreeTable(table);               /* free table of words */
     }  
  else{
    ajFatal("ERROR generating word table");
  }

  ajExit();
  return 0;
}

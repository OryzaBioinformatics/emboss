/* @source proteinmotifsearch.c
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

/*
e.g.
proteinmotifsearch embl:M94271 /nfs/disk100/pubseq/prosite/prosite.regex

get protein.
convert to upper case.

open motif file and read each motif.
convert to upper case.

comp reg exp.

search regcomp with string.
*/

int main(int argc, char **argv)
{
  EmbPPatMatch results = NULL;
  AjPSeq seq = NULL;
  AjPStr str = NULL;
  AjPStr motiffilename = NULL;
  AjPFile motiffile = NULL;
  AjPStr line = NULL;
  ajint j,pos;
  char *sptr,*sptr2;
  char temp;

  embInit ("proteinmotifsearch", argc, argv);

  seq = ajAcdGetSeq ("sequence1");
  ajStrAss(&str, ajSeqStr(seq));
  ajStrToUpper(&str);

  motiffilename = ajAcdGetString("motifs");

  motiffile = ajFileNewIn(motiffilename);

  while( ajFileReadLine(motiffile, &line) ){
    pos = ajStrFindC(line," ");
    ajStrCut(&line,0,pos);
    pos = ajStrFindC(line," ");
    ajStrCut(&line,pos,ajStrLen(line)-1);
    ajStrToUpper(&line);
    results = embPatMatchFind(line,str);
    for(j=0;j<results->number;j++){
      if(j==0){
	ajUser("%S----------------------",line);
      }
      ajUser("     start = %d len = %d",results->start[j],results->len[j]);
      sptr = ajStrStr(str) + results->start[j];
      sptr2 = sptr+results->len[j];
      temp = *sptr2;
      *sptr2 = '\0';
      ajUser("%s",sptr);
      *sptr2 = temp;
    }
  }
  ajExit();
  return 0;
}

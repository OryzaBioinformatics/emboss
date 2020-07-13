/* @source sixpack application
**
** Display a DNA sequence in both direction with its translation
**
** @author: Copyright (C) Thomas Laurent (thomas.laurent@uk.lionbioscience.com)
** 30 Sept 2002
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


/* declare functions */

static int sixpack_findorfs(AjPSeqout *outseq, AjPFile *outf, ajint s,
			    ajint len, char *seq, char *name, ajint orfml, 
			    AjBool addedasterisk, AjBool firstorf, ajint frame, 
			    char *origname, AjBool mstart);

static void sixpack_ajprintseq(AjPSeqout *outseq, char *seq, ajint begin,
			       int end, ajint orflength, char *name, ajint count, 
			       ajint frame, char *origname, ajint min_orflength);

/* @prog sixpack ****************************************************************
**
** Display a DNA sequence in both direction with its translation
**
******************************************************************************/

int main(int argc, char **argv)
{

  ajint begin, end, pepbegin, pepend, peplen;
  AjPSeq seq;
  AjPSeq pep;
  AjPStr pepseq = NULL;
  AjPStr substr = NULL;
  EmbPShow ss;
  AjPFile outfile;
  AjPSeqout outseq=NULL;
  AjPStr * tablelist;
  ajint table;
  AjPRange uppercase;
  AjPRange highlight;
  AjBool numberseq;
  AjBool nameseq;
  ajint width;
  ajint length;
  ajint margin;
  AjBool description;
  ajint offset;
  AjBool html;
  AjPStr descriptionline;
  ajint orfminsize;
  AjPTrn trnTable;
  AjBool reverse;
  AjBool addedasterisk = ajFalse;
  AjBool addlast = ajTrue;
  AjBool firstorf = ajTrue;
  AjBool mstart = ajFalse;
  ajint totalorf = 0;
  ajint totalframes= 3;

  int i;

  (void) embInit ("sixpack", argc, argv);

  seq = ajAcdGetSeq ("sequence");
  outfile = ajAcdGetOutfile ("outfile");
  outseq    = ajAcdGetSeqoutall("outseq");
  tablelist = ajAcdGetList ("table");
  uppercase = ajAcdGetRange ("uppercase");
  highlight = ajAcdGetRange ("highlight");
  numberseq = ajAcdGetBool ("number");
  width = ajAcdGetInt ("width");
  length = ajAcdGetInt ("length");
  margin = ajAcdGetInt ("margin");
  nameseq = ajAcdGetBool ("name");
  description = ajAcdGetBool ("description");
  offset = ajAcdGetInt ("offset");
  html = ajAcdGetBool ("html");
  orfminsize = ajAcdGetInt ("orfminsize");
  reverse = ajAcdGetBool ("reverse");
  addlast = ajAcdGetBool ("lastorf");
  firstorf = ajAcdGetBool ("firstorf");
  mstart = ajAcdGetBool ("mstart");


  /* get the number of the genetic code used */
  (void) ajStrToInt(tablelist[0], &table);
  trnTable = ajTrnNewI(table);

  /* get begin and end positions */
  begin = ajSeqBegin(seq)-1;
  end = ajSeqEnd(seq)-1;

  /* do the name and description */
  if (nameseq)
    {
      if (html)
	(void) ajFmtPrintF(outfile, "<H2>%S</H2>\n",
			   ajSeqGetName(seq));
      else
		(void) ajFmtPrintF(outfile, "%S\n", ajSeqGetName(seq)); 
    }

  if (description)
    {
      /*
       *  wrap the description line at the width of the sequence
       *  plus margin
       */
      if (html)
	(void) ajFmtPrintF(outfile, "<H3>%S</H3>\n",
			   ajSeqGetDesc(seq));
      else
	{
	  descriptionline = ajStrNew();
	  (void) ajStrAss(&descriptionline, ajSeqGetDesc(seq));
	  (void) ajStrWrap(&descriptionline, width+margin);
	  (void) ajFmtPrintF(outfile, "%S\n", descriptionline);
	  (void) ajStrDel(&descriptionline);
	}
    }


  /* make the Show Object */
  ss = embShowNew(seq, begin, end, width, length, margin, html, offset);

  if (html)
    (void) ajFmtPrintF(outfile, "<PRE>");

  /* create the format to display */

  if (reverse) 
    (void) embShowAddBlank(ss);
  (void) embShowAddBlank(ss);
  
  
  (void) embShowAddTran (ss, trnTable, 1, AJFALSE, AJFALSE,
			 NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
  (void) embShowAddTran (ss, trnTable, 2, AJFALSE, AJFALSE,
			 NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
  (void) embShowAddTran (ss, trnTable, 3, AJFALSE, AJFALSE,
			 NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);

  /*	(void) embShowAddBlank(ss);*/
  (void) embShowAddSeq(ss, numberseq, AJFALSE, uppercase, highlight);

  if (!numberseq)
    (void) embShowAddTicknum(ss);
  (void) embShowAddTicks(ss);
  
  if (reverse)
    {
      (void) embShowAddComp(ss, numberseq);
    }
  
  if (reverse)
    {
      totalframes = 6;
      
      (void) embShowAddTran (ss, trnTable, -3, AJFALSE,
			     AJFALSE, NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
      (void) embShowAddTran (ss, trnTable, -2, AJFALSE,
			     AJFALSE, NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
      (void) embShowAddTran (ss, trnTable, -1, AJFALSE,
			     AJFALSE, NULL, orfminsize, AJTRUE, firstorf, addlast, AJTRUE);
    }
  
  (void) embShowPrint(outfile, ss);


  /* add a gratuitous newline at the end of the sequence */
  (void) ajFmtPrintF(outfile, "\n");

  /* tidy up */
  (void) embShowDel(&ss);

  /* Print the footer */
  (void) ajFmtPrintF(outfile, "##############################\n");
  (void) ajFmtPrintF(outfile, "Minimum size of ORFs : %d\n\n", orfminsize);

  /* Write ORFs in a separate file */

  for (i=0; i<totalframes; i++) {
    ajDebug("try frame: %d\n", i);
    if (i<3)
      pep = ajTrnSeqOrig(trnTable, seq, i+1);
    else	/* frame -1 uses frame 1 codons */
      pep = ajTrnSeqOrig(trnTable, seq, 2-i);
	  
    pepbegin = ajSeqBegin(pep)-1;
    pepend = ajSeqEnd(pep)-1;
    pepseq = ajSeqStr(pep);

    ajStrAssSubC(&substr,ajStrStr(pepseq),pepbegin,pepend);

    /* end with a '*' if we want to and there is not one there already */
    ajDebug("last residue =%c\n", ajSeqChar(pep)[pepend]);
    if (addlast && ajSeqChar(pep)[pepend] != '*') {
      ajStrAppK(&substr,'*');
      addedasterisk = ajTrue;
    }
    ajDebug("After appending, sequence=%S\n", substr);
    ajStrToUpper(&substr);
	  
    peplen=ajStrLen(substr);

    totalorf += sixpack_findorfs(&outseq, &outfile, 0, peplen, ajStrStr(substr),
				 ajSeqName(pep), orfminsize, addedasterisk, firstorf,
				 i+1, ajSeqName(seq), mstart);

	  
    /*	  (void) ajSeqAllWrite (outseq, pep);*/
    (void) ajSeqDel (&pep);
  }
  ajFmtPrintF(outfile,"\nTotal ORFs : %5d\n",totalorf);
  (void) ajFmtPrintF(outfile, "##############################\n\n");

  /* tidy up */
  ajTrnDel (&trnTable);

  ajExit ();
  return 0;
}


/* @funcstatic sixpack_findorfs **********************************************
**
** Finds ORFs and prints report
**
** @param [?] outseq [AjPSeqout*] File where to write fasta sequences
** @param [?] outf [AjPFile*] File where to write the report on ORFs 
** @param [?] from [ajint] 0
** @param [?] to [ajint] Length of the sequence
** @param [?] p [char*] Sequence
** @param [?] name [char*] Name of the translated sequence (with frame number)
** @param [?] min_orflength [ajint] Minimum size of the ORFs to report
** @param [r] addedasterisk [AjBool] True if an asterisk was added at the end
** @param [?] frame [ajint] Frame number
** @param [?] origname [char*] Original name of the sequence (DNA)
** @@
******************************************************************************/

static int sixpack_findorfs (AjPSeqout *outseq, AjPFile *outf, ajint from,
			     ajint to, char *p, char *name, ajint min_orflength,
			     AjBool addedasterisk, AjBool firstorf, ajint frame, 
			     char *origname, AjBool mstart)

{
    ajint i;
    ajint j;
    ajint last_stop = 0;
    ajint orflength = 0;
    ajint orfnb = 0;
    
    for (i=from;i<to;++i)
      {
	if(p[i]=='*')
	  {
	    if (!mstart)
	      orflength=i-last_stop;
	    else 
	      {
		orflength=0;
		for (j=last_stop;j<i;j++)
		  {
		    if(p[j]=='M')
		      {
			orflength=i-j;
			break;
		      }
		  }
	      }
	    if (orflength >= min_orflength)
	      {
		sixpack_ajprintseq(outseq, p,i-orflength,i-1,orflength,name,orfnb+1,frame,origname,min_orflength);
		orfnb++;
	      }
	    else if ((last_stop == 0) && firstorf && p[0] != '*')
	      {
		if (mstart)
		  orflength=i-last_stop;
		if (orflength > 0) {
		  sixpack_ajprintseq(outseq, p,i-orflength,i-1,orflength,name,orfnb+1,frame,origname,min_orflength);
		  orfnb++;
		}
	      }
	    else if ((i == to-1) && addedasterisk)
	      {
		/*		if (mstart)
				orflength=i-last_stop; */
		if (orflength > 0) {
		  sixpack_ajprintseq(outseq, p,i-orflength,i-1,orflength,name,orfnb+1,frame,origname,min_orflength);
		  orfnb++;
		}
	      }

	    
	    last_stop = ++i;
	    
            while (p[i] == '*')
	      {	/* check to see if we have consecutive ****s */
		last_stop = ++i;
	      }
	  }
      }
    
    ajFmtPrintF(*outf,"Total ORFs in frame %d : %5d\n", frame, orfnb);
    
    return orfnb;
}




/* @funcstatic sixpack_ajprintseq *********************************************
**
** Prints ORFs in the sequence file
**
** @param [?] outseq [AjPSeqout*] File where to write fasta sequences
** @param [?] seq [char*] Sequence to write
** @param [?] begin [ajint] Start position of the ORF to write
** @param [?] end [int] End position of the ORF to write
** @param [?] orflengtht [ajint] Size of the current ORF
** @param [?] name [char*] Name of the translated sequence (with frame number)
** @param [?] count [ajint] Number of the ORF to be written in this frame
** @param [?] frame [ajint] Frame number
** @param [?] origname [char*] Original name of the sequence (DNA)
** @param [?] min_orflength [ajint] Minimum size for an ORF
** @@
******************************************************************************/



static void sixpack_ajprintseq(AjPSeqout *outseq, char *seq, ajint begin, int
			       end, ajint orflength, char *name, ajint count, ajint frame, 
			       char *origname, ajint min_orflength)
{
    AjPSeq sq;
    AjPStr str;
    AjPStr nm;

    sq   = ajSeqNew();
    str  = ajStrNew();
    nm   = ajStrNew();

    ajStrAssSubC(&str,seq,begin,end);
    ajSeqReplace(sq,str);

    ajFmtPrintS(&nm,"%s_ORF%d  Translation of %s in frame %d, ORF %d, threshold %d, %daa",name,count,origname,frame,count,min_orflength,orflength);
    ajSeqAssName(sq,nm);

    ajSeqWrite(*outseq, sq);

    ajStrDel(&nm);
    ajStrDel(&str);
    ajSeqDel(&sq);

    return;
}





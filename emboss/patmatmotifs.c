/* @source patmatmotifs.c
** @author: Copyright (C) Sinead O'Leary (soleary@hgmp.mrc.ac.uk) 
** @@
** Application for pattern matching, a sequence against a database of motifs.
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
   
void spaces(AjPFile *outf, ajint length); 
  
int main(int argc, char **argv)
{

    AjPFile inf	      = NULL;
    AjPFile inf2      = NULL;
    AjPFile outf      = NULL;

    AjPSeq sequence   = NULL;

    AjPStr redatanew  = NULL;
    AjPStr str	      = NULL;
    AjPStr regexp     = NULL;
    AjPStr temp	      = NULL;
    AjPStr text       = NULL;
    AjPStr docdata    = NULL;
    AjPStr data       = NULL;
    AjPStr accession  = NULL; 
    AjPStr name       = NULL;
    EmbPPatMatch match = NULL;
    AjPStr savereg=NULL;
    
    AjBool full;
    AjBool prune;
    
    ajint i;
    ajint number;
    ajint start;
    ajint end;
    ajint length;
    ajint zstart;
    ajint zend;
    char *p;
    ajint seqlength;
    ajint j;


    embInit ("patmatmotifs", argc, argv);



    savereg = ajStrNew();
    str = ajStrNew();
    regexp = ajStrNew();
    temp = ajStrNew();
    data = ajStrNew();
    accession = ajStrNew();
    text = ajStrNew();
    name = ajStrNew();
    
    sequence = ajAcdGetSeq("sequence");
    outf     = ajAcdGetOutfile("outfile");
    full     = ajAcdGetBool("full");
    prune    = ajAcdGetBool("prune");
    
    seqlength = ajStrLen(str);
    str       = ajSeqStrCopy(sequence);
    
    redatanew = ajStrNewC("PROSITE/prosite.lines");
    docdata   = ajStrNewC("PROSITE/");

    ajFileDataNew(redatanew, &inf);
    if(!inf)
	ajFatal("Either EMBOSS_DATA undefined or PROSEXTRACT needs running");
    

    while (ajFileReadLine(inf, &regexp))
    {
	p=ajStrStr(regexp);
	if(*p && *p!=' ' && *p!='^')
	{
	    p=strtok(p," ");
	    ajStrAssC(&name,p);
	    if(prune)
		if(ajStrMatchCaseC(name,"myristyl") ||
		   ajStrMatchCaseC(name,"asn_glycosylation") ||
		   ajStrMatchCaseC(name,"camp_phospho_site") ||
		   ajStrMatchCaseC(name,"pkc_phospho_site") ||
		   ajStrMatchCaseC(name,"ck2_phospho_site") ||
		   ajStrMatchCaseC(name,"tyr_phospho_site"))
		{
		    for(i=0;i<4;++i)
			ajFileReadLine(inf, &regexp);
		    continue;
		}
	    p=strtok(NULL," ");
	    ajStrAssC(&accession,p); 	    
	}

	if (ajStrPrefixC(regexp, "^"))
	{
	    p = ajStrStr(regexp);
	    
	    ajStrAssC(&temp,p+1);
	    ajStrAssC(&savereg,p+1);

	    match = embPatPosMatchFind(temp, str);
	    number = embPatPosMatchGetNumber(match); 
	    
	    if(number)
		ajFmtPrintF(outf, 
		      "\nNumber of matches found in this Sequence = %d\n\n",
		      number);
	
	
	    for (i=0; i<number; i++)
	    {
		seqlength = ajStrLen(str);
		ajFmtPrintF(outf,
			   "Length of the sequence = %d basepairs\n",
			    seqlength);
			
		start = embPatPosMatchGetStart(match, i); 
		ajFmtPrintF(outf,
			    "Start of match = position %d of sequence\n",
			    start+1);
	
		end = embPatPosMatchGetEnd(match, i);  		
		ajFmtPrintF(outf, 
			    "End of match = position %d of sequence\n", end+1);
		
		length = embPatPosMatchGetLen(match, i);
		ajFmtPrintF(outf, "Length of motif = %d\n\n", length);
	



		ajFmtPrintF(outf, 
		     "patmatmotifs of %s with %s from %d to %d\n", 
		      ajStrStr(name), ajSeqName(sequence), start+1, end+1);


		if(start-5<0)
		{
		    for(j=0;j<5-start; ++j) ajFmtPrintF(outf," ");
		    zstart = 0;
		}
		else zstart = start-5;
		
		if (end+5> seqlength)
		    zend = end;
		else zend = end+5; 
	
			
		ajStrAssSub(&temp, str, zstart, zend);
		ajFmtPrintF(outf, "%s\n", ajStrStr(temp));
		
		ajFmtPrintF(outf, "     |");
		spaces(&outf, length);
		ajFmtPrintF(outf, "|\n");
	    
		ajFmtPrintF(outf, "%6d", start+1);
		spaces(&outf, length);
		ajFmtPrintF(outf, "%-d\n\n", end+1);
	    }
	

	    if(full && number)
	    {
		ajStrAssC(&redatanew,ajStrStr(docdata));
		ajStrAppC(&redatanew,ajStrStr(accession));
		ajFileDataNew(redatanew, &inf2);
		if(!inf2)
		    continue;
		
		/*
		 * Insert Prosite documentation from files made by
		 * prosextract.c
		 */
		while (ajFileReadLine(inf2, &text))
		    ajFmtPrintF(outf, "%s\n", ajStrStr(text));
		ajFmtPrintF(outf, "***************\n");
		ajFileClose(&inf2);

	    }
	embPatMatchDel(&match);
	}
    }
    
    ajStrDel(&temp);
    ajStrDel(&regexp);
    ajStrDel(&savereg);
    ajStrDel(&str);
    ajStrDel(&data);
    ajStrDel(&docdata);
    ajStrDel(&text);
    ajStrDel(&redatanew);
    ajStrDel(&accession);
    ajSeqDel(&sequence);

    
    ajFileClose(&inf);
    ajFileClose(&outf);

    ajExit();
    return 0;
}



/* Functions */

void spaces(AjPFile *outf, ajint length)
{
    ajint i;

    for (i=0; i < length-2; ++i)
	ajFmtPrintF(*outf, " ");
}

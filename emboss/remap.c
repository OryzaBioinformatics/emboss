/* @source remap application
**
** Display a sequence with restriction cut sites
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** 18 Jan 2000 - GWW - written
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
static void read_equiv(AjPFile *equfile, AjPTable *table);
static void CutList (AjPFile outfile, AjPList restrictlist, AjBool isos,
	AjBool html);
static void NoCutList (AjPFile outfile, AjPList restrictlist, AjBool
	html, AjPStr enzymes, AjBool blunt, AjBool sticky, int sitelen);
static void read_file_of_enzyme_names(AjPStr *enzymes);

/* structure for counts and isoschizomers of a restriction enzyme hit */
typedef struct SValue {
    int    count;
    AjPStr iso;
} OValue, *PValue;


int main (int argc, char * argv[]) {

  int begin, end;
  AjPSeqall seqall;
  AjPSeq seq;
  EmbPShow ss;
  AjPFile outfile;
  AjPStr * tablelist;
  int table;
  AjPRange uppercase;
  AjPRange highlight;
  AjBool threeletter;
  AjBool numberseq;
  AjBool nameseq;
  int width;
  int length;
  int margin;
  AjBool description;
  int offset;
  AjBool html;
  AjPStr descriptionline;
  AjPFeatTable feat;
  int orfminsize;
  AjPTrn trnTable;
  AjBool translation;
  AjBool reverse;
  AjBool cutlist;
  AjBool flat;
  EmbPMatMatch mm=NULL;
    
/* stuff lifted from Alan's 'restrict.c' */
    AjPStr    enzymes=NULL;
    int mincuts;
    int maxcuts;
    int sitelen;
    AjBool single;
    AjBool blunt;
    AjBool sticky;
    AjBool ambiguity;
    AjBool plasmid;
    AjBool commercial;
    AjBool limit;
    AjBool equiv;
    AjPFile   enzfile=NULL;
    AjPFile   equfile=NULL;
    AjPTable  retable=NULL;
    int       hits;
    AjPList     restrictlist=NULL;
#define ENZDATA "REBASE/embossre.enz"
#define EQUDATA "embossre.equ"
#define EQUGUESS 3500     /* Estimate of number of equivalent names */
                                                                                                     


  (void) embInit ("remap", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  outfile = ajAcdGetOutfile ("outfile");
  tablelist = ajAcdGetList ("table");
  uppercase = ajAcdGetRange ("uppercase");
  highlight = ajAcdGetRange ("highlight");
  threeletter = ajAcdGetBool ("threeletter");
  numberseq = ajAcdGetBool ("number");
  width = ajAcdGetInt ("width");
  length = ajAcdGetInt ("length");
  margin = ajAcdGetInt ("margin");
  nameseq = ajAcdGetBool ("name");
  description = ajAcdGetBool ("description");
  offset = ajAcdGetInt ("offset");
  html = ajAcdGetBool ("html");
  orfminsize = ajAcdGetInt ("orfminsize");
  translation = ajAcdGetBool ("translation");
  reverse = ajAcdGetBool ("reverse");
  cutlist = ajAcdGetBool ("cutlist");
  flat = ajAcdGetBool ("flatreformat");

/*  restriction enzyme stuff */
  mincuts = ajAcdGetInt ("mincuts");
  maxcuts = ajAcdGetInt ("maxcuts");
  sitelen  = ajAcdGetInt ("sitelen");
  single = ajAcdGetBool ("single");
  blunt = ajAcdGetBool ("blunt");
  sticky = ajAcdGetBool ("sticky");
  ambiguity = ajAcdGetBool ("ambiguity");
  plasmid = ajAcdGetBool ("plasmid");
  commercial = ajAcdGetBool ("commercial");
  limit = ajAcdGetBool ("limit");
  enzymes = ajAcdGetString ("enzymes");
  equiv = ajAcdGetBool("preferred");
    

/* get the number of the genetic code used */
  (void) ajStrToInt(tablelist[0], &table);
  trnTable = ajTrnNewI(table);

/* read the local file of enzymes names */
  read_file_of_enzyme_names(&enzymes);

  while (ajSeqallNext(seqall, &seq)) {

/* get begin and end positions */
    begin = ajSeqBegin(seq)-1;
    end = ajSeqEnd(seq)-1;

/* do the name and description */
    if (nameseq) {
      if (html) {
        (void) ajFmtPrintF(outfile, "<H2>%S</H2>\n", ajSeqGetName(seq));
      } else {
        (void) ajFmtPrintF(outfile, "%S\n", ajSeqGetName(seq));
      }
    }
    if (description) {
/* wrap the description line at the width of the sequence plus margin */
      if (html) {
        (void) ajFmtPrintF(outfile, "<H3>%S</H3>\n", ajSeqGetDesc(seq));
      } else {
        descriptionline = ajStrNew();
        (void) ajStrAss(&descriptionline, ajSeqGetDesc(seq));
        (void) ajStrWrap(&descriptionline, width+margin);
        (void) ajFmtPrintF(outfile, "%S\n", descriptionline);
        (void) ajStrDel(&descriptionline);
      }
    }

/* get the feature table of the sequence */
    feat = ajSeqGetFeat(seq);

/* get the restriction cut sites */
/* most of this is lifted from the program 'restrict.c' by Alan Bleasby */    
    if (single) maxcuts=mincuts=1;
    retable = ajStrTableNew(EQUGUESS);
    ajFileDataNewC(ENZDATA, &enzfile);
    if(!enzfile)
        ajFatal("Cannot locate enzyme file. Run REBASEEXTRACT");
    if (equiv) {
      ajFileDataNewC(EQUDATA,&equfile);
      if (!equfile) equiv=ajFalse;
      else read_equiv(&equfile, &retable);   
    }    

    ajFileSeek(enzfile, 0L, 0);
    hits = embPatRestrictMatch(seq, begin+1, end+1, enzfile, enzymes,
			       sitelen,plasmid, ambiguity, mincuts, maxcuts,
			       blunt, sticky, commercial, &restrictlist);
    if (hits) {
/* this bit is lifted from printHits() */
      (void) embPatRestrictRestrict(&restrictlist, hits, !limit, ajFalse);
    }
/* tidy up */
    ajFileClose(&enzfile);      


/* make the Show Object */
    ss = embShowNew(seq, begin, end, width, length, margin, html, offset);

    if (html) (void) ajFmtPrintF(outfile, "<PRE>");

/* create the format to display */
    (void) embShowAddBlank(ss);
    (void) embShowAddRE (ss, 1, restrictlist, flat);
    (void) embShowAddSeq(ss, numberseq, threeletter, uppercase, highlight);
    if (!numberseq) (void) embShowAddTicknum(ss);
    (void) embShowAddTicks(ss);
    if (reverse) {
      (void) embShowAddComp(ss, numberseq);
      (void) embShowAddRE (ss, -1, restrictlist, flat);
    }
    if (translation) {
      if (reverse) (void) embShowAddBlank(ss);
      (void) embShowAddTran (ss, trnTable, 1, threeletter, numberseq,
			     NULL, orfminsize);
      (void) embShowAddTran (ss, trnTable, 2, threeletter, numberseq,
			     NULL, orfminsize);
      (void) embShowAddTran (ss, trnTable, 3, threeletter, numberseq,
			     NULL, orfminsize);
      if (reverse) {
        (void) embShowAddTicks(ss);
        (void) embShowAddTran (ss, trnTable, -3, threeletter, numberseq,
			       NULL, orfminsize);
        (void) embShowAddTran (ss, trnTable, -2, threeletter, numberseq,
			       NULL, orfminsize);
        (void) embShowAddTran (ss, trnTable, -1, threeletter, numberseq,
			       NULL, orfminsize);
      }
    }

    (void) embShowPrint(outfile, ss);

/* display a list of the Enzymes that cut and don't cut */
    if (cutlist) {
      CutList(outfile, restrictlist, limit, html);
      NoCutList(outfile, restrictlist, html, enzymes, blunt, sticky, sitelen);
    }

/* tidy up */
    (void) embShowDel(&ss);
    (void) ajFeatTabDel(&feat);
    while(ajListPop(restrictlist,(void **)&mm))
	embMatMatchDel(&mm);
    (void) ajListDel(&restrictlist);



/* add a gratuitous newline at the end of the sequence */
    (void) ajFmtPrintF(outfile, "\n");

    if (html) {
    	(void) ajFmtPrintF(outfile, "<PRE>");
    }
  }

/* tidy up */
  ajTrnDel (&trnTable);

  


  (void) ajExit ();
  return 0;
}



/* @funcstatic CutList ********************************************************
**
** display a list of the enzymes that cut
**
** @param [r] outfile [AjPFile] file to print to.
** @param [r] restrictlist [AjPList] List to print.
** @param [r] isos [AjBool] True if allow isoschizomers
** @param [r] html [AjBool] dump out html if true.
** @return [void]
** @@
******************************************************************************/

static void CutList (AjPFile outfile, AjPList restrictlist, AjBool isos,
	AjBool html) {
	

  AjPTable table = ajStrTableNewCase (200);
  PValue value;
  AjPStr key;
  AjIList miter;	/* iterator for matches list */
  EmbPMatMatch m=NULL;	/* restriction enzyme match structure */
  void **array;		/* array for table */  
  int i;

/* print title */
  if (html) (void) ajFmtPrintF(outfile, "<H2>");  
  (void) ajFmtPrintF(outfile, "\n\n# Enzymes that cut  Frequency");
  if (isos) {
    (void) ajFmtPrintF(outfile, "\tIsoschizomers\n");
  } else {
    (void) ajFmtPrintF(outfile, "\n");
  }
  if (html) (void) ajFmtPrintF(outfile, "</H2>");  


  miter = ajListIter(restrictlist);
  while ((m = ajListIterNext(miter)) != NULL) {
    
    key = m->cod;

/* debug */
/* (void) ajFmtPrintF(outfile, "%S =|= %S\n", key, m->iso); */


/* increment the count of key */
    value = (PValue) ajTableGet(table, (const void *)key);
    if (value == NULL) {
      AJNEW0(value);
      value->count = 1;          	
      value->iso = ajStrNew();
      ajStrAss(&(value->iso), m->iso);
    } else {
      value->count++;
    }
    ajTablePut(table, (const void *)key, (void *)value);
/* debug
    (PValue) value = ajTableGet(table, (const void *)key);
    ajDebug("key=%S value=%d", key, value->count);
*/
  }
  (void) ajListIterFree(miter);

/* print out results */
  if (html) {(void) ajFmtPrintF(outfile, "<PRE>");} 
  array = ajTableToarray(table, NULL);
  qsort(array, ajTableLength(table), 2*sizeof (*array), ajStrCmp);
  for (i = 0; array[i]; i += 2) {
    value = (PValue) array[i+1];
    (void) ajFmtPrintF (outfile, "%10S\t    %d\t%S\n", (AjPStr) array[i], value->count, value->iso);
    ajStrDel(&(value->iso));
    AJFREE(array[i+1]);		/* free the int* value */
  }  
  AJFREE(array);
  (void) ajFmtPrintF (outfile, "\n");
  if (html) {(void) ajFmtPrintF(outfile, "</PRE>");} 

/* tidy up */
  ajTableFree (&table);

}


/* @funcstatic NoCutList ********************************************************
**
** display a list of the enzymes that do NOT cut
**
** @param [r] outfile [AjPFile] file to print to.
** @param [r] restrictlist [AjPList] List to print.
** @param [r] html [AjBool] dump out html if true.
** @param [r] enzymes [AjPStr] names of enzymes to search for or 'all'
** @param [r] blunt [AjBool] Allow blunt cutters
** @param [r] sticky [AjBool] Allow sticky cutters
** @param [r] sitelen [int] minimum length of recognition site

** @return [void]
** @@
******************************************************************************/

static void NoCutList (AjPFile outfile, AjPList restrictlist, AjBool
	html, AjPStr enzymes, AjBool blunt, AjBool sticky, int sitelen) {

  AjPFile enzfile=NULL;
  AjPStr  *ea;
  int ne;		/* number of enzymes */
  AjBool isall=ajTrue;
  int i,j;
  AjIList miter;	/* iterator for matches list */
  EmbPMatMatch m=NULL;	/* restriction enzyme match structure */
  EmbPPatRestrict enz;
  char *p;
  int netmp;
  AjPList iso=ajListstrNew();	/* list of isoschizomers that cut */
  AjIList iter;		/* iterator for isoschizomers list */
  AjPStrTok tok;
  char tokens[] = " ,";
  AjPStr code = NULL;
  AjPStr code2 = NULL;

/* check on number of enzymes specified */
  ne = 0;
  if (!enzymes) {
      isall = ajTrue;
  } else {
    ne = ajArrCommaList(enzymes,&ea);
    for (i=0;i<ne;++i) {
      ajStrCleanWhite(&ea[i]);
    }
    if (ajStrMatchCaseC(ea[0],"all")) {
      isall = ajTrue;
    } else {
      isall = ajFalse;
    }
  }

  netmp = ne;
  


  if (isall) {
/* list all files in REBASE that don't cut */
    ajFileDataNewC(ENZDATA, &enzfile);

/* read all enzyme names into ea[] and set ne */
/* read once to count the valid entries in the enzyme file */
    ne = 0;
    enz = embPatRestrictNew();
    (void) ajFileSeek(enzfile,0L,0);
    while(embPatRestrictReadEntry(&enz,&enzfile)) {
      if(!enz->ncuts) continue;
      if(enz->len < sitelen) continue;
      if(!blunt && enz->blunt) continue;
      if(!sticky && !enz->blunt) continue;
      p = ajStrStr(enz->pat);
      if(*p < 'A' || *p > 'Z') continue;
      ne++;
    }


  for (i=0; i<netmp; ++i) 
  	if (ea[i]) 
  		ajStrDel(&ea[i]);
  if (netmp)
      AJFREE (ea);



/* make ea[] and populate it with enzyme names */
    AJCNEW(ea, ne);
    (void) ajFileSeek(enzfile,0L,0);
    i = 0;
    while(embPatRestrictReadEntry(&enz,&enzfile)) {
      if(!enz->ncuts) continue;
      if(enz->len < sitelen) continue;
      if(!blunt && enz->blunt) continue;
      if(!sticky && !enz->blunt) continue;
      p = ajStrStr(enz->pat);
      if(*p < 'A' || *p > 'Z') continue;
      ajStrAss(&ea[i], enz->cod);
      i++;
    }
    embPatRestrictDel(&enz);
    ajFileClose(&enzfile);

  }
  
/* find enzymes that don't cut and blank them out */
  miter = ajListIter(restrictlist);
  while ((m = ajListIterNext(miter)) != NULL) {
    for (i=0; i<ne; ++i) {
      if (ajStrMatchCase(ea[i], m->cod)) {
/* blank out this element of ea[] */
        ajStrClear(&ea[i]);
        break;
      }
    }
  }
  (void) ajListIterFree(miter);

/* make list of isoschizomers that cut */
  iter = ajListIter(restrictlist);
  while ((m = ajListIterNext(iter)) != NULL) {
/* start token to parse isoschizomers names */
    tok = ajStrTokenInit(m->iso,  tokens);
    while (ajStrToken (&code, &tok, tokens)) {
      code2 = ajStrNew();
      ajStrAss(&code2, code);
      ajListstrPush(iso, code2);
    }
  }
  ajStrTokenClear(&tok);
  (void) ajListIterFree(iter);
   

/* now check if it matches an isoschizomer - blank out if it does */
  iter = ajListIter(iso);
  while ((code2 = (AjPStr)ajListIterNext(iter)) != NULL) {
    for (i=0; i<ne; ++i) {
      if (ajStrMatchCase(ea[i], code2)) {
        ajStrClear(&ea[i]);
	break;
      }
    }
  }
  (void) ajListIterFree(iter);
  ajListstrFree(&iso);

/* now only have non-cutting enzyme names in ea[] - print them */
  if (html) (void) ajFmtPrintF(outfile, "<H2>");  
  (void) ajFmtPrintF(outfile, "\n\n# Enzymes that do not cut\n\n");
  if (html) (void) ajFmtPrintF(outfile, "</H2>");  

  if (html) {(void) ajFmtPrintF(outfile, "<PRE>");} 

  qsort(ea, ne, sizeof (*ea), ajStrCmp);
  j = 0;
  for (i = 0; i<ne; i++) {
/* if the element isn't blank, print it */
    if (ajStrLen(ea[i])) {	
      (void) ajFmtPrintF (outfile, "%-10S", ea[i]);
/* new line after every 7 names printed */      
      if (j++ == 7) {
      	(void) ajFmtPrintF (outfile, "\n");
      	j = 0;
      }
    }
  }  
  (void) ajFmtPrintF (outfile, "\n");
  if (html) {(void) ajFmtPrintF(outfile, "</PRE>");} 




/* tidy up */
  for (i=0; i<ne; ++i) 
  	if (ea[i]) 
  		ajStrDel(&ea[i]);
  if (ne)
      AJFREE (ea);

  return;
}


/* @funcstatic read_equiv *****************************************************
**
** Lifted from Alan's restrict.c but reads the equ file.
**
** @param [r] equfile [AjPFile*] file to read then close.
** @param [wP] table [AjPTable*] table to write to.
** @return [void]
** @@
******************************************************************************/

static void read_equiv(AjPFile *equfile, AjPTable *table)
{
    AjPStr line;
    AjPStr key;
    AjPStr value;

    char *p;

    line = ajStrNew();

    while(ajFileReadLine(*equfile,&line))
    {
        p=ajStrStr(line);
        if(!*p || *p=='#' || *p=='!')
            continue;
        p=strtok(p," \t\n");
        key=ajStrNewC(p);
        p=strtok(NULL," \t\n");
        value=ajStrNewC(p);
        ajTablePut(*table,(const void *)key, (void *)value);
    }
         
    ajFileClose(equfile);
}


/* @funcstatic read_file_of_enzyme_names *************************************
**
** If the list of enzymes starts with a '@' if opens that file, reads in
** the list of enzyme names and replaces the input string with the enzyme names
**
** @param [r] enzymes [AjPStr*] names of enzymes to search for or 'all' or '@file'
** @return [void]
** @@
******************************************************************************/

static void read_file_of_enzyme_names(AjPStr *enzymes) {

  AjPFile file=NULL;
  AjPStr line;
  char   *p=NULL;

  if (ajStrFindC(*enzymes, "@") == 0) {
    ajStrTrimC(enzymes, "@");	/* remove the @ */
    file = ajFileNewIn(*enzymes);
    if (file == NULL) {
      ajDie("Cannot open the file of enzyme names: '%S'", enzymes);
    }
/* blank off the enzyme file name and replace with the enzyme names */
    ajStrClear(enzymes);
    line = ajStrNew();
    while(ajFileReadLine(file, &line)) {
      p = ajStrStr(line);
      if (!*p || *p == '#' || *p == '!') continue;
      ajStrApp(enzymes, line);
      ajStrAppC(enzymes, ",");
    }
    ajStrDel(&line);
   
    ajFileClose(&file);  
  }

}

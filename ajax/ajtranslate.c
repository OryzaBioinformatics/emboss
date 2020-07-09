/********************************************************************
** @source AJAX translate functions
**
** These functions control all aspects of sequence translation
**
** These functions do not translate to the 'ambiguity' residues
** 'B' (Asn or Asp) and 'Z' (Glu or Gln). So the codons:
** RAC, RAT, RAY, RAU which could code for 'B' return 'X'
** and SAA, SAG, SAR which could code for 'Z' return 'X'.
**
** This translation table doesn't have the doubly ambiguous
** codons set up:
** YTR - L
** MGR - R
** YUR - L
** 
** This should be attended to at some time.
** 
** @author Copyright (C) 1999 Gary Williams
** @version 2.0 
** @modified Feb 15 1999 GWW First version
** @modified April 19 1999 GWW Second version using NCBI's GC tables
** @modified April 18 2000 GWW Reorganised many of the routines
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
********************************************************************
** 
** Example of typical usage (code fragment):
** 
** trnTable = ajTrnNewI (table_number);
** while (ajSeqallNext(seqall, &seq)) {
**  protein_seq = ajTrnNewPep(seq, frame);
**  ajStrClear(&protein_str);
**  ajTrnSeqFrame (trnTable, seq, frame, &protein_str)
**  ajSeqReplace (protein_seq, protein_str);
**  ajSeqAllWrite (seqout, protein_seq);
**  ajSeqDel (&protein_seq);
** }
** ajTrnDel(&trnTable);
** 
** or
** 
** trnTable = ajTrnNewI (table_number);
** ajTrnStr (trnTable, seq, &protein_str)
** ajFmtPrintF(outfile, "protein=%S\n", protein_str);
** ajTrnDel(&trnTable);
** 
** 
** 
** Example of typical ACD file list for getting the translation frame:
** 
**   list: frame [
**         opt: Y
**         default: "1"
**         min: 1
**         max: 1
**         header: "Translation frames"
**         values: 
**                 "1:1, 
**                 2:2, 
**                 3:3, 
**                 F:Forward three frames, 
**                 -1:-1, 
**                 -2:-2, 
**                 -3:-3, 
**                 R:Reverse three frames, 
**                 6:All six frames"
**         delim: ","
**         codedelim: ":"
**         prompt: "Frame(s) to translate"
**   ]
** 
** 
** Example of typical ACD file list for getting the genetic code table number:
** 
** list: table [
**        opt: Y
**        default: "0"
**        min: 1
**        max: 1
**        header: "Genetic codes"
**        values:
**                "0:Standard;
**                1:Standard (with alternative initiation codons);
**                2:Vertebrate Mitochondrial;
**                3:Yeast Mitochondrial;
**                4:Mold, Protozoan, Coelenterate Mitochondrial and Mycoplasma/Spiroplasma;
**                5:Invertebrate Mitochondrial;
**                6:Ciliate Macronuclear and Dasycladacean;
**                9:Echinoderm Mitochondrial;
**                10:Euplotid Nuclear;
**                11:Bacterial;
**                12:Alternative Yeast Nuclear;
**                13:Ascidian Mitochondrial;
**                14:Flatworm Mitochondrial;
**                15:Blepharisma Macronuclear;
**                16:Chlorophycean Mitochondrial;
**                21:Trematode Mitochondrial"
**        delim: ";"
**        codedelim: ":"
**        prompt: "Code to use"
**  ]
** 
** 
** 
********************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>

#include "ajax.h"

#define TGCFILE "EGC.0"
#define TGC "EGC."

/* table to convert character of base to translation array element value */
static ajint trnconv[] = {
/* characters less than 64 */
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 

/* @  A   B  C   D   E   F  G   H   I   J  K   L  M   N   O*/
  14, 0, 13, 1, 12, 14, 14, 2, 11, 14, 14, 9, 14, 4, 14, 14,

/* P   Q  R  S  T  U   V  W   X  Y   Z   [   \   ]   ^   _ */
  14, 14, 5, 7, 3, 3, 10, 6, 14, 8, 14, 14, 14, 14, 14, 14,

/* `  a   b  c   d   e   f  g   h   i   j  k   l  m   n   o */
  14, 0, 13, 1, 12, 14, 14, 2, 11, 14, 14, 9, 14, 4, 14, 14,

/* p   q  r  s  t  u   v  w   x  y   z   {   |   }   ~   del */
  14, 14, 5, 7, 3, 3, 10, 6, 14, 8, 14, 14, 14, 14, 14, 14
};



/* table to convert character of COMPLEMENT of base to translation array
element value */
static ajint trncomp[] = {
/* characters less than 64 */
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
  14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 

/* @  A   B  C   D   E   F  G   H   I   J  K   L  M   N   O*/
  14, 3, 10, 2, 11, 14, 14, 1, 12, 14, 14, 4, 14, 9, 14, 14,

/* P   Q  R  S  T  U   V  W   X  Y   Z   [   \   ]   ^   _ */
  14, 14, 8, 7, 0, 0, 13, 6, 14, 5, 14, 14, 14, 14, 14, 14,

/* `  a   b  c   d   e   f  g   h   i   j  k   l  m   n   o */
  14, 3, 10, 2, 11, 14, 14, 1, 12, 14, 14, 4, 14, 9, 14, 14,

/* p   q  r  s  t  u   v  w   x  y   z   {   |   }   ~   del */
  14, 14, 8, 7, 0, 0, 13, 6, 14, 5, 14, 14, 14, 14, 14, 14
};



static void trnNoComment (AjPStr* text);

static void getwobblebases(AjPTrn trnObj, AjBool *w1a, AjBool *w1c,
  AjBool *w1g, AjBool *w1t, AjBool *w3a, AjBool *w3c, AjBool *w3g,
  AjBool *w3t, char base1, char base2, char base3, char aa);


static void explode(AjPTrn trnObj, AjBool wa, AjBool wc, AjBool wg,
  AjBool wt, char base1, char base2, char base3, char aa, AjBool ajTrue);



/* @func ajTrnDel ********************************************************
**   
** Deletes a translation table object
**
** @param [w] pthis [AjPTrn*] Address of translation table object
** @return [void]
** @@
******************************************************************************/
  
void ajTrnDel (AjPTrn* pthis) {
 	  
  AjPTrn thys = *pthis;
 	     
  ajStrDel(&thys->FileName);
  ajStrDel(&thys->Title);
 	                        
  AJFREE(*pthis);  
 	                         
  return;
}
 

/* @func ajTrnNewC ************************************************************
**
** Initialises translation. Reads a translation data file
** ajTrnDel (AjPTrn); should be called when translation has ceased.
**
** @param [r] filename [char*] translation table file name
** @return [AjPTrn] Translation object
** @@
******************************************************************************/

AjPTrn ajTrnNewC (char * filename) {

  AjPStr trnFileName = NULL;
  
  trnFileName = ajStrNewC (filename);

  return ajTrnNew (trnFileName);

}

/* @func ajTrnNewI ************************************************************
**
** Initialises translation. Reads a translation data file called 'EGC.x'
** where 'x' is supplied as an ajint parameter in the range 0 to 15.
** ajTrnDel (AjPTrn); should be called when translation has ceased.
**
** @param [r] trnFileNameInt [ajint] translation table file name number
** @return [AjPTrn] Translation object
** @@
******************************************************************************/

AjPTrn ajTrnNewI (ajint trnFileNameInt) {

  AjPStr trnFileName = NULL;
  AjPStr value = NULL;
  AjPTrn ret;
  
  trnFileName = ajStrNewC (TGC);
  (void) ajStrFromInt(&value, trnFileNameInt);
  (void) ajStrApp(&trnFileName, value);

  ajStrDel(&value);
  ret = ajTrnNew (trnFileName);
  ajStrDel(&trnFileName);

  return ret;
}

/* @func ajTrnNew ************************************************************
**
** Initialises translation. Reads a translation data file
** ajTrnDel (AjPTrn); should be called when translation has ceased.
**
** @param [r] trnFileName [AjPStr] translation table file name
** @return [AjPTrn] Translation object
** @@
******************************************************************************/

AjPTrn ajTrnNew (AjPStr trnFileName) {

  AjPFile trnFile = NULL;
  AjPTrn pthis;
  ajint i, j, k;

  /* open the translation table file */

  /* if the file is not specified, use the standard table file */
  if (!ajStrLen(trnFileName)) {
  	trnFileName = ajStrNewC (TGCFILE);
  }
  
  ajFileDataNew(trnFileName, &trnFile);
  if (trnFile==NULL) ajDie ("Translation table file '%S' not found\n", trnFileName);

/* create and initialise the translation object */
  AJNEW0(pthis);
  pthis->FileName = ajStrNew();
  pthis->Title = ajStrNew();

/* initialise the GC and Starts tables */
  for (i=0; i<15; i++) {
    for (j=0; j<15; j++) {
      for (k=0; k<15; k++) {
        pthis->GC[i][j][k] = 'X';
        pthis->Starts[i][j][k] = '-';
      }
    }
  }

  (void) ajStrAss(&(pthis->FileName), trnFileName);
  ajTrnReadFile(pthis, trnFile);

  ajFileClose (&trnFile);

  /* all done */
  return pthis;

}

/* @func ajTrnReadFile ********************************************************
**
** Reads a translation data file
** ajTrnDel (trnObj); should be called when translation has ceased.
**
** @param [w] trnObj [AjPTrn] translation table object
** @param [r] trnFile [AjPFile] translation table file handle
** @return [void]
** @@
******************************************************************************/

void ajTrnReadFile (AjPTrn trnObj, AjPFile trnFile) {

  AjPStr trnLine = NULL;
  AjPStr trnText = NULL;
  AjPStrTok tokenhandle;
  AjPStr tmpstr = NULL;
  AjPStr aaline = NULL;
  AjPStr startsline = NULL;
  AjPStr base1line = NULL;
  AjPStr base2line = NULL;
  AjPStr base3line = NULL;
  char *aa;
  char *starts;
  char *base1;
  char *base2;
  char *base3;
  ajint dlen;
  ajint i, j;
  ajint firstaa[256];	/* positions of first use of a residue in the aa line */
  AjBool w1a, w1c, w1g, w1t, w3a, w3c, w3g, w3t; /* first and last base wobble results */
/* NB '-' and '*' are valid characters,
   don't skip over then when parsing tokens */
  char white[] = " \t\n\r!@#$%^&()_+=|\\~`{[}]:;\"'<,>.?/";


/* look to see if this is a Genetic Code file */
  while (ajFileReadLine (trnFile, &trnLine)) {
    trnNoComment(&trnLine); 
    if (ajStrLen(trnLine)) {
      if (ajStrFindC (trnLine, "Genetic Code") == -1) {
        ajDie ("The file '%S' is not a valid Genetic Code file.\n"
        "The 'Genetic Code' line was not found.", trnObj->FileName);
      } else {
      	break;
      }
    }
  }

/* read the title of the file */
  while (ajFileReadLine (trnFile, &trnLine)) {
    trnNoComment(&trnLine); 
    if (ajStrLen(trnLine)) {
      (void) ajStrAss(&(trnObj->Title), trnLine);
      break;
    }
  }

/* read the whole of the rest of the file into a string */
  while (ajFileReadLine (trnFile, &trnLine)) {
    trnNoComment(&trnLine);
    if (ajStrLen(trnLine)) {
      (void) ajStrApp (&trnText, trnLine);
      (void) ajStrAppC (&trnText, " ");
    }            
  }


/* read data lines */
  tokenhandle = ajStrTokenInit (trnText, white);

  (void) ajStrToken (&tmpstr, &tokenhandle, NULL);
  if (ajStrCmpC (tmpstr, "AAs") == -1) {
    ajDie ("The file '%S' is not a valid Genetic Code file.\n"
    "The 'AAs' line was not found.", trnObj->FileName);
  }
  (void) ajStrToken (&aaline, &tokenhandle, NULL);
  aa = ajStrStr(aaline);
    
  (void) ajStrToken (&tmpstr, &tokenhandle, NULL);
  if (ajStrCmpC (tmpstr, "Starts") == -1) {
    ajDie ("The file '%S' is not a valid Genetic Code file.\n"
    "The 'Starts' line was not found.", trnObj->FileName);
  }
  (void) ajStrToken (&startsline, &tokenhandle, NULL);
  starts = ajStrStr(startsline);

  (void) ajStrToken (&tmpstr, &tokenhandle, NULL);
  if (ajStrCmpC (tmpstr, "Base1") == -1) {
    ajDie ("The file '%S' is not a valid Genetic Code file.\n"
    "The 'Base1' line was not found.", trnObj->FileName);
  }
  (void) ajStrToken (&base1line, &tokenhandle, NULL);
  base1 = ajStrStr(base1line);

  (void) ajStrToken (&tmpstr, &tokenhandle, NULL);
  if (ajStrCmpC (tmpstr, "Base2") == -1) {
    ajDie ("The file '%S' is not a valid Genetic Code file.\n"
    "The 'Base2' line was not found.", trnObj->FileName);
  }
  (void) ajStrToken (&base2line, &tokenhandle, NULL);
  base2 = ajStrStr(base2line);

  (void) ajStrToken (&tmpstr, &tokenhandle, NULL);
  if (ajStrCmpC (tmpstr, "Base3") == -1) {
    ajDie ("The file '%S' is not a valid Genetic Code file.\n"
    "The 'Base3' line was not found.", trnObj->FileName);
  }
  (void) ajStrToken (&base3line, &tokenhandle, NULL);
  base3 = ajStrStr(base3line);


/* tidy up */
  ajStrTokenClear (&tokenhandle);
  ajStrDel(&tmpstr);

/* populate the Starts (Initiation sites) table */
  dlen = ajStrLen(startsline);
  for (i=0; i<dlen; i++) {
    trnObj->Starts[trnconv[(ajint)base1[i]]][trnconv[(ajint)base2[i]]][trnconv[(ajint)base3[i]]]
      = starts[i];
  }

/* populate the GC (Genetic code) table */
  dlen = ajStrLen(aaline);

  
/* initialise first use of aa array */
  for (i=0; i<256; i++) firstaa[i] = -1;
  for (i=0; i<dlen; i++) {
/* put the residue in the table using the unambiguous codon */
    trnObj->GC[trnconv[(ajint)base1[i]]][trnconv[(ajint)base2[i]]][trnconv[(ajint)base3[i]]]
      = aa[i];
/* now work out the ambiguous codons for this residue so far */
/* is this the first use of the residue in the aa line? */
    if (firstaa[(ajint)aa[i]] == -1) {
/* yes - note its position */
      firstaa[(ajint)aa[i]] = i;
    } else {
/* no - see if we can construct some ambiguous codons */
      getwobblebases(trnObj, &w1a, &w1c, &w1g, &w1t, &w3a, &w3c, &w3g, &w3t, base1[i], base2[i], base3[i], aa[i]);
      for (j=i-1; j>=firstaa[(ajint)aa[i]]; j--) {
/* if previous aa is the same as aa[i] then construct abiguity codon */
        if (aa[i] == aa[j]) {
/* there are no ambiguous codons with a differing middle base */
          if (base2[i] != base2[j]) continue;
/* don't look for ambiguous codons with a differing start and a differing end base */
          if (base1[i] != base1[j] && base3[i] != base3[j]) continue;
/* we therefore have either only a differing start base */
          if (base1[i] != base1[j]) {
            explode(trnObj, w1a, w1c, w1g, w1t, base1[i], base2[i], base3[i], aa[i], ajTrue);
          } else {
/* or only a differing end base */
            explode(trnObj, w3a, w3c, w3g, w3t, base1[i], base2[i], base3[i], aa[i], ajFalse);
          }
        }
      }
    }
  }
  ajStrDel(&trnText);
  ajStrDel(&startsline);
  ajStrDel(&base1line);
  ajStrDel(&base2line);
  ajStrDel(&base3line);
  ajStrDel(&aaline);
  ajStrDel(&trnLine);
  
  return;
}


/* @funcstatic trnNoComment ***************************************************
**
** Strips comments from a character string (a line from an trn file).
** Comments are blank lines or any text following a "#" character.
** Whitespace characters can be included in a blank line.
**
** @param [u] text [AjPStr*] Line of text from input file
** @return [void]
** @@
******************************************************************************/

static void trnNoComment (AjPStr* text) {
  ajint i;
  char *cp;

  (void) ajStrChomp (text);
  i = ajStrLen (*text);

  if (!i)                       /* empty string */
    return;

  cp = strchr(ajStrStr(*text), '#');
  if (cp) {                      /* comment found */
    *cp = '\0';
    ajStrFix (*text);
  }

  return;

}

/* @funcstatic getwobblebases ************************************************
**
** Gets the results af wobbling the first and last bases of a standard codon.
** Returns True or False for each wobble depending on whether the
** result is the same as the input residue or not.
**
** @param [u] trnObj [AjPTrn] Translation tables
** @param [w] w1a [AjBool *] result of changing base 1 to A
** @param [w] w1c [AjBool *] result of changing base 1 to C
** @param [w] w1g [AjBool *] result of changing base 1 to G
** @param [w] w1t [AjBool *] result of changing base 1 to T
** @param [w] w3a [AjBool *] result of changing base 3 to A
** @param [w] w3c [AjBool *] result of changing base 3 to C
** @param [w] w3g [AjBool *] result of changing base 3 to G
** @param [w] w3t [AjBool *] result of changing base 3 to T
** @param [r] base1 [char] standard base 1
** @param [r] base2 [char] standard base 2
** @param [r] base3 [char] standard base 3
** @param [r] aa [char] residue that the standard codon codes for
** @return [void]
** @@
******************************************************************************/
static void getwobblebases(AjPTrn trnObj, AjBool *w1a, AjBool *w1c,
  AjBool *w1g, AjBool *w1t, AjBool *w3a, AjBool *w3c, AjBool *w3g,
  AjBool *w3t, char base1, char base2, char base3, char aa) {

  *w1a = *w1c = *w1g = *w1t = *w3a = *w3c = *w3g = *w3t = ajFalse;

  if (trnObj->GC[trnconv[(ajint)'A']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] == aa) {
    *w1a = ajTrue;
  }
  if (trnObj->GC[trnconv[(ajint)'C']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] == aa) {
    *w1c = ajTrue;
  }
  if (trnObj->GC[trnconv[(ajint)'G']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] == aa) {
    *w1g = ajTrue;
  }
  if (trnObj->GC[trnconv[(ajint)'T']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] == aa) {
    *w1t = ajTrue;
  }

  if (trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'A']] == aa) {
    *w3a = ajTrue;
  }
  if (trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'C']] == aa) {
    *w3c = ajTrue;
  }
  if (trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'G']] == aa) {
    *w3g = ajTrue;
  }
  if (trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'T']] == aa) {
    *w3t = ajTrue;
  }

}

/* @funcstatic explode ************************************************
**
** Sets the ambiguity codons in the translation table.
**
** @param [u] trnObj [AjPTrn] Translation tables
** @param [r] wa [AjBool] result of changing base 1 to A
** @param [r] wc [AjBool] result of changing base 1 to C
** @param [r] wg [AjBool] result of changing base 1 to G
** @param [r] wt [AjBool] result of changing base 1 to T
** @param [r] base1 [char] standard base 1
** @param [r] base2 [char] standard base 2
** @param [r] base3 [char] standard base 3
** @param [r] aa [char] residue that the standard codon codes for
** @param [r] start [AjBool] the first base is being wobbled (else is the last)
** @return [void]
** @@
******************************************************************************/
static void explode(AjPTrn trnObj, AjBool wa, AjBool wc, AjBool wg,
  AjBool wt, char base1, char base2, char base3, char aa, AjBool start) {

  AjBool doneone = ajFalse;;  

  if (start) {
    if (wt && wc) {
      trnObj->GC[trnconv[(ajint)'Y']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
      doneone = ajTrue;
    }
 
    if (wt && wa) {
      trnObj->GC[trnconv[(ajint)'W']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
    doneone = ajTrue;
    }
    if (wt && wg) {
      trnObj->GC[trnconv[(ajint)'K']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
      doneone = ajTrue;
    }
    if (wc && wa) {
      trnObj->GC[trnconv[(ajint)'M']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
      doneone = ajTrue;
    }
    if (wc && wg) {
      trnObj->GC[trnconv[(ajint)'S']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
      doneone = ajTrue;
    }
    if (wa && wg) {
      trnObj->GC[trnconv[(ajint)'R']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
      doneone = ajTrue;
    }

/* if we haven't set any ambiguity codons for a pair of wobble bases
then there is no point in testing triples */
    if (!doneone) return;

    if (wc && wa && wg) {
      trnObj->GC[trnconv[(ajint)'V']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
    }
    if (wt && wc && wa) {
      trnObj->GC[trnconv[(ajint)'H']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
    }
    if (wt && wa && wg) {
      trnObj->GC[trnconv[(ajint)'D']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
    }
    if (wt && wc && wg) {
      trnObj->GC[trnconv[(ajint)'B']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
    }


    if (wt && wc && wa && wg) {
      trnObj->GC[trnconv[(ajint)'N']][trnconv[(ajint)base2]][trnconv[(ajint)base3]] = aa;
    }
  } else {	/* not start */
  

    if (wt && wc) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'Y']] = aa;
      doneone = ajTrue;
    }
    if (wt && wa) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'W']] = aa;
      doneone = ajTrue;
    }
    if (wt && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'K']] = aa;
      doneone = ajTrue;
    }
    if (wc && wa) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'M']] = aa;
      doneone = ajTrue;
    }
    if (wc && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'S']] = aa;
      doneone = ajTrue;
    }
    if (wa && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'R']] = aa;
      doneone = ajTrue;
    }

/* if we haven't set any ambiguity codons for a pair of wobble bases
then there is no point in testing triples */
    if (!doneone) return;

    if (wc && wa && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'V']] = aa;
    }
    if (wt && wc && wa) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'H']] = aa;
    }    
    if (wt && wa && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv['D']] = aa;
    }    
    if (wt && wc && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'B']] = aa;
    }    


    if (wt && wc && wa && wg) {
      trnObj->GC[trnconv[(ajint)base1]][trnconv[(ajint)base2]][trnconv[(ajint)'N']] = aa;
    }    

  } /* end start */

}

/* @func ajTrnNewPep ************************************************************
**
** Creates a new AjPSeq set up with an appropriate name and description
**
** It:
**  Creates a AjPSeq object
**  Sets it to be a protein.
**  Sets the description as being the same as that description of the nucleic
**    acid sequence it was translated from.
**  Gives it the same name as the nucleic acid sequence it is translated from.
**  If the frame is not specified as being '0' it will then append a '_' and the
**  number of the frame to form a unique name for the protein sequence in the event
**  of many frames being translated. If the frame number is negative, it will
**  use a number in the range 4, 5, 6, this is because ID names with '-' in them
**  were causing problems in the sequence reading routines.
**
** Frame 4 is the same as frame -1 (4=-1, 5=-2, 6=-3)
**
** You will have to set the sequence of this object with something like:
**  ajSeqReplace (trnPeptide, seqstr);
**
**
** @param [r] nucleicSeq [AjPSeq] nucleic sequence being translated
** @param [r] frame [ajint] frame of translation (-3,-2,-1,0,1,2,3,4,5,6)
** @return [AjPSeq] New peptide object
** @@
******************************************************************************/

AjPSeq ajTrnNewPep(AjPSeq nucleicSeq, ajint frame) {

  AjPSeq trnPeptide=NULL;
  AjPStr name = NULL;	/* name of the translation */
  AjPStr value = NULL;	/* value of frame of the translation */

/* set up the output sequence */
  trnPeptide = ajSeqNew ();
  ajSeqSetProt (trnPeptide);

/* create a nice name for the subsequence */
  (void) ajStrAss(&name, ajSeqGetName(nucleicSeq));

/* if the frame is not 0 then append the frame number to the name to make it unique */
  if (frame != 0) {
    if (frame < 0) frame = -frame + 3;
    (void) ajStrAppC(&name, "_");

    (void) ajStrFromInt(&value, frame);
    (void) ajStrApp(&name, value);
  }
  
  ajSeqAssName(trnPeptide, name);
  
/* set the description of the translation */
  ajSeqAssDesc(trnPeptide, ajSeqGetDesc(nucleicSeq));


  return trnPeptide;
}

/* @func ajTrnCodon ************************************************************
**
** Translates a codon
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [AjPStr] codon to translate
** @return [AjPStr] Amino acid translation
** @@
******************************************************************************/

AjPStr ajTrnCodon (AjPTrn trnObj, AjPStr codon) {

  static AjPStr trnResidue=NULL;
  char * res;
  char store[2];
  
  store[1] = '\0';	/* end the char * of store */

  res = ajStrStr(codon);
  store[0] = trnObj->GC[trnconv[(ajint)res[0]]][trnconv[(ajint)res[1]]][trnconv[(ajint)res[2]]];

  (void) ajStrAssC (&trnResidue, store);

  return trnResidue;
}

/* @func ajTrnRevCodon ************************************************************
**
** Translates the reverse complement of a codon
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [AjPStr] codon to translate
** @return [AjPStr] Amino acid translation
** @@
******************************************************************************/

AjPStr ajTrnRevCodon (AjPTrn trnObj, AjPStr codon) {

  static AjPStr trnResidue=NULL;
  char * res;
  char store[2];
  
  store[1] = '\0';	/* end the char * of store */

  res = ajStrStr(codon);
  store[0] = trnObj->GC[trncomp[(ajint)res[2]]][trncomp[(ajint)res[1]]][trncomp[(ajint)res[0]]];

  (void) ajStrAssC (&trnResidue, store);

  return trnResidue;
}

/* @func ajTrnCodonC ************************************************************
**
** Translates a const char * codon
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [char *] codon to translate (these 3 characters need not be NULL-terminated)
** @return [AjPStr] Amino acid translation
** @@
******************************************************************************/

AjPStr ajTrnCodonC (AjPTrn trnObj, char *codon) {

  static AjPStr trnResidue=NULL;
  char store[2];
  
  store[1] = '\0';	/* end the char * of store */
  store[0] = trnObj->GC[trnconv[(ajint)codon[0]]][trnconv[(ajint)codon[1]]][trnconv[(ajint)codon[2]]];
  
  (void) ajStrAssC (&trnResidue, store);

  return trnResidue;
}

/* @func ajTrnRevCodonC ************************************************************
**
** Translates the reverse complement of a const char * codon
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [char *] codon to translate (these 3 characters need not be NULL-terminated)
** @return [AjPStr] Amino acid translation
** @@
******************************************************************************/

AjPStr ajTrnRevCodonC (AjPTrn trnObj, char *codon) {

  static AjPStr trnResidue=NULL;
  char store[2];
  
  store[1] = '\0';	/* end the char * of store */
  store[0] = trnObj->GC[trncomp[(ajint)codon[2]]][trncomp[(ajint)codon[1]]][trncomp[(ajint)codon[0]]];
  
  (void) ajStrAssC (&trnResidue, store);

  return trnResidue;
}

/* @func ajTrnCodonK ************************************************************
**
** Translates a const char * codon to a char
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [char *] codon to translate (these 3 characters need not be NULL-terminated)
** @return [char] Amino acid translation
** @@
******************************************************************************/

char ajTrnCodonK (AjPTrn trnObj, char *codon) {

  return trnObj->GC[trnconv[(ajint)codon[0]]][trnconv[(ajint)codon[1]]][trnconv[(ajint)codon[2]]];

}

/* @func ajTrnRevCodonK ************************************************************
**
** Translates a the reverse complement of a const char * codon to a char
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [char *] codon to translate (these 3 characters need not be NULL-terminated)
** @return [char] Amino acid translation
** @@
******************************************************************************/

char ajTrnRevCodonK (AjPTrn trnObj, char *codon) {

  return trnObj->GC[trncomp[(ajint)codon[2]]][trncomp[(ajint)codon[1]]][trncomp[(ajint)codon[0]]];

}

/* @func ajTrnC ************************************************************
**
** Translates a sequence in a char *
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] str [char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnC (AjPTrn trnObj, char *str, ajint len, AjPStr *pep) {

  ajint i;
  ajint lenmod3;

  lenmod3 = len - (len % 3);

  for (i=0; i < lenmod3; i+=3) {
/* speed up slightly by putting the routine in-line */
/*  (void) ajStrApp(pep, ajTrnCodonC (trnObj, &str[i])); */
    (void) ajStrAppK(pep, trnObj->GC[trnconv[(ajint)str[i]]]
    				     [trnconv[(ajint)str[i+1]]]
    				     [trnconv[(ajint)str[i+2]]]);
  }

}
/* @func ajTrnRevC ************************************************************
**
** Translates the reverse complement of a sequence in a char *.
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in frame -1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] str [char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnRevC (AjPTrn trnObj, char *str, ajint len, AjPStr *pep) {

  ajint i;
  ajint ajb;
  
  ajb = (len/3)*3-1;
  for(i=ajb;i>1;i-=3)
    (void) ajStrAppK(pep, trnObj->GC[trncomp[(ajint)str[i]]]
    				     [trncomp[(ajint)str[i-1]]]
    				     [trncomp[(ajint)str[i-2]]]);

}
/* @func ajTrnStr ************************************************************
**
** Translates a sequence in a AjPStr.
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] str [AjPStr] sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnStr (AjPTrn trnObj, AjPStr str, AjPStr *pep) {

  ajTrnC(trnObj, ajStrStr(str), ajStrLen(str), pep);

}
/* @func ajTrnRevStr ************************************************************
**
** Translates the reverse complement of a sequence in a AjPStr.
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in frame -1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] str [AjPStr] sequence string to translate
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnRevStr (AjPTrn trnObj, AjPStr str, AjPStr *pep) {

  ajTrnRevC(trnObj, ajStrStr(str), ajStrLen(str), pep);

}
/* @func ajTrnSeq ************************************************************
**
** Translates a sequence in a AjPSeq
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] seq [AjPSeq] sequence to translate
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnSeq (AjPTrn trnObj, AjPSeq seq, AjPStr *pep) {

  ajTrnC(trnObj, ajSeqChar(seq), ajSeqLen(seq), pep);

}

/* @func ajTrnRevSeq ************************************************************
**
** Translates the reverse complement of a sequence in a AjPSeq
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in frame 1 (from the first base) to the last full
** triplet codon, (i.e. if there are 1 or 2 bases extra at the end, they are
** ignored)
**
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] seq [AjPSeq] sequence to translate
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnRevSeq (AjPTrn trnObj, AjPSeq seq, AjPStr *pep) {

  ajTrnRevC(trnObj, ajSeqChar(seq), ajSeqLen(seq), pep);

}



/* @func ajTrnCFrame ************************************************************
**
** Translates a sequence in a char * in the specified frame.
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in the specified frame (one of: 1,2,3,-1,-2,-3,4,5,6) 
** to the last full triplet codon, (i.e. if there are 1 or 2 bases extra at the end,
** they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement sequence which matches
** the codons used in frame 1. ie. in the sequence ACGT, the first codon of frame 1
** is ACG and the last codon of frame -1 is the reverse complement of ACG (ie. CGT).
** 
** Frame 4 is the same as frame -1 (4=-1, 5=-2, 6=-3)
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] seq [char *] sequence string to translate
** @param [r] len [ajint] length of sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnCFrame (AjPTrn trnObj, char *seq, ajint len, ajint frame, AjPStr *pep) {

  if (frame > 3) frame = -frame + 3;

  if (frame >= 1 && frame <= 3) {
    ajTrnC(trnObj, &seq[frame-1], len, pep);
  } else if (frame >= -3 && frame <= -1) {
    ajTrnRevC (trnObj, &seq[-frame-1], len, pep);
  } else {
    ajDie("Invalid frame '%d' in ajTrnCFrame()\n", frame);
  }

}


/* @func ajTrnStrFrame ************************************************************
**
** Translates a sequence in a AjStr in the specified frame.
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in the specified frame (one of: 1,2,3,-1,-2,-3) 
** to the last full triplet codon, (i.e. if there are 1 or 2 bases extra at the end,
** they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement sequence which matches
** the codons used in frame 1. ie. in the sequence ACGT, the first codon of frame 1
** is ACG and the last codon of frame -1 is the reverse complement of ACG (ie. CGT).
**
** Frame 4 is the same as frame -1
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] seq [AjPStr] sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnStrFrame (AjPTrn trnObj, AjPStr seq, ajint frame, AjPStr *pep) {

  ajTrnCFrame(trnObj, ajStrStr(seq), ajStrLen(seq), frame, pep);

}


/* @func ajTrnSeqFrame ************************************************************
**
** Translates a sequence in a AjSeq in the specified frame.
** ajTrnInit must be called before this routine to set up the translation table.
**
** This routine translates in the specified frame (one of: 1,2,3,-1,-2,-3) 
** to the last full triplet codon, (i.e. if there are 1 or 2 bases extra at the end,
** they are ignored).
**
** Frame -1 is defined as the translation of the reverse complement sequence which matches
** the codons used in frame 1. ie. in the sequence ACGT, the first codon of frame 1
** is ACG and the last codon of frame -1 is the reverse complement of ACG (ie. CGT).
**
** Frame 4 is the same as frame -1
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] seq [AjPSeq] sequence string to translate
** @param [r] frame [ajint] frame to translate in
** @param [u] pep [AjPStr *] returned peptide translation (appended to input contents)
**
** @return [void]
** @@
******************************************************************************/

void ajTrnSeqFrame (AjPTrn trnObj, AjPSeq seq, ajint frame, AjPStr *pep) {

  ajTrnCFrame(trnObj, ajSeqChar(seq), ajSeqLen(seq), frame, pep);

}


/* @func ajTrnSeqOrig ************************************************************
**
** THIS ROUTINE IS DEPRECATED. 
** DO NOT USE IT.
** THIS WILL DISAPPEAR SOON.
**
** Translates a sequence.
** ajTrnInit must be called before this routine to set up the translation table.
**
** The frame to translate is in the range -3 to 6.
** Frames 1 to 3 give normal forward translations.
** Frames -3 to -1 rev-complement the DNA sequence then give normal translations.
** Frames 4 to 6 rev-comp the DNA sequence then reverse the peptide sequence
** Frames 4 to 6 are therefore a reversed protein sequence useful mainly for
**  displaying beneath the original DNA sequence.
**
** NB.  however that the naming of the output sequence is always to take
** the name of the input sequence (eg.  ECARGS) and to append an underscore
** character and the frame number 1 to 3 for forward frames and 4 to 6 for
** reverse frames regardless of the final orientation of the reverse
** frames.  (i.e.  frame -1 = ECARGS_4, frame -2 = ECARGS_5, -3 = ECARGS_6, 4 =
** ECARGS_4, 5 = ECARGS_5 6 = ECARGS_6)
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] trnSeq [AjPSeq] sequence to translate
** @param [r] frame [ajint] frame to translate in (-3, -2, -1, 1, 2, 3, 4, 5, 6)
**
** @return [AjPSeq] Peptide translation
** @@
******************************************************************************/

AjPSeq ajTrnSeqOrig (AjPTrn trnObj, AjPSeq trnSeq, ajint frame) {

  AjPSeq trnPeptide=NULL;
  AjPStr seq=NULL; /* the string holding the nucleic sequence */
  AjPStr trn=NULL; /* the string holding the peptide sequence */
  char *seqc;	/* pointer to char sequence */
  ajint realframe;
  ajint nameframe;	/* frame number appended to name */
  ajint i;
  AjPStr name = NULL;	/* name of the translation */
  AjPStr value = NULL;	/* value of frame of the translation */
  ajint framelen;	/* length of sequence after the frame-start position */
  ajint lenmod3;	/* length of sequence with no dangling bases of incomplete codons */
  
/* get a COPY of the sequence string */
  (void) ajStrAss (&seq, ajSeqStr(trnSeq));


/* if the frame is negative, then we want the reverse complement of the
sequence */
  if (frame < 0) {
    realframe = -frame;
    ajSeqReverseStr(&seq);
    nameframe = -frame + 3;
  } else {
    realframe = frame;
    nameframe = frame;
  }
  
/* if the frame is greater than 3 , then we want the reverse complement
of the sequence */
  if (frame > 3) {
    realframe = frame - 3;
    ajSeqReverseStr(&seq);
  }

/* set up the output sequence */
  trnPeptide = ajSeqNew ();
  ajSeqSetProt (trnPeptide);

/* create a nice name for the subsequence */
  (void) ajStrAss(&name, ajSeqGetName(trnSeq));
  (void) ajStrAppC(&name, "_");

  (void) ajStrFromInt(&value, nameframe);
  (void) ajStrApp(&name, value);
  ajSeqAssName(trnPeptide, name);
  
/* set the description of the translation */
  ajSeqAssDesc(trnPeptide, ajSeqGetDesc(trnSeq));
  
/* string to hold result */
  trn = ajStrNew();

/* go through the sequence translating it */
  seqc = ajStrStr(seq);
  framelen = ajStrLen(seq) - (realframe-1);
  lenmod3 = framelen - (framelen % 3);
  for (i=realframe-1; i < lenmod3; i+=3) {
/* speed up slightly by putting the routine in-line */
/*  (void) ajStrApp(&trn, ajTrnCodonC (trnObj, &seqc[i])); */
    (void) ajStrAppK(&trn, trnObj->GC[trnconv[(ajint)seqc[i]]][trnconv[(ajint)seqc[i+1]]][trnconv[(ajint)seqc[i+2]]]);
  }

/* translate any dangling pair of bases at the end */
  if (framelen % 3 == 2) {
    (void) ajStrAppK(&trn, trnObj->GC[trnconv[(ajint)seqc[i]]][trnconv[(ajint)seqc[i+1]]][trnconv[0]]);
  } else if (framelen % 3 == 1) {
/* I don't seriously expect a single base to translate sensibly, but they asked for it... */
    (void) ajStrAppK(&trn, 'X');
  }

/* if we did frames 4 to 6, then reverse the peptide sequence for nice
alignment displays with the original nucleic acid sequence */
  if (frame > 3) {
    (void) ajStrRev (&trn);
  }

/* set the output sequence up */
  ajSeqReplace (trnPeptide, trn);

/* clean up */
  ajStrDel (&seq);
  ajStrDel (&trn);
  ajStrDel (&name);
  ajStrDel (&value);

  return trnPeptide;
}


/* @func ajTrnStrOrig ************************************************************
**
** THIS ROUTINE IS DEPRECATED. 
** DO NOT USE IT.
** THIS WILL DISAPPEAR SOON.
**
** Translates a sequence in an AjPStr.
** ajTrnInit must be called before this routine to set up the translation
** table.
**
** The frame to translate is in the range -3 to 6.
** Frames 1 to 3 give normal forward translations.
** Frames -3 to -1 rev-complement the DNA sequence
** Frames 4 to 6 also rev-comp the DNA sequence
** 
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] trnSeq [AjPStr] sequence to translate
** @param [r] frame [ajint] frame to translate in (-3, -2, -1, 1, 2, 3, 4, 5, 6)
**
** @return [AjPStr] Peptide translation
** @@
******************************************************************************/

AjPStr ajTrnStrOrig (AjPTrn trnObj, AjPStr trnSeq, ajint frame)
{
    AjPStr seq=NULL;		/* the string holding the nucleic sequence */
    AjPStr trn=NULL;		/* the string holding the peptide sequence */
    char *seqc;			/* pointer to char sequence */
    ajint realframe;
    ajint i;
    AjPStr value = NULL;	/* value of frame of the translation */
    ajint framelen;		/* length of sequence after the frame-start
                                   position */
    ajint lenmod3;		/* length of sequence with no dangling bases
                                   of incomplete codons */
  
    /* get a COPY of the sequence string */
    (void) ajStrAssC (&seq, ajStrStr(trnSeq));


    /*
     *  if the frame is negative, then we want the reverse complement of the
     *  sequence
     */
    if (frame < 0)
    {
	realframe = -frame;
	ajSeqReverseStr(&seq);
    }
    else
    {
	realframe = frame;
    }
  
    /*
     *  if the frame is greater than 3 , then we want the reverse complement
     *  of the sequence
     */
    if (frame > 3)
    {
	realframe = frame - 3;
	ajSeqReverseStr(&seq);
    }


    /* string to hold result */
    trn = ajStrNew();

    /* go through the sequence translating it */
    seqc = ajStrStr(seq);
    framelen = ajStrLen(seq) - (realframe-1);
    lenmod3 = framelen - (framelen % 3);
    for (i=realframe-1; i < lenmod3; i+=3)
    {
	/* speed up slightly by putting the routine in-line */
	/*  (void) ajStrApp(&trn, ajTrnCodonC (trnObj, &seqc[i])); */
	(void) ajStrAppK(&trn, trnObj->GC[trnconv[(ajint)seqc[i]]]
			 [trnconv[(ajint)seqc[i+1]]]
			 [trnconv[(ajint)seqc[i+2]]]);
    }

    /* translate any dangling pair of bases at the end */
    if (framelen % 3 == 2)
    {
	(void) ajStrAppK(&trn, trnObj->GC[trnconv[(ajint)seqc[i]]]
			 [trnconv[(ajint)seqc[i+1]]]
			 [trnconv[0]]);
    }
    else if (framelen % 3 == 1)
    {
	/* 
         *  I don't seriously expect a single base to translate sensibly, but
         *  they asked for it...
         */
	(void) ajStrAppK(&trn, 'X');
    }

    /* clean up */
    ajStrDel (&seq);
    ajStrDel (&value);

    return trn;
}

/* @func ajTrnGetTitle ********************************************************
**
** Returns the translation table description.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPTrn] Translation object.
** @return [AjPStr] Description as a string.
** @@
******************************************************************************/

AjPStr ajTrnGetTitle (AjPTrn thys) {
	
  return thys->Title;
}


/* @func ajTrnGetFileName ********************************************************
**
** Returns the file that the translation table was read from.
** Because this is a pointer to the real internal string
** the caller must take care not to change the character string in any way.
** If the string is to be changed (case for example) then it must first
** be copied.
**
** @param [u] thys [AjPTrn] Translation object.
** @return [AjPStr] File name as a string.
** @@
******************************************************************************/

AjPStr ajTrnGetFileName (AjPTrn thys) {
	
  return thys->FileName;
}



/* @func ajTrnStartStop ************************************************************
**
** Checks whether the input codon is a Start codon, a Stop codon or something else
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [AjPStr] codon to checks
** @param [w] aa [char *] returned translated amino acid (a char*, not a NULL-terminated array of char)
** @return [ajint] 1 if it is a start codon, -1 if it is a stop codon, else 0
** @@
******************************************************************************/

ajint ajTrnStartStop (AjPTrn trnObj, AjPStr codon, char *aa) {

  char * res=NULL;
  
int tc1 = trnconv[(ajint)res[0]];
int tc2 = trnconv[(ajint)res[1]];
int tc3 = trnconv[(ajint)res[2]];

  *aa = trnObj->GC[tc1][tc2][tc3];
  
  res = ajStrStr(codon);

  if (trnObj->Starts[tc1][tc2][tc3] == 'M')
      return 1;

  if (*aa == '*')
      return -1;

  return 0;
}

/* @func ajTrnStartStopC ************************************************************
**
** Checks whether a const char * codon is a Start codon, a Stop codon or something else
**
** @param [r] trnObj [AjPTrn] Translation tables
** @param [r] codon [char *] codon to translate (these 3 characters need not be NULL-terminated)
** @param [w] aa [char *] returned translated amino acid (a char*, not a NULL-terminated array of char)
** @return [ajint] 1 if it is a start codon, -1 if it is a stop codon, else 0
** @@
******************************************************************************/

ajint ajTrnStartStopC (AjPTrn trnObj, char *codon, char *aa) {

int tc1 = trnconv[(ajint)codon[0]];
int tc2 = trnconv[(ajint)codon[1]];
int tc3 = trnconv[(ajint)codon[2]];

  *aa = trnObj->GC[tc1][tc2][tc3];
  
  if (trnObj->Starts[tc1][tc2][tc3] == 'M')
    return 1;

  if (*aa == '*')
      return -1;

  return 0;
}



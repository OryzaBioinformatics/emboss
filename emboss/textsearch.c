/* @source textsearch application
**
** Search sequence documentation text. SRS or Entrez is faster.
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

int main(int argc, char **argv)
{

  AjPSeqall seqall;
  AjPSeq seq;
  AjPStr pattern;
  AjBool html;
  AjBool doheader;
  AjBool casesensitive;
  AjBool doname;
  AjBool dodesc;
  AjBool dousa;
  AjBool doacc;

  AjPFile outfile;

  AjPStr usa;
  AjPStr name;
  AjPStr acc;
  AjPStr altusa=ajStrNewC("-");		/* default name when the real name
  						is not known */
  AjPStr altname=ajStrNewC("-");
  AjPStr altacc=ajStrNewC("-     ");  						

  AjPStr desc = NULL;

  AjPPosRegexp exp = NULL;


  embInit ("textsearch", argc, argv);

  seqall = ajAcdGetSeqall ("sequence");
  pattern = ajAcdGetString ("pattern");
  outfile = ajAcdGetOutfile ("outfile");
  html = ajAcdGetBool("html");
  doheader = ajAcdGetBool("heading");
  casesensitive = ajAcdGetBool("casesensitive");
  dousa = ajAcdGetBool("usa");
  doname = ajAcdGetBool("name");
  doacc = ajAcdGetBool("accession");
  dodesc = ajAcdGetBool("description");

/* compile the regular expression with or without case-sensitivity */
  if (casesensitive) {
    exp = ajPosRegComp(pattern);
  } else {
    exp = ajPosRegCompCase(pattern);
  }

/* start the HTML table */
  if (html) {
    (void) ajFmtPrintF(outfile, "<table border cellpadding=4 bgcolor=\"#FFFFF0\">\n");
  }


/* print the header information */
  if (doheader) {
    if (html) {
      (void) ajFmtPrintF(outfile, "<tr>Search for: %S</tr>\n", pattern);
    } else {
      (void) ajFmtPrintF(outfile, "# Search for: %S\n", pattern);
    }
  }

  while (ajSeqallNext(seqall, &seq)) {

    ajSeqTrace(seq);
    name = ajSeqGetName(seq);
    (void) ajStrAss(&desc, ajSeqGetDesc(seq));

    if (ajStrLen(desc) && ajPosRegExec (exp, desc)) {
  
/* get the usa ('-' if unknown) */    
      usa = ajSeqGetUsa(seq);
      if (ajStrLen(usa) == 0)
          usa = altusa;

/* get the name ('-' if unknown) */    
      name = ajSeqGetName(seq);
      if (ajStrLen(name) == 0)
          name = altname;

/* get the accession number ('-' if unknown) */    
      acc = ajSeqGetAcc(seq);
      if (ajStrLen(acc) == 0)
          acc = altacc;

/* start table line */
      if (html) ajFmtPrintF(outfile, "<tr>");

      if (dousa) {
        if (html) {
          (void) ajFmtPrintF(outfile, "<td>%S</td>", usa);  
        } else {
/* 
Make the formatting nice:
                   
If this is the last item, don't put spaces or TABs after it.
Try to fit the name in 18 spaces, else just add a TAB after it
*/
        if (ajStrLen(usa) < 18) {
          if (doname || doacc || dodesc) {
            (void) ajFmtPrintF(outfile, "%-18.17S", usa);  
            } else {
              (void) ajFmtPrintF(outfile, "%S", usa);  
            }
          } else {
            (void) ajFmtPrintF(outfile, "%S", usa);  
            if (doname || doacc || dodesc) {
              (void) ajFmtPrintF(outfile, "\t");
            }
	  }
        }
      }

      if (doname) {
        if (html) {
          (void) ajFmtPrintF(outfile, "<td>%S</td>", name);  
        } else {
/* 
Make the formatting nice:

If this is the last item, don't put spaces or TABs after it.
Try to fit the name in 14 space, else just add a TAB after it
*/
          if (ajStrLen(name) < 14) {
            if (doacc || dodesc) {
              (void) ajFmtPrintF(outfile, "%-14.13S", name);  
            } else {
              (void) ajFmtPrintF(outfile, "%S", name);  
            }
          } else {
            (void) ajFmtPrintF(outfile, "%S", name);  
            if (doacc || dodesc) {
              (void) ajFmtPrintF(outfile, "\t");
            }
          }
        }
      }

      
      if (doacc) {
        if (html) {
          (void) ajFmtPrintF(outfile, "<td>%S</td>", acc);  
        } else {
          (void) ajFmtPrintF(outfile, "%S", acc);
        }
        if (dodesc) {
          (void) ajFmtPrintF(outfile, "\t");
        }
      }

      
      if (dodesc) {
        if (html) {
          (void) ajFmtPrintF(outfile, "<td>%S</td>", desc);  
        } else {
          (void) ajFmtPrintF(outfile, "%S", desc);  
        }
      }

/* end table line */
      if (html) {
        (void) ajFmtPrintF(outfile, "</tr>\n");
      } else {
        (void) ajFmtPrintF(outfile, "\n");
      }
    }

/* tidy up */
    ajStrClear(&desc);

  }


/* end the HTML table */
  if (html) {
    (void) ajFmtPrintF(outfile, "</table>\n");
  }

  (void) ajFileClose(&outfile);
  
/* tidy up */
  ajStrDel(&altusa);
  ajStrDel(&altname);
  ajStrDel(&altacc);
  ajPosRegFree(&exp);

  (void) ajExit();
  exit(0);
}


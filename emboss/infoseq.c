/* @source infoseq application
**
** Displays some simple information about sequences
**
** @author: Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
** @modified 04/02/2000 rbsk@sanger - added 'percent GC' computation
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

int main (int argc, char * argv[]) 
{
    
    AjPSeqall seqall;
    AjPSeq seq;
    
    AjBool html;
    AjBool doheader;
    AjBool dotype;
    AjBool dousa;
    AjBool doname;
    AjBool doacc;
    AjBool dolength;
    AjBool dodesc;
    AjBool dopgc;
    
    AjPFile outfile;
    
    AjPStr usa;
    AjPStr name;
    AjPStr acc;
    AjPStr altusa=ajStrNewC("-");	/* default name when the real name
					   is not known */
    AjPStr altname=ajStrNewC("-");
    AjPStr altacc=ajStrNewC("-     ");
    int length;
    AjBool type=ajTrue;			/* ajTrue if Protein */
    float pgc = 0.0;
    AjPStr desc;
    AjBool firsttime=ajTrue;
    
    (void) embInit ("infoseq", argc, argv);
    
    outfile = ajAcdGetOutfile ("outfile");
    
    seqall = ajAcdGetSeqall ("sequence");
    html = ajAcdGetBool("html");
    doheader = ajAcdGetBool("heading");
    dousa = ajAcdGetBool("usa");
    doname = ajAcdGetBool("name");
    doacc = ajAcdGetBool("accession");
    dotype = ajAcdGetBool("type");
    dolength = ajAcdGetBool("length");
    dopgc = ajAcdGetBool("pgc");
    dodesc = ajAcdGetBool("description");
    
    
    /* start the HTML table */
    if (html)
	(void) ajFmtPrintF(outfile,
			   "<table border cellpadding=4 bgcolor=\"#FFFFF0\">\n");
    
    while (ajSeqallNext(seqall, &seq))
    {
	ajSeqTrace(seq);
	if (firsttime)
	{
	    /* is this a protein or nucleic sequence? */
	    type = ajSeqIsProt(seq);

	    /* print the header information */
	    if (doheader)
	    {
		if (html)	/* start the HTML table title line and
				   output the Name header */
		    (void) ajFmtPrintF(outfile, "<tr>");
		else
		    (void) ajFmtPrintF(outfile, "%s", "# ");
    
		if (dousa)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>USA</th>");
		    else
			(void) ajFmtPrintF(outfile, "%-16s", "USA");
		}

		if (doname)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>Name</th>");
		    else
			(void) ajFmtPrintF(outfile, "%-12s", "Name");
		}

		if (doacc)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>Accession</th>");
		    else
			(void) ajFmtPrintF(outfile, "%s", "Accession ");
		}

		if (dotype)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>Type</th>");
		    else
			(void) ajFmtPrintF(outfile, "Type ");
		}

		if (dolength)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>Length</th>");
		    else
			(void) ajFmtPrintF(outfile, "Length\t");
		}

		if (!type && dopgc)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>%%GC</th>");
		    else
			(void) ajFmtPrintF(outfile, " %%GC   ");
		}

		if (dodesc)
		{
		    if (html)
			(void) ajFmtPrintF(outfile, "<th>Description</th>");
		    else
			(void) ajFmtPrintF(outfile, "Description");
		}

		/* end the HTML table title line */
		if (html)
		    (void) ajFmtPrintF(outfile, "</tr>\n");
		else
		    (void) ajFmtPrintF(outfile, "\n");
	    }
	    firsttime = ajFalse;
	}  	


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

	length = ajSeqLen(seq);
	if(dopgc && !type)
	{
	    AjPStr seqstr ;
	    seqstr = ajSeqStr(seq);
	    pgc = ajMeltGC(&seqstr,length);
	    pgc *= 100;			/* percentage */
	}
	desc = ajSeqGetDesc(seq);

	/* start table line */
	if (html) ajFmtPrintF(outfile, "<tr>");

	if (dousa)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<td>%S</td>", usa);  
	    else
	    {
		/* 
		   Make the formatting nice:
		   
		   If this is the last item, don't put spaces or TABs after it.
		   Try to fit the name in 18 spaces, else just add a TAB after it
		   */
		if (ajStrLen(usa) < 18)
		{
		    if (doname || doacc || dotype || dolength ||
			(!type && dopgc) || dodesc)
			(void) ajFmtPrintF(outfile, "%-18.17S", usa);  
		    else
			(void) ajFmtPrintF(outfile, "%S", usa);  
		}
		else
		{
		    (void) ajFmtPrintF(outfile, "%S", usa);  
		    if (doname || doacc || dotype || dolength ||
			(!type && dopgc) || dodesc)
			(void) ajFmtPrintF(outfile, "\t");
		}
	    }
	}
	if (doname)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<td>%S</td>", name);  
	    else
	    {
		/* 
		   Make the formatting nice:
		   
		   If this is the last item, don't put spaces or TABs after it.
		   Try to fit the name in 14 space, else just add a TAB after it
		   */
		if (ajStrLen(name) < 14)
		{
		    if (doacc || dotype || dolength || (!type && dopgc) ||
			dodesc)
			(void) ajFmtPrintF(outfile, "%-14.13S", name);  
		    else
			(void) ajFmtPrintF(outfile, "%S", name);  
		}
		else
		{
		    (void) ajFmtPrintF(outfile, "%S", name);  
		    if (doacc || dotype || dolength || (!type && dopgc) ||
			dodesc)
			(void) ajFmtPrintF(outfile, "\t");
		}
	    }
	}

	if (doacc)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<td>%S</td>", acc);
	    else
	    {
		(void) ajFmtPrintF(outfile, "%S", acc);
		if (dotype || dolength || (!type && dopgc) || dodesc)
		    (void) ajFmtPrintF(outfile, "\t");
	    }    	
	}    

	if (dotype)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<td>%c</td>", type?'P':'N');  
	    else
	    {
		(void) ajFmtPrintF(outfile, "%c", type?'P':'N');  
		if (dolength || (!type && dopgc) || dodesc)
		    (void) ajFmtPrintF(outfile, "    ");  
	    }
	}

	if (dolength)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<td>%d</td>", length);  
	    else
	    {
		(void) ajFmtPrintF(outfile, "%d", length);  
		if ((!type && dopgc) || dodesc )
		    (void) ajFmtPrintF(outfile, "\t");  
	    }
	}

	if (!type && dopgc)
	{
	    if (html)
	    {
		if(!type)
		    (void) ajFmtPrintF(outfile, "<td>%-6.2f</td>", pgc);
		else
		    (void) ajFmtPrintF(outfile, "<td></td>");
	    }
	    else
	    {
		/* don't use %-6.2f here as we guaranteed there would be no
		   trailing spaces */
		(void) ajFmtPrintF(outfile, "%6.2f", pgc);  
		if (dodesc)
		    (void) ajFmtPrintF(outfile, " ");  
	    }
	}

	if (dodesc)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<td>%S</td>", desc);  
	    else
	    {
		(void) ajFmtPrintF(outfile, "%S", desc);  
	    }
	}

	/* end table line */
	if (html)
	    (void) ajFmtPrintF(outfile, "</tr>\n");
	else
	    (void) ajFmtPrintF(outfile, "\n");
    }


    /* end the HTML table */
    if (html)
	(void) ajFmtPrintF(outfile, "</table>\n");

    (void) ajFileClose(&outfile);

    /* tidy up */
    ajStrDel(&altusa);
    ajStrDel(&altname);
    ajStrDel(&altacc);

    (void) ajExit();
    exit(0);
}

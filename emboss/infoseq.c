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




/* @prog infoseq **************************************************************
**
** Displays some simple information about sequences
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPSeqall seqall;
    AjPSeq seq;

    AjBool html;
    AjBool doheader;
    AjBool dotype;
    AjBool dousa;
    AjBool doname;
    AjBool doacc;
    AjBool dogi;
    AjBool dosv;
    AjBool dolength;
    AjBool dodesc;
    AjBool dopgc;

    AjPFile outfile;

    const AjPStr usa;
    const AjPStr name;
    const AjPStr acc;
    const AjPStr gi;
    const AjPStr sv;
    const AjPStr desc;
    AjPStr altusa;	/* default name when the real name is not known */
    AjPStr altname;
    AjPStr altacc;
    AjPStr altgi;
    AjPStr altsv;
    ajint length;
    AjBool type = ajTrue;			/* ajTrue if Protein */
    float pgc = 0.0;
    AjBool firsttime = ajTrue;

    embInit("infoseq", argc, argv);

    outfile  = ajAcdGetOutfile("outfile");
    seqall   = ajAcdGetSeqall("sequence");
    html     = ajAcdGetBool("html");
    doheader = ajAcdGetBool("heading");
    dousa    = ajAcdGetBool("usa");
    doname   = ajAcdGetBool("name");
    doacc    = ajAcdGetBool("accession");
    dogi     = ajAcdGetBool("gi");
    dosv     = ajAcdGetBool("version");
    dotype   = ajAcdGetBool("type");
    dolength = ajAcdGetBool("length");
    dopgc    = ajAcdGetBool("pgc");
    dodesc   = ajAcdGetBool("description");


    altusa  = ajStrNewC("-");
    altname = ajStrNewC("-");
    altacc  = ajStrNewC("-     ");
    altgi   = ajStrNewC("-     ");
    altsv   = ajStrNewC("-     ");

    /* start the HTML table */
    if(html)
	ajFmtPrintF(outfile,"<table border cellpadding=4 bgcolor=\"#FFFFF0"
		    "\">\n");

    while(ajSeqallNext(seqall, &seq))
    {
	ajSeqTrim(seq);
	ajSeqTrace(seq);

        /* is this a protein or nucleic sequence? */
        type = ajSeqIsProt(seq);

	if(firsttime)
	{
	    /* print the header information */
	    if(doheader)
	    {
		/* start the HTML table title line and output Name header */
		if(html)
		    ajFmtPrintF(outfile, "<tr>");
		else
		    ajFmtPrintF(outfile, "%s", "# ");

		if(dousa)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>USA</th>");
		    else
			ajFmtPrintF(outfile, "%-16s", "USA");
		}

		if(doname)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>Name</th>");
		    else
			ajFmtPrintF(outfile, "%-12s", "Name");
		}

		if(doacc)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>Accession</th>");
		    else
			ajFmtPrintF(outfile, "%s", "Accession ");
		}

		if(dogi)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>GI</th>");
		    else
			ajFmtPrintF(outfile, "%s", "GI        ");
		}

		if(dosv)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>Version</th>");
		    else
			ajFmtPrintF(outfile, "%s", "Version   ");
		}

		if(dotype)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>Type</th>");
		    else
			ajFmtPrintF(outfile, "Type ");
		}

		if(dolength)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>Length</th>");
		    else
			ajFmtPrintF(outfile, "Length\t");
		}

		if(!type && dopgc)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>%%GC</th>");
		    else
			ajFmtPrintF(outfile, " %%GC   ");
		}

		if(dodesc)
		{
		    if(html)
			ajFmtPrintF(outfile, "<th>Description</th>");
		    else
			ajFmtPrintF(outfile, "Description");
		}

		/* end the HTML table title line */
		if(html)
		    ajFmtPrintF(outfile, "</tr>\n");
		else
		    ajFmtPrintF(outfile, "\n");
	    }
	    firsttime = ajFalse;
	}


	/* get the usa ('-' if unknown) */
	usa = ajSeqGetUsa(seq);
	if(ajStrLen(usa) == 0)
	    usa = altusa;

	/* get the name ('-' if unknown) */
	name = ajSeqGetName(seq);
	if(ajStrLen(name) == 0)
	    name = altname;

	/* get the accession number ('-' if unknown) */
	acc = ajSeqGetAcc(seq);
	if(ajStrLen(acc) == 0)
	    acc = altacc;

	/* get the GI number ('-' if unknown) */
	gi = ajSeqGetGi(seq);
	if(ajStrLen(gi) == 0)
	    gi = altgi;

	/* get the version number ('-' if unknown) */
	sv = ajSeqGetSv(seq);
	if(ajStrLen(sv) == 0)
	    sv = altsv;

	length = ajSeqLen(seq);
	if(dopgc && !type)
	{
	    pgc = ajMeltGC(ajSeqStr(seq),length);
	    pgc *= 100;			/* percentage */
	}
	desc = ajSeqGetDesc(seq);

	/* start table line */
	if(html)
	    ajFmtPrintF(outfile, "<tr>");

	if(dousa)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%S</td>", usa);
	    else
	    {
		/*
		**  Format:
		**
		**  If this is the last item, don't put spaces or TABs after
		**  it. Try to fit the name in 18 spaces, else just add a
		**  TAB after it
		*/
		if(ajStrLen(usa) < 18)
		{
		    if(doname || doacc || dogi || dosv || dotype || dolength ||
			(!type && dopgc) || dodesc)
			ajFmtPrintF(outfile, "%-18.17S", usa);
		    else
			ajFmtPrintF(outfile, "%S", usa);
		}
		else
		{
		    ajFmtPrintF(outfile, "%S", usa);
		    if(doname || doacc || dogi || dosv || dotype || dolength ||
			(!type && dopgc) || dodesc)
			ajFmtPrintF(outfile, "\t");
		}
	    }
	}

	if(doname)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%S</td>", name);
	    else
	    {
		/*
		**  Format:
		**
		**  If this is the last item, don't put spaces or TABs after
		**  it. Try to fit the name in 14 space, else just add a
		**  TAB after it
		*/
		if(ajStrLen(name) < 14)
		{
		    if(doacc || dogi || dosv || dotype || dolength ||
		        (!type && dopgc) || dodesc)
			ajFmtPrintF(outfile, "%-14.13S", name);
		    else
			ajFmtPrintF(outfile, "%S", name);
		}
		else
		{
		    ajFmtPrintF(outfile, "%S", name);
		    if(doacc || dogi || dosv || dotype || dolength ||
		        (!type && dopgc) || dodesc)
			ajFmtPrintF(outfile, "\t");
		}
	    }
	}

	if(doacc)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%S</td>", acc);
	    else
	    {
		ajFmtPrintF(outfile, "%S", acc);
		if(dogi || dosv || dotype || dolength ||
		    (!type && dopgc) || dodesc)
		    ajFmtPrintF(outfile, "\t");
	    }
	}

	if(dogi)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%S</td>", gi);
	    else
	    {
		ajFmtPrintF(outfile, "%S", gi);
		if(dosv || dotype || dolength ||
		    (!type && dopgc) || dodesc)
		    ajFmtPrintF(outfile, "\t");
	    }
	}

	if(dosv)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%S</td>", sv);
	    else
	    {
		ajFmtPrintF(outfile, "%S", sv);
		if(dotype || dolength ||
		    (!type && dopgc) || dodesc)
		    ajFmtPrintF(outfile, "\t");
	    }
	}

	if(dotype)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%c</td>", type?'P':'N');
	    else
	    {
		ajFmtPrintF(outfile, "%c", type?'P':'N');
		if(dolength || (!type && dopgc) || dodesc)
		    ajFmtPrintF(outfile, "    ");
	    }
	}

	if(dolength)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%d</td>", length);
	    else
	    {
		ajFmtPrintF(outfile, "%d", length);
		if((!type && dopgc) || dodesc )
		    ajFmtPrintF(outfile, "\t");
	    }
	}

	if(!type && dopgc)
	{
	    if(html)
	    {
		if(!type)
		    ajFmtPrintF(outfile, "<td>%-6.2f</td>", pgc);
		else
		    ajFmtPrintF(outfile, "<td></td>");
	    }
	    else
	    {
		/* don't use %-6.2f here as there are no trailing spaces */
		ajFmtPrintF(outfile, "%6.2f", pgc);
		if(dodesc)
		    ajFmtPrintF(outfile, " ");
	    }
	}

	if(dodesc)
	{
	    if(html)
		ajFmtPrintF(outfile, "<td>%S</td>", desc);
	    else
		ajFmtPrintF(outfile, "%S", desc);
	}

	/* end table line */
	if(html)
	    ajFmtPrintF(outfile, "</tr>\n");
	else
	    ajFmtPrintF(outfile, "\n");
    }


    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");

    ajFileClose(&outfile);

    ajStrDel(&altusa);
    ajStrDel(&altname);
    ajStrDel(&altacc);
    ajStrDel(&altsv);
    ajStrDel(&altgi);

    ajExit();

    return 0;
}

/* @source showdb application
**
** Displays information on the currently available databases
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




static void showdb_DBOut(AjPFile outfile,
			 const AjPStr dbname, const AjPStr type,
			 AjBool id, AjBool qry, AjBool all,
			 const AjPStr comment,
			 const AjPStr release, AjBool html, AjBool dotype,
			 AjBool doid, AjBool doqry, AjBool doall,
			 AjBool dofields, AjBool docomment, AjBool dorelease);

static AjPStr showdb_GetFields(const AjPStr dbname);




/* @prog showdb ***************************************************************
**
** Displays information on the currently available databases
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjBool html;
    AjBool protein;
    AjBool nucleic;
    AjBool doheader;
    AjBool dotype;
    AjBool doid;
    AjBool doqry;
    AjBool doall;
    AjBool dofields;
    AjBool docomment;
    AjBool dorelease;
    AjBool only;

    AjPFile outfile = NULL;
    AjPStr dbname   = NULL;	/* the next database name to look at */
    AjPStr type     = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr comment = NULL;
    AjPStr release = NULL;

    AjPList dbnames;
    AjIList iter = NULL;

    ajNamSetControl("namvalid");	/* validate database/resource defs */

    embInit("showdb", argc, argv);

    dbname  = ajAcdGetString("database");
    outfile = ajAcdGetOutfile("outfile");

    html    = ajAcdGetBool("html");
    protein = ajAcdGetBool("protein");
    nucleic = ajAcdGetBool("nucleic");

    doheader  = ajAcdGetBool("heading");
    dotype    = ajAcdGetBool("type");
    doid      = ajAcdGetBool("id");
    doqry     = ajAcdGetBool("query");
    doall     = ajAcdGetBool("all");
    dofields  = ajAcdGetBool("fields");
    docomment = ajAcdGetBool("comment");
    dorelease = ajAcdGetBool("release");
    only      = ajAcdGetBool("only");
    
    dbnames = ajListstrNew();
    
    
    /* start the HTML table */
    if(html)
	ajFmtPrintF(outfile, "<table border cellpadding=4 bgcolor="
		    "\"#FFFFF0\">\n");
    
    
    /* print the header information */
    if(doheader)
    {
	if(html)
	    /* start the HTML table title line and output the Name header */
	    ajFmtPrintF(outfile, "<tr><th>Name</th>");
	else
	    ajFmtPrintF(outfile, "%-14.13s", "# Name");

	if(dotype)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Type</th>");
	    else
		ajFmtPrintF(outfile, "Type ");
	}

	if(doid)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>ID</th>");
	    else
		ajFmtPrintF(outfile, "ID  ");
	}

	if(doqry)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Qry</th>");
	    else
		ajFmtPrintF(outfile, "Qry ");
	}

	if(doall)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>All</th>");
	    else
		ajFmtPrintF(outfile, "All ");
	}

	if(dofields)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Fields</th>");
	    else
		ajFmtPrintF(outfile, "Fields ");
	}

	if(dorelease)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Release</th>");
	    else
		ajFmtPrintF(outfile, "Release\t");
	}

	if(docomment)
	{
	    if(html)
		ajFmtPrintF(outfile, "<th>Comment</th>");
	    else
		ajFmtPrintF(outfile, "Comment");
	}

	if(html)
	    /* end the HTML table title line */
	    ajFmtPrintF(outfile, "</tr>\n");
	else
	{
	    ajFmtPrintF(outfile, "\n");

	    ajFmtPrintF(outfile, "%-14.13s", "# ====");
	    if(dotype)
		ajFmtPrintF(outfile, "==== ");

	    if(doid)
		ajFmtPrintF(outfile, "==  ");

	    if(doqry)
		ajFmtPrintF(outfile, "=== ");

	    if(doall)
		ajFmtPrintF(outfile, "=== ");

	    if(dofields)
		ajFmtPrintF(outfile, "====== ");

	    if(dorelease)
		ajFmtPrintF(outfile, "=======\t");

	    if(docomment)
		ajFmtPrintF(outfile, "=======");

	    ajFmtPrintF(outfile, "\n");
	}
    }
    
    
    /* Just one specified name to get details on? */
    if(ajStrLen(dbname))
    {
	if(ajNamDbDetails(dbname, &type, &id, &qry, &all, &comment,
			  &release))
	    showdb_DBOut(outfile, dbname, type, id, qry, all, comment,
			 release, html, dotype, doid, doqry, doall, 
			 dofields, docomment, dorelease);
	else
	    ajFatal("The database '%S' does not exist", dbname);
    }
    else
    {
	/* get the list of database names */
	ajNamListListDatabases(dbnames);

	/* sort it */
	ajListSort(dbnames, ajStrCmp);

	/* iterate through the dbnames list */
	iter = ajListIterRead(dbnames);

	/* write out protein databases */
	while((dbname = ajListIterNext(iter)) != NULL)
	    if(ajNamDbDetails(dbname, &type, &id, &qry, &all, &comment,
			      &release))
	    {
		if(!ajStrCmpC(type, "P") && protein)
		    showdb_DBOut(outfile, dbname, type, id, qry, all,
				 comment, release, html, dotype, doid,
				 doqry, doall, dofields, docomment, 
				 dorelease);
	    }
	    else
		ajFatal("The database '%S' does not exist", dbname);


	/* reset the iterator */
	ajListIterFree(&iter);
	iter = ajListIterRead(dbnames);

	/* now write out nucleic databases */
	while((dbname = ajListIterNext(iter)) != NULL)
	{
	    if(ajNamDbDetails(dbname, &type, &id, &qry, &all, &comment,
			      &release))
	    {
		if(!ajStrCmpC(type, "N") && nucleic)
		    showdb_DBOut(outfile, dbname, type, id, qry, all,
				 comment, release, html, dotype, doid,
				 doqry, doall, dofields, docomment, 
				 dorelease);
	    }
	    else
		ajFatal("The database '%S' does not exist", dbname);
	}

	ajListIterFree(&iter);
	ajListDel(&dbnames);
    }
    
    /* end the HTML table */
    if(html)
	ajFmtPrintF(outfile, "</table>\n");
    
    ajFileClose(&outfile);
    
    ajExit();

    return 0;
}




/* @funcstatic showdb_DBOut ***************************************************
**
** Output db information
**
** @param [w] outfile [AjPFile] outfile
** @param [r] dbname [const AjPStr] database name
** @param [r] type [const AjPStr] type
** @param [r] id [AjBool] id
** @param [r] qry [AjBool] queryable
** @param [r] all [AjBool] all info
** @param [r] comment [const AjPStr] db comment
** @param [r] release [const AjPStr] db release
** @param [r] html [AjBool] do html
** @param [r] dotype [AjBool] show type
** @param [r] doid [AjBool] show id
** @param [r] doqry [AjBool] show query status
** @param [r] doall [AjBool] show everything
** @param [r] dofields [AjBool] show query fields
** @param [r] docomment [AjBool] show comment
** @param [r] dorelease [AjBool] show release
** @@
******************************************************************************/

static void showdb_DBOut(AjPFile outfile,
			 const AjPStr dbname, const AjPStr type,
			 AjBool id, AjBool qry, AjBool all,
			 const AjPStr comment,
			 const AjPStr release, AjBool html, AjBool dotype,
			 AjBool doid, AjBool doqry, AjBool doall,
			 AjBool dofields, AjBool docomment, AjBool dorelease)
{

    if(html)
	/* start table line and output name */
	ajFmtPrintF(outfile, "<tr><td>%S</td>", dbname);
    else
    {
	/* if the name is shorter than 14 characters make a nice formatted
	   output, otherwise, just output it and a space */
	if(ajStrLen(dbname) < 14)
	    ajFmtPrintF(outfile, "%-14.13S", dbname);
	else
	    ajFmtPrintF(outfile, "%S ", dbname);
    }

    if(dotype)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>%S</td>", type);
	else
	    ajFmtPrintF(outfile, "%S    ", type);
    }

    if(doid)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(id)
	    ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    ajFmtPrintF(outfile, "%s", "-   ");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doqry)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(qry)
	    ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    ajFmtPrintF(outfile, "%s", "-   ");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(doall)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(all)
	    ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    ajFmtPrintF(outfile, "%s", "-   ");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(dofields)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

        ajFmtPrintF(outfile, "%S ", showdb_GetFields(dbname));

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(dorelease)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(release != NULL)
	    ajFmtPrintF(outfile, "%S\t", release);
	else
	    ajFmtPrintF(outfile, "-\t");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }


    if(docomment)
    {
	if(html)
	    ajFmtPrintF(outfile, "<td>");

	if(comment != NULL)
	    ajFmtPrintF(outfile, "%S", comment);
	else
	    ajFmtPrintF(outfile, "-");

	if(html)
	    ajFmtPrintF(outfile, "</td>");
    }

    if(html)
	ajFmtPrintF(outfile, "</tr>\n");	/* end table line */
    else
	ajFmtPrintF(outfile, "\n");

    return;
}




/* @funcstatic showdb_GetFields **********************************************
**
** Get a database's valid query fields (apart from the default 'id' and 'acc')
**
** @param [r] dbname [const AjPStr] database name
** @return [AjPStr] the available search fields
** @@
******************************************************************************/

static AjPStr showdb_GetFields(const AjPStr dbname)
{
    static AjPStr str = NULL;
    AjPSeqQuery query;


    query = ajSeqQueryNew();

    ajStrAssS(&query->DbName, dbname);
    ajNamDbData(query);
    ajStrAssS(&str, query->DbFields);

    /* if there are no query fields, then change to a '_' */
    if(str == NULL || ajStrMatchC(str, ""))
  	ajStrAssC(&str, "-     ");
    else
	/* change spaces to commas to make the result one word */
	ajStrConvertCC(&str, " ", ",");

    return str;
}

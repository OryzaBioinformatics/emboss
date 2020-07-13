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

static void showdb_DBOut(AjPFile outfile, AjPStr dbname, AjPStr type,
			 AjBool id, AjBool qry, AjBool all, AjPStr comment,
			 AjPStr release, AjBool html, AjBool dotype,
			 AjBool doid, AjBool doqry, AjBool doall,
			 AjBool docomment, AjBool dorelease);


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
    AjBool docomment;
    AjBool dorelease;
    AjBool only;

    AjPFile outfile = NULL;
    AjPStr dbname = NULL;	/* the next database name to look at */
    AjPStr type = NULL;
    AjBool id;
    AjBool qry;
    AjBool all;
    AjPStr comment = NULL;
    AjPStr release = NULL;

    AjPList dbnames = ajListstrNew();
    AjIList iter = NULL;

    (void) embInit ("showdb", argc, argv);

    dbname  = ajAcdGetString("database");
    outfile = ajAcdGetOutfile ("outfile");

    html = ajAcdGetBool("html");
    protein = ajAcdGetBool("protein");
    nucleic = ajAcdGetBool("nucleic");

    doheader = ajAcdGetBool("heading");
    dotype = ajAcdGetBool("type");
    doid = ajAcdGetBool("id");
    doqry = ajAcdGetBool("query");
    doall = ajAcdGetBool("all");
    docomment = ajAcdGetBool("comment");
    dorelease = ajAcdGetBool("release");
    only = ajAcdGetBool("only"); /* not needed, but users can set all */
				 /* that depend on it to make it */
				 /* "unused" otherwise */

    /* start the HTML table */
    if (html)
	(void) ajFmtPrintF(outfile, "<table border cellpadding=4 bgcolor="
			   "\"#FFFFF0\">\n");


    /* print the header information */
    if (doheader)
    {
	if (html)
	    /* start the HTML table title line and output the Name header */
	    (void) ajFmtPrintF(outfile, "<tr><th>Name</th>");
	else
	    (void) ajFmtPrintF(outfile, "%-14.13s", "# Name");

	if (dotype)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Type</th>");
	    else
		(void) ajFmtPrintF(outfile, "Type ");
	}

	if (doid)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>ID</th>");
	    else
		(void) ajFmtPrintF(outfile, "ID  ");
	}

	if (doqry)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Qry</th>");
	    else
		(void) ajFmtPrintF(outfile, "Qry ");
	}

	if (doall)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>All</th>");
	    else
		(void) ajFmtPrintF(outfile, "All ");
	}

	if (dorelease)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Release</th>");
	    else
		(void) ajFmtPrintF(outfile, "Release\t");
	}

	if (docomment)
	{
	    if (html)
		(void) ajFmtPrintF(outfile, "<th>Comment</th>");
	    else
		(void) ajFmtPrintF(outfile, "Comment");
	}

	if (html)
	    /* end the HTML table title line */
	    (void) ajFmtPrintF(outfile, "</tr>\n");
	else
	{
	    (void) ajFmtPrintF(outfile, "\n");

	    (void) ajFmtPrintF(outfile, "%-14.13s", "# ====");
	    if (dotype)
		(void) ajFmtPrintF(outfile, "==== ");

	    if (doid)
		(void) ajFmtPrintF(outfile, "==  ");

	    if (doqry)
		(void) ajFmtPrintF(outfile, "=== ");

	    if (doall)
		(void) ajFmtPrintF(outfile, "=== ");

	    if (dorelease)
		(void) ajFmtPrintF(outfile, "=======\t");

	    if (docomment)
		(void) ajFmtPrintF(outfile, "=======");

	    (void) ajFmtPrintF(outfile, "\n");
	}
    }


    /* do we have just one specified name to get details on? */
    if (ajStrLen(dbname))
    {
	if (ajNamDbDetails (dbname, &type, &id, &qry, &all, &comment,
			    &release))
	    (void) showdb_DBOut(outfile, dbname, type, id, qry, all, comment,
				release, html, dotype, doid, doqry, doall,
				docomment, dorelease);
	else
	    (void) ajDie("The database '%S' does not exist", dbname);
    }
    else
    {
	/* get the list of database names */
	(void) ajNamListListDatabases (dbnames);

	/* sort it */
	(void) ajListSort(dbnames, ajStrCmp);

	/* iterate through the dbnames list */
	iter = ajListIter(dbnames);

	/* write out protein databases */
	while ((dbname = ajListIterNext(iter)) != NULL)
	    if (ajNamDbDetails (dbname, &type, &id, &qry, &all, &comment,
				&release))
	    {
		if (!ajStrCmpC(type, "P") && protein)
		    (void) showdb_DBOut(outfile, dbname, type, id, qry, all,
					comment, release, html, dotype, doid,
					doqry, doall, docomment, dorelease);
	    }
	    else
		(void) ajDie("The database '%S' does not exist", dbname);


	/* reset the iterator */
	(void) ajListIterFree(iter);
	iter = ajListIter(dbnames);

	/* now write out nucleic databases */
	while ((dbname = ajListIterNext(iter)) != NULL)
	{
	    if (ajNamDbDetails (dbname, &type, &id, &qry, &all, &comment,
				&release))
	    {
		if (!ajStrCmpC(type, "N") && nucleic)
		    (void) showdb_DBOut(outfile, dbname, type, id, qry, all,
					comment, release, html, dotype, doid,
					doqry, doall, docomment, dorelease);
	    }
	    else
		(void) ajDie("The database '%S' does not exist", dbname);
	}

	(void) ajListIterFree(iter);
    }

    /* end the HTML table */
    if (html)
	(void) ajFmtPrintF(outfile, "</table>\n");

    (void) ajFileClose(&outfile);

    ajExit();
    return 0;
}


/* @funcstatic showdb_DBOut ***************************************************
**
** Output db information
**
** @param [w] outfile [AjPFile] outfile
** @param [r] dbname [AjPStr] database name
** @param [r] type [AjPStr] type
** @param [r] id [AjBool] id
** @param [r] qry [AjBool] queryable
** @param [r] all [AjBool] all info
** @param [r] comment [AjPStr] db comment
** @param [r] release [AjPStr] db release
** @param [r] html [AjBool] do html
** @param [r] dotype [AjBool] show type
** @param [r] doid [AjBool] show id
** @param [r] doqry [AjBool] show query status
** @param [r] doall [AjBool] show everything
** @param [r] docomment [AjBool] show comment
** @param [r] dorelease [AjBool] show release
** @@
******************************************************************************/

static void showdb_DBOut(AjPFile outfile, AjPStr dbname, AjPStr type,
			 AjBool id, AjBool qry, AjBool all, AjPStr comment,
			 AjPStr release, AjBool html, AjBool dotype,
			 AjBool doid, AjBool doqry, AjBool doall,
			 AjBool docomment, AjBool dorelease)
{

    if (html)
	/* start table line and output name */
	(void) ajFmtPrintF(outfile, "<tr><td>%S</td>", dbname);
    else
    {
	/* if the name is shorter than 14 characters make a nice formatted
	   output, otherwise, just output it and a space */
	if (ajStrLen(dbname) < 14)
	    (void) ajFmtPrintF(outfile, "%-14.13S", dbname);
	else
	    (void) ajFmtPrintF(outfile, "%S ", dbname);
    }

    if (dotype)
    {
	if (html)
	    (void) ajFmtPrintF(outfile, "<td>%S</td>", type);
	else
	    (void) ajFmtPrintF(outfile, "%S    ", type);
    }

    if (doid)
    {
	if (html)
	    (void) ajFmtPrintF(outfile, "<td>");

	if (id)
	    (void) ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    (void) ajFmtPrintF(outfile, "%s", "-   ");

	if (html)
	    (void) ajFmtPrintF(outfile, "</td>");
    }

    if (doqry)
    {
	if (html)
	    (void) ajFmtPrintF(outfile, "<td>");

	if (qry)
	    (void) ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    (void) ajFmtPrintF(outfile, "%s", "-   ");

	if (html)
	    (void) ajFmtPrintF(outfile, "</td>");
    }

    if (doall)
    {
	if (html)
	    (void) ajFmtPrintF(outfile, "<td>");

	if (all)
	    (void) ajFmtPrintF(outfile, "%s", "OK  ");
	else
	    (void) ajFmtPrintF(outfile, "%s", "-   ");

	if (html)
	    (void) ajFmtPrintF(outfile, "</td>");
    }

    if (dorelease)
    {
	if (html)
	    (void) ajFmtPrintF(outfile, "<td>");

	if (release != NULL)
	    (void) ajFmtPrintF(outfile, "%S\t", release);
	else
	    (void) ajFmtPrintF(outfile, "-\t");

	if (html)
	    (void) ajFmtPrintF(outfile, "</td>");
    }


    if (docomment)
    {
	if (html)
	    (void) ajFmtPrintF(outfile, "<td>");

	if (comment != NULL)
	    (void) ajFmtPrintF(outfile, "%S", comment);
	else
	    (void) ajFmtPrintF(outfile, "-");

	if (html)
	    (void) ajFmtPrintF(outfile, "</td>");
    }

    if (html)
	(void) ajFmtPrintF(outfile, "</tr>\n");	/* end table line */
    else
	(void) ajFmtPrintF(outfile, "\n");

    return;
}

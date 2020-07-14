/* @source tfm application
**
** Displays a program's help documentation manual
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




static void tfm_FindAppDocRoot(AjPStr* docroot);
static AjBool tfm_FindAppDoc(AjPStr program, AjBool html, AjPStr* path);




/* @prog tfm ******************************************************************
**
** Displays a program's help documentation manual
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPFile outfile = NULL;
    AjPStr program  = NULL;
    AjBool html;
    AjBool more;
    AjPStr path = NULL;			/* path of file to be displayed */
    AjPStr cmd  = NULL;			/* command line for running 'more' */
    AjPFile infile = NULL;
    AjPStr line    = NULL;		/* buffer for infile lines */
    AjPStr pager   = NULL;

    char *shellpager = NULL;

    embInit("tfm", argc, argv);
    
    program = ajAcdGetString("program");
    outfile = ajAcdGetOutfile("outfile");
    html    = ajAcdGetBool("html");
    more    = ajAcdGetBool("more");

    cmd   = ajStrNew();
    path  = ajStrNew();
    pager = ajStrNew();
    line  = ajStrNew();
    
    
    /* is a search string specified - should be tested in tfm.acd file */
    if(!ajStrLen(program))
	ajFatal("No program name specified.");
    
    if(!tfm_FindAppDoc(program, html, &path))
	ajFatal("The documentation for program '%S' was not found.", program);
    
    /* outputing to STDOUT and piping through 'more'? */
    if(ajFileStdout(outfile) && more)
    {
	if(!ajNamGetValueC("PAGER",&pager))
	{
	    shellpager = getenv("PAGER");
	    if(shellpager)
		ajStrAssC(&pager,shellpager);
	    if(!ajStrLen(pager))
		ajStrAssC(&pager,"more");
	}
	ajFmtPrintS(&cmd,"%S %S",pager,path);
	ajSystem(&cmd);
    }
    else
    {
	/* output file as-is */
	infile = ajFileNewIn(path);

	while(ajFileGets(infile, &line))
	    ajFmtPrintF(outfile, "%S", line);

	ajFileClose(&infile);
    }
    
    ajFileClose(&outfile);


    ajStrDel(&path);
    ajStrDel(&pager);
    ajStrDel(&line);
    ajStrDel(&cmd);
    
    ajExit();

    return 0;
}




/* @funcstatic tfm_FindAppDocRoot *********************************************
**
** return the path to the program doc directory
**
** @param [w] docroot [AjPStr*] root dir for documentation files
** @@
******************************************************************************/

static void tfm_FindAppDocRoot(AjPStr* docroot)
{

    AjPStr docrootinst = NULL;

    ajNamGetValueC("docroot", docroot);

    /* look at EMBOSS doc files */

    /* try to open the installed doc directory */
    if(!ajStrLen(*docroot))
    {
	ajNamRootInstall(&docrootinst);
	ajFileDirFix(&docrootinst);
	ajFmtPrintS(docroot, "%Sshare/EMBOSS/doc/",
		    docrootinst);
    }
    ajFileDirFix(docroot);
    ajFmtPrintAppS(docroot, "programs/");

    if(!ajFileDir(docroot))
    {
	/*
	**  if that didn't work then try the doc directory from the
	**  distribution tarball
	*/
	ajNamRootBase(docroot);
	ajFileDirFix(docroot);

	if(ajFileDir(docroot))
	    ajStrAppC(docroot, "doc/programs/");
    }

    ajStrDel(&docrootinst);

    return;
}




/* @funcstatic tfm_FindAppDoc *************************************************
**
** return the path to the program documentation html or text file
**
** @param [r] program [AjPStr] program name
** @param [r] html [AjBool] whether html required
** @param [w] path [AjPStr*] returned path
** @return [AjBool] success
** @@
******************************************************************************/

static AjBool tfm_FindAppDoc(AjPStr program, AjBool html, AjPStr* path)
{
    AjPStr docroot = NULL;

    docroot = ajStrNew();

    tfm_FindAppDocRoot(&docroot);

    if(html)
    {
	ajStrAppC(&docroot, "html/");
	ajStrAss(path, docroot);
	ajStrApp(path, program);
	ajStrAppC(path, ".html");
    }
    else
    {
	ajStrAppC(&docroot, "text/");
	ajStrAss(path, docroot);
	ajStrApp(path, program);
	ajStrAppC(path, ".txt");
    }

    ajStrDel(&docroot);

    /* does the file exist and is it readable? */
    if(ajFileStat(path, AJ_FILE_R))
	return ajTrue;

    return ajFalse;
}


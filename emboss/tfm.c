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

/* return the path to the program doc directory */
static void FindAppDocRoot (AjPStr* docroot) {

  AjPStr docrootinst = NULL;

  docrootinst = ajStrNew();
  
/* look at EMBOSS doc files */

/* try to open the installed doc directory */
  (void) ajNamRootInstall (&docrootinst);
  (void) ajFileDirFix (&docrootinst);
  ajFmtPrintS (docroot, "%Sshare/EMBOSS/doc/programs/",
  	docrootinst);
  if (!ajFileDir(docroot)) {
/* if that didn't work then try the doc directory from the distribution tarball */
    ajNamRootBase(docroot);
    (void) ajFileDirFix (docroot);
/*ajUser("docroot=%S", *docroot);*/
    if (ajFileDir(docroot)) {
      (void) ajStrAppC (docroot, "doc/programs/");
    } else {
/*    ajDebug ("EMBOSS root directory '%S' not opened\n", *docroot); */
    }
  }

  ajStrDel(&docrootinst);
  return;
}

/* return the path to the program documentation html or text file */
static AjBool FindAppDoc (AjPStr program, AjBool html, AjPStr* path) {

  AjPStr docroot = NULL;

  docroot = ajStrNew();
  FindAppDocRoot(&docroot);

  if (html) {
    (void) ajStrAppC (&docroot, "html/");
    (void) ajStrAss (path, docroot);  	
    (void) ajStrApp (path, program);
    (void) ajStrAppC (path, ".html");
  } else {
    (void) ajStrAppC (&docroot, "text/");
    (void) ajStrAss (path, docroot);  	
    (void) ajStrApp (path, program);
    (void) ajStrAppC (path, ".txt");
  }

  ajStrDel(&docroot);

/* is the file existant and readable? */
  if (ajFileStat(path, AJ_FILE_R)) {
    return ajTrue;
  }

  return ajFalse;
}


int main (int argc, char * argv[]) {

  AjPFile outfile = NULL;
  AjPStr program = NULL;
  AjBool html;
  AjBool more;
  AjPStr path = NULL;	/* path of file to be displayed */
  AjPStr cmd = NULL;	/* command line for running 'more' */
  AjPFile infile = NULL;
  AjPStr line = NULL;	/* buffer for infile lines */

  (void) embInit ("tfm", argc, argv);

  program = ajAcdGetString ("program");
  outfile = ajAcdGetOutfile ("outfile");
  html = ajAcdGetBool("html");
  more = ajAcdGetBool("more");

/* is a search string specified */
  if (!ajStrLen(program)) {
    ajDie ("No program name specified.");
  }

  if (!FindAppDoc(program, html, &path)) {
    ajDie ("The documentation for program '%S' was not found.", program);
  }

/* are we outputting to STDOUT and piping through 'more'? */
  if (ajFileStdout(outfile) && more) {
    ajStrAssC(&cmd, "more ");
    ajStrApp(&cmd, path);
    ajSystem(&cmd);
  } else {
/* output file as-is */  
    infile = ajFileNewIn(path);
    while(ajFileGets(infile, &line)) {
      ajFmtPrintF(outfile, "%S", line);
    }
    (void) ajFileClose(&infile);
  }

/* close file */
  (void) ajFileClose(&outfile);

  (void) ajExit();
  return 0;
}


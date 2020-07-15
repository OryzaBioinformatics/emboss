/* @source tfm application
**
** Displays a program's help documentation manual
**
** @author Copyright (C) Gary Williams (gwilliam@hgmp.mrc.ac.uk)
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


static void tfm_FindAppDocRoot(const AjPStr program, AjPStr* docroot,
			       AjBool html);
static AjBool tfm_FindAppDoc(const AjPStr program, const AjPStr docroot,
			     AjBool html, AjPStr* path);
static void tfm_FixImages(AjPStr *line, const AjPStr path);




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
    AjPFile infile  = NULL;
    AjPStr  line    = NULL;		/* buffer for infile lines */
    AjPStr  pager   = NULL;
    AjPStr  docroot = NULL;
    
    char *shellpager = NULL;

    embInit("tfm", argc, argv);
    
    program = ajAcdGetString("program");
    outfile = ajAcdGetOutfile("outfile");
    html    = ajAcdGetBool("html");
    more    = ajAcdGetBool("more");

    cmd     = ajStrNew();
    path    = ajStrNew();
    pager   = ajStrNew();
    line    = ajStrNew();
    docroot = ajStrNew();
    

    tfm_FindAppDocRoot(program, &docroot, html);
    
    /* is a search string specified - should be tested in tfm.acd file */
    if(!ajStrGetLen(program))
	ajFatal("No program name specified.");
    
    if(!tfm_FindAppDoc(program, docroot, html, &path))
	ajDie("No documentation found in %S for program '%S'.",
	      docroot,program);
    
    /* outputing to STDOUT and piping through 'more'? */
    if(ajFileStdout(outfile) && more)
    {
	if(!ajNamGetValueC("PAGER",&pager))
	{
	    shellpager = getenv("PAGER");
	    if(shellpager)
		ajStrAssignC(&pager,shellpager);
	    if(!ajStrGetLen(pager))
		ajStrAssignC(&pager,"more");
	}
	ajFmtPrintS(&cmd,"%S %S",pager,path);
	system(ajStrGetPtr(cmd));
    }
    else
    {
	/* output file as-is */
	infile = ajFileNewIn(path);

	while(ajFileGets(infile, &line))
	{
	    if(html)
		tfm_FixImages(&line,docroot);
	    ajFmtPrintF(outfile, "%S", line);
	}

	ajFileClose(&infile);
    }
    
    ajFileClose(&outfile);


    ajStrDel(&path);
    ajStrDel(&pager);
    ajStrDel(&line);
    ajStrDel(&cmd);
    ajStrDel(&program);
    ajStrDel(&docroot);

    embExit();

    return 0;
}




/* @funcstatic tfm_FindAppDocRoot *********************************************
**
** return the path to the program doc directory
**
** @param [r] program [const AjPStr] program name
** @param [w] docroot [AjPStr*] root dir for documentation files
** @param [r] html [AjBool] whether html required
** @@
******************************************************************************/

static void tfm_FindAppDocRoot(const AjPStr program,
			       AjPStr* docroot, AjBool html)
{

    AjPStr docrootinst = NULL;
    AjPStr roottmp = NULL;
    AjPStr tmpstr = NULL;
    AjPStr embassy = NULL;

    AjBool is_windows = ajFalse;
#ifdef WIN32
    is_windows = ajTrue;
#endif

    docrootinst = ajStrNew();
    roottmp     = ajStrNew();
    tmpstr      = ajStrNew();
    
    ajDebug("given  docroot '%S'\n", *docroot);
    ajNamGetValueC("docroot", &roottmp);
    ajDebug("defined docroot '%S'\n", roottmp);

    if(html)
      embGrpGetEmbassy(program, &embassy);

    /* look at EMBOSS doc files */

    /* try to open the installed doc directory */
    if(ajStrGetLen(roottmp))
	ajStrAssignS(docroot, roottmp);
    else
    {
	ajNamRootInstall(&docrootinst);
	ajFileDirFix(&docrootinst);

	if(is_windows)
	{
	    ajFileDirFix(&docrootinst);
	    ajFmtPrintS(&tmpstr,"%Sdoc",docrootinst);
	    if(!ajSysIsDirectory(ajStrGetPtr(tmpstr)))
	    {
		ajFileDirUp(&docrootinst);
		ajFileDirUp(&docrootinst);
	    }
	    ajStrAppendC(&docrootinst,"doc");
	}
	else
	    ajFmtPrintAppS(&docrootinst, "share%sEMBOSS%sdoc%s",
			SLASH_STRING,SLASH_STRING,SLASH_STRING);

	ajFileDirFix(&docrootinst);

	if(html)
	{
	  if(ajStrGetLen(embassy))
	    ajFmtPrintS(docroot,"%Shtml%sembassy%s%S%s",
			docrootinst,SLASH_STRING,SLASH_STRING,
			embassy, SLASH_STRING);
	  else
	    ajFmtPrintS(docroot,"%Shtml%semboss%sapps%s",
			docrootinst,SLASH_STRING,SLASH_STRING,
			SLASH_STRING);
	}
	else
	    ajFmtPrintS(docroot,"%Sprograms%stext%s",docrootinst,SLASH_STRING,
			SLASH_STRING);
    }
    ajFileDirFix(docroot);
    ajDebug("installed docroot '%S'\n", *docroot);

    if(!ajFileDir(docroot) && !is_windows)
    {
	/*
	**  if that didn't work then try the doc directory from the
	**  distribution tarball
	*/
	ajNamRootBase(docroot);
	ajFileDirFix(docroot);

	if(ajStrGetLen(embassy))
	{
	  if(ajFileDir(docroot))
	    ajFmtPrintS(docroot, "embassy/%S/emboss_doc/", embassy);
	  if(html)
	    {
	      ajStrAppendC(docroot, "html/");
	    }
	  else
	    {
	      ajStrAppendC(docroot, "text/");
	    }
	}
	else
	{
	  if(ajFileDir(docroot))
	    ajStrAppendC(docroot, "doc/programs/");
	  if(html)
	    {
	      ajStrAppendC(docroot, "html/");
	    }
	  else
	    {
	      ajStrAppendC(docroot, "text/");
	    }
	}
    }

    ajStrDel(&roottmp);
    ajStrDel(&docrootinst);
    ajStrDel(&tmpstr);
    
    return;
}




/* @funcstatic tfm_FindAppDoc *************************************************
**
** return the path to the program documentation html or text file
**
** @param [r] program [const AjPStr] program name
** @param [r] docroot [const AjPStr] documentation root
** @param [r] html [AjBool] whether html required
** @param [w] path [AjPStr*] returned path
** @return [AjBool] success
** @@
******************************************************************************/

static AjBool tfm_FindAppDoc(const AjPStr program, const AjPStr docroot,
			     AjBool html, AjPStr* path)
{
    AjPStr target = NULL;
    AjBool ret    = ajFalse;
    
    target = ajStrNew();

    ajStrAssignS(&target,docroot);

    if(html)
    {
	ajStrAssignS(path, target);
	ajStrAppendS(path, program);
	ajStrAppendC(path, ".html");
    }
    else
    {
	ajStrAssignS(path, target);
	ajStrAppendS(path, program);
	ajStrAppendC(path, ".txt");
    }

    /* does the file exist and is it readable? */
    if(ajFileStat(*path, AJ_FILE_R))
	ret = ajTrue;

    ajDebug("tfm_FindAppDoc '%S' %B\n", *path, ret);

    ajStrDel(&target);

    return ret;
}




/* @funcstatic tfm_FixImages *************************************************
**
** Add full path to installed or local image files
**
** @param [w] line [AjPStr*] html line
** @param [r] path [const AjPStr] file location
** @return [void]
** @@
******************************************************************************/

static void tfm_FixImages(AjPStr *line, const AjPStr path)
{
    AjPStr newpath = NULL;
    AjPStr name = NULL;
    AjPStr pre  = NULL;
    AjPStr post = NULL;
    
    const char *p    = NULL;
    const char *q    = NULL;

#ifdef __CYGWIN__
    char *root = NULL;
#endif

    q = ajStrGetPtr(*line);

    if(!(p = strstr(q,"<img")))
	return;

    if(!(p=strstr(p,"src=")))
	return;

    newpath = ajStrNewC("");
    name    = ajStrNew();
    pre     = ajStrNew();
    post    = ajStrNew();
    
#ifdef __CYGWIN__
    if((root=getenv("EMBOSSCYGROOT")))
	ajFmtPrintS(&newpath,"%s",root);
#endif

    ajStrAppendS(&newpath,path);

    ajStrAssignSubC(&pre,q,0,(ajint)(p-q)+4);
    p += 5;
    while(*p && *p!='"')
    {
	ajStrAppendK(&name,*p);
	++p;
    }
    ajStrAssignC(&post,p);

    ajFmtPrintS(line,"%Sfile:/%S%S%S",pre,newpath,name,post);

    ajStrDel(&newpath);
    ajStrDel(&name);
    ajStrDel(&post);
    ajStrDel(&pre);
    
    return;
}

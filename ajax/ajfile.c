/******************************************************************************
** @source AJAX file routines
**
** @version 1.0
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
******************************************************************************/

#include "ajax.h"
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>

#define FILERECURSLV 20
static ajint fileBuffSize = 2048;

static ajint fileHandle = 0;
static ajint fileOpenCnt = 0;
static ajint fileOpenMax = 0;
static ajint fileCloseCnt = 0;
static ajint fileOpenTot = 0;

static AjPRegexp fileUserExp = NULL;
static AjPRegexp fileWildExp = NULL;
static AjPRegexp fileEntryExp = NULL;
static AjPRegexp fileFileExp = NULL;
static AjPRegexp fileRestExp = NULL;
static AjPRegexp fileDirExp = NULL;
static AjPRegexp fileFilenameExp = NULL;

static void   fileBuffInit(AjPFileBuff thys);
static void   fileBuffLineAdd(AjPFileBuff thys, AjPStr line);
static void   fileBuffLineDel(AjPFileBuff thys);
static AjBool fileBuffLineNext(AjPFileBuff thys);
static void   fileClose(AjPFile thys);
static void   fileListRecurs(const AjPStr file, AjPList list, ajint *recurs);
static DIR*   fileOpenDir(AjPStr *dir);




/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section File Constructors *************************************************
**
** All constructors return a new open file by pointer. It is the responsibility
** of the user to first destroy any previous file pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing file, see instead
** the {File Assignments} and {File Modifiers} functions.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read various kinds of data.
**
******************************************************************************/




/* @func ajFileNew ************************************************************
**
** Creates a new file object.
**
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNew(void)
{
    AjPFile thys;

    AJNEW0(thys);
    thys->fp = NULL;
    thys->Handle = 0;
    thys->Name = ajStrNew();
    thys->Buff = ajStrNewL(fileBuffSize);
    thys->List = NULL;
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;

    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewInPipe ******************************************************
**
** Creates a new file object to read the output from a command.
**
** @param [r] name [const AjPStr] Command string.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInPipe(const AjPStr name)
{
    AjPFile thys;
    
    int pid;
    ajint pipefds[2];		     /* file descriptors for a pipe */
    static AjPStr tmpname = NULL;
    char** arglist        = NULL;
    char* pgm;
    int status;
    
    AJNEW0(thys);
    ajStrAssS(&tmpname, name);

    /* pipe character at end */
    if(ajStrChar(tmpname, -1) == '|')
	ajStrTrim(&tmpname, -1);
    if(pipe(pipefds) < 0)
	ajFatal("pipe create failed");

    /* negative return indicates failure */
    pid = fork();
    if(pid < 0)
	ajFatal("fork create failed");

    /* pid is zero in the child, but is the child PID in the parent */
    
    if(!pid)
    {
	/* this is the child process */
	close(pipefds[0]);

	dup2(pipefds[1], 1);
	close(pipefds[1]);
	ajSysArglist(tmpname, &pgm, &arglist);
	ajDebug("execvp ('%S', NULL)\n", tmpname);
	execvp(pgm, arglist);
	ajFatal("execvp ('%S', NULL) failed: '%s'\n",
		tmpname, strerror(errno));
    }
    
    ajDebug("pid %d, pipe '%d', '%d'\n",
	    pid, pipefds[0], pipefds[1]);

    /* fp is what we read from the pipe */
    thys->fp = ajSysFdopen(pipefds[0], "r");
    close(pipefds[1]);
    ajStrDelReuse(&tmpname);
    
    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    ajStrAssS(&thys->Name, name);
    thys->End = ajFalse;
    
    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    
    while(wait(&status) != pid);
    
    return thys;
}




/* @func ajFileNewIn **********************************************************
**
** Creates a new file object to read a named file.
**
** If the filename ends with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewIn(const AjPStr name)
{
    AjPFile thys          = NULL;
    static AjPStr userstr = NULL;
    static AjPStr reststr = NULL;
    static AjPStr tmpname = NULL;
    struct passwd* pass   = NULL;
    AjPStr dirname        = NULL;
    AjPStr wildname       = NULL;
    AjPFile ptr;
    
    char   *p = NULL;
    
    ajDebug("ajFileNewIn '%S'\n", name);
    ajStrAssS(&tmpname, name);
    
    if(ajStrMatchC(tmpname, "stdin"))
    {
	thys = ajFileNewF(stdin);
	ajStrAssC(&thys->Name, "stdin");
	return thys;
    }
    
    if(ajStrChar(tmpname, -1) == '|')	/* pipe character at end */
	return ajFileNewInPipe(tmpname);

    if(ajStrChar(tmpname, 0) == '~')
    {
	ajDebug("starts with '~'\n");
	if(!fileUserExp) fileUserExp = ajRegCompC("^~([^/]*)");
	ajRegExec(fileUserExp, tmpname);
	ajRegSubI(fileUserExp, 1, &userstr);
	ajRegPost(fileUserExp, &reststr);
	ajDebug("  user: '%S' rest: '%S'\n", userstr, reststr);

	if(ajStrLen(userstr))
	{
	    /* username specified */
	    pass = getpwnam(ajStrStr(userstr));
	    if(!pass) return NULL;
	    ajFmtPrintS(&tmpname, "%s%S", pass->pw_dir, reststr);
	    ajDebug("use getpwnam: '%S'\n", tmpname);
	}
	else
	{
	    /* just ~/ */
	    if((p = getenv("HOME")))
		ajFmtPrintS(&tmpname, "%s%S", p, reststr);
	    else
		ajFmtPrintS(&tmpname,"%S",reststr);
	    ajDebug("use HOME: '%S'\n", tmpname);
	}
    }
    
    if(!fileWildExp)
	fileWildExp = ajRegCompC("(.*/)?([^/]*[*?][^/]*)$");
    
    if(ajRegExec(fileWildExp, tmpname))
    {
	/* wildcard file names */
	ajRegSubI(fileWildExp, 1, &dirname);
	ajRegSubI(fileWildExp, 2, &wildname);
	ajDebug("wild dir '%S' files '%S'\n", dirname, wildname);
	ptr = ajFileNewDW(dirname, wildname);
	ajStrDel(&dirname);
	ajStrDel(&wildname);
	return ptr;
    }
    
    
    
    AJNEW0(thys);
    ajStrAssS(&thys->Name, tmpname);
    ajNamResolve(&thys->Name);
    thys->fp = fopen(ajStrStr(thys->Name), "r");
    if(!thys->fp)
    {
	ajStrDel(&thys->Name);

	AJFREE(thys);
	/*    thys->Handle = 0;*/
	return NULL;
    }
    thys->Handle = ++fileHandle;
    thys->List = NULL;
    thys->End = ajFalse;
    
    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;
    
    return thys;
}




/* @func ajFileNewInC *********************************************************
**
** Creates a new file object to read a named file.
**
** If the filename begins with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const char*] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInC(const char *name)
{
    AjPStr tmp;
    AjPFile fp;

    tmp = ajStrNewC(name);
    fp = ajFileNewIn(tmp);
    ajStrDel(&tmp);

    return fp;
}




/* @func ajFileNewInList ******************************************************
**
** Creates a new file object with a list of input files.
**
** @param [r] list [AjPList] List of input filenames as strings.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInList(AjPList list)
{
    AjPFile thys;

    AJNEW0(thys);

    thys->List = list;
    thys->Name = NULL;
    ajListstrTrace(thys->List);
    ajListstrPop(thys->List, &thys->Name);
    ajDebug("ajFileNewInList pop '%S'\n", thys->Name);
    ajListstrTrace(thys->List);
    ajNamResolve(&thys->Name);
    thys->fp = fopen(ajStrStr(thys->Name), "r");
    if(!thys->fp)
    {
	ajDebug("ajFileNewInList fopen failed\n");
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewApp *********************************************************
**
** Creates an output file object with a specified name.
** The file is opened for append so it either appends to an existing file
** or opens a new one.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewApp(const AjPStr name)
{
    AjPFile thys;

    AJNEW0(thys);
    thys->fp = fopen(ajStrStr(name), "a");
    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    ajStrAssS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewOut *********************************************************
**
** Creates a new output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOut(const AjPStr name)
{
    AjPFile thys;

    if(ajStrMatchC(name, "stdout"))
    {
	thys = ajFileNewF(stdout);
	ajStrAssC(&thys->Name, "stdout");
	return thys;
    }

    if(ajStrMatchC(name, "stderr"))
    {
	thys = ajFileNewF(stderr);
	ajStrAssC(&thys->Name, "stderr");
	return thys;
    }

    AJNEW0(thys);
    thys->fp = fopen(ajStrStr(name), "w");
    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }
    thys->Handle = ++fileHandle;
    ajStrAssS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileNewOutC ********************************************************
**
** Creates a new output file object with a specified name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [r] name [const char*] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutC(const char* name)
{
    AjPStr tmp;
    AjPFile thys;

    tmp = ajStrNewC(name);
    thys = ajFileNewIn(tmp);
    ajStrDel(&tmp);

    return thys;
}




/* @func ajFileNewOutD ********************************************************
**
** Creates a new output file object with a specified directory and name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** If the filename already has a directory specified,
** the "dir" argument is ignored.
**
** @param [rN] dir [const AjPStr] Directory (optional, can be empty or NULL).
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutD(const AjPStr dir, const AjPStr name)
{
    AjPFile thys;
    static AjPStr dirfix = NULL;

    ajDebug("ajFileNewOutD('%S' '%S')\n", dir, name);

    if(ajStrMatchC(name, "stdout"))
	return ajFileNewF(stdout);

    if(ajStrMatchC(name, "stderr"))
	return ajFileNewF(stderr);

    AJNEW0(thys);

    if(!ajStrLen(dir))
    {
	thys->fp = fopen(ajStrStr(name), "w");
	ajDebug("ajFileNewOutD open name '%S'\n", name);
    }
    else
    {
	if(ajFileHasDir(name))
	    ajStrAssS(&dirfix, name);
	else
	{
	    ajStrAssS(&dirfix, dir);
	    if(ajStrChar(dir, -1) != '/')
		ajStrAppC(&dirfix, "/");
	    ajStrApp(&dirfix, name);
	}
	thys->fp = fopen(ajStrStr(dirfix), "w");
	ajDebug("ajFileNewOutD open dirfix '%S'\n", dirfix);
    }

    if(!thys->fp)
    {
	thys->Handle = 0;
	return NULL;
    }

    thys->Handle = ++fileHandle;
    ajStrAssS(&thys->Name, name);
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* @func ajFileSetDir ********************************************************
**
** Adds a default directory to a filename.
** If the filename already has a directory, then this is left unchanged.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively which need no directory.
**
** @param [r] pname [AjPStr*] File name.
** @param [rN] dir [const AjPStr] Directory (optional, can be empty or NULL).
** @return [AjBool] ajTrue if the filename was changed
** @@
******************************************************************************/

AjBool ajFileSetDir(AjPStr *pname, const AjPStr dir)
{
    static AjPStr dirfix = NULL;
    AjBool ret;


    ajDebug("ajFileSetDir name '%S' dir '%S'\n", *pname, dir);

    ret = ajFalse;

    if(ajStrMatchC(*pname, "stdout"))
	return ret;

    if(ajStrMatchC(*pname, "stderr"))
	return ret;

    if(!ajStrLen(dir))
	return ret;

    if(ajFileHasDir(*pname))
	return ret;

    ajStrAssS(&dirfix, dir);
    if(ajStrChar(dir, -1) != '/')
	ajStrAppC(&dirfix, "/");
    ajStrApp(&dirfix, *pname);
  
    ajStrAssS(pname, dirfix);
    ret = ajTrue;

    ajDebug("ajFileSetDir changed name '%S'\n", *pname);

    return ret;
}




/* @func ajFileHasDir ********************************************************
**
** Tests whether a filename includes a directory specification.
**
** @param [r] name [const AjPStr] File name.
** @return [AjBool] ajTrue if directory filename syntax was found
** @@
******************************************************************************/

AjBool ajFileHasDir(const AjPStr name)
{
    if(ajStrFindC(name, "/") < 0)
	return ajFalse;

    return ajTrue;
}




/* @func ajFileNewF ***********************************************************
**
** Creates a new file object from an open C file.
**
** @param [r] file [FILE*] C file.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewF(FILE* file)
{
    AjPFile thys;

    if(!file)
	ajFatal("Trying to create an AJAX file from a bad C RTL FILE*");

    AJNEW0(thys);
    thys->fp     = file;
    thys->Handle = ++fileHandle;
    thys->Name   = ajStrNew();
    thys->End    = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if(fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section File Destructors **************************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/




/* @func ajFileClose **********************************************************
**
** Close and free a file object.
**
** @param [w] pthis [AjPFile*] File.
** @return [void]
** @@
******************************************************************************/

void ajFileClose(AjPFile* pthis)
{
    AjPFile thys;

    thys = pthis ? *pthis : 0;

    if(!pthis)
	return;
    if(!*pthis)
	return;

    fileClose(thys);
    AJFREE(*pthis);

    return;
}




/* @func ajFileOutClose *******************************************************
**
** Closes and deletes an output file object.
**
** @param [r] pthis [AjPFile*] Output file.
** @return [void]
** @@
******************************************************************************/

void ajFileOutClose(AjPFile* pthis)
{
    AjPFile thys;

    thys = pthis ? *pthis : 0;

    ajFmtPrintF(thys, "Standard output close ...\n");
    ajFileClose(pthis);

    return;
}




/* @funcstatic fileClose ******************************************************
**
** Closes a file object. Used as part of the public destructor and
** other public functions.
**
** @param [w] thys [AjPFile] File.
** @return [void]
** @@
******************************************************************************/

static void fileClose(AjPFile thys)
{
    if(!thys)
	return;

    if(thys->Handle)
    {
	ajDebug("closing file '%F'\n", thys);
	if(thys->fp != stdout && thys->fp != stderr)
	{
	    if(fclose(thys->fp))
		ajFatal("File close problem in fileClose");
	}
	thys->Handle = 0;

	fileCloseCnt++;
	fileOpenCnt--;
    }
    else
	ajDebug("file already closed\n");

    ajStrDel(&thys->Name);
    ajStrDel(&thys->Buff);
    ajListstrFree(&thys->List);

    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section File Assignments **************************************************
**
** These functions overwrite the file provided as the first argument
**
******************************************************************************/




/* @func ajFileDataNew ********************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** a) in .   b) in ./.embossdata c) ~/ d) ~/.embossdata e) $DATA
**
** @param [r] tfile [const AjPStr] Filename in AjStr.
** @param [w] fnew [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataNew(const AjPStr tfile, AjPFile *fnew)
{
    static AjPStr bname = NULL;
    static AjPStr fname = NULL;
    AjPStr hname        = NULL;
    static AjPStr pname = NULL;
    char *p;
    
    if(tfile == NULL)
	return;
    
    ajStrAssS(&bname, tfile);
    ajDebug("ajFileDataNew trying '%S'\n", bname);
    if(ajFileStat(&bname, AJ_FILE_R))
    {
	*fnew = ajFileNewIn(bname);
	ajStrDelReuse(&bname);
	return;
    }
    
    ajStrAssC(&fname, ".embossdata/");
    ajStrApp(&fname, bname);
    ajDebug("ajFileDataNew trying '%S'\n", fname);
    if(ajFileStat(&fname, AJ_FILE_R))
    {
	*fnew = ajFileNewIn(fname);
	ajStrDelReuse(&bname);
	ajStrDelReuse(&fname);
	return;
    }
    
    
    if((p=getenv("HOME")))
    {
	hname = ajStrNew();
	ajStrAssC(&hname,p);
	ajStrAppC(&hname,"/");
	ajStrApp(&hname,bname);
	ajDebug("ajFileDataNew trying '%S'\n", hname);
	if(ajFileStat(&hname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(hname);
	    ajStrDel(&hname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}

	ajStrAssC(&hname,p);
	ajStrAppC(&hname,"/.embossdata/");
	ajStrApp(&hname,bname);
	ajDebug("ajFileDataNew trying '%S'\n", hname);
	if(ajFileStat(&hname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(hname);
	    ajStrDel(&hname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
	ajStrDel(&hname);
    }
    
    if(ajNamGetValueC("DATA", &fname))
    {
        ajFileDirFix(&fname);
	ajStrApp(&fname,bname);
	ajDebug("ajFileDataNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    /* just EMBOSS/data under installation */
    if(ajNamRootInstall(&fname))
    {
        ajNamRootPack(&pname);		/* just EMBOSS */
	ajFileDirFix(&fname);
	ajStrAppC(&fname,"share/");
	ajStrApp(&fname,pname);
	ajStrAppC(&fname,"/data/");
	ajStrApp(&fname,bname);
	ajDebug("ajFileDataNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    /* just emboss/data under source */
    if(ajNamRoot(&fname))
    {
	ajStrAppC(&fname,"/data/");
	ajStrApp(&fname,bname);
	ajDebug("ajFileDataNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    ajStrDelReuse(&bname);
    ajStrDelReuse(&fname);
    
    ajDebug("ajFileDataNew failed to find '%S'\n", fname);
    *fnew = NULL;
    
    return;
}




/* @func ajFileDataNewC *******************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** a) in .   b) in ./.embossdata c) ~/ d) ~/.embossdata e) $DATA
**
** @param [r] s [const char*] Filename.
** @param [w] f [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataNewC(const char *s, AjPFile *f)
{
    AjPStr t;

    t = ajStrNewC(s);
    ajFileDataNew(t,f);
    ajStrDel(&t);

    return;
}




/* @func ajFileDataDirNew *****************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** in the EMBOSS/data/(dir) directory, or is found in the usual directories
** by ajFileDataNew
**
** @param [r] tfile [const AjPStr] Filename in AjStr.
** @param [r] dir [const AjPStr] Data directory name in AjStr.
** @param [w] fnew [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataDirNew(const AjPStr tfile, const AjPStr dir, AjPFile *fnew)
{
    static AjPStr fname = NULL;	      /* file name to try opening   */
    static AjPStr pname = NULL;	      /* package name (e.g. EMBOSS) */
    
    if(ajNamGetValueC("DATA", &fname))
    {
        ajFileDirFix(&fname);
	if(ajStrLen(dir))
	{
	    ajStrApp(&fname,dir);
	    ajFileDirFix(&fname);
	}
	ajStrApp(&fname,tfile);
	ajDebug("ajFileDataDirNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    /* just EMBOSS/data under installation */
    if(ajNamRootInstall(&fname))
    {
	/* just EMBOSS */
        ajNamRootPack(&pname);
	ajFileDirFix(&fname);
	ajStrAppC(&fname,"share/");
	ajStrApp(&fname,pname);
	ajStrAppC(&fname,"/data/");
	if(ajStrLen(dir))
	{
	    ajStrApp(&fname,dir);
	    ajFileDirFix(&fname);
	}
	ajStrApp(&fname,tfile);
	ajDebug("ajFileDataDirNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&pname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }

    /* just emboss/data under source */
    if(ajNamRoot(&fname))
    {
	ajStrAppC(&fname,"/data/");
	if(ajStrLen(dir))
	{
	    ajStrApp(&fname,dir);
	    ajFileDirFix(&fname);
	}
	ajStrApp(&fname,tfile);
	ajDebug("ajFileDataDirNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&pname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    ajStrDelReuse(&pname);
    ajStrDelReuse(&fname);
    
    *fnew = NULL;
    
    ajFileDataNew(tfile, fnew);
    
    return;
}




/* @func ajFileDataDirNewC ****************************************************
**
** Returns an allocated AjFileInNew pointer (AjPFile) if file exists
** in the EMBOSS/data/(dir) directory, or is found in the usual directories
** by ajFileDataNew
**
** @param [r] s [const char*] Filename
** @param [r] d [const char*] Data directory name.
** @param [w] f [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataDirNewC(const char *s, const char* d, AjPFile *f)
{
    AjPStr t;
    AjPStr u;

    t = ajStrNewC(s);
    u = ajStrNewC(d);
    ajFileDataDirNew(t,u,f);
    ajStrDel(&t);
    ajStrDel(&u);

    return;
}




/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section File Modifiers ****************************************************
**
** These functions use the contents of a file object and update them.
**
******************************************************************************/




/* @func ajFileSeek ***********************************************************
**
** Sets the current position in an open file.
**
** Resets the end-of-file flag End for cases where end-of-file was
** reached and then we seek back somewhere in the file.
**
** @param [r] thys [AjPFile] File.
** @param [r] offset [ajlong] Offset
** @param [r] wherefrom [ajint] Start of offset, as defined for 'fseek'.
** @return [ajint] Result of 'fseek'
** @@
******************************************************************************/

ajint ajFileSeek(AjPFile thys, ajlong offset, ajint wherefrom)
{
    ajint ret;

    clearerr(thys->fp);
    ret = fseek(thys->fp, offset, wherefrom);

    if(feof(thys->fp))
    {
	thys->End = ajTrue;
	ajDebug("EOF ajFileSeek file %F\n", thys);
    }
    else
	thys->End = ajFalse;

    return ret;
}




/* @func ajFileRead ***********************************************************
**
** Binary read from an input file object using the C 'fread' function.
**
** @param [w] ptr [void*] Buffer for output.
** @param [r] element_size [size_t] Number of bytes per element.
** @param [r] count [size_t] Number of elements to read.
** @param [r] thys [const AjPFile] Input file.
** @return [size_t] Return value from 'fread'
** @@
******************************************************************************/

size_t ajFileRead(void* ptr, size_t element_size, size_t count,
		  const AjPFile thys)
{
    return fread(ptr, element_size, count, thys->fp);
}




/* @func ajFileReadUint *******************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from a specified endianism.
**
** @param [r] thys [const AjPFile] Input file.
** @param [r] Bigendian [AjBool] Big endian or not.
** @return [ajuint] Converted integer value
** @@
******************************************************************************/

ajuint ajFileReadUint(const AjPFile thys, AjBool Bigendian)
{
    static ajint called  = 0;
    static AjBool bigend = AJFALSE;
    ajuint ret;
    ajint ret2;

    if(!called)
	bigend = ajUtilBigendian();

    fread(&ret, 4, 1, thys->fp);
    if(Bigendian && bigend)
	return ret;
    else if(!Bigendian && !bigend)
	return ret;

    /*ajDebug("Reversed: %u", ret);*/
    ret2 = (ajint) ret;
    ajUtilRev4(&ret2);
    ret = (ajuint) ret2;
    /*ajDebug(" => %u\n", ret);*/

    return ret;
}




/* @func ajFileWrite **********************************************************
**
** Binary write to an output file object using the C 'fwrite' function.
**
** @param [r] thys [const AjPFile] Output file.
** @param [w] ptr [const void*] Buffer for output.
** @param [r] element_size [size_t] Number of bytes per element.
** @param [r] count [size_t] Number of elements to write.
** @return [size_t] Return value from 'fwrite'
** @@
******************************************************************************/

size_t ajFileWrite(const AjPFile thys, const void* ptr,
		   size_t element_size, size_t count)
{
    return fwrite(ptr, element_size, count, thys->fp);
}




/* @func ajFileNext ***********************************************************
**
** Given a file object that includes a list of input files, closes the
** current input file and opens the next one.
**
** @param [r] thys [AjPFile] File object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileNext(AjPFile thys)
{
    static AjPStr name = NULL;

    if(!thys->List)
    {
	ajDebug("ajFileNext for non-list file %F\n", thys);
	return ajFalse;
    }

    ajDebug("ajFileNext for non-list file %F name '%S'\n", thys, thys->Name);
    ajListTrace(thys->List);
    if(!ajListPop(thys->List, (void*) &name))
    {
	/* end of list */
	ajDebug("ajFileNext failed - list completed\n");
	return ajFalse;
    }

    ajDebug("ajFileNext filename '%S'\n", name);
    if(!ajFileReopen(thys, name))
    {
	/* popped from the list */
	ajStrDel(&name);
	return ajFalse;
    }

    /* popped from the list */
    ajStrDel(&name);
    thys->End = ajFalse;

    ajDebug("ajFileNext success\n");

    return ajTrue;
}




/* @func ajFileReopen *********************************************************
**
** Reopens a file with a new name.
**
** @param [r] thys [AjPFile] Input file.
** @param [r] name [AjPStr] name of file.
** @return [FILE*] copy of file pointer
** @@
******************************************************************************/

FILE* ajFileReopen(AjPFile thys, AjPStr name)
{
    ajStrAssS(&thys->Name, name);
    return freopen(ajStrStr(thys->Name), "r", thys->fp);
}




/* @func ajFileReadLine *******************************************************
**
** Reads a line from the input file, removing any trailing newline.
**
** @param [r] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReadLine(AjPFile thys, AjPStr* pdest)
{
    return ajFileGetsTrim(thys, pdest);
}




/* @func ajFileGetsTrimL ******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [r] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @param [w] fpos [ajlong*] File position before the read.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetsTrimL(AjPFile thys, AjPStr* pdest, ajlong* fpos)
{
    AjBool ok;
    AjPStr dest;

    dest = *pdest;

    ok = ajFileGetsL(thys, pdest, fpos);

    if(!ok)
	return ajFalse;

    /* trim any trailing newline */

    dest = *pdest;
    if(dest->Ptr[dest->Len-1] == '\n')
	dest->Ptr[--dest->Len] = '\0';

    if(dest->Len && dest->Ptr[dest->Len-1] == '\r')
    {
	ajDebug("Remove carriage-return characters from PC-style files\n");
	dest->Ptr[--dest->Len] = '\0';
    }

    return ajTrue;
}




/* @func ajFileGetsTrim *******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [r] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetsTrim(AjPFile thys, AjPStr* pdest)
{
    AjBool ok;
    AjPStr dest;

    dest = *pdest;

    ok = ajFileGets(thys, pdest);

    if(!ok)
	return ajFalse;

    /* trim any trailing newline */

    dest = *pdest;
    if(dest->Ptr[dest->Len-1] == '\n')
	dest->Ptr[--dest->Len] = '\0';

    if(dest->Len && dest->Ptr[dest->Len-1] == '\r')
    {
	ajDebug("Remove carriage-return characters from PC-style files\n");
	dest->Ptr[--dest->Len] = '\0';
    }

    return ajTrue;
}




/* @func ajFileGets ***********************************************************
**
** Reads a line from a file and returns the initial file position.
**
** @param [r] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGets(AjPFile thys, AjPStr* pdest)
{
    ajlong fpos = 0;

    return ajFileGetsL(thys, pdest, &fpos);
}




/* @func ajFileGetsL **********************************************************
**
** Reads a line from a file.
**
** @param [r] thys [AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @param [w] fpos [ajlong*] File position before the read.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetsL(AjPFile thys, AjPStr* pdest, ajlong* fpos)
{
    char *cp;
    char *buff;
    ajint isize;
    ajint ilen;
    ajint jlen;
    ajint ipos;
    
    ajStrModL(&thys->Buff, fileBuffSize);
    buff  = ajStrStr(thys->Buff);
    isize = ajStrSize(thys->Buff);
    ilen  = 0;
    ipos  = 0;
    
    if(!thys->fp)
	ajWarn("ajFileGets file not found");
    
    while(buff)
    {
	if(thys->End)
	{
	    ajDebug("at EOF: File already read to end %F\n", thys);
	    return ajFalse;
	}
	
	*fpos = ajFileTell(thys);

#ifndef __ppc__
	cp = fgets(&buff[ipos], isize, thys->fp);
#else
	cp = ajSysFgets(&buff[ipos], isize, thys->fp);
#endif

	if(!cp)
	{
	    if(feof(thys->fp))
	    {
		thys->End = ajTrue;
		ajStrAssC(pdest, "");
		ajDebug("EOF ajFileGetsL file %F\n", thys);
		return ajFalse;
	    }
	    else
		ajFatal("Error reading from file '%s'\n", ajFileName(thys));
	}

	jlen = strlen(&buff[ipos]);
	ilen += jlen;
	if(jlen == (isize-1))
	{
	    ajDebug("more to do: jlen: %d ipos: %d isize: %d ilen: %d "
		    "Size: %d\n",
		    jlen, ipos, isize, ilen, ajStrSize(thys->Buff));
	    ajStrFixI(thys->Buff, ilen);
	    ajStrModL(&thys->Buff, ajStrSize(thys->Buff)+fileBuffSize);
	    ipos += jlen;
	    buff = ajStrStr(thys->Buff);
	    isize = ajStrSize(thys->Buff) - ipos;
	    ajDebug("expand to: ipos: %d isize: %d Size: %d\n",
		    ipos, isize, ajStrSize(thys->Buff));
	}
	else
	    buff = NULL;
    }
    
    ajStrFixI(thys->Buff, ilen);
    ajStrAssS(pdest, thys->Buff);
    
    return ajTrue;
}




/* @func ajFileUnbuffer *******************************************************
**
** Turns off system buffering of an output file, for example to allow
** debug output to appear even in the event of a program abort.
**
** @param [r] thys [const AjPFile] File object.
** @return [void]
** @@
******************************************************************************/

void ajFileUnbuffer(const AjPFile thys)
{
    setbuf(thys->fp, NULL);

    return;
}




/* @func ajFileReadAppend *****************************************************
**
** Reads a record from a file and appends it to the user supplied buffer.
**
** @param [r] thys [AjPFile] Input file.
** @param [w] pbuff [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReadAppend(AjPFile thys, AjPStr* pbuff)
{
    static AjPStr locbuff = 0;
    AjBool ok;

    if(!locbuff)
	locbuff = ajStrNew();

    ok = ajFileGets(thys, &locbuff);

    if(ok)
	ajStrApp(pbuff, locbuff);

    return ok;
}




/* @func ajFileOutHeader ******************************************************
**
** Writes a header record to the output file.
**
** @param [r] thys [const AjPFile] Output file.
** @return [void]
** @@
******************************************************************************/

void ajFileOutHeader(const AjPFile thys)
{
    ajFmtPrintF(thys, "Standard output header ...\n");

    return;
}




/* @func ajFileNameShorten ****************************************************
**
** Truncates a filename to a basic file name.
**
** @param [uP] fname [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFileNameShorten(AjPStr* fname)
{
    static AjPStr tmpstr = NULL;


    /* entryname at end */
    if(!fileEntryExp)
	fileEntryExp = ajRegCompC(":([A-Za-z0-9_-]+)$");

    if(ajRegExec(fileEntryExp, *fname))
    {
	ajRegSubI(fileEntryExp, 1, &tmpstr);
	ajStrAssS(fname, tmpstr);
	return ajTrue;
    }

    /* name.ext */
    if(!fileFileExp)
	fileFileExp = ajRegCompC("([A-Za-z0-9_-]+)[.][A-Za-z0-9_-]+$");

    if(ajRegExec(fileFileExp, *fname))
    {
	ajRegSubI(fileFileExp, 1, &tmpstr);
	ajStrAssS(fname, tmpstr);
	return ajTrue;
    }

    /* last valid word */
    if(!fileRestExp)
	fileRestExp = ajRegCompC("([A-Za-z0-9_-]+)[^A-Za-z0-9_-]*$");

    if(ajRegExec(fileRestExp, *fname))
    {
	ajRegSubI(fileRestExp, 1, &tmpstr);
	ajStrAssS(fname, tmpstr);
	return ajTrue;
    }

    ajStrAssC(fname, "");

    return ajFalse;
}




/* @func ajFileNameTrim *******************************************************
**
** Truncates a filename to a basic file name.extension
**
** @param [uP] fname [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFileNameTrim(AjPStr* fname)
{
    static AjPStr tmpstr = NULL;
    char *p;

    if((p = strrchr(ajStrStr(*fname),(ajint)'/')))
    {
	ajStrAssC(&tmpstr,p+1);
	ajStrAssS(fname,tmpstr);
    }

    return ajTrue;
}




/* @func ajFileDataNewWrite ***************************************************
**
** Returns an allocated AjFileNewOut pointer (AjPFile) to a file in the
** emboss_DATA area
**
** @param [r] tfile [const AjPStr] Filename in AjStr.
** @param [w] fnew [AjPFile*] file pointer.
** @return [void]
** @@
******************************************************************************/

void ajFileDataNewWrite(const AjPStr tfile, AjPFile *fnew)
{
    static AjPStr fname = NULL;
    static AjPStr pname = NULL;
    
    if(tfile == NULL) return;
    
    fname = ajStrNew();
    
    if(ajNamGetValueC("DATA", &fname))
    {
	/* also does ajFileDirFix */
	if(!ajFileDir(&fname))
	{
	    ajNamRootPack(&pname);
	    ajFatal("%S_DATA directory not found: %S\n",
		    pname, fname);
	}
	ajStrApp(&fname,tfile);
	if(!(*fnew = ajFileNewOut(fname)))
	    ajFatal("Cannot write to file %S\n",fname);
	ajStrDel(&fname);
	return;
    }
    
    /* just emboss/data under installation */
    if(ajNamRootInstall(&fname))
    {
	/* just EMBOSS */
        ajNamRootPack(&pname);
	ajFileDirFix(&fname);
	ajStrAppC(&fname,"share/");
	ajStrApp(&fname,pname);
	ajStrAppC(&fname,"/data/");

	/* if we are installed, else see below */
	if(ajFileDir(&fname))
	{
	    ajStrApp(&fname,tfile);
	    if(!(*fnew = ajFileNewOut(fname)))
		ajFatal("Cannot write to file %s\n",ajStrStr(fname));
	    ajStrDel(&fname);
	    return;
	}
    }
    
    /* just emboss/data under source */
    if(ajNamRoot(&fname))
    {
	ajStrAppC(&fname,"/data/");
	if(!ajFileDir(&fname))
	    ajFatal("Not installed, and source data directory not found: %S\n",
		    fname);
	ajStrApp(&fname,tfile);
	if(!(*fnew = ajFileNewOut(fname)))
	    ajFatal("Cannot write to file %s\n",ajStrStr(fname));
	ajStrDel(&fname);
	return;
    }


    ajNamRootPack(&pname);
    ajFatal("No install or source data directory, and %S_DATA not defined\n",
	    pname);
    ajStrDelReuse(&fname);
    *fnew = NULL;
    
    return;
}




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section File Operators ****************************************************
**
** These functions use the contents of a file object but do not make
** any changes.
**
******************************************************************************/




/* @func ajFileDir ************************************************************
**
** Checks that a string is a valid existing directory, and appends a
** trailing '/' if it is missing.
**
** @param [u] dir [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajFileDir(AjPStr* dir)
{
    DIR* odir;

    odir = fileOpenDir(dir);	/* appends trailing slash if needed */
    if(!odir)
	return ajFalse;

    closedir(odir);

    return ajTrue;
}




/* @func ajFileDirPath ********************************************************
**
** Checks that a string is a valid directory, and makes sure it has the
** full path definition.
**
** @param [u] dir [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajFileDirPath(AjPStr* dir)
{
    DIR* odir;
    static AjPStr cwd = NULL;

    ajDebug("ajFileDirPath '%S'\n", *dir);

    /* appends trailing slash if needed */
    odir = fileOpenDir(dir);
    if(!odir)
	return ajFalse;
    free(odir);

    ajDebug("So far '%S'\n", *dir);

    /* full path already */
    if(*ajStrStr(*dir) == '/')
	return ajTrue;

    ajFileGetwd(&cwd);

    /* current directory */
    if(ajStrMatchC(*dir, "./"))
    {
	ajStrAssS(dir, cwd);
	ajDebug("Current '%S'\n", *dir);
	return ajTrue;
    }

    /*  going up */
    while(ajStrPrefixC(*dir, "../"))
    {
	ajFileDirUp(&cwd);
	ajStrSub(dir, 3, -1);
	ajDebug("Going up '%S' '%S'\n", *dir, cwd);
    }

    ajStrInsert(dir, 0, cwd);

    ajDebug("Full path '%S'\n", *dir);

    return ajTrue;
}




/* @func ajFileGetwd **********************************************************
**
** Returns the current directory
**
** @param [w] dir [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetwd(AjPStr* dir)
{
    char cwd[PATH_MAX+1];

    if(!getcwd(cwd,PATH_MAX))
    {
	ajStrDelReuse(dir);
	return ajFalse;
    }

    if(ajStrSuffixCC(cwd, "/"))
	ajStrAssC(dir, cwd);
    else
	ajFmtPrintS(dir, "%s/", cwd);

    return ajTrue;
}




/* @func ajFileDirUp **********************************************************
**
** Changes directory name to one level up
**
** @param [u] dir [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileDirUp(AjPStr* dir)
{
    AjPStr tmpdir = NULL;

    if(!fileDirExp)
	fileDirExp = ajRegCompC("^(.*/)[^/]+/?$");

    ajStrAssS(&tmpdir, *dir);

    /* no match to pattern */
    if(!ajRegExec(fileDirExp, tmpdir))
	return ajFalse;
 
    ajRegSubI(fileDirExp, 1, dir);
    ajStrDel(&tmpdir);
 
    return ajTrue;
}




/* @funcstatic fileOpenDir ****************************************************
**
** Runs 'opendir' on the specified directory. If the directory name
** has no trailing slash (on Unix) then one is added. This is why the
** directory name must be writeable.
**
** @param [u] dir [AjPStr*] Directory name.
** @return [DIR*] result of the opendir call.
** @@
******************************************************************************/

static DIR* fileOpenDir(AjPStr* dir)
{
    if(ajStrChar(*dir, -1) != '/')
	ajStrAppC(dir, "/");

    return opendir(ajStrStr(*dir));
}




/* @func ajFileDirFix *********************************************************
**
** If the directory name has no trailing slash (on Unix) then one is
** added. This is why the directory name must be writeable.
**
** @param [u] dir [AjPStr*] Directory name.
** @return [void]
** @@
******************************************************************************/

void ajFileDirFix(AjPStr* dir)
{
    if(ajStrChar(*dir, -1) != '/')
	ajStrAppC(dir, "/");

    return;
}




/* @func ajFileExit ***********************************************************
**
** Prints a summary of file usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajFileExit(void)
{
    ajDebug("File usage : %d opened, %d closed, %d max, %d total\n",
	     fileOpenCnt, fileCloseCnt, fileOpenMax, fileOpenTot);

    ajRegFree(&fileUserExp);
    ajRegFree(&fileWildExp);
    ajRegFree(&fileEntryExp);
    ajRegFree(&fileFileExp);
    ajRegFree(&fileRestExp);
    ajRegFree(&fileDirExp);
    ajRegFree(&fileFilenameExp);

    return;
}




/* @func ajFileTrace **********************************************************
**
** Writes debug messages to trace the contents of a file object.
**
** @param [r] thys [const AjPFile] File.
** @return [void]
** @@
******************************************************************************/

void ajFileTrace(const AjPFile thys)
{
    ajDebug("File: '%S'\n", thys->Name);
    ajDebug("  handle:  %d\n", thys->Handle);

    return;
}




/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section File Casts ********************************************************
**
** These functions examine the contents of a file object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/




/* @func ajFileBuffSize *******************************************************
**
** Returns the standard record buffer size for a file
**
** @return [ajint] File record buffer size
** @@
******************************************************************************/

ajint ajFileBuffSize(void)
{
    return fileBuffSize;
}




/* @func ajFileName ***********************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [const char*] Filename as a C character string.
** @@
******************************************************************************/

const char* ajFileName(const AjPFile thys)
{
    return ajStrStr(thys->Name);
}




/* @func ajFileGetName ********************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [AjPStr] Filename as a C character string.
** @@
******************************************************************************/

AjPStr ajFileGetName(const AjPFile thys)
{
    return thys->Name;
}




/* @func ajFileStat ***********************************************************
**
** Returns true if file exists and is read or write or executable by the user
** as determined by AJ_FILE_R AJ_FILE_W AJ_FILE_X file modes
**
** @param [r] fname [AjPStr*] Filename.
** @param [r] mode [ajint] file mode.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFileStat(AjPStr *fname, ajint mode)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrStr(*fname), &buf))
#else
	if(!stat(ajStrStr(*fname), &buf))
#endif
	    if((ajuint)buf.st_mode & mode)
		return ajTrue;

    return ajFalse;
}




/* @func ajFileLength *********************************************************
**
** Returns the length of a file
**
** @param [r] fname [AjPStr] Filename.
** @return [ajlong] length or -1 if file doesn't exist
** @@
******************************************************************************/

ajlong ajFileLength(AjPStr fname)
{
#if defined(AJ_IRIXLF)
    struct stat64 buf;
#else
    struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(!stat64(ajStrStr(fname), &buf))
	return (ajlong)buf.st_size;
#else
    if(!stat(ajStrStr(fname), &buf))
	return (ajlong)buf.st_size;
#endif

    return -1;
}




/* @func ajFileTell ***********************************************************
**
** Returns the current position in an open file.
**
** @param [r] thys [const AjPFile] File.
** @return [ajlong] Result of 'ftell'
** @@
******************************************************************************/

ajlong ajFileTell(const AjPFile thys)
{
    if(!thys->fp)
	return 0;
    return ftell(thys->fp);
}




/* @func ajFileStdout *********************************************************
**
** Tests whether a file object is really stdout.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdout.
** @@
******************************************************************************/

AjBool ajFileStdout(const AjPFile file)
{
    if(file->fp == stdout)
	return ajTrue;

    return ajFalse;
}




/* @func ajFileStderr *********************************************************
**
** Tests whether a file object is really stderr.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stderr.
** @@
******************************************************************************/

AjBool ajFileStderr(const AjPFile file)
{
    if(file->fp == stderr)
	return ajTrue;

    return ajFalse;
}




/* @func ajFileStdin **********************************************************
**
** Tests whether a file object is really stdin.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdin.
** @@
******************************************************************************/

AjBool ajFileStdin(const AjPFile file)
{
    if(file->fp == stdin)
	return ajTrue;

    return ajFalse;
}




/* @func ajFileFp *************************************************************
**
** Returns the C file pointer for an open file.
**
** @param [r] thys [const AjPFile] File.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFileFp(const AjPFile thys)
{
    return thys->fp;
}




/* @func ajFileEof ************************************************************
**
** Tests whether we have reached end of file already
**
** @param [u] thys [const AjPFile] File
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileEof(const AjPFile thys)
{
    return thys->End;
}




/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Buffered File Constructors ****************************************
**
** All constructors return a new open file by pointer. It is the responsibility
** of the user to first destroy any previous file pointer. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** To replace or reuse an existing file, see instead
** the {File Assignments} and {File Modifiers} functions.
**
** The range of constructors is provided to allow flexibility in how
** applications can open files to read various kinds of data.
**
******************************************************************************/




/* @func ajFileBuffNewIn ******************************************************
**
** Creates a new buffered input file object with an opened named file.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewIn(const AjPStr name)
{
    AjPFile file;

    file = ajFileNewIn(name);

    return ajFileBuffNewFile(file);
}




/* @func ajFileBuffNew ********************************************************
**
** Creates a new buffered input file object with an undefined file.
**
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNew(void)
{
    AjPFile file;

    file = ajFileNew();

    return ajFileBuffNewFile(file);
}




/* @func ajFileBuffNewFile ****************************************************
**
** Creates a new buffered input file object from an open file.
**
** @param [r] file [AjPFile] File object to be buffered.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewFile(AjPFile file)
{
    AjPFileBuff thys;

    if(!file)
	return NULL;

    AJNEW0(thys);
    thys->File = file;

    thys->Last = thys->Curr = thys->Prev = thys->Lines = thys->Free = NULL;
    thys->Pos = thys->Size = 0;

    return thys;
}




/* @func ajFileBuffSetFile ****************************************************
**
** Creates a new buffered input file object from an open file.
**
** The AjPFile pointer is a clone, so we should simply overwrite
** whatever was there before, but we do need to clear the buffer
**
** @param [w] pthys [AjPFileBuff*] Buffered file object.
** @param [r] file [AjPFile] File object to be buffered.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFileBuffSetFile(AjPFileBuff* pthys, AjPFile file)
{
    AjPFileBuff thys;

    if(!file)
    {
	ajFileBuffDel(pthys);
	return ajFalse;
    }

    if(!*pthys)
    {
	*pthys = ajFileBuffNewFile(file);
	thys = *pthys;
	return ajTrue;
    }

    thys = *pthys;

    /* same file ??? */
    if(thys->File && (thys->File->Handle ==  file->Handle))
    {
	ajFileBuffClear(thys, -1);
	return ajTrue;
    }

    /* No: this is a copy of the true pointer. */
    /* ajFileClose(&thys->File); */

    thys->File = file;

    fileBuffInit(thys);

    return ajTrue;
}




/* @funcstatic fileBuffInit ***************************************************
**
** Initialized the data for a buffered file.
**
** @param [u] thys [AjPFileBuff] Buffered file object.
** @return [void]
******************************************************************************/

static void fileBuffInit(AjPFileBuff thys)
{
    thys->Last = thys->Curr = thys->Prev = thys->Lines = NULL;
    thys->Free = thys->Freelast = NULL;
    thys->Pos = thys->Size = 0;

    return;
}




/* @func ajFileBuffNewS *******************************************************
**
** Creates a new buffered input file object with no file but with
** one line of buffered data provided.
**
** @param [r] data [const AjPStr] One line of buffered data.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewS(const AjPStr data)
{
    AjPFileBuff thys;
    AjPFile file;

    AJNEW0(file);
    file->End = ajTrue;
    ajDebug("EOF ajFileBuffNewS file <none>\n");

    thys = ajFileBuffNewFile(file);

    thys->Lines = AJNEW0(thys->Last);
    ajStrAssS(&thys->Last->Line,data);

    thys->Curr = thys->Lines;
    thys->Pos  = 0;
    thys->Size = 1;

    return thys;
}




/* @func ajFileBuffNewF *******************************************************
**
** Creates a new buffered input file from an already open C file.
**
** @param [r] fp [FILE*] Open C file.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewF(FILE* fp)
{
    AjPFile file;

    file = ajFileNewF(fp);
    if(!file)
	return NULL;

    return ajFileBuffNewFile(file);
}




/* @func ajFileBuffNewDW ******************************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Opens them as a list of files using a buffered file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard filename.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDW(const AjPStr dir, const AjPStr wildfile)
{
    DIR* dp;
    static AjPStr dirfix = NULL;
#if defined(AJ_IRIXLF)
    struct dirent64 *de;
#else
    struct dirent* de;
#endif
    ajint dirsize;
    AjPList list = NULL;
    AjPStr name  = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    if(ajStrLen(dir))
	ajStrAssS(&dirfix, dir);
    else
	ajStrAssC(&dirfix, "./");
    
    if(ajStrChar(dir, -1) != '/')
	ajStrAppC(&dirfix, "/");
    
    dp = fileOpenDir(&dirfix);
    if(!dp)
	return NULL;
    
    dirsize = 0;
    list = ajListstrNew();

#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(dp,(struct dirent64 *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir64(dp)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(dp,(struct dirent *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir(dp)))
	{
#endif
#endif
	/* skip deleted files with inode zero */
	if(!de->d_ino)
	    continue;
	if(!ajStrMatchWildCO(de->d_name, wildfile))
	    continue;
	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", dirfix, de->d_name);
	ajListstrPushApp(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);
    
    return ajFileBuffNewInList(list);
}




/* @func ajFileBuffNewDC ******************************************************
**
** Opens directory "dir"
** Looks for file "file"
** Opens them as a list of files using a buffered file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const char*] Filename.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDC(const AjPStr dir, const char* filename)
{
    static AjPStr namefix = NULL;

    if(ajStrLen(dir))
	ajStrAssS(&namefix, dir);
    else
	ajStrAssC(&namefix, "./");

    if(ajStrChar(namefix, -1) != '/')
	ajStrAppC(&namefix, "/");

    ajStrAppC(&namefix, filename);

    return ajFileBuffNewIn(namefix);
}




/* @func ajFileBuffNewDF ******************************************************
**
** Opens directory "dir"
** Looks for file "file"
** Opens them as a list of files using a buffered file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const AjPStr] Filename.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewDF(const AjPStr dir, const AjPStr filename)
{
    static AjPStr namefix = NULL;

    if(ajStrLen(dir))
	ajStrAssS(&namefix, dir);
    else
	ajStrAssC(&namefix, "./");

    if(ajStrChar(namefix, -1) != '/')
	ajStrAppC(&namefix, "/");

    ajStrApp(&namefix, filename);

    return ajFileBuffNewIn(namefix);
}




/* @func ajFileNewDW **********************************************************
**
** Opens directory "dir"
** Looks for file(s) matching "file"
** Opens them as a list of files using a simple file object.
**
** @param [r] dir [const AjPStr] Directory
** @param [r] wildfile [const AjPStr] Wildcard filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDW(const AjPStr dir, const AjPStr wildfile)
{
    DIR* dp;
    static AjPStr dirfix = NULL;
#if defined(AJ_IRIXLF)
    struct dirent64 *de;
#else
    struct dirent* de;
#endif
    ajint dirsize;
    AjPList list = NULL;
    AjPStr name  = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    if(ajStrLen(dir))
	ajStrAssS(&dirfix, dir);
    else
	ajStrAssC(&dirfix, "./");
    
    if(ajStrChar(dir, -1) != '/')
	ajStrAppC(&dirfix, "/");
    
    dp = fileOpenDir(&dirfix);
    if(!dp)
	return NULL;
    
    dirsize = 0;
    list = ajListstrNew();
    
#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(dp,(struct dirent64 *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir64(dp)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(dp,(struct dirent *)buf,&de))
    {
	if(!de)
	    break;
#else
	while((de=readdir(dp)))
	{
#endif
#endif
	/* skip deleted files with inode zero */
	if(!de->d_ino)
	    continue;
	if(!ajStrMatchWildCO(de->d_name, wildfile))
	    continue;
	dirsize++;
	ajDebug("accept '%s'\n", de->d_name);
	name = NULL;
	ajFmtPrintS(&name, "%S%s", dirfix, de->d_name);
	ajListstrPushApp(list, name);
    }
    
    closedir(dp);
    ajDebug("%d files for '%S' '%S'\n", dirsize, dir, wildfile);
    
    return ajFileNewInList(list);
}




/* @func ajFileNewDF **********************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const AjPStr] Wildcard Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDF(const AjPStr dir, const AjPStr filename)
{
    static AjPStr namefix = NULL;
	
    if(ajStrLen(dir))
	ajStrAssS(&namefix, dir);
    else
	ajStrAssC(&namefix, "./");
    
    if(ajStrChar(namefix, -1) != '/')
	ajStrAppC(&namefix, "/");
    
    ajStrApp(&namefix, filename);
     
    return ajFileNewIn(namefix);
}




/* @func ajFileNewDC **********************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const char*] Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDC(const AjPStr dir, const char* filename)
{
    static AjPStr namefix = NULL;

    if(ajStrLen(dir))
	ajStrAssS(&namefix, dir);
    else
	ajStrAssC(&namefix, "./");

    if(ajStrChar(namefix, -1) != '/')
	ajStrAppC(&namefix, "/");

    ajStrAppC(&namefix, filename);

    return ajFileNewIn(namefix);
}




/* @func ajFileBuffNewInList **************************************************
**
** Creates a new buffered file object from a list of filenames.
**
** @param [r] list [AjPList] List of filenames as strings.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewInList(AjPList list)
{
    AjPFile file;

    file = ajFileNewInList(list);
    if(!file)
	return NULL;

    return ajFileBuffNewFile(file);
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section Buffered File Destructors *****************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/




/* @func ajFileBuffDel ********************************************************
**
** Destructor for a buffered file object.
**
** @param [w] pthis [AjPFileBuff*] Buffered file object.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffDel(AjPFileBuff* pthis)
{
    AjPFileBuff thys;
    
    if(!pthis)
	return;
    
    thys = *pthis;
    
    if(!thys)
	return;

    ajFileBuffClear(thys, -1);
    ajFileBuffFreeClear(thys);
    ajFileClose(&thys->File);
    AJFREE(*pthis);
    
    return;
}




/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Buffered File Assignments *****************************************
**
** These functions overwrite the file provided as the first argument
**
******************************************************************************/


/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section BufferedFile Modifiers ********************************************
**
** These functions use the contents of a file object and update them.
**
******************************************************************************/




/* @func ajFileBuffGet ********************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [r] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGet(AjPFileBuff thys, AjPStr* pdest)
{
    ajlong fpos = 0;

    return ajFileBuffGetL(thys, pdest, &fpos);
}




/* @func ajFileBuffGetStore ***************************************************
**
** Reads a line from a buffered file. Also appends the line to
** a given string if the append flag is true. A double NULL character
** is added afterwards. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [r] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGetStore(AjPFileBuff thys, AjPStr* pdest,
			  AjBool store, AjPStr *astr)
{
    ajlong fpos = 0;
    AjBool ret;

    ret =  ajFileBuffGetL(thys, pdest, &fpos);

    if(store && ret)
    {
	ajDebug("ajFileBuffGetStore:\n%S", *pdest);
	ajStrApp(astr,*pdest);
    }

    return ret;
}




/* @func ajFileBuffGetStoreL **************************************************
**
** Reads a line from a buffered file. Also appends the line to
** a given string if the append flag is true. A double NULL character
** is added afterwards. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [r] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [w] fpos [ajlong*] File position before the read.
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGetStoreL(AjPFileBuff thys, AjPStr* pdest,
			   ajlong* fpos,
			   AjBool store, AjPStr *astr)
{
    AjBool ret;

    ret =  ajFileBuffGetL(thys, pdest, fpos);

    if(store && ret)
    {
	ajDebug("ajFileBuffGetStoreL:\n%S", *pdest);
	ajStrApp(astr,*pdest);
    }

    return ret;
}




/* @func ajFileBuffGetL *******************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [r] thys [AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [w] fpos [ajlong*] File position before the read.
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGetL(AjPFileBuff thys, AjPStr* pdest, ajlong* fpos)
{
    AjBool ok;
    
    /* read from the buffer if it is not empty */
    
    *fpos = 0;
    
    if(thys->Pos < thys->Size)
    {
	ajStrAssS(pdest, thys->Curr->Line);
	*fpos = thys->Curr->Fpos;
	thys->Prev = thys->Curr;
	thys->Curr = thys->Curr->Next;
	thys->Pos++;

	return ajTrue;
    }
    
    /* file has been closed */
    if(!thys->File->Handle)
	return ajFalse;
    
    /* buffer used up - try reading from the file */
    
    ok = ajFileGetsL(thys->File, pdest, &thys->Fpos);
    
    if(!ok)
    {
	if(thys->File->End)
	{
	    if(thys->Size)
	    {
		/* we have data in the buffer - fail */
		ajDebug("End of file - data in buffer - return ajFalse\n");
		return ajFalse;
	    }
	    else
	    {
		/* buffer clear - try another file */
		if(ajFileNext(thys->File))
		{
		    /* OK - read the new file */
		    ok = ajFileBuffGetL(thys, pdest, fpos);
		    ajDebug("End of file - trying next file ok: %B "
			    "fpos: %ld %ld\n",
			    ok, *fpos, thys->Fpos);
		    return ok;
		}
		else
		{
		    /* no new file, fail again */
		    ajDebug("End of file - no new file to read - "
			    "return ajFalse\n");
		    return ajFalse;
		}
	    }
	}
	else
	    ajFatal("Error reading from file '%s'\n",
		    ajFileName(thys->File));
    }
    
    if(thys->Nobuff)
    {
	*fpos = thys->Fpos;
	/*ajDebug("ajFileBuffGetL unbuffered fpos: %ld\n", *fpos);*/
	return ajTrue;
    }
    
    fileBuffLineAdd(thys, *pdest);
    *fpos = thys->Fpos;
    
    return ajTrue;
}




/* @func ajFileBuffStripHtml **************************************************
**
** Processes data in the file buffer, removing HTML tokens between
** angle brackets, plus any TITLE. This seems to be enough to make HTML
** output readable.
**
** @param [r] thys [AjPFileBuff] Buffered file with data loaded
**                                     in the buffer.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffStripHtml(AjPFileBuff thys)
{
    AjPRegexp tagexp   = NULL;
    AjPRegexp fullexp  = NULL;
    AjPRegexp httpexp  = NULL;
    AjPRegexp nullexp  = NULL;
    AjPRegexp chunkexp = NULL;
    AjPRegexp hexexp   = NULL;
    AjPRegexp ncbiexp  = NULL;
    AjPRegexp ncbiexp2 = NULL;
    AjPRegexp srsdbexp = NULL;
    
    AjPStr s1 = NULL;
    AjPStr s2 = NULL;
    AjPStr s3 = NULL;
    ajint i;
    AjBool doChunk = ajFalse;
    ajint ichunk;
    ajint chunkSize;
    ajint iline;
    AjPStr nullLine = NULL;
    AjPStr saveLine = NULL;
    AjPStr hexstr   = NULL;
    
    tagexp   = ajRegCompC("^(.*)(<[!/A-Za-z][^>]*>)(.*)$");
    fullexp  = ajRegCompC("^(.*)(<(TITLE)>.*</TITLE>)(.*)$");
    httpexp  = ajRegCompC("^HTTP/");
    nullexp  = ajRegCompC("^\r?\n?$");
    chunkexp = ajRegCompC("^Transfer-Encoding: +chunked");
    hexexp   = ajRegCompC("^([0-9a-fA-F]+) *\r?\n?$");
    ncbiexp  = ajRegCompC("^Entrez Reports\n$");
    ncbiexp2 = ajRegCompC("^----------------\n$");
    srsdbexp = ajRegCompC("^([A-Za-z0-9_-]+)(:)([A-Za-z0-9_-]+)");
    
    /* first take out the HTTP header (HTTP 1.0 onwards) */
    
    
    i = 0;
    
    ajDebug("First line [%d] '%S' \n",
	     ajStrRef(thys->Curr->Line), thys->Curr->Line);
    
    if(ajRegExec(httpexp, thys->Curr->Line))
    {
	/* ^HTTP */
	while(thys->Pos < thys->Size &&
	      !ajRegExec(nullexp, thys->Curr->Line))
	{
	    /* to empty line */
	    if(ajRegExec(chunkexp, thys->Curr->Line))
	    {
		ajDebug("Chunk encoding: %S", thys->Curr->Line);
		/* chunked - see later */
		doChunk = ajTrue;
	    }
	    fileBuffLineDel(thys);
	}
    }

    if(doChunk)
    {
	ajFileBuffTraceFull(thys, 999999, 0);
	if(!ajRegExec(nullexp, thys->Curr->Line))
	{
	    ajFatal("Bad chunk data from HTTP, expect blank line got '%S'",
		    thys->Curr->Line);
	}
	ajStrAssS(&nullLine, thys->Curr->Line);
	ajDebug("###cleanup 1 %x '%S'\n",
		thys->Curr->Line, thys->Curr->Line);
	/* blank line after header */
	fileBuffLineDel(thys);
	
	if(!ajRegExec(hexexp, thys->Curr->Line))
	{
	    ajDebug("Bad chunk (a), expect chunk size got '%S'\n",
		    thys->Curr->Line);
	    ajFatal("Bad chunk data from HTTP, expect chunk size got '%S'",
		    thys->Curr->Line);
	}
	ajRegSubI(hexexp, 1, &hexstr);
	ajStrToHex(hexstr, &chunkSize);
	
	ajDebug("###cleanup 2 %x '%S'\n",
		thys->Curr->Line, thys->Curr->Line);
	fileBuffLineDel(thys);	/* chunk size */
	
	ichunk = 0;
	iline = 0;
	while(chunkSize && thys->Curr)
	{
	    iline++;
	    /* get the chunk size - zero is the end */
	    /* process the chunk */
	    ichunk += ajStrLen(thys->Curr->Line);
	    
	    ajDebug("++input line [%d] ichunk=%x:%x:%S",
		     iline, ichunk, ajStrLen(thys->Curr->Line),
		     thys->Curr->Line);
	    if(ichunk >= chunkSize)
	    {
		if(ichunk == chunkSize)
		{
		    /* end-of-chunk at end-of-line */
		    ajDebug("# ichunk %x == %x chunkSize\n",
			    ichunk, chunkSize);
		    ajDebug("###cleanup 3 %x '%S'\n",
			    thys->Curr->Line, thys->Curr->Line);
		    fileBuffLineNext(thys);
		    ajStrAssC(&saveLine, "");
		}
		else
		{
		    /* end-of-chunk in mid-line, patch up the input */
		    ajDebug("# ichunk %x >> %x chunkSize\n",
			    ichunk, chunkSize);
		    ajDebug("join ichunk:%d chunkSize:%d 0..%d\n",
			    ichunk, chunkSize, -(ichunk-chunkSize));
		    ajDebug("orig chlist->Line %d '%S'\n",
			    ajStrLen(thys->Curr->Line), thys->Curr->Line);
		    ajStrAssSub(&saveLine, thys->Curr->Line, 0,
				-(ichunk-chunkSize));
		    ajStrSub(&thys->Curr->Line, -(ichunk-chunkSize), -1);
		    ajDebug("... saveLine %d '%S'\n",
			    ajStrLen(saveLine), saveLine);
		    ajDebug("... Curr->Line %d '%S'\n",
			    ajStrLen(thys->Curr->Line), thys->Curr->Line);
		}
		
		/* skip a blank line */
		
		if(!ajRegExec(nullexp, thys->Curr->Line))
		{
		    ajDebug("Bad chunk (b), expect blank line got '%S'",
			    thys->Curr->Line);
		    ajFatal("Bad chunk data from HTTP, expect blank line"
			    " got '%S'", thys->Curr->Line);
		}
		fileBuffLineDel(thys);
		
		/** read the next chunk size */
		
		if(!ajRegExec(hexexp, thys->Curr->Line))
		{
		    ajDebug("Bad chunk (c), expect chunk size got '%S'\n",
			    thys->Curr->Line);
		    ajFatal("Bad chunk data from HTTP, expect chunk size "
			    "got '%S'",
			    thys->Curr->Line);
		}
		ajRegSubI(hexexp, 1, &hexstr);
		ajStrToHex(hexstr, &chunkSize);
		ichunk = 0;
		ajDebug("## %4x:%S", chunkSize, thys->Curr->Line);
	    }
	    ajDebug("chlist time saveLine %x len:%d\n",
		    saveLine, ajStrLen(saveLine));
	    if(saveLine)
	    {
		if(ajStrLen(saveLine))
		{
		    /* preserve the line split by chunksize */
		    ajStrInsert(&thys->Curr->Line, 0, saveLine);
		    ajDebug("new chlist->Line '%S'\n", thys->Curr->Line);
		    fileBuffLineNext(thys); /* after restored line */
		}
		else
		{
		    /* just a chunksize, skip */
		    if(thys->Curr && chunkSize)
		    {
			ajDebug("###cleanup 5 %x '%S'\n",
				thys->Curr->Line, thys->Curr->Line);
			fileBuffLineDel(thys);
		    }
		    else
		    {
			ajDebug("###cleanup 6 %x '%S'\n",
				thys->Curr->Line, thys->Curr->Line);
			fileBuffLineDel(thys);
		    }
		}
		ajStrDel(&saveLine);
	    }
	    else
	    {
		/* next line */
		fileBuffLineNext(thys);
	    }
	}

	ajFileBuffFix(thys);
	ajFileBuffTraceFull(thys, 999999, 0);
	ajStrDel(&hexstr);
	ajStrDel(&nullLine);
    }
    
    ajFileBuffReset(thys);
    
    while(thys->Curr)
    {
	if(ajRegExec(ncbiexp, thys->Curr->Line))
	    ajStrAssC(&thys->Curr->Line, "\n");
	if(ajRegExec(ncbiexp2, thys->Curr->Line))
	    ajStrAssC(&thys->Curr->Line, "\n");

	while(ajRegExec(fullexp, thys->Curr->Line))
	{
	    ajRegSubI(fullexp, 1, &s1);
	    ajRegSubI(fullexp, 2, &s2);
	    ajRegSubI(fullexp, 4, &s3);
	    ajDebug("removing '%S' [%d]\n", s2, ajStrRef(thys->Curr->Line));
	    ajFmtPrintS(&thys->Curr->Line, "%S%S", s1, s3);
	}

	while(ajRegExec(tagexp, thys->Curr->Line))
	{
	    ajRegSubI(tagexp, 1, &s1);
	    ajRegSubI(tagexp, 2, &s2);
	    ajRegSubI(tagexp, 3, &s3);
	    ajDebug("removing '%S' [%d]\n", s2, ajStrRef(thys->Curr->Line));
	    ajFmtPrintS(&thys->Curr->Line, "%S%S", s1, s3);
	    ajDebug("leaving '%S''%S'\n", s1,s3);
	}

	if(ajRegExec(srsdbexp, thys->Curr->Line))
	{
	    ajRegSubI(srsdbexp,1,&s1);
	    ajRegSubI(srsdbexp,2,&s2);
	    ajRegSubI(srsdbexp,3,&s3);
	    ajDebug("removing '%S%S%S' [%d]\n",
		     s1,s2,s3,ajStrRef(thys->Curr->Line));
	    fileBuffLineDel(thys);
	    ++i;
	    continue;
	}

	if(ajRegExec(nullexp, thys->Curr->Line))
	{
	    /* allow for newline */
	    ajDebug("<blank line deleted> [%d]\n",
		     ajStrRef(thys->Curr->Line));
	    fileBuffLineDel(thys);
	}
	else
	{
	    ajDebug(":[%d] %S", ajStrRef(thys->Curr->Line), thys->Curr->Line);
	    fileBuffLineNext(thys);
	}
	i++;
    }
    
    ajFileBuffReset(thys);
    
    ajStrDel(&s1);
    ajStrDel(&s2);
    ajStrDel(&s3);
    
    /* free the regular expressions - we expect to use them once only */
    
    ajRegFree(&tagexp);
    ajRegFree(&fullexp);
    ajRegFree(&httpexp);
    ajRegFree(&nullexp);
    ajRegFree(&chunkexp);
    ajRegFree(&hexexp);
    ajRegFree(&ncbiexp);
    ajRegFree(&ncbiexp2);
    ajRegFree(&srsdbexp);
    
    return;
}




/* @func ajFileBuffLoad *******************************************************
**
** Reads all input lines from a file into the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tagsafter reading via HTTP.
**
** @param [r] thys [AjPFileBuff] Buffered file.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoad(AjPFileBuff thys)
{
    static AjPStr rdline = NULL;
    AjBool stat = ajTrue;

    while(stat)
	stat = ajFileBuffGet(thys, &rdline);

    ajFileBuffReset(thys);

    /*ajFileBuffTrace(thys);*/

    return;
}




/* @func ajFileBuffLoadC ******************************************************
**
** Adds a line to the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [r] thys [AjPFileBuff] Buffered file.
** @param [r] line [const char*] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoadC(AjPFileBuff thys, const char* line)
{
    if(!thys->Lines)
	thys->Curr = thys->Lines = AJNEW0(thys->Last);
    else
	thys->Last = AJNEW0(thys->Last->Next);

    ajStrAssC(&thys->Last->Line,line);
    thys->Last->Next = NULL;
    thys->Size++;

    return;
}




/* @func ajFileBuffLoadS ******************************************************
**
** Adds a copy of a line to the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [r] thys [AjPFileBuff] Buffered file.
** @param [r] line [const AjPStr] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoadS(AjPFileBuff thys, const AjPStr line)
{
    if(!thys->Lines)
	thys->Curr = thys->Lines = AJNEW0(thys->Last);
    else
	thys->Last = AJNEW0(thys->Last->Next);

    ajStrAssS(&thys->Last->Line,line);
    thys->Last->Next = NULL;
    thys->Size++;

    return;
}




/* @func ajFileBuffEof ********************************************************
**
** Tests whether we have reached end of file already
**
** @param [u] thys [const AjPFileBuff] File buffer
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileBuffEof(const AjPFileBuff thys)
{
    return thys->File->End;
}




/* @func ajFileBuffEnd ********************************************************
**
** Tests whether the file is exhausted. This means end of file is reached
** and the buffer is empty
**
** @param [u] thys [const AjPFileBuff] File buffer
** @return [AjBool] ajTrue if we already set end-of-file
** @@
******************************************************************************/

AjBool ajFileBuffEnd(const AjPFileBuff thys)
{
    ajDebug("ajFileBuffEnd End: %B Size: %d\n", thys->File->End, thys->Size);

    /* not reached EOF yet */
    if(!thys->File->End)
	return ajFalse;

    /* Something in the buffer*/
    if(thys->Size != 0)
	return ajFalse;

    return ajTrue;
}




/* @func ajFileBuffReset ******************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffReset(AjPFileBuff thys)
{
    thys->Pos  = 0;
    thys->Curr = thys->Lines;
    thys->Prev = NULL;

    return;
}




/* @func ajFileBuffResetStore *************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [void]
** @@
******************************************************************************/

void ajFileBuffResetStore(AjPFileBuff thys, AjBool store, AjPStr *astr)
{
    ajFileBuffReset(thys);
    if(store)
	ajStrAssC(astr, "");

    return;
}




/* @func ajFileBuffResetPos ***************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** Also resets the file position to the last known read, to undo the
** damage done by (for example) ajseqabi functions.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffResetPos(AjPFileBuff thys)
{
    ajFileBuffTraceFull(thys, 10, 10);

    thys->Pos  = 0;
    thys->Curr = thys->Lines;

    if(!thys->File->End && (thys->File->fp != stdin))
	ajFileSeek(thys->File, thys->Fpos, SEEK_SET);

    ajFileBuffTraceFull(thys,10,10);

    return;
}




/* @func ajFileBuffFix ********************************************************
**
** Resets the pointer and current record of a file buffer so the next
** read starts at the first buffered line. Fixes buffer size after the
** buffer has been edited.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffFix(AjPFileBuff thys)
{
    AjPFileBuffList list;
    ajint i = 0;

    ajFileBuffReset(thys);
    thys->Pos  = 0;
    thys->Curr = thys->Lines;

    list = thys->Lines;
    while(list->Next)
    {
	i++;
	list = list->Next;
    }

    thys->Size=i;
    return;
}




/* @func ajFileBuffFreeClear **************************************************
**
** Deletes freed lines from a file buffer. The free list is used to avoid
** reallocating space for new records and must be deleted as part of
** the destructor.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffFreeClear(AjPFileBuff thys)
{
    AjPFileBuffList list;

    if(!thys)
	return;

    ajDebug("ajFileBuffFreeClear %x\n", thys->Free);

    while(thys->Free)
    {
	list = thys->Free;
	thys->Free = thys->Free->Next;
	ajStrDel(&list->Line);
	AJFREE(list);
    }

    return;
}




/* @func ajFileBuffClear ******************************************************
**
** Deletes processed lines from a file buffer. The buffer has a record
** (Pos) of the next unprocessed line in the buffer.
**
** Unbuffered files need special handling. The buffer can be turned off
** while it still contains data. If so, we have to carefully run it down.
** If this runs it to zero, we may want to save the last line read.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] lines [ajint] Number of lines to retain. -1 deletes everything.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffClear(AjPFileBuff thys, ajint lines)
{
    ajint i = 0;
    AjPFileBuffList list;
    AjPFileBuffList next;
    ajint first;
    ajint ifree = 0;
    
    ajDebug("ajFileBuffClear (%d) Nobuff: %B\n", lines, thys->Nobuff);
    /*FileBuffTraceFull(thys, thys->Size, 100);*/
    if(!thys)
	return;
    
    if(!thys->File)
	return;
    
    if(lines < 0)
	first = thys->Size;
    else
	first = thys->Pos - lines;
    
    if(first < 0)
	first = 0;
    
    /* nobuff, and all read */
    if(thys->Nobuff && thys->Pos == thys->Size)
	/* delete any old saved line */
	first = thys->Pos;
    
    ajDebug(" first: %d thys->Pos: %d thys->Size: %d thys->Nobuff: %B\n",
	    first, thys->Pos, thys->Size, thys->Nobuff);
    
    list = thys->Lines;
    for(i=0; i < first; i++)
    {
	/* we save one line at a time */
	next = list->Next;
	/* so keep a note of the next one for later */
	/*ajDebug("Try to reuse %x size: %d use: %d\n",
	  list->Line, ajStrSize(list->Line), ajStrRef(list->Line));*/
	
	if(thys->Nobuff)
	    ajStrDel(&list->Line);
	
	if(!thys->Nobuff &&
	    ajStrDelReuse(&list->Line))
	{
	    /* move free line to the end */

	    /*ajDebug("can save to free list %x %d bytes\n",
	      list->Line, ajStrSize(list->Line));*/

	    ifree++;
	    /* just save the one line */
	    list->Next = NULL;
	    if(!thys->Free)
	    {
		/* start a new free list */
		thys->Free = list;
		thys->Freelast = list;
		/*ajDebug("start  list Free %x Freelast %x \n",
		  thys->Free, thys->Freelast);*/
	    }
	    else
	    {
		/* append to free list */
		thys->Freelast->Next = list;
		thys->Freelast = thys->Freelast->Next;
		/*ajDebug("append list Free %x Freelast %x \n",
		  thys->Free, thys->Freelast);*/
	    }
	}
	else
	{
	    /*ajDebug("have to delete this line\n'%S'\n", list->Line);*/
	    AJFREE(list);	     /* deleted, kill the list item */
	}
	list = next;
    }
    
    ajDebug("ajFileBuffClear '%F' (%d lines)\n"
	     "     %b size: %d pos: %d removed %d lines add to free: %d\n",
	     thys->File, lines, thys->Nobuff, thys->Size, thys->Pos, i, ifree);
    
    thys->Pos   = 0;
    thys->Size -= i;
    
    thys->Lines = thys->Curr = list;
    
    if(thys->Nobuff && !thys->Size && lines == 1)
    {
	/* unbuffered - can only save last line */
	/*ajDebug("Nobuff in effect, saving last line read\n'%S'\n",
	  thys->File->Buff);*/
	if(thys->Lines)
	    ajFatal("Buffer error clearing unbuffered file "
		    "in ajFileBuffClear\n");

	thys->Lines = AJNEW0(thys->Last);

	ajStrAssS(&thys->Last->Line, thys->File->Buff);
	thys->Curr = thys->Last;
	thys->Curr->Fpos = thys->Fpos;
	thys->Last->Next = NULL;
	thys->Pos = 0;
	thys->Size = 1;
    }
    
    return;
}




/* @func ajFileBuffClearStore *************************************************
**
** Deletes processed lines from a file buffer. The buffer has a record
** (Pos) of the next unprocessed line in the buffer.
**
** Unbuffered files need special handling. The buffer can be turned off
** while it still contains data. If so, we have to carefully run it down.
** If this runs it to zero, we may want to save the last line read.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] lines [ajint] Number of lines to retain. -1 deletes everything.
** @param [r] rdline [AjPStr] Last line of input.
**                            Used to count characters to be saved
** @param [r] store [AjBool] append if true
** @param [w] astr [AjPStr*] string to append to
** @return [void]
** @@
******************************************************************************/

void ajFileBuffClearStore(AjPFileBuff thys, ajint lines,
			   AjPStr rdline, AjBool store, AjPStr *astr)
{
    ajFileBuffClear(thys, lines);
    if(store && ajStrLen(rdline))
	ajStrTrim(astr, -ajStrLen(rdline));

    return;
}




/* @func ajFileBuffNobuff *****************************************************
**
** Sets file to be unbuffered. If it already has buffered data, we have to
** first run down the buffer.
**
** @param [r] thys [AjPFileBuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @@
******************************************************************************/

AjBool ajFileBuffNobuff(AjPFileBuff thys)
{
    AjBool ret;

    if(!thys)
	return ajFalse;

    ret = thys->Nobuff;
    ajDebug("ajFileBuffNobuff %F buffsize: %d\n", thys->File, thys->Size);
    thys->Nobuff = ajTrue;

    return ret;
}


/* @func ajFileBuffIsbuff *****************************************************
**
** Sets file to be buffered.
**
** @param [r] thys [AjPFileBuff] Buffered file object.
** @return [AjBool] ajTrue if the file was unbuffered before
** @@
******************************************************************************/

AjBool ajFileBuffIsbuff(AjPFileBuff thys)
{
    AjBool ret;

    if(!thys)
	return ajFalse;

    ret = thys->Nobuff;
    ajDebug("ajFileBuffIsbuff %F buffsize: %d\n", thys->File, thys->Size);
    thys->Nobuff = ajFalse;

    return ret;
}




/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Buffered File Operators *******************************************
**
** These functions use the contents of a file object but do not make
** any changes.
**
******************************************************************************/




/* @func ajFileBuffTrace ******************************************************
**
** Writes debug messages to indicate the contents of a buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffTrace(const AjPFileBuff thys)
{
    AjPFileBuffList test;
    ajint i = 0;
    ajint j = -1;

    ajDebug("Trace buffer file '%S'\n"
	     "             Pos: %d Size: %d End: %b\n",
	     thys->File->Name, thys->Pos, thys->Size, thys->File->End);

    if(thys->Size)
    {
	ajDebug(" Lines:\n");
	ajDebug("  Curr: %8ld <%S>\n", thys->Curr->Fpos,  thys->Curr->Line);
	ajDebug("  From: %8ld <%S>\n", thys->Lines->Fpos, thys->Lines->Line);
	ajDebug("    To: %8ld <%S>\n", thys->Last->Fpos,  thys->Last->Line);
    }

    if(thys->Free)
	for(test = thys->Free; test; test=test->Next)
	{
	    i++;
	    if(test == thys->Freelast) j = i;
	}

    ajDebug(" Free: %d Last: %d\n", i, j);

    return;
}




/* @func ajFileBuffTraceFull **************************************************
**
** Writes debug messages to show the full contents of a buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @param [r] nlines [size_t] Maximum number of lines to trace.
** @param [r] nfree [size_t] Maximum number of free lines to trace.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffTraceFull(const AjPFileBuff thys, size_t nlines,
			  size_t nfree)
{
    ajint i;
    AjPFileBuffList line;
    AjBool last = ajFalse;

    ajDebug("Trace buffer file '%S' End: %B\n"
	     "             Pos: %d Size: %d Nobuff: %B Fpos: %ld\n",
	     thys->File->Name, thys->File->End,
	     thys->Pos, thys->Size, thys->Nobuff, thys->Fpos);

    line = thys->Lines;
    for(i=1; line && (i <= nlines); i++)
    {
	if(line == thys->Curr)
	    ajDebug("*Line %x %d: %5d %5d <%-20S>\n",
		     line->Line, i,
		     ajStrLen(line->Line), strlen(ajStrStr(line->Line)),
		     line->Line);
	else
	    ajDebug(" Line %x %d: %5d %5d <%-20S>\n",
		     line->Line, i,
		     ajStrLen(line->Line), strlen(ajStrStr(line->Line)),
		     line->Line);
	line = line->Next;
    }

    line = thys->Free;
    for(i=1; line && (i <= nfree);  i++)
    {
	if(line == thys->Freelast) last = ajTrue;
	ajDebug(" Free %x %d: %d bytes %B\n",
		 line->Line, i, ajStrSize(line->Line), last);
	line = line->Next;
    }

    return;
}




/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Buffered File Casts ***********************************************
**
** These functions examine the contents of a file object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/




/* @func ajFileBuffFp *********************************************************
**
** Returns the C file pointer for an open buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFileBuffFp(const AjPFileBuff thys)
{
    return thys->File->fp;
}




/* @func ajFileBuffFile *******************************************************
**
** Returns the file object from a buffered file object.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [AjPFile] File object.
** @@
******************************************************************************/

AjPFile ajFileBuffFile(const AjPFileBuff thys)
{
    return thys->File;
}




/* @func ajFileBuffEmpty ******************************************************
**
** Tests whether a file buffer is empty.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [AjBool] ajTrue if the buffer is empty.
** @@
******************************************************************************/

AjBool ajFileBuffEmpty(const AjPFileBuff thys)
{
    ajDebug("ajFileBuffEmpty Size: %d Pos: %d End: %b Handle: %d "
	     "Fp: %x List; %d\n",
	     thys->Size, thys->Pos, thys->File->End, thys->File->Handle,
	     thys->File->fp, ajListstrLength(thys->File->List));

    if(thys->Pos < thys->Size)
	return ajFalse;

    /* not open */
    if(!thys->File->fp || !thys->File->Handle)
	return ajTrue;

    if(thys->File->End && !ajListstrLength(thys->File->List))
        /* EOF and done */
	return ajTrue;

    ajDebug("ajFileBuffEmpty false\n");

    return ajFalse;
}




/* @func ajFileNameDirSet *****************************************************
**
** Sets the directory part of a filename
**
** @param [r] filename [AjPStr*] Filename.
** @param [r] dir [const AjPStr] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameDirSet(AjPStr* filename, const AjPStr dir)
{
    if(!ajStrLen(dir))
	return ajFalse;

    return ajFileNameDirSetC(filename, ajStrStr(dir));
}




/* @func ajFileNameDirSetC ****************************************************
**
** Sets the directory part of a filename
**
** @param [r] filename [AjPStr*] Filename.
** @param [r] dir [const char*] Directory
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameDirSetC(AjPStr* filename, const char* dir)
{
    static AjPStr tmpstr = NULL;
    static AjPStr tmpdir = NULL;
    static AjPStr tmpnam = NULL;

    if(!dir)
	return ajFalse;

    if(!fileFilenameExp)
	fileFilenameExp = ajRegCompC("(.*/)?([^/]+)$");

    ajStrAssS(&tmpstr, *filename);
    if(ajRegExec(fileFilenameExp, tmpstr))
    {
	ajRegSubI(fileFilenameExp, 1, &tmpdir);
	ajRegSubI(fileFilenameExp, 2, &tmpnam);

	/* we already have a directory */
	if(ajStrLen(tmpdir))
	    return ajFalse;

	if(dir[strlen(dir)-1] == '/')
	    ajFmtPrintS(filename, "%s%S", dir, tmpnam);
	else
	    ajFmtPrintS(filename, "%s/%S", dir, tmpnam);
    }

    return ajTrue;
}




/* @func ajFileNameExt ********************************************************
**
** Replaces the extension part of a filename
**
** @param [r] filename [AjPStr*] Filename.
** @param [r] extension [const AjPStr] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameExt(AjPStr* filename, const AjPStr extension)
{
    if(!extension)
	return ajFileNameExtC(filename, NULL);

    return ajFileNameExtC(filename, ajStrStr(extension));
}




/* @func ajFileNameExtC *******************************************************
**
** Replaces the extension part of a filename
**
** @param [r] filename [AjPStr*] Filename.
** @param [r] extension [const char*] New file extension
** @return [AjBool] ajTrue if the replacement succeeded.
** @@
******************************************************************************/

AjBool ajFileNameExtC(AjPStr* filename, const char* extension)
{
    static AjPStr tmpstr = NULL;
    AjBool doext;
    char   *p = NULL;

    doext = ajTrue;
    if(!extension || !*extension)
	doext = ajFalse;


    ajDebug("ajFileNameExtC '%S' '%s'\n", *filename, extension);
    ajStrAssC(&tmpstr,ajStrStr(*filename));
    p = strrchr(ajStrStr(tmpstr),'.');
    if(p)
    {
	*p='\0';
	tmpstr->Len = p - ajStrStr(tmpstr);
    }


    if(doext)
    {
	ajStrAppC(&tmpstr,".");
	ajStrAppC(&tmpstr,extension);
    }

    ajStrAssC(filename,ajStrStr(tmpstr));

    ajDebug("result '%S'\n", *filename);

    return ajTrue;
}




/* @func ajFileScan ***********************************************************
**
** Recursively scan through a directory
**
** @param [r] path [AjPStr] Directory to scan
** @param [r] filename [AjPStr] Filename to search for (or NULL)
** @param [w] result [AjPList *] List for matching filenames
** @param [r] show [AjBool] Print all files found if set
** @param [r] dolist [AjBool] Store all filenames in a list (if set)
** @param [w] list [AjPList *] List for dolist results
** @param [r] rlist [AjPList] List of directories to ignore
** @param [r] recurs [AjBool] Do recursion
** @param [w] outf [const AjPFile] File for "show" results (or NULL)
**
** @return [ajint] number of entries in list
** @@
******************************************************************************/

ajint ajFileScan(AjPStr path, AjPStr filename, AjPList *result,
		 AjBool show, AjBool dolist, AjPList *list,
		 AjPList rlist, AjBool recurs, const AjPFile outf)
{
    AjPList dirs = NULL;
    AjIList iter = NULL;
    DIR *indir;
#if defined(AJ_IRIXLF)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif
    AjPStr s = NULL;
    AjPStr t = NULL;
    AjBool flag;
    AjPStr tpath = NULL;
#ifdef _POSIX_C_SOURCE
    char buf[sizeof(struct dirent)+MAXNAMLEN];
#endif
    
    tpath = ajStrNew();
    ajStrAssC(&tpath,ajStrStr(path));
    
    
    if(dolist)
    {
	t=ajStrNewC(ajStrStr(path));
	ajListPushApp(*list,(void *)t);
    }
    
    if(show)
	ajFmtPrintF(outf,"\n\nDIRECTORY: %s\n\n",ajStrStr(path));
    
    if(!ajFileDir(&tpath))
    {
	ajStrDel(&tpath);
	return 0;
    }


    if(!(indir=opendir(ajStrStr(tpath))))
    {
	ajStrDel(&tpath);
	return 0;
    }
    
    
    s = ajStrNew();
    dirs = ajListNew();
    
#if defined(AJ_IRIXLF)
#ifdef _POSIX_C_SOURCE
    while(!readdir64_r(indir,(struct dirent64 *)buf,&dp))
    {
	if(!dp)
	    break;
#else
	while((dp=readdir64(indir)))
	{
#endif
#else
#ifdef _POSIX_C_SOURCE
    while(!readdir_r(indir,(struct dirent *)buf,&dp))
    {
	if(!dp)
	    break;
#else
	while((dp=readdir(indir)))
	{
#endif
#endif

	if(!dp->d_ino ||
	   !strcmp(dp->d_name,".") ||
	   !strcmp(dp->d_name,".."))
	    continue;
	ajStrAssC(&s,ajStrStr(tpath));
	/*	ajStrAppC(&s,"/");*/
	ajStrAppC(&s,dp->d_name);

	/* Its a directory */
	if(ajFileStat(&s,AJ_FILE_DIR))
	{
	    if(!recurs)
		continue;

	    /* Ignore selected directories */
	    if(rlist)
	    {
		flag = ajFalse;
		iter = ajListIter(rlist);
		while(ajListIterMore(iter))
		{
		    t = ajListIterNext(iter);
		    if(!strcmp(ajStrStr(t),dp->d_name))
		    {
			flag = ajTrue;
			break;
		    }
		}
		ajListIterFree(iter);
		if(flag)
		    continue;
	    }

	    if(!ajFileStat(&s,AJ_FILE_R) || !ajFileStat(&s,AJ_FILE_X))
		continue;
	    t = ajStrNewC(ajStrStr(s));
	    ajListPushApp(dirs,(void *)t);
	}
	else if(ajFileStat(&s,AJ_FILE_R))
	{
	    if(filename)
		if(ajStrMatchWildCC(dp->d_name,ajStrStr(filename)))
		{
		    t = ajStrNewC(ajStrStr(s));
		    ajListPushApp(*result,(void *)t);
		}
	    
	    if(dolist)
	    {
		t = ajStrNewC(ajStrStr(s));
		ajListPushApp(*list,(void *)t);
	    }
	    
	    if(show)
		ajFmtPrintF(outf,"  %s\n",dp->d_name);
	}
    }
    closedir(indir);
    
    if(recurs)
	while(ajListPop(dirs,(void **)&t))
	{
	    ajFileScan(t,filename,result,show,dolist,list,rlist,recurs,outf);
	    ajStrDel(&t);
	}
    
    ajStrDel(&s);
    ajStrDel(&tpath);
    ajListDel(&dirs);
    
    if(result)
	return ajListLength(*result);
    
    return 0;
}




/* @func ajFileTestSkip *******************************************************
**
** Tests a filename against wildcard
** lists of file names to be included and excluded.
**
** By default files are included. The exclusion list is used to trim
** out files, and the inclusion list is then used to add selected
** files again.
**
** @param [r] fullname [AjPStr] File to test
** @param [r] exc [AjPStr] List of wildcard names to exclude
** @param [r] inc [AjPStr] List of wildcard names to include
** @param [r] keep [AjBool] Default to keep if ajTrue, else skip unless
**                          inc is matched.
** @param [r] ignoredirectory [AjBool] Delete directory from name
**                                     before testing.
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFileTestSkip(AjPStr fullname, AjPStr exc, AjPStr inc,
		       AjBool keep, AjBool ignoredirectory)
{
    AjBool ret = keep;

    static AjPStrTok handle = NULL;
    static AjPStr token     = NULL;
    static AjPStr tmpname   = NULL;
    
    ajStrAssS(&tmpname, fullname);
    if(ignoredirectory)
	ajFileDirTrim(&tmpname);
    
    if(keep)
    {
	/* keep, so test exclude first */
	ajStrTokenAss(&handle, exc, " \t,;\n");

	while(ajStrToken(&token, &handle, NULL))
	    if(ajStrMatchWild(fullname, token) ||
		(ignoredirectory && ajStrMatchWild(tmpname, token)))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }
    
    ajStrTokenAss(&handle, inc, " \t,;\n");

    while(ajStrToken(&token, &handle, NULL))
	if(ajStrMatchWild(fullname, token) ||
	    (ignoredirectory && ajStrMatchWild(tmpname, token)))
	    ret = ajTrue;

    ajStrTokenReset(&handle);
    
    if(!keep)
    {				       /* nokeep, test exclude last */
	ajStrTokenAss(&handle, exc, " \t,;\n");

	while(ajStrToken(&token, &handle, NULL))
	    if(ajStrMatchWild(fullname, token) ||
		(ignoredirectory && ajStrMatchWild(tmpname, token)))
		ret = ajFalse;

	ajStrTokenReset(&handle);
    }
    
    return ret;
}




/* @func ajFileDirTrim ********************************************************
**
** Trims the directory path (if any) from a filename
**
** @param [u] name [AjPStr*] Filename
** @return [AjBool] ajTrue is there was a directory
******************************************************************************/

AjBool ajFileDirTrim(AjPStr* name)
{
    ajint i;

    if(!ajStrLen(*name))
	return ajFalse;

    i = ajStrRFindC(*name, "/");
    if(i < 0)
	return ajFalse;

    ajStrTrim(name, i+1);

    return ajTrue;
}




/* @func ajFileTempName *******************************************************
**
** Returns an available temporary filename that can be opened for writing
** Filename will be of the form progname-time.randomnumber
** Tries 5 times to find a new filename. Returns NULL if not
** successful or the file cannot be opened for writing.
** This function returns only the filename, not a file pointer.
**
** @param [r] dir [const char*] Directory for filename
**                              or NULL for current dir (.)
** @return [char*] available filename or NULL if error.
** @@
******************************************************************************/

char* ajFileTempName(const char *dir)
{
#if defined(AJ_IRIXLF)
    struct  stat64 buf;
#else
    struct  stat buf;
#endif
    static  AjPStr dt = NULL;
    AjPStr  direct;
    ajint   retry;
    AjBool  ok;
    AjPFile outf;
    
    if(!dt)
	dt     = ajStrNew();
    
    direct = ajStrNew();
    
    if(!dir)
	ajStrAssC(&direct,".");
    else
	ajStrAssC(&direct,dir);
    ajStrAppC(&direct,"/");
    
    
    
    ajFmtPrintS(&dt,"%S%s-%d.%d",direct,ajAcdProgram(),time(0),
		ajRandomNumber());
    
    retry = 5;
    ok    = ajTrue;
    
#if defined(AJ_IRIXLF)
    while(!stat64(ajStrStr(dt),&buf) && retry)
#else
	while(!stat(ajStrStr(dt),&buf) && retry)
#endif
	{
	    ajFmtPrintS(&dt,"%S%s-%d.%d",direct,ajAcdProgram(),time(0),
			ajRandomNumber());
	    --retry;
	}
    
    if(!retry)
    {
	ajDebug("Cannot find a unique filename [last try %S]\n",dt);
	ok = ajFalse;
    }
    
    if(!(outf = ajFileNewOut(dt)))
    {
	ajDebug("Cannot write to file %S\n",dt);
	ok = ajFalse;
    }
    else
    {
	ajFileClose(&outf);
	unlink(ajStrStr(dt));
    }
    
    ajStrDel(&direct);
    
    if(!ok)
	return NULL;
    
    return ajStrStr(dt);
}




/* @func ajFileWriteByte ******************************************************
**
** Writes a single byte to a binary file
**
** @param [r] thys [const AjPFile] Output file
** @param [r] ch [char] Character
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteByte(const AjPFile thys, char ch)
{
    return fwrite(&ch, 1, 1, ajFileFp(thys));
}




/* @func ajFileWriteChar ******************************************************
**
** Writes a text string to a binary file
**
** @param [r] thys [const AjPFile] Output file
** @param [r] str [char*] Text string
** @param [r] len [ajint] Length (padded) to use in the file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteChar(const AjPFile thys, char* str, ajint len)
{
    static char buf[256];
    ajint i;

    i = strlen(str);

    strcpy(buf, str);
    if(i < len)
	memset(&buf[i], '\0', len-i);

    return fwrite(buf, len, 1, ajFileFp(thys));
}




/* @func ajFileWriteInt2 ******************************************************
**
** Writes a 2 byte integer to a binary file, with the correct byte orientation
**
** @param [r] thys [const AjPFile] Output file
** @param [r] i [short] Integer
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteInt2(const AjPFile thys, short i)
{
    short j;

    j = i;
    if(ajUtilBigendian())
	ajUtilRev2(&j);

    return fwrite(&j, 2, 1, ajFileFp(thys));
}




/* @func ajFileWriteInt4 ******************************************************
**
** Writes a 4 byte integer to a binary file, with the correct byte orientation
**
** @param [r] thys [const AjPFile] Output file
** @param [r] i [ajint] Integer
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteInt4(const AjPFile thys, ajint i)
{
    ajint j;

    j = i;

    if(ajUtilBigendian())
	ajUtilRev4(&j);

    return fwrite(&j, 4, 1, ajFileFp(thys));
}




/* @func ajFileWriteInt8 ******************************************************
**
** Writes an 8 byte long to a binary file, with the correct byte orientation
**
** @param [r] thys [const AjPFile] Output file
** @param [r] l [ajlong] Integer
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteInt8(const AjPFile thys, ajlong l)
{
    ajlong j;

    j = l;

    if(ajUtilBigendian())
	ajUtilRev8(&j);

    return fwrite(&j, 8, 1, ajFileFp(thys));
}




/* @func ajFileWriteStr *******************************************************
**
** Writes a string to a binary file
**
** @param [r] thys [const AjPFile] Output file
** @param [r] str [AjPStr] String
** @param [r] len [ajint] Length (padded) to use in the file
** @return [ajint] Return value from fwrite
** @@
******************************************************************************/

ajint ajFileWriteStr(const AjPFile thys, AjPStr str, ajint len)
{
    static char buf[256];
    ajint i;

    i = ajStrLen(str);
    strcpy(buf, ajStrStr(str));
    if(i < len)
	memset(&buf[i], '\0', len-i);

    return fwrite(buf, len, 1, ajFileFp(thys));
}




/* @func ajFileBuffStripSrs  **************************************************
**
** Strip out SRS6.1 header lines.
**
** @param [u] thys [AjPFileBuff] buffer
** @return [ajint] Return 1=success 0=not SRS6.1
** @@
******************************************************************************/
ajint ajFileBuffStripSrs(AjPFileBuff thys)
{
    AjPFileBuffList lptr    = NULL;
    AjPFileBuffList tptr    = NULL;
    AjPFileBuffList lastptr = NULL;
    
    AjBool found;
    
    found = ajFalse;
    lptr = thys->Curr;
    
    
    lptr=thys->Curr;
    
    /* SRS 6.1 etc has '&nbsp' lines. If not found then return false */
    /* There must be a better way but LION were unresponsive         */
    while(lptr && !found)
    {
	if(ajStrPrefixC(lptr->Line,"&nbsp"))
	    found = ajTrue;
	lptr = lptr->Next;
    }
    
    if(!found)
	return ajFalse;
    
    found = ajFalse;
    lptr=thys->Curr;
    
    while(lptr && !found)
    {
	if(!ajStrPrefixC(lptr->Line,"<pre>"))
	{
	    tptr = lptr;
	    lptr = lptr->Next;
	    thys->Curr=lptr;
	    ajStrDel(&tptr->Line);
	    AJFREE(tptr);
	    thys->Size--;
	}
	else
	    found = ajTrue;
    }
    
    thys->Lines = thys->Curr;
    
    
    
    while(lptr && !ajStrPrefixC(lptr->Line,"</pre>"))
    {
	lastptr = lptr;
	lptr    = lptr->Next;
    }
    
    
    
    while(lptr)
    {
	while(lptr && !ajStrPrefixC(lptr->Line,"<pre>"))
	{
	    tptr = lptr;
	    lptr = lptr->Next;
	    ajStrDel(&tptr->Line);
	    AJFREE(tptr);
	    thys->Size--;
	}

	if(!lptr)
	{
	    lastptr->Next = NULL;
	    continue;
	}

	lastptr->Next = lptr;
	while(lptr && !ajStrPrefixC(lptr->Line,"</pre>"))
	{
	    lastptr = lptr;
	    lptr    = lptr->Next;
	}
    }
    
    lptr = thys->Curr;
    while(lptr)
    {
	ajStrRemoveHtml(&lptr->Line);
	lptr = lptr->Next;
    }
    
    return ajTrue;
}




/* @funcstatic fileListRecurs  ************************************************
**
** Add a filename, expanded wildcard filenames and list file contents to
** a list
**
** @param [r] srcfile [const AjPStr] filename, wildfilename or listfilename
** @param [w] list [AjPList] result filename list
** @param [u] recurs [ajint *] recursion level counter
**
** @return [void]
** @@
******************************************************************************/

static void fileListRecurs(const AjPStr srcfile, AjPList list, ajint *recurs)
{
    char c;
    AjPStr ptr = NULL;
    AjPStr dir = NULL;
    char   *p;
    AjPList dlist;
    AjPFile inf;
    AjPStr  line = NULL;
    AjPStr file  = NULL;
    
    ++(*recurs);
    if(*recurs > FILERECURSLV)
	ajFatal("Filelist maximum recursion level reached");

    ajStrAssS(&file, srcfile);
    ajStrChomp(&file);
    c = *ajStrStr(file);
    
    dir   = ajStrNew();
    line  = ajStrNew();
    dlist = ajListNew();
    
    
    if(ajStrIsWild(file))
    {
	if(!(p=strrchr(ajStrStr(file),(int)'/')))
	    ajStrAssC(&dir,"./");
	else
	    ajStrAssSubC(&dir,ajStrStr(file),0,p-ajStrStr(file));
	ajFileScan(dir,file,&dlist,ajFalse,ajFalse,NULL,NULL,ajFalse,NULL);
	while(ajListPop(dlist,(void **)&ptr))
	{
	    if(ajStrPrefixC(ptr,"./"))
		ajStrTrim(&ptr,2);
	    ajListPushApp(list,(void *)ptr);
	}
    }
    else if(c=='@')
    {
	if((inf=ajFileNewInC(ajStrStr(file)+1)))
	    while(ajFileReadLine(inf,&line))
		fileListRecurs(line,list,recurs);
	if(inf)
	    ajFileClose(&inf);
    }
    else if(ajStrPrefixC(file,"list::"))
    {
	if((inf=ajFileNewInC(ajStrStr(file)+6)))
	    while(ajFileReadLine(inf,&line))
		fileListRecurs(line,list,recurs);
	if(inf)
	    ajFileClose(&inf);
    }
    else
    {
	ptr = ajStrNewC(ajStrStr(file));
	ajListPushApp(list,(void *)ptr);
    }
    
    
    ajListDel(&dlist);
    ajStrDel(&dir);
    ajStrDel(&line);
    ajStrDel(&file);
    
    --(*recurs);
    
    return;
}




/* @func ajFileFileList  ******************************************************
**
** Return a list of files that match a comma-separated string of
** filenames which can include wildcards or listfiles
**
** @param [r] files [AjPStr] comma-separated filename list
**
** @return [AjPList] or NULL if no files were specified
** @@
******************************************************************************/

AjPList ajFileFileList(AjPStr files)
{
    AjPStr *fstr = NULL;
    ajint  ncl;
    ajint  i;
    ajint  rlevel = 0;
    AjPList list;

    list = ajListNew();

    ncl = ajArrCommaList(files,&fstr);
    for(i=0;i<ncl;++i)
    {
	fileListRecurs(fstr[i],list,&rlevel);
	ajStrDel(&fstr[i]);
    }

    AJFREE(fstr);

    if(!ajListLength(list))
    {
	ajListDel(&list);
	return NULL;
    }

    return list;
}




/* @funcstatic fileBuffLineAdd ************************************************
**
** Appends a line to a buffer.
**
** @param [u] thys [AjPFileBuff] File buffer
** @param [r] line [AjPStr] Line
** @return [void]
******************************************************************************/

static void fileBuffLineAdd(AjPFileBuff thys, AjPStr line)
{
    if(thys->Free)
    {
	/*ajDebug("using Free %x %d\n",
	  thys->Free->Line, ajStrSize(thys->Free->Line));*/
	if(!thys->Lines)
	{
	    /* Need to set first line in list */
	    thys->Lines = thys->Free;
	    /*ajDebug("using Free: set new first line\n");*/
	}
	else
	    thys->Last->Next = thys->Free;

	thys->Last = thys->Free;
	thys->Free = thys->Free->Next;
	if(!thys->Free)
	{
	    /* Free list now empty */
	    thys->Freelast = NULL;
	    /*ajDebug("using Free: Free now empty\n");*/
	}
    }
    else
    {
	/* No Free list, make a new string */
	if(!thys->Lines)
	{
	    thys->Lines = AJNEW0(thys->Last);
	    /*ajDebug("new first line\n");*/
	}
	else
	{
	    thys->Last = AJNEW0(thys->Last->Next);
	    /*Debug("new last line\n");*/
	}
    }
    
    ajStrAssS(&thys->Last->Line, line);
    thys->Prev = thys->Curr;
    thys->Curr = thys->Last;
    thys->Last->Next = NULL;
    thys->Last->Fpos = thys->Fpos;
    thys->Pos++;
    thys->Size++;
        
    return;
}




/* @funcstatic fileBuffLineDel ************************************************
**
** Delete a line from a buffer.
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [void]
******************************************************************************/

static void fileBuffLineDel(AjPFileBuff thys)
{
    if(!thys->Curr)
	return;

    ajDebug("fileBuffLineDel removing line [%d], '%S' len %d\n",
	     ajStrRef(thys->Curr->Line), thys->Curr->Line,
	     ajStrLen(thys->Curr->Line));

    /* first line */
    if(!thys->Prev)
    {
	thys->Prev = thys->Lines;
	thys->Curr = thys->Lines = thys->Lines->Next;
	ajStrDel(&thys->Prev->Line);
	AJFREE(thys->Prev);
	--thys->Size;
	if(thys->Curr)
	    ajDebug("first line gone, new start [%d] %x, '%S' len %d\n",
		     ajStrRef(thys->Curr->Line), thys->Curr->Line,
		     thys->Curr->Line,
		     ajStrLen(thys->Curr->Line));
	else
	    ajDebug("first line gone, current line gone, Size %d Pos %d\n",
		    thys->Size, thys->Pos);

	return;
    }

    thys->Prev->Next = thys->Curr->Next;
    ajStrDel(&thys->Curr->Line);
    AJFREE(thys->Curr);
    thys->Curr = thys->Prev->Next;
    --thys->Size;
    if(thys->Curr)
	ajDebug("new next line  [%d] %x, '%S' len %d\n",
		 ajStrRef(thys->Curr->Line),
		 thys->Curr->Line, thys->Curr->Line,
		 ajStrLen(thys->Curr->Line));
    else
	ajDebug("no next line, Size %d Pos %d\n",
		thys->Size, thys->Pos);

    return;
}




/* @funcstatic fileBuffLineNext ***********************************************
**
** Steps the Curr pointer to the next line in a buffer.
**
** Not for use when reading from a file. This steps through the buffer
**
** @param [u] thys [AjPFileBuff] File buffer
** @return [AjBool] ajTrue if there was another line
******************************************************************************/

static AjBool fileBuffLineNext(AjPFileBuff thys)
{
    if(thys->Pos < thys->Size)
    {
	thys->Prev = thys->Curr;
	thys->Curr = thys->Curr->Next;
	thys->Pos++;
	return ajTrue;
    }

    return ajFalse;
}

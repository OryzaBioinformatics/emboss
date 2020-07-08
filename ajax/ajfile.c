/********************************************************************
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
********************************************************************/

#include "ajax.h"
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

static int fileBuffSize = 2048;

static int fileHandle = 0;
static int fileOpenCnt = 0;
static int fileOpenMax = 0;
static int fileCloseCnt = 0;
static int fileOpenTot = 0;

static void fileClose (const AjPFile thys);
static DIR* fileOpenDir (AjPStr *dir);

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section File Constructors ************************************************
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

/* @func ajFileNew *******************************************************
**
** Creates a new file object.
**
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNew (void) {

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

  if (fileOpenCnt > fileOpenMax)
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

AjPFile ajFileNewInPipe (const AjPStr name) {

  AjPFile thys;

  int pid;
  int pipefds[2];  /* file descriptors for a pipe */
  static AjPStr tmpname = NULL;
  char** arglist = NULL;
  char* pgm;

  AJNEW0(thys);
  (void) ajStrAssS (&tmpname, name);
  (void) ajStrTrim (&tmpname, -1);
  if (pipe(pipefds) < 0)
    ajFatal("pipe create failed");
  pid = fork();  /* negative return indicates failure */
  if (pid < 0)
    ajFatal("fork create failed");
  /* pid is zero in the child, but is the child PID in the parent */
  
  if (!pid) { /* this is the child process */
    (void) close (pipefds[0]);

    (void) dup2 (pipefds[1], 1);
    (void) close (pipefds[1]);
    (void) ajSysArglist (tmpname, &pgm, &arglist);
    ajDebug ("execvp ('%S', NULL)\n", tmpname);
    (void) execvp (pgm, arglist);
    ajFatal ("execvp failed\n"); /* should never get here */
  }

  ajDebug ("pid %d, pipe '%d', '%d'\n",
	  pid, pipefds[0], pipefds[1]);
  /* fp is what we read from the pipe */
  thys->fp = ajSysFdopen(pipefds[0], "r");
  (void) close (pipefds[1]);
  ajStrDelReuse (&tmpname);

  if (!thys->fp) {
    thys->Handle = 0;
    return NULL;
  }
  thys->Handle = ++fileHandle;
  thys->Name = ajStrDup(name);
  thys->End = ajFalse;

  fileOpenCnt++;
  fileOpenTot++;
  if (fileOpenCnt > fileOpenMax)
    fileOpenMax = fileOpenCnt;

  return thys;
}

/* @func ajFileNewIn *******************************************************
**
** Creates a new file object to read a named file.
**
** If the filename begins with a pipe character then a pipe is opened
** using ajFileNewInPipe.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewIn (const AjPStr name) {

    AjPFile thys;
    static AjPStr userstr = NULL;
    static AjPStr reststr = NULL;
    static AjPStr tmpname = NULL;
    static AjPRegexp userexp = NULL;
    static AjPRegexp wildexp = NULL;
    struct passwd* pass = NULL;
    AjPStr dirname=NULL;
    AjPStr wildname=NULL;
    char   *p=NULL;
    
    ajDebug("ajFileNewIn '%S'\n", name);
    (void) ajStrAssS(&tmpname, name);

    if (ajStrMatchC(tmpname, "stdin"))
	return ajFileNewF(stdin);

    if (ajStrChar(tmpname, -1) == '|')	/* pipe character at end */
	return ajFileNewInPipe(tmpname);
    if (ajStrChar(tmpname, 0) == '~')
    {
	ajDebug("starts with '~'\n");
	if (!userexp) userexp = ajRegCompC("^~([^/]*)");
	(void) ajRegExec(userexp, tmpname);
	ajRegSubI (userexp, 1, &userstr);
	(void) ajRegPost (userexp, &reststr);
	ajDebug("  user: '%S' rest: '%S'\n", userstr, reststr);
	if (ajStrLen(userstr))
	{
	    pass = getpwnam(ajStrStr(userstr)); /* username specified */
	    if (!pass) return NULL;
	    (void) ajFmtPrintS(&tmpname, "%s%S", pass->pw_dir, reststr);
	    ajDebug("use getpwnam: '%S'\n", tmpname);
	}
	else
	{				/* just ~/ */
	    if((p=getenv("HOME")))
		(void) ajFmtPrintS(&tmpname, "%s%S", p, reststr);
	    else
		(void) ajFmtPrintS(&tmpname,"%S",reststr);
	    ajDebug("use HOME: '%S'\n", tmpname);
	}
    }
    
    if (!wildexp) wildexp = ajRegCompC("(.*/)?([^/]*[*?][^/]*)$");

    if (ajRegExec(wildexp, tmpname))
    {					/* wildcard file names */
	(void) ajRegSubI(wildexp, 1, &dirname);
	(void) ajRegSubI(wildexp, 2, &wildname);
	ajDebug("wild dir '%S' files '%S'\n", dirname, wildname);
	return ajFileNewDW(dirname, wildname);
    }

    AJNEW0(thys);
    ajStrAssS(&thys->Name, tmpname);
    (void) ajNamResolve(&thys->Name);
    thys->fp = fopen (ajStrStr(thys->Name), "r");
    if (!thys->fp)
    {
	ajStrDel(&thys->Name);
	AJFREE (thys);
	/*    thys->Handle = 0;*/
	return NULL;
    }
    thys->Handle = ++fileHandle;
    thys->List = NULL;
    thys->End = ajFalse;

    fileOpenCnt++;
    fileOpenTot++;
    if (fileOpenCnt > fileOpenMax)
	fileOpenMax = fileOpenCnt;

    return thys;
}

/* @func ajFileNewInList ******************************************************
**
** Creates a new file object with a list of input files.
**
** @param [r] list [const AjPList] List of input filenames as strings.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewInList (const AjPList list) {

  AjPFile thys;

  AJNEW0(thys);

  thys->List = list;
  thys->Name = NULL;
  ajListstrTrace(thys->List);
  (void) ajListstrPop(thys->List, &thys->Name);
  ajDebug ("ajFileNewInList pop '%S'\n", thys->Name);
  ajListstrTrace(thys->List);
  (void) ajNamResolve(&thys->Name);
  thys->fp = fopen (ajStrStr(thys->Name), "r");
  if (!thys->fp) {
    ajDebug ("ajFileNewInList fopen failed\n");
    thys->Handle = 0;
    return NULL;
  }
  thys->Handle = ++fileHandle;
  thys->End = ajFalse;

  fileOpenCnt++;
  fileOpenTot++;
  if (fileOpenCnt > fileOpenMax)
    fileOpenMax = fileOpenCnt;

  return thys;
}

/* @func ajFileNewApp *******************************************************
**
** Creates an output file object with a specified name.
** The file is opened for append so it either appends to an existing file
** or opens a new one.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewApp (const AjPStr name) {

  AjPFile thys;

  AJNEW0(thys);
  thys->fp = fopen (ajStrStr(name), "a");
  if (!thys->fp) {
    thys->Handle = 0;
    return NULL;
  }
  thys->Handle = ++fileHandle;
  thys->Name = ajStrDup(name);
  thys->End = ajFalse;

  fileOpenCnt++;
  fileOpenTot++;
  if (fileOpenCnt > fileOpenMax)
    fileOpenMax = fileOpenCnt;

  return thys;
}

/* @func ajFileNewOut *******************************************************
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

AjPFile ajFileNewOut (const AjPStr name) {

  AjPFile thys;

  if (ajStrMatchC(name, "stdout"))
    return ajFileNewF(stdout);
  if (ajStrMatchC(name, "stderr"))
    return ajFileNewF(stderr);

  AJNEW0(thys);
  thys->fp = fopen (ajStrStr(name), "w");
  if (!thys->fp) {
    thys->Handle = 0;
    return NULL;
  }
  thys->Handle = ++fileHandle;
  thys->Name = ajStrDup(name);
  thys->End = ajFalse;

  fileOpenCnt++;
  fileOpenTot++;
  if (fileOpenCnt > fileOpenMax)
    fileOpenMax = fileOpenCnt;

  return thys;
}
/* @func ajFileNewOutD ******************************************************
**
** Creates a new output file object with a specified directory and name.
**
** 'stdout' and 'stderr' are special names for standard output and
** standard error respectively.
**
** @param [rN] dir [const AjPStr] Directory (optional, can be empty or NULL).
** @param [r] name [const AjPStr] File name.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewOutD (const AjPStr dir, const AjPStr name) {

  AjPFile thys;
  static AjPStr dirfix = NULL;

  if (ajStrMatchC(name, "stdout"))
    return ajFileNewF(stdout);
  if (ajStrMatchC(name, "stderr"))
    return ajFileNewF(stderr);

  AJNEW0(thys);

  if (!ajStrLen(dir)) {

    thys->fp = fopen (ajStrStr(name), "w");
  }
  else {
    ajStrAssS(&dirfix, dir);
    if (ajStrChar(dir, -1) != '/')
      ajStrAppC (&dirfix, "/");
    ajStrApp (&dirfix, name);
    thys->fp = fopen (ajStrStr(dirfix), "w");
  }

  if (!thys->fp) {
    thys->Handle = 0;
    return NULL;
  }

  thys->Handle = ++fileHandle;
  thys->Name = ajStrDup(name);
  thys->End = ajFalse;

  fileOpenCnt++;
  fileOpenTot++;
  if (fileOpenCnt > fileOpenMax)
    fileOpenMax = fileOpenCnt;

  return thys;
}

/* @func ajFileNewF *******************************************************
**
** Creates a new file object from an open C file.
**
** @param [r] file [FILE*] C file.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewF (FILE* file) {

  AjPFile thys;

  if (!file) {
    ajFatal ("Trying to create an AJAX file from a bad C RTL FILE*");
  }

  AJNEW0(thys);
  thys->fp = file;
  thys->Handle = ++fileHandle;
  thys->Name = ajStrNew();
  thys->End = ajFalse;

  fileOpenCnt++;
  fileOpenTot++;
  if (fileOpenCnt > fileOpenMax)
    fileOpenMax = fileOpenCnt;

  return thys;
}


/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section File Destructors ********************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/

/* @func ajFileClose *******************************************************
**
** Close and free a file object.
**
** @param [w] pthis [AjPFile*] File.
** @return [void]
** @@
******************************************************************************/

void ajFileClose (AjPFile* pthis) {

  AjPFile thys = pthis ? *pthis : 0;

  if (!pthis) return;
  if (!*pthis) return;

  fileClose(thys);

  AJFREE (*pthis);

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

void ajFileOutClose (AjPFile* pthis) {

  AjPFile thys = pthis ? *pthis : 0;

  (void) ajFmtPrintF (thys, "Standard output close ...\n");
  ajFileClose (pthis);

  return;
}

/* @funcstatic fileClose ******************************************************
**
** Closes a file object. Used as part of the public destructor and
** other public functions.
**
** @param [w] thys [const AjPFile] File.
** @return [void]
** @@
******************************************************************************/

static void fileClose (const AjPFile thys) {

  if (!thys) return;

  if (thys->Handle) {
    ajDebug ("closing file '%F'\n", thys);
    if(fclose (thys->fp))
      ajFatal("File close in fileClose");
    ajStrDel (&thys->Name);
    ajStrDel (&thys->Buff);
    ajListstrFree (&thys->List);
    thys->Handle = 0;

    fileCloseCnt++;
    fileOpenCnt--;
  }
  else {
    ajDebug ("file already closed\n");
  }

  return;
}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section File Assignments ************************************************
**
** These functions overwrite the file provided as the first argument
**
******************************************************************************/

/* @func ajFileDataNew *******************************************************
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
    AjPStr hname = NULL;
    static AjPStr pname = NULL;
    char *p;
    
    if(tfile == NULL) return;

    (void) ajStrAss(&bname, tfile);
    ajDebug ("ajFileDataNew trying '%S'\n", fname);
    if(ajFileStat(&bname, AJ_FILE_R))
    {
	*fnew = ajFileNewIn(bname);
	ajStrDelReuse(&bname);
	return;
    }

    (void) ajStrAssC(&fname, ".embossdata/");
    (void) ajStrApp(&fname, bname);
    ajDebug ("ajFileDataNew trying '%S'\n", fname);
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
	(void) ajStrAssC(&hname,p);
	(void) ajStrAppC(&hname,"/");
	(void) ajStrApp(&hname,bname);
	ajDebug ("ajFileDataNew trying '%S'\n", hname);
	if(ajFileStat(&hname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(hname);
	    ajStrDel(&hname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
    
	(void) ajStrAssC(&hname,p);
	(void) ajStrAppC(&hname,"/.embossdata/");
	(void) ajStrApp(&hname,bname);
	ajDebug ("ajFileDataNew trying '%S'\n", hname);
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
    
        (void) ajFileDirFix(&fname);
	(void) ajStrApp(&fname,bname);
	ajDebug ("ajFileDataNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    if(ajNamRootInstall(&fname)) /* just EMBOSS/data under installation */
    {
        (void) ajNamRootPack(&pname);	/* just EMBOSS */
	(void) ajFileDirFix(&fname);
	(void) ajStrAppC(&fname,"share/");
	(void) ajStrApp(&fname,pname);
	(void) ajStrAppC(&fname,"/data/");
	(void) ajStrApp(&fname,bname);
	ajDebug ("ajFileDataNew trying '%S'\n", fname);
	if(ajFileStat(&fname, AJ_FILE_R))
	{
	    *fnew = ajFileNewIn(fname);
	    ajStrDelReuse(&bname);
	    ajStrDelReuse(&fname);
	    return;
	}
    }
    
    if(ajNamRoot(&fname))	/* just emboss/data under source */
    {
    
	(void) ajStrAppC(&fname,"/data/");
	(void) ajStrApp(&fname,bname);
	ajDebug ("ajFileDataNew trying '%S'\n", fname);
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

    *fnew = NULL;

    return;
}

/* @func ajFileDataNewC ******************************************************
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

    t=ajStrNewC(s);
    ajFileDataNew(t,f);
    ajStrDel(&t);
    
    return;
}


/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section File Modifiers **************************************************
**
** These functions use the contents of a file object and update them.
**
******************************************************************************/

/* @func ajFileSeek *******************************************************
**
** Sets the current position in an open file.
**
** @param [r] thys [const AjPFile] File.
** @param [r] offset [long] Offset
** @param [r] wherefrom [int] Start of offset, as defined for 'fseek'.
** @return [int] Result of 'fseek'
** @@
******************************************************************************/

int ajFileSeek (const AjPFile thys, long offset, int wherefrom) {
  return fseek (thys->fp, offset, wherefrom);
}

/* @func ajFileRead *******************************************************
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

size_t ajFileRead (void* ptr, size_t element_size, size_t count,
		   const AjPFile thys) {
  return fread (ptr, element_size, count, thys->fp);
}

/* @func ajFileReadUint *******************************************************
**
** Binary read of an unsigned integer from an input file object using
** the C 'fread' function. Converts from a specified endianism.
**
** @param [r] thys [const AjPFile] Input file.
** @param [r] Bigendian [AjBool] Big endian or not.
** @return [unsigned int] Converted integer value
** @@
******************************************************************************/

unsigned int ajFileReadUint (const AjPFile thys, AjBool Bigendian) {

  static int called = 0;
  static AjBool bigend = AJFALSE;
  unsigned int ret;
  int ret2;

  if (!called)
    bigend = ajUtilBigendian();

  fread (&ret, 4, 1, thys->fp);
  if (Bigendian && bigend) {
    return ret;
  }
  else if (!Bigendian && !bigend) {
    return ret;
  }

  /*ajDebug ("Reversed: %u", ret);*/
  ret2 = (int) ret;
  ajUtilRev4(&ret2);
  ret = (unsigned int) ret2;
  /*ajDebug (" => %u\n", ret);*/
  return ret;
}

/* @func ajFileWrite *******************************************************
**
** Binary write to an output file object using the C 'fwrite' function.
**
** @param [w] ptr [const void*] Buffer for output.
** @param [r] element_size [size_t] Number of bytes per element.
** @param [r] count [size_t] Number of elements to write.
** @param [r] thys [const AjPFile] Output file.
** @return [size_t] Return value from 'fwrite'
** @@
******************************************************************************/

size_t ajFileWrite (const void* ptr, size_t element_size, size_t count,
		   const AjPFile thys) {
  return fwrite (ptr, element_size, count, thys->fp);
}

/* @func ajFileNext *******************************************************
**
** Given a file object that includes a list of input files, closes the
** current input file and opens the next one.
**
** @param [r] thys [const AjPFile] File object.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileNext (const AjPFile thys) {

  static AjPStr name = NULL;

  if (!thys->List) {
    ajDebug ("ajFileNext for non-list file %F\n", thys);
    return ajFalse;
  }

  ajListTrace(thys->List);
  if (!ajListPop (thys->List, (void*) &name)) { /* end of list */
    /*    ajStrDel (&thys->Name);
	  if(fclose(thys->fp))
	  ajFatal("fclose in ajFileNext");*/
    ajDebug("ajFileNext failed - list completed\n");
    return ajFalse;
  }

  ajDebug("ajFileNext filename '%S'\n", name);
  if (!ajFileReopen (thys, name))
    return ajFalse;

  thys->End = ajFalse;

  ajDebug("ajFileNext success\n");
  return ajTrue;
}

/* @func ajFileReopen *******************************************************
**
** Reopens a file with a new name.
**
** @param [r] thys [const AjPFile] Input file.
** @param [r] name [AjPStr] name of file.
** @return [FILE*] copy of file pointer
** @@
******************************************************************************/

FILE* ajFileReopen (const AjPFile thys, AjPStr name) {

  ajStrAssS (&thys->Name, name);
  return freopen (ajStrStr(thys->Name), "r", thys->fp);
}

/* @func ajFileReadLine *******************************************************
**
** Reads a line from the input file, removing any trailing newline.
**
** @param [r] thys [const AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReadLine (const AjPFile thys, AjPStr* pdest) {

  return ajFileGetsTrim (thys, pdest);
}

/* @func ajFileGetsTrimL ******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [r] thys [const AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @param [w] fpos [long*] File position before the read.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetsTrimL (const AjPFile thys, AjPStr* pdest, long* fpos) {
  AjBool ok;
  AjPStr dest = *pdest;

  ok = ajFileGetsL (thys, pdest, fpos);

  if (!ok)
    return ajFalse;

  /* trim any trailing newline */

  dest = *pdest;
  if (dest->Ptr[dest->Len-1] == '\n')
     dest->Ptr[--dest->Len] = '\0';

  return ajTrue;
}

/* @func ajFileGetsTrim *******************************************************
**
** Reads a line from a file and removes any trailing newline.
**
** @param [r] thys [const AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetsTrim (const AjPFile thys, AjPStr* pdest) {
  AjBool ok;
  AjPStr dest = *pdest;

  ok = ajFileGets (thys, pdest);

  if (!ok)
    return ajFalse;

  /* trim any trailing newline */

  dest = *pdest;
  if (dest->Ptr[dest->Len-1] == '\n')
     dest->Ptr[--dest->Len] = '\0';

  return ajTrue;
}

/* @func ajFileGets *******************************************************
**
** Reads a line from a file and returns the initial file position.
**
** @param [r] thys [const AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGets (const AjPFile thys, AjPStr* pdest) {

  long fpos = 0;

  return ajFileGetsL (thys, pdest, &fpos);
}

/* @func ajFileGetsL *******************************************************
**
** Reads a line from a file.
**
** @param [r] thys [const AjPFile] Input file.
** @param [w] pdest [AjPStr*] Buffer to hold the current line.
** @param [w] fpos [long*] File position before the read.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetsL (const AjPFile thys, AjPStr* pdest, long* fpos) {

  char *cp;
  char *buff;
  int isize;
  int ilen;
  int jlen;
  int ipos;

  (void) ajStrModL (&thys->Buff, fileBuffSize);
  buff = ajStrStr(thys->Buff);
  isize = ajStrSize(thys->Buff);
  ilen = 0;
  ipos = 0;

  if (!thys->fp)
    ajWarn("ajFileGets file not found");
  while (buff) {
    *fpos = ajFileTell (thys);
    cp = fgets (&buff[ipos], isize, thys->fp);

    if (!cp) {
      if (feof(thys->fp)) {
	thys->End = ajTrue;
	(void) ajStrAssC(pdest, "");
	ajDebug("end of file %F\n", thys);
	return ajFalse;
      }
      else
	ajFatal ("Error reading from file '%s'\n", ajFileName(thys));

    }
    jlen = strlen(&buff[ipos]);
    ilen += jlen;
    if (jlen == (isize-1)) {
      ajDebug("more to do: jlen: %d ipos: %d isize: %d ilen: %d Size: %d\n",
	      jlen, ipos, isize, ilen, ajStrSize(thys->Buff));
      ajStrFixI (thys->Buff, ilen);
      (void) ajStrModL(&thys->Buff, ajStrSize(thys->Buff)+fileBuffSize);
      ipos += jlen;
      buff = ajStrStr(thys->Buff);
      isize = ajStrSize(thys->Buff) - ipos;
      ajDebug("expand to: ipos: %d isize: %d Size: %d\n",
	      ipos, isize, ajStrSize(thys->Buff));
    }
    else
      buff = NULL;
  }

  ajStrFixI (thys->Buff, ilen);
  (void) ajStrAssS (pdest, thys->Buff);

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

void ajFileUnbuffer (const AjPFile thys) {

  setbuf (thys->fp, NULL);

  return;
}

/* @func ajFileReadAppend *****************************************************
**
** Reads a record from a file and appends it to the user supplied buffer.
**
** @param [r] thys [const AjPFile] Input file.
** @param [w] pbuff [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileReadAppend (const AjPFile thys, AjPStr* pbuff) {

  static AjPStr locbuff = 0;
  AjBool ok;

  if (!locbuff)
    locbuff = ajStrNew();

  ok = ajFileGets (thys, &locbuff);

  if (ok)
    (void) ajStrApp (pbuff, locbuff);

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

void ajFileOutHeader (const AjPFile thys) {

  (void) ajFmtPrintF (thys, "Standard output header ...\n");

  return;
}

/* @func ajFileNameShorten **********************************************
**
** Truncates a filename to a basic file name.
**
** @param [uP] fname [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFileNameShorten (AjPStr* fname) {

  static AjPStr tmpstr = NULL;

  static AjPRegexp entryexp = NULL;
  static AjPRegexp fileexp = NULL;
  static AjPRegexp restexp = NULL;

  if (!entryexp)		/* entryname at end */
    entryexp = ajRegCompC(":([A-Za-z0-9_-]+)$");

  if (ajRegExec(entryexp, *fname)) {
    ajRegSubI(entryexp, 1, &tmpstr);
    (void) ajStrAssS (fname, tmpstr);
    return ajTrue;
  }

  if (!fileexp)	/* name.ext */
    fileexp = ajRegCompC("([A-Za-z0-9_-]+)[.][A-Za-z0-9_-]+$");

  if (ajRegExec(fileexp, *fname)) {
    ajRegSubI(fileexp, 1, &tmpstr);
    (void) ajStrAssS (fname, tmpstr);
    return ajTrue;
  }

  if (!restexp)			/* last valid word */
    restexp = ajRegCompC("([A-Za-z0-9_-]+)[^A-Za-z0-9_-]*$");

  if (ajRegExec(restexp, *fname)) {
    ajRegSubI(restexp, 1, &tmpstr);
    (void) ajStrAssS (fname, tmpstr);
    return ajTrue;
  }

  (void) ajStrAssC (fname, "");

  return ajFalse;
}

/* @func ajFileNameTrim **********************************************
**
** Truncates a filename to a basic file name.extension
**
** @param [uP] fname [AjPStr*] File name
** @return [AjBool] ajTrue on success, and returns a filename.
**                  ajFalse on failure, and returns an empty string.
** @@
******************************************************************************/

AjBool ajFileNameTrim (AjPStr* fname) {

  static AjPStr tmpstr = NULL;
  char *p;
  
/* Replaced by AJB with routine below. Aug 2000
  static AjPRegexp fileexp = NULL;

  if (!fileexp)
    fileexp = ajRegCompC("([A-Za-z0-9_-]+[.][A-Za-z0-9_-]+)$");

  if (ajRegExec(fileexp, *fname)) {
    ajRegSubI(fileexp, 1, &tmpstr);
    (void) ajStrAssS (fname, tmpstr);
    return ajTrue;
  }

  (void) ajStrAssC (fname, "");

  return ajFalse;
*/

  if((p=strrchr(ajStrStr(*fname),(int)'/')))
  {
      (void) ajStrAssC(&tmpstr,p+1);
      (void) ajStrAssS(fname,tmpstr);
  }

  return ajTrue;
}

/* @func ajFileDataNewWrite **************************************************
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

    if(tfile == NULL) return;

    fname=ajStrNew();

    if(ajNamGetValueC("DATA", &fname))
    {
    
	(void) ajStrAppC(&fname,"/");
	(void) ajStrApp(&fname,tfile);
	if(!(*fnew = ajFileNewOut(fname)))
	    ajFatal("Cannot write to file %S\n",fname);
	ajStrDel(&fname);
	return;

    }

    if(ajNamRoot(&fname))	/* just emboss/data under installation */
    {
    
	(void) ajStrAppC(&fname,"/data/");
	(void) ajStrApp(&fname,tfile);
	if(!(*fnew = ajFileNewOut(fname)))
	    ajFatal("Cannot write to file %s\n",ajStrStr(fname));
	ajStrDel(&fname);
	return;

    }

    else
	ajFatal("Environment variable EMBOSS_DATA not defined\n");
    
    ajStrDelReuse(&fname);
    *fnew = NULL;

    return;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section File Operators ********************************************
**
** These functions use the contents of a file object but do not make
** any changes.
**
******************************************************************************/

/* @func ajFileDir ***********************************************************
**
** Checks that a string is a valid directory, and appends a trailing '/'
** if it is missing.
**
** @param [u] dir [AjPStr*] Directory path
** @return [AjBool] true if a valid directory.
** @@
******************************************************************************/

AjBool ajFileDir (AjPStr* dir) {
  DIR* odir;

  odir = fileOpenDir (dir);	/* appends trailing slash if needed */
  if (!odir)
    return ajFalse;

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

AjBool ajFileDirPath (AjPStr* dir) {

  DIR* odir;
  static AjPStr cwd = NULL;

  ajDebug ("ajFileDirPath '%S'\n", *dir);

  odir = fileOpenDir (dir);	/* appends trailing slash if needed */
  if (!odir)
    return ajFalse;
  free (odir);

  ajDebug ("So far '%S'\n", *dir);

  if (*ajStrStr(*dir) == '/')	/* full path already */
    return ajTrue;

  ajFileGetwd(&cwd);

  if (ajStrMatchC(*dir, "./")) {	/* current directory */
    ajStrAssS (dir, cwd);
    ajDebug ("Current '%S'\n", *dir);
    return ajTrue;
  }
  while (ajStrPrefixC(*dir, "../")) { /*  going up */
    ajFileDirUp (&cwd);
    ajStrSub (dir, 3, -1);
    ajDebug ("Going up '%S' '%S'\n", *dir, cwd);
  }

  ajStrInsert (dir, 0, cwd);

  ajDebug ("Full path '%S'\n", *dir);

  return ajTrue;
}


/* @func ajFileGetwd *********************************************************
**
** Returns the current directory
**
** @param [w] dir [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileGetwd (AjPStr* dir) {

  char cwd[PATH_MAX+1];

  if (!getcwd(cwd,PATH_MAX)) {
    ajStrDelReuse(dir);
    return ajFalse;
  }
  if (ajStrSuffixCC(cwd, "/"))
    ajStrAssC (dir, cwd);
  else
    ajFmtPrintS (dir, "%s/", cwd);

  return ajTrue;
}

/* @func ajFileDirUp ******************************************************
**
** Changes directory name to one level up
**
** @param [u] dir [AjPStr*] Directory name.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajFileDirUp (AjPStr* dir) {

  static AjPRegexp direxp = NULL;
  static AjPStr tmpdir = NULL;

  if (!direxp)
    direxp = ajRegCompC ("^(.*/)[^/]+/?$");

  ajStrAssS (&tmpdir, *dir);
  if (!ajRegExec (direxp, tmpdir)) /* no match to pattern */
    return ajFalse;

  ajRegSubI (direxp, 1, dir);
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

static DIR* fileOpenDir (AjPStr* dir) {
  if (ajStrChar(*dir, -1) != '/')
    (void) ajStrAppC (dir, "/");
  return opendir(ajStrStr(*dir));
}

/* @func ajFileDirFix ****************************************************
**
** If the directory name
** has no trailing slash (on Unix) then one is added. This is why the
** directory name must be writeable.
**
** @param [u] dir [AjPStr*] Directory name.
** @return [void]
** @@
******************************************************************************/

void ajFileDirFix (AjPStr* dir) {
  if (ajStrChar(*dir, -1) != '/')
    (void) ajStrAppC (dir, "/");
  return;
}

/* @func ajFileExit ***********************************************************
**
** Prints a summary of file usage with debug calls
**
** @return [void]
** @@
******************************************************************************/

void ajFileExit (void) {

  ajDebug ("File usage : %d opened, %d closed, %d max, %d total\n",
	   fileOpenCnt, fileCloseCnt, fileOpenMax, fileOpenTot);

  return;
}


/* @func ajFileTrace *******************************************************
**
** Writes debug messages to trace the contents of a file object.
**
** @param [r] thys [const AjPFile] File.
** @return [void]
** @@
******************************************************************************/

void ajFileTrace (const AjPFile thys) {
  ajDebug ("File: '%S'\n", thys->Name);
  ajDebug ("  handle:  %d\n", thys->Handle);

  return;
}

/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section File Casts ********************************************
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
** @return [int] File record buffer size
** @@
******************************************************************************/

int ajFileBuffSize (void) {
  return fileBuffSize;
}

/* @func ajFileName *******************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [const char*] Filename as a C character string.
** @@
******************************************************************************/

const char* ajFileName (const AjPFile thys) {
  return ajStrStr(thys->Name);
}

/* @func ajFileGetName *******************************************************
**
** Returns the file name for a file object. The filename returned is a pointer
** to the real string internally, so the user must take care not to change
** it and cannot trust the value if the file object is deleted.
**
** @param [r] thys [const AjPFile] File.
** @return [AjPStr] Filename as a C character string.
** @@
******************************************************************************/

AjPStr ajFileGetName (const AjPFile thys) {
  return thys->Name;
}

/* @func ajFileStat ***********************************************************
**
** Returns true if file exists and is read or write or executable by the user
** as determined by AJ_FILE_R AJ_FILE_W AJ_FILE_X file modes
**
** @param [r] fname [AjPStr*] Filename.
** @param [r] mode [int] file mode.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFileStat(AjPStr *fname, int mode)
{
    struct stat buf;

    if(!stat(ajStrStr(*fname), &buf))
	if((unsigned int)buf.st_mode & mode)
	    return ajTrue;

    return ajFalse;
}

/* @func ajFileTell *******************************************************
**
** Returns the current position in an open file.
**
** @param [r] thys [const AjPFile] File.
** @return [long] Result of 'ftell'
** @@
******************************************************************************/

long ajFileTell (const AjPFile thys)
{
  if (!thys->fp)
    return 0;
  return ftell(thys->fp);
}

/* @func ajFileStdout *******************************************************
**
** Tests whether a file object is really stdout.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdout.
** @@
******************************************************************************/

AjBool ajFileStdout (const AjPFile file) {
  if (file->fp == stdout)
    return ajTrue;

  return ajFalse;
}

/* @func ajFileStderr *******************************************************
**
** Tests whether a file object is really stderr.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stderr.
** @@
******************************************************************************/

AjBool ajFileStderr (const AjPFile file) {
  if (file->fp == stderr)
    return ajTrue;

  return ajFalse;
}

/* @func ajFileStdin *******************************************************
**
** Tests whether a file object is really stdin.
**
** @param [r] file [const AjPFile] File object.
** @return [AjBool] ajTrue if the file matches stdin.
** @@
******************************************************************************/

AjBool ajFileStdin (const AjPFile file) {
  if (file->fp == stdin)
    return ajTrue;

  return ajFalse;
}

/* @func ajFileFp *******************************************************
**
** Returns the C file pointer for an open file.
**
** @param [r] thys [const AjPFile] File.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFileFp (const AjPFile thys) {
  return thys->fp;
}

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Buffered File Constructors ***************************************
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
** Creates a new buffered input file object.
**
** @param [r] name [const AjPStr] File name.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewIn (const AjPStr name) {

  AjPFileBuff thys;

  AJNEW0(thys);
  thys->File = ajFileNewIn (name);
  if (!thys->File) {
      AJFREE (thys);
      return NULL;
  }

  thys->Last = thys->Curr = thys->Lines = thys->Free = NULL;
  thys->Pos = thys->Size = 0;

  return thys;
}

/* @func ajFileBuffNew *****************************************************
**
** Creates a new buffered input file object from a filename.
**
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNew (void) {

  AjPFileBuff thys;

  AJNEW0(thys);
  thys->File = ajFileNew();
  if (!thys->File) {
    return NULL;
  }

  thys->Last = thys->Curr = thys->Lines = thys->Free = NULL;
  thys->Pos = thys->Size = 0;

  return thys;
}

/* @func ajFileBuffNewFile ****************************************************
**
** Creates a new buffered input file object from an open file.
**
** @param [r] file [AjPFile] File object to be buffered.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewFile (AjPFile file) {

  AjPFileBuff thys;

  if (!file) {
    return NULL;
  }
  AJNEW0(thys);
  thys->File = file;

  thys->Last = thys->Curr = thys->Lines = thys->Free = NULL;
  thys->Pos = thys->Size = 0;

  return thys;
}

/* @func ajFileBuffSetFile ****************************************************
**
** Creates a new buffered input file object from an open file.
**
** @param [w] pthys [AjPFileBuff*] Buffered file object.
** @param [r] file [AjPFile] File object to be buffered.
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajFileBuffSetFile (AjPFileBuff* pthys, AjPFile file) {

  AjPFileBuff thys;

  if (!file) {
    ajFileBuffDel (pthys);
    return ajFalse;
  }

  if (!*pthys) {
    AJNEW0(*pthys);
  }
  thys = *pthys;

  if (thys->File && (thys->File->Handle ==  file->Handle)) /* same file */
    return ajTrue;

  ajFileClose (&thys->File);
  thys->File = file;

  thys->Last = thys->Curr = thys->Lines = thys->Free = NULL;
  thys->Pos = thys->Size = 0;

  return ajTrue;
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

AjPFileBuff ajFileBuffNewS (const AjPStr data) {

  AjPFileBuff thys;

  AJNEW0(thys);
  AJNEW0(thys->File);
  thys->File->End = ajTrue;

  thys->Lines = AJNEW0(thys->Last);
  (void) ajStrAssS(&thys->Last->Line,data);

  thys->Last->Next = NULL;
  thys->Curr = thys->Lines;
  thys->Free = 0;
  thys->Pos = 0;
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

AjPFileBuff ajFileBuffNewF (FILE* fp) {

  AjPFileBuff thys;

  AJNEW0(thys);
  thys->File = ajFileNewF (fp);
  if (!thys->File) {
    return NULL;
  }

  thys->Last = thys->Curr = thys->Lines = thys->Free = NULL;
  thys->Pos = thys->Size = 0;

  return thys;
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

AjPFileBuff ajFileBuffNewDW (const AjPStr dir, const AjPStr wildfile) {

  DIR* dp;
  struct dirent* de;
  int dirsize;
  AjPList list = NULL;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;

  if (ajStrLen(dir))
    (void) ajStrAssS (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dir, -1) != '/')
    ajStrAppC (&dirfix, "/");

  dp = fileOpenDir (&dirfix);
  if (!dp)
    return NULL;

  dirsize = 0;
  list = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    if (!ajStrMatchWildCO(de->d_name, wildfile)) continue;
    dirsize++;
    ajDebug ("accept '%s'\n", de->d_name);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%s", dirfix, de->d_name);
    ajListstrPushApp (list, name);
  }

  (void) closedir (dp);
  ajDebug ("%d files for '%S' '%S'\n", dirsize, dir, wildfile);

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

AjPFileBuff ajFileBuffNewDC (const AjPStr dir, const char* filename) {

  static AjPStr namefix = NULL;

  if (ajStrLen(dir))
    (void) ajStrAssS (&namefix, dir);
  else
    (void) ajStrAssC (&namefix, "./");

  if (ajStrChar(namefix, -1) != '/')
      ajStrAppC (&namefix, "/");

  ajStrAppC (&namefix, filename);

  return ajFileBuffNewIn (namefix);
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

AjPFileBuff ajFileBuffNewDF (const AjPStr dir, const AjPStr filename) {

  static AjPStr namefix = NULL;

  if (ajStrLen(dir))
    (void) ajStrAssS (&namefix, dir);
  else
    (void) ajStrAssC (&namefix, "./");

  if (ajStrChar(namefix, -1) != '/')
      ajStrAppC (&namefix, "/");

  ajStrApp (&namefix, filename);

  return ajFileBuffNewIn (namefix);
}

/* @func ajFileNewDW ******************************************************
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

AjPFile ajFileNewDW (const AjPStr dir, const AjPStr wildfile) {

  DIR* dp;
  struct dirent* de;
  int dirsize;
  AjPList list = NULL;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;

  if (ajStrLen(dir))
    (void) ajStrAssS (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dir, -1) != '/')
    ajStrAppC (&dirfix, "/");

  dp = fileOpenDir (&dirfix);
  if (!dp)
    return NULL;

  dirsize = 0;
  list = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    if (!ajStrMatchWildCO(de->d_name, wildfile)) continue;
    dirsize++;
    ajDebug ("accept '%s'\n", de->d_name);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%s", dirfix, de->d_name);
    ajListstrPushApp (list, name);
  }

  (void) closedir (dp);
  ajDebug ("%d files for '%S' '%S'\n", dirsize, dir, wildfile);

  return ajFileNewInList(list);
}

/* @func ajFileNewDF ******************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const AjPStr] Wildcard Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDF (const AjPStr dir, const AjPStr filename) {

  static AjPStr namefix = NULL;

  if (ajStrLen(dir))
    (void) ajStrAssS (&namefix, dir);
  else
    (void) ajStrAssC (&namefix, "./");

  if (ajStrChar(namefix, -1) != '/')
      ajStrAppC (&namefix, "/");

  (void) ajStrApp (&namefix, filename);

  return ajFileNewIn(namefix);
}

/* @func ajFileNewDC ******************************************************
**
** Opens directory "dir"
** Looks for file "file"
**
** @param [r] dir [const AjPStr] Directory
** @param [r] filename [const char*] Filename.
** @return [AjPFile] New file object.
** @@
******************************************************************************/

AjPFile ajFileNewDC (const AjPStr dir, const char* filename) {

  static AjPStr namefix = NULL;

  if (ajStrLen(dir))
    (void) ajStrAssS (&namefix, dir);
  else
    (void) ajStrAssC (&namefix, "./");

  if (ajStrChar(namefix, -1) != '/')
      ajStrAppC (&namefix, "/");

  (void) ajStrAppC (&namefix, filename);

  return ajFileNewIn(namefix);
}

/* @func ajFileBuffNewInList **************************************************
**
** Creates a new buffered file object from a list of filenames.
**
** @param [r] list [const AjPList] List of filenames as strings.
** @return [AjPFileBuff] New buffered file object.
** @@
******************************************************************************/

AjPFileBuff ajFileBuffNewInList (const AjPList list) {

  AjPFileBuff thys;

  AJNEW0(thys);
  thys->File = ajFileNewInList (list);
  if (!thys->File) {
    return NULL;
  }
  thys->Last = thys->Curr = thys->Lines = thys->Free = NULL;
  thys->Pos = thys->Size = 0;

  return thys;
}

/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */

/* @section Buffered File Destructors ****************************************
**
** Destruction is achieved by closing the file.
**
** Unlike ANSI C, there are tests to ensure a file is not closed twice.
**
******************************************************************************/

/* @func ajFileBuffDel *******************************************************
**
** Destructor for a buffered file object.
**
** @param [w] pthis [AjPFileBuff*] Buffered file object.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffDel (AjPFileBuff* pthis) {

  AjPFileBuff thys;

  if (!pthis)
    return;

  if (!*pthis)
    return;

  thys = *pthis;

  ajDebug("ajFileBuffDel '%F'\n", thys->File);

  ajFileBuffClear (thys, -1);
  ajFileBuffFreeClear (thys);
  ajFileClose(&thys->File);
  AJFREE(*pthis);

  return;
}

/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Buffered File Assignments ****************************************
**
** These functions overwrite the file provided as the first argument
**
******************************************************************************/


/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section BufferedFile Modifiers ******************************************
**
** These functions use the contents of a file object and update them.
**
******************************************************************************/

/* @func ajFileBuffGet *******************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [r] thys [const AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGet (const AjPFileBuff thys, AjPStr* pdest) {
  long fpos = 0;

  return ajFileBuffGetL (thys, pdest, &fpos);
}

/* @func ajFileBuffGetL *******************************************************
**
** Reads a line from a buffered file. If the buffer has data, reads from the
** buffer. If the buffer is exhausted, reads from the file. If the file is
** exhausted, sets end of file and returns. If end of file was already set,
** looks for another file to open.
**
** @param [r] thys [const AjPFileBuff] Buffered input file.
** @param [w] pdest [AjPStr*] Buffer to hold results.
** @param [w] fpos [long*] File position before the read.
** @return [AjBool] ajTrue if data was read.
** @@
******************************************************************************/

AjBool ajFileBuffGetL (const AjPFileBuff thys, AjPStr* pdest, long* fpos) {

  AjBool ok;

  /* read from the buffer if it is not empty */

  *fpos = 0;

  if (thys->Pos < thys->Size) {
    (void) ajStrAssS (pdest, thys->Curr->Line);
    *fpos = thys->Curr->Fpos;
    thys->Curr = thys->Curr->Next;
    thys->Pos++;
    /*ajDebug ("ajFileBuffGetL buffered fpos: %ld '%S'\n", *fpos, *pdest);*/
    return ajTrue;
  }

  if (!thys->File->Handle)	/* file has been closed */
    return ajFalse;

  /* buffer used up - try reading from the file */

  ok = ajFileGetsL (thys->File, pdest, &thys->Fpos);

  if (!ok) {
    if (thys->File->End) {
      if (thys->Size) {		/* we have data in the buffer - fail */
	ajDebug ("End of file - data in buffer - return ajFalse\n");
	return ajFalse;
      }
      else {			/* buffer clear - try another file */
	if (ajFileNext(thys->File)) { /* OK - read the new file */
	  ok = ajFileBuffGetL(thys, pdest, fpos);
	  ajDebug ("End of file - trying next file ok: %B fpos: %ld %ld\n",
		   ok, *fpos, thys->Fpos);
	  return ok;
	}
	else {			/* no new file, fail again */
	  ajDebug ("End of file - no new file to read - return ajFalse\n");
	  return ajFalse;
	}
      }
    }
    else
      ajFatal ("Error reading from file '%s'\n", ajFileName(thys->File));
  }

  if (thys->Nobuff) {
    *fpos = thys->Fpos;
    /*ajDebug ("ajFileBuffGetL unbuffered fpos: %ld\n", *fpos);*/
    return ajTrue;
  }

  if (thys->Free) {
    /*ajDebug("using Free %x %d\n",
      thys->Free->Line, ajStrSize(thys->Free->Line));*/
    if (!thys->Lines) {		/* Need to set first line in list */
      thys->Lines = thys->Free;
      /*ajDebug("using Free: set new first line\n");*/
    }
    else {
      thys->Last->Next = thys->Free;
    }
    thys->Last = thys->Free;
    thys->Free = thys->Free->Next;
    if (!thys->Free) {	 	/* Free list now empty */
      thys->Freelast = NULL;
      /*ajDebug("using Free: Free now empty\n");*/
    }
  }
  else {			/* No Free list, make a new string */
    if (!thys->Lines) {
      thys->Lines = AJNEW0(thys->Last);
      /*ajDebug("new first line\n");*/
    }
    else {
      thys->Last = AJNEW0(thys->Last->Next);
      /*Debug("new last line\n");*/
    }
  }

  (void) ajStrAssS (&thys->Last->Line,*pdest);
  thys->Curr = thys->Last;
  thys->Last->Next = NULL;
  thys->Last->Fpos = thys->Fpos;
  thys->Pos++;
  thys->Size++;

  /*if (usefree) ajFileBuffTrace (thys);*/
  *fpos = thys->Fpos;

  /*ajDebug ("ajFileBuffGetL new fpos: %ld %ld '%S'\n",
   *fpos, thys->Curr->Fpos, *pdest);*/
  return ajTrue;
}

/* @func ajFileBuffStripHtml **************************************************
**
** Processes data in the file buffer, removing HTML tokens between
** angle brackets, plus any TITLE. This seems to be enough to make HTML
** output readable.
**
** @param [r] thys [const AjPFileBuff] Buffered file with data loaded in the buffer.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffStripHtml (const AjPFileBuff thys) {
  AjPRegexp tagexp = NULL;
  AjPRegexp fullexp = NULL;
  AjPRegexp httpexp = NULL;
  AjPRegexp nullexp = NULL;
  AjPRegexp chunkexp = NULL;
  AjPRegexp ncbiexp = NULL;
  AjPRegexp ncbiexp2 = NULL;
  AjPFileBuffList plist;
  AjPFileBuffList pdellist;
  AjPFileBuffList plast = NULL;
  AjPStr s1 = NULL;
  AjPStr s2 = NULL;
  AjPStr s3 = NULL;
  int i;
  /*  AjBool dochunk = ajFalse;*/

  tagexp = ajRegCompC("^(.*)(<[!/A-Za-z][^>]*>)(.*)$");
  fullexp = ajRegCompC("^(.*)(<(TITLE)>.*</TITLE>)(.*)$");
  httpexp = ajRegCompC("^HTTP/");
  nullexp = ajRegCompC("^\r?\n");
  chunkexp = ajRegCompC("^Transfer-Excoding: +chunked");
  ncbiexp = ajRegCompC("^Entrez Reports\n$");
  ncbiexp2 = ajRegCompC("^----------------\n$");
  plist = thys->Curr;
  i = 0;

  /* first take out the HTTP header (HTTP 1.0 onwards) */

  ajDebug ("First line [%d] '%S' \n",
	   ajStrRef(plist->Line), plist->Line);
  if (ajRegExec(httpexp, plist->Line)) {
    while(plist && !ajRegExec(nullexp, plist->Line)) {
      ajDebug ("removing line [%d], '%S' len %d\n",
	       ajStrRef(plist->Line), plist->Line,
	       ajStrLen(plist->Line));
      ajRegExec(chunkexp, plist->Line);
      /*      if (ajRegExec(chunkexp, plist->Line))
	      dochunk = ajTrue;*/
      pdellist = plist;
      if (plast) {
	plast->Next = plist->Next;
	plist = plast->Next;
      }
      else {			/* we are on the first line */
	plist = thys->Lines = thys->Curr = plist->Next;
      }
      ajStrDel(&pdellist->Line);
      AJFREE (pdellist);
      thys->Size--;
      if (thys->Pos > i)
	thys->Pos--;
    }
  }

  while (plist) {
    if (ajRegExec(ncbiexp, plist->Line))
      (void) ajStrAssC(&plist->Line, "\n");
    if (ajRegExec(ncbiexp2, plist->Line))
      (void) ajStrAssC(&plist->Line, "\n");
    while (ajRegExec(fullexp, plist->Line)) {
      ajRegSubI (fullexp, 1, &s1);
      ajRegSubI (fullexp, 2, &s2);
      ajRegSubI (fullexp, 4, &s3);
      ajDebug ("removing '%S' [%d]\n", s2, ajStrRef(plist->Line));
      (void) ajFmtPrintS (&plist->Line, "%S%S", s1, s3);
    }
    while (ajRegExec(tagexp, plist->Line)) {
      ajRegSubI (tagexp, 1, &s1);
      ajRegSubI (tagexp, 2, &s2);
      ajRegSubI (tagexp, 3, &s3);
      ajDebug ("removing '%S' [%d]\n", s2, ajStrRef(plist->Line));
      (void) ajFmtPrintS (&plist->Line, "%S%S", s1, s3);
    }
    if (ajRegExec(nullexp, plist->Line)) { /* allow for newline */
      ajDebug ("<blank line deleted> [%d]\n", ajStrRef(plist->Line));
      pdellist = plist;
      if (plast) {
	plast->Next = plist->Next;
	plist = plast->Next;
      }
      else {			/* we are on the first line */
	plist = thys->Lines = thys->Curr = plist->Next;
      }
      ajStrDel(&pdellist->Line);
      AJFREE (pdellist);
      thys->Size--;
      if (thys->Pos > i)
	thys->Pos--;
    }
    else {
      ajDebug (":[%d] %S", ajStrRef(plist->Line), plist->Line);
      plast = plist;
      plist = plist->Next;
    }
    i++;
  }

  ajStrDel (&s1);
  ajStrDel (&s2);
  ajStrDel (&s3);

  /* free the regular expression - we expect to use them once only */

  ajRegFree (&tagexp);
  ajRegFree (&fullexp);
  ajRegFree (&httpexp);
  ajRegFree (&nullexp);
  ajRegFree (&chunkexp);
  ajRegFree (&ncbiexp);
  ajRegFree (&ncbiexp2);  

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
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoad (const AjPFileBuff thys) {

  static AjPStr rdline = NULL;
  AjBool stat = ajTrue;

  while (stat)
    stat = ajFileBuffGet (thys, &rdline);

  ajFileBuffReset (thys);

  /*ajFileBuffTrace (thys);*/

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
** @param [r] thys [const AjPFileBuff] Buffered file.
** @param [r] line [const char*] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoadC (const AjPFileBuff thys, const char* line) {

  if (!thys->Lines)
    thys->Curr = thys->Lines = AJNEW0(thys->Last);
  else
    thys->Last = AJNEW0(thys->Last->Next);

  (void) ajStrAssC (&thys->Last->Line,line);
  thys->Last->Next = NULL;
  thys->Size++;

  return;
}

/* @func ajFileBuffLoadS ******************************************************
**
** Adds a line to the buffer.
**
** Intended for cases where the file data must be preprocessed before
** being seen by the sequence reading routines. The first case was
** for stripping HTML tags after reading via HTTP.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @param [r] line [const AjPStr] Line of input.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffLoadS (const AjPFileBuff thys, const AjPStr line) {

  if (!thys->Lines)
    thys->Curr = thys->Lines = AJNEW0(thys->Last);
  else
    thys->Last = AJNEW0(thys->Last->Next);

  (void) ajStrAssS (&thys->Last->Line,line);
  thys->Last->Next = NULL;
  thys->Size++;

  return;
}

/* @func ajFileBuffReset ******************************************************
**
** Resets the pointer and current record of a file buffer so the next read
** starts at the first buffered line.
**
** @param [u] thys [const AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffReset (const AjPFileBuff thys) {
  thys->Pos = 0;
  thys->Curr = thys->Lines;
  return;
}

/* @func ajFileBuffFreeClear **************************************************
**
** Deletes freed lines from a file buffer. The free list is used to avoid
** reallocating space for new records and must be deleted as part of
*** the destructor.
**
** @param [u] thys [const AjPFileBuff] File buffer
** @return [void]
** @@
******************************************************************************/

void ajFileBuffFreeClear (const AjPFileBuff thys) {

  AjPFileBuffList list;

  if (!thys)
    return;

  ajDebug("ajFileBuffFreeClear %x\n", thys->Free);

  while (thys->Free) {
    /*ajDebug("Clearing Free %x %d %x\n",
	    thys->Free->Line, ajStrSize(thys->Free->Line),
	    thys->Free->Next);*/
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
** @param [u] thys [const AjPFileBuff] File buffer
** @param [r] lines [int] Number of lines to retain. -1 deletes everything.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffClear (const AjPFileBuff thys, int lines) {

  int i=0;
  AjPFileBuffList list;
  AjPFileBuffList next;
  int first;
  int ifree=0;

  ajDebug ("ajFileBuffClear (%d) Nobuff: %B\n", lines, thys->Nobuff);
  /*FileBuffTraceFull (thys, thys->Size, 100);*/
  if (!thys)
    return;

  if (!thys->File)
    return;

  if (lines < 0)
    first = thys->Size;
  else
    first = thys->Pos - lines;

  if (first < 0)
    first = 0;

  if (thys->Nobuff && thys->Pos == thys->Size) /* nobuff, and all read */
    first = thys->Pos;			/* delete any old saved line */

  ajDebug(" first: %d thys->Pos: %d thys->Size: %d thys->Nobuff: %B\n",
	  first, thys->Pos, thys->Size, thys->Nobuff);

  list = thys->Lines;
  for (i=0; i < first; i++) {
    next = list->Next;		/* we save one line at a time */
				/* so keep a note of the next one for later */
    /*ajDebug ("Try to reuse %x size: %d use: %d\n",
      list->Line, ajStrSize(list->Line), ajStrRef(list->Line));*/

    if (thys->Nobuff)
      ajStrDel(&list->Line);

    if (!thys->Nobuff &&
	ajStrDelReuse(&list->Line)) { /* move free line to the end */

      /*ajDebug ("can save to free list %x %d bytes\n",
	list->Line, ajStrSize(list->Line));*/

      ifree++;
      list->Next = NULL;	/* just save the one line */
      if (!thys->Free) {
	thys->Free = list; /* start a new free list */
	thys->Freelast = list;
	/*ajDebug ("start  list Free %x Freelast %x \n",
	  thys->Free, thys->Freelast);*/
      }
      else {
	thys->Freelast->Next = list; /* append to free list */
	thys->Freelast = thys->Freelast->Next;
	/*ajDebug ("append list Free %x Freelast %x \n",
	  thys->Free, thys->Freelast);*/
      }
    }
    else {
      /*ajDebug ("have to delete this line\n'%S'\n", list->Line);*/
      AJFREE(list);	/* deleted, kill the list item */
    }	
    list = next;
  }

  ajDebug ("ajFileBuffClear '%F' (%d lines)\n"
	   "     %b size: %d pos: %d removed %d lines add to free: %d\n",
	 thys->File, lines, thys->Nobuff, thys->Size, thys->Pos, i, ifree);

  thys->Pos = 0;
  thys->Size -= i;

  thys->Lines = thys->Curr = list;

  if (thys->Nobuff && !thys->Size && lines == 1) {
				/* unbuffered - can only save last line */
    /*ajDebug ("Nobuff in effect, saving last line read\n'%S'\n",
      thys->File->Buff);*/
    if (thys->Lines)
      ajFatal("Buffer error clearing unbuffered file in ajFileBuffClear\n");

    thys->Lines = AJNEW0(thys->Last);

    (void) ajStrAssS(&thys->Last->Line, thys->File->Buff);
    thys->Curr = thys->Last;
    thys->Curr->Fpos = thys->Fpos;
    thys->Last->Next = NULL;
    thys->Pos = 0;
    thys->Size = 1;
  }

  return;
}

/* @func ajFileBuffNobuff *****************************************************
**
** Sets file to be unbuffered. If it already has buffered data, we have to
** first run down the buffer.
**
** @param [r] thys [const AjPFileBuff] Buffered file object.
** @return [void]
** @@
******************************************************************************/

void ajFileBuffNobuff (const AjPFileBuff thys) {

  ajDebug ("ajFileBuffNoBuff %F buffsize: %d\n", thys->File, thys->Size);
  thys->Nobuff = ajTrue;

  return;
}

/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */

/* @section Buffered File Operators *****************************************
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

void ajFileBuffTrace (const AjPFileBuff thys) {

  AjPFileBuffList test;
  int i = 0;
  int j = -1;

  ajDebug ("Trace buffer file '%S'\n"
	   "             Pos: %d Size: %d End: %b\n",
	  thys->File->Name, thys->Pos, thys->Size, thys->File->End);
  if (thys->Size) {
    ajDebug (" Lines:\n");
    ajDebug ("  Curr: %8ld <%S>\n", thys->Curr->Fpos,  thys->Curr->Line);
    ajDebug ("  From: %8ld <%S>\n", thys->Lines->Fpos, thys->Lines->Line);
    ajDebug ("    To: %8ld <%S>\n", thys->Last->Fpos,  thys->Last->Line);
  }
  if (thys->Free) {
    for (test = thys->Free; test; test=test->Next) {
      i++;
      if (test == thys->Freelast) j = i;
    }
  }
  ajDebug (" Free: %d Last: %d\n", i, j);
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

void ajFileBuffTraceFull (const AjPFileBuff thys, size_t nlines,
			  size_t nfree) {

  int i;
  AjPFileBuffList line;
  AjBool last = ajFalse;

  ajDebug ("Trace buffer file '%S'\n"
	   "             Pos: %d Size: %d\n",
	  thys->File->Name, thys->Pos, thys->Size);

  line = thys->Lines;
  for (i=1; line && (i <= nlines); i++) {
    if (line == thys->Curr)
      ajDebug ("*Line %x %d: %8ld <%-20S>\n",
	       line->Line, i, line->Fpos, line->Line);
    else
      ajDebug (" Line %x %d: %8ld <%-20S>\n",
	       line->Line, i, line->Fpos, line->Line);
    line = line->Next;
  }
  line = thys->Free;
  for (i=1; line && (i <= nfree);  i++) {
    if (line == thys->Freelast) last = ajTrue;
    ajDebug (" Free %x %d: %d bytes %B\n",
	     line->Line, i, ajStrSize(line->Line), last);
    line = line->Next;
  }

  return;
}

/* ==================================================================== */
/* ============================ Casts ==================================*/
/* ==================================================================== */

/* @section Buffered File Casts ********************************************
**
** These functions examine the contents of a file object and return some
** derived information. Some of them provide access to the internal
** components of a file object. They are provided for programming convenience
** but should be used with caution.
**
******************************************************************************/

/* @func ajFileBuffFp *******************************************************
**
** Returns the C file pointer for an open buffered file.
**
** @param [r] thys [const AjPFileBuff] Buffered file.
** @return [FILE*] C file pointer for the file.
** @@
******************************************************************************/

FILE* ajFileBuffFp (const AjPFileBuff thys) {
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

AjPFile ajFileBuffFile (const AjPFileBuff thys) {
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

AjBool ajFileBuffEmpty (const AjPFileBuff thys) {

  ajDebug ("ajFileBuffEmpty Size: %d Pos: %d End: %b Handle: %d Fp: %x List; %d\n",
	   thys->Size, thys->Pos, thys->File->End, thys->File->Handle,
	   thys->File->fp, ajListstrLength(thys->File->List));

  if (thys->Pos < thys->Size)
    return ajFalse;

  if (!thys->File->fp || !thys->File->Handle) /* not open */
     return ajTrue;

  if (thys->File->End && !ajListstrLength(thys->File->List)) /* EOF and done */
    return ajTrue;

  ajDebug ("ajFileBuffEmpty false\n");

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

AjBool ajFileNameDirSet (AjPStr* filename, const AjPStr dir) {
  if (!ajStrLen(dir))
    return ajFalse;
  return ajFileNameDirSetC (filename, ajStrStr(dir));
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

AjBool ajFileNameDirSetC (AjPStr* filename, const char* dir) {
  static AjPRegexp fileexp = NULL;
  static AjPStr tmpstr = NULL;
  static AjPStr tmpdir = NULL;
  static AjPStr tmpnam = NULL;

  if (!dir)
    return ajFalse;

  if (!fileexp)
    fileexp = ajRegCompC ("(.*/)?([^/]+)$");

  ajDebug ("ajFileNameDirSetC '%S', '%s'\n", *filename, dir);
  ajStrAssS (&tmpstr, *filename);
  if (ajRegExec(fileexp, tmpstr)) {
    ajRegSubI(fileexp, 1, &tmpdir);
    ajRegSubI(fileexp, 2, &tmpnam);
    if (ajStrLen(tmpdir)) {
      ajDebug ("Directory not replaced\n");
      return ajFalse;			/* we already have a directory */
    }
    if (dir[strlen(dir)-1] == '/')
      ajFmtPrintS (filename, "%s%S", dir, tmpnam);
    else
      ajFmtPrintS (filename, "%s/%S", dir, tmpnam);
    ajDebug ("Directory replaced: '%S'\n", *filename);
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

AjBool ajFileNameExt (AjPStr* filename, const AjPStr extension) {
  if (!extension)
    return ajFileNameExtC (filename, NULL);
  return ajFileNameExtC (filename, ajStrStr(extension));
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

AjBool ajFileNameExtC (AjPStr* filename, const char* extension) {

  static AjPRegexp fileexp = NULL;
  static AjPStr tmpstr = NULL;
  static AjPStr tmpnam = NULL;
  static AjPStr tmpdir = NULL;
  static AjPStr tmpext = NULL;

  AjBool doext = ajTrue;
  if (!extension || !*extension)
    doext = ajFalse;
  
  if (!fileexp)
    fileexp = ajRegCompC ("(.*/)?([^./]*)([.]([^./]*))?$");

  ajDebug ("ajFileNameExtC '%S' '%s'\n", *filename, extension);

  ajStrAssS (&tmpstr, *filename);

  if (ajRegExec(fileexp, tmpstr)) {
    ajRegSubI(fileexp, 1, &tmpdir);
    ajRegSubI(fileexp, 2, &tmpnam);
    ajRegSubI(fileexp, 3, &tmpext);
    if (!ajStrLen(tmpdir))
      ajStrAssC(&tmpdir, "");
    if (ajStrLen(tmpext)) {
      ajDebug ("replace extension '%S', '%S', <%S>\n",
	       tmpdir, tmpnam, tmpext);
      if (doext)
	ajFmtPrintS (filename, "%S%S.%s", tmpdir, tmpnam, extension);
      else
	ajFmtPrintS (filename, "%S%S", tmpdir, tmpnam);
    }
    else {
      ajDebug ("new extension '%S', '%S', <%S>\n",
	       tmpdir, tmpnam, tmpext);
      if (doext)
	ajFmtPrintS (filename, "%S%S.%s", tmpdir, tmpnam, extension);
      else
	ajFmtPrintS (filename, "%S%S", tmpdir, tmpnam);
    }
  }

  return ajTrue;
}

/* @func ajFileScan ******************************************************
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
** @return [void]
** @@
******************************************************************************/

void ajFileScan(AjPStr path, AjPStr filename, AjPList *result,
		AjBool show, AjBool dolist, AjPList *list,
		AjPList rlist, AjBool recurs, const AjPFile outf)
{
    AjPList dirs=NULL;
    AjIList iter=NULL;
    DIR *indir;
    struct dirent *dp;
    AjPStr s=NULL;
    AjPStr t=NULL;
    AjBool flag;
    
    if(dolist)
    {
	t=ajStrNewC(ajStrStr(path));
	ajListPushApp(*list,(void *)t);
    }

    if(show)
	ajFmtPrintF(outf,"\n\nDIRECTORY: %s\n\n",ajStrStr(path));

    if(!(indir=opendir(ajStrStr(path))))
	return;

    s = ajStrNew();
    dirs = ajListNew();

    while((dp=readdir(indir)))
    {
	if(!dp->d_ino || !strcmp(dp->d_name,".") || !strcmp(dp->d_name,".."))
	    continue;
	ajStrAssC(&s,ajStrStr(path));
	ajStrAppC(&s,"/");
	ajStrAppC(&s,dp->d_name);
	if(ajFileStat(&s,AJ_FILE_DIR))  /* Its a directory */
	{
	    if(!recurs)
		continue;
	    if(rlist) 			/* Ignore selected directories */
	    {
		flag=ajFalse;
		iter=ajListIter(rlist);
		while(ajListIterMore(iter))
		{
		    t=ajListIterNext(iter);
		    if(!strcmp(ajStrStr(t),dp->d_name))
		    {
			flag=ajTrue;
			break;
		    }
		}
		ajListIterFree(iter);
		if(flag) continue;
	    }

	    if(!ajFileStat(&s,AJ_FILE_R) || !ajFileStat(&s,AJ_FILE_X))
		continue;
	    t = ajStrNewC(ajStrStr(s));
	    ajListPushApp(dirs,(void *)t);
	}
	else if(ajFileStat(&s,AJ_FILE_R))   /* A normal file */
	{
	    if(filename)		    /* A search file was given */
	    {
/*		if(!strcmp(dp->d_name,ajStrStr(filename)))*/
		if(ajStrMatchWildCC(dp->d_name,ajStrStr(filename)))
		{
		    t = ajStrNewC(ajStrStr(s));
		    ajListPushApp(*result,(void *)t);
		}
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
    {
	while(ajListPop(dirs,(void **)&t))
	{
	    ajFileScan(t,filename,result,show,dolist,list,rlist,recurs,outf);
	    ajStrDel(&t);
	}
    }

    ajStrDel(&s);
    ajListDel(&dirs);

    return;
}

/* @func ajFileTestSkip ****************************************************
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
** @return [AjBool] ajTrue if the filename is accepted.
** @@
******************************************************************************/

AjBool ajFileTestSkip (AjPStr fullname, AjPStr exc, AjPStr inc,
		       AjBool keep) {

  AjBool ret = keep;
  static AjPStrTok handle = NULL;
  static AjPStr token = NULL;

  /*  ajDebug ("ajFileTestSkip: file '%S' exclude: '%S' include: '%S'\n",
      fullname, exc, inc);*/

  if (keep) {			/* keep, so test exclude first */
    (void) ajStrTokenAss (&handle, exc, " \t,;\n");
    while (ajStrToken (&token, &handle, NULL)) {
      if (ajStrMatchWild (fullname, token)) {
	ret = ajFalse;
	/*ajDebug ("ajFileTestSkip: file '%S' excluded by '%S'\n",
	  fullname, token);*/
      }
    }
    (void) ajStrTokenReset (&handle);
  }

  (void) ajStrTokenAss (&handle, inc, " \t,;\n");
  while (ajStrToken (&token, &handle, NULL)) {
    if (ajStrMatchWild (fullname, token)) {
      ret = ajTrue;
      /*      ajDebug ("ajFileTestSkip: file '%S' included by '%S'\n",
	      fullname, token);*/
    }
  }
  (void) ajStrTokenReset (&handle);

  if (!keep) {			/* nokeep, test exclude last */
    (void) ajStrTokenAss (&handle, exc, " \t,;\n");
    while (ajStrToken (&token, &handle, NULL)) {
      if (ajStrMatchWild (fullname, token)) {
	ret = ajFalse;
	/*ajDebug ("ajFileTestSkip: file '%S' excluded by '%S'\n",
	  fullname, token);*/
      }
    }
    (void) ajStrTokenReset (&handle);
  }

  return ret;
}

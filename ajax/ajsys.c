/********************************************************************
** @source AJAX system functions
**
** Copyright (c) Alan Bleasby 1999
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

#include "ajsys.h"
#ifndef __VMS
#include <termios.h>
#endif
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

/* @func ajSysBasename *******************************************************
**
** Removes a directory specification from a filename
**
** @param [rw] s [AjPStr*] Filename in AjStr.
** @return [void]
** @@
******************************************************************************/

void ajSysBasename(AjPStr *s)
{
    char *p;
    char *t;
    ajint  len;

    len=ajStrLen(*s);
    if(!len) return;
    t = ajStrStr(*s);

    p = t+(len-1);
    while(p!=t)
    {
	if(*p=='/') break;
	--p;
    }
    
    if(p!=t)
    {
	(void) ajStrClear(s);
	(void) ajStrSetC(s, p+1);
    }
    return;
}

    
/* @func ajSysItoC *******************************************************
**
** Convert Int to Char
** Needed for very fussy compilers i.e. Digital C
**
** @param [r] v [ajint] integer
** @return [char] Character cast
** @@
******************************************************************************/

char ajSysItoC(ajint v)
{
    char c;

    c = (char) v;
    return c;
}


/* @func ajSysItoUC *******************************************************
**
** Convert Int to Unsigned Char
** Needed for very fussy compilers i.e. Digital C
**
** @param [r] v [ajint] integer
** @return [unsigned char] Unisigned character cast
** @@
******************************************************************************/

unsigned char ajSysItoUC(ajint v)
{
    char c;

    c = (unsigned char) v;
    return c;
}


/* @func ajSysWhich *******************************************************
**
** Gets the Basename of a file then searches $PATH sequentially until it
** finds a user-EXECUTABLE file of the same name.
**
** @param [rw] s [AjPStr*] Filename in AjStr, replaced by full pathname
** @return [AjBool] True if executable found, false otherwise
** @@
******************************************************************************/

AjBool ajSysWhich(AjPStr *s)
{
    char *p;
    static AjPStr tname=NULL;
    static AjPStr fname=NULL;

    p = getenv("PATH");
    if (!p) {
      return ajFalse;
    }

    (void) ajStrAssS(&tname, *s);

    if(!fname)
	fname = ajStrNew();

    ajSysBasename(&tname);
    
    p=ajSysStrtok(p,":");
    if(p==NULL)
    {
	ajStrDelReuse(&fname);
	ajStrDelReuse(&tname);
	return ajFalse;
    }

    while(1)
    {
	(void) ajFmtPrintS(&fname,"%s/%S",p,tname);

	if(ajFileStat(&fname, AJ_FILE_X))
	{
	    (void) ajStrClear(s);
	    (void) ajStrSet(s,fname);
	    break;
	}
	if((p=ajSysStrtok(NULL,":"))==NULL)
        {
	    ajStrDelReuse(&fname);
	    ajStrDelReuse(&tname);
	    return ajFalse;
        }
    }

    ajStrDelReuse(&fname);
    ajStrDelReuse(&tname);
    return ajTrue;
}



/* @func ajSysWhichEnv *******************************************************
**
** Gets the Basename of a file then searches $PATH sequentially until it
** finds a user-EXECUTABLE file of the same name.
**
** @param [rw] s [AjPStr*] Filename in AjStr, replaced by full pathname
** @param [rw] env [char**] Environment
** @return [AjBool] True if executable found, false otherwise
** @@
******************************************************************************/

AjBool ajSysWhichEnv(AjPStr *s, char **env)
{
    ajint count;
    char *p;
    AjPStr tname=NULL;
    AjPStr fname=NULL;
    AjPStr path=NULL;


    tname = ajStrNew();
    (void) ajStrSet(&tname, *s);


    fname = ajStrNew();
    path  = ajStrNew();

    ajSysBasename(&tname);
    
    
    count=0;
    while(*env[count])
    {
	if(!strncmp("PATH=",env[count],5)) break;
	++count;
    }
    if(!(*env[count]))
    {
	ajStrDel(&fname);
	ajStrDel(&tname);
	ajStrDel(&path);
	return ajFalse;
    }
    
    (void) ajStrSetC(&path, env[count]);
    p=ajStrStr(path);
    p+=5;
    p=ajSysStrtok(p,":");
    if(p==NULL)
    {
	ajStrDel(&fname);
	ajStrDel(&tname);
	ajStrDel(&path);
	return ajFalse;
    }

    while(1)
    {
	(void) ajStrClear(&fname);
	(void) ajFmtPrintS(&fname,"%s/%S",p,tname);

	if(ajFileStat(&fname, AJ_FILE_X))
	{
	    (void) ajStrClear(s);
	    (void) ajStrSet(s,fname);
	    break;
	}
	if((p=ajSysStrtok(NULL,":"))==NULL)
        {
	    ajStrDel(&fname);
	    ajStrDel(&tname);
	    ajStrDel(&path);
	    return ajFalse;
        }
    }

    ajStrDel(&fname);
    ajStrDel(&tname);
    ajStrDel(&path);
    return ajTrue;
}



/* @func ajSystem *******************************************************
**
** Exec a command line as if from the C shell
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cl [AjPStr*] The command line
** @return [void]
** @@
******************************************************************************/

void ajSystem(AjPStr *cl)
{
    pid_t pid;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname=NULL;

    if(!ajSysArglist(*cl, &pgm, &argptr)) return;
    
    (void) ajStrAssC(&pname, pgm);
    if (!ajSysWhich(&pname))
      ajFatal("cannot find program '%S'", pname);

    pid=fork();
    if(pid==-1)
	ajFatal("System fork failed");
    
    if(pid)
	while(wait(&status)!=pid);
    else
	(void) execv(ajStrStr(pname), argptr);

    ajStrDel(&pname);

    i=0;
    while(argptr[i])
    {
	AJFREE (argptr[i]);
	++i;
    }
    AJFREE (argptr);
    
    AJFREE (pgm);
    
    
    return;
}



/* @func ajSystemEnv *******************************************************
**
** Exec a command line as if from the C shell
**
** This routine must be passed the environment received from
** main(ajint argc, char **argv, char **env)
** The environment is used to extract the PATH list (see ajWhich)
**
** Note that the environment is passed through unaltered. The exec'd
** program is passed a new argv array
**
** @param [r] cl [AjPStr*] The command line
** @param [r] env [char**] The environment
** @return [void]
** @@
******************************************************************************/

void ajSystemEnv (AjPStr *cl, char **env)
{
    pid_t pid;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;
    

    AjPStr pname=NULL;

    if(!ajSysArglist(*cl, &pgm, &argptr)) return;
    
    (void) ajStrAssC(&pname, pgm);
    if (!ajSysWhichEnv(&pname, env))
      ajFatal("cannot find program '%S'", pname);

    pid=fork();
    if(pid==-1)
	ajFatal("System fork failed");
    
    if(pid)
	while(wait(&status)!=pid);
    else
	(void) execve(ajStrStr(pname), argptr, env);

    ajStrDel(&pname);

    i=0;
    while(argptr[i])
    {
	AJFREE (argptr[i]);
	++i;
    }
    AJFREE (argptr);
    
    AJFREE (pgm);
    
    
    return;
}



/* @func ajSysUnlink *******************************************************
**
** Deletes a file or link
**
** @param [r] s [AjPStr*] Filename in AjStr.
** @return [AjBool] true if deleted false otherwise
** @@
******************************************************************************/

AjBool ajSysUnlink(AjPStr *s)
{
    if(!unlink(ajStrStr(*s))) return ajTrue;
    return ajFalse;
}





/* @func ajSysCanon  *******************************************************
**
** Sets or unsets TTY canonical mode
**
** @param [r] state [AjBool] state=true sets canon state=false sets noncanon
** @return [void]
** @@
******************************************************************************/

void ajSysCanon(AjBool state)
{
#ifndef __VMS
    static struct termios tty;


    tcgetattr(1, &tty);
    tty.c_cc[VMIN]='\1';
    tty.c_cc[VTIME]='\0';
    tcsetattr(1,TCSANOW,&tty);
    
    if(state)
	tty.c_lflag |= ICANON;
    else
	tty.c_lflag &= ~(ICANON);

      tcsetattr(1, TCSANOW, &tty);
#endif
    return;
}

/* @func ajSysArglist *****************************************************
**
** Generates a program name and argument list from a command line string.
**
** @param [r] cmdline [AjPStr] Command line.
** @param [w] pgm [char**] Program name.
** @param [w] arglist [char***] Argument list.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSysArglist (AjPStr cmdline, char** pgm, char*** arglist) {

  static AjPRegexp argexp = NULL;
  AjPStr tmpline = NULL;
  char* cp;
  ajint ipos = 0;
  ajint iarg = 0;
  ajint ilen = 0;
  ajint i;
  char** al;
  AjPStr argstr = NULL;

  if (!argexp)
    argexp = ajRegCompC ("^[ \t]*(\"([^\"]*)\"|'([^']*)'|([^ \t]+))");

  ajDebug("ajSysArgList '%S'\n", cmdline);

  (void) ajStrAss (&tmpline, cmdline);

  cp = ajStrStr(cmdline);
  ipos = 0;
  while (ajRegExecC (argexp, &cp[ipos])) {
    ipos += ajRegLenI (argexp, 0);
    iarg++;
  }

  AJCNEW (*arglist, iarg+1);
  al = *arglist;
  ipos = 0;
  iarg = 0;
  while (ajRegExecC (argexp, &cp[ipos])) {
    ilen = ajRegLenI (argexp, 0);
    (void) ajStrDelReuse(&argstr);
    for (i=2;i<5;i++) {
      if (ajRegLenI(argexp, i)) {
	ajRegSubI (argexp, i, &argstr);
	/* ajDebug("parsed [%d] '%S'\n", i, argstr);*/
	break;
      }
    }
    ipos += ilen;
    if (!iarg) {
      *pgm = ajCharNewC (ajStrLen(argstr), ajStrStr(argstr));
    }
    al[iarg] = ajCharNewC (ajStrLen(argstr), ajStrStr(argstr));
    iarg++;
  }
  al[iarg] = NULL;

  ajRegFree(&argexp);
  argexp = NULL;
  ajStrDel(&tmpline);
  ajStrDel(&argstr);
  
  return ajTrue;
}

/* @func ajSysArgListFree *****************************************************
**
** Free memory in an argument list allocated by ajSysArgList
**
** @param [w] arglist [char***] Argument list.
** @return [void]
** @@
******************************************************************************/

void ajSysArgListFree (char*** arglist) {

  char** ca = *arglist;
  ajint i;

  i=0;
  while (ca[i]) {
    AJFREE(ca[i]);
    ++i;
  }

  AJFREE (*arglist);
}


/* @func ajSysFdopen *******************************************************
**
** Place non-ANSI fdopen here
**
** @param [r] filedes [ajint] file descriptor
** @param [r] mode [const char *] file mode
** @return [FILE*] file pointer
** @@
******************************************************************************/

FILE* ajSysFdopen(ajint filedes, const char *mode)
{
    return fdopen(filedes,(char *)mode);
}



/* @func ajSysStrdup *******************************************************
**
** Duplicate BSD strdup function for very strict ANSI compilers
**
** @param [r] s [const char *] string to duplicate
** @return [char*] Text string as for strdup
** @@
******************************************************************************/

char* ajSysStrdup(const char *s)
{
    static char *p;

    AJCNEW (p, strlen(s)+1);
    (void) strcpy(p,s);

    return p;
}


/* @func ajSysIsRegular ******************************************************
**
** Test for regular file
**
** @param [r] s [const char *] filename
** @return [AjBool] true if regular
** @@
******************************************************************************/

AjBool ajSysIsRegular(const char *s)
{
    static struct stat buf;

    if(stat(s,&buf)==-1)
	return ajFalse;
    
    if((ajint)buf.st_mode & AJ_FILE_REG)
	return ajTrue;

    return ajFalse;
}


/* @func ajSysIsDirectory ******************************************************
**
** Test for a directory
**
** @param [r] s [const char *] directory name
** @return [AjBool] true if directory
** @@
******************************************************************************/

AjBool ajSysIsDirectory(const char *s)
{
    static struct stat buf;

    if(stat(s,&buf)==-1)
	return ajFalse;
    
    if((ajint)buf.st_mode & AJ_FILE_DIR)
	return ajTrue;

    return ajFalse;
}



/* @func ajSysStrtok ******************************************************
**
** strtok that doesn't corrupt the source string
**
** @param [r] s [const char *] source string
** @param [r] t [const char *] delimiter string
**
** @return [char*] pointer or NULL
** @@
******************************************************************************/

char* ajSysStrtok(const char *s, const char *t)
{
    static AjPStr rets=NULL;
    static AjPStr sou =NULL;
    static char *p;
    ajint len;

    if(s)
    {
	if(!rets)
	{
	    sou  = ajStrNew();
	    rets = ajStrNew();
	}
	ajStrAssC(&sou,s);
	p = ajStrStr(sou);
    }

    if(!*p)
	return NULL;

    len = strspn(p,t);
    p += len;
    len = strcspn(p,t);
    ajStrAssSubC(&rets,p,0,len-1);
    p += len;
    len = strspn(p,t);
    p += len;
    
    return ajStrStr(rets);
}

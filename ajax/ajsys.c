/******************************************************************************
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
******************************************************************************/

#include "ajsys.h"
#ifndef __VMS
#include <termios.h>
#endif
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>




/* @func ajSysBasename ********************************************************
**
** Removes a directory specification from a filename
**
** @param [u] s [AjPStr*] Filename in AjStr.
** @return [void]
** @@
******************************************************************************/

void ajSysBasename(AjPStr *s)
{
    const char *p;
    const char *t;
    ajint  len;

    len = ajStrLen(*s);
    if(!len)
	return;

    t = ajStrStr(*s);

    p = t+(len-1);
    while(p!=t)
    {
	if(*p=='/')
	    break;
	--p;
    }

    if(p!=t)
	ajStrAssC(s, p+1);

    return;
}




/* @func ajSysItoC ************************************************************
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




/* @func ajSysItoUC ***********************************************************
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




/* @func ajSysWhich ***********************************************************
**
** Gets the Basename of a file then searches $PATH sequentially until it
** finds a user-EXECUTABLE file of the same name.
**
** @param [u] s [AjPStr*] Filename in AjStr, replaced by full pathname
** @return [AjBool] True if executable found, false otherwise
** @@
******************************************************************************/

AjBool ajSysWhich(AjPStr *s)
{
    char *p;
    static AjPStr tname = NULL;
    static AjPStr fname = NULL;

    p = getenv("PATH");
    if(!p)
	return ajFalse;

    ajStrAssS(&tname, *s);

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
	ajFmtPrintS(&fname,"%s/%S",p,tname);

	if(ajFileStat(fname, AJ_FILE_X))
	{
	    ajStrClear(s);
	    ajStrSet(s,fname);
	    break;
	}
	if((p = ajSysStrtok(NULL,":"))==NULL)
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




/* @func ajSysWhichEnv ********************************************************
**
** Gets the Basename of a file then searches $PATH sequentially until it
** finds a user-EXECUTABLE file of the same name. Reentrant.
**
** @param [u] s [AjPStr*] Filename in AjStr, replaced by full pathname
** @param [r] env [char* const[]] Environment
** @return [AjBool] True if executable found, false otherwise
** @@
******************************************************************************/

AjBool ajSysWhichEnv(AjPStr *s, char * const env[])
{
    ajint count;
    char *p = NULL;
    const char *cp;
    AjPStr tname = NULL;
    AjPStr fname = NULL;
    AjPStr path  = NULL;
    char   *save = NULL;
    AjPStr buf   = NULL;
    AjPStr tmp   = NULL;
    
    
    buf   = ajStrNew();
    tname = ajStrNew();
    tmp   = ajStrNew();
    ajStrAssS(&tname,*s);
    
    fname = ajStrNew();
    path  = ajStrNew();
    
    ajSysBasename(&tname);
 
    ajDebug("ajSysWhichEnv '%S' => %S\n", *s, tname);

    count = 0;
    while(env[count]!=NULL)
    {
	if(!(*env[count]))
	    break;

	ajDebug("  env[%d] '%s'\n", count, env[count]);

	if(!strncmp("PATH=",env[count],5))
	    break;

	++count;
    }
    
    ajDebug("PATH  env[%d] '%s'\n", count, env[count]);

    if(env[count]==NULL || !(*env[count]))
    {
	ajStrDel(&fname);
	ajStrDel(&tname);
	ajStrDel(&path);
	ajStrDel(&buf);
	ajStrDel(&tmp);
	return ajFalse;
    }
    
    ajStrAssC(&path, env[count]);
    cp = ajStrStr(path);
    cp += 5;
    ajStrAssC(&tmp,cp);

    ajDebug("tmp '%S' save '%S' buf '%S'\n", tmp, save, buf);
 
    p = ajSysStrtokR(ajStrStr(tmp),":",&save,&buf);
    
    if(p==NULL)
    {
	ajStrDel(&fname);
	ajStrDel(&tname);
	ajStrDel(&path);
	ajStrDel(&buf);
	ajStrDel(&tmp);
	return ajFalse;
    }
    
    
    ajFmtPrintS(&fname,"%s/%S",p,tname);
    while(!ajFileStat(fname, AJ_FILE_X))
    {
	if((p = ajSysStrtokR(NULL,":",&save,&buf))==NULL)
	{
	    ajStrDel(&fname);
	    ajStrDel(&tname);
	    ajStrDel(&path);
	    ajStrDel(&buf);
	    ajStrDel(&tmp);
	    return ajFalse;
	}
	ajFmtPrintS(&fname,"%s/%S",p,tname);
    }
    
    
    ajStrAssS(s,fname);
    ajDebug("ajSysWhichEnv returns '%S'\n", *s);

    ajStrDel(&fname);
    ajStrDel(&tname);
    ajStrDel(&path);
    ajStrDel(&buf);
    ajStrDel(&tmp);
    
    return ajTrue;
}




/* @func ajSystem *************************************************************
**
** Exec a command line as if from the C shell
**
** The exec'd program is passed a new argv array in argptr
**
** @param [r] cl [const AjPStr] The command line
** @return [void]
** @@
******************************************************************************/

void ajSystem(const AjPStr cl)
{
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm;
    char **argptr;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglist(cl, &pgm, &argptr))
	return;

    pname = ajStrNew();

    ajStrAssC(&pname, pgm);

    if(!ajSysWhich(&pname))
	ajFatal("cannot find program '%S'", pname);

    pid=fork();

    if(pid==-1)
	ajFatal("System fork failed");

    if(pid)
    {
	while((retval=waitpid(pid,&status,WNOHANG))!=pid)
	{
	    if(retval == -1)
		if(errno != EINTR)
		    break;
	}
    }
    else
	execv(ajStrStr(pname), argptr);

    ajStrDel(&pname);

    i = 0;
    while(argptr[i])
    {
	AJFREE(argptr[i]);
	++i;
    }
    AJFREE(argptr);

    AJFREE(pgm);

    return;
}




/* @func ajSystemEnv **********************************************************
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
** @param [r] cl [const AjPStr] The command line
** @param [r] env [char* const[]] The environment
** @return [void]
** @@
******************************************************************************/

void ajSystemEnv(const AjPStr cl, char * const env[])
{
    pid_t pid;
    pid_t retval;
    ajint status;
    char *pgm = NULL;
    char **argptr = NULL;
    ajint i;

    AjPStr pname = NULL;

    if(!ajSysArglist(cl, &pgm, &argptr))
	return;

    pname = ajStrNew();

    ajDebug("ajSystemEnv '%s' %S \n", pgm, cl);
    ajStrAssC(&pname, pgm);
    if(!ajSysWhichEnv(&pname, env))
	ajFatal("cannot find program '%S'", pname);

    ajDebug("ajSystemEnv %S = %S\n", pname, cl);
    for (i=0;argptr[i]; i++)
    {
	ajDebug("%4d '%s'\n", i, argptr[i]);
    }

    pid = fork();
    if(pid==-1)
	ajFatal("System fork failed");

    if(pid)
    {
	while((retval=waitpid(pid,&status,WNOHANG))!=pid)
	{
	    if(retval == -1)
		if(errno != EINTR)
		    break;
	}
    }
    else
	execve(ajStrStr(pname), argptr, env);

    ajStrDel(&pname);

    i = 0;
    while(argptr[i])
    {
	AJFREE(argptr[i]);
	++i;
    }
    AJFREE(argptr);

    AJFREE(pgm);


    return;
}




/* @func ajSysUnlink **********************************************************
**
** Deletes a file or link
**
** @param [r] s [const AjPStr] Filename in AjStr.
** @return [AjBool] true if deleted false otherwise
** @@
******************************************************************************/

AjBool ajSysUnlink(const AjPStr s)
{
    if(!unlink(ajStrStr(s)))
	return ajTrue;

    return ajFalse;
}




/* @func ajSysCanon  **********************************************************
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
    tty.c_cc[VMIN]  = '\1';
    tty.c_cc[VTIME] = '\0';
    tcsetattr(1,TCSANOW,&tty);

    if(state)
	tty.c_lflag |= ICANON;
    else
	tty.c_lflag &= ~(ICANON);

    tcsetattr(1, TCSANOW, &tty);
#endif

    return;
}




/* @func ajSysArglist *********************************************************
**
** Generates a program name and argument list from a command line string.
**
** @param [r] cmdline [const AjPStr] Command line.
** @param [w] pgm [char**] Program name.
** @param [w] arglist [char***] Argument list.
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajSysArglist(const AjPStr cmdline, char** pgm, char*** arglist)
{    
    static AjPRegexp argexp = NULL;
    AjPStr tmpline          = NULL;
    const char* cp;
    ajint ipos = 0;
    ajint iarg = 0;
    ajint ilen = 0;
    ajint i;
    char** al;
    AjPStr argstr = NULL;
    
    if(!argexp)
	argexp = ajRegCompC("^[ \t]*(\"([^\"]*)\"|'([^']*)'|([^ \t]+))");
    
    ajDebug("ajSysArgList '%S'\n", cmdline);
    
    ajStrAssS(&tmpline, cmdline);
    
    cp   = ajStrStr(cmdline);
    ipos = 0;
    while(ajRegExecC(argexp, &cp[ipos]))
    {
	ipos += ajRegLenI(argexp, 0);
	iarg++;
    }
    
    AJCNEW(*arglist, iarg+1);
    al   = *arglist;
    ipos = 0;
    iarg = 0;
    while(ajRegExecC(argexp, &cp[ipos]))
    {
	ilen = ajRegLenI(argexp, 0);
	ajStrDelReuse(&argstr);
	for(i=2;i<5;i++)
	{
	    if(ajRegLenI(argexp, i))
	    {
		ajRegSubI(argexp, i, &argstr);
		ajDebug("parsed [%d] '%S'\n", i, argstr);
		break;
	    }
	}
	ipos += ilen;

	if(!iarg)
	    *pgm = ajCharNew(argstr);

	al[iarg] = ajCharNew(argstr);
	iarg++;
    }

    al[iarg] = NULL;
    
    ajRegFree(&argexp);
    argexp = NULL;
    ajStrDel(&tmpline);
    ajStrDel(&argstr);
    
    ajDebug("ajSysArgList %d args for '%s'\n", iarg, *pgm);
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

void ajSysArgListFree(char*** arglist)
{
    char** ca;
    ajint i;

    ca = *arglist;

    i = 0;
    while(ca[i])
    {
	AJFREE(ca[i]);
	++i;
    }

    AJFREE(*arglist);

    return;
}




/* @func ajSysFdopen **********************************************************
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




/* @func ajSysStrdup **********************************************************
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

    AJCNEW(p, strlen(s)+1);
    strcpy(p,s);

    return p;
}




/* @func ajSysIsRegular *******************************************************
**
** Test for regular file
**
** @param [r] s [const char *] filename
** @return [AjBool] true if regular
** @@
******************************************************************************/

AjBool ajSysIsRegular(const char *s)
{
#if defined(AJ_IRIXLF)
    static struct stat64 buf;
#else
    static struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(stat64(s,&buf)==-1) 
	return ajFalse;
#else
    if(stat(s,&buf)==-1)
	return ajFalse;
#endif

    if((ajint)buf.st_mode & AJ_FILE_REG)
	return ajTrue;

    return ajFalse;
}




/* @func ajSysIsDirectory *****************************************************
**
** Test for a directory
**
** @param [r] s [const char *] directory name
** @return [AjBool] true if directory
** @@
******************************************************************************/

AjBool ajSysIsDirectory(const char *s)
{
#if defined(AJ_IRIXLF)
    static struct stat64 buf;
#else
    static struct stat buf;
#endif

#if defined(AJ_IRIXLF)
    if(stat64(s,&buf)==-1)
	return ajFalse;
#else
    if(stat(s,&buf)==-1)
	return ajFalse;
#endif

    if((ajint)buf.st_mode & AJ_FILE_DIR)
	return ajTrue;

    return ajFalse;
}




/* @func ajSysStrtok **********************************************************
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
    static AjPStr rets = NULL;
    static AjPStr sou  = NULL;
    static const char *p = NULL;
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

    return ajStrStrMod(&rets);
}




/* @func ajSysStrtokR *********************************************************
**
** Reentrant strtok that doesn't corrupt the source string
**
** @param [r] s [const char *] source string
** @param [r] t [const char *] delimiter string
** @param [u] ptrptr [char **] ptr save
** @param [w] buf [AjPStr *] result buffer
**
** @return [char*] pointer or NULL
** @@
******************************************************************************/

char* ajSysStrtokR(const char *s, const char *t, char **ptrptr, AjPStr *buf)
{
    const char *p;
    ajint len;

    if(!*buf)
	*buf = ajStrNew();

    if(s!=NULL)
	p = s;
    else
	p = *ptrptr;

    if(!*p)
	return NULL;

    len = strspn(p,t);
    p += len;
    len = strcspn(p,t);
    ajStrAssSubC(buf,p,0,len-1);
    p += len;
    len = strspn(p,t);
    p += len;

    *ptrptr = (char *) p;

    return ajStrStrMod(buf);
}




/* @func ajSysFgets ***********************************************************
**
** An fgets replacement that will cope with Mac OSX <CR> files
**
** @param [w] buf [char *] buffer
** @param [r] size [int] maximum length to read
** @param [u] fp [FILE *] stream
**
** @return [char*] buf or NULL
** @@
******************************************************************************/

char* ajSysFgets(char *buf, int size, FILE *fp)
{
#ifdef __ppc__
    int c;
    char *p;
    int cnt;

    p = buf;
    if(!size || size<0)
        return NULL;

    cnt = 0;

    while(cnt!=size-1)
    {
	c = getc(fp);
	if(c==EOF || c=='\r' || c=='\n')
	    break;
        *(p++) = c;
        ++cnt;
    }


    *p ='\0';

    if(c==EOF && !cnt)
	return NULL;

    if(cnt == size-1)
        return buf;

    if(c=='\r' || c=='\n')
    {
	if(c=='\r' && cnt<size-2)
	{
	    if((c=getc(fp)) == '\n')
		*(p++) = '\r';
	    else
		ungetc(c,fp);
	}
	*(p++) = '\n';
    }

    *p = '\0';

    return buf;
#else
    return fgets(buf,size,fp);
#endif
}




/* @func ajSysFopen ***********************************************************
**
** An fopen replacement to cope with cygwin and windows
**
** @param [r] name [const char *] file to open
** @param [r] flags [const char*] r/w/a flags
**
** @return [FILE*] file or NULL
** @@
******************************************************************************/

FILE* ajSysFopen(const char *name, const char *flags)
{
    FILE   *ret  = NULL;
#ifdef __CYGWIN__
    AjPStr fname = NULL;
#endif
    
#ifdef __CYGWIN__
    if(*(name+1) == ':')
    {
	fname = ajStrNew();
	ajFmtPrintS(&fname,"/cygdrive/%c/%s",*name,name+2);
	ret = fopen(ajStrStr(fname),flags);
	ajStrDel(&fname);
    }
    else
      ret = fopen(name,flags);
#else
	ret = fopen(name,flags);
#endif

	return ret;
}

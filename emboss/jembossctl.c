/******************************************************************************
** @source jembossctl
**
** @author Copyright (C) 2002 Alan Bleasby
** @version 1.0
** @modified Mar 02 2002 ajb First version
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

#ifdef HAVE_JAVA

#include "emboss.h"
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <grp.h>

#include <sys/file.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <limits.h>

#ifdef __hpux
#include <stropts.h>
#endif

#ifdef HAVE_POLL
#include <poll.h>
#endif

#if defined (__SVR4) && defined (__sun)
#include <sys/filio.h>
#endif

#ifndef TOMCAT_UID
#define TOMCAT_UID 506	  /* Set this to be the UID of the tomcat process */
#endif

#define UIDLIMIT 0
#define GIDLIMIT 0


#define TIMEOUT 30	/* Arbitrary pipe timeout (secs)                  */
#define TIMEBUFFER 256	/* Arbitrary length buffer for time printing      */
#define PUTTIMEOUT  120	/* Max no. of secs to write a file                */

#define R_BUFFER 2048   /* Arbitrary length buffer for reentrant syscalls */

static AjBool jctl_up(char *buf,int *uid,int *gid,AjPStr *home);
static AjBool jctl_do_fork(char *buf, int uid, int gid);
static AjBool jctl_do_batch(char *buf, int uid, int gid);
static AjBool jctl_do_directory(char *buf, int uid, int gid);
static AjBool jctl_do_deletefile(char *buf, int uid, int gid);
static AjBool jctl_do_seq(char *buf, int uid, int gid);
static AjBool jctl_do_seqset(char *buf, int uid, int gid);
static AjBool jctl_do_renamefile(char *buf, int uid, int gid);
static AjBool jctl_do_deletedir(char *buf, int uid, int gid);
static AjBool jctl_do_listfiles(char *buf, int uid, int gid, AjPStr *retlist);
static AjBool jctl_do_listdirs(char *buf, int uid, int gid, AjPStr *retlist);
static AjBool jctl_do_getfile(char *buf, int uid, int gid,
			      unsigned char **fbuf, int *size);
static AjBool jctl_do_putfile(char *buf, int uid, int gid);

static char **jctl_make_array(AjPStr str);
static void jctl_tidy_strings(AjPStr *tstr, AjPStr *home, AjPStr *retlist,
			      char *buf);
static void jctl_fork_tidy(AjPStr *cl, AjPStr *prog, AjPStr *enviro,
			   AjPStr *dir, AjPStr *outstd, AjPStr *errstd);
static AjBool jctl_check_buffer(char *buf, int mlen);
static AjBool jctl_chdir(char *file);
static AjBool jctl_initgroups(char *buf, int gid);
static void jctl_zero(char *buf);
static time_t jctl_Datestr(AjPStr s);
static int    jctl_date(const void* str1, const void* str2);

static AjBool jctl_GetSeqFromUsa(AjPStr thys, AjPSeq *seq);
static AjBool jctl_GetSeqsetFromUsa(AjPStr thys, AjPSeqset *seq);


#include <pwd.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#ifndef __ppc__
#include <crypt.h>
#endif

#ifdef N_SHADOW
#include <shadow.h>
#endif
#ifdef R_SHADOW
#include <shadow.h>
#endif
#ifdef HPUX_SHADOW
#include <shadow.h>
#endif

#ifdef PAM
#include <security/pam_appl.h>
#endif

#ifdef AIX_SHADOW
#include <userpw.h>
#endif

#ifdef HPUX_SHADOW
#include <prot.h>
#endif


static void jctl_empty_core_dump(void);
#ifndef NO_AUTH
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			      ajint *gid, AjPStr *home);
#endif

#ifdef PAM
static int jctl_pam_conv(int num_msg, struct pam_message **msg,
			 struct pam_response **resp, void *appdata_ptr);
#endif

#define JBUFFLEN 10000

static int jctl_pipe_read(char *buf, int n, int seconds);
static int jctl_pipe_write(char *buf, int n, int seconds);
static int jctl_snd(char *buf,int len);
static int jctl_rcv(char *buf);

static int java_block(int chan, unsigned long flag);

#ifndef __ppc__
extern char *strptime(const char *s, const char *format, struct tm *tm);
#endif

#if defined (__SVR4) && defined (__sun)
#define exit(a) _exit(a)
#endif



/* @prog jembossctl ***********************************************************
**
** Slave suid program for Jemboss
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr message=NULL;
    char *cbuf=NULL;
    int mlen;
    int command=0;



    int uid;
    int gid;
    AjPStr home=NULL;
    AjBool ok=ajFalse;
    char c='\0';
    AjPStr tstr = NULL;
    AjPStr retlist=NULL;
    unsigned char *fbuf=NULL;
    int size;



    /* Only allow user with the real uid TOMCAT_UID to proceed */
    if(getuid() != TOMCAT_UID)
	exit(-1);


    home = ajStrNew();
    tstr = ajStrNew();
    retlist = ajStrNew();


    if(!(cbuf=(char *)malloc(JBUFFLEN+1)))
    {
	jctl_tidy_strings(&tstr,&home,&retlist,cbuf);
	fprintf(stderr,"jctl buf malloc error (jembossctl)\n");
	fflush(stderr);
	exit(-1);
    }

    bzero((void*)cbuf,JBUFFLEN+1);

    jctl_empty_core_dump();


    message = ajStrNewC("OK");
    if(jctl_snd(ajStrStr(message),ajStrLen(message))==-1)
    {
	jctl_tidy_strings(&tstr,&home,&retlist,cbuf);
	ajStrDel(&message);
	fprintf(stderr,"jctl send error (jembossctl)\n");
	fflush(stderr);
	exit(-1);
    }


    /* Wait for a command from jni */

    if((mlen = jctl_rcv(cbuf))==-1)
    {
	jctl_tidy_strings(&tstr,&home,&retlist,cbuf);
	ajStrDel(&message);
	fprintf(stderr,"jctl command recv error (jembossctl)\n");
	fflush(stderr);
	exit(-1);
    }


    if(!jctl_check_buffer(cbuf,mlen))
    {
	jctl_tidy_strings(&tstr,&home,&retlist,cbuf);
	ajStrDel(&message);
	fprintf(stderr,"jctl bad buffer error (jembossctl)\n");
	fflush(stderr);
	exit(-1);
    }



    if(sscanf(cbuf,"%d",&command)!=1)
    {
	jctl_tidy_strings(&tstr,&home,&retlist,cbuf);
	ajStrDel(&message);
	fprintf(stderr,"jctl sscanf error (jembossctl)\n");
	fflush(stderr);
	exit(-1);
    }


    switch(command)
    {
    case COMM_AUTH:
	ajStrAssC(&tstr,cbuf);
	c='\0';
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    c=1;

 	if((mlen = jctl_snd(&c,1)) < 0)
	{
	    jctl_tidy_strings(&tstr,&home,&retlist,cbuf);
	    ajStrDel(&message);
	    fprintf(stderr,"jctl command send error (auth)\n");
	    fflush(stderr);
	    exit(-1);
	}
	fprintf(stdout,"%s",ajStrStr(home));
	break;

    case EMBOSS_FORK:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);

	if(ok)
	    ok = jctl_do_fork(cbuf,uid,gid);
	break;

    case MAKE_DIRECTORY:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_directory(cbuf,uid,gid);
	break;

    case DELETE_FILE:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_deletefile(cbuf,uid,gid);
	break;

    case DELETE_DIR:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_deletedir(cbuf,uid,gid);
	break;

    case LIST_FILES:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);

	if(ok)
	    ok = jctl_do_listfiles(cbuf,uid,gid,&retlist);

	fprintf(stdout,"%s",ajStrStr(retlist));
	break;

    case LIST_DIRS:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_listdirs(cbuf,uid,gid,&retlist);

	fprintf(stdout,"%s",ajStrStr(retlist));
	break;

    case GET_FILE:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_getfile(cbuf,uid,gid,&fbuf,&size);

	break;

    case PUT_FILE:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_putfile(cbuf,uid,gid);

	break;

    case BATCH_FORK:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);

	if(ok)
	    ok = jctl_do_batch(cbuf,uid,gid);
	break;

    case RENAME_FILE:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_renamefile(cbuf,uid,gid);
	break;


    case SEQ_ATTRIB:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_seq(cbuf,uid,gid);
	break;

    case SEQSET_ATTRIB:
	ajStrAssC(&tstr,cbuf);
	ok = jctl_up(ajStrStr(tstr),&uid,&gid,&home);
	if(ok)
	    ok = jctl_do_seqset(cbuf,uid,gid);
	break;

    default:
	break;
    }


    bzero((void*)cbuf,JBUFFLEN+1);
    ajStrDel(&message);
    jctl_tidy_strings(&tstr,&home,&retlist,cbuf);

    fflush(stdout);
    fflush(stderr);
    exit(0);
    return 0;
}



/* @funcstatic jctl_empty_core_dump *******************************************
**
** Set process coredump size to be zero
**
** @return [void]
** @@
******************************************************************************/

static void jctl_empty_core_dump()
{
    struct rlimit limit;

    limit.rlim_cur = 0;
    limit.rlim_max = 0;

    setrlimit(RLIMIT_CORE,&limit);

    return;
}




#ifdef N_SHADOW
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			      ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct passwd *pwd  = NULL;
    char *p = NULL;


    shadow = getspnam(ajStrStr(username));

    if(!shadow)                 /* No such username */
        return ajFalse;


    pwd = getpwnam(ajStrStr(username));

    if(!pwd)
        return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),shadow->sp_pwdp);

    if(!strcmp(p,shadow->sp_pwdp))
        return ajTrue;

    return ajFalse;
}
#endif



#ifdef AIX_SHADOW
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			      ajint *gid, AjPStr *home)
{
    struct userpw *shadow = NULL;
    struct passwd *pwd  = NULL;
    char *p = NULL;

    shadow = getuserpw(ajStrStr(username));
    if(!shadow)
	return ajFalse;

    pwd = getpwnam(ajStrStr(username));
    if(!pwd)
	return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),shadow->upw_passwd);

    if(!strcmp(p,shadow->upw_passwd))
	return ajTrue;

    return ajFalse;
}
#endif



#ifdef HPUX_SHADOW
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct spwd sresult;
    struct passwd *pwd  = NULL;
    struct passwd presult;
    char *p = NULL;
    char *epwd = NULL;
    char *buf  = NULL;
    int ret=0;
    int trusted;



    trusted = iscomsec();
    if(!(epwd=(char *)malloc(R_BUFFER)))
	return ajFalse;


    if(trusted)
    {
	shadow = getspnam(ajStrStr(username));
	if(!shadow)
	{
	    AJFREE(epwd);
	    return ajFalse;
	}
	strcpy(epwd,shadow->sp_pwdp);
    }


    if(!(buf=(char *)malloc(R_BUFFER)))
	return ajFalse;
    ret = getpwnam_r(ajStrStr(username),&presult,buf,R_BUFFER,&pwd);
    if(ret!=0)
    {
	AJFREE(buf);
	AJFREE(epwd);
	return ajFalse;
    }
    if(!trusted)
	strcpy(epwd,pwd->pw_passwd);

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;
    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),epwd);

    if(!strcmp(p,epwd))
    {
	AJFREE(buf);
	AJFREE(epwd);
	return ajTrue;
    }

    AJFREE(buf);
    AJFREE(epwd);


    return ajFalse;
}
#endif


#ifdef NO_SHADOW
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			      ajint *gid, AjPStr *home)
{
    struct passwd *pwd  = NULL;
    char *p = NULL;

    pwd = getpwnam(ajStrStr(username));
    if(!pwd)		 /* No such username */
	return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),pwd->pw_passwd);

    if(!strcmp(p,pwd->pw_passwd))
	return ajTrue;

    return ajFalse;
}
#endif

#ifdef R_SHADOW
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct spwd sresult;
    struct passwd *pwd  = NULL;
    struct passwd presult;
    char *p = NULL;
    char *sbuf = NULL;
    char *buf  = NULL;
#ifdef _POSIX_C_SOURCE
    int ret=0;
#endif

    if(!(buf=(char*)malloc(R_BUFFER)) || !(sbuf=(char*)malloc(R_BUFFER)))
	return ajFalse;

    shadow = getspnam_r(ajStrStr(username),&sresult,sbuf,R_BUFFER);

    if(!shadow)                 /* No such username */
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }


#ifdef _POSIX_C_SOURCE
    ret = getpwnam_r(ajStrStr(username),&presult,buf,R_BUFFER,&pwd);

    if(ret!=0)
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }
#else
    pwd = getpwnam_r(ajStrStr(username),&presult,buf,R_BUFFER);

    if(!pwd)
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }
#endif

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),shadow->sp_pwdp);

    if(!strcmp(p,shadow->sp_pwdp))
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajTrue;
    }

    AJFREE(buf);
    AJFREE(sbuf);

    return ajFalse;
}
#endif


#ifdef RNO_SHADOW
static AjBool jctl_check_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct passwd *pwd  = NULL;
    char *p = NULL;
    struct passwd result;
    char *buf=NULL;
#if defined(_OSF_SOURCE)
    int  ret=0;
#endif

    if(!(buf=(char *)malloc(R_BUFFER)))
	return ajFalse;

#if defined(_OSF_SOURCE)
    ret = getpwnam_r(ajStrStr(username),&result,buf,R_BUFFER,&pwd);
    if(ret!=0)		 /* No such username */
    {
	AJFREE(buf);
	return ajFalse;
    }
#else
    pwd = getpwnam_r(ajStrStr(username),&result,buf,R_BUFFER);
    if(!pwd)		 /* No such username */
    {
	AJFREE(buf);
	return ajFalse;
    }
#endif

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),pwd->pw_passwd);

    if(!strcmp(p,pwd->pw_passwd))
    {
	AJFREE(buf);
	return ajTrue;
    }

    AJFREE(buf);

    return ajFalse;
}
#endif


#ifdef PAM

struct ad_user
{
    char *username;
    char *password;
};


static int jctl_pam_conv(int num_msg, struct pam_message **msg,
			 struct pam_response **resp, void *appdata_ptr)
{
    struct ad_user *user=(struct ad_user *)appdata_ptr;
    struct pam_response *response;
    int i;

    if (msg == NULL || resp == NULL || user == NULL)
    	return PAM_CONV_ERR;

    response= (struct pam_response *)
    	malloc(num_msg * sizeof(struct pam_response));

    for(i=0;i<num_msg;++i)
    {
	response[i].resp_retcode = 0;
	response[i].resp = NULL;

	switch(msg[i]->msg_style)
	{
	case PAM_PROMPT_ECHO_ON:
	    /* Store the login as the response */
	    response[i].resp = appdata_ptr ?
		(char *)strdup(user->username) : NULL;
	    break;

	case PAM_PROMPT_ECHO_OFF:
	    /* Store the password as the response */
	    response[i].resp = appdata_ptr ?
		(char *)strdup(user->password) : NULL;
	    break;

	case PAM_TEXT_INFO:
	case PAM_ERROR_MSG:
	    break;

	default:
	    if(response)
		free(response);
	    return PAM_CONV_ERR;
	}
    }

    /* On success, return the response structure */
    *resp= response;
    return PAM_SUCCESS;
}


static AjBool jctl_check_pass(AjPStr username,AjPStr password,ajint *uid,
			      ajint *gid,AjPStr *home)
{
    struct ad_user user_info;

    struct pam_cv
    {
	int (*cv)(int,struct pam_message **,struct pam_response **,void *);
	void *userinfo;
    };

    struct pam_cv conv;
    pam_handle_t *pamh = NULL;
    int retval;

    struct passwd *pwd = NULL;

    user_info.username = ajStrStr(username);
    user_info.password = ajStrStr(password);

    conv.cv = jctl_pam_conv;
    conv.userinfo = (void *)&user_info;

    pwd = getpwnam(ajStrStr(username));
    if(!pwd)		 /* No such username */
	return ajFalse;

    *uid = pwd->pw_uid;
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    retval = pam_start("emboss_auth",ajStrStr(username),
		       (struct pam_conv*)&conv,&pamh);

    if (retval == PAM_SUCCESS)
	retval= pam_authenticate(pamh,PAM_SILENT);

    if(retval==PAM_SUCCESS)
	retval = pam_acct_mgmt(pamh,0);

    if(pam_end(pamh,retval)!=PAM_SUCCESS)
    {
	pamh = NULL;
	return ajFalse;
    }

    if(retval==PAM_SUCCESS)
	return ajTrue;

    return ajFalse;
}
#endif


/* @funcstatic jctl_up ********************************************************
**
** Primary username/password check. Return uid/gid/homedir
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int*] uid
** @param [w] gid [int*] gid
** @param [w] home [AjPStr*] home
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_up(char *buf, int *uid, int *gid, AjPStr *home)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr cstr=NULL;
    ajint command;
    AjBool ok=ajFalse;
    char *p=NULL;

    username = ajStrNew();
    password = ajStrNew();
    cstr     = ajStrNew();

    ajStrAssC(&cstr,buf);
    if(ajFmtScanS(cstr,"%d%S%S",&command,&username,&password)!=3)
    {
	if(ajStrLen(username))
	   bzero((void*)ajStrStr(username),ajStrLen(username));
	if(ajStrLen(password))
	   bzero((void*)ajStrStr(password),ajStrLen(password));
	jctl_zero(buf);

	ajStrDel(&username);
	ajStrDel(&password);
	ajStrDel(&cstr);
	return ajFalse;
    }


    p = ajStrStr(cstr);
    while(*p!=' ')
	++p;
    ++p;
    while(*p!=' ')
	++p;
    ++p;
    ajStrAssC(&password,p);


#ifndef NO_AUTH
    ok = jctl_check_pass(username,password,uid,gid,home);
#endif


    bzero((void*)ajStrStr(username),ajStrLen(username));
    bzero((void*)ajStrStr(password),ajStrLen(password));
    jctl_zero(buf);

    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&cstr);


    if((*uid)<UIDLIMIT || (*gid)<GIDLIMIT)
	return ajFalse;
    if(!(*uid) || !(*gid))
	return ajFalse;


    if(ok)
	return ajTrue;

    return ajFalse;
}


/* @funcstatic jctl_do_batch **************************************************
**
** Fork emboss program
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_batch(char *buf, int uid, int gid)
{
    AjPStr cl     = NULL;
    AjPStr prog   = NULL;
    AjPStr enviro = NULL;
    AjPStr dir    = NULL;

    char *p=NULL;
    char *q=NULL;
    char c='\0';

    /* Fork stuff */
    char **argp=NULL;
    char **envp=NULL;
    int  pid;
    int  status = 0;
    int  i=0;

    int  outpipe[2];
    int  errpipe[2];

#ifdef HAVE_POLL
    struct pollfd ufds[2];
    unsigned int  nfds;
#else
    fd_set rec;
    struct timeval t;
#endif

    int nread=0;

    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    int retval=0;
    unsigned long block=0;

    FILE *fp;
#if defined (__SVR4) && defined (__sun) && !defined (__GNUC__)
    struct tm tbuf;
#endif
    struct tm *tp=NULL;
    const time_t tim = time(0);
    char timstr[TIMEBUFFER];

    outstd = ajStrNew();
    errstd = ajStrNew();

    cl     = ajStrNew();
    prog   = ajStrNew();
    enviro = ajStrNew();
    dir    = ajStrNew();


    if(!jctl_initgroups(buf,gid))
    {
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve command line, environment and directory */
    ajStrAssC(&cl,p);
    while(*p)
	++p;
    ++p;

    ajStrAssC(&enviro,p);
    while(*p)
	++p;
    ++p;

    ajStrAssC(&dir,p);

    jctl_zero(buf);


    p = q = ajStrStr(cl);
    while((c=(*p))!=' ' && c && c!='\t' && c!='\n')
      ++p;
    ajStrAssSubC(&prog,q,0,p-q-1);

    argp = jctl_make_array(cl);
    envp = jctl_make_array(enviro);

    if(!ajSysWhichEnv(&prog,envp))
    {
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    while(pipe(outpipe)==-1);
    while(pipe(errpipe)==-1);


#if defined (__SVR4) && defined (__sun)
    pid = fork1();
#else
    pid = fork();
#endif
    if(pid == -1)
    {
	close(errpipe[0]);
	close(errpipe[1]);
	close(outpipe[0]);
	close(outpipe[1]);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    if(!pid)			/* Child */
    {
	dup2(outpipe[1],1);
	dup2(errpipe[1],2);
	if(setgid(gid)==-1)
	{
	    fprintf(stderr,"setgid failure");
	    fflush(stderr);
	    exit(-1);
	}
	if(setuid(uid)==-1)
	{
	    fprintf(stderr,"setuid failure");
	    fflush(stderr);
	    exit(-1);
	}
	if(chdir(ajStrStr(dir))==-1)
	{
	    fprintf(stderr,"chdir failure");
	    fflush(stderr);
	    exit(-1);
	}
	if(execve(ajStrStr(prog),argp,envp) == -1)
	{
	    fprintf(stderr,"execve failure");
	    fflush(stderr);
	    exit(-1);
	}
    }


    /* Tell JNI to continue */
    c=1;
    jctl_snd((char *)&c,1);

    block = 1;
    if(java_block(outpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot unblock 1. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }
    if(java_block(errpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot unblock 2. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }

    *buf = '\0';

#ifdef HAVE_POLL
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	ufds[0].fd = outpipe[0];
	ufds[1].fd = errpipe[0];
	ufds[0].events = POLLIN | POLLPRI;
	ufds[1].events = POLLIN | POLLPRI;
	nfds = 2;
	if(!(retval=poll(ufds,nfds,1)) || retval==-1)
	    continue;

	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&outstd,buf);
	}


	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&errstd,buf);
	}
    }


    ufds[0].fd = outpipe[0];
    ufds[1].fd = errpipe[0];
    ufds[0].events = POLLIN | POLLPRI;
    ufds[1].events = POLLIN | POLLPRI;
    nfds = 2;

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&outstd,buf);
	}

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&errstd,buf);
	}
#else
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(outpipe[0],&rec))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&outstd,buf);
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(errpipe[0],&rec))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&errstd,buf);
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(outpipe[0],&rec))
    {
	while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);
	buf[nread]='\0';
	ajStrAppC(&outstd,buf);
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(errpipe[0],&rec))
    {
	while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);
	buf[nread]='\0';
	ajStrAppC(&errstd,buf);
    }
#endif


    block = 0;
    if(java_block(outpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot block 3. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }
    if(java_block(errpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot block 4. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    close(errpipe[0]);
    close(errpipe[1]);
    close(outpipe[0]);
    close(outpipe[1]);

    i = 0;
    while(argp[i])
	AJFREE(argp[i]);
    AJFREE(argp);

    i = 0;
    while(envp[i])
	AJFREE(envp[i]);
    AJFREE(envp);


    if(setgid(gid)==-1)
    {
	fprintf(stderr,"Setgid error (do_batch)\n");
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    if(setuid(uid)==-1)
    {
	fprintf(stderr,"Setgid error (do_batch)\n");
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    if(chdir(ajStrStr(dir))==-1)
    {
	fprintf(stderr,"chdir error (do_batch)\n");
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


#if defined (__SVR4) && defined (__sun) && !defined (__GNUC__)
    tp = localtime_r(&tim,&tbuf);
#else
    tp = localtime(&tim);
#endif
    strftime(timstr,TIMEBUFFER,"%a %b %d %H:%M:%S %Z %Y",tp);


    if(!(fp=fopen(".finished","w")))
    {
	fprintf(stderr,"fopen error (do_batch)\n");
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }

    fprintf(fp,"%s\n",timstr);
    if(fclose(fp))
    {
	fprintf(stderr,"fclose error (do_batch)\n");
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }

    jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);


    return ajTrue;
}




/* @funcstatic jctl_do_fork ***************************************************
**
** Fork emboss program
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_fork(char *buf, int uid, int gid)
{
    AjPStr cl     = NULL;
    AjPStr prog   = NULL;
    AjPStr enviro = NULL;
    AjPStr dir    = NULL;

    char *p=NULL;
    char *q=NULL;
    char c='\0';

    /* Fork stuff */
    char **argp=NULL;
    char **envp=NULL;
    int  pid;
    int  status = 0;
    int  i=0;

    int  outpipe[2];
    int  errpipe[2];

#ifdef HAVE_POLL
    struct pollfd ufds[2];
    unsigned int  nfds;
#else
    fd_set rec;
    struct timeval t;
#endif

    int nread=0;

    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    int retval=0;
    unsigned long block=0;


    outstd = ajStrNew();
    errstd = ajStrNew();

    cl     = ajStrNew();
    prog   = ajStrNew();
    enviro = ajStrNew();
    dir    = ajStrNew();


    if(!jctl_initgroups(buf,gid))
    {
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve command line, environment and directory */
    ajStrAssC(&cl,p);
    while(*p)
	++p;
    ++p;

    ajStrAssC(&enviro,p);
    while(*p)
	++p;
    ++p;

    ajStrAssC(&dir,p);

    jctl_zero(buf);


    p = q = ajStrStr(cl);
    while((c=(*p))!=' ' && c && c!='\t' && c!='\n')
      ++p;
    ajStrAssSubC(&prog,q,0,p-q-1);

    argp = jctl_make_array(cl);
    envp = jctl_make_array(enviro);

    if(!ajSysWhichEnv(&prog,envp))
    {
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    while(pipe(outpipe)==-1);
    while(pipe(errpipe)==-1);


#if defined (__SVR4) && defined (__sun)
    pid = fork1();
#else
    pid = fork();
#endif
    if(pid == -1)
    {
	close(errpipe[0]);
	close(errpipe[1]);
	close(outpipe[0]);
	close(outpipe[1]);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    if(!pid)			/* Child */
    {
	dup2(outpipe[1],1);
	dup2(errpipe[1],2);
	if(setgid(gid)==-1)
	{
	    fprintf(stderr,"setgid failure");
	    fflush(stderr);
	    exit(-1);
	}
	if(setuid(uid)==-1)
	{
	    fprintf(stderr,"setuid failure");
	    fflush(stderr);
	    exit(-1);
	}
	if(chdir(ajStrStr(dir))==-1)
	{
	    fprintf(stderr,"chdir failure");
	    fflush(stderr);
	    exit(-1);
	}
	if(execve(ajStrStr(prog),argp,envp) == -1)
	{
	    fprintf(stderr,"execve failure");
	    fflush(stderr);
	    exit(-1);
	}
    }


    block = 1;
    if(java_block(outpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot unblock 5. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }
    if(java_block(errpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot unblock 6. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }


    *buf = '\0';

#ifdef HAVE_POLL
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	ufds[0].fd = outpipe[0];
	ufds[1].fd = errpipe[0];
	ufds[0].events = POLLIN | POLLPRI;
	ufds[1].events = POLLIN | POLLPRI;
	nfds = 2;
	if(!(retval=poll(ufds,nfds,1)) || retval==-1)
	    continue;

	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&outstd,buf);
	}


	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&errstd,buf);
	}
    }


    ufds[0].fd = outpipe[0];
    ufds[1].fd = errpipe[0];
    ufds[0].events = POLLIN | POLLPRI;
    ufds[1].events = POLLIN | POLLPRI;
    nfds = 2;

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[0].revents & POLLIN) || (ufds[0].revents & POLLPRI))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&outstd,buf);
	}

    retval=poll(ufds,nfds,1);

    if(retval>0)
	if((ufds[1].revents & POLLIN) || (ufds[1].revents & POLLPRI))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&errstd,buf);
	}
#else
    while((retval=waitpid(pid,&status,WNOHANG))!=pid)
    {
	if(retval==-1)
	    if(errno!=EINTR)
		break;

	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(outpipe[0],&rec))
	{
	    while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&outstd,buf);
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 1000;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(errpipe[0],&rec))
	{
	    while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
		  && errno==EINTR);
	    buf[nread]='\0';
	    ajStrAppC(&errstd,buf);
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(outpipe[0],&rec))
    {
	while((nread = read(outpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);
	buf[nread]='\0';
	ajStrAppC(&outstd,buf);
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(errpipe[0],&rec))
    {
	while((nread = read(errpipe[0],(void *)buf,JBUFFLEN))==-1
	      && errno==EINTR);
	buf[nread]='\0';
	ajStrAppC(&errstd,buf);
    }
#endif


    block = 0;
    if(java_block(outpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot block 7. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }
    if(java_block(errpipe[0],block)==-1)
    {
	fprintf(stderr,"Cannot block 8. %d\n",errno);
	jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);
	return ajFalse;
    }



    fprintf(stdout,"%s",ajStrStr(outstd));
    fprintf(stderr,"%s",ajStrStr(errstd));


    close(errpipe[0]);
    close(errpipe[1]);
    close(outpipe[0]);
    close(outpipe[1]);

    i = 0;
    while(argp[i])
	AJFREE(argp[i]);
    AJFREE(argp);

    i = 0;
    while(envp[i])
	AJFREE(envp[i]);
    AJFREE(envp);

    jctl_fork_tidy(&cl,&prog,&enviro,&dir,&outstd,&errstd);


    return ajTrue;
}



/* @funcstatic jctl_make_array ************************************************
**
** Construct argv and env arrays for Ajax.fork
**
** @param [r] str [AjPStr] space separated tokens
**
** @return [char**] env or argv array
******************************************************************************/

static char** jctl_make_array(AjPStr str)
{
    int n;
    char **ptr=NULL;
    AjPStr buf;
    char   *save=NULL;

    buf = ajStrNew();

    n = ajStrTokenCountR(&str," \t\n");

    AJCNEW0(ptr,n+1);

    ptr[n] = NULL;

    n = 0;

    if(!ajSysStrtokR(ajStrStr(str)," \t\n",&save,&buf))
	return ptr;
    ptr[n++] = ajCharNew(buf);

    while(ajSysStrtokR(NULL," \t\n",&save,&buf))
	ptr[n++] = ajCharNew(buf);

    ajStrDel(&buf);

    return ptr;
}


/* @funcstatic jctl_do_directory **********************************************
**
** Make user directory
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_directory(char *buf, int uid, int gid)
{
    AjPStr dir = NULL;
    AjPStr str = NULL;
    char *p=NULL;
    char *dbuf=NULL;
    int  len=0;


    dir    = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_directory)\n");
	ajStrDel(&dir);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve directory */
    ajStrAssC(&dir,p);


    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	ajStrDel(&dir);
	fprintf(stderr,"setgid error (mkdir)\n");
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	ajStrDel(&dir);
	fprintf(stderr,"setuid error (mkdir)\n");
	return ajFalse;
    }

    if(!jctl_chdir(ajStrStr(dir)))
    {
	ajStrDel(&dir);
	fprintf(stderr,"chdir error (mkdir)\n");
	return ajFalse;
    }


    if(!(dbuf=(char *)malloc((len=ajStrLen(dir))+1)))
	return ajFalse;
    strcpy(dbuf,ajStrStr(dir));

    if(dbuf[len-1]=='/')
	dbuf[len-1]='\0';

    str = ajStrNew();
    ajStrAssC(&str,dbuf);

    if(mkdir(ajStrStr(str),0751)==-1)
    {
	AJFREE(dbuf);
	ajStrDel(&str);
	ajStrDel(&dir);
	fprintf(stderr,"mkdir error (mkdir)\n");
	return ajFalse;
    }


    AJFREE(dbuf);
    ajStrDel(&str);
    ajStrDel(&dir);


    return ajTrue;
}


/* @funcstatic jctl_do_deletefile *********************************************
**
** Delete a user file
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_deletefile(char *buf, int uid, int gid)
{
    AjPStr ufile    = NULL;
    char *p=NULL;


    ufile    = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_deletefile)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve user file */
    ajStrAssC(&ufile,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (delete file)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (delete file)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }

    if(!jctl_chdir(ajStrStr(ufile)))
    {
	fprintf(stderr,"setuid error (delete file)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }

    if(unlink(ajStrStr(ufile))==-1)
    {
	fprintf(stderr,"unlink error (delete file)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }

    ajStrDel(&ufile);


    return ajTrue;
}



/* @funcstatic jctl_do_seq ****************************************************
**
** Get sequence attributes (top level)
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_seq(char *buf, int uid, int gid)
{
    AjPStr usa = NULL;
    char *p=NULL;
    AjPSeq seq = NULL;
    AjBool ok;

    usa    = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_seq)\n");
	ajStrDel(&usa);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve user file */
    ajStrAssC(&usa,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (seq attr)\n");
	ajStrDel(&usa);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (seq attr)\n");
	ajStrDel(&usa);
	return ajFalse;
    }

/*
 *  Might need a kludge for stupid solaris so leave this code here
    if(!jctl_chdir(ajStrStr(usa)))
    {
	fprintf(stderr,"setuid error (seq attr)\n");
	ajStrDel(&usa);
	return ajFalse;
    }
*/

    seq = ajSeqNew();

    ok = jctl_GetSeqFromUsa(usa,&seq);
    if(ok)
	fprintf(stdout,"%d %f %d",(int)ajSeqLen(seq),seq->Weight,
		(int)ajSeqIsNuc(seq));
    else
	fprintf(stdout,"0 0.0 0");
    fflush(stdout);


    ajStrDel(&usa);
    ajSeqDel(&seq);

    if(!ok)
	return ajFalse;

    return ajTrue;
}



/* @funcstatic jctl_do_seqset *************************************************
**
** Get seqset attributes (top level)
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_seqset(char *buf, int uid, int gid)
{
    AjPStr usa = NULL;
    char *p=NULL;
    AjBool ok;
    AjPSeqset seq = NULL;


    usa    = ajStrNew();
    seq    = ajSeqsetNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_seqset)\n");
	ajStrDel(&usa);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve user file */
    ajStrAssC(&usa,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (seqset attrib)\n");
	ajStrDel(&usa);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (seqset attrib)\n");
	ajStrDel(&usa);
	return ajFalse;
    }

    /*
     *  Leave this code here for now in case of Solaris usual stupidity
    if(!jctl_chdir(ajStrStr(usa)))
    {
	fprintf(stderr,"setuid error (seqset attrib)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }
    */


    ok = jctl_GetSeqsetFromUsa(usa,&seq);
    if(ok)
	fprintf(stdout,"%d %f %d",(int)ajSeqsetLen(seq),
		ajSeqsetTotweight(seq),(int)ajSeqsetIsNuc(seq));
    else
	fprintf(stdout,"0 0.0 0");
    fflush(stdout);


    ajStrDel(&usa);
    ajSeqsetDel(&seq);

    if(!ok)
	return ajFalse;

    return ajTrue;
}



/* @funcstatic jctl_do_renamefile *********************************************
**
** Rename a user file
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_renamefile(char *buf, int uid, int gid)
{
    AjPStr ufile    = NULL;
    AjPStr u2file   = NULL;
    char *p=NULL;


    ufile    = ajStrNew();
    u2file   = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_renamefile)\n");
	ajStrDel(&ufile);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve user file */
    ajStrAssC(&ufile,p);

    while(*p)
	++p;
    ++p;
    /* retrieve new name */
    ajStrAssC(&u2file,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (rename file)\n");
	ajStrDel(&ufile);
	ajStrDel(&u2file);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (rename file)\n");
	ajStrDel(&ufile);
	ajStrDel(&u2file);
	return ajFalse;
    }

    if(!jctl_chdir(ajStrStr(ufile)))
    {
	fprintf(stderr,"setuid error (rename file)\n");
	ajStrDel(&ufile);
	ajStrDel(&u2file);
	return ajFalse;
    }

    if(rename(ajStrStr(ufile),ajStrStr(u2file))==-1)
    {
	fprintf(stderr,"unlink error (rename file)\n");
	ajStrDel(&ufile);
	ajStrDel(&u2file);
	return ajFalse;
    }

    ajStrDel(&ufile);
    ajStrDel(&u2file);


    return ajTrue;
}



/* @funcstatic jctl_do_deletedir **********************************************
**
** Recursively delete a user directory
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_deletedir(char *buf, int uid, int gid)
{
    AjPStr dir  = NULL;
    AjPStr cmnd = NULL;
    char *p=NULL;


    dir    = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_deletedir)\n");
	ajStrDel(&dir);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve user directory */
    ajStrAssC(&dir,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (delete directory)\n");
	ajStrDel(&dir);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (delete directory)\n");
	ajStrDel(&dir);
	return ajFalse;
    }

    if(chdir(ajStrStr(dir))==-1)
    {
	fprintf(stderr,"chdir error (delete directory)\n");
	ajStrDel(&dir);
	return ajFalse;
    }

    if(!jctl_chdir(ajStrStr(dir)))
    {
	fprintf(stderr,"jctl_chdir error (delete directory)\n");
	ajStrDel(&dir);
	return ajFalse;
    }


    cmnd = ajStrNew();
    ajFmtPrintS(&cmnd,"rm -rf %S",dir);


#ifndef __ppc__
    if(system(ajStrStr(cmnd))==-1)
    {
	fprintf(stderr,"system error (delete directory)\n");
	ajStrDel(&cmnd);
	ajStrDel(&dir);
	return ajFalse;
    }

#else
    ajSystem(&cmnd);
#endif

    ajStrDel(&cmnd);
    ajStrDel(&dir);


    return ajTrue;
}


/* @funcstatic jctl_do_listfiles **********************************************
**
** Return regular files in a directory
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
** @param [w] retlist [AjPStr*] file list
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_listfiles(char *buf, int uid, int gid,AjPStr *retlist)
{
    AjPStr dir     = NULL;
    AjPStr full    = NULL;

    char *p=NULL;
    DIR  *dirp;
#if defined (HAVE64)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif

#if defined (HAVE64)
    struct stat64 sbuf;
#else
    struct stat sbuf;
#endif

    AjPList list=NULL;
    AjPStr  tstr=NULL;
#if defined (__SVR4) && defined (__sun) && defined (_POSIX_C_SOURCE)
    int ret=0;
#endif
#if defined (__SVR4) && defined (__sun)
    char *dbuf=NULL;

    if(!(dbuf=malloc(sizeof(struct dirent)+PATH_MAX)))
    {
	fprintf(stderr,"Readdir buffer failure (do_listfiles)\n");
	return ajFalse;
    }
#endif

    dir     = ajStrNew();
    full    = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_listfiles)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve user file */
    ajStrAssC(&dir,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (list files)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (list files)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }

    if(chdir(ajStrStr(dir))==-1)
    {
	fprintf(stderr,"chdir error (list files)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }


    if(!(dirp=opendir(ajStrStr(dir))))
    {
	fprintf(stderr,"opendir error (list files)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }

    ajFileDirFix(&dir);

    list = ajListNew();

#if defined (__SVR4) && defined (__sun) && \
    defined (_POSIX_C_SOURCE) && defined (HAVE64)
    for(ret=readdir64_r(dirp,(struct dirent64 *)dbuf,&dp);dp;
	ret=readdir64_r(dirp,(struct dirent64 *)dbuf,&dp))
#else
#if defined (__SVR4) && defined (__sun) && defined (_POSIX_C_SOURCE)
    for(ret=readdir_r(dirp,(struct dirent *)dbuf,&dp);dp;
	ret=readdir_r(dirp,(struct dirent *)dbuf,&dp))
#else
#if defined (__SVR4) && defined (__sun) && !defined (__GNUC__)
    for(dp=readdir_r(dirp,(struct dirent *)dbuf);dp;
	dp=readdir_r(dirp,(struct dirent *)dbuf))
#else
#if defined (HAVE64)
    for(dp=readdir64(dirp);dp;dp=readdir64(dirp))
#else
    for(dp=readdir(dirp);dp;dp=readdir(dirp))
#endif
#endif
#endif
#endif
    {
#if defined (__SVR4) && defined (__sun) && \
    defined (_POSIX_C_SOURCE) && defined (HAVE64)
	if(ret)
	    break;
#endif

	if(*(dp->d_name)=='.')
	    continue;
	ajFmtPrintS(&full,"%S%s",dir,dp->d_name);


#if defined (HAVE64)
	if(stat64(ajStrStr(full),&sbuf)==-1)
	    continue;
#else
	if(stat(ajStrStr(full),&sbuf)==-1)
	    continue;
#endif

	if(sbuf.st_mode & S_IFREG)
	{
	    tstr = ajStrNew();
	    ajStrAppC(&tstr,dp->d_name);
	    ajListPush(list,(void *)tstr);
	}
    }

    ajListSort(list,ajStrCmp);

    while(ajListPop(list,(void **)&tstr))
    {
	ajStrApp(retlist,tstr);
	ajStrAppC(retlist,"\n");
	ajStrDel(&tstr);
    }


    ajListDel(&list);

    ajStrDel(&full);
    ajStrDel(&dir);


#if defined (__SVR4) && defined (__sun)
    AJFREE(dbuf);
#endif

    return ajTrue;
}



/* @funcstatic jctl_do_listdirs ***********************************************
**
** Return directoriy files within a directory
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
** @param [w] retlist [AjPStr*] file list
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_listdirs(char *buf, int uid, int gid,AjPStr *retlist)
{
    AjPStr dir     = NULL;
    AjPStr full    = NULL;

    char *p=NULL;
    DIR  *dirp;
    time_t t;

#if defined (HAVE64)
    struct dirent64 *dp;
#else
    struct dirent *dp;
#endif

#if defined (HAVE64)
    struct stat64 sbuf;
#else
    struct stat sbuf;
#endif


    AjPList list=NULL;
    AjPStr  tstr=NULL;
#if defined (__SVR4) && defined (__sun) && defined (_POSIX_C_SOURCE)
    int ret=0;
#endif
#if defined (__SVR4) && defined (__sun)
    char *dbuf=NULL;

    if(!(dbuf=malloc(sizeof(struct dirent)+PATH_MAX)))
    {
	fprintf(stderr,"Readdir buffer failure (do_listdirs)\n");
	return ajFalse;
    }
#endif


    dir     = ajStrNew();
    full    = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_listdirs)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }



    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;


    /* retrieve directory */
    ajStrAssC(&dir,p);

    jctl_zero(buf);



    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (list dirs)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }


    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (list dirs)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }


    if(chdir(ajStrStr(dir))==-1)
    {
	fprintf(stderr,"chdir error (list dirs)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }


    if(!(dirp=opendir(ajStrStr(dir))))
    {
	fprintf(stderr,"opendir error (list dirs)\n");
	ajStrDel(&dir);
	ajStrDel(&full);
	return ajFalse;
    }

    ajFileDirFix(&dir);

    list = ajListNew();



#if defined (__SVR4) && defined (__sun) && \
    defined (_POSIX_C_SOURCE) && defined (HAVE64)
    for(ret=readdir64_r(dirp,(struct dirent64 *)dbuf,&dp);dp;
	ret=readdir64_r(dirp,(struct dirent64 *)dbuf,&dp))
#else
#if defined (__SVR4) && defined (__sun) && defined (_POSIX_C_SOURCE)
    for(ret=readdir_r(dirp,(struct dirent *)dbuf,&dp);dp;
	ret=readdir_r(dirp,(struct dirent *)dbuf,&dp))
#else
#if defined (__SVR4) && defined (__sun) && !defined (__GNUC__)
    for(dp=readdir_r(dirp,(struct dirent *)dbuf);dp;
	dp=readdir_r(dirp,(struct dirent *)dbuf))
#else
#if defined (HAVE64)
    for(dp=readdir64(dirp);dp;dp=readdir64(dirp))
#else
    for(dp=readdir(dirp);dp;dp=readdir(dirp))
#endif
#endif
#endif
#endif
    {
#if defined (__SVR4) && defined (__sun) && \
    defined (_POSIX_C_SOURCE) && defined (HAVE64)
	if(ret)
	    break;
#endif

	if(*(dp->d_name)=='.')
	    continue;

	ajFmtPrintS(&full,"%S%s",dir,dp->d_name);

#if defined (HAVE64)
	if(stat64(ajStrStr(full),&sbuf)==-1)
	    continue;
#else
	if(stat(ajStrStr(full),&sbuf)==-1)
	    continue;
#endif
	if(sbuf.st_mode & S_IFDIR)
	{
	    tstr = ajStrNew();
	    ajStrAppC(&tstr,dp->d_name);
	    ajListPush(list,(void *)tstr);
	}
    }


    if(ajListLength(list) > 1)
    {
	ajListPop(list,(void **)&tstr);
	ajListPush(list,(void *)tstr);
	t = jctl_Datestr(tstr);
	if(t)
	    ajListSort(list,jctl_date);
	else
	    ajListSort(list,ajStrCmp);
    }



    while(ajListPop(list,(void **)&tstr))
    {
	ajStrApp(retlist,tstr);
	ajStrAppC(retlist,"\n");
	ajStrDel(&tstr);
    }

    ajListDel(&list);

    ajStrDel(&full);
    ajStrDel(&dir);

#if defined (__SVR4) && defined (__sun)
    AJFREE(dbuf);
#endif

    return ajTrue;
}



/* @funcstatic jctl_do_getfile ************************************************
**
** Get a user file
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
** @param [w] fbuf [unsigned char**] file
** @param [w] size [int*] uid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_getfile(char *buf, int uid, int gid,
			      unsigned char **fbuf, int *size)
{
    AjPStr file    = NULL;
    AjPStr message = NULL;

    char *p=NULL;
    char *q=NULL;
#if defined (HAVE64)
    struct stat64 sbuf;
#else
    struct stat sbuf;
#endif
    int n=0;
    int sofar=0;
    int pos=0;
    int fd;
    int sum=0;
    unsigned long block=0;
    long then=0L;
    long now=0L;
    struct timeval tv;

    file     = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	message = ajStrNew();
	ajFmtPrintS(&message,"-1");
	if(jctl_snd(ajStrStr(message),ajStrLen(message)+1)==-1)
	{
	    fprintf(stderr,"get file send error\n");
	    return ajFalse;
	}
	ajStrDel(&message);

	fprintf(stderr,"Initgroups failure (do_getfile)\n");
	ajStrDel(&file);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve file name */
    ajStrAssC(&file,p);

    jctl_zero(buf);

    if(setgid(gid)==-1)
    {
	message = ajStrNew();
	ajFmtPrintS(&message,"-1");
	if(jctl_snd(ajStrStr(message),ajStrLen(message)+1)==-1)
	{
	    fprintf(stderr,"get file send error\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}
	ajStrDel(&message);

	fprintf(stderr,"setgid error (get file)\n");
	ajStrDel(&file);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	message = ajStrNew();
	ajFmtPrintS(&message,"-1");
	if(jctl_snd(ajStrStr(message),ajStrLen(message)+1)==-1)
	{
	    fprintf(stderr,"get file send error\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}
	ajStrDel(&message);

	fprintf(stderr,"setuid error (get file)\n");
	ajStrDel(&file);
	return ajFalse;
    }


    if(!jctl_chdir(ajStrStr(file)))
    {
	message = ajStrNew();
	ajFmtPrintS(&message,"-1");
	if(jctl_snd(ajStrStr(message),ajStrLen(message)+1)==-1)
	{
	    fprintf(stderr,"get file send error\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}
	ajStrDel(&message);

	fprintf(stderr,"chdir error (get file)\n");
	ajStrDel(&file);
	return ajFalse;
    }


#if defined (HAVE64)
    if(stat64(ajStrStr(file),&sbuf)==-1)
    {
	fprintf(stderr,"stat error (get file)\n");
	n = *size = 0;
    }
#else
    if(stat(ajStrStr(file),&sbuf)==-1)
    {
	fprintf(stderr,"stat error (get file)\n");
	n = *size = 0;
    }
#endif
    else
	n = *size = sbuf.st_size;


    message = ajStrNew();
    ajFmtPrintS(&message,"%d",n);
    if(jctl_snd(ajStrStr(message),ajStrLen(message)+1)==-1)
    {
	fprintf(stderr,"get file send error\n");
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }



    if(!n)
    {
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }

    if(!(*fbuf=(unsigned char*)malloc(n)))
    {
	fprintf(stderr,"malloc error (get file)\n");
	ajStrDel(&message);
	ajStrDel(&file);
	return ajFalse;
    }


    if((fd=open(ajStrStr(file),O_RDONLY))==-1)
    {
	fprintf(stderr,"open error (get file)\n");
	ajStrDel(&message);
	ajStrDel(&file);
	return ajFalse;
    }


    block = 1;
    if(java_block(fd,block)==-1)
    {
	fprintf(stderr,"Cannot unblock 9. %d\n",errno);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }


    gettimeofday(&tv,NULL);
    then = tv.tv_sec;

    p = q = (char *)*fbuf;
    while(sum!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then >= TIMEOUT)
	{
	    fprintf(stderr,"getfile TIMEOUT\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}

	while((sofar=read(fd,p,n-(p-q)))==-1 && errno==EINTR);
	if(sofar > 0)
	{
	    sum += sofar;
	    p   += sofar;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }


    block = 0;
    if(java_block(fd,block)==-1)
    {
	fprintf(stderr,"Cannot block 10. %d\n",errno);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }


    if(close(fd)==-1)
    {
	fprintf(stderr,"close error (get file)\n");
	ajStrDel(&message);
	ajStrDel(&file);
	return ajFalse;
    }


    gettimeofday(&tv,NULL);
    then = tv.tv_sec;


    while(pos+JBUFFLEN < n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then >= TIMEOUT)
	{
	    fprintf(stderr,"getfile TIMEOUT\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}

	sofar = fwrite((void*)&(*fbuf)[pos],1,JBUFFLEN,stdout);
	if(sofar > 0)
	{
	    pos += sofar;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
    if(n)
	if(n-pos)
	{
	    while(pos!=n)
	    {
		gettimeofday(&tv,NULL);
		now = tv.tv_sec;
		if(now-then >= TIMEOUT)
		{
		    fprintf(stderr,"getfile TIMEOUT\n");
		    ajStrDel(&file);
		    ajStrDel(&message);
		    return ajFalse;
		}

		sofar = fwrite((void *)&(*fbuf)[pos],1,n-pos,stdout);
		if(sofar > 0)
		{
		    pos += sofar;
		    gettimeofday(&tv,NULL);
		    then = tv.tv_sec;
		}
	    }

	}

    ajStrDel(&file);
    ajStrDel(&message);

    return ajTrue;
}




/* @funcstatic jctl_do_putfile ************************************************
**
** Put a user file
**
** @param [w] buf [char*] socket buffer
** @param [w] uid [int] uid
** @param [w] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_do_putfile(char *buf, int uid, int gid)
{
    int sofar=0;
    int size;
    char *p=NULL;
    int mlen;
    int fd;
    unsigned char *fbuf=NULL;
    AjPStr file;
    struct timeval tv;
    long then;
    long now;
    AjPStr message = ajStrNewC("OK");
    int rval=0;
    unsigned long block=0;
    int sum=0;
    int got=0;



    file     = ajStrNew();

    if(!jctl_initgroups(buf,gid))
    {
	fprintf(stderr,"Initgroups failure (do_putfile)\n");
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }

    /* Skip over authentication stuff */
    p=buf;
    while(*p)
	++p;
    ++p;

    /* retrieve file name */
    ajStrAssC(&file,p);

    jctl_zero(buf);

    if(jctl_snd(ajStrStr(message),2)==-1)
    {
	fprintf(stderr,"jctl OK1 error (jctl_do_putfile)\n");
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }



    rval = jctl_rcv(buf);
    if(rval==-1)
    {
	fprintf(stderr,"jctl recv error (jctl_do_putfile)\n");
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }

    if(sscanf(buf,"%d",&size)!=1)
    {
	fprintf(stderr,"jctl file size read  error (jctl_do_putfile)\n");
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }


    if(jctl_snd(ajStrStr(message),2)==-1)
    {
	fprintf(stderr,"jctl OK2 error (jctl_do_putfile)\n");
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }



    if(size)
    {
	if(!(fbuf=(unsigned char *)malloc(size)))
	{
	    fprintf(stderr,"jctl malloc error (jctl_do_putfile)\n");
	    ajStrDel(&message);
	    ajStrDel(&file);
	    return ajFalse;
	}
    }


    gettimeofday(&tv,NULL);
    then = tv.tv_sec;

    while(sofar != size)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then>PUTTIMEOUT)
	{
	    fprintf(stderr,"jctl timeout error (jctl_do_putfile)\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}



	mlen=jctl_rcv(buf);
	if(mlen==-1)
	{
	    fprintf(stderr,"jctl recv error (jctl_do_putfile)\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}
	if(mlen>0)
	{
	    memcpy((void *)&fbuf[sofar],(const void *)buf,mlen);
	    sofar += mlen;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }





    if(setgid(gid)==-1)
    {
	fprintf(stderr,"setgid error (put file)\n");
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }
    if(setuid(uid)==-1)
    {
	fprintf(stderr,"setuid error (put file)\n");
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }
    if(!jctl_chdir(ajStrStr(file)))
    {
	fprintf(stderr,"chdir error (put file)\n");
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }


    if((fd=open(ajStrStr(file),O_CREAT|O_WRONLY|O_TRUNC,0644))<0)
    {
	fprintf(stderr,"jctl open error (jctl_do_putfile)\n");
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }


    block = 1;
    if(java_block(fd,block)==-1)
    {
	fprintf(stderr,"Cannot unblock 11. %d\n",errno);
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }


    gettimeofday(&tv,NULL);
    then = tv.tv_sec;

    while(sum<size)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then>PUTTIMEOUT)
	{
	    fprintf(stderr,"jctl timeout error (jctl_do_putfile)\n");
	    ajStrDel(&file);
	    ajStrDel(&message);
	    return ajFalse;
	}

	if((got=write(fd,(void *)fbuf,size))>0)
	{
	    sum += got;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }

    block = 0;
    if(java_block(fd,block)==-1)
    {
	fprintf(stderr,"Cannot unblock 12. %d\n",errno);
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }

    if(close(fd)<0)
    {
	fprintf(stderr,"jctl close error (jctl_do_putfile)\n");
	if(size)
	    AJFREE(fbuf);
	ajStrDel(&file);
	ajStrDel(&message);
	return ajFalse;
    }

    if(size)
	AJFREE(fbuf);

    ajStrDel(&file);
    ajStrDel(&message);


    return ajTrue;
}


/* @funcstatic jctl_tidy_strings **********************************************
**
** Deallocate memory
**
** @param [w] tstr [AjPStr*] temp string
** @param [w] home [AjPStr*] home directory
** @param [w] retlist [AjPStr*] filename list
** @param [w] buf [char*] socket buffer
**
** @return [void]
******************************************************************************/

static void jctl_tidy_strings(AjPStr *tstr, AjPStr *home, AjPStr *retlist,
			      char *buf)
{

    ajStrDel(tstr);
    ajStrDel(home);
    ajStrDel(retlist);
    AJFREE(buf);

    return;
}



/* @funcstatic jctl_fork_tidy *************************************************
**
** Deallocate fork memory
**
** @param [w] cl [AjPStr*] command line
** @param [w] prog [AjPStr*] program name
** @param [w] enviro [AjPStr*] environment
** @param [w] dir [AjPStr*] directory
** @param [w] outstd [AjPStr*] stdout
** @param [w] errstd [AjPStr*] stderr
**
** @return [void]
******************************************************************************/

static void jctl_fork_tidy(AjPStr *cl, AjPStr *prog, AjPStr *enviro,
			   AjPStr *dir, AjPStr *outstd, AjPStr *errstd)
{

    ajStrDel(cl);
    ajStrDel(prog);
    ajStrDel(enviro);
    ajStrDel(dir);
    ajStrDel(outstd);
    ajStrDel(errstd);

    return;
}


/* @funcstatic jctl_check_buffer **********************************************
**
** Sanity check on socket commands
**
** @param [r] buf [char*] socket buffer
** @param [r] mlen [int] buffer length
**
** @return [AjBool] true if sane
******************************************************************************/

static AjBool jctl_check_buffer(char *buf, int mlen)
{
    char *p;
    int str1len;
    int command;
    int count;

    if(mlen==JBUFFLEN)
	return ajFalse;
    buf[mlen]='\0';

    /* get the first string and check for reasonable length */
    p = buf;
    while(*p)
	++p;

    /* Command, username & password shouldn't be >50 characters */
    str1len = p-buf+1;
    if(str1len > 50)
	return ajFalse;

    if(sscanf(buf,"%d",&command)!=1)
	return ajFalse;

    if(command<COMM_AUTH || command>SEQSET_ATTRIB)
	return ajFalse;

    if(command==COMM_AUTH)
	return ajTrue;

    count = str1len;

    while(*p && count<JBUFFLEN)
    {
	++p;
	++count;
    }
    if(count==JBUFFLEN)
	return ajFalse;

    /* All commands except the fork have two strings */
    if((command != EMBOSS_FORK) && (command!=BATCH_FORK) &&
       (command!=RENAME_FILE))
	return ajTrue;

    /* Check for valid third string */
    ++p;
    ++count;
    while(*p && count<JBUFFLEN)
    {
	++p;
	++count;
    }
    if(count==JBUFFLEN)
	return ajFalse;

    if(command==RENAME_FILE)
        return ajTrue;

    /* Check for valid fourth string */
    ++p;
    ++count;
    while(*p && count<JBUFFLEN)
    {
	++p;
	++count;
    }
    if(count==JBUFFLEN)
	return ajFalse;

    return ajTrue;
}



/* @funcstatic jctl_chdir *****************************************************
**
** If a filename is given (e.g. delete) then first chdir to the directory
**
** @param [r] file [char*] file name
**
** @return [AjBool] true if success
******************************************************************************/
static AjBool jctl_chdir(char *file)
{
    char *p;
    AjPStr str=NULL;
    int ret;
    char *buf;
    int  len=0;

    if(!(buf=(char *)malloc((len=strlen(file))+1)))
	return ajFalse;
    strcpy(buf,file);

    if(buf[len-1]=='/')
	buf[len-1]='\0';

    str = ajStrNew();
    if(!(p=strrchr(buf,(int)'/')))
	ajStrAssC(&str,".");
    else
	ajStrAssSubC(&str,buf,0,p-buf);

    ret = chdir(ajStrStr(str));
    ajStrDel(&str);
    AJFREE(buf);

    if(ret==-1)
	return ajFalse;

    return ajTrue;
}


/* @funcstatic jctl_initgroups ************************************************
**
** Initialise groups
**
** @param [r] buf [char*] socket buffer
** @param [r] gid [int] gid
**
** @return [AjBool] true if success
******************************************************************************/

static AjBool jctl_initgroups(char *buf, int gid)
{
    AjPStr str=NULL;
    AjPStr user=NULL;

    str = ajStrNewC(buf);
    user = ajStrNew();
    ajFmtScanS(str,"%*d%S",&user);
    ajStrDel(&str);

    if(initgroups(ajStrStr(user),gid)==-1)
    {
	ajStrDel(&user);
	return ajFalse;
    }
    ajStrDel(&user);

    return ajTrue;
}


/* @funcstatic jctl_zero ******************************************************
**
** Wipe username/password
**
** @param [r] buf [char*] socket buffer
**
** @return [void]
******************************************************************************/

static void jctl_zero(char *buf)
{
    char *p;

    p=buf;
    while(*p)
	*p++ = '\0';

    return;
}



/* @funcstatic jctl_pipe_read *************************************************
**
** Read a byte stream from stdin (unblocked)
**
** @param [r] buf [char *] buffer to read
** @param [r] n [int] number of bytes to read
** @param [r] seconds [int] time-out
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int jctl_pipe_read(char *buf, int n, int seconds)
{
#ifdef HAVE_POLL
    struct pollfd ufds;
    unsigned int  nfds;
#else
    fd_set fdr;
    fd_set fdw;
    struct timeval tfd;
#endif

    int  sum;
    int  got=0;
    int  ret=0;
    char *p;
    int  rchan=0;
    unsigned long block=0;
    long then = 0;
    long now  = 0;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    then = tv.tv_sec;


    block = 1;
    if(java_block(rchan,block)==-1)
    {
	fprintf(stderr,"Cannot unblock 13. %d\n",errno);
	return -1;
    }


    p = buf;
    sum = 0;


#ifdef HAVE_POLL
    while(sum!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then >= seconds)
	{
	    fprintf(stderr,"jctl_pipe_read timeout\n");
	    return -1;
	}

	/* Check pipe is readable */
	ufds.fd = rchan;
	ufds.events = POLLIN | POLLPRI;
	nfds = 1;

	ret=poll(&ufds,nfds,1);

	if(ret && ret!=-1)
	{
	    if((ufds.revents & POLLIN) || (ufds.revents & POLLPRI))
	    {
		while((got=read(rchan,p,n-(p-buf)))==-1 && errno==EINTR);
		if(got == -1)
		{
		    fprintf(stderr,"jctl_pipe_read read error\n");
		    return -1;
		}
		sum += got;
		p += got;
		gettimeofday(&tv,NULL);
		then = tv.tv_sec;
	    }
	}
    }
#else
    while(sum!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then >= seconds)
	{
	    fprintf(stderr,"jctl_pipe_read timeout\n");
	    return -1;
	}

	/* Check pipe is readable */
	tfd.tv_sec  = 0;
	tfd.tv_usec = 1000;
	FD_ZERO(&fdr);
	FD_SET(rchan,&fdr);
	fdw = fdr;

	ret = select(rchan+1,&fdr,&fdw,NULL,&tfd);

	if(ret && ret!=-1 && FD_ISSET(rchan,&fdr))
	{
	    while((got=read(rchan,p,n-(p-buf)))==-1 && errno==EINTR);
	    if(got == -1)
	    {
		fprintf(stderr,"jctl_pipe_read read error\n");
		return -1;
	    }
	    sum += got;
	    p += got;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
#endif

    block = 0;
    if(java_block(rchan,block)==-1)
    {
	fprintf(stderr,"Cannot block 14. %d\n",errno);
	return -1;
    }


    return 0;
}




/* @funcstatic jctl_pipe_write ************************************************
**
** Write a byte stream to stdout (unblocked)
**
** @param [r] buf [char *] buffer to write
** @param [r] n [int] number of bytes to write
** @param [r] seconds [int] time-out
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int jctl_pipe_write(char *buf, int n, int seconds)
{
#ifdef HAVE_POLL
    struct pollfd ufds;
    unsigned int  nfds;
#else
    fd_set fdr;
    fd_set fdw;
    struct timeval tfd;
#endif

    int  written;
    int  sent=0;
    int  ret=0;
    char *p;
    int tchan=1;
    unsigned long block=0;
    long then = 0;
    long now  = 0;
    struct timeval tv;

    gettimeofday(&tv,NULL);
    then = tv.tv_sec;


    block = 1;
    if(java_block(tchan,block)==-1)
    {
	fprintf(stderr,"Cannot unblock 15. %d\n",errno);
	return -1;
    }


    p = buf;
    written = 0;

#ifdef HAVE_POLL
    while(written!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then >= seconds)
	{
	    fprintf(stderr,"jctl_pipe_write timeout\n");
	    return -1;
	}

	/* Check pipe is writeable */
	ufds.fd = tchan;
	ufds.events = POLLOUT;
	nfds = 1;
	ret=poll(&ufds,nfds,1);

	if(ret && ret!=-1 && (ufds.revents & POLLOUT))
	{
	    while((sent=write(tchan,p,n-(p-buf)))==-1 && errno==EINTR);
	    if(sent == -1)
	    {
		fprintf(stderr,"jctl_pipe_write send error\n");
		return -1;
	    }
	    written += sent;
	    p += sent;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
#else
    while(written!=n)
    {
	gettimeofday(&tv,NULL);
	now = tv.tv_sec;
	if(now-then >= seconds)
	{
	    fprintf(stderr,"jctl_pipe_write timeout\n");
	    return -1;
	}

	/* Check pipe is writeable */
	tfd.tv_sec  = 0;
	tfd.tv_usec = 1000;
	FD_ZERO(&fdw);
	FD_SET(tchan,&fdw);
	fdr = fdw;

	ret = select(tchan+1,&fdr,&fdw,NULL,&tfd);

	if(ret && ret!=-1 && FD_ISSET(tchan,&fdw))
	{
	    while((sent=write(tchan,p,n-(p-buf)))==-1 && errno==EINTR);
	    if(sent == -1)
	    {
		fprintf(stderr,"jctl_pipe_write send error\n");
		return -1;
	    }
	    written += sent;
	    p += sent;
	    gettimeofday(&tv,NULL);
	    then = tv.tv_sec;
	}
    }
#endif

    block = 0;
    if(java_block(tchan,block)==-1)
    {
	fprintf(stderr,"Cannot block 16. %d\n",errno);
	return -1;
    }



    return 0;
}


/* @funcstatic jctl_snd *******************************************************
**
** Mimic socket write using pipes
**
** @param [r] buf [char *] buffer to write
** @param [r] len [int] number of bytes to write
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int jctl_snd(char *buf,int len)
{

    if(jctl_pipe_write((char *)&len,sizeof(int),TIMEOUT)==-1)
    {
	fprintf(stderr,"jctl_snd error\n");
	return -1;
    }
    if(jctl_pipe_write(buf,len,TIMEOUT)==-1)
    {
	fprintf(stderr,"jctl_snd error\n");
	return -1;
    }

    return 0;
}


/* @funcstatic jctl_rcv *******************************************************
**
** Mimic socket read using pipes
**
** @param [r] buf [char *] buffer for read
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int jctl_rcv(char *buf)
{
    int len;

    if(jctl_pipe_read((char *)&len,sizeof(int),TIMEOUT)==-1)
    {
	fprintf(stderr,"jctl_rcv error\n");
	return -1;
    }
    if(jctl_pipe_read(buf,len,TIMEOUT)==-1)
    {
	fprintf(stderr,"jctl_rcv error\n");
	return -1;
    }

    return len;
}



/* @funcstatic java_block *****************************************************
**
** File descriptor block/unblock
**
** @param [r] chan [int] file descriptor
** @param [r] flag [unsigned long] block=1 unblock=0
**
** @return [int] 0=success  -1=failure
** @@
******************************************************************************/

static int java_block(int chan, unsigned long flag)
{

    if(ioctl(chan,FIONBIO,&flag)==-1)
    {
#ifdef __sgi
	if(errno==ENOSYS)
	    return 0;
#endif
#ifdef __hpux
	if(errno==ENOTTY)
	    return 0;
#endif
	return -1;
    }

    return 0;
}


/* @funcstatic jctl_Datestr ***************************************************
**
** Test string for valid Jemboss date. Return time_t
** or 0 if invalid string
**
** @param [r] s [AjPStr] potential date string
**
** @return [time_t] failure=0
** @@
******************************************************************************/

static time_t jctl_Datestr(AjPStr s)
{
    AjPStr tmp = NULL;
    struct tm tm;
    char *p = NULL;
    AjPStr mon=NULL;
    ajint day=0;
    ajint hr=0;
    ajint min=0;
    ajint sec=0;
    ajint yr=0;
#ifdef __ppc__
    ajint i;
    static char *ms[] =
    {
	"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct",
	"Nov","Dec"
    };
    static char *jtz="GMT";
#endif

    tmp = ajStrNew();
    ajStrAssS(&tmp,s);
    p = ajStrStr(tmp);
    while(*p)
    {
	if(*p == '_')
	    *p=' ';
	++p;
    }

    mon = ajStrNew();
    if(ajFmtScanS(tmp,"%*s %*s %S %d %d:%d:%d %*s %d",&mon,&day,&hr,&min,
		  &sec,&yr) !=  6)
    {
	ajStrDel(&mon);
	return 0;
    }


#ifndef __ppc__
    ajFmtPrintS(&tmp,"%S %d %d:%d:%d %d",mon,day,hr,min,sec,yr);
    ajStrDel(&mon);

    p = strptime(ajStrStr(tmp),"%B %d %T %Y",&tm);
    ajStrDel(&tmp);

    if(!p)
	return 0;
#else
    i=0;
    while(i<=11)
    {
	if(!strcmp(ajStrStr(mon),ms[i]))
	    break;
	++i;
    }
    if(i==12)
	i=11;

    tm.tm_mon = i;
    tm.tm_mday = day;
    tm.tm_sec = sec;
    tm.tm_min = min;
    tm.tm_hour = hr;
    tm.tm_year = yr - 1900;
    tm.tm_isdst = 0;
    tm.tm_gmtoff = 0;
    tm.tm_zone = jtz;
#endif

    return mktime(&tm);
}


/* @funcstatic jctl_date ******************************************************
**
** Date comparison for ajListSort
**
** @param [r] str1 [const void*] date string
** @param [r] str2 [const void*] date string
**
** @return [int] comparison
** @@
******************************************************************************/

static int jctl_date(const void* str1, const void* str2)
{
    AjPStr *a = (AjPStr*)str1;
    AjPStr *b = (AjPStr*)str2;

    return (int)(jctl_Datestr(*b) - jctl_Datestr(*a));
}



/* @funcstatic jctl_GetSeqFromUsa *********************************************
**
** Return a sequence given a USA
**
** @param [r] thys [AjPStr] usa
** @param [w] seq [AjPSeq*] sequence
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool jctl_GetSeqFromUsa(AjPStr thys, AjPSeq *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    ajNamInit("emboss");

    seqin = ajSeqinNew();
    seqin->multi = ajFalse;
    seqin->Text  = ajFalse;

    ajSeqinUsa (&seqin, thys);
    ok = ajSeqRead(*seq, seqin);
    ajSeqinDel (&seqin);

    if(!ok)
	return ajFalse;

    return ajTrue;
}



/* @funcstatic jctl_GetSeqsetFromUsa ******************************************
**
** Return a seqset given a usa
**
** @param [r] thys [AjPStr] usa
** @param [w] seq [AjPSeqset*] seqset
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool jctl_GetSeqsetFromUsa(AjPStr thys, AjPSeqset *seq)
{
    AjPSeqin seqin;
    AjBool ok;

    ajNamInit("emboss");

    seqin = ajSeqinNew();
    seqin->multi = ajTrue;
    seqin->Text  = ajFalse;

    ajSeqinUsa (&seqin, thys);
    ok = ajSeqsetRead(*seq, seqin);
    ajSeqinDel (&seqin);


    if(!ok)
	return ajFalse;

    return ajTrue;
}




#else
#include <stdio.h>

int main()
{
    return 0;
}
#endif

/********************************************************************
** @source AJAX Jave Native Interface (JNI) functions
**
** @author Copyright (C) 2001 Alan Bleasby
** @version 1.0 
** @version 2.0 Added Jemboss suid authorisation functions
** @modified Jul 07 2001 ajb First version
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
********************************************************************/

#ifdef HAVE_JAVA

#include <jni.h>
#include "ajax.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>


#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <ctype.h>

#include <pwd.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <crypt.h>

#define R_BUFFER 2048		/* Reentrant buffer size */

#ifdef N_SHADOW
#include <shadow.h>
#endif
#ifdef R_SHADOW
#include <shadow.h>
#endif

#ifdef PAM
#include <security/pam_appl.h>
#endif

#ifdef AIX_SHADOW
#include <userpw.h>
#endif

#ifdef HPUX_SHADOW
#include <hpsecurity.h>
#include <prot.h>
#endif


#define AJ_OUTBUF 10000
#define AJNOTFOUND -999

#define UIDLIMIT 0
#define GIDLIMIT 0


/*
 *  The following prototypes are used by the root tomcat authorisation
 *  system.
 */

static void java_core_dump(void);
#ifndef NO_AUTH
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			 ajint *gid, AjPStr *home);
#endif


#ifdef PAM
static int PAM_conv(int num_msg, struct pam_message **msg,
		    struct pam_response **resp, void *appdata_ptr);
#endif



/*
 *  The following prototypes are used by the jembossctl authorisation
 *  system
 */
static char **make_array(AjPStr str);
static void java_tidy_command(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			      AjPStr *dir, AjPStr *envi, AjPStr *prog);
static void java_tidy_command2(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf);
static void java_tidy_command3(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf, char *cuniq, int sockdes,
			       int *outpipe, int *errpipe);


static AjPStr java_uniqueName(AjPStr username, ajint command);

static ajint java_send_auth(int conndes, char *cuser, char *cpass,
			    AjPStr *errstd);
static ajint java_emboss_fork(int conndes,char *cuser,char *cpass,
			      AjPStr clemboss, AjPStr enviro, AjPStr dir,
			      char *buf, AjPStr *errstd);
static ajint java_make_dir(int conndes, char *cuser, char *cpass, AjPStr dir,
			   char *buf, AjPStr *errstd);
static ajint java_delete_file(int conndes, char *cuser, char *cpass,
			      AjPStr ufile, char *buf, AjPStr *errstd);
static ajint java_delete_dir(int conndes, char *cuser, char *cpass, AjPStr dir,
			     char *buf, AjPStr *errstd);
static ajint java_list_files(int conndes, char *cuser, char *cpass, AjPStr dir,
			     char *buf, AjPStr *errstd);
static ajint java_list_dirs(int conndes, char *cuser, char *cpass, AjPStr dir,
			    char *buf, AjPStr *errstd);
static ajint java_get_file(int conndes, char *cuser, char *cpass, AjPStr file,
			   char *buf, unsigned char **fbuf,int *size,
			   AjPStr *errstd);
static ajint java_put_file(int conndes,char *cuser,char *cpass,AjPStr file,
			   char *buf,unsigned char *fbuf,int size,
			   AjPStr *errstd);

static void java_wait_for_term(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf);
static void java_wait_for_file(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf,
			       unsigned char *fbuf,int size);

static int java_jembossctl(ajint command, AjPStr username, AjPStr password,
			   AjPStr environment,
			   AjPStr str1, AjPStr str2,
			   AjPStr *outstd, AjPStr *errstd,
			   unsigned char **fbuf, int *fsize);


static AjBool ajJavaGetSeqFromUsa (AjPStr thys, AjPSeq *seq);
static AjBool ajJavaGetSeqsetFromUsa (AjPStr thys, AjPSeqset *seq);




/* @func Ajax.seqType **************************************************
**
** Return length, weight and type information for a sequence
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] usa [jstring] usa
**
** @return [jboolean] true if the sequence exists
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_seqType
           (JNIEnv *env, jobject obj, jstring usa)
{

    AjPStr name=NULL;
    char *javaname=NULL;
    AjBool ok=ajFalse;
    AjPSeq seq=NULL;
    AjBool nuc=ajFalse;
    ajint  len=0;
    float  weight=0.;
    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    
    name = ajStrNew();
    seq  = ajSeqNew();
    
    javaname = (char *) (*env)->GetStringUTFChars(env,usa,0);
    ajStrAssC(&name,javaname);
    (*env)->ReleaseStringUTFChars(env,usa,javaname);

    ok = ajJavaGetSeqFromUsa(name,&seq);
    if(ok)
    {
	len = ajSeqLen(seq);
	nuc = ajSeqIsNuc(seq);
	weight = seq->Weight;
	
	field = (*env)->GetStaticFieldID(env,jvc,"length","I");
	(*env)->SetStaticIntField(env,jvc,field,len);

	field = (*env)->GetStaticFieldID(env,jvc,"protein","Z");
	(*env)->SetStaticBooleanField(env,jvc,field,(unsigned char)!nuc);

	field = (*env)->GetStaticFieldID(env,jvc,"weight","F");
	(*env)->SetStaticFloatField(env,jvc,field,weight);
    }
    
    ajStrDel(&name);
    ajSeqDel(&seq);

    return (unsigned char) ok;
}

/* @func Ajax.seqsetType **************************************************
**
** Return length, weight and type information for a seqset
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] usa [jstring] usa
**
** @return [jboolean] true if the sequences exist
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_seqsetType
           (JNIEnv *env, jobject obj, jstring usa)
{
    AjPStr name=NULL;
    char *javaname=NULL;
    AjBool ok=ajFalse;
    AjPSeqset seq=NULL;
    AjBool nuc=ajFalse;
    ajint  len=0;
    float  weight=0.;
    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    
    name = ajStrNew();
    seq  = ajSeqsetNew();
    
    javaname = (char *) (*env)->GetStringUTFChars(env,usa,0);
    ajStrAssC(&name,javaname);
    (*env)->ReleaseStringUTFChars(env,usa,javaname);

    ok = ajJavaGetSeqsetFromUsa(name,&seq);
    if(ok)
    {
	len = ajSeqsetLen(seq);
	nuc = ajSeqsetIsNuc(seq);
	weight = ajSeqsetTotweight(seq);
	
	field = (*env)->GetStaticFieldID(env,jvc,"length","I");
	(*env)->SetStaticIntField(env,jvc,field,len);

	field = (*env)->GetStaticFieldID(env,jvc,"protein","Z");
	(*env)->SetStaticBooleanField(env,jvc,field,(unsigned char)!nuc);

	field = (*env)->GetStaticFieldID(env,jvc,"weight","F");
	(*env)->SetStaticFloatField(env,jvc,field,weight);
    }
    
    ajStrDel(&name);
    ajSeqsetDel(&seq);
    
    return (unsigned char) ok;
}



/* @funcstatic ajJavaGetSeqFromUsa ********************************************
**
** Return a sequence given a USA
**
** @param [r] thys [AjPStr] usa
** @param [w] seq [AjPSeq*] sequence
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool ajJavaGetSeqFromUsa(AjPStr thys, AjPSeq *seq)
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



/* @funcstatic ajJavaGetSeqsetFromUsa *****************************************
**
** Return a seqset given a usa
**
** @param [r] thys [AjPStr] usa
** @param [w] seq [AjPSeqset*] seqset
** @return [AjBool] ajTrue on success
******************************************************************************/

static AjBool ajJavaGetSeqsetFromUsa(AjPStr thys, AjPSeqset *seq)
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


/* @func Ajax.userInfo **************************************************
**
** Return the uid, gid and home directory of a user
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
**
** @return [jboolean] true if the username/password are valid
** @@
******************************************************************************/


JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_userInfo
(JNIEnv *env, jobject obj, jstring door, jstring key)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr home=NULL;
    char *juser=NULL;
    char *jpass=NULL;
    AjBool ok=ajFalse;

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  jhpstr;

    ajint uid=0;
    ajint gid=0;
    char  *hp=NULL;
    
    java_core_dump();
    
    username = ajStrNew();
    password = ajStrNew();
    home     = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);

    jpass = (char *) (*env)->GetStringUTFChars(env,key,0);
    if(jpass)
	ajStrAssC(&password,jpass);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,key,jpass);

    if(!ajStrLen(username) || !ajStrLen(password))
	return (unsigned char)ajFalse;

#ifndef NO_AUTH
    ok = java_pass(username,password,&uid,&gid,&home);
#endif

    field = (*env)->GetStaticFieldID(env,jvc,"uid","I");
    (*env)->SetStaticIntField(env,jvc,field,uid);

    field = (*env)->GetStaticFieldID(env,jvc,"gid","I");
    (*env)->SetStaticIntField(env,jvc,field,gid);


    hp = ajStrStr(home);
    field = (*env)->GetFieldID(env,jvc,"home","Ljava/lang/String;");    

    jhpstr = (*env)->NewStringUTF(env,hp);
    (*env)->SetObjectField(env,obj,field,jhpstr);
    

    bzero((void*)ajStrStr(username),ajStrLen(username));
    bzero((void*)ajStrStr(password),ajStrLen(password));
    bzero((void*)ajStrStr(home),ajStrLen(home));

    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&home);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}




/* @funcstatic java_core_dump ***********************************************
**
** Set process coredump size to be zero
**
** @return [void]
** @@
******************************************************************************/

static void java_core_dump()
{
    struct rlimit limit;

    limit.rlim_cur = 0;
    limit.rlim_max = 0;

    setrlimit(RLIMIT_CORE,&limit);

    return;
}




#ifdef N_SHADOW
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
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



#ifdef R_SHADOW
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct spwd *shadow = NULL;
    struct spwd sresult;
    struct passwd *pwd  = NULL;
    struct passwd presult;
    char *p = NULL;
    char *sbuf = NULL;
    char *buf  = NULL;

    if(!(buf=(char*)malloc(R_BUFFER)) || !(sbuf=(char*)malloc(R_BUFFER)))
	return ajFalse;

    shadow = getspnam_r(ajStrStr(username),&sresult,sbuf,R_BUFFER);
    
    if(!shadow)                 /* No such username */
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }
    
    

    pwd = getpwnam_r(ajStrStr(username),&presult,buf,R_BUFFER);
    
    if(!pwd)
    {
	AJFREE(buf);
	AJFREE(sbuf);
        return ajFalse;
    }
    
    
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



#ifdef AIX_SHADOW
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
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
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct pr_passwd *shadow = NULL;
    struct passwd pwd;
    
    char *p = NULL;

    shadow = getprpwnam(ajStrStr(username));
    if(!shadow)	
	return ajFalse;

    *uid = shadow->ufld.fd_uid;

    
    pwd = getpwnam(ajStrStr(username));
    if(!pwd)
	return ajFalse;
    
    *gid = pwd->pw_gid;

    ajStrAssC(home,pwd->pw_dir);

    p = crypt(ajStrStr(password),shadow->ufld.fd_encrypt);

    if(!strcmp(p,shadow->ufld.fd_encrypt))
	return ajTrue;

    return ajFalse;
}
#endif


#ifdef NO_SHADOW
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
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



#ifdef RNO_SHADOW
static AjBool java_pass(AjPStr username, AjPStr password, ajint *uid,
			ajint *gid, AjPStr *home)
{
    struct passwd *pwd  = NULL;
    char *p = NULL;
    struct passwd result;
    char *buf=NULL;

    if(!(buf=(char *)malloc(R_BUFFER)))
	return ajFalse;
    
    pwd = getpwnam_r(ajStrStr(username),&result,buf,R_BUFFER);
    if(!pwd)		 /* No such username */
    {
	AJFREE(buf);
	return ajFalse;
    }

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


static int PAM_conv(int num_msg, struct pam_message **msg,
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


static AjBool java_pass(AjPStr username,AjPStr password,ajint *uid,
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

    conv.cv = PAM_conv;
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



/* @func Ajax.setuid **************************************************
**
** Set the uid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] uid [jint] uid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_setuid
(JNIEnv *env, jclass j, jint uid)
{
    return((jint)setuid((uid_t)uid));
}


/* @func Ajax.seteuid **************************************************
**
** Set the effective uid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] uid [jint] uid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_seteuid
(JNIEnv *env, jclass j, jint uid)
{
    return((jint)seteuid((uid_t)uid));
}


/* @func Ajax.setgid **************************************************
**
** Set the gid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] gid [jint] gid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_setgid
(JNIEnv *env, jclass j, jint gid)
{
    return((jint)setgid((gid_t)gid));
}


/* @func Ajax.setegid **************************************************
**
** Set the effective gid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
** @param [r] gid [jint] gid
**
** @return [jint] 0 on success, -1 if failure
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_setegid
(JNIEnv *env, jclass j, jint gid)
{
    return((jint)setegid((gid_t)gid));
}


/* @func Ajax.getuid **************************************************
**
** Return the uid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] uid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_getuid
(JNIEnv *env, jclass j)
{
    return((jint)getuid());
}


/* @func Ajax.getgid **************************************************
**
** Return the gid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] gid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_getgid
(JNIEnv *env, jclass j)
{
    return((jint)getgid());
}


/* @func Ajax.geteuid **************************************************
**
** Return the effective uid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] uid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_geteuid
(JNIEnv *env, jclass j)
{
    return((jint)geteuid());
}



/* @func Ajax.getegid **************************************************
**
** Return the effective gid of the current process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] j [jclass] java class
**
** @return [jint] gid
** @@
******************************************************************************/

JNIEXPORT jint JNICALL Java_org_emboss_jemboss_parser_Ajax_getegid
(JNIEnv *env, jclass j)
{
    return((jint)getegid());
}



/* @func Ajax.fork **************************************************
**
** Fork off new process with given uid & gid, chdir and execute command
** return stdout and stderr of the child process
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] commandline [jstring] command to execute
** @param [r] directory [jstring] chdir here before execution
** @param [r] uid [jint] uid for setuid
** @param [r] gid [jint] gid for setgid
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_fork
(JNIEnv *env, jobject obj, jstring commandline,
 jstring environment, jstring directory, jint uid, jint gid)
{
    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    
    char  *sptr;

    AjPStr prog=NULL;
    AjPStr cl=NULL;
    AjPStr envi=NULL;
    AjPStr dir=NULL;
    AjPStrTok handle=NULL;
    

    char **argp=NULL;
    char **envp=NULL;
    int  pid;
    int  status = 0;
    int  i=0;

    int  outpipe[2];
    int  errpipe[2];
    
    fd_set rec;
    struct timeval t;
    int nread=0;
    char *buf;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    int retval=0;

/*
    if(!uid || !gid)
	return (unsigned char)ajFalse;
*/
    if(uid<UIDLIMIT)
	return (unsigned char)ajFalse;
    if(gid<GIDLIMIT)
	return (unsigned char)ajFalse;

    AJCNEW0(buf,AJ_OUTBUF+1);
    

    prog  = ajStrNew();
    cl    = ajStrNew();
    envi  = ajStrNew();
    dir   = ajStrNew();

    outstd = ajStrNew();
    errstd = ajStrNew();

    
    sptr = (char *) (*env)->GetStringUTFChars(env,commandline,0);
    ajStrAssC(&cl,sptr);
    (*env)->ReleaseStringUTFChars(env,commandline,sptr);


    handle = ajStrTokenInit(cl," \t\n");
    ajStrToken(&prog,&handle,NULL);
    ajStrTokenClear(&handle);
    

    sptr = (char *) (*env)->GetStringUTFChars(env,environment,0);
    ajStrAssC(&envi,sptr);
    (*env)->ReleaseStringUTFChars(env,environment,sptr);

    sptr = (char *) (*env)->GetStringUTFChars(env,directory,0);
    ajStrAssC(&dir,sptr);
    (*env)->ReleaseStringUTFChars(env,directory,sptr);


    argp = make_array(cl);
    envp = make_array(envi);

    if(!ajSysWhichEnv(&prog,envp))
	return (unsigned char)ajFalse;

    pipe(outpipe);
    pipe(errpipe);

    pid = fork();
    if(pid == -1)
	return (unsigned char)ajFalse;
    
    if(!pid)			/* Child */
    {
	dup2(outpipe[1],1);
	dup2(errpipe[1],2);
	if(setgid(gid)==-1)
	{
	    fprintf(stderr,"setgid failure");
	    exit(-1);
	}
	if(setuid(uid)==-1)
	{
	    fprintf(stderr,"setuid failure");
	    exit(-1);
	}
	if(chdir(ajStrStr(dir))==-1)
	{
	    fprintf(stderr,"chdir failure");
	    exit(-1);
	}
	if(execve(ajStrStr(prog),argp,envp) == -1)
	{
	    fprintf(stderr,"execve failure");
	    exit(-1);
	}
    }



    while((retval=waitpid(pid,&status,WNOHANG))!=pid && !retval)
    {
	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(outpipe[0],&rec))
	{
	    nread = read(outpipe[0],(void *)buf,AJ_OUTBUF);
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
	    nread = read(errpipe[0],(void *)buf,AJ_OUTBUF);
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
	nread = read(outpipe[0],(void *)buf,AJ_OUTBUF);
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
	nread = read(errpipe[0],(void *)buf,AJ_OUTBUF);
	buf[nread]='\0';
	ajStrAppC(&errstd,buf);
    }


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


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


    AJFREE(buf);

    ajStrDel(&prog);
    ajStrDel(&cl);
    ajStrDel(&envi);
    ajStrDel(&dir);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    return (unsigned char)ajTrue;
}


/* @funcstatic make_array ********************************************
**
** Construct argv and env arrays for Ajax.fork
**
** @param [r] str [AjPStr] space separated tokens
**
** @return [char**] env or argv array
******************************************************************************/

static char **make_array(AjPStr str)
{
    int n;
    char **ptr=NULL;
    AjPStrTok handle;
    AjPStr token;
    
    token = ajStrNew();
    
    n = ajStrTokenCount(&str," \t\n");

    AJCNEW0(ptr,n+1);

    ptr[n] = NULL;

    n = 0;
    
    handle = ajStrTokenInit(str," \t\n");
    while(ajStrToken(&token,&handle,NULL))
	ptr[n++] = ajCharNew(token);

    ajStrTokenClear(&handle);
    ajStrDel(&token);
    
    return ptr;
}




/* @funcstatic java_uniqueName ********************************************
**
** Construct a unique name for the jembossctl socket
**
** @param [r] username [AjPStr] username
** @param [r] command [ajint] command type
**
** @return [AjPStr] unique name
******************************************************************************/

static AjPStr java_uniqueName(AjPStr username, ajint command)
{
    AjPStr pipename=NULL;
    struct timeval tv;
    
    if(gettimeofday(&tv,NULL))
	return NULL;
    
    pipename = ajStrNew();
    ajFmtPrintS(&pipename,"/tmp/%S-%d-%Ld-%Ld-%d",username,ajRandomNumber(),tv.tv_sec,
		tv.tv_usec,command);

    return pipename;
}


/* @funcstatic java_send_auth ********************************************
**
** Send verification/homedir request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_send_auth(int conndes, char *cuser, char *cpass,
			    AjPStr *errstd)
{
    AjPStr cmnd=NULL;
    char   c;
    int    n;
    
    cmnd = ajStrNew();

    ajFmtPrintS(&cmnd,"%d %s %s",COMM_AUTH,cuser,cpass);

    if(send(conndes,ajStrStr(cmnd),ajStrLen(cmnd)+1,0) < 0)
    {
	ajStrAppC(errstd,"Error sending (java_send_auth)\n");
	ajStrDel(&cmnd);
	return -1;
    }

    if((n=recv(conndes,&c,1,0)) < 0)
    {
	ajStrAppC(errstd,"Error receiving (java_send_auth)\n");
	ajStrDel(&cmnd);
	return -1;
    }

    ajStrDel(&cmnd);


    if(!c)
	return -1;

    return 0;
}





/* @funcstatic java_emboss_fork ********************************************
**
** Send an emboss fork request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] clemboss [AjPStr] emboss command line
** @param [r] envi [AjPStr] emboss environment
** @param [r] dir [AjPStr] directory to invoke emboss from
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_emboss_fork(int conndes,char *cuser,char *cpass,
			      AjPStr clemboss, AjPStr envi, AjPStr dir,
			      char *buf, AjPStr *errstd)
{
    char *p=NULL;
    int n;
    
    
    n = sprintf(buf,"%d %s %s",EMBOSS_FORK,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(clemboss));
    p += (ajStrLen(clemboss)+1);
    sprintf(p,"%s",ajStrStr(envi));
    p += (ajStrLen(envi)+1);
    sprintf(p,"%s",ajStrStr(dir));
    n = (p-buf) + ajStrLen(dir) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_emboss_fork)\n");
	return -1;
    }

    return 0;
}


/* @funcstatic java_make_dir ********************************************
**
** Send a make directory request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] dir [AjPStr] directory to create
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_make_dir(int conndes,char *cuser,char *cpass,AjPStr dir,
			   char *buf, AjPStr *errstd)
{
    char *p=NULL;
    int n;
    
    
    n = sprintf(buf,"%d %s %s",MAKE_DIRECTORY,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(dir));
    n = (p-buf) + ajStrLen(dir) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_make_dir)\n");
	return -1;
    }

    return 0;
}



/* @funcstatic java_delete_file ********************************************
**
** Send a delete file request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] ufile [AjPStr] file to delete
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_delete_file(int conndes,char *cuser,char *cpass,AjPStr ufile,
			 char *buf, AjPStr *errstd)
{
    char *p=NULL;
    int n;
    
    
    n = sprintf(buf,"%d %s %s",DELETE_FILE,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(ufile));
    n = (p-buf) + ajStrLen(ufile) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_delete_file)\n");
	return -1;
    }

    return 0;
}


/* @funcstatic java_delete_dir ********************************************
**
** Send a delete directory request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] dir [AjPStr] directory to delete
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_delete_dir(int conndes,char *cuser,char *cpass,AjPStr dir,
			     char *buf, AjPStr *errstd)
{
    char *p=NULL;
    int n;
    
    
    n = sprintf(buf,"%d %s %s",DELETE_DIR,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(dir));
    n = (p-buf) + ajStrLen(dir) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_delete_dir)\n");
	return -1;
    }

    return 0;
}


/* @funcstatic java_list_files ********************************************
**
** Send a list files request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] dir [AjPStr] directory to scan
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_list_files(int conndes,char *cuser,char *cpass,AjPStr dir,
			     char *buf, AjPStr *errstd)
{
    char *p=NULL;
    int n;
    
    
    n = sprintf(buf,"%d %s %s",LIST_FILES,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(dir));
    n = (p-buf) + ajStrLen(dir) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_list_files)\n");
	return -1;
    }

    return 0;
}



/* @funcstatic java_list_dirs ********************************************
**
** Send a list directories request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] dir [AjPStr] directory to scan
** @param [w] buf [char*] socket buffer
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_list_dirs(int conndes,char *cuser,char *cpass,AjPStr dir,
			    char *buf, AjPStr *errstd)
{
    char *p=NULL;
    int n;
    
    
    n = sprintf(buf,"%d %s %s",LIST_DIRS,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(dir));
    n = (p-buf) + ajStrLen(dir) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_list_dirs)\n");
	return -1;
    }

    return 0;
}



/* @funcstatic java_get_file ********************************************
**
** Send a get file request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] file [AjPStr] file to get
** @param [w] buf [char*] socket buffer
** @param [w] fbuf [unsigned char**] file buffer
** @param [w] size [int*] file size
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_get_file(int conndes,char *cuser,char *cpass,AjPStr file,
			   char *buf,unsigned char **fbuf,int *size,
			   AjPStr *errstd)
{
    char *p=NULL;
    int n;
    int rlen=0;
    
    
    n = sprintf(buf,"%d %s %s",GET_FILE,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(file));
    n = (p-buf) + ajStrLen(file) +1;

    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_get_file)\n");
	return -1;
    }


    if((rlen = recv(conndes,buf,JBUFFLEN,0)) < 0)
    {
	ajStrAppC(errstd,"Reading error (java_get_file)\n");
	return -1;
    }

    if(sscanf(buf,"%d",&n)!=1)
    {
	ajStrAppC(errstd,"Bad size (java_get_file)\n");
	return -1;
    }

    *size = n;

    if(n)
	if(!(*fbuf=(unsigned char *)malloc(n)))
	{
	    ajStrAppC(errstd,"Malloc error (java_get_file)\n");
	    return -1;
	}
    
    return 0;
}


/* @funcstatic java_put_file ********************************************
**
** Send a put file request to jembossctl
**
** @param [rw] conndes [int] socket
** @param [r] cuser [char*] username
** @param [r] cpass [char*] password
** @param [r] file [AjPStr] file to get
** @param [w] buf [char*] socket buffer
** @param [r] fbuf [unsigned char*] file buffer
** @param [r] size [int] file size
** @param [w] errstd [AjPStr*] stderr from jembossctl
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static ajint java_put_file(int conndes,char *cuser,char *cpass,AjPStr file,
			   char *buf,unsigned char *fbuf,int size,
			   AjPStr *errstd)
{
    char *p=NULL;
    int n;
    int pos=0;
    int rlen=0;
    

    n = sprintf(buf,"%d %s %s",PUT_FILE,cuser,cpass);
    p=buf+n+1;
    sprintf(p,"%s",ajStrStr(file));
    n = (p-buf) + ajStrLen(file) +1;

    /* send command */
    if(send(conndes,buf,n,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_put_file)\n");
	return -1;
    }



    if((rlen = recv(conndes,buf,2,0))==-1)
    {
	ajStrAppC(errstd,"Socket Recv error 2\n");
	return -1;
    }




    /* Send file size */
    n = sprintf(buf,"%d",size);
    if(send(conndes,buf,n+1,0) < 0)
    {
	ajStrAppC(errstd,"Sending error (java_put_file)\n");
	return -1;
    }
    

    if((rlen = recv(conndes,buf,2,0))==-1)
    {
	ajStrAppC(errstd,"Socket Recv error 2\n");
	return -1;
    }



    while(pos+JBUFFLEN < size)
    {
 	memcpy((void *)buf,(const void *)&fbuf[pos],JBUFFLEN);
	if(send(conndes,buf,JBUFFLEN,0) < 0)
	{
	    ajStrAppC(errstd,"Sending error (java_put_file)\n");
	    return -1;
	}
	pos += JBUFFLEN;
    }
    if(size)
	if(size-pos)
	{
	    memcpy((void *)buf,(const void *)&fbuf[pos],size-pos);
	    if(send(conndes,buf,size-pos,0) < 0)
	    {
		ajStrAppC(errstd,"Sending error (java_put_file)\n");
		return -1;
	    }
	}

    return 0;
}


/* @funcstatic java_wait_for_term ********************************************
**
** Wait for jembossctl to terminate
**
** @param [r] pid [int] jembossctl pid
** @param [w] outstd [AjPStr*] stdout from jembossctl
** @param [w] errstd [AjPStr*] stderr from jembossctl
** @param [w] outpipe [int*] stdout pipe
** @param [w] errpipe [int*] stderr pipe
** @param [w] buf [char*] pipe buffer
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static void java_wait_for_term(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf)
{
    fd_set rec;
    struct timeval t;
    int nread=0;
    int  status;
    int retval=0;


    while((retval=waitpid(pid,&status,WNOHANG))!=pid && !retval)
    {
	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(outpipe[0],&rec))
	{
	    nread = read(outpipe[0],(void *)buf,JBUFFLEN);
	    buf[nread]='\0';
	    ajStrAppC(outstd,buf);
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(errpipe[0],&rec))
	{
	    nread = read(errpipe[0],(void *)buf,JBUFFLEN);
	    buf[nread]='\0';
	    ajStrAppC(errstd,buf);
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(outpipe[0],&rec))
    {
	nread = read(outpipe[0],(void *)buf,JBUFFLEN);
	buf[nread]='\0';
	ajStrAppC(outstd,buf);
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(errpipe[0],&rec))
    {
	nread = read(errpipe[0],(void *)buf,JBUFFLEN);
	buf[nread]='\0';
	ajStrAppC(errstd,buf);
    }


    return;
}


/* @funcstatic java_wait_for_file ********************************************
**
** Wait for jembossctl to terminate. Receive a file from stdout pipe
**
** @param [r] pid [int] jembossctl pid
** @param [w] outstd [AjPStr*] stdout from jembossctl
** @param [w] errstd [AjPStr*] stderr from jembossctl
** @param [w] outpipe [int*] stdout pipe
** @param [w] errpipe [int*] stderr pipe
** @param [w] buf [char*] pipe buffer
** @param [w] fbuf [unsigned char*] buffer for file
** @param [r] size [int] size of file to receive
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static void java_wait_for_file(int pid,AjPStr *outstd, AjPStr *errstd,
			       int *outpipe, int *errpipe, char *buf,
			       unsigned char *fbuf,int size)
{
    fd_set rec;
    struct timeval t;
    int nread=0;
    int  status;
    int retval=0;
    unsigned char *ptr;

    ptr = fbuf;

    while((retval=waitpid(pid,&status,WNOHANG))!=pid && !retval)
    {
	FD_ZERO(&rec);
	FD_SET(outpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(outpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(outpipe[0],&rec))
	{
	    nread = read(outpipe[0],(void *)buf,JBUFFLEN);
	    memcpy((void *)ptr,(const void *)buf,nread);
	    ptr += nread;
	}

	FD_ZERO(&rec);
	FD_SET(errpipe[0],&rec);
	t.tv_sec = 0;
	t.tv_usec = 0;
	select(errpipe[0]+1,&rec,NULL,NULL,&t);
	if(FD_ISSET(errpipe[0],&rec))
	{
	    nread = read(errpipe[0],(void *)buf,JBUFFLEN);
	    buf[nread]='\0';
	    ajStrAppC(errstd,buf);
	}


    }


    FD_ZERO(&rec);
    FD_SET(outpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(outpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(outpipe[0],&rec))
    {
	nread = read(outpipe[0],(void *)buf,JBUFFLEN);
	memcpy((void *)ptr,(const void *)buf,nread);
	ptr += nread;
    }


    FD_ZERO(&rec);
    FD_SET(errpipe[0],&rec);
    t.tv_sec = 0;
    t.tv_usec = 0;
    select(errpipe[0]+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(errpipe[0],&rec))
    {
	nread = read(errpipe[0],(void *)buf,JBUFFLEN);
	buf[nread]='\0';
	ajStrAppC(errstd,buf);
    }


    if(ptr-fbuf != size)
	ajStrAppC(errstd,"\nIncomplete file read\n");


    return;
}



/* @funcstatic java_jembossctl ********************************************
**
** fork jembossctl and broker requests
**
** @param [r] command [ajint] command type
** @param [r] username [AjPStr] username
** @param [r] password [AjPStr] password
** @param [r] environment [AjPStr] environment
** @param [r] str1 [AjPStr] command string. depends on command
** @param [r] str2 [AjPStr] command string. depends on command
** @param [w] outstd [AjPStr*] stdout from jembossctl
** @param [w] errstd [AjPStr*] stderr from jembossctl
** @param [r] fbuf [unsigned char**] file buffer
** @param [r] fsize [int*] file size
**
** @return [ajint] 0=success -1=failure
******************************************************************************/

static int java_jembossctl(ajint command, AjPStr username, AjPStr password,
			   AjPStr environment,
			   AjPStr str1, AjPStr str2,
			   AjPStr *outstd, AjPStr *errstd,
			   unsigned char **fbuf, int *fsize)
{
    AjPStr dir=NULL;
    AjPStr uniq=NULL;

    AjPStr prog=NULL;    /* Fork strings */
    AjPStr cl=NULL;
    AjPStr clemboss=NULL;
    AjPStr envi=NULL;

    struct timeval tv;
    long   then;
    long   now;
    int    i=0;
    
    
    char *cuser=NULL;
    char *cpass=NULL;
    char *cuniq=NULL;
    char *buff=NULL;
    int size=0;
    

    /* Socket stuff */
    int sockdes;		/* Initial socket descriptor   */
    int conndes;		/* Connected socket descriptor */
    
    struct sockaddr_un here;
    struct sockaddr_un there;
    int nlen;
    int tlen=0;
    int rlen=0;



    char **argp=NULL;
    char **envp=NULL;
    int  pid;

    int  outpipe[2];
    int  errpipe[2];
    
    int retval=0;


    uniq = java_uniqueName(username,command);
    cuniq = ajStrStr(uniq);
    cuser = ajStrStr(username);
    cpass = ajStrStr(password);

    clemboss = ajStrNew();
    dir      = ajStrNew();
    cl       = ajStrNew();
    envi     = ajStrNew();
    prog     = ajStrNew();


    ajRandomSeed();
    
    if(!(buff=(char*)malloc(JBUFFLEN)))
    {
	ajStrAppC(errstd,"Malloc error\n");
	java_tidy_command(&uniq,&cl,&clemboss,&dir,&envi,&prog);
	return -1;
    }
    

    
    /* Get socket descriptor */
    sockdes = socket(AF_UNIX,SOCK_STREAM,0);
    if(sockdes==-1)
    {
	ajStrAppC(errstd,"No socket descriptor\n");
	java_tidy_command2(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff);
	return -1;
    }
    

    /* Process might not return so set non-blocking */
    fcntl(sockdes,F_SETFL,O_NONBLOCK);


    /* Bind to socket */
    here.sun_family = AF_UNIX;
    strcpy(here.sun_path,ajStrStr(uniq));
    unlink(here.sun_path);
    nlen = ajStrLen(uniq) + sizeof(here.sun_family);
    if(bind(sockdes,(struct sockaddr*)&here,nlen)==-1)
    {
	ajStrAppC(errstd,"Bind error\n");
	close(sockdes);
	unlink(cuniq);
	java_tidy_command2(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff);
	return -1;
    }
    

    /* Set socket listening */
    if(listen(sockdes,10)==-1)
    {
	ajStrAppC(errstd,"Listen error\n");
	close(sockdes);
	unlink(cuniq);
	java_tidy_command2(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff);
	return -1;
    }
    
    /* Set environment for execve */
    ajStrAssS(&envi,environment);
    envp = make_array(envi);

    /* Set program name */
    ajStrAssC(&prog,"jembossctl");
    if(!ajSysWhichEnv(&prog,envp))
    {
	ajStrAppC(errstd,"Cannot locate jembossctl\n");
	close(sockdes);
	unlink(cuniq);
	java_tidy_command2(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff);
	return -1;
    }

    /* Setup commandline */
    ajFmtPrintS(&cl,"jembossctl %s -auto",cuniq);
    argp = make_array(cl);
    
    pipe(outpipe);
    pipe(errpipe);

    pid = fork();
    if(pid == -1)
    {
	ajStrAppC(errstd,"Fork error\n");
	java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			   sockdes,outpipe,errpipe);
	return -1;
    }

    if(!pid)			/* Child (doit suid prog) */
    {
	dup2(outpipe[1],1);
	dup2(errpipe[1],2);
	if(execve(ajStrStr(prog),argp,envp) == -1)
	{
	    fprintf(stderr,"execve failure\n");
	    exit(-1);
	}
    }

    /* Don't need the env or commline for the parent */
    i = 0;
    while(argp[i])
        AJFREE(argp[i]);
    AJFREE(argp);

    i = 0;
    while(envp[i])
        AJFREE(envp[i]);
    AJFREE(envp);
    /* End of fork code */



    /* Wait for a connection with timeout of 30 seconds */
    /* Success if jembossctl says OK across the socket  */
    if(gettimeofday(&tv,NULL))
    {
	ajStrAppC(errstd,"Gettimeofday error\n");
	java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			   sockdes,outpipe,errpipe);
	return -1;
    }
    then = tv.tv_sec;
    
    tlen = sizeof(there);
    while((conndes=accept(sockdes,(struct sockaddr*)&there,&tlen))==-1)
    {
	if(errno!=EAGAIN)
	    break;

	if(gettimeofday(&tv,NULL))
	{
	    ajStrAppC(errstd,"Gettimeofday error\n");
	    java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			       sockdes,outpipe,errpipe);
	    return -1;
	}
	now = tv.tv_sec;
	if(now-then >= 30)
	{
	    ajStrAppC(errstd,"Timeout error\n");
	    java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			       sockdes,outpipe,errpipe);
	    return -1;
	}

    }
    
    if(!conndes)
    {
	ajStrAppC(errstd,"Server connect error\n");
	java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			   sockdes,outpipe,errpipe);
	return -1;
    }


    *buff = '\0';
    if((rlen = recv(conndes,buff,2,0))==-1)
    {
	ajStrAppC(errstd,"Socket Recv error 1\n");
	java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			   sockdes,outpipe,errpipe);
	close(conndes);
	return -1;
    }


    if(rlen<2 || strncmp(buff,"OK",2))
    {
	ajStrAppC(errstd,"Incorrect ACK error\n");
	java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
			   sockdes,outpipe,errpipe);
	close(conndes);
	return -1;
    }




    

    /* At this point socket comms are active and commands can be sent */

    switch(command)
    {
    case COMM_AUTH:
	retval = java_send_auth(conndes,cuser,cpass,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case EMBOSS_FORK:
	ajStrAssS(&clemboss,str1);
	ajStrAssS(&dir,str2);
	retval = java_emboss_fork(conndes,cuser,cpass,clemboss,envi,dir,buff,
				  errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case MAKE_DIRECTORY:
	ajStrAssS(&dir,str1);
	retval = java_make_dir(conndes,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case DELETE_FILE:
	ajStrAssS(&dir,str1);
	retval = java_delete_file(conndes,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case DELETE_DIR:
	ajStrAssS(&dir,str1);
	retval = java_delete_dir(conndes,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case LIST_FILES:
	ajStrAssS(&dir,str1);
	retval = java_list_files(conndes,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case LIST_DIRS:
	ajStrAssS(&dir,str1);
	retval = java_list_dirs(conndes,cuser,cpass,dir,buff,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;

    case GET_FILE:
	ajStrAssS(&dir,str1);
	retval = java_get_file(conndes,cuser,cpass,dir,buff,fbuf,&size,errstd);
	java_wait_for_file(pid,outstd,errstd,outpipe,errpipe,buff,*fbuf,size);
	*fsize = size;
	break;

    case PUT_FILE:
	ajStrAssS(&dir,str1);
	size = *fsize;
	retval = java_put_file(conndes,cuser,cpass,dir,buff,*fbuf,size,errstd);
	java_wait_for_term(pid,outstd,errstd,outpipe,errpipe,buff);
	break;
	
    default:
	break;
    }
    

    java_tidy_command3(&uniq,&cl,&clemboss,&dir,&envi,&prog,buff,cuniq,
		       sockdes,outpipe,errpipe);
    close(conndes);

    return retval;
}


/* @funcstatic java_tidy_command ********************************************
**
** Delete allocated memory
**
** @param [w] uniq [AjPStr*] unique name
** @param [w] cl [AjPStr] command line
** @param [w] clemboss [AjPStr*] emboss command line
** @param [w] dir [AjPStr*] directory
** @param [w] envi [AjPStr*] environment
** @param [w] prog [AjPStr*] program name
**
** @return [void]
******************************************************************************/

static void java_tidy_command(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			      AjPStr *dir, AjPStr *envi, AjPStr *prog)
{

    ajStrDel(uniq);
    ajStrDel(cl);
    ajStrDel(clemboss);
    ajStrDel(dir);
    ajStrDel(envi);
    ajStrDel(prog);

    return;
}

/* @funcstatic java_tidy_command2 ********************************************
**
** Delete allocated memory
**
** @param [w] uniq [AjPStr*] unique name
** @param [w] cl [AjPStr] command line
** @param [w] clemboss [AjPStr*] emboss command line
** @param [w] dir [AjPStr*] directory
** @param [w] envi [AjPStr*] environment
** @param [w] prog [AjPStr*] program name
** @param [w] buf [char*] socket buffer
**
** @return [void]
******************************************************************************/

static void java_tidy_command2(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf)
{
    
    java_tidy_command(uniq,cl,clemboss,dir,envi,prog);
    AJFREE(buf);
    
    return;
}


/* @funcstatic java_tidy_command2 ********************************************
**
** Delete allocated memory and close pipes
**
** @param [w] uniq [AjPStr*] unique name
** @param [w] cl [AjPStr] command line
** @param [w] clemboss [AjPStr*] emboss command line
** @param [w] dir [AjPStr*] directory
** @param [w] envi [AjPStr*] environment
** @param [w] prog [AjPStr*] program name
** @param [w] buf [char*] socket buffer
** @param [r] cuniq [char*] socket name
** @param [w] sockdes [int] socket
** @param [w] outpipe [int*] stdout pipe
** @param [w] errpipe [int*] stderr pipe
**
** @return [void]
******************************************************************************/

static void java_tidy_command3(AjPStr *uniq, AjPStr *cl, AjPStr *clemboss,
			       AjPStr *dir, AjPStr *envi, AjPStr *prog,
			       char *buf, char *cuniq, int sockdes,
			       int *outpipe, int *errpipe)
{
    
    close(sockdes);
    unlink(cuniq);
    close(outpipe[0]);
    close(outpipe[1]);
    close(errpipe[0]);
    close(errpipe[1]);

    java_tidy_command2(uniq,cl,clemboss,dir,envi,prog,buf);
    
    return;
}


/* @func Ajax.userAuth **************************************************
**
** Verify user and retrieve home directory
** Loads outStd,errStd within java.
** Sets 'home' in Java
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_userAuth
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  hstr;    /* Java returned strings */
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);

    AjPStr home=NULL;

    AjBool ok=ajFalse;
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    home     = ajStrNew();
    envi     = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);

    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    ok = ajFalse;
    if(!java_jembossctl(COMM_AUTH,username,password,envi,NULL,NULL,&outstd,
			&errstd,NULL,NULL))
    {
	ok = ajTrue;
	ajStrAssS(&home,outstd);
    }
    

    field = (*env)->GetFieldID(env,jvc,"home","Ljava/lang/String;");    
    hstr = (*env)->NewStringUTF(env,ajStrStr(home));
    (*env)->SetObjectField(env,obj,field,hstr);


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&home);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}



/* @func Ajax.forkEmboss **************************************************
**
** Run an EMBOSS program
** Loads outStd,errStd within java.
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] cline [jstring] command line
** @param [r] direct [jstring] directory to create
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_forkEmboss
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring cline, jstring direct)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jcl =NULL;
    char   *jdir =NULL;

    AjPStr commandline=NULL;
    AjPStr directory=NULL;
    
    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    commandline = ajStrNew();    
    directory   = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);


    jcl = (char *) (*env)->GetStringUTFChars(env,cline,0);
    if(jcl)
	ajStrAssC(&commandline,jcl);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,cline,jcl);

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);
    if(jdir)
	ajStrAssC(&directory,jdir);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,direct,jdir);


    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    ok = ajFalse;
    if(!java_jembossctl(EMBOSS_FORK,username,password,envi,commandline,
			directory,&outstd,&errstd,NULL,NULL))
	ok = ajTrue;
    

    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);

    ajStrDel(&commandline);
    ajStrDel(&directory);
    

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}


/* @func Ajax.makeDir **************************************************
**
** Create a directory
** Loads outStd,errStd within java.
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to create
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_makeDir
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jdir =NULL;
    
    AjPStr directory=NULL;

    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    directory  = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);

    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);
    if(jdir)
	ajStrAssC(&directory,jdir);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,direct,jdir);




    ok = ajFalse;
    if(!java_jembossctl(MAKE_DIRECTORY,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&directory);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}



/* @func Ajax.delFile **************************************************
**
** Delete a file
** Loads outStd,errStd within java.
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] file to delete
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_delFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jfil =NULL;
    
    AjPStr file=NULL;

    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    file     = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);
    if(jfil)
	ajStrAssC(&file,jfil);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,filename,jfil);




    ok = ajFalse;
    if(!java_jembossctl(DELETE_FILE,username,password,envi,file,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&file);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}


/* @func Ajax.delDir **************************************************
**
** Recursively delete a directory
** Loads outStd,errStd within java.
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to delete
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_delDir
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jdir =NULL;
    
    AjPStr directory=NULL;

    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    directory  = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);
    if(jdir)
	ajStrAssC(&directory,jdir);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,direct,jdir);




    ok = ajFalse;
    if(!java_jembossctl(DELETE_DIR,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&directory);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}



/* @func Ajax.listFiles **************************************************
**
** Lists regular files
** Loads outStd,errStd within java. outStd contains the list
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to scan
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_listFiles
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jdir =NULL;
    
    AjPStr directory=NULL;

    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    directory  = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);


    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);



    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);
    if(jdir)
	ajStrAssC(&directory,jdir);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,direct,jdir);




    ok = ajFalse;
    if(!java_jembossctl(LIST_FILES,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&directory);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}



/* @func Ajax.listDirs **************************************************
**
** Lists files of type 'directory' 
** Loads outStd,errStd within java. outStd contains the list
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] direct [jstring] directory to scan
**
** @return [jboolean] true if success
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_listDirs
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring direct)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jdir =NULL;
    
    AjPStr directory=NULL;

    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);
    int i;
    
    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    directory  = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);

    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    jdir = (char *) (*env)->GetStringUTFChars(env,direct,0);
    if(jdir)
	ajStrAssC(&directory,jdir);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,direct,jdir);




    ok = ajFalse;
    if(!java_jembossctl(LIST_DIRS,username,password,envi,directory,NULL,
			&outstd,&errstd,NULL,NULL))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&directory);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}



/* @func Ajax.getFile **************************************************
**
** Get a user file
** Loads outStd,errStd within java
** Sets size (of file),fileok (success) & prnt ([non]printable]
** variables in java
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] filename to get
**
** @return [jbyteArray] contents of file
** @@
******************************************************************************/


JNIEXPORT jbyteArray JNICALL Java_org_emboss_jemboss_parser_Ajax_getFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    jbyteArray jb;
    
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jfil =NULL;
    
    AjPStr file=NULL;
    unsigned char *fbuf=NULL;
    int size=0;
    int prnt=1;
    int fileok=0;
    int c;
    int i;
    
    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);

    
    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    file     = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    (*env)->ReleaseStringUTFChars(env,door,juser);

    if(!(jpass=(char *)malloc(plen+1)))
	return NULL;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);

    (*env)->ReleaseStringUTFChars(env,environment,jenv);


    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);
    if(jfil)
	ajStrAssC(&file,jfil);
    (*env)->ReleaseStringUTFChars(env,filename,jfil);




    ok = ajFalse;
    if(!java_jembossctl(GET_FILE,username,password,envi,file,NULL,
			&outstd,&errstd,&fbuf,&size))
	ok = ajTrue;



    fileok = 0;
    if(ok)
	fileok=1;

    if(!size)
    {
	fileok = 0;
	AJCNEW0(fbuf,1);
    }
    

    for(i=0;i<size;++i)
    {
	c = (int)fbuf[i];
	if(c==(int)'\n' || c==(int)'\t' || c==(int)'\r')
	    continue;
	if(!isprint(c))
	{
	    prnt=0;
	    break;
	}
    }
    
    

    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    field = (*env)->GetStaticFieldID(env,jvc,"prnt","I");
    (*env)->SetStaticIntField(env,jvc,field,prnt);


    field = (*env)->GetStaticFieldID(env,jvc,"size","I");
    (*env)->SetStaticIntField(env,jvc,field,size);



    field = (*env)->GetStaticFieldID(env,jvc,"fileok","I");
    (*env)->SetStaticIntField(env,jvc,field,fileok);

    jb = (*env)->NewByteArray(env,size);
    (*env)->SetByteArrayRegion(env,jb,0,size,(jbyte *)fbuf);


    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&file);
    AJFREE(fbuf);


    return jb;
}



/* @func Ajax.putFile **************************************************
**
** Write a user file
** Loads outStd,errStd within java
**
** @param [rw] env [JNIEnv*] java environment
** @param [r] obj [jobject] java object
** @param [r] door [jstring] username
** @param [r] key [jstring] password
** @param [r] environment [jstring] environment
** @param [r] filename [jstring] filename to create
** @param [r] arr [jbyteArray] contents for the file
**
** @return [jboolean] true if the file was written
** @@
******************************************************************************/

JNIEXPORT jboolean JNICALL Java_org_emboss_jemboss_parser_Ajax_putFile
(JNIEnv *env, jobject obj, jstring door, jbyteArray key,
 jstring environment, jstring filename, jbyteArray arr)
{
    AjPStr username=NULL;
    AjPStr password=NULL;
    AjPStr envi=NULL;
    AjPStr outstd=NULL;
    AjPStr errstd=NULL;
    

    jclass jvc = (*env)->GetObjectClass(env,obj);
    jfieldID field;
    jstring  ostr;
    jstring  estr;
    jsize    len = (*env)->GetArrayLength(env,arr);
    jbyte    *ba = (*env)->GetByteArrayElements(env,arr,0);
    char   *juser=NULL;
    char   *jpass=NULL;
    char   *jenv =NULL;
    char   *jfil =NULL;
    
    AjPStr file=NULL;
    unsigned char *fbuf;
    int size;
    int i;
    
    AjBool ok=ajFalse;
    jsize  plen = (*env)->GetArrayLength(env,key);
    jbyte  *ca = (*env)->GetByteArrayElements(env,key,0);


    size = len;
    if(!(fbuf=(unsigned char *)malloc(size)))
	return (unsigned char)ajFalse;
    for(i=0;i<size;++i)
	fbuf[i] = (unsigned char)ba[i];
    (*env)->ReleaseByteArrayElements(env,arr,ba,0);

    username = ajStrNew();
    password = ajStrNew();
    envi     = ajStrNew();

    file     = ajStrNew();
    
    juser = (char *) (*env)->GetStringUTFChars(env,door,0);
    if(juser)
	ajStrAssC(&username,juser);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,door,juser);

    if(!(jpass=(char *)malloc(plen+1)))
	return (unsigned char)ajFalse;
    bzero((void *)jpass,plen+1);
    for(i=0;i<plen;++i)
	jpass[i] = (char)ca[i];
    ajStrAssC(&password,jpass);
    (*env)->ReleaseByteArrayElements(env,key,ca,0);


    jenv = (char *) (*env)->GetStringUTFChars(env,environment,0);
    if(jenv)
	ajStrAssC(&envi,jenv);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,environment,jenv);

    if(!ajStrLen(username) || !ajStrLen(password) || !ajStrLen(envi))
	return (unsigned char)ajFalse;

    jfil = (char *) (*env)->GetStringUTFChars(env,filename,0);
    if(jfil)
	ajStrAssC(&file,jfil);
    else
	return (unsigned char)ajFalse;
    (*env)->ReleaseStringUTFChars(env,filename,jfil);


    ok = ajFalse;
    if(!java_jembossctl(PUT_FILE,username,password,envi,file,NULL,
			&outstd,&errstd,&fbuf,&size))
	ok = ajTrue;


    field = (*env)->GetFieldID(env,jvc,"outStd","Ljava/lang/String;");    
    ostr = (*env)->NewStringUTF(env,ajStrStr(outstd));
    (*env)->SetObjectField(env,obj,field,ostr);

    field = (*env)->GetFieldID(env,jvc,"errStd","Ljava/lang/String;");
    estr = (*env)->NewStringUTF(env,ajStrStr(errstd));
    (*env)->SetObjectField(env,obj,field,estr);


    if(size)
	AJFREE(fbuf);
    
    ajStrDel(&username);
    ajStrDel(&password);
    ajStrDel(&envi);
    ajStrDel(&outstd);
    ajStrDel(&errstd);
    
    ajStrDel(&file);

    if(!ok)
	return (unsigned char)ajFalse;
    
    return (unsigned char)ajTrue;
}

#endif


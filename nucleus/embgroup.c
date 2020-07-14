/* @source embgroup.c
**
** Group Routines.
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

#include <sys/types.h>  /* for opendir etc. */
#include <dirent.h>     /* for readdir */
#include <string.h>
#include <sys/stat.h>   /* for stat */


static void grpGetAcdFiles(AjPList glist, AjPList alpha, char **env,
			   AjPStr acddir, AjBool explode, AjBool colon,
			   AjBool gui);
static void grpGetAcdDirs(AjPList glist, AjPList alpha, char **env,
			  AjPStr acddir, AjBool explode, AjBool colon,
			  AjBool gui);
static void grpParse(AjPFile file, AjPStr *appl, AjPStr *doc, AjPList groups,
		     AjBool explode, AjBool colon, AjBool *gui);
static void grpNoComment(AjPStr* text);
static AjPStr grpParseValueRB(AjPStrTok* tokenhandle, char* delim);
static void grpSplitList(AjPList groups, AjPStr value, AjBool explode,
			 AjBool colon);
static void grpSubSplitList(AjPList groups, AjPList sublist);
static void grpAddGroupsToList(AjPList alpha, AjPList glist,
			       AjPList groups, AjPStr appl, AjPStr doc);




/* @func embGrpGetProgGroups **************************************************
**
** Optionally constructs a path to the directory of normal EMBOSS or
** embassy ACD files. Calls grpGetAcdFiles to construct lists of the
** group, doc and program name information.
**
** @param [w] glist [AjPList] List of groups of programs
** @param [w] alpha [AjPList] Alphabetic list of programs
** @param [r] env [char **] Environment passed in from C main() parameters
** @param [r] emboss [AjBool] Read in EMBOSS ACD data
** @param [r] embassy [AjBool] Read in EMBASSY ACD data
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [r] gui [AjBool] Only report programs that are OK in GUIs
** @return [void]
** @@
******************************************************************************/

void embGrpGetProgGroups(AjPList glist, AjPList alpha, char **env,
			 AjBool emboss, AjBool embassy, AjBool explode,
			 AjBool colon, AjBool gui)
{

    AjPStr acdroot     = NULL;
    AjPStr acdrootdir  = NULL;
    AjPStr acdrootinst = NULL;
    AjPStr acdpack     = NULL;
    AjPStr alphaname   = NULL;
    GPnode gpnode; /* new member (first & only) of alpha being added */
    AjBool doneinstall = ajFalse;

    /* set up alpha programs group list */
    ajStrAssC(&alphaname, "Alphabetic list of programs");
    gpnode = embGrpMakeNewGnode(alphaname);
    ajListPushApp(alpha, gpnode);
    ajStrDel(&alphaname);


    /* look at all EMBOSS ACD files */
    acdpack     = ajStrNew();
    acdroot     = ajStrNew();
    acdrootdir  = ajStrNew();
    acdrootinst = ajStrNew();
    alphaname   = ajStrNew();

    ajNamRootPack(&acdpack);
    ajNamRootInstall(&acdrootinst);
    if(emboss)
    {
	if(ajNamGetValueC("acdroot", &acdroot))
	{
	    ajFileDirFix(&acdroot);
	    /*ajStrAppC(&acdroot, "acd/");*/
	}
	else
	{
	    ajFileDirFix(&acdrootinst);
	    ajFmtPrintS(&acdroot, "%Sshare/%S/acd/", acdrootinst, acdpack);

	    if(ajFileDir(&acdroot))
		doneinstall = ajTrue;
	    else
	    {
		ajNamRoot(&acdrootdir);
		ajFileDirFix(&acdrootdir);
		ajFmtPrintS(&acdroot, "%Sacd/", acdrootdir);
	    }
	}

	/* normal EMBOSS ACD */
	grpGetAcdFiles(glist, alpha, env, acdroot, explode, colon, gui);
    }

    if(embassy && !doneinstall)
    {
	ajFileDirFix(&acdroot);

	/* EMBOSS install directory */
	ajFmtPrintS(&acdroot, "%Sshare/%S/acd/",
		    acdrootinst, acdpack);

	if(ajFileDir(&acdroot))
	    /* embassadir ACD files */
	    grpGetAcdFiles(glist, alpha, env, acdroot, explode, colon, gui);
	else
	{
	    /* look for all source directories */
	    ajNamRoot(&acdrootdir);
	    ajFileDirUp(&acdrootdir);
	    ajFmtPrintS(&acdroot, "%Sembassy/", acdrootdir);
	    /* embassadir ACD files */
	    grpGetAcdDirs(glist, alpha, env, acdroot, explode, colon, gui);
	}

    }

    /* sort the groups and alpha lists */
    embGrpSortGroupsList(glist);
    embGrpSortGroupsList(alpha);

    ajStrDel(&acdroot);
    ajStrDel(&acdrootdir);
    ajStrDel(&acdrootinst);
    ajStrDel(&alphaname);
    ajStrDel(&acdpack);

    return;
}




/* @funcstatic grpGetAcdDirs **************************************************
**
** Given a directory from EMBASSY sources, it searches for directories
** of ACD files and passes processing on to grpGetAcdFiles
**
** @param [w] glist [AjPList] List of groups of programs
** @param [w] alpha [AjPList] Alphabetic list of programs
** @param [r] env [char **] Environment passed in from C main() parameters
** @param [r] acddir [AjPStr] path of directory holding ACD files to read in
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [r] gui [AjBool] Report only those applications OK in GUIs
** @return [void]
** @@
******************************************************************************/

static void grpGetAcdDirs(AjPList glist, AjPList alpha, char **env,
			  AjPStr acddir, AjBool explode, AjBool colon,
			  AjBool gui)
{
    DIR *dirp;
    DIR *dirpa;
    struct dirent *dp;
    static AjPStr dirname = NULL;


    /* go through all the directories in this directory */
    if((dirp = opendir(ajStrStr(acddir))) == NULL)
	return;			   /* could be no embassy installed */


    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if(dp->d_name[0] == '.')
	    continue;			/* don't want hidden files */
	ajFmtPrintS(&dirname, "%S%s/emboss_acd/", acddir, dp->d_name);

	if((dirpa = opendir(ajStrStr(dirname))))
	{
	    grpGetAcdFiles(glist, alpha, env, dirname, explode, colon, gui);
	    closedir(dirpa);
	}
    }

    closedir(dirp);

    return;
}




/* @funcstatic grpGetAcdFiles *************************************************
**
** Given a directory, it searches for ACD files which describe an
** existing program on the path.
** Parses out the documentation and groups from these ACD files.
** Returns a list of program names and documentation grouped by group names.
** Returns an alphabetic list of program names and documentation.
**
** @param [w] glist [AjPList] List of groups of programs
** @param [w] alpha [AjPList] Alphabetic list of programs
** @param [r] env [char **] Environment passed in from C main() parameters
** @param [r] acddir [AjPStr] path of directory holding ACD files to read in
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [r] gui [AjBool] Report only those applications OK in GUIs
** @return [void]
** @@
******************************************************************************/

static void grpGetAcdFiles(AjPList glist, AjPList alpha, char **env,
			   AjPStr acddir, AjBool explode, AjBool colon,
			   AjBool gui)
{
    DIR *dirp;
    struct dirent *dp;
    AjPStr progpath = NULL;
    AjPFile file    = NULL;
    AjPStr appl     = NULL;
    AjPStr applpath = NULL;		/* path of application */
    AjPStr doc      = NULL;
    AjPList groups  = NULL;
    AjBool guiresult;

    /* go through all the files in this directory */
    if((dirp = opendir(ajStrStr(acddir))) == NULL)
	ajFatal("You do not have read permission on the directory '%S'",
		acddir);

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	if(dp->d_name[0] != '.')
	{
	    ajStrAssL(&progpath, acddir,
		      ajStrLen(acddir)+strlen(dp->d_name)+3);
	    ajStrAppC(&progpath, dp->d_name);

	    /* does it end with ".acd" ? */
	    if(ajStrRFindC(progpath, ".acd") == ajStrLen(progpath)-4)
	    {
		/* see if it is a normal file */
		if(ajSysIsRegular(ajStrStr(progpath)))
		{
		    /* open the file and parse it */
		    if((file = ajFileNewIn(progpath)) != NULL)
		    {
			groups = ajListstrNew();
			grpParse(file, &appl, &doc, groups, explode,
				 colon, &guiresult);

			/* see if the appl is the name of a real program */
			ajStrAss(&applpath, appl);
			if(ajSysWhichEnv(&applpath, env))
			{
			    /*
			    ** see if the appl is OK in GUIs or we don't
			    ** want just GUI apps
			    */
			    if(guiresult || !gui)
				grpAddGroupsToList(alpha, glist, groups,
						   appl, doc);
			    else
				ajDebug("%S is not a OK in GUIs\n", appl);
			}

			ajFileClose(&file);
			ajListstrFree(&groups);
			ajStrDel(&appl);
			ajStrDel(&doc);
		    }
		}
	    }
	    ajStrDel(&progpath);
	}
    }

    closedir(dirp);
    ajStrDel(&applpath);

    return;
}




/* @funcstatic grpParse *******************************************************
**
** parse the acd file.
**
** @param [r] file [AjPFile]  ACD file
** @param [w] appl [AjPStr*] Application name
** @param [w] doc  [AjPStr*]  Documentation string
** @param [w] groups [AjPList] Program groups string
** @param [r] explode [AjBool] Expand group names around ':'
** @param [r] colon [AjBool] Retain ':' in group names
** @param [w] gui [AjBool*] returns ajTrue if application is OK in GUIs
**
** @return [void]
** @@
******************************************************************************/

static void grpParse(AjPFile file, AjPStr *appl, AjPStr *doc, AjPList groups,
		     AjBool explode, AjBool colon, AjBool *gui)
{

    AjPStr line = NULL;
    AjPStr text = NULL;

    AjPStrTok tokenhandle;
    char white[]     = " \t\n\r";
    char whiteplus[] = " \t\n\r:=";
    AjPStr tmpstr = NULL;
    AjPStr token  = NULL;
    AjPStr value  = NULL;
    ajint done = 0;
    ajint donedoc    = ajFalse;
    ajint donegroup  = ajFalse;
    AjPStr nullgroup = NULL;
    AjPStr newstr    = NULL;
    AjPStr tmpvalue  = NULL;

    /* initialise a name for programs with no assigned group */
    ajStrAppC(&nullgroup, "ASSORTED");

    /* if 'gui' not defined in ACD, default is 'gui: Y' */
    *gui = ajTrue;

    /* read file into one line, stripping out comment lines and blanks */
    while(ajFileReadLine(file, &line))
    {
	grpNoComment(&line);
	if(ajStrLen(line))
	{
	    ajStrApp(&text, line);
	    ajStrAppC(&text, " ");
	}
    }

    tokenhandle = ajStrTokenInit(text, white);

    /* find appl token */
    while(ajStrToken(&tmpstr, &tokenhandle, whiteplus))
	if(ajStrPrefixC(tmpstr, "appl"))
	    break;

    /* next token is the application name */
    ajStrToken(appl, &tokenhandle, white);

    /* if next token is '[' */
    ajStrToken(&tmpstr, &tokenhandle, white);
    if(ajStrCmpC(tmpstr, "[") == 0)
    {
	token=ajStrNew();

	/* is the next token 'doc' or 'groups' or 'gui' */
	while(ajStrToken(&tmpstr, &tokenhandle, whiteplus))
	{
	    while(ajStrCmpC(tmpstr, "]"))
	    {
		ajStrAss(&token, tmpstr);
		value = grpParseValueRB(&tokenhandle, white);
		done = !ajStrCmpC(value, "]");

		if(!done)
		{
		    ajStrToken(&tmpstr, &tokenhandle, whiteplus);
		    ajStrToLower(&tmpstr);
		    done = !ajStrCmpC(tmpstr, "]");
		}

		if(ajStrPrefixC(token, "doc"))
		{
		    donedoc = ajTrue;
		    ajStrAss(doc, value);
		    ajStrChomp(doc);
		    ajStrTrimC(doc, ".,");

		}
		else if(ajStrPrefixC(token, "gui"))
		{
		    ajStrAss(&tmpvalue, value);
		    ajStrChomp(&tmpvalue);
		    ajDebug("gui value '%S'\n", tmpvalue);
		    /* test for '[Nn]*' */
		    if(tolower((int)(ajStrStr(tmpvalue))[0]) == 'n')
			*gui = ajFalse;

		    ajStrDel(&tmpvalue);
		}
		else if(ajStrPrefixC(token, "group"))
		{
		    donegroup = ajTrue;
		    grpSplitList(groups, value, explode, colon);
		}
	    }
	    if(done)
		break;
	}
    }

    /* check that we got the doc and groups descriptions */
    if(!donedoc)
	ajStrAssC(doc, "");

    if(!donegroup)
    {
	newstr = ajStrDup(nullgroup);
	ajListstrPushApp(groups, newstr);
    }

    ajStrDel(&nullgroup);
    ajStrDel(&tmpstr);
    ajStrDel(&line);
    ajStrDel(&text);
    ajStrTokenClear(&tokenhandle);
    ajStrDel(&token);
    ajStrDel(&nullgroup);

    return;
}




/* @funcstatic grpNoComment ***************************************************
**
** Strips comments from a character string (a line from an trn file).
** Comments are blank lines or any text following a "#" character.
** Whitespace characters can be included in a blank line.
**
** @param [u] text [AjPStr*] Line of text from input file
** @return [void]
** @@
******************************************************************************/

static void grpNoComment(AjPStr* text)
{
    ajint i;
    char *cp;

    ajStrChomp(text);
    i = ajStrLen(*text);

    if(!i)				/* empty string */
	return;

    cp = strchr(ajStrStr(*text), '#');
    if(cp)
    {					/* comment found */
	*cp = '\0';
	ajStrFix(*text);
    }

    return;
}




/* @funcstatic grpParseValueRB ************************************************
**
** Copied from ajacd.c
**
** Uses ajStrTok to complete a (possibly) quoted value.
** Note that ajStrTok has a stored internal copy of the text string
** which is set up at the start of acdParse and is being used here.
**
** Quotes can be single or double, or any kind of parentheses,
** depending on the first character of the next token examined.
**
** @param [u] tokenhandle [AjPStrTok*] Current parsing handle for input text
** @param [r] delim [char*] Delimiter string
** @return [AjPStr] String containing next value using acdStrTok
** @@
******************************************************************************/

static AjPStr grpParseValueRB(AjPStrTok* tokenhandle, char* delim)
{
    static AjPStr strp    = NULL;
    static AjPStr tmpstrp = NULL;
    char  endq[]   = " ";
    char  endqbr[] = " ]";
    ajint iquote;
    char *cq;
    AjBool done   = ajFalse;
    AjBool rightb = ajFalse;

    char *quotes    = "\"'{(<";
    char *endquotes = "\"'})>";

    if(!ajStrToken(&strp, tokenhandle, delim))
	return NULL;

    cq = strchr(quotes, ajStrChar(strp, 0));
    if(!cq)
	return strp;


    /* quote found: parse up to closing quote then strip white space */

    ajStrDelReuse(&tmpstrp);

    iquote = cq - quotes;
    endq[0] = endqbr[0] = endquotes[iquote];
    ajStrTrim(&strp, 1);

    while(!done)
    {
	if(ajStrSuffixC(strp, endq))
	{
	    ajStrTrim(&strp, -1);
	    done = ajTrue;
	}

	if(ajStrSuffixC(strp, endqbr))
	{
	    ajStrTrim(&strp, -2);
	    rightb = ajTrue;
	    done = ajTrue;
	}

	if(ajStrLen(strp))
	{
	    if(ajStrLen(tmpstrp))
	    {
		ajStrAppC(&tmpstrp, " ");
		ajStrApp(&tmpstrp, strp);
	    }
	    else
		ajStrAssS(&tmpstrp, strp);
	}

	if(!done)
	    if(!ajStrToken(&strp, tokenhandle, delim))
		return NULL;
    }

    if(rightb)
	ajStrAppC(&tmpstrp, "]");

    return tmpstrp;
}




/* @funcstatic grpSplitList ***************************************************
**
** Split a string containing group names into a list on the delimiters
** ',' or ';' to form the primary names of the groups.
** Any names containing a colon ':' are optionally expanded in a call to
** grpSubSplitList() to form many combinations of group names.
**
** The group names are returned as a list.
**
** @param [u] groups [AjPList] List of groups
** @param [r]  value  [AjPStr] Groups string from ACD file
** @param [r]  explode [AjBool] Expand group names around ':'
** @param [r]  colon [AjBool] Retain ':' in group names
**
** @return [void]
** @@
******************************************************************************/

static void grpSplitList(AjPList groups, AjPStr value, AjBool explode,
			 AjBool colon)
{
    AjPStrTok colontokenhandle;
    AjPStrTok tokenhandle;
    char delim[]       = ",;";
    char colonstring[] = ":";
    AjPList subnames;
    AjPStr tmpstr  = NULL;
    AjPStr substr  = NULL;
    AjPStr copystr = NULL;


    tokenhandle = ajStrTokenInit(value, delim);

    while(ajStrToken(&tmpstr, &tokenhandle, NULL))
    {
	ajStrChomp(&tmpstr);
	ajStrTrimC(&tmpstr, ".");

	/*
	** split the group name on colons and expand the sub-names into several
	** combinations of name
	*/
	if(explode)
	{
	    subnames = ajListstrNew();
	    colontokenhandle  = ajStrTokenInit(tmpstr, colonstring);
	    while(ajStrToken(&substr, &colontokenhandle, NULL))
	    {
		copystr = ajStrDup(substr); /* make new copy of the string
					       for the list to hold */
		ajStrChomp(&copystr);
		ajListstrPushApp(subnames, copystr);
	    }

	    /*
	    ** make the combinations of sub-names and add them to the list of
	    ** group names
	    */
	    grpSubSplitList(groups, subnames);

	    ajStrTokenClear(&colontokenhandle);
	    ajStrDel(&substr);
	    ajListstrFree(&subnames);
	    /*
	     ** don't free up copystr - because ajListstrFree()
	     ** then tries to free
	     ** it as well ajStrDel(&copystr);
	     */

	}
	else
	{
	    /*
	    ** don't explode, just remove ':'s and excess spaces and add to
	    ** 'groups' list
	    */
	    copystr = ajStrDup(tmpstr);	/* make new copy of the string
					   for the list to hold */
	    /*
	     ** we might want to retain the ':' in the output
	     ** if it is being parsed by
	     ** other programs that create 2-level menus for an interface etc.
	     */
	    if(!colon)
	    {
		ajStrConvertCC(&copystr, ":", " ");
		ajStrClean(&copystr);
	    }
	    else
	    {
		/* tidy up spurious spaces around the colon */
		ajStrClean(&copystr);
		ajStrSubstituteCC(&copystr, " :", ":");
		ajStrSubstituteCC(&copystr, ": ", ":");
	    }

	    ajListstrPushApp(groups, copystr);
	}
    }

    ajStrTokenClear(&tokenhandle);
    ajStrDel(&tmpstr);

    return;
}




/* @funcstatic grpSubSplitList ************************************************
**
** Takes a list of words and makes several combinations of them to
** construct the expanded group constructs made from the ':' operator in
** the group names.
**
** For example, the group name 'aaa:bbb:ccc' will be passed over to this
** routine as the list 'aaa', 'bbb', 'ccc' and the group names:
** 'aaa bbb ccc'
** 'ccc bbb aaa'
** 'aaa bbb'
** 'bbb aaa'
** 'bbb ccc'
** 'ccc bbb'
** 'ccc'
** 'bbb'
** 'aaa'
** will be constructed and added to the list of group names in 'groups'
**
** @param [u] groups [AjPList] List of groups
** @param [r] sublist [AjPList] (Sub)-names of groups string from ACD file
**
** @return [void]
** @@
******************************************************************************/

static void grpSubSplitList(AjPList groups, AjPList sublist)
{
    AjPStr *sub;			/* array of sub-names */
    ajint len;			    /* length of array of sub-names */
    ajint i;
    ajint j;
    AjPStr head;			/* constructed group names */
    AjPStr tail;
    AjPStr revhead;
    AjPStr revtail;
    AjPStr dummy;		 /* dummy string for ajListstrPop() */


    len = ajListstrToArray(sublist, &sub);

    for(i=0; i<len; i++)
    {
	/* do head of list */
	head = ajStrNew();
	for(j=0; j<=i; j++)
	{
	    if(ajStrLen(head) > 0)
		ajStrAppC(&head, " ");
	    ajStrApp(&head, sub[j]);
	}

	ajListstrPushApp(groups, head);

	/*
	** do reverse head of list if there is more than
	** one name in the head
	*/
	if(i)
	{
	    revhead = ajStrNew();
	    for(j=i; j>=0; j--)
	    {
		if(ajStrLen(revhead) > 0)
		    ajStrAppC(&revhead, " ");
		ajStrApp(&revhead, sub[j]);
	    }

	    ajListstrPushApp(groups, revhead);
	}

	/* do tail of list, if there is any tail left */
	if(i < len-1)
	{
	    tail = ajStrNew();
	    for(j=i+1; j<len; j++)
	    {
		if(ajStrLen(tail) > 0)
		    ajStrAppC(&tail, " ");
		ajStrApp(&tail, sub[j]);
	    }

	    ajListstrPushApp(groups, tail);
	}

	/*
	** do reverse tail of list if there is more than
	** one name in the tail
	*/
	if(i < len-2)
	{
	    revtail = ajStrNew();
	    for(j=len-1; j>i; j--)
	    {
		if(ajStrLen(revtail) > 0) ajStrAppC(&revtail, " ");
		ajStrApp(&revtail, sub[j]);
	    }

	    ajListstrPushApp(groups, revtail);
	}

    }

    AJFREE(sub);

    /* if list length is greater than 2, pop off head and tail and recurse */
    if(len > 2)
    {
	ajListstrPop(sublist, &dummy);	/* remove first node */
	ajStrDel(&dummy);

	/* remove last node of list. */

	ajListstrReverse(sublist);
	ajListstrPop(sublist, &dummy);	/* remove first node */
	ajStrDel(&dummy);
	ajListstrReverse(sublist);

	/* recurse */
	grpSubSplitList(groups, sublist);
    }

    return;
}




/* @funcstatic grpAddGroupsToList *********************************************
**
** Add application to applications list and its groups to the full groups list
** Results are alpha list and glist.
**
** alpha is a list of application with sub-lists of their groups
** glist is a list of groups with sub-lists of their applications
**
** @param [u] alpha [AjPList] Alphabetic list of programs
** @param [u] glist [AjPList] List of all known groups
** @param [u] groups [AjPList] List of groups for this application
** @param [r]  appl  [AjPStr] Application name
** @param [r]  doc  [AjPStr] Documentation string
**
** @return [void]
** @@
******************************************************************************/

static void grpAddGroupsToList(AjPList alpha, AjPList glist, AjPList groups,
			       AjPStr appl, AjPStr doc)
{
    AjPStr g = NULL;	/* temporary value of member of groups list */
    AjIList aiter;	/* 'alpha' iterator */
    AjIList iter;	/* 'groups' iterator */
    AjIList giter;	/* 'glist' iterator */
    AjIList niter;	/* 'nlist' iterator */
    GPnode al;		/* next (first) member of alpha */
    GPnode gl;		/* next member of glist */
    GPnode nl;		/* next member of nlist */
    GPnode gpnode;	/* new member of glist being added */
    GPnode ppnode;	/* new member of plist being added */
    GPnode apnode;	/* new member of alpha list being added */
    AjPList nlist=NULL;	/* list of programs in a group - used to check
			   name is unique */
    AjBool foundit;	/* flag for found the program name */


    /* add this program to the alphabetic list of programs */
    apnode = embGrpMakeNewPnode(appl, doc);
    aiter = ajListIter(alpha);
    al = ajListIterNext(aiter);
    ajListPushApp(al->progs, apnode);
    ajListIterFree(aiter);

    /*
    ** Now step through all groups that this program belongs to and add this
    ** program to the groups
    */
    iter = ajListIter(groups);

    while((g = ajListIterNext(iter)) != NULL)
    {
	/* add the group name to the program node in alpha list */
	gpnode = embGrpMakeNewGnode(g);
	ajListPushApp(apnode->progs, gpnode);

	/* add the application to the appropriate groups list in glist */
	giter = ajListIter(glist);

	while((gl = ajListIterNext(giter)) != NULL)
	{
	    /* is this our group ? */
	    if(!ajStrCmpCase(gl->name, g))
	    {
		/*
		** found the group.
		** look through the program names in this group
		** and only add if name is
		** not already there
		*/
		foundit = ajFalse;
		nlist   = gl->progs;
		niter   = ajListIter(nlist);
		while((nl = ajListIterNext(niter)) != NULL)
		{
		    if(!ajStrCmpCase(nl->name, appl))
		    {
			/* found the program name */
			foundit = ajTrue;
			break;
		    }
		}
		ajListIterFree(niter);

		if(!foundit)
		{
		    ppnode = embGrpMakeNewPnode(appl, doc);
		    ajListPushApp(gl->progs, ppnode);
		}
		break;
	    }
	}

	if(gl == NULL)
	{
	    /* went past the end of the group list */
	    gpnode = embGrpMakeNewGnode(g);
	    ajListPushApp(glist, gpnode);
	    ppnode = embGrpMakeNewPnode(appl, doc);
	    ajListPushApp(gpnode->progs, ppnode);
	}
	ajListIterFree(giter);
    }

    ajListIterFree(iter);
    ajStrDel(&g);

    /* sort the groups for this application in alpha list */
    embGrpSortGroupsList(apnode->progs);

    return;
}




/* @func embGrpMakeNewGnode ***************************************************
**
** Creates a new pointer to a Gnode struct for holding a group's
** name and pointer to a list of programs (also held in Gnodes).
**
** @param [r] name [AjPStr] Name of the group
** @return [GPnode] pointer to a new GPnode struct
** @@
******************************************************************************/

GPnode embGrpMakeNewGnode(AjPStr name)
{
    GPnode gpnode;
    AjPStr newstr = NULL;
    AjPStr dummy  = NULL;

    ajStrToUpper(&name);

    /*  ajDebug("New groups gnode name=%S\n", name); */
    AJNEW0(gpnode);

    newstr = ajStrNewS(name);

    gpnode->name = newstr;
    ajStrAssC(&dummy, "");
    gpnode->doc = dummy;
    gpnode->progs = ajListNew();

    return gpnode;
}




/* @func embGrpMakeNewPnode ***************************************************
**
** Creates a new pointer to a Gnode struct for holding a program's
** name and documentation.
**
** @param [r] name [AjPStr] Name of the program
** @param [r] doc [AjPStr] Description of the program
** @return [GPnode] pointer to a new gnode struct
** @@
******************************************************************************/

GPnode embGrpMakeNewPnode(AjPStr name, AjPStr doc)
{
    GPnode gpnode;
    AjPStr newstr = NULL;

    AJNEW0(gpnode);
    newstr        = ajStrDup(name);
    gpnode->name  = newstr;
    newstr        = ajStrDup(doc);
    gpnode->doc   = newstr;
    gpnode->progs = ajListNew();

    return gpnode;
}




/* @func embGrpSortGroupsList *************************************************
**
** Sort a list of GPnodes by their name.
**
** @param [u] groupslist [AjPList] List to sort
** @return [void]
** @@
******************************************************************************/

void embGrpSortGroupsList(AjPList groupslist)
{
    GPnode gl;
    AjIList giter;

    /* sort the programs for each group */
    giter = ajListIter(groupslist);

    while((gl = ajListIterNext(giter)) != NULL)
	ajListSort(gl->progs, embGrpCompareTwoGnodes);

    ajListIterFree(giter);

    /* sort the groups themselves */
    ajListSort(groupslist, embGrpCompareTwoGnodes);

    return;
}




/* @func embGrpCompareTwoGnodes ***********************************************
**
** Compare two Gnodes as case-insensitive strings.
**
** @param [r] a [const void *] First node
** @param [r] b [const void *] Second node
**
** @return [ajint] Compare value (-1, 0, +1)
** @@
******************************************************************************/

ajint embGrpCompareTwoGnodes(const void * a, const void * b)
{
    return ajStrCmpCase((*(GPnode *)a)->name,(*(GPnode *)b)->name);
}




/* @func embGrpOutputGroupsList ***********************************************
**
** Displays a list of groups to an output file handle.
**
** @param [r] outfile [AjPFile] Output file handle
** @param [r] groupslist [AjPList] List of groups to be displayed
** @param [r] showprogs [AjBool] If True, display the programs in each group
** @param [r] html [AjBool] If True, format for HTML, else make a simple list
** @param [r] link1 [AjPStr] URL to put in front of group name to make a link
** @param [r] link2 [AjPStr] string (eg. '.html') to put after group name
** @return [void]
** @@
******************************************************************************/

void embGrpOutputGroupsList(AjPFile outfile, AjPList groupslist,
			    AjBool showprogs, AjBool html, AjPStr link1,
			    AjPStr link2)
{
    GPnode gl;
    AjIList giter;			/* 'groupslist' iterator */

    /* output the programs for each group */
    giter = ajListIter(groupslist);
    if(!showprogs && html)
	ajFmtPrintF(outfile,"<ul>\n");

    while((gl = ajListIterNext(giter)) != NULL)
    {
	if(html)
	{
	    if(showprogs)
		ajFmtPrintF(outfile,"<h2><a name=\"%S\">%S</a></h2>\n",
			    gl->name, gl->name);
	    else
	    {
		if(ajStrLen(link1) || ajStrLen(link2))
		    ajFmtPrintF(outfile,"<li><a href=\"%S%S%S\">%S</a></li>\n",
				link1,gl->name,link2,gl->name);
		else
		    ajFmtPrintF(outfile,"<li>%S</li>\n", gl->name);
	    }
	}
	else
	    ajFmtPrintF(outfile,"%S\n", gl->name);

	if(showprogs)
	{
	    if(html) ajFmtPrintF(outfile,"<table border cellpadding=4 "
				 "bgcolor=\"#FFFFF0\">\n");
	    embGrpOutputProgsList(outfile, gl->progs, html, link1, link2);
	    if(html)
		ajFmtPrintF(outfile,"</table>\n");
	    else
		ajFmtPrintF(outfile,"\n");
	}
    }

    if(!showprogs && html)
	ajFmtPrintF(outfile,"</ul>\n");
    ajListIterFree(giter);

    return;
}




/* @func embGrpOutputProgsList ************************************************
**
** Displays a list of programs and their descriptions to an output file handle.
**
** @param [r] outfile [AjPFile] Output file handle
** @param [r] progslist [AjPList] List of programs to be displayed
** @param [r] html [AjBool] If True, format for HTML, else make a simple list
** @param [r] link1 [AjPStr] URL to put in front of program name to make a link
** @param [r] link2 [AjPStr] string (eg. '.html') to put after program name
** @return [void]
** @@
******************************************************************************/

void embGrpOutputProgsList(AjPFile outfile, AjPList progslist, AjBool html,
			   AjPStr link1, AjPStr link2)
{
    GPnode pl;
    AjIList piter;			/* 'progslist' iterator */

    /* output the programs for each group */
    piter = ajListIter(progslist);
    if(html) ajFmtPrintF(outfile,
			 "<tr><th>Program name</th><th>Description"
			 "</th></tr>\n");

    while((pl = ajListIterNext(piter)) != NULL)
    {
	if(html)
	{
	    if(ajStrLen(link1) || ajStrLen(link2))
			ajFmtPrintF(outfile,
			    "<tr><td><a href=\"%S%S%S\">%S</a></td>"
			    "<td>%S</td></tr>\n",
			    link1, pl->name, link2, pl->name, pl->doc);
	    else
		ajFmtPrintF(outfile,
			    "<tr><td>%S</td><td>%S</td></tr>\n",
			    pl->name, pl->doc);
	}
	else
	    ajFmtPrintF(outfile, "%-16S %S\n", pl->name, pl->doc);
    }

    ajListIterFree(piter);

    return;
}




/* @func embGrpGroupsListDel **************************************************
**
** Destructor for a groups list
**
** @param [r] groupslist [AjPList*] List of groups to be destroyed
** @return [void]
** @@
******************************************************************************/

void embGrpGroupsListDel(AjPList *groupslist)
{
    GPnode gl;
    AjIList giter;

    giter = ajListIter(*groupslist);
    while((gl = ajListIterNext(giter)) != NULL)
    {
	ajStrDel(&(gl->doc));
	ajStrDel(&(gl->name));
	embGrpGroupsListDel(&(gl->progs));
	AJFREE(gl);
    }

    ajListIterFree(giter);
    ajListFree(groupslist);

    return;
}




/* @func embGrpKeySearchProgs *************************************************
**
** Searches a list of groups and programs for (partial) matches to a keyword
**
** @param [w] newlist [AjPList] List of matching GPnode struct returned
** @param [r] glist [AjPList] List of GPnode struct to search through
** @param [r] key [AjPStr] String to search for
** @return [void]
** @@
******************************************************************************/

void embGrpKeySearchProgs(AjPList newlist, AjPList glist, AjPStr key)
{
    AjIList giter;		/* 'glist' iterator */
    AjIList piter;		/* 'plist' iterator */
    GPnode gl;			/* next member of glist */
    GPnode gpnode;		/* new member of glist being added */
    GPnode pl;			/* next member of plist */
    GPnode ppnode;		/* new member of plist being added */
    AjPStr gname = NULL;
    AjPStr name  = NULL;
    AjPStr doc   = NULL;

    /*
    ** compare case independently - so use upper case of both key
    ** and name/doc
    */
    ajStrToUpper(&key);

    /* make new group */
    ajStrAssC(&gname, "Search for '");
    ajStrApp(&gname, key);
    ajStrAppC(&gname, "'");
    gpnode = embGrpMakeNewGnode(gname);
    ajListPushApp(newlist, gpnode);

    giter = ajListIter(glist); /* iterate through existing groups list */

    while((gl = ajListIterNext(giter)) != NULL)
    {
	piter = ajListIter(gl->progs);
	while((pl = ajListIterNext(piter)) != NULL)
	{
	    ajStrAss(&name, pl->name);
	    ajStrAss(&doc, pl->doc);
	    ajStrToUpper(&name);
	    ajStrToUpper(&doc);

	    if(strstr(ajStrStr(doc), ajStrStr(key)) != NULL ||
	       strstr(ajStrStr(name), ajStrStr(key)) != NULL)
	    {
		ppnode = embGrpMakeNewPnode(pl->name, pl->doc);
		ajListPushApp(gpnode->progs, ppnode);
	    }

	    ajStrDel(&name);
	    ajStrDel(&doc);
	}
	ajListIterFree(piter);
    }
    ajListIterFree(giter);

    /* sort the results */
    embGrpSortGroupsList(newlist);

    ajStrDel(&gname);

    return;
}




/* @func embGrpKeySearchSeeAlso ***********************************************
**
** Takes an application name and returns a list of the groups that the
** application belongs to and a list of the applications that are in
** those groups.
**
** If the program we are searching for is not found, it returns *appgroups
** as NULL.
**
** @param [u] newlist [AjPList] List of application GPnode returned
** @param [w] appgroups [AjPList *] List of GPnode groups of program returned
** @param [r] alpha [AjPList] List of GPnode struct to search through
** @param [r] glist [AjPList] List of GPnode struct to search through
** @param [r] key [AjPStr] program name to search for
** @return [void]
** @@
******************************************************************************/

void embGrpKeySearchSeeAlso(AjPList newlist, AjPList *appgroups,
			    AjPList alpha, AjPList glist, AjPStr key)
{

    AjIList giter;	/* 'glist' iterator */
    AjIList piter;	/* 'plist' iterator */
    AjIList griter;	/* iterator through the list of groups we have found */
    GPnode gl;		/* next member of glist */
    GPnode gpnode;	/* new member of glist being added */
    GPnode pl;		/* next member of plist */
    GPnode gr;		/* next member of list of groups we have found */
    AjPStr tmp = NULL;
    AjPList base;

    /* make initial group node and push on newlist */
    tmp = ajStrNewC("See also");
    gpnode = embGrpMakeNewGnode(tmp);
    base = gpnode->progs;
    ajListPushApp(newlist, gpnode);


    /*
    **  set *appgroups to NULL initially - test to see if still NULL after
    **  we have searched for the application name
    **/
    *appgroups = NULL;

    /*
    **  initially look for our application in list 'alpha' to get its list of
    **  groups
    **/

    /* iterate through existing applications list */
    giter = ajListIter(alpha);
    while((gl = ajListIterNext(giter)) != NULL)
    {
	piter = ajListIter(gl->progs);
	while((pl = ajListIterNext(piter)) != NULL)
	    if(!ajStrCmpCase(pl->name, key))
		*appgroups = pl->progs;
	ajListIterFree(piter);
    }
    ajListIterFree(giter);

    /* If application not found */
    if(*appgroups == NULL)
	return;

    /*
    ** go through each group in glist finding those that are
    ** used by the application
    */

    /* iterate through existing applications list */
    giter = ajListIter(glist);
    while((gl = ajListIterNext(giter)) != NULL)
    {
	griter = ajListIter(*appgroups); /* iterate through groups we found */
	while((gr = ajListIterNext(griter)) != NULL)
	{
	    if(!ajStrCmpCase(gr->name, gl->name))
	    {
		/*
		**  found one of the groups - pull out
		**  the applications
		**/
		piter = ajListIter(gl->progs);
		while((pl = ajListIterNext(piter)) != NULL)
		{
		    /* don't want to include our key program */
		    if(!ajStrCmpO(pl->name, key))
			continue;

		    /* make new application node and push on base */
		    gpnode = embGrpMakeNewPnode(pl->name, pl->doc);
		    ajListPushApp(base, gpnode);

		}
		ajListIterFree(piter);

	    }
	}
	ajListIterFree(griter);
    }
    ajListIterFree(giter);

    /* sort the results and remove duplicates */
    embGrpSortGroupsList(base);
    embGrpMakeUnique(base);

    ajStrDel(&tmp);

    return;
}




/* @func embGrpMakeUnique *****************************************************
**
** Takes a sorted GPnode list and ensures that there are no duplicate
** group or application names in that list.
**
** @param [u] list [AjPList] List of application GPnode returned
** @return [void]
** @@
******************************************************************************/

void embGrpMakeUnique(AjPList list)
{
    AjIList iter;
    GPnode l;				/* next member of list */
    AjPStr old = NULL;			/* previous name */

    old = ajStrNewC("");

    iter = ajListIter(list);
    while((l = ajListIterNext(iter)) != NULL)
    {

	if(!ajStrCmpCase(l->name, old))
	{

	    /* delete this GPnode's lists and data */
	    embGrpGroupsListDel(&l->progs);
	    ajStrDel(&(l->doc));
	    ajStrDel(&(l->name));
	    AJFREE(l);

	    /* delete this element of the list */
	    ajListRemove(iter);

	}
	else
	{
	    ajStrDel(&old);
	    old = ajStrDup(l->name);
	    embGrpMakeUnique(l->progs);
	}

    }
    ajListIterFree(iter);

    ajStrDel(&old);

    return;
}



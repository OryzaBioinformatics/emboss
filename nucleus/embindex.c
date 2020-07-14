/* @source embindex.c
**
** B+ Tree Indexing plus Disc Cache.
** Copyright (c) 2003 Alan Bleasby
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

#define BTENTRYFILE     ".ent"
#define KWLIMIT 12




static AjPFile btreeCreateFile(const AjPStr idirectory, const AjPStr dbname,
			       const char *add);




/* @func embBtreeEmblKW **************************************************
**
** Extract keywords from an EMBL KW line 
**
** @param [r] kwline[const AjPStr] keyword line
** @param [w] kwlist [AjPList] list of keywords
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblKW(const AjPStr kwline, AjPList kwlist)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssC(&line, &MAJSTRSTR(kwline)[5]);

    handle = ajStrTokenInit(line,"\n;");

    while(ajStrToken(&token,&handle,NULL))
    {
	ajStrTrimEndC(&token,".");
	ajStrChomp(&token);
	if(ajStrLen(token))
	{
	    str = ajStrNew();
	    if(ajStrLen(token) > KWLIMIT)
		ajStrAssSub(&str,token,0,KWLIMIT-1);
	    else
		ajStrAssS(&str,token);
	    ajListPush(kwlist,(void *)str);
	}
    }

    ajStrDel(&token);
    ajStrTokenClear(&handle);
    ajStrDel(&line);
    
    return;
}




/* @func embBtreeEmblDE **************************************************
**
** Extract keywords from an EMBL DE line 
**
** @param [r] deline[const AjPStr] description line
** @param [w] kwlist [AjPList] list of keywords
**
** @return [void]
** @@
******************************************************************************/

void embBtreeEmblDE(const AjPStr deline, AjPList kwlist)
{
    AjPStr line      = NULL;
    AjPStrTok handle = NULL;
    AjPStr token     = NULL;
    AjPStr str       = NULL;
    
    line  = ajStrNew();
    token = ajStrNew();
    
    ajStrAssC(&line, &MAJSTRSTR(deline)[5]);
    ajStrSubstituteKK(&line,',',' ');
    
    handle = ajStrTokenInit(line,"\n\t ");

    while(ajStrToken(&token,&handle,NULL))
    {
	ajStrTrimEndC(&token,".");
	ajStrChomp(&token);
	if(ajStrLen(token))
	{
	    str = ajStrNewC(MAJSTRSTR(token));
	    ajListPush(kwlist,(void *)str);
	}
    }
    
    return;
}




/* @func embBtreeReadDir ******************************************************
**
** Read files to index
**
** @param [w] filelist [AjPStr**] list of files to read
** @param [r] fdirectory [const AjPStr] Directory to scan
** @param [r] files [const AjPStr] Filename to search for (or NULL)
** @param [r] exclude [const AjPStr] list of files to exclude
**
** @return [ajint] number of matching files
** @@
******************************************************************************/

ajint embBtreeReadDir(AjPStr **filelist, const AjPStr fdirectory,
		      const AjPStr files, const AjPStr exclude)
{
    AjPList lfiles = NULL;
    ajint nfiles;
    ajint nremove;
    ajint i;
    ajint j;
    AjPStr file    = NULL;
    AjPStr *remove = NULL;

    /* ajDebug("In ajBtreeReadDir\n"); */

    lfiles = ajListNew();
    nfiles = ajFileScan(fdirectory,files,&lfiles,ajFalse,ajFalse,NULL,NULL,
			ajFalse,NULL);

    nremove = ajArrCommaList(exclude,&remove);
    
    for(i=0;i<nfiles;++i)
    {
	ajListPop(lfiles,(void **)&file);
	ajSysBasename(&file);
	for(j=0;j<nremove && ! ajStrMatchWild(file,remove[j]);++j);
	if(j == nremove)
	    ajListPushApp(lfiles,(void *)file);
    }

    nfiles =  ajListToArray(lfiles,(void ***)&(*filelist));
    ajListDel(&lfiles);

    for(i=0; i<nremove;++i)
	ajStrDel(&remove[i]);
    AJFREE(remove);

    return nfiles;
}




/* @func embBtreeWriteFileList ***********************************************
**
** Read files to index
**
** @param [r] filelist [const AjPStr*] list of files
** @param [r] nfiles [ajint] number of files
** @param [r] fdirectory [const AjPStr] flatfile directory
** @param [r] idirectory [const AjPStr] index directory
** @param [r] dbname [const AjPStr] name of database
**
** @return [AjBool] true if success
** @@
******************************************************************************/

AjBool embBtreeWriteFileList(const AjPStr *filelist, ajint nfiles,
			     const AjPStr fdirectory, const AjPStr idirectory,
			     const AjPStr dbname)
{
    AjPFile entfile = NULL;
    ajint i;
    
    /* ajDebug("In ajBtreeWriteFileList\n"); */

    entfile = btreeCreateFile(idirectory,dbname,BTENTRYFILE);
    if(!entfile)
	return ajFalse;
    
    ajFmtPrintF(entfile,"#Number of files\t%d\n",nfiles);
    for(i=0;i<nfiles;++i)
	ajFmtPrintF(entfile,"%S/%S\n",fdirectory,filelist[i]);

    ajFileClose(&entfile);
    
    return ajTrue;
}




/* @funcstatic btreeCreateFile ************************************************
**
** Open B+tree file for writing
**
** @param [r] idirectory [const AjPStr] Directory for index files
** @param [r] dbname [const AjPStr] name of database
** @param [r] add [const char *] type of file
**
** @return [AjPFile] opened file
** @@
******************************************************************************/

static AjPFile btreeCreateFile(const AjPStr idirectory, const AjPStr dbname,
			       const char *add)
{
    AjPStr filename = NULL;
    AjPFile fp      = NULL;
    
    /* ajDebug("In btreeCreateFile\n"); */

    filename = ajStrNew();

    ajFmtPrintS(&filename,"%S/%S%s",idirectory,dbname,add);
    
    fp =  ajFileNewOut(filename);

    ajStrDel(&filename);
    return fp;
}

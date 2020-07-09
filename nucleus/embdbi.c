/*
**
** EMBOSS database indexing routines
**
*/

#include "emboss.h"
#include <dirent.h>
#include <sys/wait.h>


/* @func embDbiAcnumNew *******************************************************
**
** Constructor for accession structures.
**
** @return [EmbPac] Accession structure.
******************************************************************************/

EmbPac embDbiAcnumNew (void) {

  EmbPac ret;
  AJNEW0 (ret);

  return ret;
}


/* @func embDbiCmpId **********************************************************
**
** Comparison function for two entries.
**
** @param [r] a [const void*] First id (EmbPentry*)
** @param [r] b [const void*] Second id (EmbPentry*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpId (const void* a, const void* b) {

  EmbPentry aa = *(EmbPentry*) a;
  EmbPentry bb = *(EmbPentry*) b;

  return strcmp(aa->entry, bb->entry);
}

/* @func embDbiCmpAcId ********************************************************
**
** Comparison function for two accession entries.
**
** @param [r] a [const void*] First id (EmbPac*)
** @param [r] b [const void*] Second id (EmbPentryca*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpAcId (const void* a, const void* b) {
  EmbPac aa = *(EmbPac*) a;
  EmbPac bb = *(EmbPac*) b;

  return strcmp(aa->entry, bb->entry);
}

/* @func embDbiCmpAcAc ********************************************************
**
** Comparison function for two accession numbers.
**
** @param [r] a [const void*] First id (EmbPac*)
** @param [r] b [const void*] Second id (EmbPentryca*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpAcAc (const void* a, const void* b) {
  EmbPac aa = *(EmbPac*) a;
  EmbPac bb = *(EmbPac*) b;

  return strcmp(aa->ac, bb->ac);
}


/* @func embDbiEntryNew *******************************************************
**
** Constructor for entry structures.
**
** @return [EmbPentry] Entry structure.
******************************************************************************/

EmbPentry embDbiEntryNew (void) {

  EmbPentry ret;

  AJNEW0 (ret);

  return ret;
}

/* @funcstatic embDbiFileList ********************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [AjPStr] Directory
** @param [r] wildfile [AjPStr] Wildcard file name
** @return [AjPList] New list of all files with full paths
** @@ 
******************************************************************************/

AjPList embDbiFileList (AjPStr dir, AjPStr wildfile, AjBool trim) {

  AjPList retlist=NULL;
    
  DIR* dp;
  struct dirent* de;
  int dirsize;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;
  AjPStr tmp;
  AjPStr s;
  AjPStr s2;
  AjPStr t;
  
  char *p;
  char *q;
  AjPList l;
  int ll;
  int i;
  AjBool d;
  
  ajDebug("embDbiFileList dir '%S' wildfile '%S'\n",
	  dir, wildfile);
  
  tmp = ajStrNewC(ajStrStr(wildfile));

  if (ajStrLen(dir))
    (void) ajStrAss (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dirfix, -1) != '/')
    (void) ajStrAppC (&dirfix, "/");

  (void) ajStrAppC(&wildfile,"*");

  dp = opendir (ajStrStr(dirfix));
  if (!dp)
    ajFatal("opendir failed on '%S'", dirfix);

  s = ajStrNew();
  l = ajListNew();
  dirsize = 0;
  retlist = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    if (!ajStrMatchWildCO(de->d_name, wildfile)) continue;
    (void) ajStrAssC(&s,de->d_name);
    p=q=ajStrStr(s);
    if (trim) {
      p=strrchr(p,(int)'.');
      if(p)
	*p='\0';
    }
    s2 = ajStrNewC(q);
    
    ll=ajListLength(l);
    
    d=ajFalse;
    for(i=0;i<ll;++i)
    {
	ajListPop(l,(void *)&t);
	if(ajStrMatch(t,s2))
	   d=ajTrue;
	ajListPushApp(l,(void *)t);
    }
    if(!d)
	ajListPush(l,(void *)s2);
    else
    {
	ajStrDel(&s2);
	continue;
    }

    dirsize++;
    ajDebug ("accept '%S'\n", s2);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%S", dirfix, s2);
    ajListstrPushApp (retlist, name);
  }

  if(!ajListLength(retlist))
      ajFatal("No match for file specification %S",tmp);

  while(ajListPop(l,(void *)&t))
      ajStrDel(&t);
  ajStrDel(&s);
  ajStrDel(&tmp);

  (void) closedir (dp);
  ajDebug ("%d files for '%S' '%S'\n", dirsize, dir, wildfile);

  return retlist;

}

/* @func embDbiFileListExc ****************************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [AjPStr] Directory
** @param [r] wildfile [AjPStr] Wildcard file list
** @param [r] exclude [AjPStr] Wildcard file list (NULL if none to exclude)
** @return [AjPList] New list of all files with full paths
** @@ 
******************************************************************************/

AjPList embDbiFileListExc (AjPStr dir, AjPStr wildfile, AjPStr exclude) {

  AjPList retlist = NULL;

  DIR* dp;
  struct dirent* de;
  ajint dirsize;
  AjPStr name = NULL;
  static AjPStr dirfix = NULL;
  static AjPStr fname = NULL;

  ajDebug("embDbiFileListExc dir '%S' wildfile '%S' exclude '%S'\n",
	  dir, wildfile, exclude);
  
  if (ajStrLen(dir))
    (void) ajStrAss (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dirfix, -1) != '/')
    (void) ajStrAppC (&dirfix, "/");

  ajDebug ("dirfix '%S'\n", dirfix);

  dp = opendir (ajStrStr(dirfix));
  if (!dp)
    ajFatal("opendir failed on '%S'", dirfix);

  dirsize = 0;
  retlist = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    ajStrAssC (&fname, de->d_name);
    if (exclude && !ajFileTestSkip(fname, exclude, wildfile, ajFalse))
      continue;
    dirsize++;
    ajDebug ("accept '%S'\n", fname);
    name = NULL;
    (void) ajFmtPrintS (&name, "%S%S", dirfix, fname);
    ajListstrPushApp (retlist, name);
  }

  (void) closedir (dp);
  ajDebug ("%d files for '%S' '%S'\n", dirsize, dir, wildfile);

  return retlist;
}

/* @func embDBiFlatOpenlib ****************************************************
**
** Open a flat file library
**
** @param [r] lname [AjPStr] Source file basename
** @param [r] libr [AjPFile*] Database file
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool embDbiFlatOpenlib(AjPStr lname, AjPFile* libr) {

  ajFileClose(libr);

  *libr = ajFileNewIn(lname);
  if(!*libr)
	ajFatal("Cannot open %S for reading",lname);
  if (!*libr) {
    ajErr(" cannot open library flat file: %S\n",
	    lname);
    return ajFalse;
  }
  
  return ajTrue;
}

/* @func embDbiRmFile ********************************************************
**
** Remove a file or a set of numbered files
**
** @param [r] dbname [AjPStr] Database name
** @param [r] ext [const char*] Base file extension
** @param [r] nfiles [ajint] Number of files, or zero for unnumbered.
** @param [r] cleanup [AjBool] If ajTrue, clean ip temporary files after
** @return [void]
** @@
******************************************************************************/

void embDbiRmFile (AjPStr dbname, const char* ext, ajint nfiles,
		   AjBool cleanup) {

  static AjPStr cmdstr = NULL;
  ajint i;

  if (!cleanup) return;

  if (nfiles) {
    ajFmtPrintS (&cmdstr, "rm ");
    for (i=1; i<= nfiles; i++) {
      ajFmtPrintAppS (&cmdstr, "%S%02d.%s ", dbname, i, ext);
    }
  }
  else
    ajFmtPrintS (&cmdstr, "rm %S.%s", dbname, ext);
    

  embDbiSysCmd (cmdstr);

  return;
}

/* @func embDbiRmFileI ********************************************************
**
** Remove a numbered file
**
** @param [r] dbname [AjPStr] Database name
** @param [r] ext [const char*] Base file extension
** @param [r] ifile [ajint] File number.
** @param [r] cleanup [AjBool] If ajTrue, clean ip temporary files after
** @return [void]
******************************************************************************/

void embDbiRmFileI (AjPStr dbname, const char* ext, ajint ifile,
		    AjBool cleanup) {

  static AjPStr cmdstr = NULL;

  if (!cleanup) return;

  ajFmtPrintS (&cmdstr, "rm %S%02d.%s ", dbname, ifile, ext);

  embDbiSysCmd (cmdstr);

  return;
}

/* @func embDbiSortFile *******************************************************
**
** Sort a file, or a set of numbered files, individually
**
** @param [r] dbname [AjPStr] Database name
** @param [r] ext1 [const char*] Input file extension
** @param [r] ext2 [const char*] Output file extension
** @param [r] nfiles [ajint] Number of files to sort (zero if unnumbered)
** @param [r] cleanup [AjBool] If ajTrue, clean ip temporary files after
** @param [r] sortopt [AjPStr] Extra options for the system sort
** @return [void]
** @@
******************************************************************************/

void embDbiSortFile (AjPStr dbname, const char* ext1, const char* ext2,
		     ajint nfiles, AjBool cleanup, AjPStr sortopt) {

  static AjPStr cmdstr = NULL;
  ajint i;
  static AjPStr infname = NULL;
  static AjPStr outfname = NULL;
  static AjPStr srtext = NULL;

  if (nfiles) {
    for (i=1; i<=nfiles; i++) {
      ajFmtPrintS (&infname, "%S%02d.%s ", dbname, i, ext1);
      ajFmtPrintS (&outfname, "%S%02d.%s.srt", dbname, i, ext1);
      if (sortopt)
	ajFmtPrintS (&cmdstr, "sort -o %S %S %S",
		     outfname, sortopt, infname);
      else
	ajFmtPrintS (&cmdstr, "sort -o %S %S",
		     outfname, infname);

      embDbiSysCmd (cmdstr);

      embDbiRmFileI (dbname, ext1, i, cleanup);
    }

    ajFmtPrintS (&cmdstr, "sort -m -o %S.%s %S",
		 dbname, ext2, sortopt);
    for (i=1; i<=nfiles; i++) {
      ajFmtPrintAppS (&cmdstr, " %S%02d.%s.srt", dbname, i, ext1);
    }
    embDbiSysCmd (cmdstr);

    ajFmtPrintS (&srtext, "%s.srt ", ext1);
    for (i=1; i<=nfiles; i++) {
      embDbiRmFileI (dbname, ajStrStr(srtext), i, cleanup);
    }
  }
  else {
    ajFmtPrintS (&infname, "%S.%s ", dbname, ext1);
    ajFmtPrintS (&outfname, "%S.%s", dbname, ext2);
    ajFmtPrintS (&cmdstr, "sort -o %S %S %S",
		 outfname, sortopt, infname);
    embDbiSysCmd (cmdstr);
    embDbiRmFile (dbname, ext1, 0, cleanup);
  }

  return;
}

/* @func embDbiSysCmd *********************************************************
**
** Fork a system command
**
** @param [r] cmdstr [AjPStr] Command line
** @return [void]
** @@
******************************************************************************/

void embDbiSysCmd (AjPStr cmdstr) {

  char** arglist = NULL;
  char* pgm;
  pid_t pid;
  ajint status;

  ajDebug ("forking '%S'", cmdstr);
  (void) ajSysArglist (cmdstr, &pgm, &arglist);

  pid=fork();
  if(pid==-1)
    ajFatal("System fork failed");


  if(!pid) {
    (void) execvp (pgm, arglist);
    return;
  }
  while(wait(&status)!=pid);

  ajSysArgListFree (&arglist);
  ajCharFree (pgm);

  return;
}

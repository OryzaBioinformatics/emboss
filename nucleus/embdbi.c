/*
**
** EMBOSS database indexing routines
**
*/

#include "emboss.h"
#include <dirent.h>
#include <sys/wait.h>


/* @func embDbiFieldNew *******************************************************
**
** Constructor for field token structures.
**
** @return [EmbPField] Field token structure.
******************************************************************************/

EmbPField embDbiFieldNew (void) {

  EmbPField ret;
  AJNEW0 (ret);

  return ret;
}


/* @func embDbiCmpId **********************************************************
**
** Comparison function for two entries.
**
** @param [r] a [const void*] First id (EmbPEntry*)
** @param [r] b [const void*] Second id (EmbPEntry*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpId (const void* a, const void* b) {

  EmbPEntry aa = *(EmbPEntry*) a;
  EmbPEntry bb = *(EmbPEntry*) b;

  return strcmp(aa->entry, bb->entry);
}

/* @func embDbiCmpFieldId *****************************************************
**
** Comparison function for the entrynames in two field structures.
**
** @param [r] a [const void*] First id (EmbPField*)
** @param [r] b [const void*] Second id (EmbPField*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpFieldId (const void* a, const void* b) {
  EmbPField aa = *(EmbPField*) a;
  EmbPField bb = *(EmbPField*) b;

  return strcmp(aa->entry, bb->entry);
}

/* @func embDbiCmpFieldField **************************************************
**
** Comparison function for two field token values
**
** @param [r] a [const void*] First id (EmbPField*)
** @param [r] b [const void*] Second id (EmbPField*)
** @return [ajint] Comparison value, -1, 0 or +1.
** @@
******************************************************************************/

ajint embDbiCmpFieldField (const void* a, const void* b) {
  ajint ret;

  EmbPField aa = *(EmbPField*) a;
  EmbPField bb = *(EmbPField*) b;

  ret = strcmp(aa->field, bb->field);
  if (ret)
    return ret;

  return strcmp(aa->entry, bb->entry);
}


/* @func embDbiEntryNew *******************************************************
**
** Constructor for entry structures.
**
** @param [r] nfields [ajint] Number of data fields to be included
** @return [EmbPEntry] Entry structure.
******************************************************************************/

EmbPEntry embDbiEntryNew (ajint nfields) {

  EmbPEntry ret;

  AJNEW0 (ret);
  AJCNEW0 (ret->nfield, nfields);
  AJCNEW0 (ret->field, nfields);
  return ret;
}

/* @func embDbiFileList *******************************************************
**
** Makes a list of all files in a directory matching a wildcard file name.
**
** @param [r] dir [AjPStr] Directory
** @param [r] wildfile [AjPStr] Wildcard file name
** @param [r] trim [AjBool] Expand to search, trim results
** @return [AjPList] New list of all files with full paths
** @@
******************************************************************************/

AjPList embDbiFileList (AjPStr dir, AjPStr wildfile, AjBool trim) {

  AjPList retlist=NULL;

  DIR* dp;
  struct dirent* de;
  int dirsize;
  static AjPStr wwildfile=NULL;
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

  ajStrAssS(&wwildfile,wildfile);

  tmp = ajStrNewC(ajStrStr(wwildfile));

  if (ajStrLen(dir))
    (void) ajStrAss (&dirfix, dir);
  else
    (void) ajStrAssC (&dirfix, "./");

  if (ajStrChar(dirfix, -1) != '/')
    (void) ajStrAppC (&dirfix, "/");

  if (trim) {
    (void) ajStrAppC(&wwildfile,"*");
  }

  dp = opendir (ajStrStr(dirfix));
  if (!dp)
    ajFatal("opendir failed on '%S'", dirfix);

  s = ajStrNew();
  l = ajListNew();
  dirsize = 0;
  retlist = ajListstrNew ();
  while ((de = readdir(dp))) {
    if (!de->d_ino) continue;	/* skip deleted files with inode zero */
    if (!ajStrMatchWildCO(de->d_name, wwildfile)) continue;
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

  ajListDel(&l);

  ajStrDel(&s);
  ajStrDel(&tmp);

  (void) closedir (dp);
  ajDebug ("%d files for '%S' '%S'\n", dirsize, dir, wwildfile);

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
    if (exclude && !ajFileTestSkip(fname, exclude, wildfile, ajFalse, ajFalse))
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

/* @func embDbiFlatOpenlib ****************************************************
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

/* @func embDbiRmFile *********************************************************
**
** Remove a file or a set of numbered files
**
** @param [r] dbname [AjPStr] Database name
** @param [r] ext [const char*] Base file extension
** @param [r] nfiles [ajint] Number of files, or zero for unnumbered.
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
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
      ajFmtPrintAppS (&cmdstr, "%S%03d.%s ", dbname, i, ext);
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
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
** @return [void]
******************************************************************************/

void embDbiRmFileI (AjPStr dbname, const char* ext, ajint ifile,
		    AjBool cleanup) {

  static AjPStr cmdstr = NULL;

  if (!cleanup) return;

  ajFmtPrintS (&cmdstr, "rm %S%03d.%s ", dbname, ifile, ext);

  embDbiSysCmd (cmdstr);

  return;
}

/* @func embDbiRmEntryFile ****************************************************
**
** Remove the sorted entryname file (kept until end of processing
** as it is the sorted list of all entries, used to count entries for
** field indexing.
**
** @param [r] dbname [AjPStr] Database name
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
** @return [void]
** @@
******************************************************************************/

void embDbiRmEntryFile (AjPStr dbname,  AjBool cleanup) {
  embDbiRmFile (dbname, "idsrt", 0, cleanup);
}

/* @func embDbiSortFile *******************************************************
**
** Sort a file, or a set of numbered files, individually
**
** @param [r] dbname [AjPStr] Database name
** @param [r] ext1 [const char*] Input file extension
** @param [r] ext2 [const char*] Output file extension
** @param [r] nfiles [ajint] Number of files to sort (zero if unnumbered)
** @param [r] cleanup [AjBool] If ajTrue, clean up temporary files after
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
      ajFmtPrintS (&infname, "%S%03d.%s ", dbname, i, ext1);
      ajFmtPrintS (&outfname, "%S%03d.%s.srt", dbname, i, ext1);
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
      ajFmtPrintAppS (&cmdstr, " %S%03d.%s.srt", dbname, i, ext1);
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

/* @func embDbiHeaderSize *****************************************************
**
** Updates the file header for an index file to include the correct file size.
**
** @param [r] file [AjPFile] Output file
** @param [r] filesize [ajint] File size (if known, can be rewritten)
** @param [r] recordcnt [ajint] Number of records
** @retyurn [void]
******************************************************************************/

void embDbiHeaderSize (AjPFile file, ajint filesize, ajint recordcnt) {

  ajFileSeek(file, 0, 0);

  ajFileWriteInt4 (file, filesize);	/* filesize */
  ajFileWriteInt4 (file, recordcnt);	/* #records */

  return;
}

/* @func embDbiHeader *********************************************************
**
** Writes the header for an index file. Resets the file pointer to beginning
** of file, and leaves the file pointer at the start of the first record.
**
** @param [r] file [AjPFile] Output file
** @param [r] filesize [ajint] File size (if known, can be rewritten)
** @param [r] recordcnt [ajint] Number of records
** @param [r] recordlen [short] Record length (bytes)
** @param [r] dbname [AjPStr] Database name (up to 20 characters used)
** @param [r] release [AjPStr] Release as a string (up to 10 characters used)
** @param [r] date [char[4]] Date dd,mm,yy,00
** @return [void]
******************************************************************************/

void      embDbiHeader (AjPFile file, ajint filesize,
			ajint recordcnt, short recordlen,
			AjPStr dbname, AjPStr release,
			char date[4]) {

  ajint i;
  static char padding[256];
  static AjBool firstcall = AJTRUE;

  if (firstcall) {
    for (i=0;i<256;i++)
      padding[i] = ' ';
    firstcall = ajFalse;
  }

  ajFileSeek(file, 0, 0);

  ajFileWriteInt4 (file, filesize);	/* filesize */

  ajFileWriteInt4 (file, recordcnt);	/* #records */

  ajFileWriteInt2 (file, recordlen);	/* recordsize */

    /* rest of the header */
  ajFileWriteStr  (file, dbname,  20); /* dbname */
  ajFileWriteStr  (file, release, 10); /* release */
  ajFileWriteByte (file, date[0]);	/* release date */
  ajFileWriteByte (file, date[1]);	/* release date */
  ajFileWriteByte (file, date[2]);	/* release date */
  ajFileWriteByte (file, date[3]);	/* release date */
  ajFileWrite (file, padding, 256, 1); /* padding 256 bytes */

  return;
}

/* @func embDbiFileSingle *****************************************************
**
** Builds a filename for a single temporary file to save IDs or some other
** index field, for example EMBL01.list
**
** @param [R] dbname [AjPStr] Database name
** @param [R] extension [char*] Filename extension.
** @param [R] num [ajint] Number for this file (start at 1)
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileSingle (AjPStr dbname, char* extension, ajint num) {
  AjPFile ret;
  static AjPStr filename=NULL;

  ajFmtPrintS (&filename, "%S%03d.%s", dbname, num, extension);
  ret = ajFileNewOut (filename);
  if(!ret)
    ajFatal("Cannot open %S for writing", filename);
  return ret;
}

/* @func embDbiFileIn *********************************************************
**
** Builds a filename for a summary file to read IDs or some other
** index field, for example EMBL.acnum_sort
**
** @param [R] dbname [AjPStr] Database name
** @param [R] extension [char*] Filename extension.
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileIn (AjPStr dbname, char* extension) {
  AjPFile ret;
  static AjPStr filename=NULL;

  ajFmtPrintS (&filename, "%S.%s", dbname, extension);
  ret = ajFileNewIn (filename);
  if(!ret)
    ajFatal("Cannot open %S for reading", filename);
  return ret;
}

/* @func embDbiFileOut ********************************************************
**
** Builds a filename for a summary file to save IDs or some other
** index field, for example EMBL.acnum_srt2
**
** @param [R] dbname [AjPStr] Database name
** @param [R] extension [char*] Filename extension.
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileOut (AjPStr dbname, char* extension) {
  AjPFile ret;
  static AjPStr filename=NULL;

  ajFmtPrintS (&filename, "%S.%s", dbname, extension);
  ret = ajFileNewOut (filename);
  if(!ret)
    ajFatal("Cannot open %S for writing", filename);
  return ret;
}

/* @func embDbiFileIndex ******************************************************
**
** Builds a filename for a summary file to save IDs or some other
** index field, for example EMBL.acsrt2
**
** @param [R] indexdir [AjPStr] Index directory
** @param [R] field [AjPStr] Field name
** @param [R] extension [char*] Filename extension.
** @return [AjPFile] Opened output file
**
******************************************************************************/

AjPFile embDbiFileIndex (AjPStr indexdir, AjPStr field, char* extension) {
  AjPFile ret;
  static AjPStr filename=NULL;

  ajFmtPrintS (&filename, "%S.%s", field, extension);
  ret = ajFileNewOutD (indexdir, filename);
  if(!ret)
    ajFatal("Cannot open %S for writing", filename);
  return ret;
}

/* @func embDbiWriteDivision **************************************************
**
** Writes the division index file
**
** @param [R] indexdir [AjPStr] Index directory
** @param [R] dbname [AjPStr] Database name
** @param [R] release [AjPStr] Release number as a string
** @param [R] date [char[4]] Date
** @param [R] maxfilelen [ajint] Max file length
** @param [R] nfiles [ajint] Number of files indexes
** @param [R] divfiles [AjPStr*] Division filenames
** @param [R] seqfiles [AjPStr*] Sequence filenames (or NULL if none)
** @return [void]
******************************************************************************/

void embDbiWriteDivision (AjPStr indexdir,
			  AjPStr dbname, AjPStr release, char date[4],
			  ajint maxfilelen, ajint nfiles,
			  AjPStr* divfiles, AjPStr* seqfiles)
{
    AjPFile divFile;
    AjPStr tmpfname=NULL;
    ajint i;
    ajint filesize;
    short recsize;

    ajStrAssC (&tmpfname, "division.lkp");
    divFile = ajFileNewOutD(indexdir, tmpfname);

    filesize = 256 + 44 + (nfiles * (maxfilelen+2));
    recsize = maxfilelen + 2;

    embDbiHeader (divFile, filesize, nfiles, recsize, dbname, release, date);

    for (i=0; i<nfiles; i++)
    {
        if (seqfiles)
	    embDbiWriteDivisionRecord (divFile, maxfilelen, (short)(i+1),
				       divfiles[i], seqfiles[i]);
	else
	    embDbiWriteDivisionRecord (divFile, maxfilelen, (short)(i+1),
				       divfiles[i], NULL);
    }

    ajFileClose (&divFile);
    ajStrDel(&tmpfname);

    return;
}

/* @func embDbiWriteDivisionRecord ********************************************
**
** Writes a record to the division lookup file
**
** @param [R] file [AjPFile] Index file
** @param [R] maxnamlen [ajint] Maximum name length
** @param [R] recnum [short] Record number
** @param [R] datfile [AjPStr] Data file name
** @param [R] seqfile [AjPStr] Seqeunce file name (or NULL if none)
** @return [void]
******************************************************************************/

void embDbiWriteDivisionRecord (AjPFile file, ajint maxnamlen, short recnum,
				AjPStr datfile, AjPStr seqfile) {

  static AjPStr recstr=NULL;

  ajFileWriteInt2 (file, recnum);

  if (ajStrLen(seqfile)) {
    ajFmtPrintS(&recstr, "%S %S", datfile, seqfile);
    ajFileWriteStr(file, recstr, maxnamlen);
  }
  else {
    ajFileWriteStr(file, datfile, maxnamlen);
  }

  return;
}

/* @func embDbiWriteEntryRecord ***********************************************
**
** Writes a record to the entryname index file
**
** @param [r] file [AjPFile] hit file
** @param [r] maxidlen [ajint] Maximum length for an id string
** @param [r] id [AjPStr] The id string for this entry
** @param [r] rpos [ajint] Data file offset
** @param [r] spos [ajint] sequence file offset
** @param [r] filenum [short] file number in division file
** @return [void]
******************************************************************************/

void embDbiWriteEntryRecord (AjPFile file, ajint maxidlen, AjPStr id,
			  ajint rpos, ajint spos, short filenum) {

  ajFileWriteStr (file, id, maxidlen);
  ajFileWriteInt4 (file, rpos);
  ajFileWriteInt4 (file, spos);
  ajFileWriteInt2 (file, filenum);

  return;
}

/* @func embDbiWriteHit *******************************************************
**
** Writes a record to the field hit (.hit) index file
**
** @param [r] file [AjPFile] hit file
** @param [r] idnum [ajint] Entry number (1 for the first) in the
**                          entryname file
** @return [void]
******************************************************************************/

void embDbiWriteHit (AjPFile file, ajint idnum) {

  ajFileWriteInt4 (file, idnum);

  return;
}

/* @func embDbiWriteTrg *******************************************************
**
** Writes a record to the field target (.trg) index file
**
** @param [r] file [AjPFile] hit file
** @param [r] maxfieldlen [ajint] Maximum field token length
** @param [r] idnum [ajint] First record number (1 for the first) in the
**                          field hit index file
** @param [r] idcnt [ajint] Number of entries for this field value
**                          in the field hit index file
** @param [r] hitstr [AjPStr] Field token string
** @return [void]
******************************************************************************/

void embDbiWriteTrg (AjPFile file, ajint maxfieldlen,
		     ajint idnum, ajint idcnt, AjPStr hitstr) {

  ajFileWriteInt4 (file, idnum);
  ajFileWriteInt4 (file, idcnt);
  ajFileWriteStr (file, hitstr, maxfieldlen);

  return;
}

/* @func embDbiSortOpen *******************************************************
**
** Open sort files for entries and all fields
**
** @param [r] alistfile [AjPFile*] Sort files for each field.
** @param [r] ifile [ajint] Input file number (used for temporary file names)
** @param [r] dbname [AjPStr] Database name (used for temporary file names)
** @param [r] fields [AjPStr*] Field names (used for temporary file names)
** @param [r] nfields [ajint] Number of fields
** @return [AjPFile] Sort file for entries
******************************************************************************/

AjPFile embDbiSortOpen (AjPFile* alistfile, ajint ifile,
			AjPStr dbname, AjPStr* fields, ajint nfields)
{
    AjPFile elistfile;
    ajint ifield;


    elistfile = embDbiFileSingle (dbname, "list", ifile+1);
    for (ifield=0;ifield < nfields; ifield++)
    {
	alistfile[ifield] = embDbiFileSingle (dbname,
					      ajStrStr(fields[ifield]),
					      ifile+1);
    }

    return elistfile;
}

/* @func embDbiSortClose ******************************************************
**
** Close the sort files for entries and all fields
**
** @param [r] elistfile [AjPFile*] Sort file for entries
** @param [r] alistfile [AjPFile*] Sort files for each field.
** @param [r] nfields [ajint] Number of fields
** @return [void]
******************************************************************************/

void embDbiSortClose (AjPFile* elistfile, AjPFile* alistfile, ajint nfields)
{
  ajint ifield;

  ajFileClose (elistfile);
  for (ifield=0; ifield < nfields; ifield++)
  {
      ajFileClose (&alistfile[ifield]);
  }

  return;
}

/* @func embDbiMemEntry *******************************************************
**
** Stores data for current entry in memory by appending to lists
**
** @param [r] idlist [AjPList] List if entry IDs
** @param [r] fieldList [AjPList*] List of field tokens for each field
** @param [r] nfields [ajint] Number of fields
** @param [r] entry [EmbPEntry] Current entry
** @param [r] ifile [ajint] Current input file number
** @return [void]
******************************************************************************/

void embDbiMemEntry (AjPList idlist, AjPList* fieldList, ajint nfields,
		     EmbPEntry entry, ajint ifile)
{
    ajint ifield;
    ajint i;
    EmbPField fieldData=NULL;

    entry->filenum = ifile+1;
    ajListPushApp (idlist, entry);
    for (ifield=0; ifield < nfields; ifield++)
    {
	for (i=0;i<entry->nfield[ifield]; i++)
	{
	    fieldData = embDbiFieldNew();
	    fieldData->entry = entry->entry;
	    fieldData->field = entry->field[ifield][i];
	    ajListPushApp (fieldList[ifield], fieldData);
	}
    }
    return;
}

/* @func embDbiSortWriteEntry *************************************************
**
** Write the entryname index file using data from the entry sort file.
**
** @param [r] entFile [AjPFile] Entry file
** @param [r] maxidlen [ajint] Maximum id length
** @param [r] dbname [AjPStr] Database name (used in temp file names)
** @param [r] nfiles [ajint] Number of files
** @param [r] cleanup [AjBool] Cleanup temp files if true
** @param [r] sortopt [AjPStr] Sort commandline options
** @return [ajint] Number of entries
******************************************************************************/

ajint embDbiSortWriteEntry (AjPFile entFile, ajint maxidlen,
			    AjPStr dbname, ajint nfiles,
			    AjBool cleanup, AjPStr sortopt)
{
    AjPFile esortfile;
    static AjPStr lastidstr=NULL;
    static AjPStr idstr=NULL;
    static AjPStr tmpstr=NULL;
    static AjPStr rline=NULL;
    AjPRegexp idsrtexp = NULL;
    ajint rpos;
    ajint spos;
    ajint filenum;
    ajint idcnt=0;

    idsrtexp = ajRegCompC ("^([^ ]+) +([0-9]+) +([0-9]+) +([0-9]+)");

    embDbiSortFile (dbname, "list", "idsrt", nfiles, cleanup, sortopt);
    ajStrAssC(&lastidstr, " ");
    esortfile = embDbiFileIn (dbname, "idsrt");
    while (ajFileGets (esortfile, &rline))
    {
	ajRegExec (idsrtexp, rline);
	ajRegSubI (idsrtexp, 1, &idstr);
	if (ajStrMatchCase(idstr, lastidstr))
	{
	    ajWarn ("Duplicate ID skipped: '%S' "
		    "All hits will point to first ID found",
		    idstr);
	    continue;
	}
	ajRegSubI (idsrtexp, 2, &tmpstr);
	ajStrToInt (tmpstr, &rpos);
	ajRegSubI (idsrtexp, 3, &tmpstr);
	ajStrToInt (tmpstr, &spos);
	ajRegSubI (idsrtexp, 4, &tmpstr);
	ajStrToInt (tmpstr, &filenum);
	embDbiWriteEntryRecord (entFile, maxidlen, idstr,
			      rpos, spos, filenum);
	ajStrAss  (&lastidstr, idstr);
	idcnt++;
    }
    ajFileClose (&esortfile);

    ajRegFree(&idsrtexp);

    return idcnt;
}

/* @func embDbiMemWriteEntry **************************************************
**
** Write entryname index for in-memory processing
**
** @param [r] entFile [AjPFile] entryname index file
** @param [r] maxidlen [ajint] Maximum entry id length
** @param [r] idlist [AjPList] List of entry IDs to be written
** @param [w] ids [void***] AjPStr* array of IDs from list
** @return [ajint] Number of entries written (excluding duplicates)
******************************************************************************/

ajint embDbiMemWriteEntry (AjPFile entFile, ajint maxidlen,
			  AjPList idlist, void ***ids)
{
    ajint idCount;
    ajint i;
    static AjPStr idstr=NULL;
    EmbPEntry entry;
    ajint idcnt = 0;

    idCount = ajListToArray (idlist, ids);
    qsort (*ids, idCount, sizeof(void*), embDbiCmpId);
    ajDebug ("ids sorted\n");

    for (i = 0; i < idCount; i++)
    {
	entry = (EmbPEntry)(*ids)[i];
	if (ajStrMatchCaseC(idstr, entry->entry))
	{
	    ajErr ("Duplicate ID found: '%S'. ", idstr);
	    continue;
	}
	ajStrAssC(&idstr, entry->entry);
	embDbiWriteEntryRecord (entFile, maxidlen, idstr,
				entry->rpos, entry->spos, entry->filenum);
	idcnt++;
    }

    return idcnt;
}

/* @func embDbiSortWriteFields ************************************************
**
** Write the indices for a field.
**
** @param [r] dbname [AjPStr] Database name (used for temp file names)
** @param [r] release [AjPStr] Release number as a string
** @param [r] date [char[4]] Date
** @param [r] indexdir [AjPStr] Index directory
** @param [r] field [AjPStr] Field name (used for temp file names)
** @param [r] maxFieldLen [ajint] Maximum field token length
** @param [r] nfiles [ajint] Number of data files
** @param [r] nentries [ajint] Number of entries
** @param [r] cleanup [AjBool] Cleanup temp files if true
** @param [r] sortopt [AjPStr] Sort command line options
** @return [ajint] Number of field targets written
******************************************************************************/

ajint embDbiSortWriteFields (AjPStr dbname, AjPStr release,
			     char date[4], AjPStr indexdir,
			     AjPStr field, ajint maxFieldLen,
			     ajint nfiles, ajint nentries,
			     AjBool cleanup, AjPStr sortopt)
{
    AjPFile asortfile;
    AjPFile asrt2file;
    AjPFile blistfile;
    AjPFile elistfile;
    static AjPStr fieldSort=NULL;
    static AjPStr fieldSrt2=NULL;
    static AjPStr fieldId2=NULL;
    ajint ient;
    static AjPStr rline=NULL;
    static AjPStr eline=NULL;
    static AjPStr idstr=NULL;
    static AjPStr lastidstr=NULL;
    static AjPStr tmpstr=NULL;
    static AjPStr fieldStr=NULL;

    static AjPRegexp idsrtexp=NULL;
    static AjPRegexp toksrtexp=NULL;
    static AjPRegexp tokidsrtexp=NULL;

    ajint fieldCount=0;
    ajint idwidth;

    AjPFile trgFile;
    AjPFile hitFile;
    short alen;
    ajint asize;
    ajint ahsize;
    ajint itoken = 0;
    ajint i;
    ajint j;
    ajint k;
    ajint idnum;
    ajint lastidnum;
    static AjPStr currentid=NULL;

    ajFmtPrintS(&tmpstr, "%d", nentries);
    idwidth = ajStrLen(tmpstr);

    if (!idsrtexp)
      idsrtexp = ajRegCompC ("^([^ ]+) +");
    if (!toksrtexp)
      toksrtexp = ajRegCompC ("^([^ ]+) +([^\n]+)");
    if (!tokidsrtexp)
      tokidsrtexp = ajRegCompC ("^(.*[^ ]) +([0-9]+)\n$");

    ajFmtPrintS (&fieldId2, "%S_id2", field);
    ajFmtPrintS (&fieldSort, "%S_sort", field);
    ajFmtPrintS (&fieldSrt2, "%S_sort2", field);

    trgFile = embDbiFileIndex (indexdir, field, "trg");
    hitFile = embDbiFileIndex (indexdir, field, "hit");

    embDbiSortFile (dbname, ajStrStr(field),
		    ajStrStr(fieldSort),
		    nfiles, cleanup, sortopt);

    /* put in the entry numbers and remove the names */
    /* read dbname.<field>srt, for each entry, increment the count */

    elistfile = embDbiFileIn (dbname, "idsrt");
    asortfile = embDbiFileIn (dbname, ajStrStr(fieldSort));
    blistfile = embDbiFileOut (dbname, ajStrStr(fieldId2));

    fieldCount = 0;

    ient=0;
    ajStrAssC (&currentid, "");

    while (ajFileGets (asortfile, &rline))
    {
	ajRegExec (toksrtexp, rline);
	ajRegSubI (toksrtexp, 1, &idstr);
	ajRegSubI (toksrtexp, 2, &fieldStr);
	while (!ajStrMatch(idstr, currentid))
	{
	    ajStrAssS(&lastidstr, currentid);
	    if (!ajFileGets(elistfile, &eline))
	      ajFatal ("Error in embDbiSortWriteFields, "
		       "expected entry %S not found",
		       idstr);
	    ajRegExec (idsrtexp, eline);
	    ajRegSubI (idsrtexp, 1, &currentid);
	    if (!ajStrMatch(lastidstr, currentid))
	      ient++;
	}
	ajFmtPrintF (blistfile, "%S %0*d\n", fieldStr, idwidth, ient);
	fieldCount++;
    }

    ajFileClose (&asortfile);
    ajFileClose (&blistfile);
    ajFileClose (&elistfile);

    /* sort again */

    embDbiRmFile (dbname, ajStrStr(fieldSort), 0, cleanup);
    embDbiSortFile (dbname, ajStrStr(fieldId2),
		    ajStrStr(fieldSrt2),
		    0, cleanup, sortopt);

    alen = maxFieldLen+8;
    asize = 300 + (fieldCount*(ajint)alen); /* to be fixed later */
    embDbiHeader (trgFile, asize, fieldCount,
		  alen, dbname, release, date);

    ahsize = 300 + (fieldCount*4);
    embDbiHeader (hitFile, ahsize, fieldCount, 4,
		  dbname, release, date);

    itoken = 0;
    j = 0;
    k = 1;

    i=0;
    lastidnum=999999999;
    ajStrAssC(&lastidstr, "");
    asrt2file = embDbiFileIn (dbname, ajStrStr(fieldSrt2));
    while (ajFileGets (asrt2file, &rline))
    {
	ajRegExec (tokidsrtexp, rline);
	ajRegSubI (tokidsrtexp, 1, &idstr);
	ajRegSubI (tokidsrtexp, 2, &tmpstr);
	ajStrToInt (tmpstr, &idnum);
	if (!i)
	  ajStrAssS (&lastidstr, idstr);
	if (!ajStrMatch(lastidstr, idstr))
	{
	    embDbiWriteHit (hitFile, idnum);
	    embDbiWriteTrg (trgFile, maxFieldLen,
			    j, k, lastidstr);
	    j = 1;			/* number of hits */
	    k = i+1;		/* first hit */
	    ajStrAssS (&lastidstr, idstr);
	    i++;
	    itoken++;
	    lastidnum=idnum;
	}
	else if (idnum != lastidnum) /* idstr is the same */
	{
	    embDbiWriteHit (hitFile, idnum);
	    lastidnum = idnum;
	    j++;
	    i++;
	}
    }
    ajFileClose (&asrt2file);
    embDbiRmFile (dbname, ajStrStr(fieldSrt2), 0, cleanup);

    ajDebug ("targets i:%d itoken: %d\n", i, itoken);

    if (i) {		/* possibly there were no target tokens */
      embDbiWriteTrg (trgFile, maxFieldLen,
		      j, k, lastidstr);
      itoken++;
    }

    ajDebug ("wrote %F %d\n", trgFile, itoken);

    embDbiHeaderSize(trgFile, 300+itoken*(ajint)alen, itoken);

    ajDebug ("finished...\n%7d files\n%7d %F\n%7d %F\n",
	     nfiles, itoken, trgFile,
	     fieldCount, hitFile);

    ajFileClose (&trgFile);
    ajFileClose (&hitFile);
    return fieldCount;
}

/* @func embDbiMemWriteFields *************************************************
**
** Write the fields indices
**
** @param [r] dbname [AjPStr] Database name (used for temp file names)
** @param [r] release [AjPStr] Release number as a string
** @param [r] date [char[4]] Date
** @param [r] indexdir [AjPStr] Index directory
** @param [r] field [AjPStr] Field name (used for file names)
** @param [r] maxFieldLen [ajint] Maximum field token length
** @param [r] fieldList [AjPList] List of field tokens to be written
** @param [r] ids [void**] AjPStr* array offield token s from list
** @return [ajint] Number of field targets written
******************************************************************************/

ajint embDbiMemWriteFields (AjPStr dbname, AjPStr release,
			    char date[4], AjPStr indexdir,
			    AjPStr field, ajint maxFieldLen,
			    AjPList fieldList, void** ids)
{
    ajint fieldCount=0;
    ajint ient;
    ajint fieldent;
    ajint i;
    ajint j;
    ajint k;
    void **fieldItems = NULL;
    AjPFile trgFile;
    AjPFile hitFile;
    short alen;
    ajint asize;
    ajint ahsize;
    ajint itoken = 0;
    ajint idup=0;
    EmbPField fieldData=NULL;
    static char* lastfd=NULL;
    static AjPStr fieldStr=NULL;
    ajint lastidnum=0;

    trgFile = embDbiFileIndex (indexdir, field, "trg");
    hitFile = embDbiFileIndex (indexdir, field, "hit");

    fieldCount = ajListToArray (fieldList,
				&fieldItems);

    ajDebug ("fieldItems: %d %x\n",
	     fieldCount, fieldItems);

    if (fieldCount)
    {
	qsort (fieldItems, fieldCount, sizeof(void*),
	       embDbiCmpFieldId);
	ajDebug ("%S sorted by id\n", field);
	ient=0;
	fieldent=0;

	while (ids[ient] && fieldItems[fieldent])
	{
	    k = strcmp(((EmbPEntry)ids[ient])->entry,
		       ((EmbPField)fieldItems[fieldent])->entry);
	    if (k < 0)
	      ient++;
	    else if (k > 0)
	      fieldent++;
	    else
	      ((EmbPField)fieldItems[fieldent++])->nid = ient+1;
	}
	ajDebug ("checked ids: %d fieldItems: %d %d\n",
		 ient, fieldent, fieldCount);

	qsort (fieldItems, fieldCount, sizeof(void*),
	       embDbiCmpFieldField);
	ajDebug ("%S sorted by %S\n", field, field);
    }

    alen = maxFieldLen+8;
    asize = 300 + (fieldCount*(ajint)alen); /* to be fixed later */
    embDbiHeader (trgFile, asize, fieldCount,
		  alen, dbname, release, date);

    ahsize = 300 + (fieldCount*4);
    embDbiHeader (hitFile, ahsize, fieldCount, 4,
		  dbname, release, date);

    itoken = 0;
    j = 0;
    k = 1;

    idup=0;
    for (i = 0; i < fieldCount; i++)
    {
	fieldData = (EmbPField)fieldItems[i];

	if (!i)
	{
	    lastfd = fieldData->field;
	    lastidnum = 999999999;
	}
	if (strcmp(lastfd, fieldData->field))
	{
	    embDbiWriteHit (hitFile, fieldData->nid);
	    ajStrAssC(&fieldStr, lastfd);
	    embDbiWriteTrg (trgFile, maxFieldLen,
			    j, k, fieldStr);
	    j = 1;
	    k = i+1-idup;
	    itoken++;
	    lastfd = fieldData->field;
	    lastidnum=fieldData->nid;
	}
	else if (fieldData->nid != lastidnum) /* lastfd is the same */
	{
	    embDbiWriteHit (hitFile, fieldData->nid);
	    lastidnum = fieldData->nid;
	    j++;
	}
	else
	{
	    idup++;
	}
    }
    ajStrAssC(&fieldStr, lastfd);
    if (fieldCount)
    {
	embDbiWriteTrg (trgFile, maxFieldLen, j, k, fieldStr);
	itoken++;
    }

    ajDebug ("wrote %F %d\n", trgFile, itoken);

    embDbiHeaderSize(trgFile, 300+itoken*(ajint)alen, itoken);

    ajDebug ("finished...\n%%7d %F\n%7d %F\n",
	     itoken, trgFile,
	     fieldCount, hitFile);

    ajFileClose (&trgFile);
    ajFileClose (&hitFile);

    return fieldCount;
}

/* @func embDbiDateSet ********************************************************
**
** Sets the date as an integer array from a formatted string.
** The integer array is the internal format in database index headers
**
** @param [r] datestr [AjPStr] Date as a string
** @param [w] date [char[4]] Data char (1 byte int) array
** @return [void]
******************************************************************************/

void embDbiDateSet (AjPStr datestr, char date[4])
{
    ajint i;
    ajint j;
    AjPRegexp datexp;
    static AjPStr tmpstr=NULL;

    datexp = ajRegCompC("^([0-9]+).([0-9]+).([0-9]+)");

    date[3]=0;

    if (ajRegExec (datexp, datestr))
      for (i=1; i<4; i++)
      {
	  ajRegSubI (datexp, i, &tmpstr);
	  ajStrToInt (tmpstr, &j);
	  date[3-i] = j;
      }

    ajRegFree(&datexp);

    return;
}

/* @func embDbiMaxlen *********************************************************
**
** Compares a string to a maximum string length.
**
** A negative maximum length limits the string to that absolute length.
**
** A non-negative length is updated if the string is longer
**
** @param [r] token [AjPStr*] Token string
** @param [u] maxlen [ajint*] Maximum string length
** @return [void]
******************************************************************************/

void embDbiMaxlen (AjPStr* token, ajint* maxlen)
{
    if (*maxlen < 0)
      ajStrSub (token, 1, -(*maxlen));
    else
    {
	if (ajStrLen(*token) > *maxlen)
	  *maxlen = ajStrLen(*token);
    }
    return;
}

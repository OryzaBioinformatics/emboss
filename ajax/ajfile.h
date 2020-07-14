#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfile_h
#define ajfile_h

#include "ajlist.h"
#include <sys/stat.h>

/* @data AjPFile **************************************************************
**
** Ajax file object. Holds information for an open (unbuffered)
** input or output file.
**
** On output, conversion code "%F" writes the filename.
**
** @new ajFileNew Default constructor for an input file
** @new ajFileNewF Constructor using an existing open file,
**                 for example stdin or stdout.
** @new ajFileNewIn Constructor using a filename for an input file
** @new ajFileNewApp Default constructor using a filename for an output file
**                   to be opened for appending records to the end of the file
** @new ajFileNewOut Default constructor using a filename for an output file
** @new ajFileNewInPipe Constructor using output from a forked command
** @new ajFileNewInList Constructor using an AjPList of filenames
** @delete ajFileClose Default destructor
** @mod ajFileGets Reads a record from a file
** @mod ajFileGetsTrim Reads a record from a file and removes
**                     newline characters
** @cast ajFileName Returns the filename as char*
** @cast ajFileFp Returns the equivalent C file pointer
** @other AjPFileBuff Buffered input file.
** @@
******************************************************************************/

typedef struct AjSFile {
  FILE *fp;			/* C file pointer */
  ajint Handle;			/* AJAX file number 0 if unused */
  AjPStr Name;			/* File name */
  AjPList List;			/* List of file names (first is open) */
  AjBool End;			/* True if EOF has been reached */
  AjPStr Buff;			/* Buffer for latest line read */
} AjOFile;

#define AjPFile AjOFile*

/* @data AjPFileBuffList ******************************************************
**
** Ajax file buffer, holding a simple linked list of buffered lines.
** This does not use the AjPList objects.
**
** This is a substructure of the AjPFileBuff object.
** @@
******************************************************************************/

typedef struct AjSFileBuffList {
  AjPStr Line;			/* String : this line*/
  struct AjSFileBuffList* Next;	/* Next line in the list, NULL for last */
  ajlong Fpos;			/* File offset for start of this line */
} AjOFileBuffList;

#define AjPFileBuffList AjOFileBuffList*

/* @data AjPFileBuff **********************************************************
**
** Ajax buffered file object. Holds information for a buffered input file.
**
** @new ajFileBuffNew Default constructor for an input file
** @new ajFileBuffNewF Constructor using an existing open file,
**                 for example stdin or stdout.
** @new ajFileBuffNewS Constructor using a filename
** @new ajFileBuffNewDW Constructor using a directory and wildcard filename
** @new ajFileBuffNewInList Constructor using a list of filenames
** @delete ajFileBuffDel Default destructor
** @mod ajFileBuffIsbuff Turns on input buffering.
** @mod ajFileBuffNobuff Turns off input buffering.
** @mod ajFileBuffReset Resets so the next read uses the first buffered line
** @mod ajFileBuffClear Deletes unwanted old lines from the buffer
**                      but can keep the most recent line(s) for reuse.
** @mod ajFileBuffStripHtml Processes data in the file buffer,
**                          removing HTML tokens
** @mod ajFileBuffLoad Reads all input lines from a file into the buffer.
** @mod ajFileBuffGet Reads a line from a buffered file.
** @mod ajFileBuffGetStore Reads a line from a buffered file with append.
** @mod ajFileBuffGetStoreL Reads a line from a buffered file with append.
** @use ajFileBuffEmpty Tests whether a file buffer is empty.
** @use ajFileBuffTrace Writes debug messages to indicate the contents
**                      of a buffered file.
** @cast ajFileBuffFile Returns the equivalent AjPFile without the
**                      buffer access.
** @use ajFileBuffTraceFull Writes debug messages to show the full contents
**                      of a buffered file.
** @cast ajFileBuffFp Returns the equivalent C file pointer
** @other AjPFile Simple input file.
** @@
******************************************************************************/

typedef struct AjSFileBuff {
  AjPFile File;			/* The input file - data to be buffered */
  AjPFileBuffList Lines;	/* All lines ... where the data really is */
  AjPFileBuffList Curr;		/* Current line in Lines list */
  AjPFileBuffList Prev;		/* Previous line (points to Curr for delete) */
  AjPFileBuffList Last;		/* Last line for quick appending */
  AjPFileBuffList Free;		/* Free list of lines for reuse */
  AjPFileBuffList Freelast;	/* Last free line for quick append*/
  AjBool Nobuff;		/* if true, do not buffer the file */
  ajint Pos;			/* Position in list */
  ajint Size;			/* Size of list */
  ajint FreeSize;		/* Size of free list */
  ajlong Fpos;			/* File pointer in File */
} AjOFileBuff;

#define AjPFileBuff AjOFileBuff*

/* ============= prototypes =========================*/

void        ajFileBuffClear (AjPFileBuff thys, ajint lines);
void        ajFileBuffClearStore (AjPFileBuff thys, ajint lines,
				  AjPStr rdline, AjBool store, AjPStr *astr);
void        ajFileBuffDel (AjPFileBuff* pthis);
AjBool      ajFileBuffEmpty (const AjPFileBuff thys);
AjBool      ajFileBuffEnd (const AjPFileBuff thys);
AjBool      ajFileBuffEof (const AjPFileBuff thys);
AjPFile     ajFileBuffFile (const AjPFileBuff thys);
void        ajFileBuffFix (AjPFileBuff thys);
FILE*       ajFileBuffFp (const AjPFileBuff thys);
void        ajFileBuffFreeClear (AjPFileBuff thys);
AjBool      ajFileBuffGet  (AjPFileBuff thys, AjPStr *pdest);
AjBool      ajFileBuffGetL (AjPFileBuff thys, AjPStr *pdest,
			    ajlong* fpos);
AjBool      ajFileBuffGetStore (AjPFileBuff thys, AjPStr* pdest,
				AjBool store, AjPStr *astr);
AjBool      ajFileBuffGetStoreL (AjPFileBuff thys, AjPStr* pdest,
				 ajlong* fpos, AjBool store, AjPStr *astr);
AjBool      ajFileBuffIsbuff (AjPFileBuff thys);
void        ajFileBuffLoad (AjPFileBuff thys);
void        ajFileBuffLoadC (AjPFileBuff thys, const char* str);
void        ajFileBuffLoadS (AjPFileBuff thys, const AjPStr str);
AjPFileBuff ajFileBuffNew (void);
AjPFileBuff ajFileBuffNewDC (const AjPStr dir, const char* filename);
AjPFileBuff ajFileBuffNewDF (const AjPStr dir, const AjPStr filename);
AjPFileBuff ajFileBuffNewDW (const AjPStr dir, const AjPStr wildfile);
AjPFileBuff ajFileBuffNewF (FILE *fp);
AjPFileBuff ajFileBuffNewFile (AjPFile file);
AjPFileBuff ajFileBuffNewIn (const AjPStr name);
AjPFileBuff ajFileBuffNewInList (AjPList list);
AjPFileBuff ajFileBuffNewS (const AjPStr data);
AjBool      ajFileBuffNobuff (AjPFileBuff thys);
void        ajFileBuffReset (AjPFileBuff thys);
void        ajFileBuffResetPos (AjPFileBuff thys);
void        ajFileBuffResetStore (AjPFileBuff thys,
				  AjBool store, AjPStr *astr);
AjBool      ajFileBuffSetFile (AjPFileBuff* pthys, AjPFile file);
ajint       ajFileBuffSize (void);
void        ajFileBuffStripHtml (AjPFileBuff thys);
ajint       ajFileBuffStripSrs (AjPFileBuff thys);
void        ajFileBuffTrace (const AjPFileBuff thys);
void        ajFileBuffTraceFull (const AjPFileBuff thys, size_t nlines,
				 size_t nfree);
void        ajFileClose (AjPFile *pthis);
void        ajFileDataDirNew  (const AjPStr tfile, const AjPStr dir,
			       AjPFile *fnew);
void        ajFileDataDirNewC (const char *s, const char* d,
			       AjPFile *f);
void        ajFileDataNew  (const AjPStr filename, AjPFile *newfileptr);
void        ajFileDataNewC (const char *s, AjPFile *f);
void        ajFileDataNewWrite (const AjPStr tfile, AjPFile *fnew);
AjBool      ajFileDir (AjPStr* dir);
void        ajFileDirFix (AjPStr* dir);
AjBool      ajFileDirUp (AjPStr* dir);
AjBool      ajFileDirPath (AjPStr* dir);
AjBool      ajFileDirTrim (AjPStr* name);
AjBool      ajFileEof (const AjPFile thys);
void        ajFileExit (void);
AjPList     ajFileFileList(AjPStr files);
FILE*       ajFileFp (const AjPFile thys);
AjBool      ajFileGetwd (AjPStr* dir);
AjPStr      ajFileGetName (const AjPFile thys);
AjBool      ajFileGets (AjPFile thys, AjPStr *pdest);
AjBool      ajFileGetsL (AjPFile thys, AjPStr *pdest, ajlong* fpos);
AjBool      ajFileGetsTrim  (AjPFile thys, AjPStr *pdest);
AjBool      ajFileGetsTrimL (AjPFile thys, AjPStr *pdest, ajlong* fpos);
AjBool      ajFileHasDir (const AjPStr name);
ajlong      ajFileLength (AjPStr fname);
const char* ajFileName (const AjPFile thys);
AjBool      ajFileNameDirSet  (AjPStr* filename, const AjPStr dir);
AjBool      ajFileNameDirSetC (AjPStr* filename, const char* dir);
AjBool      ajFileNameExt  (AjPStr* filename, const AjPStr extension);
AjBool      ajFileNameExtC (AjPStr* filename, const char* extension);
AjBool      ajFileNameShorten(AjPStr *fname);
AjBool      ajFileNameTrim(AjPStr *fname);
AjPFile     ajFileNew (void);
AjPFile     ajFileNewApp (const AjPStr name);
AjPFile     ajFileNewDC (const AjPStr dir, const char* filename);
AjPFile     ajFileNewDF (const AjPStr dir, const AjPStr filename);
AjPFile     ajFileNewDW (const AjPStr dir, const AjPStr wildfile);
AjPFile     ajFileNewF (FILE* file);
AjPFile     ajFileNewIn (const AjPStr name);
AjPFile     ajFileNewInC (const char *name);
AjPFile     ajFileNewInPipe (const AjPStr name);
AjPFile     ajFileNewInList (AjPList list);
AjPFile     ajFileNewOut (const AjPStr name);
AjPFile     ajFileNewOutC (const char *name);
AjPFile     ajFileNewOutD (const AjPStr dir, const AjPStr name);
void        ajFileOutHeader (const AjPFile thys);
void        ajFileOutClose (AjPFile *pthis);
AjBool      ajFileNext (AjPFile thys);
size_t      ajFileRead (void* ptr, size_t element_size, size_t count,
			const AjPFile thys);
AjBool      ajFileReadAppend (AjPFile thys, AjPStr* pbuff);
AjBool      ajFileReadLine (AjPFile thys, AjPStr *pdest);
ajuint      ajFileReadUint (const AjPFile thys, AjBool Bigendian);
FILE*       ajFileReopen (AjPFile thys, AjPStr name);
ajint 	    ajFileScan (AjPStr path, AjPStr filename, AjPList *result,
			AjBool show, AjBool dolist, AjPList *list,
			AjPList rlist, AjBool recurs, const AjPFile outf);
ajint       ajFileSeek (AjPFile thys, ajlong offset, ajint wherefrom);
AjBool      ajFileSetDir (AjPStr *pname, const AjPStr dir);
AjBool      ajFileStat (AjPStr *filename, ajint mode);
AjBool      ajFileStderr (const AjPFile file);
AjBool      ajFileStdin (const AjPFile file);
AjBool      ajFileStdout (const AjPFile file);
ajlong      ajFileTell (const AjPFile thys);
char*       ajFileTempName (const char *dir);
AjBool      ajFileTestSkip (AjPStr fullname, AjPStr exc, AjPStr inc,
			    AjBool keep, AjBool ignoredirectory);
void        ajFileTrace (const AjPFile thys);
void        ajFileUnbuffer (const AjPFile thys);
size_t      ajFileWrite (const AjPFile thys, const void* ptr,
			 size_t element_size, size_t count);

ajint       ajFileWriteByte (const AjPFile thys, char ch);
ajint       ajFileWriteChar (const AjPFile thys, char* str, ajint len);
ajint       ajFileWriteInt2 (const AjPFile thys, short i);
ajint       ajFileWriteInt4 (const AjPFile thys, ajint i);
ajint       ajFileWriteInt8 (const AjPFile thys, ajlong i);
ajint       ajFileWriteStr  (const AjPFile thys, AjPStr str, ajint len);

/* ============= definitions =========================*/
#define AJ_FILE_R S_IRUSR
#define AJ_FILE_W S_IWUSR
#define AJ_FILE_X S_IXUSR

#endif

#ifdef __cplusplus
}
#endif

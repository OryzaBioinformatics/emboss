#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajfile_h
#define ajfile_h

#include "ajlist.h"
#include <sys/stat.h>

/* @data AjPFile *******************************************************
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
  FILE *fp;
  int Handle;
  AjPStr Name;
  AjPList List;
  AjBool End;
  AjPStr Buff;
} AjOFile, *AjPFile;

/* @data AjPFileBuffList ***************************************************
**
** Ajax file buffer, holding a simple linked list of buffered lines.
** This does not use the AjPList objects.
**
** This is a substructure of the AjPFileBuff object.
** @@
******************************************************************************/

typedef struct AjSFileBuffList {
  AjPStr Line;
  struct AjSFileBuffList* Next;
  long Fpos;
} AjOFileBuffList, *AjPFileBuffList;

/* @data AjPFileBuff *******************************************************
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
** @mod ajFileBuffNobuff Turns off input buffering.
** @mod ajFileBuffReset Resets so the next read uses the first buffered line
** @mod ajFileBuffClear Deletes unwanted old lines from the buffer
**                      but can keep the most recent line(s) for reuse.
** @mod ajFileBuffStripHtml Processes data in the file buffer,
**                          removing HTML tokens
** @mod ajFileBuffLoad Reads all input lines from a file into the buffer.
** @mod ajFileBuffGet Reads a line from a buffered file.
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
  AjPFile File;
  AjPFileBuffList Lines;
  AjPFileBuffList Curr;
  AjPFileBuffList Last;
  AjPFileBuffList Free;
  AjPFileBuffList Freelast;
  AjBool Nobuff;
  int Pos;
  int Size;
  long Fpos;
} AjOFileBuff, *AjPFileBuff;

/* ============= prototypes =========================*/

void        ajFileBuffClear (const AjPFileBuff thys, int lines);
void        ajFileBuffDel (AjPFileBuff* pthis);
AjBool      ajFileBuffEmpty (const AjPFileBuff thys);
AjPFile     ajFileBuffFile (const AjPFileBuff thys);
FILE*       ajFileBuffFp (const AjPFileBuff thys);
void        ajFileBuffFreeClear (const AjPFileBuff thys);
AjBool      ajFileBuffGet  (const AjPFileBuff thys, AjPStr *pdest);
AjBool      ajFileBuffGetL (const AjPFileBuff thys, AjPStr *pdest, long* fpos);
void        ajFileBuffLoad (const AjPFileBuff thys);
void        ajFileBuffLoadC (const AjPFileBuff thys, const char* str);
void        ajFileBuffLoadS (const AjPFileBuff thys, const AjPStr str);
AjPFileBuff ajFileBuffNew (void);
AjPFileBuff ajFileBuffNewDC (const AjPStr dir, const char* filename);
AjPFileBuff ajFileBuffNewDF (const AjPStr dir, const AjPStr filename);
AjPFileBuff ajFileBuffNewDW (const AjPStr dir, const AjPStr wildfile);
AjPFileBuff ajFileBuffNewF (FILE *fp);
AjPFileBuff ajFileBuffNewFile (AjPFile file);
AjPFileBuff ajFileBuffNewIn (const AjPStr name);
AjPFileBuff ajFileBuffNewInList (const AjPList list);
AjPFileBuff ajFileBuffNewS (const AjPStr data);
void        ajFileBuffNobuff (const AjPFileBuff thys);
void        ajFileBuffReset (const AjPFileBuff thys);
AjBool      ajFileBuffSetFile (AjPFileBuff* pthys, AjPFile file);
int         ajFileBuffSize (void);
void        ajFileBuffStripHtml (const AjPFileBuff thys);
void        ajFileBuffTrace (const AjPFileBuff thys);
void        ajFileBuffTraceFull (const AjPFileBuff thys, size_t nlines,
				 size_t nfree);
void        ajFileClose (AjPFile *pthis);
void        ajFileDataNew (const AjPStr filename, AjPFile *newfileptr);
void        ajFileDataNewC(const char *s, AjPFile *f);
void        ajFileDataNewWrite(const AjPStr tfile, AjPFile *fnew);
AjBool      ajFileDir (AjPStr* dir);
void        ajFileDirFix (AjPStr* dir);
AjBool      ajFileDirUp (AjPStr* dir);
AjBool      ajFileDirPath (AjPStr* dir);
void        ajFileExit (void);
FILE*       ajFileFp (const AjPFile thys);
AjBool      ajFileGetwd (AjPStr* dir);
AjPStr      ajFileGetName (const AjPFile thys);
AjBool      ajFileGets (const AjPFile thys, AjPStr *pdest);
AjBool      ajFileGetsL (const AjPFile thys, AjPStr *pdest, long* fpos);
AjBool      ajFileGetsTrim  (const AjPFile thys, AjPStr *pdest);
AjBool      ajFileGetsTrimL (const AjPFile thys, AjPStr *pdest, long* fpos);
const char* ajFileName (const AjPFile thys);
AjBool      ajFileNameDirSetC (AjPStr* filename, const char* dir);
AjBool      ajFileNameDirSet (AjPStr* filename, const AjPStr dir);
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
AjPFile     ajFileNewInPipe (const AjPStr name);
AjPFile     ajFileNewInList (const AjPList list);
AjPFile     ajFileNewOut (const AjPStr name);
AjPFile     ajFileNewOutD (const AjPStr dir, const AjPStr name);
AjBool      ajFileNext (const AjPFile thys);
size_t      ajFileRead (void* ptr, size_t element_size, size_t count,
			const AjPFile thys);
FILE*       ajFileReopen (const AjPFile thys, AjPStr name);
AjBool      ajFileReadLine (const AjPFile thys, AjPStr *pdest);
unsigned int ajFileReadUint (const AjPFile thys, AjBool Bigendian);
void 	    ajFileScan(AjPStr path, AjPStr filename, AjPList *result,
		       AjBool show, AjBool dolist, AjPList *list,
		       AjPList rlist, AjBool recurs, const AjPFile outf);
int         ajFileSeek (const AjPFile thys, long offset, int wherefrom);
AjBool      ajFileStat (AjPStr *filename, int mode);
AjBool      ajFileStderr (const AjPFile file);
AjBool      ajFileStdin (const AjPFile file);
AjBool      ajFileStdout (const AjPFile file);
long        ajFileTell (const AjPFile thys);
AjBool      ajFileTestSkip (AjPStr fullname, AjPStr exc, AjPStr inc,
			    AjBool keep);
void        ajFileTrace (const AjPFile thys);
void        ajFileUnbuffer (const AjPFile thys);
size_t      ajFileWrite (const void* ptr, size_t element_size, size_t count,
			 const AjPFile thys);

void        ajFileOutHeader (const AjPFile thys);
void        ajFileOutClose (AjPFile *pthis);


/* ============= definitions =========================*/
#define AJ_FILE_R S_IRUSR
#define AJ_FILE_W S_IWUSR
#define AJ_FILE_X S_IXUSR

#endif

#ifdef __cplusplus
}
#endif

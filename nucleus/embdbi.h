#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embdbi_h
#define embdbi_h

/* @data EmbPField ************************************************************
**
** NUCLEUS internal structure for database indexing applications
** to store field tokens with links to the entry index number.
**
******************************************************************************/

typedef struct EmbSField {
  char* field;			/* field token */
  char* entry;			/* entry name */
  ajint nid;			/* entry number */
} EmbOField, *EmbPField;

/* @data EmbPEntry ************************************************************
**
** NUCLEUS internal structure for database indexing applications
** to store an entry id with a list of field tokens and file
** positions for writing to the index files.
**
******************************************************************************/

typedef struct EmbSEntry {
  char* entry;			/* entry name */
  ajint filenum;		/* record in division file */
  ajint rpos;			/* entry offset in data file */
  ajint spos;			/* entry offset in sequence file */
  ajint* nfield;		/* number of tokens for each field */
  char*** field;		/* array of tokens for each field */
} EmbOEntry, *EmbPEntry;

ajint     embDbiCmpId (const void* a, const void* b);
ajint     embDbiCmpFieldId (const void* a, const void* b);
ajint     embDbiCmpFieldField (const void* a, const void* b);
void      embDbiDateSet (AjPStr datestr, char date[4]);
EmbPEntry embDbiEntryNew (ajint nfields);
EmbPField embDbiFieldNew (void);
AjPFile   embDbiFileIn (AjPStr dbname, char* extension);
AjPFile   embDbiFileIndex (AjPStr indexdir, AjPStr field, char* extension);
AjPList   embDbiFileList (AjPStr dir, AjPStr wildfile, AjBool trim);
AjPList   embDbiFileListExc (AjPStr dir, AjPStr wildfile, AjPStr exclude);
AjPFile   embDbiFileOut (AjPStr dbname, char* extension);
AjPFile   embDbiFileSingle (AjPStr dbname, char* extension, ajint num);
AjBool    embDbiFlatOpenlib(AjPStr lname, AjPFile* libr);
void      embDbiHeader (AjPFile file, ajint filesize,
			ajint recordcnt, short recordlen,
			AjPStr dbname, AjPStr release,
			char date[4]);
void      embDbiHeaderSize (AjPFile file, ajint filesize, ajint recordcnt);
void      embDbiMaxlen (AjPStr* token, ajint* maxlen);
void      embDbiMemEntry (AjPList idlist, AjPList* fieldList, ajint nfields,
			  EmbPEntry entry, ajint ifile);
ajint     embDbiMemWriteEntry (AjPFile entFile, ajint maxidlen,
			       AjPList idlist, void ***ids);
ajint     embDbiMemWriteFields (AjPStr dbname, AjPStr release,
				char date[4], AjPStr indexdir,
				AjPStr field, ajint maxFieldLen,
				AjPList fieldList,
				void** ids);
void      embDbiRmEntryFile (AjPStr dbname,  AjBool cleanup);
void      embDbiRmFile (AjPStr dbname, const char* ext, ajint nfiles,
			AjBool cleanup);
void      embDbiRmFileI (AjPStr dbname, const char* ext, ajint ifile,
			 AjBool cleanup);
void      embDbiSortClose (AjPFile* elistfile, AjPFile* alistfile,
			   ajint nfields);
void      embDbiSortFile (AjPStr dbname, const char* ext1, const char* ext2,
			  ajint nfiles, AjBool cleanup, AjPStr sortopt);
AjPFile   embDbiSortOpen (AjPFile* alistfile, ajint ifile,
			  AjPStr dbname, AjPStr* fields, ajint nfields);
ajint     embDbiSortWriteEntry (AjPFile entFile, ajint maxidlen,
				AjPStr dbname, ajint nfiles,
				AjBool cleanup, AjPStr sortopt);
ajint     embDbiSortWriteFields (AjPStr dbname, AjPStr release,
				 char date[4], AjPStr indexdir,
				 AjPStr field,  ajint maxFieldLen,
				 ajint nfiles, ajint nentries,
				 AjBool cleanup, AjPStr sortopt);
void      embDbiSysCmd (AjPStr cmdstr);
void      embDbiWriteDivision (AjPStr indexdir,
			       AjPStr dbname, AjPStr release, char date[4],
			       ajint maxfilelen, ajint nfiles,
			       AjPStr* divfiles, AjPStr* seqfiles);
void      embDbiWriteDivisionRecord (AjPFile file,
				     ajint maxnamlen, short recnum,
				     AjPStr datfile, AjPStr seqfile);
void      embDbiWriteEntryRecord (AjPFile file, ajint maxidlen, AjPStr id,
				  ajint rpos, ajint spos, short filenum);
void      embDbiWriteHit (AjPFile file, ajint idnum);
void      embDbiWriteTrg (AjPFile file, ajint maxfieldlen,
			  ajint idnum, ajint idcnt, AjPStr hitstr);

#endif

#ifdef __cplusplus
}
#endif

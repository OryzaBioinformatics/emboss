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
** @attr field [char*] field token
** @attr entry [char*] entry name
** @attr nid [ajint] entry number
** @@
******************************************************************************/

typedef struct EmbSField {
  char* field;
  char* entry;
  ajint nid;
} EmbOField;
#define EmbPField EmbOField*

/* @data EmbPEntry ************************************************************
**
** NUCLEUS internal structure for database indexing applications
** to store an entry id with a list of field tokens and file
** positions for writing to the index files.
**
** @attr entry [char*] entry name
** @attr filenum [ajint] record in division file
** @attr rpos [ajint] entry offset in data file
** @attr spos [ajint] entry offset in sequence file
** @attr nfield [ajint*] number of tokens for each field
** @attr field [char***] array of tokens for each field
** @@
******************************************************************************/

typedef struct EmbSEntry {
  char* entry;
  ajint filenum;
  ajint rpos;
  ajint spos;
  ajint* nfield;
  char*** field;
} EmbOEntry;
#define EmbPEntry EmbOEntry*

ajint     embDbiCmpId (const void* a, const void* b);
ajint     embDbiCmpFieldId (const void* a, const void* b);
ajint     embDbiCmpFieldField (const void* a, const void* b);
void      embDbiDateSet (const AjPStr datestr, char date[4]);
EmbPEntry embDbiEntryNew (ajint nfields);
EmbPField embDbiFieldNew (void);
AjPFile   embDbiFileIn (const AjPStr dbname, const char* extension);
AjPFile   embDbiFileIndex (const AjPStr indexdir, const AjPStr field,
			   const char* extension);
AjPList   embDbiFileList (const AjPStr dir, const AjPStr wildfile,
			  AjBool trim);
AjPList   embDbiFileListExc (const AjPStr dir, const AjPStr wildfile,
			     const AjPStr exclude);
AjPFile   embDbiFileOut (const AjPStr dbname, const char* extension);
AjPFile   embDbiFileSingle (const AjPStr dbname, const char* extension,
			    ajint num);
AjBool    embDbiFlatOpenlib(const AjPStr lname, AjPFile* libr);
void      embDbiHeader (AjPFile file, ajint filesize,
			ajint recordcnt, short recordlen,
			const AjPStr dbname, const AjPStr release,
			const char date[4]);
void      embDbiHeaderSize (AjPFile file, ajint filesize, ajint recordcnt);
void      embDbiMaxlen (AjPStr* token, ajint* maxlen);
void      embDbiMemEntry (AjPList idlist,
			  AjPList* fieldList, ajint nfields,
			  EmbPEntry entry, ajint ifile);
ajint     embDbiMemWriteEntry (AjPFile entFile, ajint maxidlen,
			       const AjPList idlist, void ***ids);
ajint     embDbiMemWriteFields (const AjPStr dbname, const AjPStr release,
				const char date[4], const AjPStr indexdir,
				const AjPStr field, ajint maxFieldLen,
				const AjPList fieldList,
				void** ids);
void      embDbiRmEntryFile (const AjPStr dbname,  AjBool cleanup);
void      embDbiRmFile (const AjPStr dbname, const char* ext, ajint nfiles,
			AjBool cleanup);
void      embDbiRmFileI (const AjPStr dbname, const char* ext, ajint ifile,
			 AjBool cleanup);
void      embDbiSortClose (AjPFile* elistfile, AjPFile* alistfile,
			   ajint nfields);
void      embDbiSortFile (const AjPStr dbname,
			  const char* ext1, const char* ext2,
			  ajint nfiles, AjBool cleanup, const AjPStr sortopt);
AjPFile   embDbiSortOpen (AjPFile* alistfile, ajint ifile,
			  const AjPStr dbname, AjPStr const * fields,
			  ajint nfields);
ajint     embDbiSortWriteEntry (AjPFile entFile, ajint maxidlen,
				const AjPStr dbname, ajint nfiles,
				AjBool cleanup, const AjPStr sortopt);
ajint     embDbiSortWriteFields (const AjPStr dbname, const AjPStr release,
				 const char date[4], const AjPStr indexdir,
				 const AjPStr field,  ajint maxFieldLen,
				 ajint nfiles, ajint nentries,
				 AjBool cleanup, const AjPStr sortopt);
void      embDbiSysCmd (const AjPStr cmdstr);
void      embDbiWriteDivision (const AjPStr indexdir,
			       const AjPStr dbname, const AjPStr release,
			       const char date[4],
			       ajint maxfilelen, ajint nfiles,
			       AjPStr const * divfiles,
			       AjPStr const * seqfiles);
void      embDbiWriteDivisionRecord (AjPFile file,
				     ajint maxnamlen, short recnum,
				     const AjPStr datfile,
				     const AjPStr seqfile);
void      embDbiWriteEntryRecord (AjPFile file, ajint maxidlen,
				  const AjPStr id,
				  ajint rpos, ajint spos, short filenum);
void      embDbiWriteHit (AjPFile file, ajint idnum);
void      embDbiWriteTrg (AjPFile file, ajint maxfieldlen,
			  ajint idnum, ajint idcnt, const AjPStr hitstr);

#endif

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqreport_h
#define ajseqreport_h

/* @data AjPReport ************************************************************
**
** Ajax Report Output object.
**
** Holds definition of feature report output.
**
** @new ajReportNew Default constructor
** @delete ajReportDel Default destructor
** @set ajReportClear Resets ready for reuse.
** @use ajReportWrite Master sequence output routine
** @use ajReportNewOut Opens an output file for sequence writing.
** @other AjPSeqout Sequence output
** @other AjPFile Input and output files
** @@
******************************************************************************/

typedef struct AjSReport {
  AjPStr Name;			/* As "Source" for features, usually empty */
  AjPStr Type;			/* "P" Protein or "N" Nucleotide */
  AjPStr Formatstr;		/* Report format (-rformat) */
  AjEnum Format;		/* Report format (index number) */
  AjPFeattable Fttable;		/* Feature table to use (obsolete?) */
  AjPFeattabOut Ftquery;	/* Output definition for features*/
  AjPStr Extension;		/* Output file extension */
  AjPFile File;			/* Output file object */
  AjPList Tagnames;		/* List of extra tag names (from ACD) */
  AjPList Tagprints;		/* List of extra tag printnames (from ACD) */
  AjPList Tagtypes;		/* List of extra tag datatypes (from ACD) */
  AjPStr Header;		/* Text to add to header with newlines */
  AjPStr Tail;			/* Text to add to tail with newlines */
  AjPList FileNames;		/* Names of extra files (see FileTypes) */
  AjPList FileTypes;		/* Types of extra files (see FileNames) */
  ajint Precision;		/* Floating precision for score */
  AjBool Showacc;		/* Report accession number */
  AjBool Showdes;		/* Report sequence description */
  AjBool Showusa;		/* Report USA (-rusa) or only seqname */
  AjBool Showscore;		/* Report score (if optional for format) */
  AjBool Multi;			/* if true, assume >1 sequence */
  ajint Mintags;		/* Minimum number of tags to report */
  ajint Count;			/* Number of sequences reported so far */
} AjOReport;

#define AjPReport AjOReport*

void         ajReportClose (AjPReport pthys);
void         ajReportDel (AjPReport* pthys);
AjBool       ajReportOpen (AjPReport thys, const AjPStr name);
void         ajReportFileAdd (AjPReport thys,
			      const AjPFile file, const AjPStr type);
AjBool       ajReportFindFormat (const AjPStr format, ajint* iformat);
AjBool       ajReportFormatDefault (AjPStr* pformat);
ajint        ajReportLists (const AjPReport thys,
			    AjPStr** types, AjPStr** names,
			    AjPStr** prints, ajint** tagsizes);
AjPReport    ajReportNew (void);
void         ajReportPrintFormat (const AjPFile outf, AjBool full);
AjPStr       ajReportSeqName (const AjPReport thys, AjPSeq seq);
void         ajReportSetHeader (AjPReport thys, const AjPStr header);
void         ajReportSetHeaderC (AjPReport thys, const char* header);
AjBool       ajReportSetTags (AjPReport thys,
			      const AjPStr taglist, ajint mintags);
void         ajReportSetTail (AjPReport thys, const AjPStr tail);
void         ajReportSetTailC (AjPReport thys, const char* tail);
void         ajReportSetType (AjPReport thys,
			      const AjPFeattable ftable, const AjPSeq seq);
void         ajReportTrace (const AjPReport thys);
AjBool       ajReportValid (AjPReport thys);
void         ajReportWrite (AjPReport thys,
			    AjPFeattable ftable,  AjPSeq seq);
void         ajReportWriteClose (AjPReport thys);
void         ajReportWriteHeader (AjPReport thys,
				  const AjPFeattable ftable, AjPSeq seq);
void         ajReportWriteTail (AjPReport thys,
				const AjPFeattable ftable, AjPSeq seq);

#endif

#ifdef __cplusplus
}
#endif

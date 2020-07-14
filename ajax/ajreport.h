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
** @alias AjSReport
** @alias AjOReport
**
** @other AjPSeqout Sequence output
** @other AjPFile Input and output files
**
** @attr Name [AjPStr] As "Source" for features, usually empty
** @attr Type [AjPStr] "P" Protein or "N" Nucleotide
** @attr Formatstr [AjPStr] Report format (-rformat)
** @attr Format [AjEnum] Report format (index number)
** @attr Fttable [AjPFeattable] Feature table to use (obsolete?)
** @attr Ftquery [AjPFeattabOut] Output definition for features
** @attr Extension [AjPStr] Output file extension
** @attr File [AjPFile] Output file object
** @attr Tagnames [AjPList] List of extra tag names (from ACD)
** @attr Tagprints [AjPList] List of extra tag printnames (from ACD)
** @attr Tagtypes [AjPList] List of extra tag datatypes (from ACD)
** @attr Header [AjPStr] Text to add to header with newlines
** @attr SubHeader [AjPStr] Text to add to subheader with newlines
** @attr Tail [AjPStr] Text to add to tail with newlines
** @attr SubTail [AjPStr] Text to add to subtail with newlines
** @attr FileNames [AjPList] Names of extra files (see FileTypes)
** @attr FileTypes [AjPList] Types of extra files (see FileNames)
** @attr Precision [ajint] Floating precision for score
** @attr Showacc [AjBool] Report accession number
** @attr Showdes [AjBool] Report sequence description
** @attr Showusa [AjBool] Report USA (-rusa) or only seqname
** @attr Showscore [AjBool] Report score (if optional for format)
** @attr Multi [AjBool] if true, assume >1 sequence
** @attr Mintags [ajint] Minimum number of tags to report
** @attr Count [ajint] Number of sequences reported so far
**
** @new ajReportNew Default constructor
** @delete ajReportDel Default destructor
** @output ajReportWrite Master sequence output routine
** @@
******************************************************************************/

typedef struct AjSReport {
  AjPStr Name;
  AjPStr Type;
  AjPStr Formatstr;
  AjEnum Format;
  AjPFeattable Fttable;
  AjPFeattabOut Ftquery;
  AjPStr Extension;
  AjPFile File;
  AjPList Tagnames;
  AjPList Tagprints;
  AjPList Tagtypes;
  AjPStr Header;
  AjPStr SubHeader;
  AjPStr Tail;
  AjPStr SubTail;
  AjPList FileNames;
  AjPList FileTypes;
  ajint Precision;
  AjBool Showacc;
  AjBool Showdes;
  AjBool Showusa;
  AjBool Showscore;
  AjBool Multi;
  ajint Mintags;
  ajint Count;
} AjOReport;

#define AjPReport AjOReport*

void         ajReportClose (AjPReport pthys);
void         ajReportDel (AjPReport* pthys);
void         ajReportDummyFunction(void);
void         ajReportFileAdd (AjPReport thys,
			      AjPFile file, const AjPStr type);
AjBool       ajReportFindFormat (const AjPStr format, ajint* iformat);
AjBool       ajReportFormatDefault (AjPStr* pformat);
ajint        ajReportLists (const AjPReport thys,
			    AjPStr** types, AjPStr** names,
			    AjPStr** prints, ajint** tagsizes);
AjPReport    ajReportNew (void);
AjBool       ajReportOpen (AjPReport thys, const AjPStr name);
void         ajReportPrintFormat (AjPFile outf, AjBool full);
const AjPStr ajReportSeqName (const AjPReport thys, const AjPSeq seq);
void         ajReportSetHeader (AjPReport thys, const AjPStr header);
void         ajReportSetHeaderC (AjPReport thys, const char* header);
void         ajReportSetSubHeader (AjPReport thys, const AjPStr header);
void         ajReportSetSubHeaderC (AjPReport thys, const char* header);
AjBool       ajReportSetTags (AjPReport thys,
			      const AjPStr taglist, ajint mintags);
void         ajReportSetTail (AjPReport thys, const AjPStr tail);
void         ajReportSetTailC (AjPReport thys, const char* tail);
void         ajReportSetSubTail (AjPReport thys, const AjPStr tail);
void         ajReportSetSubTailC (AjPReport thys, const char* tail);
void         ajReportSetType (AjPReport thys,
			      const AjPFeattable ftable, const AjPSeq seq);
void         ajReportTrace (const AjPReport thys);
AjBool       ajReportValid (AjPReport thys);
void         ajReportWrite (AjPReport thys,
			    const AjPFeattable ftable,  const AjPSeq seq);
void         ajReportWriteClose (AjPReport thys);
void         ajReportWriteHeader (AjPReport thys,
				  const AjPFeattable ftable, const AjPSeq seq);
void         ajReportWriteTail (AjPReport thys,
				const AjPFeattable ftable, const AjPSeq seq);

#endif

#ifdef __cplusplus
}
#endif

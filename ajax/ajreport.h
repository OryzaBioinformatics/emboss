#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqreport_h
#define ajseqreport_h

/* @data AjPReport *******************************************************
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
  ajint Count;
  AjPStr Name;
  AjPStr Type;
  AjPStr Formatstr;
  AjEnum Format;
  AjPFeattable Fttable;
  AjPFeattabOut Ftquery;
  AjPStr Filename;
  AjPStr Extension;
  AjPFile File;
} AjOReport, *AjPReport;

void         ajReportDel (AjPReport* pthys);
AjBool       ajReportHeadSeq (AjPReport thys, AjPSeq seq);
AjBool       ajReportOpen (AjPReport thys, AjPStr name);
AjBool       ajReportFindFormat (AjPStr format, ajint* iformat);
AjBool       ajReportFormatDefault (AjPStr* pformat);
AjPReport    ajReportNew (void);
void         ajReportTrace (AjPReport thys);
void         ajReportWrite (AjPReport thys, AjPFeattable ftable);
void         ajReportWriteClose (AjPReport thys);

#endif

#ifdef __cplusplus
}
#endif

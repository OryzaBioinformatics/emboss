/********************************************************************
** @source AJAX REPORT (ajax feature reporting) functions
**
** These functions report AJAX sequence feature data in a variety
** of formats.
**
** @author Copyright (C) 2000 Peter Rice, LION Bioscience Ltd.
** @version 1.0 
** @modified Nov 10 First version
** @@
** 
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
** 
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
** 
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
********************************************************************/

#include <stddef.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "ajax.h"

typedef struct ReportSFormat {
  char *Name;
  void (*Write) (AjPReport outrpt);
} ReportOFormat, *ReportPFormat;

static void reportWriteTrace (AjPReport outrpt);
static void reportWriteEmbl (AjPReport outrpt);
static void reportWriteGenbank (AjPReport outrpt);
static void reportWriteGff (AjPReport outrpt);

/* @funclist reportFormat *****************************************************
**
** Functions to write feature reports
**
******************************************************************************/

static ReportOFormat reportFormat[] = { 
  {"embl",      reportWriteEmbl},
  {"genbank",   reportWriteGenbank},
  {"gff",       reportWriteGff},
  {"trace",     reportWriteTrace}, /* trace contents for debugging */
  {NULL, NULL}
};

/* @funcstatic reportWriteTrace ***********************************************
**
** Writes a report in Trace format
**
** @param [R] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

static void reportWriteTrace (AjPReport thys) {

  ajFmtPrintF (thys->File, "Trace output\n");

  return;
}

/* @func reportWriteAntigenic *********************************************
**
** Writes a report in the format of the original antigenic program
**
** @param [R] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

void reportWriteAntigenic (AjPReport thys) {

  ajint i, j, k, m;
  ajint nhits=0;
  ajint istart=0;
  ajint iend=0;
  ajint minlen=0;
  ajint maxlen=0;
  ajint begin=0;
  ajint end=0;
  float minap=0.;
  float averap=0.;

  ajint fpos=0;
  ajint lpos=0;
  static AjPStr sstr = NULL;
  static AjPStr stmp = NULL;

  AjPFloat thisap=NULL;
  AjPFloat hwt=NULL;
  AjPInt   hpos=NULL;
  AjPInt   hp=NULL;
  AjPInt   hlen=NULL;
  AjPStr    strand=NULL;

  AjPSeq seq = NULL;
  AjPFile outf = thys->File;
  AjPFeattable ftab = thys->Fttable;

  /* == print the report header == */

  ajFmtPrintF(outf,"ANTIGENIC of %s  from: %d  to: %d\n\n",
	      ajSeqName(seq),begin,end);
  ajFmtPrintF(outf,"Length %d residues, score calc from %d to %d\n",
	      ajSeqLen(seq),fpos+3+begin,lpos+3+begin);
  ajFmtPrintF(outf,"Reporting all peptides over %d residues\n\n",minlen);
  ajFmtPrintF(outf,
	      "Found %d hits scoring over %.2f (true average %.2f)\n",
	      nhits,minap,averap);

  /* == end of report header == */

  /* == report top hit first == */

  ajFmtPrintF(outf,"Maximum length %d at residues %d->%d\n\n", maxlen,
	      istart+begin, iend+begin);
  ajFmtPrintF(outf," Sequence:  %S\n",stmp);
  ajFmtPrintF(outf,"            |");
  ajFmtPrintF(outf,"|\n");
  ajFmtPrintF(outf,"%13d",istart+begin);
  for(i=0;i<iend-istart-1;++i) {
    ajFmtPrintF(outf," ");
  }

  ajFmtPrintF(outf,"%d\n",iend+begin);

  /* == end of top hit report */

  /* == print the report features == */

  nhits = ajFeatSize(ftab);
  if(nhits > 1) {
    ajFmtPrintF(outf,
		"\nEntries in score order, max score at \"*\"\n\n");

    /* step through the feature table */

    for(i=0,j=0;i < nhits;--i) {
      k = ajIntGet(hp,i);
      istart = ajIntGet(hpos,k);
      iend = istart + ajIntGet(hlen,k) -1;
      ajFmtPrintF(outf,
		  "\n[%d] Score %.3f length %d at residues %d->%d\n",
		  ++j,ajFloatGet(hwt,k),ajIntGet(hlen,k),
		  istart+begin,iend+begin);
      ajFmtPrintF(outf,"            ");
      for(m=istart;m<=iend;++m) {
	if(ajFloatGet(thisap,m) == ajFloatGet(hwt,k))
	  ajFmtPrintF(outf,"*");
	else
	  ajFmtPrintF(outf," ");
      }
      ajFmtPrintF(outf,"\n");
      ajStrAssSubC(&stmp,ajStrStr(sstr),istart,iend);	   ;
      ajFmtPrintF(outf," Sequence:  %S\n",stmp);
      ajFmtPrintF(outf,"            |");
      for(i=0;i<iend-istart-1;++i) {
	ajFmtPrintF(outf," ");
      }
      ajFmtPrintF(outf,"|\n");
      ajFmtPrintF(outf,"%13d",istart+begin);
      for(i=0;i<iend-istart-1;++i) {
	ajFmtPrintF(outf," ");
      }
      ajFmtPrintF(outf,"%d\n",iend+begin);
    }
  }

  /* == end of report features == */

  ajStrDel(&strand);

  return;
}

/* @funcstatic reportWriteEmbl ************************************************
**
** Writes a report in EMBL format
**
** @param [R] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

static void reportWriteEmbl (AjPReport thys) {

  static AjPStr ftfmt = NULL;

  if (!ftfmt)
    ajStrAssC (&ftfmt, "embl");

  ajFmtPrintF (thys->File, "#EMBL output\n");

  thys->Ftquery = ajFeattabOutNewSSF (ftfmt, thys->Name,
				      ajStrStr(thys->Type),
				      thys->File);
  if (!ajFeatWrite (thys->Ftquery, thys->Fttable)) {
    ajWarn ("ajReportWrite features output failed format: '%S'",
	    ftfmt);
  }
  return;
}

/* @funcstatic reportWriteGenbank *********************************************
**
** Writes a report in Genbank format
**
** @param [R] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

static void reportWriteGenbank (AjPReport thys) {

  static AjPStr ftfmt = NULL;

  if (!ftfmt)
    ajStrAssC (&ftfmt, "genbank");

  ajFmtPrintF (thys->File, "#Genbank output\n");

  thys->Ftquery = ajFeattabOutNewSSF (ftfmt, thys->Name,
				      ajStrStr(thys->Type),
				      thys->File);
  if (!ajFeatWrite (thys->Ftquery, thys->Fttable)) {
    ajWarn ("ajReportWrite features output failed format: '%S'",
	    ftfmt);
  }
  return;
}
/* @funcstatic reportWriteGff *************************************************
**
** Writes a report in GFF format
**
** @param [R] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

static void reportWriteGff (AjPReport thys) {

  static AjPStr ftfmt = NULL;

  if (!ftfmt)
    ajStrAssC (&ftfmt, "gff");

  ajFmtPrintF (thys->File, "#GFF output\n");

  thys->Ftquery = ajFeattabOutNewSSF (ftfmt, thys->Name,
				      ajStrStr(thys->Type),
				      thys->File);
  if (!ajFeatWrite (thys->Ftquery, thys->Fttable)) {
    ajWarn ("ajReportWrite features output failed format: '%S'",
	    ftfmt);
  }
  return;
}

/* @func ajReportDel **********************************************************
**
** Destructor for report objects
**
** @param [D] pthys [AjPReport*] Report object reference
** @return [void]
** @@
******************************************************************************/

void ajReportDel (AjPReport* pthys) {

  AjPReport thys = *pthys;

  ajStrDel (&thys->Name);
  ajStrDel (&thys->Formatstr);
  ajStrDel (&thys->Filename);
  ajStrDel (&thys->Extension);

  AJFREE(*pthys);

  return;
}

/* @func ajReportOpen *********************************************************
**
** Opens a new report file
**
** @param [R] thys [AjPReport] Report object
** @param [R] name [AjPStr] File name
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

AjBool ajReportOpen (AjPReport thys, AjPStr name) {

  thys->File = ajFileNewOut(name);
  if (thys->File)
    return ajTrue;

  return ajFalse;
}

/* @func ajReportFormatDefault ************************************************
**
** Sets the default format for a feature report
**
** @param [W] pformat [AjPStr*] Default format returned
** @return [AjBool] ajTrue is format was returned
** @@
******************************************************************************/

AjBool ajReportFormatDefault (AjPStr* pformat) {

  if (ajStrLen(*pformat)) {
    ajDebug ("... output format '%S'\n", *pformat);
  }
  else {
    /* ajStrSetC (pformat, reportFormat[0].Name);*/
    (void) ajStrSetC (pformat, "gff"); /* use the real name */
    ajDebug ("... output format not set, default to '%S'\n", *pformat);
  }

  return ajTrue;
}

/* @func ajReportFindFormat ***********************************************
**
** Looks for the specified report format in the internal definitions and
** returns the index.
**
** @param [P] format [AjPStr] Format required.
** @param [w] iformat [ajint*] Index
** @return [AjBool] ajTrue on success.
** @@
******************************************************************************/

AjBool ajReportFindFormat (AjPStr format, ajint* iformat) {

  AjPStr tmpformat = NULL;
  ajint i = 0;

  if (!ajStrLen(format))
    return ajFalse;

  (void) ajStrAss (&tmpformat, format);
  (void) ajStrToLower(&tmpformat);

  while (reportFormat[i].Name) {
    if (ajStrMatchCaseC(tmpformat, reportFormat[i].Name)) {
      *iformat = i;
      ajStrDel(&tmpformat);
      return ajTrue;
    }
    i++;
  }

  ajStrDel(&tmpformat);
  return ajFalse;
}

/* @func ajReportNew ************************************************
**
** Constructor for a report object
**
** @return [AjPReport] New report object
** @@
******************************************************************************/

AjPReport ajReportNew (void) {

  AjPReport pthis;

  AJNEW0(pthis);

  pthis->Count = 0;
  pthis->Name = ajStrNew();
  pthis->Formatstr = ajStrNew();
  pthis->Format = 0;
  pthis->Fttable = NULL;
  pthis->Ftquery = ajFeattabOutNew();
  pthis->Filename = ajStrNew();
  pthis->Extension = ajStrNew();
  pthis->File = NULL;

  return pthis;
}

/* @func ajReportWrite ************************************************
**
** Writes a feature report
**
** @param [R] thys [AjPReport] Report object
** @param [R] ftable [AjPFeattable] Feature table object
** @return [void]
** @@
******************************************************************************/

void ajReportWrite (AjPReport thys, AjPFeattable ftable) {

  ajDebug ("ajReportWrite\n");	/* add ftable name and size */

  if (!thys->Format) {
    if (!ajReportFindFormat(thys->Formatstr, &thys->Format)) {
      ajErr ("unknown report format '%S'", thys->Formatstr);
    }
  }

  ajDebug ("ajReportWrite %d '%s'\n",
	   thys->Format, reportFormat[thys->Format].Name);

  reportFormat[thys->Format].Write (thys);

  return;
}

/* @func ajReportClose ********************************************************
**
** Closes a feature report
**
** @param [R] thys [AjPReport] Report object
** @return [void]
** @@
******************************************************************************/

void ajReportClose (AjPReport thys) {

  ajDebug ("ajReportClose '%F'\n", thys->File);

  ajFileClose (&thys->File);

  return;
}


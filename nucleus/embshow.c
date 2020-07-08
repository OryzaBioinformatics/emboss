/* @source embshow.c
**
** General routines for sequence display.
** Copyright (c) 2000 Gary Williams
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
******************************************************************************/

#include "emboss.h"
#include <ctype.h>

/************************************
**
** TO ADD A NEW DISPLAY TYPE:
**
*************************************

Example: complement of the sequence

Create a new type in the descriptor object types 'enum ShowEValtype'. 
eg: SH_COMP
Use this to refer to this type in later routines. (eg: by embShowAddComp())

Create a structure to hold information about what options for this type
you can have. 
eg: typedef struct EmbSShowComp { ... }

Create a function called by the user to set up the new type as the next
thing to be displayed in the list of things.
eg: embShowAddComp()

Create the routine to actually output a line's length of whatever is
being displayed from position 'pos' of the sequence - the output is
added to the end of a list of AjPStr. 
Everything to be printed should be ajListPushApp'd on to the 'lines' list.
These strings need not be complete lines - you can push many strings of
partial lines if you prefer.
End the lines to be output by pushing a string ending with a '\n'.
As many lines as you wish may be pushed onto the lines list.
eg: showFillComp()

Add a case statement to showFillLines to call the showFill* routine.
eg:
      case SH_COMP:
      showFillComp(this, lines, info, pos);
      break;



*/


/* @data EmbPShowInfo ***************************************************
**
** 
** The sequence and associated things to show are held in an ordered list
** of type EmbPShowInfo. This list is held in the structure EmbPShow.
** 
** The things to show are displayed around the sequence in the order that
** they are held on the list. 
** 
** EmbPShowInfo holds the descriptor (one of EmbPShowBlank, EmbPShowTicks,
** EmbPShowSeq, EmbPShowComp, etc.) and the type of the descriptor (one of
** SH_BLANK, SH_TICKS, SH_SEQ, SH_COMP, etc.  )
** 
** Each descriptor (EmbPShowSeq, EmbPShowBlank, EmbPShowTicks, etc.) holds
** information that could be useful in displaying its type of information. 
** 
** So, for example:
** 
** EmbPShow could have a list of:
** ----------------------------
**
** EmbPShowInfo->type=SH_BLANK
**    |       ->info=EmbPShowBlank
**    |
** EmbPShowInfo->type=SH_TICKS
**    |       ->info=EmbPShowTicks
**    |
** EmbPShowInfo->type=SH_SEQ
**    |       ->info=EmbPShowSeq
**    |
** EmbPShowInfo->type=SH_COMP
**    |       ->info=EmbPShowComp
**    |
** EmbPShowInfo->type=etc.
**    |       ->info=etc.
**    |
**   etc.
** 
**
** 
** @@
******************************************************************************/

static void    showFillRE(EmbPShow thys, AjPList lines,
			  EmbPShowRE info, int pos);
static void    showFillREflat(EmbPShow thys, AjPList lines, EmbPShowRE info,
			      int pos);
static void    showFillREupright(EmbPShow thys, AjPList lines,
				EmbPShowRE info, int pos);
static int     showFillREuprightSort (const void* a, const void* b);
static void    showOverPrint(AjPStr *target, int start, AjPStr insert);
static AjBool  showLineIsClear(AjPStr *line, int start, int end);
static void    showFillLines(AjPList lines, EmbPShow thys, int pos);
static void    showPrintLines(AjPFile out, AjPList lines);
static void    showMargin(EmbPShow thys, AjPList lines);
static void    showMarginNumber(EmbPShow thys, AjPList lines, int number);
static void    showPad(AjPList lines, int number);
static void    showUpperRange(AjPStr *line, AjPRange upperrange, int pos);
static void    showColourRange(AjPStr *line, AjPRange colour, int pos);
static void    showInsertHTML(AjPStr *target, int pos, AjPStr insert);

static void    showFillSeq(EmbPShow thys, AjPList lines, EmbPShowSeq info,
			   int pos);
static void    showFillBlank(EmbPShow thys, AjPList lines, EmbPShowBlank info,
			     int pos);
static void    showFillTicks(EmbPShow thys, AjPList lines, EmbPShowTicks info,
			     int pos);
static void    showFillTicknum(EmbPShow thys, AjPList lines,
			       EmbPShowTicknum info, int pos);
static void    showFillComp(EmbPShow thys, AjPList lines, EmbPShowComp info,
			    int pos);
static void    showFillTran(EmbPShow thys, AjPList lines, EmbPShowTran info,
			    int pos);
static void    showFillFT(EmbPShow thys, AjPList lines, EmbPShowFT info,
			  int pos);

static void showDelSeq (EmbPShowSeq info);
static void showDelBlank (EmbPShowBlank info);
static void showDelTicks (EmbPShowTicks info);
static void showDelTicknum (EmbPShowTicknum info);
static void showDelComp (EmbPShowComp info);
static void showDelTran (EmbPShowTran info);
static void showDelRE (EmbPShowRE info);
static void showDelFT (EmbPShowFT info);
static void showAddTags(AjPStr *tagsout, AjPList taglist, AjBool values);
	


/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Show Sequence Constructors ***************************************
**
** All constructors return a new show sequence object by pointer.
** The target pointer does not need to be initialised to NULL, but it is
** good programming practice to do so anyway. 
**
******************************************************************************/


/* @func embShowNew ******************************************************
**
** Creates a new sequence show object.
**
** @param [r] seq [AjPSeq] Sequence to describe
** @param [r] begin [int] start position in  sequence
** @param [r] end [int] end position in  sequence
** @param [r] width [int] width of displayed sequence on a line
** @param [r] length [int] length of a page in lines (0=no length)
** @param [r] margin [int] margin for numbers etc.
** @param [r] html [AjBool] format output for HTML
** @param [r] offset [int] number to start display of position numbering at
** @return [EmbPShow] New sequence show object.
** @@
******************************************************************************/

EmbPShow embShowNew (AjPSeq seq, int begin, int end, int width, 
		    int length, int margin, AjBool html, int offset) {
	
  EmbPShow pthis;
  
  AJNEW0(pthis);
  
  pthis->list = ajListNew();

/* information about the sequence */
  pthis->seq = seq;			/* the sequence */
  pthis->nucleic = ajSeqIsNuc(seq);	/* ajTrue = sequence is nucleic */
  pthis->start = begin;
  pthis->end = end;

/* information about the page layout */
  pthis->width = width;		/* length of sequence per line */
  pthis->length = length;	/* length of a page (0 = indefinite) */
  pthis->margin = margin;	/* margin for numbering */
  pthis->html = html;		/* ajTrue = format for HTML */
  pthis->offset = offset;	/* number to start displaying with */
  
  return pthis;
}
  



/* @funcstatic showInfoNew **************************************************
**
** Creates a new descriptor structure to be pushed on the list
**
** @param [r] info [void*] descriptor
** @param [r] type [int] type of descriptor
** @return [EmbPShowInfo] New sequence show object.
** @@
******************************************************************************/

static EmbPShowInfo showInfoNew (void* info, int type) {
	
  EmbPShowInfo pthis;
  
  AJNEW0(pthis);
  
  pthis->info = info;
  pthis->type = type;
  
  return pthis;
}
  



/* ==================================================================== */
/* ========================== destructors ============================= */
/* ==================================================================== */

/* @section Show Sequence Destructors ****************************************
**
** Destruction destroys all internal data structures and frees the
** memory allocated for the show sequence object.
**
******************************************************************************/

/* @func embShowDel ***********************************************************
**
** Deletes a show sequence object.
**
** @param [P] pthis [EmbPShow*] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowDel (EmbPShow* pthis) {

  EmbPShow thys = *pthis;
  AjIList iter;
  EmbPShowInfo infostruct;
  int type;			/* descriptor type */
  void * info;			/* descriptor */
  
(void) ajDebug("embShowDel\n");

/* ***
****** MOST EMPHATICALLY DO NOT:
****** <free the sequence>
****** 
****** It took me a long time to work out why showseq was crashing after
****** the first sequence.
****** 
****** It was because I was dutifully freeing up all the data held in
****** this structure and 'seq' is later freed by ajSeqallNext()
******/
/*****  DO NOT: ajSeqDel(&pthis->seq); *****/


/* free the descriptors */
  iter = ajListIter(thys->list);

  while ((infostruct = ajListIterNext(iter)) != NULL) {

/* iterate through the descriptors filling out the lines */
    type = infostruct->type;
    info = infostruct->info;

    switch (type) {
      case SH_SEQ:
      (void) showDelSeq(info);
      break;
      
      case SH_BLANK:
      (void) showDelBlank(info);
      break;
      
      case SH_TICK:
      (void) showDelTicks(info);
      break;

      case SH_TICKNUM:
      (void) showDelTicknum(info);
      break;

      case SH_COMP:
      (void) showDelComp(info);
      break;

      case SH_TRAN:
      (void) showDelTran(info);
      break;

      case SH_RE:
      (void) showDelRE(info);
      break;

      case SH_FT:
      (void) showDelFT(info);
      break;

      default:
      (void) ajFatal("Unknown descriptor type found in embShowDel(): %d",
		     type);
    }


    AJFREE(infostruct);
  }

  (void) ajListIterFree(iter);

/* we have already freed the descriptors, so use ajListDel() here */ 
  (void) ajListDel(&thys->list);

  AJFREE(*pthis);

  return;
}

/* @funcstatic showDelSeq ***********************************************************
**
** Deletes a show sequence descriptor object.
**
** @param [P] info [EmbPShowSeq] Show sequence descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelSeq (EmbPShowSeq info) {

  AJFREE(info);

}

/* @funcstatic showDelBlank ***********************************************************
**
** Deletes a show blank descriptor object.
**
** @param [P] info [EmbPShowBlank] Show blank descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelBlank (EmbPShowBlank info) {

  AJFREE(info);

}

/* @funcstatic showDelTicks ***********************************************************
**
** Deletes a show ticks descriptor object.
**
** @param [P] info [EmbPShowTicks] Show ticks descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelTicks (EmbPShowTicks info) {

  AJFREE(info);

}

/* @funcstatic showDelTicknum ***********************************************************
**
** Deletes a show tick numbers descriptor object.
**
** @param [P] info [EmbPShowTicknum] Show tick numbers descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelTicknum (EmbPShowTicknum info) {

  AJFREE(info);

}

/* @funcstatic showDelComp ***********************************************************
**
** Deletes a show complement descriptor object.
**
** @param [P] info [EmbPShowComp] Show complement descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelComp (EmbPShowComp info) {

  AJFREE(info);

}

/* @funcstatic showDelTran ***********************************************************
**
** Deletes a show translation descriptor object.
**
** @param [P] info [EmbPShowTran] Show translation descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelTran (EmbPShowTran info) {

  ajSeqDel (&info->transeq);
  AJFREE(info);

}

/* @funcstatic showDelRE ***********************************************************
**
** Deletes a show restriction enzyme descriptor object.
**
** @param [P] info [EmbPShowRE] Show restriction enzyme descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelRE (EmbPShowRE info) {

  ajListFree(&info->sitelist);
  AJFREE(info);

}

/* @funcstatic showDelFT ***********************************************************
**
** Deletes a show feature table descriptor object.
**
** @param [P] info [EmbPShowFT] Show feature table descriptor object
** @return [void]
** @@
******************************************************************************/

static void showDelFT (EmbPShowFT info) {

  (void) ajFeatTabDel(&(info->feat));
  AJFREE(info);

}


/* ==================================================================== */
/* ========================== Assignments ============================= */
/* ==================================================================== */

/* @section Show Sequence Assignments ***************************************
**
** These functions add to the show sequence object provided as the
** first argument.
**
******************************************************************************/

/* @func embShowAddSeq ********************************************************
**
** Adds the sequence to be displayed to the list of things to show
** This must be done before the final printing is done as without a sequence
** to hang all the other features and things on, there can be no output to
** show.
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [r] number [AjBool] Number the sequence
** @param [r] threeletter [AjBool] Use three letter protein code
** @param [r] upperrange [AjPRange] Range of sequence to uppercase
** @param [r] colour [AjPRange] Range of sequence to colour in HTML
** @return [void]
** @@
******************************************************************************/

void embShowAddSeq (EmbPShow thys, AjBool number, AjBool threeletter,
AjPRange upperrange, AjPRange colour) {

  EmbPShowSeq info;
  ajDebug("embShowAddSeq\n");
  
  AJNEW0(info);
  
  info->number = number;		/* number */
  info->threeletter = threeletter;	/* use three-letter protein code */
  info->upperrange = upperrange;	/* Range of sequence to uppercase */
  info->highlight = colour;		/* Range to colour in HTML */

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_SEQ));

  return;
}

/* @func embShowAddBlank ******************************************************
**
** Adds a blank line to the list of things to show.
**
** @param [P] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowAddBlank (EmbPShow thys) {

  EmbPShowBlank info;
  (void) ajDebug("embShowAddBlank\n");
  
  AJNEW0(info);
  
  (void) ajListPushApp(thys->list, showInfoNew(info, SH_BLANK));

  return;
}

/* @func embShowAddTicks ******************************************************
**
** Adds a ticks line to the list of things to show.
**
** @param [P] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowAddTicks (EmbPShow thys) {

  EmbPShowTicks info;
(void) ajDebug("embShowAddTicks\n");
  
  AJNEW0(info);

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_TICK));

  return;
}

/* @func embShowAddTicknum ****************************************************
**
** Adds a ticks number line to the list of things to show.
**
** @param [P] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowAddTicknum (EmbPShow thys) {

  EmbPShowTicknum info;
(void) ajDebug("embShowAddTicknum\n");
  
  AJNEW0(info);

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_TICKNUM));

  return;
}

/* @func embShowAddComp *******************************************************
**
** Adds the sequence complement to be displayed to the list of things to show
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [r] number [AjBool] ajTrue = number the complement
** @return [void]
** @@
******************************************************************************/

void embShowAddComp (EmbPShow thys, AjBool number) {

  EmbPShowComp info;
(void) ajDebug("embShowAddComp\n");
  
  AJNEW0(info);
  
  info->number = number;	/* number */

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_COMP));

  return;
}

/* @func embShowAddTran *******************************************************
**
** Adds the translation to be displayed to the list of things to show
**
** @param [r] thys [EmbPShow] Show sequence object
** @param [r] trnTable [AjPTrn] Translation table
** @param [r] frame [int] Reading frame to translate
** @param [r] threeletter [AjBool] ajTrue for 3 letter code
** @param [r] number [AjBool] ajTrue for numbering
** @param [r] regions [AjPRange] Sequence range(s)
** @param [r] orfminsize [int] Minimum length of ORF to be shown
** @return [void]
** @@
******************************************************************************/

void embShowAddTran (EmbPShow thys, AjPTrn trnTable, int frame,
		     AjBool threeletter, AjBool number, AjPRange regions, 
		     int orfminsize) {

  EmbPShowTran info;
(void) ajDebug("embShowAddTran\n");
  
  AJNEW0(info);

  info->trnTable = trnTable;	/* translation table */
  info->frame = frame;		/* 1,2,3,-1,-2 or -3 = frame to translate */
  info->threeletter = threeletter;	/* ajTrue = display in 3 letter code */
  info->regions = regions;	/* only translate these regions, NULL = all */
  info->number = number;	/* ajTrue = number the translation */
  info->orfminsize = orfminsize;/* minimum size of ORF to display */

/* these are used by showFillTran */
  info->transeq = NULL;	/* we have not yet stored the translation here */
  info->tranpos = 0;		/* store translation position for numbering */

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_TRAN));

  return;
}

/* @func embShowAddRE *********************************************************
**
** Adds the Ristriction Enzymes to be displayed to the list of things to show
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [r] sense [int] sense to translate (+1 or -1)
** @param [r] restrictlist [AjPList] restriction enzyme cut site list
** @param [r] flat [AjBool] show in flat format with recognition sites
** @return [void]
** @@
******************************************************************************/

void embShowAddRE (EmbPShow thys, int sense, AjPList restrictlist,
		   AjBool flat) {
  EmbPShowRE info;
(void) ajDebug("embShowAddRE\n");
  
  AJNEW0(info);

  info->sense = sense;		/* 1 or -1 = sense to translate */
  info->flat = flat;		/* upright or flat display */
  info->hits = ajListLength(restrictlist);
  info->matches = restrictlist;
  info->sitelist = NULL;	/* show we have not yet created this list */

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_RE));

  return;
}

/* @func embShowAddFT *********************************************************
**
** Adds the Features to be displayed to the list of things to show
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [r] feat [AjPFeatTable] features
** @return [void]
** @@
******************************************************************************/

void embShowAddFT (EmbPShow thys, AjPFeatTable feat) {

  EmbPShowFT info;
  (void) ajDebug("embShowAddFT\n");
  
  AJNEW0(info);

  info->feat = feat;	/* store the feature table */

  (void) ajListPushApp(thys->list, showInfoNew(info, SH_FT));

  return;
}

/* ==================================================================== */
/* =========================== Modifiers ============================== */
/* ==================================================================== */

/* @section Show Sequence Modifiers *******************************************
**   
** These functions modify the behaviour of the last show descriptor on
** the list.
**
**
******************************************************************************/






/* ==================================================================== */
/* ======================== Operators ==================================*/
/* ==================================================================== */
    
/* @section Show Sequence Operators *******************************************
**
** These functions use the contents of a Show object but do not modify it.
**
******************************************************************************/
   

/* @func embShowPrint *********************************************************
**
** Prints a Show object
**
** @param [r] out [AjPFile] Output file handle
** @param [P] thys [EmbPShow] Show sequence object
** @return [void]
** @@
******************************************************************************/

void embShowPrint(AjPFile out, EmbPShow thys) {

  AjPList lines;		/* list of lines to be printed */
  int pos;			/* current printing position in sequence */
  int start, end;		/* sequence postions to print between */
  AjIList liter;		/* iterator for lines */
  AjPStr line;
  int count;			/* count of newlines in the list */
  int line_no = 0;		/* line number on page */

(void) ajDebug("embShowPrint\n");
/* set up the start and end positions to print */
  start = thys->start;
  end = thys->end;

/* run through the whole sequence, line-width by line-width */  
  for (pos = start; pos<=end; pos += thys->width) {
/* make a new list of lines */
    lines=ajListstrNew();
/* put the sequence and any other descriptors in the lines list */
    (void) showFillLines(lines, thys, pos);

/* throw a formfeed if we would go over the length of the page */
    count = 0;
    liter = ajListIter(lines);
    while ((line = ajListIterNext(liter)) != NULL) {
/* we assume that newline characters only occur at the end of lines */
      if (ajStrStr(line)[ajStrLen(line)-1] == '\n') count++;
    }
    (void) ajListIterFree(liter);
/* thys->length is zero if we have an indefinite page length */
    if (thys->length && (count+line_no > thys->length) && (count < thys->length)) {
      line_no = 0;
      (void) ajFmtPrintF(out, "%c", '\f');	/* form feed character */
    }

/* print the lines */
    (void) showPrintLines(out, lines);

/* delete */
    (void) ajListstrFree(&lines);
    
  }

}

/* @funcstatic showPrintLines *************************************************
**
** Print the lines to the output.
**
** @param [P] out [AjPFile] Output file handle
** @param [P] lines [AjPList] lines to print
** @return [void]
** @@
******************************************************************************/

static void showPrintLines(AjPFile out, AjPList lines) {
	
  AjIList liter;		/* iterator for lines */
  AjPStr str;

(void) ajDebug("showPrintLines\n");

/* iterate through the lines and print them */
  liter = ajListIter(lines);
  while ((str = ajListIterNext(liter)) != NULL) {
    (void) ajFmtPrintF(out, "%S", str);
  }
  (void) ajListIterFree(liter);

}

/* ==================================================================== */
/* ======================== Assignments ================================*/
/* ==================================================================== */
    

/* @section Show Fill Assignments *******************************************
**
** These functions fill out the sequence and features lines according to the
** descriptor data.
**
******************************************************************************/


/* @funcstatic showFillLines **************************************************
**
** Calls the descriptor routines to fill the lines.
**
** @param [P] lines [AjPList] Lines list
** @param [P] thys [EmbPShow] Show sequence object
** @param [r] pos [int] position in sequence so far while printing
** @return [void]
** @@
******************************************************************************/

static void showFillLines(AjPList lines, EmbPShow thys, int pos) {

  EmbPShowInfo infostruct;	/* structure of type and descriptor */
  int type;			/* descriptor type */
  void * info;			/* descriptor */

  AjIList diter;		/* iterator for descriptors */

(void) ajDebug("showFillLines\n");
/* iterate through the descriptors filling out the lines */
  diter = ajListIter(thys->list);

  while ((infostruct = ajListIterNext(diter)) != NULL) {
    type = infostruct->type;
    info = infostruct->info;

    switch (type) {
      case SH_SEQ:
      (void) showFillSeq(thys, lines, info, pos);
      break;
      
      case SH_BLANK:
      (void) showFillBlank(thys, lines, info, pos);
      break;
      
      case SH_TICK:
      (void) showFillTicks(thys, lines, info, pos);
      break;

      case SH_TICKNUM:
      (void) showFillTicknum(thys, lines, info, pos);
      break;

      case SH_COMP:
      (void) showFillComp(thys, lines, info, pos);
      break;

      case SH_TRAN:
      (void) showFillTran(thys, lines, info, pos);
      break;

      case SH_RE:
      (void) showFillRE(thys, lines, info, pos);
      break;

      case SH_FT:
      (void) showFillFT(thys, lines, info, pos);
      break;

      default:
      (void) ajFatal("Unknown descriptor type found in showFillLines(): %d",
		     type);
    }
  }
  (void) ajListIterFree(diter);

}

/* @funcstatic showMargin *****************************************************
**
** Add a blank margin to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @return [void]
** @@
******************************************************************************/
static void showMargin(EmbPShow thys, AjPList lines) {
	
  AjPStr marginfmt=ajStrNewL(10);


/* variable width margin */
  if (thys->margin) {
    (void) ajFmtPrintS(&marginfmt, "%%-%ds ", thys->margin-1);
    (void) ajListstrPushApp(lines, ajFmtStr(ajStrStr(marginfmt), ""));
  }
  (void) ajStrDel(&marginfmt);
}

/* @funcstatic showMarginNumber ***********************************************
** Add a margin containing a number to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] number [int] number to display
** @return [void]
** @@
******************************************************************************/
static void showMarginNumber(EmbPShow thys, AjPList lines, int number) {
	
  AjPStr marginfmt=ajStrNewL(10);


/* variable width margin containing a number */
  if (thys->margin) {
    (void) ajFmtPrintS(&marginfmt, "%%%dd ", thys->margin-1);
    (void) ajListstrPushApp(lines, ajFmtStr(ajStrStr(marginfmt), number));
  }
  (void) ajStrDel(&marginfmt);
}

/* @funcstatic showPad ********************************************************
**
** Add a set of space characters to the lines list to pad out an output line
**
** @param [u] lines [AjPList] list of lines to add to
** @param [r] number [int] number of space characters to output
** @return [void]
** @@
******************************************************************************/
static void showPad(AjPList lines, int number) {
	
  AjPStr marginfmt=ajStrNewL(10);


/* variable width pad of spaces */
  if (number>0) {
    (void) ajFmtPrintS(&marginfmt, "%%-%ds", number);
    (void) ajListstrPushApp(lines, ajFmtStr(ajStrStr(marginfmt), ""));
  }
  (void) ajStrDel(&marginfmt);
}

/* @funcstatic showUpperRange *************************************************
**
** Uppercase a string from a sequence with a range
** I.e ranges of a sequence are to be uppercased.
** We have a small region of the original sequence in a string.
** We want to uppercase any bits of the string that are in the ranges.
**
** @param [u] line [AjPStr *] line to uppercase if it is in the ranges
** @param [r] upperrange [AjPRange] range of original sequence to uppercase
** @param [r] pos [int] position in sequence that line starts at
** @return [void]
** @@
******************************************************************************/
static void showUpperRange(AjPStr * line, AjPRange upperrange, int pos) {

  int nr = ajRangeNumber(upperrange);
  int i,j;
  int start;	/* start of next range */
  int end;	/* end of next range */
  int value;	/* code for type of overlap of range with line */
  char *p;	/* ptr to start of range in line to uppercase */

  for(i=0; i<nr; i++) {	/* for each range in AjPRange upperrange */
    (void) ajRangeValues(upperrange, i, &start, &end);
/* get type of overlap */    
    value = ajRangeOverlapSingle (start, end, pos, ajStrLen(*line));

/* complete overlap */
    if (value == 2) {	
      (void) ajStrToUpper(line);
      return;

/* partial overlap */      
    } else if (value) {
      start--;	/* change start,end from human-readable position*/
      end--;			/*to index into sequence */
      if (start < pos) start = pos;
      p = ajStrStr(*line)+start-pos;
      for (j=start; *p && j<=end; j++, p++) {
      	if (pos-j < ajStrLen(*line)) {
          *p = toupper((int) *p);
        }
      }
    }
  }
  
	
}

/* @funcstatic showColourRange ************************************************
**
** colour a string from a sequence with a range
** I.e ranges of a sequence are to be coloured in HTML.
** We have a small region of the original sequence in a string.
** We want to colour any bits of the string that are in the ranges.
**
** @param [u] line [AjPStr *] line to colour if it is in the ranges
** @param [r] colour [AjPRange] range of original sequence to colour
** @param [r] pos [int] position in sequence that line starts at
** @return [void]
** @@
******************************************************************************/
static void showColourRange(AjPStr * line, AjPRange colour, int pos) {

  int nr = ajRangeNumber(colour);
  int i;
  int start;	/* start of next range */
  int end;	/* end of next range */
  int value;	/* code for type of overlap of range with line */
  AjPStr html = NULL;
  AjPStr col;

  for(i=0; i<nr; i++) {	/* for each range in AjPRange colour */
    (void) ajRangeValues(colour, i, &start, &end);

/* get type of overlap */    
    value = ajRangeOverlapSingle (start, end, pos, ajStrLen(*line));

/* partial or complete overlap */      
    if (value) {
      start--;	/* change start from human-readable position to sequence pos */
      end--;	/* ditto end */

      start -= pos;
      end -= pos;
      
      if (start < 0) start = 0;
      if (end > ajStrLen(*line)-1) end = ajStrLen(*line)-1;

/* start */
      ajStrAssC(&html, "<font color=");
      ajRangeText(colour, i, &col);
      if (col != NULL && ajStrLen(col)) {
        ajStrApp(&html, col);
      } else {	/* if no colour specified in the range, use 'red' as default */
      	ajStrAppC(&html, "red");
      }
      ajStrAppC(&html, ">");
      showInsertHTML(line, start, html);

/* end */
      ajStrAssC(&html, "</font>");
      showInsertHTML(line, end+1, html); /* end+1 because want the tag after */
				         /*this position */

    }
  }
  
/* tidy up */
  ajStrDel(&html);
	
}

/* @funcstatic showInsertHTML *************************************************
**
** Insert a string at a position in another string
** The position ignores any inserted HTML tags (anything between '<>')
** For example, insert "*" at position 3 of "<html tag>0123"
** gives "<html tag>012*3"
**
** If the insert position is past the end of the string, it inserts at the end.
**
** @param [u] target [AjPStr *] HTMLised string to insert into
** @param [r] pos [int] position (ignoreing HTML tags) to insert at
** @param [r] insert [AjPStr] string to insert
** @return [void]
** @@
******************************************************************************/
static void showInsertHTML(AjPStr *target, int pos, AjPStr insert) {

  int i, j;
  AjBool tag=ajFalse;

  for (i=0, j=0; j<pos && i<ajStrLen(*target); i++) {
    if (tag == ajFalse) {
      if (ajStrStr(*target)[i] == '<') {	/* start tag */
      	tag = ajTrue;
      } else {
        j++;	/* count the non-tag characters */
      }
    } else {
      if (ajStrStr(*target)[i] == '>') {	/* end tag */
      	tag = ajFalse;
      }
    }    	
  }

  ajStrInsert(target, i, insert);

}


/* @funcstatic showFillSeq ***************************************************
**
** Add this line's worth of sequence to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowSeq] data on how to display the sequence data
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillSeq(EmbPShow thys, AjPList lines, EmbPShowSeq info,
			int pos) {
	
  AjPStr line=ajStrNewL(81);

  AjPStr line1;	/* used to make the three-letter codes */
  AjPStr line2;
  AjPStr line3;
  char *p, *p3;
  int count;

(void) ajDebug("showFillSeq\n");

/* variable width margin at left with optional number in it */
  if (info->number) {
    (void) showMarginNumber(thys, lines, pos+thys->offset);
  } else {
    (void) showMargin(thys, lines);
  }

/* get the bit of the sequence to display */
  (void) ajStrAppSub(&line, ajSeqStr(thys->seq), pos, pos+thys->width-1);

/* change to three-letter code */
  if (!thys->nucleic && info->threeletter) {

    line1=ajStrNewL(81);
    line2=ajStrNewL(81);
    line3=ajStrNewL(81);
    for (count=0, p=ajStrStr(line); count < ajStrLen(line); count++, p++) {

      if (*p == '*') {
        (void) ajStrAppC(&line1, "*");
        (void) ajStrAppC(&line2, "*");
        (void) ajStrAppC(&line3, "*");
      } else if (!isalpha((int)*p)) {
        (void) ajStrAppC(&line1, "?");
        (void) ajStrAppC(&line2, "?");
        (void) ajStrAppC(&line3, "?");
      } else {
        p3 = embPropCharToThree(*p);
        (void) ajStrAppK(&line1, *p3);
        (void) ajStrAppK(&line2, *(p3+1));
        (void) ajStrAppK(&line3, *(p3+2));
      }
    }

    (void) ajListstrPushApp(lines, line1);
    (void) ajStrDel(&line);	/* tidy up */

  } else {

/* nucleic or single-letter code */  	
/* do uppercase ranges */
    if (ajRangeOverlaps(info->upperrange, pos, thys->width)) {
      (void) showUpperRange(&line, info->upperrange, pos);
    }
/* do colour ranges if we are displaying HTML*/
    if (thys->html && ajRangeOverlaps(info->highlight, pos, thys->width)) {
      (void) showColourRange(&line, info->highlight, pos);
    }
    (void) ajListstrPushApp(lines, line);
  }

/* optional number at right */
  if (info->number) {

/* if the sequence has ended we might have to fill out the end with blanks */
    if (pos+thys->width > ajSeqLen(thys->seq)) {
      (void) showPad(lines, thys->width - ajSeqLen(thys->seq) + pos);
      (void) ajListstrPushApp(lines,
			      ajFmtStr(" %d",
				       ajSeqLen(thys->seq)+thys->offset-1));
    } else {
      (void) ajListstrPushApp(lines,
			      ajFmtStr(" %d",
				       pos+thys->width+thys->offset-1));
    }
  }

/* end the output line */
  (void) ajListstrPushApp(lines, ajFmtStr("\n"));


/* if we are doing three-letter code, add the other 3-letter output lines */
  if (!thys->nucleic && info->threeletter) {
    (void) showMargin(thys, lines);
    (void) ajListstrPushApp(lines, line2);
    (void) ajListstrPushApp(lines, ajFmtStr("\n"));
    (void) showMargin(thys, lines);
    (void) ajListstrPushApp(lines, line3);
    (void) ajListstrPushApp(lines, ajFmtStr("\n"));
  }

}


/* @funcstatic showFillBlank **************************************************
**
** Add a blank line to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowBlank] data on how to display the sequence data
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillBlank(EmbPShow thys, AjPList lines, EmbPShowBlank info,
			  int pos) {
	
/* At last - an easy one! :-) */

  AjPStr line=ajStrNewL(2);
  
(void) ajDebug("showFillBlank\n");

  (void) ajStrAssC(&line, "\n");
  (void) ajListstrPushApp(lines, line);

}


/* @funcstatic showFillTicks **************************************************
**
** Add a tick line to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowTicks] data on how to display the sequence data
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillTicks(EmbPShow thys, AjPList lines, EmbPShowTicks info,
			  int pos) {
	
  AjPStr line=ajStrNewL(81);
  int i;
  int offset = thys->offset;
  int width = thys->width;

(void) ajDebug("showFillTicks\n");

/* make the ticks line */
  for (i=pos+offset; i<pos+offset+width; i++) {
    if (!(i % 10)) {
      (void) ajStrAppC(&line, "|");
    } else if (!(i % 5)) {
      (void) ajStrAppC(&line, ":");
    } else {
      (void) ajStrAppC(&line, "-");
    }
  }
  (void) showMargin(thys, lines);
  (void) ajListstrPushApp(lines, line);

/* end the output ticks line */
  (void) ajListstrPushApp(lines, ajFmtStr("\n"));
}

/* @funcstatic showFillTicknum ************************************************
**
** Add a tick line numbers to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowTicknum] data on how to display the sequence data
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillTicknum(EmbPShow thys, AjPList lines, EmbPShowTicknum info,
			    int pos) {
	
  AjPStr line=ajStrNewL(81);
  int i;
  int offset = thys->offset;
  int width = thys->width;
  int pad;

(void) ajDebug("showFillTicknum\n");

  (void) showMargin(thys, lines);
  pad = 9 - ((pos+offset-1) % 10);
  if (pad) {
    (void) showPad(lines, pad);
  }
  
  for (i=pos + offset + pad; 
   i < pos+offset+width; i+=10) {
    (void) ajFmtPrintAppS(&line, "%-10d", i);
  }

  (void) ajListstrPushApp(lines, line);

/* end the output line */
  (void) ajListstrPushApp(lines, ajFmtStr("\n"));

}

/* @funcstatic showFillComp ***************************************************
**
** Add thys line's worth of sequence complement to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowComp] data on how to display the sequence data
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillComp(EmbPShow thys, AjPList lines, EmbPShowComp info,
			 int pos) {
	
  AjPStr line=ajStrNewL(81);

(void) ajDebug("showFillComp\n");

/* do a quick check that we have a nucleic sequence - else just ignore this */
  if (!thys->nucleic) {
    return;
  }

/* variable width margin at left with optional number in it */
  if (info->number) {
    (void) showMarginNumber(thys, lines, pos+thys->offset);
  } else {
    (void) showMargin(thys, lines);
  }

/* get the sequence at this position */ 
  (void) ajStrAppSub(&line, ajSeqStr(thys->seq), pos, pos+thys->width-1);
/* get the complement */
  (void) ajSeqCompOnlyStr(&line);
  (void) ajListstrPushApp(lines, line);

/* optional number at right */
  if (info->number) {
/* if the sequence has ended we might have to fill out the end with blanks */
    if (pos+thys->width > ajSeqLen(thys->seq)) {
      (void) showPad(lines, thys->width - ajSeqLen(thys->seq) + pos);
      (void) ajListstrPushApp(lines,
			      ajFmtStr(" %d",
				       ajSeqLen(thys->seq)+thys->offset-1));
    } else {
      (void) ajListstrPushApp(lines,
			      ajFmtStr(" %d",
				       pos+thys->width+thys->offset-1));
    }
  }


/* end the output line */
  (void) ajListstrPushApp(lines, ajFmtStr("\n"));


}

/* @funcstatic showFillTran ***************************************************
**
** Add this line's worth of sequence translation to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowTran] data on how to display the sequence data
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillTran(EmbPShow thys, AjPList lines, EmbPShowTran info,
			 int pos) {
	
  AjPStr line=ajStrNewL(81);
  AjPSeq tran=NULL;
  AjPStr seqstr=NULL;	/* local copy of seq string for translating ranges */
  AjPSeq seq=NULL;	/* local copy of sequence for translating ranges */
  AjPStr temp=NULL;
  int frame;
  int framepad=0;	/* number of spaces to pad to the correct frame pos */
  int linepos;
  int startpos;		/* number at start of line */
  int endpos;		/* number at end of line */
  int shift=0;		/* length of sequence mod 3 */
  int i, j;
  int last;

(void) ajDebug("showFillTran\n");

/* do a quick check that we have a nucleic sequence - else just ignore this */
  if (!thys->nucleic) {
    return;
  }

/* if the translation has not yet been done, do it now - once only */
  if (!info->transeq) {
/* translate a set of ranges ... */
    if (info->regions && ajRangeNumber(info->regions)) {
      framepad = 0;
      seqstr = ajSeqStrCopy(thys->seq);
      temp = ajStrNew();
      (void) ajRangeStrExtract (&temp, info->regions, seqstr);
      (void) ajStrDel (&seqstr);
      seq = ajSeqNew();
      (void) ajSeqReplace(seq, temp);
      (void) ajStrClear(&temp);
      tran = ajTrnSeqOrig (info->trnTable, seq, 1);
      (void) ajSeqDel (&seq);
/* expand to fill line or change to three-letter code */
      if (info->threeletter) {
        (void) ajSeqReplace (tran, embPropProt1to3 (tran, framepad));
      } else {
/* pad with 2 spaces after every residue */
        (void) ajSeqReplace (tran, embPropProtGaps (tran, framepad));
      }
/* now put in spaces to align the translation to the sequence ranges */
      (void) ajRangeStrStuff (&temp, info->regions, ajSeqStr(tran));
      (void) ajSeqReplace(tran, temp);
      (void) ajStrClear(&temp);
    } else {

/* ... or just translate in the required frame */

/* change frames -1 to -3 to frames 6 to 4 for translation of complement */
      frame = info->frame;
      if (frame < 0) {
        frame = 7 + frame; 
      }

      /* if we are doing the reverse complement then the frame is */
      /* dependent on the length of the sequence%3 */
      if (info->frame < 0) { 
	/* get the length mod 3 - used for adjusting the frame of */
	/* reverse sense translations */
        shift = ajSeqLen(thys->seq)%3;
        frame += shift;
        if (frame > 6) frame -= 3;
      }
      
     
/* do the translation */    
      tran = ajTrnSeqOrig (info->trnTable, thys->seq, frame);

/* shift the translation to the correct frame if it is not in frame 1 or 4 */
      if (frame == 1 || frame == 6) {
        framepad = 0+shift;
      } else if (frame == 2 || frame == 5) {
        framepad = 1+shift;
      } else if (frame == 3 || frame == 4) {
        framepad = 2+shift;
      }
      if (framepad > 2) framepad -= 3;

/* convert inter-ORF regions to '.'s */
      last = -1;
      for (i=0; i<ajSeqLen(tran); i++) {
      	if (ajStrStr(ajSeqStr(tran))[i] == '*') {
      	  if (i-last < info->orfminsize+1) {
      	    for (j=last+1; j<i; j++) {
      	      ajStrStr(ajSeqStr(tran))[j] = '-';
      	    }
      	  }
      	  last = i;
      	}
      }

/* expand to fill line or change to three-letter code */
      if (info->threeletter) {
        (void) ajSeqReplace (tran, embPropProt1to3 (tran, framepad));
      } else {
/* pad with 2 spaces after every residue */
        (void) ajSeqReplace (tran, embPropProtGaps (tran, framepad));
      }
    }

/* store the resulting translation in our descriptor structure */
    info->transeq = tran;
  }

/*** OK - we have done the whole translation. Now get bit for this line ***/

/* get the sequence at this position */ 
  (void) ajStrAppSub(&line, ajSeqStr(info->transeq), pos, pos+thys->width-1);

/* get the number of the starting and ending amino-acid on this line */
    startpos = info->tranpos;
    endpos = info->tranpos;
    for (linepos=0; linepos<ajStrLen(line); linepos++) {
/* only count the starting letter of 3-letter codes and don't count *'s */
      if (ajStrStr(line)[linepos] >= 'A' && ajStrStr(line)[linepos] <= 'Z') {
        info->tranpos++;
      }
    }

/* do we have less than width in the line? Add blanks to pad it out */
    for (;linepos < thys->width; linepos++) {
      ajStrAppC(&line, " ");
    }

/* if we have at least one residue, count it at start */
    if (info->tranpos != endpos) {
      startpos++;
    }


/* variable width margin at left with optional number in it */
  if (info->number) {
    (void) showMarginNumber(thys, lines, startpos);
  } else {
    (void) showMargin(thys, lines);
  }

/* put the translation line on the output list */
  (void) ajListstrPushApp(lines, line);

/* optional number at right */
  if (info->number) {
    (void) ajListstrPushApp(lines, ajFmtStr(" %d", info->tranpos));
  }

/* end the output line */
  (void) ajListstrPushApp(lines, ajFmtStr("\n"));

}


/* @funcstatic showFillRE ***************************************************
**
** Add this line's worth of Restriction Enzyme cut sites to the lines list
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowRE] data on how to display the RE cut sites
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillRE(EmbPShow thys, AjPList lines,
			 EmbPShowRE info, int pos) {
	
  (void) ajDebug("showFillRE\n");

/* do a quick check that we have a nucleic sequence - else just ignore this */
  if (!thys->nucleic) {
    return;
  }

  if (info->flat) {
    showFillREflat(thys, lines, info, pos);
  } else {
    showFillREupright(thys, lines, info, pos);
  }
}

/* @funcstatic showFillREupright ********************************************
**
** Add this line's worth of Restriction Enzyme cut sites to the lines list
** Display in upright sit-up-and-beg format
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowRE] data on how to display the RE cut sites
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillREupright(EmbPShow thys, AjPList lines,
				EmbPShowRE info, int pos) {
	


  AjPStr line=NULL;
  AjPStr newline=NULL;
  AjPStr baseline=ajStrNew();		/* line holding first set of ticks */
  AjPList linelist = ajListstrNew();	/* list of lines to fill */
  int cut;				/* the sites to display */
  AjIList liter;			/* iterator for linelist */
  AjBool freespace;			/* flag: found free space to print */ 
  EmbPMatMatch m=NULL;			/* restriction enz match structure */
  AjIList miter;			/* iterator for matches list */
  EmbPShowREsite sitenode;		/* site node structure */
  AjIList siter;			/* iterator for sites list */
  EmbPShowREsite s=NULL;			/* site node structure */
  AjPStr tick=NULL;			/* tick "|" string */
  int ln;

/* if we have not yet produced a sorted list of cut sites, do it now */
  if (info->sitelist == NULL) {
    info->sitelist = ajListNew();

    miter = ajListIter(info->matches);
    while ((m = ajListIterNext(miter)) != NULL) {

/* store the first cut site in this sense */
      if (info->sense == 1) {	/* forward sense */
        cut = m->cut1;
      } else {			/* reverse sense */
        cut = m->cut2;
      }
      cut--;

      AJNEW0(sitenode);
      sitenode->pos = cut;
      sitenode->name = m->cod;
      (void) ajListPushApp(info->sitelist, sitenode);

/* now store the potential second cut site on this sense */
      if (info->sense == 1) {	/* forward sense */
        cut = m->cut3;
      } else {			/* reverse sense */
        cut = m->cut4;
      }
      if (cut) {
        cut--;
        AJNEW0(sitenode);
        sitenode->pos = cut;
        sitenode->name = m->cod;
        (void) ajListPushApp(info->sitelist, sitenode);
      }
    }

    (void) ajListIterFree(miter);
    ajListSort (info->sitelist, showFillREuprightSort);
  }

  ajStrAssC(&tick, "|");	/* a useful string */

/* iterate through the site list */
  siter = ajListIter(info->sitelist);
  while ((s = ajListIterNext(siter)) != NULL) {
  	        
    cut = s->pos;

/* ignore this match if nothing is to be displayed on this line */
    if (cut >= pos && cut <= pos+thys->width-1) {

/* convert to position in the line */
      cut = cut-pos;

/* put tick in base line */
      showOverPrint(&baseline, cut, tick);

/* work up list of lines */
      freespace = ajFalse;

/* we will be potentially updating the nodes of linelist, so don't just
iterate, use ajListstrPop() and ajListstrPushApp() to pop off the bottom
and then push the altered node back on the top of the list */
      for (ln = ajListstrLength(linelist); ln>0; ln--) {
        (void) ajListstrPop(linelist, &line);
	/* if we have not yet written the name in this set of iterations, */
	/*see if we can do so now */
        if (!freespace) {
/* if name space is clear, write name and break */
          if (showLineIsClear(&line, cut, cut+ajStrLen(s->name))) {
            showOverPrint(&line, cut, s->name);
            freespace = ajTrue;	/* flag to show we have written name */
          } else {
        	
/* if cut site position character is space, change it to a tick */
            if (*(ajStrStr(line)+cut) == ' ') {
              showOverPrint(&line, cut, tick);
	    }
          }
        }

        (void) ajListstrPushApp(linelist, line);
/* end 'iteration' through lines */
      }


/* if name was not written, append a new line and write name */
      if (!freespace) {
        newline=ajStrNew();
        showOverPrint(&newline, cut, s->name);
        (void) ajListstrPushApp(linelist, newline);
      }
    }
  }	/* end iteration through the sites list */
  (void) ajListIterFree(siter);


/* convert base line ticks to forward or reverse slashes */
  if (info->sense == 1) {	/* forward sense */
    ajStrConvertCC (&baseline, "|", "\\");
  } else {			/* reverse sense */
    ajStrConvertCC (&baseline, "|", "/");
  }

/* put base line at start of lines list */
  (void) ajListstrPush(linelist, baseline);

/* reverse the order of the lines if we are in the forward sense as 
then we get the ticks pointing downwards :-) */
  if (info->sense == 1) ajListstrReverse(linelist);

/* iterate through the lines and print them */
  liter = ajListIter(linelist);
  while ((line = ajListIterNext(liter)) != NULL) {

/* output to the lines list */
    (void) showMargin(thys, lines);	/* variable width margin at left */
				        /* with optional number in it */
    (void) ajListstrPushApp(lines, line);	/* put the translation line */
				                /* on the output list */
    (void) ajListstrPushApp(lines, ajFmtStr("\n"));  /* end the output line */
  }
  (void) ajListIterFree(liter);

/* tidy up */
  (void) ajListstrDel(&linelist); /* do not use ajListstrFree() here! */
  ajStrDel(&tick);
}

/* @funcstatic showFillREuprightSort ****************************************
**
** Sort routine for showFillREupright - produces reverse cut site order
**
** @param [r] a [const void*] First pos
** @param [r] b [const void*] Second pos
** @return [int] Comparison value. 0 if equal, -1 if first is lower,
**               +1 if first is higher.
** @@
******************************************************************************/
static int showFillREuprightSort (const void* a, const void* b) {

  int res;

  res = (*(EmbPShowREsite *)b)->pos - (*(EmbPShowREsite *)a)->pos;

/* if the cut sites are equal, reverse sort by the length of the name */
  if (!res) {
    res = ajStrLen((*(EmbPShowREsite *)b)->name) -
          ajStrLen((*(EmbPShowREsite *)a)->name);
  }
  
  return res;
}

/* @funcstatic showFillREflat ***********************************************
**
** Add this line's worth of Restriction Enzyme cut sites to the lines list
** Display in flat format with recognition sites
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowRE] data on how to display the RE cut sites
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillREflat(EmbPShow thys, AjPList lines, EmbPShowRE info,
			   int pos) {
		


  AjPStr line=NULL;
  AjPStr line2=NULL;
  AjPList linelist = ajListstrNew();	/* list of lines to fill */
  int start, end;			/* start and end position of site */
  int nameend;				/* end position of name */
  int base;				/* base position of binding site */
  int cut1, cut2, cut3, cut4;		/* cut sites */
  AjIList liter;			/* iterator for linelist */
  AjPStr namestr=NULL;			/* name of RE to insert into line */
  AjPStr sitestr=NULL;			/* binding and cut site to insert */
  int i, j;
  char *claimchar = "*";		/* char used to stake a claim to */
				        /* that position in the string */
  AjBool freespace;			/* flag for found a free space to */
				        /* print in */ 
  EmbPMatMatch m=NULL;			/* restriction enzyme match struct */
  AjIList miter;			/* iterator for matches list */
  int ln;

/* debug */
/*ajDebug("No. of hits in matches list =%d", info->hits);*/

/* iterate through the list */
  miter = ajListIter(info->matches);
  while ((m = ajListIterNext(miter)) != NULL) {
  	        
/* debug stuff */
/*ajDebug ("base=%d\tenz=%S\tpat=%S\tcut1=%d\tcut2=%d\tcut3=%d\tcut4=%d",
	 m->start, m->cod, m->pat, m->cut1, m->cut2, m->cut3, m->cut4 );*/

/* get the start and end positions */
    cut1 = m->cut1;
    cut2 = m->cut2+1;		/* the display points back '<' at cut pos */
    cut3 = m->cut3;
    cut4 = m->cut4;
    if (m->cut4) cut4++;	/* the display points back '<' at cut pos */
    base = m->start;
    start = base;
    if (info->sense == 1) {	/* forward sense */
      if (cut1 < start) start = cut1;
      if (cut3 && cut3 < start) start = cut3;
    } else {			/* reverse sense */
      if (cut2 < start) start = cut2;
      if (cut4 && cut4 < start) start = cut4;  	
    }
  
    end = base + ajStrLen(m->pat)-1;
    nameend = base + ajStrLen(m->cod)-1;

    if (info->sense == 1) {	/* forward sense */
      if (cut1 > end) end = cut1;
      if (cut3 && cut3 > end) end = cut3;
    } else {			/* reverse sense */
      if (cut2 > end) end = cut2;
      if (cut4 && cut4 > end) end = cut4;
    }

/* convert human-readable sequence positions to string positions */
    start--;
    end--;
    base--;
    nameend--;

/* ignore this match if nothing is to be displayed on this line */
    if (start <= pos+thys->width-1 && end >= pos) {

/*ajDebug ("base=%d\tenz=%S\tpat=%S\tcut1=%d\tcut2=%d\tcut3=%d\tcut4=%d",
	 m->start, m->cod, m->pat, m->cut1, m->cut2, m->cut3, m->cut4 );*/

/* make a standard name and site string to be chopped up later */

/* site string stuff */
/* initial string of '.'s */
      sitestr = ajStrNew();
      (void) ajStrAppKI(&sitestr, '.', end-start+1 );
/* add on any claim characters required to stake a claim to positions 
used by the name */
      if (nameend > end) (void) ajStrAppKI(&sitestr, *claimchar, nameend-end);

/* debug - sanity check to see if overwriting NULLs */
/*
for (i=base-start; i<base-start+ajStrLen(m->pat); i++)
  if (*(ajStrStr(sitestr)+i) == '\0') ajDebug("AWOOOGA! Alert!");
*/

/* cover binding site with '='s */
      for (i=base-start; i<base-start+ajStrLen(m->pat); i++)
	*(ajStrStr(sitestr)+i) = '='; 

/* I tried showing the pattern instead of '='s, but it looks awful - GWW 12 Jan 2000 */
/*      for (j=0, i=base-start; i<base-start+ajStrLen(m->pat); j++, i++)
          *(ajStrStr(sitestr)+i) = ajStrStr(m->pat)[j]; */

/* put in cut sites */
      if (info->sense == 1) {	/* forward sense */
        *(ajStrStr(sitestr)+cut1-start-1) = '>';
/* debug - sanity check to see if overwriting NULLs */
/*if (cut3) if (*(ajStrStr(sitestr)+cut3-start-1) == '\0') ajDebug("AWOOOGA!");*/
        if (cut3) *(ajStrStr(sitestr)+cut3-start-1) = '>';
      } else {		/* reverse sense */
        *(ajStrStr(sitestr)+cut2-start-1) = '<';
/* debug - sanity check to see if overwriting NULLs */
/*if (cut4) if (*(ajStrStr(sitestr)+cut4-start-1) == '\0') ajDebug("AWOOOGA!");*/
        if (cut4) *(ajStrStr(sitestr)+cut4-start-1) = '<';
      }


/* name string stuff */
/* initial string of claimchar's */
      namestr = ajStrNew();
      (void) ajStrAppKI(&namestr, *claimchar, end-start+1 );
      if (nameend > end) (void) ajStrAppKI(&namestr, *claimchar, nameend-end);

/* debug - sanity check to see if overwriting NULLs */
/*
for (j=0, i=base-start; i<base-start+ajStrLen(m->cod); j++, i++)
  if (*(ajStrStr(namestr)+i) == '\0' ||  *(ajStrStr(m->cod)+j) == '\0')
    ajDebug("AWOOOGA! AWOOGA! Alert Alert Alert!");
*/

/* insert the name in the namestr */
      for (j=0, i=base-start; i<base-start+ajStrLen(m->cod); j++, i++)
	*(ajStrStr(namestr)+i) = *(ajStrStr(m->cod)+j);


/* now chop up the name and site strings to fit in the line */

/* is the feature completely within the line */
      if (start >= pos && end <= pos+thys->width-1) {
/*ajDebug("*** Completely in line: name=%S start=%d pos=%d, end=%d, lineend=%d",
	m->cod, start, pos, end, pos+thys->width-1);*/
/* add on an extra pair of claim characters to make a space between */
/* adjacent matches */
        ajStrAppC(&sitestr, claimchar);
        ajStrAppC(&sitestr, claimchar);

      } else if (start < pos && end <= pos+thys->width-1) {
/*ajDebug("*** starts before the line");*/

/* starts before the line */
/* cut off the start */
        (void) ajStrSub(&sitestr, pos-start, ajStrLen(sitestr)-1);

/* add on an extra couple of claim chars to make a space between */
/* adjacent matches */
        ajStrAppC(&sitestr, claimchar);
        ajStrAppC(&sitestr, claimchar);

/* if the base position is not displayed, move the name to the start */
        if (base < pos) {
          ajStrAss(&namestr, m->cod);
          ajStrAppC(&namestr, claimchar);
          ajStrAppC(&namestr, claimchar);

/* add claim characters to end of namestring if sitestring is longer,
   and vice versa */
          if (ajStrLen(namestr) < ajStrLen(sitestr))
	    (void) ajStrAppKI(&namestr, *claimchar,
			      ajStrLen(sitestr)-ajStrLen(namestr));
          if (ajStrLen(namestr) > ajStrLen(sitestr))
	    (void) ajStrAppKI(&sitestr, *claimchar,
			      ajStrLen(namestr)-ajStrLen(sitestr));
        } else {
/* cut off the start of the name string to make it line up with the sitestr */
          (void) ajStrSub(&namestr, pos-start, ajStrLen(namestr)-1);
        }
/* make it display from the start of the line */
        start = pos; 


      } else if (start >= pos && end > pos+thys->width-1) {
/*ajDebug("*** ends after the line");*/
/* ends after the line */
/* cut off the end */
        (void) ajStrSub(&sitestr, 0, pos+thys->width-start-1);
/* if the base position is not displayed, move the name to the start */
        if (base > pos+thys->width-1) {
          ajStrAss(&namestr, m->cod);
          ajStrAppC(&namestr, claimchar);
          ajStrAppC(&namestr, claimchar);

/* add claim characters to end of namestring if sitestring is longer, 
   and vice versa */
          if (ajStrLen(namestr) < ajStrLen(sitestr))
	    (void) ajStrAppKI(&namestr, *claimchar,
			      ajStrLen(sitestr)-ajStrLen(namestr));
          if (ajStrLen(namestr) > ajStrLen(sitestr))
	    (void) ajStrAppKI(&sitestr, *claimchar,
			      ajStrLen(namestr)-ajStrLen(sitestr));
        }
/* make it display to the end of the line */
        end = pos+thys->width-1;


      } else if (start < pos && end > pos+thys->width-1) {
/*ajDebug("*** completely overlaps the line");*/
/* completely overlaps the line! */
/* cut off the start and end */
        (void) ajStrSub(&sitestr, pos-start, pos+thys->width-start-1);
/* if the base position is not displayed, move the name to the start */
        if (base < pos) {
          ajStrAss(&namestr, m->cod);
          ajStrAppC(&namestr, claimchar);
          ajStrAppC(&namestr, claimchar);

/* add claim characters to end of namestring if sitestring is longer, */
/* and vice versa */
          if (ajStrLen(namestr) < ajStrLen(sitestr))
	    (void) ajStrAppKI(&namestr, *claimchar,
			      ajStrLen(sitestr)-ajStrLen(namestr));
          if (ajStrLen(namestr) > ajStrLen(sitestr))
	    (void) ajStrAppKI(&sitestr, *claimchar,
			      ajStrLen(namestr)-ajStrLen(sitestr));
        } else {
/* cut off the start of the name string to make it line up with the sitestr */
          (void) ajStrSub(&namestr, pos-start,  /* ...or should this be , */
			  ajStrLen(namestr)-1); /* pos+thys->width-start-1); */
        }
/* make it display from the start of the line */
        start = pos;
        end = pos+thys->width-1;

      } 

/* debug */
else {
ajDebug("Shouldn't get to here!");
continue;
      }
  
/* work up list of lines */
      freespace = ajFalse;

/* we will be potentially updating the nodes of linelist, so don't just
iterate, use ajListstrPop() and ajListstrPushApp() to pop off the bottom
and then push the altered node back on the top of the list */
      for (ln = ajListstrLength(linelist); ln>0; ln--) {
        (void) ajListstrPop(linelist, &line);	/* get the site line */
        (void) ajListstrPop(linelist, &line2);	/* get the name line */
/* if we have not yet written the name in this set of iterations, */
/* see if we can do so now */
        if (!freespace) {
/* if name space is clear, write name and site */
/*ajDebug("Calling showLineIsClear() for region %d to %d", start-pos, end-pos);*/
          if (showLineIsClear(&line, start-pos, end-pos)) {
/*ajDebug("over printing clear region start=%d-pos=%d", start-pos, end-pos);*/
            showOverPrint(&line, start-pos, sitestr);
/*ajDebug("over printing clear region start=%d-pos=%d", start-pos, end-pos);*/
            showOverPrint(&line2, start-pos, namestr);
            freespace = ajTrue;	/* flag to show we have written name */
          }
        }

        (void) ajListstrPushApp(linelist, line);
        (void) ajListstrPushApp(linelist, line2);
/* end 'iteration' through lines */
      }

/* if we didn't find a clear region to print in, append two new strings and
print in them */
      if (!freespace) {
/*ajDebug("Create a new list line. No=%d+2", ajListstrLength(linelist));*/
        line=ajStrNew();
        (void) ajStrAppKI(&line, ' ', thys->width); /* fill with spaces */
        line2=ajStrNew();
        (void) ajStrAppKI(&line2, ' ', thys->width); /* fill with spaces */
/*ajDebug("start=%d, pos=%d", start-pos, end-pos);*/
        showOverPrint(&line, start-pos, sitestr);
/*ajDebug("start=%d, pos=%d", start-start, end-pos);*/
        showOverPrint(&line2, start-pos, namestr);
        (void) ajListstrPushApp(linelist, line);
        (void) ajListstrPushApp(linelist, line2);
      }
  
/* delete the name and site strings */
      ajStrDel(&namestr);
      ajStrDel(&sitestr);

/* end of if-block for match in this line */
    }

/* end iteration through the info->matches array */
  }
  (void) ajListIterFree(miter);


/* reverse the order of the lines if we are in the forward sense as then
we get the most densely populated lines at the bottom closest to the
sequence (and we get the names above the cut-sites) */
  if (info->sense == 1) ajListstrReverse(linelist);

/* iterate through the lines and print them */
  liter = ajListIter(linelist);
  while ((line = ajListIterNext(liter)) != NULL) {

/* convert claim characters in the line to spaces as these were used to
stake a claim to the space */
    ajStrConvertCC (&line, claimchar, " ");

/* remove trailing spaces - these can be very long in namestr when the
cut and recognition sites are widely separated and so many claimchars
have been appended */
    for (i=ajStrLen(line)-1; i>=0; i--) {
      if (*(ajStrStr(line)+i) != ' ') {
        break;
      }
    }
    ajStrTruncate(&line, i+1);

/* output to the lines list */
    (void) showMargin(thys, lines);	/* variable width margin at left */
				        /* with optional number in it */
    (void) ajListstrPushApp(lines, line);	/* put the translation line */
				                /* on the output list */
    (void) ajListstrPushApp(lines, ajFmtStr("\n"));	/* end output line */
  }
  (void) ajListIterFree(liter);

/* tidy up */
  (void) ajListstrDel(&linelist); /* do not use ajListstrFree() here! */

}

/* @funcstatic showFillFT ***************************************************
**
** Add this line's worth of features to the lines list
** NB. the 'source' feature is always ignored
**
** @param [P] thys [EmbPShow] Show sequence object
** @param [u] lines [AjPList] list of lines to add to
** @param [r] info [EmbPShowFT] data on how to display the features
** @param [r] pos [int] current printing position in the sequence
** @return [void]
** @@
******************************************************************************/
static void showFillFT(EmbPShow thys, AjPList lines,
		       EmbPShowFT info, int pos) {


  AjIList    iter = NULL;
  AjPFeature gf   = NULL;
  
  AjPStr line=NULL;
  AjPStr line2=NULL;
  AjPList linelist = ajListstrNew();	/* list of lines to fill */
  int start, end;			/* start and end position of linestr */
  int namestart, nameend;		/* start and end position of namestr */
  AjIList liter;			/* iterator for linelist */
  AjPStr namestr=NULL;			/* name of feature to insert into line */
  AjPStr linestr=NULL;			/* line graphics to insert */
  int i, j;
  char *claimchar = "*";		/* char used to stake a claim to */
				        /* that position in the string */
  AjBool freespace;			/* flag for found a free space to */
				        /* print in */ 
  int ln;



  (void) ajDebug("showFillFT\n");

/* if feat is NULL then there are no features associated with this sequence */
  if (!info->feat) return;
      
/*  ajDebug("No. of features=%d", ajFeatTabCount(info->feat));*/

/* reminder of the AjSFeature structure for handy reference
*
*
*  AjEFeatClass      Class ;
*  AjPFeatTable      Owner ;
*  AjPFeatVocFeat     Source ;
*  AjPFeatVocFeat     Type ;
*  int               Start ;
*  int               End; 
*  int               Start2;
*  int               End2;
*  AjPStr            Score ;
*  AjPList           Tags ;     a.k.a. the [group] field tag-values of GFF2 
*  AjPStr            Comment ;
*  AjEFeatStrand     Strand ;
*  AjEFeatFrame      Frame ;
*  AjPStr            desc ;
*  int               Flags;
*
*/


/* iterate through the features */
  if (info->feat->Features) {
    iter = ajListIter(info->feat->Features) ;
    while(ajListIterMore(iter)) {
      gf = ajListIterNext (iter) ;
    
/* don't output the 'source' feature - it is very irritating! */
      if (!ajStrCmpC(gf->Type->name, "source")) continue;

/* check that the feature is within the line to display (NB.  We are
working in human coordinates here: 1 to SeqLength, not 0 to SeqLength-1)
*/
      if (pos+1 > gf->End || pos+thys->width < gf->Start) continue;

/*    
ajDebug("type = %S %d-%d", gf->Type->name, gf->Start, gf->End);   
*/

/* prepare name string */
      namestr = ajStrNew();
      ajStrAss(&namestr,  gf->Type->name);

/* add tags to namestr*/
      showAddTags(&namestr, gf->Tags, ajTrue);
      
/* note the start and end positions of the name and line graphics */
      start = (gf->Start-1<pos)?pos:gf->Start-1;
      end = (gf->End-1>pos+thys->width-1)?pos+thys->width-1:gf->End-1;
      namestart = start;	/* print the name starting with the line */
      nameend =  start + ajStrLen(namestr)-1;

/* shift long namestr back if longer than the line when printed */
      if (nameend > pos+thys->width-1+thys->margin) {
/*ajDebug("name is longer than margin");*/
        if (ajStrLen(namestr) > end-pos+1) {
/*ajDebug("name is longer than the line - shifting to start of line");*/
          namestart = pos;
          nameend = pos + ajStrLen(namestr) -1;
/* it is shifted back to the start of the display line
   is it still longer than the line? truncate it */
          if (nameend > thys->width-1+thys->margin) {
/*ajDebug("...still longer than the line, truncate it to %d chars", thys->width-1+thys->margin);*/
            ajStrTruncate(&namestr, thys->width-1+thys->margin);
            nameend = pos+thys->width-1+thys->margin;
          }
        } else {
/*ajDebug("Not longer than the line now");*/
          namestart = end - ajStrLen(namestr)+1;
          nameend = namestart + ajStrLen(namestr)-1;
	}
      }

/* add on any claim characters required to stake a claim to positions 
used by the line graphics */
      if (end > nameend) {
      	(void) ajStrAppKI(&namestr, *claimchar, end-nameend);
        nameend = end;
      }

/* add on a couple more claim characters to space out the features */
      ajStrAppKI(&namestr, *claimchar, 2);
      nameend += 2;

/* prepare line string */
/* initial string of '='s */
      linestr = ajStrNew();
      (void) ajStrAppKI(&linestr, '=', end-start+1 );

/* put in end position characters */
      if (gf->Start-1>=pos) *(ajStrStr(linestr)) = '|';
      if (gf->End-1<=pos+thys->width-1) *(ajStrStr(linestr)+end-start) = '|';


/*
ajDebug("pos=%d, start=%d, end=%d, namestart=%d, nameend=%d, end-pos=%d", pos, start, end, namestart, nameend, end-pos);
ajDebug("namestr=  %S", namestr);
ajDebug("linestr=  %S", linestr);
*/


/* work up list of lines */
      freespace = ajFalse;

/* iterate through list of existing line to find no overlap with existing lines */
/* we will be potentially updating the nodes of linelist, so don't just
iterate, use ajListstrPop() and ajListstrPushApp() to pop off the bottom
and then push the altered node back on the top of the list */
      for (ln = ajListstrLength(linelist); ln>0; ln--) {
        (void) ajListstrPop(linelist, &line);	/* get the linestr line */
        (void) ajListstrPop(linelist, &line2);	/* get the namestr line */
/* if we have not yet written the name in this set of iterations, */
/* see if we can do so now */
        if (!freespace) {
/* if name space is clear, write namestr and sitestr */
/*ajDebug("Calling showLineIsClear() for region %d to %d", start-pos, end-pos);*/
          if (showLineIsClear(&line2, start-pos, end-pos) &&
              showLineIsClear(&line2, namestart-pos, nameend-pos)) {
/*ajDebug("over printing clear region start=%d-pos=%d", start-pos, end-pos);*/
            showOverPrint(&line, start-pos, linestr);
/*ajDebug("over printing clear region namestart=%d-nameend=%d", namestart-pos, nameend-pos);*/
            showOverPrint(&line2, namestart-pos, namestr);
            freespace = ajTrue;	/* flag to show we have written name */
          }
        }

        (void) ajListstrPushApp(linelist, line);
        (void) ajListstrPushApp(linelist, line2);
/* end 'iteration' through lines */
      }

/* if we didn't find a clear region to print in, append two new strings and
print in them */
      if (!freespace) {
/*ajDebug("Create a new list line. No=%d+2", ajListstrLength(linelist));*/
        line=ajStrNew();
        (void) ajStrAppKI(&line, ' ', thys->width); /* fill with spaces */
        line2=ajStrNew();
        (void) ajStrAppKI(&line2, ' ', thys->width); /* fill with spaces */
/*ajDebug("start=%d, pos=%d", start-pos, end-pos);*/
        showOverPrint(&line, start-pos, linestr);
/*ajDebug("start=%d, pos=%d", start-start, end-pos);*/
        showOverPrint(&line2, namestart-pos, namestr);
        (void) ajListstrPushApp(linelist, line);
        (void) ajListstrPushApp(linelist, line2);
      }
  
/* if no suitable line found, create a new pair of lines */

/* print name and line information into the correct pair of lines */

/* tidy up */
      ajStrDel(&namestr);
      ajStrDel(&linestr);


    }
    ajListIterFree(iter) ;

  }

/* iterate through the lines and print them */
  liter = ajListIter(linelist);
  while ((line = ajListIterNext(liter)) != NULL) {

/* convert claim characters in the line to spaces as these were used to
stake a claim to the space */
    ajStrConvertCC (&line, claimchar, " ");

/* remove trailing spaces - these can be very long */
    for (i=ajStrLen(line)-1; i>=0; i--) {
      if (*(ajStrStr(line)+i) != ' ') {
        break;
      }
    }
    ajStrTruncate(&line, i+1);

/* output to the lines list */
    (void) showMargin(thys, lines);	/* variable width margin at left */
				        /* with optional number in it */
    (void) ajListstrPushApp(lines, line);	/* put the translation line */
				                /* on the output list */
    (void) ajListstrPushApp(lines, ajFmtStr("\n"));	/* end output line */
  }
  (void) ajListIterFree(liter);

/* tidy up */
  (void) ajListstrDel(&linelist); /* do not use ajListstrFree() here! */

  
}


/* @funcstatic showOverPrint ************************************************
**
** Overwrite (appending if necessary) a portion of a string with another
**
** @param [u] target [AjPStr *] target string to overwrite
** @param [r] start [int] start position in target to begin overwriting at
** @param [u] insert [AjPStr] string to overwrite with
** @return [void]
** @@
******************************************************************************/
static void showOverPrint(AjPStr *target, int start, AjPStr insert) {

  int i;

/* if start position of insert is less than length of target, pad it out
with space characters to get the required length */
/*
ajDebug("Starting overprint");
ajDebug("In overprint");
ajDebug("Target=>%S<", *target);
ajDebug("Insert=>%S<", insert);
ajDebug("Target Len=%d Insert Len=%d, start pos=%d",
	ajStrLen(*target), ajStrLen(insert), start);
*/
  if (ajStrLen(*target) < start+ajStrLen(insert)) {
/*ajDebug("In overprint, appending %d spaces to target",
	start+ajStrLen(insert) - ajStrLen(*target));*/
    (void) ajStrAppKI(target, ' ',
		      start+ajStrLen(insert) - ajStrLen(*target));
  }

/* debug */
/*
for (i=0; i<ajStrLen(insert); i++) {
if (*(ajStrStr(*target)+i+start) == '\0' || *(ajStrStr(insert)+i) == '\0') 
{ajDebug("ARGGGGGGGGGHHHHHHHHHHHHHHHHHHHHHHHHHH!!!!!!!!!!!!!!!!!!!!!!!!!!!!");}
}
*/

/* overwrite the remaining characters */
  for (i=0; i<ajStrLen(insert); i++) {
/*ajDebug("In overprint, inserting char: %c", *(ajStrStr(insert)+i));*/
    *(ajStrStr(*target)+i+start) = *(ajStrStr(insert)+i);
  }
/*  
ajDebug("result is: >%S<", *target);
ajDebug("exiting showOverPrint()");  
*/
}

/* @funcstatic showLineIsClear **********************************************
**
** Checks that a section of a string is clear to print in (only has blanks in)
**
** @param [u] line [AjPStr *] target string to check
** @param [r] start [int] start position in target to begin checking at
** @param [r] end [int] end position in target to check
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/
static AjBool showLineIsClear(AjPStr *line, int start, int end) {

  int i;
  int len = ajStrLen(*line)-1;
/*
ajDebug("In showLineIsClear(): Looking for clear line at positions:");
ajDebug("target len=%d", len+1);
ajDebug("start=%d, end=%d", start, end);
ajDebug("target=>%S<", *line);
*/
  if (len < end) {
/*ajDebug("In showLineIsClear(): ajStrAppKI of %d chars", end-len);*/
    (void) ajStrAppKI(line, ' ', end-len);
  }
  
  for (i=start; i<=end; i++) {
/*ajDebug("In showLineIsClear(): Line at pos %d is >%c<",
	i, *(ajStrStr(*line)+i));*/
    if (*(ajStrStr(*line)+i) != ' ') return ajFalse;
  }

/*ajDebug("In showLineIsClear(): Line is clear **** :-)");*/
  return ajTrue;

}

/** @funcstatic showAddTags ***********************************************
**
** writes feature tags to the tagsout string
**
** @param [r] tagsout [AjPStr *] tags out string
** @param [r] taglist [AjPList] list of tags
** @param [r] values [AjBool] display values of tags
**
** @return [void] 
** @@
******************************************************************************/

static void showAddTags(AjPStr *tagsout, AjPList taglist, AjBool values) {

  AjIList titer;                /* iterator for taglist */
  LPFeatTagValue tagstr;        /* tag structure */

/* iterate through the tags and test for match to patterns */
/* debug - there is something wrong with the list */

  titer = ajListIter(taglist);
  while (ajListIterMore(titer)) {
    tagstr = (LPFeatTagValue)ajListIterNext(titer);
/* don't display the translation tag - it is far too long :-) */
    if (ajStrCmpC(tagstr->Tag->VocTag->name, "translation")) {
      if (values == ajTrue) {
        (void) ajFmtPrintAppS(tagsout, " %S=\"%S\"", tagstr->Tag->VocTag->name, tagstr->Value);
      } else {
        (void) ajFmtPrintAppS(tagsout, " %S", tagstr->Tag->VocTag->name);
      }
    }
  }
  (void) ajListIterFree(titer);
}


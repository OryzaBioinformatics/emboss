#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embshow_h
#define embshow_h




/********* the EmbPShow object ***********/

typedef struct EmbSShow {
  AjPList list;		/* list of EmbPShowInfo structures */
/* information about the sequence */
  AjPSeq seq;		/* the sequence */
  AjBool nucleic;	/* ajTrue = the sequence is nucleic */
  int offset;  		/* offset to start numbering at */
  int start;		/* sequence position to start printing at */
  int end;		/* sequence position to stop printing at */
/* information about the page layout */
  int width;		/* width of sequence to display on each line */
  int length;		/* length of a page (0 = indefinite) */
  int margin;		/* margin for numbers */
  AjBool html;		/* ajTrue = format page for HTML */
} EmbOShow, *EmbPShow;



/****** struct for node of list of descriptors object pointers ******/

typedef struct EmbSShowInfo {
  int type;  
  void * info;
} EmbOShowInfo, *EmbPShowInfo;




/********* Descriptor object types *****************/

enum ShowEValtype {SH_SEQ, SH_BLANK, SH_TICK, SH_TICKNUM, SH_COMP,
SH_TRAN, SH_RE, SH_FT};



/********* the display option structures ***********/

/* sequence information, type = SH_SEQ */
typedef struct EmbSShowSeq {
  AjBool number;	/* ajTrue = number the sequence */
  AjBool threeletter;	/* ajTrue = display proteins in three letter code */
  AjPRange upperrange;	/* range of sequence to uppercase */
  AjPRange highlight;	/* range of sequence to colour in HTML */
} EmbOShowSeq, *EmbPShowSeq;

/* blank line information, type = SH_BLANK */
typedef struct EmbSShowBlank {
/* AJNEW0() falls over if 0 bytes are allocated, so put in this dummy to
pad the structure out
*/
  AjBool dummy;
} EmbOShowBlank, *EmbPShowBlank;

/* tick line information, type = SH_TICK */
typedef struct EmbSShowTicks {
/* AJNEW0() falls over if 0 bytes are allocated, so put in this dummy to
pad the structure out
*/
  AjBool dummy;
} EmbOShowTicks, *EmbPShowTicks;

/* tick number line information, type = SH_TICKNUM */
typedef struct EmbSShowTicknum {
/* AJNEW0() falls over if 0 bytes are allocated, so put in this dummy to
pad the structure out
*/
  AjBool dummy;
} EmbOShowTicknum, *EmbPShowTicknum;

/* translation information, type = SH_TRAN */
typedef struct EmbSShowTran {
  AjPSeq transeq;	/* our stored translation */
  AjPTrn trnTable;	/* translation table */
  int frame;		/* 1,2,3,-1,-2 or -3 = frame to translate */
  AjBool threeletter;	/* ajTrue = display in three letter code */
  AjBool number;	/* ajTrue = number the translation */
  int tranpos;		/* store of translation position for numbering */
  AjPRange regions;	/* only translate in these regions, NULL = do all */
  int orfminsize;	/* minimum size of ORF to display */
} EmbOShowTran, *EmbPShowTran;

/* sequence complement information, type = SH_COMP */
typedef struct EmbSShowComp {
  AjBool number;	/* ajTrue = number the complement */
} EmbOShowComp, *EmbPShowComp;

/* RE cut site information, type = SH_RE */
typedef struct EmbSShowRE {
  int sense;			/* 1 or -1 = sense to display */
  AjBool flat;			/* ajTrue = display in flat format with recognition sites */
  AjPList matches;		/* list of AjPMatmatch matches */  
  int hits;			/* number of hits in list */
  AjPList sitelist;		/* list of EmbSShowREsite */
} EmbOShowRE, *EmbPShowRE;

/* Feature information, type = SH_FT */
typedef struct EmbSShowFT {
  AjPFeatTable feat;
} EmbOShowFT, *EmbPShowFT;


/********* assorted structures ***********/
/* RE cut site position list node */
typedef struct EmbSShowREsite {
  int pos;		/* cut site position */
  AjPStr name;		/* name of RE */
} EmbOShowREsite, *EmbPShowREsite;


/* declare functions **********************************************/
EmbPShow embShowNew (AjPSeq seq, int begin, int end, int width,
		    int length, int margin, AjBool html, int offset);
void embShowDel (EmbPShow* pthis);

void embShowAddSeq (EmbPShow thys, AjBool number, AjBool threeletter,
		    AjPRange upperrange, AjPRange colour);
void embShowAddBlank (EmbPShow thys);
void embShowAddTicks (EmbPShow thys);
void embShowAddTicknum (EmbPShow thys);
void embShowAddComp (EmbPShow thys, AjBool number);
void embShowAddTran (EmbPShow thys, AjPTrn trnTable, int frame,
		     AjBool threeletter, AjBool number, AjPRange regions,
		     int orfminsize);
void embShowAddRE (EmbPShow thys, int sense, AjPList restrictlist, AjBool flat);
void embShowAddFT (EmbPShow thys, AjPFeatTable feat);
void embShowPrint (AjPFile out, EmbPShow thys);


#endif

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajseqalign_h
#define ajseqalign_h

/* @data AjPAlign *************************************************************
**
** Ajax Align Output object.
**
** Holds definition of feature align output.
**
** @new ajAlignNew Default constructor
** @delete ajAlignDel Default destructor
** @modify ajAlignReset Resets ready for reuse.
** @output ajAlignWrite Master alignment output routine
** @output ajAlignWriteHeader Master header output routine
** @output ajAlignWriteTail Master footer output routine
** @cast ajAlignGetFormat Returns the format of an alignment
**
** @other AjPSeqout Sequence output
** @other AjPFeatout Feature output
** @other AjPReport Report output
** @other AjPFile Input and output files
**
** @attr Type [AjPStr] "P" Protein or "N" Nucleotide
** @attr Formatstr [AjPStr] Report format (-aformat qualifier)
** @attr Format [AjEnum] Report format (index number)
** @attr File [AjPFile] Output file object
** @attr Header [AjPStr] Text to add to header with newlines
** @attr SubHeader [AjPStr] Text to add to align subheader with newlines
** @attr Tail [AjPStr] Text to add to tail with newlines
** @attr SubTail [AjPStr] Text to add to subtail with newlines
** @attr Showacc [AjBool] Report accession number if ajTrue
** @attr Showdes [AjBool] Report sequence description if ajTrue
** @attr Showusa [AjBool] Report full USA (-ausaqualifier) if ajTrue
**                        or only seqname if ajFalse
** @attr Multi [AjBool] if true, assume >1 alignment
** @attr Global [AjBool] if true, show full sequence beyond match
** @attr Data [AjPList] Alignment specific data - see ajalign.c
** @attr Nseqs [ajint] Number of sequences in all alignments
** @attr Nmin [ajint] Minimum number of sequences e.g. 2
** @attr Nmax [ajint] Maximum number of sequences e.g. 2
** @attr Width [ajint] Output width (minimum 10)
** @attr Count [ajint] Use count
** @attr IMatrix [AjPMatrix] Integer matrix (see also FMatrix)
** @attr FMatrix [AjPMatrixf] Floating Pt matrix (see also IMatrix)
** @attr Matrix [AjPStr] Matrix name
** @attr GapPen [AjPStr] Gap penalty (converted to string)
** @attr ExtPen [AjPStr] Gap extend penalty (converted to string)
** @attr SeqOnly [AjBool] Sequence output only, no head or tail
** @attr SeqExternal [AjBool] Sequence is non-local, do not delete
** @@
******************************************************************************/

typedef struct AjSAlign {
  AjPStr Type;
  AjPStr Formatstr;
  AjEnum Format;
  AjPFile File;
  AjPStr Header;
  AjPStr SubHeader;
  AjPStr Tail;
  AjPStr SubTail;
  AjBool Showacc;
  AjBool Showdes;
  AjBool Showusa;
  AjBool Multi;
  AjBool Global;
  AjPList Data;
  ajint Nseqs;
  ajint Nmin;
  ajint Nmax;
  ajint Width;
  ajint Count;
  AjPMatrix  IMatrix;
  AjPMatrixf FMatrix;
  AjPStr Matrix;
  AjPStr GapPen;
  AjPStr ExtPen;
  AjBool SeqOnly;
  AjBool SeqExternal;
} AjOAlign;

#define AjPAlign AjOAlign*




/*
** Prototype definitions
*/

void         ajAlignClose (AjPAlign thys);
AjBool       ajAlignConsStats(const AjPSeqset thys, AjPMatrix mymatrix,
			      AjPStr *cons, ajint* retident, 
			      ajint* retsim, ajint* retgap,
			      ajint* retlen);
AjBool       ajAlignDefine (AjPAlign pthys, AjPSeqset seqset);
AjBool       ajAlignDefineSS (AjPAlign pthys,
			      AjPSeq seqa, AjPSeq seqb);
AjBool       ajAlignDefineCC (AjPAlign pthys,
			      const char* seqa, const char* seqb,
			      const char* namea,const  char* nameb);
void         ajAlignDel (AjPAlign* pthys);
void         ajAlignExit(void);
AjBool       ajAlignFindFormat (const AjPStr format, ajint* iformat);
AjBool       ajAlignFormatDefault (AjPStr* pformat);
const char*  ajAlignGetFilename(const AjPAlign thys);
const AjPStr ajAlignGetFormat(const AjPAlign thys);
AjPAlign     ajAlignNew (void);
AjBool       ajAlignOpen (AjPAlign thys, const AjPStr name);
void         ajAlignPrintFormat (AjPFile outf, AjBool full);
void         ajAlignReset (AjPAlign thys);
void         ajAlignSetExternal (AjPAlign thys, AjBool external);
void         ajAlignSetHeader (AjPAlign thys, const AjPStr header);
void         ajAlignSetHeaderApp (AjPAlign thys, const AjPStr header);
void         ajAlignSetHeaderC (AjPAlign thys, const char* header);
void         ajAlignSetGapI (AjPAlign thys, ajint gappen, ajint extpen);
void         ajAlignSetGapR (AjPAlign thys, float gappen, float extpen);
void         ajAlignSetMatrixName (AjPAlign thys, const AjPStr matrix);
void         ajAlignSetMatrixNameC (AjPAlign thys, const char* matrix);
void         ajAlignSetMatrixInt (AjPAlign thys, const AjPMatrix matrix);
void         ajAlignSetMatrixFloat (AjPAlign thys, const AjPMatrixf matrix);
AjBool       ajAlignSetRange (AjPAlign thys,
			      ajint start1, ajint end1,
			      ajint len1, ajint off1,
			      ajint start2, ajint end2,
			      ajint len2, ajint off2);
void         ajAlignSetScoreI (AjPAlign thys, ajint score);
void         ajAlignSetScoreL (AjPAlign thys, ajlong score);
void         ajAlignSetScoreR (AjPAlign thys, float score);
void         ajAlignSetStats (AjPAlign thys, ajint iali, ajint len,
				    ajint ident, ajint sim, ajint gaps,
				    const AjPStr score);
AjBool       ajAlignSetSubRange (AjPAlign thys,
				 ajint substart1, ajint start1,
				 ajint end1, AjBool rev1, ajint len1,
				 ajint substart2, ajint start2,
				 ajint end2, AjBool rev2, ajint len2);
void         ajAlignSetSubHeader (AjPAlign thys, const AjPStr subheader);
void         ajAlignSetSubHeaderApp (AjPAlign thys, const AjPStr subheader);
void         ajAlignSetSubHeaderC (AjPAlign thys, const char* subheader);
void         ajAlignSetSubHeaderPre (AjPAlign thys, const AjPStr subheader);
void         ajAlignSetSubStandard (AjPAlign thys, ajint iali);
void         ajAlignSetSubTail(AjPAlign thys, const AjPStr tail);
void         ajAlignSetSubTailC(AjPAlign thys, const char* tail);
void         ajAlignSetSubTailApp(AjPAlign thys, const AjPStr tail);
void         ajAlignSetTail (AjPAlign thys, const AjPStr tail);
void         ajAlignSetTailApp (AjPAlign thys, const AjPStr tail);
void         ajAlignSetTailC (AjPAlign thys, const char* tail);
void         ajAlignSetType (AjPAlign thys);
void         ajAlignTrace (const AjPAlign thys);
void         ajAlignTraceT (const AjPAlign thys, const char* title);
AjBool       ajAlignValid (AjPAlign thys);
void         ajAlignWrite (AjPAlign thys);
void         ajAlignWriteHeader (AjPAlign thys);
void         ajAlignWriteTail (AjPAlign thys);

/*
** End of prototype definitions
*/

#endif

#ifdef __cplusplus
}
#endif

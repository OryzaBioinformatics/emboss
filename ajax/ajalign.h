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
** @set ajAlignReset Resets ready for reuse.
** @use ajAlignWrite Master alignment output routine
** @use ajAlignWriteHeader Master header output routine
** @use ajAlignWriteTail Master footer output routine
** @other AjPSeqout Sequence output
** @other AjPFeatout Feature output
** @other AjPReport Report output
** @other AjPFile Input and output files
** @@
******************************************************************************/

typedef struct AjSAlign {
  AjPStr Name;			/* As "Source" for features, usually empty */
  AjPStr Usa;			/* Name built from input USA, usually empty  */
  AjPStr Type;			/* "P" Protein or "N" Nucleotide */
  AjPStr Formatstr;		/* Report format (-aformat) */
  AjEnum Format;		/* Report format (index number) */
  AjPStr Extension;		/* Output file extension */
  AjPFile File;			/* Output file object */
  AjPStr Header;		/* Text to add to header with newlines */
  AjPStr SubHeader;		/* Text to add to align subheader */
  AjPStr Tail;			/* Text to add to tail with newlines */
  AjBool Showacc;		/* Report accession number */
  AjBool Showdes;		/* Report sequence description */
  AjBool Showusa;		/* Report USA (-ausa) or only seqname */
  AjBool Multi;			/* if true, assume >1 alignment */
  AjBool Global;		/* if true, show full sequence beyond match */
  AjPList Data;			/* Alignment specific data - see ajalign.c */
  ajint Nseqs;			/* Number of sequences in all alignments */
  ajint Nmin;			/* Minimum number of sequences e.g. 2 */
  ajint Nmax;			/* Maximum number of sequences e.g. 2 */
  ajint Width;			/* Output width (minimum 10) */
  ajint Count;			/* Use count */
  AjPMatrix  IMatrix;		/* Integer matrix (see also FMatrix) */
  AjPMatrixf FMatrix;		/* Floating Pt matrix (see also IMatrix) */
  AjPStr Matrix;		/* Matrix name */
  AjPStr GapPen;		/* Gap penalty (converted to string)  */
  AjPStr ExtPen;		/* Gap extend penalty (to string) */
  AjBool SeqOnly;		/* Sequence output only, no head or tail */
  AjBool SeqExternal;		/* Sequence is non-local, do not delete */
} AjOAlign;

#define AjPAlign AjOAlign*

void         ajAlignClose (AjPAlign thys);
AjBool       ajAlignDefine (AjPAlign pthys, AjPSeqset seqset);
AjBool       ajAlignDefineSS (AjPAlign pthys,
			      AjPSeq seqa, AjPSeq seqb);
void         ajAlignDel (AjPAlign* pthys);
AjBool       ajAlignOpen (AjPAlign thys, const AjPStr name);
AjBool       ajAlignFindFormat (const AjPStr format, ajint* iformat);
AjBool       ajAlignFormatDefault (AjPStr* pformat);
AjPAlign     ajAlignNew (void);
void         ajAlignPrintFormat (const AjPFile outf, AjBool full);
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
			      ajint start2, ajint end2);
void         ajAlignSetScoreI (AjPAlign thys, ajint score);
void         ajAlignSetScoreR (AjPAlign thys, float score);
void         ajAlignSetSubHeader (AjPAlign thys, const AjPStr subheader);
void         ajAlignSetSubHeaderApp (AjPAlign thys, const AjPStr subheader);
void         ajAlignSetSubHeaderC (AjPAlign thys, const char* subheader);
void         ajAlignSetSubHeaderPre (AjPAlign thys, const AjPStr subheader);
void         ajAlignSetStats (AjPAlign thys, ajint iali, ajint len,
				    ajint ident, ajint sim, ajint gaps,
				    const AjPStr score);
AjBool       ajAlignSetSubRange (AjPAlign thys, ajint substart1, ajint start1,
				 ajint end1, ajint substart2, ajint start2,
				 ajint end2);
void         ajAlignSetSubStandard (AjPAlign thys, ajint iali);
void         ajAlignSetTail (AjPAlign thys, const AjPStr tail);
void         ajAlignSetTailApp (AjPAlign thys, const AjPStr tail);
void         ajAlignSetTailC (AjPAlign thys, const char* tail);
void         ajAlignSetType (AjPAlign thys);
void         ajAlignTrace (const AjPAlign thys);
AjBool       ajAlignValid (AjPAlign thys);
void         ajAlignWrite (AjPAlign thys);
void         ajAlignWriteClose (AjPAlign thys);
void         ajAlignWriteHeader (AjPAlign thys);
void         ajAlignWriteTail (AjPAlign thys);

#endif

#ifdef __cplusplus
}
#endif

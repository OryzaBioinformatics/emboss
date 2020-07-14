#ifdef __cplusplus
extern "C"
{
#endif




#ifndef embgotoh_h
#define embgotoh_h

/* @data AjPGotohCell *********************************************************
**
** NUCLEUS Gotoh path matrix cell for pairwise dynamic programming
**
** Holds integer co-ordinates of preceding cell in backtrace, cumulative trace
**  score, indexing down and across residue characters and flag for indel status
**
** AjPGotohCell is implemented as a pointer to a C data structure.
**
** @alias AjSGotohCell
** @alias AjOGotohCell
**
** @new ajGotohNew default constructor
** @new ajGotohCreate constructor initializing values of attributes
**
** @delete ajGotohDel default destructor
**
** @use embGotohCellCalculateSumScore fill in alignment array of ajGotohCells
** @use embGotohCellBacktrace find highest scoring path through array of
** ajGotohCells
**
** @attr ajIntRowPointer [ajint] Row number of next cell in backtrace
** @attr ajIntColumnPointer [ajint] Column number of next cell in backtrace
** @attr fSubScore [float] Intermediate cumulative alignment score
** @attr cDownResidue [char] Residue in template indexed by row
** @attr cAcrossResidue [char] Residue in query indexed by column
** @attr ajBoolIsIndel [AjBool] Does cell correspond to an indel
** @@
******************************************************************************/

typedef struct AjGotohCell
{
    ajint ajIntRowPointer;
    ajint ajIntColumnPointer;
    float fSubScore;
    char cDownResidue;
    char cAcrossResidue;
    AjBool ajBoolIsIndel;
} AjOGotohCell;
#define AjPGotohCell AjOGotohCell*




/* ========================================================================= */
/* =================== All functions in alphabetical order ================= */
/* ========================================================================= */

/* ajgotoh.h() $Date: 2004/06/24 22:16:55 $                        DJC Oct03 */

AjPGotohCell** const embGotohCellGetArray(ajint ajIntDownSeqLen,
					  ajint ajIntAcrossSeqLen);
    

AjPGotohCell   embGotohCellCreate(ajint ajIntRow, ajint ajIntColumn,
				  float fSubScore,
				  char cDownResidue,
				  char cAcrossResidue,
				  AjBool ajBoolIsIndel);

void           embGotohCellDel(AjPGotohCell* pthis);

AjPGotohCell   embGotohCellNew(void);

AjPFloat2d     embGotohPairScore(const AjPMatrixf ajpMatrixFscoring,
				 const AjPSeq ajpSeqDown,
				 const AjPSeq ajpSeqAcross,
				 float fExtensionPenalty);

void embGotohCellCalculateSumScore(const AjPFloat2d ajpFloat2dPairScores,
				   const AjPSeq ajpSeqDown,
				   const AjPSeq ajpSeqAcross,
				   AjPGotohCell const **ajpGotohCellGotohScores,
				   float fGapPenalty,
				   float fExtensionPenalty);

ajint   embGotohCellBacktrace(AjPGotohCell const **ajpGotohCellGotohScores,
			      const AjPSeq ajpSeqDown,
			      const AjPSeq ajpSeqAcross,
			      AjPList ajpListGotohCellsMaxScoringTrace);

ajint  embGotohReadOffBacktrace(const AjPList ajpListGotohCellsMaxScoringTrace,
				AjPSeq ajpSeqDown,
				AjPSeq ajpSeqAcross);

#endif




#ifdef __cplusplus
}
#endif

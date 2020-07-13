#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embmat_h
#define embmat_h

#define PRINTS_MAT "PRINTS/prints.mat"

typedef ajint *PMAT_INT[26];

/* @data EmbPMatPrints ********************************************************
**
** NUCLEUS data structure for PRINTS protein fingerprints
**
******************************************************************************/

typedef struct EmbSMatPrints
{
    AjPStr cod;				/* gc line                         */
    AjPStr acc;				/* gx line                         */
    AjPStr tit;				/* gt line                         */
    ajint    n;				/* Number of motifs in fingerprint */
    ajint    *len;			/* Lengths of motifs               */
    ajint    *thresh;			/* % of maximum score for matrix   */
    ajint    *max;			/* Maximum score for matrix        */
    PMAT_INT *matrix;			/* Matrices                        */
} EmbOMatPrints, *EmbPMatPrints;

/* @data EmbPMatMatch *********************************************************
**
** NUCLEUS data structure for sequence matrix matches
**
******************************************************************************/

typedef struct EmbSMatMatch
{
    AjPStr seqname;			/* Sequence name                   */
    AjPStr cod;				/* Matrix name                     */
    AjPStr acc;				/* Matrix accession number         */
    AjPStr tit;				/* Matrix title                    */
    AjPStr pat;                         /* Pattern                         */
    ajint    n;				/* Number of motifs in fingerprint */
    ajint    len;				/* Lengths of motifs               */
    ajint    thresh;			/* % of maximum score for matrix   */
    ajint    max;				/* Maximum score for matrix        */
    ajint    element;			/* Number of matching element      */
    ajint    start;			/* Start of match                  */
    ajint    end;				/* End of match			   */
    ajint    score;			/* Score of match                  */
    ajint    hpe;				/* Hits per element (so far)       */
    ajint    hpm;				/* Hits per motif (so far)         */
    AjBool all;			      /* Can be set if all elements match  */
    AjBool ordered;		      /* Can be set if "all" and in order  */
    AjBool forward;			/* on forward strand               */
    ajint    mm;				/* Number of mismatches            */
    ajint    cut1;
    ajint    cut2;
    ajint    cut3;
    ajint    cut4;
    AjPStr iso;				/* Holds names of isoschizomers    */
} EmbOMatMatch, *EmbPMatMatch;



void   embMatMatchDel (EmbPMatMatch *s);
void   embMatPrintsInit (AjPFile *fp);
void   embMatProtDelInt (EmbPMatPrints *s);
AjBool embMatProtReadInt (AjPFile *fp, EmbPMatPrints *s);
ajint    embMatProtScanInt (AjPStr *s, AjPStr *n, EmbPMatPrints *m, AjPList *l,
			  AjBool *all, AjBool *ordered, AjBool overlap);

#endif

#ifdef __cplusplus
}
#endif

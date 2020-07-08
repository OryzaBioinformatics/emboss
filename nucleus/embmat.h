/*  Last edited: Mar  1 18:56 2000 (pmr) */
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embmat_h
#define embmat_h

#define PRINTS_MAT "PRINTS/prints.mat"

typedef int *PMAT_INT[26];


typedef struct EmbSMatPrints
{
    AjPStr cod;				/* gc line                         */
    AjPStr acc;				/* gx line                         */
    AjPStr tit;				/* gt line                         */
    int    n;				/* Number of motifs in fingerprint */
    int    *len;			/* Lengths of motifs               */
    int    *thresh;			/* % of maximum score for matrix   */
    int    *max;			/* Maximum score for matrix        */
    PMAT_INT *matrix;			/* Matrices                        */
} EmbOMatPrints, *EmbPMatPrints;


typedef struct EmbSMatMatch
{
    AjPStr seqname;			/* Sequence name                   */
    AjPStr cod;				/* Matrix name                     */
    AjPStr acc;				/* Matrix accession number         */
    AjPStr tit;				/* Matrix title                    */
    AjPStr pat;                         /* Pattern                         */
    int    n;				/* Number of motifs in fingerprint */
    int    len;				/* Lengths of motifs               */
    int    thresh;			/* % of maximum score for matrix   */
    int    max;				/* Maximum score for matrix        */
    int    element;			/* Number of matching element      */
    int    start;			/* Start of match                  */
    int    end;				/* End of match			   */
    int    score;			/* Score of match                  */
    int    hpe;				/* Hits per element (so far)       */
    int    hpm;				/* Hits per motif (so far)         */
    AjBool all;			      /* Can be set if all elements match  */
    AjBool ordered;		      /* Can be set if "all" and in order  */
    AjBool forward;			/* on forward strand               */
    int    mm;				/* Number of mismatches            */
    int    cut1;
    int    cut2;
    int    cut3;
    int    cut4;
} EmbOMatMatch, *EmbPMatMatch
;



void   embMatMatchDel (EmbPMatMatch *s);
void   embMatPrintsInit (AjPFile *fp);
void   embMatProtDelInt (EmbPMatPrints *s);
AjBool embMatProtReadInt (AjPFile *fp, EmbPMatPrints *s);
int    embMatProtScanInt (AjPStr *s, AjPStr *n, EmbPMatPrints *m, AjPList *l,
			  AjBool *all, AjBool *ordered, AjBool overlap);

#endif

#ifdef __cplusplus
}
#endif

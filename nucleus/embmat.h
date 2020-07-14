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
** @attr cod [AjPStr] gc line
** @attr acc [AjPStr] gx line
** @attr tit [AjPStr] gt line
** @attr n [ajint] Number of motifs in fingerprint
** @attr len [ajint*] Lengths of motifs
** @attr thresh [ajint*] % of maximum score for matrix
** @attr max [ajint*] Maximum score for matrix
** @attr matrix [PMAT_INT*] Matrices
** @@
******************************************************************************/

typedef struct EmbSMatPrints
{
    AjPStr cod;
    AjPStr acc;
    AjPStr tit;
    ajint    n;
    ajint    *len;
    ajint    *thresh;
    ajint    *max;
    PMAT_INT *matrix;
} EmbOMatPrint;
#define EmbPMatPrints EmbOMatPrint*

/* @data EmbPMatMatch *********************************************************
**
** NUCLEUS data structure for sequence matrix matches
**
** @attr seqname [AjPStr] Sequence name
** @attr cod [AjPStr] Matrix name
** @attr acc [AjPStr] Matrix accession number
** @attr tit [AjPStr] Matrix title
** @attr pat [AjPStr] Pattern
** @attr n [ajint] Number of motifs in fingerprint
** @attr len [ajint] Lengths of motifs
** @attr thresh [ajint] % of maximum score for matrix
** @attr max [ajint] Maximum score for matrix
** @attr element [ajint] Number of matching element
** @attr start [ajint] Start of match
** @attr end [ajint] End of match
** @attr score [ajint] Score of match
** @attr hpe [ajint] Hits per element (so far)
** @attr hpm [ajint] Hits per motif (so far)
** @attr all [AjBool] Can be set if all elements match
** @attr ordered [AjBool] Can be set if "all" and in order
** @attr forward [AjBool] on forward strand
** @attr mm [ajint] Number of mismatches
** @attr cut1 [ajint] Undocumented
** @attr cut2 [ajint] Undocumented
** @attr cut3 [ajint] Undocumented
** @attr cut4 [ajint] Undocumented
** @attr iso [AjPStr] Holds names of isoschizomers
** @@
******************************************************************************/

typedef struct EmbSMatMatch
{
    AjPStr seqname;
    AjPStr cod;
    AjPStr acc;
    AjPStr tit;
    AjPStr pat;
    ajint    n;
    ajint    len;
    ajint    thresh;
    ajint    max;
    ajint    element;
    ajint    start;
    ajint    end;
    ajint    score;
    ajint    hpe;
    ajint    hpm;
    AjBool all;
    AjBool ordered;
    AjBool forward;
    ajint    mm;
    ajint    cut1;
    ajint    cut2;
    ajint    cut3;
    ajint    cut4;
    AjPStr iso;
} EmbOMatMatch;
#define EmbPMatMatch EmbOMatMatch*



void   embMatMatchDel (EmbPMatMatch *s);
void   embMatPrintsInit (AjPFile *fp);
void   embMatProtDelInt (EmbPMatPrints *s);
EmbPMatPrints embMatProtReadInt (AjPFile fp);
ajint    embMatProtScanInt (const AjPStr s, const AjPStr n,
			    const EmbPMatPrints m, AjPList *l,
			    AjBool *all, AjBool *ordered, AjBool overlap);

#endif

#ifdef __cplusplus
}
#endif

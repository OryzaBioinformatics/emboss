/****************************************************************************
** 
** @source embsig.h
**
** Data structures and algorithms for use with sparse sequence signatures.
** Hit, Hitlist, Sigpos, Sigdat and Signature objects.
** 
** Copyright (c) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
** @version 1.0 
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
**
****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef embsig_h
#define embsig_h





/* @data AjPHit *************************************************************
**
** Ajax hit object.
**
** Holds data associated with a protein / domain sequence that is generated 
** and or manipulated by the EMBOSS applications seqsearch, seqsort, and 
** sigscan.
**
** AjPHit is implemented as a pointer to a C data structure.
**
** @alias AjSHit
** @alias AjOHit
**
**
**
** @attr  Seq	   [AjPStr]  Sequence as string.
** @attr  Start    [ajint]   Start of sequence or signature alignment relative
**	           	     to full length swissprot sequence, this is an 
**		             index so starts at 0. 
** @attr  End      [ajint]   End of sequence or signature alignment relative
**		             to full length swissprot sequence, this is an
**         		     index so starts at 0. 
** @attr  Acc      [AjPStr]  Accession number of sequence entry.  
** @attr  Spr      [AjPStr]  Swissprot code of sequence entry. 
** @attr  Rank     [ajint]   Rank order of hit 	
** @attr  Score    [float]   Score of hit 
** @attr  Eval     [float]   E-value of hit 
** @attr  Pval     [float]   p-value of hit 
**  
** @attr  Typeobj  [AjPStr]  Primary (objective) classification of hit.
** @attr  Typesbj  [AjPStr]  Secondary (subjective) classification of hit 
** @attr  Model    [AjPStr]  String for model type if used (HMM, Gribskov etc) 
** @attr  Alg      [AjPStr]  Alignment, e.g. of a signature to the sequence 
** @attr  Group    [AjPStr]  Grouping of hit, e.g. 'REDUNDANT' or 
**                           'NON_REDUNDANT' 
** @attr  Target   [AjBool]  Used for garbage collection.
** @attr  Target2  [AjBool]  Also used for garbage collection.
** @attr  Priority [AjBool]  Also used for garbage collection.
**
**
** 
** @new    embHitNew Default Hit constructor
** @delete embHitDel Default Hit destructor
** @assign embHitMerge Create new Hit from merging two Hit objects
** @use    embMatchScore Sort Hit objects by Score element.
** @use    embMatchinvScore Sort (inverted order) Hit objects by Score 
**         element.
** @use    embHitsOverlap Checks for overlap between two Hit objects.
** 
** @@
****************************************************************************/

typedef struct AjSHit
{
  AjPStr  Seq;	
  ajint   Start;      
  ajint   End;        
  AjPStr  Acc;           
  AjPStr  Spr;        
  ajint   Rank;       
  float   Score;      
  float   Eval;       
  float   Pval;       

  AjPStr  Typeobj;    
  AjPStr  Typesbj;    
  AjPStr  Model;      
  AjPStr  Alg;        
  AjPStr  Group;      
  AjBool  Target;     
  AjBool  Target2;    
  AjBool  Priority;   
} AjOHit, *AjPHit;





/* @data AjPHitlist *********************************************************
**
** Ajax hitlist object.
**
** Holds an array of hit structures and associated SCOP classification 
** records.
**
** AjPHitlist is implemented as a pointer to a C data structure.
**
** @alias AjSHitlist
** @alias AjOHitlist
**
** 
**
** @attr  Class         [AjPStr]    SCOP classification.
** @attr  Fold         [AjPStr]    SCOP classification.
** @attr  Superfamily  [AjPStr]    SCOP classification.
** @attr  Family       [AjPStr]    SCOP classification.
** @attr  Model        [AjPStr]    SCOP classification.
** @attr  Sunid_Family [ajint]     SCOP sunid for family. 
** @attr  Priority     [AjBool]    True if the Hitlist is high priority. 
** @attr  N             [ajint]     No. of hits. 
** @attr  hits         [AjPHit*]  Array of hits. 
**
** @new    embHitlistNew Default Hitlist constructor
** @delete embHitlistDel Default Hitlist destructor
** @use    embHitlistMatchFold Sort Hitlist objects by Fold element
** @input  embHitlistRead Construct Hitlist object from reading the next entry
**         from a file in embl-like format (see documentation for the 
**         DOMAINATRIX "seqsearch" application). 
** @input  embHitlistReadNode Construct Hitlist object from reading a specific
**         entry from a file in embl-like format (see documentation for the 
**         DOMAINATRIX "seqsearch" application). 
** @output embHitlistWrite Write Hitlist to file in embl-like format (see 
**         documentation for the DOMAINATRIX "seqsearch" application). 
** @output embHitlistWriteSubset Write a subset of a Hitlist to file in 
**         embl-like format (see documentation for the DOMAINATRIX "seqsearch"
**         application). 
** @use    embHitlistClassify Classifies a list of signature-sequence hits 
**         (held in a Hitlist object) according to list of target sequences 
**         (a list of Hitlist objects).
** @@
****************************************************************************/

typedef struct AjSHitlist
{
    AjPStr   Class;
    AjPStr   Fold;
    AjPStr   Superfamily;
    AjPStr   Family;
    AjPStr   Model;
    ajint    Sunid_Family;
    AjBool   Priority;     
    ajint    N;            
    AjPHit  *hits;         
} AjOHitlist, *AjPHitlist;





/* @data AjPSigpos **********************************************************
**
** Ajax Sigpos object.
**
** Holds data for compiled signature position
**
** AjPSigpos is implemented as a pointer to a C data structure.
**
** @alias AjSSigpos
** @alias AjOSigpos
**
**
** 
** @attr  ngaps [ajint]   No. of gaps 
** @attr  gsiz  [ajint*]  Gap sizes 
** @attr  gpen  [float*]  Gap penalties 
** @attr  subs  [float*]  Residue match values 
** 
** @new embSigposNew Default Sigdat object constructor
** @delete embSigposDel Default Sigdat object destructor
** @@
****************************************************************************/
typedef struct AJSSigpos
{
    ajint    ngaps;      
    ajint   *gsiz;       
    float   *gpen;       
    float   *subs;       
} AJOSigpos, *AjPSigpos;





/* @data AjPSigdat **********************************************************
**
** Ajax Sigdat object.
**
** Holds empirical data for an (uncompiled) signature position. 
** Important: Functions which manipulate this structure rely on the data in 
** the gap arrays (gsiz and grfq) being filled in order of increasing gap 
** size.
**
** AjPSigdat is implemented as a pointer to a C data structure.
**
** @alias AjSSigdat
** @alias AjOSigdat
**
**
**
** @attr  nres [ajint]    No. diff. types of residue
** @attr  rids [AjPChar]  Residue id's 
** @attr  rfrq [AjPInt]   Residue frequencies 
**
** @attr  ngap [ajint]    No. diff. sizes of empirical gap
** @attr  gsiz [AjPInt]   Gap sizes 
** @attr  gfrq [AjPInt]   Frequencies of gaps of each size
** @attr  wsiz [ajint]    Window size for this gap 
**
** @new embSigdatNew Default Sigdat object constructor
** @delete embSigdatDel Default Sigdat object destructor
** @@
****************************************************************************/
typedef struct AJSSigdat
{
    ajint       nres;         
    AjPChar     rids;         
    AjPInt      rfrq;         
    ajint       ngap;         
    AjPInt      gsiz;         
    AjPInt      gfrq;         
    ajint       wsiz;         
} AJOSigdat, *AjPSigdat;





/* @data AjPSignature *******************************************************
**
** Ajax Signature object.
**
** AjPSignature is implemented as a pointer to a C data structure.
**
** @alias AjSSignature
** @alias AjOSignature
**
** 
**
** @attr  Class        [AjPStr]      SCOP classification.
** @attr  Fold         [AjPStr]      SCOP classification.
** @attr  Superfamily  [AjPStr]      SCOP classification.
** @attr  Family       [AjPStr]      SCOP classification.
** @attr  Sunid_Family [ajint]       SCOP sunid for family. 
** @attr  npos         [ajint]       No. of signature positions.
** @attr  pos          [AjPSigpos*]  Array of derived data for puropses of 
**                                   alignment.
** @attr  dat          [AjPSigdat*]  Array of empirical data.
**
** @new    embSignatureNew Default Signature constructor
** @delete embSignatureDel Default Signature destructor
** @output embSignatureWrite Write signature to file.
** @input  embSignatureReadNew Construct a Signature object from reading a 
**         file in embl-like format (see documentation for the DOMAINATRIX
**         "sigscan" application).
** @output embSignatureWrite Write a Signature object to a file in embl-like 
**         format (see documentation for the DOMAINATRIX "sigscan" 
**         application).
** @input  embSignatureHitsRead Construct a Hitlist object from reading a 
**         signature hits file (see documentation for the DOMAINATRIX 
**         "sigscan" application). 
** @output embSignatureHitsWrite Writes a list of Hit objects to a 
**         signature hits file (see documentation for the DOMAINATRIX 
**         "sigscan" application). 
** @modify embSignatureCompile Compiles a Signature object.  The signature 
**         must first have been allocated by using the embSignatureNew 
**         function.
** @use    embSignatureAlignSeq Performs an alignment of a signature to a 
**         protein sequence. The signature must have first been compiled by 
**         calling embSignatureCompile.  Write a Hit object with the result.
** @use    embSignatureAlignSeqall Performs an alignment of a signature to
**         protein sequences. The signature must have first been compiled by 
**         calling embSignatureCompile.  Write a list of Hit objects with 
**         the result.
** @@
****************************************************************************/
typedef struct AjSSignature
{
    AjPStr      Class;
    AjPStr      Fold;
    AjPStr      Superfamily;
    AjPStr      Family;
    ajint       Sunid_Family; 
    ajint       npos;       
    AjPSigpos  *pos;        
    AjPSigdat  *dat;        
} AjOSignature, *AjPSignature;





/* ======================================================================= */
/* ========================== Signature object =========================== */
/* ======================================================================= */
AjPSignature  embSignatureNew(ajint n);
void          embSignatureDel(AjPSignature *ptr);
AjPSignature  embSignatureReadNew(AjPFile inf);
AjBool        embSignatureWrite(AjPFile outf, const AjPSignature obj);
AjBool        embSignatureCompile(AjPSignature *S, float gapo, float gape,
				  const AjPMatrixf matrix);
AjBool        embSignatureAlignSeq(const AjPSignature S, const AjPSeq seq,
				   AjPHit *hit, 
				   ajint nterm);
AjBool        embSignatureAlignSeqall(const AjPSignature sig,
				      AjPSeqall db, 
				      ajint n, AjPHitlist *hitlist, 
				      ajint nterm);
AjBool        embSignatureHitsWrite(AjPFile outf, const AjPSignature sig, 
				    const AjPHitlist hitlist, ajint n);
AjPHitlist    embSignatureHitsRead(AjPFile inf);





/* ======================================================================= */
/* ============================= Hit object ============================== */
/* ======================================================================= */
AjPHit        embHitNew(void);
void          embHitDel(AjPHit *ptr);
AjPHit        embHitMerge(const AjPHit hit1, const AjPHit hit2);
AjBool        embHitsOverlap(const AjPHit hit1, const AjPHit hit2, ajint n);
ajint         embMatchScore(const void *hit1, const void *hit2);
ajint         embMatchinvScore(const void *hit1, const void *hit2);





/* ======================================================================= */
/* =========================== Hitlist object ============================ */
/* ======================================================================= */
AjPHitlist    embHitlistNew(ajint n);
void          embHitlistDel(AjPHitlist *ptr);
AjPHitlist    embHitlistRead(AjPFile inf);
AjBool        embHitlistWrite(AjPFile outf, const AjPHitlist obj);
AjBool        embHitlistWriteSubset(AjPFile outf, const AjPHitlist obj, 
				    const AjPInt ok);
AjPList       embHitlistReadNode(AjPFile inf, const AjPStr fam, 
				 const AjPStr sfam, const AjPStr fold,
				 const AjPStr klass);
AjBool        embHitlistClassify(AjPHitlist const *hits,
				 const AjPList targets, 
				 ajint thresh);
ajint         embHitlistMatchFold(const void *hit1, const void *hit2);





#endif

#ifdef __cplusplus
}
#endif














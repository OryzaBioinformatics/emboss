/***************************************************************************
**
** @source ajdmx.h
**
** AJAX objects for some of the DOMAINATRIX EMBASSY applications. 
** Scophit and Scopalg objects. 
** 
** @author: Copyright (C) 2004 Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author: Copyright (C) 2004 Jon Ison (jison@hgmp.mrc.ac.uk)
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
***************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdmx_h
#define ajdmx_h





/* @data AjPScophit *********************************************************
**
** Ajax Scophit object.
**
** Holds data associated with a protein / domain sequence that is generated 
** / manipulated by the EMBOSS applications psiblasts, swissparse, seqsort, 
** seqmerge and groups.  Includes SCOP classification records.
**
** AjPScophit is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSScophit
** @alias AjOScophit
**
** @attr Class [AjPStr] Class name
** @attr Fold [AjPStr] Fold name
** @attr Superfamily [AjPStr] Superfamily name
** @attr Family [AjPStr] Family name
** @attr Sunid_Family [ajint] SCOP sunid for family
** @attr Seq [AjPStr] Sequence as string
** @attr Start [ajint] Start of sequence or signature alignment 
**	 relative to full length swissprot sequence
** @attr End [ajint] End of sequence or signature alignment relative 
**	 to full length swissprot sequence
** @attr Acc [AjPStr] Accession number of sequence entry
** @attr Spr [AjPStr] Swissprot code of sequence entry
** @attr Typeobj [AjPStr] Bibliographic information ... objective
** @attr Typesbj [AjPStr] Bibliographic information ... subjective
** @attr Model [AjPStr] String for model type (HMM, Gribskov etc)
** @attr Group [AjPStr] 'REDUNDANT' or 'NON_REDUNDANT'
** @attr Rank [ajint] Rank order of hit
** @attr Score [float] Score of hit
** @attr Eval [float] E-value of hit
** @attr Pval [float] p-value of hit
** @attr Alg [AjPStr] Alignment, e.g. of a signature to the sequence
** @attr Target [AjBool] True if the Scophit is targetted for removal 
**	 from a list of Scophit objects
** @attr Target2 [AjBool] Also used for garbage collection
** @attr Priority [AjBool] True if the Scop hit is high priority
** 
** 
** 
** @new   ajDmxScophitNew Default Scophit object constructor. 
** @delete ajDmxScophitDel Default Scophit object destructor. 
** @delete ajDmxScophitDelWrap Wrapper to destructor for Scophit object for use
**        with generic functions.
** @assign ajDmxScophitListCopy Reads a list of Scophit structures and returns
**        a pointer to a duplicate of the list. 
** @use   ajDmxScophitCheckTarget Checks to see if the Target element of a 
**        Scophit object == ajTrue.
** @modify ajDmxScophitTarget Sets the Target element of a Scophit object to 
**        True.
** @modify ajDmxScophitTarget2 Sets the Target2 element of a Scophit object to 
**        True.
** @modify ajDmxScophitTargetLowPriority  Sets the Target element of a Scophit 
**        object to True if its Priority is low.
** @use   ajDmxScophitCompSpr Function to sort Scophit object by Spr element. 
** @use   ajDmxScophitCompStart  Function to sort Scophit object by Spr 
**        element.
** @use   ajDmxScophitCompEnd  Function to sort Scophit object by End element. 
** @use   ajDmxScophitCompFold  Function to sort Scophit object by Fold 
**        element.
** @use   ajDmxScophitCompSfam  Function to sort Scophit object by Superfamily 
**        element. 
** @use   ajDmxScophitCompFam  Function to sort Scophit object by Family 
**        element. 
** @use   ajDmxScophitCompAcc  Function to sort Scophit objects by Acc element.
** @use   ajDmxScophitCompSunid  Function to sort Scophit objects by Sunid 
**        element. 
** @use   ajDmxScophitCompScore  Function to sort Scophit objects by Score 
**        element. 
** @use   ajDmxScophitCompPval  Function to sort Scophit objects by Pval 
**        element.
** @use   ajDmxScopSeqFromSunid  Writes a sequence corresponding to a Scop 
**        domain given a Sunid for the domain. The sequence is taken from one
**        of a list of Scop objects that is provided.  
** @output ajDmxScophitsWrite Write contents of a list of Scophits to an output
**        file.
** @assign ajDmxScophitCopy Copies the contents from one Scophit object to 
**        another.
** @@
****************************************************************************/

typedef struct AjSScophit
{
    AjPStr    Class;
    AjPStr    Fold;
    AjPStr    Superfamily;
    AjPStr    Family;
    ajint     Sunid_Family;
    AjPStr    Seq;
    ajint     Start;
    ajint     End;
    AjPStr    Acc;
    AjPStr    Spr;
    AjPStr    Typeobj;
    AjPStr    Typesbj;
    AjPStr    Model;
    AjPStr    Group;
    ajint     Rank;
    float     Score;
    float     Eval;
    float     Pval;
    AjPStr    Alg;
    AjBool    Target;
    AjBool    Target2;
    AjBool    Priority;
} AjOScophit;
#define AjPScophit AjOScophit*





/* @data AjPScopalg *********************************************************
**
** Ajax Scopalg object.
**
** Holds data associated with a structure alignment that is generated 
** by the EMBOSS applications scopalign.
**
** AjPScopalg is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSScopalg
** @alias AjOScopalg
**
** @attr Class [AjPStr] Class name
** @attr Fold [AjPStr] Fold name
** @attr Superfamily [AjPStr] Superfamily name
** @attr Family [AjPStr] Family name
** @attr Sunid_Family [ajint] SCOP sunid for family
** @attr width [ajint] Width (residues) of widest part of alignment
** @attr N [ajint] No. of sequences in alignment
** @attr Codes [AjPStr*] Array of domain id codes of sequences
** @attr Seqs [AjPStr*] Array of sequences
** @attr Post_similar [AjPStr] Post_similar line from alignment
** @attr Positions [AjPStr] Array of integers from 'Position' line in 
**			      alignment, used for manual specification of 
**			      signature positions
**
**
**
** @input ajDmxScopalgRead Read a Scopalg object from a file.
** @output ajDmxScopalgWrite Write a Scopalg object to file in clustal format
**        annotated with SCOP classification info.
** @output ajDmxScopalgWriteClustal Writes a Scopalg object to file in clustal
**        format (just the alignment without the SCOP classification 
**        information).
** @output ajDmxScopalgWriteClustal2 Writes a Scopalg object to file in clustal
**        format (just the alignment without the SCOP classification 
**        information).
** @output ajDmxScopalgWriteFasta Writes a Scopalg object to file in clustal
**        format (just the alignment without the SCOP classification 
**        information).
** @new   ajDmxScopalgNew Scopalg object constructor.
** @delete ajDmxScopalgDel Scopalg object destructor.
** @input ajDmxScopalgGetseqs Read a Scopalg object and writes an array of 
**        AjPStr containing the sequences without gaps.
** @@
****************************************************************************/
typedef struct AjSScopalg
{
    AjPStr   Class;
    AjPStr   Fold;
    AjPStr   Superfamily;
    AjPStr   Family;
    ajint    Sunid_Family;
    ajint    width;
    ajint    N;
    AjPStr  *Codes;
    AjPStr  *Seqs;
    AjPStr   Post_similar;
    AjPStr   Positions;
} AjOScopalg;
#define AjPScopalg AjOScopalg*



void          ajDmxDummyFunction(void);


/* ======================================================================= */
/* =========================== Scophit object ============================ */
/* ======================================================================= */
AjPScophit    ajDmxScophitNew(void);
void          ajDmxScophitDel(AjPScophit *pthis);
void          ajDmxScophitDelWrap(const void  **ptr);
AjPList       ajDmxScophitListCopy(const AjPList ptr);

AjBool        ajDmxScophitCheckTarget(const AjPScophit ptr);
AjBool        ajDmxScophitTarget(AjPScophit *h);
AjBool        ajDmxScophitTarget2(AjPScophit *h);
AjBool        ajDmxScophitTargetLowPriority(AjPScophit *h);

ajint         ajDmxScophitCompSpr(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompStart(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompEnd(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompFold(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompSfam(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompFam(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompAcc(const void *hit1, const void *hit2);
ajint         ajDmxScophitCompSunid(const void *entry1, const void *entry2);
ajint         ajDmxScophitCompScore(const void *hit1, const void *hit2); 
ajint         ajDmxScophitCompPval(const void *hit1, const void *hit2); 

AjBool        ajDmxScopSeqFromSunid(ajint id, AjPStr *seq, const AjPList list);
AjBool        ajDmxScophitsWrite(AjPFile outf, const AjPList list);
AjBool        ajDmxScophitCopy(AjPScophit *to, const AjPScophit from);





/* ======================================================================= */
/* ========================== Scopalg object ============================= */
/* ======================================================================= */
AjBool        ajDmxScopalgRead(AjPFile inf, AjPScopalg *thys);
AjBool        ajDmxScopalgWrite(const AjPScopalg scop, AjPFile outf);
AjBool        ajDmxScopalgWriteClustal(const AjPScopalg align, AjPFile outf);
AjBool        ajDmxScopalgWriteClustal2(const AjPScopalg align, AjPFile outf);
AjBool        ajDmxScopalgWriteFasta(const AjPScopalg align, AjPFile outf);
AjPScopalg    ajDmxScopalgNew(ajint n);
void          ajDmxScopalgDel(AjPScopalg *pthis);
ajint         ajDmxScopalgGetseqs(const AjPScopalg thys, AjPStr **arr);






#endif

#ifdef __cplusplus
}
#endif




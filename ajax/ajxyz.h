#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajxyz_h
#define ajxyz_h

#define ajXRAY  0    /* Structure was determined by X-ray crystallography */
#define ajNMR   1    /* Structure was determined by NMR or is a model     */


#define ajESCOP "Escop.dat" /* Scop data file */




/* @data AjPScorealg *******************************************************
**
** Ajax Scorealg object.
**
** Holds scores associated with the 5 scoring methods used in siggen
**
** AjPScorealg is implemented as a pointer to a C data structure.
**
** @alias AjSScorealg
** @alias AjOScorealg
**
** @@
******************************************************************************/
typedef struct AjSScorealg
{   
/*JC AjPInt ==> AjPFloat */    AjPFloat  seq_score;    /* Array of scores based on residue convervation */
    AjPInt    post_similar; /* Array of scores based on stamp pij value      */
/*JC AjPInt ==> AjPFloat */    AjPFloat    ncon_score;   /* Array of scores based on number of contacts   */
/*JC AjPInt ==> AjPFloat */     AjPFloat    ccon_score;   /* Array of scores based on convervation of contacts */
    AjPInt    nccon_score; /* Array of total score based on convervation and number of contacts */

    AjPInt    combi_score;  /* Array of total score based on users scoring criteria  */
    
    AjBool    seq_do;       /* Whether to use score based on residue convervation */
    AjBool    filter;       /* Whether to filter on basis of post_similar line      */
    AjBool    ncon_do;      /* Whether to use score based on number of contacts   */
    AjBool    ccon_do;      /* Whether to use score based on convervation of contacts */
    AjBool    nccon_do;    /* Whether to use score based on convervation and number of contacts */
} AjOScorealg, *AjPScorealg;



/* @data AjPVdwres *******************************************************
**
** Ajax Vdwres object.
**
** Holds the Van der Waals radius for atoms in a residue 
**
** AjPVdwres is implemented as a pointer to a C data structure.
**
** @alias AjSVdwres
** @alias AjOVdwres
**
** @@
******************************************************************************/
typedef struct AjSVdwres
{
    char       Id1;        /* Standard residue identifier or 'X' for unknown */
    AjPStr     Id3;        /* 3 character residue identifier */
    ajint      N;          /* Nummber of atoms in residue */
    AjPStr    *Atm;        /* Array of atom identifiers */
    float     *Rad;        /* Array of van der Waals radii */
} AjOVdwres, *AjPVdwres;







/* @data AjPVdwall *******************************************************
**
** Ajax Vdwall object.
**
** Holds the Van der Waals radii for all types of protein atoms
**
** AjPVdwall is implemented as a pointer to a C data structure.
**
** @alias AjSVdwall
** @alias AjOVdwall
**
** @@
******************************************************************************/
typedef struct AjSVdwall
{
    ajint       N;      /* Number of residues */
    AjPVdwres  *Res;    /* Array of Vdwres structures */
} AjOVdwall, *AjPVdwall;






/* @data AjPCmap *******************************************************
**
** Ajax Cmap object.
**
** Holds a contact map and associated data for a protein domain / chain.
**
** AjPCmap is implemented as a pointer to a C data structure.
**
** @alias AjSCmap
** @alias AjOCmap
**
** @@
******************************************************************************/
typedef struct AjSCmap
{
    AjPStr      Id;     /* Protein id code */
    AjPInt2d    Mat;    /* Contact map */
    ajint       Dim;    /* Dimension of contact map */
    ajint       Ncon;   /* No. of contacts (1's in contact map) */
} AjOCmap, *AjPCmap;





/* @data AjPScopalg *******************************************************
**
** Ajax Scopalg object.
**
** Holds data associated with a structure alignment that is generated 
** by the EMBOSS applications scopalign.
**
** AjPScopalg is implemented as a pointer to a C data structure.
**
** @alias AjSScopalg
** @alias AjOScopalg
**
** @@
******************************************************************************/
typedef struct AjSScopalg
{
    AjPStr   Class;
    AjPStr   Fold;
    AjPStr   Superfamily;
    AjPStr   Family;
    ajint    width;        /* Width (residues) of widest part of alignment */
    ajint    N;            /* No. of sequences in alignment */
    AjPStr  *Codes;        /* Array of id codes of sequences */
    AjPStr  *Seqs;         /* Array of sequences */
    AjPStr   Post_similar; /* Post_similar line from alignment */
} AjOScopalg, *AjPScopalg;






/* @data AjPScophit *******************************************************
**
** Ajax Scophit object.
**
** Holds data associated with a protein / domain sequence that is generated 
** / manipulated by the EMBOSS applications psiblasts, swissparse, seqsort, 
** seqmerge and groups.  Includes SCOP classification records.
**
** AjPScophit is implemented as a pointer to a C data structure.
**
** @alias AjSScophit
** @alias AjOScophit
**
** @@
******************************************************************************/

typedef struct AjSScophit
{
    AjPStr    Class;
    AjPStr    Fold;
    AjPStr    Superfamily;
    AjPStr    Family;
    AjPStr    Seq;	  /* Sequence as string */
    ajint     Start;      /* Start of sequence or signature alignment relative to full length 
			    swissprot sequence */
    ajint     End;        /* End of sequence or signature alignment relative to full length 
			    swissprot sequence */
    AjPStr    Id;         /* Identifier */  
    AjPStr    Typeobj;    /* Bibliographic information ... objective*/ 
    AjPStr    Typesbj;    /* Bibliographic information ... subjective */ 
    ajint     Group;      /* Group no. of sequence */
    ajint     Rank;       /* Rank order of hit */	
    float     Score;      /* Score of hit */
    float     Eval;       /* E-value of hit */
    AjPStr    Alg;        /* Alignment, e.g. of a signature to the sequence */
} AjOScophit, *AjPScophit;




/* @data AjPHit *******************************************************
**
** Ajax hit object.
**
** Holds data associated with a protein / domain sequence that is generated 
** / manipulated by the EMBOSS applications psiblasts, swissparse, seqsort, 
** seqmerge, groups and sigscan.
**
** AjPHit is implemented as a pointer to a C data structure.
**
** @alias AjSHit
** @alias AjOHit
**
** @@
******************************************************************************/

typedef struct AjSHit
{
  AjPStr    Seq;	/* Sequence as string */
  ajint     Start;      /* Start of sequence or signature alignment relative to full length 
			    swissprot sequence */
  ajint     End;        /* End of sequence or signature alignment relative to full length 
			    swissprot sequence */
  AjPStr    Id;         /* Identifier */  
  AjPStr    Typeobj;    /* Primary classification of hit - objective*/
  AjPStr    Typesbj;    /* Secondary classification of hit */
  AjPStr    Alg;        /* Alignment, e.g. of a signature to the sequence */
  ajint     Group;      /* Group no. of sequence */
  ajint     Rank;       /* Rank order of hit */	
  float     Score;      /* Score of hit */
  float     Eval;       /* E-value of hit */
} AjOHit, *AjPHit;


/* @data AjPHitlist *******************************************************
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
** @@
******************************************************************************/

typedef struct AjSHitlist
{
    AjPStr  Class;
    AjPStr  Fold;
    AjPStr  Superfamily;
    AjPStr  Family;
    ajint   N;            /* No. of hits */
    AjPHit *hits;        /* Array of hits */
} AjOHitlist, *AjPHitlist;





/* @data AjPHitidx *******************************************************
**
** Ajax Hitidx object.
**
** Holds data for an indexing Hit and Hitlist objects
**
** AjPHitidx is implemented as a pointer to a C data structure.
**
** @alias AjSHitidx
** @alias AjOHitidx
**
** @@
******************************************************************************/
typedef struct AjSHitidx
{  
    AjPStr      Id;        /* Identifier */  
    AjPHit      hptr;      /* Pointer to AjPHit structure*/
    AjPHitlist  lptr;      /* Pointer to AjPHitlist structure*/
}AjOHitidx, *AjPHitidx;






/* @data AjPAtom *******************************************************
**
** Ajax atom object.
**
** Holds protein atom data
**
** AjPAtom is implemented as a pointer to a C data structure.
**
** @alias AjSAtom
** @alias AjOAtom
**
** @@
******************************************************************************/

typedef struct AjSAtom
{
  ajint        Mod;        /*Model number*/
  ajint        Chn;        /*Chain number*/
  char       Type;       /*'P' (protein atom), 'H' ("heterogens") or 'w' 
			   (water)*/
  ajint        Idx;        /*Residue number - index into sequence*/
  AjPStr     Pdb;        /*Residue number - according to original PDB file*/
  char       Id1;        /*Standard residue identifier or 'X' for unknown 
			   types or '.' for heterogens and water*/
  AjPStr     Id3;        /*Residue or group identifier*/
  AjPStr     Atm;        /*Atom identifier*/
  float      X;          /*X coordinate*/
  float      Y;          /*Y coordinate*/
  float      Z;          /*Z coordinate*/
  float      O;          /*Occupancy */
  float      B;          /*B value thermal factor*/
} AjOAtom, *AjPAtom;


/* @data AjPChain *******************************************************
**
** Ajax chain object.
**
** Holds protein chain data
**
** AjPChain is implemented as a pointer to a C data structure.
**
** @alias AjSChain
** @alias AjOChain
**
** @@
******************************************************************************/

typedef struct AjSChain
{
  char       Id;         /*Chain id, ('.' if one wasn't specified in the 
			   original PDB file)*/
  ajint        Nres;       /*No. of amino acid residues*/
  ajint        Nhet;       /*No. of atoms which are non-covalently associated 
			   with the chain, excluding water ("heterogens")*/
  ajint        Nwat;       /*No. of water atoms which are associated with the 
			   chain*/
  AjPStr     Seq;	 /* sequence as string */
  AjPList    Atoms;      /*List of Atoms */
} AjOChain, *AjPChain;



/* @data AjPPdb *******************************************************
**
** Ajax pdb object.
**
** Holds arrays describing pdb data
**
** AjPPdb is implemented as a pointer to a C data structure.
**
** @alias AjSPdb
** @alias AjOPdb
**
** @@
******************************************************************************/

typedef struct AjSPdb
{
  AjPStr     Pdb;        /*PDB code*/
  AjPStr     Compnd;     /*Text from COMPND records in PDB file*/
  AjPStr     Source;     /*Text from SOURCE records in PDB file*/
  ajint        Method;     /*Exp. type, value is either XRAY or NMR*/
  float      Reso;       /*Resolution of an XRAY structure or 0*/
  ajint        Nmod;       /*No. of models (always 1 for XRAY structures)*/
  ajint        Nchn;       /*No. polypeptide chains */
  AjPChain  *Chains;     /*Array of pointers to AjSChain structures*/
}AjOPdb, *AjPPdb;

/* @data AjPScop *******************************************************
**
** Ajax scop object.
**
** Holds scop database data
**
** AjPScop is implemented as a pointer to a C data structure.
**
** @alias AjSScop
** @alias AjOScop
**
** @@
******************************************************************************/

typedef struct AjSScop
{
    AjPStr Entry;
    AjPStr Pdb;
    AjPStr Class;
    AjPStr Fold;
    AjPStr Superfamily;
    AjPStr Family;
    AjPStr Domain;
    AjPStr Source;
    ajint    N;
    char   *Chain;
    AjPStr *Start;
    AjPStr *End;
} AjOScop,*AjPScop;




/* @data AjPSigcell *******************************************************
**
** Ajax Sigcell object.
**
** Holds data for a cell of a path matrix for signature:sequence alignment.
**
** AjPSigcell is implemented as a pointer to a C data structure.
**
** @alias AjSSigcell
** @alias AjOSigcell
**
** @@
******************************************************************************/
typedef struct AjSSigcell
{
    float  val;            /* Value for this cell */
    ajint  prev;           /* Index in path matrix of prev. highest value */
    AjBool try;            /* == ajTrue if this cell has been visited */
} AjOSigcell, *AjPSigcell;





/* @data AjPSigdat *******************************************************
**
** Ajax Sigdat object.
**
** Holds empirical data for an (uncompiled) signature position. 
** Important: Functions which manipulate this structure rely on the data in 
** the gap arrays (gsiz and grfq) being filled in order of increasing gap size.
**
** AjPSigdat is implemented as a pointer to a C data structure.
**
** @alias AjSSigdat
** @alias AjOSigdat
**
** @@
******************************************************************************/
typedef struct AJSSigdat
{
    ajint       nres;         /* No. diff. types of residue*/
    AjPChar     rids;         /* Residue id's */
    AjPInt      rfrq;         /* Residue frequencies */

    ajint       ngap;         /* No. diff. sizes of empirical gap*/
    AjPInt      gsiz;         /* Gap sizes */
    AjPInt      gfrq;         /* Frequencies of gaps of each size*/
    ajint       wsiz;         /* Window size for this gap */
} AJOSigdat, *AjPSigdat;






/* @data AjPSigpos *******************************************************
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
** @@
******************************************************************************/
typedef struct AJSSigpos
{
    ajint    ngaps;        /* No. of gaps */
    ajint   *gsiz;         /* Gap sizes */
    float   *gpen;         /* Gap penalties */
    float   *subs;         /* Residue match values */
} AJOSigpos, *AjPSigpos;




/* @data AjPSignature *******************************************************
**
** Ajax Signature object.
**
** Holds data for 
**
** AjPSignature is implemented as a pointer to a C data structure.
**
** @alias AjSSignature
** @alias AjOSignature
**
** @@
******************************************************************************/
typedef struct AjSSignature
{
    AjPStr     Class;
    AjPStr     Fold;
    AjPStr     Superfamily;
    AjPStr     Family;
    ajint      npos;       /*No. of signature positions*/
    AjPSigpos *pos;        /*Array of derived data for puropses of 
			     alignment*/
    AjPSigdat *dat;        /*Array of empirical data*/
} AjOSignature, *AjPSignature;










/* @data AjPXXX *******************************************************
**
** Ajax XXX object.
**
** Holds data for 
**
** AjPXXX is implemented as a pointer to a C data structure.
**
** @alias AjSXXX
** @alias AjOXXX
**
** @@
******************************************************************************/








AjPAtom  ajXyzAtomNew(void);
void     ajXyzAtomDel(AjPAtom *thys);
AjPChain ajXyzChainNew(void);
void     ajXyzChainDel(AjPChain *thys);
AjPPdb   ajXyzPdbNew(ajint chains);
void     ajXyzPdbDel(AjPPdb *thys);
void     ajXyzScopDel(AjPScop *pthis);
AjPScop  ajXyzScopNew(ajint n);


AjPScophit  ajXyzScophitNew(void);
void     ajXyzScophitDel(AjPScophit *pthis);
AjBool ajXyzHitlistToScophits(AjPList in, AjPList *out);
AjBool        ajXyzHitsOverlap(AjPHit h1, AjPHit h2, ajint n);
AjBool        ajXyzScophitsOverlap(AjPScophit h1, AjPScophit h2, ajint n);
AjBool ajXyzScophitCopy(AjPScophit *to, AjPScophit from);




void     ajXyzHitDel(AjPHit *pthis);
AjPHit   ajXyzHitNew(void);
void     ajXyzHitlistDel(AjPHitlist *pthis);
AjPHitlist  ajXyzHitlistNew(int n);
AjBool   ajXyzHitlistRead(AjPFile inf, char *delim, AjPHitlist *thys);
AjBool   ajXyzHitlistWrite(AjPFile outf, AjPHitlist thys);

AjBool   ajXyzScopRead(AjPFile inf, AjPStr entry, AjPScop *thys);
AjBool   ajXyzScopReadC(AjPFile inf, char *entry, AjPScop *thys);
void     ajXyzScopWrite(AjPFile outf, AjPScop thys);

AjBool   ajXyzCpdbRead(AjPFile inf, AjPPdb *thys);
AjBool   ajXyzCpdbWriteAll(AjPFile out, AjPPdb thys);
AjBool   ajXyzCpdbWriteDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
			      AjPScop scop);

AjBool   ajXyzPdbWriteAll(AjPFile errf, AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPdbWriteDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
			     AjPScop scop); 

AjBool   ajXyzPrintPdbSeqresChain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				  ajint chn);
AjBool   ajXyzPrintPdbSeqresDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				   AjPScop scop);
AjBool   ajXyzPrintPdbAtomChain(AjPFile outf, AjPPdb pdb, ajint mod, ajint chn);
AjBool   ajXyzPrintPdbAtomDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				 AjPScop scop, ajint mod);
AjBool   ajXyzPrintPdbText(AjPFile outf, AjPStr str, char *prefix);
AjBool   ajXyzPrintPdbHeader(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbHeaderScop(AjPFile outf, AjPScop scop);
AjBool   ajXyzPrintPdbTitle(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbCompnd(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbSource(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbEmptyRemark(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbResolution(AjPFile outf, AjPPdb pdb);

AjBool   ajXyzPdbChain(char id, AjPPdb pdb, ajint *chn);
void     ajXyzScopToPdb(AjPStr scop, AjPStr *pdb);


AjBool   ajXyzScopalgRead(AjPFile inf, AjPScopalg *thys);
AjBool   ajXyzScopalgWrite(AjPFile outf, AjPScopalg *thys);
void ajXyzScopalgDel(AjPScopalg *pthis);
AjPScopalg  ajXyzScopalgNew(int n);
ajint ajXyzScopalgGetseqs(AjPScopalg thys, AjPStr **arr);


void ajXyzCmapDel(AjPCmap *pthis);
AjPCmap  ajXyzCmapNew(int dim);

AjBool   ajXyzCmapRead(AjPFile inf, ajint mode, ajint chn, ajint mod, AjPCmap *thys);
AjBool   ajXyzCmapReadC(AjPFile inf, char chn, ajint mod, AjPCmap *thys);
AjBool   ajXyzCmapReadI(AjPFile inf, ajint chn, ajint mod, AjPCmap *thys);


AjPVdwall  ajXyzVdwallNew(ajint n);
AjPVdwres  ajXyzVdwresNew(ajint n);
void ajXyzVdwresDel(AjPVdwres *pthis);
void ajXyzVdwallDel(AjPVdwall *pthis);
AjBool   ajXyzVdwallRead(AjPFile inf, AjPVdwall *thys);

AjPScorealg  ajXyzScorealgNew(ajint len);
void ajXyzScorealgDel(AjPScorealg *pthis);
float ajXyzVdwRad(AjPAtom atm, AjPVdwall vdw);


AjPSigdat     ajXyzSigdatNew(int nres, int ngap);
AjPSigpos     ajXyzSigposNew(ajint ngap);
void          ajXyzSigdatDel(AjPSigdat *pthis);
void          ajXyzSigposDel(AjPSigpos *thys);
AjPHitidx     ajXyzHitidxNew(void);
void          ajXyzHitidxDel(AjPHitidx *pthis);

AjPSignature  ajXyzSignatureNew(int n);
void          ajXyzSignatureDel(AjPSignature *pthis);
AjBool        ajXyzSignatureRead(AjPFile inf, AjPSignature *thys);
AjBool        ajXyzSignatureWrite(AjPFile outf, AjPSignature thys);
AjBool        ajXyzSignatureCompile(AjPSignature *S, float gapo, float gape, 	
				    AjPMatrixf matrix);
AjBool        ajXyzSignatureAlignSeq(AjPSignature S, AjPSeq seq, AjPHit *hit, 
				     ajint nterm);
AjBool        ajXyzSignatureAlignSeqall(AjPSignature sig, AjPSeqall db, ajint n, 
					AjPHitlist *hits, ajint nterm);
AjBool        ajXyzSignatureHitsWrite(AjPFile outf, AjPSignature sig, 
				      AjPHitlist hits);
AjBool        ajXyzSignatureAlignWrite(AjPFile outf, AjPSignature sig, 
				       AjPHitlist hits);


ajint         ajXyzCompScore(const void *hit1, const void *hit2);
ajint         ajXyzCompId(const void *hit1, const void *hit2);
ajint         ajXyzBinSearch(AjPStr id, AjPHitidx *arr, ajint siz);

AjBool        ajXyzHitlistClassify(AjPHitlist *hits, AjPList targets, 
				   ajint thresh);






#endif

#ifdef __cplusplus
}
#endif


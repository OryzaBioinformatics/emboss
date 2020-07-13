#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajxyz_h
#define ajxyz_h

#define ajXRAY  0    /* Structure was determined by X-ray crystallography */
#define ajNMR   1    /* Structure was determined by NMR or is a model     */


#define ajESCOP "Escop.dat" /* Scop data file */




/* @data AjPDomConts ***********************************************************
**
** Ajax DomConts object.
**
** Holds the domain contact data
**
** AjPDomConts is implemented as a pointer to a C data structure.
**
** @alias AjSDomConts
** @alias AjODomConts
**
** @@
******************************************************************************/



typedef struct AjSDomConts
{
  AjPStr het_name;       /* 3-character code of heterogen */
  AjPStr scop_name;      /* 7-character scop id domain name */  
  ajint  no_keyres;      /* number of key binding residues */
  AjPStr *aa_code;       /* Array for 3-character amino acid codes */ 
  AjPInt res_pos;        /* Array of ints for residue positions in domain file */
  AjPStr *res_pos2;      /* Array of residue positions in complete protein 
			    coordinate file - exist as strings */
}AjODomConts, *AjPDomConts;


/* @data AjPDbaseEnt ***********************************************************
**
** Ajax DbaseEnt object.
**
** Holds the data required for the database of functional sites
**
** AjPDbaseDat is implemented as a pointer to a C data structure.
**
** @alias AjSDbaseEnt
** @alias AjODbaseEnt
**
** @@
******************************************************************************/
typedef struct AjSDbaseEnt
{
  AjPStr      abv;         /* 3-letter abbreviation of heterogen */
  AjPStr      ful;         /* Full name */
  ajint       no_dom;      /* number of domains */
  AjPDomConts *cont_data;  /* array of domain contact data (derived from tmp)*/
  AjPList     tmp;         /* Temp. list of domain contact data */
} AjODbaseEnt, *AjPDbaseEnt;


/* @data AjPDbase ***********************************************************
**
** Ajax Dbase object.
**
** Holds a Database of functional residues.
**
** AjPDbase is implemented as a pointer to a C data structure.
**
** @alias AjSDbase
** @alias AjODbase  
**
** @@
******************************************************************************/
typedef struct AjSDbase
{
  ajint         n;        /* Number of entries */
  AjPDbaseEnt *entries;   /* Array of entries */
} AjODbase, *AjPDbase;




/*@data AjPCath **********************************************************
**
** Ajax cath object
**
** Holds cath database data
**
** The variables have the following meaning:
**
**  AjPStr DomainID;       Domain identifer code        
**  AjPStr Pdb;            Corresponding pdb identifer code
**  AjPStr Class;          CATH class name as an AjPStr
**  AjPStr Architecture;   CATH architecture name as an AjPStr
**  AjPStr Topology;       CATH topology name as an AjPStr
**  AjPStr Superfamily;    CATH homologous superfamily name as an AjPStr
**  ajint  Length;         No. of residues in domain
**  char   Chain;          Chain identifier
**  ajint  NSegment;       No. of chain segments domain is comprised of
**  AjPStr *Start;         PDB residue number of first residue in segment 
**  AjPStr *End;           PDB residue number of last residue in segment
**  ajint Class_Id;        CATH class no. as an ajint
**  ajint Arch_Id;         CATH architecture no.as an ajint
**  ajint Topology_Id;     CATH topology no. as an ajint
**  ajint Superfamily_Id;  CATH superfamily no. as an ajint
**  ajint Family_Id;       CATH family no. as an ajint 
**  ajint NIFamily_Id;     CATH near identical family no. as an ajint 
**  ajint IFamily_Id;      CATH identical family no. as an ajint 
**
**  @alias AjSCath
**  @alias AjOCath
**
**  @@
**************************************************************************/

typedef struct AjSCath
{
    AjPStr DomainID;       
    AjPStr Pdb;            
    AjPStr Class;          
    AjPStr Architecture;   
    AjPStr Topology;       
    AjPStr Superfamily;    
    
    ajint  Length;         
    char   Chain;          
    
    ajint  NSegment;       
    AjPStr *Start;   /*String used instead of int as need to use '.'*/      
    AjPStr *End;          
    
    ajint Class_Id;        
    ajint Arch_Id;         
    ajint Topology_Id;     
    ajint Superfamily_Id;  
    ajint Family_Id;      
    ajint NIFamily_Id;     
    ajint IFamily_Id;     
} AjOCath, *AjPCath;








/* @data AjPPdbtosp *******************************************************
**
** Ajax Pdbtosp object.
**
** Holds swissprot codes and accession numbers for a PDB code.
**
** AjPPdbtosp is implemented as a pointer to a C data structure.
**
** @alias AjSPdbtosp
** @alias AjOPdbtosp
**
** @@
******************************************************************************/
typedef struct AjSPdbtosp
{   	
    AjPStr     Pdb;    /* PDB code*/
    ajint      n;      /* No. entries for this pdb code */
    AjPStr    *Acc;    /* Accession numbers */
    AjPStr    *Spr;    /* Swissprot codes */
} AjOPdbtosp, *AjPPdbtosp;




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
    AjPFloat  seqmat_score;    /* Array of scores based on residue convervation */
    AjPFloat  seqvar_score;    /* Array of scores based on residue variability */
    AjPInt    post_similar;    /* Array of scores based on stamp pij value      */
    AjPInt    positions;       /* Array of integers from 'Position' line in alignment, 
				  used for manual specification of signature positions*/
    AjPFloat  ncon_score;      /* Array of scores based on number of contacts   */
    AjPFloat  ccon_score;      /* Array of scores based on convervation of contacts */
    AjPInt    nccon_score;     /* Array of total score based on convervation and number of contacts */
    AjPInt    combi_score;     /* Array of total score based on users scoring criteria  */
    AjPInt    ncon_thresh;     /* Array of positions with > threshold number of contacts */
    AjBool    seqmat_do;       /* Whether to use score based on residue convervation */
    AjBool    seqvar_do;       /* Whether to use score based on residue variablility */
    AjBool    filterpsim;      /* Whether to filter on basis of post_similar line      */
    AjBool    filtercon;       /* Whether to filter on basis of number of contacts      */
    ajint     conthresh;       /* Threshold number of contacts for filtercon */
    AjBool    ncon_do;         /* Whether to use score based on number of contacts   */
    AjBool    ccon_do;         /* Whether to use score based on convervation of contacts */
    AjBool    nccon_do;        /* Whether to use score based on convervation and number of contacts */

    AjBool    random;          /* Whether to generate a randomised signature */
    AjBool    manual;          /* Whether signature positions were taken from alignment file (manual selection) */
    
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
    ajint    Sunid_Family;        /* SCOP sunid for family */
    ajint    width;        /* Width (residues) of widest part of alignment */
    ajint    N;            /* No. of sequences in alignment */
    AjPStr  *Codes;        /* Array of domain id codes of sequences */
    AjPStr  *Seqs;         /* Array of sequences */
    AjPStr   Post_similar; /* Post_similar line from alignment */
    AjPStr   Positions;   /* Array of integers from 'Position' line in alignment, 
				  used for manual specification of signature positions*/
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
    ajint    Sunid_Family;        /* SCOP sunid for family */
    AjPStr    Seq;	  /* Sequence as string */
    ajint     Start;      /* Start of sequence or signature alignment relative to full length 
			    swissprot sequence */
    ajint     End;        /* End of sequence or signature alignment relative to full length 
			    swissprot sequence */
    AjPStr    Acc;        /* Accession number of sequence entry  */
    AjPStr    Spr;        /* Swissprot code of sequence entry */
    AjPStr    Typeobj;    /* Bibliographic information ... objective*/ 
    AjPStr    Typesbj;    /* Bibliographic information ... subjective */ 
    AjPStr    Model;      /* String for model type (HMM, Gribskov etc) */
    AjPStr    Group;      /* 'REDUNDANT' or 'NON_REDUNDANT' */
    ajint     Rank;       /* Rank order of hit */	
    float     Score;      /* Score of hit */
    float     Eval;       /* E-value of hit */
    float     Pval;       /* p-value of hit */
    AjPStr    Alg;        /* Alignment, e.g. of a signature to the sequence */
    AjBool    Target;     /* True if the Scophit is targetted for removal from 
			     a list of Scophit objects */
    AjBool    Target2;    /* Also used for garbage collection */
    AjBool    Priority;   /* True if the Scop hit is high priority. */
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
  AjPStr Acc;           /* Accession number of sequence entry  */
  AjPStr    Spr;        /* Swissprot code of sequence entry */
  AjPStr    Typeobj;    /* Primary classification of hit - objective*/
  AjPStr    Typesbj;    /* Secondary classification of hit */
  AjPStr    Model;      /* String for model type (HMM, Gribskov etc) */
  AjPStr    Alg;        /* Alignment, e.g. of a signature to the sequence */
  AjPStr    Group;      /* 'REDUNDANT' or 'NON_REDUNDANT' */
  ajint     Rank;       /* Rank order of hit */	
  float     Score;      /* Score of hit */
  float     Eval;       /* E-value of hit */
  float     Pval;       /* p-value of hit */
  AjBool    Target;     /* True if the Scophit is targetted for removal from 
			     a list of Scophit objects */
  AjBool    Target2;    /* Also used for garbage collection */
  AjBool    Priority;   /* True if the Scop hit is high priority. */
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
    ajint    Sunid_Family;        /* SCOP sunid for family */
    AjBool  Priority;   /* True if the Hitlist is high priority. */
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
  ajint        Mod;       /*Model number*/
  ajint        Chn;       /*Chain number*/
  ajint        Gpn;       /*Group number*/
  char       Type;        /*'P' (protein atom), 'H' ("heterogens") or 'w' 
			    (water)*/
  ajint        Idx;       /*Residue number - index into sequence*/
  AjPStr     Pdb;         /*Residue number - according to original PDB file*/
  char       Id1;         /*Standard residue identifier or 'X' for unknown 
			    types or '.' for heterogens and water*/
  AjPStr     Id3;         /*Residue or group identifier*/
  AjPStr     Atm;         /*Atom identifier*/
  float      X;           /*X coordinate*/
  float      Y;           /*Y coordinate*/
  float      Z;           /*Z coordinate*/
  float      O;           /*Occupancy */
  float      B;           /*B value thermal factor*/
  float      Phi;        /*18:Phi angle*/
  float      Psi;        /*19:Psi angle*/
  float      Area;       /*20:Residue solvent accessible area*/
  
  /* Secondary structure-specific variables from the PDB file*/
  ajint      eNum;        /* Serial number of the element */
  AjPStr     eId;         /* Element identifier */
  char       eType;       /* Element type COIL ('C'), HELIX ('H'), SHEET ('E') or TURN ('T'). Has a default value of COIL. */
  ajint      eClass;      /* Class of helix, an int from 1-10,  from 
			     http://www.rcsb.org/pdb/docs/format/pdbguide2.2/guide2.2_frame.html (see below)*/

  /* Variables for data derived from stride */
  ajint      eStrideNum;  /* Number of the element (sequential count from N-term) */
  char       eStrideType; /* 8: Element type:  ALPHA HELIX ('H'),3-10 HELIX ('G')
			     ,PI-HELIX ('I'), EXTENDED CONFORMATION ('E'), 
			     ISOLATED BRIDGE ('B' or 'b'), TURN ('T') or COIL 
			     (none of the above) ('C'). (from STRIDE)*/

  /* Variables for data derived from naccess */
  float      all_abs;     /* Absolute accessibility, all atoms */
  float      all_rel;     /* Relative accessibility, all atoms */
  float      side_abs;    /* Absolute accessibility, atoms in sidechain */
  float      side_rel;    /* Relative accessibility, atoms in sidechain */
  float      main_abs;    /* Absolute accessibility, atoms in mainchain */
  float      main_rel;    /* Relative accessibility, atoms in mainchain */
  float      npol_abs;    /* Absolute accessibility, nonpolar atoms */
  float      npol_rel;    /* Relative accessibility, nonpolar atoms */
  float      pol_abs;     /* Absolute accessibility, polar atoms */
  float      pol_rel;     /* Relative accessibility, polar atoms */

} AjOAtom, *AjPAtom;


/* @data AjPChain ***********************************************************
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
  ajint        Nlig;       /*No. of groups which are non-covalently associated 
			   with the chain, excluding water ("heterogens")*/

  ajint    numHelices;   /* No. of helices in the chain according to the PDB file*/
  ajint   numStrands;   /* No. of strands in the chain according to the PDB file */

  AjPStr     Seq;	 /* sequence as string */
  AjPList    Atoms;      /*List of Atom objects for (potentially multiple models)
			  of the polypeptide chain and any groups (ligands) that 
			  could be uniquely associated with a chain*/
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
  ajint      Ngp;        /* No. groups that could not be uniquely associated with a chain 
			    in the SEQRES records */
  AjPChar    gpid;	   /*Array of chain (group) id's for groups that 
			     could not be uniquely associated with a chain */
  AjPList   Groups;     /*List of Atom objects for groups that could not 
			   be uniquely associated with a chain */
  AjPList   Water;     /*List of Atom objects for water molecules (which can 
			   never be uniquely associated with a chain */
}AjOPdb, *AjPPdb;




/* @data AjPScop *******************************************************
**
** Ajax scop object.
**
** Holds scop database data
**
** The variables have the following meaning:
**
**  AjPStr Entry;          Domain identifer code 
**  AjPStr Pdb;            Corresponding pdb identifier code
**  AjPStr Class;          SCOP class name as an AjPStr 
**  AjPStr Fold;           SCOP fold  name as an AjPStr  
**  AjPStr Superfamily;    SCOP superfamily name as an AjPStr 
**  AjPStr Family;         SCOP family name as an AjPStr 
**  AjPStr Domain;         SCOP domain name as an AjPStr 
**  AjPStr Source;         SCOP source (species) as an AjPStr 
**  ajint    N;            No. chains from which this domain is comprised 
**  char   *Chain;         Chain identifiers 
**  AjPStr *Start;         PDB residue number of first residue in domain 
**  AjPStr *End;           PDB residue number of last residue in domain 
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
    AjPStr Entry;         /* Domain identifer code */
    AjPStr Pdb;           /* Corresponding pdb identifier code*/
    AjPStr Class;         /* SCOP class name as an AjPStr */
    AjPStr Fold;          /* SCOP fold  name as an AjPStr */ 
    AjPStr Superfamily;   /* SCOP superfamily name as an AjPStr */
    AjPStr Family;        /* SCOP family name as an AjPStr */
    AjPStr Domain;        /* SCOP domain name as an AjPStr */
    AjPStr Source;        /* SCOP source (species) as an AjPStr */
    ajint    N;           /* No. chain or segments from which this domain is comprised */
    char   *Chain;        /* Chain identifiers */
    AjPStr *Start;        /* PDB residue number of first residue in domain */
    AjPStr *End;          /* PDB residue number of last residue in domain */

    ajint  Sunid_Class;         /* SCOP sunid for class */
    ajint  Sunid_Fold;          /* SCOP sunid for fold */
    ajint  Sunid_Superfamily;   /* SCOP sunid for superfamily */
    ajint  Sunid_Family;        /* SCOP sunid for family */
    ajint  Sunid_Domain;        /* SCOP sunid for domain */  
    ajint  Sunid_Source;        /* SCOP sunid for species */
    ajint  Sunid_Domdat;        /* SCOP sunid for domain data */

    AjPStr Acc;           /* Accession number of sequence entry  */
    AjPStr Spr;           /* Swissprot code of sequence entry */
    AjPStr SeqPdb;	  /* Sequence (from pdb) as string */
    AjPStr SeqSpr;	  /* Sequence (from swissprot) as string */
    ajint  Startd;      /* Start of sequence relative to full length 
			    swissprot sequence */
    ajint  Endd;        /* End of sequence relative to full length 
			    swissprot sequence */
} AjOScop,*AjPScop;






/* @data AjPScopcla *******************************************************
**
** Ajax scopcla object.
**
** Holds scop database data from raw file (dir.cla.scop.txt from SCOP authors)
**
** The variables have the following meaning:
**
**  AjPStr Entry;          Domain identifer code 
**  AjPStr Pdb;            Corresponding pdb identifier code
**  AjPStr Sccs;           Scop compact classification string
**  ajint  Class;          SCOP sunid for class 
**  ajint  Fold;           SCOP sunid for fold 
**  ajint  Superfamily;    SCOP sunid for superfamily 
**  ajint  Family;         SCOP sunid for family 
**  ajint  Domain;         SCOP sunid for domain   
**  ajint  Source;         SCOP sunid for species 
**  ajint  Domdat;         SCOP sunid for domain data
**  ajint  N;              No. chains from which this domain is comprised 
**  char   *Chain;         Chain identifiers 
**  AjPStr *Start;         PDB residue number of first residue in domain 
**  AjPStr *End;           PDB residue number of last residue in domain 
**
** AjPScopcla is implemented as a pointer to a C data structure.
**
** @alias AjSScopcla
** @alias AjOScopcla
**
** @@
******************************************************************************/

typedef struct AjSScopcla
{
    AjPStr Entry;
    AjPStr Pdb;
    AjPStr Sccs;

    ajint  Class;
    ajint  Fold;
    ajint  Superfamily;
    ajint  Family;  
    ajint  Domain;
    ajint  Source;
    ajint  Domdat;
        
    ajint    N;
    char   *Chain;
    AjPStr *Start;
    AjPStr *End;
} AjOScopcla,*AjPScopcla;



/* @data AjPScopdes *******************************************************
**
** Ajax scopdes object.
**
** Holds scop database data from raw file (dir.des.scop.txt from SCOP authors)
**
** The variables have the following meaning:
**
**  ajint  Sunid;   SCOP sunid for node 
**  AjPStr Type;    Type of node, either 'px' (domain data), 'cl' (class),
**  'cf' (fold), 'sf' (superfamily), 'fa' (family), 'dm' (domain) or 
**  'sp' (species).
**  AjPStr Sccs;    Scop compact classification string
**  AjPStr Entry;   Domain identifer code (or '-' if Type!='px')
**  AjPStr Desc;    Description in english of the node
**
** AjPScopdes is implemented as a pointer to a C data structure.
**
** @alias AjSScopdes
** @alias AjOScopdes
**
** @@
******************************************************************************/

typedef struct AjSScopdes
{
    ajint  Sunid;
    AjPStr Type;
    AjPStr Sccs;
    AjPStr Entry;
    AjPStr Desc;
} AjOScopdes,*AjPScopdes;





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
    ajint    Sunid_Family;        /* SCOP sunid for family */
    ajint      npos;       /*No. of signature positions*/
    AjPSigpos *pos;        /*Array of derived data for puropses of 
			     alignment*/
    AjPSigdat *dat;        /*Array of empirical data*/
} AjOSignature, *AjPSignature;







/* @data AjPHetent ***********************************************************
**
** Ajax Hetent object.
**
** Holds a single entry from a dictionary of heterogen groups.
**
** AjPHetent is implemented as a pointer to a C data structure.
**
** @alias AjSHetent
** @alias AjOHetent
**
** @@
******************************************************************************/
typedef struct AjSHetent
{
    AjPStr   abv;   /* 3-letter abbreviation of heterogen */
    AjPStr   syn;   /* Synonym */
    AjPStr   ful;   /* Full name */
    ajint    cnt;   /* No. of occurences (files) of this heterogen in a directory */
} AjOHetent, *AjPHetent;




/* @data AjPHet ***********************************************************
**
** Ajax Het object.
** Holds a dictionary of heterogen groups.
**
** AjPHet is implemented as a pointer to a C data structure.
**
** @alias AjSHet
** @alias AjOHet
**
** @@
******************************************************************************/
typedef struct AjSHet
{
    ajint         n;        /* Number of entries */
    AjPHetent *entries;  /* Array of entries */
} AjOHet, *AjPHet;




typedef struct AjSCoord
{
    AjPStr   Class;        /* SCOP class */
    AjPStr   Fold;         /* SCOP fold */
    AjPStr   Superfamily;  /* SCOP superfamily */
    AjPStr   Family;       /* SCOP family */
    ajint    Sunid_Family; /* the sun_id for a given SCOP family */
    AjPStr   Model_Type;   /* The type of model used to generate the scores */
    AjPStr   Acc;          /* Accession number of sequence entry  */
    AjPStr   Spr;          /* Swissprot code of sequence entry */
    ajint    x;            /* The score interval */
    ajint    y;            /* Frequency of scores */
} AjOCoord, *AjPCoord;



typedef struct AjSDatapoint
{
    AjPStr    Acc;                      /* Accession number of sequence entry  */
    AjPStr    Spr;                      /* Swissprot code of sequence entry */
    ajint     x;                        /* The score interval */
    ajint     y;                        /* Frequency of scores */
} AjODatapoint, *AjPDatapoint;


typedef struct AjSDiscord
{
    AjPStr        Class;                /* SCOP class */
    AjPStr        Fold;                 /* SCOP fold */
    AjPStr        Superfamily;          /* SCOP superfamily */
    AjPStr        Family;               /* SCOP family */
    ajint         Sunid_Family;         /* the sun_id for a given SCOP family */
    AjPStr        Model_Type;           /* The type of model used to generate the scores */
    ajint         N;                    /* No. of data points */

    AjPDatapoint  *Points;              /* an array of coordinates */
} AjODiscord, *AjPDiscord;








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



/* Waqas funky */
AjBool      ajXyzFunkyRead(AjPFile funky_fptr, AjPList *all_entries);
AjPDbase    ajXyzDbaseNew(ajint n);
void        ajXyzDbaseDel(AjPDbase *ptr);

AjPDbaseEnt ajXyzDbaseEntNew(ajint n);
void        ajXyzDbaseEntDel(AjPDbaseEnt *ptr);

AjPDomConts ajXyzDomContsNew(ajint n);
void        ajXyzDomContsDel(AjPDomConts *ptr);


/*  Ranjeeva stuff */

ajint ajXyzCoordBinSearchScore(float score, AjPCoord *arr, ajint siz);
float ajXyzScoreToPvalue (float score, AjPList list);
float ajXyzPvalueFromDist (float score, AjPList list);
AjBool ajXyzDiscordWrite(AjPFile outf, AjPDiscord thys);
AjBool ajXyzDiscordRead(AjPFile inf, char *delim, AjPDiscord *thys);
void ajXyzDatapointDel(AjPDatapoint *thys);
void ajXyzDiscordDel(AjPDiscord *pthis);
AjPDatapoint ajXyzDatapointNew(void);
AjPDiscord  ajXyzDiscordNew(ajint n);
AjBool ajXyzDiscordToCoords(AjPDiscord dis_cord, AjPList *out);
void ajXyzCoordDel(AjPCoord *pthis);
AjPCoord ajXyzCoordNew(void);

AjBool ajXyzSunidToScopInfo (ajint sunid, AjPStr *family, AjPStr *superfamily, AjPStr *fold, AjPList list);


AjPPdbtosp ajXyzPdbtospNew(ajint n);
void ajXyzPdbtospDel(AjPPdbtosp *thys);
AjBool ajXyzPdbtospRead(AjPFile inf, AjPStr entry, AjPPdbtosp *thys);
AjBool ajXyzPdbtospReadC(AjPFile inf, char *entry, AjPPdbtosp *thys);
AjBool ajXyzPdbtospReadAll(AjPFile inf, AjPList *list);
ajint ajXyzPdbtospBinSearch(AjPStr id, AjPPdbtosp *arr, ajint siz);
ajint ajXyzSortPdbtospPdb(const void *ptr1, const void *ptr2);


AjPAtom       ajXyzAtomNew(void);
void          ajXyzAtomDel(AjPAtom *thys);
AjBool        ajXyzAtomCopy(AjPAtom *to, AjPAtom from);
AjPList       ajXyzAtomListCopy(AjPList ptr);
AjBool        ajXyzInContact(AjPAtom atm1, AjPAtom atm2, float thresh,
			    AjPVdwall vdw);
float         ajXyzAtomDistance(AjPAtom atm1, AjPAtom atm2, AjPVdwall vdw);

AjPChain      ajXyzChainNew(void);
void          ajXyzChainDel(AjPChain *thys);


AjPPdb        ajXyzPdbNew(ajint chains);
void          ajXyzPdbDel(AjPPdb *thys);
AjBool        ajXyzPdbWriteAll(AjPFile errf, AjPFile outf, AjPPdb pdb);
AjBool        ajXyzPdbWriteDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				AjPScop scop); 
AjBool        ajXyzPdbChain(char id, AjPPdb pdb, ajint *chn);
AjBool        ajXyzPdbAtomIndexI(AjPPdb pdb, ajint chn, AjPInt *idx);
AjBool        ajXyzPdbAtomIndexC(AjPPdb pdb, char chn, AjPInt *idx);
AjBool        ajXyzPdbAtomIndexICA(AjPPdb pdb, ajint chn, AjPInt *idx, ajint *nres);
AjBool        ajXyzPdbAtomIndexCCA(AjPPdb pdb, char chn, AjPInt *idx, ajint *nres);
AjBool        ajXyzPdbToSp(AjPStr pdb, AjPStr *spr, AjPList list);
AjBool        ajXyzPdbToAcc(AjPStr pdb, AjPStr *acc, AjPList list);
AjBool      ajXyzPdbToScop(AjPPdb pdb, AjPList list_allscop, 
			   AjPList *list_pdbscopids);
AjBool  ajXyzPdbToIdx(ajint *idx, AjPPdb pdb, AjPStr res, ajint chn);




AjBool        ajXyzCpdbRead(AjPFile inf, AjPPdb *thys);
AjBool        ajXyzCpdbReadFirstModel(AjPFile inf, AjPPdb *thys);
AjBool        ajXyzCpdbReadOld(AjPFile inf, AjPPdb *thys);
AjBool ajXyzCpdbCopy(AjPPdb *to, AjPPdb from);
AjBool        ajXyzCpdbWriteAll(AjPFile out, AjPPdb thys);
AjBool        ajXyzCpdbWriteDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
			      AjPScop scop);
AjBool      ajXyzCpdbListHeterogens(AjPPdb pdb, AjPList *list_heterogens, 
					    AjPInt *siz_heterogens, ajint *nhet, 
					    AjPFile logf );




void ajXyzCathWrite(AjPFile outf, AjPCath ptr);
void ajXyzCathDel(AjPCath *ptr);
AjPCath ajXyzCathNew(ajint NSegment);
AjBool ajXyzCathReadC(AjPFile inf, char *entry, AjPCath *thys);


AjPScop       ajXyzScopNew(ajint n);
void          ajXyzScopDel(AjPScop *pthis);
AjBool        ajXyzScopRead(AjPFile inf, AjPStr entry, AjPScop *thys);
AjBool        ajXyzScopReadC(AjPFile inf, char *entry, AjPScop *thys);
AjBool        ajXyzScopReadAll(AjPFile fptr, AjPList *list); 
void          ajXyzScopWrite(AjPFile outf, AjPScop thys);
AjPStr        ajXyzScopToPdb(AjPStr scop, AjPStr *pdb);
AjBool        ajXyzScopToSp(AjPStr scop, AjPStr *spr, AjPList list);
AjBool        ajXyzScopToAcc(AjPStr scop, AjPStr *acc, AjPList list);
AjBool        ajXyzScopToScophit(AjPScop source, AjPScophit* target);
ajint         ajXyzScopBinSearchScop(AjPStr id, AjPScop *arr, ajint siz);
ajint         ajXyzScopCompId(const void *hit1, const void *hit2);
ajint         ajXyzScopCompPdbId(const void *hit1, const void *hit2);
AjBool        ajXyzScopCopy(AjPScop *to, AjPScop from);
ajint ajXyzScopBinSearchSunid(ajint id, AjPScop *arr, ajint siz);
ajint ajXyzScopCompSunid(const void *entry1, const void *entry2);
AjBool ajXyzScopSeqFromSunid(ajint id, AjPStr *seq, AjPList list);
ajint ajXyzScopCompPdbId(const void *hit1, const void *hit2);
ajint ajXyzScopBinSearchPdb(AjPStr id, AjPScop *arr, ajint siz);



ajint ajXyzCathCompPdbId(const void *hit1, const void *hit2);
ajint ajXyzCathBinSearchPdb(AjPStr id, AjPCath *arr, ajint siz);


AjPScopcla    ajXyzScopclaNew(ajint chains);
void          ajXyzScopclaDel(AjPScopcla *thys);
AjBool        ajXyzScopclaRead(AjPFile inf, AjPStr entry, AjPScopcla *thys);
AjBool        ajXyzScopclaReadC(AjPFile inf, char *entry, AjPScopcla *thys);

AjPScopdes    ajXyzScopdesNew(void);
void          ajXyzScopdesDel(AjPScopdes *ptr);
AjBool        ajXyzScopdesRead(AjPFile inf, AjPStr entry, AjPScopdes *thys);
AjBool        ajXyzScopdesReadC(AjPFile inf, char *entry, AjPScopdes *thys);
ajint ajXyzScopdesBinSearch(ajint id, AjPScopdes *arr, ajint siz);
ajint ajXyzScopdesCompSunid(const void *scop1, const void *scop2);





AjPScophit    ajXyzScophitNew(void);
void          ajXyzScophitDel(AjPScophit *pthis);
void          ajXyzScophitDelWrap(const void  **ptr);
AjBool        ajXyzScophitsToHitlist(AjPList in, AjPHitlist *out,   AjIList *iter);
AjBool ajXyzScophitsWrite(AjPFile outf, AjPList list);
AjBool ajXyzScophitsAccToHitlist(AjPList in, AjPHitlist *out,   AjIList *iter);
AjBool        ajXyzScophitsOverlap(AjPScophit h1, AjPScophit h2, ajint n);
AjBool        ajXyzScophitsOverlapAcc(AjPScophit h1, AjPScophit h2, ajint n);
AjBool        ajXyzScophitCopy(AjPScophit *to, AjPScophit from);
AjPList       ajXyzScophitListCopy(AjPList ptr);
AjBool        ajXyzScophitToHit(AjPHit *to, AjPScophit from);
AjBool        ajXyzScophitCheckTarget(AjPScophit ptr);
AjBool        ajXyzScophitTarget(AjPScophit *h);
AjBool        ajXyzScophitTarget2(AjPScophit *h);
AjBool        ajXyzScophitTargetLowPriority(AjPScophit *h);
AjBool        ajXyzScophitMergeInsertThis(AjPList list, AjPScophit hit1, 
				   AjPScophit hit2,  AjIList iter);
AjBool        ajXyzScophitMergeInsertThisTarget(AjPList list, AjPScophit hit1, 
				   AjPScophit hit2,  AjIList iter);
AjBool ajXyzScophitMergeInsertThisTargetBoth(AjPList list, AjPScophit hit1, 
					     AjPScophit hit2,  AjIList iter);
AjBool        ajXyzScophitMergeInsertOther(AjPList list, AjPScophit hit1, AjPScophit hit2);
AjBool        ajXyzScophitMergeInsertOtherTargetBoth(AjPList list, AjPScophit hit1, AjPScophit hit2);
AjBool        ajXyzScophitMergeInsertOtherTarget(AjPList list, AjPScophit hit1, AjPScophit hit2);
AjPScophit    ajXyzScophitMerge(AjPScophit hit1, AjPScophit hit2);
ajint         ajXyzScophitCompSpr(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompStart(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompEnd(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompFold(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompSfam(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompFam(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompAcc(const void *hit1, const void *hit2);
ajint         ajXyzScophitCompSunid(const void *entry1, const void *entry2);
ajint         ajXyzScophitCompScore(const void *hit1, const void *hit2); 

AjPHit        ajXyzHitNew(void);
void          ajXyzHitDel(AjPHit *pthis);
AjPHit        ajXyzHitMerge(AjPHit hit1, AjPHit hit2);
AjBool        ajXyzHitsOverlap(AjPHit h1, AjPHit h2, ajint n);
ajint         ajXyzCompScore(const void *hit1, const void *hit2);
ajint         ajXyzCompScoreInv(const void *hit1, const void *hit2);
ajint         ajXyzCompId(const void *hit1, const void *hit2);

AjPHitlist    ajXyzHitlistNew(ajint n);
void          ajXyzHitlistDel(AjPHitlist *pthis);
AjBool        ajXyzHitlistRead(AjPFile inf, char *delim, AjPHitlist *thys);
AjBool        ajXyzHitlistReadNode(AjPFile scopf, AjPList *list, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class);
AjBool        ajXyzHitlistReadFam(AjPFile scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class, AjPList* list);
AjBool        ajXyzHitlistReadSfam(AjPFile scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class,AjPList* list);
AjBool        ajXyzHitlistReadFold(AjPFile scopf, AjPStr fam, AjPStr sfam, AjPStr fold, AjPStr class,AjPList* list);
AjBool        ajXyzHitlistWrite(AjPFile outf, AjPHitlist thys);
AjBool ajXyzHitlistWriteSubset(AjPFile outf, AjPHitlist thys, AjPInt ok);

AjBool        ajXyzHitlistToScophits(AjPList in, AjPList *out);
AjBool        ajXyzHitlistClassify(AjPHitlist *hits, AjPList targets, 
				   ajint thresh);
AjBool        ajXyzHitlistPriorityHigh(AjPHitlist *list);
AjBool        ajXyzHitlistPriorityLow(AjPHitlist *list);
AjBool        ajXyzHitlistToThreeScophits(AjPList in, AjPList *fam, AjPList *sfam, AjPList *fold);

AjBool        ajXyzHitlistsWriteFasta(AjPList *list, AjPFile *outf);
ajint         ajXyzHitlistCompFold(const void *hit1, const void *hit2);



AjBool        ajXyzPrintPdbSeqresChain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				  ajint chn);
AjBool        ajXyzPrintPdbSeqresDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				   AjPScop scop);
AjBool        ajXyzPrintPdbAtomChain(AjPFile outf, AjPPdb pdb, ajint mod, ajint chn);
AjBool        ajXyzPrintPdbAtomDomain(AjPFile errf, AjPFile outf, AjPPdb pdb,
				 AjPScop scop, ajint mod);
AjBool        ajXyzPrintPdbHeterogen(AjPFile outf, AjPPdb pdb, ajint mod);
AjBool        ajXyzPrintPdbText(AjPFile outf, AjPStr str, char *prefix);
AjBool        ajXyzPrintPdbHeader(AjPFile outf, AjPPdb pdb);
AjBool        ajXyzPrintPdbHeaderScop(AjPFile outf, AjPScop scop);
AjBool        ajXyzPrintPdbTitle(AjPFile outf, AjPPdb pdb);
AjBool        ajXyzPrintPdbCompnd(AjPFile outf, AjPPdb pdb);
AjBool        ajXyzPrintPdbSource(AjPFile outf, AjPPdb pdb);
AjBool        ajXyzPrintPdbEmptyRemark(AjPFile outf, AjPPdb pdb);
AjBool        ajXyzPrintPdbResolution(AjPFile outf, AjPPdb pdb);


AjBool        ajXyzScopalgRead(AjPFile inf, AjPScopalg *thys);
AjBool        ajXyzScopalgWrite(AjPFile outf, AjPScopalg scop);
AjBool        ajXyzScopalgWriteClustal(AjPScopalg align, AjPFile* outf);
AjBool        ajXyzScopalgWriteClustal2(AjPScopalg align, AjPFile* outf);
AjPScopalg    ajXyzScopalgNew(ajint n);
void          ajXyzScopalgDel(AjPScopalg *pthis);
ajint         ajXyzScopalgGetseqs(AjPScopalg thys, AjPStr **arr);
AjBool ajXyzScopalgToScop(AjPScopalg align, AjPScop *scop_arr, ajint dim, AjPList* list);

AjPCmap       ajXyzCmapNew(ajint dim);
void          ajXyzCmapDel(AjPCmap *pthis);
AjBool        ajXyzCmapRead(AjPFile inf, ajint mode, ajint chn, ajint mod, AjPCmap *thys);
AjBool        ajXyzCmapReadC(AjPFile inf, char chn, ajint mod, AjPCmap *thys);
AjBool        ajXyzCmapReadI(AjPFile inf, ajint chn, ajint mod, AjPCmap *thys);


float         ajXyzVdwRad(AjPAtom atm, AjPVdwall vdw);
 
AjPVdwall     ajXyzVdwallNew(ajint n);
void          ajXyzVdwallDel(AjPVdwall *pthis);
AjBool        ajXyzVdwallRead(AjPFile inf, AjPVdwall *thys);


AjPVdwres     ajXyzVdwresNew(ajint n);
void          ajXyzVdwresDel(AjPVdwres *pthis);


AjPScorealg   ajXyzScorealgNew(ajint len);
void          ajXyzScorealgDel(AjPScorealg *pthis);


AjPSigdat     ajXyzSigdatNew(ajint nres, ajint ngap);
AjPSigpos     ajXyzSigposNew(ajint ngap);


void          ajXyzSigdatDel(AjPSigdat *pthis);
void          ajXyzSigposDel(AjPSigpos *thys);


AjPHitidx     ajXyzHitidxNew(void);
void          ajXyzHitidxDel(AjPHitidx *pthis);
ajint         ajXyzHitidxBinSearch(AjPStr id, AjPHitidx *arr, ajint siz);


AjPSignature  ajXyzSignatureNew(ajint n);
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
				      AjPHitlist hits, ajint n);
AjBool        ajXyzSignatureAlignWrite(AjPFile outf, AjPSignature sig, 
				       AjPHitlist hits);
AjBool        ajXyzSignatureAlignWriteBlock(AjPFile outf, AjPSignature sig, 
				       AjPHitlist hits);


AjPHetent     ajXyzHetentNew(void);
void          ajXyzHetentDel(AjPHetent *ptr);

AjPHet        ajXyzHetNew(ajint n);
void          ajXyzHetDel(AjPHet *ptr);
AjBool        ajXyzHetRawRead(AjPFile fptr, AjPHet *ptr);
AjBool        ajXyzHetRead(AjPFile fptr, AjPHet *ptr);
AjBool        ajXyzHetWrite(AjPFile fptr, AjPHet ptr, AjBool dogrep);


ajint StrBinSearchScop(AjPStr id, AjPStr *arr, ajint siz);
ajint StrComp(const void *str1, const void *str2);




#endif

#ifdef __cplusplus
}
#endif




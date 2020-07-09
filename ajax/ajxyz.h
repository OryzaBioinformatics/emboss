#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajxyz_h
#define ajxyz_h

#define ajXRAY  0    /* Structure was determined by X-ray crystallography */
#define ajNMR   1    /* Structure was determined by NMR or is a model     */


#define ajESCOP "Escop.dat" /* Scop data file */


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
  char       Id1;        /*Standard residue identifier or '?' for unknown 
			   types or '.' for heterogens and water*/
  AjPStr     Id3;        /*Residue or group identifier*/
  AjPStr     Atm;        /*Atom identifier*/
  float      X;          /*X coordinate*/
  float      Y;          /*Y coordinate*/
  float      Z;          /*Z coordinate*/
  float      O;          /*Occupancy */
  float      B;          /*B value thermal factor*/
} AjSAtom, *AjPAtom;


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




AjPAtom  ajXyzAtomNew(void);
void     ajXyzAtomDel(AjPAtom *thys);
AjPChain ajXyzChainNew(void);
void     ajXyzChainDel(AjPChain *thys);
AjPPdb   ajXyzPdbNew(ajint chains);
void     ajXyzPdbDel(AjPPdb *thys);
void     ajXyzScopDel(AjPScop *pthis);
AjPScop  ajXyzScopNew(ajint n);

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
AjBool   ajXyzPrintPdbText(AjPFile outf, AjPStr str, char *prefix, ajint len,
			   char *delim);
AjBool   ajXyzPrintPdbHeader(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbHeaderScop(AjPFile outf, AjPScop scop);
AjBool   ajXyzPrintPdbTitle(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbCompnd(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbSource(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbEmptyRemark(AjPFile outf, AjPPdb pdb);
AjBool   ajXyzPrintPdbResolution(AjPFile outf, AjPPdb pdb);

AjBool   ajXyzPdbChain(char id, AjPPdb pdb, ajint *chn);
void     ajXyzScopToPdb(AjPStr scop, AjPStr *pdb);

#endif

#ifdef __cplusplus
}
#endif

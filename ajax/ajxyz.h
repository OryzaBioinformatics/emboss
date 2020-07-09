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
  int        Mod;        /*Model number*/
  int        Chn;        /*Chain number*/
  char       Type;       /*'P' (protein atom), 'H' ("heterogens") or 'w' 
			   (water)*/
  int        Idx;        /*Residue number - index into sequence*/
  int        Pdb;        /*Residue number - according to original PDB file*/
  char       Id1;        /*Standard residue identifier or '?' for unknown 
			   types or '.' for heterogens and water*/
  AjPStr     Id3;        /*Residue or group identifier*/
  AjPStr     Atm;        /*Atom identifier*/
  float      X;          /*X coordinate*/
  float      Y;          /*Y coordinate*/
  float      Z;          /*Z coordinate*/
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
  int        Nres;       /*No. of amino acid residues*/
  int        Nhet;       /*No. of atoms which are non-covalently associated 
			   with the chain, excluding water ("heterogens")*/
  int        Nwat;       /*No. of water atoms which are associated with the 
			   chain*/
  AjPStr     Seq;	 /* sequence as string */
  AjPList    Atoms;      /*List of Atoms */
  AjPInt     Models;     /*Offsets to the first first atom of each model*/
                         /*in the chain. */
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
  int        Method;     /*Exp. type, value is either XRAY or NMR*/
  float      Reso;       /*Resolution of an XRAY structure or 0*/
  int        Nmod;       /*No. of models (always 1 for XRAY structures)*/
  int        Nchn;       /*No. polypeptide chains */
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
    AjPStr Db;
    AjPStr Class;
    AjPStr Fold;
    AjPStr Superfamily;
    AjPStr Family;
    AjPStr Domain;
    AjPStr Source;
    int    N;
    AjPStr *Chain;
    int    *Start;
    int    *End;
    AjPFile Fp;
} AjOScop,*AjPScop;




AjPAtom  ajAtomNew(void);
void     ajAtomDel(AjPAtom *thys);
AjPChain ajChainNew(int models);
void     ajChainDel(AjPChain *thys);
AjBool   ajCpdbRead(AjPStr name, AjPPdb *thys);
AjBool   ajCpdbWriteAll(AjPFile out, AjPPdb thys);
AjPPdb   ajPdbNew(int chains, int models);
void     ajPdbDel(AjPPdb *thys);
void     ajScopDel(AjPScop *pthis);
AjPScop  ajScopNew(int n);
AjBool   ajScopRead(AjPFile inf, AjPStr entry, AjPScop *thys);
AjBool   ajScopReadC(AjPFile inf, char *entry, AjPScop *thys);
void     ajScopWrite(AjPFile outf, AjPScop thys);


#endif

#ifdef __cplusplus
}
#endif

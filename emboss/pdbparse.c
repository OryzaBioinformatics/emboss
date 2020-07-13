/* @Source pdbparse application
**
** Parses pdb files and writes cleaned-up protein coordinate files.
**
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
** @@
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** 
** 
** 
******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
******************************************************************************
**
** Mon May 20 11:43:39 BST 2002
**
** The following documentation is out-of-date and should be disregarded.  It 
** will be updated shortly. 
** 
******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
******************************************************************************
** 
** Operation
** 
** pdbparse parses pdb files in a given directory and writes files of clean 
** coordinate data in embl-like format.
**
** Known limitations
** pdbparse is not guaranteed to work correctly (or even at all) for the 
** theoreticalcase where an NMR structure contains multiple models but the 
** models have different sequence of residues due to errors.
** 
** Pdbparse will not work in cases where a residue number is
** duplicated AND an alternative residue numbering system is used
** somewhere else in the same chain.  If such cases exist they could
** be parsed by having a variable corresponding to oddnum, but just
** for duplicate residue positions. The new variable would get writtne
** in the same place as oddnum is written.
**
** 
** Preliminary documentation
** DE :  Text from COMPND records in PDB file
** OS : Text from SOURCE records in PDB file
** 
** EX line:
** METHOD  Exp. type, value is always either xray or nmr_or_model
** RESO: Resolution of an XRAY structure or 0
** NMOD:  No. of models (always 1 for XRAY structures)
** NCHN:  No. polypeptide chains 
** NGRP:      No. groups (non-covalently bound species) that 
**                     could not be uniquely associated with a chain 
**                    in the SEQRES records 
** CN  [1]   Start of data for chain 1
** 
** IN line:
** ID:   Chain id, ('.' if one wasn't specified in the 
**           original PDB file)
** NRES:   No. of amino acid residues
** NLIG:   No. of groups which are non-covalently associated 
**       with the chain ("heterogens") excluding water
** NH:  No. of helices in the chain 
** NE:    No. of strands in the chain 
** NS:     No. of sheets in the chain 
** NT:     No. of turns in the chain 
** SQ:    sequence as string 
** 
** CO line:
** 1)  'CO' is always given
** 2)   Model number
** 3)   Chain number
** 4)   Group number
** 5)   Type, either  'P' (protein atom), 'H' ("heterogens") or 'w' (water)
** 6)  Residue number - index into sequence
** 7) Residue number - according to original PDB file
** 8)  Element type, one of 'C' (COIL), 'H' (HELIX), 'E' (SHEET)
**     or 'T' (TURN). 
** 9)  Serial number of the element (or '.' for coils)
** 10)  Serial number of the element (or '.' for coils)
** 11)  Element identifier (or '.' for coils)
** 12)   Class of helix, an int from 1-10 (or '.' for coils)
** 13 Single letter residue identifier or 'X' for unknown 
**       types or '.' for heterogens and water
** 14)  Residue or group identifier
** 15)  Atom identifier
** 16) X coordinate
** 17) Y coordinate
** 18) Z coordinate
** 19)  Occupancy 
** 20)  B value thermal factor
**
**
** Description of diagnostic messages
** 
** FILE_OPEN      my.file      
** my.file could not be opened for reading or writing (the file is ignored)
**
** FILE_READ      my.file      
** my.file could not be read (the file is ignored)
**
** NO_OUTPUT      my.file
** No clean coordinate file was generated for my.file
** This will happen if there was a FILE_READ error on the raw pdb file, or a 
** NOSEQRES, NOATOM or NOPROTEINS error when reading the file.
** 
** FILE_WRITE     my.file      
** my.file could not be written (the file is ignored)
**
** DUPATOMRES     3            
** The file contained duplicate ATOM residues for atoms or whole residues, 
** first instance on line 3 (this message is reported once only. Lines for 
** duplicate atoms are discarded).
**
** NOATOMRESID    123          
** No atom or residue identifier specified, first instance on line 123 (this 
** message is reported once only. All such lines are discarded).
**
** SEQRESLEN      1 (A)        
** No indicated length of SEQRES sequence could be found for chain 1 with 
** identifier 'A' (the actual length of the sequence is used).
**
** SEQRESLENDIF   1 (A)        
** Indicated and actual length of SEQRES sequence differs for chain 1 with 
** identifier 'A' (the actual length of the sequence is used).
**
** SECMISS        123          
** One or more records were missing for a secondary structure element on line 
** 123. This message may appear more than once (the element(s) are discarded).
**
** SECBOTH        1 2 ALA 2 ARG 6  
** The start and end residues (ALA 2 ARG 6) of a secondary structure element 
** as indicated in the HELIX, SHEET or TURN records was not found in the ATOM 
** records of chain 1, model 2 (the element is discarded).
**
** SECSTART       1 2 ALA 2        
** The start residue (ALA 2) of a secondary structure element as indicated in
** the HELIX, SHEET or TURN records was not found in the ATOM records of 
** chain 1, model 2 (the element is discarded).
**
** SECEND         1 2 ARG 6        
** The end residue (ARG 6) of a secondary structure element as indicated in 
** the HELIX, SHEET or TURN records was not found in the ATOM records of
** chain 1, model 2 (the element is discarded).
**
** SECCHAIN       A            
** Chain A indicated for secondary structure element record not found in PDB 
** file (the element is discarded).
**
** SECTWOCHN      A   B          
** 2 chain identifiers (A and B) indicated for a secondary structure element
** (the element is discarded).
**
** NOCOMPND                    
** COMPND records not found
**
** NOSOURCE                    
** SOURCE records not found
**
** NOMODEL                     
** NMR structure with no MODEL records
**
** RESOLMOD
** A value for the RESOLUTION record is given but MODEL records are also 
** found (an NMR structure or model is presumed).
**
** RESOLUNK
** Value for RESOLUTION record not found (the value of 0 is used).
**
** NORESOLUTION                
** RESOLUTION record not found (an NMR structure or model is presumed).
**
** NOSEQRES                    
** No SEQRES records (the file is not parsed).
**
** NOATOM                      
** No ATOM records (the file is not parsed).
**
** CHAINIDS       1 (A) 2 (A)  
** Chain identifiers of chains 1 and 2 not unique; 'A' in both cases 
** (the chains are discarded).
**
** CHAINIDSPC                  
** Space (' ') and non-space character both used for chain ids.
**
** SEQRESNOAA     1 (A)        
** No known amino acids in SEQRES records for chain 1 ('A') (the chain is 
** discarded).
**
** SEQRESFEWAA    1 (A)        
** Fewer than the user-specified minimum number of known amino acids in SEQRES 
** records for chain 1 ('A') (the chain is discarded).
**
** NOPROTEINS                  
** The file did not contain any chains with at least the user-specified 
** minimum number of known amino acids (the file is not parsed).
**
** TERNONE                     
** TER records not found.
**
** TERTOOMANY                  
** Number of TER records is greater than the number of chains; possible digest.
**
** TERTOOFEW                   
** Number of TER records is less than the number of chains.
**
** MODELDUP       123          
** Duplicate MODEL records on line 123 
**
** TERMISS        123 124      
** Different chains or heterogen groups not separated by TER record between 
** lines 123 and 124
**
** TERMISSHET     123 124      
** A chain is not seperated from its heterogen group by a TER record between 
** lines 123 and 124
**
** TERMISSCHN     123 124      
** Two chains are not seperated by TER records between lines 123 and 124
**
** ATOMCOL12      123          
** Column 12 used for atom type, first instance on line 123 (this message is 
** reported once only).
**
** CHAINORDER     123          
** Chain order in ATOM records is inconsistent with order in SEQRES records, 
** for the ATOM record on line 123.
**
** ATOMONEONLY    123          
** Amino acid residue in protein chain with single atom only on line 123 
** (the atom is discarded if masking is specified by the user).
**
** ATOMNOCA       123 132      
** Amino acid residue with no CA atom whose ATOM records span lines 123 - 132 
** (the atoms are discarded if masking is specified by the user).
**
** ATOMNONUM      123 
** Residue number for atom on line 123 did not have an integer component.
**
** ATOMFEWAA      1 (A) 3      
** Fewer than the user-specified minimum number of known amino acids in ATOM 
** records for chain 1 ('A'), model 3 (the chain is discarded for all models).
**
** BADINDEX       1 (A)        
** Raw residue numbering from ATOM records does not give index into SEQRES 
** sequence.
**
** NEGNUM         1 (A) 123    
** Negative residue number found for chain 1 ('A') on line 123 (this message 
** is reported once only).
**
** ZERNUM         1 (A) 123    
** Residue number of zero found for chain 1 ('A') on line 123 (this message 
** is reported once only).
**
** ODDNUM         1 (A) 123    
** Possible heterogeneity or alternative residue numbering scheme for chain 1 
** ('A') on line 123 (this message is reported once only).
**
** NONSQNTL       1 (A) 123    
** Possible case of non-sequential numbering error for chain1 ('A') on line 
** 123 (this message is reported once only).
**
** HETEROK        1 (A)        
** Alignment of ATOM to SEQRES sequence for chain 1 ('A') found by presuming 
** an alternative residue numbering scheme.
**
** ALTERNOK       1 (A)        
** Alignment of ATOM to SEQRES sequence for chain 1 ('A') found by presuming 
** one or more instances of heterogeneity.
**
** MISSNTERM      1 (A) 3      
** SEQRES records appeared to be missing 3 N-terminal residues relative to 
** ATOM sequence for chain 1 ('A').
**
** MISSCTERM      1 (A) 3      
** SEQRES records appeared to be missing 3 C-terminal residues relative to 
** ATOM sequence for chain 1 ('A').
**
** NOMATCH        1 (A)        
** Correspondence between ATOM/SEQRES sequences could not be found for chain 1 
** ('A') (the raw sequence from the ATOM records is taken to be the true 
** sequence).
**
** GAPPED         1 (A) 2 ALA 2 ARG 6;    ALA 12 TYR 16        
** Residue numbering for chain 1 ('A') was incorrect but was correctd by gap 
** insertion but contained 2 cases of mismatch between ATOM and SEQRES
** residues  (ALA 2 vs ARG 6and ALA 12 vs TYR 16).
**
** GAPPEDOK       1 (A)       
** Residue numbering for chain 1 ('A') was incorrect but was correctd by gap 
** insertion with no mismatches.
**
** MISMATCH         1 (A) 2 ALA 2 ARG 6;    ALA 12 TYR 16        
** Residue numbering for chain 1 ('A') was correct but but contained 2 cases
** of  mismatch between ATOM and SEQRES residues (ALA 2 vs ARG 6and ALA 12
** vs TYR 16).
**
**
** Important Notes
** pdbparse necessarily must hold the entire pdb file and some derived data
** in memory. If an error of the type 'Uncaught exception:  Allocation failed, 
** insufficient memory available' is raised then this is probably because
** the memory requirements exceed per-user memory defaults (that are usually 
** set quite low). This can easily be unlimited by the sysop in the login 
** process. If tcsh is used, then simply type 'unlimit' before pdbparse is run.
**
******************************************************************************/





#include "emboss.h"

#define POS_CHID        21  /*Position in ATOM line of chain id.*/

#define PDBPARSE_IGNORE   0   /*Ignore this line when parsing coordinates*/  
#define PDBPARSE_COORD    1   /*Coordinate line */
#define PDBPARSE_COORDHET 2   /*Coordinate line for non-protein atoms */
#define PDBPARSE_TER      3   /*TER record */
#define PDBPARSE_MODEL    4   /*MODEL record */ 
#define PDBPARSE_COORDGP  5   /*Coordinate line for groups that could not be
			      associated with a SEQRES chain*/
#define PDBPARSE_COORDWAT 6   /*Coordinate line for water */
#define PDBPARSE_ENDMDL   7   /*ENDMDL record */ 

#define MAXMISSNTERM    3   /*A number of residues may be missing from the 
			      N-terminus of the SEQRES records relative to the 
			      ATOM records (e.g. MET and ACE often do not 
			      appear).  The parser will search and correct for 
			      such cases. MAXMISSNTERM is the maximum number 
			      of such missing residues that can be accounted 
			      for. */
#define COIL            0
#define HELIX           1
#define SHEET           2
#define TURN            3






/* @data AjPElement *******************************************************
**
** Ajax element object.
**
** Object for holding a single secondary structure element for parsing
**
** AjPElement is implemented as a pointer to a C data structure.
**
** @alias AjSElement
** @alias AjOElement
**
** The variables have the following meaning (column numbers refer to the pdb 
** file):
**
** elementNum;    Serial number of the element (columns 8 - 10) 
**
** elementId;     Element identifier (columns 12 - 14) 
**
** elementType;   Element type COIL ('C'), HELIX ('H'), SHEET ('E')
**                or TURN ('T') 
**
** initResName;   Name of first residue in each element (columns 
**	    	  16 - 18 (HELIX & TURN) or 18 - 20 (SHEET) ) 
**
** initSeqNum;    Residue number (including insertion code) of first 
**		  residue in each element (columns 22 - 26 (HELIX), 23 - 
**                27 (SHEET) or 21 - 25 (TURN) )
**
** endResName;    Name of last residue in each element (columns 28 - 30 
**                (HELIX), 29 - 31 (SHEET) or  27 - 29 (TURN) ) 
**
** endSeqNum;     Residue number (including insertion code) of 
**		  last residue in each element (columns 34 - 38 (HELIX and 
**                SHEET) or 32 - 36 (TURN)  )
**
** chainId;       Chain identifiers for chains containing the elements (column 
**                20 (HELIX & TURN) or 22 (SHEET) )
**
** helixClass;    Classes of helices (columns 39 - 40), an int from 1-10 from
**	   http://www.rcsb.org/pdb/docs/format/pdbguide2.2/guide2.2_frame.html 
** 
** TYPE OF HELIX             CLASS NUMBER (COLUMNS 39 - 40)
** --------------------------------------------------------------
** Right-handed alpha (default)                1
** Right-handed omega                          2
** Right-handed pi                             3
** Right-handed gamma                          4
** Right-handed 310                            5
** Left-handed alpha                           6
** Left-handed omega                           7
** Left-handed gamma                           8
** 27 ribbon/helix                             9
** Polyproline                                10
**
** @@
******************************************************************************/
typedef struct AjSElement
{
    ajint    elementNum; 
    AjPStr   elementId;  
    char     elementType; 
    AjPStr   initResName;     
				
    AjPStr   initSeqNum;    
    AjPStr   endResName;    
    AjPStr   endSeqNum;     
    char     chainId;       
    ajint    helixClass;    
}AjOElement, *AjPElement;





/* @data AjPElements **********************************************************
**
** Ajax elements object.
**
** Object for holding secondary structure elements from a PDB file
**
** AjPElements is implemented as a pointer to a C data structure.
**
** @alias AjSElements
** @alias AjOElements
**
** The variables have the following meaning:
**
**   n;     Total no. of secondary structure elements (helices, strands or 
**           turns) 
**  *elms;  Array of element objects 
** @@
*****************************************************************************/
typedef struct AjSElements
{
    ajint      n;            
    AjPElement *elms;        
}
AjOElements, *AjPElements;





/* @data AjPPdbfile ***********************************************************
**
** Ajax pdbfile object.
**
** Holds a pdb file for parsing.
**
** AjPPdbfile is implemented as a pointer to a C data structure.
**
** @alias AjSPdbfile
** @alias AjOPdbfile
**
**
** The variables have the following meaning:
**
** Following are for each pdb file
**  pdbid;       4 character pdb id code 
**  tercnt;      The number of TER records in the pdb file 
**  toofewter;   True if the file contained too few TER records 
**  modcnt;      The number of MODEL records in the pdb file (does not count
**	         duplicate MODEL records that are masked out)
**  nomod;       True if the file contained no MODEL records 
**  compnd;      Text from COMPND records 
**  source;      Text from SOURCE records 
**  reso;        Resolution of structure 
**  method;      Structural method,  either ajXRAY or ajNMR 
**  ngroups;     Number of groups (non-protein groups that could not be
**	         associated with a chain in the SEQRES section 
**  gpid;        Array of chain (group) id's for groups that cannot be 
**	         associated with a chain in the SEQRES section 
**  idxfirst;    Index in <lines> of first ATOM, HETATM or MODEL line 
**
** Following are for each chain
**  nchains;     Number of chains (from SEQRES record) 
** *seqres;      Array of sequences taken from the SEQRES records
** *seqresful;   Array of sequences using 3 letter codes taken from the 
**               SEQRES records
** *nres;        Number of residues in each chain 
** *chainok;     Array of flags which are True if a chain in the SEQRES
**	         record contains >= minimum no. of amino acids and has a 
**	         unique chain identifier
** *resn1ok;     Bool's for each chain which are TRUE if resn1 was 
**	         used to derive resni, i.e. gave correct alignment to 
**	         seqres sequence.  If False then resn2 was used. 
** *nligands;    Number of ligands for each chain 
** *numHelices;  No. of helices in each chain 
** *numStrands;  No. of strands in each chain 
** *numSheets;   No. of sheets in each chain 
** *numTurns;    No. of turns in each chain 
**  chid;        Array of chain id's for chains from SEQRES records
**
** 
** Following are for each line in the pdb file
**  nlines;    Number of lines in the pdb file 
** *lines;     Array of lines in the pdb file 
** *linetype;   Array of int's describing the lines, have values
**	       of PDBPARSE_IGNORE (do not consider this line when parsing 
**	       coordinates from the file), PDBPARSE_COORD (coordinate line
**	       (ATOM or HETATM record) for protein atoms, PDBPARSE_COORDHET
**             (coordinate line for non-protein atoms), PDBPARSE_COORDGP 
**             (oordinate line for groups that could not be associated 
**             with a SEQRES chain), PDBPARSE_COORDWAT (oordinate line for 
**             water), PDBPARSE_TER (it is a TER record) or PDBPARSE_MODEL 
**             (it is a MODEL record).
** *chnn;      Array of chain numbers for each PDBPARSE_COORD &
**             PDBPARSE_COORDHET
**             line.
** *gpn;       Array of group numbers for each line. Each group 
**	       (heterogen) is given a group number, that is either 
**	       relative to a chain or the whole file (for groups that could 
**             not be associated with a chain from the SEQRES records) 
** *modn;      Array of model numbers for each PDBPARSE_COORD line 
**
**
** *resni;     Residue numbers for each PDBPARSE_COORD line.
**	       These give the correct index into the <seqres> sequences 
** *resn1;     Array of residue numbers for each PDBPARSE_COORD line.
**	       This is pdbn converted to a sequential integer where 
**	       alternative residue numbering is presumed for lines 
**	       where line[26] is used (residues for which  oddnum==True
**	       are considered).
** *resn2;        Array of residue numbers for each PDBPARSE_COORD line.
**	       This is pdbn converted to a sequential integer where 
**	       heterogeneity is presumed for lines where line[26] is 
**	       used (residues where oddnum==True are ignored).
** *pdbn;      Array with a residue number for each line
**	       for which <cooord> == ajTrue. This is the original residue
**	       number string (including insertion code) from the pdb file
** *oddnum;    Bool's for each line which are TRUE for duplicate residues of 
**	       heterogenous positions (e.g. if 2 different residues
**	       are both numbered '8' or one is '8' and the other '8A'
**    	       for example then <oddnum> would be set True for the second
**	       residue. Heterogeneity is indicated by a character in 
**             position lines[26] (the same position used to indicate 
**	       alternative residue numbering schemes).    
**  *atype;    Atom type for each line 
**  *rtype;    Residue type for each line 
**  *x;        x-coordinate for each line 
**  *y;        y-coordinate for each line 
**  *z;        z-coordinate for each line 
**  *o;        occupancy for each line 
**  *b;        thermal factor for each line 
**
**  *elementNum;   Serial number of the secondary structure element (columns 
**                 8 - 10) 
**  *elementId;    Secondary structure element identifier (columns 12 - 14) 
**  *elementType;  Secondary structure element type COIL ('C'), HELIX ('H'), 
**                 SHEET ('E') or TURN ('T') 
**  *helixClass;   Classes of helices (columns 39 - 40)  from 
**	   http://www.rcsb.org/pdb/docs/format/pdbguide2.2/guide2.2_frame.html 
**                 (see below). Has a value of 0 (printed out as '.') for 
**                 non-helical elements.
** @@
******************************************************************************/






typedef struct AjSPdbfile
{
    /* Following are for each file */
    AjPStr    pdbid;    

    ajint     tercnt;   
    AjBool    toofewter;   
    ajint     modcnt;   
    AjBool    nomod;       
    AjPStr    compnd;      
    AjPStr    source;	   
    float     reso;	   
    ajint     method;      
    ajint     ngroups;   
    AjPChar   gpid;	       
    ajint     idxfirst;   


    /* Following are for each chain */
    ajint     nchains;    

    AjPStr    *seqres;      
    AjPStr    *seqresful;      
    ajint     *nres;            
    AjBool    *chainok;     
    AjBool    *resn1ok; 
    ajint     *nligands;      
    ajint    *numHelices;  
    ajint    *numStrands;  
    ajint    *numSheets;   
    ajint    *numTurns;    
    AjPChar    chid;	  


    /* Following are for each line */
    ajint      nlines;   

    AjPStr    *lines;    
    ajint     *linetype; 
    ajint     *chnn;     
    ajint     *gpn;         
    ajint     *modn;    
    ajint     *resni;   
    ajint     *resn1;   
    ajint     *resn2;   
    AjPStr    *pdbn;    
    AjBool    *oddnum;  
    AjPStr    *atype;      
    AjPStr    *rtype;      
    float     *x;          
    float     *y;          
    float     *z;          
    float     *o;          
    float     *b;          
    ajint    *elementNum;  
    AjPStr   *elementId;   
    char     *elementType; 
    ajint    *helixClass;  
}AjOPdbfile, *AjPPdbfile;






/*THIS_DIAGNOSTIC */ /*static void       pdbparse_diagnostic(AjPPdbfile *pdbfile, ajint n);  */
static void       pdbparse_PdbfileDel(AjPPdbfile *pthis);
static AjPPdb     pdbparse_PdbRead(AjPFile inf, AjPFile logf, AjPStr fname, 
				   AjPStr extn, ajint min_chain_size, 
				   ajint max_mismatch, AjBool nocamask, 
				   AjBool nocamask1, AjBool atommask);
static AjPPdbfile pdbparse_ReadLines(AjPFile inf);
static AjPPdbfile pdbparse_PdbfileNew(ajint nlines, ajint nchains);
static AjBool     pdbparse_FirstPass(AjPPdbfile *pdbfile, AjPFile logf, 
			    AjPElements *elms, AjBool nocamask);
static AjBool     pdbparse_CheckChains(AjPPdbfile *pdbfile, AjPFile logf, 
			      ajint min_chain_size);
static AjBool     pdbparse_SeqresToSequence(AjPStr seqres, AjPStr *seq, 
					    AjBool nocamask, ajint *len);
static AjBool     pdbparse_CheckTer(AjPPdbfile *pdbfile, AjPFile logf);
static AjBool     pdbparse_BaseAa3ToAa1(char *aa1, AjPStr aa3);
static AjBool     pdbparse_NumberChains(AjPPdbfile *pdbfile, AjPFile logf);
static AjBool     pdbparse_MaskChains(AjPPdbfile *pdbfile, AjPFile logf, 
			     ajint min_chain_size, AjBool nocamask, 
				      AjBool nocamask1, AjBool atommask);
static AjBool     pdbparse_StandardiseNumbering(AjPPdbfile *pdbfile, 
						AjPFile logf);
static AjBool     pdbparse_AlignNumbering(AjPPdbfile *pdbfile, AjPFile logf, 
					  ajint lim);
static AjBool     pdbparse_PdbfileToPdb(AjPPdb *ret, AjPPdbfile pdb);
static ajint      pdbparse_PdbfileFindLine(AjPPdbfile pdb, ajint chn, 
					   ajint which, ajint pos);


static AjPElements  pdbparse_ElementsNew(ajint nchains);
static void         pdbparse_ElementsDel(AjPElements *ptr);
static AjPElement   pdbparse_ElementNew(void);
static void         pdbparse_ElementDel(AjPElement *ptr);
static AjBool pdbparse_PdbfileChain(char id, AjPPdbfile pdb, ajint *chn);
static AjBool pdbparse_WriteElementData(AjPPdbfile *pdbfile, AjPFile logf, 
			       AjPElements elms);





/*THIS_DIAGNOSTIC  
  AjPStr  tempstr;    
  AjPFile tempfile;*/
  


/* @prog pdbparse *************************************************************
**
** Parse a pdb file and write a clean coordinate file.
**
******************************************************************************/

int main(ajint argc, char **argv)
{
    AjPStr     pdb_path     =NULL;	/* Path of pdb files */
    AjPStr     pdb_extn     =NULL;	/* Extn. of pdb files */
    AjPStr     pdb_name     =NULL;	/* Name of pdb file */
    AjPStr     cpdb_path    =NULL;	/* Path of cpdb files */
    AjPStr     cpdb_extn    =NULL;	/* Extn. of cpdb files */
    AjPStr     cpdb_name    =NULL;	/* Name of cpdb file */
    AjBool     cpdbnaming   =ajFalse;   
    /* True == use the pdbid code to name the output file, 
       False== use the name of the original pdb file*/

    /* Mask non-amino acid groups in protein chains that do not 
       contain a C-alpha atom. The group will not appear in either 
       the CO or SQ records of the clean coordinate file */
    AjBool     nocamask     =ajFalse;  
 

    /* Mask amino acids in protein chains that do not contain a 
       C-alpha atom. The amino acid will appear not appear in the 
       CO record but will still be present in the SQ record of the 
       clean coordinate file */
    AjBool     nocamask1    =ajFalse;   

    /* Mask residues or groups in protein chains with a single atom only */
    AjBool     atommask     =ajFalse;  
    
    AjPStr     temp         =NULL;	/* Temp string */   
    AjPStr     msg          =NULL;	/* Error message */
    AjPStr     base_name    =NULL;      /* Name of pdb file w/o path or 
					   extension */
    
    
    AjPFile    pdb_inf      =NULL;      /* pdb input file pointer */
    AjPFile    cpdb_outf    =NULL;      /* cpdb output file pointer */
    AjPFile    logf         =NULL;      /* log file pointer*/ 
    
    AjPList    list         =NULL;      /* For list of files in pdb 
					   directory */
    
    AjPPdb     pdb          =NULL;      /* Pdb structure (for parsed data) */
    
    ajint      min_chain_size=0;        /* Minimum length of a SEQRES chain 
					   for it to be parsed */
    ajint      max_mismatch=0;          /* Maximum number of permissible 
					   mismatches between the ATOM and 
					   SEQRES sequences */
    
    
    
    
/*     THIS_DIAGNOSTIC  
       tempstr=ajStrNew();    
       ajStrAssC(&tempstr,     "pdbparse_diagnostics");
    tempfile=ajFileNewOut(tempstr);
       ajStrDel(&tempstr);*/
       
    
    /* Initialise strings */
    cpdb_path     = ajStrNew();
    cpdb_extn     = ajStrNew();
    cpdb_name     = ajStrNew();
    pdb_path      = ajStrNew();
    pdb_extn      = ajStrNew();
    pdb_name      = ajStrNew();
    temp          = ajStrNew();
    msg           = ajStrNew();
    base_name     = ajStrNew();
    
    
    
    /* Read data from acd */
    embInit("pdbparse",argc,argv); 
    pdb_path     = ajAcdGetString("pdb");
    pdb_extn     = ajAcdGetString("pdbextn");
    cpdb_path    = ajAcdGetString("cpdb");
    cpdb_extn    = ajAcdGetString("cpdbextn");
    logf         = ajAcdGetOutfile("errf");
    min_chain_size=ajAcdGetInt("chnsiz");
    max_mismatch  =ajAcdGetInt("maxmis");
    cpdbnaming   = ajAcdGetBool("cpdbnaming");
    nocamask     = ajAcdGetBool("nocamask");
    nocamask1    = ajAcdGetBool("nocamaska");
    atommask     = ajAcdGetBool("atommask");
    
    
    /* Check directories*/
    if(!ajFileDir(&pdb_path))
	ajFatal("Could not open pdb directory");
    if(!ajFileDir(&cpdb_path))
	ajFatal("Could not open cpdb directory");
    
    
    /* Create list of files in pdb directory */
    list = ajListNew();
    ajStrAssC(&temp, "*");	
    
    
    if((ajStrChar(pdb_extn, 0)!='.'))
	ajStrInsertC(&pdb_extn, 0, ".");
    ajStrApp(&temp, pdb_extn);    
    
    
    ajFileScan(pdb_path, temp, &list, ajFalse, ajFalse, NULL, NULL, 
	       ajFalse, NULL); 
    ajStrDel(&temp);
    
    
    
    
    
    /*Start of main application loop*/
    while(ajListPop(list,(void **)&temp))
    {
	ajFmtPrint("Processing %S\n", temp);   
	ajFmtPrintF(logf, "%S\n", temp);    


	

	
	/* Read pdb file*/
	if((pdb_inf=ajFileNewIn(temp))==NULL)
	{
	    ajFmtPrintS(&msg, "Could not open for reading %S ", 
			temp);
	    ajWarn(ajStrStr(msg));
	    ajFmtPrintF(logf, "%-15s%S\n//\n", "FILE_OPEN", temp); 
	    ajStrDel(&temp);	
	    continue;	    
	}




	/* Parse pdb file and write pdb structure */
	if(!(pdb=pdbparse_PdbRead(pdb_inf, logf, temp, pdb_extn, 
				min_chain_size, max_mismatch,
				  nocamask, nocamask1, 
				  atommask)))
	{
	    ajFmtPrintS(&msg, "Clean coordinate file not generated for %S",
			temp);
	    ajWarn(ajStrStr(msg));
	    ajFmtPrintF(logf, "%-15s%S\n//\n", "NO_OUTPUT", temp); 
	    
	    ajFileClose(&pdb_inf);
	    ajStrDel(&temp);
	    continue;
	}
	

	/* Open clean coordinate file for writing*/
	if(cpdbnaming)
	{
	    ajStrAssS(&cpdb_name, cpdb_path);
	    ajStrApp(&cpdb_name, pdb->Pdb);
	    ajStrAppC(&cpdb_name, ajStrStr(cpdb_extn));
	    ajStrToLower(&cpdb_name);

	}
	else
	{
	    ajStrAssS(&base_name, temp);
	    ajStrTrim(&base_name, MAJSTRLEN(pdb_path));
	    ajStrTrim(&base_name, (-1 * MAJSTRLEN(pdb_extn)));
	    ajStrAssS(&cpdb_name, cpdb_path);
	    ajStrApp(&cpdb_name, base_name);
	    ajStrToLower(&cpdb_name);
	    ajStrAppC(&cpdb_name, ajStrStr(cpdb_extn));
	}
	
	if(!(cpdb_outf=ajFileNewOut(cpdb_name)))
	{
	    ajFmtPrintS(&msg, "Could not open %S for writing", 
			cpdb_name);
	    ajWarn(ajStrStr(msg));
	    ajFmtPrintF(logf, "%-15s%S\n//\n", "FILE_OPEN", cpdb_name); 
	    ajFileClose(&pdb_inf);
	    ajXyzPdbDel(&pdb);
	    ajStrDel(&temp);	
	    continue;
	}  
	


	/* Write pdb file */
	if(!ajXyzCpdbWriteAll(cpdb_outf, pdb))
	{
	    ajFmtPrintS(&msg, "Could not write file %S", cpdb_name);
	    ajWarn(ajStrStr(msg));
	    ajFmtPrintF(logf, "%-15s%S\n//\n", "FILE_WRITE", cpdb_name); 
	    
	    ajFmtPrintS(&temp, "rm %S", cpdb_name);
	    ajFmtPrint("%S", temp);
	    ajSystem(&temp);

	    ajFileClose(&pdb_inf);
	    ajFileClose(&cpdb_outf);
	    ajXyzPdbDel(&pdb);
	    ajStrDel(&temp);	
	    continue;
	}
	

    
	/* Tidy up*/
	ajFileClose(&pdb_inf);
	ajFileClose(&cpdb_outf);
	ajXyzPdbDel(&pdb);   
	ajStrDel(&temp);	


	ajFmtPrintF(logf, "//\n");    
    }
    /*End of main application loop*/    
    
    
    
    
    
    /*Tidy up */
    ajStrDel(&pdb_path);
    ajStrDel(&pdb_extn);
    ajStrDel(&pdb_name);
    ajStrDel(&cpdb_path);
    ajStrDel(&cpdb_extn);
    ajStrDel(&cpdb_name);
    ajStrDel(&base_name);
    ajStrDel(&msg);
    
    ajFileClose(&logf);
    
    ajListDel(&list);
    
    
    
    /* DIAGNOSTIC
       ajFileClose(&tempfile);
       */
    
    /* Return */
    ajExit();
    return 0;
}	





/* @funcstatic pdbparse_ReadLines *********************************************
**
** Reads a pdb file and returns a pointer to a partially filled
** Pdbfile object.  All of the lines from the pdb file are written to
** the <lines> array of the object and the <nlines> element is
** written.
**
** Memory for the object itself and any arrays whose size is equal to the 
** number of lines is allocated:
** lines, linetype, chnn, gpn, modn, resni, resn1, resn2, pdbn, oddnum, atype, 
** rtype, x,y,z,o,b, elementNum, elementId, elementType & helixClass.
** 
** The following elements are written:
** nlines, lines.
** 
** The linetype array is set to default value of PDBPARSE_IGNORE
**
** @param [r] inf  [AjPFile] Pointer to pdb file
**
** @return [AjPPdbfile] Pdbfile object pointer, or NULL on failure.
** @@
****************************************************************************/
static AjPPdbfile pdbparse_ReadLines(AjPFile inf)
{
    AjPPdbfile ptr    =NULL;		/* pdbfile object to be returned */
    AjPList    list   =NULL;		/* List of lines in pdb file */
    AjPStr     line   =NULL;		/* A line from a pdb file */
    ajint      i      =0;
    




    /* Check args */
    if(!inf)
	return NULL;
    
    
    /* Allocate list and pdbfile object*/
    list = ajListstrNew();
    /* Don't know number of lines or chains yet */
    ptr  = pdbparse_PdbfileNew(0,0);	
    
    
    /* Read pdb file and append lines to list */ 
    line = ajStrNew();
    while(ajFileReadLine(inf,&line))
    {
	ajListstrPushApp(list, line);
	line = ajStrNew();
    }

    
    /* Convert list to array in pdbfile object */
    ptr->nlines = ajListstrToArray(list, &(ptr->lines));

    if(ptr->nlines==0)
    {
	ajStrDel(&line);
	ajListstrDel(&list);
	pdbparse_PdbfileDel(&ptr);
	return NULL;
    }
    
    
        
    /* Allocate memory for x,y,z,o,b, modn, chnn, linetype, ok, coord and pdbn 
       arrays in pdbfile object */
    AJCNEW0(ptr->x, ptr->nlines); 
    AJCNEW0(ptr->y, ptr->nlines); 
    AJCNEW0(ptr->z, ptr->nlines); 
    AJCNEW0(ptr->o, ptr->nlines); 
    AJCNEW0(ptr->b, ptr->nlines); 

    AJCNEW0(ptr->elementNum, ptr->nlines); 
    AJCNEW0(ptr->elementType, ptr->nlines); 
    AJCNEW0(ptr->helixClass, ptr->nlines); 
    AJCNEW0(ptr->resni, ptr->nlines); 
    AJCNEW0(ptr->resn1, ptr->nlines); 
    AJCNEW0(ptr->resn2, ptr->nlines); 
    AJCNEW0(ptr->modn, ptr->nlines);

    AJCNEW0(ptr->chnn, ptr->nlines);
    AJCNEW0(ptr->gpn, ptr->nlines);

    AJCNEW0(ptr->linetype, ptr->nlines);
    for(i=0;i<ptr->nlines;i++)
	ptr->linetype[i]=PDBPARSE_IGNORE;


    AJCNEW0(ptr->pdbn, ptr->nlines);
    for(i=0;i<ptr->nlines;i++)
	ptr->pdbn[i]=ajStrNew();

    AJCNEW0(ptr->elementId, ptr->nlines);
    for(i=0;i<ptr->nlines;i++)
	ptr->elementId[i]=ajStrNew();


    AJCNEW0(ptr->atype, ptr->nlines);
    for(i=0;i<ptr->nlines;i++)
	ptr->atype[i]=ajStrNewL(4);

    AJCNEW0(ptr->rtype, ptr->nlines);
    for(i=0;i<ptr->nlines;i++)
	ptr->rtype[i]=ajStrNewL(4);


    AJCNEW0(ptr->oddnum, ptr->nlines);
    for(i=0;i<ptr->nlines;i++)
	ptr->oddnum[i]=ajFalse;
    
    /*DIAGNOSTIC 
      for(i=0;i<ptr->nlines;i++)
      {
      ajFmtPrintF(tempfile, "%S\n", ptr->lines[i]);
      fflush(tempfile->fp);
      }*/
    

    /* Tidy up and return */
    ajStrDel(&line);
    ajListstrDel(&list);
    return ptr;
}










/* @funcstatic pdbparse_PdbRead ***********************************************
**
** Reads a pdb file and returns a pointer to a filled Pdb object. 
** 
** The pdb id is derived from the file name and extension of the pdb file 
** (these are passed in by argument).
** 
** @param [r] inf             [AjPFile] Pointer to pdb file 
** @param [r] logf            [AjPFile] Pointer to log file (build diagnostics)
** @param [r] fname           [AjPStr]  Name of the pdb file 
** @param [r] extn            [AjPStr]  Extension of the pdb file 
** @param [r] min_chain_size  [ajint]   Minimum number of amino acids in a
**                                      chain 
** @param [r] max_mismatch    [ajint]   Maximum number of permissible
**                                      mismatches 
** between the ATOM and SEQRES sequences 
** @param [r] nocamask        [AjBool]  Whether to mask non-amino acid groups
** within protein chains which do not have a C-alpha atom.
** @param [r] nocamask1       [AjBool]  Whether to mask amino acid residues
** within protein chains which do not have a C-alpha atom.
** @param [r] atommask        [AjBool]  Whether to mask residues or groups 
** in protein chains with a single atom only.
**
** @return [AjPPdb] pdb object pointer, or NULL on failure.
** @@
******************************************************************************/
static AjPPdb pdbparse_PdbRead(AjPFile inf, AjPFile logf, AjPStr fname,
			       AjPStr extn, ajint min_chain_size,
			       ajint max_mismatch, AjBool nocamask,
			       AjBool nocamask1, AjBool atommask)
{
    AjPPdbfile pdbfile      =NULL;      /* Pdbfile structure (for raw data) */
    AjPPdb     ret          =NULL;      /* Pdb structure (for parsed data) */
    ajint      pos          =0;         /* Location of the file extension in 
					   the pdb file name */
    AjPElements elms=NULL;              /* Elements structure (for
                                           parsed data) */
    


    if(!inf || !logf || !fname)
    {
	ajWarn("Null arg passed to pdbparse_PdbRead");
	return NULL;
    }
    


/*    printf("1.\n");
    fflush(stdout); */
    
    /* Write pdbfile structure */
    if(!(pdbfile=pdbparse_ReadLines(inf)))
    {
	ajWarn("Problem reading raw pdb file");
	ajFmtPrintF(logf, "%-15s%S%S\n", "FILE_READ", fname, extn); 
	return NULL;
    }
/*    printf(".1\n");
    fflush(stdout);  */
    
    /* Allocate Elements object */
    elms = pdbparse_ElementsNew(0); 
    
    
    /* Assign pdb id code from file name - take the 4 characters preceeding 
       the file extension */
    if((pos=ajStrRFindC(fname, ajStrStr(extn)))<4)
	{
	    /* The file name is too short to contain the extension and
	       a 4 character pdbid code. Take everything to the left
	       of the extension to be the pdbid code */
	    ajWarn("Could not determine pdbid code from file name.");
	    if(pos)
		ajStrAssS(&(pdbfile->pdbid), fname);
	    else
		ajStrAssC(&(pdbfile->pdbid), ".");
	}	
    /* The file name is prob. of the form 1rbp.ent */
    else 
	ajStrAssSub(&(pdbfile->pdbid), fname, pos-4, pos-1);
    ajStrToLower(&(pdbfile->pdbid));
    
    
/*        printf("2.\n");
    fflush(stdout);   */

    /* Initial read of pdb file, read sequences for chains from 
       SEQRES records, mark lines up to ignore or as coordinate 
       lines, assigning initial residue numbers, read bibliographic
       information etc. */
    if(!pdbparse_FirstPass(&pdbfile, logf, &elms, nocamask))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;
    }

/*        printf(".2\n");
    fflush(stdout);   */
    
    /*DIAGNOSTIC
      pdbparse_diagnostic(&pdbfile, 0);        
      pdbparse_diagnostic(&pdbfile, 1);*/
    
    
/*        printf("3.\n");
    fflush(stdout);   */

    /* Check that SEQRES records contain protein chains. Check 
       that chain id's are unique */
    if(!pdbparse_CheckChains(&pdbfile, logf, min_chain_size))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;
    }

/*    printf(".3\n");
    fflush(stdout);  

    printf("4.\n");
    fflush(stdout);   */

    /* Check for correct number of TER records. Mask unwanted TER
       records */
    if(!pdbparse_CheckTer(&pdbfile, logf))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;
    }

/*    printf(".4\n");
    fflush(stdout);

    printf("5.\n");
    fflush(stdout);   */

    /* Assign model and chain number to each coordinate line. Mark
       up non-protein coordinates */
    if(!pdbparse_NumberChains(&pdbfile, logf))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;
    }

/*    printf(".5\n");
    fflush(stdout);

    printf("6.\n");
    fflush(stdout);   */




    /* Mask any ATOM or HETATM records with chain id's of chains of non-proteins
       or chains that have non-unique id's (chainok==ajFalse) */
    if(!pdbparse_MaskChains(&pdbfile, logf, min_chain_size, nocamask, 
			    nocamask1, atommask))
    {
	pdbparse_ElementsDel(&elms);
	pdbparse_PdbfileDel(&pdbfile);
	return NULL;

    }

/*    printf(".6\n");
    fflush(stdout);    */

    /* DIAGNOSTIC      
    pdbparse_diagnostic(&pdbfile, 0);        
    pdbparse_diagnostic(&pdbfile, 1);      */
  
    
   
/*    printf("7.\n");
    fflush(stdout);        */

    /* Standardise residue numbering */
    if(!pdbparse_StandardiseNumbering(&pdbfile, logf))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;
    }

/*        printf(".7\n");
    fflush(stdout);
    
    
    printf("8.\n");
    fflush(stdout);    */


    /* Find correct residue numbering */
    if(!pdbparse_AlignNumbering(&pdbfile, logf, max_mismatch))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;
    }

/*        printf(".8\n");
    fflush(stdout); 
      
        printf("9.\n");
    fflush(stdout);   */


    if(!pdbparse_WriteElementData(&pdbfile, logf, elms))
    {
	pdbparse_PdbfileDel(&pdbfile);
	pdbparse_ElementsDel(&elms);
	return NULL;	
    }

    pdbparse_ElementsDel(&elms);

/*        printf(".9\n");
    fflush(stdout);    */
    
    
    /* Create Pdb object to hold parsed data*/
    ret=ajXyzPdbNew(pdbfile->nchains);
    
    
/*        printf("10.\n");
    fflush(stdout);    */

    /* Copy data from Pdbfile object to Pdb object */
    if(!pdbparse_PdbfileToPdb(&ret, pdbfile))
    {
	pdbparse_PdbfileDel(&pdbfile);
	ajXyzPdbDel(&ret);
	return NULL;	
    }

/*        printf(".10\n");
    fflush(stdout);    */
    
    
    /* Tidy up and return */
    pdbparse_PdbfileDel(&pdbfile);
    
    return ret;
}





/* @funcstatic pdbparse_PdbfileDel ********************************************
**
** Destructor for pdbfile object.
**
** @param [w] pthis [AjPPdbfile*] Pdbfile object pointer
**
** @return [void]
** @@
******************************************************************************/
static void pdbparse_PdbfileDel(AjPPdbfile *pthis)
{
    ajint i;

    
    /* Check args */
    if(!(*pthis))
    {
	ajWarn("NULL arg passed to pdbparse_PdbfileDel.\n");
	return;
    }
    
    if((*pthis)->resn1ok)
	AJFREE((*pthis)->resn1ok);
    
    if((*pthis)->nres)
	AJFREE((*pthis)->nres);
    
    if((*pthis)->nligands)
	AJFREE((*pthis)->nligands);
    
    if((*pthis)->chainok)
	AJFREE((*pthis)->chainok);
    
    if((*pthis)->pdbid)
	ajStrDel(&(*pthis)->pdbid);
        
    if((*pthis)->compnd)
	ajStrDel(&(*pthis)->compnd);
    
    if((*pthis)->source)
	ajStrDel(&(*pthis)->source);
    
    
    if((*pthis)->modn)
	AJFREE((*pthis)->modn);
    
    if((*pthis)->x)
	AJFREE((*pthis)->x); 
    if((*pthis)->y)
	AJFREE((*pthis)->y); 
    if((*pthis)->z)
	AJFREE((*pthis)->z); 
    if((*pthis)->o)
	AJFREE((*pthis)->o); 
    if((*pthis)->b)
	AJFREE((*pthis)->b); 
    
    if((*pthis)->numHelices)
	AJFREE((*pthis)->numHelices); 
    if((*pthis)->numStrands)
	AJFREE((*pthis)->numStrands); 
    if((*pthis)->numSheets)
	AJFREE((*pthis)->numSheets); 
    if((*pthis)->numTurns)
	AJFREE((*pthis)->numTurns); 
    
    if((*pthis)->elementNum)
	AJFREE((*pthis)->elementNum); 
    if((*pthis)->elementType)
	AJFREE((*pthis)->elementType); 
    if((*pthis)->helixClass)
	AJFREE((*pthis)->helixClass); 
    
    if((*pthis)->resni)
	AJFREE((*pthis)->resni); 
    
    if((*pthis)->resn1)
	AJFREE((*pthis)->resn1); 
    
    if((*pthis)->resn2)
	AJFREE((*pthis)->resn2); 
    
    if((*pthis)->chnn)
	AJFREE((*pthis)->chnn);
    
    if((*pthis)->gpn)
	AJFREE((*pthis)->gpn);
    
    if((*pthis)->linetype)
	AJFREE((*pthis)->linetype);
    
    if((*pthis)->oddnum)
	AJFREE((*pthis)->oddnum);
    
    if((*pthis)->lines)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->lines[i]);
	AJFREE((*pthis)->lines);
    }
    
    if((*pthis)->pdbn)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->pdbn[i]);
	AJFREE((*pthis)->pdbn);
    }
    
    if((*pthis)->elementId)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->elementId[i]);
	AJFREE((*pthis)->elementId);
    }
    
    if((*pthis)->atype)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->atype[i]);
	AJFREE((*pthis)->atype);
    }
    
    if((*pthis)->rtype)
    {
	for(i=0;i<(*pthis)->nlines; i++)
	    ajStrDel(&(*pthis)->rtype[i]);
	AJFREE((*pthis)->rtype);
    }
    
    if((*pthis)->seqres)
    {
	for(i=0;i<(*pthis)->nchains; i++)
	    ajStrDel(&(*pthis)->seqres[i]);
	AJFREE((*pthis)->seqres);
    }

    if((*pthis)->seqresful)
    {
	for(i=0;i<(*pthis)->nchains; i++)
	    ajStrDel(&(*pthis)->seqresful[i]);
	AJFREE((*pthis)->seqresful);
    }
    
    
    if((*pthis)->chid)
	ajChararrDel(&(*pthis)->chid);
    
    if((*pthis)->gpid)
	ajChararrDel(&(*pthis)->gpid);
    
    
    AJFREE(*pthis);
    *pthis=NULL;
    
    return;
}




/* @funcstatic pdbparse_ElementsNew *******************************************
**
** Constructor for Elements object.
**
** @param [r] nelms    [ajint] no. of elements
**
** @return [AjPElements] Pointer to Elements object, or NULL on failure.
** @@
******************************************************************************/
static AjPElements pdbparse_ElementsNew(ajint nelms)
{
    AjPElements ret=NULL;
    ajint       x  =0;
    




    AJNEW0(ret);

    ret->n=nelms;
    
    if(nelms)
    {	
	AJCNEW0(ret->elms, nelms);
	for(x=0;x<nelms;x++)
	    ret->elms[x] = pdbparse_ElementNew();
    }
/*
    else
	ajWarn("Value of zero passed to pdbparse_ElementsNew");
  */  
    return ret;
}






/* @funcstatic pdbparse_ElementsDel *******************************************
**
** Destructor for Elements object.
**
** @param [w] ptr [AjPElements*] Elements object pointer
**
** @return [void]
** @@
******************************************************************************/
static void pdbparse_ElementsDel(AjPElements *ptr)
{
    ajint x=0;




    
    if(!(*ptr))
    {
	ajWarn("NULL arg passed to pdbparse_ElementsDel");
	return;
    }
    
    if((*ptr)->elms)
    {
	for(x=0; x<(*ptr)->n; x++)
	    pdbparse_ElementDel(&(*ptr)->elms[x]);
	AJFREE((*ptr)->elms);
    }
    
    AJFREE((*ptr));
    *ptr=NULL;
    
    return;
}




/* @funcstatic pdbparse_ElementNew ********************************************
**
** Constructor for Element object.
**
** @return [AjPElement] Pointer to Element object, or NULL on failure.
** @@
******************************************************************************/
static AjPElement pdbparse_ElementNew(void)
{
    AjPElement ret;



    
    AJNEW0(ret);
    
    ret->elementId   = ajStrNew();
    ret->initResName = ajStrNew();
    ret->initSeqNum  = ajStrNew();
    ret->endResName  = ajStrNew();
    ret->endSeqNum   = ajStrNew();

    ret->elementNum  = 0;
    ret->elementType = ' ';
    ret->helixClass  = 0;
    ret->chainId     = ' ';

    return ret;
}






/* @funcstatic pdbparse_ElementDel ********************************************
**
** Destructor for Element object.
**
** @param [w] ptr [AjPElement*] Element object pointer
**
** @return [void]
** @@
******************************************************************************/
static void pdbparse_ElementDel(AjPElement *ptr)
{
    ajStrDel(&((*ptr)->elementId));
    ajStrDel(&((*ptr)->initResName));
    ajStrDel(&((*ptr)->initSeqNum));
    ajStrDel(&((*ptr)->endResName));    
    ajStrDel(&((*ptr)->endSeqNum));

    AJFREE((*ptr));
    *ptr=NULL;

    return;
}




/* @funcstatic pdbparse_PdbfileNew ********************************************
**
** Constructor for Pdbfile object.
**
** @param [r] nlines  [ajint] no. of lines in pdb file
** @param [r] nchains [ajint] no. of chains in pdb file
**
** @return [AjPPdbfile] Pointer to pdbfile object, or NULL on failure.
** @@
******************************************************************************/
static AjPPdbfile pdbparse_PdbfileNew(ajint nlines, ajint nchains)
{
    AjPPdbfile ret  =NULL;
    ajint i=0;
    




    AJNEW0(ret);

    ret->pdbid = ajStrNew();
    ret->compnd = ajStrNew();
    ret->source = ajStrNew();

    ret->nomod=ajFalse;
    ret->toofewter=ajFalse;
    
    

    ret->nchains=nchains;
    if(nchains)
    {
	AJCNEW0(ret->resn1ok, nchains);
	for(i=0;i<nchains;i++)
	    ret->resn1ok[i]=ajTrue;


	AJCNEW0(ret->numHelices, nchains);
	for(i=0;i<nchains;i++)
	    ret->numHelices[i]=ajTrue;

	AJCNEW0(ret->numStrands, nchains);
	for(i=0;i<nchains;i++)
	    ret->numStrands[i]=ajTrue;

	AJCNEW0(ret->numSheets, nchains);
	for(i=0;i<nchains;i++)
	    ret->numSheets[i]=ajTrue;

	AJCNEW0(ret->numTurns, nchains);
	for(i=0;i<nchains;i++)
	    ret->numTurns[i]=ajTrue;


	AJCNEW0(ret->chainok, nchains);
	for(i=0;i<nchains;i++)
	    ret->chainok[i]=ajTrue;

	AJCNEW0(ret->nres, nchains);

	AJCNEW0(ret->nligands, nchains);
	
	AJCNEW0(ret->seqres, nchains);
	for(i=0;i<nchains;i++)
	    ret->seqres[i]=ajStrNew();

	AJCNEW0(ret->seqresful, nchains);
	for(i=0;i<nchains;i++)
	    ret->seqresful[i]=ajStrNew();


	ret->chid = ajChararrNewL(nchains);
    }
    else
    {
	ret->chid = ajChararrNew();	
    }

    ret->gpid = ajChararrNew();
    

    ret->nlines = nlines;
    if(nlines)
    {
	AJCNEW0(ret->x, nlines); 
	AJCNEW0(ret->y, nlines); 
	AJCNEW0(ret->z, nlines); 
	AJCNEW0(ret->o, nlines); 
	AJCNEW0(ret->b, nlines); 

	AJCNEW0(ret->elementNum, nlines); 
	AJCNEW0(ret->elementType, nlines); 
	AJCNEW0(ret->helixClass, nlines); 

	AJCNEW0(ret->resni, nlines); 
	AJCNEW0(ret->resn1, nlines); 
	AJCNEW0(ret->resn2, nlines); 
	AJCNEW0(ret->modn, nlines);
	AJCNEW0(ret->chnn, nlines);
	AJCNEW0(ret->gpn, nlines);
	AJCNEW0(ret->linetype, nlines);
	AJCNEW0(ret->oddnum, nlines);
	AJCNEW0(ret->lines, nlines);
	AJCNEW0(ret->pdbn, nlines);
	AJCNEW0(ret->elementId, nlines);
	AJCNEW0(ret->atype, nlines);
	AJCNEW0(ret->rtype, nlines);

	for(i=0;i<nlines;i++)
	{
	    ret->linetype[i]=PDBPARSE_IGNORE;
	    ret->oddnum[i]=ajFalse;
	    ret->lines[i]=ajStrNew();
	    ret->pdbn[i]=ajStrNew();
	    ret->elementId[i]=ajStrNew();
	    ret->atype[i]=ajStrNewL(4);
	    ret->rtype[i]=ajStrNewL(4);
	}
    }
/*
    else
	ajWarn("Zero sized arg passed to pdbparse_PdbfileNew.\n");
*/
    
    return ret;
}



/* @funcstatic pdbparse_FirstPass *********************************************
**
** The initial read of the pdb file as held in the <lines> array of a Pdbfile 
** object. Bibliographic information is parsed, the number of chains 
** determined and the sequences and chain ids from the SEQRES records are
** parsed. The line type (see below) of each line is determined, and for 
** lines with coordinates, the residue type is parsed. A count of the number
** of TER records and the location of the first coordinte line is also 
** determined.
** Secondary structure information is also parsed and an AjPElements 
** object is written.
** 
** Memory for any arrays whose size is equal to the number of chains is 
** allocated:
** seqres, seqresful, nres, chainok, resn1ok, nligands, numHelices, 
** numStrands, numSheets, numTurns 
**
** The following arrays are written:
** seqres, seqresful, chid, pdbn, resn1 and resn2, rtype, linetype
** 
** The following elements are written:
** nchains, tercnt, modcnt, nomod, source, compnd, method, reso, idxfirst
**
** The chainok & resn1ok arrays are set to default values of ajTrue
** 
**
** Writing resn1/resn2 & pdbn arrays The pdbn array is the raw residue
** number (as a string) and is filled for lines for which
** <linetype>==PDBPARSE_COORD. The resn1/resn2 arrays are given
** initial values which at this stage are simply the integer component
** of pdbn. The values for resn1/resn2 are changed later in the
** program.
**
** Writing modcnt and nomod elements
** modcnt is a count of the number of MODEL records (excluding duplicate 
** records). However, if no MODEL records are found, modcnt is set to the
** minimum value of 1, and nomod is set to ajTrue.
**
** Writing linetype array  
** The linetype array is set as follows:
**
** PDBPARSE_COORD for ATOM or HETATM records which contain both atom
** and residue identifier codes and which are not duplicate
** positions. Duplicate positions for (i) whole residues or (ii)
** individual atoms are presumed where a ATOM or HETATM record uses a
** value other than '1' or 'A' in the (i) residue alternate location
** indicator (column 17) or (ii) the first column of the atom name
** (column 13) respectively.
**
** PDBPARSE_TER for TER records
**
** PDBPARSE_MODEL for MODEL records
**
** For all other lines, it is left as the default of PDBPARSE_IGNORE (the value
** might change later in the program).
**
**
** @param [w] pdbfile  [AjPPdbfile*]  Pdbfile object pointer
** @param [r] logf     [AjPFile]      Pointer to log file (build diagnostics)
** @param [W] elms     [AjPElements*] Elements object pointer    
** @param [r] nocamask [AjBool]       Whether to mask non-amino acid residues 
** within protein chains which do not have a C-alpha atom.
** 
** @return [AjBool]  True if file was parsed, False otherwise
** @@
******************************************************************************/
static AjBool pdbparse_FirstPass(AjPPdbfile *pdbfile, AjPFile logf,
				 AjPElements *elms, 
				 AjBool nocamask)
{
    ajint    i=0;			/* Loop counter */
    ajint    j=0;			/* Loop counter */
    ajint    k=0;			/* Loop counter */
    AjBool   donefirstatom=ajFalse;	/* Flag for finding first ATOM
    or HETATM line */
    char     pdbn[6];			/* Residue number */
    AjBool   resolfound=ajFalse;	/* Flag for finding RESOLUTION
                                           record */
    AjBool   seqresfound=ajFalse;	/* Flag for finding SEQRES record */
    AjPStr   tmpstr=NULL;		/* A temp. string */
    AjPStr   seqres=NULL;		/* Sequence from SEQRES records */
    ajint    seqreslen=0;		/* Indicated length of sequence from 
					   SEQRES records */
    ajint    lenful=0;                  /* Length of SEQRES sequence including
					   ACE, FOR & NH2 groups that might be 
					   discarded by the call to 
					   pdbparse_SeqresToSequence */
    char     last_id=' ';		/* CHain id of last SEQRES
                                           line read in */
    AjPStr   tmpseq=NULL;		/* A temp. string for a sequence */
    AjPStr   tmpseqful=NULL;		/* A temp. string for a sequence */
    AjPList  listseqs=NULL;		/* For list of sequences from SEQRES 
					   records*/
    AjPList  listseqsful=NULL;		/* For list of sequences (using 
					   3-letter codes) from SEQRES 
					   records*/
    AjBool   done_msg=ajFalse;		/* Flag for error messaging */
    AjPList  listelms=NULL;		/* Temp. list of secondary
					   structure elements (from
					   HELIX, SHEET and TURN
					   records) */
    AjPElement  elm=NULL;		/* Temp. Element object pointer */
    AjPElement  FirstStrand=NULL;	/* Temp. pointer to first strand of 
					   each sheet*/
    AjBool     doneFirstStrand=ajFalse; /* Flag for parsing first strand of 
					   each sheet */
    AjPStr     LastSheetId=NULL;        /* Sheet identifier of the last sheet 
					   read in */
    




    /* Check args */
    if(!(*pdbfile) || !logf || !(*elms))
    {
	ajWarn("Bad args passed to pdbparse_FirstPass\n");
	return ajFalse;
    }
    
    
    /* Allocate memory etc*/
    tmpstr=ajStrNew();
    seqres=ajStrNew();
    LastSheetId=ajStrNew();
    
    listseqs=ajListstrNew();
    listseqsful=ajListstrNew();
    listelms=ajListNew();

    
    /* Start of main loop */
    for(i=0;i<(*pdbfile)->nlines;i++)
    {
	if((ajStrPrefixC((*pdbfile)->lines[i],"ATOM")) || 
	   (ajStrPrefixC((*pdbfile)->lines[i],"HETATM")))
	{ 
	    /*In instances where >1 residue positions are given, ignore 
	      all but position 'A' & '1'
	      In instances where >1 atom positions are given, ignore all 
	      but position '1'*/
	    if(((ajStrChar((*pdbfile)->lines[i], 16) != ' ')    && 
		((ajStrChar((*pdbfile)->lines[i], 16) != 'A')   &&
		 (ajStrChar((*pdbfile)->lines[i], 16) != '1'))) ||
	       ((ajStrChar((*pdbfile)->lines[i], 12) != ' ')    && 
		((ajStrChar((*pdbfile)->lines[i], 12) != '1')   &&
		 (isdigit((int)ajStrChar((*pdbfile)->lines[i], 12))))))
	    {
		if(!done_msg)
		{
		    ajFmtPrintF(logf, "%-15s%d\n", "DUPATOMRES", i+1); 
		    done_msg=ajTrue;
		}
		

	    }
	    /*In instances where no atom or residue identity is given, 
	      ignore line */
	    else if((ajStrChar((*pdbfile)->lines[i], 12)==' '  && 
		     ajStrChar((*pdbfile)->lines[i], 13)==' '  && 
		     ajStrChar((*pdbfile)->lines[i], 14)==' ') ||
		    (ajStrChar((*pdbfile)->lines[i], 17)==' '  &&
		     ajStrChar((*pdbfile)->lines[i], 19)==' '))
	    {

		ajFmtPrintF(logf, "%-15s%d\n", "NOATOMRESID", i+1); 

	    }
	    else 
	    {

		(*pdbfile)->linetype[i]=PDBPARSE_COORD;
		
               
		if(!donefirstatom)
		{
		    donefirstatom=ajTrue;
		    (*pdbfile)->idxfirst=i;
		}

		
		/*Write residue number for the line */
		for(k=22, j=0; k<=26; k++)
		    if((isalnum((int)ajStrChar((*pdbfile)->lines[i], k))) || 
		       ajStrChar((*pdbfile)->lines[i], k)=='-')
			pdbn[j++] = ajStrChar((*pdbfile)->lines[i], k);
		pdbn[j]='\0';
		
		ajStrAssC(&((*pdbfile)->pdbn[i]), pdbn);


		
		if(!ajFmtScanS((*pdbfile)->pdbn[i], "%d",
			       &((*pdbfile)->resn1[i])))
		{
		    ajFmtPrintF(logf, "%-15s%d\n", "ATOMNONUM", i+1);
		    (*pdbfile)->linetype[i]=PDBPARSE_IGNORE;		    
		}
		else
		{
		    (*pdbfile)->resn2[i]=(*pdbfile)->resn1[i];

		    /* Assign residue type */ 
		    ajStrAssSub(&(*pdbfile)->rtype[i], 
				(*pdbfile)->lines[i], 17, 19);
		    ajStrCleanWhite(&(*pdbfile)->rtype[i]);
		}
	    }
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"SEQRES"))
	{
	    seqresfound = ajTrue;
	    
	    /* Read first SEQRES line */
	    ajStrAssSub(&seqres, (*pdbfile)->lines[i], 14, 18);
	    if(!ajFmtScanS(seqres, "%d", &seqreslen))
		ajFmtPrintF(logf, "%-15s%d (%c)\n", "SEQRESLEN", 
			    (*pdbfile)->nchains, 
			    ajStrChar((*pdbfile)->lines[i], 11));


	    ajStrAssSub(&seqres, (*pdbfile)->lines[i], 19, 70);
	    
	    ajChararrPut(&((*pdbfile)->chid), 0, 
			 (last_id=ajStrChar((*pdbfile)->lines[i], 11))); 

	    
	    (*pdbfile)->nchains++;
	    

	    /* Read subsequent SEQRES lines */
	    for(i++;i<(*pdbfile)->nlines;i++)	    
		if(ajStrPrefixC((*pdbfile)->lines[i],"SEQRES"))
		{
		    /* Still on same chain */
		    if(ajStrChar((*pdbfile)->lines[i], 11) == last_id)
			ajStrAppSub(&seqres, (*pdbfile)->lines[i], 19, 70);
		    /* On new chain */
		    else
		    {	
			tmpseq=ajStrNew();


			/* Process last chain */
			if(!pdbparse_SeqresToSequence(seqres,
						      &tmpseq, nocamask, 
						      &lenful))
			{
			    ajWarn("Sequence conversion error in"
				   " pdbparse_FirstPass\nEmail "
				   "jison@hgmp.mrc.ac.uk\n");
			    ajStrDel(&tmpseq);
			    continue;
			}
			
			tmpseqful=ajStrNew();			
			ajStrAssS(&tmpseqful, seqres);
			


			

			/* Check length of sequenece vs indicated length */
			if(lenful != seqreslen)
			{
			    ajFmtPrintF(logf,
					"%-15s%d (%c)\n", "SEQRESLENDIF", 
					(*pdbfile)->nchains, 
					last_id);
			    
			}		

			/* Push sequences onto lists */
			ajListstrPushApp(listseqs, tmpseq);
			ajListstrPushApp(listseqsful, tmpseqful);

			
			/* Read first SEQRES line of new chain */
			ajStrAssSub(&seqres, (*pdbfile)->lines[i], 14, 18);
			if(!ajFmtScanS(seqres, "%d", &seqreslen))
			    ajFmtPrintF(logf, "%-15s%d (%c)\n", "SEQRESLEN", 
					(*pdbfile)->nchains, 
					ajStrChar((*pdbfile)->lines[i], 11));



			ajStrAssSub(&seqres, (*pdbfile)->lines[i], 19, 70);
			
			ajChararrPut(&((*pdbfile)->chid), (*pdbfile)->nchains, 
				(last_id=ajStrChar((*pdbfile)->lines[i], 11)));

		/*
		   if((last_id=ajStrChar((*pdbfile)->lines[i], 11))==' ')
		   ajChararrPut(&((*pdbfile)->chid), (*pdbfile)->nchains, '.');
		   else
		   ajChararrPut(&((*pdbfile)->chid),
		   (*pdbfile)->nchains, last_id); 
		   */
			


			(*pdbfile)->nchains++;
		    }
		}
		else
		{
		    tmpseq=ajStrNew();

		    /* Process last chain */
		    if(!pdbparse_SeqresToSequence(seqres, &tmpseq, nocamask, 
						  &lenful))
		    {
			ajWarn("Sequence conversion error in "
			       "pdbparse_FirstPass\nEmail "
			       "jison@hgmp.mrc.ac.uk\n");
			ajStrDel(&tmpseq);
			continue;
		    }

		    tmpseqful=ajStrNew();			
		    ajStrAssS(&tmpseqful, seqres);


		    /* Check length of sequenece vs indicated length */
		    if(lenful != seqreslen)
			{
			    ajFmtPrintF(logf, "%-15s%d (%c)\n",
					"SEQRESLENDIF", 
					(*pdbfile)->nchains, 
					last_id);
			}			
		    
		    /* Push sequences onto lists */
		    ajListstrPushApp(listseqs, tmpseq);
		    ajListstrPushApp(listseqsful, tmpseqful);
		    

		    /* Convert lists to arrays in pdbfile object and
                       delete list */
		    ajListstrToArray(listseqs, &((*pdbfile)->seqres));
		    ajListDel(&listseqs);

		    ajListstrToArray(listseqsful, &((*pdbfile)->seqresful));
		    ajListDel(&listseqsful);
		    
		    
		    i--;	     /* i will get incremented in main loop */
		    break;
		}
	    
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"COMPND"))
	{	
	    /* Read first COMPND line */
	    ajStrAssSub(&((*pdbfile)->compnd), (*pdbfile)->lines[i], 10, 71);
	    
	    /* Read subsequent COMPND lines */
	    for(i++;i<(*pdbfile)->nlines;i++)	    
		if(ajStrPrefixC((*pdbfile)->lines[i],"COMPND"))
		{
		    ajStrAppSub(&((*pdbfile)->compnd), 
				(*pdbfile)->lines[i], 10, 71);
		}
		else	
		{
		    ajStrClean(&((*pdbfile)->compnd));
		    i--;		/* i will get incremented in
                                           main loop */
		    break;
		}
	    
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"SOURCE"))
	{	
	    /* Read first SOURCE line */
	    ajStrAssSub(&((*pdbfile)->source), (*pdbfile)->lines[i], 10, 71);
	    
	    /* Read subsequent SOURCE lines */
	    for(i++;i<(*pdbfile)->nlines;i++)	    
		if(ajStrPrefixC((*pdbfile)->lines[i],"SOURCE"))
		{
		    ajStrAppSub(&((*pdbfile)->source), 
				(*pdbfile)->lines[i], 10, 71);
		}
		else	
		{
		    ajStrClean(&((*pdbfile)->source));
		    i--;		/* i will get incremented in
                                           main loop */
		    break;
		}
	    
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"TER")) 
	{
	    /* JISON do not need this line, by default ok == ajTrue 
	       (*pdbfile)->ok[i]=ajTrue; */
	    (*pdbfile)->tercnt++;
	    (*pdbfile)->linetype[i]=PDBPARSE_TER;
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"MODEL"))
	{
	    (*pdbfile)->modcnt++;
	    (*pdbfile)->linetype[i]=PDBPARSE_MODEL;
	    
	    if(!donefirstatom)
	    {
		donefirstatom=ajTrue;
		(*pdbfile)->idxfirst=i;
	    }
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"ENDMDL"))
	{
	    (*pdbfile)->linetype[i]=PDBPARSE_ENDMDL;
	}
	
	else if((!resolfound) && (ajStrPrefixC((*pdbfile)->lines[i],"REMARK")))
 	{
	    /*Assign method and resolution */
	    ajFmtScanS((*pdbfile)->lines[i], "%*s %*d %S", &tmpstr);
	    	    
	    if(!ajStrNCmpC(tmpstr, "RESOLUTION", 10))
	    {
		resolfound = ajTrue;
		  
		if(isdigit((int)ajStrChar((*pdbfile)->lines[i], 23)))
		{
		    if( (ajFmtScanS((*pdbfile)->lines[i], 
				    "%*s %*d %*s %f", 
				    &((*pdbfile)->reso))) != 1)
			ajFmtPrintF(logf, "%-15s\n", "RESOLUNK");
		    
		    (*pdbfile)->method=ajXRAY;
		}
		else
		{
		    (*pdbfile)->reso=0;
		    (*pdbfile)->method=ajNMR;
		}
	    }
	    
	}


	else if(ajStrPrefixC((*pdbfile)->lines[i],"HELIX"))
	{
	    doneFirstStrand=ajFalse;
	    

	    elm = pdbparse_ElementNew();

	    ajStrAssSub(&tmpstr, (*pdbfile)->lines[i], 7, 9);
	    ajStrCleanWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->elementNum);
	    
	    ajStrAssSub(&elm->elementId, (*pdbfile)->lines[i], 11, 13);
	    ajStrCleanWhite(&elm->elementId);

	    elm->elementType = 'H';

	    ajStrAssSub(&elm->initResName, (*pdbfile)->lines[i], 15, 17);
	    ajStrCleanWhite(&elm->initResName);

	    ajStrAssSub(&elm->initSeqNum, (*pdbfile)->lines[i], 21, 25);
	    ajStrCleanWhite(&elm->initSeqNum);

	    ajStrAssSub(&elm->endResName, (*pdbfile)->lines[i], 27, 29);
	    ajStrCleanWhite(&elm->endResName);

	    ajStrAssSub(&elm->endSeqNum, (*pdbfile)->lines[i], 33, 37);
	    ajStrCleanWhite(&elm->endSeqNum);

	    elm->chainId = ajStrChar((*pdbfile)->lines[i], 19);
	    
	    if(elm->chainId != ajStrChar((*pdbfile)->lines[i], 31))
	    {
		ajFmtPrintF(logf, "%-15s%c %c\n", "SECTWOCHN", elm->chainId,
			    ajStrChar((*pdbfile)->lines[i], 31));
		pdbparse_ElementDel(&elm);
		continue;	
	    }
	    
	    
	    ajStrAssSub(&tmpstr, (*pdbfile)->lines[i], 38, 39);
	    ajStrCleanWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->helixClass);
	    

	    /* Check that all records are present and flag an error if
               they're not */
	    if(MAJSTRLEN(elm->initResName) && MAJSTRLEN(elm->initSeqNum) 
	       && MAJSTRLEN(elm->endResName) && MAJSTRLEN(elm->endSeqNum) && 
	       MAJSTRLEN(elm->elementId))
	    {
		ajListPushApp(listelms, elm);
	    }
	    else	 
	    {
		ajFmtPrintF(logf, "%-15s%d\n", "SECMISS", i+1);
		pdbparse_ElementDel(&elm);
		continue;	
	    }
	    
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"SHEET"))
	{
	    elm = pdbparse_ElementNew();

	    ajStrAssSub(&tmpstr, (*pdbfile)->lines[i], 7, 9);
	    ajStrCleanWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->elementNum);

	    ajStrAssSub(&elm->elementId, (*pdbfile)->lines[i], 11, 13);
	    ajStrCleanWhite(&elm->elementId);

	    if(!ajStrMatch(elm->elementId, LastSheetId))
		doneFirstStrand=ajFalse;		

	    elm->elementType = 'E';

	    ajStrAssSub(&elm->initResName, (*pdbfile)->lines[i], 17, 19);
	    ajStrCleanWhite(&elm->initResName);

	    ajStrAssSub(&elm->initSeqNum, (*pdbfile)->lines[i], 22, 26);
	    ajStrCleanWhite(&elm->initSeqNum);

	    ajStrAssSub(&elm->endResName, (*pdbfile)->lines[i], 28, 30);
	    ajStrCleanWhite(&elm->endResName);

	    ajStrAssSub(&elm->endSeqNum, (*pdbfile)->lines[i], 33, 37);
	    ajStrCleanWhite(&elm->endSeqNum);

	    elm->chainId = ajStrChar((*pdbfile)->lines[i], 21);
	    if(elm->chainId != ajStrChar((*pdbfile)->lines[i], 32))
	    {
		ajFmtPrintF(logf, "%-15s%c %c\n", "SECTWOCHN", elm->chainId,
			    ajStrChar((*pdbfile)->lines[i], 32));
		pdbparse_ElementDel(&elm);
		continue;	
	    }
	    
	    
	    /* Check for beta-barrels - where the first and last strands 
	       are identical requiring us to ignore the last strand */
	    
	    if(doneFirstStrand)
	    {
		if(ajStrMatch(elm->initResName, FirstStrand->initResName)  &&
		   ajStrMatch(elm->endResName, FirstStrand->endResName)    &&
		   ajStrMatch(elm->initSeqNum, FirstStrand->initSeqNum)    &&
		   ajStrMatch(elm->endSeqNum, FirstStrand->endSeqNum))
		{			
		    pdbparse_ElementDel(&elm);
		    continue;
		}
	    }
	    
	    /* Check that all records are present and flag an error if
               they're not */
	    if(MAJSTRLEN(elm->initResName) && MAJSTRLEN(elm->initSeqNum) && 
	       MAJSTRLEN(elm->endResName) && MAJSTRLEN(elm->endSeqNum) && 
	       MAJSTRLEN(elm->elementId))	    
	    {
		ajListPushApp(listelms, elm); 
	    }
	    else
	    {	
		ajFmtPrintF(logf, "%-15s%d\n", "SECMISS", i+1);
		pdbparse_ElementDel(&elm);
		continue;	

	    }
	    
	    	

	    ajStrAssS(&LastSheetId, elm->elementId);
	    FirstStrand=elm;
	    doneFirstStrand=ajTrue;
	}
	else if(ajStrPrefixC((*pdbfile)->lines[i],"TURN"))
	{
	    doneFirstStrand=ajFalse;

	    elm = pdbparse_ElementNew();

	    ajStrAssSub(&tmpstr, (*pdbfile)->lines[i], 7, 9);
	    ajStrCleanWhite(&tmpstr);
	    ajFmtScanS(tmpstr, "%d", &elm->elementNum);

	    ajStrAssSub(&elm->elementId, (*pdbfile)->lines[i], 11, 13);
	    ajStrCleanWhite(&elm->elementId);

	    elm->elementType = 'T';

	    ajStrAssSub(&elm->initResName, (*pdbfile)->lines[i], 15, 17);
	    ajStrCleanWhite(&elm->initResName);

	    ajStrAssSub(&elm->initSeqNum, (*pdbfile)->lines[i], 20, 24);
	    ajStrCleanWhite(&elm->initSeqNum);

	    ajStrAssSub(&elm->endResName, (*pdbfile)->lines[i], 26, 28);
	    ajStrCleanWhite(&elm->endResName);

	    ajStrAssSub(&elm->endSeqNum, (*pdbfile)->lines[i], 31, 35);
	    ajStrCleanWhite(&elm->endSeqNum);

	    elm->chainId = ajStrChar((*pdbfile)->lines[i], 19);
	    if(elm->chainId != ajStrChar((*pdbfile)->lines[i], 30))
	    {
		ajFmtPrintF(logf, "%-15s%c %c\n", "SECTWOCHN", elm->chainId,
			    ajStrChar((*pdbfile)->lines[i], 30));
		pdbparse_ElementDel(&elm);
		continue;	
	    }	


	    /* Check that all records are present and flag an error if
               they're not */
	    if(MAJSTRLEN(elm->initResName) && MAJSTRLEN(elm->initSeqNum) 
	       && MAJSTRLEN(elm->endResName) && MAJSTRLEN(elm->endSeqNum) 
	       && MAJSTRLEN(elm->elementId))	    
	    {
		ajListPushApp(listelms, elm);
	    }
	    else	 
	    {
		ajFmtPrintF(logf, "%-15s%d\n", "SECMISS", i+1);
		pdbparse_ElementDel(&elm);
		continue;	
	    }
	}
    }



    
    /* Write array in Elements structure */
    (*elms)->n=ajListToArray(listelms, (void ***) &(*elms)->elms);
        

    /* Generate diagnostics and set defaults */
    if((ajStrLen((*pdbfile)->compnd) == 0))
    {
	ajStrAssC(&(*pdbfile)->compnd, ".");
	ajFmtPrintF(logf, "%-15s\n", "NOCOMPND");  
    }
    if((ajStrLen((*pdbfile)->source) == 0))
    {
	ajStrAssC(&(*pdbfile)->source, ".");
	ajFmtPrintF(logf, "%-15s\n", "NOSOURCE");
    }
    
    if(((*pdbfile)->method==ajNMR) && ((*pdbfile)->modcnt==0))
	ajFmtPrintF(logf, "%-15s\n", "NOMODEL");


    if((*pdbfile)->reso && (*pdbfile)->modcnt)
    {
	ajFmtPrintF(logf, "%-15s\n", "RESOLMOD");
	(*pdbfile)->method=ajNMR;
    }
    


    /* Every pdb file is considered to have at least one model */
    if((*pdbfile)->modcnt==0)
    {
	(*pdbfile)->modcnt=1;
	(*pdbfile)->nomod=ajTrue;
    }
    


    if(!resolfound)
    {
	(*pdbfile)->reso=0;
	(*pdbfile)->method=ajNMR;
	ajFmtPrintF(logf, "%-15s\n","NORESOLUTION");
    }



    if(!seqresfound)
    {
	ajWarn("No SEQRES record found in raw pdb file");
	ajFmtPrintF(logf, "%-15s\n", "NOSEQRES");
	
	/* Free memory and return */
	ajListDel(&listelms);
	ajStrDel(&LastSheetId);
	ajStrDel(&tmpstr);
	ajStrDel(&seqres);
	return ajFalse;
    }


    

    if(!donefirstatom)    
    {
	ajWarn("No ATOM record found in raw pdb file");
	ajFmtPrintF(logf, "%-15s\n", "NOATOM");
	
	/* Free memory and return */
	ajListDel(&listelms);
	ajStrDel(&LastSheetId);    
	ajStrDel(&tmpstr);
	ajStrDel(&seqres);
	return ajFalse;
    }


    AJCNEW0((*pdbfile)->nligands, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->nres, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->numHelices, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->numStrands, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->numSheets, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->numTurns, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->chainok, (*pdbfile)->nchains);
    AJCNEW0((*pdbfile)->resn1ok, (*pdbfile)->nchains);
    


    for(i=0;i<(*pdbfile)->nchains;i++)
    {
	(*pdbfile)->chainok[i]=ajTrue;
	(*pdbfile)->resn1ok[i]=ajTrue;
    }


    /* Free memory and return */
    ajStrDel(&LastSheetId);
    ajListDel(&listelms);

    ajStrDel(&tmpstr);
    ajStrDel(&seqres);
    return ajTrue;
}




/* @funcstatic pdbparse_SeqresToSequence **************************************
**
** Reads a string containing a SEQRES sequence  (e.g. "ALA ALA LEU" ) and 
** writes a string containing a normal sequence (e.g. "AAL").
**
** @param [r] seqres   [AjPStr]   SEQRES sequence
** @param [w] seq      [AjPStr *] Output sequence
** @param [r] nocamask [AjBool]   Whether to ignore residues which do not 
** have a C-alpha atom, these are defined as ACE, FOR and NH2 groups.
** @param [w] len      [ajint *]  Length of sequence INCLUDING ACE, FOR and 
** NH2 groups.
** 
** @return [AjBool] ajTrue on success, ajFalse otherwise.
** @@
*****************************************************************************/
static AjBool pdbparse_SeqresToSequence(AjPStr seqres, AjPStr *seq,
					AjBool nocamask, 
			       ajint *len)
{
    static AjPStr    aa3  =NULL;
    char aa1;
    ajint nrem=0;  /* No. 'residues' that were removed */
    

    /* Check args */   
    if(!seqres || !seq)
    {
	ajWarn("Bad args passed to pdbparse_SeqresToSequence\n");
	return ajFalse;
    }

    /* Allocate memory */

    
    if((aa3=ajStrTokC(seqres, " \n")))
	/* Parse seqres string */
	do
	{
	    if(ajStrMatchC(aa3, "FOR") ||
	       ajStrMatchC(aa3, "ACE") ||
	       ajStrMatchC(aa3, "NH2"))
		{
		    nrem++;
		    continue;
		}
	    
	    else
	    {
		pdbparse_BaseAa3ToAa1(&aa1, aa3);
		ajStrAppK(seq, aa1);
	    }
	}
	while((aa3=ajStrTokC(NULL, " \n")));
    else
	return ajFalse;
    
    *len = ajStrLen(*seq)+nrem;
    return ajTrue;
}





/* @funcstatic pdbparse_BaseAa3ToAa1 ******************************************
**
** Takes a 3 character amino acid code and writes a char with the corresponding
** single letter code.
**
** @param [w] aa1 [char *]   Single letter identifier of amino acid
** @param [r] aa3 [AjPStr]   AjPStr object (3 letter code)
**
** @return [AjBool] True on success
** @@
** NOTE THIS SHOULD BE MOVE TO ajbase.c / h at some point.
******************************************************************************/
static AjBool  pdbparse_BaseAa3ToAa1(char *aa1, AjPStr aa3)
{
    ajint i;
    
    static char *tab[]=
    {
	"ALA\0","ASX\0","CYS\0","ASP\0","GLU\0","PHE\0","GLY\0","HIS\0",
	"ILE\0","---\0","LYS\0","LEU\0","MET\0","ASN\0","---\0","PRO\0",
	"GLN\0","ARG\0","SER\0","THR\0","---\0","VAL\0","TRP\0","XAA\0",
	"TYR\0","GLX\0" 
    };





    for(i=0; i<26; i++)
	if(!ajStrCmpC(aa3, tab[i]))
	{
	    *aa1 = (char) (i + (int) 'A');
	    return ajTrue;
	}
    
    if(!ajStrCmpC(aa3, "UNK"))
    {
	*aa1 = 'X';
	return ajTrue;
    }	
    
    *aa1='X';
    return ajFalse;
}







/* @funcstatic pdbparse_CheckChains *******************************************
**
** Reads a Pdbfile object and checks whether chains from the SEQRES records 
** (i) use unique chain ids, (ii) do not use an id of a space (' ') alongside 
** non-space chain ids and (iii) contain at least the user-defined threshold 
** number of amino acid residues. If any of these conditions are not met then 
** the chain is discarded (chainok array is set to ajFalse).
**
** The chainok array is written.
** 
**
** @param [w] pdbfile         [AjPPdbfile*] Pdbfile object pointer
** @param [r] logf            [AjPFile]     Pointer to log file (build 
** diagnostics).
** @param [r] min_chain_size  [ajint]       Minimum number of amino acids in 
** a chain.
** 
** @return [AjBool]  True if SEQRES records contained at least one protein 
** chain, False otherwise.
** @@
******************************************************************************/

static AjBool pdbparse_CheckChains(AjPPdbfile *pdbfile, AjPFile logf,
				   ajint min_chain_size)
{
    ajint    i=0;			/* Loop counter */
    ajint    j=0;			/* Loop counter */
    AjIStr   iter=NULL;			/* Iterator for sequence strings */
    ajint    aacnt=0;			
    /* Counter for no. of amino acids in sequence strings */

    AjBool   ok=ajFalse;
    /* Flag which is True if amino acid chains are found  in the SEQRES records */

    char     id1=' ';			/* Chain id */
    char     id2=' ';			/* Chain id */
    AjBool   iderr=ajFalse;	
    /* ajTrue if both a space and a character are used as chain 
       id's in the same file */




    
    /* Check args */
    if(!(*pdbfile) || !logf)
    {
	ajWarn("Bad args passed to pdbparse_CheckChains\n");
	return ajFalse;
    }
    
    
    /* Report problems with chain id's */
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	id1 = ajChararrGet((*pdbfile)->chid, i);
	for(j=i+1; j<(*pdbfile)->nchains; j++)
	{
	    if(id1==(id2=ajChararrGet((*pdbfile)->chid, j)))
	    {
		ajFmtPrintF(logf, "%-15s%d (%c) %d (%c)\n", "CHAINIDS", i+1, 
			    ajChararrGet((*pdbfile)->chid, i), j+1, 
			    ajChararrGet((*pdbfile)->chid, j));
		
		(*pdbfile)->chainok[i] = ajFalse;
		(*pdbfile)->chainok[j] = ajFalse;
	    }
	    
	    if((((id1 == ' ')&&(id2 != ' '))||((id2 == ' ')&&(id1 != ' '))) 
	       && (!(iderr)))
	    {
		ajFmtPrintF(logf, "%-15s\n", "CHAINIDSPC");
		iderr=ajTrue;
	    }
	    
	}
    }


    /* Report problems with non-protein chains */
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	if(!(*pdbfile)->chainok[i])
	    continue;
	

	aacnt=0;
	iter=ajStrIter((*pdbfile)->seqres[i]);
	if(toupper(ajStrIterGetK(iter)) != 'X')
	    ++aacnt;
	
	while(ajStrIterNext(iter))
	    if(toupper(ajStrIterGetK(iter)) != 'X')
		if(++aacnt >= min_chain_size)
		    break;

	ajStrIterFree(&iter);
	
	
	if(aacnt==0)
	{
	    ajFmtPrintF(logf, "%-15s%d (%c)\n", "SEQRESNOAA", i+1, 
			ajChararrGet((*pdbfile)->chid, i));
	    (*pdbfile)->chainok[i] = ajFalse;
	}
	else if(aacnt < min_chain_size)
	{	
	    ajFmtPrintF(logf, "%-15s%d (%c)\n", "SEQRESFEWAA", i+1, 
			ajChararrGet((*pdbfile)->chid, i));
	    
	    (*pdbfile)->chainok[i] = ajFalse;


	}
	else 
	{
	    ok = ajTrue;
	}
    }

    /* Return now if no protein chains are found */
    if(!ok)
    {
	ajWarn("No protein chains found in raw pdb file");
	ajFmtPrintF(logf, "%-15s\n", "NOPROTEINS");
	return ajFalse;
    }
    

    return ajTrue;
}







/* @funcstatic pdbparse_CheckTer **********************************************
**
** Reads a Pdbfile object and checks whether the expected number of TER 
** and MODEL records are present. Any unwanted records (e.g. TER records that
** delimit fragments of chain digests and duplicate MODEL records) are 
** discared (the linetype array for the lines are set to PDBPARSE_IGNORE).
** 
** The linetype array and modcnt variable may be modified. The toofewter
** element is written.
** The value of modcnt is reduced by 1 for each MODEL record that was masked
** but this is not done for tercnt.
**
** @param [w] pdbfile         [AjPPdbfile*] Pdbfile object pointer
** @param [r] logf            [AjPFile]     Pointer to log file
**                                          (build diagnostics)
** 
** @return [AjBool]  True on success, False otherwise.
** @@
******************************************************************************/
static AjBool pdbparse_CheckTer(AjPPdbfile *pdbfile, AjPFile logf)
{
    AjBool toomany=ajFalse;
    AjBool toofew =ajFalse;
    ajint    i=0;			/* Loop counter */
    AjPStr aa1=NULL;
    AjPStr aa2=NULL;
    char   aa=' ';




    
    /* Check args */
    if(!(*pdbfile) || !logf)
    {
	ajWarn("Bad args passed to pdbparse_CheckTer\n");
	return ajFalse;
    }
    

    /* Allocate memory */
    aa1=ajStrNew();
    aa2=ajStrNew();
    
    
    /* Report problems with TER records */
    if(!(*pdbfile)->tercnt)
	ajFmtPrintF(logf, "%-15s\n", "TERNONE");
    else
    {
	if((*pdbfile)->method==ajNMR)
	{
	    if((*pdbfile)->tercnt > ((*pdbfile)->nchains * 
				     (*pdbfile)->modcnt))
		toomany=ajTrue;
	    else if((*pdbfile)->tercnt < ((*pdbfile)->nchains * 
					  (*pdbfile)->modcnt))
		toofew=ajTrue;
	}
	else
	{
	    if((*pdbfile)->tercnt > (*pdbfile)->nchains)
		toomany=ajTrue;
	    else if((*pdbfile)->tercnt < (*pdbfile)->nchains)
		toofew=ajTrue;
	}
    }	


    /* Report diagnostics */
    if(toomany)
	ajFmtPrintF(logf, "%-15s\n", "TERTOOMANY");
    else if(toofew)
    {
	ajFmtPrintF(logf, "%-15s\n", "TERTOOFEW");
	(*pdbfile)->toofewter=ajTrue;
    }
    

    /* Mask out the extra TER records */ 
    if(toomany)
    {
	for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
	{
	    /* This is a TER record. Mask it out if it is flanked
	       by ATOM or HETATM records for AMINO ACIDS and with 
	       identical chain ids. */
	    if((*pdbfile)->linetype[i]==PDBPARSE_TER)
	    {
		if(((*pdbfile)->linetype[i-1]==PDBPARSE_COORD) && 
		   ((*pdbfile)->linetype[i+1]==PDBPARSE_COORD) &&
		   (ajStrChar((*pdbfile)->lines[i-1], POS_CHID)
		    ==ajStrChar((*pdbfile)->lines[i+1], POS_CHID)))
		{
		    ajStrAssSub(&aa1, (*pdbfile)->lines[i-1], 17, 19);
		    ajStrAssSub(&aa2, (*pdbfile)->lines[i+1], 17, 19);
		    if((pdbparse_BaseAa3ToAa1(&aa, aa1)) && 
		       (pdbparse_BaseAa3ToAa1(&aa, aa2)))
		    {
			(*pdbfile)->linetype[i]=PDBPARSE_IGNORE;

		    }
			
		}
		
	    }
	    
		
	}
    }


    /* Check for duplicate MODEL records */
    for(i=(*pdbfile)->idxfirst+1;
	i<(*pdbfile)->nlines;i++)
    {
	if(((*pdbfile)->linetype[i-1]==PDBPARSE_MODEL) && 
	   ((*pdbfile)->linetype[i]==PDBPARSE_MODEL))
	{
	    (*pdbfile)->linetype[i-1]=PDBPARSE_IGNORE;
	    (*pdbfile)->modcnt--;
	    ajFmtPrintF(logf, "%-15s%d\n", "MODELDUP", i+1);


	}
    }
    

    /*Tidy up and return */
    ajStrDel(&aa1);
    ajStrDel(&aa2);
    return ajTrue;
}






/* @funcstatic pdbparse_NumberChains ******************************************
**
** Reads a Pdbfile object and assigns each ATOM or HETATM record to a specific
** chain and model. Lines containing coordinates for water molecules and 
** other non-protein groups ("heterogens") are identified.  Water molecules are
** uniquely associated with a whole model whereas other non-protein groups are 
** associated with a unique chain if possible. If this is not possible they 
** are assigned a unique group number. 
**
** The modn, chnn and gpn arrays are written. The linetype array is modified.
** The nligands array (count of groups associated with a chain), ngroups 
** element (count of groups not associated with a chain) and gpid (identifiers
** of these later groups) are written.
** 
** linetype array:
**
** The linetype array is written with a value of PDBPARSE_COORDWAT for
** lines containing coordinates for water, and to PDBPARSE_COORDHET or
** PDBPARSE_COORDGP for non-protein groups that, respectively, could
** or could not be uniquely associated with a chain.
** 
**
** @param [w] pdbfile         [AjPPdbfile*] Pdbfile object pointer
** @param [r] logf            [AjPFile]     Pointer to log file
**                                          (build diagnostics)
** 
** @return [AjBool]  True on success, False otherwise
** @@
******************************************************************************/

static AjBool pdbparse_NumberChains(AjPPdbfile *pdbfile, AjPFile logf)
{
    ajint  i=0;			/* Loop counter */
    ajint  j=0;			/* Loop counter */
    ajint  mod=0;		/* Model number */
    char   id=' ';		/* Chain id */
    AjBool doneoneter=ajFalse;	/*True if we have read in at least one TER 
				  record for this model */

    AjBool done=ajFalse;  /*True if we have assigned a chain id for
                            this line */
    ajint  this=0;	  /* Chain number of last line read in */
    ajint  prev=-1;	  /* Chain number of last line read in before the TER 
			     record */
    ajint  chn=0;	  /* Chain number as index (starting from 0) */
    AjPInt gpns=NULL;	  /* Gives the correct group number 
			     for groups that could not be identified as 
			     belonging to a chain, in cases where a single 
			     chain only is present in the file */
    ajint  gpn=0;	  /* Current group number */
    ajint  offset=0;	  /* Offset for finding correct value for gpns (for 
			     use with files with a single chain only */
    AjBool doneter=ajFalse; /*True if we have read a TER record for
                              this chain */
    AjBool *chndone=NULL;   /* Array whose elements are TRUE if we have already
			   read a line in belonging to the appropriate chain 
			   for this model*/
    AjPStr *htype;   	    /* Array holding the residue type of the
                               last heterogen read in for the
                               appropriate chain */
    



    
    /* Check args */
    if(!(*pdbfile) || !logf)
    {
	ajWarn("Bad args passed to pdbparse_NumberChains\n");
	return ajFalse;
    }    

    /* Allocate memory */
    gpns = ajIntNew();
    
    AJCNEW0(htype, (*pdbfile)->nchains);
    for(i=0;i<(*pdbfile)->nchains; i++)
	htype[i]=ajStrNew();
    

    AJCNEW0(chndone, (*pdbfile)->nchains);
    for(i=0;i<(*pdbfile)->nchains; i++)
	chndone[i]=ajFalse;



    if((((*pdbfile)->method==ajNMR) && (*pdbfile)->nomod) ||
       ((*pdbfile)->method==ajXRAY))
	mod=1;
    
    

    for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{

	    (*pdbfile)->modn[i]=mod;


	    /*Assign appropriate chain number to any ATOM or HETATM 	
	      line which has an id exhibited in the SEQRES records. */
	    for(done=ajFalse, id=ajStrChar((*pdbfile)->lines[i], POS_CHID), 
		j=0;j<(*pdbfile)->nchains; j++)
	    {
		if(ajChararrGet((*pdbfile)->chid, j) == id)
		{
		    (*pdbfile)->chnn[i]=j+1;
		    chn=(*pdbfile)->chnn[i]-1;
		    this=j+1;



		    if(chndone[this-1])
		    {
			/* Mark up water coordintes */
			if(ajStrMatchC((*pdbfile)->rtype[i], "HOH"))
			    (*pdbfile)->linetype[i]=PDBPARSE_COORDWAT;
			else
			{
			    /* Mark up ligand coordinates */
			    (*pdbfile)->linetype[i]=PDBPARSE_COORDHET;



			    /* New heterogen */
			    if(!ajStrMatch(htype[chn], (*pdbfile)->rtype[i]))
			    {
				offset++;
				(*pdbfile)->nligands[chn]++;
				(*pdbfile)->gpn[i] = (*pdbfile)->nligands[chn];
				ajStrAssS(&htype[chn], (*pdbfile)->rtype[i]);
			    }
			    /* More atoms of the same heterogen */
			    else
			    {
				(*pdbfile)->gpn[i] = (*pdbfile)->nligands[chn];
			    }
			}	
		    }
		    else 
			doneter=ajFalse; 


		    done=ajTrue;
		    break;
		}
	    }
	    
	    
	    if(!done)
	    {		
		/* Any ATOM or HETATM
		   record with a whitspace as chain id and which has not 
		   already been assigned belongs to the chain immediately 
		   preceeding it. Assign these lines as NON_PROTEIN_CHAIN 
		   lines.  */

		if(id==' ')
		{
		    /* This won't be set until we've read in at least one
		       coordinate line for protein chain, so if this==0, 
		       set it to 1 (first chain). This prevents problems
		       for 1qjh.pxyz*/
		  
		  if(this==0)
		    this=1;

		    (*pdbfile)->chnn[i]=this;
		    chn=(*pdbfile)->chnn[i]-1;

		    /* Mark up water coordintes */
		    if(ajStrMatchC((*pdbfile)->rtype[i], "HOH"))
			(*pdbfile)->linetype[i]=PDBPARSE_COORDWAT;
		    else
		    {
			/* Mark up ligand coordinates */
			  
			(*pdbfile)->linetype[i]=PDBPARSE_COORDHET;



    
			/* New ligand */
			if(!ajStrMatch(htype[chn], (*pdbfile)->rtype[i]))
			{

			    offset++;
			    (*pdbfile)->nligands[chn]++;
			    (*pdbfile)->gpn[i] = (*pdbfile)->nligands[chn];
			    ajStrAssS(&htype[chn], (*pdbfile)->rtype[i]);
			}	
			/* More atoms of the same heterogen */
			else
			{
			    (*pdbfile)->gpn[i] = (*pdbfile)->nligands[chn];
			}
		    }
		}
		else
		    /* Assign any ATOM or HETATM records with a
		       non-whitespace chain id that does not appear in
		       the SEQRES records as a
		       NON_PROTEIN_CHAIN. Record the chain id's used
		       and assign a GROUP NUMBER as appropriate.*/
		{
		    /* Mark up water coordintes */
		    if(ajStrMatchC((*pdbfile)->rtype[i], "HOH"))
			(*pdbfile)->linetype[i]=PDBPARSE_COORDWAT;
		    else
		    {
			/* If there is a single chain only then the
			   group is of course associated with that
			   chain */
			if((*pdbfile)->nchains==1)
			{
			    /* Mark up ligand coordinates */
			    (*pdbfile)->linetype[i]=PDBPARSE_COORDHET;
			    (*pdbfile)->chnn[i]=1;
			    

			    for(done=ajFalse, j=0;j<(*pdbfile)->ngroups; j++)
				if(ajChararrGet((*pdbfile)->gpid, j) == id)
				{
				    (*pdbfile)->gpn[i]=ajIntGet(gpns, j);
				    done=ajTrue;
				    break;
				}
			    if(!done)
			    {
				ajIntPut(&gpns, j, (gpn=j+1+offset));
				(*pdbfile)->gpn[i]=gpn;

				ajChararrPut(&((*pdbfile)->gpid), 
					     (*pdbfile)->ngroups, id);
				(*pdbfile)->ngroups++;

				/* Increment number of ligands and reset last 
				   ligand type read in */
				(*pdbfile)->nligands[0]++;
				ajStrAssC(&htype[0], " ");
			    }
			}
			else
			{
			    /* Mark up ligand coordinates */
			    (*pdbfile)->linetype[i]=PDBPARSE_COORDGP;
			
			    for(done=ajFalse, j=0;j<(*pdbfile)->ngroups; j++)
				if(ajChararrGet((*pdbfile)->gpid, j) == id)
				{
				    (*pdbfile)->gpn[i]=j+1;
				    done=ajTrue;
				    break;
				}

			    if(!done)
			    {
				ajChararrPut(&((*pdbfile)->gpid), 
					     (*pdbfile)->ngroups, id);
				(*pdbfile)->ngroups++;
				(*pdbfile)->gpn[i]=j+1;
			    }
			}
		    }
		}
	    }
	}
	else if((*pdbfile)->linetype[i]==PDBPARSE_MODEL)
	{
	    mod++;

 	    doneter=ajFalse; 
	    for(j=0;j<(*pdbfile)->nchains; j++)
		chndone[j]=ajFalse;

	    doneoneter=ajFalse;
	}
	else if((*pdbfile)->linetype[i]==PDBPARSE_TER)
	{
	    chndone[this-1]=ajTrue;
	    

	    prev = this;
	    doneoneter=ajTrue;
	    doneter=ajTrue; 
	}	

	

	/* 
	   Check for missing TER records.  Where chains are not
	   separated by TER records (the chain id changes from line to
	   line without an intervening TER record and both chain id's
	   are not whitespace).  Where ATOM and HETATM groups are not
	   separated by TER records (a chain id is given on one line,
	   a whitspace chain id is given on the next line, and there
	   is no intervening TER record).
	   
	   This code is identical to a fragment from pdbparse_CheckTer.
	   
	   Note that chndone only is modified. If the code in the function was 
	   made to use prev, doneoneter, doneter then the code below would also
	   have to be modified
	   
	   Only do this now where there aren't enough TER records in
	   the file.  Without this check, it was failing for cases
	   where the order of chains is inconsistent (see around line
	   4095 of pdb1cm4.ent) */

	
	if((*pdbfile)->toofewter)
	{
	    
	    if(i>(*pdbfile)->idxfirst)
		if((*pdbfile)->linetype[i-1]==PDBPARSE_COORD  	&& 
		   (((*pdbfile)->linetype[i]==PDBPARSE_COORD)||
		    ((*pdbfile)->linetype[i]==PDBPARSE_COORDHET)) &&
		   ((ajStrChar((*pdbfile)->lines[i-1], POS_CHID)) !=
		    ajStrChar((*pdbfile)->lines[i], POS_CHID)))
		{
		    for(j=0;j<(*pdbfile)->nchains; j++)
			if(ajChararrGet((*pdbfile)->chid, j)
			   ==ajStrChar((*pdbfile)->lines[i-1], POS_CHID))
			{	
			    chndone[j]=ajTrue;	    
			    break;
			}
		}
	}
    }	


    /* The above code cannot cope for cases where the ATOM and HETATM records 
       use the same (or no) chain identifier and are not separated by a TER 
       record (e.g. 1rbp) 
       
       For files with less than the expected number of TER records, 
       check again for COORDHET lines, which are identified as a line
       beginning with a HETATM record with the same chain identifier 
       but lower residue number than the preceeding line. 
       */

    for(i=0;i<(*pdbfile)->nchains;i++)
	ajStrAssC(&htype[i], "\0");
    

    if((*pdbfile)->tercnt < ((*pdbfile)->nchains * (*pdbfile)->modcnt))
	for(i=(*pdbfile)->idxfirst+1;i<(*pdbfile)->nlines;i++)
	    if((*pdbfile)->linetype[i-1]==PDBPARSE_COORD  	&& 
	       (*pdbfile)->linetype[i]==PDBPARSE_COORD &&
	       ((ajStrChar((*pdbfile)->lines[i-1], POS_CHID)) ==
		ajStrChar((*pdbfile)->lines[i], POS_CHID)))
		if(ajStrPrefixC((*pdbfile)->lines[i],"HETATM"))
		    if((*pdbfile)->resn1[i]<(*pdbfile)->resn1[i-1])
			while((ajStrPrefixC((*pdbfile)->lines[i],"HETATM")))
			{			
			    if(ajStrMatchC((*pdbfile)->rtype[i], "HOH"))
				(*pdbfile)->linetype[i]=PDBPARSE_COORDWAT;
			    else
			    {
				(*pdbfile)->linetype[i]=PDBPARSE_COORDHET;

				/* New heterogen */
				if(!ajStrMatch(htype[chn],
					       (*pdbfile)->rtype[i]))
				{
				    offset++;
				    (*pdbfile)->nligands[chn]++;
				    (*pdbfile)->gpn[i] =
				      (*pdbfile)->nligands[chn];
				    ajStrAssS(&htype[chn],
					      (*pdbfile)->rtype[i]);
				}	
				/* More atoms of the same heterogen */
				else
				{
				    (*pdbfile)->gpn[i] =
				      (*pdbfile)->nligands[chn];
				}
			    }
			    i++;
			}

    /* For files with a single chain oly, set the number of groups
       that could not be associated with a chain to zero */
    if((*pdbfile)->nchains==1)
	(*pdbfile)->ngroups = 0;
    


    /* Tidy up and return */
    for(i=0;i<(*pdbfile)->nchains; i++)
	ajStrDel(&htype[i]);
    AJFREE(htype);
    
    ajIntDel(&gpns);

    AJFREE(chndone);
    return ajTrue;
}




/* @funcstatic pdbparse_MaskChains ********************************************
**
** Reads a Pdbfile object and checks to see whether the ATOM records
** for each chain contain sufficient amino acids. Any chains with
** insufficient amino acids either in the SEQRES or ATOM records, or
** with ambiguous chain id's are discarded. Optionally, amino acid
** residues and non-amino acid groups (e.g. ACE, NH2 etc) in protein
** chains with no CA atom are also discarded (the linetype array for
** the lines are set to PDBPARSE_IGNORE). For non-amino acid groups,
** the corresponding characters are removed from the sequence derived
** from the SEQRES records.  Coordinate data and atom type are parsed
** for each atom.  Optionally, amino acids or groups in protein chains
** with a single atom only are also discarded.
**
** Writes the x,y,z,o,b and atype elements of a Pdbfile object.  The linetype,
** and possibly seqres, seqresful and nres arrays are modified.
** 
** linetype array
** Coordinate data are extracted for lines of linetype PDBPARSE_COORD, 
** PDBPARSE_COORDHET and PDBPARSE_COORDGP.
**
** seqres & seqresful arrays
** Three-letter codes of any groups that are (i) not standard amino acids and 
** (ii) which do not contain a CA atom are removed from the seqres sequence if
** the <nocamask> is set. The seqresful array is an intermediate array to 
** achieve this.
** 
** @param [w] pdbfile         [AjPPdbfile*] Pdbfile object pointer
** @param [r] logf            [AjPFile]     Pointer to log file
**                                          (build diagnostics)
** @param [r] min_chain_size  [ajint]     Min. no. of amino acids in a chain
** @param [r] nocamask        [AjBool]      Whether to mask non-amino acid 
** residues within protein chains which do not have a C-alpha atom (remove them
** from the seqres sequence and set the linetype array for the lines 
** to PDBPARSE_IGNORE).
** @param [r] nocamask1        [AjBool]  Whether to mask amino acid residues
** within protein chains which do not have a C-alpha atom (set the linetype 
** array for the lines to PDBPARSE_IGNORE).
** @param [r] atommask        [AjBool]  Whether to mask residues or groups 
** with a single atom only.
** 
** @return [AjBool]  True on success, False otherwise
** @@
******************************************************************************/

static AjBool pdbparse_MaskChains(AjPPdbfile *pdbfile, AjPFile logf, 
				  ajint min_chain_size, 
				  AjBool nocamask, AjBool nocamask1, 
				  AjBool atommask)
{
    ajint  i=0;			/*Loop counter */
    ajint  j=0;			/*Loop counter */
    AjPStr aa3=NULL;		/*Amino acid */
    ajint  rcnt=0;		/* Residue count */
    ajint  acnt=0;		/* Atom count */
    ajint  modcnt=0;		/*Count of MODEL records */
    ajint  lastatm=0;		/*Line number of last coordinate line read in*/
    ajint  firstatm=0;		/*Line number of coordinate line for first 
				  atom of residue */
    AjBool noca=ajFalse;	/*True if this residue does not contain a 
				  CA atom */
    ajint  lastchn=0;		/*Chain number of last line read in*/
    AjBool *chainok;		/*Array of flags which are True if a chain 
				  in the SEQRES records is found in the ATOM 
				  records */
    char aa1=' ';		/*Amino acid id */
    AjPStr lastrn=NULL;		/*Number of last residue read in */
    AjBool msgdone=ajFalse;	/* Flag for message reporting */
    AjPStr sub=NULL;
    AjPStr tmpseq=NULL;
    ajint    lenful=0;          /* Length of SEQRES sequence including
				   ACE, FOR & NH2 groups that might be 
				   discarded by the call to 
				   pdbparse_SeqresToSequence */
    
    char   tmp=' ';             
    AjBool odd=ajFalse;         /* Whether the current residue / group
				   is of unknown type */
    
    



    /* Check args */
    if(!(*pdbfile) || !logf)
    {
	ajWarn("Bad args passed to pdbparse_MaskChains\n");
	return ajFalse;
    }
    
    /* Allocate memory */
    AJCNEW0(chainok, (*pdbfile)->nchains);
    for(i=0;i<(*pdbfile)->nchains;i++)
	chainok[i]=ajFalse;
    
    aa3=ajStrNew();
    lastrn=ajStrNew();
    sub=ajStrNew();
    ajStrAssC(&sub, "");
    
    
    firstatm=lastatm=(*pdbfile)->idxfirst;
    
    
    for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	if(((*pdbfile)->linetype[i]==PDBPARSE_COORD)    ||
	   ((*pdbfile)->linetype[i]==PDBPARSE_COORDHET) ||
	   ((*pdbfile)->linetype[i]==PDBPARSE_COORDGP)  ||
	   ((*pdbfile)->linetype[i]==PDBPARSE_COORDWAT))
	{	

	    /* Assign x, y, z, o, b data */
	    /* JISON replace this with code so that internals of structure 
	       are not probed (when code becomes available)*/
	    if( (sscanf(&((*pdbfile)->lines[i]->Ptr[28]), "%f %f %f %f", 
			&((*pdbfile)->x[i]), 
			&((*pdbfile)->y[i]), 
			&((*pdbfile)->z[i]), 
			&((*pdbfile)->o[i]))) != 4)
		ajFatal("Scan error in pdbparse_MaskChains\n"
			"Email jison@hgmp.mrc.ac.uk");

	    if(!sscanf(&((*pdbfile)->lines[i]->Ptr[60]), "%f", 
		       &((*pdbfile)->b[i])))
		ajFatal("Scan error in pdbparse_MaskChains\n"
			"Email jison@hgmp.mrc.ac.uk");
	    	    
	    /* Usually position 12 is used for the alternative position 
	       indicator (taken in the code below to be indicated by a 
	       number) for atoms, but occasionally can be incorrectly 
	       used for the atom type itself (indicated in the code below 
	       by a character in pos 12). This code copes for both cases */
	    /* Assign atom type */
	    if(isalpha((int)(*pdbfile)->lines[i]->Ptr[12]))
	    {
		ajStrAssSub(&(*pdbfile)->atype[i],
			    (*pdbfile)->lines[i], 12, 15);
		ajStrCleanWhite(&(*pdbfile)->atype[i]);
		if(!msgdone)
		{
		    ajFmtPrintF(logf, "%-15s%d\n", "ATOMCOL12", i+1);
		    msgdone=ajTrue;
		}
	    }
	    else
	    {
		ajStrAssSub(&(*pdbfile)->atype[i],
			    (*pdbfile)->lines[i], 13, 15);
		ajStrCleanWhite(&(*pdbfile)->atype[i]);
	    }
	   
	}
	
	


	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{	
	    /* Check whether CA atom is present */
	    if(!(ajStrCmpC((*pdbfile)->atype[i],  "CA")))
		noca=ajFalse;


	    if((*pdbfile)->chnn[i]<lastchn)
		ajFmtPrintF(logf, "%-15s%d\n", "CHAINORDER", i+1);
	    if((*pdbfile)->chnn[i]!=lastchn)
	    {
		rcnt=0;
		lastchn=(*pdbfile)->chnn[i];
	    }
	    

	    /* If this is a new residue */
	    if(!ajStrMatch((*pdbfile)->pdbn[i], lastrn))
	    {
		/* Mask coordinate lines where there are only a single atom */
		if(acnt==1)
		{
		    ajFmtPrintF(logf, "%-15s%d\n", "ATOMONEONLY", lastatm+1);
		    if(atommask)
			{
			    (*pdbfile)->linetype[lastatm]=PDBPARSE_IGNORE;
			}
		    
		}
		
		/* Mask coordinate lines for residues lacking a CA atom */
		if(noca)
		{
		    odd = (!(pdbparse_BaseAa3ToAa1(&tmp,
					  (*pdbfile)->rtype[lastatm-1])));
		    
		    if((nocamask  && odd) ||
		       (nocamask1 && !odd))
			for(j=firstatm; j<=lastatm; j++)
			    (*pdbfile)->linetype[j]=PDBPARSE_IGNORE; 

			/* Remove residues from SEQRES records */
		    if((nocamask  && odd))
       ajStrSubstitute(&(*pdbfile)->seqresful[(*pdbfile)->chnn[firstatm]-1], 
					(*pdbfile)->rtype[firstatm], sub);

		    
		    if(firstatm == lastatm)
			{
			    ajFmtPrintF(logf, "%-15s%d\n", "ATOMNOCA",
					firstatm+1);
			    
			}
		    
		    else
			ajFmtPrintF(logf, "%-15s%d %d\n", "ATOMNOCA", 
				    firstatm+1, lastatm+1);
		}
		
		
		/* Increment the residue counter if the code is recognised */
		if(pdbparse_BaseAa3ToAa1(&aa1, (*pdbfile)->rtype[i]))
		    rcnt++;	
		
		if(rcnt>=min_chain_size)
		    chainok[(*pdbfile)->chnn[i]-1]=ajTrue;
		
		ajStrAssS(&lastrn, (*pdbfile)->pdbn[i]);
		
		/* Set count of atoms to zero, set the position of the
		   first atom and set flag for recognising CA atom*/
		acnt=1;
		firstatm=i;	
		
		if(!(ajStrCmpC((*pdbfile)->atype[i],  "CA")))
		    noca=ajFalse;
		else
		    noca=ajTrue;
	    }


	    /* Set the position for the last atom read in */
	    lastatm=i;
	    
	    
	    /* Increment the atom counter */
	    acnt++;
	}	
    	else if((*pdbfile)->linetype[i]==PDBPARSE_MODEL)
	{
	    rcnt=0;
	    lastchn=0;
	    
	    modcnt++;
	    if(modcnt!=1)
	    {
		for(j=0;j<(*pdbfile)->nchains;j++)
		{	
		    /* Only bother reporting error messages if a message
		       about the SEQRES records not containing enough aa's
		       has not already been reported */
		    /* If (*pdbfile)->chainok is False, leave it so  */
		    if(!(*pdbfile)->chainok[j])
			continue;
		    else
		    {
			if(!chainok[j])
			{
			    (*pdbfile)->chainok[j]=chainok[j];
			    ajFmtPrintF(logf, "%-15s%d (%c) %d\n", 
					"ATOMFEWAA", j+1,
					(ajChararrGet((*pdbfile)->chid, j)),
					modcnt);
			    			    
			}
		    }
		}
	    }

	}
    }
	/* Ensure that C-terminal residues are masked if necessary */
/*	else if( ((*pdbfile)->linetype[i]==PDBPARSE_TER)||
		((*pdbfile)->linetype[i]==PDBPARSE_ENDMDL)) 
	{*/
	    if(noca)
	    {
		odd = (!(pdbparse_BaseAa3ToAa1(&tmp,
					       (*pdbfile)->rtype[lastatm-1])));
		

		if((nocamask  && odd) ||
		   (nocamask1 && !odd))
		    for(j=firstatm; j<=lastatm; j++)
			(*pdbfile)->linetype[j]=PDBPARSE_IGNORE; 

		    /* Remove residues from SEQRES records */
		if((nocamask  && odd))
	ajStrSubstitute(&(*pdbfile)->seqresful[(*pdbfile)->chnn[firstatm]-1], 
				    (*pdbfile)->rtype[firstatm], sub);

		if(firstatm == lastatm)
		  {
		    ajFmtPrintF(logf, "%-15s%d\n", "ATOMNOCA", firstatm+1);
		    
		}

		else
		    ajFmtPrintF(logf, "%-15s%d %d\n", "ATOMNOCA", 
				firstatm+1, lastatm+1);
	    }

/*	}	 */





    /* Write the new (masked) seqres sequences if necessary */
	    
     if(nocamask)
     {
	for(i=0;i<(*pdbfile)->nchains;i++)
	{	
	    tmpseq=ajStrNew();	 
	
	    if(!pdbparse_SeqresToSequence((*pdbfile)->seqresful[i], 
					&tmpseq, nocamask, &lenful))
	    {
		ajFatal("Sequence conversion error in "
			"pdbparse_FirstPass\nEmail jison@hgmp.mrc.ac.uk\n");
	    }
	    ajStrAssS(&(*pdbfile)->seqres[i], tmpseq);	
	    (*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);
	    
	    ajStrDel(&tmpseq);
	}
    }
    
    

    /* Set modcnt to 1 for xray structures */
    if(!modcnt)
	modcnt=1;
    
    
    /*Check for xray structures or last model of nmr structures */
    for(i=0;i<(*pdbfile)->nchains;i++)
    {	
	/* Only bother reporting error messages if a message
	   about the SEQRES records not containing enough aa's
	   has not already been reported */
	/* If (*pdbfile)->chainok is False, leave it so  */
	if(!(*pdbfile)->chainok[i])
	    continue;
	else
	{
	    if(!chainok[i])
	    {
		(*pdbfile)->chainok[i]=chainok[i];
		ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "ATOMFEWAA", 
			    i+1,(ajChararrGet((*pdbfile)->chid, i)),modcnt);

	    }
	}
    }


    /* Mask out any chains with insufficient amino acids either in the 
       SEQRES or ATOM records */
    for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
	if((((*pdbfile)->linetype[i]==PDBPARSE_COORD)    ||
	    ((*pdbfile)->linetype[i]==PDBPARSE_COORDHET)) &&
	   (!(*pdbfile)->chainok[(*pdbfile)->chnn[i]-1]))
	{
	    (*pdbfile)->linetype[i]=PDBPARSE_IGNORE;


	}		
    
    
    
    /* Check for missing TER records.  Where chains are not separated
       by TER records (the chain id changes from line to line without
       an intervening TER record and both chain id's are not
       whitespace).  Where ATOM and HETATM groups are not separated by
       TER records (a chain id is given on one line, a whitspace chain
       id is given on the next line, and there is no intervening TER
       record) */
    
    
    for(i=(*pdbfile)->idxfirst+1;
	i<(*pdbfile)->nlines;i++)
    {
	if(((*pdbfile)->linetype[i-1]==PDBPARSE_COORD) && 
	   ((*pdbfile)->linetype[i]==PDBPARSE_COORD) &&
	   (*pdbfile)->chnn[i-1] != (*pdbfile)->chnn[i])
	    ajFmtPrintF(logf, "%-15s%d %d\n", "TERMISSCHN", i, i+1);
	else if(((*pdbfile)->linetype[i-1]==PDBPARSE_COORD) && 
		(((*pdbfile)->linetype[i]==PDBPARSE_COORDHET) || 
		 ((*pdbfile)->linetype[i]==PDBPARSE_COORDWAT))&&
		(*pdbfile)->chnn[i-1] == (*pdbfile)->chnn[i])
	    ajFmtPrintF(logf, "%-15s%d %d\n", "TERMISSHET", i, i+1);
    }
    


    /* Tidy up and return */
    AJFREE(chainok);
    ajStrDel(&aa3);
    ajStrDel(&lastrn);
    ajStrDel(&sub);

    return ajTrue;
}






/* @funcstatic pdbparse_StandardiseNumbering **********************************
**
** Reads a Pdbfile object and standardises the two sets of residue numbers 
** (resn1 & resn2 arrays) derived from the raw residue numbers. The residue 
** numbering is corrected for zero or negative residue numbers, non-standard 
** numbering schemes and any other cases of non-sequentiality. 
** resn1 gives the sequence presuming an alternative numbering scheme, resn2 
** gives the sequence presuming heterogeneity. Heterogeneity is indicated by 
** a character in position lines[26] (the same position used to indicate 
** alternative residue numbering schemes).
**
** The resn1 & resn2 arrays of a Pdbfile object are modified.  The oddnum 
** array is written. 
** 
** oddnum array
** This is an array of Bool's which are TRUE for duplicate residues of 
** heterogenous positions (e.g. if 2 different residues are both numbered '8' 
** or one is '8' and the other '8A' for example then <oddnum> would be set 
** True for the second residue. 
**
** @param [w] pdbfile         [AjPPdbfile*] Pdbfile object pointer
** @param [r] logf            [AjPFile]     Pointer to log file
**                                          (build diagnostics)
** 
** @return [AjBool]  True on success, False otherwise
** @@
******************************************************************************/
static AjBool pdbparse_StandardiseNumbering(AjPPdbfile *pdbfile, AjPFile logf)
{	
    ajint   i=0;
    ajint   lastchn=-1;			/* Chain number of last line read in */
    AjBool  first=ajFalse;		/* True if we have processed the first 
					   residue in a chain */
    ajint   first_num=0;		/* Number of first residue in chain */
    ajint   modrn=0;			/* Corrected first residue number */
    AjBool  neg=ajFalse;		/* True if first residue number is 
					   negative */
    AjBool  zer=ajFalse;		/* True if first residue
                                           number is zero */
    AjBool  report_neg=ajFalse;		/* True if we have reported an error 
					   that a residue number is negative 
					   for this chain*/
    AjBool  report_zer=ajFalse;		/* True if we have reported an error 
					   that a residue number is zero 
					   for this chain*/
    ajint   add=0;			/* An amount to add to the residue 
					   numbers to correct them */
    AjBool  ignore=ajFalse;
    ajint   rn=0;			/* Current residue number */
    ajint   last_rn=0;			/* Last residue number read in */    
    ajint   this_rn=0;			/* Current residue number read in */
    char    last=' ';			/* Chain id of last chain */
    char    curr=' ';			/* Chain id of current chain */
    AjPStr  last_rt=NULL;		/* Type of previous residue */
    AjPStr  this_rt=NULL;		/* Type of current residue */
    AjBool  report_nonstd=ajFalse;	/* True if we have reported an error 
					   that a non-standard
					   residue numbering scheme is used 
					   for this chain */
    AjBool  report_nonsqt=ajFalse;	/* True if we have reported an error 
					   that any other cases
					   of non-sequential numbering are 
					   found for this this chain */
    char     aa1=' ';			/* Amino acid single character code*/


    




    last_rt=ajStrNew();
    this_rt=ajStrNew();
    
 

    /* Check args */
    if(!(*pdbfile) || !logf)
    {
	ajWarn("Bad args passed to pdbparse_StandardiseNumbering\n");
	return ajFalse;
    }    


    /* Check whether the integer part of the original pdb 
       numbering (at this point in code held in resn1 and
       resn2) gives the correct index into the SEQRES sequence */
    for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    { 
	/* If model number has gone past 1 then we must have checked
	   all the chains, so break */
	if((*pdbfile)->modn[i] > 1)
	    break;

	if((*pdbfile)->linetype[i]!=PDBPARSE_COORD)
	    continue;
	

	/* If residue number is not negative, zero, or greater then the length
	   of the SEQRES sequence and if the residue matches then continue */
	 
	if(((*pdbfile)->resn1[i] <= (*pdbfile)->nres[(*pdbfile)->chnn[i]-1])||
	   ((*pdbfile)->resn1[i] >=  1))
	{
	    pdbparse_BaseAa3ToAa1(&aa1, (*pdbfile)->rtype[i]);
	    if(aa1 == ajStrChar((*pdbfile)->seqres[(*pdbfile)->chnn[i]-1], 
				(*pdbfile)->resn1[i]-1))
		continue;
	}
	
	/* Otherwise flag an error for this chain and move to the 
	   end of the chain */

	ajFmtPrintF(logf, "%-15s%d (%c)\n", "BADINDEX",  
		    (*pdbfile)->chnn[i], 
		    ajChararrGet((*pdbfile)->chid,(*pdbfile)->chnn[i]-1));
	
	
	for(lastchn=(*pdbfile)->chnn[i]; i<(*pdbfile)->nlines;i++)
	{
	    if((*pdbfile)->linetype[i]!=PDBPARSE_COORD)
		continue;

	    if((*pdbfile)->modn[i] > 1)
		break;
	    if((*pdbfile)->chnn[i] != lastchn)
	    {
		i--;
		break;
	    }
	}
    } 
    



    /* Fix for zero or negative residue numbers.
       This is done for both resn1 and resn2 arrays of 
       a Pdbfile object */
    
    for(first=ajFalse, i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	/* Co-ordinate line */
	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{
	    /* New chain */
	    if((*pdbfile)->chnn[i]!=lastchn)
	    {
		neg=ajFalse;
		zer=ajFalse;
		report_neg=ajFalse;
		report_zer=ajFalse;
		first=ajFalse;
		lastchn=(*pdbfile)->chnn[i];
	    }
	    
	    rn=(*pdbfile)->resn1[i];
	    

	    if(!first)
	    {
		first_num = rn;
		if(first_num > 0)
		{
		    /* Advance counter to next chain */
		    for( ;i<(*pdbfile)->nlines;i++)
			if((((*pdbfile)->linetype[i]==
			     PDBPARSE_COORD)&&((*pdbfile)->chnn[i]!=lastchn))
			   || (*pdbfile)->linetype[i]==PDBPARSE_MODEL)
			{
			    neg=ajFalse;
			    zer=ajFalse;
			    report_neg=ajFalse;
			    report_zer=ajFalse;
			    first=ajFalse;
			    lastchn=-1;
			    
			    break;
			}
		    /* i will get incremented in main loop above */
		    i--;		
		    continue;
		}
		
		first=ajTrue;	
	    }
	    if(rn < 0) 
	    {		
		neg=ajTrue;
		if(zer)
		    modrn=rn-(first_num-1);
		else			/* if(neg && !zer) */
		    modrn=rn-(first_num-1);

		if(!report_neg)
		{
		    ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "NEGNUM", 
				(*pdbfile)->chnn[i], 
				ajChararrGet((*pdbfile)->chid, 
					     ((*pdbfile)->chnn[i]-1)),i+1);
		    report_neg=ajTrue;
		}
	    }
	    else if (rn == 0)
	    {
		zer=ajTrue;
		if(neg)
		    modrn=rn-(first_num-1);
		else			/* if(!neg) */
		    modrn=rn+1;

		if(!report_zer)
		{	
		    ajFmtPrintF(logf, "%-15s%d (%c) %d\n", 
				"ZERNUM", (*pdbfile)->chnn[i], 
				ajChararrGet((*pdbfile)->chid, 
					     ((*pdbfile)->chnn[i]-1)),i+1);
		    report_zer=ajTrue;
		}
	    }
	    else			/*rn is (+ve)*/
	    {
		if(!neg && zer)
		    modrn=rn+1;
		else 	if(neg && zer)
		    modrn=rn-(first_num-1);
		else			/* if(neg && !zer) */
		    modrn=rn-(first_num);
	    }

	    (*pdbfile)->resn1[i]=modrn;
	    (*pdbfile)->resn2[i]=modrn;
	}
	/* New model */
	else if((*pdbfile)->linetype[i]==PDBPARSE_MODEL)
	{
	    neg=ajFalse;
	    zer=ajFalse;
	    report_neg=ajFalse;
	    report_zer=ajFalse;
	    first=ajFalse;
	    lastchn=-1;
	}	
    }
    

    


    
    /* Fix non-standard residue numbering scheme.
       This is done for resn1 array of a Pdbfile object only. */
    for(lastchn=-1, i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	/* Co-ordinate line */
	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{
	    /* New chain */
	    if((*pdbfile)->chnn[i]!=lastchn)
	    {
		add = 0;
		first=ajFalse;
		ignore=ajFalse;
		lastchn=(*pdbfile)->chnn[i];
		report_nonstd=ajFalse;		
	    }

	    if(!first)
	    {
		/*Remove the chmyotrypsin numbering code*/
		last_rn = (*pdbfile)->resn1[i];
		last=ajStrChar((*pdbfile)->lines[i], 26);
		first=ajTrue;
		continue;
	    }
	    
	    rn = (*pdbfile)->resn1[i];
	    curr=ajStrChar((*pdbfile)->lines[i], 26);

	    if(curr!=last)
	    {
		if(rn == last_rn) 
		{
		    add++;
		    ignore=ajTrue;
		}
	    }
	    if(rn != last_rn) 
	    {
		ignore = ajFalse;
	    }
	      
	    last=curr;
	    last_rn = rn;

	    (*pdbfile)->resn1[i]=rn+add;
	    if(ignore)
	    {
		(*pdbfile)->oddnum[i]=ajTrue;
		if(!report_nonstd)
		{
		    ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "ODDNUM", 
				(*pdbfile)->chnn[i], 
				ajChararrGet((*pdbfile)->chid, 
					     ((*pdbfile)->chnn[i]-1)),i+1);
		    report_nonstd=ajTrue;		
		}
		    
	    }
	    
	}
	else if((*pdbfile)->linetype[i]==PDBPARSE_MODEL)
	{
	    add = 0;
	    first=ajFalse;
	    ignore=ajFalse;
	    lastchn=-1;
	    report_nonstd=ajFalse;		
	}
    }
    




    

    /* Fix remaining non-sequential residue numbering in resn1
       array of Pdbfile object*/
    for(lastchn=-1, i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	/* Co-ordinate line */
	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{
	    /* New chain */
	    if((*pdbfile)->chnn[i]!=lastchn)
	    {
		add = 0;
		first=ajFalse;
		ignore=ajFalse;
		lastchn=(*pdbfile)->chnn[i];
		report_nonsqt=ajFalse;
	    }

	    if(!first)
	    {
		last_rn=(*pdbfile)->resn1[i];
		ajStrAssS(&last_rt, (*pdbfile)->rtype[i]);

		first=ajTrue;
		continue;
	    }
	    
	    this_rn=(*pdbfile)->resn1[i];
	    ajStrAssS(&this_rt, (*pdbfile)->rtype[i]);


	    /*If this ATOM is 'N' or if this is a different residue type*/
	    if( !(ajStrCmpC((*pdbfile)->atype[i],  "N")) || 
	       !(ajStrMatch(this_rt, last_rt)))
	    {
		if(this_rn == last_rn)
		{
		    add++;
		    ignore=ajTrue;
		}	
	    }


	    if(this_rn != last_rn)
		ignore = ajFalse;

	    (*pdbfile)->resn1[i]=this_rn+add;


	    

	    ajStrAssS(&last_rt, this_rt);
	    last_rn = this_rn;
	    

	    if(ignore)
	    {
		(*pdbfile)->oddnum[i]=ajTrue;
		if(!report_nonsqt)
		{
		    ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "NONSQNTL", 
				(*pdbfile)->chnn[i], 
				ajChararrGet((*pdbfile)->chid, 
					     ((*pdbfile)->chnn[i]-1)),i+1);
		    report_nonsqt=ajTrue;		
		}
	    }
	}
	
	else if((*pdbfile)->linetype[i]==PDBPARSE_MODEL)    
	{
	    add = 0;
	    first=ajFalse;
	    ignore=ajFalse;
	    lastchn=-1;
	    report_nonsqt=ajFalse;
	}
    }
    






    /* Fix remaining non-sequential residue numbering in resn2
       array of Pdbfile object (duplicate lines for presumed 
       heterogenous residues positions are ignored)*/
    for(lastchn=-1, i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	/* Co-ordinate line */
	if(((*pdbfile)->linetype[i]==PDBPARSE_COORD) && 
	   (!(*pdbfile)->oddnum[i]))
	{
	    /* New chain */
	    if((*pdbfile)->chnn[i]!=lastchn)
	    {
		add = 0;
		first=ajFalse;
		lastchn=(*pdbfile)->chnn[i];
	    }

	    if(!first)
	    {
		last_rn=(*pdbfile)->resn2[i];
		ajStrAssS(&last_rt, (*pdbfile)->rtype[i]);

		first=ajTrue;
		continue;
	    }
	    this_rn=(*pdbfile)->resn2[i];
	    ajStrAssS(&this_rt, (*pdbfile)->rtype[i]);


	    /*If this ATOM is 'N' or if this is a different residue type*/
	    if(!(ajStrCmpC((*pdbfile)->atype[i],  "N")) || 
	       !(ajStrMatch(this_rt, last_rt)))
	    {
		if(this_rn == last_rn)
		{
		    add++;
		}	
	    }

	    (*pdbfile)->resn2[i]=this_rn+add;
	    ajStrAssS(&last_rt, this_rt);
	    last_rn = this_rn;
	}
	else if ((*pdbfile)->linetype[i]==PDBPARSE_MODEL)   
	{
	    add = 0;
	    first=ajFalse;
	    lastchn=-1;
	}
    }





    
    /* Tidy up and return */
    ajStrDel(&last_rt);
    ajStrDel(&this_rt);

    return ajTrue;
}





/* @funcstatic pdbparse_AlignNumbering ****************************************
**
** Reads a Pdbfile object and determines for each chain a set of residue 
** numbers (the resni array) that give the correct index into the full length 
** (SEQRES) sequence for residues listed in the ATOM records.    
**
** The resni and resn1ok arrays of a Pdbfile object are written. 
** The seqres and nres elements may be modified for any missing N-terminal 
** residues.
**
** resn1ok array
** This array contains Bool's for each chain which are TRUE if resn1 was 
** used to derive resni, i.e. gave correct alignment to the full-length 
** (SEQRES) sequence.  If False then resn2 was used ( resn1 gives the sequence 
** presuming an alternative numbering scheme, resn2 gives the sequence 
** presuming heterogeneity).
**
** @param [w] pdbfile         [AjPPdbfile*] Pdbfile object pointer
** @param [r] logf            [AjPFile]     Pointer to log file
**                                          (build diagnostics)
** @param [r] lim             [ajint]       Max. no. permissible mismatches
**                                          between the ATOM & SEQRES
**                                          sequences.
** 
** @return [AjBool]  True on success, False otherwise
** @@
** JISON : Must comment what diagnostics this writes
******************************************************************************/
static AjBool pdbparse_AlignNumbering(AjPPdbfile *pdbfile, AjPFile logf,
				      ajint lim)
{
    /* Sequence and residue number data are given for each unique chain 
       (data for the first model only is considered when assigning residue
       numbers) */
       

    /* JISON ... Rather than use, e.g. seq1, seq2 & seq, we could use a
       single 2 dimensional array, but we would need new code for 2d
       arrays of chararacters*/
    

    ajint       a=0;		/* Loop counter */
    ajint       i=0;		/* Loop counter */
    ajint       j=0;		/* Loop counter */
    ajint       k=0;		/* Loop counter */
    ajint       x=0;		/* Loop counter */
    ajint       y=0;		/* Loop counter */
    ajint       z=0;		/* Loop counter */


    AjPStr  *seq1=NULL;		/* Sequences of residues from ATOM 
				   records (all residues) */
    AjPStr  *seq2=NULL;		/* Sequences of residues from ATOM 
				   records (excluding residues
				   for which oddnum array in Pdbfile 
				   object is True) */
    AjPStr  *seq=NULL;		/* Pointer to seq1 or seq2 */
    

    ajint   *nres1=NULL;	/* No. residues for seq1/arr1 */
    ajint   *nres2=NULL;	/* No. residues for seq2/arr2 */
    ajint   *nres=NULL;		/* Pointer to nres1 or nres2 */
    

    AjPInt  *num1=NULL;		/* Residue numbers for seq1 (from
                                   resn1 element of the Pdbfile
                                   object) */
    AjPInt  *num2=NULL;		/* Residue numbers for seq2 (from
                                   resn2 element of the Pdbfile
                                   object) */
    AjPInt  *num=NULL;		/* Pointer to num1 or num2 */
    
    AjPInt  *idx=NULL;		/* Gives correct index into seqres sequence 
				   (from Pdbfile object) for the 
				   current sequence. These are residue numbers 
				   and therefore idx would 
				   have a value of 1 for the first seqres 
				   residue.*/
    AjPInt  *idx_full=NULL;	/* As idx but with empty array elements 
				   replacing missing residues
				   so that we can index into idx_full using 
				   residue numbers from num */


    ajint    last1=-1000;	/* Number of last residue for seq1/arr1*/
    ajint    last2=-1000;	/* Number of last residue for seq2/arr2*/
    
    char     aa1=' ';		/* Amino acid single character code*/
    ajint    c=0;		/* No. of current chain */
    
    AjBool   done=ajFalse;	/* True if we have found the correct residue 
				   numbering */
    
    char    *insert=NULL;	/* String from N-terminus of ATOM sequence to 
				   insert at N-terminus of SEQRES sequence in 
				   case of the later missing residues */
    AjPStr   tmpseqres=NULL;	/* Temp. string for seqres sequence from 
				   Pdbfile object */
    AjPStr   bit=NULL;	        /* Temp. string for a bit of sequence */
    ajint    nmismatches=0;	/* No. of mismatches between ATOM and SEQRES 
				   sequence */
    ajint    loc=0;		/* Location of ATOM sequence in SEQRES
				   sequence (if applicable) */
    ajint    len=0;		/* Length of seqres sequence from
                                   Pdbfile object*/
    AjBool   err=ajFalse;	/* True if a residue number from the ATOM 
				   records would cause
				   an array boundary error in the seqres 
				   sequence */
    ajint siz_substr=0;		/* Size of substring for alignment of ATOM and 
				   SEQRES sequences */
    char *atm_ptr=NULL; 	/* Pointer to ATOM sequence */
    char *seqres_ptr=NULL;	/* Pointer to SEQRES sequence */
    char *loc_ptr=NULL;		/* Pointer for location of match of
				   substring to SEQRES sequence*/
    AjPStr substr=NULL;		/* Substring of ATOM sequence */
    AjPStr substr2=NULL;	/* Substring of ATOM sequence */
    ajint  atm_idx=0;		/* Index into ATOM sequence */
    ajint  seqres_idx=0;	/* Index into SEQRES sequence */
    ajint  seqres_idx_last=0;	/* Index into SEQRES sequence for C-terminal 
				   residue of substring*/		
    char   aa_last=' ';         /* Amino acid residue code of
				   C-terminal residue of substring*/
    AjBool fixed=ajFalse;       /* Whether the mismatch residue of the
				   substring was later aligned correctly */
    AjBool done_end=ajFalse;	/* True if we have aligned the terminus of the 
				   ATOM sequence */
    AjBool founderr=ajFalse;    /* Match of substring of ATOM sequence
				   to SEQRES found with potential
				   mismatched residue */
    AjPStr  msgstr=NULL;	/* A string to hold a message */
    AjPStr  msgbit=NULL;	/* A temp. string to hold part of a message */
    ajint  idx_misfit_atm=0;    /* Index into ATOM sequence (seq) for
				   first residue that does not match
				   SEQRES sequence*/
    
    ajint  idx_misfit_seqres =0;/* Index into SEQRES sequence for
				   first residue that does not match
				   ATOM sequence */
    AjPStr aa_misfit=NULL;	/* Original (PDB) residue number for first 
				   residue mismatch between ATOM and SEQRES 
				   sequences */
    ajint  this_num = 0;	/* Current residue number */
    
    
    
    
    
    
    
    
    /* Check args */
    if(!(*pdbfile) || !logf)
    {
	ajWarn("Bad args passed to pdbparse_AlignNumbering\n");
	return ajFalse;
    }
    
    
    
    /* Allocate memory for arrays etc*/
    aa_misfit=ajStrNew();
    msgstr=ajStrNew();
    msgbit=ajStrNew();
    

    insert=ajCharNewL(MAXMISSNTERM);
    tmpseqres=ajStrNew();
    bit=ajStrNew();
    substr=ajStrNew();    
    substr2=ajStrNew();    
    
    
    AJCNEW0(seq1, (*pdbfile)->nchains);
    AJCNEW0(seq2, (*pdbfile)->nchains);
    
    
    AJCNEW0(num1, (*pdbfile)->nchains);
    AJCNEW0(num2, (*pdbfile)->nchains);
    
    AJCNEW0(idx, (*pdbfile)->nchains);
    AJCNEW0(idx_full,  (*pdbfile)->nchains);
    
    AJCNEW0(nres1, (*pdbfile)->nchains);
    AJCNEW0(nres2, (*pdbfile)->nchains);
    
    
    
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	if(!(*pdbfile)->chainok[i])
	    continue;
	

	seq1[i]=ajStrNew();
	seq2[i]=ajStrNew();
	


	num1[i]=ajIntNew();
	num2[i]=ajIntNew();
    }


    
    
    /* Assign arrays */
    for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	/* Co-ordinate line */
	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{
	    /* Break if we are no longer in the first model */
	    if((*pdbfile)->modn[i] != 1)
		break;
	    else
		c=(*pdbfile)->chnn[i]-1;
	       

/*	    ajFmtPrint("%S\n", (*pdbfile)->lines[i]); */
	    

	    if((*pdbfile)->resn1[i] != last1)
	    {
		pdbparse_BaseAa3ToAa1(&aa1, (*pdbfile)->rtype[i]);
		ajStrAppK(&seq1[c], aa1);
		
		ajIntPut(&num1[c], nres1[c],(*pdbfile)->resn1[i]);
		last1=(*pdbfile)->resn1[i];
		nres1[c]++;
	    }
	    if(((*pdbfile)->resn2[i] != last2) && (!(*pdbfile)->oddnum[i]))
	    {
		pdbparse_BaseAa3ToAa1(&aa1, (*pdbfile)->rtype[i]);
		ajStrAppK(&seq2[c], aa1);
		
		ajIntPut(&num2[c], nres2[c],(*pdbfile)->resn2[i]);
		last2=(*pdbfile)->resn2[i];
		nres2[c]++;
	    }
	}
    }
    
    

    /* Allocate memory for arrays of residue numbers */
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	if(!(*pdbfile)->chainok[i])
	    continue;


	/* Array must be big enough to cope with either sequence */
	if(nres1[i] > nres2[i])
	    idx[i]=ajIntNewL(nres1[i]);
	else
	    idx[i]=ajIntNewL(nres2[i]);

	

	/* Array must be big enough to cope with highest the residue 
	   number from either array */
	if(ajIntGet(num1[i], nres1[i]-1) > ajIntGet(num2[i], nres2[i]-1))
	{
	    idx_full[i]=ajIntNewL(ajIntGet(num1[i], nres1[i]-1)+1);
	}
	
	else
	{
	    idx_full[i]=ajIntNewL(ajIntGet(num2[i], nres2[i]-1)+1);
	}

    }
    
    

    
    /* Loop for each chain */
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	
	/* Skip this chain if necessary */
	if(!((*pdbfile)->chainok[i]))
	    continue;
	else
	    ajStrAssS(&tmpseqres, (*pdbfile)->seqres[i]);


	/* Loop for checking for missing residues from N-term of
           SEQRES sequence */
	for(done=ajFalse, j=0;j<MAXMISSNTERM+1;j++)
	{
	    /* Loop for the 2 sequences derived from the ATOM records */
	    for(x=0;x<2;x++)
	    {
		if(x==0)
		{
		    seq=seq1;
		    nres=nres1;
		    num=num1;
		}
		else
		{
		    seq=seq2;
		    nres=nres2;
		    num=num2;
		}	

		
		/* Restore the original seqres sequence */
		ajStrAssS(&((*pdbfile)->seqres[i]), tmpseqres);
		(*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);
		
		
		/* Modify the seqres sequence in the Pdbfile object 
		   by adding the first <j> residues from the N-terminus 
		   of the ATOM sequence to the N-terminus of <seqres>. */
		
		for(k=0;(k<j)&&(k<nres[i]);k++)
		    insert[k]=ajStrChar(seq[i], k);
		insert[k]='\0';

		ajStrInsertC(&((*pdbfile)->seqres[i]), 0, insert);
		(*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);
		
		
		

		/* JISON DIAGNOSTIC CODE 
		   
		   ajFmtPrintF(tempfile,
		   "\nChainMod %d (seq %d)\n%S\n%S\n\n\n", 
		   i+1,x+1,
		   seq[i], 
		   (*pdbfile)->seqres[i]);
		   */
		    

		/***********************************************/
		/******************* STEP 1 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrint("STEP1 tmpseqres: %S\n", tmpseqres); 
		  
		  ajFmtPrint("chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i],
		  (*pdbfile)->seqres[i]); 
		  printf("\n");
		  if(ajStrMatch(seq1[i], seq2[i]))
		  printf("seq1 and seq2 match\n");
		  else
		  printf("seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		  printf("seq1 and seqres match\n");
		  else
		  printf("seq1 and seqres DO NOT match\n");
		  printf("\n");
		  */

		/* Check whether sequences are identical length*/
		if(nres[i]==(*pdbfile)->nres[i])
		{
		    /* Sequences are identical - assign residue
                       numbers 'by hand' */
		    if(ajStrMatch(seq[i], (*pdbfile)->seqres[i]))
		    {
			for(k=0;k<nres[i];k++)
			    ajIntPut(&idx[i], k, k+1);
			
			if(x==0)
			    (*pdbfile)->resn1ok[i]=ajTrue;
			else
			    (*pdbfile)->resn1ok[i]=ajFalse;		       
			
			done=ajTrue;

			/*DIAGNOSTIC		     printf("STEP1 OK\n"); */
			
			break;
		    }
		    /* Sequence are same length but contain mismatches */
		    else
		    {	
			for(ajStrAssC(&msgstr, ""), nmismatches=0, k=0;
			    k<nres[i];k++)
			    if(ajStrChar(seq[i], k) != 
			       ajStrChar((*pdbfile)->seqres[i], k))
			    {
				nmismatches++;
				/* Correct the seqres sequence.  JISON
				   replace this with appropriate
				   library call once available so we
				   don't have to probe the internals
				   of the structure */


				/* a will give the number of the first
				   coordinate line for the mismatch
				   residue from the ATOM records*/
				a = pdbparse_PdbfileFindLine(*pdbfile, i+1, x, 
							 ajIntGet(num[i],k));
								

				/* Get the id of the mismatch residue in the 
				   SEQRES sequence.  */
				ajBaseAa1ToAa3((*pdbfile)->seqres[i]->Ptr[k], 
					       &aa_misfit);

				/* To give correct index into SEQRES
				   records in original PDB file,
				   subtract j to account for
				   modifications to the N-terminus
				   that were made for missing residues
				   relative to ATOM sequence.  A
				   further 1 is added to give a number
				   starting from 1 (rather than 0)*/

				ajFmtPrintS(&msgbit,  "%S%S:%S%d.    ", 
					    (*pdbfile)->rtype[a], 
					    (*pdbfile)->pdbn[a], 
					    aa_misfit, k-j+1);


				ajStrApp(&msgstr, msgbit);
				    
				    
			    (*pdbfile)->seqres[i]->Ptr[k]=ajStrChar(seq[i], k);

			    }
			
			
			/* Sequences are same length (acceptable
                           number of mismatches) */
			if(nmismatches<=lim)
			{
			    if(nmismatches)
				ajFmtPrintF(logf, "%-15s%d (%c) %d %S\n", 
					    "MISMATCH",  i+1,  
					    ajChararrGet((*pdbfile)->chid, i),
					    nmismatches, msgstr);
			    

			    for(k=0;k<nres[i];k++)
				ajIntPut(&idx[i], k, k+1);
			
			    if(x==0)
				(*pdbfile)->resn1ok[i]=ajTrue;
			    else
				(*pdbfile)->resn1ok[i]=ajFalse;		       
			
			    done=ajTrue;
			    /*DIAGNOSTIC		
			    printf("STEP1 OK %d mismatches\n", nmismatches); */

			    break;
			}
			else
			{
			    /* Otherwise, sequences are same length 
			       (unacceptable number of mismatches) 
			       Restore the original seqres sequence */
			    ajStrAssS(&((*pdbfile)->seqres[i]), tmpseqres);
		         (*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);

			    /*DIAGNOSTIC
			      printf("STEP1 **NOT** OK %d mismatches\n", 
			      nmismatches);  */
			    
			}
		    }
		}


		
				    
		/***********************************************/
		/******************* STEP 2 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrint("STEP2 tmpseqres: %S\n", tmpseqres);
		  
		  ajFmtPrint("chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i],
		  (*pdbfile)->seqres[i]); 
		  printf("\n");
		  if(ajStrMatch(seq1[i], seq2[i]))
		  printf("seq1 and seq2 match\n");
		  else
		  printf("seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		  printf("seq1 and seqres match\n");
		  else
		  printf("seq1 and seqres DO NOT match\n");
		  printf("\n");
		  */


		/* Check whether ATOM is substring of SEQRES sequence */
		if((loc=ajStrFind((*pdbfile)->seqres[i], seq[i]))!=-1)
		{
		    /* ATOM is substring of SEQRES sequence - assign residue 
		       numbers 'by hand' */

		    for(k=0;k<nres[i];k++)
			ajIntPut(&idx[i], k, k+loc+1);
		    
		    if(x==0)
			(*pdbfile)->resn1ok[i]=ajTrue;
		    else
			(*pdbfile)->resn1ok[i]=ajFalse;		       
		    
		    done=ajTrue;

		    /*DIAGNOSTIC
		      printf("STEP2 OK\n"); */

		    break;
		}

		/*DIAGNOSTIC		 printf("STEP2 **NOT** OK\n"); */

		/* Otherwise, ATOM is NOT a substring of SEQRES sequence */




		/***********************************************/
		/******************* STEP 3 ********************/
		/***********************************************/
		/*DIAGNOSTIC	
		  ajFmtPrint("STEP3 tmpseqres: %S\n", tmpseqres);
		  
		  ajFmtPrint("chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i],
		  (*pdbfile)->seqres[i]); 
		  printf("\n");
		  if(ajStrMatch(seq1[i], seq2[i]))
		  printf("seq1 and seq2 match\n");
		  else
		  printf("seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		  printf("seq1 and seqres match\n");
		  else
		  printf("seq1 and seqres DO NOT match\n");
		  printf("\n");
		  */


		/* Check whether SEQRES is substring of ATOM sequence */
		if((loc=ajStrFind(seq[i],(*pdbfile)->seqres[i])) !=-1)
		{
		    /* SEQRES is substring of ATOM sequence - correct for
		       residues missing from SEQRES sequence and assign 
		       residue numbers 'by hand' */
		    
		    /* N-terminal insertion needed*/
		    if(loc!=0)
		    {
			ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "MISSNTERM",
				    i+1, 
				    ajChararrGet((*pdbfile)->chid, i), loc);
						

			ajStrAssSub(&bit, seq[i], 0, loc-1);
			ajStrInsert(&((*pdbfile)->seqres[i]), 0, bit); 
			(*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);
		    }
		    
		    /* C-terminal insertion needed */
		    if((*pdbfile)->nres[i]!=nres[i])
		    {
			ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "MISSCTERM", 
				    i+1, ajChararrGet((*pdbfile)->chid, i),
				    ( nres[i] - (*pdbfile)->nres[i]));
			

			ajStrAssSub(&bit, seq[i],
				    (*pdbfile)->nres[i], nres[i]-1);
			ajStrApp(&((*pdbfile)->seqres[i]), bit);
			(*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);
		    }

		    for(k=0;k<nres[i];k++)
			ajIntPut(&idx[i], k, k+1);
		    
		    if(x==0)
			(*pdbfile)->resn1ok[i]=ajTrue;
		    else
			(*pdbfile)->resn1ok[i]=ajFalse;		       

		    /*DIAGNOSTIC		     printf("STEP3 OK\n"); */

		    done=ajTrue;
		    break;
		}

		/* Otherwise, SEQRES is NOT a substring of the ATOM sequence */
		/*DIAGNOSTIC		 printf("STEP3 **NOT** OK\n");  */

		

		/***********************************************/
		/******************* STEP 4 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrint("STEP4.1 tmpseqres: %S\n", tmpseqres);
		  
		  ajFmtPrint("chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i],
		  (*pdbfile)->seqres[i]); 
		  printf("\n");
		  if(ajStrMatch(seq1[i], seq2[i]))
		  printf("seq1 and seq2 match\n");
		  else
		  printf("seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		  printf("seq1 and seqres match\n");
		  else
		  printf("seq1 and seqres DO NOT match\n");
		  printf("\n");
		  
		  ajFmtPrint("STEP4.2 tmpseqres: %S\n", tmpseqres);
		  */

		/* Check whether residue numbering is correct (and count 
		   the number of mismatches) */
		for(err=ajFalse, ajStrAssC(&msgstr, ""), nmismatches=0, 
		    k=0;k<nres[i];k++)
		{
		    this_num = ajIntGet(num[i], k);

		    /* Check to prevent array boundary error */
		    if(this_num <= (*pdbfile)->nres[i])
		    {
			if(ajStrChar(seq[i], k) != 
			   ajStrChar((*pdbfile)->seqres[i], this_num-1))
			{
			    nmismatches++;
			    /* Correct the seqres sequence.
			       JISON replace this with appropriate library call
			       once available so we don't have to probe the 
			       internals of the structure */

			    
			    /* a will give the number of the first
			       coordinate line for the mismatch
			       residue from the ATOM records*/
			    a = pdbparse_PdbfileFindLine(*pdbfile, i+1, x, 
						       ajIntGet(num[i], k));
			    
			    /* Get the id of the mismatch residue in the 
			       SEQRES sequence.  */
			    
		        ajBaseAa1ToAa3((*pdbfile)->seqres[i]->Ptr[this_num-1], 
					   &aa_misfit);

			    ajFmtPrintS(&msgbit,  "%S%S:%S%d.    ", 
					(*pdbfile)->rtype[a],
					(*pdbfile)->pdbn[a], 
					aa_misfit, this_num-j);
			    

			    ajStrApp(&msgstr, msgbit);
				    
			    
		  (*pdbfile)->seqres[i]->Ptr[this_num-1]=ajStrChar(seq[i], k); 

			}
		    }
		    else
		    {
			err=ajTrue;
			break;
		    }
		}
		

		/*DIAGNOSTIC	
		  ajFmtPrint("STEP4.3 tmpseqres: %S\n", tmpseqres); */

		if(!err)
		{
		    /* Residue numbering is correct (no or acceptable number 
		       of mismatches) */		
		    if(nmismatches<=lim)
		    {
			if(nmismatches)
			    ajFmtPrintF(logf,
					"%-15s%d (%c) %d %S\n", "MISMATCH",  
					i+1, 
					ajChararrGet((*pdbfile)->chid, i),
					nmismatches, msgstr);


			

			for(k=0;k<nres[i];k++)  
			    ajIntPut(&idx[i], k, ajIntGet(num[i], k));
			
			if(x==0)
			    (*pdbfile)->resn1ok[i]=ajTrue;
			else
			    (*pdbfile)->resn1ok[i]=ajFalse;		       

			done=ajTrue;
			
			/*DIAGNOSTIC	
			  printf("STEP4 OK %d mismatches\n", nmismatches); */
			
 			break;
		    }
		    else
		    {
			/* Otherwise, residue numbering is incorrect
			   (unacceptable number of mismatches) Restore
			   the original seqres sequence*/
			ajStrAssS(&((*pdbfile)->seqres[i]), tmpseqres);
			(*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);

			
			/*DIAGNOSTIC printf("STEP4 **NOT** OK %d
			  mismatches\n", nmismatches); */
		    }
		}
		else
		{
		    /* Otherwise, residue numbering is incorrect (residue 
		       number is out of range) 
		       Restore the original seqres sequence*/
		    ajStrAssS(&((*pdbfile)->seqres[i]), tmpseqres);
		    (*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);



		    /*DIAGNOSTIC		
		      printf("STEP4 **NOT** OK out_of_range\n");  */
		}





		/***********************************************/
		/******************* STEP 5 ********************/
		/***********************************************/
		/*DIAGNOSTIC
		  ajFmtPrint("STEP5 tmpseqres: %S\n", tmpseqres); 
		  
		  ajFmtPrint("chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n", i+1, seq1[i], seq2[i], 
		  (*pdbfile)->seqres[i]); 
		  printf("\n");
		  if(ajStrMatch(seq1[i], seq2[i]))
		  printf("seq1 and seq2 match\n");
		  else
		  printf("seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		  printf("seq1 and seqres match\n");
		  else
		  printf("seq1 and seqres DO NOT match\n");
		  printf("\n");
		  */



		/* None of the measures above could find the correct residue 
		   numbering so try by alignment.
		   
		   Align the SEQRES sequence to the ATOM sequence by taking 
		   progressively decreasing sized substrings from the ATOM 
		   sequence.  It returns 0 if any of the alignments would not 
		   leave enough space to be able to align the rest of the ATOM 
		   sequence to the SEQRES sequence - i.e. alignments giving an 
		   overspill of the ATOM sequence past the C-terminus of the 
		   SEQRES sequence are NOT allowed. 
		   
		   NO Mismatches are allowed at this stage 
		   */


		for(done_end=ajFalse,
		    len=(*pdbfile)->nres[i],
		    siz_substr=nres[i], 
		    atm_ptr=ajStrStr(seq[i]), 
		    seqres_ptr=ajStrStr((*pdbfile)->seqres[i]); 
		    siz_substr>0; )
		{
		    ajStrAssSubC(&substr, atm_ptr, 0, siz_substr-1);
		    
		    if((loc_ptr = (char *) strstr(seqres_ptr, 
						  ajStrStr(substr)))==NULL)
		    {
			siz_substr--;
			continue;
		    }
		    else
		    {
			atm_idx= (int) ((atm_ptr-ajStrStr(seq[i]))
					/sizeof(char));
			seqres_idx= (int) 
			    ((loc_ptr-ajStrStr((*pdbfile)->seqres[i]))
			     /sizeof(char));

			


			/*CHECK TO SEE IF THERE IS SPACE TO FIT THE
			  REMAINER OF THE ATOM SEQUENCE IN THE SEQRES
			  SEQUENCE GIVEN THIS ALIGNMENT */
			if((nres[i] - atm_idx) > (len - seqres_idx))
			    break;
			
			for(k=0, y=atm_idx, z=seqres_idx; k<siz_substr; k++, 
			    y++, z++)
			    ajIntPut(&idx[i], y, z+1);

			
			
			/*Mark up last SEQRES residue as having been done */
			if(y==nres[i])
			{
			    done_end = ajTrue;
			}
			
	  
		    }
      
		    atm_ptr+=siz_substr;
		    seqres_ptr=loc_ptr+siz_substr;
		    siz_substr=nres[i]-(atm_idx+siz_substr);
		}
		
		
		/*Check to ensure that position for last residue has been 
		  worked out */
		if(done_end)
		{
		    
		    /*DIAGNOSTIC		ajFmtPrint("chnn : %d\n"
		      "seq1 : %S\n"
		      "seq2 : %S\n"
		      "seqr : %S\n", i+1, seq1[i], seq2[i], 
		      (*pdbfile)->seqres[i]); 
		      printf("\n");
		      if(ajStrMatch(seq1[i], seq2[i]))
		      printf("seq1 and seq2 match\n");
		      else
		      printf("seq1 and seq2 DO NOT match\n");
		      
		      if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		      printf("seq1 and seqres match\n");
		      else
		      printf("seq1 and seqres DO NOT match\n");
		      printf("\n");
		      */


		    


		    		    

		    /* Residue numbering is correct after alignment 
		       (no mismatches) */
		    if(x==0)
			(*pdbfile)->resn1ok[i]=ajTrue;
		    else
			(*pdbfile)->resn1ok[i]=ajFalse;		       

		    ajFmtPrintF(logf, "%-15s%d (%c)\n", "GAPPEDOK", i+1,  
				ajChararrGet((*pdbfile)->chid, i));
		    		    
		    

		    /*DIAGNOSTIC		     printf("STEP5 OK\n"); */
		    
		    done=ajTrue;
		    break;
		}


		/*DIAGNOSTIC		 printf("STEP5 **NOT** OK\n"); */

		/* Otherwise, agreement could not be found */

		/* JISON Note ... idx array might contain junk values
		   now but this should not matter as the array should
		   be overwritten */



		
		/***********************************************/
		/******************* STEP 6 ********************/
		/***********************************************/
		/*DIAGNOSTIC		ajFmtPrint("STEP6 tmpseqres: %S\n", 
		  tmpseqres);
		  
		  ajFmtPrint("chnn : %d\n"
		  "seq1 : %S\n"
		  "seq2 : %S\n"
		  "seqr : %S\n",
		  i+1, seq1[i], seq2[i], (*pdbfile)->seqres[i]); 
		  printf("\n");
		  if(ajStrMatch(seq1[i], seq2[i]))
		  printf("seq1 and seq2 match\n");
		  else
		  printf("seq1 and seq2 DO NOT match\n");
		  
		  if(ajStrMatch(seq1[i], (*pdbfile)->seqres[i]))
		  printf("seq1 and seqres match\n");
		  else
		  printf("seq1 and seqres DO NOT match\n");
		  printf("\n");
		  */


		/* Try again by alignment.
		   Mismatches ARE allowed at this stage 
		   Must change:
		   */
		

		for(ajStrAssC(&msgstr, ""), 
		    nmismatches=0,
		    done_end=ajFalse,  
		    len=(*pdbfile)->nres[i],
		    siz_substr=nres[i], 
		    atm_ptr=ajStrStr(seq[i]), 
		    seqres_idx_last=-1,
		    seqres_ptr=ajStrStr((*pdbfile)->seqres[i]); 
		    siz_substr>0; )
		{	
		    /* On the first pass, atm_ptr and seqres_ptr point to 
		       the start of the ATOM and SEQRES sequences 
		       respectively. */


		    founderr=ajFalse;

		    /* Copy block (of progressively decreasing size) from ATOM 
		       sequence to substring1  */
		    ajStrAssSubC(&substr, atm_ptr, 0, siz_substr-1);
      

		    /*DIAGNOSTIC		
		      ajFmtPrintF(tempfile, "\n***\n%12s\n%s\n%12s\n%s\n", 
		      "Aligning ", ajStrStr(substr), "To ", seqres_ptr);  */
		    
		    
		    /* Set loc_ptr to point to the first occurence of
		       substring1 in SEQRES sequence */
		    /* If substring1 does not occur in SEQRES sequence */
		    if( (loc_ptr = (char *) strstr(seqres_ptr, 
						   ajStrStr(substr)))==NULL)
		    {
			/*See if there is an error in the residue id
			  at the start of <substr> Create a new
			  substring from the ATOM records but omit the
			  first character.  There is a specical case
			  if substr is only 1 character long.*/
			
			if(siz_substr == 1)
			{
			    loc_ptr = seqres_ptr;
			    nmismatches++;
			    founderr=ajTrue;
			}
			else
			{
			    /* Copy substring1 to substring2 but omit
			       the first character */
			    ajStrAssSub(&substr2, substr, 1, -1);
			


			    /*DIAGNOSTIC
			      ajFmtPrintF(tempfile,
			      "\n!!!\n%12s\n%s\n%12s\n%s\n", 
			      "Aligning ", ajStrStr(substr2),
			      "To ", seqres_ptr);  */

			    /* Set loc_ptr to point to the first occurence of 
			       substring1 in SEQRES sequence */
			    /* If substring2 does not occur in the SEQRES 
			       sequence, continue with a smaller substring */
			    if((loc_ptr = (char *) strstr(seqres_ptr, 
					      ajStrStr(substr2)))==NULL)
			    {
				siz_substr--;
				continue;
			    }
			    /* substring2 is found in the SEQRES sequence */
			    else
			    {
				/* If there is not enough space to
				   accomodate the 'missing' residue
				   continue (with a smaller substring) */
				if(loc_ptr == seqres_ptr)
				{	
				    siz_substr--;
				    continue;
				}


				/*DIAGNOSTIC
				  ajFmtPrintF(tempfile, "\n\n\nnmismatches = "
				  "%d\n%12s%s\n%12s%s\n%12s%s\n%12s%s\n", 
				  nmismatches, 
				  "atm_ptr:",
				  atm_ptr, 
				  "seqres_ptr:",
				  seqres_ptr, 
				  "substr:",
				  substr, 
				  "substr2", 
				  substr2);  */

			    
				/*DIAGNOSTIC
				  ajFmtPrintF(tempfile,
				  "MISMATCH FOUND OK\n");  */

				/* There is enough space to accommodate 
				   substring2 and the 'missing' 
				   (mismatch) residue */
				nmismatches++;
				founderr=ajTrue;
			    }
			}
		    }
		    /*DIAGNOSTIC
		      else
		      ajFmtPrintF(tempfile, "ALIGNMENT FOUND OK\n");  */
		    
		    
		    /* atm_idx and seqres_idx are set to give the
		       index into ATOM and SEQRES sequences
		       respectively for the position of match of
		       N-terminal residue of substring (if founderr is
		       True this will be the position of the
		       N-terminal mismatch residue) */
		    atm_idx= (int) ((atm_ptr-ajStrStr(seq[i]))/sizeof(char));

		    if(founderr)
			seqres_idx= (int) (((loc_ptr-1)-
					    ajStrStr((*pdbfile)->seqres[i]))
					   /sizeof(char));
		    else
			seqres_idx= (int) ((loc_ptr-
					    ajStrStr((*pdbfile)->seqres[i]))
					   /sizeof(char));
		    
		    /*DIAGNOSTIC printf("seqres_idx : %d\n", seqres_idx); */
		    


		    /* If there was a mismatch residue, idx_misfit_atm and 
		       idx_misfit_seqres will give the index into the ATOM 
		       and SEQRES sequences respectively for its position */
		    if(founderr)
		    {
			idx_misfit_atm    = atm_idx;
			idx_misfit_seqres = seqres_idx;
		    }
		    

      
		    /*CHECK TO SEE IF THERE IS SPACE TO FIT THE REMAINDER OF 
		      THE ATOM SEQUENCE IN THE SEQRES SEQUENCE GIVEN THIS 
		      ALIGNMENT */
		    if((nres[i] - atm_idx) > (len - seqres_idx))
			break;
		    
		    /**************************************************/
		    /* JISON this will have to change for 1st residue */
		    /**************************************************/
		    
		    /* Try and find an exact match within the gap for the 
		       mismatch residue */
		    fixed = ajFalse;
		    if(founderr)
		    {
			/*DIAGNOSTIC
			  printf("About to try (seqres_idx_last: %d,  
			  seqres_idx: %d) ...\n",
			  seqres_idx_last, seqres_idx); */
			
			
			aa_last = ajStrChar(substr, 0);
			for(z=seqres_idx_last+1; z<seqres_idx; z++)
			{
			    /*DIAGNOSTIC
			      printf("Trying ATOM:SEQRES  %c:%c\n", 
			      aa_last,(*pdbfile)->seqres[i]->Ptr[z]);*/
			    
			    if((*pdbfile)->seqres[i]->Ptr[z] == aa_last)
			    {
				nmismatches--;
				founderr=ajFalse;
				fixed = ajTrue;
				

				/* Assign residue number */
				ajIntPut(&idx[i], atm_idx, z+1);
				for(k=0,  y=atm_idx+1,  z=seqres_idx+1; 
				    k<siz_substr-1; k++,  y++,  z++)
				    ajIntPut(&idx[i], y, z+1);

				break;
			    }
			}
		    }
		    
		    if(!fixed)
		    {
			/*DIAGNOSTIC
			  printf("FAILED TO FIX\n"); */
			/* Assign residue number */
			for(k=0, y=atm_idx,  z=seqres_idx; k<siz_substr; 
			    k++,  y++,  z++)
			    ajIntPut(&idx[i], y, z+1);

		    }
		    


		    /*Mark up last SEQRES residue as having been done */
		    if(y==nres[i])
			done_end = ajTrue;





		    /* If the substring matched but with a residue
		       mismatch for the 1st residue */
		    /**************************************************/
		    /* JISON This block should only be called if we   */
		    /* can't fit the mismatch residue in somewhere.   */
		    /**************************************************/
		    if(founderr)
		    {
			/* a will give the number of the first coordinate line 
			   for the mismatch residue from the ATOM records*/
			
			a = pdbparse_PdbfileFindLine(*pdbfile, i+1, x, 
						   ajIntGet(num[i], 
							    idx_misfit_atm));

			/* Get the id of the mismatch residue 
			   in the SEQRES sequence.  */
		 ajBaseAa1ToAa3((*pdbfile)->seqres[i]->Ptr[idx_misfit_seqres], 
				       &aa_misfit);
			
			
			/* To give correct index into SEQRES records in 
			   original PDB file, subtract j to account for 
			   modifications to the N-terminus that were made 
			   for missing residues relative to ATOM sequence.
			   A further 1 is added to give a number starting 
			   from 1 (rather than 0)*/
			
			/*DIAGNOSTIC
			  ajFmtPrint("a : %d\n(*pdbfile)->rtype[a] : "
			  "%S\n(*pdbfile)->pdbn[a] : %S\n", 
			  a, (*pdbfile)->rtype[a], (*pdbfile)->pdbn[a]);
			  */
			
			ajFmtPrintS(&msgbit,  "%S %S %S %d;    ", 
				    (*pdbfile)->rtype[a], (*pdbfile)->pdbn[a], 
				    aa_misfit, idx_misfit_seqres-j+1);
			

			/*DIAGNOSTIC
			  ajFmtPrintS(&msgbit, "ATOM residue %d (%c) vs "
			  "SEQRES residue %d (%c).   ", 
			  ajIntGet(num[i], atm_idx), ajStrChar(substr, 0), 
			  seqres_idx+1,
			  (*pdbfile)->seqres[i]->Ptr[seqres_idx]); */
			ajStrApp(&msgstr, msgbit);

			
		  (*pdbfile)->seqres[i]->Ptr[seqres_idx]=ajStrChar(substr, 0);
		    }
      
		    
		    /* atm_ptr and seqres_ptr now point to 1 residue past
		       the end of the match of the substring in the ATOM 
		       and SEQRES sequences respectively. */

		    atm_ptr+=siz_substr;
      
		    if(founderr)
			seqres_ptr=(loc_ptr-1)+siz_substr;
		    else
			seqres_ptr=loc_ptr+siz_substr;
      
		    siz_substr=nres[i]-(atm_idx+siz_substr);



		    /**************************************************/
		    /* JISON Must assign index into SEQRES for        */
		    /* C-terminal residue of substring                */
		    /**************************************************/


		    /* seqres_idx_last is set to give the index into SEQRES 
		       sequence for the position of match of C-terminal 
		       residue of substring + 1 */
		    seqres_idx_last = (int) z-1;
		    
		    

		}
		
		
		
		
		
		/*Check to ensure that position for last residue has
                  been worked out */
		if((done_end) && (nmismatches<=lim))
		{

		    
		    if(nmismatches)
			ajFmtPrintF(logf,
				    "%-15s%d (%c) %d %S\n", "GAPPED", i+1,  
				    ajChararrGet((*pdbfile)->chid, i),
				    nmismatches, msgstr);
		    else
			ajFmtPrintF(logf, "%-15s%d (%c)\n", "GAPPEDOK", i+1,  
				    ajChararrGet((*pdbfile)->chid, i));
		    		    
		    
		    /* Residue numbering is correct after alignment
		       (acceptable number of mismatches) */
		    if(x==0)
			(*pdbfile)->resn1ok[i]=ajTrue;
		    else
			(*pdbfile)->resn1ok[i]=ajFalse;		       
		    
		    /*DIAGNOSTIC		 
		      printf("STEP6 OK %d mismatches\n", nmismatches);  */

		    done=ajTrue;
		    break;
		}
		
		
		/*DIAGNOSTIC	
		  printf("STEP6 **NOT** OK %d mismatches\n", nmismatches);  */


		/* Otherwise, agreement could not be found -
		   unacceptable number of mismatches.  Restore the
		   original seqres sequence*/
		ajStrAssS(&((*pdbfile)->seqres[i]), tmpseqres);
		(*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);

	    } /* for(x=0;x<2;x++) */
	    if(done)
		break;
	}
	
	
	
		
	/* After trying 6 different alignment steps with (potentially
	   modified) seqres sequences and 2 (possibly different)
	   sequences derived from the ATOM records an alignment with
	   agreement in residue numbering still cannot be found.  Use
	   data from ATOM records only - use seq1 (all residues) and
	   presume there are no missing residues.*/
	if(!done)
	{
	    ajFmtPrintF(logf, "%-15s%d (%c)\n", "NOMATCH", i+1, 
			ajChararrGet((*pdbfile)->chid, i));


	    ajStrAssS(&((*pdbfile)->seqres[i]), seq1[i]);
	    (*pdbfile)->nres[i]=ajStrLen((*pdbfile)->seqres[i]);
	    for(k=0;k<nres1[i];k++)
		ajIntPut(&idx[i], k, k+1);


	    (*pdbfile)->resn1ok[i]=ajTrue;

	    
	}
	else
	{
	    if(j)
		ajFmtPrintF(logf, "%-15s%d (%c) %d\n", "MISSNTERM", i+1, 
			    ajChararrGet((*pdbfile)->chid, i), j);
	    

	    if(!ajStrMatch(seq1[i], seq2[i]))
	    {  
		if(x==0)
		    ajFmtPrintF(logf, "%-15s%d\n", "ALTERNOK", i+1);   
		else	
		    ajFmtPrintF(logf, "%-15s%d\n", "HETEROK", i+1);   
		
	    }
	}
    }
    

    
    
    /* Write the index arrays */
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	if(!(*pdbfile)->chainok[i])
	    continue;


	if((*pdbfile)->resn1ok[i])
	{
	    for(j=0;j<nres1[i];j++)
		ajIntPut(&idx_full[i], ajIntGet(num1[i], j), 
			 ajIntGet(idx[i], j));
	}
	else
	{
	    for(j=0;j<nres2[i];j++)
		ajIntPut(&idx_full[i], ajIntGet(num2[i], j), 
			 ajIntGet(idx[i], j));
	}
    }
    
    

    
    /* Write the resni element of the Pdbfile object.
       These are the residue numbers that give the correct index into 
       the finalised seqres sequence */
    
    
    
    for(i=(*pdbfile)->idxfirst;i<(*pdbfile)->nlines;i++)
    {
	if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
	{	
	    if((*pdbfile)->resn1ok[(*pdbfile)->chnn[i]-1])
	    {
		(*pdbfile)->resni[i]=ajIntGet(idx_full[(*pdbfile)->chnn[i]-1], 
					      (*pdbfile)->resn1[i]);	
		/*DIAGNOSTIC
		  printf("Got position %d (%d) ok\n", (*pdbfile)->resn1[i], 
		  ajIntGet(idx_full[(*pdbfile)->chnn[i]-1], 
		  (*pdbfile)->resn1[i])); */
	    }

	    else
	    {
		(*pdbfile)->resni[i]=ajIntGet(idx_full[(*pdbfile)->chnn[i]-1], 
					      (*pdbfile)->resn2[i]);
		/*DIAGNOSTIC printf("Got position %d (%d) ok\n", 
		  (*pdbfile)->resn2[i],
		  ajIntGet(idx_full[(*pdbfile)->chnn[i]-1], 
		  (*pdbfile)->resn2[i])); */
	    }
	}
    }
    

    
    
    /* DIAGNOSTIC 
       
       ajFmtPrintF(tempfile, "\n\n\n"); 
       
       seq=seq1;
       
       for(i=0;i<(*pdbfile)->nchains; i++)
       {
       if(!(*pdbfile)->chainok[i])
       {
       ajFmtPrintF(tempfile, "Chain %d\nSEQRES %S\nCHAIN NOT OK\n\n\n", 
       i+1,
       (*pdbfile)->seqres[i]);
       
       continue;
       }
       
       ajFmtPrintF(tempfile,
       "Chain %d\nSEQRES %S\nSEQ__1 %S\nSEQ__2 %S\n\n\n", 
       i+1,
       (*pdbfile)->seqres[i],
       seq1[i],
       seq2[i]);
       
       }
       
       
       
       
       
       
       for(i=0;i<(*pdbfile)->nchains; i++)
       {
       if(!(*pdbfile)->chainok[i])
       continue;	
       
       if(nres1[i] > nres2[i])
       max = nres1[i];
       else
       max = nres2[i];
       
       ajFmtPrintF(tempfile, "CHAIN %d\n", i+1);
       ajFmtPrintF(tempfile, "seqres %S\n", (*pdbfile)->seqres[i]);
       
       
       ajFmtPrintF(tempfile, "%-6s%-6s%-6s%-6s%-6s%-6s%-6s\n", 
       "RES", "NUM", "SEQ1", "RESN1", "SEQ2", "RESN2", "IDX");
       
       for(j=0;j<max;j++)
       {
       if(j<nres1[i] && j<nres2[i])
       {
       
       ajFmtPrintF(tempfile, "%-6s%-6d%-6c%-6d%-6c%-6d%-6d\n", 
       "RES", 
       j+1,
       ajStrChar(seq1[i], j),
       ajIntGet(num1[i], j),     
       ajStrChar(seq2[i], j),
       ajIntGet(num2[i], j),	
       ajIntGet(idx[i], j));
       }
       
       else if (j<nres1[i])
       {
       ajFmtPrintF(tempfile, "%-6s%-6d%-6c%-6d%-6c%-6c", 
       "RES", 
       j+1,
       ajStrChar(seq1[i], j), 
       ajIntGet(num1[i], j),     
       '.', '.');
       
       
       if((*pdbfile)->resn1ok[i])
       {
       ajFmtPrintF(tempfile, "%-6d\n", ajIntGet(idx[i], j));
       }
       
       else
       {
       ajFmtPrintF(tempfile, "%-6c\n", '.');
       }
       
       
       }
       
       else 
       {
       ajFmtPrintF(tempfile, "%-6s%-6d%-6c%-6c%-6c%-6d", 
       "RES", 
       j+1,
       '.', '.',
       ajStrChar(seq2[i], j), 
       ajIntGet(num2[i], j));
       
       if(!(*pdbfile)->resn1ok[i])
       {
       ajFmtPrintF(tempfile, "%-6d\n", ajIntGet(idx[i], j));
       }
       
       else
       {
       ajFmtPrintF(tempfile, "%-6c\n", '.');
       }
       
       
       }
       
       }
       
       }
       
       */
    
    
    
    
    /* Tidy up and return */
    ajStrDel(&aa_misfit);
    ajStrDel(&msgstr);
    ajStrDel(&msgbit);
    AJFREE(insert);
    ajStrDel(&tmpseqres);
    ajStrDel(&bit);
    ajStrDel(&substr);    
    ajStrDel(&substr2);    
    
    for(i=0;i<(*pdbfile)->nchains; i++)
    {
	if(!(*pdbfile)->chainok[i])
	    continue;


	ajStrDel(&seq1[i]);
	ajStrDel(&seq2[i]);


	ajIntDel(&num1[i]);
	ajIntDel(&num2[i]);
	
	ajIntDel(&idx[i]);
	ajIntDel(&idx_full[i]);

    }    
    
    AJFREE(seq1);
    AJFREE(seq2);
    
    
    AJFREE(num1);
    AJFREE(num2);
    
    AJFREE(idx);
    AJFREE(idx_full);
    
    AJFREE(nres1);
    AJFREE(nres2);
    
    
    return ajTrue;
}








/* #funcstatic pdbparse_diagnostic ******************************************
**
** For printing out diagnostics for pdbparse build
** 
**
** #param [r] pdbfile [AjPPdbfile*]  Pdbfile object pointer
** #param [r] n       [ajint]       Flag for controlling output
** 
** #return [void]
** @@
******************************************************************************/
/*THIS_DIAGNOSTIC */
/* static void pdbparse_diagnostic(AjPPdbfile *pdbfile, ajint n)
{
    ajint i;
    i=0;
    
    

         
       
       if(n==0)
       {
       ajFmtPrintF(tempfile, "nchains: %d\n", (*pdbfile)->nchains);
       for(i=0;i<(*pdbfile)->nchains; i++)
       ajFmtPrintF(tempfile, "chid: %c\n", ajChararrGet((*pdbfile)->chid, i));
       for(i=0;i<(*pdbfile)->nchains; i++)
       {
       ajFmtPrintF(tempfile, "seqres %d: %S\n", i, (*pdbfile)->seqres[i]);
       }
       
       
       ajFmtPrintF(tempfile, "tercnt: %d\n", (*pdbfile)->tercnt);
       ajFmtPrintF(tempfile, "COMPND: %S\n", (*pdbfile)->compnd);
       ajFmtPrintF(tempfile, "SOURCE: %S\n", (*pdbfile)->source);
       ajFmtPrintF(tempfile, "reso: %f\n", (*pdbfile)->reso);
       if((*pdbfile)->method == ajXRAY)
       ajFmtPrintF(tempfile, "method: ajXRAY\n");
       else
       ajFmtPrintF(tempfile, "method: ajNMR\n");
       }
       else if(n==1)
       {
       for(i=0;i<(*pdbfile)->nlines;i++)
       {
       ajFmtPrintF(tempfile, "%-5S",  (*pdbfile)->pdbid);
       
       if((*pdbfile)->linetype[i]==PDBPARSE_IGNORE)
       ajFmtPrintF(tempfile, "%-10s",  "IGNORE");
       else if((*pdbfile)->linetype[i]==PDBPARSE_COORD)
       ajFmtPrintF(tempfile, "%-10s",  "COORD");
       else if((*pdbfile)->linetype[i]==PDBPARSE_COORDHET)
       ajFmtPrintF(tempfile, "%-10s",  "COORDHET");
       else if((*pdbfile)->linetype[i]==PDBPARSE_COORDGP)
       ajFmtPrintF(tempfile, "%-10s",  "COORDGP");
       else if((*pdbfile)->linetype[i]==PDBPARSE_COORDWAT)
       ajFmtPrintF(tempfile, "%-10s",  "COORDWAT");
       else if((*pdbfile)->linetype[i]==PDBPARSE_TER)
       ajFmtPrintF(tempfile, "%-10s",  "TER");
       else if((*pdbfile)->linetype[i]==PDBPARSE_MODEL)
       ajFmtPrintF(tempfile, "%-10s",  "MODEL");
       
       
       
       ajFmtPrintF(tempfile, "M%-2dC%-2d%-6S%-5d%-5d%-4B%-4S%-4S%-7.3f%-7.3f"
       "%-7.3f%-6.3f%-6.3f\n", 
       (*pdbfile)->modn[i],
       (*pdbfile)->chnn[i],
       (*pdbfile)->pdbn[i], 
       (*pdbfile)->resn1[i],
       (*pdbfile)->resn2[i],
       (*pdbfile)->oddnum[i], 
       (*pdbfile)->atype[i], 
       (*pdbfile)->rtype[i], 
       (*pdbfile)->x[i], 
       (*pdbfile)->y[i], 
       (*pdbfile)->z[i], 
       (*pdbfile)->o[i], 
       (*pdbfile)->b[i]);
       
       }	
       }
       

}*/








/* @funcstatic pdbparse_PdbfileToPdb ******************************************
**
** Reads data from a Pdbfile object and writes a Pdb object.
** 
**
** @param [w] ret     [AjPPdb *]     Pdb object pointer
** @param [r] pdb     [AjPPdbfile]   Pdbfile object pointer
** 
** @return [AjBool]  True on success, False otherwise
** @@
******************************************************************************/
static AjBool pdbparse_PdbfileToPdb(AjPPdb *ret, AjPPdbfile pdb)
{
    ajint i=0;				/* Loop counter */
    ajint j=0;				/* Loop counter */
    AjPAtom atm=NULL;			/* Atom object */
    


    
    if( !ret || !pdb)
    {
	ajWarn("Bad arg's passed to pdbparse_PdbfileToPdb");
	return ajFalse;
    }

    if( !(*ret))
    {
	ajWarn("Bad arg's passed to pdbparse_PdbfileToPdb");
	return ajFalse;
    }
    
    ajStrAssS(&((*ret)->Pdb), pdb->pdbid);
    ajStrAssS(&((*ret)->Compnd), pdb->compnd);
    ajStrAssS(&((*ret)->Source), pdb->source);    
    (*ret)->Method = pdb->method;
    (*ret)->Reso   = pdb->reso;
    (*ret)->Nmod   = pdb->modcnt;
    (*ret)->Nchn   = pdb->nchains;
    (*ret)->Ngp    = pdb->ngroups;


    for(i=0; i<pdb->ngroups; i++)
	ajChararrPut(&((*ret)->gpid), i, ajChararrGet(pdb->gpid, i));
    

    for(i=0;i<pdb->nchains;i++)
    {
	(*ret)->Chains[i]->Id   = ajChararrGet((pdb)->chid, i);


	if(!(pdb)->chainok[i])
	    continue;
		    
	(*ret)->Chains[i]->numHelices = (pdb)->numHelices[i];
	(*ret)->Chains[i]->numStrands = (pdb)->numStrands[i];
	(*ret)->Chains[i]->numSheets  = (pdb)->numSheets[i];
	(*ret)->Chains[i]->numTurns   = (pdb)->numTurns[i];
	

	(*ret)->Chains[i]->Nres = (pdb)->nres[i];
	(*ret)->Chains[i]->Nlig = (pdb)->nligands[i];
	ajStrAssS(&((*ret)->Chains[i]->Seq), (pdb)->seqres[i]);
    }
    
    
    for(j=(pdb)->idxfirst; j<(pdb)->nlines; j++)
    {
	if(((pdb)->linetype[j]==PDBPARSE_COORD) ||
	   ((pdb)->linetype[j]==PDBPARSE_COORDHET) ||
	   ((pdb)->linetype[j]==PDBPARSE_COORDGP) ||
	   ((pdb)->linetype[j]==PDBPARSE_COORDWAT))
	{
	    /* Skip this line if it for a heterogenous (duplicate) position  */
	    if( (!(pdb)->resn1ok[(pdb)->chnn[j]-1]) && (pdb)->oddnum[j])
		continue;
		
	    atm = ajXyzAtomNew();

	    atm->Mod = (pdb)->modn[j];
	    atm->Chn = (pdb)->chnn[j];
	    atm->Gpn = (pdb)->gpn[j];

	    if((pdb)->linetype[j]==PDBPARSE_COORDHET)
		atm->Type = 'H';
	    else if((pdb)->linetype[j]==PDBPARSE_COORDGP)
		atm->Type = 'H';
	    else if((pdb)->linetype[j]==PDBPARSE_COORD)
		atm->Type = 'P';
	    else if((pdb)->linetype[j]==PDBPARSE_COORDWAT)
		atm->Type = 'W';
	    atm->Idx = (pdb)->resni[j];
	    
	    ajStrAssS(&atm->Pdb, (pdb)->pdbn[j]);
	    
		
	    if(((pdb)->linetype[j]==PDBPARSE_COORDHET) ||
	       ((pdb)->linetype[j]==PDBPARSE_COORDGP) ||
	       ((pdb)->linetype[j]==PDBPARSE_COORDWAT))
		atm->Id1='.';
	    else
		pdbparse_BaseAa3ToAa1(&atm->Id1, (pdb)->rtype[j]);
		
	    ajStrAssS(&atm->Id3, (pdb)->rtype[j]);
	    ajStrAssS(&atm->Atm, (pdb)->atype[j]);
	    atm->X=(pdb)->x[j];
	    atm->Y=(pdb)->y[j];
	    atm->Z=(pdb)->z[j];
	    atm->O=(pdb)->o[j];
	    atm->B=(pdb)->b[j];


	    ajStrAssS(&atm->eId, (pdb)->elementId[j]);
	    atm->eNum   = (pdb)->elementNum[j];
	    atm->eType  = (pdb)->elementType[j];
	    atm->eClass = (pdb)->helixClass[j];
	    

	    if((pdb)->linetype[j]==PDBPARSE_COORDGP)
		ajListPushApp((*ret)->Groups, atm);
	    else if((pdb)->linetype[j]==PDBPARSE_COORDWAT)
		ajListPushApp((*ret)->Water, atm);
	    else
		ajListPushApp((*ret)->Chains[(pdb)->chnn[j]-1]->Atoms, atm);
	}
	else continue;
    }
    
    
    return ajTrue;
}	




/* @funcstatic pdbparse_PdbfileFindLine ***************************************
**
** Returns the line number of the first instance of a line with a specified 
** residue and chain number. 	
** 
** @param [r] pdb     [AjPPdbfile] Pdbfile object pointer
** @param [r] chn     [ajint] Chain number
** @param [r] which   [ajint] 0 or 1, refer to resn1 or resn2 residue
** @param [r] pos     [ajint] Residue number
**             	                   numbering (see Pdbfile object)
** 
** @return [ajint]  Line number (index, i.e. starts from 0).
** @@
******************************************************************************/
static ajint  pdbparse_PdbfileFindLine(AjPPdbfile pdb, ajint chn,
				       ajint which, ajint pos)
{
    ajint a=0;
    /* a will give the number of the first coordinate line 
       for the mismatch residue from the ATOM records*/
			



    for(a=pdb->idxfirst;a<pdb->nlines;a++)
	if(pdb->linetype[a]==PDBPARSE_COORD &&
	   pdb->chnn[a] == chn)
	    /* First sequence (all residues) derived for atom records */
	    /* OR Second sequence (excluding certain residues) derived
	       for atom records */
	    if(((which==0) && (pos == pdb->resn1[a])) ||
	       ((which==1) && (pos == pdb->resn2[a])))
		break;
			
			
    if(a==pdb->nlines)
	ajFatal("Unexpected loop failure in pdbparse_PdbfileFindLine. "
		"Email jison@hgmp.mrc.ac.uk\n");

    return a;
}






/* @funcstatic pdbparse_PdbfileChain ******************************************
**
** Finds the chain number for a given chain identifier in a pdbfile structure
**
** @param [r] id  [char]       Chain identifier
** @param [r] pdb [AjPPdbfile] Pdbfile object
** @param [w] chn [ajint *]    Chain number
**
** @return [AjBool] True on success
** @@
******************************************************************************/
static AjBool pdbparse_PdbfileChain(char id, AjPPdbfile pdb, ajint *chn)
{
    ajint a;
 


   
    for(a=0;a<pdb->nchains;a++)
    {
	if(toupper(ajChararrGet(pdb->chid, a)) == toupper(id))
	{
	    *chn=a+1;
	    return ajTrue;
	}
	/* Cope with chain id's of ' ' (which might be given as '.' in 
	   the Pdbfile object) */
	if((id==' ')&&(ajChararrGet(pdb->chid, a)=='.'))
	{
	    *chn=a+1;
	    return ajTrue;
	}
    }

    /* A '.' may be given as the id for domains comprising more than one
       chain*/
    if(id=='.')
    {
	*chn=1;
	return ajTrue;
    }
    
	
    return ajFalse;
}






/* @funcstatic pdbparse_WriteElementData **************************************
**
** Reads the secondary structure information from an Elements object 
** and writes equivalent variables in an Pdbfile object.
** 
** @param [W] pdbfile [AjPPdbfile *] Pdbfile object pointer
** @param [r] logf    [AjPFile] Pointer to log file (build diagnostics)
** @param [r] elms    [AjPElements] Elements object pointer
** 
** @return [AjBool]  True on success, False otherwise
** @@
******************************************************************************/
static AjBool pdbparse_WriteElementData(AjPPdbfile *pdbfile, AjPFile logf, 
			       AjPElements elms)
{
    ajint x           =0;
    ajint y           =0;
    ajint z           =0;
    ajint modn        =0;	/* Model number */
    
    ajint idx         =0;	/* idx into lines in Pdbfile object */
    ajint idx_start   =0;	/* Line index of start of element */
    ajint idx_end     =0;	/* Line index of end of element */
    ajint idx_last    =0;	/* Line index of last line to try */
    ajint idx_tmp     =0;	/* Temp. line index */
    
    ajint chn         =0;	/* Chain id of current element as integer */
    
    AjPInt nsheets    =NULL;	/* Number of sheets in each chain */
    AjPStr *lastids   =NULL;	/* Last sheet identifier read in for each 
				   chain */
    AjBool found_start=ajFalse;	/* Whether start residue of the
				   current element has been found yet */
    AjBool found_end  =ajFalse;	/* Whether the end residue of the current 
				   element has been found yet */
    ajint  n_unknown  =0;	/* No. of unknown chain ids */
    AjPChar unknowns  =NULL;	/* Unknown chain ids */
    AjBool found      =ajFalse;	/* True if we have already reported an error 
				   message for the unknown chain id */


    
    
    
    /* Check args */
    if(!(*pdbfile) || !(elms) || !(logf))
    {
	ajWarn("NULL arg passed to pdbparse_WriteElementData");
	return ajFalse;
    }
    
    
    /* Assign default values for secondary structure fields in Pdbfile
       object */
    for(x=0; x<(*pdbfile)->nlines; x++)
    {
	(*pdbfile)->elementType[x]='.';
	ajStrAssC(&(*pdbfile)->elementId[x], ".");
    }
    
    
    
    
    /* Allocate memory */
    nsheets = ajIntNewL((*pdbfile)->nchains); 
    ajIntPut(&nsheets, (*pdbfile)->nchains, 0);
    
    
    unknowns = ajChararrNew();
    
    
    AJCNEW0(lastids, (*pdbfile)->nchains);
    for(x=0;x<(*pdbfile)->nchains;x++)
    {
	lastids[x]=ajStrNew();
	/* Assign a silly value for starters */
	ajStrAssC(&lastids[x], "?????");
    }
    
    
    
/*    printf("LOOK HERE x:%d elms->n:%d\n", x, elms->n); */
    
    for(modn=1; modn<=(*pdbfile)->modcnt; modn++)	{
	
	/* Loop for each element. 
	   Set the current line to the first line in the file*/
	for(idx=(*pdbfile)->idxfirst, 
	    x=0;x<elms->n; x++)
	{
	    /* Find the chain number of the current element */
	    if(!pdbparse_PdbfileChain(elms->elms[x]->chainId, *pdbfile, &chn))
	    {
		/* Only report errors once for each unknown id */
		if(modn==1)
		{
		    for(found=ajFalse, y=0; y<n_unknown; y++)
			if(ajChararrGet(unknowns, y)==elms->elms[x]->chainId)
			{
			    found=ajTrue;
			    break;
			}
		
		    if(!found)
		    {
			ajFmtPrintF(logf, "%-15s%c %d\n", "SECCHAIN", 
				    elms->elms[x]->chainId, idx);
			ajChararrPut(&unknowns, n_unknown, 
				     elms->elms[x]->chainId);
			n_unknown++;
		    }
		
		}
	
		continue;
	    }
	    
	    
	    /* Only want to do this once (for the first model ) */
	    if(modn==1)
	    {
	    

		/* Make a count of the number of beta sheets */
		if(elms->elms[x]->elementType == 'E')
		    if(!ajStrMatch(lastids[chn-1], elms->elms[x]->elementId))
		    {
			ajIntInc(&nsheets, chn-1); 
			ajStrAssS(&lastids[chn-1], elms->elms[x]->elementId);
		    }

	    }
	    
	    
	    /* Loop for two passes.
	       z is for efficiency, if z==0 it will check from the current 
	       position up to the last coordinate line, if z==1 it will check
	       from the first coordinate line
	       up to the last position checked*/
	    for(found_start=ajFalse, found_end=ajFalse,
		z=0; z<2; z++)
	    {
		if(z==0)
		    idx_last = (*pdbfile)->nlines;
		else
		{
		    idx=(*pdbfile)->idxfirst;
		    idx_last = idx_tmp;
		}
	    

		/* Find the start and end of the current element (as an 
		   index into the line array) */
		for(;idx<idx_last; idx++)
		{
		    /* Find the correct chain and skip lines that are not for 
		       amino acids*/
		    if(((*pdbfile)->chnn[idx] != chn) || 
		       ((*pdbfile)->linetype[idx]!=PDBPARSE_COORD) || 
		       ((*pdbfile)->modn[idx] != modn))
			continue;

		    /* We have not found the start residue yet */
		    if(!found_start)
			if(ajStrMatch(elms->elms[x]->initSeqNum, 
				      (*pdbfile)->pdbn[idx]))
			    if(ajStrMatch(elms->elms[x]->initResName, 
					  (*pdbfile)->rtype[idx]))
			    {
				/* Residue number for start found and residue
				   type matches */
				idx_start = idx;
				found_start=ajTrue;
			    }
		
		    if(ajStrMatch(elms->elms[x]->endSeqNum, 
				  (*pdbfile)->pdbn[idx]))
			if(ajStrMatch(elms->elms[x]->endResName, 
				      (*pdbfile)->rtype[idx]))
			{
			    /* Residue number for end found and residue 
			       type matches */
			    idx_end = idx;

			
			    /* Set the index to the LAST atom of the residue */

			    for(; idx_end < (*pdbfile)->nlines; idx_end++)
				if(!ajStrMatch(elms->elms[x]->endSeqNum, 
					       (*pdbfile)->pdbn[idx_end]) ||
				   !ajStrMatch(elms->elms[x]->endResName,
					       (*pdbfile)->rtype[idx_end])||
				   (*pdbfile)->chnn[idx_end] != chn ||
				   (*pdbfile)->modn[idx_end] != modn)
				    break;
			    idx_end--;
			

			    found_end=ajTrue;
			    idx_tmp = idx;
			    break;
			}
		
		}
	    
		if(found_start && found_end)
		    break; 
	    }
	    
	    
	    if(!found_start || !found_end)
	    {
		if(!found_start && !found_end)
		{
		    ajFmtPrintF(logf, "%-15s%d %d %S %S %S %S\n", "SECBOTH", 
				chn, modn, elms->elms[x]->initResName, 
				elms->elms[x]->initSeqNum, 
				elms->elms[x]->endResName, 
				elms->elms[x]->endSeqNum);
		
		}
		else if(!found_start)
		{
		    ajFmtPrintF(logf, "%-15s%d %d %S %S\n", "SECSTART", 
				chn, modn, elms->elms[x]->initResName, 
				elms->elms[x]->initSeqNum);
		}
		else if(!found_end)
		{
		    ajFmtPrintF(logf, "%-15s%d %d %S %S\n", "SECEND", chn, 
				modn, elms->elms[x]->endResName, 
				elms->elms[x]->endSeqNum);
		}
	    }
	    
	    
	    
	    
	
	
	    /* Assign secondary structure fields in Pdbfile object */
	    for(idx=idx_start; idx<=idx_end; idx++)
	    {
		(*pdbfile)->elementNum[idx] = elms->elms[x]->elementNum;
		(*pdbfile)->elementType[idx] = elms->elms[x]->elementType;
		if(elms->elms[x]->elementType == 'H')
		    (*pdbfile)->helixClass[idx] = elms->elms[x]->helixClass;
		ajStrAssS(&(*pdbfile)->elementId[idx],
			  elms->elms[x]->elementId);
	    }


	    /* Only want to do this once */
	    if(modn==1)
	    {
		if(elms->elms[x]->elementType == 'H')
		    (*pdbfile)->numHelices[chn-1]++;	
		else if(elms->elms[x]->elementType == 'E')
		    (*pdbfile)->numStrands[chn-1]++;	
		else if(elms->elms[x]->elementType == 'T')
		    (*pdbfile)->numTurns[chn-1]++;	


	    }
	
	
	
	}
    
    

    }
	

    

    
    /* Assign number of sheets */
    if(modn==1)
	for(x=0; x<(*pdbfile)->nchains; x++)
	    (*pdbfile)->numSheets[x] = ajIntGet(nsheets, x);
    
    
    /* Tidy up and return */
    ajIntDel(&nsheets);
    for(x=0;x<(*pdbfile)->nchains;x++)
	ajStrDel(&lastids[x]);
    AJFREE(lastids);
    
    ajChararrDel(&unknowns);


    return ajTrue;
}

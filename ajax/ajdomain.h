/****************************************************************************
**
** @source ajdomain.h
**
** AJAX objects for handling protein domain data.  
** Scop and Cath objects. 
** 
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
****************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajdomain_h
#define ajdomain_h





/* @data AjPScop ************************************************************
**
** Ajax scop object.
**
** Holds scop database data
**
** AjPScop is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSScop
** @alias AjOScop
**
**
**
** @attr Entry              [AjPStr]  Domain identifer code.
** @attr Pdb                [AjPStr]  Corresponding pdb identifier code.
** @attr Class              [AjPStr]  SCOP class name as an AjPStr.
** @attr Fold               [AjPStr]  SCOP fold  name as an AjPStr. 
** @attr Superfamily        [AjPStr]  SCOP superfamily name as an AjPStr.
** @attr Family             [AjPStr]  SCOP family name as an AjPStr.
** @attr Domain             [AjPStr]  SCOP domain name as an AjPStr.
** @attr Source             [AjPStr]  SCOP source (species) as an AjPStr.
** @attr N                  [ajint]   No. chains from which this domain is 
**                                    comprised.
** @attr Chain              [char*]   Chain identifiers.
** @attr Start              [AjPStr*] PDB residue number of first residue in 
**                                    domain.
** @attr End                [AjPStr*] PDB residue number of last residue in 
**                                    domain.
** @attr Sunid_Class        [ajint]   SCOP sunid for class.
** @attr Sunid_Fold         [ajint]   SCOP sunid for fold.
** @attr Sunid_Superfamily  [ajint]   SCOP sunid for superfamily.
** @attr Sunid_Family       [ajint]   SCOP sunid for family.
** @attr Sunid_Domain       [ajint]   SCOP sunid for domain.  
** @attr Sunid_Source       [ajint]   SCOP sunid for species.
** @attr Sunid_Domdat       [ajint]   SCOP sunid for domain data.
**
** @attr Acc                [AjPStr]  Accession number of sequence entry.
** @attr Spr                [AjPStr]  Swissprot code of sequence entry.
** @attr SeqPdb	            [AjPStr]  Sequence (from pdb) as string.
** @attr SeqSpr	            [AjPStr]  Sequence (from swissprot) as string.
** @attr Startd             [ajint]   Start of sequence relative to full 
**                                    length swissprot sequence.
** @attr Endd               [ajint]   End of sequence relative to full length 
**                                    swissprot sequence.
**
**
**
** @new    ajScopNew Scop default constructor.
** @input  ajScopReadNew Scop constructor from reading dcf format file.
** @input  ajScopReadCNew Scop constructor from reading dcf format file.
** @delete ajScopDel Default Scop destructor.
** @assign ajScopCopy Replicates a Scop object.
** @use    ajScopMatchSunid Sort Scop objects by Sunid_Family element.
** @use    ajScopMatchScopid Sort Scop objects by Entry element.
** @use    ajScopMatchPdbId Sort Scop objects by Pdb element.
** @use    embScopToPdbid  Read a scop identifier code and writes the 
**         equivalent PDB identifier code.
** @use    embScopToSp  Read a scop identifier code and writes the 
**         equivalent swissprot identifier code.
** @use    embScopToAcc  Read a scop identifier code and writes the 
**         equivalent accession number.
** @use    ajScopArrFindScopid Binary search for Entry element over array 
**         of Scop objects. 
** @use    ajScopArrFindSunid Binary search for Sunid_Family element over 
**         array of Scop objects. 
** @use    ajScopArrFindPdbid Binary search for Pdb element over array of
**         Scop objects. 
** @input  ajScopReadAllNew Construct list of Scop objects from reading dcf
**         format file.
** @input  ajScopReadAllRawNew Construct list of Scop objects from reading 
**         raw SCOP parsable files.
** @output ajScopWrite Write Scop object to dcf format file.
** @output ajPdbWriteDomain Writes a ccf format file for a SCOP domain.
** @output ajPdbWriteDomainRaw Writes a pdb-format file for a SCOP domain.
** @output ajPdbWriteDomainRecordRaw Writes lines to a pdb format file for 
**         a domain.
**
** @@
****************************************************************************/
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

    ajint  Sunid_Class;       
    ajint  Sunid_Fold;        
    ajint  Sunid_Superfamily; 
    ajint  Sunid_Family;      
    ajint  Sunid_Domain;      
    ajint  Sunid_Source;      
    ajint  Sunid_Domdat;      

    AjPStr Acc;        
    AjPStr Spr;        
    AjPStr SeqPdb;	
    AjPStr SeqSpr;	
    ajint  Startd;      
    ajint  Endd;        
} AjOScop;
#define AjPScop AjOScop*





/* @data AjPCath ************************************************************
**
** Ajax cath object
**
** Holds cath database data
**
** AjPScop is implemented as a pointer to a C data structure.
**
**
**
** @alias AjSCath
** @alias AjOCath
**
**
**
** @attr DomainID       [AjPStr]  Domain identifer code        
** @attr Pdb            [AjPStr]  Corresponding pdb identifer code
** @attr Class          [AjPStr]  CATH class name as an AjPStr
** @attr Architecture   [AjPStr]  CATH architecture name as an AjPStr
** @attr Topology       [AjPStr]  CATH topology name as an AjPStr
** @attr Superfamily    [AjPStr]  CATH homologous superfamily name as an AjPStr
** @attr Length         [ajint]   No. of residues in domain
** @attr Chain          [char]    Chain identifier
** @attr NSegment       [ajint]   No. of chain segments domain is comprised of
** @attr Start          [AjPStr*] PDB residue number of 1st residue in segment 
** @attr End            [AjPStr*] PDB residue number of last residue in segment
** @attr Class_Id       [ajint]   CATH class no. as an ajint
** @attr Arch_Id        [ajint]   CATH architecture no.as an ajint
** @attr Topology_Id    [ajint]   CATH topology no. as an ajint
** @attr Superfamily_Id [ajint]   CATH superfamily no. as an ajint
** @attr Family_Id      [ajint]   CATH family no. as an ajint 
** @attr NIFamily_Id    [ajint]   CATH near identical family no. as an ajint 
** @attr IFamily_Id     [ajint]   CATH identical family no. as an ajint 
**
**
** 
** @new    ajCathNew Default Cath constructor
** @input  ajCathReadCNew Cath constructor from reading dcf format file.
** @input  ajCathReadNew Cath constructor from reading dcf format file.
** @delete ajCathDel Default Cath destructor
** @use    ajCathArrFindPdbid Binary search for Pdb element over array of
**         Cath objects. 
** @use    ajCathMatchPdbId Sort Cath objects by Pdb element.
** @input  ajCathReadAllNew Construct list of Cath objects from reading dcf
**         format file.
** @input  ajCathReadAllRawNew Construct list of Cath objects from reading 
**         raw CATH parsable files.
** @other  ajCathWrite Write Cath object to dcf format file.
** 
** @@
****************************************************************************/

typedef struct AjSCath
{
    AjPStr  DomainID;       
    AjPStr  Pdb;            
    AjPStr  Class;          
    AjPStr  Architecture;   
    AjPStr  Topology;       
    AjPStr  Superfamily;    
    
    ajint   Length;         
    char    Chain;          
    
    ajint   NSegment;       
    AjPStr *Start;      
    AjPStr *End;          
    
    ajint   Class_Id;        
    ajint   Arch_Id;         
    ajint   Topology_Id;     
    ajint   Superfamily_Id;  
    ajint   Family_Id;      
    ajint   NIFamily_Id;     
    ajint   IFamily_Id;     
} AjOCath;
#define AjPCath AjOCath*



void ajDomainDummyFunction(void);

/* ======================================================================= */
/* =========================== Scop object =============================== */
/* ======================================================================= */
AjPScop  ajScopNew(ajint n);
void     ajScopDel(AjPScop *ptr);
AjBool   ajScopCopy(AjPScop *to, const AjPScop from);

ajint    ajScopArrFindScopid(AjPScop const *arr, ajint siz, const AjPStr id);
ajint    ajScopArrFindSunid(AjPScop const *arr, ajint siz, ajint id);
ajint    ajScopArrFindPdbid(AjPScop const *arr, ajint siz, const AjPStr id);

ajint    ajScopMatchScopid(const void *hit1, const void *hit2);
ajint    ajScopMatchPdbId(const void *hit1, const void *hit2);
ajint    ajScopMatchSunid(const void *entry1, const void *entry2);

AjPScop  ajScopReadCNew(AjPFile inf, const char *entry);
AjPScop  ajScopReadNew(AjPFile inf, const AjPStr entry);
AjPList  ajScopReadAllNew(AjPFile inf); 
AjPList  ajScopReadAllRawNew(AjPFile claf, AjPFile desf, AjBool outputall);
AjBool   ajScopWrite(AjPFile outf, const AjPScop obj);

AjBool   ajPdbWriteDomain(AjPFile outf, const AjPPdb pdb,
	        	   const AjPScop scop, AjPFile errf);






/* ======================================================================= */
/* =========================== Cath object =============================== */
/* ======================================================================= */
AjPCath   ajCathNew(ajint n);
void      ajCathDel(AjPCath *ptr);

ajint     ajCathArrFindPdbid(const AjPCath *arr, ajint siz, const AjPStr id);
ajint     ajCathMatchPdbid(const void *hit1, const void *hit2);

AjPCath   ajCathReadCNew(AjPFile inf, const char *entry);
AjPCath   ajCathReadNew(AjPFile inf, const AjPStr entry);
AjPList   ajCathReadAllNew(AjPFile inf); 
AjPList   ajCathReadAllRawNew(AjPFile cathf, AjPFile domf, 
			      AjPFile namesf, AjPFile logf);
AjBool    ajCathWrite(AjPFile outf, const AjPCath obj);


#endif

#ifdef __cplusplus
}
#endif


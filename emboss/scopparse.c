/* @source scopparse application
**
** Converts raw scop classification files to a file in embl-like format.
**
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
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
** 
** 
** Operation
** 
** scopparse parses the dir.cla.scop.txt and dir.des.scop.txt scop classification 
** files (e.g. available at URL (1).)
** (1) http://scop.mrc-lmb.cam.ac.uk/scop/parse/dir.cla.scop.txt_1.57
**     http://scop.mrc-lmb.cam.ac.uk/scop/parse/dir.des.scop.txt_1.57
** The format of these files is explained at URL (2).
** (2) http://scop.mrc-lmb.cam.ac.uk/scop/release-notes-1.55.html
** 
** scopparse writes the scop classification to an embl-like format file (Figure 1). 
** No changes are made to the data other than changing the format in which it 
** is held. This EMBL-like format SCOP file is used by several other EMBOSS 
** programs. The reason why the SCOP database format is changed to an EMBL-like 
** format before being used used by other EMBOSS programs is that it is an
** easier format to work with than the native SCOP database format. The 
** records used to describe an entry are given below.  Records (4) to (8) 
** are used to describe the position of the domain in the scop hierarchy.
**
** (1)  ID - Domain identifier code.  This is a 7-character code that uniquely
** identifies the domain in scop.  It is identical to the first 7 characters 
** of a line in the scop classification file.  The first character is always 
** 'D', the next four characters are the PDB identifier code, the fifth 
** character is the PDB chain identifier to which the domain belongs (a '.' is
** given in cases where the domain is composed of multiple chains, a '_' is 
** given where a chain identifier was not specified in the PDB file) and the 
** final character is the number of the domain in the chain (for chains 
** comprising more than one domain) or '_' (the chain comprises a single 
** domain only).
** (2)  EN - PDB identifier code.  This is the 4-character PDB identifier code
** of the PDB entry containing the domain.
** (3)  SI - SCOP Sunid's.  The integers preceeding the codes CL, FO, SF, FA, DO, 
** SO and DD are the SCOP sunids for Class, Fold, SuPerfamily, Family, Domain, 
** Source and domain data respectively. These numbers uniquely identify the 
** appropriate node in the SCOP parsable files.
** (4)  OS - Source of the protein.  It is identical to the text given after 
** 'Species' in the scop classification file.
** (5)  CL - Domain class.  It is identical to the text given after 'Class' in 
** the scop classification file.
** (6)  FO - Domain fold.  It is identical to the text given after 'Fold' in 
** the scop classification file.
** (7)  SF - Domain superfamily.  It is identical to the text given after 
** 'Superfamily' in the scop classification file.
** (8)  FA - Domain family. It is identical to the text given after 'Family' in 
** the scop classification file.
** (9)  DO - Domain name. It is identical to the text given after 'Protein' in 
** the scop classification file.
** (10)  NC - Number of chains comprising the domain (usually 1).  If the number
** of chains is greater than 1, then the domain entry will have a section  
** containing a CN and a CH record (see below) for each chain.
** (11) CN - Chain number.  The number given in brackets after this record 
** indicates the start of the data for the relevent chain.
** (12) CH - Domain definition.  The character given before CHAIN is the PDB 
** chain identifier (a '.' is given in cases where a chain identifier was not 
** specified in the scop classification file), the strings before START and 
** END give the start and end positions respectively of the domain in the PDB 
** file (a '.' is given in cases where a position was not specified).  Note 
** that the start and end positions refer to residue numbering given in the 
** original pdb file and therefore must be treated as strings.
** (13) XX - used for spacing.
** (14) // - used to delimit records for a domain.
**
** Figure 1  Excerpt from embl-like format scop classification file
**
**  
**  ID   D3SDHA_
**  XX
**  EN   3SDH
**  XX
**  OS   Ark clam (Scapharca inaequivalvis)
**  XX
**  CL   All alpha proteins
**  XX
**  FO   Globin-like
**  XX
**  SF   Globin-like
**  XX
**  FA   Globins
**  XX
**  DO   Hemoglobin I
**  XX
**  NC   1
**  XX
**  CN   [1]
**  XX
**  CH   a CHAIN; . START; . END;
**  //
**  ID   D3SDHB_
**  XX
**  EN   3SDH
**  XX
**  OS   Ark clam (Scapharca inaequivalvis)
**  XX
**  CL   All alpha proteins
**  XX
**  FO   Globin-like
**  XX
**  SF   Globin-like
**  XX
**  FA   Globins
**  XX
**  DO   Hemoglobin I
**  XX
**  NC   1
**  XX
**  CN   [1]
**  XX
**  CH   b CHAIN; . START; . END;
**  //
**  ID   D3HBIA_
**  XX
**  EN   3HBI
**  XX
**  OS   Ark clam (Scapharca inaequivalvis)
**  XX
**  CL   All alpha proteins
**  XX
**  FO   Globin-like
**  XX
**  SF   Globin-like
**  XX
**  FA   Globins
**  XX
**  DO   Hemoglobin I
**  XX
**  NC   1
**  XX
**  CN   [1]
**  XX
**  CH   a CHAIN; . START; . END;
**  //
**  ID   D3HBIB_
**  
******************************************************************************/






#include "emboss.h"



static ajint scopparse_CompSunid(const void *scop1, const void *scop2);
static ajint scopparse_search(ajint id, AjPScopdes *arr, ajint siz);



/* @prog scopparse **********************************************************
**
** Convert raw scop classification file to embl-like format
**
*****************************************************************************/

int main(int argc, char **argv)
{
    AjPFile inf1=NULL;
    AjPFile inf2=NULL;
    AjPFile outf=NULL;

    AjPScopcla cla=NULL;   
    AjPScopdes des=NULL;  
    AjPScopdes *desarr=NULL;
    
    AjPList  clalist=NULL;
    AjPList  deslist=NULL;

    ajint  dim=0;  /* Dimension of array */
    ajint  idx=0;  /* Index into array */
    ajint  i=0;
    

    clalist = ajListNew();
    deslist = ajListNew();

    /* Read data from acd*/
    embInit("scopparse", argc, argv);
    
    inf1  =  ajAcdGetInfile("infilea");
    inf2  =  ajAcdGetInfile("infileb");
    outf  =  ajAcdGetOutfile("outfile");
    


    /* Read the dir.cla.scop.txt file */ 
    while(ajXyzScopclaReadC(inf1, "*", &cla))
    {
	ajListPushApp(clalist, cla);
/*	ajFmtPrint(" %d ", cla->Domdat); */
    }
    
    ajFileClose(&inf1);
    
    
    /* Read the dir.des.scop.txt file, sort the list by Sunid
       and convert to an array */
    while(ajXyzScopdesReadC(inf2, "*", &des))
    {
	ajListPush(deslist, des);
/*	ajFmtPrint("%d\n", des->Sunid); */
    }
    
    ajFileClose(&inf2);


    ajListSort(deslist, scopparse_CompSunid);
    dim=ajListToArray(deslist, (void ***) &desarr);
    

    while(ajListPop(clalist, (void **)&cla))
    {
	ajFmtPrintF(outf,"ID   %S\nXX\n",cla->Entry);
	ajFmtPrintF(outf,"EN   %S\nXX\n",cla->Pdb);
	ajFmtPrintF(outf,"SI   %d CL; %d FO; %d SF; %d FA; %d DO; %d SO; %d DD;\nXX\n",	
		    cla->Class,cla->Fold, cla->Superfamily, 
		    cla->Family,cla->Domain, cla->Source,
		    cla->Domdat);
	/*ajFmtPrintF(outf,"SI   %d\nXX\n",cla->Domdat);*/
	idx = scopparse_search(cla->Class,  desarr, dim);
	ajFmtPrintF(outf,"CL   %S\n",desarr[idx]->Desc);

	idx = scopparse_search(cla->Fold,  desarr, dim);
	ajFmtPrintF(outf,"XX\n");
	ajFmtPrintSplit(outf,desarr[idx]->Desc,"FO   ",75," \t\n\r");


	idx = scopparse_search(cla->Superfamily,  desarr, dim);
	ajFmtPrintF(outf,"XX\n");
	ajFmtPrintSplit(outf,desarr[idx]->Desc,"SF   ",75," \t\n\r");

	idx = scopparse_search(cla->Family,  desarr, dim);
	ajFmtPrintF(outf,"XX\n");
	ajFmtPrintSplit(outf,desarr[idx]->Desc,"FA   ",75," \t\n\r");

	idx = scopparse_search(cla->Domain,  desarr, dim);
	ajFmtPrintF(outf,"XX\n");
	ajFmtPrintSplit(outf,desarr[idx]->Desc,"DO   ",75," \t\n\r");

	idx = scopparse_search(cla->Source,  desarr, dim);
	ajFmtPrintF(outf,"XX\n");
	ajFmtPrintSplit(outf,desarr[idx]->Desc,"OS   ",75," \t\n\r");

	ajFmtPrintF(outf,"XX\nNC   %d\n",cla->N);

	for(i=0;i<cla->N;++i)
	{
	    ajFmtPrintF(outf,"XX\nCN   [%d]\n",i+1);
	    ajFmtPrintF(outf,"XX\nCH   %c CHAIN; %S START; %S END;\n",
			cla->Chain[i],
			cla->Start[i],
			cla->End[i]);
	}
	ajFmtPrintF(outf,"//\n");

	ajXyzScopclaDel(&cla);
    
    }

    while(ajListPop(deslist, (void **)&des))
	ajXyzScopdesDel(&des);
    
    /* Tidy up */
    ajFileClose(&outf);
    AJFREE(desarr);
    ajListDel(&clalist);
    ajListDel(&deslist);
    

    ajExit();
    return 0;
}





/* @funcstatic scopparse_CompSunid ********************************************
**
** Function to sort AjOScopdes objects by Sunid element.
**
** @param [r] scop1  [const void*] Pointer to AjOScopdes object 1
** @param [r] scop2  [const void*] Pointer to AjOScopdes object 2
**
** @return [ajint] -1 if Sunid1 should sort before Sunid2, +1 if the Sunid2 
** should sort first. 0 if they are identical in value.
** @@
******************************************************************************/
static ajint scopparse_CompSunid(const void *scop1, const void *scop2)
{
    AjPScopdes p  = NULL;
    AjPScopdes q  = NULL;

    p = (*  (AjPScopdes*)scop1);
    q = (*  (AjPScopdes*)scop2);
    
    if(p->Sunid < q->Sunid)
	return -1;
    else if(p->Sunid == q->Sunid)
	return 0;
    else 
	return 1;
}






/* @funcstatic scopparse_search ***********************************************
**
** Performs a binary search for a Sunid over an array of Scopdes objects 
** structures (which of course must first have been sorted). 
**
** @param [r] id  [ajint]       Search value of Sunid
** @param [r] arr [AjPScopdes*] Array of AjOScopdes objects
** @param [r] siz [ajint]       Size of array
**
** @return [ajint] Index of first AjOScopdes object found with an
** Sunid element matching id, or -1 if id is not found.
** @@
******************************************************************************/
static ajint scopparse_search(ajint id, AjPScopdes *arr, ajint siz)
{
    int l;
    int m;
    int h;
    

    l=0;
    h=siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;

	if(id < arr[m]->Sunid)
	    h=m-1;
	else if(id > arr[m]->Sunid)
	    l=m+1;
	else
	    return m;
    }
    return -1;
}

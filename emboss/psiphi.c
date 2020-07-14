/* @source psiphi application
**
** Calculates the psi and phi torsion angles around the alpha carbons (CA)
**  in (a specified stretch of) a specified chain of a protein structure
**  from co-ordinates of the mainchain atoms in the two planes around it
**
** Outputs 360 degrees if an angle cannot be calculated
**
** @author: Copyright (C) Damian Counsell
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
******************************************************************************/

#include "emboss.h"
#include <math.h>



static ajint chain_index(ajint ajIntSelectedChainNumber,
			 ajint ajIntHighestChainNumber,
			 ajint ajIntLowestChainNumber);

static ajint first_residue_number(const AjPPdb ajpPdbCleanStructure,
				  ajint ajIntChainIndex,
				  ajint ajIntStartResidueNumber);

static ajint last_residue_number(const AjPPdb PdbCleanStructure,
				 ajint ajIntChainIndex,
				 ajint ajIntStartResidueNumber,
				 ajint ajIntFinishResidueNumber);

static AjBool phi_calculable(const AjBool* arrayOfAjboolsResidue);

static AjBool psi_calculable(const AjBool* arrayOfAjboolsResidue);

static float phi(AjPAtom const *arrayOfAjpatomsWindow);

static float psi(AjPAtom const *arrayOfAjpatomsWindow);

static AjBool load_previous_residue(AjPAtom ajpAtomCurrent,
				    AjPAtom* arrayOfAjpatomsResidue,
				    AjBool* arrayOfAjboolsResidue);

static AjBool load_next_residue(AjPAtom ajpAtomCurrent,
				AjPAtom* arrayOfAjpatomsResidue,
				AjBool* arrayOfAjboolsResidue);

static AjBool load_current_residue(AjPAtom ajpAtomCurrent,
				   AjPAtom* arrayOfAjpatomsResidue,
				   AjBool* arrayOfAjboolsResidue);

static AjPFeature write_psi_phi(AjPFeattable ajpFeattableTorsionAngles,
				ajint ajIntFeatureResidueNumber,
				float fPhiTorsionAngle,
				float fPsiTorsionAngle);

static void shift_residues(AjPAtom* arrayOfAjpatomsResidue,
			   AjBool* arrayOfAjboolsResidue);


/* constant window size and enumerated indexes to atoms in window */
const ajint ajIntWindowSize = 9;
enum enumAtomWindowPoint
{
    ENitrogenPrevious,
    EAlphaCarbonPrevious,
    EPrimeCarbonPrevious,
    ENitrogenCurrent,
    EAlphaCarbonCurrent,
    EPrimeCarbonCurrent,
    ENitrogenNext,
    EAlphaCarbonNext,
    EPrimeCarbonNext
};

/* for unavailable angles (360 deg is an impossible torsion angle) */
const float FUnavailableAngle = 360.0;

/* @prog psiphi **************************************************************
** 
** protein structure C-alpha psi and phi angles given neighbour co-ordinates
**
******************************************************************************/

int main( int argc , char **argv )
{
    float fPhiTorsionAngle = 0.0;
    float fPsiTorsionAngle = 0.0;

    /*
     * coordinates from atoms in at least
     * THREE residues are required
     * to calculate psi and phi angles
     */
    ajint ajIntFirstResidueInWindow;
    ajint ajIntSecondResidueInWindow;
    ajint ajIntThirdResidueInWindow;

    /* declare position counters and limits */
    /* ...for residues */
    ajint ajIntCarbonCurrent          = 0;
    ajint ajIntResidueNumber          = 0;
    ajint ajIntPreviousResidueNumber  = 0;
    ajint ajIntFinalResidueNumber     = 0;

    /* ...for chains */
    ajint ajIntChainIndex             = 0; /* ...into structure object */
    ajint ajIntLowestChainNumber      = 0; /* ...in structure file     */
    ajint ajIntHighestChainNumber     = 0; /* ...in structure file     */

    /* variables for (user-specified) chain and residue numbers */
    ajint ajIntSelectedChainNumber;
    ajint ajIntStartResidueNumber;
    ajint ajIntFinishResidueNumber;
    
    /* window of AjPAtoms for co-ords */
    AjPAtom* arrayOfAjpatomsWindow = NULL;
    /* window of AjBools for presence or absence  */
    AjBool* arrayOfAjboolsWindow   = NULL;
    AjBool ajBoolWindowFull;

    AjPStr ajpStrReportHeader = NULL;
    /* DDDDEBUG: string for report footer */
    /*     AjPStr ajpStrReportTail      = NULL; */

    /* cleaned-up structure */
    AjPFile ajpFileCleanProteinStructure = NULL; /* file           */
    AjPPdb  ajpPdbCleanStructure         = NULL; /* object         */
    AjPSeq  ajpSeqCleanChain             = NULL; /* current chain  */
    AjIList ajIteratorAtomList           = NULL; /* list of atoms  */
    AjPAtom ajpAtomCurrentInList         = NULL; /* current atom   */

    /* output report file for torsion angles */
    AjPReport ajpReportTorsionAngles       = NULL;
    AjPFeattable ajpFeattableTorsionAngles = NULL;
    AjPFeature ajpFeatCurrent              = NULL;

    embInit( "psiphi", argc ,argv );

    /* get protein structure from ACD */
    ajpFileCleanProteinStructure = ajAcdGetInfile("sequence" );
    /* get angle output file from ACD */
    ajpReportTorsionAngles = ajAcdGetReport("outfile");
    /* get chain to be scanned from ACD */
    ajIntSelectedChainNumber = ajAcdGetInt("chainnumber");
    /* get first residue to be scanned from ACD */
    ajIntStartResidueNumber = ajAcdGetInt("startresiduenumber");
    /* get last residue to be scanned from ACD */
    ajIntFinishResidueNumber = ajAcdGetInt("finishresiduenumber");

    /* reserve memory for and read in structure */
    /* JISON */    ajpPdbCleanStructure = ajPdbReadNew(ajpFileCleanProteinStructure);
    
    /* check and set number of chain to be analysed */
    ajIntHighestChainNumber = ajpPdbCleanStructure->Nchn;
    ajIntChainIndex = chain_index(ajIntSelectedChainNumber,
				  ajIntHighestChainNumber,
				  ajIntLowestChainNumber);

    /* check and set range of residues to be analysed */
    ajIntFirstResidueInWindow = 
	first_residue_number(ajpPdbCleanStructure,
				    ajIntChainIndex,
				    ajIntStartResidueNumber);    
    ajIntFinalResidueNumber = last_residue_number(ajpPdbCleanStructure,
						  ajIntChainIndex,
						  ajIntStartResidueNumber,
						  ajIntFinishResidueNumber);
    ajIntSecondResidueInWindow = ajIntFirstResidueInWindow+1;
    ajIntThirdResidueInWindow = ajIntFirstResidueInWindow+2;

    /*
     * start loop over atoms in chain at
     * ajIntFirstResidueInWindow and
     * finish at ajIntFinalResidueNumber
     */

    /* obtain iterator for list of atoms in chain */
    ajIteratorAtomList = 
	ajListIterRead(ajpPdbCleanStructure->Chains[ajIntChainIndex]->Atoms);

    /* obtain sequence from residues in chain */
    ajpSeqCleanChain =
	ajSeqNewStr(ajpPdbCleanStructure->Chains[ajIntChainIndex]->Seq);

    ajIntResidueNumber = 0;

    /* create feature table for torsion angle output */
    ajpFeattableTorsionAngles = ajFeattableNewSeq(ajpSeqCleanChain);    

    /* chain info for head of report */
    ajFmtPrintS(&ajpStrReportHeader, "Chain: %d", (ajIntChainIndex+1));
    ajReportSetHeader(ajpReportTorsionAngles, ajpStrReportHeader);

    /* BEGIN ANALYSIS OF CHAIN HERE */
    /* loop through list until first residue in window reached */
    do
    {
	/* do nothing until you reach the start residue */
	ajpAtomCurrentInList = ajListIterNext(ajIteratorAtomList);
	ajIntResidueNumber = ajpAtomCurrentInList->Idx;
    }
    while(ajIntResidueNumber < ajIntFirstResidueInWindow);

    ajIntPreviousResidueNumber = ajIntResidueNumber;

    /* create and initialize AjPAtom window array */
    arrayOfAjpatomsWindow =
	(AjPAtom *) AJALLOC(ajIntWindowSize*sizeof(AjPAtom));
    arrayOfAjpatomsWindow[ENitrogenPrevious]    = NULL;
    arrayOfAjpatomsWindow[EAlphaCarbonPrevious] = NULL;
    arrayOfAjpatomsWindow[EPrimeCarbonPrevious] = NULL;
    arrayOfAjpatomsWindow[ENitrogenCurrent]     = NULL;
    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]  = NULL;
    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]  = NULL;
    arrayOfAjpatomsWindow[ENitrogenNext]        = NULL;
    arrayOfAjpatomsWindow[EAlphaCarbonNext]     = NULL;
    arrayOfAjpatomsWindow[EPrimeCarbonNext]     = NULL;
    /* create and initialize AjBool window array */
    arrayOfAjboolsWindow =
	(AjBool *) AJALLOC(ajIntWindowSize*sizeof(AjBool));
    arrayOfAjboolsWindow[ENitrogenPrevious]    = AJFALSE;
    arrayOfAjboolsWindow[EAlphaCarbonPrevious] = AJFALSE;
    arrayOfAjboolsWindow[EPrimeCarbonPrevious] = AJFALSE;
    arrayOfAjboolsWindow[ENitrogenCurrent]     = AJFALSE;
    arrayOfAjboolsWindow[EAlphaCarbonCurrent]  = AJFALSE;
    arrayOfAjboolsWindow[EPrimeCarbonCurrent]  = AJFALSE;
    arrayOfAjboolsWindow[ENitrogenNext]        = AJFALSE;
    arrayOfAjboolsWindow[EAlphaCarbonNext]     = AJFALSE;
    arrayOfAjboolsWindow[EPrimeCarbonNext]     = AJFALSE;
    ajBoolWindowFull = AJFALSE;
    
    /* loop through list until window is full */
    do
    {
	ajIntResidueNumber = ajpAtomCurrentInList->Idx;

	/* load window with atom co-ordinates */
	/* get previous N, CA and C' */
	if(ajIntResidueNumber == ajIntFirstResidueInWindow)
	    load_previous_residue(ajpAtomCurrentInList,
				  arrayOfAjpatomsWindow,
				  arrayOfAjboolsWindow);
	/* get current N, CA and C' */
	else if(ajIntResidueNumber == ajIntSecondResidueInWindow)
	    load_current_residue(ajpAtomCurrentInList,
				 arrayOfAjpatomsWindow,
				 arrayOfAjboolsWindow);
	/* get new next N, CA and C'  */
	else if(ajIntResidueNumber == ajIntThirdResidueInWindow)
	{
	    /* ajIntCarbonCurrent residue no. for which angles calc'd */
	    ajIntCarbonCurrent = ajIntResidueNumber-1;
	    load_next_residue(ajpAtomCurrentInList,
			      arrayOfAjpatomsWindow,
			      arrayOfAjboolsWindow);
	}
	else
	    break;
    }while((ajpAtomCurrentInList = ajListIterNext(ajIteratorAtomList)));

    /* analyse first residue */
    if(phi_calculable(arrayOfAjboolsWindow))
	fPhiTorsionAngle = phi(arrayOfAjpatomsWindow);
    else
	fPhiTorsionAngle = FUnavailableAngle;
    if(psi_calculable(arrayOfAjboolsWindow))
	fPsiTorsionAngle = psi(arrayOfAjpatomsWindow);
    else
	fPsiTorsionAngle = FUnavailableAngle;
    ajpFeatCurrent = write_psi_phi(ajpFeattableTorsionAngles,
				   ajIntCarbonCurrent,
				   fPhiTorsionAngle,
				   fPsiTorsionAngle);

    /* loop through list until last residue to be analysed */
    ajIntPreviousResidueNumber = ajIntResidueNumber;
    shift_residues(arrayOfAjpatomsWindow,
		   arrayOfAjboolsWindow);
    do
    {
	ajIntResidueNumber = ajpAtomCurrentInList->Idx;
	/* ajIntCarbonCurrent residue no. for which angles calc'd */

	/* new residue? */
	if(ajIntResidueNumber > ajIntPreviousResidueNumber)
	{
	    /* analyse previous previous residue */
	    if(phi_calculable(arrayOfAjboolsWindow))
		fPhiTorsionAngle = phi(arrayOfAjpatomsWindow);
	    else
		fPhiTorsionAngle = FUnavailableAngle;
	    if(psi_calculable(arrayOfAjboolsWindow))
		fPsiTorsionAngle = psi(arrayOfAjpatomsWindow);
	    else
		fPsiTorsionAngle = FUnavailableAngle;
	    ajpFeatCurrent = write_psi_phi(ajpFeattableTorsionAngles,
					   ajIntCarbonCurrent,
					   fPhiTorsionAngle,
					   fPsiTorsionAngle);

	    shift_residues(arrayOfAjpatomsWindow, arrayOfAjboolsWindow);
	}
	/* not finished? get new next N, CA and C'  */
	if(ajIntResidueNumber <= ajIntFinalResidueNumber)
	{
	    /* conditional is kludge for bad residue numbers at chain termini */
	    if( ajIntResidueNumber > 1 )
		ajIntCarbonCurrent = ajIntResidueNumber-1;
	    load_next_residue(ajpAtomCurrentInList,
			      arrayOfAjpatomsWindow,
			      arrayOfAjboolsWindow);
	}
	else
	    break;
	ajIntPreviousResidueNumber = ajIntResidueNumber;
    }
    while((ajpAtomCurrentInList = ajListIterNext(ajIteratorAtomList)));

    /* conditional is kludge for bad residue numbers at chain termini */
    if( ajIntResidueNumber > 1 )
	ajIntCarbonCurrent = ajIntResidueNumber-1;

    /* analyse penultimate residue */
    if(ajIntCarbonCurrent < ajIntFinalResidueNumber)
    {

	if(phi_calculable(arrayOfAjboolsWindow))
	    fPhiTorsionAngle = phi(arrayOfAjpatomsWindow);
	else
	    fPhiTorsionAngle = FUnavailableAngle;
	if(psi_calculable(arrayOfAjboolsWindow))
	    fPsiTorsionAngle = psi(arrayOfAjpatomsWindow);
	else
	    fPsiTorsionAngle = FUnavailableAngle;

	ajpFeatCurrent = write_psi_phi(ajpFeattableTorsionAngles,
				       ajIntCarbonCurrent,
				       fPhiTorsionAngle,
				       fPsiTorsionAngle);
	ajIntCarbonCurrent++;
	shift_residues(arrayOfAjpatomsWindow,
		       arrayOfAjboolsWindow);
    }
    /* analyse last residue */
    if((ajIntCarbonCurrent < ajIntFinalResidueNumber ) &&
       (phi_calculable(arrayOfAjboolsWindow)))
    {
	fPhiTorsionAngle = phi(arrayOfAjpatomsWindow);
	fPsiTorsionAngle = FUnavailableAngle;
	ajpFeatCurrent   = write_psi_phi(ajpFeattableTorsionAngles,
					 ajIntCarbonCurrent,
					 fPhiTorsionAngle,
					 fPsiTorsionAngle);
    }
    /* END ANALYSIS OF CHAIN HERE */

    /* DDDDEBUG TEST INFO FOR TAIL OF REPORT */
    /*     ajFmtPrintS(&ajpStrReportTail, "This is some tail text"); */
    /*     ajReportSetTail(ajpReportTorsionAngles, ajpStrReportTail); */

    /* write the report to the output file */
    ajReportWrite(ajpReportTorsionAngles,
		  ajpFeattableTorsionAngles,
		  ajpSeqCleanChain);

    /* clear up windows */
    AJFREE(arrayOfAjpatomsWindow);
    AJFREE(arrayOfAjboolsWindow);

    /* clear up report objects */
    ajFeattableDel(&ajpFeattableTorsionAngles);

    /* delete the atom list */
    ajListIterFree(&ajIteratorAtomList);

    /* close the input file */
    ajFileClose(&ajpFileCleanProteinStructure);

    /* close the report file */
    ajReportDel(&ajpReportTorsionAngles);

    /* clear up the structure */
    /* JISON */ ajPdbDel(&ajpPdbCleanStructure);
    ajSeqDel(&ajpSeqCleanChain);

    /*  tidy up everything else... */
    ajExit();

    return 0;
}




/* @funcstatic chain_index ***************************************************
**
** check selected protein chain number present in structure file; return index
**
** @param [r] ajIntSelectedChainNumber [ajint] number of chain selected by user
** @param [r] ajIntHighestChainNumber [ajint] number of highest chain in
**                                            structure
** @param [r] ajIntLowestChainNumber [ajint] number of lowest chain in
**                                           structure
** @return [ajint] index
** @@
******************************************************************************/
static ajint chain_index(ajint ajIntSelectedChainNumber,
			 ajint ajIntHighestChainNumber,
			 ajint ajIntLowestChainNumber)
{
    /* ERROR: chain number too high */ 
    if(ajIntSelectedChainNumber > ajIntHighestChainNumber)
	ajDie("There is no chain %d---highest chain number: %d.",
	       ajIntSelectedChainNumber, ajIntHighestChainNumber );
    /* ERROR: chain number too low */ 
    if(ajIntSelectedChainNumber < ajIntLowestChainNumber )
	ajWarn("There is no chain %d---lowest chain number %d.",
	       ajIntSelectedChainNumber, ajIntLowestChainNumber );
    return ajIntSelectedChainNumber-1;
}




/* @funcstatic first_residue_number ***********************************
**
** check selected lower residue within chain's range and return 1st window res
**
** @param [r] ajpPdbCleanStructure [const AjPPdb] cleaned AjPPdb structure
** @param [r] ajIntChainIndex [ajint] number of user-selected chain in
**                                    structure
** @param [r] ajIntStartResidueNumber [ajint] user-selected lower residue
**                                            number
** @return [ajint] First window residue number
** @@
******************************************************************************/
static ajint first_residue_number (const AjPPdb ajpPdbCleanStructure,
				   ajint ajIntChainIndex,
				   ajint ajIntStartResidueNumber)
{
    ajint ajIntFirstResidueNumber  = 0;
    ajint ajIntLowestResidueNumber = 0;

    AjPAtom ajpAtomCurrentInList   = NULL;
    
    /* read first atom in list into memory, but keep it on list */
    ajListPeek(ajpPdbCleanStructure->Chains[ajIntChainIndex]->Atoms,
	       (void**)&ajpAtomCurrentInList);

    /* get number of lowest residue available in chain */
    ajIntLowestResidueNumber = ajpAtomCurrentInList->Idx;
    
    /* ERROR: start residue too low */ 
    if(ajIntStartResidueNumber < ajIntLowestResidueNumber)
    {	
	ajWarn("No residue %d---number of lowest residue in chain %d is %d.",
	       ajIntStartResidueNumber, ajIntChainIndex,
	       ajIntLowestResidueNumber );
    }

    /* use user-specified starting position... */
    if(ajIntStartResidueNumber > ajIntLowestResidueNumber)
	ajIntFirstResidueNumber = ajIntStartResidueNumber-1;
    /* ...or use start of chain */
    else
    {
	ajIntFirstResidueNumber = ajIntLowestResidueNumber-1;
	
    }

    return ajIntFirstResidueNumber;
}




/* @funcstatic last_residue_number ******************************************
**
** check selected upper protein residue within chain's range and return limit
**
** @param [r] ajpPdbCleanStructure [const AjPPdb] cleaned AjPPdb structure
** @param [r] ajIntChainIndex [ajint] number of user-selected chain in
**                                    structure
** @param [r] ajIntStartResidueNumber [ajint] user-selected lower residue
**                                             number
** @param [r] ajIntFinishResidueNumber [ajint] user-selected upper residue
**                                             number
** @return [ajint] Last residue number
** @@
******************************************************************************/
static ajint last_residue_number(const AjPPdb ajpPdbCleanStructure,
				 ajint ajIntChainIndex,
				 ajint ajIntStartResidueNumber,
				 ajint ajIntFinishResidueNumber)
{
    ajint ajIntFinalResidueNumber   = 0;
    ajint ajIntHighestResidueNumber = 0;

    /* get number of highest residue available in chain */
    ajIntHighestResidueNumber = 
	ajpPdbCleanStructure->Chains[ajIntChainIndex]->Nres;

    /* last residue defaults to end of chain... */
    if(ajIntFinishResidueNumber == 1)
	ajIntFinalResidueNumber = ajIntHighestResidueNumber+1;
    /* ERROR: finish residue too low */ 
    else if(ajIntFinishResidueNumber < ajIntStartResidueNumber)
	ajDie("Residue %d too low---number of lowest residue you chose is %d.",
	      ajIntFinishResidueNumber, ajIntStartResidueNumber );
    /* any other legitimate choice used as given */
    else if(ajIntFinishResidueNumber < ajIntHighestResidueNumber)
	    ajIntFinalResidueNumber = ajIntFinishResidueNumber+1;
    else
	/* ERROR: finish residue too high */ 
	ajDie("No residue %d---number of highest residue in chain %d is %d.",
	      ajIntFinishResidueNumber, ajIntChainIndex,
	      ajIntHighestResidueNumber );
    
    return ajIntFinalResidueNumber;
}




/* @funcstatic phi_calculable ************************************************
**
** are all necessary atoms present to calculate phi torsion angle?
**
** @param [r] arrayOfAjboolsWindow [const AjBool*] corresponding array of AjBools
** @return [AjBool] ajTrue if calculable
** @@
******************************************************************************/
static AjBool phi_calculable(const AjBool* arrayOfAjboolsWindow)
{
    AjBool ajBoolPhiCalculable = AJFALSE;
    /*
     * check for a complete set of atoms needed to calculate PHI
     */
    if(arrayOfAjboolsWindow[EPrimeCarbonPrevious] &&
       arrayOfAjboolsWindow[ENitrogenCurrent] &&
       arrayOfAjboolsWindow[EAlphaCarbonCurrent] &&
       arrayOfAjboolsWindow[EPrimeCarbonCurrent])
	ajBoolPhiCalculable = AJTRUE;

    return ajBoolPhiCalculable;
}




/* @funcstatic psi_calculable ************************************************
**
** are all necessary atoms present to calculate psi torsion angle?
**
** @param [r] arrayOfAjboolsWindow [const AjBool*] corresponding array of AjBools
** @return [AjBool] ajTrue if calculable
** @@
******************************************************************************/
static AjBool psi_calculable(const AjBool* arrayOfAjboolsWindow)
{
    AjBool ajBoolPsiCalculable = AJFALSE;
    /*
     * If you've got a complete set of the relevant
     * torsion atoms then calculate the PSI angle
     */
    if(arrayOfAjboolsWindow[ENitrogenCurrent] &&
       arrayOfAjboolsWindow[EAlphaCarbonCurrent] &&
       arrayOfAjboolsWindow[EPrimeCarbonCurrent] &&
       arrayOfAjboolsWindow[ENitrogenNext])
    {
	ajBoolPsiCalculable = AJTRUE;
    }
    return ajBoolPsiCalculable;
}




/* @funcstatic phi ***********************************************************
**
** returns the phi torsion angle between a specified set of AjPAtoms
**
** @param [r] arrayOfAjpatomsWindow [AjPAtom const *] window of nine mainchain atoms
** @return [float] phi torsion angle
** @@
******************************************************************************/

static float phi (AjPAtom const * arrayOfAjpatomsWindow)
{
    float fPhiTorsionAngle;
    
    AjP3dVector ajp3dVector1To2 = NULL;
    AjP3dVector ajp3dVector3To2 = NULL;
    AjP3dVector ajp3dVector3To4 = NULL;

    /* construct vectors between four atoms relevant to torsion angles */
    ajp3dVector1To2 = aj3dVectorNew();
    ajp3dVector3To2 = aj3dVectorNew();
    ajp3dVector3To4 = aj3dVectorNew();

    /* calculate PHI angle for current window */
    aj3dVectorBetweenPoints(ajp3dVector1To2,
			    arrayOfAjpatomsWindow[EPrimeCarbonPrevious]->X,
			    arrayOfAjpatomsWindow[EPrimeCarbonPrevious]->Y,
			    arrayOfAjpatomsWindow[EPrimeCarbonPrevious]->Z,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->X,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->Y,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->Z);
    aj3dVectorBetweenPoints(ajp3dVector3To2,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Z,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->X,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->Y,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->Z);
    aj3dVectorBetweenPoints(ajp3dVector3To4,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Z,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->Z);
		
    fPhiTorsionAngle = -1.0 *
	aj3dVectorDihedralAngle(ajp3dVector1To2,
				ajp3dVector3To2,
				ajp3dVector3To4);
    /* clean up vectors */
    aj3dVectorDel(&ajp3dVector1To2);
    aj3dVectorDel(&ajp3dVector3To2);
    aj3dVectorDel(&ajp3dVector3To4);
    
    return fPhiTorsionAngle;
}




/* @funcstatic psi ***********************************************************
**
** returns the psi torsion angle between a specified set of AjPAtoms
**
** @param [r] arrayOfAjpatomsWindow [AjPAtom const *] window of nine mainchain atoms
** @return [float]  psi torsion angle
** @@
******************************************************************************/

static float psi (AjPAtom const * arrayOfAjpatomsWindow)
{
    float fPsiTorsionAngle;
    
    AjP3dVector ajp3dVector1To2 = NULL;
    AjP3dVector ajp3dVector3To2 = NULL;
    AjP3dVector ajp3dVector3To4 = NULL;

    /* construct vectors between four atoms relevant to torsion angles */
    ajp3dVector1To2 = aj3dVectorNew();
    ajp3dVector3To2 = aj3dVectorNew();
    ajp3dVector3To4 = aj3dVectorNew();

    /* calculate PSI angle for current window */
    aj3dVectorBetweenPoints(ajp3dVector1To2,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->X,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->Y,
			    arrayOfAjpatomsWindow[ENitrogenCurrent]->Z,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Z);
		
    aj3dVectorBetweenPoints(ajp3dVector3To2,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->Z,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EAlphaCarbonCurrent]->Z);
		
    aj3dVectorBetweenPoints(ajp3dVector3To4,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->X,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->Y,
			    arrayOfAjpatomsWindow[EPrimeCarbonCurrent]->Z,
			    arrayOfAjpatomsWindow[ENitrogenNext]->X,
			    arrayOfAjpatomsWindow[ENitrogenNext]->Y,
			    arrayOfAjpatomsWindow[ENitrogenNext]->Z);
    fPsiTorsionAngle = -1.0 *
	aj3dVectorDihedralAngle(ajp3dVector1To2,
				ajp3dVector3To2,
				ajp3dVector3To4);
    /* clean up vectors */
    aj3dVectorDel(&ajp3dVector1To2);
    aj3dVectorDel(&ajp3dVector3To2);
    aj3dVectorDel(&ajp3dVector3To4);

    return fPsiTorsionAngle;
}




/* @funcstatic load_previous_residue *****************************************
**
** checks and/or loads one mainchain AjPAtom from into window of AjPAtoms
** returns AJFALSE if non-residue atom
**
** @param [u] ajpAtom [AjPAtom] current AjPAtom from ajPPdb object
** @param [u] arrayOfAjpatomsWindow [AjPAtom*] array of nine mainchain atoms
** @param [u] arrayOfAjboolsWindow [AjBool*] corresponding array of AjBools
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool load_previous_residue(AjPAtom ajpAtom,
				    AjPAtom* arrayOfAjpatomsWindow,
				    AjBool* arrayOfAjboolsWindow)
{
    AjBool ajBoolIsResidueAtom = AJFALSE;

    if(ajpAtom->Id1 == 'X')
    {
	/* do nothing: this is not a residue */
	ajBoolIsResidueAtom = AJFALSE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "N"))
    {
	arrayOfAjboolsWindow[ENitrogenPrevious] = AJTRUE;
	arrayOfAjpatomsWindow[ENitrogenPrevious]  = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "CA"))
    {
	arrayOfAjboolsWindow[EAlphaCarbonPrevious] = AJTRUE;
	arrayOfAjpatomsWindow[EAlphaCarbonPrevious]  = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "C"))
    {
	arrayOfAjboolsWindow[EPrimeCarbonPrevious] = AJTRUE;
	arrayOfAjpatomsWindow[EPrimeCarbonPrevious]  = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    
    return ajBoolIsResidueAtom;
}




/* @funcstatic load_current_residue ******************************************
**
** check and/or loads one mainchain AjPAtom into window of AjPAtoms
** returns AJFALSE if non-residue atom
**
** @param [u] ajpAtom [AjPAtom] current AjPAtom from ajPPdb object
** @param [u] arrayOfAjpatomsWindow [AjPAtom*] array of nine mainchain atoms
** @param [u] arrayOfAjboolsWindow [AjBool*] corresponding array of AjBools
** @return [AjBool]  ajTrue on success
** @@
******************************************************************************/

static AjBool load_current_residue(AjPAtom ajpAtom,
				   AjPAtom* arrayOfAjpatomsWindow,
				   AjBool* arrayOfAjboolsWindow)
{
    AjBool ajBoolIsResidueAtom = AJFALSE;
    
    if(ajpAtom->Id1 == 'X')
    {
	/* do nothing: this is not a residue */
	ajBoolIsResidueAtom = AJFALSE;
	
    }
    else if(ajStrMatchC(ajpAtom->Atm, "N"))
    {
	arrayOfAjboolsWindow[ENitrogenCurrent]  = AJTRUE;
	arrayOfAjpatomsWindow[ENitrogenCurrent] = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "CA"))
    {
	arrayOfAjboolsWindow[EAlphaCarbonCurrent]  = AJTRUE;
	arrayOfAjpatomsWindow[EAlphaCarbonCurrent] = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "C"))
    {
	arrayOfAjboolsWindow[EPrimeCarbonCurrent]  = AJTRUE;
	arrayOfAjpatomsWindow[EPrimeCarbonCurrent] = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    
    return ajBoolIsResidueAtom;
}




/* @funcstatic load_next_residue *********************************************
**
** loads AjPAtoms from next residue into window; returns AJTRUE if window full
** returns AJFALSE if non-residue atom
**
** @param [u] ajpAtom [AjPAtom] current AjPAtom from ajPPdb object
** @param [u] arrayOfAjpatomsWindow [AjPAtom*] array of nine mainchain atoms
** @param [u] arrayOfAjboolsWindow [AjBool*] corresponding array of AjBools
** @return [AjBool]  ajTrue on success
** @@
******************************************************************************/

static AjBool load_next_residue(AjPAtom ajpAtom,
				AjPAtom* arrayOfAjpatomsWindow,
				AjBool* arrayOfAjboolsWindow)
{
    AjBool ajBoolIsResidueAtom = AJFALSE;

    if(ajpAtom->Id1 == 'X')
    {
	/* do nothing: this is not a residue */
	;
	
    }
    else if(ajStrMatchC(ajpAtom->Atm, "N"))
    {
	arrayOfAjboolsWindow[ENitrogenNext]  = AJTRUE;
	arrayOfAjpatomsWindow[ENitrogenNext] = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "CA"))
    {
	arrayOfAjboolsWindow[EAlphaCarbonNext]  = AJTRUE;
	arrayOfAjpatomsWindow[EAlphaCarbonNext] = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }
    else if(ajStrMatchC(ajpAtom->Atm, "C"))
    {
	arrayOfAjboolsWindow[EPrimeCarbonNext]  = AJTRUE;
	arrayOfAjpatomsWindow[EPrimeCarbonNext] = ajpAtom;
	ajBoolIsResidueAtom = AJTRUE;
    }

    return ajBoolIsResidueAtom;
}




/* @funcstatic write_psi_phi *************************************************
**
** writes torsion angle features to a feature table and returns new feature  
**
** @param [u] ajpFeattableTorsionAngles [AjPFeattable] table to write torsion
**                                                     angle to
** @param [r] ajIntFeatureResidueNumber [ajint] residue that angle belongs to
** @param [r] fPhiTorsionAngle [float] phi torsion angle for residue
** @param [r] fPsiTorsionAngle [float] psi torsion angle for residue
** @return [AjPFeature] New feature stored in feature table
** @@
******************************************************************************/

static AjPFeature write_psi_phi (AjPFeattable ajpFeattableTorsionAngles,
				 ajint ajIntFeatureResidueNumber,
				 float fPhiTorsionAngle,
				 float fPsiTorsionAngle)
{
    AjPFeature ajpFeatTorsionAngles;
    AjPStr ajpStrFeatTemp;

    ajpStrFeatTemp = ajStrNew();

    /* create feature for torsion angles and write psi/phi */
    ajpFeatTorsionAngles = ajFeatNewII(ajpFeattableTorsionAngles,
				       ajIntFeatureResidueNumber,
				       ajIntFeatureResidueNumber);
    ajFmtPrintS(&ajpStrFeatTemp, "*phi: %7.2f", fPhiTorsionAngle);
    ajFeatTagAdd(ajpFeatTorsionAngles, NULL, ajpStrFeatTemp);
    ajFmtPrintS(&ajpStrFeatTemp, "*psi: %7.2f", fPsiTorsionAngle);
    ajFeatTagAdd(ajpFeatTorsionAngles, NULL, ajpStrFeatTemp);

    ajStrDel(&ajpStrFeatTemp);
    
    return ajpFeatTorsionAngles;
}




/* @funcstatic shift_residues ***************************************
**
** moves AjPAtoms one residue along an array of mainchain AjPAtoms
**
** @param [u] arrayOfAjpatomsWindow [AjPAtom*] array of nine mainchain atoms
** @param [u] arrayOfAjboolsWindow [AjBool*] corresponding array of AjBools
*8 @return [void]
** @@
******************************************************************************/

static void shift_residues(AjPAtom* arrayOfAjpatomsWindow,
				     AjBool* arrayOfAjboolsWindow)
{
    /* move previous atoms */
    if(arrayOfAjboolsWindow[ENitrogenCurrent])
    {
	/* use old current N as new previous N */
	arrayOfAjboolsWindow[ENitrogenPrevious]=
	    arrayOfAjboolsWindow[ENitrogenCurrent];
	arrayOfAjpatomsWindow[ENitrogenPrevious] =
	    arrayOfAjpatomsWindow[ENitrogenCurrent];
    }
    if(arrayOfAjboolsWindow[EAlphaCarbonCurrent])
    {
	/* use old current CA as new previous CA */
	arrayOfAjboolsWindow[EAlphaCarbonPrevious] =
	    arrayOfAjboolsWindow[EAlphaCarbonCurrent];
	arrayOfAjpatomsWindow[EAlphaCarbonPrevious] =
	    arrayOfAjpatomsWindow[EAlphaCarbonCurrent];
    }
    if(arrayOfAjboolsWindow[EPrimeCarbonCurrent])
    {
	/* use old current C' as new previous C' */
	arrayOfAjboolsWindow[EPrimeCarbonPrevious] =
	    arrayOfAjboolsWindow[EPrimeCarbonCurrent];
	arrayOfAjpatomsWindow[EPrimeCarbonPrevious] =
	    arrayOfAjpatomsWindow[EPrimeCarbonCurrent];
    }
    if(arrayOfAjboolsWindow[ENitrogenNext])
    { 
	/* use old next N as new current N */
	arrayOfAjboolsWindow[ENitrogenCurrent] =
	    arrayOfAjboolsWindow[ENitrogenNext];
	arrayOfAjpatomsWindow[ENitrogenCurrent] =
	    arrayOfAjpatomsWindow[ENitrogenNext];
    }
    if(arrayOfAjboolsWindow[EAlphaCarbonNext])
    {    
	/* use old next CA as new current CA */
	arrayOfAjboolsWindow[EAlphaCarbonCurrent] =
	    arrayOfAjboolsWindow[EAlphaCarbonNext];
	arrayOfAjpatomsWindow[EAlphaCarbonCurrent] =
	    arrayOfAjpatomsWindow[EAlphaCarbonNext];
    }
    if(arrayOfAjboolsWindow[EPrimeCarbonNext])
    {	    
	/* use old next C as new current C */
	arrayOfAjboolsWindow[EPrimeCarbonCurrent] =
	    arrayOfAjboolsWindow[EPrimeCarbonNext];
	arrayOfAjpatomsWindow[EPrimeCarbonCurrent] =
	    arrayOfAjpatomsWindow[EPrimeCarbonNext];
    }
    /* clear next atoms */
    arrayOfAjboolsWindow[EPrimeCarbonNext] = AJFALSE;
    arrayOfAjboolsWindow[ENitrogenNext]    = AJFALSE;
    arrayOfAjboolsWindow[EAlphaCarbonNext] = AJFALSE;
    
    return;
}

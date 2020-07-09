/********************************************************************
** @source AJAX structure functions
**
** @author Copyright (C) 2000 Jon Ison (jison@hgmp.mrc.ac.uk)
** @author Copyright (C) 2000 Alan Bleasby
** @version 1.0 
** @modified Jan 22 JCI First version
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
********************************************************************/


/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include <math.h>
#include "ajax.h"


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

#define OUT_BLK       75

/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */


/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Protein Constructors ********************************************
**
** All constructors return a new item by pointer. It is the responsibility
** of the user to first destroy any previous item. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
** The range of constructors is provided to allow flexibility in how
** applications can define new items.
**
******************************************************************************/

/* @func ajXyzPdbNew ************************************************************
**
** Pdb object constructor. Fore-knowledge of the number of chains 
** is required. This is normally called by the Cpdb reading
** routine.
**
** @param [r] chains [ajint] Number of chains in this pdb entry
**
** @return [AjPPdb] Pointer to a pdb object
** @@
******************************************************************************/

AjPPdb ajXyzPdbNew(ajint chains)
{
    AjPPdb ret=NULL;
    ajint i;
    
    AJNEW0(ret);
  

    ret->Pdb = ajStrNew();
    ret->Compnd = ajStrNew();
    ret->Source = ajStrNew();

    AJCNEW0(ret->Chains,chains);
    for(i=0;i<chains;++i)
	ret->Chains[i] = ajXyzChainNew();
    
    return ret;
}

/* @func ajXyzChainNew ***********************************************************
**
** Chain object constructor. 
** This is normally called by the ajXyzPdbNew function
**
** @return [AjPChain] Pointer to a chain object
** @@
******************************************************************************/

AjPChain ajXyzChainNew(void)
{
  AjPChain ret=NULL;
  
  AJNEW0(ret);

  ret->Seq    = ajStrNewC("");
  ret->Atoms  = ajListNew();

  return ret;
}

/* @func ajXyzAtomNew ***********************************************************
**
** Constructor for atom co-ordinates.
** This is normally called by the ajXyzChainNew function
**
** @return [AjPAtom] Pointer to an atom object
** @@
******************************************************************************/

AjPAtom ajXyzAtomNew(void)
{
    AjPAtom ret=NULL;

    AJNEW0(ret);
    
    ret->Id3 = ajStrNew();
    ret->Atm = ajStrNew();
    ret->Pdb = ajStrNew();

    return ret;
}

/* @func ajXyzScopNew ***********************************************************
**
** Scop object constructor. Fore-knowledge of the number of chains is 
** required.
**
** @param [r] chains [ajint] Number of chains
**
** @return [AjPScop] Pointer to a scop object
** @@
******************************************************************************/

AjPScop ajXyzScopNew(ajint chains)
{

    AjPScop ret = NULL;
    ajint i;

    AJNEW0(ret);

    ret->Entry       = ajStrNew();
    ret->Pdb         = ajStrNew();
    ret->Class       = ajStrNew();
    ret->Fold        = ajStrNew();
    ret->Superfamily = ajStrNew();
    ret->Family      = ajStrNew();
    ret->Domain      = ajStrNew();
    ret->Source      = ajStrNew();


    if(chains)
    {
	ret->Chain=ajCharNewL(chains);
	AJCNEW0(ret->Start,chains);
	AJCNEW0(ret->End,chains);
	for(i=0; i<chains; i++)
	{
	    ret->Start[i]=ajStrNew();
	    ret->End[i]=ajStrNew();
	}
    }

    ret->N = chains;

    return ret;
}


/* ==================================================================== */
/* ========================= Destructors ============================== */
/* ==================================================================== */



/* @func ajXyzPdbDel ***********************************************************
**
** Destructor for pdb object.
**
** @param [w] thys [AjPPdb*] Pdb object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajXyzPdbDel(AjPPdb *thys)
{
    AjPPdb pthis = *thys;
    
    ajint nc=0;
    ajint i=0;

    if(!pthis || !thys)
	return;
    
    nc = pthis->Nchn;

    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Compnd);
    ajStrDel(&pthis->Source);
    
    for(i=0;i<nc;++i)
	ajXyzChainDel(&pthis->Chains[i]);
    AJFREE(pthis);

    return;
}


/* @func ajXyzChainDel ***********************************************************
**
** Destructor for chain object.
**
** @param [w] thys [AjPChain*] Chain object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajXyzChainDel(AjPChain *thys)
{
    AjPChain pthis = *thys;
    AjPAtom atm=NULL;

    if(!thys || !pthis)
	return;
    
    while(ajListPop(pthis->Atoms,(void **)&atm))
	ajXyzAtomDel(&atm);

    ajListDel(&pthis->Atoms);

    AJFREE(pthis);

    return;
}

/* @func ajXyzAtomDel ***********************************************************
**
** Destructor for atom co-ordinates.
**
** @param [w] thys [AjPAtom*] Atom object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajXyzAtomDel(AjPAtom *thys)
{
    AjPAtom pthis = *thys;

    if(!thys || !pthis)
	return;

    ajStrDel(&pthis->Id3);
    ajStrDel(&pthis->Atm);
    ajStrDel(&pthis->Pdb);

    AJFREE(pthis);

    return;
}

/* @func ajXyzScopDel ***********************************************************
**
** Destructor for scop object.
**
** @param [w] thys [AjPScop*] Atom object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajXyzScopDel(AjPScop *thys)
{
    AjPScop pthis = *thys;
    
    ajint i;

    if(!pthis || !thys)
	return;

    ajStrDel(&pthis->Entry);
    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Class);
    ajStrDel(&pthis->Fold);
    ajStrDel(&pthis->Superfamily);
    ajStrDel(&pthis->Family);
    ajStrDel(&pthis->Domain);
    ajStrDel(&pthis->Source);


    if(pthis->N)
    {
	for(i=0; i<pthis->N; i++)
	{
	    ajStrDel(&pthis->Start[i]);
	    ajStrDel(&pthis->End[i]);
	}
	AJFREE(pthis->Start);
	AJFREE(pthis->End);
	AJFREE(pthis->Chain);
    }

    AJFREE(pthis);

    return;
}



/* ==================================================================== */
/* ======================== Exported functions ======================== */
/* ==================================================================== */

/* @func ajXyzCpdbRead ***********************************************************
**
** Reads a Cpdb database entry and returns a filled Pdb object.
** Needs modifying to return ajFalse in case of bad format etc
**
** @param [r] inf  [AjPFile] Pointer to cpdb file
** @param [w] thys [AjPPdb*] Pdb object pointer
**
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajXyzCpdbRead(AjPFile inf, AjPPdb *thys)
{
    ajint         nmod =0;
    ajint         ncha =0;
    ajint           nc =0;
    ajint          mod =0;
    ajint          chn =0;
    ajint     last_chn =0;
    ajint     last_mod =0;
    ajint done_co_line =0;

    float       reso =0.0;

    AjPStr      line =NULL;
    AjPStr     token =NULL;
    AjPStr     idstr =NULL;
    AjPStr     destr =NULL;
    AjPStr     osstr =NULL;
    AjPStr      xstr =NULL;
    AjPStrTok handle =NULL;
    
    AjPAtom     atom =NULL;


    



    /* Intitialise strings */
    line  = ajStrNew();
    token = ajStrNew();
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr  = ajStrNew();






    /* Start of main application loop*/
    while(ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"XX"))
	    continue;
	

	/* Parse ID */
	if(ajStrPrefixC(line,"ID"))
	{
	    ajStrTokenAss(&handle,line," \n\t\r");
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&idstr,&handle,NULL);
	    continue;
	}

	
	/* Parse number of chains*/
	if(ajStrPrefixC(line,"CN"))
	{
	    ajStrTokenAss(&handle,line," []\n\t\r");
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToInt(token,&nc);
	    continue;
	}
	

	/* Parse description text*/
	if(ajStrPrefixC(line,"DE"))
	{
	    (void) ajStrTokenAss (&handle, line, " ");
	    (void) ajStrToken (&token, &handle, NULL);
	    /* 'DE' */
	    (void) ajStrToken (&token, &handle, "\n\r");
	    /* desc */
	    if (ajStrLen(destr))
	    {
		(void) ajStrAppC (&destr, " ");
		(void) ajStrApp (&destr, token);
	    }
	    else
		(void) ajStrAss (&destr, token);
	    continue;
	}


	/* Parse source text */
	if(ajStrPrefixC(line,"OS"))
	{
	    (void) ajStrTokenAss (&handle, line, " ");
	    (void) ajStrToken (&token, &handle, NULL);
	    /* 'OS' */
	    (void) ajStrToken (&token, &handle, "\n\r");
	    /* source */
	    if (ajStrLen(osstr))
	    {
		(void) ajStrAppC (&osstr, " ");
		(void) ajStrApp (&osstr, token);
	    }
	    else
		(void) ajStrAss (&osstr, token);
	    continue;
	}
	

	/* Parse experimental line*/
	if(ajStrPrefixC(line,"EX"))
	{
	    ajStrTokenAss(&handle,line," ;\n\t\r");
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL);

	    ajStrToken(&xstr,&handle,NULL); /* xray */
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* reso */
	    ajStrToFloat(token,&reso);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* nmod */
	    ajStrToInt(token,&nmod);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* ncha */
	    
	    ajStrToInt(token,&ncha);

	    *thys = ajXyzPdbNew(ncha);

	    ajStrAssS(&(*thys)->Pdb,idstr);
	    ajStrAssS(&(*thys)->Compnd,destr);
	    ajStrAssS(&(*thys)->Source,osstr);
	    if(ajStrMatchC(xstr,"xray"))
		(*thys)->Method = ajXRAY;
	    else
		(*thys)->Method = ajNMR;

	    (*thys)->Reso = reso;
	    (*thys)->Nmod = nmod;
	    (*thys)->Nchn = ncha;
	}
	

	/* Parse information line*/
	if(ajStrPrefixC(line,"IN"))
	{
	    ajStrTokenAss(&handle,line," ;\n\t\r");
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* id value */
	    (*thys)->Chains[nc-1]->Id=*ajStrStr(token);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* residues */
	    ajStrToInt(token,&(*thys)->Chains[nc-1]->Nres);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* hetatm */
	    ajStrToInt(token,&(*thys)->Chains[nc-1]->Nhet);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL); /* water */
	    ajStrToInt(token,&(*thys)->Chains[nc-1]->Nwat);
	    continue;
	}
  

	/* Parse sequence line*/
	if(ajStrPrefixC(line,"SQ"))
	{
	    while(ajFileReadLine(inf,&line) && !ajStrPrefixC(line,"XX"))
		ajStrAppC(&(*thys)->Chains[nc-1]->Seq,ajStrStr(line));
	    ajStrCleanWhite(&(*thys)->Chains[nc-1]->Seq);
	    continue;
	}


	/* Parse coordinate line*/
	if(ajStrPrefixC(line,"CO"))
	{
	    ajStrTokenAss(&handle,line," \t\n\r");
	    ajStrToken(&token,&handle,NULL);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToInt(token,&mod);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToInt(token,&chn);
	    
	    AJNEW0(atom);
	    atom->Mod = mod;
	    atom->Chn = chn;


	    if(done_co_line == 0)
	    {
		last_chn=chn;
		last_mod=mod;
		done_co_line=1;
	    }
	    
	    ajStrToken(&token,&handle,NULL);
	    atom->Type = *ajStrStr(token);
	    
	    ajStrToken(&token,&handle,NULL);
	    ajStrToInt(token,&atom->Idx);

	    ajStrToken(&token,&handle,NULL);
	    ajStrAssS(&atom->Pdb,token);

	    ajStrToken(&token,&handle,NULL);
	    atom->Id1 = *ajStrStr(token);
	    
	    ajStrToken(&token,&handle,NULL);
	    ajStrAssS(&atom->Id3,token);

	    ajStrToken(&token,&handle,NULL);
	    ajStrAssS(&atom->Atm,token);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToFloat(token,&atom->X);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToFloat(token,&atom->Y);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToFloat(token,&atom->Z);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToFloat(token,&atom->O);

	    ajStrToken(&token,&handle,NULL);
	    ajStrToFloat(token,&atom->B);

	    ajListPushApp((*thys)->Chains[chn-1]->Atoms,(void *)atom);
	}
    }
    /* End of main application loop*/
    





    /* Tidy up*/
    ajStrTokenClear(&handle);
    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&idstr);
    ajStrDel(&destr);
    ajStrDel(&osstr);
    ajStrDel(&xstr);


    /* Bye Bye*/
    return ajTrue;
}







/* @func ajXyzCpdbWriteDomain ***************************************************
**
** Write domain-specific contents of a Pdb object to an output file in cpdb 
** format
** Hard-coded to write data for model 1.  For these files, the coordinates
** are presented as belonging to a single chain regardless of how many chains
** the domain comprised.
**
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [r] pdb  [AjPPdb]  Pdb object
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on success
** @@
** 
******************************************************************************/
AjBool ajXyzCpdbWriteDomain(AjPFile errf, AjPFile outf, AjPPdb pdb, AjPScop scop)
{
    /*rn_mod is a modifier to the residue number to give correct residue
      numbering for the domain*/
    ajint      z;
    ajint      chn;
    ajint      start       =0;
    ajint      end         =0;
    ajint      finalrn     =0;
    ajint      rn_mod      =0;  
    ajint      last_rn     =0;  
    ajint      this_rn;
    char     id;
    
    AjPStr   tmpseq      =NULL;   
    AjPStr   seq         =NULL;   
    
    AjBool   found_start =ajFalse;
    AjBool   found_end   =ajFalse;
    AjBool   nostart     =ajFalse;
    AjBool   noend       =ajFalse;
    AjIList  iter        =NULL;
    AjPAtom  atm         =NULL;
    AjPAtom  atm2        =NULL;
    
    
    
    
    
    
    /* Intitialise strings*/
    seq   =ajStrNew();
    tmpseq=ajStrNew();
    
    
    /* Check for unknown or zero-length chains*/
    for(z=0;z<scop->N;z++)
	if(!ajXyzPdbChain(scop->Chain[z], pdb, &chn))
	{
	    ajWarn("Chain incompatibility error in "
		   "ajXyzCpdbWriteDomain");			
		
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
			"error in ajXyzCpdbWriteDomain\n", scop->Entry);
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    return ajFalse;
	}
	else if(pdb->Chains[chn-1]->Nres==0)
	{		
	    ajWarn("Chain length zero");			
	    
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n", 
			scop->Entry);
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    return ajFalse;
	}
    
    
    /* Write header info. to domain coordinate file*/
    ajFmtPrintF(outf, "%-5s%S\n", "ID", scop->Entry);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sCo-ordinates for SCOP domain %S\n", 
		"DE", scop->Entry);
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sSee Escop.dat for domain classification\n", 
		"OS");
    ajFmtPrintF(outf, "XX\n");
    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");
    if(pdb->Method == ajXRAY)
	ajFmtPrintF(outf, "xray; ");	
    else
	ajFmtPrintF(outf, "nmr_or_model; ");		
    ajFmtPrintF(outf, "RESO %.2f; NMOD 1; NCHA 1;\n", pdb->Reso);
    /* JCI The NCHA and NMOD are hard-coded to 1 for domain files*/
    
    
    /* Start of main application loop*/
    /* Print out data up to co-ordinates list*/
    for(z=0;
	z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse, 
	nostart=ajFalse, noend=ajFalse, last_rn=0)
    {	
	/* Unknown or zero length chains have already been checked for
	   so no additional checking is needed here */
	ajXyzPdbChain(scop->Chain[z], pdb, &chn);
	

	/* Initialise the iterator*/
	iter=ajListIter(pdb->Chains[chn-1]->Atoms);


	/*If start of domain is unspecified 
	  then assign start to first residue in chain*/
	if(!ajStrCmpC(scop->Start[z], "."))
	{
	    nostart = ajTrue;
	    start=1;
	    found_start=ajTrue;	
	}
		


	/*If end of domain is unspecified 
	  then assign end to last residue in chain*/
	if(!ajStrCmpC(scop->End[z], "."))
	{
	    noend = ajTrue;
	    end=pdb->Chains[chn-1]->Nres;
	    found_end=ajTrue;	
	}
		

	/*Find start and end of domains in chain*/
	if(!found_start || !found_end)
	{
	    /* Iterate through the list of atoms*/
	    while((atm=(AjPAtom)ajListIterNext(iter)))
	    {
		/* JCI hard-coded to work on model 1*/
		/* Break if a non-protein atom is found or model no. !=1*/
		if(atm->Type!='P' || atm->Mod!=1 
		   || (found_start && found_end))
		    break;


		/* If we are onto a new residue*/
		this_rn=atm->Idx;
		if(this_rn!=last_rn)
		{
		    last_rn=this_rn;


		    /* The start position was specified, but has not 
		       been found yet*/
		    if(!found_start && !nostart)		
		    {
			/* Start position found */
			if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
			{
			    start=atm->Idx;
			    found_start=ajTrue;	
			}
			else	
			    continue;
		    }


		    /* The end position was specified, but has not 
		       been found yet*/
		    if(!found_end && !noend)		
		    {
			/* End position found */
			if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
			{
			    end=atm->Idx;
			    found_end=ajTrue;       
			    break;
			}
		    }	
		}
	    }
	}
	
	

	/* Diagnostics if start position was not found*/
	if(!found_start)		
	{
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajListIterFree(iter);	
	    ajWarn("Domain start not found in ajXyzCpdbWriteDomain");
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain start not found "
			"in in ajXyzCpdbWriteDomain\n", scop->Entry);
	    return ajFalse;
	}
	

	/* Diagnostics if end position was not found*/
	if(!found_end)		
	{
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajListIterFree(iter);	
	    ajWarn("Domain end not found in ajXyzCpdbWriteDomain");
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain end not found "
			"in ajXyzCpdbWriteDomain\n", scop->Entry);
	    return ajFalse;
	}
	

	/*Write <seq> string here */
	ajStrAssSub(&tmpseq, pdb->Chains[chn-1]->Seq, start-1, end-1);
	ajStrApp(&seq, tmpseq);


	/* Free the iterator*/
	ajListIterFree(iter);	
    }
    /* End of main application loop*/
    
    
    



    /* If the domain was composed of more than once chain then a '.' is
       given as the chain identifier*/
    if(scop->N > 1)
	id = '.';
    else
	id = pdb->Chains[chn-1]->Id;


    /* Write sequence to domain coordinate file*/
    ajFmtPrintF(outf, "XX\n");	
    ajFmtPrintF(outf, "%-5s[1]\n", "CN");	
    ajFmtPrintF(outf, "XX\n");	
    ajFmtPrintF(outf, "%-5sID %c; NR %d; NH 0; NW 0;\n", 
		"IN", 
		id,
		ajStrLen(seq));
    ajFmtPrintF(outf, "XX\n");	
    ajSeqWriteCdb(outf, seq);
    ajFmtPrintF(outf, "XX\n");	

    
    /* Write co-ordinates list to domain coordinate file*/        
    for(nostart=ajFalse, noend=ajFalse, 
	z=0;z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse)
    {
	/* Unknown or zero length chains have already been checked for
	   so no additional checking is needed here */

	ajXyzPdbChain(scop->Chain[z], pdb, &chn);
	
	
	/* Initialise the iterator*/
	iter=ajListIter(pdb->Chains[chn-1]->Atoms);


	/*Increment res. counter from last chain if appropriate*/
	if(noend)
	    rn_mod += atm2->Idx;
	else	 
	    rn_mod += finalrn;

	
	/*Check whether start and end of domain are specified*/
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else
	    nostart=ajFalse;
	
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else 
	    noend=ajFalse;
	

	/* Iterate through the list of atoms*/
	while((atm=(AjPAtom)ajListIterNext(iter)))
	{
	    /* Break if a non-protein atom is found or model no. !=1*/
	    if(atm->Mod!=1 || atm->Type!='P')
		break;
	    
	    
	    /* The start position has not been found yet*/
	    if(!found_start)
	    {
		/* Start position was specified*/
		if(!nostart)
		{
		    /* Start position found*/
		    if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
		    {
			rn_mod -= atm->Idx-1;
			found_start=ajTrue;	
		    }
		    else	
			continue;
		}
		else	
		{
		    found_start=ajTrue;	
		}
	    }	

	    
	    /* The end position was specified, but has not 
	       been found yet*/
	    if(!found_end && !noend)
	    {
		/* End position found */
		if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
		{
		    found_end=ajTrue;     
		    finalrn=atm->Idx;
		}
	    }	
	    /* The end position was specified and has been found, and
	       the current atom no longer belongs to this final residue*/
	    else if(atm->Idx != finalrn && !noend)
		break;
	    
	    
	    /* Print out coordinate line*/
	    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-6d%-6S%-2c%6S    %-4S"
			"%8.3f%9.3f%9.3f%9.2f%9.2f\n", 
			"CO", 
			atm->Mod, 
			1,		/*JCI chn number is always given as 1*/
			atm->Type, 
			atm->Idx+rn_mod, 
			atm->Pdb, 
			atm->Id1, 
			atm->Id3,
			atm->Atm, 
			atm->X, 
			atm->Y, 
			atm->Z, 
			atm->O, 
			atm->B);


	    /* Assign pointer for this chain*/
	    atm2=atm;
	}
	/*Free list iterator*/
	ajListIterFree(iter);			
    } 	
    
    
    /* Write last line in file*/
    ajFmtPrintF(outf, "//\n");    
    

    /* Tidy up*/
    ajListIterFree(iter);	    
    ajStrDel(&seq);
    ajStrDel(&tmpseq);
    

    /* Bye Bye*/
    return ajTrue;
}





/* @func ajXyzCpdbWriteAll ******************************************************
**
** Write contents of a Pdb object to an output file in cpdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] thys [AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajXyzCpdbWriteAll(AjPFile outf, AjPPdb thys)
{
    ajint         x;
    ajint         y;
    AjIList  iter =NULL;
    AjPAtom   tmp =NULL;
    





    /* Write the header information*/
    ajFmtPrintF(outf, "%-5s%S\n", "ID", thys->Pdb);
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintSplit(outf,thys->Compnd,"DE   ",75," \t\r\n");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintSplit(outf,thys->Source,"OS   ",75," \t\r\n");
    ajFmtPrintF(outf, "XX\n");

    ajFmtPrintF(outf, "%-5sMETHOD ", "EX");
    if(thys->Method == ajXRAY)
	ajFmtPrintF(outf, "xray; ");	
    else
	ajFmtPrintF(outf, "nmr_or_model; ");		
    ajFmtPrintF(outf, "RESO %.2f; NMOD %d; NCHA %d;\n", thys->Reso,
		thys->Nmod, thys->Nchn);


    /* Write chain-specific information*/
    for(x=0;x<thys->Nchn;x++)
    { 
	ajFmtPrintF(outf, "XX\n");	
	ajFmtPrintF(outf, "%-5s[%d]\n", 
		    "CN", 
		    x+1);	
	ajFmtPrintF(outf, "XX\n");	
	ajFmtPrintF(outf, "%-5sID %c; NR %d; NH %d; NW %d;\n", 
		    "IN", 
		    thys->Chains[x]->Id,
		    thys->Chains[x]->Nres,
		    thys->Chains[x]->Nhet,
		    thys->Chains[x]->Nwat);
	ajFmtPrintF(outf, "XX\n");	
	ajSeqWriteCdb(outf, thys->Chains[x]->Seq);
    }
    ajFmtPrintF(outf, "XX\n");	


    /* Write coordinate list*/
    for(x=1;x<=thys->Nmod;x++)
    {
	for(y=0;y<thys->Nchn;y++)
	{
	    iter=ajListIter(thys->Chains[y]->Atoms);
	    while(ajListIterMore(iter))
	    {
		tmp=(AjPAtom)ajListIterNext(iter);
		if(tmp->Mod!=x)
			continue;
		else	
		{
		    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-6d%-6S%-2c"
				"%6S    %-4S"
				"%8.3f%9.3f%9.3f%9.2f%9.2f\n", 
				"CO", 
				tmp->Mod, 
				tmp->Chn, 
				tmp->Type, 
				tmp->Idx, 
				tmp->Pdb, 
				tmp->Id1, 
				tmp->Id3,
				tmp->Atm, 
				tmp->X, 
				tmp->Y, 
				tmp->Z,
				tmp->O,
				tmp->B);
		}
	    }
	    ajListIterFree(iter);			
	} 	
    }
    ajFmtPrintF(outf, "//\n");    


    /* Bye Bye*/
    return ajTrue;
}







/* @func ajXyzPdbChain **********************************************************
**
** Finds the chain number for a given chain in a pdb structure
**
** @param [w] chn [int *] Chain number
** @param [r] id  [char] Chain identifier
** @param [r] pdb [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPdbChain(char id, AjPPdb pdb, ajint *chn)
{
    ajint a;
    
    for(a=0;a<pdb->Nchn;a++)
	if(toupper(pdb->Chains[a]->Id) == toupper(id))
	{
	    *chn=a+1;
	    return ajTrue;
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




/* @func ajXyzPrintPdbText ******************************************************
**
** Writes pdb-formatted text to an output file
**
** @param [w] outf   [AjPFile] Output file stream
** @param [r] str    [AjPStr]  Text to print out
** @param [r] prefix [char *]  Text to print out at start of line
** @param [r] len    [ajint]     Width of record to print into
** @param [r] delim  [char *]  String for tokenization of text
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajXyzPrintPdbText(AjPFile outf, AjPStr str, char *prefix, ajint len, 
		       char *delim)
{
    ajint        n      = 0;
    ajint        l      = 0;
    ajint        c      = 0;

    AjPStrTok  handle = NULL;
    AjPStr     token  = NULL;
    AjPStr     tmp    = NULL;
    
    if(!outf)
	return ajFalse;



    /* Initialise strings*/    
    token = ajStrNew();
    tmp   = ajStrNewC("");
    

    handle = ajStrTokenInit(str,delim);
    
    while(ajStrToken(&token,&handle,NULL))
    {
	if(!c)
	    ajFmtPrintF(outf,"%s",prefix);
	
	if((l=n+ajStrLen(token)) < len)
	{
	    if(c++)
		ajStrAppC(&tmp," ");
	    ajStrApp(&tmp,token);
	    n = ++l;
	}
	else
	{
	    ajFmtPrintF(outf,"%-*S\n",len+1, tmp);

	    ajStrAssS(&tmp,token);
	    ajStrAppC(&tmp," ");
	    n = ajStrLen(token);
	    c = 0;
	}
    }

    if(c)
    {
	ajFmtPrintF(outf,"%-*S\n",len+1, tmp);
    }
    

    ajStrTokenClear(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);
    
    return ajTrue;
}






/* @func ajXyzPrintPdbAtomDomain ************************************************
**
** Writes the ATOM records for a domain to an output file in pdb format using 
** data from a Pdb structure and a Scop structure
**
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
** @param [r] mod  [ajint] Model number, beginning at 1
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbAtomDomain(AjPFile errf, AjPFile outf, AjPPdb pdb, 
			    AjPScop scop, ajint mod)
{
    /*rn_mod is a modifier to the residue number to give correct residue 
      numbering for the domain*/

    ajint      acnt        =1;
    ajint      rn_mod      =0;  
    ajint      z;
    ajint      finalrn     =0;
    ajint      chn;
    char     id='\0';

    AjBool   found_start =ajFalse;
    AjBool   found_end   =ajFalse;
    AjBool   nostart     =ajFalse;
    AjBool   noend       =ajFalse;
    AjIList  iter        =NULL;
    AjPAtom  atm         =NULL;
    AjPAtom  atm2        =NULL;



    /* Loop for each chain in the domain*/
    for(z=0;z<scop->N;z++,found_start=ajFalse, found_end=ajFalse)
    {
	/* Check for chain error*/
	if(!ajXyzPdbChain(scop->Chain[z], pdb, &chn))
	    {
		ajListIterFree(iter);	
		ajWarn("Chain incompatibility error in ajXyzPrintPdbAtomDomain");		
		ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
			    "error in ajXyzPrintPdbAtomDomain\n", scop->Entry);
		return ajFalse;
	    }
	
	
	/* Iteratre up to the correct model*/
	iter=ajListIter(pdb->Chains[chn-1]->Atoms);	
	
	while((atm=(AjPAtom)ajListIterNext(iter)))
	    if(atm->Mod==mod)
		break;
	
	
	/*Increment res. counter from last chain if appropriate*/
	if(noend)
	    rn_mod += atm2->Idx;
	else	 
	    rn_mod += finalrn;


	/* Start of chain was not specified*/
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else 
	    nostart=ajFalse;
	
			    
	/* End of chain was not specified*/
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else
	    noend=ajFalse;
	

	/* If the domain was composed of more than once chain then a '.' is
	   given as the chain identifier*/
	if(scop->N > 1)
	    id = '.';
	else 
	    id = pdb->Chains[chn-1]->Id;


	  
	for(; atm; atm=(AjPAtom)ajListIterNext(iter)) 	
	{
	    /* Break if a non-protein atom is found or model no. is
	     incorrect */
	    if(atm->Mod!=mod || atm->Type!='P')
		break;


	    /* The start position was specified, but has not 
		       been found yet*/
	    if(!found_start && !nostart)
	    {
		/* Start position found */
		if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
		{
		    rn_mod -= atm->Idx-1;
		    found_start=ajTrue;	
		}
		else	
		    continue;
	    }	
	    

	    /* The end position was specified, but has not 
		       been found yet*/
	    if(!found_end && !noend)
	    {
		/* End position found */
		if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
		{
		    found_end=ajTrue;     
		    finalrn=atm->Idx;
		}
	    }	
	    else if(atm->Idx != finalrn && !noend)
		break;

	    
	    /* Write out ATOM line to pdb file*/
	    ajFmtPrintF(outf, "%-6s%5d  %-4S%-4S%c%4d%12.3f%8.3f"
			"%8.3f%6.2f%6.2f%11s%-3c\n", 
			"ATOM", 
			acnt++, 
			atm->Atm, 
			atm->Id3, 
			id,
			atm->Idx+rn_mod, 
			atm->X, 
			atm->Y, 
			atm->Z, 
			atm->O, 
			atm->B, 
			" ", 
			*ajStrStr(atm->Atm));

	    /* Assign pointer for this chain*/
	    atm2=atm;
	}
	
	
	/* Diagnostic if start was specified but not found*/
	if(!found_start && !nostart)
	    {
		ajListIterFree(iter);	
		ajWarn("Domain start not found in ajXyzPrintPdbAtomDomain");		
		ajFmtPrintF(errf, "//\n%S\nERROR Domain start not "
			    "found in ajXyzPrintPdbAtomDomain\n", scop->Entry);
		return ajFalse;
	    }
	

	/* Diagnostic if end was specified but not found*/
	if(!found_end && !noend)
	    {
		ajListIterFree(iter);	
		ajWarn("Domain end not found in ajXyzPrintPdbAtomDomain");		
		ajFmtPrintF(errf, "//\n%S\nERROR Domain end not "
			    "found in ajXyzPrintPdbAtomDomain\n", scop->Entry);
		return ajFalse;
	    }
	
	
	ajListIterFree(iter);	
    }
    

    /* Write the TER record to the pdb file*/
    ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
		"TER", 
		acnt++, 
		atm2->Id3, 
		id,	
		atm2->Idx+rn_mod, 
		" ");
    

    return ajTrue;
}








/* @func ajXyzPrintPdbAtomChain *************************************************
**
** Writes the ATOM records for a chain to an output file in pdb format using 
** data from a Pdb structure 
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] chn  [ajint] Chain number, beginning at 1
** @param [r] mod  [ajint] Model number, beginning at 1
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbAtomChain(AjPFile outf, AjPPdb pdb, ajint mod, ajint chn)
{
    AjBool   doneter=ajFalse;
    AjIList  iter=NULL;
    AjPAtom  atm=NULL;
    AjPAtom  atm2=NULL;
    ajint      acnt;
    

    /* Check args are not NULL */
    if(!outf || !pdb || mod<1 || chn<1)
	return ajFalse;
    

    doneter=ajFalse;
    iter=ajListIter(pdb->Chains[chn-1]->Atoms);	

    while((atm=(AjPAtom)ajListIterNext(iter)))
	if(atm->Mod==mod)
	    break;
  
    for(acnt=1; atm; atm=(AjPAtom)ajListIterNext(iter)) 	
    {
	/* Break if ont a new model*/
	if(atm->Mod!=mod)
	    break;
		
	
	/* End of protein atoms - so write a TER record*/
	if(atm->Type!='P' && (!doneter))
	{
	    ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
			"TER", 
			acnt++, 
			atm2->Id3, 
			pdb->Chains[chn-1]->Id, 
			atm2->Idx, 
			" ");
	    
	    doneter=ajTrue;
	}

	
	/* Write out ATOM or HETATM line*/
	if(atm->Type=='P')
	    ajFmtPrintF(outf, "%-6s", "ATOM");
	else
	    ajFmtPrintF(outf, "%-6s", "HETATM");

	ajFmtPrintF(outf, "%5d  %-4S%-4S%c%4d%12.3f%8.3f%8.3f"
		    "%6.2f%6.2f%11s%-3c\n", 
		    acnt++, 
		    atm->Atm, 
		    atm->Id3, 
		    pdb->Chains[chn-1]->Id, 
		    atm->Idx, 
		    atm->X, 
		    atm->Y, 
		    atm->Z, 
		    atm->O,
		    atm->B,
		    " ", 
		    *ajStrStr(atm->Atm));

	atm2=atm;
    }

    
    /* Write TER record if its not already done*/
    if(!doneter)
    {
	ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
		    "TER", 
		    acnt++, 
		    atm2->Id3, 
		    pdb->Chains[chn-1]->Id, 
		    atm2->Idx, 
		    " ");
	doneter=ajTrue;
    }
    ajListIterFree(iter);				    

    return ajTrue;
}









/* @func ajXyzPrintPdbSeqresDomain **********************************************
**
** Writes the SEQRES record for a domain to an output file in pdb format using 
** data from a Pdb structure and a Scop structure
**
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbSeqresDomain(AjPFile errf, AjPFile outf, AjPPdb pdb, 
			      AjPScop scop)
{
    ajint      last_rn=0;  
    ajint      this_rn;
    ajint      x;
    ajint      y;
    ajint      z;
    ajint      rcnt=0;
    ajint      len;
    ajint	     chn=-1;
    char    *p;
    char     id;

    AjPStr   tmp1=NULL;
    AjPStr   tmp2=NULL;
    AjBool   found_start=ajFalse;
    AjBool   found_end=ajFalse;
    AjBool   nostart=ajFalse;
    AjBool   noend=ajFalse;
    AjIList  iter=NULL;
    AjPAtom  atm=NULL;


    tmp1 = ajStrNew();
    tmp2 = ajStrNew();



    /* Loop for each chain in the domain*/
    for(z=0;z<scop->N;z++,found_start=ajFalse, found_end=ajFalse, 
	last_rn=0)
    {
	/*Check for error in chain id*/
	if(!ajXyzPdbChain(scop->Chain[z], pdb, &chn))
	    {
		ajListIterFree(iter);			
		ajStrDel(&tmp1);
		ajStrDel(&tmp2);
		ajWarn("Chain incompatibility error in "
		       "ajXyzPrintPdbSeqresDomain");		

		ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility "
			    "error in ajXyzPrintPdbSeqresDomain\n", 
			    scop->Entry);

		return ajFalse;
	    }


	/* Intitialise iterator for list of atoms*/
	iter=ajListIter(pdb->Chains[chn-1]->Atoms);	
	

	/* Start of chain not specified*/
	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else
	    nostart=ajFalse;

	
	/* End of chain not specified*/
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else	
	    noend=ajFalse;
	

	/* Iterate through list of atoms*/
	while((atm=(AjPAtom)ajListIterNext(iter)))
	{
	    /* JCI hard-coded to work on model 1*/	
	    /* Break if a non-protein atom is found or model no. !=1*/
	    if(atm->Type!='P' || atm->Mod!=1)
		break;
	
	    
	    /* If we are onto a new residue*/
	    this_rn=atm->Idx;
	    if(this_rn!=last_rn)
	    {	
		/* The start position was specified, but has not 
		   been found yet*/
		if(!found_start && !nostart)
		{
		    /* Start position found */
		    if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
		    {
			last_rn=this_rn;
			found_start=ajTrue;	
		    }
		    else	
		    {
			last_rn=this_rn;
			continue;
		    }
		    
		}
	    

		/*Assign sequence for residues missing from the linked list*/
		/*of atoms of known structure*/
		for(x=last_rn; x<this_rn-1; x++)
		{	
		    /* Check that position x is in range for the sequence*/
		    if(!ajBaseAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), 
				   &tmp2))
		    {
			ajListIterFree(iter);			
			ajStrDel(&tmp1);
			ajStrDel(&tmp2);
			ajWarn("Index out of range in "
			       "ajXyzPrintPdbSeqresDomain");		
			ajFmtPrintF(errf, "//\n%S\nERROR Index out of range "
				    "in ajXyzPrintPdbSeqresDomain\n", scop->Entry);
			return ajFalse;
		    }
		    else
		    {	
			ajStrApp(&tmp1, tmp2);
			ajStrAppC(&tmp1, " ");
			rcnt++;
		    }	
		}

		last_rn=this_rn;

		
		/* Append the residue to the sequence*/
		ajStrApp(&tmp1, atm->Id3);
		ajStrAppC(&tmp1, " ");
		rcnt++;
		

		/* The end position was specified, but has not 
		       been found yet*/
		if(!found_end && !noend)
		{
		    /* End found*/
		    if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
		    {
			found_end=ajTrue;       
			break;
		    }
		}	
	    }
	}
	
	
	/*Domain start specified but not found*/
	if(!found_start && !nostart)
	{
	    ajListIterFree(iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajWarn("Domain start not found in ajXyzPrintPdbSeqresDomain");		
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain start not found "
			"in ajXyzPrintPdbSeqresDomain\n", scop->Entry);
	    return ajFalse;
	}
	

	/*Domain end specified but not found*/
	if(!found_end && !noend)
	{
	    ajListIterFree(iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajWarn("Domain end not found in ajXyzPrintPdbSeqresDomain");		
	    ajFmtPrintF(errf, "//\n%S\nERROR Domain end not found "
			"in ajXyzPrintPdbSeqresDomain\n", scop->Entry);
	    return ajFalse;
	}


	/*Assign sequence for residues missing from end of linked list*/
	/*Only needs to be done where the end of the domain is not specified*/
	if(noend)
	{	    
	    for(x=last_rn; x<pdb->Chains[chn-1]->Nres; x++)    
		if(!ajBaseAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), 
			       &tmp2))
		{	 
		    ajStrDel(&tmp1);
		    ajStrDel(&tmp2);
		    ajListIterFree(iter);	
		    ajWarn("Index out of range in ajXyzPrintPdbSeqresDomain");		
		    ajFmtPrintF(errf, "//\n%S\nERROR Index out of "
				"range in ajXyzPrintPdbSeqresDomain\n", 
				scop->Entry);
		    return ajFalse;
		}
		else
		{
		    ajStrApp(&tmp1, tmp2);
		    ajStrAppC(&tmp1, " ");
		    rcnt++;
		}	
	}


	/*Free iterator*/
	ajListIterFree(iter);	 		
    }
    

    /* If the domain was composed of more than once chain then a '.' is
       given as the chain identifier*/
    if(scop->N > 1)
	id = '.';
    else 
	id = pdb->Chains[chn-1]->Id;
       
 
    /*Print out SEQRES records*/
    for(p=ajStrStr(tmp1), len=ajStrLen(tmp1), x=0, y=1; 
	x<len; 
	x+=52, y++, p+=52)
	ajFmtPrintF(outf, "SEQRES%4d %c%5d  %-61.52s\n", 
		    y, 
		    id, 
		    rcnt,
		    p);


    /* Tidy up*/
    ajListIterFree(iter);			
    ajStrDel(&tmp1);
    ajStrDel(&tmp2);

    return ajTrue;
}

       





/* @func ajXyzPrintPdbSeqresChain ***********************************************
**
** Writes the SEQRES record for a chain to an output file in pdb format using 
** data from a Pdb structure
**
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] chn  [ajint] chain number, beginning at 1
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbSeqresChain(AjPFile errf, AjPFile outf, AjPPdb pdb, 
			     ajint chn)
{
    ajint      last_rn =0;  
    ajint      this_rn;
    ajint      x;
    ajint      y;
    ajint      len;
    char    *p;

    AjPStr   tmp1    =NULL;
    AjPStr   tmp2    =NULL;
    AjIList  iter    =NULL;
    AjPAtom  atm     =NULL;

    tmp1 = ajStrNew();
    tmp2 = ajStrNew();


    iter=ajListIter(pdb->Chains[chn-1]->Atoms);	


    /* Iterate through list of atoms*/
    while((atm=(AjPAtom)ajListIterNext(iter)))
    {
	/* JCI hard-coded to work on model 1*/	
	/* Break if a non-protein atom is found or model no. !=1*/
	if(atm->Type!='P' || atm->Mod!=1)
	    break;
	

	/* If we are onto a new residue*/
	this_rn=atm->Idx;
	if(this_rn!=last_rn)
	{
	    /*Assign sequence for residues missing from the linked list*/
	    for(x=last_rn; x<this_rn-1; x++)
	    {	
		/* Check that position x is in range for the sequence*/
		if(!ajBaseAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), 
				&tmp2))
		    {
			ajWarn("Index out of range in ajXyzPrintPdbSeqresChain");		

			ajFmtPrintF(errf, "//\n%S\nERROR Index out "
				    "of range in ajXyzPrintPdbSeqresChain\n", 
				    pdb->Pdb);

			ajStrDel(&tmp1);
			ajStrDel(&tmp2);
			ajListIterFree(iter);	

			return ajFalse;
		    }
		
		else
		{
		    ajStrApp(&tmp1, tmp2);
		    ajStrAppC(&tmp1, " ");
		}	
	    }
	    ajStrApp(&tmp1, atm->Id3);
	    ajStrAppC(&tmp1, " ");

	    last_rn=this_rn;
	}
    }

    
    /*Assign sequence for residues missing from end of linked list*/
    for(x=last_rn; x<pdb->Chains[chn-1]->Nres; x++)
	if(!ajBaseAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), &tmp2))
	    { 
		ajStrDel(&tmp1);
		ajStrDel(&tmp2);
		ajListIterFree(iter);	
		ajWarn("Index out of range in ajXyzPrintPdbSeqresChain");		
		ajFmtPrintF(errf, "//\n%S\nERROR Index out of range "
			    "in ajXyzPrintPdbSeqresChain\n", pdb->Pdb);
		return ajFalse;
	    }
    
	else
	{
	    ajStrApp(&tmp1, tmp2);
	    ajStrAppC(&tmp1, " ");
	}	

    
    /*Print out SEQRES records*/
    for(p=ajStrStr(tmp1), len=ajStrLen(tmp1), x=0, y=1; 
	x<len; 
	x+=52, y++, p+=52)
	ajFmtPrintF(outf, "SEQRES%4d %c%5d  %-61.52s\n", 
		    y, 
		    pdb->Chains[chn-1]->Id, 
		    pdb->Chains[chn-1]->Nres, 
		    p);


    /* Tidy up*/
    ajStrDel(&tmp1);
    ajStrDel(&tmp2);
    ajListIterFree(iter);	

    return ajTrue;
}





/* @func ajXyzPrintPdbResolution ************************************************
**
** Writes the Reso element of a Pdb structure to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbResolution(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11sRESOLUTION. %-6.2f%-51s\n", 
		    "REMARK", pdb->Reso, "ANGSTROMS.");
	return ajTrue;
    }
    else
	return ajFalse;
}





/* @func ajXyzPrintPdbEmptyRemark ***********************************************
**
** Writes an empty REMARK record to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbEmptyRemark(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11s%-69s\n", "REMARK", " ");
	return ajTrue;
    }
    else
	return ajFalse;
}






/* @func ajXyzPrintPdbSource ****************************************************
**
** Writes the Source element of a Pdb structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbSource(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajXyzPrintPdbText(outf,pdb->Source,"SOURCE     ", 68," \t\r\n");
	return ajTrue;
    }
    else
	return ajFalse;
}





/* @func ajXyzPrintPdbCompnd ****************************************************
**
** Writes the Compnd element of a Pdb structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbCompnd(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajXyzPrintPdbText(outf,pdb->Compnd,"COMPND     ", 
		       68," \t\r\n");
	return ajTrue;
    }
    else
	return ajFalse;
}





/* @func ajXyzPrintPdbTitle ****************************************************
**
** Writes a TITLE record to an output file in pdb format
** The text is hard-coded.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbTitle(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11sTHIS FILE IS MISSING MOST RECORDS FROM THE "
		    "ORIGINAL PDB FILE%9s\n", 
		    "TITLE", " ");
	return ajTrue;
    }
    else
	return ajFalse;
}







/* @func ajXyzPrintPdbHeader ****************************************************
**
** Writes the Pdb element of a Pdb structure to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbHeader(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11sCLEANED-UP PDB FILE FOR %-45S\n", 
		    "HEADER", 
		    pdb->Pdb);    
	return ajTrue;
    }
    else
	return ajFalse;
}





/* @func ajXyzPrintPdbHeaderScop ************************************************
**
** Writes the Entry element of a Scop structure to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajXyzPrintPdbHeaderScop(AjPFile outf, AjPScop scop)
{
    if(scop && outf)
    {
	ajFmtPrintF(outf, "%-11sCLEANED-UP PDB FILE FOR SCOP DOMAIN %-33S\n", 
		    "HEADER", 
		    scop->Entry);    
	return ajTrue;
    }
    else
	return ajFalse;
}




/* @func ajXyzPdbWriteDomain ****************************************************
**
** Calls functions to write domain-specific contents of a Pdb object to an 
** output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool   ajXyzPdbWriteDomain(AjPFile errf, AjPFile outf, AjPPdb pdb, AjPScop scop)
{
    ajint z;     /* A counter */
    ajint chn;   /* No. of the chain in the pdb structure */


    /* Check for errors in chain identifier and length*/
    for(z=0;z<scop->N;z++)
	if(!ajXyzPdbChain(scop->Chain[z], pdb, &chn))
	{
	    ajWarn("Chain incompatibility error in "
		   "ajXyzPdbWriteDomain");			
		
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain incompatibility error "
			"in ajXyzPdbWriteDomain\n", scop->Entry);
		
	    return ajFalse;
	}
	else if(pdb->Chains[chn-1]->Nres==0)
	{		
	    ajWarn("Chain length zero");			
	    
	    ajFmtPrintF(errf, "//\n%S\nERROR Chain length zero\n", scop->Entry);
	    
	    return ajFalse;
	}
    

    /* Write bibliographic info.*/
    ajXyzPrintPdbHeaderScop(outf, scop);
    ajXyzPrintPdbTitle(outf, pdb);
    ajXyzPrintPdbCompnd(outf, pdb);
    ajXyzPrintPdbSource(outf, pdb);
    ajXyzPrintPdbEmptyRemark(outf, pdb);
    ajXyzPrintPdbResolution(outf, pdb);
    ajXyzPrintPdbEmptyRemark(outf, pdb);
    

    /* Write SEQRES records*/
    if(!ajXyzPrintPdbSeqresDomain(errf, outf, pdb, scop))
    {
	ajWarn("Error writing file in ajXyzPdbWriteDomain"); 
	return ajFalse;
    } 


    /* Write MODEL record, if appropriate*/
    if(pdb->Method == ajNMR)
	ajFmtPrintF(outf, "MODEL%9d%66s\n", 1, " ");


    /* Write ATOM/HETATM records*/
    if(!ajXyzPrintPdbAtomDomain(errf, outf, pdb, scop, 1))
    {
	ajWarn("Error writing file in ajXyzPdbWriteDomain"); 
	return ajFalse;
    }  

    
    /* Write END/ENDMDL records*/
    if(pdb->Method == ajNMR)
	ajFmtPrintF(outf, "%-80s\n", "ENDMDL");

    ajFmtPrintF(outf, "%-80s\n", "END");

    return ajTrue;
}








/* @func ajXyzPdbWriteAll *******************************************************
**
** Calls functions to write contents of a Pdb object to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [w] errf [AjPFile] Output file stream for error messages
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool   ajXyzPdbWriteAll(AjPFile errf, AjPFile outf, AjPPdb pdb)
{
    ajint x;
    ajint y;
    

    /* Write bibliographic info.*/
    ajXyzPrintPdbHeader(outf, pdb);
    ajXyzPrintPdbTitle(outf, pdb);
    ajXyzPrintPdbCompnd(outf, pdb);
    ajXyzPrintPdbSource(outf, pdb);
    ajXyzPrintPdbEmptyRemark(outf, pdb);
    ajXyzPrintPdbResolution(outf, pdb);
    ajXyzPrintPdbEmptyRemark(outf, pdb);
    
    
    /* Write SEQRES records*/
    for(x=0;x<pdb->Nchn;x++)
	if(!ajXyzPrintPdbSeqresChain(errf, outf, pdb, x+1))
	{
	    ajWarn("Error writing file in ajXyzPdbWriteAll"); 
	    return ajFalse;
	}

    
    /* Loop for each model */
    for(y=0;y<pdb->Nmod;y++)
    {
	/* Write the MODEL record*/
	if(pdb->Method == ajNMR)
	    ajFmtPrintF(outf, "MODEL%9d%66s\n", y+1, " ");

	
	/* Write ATOM/HETATM records*/
	for(x=0;x<pdb->Nchn;x++)
	    if(!ajXyzPrintPdbAtomChain(outf, pdb, y+1, x+1))
	    {
		ajWarn("Error writing file in ajXyzPdbWriteAll"); 
		return ajFalse;
	    }
	

	/* Write ENDMDL record*/
	if(pdb->Method == ajNMR)
	    ajFmtPrintF(outf, "%-80s\n", "ENDMDL");
    }

    
    /* Write END record*/
    ajFmtPrintF(outf, "%-80s\n", "END");

    return ajTrue;
}







/* @func ajXyzScopWrite ******************************************************
**
** Write contents of a Scop object to an output file
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] thys [AjPScop] Scop object
**
** @return [void]
** @@
******************************************************************************/

void ajXyzScopWrite(AjPFile outf, AjPScop thys)
{
    ajint i;

    ajFmtPrintF(outf,"ID   %S\nXX\n",thys->Entry);
    ajFmtPrintF(outf,"EN   %S\nXX\n",thys->Pdb);
    ajFmtPrintF(outf,"OS   %S\nXX\n",thys->Source);
    ajFmtPrintF(outf,"CL   %S",thys->Class);

    ajFmtPrintSplit(outf,thys->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Domain,"XX\nDO   ",75," \t\n\r");;
    
    ajFmtPrintF(outf,"XX\nNC   %d\n",thys->N);

    for(i=0;i<thys->N;++i)
    {
	ajFmtPrintF(outf,"XX\nCN   [%d]\n",i+1);
	ajFmtPrintF(outf,"XX\nCH   %c CHAIN; %S START; %S END;\n",
		    thys->Chain[i],
		    thys->Start[i],
		    thys->End[i]);
    }
    ajFmtPrintF(outf,"//\n");
    
    return;
}






/* @func ajXyzScopRead *********************************************************
**
** Read a Scop object from a file
**
** @param [r] inf [AjPFile] Output file stream
** @param [r] entry [AjPStr] id
** @param [w] thys [AjPScop*] Scop object
**
** @return [void]
** @@
******************************************************************************/

AjBool ajXyzScopRead(AjPFile inf, AjPStr entry, AjPScop *thys)
{
    return ajXyzScopReadC(inf,ajStrStr(entry),thys);
}






/* @func ajXyzScopReadC ******************************************************
**
** Read a Scop object from a file
**
** @param [r] inf [AjPFile] Input file stream
** @param [r] entry [char*] id
** @param [w] thys [AjPScop*] Scop object
**
** @return [void]
** @@
******************************************************************************/

AjBool ajXyzScopReadC(AjPFile inf, char *entry, AjPScop *thys)
{
    static AjPRegexp exp1 =NULL;
    static AjPRegexp exp2 =NULL;
    static AjPStr line    =NULL;
    static AjPStr str     =NULL;
    static AjPStr xentry  =NULL;
    static AjPStr source  =NULL;
    static AjPStr class   =NULL;
    static AjPStr fold    =NULL;
    static AjPStr super   =NULL;
    static AjPStr family  =NULL;
    static AjPStr domain  =NULL;
    static AjPStr pdb     =NULL;
    static AjPStr tentry  =NULL;
    static AjPStr stmp    =NULL;
    
    AjBool ok             =ajFalse;
    
    char   *p;
    ajint    idx            =0;
    ajint    n=0;
    

    /* Only initialise strings if this is called for the first time*/
    if(!line)
    {
	str     = ajStrNew();
	xentry  = ajStrNew();
	pdb     = ajStrNew();
	source  = ajStrNew();
	class   = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	domain  = ajStrNew();
	line    = ajStrNew();
	tentry  = ajStrNew();
	stmp    = ajStrNew();
	exp1    = ajRegCompC("^([^ \t\r\n]+)[ \t\n\r]+");
	exp2    = ajRegCompC("^([A-Za-z0-9.]+)[ ]*[^ \t\r\n]+[ ]*([0-9.-]+)[ ]*"
			     "[^ \t\r\n]+[ ]*([0-9.-]+)");
    }


    
    ajStrAssC(&tentry,entry);
    ajStrToUpper(&tentry);
    
    while((ok=ajFileReadLine(inf,&line)))
    {
	if(!ajStrPrefixC(line,"ID   "))
	    continue;
	
	if(!ajRegExec(exp1,line))
	    return ajFalse;
	ajRegPost(exp1,&stmp);
	if(ajStrMatchWild(stmp,tentry))
	    break;
    }

    
    if(!ok)
	return ajFalse;
    
    
    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	ajRegExec(exp1,line);
	ajRegPost(exp1,&str);

	if(ajStrPrefixC(line,"ID"))
	    ajStrAssS(&xentry,str);
	else if(ajStrPrefixC(line,"EN"))
	    ajStrAssS(&pdb,str);
	else if(ajStrPrefixC(line,"OS"))
	    ajStrAssS(&source,str);
	else if(ajStrPrefixC(line,"CL"))
	    ajStrAssS(&class,str);
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssS(&fold,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssS(&super,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&super,ajStrStr(line)+3);
	    }
	    ajStrClean(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssS(&family,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"DO"))
	{
	    ajStrAssS(&domain,str);
	    while(ajFileReadLine(inf,&line))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&domain,ajStrStr(line)+3);
	    }
	    ajStrClean(&domain);
	}
	else if(ajStrPrefixC(line,"NC"))
	{
	    ajStrToInt(str,&n);
	    (*thys) = ajXyzScopNew(n);
	    ajStrAssS(&(*thys)->Entry,xentry);
	    ajStrAssS(&(*thys)->Pdb,pdb);
	    ajStrAssS(&(*thys)->Source,source);
	    ajStrAssS(&(*thys)->Class,class);
	    ajStrAssS(&(*thys)->Fold,fold);
	    ajStrAssS(&(*thys)->Domain,domain);
	    ajStrAssS(&(*thys)->Superfamily,super);
	    ajStrAssS(&(*thys)->Family,family);
	    
	}
	else if(ajStrPrefixC(line,"CN"))
	{
	    p = ajStrStr(str);
	    sscanf(p,"[%d]",&idx);
	}
	else if(ajStrPrefixC(line,"CH"))
	{
	    if(!ajRegExec(exp2,str))
		return ajFalse;
	    ajRegSubI(exp2,1,&stmp);
	    (*thys)->Chain[idx-1] = *ajStrStr(stmp);
	    ajRegSubI(exp2,2,&str);
	    ajStrAssC(&(*thys)->Start[idx-1],ajStrStr(str)); 

	    ajRegSubI(exp2,3,&str);
	    ajStrAssC(&(*thys)->End[idx-1],ajStrStr(str)); 

	}
	ok = ajFileReadLine(inf,&line);
    }
    
    
    return ajTrue;
}



/* @func ajXyzScopToPdb *********************************************************
**
** Read a scop identifier code and writes the equivalent pdb identifier code
**
** @param [r] scop [AjPStr]   Scop identifier code
** @param [w] pdb  [AjPStr*]  Pdb identifier code
**
** @return [void]
** @@
******************************************************************************/
void   ajXyzScopToPdb(AjPStr scop, AjPStr *pdb)
{
    ajStrAssSub(pdb, scop, 1, 4);
}




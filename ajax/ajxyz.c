/********************************************************************
** @source AJAX structure functions
**
** @author Copyright (C) 2000 Jon Ison
** @version 1.0 
** @modified Nov 08 jci First version
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

/* @func ajPdbNew ************************************************************
**
** Pdb object constructor. Fore-knowledge of the number of chains 
** is required. This is normally called by the Cpdb reading
** routine.
**
** @param [r] chains [int] Number of chains in this pdb entry
**
** @return [AjPPdb] Pointer to a pdb object
** @@
******************************************************************************/

AjPPdb ajPdbNew(int chains)
{
    AjPPdb ret=NULL;
    int i;
    
    AJNEW0(ret);
  

    ret->Pdb = ajStrNew();
    ret->Compnd = ajStrNew();
    ret->Source = ajStrNew();

    AJCNEW0(ret->Chains,chains);
    for(i=0;i<chains;++i)
	ret->Chains[i] = ajChainNew();
    
    return ret;
}

/* @func ajChainNew ***********************************************************
**
** Chain object constructor. 
** This is normally called by the ajPdbNew function
**
** @return [AjPChain] Pointer to a chain object
** @@
******************************************************************************/

AjPChain ajChainNew(void)
{
  AjPChain ret=NULL;
  
  AJNEW0(ret);

  ret->Seq    = ajStrNewC("");
  ret->Atoms  = ajListNew();

  return ret;
}

/* @func ajAtomNew ***********************************************************
**
** Constructor for atom co-ordinates.
** This is normally called by the ajChainNew function
**
** @return [AjPAtom] Pointer to an atom object
** @@
******************************************************************************/

AjPAtom ajAtomNew(void)
{
    AjPAtom ret=NULL;

    AJNEW0(ret);
    
    ret->Id3 = ajStrNew();
    ret->Atm = ajStrNew();
    ret->Pdb = ajStrNew();

    return ret;
}

/* @func ajScopNew ***********************************************************
**
** Scop object constructor. Fore-knowledge of the number of chains is 
** required.
**
** @param [r] chains [int] Number of chains
**
** @return [AjPScop] Pointer to a scop object
** @@
******************************************************************************/

AjPScop ajScopNew(int chains)
{

    AjPScop ret = NULL;
    int i;

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



/* @func ajPdbDel ***********************************************************
**
** Destructor for pdb object.
**
** @param [w] thys [AjPPdb*] Pdb object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajPdbDel(AjPPdb *thys)
{
    AjPPdb pthis = *thys;
    
    int nc=0;
    int i=0;

    if(!pthis || !thys)
	return;
    
    nc = pthis->Nchn;

    ajStrDel(&pthis->Pdb);
    ajStrDel(&pthis->Compnd);
    ajStrDel(&pthis->Source);
    
    for(i=0;i<nc;++i)
	ajChainDel(&pthis->Chains[i]);
    AJFREE(pthis);

    return;
}


/* @func ajChainDel ***********************************************************
**
** Destructor for chain object.
**
** @param [w] thys [AjPChain*] Chain object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajChainDel(AjPChain *thys)
{
    AjPChain pthis = *thys;
    AjPAtom atm=NULL;

    if(!thys || !pthis)
	return;
    
    while(ajListPop(pthis->Atoms,(void **)&atm))
	ajAtomDel(&atm);

    ajListDel(&pthis->Atoms);

    AJFREE(pthis);

    return;
}

/* @func ajAtomDel ***********************************************************
**
** Destructor for atom co-ordinates.
**
** @param [w] thys [AjPAtom*] Atom object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajAtomDel(AjPAtom *thys)
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

/* @func ajScopDel ***********************************************************
**
** Destructor for scop object.
**
** @param [w] thys [AjPScop*] Atom object pointer
**
** @return [void]
** @@
******************************************************************************/

void ajScopDel(AjPScop *thys)
{
    AjPScop pthis = *thys;
    
    int i;

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

/* @func ajCpdbRead ***********************************************************
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

AjBool ajCpdbRead(AjPFile inf, AjPPdb *thys)
{
/*    AjPFile   inf=NULL;
    AjPStr    fn=NULL;  */
    AjPStr    line=NULL;
    AjPStr    token=NULL;
    AjPStr    idstr=NULL;
    AjPStr    destr=NULL;
    AjPStr    osstr=NULL;
    AjPStr     xstr=NULL;
    AjPStrTok handle=NULL;
    
    AjPAtom atom=NULL;
    
    float reso=0.0;
    int   nmod=0;
    int   ncha=0;
    int   nc=0;
    int   mod=0;
    int   chn=0;
    int   last_chn=0;
    int   last_mod=0;
    int   done_first_co_line=0;
    

    /*
    fn = ajStrNewC(ajStrStr(name));
    ajFileDataNew(fn,&inf);
    if(!inf)
    {
	(void) ajStrAssC(&fn,"CPDB/");
	(void) ajStrAppC(&fn,ajStrStr(name));
	ajFileDataNew(fn,&inf);
	if(!inf)
	{
	    ajStrDel(&fn);
	    return ajFalse;
	}
    }
    */
 

    line  = ajStrNew();
    token = ajStrNew();
    
    idstr = ajStrNew();
    destr = ajStrNew();
    osstr = ajStrNew();
    xstr  = ajStrNew();


    while(ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"XX"))
	    continue;
	
	if(ajStrPrefixC(line,"ID"))
	{
	    ajStrTokenAss(&handle,line," \n\t\r");
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&idstr,&handle,NULL);
	    continue;
	}

	if(ajStrPrefixC(line,"CN"))
	{
	    ajStrTokenAss(&handle,line," []\n\t\r");
	    ajStrToken(&token,&handle,NULL);
	    ajStrToken(&token,&handle,NULL);
	    ajStrToInt(token,&nc);
	    continue;
	}
	
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

	    *thys = ajPdbNew(ncha);

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
  



	if(ajStrPrefixC(line,"SQ"))
	{
	    while(ajFileReadLine(inf,&line) && !ajStrPrefixC(line,"XX"))
		ajStrAppC(&(*thys)->Chains[nc-1]->Seq,ajStrStr(line));
	    ajStrCleanWhite(&(*thys)->Chains[nc-1]->Seq);
	    continue;
	}
    
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


	    if(done_first_co_line == 0)
	    {
		last_chn=chn;
		last_mod=mod;
		done_first_co_line=1;
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
    
    ajStrTokenClear(&handle);

    ajStrDel(&line);
    ajStrDel(&token);
    ajStrDel(&idstr);
    ajStrDel(&destr);
    ajStrDel(&osstr);
    ajStrDel(&xstr);

/*    ajFileClose(&inf); */

    return ajTrue;
}







/* @func ajCpdbWriteDomain ***************************************************
**
** Write domain-specific contents of a Pdb object to an output file in cpdb 
** format
** Hard-coded to write data for model 1
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on success
** @@
** 
******************************************************************************/
AjBool ajCpdbWriteDomain(AjPFile outf, AjPPdb pdb, AjPScop scop)
{
    /*rn_mod is a modifier to the residue number to give correct residue
      numbering for the domain*/
    int      z;
    int      chn;
    int      start=0;
    int      end=0;
    int      finalrn=0;
    int      rn_mod=0;  
    char     id;
    int      last_rn=0;  
    int      this_rn;
    AjIList  iter=NULL;
    AjPAtom  atm=NULL;
    AjPAtom  atm2=NULL;
    AjPStr   tmpseq=NULL;   
    AjPStr   seq=NULL;   
    AjBool   found_start=ajFalse;
    AjBool   found_end=ajFalse;
    AjBool   nostart=ajFalse;
    AjBool   noend=ajFalse;

    seq=ajStrNew();
    tmpseq=ajStrNew();
    
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
    ajFmtPrintF(outf, "RESO %.2f; NMOD %d; NCHA %d;\n", pdb->Reso,
		pdb->Nmod, pdb->Nchn);

    /*Start of code for printing out data up to co-ordinates list*/
    for(z=0;
	z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse, 
	nostart=ajFalse, noend=ajFalse, last_rn=0)
    {	
	if(!ajPdbChain(scop->Chain[z], pdb, &chn))
	{
		ajStrDel(&seq);
		ajStrDel(&tmpseq);
		ajWarn("Chain incompatibility error in ajCpdbWriteDomain");	
		return ajFalse;
	    }
	

	iter=ajListIter(pdb->Chains[chn-1]->Atoms);


	/*JC
	  while(ajListIterMore(iter))	
	  {
	  atm=(AjPAtom)ajListIterNext(iter);
	  */


	/*If start of domain is unspecified 
	  then assign start to first residue in chain*/
	if(!ajStrCmpC(scop->Start[z], "."))
	{
	    nostart = ajTrue;

	    atm=(AjPAtom)ajListIterNext(iter);
	    if(atm->Type=='P' && atm->Mod==1)
	    {
		start=atm->Idx;
		found_start=ajTrue;	
		ajListIterFree(iter);	
		iter=ajListIter(pdb->Chains[chn-1]->Atoms);
	    }
	}

	/*If end of domain is unspecified 
	  then assign end to last residue in chain*/
	if(!ajStrCmpC(scop->End[z], "."))
	{
	    noend = ajTrue;
	    
	    while((atm=(AjPAtom)ajListIterNext(iter)))
	    {
		if(atm->Type!='P' || atm->Mod!=1)
		    break;	

		atm2=atm;
	    }
	    if(atm2)
	    {
		end=atm2->Idx;
		found_end=ajTrue;	
		ajListIterFree(iter);	
		iter=ajListIter(pdb->Chains[chn-1]->Atoms);
	    }
	}
	

	/*Find start and end of domains in chain*/
	if(!found_start || !found_end)
	{
	    while((atm=(AjPAtom)ajListIterNext(iter)))
	    {

		if(atm->Type!='P' || atm->Mod!=1 
		   || (found_start && found_end))
		    break;

		this_rn=atm->Idx;

		if(this_rn!=last_rn)
		{
		    last_rn=this_rn;

		    if(!found_start && !nostart)		
		    {
			if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
			{
			    start=atm->Idx;
			    found_start=ajTrue;	
			}
			else	
			    continue;
		    }

		    if(!found_end && !noend)		
		    {
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
	
	if(!found_start)		
	{
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajListIterFree(iter);	
	    ajWarn("Domain start not found in ajCpdbWriteDomain");
	    return ajFalse;
	}
	
	if(!found_end)		
	    {
		ajStrDel(&seq);
		ajStrDel(&tmpseq);
		ajListIterFree(iter);	
		ajWarn("Domain end not found in ajCpdbWriteDomain");
		return ajFalse;
	    }
	

	/*Write <seq> string here */
	ajStrAssSub(&tmpseq, pdb->Chains[chn-1]->Seq, start-1, end-1);
	ajStrApp(&seq, tmpseq);

	ajListIterFree(iter);	
    }
    
    
    
    if(scop->N > 1)
	id = '.';
    else
	id = pdb->Chains[chn-1]->Id;
    
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

    /*Start of code for printing out co-ordinates list*/        
    for(nostart=ajFalse, noend=ajFalse, 
	z=0;z<scop->N;
	z++,found_start=ajFalse, found_end=ajFalse)
    {
	if(!ajPdbChain(scop->Chain[z], pdb, &chn))
	{
	    ajStrDel(&seq);
	    ajStrDel(&tmpseq);
	    ajListIterFree(iter);	
	    ajWarn("Chain incompatibility error in ajCpdbWriteDomain");
	    return ajFalse;
	}

	iter=ajListIter(pdb->Chains[chn-1]->Atoms);

	/*DEBUG
	  while(ajListIterMore(iter))	
	  {
	  atm=(AjPAtom)ajListIterNext(iter);
	  */

	/*Increment res. counter from last chain if appropriate*/
	if(noend)
	    rn_mod += atm2->Idx;
	else	 
	    rn_mod += finalrn;

	/*Check whether start and end of domain are specified*/
	if(!ajStrCmpC(scop->Start[z], "."))
	{
	    nostart = ajTrue;
	    /*Increment res. counter for the start of this chain*/
	    rn_mod -= atm->Idx-1;
	}
	else
	    nostart=ajFalse;
	
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else 
	    noend=ajFalse;
	
	while((atm=(AjPAtom)ajListIterNext(iter)))
	{
	    if(atm->Mod!=1 || atm->Type!='P')
		break;
	    
	    if(!found_start)
	    {
		if(!nostart)
		{
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
		    rn_mod -= atm->Idx-1;
		    found_start=ajTrue;	
		}
	    }	
	    
	    if(!found_end && !noend)
	    {
		if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
		{
		    found_end=ajTrue;     
		    finalrn=atm->Idx;
		}
	    }	
	    else if(atm->Idx != finalrn && !noend)
		break;
	    
	    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-6d%-6S%-2c%6S    %-4S"
			"%8.3f%9.3f%9.3f%9.2f%9.2f\n", 
			"CO", 
			atm->Mod, 
			1,	/*chn number is always given as 1*/
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

	    atm2=atm;
	}
	ajListIterFree(iter);			
    } 	
    
    ajFmtPrintF(outf, "//\n");    
    
    ajListIterFree(iter);	    
    ajStrDel(&seq);
    ajStrDel(&tmpseq);
    
    return ajTrue;
}





/* @func ajCpdbWriteAll ******************************************************
**
** Write contents of a Pdb object to an output file in cpdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] thys [AjPPdb] Pdb object
**
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajCpdbWriteAll(AjPFile outf, AjPPdb thys)
{
    int      x;
    int      y;
    AjIList  iter=NULL;
    AjPAtom  tmp=NULL;
    

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
		    ajFmtPrintF(outf, "%-5s%-5d%-5d%-5c%-6d%-6S%-2c%6S    %-4S"
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

    return ajTrue;
}







/* @func ajPdbChain **********************************************************
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
AjBool ajPdbChain(char id, AjPPdb pdb, int *chn)
{
    int a;
    
    for(a=0;a<pdb->Nchn;a++)
	if(toupper(pdb->Chains[a]->Id) == toupper(id))
	{
	    *chn=a+1;
	    return ajTrue;
	}

    if(id=='.')
    {
	*chn=1;
	return ajTrue;
    }
    
	
    return ajFalse;
}





/* @func ajAa1ToAa3 **********************************************************
**
** Writes an AjPStr with a amino acid 3 letter code
** NOTE - This should probably be an emb function and might replace the use 
** of embPropCharToThree &  embPropIntToThree
**
** @param [w] aa3  [AjPStr *] AjPStr object
** @param [r] char [aa1]    Single letter identifier of amino acid
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajAa1ToAa3(char aa1, AjPStr *aa3)
{
    int idx;
    
    static char *tab[]=
    {
	"ALA\0","ASX\0","CYS\0","ASP\0","GLU\0","PHE\0","GLY\0","HIS\0",
	"ILE\0","---\0","LYS\0","LEU\0","MET\0","ASN\0","---\0","PRO\0",
	"GLN\0","ARG\0","SER\0","THR\0","---\0","VAL\0","TRP\0","XAA\0",
	"TYR\0","GLX\0" 
    };

    if((idx=ajAZToInt(aa1))==27)
	return ajFalse;

    ajStrAssC(aa3, tab[idx]);
    return ajTrue;
}






/* @func ajPrintPdbText ******************************************************
**
** Writes pdb-formatted text to an output file
**
** @param [w] outf   [AjPFile] Output file stream
** @param [r] str    [AjPStr]  Text to print out
** @param [r] prefix [char *]  Text to print out at start of line
** @param [r] len    [int]     Width of record to print into
** @param [r] delim  [char *]  String for tokenization of text
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajPrintPdbText(AjPFile outf, AjPStr str, char *prefix, int len, 
		       char *delim)
{
    AjPStrTok handle=NULL;
    AjPStr token = NULL;
    AjPStr tmp   = NULL;
    int    n = 0;
    int    l = 0;
    int    c = 0;
    
    if(!outf)
	return ajFalse;
    
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






/* @func ajPrintPdbAtomDomain ************************************************
**
** Writes the ATOM records for a domain to an output file in pdb format using 
** data from a Pdb structure and a Scop structure
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
** @param [r] mod  [int] Model number, beginning at 1
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbAtomDomain(AjPFile outf, AjPPdb pdb, AjPScop scop, int mod)
{
    /*rn_mod is a modifier to the residue number to give correct residue 
      numbering for the domain*/

    AjIList  iter=NULL;
    AjPAtom  atm=NULL;
    AjPAtom  atm2=NULL;
    int      acnt=1;
    AjBool   found_start=ajFalse;
    AjBool   found_end=ajFalse;
    AjBool   nostart=ajFalse;
    AjBool   noend=ajFalse;
    int      rn_mod=0;  
    int      z;
    int      finalrn=0;
    char     id='\0';
    int      chn;


    for(z=0;z<scop->N;z++,found_start=ajFalse, found_end=ajFalse)
    {
	if(!ajPdbChain(scop->Chain[z], pdb, &chn))
	    {
		ajListIterFree(iter);	
		ajWarn("Chain incompatibility error in ajPrintPdbAtomDomain");		
		return ajFalse;
	    }
	

	iter=ajListIter(pdb->Chains[chn-1]->Atoms);	
	
	while((atm=(AjPAtom)ajListIterNext(iter)))
	    if(atm->Mod==mod)
		break;

	/*Increment res. counter from last chain if appropriate*/
	if(noend)
	    rn_mod += atm2->Idx;
	else	 
	    rn_mod += finalrn;

	if(!ajStrCmpC(scop->Start[z], "."))
	{
	    nostart = ajTrue;
	    /*Increment res. counter for the start of this chain*/
	    rn_mod -= atm->Idx-1;
	}
	else 
	    nostart=ajFalse;
			    
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else
	    noend=ajFalse;
	
	if(scop->N > 1)
	    id = '.';
	else 
	    id = pdb->Chains[chn-1]->Id;


	  
	for(; atm; atm=(AjPAtom)ajListIterNext(iter)) 	
	{
	    if(atm->Mod!=mod || atm->Type!='P')
		break;

	    if(!found_start && !nostart)
	    {
		if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
		{
		    rn_mod -= atm->Idx-1;
		    found_start=ajTrue;	
		}
		else	
		    continue;
	    }	
	    
	    if(!found_end && !noend)
	    {
		if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
		{
		    found_end=ajTrue;     
		    finalrn=atm->Idx;
		}
	    }	
	    else if(atm->Idx != finalrn && !noend)
		break;

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

	    atm2=atm;
	}
	

	if(!found_start && !nostart)
	    {
		ajListIterFree(iter);	
		ajWarn("Domain start not found in ajPrintPdbAtomDomain");		
		return ajFalse;
	    }
	

	if(!found_end && !noend)
	    {
		ajListIterFree(iter);	
		ajWarn("Domain end not found in ajPrintPdbAtomDomain");		
		return ajFalse;
	    }
	
	
	ajListIterFree(iter);	

    }

	ajFmtPrintF(outf, "%-6s%5d      %-4S%c%4d%54s\n", 
		    "TER", 
		    acnt++, 
		    atm2->Id3, 
		    id,	
		    atm2->Idx+rn_mod, 
		    " ");
    

    return ajTrue;
}








/* @func ajPrintPdbAtomChain *************************************************
**
** Writes the ATOM records for a chain to an output file in pdb format using 
** data from a Pdb structure 
** NOTE - never returns AjFalse - implement error catching!
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] chn  [int] Chain number, beginning at 1
** @param [r] mod  [int] Model number, beginning at 1
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbAtomChain(AjPFile outf, AjPPdb pdb, int mod, int chn)
{
    AjBool   doneter=ajFalse;
    AjIList  iter=NULL;
    AjPAtom  atm=NULL;
    AjPAtom  atm2=NULL;
    int      acnt;
    

    doneter=ajFalse;
    iter=ajListIter(pdb->Chains[chn-1]->Atoms);	

    while((atm=(AjPAtom)ajListIterNext(iter)))
	if(atm->Mod==mod)
	    break;
  
/*DEBUG
    for(acnt=1; (ajListIterMore(iter)) && (atm->Mod==mod); ) 	
    {
	atm=(AjPAtom)ajListIterNext(iter);
*/

    for(acnt=1; atm; atm=(AjPAtom)ajListIterNext(iter)) 	
    {
	if(atm->Mod!=mod)
	    break;
		
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









/* @func ajPrintPdbSeqresDomain **********************************************
**
** Writes the SEQRES record for a domain to an output file in pdb format using 
** data from a Pdb structure and a Scop structure
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbSeqresDomain(AjPFile outf, AjPPdb pdb, AjPScop scop)
{
    AjIList  iter=NULL;
    AjPAtom  atm=NULL;
    AjPStr   tmp1=NULL;
    AjPStr   tmp2=NULL;
    int      last_rn=0;  
    int      this_rn;
    int      x;
    int      y;
    int      z;
    int      rcnt=0;
    int      len;
    int	     chn=-1;
    char    *p;
    char     id;
    AjBool   found_start=ajFalse;
    AjBool   found_end=ajFalse;
    AjBool   nostart=ajFalse;
    AjBool   noend=ajFalse;


    tmp1 = ajStrNew();
    tmp2 = ajStrNew();

    for(z=0;z<scop->N;z++,found_start=ajFalse, found_end=ajFalse, last_rn=0)
    {
	if(!ajPdbChain(scop->Chain[z], pdb, &chn))
	    {
		ajListIterFree(iter);			
		ajStrDel(&tmp1);
		ajStrDel(&tmp2);
		ajWarn("Chain incompatibility error in "
		       "ajPrintPdbSeqresDomain");		
		return ajFalse;
	    }
	
	iter=ajListIter(pdb->Chains[chn-1]->Atoms);	
	
	/*JC 
	  while(ajListIterMore(iter))	
	  {
	  atm=(AjPAtom)ajListIterNext(iter);
	  */

	if(!ajStrCmpC(scop->Start[z], "."))
	    nostart = ajTrue;
	else
	    nostart=ajFalse;
	
	if(!ajStrCmpC(scop->End[z], "."))
	    noend = ajTrue;
	else	
	    noend=ajFalse;
	
	while((atm=(AjPAtom)ajListIterNext(iter)))
	{
	    if(atm->Type!='P' || atm->Mod!=1)
		break;
	
	    this_rn=atm->Idx;

	    if(this_rn!=last_rn)
	    {
		if(!found_start && !nostart)
		{
		    if(!ajStrCmpCase(atm->Pdb, scop->Start[z]))
			found_start=ajTrue;	
		    else	
		    {
			last_rn=this_rn;
			continue;
		    }
		    
		}
	    
		/*Assign sequence for residues missing from the linked list*/
		for(x=last_rn; x<this_rn-1; x++)
		{	
		    if(!ajAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), 
				    &tmp2))
			{
			    ajListIterFree(iter);			
			    ajStrDel(&tmp1);
			    ajStrDel(&tmp2);
			    ajWarn("Index out of range in "
				   "ajPrintPdbSeqresDomain");		
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

		ajStrApp(&tmp1, atm->Id3);
		ajStrAppC(&tmp1, " ");
		rcnt++;
		
		
		if(!found_end && !noend)
		{
		    if(!ajStrCmpCase(atm->Pdb, scop->End[z]))
		    {
			found_end=ajTrue;       
			break;
		    }
		}	
	    }
	}
	
	if(!found_start && !nostart)
	{
	    ajListIterFree(iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajWarn("Domain start not found in ajPrintPdbSeqresDomain");		
	    return ajFalse;
	}
	

	if(!found_end && !noend)
	{
	    ajListIterFree(iter);			
	    ajStrDel(&tmp1);
	    ajStrDel(&tmp2);
	    ajWarn("Domain end not found in ajPrintPdbSeqresDomain");		
	    return ajFalse;
	}

	/*Assign sequence for residues missing from end of linked list*/
	
	
	


	
	ajListIterFree(iter);	 		
    }
        
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

    ajListIterFree(iter);			
    ajStrDel(&tmp1);
    ajStrDel(&tmp2);

    return ajTrue;
}

       





/* @func ajPrintPdbSeqresChain ***********************************************
**
** Writes the SEQRES record for a chain to an output file in pdb format using 
** data from a Pdb structure
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] chn  [int] chain number, beginning at 1
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbSeqresChain(AjPFile outf, AjPPdb pdb, int chn)
{
    AjIList  iter=NULL;
    AjPAtom  atm=NULL;
    AjPStr   tmp1=NULL;
    AjPStr   tmp2=NULL;
    int      last_rn=0;  
    int      this_rn;
    int      x;
    int      y;
    int      len;
    char    *p;
    

    tmp1 = ajStrNew();
    tmp2 = ajStrNew();
    iter=ajListIter(pdb->Chains[chn-1]->Atoms);	

    /*DEBUG
      while(ajListIterMore(iter))	
      {
      atm=(AjPAtom)ajListIterNext(iter);
      */

    while((atm=(AjPAtom)ajListIterNext(iter)))
    {
	if(atm->Type!='P' || atm->Mod!=1)
	    break;
	
	this_rn=atm->Idx;

	if(this_rn!=last_rn)
	{
	    /*Assign sequence for residues missing from the linked list*/
	    for(x=last_rn; x<this_rn-1; x++)
	    {	
		if(!ajAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), 
				&tmp2))
		    {
			ajWarn("Index out of range in ajPrintPdbSeqresChain");		
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
	if(!ajAa1ToAa3(ajStrChar(pdb->Chains[chn-1]->Seq, x), &tmp2))
	    { 
		ajStrDel(&tmp1);
		ajStrDel(&tmp2);
		ajListIterFree(iter);	
		ajWarn("Index out of range in ajPrintPdbSeqresChain");		
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

    ajStrDel(&tmp1);
    ajStrDel(&tmp2);
    ajListIterFree(iter);	

    return ajTrue;
}





/* @func ajPrintPdbResolution ************************************************
**
** Writes the Reso element of a Pdb structure to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbResolution(AjPFile outf, AjPPdb pdb)
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





/* @func ajPrintPdbEmptyRemark ***********************************************
**
** Writes an empty REMARK record to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbEmptyRemark(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajFmtPrintF(outf, "%-11s%-69s\n", "REMARK", " ");
	return ajTrue;
    }
    else
	return ajFalse;
}






/* @func ajPrintPdbSource ****************************************************
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
AjBool ajPrintPdbSource(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajPrintPdbText(outf,pdb->Source,"SOURCE     ", 68," \t\r\n");
	return ajTrue;
    }
    else
	return ajFalse;
}





/* @func ajPrintPdbCompnd ****************************************************
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
AjBool ajPrintPdbCompnd(AjPFile outf, AjPPdb pdb)
{
    if(pdb && outf)
    {
	ajPrintPdbText(outf,pdb->Compnd,"COMPND     ", 68," \t\r\n");
	return ajTrue;
    }
    else
	return ajFalse;
}





/* @func ajPrintPdbTitle ****************************************************
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
AjBool ajPrintPdbTitle(AjPFile outf, AjPPdb pdb)
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







/* @func ajPrintPdbHeader ****************************************************
**
** Writes the Pdb element of a Pdb structure to an output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool ajPrintPdbHeader(AjPFile outf, AjPPdb pdb)
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





/* @func ajPrintPdbHeaderScop ************************************************
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
AjBool ajPrintPdbHeaderScop(AjPFile outf, AjPScop scop)
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




/* @func ajPdbWriteDomain ****************************************************
**
** Calls functions to write domain-specific contents of a Pdb object to an 
** output file in pdb format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
** @param [r] scop [AjPScop] Scop object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool   ajPdbWriteDomain(AjPFile outf, AjPPdb pdb, AjPScop scop)
{
    ajPrintPdbHeaderScop(outf, scop);
    ajPrintPdbTitle(outf, pdb);
    ajPrintPdbCompnd(outf, pdb);
    ajPrintPdbSource(outf, pdb);
    ajPrintPdbEmptyRemark(outf, pdb);
    ajPrintPdbResolution(outf, pdb);
    ajPrintPdbEmptyRemark(outf, pdb);
    
    if(!ajPrintPdbSeqresDomain(outf, pdb, scop))
    {
	ajWarn("Error writing file in ajPdbWriteDomain");
	return ajFalse;
    } 

    if(pdb->Method == ajNMR)
	ajFmtPrintF(outf, "MODEL%9d%66s\n", 1, " ");

    if(!ajPrintPdbAtomDomain(outf, pdb, scop, 1))
    {
	ajWarn("Error writing file in ajPdbWriteDomain");
	return ajFalse;
    }  

    if(pdb->Method == ajNMR)
	ajFmtPrintF(outf, "%-80s\n", "ENDMDL");

    ajFmtPrintF(outf, "%-80s\n", "END");

    return ajTrue;
}








/* @func ajPdbWriteAll *******************************************************
**
** Calls functions to write contents of a Pdb object to an output file in pdb 
** format
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] pdb  [AjPPdb] Pdb object
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool   ajPdbWriteAll(AjPFile outf, AjPPdb pdb)
{
    int x;
    int y;
    
    ajPrintPdbHeader(outf, pdb);
    ajPrintPdbTitle(outf, pdb);
    ajPrintPdbCompnd(outf, pdb);
    ajPrintPdbSource(outf, pdb);
    ajPrintPdbEmptyRemark(outf, pdb);
    ajPrintPdbResolution(outf, pdb);
    ajPrintPdbEmptyRemark(outf, pdb);
    
    for(x=0;x<pdb->Nchn;x++)
	if(!ajPrintPdbSeqresChain(outf, pdb, x+1))
	{
	    ajWarn("Error writing file in ajPdbWriteAll");
	    return ajFalse;
	}

    for(y=0;y<pdb->Nmod;y++)
    {
	if(pdb->Method == ajNMR)
	    ajFmtPrintF(outf, "MODEL%9d%66s\n", y+1, " ");

	for(x=0;x<pdb->Nchn;x++)
	    if(!ajPrintPdbAtomChain(outf, pdb, y+1, x+1))
	    {
		ajWarn("Error writing file in ajPdbWriteAll");
		return ajFalse;
	    }
	

	if(pdb->Method == ajNMR)
	    ajFmtPrintF(outf, "%-80s\n", "ENDMDL");
    }
    ajFmtPrintF(outf, "%-80s\n", "END");

    return ajTrue;
}







/* @func ajScopWrite ******************************************************
**
** Write contents of a Scop object to an output file
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] thys [AjPScop] Scop object
**
** @return [void]
** @@
******************************************************************************/

void ajScopWrite(AjPFile outf, AjPScop thys)
{
    int i;

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






/* @func ajScopRead *********************************************************
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

AjBool ajScopRead(AjPFile inf, AjPStr entry, AjPScop *thys)
{
    return ajScopReadC(inf,ajStrStr(entry),thys);
}






/* @func ajScopReadC ******************************************************
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

AjBool ajScopReadC(AjPFile inf, char *entry, AjPScop *thys)
{
    static AjPRegexp exp1=NULL;
    static AjPRegexp exp2=NULL;
    static AjPStr line=NULL;
    static AjPStr str=NULL;
    static AjPStr xentry=NULL;
    static AjPStr source=NULL;
    static AjPStr class=NULL;
    static AjPStr fold=NULL;
    static AjPStr super=NULL;
    static AjPStr family=NULL;
    static AjPStr domain=NULL;
    static AjPStr pdb=NULL;
    static AjPStr tentry=NULL;
    static AjPStr stmp=NULL;
    
    AjBool ok=ajFalse;
    
    char   *p;
    int    idx=0;
    int    n=0;
    

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
	    (*thys) = ajScopNew(n);
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

    if(!ok)
	return ajFalse;
    
    return ajTrue;
}



/* @func ajScopToPdb *********************************************************
**
** Read a scop identifier code and writes the equivalent pdb identifier code
**
** @param [r] scop [AjPStr]   Scop identifier code
** @param [w] pdb  [AjPStr*]  Pdb identifier code
**
** @return [void]
** @@
******************************************************************************/
void   ajScopToPdb(AjPStr scop, AjPStr *pdb)
{
    ajStrAssSub(pdb, scop, 1, 4);
}

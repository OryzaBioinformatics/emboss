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






/* @func ajXyzScorealgNew ***********************************************************
**
** Scorealg object constructor. 
** Fore-knowledge of the length of the alignment is required.
**
** @return [AjPScorealg] Pointer to a Scorealg object
** @param  [r] len [ajint]   Length of alignment
** 
** @@
******************************************************************************/
AjPScorealg  ajXyzScorealgNew(ajint len)
{
    AjPScorealg ret=NULL;
        

    AJNEW0(ret);


    /* Create the scoring arrays */
    ret->seq_score    = ajIntNewL((ajint)len);
    ret->post_similar = ajIntNewL((ajint)len);
    ret->ncon_score   = ajIntNewL((ajint)len);
    ret->ccon_score   = ajIntNewL((ajint)len);
    ret->nccon_score = ajIntNewL((ajint)len);
    ret->combi_score  = ajIntNewL((ajint)len);

    ret->seq_do    = ajFalse;
    ret->filter    = ajFalse;
    ret->ncon_do   = ajFalse;
    ret->ccon_do   = ajFalse;
    ret->nccon_do = ajFalse;
    
    return ret;
}







/* @func ajXyzVdwallNew ********************************************************
**
** Vdwall object constructor. This is normally called by the ajXyzVdwallRead
** function. Fore-knowledge of the number of residues is required.
**
** @return [AjPVdwall] Pointer to a Vdwall object
** @param  [r] n [ajint]  Number of residues
** @@
******************************************************************************/
AjPVdwall  ajXyzVdwallNew(ajint n)
{
    AjPVdwall ret=NULL;
    
    AJNEW0(ret);

    ret->N=n;

    AJCNEW0(ret->Res, n);

    return ret;
}





/* @func ajXyzVdwresNew ********************************************************
**
** Vdwres object constructor. This is normally called by the ajXyzVdwallRead
** function. Fore-knowledge of the number of atoms is required.
**
** @return [AjPVdwres] Pointer to a Vdwres object
** @param  [r] n [ajint]  Number of atoms
** @@
******************************************************************************/
AjPVdwres  ajXyzVdwresNew(ajint n)
{
    ajint x;
    AjPVdwres ret=NULL;
    
    AJNEW0(ret);

    ret->Id3=ajStrNew();    
    ret->N=n;

    AJCNEW0(ret->Atm, n);
    for(x=0;x<n;++x)
	ret->Atm[x]=ajStrNew();

    AJCNEW0(ret->Rad, n);


    return ret;
}




/* @func ajXyzCmapNew ***********************************************************
**
** Cmap object constructor. This is normally called by the ajXyzCmapRead
** function. Fore-knowledge of the dimension (number of residues) for the 
** contact map is required.
**
** @return [AjPCmap] Pointer to a Cmap object
** @param  [r] dim [ajint]   Dimenion of contact map
** 
** @@
******************************************************************************/
AjPCmap  ajXyzCmapNew(ajint dim)
{
    AjPCmap ret=NULL;
    ajint z=0;
    

    AJNEW0(ret);

    ret->Id=ajStrNew();    

    /* Create the SQUARE contact map */
    ret->Mat = ajInt2dNewL((ajint)dim);
    for(z=0;z<dim;++z)
	ajInt2dPut(&ret->Mat, z, dim-1, (ajint) 0);


    ret->Dim=dim;
    ret->Ncon=0;


    return ret;
}


/* @func ajXyzScopalgNew ***********************************************************
**
** Scopalg object constructor. This is normally called by the ajXyzScopalgRead
** function. Fore-knowledge of the number of sequences is required.
**
** @param [r] n [int]   Number of sequences
** 
** @return [AjPScopalg] Pointer to a Scopalg object
** @@
******************************************************************************/
AjPScopalg  ajXyzScopalgNew(int n)
{
    AjPScopalg ret = NULL;
    ajint i=0;
    

    AJNEW0(ret);
    ret->Class=ajStrNew();
    ret->Fold=ajStrNew();
    ret->Superfamily=ajStrNew();
    ret->Family=ajStrNew();
    ret->Post_similar=ajStrNew();
    ret->width=0;
    ret->N=n;

    AJCNEW0(ret->Codes,n);
    for(i=0;i<n;++i)
	ret->Codes[i] = ajStrNew();

    AJCNEW0(ret->Seqs,n);
    for(i=0;i<n;++i)
	ret->Seqs[i] = ajStrNew();
    
    return ret;
}






/* @func ajXyzScophitNew *********************************************************
**
** Scophit object constructor. 
**
** @return [AjPScophit] Pointer to a Scophit object
** @@
******************************************************************************/
AjPScophit  ajXyzScophitNew(void)
{
    AjPScophit ret = NULL;

    AJNEW0(ret);

    ret->Class       =ajStrNew();
    ret->Fold        =ajStrNew();
    ret->Superfamily =ajStrNew();
    ret->Family      =ajStrNew();
    ret->Seq         = ajStrNew();
    ret->Id          = ajStrNew();
    ret->Type        = ajStrNew();
    ret->Start       =0;
    ret->End         =0;
    ret->Group       =0;
    
    return ret;
}




/* @func ajXyzHitNew *********************************************************
**
** Hit object constructor. This is normally called by the ajXyzHitlistNew
** function.
**
** @return [AjPHit] Pointer to a hit object
** @@
******************************************************************************/
AjPHit  ajXyzHitNew(void)
{
    AjPHit ret = NULL;

    AJNEW0(ret);

    ret->Seq       = ajStrNew();
    ret->Id        = ajStrNew();
    ret->Type      = ajStrNew();
    ret->Start     =0;
    ret->End       =0;
    ret->Group     =0;
    
    return ret;
}




/* @func ajXyzHitlistNew ***********************************************************
**
** Hitlist object constructor. This is normally called by the ajXyzHitlistRead
** function. Fore-knowledge of the number of hits is required.
**
** @param [r] n [int *] Number of hits
** 
** @return [AjPHitlist] Pointer to a hitlist object
** @@
******************************************************************************/
AjPHitlist  ajXyzHitlistNew(int n)
{
    AjPHitlist ret = NULL;
    ajint i=0;
    

    AJNEW0(ret);
    ret->Class=ajStrNew();
    ret->Fold=ajStrNew();
    ret->Superfamily=ajStrNew();
    ret->Family=ajStrNew();
    ret->N=n;

    AJCNEW0(ret->hits,n);
    for(i=0;i<n;++i)
	ret->hits[i] = ajXyzHitNew();
    
    return ret;
}



/* @func ajXyzPdbNew ************************************************************
**
** Pdb object constructor. Fore-knowledge of the number of chains 
** is required. This is normally called by the ajXyzCpdbRead function.
**
** @param [r] chains [ajint] Number of chains in this pdb file
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
** Atom object constructor.
** This is normally called by the ajXyzChainNew function.
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
** required. This is normally called by the ajXyzScopReadC / ajXyzScopRead 
** functions.
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




/* @func ajXyzScorealgDel *******************************************************
**
** Destructor for Scorealg object.
**
** @param [w] pthis [AjPScorealg*] Scorealg object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzScorealgDel(AjPScorealg *pthis)
{
    ajIntDel(&(*pthis)->seq_score);
    ajIntDel(&(*pthis)->post_similar);
    ajIntDel(&(*pthis)->ncon_score);
    ajIntDel(&(*pthis)->ccon_score);
    ajIntDel(&(*pthis)->nccon_score);
    ajIntDel(&(*pthis)->combi_score);

    AJFREE(*pthis);    

    return;
}	




/* @func ajXyzVdwresDel *******************************************************
**
** Destructor for Vdwres object.
**
** @param [w] pthis [AjPVdwres*] Vdwres object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzVdwresDel(AjPVdwres *pthis)
{
    ajint x=0;
    
    ajStrDel(&(*pthis)->Id3);
    
    for(x=0;x<(*pthis)->N; ++x)
	ajStrDel(&(*pthis)->Atm[x]);
    
    AJFREE((*pthis)->Atm);
    AJFREE((*pthis)->Rad);
    AJFREE(*pthis);    

    return;
}	



/* @func ajXyzVdwallDel *******************************************************
**
** Destructor for Vdwall object.
**
** @param [w] pthis [AjPVdwall*] Vdwall object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzVdwallDel(AjPVdwall *pthis)
{
    ajint x=0;
    
    for(x=0;x<(*pthis)->N; ++x)
	ajXyzVdwresDel(&(*pthis)->Res[x]);
    
    AJFREE((*pthis)->Res);
    AJFREE(*pthis);    

    return;
}	




/* @func ajXyzCmapDel *******************************************************
**
** Destructor for Cmap object.
**
** @param [w] pthis [AjPCmap*] Cmap object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzCmapDel(AjPCmap *pthis)
{
    ajStrDel(&(*pthis)->Id);
    ajInt2dDel(&(*pthis)->Mat);
    AJFREE(*pthis);    

    return;
}	



/* @func ajXyzScopalgDel *******************************************************
**
** Destructor for Scopalg object.
**
** @param [w] pthis [AjPScopalg*] Scopalg object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzScopalgDel(AjPScopalg *pthis)
{
    int x=0;  /* Counter */
    
    ajStrDel(&(*pthis)->Class);
    ajStrDel(&(*pthis)->Fold);
    ajStrDel(&(*pthis)->Superfamily);
    ajStrDel(&(*pthis)->Family);
    ajStrDel(&(*pthis)->Post_similar);

    for(x=0;x<(*pthis)->N; x++)
    {
	ajStrDel(&(*pthis)->Codes[x]);
	ajStrDel(&(*pthis)->Seqs[x]);
    }
    
    AJFREE((*pthis)->Codes);
    AJFREE((*pthis)->Seqs);
    
    AJFREE(*pthis);
    
    return;
}





/* @func ajXyzScophitDel ******************************************************
**
** Destructor for Scophit object.
**
** @param [w] pthis [AjPScophit*] Scophit object pointer
**
** @return [void]
** @@
******************************************************************************/
void     ajXyzScophitDel(AjPScophit *pthis)
{
    ajStrDel(&(*pthis)->Class);
    ajStrDel(&(*pthis)->Fold);
    ajStrDel(&(*pthis)->Superfamily);
    ajStrDel(&(*pthis)->Family);
    ajStrDel(&(*pthis)->Seq);
    ajStrDel(&(*pthis)->Id);
    ajStrDel(&(*pthis)->Type);

    AJFREE(*pthis);
    
    return;
}




/* @func ajXyzHitDel ***********************************************************
**
** Destructor for hit object.
**
** @param [w] pthis [AjPHit*] Hit object pointer
**
** @return [void]
** @@
******************************************************************************/
void     ajXyzHitDel(AjPHit *pthis)
{
    ajStrDel(&(*pthis)->Seq);
    ajStrDel(&(*pthis)->Id);
    ajStrDel(&(*pthis)->Type);

    AJFREE(*pthis);
    
    return;
}



/* @func ajXyzHitlistDel *******************************************************
**
** Destructor for hitlist object.
**
** @param [w] pthis [AjPHitlist*] Hitlist object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzHitlistDel(AjPHitlist *pthis)
{
    int x=0;  /* Counter */
    
    ajStrDel(&(*pthis)->Class);
    ajStrDel(&(*pthis)->Fold);
    ajStrDel(&(*pthis)->Superfamily);
    ajStrDel(&(*pthis)->Family);
    
    for(x=0;x<(*pthis)->N; x++)
	ajXyzHitDel(&(*pthis)->hits[x]);

    AJFREE((*pthis)->hits);
    
    AJFREE(*pthis);
    
    return;
}




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
** Destructor for atom object.
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
** Reads a Cpdb file and writes a filled Pdb object.
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
** Writes a Cpdb file for a SCOP domain. Where coordinates for multiple 
** models (e.g. NMR structures) are given, data for model 1 are written.
** In the Cpdb file, the coordinates are presented as belonging to a single 
** chain regardless of how many chains the domain comprised.
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
** Writes a Cpdb file for a protein.
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
** Finds the chain number for a given chain identifier in a pdb structure
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
** Writes text to file in the format of pdb records
** 
** @param [w] outf   [AjPFile] Output file stream
** @param [r] str    [AjPStr]  Text to print out
** @param [r] prefix [char *]  pdb record (e.g. "HEADER")
**
**
** @return [AjBool] True on succcess
** @@
******************************************************************************/
AjBool  ajXyzPrintPdbText(AjPFile outf, AjPStr str, char *prefix)
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
    

    handle = ajStrTokenInit(str,(char *) " \t\r\n");
    
    while(ajStrToken(&token,&handle,NULL))
    {
	if(!c)
	    ajFmtPrintF(outf,"%-11s",prefix);
	
	if((l=n+ajStrLen(token)) < 68)
	{
	    if(c++)
		ajStrAppC(&tmp," ");
	    ajStrApp(&tmp,token);
	    n = ++l;
	}
	else
	{
	    ajFmtPrintF(outf,"%-*S\n",69, tmp);

	    ajStrAssS(&tmp,token);
	    ajStrAppC(&tmp," ");
	    n = ajStrLen(token);
	    c = 0;
	}
    }

    if(c)
    {
	ajFmtPrintF(outf,"%-*S\n",69, tmp);
    }
    

    ajStrTokenClear(&handle);
    ajStrDel(&token);
    ajStrDel(&tmp);
    
    return ajTrue;
}






/* @func ajXyzPrintPdbAtomDomain ************************************************
**
** Writes coordinates for a SCOP domain to an output file in pdb format (ATOM 
** records).  Coordinates are taken from a Pdb structure, domain definition is 
** taken from a Scop structure. The model number argument should have a value of 
** 1 for x-ray structures.
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
** Writes coordinates for a protein chain to an output file in pdb format (ATOM 
** records). Coordinates are taken from a Pdb structure. The model number 
** argument should have a value of 1 for x-ray structures.
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
** Writes sequence for a SCOP domain to an output file in pdb format (SEQRES 
** records). Sequence is taken from a Pdb structure, domain definition is taken 
** from a Scop structure.  Where coordinates for multiple models (e.g. NMR 
** structures) are given, data for model 1 are written.
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
** Writes sequence for a protein chain to an output file in pdb format (SEQRES
** records).  Sequence is taken from a Pdb structure.  The model number argument 
** should have a value of 1 for x-ray structures.
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
** Writes the Reso element of a Pdb structure to an output file in pdb 
** format
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
	ajXyzPrintPdbText(outf,pdb->Source,"SOURCE");
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
	ajXyzPrintPdbText(outf,pdb->Compnd,"COMPND");
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
** Writes a pdb file for a SCOP domain. Where coordinates for multiple 
** models (e.g. NMR structures) are given, data for model 1 are written. 
** Coordinates are taken from a Pdb structure, domain definition is taken from 
** a Scop structure.
** In the pdb file, the coordinates are presented as belonging to a single 
** chain regardless of how many chains the domain comprised.
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
** Writes a pdb file for a protein.
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
** Write contents of a Scop object to an output file in embl-like format.
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
** Read a Scop object from a file in embl-like format.
**
** @param [r] inf [AjPFile] Input file stream
** @param [r] entry [AjPStr] id
** @param [w] thys [AjPScop*] Scop object
**
** @return [AjBool] True on success
** @@
******************************************************************************/

AjBool ajXyzScopRead(AjPFile inf, AjPStr entry, AjPScop *thys)
{
    return ajXyzScopReadC(inf,ajStrStr(entry),thys);
}






/* @func ajXyzScopReadC ******************************************************
**
** Read a Scop object from a file in embl-like format.
**
** @param [r] inf [AjPFile] Input file stream
** @param [r] entry [char*] id
** @param [w] thys [AjPScop*] Scop object
**
** @return [AjBool] True on success
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






/* @func ajXyzScopalgRead ****************************************************
**
** Read a Scopalg object from a file in embl-like format.
** 
** @param [r] inf      [AjPFile] Input file stream
** @param [w] thys     [AjPScopalg*]  Scopalg object
**
** @return [AjBool] True on success (an alignment was read)
** @@
******************************************************************************/
AjBool   ajXyzScopalgRead(AjPFile inf, AjPScopalg *thys)
{
    static   AjPStr line    =NULL;     /* Line of text */
    static   AjPStr class   =NULL;
    static   AjPStr fold    =NULL;
    static   AjPStr super   =NULL;
    static   AjPStr family  =NULL;
    static   AjPStr postsim =NULL;     /* Post-similar line */
    static   AjPStr posttmp =NULL;     /* Temp. storage for post-similar line */
    
    AjBool  done_1st_blk    =ajFalse;  /* Flag for whether we've read first block of sequences */
    ajint   x               =0;        /* Loop counter */
    ajint   y               =0;        /* Loop counter */
    ajint   cnt             =0;
    
    AjPList list_seqs    =NULL;     /* List of sequences */
    AjPList list_codes   =NULL;     /* List of codes */
    AjPStr  *arr_seqs       =NULL;     /* Array of sequences */
    AjPStr  seq             =NULL;     
    AjPStr  code            =NULL;     /* Id code of sequence */
    AjPStr  seq1            =NULL;


    /* Check args */	
    if(!inf)
	return ajFalse;
    

    /* Allocate strings */
    /* Only initialise strings if this is called for the first time*/
    if(!line)
    {
	class   = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	line    = ajStrNew();
	postsim = ajStrNew();
	posttmp = ajStrNew();
	seq1    = ajStrNew();
    }

    
    /* Create new lists */
    list_seqs = ajListstrNew();
    list_codes = ajListstrNew();


    /* Start of code for reading input file */
    /*Ignore everything up to first line beginning with 'Number'*/
    while(ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"Number"))
	    break;
    }

    /* Read the rest of the file */
    while(ajFileReadLine(inf,&line))
    {
	/* Ignore 'Number' lines */
	if((ajStrPrefixC(line,"Number")))
	    continue;
    	else if(ajStrPrefixC(line,"CL"))
	    ajStrAssC(&class,ajStrStr(line)+3);
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&fold,ajStrStr(line)+3);
	    while((ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssC(&super,ajStrStr(line)+3);
	    while((ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&super,ajStrStr(line)+3);
	    }
	    ajStrClean(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssC(&family,ajStrStr(line)+3);
	    while((ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"XX"))
	    continue;
	else if (ajStrPrefixC(line,"Post_similar"))
	{
	    /* Parse post_similar line */
	    ajFmtScanS(line, "%*s%S", &posttmp);
	    if(done_1st_blk == ajTrue)
		ajStrApp(&postsim, posttmp);
	    else
		ajStrAss(&postsim, posttmp);
	    
	    continue;
	}
	else if(ajStrChar(line,1)=='\0')
	{ 
	    /* If we are on a blank line */
	    /* ajFileReadLine will trim the tailing \n */

	    done_1st_blk=ajTrue;
	    cnt = 0;
	    y++;

	    if(y == 1)
	    {
		x = ajListstrToArray(list_seqs, &arr_seqs); 
	    }
	    continue;
	}
	else
	{
	    /* Parse a line of sequence */
	    if(done_1st_blk == ajTrue)
	    {
		/* We have already read in the first block of sequences */
		ajFmtScanS(line, "%*s%S", &seq1);
		ajStrApp(&arr_seqs[cnt], seq1);
		cnt++;
		continue;
	    }	
	    else
	    {
		/* It is a sequence line from the first block */
		/* Read in sequence */
		seq = ajStrNew();		
		code = ajStrNew();		
		ajFmtScanS(line, "%S%S", &code, &seq);
		
		/* Push strings onto lists */
		ajListstrPushApp(list_seqs,seq);
		ajListstrPushApp(list_codes,code);
		continue;
	    }
	}	
    }


    if(!cnt)
    {
	ajWarn("No sequences in alignment !\n");
	ajListstrDel(&list_seqs); 
	return ajFalse;
    }
    

    /* Allocate memory for Scopalg structure */
    (*thys) = ajXyzScopalgNew(cnt);


    /* Assign SCOP records */
    ajStrAssS(&(*thys)->Class,class);
    ajStrAssS(&(*thys)->Fold,fold);
    ajStrAssS(&(*thys)->Superfamily,super);
    ajStrAssS(&(*thys)->Family,family); 
    
    /* Assign width */
    (*thys)->width = ajStrLen((*thys)->Seqs[0]);

    
    /* Assign sequences and free memory */
    for(x=0; x<cnt; x++)
	{
	    ajStrAssS(&(*thys)->Seqs[x],arr_seqs[x]); 
	    AJFREE(arr_seqs[x]);
	}
    /* Free array */
    AJFREE(arr_seqs);


    for(x=0; ajListstrPop(list_codes,&code); x++)
	ajStrAssS(&(*thys)->Codes[x],arr_seqs[x]); 	


    /* Assign Post_similar line */
    ajStrAssS(&(*thys)->Post_similar,postsim); 


    /* Clean up */
    ajListstrDel(&list_seqs); 
    ajListstrDel(&list_codes); 
    

    /* Return */
    ajExit();
    return ajTrue;
}







/* @func ajXyzHitlistRead ****************************************************
**
** Read a hitlist object from a file in embl-like format. 
** 
** @param [r] inf      [AjPFile] Input file stream
** @param [r] delim    [char *]  Delimiter for block of hits
** @param [w] thys     [AjPHitlist*] Hitlist object
**
** @return [AjBool] True on success (a list of hits was read)
** @@
******************************************************************************/
AjBool   ajXyzHitlistRead(AjPFile inf, char *delim, AjPHitlist *thys)
{
    static   AjPStr line    =NULL;   /* Line of text */
    static   AjPStr class   =NULL;
    static   AjPStr fold    =NULL;
    static   AjPStr super   =NULL;
    static   AjPStr family  =NULL;
    AjBool   ok             =ajFalse;
    ajint    n              =0;      /* Number of current sequence */
    ajint    nset           =0;      /* Number in set */
    



    /* Allocate strings */
    /* Only initialise strings if this is called for the first time*/
    if(!line)
    {
	class   = ajStrNew();
	fold    = ajStrNew();
	super   = ajStrNew();
	family  = ajStrNew();
	line    = ajStrNew();
    }
    

    
    /* Read first line */
    ok = ajFileReadLine(inf,&line);

    while(ok && !ajStrPrefixC(line,delim))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"CL"))
	    ajStrAssC(&class,ajStrStr(line)+3);
	else if(ajStrPrefixC(line,"FO"))
	{
	    ajStrAssC(&fold,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&fold,ajStrStr(line)+3);
	    }
	    ajStrClean(&fold);
	}
	else if(ajStrPrefixC(line,"SF"))
	{
	    ajStrAssC(&super,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&super,ajStrStr(line)+3);
	    }
	    ajStrClean(&super);
	}
	else if(ajStrPrefixC(line,"FA"))
	{
	    ajStrAssC(&family,ajStrStr(line)+3);
	    while((ok = ajFileReadLine(inf,&line)))
	    {
		if(ajStrPrefixC(line,"XX"))
		    break;
		ajStrAppC(&family,ajStrStr(line)+3);
	    }
	    ajStrClean(&family);
	}
	else if(ajStrPrefixC(line,"NS"))
	{
	    ajFmtScanS(line, "NS%d", &nset);


	    /* Create hitlist structure */
	    (*thys)=ajXyzHitlistNew(nset);
	    (*thys)->N=nset;
	    ajStrAssS(&(*thys)->Class, class);
	    ajStrAssS(&(*thys)->Fold, fold);
	    ajStrAssS(&(*thys)->Superfamily, super);
	    ajStrAssS(&(*thys)->Family, family);
	}
	else if(ajStrPrefixC(line,"NN"))
	{
	    /* Increment hit counter */
	    n++;

	    /* Safety check */
	    if(n>nset)
		ajFatal("Dangerous error in input file caught in ajXyzHitlistRead.\n Email jison@hgmp.mrc.ac.uk");
	}
	else if(ajStrPrefixC(line,"ID"))
	    ajStrAssC(&(*thys)->hits[n-1]->Id,ajStrStr(line)+3);
	else if(ajStrPrefixC(line,"TY"))
	    ajStrAssC(&(*thys)->hits[n-1]->Type,ajStrStr(line)+3);
	else if(ajStrPrefixC(line,"RA"))
	    ajFmtScanS(line, "%*s %d %*s %d", &(*thys)->hits[n-1]->Start, &(*thys)->hits[n-1]->End);
	else if(ajStrPrefixC(line,"GP"))
	    ajFmtScanS(line, "%*s %d", &(*thys)->hits[n-1]->Group);
	else if(ajStrPrefixC(line,"SQ"))
	{
	    while((ok=ajFileReadLine(inf,&line)) && !ajStrPrefixC(line,"XX"))
		ajStrAppC(&(*thys)->hits[n-1]->Seq,ajStrStr(line));
	    ajStrCleanWhite(&(*thys)->hits[n-1]->Seq);
	    continue;
	}
	
	
	ok = ajFileReadLine(inf,&line);
    }


    /* Return */
    if(!ok)
	return ajFalse;
    else
	return ajTrue;
}





/* @func ajXyzHitlistWrite ***************************************************
**
** Write contents of a Hitlist object to an output file in embl-like format.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] thys [AjPHitlist] Hitlist object
**
** @return [AjBool] True on success
** @@
******************************************************************************/
AjBool ajXyzHitlistWrite(AjPFile outf, AjPHitlist thys)
{
    ajint x=0;  /* Counter */
    
    if(!thys)
	return ajFalse;

    ajFmtPrintF(outf,"CL   %S",thys->Class);
    ajFmtPrintSplit(outf,thys->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nNS   %d\nXX\n",thys->N);

    for(x=0;x<thys->N;x++)
    {
	ajFmtPrintF(outf, "%-5s[%d]\nXX\n", "NN", x+1);
	ajFmtPrintF(outf, "%-5s%S\n", "ID", thys->hits[x]->Id);
	ajFmtPrintF(outf, "XX\n");
	ajFmtPrintF(outf, "%-5s%S\n", "TY", thys->hits[x]->Type);
	ajFmtPrintF(outf, "XX\n");
	if(thys->hits[x]->Group)
	{
	    ajFmtPrintF(outf, "%-5s%S\n", "GP", thys->hits[x]->Group);
	    ajFmtPrintF(outf, "XX\n");
	}
	ajFmtPrintF(outf, "%-5s%d START; %d END;\n", "RA", thys->hits[x]->Start, thys->hits[x]->End);
	ajFmtPrintF(outf, "XX\n");
	ajSeqWriteCdb(outf, thys->hits[x]->Seq);
	ajFmtPrintF(outf, "XX\n//\n");
    }
    

    /* Return */
    return ajTrue;
}






/* @func ajXyzCmapRead ****************************************************
**
** Read a Cmap object from a file in embl-like format. 
** 
** @param [r] inf     [AjPFile]  Input file stream
** @param [r] chn     [ajint]    Chain number
** @param [r] mod     [ajint]    Model number
** @param [w] thys    [AjPCmap*] Pointer to Cmap object
**
** @return [AjBool] True on success (a list of hits was read)
** @@
******************************************************************************/
AjBool   ajXyzCmapRead(AjPFile inf, ajint chn, ajint mod, AjPCmap *thys)
{	
    static   AjPStr line    =NULL;   /* Line of text */
    static   AjPStr temp_id =NULL;   /* Temp location for protein id */
    
    ajint    num_res   	    =0;      /* No. of residues in domain */	
    ajint    num_con   	    =0;      /* Total no. of contacts in domain */	
    ajint    x		    =0;      /* No. of first residue making contact */	
    ajint    y              =0;      /* No. of second residue making contact */	
    ajint    md             =-1;     /* Model number */
    ajint    cn             =-1;     /* Chain number */


    /* Check args */	
    if(!inf)
    {	
	ajWarn("Invalid args to ajXyzCmapRead");	
	return ajFalse;
    }
    

    /* Initialise strings */
    if(!line)
    {
	line     = ajStrNew();
	temp_id  = ajStrNew();
    }
    

    /* Start of main loop */
    while((ajFileReadLine(inf, &line)))
    {
	/* Parse ID line */
	if(ajStrPrefixC(line, "ID"))
	    ajFmtScanS(line, "%*s %S", &temp_id);
	/* Parse model number */
	else if(ajStrPrefixC(line, "MO"))
	    ajFmtScanS(line, "%*s[%d]", &md);
	/* Parse chain number */
	else if(ajStrPrefixC(line, "CN"))
	    ajFmtScanS(line, "%*s[%d]", &cn);
	/* Read IN line */	    
	/* Parse number of residues and total number of contacts */
	else if((ajStrPrefixC(line, "IN")) && (md==mod) && (cn==chn))
	{
	    ajFmtScanS(line, "%*s %*s %*s %*s %d; %*s %d;", 
		       &num_res, &num_con);


	    /* Allocate contact map and write values */
	    (*thys)=ajXyzCmapNew(num_res);
	    (*thys)->Ncon = num_con;
	    ajStrAssS(&(*thys)->Id, temp_id);
	}
	/* Read and parse residue contacts */
	else if((ajStrPrefixC(line, "SM")) && (md==mod) && (cn==chn))
	{
	    ajFmtScanS(line, "%*s %*s %d %*c %*s %d", &x, &y);

	    /* Check residue number is in range */
	    if((x>(*thys)->Dim) || (y>(*thys)->Dim))
		ajFatal("Fatal attempt to write bad data in ajXyzCmapRead\nEmail culprit: jison@hgmp.mrc.ac.uk\n");
	    

	    /* Enter '1' in matrix to indicate contact */
	    ajInt2dPut(&(*thys)->Mat, x-1, y-1, 1);
	    ajInt2dPut(&(*thys)->Mat, y-1, x-1, 1);
	}
    }	
    
    /* Return */
    return ajTrue;	
}	





/* @func ajXyzVdwallRead ****************************************************
**
** Read a Vdwall object from a file in embl-like format. 
** 
** @param [r] inf     [AjPFile]  Input file stream
** @param [w] thys    [AjPVdwall*] Pointer to Vdwall object
**
** @return [AjBool] True on success (a list of hits was read)
** @@
******************************************************************************/
AjBool   ajXyzVdwallRead(AjPFile inf, AjPVdwall *thys)
{
    AjPStr line    =NULL;   /* Line of text */
    ajint  nres    =0;      /* No. residues */
    ajint  natm    =0;      /* No. atoms */
    ajint  rcnt    =0;      /* Residue count */
    ajint  acnt    =0;      /* Atom count */
    char   id1     ='\0';             /* Residue 1 char id code */
    AjPStr id3     =NULL;   /* Residue 3 char id code */
    
    
    /* Allocate strings */
    line     = ajStrNew();
    id3      = ajStrNew();


    /* Start of main loop */
    while((ajFileReadLine(inf, &line)))
    {
	/* Parse ID line */
	if(ajStrPrefixC(line, "NR"))
	    {	
		ajFmtScanS(line, "%*s %d", &nres);
		
		/* Allocate Vdwall object */
		(*thys)=ajXyzVdwallNew(nres);
		
	    }
	/* Parse residue id 3 char */
	else if(ajStrPrefixC(line, "AA"))
	    {	
		rcnt++;
		acnt=0;
		ajFmtScanS(line, "%*s %S", &id3);
	    }
	/* Parse residue id 1 char */
	else if(ajStrPrefixC(line, "ID"))
	    ajFmtScanS(line, "%*s %c", &id1);
	/* Parse number of atoms */
	else if(ajStrPrefixC(line, "NN"))
	{
	    ajFmtScanS(line, "%*s %d", &natm);
	    
	    /* Allocate next Vdwres object */
	    (*thys)->Res[rcnt-1]=ajXyzVdwresNew(natm);
	    
	    /* Write members of Vdwres object */
	    (*thys)->Res[rcnt-1]->Id1=id1;
	    ajStrAss(&(*thys)->Res[rcnt-1]->Id3, id3);
	    
	}
	/* Parse atom line */
	else if(ajStrPrefixC(line, "AT"))
	{
	    acnt++;
	    ajFmtScanS(line, "%*s %S %*c %f", 
		       &(*thys)->Res[rcnt-1]->Atm[acnt-1], 
		       &(*thys)->Res[rcnt-1]->Rad[acnt-1]);	
	}
    }	

    /* Clear up */
    ajStrDel(&line);
    ajStrDel(&id3);
    

    /* Return */
    return ajTrue;
}





/* @func ajXyzHitlistToScophits ****************************************************
**
** Read from a list of pointers to Hitlist structures and writes a pointer to a 
** list of Scophit structures.
** 
** @param [r] in      [AjPList]  List of pointers to Hitlist structures
** @param [w] out     [AjPList*] Pointer to list of Scophit structures
**
** @return [AjBool] True on success (lists were processed ok)
** @@
******************************************************************************/
AjBool ajXyzHitlistToScophits(AjPList in, AjPList *out)
{
    AjPScophit   scophit=NULL;   /* Pointer to Scophit object */
    AjPHitlist   hitlist=NULL;   /* Pointer to Hitlist object */
    AjIList      iter   =NULL;   /* List iterator */
    ajint        x      =0;      /* Loop counter */
   

    /* Check args */
    if(!in)
    {
	ajWarn("Null arg passed to ajXyzHitlistToScophits");
	return ajFalse;
    }

    /* Create list iterator and new list */
    iter=ajListIter(in);	
    

    /* Iterate through the list of Hitlist pointers */
    while((hitlist=(AjPHitlist)ajListIterNext(iter)))
    {
	/* Loop for each hit in hitlist structure */
	for(x=0; x<hitlist->N; ++x)
	{
	    /* Create a new scophit structure */
	    AJNEW0(scophit);

	    /* Assign scop classification records from hitlist structure */
	    ajStrAss(&scophit->Class, hitlist->Class);
	    ajStrAss(&scophit->Fold, hitlist->Fold);
	    ajStrAss(&scophit->Superfamily, hitlist->Superfamily);
	    ajStrAss(&scophit->Family, hitlist->Family);

	    /* Assign records from hit structure */
	    ajStrAss(&scophit->Seq, hitlist->hits[x]->Seq);
	    ajStrAss(&scophit->Id, hitlist->hits[x]->Id);
	    ajStrAss(&scophit->Type, hitlist->hits[x]->Type);
	    scophit->Start = hitlist->hits[x]->Start;
	    scophit->End = hitlist->hits[x]->End;
	    scophit->Group = hitlist->hits[x]->Group;

	    
	    /* Push scophit onto list */
	    ajListPushApp(*out,scophit);
	}
    }	
    

    /* Clean up and return */
    ajListIterFree(iter);	

    return ajTrue;
}



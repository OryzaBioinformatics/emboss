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
#define CMAP_MODE_I   1
#define CMAP_MODE_C   2



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



/* @func ajXyzSigposNew ******************************************************
**
** Sigpos object constructor. This is normally called by the ajXyzSignatureCompile
** function. Fore-knowledge of the number of permissible gaps is required.
**
** @param [r] ngap [ajint]   Number of permissible gaps.
** 
** @return [AjPSigpos] Pointer to a Sigpos object
** @@
******************************************************************************/
AjPSigpos     ajXyzSigposNew(ajint ngap)
{
    AjPSigpos ret = NULL;

    AJNEW0(ret);
    ret->ngaps=ngap;

    /* Create arrays */
    AJCNEW0(ret->gsiz, ngap);
    AJCNEW0(ret->gpen, ngap);
    AJCNEW0(ret->subs, 26);
        
    return ret;
}




/* @func ajXyzSigdatNew ******************************************************
**
** Sigdat object constructor. This is normally called by the ajXyzSignatureRead
** function. Fore-knowledge of the number of empirical residues and gaps is 
** required.
** Important: Functions which manipulate the Sigdat object rely on data in the 
** gap arrays (gsiz and grfq) being filled in order of increasing gap size.
**
** @param [r] nres [ajint]   Number of emprical residues.
** @param [r] ngap [ajint]   Number of emprical gaps.
** 
** @return [AjPSigdat] Pointer to a Sigdat object
** @@
******************************************************************************/
AjPSigdat     ajXyzSigdatNew(ajint nres, ajint ngap)
{
    AjPSigdat ret = NULL;


    AJNEW0(ret);
    ret->nres=nres;
    ret->ngap=ngap;

    /* Create arrays */
/*  
    AJCNEW0(ret->gsiz, ngap);
    AJCNEW0(ret->gfrq, ngap);
    AJCNEW0(ret->rids, nres);
    AJCNEW0(ret->rfrq, nres); */
        
    if(ngap)
    {
	ret->gsiz = ajIntNewL((ajint) ngap);
	ret->gfrq = ajIntNewL((ajint) ngap);
	ajIntPut(&ret->gsiz, ngap-1, (int)0);
	ajIntPut(&ret->gfrq, ngap-1, (int)0);
    }
    else
    {
	ret->gsiz = ajIntNew();
	ret->gfrq = ajIntNew();
	ajIntPut(&ret->gsiz, 0, (int)0);
	ajIntPut(&ret->gfrq, 0, (int)0);
    }

    if(nres)
    {
	ret->rids = ajChararrNewL((ajint) nres);
	ret->rfrq = ajIntNewL((ajint) nres);
        ajIntPut(&ret->rfrq, nres-1, (int)0);
	ajChararrPut(&ret->rids, nres-1, (char)' ');
    }
    else
    {
	ret->rids = ajChararrNew();
	ret->rfrq = ajIntNew();
	ajIntPut(&ret->rfrq, 0, (int)0);
	ajChararrPut(&ret->rids, 0, (char)' ');
    }
    
    return ret;
}





/* @func ajXyzSignatureNew ***************************************************
**
** Signature object constructor. This is normally called by the ajXyzSignatureRead
** function. Fore-knowledge of the number of signature positions is required.
**
** @param [r] n [ajint]   Number of signature positions
** 
** @return [AjPSignature] Pointer to a Signature object
** @@
******************************************************************************/
AjPSignature  ajXyzSignatureNew(ajint n)
{
    AjPSignature ret = NULL;


    AJNEW0(ret);
    ret->Class=ajStrNew();
    ret->Fold=ajStrNew();
    ret->Superfamily=ajStrNew();
    ret->Family=ajStrNew();
    ret->npos=n;

    /* Create arrays of pointers to Sigdat & Sigpos structures */
    if(n)
    {
	ret->dat = AJCALLOC0(n, sizeof(AjPSigdat));
	ret->pos = AJCALLOC0(n, sizeof(AjPSigpos));
    }
    
    return ret;
}



/* @func ajXyzHitidxNew ******************************************************
**
** Hitidx object constructor. This is normally called by the ajXyzHitlistClassify
** function.
**
** @return [AjPHitidx] Pointer to a Hitidx object
** @@
******************************************************************************/
AjPHitidx  ajXyzHitidxNew(void)
{
    AjPHitidx ret  =NULL;

    AJNEW0(ret);

    ret->Id        =ajStrNew();
    ret->hptr       =NULL;
    ret->lptr       =NULL;
    
    return ret;
}





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
    if(len)
    {
	ret->seq_score    = ajFloatNewL((ajint)len);
	ajFloatPut(&ret->seq_score, len-1, (float)0.0);
	ret->post_similar = ajIntNewL((ajint)len);
	ajIntPut(&ret->post_similar , len-1, (int)0);
	ret->ncon_score   = ajFloatNewL((ajint)len);
	ajFloatPut(&ret->ncon_score , len-1, (float)0.0);
	ret->ccon_score   = ajFloatNewL((ajint)len);
	ajFloatPut(&ret->ccon_score  , len-1, (float)0.0);
	ret->nccon_score = ajIntNewL((ajint)len);
	ajIntPut(&ret->nccon_score, len-1, (int)0);
	ret->combi_score  = ajIntNewL((ajint)len);
	ajIntPut(&ret->combi_score, len-1, (int)0);
    }
    else
	ajWarn("Zero sized arg passed to ajXyzScorealgNew.\n");
    
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

    if(n)
	AJCNEW0(ret->Res, n);
    else
	ajWarn("Zero sized arg passed to ajXyzVdwallNew.\n");

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

    if(n)
    {
	AJCNEW0(ret->Atm, n);
	for(x=0;x<n;++x)
	    ret->Atm[x]=ajStrNew();

	AJCNEW0(ret->Rad, n);
    }
    else
	ajWarn("Zero sized arg passed to ajXyzVdwresNew.\n");


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

    if(dim)
    {
	/* Create the SQUARE contact map */
	ret->Mat = ajInt2dNewL((ajint)dim);
	for(z=0;z<dim;++z)
	    ajInt2dPut(&ret->Mat, z, dim-1, (ajint) 0);
    }
   else
       ajWarn("Zero sized arg passed to ajXyzCmapNew.\n");


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

    if(n)
    {
	AJCNEW0(ret->Codes,n);
	for(i=0;i<n;++i)
	    ret->Codes[i] = ajStrNew();

	AJCNEW0(ret->Seqs,n);
	for(i=0;i<n;++i)
	    ret->Seqs[i] = ajStrNew();
    }

   else
       ajWarn("Zero sized arg passed to ajXyzScopalgNew.\n");    


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
    ret->Typeobj     = ajStrNew();
    ret->Typesbj     = ajStrNew();
    ret->Alg         = ajStrNew();
    ret->Start       =0;
    ret->End         =0;
    ret->Group       =0;
    ret->Rank        =0;
    ret->Score       =0;    
    ret->Eval        =0;
    
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
    ret->Typeobj   = ajStrNew();
    ret->Typesbj   = ajStrNew();
    ret->Alg         = ajStrNew();
    ret->Start     =0;
    ret->End       =0;
    ret->Group     =0;
    ret->Rank      =0;
    ret->Score     =0;    
    ret->Eval      =0;

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

    if(n)
    {
	AJCNEW0(ret->hits,n);
	for(i=0;i<n;++i)
	    ret->hits[i] = ajXyzHitNew();
    }	
    else
	ajWarn("Zero sized arg passed to ajXyzHitlistNew.\n");

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

    if(chains)
    {	
	AJCNEW0(ret->Chains,chains);
	for(i=0;i<chains;++i)
	    ret->Chains[i] = ajXyzChainNew();
    }
   else
       ajWarn("Zero sized arg passed to ajXyzPdbNew.\n");
    

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
   else
       ajWarn("Zero sized arg passed to ajXyzScopNew.\n");


    ret->N = chains;

    return ret;
}


/* ==================================================================== */
/* ========================= Destructors ============================== */
/* ==================================================================== */




/* @func ajXyzSigposDel ******************************************************
**
** Destructor for Sigpos object.
**
** @param [w] pthis [AjPSigpos*] Sigpos object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzSigposDel(AjPSigpos *pthis)
{
    AJFREE((*pthis)->gsiz);
    AJFREE((*pthis)->gpen);
    AJFREE((*pthis)->subs);

    AJFREE(*pthis); 
    *pthis=NULL;
    
    return; 
}


/* @func ajXyzSigdatDel ******************************************************
**
** Destructor for Sigdat object.
**
** @param [w] pthis [AjPSigdat*] Sigdat object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzSigdatDel(AjPSigdat *pthis)
{
/*
    AJFREE((*pthis)->gsiz);
    AJFREE((*pthis)->gfrq);
    AJFREE((*pthis)->rids);
    AJFREE((*pthis)->rfrq);
*/
    ajIntDel(&(*pthis)->gsiz);
    ajIntDel(&(*pthis)->gfrq);
    ajIntDel(&(*pthis)->rfrq);
    ajChararrDel(&(*pthis)->rids);
    
    AJFREE(*pthis);    
    *pthis=NULL;

    return; 
}



/* @func ajXyzSignatureDel ***************************************************
**
** Destructor for Signature object.
**
** @param [w] pthis [AjPSignature*] Signature object pointer
**
** @return [void]
** @@
******************************************************************************/
void ajXyzSignatureDel(AjPSignature *pthis)
{
    ajint x=0;
    
    if(!(*pthis))
	return;
    
    if((*pthis)->dat)
	for(x=0;x<(*pthis)->npos; ++x)
	    if((*pthis)->dat[x])
		ajXyzSigdatDel(&(*pthis)->dat[x]);

    if((*pthis)->pos)
	for(x=0;x<(*pthis)->npos; ++x)
	    if((*pthis)->pos[x])
		ajXyzSigposDel(&(*pthis)->pos[x]);

    if((*pthis)->Class)
	ajStrDel(&(*pthis)->Class);
    if((*pthis)->Fold)
	ajStrDel(&(*pthis)->Fold);
    if((*pthis)->Superfamily)
	ajStrDel(&(*pthis)->Superfamily);
    if((*pthis)->Family)
	ajStrDel(&(*pthis)->Family);

    if((*pthis)->dat)
	AJFREE((*pthis)->dat);

    if((*pthis)->pos)
	AJFREE((*pthis)->pos);

    AJFREE(*pthis);    
    *pthis=NULL;

    return;
}



/* @func ajXyzHitidxDel ******************************************************
**
** Destructor for Hitidx object.
**
** @param [w] pthis [AjPHitidx*] Hitidx object pointer
**
** @return [void]
** @@
******************************************************************************/
void     ajXyzHitidxDel(AjPHitidx *pthis)
{
    ajStrDel(&(*pthis)->Id);

    AJFREE(*pthis);
    *pthis=NULL;
    
    return;
}





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
    ajFloatDel(&(*pthis)->seq_score);
    ajIntDel(&(*pthis)->post_similar);
    ajFloatDel(&(*pthis)->ncon_score);
    ajFloatDel(&(*pthis)->ccon_score);
    ajIntDel(&(*pthis)->nccon_score);
    ajIntDel(&(*pthis)->combi_score);

    AJFREE(*pthis);    
    *pthis=NULL;

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
    *pthis=NULL;

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
    *pthis=NULL;

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
    *pthis=NULL;

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
    *pthis=NULL;
    
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
    ajStrDel(&(*pthis)->Typeobj);
    ajStrDel(&(*pthis)->Typesbj);
    ajStrDel(&(*pthis)->Alg);

    AJFREE(*pthis);
    *pthis=NULL;
    
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
    ajStrDel(&(*pthis)->Typeobj);
    ajStrDel(&(*pthis)->Typesbj);
    ajStrDel(&(*pthis)->Alg);

    AJFREE(*pthis);
    *pthis=NULL;
    
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
    *pthis=NULL;
    
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
    AJFREE(pthis->Chains);

    AJFREE(pthis);
    pthis=NULL;

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

    ajStrDel(&pthis->Seq);
    ajListDel(&pthis->Atoms);

    AJFREE(pthis);
    pthis=NULL;

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
    pthis=NULL;

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
    pthis=NULL;

    return;
}



/* ==================================================================== */
/* ======================== Exported functions ======================== */
/* ==================================================================== */


/* @func ajXyzSignatureRead **************************************************
**
** Read the next Signature object from a file in embl-like format.
**
** @param [r] inf [AjPFile] Input file stream
** @param [w] thys [AjPSignature*] Signature object
**
** @return [AjBool] True on success
** @@
******************************************************************************/
AjBool ajXyzSignatureRead(AjPFile inf, AjPSignature *thys)
{
    static AjPStr line    =NULL;
    static AjPStr class   =NULL;
    static AjPStr fold    =NULL;
    static AjPStr super   =NULL;
    static AjPStr family  =NULL;
    
    AjBool ok             =ajFalse;
    ajint  npos           =0;   /* No. signature positions*/
    ajint  i              =0;   /* Loop counter*/
    ajint  n              =0;   /* Counter of signature positions*/
    ajint  nres           =0;   /* No. residues for a sig. position*/
    ajint  ngap           =0;   /* No. gaps for a sig. position*/
    ajint  wsiz           =0;   /* Windows size for a sig. position*/
    ajint  v1             =0;
    ajint  v2             =0;
    char   c1             ='\0';
    
    
    /*CHECK ARG'S */
    if(!inf)
	return ajFalse;
    

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
    ok=ajFileReadLine(inf,&line);

    while(ok && !ajStrPrefixC(line,"//"))
    {
	if(ajStrPrefixC(line,"XX"))
	{
	    ok = ajFileReadLine(inf,&line);
	    continue;
	}
	else if(ajStrPrefixC(line,"CL"))
	{
	    ajStrAssC(&class,ajStrStr(line)+3);
	    ajStrClean(&class);
	}
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
	else if(ajStrPrefixC(line,"NP"))
	{
	    ajFmtScanS(line, "NP%d", &npos);

	    /* Create signature structure */
	    (*thys)=ajXyzSignatureNew(npos);
	    ajStrAssS(&(*thys)->Class, class);
	    ajStrAssS(&(*thys)->Fold, fold);
	    ajStrAssS(&(*thys)->Superfamily, super);
	    ajStrAssS(&(*thys)->Family, family);
	}
	else if(ajStrPrefixC(line,"NN"))
	{
	    /* Increment position counter */
	    n++;

	    /* Safety check */
	    if(n>npos)
		ajFatal("Dangerous error in input file caught in "
			"ajXyzSignatureRead.\n Email jison@hgmp.mrc.ac.uk");
	}
	else if(ajStrPrefixC(line,"IN"))
	    {
		ajFmtScanS(line, "%*s %*s %d %*c %*s %d %*c %*s %d", 
			   &nres, &ngap, &wsiz);
	
		/* Create Sigdat structures and fill some elements*/
		(*thys)->dat[n-1]=ajXyzSigdatNew(nres, ngap);

		(*thys)->dat[n-1]->wsiz=wsiz;

		/* Skip 'XX' line */
		if(!(ok = ajFileReadLine(inf,&line)))
		    break;

		/* Read in residue data */
		for(i=0; i<(*thys)->dat[n-1]->nres; i++)
		{
		    if(!(ok = ajFileReadLine(inf,&line)))
			break;
		    ajFmtScanS(line, "%*s %c %*c %d", &c1,&v2);
		    ajChararrPut(&(*thys)->dat[n-1]->rids,i,c1);
		    ajIntPut(&(*thys)->dat[n-1]->rfrq,i,v2);
		}
		if(!ok) break;
	       

		/* Skip 'XX' line */
		if(!(ok = ajFileReadLine(inf,&line)))
		    break;

		/* Read in gap data */
		for(i=0; i<(*thys)->dat[n-1]->ngap; i++)
		{
		    if(!(ok = ajFileReadLine(inf,&line)))
			break;
		    ajFmtScanS(line, "%*s %d %*c %d", &v1,&v2);
		    ajIntPut(&(*thys)->dat[n-1]->gsiz,i,v1);
		    ajIntPut(&(*thys)->dat[n-1]->gfrq,i,v2);
		}
		if(!ok) break;
	    }

	ok = ajFileReadLine(inf,&line);
    }
    

    /* Return */
    if(!ok)
	return ajFalse;
    else
	return ajTrue;
}



/* @func ajXyzSignatureWrite *************************************************
**
** Write contents of a Signature object to an output file in embl-like format.
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] thys [AjPSignature] Signature object
**
** @return [void]
** @@
******************************************************************************/
AjBool ajXyzSignatureWrite(AjPFile outf, AjPSignature thys)
{ 
    ajint i,j;

    if(!outf || !thys)
	return ajFalse;


    ajFmtPrintF(outf,"CL   %S",thys->Class);
    ajFmtPrintSplit(outf,thys->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,thys->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\nNP   %d\n",thys->npos);


    for(i=0;i<thys->npos;++i)
    {
	ajFmtPrintF(outf,"XX\nNN   [%d]\n",i+1);
	ajFmtPrintF(outf,"XX\nIN   NRES %d ; NGAP %d ; WSIZ %d\nXX\n",
		    thys->dat[i]->nres, thys->dat[i]->ngap, thys->dat[i]->wsiz);

	for(j=0;j<thys->dat[i]->nres;++j)
	    ajFmtPrintF(outf,"AA   %c ; %d\n",
			(char)  ajChararrGet(thys->dat[i]->rids, j),
			(ajint) ajIntGet(thys->dat[i]->rfrq, j));
	ajFmtPrintF(outf,"XX\n");
	for(j=0;j<thys->dat[i]->ngap;++j)
	    ajFmtPrintF(outf,"GA   %d ; %d\n",
			(ajint) ajIntGet(thys->dat[i]->gsiz, j),
			(ajint) ajIntGet(thys->dat[i]->gfrq, j));
    }
    ajFmtPrintF(outf,"//\n");
    


    return ajTrue;
}





/* @func ajXyzSignatureCompile ***********************************************
**
** Calls ajXyzSigposNew to allocate an array of AjOSigpos objects within an 
** AjOSignature object, and then writes this array. A signature must have been 
** allocated by using the ajXyzSignatureNew function.
**
** @param [w] S      [AjPSignature] Signature object
** @param [r] gapo   [float]        Gap opening penalty
** @param [r] gape   [float]        Gap extension penalty
** @param [r] matrix [AjPMatrixf]   Residue substitution matrix
**
** @return [AjBool] True if array was written succesfully.
** @@
******************************************************************************/
AjBool ajXyzSignatureCompile(AjPSignature *S, float gapo, float gape, 
			     AjPMatrixf matrix)
{
    AjPSeqCvt         cvt=NULL;   /* Conversion array for AjPMatrixf */
    float           **sub=NULL;   /* Substitution matrix from AjPMatrixf */
    ajint               x=0;      
    ajint               y=0;      
    ajint               z=0;      
    AjBool           *tgap=NULL;   /* Temporary array of gap sizes. A cell 
				      == True if a gap is permissible */
    float            *tpen=NULL;   /* Temporary array of gap penalties */
    ajint              dim=0;      /* Dimension of tgap & tpen arrays */
    float              pen=0.0;    /* Gap penalty */
    ajint             ngap=0;      /* No. of gaps */
    ajint              div=0;      /* Used in calculating residue match values */
    
    

    /* CHECK ARGS */
    if(!(*S) || !((*S)->dat) || !((*S)->pos) || !matrix)
	return ajFalse;
    

    /* INITIALISE SUBSTITUTION MATRIX */
    sub  = ajMatrixfArray(matrix);
    cvt  = ajMatrixfCvt(matrix);    

    

    /* LOOP FOR EACH SIGNATURE POSITION */
    for(x=0; x<(*S)->npos; x++)
    {
	/* ALLOCATE TEMP. ARRAY OF GAP SIZES, OF SIZE == 
	   LARGEST GAP + WINDOW SIZE */
	/*      (*S)->dat[x]->gsiz[(*S)->dat[x]->ngap - 1] */
	dim = (ajIntGet((*S)->dat[x]->gsiz, (*S)->dat[x]->ngap - 1))
	    + (*S)->dat[x]->wsiz + 1;
	AJCNEW0(tgap, dim);
	AJCNEW0(tpen, dim);
	
	/*FILL TEMP ARRAYS */
	for(y=0; y<(*S)->dat[x]->ngap; y++)
	{
	    /*GAP NOT EXTENDED BY WINDOW */
	    tgap[(ajIntGet((*S)->dat[x]->gsiz, y))]=ajTrue;
	    tpen[(ajIntGet((*S)->dat[x]->gsiz, y))]=0;
	    

	    /*GAP IS EXTENDED BY WINDOW */
	    for(z=1; z<=(*S)->dat[x]->wsiz; z++)
	    {
		pen=gapo+gape*(z-1);
		
		/* A penalty has been assigned for this gap distance before*/
		if(tgap[(ajIntGet((*S)->dat[x]->gsiz, y))+z])
		{
		    if( pen < tpen[(ajIntGet((*S)->dat[x]->gsiz, y))+z])
			tpen[(ajIntGet((*S)->dat[x]->gsiz, y))+z]=pen;
		}
		/* We have not assigned a penalty to this gap distance before*/
		else
		{
		    tpen[(ajIntGet((*S)->dat[x]->gsiz, y))+z]=pen;
		    tgap[(ajIntGet((*S)->dat[x]->gsiz, y))+z]=ajTrue;
		}
		

		
		if( ajIntGet((*S)->dat[x]->gsiz, y)-z >= 0)
		{
		    /* A penalty has been assigned for this gap distance before*/
		    if(tgap[ajIntGet((*S)->dat[x]->gsiz, y)-z])
		    {
			if(pen < tpen[(ajIntGet((*S)->dat[x]->gsiz, y))-z])
			    tpen[(ajIntGet((*S)->dat[x]->gsiz, y))-z]=pen;
		    }
		    /* We have not assigned a penalty to this gap distance before*/
		    else
		    { 
			tpen[(ajIntGet((*S)->dat[x]->gsiz, y))-z]=pen;
			tgap[(ajIntGet((*S)->dat[x]->gsiz, y))-z]=ajTrue;
		    }
		}
	    }
	}
	

	/* ALLOCATE ARRAY OF Sigpos OBJECTS */
	for(ngap=0, y=0; y<dim; y++)
	    if(tgap[y])
		ngap++;
	(*S)->pos[x] = ajXyzSigposNew(ngap);



	/*ASIGN THE GAP DATA  */
	for(ngap=0, y=0; y<dim; y++)
	    if(tgap[y])
	    {
		(*S)->pos[x]->gsiz[ngap]=y;
		(*S)->pos[x]->gpen[ngap]=tpen[y];
		ngap++;
	    }
	/* CALCULATE RESIDUE MATCH VALUES */
	for(z=0;z<26; z++)
	{
	    for(div=0, y=0; y<(*S)->dat[x]->nres; y++)
	    {
		div+=(ajIntGet((*S)->dat[x]->rfrq, y));
		
		(*S)->pos[x]->subs[z] += 
		    (ajIntGet((*S)->dat[x]->rfrq, y)) * 
			sub[ajSeqCvtK(cvt,(char)((ajint)'A'+z))]
			    [ajSeqCvtK(cvt, ajChararrGet((*S)->dat[x]->rids, y))];
	    }
	    (*S)->pos[x]->subs[z] /= div;
	}
	
	

	/* FREE tgap & tpen ARRAYS */
	AJFREE(tgap);
	AJFREE(tpen);
}
    
    

    /* Return */
    return ajTrue;
}






/* @func ajXyzSignatureAlignSeq **********************************************
**
** Performs an alignment of a signature to a protein sequence. The signature
** must have first been compiled by calling the ajXyzSignatureCompile function.
** An AjOHit object is written.
**
** @param [r] S      [AjPSignature] Signature object
** @param [r] seq    [AjPSeq]       Protein sequence
** @param [r] matrix [AjPMatrixf]   Residue substitution matrix
** @param [w] hit    [AjPHit*]      Hit object pointer
** @param [w] nterm  [ajint]        N-terminal matching option
**
** @return [AjBool] True if a signature-sequence alignment was successful and 
** the Hit object was written.  Returns False if there was an internal error, 
** bad arg's etc. or in cases where a sequence is rejected because of 
** N-terminal matching options). 
** @@
******************************************************************************/
AjBool        ajXyzSignatureAlignSeq(AjPSignature S, AjPSeq seq, AjPHit *hit, 
				     ajint nterm)
{
    AjPStr            P=NULL; 
    ajint             gidx=0;	  /*Index into gap array */
    ajint             glast=0;	  /*Index of last gap to try*/
    ajint             nres=0;	  /*No. of residues in protein*/
    ajint             nresm1=0;	  /*== nres-1*/
    static AjPSigcell path=NULL;  /*Path matrix as 1D array */
    ajint             dim=0;      /*Dimension of 1D path matrix == nres 
				   * S->npos */
    static char      *p=NULL;	  /*Protein sequence*/
    ajint             start=0;	  /*Index into path matrix of first position 
				    in the previous row to grow an alignment 
				    from*/
    ajint             startp=0;	  /*Index into protein sequence for this 
				    position*/
    ajint             stop=0;	  /*Index into path matrix of last position in 
				    previous row to grow an alignment from*/
    ajint             this=0;	  /*Index into path matrix for current row*/
    ajint             last=0;	  /*Index into path matrix for last row*/
    ajint             thisp=0;	  /*Index into protein sequence for current row*/
    ajint             lastp=0;	  /*Index into protein sequence for last row*/
    ajint             sidx=0;	  /*Index into signature*/
    float             val=0;	  /*Value for signature position:residue match*/
    float             mval=0;	  /*Max. value of matches of last signature 
				    position:protein sequence*/
    ajint             max=0;	  /*Index into path matrix for cell with mval */
    ajint             maxp=0;     /*Index into protein sequence for path matrix 
				    cell with mval */
    static char      *alg=NULL;   /*String for alignment*/
    ajint             cnt;        /*A loop counter */
    ajint             mlen=0;     /*Min. possible length of the alignment of the 
				    signature*/
    float             score=0;    /*Score for alignment */
    
    



    /* CHECK ARGS AND CREATE STRINGS */
    if(!S || !seq || !hit)
	return ajFalse;
    

    P= ajSeqStr(seq);
    /*Check protein sequence contains alphabetic characters only */
    if(!ajStrIsAlpha(P))
	return ajFalse;


    /* INITIALISE VARIABLES */
    nres   = ajStrLen(P);    /* No. columns in path matrix */
    nresm1 = nres-1;         /* Index of last column in path matrix */
    dim = nres * S->npos;



    /* ALLOCATE MEMORY */
    /*First time the function is called */
    if(!path)
    {
	/* CREATE PATH MATRIX */
	AJCNEW(path, dim);

	/* CREATE ALIGNMENT AND PROTEIN SEQUENCE STRINGS */
	alg = AJALLOC((nres*sizeof(char))+1);
	p = AJALLOC((nres*sizeof(char))+1);
    }	
    else 
    {
	/* CREATE PATH MATRIX */
	if(dim > (ajint) sizeof(path)/sizeof(AjOSigcell))
	    AJCRESIZE(path, dim);

	/* CREATE ALIGNMENT AND PROTEIN SEQUENCE STRINGS */
	if((nres) > (ajint) sizeof(alg)/sizeof(char))
	{
	    AJCRESIZE(alg, nres+1);
	    AJCRESIZE(p, nres+1);
	}
    }



    /* INITIALISE PATH MATRIX
       Only necessary to initilise <try> element to ajFalse*/
    for(cnt=0;cnt<dim;cnt++)
	path[cnt].try = ajFalse;
    
    


    /*COPY SEQUENCE AND CONVERT TO UPPER CASE, OVERWRITE ALIGNMENT STRING */
    strcpy(p, ajStrStr(P));
    ajCharToUpper(p);
    for(cnt=0;cnt<nres;cnt++)
	alg[cnt]='-';
    alg[cnt]='\0';
    




    switch(nterm)
    {
    case 1:
	/*The first position 
	  can be aligned anywhere in the protein sequence, so long
	  as there is sufficient space to align the rest of the 
	  signature (this is fast, but might not be ideal, e.g. for
          detection of fragments.).  Note that gap distance for 
	  first signature position is ignored. Note the function 
	  will return if the whole of the signature can not
	  be aligned
	  This is the RECOMMENDED option*/
	
	/*Find last gap to try for first sig. position and return an 
	  error if first sig. position cannot be fitted */
	mlen=1;   /*For first signature position*/
	for(sidx=1;sidx<S->npos;sidx++)
	    mlen+=(1+S->pos[sidx]->gsiz[0]);
	start=startp=0;
	stop=nres-mlen;
	if(stop<0)
	    return ajFalse;    
	
	/*	Assign path matrix for row 0. 'this' is index into both path
		matrix and protein sequence in this case.  There is no gap 
		penalty for the first position.
		Assign indeces into path matrix of start and stop positions for row 0.  
		Assign index into protein sequence for start position.*/
	for(this=0;this<=stop;this++)
	{
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] - (ajint)'A')];
	    path[this].prev=0;
	    path[this].try=ajTrue;
	}
	break;
	
    case 2:
	/*The first position 
	  can be aligned anywhere in the protein sequence (this is
	  slower, but means that, e.g. high scoring alignments that
	  are lacking C-terminal signature positions, will not be 
	  discarded.    */
	
	for(this=0;this<nres;this++)
	{
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] - (ajint)'A')];
	    path[this].prev=0;
	    path[this].try=ajTrue;
	}
	start=startp=0;
	stop=nresm1;
	break;
	
    case 3:
	/* Use empirical gaps only, rather than allowing the 
	   first signature positions to be aligned to anywhere
	   within the sequence */
	
	for(glast=S->pos[0]->ngaps-1; glast>=0; glast--)
	    if(S->pos[0]->gsiz[glast]<nresm1)
		break;
	if(glast==-1)
	    return ajFalse;
	
	
	
	for(gidx=0; gidx<=glast; ++gidx)
	{	
	    this=S->pos[0]->gsiz[gidx];
	    path[this].val=S->pos[0]->subs[(ajint) ((ajint)p[this] - (ajint)'A')];
	    path[this].prev=0;
	    path[this].try=ajTrue;
	}
	startp=start=S->pos[0]->gsiz[0];
	stop=S->pos[0]->gsiz[gidx-1];
	break;
	
    default:
	ajFatal("Bad nterm value for ajXyzSignatureAlignSeq. This should never happen.\n");
	break;
    }
    



    
    /*Assign path matrix for other rows */
    /*Loop for each signature position, beginning at row 1*/
    for(sidx=1;sidx<S->npos;sidx++)
    {
	/*Loop for permissible region of previous row*/
	for(last=start, lastp=startp; last<=stop; last++, lastp++)
	{
	    if(path[last].try==ajFalse)
		continue;

	    /*Loop for each permissible gap in current row*/
	    for(gidx=0;gidx<S->pos[sidx]->ngaps;gidx++)
	    {
		if((thisp=lastp+S->pos[sidx]->gsiz[gidx]+1)>nresm1)
		    break;
		

		this=last+nres+S->pos[sidx]->gsiz[gidx]+1;
		val=path[last].val +
		    S->pos[sidx]->subs[(ajint) (p[thisp] - (ajint)'A')] -
			S->pos[sidx]->gpen[gidx];
		

		if((path[this].try==ajTrue)&&(val > path[this].val))
		{
		    path[this].val=val;
		    path[this].prev=last;
		    continue;
		}				
		/*The cell hasn't been visited before so give it a score*/
		if(path[this].try==ajFalse)
		{
		    path[this].val=val;
		    path[this].prev=last;
		    path[this].try=ajTrue;
		    continue;
		}	
	    }
	}
    	
	/*We cannot accomodate the next position*/
	if((startp+=(1+S->pos[sidx]->gsiz[0]))>=nresm1)
	    break;
	start+=(nres+1+S->pos[sidx]->gsiz[0]);
	/*last gives (index into last position tried)+1  because
	  of loop increment.  */
	
	stop=this;
    }


    /* Find index into protein sequence and number of signature position 
       (row) corresponding to the last cell in the path matrix which was
       assigned */
    thisp= this - (ajint) ((sidx=(ajint)floor((double)(this/nres))) * nres);
    


    /*Find maximal value in this row ... give mval a silly value
     so it is assigned at least once*/
    for(mval=-1000000 ; thisp>=0; this--, thisp--)
    {
	if(path[this].try==ajFalse)
	    continue;
	if(path[this].val > mval)
	{
	    mval=path[this].val;
	    max=this;
	    maxp=thisp;
	}
    }


    /*Assign score for alignment*/
    score=mval; 
    score /= S->npos;


    /* Backtrack through matrix */
    alg[maxp]='*';
    for(this=path[max].prev, score=path[max].val; sidx>0; this=path[this].prev)
	{
	    thisp= this - (ajint) ((sidx=(ajint)floor((double)(this/nres))) 
				   * nres);
	    alg[thisp]='*';
	}


    /* Write hit structure */
    ajStrAssC(&(*hit)->Alg, alg);
    ajStrAss(&(*hit)->Seq, P);
    (*hit)->Start=thisp;
    (*hit)->End=maxp;
    ajStrAss(&(*hit)->Id, ajSeqGetAcc(seq));
    (*hit)->Score=score;
    

    /* Clean up and return */
    return ajTrue;
}



/* @func ajXyzScophitsOverlap ****************************************************
**
** Checks for overlap between two hits.
**
** @param [r] h1  [AjPHit]     Pointer to hit object 1
** @param [r] h2  [AjPHit]     Pointer to hit object 2
** @param [r] n   [ajint]      Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
******************************************************************************/
AjBool        ajXyzScophitsOverlap(AjPScophit h1, AjPScophit h2, ajint n)
{
    if( (((h1->End - h2->Start + 1)>=n) && (h2->Start >= h1->Start)) ||
       (((h2->End - h1->Start + 1)>=n) && (h1->Start >= h2->Start)))
	return ajTrue;
    else 
	return ajFalse;
}




/* @func ajXyzHitsOverlap ****************************************************
**
** Checks for overlap between two hits.
**
** @param [r] h1  [AjPHit]     Pointer to hit object 1
** @param [r] h2  [AjPHit]     Pointer to hit object 2
** @param [r] n   [ajint]      Threshold number of residues for overlap
**
** @return [AjBool] True if the overlap between the sequences is at least as 
** long as the threshold. False otherwise.
** @@
******************************************************************************/
AjBool        ajXyzHitsOverlap(AjPHit h1, AjPHit h2, ajint n)
{
    if( (((h1->End - h2->Start + 1)>=n) && (h2->Start >= h1->Start)) ||
       (((h2->End - h1->Start + 1)>=n) && (h1->Start >= h2->Start)))
	return ajTrue;
    else 
	return ajFalse;
}



/* @func ajXyzSignatureAlignWrite ********************************************
**
** Writes the alignments of a Signature to a list of AjOHit objects to an output 
** file. This is intended for displaying the results from scans of a signature 
** against a protein sequence database. 
**
** @param [w] outf [AjPFile] Output file stream
** @param [r] hits [AjPHitlist] Hitlist objects with hits from scan
**
** @return [AjBool] True if file was written
** @@
******************************************************************************/
AjBool        ajXyzSignatureAlignWrite(AjPFile outf, AjPSignature sig, 
				       AjPHitlist hits)
{
    /*A line of the alignment (including accession number, a space and the 
      sequence) in the output file is 70 characters long. An index number is 
      also printed after this 70 character field.*/
    ajint  wid1=0;     /*Temp. width of Accession Number */
    ajint  mwid1=0;    /*Max. width of Accession Number or the string "Number". 
			 This is the field width the accession numbers will be 
			 printed into */
    ajint  mwid2=0;    /*Width of region to print sequence into*/
    ajint  len=0;      /*Temp. length of sequence*/
    ajint  mlen=0;     /*Max. length of sequence*/
    char   *ptrp=NULL; /*Pointer to sequence string*/ 
    char   *ptrs=NULL; /*Pointer to alignment string */ 
    ajint  idx=0;      /*Start position for printing*/
    ajint  niter=0;    /*No. iterations of loop for printing out sequence blocks*/
    ajint  fwid1=70;   /*Including accession number, a space, 7 characters 
			 for the first index number, and the sequence*/
    ajint  fwid2=7;    /*Field width for the first index number*/
    ajint  num=0;      /*Index number for alignment*/
    ajint  y=0;        /*Loop counter*/
    ajint  x=0;        /*Loop counter*/
    

    /*Check args*/
    if(!outf || !hits || !sig)
	return ajFalse;

    /*Cycle through hits to find longest width of accession number*/
    for(len=0, mlen=0, wid1=0, mwid1=0, x=0;
	x<hits->N; 
	x++)
    {
	if((wid1=MAJSTRLEN(hits->hits[x]->Id))>mwid1)
	    mwid1=wid1; 
	if((len=MAJSTRLEN(hits->hits[x]->Seq))>mlen)
	    mlen=len;
    }

    /*Assign field widths and number of iterations for printing*/
    if((wid1=strlen("SIGNATURE"))>mwid1)
	mwid1=wid1;
    mwid1++;   /*A space*/
    mwid2=fwid1-fwid2-mwid1;
    niter=(ajint)ceil( ((double)mlen/(double)mwid2));
    

    /*Print header info and SCOP classification records of signature */
    ajFmtPrintF(outf, "DE   Results of signature search\nXX\n");
    ajFmtPrintF(outf,"CL   %S",sig->Class);
    ajFmtPrintSplit(outf,sig->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,sig->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,sig->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\n");
    

    printf("hits: %d\n", hits->N);
    

    /*Main loop for printing alignment*/
    for(num=0, idx=0, y=0;y<niter;y++)
    {
	num+=mwid2;
		

	/*Loop for each protein in Hitlist*/
	for(x=0;x<hits->N; x++)
	{
	    /*Get pointer to sequence & alignment string*/
	    ptrp=ajStrStr(hits->hits[x]->Seq);
	    ptrs=ajStrStr(hits->hits[x]->Alg);

	    /*There is some of the sequence left to print*/
	    if(idx<MAJSTRLEN(hits->hits[x]->Seq))
		{
		    ajFmtPrintF(outf,"%-*S%-*d%-*.*s %d\n", 
				mwid1, hits->hits[x]->Id, fwid2, 
				(num-mwid2+1), mwid2, mwid2, ptrp+idx, num);
		    ajFmtPrintF(outf,"%-*s%-*c%-*.*s\n", 
				mwid1, "SIGNATURE", fwid2, '-', mwid2, 
				mwid2, ptrs+idx);
		}
	    
	    /*We have printed all the sequence already*/
	    else
	    {
		ajFmtPrintF(outf,"%-*S%-*d%-*.*s %d\n", 
			    mwid1, hits->hits[x]->Id, fwid2,  
			    (num-mwid2+1), mwid2, mwid2, " ", num);
		ajFmtPrintF(outf,"%-*s%-*c%-*.*s\n", 
			    mwid1, "SIGNATURE", fwid2, '-', mwid2, 
			    mwid2, " " );
	    }
	}
	idx+=mwid2;


	/*Print spacer*/
	ajFmtPrintF(outf, "XX\n");
    }	 

    
    /*Print tail info*/
    ajFmtPrintF(outf, "//\n");

    
    /*Tidy up and return */
    return ajTrue;
}





/* @func ajXyzSignatureHitsWrite *********************************************
**
** Writes a list of AjOHit objects to an output file. This is intended for 
** displaying the results from scans of a signature against a protein sequence
** database.
**
** @param [w] outf [AjPFile]      Output file stream
** @param [w] sig  [AjPSignature] Signature object
** @param [r] hits [AjPHitlist]   Hitlist objects with hits from scan
**
** @return [AjBool] True if file was written
** @@
******************************************************************************/
AjBool        ajXyzSignatureHitsWrite(AjPFile outf, AjPSignature sig, 
				      AjPHitlist hits)
{
    ajint  x=0;
    
    
    /*Check args*/
    if(!outf || !hits || !sig)
	return ajFalse;

    
    /*Print header info*/
    ajFmtPrintF(outf, "DE   Results of signature search\nXX\n");


    /*Print SCOP classification records of signature */
    ajFmtPrintF(outf,"CL   %S",sig->Class);
    ajFmtPrintSplit(outf,sig->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,sig->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,sig->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\n");
    
    
    /*Loop through list and print out data*/
    for(x=0;x<hits->N; x++)
	ajFmtPrintF(outf, "HI  %-6d%-10S%-3d%-10S%-10S%-7.1f%-7.3f\n", 
		    x+1, hits->hits[x]->Id, hits->hits[x]->Group, 
		    hits->hits[x]->Typeobj, hits->hits[x]->Typesbj, 
		    hits->hits[x]->Score, hits->hits[x]->Eval);

    /*Print tail info*/
    ajFmtPrintF(outf, "XX\n//\n");
    
    
    /*Clean up and return*/ 
    return ajTrue;
}





/* @func ajXyzSignatureAlignSeqall *******************************************
**
** Aligns a signature to a set of sequences and writes a Hitlist object with 
** the results. The top-scoring <n> hits are written. The signature must have 
** first been compiled by calling the ajXyzSignatureCompile function.
** Memory for an AjOHitlist object must be allocated beforehand by using the 
** Hitlist constructor with an arg. of 0.
**
** @param [r] sig    [AjPSignature] Signature object
** @param [r] db     [AjPSeqall]    Protein sequences
** @param [w] n      [ajint]        Number of top-scoring hits to store
** @param [w] hits   [AjPHitlist*]  Hitlist object pointer
** @param [w] nterm  [ajint]        N-terminal matching option
**
** @return [AjBool] True if Hitlist object was written succesfully.
** @@
******************************************************************************/
AjBool ajXyzSignatureAlignSeqall(AjPSignature sig, AjPSeqall db, ajint n, 
				 AjPHitlist *hits, ajint nterm)
{
    ajint        nhits =0;       /* Counter of number of hits */
    AjPHit       hit =NULL;	 /* The current hit */    
    AjPHit       ptr=NULL;	 /* Temp. pointer to hit structure */    
    AjPSeq       seq=NULL;       /* The current protein sequence from db */ 
    AjPList      listhits=NULL;  /* Temp. list of hits */



    /* Check args */
    if(!sig || !db || !hits)
    {
	ajWarn("NULL arg passed to ajXyzSignatureAlignSeqall");
	return ajFalse;
    }
    
    printf("1\n");
    fflush(stdout);
    

    /* Memory allocation*/
    listhits = ajListNew();
/*    seq=ajSeqNew();    */

printf("2\n");
    fflush(stdout);

    /*Initialise Hitlist object with SCOP records from Signature*/
    ajStrAss(&(*hits)->Class, sig->Class);
    ajStrAss(&(*hits)->Fold, sig->Fold);
    ajStrAss(&(*hits)->Superfamily, sig->Superfamily);
    ajStrAss(&(*hits)->Family, sig->Family);

        
printf("3\n");
    fflush(stdout);

    /*Search the database*/
    while(ajSeqallNext(db,&seq))
    {

printf("4\n");
    fflush(stdout);

	/* Allocate memory for hit */
	hit=ajXyzHitNew();
	

printf("5\n");
    fflush(stdout);

	if(!ajXyzSignatureAlignSeq(sig, seq, &hit, nterm))
	{	
	    ajXyzHitDel(&hit);
	    continue;
	}
	else
	    nhits++;
	

printf("6\n");
    fflush(stdout);

	/* Push hit onto list */
	ajListPush(listhits,(AjPHit) hit);
	

printf("7\n");
    fflush(stdout);

	if(nhits>n)
	{	

printf("8\n");
    fflush(stdout);

	    /* Sort list according to score, highest first*/
	    ajListSort(listhits, ajXyzCompScore);
	 
printf("9\n");
    fflush(stdout);   

	    /* Pop the hit (lowest scoring) from the bottom of the list */
	    ajListPopEnd(listhits, (void *) &ptr);
	    ajXyzHitDel(&ptr);

printf("10\n");
    fflush(stdout);

	}
    }
    

printf("11 nhits: %d\n", nhits);
    fflush(stdout);
    
    /* Sort list according to score, highest first*/
    ajListSort(listhits, ajXyzCompScore);


printf("12\n");
    fflush(stdout);

    /* Convert list to array within Hitlist object */
    ajListToArray(listhits, (void ***)  &(*hits)->hits);
    (*hits)->N = nhits;
    

printf("13\n");
    fflush(stdout);

    /*Tidy up and return */
    ajListDel(&listhits);
    ajSeqDel(&seq);
    return ajTrue;
}



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
	/* Unknown or Zero sized chains have already been checked for
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
	/* Unknown or Zero length chains have already been checked for
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






/* @func ajXyzScopalgWrite ***************************************************
**
** Write a Scopalg object to file in embl-like format.
** 
** @param [r] outf     [AjPFile] Output file stream
** @param [w] thys     [AjPScopalg*]  Scopalg object
**
** @return [AjBool] True on success (an alignment was written)
** @@
******************************************************************************/
AjBool   ajXyzScopalgWrite(AjPFile outf, AjPScopalg *thys)
{
    /* JC Write this function and modify scopalign.c etc. Not urgent. */
    return ajFalse;
    
}


/* @func ajXyzScopalgGetseqs *************************************************
**
** Read a Scopalg object and writes an array of AjPStr containing the sequences
** without gaps.
** 
** @param [r] thys     [AjPScopalg]  Scopalg object
** @param [w] arr      [AjPStr **]   Array of AjPStr 
**
** @return [ajint] Number of sequences read
** @@
******************************************************************************/
ajint ajXyzScopalgGetseqs(AjPScopalg thys, AjPStr **arr)
{
    ajint i;
        
    /*Check args*/
    if(!thys)
	{
	    ajWarn("Null args passed to ajXyzScopalgGetseqs");
	    return 0;
	}
    
    
    *arr = (AjPStr *) AJCALLOC0(thys->N, sizeof(AjPStr));
    
    for(i=0;i<thys->N;++i)
    {
	(*arr)[i] = ajStrNew();

	ajStrAssS(&((*arr)[i]), thys->Seqs[i]);
	
	ajStrDegap(&((*arr)[i]));
	
/*	ajFmtPrint("i:%d %S\n", i, (*arr)[i]); */
	
    }
    return thys->N;


    /*
    AJCNEW0(*arr,thys->N);

    for(i=0;i<thys->N;++i)
    {
	*arr[i] = ajStrNew();

	ajStrAssS(&(*arr[i]), thys->Seqs[i]); 

	ajStrDegap(&(*arr[i]));

    }
    return thys->N;
    */
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
    AjPStr  codetmp         =NULL;     /* Id code of sequence */
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
	codetmp = ajStrNew();
    }

    
    /* Create new lists */
    list_seqs = ajListstrNew();
    list_codes = ajListstrNew();


    /* Start of code for reading input file */
    /*Ignore everything up to first line beginning with 'Number'*/
    /*
    while(ajFileReadLine(inf,&line))
    {
	if(ajStrPrefixC(line,"Number"))
	    break;
    }
*/

    /* Read the rest of the file */
    while(ajFileReadLine(inf,&line))
    {
	/* Ignore 'Number' lines */
	if((ajStrPrefixC(line,"Number")))
	    continue;
    	else if(ajStrPrefixC(line,"CL"))
	    {
		ajStrAssC(&class,ajStrStr(line)+3);
		ajStrClean(&class);
	    }
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

    ajStrDel(&seq1);
    
    if(!cnt)
    {
	ajWarn("No sequences in alignment !\n");
	ajListstrDel(&list_seqs); 
	return ajFalse;
    }
    


    /* Allocate memory for Scopalg structure */
    (*thys) = ajXyzScopalgNew(cnt);



    /* Assign SCOP records */
    ajStrAss(&(*thys)->Class,class);
    ajStrAss(&(*thys)->Fold,fold);
    ajStrAss(&(*thys)->Superfamily,super);
    ajStrAss(&(*thys)->Family,family); 
    


    
    /* Assign sequences and free memory */
    for(x=0; x<cnt; x++)
    {
	ajStrAssS(&(*thys)->Seqs[x],arr_seqs[x]); 
	ajStrDel(&arr_seqs[x]);

    }
    AJFREE(arr_seqs);
    

    /* Assign width */
    (*thys)->width = ajStrLen((*thys)->Seqs[0]);


    for(x=0; ajListstrPop(list_codes,&codetmp); x++)
    {
	ajStrAssS(&(*thys)->Codes[x],codetmp);
	ajStrDel(&codetmp);
    }
    
/*JC	ajStrAssS(&(*thys)->Codes[x],arr_seqs[x]); 	 */




    /* Assign Post_similar line */
    ajStrAssS(&(*thys)->Post_similar,postsim); 



    /* Clean up */
    ajListstrDel(&list_seqs); 
    ajListstrDel(&list_codes); 
    


    /* Return */
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
	    {
		ajStrAssC(&class,ajStrStr(line)+3);
		ajStrClean(&class);
	    }
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
	else if(ajStrPrefixC(line,"AC"))
	    {
		ajStrAssC(&(*thys)->hits[n-1]->Id,ajStrStr(line)+3);
		ajStrClean(&(*thys)->hits[n-1]->Id);
	    }
	else if(ajStrPrefixC(line,"TY"))
	    {
		ajStrAssC(&(*thys)->hits[n-1]->Typeobj,ajStrStr(line)+3);	
		ajStrClean(&(*thys)->hits[n-1]->Typeobj);		
	    }
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
	ajFmtPrintF(outf, "%-5s%S\n", "AC", thys->hits[x]->Id);
	ajFmtPrintF(outf, "XX\n");
	ajFmtPrintF(outf, "%-5s%S\n", "TY", thys->hits[x]->Typeobj);
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


/* @func ajXyzCmapReadI ****************************************************
**
** Read a Cmap object from a file in embl-like format. Takes the chain 
** identifier as an integer.
** 
** @param [r] inf     [AjPFile]  Input file stream
** @param [r] chn     [ajint]    Chain number
** @param [r] mod     [ajint]    Model number
** @param [w] thys    [AjPCmap*] Pointer to Cmap object
**
** @return [AjBool] True on success (an object read)
** @@
******************************************************************************/
AjBool   ajXyzCmapReadI(AjPFile inf, ajint chn, ajint mod, AjPCmap *thys)
{
    if(ajXyzCmapRead(inf, CMAP_MODE_I, chn, mod, thys))
	return ajTrue;
    else 
	return ajFalse;
}



/* @func ajXyzCmapReadC ****************************************************
**
** Read a Cmap object from a file in embl-like format. Takes the chain 
** identifier as a character.
** 
** @param [r] inf     [AjPFile]  Input file stream
** @param [r] chn     [char]     Chain number
** @param [r] mod     [ajint]    Model number
** @param [w] thys    [AjPCmap*] Pointer to Cmap object
**
** @return [AjBool] True on success (an object read)
** @@
******************************************************************************/
AjBool   ajXyzCmapReadC(AjPFile inf, char chn, ajint mod, AjPCmap *thys)
{
    if(ajXyzCmapRead(inf, CMAP_MODE_C, (ajint)chn, mod, thys))
	return ajTrue;
    else 
	return ajFalse;
}




/* @func ajXyzCmapRead ****************************************************
**
** Read a Cmap object from a file in embl-like format. This is not usually
** called by the user, who uses ajXyzCmapReadI or ajXyzCmapReadC instead.
** 
** @param [r] inf     [AjPFile]  Input file stream
** @param [r] mode    [ajint]    Mode, either CMAP_MODE_I (treat chn arg as  
** an integer) or CMAP_MODE_C (treat chn arg as a character)
** @param [r] chn     [ajint]    Chain identifier / number
** @param [r] mod     [ajint]    Model number
** @param [w] thys    [AjPCmap*] Pointer to Cmap object
**
** @return [AjBool] True on success (an object read)
** @@
******************************************************************************/
AjBool   ajXyzCmapRead(AjPFile inf, ajint mode, ajint chn, ajint mod, AjPCmap *thys)
{	
    static   AjPStr line    =NULL;   /* Line of text */
    static   AjPStr temp_id =NULL;   /* Temp location for protein id */
    
    ajint    num_res   	    =0;      /* No. of residues in domain */	
    ajint    num_con   	    =0;      /* Total no. of contacts in domain */	
    ajint    x		    =0;      /* No. of first residue making contact */	
    ajint    y              =0;      /* No. of second residue making contact */	
    ajint    md             =-1;     /* Model number */
    ajint    cn             =-1;     /* Chain number */
    char     chnid          ='.';    /* Temp. chain identifier*/
    AjBool   idok           =ajFalse; /* If the required chain has been found */
    

    /* Check args */	
    if(!inf)
    {	
	ajWarn("Invalid args to ajXyzCmapRead");	
	return ajFalse;
    }


    /* Convert '_' chain identifiers to '.' if necessary */
    if(mode==CMAP_MODE_C)
	if(chn=='_')
	    chn='.';
    
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
	else if((ajStrPrefixC(line, "IN")) && (md==mod))
	{
	    ajFmtScanS(line, "%*s %*s %c; %*s %d; %*s %d;", 
		       &chnid, &num_res, &num_con);
	    
	    if(((cn==chn)&&(mode==CMAP_MODE_I)) ||
	       ((toupper(chnid)==toupper((char)chn))&&(mode==CMAP_MODE_C)))
	    {
		idok=ajTrue;
		
		/* Allocate contact map and write values */
		(*thys)=ajXyzCmapNew(num_res);
		(*thys)->Ncon = num_con;
		ajStrAssS(&(*thys)->Id, temp_id);
	    }
	}
    
	/* Read and parse residue contacts */
	else if((ajStrPrefixC(line, "SM")) && (md==mod) && (idok))
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
** @return [AjBool] True on success (an object read)
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
	/* Parse NR line */
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




/* @func ajXyzScophitCopy ******************************************************
**
** Copies the contents from one Scophit object to another.
**
** @param [w] to   [AjPScophit*] Scophit object pointer 
** @param [w] from [AjPScophit] Scophit object 
**
** @return [AjBool] True if copy was successful.
** @@
******************************************************************************/
AjBool ajXyzScophitCopy(AjPScophit *to, AjPScophit from)
{
    /* Check args */
    if(!(*to) || !from)
	return ajFalse;

    ajStrAss(&(*to)->Class, from->Class);
    ajStrAss(&(*to)->Fold, from->Fold);
    ajStrAss(&(*to)->Superfamily, from->Superfamily);
    ajStrAss(&(*to)->Family, from->Family);
    ajStrAss(&(*to)->Seq, from->Seq);
    ajStrAss(&(*to)->Id, from->Id);
    ajStrAss(&(*to)->Typeobj, from->Typeobj);
    ajStrAss(&(*to)->Typesbj, from->Typesbj);
    ajStrAss(&(*to)->Alg, from->Alg);
    (*to)->Start = from->Start;
    (*to)->End = from->End;
    (*to)->Group = from->Group;
    (*to)->Rank = from->Rank;
    (*to)->Score = from->Score;
    (*to)->Eval = from->Eval;

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
	    ajStrAss(&scophit->Typeobj, hitlist->hits[x]->Typeobj);
	    ajStrAss(&scophit->Typesbj, hitlist->hits[x]->Typesbj);
	    ajStrAss(&scophit->Alg, hitlist->hits[x]->Alg);
	    scophit->Start = hitlist->hits[x]->Start;
	    scophit->End = hitlist->hits[x]->End;
	    scophit->Group = hitlist->hits[x]->Group;
	    scophit->Rank = hitlist->hits[x]->Rank;
	    scophit->Score = hitlist->hits[x]->Score;
	    scophit->Eval = hitlist->hits[x]->Eval;
	    
	    
	    /* Push scophit onto list */
	    ajListPushApp(*out,scophit);
	}
    }	
    

    /* Clean up and return */
    ajListIterFree(iter);	

    return ajTrue;
}




/* @func ajXyzCompScore ******************************************************
**
** Function to sort AjOHit objects by score record. Usually called by 
** ajListSort.
**
** @param [r] hit1  [const void*] Pointer to AjOHit object 1
** @param [r] hit2  [const void*] Pointer to AjOHit object 2
**
** @return [ajint] 1 if score1<score2, 0 if score1==score2, else -1.
** @@
******************************************************************************/
ajint ajXyzCompScore(const void *hit1, const void *hit2)
{
    AjPHit p  = NULL;
    AjPHit q  = NULL;

    p = (*(AjPHit*)hit1);
    q = (*(AjPHit*)hit2);
    
    if(p->Score > q->Score)
	return -1;
    else if (p->Score == q->Score)
	return 0;
    else
	return 1;

}





/* @func ajXyzBinSearch ******************************************************
**
** Performs a binary search for an accession number over an array of Hitidx
** structures. This is a case-insensitive search.
**
** @param [r] id  [AjPStr]     Search term
** @param [r] arr [AjPHitlist] Array of AjOHitidx objects
** @param [r] siz [ajint]      Size of array
**
** @return [AjBool] Index of first AjOHitidx object found with an Id element 
** matching id, or -1 if id is not found.
** @@
******************************************************************************/
ajint ajXyzBinSearch(AjPStr id, AjPHitidx *arr, ajint siz)
{
    int l;
    int m;
    int h;
    int c;


    l=0;
    h=siz-1;
    while(l<=h)
    {
        m=(l+h)>>1;

        if((c=ajStrCmpCase(id, arr[m]->Id)) < 0) 
	    h=m-1;
        else if(c>0) 
	    l=m+1;
        else 
	    return m;
    }
    return -1;
}




/* @func ajXyzCompId *********************************************************
**
** Function to sort AjOHitidx objects by ID element. Usually called by 
** ajXyzHitlistClassify.
**
** @param [r] hit1  [const void*] Pointer to AjOHitidx object 1
** @param [r] hit2  [const void*] Pointer to AjOHitidx object 2
**
** @return [ajint] 1 if Id1 should sort before Id2, +1 if the Id2 should sort 
** first. 0 if they are identical in length and content. 
** @@
******************************************************************************/
ajint ajXyzCompId(const void *hit1, const void *hit2)
{
    AjPHitidx p  = NULL;
    AjPHitidx q  = NULL;

    p = (*(AjPHitidx*)hit1);
    q = (*(AjPHitidx*)hit2);
    
    return ajStrCmpO(p->Id, q->Id);

}





/* @func ajXyzHitlistClassify *************************************************
**
** Classifies a list of signature-sequence hits (held in a Hitlist object) 
** according to list of target sequences (a list of AjOHitlist objects).
** 
** Writes the Group, Typeobj & Typesbj elements depending on how the SCOP 
** classification records of the Hit object and target sequence in question
** compare.
**
** @param [r] hits    [AjPHitist *] Pointer to Hitlist object with hits
** @param [r] targets [AjPList]     List of AjOHitlist objects with targets
** @param [r] thresh  [ajint]       Minimum length (residues) of overlap 
** required for two hits with the same code to be counted as the same hit.
**
** @return [AjBool] True on success
** @@
******************************************************************************/
AjBool        ajXyzHitlistClassify(AjPHitlist *hits, AjPList targets, 
				   ajint thresh)
{  
    /* A list of Hitidx structures is derived from the list of AjOHitlist 
       objects to allow rapid searching for a given protein accession number*/

    AjIList     itert=NULL;		/*List iterator for targets*/
    AjPHitlist  ptrt=NULL;		/*Pointer for targets (hitlist structure) */
    AjPHitidx   ptri=NULL;		/*Pointer for index (Hitidx structure) */

    AjPHitidx  *idxarr=NULL;		/*Array of Hitidx structures */
    AjPList     idxlist=NULL;		/*List of Hitidx structures */
    ajint       idxsiz=0;		/*No.target sequences*/
    ajint       pos=0;			/*Position of a matching code in Hitidx 
					  structure*/
    ajint       tpos=0;			/*Temp. position counter */
    ajint       x=0;			/*Loop counter*/

    

    /*Create list & list iterator*/
    itert=ajListIter(targets);
    idxlist = ajListNew();
    

    /*Loop through list of targets filling list of Hitidx structures */
    while((ptrt=(AjPHitlist)ajListIterNext(itert)))
    {
	/*Write Hitidx structure*/
	for(x=0;x<ptrt->N;x++)
	{
	    ptri=ajXyzHitidxNew();
	    ptri->hptr=ptrt->hits[x];
	    ptri->lptr=ptrt;
	    ajStrAss(&ptri->Id, ptrt->hits[x]->Id);
	    ajListPush(idxlist,(AjPHitidx) ptri);
	}
    }


    /* Order the list of Hitidx structures by Id and transform into an array*/
    ajListSort(idxlist, ajXyzCompId);
    idxsiz = ajListToArray(idxlist, (void ***) &idxarr);
        
    


    /*Loop through list of hits */
    for(x=0; x<(*hits)->N; x++)
    {


	if((pos=ajXyzBinSearch((*hits)->hits[x]->Id, idxarr, idxsiz))!=-1)
	{
	    /* Id was found */
	    /*The list may contain multiple entries for the same Id, so 
	      search the current position and then up the list for other 
	      matching strings*/
	    tpos=pos; 
	    while(ajStrMatchCase(idxarr[tpos]->Id, (*hits)->hits[x]->Id))
	    {
		if(ajXyzHitsOverlap(idxarr[tpos]->hptr, 
				    (*hits)->hits[x], thresh))
		{	

		    if(ajStrMatchCase((idxarr[tpos]->lptr)->Family, 
				      (*hits)->Family))
			/*SCOP family is identical*/
		    {
			ajStrAss(&(*hits)->hits[x]->Typeobj, 
				 (idxarr[tpos]->hptr)->Typeobj);
			ajStrAssC(&(*hits)->hits[x]->Typesbj, 
				  "TRUE");
			(*hits)->hits[x]->Group=(idxarr[tpos]->hptr)->Group;
		    }
		    else if(ajStrMatchCase((idxarr[tpos]->lptr)->Fold, 
					   (*hits)->Fold))
			/*SCOP folds are identical*/
		    {
			ajStrAssC(&(*hits)->hits[x]->Typeobj, "CROSS");
			ajStrAssC(&(*hits)->hits[x]->Typesbj, "CROSS");
			(*hits)->hits[x]->Group=0;
		    }
		    else
			/*SCOP folds are different*/
		    {
			ajStrAssC(&(*hits)->hits[x]->Typeobj, "FALSE");
			ajStrAssC(&(*hits)->hits[x]->Typesbj, "FALSE");
			(*hits)->hits[x]->Group=0;
		    }
		}
		else
		{
		    /* Id was found but there was no overlap so set 
		       classification to UNKNOWN*/
		    ajStrAssC(&(*hits)->hits[x]->Typeobj, "UNKNOWN");
		    ajStrAssC(&(*hits)->hits[x]->Typesbj, "UNKNOWN");
		    (*hits)->hits[x]->Group=0;
		}
		    


		tpos--;	
		if(tpos<0) 
		    break;
	    }	    
				    
	    /*Search down the list*/
	    tpos=pos+1; 


	    if(tpos<idxsiz) 
		while(ajStrMatchCase(idxarr[tpos]->Id, (*hits)->hits[x]->Id))
		{

		    if(ajXyzHitsOverlap(idxarr[tpos]->hptr, 
					(*hits)->hits[x], thresh))
		    {	
			if(ajStrMatchCase((idxarr[tpos]->lptr)->Family, 
					  (*hits)->Family))
			    /*SCOP family is identical*/
			{
			    ajStrAss(&(*hits)->hits[x]->Typeobj, 
				     (idxarr[tpos]->hptr)->Typeobj);
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "TRUE");
			    (*hits)->hits[x]->Group=(idxarr[tpos]->hptr)->Group;
			}
			else if(ajStrMatchCase((idxarr[tpos]->lptr)->Fold, 
					       (*hits)->Fold))
			    /*SCOP fold is identical*/
			{	
			    ajStrAssC(&(*hits)->hits[x]->Typeobj, "CROSS");
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "CROSS");
			    (*hits)->hits[x]->Group=0;
			}
			else
			    /*SCOP folds are different*/
			{
			    ajStrAssC(&(*hits)->hits[x]->Typeobj, "FALSE");
			    ajStrAssC(&(*hits)->hits[x]->Typesbj, "FALSE");
			    (*hits)->hits[x]->Group=0;
			}
		    }
  		    else
		    {
			/* Id was found but there was no overlap so set 
			   classification to UNKNOWN*/
			ajStrAssC(&(*hits)->hits[x]->Typeobj, "UNKNOWN");
			ajStrAssC(&(*hits)->hits[x]->Typesbj, "UNKNOWN");
			(*hits)->hits[x]->Group=0;
		    }


		    tpos++;	
		    if(tpos==idxsiz) 
			break;
		}

	    
	}
    
    
	else
	{
	    /* Id was NOT found so set classification to UNKNOWN*/
	    ajStrAssC(&(*hits)->hits[x]->Typeobj, "UNKNOWN");
	    ajStrAssC(&(*hits)->hits[x]->Typesbj, "UNKNOWN");
	    (*hits)->hits[x]->Group=0;
	}
    }
    

    /*Clean up and return*/ 
    while(ajListPop(idxlist, (void **) &ptri))
	ajXyzHitidxDel(&ptri);	
    ajListDel(&idxlist);
    AJFREE(idxarr);
    ajListIterFree(itert);
    return ajTrue;
}

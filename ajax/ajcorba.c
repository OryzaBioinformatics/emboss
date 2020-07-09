/********************************************************************
** @source AJAX corba functions
**
** @author Copyright (C) 2000 Alan Bleasby
** @version 1.0 
** @modified Nov 10 ajb First version
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

#ifdef HAVE_ORB_ORBIT_H

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"
#include "orb/orbit.h"
#include "ajcorbaembl.h"

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

#define AJEMBLIOR "Eembl.ior"

/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */



/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section Corba Constructors ***********************************************
**
** All constructors return a new object by pointer. It is the responsibility
** of the user to first destroy any previous object. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/

/* @func ajCorbafeatNew ******************************************************
**
** Default constructor for empty AJAX corba feature objects.
**
** @param [r] ntypes [ajint] Number of keys e.g. CDS
**
** @return [AjPCorbafeat] Pointer to an codon object
** @@
******************************************************************************/

AjPCorbafeat ajCorbafeatNew(ajint ntypes)
{
    AjPCorbafeat feat;
    
    AJNEW0(feat);
    
    feat->Ntypes = ntypes;

    if(ntypes)
	AJCNEW0(feat->Types,ntypes);

    return feat;
}


/* @func ajCorbatypeNew ******************************************************
**
** Default constructor for empty AJAX corba feature type objects.
**
** @param [r] ntags [ajint] Number of tags e.g. note
** @param [r] nlocs [ajint] Number of location positions
**
** @return [AjPCorbatype] Pointer to an codon object
** @@
******************************************************************************/

AjPCorbatype ajCorbatypeNew(ajint ntags, ajint nlocs)
{
    AjPCorbatype Ctype=NULL;
    ajint i;


    AJNEW0(Ctype);

    Ctype->Ntags  = ntags;
    Ctype->Seq    = ajStrNew();
    Ctype->Name   = ajStrNew();
    Ctype->Source = ajStrNew();
    Ctype->Id     = ajStrNew();
    Ctype->Nlocs  = nlocs;
    
    if(ntags)
    {
	AJCNEW0(Ctype->Tag,ntags);
	AJCNEW0(Ctype->Val,ntags);
	AJCNEW0(Ctype->Nval,ntags);

	for(i=0;i<ntags;++i)
	    Ctype->Tag[i] = ajStrNew();
    }

    if(nlocs)
    {
	AJCNEW0(Ctype->LSpos,nlocs);
	AJCNEW0(Ctype->LSex,nlocs);
	AJCNEW0(Ctype->LSfuzzy,nlocs);
	AJCNEW0(Ctype->LEpos,nlocs);
	AJCNEW0(Ctype->LEex,nlocs);
	AJCNEW0(Ctype->LEfuzzy,nlocs);
	AJCNEW0(Ctype->LStrand,nlocs);
    }

    return Ctype;
}


/* @func ajCorbatypeDel ******************************************************
**
** Default destructor for AJAX corba feature type objects.
**
** @param [w] thys [AjPCorbatype *] corba feature type structure
**
** @return [void]
** @@
******************************************************************************/

void ajCorbatypeDel(AjPCorbatype *thys)
{
    AjPCorbatype pthis=NULL;
    ajint ntags;
    ajint nval;
    
    ajint i;
    ajint j;
    
    if(!thys)
	return;

    pthis = *thys;

    ntags = pthis->Ntags;
    for(i=0;i<ntags;++i)
    {
	nval = pthis->Nval[i];
	for(j=0;j<nval;++j)
	    ajStrDel(&pthis->Val[i][j]);
	ajStrDel(&pthis->Tag[i]);
    }
    
    if(ntags)
    {
	AJFREE(pthis->Tag);
	AJFREE(pthis->Val);
	AJFREE(pthis->Nval);
    }

    if(pthis->Nlocs)
    {
	AJFREE(pthis->LSpos);
	AJFREE(pthis->LSex);
	AJFREE(pthis->LSfuzzy);
	AJFREE(pthis->LEpos);
	AJFREE(pthis->LEex);
	AJFREE(pthis->LEfuzzy);
	AJFREE(pthis->LStrand);
    }
    
    ajStrDel(&pthis->Name);
    ajStrDel(&pthis->Source);
    ajStrDel(&pthis->Id);
    ajStrDel(&pthis->Seq);

    return;
}

/* @func ajCorbafeatDel ******************************************************
**
** Default destructor for AJAX corba top level feature objects.
** Also deletes associated corba type objects
**
** @param [w] thys [AjPCorbafeat *] corba feature structure
**
** @return [void]
** @@
******************************************************************************/

void ajCorbafeatDel(AjPCorbafeat *thys)
{
    AjPCorbafeat pthis=NULL;
    ajint i;
    ajint ntypes;
    

    if(!thys)
	return;

    pthis = *thys;
    
    ntypes = pthis->Ntypes;
    for(i=0;i<ntypes;++i)
	ajCorbatypeDel(&pthis->Types[i]);
    if(ntypes)
	AJFREE(pthis->Types);

    return;
}

/* @func ajSeqCorbaEmbl ******************************************************
**
** Returns a sequence and feature information from the Biocorba 0.2 EBI server
**
** @param [r] code [char *] EMBL id or accesion number
** @param [w] exerr [char **] Error string
** @param [w] exint [int *] Error number
** @param [w] thys [AjPCorbafeat *] corba feature structure
** @param [r] dofeat [AjBool] Return features
**
** @return [AjPStr] constructed sequence or NULL if error
** @@
******************************************************************************/

AjPStr ajSeqCorbaEmbl(char *code, char **exerr, ajint *exint, AjPCorbafeat *feat,
		      AjBool dofeat)
{
    AjPFile inf      = NULL;
    AjPStr  ior      = NULL;
    AjPStr  sequence = NULL;

/*  org_biocorba_seqcore_SeqType            Cseqtype;       */
/*  org_biocorba_seqcore_AnonymousSeq       Canonseq;       */
    org_biocorba_seqcore_PrimarySeq         Cpriseq;
    org_biocorba_seqcore_SeqFeatureVector   Cseqfeatvec=NULL;
    org_biocorba_seqcore_Seq                Cseq;
/*  org_biocorba_seqcore_FuzzyTypeCode      Cfuzzytypecode; */
    org_biocorba_seqcore_SeqFeature         Cseqfeat;
/*  org_biocorba_seqcore_PrimarySeqIterator Cpriseqiter;    */
    org_biocorba_seqcore_SeqFeatureIterator Cseqfeatiter=NULL;
/*  org_biocorba_seqcore_PrimarySeqVector   Cpriseqvect;    */
/*  org_biocorba_seqcore_SeqFeatureVector   Cseqfeatvect;   */
/*  org_biocorba_seqcore_PrimarySeqDB       CpriseqDB;      */
    org_biocorba_seqcore_SeqDB              CseqDB;
/*  org_biocorba_seqcore_UpdateableSeqDB    CupdseqDB;      */
    org_biocorba_seqcore_BioEnv             Cbioenv;
    
    CORBA_ORB orb;
    CORBA_Environment Cenv;

    ajint          Eargc   = 1;
    static char *Eargv[] = {"E"};


    CORBA_char *dbname="EMBL";
    CORBA_long version=0;

    CORBA_char *Ename;
    CORBA_char *Edispid;
    CORBA_char *Eprimid;
    CORBA_char *Eaccno;
    CORBA_long Eversion;
    CORBA_long Esversion;
    CORBA_long Emaxlen;
    CORBA_long Eseqlen;
    
    CORBA_char *sq;
    
    CORBA_long Enfeat;

    CORBA_boolean morefeat;
    CORBA_char    *Eftype;
    CORBA_char    *Efsource;
    CORBA_char    *Efid;
    CORBA_long    Efstart;
    CORBA_long    Efend;
    CORBA_short   Efstrand;

    org_biocorba_seqcore_NameValueSetList *Efptr;
    org_biocorba_seqcore_NameValueSet     *Eqptr;
    org_biocorba_seqcore_StringList       Esl;
    org_biocorba_seqcore_SeqFeatureLocationList *Ell;
    org_biocorba_seqcore_SeqFeatureLocation *Esfl;
    
    ajint i;
    ajint j;
    unsigned ajlong len;
    unsigned ajlong qlen;
    ajint fcnt  = 0;
    ajint ntags = 0;
    ajint nlocs = 0;
    
    

    /*
     *  Get the EMBL IOR
     */
    ajFileDataNewC(AJEMBLIOR,&inf);
    if(!inf)
	return NULL;
    ior = ajStrNew();
    if(!ajFileReadLine(inf,&ior))
	return NULL;
    ajFileClose(&inf);


    /*
     *  Initialise the orb
     */
    CORBA_exception_init(&Cenv);
    orb = CORBA_ORB_init(&Eargc,Eargv,"orbit-local-orb",&Cenv);

    /*
     *  Get a bioenv object
     */
    Cbioenv = CORBA_ORB_string_to_object(orb, ajStrStr(ior), &Cenv);
    if(!Cbioenv)
    {
        ajWarn("IOR failure %S\n", ior);
        return NULL;
    }
    ajStrDel(&ior);
    
    /*
     *  Need a seqdb object to get dbname, dbversion, maxlen & sequences
     */
    CseqDB = org_biocorba_seqcore_BioEnv_get_SeqDB_by_name(Cbioenv,
							   dbname,version,
							   &Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "SeqDB";
	*exint  = Cenv._major;
	return NULL;
    }


    Ename = org_biocorba_seqcore_SeqDB_name(CseqDB,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "EMBL name";
	*exint  = Cenv._major;
	return NULL;
    }

    Eversion = org_biocorba_seqcore_SeqDB_version(CseqDB,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "EMBL version";
	*exint  = Cenv._major;
	return NULL;
    }

    Emaxlen = org_biocorba_seqcore_SeqDB_max_sequence_length(CseqDB,
							     &Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "EMBL maxlength";
	*exint  = Cenv._major;
	return NULL;
    }


    /*
     *  Now we need the sequence so we first need to get a Cseq object.
     *  This allows us to get the sequence, length, is_circular, subseq, type
     *
     *  Can also get get a Cpriseq object from the Cseq object though
     *  its unneeded for this.
     */
    Cseq = org_biocorba_seqcore_SeqDB_get_Seq(CseqDB,code,0L,
					      &Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "Cseq";
	*exint  = Cenv._major;
	return NULL;
    }

    Cpriseq = org_biocorba_seqcore_Seq_get_PrimarySeq(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "Cpriseq";
	*exint  = Cenv._major;
	return NULL;
    }
    
    /*
     *  Get the sequence length, Don't need to chunk yet as max seq size
     *  is 2Gb, so its out of interest only
     */
    Eseqlen = org_biocorba_seqcore_Seq_length(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "Seqlen";
	*exint  = Cenv._major;
	return NULL;
    }

    sq = org_biocorba_seqcore_Seq_seq(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "Sequence";
	*exint  = Cenv._major;
	return NULL;
    }

    Edispid   = org_biocorba_seqcore_Seq_display_id(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "dispid";
	*exint  = Cenv._major;
	return NULL;
    }

    Eprimid   = org_biocorba_seqcore_Seq_primary_id(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "primid";
	*exint  = Cenv._major;
	return NULL;
    }

    Eaccno    = org_biocorba_seqcore_Seq_accession_number(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "Accession";
	*exint  = Cenv._major;
	return NULL;
    }

    Esversion = org_biocorba_seqcore_Seq_version(Cseq,&Cenv);
    if(Cenv._major != CORBA_NO_EXCEPTION)
    {
	*exerr  = "Sequence version";
	*exint  = Cenv._major;
	return NULL;
    }


    if(dofeat)
    {
	/*
	 *  Features: First need a sequence feature vector object
	 *  which can be picked up from the Seq interface
	 *  via inheritance
	 */
	Cseqfeatvec = org_biocorba_seqcore_Seq_all_SeqFeatures(Cseq,1,&Cenv);
	if(Cenv._major != CORBA_NO_EXCEPTION)
	{
	    *exerr  = "Feature vector";
	    *exint  = Cenv._major;
	    return NULL;
	}


	/*
	 *  From the feature vector we can retrieve the number of seqfeature
	 *  objects and also an iterator over the objects
	 */

	Enfeat = org_biocorba_seqcore_SeqFeatureVector_size(Cseqfeatvec,
							    &Cenv);
	if(Cenv._major != CORBA_NO_EXCEPTION)
	{
	    *exerr  = "Feature size";
	    *exint  = Cenv._major;
	    return NULL;
	}

	*feat = ajCorbafeatNew(Enfeat);

	Cseqfeatiter =
	    org_biocorba_seqcore_SeqFeatureVector_iterator(Cseqfeatvec,&Cenv);
	if(Cenv._major != CORBA_NO_EXCEPTION)
	{
	    *exerr  = "Feature Iterator";
	    *exint  = Cenv._major;
	    return NULL;
	}

	/*
	 *  Now iterate over the entire set of features
	 */
	fcnt = -1;
	morefeat = 1;
	while(morefeat)
	{
	    ++fcnt;
	    morefeat =
		org_biocorba_seqcore_SeqFeatureIterator_has_more(Cseqfeatiter,
								 &Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Has_more";
		*exint  = Cenv._major;
		return NULL;
	    }
	    if(!morefeat)
		continue;

	    /* Pick up the associated seqfeature object */
	    Cseqfeat =
		org_biocorba_seqcore_SeqFeatureIterator_next(Cseqfeatiter,
							     &Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Cseqfeat";
		*exint  = Cenv._major;
		return NULL;
	    }

	    Ell = org_biocorba_seqcore_SeqFeature_locations(Cseqfeat,&Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature Locations list";
		*exint  = Cenv._major;
		return NULL;
	    }

	    nlocs = Ell->_length;
	    Esfl  = Ell->_buffer;
	    


	    Efptr = org_biocorba_seqcore_SeqFeature_qualifiers(Cseqfeat,
							       &Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature NameValList";
		*exint  = Cenv._major;
		return NULL;
	    }

	    ntags = Efptr->_length;
	    printf("NLOCS=%d\n",nlocs);
	    (*feat)->Types[fcnt] = ajCorbatypeNew(ntags,nlocs);

	    for(i=0;i<nlocs;++i)
	    {
		(*feat)->Types[fcnt]->LStrand[i] = (ajint) ((short)Esfl->strand);
		printf("STRAND=%d\n", (ajint) ((short)Esfl->strand));
		(*feat)->Types[fcnt]->LSpos[i]   = Esfl->start.position;
		(*feat)->Types[fcnt]->LEpos[i]   = Esfl->end.position;
		(*feat)->Types[fcnt]->LSex[i]    = Esfl->start.extension;
		(*feat)->Types[fcnt]->LEex[i]    = Esfl->end.extension;
		(*feat)->Types[fcnt]->LSfuzzy[i] = Esfl->start.fuzzy;
		(*feat)->Types[fcnt]->LEfuzzy[i] = Esfl->end.fuzzy;
	    }
	    

	

	    /* Get Top-level feature information */
	    Eftype = org_biocorba_seqcore_SeqFeature_type(Cseqfeat,&Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature type";
		*exint  = Cenv._major;
		return NULL;
	    }
	    ajStrAssC(&(*feat)->Types[fcnt]->Name,Eftype);
	
	    Efsource = org_biocorba_seqcore_SeqFeature_source(Cseqfeat,
							      &Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature source";
		*exint  = Cenv._major;
		return NULL;
	    }
	    ajStrAssC(&(*feat)->Types[fcnt]->Source,Efsource);

	    Efid = org_biocorba_seqcore_SeqFeature_seq_primary_id(Cseqfeat,
								  &Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature id";
		*exint  = Cenv._major;
		return NULL;
	    }
	    ajStrAssC(&(*feat)->Types[fcnt]->Id,Efid);

	    Efstart = org_biocorba_seqcore_SeqFeature_start(Cseqfeat,&Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature start";
		*exint  = Cenv._major;
		return NULL;
	    }
	    (*feat)->Types[fcnt]->Start = Efstart;
	
	    Efend = org_biocorba_seqcore_SeqFeature_end(Cseqfeat,&Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature end";
		*exint  = Cenv._major;
		return NULL;
	    }
	    (*feat)->Types[fcnt]->End = Efend;

	    Efstrand = org_biocorba_seqcore_SeqFeature_strand(Cseqfeat,
							      &Cenv);
	    if(Cenv._major != CORBA_NO_EXCEPTION)
	    {
		*exerr  = "Feature strand";
		*exint  = Cenv._major;
		return NULL;
	    }
	    (*feat)->Types[fcnt]->Strand = Efstrand;


	    len = Efptr->_length;
	    Eqptr = Efptr->_buffer;
	    for(i=0;i<ntags;++i)
	    {
		ajStrAssC(&(*feat)->Types[fcnt]->Tag[i],Eqptr[i].name);
		Esl = Eqptr[i].values;
		qlen = Esl._length;
		(*feat)->Types[fcnt]->Nval[i] = qlen;
		AJCNEW0((*feat)->Types[fcnt]->Val[i],qlen);
		for(j=0;j<qlen;++j)
		    (*feat)->Types[fcnt]->Val[i][j] =
			ajStrNewC(Esl._buffer[j]);
	    }
	



	    /* Clean up the feature object */
	    CORBA_free(Ell);
	    CORBA_free(Efptr);
	    CORBA_free(Eftype);
	    CORBA_free(Efsource);
	    CORBA_free(Efid);

	    org_biocorba_seqcore_SeqFeature_unref(Cseqfeat,&Cenv);
	    CORBA_Object_release(Cseqfeat, &Cenv);
	}

	org_biocorba_seqcore_SeqFeatureIterator_unref(Cseqfeatiter,&Cenv);
	org_biocorba_seqcore_SeqFeatureVector_unref(Cseqfeatvec,&Cenv);
	CORBA_Object_release(Cseqfeatiter, &Cenv);
	CORBA_Object_release(Cseqfeatvec, &Cenv);
    }
    
    sequence = ajStrNewC(sq);
    

    /*
     *  Tidy
     */

    CORBA_free(sq);
    CORBA_free(Ename);
    CORBA_free(Edispid);
    CORBA_free(Eprimid);
    CORBA_free(Eaccno);



    org_biocorba_seqcore_PrimarySeq_unref(Cpriseq,&Cenv);
    org_biocorba_seqcore_Seq_unref(Cseq,&Cenv);
    org_biocorba_seqcore_SeqDB_unref(CseqDB,&Cenv);
    

    CORBA_Object_release(Cpriseq, &Cenv);
    CORBA_Object_release(Cseq, &Cenv);
    CORBA_Object_release(CseqDB, &Cenv);
    CORBA_Object_release(Cbioenv, &Cenv);
    CORBA_Object_release((CORBA_Object)orb, &Cenv);

    return sequence;
}


#endif

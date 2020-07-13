/* @source funky application
**
** Reads clean coordinate files and writes file of protein-heterogen contact
** data.
**
** @author: Copyright (C) Waqas Awan (wawan@hgmp.mrc.ac.uk)
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
*******************************************************************************
**
**
**
*******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
*******************************************************************************
**
** Mon May 20 11:43:39 BST 2002
**
** The following documentation is out-of-date and should be disregarded.  It
** will be updated shortly.
**
*******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
*******************************************************************************

** Important Notes pdbparse necessarily must hold the entire pdb file
** and some derived data in memory. If an error of the type 'Uncaught
** exception: Allocation failed, insufficient memory available' is
** raised then this is probably because the memory requirements exceed
** per-user memory defaults (that are usually set quite low). This can
** easily be unlimited by the sysop in the login process. If tcsh is
** used, then simply type 'unlimit' before pdbparse is run.
*/

/*---------------------------------------------------------------------------*/
/* DATA STRUCTURES                                                           */
/*---------------------------------------------------------------------------*/

/* @data AjPDomConts **********************************************************
**
** Ajax DomConts object.
**
** Holds the domain contact data
**
** AjPDomConts is implemented as a pointer to a C data structure.
**
** @alias AjSDomConts
** @alias AjODomConts
**
** @@
******************************************************************************/


#include "emboss.h"

typedef struct AjSDomConts
{
  AjPStr het_name;       /* 3-character code of heterogen */
  AjPStr scop_name;      /* 7-character scop id domain name */
  ajint  no_keyres;      /* number of key binding residues */
  AjPStr *aa_code;       /* Array for 3-character amino acid codes */
  AjPInt res_pos;        /* Array of ints for residue positions in
                            domain file */
  AjPStr *res_pos2;      /* Array of residue positions in complete
                            protein coordinate file - exist as strings */
}AjODomConts, *AjPDomConts;


/* @data AjPDbaseEnt **********************************************************
**
** Ajax DbaseEnt object.
**
** Holds the data required for the database of functional sites
**
** AjPDbaseDat is implemented as a pointer to a C data structure.
**
** @alias AjSDbaseEnt
** @alias AjODbaseEnt
**
** @@
******************************************************************************/
typedef struct AjSDbaseEnt
{
  AjPStr      abv;         /* 3-letter abbreviation of heterogen */
  AjPStr      ful;         /* Full name */
  ajint       no_dom;      /* number of domains */
  AjPDomConts *cont_data;  /* array of domain contact data (derived from tmp)*/
  AjPList     tmp;         /* Temp. list of domain contact data */
} AjODbaseEnt, *AjPDbaseEnt;


/* @data AjPDbase *************************************************************
**
** Ajax Dbase object.
**
** Holds a Database of functional residues.
**
** AjPDbase is implemented as a pointer to a C data structure.
**
** @alias AjSDbase
** @alias AjODbase
**
** @@
******************************************************************************/
typedef struct AjSDbase
{
  ajint         n;        /* Number of entries */
  AjPDbaseEnt *entries;   /* Array of entries */
} AjODbase, *AjPDbase;



/*---------------------------------------------------------------------------*/
/* FUNCTION PROTOTYPES                                                       */
/*---------------------------------------------------------------------------*/
static AjPDbase    funky_DbaseNew(ajint n);
static void        funky_DbaseDel(AjPDbase *ptr);
static AjPDbaseEnt funky_DbaseEntNew(ajint n);
static void        funky_DbaseEntDel(AjPDbaseEnt *ptr);
static void        funky_DomContsDel(AjPDomConts *ptr);
static AjPDomConts funky_DomContsNew(ajint n);
static AjBool      funky_HetTest(AjPPdb ptr);
static AjBool      funky_DichetToDbase(AjPDbase *dbase, AjPHet hetdic);
static AjBool      funky_CpdbListHeterogens(AjPPdb pdb,
					    AjPList *list_heterogens,
					    AjPInt *siz_heterogens,
					    ajint *nhet,
					    AjPFile logf );
static AjBool      funky_PdbGetDomain(AjPPdb pdb, AjPList list_allscop,
				      AjPList *list_pdbscopids);
static AjBool      funky_HeterogenContacts(float dist_thresh,
					   AjPList list_domains,
					   AjPList list_pdbscopids,
					   AjPList list_heterogens,
					   AjPInt siz_domains,
					   AjPInt siz_heterogens,
					   AjPVdwall vdw,
					   AjPDbase *dbase);
static AjBool      funky_HeterogenContactsWrite(AjPFile funky_out,
						AjPDbase dbase);
/*--------------------------------------------------------------------------*/


/* @prog funky ****************************************************************
**
** Calculate protein-heterogen contacts from clean coordinate files
**
******************************************************************************/

int main(ajint argc, char **argv)
{

  /* DECLARE ACD STUFF */

  /* Declare ACD strings */
  AjPStr   prot=NULL;			/* Location of protein
                                           coordinate files (path) */
  AjPStr   protextn=NULL;		/* Extension of protein
                                           coordinate files */
  AjPStr   dom=NULL;			/* Location of domain
                                           coordinate files (path) */
  AjPStr   domextn=NULL;		/* Extension of domain
                                           coordinate files */
  /* Declare ACD file pointers */
  AjPFile  escop_fptr=NULL;		/* Pointer to scop domain
                                           index file (Escop.dat) */
  AjPFile  het_fptr=NULL;               /* Pointer to dictionary of
                                           heterogens file (output
                                           from Dichet.c) */
  AjPFile  funky_out=NULL;		/* Pointer to output file */
  AjPFile  vdwf=NULL;			/* Pointer to van der Waals file */
  /* Other ACD stuff */
  float    dist_thresh;		        /* Threshold contact distance */

  /* Reading data files */
  AjPList    list_allscop=NULL;	        /* List for SCOP classification */
  AjPScop    scoptemp=NULL;		/* Temp. pointer for ajListPop
                                           function */
  AjPHet  hetdic=NULL;		        /* Dichet object to hold entry
                                           from heterogen dictionary */
  AjPVdwall  vdw=NULL;                  /* Vdwall object to hold van
                                           der Waals radii data */

  /* Creating Dbase object */
  AjPDbase   dbase=NULL;		/* Database of functional
                                           residues for whole build */
  ajint      i=0;			/* Counter */

  /* For creating list of protein coordinate files */
  AjPStr   p_fname=NULL;                /* Protein coordinate filename */
  AjPList  pdbfile_list=NULL;		/* List for protein coordinate
                                           file names */

  /* Main application loop */
  AjPStr   cp_file=NULL;                /* Current protein coordinate file */
  AjPFile  prot_fptr=NULL;	        /* Protein coordinate file pointer */
  AjPStr   msg=NULL;                    /* String for messaging */
  AjPPdb   pdb=NULL;		        /* Pdb object for protein */
  ajint    f_ctr=0;                     /* Protein coordinate file counter */
  ajint    f_num=0;                     /* Total number of protein
                                           coordinate files */


  /* Create list of heterogen atom arrays */
  AjPInt   siz_heterogens=NULL;        /* Integer array with numbers
                                          of elements in
                                          list_heterogens */
  AjPList  list_heterogens=NULL;       /* List of heterogen
                                          Atom-arrays (each for each
                                          heterogen in current Pdb object */
  ajint    n_heterogens=0;             /* no. of heterogens/arrays in
                                          current Pdb object */

  /* Create list of scop identifiers for domains in pdb object */
  AjPList  list_pdbscopids=NULL;       /* List to hold SCOP domain
                                          identifiers for domains in
                                          pdb object */
  AjPStr   scopid_tmp=NULL;            /* Temprorary pointer for
                                          ajListPop to free memory for
                                          list_pdbscopid */

  /* Create list of domain coordinate filenames */
  AjIList  iter_scopids=NULL;          /* Iterator for list_pdbscopids */
  AjPStr   pdbscopid=NULL;             /* Pointer to current domain scop id */
  AjPStr   dom_co_fname=NULL;          /* Domain coordinate filename */
  AjPList  list_domfnames=NULL;        /* List of domain coordinate
                                          filenames for domains in pdb
                                          object */
  AjPStr   domfname_tmp=NULL;          /* Temprorary pointer for
                                          ajListPop to free memory for
                                          list_domfnames */

  /* Open domain coordinate file and create list of domain pdb objects */
  AjIList  iter_domfnames=NULL;        /* Iterator for list of domain
                                          coordinate filenames */
  AjPStr   pdbscop_fname=NULL;         /* Pointer to current domain
                                          coordinate filename */
  ajint    ndomain=0;                  /* Number of the current domain
                                          being processed */
  AjPFile  dom_fptr=NULL;              /* Domain coordinate file pointer */
  AjPPdb   pdbdom=NULL;                /* Pdb object for domain */
  AjPList  pdbdomList=NULL;             /* List of domain pdb objects
                                           for domains in current
                                           protein coordinate file */

  /* Create list of domain atom arrays in current pdb object */
  ajint    siz_atomarr=0;	       /* Number of elements in
                                          current array of domain
                                          atoms */
  AjPAtom  *atomarr=NULL;              /* Array of domain atoms */
  AjPList  list_domains=NULL;          /* List of domain Atom arrays
                                          from pdb object */
  AjPInt   siz_domains=NULL;           /* Integer array with array
                                          sizes for list_domains */
  AjIList  iter_pdbdom=NULL;           /* Iterator for pdbdomList */
  AjPPdb   pdbdom_ptr=NULL;            /* Pointer to current domain
                                          pdb object in pdbdomList -
                                          also used for freeing
                                          memory*/

  /* Calculate contacts between domain atoms and heterogen atoms */
  ajint    l=0;                        /* Lengths of atom arrays */

  /* Freeing memory for the list of atom arrays */
  AjIList  iter_loa=NULL;	       /* Iterator for list of atom arrays*/
  AjPAtom  *l_atm=NULL;		       /* Current array of atom objects */

  /* log file */
  AjPFile logf=NULL;


  embInit("funky", argc, argv);


  /* READ ACD STUFF */

  /* Strings */
  prot=ajAcdGetString("prot");
  protextn=ajAcdGetString("protextn");
  dom=ajAcdGetString("dom");
  domextn=ajAcdGetString("domextn");
  /* File Pointers */
  escop_fptr=ajAcdGetInfile("scop");
  het_fptr=ajAcdGetInfile("dic");
  funky_out=ajAcdGetOutfile("outf");
  vdwf=ajAcdGetInfile("vdwf");
  /* Other */
  dist_thresh=ajAcdGetFloat("thresh");

  logf = ajAcdGetOutfile("logf");





  /* Allocate strings etc */
  p_fname = ajStrNew();
  msg = ajStrNew();

  /* CHECK DIRECTORIES */
  if(!ajFileDir(&prot))
    {
      ajStrDel(&p_fname);
      ajStrDel(&prot);
      ajStrDel(&protextn);
      ajStrDel(&dom);
      ajStrDel(&domextn);
      ajStrDel(&msg);
      ajFileClose(&escop_fptr);
      ajFileClose(&het_fptr);
      ajFileClose(&funky_out);
      ajFileClose(&vdwf);
      ajFatal("Could not open protein coordinate files directory");
    }
  if(!ajFileDir(&dom))
    {
      ajStrDel(&p_fname);
      ajStrDel(&prot);
      ajStrDel(&protextn);
      ajStrDel(&dom);
      ajStrDel(&domextn);
      ajStrDel(&msg);
      ajFileClose(&escop_fptr);
      ajFileClose(&het_fptr);
      ajFileClose(&funky_out);
      ajFileClose(&vdwf);
      ajFatal("Could not open domain coordinate files directory");
    }

  /* SPECIFY EXTENSION FOR DOMAIN COORDINATE FILES - APPEND WITH '.'
     IF NOT ALREADY PRESENT */
  if(!(ajStrChar(domextn, 0)=='.'))
    {
      ajStrInsertC(&domextn, 0, ".");
    }


  /* READ IN DATA FILES AND CREATE NECESSARY OBJECT */

  /* Read Escop.dat and create SCOP classification as list of SCOP objects */
  list_allscop=ajListNew();
  if(!ajXyzScopReadAll(escop_fptr, &list_allscop))
    {
      ajStrDel(&p_fname);
      ajStrDel(&prot);
      ajStrDel(&protextn);
      ajStrDel(&dom);
      ajStrDel(&domextn);
      ajStrDel(&msg);
      ajFileClose(&escop_fptr);
      ajFileClose(&het_fptr);
      ajFileClose(&funky_out);
      ajFileClose(&vdwf);
      ajFatal("Error reading SCOP classification file\n");
    }

  /* Read heterogen_dictionary.out and create Dichet object */
  hetdic = ajXyzHetNew(0);
  ajXyzHetRead(het_fptr, &hetdic);


  /* Read van der Waals data file (Evdw.dat) and create vdw object*/
  if(!ajXyzVdwallRead(vdwf, &vdw))
    {
      ajStrDel(&p_fname);
      ajStrDel(&prot);
      ajStrDel(&protextn);
      ajStrDel(&dom);
      ajStrDel(&domextn);
      ajStrDel(&msg);
      ajFileClose(&escop_fptr);
      ajFileClose(&het_fptr);
      ajFileClose(&funky_out);
      ajFileClose(&vdwf);
      ajFatal("Error reading vdw radii file\n");
    }

  /* CREATE DBASE OBJECT */
  dbase=funky_DbaseNew(hetdic->n);
  for(i=0; i<hetdic->n; i++)
    {
      dbase->entries[i] = funky_DbaseEntNew(0);
    }

  /* WRITE HETEROGEN INFORMATION FROM DICHET OBJECT (ABBREVIATIONS AND
     FULL NAMES) TO DBASE OBJECT */
  funky_DichetToDbase(&dbase, hetdic);

  /* CREATE LIST OF PROTEIN COORDINATE FILES IN PROTEIN COORDINATE DIRECTORY */
  pdbfile_list=ajListstrNew();
  ajStrAssC(&p_fname, "*");		/* Assign wildcard character
                                           to p_fname */
  if((ajStrChar(protextn, 0)=='.'))
    {
      ajStrApp(&p_fname, protextn);
    }
  else
    {
      ajStrAppC(&p_fname, ".");
      ajStrApp(&p_fname, protextn);
    }
  ajFileScan(prot, p_fname, &pdbfile_list, ajFalse, ajFalse, NULL, NULL,
	     ajFalse, NULL);

  f_num=ajListLength(pdbfile_list);

  /* START OF MAIN APPLICATION LOOP - PER PROTEIN COORDINATE FILE */
  while(ajListstrPop(pdbfile_list, &cp_file))
    {
	ajFmtPrint("CPDB FILE: %S (%d/%d)\n", cp_file, f_ctr++, f_num);
      ajFmtPrintF(logf, "CPDB: %S", cp_file);
      fflush(stdout);
      ndomain=0;			/* Reset counter for the
                                           number of domains in this
                                           file */

      /* OPEN FILE AND WRITE PDB OBJECT */
      /* Open file */
      if((prot_fptr=ajFileNewIn(cp_file))==NULL)
	{
	  ajFmtPrintS(&msg, "Could not open for reading %S", cp_file );
	  ajWarn(ajStrStr(msg));
	  ajStrDel(&cp_file);
	  continue;
	}

      /* Write pdb object */
      if(!ajXyzCpdbReadFirstModel(prot_fptr, &pdb))
	{
	  /* No need to free memory for pdb if CpdbRead fails */
	  ajFmtPrintS(&msg, "ERROR file read error %S", cp_file);
	  ajWarn(ajStrStr(msg));
	  ajStrDel(&cp_file);
	  ajFileClose(&prot_fptr);
	  continue;
	}

      /* Close protein coordinate file */
      ajFileClose(&prot_fptr);

      /* Check for heterogens in pdb object, if no heterogens skip to
         next file */
      if(!funky_HetTest(pdb))
	{
	  ajStrDel(&cp_file);
	  ajFmtPrintF(logf, "\tHET:0\tDOM:-\n");
	  ajXyzPdbDel(&pdb);
	  pdb=NULL;
	  continue;
	}

      /* Free string */
      ajStrDel(&cp_file);

      /* CREATE LIST OF ATOM ARRAYS FOR HETEROGENS IN CURRENT PDB OBJECT */
      list_heterogens=ajListNew();
      siz_heterogens=ajIntNew();
      n_heterogens=0;
      funky_CpdbListHeterogens(pdb, &list_heterogens, &siz_heterogens,
			       &n_heterogens, logf);

      l=ajListLength(list_heterogens);
      ajFmtPrintF(logf, "\tHET:%d", l);

      /* CREATE LIST OF SCOP IDENTIFIERS FOR SCOP DOMAINS OCCURRING IN
         CURRENT PDB OBJECT */
      list_pdbscopids=ajListstrNew();

      /* Check for domains in the pdb object, if no domains skip to
         next file */
      if(!funky_PdbGetDomain(pdb, list_allscop, &list_pdbscopids))
	{
	  ajFmtPrintF(logf, "\tDOM:0");
	  ajXyzPdbDel(&pdb);
	  pdb=NULL;

	  /* Free list of heterogen atom arrays */
	  iter_loa=ajListIter(list_heterogens);
	  while((l_atm=(AjPAtom *)ajListIterNext(iter_loa)))
	    AJFREE(l_atm);
	  ajListDel(&list_heterogens);
	  list_heterogens=NULL;
	  ajListIterFree(iter_loa);
	  iter_loa=NULL;

	  /* Free siz_heterogens */
	  ajIntDel(&siz_heterogens);
	  siz_heterogens=NULL;
	  continue;
	}

      /* CREATE LIST OF SCOP DOMAIN FILENAMES */
      list_domfnames=ajListstrNew();

      /* Initialise iterator for list of scop ids */
      iter_scopids=ajListIter(list_pdbscopids);
      while((pdbscopid=ajListIterNext(iter_scopids)))
	{
	  dom_co_fname=ajStrNew();
	  ajStrAssS(&dom_co_fname, dom);
	  ajStrApp(&dom_co_fname, pdbscopid);
	  ajStrApp(&dom_co_fname, domextn);
	  ajListstrPushApp(list_domfnames, dom_co_fname);
	}
      ajListIterFree(iter_scopids);


      /* OPEN DOMAIN COORDINATE FILE AND WRITE LIST OF DOMAIN PDB OBJECTS */

      list_domains=ajListNew();
      siz_domains=ajIntNew();
      pdbdomList=ajListNew();

      /* Initialise iterator */
      iter_domfnames=ajListIter(list_domfnames);

      /* START OF DOMAIN LOOP - PER DOMAIN IN list_scopids */

      while((pdbscop_fname=ajListIterNext(iter_domfnames)))
	{
	  if((dom_fptr=ajFileNewIn(pdbscop_fname))==NULL)
	    {
	      ajFmtPrintS(&msg, "Could not open for reading: %S",
			  pdbscop_fname);
	      ajWarn(ajStrStr(msg));
	      continue;
	    }
	  /* Write pdb object */
	  if(!ajXyzCpdbRead(dom_fptr, &pdbdom))
	    {
	      ajFmtPrintS(&msg, "ERROR file read error");
	      ajWarn(ajStrStr(msg));
	      ajFileClose(&dom_fptr);
	      continue;
	    }

	  /* Create list of domain pdb objects */
	  ajListPushApp(pdbdomList, pdbdom);

	  /* Close domain coordinate file */
	  ajFileClose(&dom_fptr);

	}
      /* END OF DOMAIN LOOP */


      /* CREATE LIST OF DOMAIN ATOM ARRAYS FOR DOMAINS IN CURRENT
         PROTEIN COORDINATE FILE */
      iter_pdbdom=ajListIter(pdbdomList);
      while((pdbdom_ptr=ajListIterNext(iter_pdbdom)))
	{
	  /* note: only one chain in domain coordinate file and so Chains[0] */
	    siz_atomarr=ajListToArray(pdbdom_ptr->Chains[0]->Atoms,
				      (void ***) &atomarr);
	  ajListPushApp(list_domains, (AjPAtom *) atomarr);
	  ajIntPut(&siz_domains, ndomain, siz_atomarr);
	  ndomain++;
	}
      ajListIterFree(iter_pdbdom);
      pdbdom_ptr=NULL;

      l=ajListLength(list_domains);
      ajFmtPrintF(logf, "\tDOM:%d\n", l);

      /* CALCULATE CONTACTS BETWEEN ALL COMBINATIONS OF DOMAINS AND
         HETEROGENS IN CURRENT PDB OBJECT */
      funky_HeterogenContacts(dist_thresh, list_domains, list_pdbscopids,
			      list_heterogens, siz_domains, siz_heterogens,
			      vdw,  &dbase);

      /* Free memory for domain pdb objects and pdbdomList */
      while(ajListPop(pdbdomList, (void **) &pdbdom_ptr))
	{
	  ajXyzPdbDel(&pdbdom_ptr);
	}
      ajListDel(&pdbdomList);

      /* Free siz_domains */
      ajIntDel(&siz_domains);

      /* Free siz_heterogens */
      ajIntDel(&siz_heterogens);

      /* Free list of domain atom arrays */

      /* Free memory for the arrays themselves (but not the data that
	 the array elements point to because this is free'd by the
	 call to ajXyzPdbDel) */


      iter_loa=ajListIter(list_domains);
      while((l_atm=(AjPAtom *)ajListIterNext(iter_loa)))
	AJFREE(l_atm);
      ajListIterFree(iter_loa);


      ajListDel(&list_domains);


      /* Free list of heterogen atom arrays */

      iter_loa=ajListIter(list_heterogens);
      while((l_atm=(AjPAtom *)ajListIterNext(iter_loa)))
	AJFREE(l_atm);
      ajListIterFree(iter_loa);


      ajListDel(&list_heterogens);

      /* Free memory for protein pdb object */
      ajXyzPdbDel(&pdb);
      pdb=NULL;


      /* Free list of scop ids in pdb object */
      while(ajListstrPop(list_pdbscopids, &scopid_tmp))
	ajStrDel(&scopid_tmp);
      ajListstrDel(&list_pdbscopids);

      /* Free list of domain coordinate filenames */
      while(ajListstrPop(list_domfnames, &domfname_tmp))
	{
	  ajStrDel(&domfname_tmp);
	}
      ajListstrDel(&list_domfnames);
      ajListIterFree(iter_domfnames);

    }
  /* END OF MAIN APPLICATION LOOP - PER PROTEIN COORDINATE FILE */


  /* COMPLETE THE DBASE OBJECT - WRITE CONTACT DATA ARRAY FOR EACH
     HETEROGEN ENTRY IN THE OBJECT */
  for(i=0; i<(dbase)->n; i++)
  {
      (dbase)->entries[i]->no_dom=ajListToArray((dbase)->entries[i]->tmp,
			   (void ***) &(dbase)->entries[i]->cont_data);

      ajFmtPrint("dbase->entries[%d]->no_dom = %d\n",
		 i, (dbase)->entries[i]->no_dom);
  }



  /* WRITE OUTPUT FILE */
  funky_HeterogenContactsWrite(funky_out, dbase);


  /* Free ACD strings */
  ajStrDel(&prot);
  ajStrDel(&protextn);
  ajStrDel(&dom);
  ajStrDel(&domextn);
  ajFileClose(&escop_fptr);
  ajFileClose(&het_fptr);
  ajFileClose(&funky_out);
  ajFileClose(&vdwf);
  ajStrDel(&p_fname);
  ajStrDel(&msg);
  ajXyzVdwallDel(&vdw);
  /* scop classification*/
  while(ajListPop(list_allscop,(void **)&scoptemp))
    {
      ajXyzScopDel(&scoptemp);
    }
  ajListDel(&list_allscop);
  ajXyzHetDel(&hetdic);
  ajListstrDel(&pdbfile_list);


  /* Free dbase object */
  funky_DbaseDel(&dbase);

  ajFileClose(&logf);

  ajExit();
  return 0;
}



/* @funcstatic funky_DbaseNew *************************************************
**
** Constructor for Dbase object
**
** @param [r] n [ajint] number of entries in database
** @return [AjPDbase] Pointer to Dbase object
** @@
******************************************************************************/
static AjPDbase  funky_DbaseNew(ajint n)
{
    AjPDbase ret=NULL;

    AJNEW0(ret);

    ret->n=n;

    if(n)
    {
	AJCNEW0(ret->entries, n);
    }
    else
    {
	ajWarn("Arg with value zero passed to funky_DbaseNew");
	ret->entries=NULL;
    }
    return ret;
}


/* @funcstatic funky_DbaseDel *************************************************
**
** Destructor for Dbase object
**
** @param [r] ptr [AjPDbase*] Undocumented
** @return [void]
** @@
******************************************************************************/
static void funky_DbaseDel(AjPDbase *ptr)
{
  ajint i=0;

  /* Check arg's */
  if(ptr==NULL)
    {
      ajWarn("Attemp to free NULL pointer in funky_DbaseDel");
      return;
    }

  if(*ptr==NULL)
    {
      ajWarn("Attemp to free NULL pointer in funky_DbaseDel");
      return;
    }

  if((*ptr)->entries)
    {
	for(i=0;i<(*ptr)->n;i++)
	{
	    if((*ptr)->entries[i])
	    {
		funky_DbaseEntDel(&((*ptr)->entries[i]));
	    }
	}
	AJFREE((*ptr)->entries);
    }
  AJFREE((*ptr));
  *ptr=NULL;
  return;
}

/* @funcstatic funky_DbaseEntNew **********************************************
**
** Constructor for DbaseEnt object
**
** @param [r] n [ajint] number of entries in array of domain contact residues
** @return [AjPDbaseEnt] Pointer to DbaseEnt object
** @@
******************************************************************************/
static AjPDbaseEnt funky_DbaseEntNew(ajint n)
{
  AjPDbaseEnt ret=NULL;
  AJNEW0(ret);


  ret->abv=ajStrNew();
  ret->ful=ajStrNew();
  ret->tmp=ajListNew();
  ret->no_dom = n;


  if(n)
    {
      AJCNEW0(ret->cont_data, n);
    }
  else
    {
      /*ajWarn("Zero sized arg passed to funky_DbaseEntNew.\n");*/
      ret->cont_data = NULL;
    }
  return ret;
}

/* @funcstatic funky_DbaseEntDel **********************************************
**
** Destructor for DbaseEnt object
**
** @param [r] ptr [AjPDbaseEnt*] Undocumented
** @return [void]
** @@
******************************************************************************/
static void funky_DbaseEntDel(AjPDbaseEnt *ptr)
{
  ajint x=0;

  /* Check arg's */
  if(*ptr==NULL)
    {
      ajWarn("Attemp to free NULL pointer in funky_DbaseEntDel");
      return;
    }

  ajStrDel(&(*ptr)->abv);
  ajStrDel(&(*ptr)->ful);
  ajListDel(&(*ptr)->tmp);

  if((*ptr)->cont_data)
  {
      ajFmtPrint("no_dom = %d\n", (*ptr)->no_dom);


      for(x=0;x<(*ptr)->no_dom;x++)
      {
	  funky_DomContsDel(&((*ptr)->cont_data[x]));
      }
      AJFREE((*ptr)->cont_data);
  }


  AJFREE((*ptr));
  *ptr=NULL;

  return;
}


/* @funcstatic funky_DomContsNew **********************************************
**
** Constructor for DomConts object
**
** @param [r] n [ajint] no. of amino acids that make contact with the ligand
** @return [AjPDomConts] Pointer to DomConts object
** @@
******************************************************************************/
static AjPDomConts funky_DomContsNew(ajint n)
{
  AjPDomConts ret=NULL;
  ajint i=0;
  AJNEW0(ret);

  ret->scop_name=ajStrNew();
  ret->het_name=ajStrNew();
  ret->no_keyres=n;

  if(n)
    {
	AJCNEW0(ret->aa_code, n);
	AJCNEW0(ret->res_pos2, n);
      for(i=0;i<n;i++)
	{
	  ret->aa_code[i]=ajStrNew();
	  ret->res_pos2[i]=ajStrNew(); /* NEW */
	}
      ret->res_pos=ajIntNewL(n);
    }
  else
    {
      /* ajWarn("Zero sized arg passed to funky_DomContsNew.\n"); */
      ret->res_pos=ajIntNew();
      ret->aa_code=NULL;
      ret->res_pos2=NULL;              /* NEW */
    }
  return ret;
}


/* @funcstatic funky_DomContsDel **********************************************
**
** Destructor for DomConts object
**
** @param [r]  ptr [AjPDomConts*] pointer to AjPDomConts object
** @return [void]
** @@
******************************************************************************/
static void funky_DomContsDel(AjPDomConts *ptr)
{
  ajint i=0;

  /* Check arg's */
  if(*ptr==NULL)
    {
      ajWarn("Attemp to free NULL pointer in funky_DomContsDel");
      return;
    }
  ajStrDel(&(*ptr)->scop_name);
  ajStrDel(&(*ptr)->het_name);


  for(i=0;i<(*ptr)->no_keyres;++i)
  {
      ajStrDel(&(*ptr)->aa_code[i]);
      ajStrDel(&(*ptr)->res_pos2[i]); /* NEW */
  }
  if((*ptr)->aa_code)
      AJFREE((*ptr)->aa_code);
  if((*ptr)->res_pos2)
      AJFREE((*ptr)->res_pos2);           /* NEW */


  ajIntDel(&(*ptr)->res_pos);
  AJFREE(*ptr);
  *ptr=NULL;

  return;
}


/* @funcstatic funky_DichetToDbase ********************************************
**
** Function to populate the Dbase object (database of functional residues)
** with abbreviations and full names from the heterogen dictionary.
**
** @param [w] dbase  [AjPDbase*]   Pointer to Dbase object
** @param [r] hetdic [AjPHet]   Pointer to Dichet object
** @return [AjBool] True on success
** @@
******************************************************************************/
static AjBool      funky_DichetToDbase(AjPDbase *dbase, AjPHet hetdic)
{
  ajint i=0;

  /* Check args */
  if(!hetdic || !dbase)
    {
      ajWarn("NULL arg passed to funky_DichetToDbase\n");
      return ajFalse;
    }

  if(*dbase==NULL)
    {
      *dbase=funky_DbaseNew(hetdic->n);
      for(i=0; i< hetdic->n; i++)
	(*dbase)->entries[i] = funky_DbaseEntNew(0);

    }
  for(i=0;i<(hetdic)->n;++i)
    {
      ajStrAssS(&(*dbase)->entries[i]->abv, hetdic->entries[i]->abv);
      ajStrAssS(&(*dbase)->entries[i]->ful, hetdic->entries[i]->ful);
    }

  return ajTrue;
}


/* @funcstatic funky_CpdbListHeterogens ***************************************
**
** Function to create a list of array of Atom objects for ligands in
** the current Pdb object (a single array for each ligand)
**
** @param [r] pdb             [AjPPdb]   Pointer to pdb object
** @param [w] list_heterogens [AjPList*] Pointer to list of heterogen Atom
**                                       arrays
** @param [w] siz_heterogens  [AjPInt*]  Pointer to integer array of sizes
**                                       (number of Atom objects in each array)
** @param [w] nhet            [ajint*]   Number of arrays in the list that was
**                                       written.
** @param [r] logf            [AjPFile]  Log file
**
** @return [AjBool] ajTrue on success
** @@
** JISON added line in comments above for new arg
******************************************************************************/
static AjBool      funky_CpdbListHeterogens(AjPPdb pdb,
					    AjPList *list_heterogens,
					    AjPInt *siz_heterogens,
					    ajint *nhet, AjPFile logf )
{
 /* JISON added arg*/

  /* NOTE: EVERYTHING IN THE CLEAN PDB FILES IS CURRENTLY CHAIN
     ASSOCIATED! THIS WILL BE CHANGED IN FUTURE */

  /* Declare variables */

  AjIList    iter=NULL;		        /* Iterator for atoms in
                                           current pdb object */
  AjPAtom    hetat=NULL;		/* Pointer to current Atom object */
  ajint      i=0;			/* Counter for chains */
  ajint      prev_gpn=-10000;		/* Group number of atom object
                                           from previous iteration */
  AjPList    GrpAtmList=NULL;		/* List to hold atoms from the
                                           current group */
  AjPAtom    *AtmArray=NULL;		/* Array of atom objects */
  ajint      n=0;			/* number of elements in AtmArray */
  ajint      grp_count=0;		/* No. of groups */
  ajint      arr_count=0;               /* Index for siz_heterogens */

  /* Check args */
  if((pdb==NULL)||(list_heterogens==NULL)||(siz_heterogens==NULL))
    {
      ajWarn("Bad args passed to funky_CpdbListHeterogens\n");
      return ajFalse;
    }

  if((!(*list_heterogens))||(!(*siz_heterogens)))
    {
      ajWarn("Bad args passed to funky_CpdbListHeterogens\n");
      return ajFalse;
    }

  if(pdb->Ngp>0) { ajFmtPrintF(logf, "\tNGP:%d\n", pdb->Ngp); }

  if(pdb->Nchn>0)
    {
      /* ajFmtPrint("          Entry has %d chain(s)\n", pdb->Nchn); */
      for(i=0;i<pdb->Nchn;++i)
	{
	 /*  if(pdb->Chains[i]->Nlig==0) {continue;} */
	  prev_gpn=-100000;		/* Reset prev_gpn for each chain */
	  /* ajFmtPrint("          Chain %d\t %d group(s)\n",
                        i, pdb->Chains[i]->Nlig);  */
	  /* initialise iterator for pdb->Chains[i]->Atoms */
	  iter=ajListIter(pdb->Chains[i]->Atoms);
	  /* Iterate through list of Atom objects */
	  while((hetat=(AjPAtom)ajListIterNext(iter)))
	    {
	      /* check for type  */
	      if(hetat->Type != 'H') { continue; }
	      /* TEST FOR A NEW GROUP */
	      if(prev_gpn != hetat->Gpn)
		{
		  grp_count++;
		  /* ajFmtPrint("          *****NEW GROUP %d*****\n",
                                grp_count); */
		  if(GrpAtmList)
		    {
		  /* ajFmtPrint("            Converting previous group (%d)\n",
                                grp_count-1); */
		      n=(ajListToArray(GrpAtmList, (void ***) &AtmArray));
		      ajListPushApp(*list_heterogens, AtmArray);
       /* ajFmtPrint("            AtmArray for group %d contains %d Atom(s)\n",
                     grp_count-1,  n); */
		      ajIntPut(siz_heterogens, arr_count, n);
		      (*nhet)++;
		      ajListDel(&GrpAtmList);
		      GrpAtmList=NULL;
		      arr_count++;
		    }
		  GrpAtmList=ajListNew();
		  prev_gpn=hetat->Gpn;
		} /* End of new group loop */
	      ajListPushApp(GrpAtmList, (AjPAtom) hetat);
	    } /* End of list iteration loop */

	  /* Free list iterator */
	  ajListIterFree(iter);


	  /*  ajFmtPrint("  Group %d exists\n", grp_count);    */

	} /* End of chain for loop */

      if(GrpAtmList)
	{
         /* ajFmtPrint("          Converting last group (%d)\n", grp_count); */
	    n=(ajListToArray(GrpAtmList, (void ***) &AtmArray));
	  ajListPushApp(*list_heterogens, AtmArray);
  /* ajFmtPrint("          AtmArray for last group (%d) contains %d Atom(s)\n",
                grp_count,  n); */
	  ajIntPut(siz_heterogens, arr_count, n);
	  (*nhet)++;
	  ajListDel(&GrpAtmList);
	  GrpAtmList=NULL;
	}

      /* ajFmtPrint("            GrpAtmList created for group %d\n",
                    grp_count); */
      GrpAtmList=NULL;
      prev_gpn = -10000;

    } /* End of chain loop */


  return ajTrue;
}


/* @funcstatic funky_PdbGetDomain *********************************************
**
** Function to create a list of SCOP domains in the current Pdb object
** using SCOP classification
**
** @param [r] pdb             [AjPPdb]   Pointer to pdb object
** @param [r] list_allscop    [AjPList]  Pointer to SCOP list of SCOP
**                                       classification objects
** @param [w] list_pdbscopids [AjPList*] Pointer to list of scop domain ids in
**                                       the current pdb object
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/
static AjBool      funky_PdbGetDomain(AjPPdb pdb, AjPList list_allscop,
				      AjPList *list_pdbscopids)
{

  AjIList iter=NULL; /* List iterator for SCOP classification list */
  AjPScop ptr=NULL;
  AjPStr  tmpPdbId=NULL;
  AjPStr  tmpDomId=NULL;
  ajint   found=0;

  iter=ajListIter(list_allscop);

  /* ajFmtPrint("  GetDomains for PDB entry %S\n", pdb->Pdb); */
  fflush(stdout);
  while((ptr=(AjPScop)ajListIterNext(iter)))
    {
      ajStrAssS(&tmpPdbId, ptr->Pdb);
      ajStrToLower(&tmpPdbId);
      /*ajFmtPrint("PDB file: %S  Domain %S\n", pdb->Pdb, temp);*/
      if(ajStrMatch(pdb->Pdb, tmpPdbId))
	{
	  ajStrAssS(&tmpDomId, ptr->Entry);
	  ajStrToLower(&tmpDomId);
	  /* ajFmtPrint("   --PDB file %S.pxyz contains Domain %S\n",
                        pdb->Pdb, tmpDomId); */
	  ajListPushApp(*list_pdbscopids, tmpDomId);
	  /* ajFmtPrint("/////just pushed %S\n", tmpDomId); */
	  tmpDomId=NULL;
	  found=1;
	}
    }
  ajListIterFree(iter);
  ajStrDel(&tmpPdbId);
  ajStrDel(&tmpDomId);

  if(found==1)
    {
      return ajTrue;
    }
  else
    {
      return ajFalse;
    }
}


/* @funcstatic funky_HeterogenContacts ****************************************
**
** Function to calculate contacts between domains and ligands
**
** @param [r] dist_thresh        [float]     Undocumented
** @param [r] list_domains       [AjPList]   Pointer to list of domain Atom
**                                           arrays
** @param [r] list_pdbscopids    [AjPList]   Undocumented
** @param [r] list_heterogens    [AjPList]   Pointer to list of heterogen
**                                           Atom arrays
** @param [r] siz_domains        [AjPInt]    Undocumented
** @param [r] siz_heterogens     [AjPInt]    Undocumented
** @param [r] vdw                [AjPVdwall] van der Waals radii data
** @param [w] dbase              [AjPDbase*] Pointer to Dbase object
**
**
** @return [AjBool] ajTrue on success
** @@
******************************************************************************/

static AjBool funky_HeterogenContacts(float dist_thresh,
				      AjPList list_domains,
				      AjPList list_pdbscopids,
				      AjPList list_heterogens,
				      AjPInt siz_domains,
				      AjPInt siz_heterogens,
				      AjPVdwall vdw, AjPDbase *dbase)
{

  AjIList iter_dom=NULL;		/* Iterator for domain atoms LOA*/
  AjIList iter_het=NULL;		/* Iterator for heterogen atoms LOA */
  AjPAtom *dom_atm=NULL;		/* Current array of atom
                                           objects (for domains) */
  AjPAtom *het_atm=NULL;		/* Current array of atom
                                           objects (for heterogens) */
  AjPStr  *scopidArr=NULL;		/* Array of scop ids */
  ajint   scopctr=0;                    /* Counter for looping through
                                           array of scop ids */

  AjPDomConts cont_dataTemp=NULL;	/* Temporary pointer for
                                           current heterogen:domain
                                           contact data */
  AjPStr   tempaa;                      /* Temp. pointer for amino acid code */
  AjPList aaTempList=NULL;		/* Temporary list for amino
                                           acid code */
  AjPStr  tempres_pos2=NULL;            /* NEW Temporary pointer for
                                           the original pdb residue
                                           positions */
  AjPList res_pos2TempList=NULL;        /* NEW Temporary list for
                                           residue positions from
                                           original pdb file */
  AjPList cont_dataList=NULL;		/* Temporary list of all
                                           AjPDomConts objects */


  AjPDomConts dom_cont=NULL;		/* Current DomConts object */


  ajint   DomIdx=0;			/* For index into siz_domains array */
  ajint   HetIdx=0;			/* For index into
                                           siz_heterogens array */
  ajint   dom_max=0;			/* Size of current domain
                                           atoms array */
  ajint   het_max=0;			/* Size of current heterogen
                                           atoms array */
  ajint   idx_tmp=0;			/* Current residue identifier */
  ajint   i=0;                          /* Counter for looping through
                                           domain atom array also used
                                           for looping through
                                           (*dbase)->entries */
  ajint   j=0;                          /* Counter for looping through
                                           heterogen atom array */

  ajint res_ctr=0;                      /* Counter for looping through
                                           aa_code */
  AjPStr temp=NULL;                     /* Temporary string for
                                           holding Hetrogen atom
                                           information - diagnostic */

  if((!dist_thresh) || (list_domains==NULL) || (list_pdbscopids==NULL)
     || (list_heterogens==NULL) || (list_pdbscopids==NULL)
     || (siz_domains==NULL) || (siz_heterogens==NULL) || (vdw==NULL)
     || (dbase==NULL))
    {
      ajWarn("funky_HeterogenContacts: Bad arguments passed to function\n");
      return ajFalse;
    }

  if(!(*dbase))
    {
      ajWarn("funky_HeterogenContacts: Bad arguments passed to function\n");
      return ajFalse;
    }



  ajListToArray(list_pdbscopids, (void ***) &scopidArr);

  /* Create Temporary list for all AjPDomConts - check this i.e. all? */
  cont_dataList=ajListNew();

  iter_dom=ajListIter(list_domains);
  while((dom_atm=(AjPAtom *)ajListIterNext(iter_dom)))
    {
      idx_tmp=0;
      scopctr++;			        /* increment scopidArr index */
      DomIdx++;			                /* increment
                                                   siz_domain index */
      HetIdx=0;			                /* initialise
                                                   siz_heterogens
                                                   index */
      iter_het=ajListIter(list_heterogens);     /* initialise iterator
                                                   for list_heterogens */
      dom_max=ajIntGet(siz_domains, DomIdx-1);  /* Get size of current
                                                   domain atom array */
      /* ajFmtPrint("Dom %S Idx %d Siz %d\n",
                    scopidArr[scopctr-1], DomIdx-1, dom_max); */
      while((het_atm=(AjPAtom *)ajListIterNext(iter_het)))
	{
	  temp=ajStrNew();
	  aaTempList=ajListNew();
	  res_pos2TempList=ajListNew();
	  /* ajFmtPrint("***New DomConts created for Dom %S:Het %d (pair %d) "
                        "- aaTempList also created\n",
                         scopidArr[scopctr-1], HetIdx, l++); */
	  cont_dataTemp=funky_DomContsNew(0);

	  HetIdx++;		/* increment siz_heterogens index */
	  het_max=ajIntGet(siz_heterogens, HetIdx-1); /* Get size of
                                                         current
                                                         heterogen
                                                         atom array */
	  /* ajFmtPrint("Dom %S Size %d  Het %d Size %d\n",
	     scopidArr[scopctr-1], dom_max, HetIdx-1, het_max); */
	  /* loop through current domain atoms array */
	  for(i=0;i<dom_max;++i)
	    {
	      if(dom_atm[i]->Idx==idx_tmp)
		{
		  continue;
		}
	      /*  ajFmtPrint("\tDom %S Atom %d X:%.3f Y:%.3f Z:%.3f\n",
		  scopidArr[scopctr-1], i, dom_atm[i]->X, dom_atm[i]->Y,
		  dom_atm[i]->Z); */

	      /* loop through current hetetrogen atoms array */
	      for(j=0;j<het_max;j++)
		{
		  ajStrAssS(&(cont_dataTemp)->scop_name,
			    scopidArr[scopctr-1]);
		  /* Check with jon that the group numbers have been
                     assigned correctly here, temp can be used in
                     place of het_atm[j]->Id3 for diagnostics */
		  ajFmtPrintS(&temp, "Heterogen %S Chain %d Group %d",
			      het_atm[j]->Id3, het_atm[j]->Chn,
			      het_atm[j]->Gpn);

		  ajStrAssS(&(cont_dataTemp)->het_name, het_atm[j]->Id3);
		  if(ajXyzInContact(dom_atm[i], het_atm[j], dist_thresh, vdw))
		    {

	           /* ajFmtPrint("\tCONTACT: Domain %S atm %d Res. %d -- "
		      "Heterogen %S Chain %d Group %d atm %d\n",
		      scopidArr[scopctr-1], i, dom_atm[i]->Idx,
		      het_atm[j]->Id3, het_atm[j]->Chn,
		      het_atm[j]->Gpn, j); */
		      (cont_dataTemp)->no_keyres++;
		      ajIntPut(&(cont_dataTemp)->res_pos,
			       ((cont_dataTemp)->no_keyres)-1,
			       dom_atm[i]->Idx);
		      /* ajFmtPrint("\tPushed residue %S onto aaTempList\n",
			 dom_atm[i]->Id3); */
		      tempaa=ajStrNew();
		      ajStrAssS(&tempaa, dom_atm[i]->Id3);
		      /*JISON  ajListPushApp(aaTempList,
			(void *) dom_atm[i]->Id3);	*/
		      ajListPushApp(aaTempList, (void *) tempaa);
		      tempres_pos2=ajStrNew();
		      ajStrAssS(&tempres_pos2, dom_atm[i]->Pdb);
		      ajListPushApp(res_pos2TempList, tempres_pos2);
		      idx_tmp=dom_atm[i]->Idx;
		      break;		/* breaks out of the heterogen
                                           atoms array loop */

		    } /* if ajXyzInContact */
		  ajStrDel(&temp);
		} /* Current heterogen atoms array loop */
	    } /* Current domain atoms array loop */


	  if(cont_dataTemp->no_keyres > 0)
	    {
	      ajListToArray(aaTempList, (void ***) &(cont_dataTemp)->aa_code);
	      ajListToArray(res_pos2TempList,
			    (void ***) &(cont_dataTemp)->res_pos2);
	      ajListPushApp(cont_dataList, cont_dataTemp);
	      cont_dataTemp=NULL;
	    }
	  else
	    funky_DomContsDel(&cont_dataTemp);

	  ajListDel(&aaTempList);
	  ajListDel(&res_pos2TempList);

	  /* cont_dataTemp=NULL; */

	} /* while het_atm */

      ajListIterFree(iter_het);

    } /* while dom_atm */

  res_ctr=0;

  /* ajFmtPrint("Write the dabase object\n"); */
  while(ajListPop(cont_dataList,(void **)&dom_cont))
    {
      for(i=0; i<(*dbase)->n; i++)
	{
	  /* ajFmtPrint("dom_cont -%S- dbase -%S-\n", dom_cont->het_name,
	     (*dbase)->entries[i]->abv); */
	  if(ajStrMatch(dom_cont->het_name, (*dbase)->entries[i]->abv))
	    {
	     /* ajFmtPrint("dom_cont %S---dbase %S\n",
		dom_cont->het_name, (*dbase)->entries[i]->abv); */
	      ajListPushApp((*dbase)->entries[i]->tmp, dom_cont);
	    }
	}
    }

  ajListDel(&cont_dataList);
  ajListIterFree(iter_dom);
  AJFREE(scopidArr);


  return ajTrue;

}


/* @funcstatic funky_HeterogenContactsWrite ***********************************
**
** Function to write Dbase object to file i.e. the database of
** functional residues
**
** @param [w] funky_out    [AjPFile]  Pointer to output file
** @param [r] dbase        [AjPDbase] Dbase object
**
**
** @return [AjBool] True on success
** @@
******************************************************************************/

static AjBool    funky_HeterogenContactsWrite(AjPFile funky_out,
					      AjPDbase dbase)
{

  ajint i=0;       /* loop counter for dbase->entries[i] */
  ajint j=0;       /* loop counter for dbase->entries[i]->cont_data[j] */
  ajint k=0;       /* loop counter for
		      dbase->entries[i]->cont_data[j]->aa_code[j] and
		      dbase->entries[i]->cont_data[j]->res_pos[j] */



  /* Check arguments */
  if((funky_out==NULL) || (dbase==NULL))
    return ajFalse;




  for(i=0;i<dbase->n;i++)
    {
      if((dbase)->entries[i]->no_dom >0)
	{
	  ajFmtPrintF(funky_out, "ID   %S\n", dbase->entries[i]->abv);
	  ajFmtPrintF(funky_out, "DE   %S\n", dbase->entries[i]->ful);
	  ajFmtPrintF(funky_out, "ND   %d\n", dbase->entries[i]->no_dom);
	  for(j=0;j<(dbase)->entries[i]->no_dom; j++)
	    {
	      ajFmtPrintF(funky_out, "DN   %d\n", j+1);
	      ajFmtPrintF(funky_out, "XX\n");
	      ajFmtPrintF(funky_out, "SC   %S\n",
			  dbase->entries[i]->cont_data[j]->scop_name);
	      ajFmtPrintF(funky_out, "XX\n");
	      if(dbase->entries[i]->cont_data[j]->no_keyres > 0)
		{
		  ajFmtPrintF(funky_out, "NR   %d\n",
			      dbase->entries[i]->cont_data[j]->no_keyres);
		  ajFmtPrintF(funky_out, "XX\n");

		  for(k=0; k< dbase->entries[i]->cont_data[j]->no_keyres; k++)
		    {
		      ajFmtPrintF(funky_out, "RE   %S %d %S\n",
				dbase->entries[i]->cont_data[j]->aa_code[k],
			 ajIntGet(dbase->entries[i]->cont_data[j]->res_pos, k),
				dbase->entries[i]->cont_data[j]->res_pos2[k]);
		    }
		  ajFmtPrintF(funky_out, "XX\n");
		  ajFmtPrintF(funky_out, "//\n");
		}
	    }
	}
    }

  return ajTrue;
}





/* @funcstatic funky_HetTest **************************************************
**
** Undocumented
**
** @param [r] pdb_ptr [AjPPdb] Undocumented
**
** @return [AjBool] True on success
** @@
******************************************************************************/
static AjBool funky_HetTest(AjPPdb pdb_ptr)
{
  /* False = no hets found */
  /* True = hets found */
  ajint i=0;

  if(pdb_ptr->Nchn>0)
    {
      for(i=0;i<pdb_ptr->Nchn;++i)
	{
	  if(pdb_ptr->Chains[i]->Nlig>0)
	    {
	      return ajTrue;
	    }
	}
    }

  if(pdb_ptr->Ngp>0)
    {
      return ajTrue;
    }

  return ajFalse;
}

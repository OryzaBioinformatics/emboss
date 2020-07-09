/* @source psiblasts application
**
** Runs PSI-BLAST given a seed alignment
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
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
** 
** 
** 
** Operation 
** 
** psiblasts parses a stamp alignment annotated with SCOP classification 
** records, such as those generated by the EMBOSS applications scopalign, and 
** using the alignment as a seed, calls PSIBLAST to search a sequence database. 
** It writes a file of hits (relatives to proteins of each alignment) in 
** embl-like format. 
** 
** PSIBLAST runs on each alignment in a specified directory. The output files 
** are parsed and those hits achieving less than a specified threshold E-value 
** are written to the file of hits. 
** 
** The output file (Figure 1) uses the following records 
** (1) DE - bibliographic information. The text 'Members of scop families' is 
** always given. 
** (2) EX - experimental information. The value of the threshold E-value is 
** is given as a floating point number after 'THRESH'. The name of the residue 
** substitution matrix is given after 'MATRIX'. 
** The following four SCOP classification records are taken from the alignment 
** input file: 
** (3)  CL - Domain class.  It is identical to the text given after 'Class' in 
** the scop classification file (see documentation for the EMBOSS application 
** scope). 
** (4)  FO - Domain fold.  It is identical to the text given after 'Fold' in 
** the scop classification file (see scope documentation). 
** (5)  SF - Domain superfamily.  It is identical to the text given after 
** 'Superfamily' in the scop classification file (see scope documentation). 
** (6)  FA - Domain family. It is identical to the text given after 'Family' in 
** the scop classification file (see scope documentation). 
** (7)  NS - Number in set. The number of hits for this family. The file will 
** have a section containing an NN, AC, CL, RA and SQ records (see below) for 
** each sequence in the set. 
** (8) NN - Sequence number.  The number given in brackets after this record 
** indicates the start of the data for the relevent sequence. 
** (9) AC - Accession number of the hit. 
** (10) TY - Classification of hit.  Has the value PSIBLAST (for sequences retrieved 
** by psi-blast) or TRAIN (if the sequence was included in the seed alignment). 
** (Files of this type are also generated by the EMBOSS application swissparse 
** and may be be hand edited with additional sequences, in either case, the 
** value OTHER will be given for this record). 
** (11) RA - Sequence range. The numbers before START and END give the start and 
** end positions respectively of the hit relative to the full length sequence 
** in the swissprot database (a '.' may be given for swissparse output files - 
** see swissparse  documentation). 
** (12)  SQ - protein sequence. The number of residues is given before AA on the 
** first line. The protein sequence is given on subsequent lines. 
** (13) XX - used for spacing. 
** (14) // - used to delimit data for each psiblast run (family).
** 
** 
** Figure 1  Excerpt from psiblasts output file 
** 
**  DE   Members of scop families 
**  XX 
**  EX   THRESH 0.001; MATRIX BLOSUM62; 
**  XX 
**  CL   All alpha proteins 
**  XX 
**  FO   Globin-like 
**  XX 
**  SF   Globin-like 
**  XX 
**  FA   Globins 
**  XX 
**  NS   1 
**  XX 
**  NN   [1] 
**  XX 
**  AC   HBDEX1 
**  XX 
**  CL   PSIBLAST
**  XX 
**  RA   2 START; 79 END; 
**  XX 
**  SQ   SEQUENCE   141 AA;  15127 MW;  5EC7DB1E CRC32; 
**       VLSPADKTNV KAAWGKVGAH AGEYGAEALE RMFLSFPTTK TYFPHFDLSH GSAQVKGHGK 
**       KVADALTNAV AHVDDMPNAL SALSDLHAHK LRVDPVNFKL LSHCLLVTLA AHLPAEFTPA 
**       VHASLDKFLA SVSTVLTSKY R 
**  XX 
**  NN   [2] 
**  XX 
**  AC   HBDEX2 
**  XX 
**  CL   PSIBLAST 
**  XX 
**  RA   2 START; 79 END; 
**  XX 
**  SQ   SEQUENCE   141 AA;  15127 MW;  5EC7DB1E CRC32; 
**       VLSPADKTNV KAAWGKVGAH AGEYGAEALE RMFLSFPTTK TYFPHFDLSH GSAQVKGHGK 
**       KVADALTNAV AHVDDMPNAL SALSDLHAHK LRVDPVNFKL LSHCLLVTLA AHLPAEFTPA 
**       VHASLDKFLA SVSTVLTSKY R 
**  XX 
**  // 
**  CL   All alpha proteins 
**  XX 
**  FO   Globin-like 
**  XX 
**  SF   Globin-like 
**  XX 
**  FA   Phycocyanins 
**  XX 
** 
** 
** 
**  Notes 
** 
**  Must implement better way of producing value of TRAIN for TY record - 
**  which uses file of correspondence between SCOP domains and SWISPROT 
**  sequences ?   At the moment, if a hit is an exact substring of at 
**  least one of the sequences from the SCOP alignment, then it is taken
**  to be a member of the training set.
**
** 
**  Important Note
**
**  WHEN RUNNING PSIBLASTS AT THE HGMP IT IS ESSENTIAL THAT THE COMMAND 
**  'use blast_v2' (which runs the script /packages/menu/USE/blast_v2) IS GIVEN 
**  BEFORE IT IS RUN. 
**  psiblasts is hard-coded to give to scan swissprot, (-d swissprot  option 
**  to blastpgp. This is probably specific to use on the HGMP server. Option
**  to run psiblasts on any blast-indexed database will be implemented in the 
**  future (acd entry and code is below).
**  string: database 	
**  [ 
**  	  param: Y
**  	  prompt: "BLAST database to search"
** 	  def: "./swissprot"
**  ]
**
**  database   = ajAcdGetString("database");
**  

******************************************************************************/ 
        

#include "emboss.h"




static AjPHitlist psiblasts_ajXyzHitlistPsiblast(AjPScopalg scopalg,
						 AjPFile psif);
static AjPFile psiblasts_ajXyzScopalgPsiblast(AjPScopalg scopalg,
					      AjPFile alignf, 
					      AjPStr *psiname, ajint niter, 
					      ajint maxhits, float evalue, 
					      AjPStr database);






/* @prog psiblasts *******************************************************
**
** Testing
**
******************************************************************************/

int main(int argc, char **argv)
{
    AjPStr     align     = NULL;   /* Location of alignment files for input */
    AjPStr     alignextn = NULL;   /* File extension of alignment files */
    AjPStr     alignname = NULL;   /* Name of alignment file */
    AjPStr     database  = NULL;   /* Name of BLAST database to search */ 
    AjPStr     submatrix = NULL;   /* Name of residue substitution matrix */
    AjPStr     msg       = NULL;   /* Error message */
    AjPStr     temp      = NULL;   /* Temp string */
    AjPStr     psiname   = NULL;   /* Name of psiblast output file */

    AjPFile    families  = NULL;   /* Name of families file for output */
    AjPFile    logf      = NULL;   /* Log file pointer */
    AjPFile    psif      = NULL;   /* Pointer to psiblast output file*/
    AjPFile    alignf    = NULL;   /* Alignment file pointer */

    ajint      maxhits   = 0;	   /* Maximum number of hits reported by PSIBLAST */          
    ajint      niter     = 0;	   /* Number of PSIBLAST iterations */          
    float      evalue    = 0.0;	   /* Threshold E-value for inclusion in family */

    AjPList    list      = NULL;   /* Used to hold list of names of files in 
				      align directory */
    AjPScopalg scopalg   = NULL;   /*Scop alignment from input file */
    AjPHitlist hitlist   = NULL;   /* Hitlist object for holding results of 
				      PSIBLAST hits*/


    

    
    /* Initialise strings etc */
    alignname = ajStrNew();
    msg       = ajStrNew();
    temp      = ajStrNew();
    psiname   = ajStrNew();

    psif     = ajFileNew();
    alignf   = ajFileNew();
    
    list     = ajListNew();


    /* Read data from acd */
    embInit("psiblasts",argc,argv); 
    align      = ajAcdGetString("align");
    alignextn  = ajAcdGetString("alignextn");
    submatrix  = ajAcdGetString("submatrix");
    maxhits    = ajAcdGetInt("maxhits");
    niter      = ajAcdGetInt("niter");
    evalue     = ajAcdGetFloat("evalue");
    families   = ajAcdGetOutfile("families");
    logf       = ajAcdGetOutfile("logf");
    
    
    /* Check directories */
    if(!ajFileDir(&align))
	ajFatal("Could not open alignments directory");
    
    
    /* Create list of files in align directory */
    ajStrAssC(&temp, "*");	
    if((ajStrChar(alignextn, 0)=='.'))
	ajStrApp(&temp, alignextn);    
    else
    {
	ajStrAppC(&temp, ".");    
	ajStrApp(&temp, alignextn);    
    }
    ajFileScan(align, temp, &list, ajFalse, ajFalse, NULL, NULL, ajFalse, NULL); 


        
    /* Write header info. of output file */	       
    ajFmtPrintF(families,"DE   Members of scop families\nXX\n"
		"EX   THRESH %.3f; MATRIX %S;\nXX\n",
		evalue,submatrix);




    
    /*Start of main application loop*/   
    while(ajListPop(list,(void **)&alignname))
    { 
	/* Open alignment file*/
	if((alignf = ajFileNewIn(alignname)) == NULL)
	{
	    ajFmtPrintS(&msg, "Could not open for reading %S", 
			alignname);
	    ajWarn(ajStrStr(msg));
	    ajFmtPrintF(logf, "WARN  Could not open for reading %S\n", 
			alignname);
	    continue;	    
	}
	

	/* Read alignment file */
	ajXyzScopalgRead(alignf, &scopalg);
	

	/* psiblasts is hard-coded to give to scan swissprot, (-d swissprot  option 
	   to blastpgp. This is probably specific to use on the HGMP server. Option
	   to run psiblasts on any blast-indexed database will be implemented in the 
	   future */
	ajStrAssC(&database, "swissprot");
	

	/* Generate input files for psiblast and callpsiblast */
	if(!(psif = psiblasts_ajXyzScopalgPsiblast(scopalg, alignf, &psiname, niter, 
						   maxhits, evalue, database)))
	    ajFatal("Error creating psiblast file");
	
		
	/*  Parse the Psi-Blast output file and write a Hitlist object */
	hitlist = psiblasts_ajXyzHitlistPsiblast(scopalg, psif);

	
	/* Close alignment file and delete psiblast output file*/
   	ajFileClose(&alignf);
	ajFmtPrintS(&temp, "rm %S", psiname);
	system(ajStrStr(temp));
	
	
	/* Write the Hitlist object to file */	       
	ajXyzHitlistWrite(families, hitlist);
	

	/* Free memory */
	ajXyzHitlistDel(&hitlist);
	ajXyzScopalgDel(&scopalg);
    }
    


        

    /*Tidy up and return */
    ajStrDel(&align);
    ajStrDel(&alignextn);
    ajStrDel(&alignname);
    ajStrDel(&psiname);
    ajStrDel(&database);
    ajStrDel(&submatrix);
    ajStrDel(&msg);
    ajStrDel(&temp);

    ajFileClose(&families);
    ajFileClose(&logf);
    ajFileClose(&psif);	
    ajFileClose(&alignf);		
    
    ajListDel(&list);

    ajExit();
    return 0;
}





/* @funcstatic psiblasts_ajXyzScopalgPsiblast *******************************
**
** Reads a Scopalg object and the corresponding alignment file. Calls psiblast
** to do a search for the SCOP family over a specified database. The psiblast 
** output file is created and a pointer to it provided.
**
** @param [r] scopalg    [AjPScopalg]  Alignment    
** @param [r] alignf     [AjPFile]     Alignment file
** @param [r] psiname    [AjPStr *]    Name of psiblast output file created
** @param [r] niter      [ajint]       No. psiblast iterations
** @param [r] maxhits    [ajint]       Maximum number of hits to generate
** @param [r] evalue     [float]       Threshold E-value for psiblast
** @param [r] database   [AjPStr]      Database name
**
** @return [AjPFile] Pointer to psiblast output file for reading or
** NULL for error.
** @@
** 
** Note
** When the library code function ScopalgWrite is written, we will no longer
** need to pass in a pointer to the alignment file itself. Write ScopalgWrite
** and modify this function accordingly - not urgent.
******************************************************************************/
static AjPFile psiblasts_ajXyzScopalgPsiblast(AjPScopalg scopalg,
					      AjPFile alignf, 
					      AjPStr *psiname, ajint niter,
					      ajint maxhits, 
					      float evalue,  AjPStr database)
{
    AjPStr    line      = NULL;	 /* Temp string for reading alignment file */
    AjPStr    name      = NULL;	 /* Base name of STAMP temp files */
    AjPStr    temp      = NULL;	 /* Temp. string */
    AjPStr    seqin     = NULL;	 /* Name of temp. file for PSIBLAST input file 
				    (single sequence in FASTA format from alignment */
    AjPStr    seqsin    = NULL;	 /* Name of temp. file for PSIBLAST input file 
				    (sequences alignment w/o scop records, 
				    'Post_similar' or 'Number' lines*/
    AjPStr   *seqs      = NULL;  /* Sequences from alignment */    

    AjPFile   seqsinf   = NULL;  /* File pointer for  PSIBLAST input file 
				    (multiple sequences)*/
    AjPFile   seqinf    = NULL;  /* File pointer for  PSIBLAST input file 
				    (single sequence)*/
    AjPFile   psif      = NULL;  /* Pointer to psiblast output file*/
    
    ajint     nseqs     = 0;     /* No. of sequences in alignment */
    ajint     x         = 0;     /* Loop counter */
    

    

    
    /* Rewind alignment file */
    ajFileSeek(alignf,0,SEEK_SET);



    /* Allocate strings */
    line      = ajStrNew();
    name      = ajStrNew();
    temp      = ajStrNew();
    seqin     = ajStrNew();
    seqsin    = ajStrNew();
    


    /* Read scopalg structure and extract sequences only */
    if(!(nseqs=ajXyzScopalgGetseqs(scopalg, &seqs)))
	ajFatal("ajXyzScopalgGetseqs returned 0 sequences in "
		"psiblasts_ajXyzScopalgPsiblast. Email jison@hgmp.mrc.ac.uk\n");


    /* Initialise random number generator for naming of temp. files
       and create  psiblast input files */
    ajRandomSeed();
    ajStrAssC(&name, ajFileTempName(NULL));
    ajStrAss(&seqsin, name);
    ajStrAppC(&seqsin, ".seqsin");
    ajStrAss(&seqin, name);
    ajStrAppC(&seqin, ".seqin");
    ajStrAss(psiname, name);
    ajStrAppC(psiname, ".psiout");


    seqsinf = ajFileNewOut(seqsin);
    seqinf = ajFileNewOut(seqin);
    

    /* Read alignment file and write psiblast input file
     of multiple sequences */
    while(ajFileReadLine(alignf,&line))
    {
	/* Ignore 'Number', 'Post_similar' and Scop classification lines */
	if((ajStrPrefixC(line,"Number")))
	    continue;
	else if (ajStrPrefixC(line,"Post_similar"))
	    continue;
	else if(ajStrPrefixC(line,"CL"))
	{
	    /*Print blank line at top of output file*/
	    ajFmtPrintF(seqsinf,"\n");	
	    continue;
	}
	else if(ajStrPrefixC(line,"FO"))
	    continue;
	else if(ajStrPrefixC(line,"SF"))
	    continue;
	else if(ajStrPrefixC(line,"FA"))
	    continue;
	else if(ajStrPrefixC(line,"XX"))
	    continue;
	else if(ajStrChar(line,1)=='\0')
	{ 
	    /* If we are on a blank line
	       Print blank line at top of output file*/
	    ajFmtPrintF(seqsinf,"\n");	
	    continue;
	}
	else
	    ajFmtPrintF(seqsinf,"%S\n",line);
    }
    
    

    /* Write psiblast input file of single sequence */    
    ajFmtPrintF(seqinf,">\n%S\n",seqs[0]);
    

    /* Close psiblast input files before psiblast opens them */
    ajFileClose(&seqinf);
    ajFileClose(&seqsinf);

    
    /* Run PSI-BLAST */
    ajFmtPrintS(&temp,"blastpgp -i %S -B %S -j %d -e %f -b %d -v %d -d %S > %S\n",
		seqin, seqsin, niter,evalue, maxhits, maxhits, database, *psiname);
    ajFmtPrint("%S\n", temp);
    system(ajStrStr(temp));
    
    

    /* Remove temp. files */
    ajFmtPrintS(&temp, "rm %S", seqin);
    system(ajStrStr(temp));
    ajFmtPrintS(&temp, "rm %S", seqsin);
    system(ajStrStr(temp));


    /* Tidy up */
    ajStrDel(&line);
    ajStrDel(&name);
    ajStrDel(&temp);
    ajStrDel(&seqin);	
    ajStrDel(&seqsin);
    for(x=0;x<nseqs;x++)
	ajStrDel(&seqs[x]);	
    AJFREE(seqs);
        

    /* Open psiblast output file and return pointer */
    psif = ajFileNewIn(*psiname);
    return psif;
}





/* @funcstatic psiblasts_ajXyzHitlistPsiblast ********************************
**
** Reads a psiblast output file and writes a Hitlist object containing the 
** hits.
**
** @param [r] scopalg   [AjPScopalg]  Alignment    
** @param [r] psif      [AjPFile]     psiblast output file 
**
** @return [AjPHitlist] Pointer to Hitlist object (or NULL on failure)
** @@
** 
******************************************************************************/
static AjPHitlist psiblasts_ajXyzHitlistPsiblast(AjPScopalg scopalg,
						 AjPFile psif)
{
    /* The hits are organised into blocks, each block contains hits to 
       a protein with a unique accession number.  
       Each hit in a block mnay be spread over multiple lines. nlines 
       is the number of the line (sequence fragment) we are currently on */


    AjPStr  line       = NULL;	/* Temp string */ 
    AjPStr  acc        = NULL;	/* Acc. number of hit*/ 
    AjPStr  fragseq    = NULL;	/* Sequence fragment */ 
    AjPStr  fullseq    = NULL;	/* Sequence of entire hit */ 
    AjPStr  *seqs      = NULL;  /* Sequences from alignment */

    ajint   start      = 0;     /* Start of hit */
    ajint   fragstart  = 0;     /* Start of sequence fragment */
    ajint   fragend    = 0;     /* End of sequence fragment */
    ajint   hitn       = 0;     /* The number of the hit we are on */
    ajint   nhits      = 0;	/* No. of hits in alignment file */
    ajint   fseekr     = 0;
    ajint   x          = 0;     /* Loop counter */
    ajint   nseqs      = 0;     /* No. of sequences in alignment */
    ajlong  offset     = 0;
    
    AjPHitlist hitlist = NULL;  /* Hitlist object for holding results 
				   of PSIBLAST hits*/





     /* Allocate strings etc */
    line      = ajStrNew();
    acc       = ajStrNew();
    fragseq   = ajStrNew();
    fullseq   = ajStrNew();
    

    /* Read scopalg structure and write array of sequences only */    
    if(!(nseqs=ajXyzScopalgGetseqs(scopalg, &seqs)))
	ajFatal("No sequences in alignment passed to psiblasts_ajXyzHitlistPsiblast. "
		"Email jison@hgmp.mrc.ac.uk\n");
    

    /* Calculate the number of hits */
    while(ajFileReadLine(psif,&line))
	if(ajStrFindCaseC(line,"score = ")>=0)
	    nhits++;
    fseekr = ajFileSeek(psif,offset,SEEK_SET);


    /* Allocate memory for Hitlist object */
    if(!(hitlist = ajXyzHitlistNew(nhits)))
	return(hitlist);

    
    /* Copy SCOP classification records*/
    /* Assign scop classification records from hitlist structure */
    ajStrAss(&hitlist->Class, scopalg->Class);
    ajStrAss(&hitlist->Fold, scopalg->Fold);
    ajStrAss(&hitlist->Superfamily, scopalg->Superfamily);
    ajStrAss(&hitlist->Family, scopalg->Family);

    
    /* Loop for whole of input file*/
    while(ajFileReadLine(psif,&line))
    {
	/* We've found a line beginning with > i.e. the start 
	   of a block of hits to a single protein*/
	if(ajStrPrefixC(line,">SW:"))
	{
	    /* Parse the accession number */
	    ajFmtScanS(line, "%*s %S", &acc);
	}
	/* We've found a line beginning with ' Score = ' i.e. the
	   start of data for a hit */
	else if(ajStrPrefixC(line," Score = "))
	{
	    /* Write hit structure, we've parsed  */
	    if(hitn!=0)
	    {
		hitlist->hits[hitn-1]->Start = start;
		hitlist->hits[hitn-1]->End = fragend;
		ajStrAss(&hitlist->hits[hitn-1]->Id, acc);
		ajStrAss(&hitlist->hits[hitn-1]->Seq, fullseq);
		ajStrDegap(&hitlist->hits[hitn-1]->Seq);
		ajStrAssC(&hitlist->hits[hitn-1]->Typeobj, "PSIBLAST");
		for(x=0;x<scopalg->N; x++)
		    if((ajStrFindCase(seqs[x], hitlist->hits[hitn-1]->Seq))>=0)
		    {
			ajStrAssC(&hitlist->hits[hitn-1]->Typeobj, "TRAIN");
			break;
		    }
	    }
	    

	    /* Reset the sequence of the full hit */
	    ajStrAssC(&fullseq, "");

	    /* Incremenet hit counter */
	    hitn++;
	}
	/* Line containing sequence segment of the hit */
	else if(ajStrPrefixC(line,"Sbjct: "))
	{
	    /* Parse the start, end and sequence of the fragment */
	    ajFmtScanS(line, "%*s %d %S %d", &fragstart, &fragseq, &fragend);

	    /* If this is the first fragment, get the start point */
	    if(!ajStrCmpC(fullseq, ""))
		start=fragstart;
   
	    /* Add fragment to end of sequence of full hit */
	    ajStrApp(&fullseq, fragseq);
	}
    }


    /* Write hit structure for last hit */
    if(hitn!=0)
    {
	hitlist->hits[hitn-1]->Start = start;
	hitlist->hits[hitn-1]->End = fragend;
	ajStrAss(&hitlist->hits[hitn-1]->Id, acc);
	ajStrAss(&hitlist->hits[hitn-1]->Seq, fullseq);
	ajStrDegap(&hitlist->hits[hitn-1]->Seq);
	ajStrAssC(&hitlist->hits[hitn-1]->Typeobj, "PSIBLAST");
	for(x=0;x<scopalg->N; x++)
	    if((ajStrFindCase(seqs[x], hitlist->hits[hitn-1]->Seq))>=0)
	    {
		ajStrAssC(&hitlist->hits[hitn-1]->Typeobj, "TRAIN");
		break;
	    }
    }

    /*Tidy up and return */
    ajStrDel(&line);
    ajStrDel(&acc);
    ajStrDel(&fragseq);
    ajStrDel(&fullseq);
    for(x=0;x<nseqs;x++)
	ajStrDel(&seqs[x]);	
    AJFREE(seqs);

    return hitlist;
}


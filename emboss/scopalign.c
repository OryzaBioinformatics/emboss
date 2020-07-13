/* @source scopalign application
**
** Generate alignments for families in a scop classification file by using STAMP.
**
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
******************************************************************************
** 
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
** Operation
** 
** scopalign parses a scop classification file in embl-like format generated by 
** the EMBOSS applications scope or nrscope, and domain coordinate files 
** generated by the EMBOSS application domainer, and calls stamp to generate 
** structural alignments for each SCOP family in turn.
** 
** VERY IMPORTANT NOTE 
** 1. Adaption of STAMP for domain codes
** scopalign will only run with with a version of stamp which has been modified
** so that pdb id codes of length greater than 4 characters are acceptable.
** This involves a trivial change to the stamp module getdomain.c (around line
** number 155), a 4 must be changed to a 7 as follows:
** temp=getfile(domain[0].id,dirfile,4,OUTPUT); 
** temp=getfile(domain[0].id,dirfile,7,OUTPUT); 
** 
** 2. Adaption of STAMP for larger datasets
** STAMP was failing to align a large dataset of all the available V set Ig 
** domains. The ver2hor module generated the following error:
** Transforming coordinates...
**  ...done.
** ver2hor -f ./scopalign-1022069396.11280.76.post > ./scopalign-1022069396.11280.out
** error: something wrong with STAMP file
**          STAMP length is 370, Alignment length is 422
**          STAMP nseq is 155, Alignment nseq is 155
**
** This was fixed by changing #define MAXtlen 200 to #define MAXtlen 2000 in 
** alignfit.h.
** 
** At the same time I changed the following as a safety measure:
** gstamp.c  : #define MAX_SEQ_LEN 10000    (was 2000)
** pdbseq.c  : #define MAX_SEQ_LEN 10000    (was 3000)
** defaults.h: #define MAX_SEQ_LEN 10000    (was 8000)
** defaults.h: #define MAX_NSEQ 10000       (was 1000)
** defaults.h: #define MAX_BLOC_SEQ 5000    (was 500)
** dstamp.h  : #define MAX_N_SEQ 10000      (was 1000)
** ver2hor.h : #define MAX_N_SEQ 10000      (was 1000)
**
**
** The modified code is kept on the HGMP file system in /packages/stamp/src2
** WHEN RUNNING SCOPALIGN AT THE HGMP IT IS ESSENTIAL THAT THE COMMAND 
** 'use stamp2' (which runs the script /packages/menu/USE/stamp2) IS GIVEN 
** BEFORE SCOPALIGN IS RUN.  This will ensure that the modified version of 
** stamp is used.
** 
** stamp searches for pdb files with a certain prefix, extension and path as 
** specified in the stamp "pdb.directories" file.  For the HGMP, this file is
** /packages/stamp/defs/pdb.directories and should look like :
**
** /data/pdb - -
** /data/pdb _ .ent
** /data/pdb _ .pdb
** /data/pdb pdb .ent
** /data/pdbscop _ _
** /data/pdbscop _ .ent
** /data/pdbscop _ .pdb
** /data/pdbscop pdb .ent
** ./ _ _
** ./ _ .ent
** ./ _ .ent.z
** ./ _ .ent.gz
** ./ _ .pdb
** ./ _ .pdb.Z
** ./ _ .pdb.gz
** ./ pdb .ent
** ./ pdb .ent.Z
** ./ pdb .ent.gz
** /data/CASS1/pdb/coords/ _ .pdb
** /data/CASS1/pdb/coords/ _ .pdb.Z
** /data/CASS1/pdb/coords/ _ .pdb.gz

** 
** The names of the output files are identical to the names of the families
** given in the SCOP classification records, except that if a file of a 
** certain name already exists, then an "_1", "_2" etc will be added as 
** appropriate.
**
** The format of the scopalign output file (Figure 1) is similar to the output
** file generated by stamp when issued with the following three types of 
** command:
** 
** (1) stamp -l ./stamps_file.dom -s -n 2 -slide 5 -prefix ./stamps_file -d 
** ./stamps_file.set;sorttrans -f ./stamps_file.scan -s Sc 2.5 > 
** ./stamps_file.sort;stamp -l ./stamps_file.sort -prefix ./stamps_file > 
** ./stamps_file.log
**
** (2) poststamp -f ./stamps_file.3 -min 0.5
** 
** (3) ver2hor -f ./stamps_file.3.post > ./stamps_file.out
**
** However, the SCOP classification records for the appopriate family are 
** written above the alignment, no dssp assignments are given, and only the
** 'Post similar' line is given. Also, 7 character domain identifier codes 
** taken from the scop classificaiton file are given.
** 
**
** 
** Figure 1 Example of scopalign output file
**
**  CL   All alpha proteins
**  XX
**  FO   Globin-like
**  XX
**  SF   Globin-like
**  XX
**  FA   Globins
**  XX
**  SI   1321321
**  XX
**  Number               10        20        30        40        50    
**  d1vrea_              LSAAQRQVVASTWKDIAgsdngAGVGKECFTKFLSAHHDMAAV f gFS
**  d3sdhb_      svydaaaqLTADVKKDLRDSWKVIG sd kKGNGVALMTTLFADNQETIGYfkrlGN
**  d3hbia_      svydaaaqLTADVKKDLRDSWKVIG sd kKGNGVALMTTLFADNQETIGYfkrlGN
**  d3sdha_      svydaaaqLTADVKKDLRDSWKVIG sd kKGNGVALMTTLFADNQETIGYfkrlGN
**  Post_similar --------11111111111111111-00-1111111111111111111111-0-111
**  
**  Number        60        70        80        90       100       110 
**  d1vrea_      GAS   dpGVADLGAKVLAQIGVAVSHLgDEGKMVAEMKAVGVRHKgygnkhIKAEY
**  d3sdhb_      VSQgmandKLRGHSITLMYALQNFIDQLdNPDDLVCVVEKFAVNHI  t rkISAAE
**  d3hbia_      VSQgmandKLRGHSITLMYALQNFIDQLdNPDDLVCVVEKLAVNHI  t rkISAAE
**  d3sdha_      VSQgmandKLRGHSITLMYALQNFIDQLdNPDDLVCVVEKFAVNHI  t rkISAAE
**  Post_similar 111---0011111111111111111111011111111111111111--0-0011111
**  
**  Number          120       130       140       150       160
**  d1vrea_      FEPlGASL LSAMEhriggkMNAAAKDAWAAAYADisgalisglqs
**  d3sdhb_      FGK INGPiKKVLA s k nFGDKYANAWAKLVAVvqa al     
**  d3hbia_      FGK INGPiKKVLA s k nFGDKYANAWAKLVAVvqa al     
**  d3sdha_      FGK INGPiKKVLA s k nFGDKYANAWAKLVAVvqa al     
**  Post_similar 111-1111-11111-0-0-1111111111111111100-00-----
**  
**
** Important Notes
** stamp will ignore (omit from the alignment and *not* replace with '-' or any
** other symbol) ANY residues or groups in a pdb file that
** (i) are not structured (i.e. do not appear in the ATOM records) or
** (ii) lack a CA atom, regardless of whether it is a known amino acid or not.
** 
** This means that the position (column) in the alignment cannot reliably be 
** used as the basis for an index into arrays representing the full length 
** sequences.
**
** stamp will however include in the alignment residues with a single atom
** only, so long as it is the CA atom.
*/





#include "emboss.h"


static void scopalign_ProcessStampFile(AjPStr in, AjPStr out, AjPScop scop);



/* @prog scopalign *******************************************************
**
** Generate alignments for SCOP families
**
******************************************************************************/

int main(int argc, char **argv)
{
    ajint     nfam      = 0;	/* Counter for the families */
    ajint     ncluster  = 0;	/* Counter for the number of clusters*/    
    ajint     x         = 0;  /* Counter */
    
/*    ajint     last_fam = NULL; */ /* SCOP Sunid of last family that was processed */
    AjPStr    last_fam  = NULL; /* Last family that was processed */
    AjPStr    exec      = NULL;	/* The UNIX command line to be executed*/
    AjPStr    out       = NULL;	/* Name of stamp alignment file */
    AjPStr    align     = NULL;	/* Name of sequence alignment file */
    AjPStr    alignc    = NULL;	/* Name of structure alignment file */
    AjPStr    log       = NULL;	/* Name of STAMP log file */
    AjPStr    dom       = NULL;	/* Name of file containing single domain*/
    AjPStr    set       = NULL;	/* Name of file containing set of domains*/
    AjPStr    scan      = NULL;	/* Name of temp. file used by STAMP*/
    AjPStr    sort      = NULL;	/* Name of temp. file used by STAMP*/
    AjPStr    name      = NULL;	/* Base name of STAMP temp files */
    AjPStr    line      = NULL;	/* Holds a line from the log file*/
    AjPStr    path      = NULL;	/* Path of sequence alignment files for output */
    AjPStr    extn      = NULL;	/* Extension of sequence alignment files for output */
    AjPStr    pathc     = NULL;	/* Path of structure alignment files for output */
    AjPStr    extnc     = NULL;	/* Extension of structure alignment files for output */
    AjPStr    temp      = NULL;	/* A temporary string */
    AjPStr    temp1     = NULL;	/* A temporary string */

    AjPFile   scopf     = NULL;	/* File pointer for original Escop.dat file */
    AjPFile   domf      = NULL;	/* File pointer for single domain file */
    AjPFile   setf      = NULL;	/* File pointer for domain set file */
    AjPFile   logf      = NULL;	/* File pointer for log file */

    AjPRegexp rexp      = NULL;	/*For parsing no. of clusters in log file*/
    AjPScop   scop      = NULL;	/* Pointer to scop structure */
    AjPScop   prevscop  = NULL;	/* Pointer to previous scop structure */





    /* Initialise strings etc*/
    last_fam = ajStrNew();
    exec     = ajStrNew();
    out      = ajStrNew();
    align    = ajStrNew();
    alignc   = ajStrNew();
    log      = ajStrNew();
    dom      = ajStrNew();
    set      = ajStrNew();
    scan     = ajStrNew();
    sort     = ajStrNew();
    name     = ajStrNew();
    line     = ajStrNew();
    temp     = ajStrNew();
    temp1    = ajStrNew();

    

    /* Read data from acd */
    embInit("scopalign",argc,argv);
    scopf     = ajAcdGetInfile("scopf");
    path      = ajAcdGetString("path");
    extn      = ajAcdGetString("extn");
    pathc     = ajAcdGetString("pathc");
    extnc     = ajAcdGetString("extnc");

    
    /* Check directory is OK*/
    if(!ajFileDir(&path))
	ajFatal("Could not open directory for output");
    if(!ajFileDir(&pathc))
	ajFatal("Could not open directory for output");


    /* Compile regular expression*/
    /*    rexp     = ajRegCompC("^(Cluster:)  ([0-9])"); */
    rexp     = ajRegCompC("^(Cluster:)");

    /* Initialise random number generator for naming of temp. files*/
    ajRandomSeed();
    ajStrAssC(&name, ajFileTempName(NULL));


    /* Create names for temp. files*/
    ajStrAssS(&log, name);	
    ajStrAppC(&log, ".log");
    ajStrAssS(&dom, name);	
    ajStrAppC(&dom, ".dom");
    ajStrAssS(&set, name);	
    ajStrAppC(&set, ".set");
    ajStrAssS(&scan, name);	
    ajStrAppC(&scan, ".scan");
    ajStrAssS(&sort, name);	
    ajStrAppC(&sort, ".sort");
    ajStrAssS(&out, name);	
    ajStrAppC(&out, ".out");


    /* Initialise last_fam with something that is not in SCOP*/
    ajStrAssC(&last_fam,"!!!!!");
    
    

    
    /* Open domain set file*/
    if(!(setf=ajFileNewOut(set)))
	ajFatal("Could not open domain set file\n");





    /* Start of main application loop*/
    while((ajXyzScopReadC(scopf, "*", &scop)))
    {
	/* A new family */
	if(ajStrMatch(last_fam, scop->Family)==ajFalse)
	{
	    /* If we have done the first family*/
	    if(nfam)
	    {
		/*Close domain set file*/
		ajFileClose(&setf);	
		


		/* Create the output file for the alignment - the name will
		   be the same as the Sunid for the SCOP family */

		ajStrFromInt(&temp, prevscop->Sunid_Family);
		ajStrAssS(&align, temp);	
		ajStrInsert(&align, 0, path);	
		ajStrApp(&align, extn);

		ajStrAssS(&alignc, temp);	
		ajStrInsert(&alignc, 0, pathc);	
		ajStrApp(&alignc, extnc);



		/* Create the output file for the alignment - the name will
		  be the same as the SCOP family but with ' ' and '&' 
		  replaced by '_'*/
		/*
		ajStrAssS(&align, last_fam);	
		ajStrSubstituteCC(&align, " ", "_");
		ajStrSubstituteCC(&align, "&", "_");
		ajStrInsert(&align, 0, path);	
		ajStrApp(&align, extn);
		*/
		
		/* If a file of that name exists, then append _1 or _2 etc 
		   as necessary until a unique name is found */
		/*
		ajStrAssS(&temp, align);	
		for(x=1;
		    (ajFileStat(&temp, AJ_FILE_R ) ||
		     ajFileStat(&temp, AJ_FILE_W ) ||
		     ajFileStat(&temp, AJ_FILE_X ));
		    x++)
		{
		    ajStrAssS(&temp, align);	
		    ajStrAppC(&temp, "_");
		    ajFmtPrintS(&temp1, "%d", x);
		    ajStrApp(&temp, temp1);
		}
		ajStrAssS(&align, temp);	
		*/




		/* Call STAMP */
		ajFmtPrintS(&exec,"stamp -l %S -s -n 2 -slide 5 -prefix "
			    "%S -d %S;sorttrans -f %S -s Sc 2.5 > %S;"
			    "stamp -l %S -prefix %S > %S\ntransform -f %S -g -o %S\n", 
			    dom, name, set, scan, sort, sort, name, log, sort, alignc); 


/*		ajFmtPrintS(&exec,"stamp -l %S -s -n 2 -slide 5 -prefix "
			    "%S -d %S;sorttrans -f %S -s Sc 2.5 > %S;"
			    "stamp -l %S -prefix %S > %S\n", 
			    dom, name, set, scan, sort, sort, name, log);  */


		ajFmtPrint("%S\n", exec);
		system(ajStrStr(exec));  
		

		/* Count the number of clusters in the log file*/
		if(!(logf=ajFileNewIn(log)))
		    ajFatal("Could not open log file\n");
		ncluster=0;
		while(ajFileReadLine(logf,&line))
		    if(ajRegExec(rexp,line))
			ncluster++;
		ajFileClose(&logf);	
		


				
		/* Call STAMP ... calculate two fields for structural 
		   equivalence using threshold Pij value of 0.5, see stamp 
		   manual v4.1 pg 27*/
		ajFmtPrintS(&exec,"poststamp -f %S.%d -min 0.5\n",
			    name, ncluster);
		ajFmtPrint("%S\n", exec);
		system(ajStrStr(exec));


		/* Call STAMP ... convert block format alignment into clustal 
		   format*/
/*
		ajFmtPrintS(&exec,"aconvert  -in b -out c <%S.%d.post > %S\n",
			    name, ncluster, out); */
		ajFmtPrintS(&exec,"ver2hor -f %S.%d.post > %S\n",
			    name, ncluster, out); 
		ajFmtPrint("%S\n", exec);
		system(ajStrStr(exec));


		/* Process STAMP alignment file and generate alignment file 
		   for output */
		scopalign_ProcessStampFile(out, align, prevscop);
		

		/* Remove all temporary files */
		for(x=1;x<ncluster+1;x++)
		{
		    ajFmtPrintS(&temp, "rm %S.%d", name, x);
		    ajSystem(&temp);
		}
		ajFmtPrintS(&temp, "rm %S.%d.post", name, ncluster);
		ajSystem(&temp);
		
		/* Open domain set file */
		if(!(setf=ajFileNewOut(set)))
		    ajFatal("Could not open domain set file\n");
	    }
	    
	    
	    	    	    	    
	    
	    /* Open, write and close domain file*/
	    if(!(domf=ajFileNewOut(dom)))
		ajFatal("Could not open domain file\n");
	    ajStrAssS(&temp, scop->Entry);
	    ajStrToLower(&temp);
	    ajFmtPrintF(domf, "%S %S { ALL }\n", temp, temp);
	    ajFileClose(&domf);	

	    
	    /* Increment family counter*/
	    nfam++;


	    /* Copy current family name to last_fam*/
	    ajStrAssS(&last_fam,scop->Family);

	    
	    /* Copy current scop pointer to prevscop */
	    ajXyzScopDel(&prevscop);
	    prevscop=NULL;
	    ajXyzScopCopy(&prevscop, scop);
	    /* prevscop = scop; */
	}
						
	
	/* Write to domain set file*/
	ajStrAssS(&temp, scop->Entry);
	ajStrToLower(&temp);
	ajFmtPrintF(setf, "%S %S { ALL }\n", temp, temp);

	ajXyzScopDel(&scop);
    }
    /* End of main application loop*/

    scop=prevscop;
    



    /* Start of code to process last family */
    /*Close domain set file*/
    ajFileClose(&setf);	
		


    /* Create the output file for the alignment - the name will
       be the same as the Sunid for the SCOP family */
    ajStrFromInt(&temp, scop->Sunid_Family);
    ajStrAssS(&align, temp);	
    ajStrInsert(&align, 0, path);	
    ajStrApp(&align, extn);

    ajStrAssS(&alignc, temp);	
    ajStrInsert(&alignc, 0, pathc);	
    ajStrApp(&alignc, extnc);
    

    /* Create the output file for the alignment*/
    /*
    ajStrAssS(&align, scop->Family);	
    ajStrSubstituteCC(&align, " ", "_");
    ajStrInsert(&align, 0, path);	
    ajStrApp(&align, extn);
    */

    /* If a file of that name exists, then append _1 or _2 etc 
       as necessary until a unique name is found */
    /*
    ajStrAssS(&temp, align);	
    for(x=1;
	(ajFileStat(&temp, AJ_FILE_R ) ||
	 ajFileStat(&temp, AJ_FILE_W ) ||
	 ajFileStat(&temp, AJ_FILE_X ));
	x++)
    {
	ajStrAssS(&temp, align);	
	ajStrAppC(&temp, "_");
	ajFmtPrintS(&temp1, "%d", x);
	ajStrApp(&temp, temp1);
    }
    ajStrAssS(&align, temp);	
    */




    /* Call STAMP */
    ajFmtPrintS(&exec,"stamp -l %S -s -n 2 -slide 5 -prefix %S -d %S;"
		"sorttrans -f %S -s Sc 2.5 > %S;stamp -l %S "
		"-prefix %S > %S\ntransform -f %S -g -o %S\n", 
		dom, name, set, scan, sort, sort, name, log, sort, alignc);  

/*    ajFmtPrintS(&exec,"stamp -l %S -s -n 2 -slide 5 -prefix %S -d %S;"
		"sorttrans -f %S -s Sc 2.5 > %S;stamp -l %S "
		"-prefix %S > %S\n", 
		dom, name, set, scan, sort, sort, name, log); */

    system(ajStrStr(exec));  
    ajFmtPrint("%S\n", exec);


    /* Count the number of clusters in the log file*/
    if(!(logf=ajFileNewIn(log)))
	ajFatal("Could not open log file\n");
    /*count the number of clusters*/
    ncluster=0;
    while(ajFileReadLine(logf,&line))
	if(ajRegExec(rexp,line))
	    ncluster++;
    ajFileClose(&logf);	
    		


    /* Call STAMP ... calculate two fields for structural equivalence */
    /* using threshold Pij value of 0.5, see stamp manual v4.1 pg 27*/
    ajFmtPrintS(&exec,"poststamp -f %S.%d -min 0.5\n",
		name, ncluster);
    ajFmtPrint("%S\n", exec);
    system(ajStrStr(exec));


    /* Call STAMP ... convert block format alignment into clustal format*/
    /*    ajFmtPrintS(&exec,"aconvert -in b -out c <%S.%d.post > %S\n",
	  name, ncluster, out); */
    ajFmtPrintS(&exec,"ver2hor -f %S.%d.post > %S\n",name, ncluster, out); 
    ajFmtPrint("%S\n", exec);
    system(ajStrStr(exec));
    

    /* Process STAMP alignment file and generate alignment file for output */
    scopalign_ProcessStampFile(out, align, scop);


    /* Remove all temporary files */
    ajFmtPrintS(&temp, "rm %S", log);
    ajSystem(&temp);
    ajFmtPrintS(&temp, "rm %S", dom);
    ajSystem(&temp);
    ajFmtPrintS(&temp, "rm %S", set);
    ajSystem(&temp);
    ajFmtPrintS(&temp, "rm %S", scan);
    ajSystem(&temp);
    ajFmtPrintS(&temp, "rm %S", sort);
    ajSystem(&temp);
    ajFmtPrintS(&temp, "rm %S", out);
    ajSystem(&temp);
    ajStrAssS(&temp, name);	
    ajStrAppC(&temp, ".mat");
    ajFmtPrintS(&temp1, "rm %S", temp);
    ajSystem(&temp1);

    /* Remove all temporary files */
    for(x=1;x<ncluster+1;x++)
    {
	ajFmtPrintS(&temp, "rm %S.%d", name, x);
	ajSystem(&temp);
    }
    ajFmtPrintS(&temp, "rm %S.%d.post", name, ncluster);
    ajSystem(&temp);
    
    /* Tidy up*/
    ajXyzScopDel(&scop);
    ajFileClose(&scopf);	
    ajRegFree(&rexp);
    ajStrDel(&last_fam);
    ajStrDel(&exec);
    ajStrDel(&log);
    ajStrDel(&dom);
    ajStrDel(&set);
    ajStrDel(&scan);
    ajStrDel(&sort);
    ajStrDel(&name);
    ajStrDel(&out);
    ajStrDel(&align);
    ajStrDel(&alignc);
    ajStrDel(&line);
    ajStrDel(&path); 
    ajStrDel(&extn); 
    ajStrDel(&pathc); 
    ajStrDel(&extnc); 
    ajStrDel(&temp); 
    ajStrDel(&temp1); 


    /* Bye Bye */
    ajExit();
    return 0;
}




/* @funcstatic scopalign_ProcessStampFile ***********************************
**
** This function is very specific to scopalign, hence it is not library code.
** This function reads the output of ver2hor, i.e. a stamp alignment (Figure 
** 1 below) and creates an output file which is annotated with SCOP records 
** (Figure 2 below).
** 
** 
** Figure 1
** 
** VER2HOR R.B. Russell, 1995
**  Prints STAMP alignments in horizontal format
**   for quick viewing
**  Reading Alignment...
**  Getting STAMP information...
**  6 STAMP fields read in for 547 positions 
**  Processing the alignment...
**  Output:
**  Very reliable => Pij' >=6 for stretches of >=3
**  Less reliable => Pij' >=4.5 for stretches of >=3
**  Post reliable => All Pij' > stamp_post parameter for stretches >=3
** 
** Number               10        20        30        40        50    
**   d1maac_    egrEDPQLLVRVRGGQLRGIRLKAPGGPVSAFLGIPFAEPPVGSRRFMPPEPKRPWS
** d1maad__1       EDPQLLVRVRGGQLRGIRLKAPGGPVSAFLGIPFAEPPVGSRRFMPPEPKRPWS
** 
** d1maac__ds   ?????????????????????????????????????????????????????????
** d1maad__1_      ??????????????????????????????????????????????????????
** 
** Very similar ---111111111111111111111111111111111111111111111111111111
** Less similar ---111111111111111111111111111111111111111111111111111111
** Post similar ---111111111111111111111111111111111111111111111111111111
** 
** Number        60        70        80        90       100       110 
**   d1maac_    GVLDATTFQNVCYQYVDTLYPGFEGTEMWNPNRELSEDCLYLNVWTPYPRPASPTPV
** d1maad__1    GVLDATTFQNVCYQYVDTLYPGFEGTEMWNPNRELSEDCLYLNVWTPYPRPASPTPV
** 
** d1maac__ds   ?????????????????????????????????????????????????????????
** d1maad__1_   ?????????????????????????????????????????????????????????
** 
** Very similar 111111111111111111111111111111111111111111111111111111111
** Less similar 111111111111111111111111111111111111111111111111111111111
** Post similar 111111111111111111111111111111111111111111111111111111111
** 
** Number          120       130       140       150       160       170
**   d1maac_    LIWIYGGGFYSGAASLDVYDGRFLAQVEGAVLVSMNYRVGTFGFLALPGSREAPGNV
** d1maad__1    LIWIYGGGFYSGAASLDVYDGRFLAQVEGAVLVSMNYRVGTFGFLALPGSREAPGNV
** 
** d1maac__ds   ?????????????????????????????????????????????????????????
** d1maad__1_   ?????????????????????????????????????????????????????????
** 
** Very similar 111111111111111111111111111111111111111111111111111111111
** Less similar 111111111111111111111111111111111111111111111111111111111
** Post similar 111111111111111111111111111111111111111111111111111111111
** 
** 
** 
** Figure 2
**
** CL   Alpha and beta proteins (a/b)
** XX
** FO   alpha/beta-Hydrolases
** XX
** SF   alpha/beta-Hydrolases
** XX
** FA   Acetylcholinesterase-like
** XX
** SI   1321321
** XX
** Number               10        20        30        40        50    
**   d1maac_    egrEDPQLLVRVRGGQLRGIRLKAPGGPVSAFLGIPFAEPPVGSRRFMPPEPKRPWS
**   d1maad_       EDPQLLVRVRGGQLRGIRLKAPGGPVSAFLGIPFAEPPVGSRRFMPPEPKRPWS
** Post similar ---111111111111111111111111111111111111111111111111111111
** 
** Number        60        70        80        90       100       110 
**   d1maac_    GVLDATTFQNVCYQYVDTLYPGFEGTEMWNPNRELSEDCLYLNVWTPYPRPASPTPV
**   d1maad_    GVLDATTFQNVCYQYVDTLYPGFEGTEMWNPNRELSEDCLYLNVWTPYPRPASPTPV
** Post similar 111111111111111111111111111111111111111111111111111111111
** 
** Number          120       130       140       150       160       170
**   d1maac_    LIWIYGGGFYSGAASLDVYDGRFLAQVEGAVLVSMNYRVGTFGFLALPGSREAPGNV
**   d1maad_    LIWIYGGGFYSGAASLDVYDGRFLAQVEGAVLVSMNYRVGTFGFLALPGSREAPGNV
** Post similar 111111111111111111111111111111111111111111111111111111111
** 
**
** @param [r] in  [AjPStr] Name of input file
** @param [r] out [AjPStr] Name of output file
** @param [r] scop [AjPScop] SCOP structure with SCOP classification records
**
** @return [void]
** @@
*****************************************************************************/

static void scopalign_ProcessStampFile(AjPStr in, AjPStr out, AjPScop scop)
{
    AjPFile  outf =NULL;  /* Output file pointer */
    AjPFile   inf =NULL;  /* Input file pointer */
    AjPStr  temp1 =NULL;  /* Temporary string */
    AjPStr  temp2 =NULL;  /* Temporary string */
    AjPStr  temp3 =NULL;  /* Temporary string */
    AjPStr   line =NULL;  /* Line of text from input file */
    ajint     blk =1;     /* Count of the current block in the input file.
			     Block 1 is the numbering and protein sequences, 
			     Block 2 is the secondary structure, 
			     Block 3 is the Very/Less/Post similar records*/
    
    
    /* Initialise strings */
    line    = ajStrNew();
    temp1    = ajStrNew();
    temp2    = ajStrNew();
    temp3    = ajStrNew();


    /* Open input and output files */
    if(!(outf=ajFileNewOut(out)))
	ajFatal("Could not open output file in scopalign_ProcessStampFile");
    if(!(inf=ajFileNewIn(in)))
	ajFatal("Could not open input file in scopalign_ProcessStampFile");
    

    /*Write SCOP classification records to file*/
    ajFmtPrintF(outf,"CL   %S",scop->Class);
    ajFmtPrintSplit(outf,scop->Fold,"\nXX\nFO   ",75," \t\n\r");
    ajFmtPrintSplit(outf,scop->Superfamily,"XX\nSF   ",75," \t\n\r");
    ajFmtPrintSplit(outf,scop->Family,"XX\nFA   ",75," \t\n\r");
    ajFmtPrintF(outf,"XX\n");
    ajFmtPrintF(outf,"SI   %d\nXX",scop->Sunid_Family);

    
    /* Start of code for reading input file */
    /*Ignore everything up to first line beginning with 'Number'*/
    while((ajFileReadLine(inf,&line)))
	/* ajFileReadLine will trim the tailing \n */
	if((ajStrChar(line, 1)=='\0'))
	    break;

    
    /* Read rest of input file */
    while((ajFileReadLine(inf,&line)))
    {
	/* Increment counter for block of file */
	if((ajStrChar(line, 1)=='\0'))
	{
	    blk++;
	    if(blk==4)
		blk=1;
	    
	    continue;
	}


	/* Block of numbering line and protein sequences */
	if(blk==1)
	{
	    /* Print the number line out as it is */
	    if(ajStrPrefixC(line,"Number"))
		ajFmtPrintF(outf,"\n%S\n",line);
	    else
	    {
		/* Read only the 7 characters of the domain identifier code in */
		ajFmtScanS(line, "%S", &temp1);
		ajStrAssSub(&temp2, temp1, 0, 6);


		/* Read the sequence */
		ajStrAssSub(&temp3, line, 13, 69);
		ajStrConvertCC(&temp3, " ", "-");
		ajStrToUpper(&temp3);
		

		/* Write domain id code and sequence out */
		ajFmtPrintF(outf,"%-13S%S\n",temp2, temp3);
	    }
	}
	/* Secondary structure filled with '????' (unwanted) */
	else if(blk==2)
	{
	    continue;
	}
	/* Similarity lines */
	else
	{
	    if(ajStrPrefixC(line,"Post"))
	    {
		/* Read the sequence */
		ajStrAssSub(&temp3, line, 13, 69);

		/* Write post similar line out */
		ajFmtPrintF(outf,"%-13s%S\n","Post_similar", temp3);
	    }
	    /* Ignore Very and Less similar lines */
	    else continue;
	    
	   
	    
	}
    }
    

    /* Clean up and close input and output files */
    ajFileClose(&outf);
    ajFileClose(&inf);
    ajStrDel(&line);
    ajStrDel(&temp1);
    ajStrDel(&temp2);
    ajStrDel(&temp3);
    

    /* All done */
    return;
}






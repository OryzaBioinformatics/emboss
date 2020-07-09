/* @source stamps application
**
** Runs STAMP over SCOP families
** @author: Copyright (C) Jon Ison (jison@hgmp.mrc.ac.uk)
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
** @author: Copyright (C) Alan Bleasby (ableasby@hgmp.mrc.ac.uk)
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






int main(int argc, char **argv)
{
    ajint       nfam      = 0;	/* Counter for the families */
    ajint       ncluster  = 0;	/* Counter for the number of clusters*/    

    AjPStr    last_fam  = NULL;	/* Last family that was processed */
    AjPStr    exec      = NULL;	/* The UNIX command line to be executed*/
    AjPStr    out       = NULL;	/* Name of alignment file */
    AjPStr    log       = NULL;	/* Name of STAMP log file */
    AjPStr    dom       = NULL;	/* Name of file containing single domain*/
    AjPStr    set       = NULL;	/* Name of file containing set of domains*/
    AjPStr    scan      = NULL;	/* Name of temp. file used by STAMP*/
    AjPStr    sort      = NULL;	/* Name of temp. file used by STAMP*/
    AjPStr    name      = NULL;	/* Base name of STAMP temp files */
    AjPStr    line      = NULL;	/* Holds a line from the log file*/
    AjPStr    path      = NULL;	/* Path of alignment files for output */
    AjPStr    temp      = NULL;	/* A temporary string */

    AjPFile   scopf     = NULL;	/* File pointer for original Escop.dat file */
    AjPFile   domf      = NULL;	/* File pointer for single domain file */
    AjPFile   setf      = NULL;	/* File pointer for domain set file */
    AjPFile   logf      = NULL;	/* File pointer for log file */

    AjPRegexp rexp      = NULL;	/*For parsing no. of clusters in log file*/
    AjPScop   scop      = NULL;	/* Pointer to scop structure */






    /* Initialise strings*/
    last_fam = ajStrNew();
    exec     = ajStrNew();
    out      = ajStrNew();
    log      = ajStrNew();
    dom      = ajStrNew();
    set      = ajStrNew();
    scan     = ajStrNew();
    sort     = ajStrNew();
    name     = ajStrNew();
    line     = ajStrNew();
    path     = ajStrNew();
    temp     = ajStrNew();


    /* Read data from acd */
    embInit("stamps",argc,argv);
    scopf     = ajAcdGetInfile("scopf");
    path      = ajAcdGetString("path");

    
    /* Check directory is OK*/
    if(!ajFileDir(&path))
	ajFatal("Could not open directory for output");


    /* Compile regular expression*/
    rexp     = ajRegCompC("^(Cluster:)  ([0-9])");


    /* Initialise random number generator for naming of temp. files*/
    ajRandomSeed();
    ajStrAssC(&name, ajFileTempName(NULL));


    /* Create names for temp. files*/
    ajStrAss(&log, name);	
    ajStrAppC(&log, ".log");
    ajStrAss(&dom, name);	
    ajStrAppC(&dom, ".dom");
    ajStrAss(&set, name);	
    ajStrAppC(&set, ".set");
    ajStrAss(&scan, name);	
    ajStrAppC(&scan, ".scan");
    ajStrAss(&sort, name);	
    ajStrAppC(&sort, ".sort");


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
	    /* Copy current family name to last_fam*/
	    ajStrAss(&last_fam,scop->Family);

	    /* If we have done the first family*/
	    if(nfam)
	    {
		/*Close domain set file*/
		ajFileClose(&setf);	
		

		/* Call STAMP */
		ajFmtPrintS(&exec,"stamp -l %S -s -n 2 -slide 5 -prefix "
			    "%S -d %S;sorttrans -f %S -s Sc 2.5 > %S;"
			    "stamp -l %S -prefix %S > %S\n", 
			    dom, name, set, scan, sort, sort, name, log);
		system(ajStrStr(exec));  
		ajFmtPrint("%S\n", exec);
		

		/* Count the number of clusters in the log file*/
		if(!(logf=ajFileNewIn(log)))
		    ajFatal("Could not open log file\n");
		ncluster=0;
		while(ajFileReadLine(logf,&line))
		    if(ajRegExec(rexp,line))
			ncluster++;
		ajFileClose(&logf);	
		
		
		/* Create the output file for the alignment - the name will
		  be the same as the SCOP family but with ' ' replaced by '_'*/
		ajStrAss(&out, scop->Family);	
		ajStrSubstituteCC(&out, " ", "_");
		ajStrInsert(&out, 0, path);	
		ajStrAppC(&out, ".align");

		
		/* Call STAMP */
		ajFmtPrintS(&exec,"ver2hor -f %S.%d > %S\n",
			    name, ncluster, out);
		system(ajStrStr(exec));
		ajFmtPrint("%S\n", exec);


		/* Open domain set file */
		if(!(setf=ajFileNewOut(set)))
		    ajFatal("Could not open domain set file\n");
	    }
	    /* Open, write and close domain file*/
	    if(!(domf=ajFileNewOut(dom)))
		ajFatal("Could not open domain file\n");
	    ajStrAss(&temp, scop->Entry);
	    ajStrToLower(&temp);
	    ajFmtPrintF(domf, "%S %S { ALL }\n", temp, temp);
	    ajFileClose(&domf);	

	    
	    /* Increment family counter*/
	    nfam++;
	}
	/* Write to domain set file*/
	ajStrAss(&temp, scop->Entry);
	ajStrToLower(&temp);
	ajFmtPrintF(setf, "%S %S { ALL }\n", temp, temp);
    }
    /* End of main application loop*/






    /*Code to process last family*/
    /*Close domain set file*/
    ajFileClose(&setf);	
		

    /* Call STAMP */
    ajFmtPrintS(&exec,"stamp -l %S -s -n 2 -slide 5 -prefix %S -d %S;"
		"sorttrans -f %S -s Sc 2.5 > %S;stamp -l %S "
		"-prefix %S > %S\n", 
		dom, name, set, scan, sort, sort, name, log);
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
    		

    /* Create the output file for the alignment*/
    ajStrAss(&out, scop->Family);	
    ajStrSubstituteCC(&out, " ", "_");
    ajStrAppC(&out, ".align");


    /* Call STAMP */
    ajFmtPrintS(&exec,"ver2hor -f %S.%d > %S\n",name, ncluster, out);
    system(ajStrStr(exec));
    ajFmtPrint("%S\n", exec);

    
    /* Tidy up*/
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
    ajStrDel(&line);
    ajStrDel(&path); 
    ajStrDel(&temp); 


    /* Bye Bye */
    ajExit();
    return 0;
}


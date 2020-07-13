/* @source hmmgen application
**
** Generates a hidden Markov model for each alignment in a directory.
**
** @author: Copyright (C) Ranjeeva Ranasinghe (rranasin@hgmp.mrc.ac.uk)
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
**
**
** OPERATION
**
** Need to type 'use hmmer' if application is used at or remotely from
** the HGMP.
**
******************************************************************************/

#include "emboss.h"


/* @prog hmmgen****** *********************************************************
**
** Generate hidden markov models using the hmmer package.
**
******************************************************************************/

int main(int argc, char **argv)
{

    AjPStr infpath    = NULL;      /* path to directory containing
                                      extended alignments */
    AjPStr infextn    = NULL;      /* file extension of extended
                                      alignment files */
    AjPStr outfpath   = NULL;      /* output directory for the HMM profiles */
    AjPStr outfextn   = NULL;      /* output file extension for HMM profiles */
    AjPStr filename   = NULL;      /* name of extended alignment file */
    AjPStr outfile    = NULL;      /* name of output file a HMM */
    AjPStr tmp        = NULL;      /* temporary string variable */
    AjPStr cmd        = NULL;      /* the unix command line to execute hmmer */
    AjPStr tmpname    = NULL;      /* randomly generated name for
                                      hmmer input */
    AjPStr seqsin     = NULL;      /* name of sequence file for input
                                      into hmmer in CLUSTAL format */

    AjPList list      = NULL;      /* a list of file names */

    AjPFile inf       = NULL;      /* file pointer for extn alignments */
    AjPFile seqsinf   = NULL;      /* file pointer of seqsin */

    AjPScopalg scopalg= NULL;      /* scopalg object for read in extn
                                      alignment files */

    ajint posdash     = 0;
    ajint posdot      = 0;

    embInit("hmmgen",argc,argv);

    infpath  = ajAcdGetString("infpath");
    infextn  = ajAcdGetString("infextn");
    outfpath = ajAcdGetString("outfpath");
    outfextn = ajAcdGetString("outfextn");

    filename = ajStrNew();
    outfile  = ajStrNew();
    tmp      = ajStrNew();
    cmd      = ajStrNew();
    tmpname  = ajStrNew();

    list = ajListNew();

    /* Check directories */
    if((!ajFileDir(&infpath)) || (!ajFileDir(&outfpath)) || (!(infextn)))
        ajFatal("Could not open extended alignment directory");

    /* Create list of files in the path */
    ajStrAssC(&tmp, "*");               /* assign a wildcard to tmp */

    if((ajStrChar(infextn, 0)=='.'))    /* checks if the file
                                           extension starts with "."  */
        ajStrApp(&tmp, infextn);        /* assign the acd input file
                                           extension to tmp */

    /* this picks up situations where the user has specified an
       extension without a "." */
    else
    {
        ajStrAppC(&tmp, ".");           /* assign "." to tmp */
        ajStrApp(&tmp, infextn);        /* append tmp with a user
                                           specified extension */
    }

    /* all files containing extended alignments will be in a list */
    ajFileScan(infpath, tmp, &list, ajFalse, ajFalse, NULL, NULL,
	       ajFalse, NULL);

    /* read each each extended alignment file and run prophecy to
       generate profile */
    while(ajListPop(list,(void **)&filename))
    {
        if((inf = ajFileNewIn(filename)) == NULL)
        {
            ajWarn("Could not open file %S\n",filename);
	    ajStrDel(&filename);
            continue;
        }

        else
        {
            /* Initialise random number generator for naming of temp. files
               and create  psiblast input files */
            ajRandomSeed();
            ajStrAssC(&tmpname, ajFileTempName(NULL));
            ajStrAss(&seqsin, tmpname);
            ajStrAppC(&seqsin, ".seqsin");

            /* create an output filename */
            /* Add a '.' to outextn if one does not already exist */
            if((ajStrChar(outfextn, 0)!='.'))        /* checks if the
                                                        file extension
                                                        starts with
                                                        "." */
                ajStrInsertC(&outfextn, 0, ".");

            /* Create the name of the output file */
            posdash = ajStrRFindC(filename, "/");
            posdot  = ajStrRFindC(filename, ".");

            if(posdash >= posdot)
                ajFatal("Could not create filename. "
			"Email rranasin@hgmp.mrc.ac.uk");
            else
            {
                ajStrAssS(&outfile,outfpath);
                ajStrAppSub(&outfile, filename, posdash+1, posdot-1);
                ajStrApp(&outfile,outfextn);
            }

            /* read alignment file into a scopalg structure */
            ajXyzScopalgRead(inf,&scopalg);

	    printf("scopalg structure read ok\n");
	    fflush(stdout);


            /* open up a file and write out the alignment in CLUSTAL format */
            seqsinf = ajFileNewOut(seqsin);

            ajXyzScopalgWriteClustal2(scopalg,&seqsinf);

	    printf("ScopalgWriteClustal2 called ok\n");
	    fflush(stdout);

            ajFileClose(&seqsinf);

            /* construct command line to run hmmer to build model */
            ajFmtPrintS(&cmd,"hmmbuild -g %S %S",outfile,seqsin);

            /* run hmmer */
	    ajFmtPrint("%S\n", cmd);
	    fflush(stdout);
	    system(ajStrStr(cmd));

            /* clean up temperary files */
            ajFmtPrintS(&cmd,"rm %S",seqsin);
            system(ajStrStr(cmd));

	    ajStrDel(&filename);
	    ajFileClose(&inf);
        }
    }

    /* clean up */
    ajStrDel(&infpath);
    ajStrDel(&infextn);
    ajStrDel(&outfpath);
    ajStrDel(&outfextn);
    ajStrDel(&outfile);
    ajStrDel(&tmp);
    ajStrDel(&cmd);
    ajStrDel(&tmpname);

    ajListDel(&list);

    /* exit */
    ajExit();
    return 0;
}

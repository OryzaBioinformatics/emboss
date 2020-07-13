/* @source fraggle application
**
**
** @author: Copyright (C) Matt Blades (mblades@hgmp.mrc.ac.uk)
**          The University of Leeds
**
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
*******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
*******************************************************************************
**
** Mon Jun 24 15:56:22 BST 2002
**
** The following documentation is out-of-date and should be disregarded.  It
** will be updated shortly.
**
*******************************************************************************
**IMPORTANT NOTE      IMPORTANT NOTE      IMPORTANT NOTE        IMPORTANT NOTE
*******************************************************************************
**
**
**
** Operation
** Fraggle reads a directory of .hits files produced by the application
** seqsearch, and for each individual file writes a new file in which
** sequence hits deemed to be fragments have been removed.
**
** Fraggle proceeds by determining the median length of all the
** hit sequences in the file, then discards any hit sequences which are
** not within a % threshold of the median length. Then writes the
** remaining hitlist objects to a new output file.
**
**
**
*/



/* #include "math.h" */
#include "emboss.h"



/* @prog fraggle **************************************************************
**
** Removes fragment sequences from a seqsearch hits file
**
******************************************************************************/
int main(int argc, char **argv)
{

    AjPStr      hitsin          = NULL;  /* Location of hits files for
					    input  */
    AjPStr      hitsextn        = NULL;  /* Extension of hits files
					    for input  */
    AjPStr      hitsout         = NULL;  /* Location of new hits files
					    for output  */
    AjPStr      hitsoutextn     = NULL;  /* Extension of new hits
					    files for output */

    AjPStr      temp            = NULL;  /* Temp string  */
    AjPStr      name            = NULL;  /* Temp string  */
    AjPStr      hold            = NULL;  /* Temp string  */
    AjPStr      line            = NULL;  /* String to hold file
					    lines  */
    AjPStr      exec            = NULL;  /* The UNIX command line to
					    be executed  */

    ajint       thresh          = 0;     /* Threshold for definition
					    of fragments  */
    ajint       start           = 0;     /* int to hold start of
					    sequence range  */
    ajint       end             = 0;     /* int to hold end of
					    sequence range  */
    ajint       num             = 0;     /* number of nodes on
					    list  */
    ajint       len             = 0;     /* length of sequence hit  */
    ajint       x               = 0;     /* Loop counters  */
    ajint       y               = 0;     /* Loop counters  */
    ajint       median          = 0;     /* Median length of sequence
					    hits  */
    ajint       mid             = 0;     /* Middle value of seq_len
					    array  */
    ajint       num_hits        = 0;     /* Number of hits in file  */

    float       score           = 0.0;   /* Float for storing
					    length/median value  */


    AjPInt      seq_len_sort    = NULL;  /* Array to hold length of
					    each hit seq  */
    AjPInt      seq_len         = NULL;  /* Array to hold sorted
					    lengths  */
    AjPInt      seq_ok          = NULL;  /* Array indicating if length
					    is > thresh */

    AjPList     list            = NULL;  /* List to hold hits file in
					    directory  */
    AjPFile     hitsPtr         = NULL;  /* Pointer to hits file  */
    AjPFile     hitsoutPtr      = NULL;  /* Pointer to hits output
					    file  */

    AjPHitlist  hitlist         = NULL;  /* Hitlist structure  */

    AjBool      ok              = ajFalse; /* Bool  */

    /* Assign strings and list */
    hitsin       = ajStrNew();
    hitsextn     = ajStrNew();
    hitsout      = ajStrNew();
    hitsoutextn  = ajStrNew();
    name         = ajStrNew();
    line         = ajStrNew();
    hold         = ajStrNew();
    exec         = ajStrNew();
    list         = ajListNew();



    /* Read data from acd */
    embInit("fraggle",argc,argv);
    hitsin      = ajAcdGetString("hitsin");
    hitsextn    = ajAcdGetString("hitsextn");
    hitsout     = ajAcdGetString("hitsout");
    hitsoutextn = ajAcdGetString("hitsoutextn");
    thresh      = ajAcdGetInt("thresh");


    /* Check directories */
    if(!ajFileDir(&hitsin))
        ajFatal("Could not open hits directory");

    if(!ajFileDir(&hitsout))
        ajFatal("Could not open hits directory");


    /* Create list of files in align directory */
    ajStrAssC(&temp, "*");
    if((ajStrChar(hitsextn, 0)=='.'))
        ajStrApp(&temp, hitsextn);
    else
    {
        ajStrAppC(&temp, ".");
        ajStrApp(&temp, hitsextn);
    }

    /* scan directory for hits files and add to list */
    ajFileScan(hitsin,temp,&list,ajFalse,ajFalse,NULL,NULL,ajFalse,NULL);
    ajStrDel(&temp);


    /* Determine number of nodes on list    */
    num = ajListLength(list);



    /* Start of main application loop                         */
    /* determine median length of sequences in each hits file */
    while(ajListPop(list,(void **)&temp))
    {
        /* Open hits file */
        if((hitsPtr=ajFileNewIn(temp))==NULL)
        {
            ajFileClose(&hitsPtr);
            ajWarn("Could not open hits file");
            ajStrDel(&temp);
            continue;
        }

        ok = ajFalse;
        x = 0;
        ajFmtPrint("Processing %S\n", temp);

        /* Read through file once and determine median      */
        /* sequence length, by reading range from RA field  */
        while((ajFileReadLine(hitsPtr, &line)) && !ajStrPrefixC(line,"//"))
        {
            if(ajStrPrefixC(line, "NS"))
            {
                ajFmtScanS(line, "%*s %d", &num_hits);

                if((num_hits == 0) || (num_hits == 1))
                {
                /* printf("Number of hits = 0 or 1....exiting hits file\n"); */

                    /* Set bool to false so we do NOT carry on any */
                    /* further with this hits file                 */
                    ok = ajFalse;
                }

                else
                {
                    /* printf("NS =%6d.....", num_hits); */

                    /* Create array to hold sequence lengths */
                    seq_len_sort = ajIntNewL(num_hits);
                    seq_len      = ajIntNewL(num_hits);
                    seq_ok       = ajIntNewL(num_hits);

                    /* Assign zeros to array elements */
                    for(y=0;y<num_hits;y++)
                    {
                        ajIntPut(&seq_ok, y, 0);
                        ajIntPut(&seq_len, y, 0);
                        ajIntPut(&seq_len_sort, y, 0);
                    }

                    /* Set bool to true in order to proceed */
                    ok = ajTrue;
                }
            }


            /* if line starts with RA then parse sequence range */
            if((ajStrPrefixC(line, "RA")) && (ok == ajTrue))
            {
                /* Scan line for sequence range */
                ajFmtScanS(line, "%*s %d %*s %d", &start, &end);

                /* calculate length and assign to array */
                len = (end - (start-1));
                /*printf("len = %d\n", len);*/

                /* Put len into arrays */
                ajIntPut(&seq_len, x, len);
                ajIntPut(&seq_len_sort, x, len);

                /* Increment array element counter */
                x++;
            }

            else
                continue;
        }



        if(ok == ajTrue)
        {
            /* Calculate median length */
            /* Reorder seq_len_sort array into ascending order */
            for(x=0;x<num_hits;x++)
                ajSortIntInc((ajint *) ajIntInt(seq_len_sort), num_hits);


            /* If num_hits == even then the median = average of middle
	       two values */
            if((num_hits % 2) == 0)
            {
                mid = (num_hits / 2);
                /*printf("mid = %d\n", mid);
                ajIntPut(&seq_len_sort, mid-1, 10000);
                  ajIntPut(&seq_len_sort, mid, 15000);
                  for(x=0;x<num_hits;x++)
                  ajFmtPrint("%4d\n", ajIntGet(seq_len_sort, x));*/
                median = (((ajIntGet(seq_len_sort, mid-1)) +
			   (ajIntGet(seq_len_sort, mid))) / 2);
                /* printf("median = %4d\n", median); */
            }

            /* else if num == odd number, then median = middle value */
            else
            {
                mid = (num_hits / 2);
                /*printf("mid = %d\n", mid);
                ajIntPut(&seq_len_sort, mid, 1000);
                for(x=0;x<num_hits;x++)
                    ajFmtPrint("%4d\n", ajIntGet(seq_len_sort, x));*/
                median = ajIntGet(seq_len_sort, mid);
                /* printf("median = %4d\n", median); */
            }




            /* Go through seq_len and determine which have  */
            /* lengths > threshold and should be written to */
            /* output file                                      */

            /* Reset file pointer */
            ajFileSeek(hitsPtr, 0, 0);

            /* Read hitlist into structure */
            ajXyzHitlistRead(hitsPtr, "//", &hitlist);

            /* create output file name and path */
            ajStrAss(&name, hitsout);
            ajStrFromInt(&hold, hitlist->Sunid_Family);

            ajStrApp(&name, hold);

            if((ajStrChar(hitsoutextn, 0) == '.') ||
	       (ajStrChar(hitsoutextn, 0) == '_'))
            {
                ajStrApp(&name, hitsoutextn);
            }

            else
            {
                ajStrAppC(&name, ".");
                ajStrApp(&name, hitsoutextn);
            }

            /*ajFmtPrint("filename = %S\n", name);*/

            /* Create output file */
            hitsoutPtr = ajFileNewOut(name);

            /* Write family header information to output file */
            if(MAJSTRLEN(hitlist->Class))
                ajFmtPrintF(hitsoutPtr,"CL   %S\n",hitlist->Class);
            if(MAJSTRLEN(hitlist->Fold))
                ajFmtPrintSplit(hitsoutPtr,hitlist->Fold,
				"XX\nFO   ",75," \t\n\r");
            if(MAJSTRLEN(hitlist->Superfamily))

                ajFmtPrintSplit(hitsoutPtr,hitlist->Superfamily,
				"XX\nSF   ",75," \t\n\r");
            if(MAJSTRLEN(hitlist->Family))
                ajFmtPrintSplit(hitsoutPtr,hitlist->Family,
				"XX\nFA   ",75," \t\n\r");
            if(MAJSTRLEN(hitlist->Family))

                ajFmtPrintF(hitsoutPtr,"XX\nSI   %d\n", hitlist->Sunid_Family);



            y = 0;
            /* Create array of 1's and 0's */
            for(x=0;x<num_hits;x++)
            {
                score = ((((float)ajIntGet(seq_len,x) / (float)median)) * 100);
                /*printf("thresh = %d\n", thresh);

                  printf("score = %f\n", score);*/
                /*if((score < 0.5) || (score > 2)) */
                if(score < thresh)
                {
                    /*ajFmtPrint("Acc = %S length = %d\n",
		      hitlist->hits[x]->Acc,
                    ajIntGet(seq_len, x));  */
                    ajIntPut(&seq_ok, x, 0);
                }

                else if(score >= thresh)
                {
                    ajIntPut(&seq_ok, x, 1);
                    y++;
                }

            }

            /* Write NS field to file */
            ajFmtPrintF(hitsoutPtr,"XX\nNS   %d\nXX\n",y);

            /* Go through seq_ok array if element == 1 then   */
            /* write that element of structure to output file */
            y = 0;
            for(x=0;x<num_hits;x++)
            {

                if(ajIntGet(seq_ok, x) == 1)
                {
                    y++;
                    ajFmtPrintF(hitsoutPtr, "%-5s[%d]\nXX\n", "NN", y);
                    ajFmtPrintF(hitsoutPtr, "%-5s%S\n", "AC",
				hitlist->hits[x]->Acc);
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                    ajFmtPrintF(hitsoutPtr, "%-5s%S\n", "TY",
				hitlist->hits[x]->Typeobj);
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                    if(hitlist->hits[x]->Group)
                    {
                        ajFmtPrintF(hitsoutPtr, "%-5s%S\n", "GP",
				    hitlist->hits[x]->Group);
                        ajFmtPrintF(hitsoutPtr, "XX\n");
                    }
                    ajFmtPrintF(hitsoutPtr, "%-5s%d START; %d END;\n", "RA",
                                hitlist->hits[x]->Start,
				hitlist->hits[x]->End);
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                    ajSeqWriteXyz(hitsoutPtr, hitlist->hits[x]->Seq, "SQ");
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                }

                else
                    continue;
            }

            /* Print end of data marker */
            ajFmtPrintF(hitsoutPtr, "//\n");

            /* Close input and output files and tidy up */
            ajXyzHitlistDel(&hitlist);
            hitlist=NULL;
            ajFileClose(&hitsoutPtr);
            ajFileClose(&hitsPtr);
            ajIntDel(&seq_len);
            ajIntDel(&seq_len_sort);
            ajIntDel(&seq_ok);
        }



        /* Hits file contains either one or zero hits, therefore just copy */
        /* original hits file to new output file                           */

        else
        {
            /* Reset file pointer */
            ajFileSeek(hitsPtr, 0, 0);

            /* Read hitlist into structure */
            ajXyzHitlistRead(hitsPtr, "//", &hitlist);

            /* create output file name and path */
            ajStrAss(&name, hitsout);
            ajStrFromInt(&hold, hitlist->Sunid_Family);
            ajStrApp(&name, hold);

            if((ajStrChar(hitsoutextn, 0) == '.') ||
	       (ajStrChar(hitsoutextn, 0) == '_'))
            {
                ajStrApp(&name, hitsoutextn);
            }

            else
            {
                ajStrAppC(&name, ".");
                ajStrApp(&name, hitsoutextn);
            }

            /* Create output file */
            hitsoutPtr = ajFileNewOut(name);


            /* Write family header information to output file */
            if(MAJSTRLEN(hitlist->Class))
                ajFmtPrintF(hitsoutPtr,"CL   %S\n",hitlist->Class);
            if(MAJSTRLEN(hitlist->Fold))
                ajFmtPrintSplit(hitsoutPtr,hitlist->Fold,
				"XX\nFO   ",75," \t\n\r");
            if(MAJSTRLEN(hitlist->Superfamily))
                ajFmtPrintSplit(hitsoutPtr,hitlist->Superfamily,
				"XX\nSF   ",75," \t\n\r");
            if(MAJSTRLEN(hitlist->Family))
                ajFmtPrintSplit(hitsoutPtr,hitlist->Family,
				"XX\nFA   ",75," \t\n\r");
            if(MAJSTRLEN(hitlist->Family))
                ajFmtPrintF(hitsoutPtr,"XX\nSI   %d\n", hitlist->Sunid_Family);

            /* Write NS field to file */
            ajFmtPrintF(hitsoutPtr,"XX\nNS   %d\nXX\n",num_hits);


            y = 0;
            if(num_hits == 1)
            {
                for(x=0;x<num_hits;x++)

                {
                    y++;
                    ajFmtPrintF(hitsoutPtr, "%-5s[%d]\nXX\n", "NN", y);
                    ajFmtPrintF(hitsoutPtr, "%-5s%S\n", "AC",
				hitlist->hits[x]->Acc);
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                    ajFmtPrintF(hitsoutPtr, "%-5s%S\n", "TY",
				hitlist->hits[x]->Typeobj);
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                    if(hitlist->hits[x]->Group)
                    {
                        ajFmtPrintF(hitsoutPtr, "%-5s%S\n", "GP",
				    hitlist->hits[x]->Group);
                        ajFmtPrintF(hitsoutPtr, "XX\n");
                    }
                    ajFmtPrintF(hitsoutPtr, "%-5s%d START; %d END;\n", "RA",
                                hitlist->hits[x]->Start,
				hitlist->hits[x]->End);
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                    ajSeqWriteXyz(hitsoutPtr, hitlist->hits[x]->Seq, "SQ");
                    ajFmtPrintF(hitsoutPtr, "XX\n");
                }
            }

            /* Print end of data marker */
            ajFmtPrintF(hitsoutPtr, "//\n");

            /* tidy and go on to next file */
            ajFileClose(&hitsPtr);
            ajFileClose(&hitsoutPtr);
            ajXyzHitlistDel(&hitlist);
            hitlist=NULL;
        }

	ajStrDel(&temp);
    }




    /* Tidy up */
    /* Delete strings and list */
    ajStrDel(&hitsin);
    ajStrDel(&hitsextn);
    ajStrDel(&hitsout);
    ajStrDel(&hitsoutextn);
    ajStrDel(&line);
    ajStrDel(&name);
    ajStrDel(&hold);
    ajStrDel(&exec);
    ajListDel(&list);


    /* Return */
    ajExit();
    return 0;


}











/* @source pepinfo application
**
** Displays properties of the amino acid residues in a peptide sequence
** @author: Copyright (C) Mark Faller (mfaller@hgmp.mrc.ac.uk)
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
#include "ajhist.h"
#include <stdlib.h>


#define NOY (AJGRAPH_X_BOTTOM + AJGRAPH_Y_LEFT + AJGRAPH_Y_RIGHT+ AJGRAPH_Y_INVERT_TICK + AJGRAPH_X_INVERT_TICK + AJGRAPH_X_TICK + AJGRAPH_X_LABEL + AJGRAPH_Y_LABEL + AJGRAPH_TITLE )


static void plotHistInt2( AjPHist hist, AjPSeq seq,
			  int * results, int hist_num,
			  char* header, char* xtext, char * ytext);
static void plotGraph2Float(AjPGraph graphs, AjPSeq seq, float * results,
			    char* title_text, char * xtext, char * ytext,
			    int plotcolour);
static void printFloatResults( AjPFile outfile, AjPSeq seq, float * results,
			       char* header);
static void printIntResults( AjPFile outfile, AjPSeq seq, int * results,
			     char* header);

static int seq_start, seq_end, win_mid, seq_begin;

int main( int argc, char **argv, char ** env) {

  AjPSeq inseq;
  AjPFile outfile;
  int hwindow/*, geswindow*/;
  AjPStr aa_properties, aa_hydropathy/*, aa_acc_surface*/;

  AjBool do_seq, do_general, do_hydropathy, do_plot;
  AjPStr key, value;
  AjIList listIter;
  AjPTable table;

  AjPStr tmpa=NULL;
  AjPStr tmpb=NULL;
  int i, j, k;
  int cnt;

  int * ival;
  int * iv[9];
  float * pfloat;
  float * pf[3];
  float num, total;

  AjPGraph graphs = NULL;
  AjPHist hist = NULL;
  int numGraphs = 0;
   
  /*   Data_table aa_props, aa_hydro, aa_acc;*/
  AjPList aa_props, aa_hydro/*, aa_acc*/;

  char * propertyTitles[] = {
    "Tiny",
    "Small",
    "Aliphatic",
    "Aromatic",
    "Non-polar",
    "Polar",
    "Charged",
    "Positive",
    "Negative"
  };

  char * hydroTitles[] = {
    "Kyte & Doolittle hydropathy parameters",
    "OHM  Hydropathy parameters (Sweet & Eisenberg)",
    "Consensus parameters (Eisenberg et al)"
  };

  (void) ajGraphInit ("pepinfo", argc, argv);

  aj_hist_mark=NOY;

  aa_props = ajListNew();
  aa_hydro = ajListNew();
  /*aa_acc = ajListNew(); NOT USED */

  /* Get parameters */
  inseq = ajAcdGetSeq( "inseq");
  seq_begin = ajSeqBegin ( inseq );
  seq_end = ajSeqEnd ( inseq );
  seq_start = seq_begin - 1;
  outfile = ajAcdGetOutfile( "outfile");
  hwindow = ajAcdGetInt( "hwindow");
  do_general = ajAcdGetBool ("generalplot");
  do_hydropathy = ajAcdGetBool ("hydropathyplot");
  /*    geswindow = ajAcdGetInt( "geswindow"); NOT USED */
  aa_properties = ajAcdGetString( "aaproperties");
  aa_hydropathy = ajAcdGetString( "aahydropathy");
  /*    aa_acc_surface = ajAcdGetString( "aaaccsurface"); NOT USED */
  graphs = ajAcdGetGraphxy( "graph");

  do_plot = do_general || do_hydropathy;

  /*    
  {
    FILE* fp;
    fp = fopen ("pltrace.1", "w");
    ajGraphTraceInt (graphs, fp);
    fclose (fp);
  }
  */

  /* Set begin and end position in sequence structure */
  ajSeqSetRange( inseq, seq_begin, seq_end);

  /* Find out which tables are required in the output */
  do_seq = ajFalse;

  if (do_hydropathy) numGraphs +=3;

  key = ajStrNew();
  value = ajStrNew();

  /* if sequence plot required */
  if (do_seq) {
    ajDebug ("sequence plot\n");
  }

  /* if general properties plot required  */
  if (do_general) {
    ajDebug ("general plot\n");

    /*initialize properties list*/
    aa_props = ajListNew();
    embDataListInit( aa_props, aa_properties);

    /* Get first table from properties list of tables */
    listIter = ajListIter( aa_props);

    /* calculate plot*/
    /* changes to test embDataListGetTables */
    for (i = 0; i < 9; i++) {
	 
      if (!ajListIterDone( listIter)) {
		  
	/* ajalloc new int array for storing results */
	AJCNEW(ival,(seq_end-seq_start));
	iv[i] = ival;
	table = ajListIterNext(listIter);
	for (j = seq_start; j < seq_end; j++) {
	  ajStrAssSub( &key, ajSeqStr( inseq), j, j);
	  value = ajTableGet( table, key);
	  if ( value != NULL) {
	    if (ajStrToInt( value, ival))
	      ival++;
	    else {
	      ajErr( "value is not integer ..%s..\n",
			  ajStrStr(value));
	      ajExit();
	    }
	  }
	  else {    
	    ajErr( "At position %d in seq, couldn't find key %s in table",
			j, ajStrStr(key));
	    ajExit();
	  }
	}
      }
      else {
	ajErr( "No more tables in list\n");
	ajExit();
      }
    }

    /* print out results */

    for (i=0; i<9; i++) {
      ajFmtPrintS(&tmpa, "%s residues in %s from position %d to %d",
		  propertyTitles[i], ajSeqName( inseq),
		  seq_begin, seq_end);
      printIntResults( outfile, inseq, iv[i], ajStrStr(tmpa));
    }

    /* plot out results */

    hist = ajHistNewG ( 9, (seq_end - seq_begin+1), graphs);
    hist->bins = seq_end - seq_begin +1;

    hist->xmin = seq_begin;
    hist->xmax = seq_end;

    hist->displaytype=HIST_SEPARATE;

    ajFmtPrintS(&tmpa, "Properties of residues in %s from position %d to %d",
		ajSeqName( inseq),seq_begin, seq_end);
    ajHistSetTitleC( hist, ajStrStr(tmpa));

    ajHistSetXAxisC( hist, "Residue Number");
    ajHistSetYAxisLeftC( hist, "");

    for (i=0; i<9; i++) {
      ajFmtPrintS(&tmpa,  "%s residues in %s from position %d to %d",
		  propertyTitles[i], ajSeqName( inseq), seq_begin,
		  seq_end);
      ajFmtPrintS(&tmpb,  "%s residues", propertyTitles[i]);
      plotHistInt2( hist, inseq, iv[i], i,
		    ajStrStr(tmpa), ajStrStr(tmpb), "");
    }

    /*
    {
      FILE* fp;
      fp = fopen ("pltrace.2", "w");
      ajGraphTraceInt (graphs, fp);
      fclose (fp);
    }
    */

    ajHistDisplay( hist);

    /*
    {
      FILE* fp;
      fp = fopen ("pltrace.3", "w");
      ajGraphTraceInt (graphs, fp);
      fclose (fp);
    }
    */

    /* tidy up */
    /* Delete results lists */

    for (i = 0; i < 9; i++)
      AJFREE(iv[i]);


    /* Delete Data tables*/
    embDataListDel(aa_props);

    /*delete hist object*/
    ajHistDelete( hist);

  }

  /* if hydropathy plot required */
  if (do_hydropathy) {
    ajDebug ("hydropathy plot\n");

    if (numGraphs) {
      ajGraphxySetOverLap(graphs, ajFalse);
    }

    /* get data from amino acid properties */
    aa_hydro = ajListNew();
    embDataListInit( aa_hydro, aa_hydropathy);

    /* Get first table from properties list */
    listIter = ajListIter( aa_hydro);

     /* calculate plot */
    for (i=0; i < 3; i++) {

      /* make sure we have another table from the list to calculate */
      if (ajListIterDone( listIter)) {
	ajErr( "No more tables in list\n");
	ajExit();
      }

      /* Get next table of parameters */
      table = ajListIterNext( listIter);
      
      win_mid = (hwindow / 2);

      /* get array to store result */
      AJCNEW(pfloat, (seq_end - seq_start));
      pf[i] = pfloat;

      /* Fill in 0.00 for seq begin to win_mid */
      for (j=0,cnt=0;j<win_mid; j++)
	pfloat[cnt++]=0.0;
      
      /* start loop */
      for (j = seq_start; j<=(seq_end-hwindow); j++) {
	total = 0.00;
	for (k=0; k < hwindow; k++) {
	  ajStrAssSub( &key, ajSeqStr( inseq), (j+k), (j+k));
	  value = ajTableGet( table, key);
	  if (value == NULL) {
	    ajErr ("At position %d in seq, couldn't find key %s",
		       k, ajStrStr(key));
	    ajExit();
	  }
	  if (!ajStrIsFloat( value)) {
	    ajErr( "value is not float ..%s..",
		   ajStrStr(value));
	    ajExit();
	  }
	  ajStrToFloat( value, &num);
	  total +=num;
	}
	pfloat[cnt++] = total / hwindow;
      }

      /* fill in value of 0 for end of sequence */
      for (j = win_mid+1; j<hwindow; j++)
	pfloat[cnt++] = 0.00;
    }

    /* Print out results */

    for (i=0; i<3; i++) {
      ajFmtPrintS(&tmpa,  "Results from %s", hydroTitles[i]);
      printFloatResults( outfile, inseq, pf[i], ajStrStr(tmpa)); 
    }

    /*Plot results*/
    for (i=0; i<3; i++) {
      ajFmtPrintS( &tmpa,
		   "Hydropathy plot of residues %d to %d of sequence %s using %s",
		   seq_begin, seq_end, ajSeqName( inseq), hydroTitles[i]);
      plotGraph2Float( graphs, inseq, pf[i], ajStrStr(tmpa),
		       "Residue Number", "Hydropathy value", BLACK);
    }

    /*tidy up*/
    for (i=0; i<3; i++) {
      AJFREE(pf[i]);
    }
  }

  if (numGraphs) {
    /*
    {
      FILE* fp;
      fp = fopen ("pltrace.4", "w");
      ajGraphTraceInt (graphs, fp);
      fclose (fp);
    }
    */
    /*ajGraphTrace (graphs);*/
    if (do_general || do_seq)
      ajGraphNewPage (ajFalse);

    ajGraphSetCharSize(0.50);
    ajGraphxyTitleC(graphs,"Pepinfo");
    
    ajGraphxyDisplay(graphs,AJTRUE);
  }
  if (do_plot) {
    ajGraphCloseWin();
    ajGraphxyDel(graphs);
  }

  ajExit();
  return 0;
}

/* @funcstatic printIntResults ********************************************
**
**  prints out a resultsList. Very basic at the moment, really just used to
**  prove I have the results and they are correct. There are several of these
**  for each primitive data type (as I write them). So far have
**  printIntResults and printFloatResults. They are public routines
**
** @param [r] outfile [AjPFile] file to output to.
** @param [r] seq     [AjPSeq]  Sequence
** @param [r] results [int*]    int array of reuslts.
** @param [r] header  [char*]   header line
** @return [void]
** @@
******************************************************************************/

static void printIntResults( AjPFile outfile, AjPSeq seq,
			     int * results, char * header)
{

    int i;
    AjPStr aa;

    aa = ajStrNew();
    ajFmtPrintF( outfile,  "Printing out %s\n\n", header);
    ajFmtPrintF( outfile, "Position  Residue\t\t\tResult\n");
    for (i = seq_start; i<seq_end; i++)
    {
       ajStrAssSub( &aa, ajSeqStr( seq), i, i);
       ajFmtPrintF(outfile,  "   %5d%8s%32d\n", (i+1),
			ajStrStr(aa), *results++);
    }
	  
    ajFmtPrintF( outfile,  "\n\n\n");
   
    /* clean up */
    ajStrDel(&aa);

}

/* @funcstatic printFloatResults ********************************************
**
** Routine to print out Float results data
**
** @param [r] outfile [AjPFile] file to output to.
** @param [r] seq     [AjPSeq]  Sequence
** @param [r] results [float*]  float array of results.
** @param [r] header  [char*]   header line
** @return [void]
** @@
******************************************************************************/

static void printFloatResults( AjPFile outfile, AjPSeq seq, float * results,
			       char * header)
{

    int i;
    AjPStr aa;

    aa = ajStrNew();
    ajFmtPrintF( outfile,  "Printing out %s\n\n", header);
    ajFmtPrintF( outfile, "Position  Residue\t\t\tResult\n");
    for (i = seq_start; i<seq_end; i++)
    {
       ajStrAssSub( &aa, ajSeqStr( seq), i, i);
       ajFmtPrintF(outfile,  "%5d%8s%32.3f\n", (i+1),
			ajStrStr(aa), *results++);
    }
    ajFmtPrintF( outfile, "\n\n\n");


    /* clean up */
    ajStrDel(&aa);

}

/* @funcstatic plotGraph2Float ********************************************
**
** Create and add graph from set of results to graph set.
**
** @param [rw] graphs [AjPGraph] Graphs set to add new graph to.
** @param [r] seq     [AjPSeq]  Sequence
** @param [r] results [float*]  float array of results.
** @param [r] title_text [char*] title for graph
** @param [r] xtext      [char*] x label.      
** @param [r] ytext      [char*] y label.
** @param [r] plotcolour [int]   Pen colour to plot the graph in.
** @return [void]
**
******************************************************************************/

static void plotGraph2Float(AjPGraph graphs, AjPSeq seq, float * results, 
      char * title_text, char * xtext, char * ytext, int plotcolour)
{

    AjPGraphData plot;

    int npts = 0;
    
    float ymin=64000.;
    float ymax=-64000.;
    
    npts = seq_end - seq_start;

    ajGraphDataxyMaxMin(results,npts,&ymin,&ymax);
    
    /*
     *  initialise plot, the number of points will be the length of the data
     *  in the results structure
     */
    plot = ajGraphxyDataNewI( npts);
   
    /*Set up rest of plot information*/
    ajGraphxyDataSetTitleC( plot, title_text);
    ajGraphxyDataSetXtitleC( plot, xtext);
    ajGraphxyDataSetYtitleC( plot, ytext);
    ajGraphDataxySetMaxMin(plot,(float)1,(float)npts,ymin,ymax);
    ajGraphDataxySetMaxima(plot,(float)1,(float)npts,ymin,ymax);
    ajGraphDataxySetTypeC(plot,"2D Plot");

    ajGraphxyAddDataCalcPtr(plot, npts, seq_begin, 1.0, results);
    ajGraphxyAddGraph( graphs, plot);


}

/* @funcstatic plotHistInt2 ********************************************
**
** Add a histogram data to the set set.
**
** @param [rw] hist   [AjPHist] Histogram set to add new set to.
** @param [r] seq     [AjPSeq]  Sequence
** @param [r] results [int*]  float array of results.
** @param [r] hist_num [int] the number of the histogram set. 
** @param [r] header  [char*] title.      
** @param [r] xtext   [char*] x label.      
** @param [r] ytext   [char*] y label.
** @return [void]
**
******************************************************************************/


static void plotHistInt2( AjPHist hist, AjPSeq seq, int * results,
			  int hist_num, char * header,
			  char * xtext, char * ytext)
{
   int npts, i;

   float *farray;

   npts = seq_end - seq_start;

   AJCNEW(farray, npts);
   for (i=0; i<npts; i++)
       farray[i] = results[i];

   ajHistCopyData( hist, hist_num, farray);

   ajHistSetMultiTitleC( hist, hist_num, header);
   ajHistSetMultiXTitleC( hist, hist_num, xtext);
   ajHistSetMultiYTitleC( hist, hist_num, ytext);

   /*tidy up*/
   AJFREE( farray);
}

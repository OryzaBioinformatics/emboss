/* @source abiview application
**
** Display an ABI trace file and write out the sequence.
** @author: Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
** The sequence is taken from a ABI trace file and written to a 
** sequence file. 
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
*****************************************************************************/


#include "emboss.h"


static AjPGraphData graphDisplay(AjPGraph graphs, AjPInt2d trace, 
            int nstart, int nstop, AjPShort  basePositions, int base,
            int colour, AjBool overlay, float tmax, int* ntrace);

static AjPGraphData graphTextDisplay(AjPGraph graphs, int nstart,
             int nstop, AjPShort  basePositions, 
             AjBool overlay, AjPStr nseq, float tmax, int nt);

static void TextDisplay(AjPGraph graphs, int nstart, int nstop,
                 AjPStr nseq);
static int getResColour(char B);



int main (int argc, char * argv[])
{
    AjPStr  fname;
    AjPFile fp;
    AjPSeqout seqout;
    AjPSeq    seqo;
    AjPStr    nseq;
    AjPInt2d  trace=NULL;
    AjPShort  basePositions=NULL;
    AjPGraph  graphs = NULL;
    AjBool overlay;
    AjBool separate;
    AjBool yticks;

    AjPGraphData gd1 = NULL;
    AjPGraphData gd2 = NULL;
    AjPGraphData gd3 = NULL;
    AjPGraphData gd4 = NULL;
    AjPGraphData gd5 = NULL;

    int ntrace;
    int strace;
    long int fwo_;

    float tmax;
    int nstart;
    int nstop;
    int window;
    int i;
    int base;
    long int baseO;
    long int numBases;
    long int numPoints;
    long int dataOffset[4];

    char res1;
    char res2;
    char res3;
    char res4;

    /* BYTE[i] is a byte mask for byte i */
    const long int BYTE[] = { 0x000000ff };
   


    (void) ajGraphInit ("abiview", argc, argv);

    fname    = ajAcdGetString("fname");
    graphs   = ajAcdGetGraphxy( "graph");
    seqout   = ajAcdGetSeqout("outseq");
    separate = ajAcdGetBool("separate");
    yticks = ajAcdGetBool("yticks");
    window   = ajAcdGetInt("window");


    overlay = !separate;

    fp = ajFileNewIn(fname);
    if(!fp) ajFatal("%S not found",fname);
    if(!ajSeqABITest(fp)) ajFatal("Not an ABI file");

    numBases = ajSeqABIGetNBase(fp);
    baseO = ajSeqABIGetBaseOffset(fp); /* find BASE tag & get offset */
    nseq = ajStrNew();                 /* read in sequence */

    trace = ajInt2dNew();
    basePositions = ajShortNew();

    numPoints = ajSeqABIGetNData(fp);  /* find DATA tag & get no. of points */
                                       /* get data trace offsets            */
    ajSeqABIGetTraceOffset(fp,dataOffset);
    ajSeqABIGetData(fp,dataOffset,numPoints,trace);  /* read in trace data  */

    fwo_ = ajSeqABIGetFWO(fp);         /* find FWO tag - field order "GATC  */

    res1 = (char)(fwo_>>24&BYTE[0]);
    res2 = (char)(fwo_>>16&BYTE[0]);
    res3 = (char)(fwo_>>8&BYTE[0]);
    res4 = (char)(fwo_&BYTE[0]);

    ajSeqABIReadSeq(fp,baseO,numBases,&nseq);
    ajSeqABIGetBasePosition(fp,numBases,&basePositions);


    /* find trace max */
    tmax = 0.;
    for(i=0;i<numPoints;i++)
    {
      for(base=0;base<4;base++)
        if(tmax < (float)ajInt2dGet(trace,base,i))
            tmax = (float)ajInt2dGet(trace,base,i);
    }


    /* setup graph parameters */
    nstart = 0;
    nstop  = window+1;

    ajGraphxyTitle(graphs,fname);
    ajGraphxyYtitleC(graphs,"Signal");
    if(yticks)
    {
       ajGraphxySetYTick(graphs,ajTrue);
       ajGraphxySetYInvTicks(graphs,ajTrue);
    }
    else
    {
       ajGraphxySetYTick(graphs,ajFalse);
    }
    ajGraphxySetXInvTicks(graphs,ajTrue);


    ntrace = 0;
    strace = 0;

    /* loop over pages to be displayed */
    while(nstart < numBases-1)  
    { 
      if(nstop > numBases) 
          nstop = numBases;

      ajGraphSetMulti(graphs,5);
      ajGraphxySetOverLap(graphs,overlay);

      gd1 = graphDisplay(graphs,trace,nstart,nstop,basePositions,
                 0,getResColour(res1),overlay,tmax,&ntrace);

      ntrace = strace;
      gd2 = graphDisplay(graphs,trace,nstart,nstop,basePositions,
                 1,getResColour(res2),overlay,tmax,&ntrace);
      ntrace = strace;

      gd3 = graphDisplay(graphs,trace,nstart,nstop,basePositions,
                 2,getResColour(res3),overlay,tmax,&ntrace);
      ntrace = strace;

      gd4 = graphDisplay(graphs,trace,nstart,nstop,basePositions,
                 3,getResColour(res4),overlay,tmax,&ntrace);

      if(!overlay)
        gd5 = graphTextDisplay(graphs,nstart,nstop,basePositions,
                              overlay,nseq,tmax,strace);
      else
        TextDisplay(graphs,nstart,nstop,nseq);

      strace = ntrace;

      ajGraphxyDisplay(graphs,ajFalse); 

      if(nstop<numBases)
      {
         ajGraphDataDel(&gd1);                     /* free graph data mem */
         ajGraphDataDel(&gd2);
         ajGraphDataDel(&gd3);
         ajGraphDataDel(&gd4);
         if(!overlay)          
         {                     
            ajGraphDataObjDel(&gd5);            /* free seq text mem */
            ajGraphDataDel(&gd5);
         }
         else
            ajGraphObjDel(&graphs);	        /* free seq text mem */
         ajGraphNewPage(ajFalse);               /* display new page  */
      }

      nstart = nstop-1;
      nstop  = nstart+window+1;
    }
 
   
    ajGraphCloseWin();
    ajGraphxyDel(graphs);
    ajInt2dDel(&trace);
    ajShortDel(&basePositions);

    /* write out consensus sequence */
    seqo = ajSeqNew();
    ajSeqAssName(seqo,fname);
    ajSeqAssSeq(seqo,nseq);
    ajSeqWrite(seqout,seqo);    
    ajStrDel(&nseq);
    ajSeqDel(&seqo);

    ajExit ();
    return 0;

}



/* @funcstatic graphDisplay *********************************************
**
** Load in ABI trace data into graph data object.
**
** returns: graph data object 
**     
*************************************************************************/
static AjPGraphData graphDisplay(AjPGraph graphs, AjPInt2d trace, 
             int nstart, int nstop, AjPShort  basePositions,
             int base, int colour, AjBool overlay, float tmax,
             int* nt)
{
    int i;
    short bP;
    short lastbP;
    int bstart;

    AjPGraphData gdata;


/* create graph data object */
    gdata = ajGraphxyDataNewI(ajShortGet(basePositions,nstop-1)-(*nt));

    if(nstart>0)
      lastbP = ajShortGet(basePositions,nstart-1);
    else
      lastbP = 0;

    bstart = *nt;

    for(i=nstart;i<nstop;i++)
    {
       bP = ajShortGet(basePositions,i);
       while(*nt < bP)
       { 
          gdata->x[*nt-bstart] = (float)i + (float)(*nt+1-lastbP)/
                                      (float)(bP-lastbP);
          gdata->y[*nt-bstart] = (float)ajInt2dGet(trace,base,*nt);
          *nt = *nt+1;
       }
       lastbP = bP;
    }
 
    ajGraphxySetColour(gdata,colour);
    ajGraphDataxySetMaxMin(gdata,(float)nstart+1.,
                           (float)nstop,0.,tmax+80.);  

    /* add graph to list in a multiple graph */
    ajGraphxyAddGraph(graphs,gdata);

    return gdata;

}



/* @funcstatic graphTextDisplay *****************************************
**
** Draw sequence in a separate graph if the trace data is plotted
** in separate graphs (i.e. not overlayed).
**
** returns: graph data object containing the sequence text
**          
*************************************************************************/
static AjPGraphData graphTextDisplay(AjPGraph graphs, int nstart,
             int nstop, AjPShort  basePositions, AjBool overlay,
             AjPStr nseq, float tmax, int nt)
{
    int i;
    int colres;

    AjPGraphData gdata;
   
    char res[2];

    res[1]='\0';

/* create graph data object */
    gdata = ajGraphxyDataNewI(ajShortGet(basePositions,nstop-1)-nt);

    for(i=nstart;i<nstop;i++)
    {
       *res = ajStrChar(nseq,i);
       colres = getResColour(*res);
       ajGraphDataObjAddText(gdata,(float)i+1.,tmax+75.,colres,res);
    }

    ajGraphDataxySetMaxMin(gdata,(float)nstart+1,
                 (float)nstop,tmax+70.,tmax+80.);

    /* add graph to list in a multiple graph */
    ajGraphxyAddGraph(graphs,gdata);

    return gdata;
}



/* @funcstatic TextDisplay ***********************************************
**
** Add sequence on top of the same graph as the trace data 
** (i.e. overlayed).
**
** returns: 
**          
*************************************************************************/
static void TextDisplay(AjPGraph graphs, int nstart, int nstop,
                        AjPStr nseq)
{
    int i;
    int colres;

    char res[2];

    res[1] = '\0';
    for(i=nstart;i<nstop-1;i++)
    {
       *res = ajStrChar(nseq,i);
       colres = getResColour(*res);
       ajGraphObjAddText(graphs,(float)i+1.,1225.,colres,res);
    }

    return;
}


/* @funcstatic getResColour *********************************************
**
** Assign colour to a given nucleotide.
**
** returns: base colour
**         
*************************************************************************/
static int getResColour(char B)
{
  return ((B)=='C'?RED:(B)=='A'?GREEN:(B)=='G'?BLUE:(B)=='T'?BLACK:YELLOW);
}

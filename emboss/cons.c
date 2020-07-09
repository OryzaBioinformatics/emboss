/* @source cons application
**
** Calculates a consensus
** @author: Copyright (C) Tim Carver (tcarver@hgmp.mrc.ac.uk)
** @@
**
**  
** -plurality  	- defines no. of +ve scoring matches below 
**                which there is no consensus.
**
** -identity   	- defines the number of identical symbols
**                requires in an alignment column for it to
**                included in the consensus.
**
** -setcase   	- upper/lower case given if score above/below
**                user defined +ve matching threshold.
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
*****************************************************************************/


#include "emboss.h"
#include <ctype.h>          /* for tolower function */
#include <limits.h>         /* for INT_MAX */


void calc_consensus(AjPSeqset seqset,AjPFile outf,AjPMatrix cmpmatrix,
                    int nseqs, int mlen, float fplural, float setcase, 
                    int identity, AjPStr *cons);


int main (int argc, char * argv[])
{
    int   nseqs;
    int   mlen;
    int   i;
    int   identity;
    float fplural;
    float setcase;
    char  *p;
    AjPSeqset seqset;
    AjPFile   outf;
    AjPStr    cons;
    AjPMatrix cmpmatrix=0;
 

    embInit ("cons", argc, argv);

    seqset    = ajAcdGetSeqset ("msf");
    cmpmatrix = ajAcdGetMatrix("datafile");
    fplural   = ajAcdGetFloat("plurality");
    setcase   = ajAcdGetFloat("setcase");
    identity  = ajAcdGetInt("identity");
    outf      = ajAcdGetOutfile ("outf");

    nseqs = ajSeqsetSize(seqset);
    if(nseqs<2)
      ajFatal("Insufficient sequences (%d) to create a matrix",nseqs);

    mlen = ajSeqsetLen(seqset);
    for(i=0;i<nseqs;++i)         /* check sequences are same length */
    {
      p = ajSeqsetSeq(seqset,i);
      if(strlen(p)!=mlen)
          ajWarn("Sequence lengths are not equal!");
      ajSeqsetToUpper(seqset);
    }

    cons = ajStrNew();
    calc_consensus(seqset,outf,cmpmatrix,nseqs,mlen,
                    fplural,setcase,identity,&cons);

    /* print consensus */
    ajFmtPrintF(outf,"Consensus \n%s\n",ajStrStr(cons));   

    ajFileClose(&outf);
    ajStrDel(&cons);

    ajExit ();
    return 0;

}

void calc_consensus(AjPSeqset seqset,AjPFile outf,AjPMatrix cmpmatrix,
                    int nseqs,int mlen,float fplural,float setcase,
                    int identity, AjPStr *cons)
{
    int   i; 
    int   j; 
    int   k;
    int   **matrix;
    int   m1=0;
    int   m2=0;
    int   highindex;
    int   matsize;
    int   matchingmaxindex;
    int   identicalmaxindex;

    float max;
    float contri=0;
    float contrj=0;
    float *identical;
    float *matching;

    AjPSeqCvt cvt=0;
    AjPFloat score=NULL;
    char **seqcharptr;
    char res;



    matrix  = ajMatrixArray(cmpmatrix);
    cvt     = ajMatrixCvt(cmpmatrix);    /* return conversion table */
    matsize = ajMatrixSize(cmpmatrix);

    AJCNEW(seqcharptr,nseqs);
    AJCNEW(identical,matsize);
    AJCNEW(matching,matsize);

    score = ajFloatNew();

    for(i=0;i<nseqs;i++)                  /* get sequence as string */
      seqcharptr[i] =  ajSeqsetSeq(seqset, i);  

    for(k=0; k< mlen; k++)
    {
      res = '-';

      for(i=0;i<matsize;i++)          /* reset id's and +ve matches */
      {
        identical[i] = 0.0;
        matching[i] = 0.0;
      }

      for(i=0;i<nseqs;i++) 
        ajFloatPut(&score,i,0.);

      for(i=0;i<nseqs;i++)            /* generate score for columns */
      {
        m1 = ajSeqCvtK(cvt,seqcharptr[i][k]);
        if(m1)
          identical[m1] += ajSeqsetWeight(seqset,i);
        for(j=i+1;j<nseqs;j++) 
        {
          m2 = ajSeqCvtK(cvt,seqcharptr[j][k]);
          if(m1 && m2)
          {
            contri = (float)matrix[m1][m2]*ajSeqsetWeight(seqset,j) 
                                          +ajFloatGet(score,i);
            contrj = (float)matrix[m1][m2]*ajSeqsetWeight(seqset,i)
                                          +ajFloatGet(score,j);
                    
            ajFloatPut(&score,i,contri);
            ajFloatPut(&score,j,contrj);
          }
        }
      }

      highindex = -1;
      max  = -(float)INT_MAX;
      for(i=0;i<nseqs;i++)
      {
        if(ajFloatGet(score,i) > max) 
        {
          highindex = i;
          max       = ajFloatGet(score,i);
        }
      }

      for(i=0;i<nseqs;i++)        /* find +ve matches in the column */
      {
        m1 = ajSeqCvtK (cvt, seqcharptr[i][k]);
        if(!matching[m1])
        {
          for(j=0;j<nseqs;j++)
          {
            if( i != j) 
            {
              m2 = ajSeqCvtK (cvt, seqcharptr[j][k]);
              if(m1 && m2 && matrix[m1][m2] > 0)
                matching[m1] += ajSeqsetWeight(seqset, j);
            }
          }
        }
      }


      matchingmaxindex  = 0;      /* get max matching and identical */
      identicalmaxindex = 0;
      for(i=0;i<nseqs;i++)
      {
        m1 = ajSeqCvtK(cvt,seqcharptr[i][k]);
        if(identical[m1] > identical[identicalmaxindex])
          identicalmaxindex= m1;
      }
      for(i=0;i<nseqs;i++)
      {
        m1 = ajSeqCvtK(cvt,seqcharptr[i][k]);
        if(matching[m1] > matching[matchingmaxindex])
          matchingmaxindex= m1;
        else if(matching[m1] ==  matching[matchingmaxindex])
        {
          if(identical[m1] > identical[matchingmaxindex])
            matchingmaxindex= m1;
        }
      }

      /* plurality check */
      if(matching[ajSeqCvtK(cvt,seqcharptr[highindex][k])] >= fplural)
         res = seqcharptr[highindex][k];

      if(matching[highindex]<= setcase)
        res = tolower(res);
  
      if(identity)                      /* if just looking for id's */
      {
        j=0;
        for(i=0;i<nseqs;i++)
        {
          if(matchingmaxindex == ajSeqCvtK(cvt,seqcharptr[i][k]))
          j++;
        }
        if(j<identity) 
          res = '-';
      }

      ajStrAppK(cons,res);
    }

    AJFREE(seqcharptr);
    AJFREE(matching);
    AJFREE(identical);
    ajFloatDel(&score);
 
    return;

}


/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.editor;

import java.io.*;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;

public class Consensus
{
  private int matrix[][];
  private String cons = "";

  public Consensus(File matrixFile, Vector seqs, float fplural,
                   float setcase, int identity)
  {
    Matrix mat = new Matrix(matrixFile);
    matrix = mat.getMatrix();
    calculateCons(mat,seqs,fplural,setcase,identity);
  }

  public Consensus(String matrixJar, String matrixFileName, 
                   Vector seqs, float fplural,
                   float setcase, int identity)
  {
    this(new Matrix(matrixJar,matrixFileName),
               seqs,fplural,setcase,identity);
  }

  public Consensus(Matrix mat,
                   Vector seqs, float fplural,
                   float setcase, int identity)
  {
    matrix = mat.getMatrix();
    calculateCons(mat,seqs,fplural,setcase,identity);
  }


  private void calculateCons(Matrix mat, Vector seqs, float fplural,
                            float setcase, int identity)
  {
    int nseqs = seqs.size();
//  int mlen = ((Sequence)seqs.get(0)).getLength();
    int mlen = getMaxSequenceLength(seqs);

    String nocon = "-";
    String res = "";

    int matsize = mat.getIDimension();
    float identical[] = new float[matsize];
    float matching[] = new float[matsize];
    float score[]    = new float[nseqs];

    int i;
    int j;
    int m1;
    int m2;
    float contri = 0.f;
    float contrj = 0.f;
    String s1;
    String s2;

    for(int k=0; k< mlen; k++)
    {
      res = nocon;
      for(i=0;i<matsize;i++)          /* reset id's and +ve matches */
      {
        identical[i] = 0.f;
        matching[i] = 0.f;
      }

      for(i=0;i<nseqs;i++)
        score[i] = 0.f;
 
      for(i=0;i<nseqs;i++)            /* generate score for columns */
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1); 
        for(j=i+1;j<nseqs;j++)
        {
          s2 = getResidue(seqs,j,k);
          m2 = mat.getMatrixIndex(s2);
          if(m1 >= 0 && m2 >= 0)
          { 
            contri = matrix[m1][m2]*getSequenceWeight(seqs,j)+score[i];
            contrj = matrix[m1][m2]*getSequenceWeight(seqs,i)+score[j];
            score[i] = contri;
            score[j] = contrj;
          }
        }
      }

      int highindex = -1;
      float max = -(float)Integer.MAX_VALUE;
      for(i=0;i<nseqs;i++)
      {
        if( score[i] >= max ||
           (score[i] == max &&
            getResidue(seqs,highindex,k).equals("-") ))
        {
          highindex = i;
          max       = score[i];
        }
      }


      for(i=0;i<nseqs;i++)        /* find +ve matches in the column */
      {
        s1 = getResidue(seqs,i,k); 
        m1 = mat.getMatrixIndex(s1);
        for(j=0;j<nseqs;j++)
        {
          if( i != j)
          {
            s2 = getResidue(seqs,j,k);
            m2 = mat.getMatrixIndex(s2);
            if(m1 >= 0 && m2 >= 0 && matrix[m1][m2] > 0)
              matching[m1] += getSequenceWeight(seqs,j);
          }
        }
      }

      
      int matchingmaxindex  = 0;      /* get max matching and identical */
      int identicalmaxindex = 0;
      for(i=0;i<nseqs;i++)
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1);
        if(m1 >= 0)
          if(identical[m1] > identical[identicalmaxindex])
            identicalmaxindex= m1;
      }
      for(i=0;i<nseqs;i++)
      {
        s1 = getResidue(seqs,i,k);
        m1 = mat.getMatrixIndex(s1);
        if(m1 >= 0)
        {
          if(matching[m1] > matching[matchingmaxindex])
            matchingmaxindex= m1;
          else if(matching[m1] ==  matching[matchingmaxindex])
          {
            if(identical[m1] > identical[matchingmaxindex])
              matchingmaxindex= m1;
          }
        }
      }


      /* plurality check */
      s1 = getResidue(seqs,highindex,k); 
      m1 = mat.getMatrixIndex(s1);

      if(m1 >= 0 && matching[m1] >= fplural
         && !s1.equals("-"))
         res = s1;

      if(matching[highindex] <= setcase)
        res = res.toLowerCase();

      if(identity>0)                      /* if just looking for id's */
      {
        j=0;
        for(i=0;i<nseqs;i++)
        {
          s1 = getResidue(seqs,i,k); 
          m1 = mat.getMatrixIndex(s1);
          if(matchingmaxindex == m1)
          j++;
        }
        if(j<identity)
          res = nocon;
      }

      cons = cons.concat(res);
    }
  }

/**
*
* Check all sequences are the same length
*
*/
  public boolean isEqualSequenceLength(Vector seqs)
  {
    int len = 0;
    int numseq=0;
    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
    {
      Sequence seq = (Sequence)enum.nextElement();
      if(numseq > 0)
        if(len != seq.getLength())
          return false;
     
      len = seq.getLength();
      numseq++;
    }
    return true;
  }

/**
*
* Check all sequences lengths and return length of
* the longest sequence
*
*/
  public int getMaxSequenceLength(Vector seqs)
  {
    int len = 0;

    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
    {
      Sequence seq = (Sequence)enum.nextElement();
      if(len < seq.getLength())
        len = seq.getLength();
    }
    return len;
  }

  public Sequence getConsensusSequence()
  {
    return new Sequence("Consensus",cons);
  }

  public float getSequenceWeight(Vector seqs, int i)
  {
    return ((Sequence)seqs.get(i)).getWeight();
  }

  public String getResidue(Vector seqs, int i, int k)
  {
    String res = "-";
    try
    {
      res = ((Sequence)seqs.get(i)).getSequence().substring(k,k+1);
    }
    catch(StringIndexOutOfBoundsException sexp){}
    return res;
  }

  public static void main(String args[])
  {
    Vector seqs = new Vector();
//  seqs.add(new Sequence("MALEGFP"));
//  seqs.add(new Sequence("MALEGFP"));
//  seqs.add(new Sequence("MAPEGFP"));
//  seqs.add(new Sequence("MAPEGFP"));

    seqs.add(new Sequence("MHQDGISSMNQLGGLFVNGRP"));
    seqs.add(new Sequence("-MQNSHSGVNQLGGVFVNGRP"));
    seqs.add(new Sequence("STPLGQGRVNQLGGVFINGRP"));
    seqs.add(new Sequence("STPLGQGRVNQLGGVFINGRP"));
    seqs.add(new Sequence("-MEQTYGEVNQLGGVFVNGRP"));

    new Consensus(new File("/packages/emboss_dev/tcarver/emboss/emboss/emboss/data/EBLOSUM80"),
                  seqs,1.f,1.f,1);
  }
}


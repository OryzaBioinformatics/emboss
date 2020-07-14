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

import javax.swing.*;
import java.io.*;
import java.util.Vector;
import java.util.Hashtable;
import java.util.StringTokenizer;

import org.emboss.jemboss.gui.sequenceChooser.SequenceFilter;

/**
*
* This class reads sequences in FASTA and MSF format.
* The sequences are read and stored as a Vector.
*
*/
public class SequenceReader
{

  private File seqFile;
  private Vector seqs;
  private boolean reading = false;

  public SequenceReader()
  {
    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);
    JFileChooser fc = new JFileChooser(System.getProperty("user.home"));
    System.setSecurityManager(sm);

    fc.addChoosableFileFilter(new SequenceFilter());
    int returnVal = fc.showOpenDialog(fc);
  
    if(returnVal == JFileChooser.APPROVE_OPTION)
    {
      seqFile = fc.getSelectedFile();
      readSequenceFile();
      reading = true;
    }
  } 

  public SequenceReader(File seqFile)
  {
    this.seqFile = seqFile;
    readSequenceFile();
    reading = true;
  }

  public SequenceReader(String seqString)
  {
    readSequenceString(seqString);
    reading = true;
  }

/**
*
*
*/
  public boolean isReading()
  {
    return reading;
  }

  public Vector readSequenceFile()
  {
    BufferedReader in = null;
    try
    {
      in = new BufferedReader(new FileReader(seqFile));
      String line = in.readLine();

// fasta
      if(line.startsWith(">"))
        return readFastaFile(new BufferedReader(new FileReader(seqFile)));

// msf
      int index = line.indexOf("PileUp");
      if(index > -1)
        return readMSFFile(new BufferedReader(new FileReader(seqFile)));
      index = line.indexOf("!!AA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new FileReader(seqFile)));
      index = line.indexOf("!!NA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new FileReader(seqFile)));
    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Error");
    }
    return null;
  }

  public Vector readSequenceString(String seqString)
  {
    BufferedReader in = null;
    try
    {
      in = new BufferedReader(new StringReader(seqString));
      String line = in.readLine();

// fasta
      if(line.startsWith(">"))
        return readFastaFile(new BufferedReader(new StringReader(seqString)));

// msf
      int index = line.indexOf("PileUp");
      if(index > -1)
        return readMSFFile(new BufferedReader(new StringReader(seqString)));
      index = line.indexOf("!!AA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new StringReader(seqString)));
      index = line.indexOf("!!NA_MULTIPLE_ALIGNMENT");
      if(index > -1)
        return readMSFFile(new BufferedReader(new StringReader(seqString)));
    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Error");
    }
    return null;
  }

/**
*
* Reads in the FASTA sequence file and creates a Vector 
* containing the sequence(s).
*
*/
  public Vector readFastaFile(BufferedReader in)
  {
    seqs = new Vector();
//  BufferedReader in = null;
    String seqString = "";

    try
    {
//    in = new BufferedReader(new FileReader(seqFile));
      String line;
      String name = null;
      Sequence seq;

      while((line = in.readLine()) != null )
      {
        if(line.startsWith(">"))
        {
          if(!seqString.equals(""))
          {
            seqs.add(new Sequence(name,seqString));
            seqString = "";
            name = line.substring(1);
          }
          else if(line.startsWith(">") && seqString.equals(""))
            name = line.substring(1);
        }
        else
          seqString = seqString.concat(line);
      }
      in.close();

      if(!seqString.equals(""))
        seqs.add(new Sequence(name,seqString));
    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Error");
    }
    return seqs;
  }

/**
*
* Reads in the MSF sequence file and creates a Vector
* containing the sequence(s).
*
*/
  public Vector readMSFFile(BufferedReader in)
  {
    seqs = new Vector();
//  BufferedReader in = null;
    String seqString = "";

    try
    {
//    in = new BufferedReader(new FileReader(seqFile));
      String line;
      Sequence seq;
      String type = null;
      String bit;
      StringTokenizer st;
      boolean header = true;
      Hashtable seqIndex = new Hashtable();
      int num = 0;

      while((line = in.readLine()) != null )
      {
        if(line.startsWith("//"))  //end of header
          header = false;
        if(line.equals(""))
          continue;

        if(header)      // read the MSF header
        {
          st = new StringTokenizer(line," ");
          while (st.hasMoreTokens())
          {
            bit = st.nextToken(" ").trim();
            if(bit.startsWith("Name:"))
            {
              String name = st.nextToken(" ").trim();
              seq = new Sequence(name,"");
              seqs.add(num,seq);

              if(type != null)
              {
                if(type.equalsIgnoreCase("P"))
                  seq.setType(true);
                else if(type.equalsIgnoreCase("N"))
                  seq.setType(false);
              }   
              seqIndex.put(name,new Integer(num));
              num++;
            }
            else if(bit.startsWith("Type:"))
            {
              type = st.nextToken(" ").trim();
            }
          }
        }
        else      // read the MSF sequences
        {
          int index = line.indexOf(" ");
          if(index > -1)
          {
            String name = line.substring(0,index);
            if(!seqIndex.containsKey(name))
              System.out.println("Error reading sequence ");
            else
            {
              st = new StringTokenizer(line.substring(index)," ");
              seqString = new String();
              while (st.hasMoreTokens())
                seqString = seqString.concat(st.nextToken(" ").trim());

              int seqInd = ((Integer)seqIndex.get(name)).intValue();
              seq = (Sequence)seqs.elementAt(seqInd);
              seq.appentToSequence(seqString);
            }
          }
        }
      }
      in.close();

    }
    catch (IOException e)
    {
      System.out.println("SequenceReader Error");
    }
    return seqs;
  }


/**
*
* Returns the number of sequences.
*
*/
  public int getNumberOfSequences()
  {
    return seqs.size();
  }

/**
*
* Returns the sequence at a given position in
* the Sequence Vector store.
*
*/
  public Sequence getSequence(int index)
  {
    return (Sequence)seqs.get(index);
  }

/**
*
* Returns the Sequence Vector store
*
*/
  public Vector getSequenceVector()
  {
    return seqs;
  }

  public File getSequenceFile()
  {
    return seqFile;
  }
  
}


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
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Arrays;
import javax.swing.JOptionPane;

import org.apache.regexp.*;
import org.emboss.jemboss.JembossJarUtil;

public class Matrix
{
  private int matrix[][];
  private int idimension;
  private int jdimension;

  private int i=0;
  private int k=0;

  private Hashtable residueMatrixPosition;
  private Object[] keys = null;
  private String cons = "";
  private String matrixString = null;
  private String matrixFileName = null;

  public Matrix(File matrixFile)
  {
    this.matrixFileName = matrixFile.getName();
    matrixRead(matrixFile);
  }

  public Matrix(String matrixJar, String matrixFileName)
  {
    this.matrixFileName = matrixFileName;
    try
    {
      Hashtable matrixHash = (new JembossJarUtil(matrixJar)).getHash();
      keys = matrixHash.keySet().toArray();
      Arrays.sort(keys);
      if(matrixHash.containsKey(matrixFileName))
      {
        matrixString = new String((byte[])matrixHash.get(matrixFileName));
        matrixReadString(matrixString);
      }
      else
        System.err.println("Matrix file "+matrixFileName+
                    " not found in jar file "+matrixJar);
    }
    catch(Exception exp)
    {
      JOptionPane.showMessageDialog(null,
                 "Failed to read "+matrixFileName+
                 "\nfrom the matrix archive "+matrixJar,
                 "Missing matrix archive",
                  JOptionPane.ERROR_MESSAGE);

//    System.err.println("Failed to read "+matrixFileName+
//                       "\nfrom the matrix archive "+matrixJar);
    }
  }

  public int[][] getMatrix()
  {
    return matrix;
  }

  public String getMatrixTable()
  {
    return matrixString;
  }

  public String getCurrentMatrixName()
  {
    int index = matrixFileName.lastIndexOf("/");
    if(index > -1)
      return matrixFileName.substring(index+1);
    return matrixFileName;
  }

  public Object[] getKeys()
  {
    return keys;
  }

  public Object[] getKeyNames()
  {
    try
    {
      int nkeys = keys.length;
      Object[] kname = new Object[nkeys];
      for(int i=0;i<nkeys;i++)
      {
        int pos = ((String)keys[i]).indexOf("/")+1;
        kname[i] = ((String)keys[i]).substring(pos);
      }
      return kname;
      }
    catch(NullPointerException npe)
    {
      JOptionPane.showMessageDialog(null, 
                 "No matrix files found!", 
                 "Matrix files missing",
                  JOptionPane.ERROR_MESSAGE); 
      return null;  
    }
  }

  public Hashtable getResidueMatrixPosition()
  {
    return residueMatrixPosition;
  }

  public int getMatrixIndex(String s)
  {
    s = s.toUpperCase();
    if(!residueMatrixPosition.containsKey(s))
      return -1;
    return ((Integer)residueMatrixPosition.get(s)).intValue();
  }

  public int getIDimension()
  {
    return idimension;
  }

  public int getJDimension()
  {
    return jdimension;
  }

  private RE getRegularExpression()
  {
    RE regexp = null;

    try
    {
      RECompiler rec = new RECompiler();
      REProgram  rep = rec.compile("[:digit:]");
      regexp = new RE(rep);
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }
    return regexp;
  }

  private void matrixLineCount(String line, RE regexp)
  {
    String delim = " :\t\n";
    if(!line.startsWith("#") && !line.equals("")
                             && regexp.match(line))
    {
      jdimension = 0;
      idimension++;
      line = line.trim();
      StringTokenizer st = new StringTokenizer(line,delim);
      while (st.hasMoreTokens())
      {
        st.nextToken();
        jdimension++;
      }
    }
  }

  private void matrixLineParse(String line, RE regexp)
  {
    String delim = " :\t\n";
    int j = 0;

    if(!line.startsWith("#") && !line.equals(""))
    {
      line = line.trim();
      StringTokenizer st = new StringTokenizer(line,delim);
      if(!regexp.match(line))
      {
        while (st.hasMoreTokens())
        {
          residueMatrixPosition.put(st.nextToken(),new Integer(k));
          k++;
        }
      }
      else
      {
        st.nextToken();
        while (st.hasMoreTokens())
        {
          String s = st.nextToken();
          matrix[i][j] = Integer.parseInt(s);
          j++;
        }
        i++;
      }
    }
  }

  private int[][] matrixRead(File matrixFile)
  {
    String delim = " :\t\n";
    String line  = "";
    BufferedReader in;
    residueMatrixPosition = new Hashtable();

    idimension = 1;
    RE regexp = getRegularExpression();

// determine dimensions of the matrix
    try
    {
      in = new BufferedReader(new FileReader(matrixFile));
      while((line = in.readLine()) != null )
        matrixLineCount(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }

//  initialise matrix
    matrix = new int[idimension][jdimension];

// read the matrix 
    try
    {
      in = new BufferedReader(new FileReader(matrixFile));
      while((line = in.readLine()) != null )
        matrixLineParse(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }
    return matrix;
  }

  private int[][] matrixReadString(String matrixString)
  {
    String delim = " :\t\n";
    String line  = "";
    BufferedReader in;
    residueMatrixPosition = new Hashtable();

    idimension = 1;
    RE regexp = getRegularExpression();

// determine dimensions of the matrix
    try
    {
      in = new BufferedReader(new StringReader(matrixString));
      while((line = in.readLine()) != null )
        matrixLineCount(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }

//  initialise matrix
    matrix = new int[idimension][jdimension];

// read the matrix
    try
    {
      in = new BufferedReader(new StringReader(matrixString));
      while((line = in.readLine()) != null )
        matrixLineParse(line,regexp);
      in.close();
    }
    catch (IOException e)
    {
      System.out.println("Cannot read matrix file in!");
    }
    return matrix;
  }

}


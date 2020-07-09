/********************************************************************
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public
*  License along with this library; if not, write to the
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
*  Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/


package org.emboss.jemboss.gui.form;

import java.awt.*;
import javax.swing.*;
import org.emboss.jemboss.gui.sequenceChooser.*;


public class ListFilePanel extends JPanel
{

  private int nFiles;
  private FileChooser fileChooser[];

/**
*
* @param int number of files for the list
*
*/  
  public ListFilePanel(int nFiles)
  {
    super(new BorderLayout());
    this.nFiles = nFiles;

    Box bdown = Box.createVerticalBox();
    fileChooser = new FileChooser[nFiles];

    bdown.add(Box.createVerticalStrut(2));
    for(int i=0;i < nFiles;i++)
    {
      fileChooser[i] = new FileChooser(bdown,""); 
      bdown.add(Box.createVerticalStrut(2));  
    }

    JScrollPane scroll = new JScrollPane(bdown);
    scroll.setPreferredSize(new Dimension(350,100));
    
    this.add(scroll, BorderLayout.WEST);
    setPreferredSize(new Dimension(350,100));
    setMaximumSize(new Dimension(350,100));
  }

/**
*
* @return list of filenames
*
*/
  public String getListFile()
  {
    String list = "";
    String ls = System.getProperty("line.separator");
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
        list = list.concat(fileChooser[i].getFileChosen() + ls);
    }

    return list;
  }

/**
*
* @return list of filenames
*
*/
  public String[] getArrayListFile()
  {

    int nseqs = 0;
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
        nseqs++;
      System.out.println("HERE " + fileChooser[i].getFileChosen() + nseqs);
    }

    String list[] = new String[nseqs];
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
        list[i] = fileChooser[i].getFileChosen();
    }

    return list;
  }

/**
*
* @return the nth sequence file name
*
*/
  public String getSequence(int n)
  {
    String fn="";

    int nseqs = 0;
    for(int i=0;i < nFiles;i++)
    {
      if(!fileChooser[i].getFileChosen().equals(""))
      {
        nseqs++;
        if(nseqs == n)
        {
          fn = fileChooser[i].getFileChosen();
          break;
        }
      }
    }

    return fn;
  }

/**
*
* Reset the values of the list files
*
*/
  public void doReset()
  {
    for(int i=0;i < nFiles;i++)
      fileChooser[i].setText("");
  }

}



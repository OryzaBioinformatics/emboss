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


package org.emboss.jemboss.gui;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.tree.*;

import org.apache.regexp.*;

import java.awt.event.*;
import java.io.*;
import java.util.*;

/**
*
* Displays JTabbedPane of the contents of the Hashtable
*
*/
public class ShowResultSet
{

/**
*  
* @param the data to display
*
*/
  public ShowResultSet(Hashtable reslist)
  {
    JTabbedPane rtp = new JTabbedPane();
    Enumeration enum = reslist.keys();
    JFrame resFrame = new JFrame("Saved Results on the Server");

    new ResultsMenuBar(resFrame,rtp,reslist);
    resFrame.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
    String stabs[] = new String[reslist.size()];
    int ntabs = 0;

    while (enum.hasMoreElements()) 
    {
      String thiskey = (String)enum.nextElement().toString();
      JPanel s1 = new JPanel(new BorderLayout());
      JScrollPane r1 = new JScrollPane(s1);
      if (thiskey.endsWith("png") || thiskey.endsWith("html")) 
      {
        int index = findInt(thiskey);
        if(index>0)
        {
          stabs[index-1] = new String(thiskey);
          ntabs++;
        }
        else
        {
          ImageIcon i1 = new ImageIcon((byte [])reslist.get(thiskey));
          JLabel l1 = new JLabel(i1);
          int hh = i1.getIconHeight();
          int ww = i1.getIconWidth();
          s1.add(l1);
          rtp.add(thiskey,r1);
        }
      } 
      else 
      {
	JTextArea o1 = new JTextArea((String)reslist.get(thiskey));
        o1.setFont(new Font("monospaced", Font.PLAIN, 12));
        o1.setCaretPosition(0);
	s1.add(o1, BorderLayout.CENTER);
        rtp.add(thiskey,r1);
      }
    }

// now load png files into pane
    for(int i=0; i<ntabs;i++)
    {
      JPanel s1 = new JPanel(new BorderLayout());
      JScrollPane r1 = new JScrollPane(s1);
      ImageIcon i1 = new ImageIcon((byte [])reslist.get(stabs[i]));
      JLabel l1 = new JLabel(i1);
      int hh = i1.getIconHeight();
      int ww = i1.getIconWidth();
      s1.add(l1);
      if(stabs[i] != null)
      {
        rtp.add(r1,i);
        rtp.setTitleAt(i,stabs[i]);
      }
    }

    resFrame.setSize(640,480);
    resFrame.getContentPane().add(rtp,BorderLayout.CENTER);
    resFrame.setVisible(true);
  }


  private int findInt(String exp)
  {

    RECompiler rec = new RECompiler();
    try
    {
      REProgram  rep = rec.compile("^(.*)([:digit:]+)");
      RE regexp = new RE(rep);
      if(regexp.match(exp))
      {
        int ia = (new Integer(regexp.getParen(2))).intValue();
        return ia;
      }
    }
    catch (RESyntaxException rese)
    {
      System.out.println("RESyntaxException ");
    }

    return -1;
  }

}

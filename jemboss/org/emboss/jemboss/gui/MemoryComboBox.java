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
import java.awt.event.*;
import java.io.*;
import java.util.Vector;
import javax.swing.*;
import javax.swing.event.*;


public class MemoryComboBox extends JComboBox 
{

  public static final int MAX_MEM_LEN = 30;

  public MemoryComboBox(Vector v)
  {
    super(v);
    setEditable(true);

    addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        MemoryComboBox cb = (MemoryComboBox)e.getSource();
        String newEntry = (String)cb.getSelectedItem();
        cb.add(newEntry);
      }
    });

  }

  public void add(String item)
  {
    removeItem(item);
    insertItemAt(item, 0);
    setSelectedItem(item);
    if (getItemCount() > MAX_MEM_LEN)
      removeItemAt(getItemCount()-1);
  }


}


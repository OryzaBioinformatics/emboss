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


package org.emboss.jemboss.gui.sequenceChooser;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.filechooser.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.io.*;

import org.emboss.jemboss.gui.form.*;
import org.emboss.jemboss.gui.AdvancedOptions;

/**
*
* Creates a JFileChooser and add it to the Box.
*
*/
public class FileChooser
{

  private String fileSelected;
  private final JFileChooser fc;
  private TextFieldSink fileName;

  private String currentDirectory;
  private JLabel lname;

  public FileChooser (Box bdown, String name) 
  {

    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);

    fc = new JFileChooser();
    System.setSecurityManager(sm);

    fileName = new TextFieldSink();

    Box pname;
    pname = Box.createHorizontalBox();
    lname = new JLabel(name);
    lname.setFont(org.emboss.jemboss.gui.form.SectionPanel.labfont);

    bdown.add(pname);
    pname.add(lname);
    pname.add(Box.createHorizontalGlue());

    Box file;
    file = Box.createHorizontalBox();
    bdown.add(file);

    JButton openButton = new JButton("Browse files...");
    file.add(fileName);
    file.add(Box.createRigidArea(new Dimension(2,2)));
    file.add(openButton);

    fc.addChoosableFileFilter(new SequenceFilter());
    openButton.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e)       
      {
        String dir = AdvancedOptions.cwd;
        File cwd = new File(dir);
        if(cwd.isDirectory() && cwd.canRead())
          fc.setCurrentDirectory(cwd);

        int returnVal = fc.showOpenDialog(fc);

        if (returnVal == JFileChooser.APPROVE_OPTION) 
        {
          fileSelected = fc.getSelectedFile().getAbsolutePath();
          currentDirectory = fc.getCurrentDirectory().getPath();
          fileName.setText(fileSelected);
        }
      }
    });

    file.add(Box.createHorizontalGlue());
  }

  
  public String getFileChosen() 
  {
    return fileName.getText();
  }

  public JFileChooser getJFileChooser() 
  {
    return fc;
  }

  public TextFieldSink getTextFieldSink() 
  {
    return fileName;
  }

  public void setText(String txt)
  {
    fileName.setText(txt);
  }

  public void setSize(Dimension d)
  {
    fileName.setPreferredSize(d);
    fileName.setMinimumSize(d);
    fileName.setMaximumSize(d);
  }

  public void setForeground(Color c)
  {
    lname.setForeground(c);
  }

}


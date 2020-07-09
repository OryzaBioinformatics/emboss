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


package org.emboss.jemboss.gui.filetree;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import org.emboss.jemboss.gui.sequenceChooser.*;
import org.emboss.jemboss.gui.AdvancedOptions;

import java.awt.event.*;
import java.io.*;

/**
*
* Saves files 
*
*/
public class FileSaving
{

  final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  private String fs = new String(System.getProperty("file.separator"));

  public FileSaving(JTextPane seqText, byte[] pngContent)
  {


    SecurityManager sm = System.getSecurityManager();
    System.setSecurityManager(null);
    JFileChooser fc = new JFileChooser(AdvancedOptions.cwd);
    System.setSecurityManager(sm);

    fc.addChoosableFileFilter(new SequenceFilter());
    int returnVal = fc.showSaveDialog(fc);

    if (returnVal == JFileChooser.APPROVE_OPTION)
    {
      File files = fc.getSelectedFile();
      String cwd = (fc.getCurrentDirectory()).getAbsolutePath();
      String fileSelected = files.getName();

      seqText.setCursor(cbusy);
      FileSave fsave = new FileSave(new File(cwd + fs + fileSelected));
      if(fsave.doWrite())
      {
        if(pngContent != null)
          fsave.fileSaving(pngContent);
        else
          fsave.fileSaving(seqText.getText());
      
        if(!fsave.fileExists())
          org.emboss.jemboss.Jemboss.tree.addObject(fileSelected,cwd);
      }
      seqText.setCursor(cdone);
    }
  }

}


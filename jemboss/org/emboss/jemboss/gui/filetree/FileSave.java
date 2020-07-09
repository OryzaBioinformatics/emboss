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

import java.io.*;
import java.util.*;
import javax.swing.*;

public class FileSave 
{

  private File f;
  private boolean fileWritten = false;
  private boolean fExists = false;
  private boolean canDo = true;

/**
*
* Check if we can/want to save file to disk. 
* @param  f  the File to save to
*
*/
  public FileSave(File f) 
  {

   this.f = f;
// error cases
 
    if(f.exists()) 
    {
      fExists = true;
      if(!f.canWrite()) 
      {
	canDo = false;
        JOptionPane.showMessageDialog(null,
                  "Unable to overwite file or directory",
                  "Message",
                  JOptionPane.WARNING_MESSAGE);
      }
      else 
      {
        canDo = false;
	int ok = JOptionPane.showConfirmDialog(null,
                f.toString() + " already exists. Overwrite?",
                "Overwrite file",
                JOptionPane.YES_NO_OPTION);
	if (ok == JOptionPane.YES_OPTION) 
	    canDo = true;
      }
    }

    this.canDo = canDo;
  }

/**
*
* @param  file  An Object with the file contents
*
*/
  public void fileSaving(Object file)
  {

// try the write
    if (canDo) 
    {
      if (file.getClass().equals(String.class))
      {
	try 
        {
	  FileWriter out = new FileWriter(f);
	  out.write((String) file);
	  out.close();
	  fileWritten = true;
	}
        catch(IOException ioe) 
        {
	  fileWritten = false;
	}
      }
      else 
      {
	try
        {
	  FileOutputStream out = new FileOutputStream(f);
	  out.write((byte []) file);
	  out.close();
	  fileWritten = true;
	} 
        catch(IOException ioe) 
        {
	  fileWritten = false;
	}
      }
    }
  }

/**
*
* @return true if we can write and want too
*
*/
  public boolean doWrite()
  {
    return canDo;
  }

/**
*
* Whether the fileSaving() operation was successful
* @return true if we wrote the file, otherwise false
*
*/
  public boolean writeOK() 
  {
    return fileWritten;
  }

  public boolean fileExists()
  {
    return fExists;
  }

}

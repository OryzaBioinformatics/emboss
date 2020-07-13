/****************************************************************
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  Based on EmbreoFileRoots
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.util.*;
import java.io.*;

import org.emboss.jemboss.JembossParams;

public class FileRoots implements Serializable 
{

  private Hashtable fileRoots;
  private String currentRoot;
  private String currentDir;
  private Vector rootVector;
  private String defaultRoot;
  private int defaultRootIndex;

/**
*
* Retrieve the list of available filesystem roots
* @param mysettings JembossParams defining server parameters
*
*/
  public FileRoots(JembossParams mysettings) throws JembossSoapException
  {

    fileRoots = null;
    defaultRootIndex = -1;
    PrivateRequest rReq = new PrivateRequest(mysettings, "EmbreoFile", 
                                                "embreo_roots");

    fileRoots = rReq.getHash();
    if(fileRoots.containsKey("status"))
      fileRoots.remove("status");
    if(fileRoots.containsKey("msg"))
      fileRoots.remove("msg");

    if (fileRoots.containsKey("default-root")) 
    {
      defaultRoot = (String) fileRoots.get("default-root");
      currentRoot = defaultRoot;
      fileRoots.remove("default-root");
    }
    else
      defaultRoot = null;
    
    rootVector = new Vector();
    Enumeration enum = fileRoots.keys();
    while (enum.hasMoreElements()) 
    {
      String s = (String) enum.nextElement();
      rootVector.add(s);
    }

    if (defaultRoot != null ) 
      defaultRootIndex = rootVector.indexOf(defaultRoot);
    
  }

/**
*
* Get the list of filesystem roots
*
*/
  public Hashtable getRoots() 
  {
    return fileRoots;
  }

/**
*
* Get the list of filesystem roots
*
*/
  public Vector getRootVector() 
  {
    return rootVector;
  }

/**
*
* Get the default root, if defined, else return null
*
*/
  public String getDefaultRoot() 
  {
    return defaultRoot;
  }

/**
*
* Get the index of the default root in the Vector, else return -1
*
*/
  public int getDefaultRootIndex() 
  {
    return defaultRootIndex;
  }

/**
*
* Get the currently selected directory
*
*/
  public String getCurrentDir() 
  {
    return currentDir;
  }

/**
*
* Set the current directory
* @param newRoot The name of the new current directory
*
*/
  public void setCurrentDir(String newDir) 
  {
    currentDir = newDir;
  }

/**
*
* Get the currently selected root
*
*/
  public String getCurrentRoot() 
  {
    return currentRoot;
  }

/**
*
* Set the current root
* @param newRoot The name of the new filesystem root.
*
*/
  public void setCurrentRoot(String newRoot)
  {
    currentRoot = newRoot;
  }

  //for Serializable
   private void writeObject(java.io.ObjectOutputStream out) throws IOException 
   {

     out.defaultWriteObject();
   }

   private void readObject(java.io.ObjectInputStream in)
     throws IOException, ClassNotFoundException 
   {
     in.defaultReadObject();
   }
}

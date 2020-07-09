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
*  Based on EmbreoFileList
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.io.*;
import java.util.*;
import javax.swing.DefaultListModel;

import org.emboss.jemboss.JembossParams;

import org.apache.soap.rpc.*;

public class FileList 
{

  private String statusmsg;
  //actually status is probably an int:
  private String status;
  private Hashtable proganswer;
  private String currentRes = null;
  private String flist = null;
  private String directories = null;
  private Vector vdir;

/**
* Retrieves a directory listing from an embreo server.
* The directory listing consists of a hash with two entries, a full
* list of files (including directories) and a list of those files
* that are directories.
*
* @param mysettings JembossParams defining server parameters
* @param fileRoot the filesystem root being used
* @param dir directory to list files in, relative to fileRoot
*
* @throws JembossSoapException If authentication fails
*/
   public FileList(JembossParams mysettings, String fileRoot, String dir)
                      throws JembossSoapException 
   {

     if (mysettings.getDebug()) 
       System.out.println("FileList: start " + fileRoot + " :/: " + dir);
     
     Vector params = new Vector();
     String options= "fileroot=" + fileRoot;
     params.addElement(new Parameter("options", String.class,
				     options, null));
     params.addElement(new Parameter("dirname", String.class,
				     dir, null));
     PrivateRequest eRun;
     try
     {
       eRun = new PrivateRequest(mysettings,"EmbreoFile","directory_shortls", params);
     }
     catch (JembossSoapException e) 
     {
       throw new JembossSoapException("Authentication Failed");
     }

     flist = eRun.getHash().get("list").toString();
     directories = eRun.getHash().get("dirlist").toString();

     vdir = new Vector();
     StringTokenizer tokenizer = new StringTokenizer(directories,"\n");
     while (tokenizer.hasMoreTokens())
     {
       String image = tokenizer.nextToken();
       vdir.add(image);
     }

     if (mysettings.getDebug()) 
       System.out.println("FileList: done");
     
   }


/**
* Gets the list of files as a Vector
*
* @return The list of files as a Vector
*/
  public Vector fileVector() 
  {
    Vector v = new Vector();
    StringTokenizer tokenizer = new StringTokenizer(flist,"\n");
    while (tokenizer.hasMoreTokens()) 
    {
      String image = tokenizer.nextToken();
      v.add(image);
    }
    return v;
  }

/**
* Gets whether this name is a directory
*
* @return true if it's a directory (in other words, its in the
* list of directories), else returns false
*/
  public boolean isDirectory(String d) 
  {
    return vdir.contains(d);
  }

}

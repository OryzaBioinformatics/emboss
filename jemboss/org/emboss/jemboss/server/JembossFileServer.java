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
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.server;

import org.emboss.jemboss.programs.*;
import org.emboss.jemboss.parser.*;
import org.apache.soap.rpc.Parameter;

import java.io.*;
import java.util.*;

public class JembossFileServer
{


//SITE SPECIFIC CHANGE USER DIRECTORIES HERE
  public Vector embreo_roots()
  {
    Vector vans = new Vector();
    vans.add("status");
    vans.add("0");
    vans.add("msg");
    vans.add("");

    vans.add("default-root");
    vans.add("HOME");

    vans.add("HOME");
    vans.add("/m3/users/tim");
 
    vans.add("SCRATCH");
    vans.add("/m3/users/tim/soap");
  
    return vans;
  }


/**
*
* Given the alias a user root alias e.g. "HOME" return
* the directory this represents
*
* @param root alias (e.g "HOME")
* @return directory path
*
*/
  private String getRoot(String s)
  {
    String rt = null;

    Vector userRoots = embreo_roots();

    for(int i=0; i<userRoots.size();i+=2)
    {
      String root = (String)userRoots.get(i);
      if(root.equalsIgnoreCase(s))
        return (String)userRoots.get(i+1);
    }

    return rt;
  }


  public Vector directory_shortls(String options, String dirname)
  {
    Vector vans = new Vector();
    int split = options.indexOf("=")+1;

    File dir = new File(getRoot(options.substring(split)) + "/" + dirname);

// filter out dot files
    File files[] = dir.listFiles(new FilenameFilter()
    {
      public boolean accept(File d, String n)
      {
        return !n.startsWith(".");
      }
    });

    String listAll = "";
    String listDir = "";

    for(int i=0;i<files.length;i++)
    {
      if(files[i].isDirectory())
        listDir = listDir.concat(files[i].getName() + "\n");
      listAll = listAll.concat(files[i].getName() + "\n");
    }

    vans.add("status");
    vans.add("0");
    vans.add("msg");
    vans.add("");
    vans.add("list");
    vans.add(listAll);
    vans.add("dirlist");
    vans.add(listDir);

    return vans;
  }

  public Vector get_file(String options, String filename)
  {
    Vector vans = new Vector();
//  File dir = new File("/m3/users/tim/"+filename);
    
    int split = options.indexOf("=")+1;    
    File dir = new File(getRoot(options.substring(split)) + "/" + filename);

    String line = new String("");
    String fc = new String("");
    try
    {
      BufferedReader in = new BufferedReader(new FileReader(dir));
      while((line = in.readLine()) != null)
        fc = fc.concat(line + "\n");
    }
    catch (IOException ioe){}
    vans.add("contents");
    vans.add(fc);
    vans.add("status");
    vans.add("0");
    vans.add("msg");
    vans.add("");

    return vans;
  } 


/**
*
* @param option determines the root directory to put the file
* @param filename name of the file to put
* @param filedata file contents
*
*/
  public Vector put_file(String options, String filename, byte[] filedata)
  {
    Vector vans = new Vector();

    int split = options.indexOf("=")+1;
    File f = new File(getRoot(options.substring(split)) + "/" + filename);

    try
    {
      FileOutputStream out = new FileOutputStream(f);
      out.write(filedata);
      out.close();
      vans.add("status");
      vans.add("0");
//    System.out.println("WRITTEN TO " + getRoot(options.substring(split)) +
//                       "/" + filename);
    }
    catch(IOException ioe)
    {
      vans.add("status");
      vans.add("1");
    }

    vans.add("msg");
    vans.add("");
   
    return vans;
  }
 
}


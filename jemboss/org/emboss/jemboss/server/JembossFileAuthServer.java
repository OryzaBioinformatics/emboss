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

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.parser.Ajax;

import java.io.*;
import java.util.*;

public class JembossFileAuthServer
{

  private String ps = new String(System.getProperty("path.separator"));

//get paths to EMBOSS
  JembossParams jp = new JembossParams();
  String embossBin = jp.getEmbossBin();
  String embossPath = jp.getEmbossPath();
  private String environ = "PATH=" + embossBin + ps + embossPath;


  public Vector embreo_roots(String userName, byte[] passwd)
  {

    Ajax aj = new Ajax();
    Vector vans = new Vector();
    if(!verifyUser(aj,userName,passwd,vans))
      return vans;

    String userHomeDir = aj.home+"/";

    vans.add("status");
    vans.add("0");
    vans.add("msg");
    vans.add("");

    vans.add("default-root");
    vans.add("HOME");

    vans.add("HOME");
    vans.add(userHomeDir);

    vans.add("EMBOSS results");
    vans.add("/tmp/SOAP/emboss/"+userName);
 
//ADD IN OTHER USER ROOT DIRECTORIES HERE

//  System.out.println(userHomeDir);
//  vans.add("SCRATCH");
//  vans.add("/m3/users/tim/soap");
  
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
  private String getRoot(String s, String userName, byte[] passwd)
  {
    String rt = null;
    Vector userRoots = embreo_roots(userName,passwd);
 
    for(int i=0; i<userRoots.size();i+=2)
    {
      String root = (String)userRoots.get(i);
      if(root.equalsIgnoreCase(s))
        return (String)userRoots.get(i+1);
    }

    return rt;
  }


  public Vector directory_shortls(String options, String dirname,
                           String userName, byte[] passwd)
  {

    Vector vans = new Vector();
    Ajax aj = new Ajax();
    if(!verifyUser(aj,userName,passwd,vans))
      return vans;

    int split = options.indexOf("=")+1;

    aj.listFiles(userName,passwd,environ,
                 getRoot(options.substring(split),userName,passwd)
                 + "/" + dirname);
    String listAll = aj.getOutStd();

//  System.out.println("listAll "+listAll+" STDERR "+aj.getErrStd());
    aj.listDirs(userName,passwd,environ,
                 getRoot(options.substring(split),userName,passwd)
                 + "/" + dirname);
    String listDir = aj.getOutStd();

    listAll = listAll.concat("\n"+listDir); 

    vans.add("status");
    vans.add("0");
    vans.add("msg");
    vans.add("");
    vans.add("list");
    vans.add(listAll);
    vans.add("dirlist");
    vans.add(listDir);

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return vans;
  }

  public Vector get_file(String options, String filename,
                           String userName, byte[] passwd)
  {

    Vector vans = new Vector();
    Ajax aj = new Ajax();

    int split = options.indexOf("=")+1;    
    String fullFileName = getRoot(options.substring(split),userName,passwd)
                         + "/" + filename;

    byte fbuf[] =  aj.getFile(userName,passwd,environ,
                              fullFileName);
    
    if(aj.getFileok()==1)
    {
      vans.add("contents");
      if(aj.getPrnt() == 1)
        vans.add(new String(fbuf));
      else
        vans.add(fbuf);
    }
    else
      System.out.println("Cannot getFile "+fullFileName);

    vans.add("status");
    vans.add("0");
    vans.add("msg");
    vans.add("");

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return vans;
  } 


/**
*
* @param option determines the root directory to put the file
* @param filename name of the file to put
* @param filedata file contents
*
*/
  public Vector put_file(String options, String filename, byte[] filedata,
                           String userName, byte[] passwd)
  {
    Vector vans = new Vector();
    Ajax aj = new Ajax();

    int split = options.indexOf("=")+1;
    String fullFilename = getRoot(options.substring(split),userName,passwd)+
                            "/" + filename;
    aj.putFile(userName,passwd,environ,fullFilename,filedata);

    vans.add("msg");
    vans.add("");
 
    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return vans;
  }
 
  private boolean verifyUser(Ajax aj, String userName,
                             byte[] passwd, Vector res)
  {

    if(userName == null || passwd == null)
    {
//    System.out.println("Failed Authorisation "+userName);
      res.add("msg");
      res.add("Failed Authorisation ");
      res.add("status");
      res.add("1");
      return false;
    }

    boolean ok = false;

    try
    {
      ok = aj.userAuth(userName,passwd,environ);
    }
    catch(Exception exp) 
    {
      ok = false;
    }

    if(!ok)
    {
      System.out.println("Failed Authorisation "+userName);
      System.out.println("STDERR "+aj.getErrStd());
      res.add("msg");
      res.add("Failed Authorisation "+userName);
      res.add("status");
      res.add("1");
      return false;
    }

    return true;

  }

}


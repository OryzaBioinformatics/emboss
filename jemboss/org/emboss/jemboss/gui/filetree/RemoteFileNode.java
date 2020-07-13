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

import java.awt.datatransfer.*;
import javax.swing.tree.*;
import java.io.*;
import java.util.*;

import org.emboss.jemboss.soap.*;

import org.emboss.jemboss.JembossParams;


public class RemoteFileNode extends DefaultMutableTreeNode 
                    implements Transferable, Serializable
{
    private boolean explored = false;
    private boolean isDir = false;
    private String fullname;
    private String serverPathToFile;

    private String rootdir;   
    private transient FileList parentList;        // make transient for
    private transient JembossParams mysettings;    // Transferable to work
    private transient FileRoots froots;

    private String fs = "/";

    final public static DataFlavor REMOTEFILENODE = 
           new DataFlavor(RemoteFileNode.class, "Remote file");
    static DataFlavor flavors[] = { REMOTEFILENODE, DataFlavor.stringFlavor };


    public RemoteFileNode(JembossParams mysettings, FileRoots froots,
                    String file, FileList parentList, String parent)
    {
      this(mysettings, froots, file, parentList, parent, false);
    }

    public RemoteFileNode(JembossParams mysettings, FileRoots froots,
                    String file, FileList parentList, String parent,
                    boolean ldir)
    { 
      this.mysettings = mysettings;
      this.froots = froots;
      this.parentList = parentList;
      isDir = ldir;
      rootdir = froots.getCurrentRoot();
      serverPathToFile = (String)froots.getRoots().get(rootdir);

      if(file.equals(" "))
        isDir = true;

      if(parent != null)
      {
        if(parent.endsWith("/."))
          parent = parent.substring(0,parent.length()-1);
        else if(parent.endsWith("/"))
          parent = parent.substring(0,parent.length());

        if(parent.equals("."))
          fullname = file;
        else
        {
          fullname = parent + fs + file;
          serverPathToFile = serverPathToFile.concat(fs+parent);
        }
      }
    
      if(parentList != null)
      {
        if(parentList.isDirectory(file))
          isDir = true;
      }
      else if(parent == null)
        fullname = ".";

      setUserObject(file); 
    }
   
    public boolean getAllowsChildren() { return isDir; }
    public boolean isLeaf() { return !isDir; }
    public boolean isDirectory() { return isDir; }
    public String getFile() { return (String)getUserObject(); }
    public String getRootDir() { return rootdir; }
    public String getFullName() { return fullname; }
    public String getPathName() { return serverPathToFile; }
    public boolean isExplored() { return explored; }
    public String getServerName() 
    { 
      String prefix = (String)froots.getRoots().get(froots.getCurrentRoot());
      if(!prefix.endsWith(fs))
        prefix = prefix.concat(fs);
      return prefix + fullname;
    }

    public void explore() 
    {
      if(!isDir)
        return;

      if(!explored)
      {
        try
        {
//        System.out.println(froots.getCurrentRoot() + " :: " + fullname);
          FileList efl = new FileList(mysettings,
                                   froots.getCurrentRoot(),fullname);
          Vector children = efl.fileVector();
          for(int i=0;i<children.size();i++)
            add(new RemoteFileNode(mysettings,froots,(String)children.get(i),
                                   efl,fullname));
        }
        catch(JembossSoapException eae) {}
      }
      explored = true;
    }

// Transferable
    public DataFlavor[] getTransferDataFlavors()
    {
      return flavors;
    }
 
    public boolean isDataFlavorSupported(DataFlavor f)
    {
      if(f.equals(REMOTEFILENODE) || f.equals(DataFlavor.stringFlavor))
        return true;
      return false;
    }

    public Object getTransferData(DataFlavor d) 
        throws UnsupportedFlavorException, IOException
    {
      if(d.equals(REMOTEFILENODE))
        return this;
      else if(d.equals(DataFlavor.stringFlavor))
        return getServerName();
      else throw new UnsupportedFlavorException(d);
    } 

// Serializable    
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


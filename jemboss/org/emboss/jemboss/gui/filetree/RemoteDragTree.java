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
import java.awt.event.*;
import java.awt.datatransfer.*;
import java.awt.dnd.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.io.*;
import java.util.*;

import org.emboss.jemboss.gui.ResultsMenuBar;
import org.emboss.jemboss.soap.*;
import uk.ac.mrc.hgmp.embreo.EmbreoParams;
import uk.ac.mrc.hgmp.embreo.EmbreoAuthException;
import uk.ac.mrc.hgmp.embreo.filemgr.EmbreoFileRoots;
import uk.ac.mrc.hgmp.embreo.filemgr.EmbreoFileRequest;
import uk.ac.mrc.hgmp.embreo.filemgr.EmbreoFileGet;
import org.apache.soap.rpc.*;

/**
*
* Creates a remote file tree which is a drag source & sink
*
*/
class RemoteDragTree extends JTree implements DragGestureListener,
                           DragSourceListener, DropTargetListener 
{

  public static DefaultTreeModel model;
  private EmbreoParams mysettings; 
  private EmbreoFileRoots froots;

  private String fs = new String(System.getProperty("file.separator"));
  final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);


  public RemoteDragTree(EmbreoParams mysettings, EmbreoFileRoots froots,
                        final JPanel viewPane) 
  {

    this.mysettings = mysettings;
    this.froots = froots;

    DragSource dragSource = DragSource.getDefaultDragSource();
    dragSource.createDefaultDragGestureRecognizer(
             this,                             // component where drag originates
             DnDConstants.ACTION_COPY_OR_MOVE, // actions
             this);                            // drag gesture recognizer

    setDropTarget(new DropTarget(this,this));
    model = createTreeModel(" ");
    setModel(model);
    createTreeModelListener();

    this.getSelectionModel().setSelectionMode
                  (TreeSelectionModel.SINGLE_TREE_SELECTION);

    //Listen for when a file is selected
    addMouseListener(new MouseListener()
    {
      public void mouseClicked(MouseEvent me)
      {
        if(me.getClickCount() == 2 && isFileSelection())
        {
          setCursor(cbusy);
          RemoteFileNode node = (RemoteFileNode)getLastSelectedPathComponent();
          if(node==null)
            return;
          if(node.isLeaf())
            showFilePane(node.getFullName());
          setCursor(cdone);
        }
      }
      public void mousePressed(MouseEvent me){}
      public void mouseEntered(MouseEvent me){}
      public void mouseExited(MouseEvent me){}
      public void mouseReleased(MouseEvent me){}
    });

    addTreeExpansionListener(new TreeExpansionListener()
    {
      public void treeExpanded(TreeExpansionEvent e) 
      {
        TreePath path = e.getPath();
        if(path != null) 
        {
          setCursor(cbusy);
          RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();

          if(!node.isExplored()) 
          {  
            model = (DefaultTreeModel)getModel();
            node.explore();
            model.nodeStructureChanged(node);
          }
          setCursor(cdone);
        }
      }
      public void treeCollapsed(TreeExpansionEvent e){}
    });

  }


  public void dragGestureRecognized(DragGestureEvent e) 
  {
    // drag only files 
    if(isFileSelection())
      e.startDrag(DragSource.DefaultCopyDrop, // cursor
                 (Transferable)getNodename(), // transferable data
                                       this); // drag source listener
  }

// Source
  public void dragDropEnd(DragSourceDropEvent e) {}
  public void dragEnter(DragSourceDragEvent e) {}
  public void dragExit(DragSourceEvent e) {}
  public void dragOver(DragSourceDragEvent e) {}
  public void dropActionChanged(DragSourceDragEvent e) {}
//public void dragEnter(DragSourceDropEvent e){}
//public void dragOver(DragSourceDropEvent e) {}  


// Target
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(FileNode.FILENODE))
    {
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
      System.out.println("dragEnter");
    }
  }

  public void drop(DropTargetDropEvent e)
  {
    Transferable t = e.getTransferable();

    if(t.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
       System.out.println("Detected local drop");
    else if(t.isDataFlavorSupported(FileNode.FILENODE))
    {
      try
      {
        Point ploc = e.getLocation();
        TreePath dropPath = getPathForLocation(ploc.x,ploc.y);
        if (dropPath != null) 
        {
          FileNode fn = (FileNode)t.getTransferData(FileNode.FILENODE);
          File lfn = fn.getFile();

          String dropDest = null;
          RemoteFileNode fdropPath = (RemoteFileNode)dropPath.getLastPathComponent();
          String dropRoot = fdropPath.getRootDir();
          if(fdropPath.isLeaf()) 
          {
            RemoteFileNode pn = (RemoteFileNode)fdropPath.getParent();
            dropDest = pn.getFullName() + "/" + lfn.getName(); //assumes unix file sep.!
          } 
          else 
            dropDest = fdropPath.getFullName() + "/" + lfn.getName();

          try 
          {
            Vector params = new Vector();
            byte[] fileData = getLocalFile(lfn);
            params.addElement(new Parameter("options", String.class,
                              "fileroot=" + dropRoot, null));
            params.addElement(new Parameter("filename", String.class,
                              dropDest, null));
            params.addElement(new Parameter("filedata", fileData.getClass(),
                              fileData, null));
            EmbreoFileRequest gReq = new EmbreoFileRequest(mysettings,"put_file",params);
            System.out.println("DROPPED - SUCCESS!!!");
          } 
          catch (Exception exp) 
          {
            System.out.println("RemoteDragTree: caught exception " + dropRoot +
                " Destination: " + dropDest + " Local File " + lfn.toString());
          }
        }
      }
      catch (Exception ex) {}
    } 
    else
    {
      e.rejectDrop();
      return;
    }

  }

/**
*
* When a suitable DataFlavor is offered over a remote file
* node the node is highlighted/selected and the drag
* accepted. Otherwise the drag is rejected.
*
*/
  public void dragOver(DropTargetDragEvent e)
  {
    if (e.isDataFlavorSupported(FileNode.FILENODE))
    {
      Point ploc = e.getLocation();
      TreePath ePath = getPathForLocation(ploc.x,ploc.y);
      if (ePath == null)
        e.rejectDrag();
      else
      {
        setSelectionPath(ePath);
        e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
      }
    }
    else
    {
      e.rejectDrag();
    }
  }

  public void dropActionChanged(DropTargetDragEvent e) {}
  public void dragExit(DropTargetEvent e){}

  public byte[] getLocalFile(File name)
  {
    byte[] b = null;
    try
    {
      long s = name.length();
      b = new byte[(int)s];
      FileInputStream fi = new FileInputStream(name);
      fi.read(b);
      fi.close();
    } 
    catch (IOException ioe) 
    {
      System.out.println("Cannot read file: " + name);
    }
    return b;
  }

  public RemoteFileNode getNodename()
  {
    TreePath path = getLeadSelectionPath();
    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return node;
  }


//
  public boolean isFileSelection()
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return false;

    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return !node.isDirectory();
  }

  public String getFilename()
  {
    TreePath path = getLeadSelectionPath();
    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return node.getServerName();
  }

  private DefaultTreeModel createTreeModel(String root) 
  {
    RemoteFileNode rootNode = new RemoteFileNode(mysettings,froots,root,null,null);
    rootNode.explore();
    return new DefaultTreeModel(rootNode);
  }

  public DefaultTreeModel getTreeModel () 
  {
    return model;
  }


/**
*
* Adding a file (or directory) to the file tree manager.
* This looks to see if the directory has already been opened
* and updates the filetree if it has.
* @param file to add to the tree
*
*/
//  public DefaultMutableTreeNode addObject(String child, String path)
//  {

//    return childNode;
//  }


/**
*
* Opens a JFrame with the file contents displayed.
* @param the file name
*
*/
  public void showFilePane(String filename)
  {
    setCursor(cbusy);

    try
    {
      JFrame ffile = new JFrame(filename);
      JPanel pfile = (JPanel)ffile.getContentPane();
      pfile.setLayout(new BorderLayout());
      JPanel pscroll = new JPanel(new BorderLayout());
      JScrollPane rscroll = new JScrollPane(pscroll);

      EmbreoFileGet efg = new EmbreoFileGet(mysettings, froots.getCurrentRoot(),filename);
      FileEditorDisplay fed = new FileEditorDisplay(ffile,filename,efg.contents());
      new ResultsMenuBar(ffile,fed);
      pfile.add(rscroll, BorderLayout.CENTER);
      JTextPane seqText = fed.getJTextPane();
      pscroll.add(seqText, BorderLayout.CENTER);
      seqText.setCaretPosition(0);
      ffile.setSize(450,400);
      ffile.setVisible(true);
    }
    catch(EmbreoAuthException eae)
    {
      setCursor(cdone);
    }
    setCursor(cdone);
  }


}

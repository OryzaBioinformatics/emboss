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
import org.emboss.jemboss.JembossParams;

/**
*
* Creates a remote file tree which is a drag source & sink
*
*/
public class RemoteDragTree extends JTree implements DragGestureListener,
                           DragSourceListener, DropTargetListener, ActionListener 
{

  private JembossParams mysettings; 
  private static FileRoots froots;

  private JPopupMenu popup;
  private String fs = new String(System.getProperty("file.separator"));
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

  public RemoteDragTree(final JembossParams mysettings, FileRoots froots,
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
    DefaultTreeModel model = createTreeModel(" ");
    setModel(model);
    createTreeModelListener();

    this.getSelectionModel().setSelectionMode
                  (TreeSelectionModel.SINGLE_TREE_SELECTION);


    // Popup menu
    addMouseListener(new PopupListener());
    popup = new JPopupMenu();
    JMenuItem menuItem = new JMenuItem("Refresh");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    popup.add(new JSeparator());
//open menu
    JMenu openMenu = new JMenu("Open With");
    popup.add(openMenu);
    menuItem = new JMenuItem("Jemboss Aligmnment Editor");
    menuItem.addActionListener(this);
    openMenu.add(menuItem);
    menuItem = new JMenuItem("Text Editor");
    menuItem.addActionListener(this);
    openMenu.add(menuItem);

    menuItem = new JMenuItem("Rename...");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("New Folder...");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("Delete...");
    menuItem.addActionListener(this);
    popup.add(menuItem);

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
            showFilePane(node.getFullName(),mysettings);
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
            DefaultTreeModel model = (DefaultTreeModel)getModel();
            node.explore();
            model.nodeStructureChanged(node);
          }
          setCursor(cdone);
        }
      }
      public void treeCollapsed(TreeExpansionEvent e){}
    });

  }


  /**
  *
  * This is used to refresh the file manager
  *
  */
  public void refreshRoot()
  {
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model = createTreeModel(" ");
    setModel(model);
  }

  /**
  *
  * Popup menu actions
  *
  */
  public void actionPerformed(ActionEvent e)
  {

    JMenuItem source = (JMenuItem)(e.getSource());
    final RemoteFileNode node = getNodename();
    if(node == null)
    {
      JOptionPane.showMessageDialog(null,"No file selected.",
                                    "Warning",
                                    JOptionPane.WARNING_MESSAGE);
      return;
    }

    final String fn = node.getFullName();
    final String parent = node.getPathName();
    String rootPath = node.getRootDir();
    RemoteFileNode pn = node;

    if(source.getText().equals("Refresh"))
    {
      refreshRoot();
    }
    else if(source.getText().equals("Jemboss Aligmnment Editor"))
    {
      Vector params = new Vector();
      String options= "fileroot=" + froots.getCurrentRoot();
      params.addElement(options);
      params.addElement(fn);
  
      try
      {
        PrivateRequest gReq = new PrivateRequest(mysettings,"EmbreoFile",
                                                    "get_file",params);
        new org.emboss.jemboss.editor.AlignJFrame((String)gReq.getHash().get("contents"),
                                                  fn);
      }
      catch(JembossSoapException eae)
      {
        System.out.println("RemoteDragTree :: JembossSoapException "+fn);
      }
    }
    else if(source.getText().equals("Text Editor"))
      showFilePane(fn, mysettings);
    else if(source.getText().equals("New Folder..."))
    {
      final String inputValue = JOptionPane.showInputDialog(null,
                          "Folder Name","Create New Folder in",
                          JOptionPane.QUESTION_MESSAGE);

      String dropDest = null;
    
      if(node.isLeaf())
      {
        pn = (RemoteFileNode)node.getParent();
        dropDest = pn.getFullName() + "/" + inputValue; //assumes unix file sep.!
      }
      else
        dropDest = node.getFullName() + "/" + inputValue;

      if(inputValue != null && !inputValue.equals("") )
      {
        final RemoteFileNode pnn = pn;

        Vector params = new Vector();
        params.addElement("fileroot=" + rootPath);
        params.addElement(dropDest);

        try
        {
          setCursor(cbusy);
          PrivateRequest gReq = new PrivateRequest(mysettings,
                                 "EmbreoFile","mkdir",params);
          setCursor(cdone);
          Runnable addDirToTree = new Runnable()
          {
            public void run () { addObject(pnn,inputValue,true); };
          };
          SwingUtilities.invokeLater(addDirToTree);
        }
        catch (JembossSoapException jse)
        {
          setCursor(cdone);
        }
      }
    }
    else if(source.getText().equals("Delete..."))
    {
      String sname = node.getServerName();
      int n = JOptionPane.showConfirmDialog(null,
             "Delete "+sname+"?", "Delete "+sname,
             JOptionPane.YES_NO_OPTION);
      if(n == JOptionPane.YES_OPTION)
      {
        String dropDest = pn.getFullName();
        Vector params = new Vector();
        params.addElement("fileroot=" + rootPath);
        params.addElement(dropDest);

        if(node.isLeaf())         // file deletion
        {
          try
          {
            setCursor(cbusy);
            PrivateRequest gReq = new PrivateRequest(mysettings,
                                   "EmbreoFile","delFile",params);
            setCursor(cdone);
            Runnable deleteFileFromTree = new Runnable()
            {
              public void run () { deleteObject(node,fn); };
            };
            SwingUtilities.invokeLater(deleteFileFromTree);
          }
          catch (JembossSoapException jse)
          {
            setCursor(cdone);
          }
        }
        else                      // directory deletion
        {
          try
          {
            FileList efl = new FileList(mysettings,rootPath,dropDest);
            Vector children = efl.fileVector();
            if(children.size() > 0)
              JOptionPane.showMessageDialog(null,"Cannot delete\n"+
                     sname+"\nthis directory is not empty","Warning",
                     JOptionPane.ERROR_MESSAGE);
            else
            {
              setCursor(cbusy);
              PrivateRequest gReq = new PrivateRequest(mysettings,
                                   "EmbreoFile","delDir",params);
              setCursor(cdone);
              Runnable deleteFileFromTree = new Runnable()
              {
                public void run () { deleteObject(node,fn); };
              };
              SwingUtilities.invokeLater(deleteFileFromTree);
            }
          }
          catch (JembossSoapException jse)
          {
            setCursor(cdone);
          }
        }
      }
    }
    else if(source.getText().equals("Rename..."))
    {
      if(node.isLeaf())
      {
        final String inputValue = JOptionPane.showInputDialog(null,
                                "New File Name","Rename "+fn,
                                JOptionPane.QUESTION_MESSAGE);

        pn = (RemoteFileNode)node.getParent();

        if(inputValue != null && !inputValue.equals("") )
        {
          final RemoteFileNode pnn = pn;
          String newfile   = parent+"/"+inputValue;

          Vector params = new Vector();
          params.addElement("fileroot=" + rootPath);
          params.addElement(fn);
          params.addElement(newfile);

          try
          {
            setCursor(cbusy);
            PrivateRequest gReq = new PrivateRequest(mysettings,
                                   "EmbreoFile","rename",params);
            setCursor(cdone);
            Runnable deleteFileFromTree = new Runnable()
            {
              public void run () 
              { 
                addObject(pnn,inputValue,false);
                deleteObject(node,fn); 
              };
            };
            SwingUtilities.invokeLater(deleteFileFromTree);
          }
          catch (JembossSoapException jse)
          {
            setCursor(cdone);
          }

        }
      }
    }
  }

  public void deleteObject(RemoteFileNode node, String parentPath)
  {
    RemoteFileNode parentNode = (RemoteFileNode)node.getParent();
    DefaultTreeModel model = (DefaultTreeModel)getModel();

    if(parentNode == null)
      return;
    else if(!parentNode.isExplored())
    {
      parentNode.explore();
      model.nodeStructureChanged(parentNode);
    }

    model.removeNodeFromParent(node);
    return;
  }


  public void dragGestureRecognized(DragGestureEvent e) 
  {
    // ignore if mouse popup trigger
    InputEvent ie = e.getTriggerEvent();
    if(ie instanceof MouseEvent)
      if(((MouseEvent)ie).isPopupTrigger())
        return;

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

// Target
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(FileNode.FILENODE))
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
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
            dropDest = fdropPath.getFullName()+ "/" + lfn.getName();

          try 
          {
            Vector params = new Vector();
            byte[] fileData = getLocalFile(lfn);
            params.addElement("fileroot=" + dropRoot);
            params.addElement(dropDest);
            params.addElement(fileData);

            setCursor(cbusy);
            PrivateRequest gReq = new PrivateRequest(mysettings,"EmbreoFile",
                                                           "put_file",params);
            setCursor(cdone);
            //add file to remote file tree
            RemoteFileNode parentNode = fdropPath;
            if(parentNode.isLeaf())
              parentNode = (RemoteFileNode)fdropPath.getParent();
            else
              parentNode = fdropPath;
           
            if(parentNode.isExplored())
              addObject(parentNode,lfn.getName(),false);
          } 
          catch (Exception exp) 
          {
            setCursor(cdone);
            System.out.println("RemoteDragTree: caught exception " + dropRoot +
                " Destination: " + dropDest + " Local File " + lfn.toString());
          }
        }
      }
      catch (Exception ex) 
      {
      }
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
    if(path == null)
      return null;
    RemoteFileNode node = (RemoteFileNode)path.getLastPathComponent();
    return node;
  }


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
    setCursor(cbusy);
    RemoteFileNode rootNode = new RemoteFileNode(mysettings,froots,
                                                   root,null,null);
    rootNode.explore();
    setCursor(cdone);
    return new DefaultTreeModel(rootNode);
  }


/**
*
* Adding a file (or directory) to the file tree manager.
* This looks to see if the directory has already been opened
* and updates the filetree if it has.
* @param file to add to the tree
*
*/
  public void addObject(RemoteFileNode parentNode,String child,
                        boolean ldir)
  {
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    String path = parentNode.getFullName();
    //create new file node
    if(path.equals(" "))
      path = "";
    File newleaf = new File(path + "/" + child);

    RemoteFileNode childNode = new RemoteFileNode(mysettings,froots,
                                              child,null,path,ldir);

    //find the index for the child
    int num = parentNode.getChildCount();
    int childIndex = num;
    for(int i=0;i<num;i++)
    {
      String nodeName = ((RemoteFileNode)parentNode.getChildAt(i)).getFile();
      if(nodeName.compareTo(child) > 0)
      {
        childIndex = i;
        break;
      }
      else if (nodeName.compareTo(child) == 0)  //file already exists
      {
        childIndex = -1;
        break;
      }
    }
    if(childIndex != -1)
      model.insertNodeInto(childNode,parentNode,childIndex);

    // Make sure the user can see the new node.
    this.scrollPathToVisible(new TreePath(childNode.getPath()));

    return; 
  }


/**
*
* Opens a JFrame with the file contents displayed.
* @param the file name
*
*/
  public static void showFilePane(String filename, JembossParams mysettings)
  {
    try
    {
      JFrame ffile = new JFrame(filename);
      JPanel pfile = (JPanel)ffile.getContentPane();
      pfile.setLayout(new BorderLayout());

      Vector params = new Vector();
      String options= "fileroot=" + froots.getCurrentRoot();
      params.addElement(options);
      params.addElement(filename);

      PrivateRequest gReq = new PrivateRequest(mysettings,"EmbreoFile",
                                                    "get_file",params);
       
      FileEditorDisplay fed = new FileEditorDisplay(ffile,filename,
                                   gReq.getHash().get("contents"));
      new ResultsMenuBar(ffile,fed,mysettings);
      JScrollPane rscroll = new JScrollPane(fed);

      pfile.add(rscroll, BorderLayout.CENTER);
      fed.setCaretPosition(0);
      ffile.pack();
      ffile.setSize(450,400);
      ffile.setVisible(true);
    }
    catch(JembossSoapException eae)
    {  
    }
    
  }

  class PopupListener extends MouseAdapter
  {
    public void mousePressed(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    public void mouseReleased(MouseEvent e)
    {
      maybeShowPopup(e);
    }

    private void maybeShowPopup(MouseEvent e)
    {
      if(e.isPopupTrigger())
        popup.show(e.getComponent(),
                e.getX(), e.getY());
    }
  }

}

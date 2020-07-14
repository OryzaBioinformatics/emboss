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
import java.util.Vector;
import java.util.Enumeration;

import org.emboss.jemboss.soap.PrivateRequest;
import org.emboss.jemboss.gui.ResultsMenuBar;
import org.emboss.jemboss.JembossParams;

/**
*
* Creates a local file tree for Jemboss. This acts as a drag 
* source and sink for files.
*
*/
public class DragTree extends JTree implements DragGestureListener,
                 DragSourceListener, DropTargetListener, ActionListener 
{

  private JembossParams mysettings;
  private File root;
  private Vector openNode;
  private String fs = new String(System.getProperty("file.separator"));
  private JPopupMenu popup;
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);


  public DragTree(File rt, final JFrame f, final JembossParams mysettings) 
  {
    this.mysettings = mysettings;
    this.root = rt;

    DragSource dragSource = DragSource.getDefaultDragSource();

    dragSource.createDefaultDragGestureRecognizer(
       this,                             // component where drag originates
       DnDConstants.ACTION_COPY_OR_MOVE, // actions
       this);                            // drag gesture recognizer

    setDropTarget(new DropTarget(this,this));
    DefaultTreeModel model = createTreeModel(root);
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
    MouseListener mouseListener = new MouseAdapter() 
    {
      public void mouseClicked(MouseEvent me) 
      {
        if(me.getClickCount() == 2 && isFileSelection() &&
           !me.isPopupTrigger()) 
        {
          f.setCursor(cbusy);
          FileNode node = getSelectedNode();
          String selected = node.getFile().getAbsolutePath();
          showFilePane(selected,mysettings);
          f.setCursor(cdone);
        }
      }
    };
    this.addMouseListener(mouseListener);

    addTreeExpansionListener(new TreeExpansionListener()
    {
      public void treeCollapsed(TreeExpansionEvent e){} 
      public void treeExpanded(TreeExpansionEvent e) 
      {
        TreePath path = e.getPath();
        if(path != null) 
        {
          FileNode node = (FileNode)path.getLastPathComponent();

          if(!node.isExplored()) 
          {  
            f.setCursor(cbusy);
            DefaultTreeModel model = (DefaultTreeModel)getModel();
            node.explore();
            openNode.add(node);
            model.nodeStructureChanged(node);
            f.setCursor(cdone);
          }
        }
      }
    });

  }

  /**
  *
  * Popup menu actions
  * 
  */
  public void actionPerformed(ActionEvent e) 
  {

    JMenuItem source = (JMenuItem)(e.getSource());
    final FileNode node = getSelectedNode();


    if(source.getText().equals("Refresh")) 
    {
      if(node == null)
        newRoot(root.getAbsolutePath());
      else if(node.isLeaf())
        refresh((FileNode)node.getParent());
      else 
        refresh(node);
      return;
    }

    if(node == null)
    {
      JOptionPane.showMessageDialog(null,"No file selected.",
                        "Warning",
                        JOptionPane.WARNING_MESSAGE);
      return;
    }
    
    final File f = node.getFile();

    if(source.getText().equals("Jemboss Aligmnment Editor"))
      new org.emboss.jemboss.editor.AlignJFrame(f);
    else if(source.getText().equals("Text Editor"))
      showFilePane(f.getAbsolutePath(), mysettings);
    else if(source.getText().equals("New Folder..."))
    {
      String path = null;
      if(isFileSelection())
        path = f.getParent();
      else
        path = f.getAbsolutePath();

      String inputValue = JOptionPane.showInputDialog(null,
                    "Folder Name","Create New Folder in "+path,
                    JOptionPane.QUESTION_MESSAGE);

      if(inputValue != null && !inputValue.equals("") )
      {
        String fullname = path+fs+inputValue;
        File dir = new File(fullname);
        
        if(dir.exists())
          JOptionPane.showMessageDialog(null, fullname+" alread exists!",
                                   "Error", JOptionPane.ERROR_MESSAGE);
        else
        {
          if(dir.mkdir())
            addObject(inputValue,path,node);
          else
            JOptionPane.showMessageDialog(null,
                       "Cannot make the folder\n"+fullname,
                       "Error", JOptionPane.ERROR_MESSAGE);     
        }
      }
    }
    else if(source.getText().equals("Delete..."))
    {
      int n = JOptionPane.showConfirmDialog(null,
                                 "Delete "+f.getAbsolutePath()+"?",
                                 "Delete f.getAbsolutePath()",
                                 JOptionPane.YES_NO_OPTION);
      if(n == JOptionPane.YES_OPTION)
      {
        if(f.delete())
        {
          Runnable deleteFileFromTree = new Runnable()
          {
            public void run () { deleteObject(node); };
          };
          SwingUtilities.invokeLater(deleteFileFromTree);
        }
        else
          JOptionPane.showMessageDialog(null,"Cannot delete\n"+
                             f.getAbsolutePath(),"Warning",
                             JOptionPane.ERROR_MESSAGE);
      }
    }
    else if(isFileSelection() && source.getText().equals("Rename..."))
    {
      final String inputValue = JOptionPane.showInputDialog(null,
                              "New File Name","Rename "+f.getName(), 
                              JOptionPane.QUESTION_MESSAGE);

      if(inputValue != null && !inputValue.equals("") )
      {
        String path = f.getParent();
        String fullname   = path+fs+inputValue;
        File newFile = new File(fullname);

        try
        {
          renameFile(f,node,newFile.getCanonicalPath());
        }
        catch(IOException ioe){}
      }
    }
  }

/**
*
* Method to rename a file and update the filenode's.
* @param File oldFile file to rename
* @param FileNode oldNode filenode to be removed
* @param String newFullName name of the new file
*
*/
  private void renameFile(final File oldFile, final FileNode oldNode, 
                          String newFullName)
  {
    final File fnew = new File(newFullName);
    if(fnew.exists())
      JOptionPane.showMessageDialog(null, newFullName+" alread exists!",
                               "Warning", JOptionPane.ERROR_MESSAGE);
    else
    {
      if(oldFile.renameTo(fnew))
      {
        Runnable renameFileInTree = new Runnable()
        {
          public void run ()
          {
            addObject(fnew.getName(),fnew.getParent(),oldNode);
	    deleteObject(oldNode);
          };
        };
        SwingUtilities.invokeLater(renameFileInTree);
      }
      else
      {
        JOptionPane.showMessageDialog(null, 
                   "Cannot rename \n"+oldFile.getAbsolutePath()+
                   "\nto\n"+fnew.getAbsolutePath(), "Rename Error",
                   JOptionPane.ERROR_MESSAGE);
      }
    }
    return;
  }

/**
* 
* @param String newRoot directory yo use as the root for
*        the tree.
*
*/
  public void newRoot(String newRoot)
  {
    root = new File(newRoot);
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model = createTreeModel(root);
    setModel(model);
  }

/**
*
* @param FileNode node to refresh 
*
*/
  public void refresh(FileNode node)
  {
    node.reExplore();
    DefaultTreeModel model = (DefaultTreeModel)getModel();
    model.nodeStructureChanged(node);
  }

// drag source
  public void dragGestureRecognized(DragGestureEvent e) 
  {
    // ignore if mouse popup trigger
    InputEvent ie = e.getTriggerEvent();
    if(ie instanceof MouseEvent) 
      if(((MouseEvent)ie).isPopupTrigger()) 
        return;

    // drag only files 
    if(isFileSelection())
      e.startDrag(DragSource.DefaultCopyDrop,     // cursor
                 (Transferable)getSelectedNode(), // transferable data
                                       this);     // drag source listener
  }
  public void dragDropEnd(DragSourceDropEvent e) {}
  public void dragEnter(DragSourceDragEvent e) {}
  public void dragExit(DragSourceEvent e) {}
  public void dragOver(DragSourceDragEvent e) {}
  public void dropActionChanged(DragSourceDragEvent e) {}

// drop sink
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
  }

  public void drop(DropTargetDropEvent e)
  {

    Transferable t = e.getTransferable();
    final FileNode dropNode = getSelectedNode();
    if(dropNode == null)
    {
      e.rejectDrop();
      return;
    }

    //local drop
    if(t.isDataFlavorSupported(FileNode.FILENODE))
    {
//     try
//     {
//       FileNode fn = (FileNode)t.getTransferData(FileNode.FILENODE);
//       if (dropNode.isLeaf())
//       {
//         e.rejectDrop();
//         return;
//       }
//      
//       String dropDir = dropNode.getFile().getAbsolutePath();
//       String newFullName = dropDir+fs+fn.toString();
//       renameFile(fn.getFile(),fn,newFullName);  
//     }
//     catch(Exception ufe){}        
    }
    //remote drop
    else if(t.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
    {
      try
      {
        final RemoteFileNode fn = 
          (RemoteFileNode)t.getTransferData(RemoteFileNode.REMOTEFILENODE);
        File dropDest = null;
        String dropDir = null;
        if (dropNode.isLeaf()) 
        {
          FileNode pn = (FileNode)dropNode.getParent();
          dropDir = pn.getFile().getAbsolutePath();
          dropDest = new File(dropDir,fn.getFile());
        } 
        else 
        {
          dropDir = dropNode.getFile().getAbsolutePath();
          dropDest = new File(dropDir,fn.getFile());
        }
        try
        {
          setCursor(cbusy);
                                //check we want to & can save
          FileSave fsave = new FileSave(dropDest); 
          if(fsave.doWrite())
          {
            Vector params = new Vector();
            params.addElement("fileroot=" + fn.getRootDir());
            params.addElement(fn.getFullName());
            PrivateRequest gReq = new PrivateRequest(mysettings,
                                "EmbreoFile","get_file",params);

            fsave.fileSaving(gReq.getHash().get("contents"));

            if(fsave.writeOK() && !fsave.fileExists())
            {
              final String ndropDir = dropDir;
              Runnable updateTheTree = new Runnable() 
              {
                public void run () 
                { 
                  addObject(fn.getFile(),ndropDir,dropNode); 
                };
              };
              SwingUtilities.invokeLater(updateTheTree);
            }
          }
          setCursor(cdone);
        } 
        catch (Exception exp) 
        {
          setCursor(cdone);
          System.out.println("DragTree: caught exception");
        }
        e.getDropTargetContext().dropComplete(true);   
      }
      catch (Exception exp)
      {
        e.rejectDrop();
        return;
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
    if (e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
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
//  else if(e.isDataFlavorSupported(FileNode.FILENODE))
//  {
//    Point ploc = e.getLocation();
//    TreePath ePath = getPathForLocation(ploc.x,ploc.y);
//    if (ePath == null)
//    {
//      e.rejectDrag();
//      return;
//    }
//    FileNode node = (FileNode)ePath.getLastPathComponent();
//    if(!node.isDirectory())
//      e.rejectDrag();
//    else 
//    {
//      setSelectionPath(ePath);
//      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
//    }
//  }
    else
      e.rejectDrag();
    
    return;
  }

  public void dropActionChanged(DropTargetDragEvent e) {}
  public void dragExit(DropTargetEvent e){}

/**
*
* @return String path name to file that is currently selected
*
*/
  public String getFilename()
  {
    TreePath path = getLeadSelectionPath();
    FileNode node = (FileNode)path.getLastPathComponent();
    return ((File)node.getUserObject()).getAbsolutePath();
  }

/**
*
* @return FileNode node that is currently selected
*
*/
  public FileNode getSelectedNode()
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return null;
    FileNode node = (FileNode)path.getLastPathComponent();
//  System.out.println(node.getFile().getAbsolutePath());
    return node;
  }

/**
*
* @return boolean true is a file is selected, false if
*         a directory is selected
*
*/
  public boolean isFileSelection() 
  {
    TreePath path = getLeadSelectionPath();
    if(path == null)
      return false;

    FileNode node = (FileNode)path.getLastPathComponent();
    return !node.isDirectory();
  }

/**
*
* Make the given directory the root and create a new
* DefaultTreeModel.
* @param File root the root directory
* @param DefaultTreeModel with the root node set to the
*        given directory
*
*/
  private DefaultTreeModel createTreeModel(File root) 
  {
    FileNode rootNode = new FileNode(root);
    rootNode.explore();
    openNode = new Vector();
    openNode.add(rootNode);
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
  public DefaultMutableTreeNode addObject(String child,
                            String path, FileNode node)
  {

    DefaultTreeModel model = (DefaultTreeModel)getModel();
    if(node == null)
    {
      node = getNode(path); 
      if(node==null)
        return null;
    }

    FileNode parentNode = getNode(path);
    File newleaf = new File(path + fs + child);
    FileNode childNode = new FileNode(newleaf);

    try
    {
      if(parentNode.isExplored()) 
      {
        int index = getAnIndex(parentNode,child);
        if(index > -1)     
          model.insertNodeInto(childNode, parentNode, index);
      }
    }
    catch(NullPointerException npe)
    {
      parentNode = node;
      if(node.isLeaf())
        parentNode = (FileNode)node.getParent();
      parentNode.explore();
      model.nodeChanged(parentNode);
      model.nodeStructureChanged(parentNode);
    }

    // Make sure the user can see the new node.
    this.scrollPathToVisible(new TreePath(childNode.getPath()));

    return childNode;
  }

/**
*
* Gets the node from the existing explored nodes.
* @param String path to a file or directory
* @return FileNode node the corresponding node if the 
*         directory or file is visible otherwise returns null.
*
*/
  private FileNode getNode(String path)
  {
    Enumeration en = openNode.elements();

    while(en.hasMoreElements()) 
    {
      FileNode node = (FileNode)en.nextElement();
      String nodeName = node.getFile().getAbsolutePath();
      if(nodeName.equals(path))
        return node;
    }
    return null;
  }

/**
*
* Finds a new index for adding a new file to the file manager.
* @param FileNode parentNode directory node
* @param String child
* @return int childIndex the index of the child in the directory
*
*/
  private int getAnIndex(FileNode parentNode, String child)
  {
    //find the index for the child
    int num = parentNode.getChildCount();
    int childIndex = num;
    for(int i=0;i<num;i++)
    {
      String nodeName = 
            ((FileNode)parentNode.getChildAt(i)).getFile().getName();
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
    return childIndex;
  }

/**
*
* Delete a node from the JTree
* @param FileNode node for deletion
*
*/
  public void deleteObject(FileNode node)
  {
    DefaultTreeModel model =(DefaultTreeModel)getModel();
    FileNode parentNode = getNode(node.getFile().getAbsolutePath());
    model.removeNodeFromParent(node);
    model.nodeStructureChanged(parentNode);
  }

/**
*
* Opens a JFrame with the file contents displayed.
* @param the file name
*
*/
  public static void showFilePane(String filename, JembossParams mysettings)
  {
    JFrame ffile = new JFrame(filename);
    JPanel pfile = (JPanel)ffile.getContentPane();
    pfile.setLayout(new BorderLayout());

    FileEditorDisplay fed = new FileEditorDisplay(ffile, filename);
    new ResultsMenuBar(ffile,fed,mysettings);

    JScrollPane rscroll = new JScrollPane(fed);
    pfile.add(rscroll, BorderLayout.CENTER);
    fed.setCaretPosition(0);
    ffile.setSize(450,400);
    ffile.setVisible(true);
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


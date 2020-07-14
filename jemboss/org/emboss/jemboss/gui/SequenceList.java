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

package org.emboss.jemboss.gui;

import java.awt.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.table.*;
import javax.swing.border.*;
import java.awt.event.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.awt.datatransfer.*;
import java.awt.dnd.*;

import org.emboss.jemboss.parser.*;
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.filetree.*;

/**
*
* SequenceList extends JFrame to display a set of sequences
* that the user is working on in a session. The sequence list
* can be stored and read back in by the application. This allows
* the user to usefully maintain a list of sequence that are
* being worked on and easily drag and drop them into sequence
* fields of applications.
*
*/
public class SequenceList extends JFrame
{

  /** drag and drop table containing the list of sequences */
  private DragJTable table;
  /** model for the sequence table */
  private SequenceListTableModel seqModel;
  /** select to save the sequence list between sessions */
  private JCheckBoxMenuItem storeSeqList;

  final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

  /**
  *
  * @param withSoap	true if in client-server mode
  * @param mysettings	jemboss properties
  *
  */
  public SequenceList(final boolean withSoap,final JembossParams mysettings)
  {
    super("Sequence List");
    setSize(400,155);
    seqModel = new SequenceListTableModel();
    table = new DragJTable(seqModel);
    table.setModel(seqModel);
    table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

    //column width
    for(int i=0;i<SequenceListTableModel.modelColumns.length;i++)
    {
      TableColumn column = table.getColumn(
            SequenceListTableModel.modelColumns[i].title);
      column.setPreferredWidth(
             SequenceListTableModel.modelColumns[i].width);
    }

    JScrollPane scrollpane = new JScrollPane(table);
    scrollpane.setSize(300,100);
    getContentPane().add(scrollpane, BorderLayout.CENTER);

//setup menu bar 
    JMenuBar menuPanel = new JMenuBar();
    new BoxLayout(menuPanel,BoxLayout.X_AXIS);
    setJMenuBar(menuPanel);

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuPanel.add(fileMenu);
    JMenu toolMenu = new JMenu("Tools");
    toolMenu.setMnemonic(KeyEvent.VK_T);
    menuPanel.add(toolMenu);

    JMenuItem openMenuItem = new JMenuItem("Open");
    openMenuItem.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        int nrow = table.getSelectedRow();
        String fileName = (String)table.getValueAt(nrow,
                table.convertColumnIndexToView(SequenceListTableModel.COL_NAME));

        SequenceData row = (SequenceData)seqModel.modelVector.elementAt(nrow);

        if(!(row.s_remote.booleanValue()))
          DragTree.showFilePane(fileName,mysettings);        //local file
        else
          RemoteDragTree.showFilePane(fileName,mysettings);  //remote file
      }
    });
    fileMenu.add(openMenuItem);

    JMenuItem ajaxSeq = new JMenuItem("Calculate Sequence Attributes");
    ajaxSeq.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {

        int row = table.getSelectedRow();
        String fn = table.getFileName(row);

        if(!fn.equals("") && fn!=null)
        {
          setCursor(cbusy);
          if(table.isListFile(row).booleanValue())
            fn = "list::"+fn;
        
          String fc = AjaxUtil.getFileOrDatabaseForAjax(fn,
                      BuildProgramMenu.getDatabaseList(),
                      null,withSoap);
        
          boolean ok = false;
          int ajaxLength=0;
          float ajaxWeight;
          boolean ajaxProtein;

          if(!withSoap && fc!=null)    //Ajax without SOAP
          {
            Ajax aj = new Ajax();
            ok = aj.seqType(fc);
            if(ok)
            {
              ajaxLength  = aj.length;
              ajaxWeight  = aj.weight;
              ajaxProtein = aj.protein;
            }
          }
          else if(fc!=null)    //Ajax with SOAP
          {
            try
            {  
              CallAjax ca = new CallAjax(fc,"sequence",mysettings);
              if(ca.getStatus().equals("0"))
              {
                ajaxLength  = ca.getLength();
                ajaxWeight  = ca.getWeight();
                ajaxProtein = ca.isProtein();
                ok = true;
              }
            }
            catch (JembossSoapException eae)
            {
              System.out.println("Call to Ajax library failed");
              setCursor(cdone);
            }
          }

          if(!ok && fc!=null)                          //Ajax failed
          {
            if( mysettings.getServicePasswdByte()!=null ||
                mysettings.getUseAuth() == false )
            {
              JOptionPane.showMessageDialog(null,
                     "Sequence not found!", "Error Message",
                     JOptionPane.ERROR_MESSAGE);
            }
            else 
            {
              AuthPopup ap = new AuthPopup(mysettings,null);
              ap.setBottomPanel();
              ap.setSize(380,170);
              ap.pack();
              ap.setVisible(true);
            }
          }
          else
          {
            seqModel.setValueAt(new Integer(1),row,
                      SequenceListTableModel.COL_BEG);
            seqModel.setValueAt(new Integer(ajaxLength),row,
                      SequenceListTableModel.COL_END);
            table.repaint();
          }
          setCursor(cdone);
        }
      }
    });
    toolMenu.add(ajaxSeq);


    JMenuItem addSeq = new JMenuItem("Add Sequence");
    addSeq.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        int row = table.getSelectedRow();
        seqModel.insertRow(row+1);
        table.tableChanged(new TableModelEvent(seqModel, row+1, row+1, 
                TableModelEvent.ALL_COLUMNS, TableModelEvent.INSERT));
      }
    });
    toolMenu.add(addSeq);

    JMenuItem deleteSeq = new JMenuItem("Delete Sequence");
    deleteSeq.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        int row = table.getSelectedRow();
        if(seqModel.deleteRow(row))
          table.tableChanged(new TableModelEvent(seqModel, row, row,
                TableModelEvent.ALL_COLUMNS, TableModelEvent.DELETE));
        
      }
    });
    toolMenu.add(deleteSeq);
 

    JMenuItem reset = new JMenuItem("Remove All Sequences");
    reset.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        seqModel.setDefaultData();
        table.repaint();
      }
    });
    toolMenu.add(reset);

    toolMenu.addSeparator();

    storeSeqList = new JCheckBoxMenuItem("Save Sequence List");
    File fseq = new File(System.getProperty("user.home")
                           + System.getProperty("file.separator")
                           + ".jembossSeqList");
    if(fseq.canRead())
      storeSeqList.setSelected(true);
    else
      storeSeqList.setSelected(false);
    toolMenu.add(storeSeqList);

    fileMenu.addSeparator();

    JMenuItem closeFrame = new JMenuItem("Close");
    closeFrame.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    closeFrame.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(closeFrame);

    // help menu
   JMenu helpMenu = new JMenu("Help");
   helpMenu.setMnemonic(KeyEvent.VK_H);
   JMenuItem fmh = new JMenuItem("About Sequence List");
   fmh.addActionListener(new ActionListener()
   {
     public void actionPerformed(ActionEvent e)
     {
        ClassLoader cl = this.getClass().getClassLoader();
        try
        {
          URL inURL = cl.getResource("resources/seqList.html");
          new Browser(inURL,"resources/seqList.html");
        }
        catch (MalformedURLException mex)
        {
          System.out.println("Didn't find resources/seqList.html");
        }
        catch (IOException iex)
        {
          System.out.println("Didn't find resources/seqList.html");
        }
      }
    });
    helpMenu.add(fmh);
    menuPanel.add(helpMenu);
  }

  /**
  *
  * Look for first set to default in the table.
  * @return 	index to row
  *
  */
  private int getDefaultRow()
  {
    int nrow = seqModel.getRowCount();
    for(int i =0;i<nrow;i++)
    {
      Boolean isDef = (Boolean)seqModel.getValueAt(i,
                        SequenceListTableModel.COL_DEF);
      if(isDef.booleanValue())
        return i;
    }
    return -1;
  }

  /**
  *
  * Get the default Sequence name
  * @return 	default sequence name or null if no default 
  *
  */
  public String getDefaultSequenceName()
  {
    int ndef = getDefaultRow();
    if(ndef<0)
      return null;

    String seqName = (String)seqModel.getValueAt(ndef,
             SequenceListTableModel.COL_NAME);

    if(table.isListFile(ndef).booleanValue())
      seqName = "@".concat(seqName);

    return seqName;
  }

  /**
  *
  * Return the number of rows in the table
  * @return 	number of rows in the table
  *
  */
  public int getRowCount()
  {
    return seqModel.getRowCount();
  }

  /**
  *
  * The <code>SequenceData</code> for a given row
  * number.
  * @param row 	number
  * @return 	<code>SequenceData</code> for the row
  *
  */
  public SequenceData getSequenceData(int nrow)
  {
    return seqModel.getSequenceData(nrow);
  }

  /**
  *
  * Determines if a sequence list has been stored
  * ("~/.jembossSeqList")
  * @return 	true if a sequence list has been stored
  *
  */
  public boolean isStoreSequenceList()
  {
    return storeSeqList.isSelected();
  }

}


/**
*
* Extend JTable to implement a drag and drop table for
* storing sequence lists (SequenceList)
*
*/
class DragJTable extends JTable implements DragGestureListener,
             DragSourceListener, DropTargetListener
{

  /** model for the sequence table */
  private SequenceListTableModel seqModel;

  /**
  *
  * @param seqModel	model for the sequence table
  *
  */
  public DragJTable(SequenceListTableModel seqModel)
  {
    super();
    this.seqModel = seqModel;
    DragSource dragSource = DragSource.getDefaultDragSource();
    dragSource.createDefaultDragGestureRecognizer(
           this,                              // component where drag originates
           DnDConstants.ACTION_COPY_OR_MOVE,  // actions
           this);
    setDropTarget(new DropTarget(this,this));
  }

// drag source
  public void dragGestureRecognized(DragGestureEvent e)
  {
    Point p = e.getDragOrigin();
    int ncol = columnAtPoint(p);

    if(ncol == convertColumnIndexToView(SequenceListTableModel.COL_NAME))
    {
      int nrow = getSelectedRow();
      e.startDrag(DragSource.DefaultCopyDrop,             // cursor
            (Transferable)seqModel.getSequenceData(nrow), // transferable data
                  this);                                  // drag source listener
    }
  }
  public void dragDropEnd(DragSourceDropEvent e) {}
  public void dragEnter(DragSourceDragEvent e) {}
  public void dragExit(DragSourceEvent e) {}
  public void dragOver(DragSourceDragEvent e) {}
  public void dropActionChanged(DragSourceDragEvent e) {}

  public String getFileName(int row)
  {
    return (String)seqModel.getValueAt(row,
                  SequenceListTableModel.COL_NAME);
  }

  /**
  *
  * Determine if a row in the table contains a list file
  * @param row	row in the table
  * @return 	true if row contains a list file
  *
  */
  public Boolean isListFile(int row)
  {
    return (Boolean)seqModel.getValueAt(row,
                  SequenceListTableModel.COL_LIST);
  }


// drop sink
  public void dragEnter(DropTargetDragEvent e)
  {
    if(e.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE) ||
       e.isDataFlavorSupported(FileNode.FILENODE) ||
       e.isDataFlavorSupported(DataFlavor.stringFlavor) ||
       e.isDataFlavorSupported(SequenceData.SEQUENCEDATA) )
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
  }

  public void drop(DropTargetDropEvent e)
  {
    Transferable t = e.getTransferable();

    if(t.isDataFlavorSupported(FileNode.FILENODE))
    {
      try
      {
        final FileNode fn =
           (FileNode)t.getTransferData(FileNode.FILENODE);
        insertData(seqModel,e.getLocation(),fn.getFile().getCanonicalPath(),
                   "","",new Boolean(false),new Boolean(false),
                   new Boolean(false));
        e.getDropTargetContext().dropComplete(true);
      }
      catch(UnsupportedFlavorException ufe){}
      catch(IOException ioe){}
    }
    else if(t.isDataFlavorSupported(RemoteFileNode.REMOTEFILENODE))
    {
      try
      {
        String dropS = (String)t.getTransferData(DataFlavor.stringFlavor);
        insertData(seqModel,e.getLocation(),dropS,
                   "","",new Boolean(false),new Boolean(false),
                   new Boolean(true));
        e.getDropTargetContext().dropComplete(true);
      }
      catch (Exception exp)
      {
        e.rejectDrop();
      }
    }
    else if(t.isDataFlavorSupported(SequenceData.SEQUENCEDATA))
    {
      try
      {
        SequenceData seqData = (SequenceData)
           t.getTransferData(SequenceData.SEQUENCEDATA);

        String seqName = seqData.s_name;

        insertData(seqModel,e.getLocation(),seqData.s_name,
                   seqData.s_beg,seqData.s_end,
                   seqData.s_listFile,seqData.s_default,
                   seqData.s_remote);
      }
      catch (UnsupportedFlavorException ufe){}
      catch (IOException ioe){}
    }

  }

  /**
  *
  * Inser sequence data into the sequence table
  * @param seqModel 	model for the sequence table 
  * @param ploc		point location to insert
  * @param fileName	name of file
  * @param sbeg		start of sequence
  * @param send		end of sequence
  * @param lis		true if a list file
  * @param def		true if the default sequence
  * @param bremote	true if a sequence on the server
  *
  */
  public void insertData(SequenceListTableModel seqModel, Point ploc,
                         String fileName, String sbeg, String send,
                         Boolean lis, Boolean def, Boolean bremote)
  {
    int row = rowAtPoint(ploc);
    seqModel.modelVector.insertElementAt(new SequenceData(fileName,
                                   sbeg,send,lis,def,bremote),row);

    tableChanged(new TableModelEvent(seqModel, row+1, row+1,
            TableModelEvent.ALL_COLUMNS, TableModelEvent.INSERT));
  }

  public void dragOver(DropTargetDragEvent e)
  { 
    Point ploc = e.getLocation();
    int row = rowAtPoint(ploc);
    setRowSelectionInterval(row,row);
  }
  public void dropActionChanged(DropTargetDragEvent e) {}
  public void dragExit(DropTargetEvent e){}


}

/**
*
* Content of each column in the DragJTable
*
*/
class ColumnData
{
  protected String title;
  protected int width;
  protected int alignment;
  public ColumnData(String title, int width, int alignment)
  {
    this.title = title;
    this.width = width;
    this.alignment = alignment;
  }
}

/**
*
* Model for the sequence table 
*
*/
class SequenceListTableModel extends AbstractTableModel
{

  protected static Vector modelVector;
  public static final int COL_NAME = 0;
  public static final int COL_BEG  = 1;
  public static final int COL_END  = 2;
  public static final int COL_LIST = 3;
  public static final int COL_DEF  = 4;

  public SequenceListTableModel()
  {
    modelVector = new Vector();
    File fseq = new File(System.getProperty("user.home")
                  + System.getProperty("file.separator")
                  + ".jembossSeqList");

    setDefaultData();

    if(fseq.canRead())
      loadStoredSeqList(fseq);
  }

  /**
  *
  * Define the columns as file/start/end/list and default
  *
  */
  public static final ColumnData modelColumns[] =
  {
    new ColumnData("File / Database Entry",170,JLabel.LEFT),
    new ColumnData("Start",45,JLabel.LEFT),
    new ColumnData("End",45,JLabel.LEFT),
    new ColumnData("List File",15,JLabel.LEFT),
    new ColumnData("Default",15,JLabel.LEFT)
  };

  /**
  *
  * DragJTable uses this method to determine the default renderer/
  * editor for each cell.  If we didn't implement this method,
  * then the last column would contain text ("true"/"false"),
  * rather than a check box.
  * @param c	column index
  * @return 	class represented in that column
  *
  */
  public Class getColumnClass(int c) 
  {
    return getValueAt(0, c).getClass();
  }

  /**
  *
  * Load from stored file the SequenceList created from
  * a previous session.
  * @param fseq 	contains stored sequence list
  *
  */
  protected void loadStoredSeqList(File fseq)
  {
    try
    {
      BufferedReader in = new BufferedReader(new FileReader(fseq));
      String line;
      int nrow = 0;

      while((line = in.readLine()) != null)
      {
        if(!line.equals(""))
        {
          line = line.trim();
          StringTokenizer st = new StringTokenizer(line, " ");
          for(int i=0;i<getColumnCount();i++)
          {
            if(!st.hasMoreTokens())
              break;

            if(nrow >= getRowCount())
            {
              Boolean bdef = new Boolean(false);
              modelVector.addElement(new SequenceData("","","",bdef,bdef,bdef));
            }
             
            String value = st.nextToken();
            if(value.equals("-"))
              value = "";
            if(value.equalsIgnoreCase("true") || value.equalsIgnoreCase("false"))
              setValueAt(new Boolean(value),nrow,i);
            else 
              setValueAt(value,nrow,i);
          }
          nrow++;
        }
      } 
    }
    catch (ArrayIndexOutOfBoundsException ai)
    {
      System.out.println("ArrayIndexOutOfBoundsException in SequenceList");
      setDefaultData();
    }
    catch (IOException ioe)
    {
      setDefaultData();
    }

  }

  /**
  *
  * Setup a blank square table 
  *
  */
  protected void setDefaultData()
  {
    modelVector.removeAllElements();
    Boolean bdef = new Boolean(false);
    for(int i=0;i<getColumnCount();i++)
      modelVector.addElement(new SequenceData("","","",bdef,bdef,bdef)); 
    
  }
  

  /**
  *
  * The <code>SequenceData</code> for a given row
  * number.
  * @param row 	number
  * @return 	<code>SequenceData</code> for the row
  *
  */
  protected SequenceData getSequenceData(int nrow)
  {
    return (SequenceData)modelVector.get(nrow);
  }

  /**
  *
  * Return the number of rows in the table
  * @return 	number of rows in the table
  *
  */
  public int getRowCount()
  {
    return modelVector==null ? 0 : modelVector.size();
  }

  /**
  *
  * Return the number of columns in the table
  * @return 	number of columns in the table
  *
  */
  public int getColumnCount()
  {
    return modelColumns.length;
  }

  /**
  *
  * Return the name columns in the table
  * @param c	column index
  * @return 	name columns in the table
  *
  */
  public String getColumnName(int c)
  {
    return modelColumns[c].title;
  }

  /**
  *
  * Define if a cell is editable by the user
  * @param nRow	row number
  * @param nCol	column number
  * @return 	true if editable
  *
  */
  public boolean isCellEditable(int nRow, int nCol)
  {
    return true;
  }


  /**
  *
  * Get the Object in a cell in the table
  * @param nRow	row number
  * @param nCol	column number
  * @return 	value of a cell in the table
  *
  */
  public Object getValueAt(int nRow, int nCol)
  {
    if(nRow < 0 || nCol>=getRowCount())
      return "";

    SequenceData row = (SequenceData)modelVector.elementAt(nRow);
    switch(nCol)
    { 
      case COL_NAME: return row.s_name;
      case COL_BEG: return row.s_beg;
      case COL_END: return row.s_end;
      case COL_DEF:  return row.s_default;
      case COL_LIST:  return row.s_listFile;
    }
    return ""; 
  }

  /**
  *
  * Set the Object in a cell in the table
  * @param value to set
  * @param row number
  * @param column number
  *
  */
  public void setValueAt(Object value, int nRow, int nCol)
  {
    if(nRow < 0 || nCol>=getRowCount())
      return ;

    SequenceData row = (SequenceData)modelVector.elementAt(nRow);
    String svalue = value.toString();
    
    switch(nCol)
    {
      case COL_NAME: 
        row.s_name = svalue;
        break;
      case COL_BEG: 
        row.s_beg = svalue;
        break;
      case COL_END:
        row.s_end = svalue;
        break;
      case COL_DEF:  
        row.s_default = (Boolean)value;
        break;
      case COL_LIST:
        row.s_listFile = (Boolean)value;
        break;
    }
  }

  /**
  *
  * Insert a blank row
  * @param row index to insert at
  *
  */
  public void insertRow(int row)
  {
    if(row < 0)
      row = 0;
    if(row > modelVector.size())
      row = modelVector.size();
    modelVector.insertElementAt(new SequenceData(), row);
  }
  

  /**
  *
  * Delete a row from the table
  * @param row 	number to delete
  * @return 	true if deleted
  *
  */
  public boolean deleteRow(int row)
  {
    if(row < 0 || row>=modelVector.size())
      return false;
    modelVector.remove(row);

//add in blank row so there is always at least
//same number of rows as there is columns
    if(getRowCount() < getColumnCount())
    {
      Boolean bdef = new Boolean(false);
      modelVector.addElement(new SequenceData("","","",bdef,bdef,bdef));
    }

    return true;
  }

}



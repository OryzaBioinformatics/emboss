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

package org.emboss.jemboss.gui.form;

import org.emboss.jemboss.gui.SequenceData;

import java.awt.datatransfer.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.awt.dnd.*;
import java.awt.*;
import java.io.*;


/**
*
* Extends JTextField to enable pasting & drag and drop into it.
* @author T. J. Carver
*
*/
public class TextFieldSink extends JTextField implements DropTargetListener
{

  public TextFieldSink() 
  {

//pasting not really needed?
//    addMouseListener(new MouseAdapter() 
//    {
//      public void mouseClicked(MouseEvent e) 
//      {
//        if(e.getClickCount() == 2) 
//        {
//          pasteText();
//          e.consume();
//        };
//      }
//    });
    
    setDropTarget(new DropTarget(this,this));

  }

  public void pasteText() 
  {
    Clipboard c = this.getToolkit().getSystemClipboard();
    
    Transferable t = c.getContents(this);
    if(t==null) 
    {
      this.getToolkit().beep();
      return;
    } 
    try
    {
      if(t.isDataFlavorSupported(DataFlavor.stringFlavor))
      {
        String s = (String) t.getTransferData(DataFlavor.stringFlavor);
        this.replaceSelection(s);
      } 
      else
        this.getToolkit().beep();
    }
    catch (UnsupportedFlavorException ex) { this.getToolkit().beep(); }
    catch (IOException ex) { this.getToolkit().beep(); }

  }

  protected static Border dropBorder = new BevelBorder(BevelBorder.LOWERED);
  protected static Border endBorder = 
                               BorderFactory.createLineBorder(Color.black);

  public void dragEnter(DropTargetDragEvent e) 
  {
    if(e.isDataFlavorSupported(DataFlavor.stringFlavor) ||
       e.isDataFlavorSupported(SequenceData.SEQUENCEDATA) ) 
    {
      e.acceptDrag(DnDConstants.ACTION_COPY_OR_MOVE);
      this.setBorder(dropBorder);
    }
  }

  public void dragExit(DropTargetEvent e) 
  { 
    this.setBorder(endBorder); 
  }

  public void drop(DropTargetDropEvent e)
  {
    this.setBorder(endBorder);
    Transferable t = e.getTransferable();
    if(t.isDataFlavorSupported(DataFlavor.stringFlavor))
    {
      e.acceptDrop(DnDConstants.ACTION_COPY_OR_MOVE);
      try 
      {
        String dropS = (String) t.getTransferData(DataFlavor.stringFlavor);
        this.replaceSelection(dropS);
        e.dropComplete(true);
      } 
      catch (Exception ex) {}
        
    } 
    else if(t.isDataFlavorSupported(SequenceData.SEQUENCEDATA))
    {
      e.acceptDrop(DnDConstants.ACTION_COPY_OR_MOVE);
      try
      {
        SequenceData seqData = (SequenceData)
             t.getTransferData(SequenceData.SEQUENCEDATA);
        String seq = seqData.s_name;
        if(seqData.s_listFile.booleanValue())
          seq = "@".concat(seq);

        this.replaceSelection(seq);
        e.dropComplete(true);
      }
      catch (Exception ex) {}
    }
    else
    {
      e.rejectDrop();
      return;
    }
    return;
  }

  public void dragOver(DropTargetDragEvent e) {}
  public void dropActionChanged(DropTargetDragEvent e) {}

}


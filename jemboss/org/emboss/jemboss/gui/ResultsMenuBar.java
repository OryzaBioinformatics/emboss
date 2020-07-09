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

import java.util.Hashtable;
import java.util.Enumeration;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import org.emboss.jemboss.gui.sequenceChooser.*;
import org.emboss.jemboss.gui.filetree.*;
import org.emboss.jemboss.gui.AdvancedOptions;

import java.awt.event.*;
import java.io.*;


/**
*
* Sets up a results menu bar with save and close
* 
*
*/
public class ResultsMenuBar
{

  final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  private String fs = new String(System.getProperty("file.separator"));
  private JMenuItem fileMenuShowres;
  private JMenuBar menuPanel;

/**
*
* Sets up a results menu bar with save and close
* @param JFrame frame containing the results
*
*/
  public ResultsMenuBar(final JFrame frame)
  {

    menuPanel = new JMenuBar();
    menuPanel.setLayout(new FlowLayout(FlowLayout.LEFT,10,5));
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    fileMenuShowres = new JMenuItem("Save...");

    fileMenu.add(fileMenuShowres);

    JMenuItem resFileMenuExit = new JMenuItem("Close");
    resFileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        frame.setVisible(false);
      }
    });
    fileMenu.add(resFileMenuExit);
    menuPanel.add(fileMenu);

    frame.setJMenuBar(menuPanel);
  }


/**
*
* Adds action listener to save contents of a JTextPane. This
* allows editing of the area to be saved.
* @param JFrame frame containing the results
* @param JTextPane text area to add listener to
*
*/
  public ResultsMenuBar(final JFrame frame, final FileEditorDisplay fed)
  {
    this(frame);

    final JTextPane seqText = fed.getJTextPane();

    fileMenuShowres.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new FileSaving(seqText, fed.getPNGContent());
      }
    });

   
    ButtonGroup group = new ButtonGroup();
    JMenu optionsMenu   = new JMenu("Options");
    JRadioButtonMenuItem optionsMenuText = new JRadioButtonMenuItem("Text");
    optionsMenu.add(optionsMenuText);
    optionsMenuText.setSelected(true);
    group.add(optionsMenuText);
    JRadioButtonMenuItem optionsMenuSeq = new JRadioButtonMenuItem("Sequence");
    optionsMenu.add(optionsMenuSeq);
    group.add(optionsMenuSeq);

    menuPanel.add(optionsMenu);

    optionsMenuSeq.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(((JRadioButtonMenuItem)e.getSource()).isSelected())
        {
          String text = seqText.getText();
          seqText.setText("");
          fed.setText(text,"sequence",seqText);
          seqText.setCaretPosition(0);
        }
      }
    });

   optionsMenuText.addActionListener(new ActionListener()
   {
     public void actionPerformed(ActionEvent e)
     {
       if(((JRadioButtonMenuItem)e.getSource()).isSelected())
       {
         String text = seqText.getText();
         seqText.setText("");
         fed.setText(text,"regular",seqText);
         seqText.setCaretPosition(0);
       }
     }
   });


 }


/**
*
* Adds action listener to save contents of contents of a
* tabbed pane. Allows saving to files of text and png files.
* @param JFrame frame containing the results
* @param JTabbedPane tab pane containing results
* @param Hashtable containing results
*
*/
  public ResultsMenuBar(final JFrame frame, final JTabbedPane rtb,
                        final Hashtable hash)
  {

    this(frame);
    
    fileMenuShowres.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {

        String fileSelected = "";
        String cwd = "";

        SecurityManager sm = System.getSecurityManager();
        System.setSecurityManager(null);
        JFileChooser fc = new JFileChooser(AdvancedOptions.cwd);
        System.setSecurityManager(sm);

        fc.addChoosableFileFilter(new SequenceFilter());
        int returnVal = fc.showSaveDialog(fc);

        if (returnVal == JFileChooser.APPROVE_OPTION)
        {
          File files = fc.getSelectedFile();
          cwd = (fc.getCurrentDirectory()).getAbsolutePath();
          fileSelected = files.getName();

          frame.setCursor(cbusy);
//        save results
          String tabTitle = rtb.getTitleAt(rtb.getSelectedIndex());
          Enumeration enum = hash.keys();

          while(enum.hasMoreElements())
          {
            String thiskey = (String)enum.nextElement();
            if(tabTitle.equals(thiskey))
            {
              FileSave fsave = new FileSave(new File(cwd + fs + fileSelected));
              if(fsave.doWrite())
                fsave.fileSaving(hash.get(thiskey));
              if(!fsave.fileExists())
                org.emboss.jemboss.Jemboss.tree.addObject(fileSelected,cwd);
            }
          }

          frame.setCursor(cdone);
        }
      }
    });
  }


  public JMenuBar getJMenuBar()
  {
    return menuPanel;
  }
  
}


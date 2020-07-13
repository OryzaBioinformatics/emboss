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
import java.util.Vector;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.io.File;
import javax.swing.border.*;
import javax.swing.undo.UndoManager;
import javax.swing.undo.CannotRedoException;
import javax.swing.text.JTextComponent;
import org.apache.soap.rpc.Parameter;

import org.emboss.jemboss.soap.PrivateRequest;
import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.gui.sequenceChooser.SequenceFilter;
import org.emboss.jemboss.gui.filetree.*;
import org.emboss.jemboss.gui.AdvancedOptions;
import org.emboss.jemboss.JembossParams;

/**
*
* Sets up a results menu bar with save and close
* 
*
*/
public class ResultsMenuBar extends JMenuBar
{

  private JMenuItem fileMenuShowres;
  private JMenuItem saveToRemoteFile;
  private JFrame frame;
  private JToolBar toolBar = new JToolBar();
  private JMenuItem undo = new JMenuItem("Undo");
  private JMenuItem redo = new JMenuItem("Redo");
  private UndoManager undoManager = new UndoManager();


  public ResultsMenuBar(final JFrame frame, final JTabbedPane rtb,
                        final Hashtable hashOut, final Hashtable hashIn)
  {
    this(frame,rtb,hashOut,hashIn,null,null);
  }


/**
*
* Sets up a results menu bar with save and close
* @param JFrame frame containing the results
*
*/
  public void setResultsMenuBar(final JFrame frame, boolean addRemoteSaveMenu)
  {
    this.frame = frame;
    add(Box.createRigidArea(new Dimension(5,24)));

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

    if(addRemoteSaveMenu)
    {
      saveToRemoteFile = new JMenuItem("Save to Server File");
      fileMenu.add(saveToRemoteFile);
    }
    fileMenuShowres = new JMenuItem("Save to Local File...");
    fileMenu.add(fileMenuShowres);
    fileMenu.addSeparator();

    // undo - redo
    fileMenu.add(undo);
    undo.setEnabled(false);
    undo.setAccelerator(KeyStroke.getKeyStroke(
                KeyEvent.VK_U, ActionEvent.CTRL_MASK));
    fileMenu.add(redo);
    redo.setEnabled(false);
    redo.setAccelerator(KeyStroke.getKeyStroke(
                KeyEvent.VK_R, ActionEvent.CTRL_MASK));
    fileMenu.addSeparator();

    // close
    JMenuItem resFileMenuExit = new JMenuItem("Close");
    resFileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
                    KeyEvent.VK_E, ActionEvent.CTRL_MASK));
    
    resFileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        frame.setVisible(false);
      }
    });
    fileMenu.add(resFileMenuExit);
    add(fileMenu);
    
    frame.setJMenuBar(this);
    frame.getContentPane().add(toolBar, BorderLayout.NORTH);
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
    setResultsMenuBar(frame,false);


    fileMenuShowres.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        FileSaving fsave = new FileSaving(fed, fed.getPNGContent());

        if(fsave.writeOK())
        {
          String fileSelected = fsave.getFileName();
          String pathSelected = fsave.getPath();
          try
          {
            org.emboss.jemboss.Jemboss.tree.addObject(fileSelected,pathSelected,null);
          }
          catch(NullPointerException npe){}
          DragTree ltree = LocalAndRemoteFileTreeFrame.getLocalDragTree();
          if(ltree!=null)
            ltree.addObject(fileSelected,pathSelected,null);

        }
      }
    });


    // undo - redo
    fed.getDocument().addUndoableEditListener(new UndoableEditListener() 
    {
      public void undoableEditHappened(UndoableEditEvent e) 
      {
        undoManager.addEdit(e.getEdit());
        updateMenu();
      }
    });

    undo.addActionListener(new ActionListener() 
    {
      public void actionPerformed(ActionEvent e) 
      {
        try { undoManager.undo(); }
        catch (CannotRedoException cre) { cre.printStackTrace(); }
        updateMenu();
      }
    });

    redo.addActionListener(new ActionListener() 
    {
      public void actionPerformed(ActionEvent e) 
      {
        try { undoManager.redo(); }
        catch (CannotRedoException cre) { cre.printStackTrace(); }
        updateMenu();
      }
    });

    //Colour selection
    JMenu colourMenu   = new JMenu("Colour");
    colourMenu.setMnemonic(KeyEvent.VK_L);

    ColorMenu cm = new ColorMenu("Text");
    cm.setColor(fed.getForeground());
    cm.setMnemonic('t');
    ActionListener lst = new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ColorMenu m = (ColorMenu)e.getSource();
        fed.setForeground(m.getColor());
      }
    };
    cm.addActionListener(lst);
    colourMenu.add(cm);

    cm = new ColorMenu("Background");
    cm.setColor(fed.getBackground());
    cm.setMnemonic('b');
    lst = new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ColorMenu m = (ColorMenu)e.getSource();
        fed.setBackground(m.getColor());
      }
    };
    cm.addActionListener(lst);
    colourMenu.add(cm);
    add(colourMenu);

    //Text - sequence display option
    ButtonGroup group = new ButtonGroup();
    JMenu optionsMenu   = new JMenu("Options");
    JRadioButtonMenuItem optionsMenuText = new JRadioButtonMenuItem("Text");
    optionsMenu.add(optionsMenuText);
    optionsMenuText.setSelected(true);
    group.add(optionsMenuText);
    JRadioButtonMenuItem optionsMenuSeq = new JRadioButtonMenuItem("Sequence");
    optionsMenu.add(optionsMenuSeq);
    group.add(optionsMenuSeq);

    add(optionsMenu);

    optionsMenuSeq.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(((JRadioButtonMenuItem)e.getSource()).isSelected())
        {
          String text = fed.getText();
          fed.setText("");
          fed.setText(text,"sequence");
          fed.setCaretPosition(0);
        }
      }
    });

    optionsMenuText.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(((JRadioButtonMenuItem)e.getSource()).isSelected())
        {
          String text = fed.getText();
          fed.setText("");
          fed.setText(text,"regular");
          fed.setCaretPosition(0);
        }
      }
    });
    add(optionsMenu);
 

    //Font size selection
    String sizes[] = {"10", "12", "14", "16", "18"};
    final JComboBox fntSize = new JComboBox(sizes);
    Font fnt = fed.getFont();
    fntSize.setSelectedItem(Integer.toString(fnt.getSize()));
    fntSize.setPreferredSize(fntSize.getMinimumSize());
    fntSize.setMaximumSize(fntSize.getMinimumSize());
    fntSize.setEditable(true);
    toolBar.add(fntSize);
    fntSize.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        Font fnt = fed.getFont();
        try
        {
          String fsize = (String)fntSize.getSelectedItem();
          if(fsize.indexOf(".") > -1)
            fsize = fsize.substring(0,fsize.indexOf("."));
          fnt = new Font(fnt.getFontName(),fnt.getStyle(),
                         Integer.parseInt(fsize));
          fed.setFont(fnt);
        } catch(NumberFormatException nfe){}
      }
    });

    //Font style
    String styles[] = {"Plain","Bold","Italic"};
    final JComboBox cbFonts = new JComboBox(styles);
    cbFonts.setMaximumSize(cbFonts.getPreferredSize());
    cbFonts.setToolTipText("Available styles");
    cbFonts.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        int index = cbFonts.getSelectedIndex();
        if (index < 0)
          return;
        Font fnt = fed.getFont();
        if(index == 0)
          fnt = fnt.deriveFont(Font.PLAIN);
        else if(index == 1)
          fnt = fnt.deriveFont(Font.BOLD);
        else
          fnt = fnt.deriveFont(Font.ITALIC);
        fed.setFont(fnt);
      }
    });
    toolBar.add(cbFonts);

 }


/**
*
* Adds action listener to save contents of contents of a
* tabbed pane. Allows saving to files of text and png files.
* @param JFrame frame containing the results
* @param JTabbedPane tab pane containing results
* @param Hashtable containing results
* @param Hashtable containing input files
*
*/
  public ResultsMenuBar(final JFrame frame, final JTabbedPane rtb,
                        final Hashtable hashOut, final Hashtable hashIn,
                        final String project, final JembossParams mysettings)
  {
    boolean addRemoteSaveMenu = false;
    if(mysettings != null)
      addRemoteSaveMenu = true;

    setResultsMenuBar(frame, addRemoteSaveMenu);

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

          frame.setCursor(new Cursor(Cursor.WAIT_CURSOR));
//        save results
          String tabTitle = rtb.getTitleAt(rtb.getSelectedIndex());

          if(hashOut.containsKey(tabTitle))
            fileSave(cwd,fileSelected,tabTitle,hashOut);
          else if(hashIn != null)
          {
            if(hashIn.containsKey(tabTitle))
              fileSave(cwd,fileSelected,tabTitle,hashIn);
          }
          frame.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
        }
      }
    });


// undo - redo
    for(int i =0; i<rtb.getTabCount(); i++)
    {
      JTextComponent jtc = getJTextComponentAt(rtb,i);
      if(jtc != null)
        jtc.getDocument().addUndoableEditListener(new UndoableEditListener()
        {
          public void undoableEditHappened(UndoableEditEvent e)
          {
            undoManager.addEdit(e.getEdit());
            updateMenu();
          }
        });
    }

    undo.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        try { undoManager.undo(); }
        catch (CannotRedoException cre) { cre.printStackTrace(); }
        updateMenu();
      }
    });

    redo.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        try { undoManager.redo(); }
        catch (CannotRedoException cre) { cre.printStackTrace(); }
        updateMenu();
      }
    });


    //Colour selection
    JMenu colourMenu   = new JMenu("Colour");
    colourMenu.setMnemonic(KeyEvent.VK_L);
    ColorMenu cm = new ColorMenu("Text");
    cm.setMnemonic('t');
    ActionListener lst = new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ColorMenu m = (ColorMenu)e.getSource();
        JTextPane jtp = getSelectedJTextPane(rtb);
        if(jtp != null)
          jtp.setForeground(m.getColor());
      }
    };
    cm.addActionListener(lst);
    colourMenu.add(cm);
 
    cm = new ColorMenu("Background");
    cm.setMnemonic('b');
    lst = new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        ColorMenu m = (ColorMenu)e.getSource();
        JTextPane jtp = getSelectedJTextPane(rtb);
        if(jtp != null)
          jtp.setBackground(m.getColor());
      }
    };
    cm.addActionListener(lst);
    colourMenu.add(cm);
    add(colourMenu);

  
    // saving to the remote file system
    if(addRemoteSaveMenu)
    {
      saveToRemoteFile.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          JTextComponent jtc = getJTextComponentAt(rtb,rtb.getSelectedIndex());      

          Vector params = new Vector();
          params.addElement(new Parameter("project", String.class,
                            project, null));
          params.addElement(new Parameter("filename", String.class,
                            rtb.getTitleAt(rtb.getSelectedIndex()), null));
          params.addElement(new Parameter("filecontent", String.class,
                            jtc.getText(), null));

          try
          {
            PrivateRequest gReq = new PrivateRequest(mysettings,
                                    "save_project_file",params);
          }
          catch(JembossSoapException jse){}

        }
      });
    } 

    //Font size selection
    String sizes[] = {"10", "12", "14", "16", "18"};
    final JComboBox fntSize = new JComboBox(sizes);
    fntSize.setSelectedItem("12");
    fntSize.setPreferredSize(fntSize.getMinimumSize());
    fntSize.setMaximumSize(fntSize.getMinimumSize());
    fntSize.setEditable(true);
    toolBar.add(fntSize);
    fntSize.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JTextPane jtp = getSelectedJTextPane(rtb);
        if(jtp != null)
        {
          Font fnt = jtp.getFont();
          try
          {
            String fsize = (String)fntSize.getSelectedItem();
            if(fsize.indexOf(".") > -1)
              fsize = fsize.substring(0,fsize.indexOf("."));
            fnt = new Font(fnt.getFontName(),fnt.getStyle(),
                           Integer.parseInt(fsize));
            jtp.setFont(fnt);
          } catch(NumberFormatException nfe){}
        }
      }
    });

    //Font style
    String styles[] = {"Plain","Bold","Italic"};
    final JComboBox cbFonts = new JComboBox(styles);
    cbFonts.setMaximumSize(cbFonts.getPreferredSize());
    cbFonts.setToolTipText("Available styles");
    cbFonts.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        int index = cbFonts.getSelectedIndex();
        if (index < 0)
          return;
        JTextPane jtp = getSelectedJTextPane(rtb);
        if(jtp == null)
          return;

        Font fnt = jtp.getFont();
        if(index == 0)
          fnt = fnt.deriveFont(Font.PLAIN);
        else if(index == 1)
          fnt = fnt.deriveFont(Font.BOLD);
        else
          fnt = fnt.deriveFont(Font.ITALIC);
        jtp.setFont(fnt);
      }
    });
    toolBar.add(cbFonts);

  }


/**
*
* Update the undo and redo menus and enable or disable
* dependent on the editing event that proceeded.
*
*/
  private void updateMenu()
  {
    undo.setText(undoManager.getUndoPresentationName());
    redo.setText(undoManager.getRedoPresentationName());
    undo.setEnabled(undoManager.canUndo());
    redo.setEnabled(undoManager.canRedo());
  }

  private void fileSave(String cwd, String fileSelected, 
                        String tabTitle, Hashtable h)
  {
    String fs = new String(System.getProperty("file.separator"));
    FileSave fsave = new FileSave(new File(cwd + fs + fileSelected));
    if(fsave.doWrite())
      fsave.fileSaving(h.get(tabTitle));
    if(!fsave.fileExists())
    {
      org.emboss.jemboss.Jemboss.tree.addObject(fileSelected,cwd,null);
      DragTree ltree = SetUpMenuBar.getLocalDragTree();
      if(ltree!=null)
        ltree.addObject(fileSelected,cwd,null);
    }
  }

  public JMenuBar getJMenuBar()
  {
    return this;
  }

  private JTextPane getSelectedJTextPane(JTabbedPane rtb)
  {
    JScrollPane jsp = (JScrollPane)(rtb.getSelectedComponent());
    JPanel jp = (JPanel)(jsp.getViewport().getView());
    try
    {
      return (JTextPane)jp.getComponent(0);
    }
    catch(ClassCastException cce){}
    return null;
  }

  private JTextComponent getJTextComponentAt(JTabbedPane rtb, int index)
  {
    JPanel jp;
    try
    {
      JScrollPane jsp = (JScrollPane)(rtb.getComponentAt(index));
      jp = (JPanel)(jsp.getViewport().getView());

    }
    catch(ClassCastException cce)
    {
      jp = (JPanel)(rtb.getComponentAt(index));
    }

    try
    {
      return (JTextComponent)jp.getComponent(0);
    }
    catch(ClassCastException cce){}
    return null;
  }
  
}

class ColorMenu extends JMenu
{
  protected Border unselectedBorder;
  protected Border selectedBorder;
  protected Border activeBorder;

  protected Hashtable panes;
  protected ColorPane selected;

  public ColorMenu(String name) 
  {
    super(name);
    unselectedBorder = new CompoundBorder(
      new MatteBorder(1, 1, 1, 1, getBackground()),
      new BevelBorder(BevelBorder.LOWERED, 
      Color.white, Color.gray));
    selectedBorder = new CompoundBorder(
      new MatteBorder(2, 2, 2, 2, Color.red),
      new MatteBorder(1, 1, 1, 1, getBackground()));
    activeBorder = new CompoundBorder(
      new MatteBorder(2, 2, 2, 2, Color.blue),
      new MatteBorder(1, 1, 1, 1, getBackground()));
        
    JPanel p = new JPanel();
    p.setBorder(new EmptyBorder(5, 5, 5, 5));
    p.setLayout(new GridLayout(8, 8));
    panes = new Hashtable();

    int[] values = new int[] { 0, 128, 192, 255 };
    for (int r=0; r<values.length; r++) 
    {
      for (int g=0; g<values.length; g++) 
      {
        for (int b=0; b<values.length; b++)
        {
          Color c = new Color(values[r], values[g], values[b]);
          ColorPane pn = new ColorPane(c);
          p.add(pn);
          panes.put(c, pn);
        }
      }
    }
    add(p);
  }

  public void setColor(Color c) 
  {
    Object obj = panes.get(c);
    if (obj == null)
      return;
    if (selected != null)
      selected.setSelected(false);
    selected = (ColorPane)obj;
    selected.setSelected(true);
  }

  public Color getColor() 
  {
    if (selected == null)
      return null;
    return selected.getColor();
  }

  public void doSelection() 
  {
    fireActionPerformed(new ActionEvent(this, 
      ActionEvent.ACTION_PERFORMED, getActionCommand()));
  }

  class ColorPane extends JPanel implements MouseListener
  {
    protected Color col;
    protected boolean selected;

    public ColorPane(Color c) 
    {
      col = c;
      setBackground(c);
      setBorder(unselectedBorder);
      String msg = "R "+c.getRed()+", G "+c.getGreen()+
        ", B "+c.getBlue();
      setToolTipText(msg);
      addMouseListener(this);
    }

    public Color getColor() { return col; }

    public Dimension getPreferredSize() 
    {
      return new Dimension(15, 15);
    }
    public Dimension getMaximumSize() { return getPreferredSize(); }
    public Dimension getMinimumSize() { return getPreferredSize(); }

    public void setSelected(boolean select) 
    {
      selected = select;
      if (selected)
        setBorder(selectedBorder);
      else
        setBorder(unselectedBorder);
    }

    public boolean isSelected() { return selected; }
    public void mousePressed(MouseEvent e) {}
    public void mouseClicked(MouseEvent e) {}

    public void mouseReleased(MouseEvent e) 
    {
      setColor(col);
      MenuSelectionManager.defaultManager().clearSelectedPath();
      doSelection();
    }

    public void mouseEntered(MouseEvent e) 
    {
      setBorder(activeBorder);
    }

    public void mouseExited(MouseEvent e) 
    {
      setBorder(selected ? selectedBorder : 
        unselectedBorder);
    }
  }
}

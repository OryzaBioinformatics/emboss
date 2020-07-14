/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.editor;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import org.emboss.jemboss.gui.ScrollPanel;

public class ColourJFrame extends JFrame
{
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  private JPopupMenu popup;
  private Box YBox = new Box(BoxLayout.Y_AXIS);
  private Hashtable colourTable;
  private JScrollPane jspColour;
  private AlignJFrame align;

  public ColourJFrame(AlignJFrame align)
  {
    super("Colour");

    this.align = align;
    JPanel mainPane = (JPanel)getContentPane();
    mainPane.setLayout(new BorderLayout());
    
    ScrollPanel mainColourPanel = new ScrollPanel(new BorderLayout());
    mainColourPanel.add(YBox);
    jspColour = new JScrollPane(mainColourPanel);
    mainPane.add(jspColour);
    mainColourPanel.setBackground(Color.white);

// set up a menu bar
    JMenuBar menuBar = new JMenuBar();

// File menu
    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

// exit
    fileMenu.add(new JSeparator());
    JMenuItem fileMenuExit = new JMenuItem("Close");
    fileMenuExit.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(fileMenuExit);
    menuBar.add(fileMenu);
    setJMenuBar(menuBar);
    setSize(70,150);
  }

  public void setCurrentColour(Hashtable colourTable)
  {
    this.colourTable = colourTable;
    Box XBox;
    ColourMenu cm;
    JLabel residueField;
    YBox.removeAll();
    
    Enumeration enum = colourTable.keys();
    while(enum.hasMoreElements())
    {
      String res = (String)enum.nextElement();
      XBox = new Box(BoxLayout.X_AXIS);
      residueField = new JLabel(res);
      residueField.setPreferredSize(new Dimension(20,20));
      residueField.setMaximumSize(new Dimension(20,20));
      XBox.add(residueField);
      ColourPanel colPane = new ColourPanel(res,(Color)colourTable.get(res));
      XBox.add(colPane);
      YBox.add(XBox);
    }
    YBox.add(Box.createVerticalGlue());

    jspColour.getViewport().setViewPosition(new Point(0,0));
  }

  public Hashtable getCurrentColourScheme()
  {
    return colourTable;
  }

  class ColourPanel extends JPanel
                        implements ActionListener
  {
    private Color col;
    private String res;
    private int xsize = 20;
    private int ysize = 20;
    private JPopupMenu popup;

    public ColourPanel(String res,Color col)
    {
      super();
      this.col = col;
      this.res = res;
      setPreferredSize(new Dimension(xsize,ysize));

      ColourMenu cm = new ColourMenu(res+" Colour");
      popup = new JPopupMenu();
      addMouseListener(new PopupListener());
      cm.addActionListener(this);
      popup.add(cm);
    }

    /**
    * Popup menu actions
    */
    public void actionPerformed(ActionEvent e)
    {
      ColourMenu m = (ColourMenu)e.getSource();
      col = m.getColor();
      repaint();
      colourTable.remove(res);
      colourTable.put(res,col);
      align.repaintSequences(colourTable);
    }

    public void paintComponent(Graphics g)
    {
// let UI delegate paint first (incl. background filling)
      super.paintComponent(g);
      g.setColor(col);
      g.fillRect(0,0,xsize,ysize);
      g.setColor(Color.black);
      g.drawRect(0,0,xsize,ysize);
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

}


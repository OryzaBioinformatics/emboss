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

import org.emboss.jemboss.gui.filetree.FileEditorDisplay;

public class MatrixJFrame extends JFrame
{
  private JScrollPane mScroll =  new JScrollPane();
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  private Matrix mat;

  public MatrixJFrame(Matrix m, final JTextField statusField, 
                      final AlignJFrame alignFrame)
  {
    super();
    this.mat = m;

    JPanel mainPane = (JPanel)getContentPane();
    mainPane.setLayout(new BorderLayout());
    mainPane.add(mScroll, BorderLayout.CENTER);
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

//
    final Object matKeys[] = mat.getKeyNames();
    final JList list = new JList(matKeys);
    JScrollPane jspList = new JScrollPane(list);
    JPanel jp = new JPanel(new BorderLayout());
    jp.add(jspList,BorderLayout.CENTER);
    JButton matButt= new JButton("Select");
    matButt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setCursor(cbusy);
        String selMat = (String)list.getSelectedValue();
        mat = new Matrix("resources/resources.jar",
                         selMat);
        statusField.setText("Current matrix: "+selMat);
        setMatrix(mat);
        alignFrame.setMatrix(mat);
        setCursor(cdone);
      }
    });
    jp.add(matButt,BorderLayout.SOUTH);
    mainPane.add(jp,BorderLayout.EAST);
    setSize(500,400);
  }

  public void setMatrix(Matrix mat)
  {
    FileEditorDisplay fed = new FileEditorDisplay(this,"Matrix",
                                   mat.getMatrixTable().trim());
    fed.setCaretPosition(0);
    mScroll.setViewportView(fed);
    setTitle(mat.getCurrentMatrixName());
//  pack();
  }
}


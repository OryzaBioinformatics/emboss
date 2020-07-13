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
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import org.emboss.jemboss.gui.form.Separator;


public class AdvancedOptions extends JPanel
{

  public static JCheckBox prefjni;
  public static JCheckBox prefShadeGUI;
  public static JComboBox jobMgr;
  public static JTextField mailServer;
  public static String cwd = System.getProperty("user.home");
  private static JTextField userHome = new JTextField();

  private String time[] = new String[6];

  public AdvancedOptions()
  {
    super();
    time[0] = "5 s";
    time[1] = "10 s";
    time[2] = "15 s";
    time[3] = "20 s";
    time[4] = "30 s";
    time[5] = "60 s";

    Box bdown =  Box.createVerticalBox();
    Box bleft =  Box.createHorizontalBox();

//shade or remove unused parameters
    prefShadeGUI = new JCheckBox("Shade unused parameters");
    prefShadeGUI.setSelected(true);
    bleft.add(prefShadeGUI);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    bdown.add(Box.createVerticalStrut(4));

//use JNI to calculate parameter dependencies
    prefjni = new JCheckBox("Calculate dependencies (JNI)");
    prefjni.setSelected(true);
    bleft =  Box.createHorizontalBox();
    bleft.add(prefjni);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    bdown.add(Box.createVerticalStrut(5));

//frequency of job manager updates
    jobMgr = new JComboBox(time);
    jobMgr.setSelectedIndex(2);
    int hgt = (new Double(jobMgr.getPreferredSize().getHeight())).intValue();
    jobMgr.setPreferredSize(new Dimension(70,hgt));
    jobMgr.setMaximumSize(new Dimension(70,hgt));
    bleft =  Box.createHorizontalBox();
    bleft.add(jobMgr);
    JLabel ljobMgr = new JLabel(" Job Manager update frequency");
    ljobMgr.setForeground(Color.black);
    bleft.add(ljobMgr);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);

    bdown.add(Box.createVerticalStrut(5));
    bdown.add(new Separator(new Dimension(400,10)));
    bdown.add(Box.createVerticalStrut(5));

//set users home root directory
    bleft =  Box.createHorizontalBox();         
    JLabel lhome = new JLabel("Local Home Directory:");
    lhome.setForeground(Color.black);
    bleft.add(lhome);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);

    userHome.setText(cwd);
    bleft =  Box.createHorizontalBox();
    bleft.add(userHome);
    bdown.add(bleft);
    JButton jroot = new JButton("Set");
    bleft =  Box.createHorizontalBox();
    bleft.add(jroot);
    JButton jreset = new JButton("Reset");
    bleft.add(jreset);
    bleft.add(Box.createHorizontalGlue());
    bdown.add(bleft);
    
    jroot.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        File f = new File(userHome.getText());
        if(f.exists() && f.canRead())
        {
          cwd = userHome.getText();
          org.emboss.jemboss.Jemboss.tree.newRoot(cwd);
          if(SetUpMenuBar.localAndRemoteTree != null)
            SetUpMenuBar.localAndRemoteTree.getLocalDragTree().newRoot(cwd);

          if(!f.canWrite())
            JOptionPane.showMessageDialog(null,
                          "You cannot write to this directory.",
                          "Warning: Write",
                          JOptionPane.WARNING_MESSAGE);
        }
        else
          JOptionPane.showMessageDialog(null,
                          "No access to this directory.",
                          "Error: Access",
                          JOptionPane.ERROR_MESSAGE);

      }
    });

//reset button
    jreset.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        cwd = System.getProperty("user.home");
        org.emboss.jemboss.Jemboss.tree.newRoot(cwd);
        if(SetUpMenuBar.localAndRemoteTree != null)
          SetUpMenuBar.localAndRemoteTree.getLocalDragTree().newRoot(cwd);
        userHome.setText(cwd);
      }
    });

    bdown.add(Box.createVerticalStrut(5));
    bdown.add(new Separator(new Dimension(400,10)));
    bdown.add(Box.createVerticalStrut(5));

    this.add(bdown);
  }


  public static String getHomeDirectory()
  {
    return userHome.getText();
  }
}



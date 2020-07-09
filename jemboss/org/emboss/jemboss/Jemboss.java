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

package org.emboss.jemboss;


import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;

import java.awt.event.*;
import java.io.*;

import org.emboss.jemboss.gui.startup.*;    // splash window
import org.emboss.jemboss.gui.filetree.*;   // local files
import uk.ac.mrc.hgmp.embreo.*;             // SOAP settings
import org.emboss.jemboss.gui.*;            // Jemboss graphics
import org.emboss.jemboss.soap.*;           // results manager

/**
*
*  Java interface to EMBOSS (http://www.emboss.org/)
*  (i)  standalone - with a locally installation of EMBOSS.
*  (ii) client / server mode - download the client from a site,
*       such as the HGMP, which runs the Jemboss server.
*
*/
public class Jemboss implements ActionListener
{

// system properties
  private String fs = new String(System.getProperty("file.separator")); 
  private String ps = new String(System.getProperty("path.separator"));

// path names
  private String plplot;
  private String embossData;
  private String embossBin;
  private String embossPath;
  private String acdDirToParse;

  private String cwd = new String(
                       System.getProperty("user.dir") + fs);
  private String homeDirectory = new String(
                       System.getProperty("user.home") + fs);

// Swing components
  private JFrame f;
  private JSplitPane pmain;
  private JSplitPane ptree;

  private JPanel p3;
  public static DragTree tree;
  private JButton extend;
  private JScrollPane scrollTree;

/** environment variables */
  private String[] envp = new String[4];

/** SOAP settings */
  static EmbreoParams mysettings;

/** true if in client-server mode (using SOAP) */
  static boolean withSoap;

/** to manage the pending results */
  public static PendingResults resultsManager;

/** Jemboss window dimension */
  public static Dimension jdim;
  public static Dimension jdimExtend;

  private ImageIcon fwdArrow;
  private ImageIcon bwdArrow;

  public Jemboss ()
  {

    ClassLoader cl = this.getClass().getClassLoader();
    fwdArrow = new ImageIcon(cl.getResource("images/Forward_arrow_button.gif"));
    bwdArrow = new ImageIcon(cl.getResource("images/Backward_arrow_button.gif"));

    AuthPopup splashing = null;
    if(!withSoap)
    {
      JembossParams jp = new JembossParams();
      plplot = jp.getPlplot();
      embossData = jp.getEmbossData();
      embossBin = jp.getEmbossBin();
      embossPath = jp.getEmbossPath();
      acdDirToParse = jp.getAcdDirToParse();
      embossPath = new String("PATH" + ps +
                               embossPath + ps);
      envp[0] = "PATH=" + embossPath;        
      envp[1] = "PLPLOT_LIB=" + plplot;
      envp[2] = "EMBOSS_DATA=" + embossData;
      envp[3] = "HOME=" + homeDirectory;
    }

    f = new JFrame("Jemboss");
// make the local file manager
    tree = new DragTree( new File(System.getProperty("user.home")), f, mysettings);
    scrollTree = new JScrollPane(tree);

    JPanel p1 = new JPanel(new BorderLayout()); // menu panel
    JPanel p2 = new JPanel(new GridLayout());   // emboss form pain
    p3 = new JPanel(new BorderLayout());        // filemanager panel

    JScrollPane scrollProgForm = new JScrollPane(p2);
    JPanel pwork = new JPanel(new BorderLayout());
    JPanel pform = new JPanel(new BorderLayout());

    pform.add(scrollProgForm, BorderLayout.CENTER);
    pwork.add(pform, BorderLayout.WEST);
    pwork.add(p3, BorderLayout.CENTER);

    JMenuBar btmMenu = new JMenuBar();

// button to extend window
    extend = new JButton(fwdArrow);
    extend.setBorder(BorderFactory.createMatteBorder(0,0,0,0, Color.black));
    extend.addActionListener(this);
    extend.setToolTipText("Open and close file manager.");

    Dimension d = f.getToolkit().getScreenSize();
    if(withSoap)
    {
      splashing = new AuthPopup(mysettings,3);
//    splashing = new Splash(mysettings,3);   //splash frame
      resultsManager = new PendingResults(mysettings);
      btmMenu.add(resultsManager.statusPanel(f));
    }
    else
    {
      btmMenu.add(Box.createHorizontalGlue());
      btmMenu.add(Box.createHorizontalStrut(5));
    }
    btmMenu.add(extend);
    pform.add(btmMenu,BorderLayout.SOUTH);

    pmain = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT,
                                  p1,pwork);
    pmain.setOneTouchExpandable(true);

// set window dimensions, dependent on screen size
    if(d.getWidth()<1024)
    {
      jdim = new Dimension(615,500);
      jdimExtend = new Dimension(795,500);
      pmain.setPreferredSize(jdim);
      scrollTree.setPreferredSize(new Dimension(180,500));
    }
    else
    {
      jdim = new Dimension(660,540);
      jdimExtend = new Dimension(840,540);
      pmain.setPreferredSize(jdim);
      scrollTree.setPreferredSize(new Dimension(180,540));
    }

// setup the top menu bar
    new SetUpMenuBar(mysettings, f, envp, cwd, withSoap);

// add to Jemboss main frame and locate it center left of screen
    f.getContentPane().add(pmain);
    f.pack();
    f.setLocation(0,((int)d.getHeight()-f.getHeight())/2);

    new BuildProgramMenu(p1,p2,pform,scrollProgForm,embossBin,envp,mysettings,
                         withSoap,cwd,acdDirToParse,f,splashing);

    f.addWindowListener(new winExit());

  }


/**
*
*  Action event to open the file manager
*
*
*/
  public void actionPerformed(ActionEvent ae)
  {
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

    if( p3.getComponentCount() > 0 )
    {
      p3.remove(0);
      extend.setIcon(fwdArrow);
      pmain.setPreferredSize(jdim);
      f.pack();
    }
    else
    {
      p3.add(scrollTree, BorderLayout.CENTER);
      extend.setIcon(bwdArrow);
      pmain.setPreferredSize(jdimExtend);
      f.pack();
    }

  }

/**
*
*  Delete temporary files
*  @param current working directory (local)
*
*/
  private void deleteTmp(File cwd, final String suffix) 
  {

    String tmpFiles[] = cwd.list(new FilenameFilter()
    {
      public boolean accept(File cwd, String name)
      {
        return name.endsWith(suffix);
      };
    });

    for(int h =0;h<tmpFiles.length;h++)
    {
      File tf = new File(tmpFiles[h]);
      tf.delete();
    }
  }


/**
*
* Extends WindowAdapter to close window 
*
*/
  class winExit extends WindowAdapter
  {
     public void windowClosing(WindowEvent we)
     {
        deleteTmp(new File(cwd), ".jembosstmp");
        System.exit(0);
     }
  }


/**
*
* Launches Jemboss in standalone or client-server mode.
*
*/
  public static void main (String args[])
  {
    
    // initialize settings
    mysettings = new EmbreoParams("jemboss");

    if(args.length > 0)
    {
      if(args[0].equalsIgnoreCase("local"))
      {
        withSoap = false; 
        System.out.println("Standalone mode");
      }
      else 
      {
        withSoap = true; 
        System.out.println("Client-server mode");
      }
    }
    else
      withSoap = true;

    new Jemboss();

  }

}


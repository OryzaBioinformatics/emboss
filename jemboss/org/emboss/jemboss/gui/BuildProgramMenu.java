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
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.util.zip.*;

import java.awt.event.*;
import java.io.*;
import java.util.*;

import org.emboss.jemboss.JembossJarUtil;
import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.programs.*;      // running EMBOSS programs
import org.emboss.jemboss.gui.startup.*;   // finds progs, groups, docs & db's
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.gui.form.*;      // prog forms constructed from ACD
import org.emboss.jemboss.soap.GetWossname;

/**
*
* BuildProgramMenu class construct the program menus.
*
* @author T. J. Carver
*
*/

public class BuildProgramMenu
{
  /** database names */
  private static String db[] = {"",""};
  /** matrices */
  private static Vector matrices = new Vector();
  /** codons usage tables  */
  private static Vector codons = new Vector();
  /** acd files cache */
  private Hashtable acdStore = new Hashtable();   
  private AuthPopup splashing;
  private SplashThread splashThread;

/**
*
*  @param  JPanel p1 is the menu pane
*  @param  ScollPanel p2 is the form pane
*  @param  JScrollPane EMBOSS form scroll pane 
*  @param  String location of the EMBOSS binaries
*  @param  String array of environment variables for EMBOSS applications.
*  @param  JembossParams SOAP parameter settings
*  @param  boolean true if using SOAP server
*  @param  String current working directory (local)
*  @param  String location of the ACD directory
*  @param  JFrame Jemboss frame
*  @param  Dimension form pane dimension
*
*/
  public BuildProgramMenu(final JPanel p1, final ScrollPanel p2, 
           final JPanel pform, final JScrollPane scrollProgForm,
           final String embossBin, final String envp[],
           final JembossParams mysettings, final boolean withSoap,
           final String cwd, final String acdDirToParse,
           final SetUpMenuBar mainMenu, final JFrame f,
           final Dimension jform)
  {
  
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  
    if(withSoap)
    {  
      splashing = new AuthPopup(mysettings,400);
      if(mysettings.getUseAuth())
      splashing.setBottomPanel();
      splashing.setSize(380,200);
      splashing.pack();
      splashing.setVisible(true);

      splashThread = new SplashThread(splashing,400-4);
      splashThread.start();
    }

    SwingWorker groupworker = new SwingWorker() 
    {

      String woss = "";

      public Object construct() 
      {
        if(withSoap) 
        {
          if(mysettings.getPublicSoapURL().startsWith("https"))
          {
            System.setProperty("https.proxyHost", "");
            System.setProperty("http.proxyHost", "");
            System.setProperty("proxyHost", "");  
            String settings[] = new String[1];
            settings[0] = new String("proxy.override=true");
            mysettings.updateJembossPropStrings(settings);
          }

          SwingWorker databaseworker = new SwingWorker()
          {
            public Object construct()
            {
              ShowDB showdb = null;
              try
              {
                showdb  = new ShowDB(mysettings);
              }
              catch (Exception ex)
              {
                splashing.doneSomething("Cannot connect!");
                ServerSetup ss = new ServerSetup(mysettings);
                int sso = JOptionPane.showConfirmDialog(f,ss,
                           "Check Settings",
                           JOptionPane.OK_CANCEL_OPTION,
                           JOptionPane.ERROR_MESSAGE,null);
                if(sso == JOptionPane.OK_OPTION)
                  ss.setNewSettings();
                else
                  System.exit(0);

                try
                {
                  showdb  = new ShowDB(mysettings);
                }
                catch (Exception exp)
                {
                  exp.printStackTrace();
                }
              }
              String showdbOut = showdb.getDBText();

              Database d = new Database(showdbOut);
              db = d.getDB();
              mainMenu.setEnableFileManagers(true);
              mainMenu.setEnableShowResults(true);
              splashing.doneSomething("");
              splashThread.setInterval(0);

              matrices = showdb.getMatrices();  // get the available matrices
              codons   = showdb.getCodonUsage();

              JLabel jl = new JLabel("<html>"); // not used but speeds first
                                                // ACD form loading which
                                                // uses html
              return null;
            }
          };
          databaseworker.start();
          
          splashing.doneSomething("");

          int iloop = 0;

          try
          {
            try
            {
              Hashtable hwoss = (new JembossJarUtil("resources/wossname.jar")).getHash();
              if(hwoss.containsKey("wossname.out"))
                woss = new String((byte[])hwoss.get("wossname.out"));

              mainMenu.setEnableFileManagers(false);
              mainMenu.setEnableShowResults(false);

              Hashtable hshowdb = (new JembossJarUtil("resources/showdb.jar")).getHash();
              mainMenu.setEnableFileManagers(false);
              mainMenu.setEnableShowResults(false);   

              if(hshowdb.containsKey("showdb.out"))
              {
                String showdbOut = new String((byte[])hshowdb.get("showdb.out"));
                Database d = new Database(showdbOut);
                db = d.getDB();
              }
            }
            catch (Exception ex){ System.out.println("calling the server"); }

            if(woss.equals(""))
            {
              GetWossname ewoss = new GetWossname(mysettings);
              woss = ewoss.getDBText(); 
              mainMenu.setEnableFileManagers(true);
              mainMenu.setEnableShowResults(true);
            }
            
            splashing.doneSomething("");
          } 
          catch(Exception e)
          {
            splashing.doneSomething("Cannot connect!");
            ServerSetup ss = new ServerSetup(mysettings);
            int sso = JOptionPane.showConfirmDialog(f,ss,
                           "Check Settings",
                           JOptionPane.OK_CANCEL_OPTION,
                           JOptionPane.ERROR_MESSAGE,null);
            if(sso == JOptionPane.OK_OPTION)
              ss.setNewSettings();
            else
              System.exit(0);
          }
        } 
        else 
        {
          String embossCommand = new String(embossBin + "wossname -colon -auto");
          System.out.println(embossCommand);
          RunEmbossApplication rea = new RunEmbossApplication(
                                      embossCommand,envp,null);
          rea.isProcessStdout();
          woss = rea.getProcessStdout();
          Process processWoss = rea.getProcess();

          embossCommand = new String(embossBin + "showdb -auto");
          rea = new RunEmbossApplication(embossCommand,envp,null);
          rea.isProcessStdout();
          String showdbOut = rea.getProcessStdout();

          try 
          {
            processWoss.waitFor();
          } 
          catch (InterruptedException interre)
          {
            System.out.println("BuildProgramMenu received interruption error");
          }
          Database d = new Database(showdbOut);
          db = d.getDB();

          // get the available matrices
          String dataFile[] = (new File(mysettings.getEmbossData())).list(new FilenameFilter()
          {
            public boolean accept(File dir, String name)
            {
              File fileName = new File(dir, name);
              return !fileName.isDirectory();
            };
          });

          matrices = new Vector();
          for(int i=0;i<dataFile.length;i++)
            matrices.add(dataFile[i]);
          
          // get the available codon usage tables
          dataFile = (new File(mysettings.getEmbossData()+
                                  "/CODONS")).list(new FilenameFilter()
          {
            public boolean accept(File dir, String name)
            {
              File fileName = new File(dir, name);
              return !fileName.isDirectory();
            };
          });

          codons = new Vector();
          for(int i=0;i<dataFile.length;i++)
            codons.add(dataFile[i]);
        }

        return woss;
      }


      public void finished() 
      {
// sets the delay for dismissing tooltips
        MultiLineToolTipUI.initialize();
        ToolTipManager toolTipManager = ToolTipManager.sharedInstance();
        toolTipManager.setDismissDelay(80000);

        try
        {
          acdStore = (new JembossJarUtil("resources/acdstore.jar")).getHash();
        }
        catch (Exception ex){}

// program menu
        JMenuBar menuBar = new JMenuBar();
        ProgList progs = new ProgList(woss,cwd,menuBar);

        if(withSoap)
          splashing.doneSomething("");

        int npG = progs.getNumPrimaryGroups();
        menuBar.setLayout(new  GridLayout(npG,1));
   
        final int numProgs = progs.getNumProgs();
        final String allAcd[] = progs.getProgsList();
        final String allDes[] = progs.getProgDescription();

        p1.add(menuBar, BorderLayout.NORTH);
        f.setVisible(true);

        JMenuItem mi[] = new JMenuItem[numProgs];
        mi = progs.getMenuItems();
        int nm = progs.getNumberMenuItems();

// create action listeners into menu to build Jemboss program forms
        for(int i=0; i<nm;i++)
        {
          mi[i].addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              f.setCursor(cbusy);
              JMenuItem source = (JMenuItem)(e.getSource());
              String p = source.getText();
              int ind = p.indexOf(" ");
              p = p.substring(0,ind).trim();

              for(int k=0;k<numProgs;k++)
              {
                if(p.equalsIgnoreCase(allAcd[k]))
                {
                  p2.removeAll();
                  String acdText = getAcdText(allAcd[k],acdDirToParse,
                                              mysettings,withSoap);
                  BuildJembossForm bjf = new BuildJembossForm(allDes[k],
                                db,allAcd[k],envp,cwd,embossBin,acdText,
                                withSoap,p2,mysettings,f);
                  scrollProgForm.setViewportView(p2);               
                  JViewport vp = scrollProgForm.getViewport();
                  vp.setViewPosition(new Point(0,0));
                  break;
                }
              }
              f.setCursor(cdone);
            }
          });

          if(withSoap)
          {
            JFrame splashf = splashing.getSplashFrame();
            if(splashf.isVisible())
              splashf.toFront();
          }
        }

// program scroll list
        final JList progList = new JList(allAcd);
        JScrollPane scrollPane = new JScrollPane(progList);

        Box alphaPane = new Box(BoxLayout.Y_AXIS);
        Box alphaTextPane = new Box(BoxLayout.X_AXIS);
        alphaPane.add(Box.createRigidArea(new Dimension(0,10)));
        alphaTextPane.add(new JLabel("GoTo:"));
        alphaTextPane.add(Box.createRigidArea(new Dimension(5,0)));

        final JTextField alphaTextPaneEntry = new JTextField(12);
        alphaTextPaneEntry.setMaximumSize(new Dimension(100,20));
        //scroll program list on typing 
        alphaTextPaneEntry.getDocument().addDocumentListener(new DocumentListener()
        {
          public void insertUpdate(DocumentEvent e)
          {
            updateScroll();
          }
          public void removeUpdate(DocumentEvent e) 
          {
            updateScroll();
          }
          public void changedUpdate(DocumentEvent e) {}
          public void updateScroll()
          {
            for(int k=0;k<numProgs;k++)
              if(allAcd[k].startsWith(alphaTextPaneEntry.getText()))
              {
                progList.ensureIndexIsVisible(k);
                progList.setSelectionBackground(Color.cyan);
                progList.setSelectedIndex(k);
                break;
              }
          }
        });
        //load program form on carriage return
        alphaTextPaneEntry.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            f.setCursor(cbusy);
            int index = progList.getSelectedIndex();
            p2.removeAll();
            String acdText = getAcdText(allAcd[index],acdDirToParse,
                                        mysettings,withSoap);
            BuildJembossForm bjf = new BuildJembossForm(allDes[index],
                                  db,allAcd[index],envp,cwd,embossBin,
                                  acdText,withSoap,p2,mysettings,f);
            scrollProgForm.setViewportView(p2);   
            JViewport vp = scrollProgForm.getViewport();
            vp.setViewPosition(new Point(0,0));
            f.setCursor(cdone);
          }
        });

        alphaTextPane.add(alphaTextPaneEntry);
        alphaPane.add(alphaTextPane);
        alphaPane.add(scrollPane);

        p1.add(alphaPane, BorderLayout.CENTER);

        Dimension dp1 = p1.getMinimumSize();
        dp1 = new Dimension((int)dp1.getWidth()-10,(int)dp1.getHeight());
        p1.setPreferredSize(dp1);
        p1.setMaximumSize(dp1);
        p1.setMinimumSize(dp1);

// put on the logo
        ClassLoader cl = this.getClass().getClassLoader();
        ImageIcon jlo = new ImageIcon(
                  cl.getResource("images/Jemboss_logo_large.gif"));
        JLabel jlablogo = new JLabel(jlo); 
        JPanel pFront = new JPanel();
        pFront.setBackground(Color.white);
        pFront.add(jlablogo);

// ensure fill the screen here as pform is BorderLayout.WEST
        int pwidth = (int)(f.getSize().getWidth()-p1.getSize().getWidth())-14;
        Dimension d = new Dimension(pwidth,100);
        pform.setPreferredSize(d);
        pform.setMinimumSize(d);
        jlablogo.setPreferredSize(jform);

        p2.add(pFront);

        progList.setSelectionBackground(Color.cyan);

// create listener to build Jemboss program forms
        MouseListener mouseListener = new MouseAdapter()
        {
          public void mouseClicked(MouseEvent e)
          {
//          System.gc();
            f.setCursor(cbusy);
            JList source = (JList)e.getSource();
            source.setSelectionBackground(Color.cyan);
            int index = source.getSelectedIndex();
            p2.removeAll();
            String acdText = getAcdText(allAcd[index],acdDirToParse,
                                        mysettings,withSoap);
            BuildJembossForm bjf = new BuildJembossForm(allDes[index],
                                  db,allAcd[index],envp,cwd,embossBin,
                                  acdText,withSoap,p2,mysettings,f);
            scrollProgForm.setViewportView(p2);
            JViewport vp = scrollProgForm.getViewport();
            vp.setViewPosition(new Point(0,0));
            f.setCursor(cdone);
          }
        };
        progList.addMouseListener(mouseListener);

        p1.setVisible(false);
        p1.setVisible(true);

      }
    };
    groupworker.start();

  }

/**
*
* List of available EMBOSS databases.
* @return String[] list of databases
*
*/
  protected static String[] getDatabaseList()
  {
    return db;
  }

  public static Vector getMatrices()
  {
    return matrices;
  }


  public static Vector getCodonUsage()
  {
    return codons;
  }




/**
*
* Get the contents of an ACD file in the form of a String.
* @param String of the ACD file name
* @param String representation of the ACD
*
*/
  private String getAcdText(String applName, String acdDirToParse,
                            JembossParams mysettings, boolean withSoap)
  {

    String acdText = new String("");
    String line;

    if(!withSoap)
    {
      String acdToParse = acdDirToParse.concat(applName).concat(".acd");
      try
      {
        BufferedReader in = new BufferedReader(new FileReader(acdToParse));
        while((line = in.readLine()) != null)
          acdText = acdText.concat(line + "\n");
        in.close();
      }
      catch (IOException e)
      {
        System.out.println("BuildProgramMenu: Cannot read acd file " + acdText);
      }
    }
    else 
    {
      if(acdStore.containsKey(applName+".acd"))
      {
        Object obj = acdStore.get(applName+".acd");

        if(obj.getClass().getName().equals("java.lang.String"))
          acdText = (String)obj;
        else
          acdText = new String((byte[])obj);
 
//      System.out.println("Retrieved "+applName+" acd file from cache");
      }
      else
      {
        GetACD progacd = new GetACD(applName,mysettings);
        acdText = progacd.getAcd();
//      System.out.println("Retrieved "+applName+" acd file via soap");
        acdStore.put(applName+".acd",acdText);
      }
    }
    return acdText;
  }

}



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

import org.emboss.jemboss.*;
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.gui.filetree.*;

import java.net.URL;

import java.awt.*;
import java.io.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;

/**
*
* Sets the top menu bar for Jemboss main window
*
*/
public class SetUpMenuBar
{

  public static SequenceList seqList;
  public static LocalAndRemoteFileTreeFrame localAndRemoteTree = null;
  private JMenuItem showLocalRemoteFile;
  private JMenuItem fileMenuShowres;
  private ServerSetup ss = null;

  public SetUpMenuBar(final JembossParams mysettings, final JFrame f,
                      final String envp[], final String cwd,
                      final boolean withSoap)
  {

    // cursors to show when we're at work
    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);

    JMenuBar menuPanel = new JMenuBar();
    new BoxLayout(menuPanel,BoxLayout.X_AXIS);
    menuPanel.add(Box.createRigidArea(new Dimension(5,24))); 

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);

    if(withSoap)
    {
      fileMenuShowres = new JMenuItem("Saved Results");
      fileMenuShowres.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
	  f.setCursor(cbusy);
          new ShowSavedResults(mysettings,f);
	  f.setCursor(cdone);
        }
      });
      fileMenu.add(fileMenuShowres);
 
      showLocalRemoteFile = new JMenuItem("Local and Remote Files");
      showLocalRemoteFile.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          f.setCursor(cbusy);
     
          if(localAndRemoteTree == null)
          {
            try
            {
              localAndRemoteTree = new LocalAndRemoteFileTreeFrame(mysettings);
              Dimension d = f.getToolkit().getScreenSize();
              int locY = (int)(d.getHeight()-localAndRemoteTree.getHeight())/2;
              int wid1 = (int)localAndRemoteTree.getPreferredSize().getWidth();
              int wid2 = f.getWidth();
              wid1 = (int)d.getWidth()-wid1;
              if(wid2 < wid1)
                wid1 = wid2;     
              localAndRemoteTree.setLocation(wid1,locY);
              localAndRemoteTree.setVisible(true);
            }
            catch(JembossSoapException jse)
            {
              localAndRemoteTree = null;
              AuthPopup ap = new AuthPopup(mysettings,f); 
              ap.setBottomPanel();
              ap.setSize(380,170);
              ap.pack();
              ap.setVisible(true);
            }
          }
          else
            localAndRemoteTree.setVisible(true);
          f.setCursor(cdone);
        }
      });
      fileMenu.add(showLocalRemoteFile);
      fileMenu.addSeparator();
    }

    final AdvancedOptions ao = new AdvancedOptions(mysettings);
    JMenuItem fileMenuExit = new JMenuItem("Exit");
    fileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {

        if(ao.isSaveUserHomeSelected())
          ao.userHomeSave();

        if(seqList.isStoreSequenceList())  //create a SequenceList file
          saveSequenceList();

        deleteTmp(new File(cwd), ".jembosstmp");
        System.exit(0);
      }
    });
    fileMenu.add(fileMenuExit);
    menuPanel.add(fileMenu);

    JMenu prefsMenu = new JMenu("Preferences");
    prefsMenu.setMnemonic(KeyEvent.VK_P);

    JMenuItem showAdvOpt = new JMenuItem("Advanced Options");
    showAdvOpt.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        JOptionPane jao = new JOptionPane();
        jao.showMessageDialog(f,ao,"Advanced Options",
                              JOptionPane.PLAIN_MESSAGE);
      }
    });
    prefsMenu.add(showAdvOpt);
    prefsMenu.addSeparator();

    JMenuItem serverSettings = new JMenuItem("Settings");
    serverSettings.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        if(ss == null)
          ss = new ServerSetup(mysettings);
        int sso = JOptionPane.showConfirmDialog(f,ss,"Jemboss Settings",
                             JOptionPane.OK_CANCEL_OPTION,
                             JOptionPane.PLAIN_MESSAGE,null);
        if(sso == JOptionPane.OK_OPTION)
          ss.setNewSettings();
      }
    });
    prefsMenu.add(serverSettings);

    
//  JMenuItem showEnvironment = new JMenuItem("Show Environment");
//  showEnvironment.addActionListener(new ActionListener()
//  {
//    public void actionPerformed(ActionEvent e) 
//    {
//      if(withSoap)
//        JOptionPane.showMessageDialog(f,
//         "Public Server: " + mysettings.getPublicSoapURL() +
//         "\nPublic Server Name: " + mysettings.getPublicSoapService() +
//         "\nPrivate Server: " + mysettings.getPrivateSoapURL() +
//         "\nPrivate Server Name: " + mysettings.getPrivateSoapService());
//      else
//        JOptionPane.showMessageDialog(f, 
//            envp[0] + "\n" + envp[1] + "\n" +
//            envp[2] + "\n" + envp[3] + "\n");
//    }
//  });
//  prefsMenu.add(showEnvironment);
    menuPanel.add(prefsMenu);


    JMenu toolMenu = new JMenu("Tools");
    toolMenu.setMnemonic(KeyEvent.VK_T);

    JMenuItem toolJalview  = new JMenuItem("Multiple Sequence Editor - Jalview");
    toolJalview.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new LaunchJalView();
      }
    });
    toolMenu.add(toolJalview);

    JMenuItem toolAlignJFrame = new JMenuItem("Multiple Sequence Editor - Jemboss");
    toolAlignJFrame.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new org.emboss.jemboss.editor.AlignJFrame();
      }
    });
    toolMenu.add(toolAlignJFrame);
    toolMenu.addSeparator();

    JMenuItem toolWorkList = new JMenuItem("Sequence List");
    seqList = new SequenceList(withSoap,mysettings);
    toolWorkList.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        seqList.setVisible(true); 
      }
    });
    toolMenu.add(toolWorkList);
    menuPanel.add(toolMenu);

    JMenu helpMenu = new JMenu("Help");
    helpMenu.setMnemonic(KeyEvent.VK_H);

    JMenuItem helpMenuAboutJemboss = new JMenuItem("About Jemboss");
    helpMenuAboutJemboss.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        ClassLoader cl = this.getClass().getClassLoader();
        try
        {
          URL inURL = cl.getResource("resources/readme.txt");
                                      
          JTextPane textURL = new JTextPane();

          ScrollPanel pscroll = new ScrollPanel(new BorderLayout());
          JScrollPane rscroll = new JScrollPane(pscroll);
          rscroll.getViewport().setBackground(Color.white);
          textURL.setPage(inURL);
          textURL.setEditable(false);
          pscroll.add(textURL);
          JOptionPane jop = new JOptionPane();
          rscroll.setPreferredSize(new Dimension(560,400));
          rscroll.setMinimumSize(new Dimension(560,400));
          rscroll.setMaximumSize(new Dimension(560,400));

          jop.showMessageDialog(f,rscroll,"Jemboss Help",
                              JOptionPane.PLAIN_MESSAGE);
        } 
        catch (Exception ex)
        {
          System.out.println("Didn't find resources/" +
                             "readme.txt");
        }
      }
    });
    helpMenu.add(helpMenuAboutJemboss);

    JMenuItem helpMenuAbout = new JMenuItem("Version");
    helpMenuAbout.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      { 
        ClassLoader cl = this.getClass().getClassLoader();
        String fc = "";
        try
        {
          String line;

          InputStream in = cl.getResourceAsStream("resources/version");
          BufferedReader reader = new BufferedReader(new InputStreamReader(in));
          while((line = reader.readLine()) != null)
            fc = fc.concat(line);
        }
        catch (Exception ex)
        {
          System.out.println("Didn't find resources/" +
                             "version");
        }
        JOptionPane.showMessageDialog(f, fc +
                  " by the EMBOSS team at the HGMP-RC (UK)");
      }
    });
    helpMenu.add(helpMenuAbout);

    menuPanel.add(helpMenu);
    menuPanel.add(Box.createHorizontalGlue());

    f.setJMenuBar(menuPanel);

   }

   public void setEnableFileManagers(boolean b)
   {
     showLocalRemoteFile.setEnabled(b);
   }

   public void setEnableShowResults(boolean b)
   {
     fileMenuShowres.setEnabled(b);
   }

   public static DragTree getLocalDragTree()
   {
     if(localAndRemoteTree == null)
       return null;

     return localAndRemoteTree.getLocalDragTree();
   }

/**
*
*  Delete temporary files
*
*/
  public void deleteTmp(File cwd, final String suffix) 
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
* Save the sequence list for a future session.
*
*/
  public static void saveSequenceList()
  {
    File fseq = new File(System.getProperty("user.home")
                + System.getProperty("file.separator")
                + ".jembossSeqList");
    try
    {
      PrintWriter fout = new PrintWriter(new FileWriter(fseq));

      for(int i=0;i<seqList.getRowCount();i++)
      {
        SequenceData seqData = seqList.getSequenceData(i);
        String sbeg = seqData.s_beg;
        if(sbeg.equals(""))
          sbeg = "-";
        String send = seqData.s_end;
        if(send.equals(""))
          send = "-";

        if(!seqData.s_name.equals(""))
          fout.println(seqData.s_name + " " +
                     sbeg + " " + send + " " +
                     seqData.s_listFile.toString()+ " " +
                     seqData.s_default.toString() + " " +
                     seqData.s_remote.toString() );
      }
      fout.close();
    }
    catch (IOException ioe){}

  }

}


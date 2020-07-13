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


import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.net.*;
import java.io.*;
import java.util.Vector;

public class Browser extends JFrame implements HyperlinkListener, 
                                 ActionListener 
{

  private MemoryComboBox urlField;
  private JEditorPane htmlPane;
  private String initialURL;
  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);


  public Browser(String initialURL, String name) throws IOException
  {
    this(initialURL,name,false,"");
  }

  public Browser(String initialURL, String name,  boolean ltext, 
                         String text) throws IOException
  {

    super(name);
    this.initialURL = initialURL;

    Vector urlCache = new Vector();
    if(!ltext)
      urlCache.add(initialURL);
    else
      urlCache.add(name+".html");

    if(ltext)
    {
      htmlPane = new JEditorPane();
      if( (text.indexOf("<html>") > -1) ||
          (text.indexOf("<HTML>") > -1) )
        htmlPane.setContentType("text/html");
      htmlPane.setText(text);
    }
    else 
    {
      try
      {
        htmlPane = new JEditorPane(initialURL);
        htmlPane.addHyperlinkListener(this);
      } 
      catch(IOException ioe) 
      {
        throw new IOException();
      }
    }

    setBrowserSize();
    setUpJMenuBar(urlCache);
    addToScrollPane();
    setVisible(true);
  }

  public Browser(URL urlName, String initialURL) throws IOException
  {
    super(initialURL);
    this.initialURL = initialURL;
    Vector urlCache = new Vector();
    urlCache.add(initialURL);

    try
    {
      htmlPane = new JEditorPane(urlName);
    }
    catch(IOException ioe)
    {
      throw new IOException();
    }

    setBrowserSize();
    setUpJMenuBar(urlCache);
    addToScrollPane();
    setVisible(true);
  }

/**
*
* Method to create the frames menu and tool bar.
*
*/
  private void setUpJMenuBar(Vector urlCache)
  {
    JMenuBar menuBar = new JMenuBar();
    JToolBar toolBarURL  = new JToolBar();
    JToolBar toolBarIcon = new JToolBar();

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(fileMenu);

    // back
    JMenuItem backMenu = new JMenuItem("Back");
    backMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_B, ActionEvent.CTRL_MASK));
    backMenu.setActionCommand("BACK");
    backMenu.addActionListener(this);
    fileMenu.add(backMenu);

    // close
    fileMenu.addSeparator();
    JMenuItem closeMenu = new JMenuItem("Close");
    closeMenu.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    closeMenu.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        setVisible(false);
      }
    });
    fileMenu.add(closeMenu);

    // jemboss logo button
    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon jem = new ImageIcon(cl.getResource("images/Jemboss_logo_small.gif"));
    JIconButton jembossButton = new JIconButton(jem);
    jembossButton.addActionListener(this);
    jembossButton.setActionCommand("JEMBOSS");

    // url field
    JLabel urlLabel = new JLabel("URL:");
    urlField = new MemoryComboBox(urlCache);
    
    int urlFieldHeight = (int)urlField.getPreferredSize().getHeight();
    urlField.addActionListener(this);

    toolBarIcon.add(jembossButton);
    toolBarURL.add(urlLabel);
    toolBarURL.add(urlField);

    setJMenuBar(menuBar);
    getContentPane().add(toolBarURL, BorderLayout.NORTH);
    getContentPane().add(toolBarIcon, BorderLayout.SOUTH);

    int urlFieldWidth  = (int)toolBarURL.getPreferredSize().getWidth(); 
    Dimension d = new Dimension(urlFieldWidth,urlFieldHeight);
    urlField.setMaximumSize(d);

    int iconBarWidth  = (int)toolBarIcon.getPreferredSize().getWidth();
    int iconBarHeight = jem.getIconHeight();
    d = new Dimension(iconBarWidth,iconBarHeight);
    toolBarIcon.setPreferredSize(d);

  }

  private void setBrowserSize()
  {
    Dimension screenSize = getToolkit().getScreenSize();
    int width  = screenSize.width * 5 / 10;
    int height = screenSize.height * 6 / 10;
    setBounds(width/5, height/6, width, height);
  }

  private void addToScrollPane()
  {
    htmlPane.setEditable(false);
    htmlPane.setCaretPosition(0);
    JScrollPane scrollPane = new JScrollPane(htmlPane);
   
    // ensures html wraps properly
    htmlPane.setPreferredSize(getPreferredSize());
    getContentPane().add(scrollPane, BorderLayout.CENTER);
  }

  public void actionPerformed(ActionEvent event) 
  {
    String url;
    setCursor(cbusy);
    if (event.getSource() == urlField) 
      url = (String)urlField.getSelectedItem();
    else if (event.getActionCommand().equals("JEMBOSS"))
      url = "http://www.hgmp.mrc.ac.uk/Software/EMBOSS/Jemboss/";
    else if (event.getActionCommand().equals("BACK"))
      url = (String)urlField.getItemAt(1);
    else
      url = initialURL;

    try
    {
      htmlPane.setPage(new URL(url));
      urlField.add(url);
    }
    catch(IOException ioe)
    {
      setCursor(cdone);
      warnUser("Can't follow link to " + url );
    }
    setCursor(cdone);
  }

/**
*
* Method to handle hyper link events.
*
*/
  public void hyperlinkUpdate(HyperlinkEvent event) 
  {
    if (event.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
    {
      setCursor(cbusy);
      try 
      {
        htmlPane.setPage(event.getURL());
        urlField.add(event.getURL().toExternalForm());
      } 
      catch(IOException ioe) 
      {
        setCursor(cdone);
        warnUser("Can't follow link to " +  
                  event.getURL().toExternalForm() );
      }
      
      setCursor(cdone);
    }
  }

  private void warnUser(String message)
  {
    JOptionPane.showMessageDialog(this, message, "Warning", 
                                  JOptionPane.ERROR_MESSAGE);
  }


  public class JIconButton extends JButton 
  {
    public JIconButton(ImageIcon ii) 
    {
      super(ii);
      setContentAreaFilled(false);
      setBorderPainted(false);
      setFocusPainted(false);
    }
  }


}



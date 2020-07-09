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
import javax.swing.border.*;
import javax.swing.event.*;
import javax.swing.tree.*;

import java.awt.event.*;
import java.io.*;
import java.util.*;

import org.emboss.jemboss.programs.*;
import org.emboss.jemboss.soap.*;
import uk.ac.mrc.hgmp.embreo.*;

/**
*
* Shows a list of results from the SOAP server
* and displays individual result sets
*
*/
public class ShowSavedResults
{


  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
  private DefaultListModel datasets = new DefaultListModel();
  private JFrame savedResFrame;
  private JPanel sp = new JPanel();
  private JTextArea aboutRes; 
  private JScrollPane aboutScroll;
  private JScrollPane ss;
  private JPanel resButtonStatus;
  private JTextField statusField;
  private JMenuBar resMenu = new JMenuBar();
  private ImageIcon rfii;

  public ShowSavedResults(String frameName)
  {
    savedResFrame = new JFrame(frameName);
    aboutRes = new JTextArea("Select a result set from"
                              +"\nthose listed and details"
                              +"\nof that analysis will be"
                              +"\nshown here. Then you can"
                              +"\neither delete or view those"
                              +"\nresults using the buttons below.");
    aboutScroll = new JScrollPane(aboutRes);  
    ss = new JScrollPane(sp);
    resMenu.setLayout(new FlowLayout(FlowLayout.LEFT,10,1));
    ClassLoader cl = this.getClass().getClassLoader();
    rfii = new ImageIcon(cl.getResource("images/Refresh_button.gif"));

//results status
    resButtonStatus = new JPanel(new BorderLayout());
    Border loweredbevel = BorderFactory.createLoweredBevelBorder();
    Border raisedbevel = BorderFactory.createRaisedBevelBorder();
    Border compound = BorderFactory.createCompoundBorder(raisedbevel,loweredbevel);
    statusField = new JTextField();
    statusField.setBorder(compound);
    statusField.setEditable(false);
  }


/**
*
* Show the saved results on the server.
*
*/
  public ShowSavedResults(final EmbreoParams mysettings, final JFrame f) 
  {

    this("Saved Results on Server");
     
    Dimension d = new Dimension(270,270);
    ss.setPreferredSize(d);
    ss.setMaximumSize(d);

    try
    {
      final ResultList reslist = new ResultList(mysettings);

      JMenu resFileMenu = new JMenu("File");
      resMenu.add(resFileMenu);

      JButton refresh = new JButton(rfii);
      refresh.setMargin(new Insets(0,1,0,1));
      refresh.setToolTipText("Refresh");
      resMenu.add(refresh);

      refresh.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
          try
          {
            savedResFrame.setCursor(cbusy);
            ResultList newlist = new ResultList(mysettings);
            savedResFrame.setCursor(cdone);
            if (newlist.getStatus().equals("0")) 
            {
              reslist.updateRes(newlist.hash());
              datasets.removeAllElements();
              StringTokenizer tok = new StringTokenizer((String)reslist.get("list"), "\n");
              while (tok.hasMoreTokens()) 
              {
                String image = tok.nextToken();
                datasets.addElement(image);
              }
            } 
            else 
            {
              EmbreoUtils.warningPopup(savedResFrame,newlist.getStatusMsg());
            }
          } 
          catch (JembossSoapException eae) 
          {
            new AuthPopup(mysettings,savedResFrame);
          }
        }
      });

      JMenuItem resFileMenuExit = new JMenuItem("Close");
      resFileMenuExit.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
          savedResFrame.setVisible(false);
        }
      });
      resFileMenu.add(resFileMenuExit);
      savedResFrame.setJMenuBar(resMenu);
        
      // this is the list of saved results
        
      StringTokenizer tokenizer =
                 new StringTokenizer((String)reslist.get("list"), "\n");

      while (tokenizer.hasMoreTokens()) 
      {
        String image = tokenizer.nextToken();
        datasets.addElement(image);
      }

      final JList st = new JList(datasets);
      st.addListSelectionListener(new ListSelectionListener()
      {
        public void valueChanged(ListSelectionEvent e) 
        {
          if (e.getValueIsAdjusting())
            return;

          JList theList = (JList)e.getSource();
          if (theList.isSelectionEmpty()) 
          {
            System.out.println("Empty selection");
          } 
          else 
          {
            int index = theList.getSelectedIndex();
            String thisdata = datasets.elementAt(index).toString();
            reslist.setCurrent(thisdata);
	    aboutRes.setText((String)reslist.get(thisdata));
            aboutRes.setCaretPosition(0);
	  }
	}
      });

      st.addMouseListener(new MouseAdapter() 
      {
        public void mouseClicked(MouseEvent e) 
        {
          if (e.getClickCount() == 2) 
          {
            int index = st.locationToIndex(e.getPoint());
            try
            {
              savedResFrame.setCursor(cbusy);
              ResultList thisres = new ResultList(mysettings, reslist.getCurrent(),
                                                      "show_saved_results");
              new ShowResultSet(thisres.hash());
              savedResFrame.setCursor(cdone);
            } 
            catch (JembossSoapException eae) 
            {  
              new AuthPopup(mysettings,f);
            }
          }
        }
      });
      sp.add(st);
        
      // action buttons
      // display retrieves all the files and shows them in a window
      JPanel resButtonPanel = new JPanel();
      JButton showResButton = new JButton("Display");
      showResButton.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
	{
	  if(reslist.getCurrent() != null)
          {
	    try 
	    {
	      savedResFrame.setCursor(cbusy);
	      ResultList thisres = new ResultList(mysettings, reslist.getCurrent(), 
                                                       "show_saved_results");
              new ShowResultSet(thisres.hash());
	      savedResFrame.setCursor(cdone);
	    } 
            catch (JembossSoapException eae)
            {
              new AuthPopup(mysettings,f);
	    }
	  } 
	  else 
	  {
	    System.out.println("Nothing selected.");
	  }
        }
      });
        
      // delete removes the file on the server
      // and edits the list
      JButton delResButton = new JButton("Delete");
      delResButton.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e) 
        {
          if(reslist.getCurrent() != null) 
          {
            try        // ask the server to delete these results
	    {
	      savedResFrame.setCursor(cbusy);
	      ResultList thisres = new ResultList(mysettings, reslist.getCurrent(), 
                                                       "delete_saved_results");
	      savedResFrame.setCursor(cdone);
	       
              statusField.setText("Deleted " +reslist.getCurrent() + "  results set");
     
	      // clean up the list so they can't see it any more
	        
	      reslist.setCurrent(null);
	      aboutRes.setText("");
	      int index = st.getSelectedIndex();
	      datasets.remove(index);
	      st.setSelectedIndex(-1);
	    } 
	    catch (JembossSoapException eae) 
	    {
              new AuthPopup(mysettings,f);
	    }
	  } 
          else 
          {
            System.out.println("Nothing selected.");
	  }
	}
      });
      resButtonPanel.add(delResButton);
      resButtonPanel.add(showResButton);
      resButtonStatus.add(resButtonPanel, BorderLayout.CENTER);
      resButtonStatus.add(statusField, BorderLayout.SOUTH);
      savedResFrame.getContentPane().add(ss,BorderLayout.WEST);
      savedResFrame.getContentPane().add(aboutScroll,BorderLayout.CENTER);
      savedResFrame.getContentPane().add(resButtonStatus,BorderLayout.SOUTH);
      savedResFrame.pack();
      
      savedResFrame.setVisible(true);
    } 
    catch (JembossSoapException eae) 
    {
      new AuthPopup(mysettings,f);
    }

  }


/**
*
* Show the results sent to a batch queue.
*
*/
  public ShowSavedResults(final EmbreoParams mysettings, final PendingResults epr)
                                           throws JembossSoapException
  {

    this("Current Sessions Results");

    Dimension d = new Dimension(270,100);
    ss.setPreferredSize(d);
    ss.setMaximumSize(d);

    JMenu resFileMenu = new JMenu("File");
    resMenu.add(resFileMenu);

    JButton refresh = new JButton(rfii);
    refresh.setMargin(new Insets(0,1,0,1));
    refresh.setToolTipText("Refresh");
    resMenu.add(refresh);
    refresh.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
	savedResFrame.setCursor(cbusy);
	epr.updateStatus();
	savedResFrame.setCursor(cdone);
	datasets.removeAllElements();
	Enumeration enum = epr.descriptionHash().keys();
	while (enum.hasMoreElements()) 
        {
	  String image = (String)enum.nextElement().toString();
	  datasets.addElement(image);
	}
      }
    });

    JMenuItem resFileMenuExit = new JMenuItem("Close",KeyEvent.VK_C);
    resFileMenuExit.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
	savedResFrame.dispose();
      }
    });
    resFileMenu.add(resFileMenuExit);
    savedResFrame.setJMenuBar(resMenu);
    
    // set up the results list in the gui
    Enumeration enum = epr.descriptionHash().keys();
    while (enum.hasMoreElements()) 
    {
      String image = (String)enum.nextElement().toString();
      datasets.addElement(image);
    }

    final JList st = new JList(datasets);
    st.addListSelectionListener(new ListSelectionListener()
    {
      public void valueChanged(ListSelectionEvent e) 
      {
	if (e.getValueIsAdjusting())
	  return;
	
	JList theList = (JList)e.getSource();
	if (theList.isSelectionEmpty()) 
        {
	  if (mysettings.getDebug()) 
	    System.out.println("ResListView: Empty selection");
	} 
        else
        {
	  int index = theList.getSelectedIndex();
	  String thisdata = datasets.elementAt(index).toString();
	  epr.setCurrent(thisdata);
	  aboutRes.setText((String)epr.descriptionHash().get(thisdata));
      	  aboutRes.setCaretPosition(0);
	  aboutRes.setEditable(false);
	}
      }
    });


    st.addMouseListener(new MouseAdapter() 
    {
      public void mouseClicked(MouseEvent e) 
      {
	if (e.getClickCount() == 2) 
        {
	  int index = st.locationToIndex(e.getPoint());
	  try
          {
	    savedResFrame.setCursor(cbusy);
	    ResultList thisres = new ResultList(mysettings, epr.getCurrent(), 
                                                     "show_saved_results");
	    savedResFrame.setCursor(cdone);
	    if (thisres.getStatus().equals("0")) 
              new ShowResultSet(thisres.hash());
            else  
	      EmbreoUtils.errorPopup(savedResFrame,thisres.getStatusMsg());
 
	  } 
          catch (JembossSoapException eae) 
          {
	    new AuthPopup(mysettings,savedResFrame);
	  }
	}
      }
    });
    sp.add(st);
    
    // display retrieves all the files and shows them in a window
   
    JPanel resButtonPanel = new JPanel();
    JButton showResButton = new JButton("Display");
    showResButton.addActionListener(new ActionListener(){
      public void actionPerformed(ActionEvent e) 
      {
	if(epr.getCurrent() != null) 
        {
	  try
          {
	    savedResFrame.setCursor(cbusy);
	    ResultList thisres = new ResultList(mysettings, epr.getCurrent(), 
                                                      "show_saved_results");
	    savedResFrame.setCursor(cdone);
	    if (thisres.getStatus().equals("0")) 
              new ShowResultSet(thisres.hash());
            else 
	      EmbreoUtils.errorPopup(savedResFrame,thisres.getStatusMsg());
	    
	  } 
          catch (JembossSoapException eae) 
          {
            savedResFrame.setCursor(cdone);
	    new AuthPopup(mysettings,savedResFrame);
	  }
	}
      }
    });
    
    // delete removes the file on the server and edits the list
   
    JButton delResButton = new JButton("Delete");
    delResButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
	if(epr.getCurrent() != null) 
        {
	  try 
          {
	    savedResFrame.setCursor(cbusy);
	    ResultList thisres = new ResultList(mysettings, epr.getCurrent(),
                                                      "delete_saved_results");
	    savedResFrame.setCursor(cdone);

            JembossProcess jp = epr.getResult(epr.getCurrent());
            epr.removeResult(jp);

            statusField.setText("Deleted " + epr.getCurrent() + "  results set");
	    epr.setCurrent(null);
	    
	    // clean up the list so they can't see it any more
	   
	    aboutRes.setText("");
	    int index = st.getSelectedIndex();
            if(index >-1)
	      datasets.remove(index);
	    st.setSelectedIndex(-1);
	  }
          catch (JembossSoapException eae)
          {
	    // shouldn't happen
	    new AuthPopup(mysettings,savedResFrame);
	  }
	}
      }
    });
    resButtonPanel.add(delResButton);
    resButtonPanel.add(showResButton);
    resButtonStatus.add(resButtonPanel, BorderLayout.CENTER);
    resButtonStatus.add(statusField, BorderLayout.SOUTH);

    savedResFrame.getContentPane().add(ss,BorderLayout.WEST);
    savedResFrame.getContentPane().add(aboutScroll,BorderLayout.CENTER);
    savedResFrame.getContentPane().add(resButtonStatus,BorderLayout.SOUTH);
    savedResFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
    
    savedResFrame.pack();
    savedResFrame.setVisible(true);

//add in automatic updates
    String freq = (String)AdvancedOptions.jobMgr.getSelectedItem();
    int ind = freq.indexOf(" ");
    new ResultsUpdateTimer(Integer.parseInt(freq.substring(0,ind)),
                           datasets, savedResFrame);
    statusField.setText("Window refresh rate " + freq);

  }

}


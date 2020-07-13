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
********************************************************************/

package org.emboss.jemboss.soap;

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.gui.*;
import org.emboss.jemboss.programs.*;

import java.util.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class PendingResults 
{

  private int completed_jobs = 0;
  private int running_jobs = 0;
  private JembossParams mysettings;
  private Vector pendingResults;
  private JButton jobButton = null;
  private JComboBox jobComboBox = null;
  private boolean autoUpdates = false;


  public PendingResults(JembossParams mysettings)
  {
    this.mysettings = mysettings;
    pendingResults = new Vector();
  }


  public void resetCount() 
  {
    completed_jobs = 0;
    running_jobs = 0;
    pendingResults.removeAllElements();
  }

  public void addResult(JembossProcess res) 
  {
    pendingResults.add(res);
  }

  public void removeResult(JembossProcess res)
  {
    pendingResults.remove(res);
  }

/**
*
* @param s  The name of the dataset
* @return the process object
*
*/
  public JembossProcess getResult(String s)
  {
    for (int i=0 ; i < pendingResults.size(); ++i)
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      if(er.getJob().equals(s))
        return er;
    }
    return null;
  }


  public Hashtable descriptionHash() 
  {
    Hashtable h = new Hashtable();
    for (int i=0 ; i < pendingResults.size(); ++i) 
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      String desc = er.getDescription();
      if(desc == null || desc.equals("")) 
        desc = " Pending";
      h.put(er.getJob(),desc);
    }
    return h;
  }

  public Hashtable statusHash() 
  {
    Hashtable h = new Hashtable();
    for (int i=0 ; i < pendingResults.size(); ++i)
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      h.put(er.getJob(),new Boolean(er.isCompleted()));
    }
    return h;
  }

  public void updateJobStats() 
  {
    int ic = 0;
    int ir = 0;
    for (int i=0 ; i < pendingResults.size(); ++i) 
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      if(er.isCompleted())  
        ++ic; 
      if(er.isRunning())  
        ++ir; 
    }
    completed_jobs = ic;
    running_jobs = ir;
  }

/**
*
* Report the status of completed and running processes.
*
*/
  public String jobStatus() 
  {
    String sc =  new Integer(completed_jobs).toString();
    String sr =  new Integer(running_jobs).toString();
    String s;

    if (completed_jobs == 0) 
    {
      if(running_jobs == 0) 
        s = "Jobs: no pending jobs";
      else 
        s = "Jobs: " + sr + " running";
    }
    else
    {
      if (running_jobs == 0) 
        s = "Jobs: " + sc + " completed";
      else 
        s = "Jobs: " + sc + " completed / " + sr + " running";
    }
    return s;
  }

/**
*
* Connect to the embreo server, and update the status of the jobs
* in the list. If a statusPanel is active, updates the text on that.
*
*/
  public void updateStatus() 
  {
    Vector params = new Vector();
    Hashtable resToQuery = new Hashtable();

    //initialize hash with project/jobid
    for (int i=0 ; i < pendingResults.size(); ++i) 
    {
      JembossProcess er = (JembossProcess)pendingResults.get(i);
      resToQuery.put(er.getJob(),er.getProject());
    }

//  params.addElement(new Parameter("prog", String.class,
//                                  "", null));
//  params.addElement(new Parameter("options", String.class,
//                                  "", null));
//  params.addElement(new Parameter("queries", Hashtable.class,
//                                  resToQuery, null));

    params.addElement("");
    params.addElement("");
    params.addElement(getVector(resToQuery));
    try 
    {
      PrivateRequest eq = new PrivateRequest(mysettings,
                               "update_result_status", params);
      // update the results
      for(int i=0; i < pendingResults.size(); ++i) 
      {
        JembossProcess er = (JembossProcess)pendingResults.get(i);
        String jobid = er.getJob();
        String s = (String)eq.getVal(jobid);
        if (mysettings.getDebug()) 
          System.out.println("PendingResults: "+jobid+" : "+s);

        if (s.equals("complete"))
        {
          er.complete();
          String sd = (String)eq.getVal(jobid+"-description");
          if (!sd.equals("")) 
            er.setDescription(sd);
        }
      }
      updateJobStats();
      if (jobButton != null) 
        jobButton.setText(jobStatus());
      
    } 
    catch (JembossSoapException e)
    {
      //throw new JembossSoapException();
    }

  }

  private Vector getVector(Hashtable h)
  {
    Vector v = new Vector();
    for(Enumeration e = h.keys() ; e.hasMoreElements() ;)
    {
      String s = (String)e.nextElement();
      v.add(s);
      v.add(h.get(s));
    }

    return v;
  }


/**
*
*  @return true if automatically updating status manager 
*        with BatchUpdateTimer thread
*
*/
  public boolean isAutoUpdate()
  {
    return autoUpdates;
  }

/**
*
* @param true if automatically updating status manager 
*        with BatchUpdateTimer thread
*
*/
  public void setAutoUpdate(boolean b)
  {
    autoUpdates = b;
  }

/**
*
* Updates the mode on the combo box to reflect the current state
*
*/
  public void updateMode() 
  {
    if (jobComboBox != null) 
      jobComboBox.setSelectedItem(mysettings.getCurrentMode());
  }

/**
*
* Updates the mode on the combo box to reflect the
* requested value
*
*/
  public void updateMode(String s) 
  {
    mysettings.setCurrentMode(s);
    if (jobComboBox != null) 
      jobComboBox.setSelectedItem(mysettings.getCurrentMode());
  }


/**
*
* A panel with appropriate gadgets to show the status of any jobs
* and to view them, and to set the mode.
* @param f The parent frame, to which dialogs will be attached.
*
*/
  public JPanel statusPanel(final JFrame f) 
  {
    final JPanel jobPanel = new JPanel(new BorderLayout());

    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon jobIcon = new ImageIcon(cl.getResource("images/Job_manager_button.gif"));
    JLabel jobLabel = new JLabel(jobIcon);
    jobLabel.setToolTipText("Batch Job Manager");

    jobPanel.add(jobLabel,BorderLayout.WEST);
    jobButton = new JButton("(No Current Jobs)");
    jobButton.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        jobPanel.setCursor(new Cursor(Cursor.WAIT_CURSOR));
        showPendingResults(f);
        jobPanel.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
      }
    });
    jobPanel.add(jobButton,BorderLayout.CENTER);
    jobComboBox = new JComboBox(mysettings.modeVector());

    updateMode();
    jobComboBox.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e) 
      {
        JComboBox cb = (JComboBox)e.getSource();
        String modeName = (String)cb.getSelectedItem();
        mysettings.setCurrentMode(modeName);
      }
    });
    jobPanel.add(jobComboBox,BorderLayout.EAST);

    Dimension d = jobPanel.getPreferredSize();
    d = new Dimension((int)d.getWidth(),
                      jobIcon.getIconHeight()-2);

    jobPanel.setPreferredSize(d);
    return jobPanel;
  }

  public void showPendingResults(JFrame f) 
  {
    if ((completed_jobs == 0) && (running_jobs == 0)) 
    {
      JOptionPane.showMessageDialog(f,"You can only view pending results\n"
				    + "if any background jobs have been\n"
				    + "submitted in the current session.");
    } 
    else 
    {
      try
      {
        new ShowSavedResults(mysettings, this);
      } 
      catch (JembossSoapException eae) 
      {
        new AuthPopup(mysettings,f);
      }
    }
  } 

/**
*
* @return job status text
*
*/
  public String getStatus()
  {
    return jobButton.getText();
  }

}


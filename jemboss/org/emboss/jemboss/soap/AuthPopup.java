/****************************************************************
*
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  Based on EmbreoAuthPopup
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.awt.*;
import javax.swing.*;
import java.awt.event.*;
import org.emboss.jemboss.JembossParams;

public class AuthPopup 
{

  private int iprogress = 0;
  private int iprogressmax;
  private JembossParams mysettings;
  private JPanel splashp;
  private JFrame splashf;
  JProgressBar progressBar;
  JLabel progressLabel;

  private boolean exitOnDone = false;

/**
*
* Display a popup asking the user to enter a username and password
* @param mysettings JembossParams defining server parameters
* @param f          Parent frame the popup is associated with
*
*/
  public AuthPopup(final JembossParams mysettings, JFrame f) 
  {

    if(f != null)
    {
      String text = "";
      if (mysettings.getUseAuth() == true) 
      {
        if (mysettings.getServiceUserName() == null) 
	  text = "You need to supply a username and password\n"
	       + "before running an application.";
        else 
          text = "Login to server failed\n"
	       + "Please check your login details.";
      } 
      else
      {
        text = "The server wants a username and password,\n"
	     + "but we weren't expecting to need to.\n"
             + "Please supply the correct login details.";
      }
      JOptionPane.showMessageDialog(f,text,"Authentication failed",
                                    JOptionPane.ERROR_MESSAGE);
    }

// setup a Jemboss login box
    JPanel logoPanel = new JPanel(new BorderLayout());
    ClassLoader cl = this.getClass().getClassLoader();
    ImageIcon ii = new ImageIcon(
               cl.getResource("images/Jemboss_logo_greyback.gif"));
    logoPanel.add(new JLabel(ii),BorderLayout.WEST);

    splashp = new JPanel(new BorderLayout());
    splashp.add(logoPanel, BorderLayout.NORTH);

    //if required, a login prompt
    if (mysettings.getUseAuth()) 
    {
      splashf = new JFrame("Login");
      JPanel promptPanel = new JPanel(new BorderLayout());
      JPanel loginPanel = new JPanel();
      loginPanel.setLayout(new GridLayout(2,2));
      
      final JTextField ufield = new JTextField(16);
      if (mysettings.getServiceUserName() != null) 
	ufield.setText(mysettings.getServiceUserName());
      
      final JPasswordField pfield = new JPasswordField(16);
      final JTextField xfield = new JTextField(16);

      //close login box on carriage return in passwd field
      pfield.addActionListener(new ActionListener()
      {
        public void actionPerformed(ActionEvent e)
        {
          mysettings.setServiceUserName(ufield.getText());
          mysettings.setServicePasswd(pfield.getPassword());
          exitOnDone = true;
          splashf.dispose();
        }
      });

      JLabel ulab = new JLabel(" Username:", SwingConstants.LEFT);
      JLabel plab = new JLabel(" Password:", SwingConstants.LEFT);
      //add labels etc
      loginPanel.add(ulab);
      loginPanel.add(ufield);
      loginPanel.add(plab);
      loginPanel.add(pfield);

      promptPanel.add(loginPanel, BorderLayout.CENTER);
      // buttons across the bottom
      JPanel buttonPanel = new JPanel();
      buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
      JButton cancelButton = new JButton("Cancel");
      JButton exitButton = new JButton("Exit");
      JButton okButton = new JButton("OK");
      cancelButton.addActionListener(new ActionListener() 
      {
	public void actionPerformed(ActionEvent e) 
        {
	  splashf.setVisible(false);
	}
      });

      exitButton.addActionListener(new ActionListener() 
      {
	public void actionPerformed(ActionEvent e) 
        {
	  System.exit(0);
	}
      });

      okButton.addActionListener(new ActionListener()
      {
	public void actionPerformed(ActionEvent e)
        {
	  mysettings.setServiceUserName(ufield.getText());
	  mysettings.setServicePasswd(pfield.getPassword());
          exitOnDone = true;
	  splashf.dispose();
	}
      });
      /*
      * the cancel button isn't currently used; it's the same as
      * the OK button. But if the OK button actually did login
      * validation then we should enable this button again.
      *
          buttonPanel.add(cancelButton);
      */
      buttonPanel.add(exitButton);
      buttonPanel.add(okButton);
      promptPanel.add(buttonPanel, BorderLayout.SOUTH);
      splashp.add(promptPanel);
      splashf.getContentPane().add(splashp);
      splashf.setSize(380,170);
      splashf.pack();

      // all added, display the frame
      splashf.setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
      splashf.setVisible(true);
    }
    else
    {
      splashf = new JFrame("Jemboss Launch");
      exitOnDone = true;
    }
    splashf.setLocation(1,5);

  }

/**
*
* Called on startup to display the login and progress bar.
*
*/
  public AuthPopup(final JembossParams mysettings, int iprogressmax)
  {
     this(mysettings,null);
     this.mysettings = mysettings;
     this.iprogressmax = iprogressmax;
     
     //progress meter at startup
     if (iprogressmax > 0)
     {
       JPanel progressPanel = new JPanel();
       progressPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
       progressBar = new JProgressBar(0,iprogressmax);
       progressBar.setValue(0);
       progressLabel = new JLabel("Starting up.");
       progressPanel.add(progressBar);
       progressPanel.add(progressLabel);
       splashp.add(progressPanel,BorderLayout.SOUTH);
     }
    
    // add a border to the main pane to make is stand out
    splashp.setBorder(BorderFactory.createMatteBorder(2,2,2,2,Color.black));

    splashf.getContentPane().add(splashp);
    splashf.setSize(380,200);

    // all added, display the frame
    splashf.setDefaultCloseOperation(WindowConstants.HIDE_ON_CLOSE);
    splashf.setVisible(true);
  }

/**
*
* Update the progress bar and label
*
*/
  public void doneSomething(String s)
  {
    if (iprogressmax > 0)
    {
      if (iprogress < iprogressmax)
      {
        iprogress++;
        progressBar.setValue(iprogress);
      }
      progressLabel.setText(s);
      if (iprogress == iprogressmax)
      {
        progressLabel.setText("Startup complete.");
        if (exitOnDone)
          splashf.setVisible(false);
      }
    }
  }

/**
*
* Finish the progress bar and label
*
*/
  public void doneEverything(String s)
  {
    if (iprogressmax > 0)
    {
      progressBar.setValue(iprogressmax);
      progressLabel.setText(s);
      if (exitOnDone)
        splashf.setVisible(false);
    }
  }


  public JFrame getSplashFrame()
  {
    return splashf;
  }

}

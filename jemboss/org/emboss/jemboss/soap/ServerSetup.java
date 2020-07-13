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

package org.emboss.jemboss.soap;

import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.util.*;

import java.awt.event.*;
import org.emboss.jemboss.gui.MemoryComboBox;
import org.emboss.jemboss.JembossParams;


public class ServerSetup extends JTabbedPane 
{
  
  private MemoryComboBox publicURL;
  private MemoryComboBox privateURL;
  private MemoryComboBox publicName;
  private MemoryComboBox privateName;
  private MemoryComboBox proxyName;
  private MemoryComboBox proxyPort;

  private JCheckBox userAuth;
  private JCheckBox useProxy;

  private JembossParams myset; 

/**
*
* Setting for public and private settings panel.
* @param JembossParams SOAP parameters
*
*/
  public ServerSetup(JembossParams mysettings)
  {

    this.myset = mysettings;

    Vector PublicServerURL = new Vector();
    PublicServerURL.add(myset.getPublicSoapURL());

    Vector PrivateServerURL = new Vector();
    PrivateServerURL.add(myset.getPrivateSoapURL());

    Vector PublicServerName = new Vector();
    PublicServerName.add(myset.getPublicSoapService());

    Vector PrivateServerName = new Vector();
    PrivateServerName.add(myset.getPrivateSoapService());

    Vector proxyNameSettings = new Vector();
    proxyNameSettings.add(myset.getProxyHost());

    Vector proxyPortSettings = new Vector();
    proxyPortSettings.add(new Integer(myset.getProxyPortNum()));


//servers tabbed pane

    GridLayout gl = new GridLayout(6,1,6,6);
 
    JPanel general  = new JPanel(new BorderLayout());   
    JPanel jpWest   = new JPanel(gl);
    JPanel jpCenter = new JPanel(gl);

//public server
    JLabel lab = new JLabel("Public Server");
    jpWest.add(lab);
    publicURL = new MemoryComboBox(PublicServerURL);
    jpCenter.add(publicURL);

    lab = new JLabel("Public Service Name");
    jpWest.add(lab);
    publicName = new MemoryComboBox(PublicServerName);
    jpCenter.add(publicName);

//separator
    jpWest.add(new JLabel(""));
    jpCenter.add(new JLabel(""));

//private server
    lab = new JLabel("Private Server");
    jpWest.add(lab);
    privateURL = new MemoryComboBox(PrivateServerURL);
    jpCenter.add(privateURL);    

    lab = new JLabel("Private Service Name ");
    jpWest.add(lab);
    privateName = new MemoryComboBox(PrivateServerName);
    jpCenter.add(privateName);

//separator
    jpWest.add(new JLabel(""));
    jpCenter.add(new JLabel(""));

//authentication 
    userAuth = new JCheckBox("User authentication required by private server",
                              myset.getUseAuth());

    general.add(jpWest,BorderLayout.WEST);
    general.add(jpCenter,BorderLayout.CENTER);
    general.add(userAuth,BorderLayout.SOUTH);
    addTab("Servers",general);


//proxy tabbed pane

    gl = new GridLayout(6,1,6,6);
    JPanel proxy = new JPanel(new BorderLayout());
    useProxy = new JCheckBox("Use proxy settings",
                              myset.getUseProxy());

    jpWest = new JPanel(gl);
    jpCenter = new JPanel(gl);
    jpWest.add(useProxy);
    jpCenter.add(new JLabel(""));

    lab = new JLabel("Proxy ");
    jpWest.add(lab);
    proxyName = new MemoryComboBox(proxyNameSettings);
    jpCenter.add(proxyName);

    lab = new JLabel("Proxy Port");
    jpWest.add(lab);
    proxyPort = new MemoryComboBox(proxyPortSettings);
    jpCenter.add(proxyPort);

    proxyName.setEnabled(useProxy.isSelected());
    proxyPort.setEnabled(useProxy.isSelected());

    useProxy.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        proxyName.setEnabled(useProxy.isSelected());
        proxyPort.setEnabled(useProxy.isSelected());
        String settings[] = new String[1];
        settings[0] = new String("proxy.override=true");
        myset.updateJembossPropStrings(settings);
      }
    });
  
    jpWest.add(new JLabel(myset.proxyDescription()));

//separator
    jpWest.add(new JLabel(""));
    jpCenter.add(new JLabel(""));
    

    proxy.add(jpWest, BorderLayout.WEST);
    proxy.add(jpCenter, BorderLayout.CENTER);

    addTab("Proxies",proxy);


//client tabbed pane
    
    gl = new GridLayout(6,1,6,6);
    JPanel client = new JPanel(new BorderLayout());
    jpWest = new JPanel(gl);
    jpCenter = new JPanel(gl);

    jpWest.add(new JLabel("Java version  "));
    jpCenter.add(new JLabel(System.getProperty("java.version")));
     
    jpWest.add(new JLabel("Java home"));
    jpCenter.add(new JLabel(System.getProperty("java.home")));
     
    jpWest.add(new JLabel("User name "));
    jpCenter.add(new JLabel(System.getProperty("user.name")));
    
    jpWest.add(new JLabel("User home"));
    jpCenter.add(new JLabel(System.getProperty("user.home")));

    client.add(jpWest, BorderLayout.WEST);
    client.add(jpCenter, BorderLayout.CENTER);

    addTab("Client properties",client);
    
  }

  public JembossParams setNewSettings()
  {
    String settings[] = new String[8];
    settings[0] = new String("server.public="+
                             (String)publicURL.getSelectedItem());
    settings[1] = new String("server.private="+
                             (String)privateURL.getSelectedItem());
    settings[2] = new String("service.public="+
                             (String)publicName.getSelectedItem());
    settings[3] = new String("service.private="+
                             (String)privateName.getSelectedItem());

    if(userAuth.isSelected())
      settings[4] = new String("user.auth=true");
    else
      settings[4] = new String("user.auth=false");

    settings[6] = new String("proxy.host="+
                             (String)proxyName.getSelectedItem());
    settings[7] = new String("proxy.port="+
               ((Integer)proxyPort.getSelectedItem()).toString());

    if(useProxy.isSelected())
      settings[5] = new String("proxy.use=true");
    else
      settings[5] = new String("proxy.use=false");

    myset.updateJembossPropStrings(settings);

    return myset;
  }

}


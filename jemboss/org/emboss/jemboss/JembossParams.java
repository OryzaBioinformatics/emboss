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

import java.util.*;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.net.InetAddress;


public class JembossParams
{

// for SOAP requests via http  
//static public final int PROTOCOL_SOAP = 1;
// for requests using CORBA  
//static public final int PROTOCOL_CORBA = 2;
// for requests using Java RMI 
//static public final int PROTOCOL_RMI = 3;
// for requests using RMI over IIOP 
//static public final int PROTOCOL_RMI_IIOP = 4;
// for requests using Jini 
//static public final int PROTOCOL_JINI = 5;
// for requests using Jxta 
//static public final int PROTOCOL_JXTA = 6;

// denotes a server is OK 
  static public final int SERVER_OK = 0;
// denotes a server is giving errors  
  static public final int SERVER_ERR = 1;
// denotes a server is not responding 
  static public final int SERVER_DOWN = 2;

  // these are the things that could be set

  private boolean useProxy = false;
  private String useProxyName = "proxy.use";

  private String proxyHost = "wwwcache";
  private String proxyHostName = "proxy.host";

  private int proxyPortNum = 8080;
  private String proxyPortNumName = "proxy.port";

  private boolean useProxyAuth = false;
  private String useProxyAuthName = "proxy.auth";

  private String proxyAuthUser = "";
  private String proxyAuthUserName = "proxy.user";

  private String proxyAuthPasswd = "";
  private String proxyAuthPasswdName = "proxy.passwd";

  private boolean proxyOverride = false;
  private String proxyOverrideName = "proxy.override";

  private boolean useAuth = true;
  private String useAuthName = "user.auth";

  private String publicSoapURL = 
             "https://jemboss.hgmp.mrc.ac.uk:8443/soap/servlet/rpcrouter";
  private String publicSoapURLName = "server.public";

  private String privateSoapURL = 
             "https://jemboss.hgmp.mrc.ac.uk:8443/soap/servlet/rpcrouter";
  private String privateSoapURLName = "server.private";

  private String soapService = "EmbossSoap";
  private String soapServiceName = "service.name";

  private String privateSoapService = "JembossAuthServer";
  private String privateSoapServiceName = "service.private";

  private String publicSoapService = "JembossAuthServer";
  private String publicSoapServiceName = "service.public";

  //soap options
  private boolean debug = false;
  private String debugName = "jemboss.debug";

  private boolean hasBatchMode = true;
  private String hasBatchModeName = "jemboss.hasbatchmode";
  private boolean hasInteractiveMode = true;
  private String hasInteractiveModeName = "jemboss.hasinteractivemode";
  private String currentMode = "interactive";
  private String currentModeName = "jemboss.mode";
  
  // server lists for redundancy
  private String serverPublicList = "";
  private String serverPublicListName = "server.publiclist";

  private String serverPrivateList = "";
  private String serverPrivateListName = "server.privatelist";
  
  // we don't remember these perhaps we should for captive systems
  private String serviceUserName = "";
  private String serviceUserNameName = "user.name";
  private char[] servicePasswd = null;
  private byte[] servicePasswdByte = null;

  Properties jembossSettings;

  // Internal flags to help in the dynamic evaluation of properties
  private boolean useJavaProxy = false;
  private String javaProxyPort = "";
  private String javaProxyHost = "";
  private boolean useJavaNoProxy = false;
  private String javaNoProxy = "";
  private Vector javaNoProxyEntries;
  private int javaProxyPortNum = 8080;

  // structures for server redundancy
  private boolean publicServerFailOver = false;
  private boolean privateServerFailOver = false;
  private Hashtable serverStatusHash;
  private Vector publicServers;
  private Vector privateServers;

  /** Jemboss java server */
  private static boolean jembossServer = false;
  private String jembossServerName = "jemboss.server";

  //EMBOSS directories
  private String plplot = "/usr/local/share/EMBOSS/";
  private String plplotName = "plplot";
  private String embossData = "/usr/local/share/EMBOSS/data/";
  private String embossDataName = "embossData";
  private String embossBin = "/usr/local/bin/";
  private String embossBinName = "embossBin";
  private String embossPath = "/usr/bin/:/bin";
  private String embossPathName = "embossPath";
  private String embossEnvironment = "";
  private String embossEnvironmentName = "embossEnvironment";
  private String acdDirToParse = "/usr/local/share/EMBOSS/acd/";
  private String acdDirToParseName = "acdDirToParse";

  //EMBOSS Application pages
  private String embURL = "http://www.uk.embnet.org/Software/EMBOSS/Apps/";
  private String embossURL = "embossURL";

/**
*
* Loads and holds the properties
* @param name of the current application
*
*/
  public JembossParams() 
  {
    Properties defaults = new Properties();
    ClassLoader cl = this.getClass().getClassLoader();

    // initialize data structures
    serverStatusHash = new Hashtable();
    publicServers = new Vector();
    privateServers = new Vector();

    // initialize settings from table above
    defaults.put(embossURL,embURL);
    defaults.put(plplotName,plplot);
    defaults.put(embossDataName,embossData);
    defaults.put(embossBinName,embossBin);
    defaults.put(embossPathName,embossPath);
    defaults.put(embossEnvironmentName,embossEnvironment);
    defaults.put(acdDirToParseName,acdDirToParse);

    defaults.put(useProxyName, new Boolean(useProxy).toString());
    defaults.put(proxyHostName,proxyHost);
    defaults.put(proxyPortNumName, new Integer(proxyPortNum).toString());
    defaults.put(useProxyAuthName, new Boolean(useProxyAuth).toString());
    defaults.put(proxyAuthUserName, proxyAuthUser);
    defaults.put(proxyAuthPasswdName, proxyAuthPasswd);
    defaults.put(proxyOverrideName, new Boolean(proxyOverride).toString());
    defaults.put(useAuthName, new Boolean(useAuth).toString());
    defaults.put(publicSoapURLName, publicSoapURL);
    defaults.put(privateSoapURLName, privateSoapURL);
    defaults.put(privateSoapServiceName, privateSoapService);
    defaults.put(publicSoapServiceName, publicSoapService);
    defaults.put(debugName, new Boolean(debug).toString());
    defaults.put(hasBatchModeName, new Boolean(hasBatchMode).toString());
    defaults.put(hasInteractiveModeName, new Boolean(hasInteractiveMode).toString());
    defaults.put(currentModeName, currentMode);
    defaults.put(serverPublicListName, serverPublicList);
    defaults.put(serverPrivateListName, serverPrivateList);
    defaults.put(serviceUserNameName, serviceUserName);

    // load into real settings
    jembossSettings = new Properties(defaults);

    // try out of the classpath
    try
    {
      jembossSettings.load(cl.getResourceAsStream("resources/jemboss.properties"));
    } 
    catch (Exception e) 
    {
      if(debug)
        System.out.println("Didn't find properties file in classpath.");
    }

    // override with local system settings
    loadIn(System.getProperty("user.dir"));
    
    // override with local user settings
    loadIn(System.getProperty("user.home"));

    // update our settings
    updateSettingsFromProperties();

    // set up for overrides
    javaNoProxyEntries = new Vector();
    if(System.getProperty("proxyPort") != null) 
    {
      if(System.getProperty("proxyHost") != null)
      {
	useJavaProxy = true;
        useProxy = useJavaProxy;
	javaProxyPort = System.getProperty("proxyPort");
	javaProxyPortNum = Integer.parseInt(javaProxyPort);
	javaProxyHost = System.getProperty("proxyHost");
	if(System.getProperty("http.nonProxyHosts") != null) 
        {
	  useJavaNoProxy = true;
	  javaNoProxy = System.getProperty("http.nonProxyHosts");
	  StringTokenizer tok = new StringTokenizer(javaNoProxy,"|");
	  while (tok.hasMoreTokens()) 
          {
	    String toks = tok.nextToken() + "/";
	    javaNoProxyEntries.add(toks);
	  }
	}
      }
    }

  }

  private void loadIn(String folder)
  {
    FileInputStream in = null;
    try
    {
      String fs = System.getProperty("file.separator");
      in = new FileInputStream(folder + fs + "jemboss.properties");
      jembossSettings.load(in);
    }
    catch (java.io.FileNotFoundException e)
    {
      in = null;
      if(debug)
        System.out.println("Can't find properties file in"+folder+"."+
                           " Using defaults.");
    }
    catch (java.io.IOException e)
    {
      if(debug)
        System.out.println("Can't read properties file. " +
                           "Using defaults.");
    }
    finally
    {
      if (in != null)
      {
        try { in.close(); } catch (java.io.IOException e) { }
        in = null;
      }
    }

  }

  protected void updateSettingsFromProperties()
  {
    try
    {
      String tmp;

      embURL = jembossSettings.getProperty(embossURL);
      plplot = jembossSettings.getProperty(plplotName);
      embossData = jembossSettings.getProperty(embossDataName);
      embossBin = jembossSettings.getProperty(embossBinName);
      embossPath = jembossSettings.getProperty(embossPathName);
      embossEnvironment = jembossSettings.getProperty(embossEnvironmentName);
      acdDirToParse = jembossSettings.getProperty(acdDirToParseName);
      tmp = jembossSettings.getProperty(jembossServerName);
      jembossServer = new Boolean(tmp).booleanValue();

      tmp = jembossSettings.getProperty(useProxyName);
      useProxy = new Boolean(tmp).booleanValue();
      proxyHost = jembossSettings.getProperty(proxyHostName);
      tmp = jembossSettings.getProperty(proxyPortNumName);
      proxyPortNum = Integer.parseInt(tmp);
      tmp = jembossSettings.getProperty(useProxyAuthName);
      useProxyAuth = new Boolean(tmp).booleanValue();
      proxyAuthUser = jembossSettings.getProperty(proxyAuthUserName);
      proxyAuthPasswd = jembossSettings.getProperty(proxyAuthPasswdName);
      tmp = jembossSettings.getProperty(proxyOverrideName);
      proxyOverride = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(useAuthName);
      useAuth = new Boolean(tmp).booleanValue();
      publicSoapURL = jembossSettings.getProperty(publicSoapURLName);
      privateSoapURL = jembossSettings.getProperty(privateSoapURLName);
      soapService = jembossSettings.getProperty(soapServiceName);
      privateSoapService = jembossSettings.getProperty(privateSoapServiceName);
      publicSoapService = jembossSettings.getProperty(publicSoapServiceName);
      tmp = jembossSettings.getProperty(debugName);
      debug = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(hasBatchModeName);
      hasBatchMode = new Boolean(tmp).booleanValue();
      tmp = jembossSettings.getProperty(hasInteractiveModeName);
      hasInteractiveMode = new Boolean(tmp).booleanValue();
      currentMode = jembossSettings.getProperty(currentModeName);
      serverPublicList = jembossSettings.getProperty(serverPublicListName);
      serverPrivateList = jembossSettings.getProperty(serverPrivateListName);
//    serviceUserName = jembossSettings.getProperty(serviceUserNameName);
    } 
    catch (Exception e) {  }
  }

/**
*
* Initialize the server redundancy data. This is a separate
* method because the server info might not be initialized in
* the constructor, but may be imported later from the command line.
*
*/
  protected void setupServerRedundancy() 
  {
    if (!serverPublicList.equals("")) 
    {
      if(debug) 
	System.out.println("JembossParams: Redundant public servers\n  "
			   +serverPublicList);
  
      publicServerFailOver = true;
      StringTokenizer tok = new StringTokenizer(serverPublicList,"|");
      while (tok.hasMoreTokens()) 
      {
	String toks = tok.nextToken();
	publicServers.add(toks);
	if(debug) 
	  System.out.println(" Entry " + toks);
	
	serverStatusHash.put(toks, new Integer(SERVER_OK));
      }
    }

    if(!serverPrivateList.equals("")) 
    {
      if(debug) 
	System.out.println("JembossParams: Redundant private servers\n  "
			   +serverPrivateList);
      
      privateServerFailOver = true;
      StringTokenizer tok = new StringTokenizer(serverPrivateList,"|");
      while (tok.hasMoreTokens()) 
      {
	String toks = tok.nextToken();
	privateServers.add(toks);
	if(debug) 
	  System.out.println(" Entry " + toks);
	
	serverStatusHash.put(toks,new Integer(SERVER_OK));
      }
    }
  }


/**
*
* If using a proxy server
*
*/
  public boolean getUseProxy() 
  {
    return useProxy;
  }

/**
*
* If using a proxy server for a given URL
* @param s The URL we wish to connect to
*
*/
  public boolean getUseProxy(String s) 
  {
    if(proxyOverride) 
      return useProxy;
    else 
    {
      if(!useJavaProxy) 
	return useProxy;
      else 
      {
	boolean jp = true;
	if (useJavaNoProxy) 
        {
	  int ip = javaNoProxyEntries.size();
	  for(int j = 0 ; j<ip ; ++j) 
	    if(s.indexOf((String)javaNoProxyEntries.get(j).toString()) != -1) 
	      jp = false;
	}
	return jp;
      }
    }
  }

/**
*
* The name of the proxy server
*
*/
  public String getProxyHost() 
  {
    if (proxyOverride) 
      return proxyHost;
    else 
    {
      if(!useJavaProxy) 
	return proxyHost;
      else 
	return javaProxyHost;
    }
  }

/**
*
* The port the proxy server listens on
*
*/
  public int getProxyPortNum() 
  {
    if(proxyOverride) 
      return proxyPortNum;
    else
    { 
      if(!useJavaProxy) 
	return proxyPortNum;
      else
	return javaProxyPortNum;
    }
  }

/**
*
* If using authenticate with the proxy 
*
*/
  public boolean getUseProxyAuth() 
  {
    return useProxyAuth;
  }

/**
*
* username needed to use for the proxy server
*
*/
  public String getProxyAuthUser() 
  {
    return proxyAuthUser;
  }

/**
*
* password needed to use for the proxy server
*
*/
  public String getProxyAuthPasswd() 
  {
    return proxyAuthPasswd;
  }

/**
*
* A description of the proxy settings
*
*/
  public String proxyDescription() 
  {
    String pdesc = "";
    if (proxyOverride) 
    {
      if(useProxy)
      {
	String spnum = new Integer(proxyPortNum).toString();
	pdesc = "Current Settings: " + "Proxy Host: " + proxyHost + 
                                           " Proxy Port: " + spnum;
      } 
      else 
	pdesc = "No proxies, connecting direct.";
    } 
    else 
    {
      if (useJavaProxy) 
      {
	pdesc = "Settings Imported from Java: " + "Proxy Host: " + javaProxyHost
	                                      + " Proxy Port: " + javaProxyPort;
	if(useJavaNoProxy) 
	  pdesc = pdesc + "\nNo Proxy On: " + javaNoProxy;
      } 
      else
      {
	if(useProxy) 
        {
	  String spnum = new Integer(proxyPortNum).toString();
	  pdesc = "Current Settings: " + "Proxy Host: " + proxyHost + 
                                             " Proxy Port: " + spnum;
	} 
        else 
	  pdesc = "No proxies, connecting direct.";
      }
    }
    return pdesc;
  }

/**
*
* Whether the main service requires authentication
*
*/
  public boolean getUseAuth() 
  {
    return useAuth;
  }

/**
*
* Returns the URL of the public soap server
*
*/
  public String getPublicSoapURL() 
  {
    return publicSoapURL;
  }


  public static boolean isJembossServer()
  {
    return jembossServer;
  }

  public String getPlplot()
  {
    return plplot;
  }

  public String getembURL()
  {
    return embURL;
  }

  public String getEmbossData()
  {
    return embossData;
  }
  
  public String getEmbossBin()
  {
    return embossBin;
  }

  public String getEmbossPath()
  {
    return embossPath;
  }

  public String getEmbossEnvironment()
  {
    embossEnvironment = embossEnvironment.trim();
    embossEnvironment = embossEnvironment.replace(':',' ');
    embossEnvironment = embossEnvironment.replace(',',' ');
    return embossEnvironment;
  }

  public String[] getEmbossEnvironmentArray(String[] envp)
  {
    embossEnvironment = embossEnvironment.trim();
    embossEnvironment = embossEnvironment.replace(':',' ');
    embossEnvironment = embossEnvironment.replace(',',' ');

    if(embossEnvironment.equals(""))
      return envp;

    StringTokenizer st = new StringTokenizer(embossEnvironment," ");
    int n=0;
    while(st.hasMoreTokens())
    {
      st.nextToken();
      n++;
    }
    
    int sizeEnvp = envp.length;
    String environ[] = new String[n+sizeEnvp];
    st = new StringTokenizer(embossEnvironment," ");
    for(int i=0;i<sizeEnvp;i++)
      environ[i] = envp[i];

    n=sizeEnvp;
    while(st.hasMoreTokens())
    {
      environ[n] = new String(st.nextToken()); 
      n++;
    }

    return environ;
  }

  public String getAcdDirToParse()
  {
    return acdDirToParse;
  }

/**
*
* Set the URL of the public soap server
*
*/
  public void setPublicSoapURL(String s) 
  {
    publicSoapURL = s;
  }

/**
*
* Returns the URL of the private soap server
*
*/
  public String getPrivateSoapURL() 
  {
    return privateSoapURL;
  }

/**
*
* Set the URL of the private soap server
*
*/
  public void setPrivateSoapURL(String s) 
  {
    privateSoapURL = s;
  }

/**
*
* Return whether we have failover on the public server
*
*/
  public boolean getPublicServerFailover() 
  {
    return publicServerFailOver;
  }

/**
*
* Return whether we have failover on the private server
*
*/
  public boolean getPrivateServerFailover() 
  {
    return privateServerFailOver;
  }

/**
*
* Return a vector containing the list of public servers
*
*/
  public Vector getPublicServers() 
  {
    return publicServers;
  }

/**
*
* Return a vector containing the list of private servers
*
*/
  public Vector getPrivateServers() 
  {
    return privateServers;
  }

/**
*
* Mark a server as bad
*
*/
  public void setServerStatus(String server, int i) 
  {
    serverStatusHash.put(server, new Integer(i));
  }

/**
*
* Return the username needed for the remote service
*
*/
  public String getServiceUserName() 
  {
    return serviceUserName;
  }

/**
*
* Save the username needed for the remote service
* @param newUserName  The username
*
*/
  public void setServiceUserName(String newUserName)
  {
    serviceUserName = newUserName;
  }

/**
*
* Return the password needed for the remote service
*
*/
  public char[] getServicePasswd() 
  {
    return servicePasswd;
  }


  public byte[] getServicePasswdByte()
  {
    return servicePasswdByte;
  }

/**
*
* Return the password as byte array
*
*/
  private static byte[] toByteArr(char ch[])
  {
    int len = ch.length;
    byte msb[] = new byte[len];

    for(int i=0;i<len;i++)
      msb[i] = (byte)(ch[i]);
    return msb;
  }


/**
*
* Save the password needed for the remote service
* @param newPasswd  The username
*
*/
  public void setServicePasswd(char[] newPasswd) 
  {
    int csize = newPasswd.length;
    servicePasswd = new char[csize];

    for(int i=0;i<csize;i++)
      servicePasswd[i] = newPasswd[i];

    servicePasswdByte = toByteArr(newPasswd);
  }

/**
*
* Get the name of the soap service we're using
*
*/
  public String getSoapService() 
  {
    return soapService;
  }

/**
*
* Get the name of the private soap service we're using
*
*/
  public String getPrivateSoapService() 
  {
    return privateSoapService;
  }

/**
*
* Set the name of the private soap service we're using
* @param s  The name of the service
*
*/
  public void setPrivateSoapService(String s) 
  {
    privateSoapService = s;
  }

/**
*
* Get the name of the public soap service we're using
*
*/
  public String getPublicSoapService() 
  {
    return publicSoapService;
  }

/**
*
* Set the name of the public soap service we're using
* @param s  The name of the service
*
*/
  public void setPublicSoapService(String s) 
  {
    publicSoapService = s;
  }

/**
*
* A description of the server settings
*
*/
  public String serverDescription() 
  {
    String serverdesc = "Current Settings:"
      +"\nPublic Server: "+publicSoapURL
      +"\nPrivate Server: "+privateSoapURL
      +"\nPublic SOAP service: "+publicSoapService
      +"\nPrivate SOAP service: "+privateSoapService;
    return serverdesc;
  }

/**
*
* Whether to show debugging information
*
*/
  public boolean getDebug() 
  {
    return debug;
  }

/**
*
* Whether this service supports batch mode
*
*/
  public boolean getHasBatchMode() 
  {
    return hasBatchMode;
  }

/**
*
* Whether this service supports interactive mode
*
*/
  public boolean getHasInteractiveMode() 
  {
    return hasInteractiveMode;
  }

/**
*
* The current mode (interactive or batch).
* @return a String containing the current mode
*
*/
  public String getCurrentMode() 
  {
    if(hasInteractiveMode) 
    {
      if (hasBatchMode) 
	return currentMode;
      else 
	return "interactive";
    } 
    else
    {
      if (hasBatchMode) 
	return "batch";
      else 
	return currentMode;
    }
  }

/**
*
* Set the current mode (interactive or batch). 
* @param newMode The new execution mode
*
*/
  public void setCurrentMode(String newMode) 
  {
    currentMode = newMode;
  }

/**
*
* Return the mode list as a vector, suitable for loading into
* a combobox.
*
*/
  public Vector modeVector() 
  {
    Vector mv = new Vector();
    if (hasInteractiveMode) 
    {
      if (hasBatchMode) 
      {
	if (currentMode.equals("interactive")) 
        {
	  mv.add("interactive");
	  mv.add("batch");
	} 
        else if (currentMode.equals("batch")) 
        {
	  mv.add("batch");
	  mv.add("interactive");
	} 
        else 
        {
	  mv.add(currentMode);
	  mv.add("interactive");
	  mv.add("batch");
	}
      } 
      else 
	mv.add("interactive");
    } 
    else 
    {
      if(hasBatchMode) 
	mv.add("batch");
    }
    return mv;
  }

/**
*
* Update the properties structure. 
* This doesn't update the actual properties, just the Properties object
* so you must call updateSettingsFromProperties yoursefl
*
* @param  name   A String naming the property to be updated
* @param  newvalue  A String containing the new value of the property
*
*/
  public void updateJembossProperty(String name, String newvalue) 
  {
    if (jembossSettings.getProperty(name) != null) 
      jembossSettings.setProperty(name,newvalue);
  }

/**
*
* Parse a key=value string to update the properties structure
*
* @param entry String containing a key=value message
*
*/
  public void updateJembossPropString(String entry) 
  {
    int isep = entry.indexOf('=');
    if(isep != -1)
    {
      String pkey = entry.substring(0,isep);
      String pvalue = entry.substring(isep+1);
      updateJembossProperty(pkey, pvalue);
    }
  }

/**
*
* Parse an array of key=value strings to update the properties structure
* @param entries Array of Strings containing key=value messages
*
*/
  public void updateJembossPropStrings(String[] entries) 
  {
    for (int i=0; i < entries.length; ++i) 
      updateJembossPropString(entries[i]);
    
    updateSettingsFromProperties();
    setupServerRedundancy();
  }

/**
*
* Update properties from a Hashtable
*
* @param hash Hashtable containg properties
*
*/
  public void updateJembossPropHash(Hashtable hash) 
  {
    Enumeration enum = hash.keys();
    while(enum.hasMoreElements()) 
    {
      String thiskey = (String)enum.nextElement().toString();
      String thisval = (String)hash.get(thiskey);
      updateJembossProperty(thiskey,thisval);
    }
    updateSettingsFromProperties();
  }

}


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
*  Based on EmbreoResList
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.io.*;
import java.util.*;

import org.emboss.jemboss.JembossParams;

import java.net.*;
import org.w3c.dom.*;
import org.xml.sax.*;
import javax.xml.parsers.*;
import javax.mail.*;
import org.apache.soap.util.xml.*;
import org.apache.soap.*;
import org.apache.soap.encoding.*;
import org.apache.soap.encoding.soapenc.*;
import org.apache.soap.rpc.*;
import org.apache.soap.transport.http.SOAPHTTPConnection;

public class PrivateRequest 
{

  private Hashtable proganswer;
  private String result = "";
  private boolean successful = false;
  private boolean authenticationerror = false;


/**
*
* Make a soap call to a private server, using the default service
* @param mysettings JembossParams defining server parameters
* @param method     String defining which method to call
* @param args       Vector of arguments
* @throws JembossSoapException If authentication fails
*
*/
   public PrivateRequest(JembossParams mysettings, String method, Vector args) 
                throws JembossSoapException 
   {
     this(mysettings, mysettings.getPrivateSoapService(), method, args);
   }

/**
*
* Make a soap call to a private server
* @param mysettings JembossParams defining server parameters
* @param service    String defining which service to call
* @param method     String defining which method to call
*
* @throws JembossSoapException If authentication fails
*/
   public PrivateRequest(JembossParams mysettings, String service, String method)
                throws JembossSoapException
   {
     this(mysettings, service, method, (Vector) null);
   }

/**
*
* Make a soap call to a private server
* @param mysettings JembossParams defining server parameters
* @param service    String defining which service to call
* @param method     String defining which method to call
* @param args       Vector of arguments
*
* @throws JembossSoapException If authentication fails
*/
   public PrivateRequest(JembossParams mysettings, String service, String method,
                         Vector args) throws JembossSoapException 
   {

     if (mysettings.getDebug()) 
     {
       System.out.println("PrivateRequest: Invoked, parameters are");
       System.out.println("  Server:  " + mysettings.getPrivateSoapURL());
       System.out.println("  Service: " + service);
       System.out.println("  Method:  " + method);
     }

     String soapURLName;
     URL proglisturl;
     soapURLName = mysettings.getPrivateSoapURL();
     try 
     {
       proglisturl = new URL(soapURLName);
     }
     catch (Exception e) 
     { 
       System.err.println("PrivateRequest: While Initialising URL Caught Exception (" +
			  e.getMessage ());
       return;
     }


     SOAPHTTPConnection proglistconn = new SOAPHTTPConnection();

     // if proxy, set the proxy values
     if (mysettings.getUseProxy(soapURLName) == true) 
     {
       if (mysettings.getDebug()) 
	 System.out.println("PrivateRequest: Using proxy");
       
       if (mysettings.getProxyHost() != null)
       {
	 proglistconn.setProxyHost(mysettings.getProxyHost());
	 proglistconn.setProxyPort(mysettings.getProxyPortNum());
       }
     }

     
     if(JembossParams.isJembossServer())  //JembossServer.java servers
     {
       if(mysettings.getUseAuth() == true)
       {
         if(args == null)
           args = new Vector();
         args.addElement(new Parameter("user", String.class,
                    mysettings.getServiceUserName(), null));
   
         args.addElement(new Parameter("p",  byte[].class,
                         mysettings.getServicePasswdByte(), null));

//       args.addElement(new Parameter("p", String.class,
//                 mysettings.getServicePasswd(), null));
       }
       else       //No authorization reqd, so use user name here
       {          //to create own sand box on server

         if(args == null)
           args = new Vector();
         String userName = System.getProperty("user.name");
         args.addElement(new Parameter("USERNAME", String.class,
                                      userName, null));
       }
     }
     else         //cgi server at HGMP, add authentication headers
     {
       proglistconn.setUserName(mysettings.getServiceUserName());
       proglistconn.setPassword(new String(mysettings.getServicePasswd()));
     }


     Call proglistcall = new Call();
     proglistcall.setSOAPTransport(proglistconn);
     proglistcall.setTargetObjectURI(service);
     proglistcall.setMethodName(method);
     proglistcall.setEncodingStyleURI(Constants.NS_URI_SOAP_ENC);


     if(args != null) 
       proglistcall.setParams(args);
     
     Response proglistresp = null;
     try 
     {
       proglistresp = proglistcall.invoke(proglisturl,null);
       successful = true;
     }
     catch(SOAPException e) 
     {
       System.err.println("PrivateRequest: Caught SOAPException (" +
			  e.getFaultCode () + "): " +
			  e.getMessage ());
       Hashtable efaulth = proglistconn.getHeaders();
       
       if((efaulth != null) && efaulth.containsKey("WWW-Authenticate")) 
       {
	 if (mysettings.getDebug()) 
	   System.out.println("PrivateRequest: Auth header found!");
	 throw new JembossSoapException("Authentication Failed");
       } 
       else 
       {
	 mysettings.setServerStatus(soapURLName, JembossParams.SERVER_DOWN);
	 if (mysettings.getPrivateServerFailover()) 
         {
	   if (mysettings.getDebug()) 
	     System.out.println("PrivateRequest: trying server failover");
	   Vector servlist = mysettings.getPrivateServers();
	   int iserv = servlist.size();
	 }
       }
     }

     if (authenticationerror) 
     {
       successful = true;
       throw new JembossSoapException("Authentication Failed");
     }
     
     // must do something more intelligent to aid recovery
     if (!successful) 
     {
       System.err.println("PrivateRequest: Failed.");
       return;
     }


    if (!proglistresp.generatedFault())   //check response
    {
      Parameter progret = proglistresp.getReturnValue();
      Object progvalue = progret.getValue();
      result = progvalue.toString();

      Vector progans = proglistresp.getParams();
      Parameter progansp;
      String tstr;

      // only if we have more data
      if (progans != null) 
      {
	progansp = (Parameter) progans.get(0);
	int progians = progans.size();
	proganswer = new Hashtable(progians);

	tstr = (String)progvalue;
	proganswer.put((String)progvalue,progansp.getValue());
	for (int j=1; j<progians;j++)
        {
	  Parameter progansk = (Parameter) progans.get(j);
	  tstr = progansk.getValue().toString();
	  j++;
	  Parameter progansv = (Parameter) progans.get(j);
	  proganswer.put(progansk.getValue().toString(),progansv.getValue());
	}
      } 
      else         // what do we get back - Vector or Hashtable
      {
	if(progvalue.getClass().equals(Hashtable.class)) 
        {
	  proganswer = (Hashtable)progvalue;
	}
        else if(progvalue.getClass().equals(Vector.class)) 
        {
	  proganswer = new Hashtable();
	  Vector vans = (Vector)progvalue;

	  int n = vans.size();
	  for(int j=0;j<n;j+=2)  //assumes it's even sized
          {
            if(vans.get(j).equals("msg"))
              if(((String)vans.get(j+1)).startsWith("Failed Authorisation"))
                throw new JembossSoapException("Authentication Failed");

	    proganswer.put(vans.get(j),vans.get(j+1));
          }
        }
      }

    } 
    else
    {
      Fault fault = proglistresp.getFault();
      System.err.println("Generated fault: ");
      System.out.println("  Fault Code   = " + fault.getFaultCode());
      System.out.println("  Fault String = " + fault.getFaultString());
      throw new JembossSoapException("  Fault Code   = " + fault.getFaultCode());
    }


   }


/**
*
* @return true if the call succeeded (eventually) or not
*
*/
  public boolean succeeded() 
  {
    return successful;
  }

/**
*
* Gets an element out of the embreo result hash
* @param val The key to look up
* @return Either the element, or an empty String if there is no
* an element that matches the key
*
*/
  public Object getVal(String val) 
  {
    if (proganswer.containsKey(val)) 
      return proganswer.get(val);
    else 
      return "";
  }

/**
*
* @return The result of the soap server call. This is the first element that
* is returned. Only useful for server methods that return a simple value.
*
*/
  public String getResult() 
  {
    return result;
  }

/**
*
* @return Hahtable of results returned by the server call. 
*
*/
  public Hashtable getHash() 
  {
    return proganswer;
  }
}


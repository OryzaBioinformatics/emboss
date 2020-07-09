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
*  Based on EmbreoPublicRequest
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.soap;

import java.io.*;
import java.util.*;

import org.emboss.jemboss.JembossParams;

//SOAP 
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

public class PublicRequest
{

  private Hashtable proganswer;
  private String result = "";
  private boolean successful = false;

/**
* Makes a soap call to a public server, using the default service
*
* @param mysettings JembossParams defining server parameters
* @param method     String defining which method to call
*/
   public PublicRequest(JembossParams mysettings, String method)
   {
     this(mysettings, mysettings.getPublicSoapService(), method);
   }

/**
* Makes a soap call to a public server, using the default service
*
* @param mysettings JembossParams defining server parameters
* @param method     String defining which method to call
* @param args       Vector of arguments
*/
   public PublicRequest(JembossParams mysettings, String method, Vector args)
   {
     this(mysettings, mysettings.getPublicSoapService(), method, args);
   }

/**
* Makes a soap call to a public server
*
* @param mysettings JembossParams defining server parameters
* @param service    String defining which service to call
* @param method     String defining which method to call
*/
   public PublicRequest(JembossParams mysettings, String service, String method)
   {
     this(mysettings, service, method, (Vector) null);
   }

/**
* Makes a soap call to a public server
*
* @param mysettings JembossParams defining server parameters
* @param service    String defining which service to call
* @param method     String defining which method to call
* @param args       Vector of arguments
*/
   public PublicRequest(JembossParams mysettings, String service, String method, Vector args)
   {

     if (mysettings.getDebug())
     {
       System.out.println("PublicRequest: Invoked, parameters are");
       System.out.println("  Server:  " + mysettings.getPublicSoapURL());
       System.out.println("  Service: " + service);
       System.out.println("  Method:  " + method);
     }

     String soapURLName;
     URL proglisturl;
     soapURLName = mysettings.getPublicSoapURL();
     try
     {
       proglisturl = new URL(soapURLName);
     } 
     catch (Exception e) 
     { 
       System.err.println("PublicRequest: While Initialising URL Caught Exception (" +
			  e.getMessage ());
       return;
     }
     int i;

     SOAPHTTPConnection proglistconn = new SOAPHTTPConnection();
     
     // if proxy, set the proxy values
     if (mysettings.getUseProxy(soapURLName) == true)
     {
       if (mysettings.getDebug())
	 System.out.println("PublicRequest: Using proxy");
       
       if (mysettings.getProxyHost() != null) 
       {
	 proglistconn.setProxyHost(mysettings.getProxyHost());
	 proglistconn.setProxyPort(mysettings.getProxyPortNum());
       }
     }
     

     Call proglistcall = new Call();
     proglistcall.setSOAPTransport(proglistconn);
     proglistcall.setTargetObjectURI(service);
     proglistcall.setMethodName(method);
     proglistcall.setEncodingStyleURI(Constants.NS_URI_SOAP_ENC);

     if (args != null) 
       proglistcall.setParams(args);

     Response proglistresp = null;
     try
     {
       proglistresp = proglistcall.invoke(proglisturl,null);
       successful = true;
     } 
     catch(SOAPException e) 
     {
       mysettings.setServerStatus(soapURLName, JembossParams.SERVER_DOWN);
       System.err.println("PublicRequest: Caught SOAPException (" +
			  e.getFaultCode () + "): " +
			  e.getMessage ());
       if (mysettings.getPublicServerFailover()) 
       {
	 if (mysettings.getDebug())
	   System.out.println("PublicRequest: trying server failover");
	 
	 Vector servlist = mysettings.getPublicServers();
	 int iserv = servlist.size();
	 for ( int j = 0 ; j < iserv; j++) 
         {
	   String newurl = (String)servlist.get(j);
	   System.out.println("PublicRequest: trying " + newurl);
	   try 
           {
	     proglisturl = new URL(newurl);
	     try 
             {
	       proglistresp = proglistcall.invoke(proglisturl,null);
	       System.out.println("PublicRequest: success!");
	       mysettings.setServerStatus(newurl, JembossParams.SERVER_OK);
	       mysettings.setPublicSoapURL(newurl);
	       successful = true;
	       break;
	     } 
             catch (SOAPException e2) 
             {
	       mysettings.setServerStatus(newurl, JembossParams.SERVER_DOWN);
	       System.err.println("PublicRequest: Caught SOAPException (" +
			  e2.getFaultCode () + "): " +
			  e2.getMessage ());
	     }
	   }
           catch (Exception e3) 
           {
	     System.err.println("PublicRequest: While Initialising URL Caught Exception (" +
			  e3.getMessage ());
	   }
	 }
       }
     }

     // do something more intelligent to aid recovery
     if (!successful) 
     {
       System.err.println("PublicRequest: Failed.");
       return;
     }

    //Check response
    if (!proglistresp.generatedFault()) 
    {
      Parameter progret = proglistresp.getReturnValue();
      Object progvalue = progret.getValue();
      result = progvalue.toString();

      Vector progans = proglistresp.getParams();
      Parameter progansp;
      String tstr;

      // only iterate if we have more data
      if (progans != null) 
      {
	progansp = (Parameter) progans.get(0);
	int progians = progans.size();
	proganswer = new Hashtable(progians);

	tstr = (String)progvalue;
	proganswer.put((String)progvalue,progansp.getValue().toString());
	for (int j=1; j<progians;j++) {
	  Parameter progansk = (Parameter) progans.get(j);
	  tstr = progansk.getValue().toString();
	  j++;
	  Parameter progansv = (Parameter) progans.get(j);
	  proganswer.put(progansk.getValue().toString(),progansv.getValue().toString());
	}
      }
      else
      {
	// what do we get back - Vector or Hashtable
	if (progvalue.getClass().equals(Hashtable.class)) 
        {
	  proganswer = (Hashtable)progvalue;
	} 
        else if(progvalue.getClass().equals(Vector.class)) 
        {
	  proganswer = new Hashtable();
	  Vector vans = (Vector)progvalue;
	  // assumes it's even sized
	  int n = vans.size();
	  for(int j=0;j<n;j+=2)
	    proganswer.put(vans.get(j),vans.get(j+1));
	}
      }

    } 
    else 
    {
      Fault fault = proglistresp.getFault();
      System.err.println("Generated fault: ");
      System.out.println("  Fault Code   = " + fault.getFaultCode());
      System.out.println("  Fault String = " + fault.getFaultString());
    }

   }

/**
* Whether the call succeeded (eventually) or not
*/
  public boolean succeeded() 
  {
    return successful;
  }

/**
* Gets an element out of the embreo result hash
*
* @param val The key to look up
*
* @return Either the element, or an empty String if there isn't
* an element that matches the key
*/
  public String getVal(String val)
  {
    if (proganswer.containsKey(val)) 
      return (String)proganswer.get(val);
    else 
      return "";
  }

/**
* The result of the embreo call. This is the first element that
* is returned. Only useful for server methods that return a simple value.
*/
  public String getResult() 
  {
    return result;
  }

/**
* The hash returned by the embreo call. We either return a hash or
* a simple String.
*/
  public Hashtable getHash() 
  {
    return proganswer;
  }

}

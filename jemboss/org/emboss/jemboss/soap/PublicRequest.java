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

//AXIS
import org.apache.axis.client.Call;
import org.apache.axis.client.Service;
import javax.xml.namespace.QName;
import org.apache.axis.encoding.XMLType;

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
               throws JembossSoapException
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
               throws JembossSoapException
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
               throws JembossSoapException
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
   public PublicRequest(JembossParams mysettings, String service, 
                        String method, Vector args)
               throws JembossSoapException
   {

     try
     {
       String  endpoint = mysettings.getPublicSoapURL();
       org.apache.axis.client.Service serv = 
                        new org.apache.axis.client.Service();

       Call    call     = (Call) serv.createCall();
       call.setTargetEndpointAddress( new java.net.URL(endpoint) );
       call.setOperationName(new QName(service, method));

       Object params[] = null;
       if(args != null)
       {
         params = new Object[args.size()];
         Enumeration e = args.elements();
         for(int i=0;i<args.size();i++)
         {
           Object obj = e.nextElement();
           if(obj.getClass().equals(String.class))
           {
             params[i] = (String)obj;
             call.addParameter("Args", XMLType.XSD_STRING,
                             javax.xml.rpc.ParameterMode.IN);
           }
           else
           {
             params[i] = (byte[])obj;
             call.addParameter("Args", XMLType.XSD_BYTE,
                             javax.xml.rpc.ParameterMode.IN);   
           }
         }
       }
       call.setReturnType(org.apache.axis.Constants.SOAP_VECTOR);

       Vector vans;
       if(args != null)
         vans = (Vector)call.invoke( params );
       else
         vans = (Vector)call.invoke( new Object[] {});

       proganswer = new Hashtable();
       // assumes it's even sized
       int n = vans.size();
       for(int j=0;j<n;j+=2)
       {
         String s = (String)vans.get(j);
         proganswer.put(s,vans.get(j+1));
       }
     } 
     catch (Exception e) 
     {
       throw new JembossSoapException("Connection failed");
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

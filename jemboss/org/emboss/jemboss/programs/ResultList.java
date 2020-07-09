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

package org.emboss.jemboss.programs;

import java.io.*;
import java.util.*;

import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.JembossParams;

import org.apache.soap.rpc.*;

public class ResultList 
{

  private String statusmsg;
  private String status;
  private Hashtable proganswer;
  private String currentRes = null;

/**
*
* Holds the list of stored results
* @param mysettings JembossParams defining server parameters
* @throws JembossSoapException If authentication fails
*
*/
  public ResultList(JembossParams mysettings) throws JembossSoapException 
  {
    this(mysettings,null,"list_saved_results");
  }

/**
*
* Manipulate a dataset.
* @param mysettings JembossParams defining server parameters
* @param dataset    Which dataset to manipulate
* @param methodname What method to invoke on this dataset
*
*/
   public ResultList(JembossParams mysettings, String dataset, 
                              String methodname) throws JembossSoapException 
   {

     String options = "";
     PrivateRequest eRun;

     Vector params = new Vector();

     if(dataset != null) 
     {
       params.addElement(new Parameter("dataset", String.class,
                                    dataset, null));
       params.addElement(new Parameter("options", String.class,
                                    options, null));
     }

     try 
     {
       eRun = new PrivateRequest(mysettings,methodname, params);
     } 
     catch (JembossSoapException e) 
     {
       throw new JembossSoapException("Authentication Failed");
     }

     proganswer = eRun.getHash();
     status = proganswer.get("status").toString();
     statusmsg = proganswer.get("msg").toString();
     
     proganswer.remove("status");      //delete out of the hash
     proganswer.remove("msg");

   }

/**
*
* The status of the request
* @return String 0 for success, anything else for failure
*
*/
  public String getStatus() 
  {
    return status;
  }

/**
*
* A status message
* @return A string containing a status message. In the case of an error,
* contains a description of the error.
*
*/
  public String getStatusMsg() 
  {
    return statusmsg;
  }
  
/**
*
* @return Hashtable of the results
*
*/
  public Hashtable hash() 
  {
    return proganswer;
  }

/**
*
* @return Enumeration of the elements of the results Hashtable
*
*/
  public Enumeration elements() 
  {
    return proganswer.elements();
  }

/**
*
* @return Enumeration of the keys of the results Hashtable
*
*/
  public Enumeration keys() 
  {
    return proganswer.keys();
  }

/**
*
* @param Object key of the element to return
* @returns the element in the result Hashtable defined by the key
*
*/
  public Object get(Object key) 
  {
    return proganswer.get(key);
  }

/**
*
* @return String of the current dataset being looked at
*
*/
  public String getCurrent() 
  {
    return(currentRes);
  }

/**
*
* Save the name of a dataset, marking it as the current dataset
* @param s  The name of the dataset
*
*/
  public void setCurrent(String s) 
  {
    currentRes = s;
  }

/**
*
* Replace the current results hash
* @param newres  The new results hash
*
*/
  public void updateRes(Hashtable newres)
  {
    proganswer = newres;
    if (currentRes != null) 
    {
      if (!proganswer.containsKey(currentRes)) 
	currentRes = null;
    }
  }

}


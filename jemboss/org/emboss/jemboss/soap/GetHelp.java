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
import java.util.*;

import org.emboss.jemboss.JembossParams;
import org.apache.soap.rpc.*;

public class GetHelp 
{

  private String statusmsg;
  private String status;
  private String helpText;

/**
*
* Get the help text for an application
* @param acdProg name of the application to get the help text for
* @param mysettings JembossParams defining server parameters
*
*/
   public GetHelp(String acdProg, JembossParams mysettings)
   {

     Vector params = new Vector();

     params.addElement(new Parameter("option", String.class,
                                    acdProg, null));
     PublicRequest hReq = new PublicRequest(mysettings, "show_help", params);
     helpText = hReq.getVal("helptext");
     status = hReq.getVal("status");
     statusmsg = hReq.getVal("msg");

   }

/**
*
* The status of the request
* @return String 0 for success, anything else for failure. 
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
* @return String help text 
*
*/
  public String getHelpText() 
  {
    return helpText;
  }

}


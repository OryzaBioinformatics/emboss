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

public class ShowDB 
{

  private String statusmsg;
  private String status;
  private String dbText;

/**
*
* Makes a soap call to retrieve the showdb output
* @param mysettings JembossParams defining server parameters
*
*/
  public ShowDB(JembossParams mysettings)
  {
    
    PublicRequest dbReq = new PublicRequest(mysettings, "show_db");
    statusmsg = dbReq.getVal("msg");
    status = dbReq.getVal("status");
    dbText= dbReq.getVal("showdb");
    
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
* @return string containing a status message, contains
* description of any error.
*
*/
  public String getStatusMsg() 
  {
    return statusmsg;
  }

/**
*
* The output from showdb
*
* @return  output from showdb
*
*/
  public String getDBText() 
  {
    return dbText;
  }
}

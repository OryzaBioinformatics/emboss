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


public class GetWossname 
{

   private String statusmsg;
   private String status;
   private String dbText;
   private PublicRequest epr;

   public GetWossname(JembossParams mysettings)
             throws JembossSoapException
   {

     try
     {
       epr = new PublicRequest(mysettings,"getWossname");
     }
     catch (JembossSoapException jse)
     {
       throw new JembossSoapException("Connection failed");
     }
     

   }

   public boolean getStatus()
   {
     return epr.succeeded();
   }

// public String getStatus() 
// {
//   return status;
// }

   public String getStatusMsg() 
   {
     return statusmsg;
   }

   public String getDBText() 
   {
     return epr.getVal("wossname");
   }

}

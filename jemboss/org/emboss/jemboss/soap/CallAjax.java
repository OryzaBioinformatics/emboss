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
import org.apache.soap.rpc.*;
import org.emboss.jemboss.JembossParams;

public class CallAjax {

  private String statusmsg;
  private String status;
  private int length;
  private float weight;
  private boolean protein;
  private PublicRequest epr;

   public CallAjax(String fileContent, String seqtype, JembossParams mysettings) 
       throws JembossSoapException 
   {

     Vector params = new Vector();
     params.addElement(new Parameter("fileContent", String.class,
                                    fileContent, null));
     params.addElement(new Parameter("seqtype", String.class,
                                     seqtype, null));

     epr = new PublicRequest(mysettings,"call_ajax",params);

     Hashtable res = epr.getHash();
     Enumeration enumRes = res.keys();
     while(enumRes.hasMoreElements()) 
     {
       String thiskey = (String)enumRes.nextElement().toString();
       if(thiskey.equals("length"))
         length = ((Integer)res.get(thiskey)).intValue();
       if(thiskey.equals("weight"))
         weight = ((Float)res.get(thiskey)).floatValue();
       if(thiskey.equals("protein"))
         protein = ((Boolean)res.get(thiskey)).booleanValue();
     }
   }


  public String getStatus() 
  {
    return epr.getVal("status");
  }
  
  public String getStatusMsg() 
  {
    return epr.getVal("msg");
  }

  public int getLength()
  {
    return length;
  }
  
  public float getWeight()
  {
    return weight;
  }

  public boolean isProtein()
  {
    return protein;
  }

}

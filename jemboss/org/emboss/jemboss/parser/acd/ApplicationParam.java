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


package org.emboss.jemboss.parser.acd;


public class ApplicationParam 
{

  private String attribute=null;
  private String svalue=null;
  private double nvalue=0;


  public ApplicationParam(String attr, String sval) 
  {
    setAttribute(attr);
    setValue(sval);
  }

  public ApplicationParam(String attr, double nval) 
  {
    setAttribute(attr);
    setValue(nval);
  }

  public void setAttribute(String attribute) 
  {
     this.attribute = attribute;
  }

  public void reset(String sval)
  {
    setValue(sval);
    nvalue=0;
  }
  
  public void reset(double nval)
  {
    setValue(nval);
    svalue=null;
  }

  public void setValue(String svalue) 
  {
     this.svalue = svalue;
  }

  public void setValue(double nvalue) 
  {
     this.nvalue = nvalue;
  }

  public String getAttribute() 
  {
     return attribute;
  }

  public String getValueStr() 
  {
     return svalue;
  }

  public double getValueDbl() 
  {
     return nvalue;
  }


  public boolean isParamValueString() 
  {
    if(svalue != null) {
     return true;
    } else {
     return false;
    }
  }

}




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

import java.io.*;

public class ApplicationFields 
{

  private int numofParams;
  private int guiHandleNumber;
  private ApplicationParam appP[];


  public ApplicationFields() 
  {
  }

  public void setNumberOfParam(int numofParams) 
  {
   this.numofParams = numofParams;
   appP = new ApplicationParam[numofParams];
  }

  public void setParam(int index, String attr, String sval) 
  {
    appP[index] = new ApplicationParam(attr, sval);
  }

  public void setParam(int index, String attr, double nvalue) 
  {
    appP[index] = new ApplicationParam(attr, nvalue);
  }

  public void resetParam(int index, String sval) 
  {
    appP[index].reset(sval);
  }

  public void resetParam(int index, double nvalue) 
  {
    appP[index].reset(nvalue);
  }

  public String getParamAttribute(int index) 
  {
    return appP[index].getAttribute();
  }

  public String getParamValueStr(int index) 
  {
    return appP[index].getValueStr();
  }

  public double getParamValueDbl(int index) 
  {
    return appP[index].getValueDbl();
  }

  public boolean isParamValueStr(int index) 
  {
     return appP[index].isParamValueString();
  }

  public int getNumberOfParam() 
  {
   return numofParams;
  }

  public void setGuiHandleNumber(int guiHandleNumber) 
  {
    this.guiHandleNumber = guiHandleNumber;
  }

  public int getGuiHandleNumber() 
  {
    return guiHandleNumber;
  }

}


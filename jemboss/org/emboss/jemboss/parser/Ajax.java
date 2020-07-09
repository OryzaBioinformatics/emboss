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
*  @author: Copyright (C) Alan Bleasby 
*
********************************************************************/

package org.emboss.jemboss.parser;

/**
*
* Used with JNI to EMBOSS ajax library to determine sequence attributes.
*
*/
public class Ajax
{

/** true if the sequence is protein */
  public static boolean protein;
/** sequence length */
  public static int     length;
/** sequence weight */
  public static float   weight;

/** determine sequence attributes */
  public native boolean seqType(String usa);
  public native boolean seqsetType(String usa);

  static
  {
    System.loadLibrary("ajax");
  }

/**
*
* Sets the sequence length
* @param int sequence length
*/
  public void setLength(int length)
  {
    this.length = length;
  }

/**
*
* Sets the sequence weight
* @param float sequence weight
*/
  public void setWeight(float weight)
  {
    this.weight = weight;
  }

/**
*
* Sets whether sequence is protein (true)
* @param boolean sequence type
*/
  public void setProtein(boolean protein)
  {
    this.protein = protein;
  }

}


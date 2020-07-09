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


public class Application
{

 private String name;
 private String doc;
 private String groups;
 private ApplicationFields appfields[]; //contents of fields


 public Application(String name) 
 {
   setName(name);
   ApplicationFields appfields[] = new ApplicationFields[30];
 }

 public void setName(String name) 
 {
    this.name = name;
 }

 public void setDoc(String doc) 
 {
    this.doc    = doc;
 }

 public void setGroups(String groups) 
 {
    this.groups = groups;
 }

 public String getName()
 {
    return name;
 }

 public String getDoc()
 {
    return doc;
 }

 public String getGroups()
 {
    return groups;
 }


}


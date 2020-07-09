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
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.server;

import java.io.*;
import java.util.Date;

public class JembossThread extends Thread
{

  Process p;
  String project;

  public JembossThread(Process p, String project)
  {
    this.p = p;
    this.project = project;
  }
 
  public void run() 
  { 
    try
    {
      p.waitFor();
      createFinishedFile();
    }
    catch(InterruptedException intr)
    {
      createFinishedFile();
    }
  }
 
/**
*
* Creates a file named "finished" in the project directory,
* that contains a time stamp.
*
*/
  private void createFinishedFile()
  {
    String fs = new String(System.getProperty("file.separator"));
    File finished = new File(project + fs + ".finished");

    try
    {
      PrintWriter fout = new PrintWriter(new FileWriter(finished));
      fout.println((new Date()).toString());
      fout.close();
    }
    catch (IOException ioe) {}
  }
 

}



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

package org.emboss.jemboss.programs;

import java.io.*;


/**
*
* RunEmbossApplication class used to run an EMBOSS process
*
*/
public class RunEmbossApplication
{

  private Process p;
  private String stdout;
  private File project;
  private String status;

  public RunEmbossApplication(String embossCommand, String[] envp, File project)
  {
    this.project = project;
    status = "0";

    Runtime embossRun = Runtime.getRuntime();
    try
    {
      p = embossRun.exec(embossCommand,envp,project);
    }
    catch(IOException ioe)
    {
      status = "1";
    }
  }

  /**
  *
  * @return true if there is any standard out
  *
  */
  public boolean isProcessStdout()
  {
    stdout = "";

    try
    {
      String line;
      BufferedInputStream stdoutStream =
         new BufferedInputStream(p.getInputStream());
      BufferedReader stdoutRead =
         new BufferedReader(new InputStreamReader(stdoutStream));

      if((line = stdoutRead.readLine()) != null)
      {
        stdout = stdout.concat(line + "\n");
        while((line = stdoutRead.readLine()) != null)
          stdout = stdout.concat(line + "\n");
    
        if(project != null)
        {
          try{
            File so = new File(project.getCanonicalPath() + "/stdout");
            so.createNewFile();
            PrintWriter out = new PrintWriter(new FileWriter(so));
            out.println(stdout);
            out.close();
          }catch(IOException ioe){}
        }
      }
    }
    catch (IOException io)
    {
      System.out.println("Error in collecting standard out");
    }
 
    boolean std = false;
    if(!stdout.equals(""))
      std = true;

    return std;
  }


  /**
  *
  * @return standard out
  *
  */
  public String getProcessStdout()
  {
    return stdout;
  }

  /**
  *
  * @return process
  *
  */
  public Process getProcess()
  {
    return p;
  }

  /**
  *
  * @return status
  *
  */
  public String getStatus()
  {
    return status;
  }

}

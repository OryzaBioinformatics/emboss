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
* RunEmbossApplication2 class used to run an EMBOSS process
* this reads stdout and stderr in separate threads.
*
*/
public class RunEmbossApplication2
{

  /** running process */
  private Process p;
  /** standard out */
  private String stdout = "";
  /** standard error */
  private String stderr = "";
  /** running directory */
  private File project;
  /** process status */
  private String status;

  /**
  *
  * @param embossCommand        emboss command to run
  * @param envp                 environment
  * @param project              running directory
  *
  */
  public RunEmbossApplication2(String embossCommand, 
                       String[] envp, File project)
  {
    this.project = project;
    status = "0";

    Runtime embossRun = Runtime.getRuntime();
    try
    {
      p = embossRun.exec(embossCommand,envp,project);

      // 2 threads to read in stdout & stderr buffers 
      // to prevent blocking
//    StdoutHandler stdouth = new StdoutHandler(this);
      StderrHandler stderrh = new StderrHandler(this);
//    stdouth.start();
      stderrh.start();

    }
    catch(IOException ioe)
    {
      System.out.println("RunEmbossApplication2 Error executing: "+
                          embossCommand);
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
    if(stdout.equals(""))
      return false;
    return true;
  }

  /**
  *
  * Read in the process stderr.
  * @return	stderr
  *
  */
  public String readProcessStderr()
  {

    BufferedInputStream stderrStream = null;
    BufferedReader stderrRead = null;
    try
    {
      String line;
      stderrStream =
         new BufferedInputStream(p.getErrorStream());
      stderrRead =
         new BufferedReader(new InputStreamReader(stderrStream));
      char c[] = new char[100];
      int noff = 0;
      int nc = 0;

      while((nc = stderrRead.read(c,0,100)) != -1)
      {
        stderr = stderr.concat(new String(c,0,nc));
        noff += nc;
      }
    }
    catch (IOException io)
    {
      System.err.println("RunEmbossApplication2: Error in "+
                                "collecting standard out");
    }
    finally
    {
      try
      {
        if(stderrStream!=null)
          stderrStream.close();
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication2: Error closing stream");
      }
      try
      {
        if(stderrRead!=null)
          stderrRead.close();
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication2: Error closing reader");
      }
    }

    return stderr;
  }

  /**
  *
  * Read in the process stdout.
  * @return 	stdout
  *
  */
  public String readProcessStdout()
  {
    
    BufferedInputStream stdoutStream = null;
    BufferedReader stdoutRead = null;
    try
    {
      String line;
      stdoutStream =
         new BufferedInputStream(p.getInputStream());
      stdoutRead =
         new BufferedReader(new InputStreamReader(stdoutStream));
 
      
      char c[] = new char[100];
      int noff = 0;
      int nc = 0;

      while((nc = stdoutRead.read(c,0,100)) != -1)
      {
        stdout = stdout.concat(new String(c,0,nc));
        noff += nc;
      }

    }
    catch (IOException io)
    {
      System.err.println("RunEmbossApplication2: Error in "+ 
                                "collecting standard out");
    }
    finally
    {
      try
      {
        if(stdoutStream!=null)
          stdoutStream.close();
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication2: Error closing stream");
      } 
      try
      {
        if(stdoutRead!=null)
          stdoutRead.close();
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication2: Error closing reader");
      }
    }
 
    return stdout;
  }

  /**
  *
  * This method can be called after the process has completed
  * to write the stdout to the project directory.
  *
  */
  public void writeStdout()
  {
    if(project != null)
    {
      PrintWriter out = null;
      String fname = "";
      try
      {
        fname = project.getCanonicalPath() + 
                             "/application_stdout";
        File so = new File(fname);

        if(!so.exists())
          so.createNewFile();

        out = new PrintWriter(new FileWriter(fname,true));
        out.println(stdout);
      }
      catch(IOException ioe)
      {
        System.err.println("RunEmbossApplication2: Error writing" +
                            fname);
      }
      finally
      {
        if(out!=null)
          out.close();
      }
    }
  }

  /**
  *
  * @return standard out
  *
  */
  public String getProcessStdout()
  {
//  readProcessStdout();
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

  class StdoutHandler extends Thread
  {
    RunEmbossApplication2 rea;

    protected StdoutHandler(RunEmbossApplication2 rea)
    {
      this.rea = rea;
    }

    public void run()
    {
      rea.readProcessStdout();
    }
  }

  class StderrHandler extends Thread
  {
    RunEmbossApplication2 rea;

    protected StderrHandler(RunEmbossApplication2 rea)
    {
      this.rea = rea;
    }

    public void run()
    {
      rea.readProcessStderr();
    }
  }

}


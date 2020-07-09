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
*  Based on EmbreoResult
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.programs;

import java.util.Hashtable;

public class JembossProcess
{

  private boolean completed = false;
  private boolean running = true;
  private boolean hasresults = false;
  private String project = "";
  private String jobid = "";
  private String description = "";
  final private String blankstring = "";
  private Hashtable results;

  public JembossProcess(String project, String jobid, String description) 
  {
    this.project = project;
    this.jobid = jobid;
    this.description = description;
  }

  public JembossProcess(String project, String jobid) 
  {
    this.project = project;
    this.jobid = jobid;
  }

  public JembossProcess(String jobid) 
  {
    this.jobid = jobid;
  }

  public boolean isCompleted() 
  {
    return completed;
  }

  public boolean isRunning() 
  {
    return running;
  }

  public boolean hasResults() 
  {
    return hasresults;
  }

  public String getProject() 
  {
    return project;
  }

  public String getJob() 
  {
    return jobid;
  }

  public String getDescription() 
  {
    return description;
  }

  public void complete() 
  {
    completed = true;
    running = false;
  }

  public void setDescription(String s) 
  {
    description = s;
  }

  public void setResults(Hashtable newres) 
  {
    results = new Hashtable();
    results = newres;
    hasresults = true;
  }

  public Hashtable getResults() 
  {
    return results;
  }

}

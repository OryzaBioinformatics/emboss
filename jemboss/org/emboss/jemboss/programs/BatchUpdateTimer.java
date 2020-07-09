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

package org.emboss.jemboss.programs;

import org.emboss.jemboss.*;
import java.util.Timer;
import java.util.TimerTask;

/**
*
* Simple demo that uses java.util.Timer to schedule a task to execute
* once 10 seconds have passed.
*
*/
public class BatchUpdateTimer
{
  private static Timer timer;

  public BatchUpdateTimer(int seconds)
  {
    timer = new Timer();
    timer.schedule(new RemindTask(), seconds*1000, seconds*1000);
  }


  class RemindTask extends TimerTask
  {
    public void run()
    {
      Jemboss.resultsManager.updateStatus();
//    System.out.println("Results Manager " + Jemboss.resultsManager.getStatus());
      if(Jemboss.resultsManager.getStatus().endsWith("completed"))
      {
        timer.cancel(); //Terminate the timer thread
//      System.out.println("End timer thread");
        Jemboss.resultsManager.setAutoUpdate(false);
      }
    }
  }

}


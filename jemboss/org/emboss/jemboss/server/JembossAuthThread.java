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
import org.emboss.jemboss.parser.Ajax;

public class JembossAuthThread extends Thread
{

  String embossCommand;
  String environ;
  String project;
  String userName;
  byte[] passwd;

  protected JembossAuthThread(String userName, byte[] passwd,
        String embossCommand,String environ, String project) 
  {
    this.embossCommand = embossCommand;
    this.environ = environ;
    this.project = project;
    this.userName = userName;
    this.passwd = passwd;
  }
 
  public void run() 
  {
    Ajax aj = new Ajax(); 
    boolean lfork = aj.forkEmboss(userName,passwd,environ,
                                     embossCommand,project);
    boolean ok = aj.putFile(userName,passwd,environ,
               new String(project+"/.finished"),
               (new Date()).toString().getBytes());

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';
  }
 

}



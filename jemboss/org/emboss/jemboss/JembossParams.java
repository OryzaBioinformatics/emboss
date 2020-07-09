/** Jemboss *****************************************************
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
***************************************************************/

package org.emboss.jemboss;


import java.util.Properties;
import java.io.FileInputStream;

public class JembossParams
{

  private String plplot;
  private String plplotName = "plplot";
  private String embossData;
  private String embossDataName = "embossData";
  private String embossBin;
  private String embossBinName = "embossBin";
  private String embossPath;
  private String embossPathName = "embossPath";
  private String acdDirToParse;
  private String acdDirToParseName = "acdDirToParse";

  private Properties jembossSettings;
  public JembossParams()
  {
     
    ClassLoader cl = this.getClass().getClassLoader();
    jembossSettings = new Properties();
    String fs = new String(System.getProperty("file.separator"));

    // look on classpath
    try 
    {
      jembossSettings.load(cl.getResourceAsStream("resources" + fs +
                              "jemboss.properties"));
    }  
    catch (Exception e)
    {
      System.out.println("Didn't find resources" + fs +
                              "jemboss.properties");
    }

    FileInputStream in = null;
    // override with local system settings
    try
    {
      String folder = System.getProperty("user.dir");
      String filesep = System.getProperty("file.separator");
      in = new FileInputStream(folder
                + filesep
                + "jemboss.properties");
      jembossSettings.load(in);

    } 
    catch (java.io.FileNotFoundException e) 
    {
      in = null;
      System.out.println("Can't find properties file. " +
                          "Using defaults.");
    } 
    catch (java.io.IOException e) 
    {
      System.out.println("Can't read properties file. " +
                          "Using defaults.");
    }
    finally 
    {
      if(in != null) 
      {
        try { in.close(); } catch (java.io.IOException e) { }
        in = null;
      }
    }
 
    // override with local user settings
    try 
    {
      String folder = System.getProperty("user.home");
      String filesep = System.getProperty("file.separator");
      in = new FileInputStream(folder
                + filesep
                + "jemboss.properties");
      jembossSettings.load(in);

    } 
    catch (java.io.FileNotFoundException e) 
    {
      in = null;
      System.out.println("Can't find user properties file. " +
                          "Using defaults.");
    }
    catch (java.io.IOException e) 
    {
      System.out.println("Can't read user properties file. " +
                          "Using defaults.");
    } 
    finally
    {
      if (in != null)
      {
        try { in.close(); } catch (java.io.IOException e) { }
        in = null;
      }
    }

    plplot = jembossSettings.getProperty(plplotName);
    embossData = jembossSettings.getProperty(embossDataName);
    embossBin = jembossSettings.getProperty(embossBinName);
    embossPath = jembossSettings.getProperty(embossPathName);
    acdDirToParse = jembossSettings.getProperty(acdDirToParseName);

  }  

  public String getPlplot()
  {
    return plplot;
  }

  public String getEmbossData()
  {
    return embossData;
  }
  
  public String getEmbossBin()
  {
    return embossBin;
  }

  public String getEmbossPath()
  {
    return embossPath;
  }


  public String getAcdDirToParse()
  {
    return acdDirToParse;
  }

}


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
*  Based on EmbreoListFile but adding in list files from list pane.
*
*  @author: Copyright (C) Tim Carver
*
********************************************************************/


package org.emboss.jemboss.programs;


import java.io.*;
import java.util.Hashtable;
import uk.ac.mrc.hgmp.embreo.*;


public class ListFile 
{

/**
* Parse a list file, reading it in line by line, loading other
* list files recursively if necessary, and loading any files
* referred to into the filesToMove hash
*
* @param   fn  The name of the list file
* @param   filesToMove The hash to put the files into
*/
  public static void parse(String fn, Hashtable filesToMove) 
  {

    String lfn = trim(fn);
    File inFile = new File(lfn);

    if((inFile.exists() && inFile.canRead() && inFile.isFile()) 
        || fn.startsWith("internalList::")) 
    {
      EmbreoMakeFileSafe sf;
      String sfs = new String("internalList");  //default name1
      if(inFile.exists())
      {
        sf = new EmbreoMakeFileSafe(lfn);
        sfs = sf.getSafeFileName();
      }

      try 
      {
        BufferedReader in;
        if(fn.startsWith("internalList::"))
        {
          fn = fn.substring(14);
          in = new BufferedReader(new StringReader(fn)); 
          in.readLine();
        }
        else
        in = new BufferedReader(new FileReader(inFile));
        
        String listfile = "";
        String line;
        while((line = in.readLine()) != null) 
        {
          if (line.startsWith("@")||line.startsWith("list::")) 
          {
            lfn = trim(line);
	      
            if((new File(lfn)).exists()) 
            {
              EmbreoMakeFileSafe lf = new EmbreoMakeFileSafe(lfn);
	      String slf = lf.getSafeFileName();

	      if(!filesToMove.containsKey(slf)) 
	      ListFile.parse(lfn, filesToMove);
	      listfile = listfile.concat("@"+slf+"\n");
	    }
            else 
            {
	      listfile = listfile.concat(line+"\n");  //remote list file
	    }
	  } 
          else               // plain sequence file
          {
	    File pFile = new File(line);
            if(pFile.exists() && pFile.canRead() && pFile.isFile()) 
            {
	      EmbreoMakeFileSafe pf = new EmbreoMakeFileSafe(line);
	      String spf = pf.getSafeFileName();
	      // only read if we haven't already
	      if(!filesToMove.containsKey(spf)) 
              {
	        try
                {
	          BufferedReader fin = new BufferedReader(new FileReader(pFile));
	          String ftext = "";
	          String fline;
	          while((fline = fin.readLine()) != null) 
	            ftext = ftext.concat(fline+"\n");

	          filesToMove.put(spf,ftext);
	        } 
                catch (IOException e) 
                {
	          System.out.println("ListFile: Error reading list file " + line);
	        }
	      }
	      // add the server reference to the listfile
	      listfile = listfile.concat(spf+"\n");
	    } 
            else    // presumably remote
            {
	      listfile = listfile.concat(line+"\n");
	    }
	  }
        }
	
        filesToMove.put(sfs,listfile);   // add list file itself
      }
      catch (IOException e) {}
    } 
    
    return;
  }


/**
*
* Given a list file @file or list::file return just the file name
* @param String file name
*
*/
  private static String trim(String fn) 
  {
    String lfn;
    if (fn.startsWith("@")) 
      lfn = new String(fn.substring(1));
    else if (fn.startsWith("list::")) 
      lfn = new String(fn.substring(6));
    else 
      lfn = fn;
    return lfn;
  }
 
}

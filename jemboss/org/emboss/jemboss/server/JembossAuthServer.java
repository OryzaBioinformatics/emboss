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

import org.emboss.jemboss.JembossParams;
import org.emboss.jemboss.programs.RunEmbossApplication;
import org.emboss.jemboss.parser.Ajax;

import java.io.*;
import java.util.*;

/**
*
* Jemboss Authenticated Server for SOAP
*
*/
public class JembossAuthServer
{

//SOAP results directory
  private String tmproot = new String("/tmp/SOAP/emboss/");
  private String logFile = new String(tmproot+"/jemboss.log");
  private String errorLog = new String(tmproot+"/jemboss_error.log");
  private File tmprootDir = new File(tmproot);


  private String fs = new String(System.getProperty("file.separator"));
  private String ps = new String(System.getProperty("path.separator"));
  private String ls = new String(System.getProperty("line.separator"));

//get paths to EMBOSS
  JembossParams jp  = new JembossParams();
  String plplot     = jp.getPlplot();
  String embossData = jp.getEmbossData();
  String embossBin  = jp.getEmbossBin();
  String embossPath = jp.getEmbossPath();
  String acdDirToParse     = jp.getAcdDirToParse();
  String embossEnvironment = jp.getEmbossEnvironment();

  private String[] env = 
  {
    "PATH=" + embossPath + ps + embossBin,
    "PLPLOT_LIB=" + plplot,
    "EMBOSS_DATA=" + embossData
//  ,"LD_LIBRARY_PATH=/usr/local/lib"
// FIX FOR SOME SUNOS
  };
  private String[] envp = jp.getEmbossEnvironmentArray(env);
  
  private String environ = "PATH=" + embossBin+ ps + embossPath +" "+
                           "PLPLOT_LIB=" + plplot +" "+
                           "EMBOSS_DATA=" + embossData +" "+
                           jp.getEmbossEnvironment();
// "LD_LIBRARY_PATH=/usr/local/lib"+" ";
// FIX FOR SOME SUNOS

  public String name()
  {
    return "The EMBOSS Application Suite";
  }

  public String version()
  {
    String embossCommand = new String(embossBin + "embossversion");
    RunEmbossApplication rea = new RunEmbossApplication(embossCommand,
                                                           envp,null);
    rea.isProcessStdout();
    return rea.getProcessStdout();
  }


  public String appversion()
  {
    String embossCommand = new String(embossBin + "embossversion");
    RunEmbossApplication rea = new RunEmbossApplication(embossCommand,
                                                           envp,null);
    rea.isProcessStdout();
    return rea.getProcessStdout();
  }


  public String about()
  {
    return "Jemboss is an interface to the EMBOSS suite of programs.";
  }

  public String helpurl()
  {
    return "http://www.uk.embnet.org/Software/EMBOSS/";
  }

  public String abouturl()
  {
    return "http://www.uk.embnet.org/Software/EMBOSS/overview.html";
  }

  public String docurl()
  {
    return "http://www.uk.embnet.org/Software/EMBOSS/general.html";
  }

  public Hashtable servicedesc()
  {
    Hashtable desc = new Hashtable();
    desc.put("name",name());
    desc.put("version",version());
    desc.put("appversion",appversion());
    desc.put("about",about());
    desc.put("helpurl",helpurl());
    desc.put("abouturl",abouturl());
    desc.put("docurl",docurl());
    return desc;
  }


/**
*
* Retrieves the ACD file of an application.
* @param  application name
* @return Vector of containing the ACD string
*
*/
  public Vector show_acd(String appName)
  {

    Vector acd = new Vector();
    String acdText = new String("");
    String line;
    String acdToParse = new String(acdDirToParse + appName + ".acd");

    try
    {
      BufferedReader in = new BufferedReader(new FileReader(acdToParse));
      while((line = in.readLine()) != null )
      {
        if(!line.startsWith("#") && !line.equals(""))
        { 
          line = line.trim();
          line = line.replace('}',')');
          acdText = acdText.concat(line + "\n");
        }
      }
    }
    catch (IOException e)
    {
      appendToLogFile("Cannot open EMBOSS acd file "+acdToParse,errorLog);
    }

    acd.add("status");
    acd.add("0");
    acd.add("acd");
    acd.add(acdText);

    return acd;
  }

/**
*
* Returns the output of the EMBOSS utility wossname
* @return wossname output
*
*/
  public Vector getWossname()
  {
    Vector wossOut = new Vector();
    String embossCommand = new String(embossBin + 
                   "wossname -colon -gui -auto");
 
    RunEmbossApplication rea = new RunEmbossApplication(embossCommand,
                                                           envp,null);
    wossOut.add("status");
    wossOut.add("0");
    rea.isProcessStdout();
    wossOut.add("wossname");
    wossOut.add(rea.getProcessStdout());

    return wossOut;
  }


/**
*
* Returns the help for an application as given by 'tfm'
* @param String application name
* @return help 
*
*/
  public Vector show_help(String applName)
  {
    String command = embossBin.concat("tfm " + applName + " -html -nomore");
    RunEmbossApplication rea = new RunEmbossApplication(command,envp,null);
    String helptext = "";
    if(rea.isProcessStdout())
      helptext = rea.getProcessStdout();
    else
      helptext = "No help available for this application.";

    Vector vans = new Vector();
    vans.add("helptext");
    vans.add(helptext);

    return vans;
  }


/**
*
* Uses JNI to calculate sequence attributes using EMBOSS library call. 
* @param sequence filename or database entry
* @return sequence length, weight & type (protein/nucleotide)
*
*/
  public Vector call_ajax(String fileContent, String seqtype,
                          String userName, byte[] passwd)
  {

    Vector vans = new Vector();
    Ajax aj = new Ajax();
    if(!verifyUser(aj,userName,passwd,vans))
      return vans;

    boolean afile = false;
    boolean fexists = false;
    String fn = null;

    // local file exists?
    if(fileContent.startsWith(fs))
    {
      int ind = fileContent.lastIndexOf(fs);
      String fdir  = fileContent.substring(0,ind);
      String ffile = fileContent.substring(ind+1).trim(); 
      aj.listFiles(userName,passwd,environ,fdir);
      if(aj.getOutStd().indexOf(ffile+"\n") > -1)
        fexists = true;
    }

    // create temporary file
    if( ((fileContent.indexOf(":") < 0) || 
         (fileContent.indexOf("\n") > 0) ) &&
         (!fexists) ) 
    {
      try
      {
        fn = tmproot+fs+userName+fs+".jembosstmp";
        boolean ok = aj.putFile(userName,passwd,environ,
                             fn,fileContent.getBytes());
        afile = true;
      }
      catch (Exception ioe) 
      {
        appendToLogFile("IOException: call_ajax creating tmp.jembosstmp",
                         errorLog);
        vans.add("status");
        vans.add("1");
        return vans;
      }
    }
    else
    {
      fn = fileContent;     //looks like db entry or local file name
    }


    boolean ok = false;
    if( fexists  || afile ||    //call ajax if sequence file
        fn.indexOf(":") > 0 )   //or db
    {
      try
      {
        if(seqtype.startsWith("seqset"))
          ok = aj.seqsetAttrib(userName,passwd,environ,fn);
        else
          ok = aj.seqAttrib(userName,passwd,environ,fn);
      }
      catch (Exception e)
      {
        appendToLogFile("Exception: call_ajax status not ok",
                         errorLog);
        vans.add("status");
        vans.add("1");
        return vans;
      }
    }

    if(afile)
      aj.delFile(userName,passwd,environ,fn);

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    if(ok)
    {
//    System.out.println("STATUS OK");
      vans.add("length");
      vans.add(new Integer(aj.length_soap));
      vans.add("protein");
      vans.add(new Boolean(aj.protein_soap));
      vans.add("weight");
      vans.add(new Float(aj.weight_soap));
      vans.add("status");
      vans.add("0");
    }
    else
    {
      appendToLogFile("Error: call_ajax status not ok",
                         errorLog);
      vans.add("status");
      vans.add("1");
    }

    return vans;
  }


/**
*
* Uses JNI to calculate sequence attributes using EMBOSS library call. 
* @param sequence filename or database entry
* @return sequence length, weight & type (protein/nucleotide)
*
*/
  public Vector call_ajax(String fileContent, String seqtype)
  {
    boolean afile = false;
    String fn = null;
    File tf = null;

    Vector vans = new Vector();

    // create temporary file
    if( ((fileContent.indexOf(":") < 0) ||
         (fileContent.indexOf("\n") > 0) ) &&
       !((new File(fileContent)).exists()) )
    {
      afile = true;
      try
      {
        tf = File.createTempFile("tmp",".jembosstmp", tmprootDir);
        PrintWriter out = new PrintWriter(new FileWriter(tf));
        out.println(fileContent);
        out.close();

        fn = new String(tf.getCanonicalPath());
      }
      catch (IOException ioe)
      {
        appendToLogFile("IOException: call_ajax creating tmp.jembosstmp",
                         errorLog);
        vans.add("status");
        vans.add("1");
        return vans;
      }
    }
    else
    {
      fn = fileContent;     //looks like db entry or local file name
    }


    boolean ok = false;
    Ajax aj = null;

    if( ((new File(fn)).exists()) ||    //call ajax if sequence file
         (fn.indexOf(":") > 0) )        //or db
    {
      try
      {
        aj = new Ajax();
        if(seqtype.startsWith("seqset"))
          ok = aj.seqsetType(fn);
        else
          ok = aj.seqType(fn);
      }
      catch (Exception e)
      {
         appendToLogFile("Exception: call_ajax status not ok",
                         errorLog);
        vans.add("status");
        vans.add("1");
        return vans;
      }
    }

    if(ok)
    {
//    System.out.println("STATUS OK");
      vans.add("length");
      vans.add(new Integer(aj.length));
      vans.add("protein");
      vans.add(new Boolean(aj.protein));
      vans.add("weight");
      vans.add(new Float(aj.weight));
      vans.add("status");
      vans.add("0");
    }
    else
    {
      appendToLogFile("Error: call_ajax status not ok",
                         errorLog);
      vans.add("status");
      vans.add("1");
    }

    if(afile)
      tf.delete();

    return vans;
  }


/**
*
* Returns the databases held on the server
* @return output from 'showdb'
*
*/
  public Vector show_db()
  {
    Vector showdbOut = new Vector();
    String embossCommand = new String(embossBin + "showdb -auto");
    RunEmbossApplication rea = new RunEmbossApplication(embossCommand,
                                                           envp,null);
    showdbOut.add("status");
    showdbOut.add("0");
    rea.isProcessStdout();
    showdbOut.add("showdb");
    showdbOut.add(rea.getProcessStdout());
     
    // find available matrices
    String dataFile[] = (new File(embossData)).list(new FilenameFilter()
    {
      public boolean accept(File dir, String name)
      {
        File fileName = new File(dir, name);
        return !fileName.isDirectory();
      };
    });
    String matrices ="";
    for(int i=0;i<dataFile.length;i++)
      matrices=matrices.concat(dataFile[i]+"\n");
    showdbOut.add("matrices");
    showdbOut.add(matrices);

    // find available codon usage tables
    
    dataFile = (new File(embossData+fs+"CODONS")).list(new FilenameFilter()
    {
      public boolean accept(File dir, String name)
      {
        File fileName = new File(dir, name);
        return !fileName.isDirectory();
      };
    });
    matrices ="";
    for(int i=0;i<dataFile.length;i++)
      matrices=matrices.concat(dataFile[i]+"\n");
    showdbOut.add("codons");
    showdbOut.add(matrices);

    return showdbOut;
  }


/**
*
* Private Authenticated Server
* 
* Run an EMBOSS application
* @param command line to run
* @param unused 
* @param Hashtable of input files
* @return output files from application run
*
*/
  public synchronized Vector run_prog(String embossCommand, String options,
                          Hashtable inFiles,String userName, byte[] passwd)
  {

    tmproot = tmproot.concat(userName+fs);

    tmprootDir = new File(tmproot);

    String name = Thread.currentThread().getName();
    Ajax aj = new Ajax();

    Vector result = new Vector();

    if(!verifyUser(aj,userName,passwd,result))
      return result;

    //disallow multiple command constructions
    if(embossCommand.indexOf(";") > -1) 
    {
      String warn = new String("ERROR: Disallowed command syntax "+
                               embossCommand);
      appendToLogFile(warn,errorLog);
      result.add("msg");
      result.add(warn);
      result.add("status");
      result.add("1");
      return result;
    }

    //trap anything that is trying to write to stdout
    int stdIndex = embossCommand.indexOf(" stdout ");
    if(stdIndex > -1)
    {
      String startCmd = embossCommand.substring(0,stdIndex+7);
      String endCmd = embossCommand.substring(stdIndex+8);
      embossCommand = startCmd.concat("file ");
      embossCommand = embossCommand.concat(endCmd);
    }

    Enumeration enum = inFiles.keys();
    String appl   = embossCommand.substring(0,embossCommand.indexOf(" "));
    String rest   = embossCommand.substring(embossCommand.indexOf(" "));
    embossCommand = embossBin.concat(embossCommand);
    String msg = new String("");

    boolean ok=false; 
    boolean lsd = false;
    try
    {
     lsd = aj.listDirs(userName,passwd,environ,tmproot);
    }
    catch(Exception exp){}

//  appendToLogFile("listDirs "+name+" STDERR "+aj.getErrStd(),errorLog);   //DEBUG

// create the results directory structure for this user if necessary
    if(!lsd || !aj.getErrStd().equals(""))
    {
      try
      {
        ok = aj.makeDir(userName,passwd,environ,tmproot);
      }
      catch(Exception exp){}

//    if(ok && aj.getErrStd().equals(""))
//    Linux fix - ignore stderr here
      if(ok)
        appendToLogFile("Created directory "+tmproot,errorLog);
      else
      {
        String warnmsg = new String("Failed to create dir "+tmproot+
                                    "\nSTDERR :"+ aj.getErrStd());
        appendToLogFile(warnmsg,errorLog);
        result.add("msg");
        result.add(warnmsg);
        result.add("status");
        result.add("1");
        return result;
      }
    }

    Random rnd = new Random();
    String dat = new Date().toString();

//get a unique project name 
    String project = new String(tmproot + appl + "_" +
         dat.replace(' ','_') + "_" + rnd.nextInt(99999));

    File projectDir = new File(project);
    try
    {
      ok = aj.makeDir(userName,passwd,environ,project);
    }
    catch(Exception exp){}

//  appendToLogFile("makeDir "+name+" STDERR "+aj.getErrStd(),errorLog);  //DEBUG

    if(!ok || !aj.getErrStd().equals("")) 
    {
      String warnmsg = new String("run_prog failed to create dir "+
                                project+"\nSTDERR :"+aj.getErrStd());
      appendToLogFile(warnmsg,errorLog);
      result.add("msg");
      result.add(warnmsg);
      result.add("status");
      result.add("1");
      return result;
    }

//create description file & input files
    String descript = "EMBOSS run details"+ls+ls+
                      "Application: "+appl+ls+rest+ls+
                      "Started at "+dat+ls+ls+"Input files:"+ls;

    while(enum.hasMoreElements())
    {
      String thiskey = (String)enum.nextElement().toString();
      descript = descript.concat(project+fs+thiskey+ls);

      ok = false;
      try
      {
        ok = aj.putFile(userName,passwd,environ,
                 new String(project+fs+thiskey),
                 (byte[])inFiles.get(thiskey));
      }
      catch(Exception exp){}

      if(!ok)
      {
        appendToLogFile("Failed to make file "+project+fs+thiskey,errorLog);
        appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
        appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
        return result;
      }

//    appendToLogFile("putFile "+name+" STDERR "+aj.getErrStd(),errorLog);  //DEBUG
    }

    ok = false;
    try
    {
      ok = aj.putFile(userName,passwd,environ,
           new String(project + fs + ".desc"),
           descript.getBytes());
    }
    catch(Exception exp){}

    if(!ok)
    {
      appendToLogFile("Failed to make file "+project+fs+".desc",errorLog);
      appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
      return result;
    }

//  appendToLogFile("putFile "+name+" STDERR "+aj.getErrStd(),errorLog);  //DEBUG
    appendToLogFile(options+" "+dat+" "+embossCommand,logFile);

    result.add("cmd");
    result.add(appl + " " + rest);
    result.add("status");
    result.add("0");

    if(options.toLowerCase().indexOf("interactive") > -1)
    {
      boolean lfork=true;
      try
      {
        lfork = aj.forkEmboss(userName,passwd,environ,
                               embossCommand,project);
      }
      catch(Exception exp){} 
 
      result.add("msg");
      if(!lfork || !aj.getErrStd().equals(""))
      {
        result.add("stderr");
        result.add("stderr");
        result.add("ERROR REPORTED\n"+aj.getErrStd());
        appendToLogFile("Fork process failed "+embossCommand,errorLog);
      }
      else
        result.add("");

      try
      {
        aj.putFile(userName,passwd,environ,
             new String(project+fs+".finished"),
            (new Date()).toString().getBytes());
      }
      catch(Exception exp){}

//    appendToLogFile("putFile "+name+" STDERR "+aj.getErrStd(),errorLog);  //DEBUG

//get the output files
      result = loadFilesContent(aj,userName,passwd,
                      projectDir,project,result,inFiles);

      for(int i=0;i<passwd.length;i++)
        passwd[i] = '\0';

    }
    else      //batch or background
    {

// COMMENT THIS LINE TO USE QUEUEING SOFTWARE
      boolean lforkB = aj.forkBatch(userName,passwd,environ,
                                    embossCommand,project);

// UNCOMMENT THIS LINE TO USE QUEUEING SOFTWARE
//    runAsBatch(aj,userName,passwd,project,embossCommand)

      result.add("msg");
      result.add("");
      result.add("job_submitted");
      result.add("Job " + projectDir.getName() + "submitted.");
      result.add("jobid");
      result.add(projectDir.getName());
      result.add("description");
      result.add(descript+ls+"Application pending"+ls);
    }

    return result;
  }


/**
*
* Submit to a batch queue. This method creates a script for
* submission to a batch queueing system.
*
*/
  private void runAsBatch(Ajax aj, String userName, byte[] passwd,
                    String project, String embossCommand)
  {
    String scriptIt = "#!/bin/sh\n";
    scriptIt = scriptIt.concat(environ.replace(' ','\n'));
    scriptIt = scriptIt.concat("export PATH\n");
    scriptIt = scriptIt.concat("export PLPLOT_LIB\n");
    scriptIt = scriptIt.concat("export EMBOSS_DATA\n");
    scriptIt = scriptIt.concat("cd "+project+"\n"+embossCommand+"\n");
    scriptIt = scriptIt.concat("date > "+project+"/.finished\n");
    
    boolean ok = false;
    try
    {
      ok = aj.putFile(userName,passwd,environ,
               new String(project+fs+".scriptfile"),
               scriptIt.getBytes());
    }
    catch(Exception exp){}

    if(!ok)
    {
      appendToLogFile("Failed to make file "+project+fs+".scriptfile",errorLog);
      appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
    }
  
    boolean lfork=true;
    try
    {
      //EDIT batchCommand 
      String batchCommand = "/bin/batchQueue.sh " + project +
                            "/.scriptfile ";
                         
      lfork = aj.forkEmboss(userName,passwd,environ,
                            batchCommand,project);
    }
    catch(Exception exp){}

    if(!lfork || !aj.getErrStd().equals(""))
      appendToLogFile("Fork batch process failed "+embossCommand,errorLog);

    return;
  }

/**
*
* Private Server
*
* Returns the results for a saved project.
* @param project/directory name
* @param unused if showing all results otherwise this
*        is the name of the file to display
* @return saved results files
*
*/
  public Vector show_saved_results(String project, String cl,
                              String userName, byte[] passwd)
  {

    Ajax aj = new Ajax();
    Vector ssr = new Vector();
    if(!verifyUser(aj,userName,passwd,ssr))
      return ssr;

    tmproot = tmproot.concat(userName+fs); 

    project = tmproot.concat(project);
    File projectDir = new File(project);

//  ssr = loadFilesContent(aj,userName,passwd,
//                    projectDir,project,ssr,null);
    if(cl.equals(""))
    {
      ssr = loadFilesContent(aj,userName,passwd,
                      projectDir,project,ssr,null);
    }
    else
    {
      byte fbuf[]=null;
      try
      {
        fbuf =  aj.getFile(userName,passwd,environ,
                                  project+"/"+cl);
        ssr.add(cl);
        ssr.add(new String(fbuf));
      }
      catch(Exception exp){}
    }
 
    ssr.add("status");
    ssr.add("0");

    ssr.add("msg");
    ssr.add("OK");

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return ssr;
  }

/**
*  
* Private server
*
* Save a file to a project directory on the server.
* @return message
*
*/
  public Vector save_project_file(String project, String filename, 
                    String notes, String userName, byte[] passwd)
  {
    Ajax aj = new Ajax();
    Vector v = new Vector();
    if(!verifyUser(aj,userName,passwd,v))
      return v;

    String fn = tmproot + fs + userName+ fs + 
                     project + fs + filename;
    boolean ok = aj.putFile(userName,passwd,environ,
                            fn,notes.getBytes());

    v.add("status");
    v.add("0");
    v.add("msg");

    if(ok)
      v.add("OK"); 
    else
    {
      appendToLogFile("Failed to save file "+fn,errorLog);
      appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
    }
   
    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';
        
    return v;
  }

/**
*  
* Private server
*
* Deletes a projects saved results.
* @param project/directory name
* @param unused
* @return message
*
*/
  public Vector delete_saved_results(String project, String cl,
                                String userName, byte[] passwd)
  {

    Vector dsr = new Vector();
    Ajax aj = new Ajax();
    if(!verifyUser(aj,userName,passwd,dsr))
      return dsr;

    tmproot = tmproot.concat(userName+fs);

    StringTokenizer st = new StringTokenizer(project,"\n");
    while(st.hasMoreTokens()) 
    {
      String proj = tmproot.concat(st.nextToken());
      boolean ok = aj.delDir(userName,passwd,environ,proj);
      if(!ok || !aj.getErrStd().equals(""))
      {
        appendToLogFile("Failed deletion of directory "+proj,errorLog);
        appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
        appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
      }
    }

    dsr.add("status");
    dsr.add("0");
    dsr.add("msg");
    dsr.add("Results deleted successfully.");

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return dsr;
  }


/**
*
* Private Server
*
* List of the saved results on the server.
* @return list of the saved results.
*
*/
  public Vector list_saved_results(String userName, byte[] passwd)
  {
    Ajax aj = new Ajax();
    Vector lsr = new Vector();
    if(!verifyUser(aj,userName,passwd,lsr))
      return lsr;

    tmproot = tmproot.concat(userName+fs);

//  tmprootDir = new File(tmproot);
   
    lsr.add("status");
    lsr.add("0");
    lsr.add("msg");
    lsr.add("OK");

    boolean lsd = aj.listDirs(userName,passwd,environ,tmproot);
    
    if(!lsd || !aj.getErrStd().equals(""))
    {
      appendToLogFile("Failed list_saved_results "+tmproot,errorLog);
      appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
    }

    String outStd = aj.getOutStd();
    StringTokenizer stok = new StringTokenizer(outStd,"\n");

    while(stok.hasMoreTokens())
    {
      String dirname = stok.nextToken();
      lsr.add(dirname);
      byte fbuf[] = aj.getFile(userName,passwd,environ,
                tmproot + fs + dirname + fs + ".desc");
      lsr.add(new String(fbuf));

      if(aj.getFileok()!=1)
      {
        appendToLogFile("Calling getFile : "+tmproot + fs +
                          dirname + fs + ".desc",errorLog);
        appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
        appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
      }
    }
    
    lsr.add("list");
    lsr.add(outStd);

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return lsr;
  }


/**
*
* Appends a log entry to the log file
*
*/ 
  private void appendToLogFile(String logEntry, String logFileName)
  {
    BufferedWriter bw = null;
    try 
    {
      bw = new BufferedWriter(new FileWriter(logFileName, true));
      bw.write(logEntry);
      bw.newLine();
      bw.flush();
    } 
    catch (IOException ioe) 
    {
      System.out.println("Error writing to log file "+logFile);
      ioe.printStackTrace();
    } 
    finally                     // always close the file
    {                       
      if(bw != null) 
        try
        {
          bw.close();
        } 
        catch (IOException ioe2) {}
    }

  }


/**
*
* Reads in files from EMBOSS output
*
*/
  private Vector loadFilesContent(Ajax aj, String userName, 
            byte[] passwd, File projectDir, String project,
            Vector result, Hashtable inFiles)
  {

    boolean ls = false;

    try
    {
      ls = aj.listFiles(userName,passwd,environ,project);
    }
    catch(Exception exp){}

    String name = Thread.currentThread().getName();
//  appendToLogFile("listFiles "+name+" STDERR "+aj.getErrStd(),errorLog);  //DEBUG

    if(!ls)
    {
      appendToLogFile("Failed loadFilesContent "+project,errorLog);
      appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
      appendToLogFile("STDOUT "+aj.getOutStd(),errorLog);
    }

    String outStd = aj.getOutStd();
    StringTokenizer stok = new StringTokenizer(outStd,"\n");
    Vector outFiles = new Vector();
    while(stok.hasMoreTokens()) 
    {
      String fn = stok.nextToken();
      if(inFiles != null)
      {
        if(!inFiles.containsKey(fn))        // leave out input files
          outFiles.add(fn);
      }
      else
        outFiles.add(fn);
    }

    Enumeration en = outFiles.elements();
    while(en.hasMoreElements()) 
    {
      String key = (String)en.nextElement();
      byte fbuf[]=null;
     
      try
      {
        fbuf =  aj.getFile(userName,passwd,environ,
                                  project+"/"+key);
      }
      catch(Exception exp){}

//    appendToLogFile("getFile "+name+" STDERR "+aj.getErrStd(),errorLog);  //DEBUG

      if(aj.getFileok()==1)
      {
        result.add(key);
        if(aj.getPrnt() == 1)
          result.add(new String(fbuf));
        else
          result.add(fbuf);
      }
      else
        appendToLogFile("Cannot getFile "+project+"/"+key,errorLog);
    }

    return result;
  }


/**
*
* Used to provide information on the batch/background
* processes.
*
*/
  public Vector update_result_status(String prog, String opt,
                        Hashtable resToQuery,String userName,
                        byte[] passwd)
  {
    Ajax aj = new Ajax();
    Vector vans = new Vector();
    if(!verifyUser(aj,userName,passwd,vans))
      return vans;

    tmproot = tmproot.concat(userName+fs);

    Enumeration enum = resToQuery.keys();
    while (enum.hasMoreElements())
    {
      String thiskey = (String)enum.nextElement().toString();
      String thiselm = (String)resToQuery.get(thiskey);

      try
      {
        aj.getFile(userName,passwd,environ,
                 tmproot+fs+thiskey+fs+".finished");
      }
      catch(Exception ex){}

      if((aj.getErrStd().indexOf("stat error") == -1) &&
         (aj.getFileok()==1))
      {
        vans.add(thiskey);
        vans.add("complete");
        
        byte fbuf[] = aj.getFile(userName,passwd,environ,
                tmproot + fs + thiskey+ fs + ".desc");

        if(aj.getFileok()==1)
        {
          vans.add(thiskey+"-description");
          vans.add(new String(fbuf));
        }
      }
      else
      {
        vans.add(thiskey);
        vans.add("pending");
      }
    }

    for(int i=0;i<passwd.length;i++)
      passwd[i] = '\0';

    return vans;
  }


  private boolean verifyUser(Ajax aj, String userName, 
                            byte[] passwd, Vector res)
  {


    if(userName == null || passwd == null)
    {
//    System.out.println("Failed Authorisation "+userName);
      res.add("msg");
      res.add("Failed Authorisation ");
      res.add("status");
      res.add("1");
      return false;
    }

    boolean ok = false;

    try
    {
      ok = aj.userAuth(userName,passwd,environ);
    }
    catch(Exception exp) 
    {
      ok = false;
    }

    if(!ok)
    {
      appendToLogFile("Failed Authorisation "+userName,errorLog);
      appendToLogFile("STDERR "+aj.getErrStd(),errorLog);
      res.add("msg");
      res.add("Failed Authorisation "+userName);
      res.add("status");
      res.add("1");
      return false;
    }

//  appendToLogFile("userAuth STDERR "+aj.getErrStd(),errorLog);  //DEBUG
       
    return true;
  }


  public final Object clone() throws java.lang.CloneNotSupportedException
  {
    throw new java.lang.CloneNotSupportedException();
  }

}


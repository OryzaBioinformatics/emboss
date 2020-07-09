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


package org.emboss.jemboss.gui.form;

import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;
import java.net.URL;

import java.util.*;
import java.awt.event.*;
import java.io.*;
import org.emboss.jemboss.parser.*;
import org.emboss.jemboss.programs.*;
import org.emboss.jemboss.*;
import org.emboss.jemboss.gui.*;
import org.emboss.jemboss.soap.*;
import org.emboss.jemboss.gui.sequenceChooser.*;


/**
*
* Responsible for displaying the graphical representation
* of the ACD EMBOSS files.
*
* Generates & runs command lines and display results.
*
* @author T. J. Carver
*
*/
public class BuildJembossForm implements ActionListener 
{

  private TextFieldSink textf[];
  private TextFieldInt textInt[];
  private TextFieldFloat textFloat[];
  private JTextField rangeField[];
  private JCheckBox  checkBox[];
  private InputSequenceAttributes inSeqAttr[];
  protected static OutputSequenceAttributes outSeqAttr;

  private Box advSectionBox;
  protected static JPanel advSection;
  protected static JPanel reqdSection;
  protected static JPanel outSection;
  protected static JPanel inpSection;

//private JComboBox fieldOption[];
  private myComboPopup fieldOption[];
  private JList multiOption[];
  private SetInFileCard inSeq[];
  private JButton bresults;
  private String applName;
  private String db[];
  private String[] envp;

  private String cwd;
  private ParseAcd parseAcd;

  private Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
  private Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
// result files
  private String seqoutResult;
  private String outfileResult;
  private String stdout;
  private String helptext = "";
  private boolean withSoap;
  private JFrame f;
  private JPanel p2;
  private String embossBin;

  private int numofFields;
  private JembossParams mysettings;

  
  public BuildJembossForm(String appDescription, String db[],
        final String applName, String[] envp, String cwd, String embossBin,
        String acdText, final boolean withSoap, JPanel p2, 
        final JembossParams mysettings, final JFrame f)
  {

    this.f = f;
    this.p2 = p2;
    this.db = db;
    this.cwd = cwd;
    this.mysettings = mysettings;
    this.withSoap = withSoap;
    this.embossBin = embossBin;
    this.envp = envp;
    this.applName = applName;

    JPanel pC = new JPanel();
    pC.setBackground(Color.white);

    p2.add(pC, applName);
    pC.setLayout(new BorderLayout());

    Box fieldPane = Box.createVerticalBox();

    parseAcd = new ParseAcd(acdText,false);
    numofFields = parseAcd.getNumofFields();

    attach(pC, fieldPane, appDescription);

    JScrollPane scroll = new JScrollPane(fieldPane);
//  scroll.getViewport().setBackground(Color.lightGray);
    pC.add(scroll, BorderLayout.CENTER);

// get help for current application
    if(!withSoap) 
    {
      String command = embossBin.concat("tfm " + applName + " -nomore");
      RunEmbossApplication rea = new RunEmbossApplication(command,envp,null);
      rea.isProcessStdout();
      helptext = rea.getProcessStdout(); 
    }

// Help button
    ClassLoader cl = this.getClass().getClassLoader();
    JButton bhelp = new JButton(new ImageIcon(
           cl.getResource("images/Information_button.gif")));
    bhelp.addActionListener(this);
    bhelp.setMargin(new Insets(0,1,0,1));
    bhelp.setToolTipText("Help");

    bhelp.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        f.setCursor(cbusy);
        JFrame fhelp = new JFrame(applName + " Help");
        JPanel phelp = (JPanel)fhelp.getContentPane();
        phelp.setLayout(new BorderLayout());
        JPanel pscroll = new JPanel(new BorderLayout());
        JScrollPane rscroll = new JScrollPane(pscroll);
        phelp.add(rscroll, BorderLayout.CENTER);
    
        String text = "";
        if(!withSoap)
        {
          text = helptext;
        }
        else
        {
          GetHelp thishelp = new GetHelp(applName,mysettings);
          text = thishelp.getHelpText();
        }
        JTextArea helpText = new JTextArea(text);
        pscroll.add(helpText, BorderLayout.CENTER);
        helpText.setFont(new Font("monospaced", Font.PLAIN, 12));
        helpText.setCaretPosition(0);
        helpText.setEditable(false);
        fhelp.setSize(520,395);
        fhelp.setVisible(true);
        f.setCursor(cdone);
      }
    });


// Display results button
    bresults = new JButton("Show results");
    bresults.addActionListener(this);
    bresults.setMargin(new Insets(0,1,0,1));

// Go button
    
    ImageIcon rfii = new ImageIcon(cl.getResource("images/Go_button.gif"));
    JButton bgo = new JButton(rfii);
    bgo.setActionCommand("GO");
    bgo.setIcon(rfii);
    bgo.setMargin(new Insets(0,0,0,0));
    bgo.addActionListener(this);

// Advanced options
    JButton badvanced = new JButton("Advanced Options");
    badvanced.addActionListener(this);

    Box tools;
    tools = Box.createHorizontalBox();
    tools.add(Box.createRigidArea(new Dimension(2,0)));
    tools.add(bgo);
    tools.add(Box.createRigidArea(new Dimension(4,0)));
    tools.add(bhelp);
      
    if(advSectionBox!= null)
    {
      tools.add(Box.createRigidArea(new Dimension(4,0)));
      tools.add(badvanced);
    }

    tools.add(Box.createRigidArea(new Dimension(4,0)));
    tools.add(bresults);
    bresults.setVisible(false);
    tools.add(Box.createHorizontalGlue());
    fieldPane.add(Box.createRigidArea(new Dimension(0,10)));
    fieldPane.add(tools);

    fieldPane.add(Box.createVerticalStrut(10));
    bgo.setMinimumSize(new Dimension(200, 40));
    bgo.setMaximumSize(new Dimension(200, 40));

    if(advSectionBox!= null)
    {
      fieldPane.add(advSectionBox);
      advSectionBox.setVisible(false);
    }
     
    fieldPane.add(Box.createVerticalGlue());
  }


  public void attach(JPanel p3, Box fieldPane, 
                     String appDescription)
  {

    String appN = "";

// get total number of Swing components
    int ntextf = parseAcd.getNumTextf();
    int nint   = parseAcd.getNumNint();
    int nfloat = parseAcd.getNumNfloat();
    int nbool  = parseAcd.getNumBool();
    int nseqs  = parseAcd.getNumSeq();
    int nlist  = parseAcd.getNumList();
    int mlist  = parseAcd.getNumMList();
    int nrange = parseAcd.getNumRange();

    textf     = new TextFieldSink[ntextf];
    textInt   = new TextFieldInt[nint];
    textFloat = new TextFieldFloat[nfloat];
    checkBox  = new JCheckBox[nbool];
    inSeqAttr = new InputSequenceAttributes[nseqs];
    fieldOption = new myComboPopup[nlist];
    multiOption = new JList[mlist];
    rangeField  = new JTextField[nrange];

    inSeq  = new SetInFileCard[nseqs];
//  JRadioButton rpaste[] = new JRadioButton [nseqs];
    Box lab[] = new Box[numofFields];

    for(int j=0;j<ntextf;j++)
    {
      textf[j] = new TextFieldSink();
      Dimension d = new Dimension(150, 30);
      textf[j].setPreferredSize(d);
      textf[j].setMinimumSize(d);
      textf[j].setMaximumSize(d);
    }

    for(int j=0;j<nint;j++)
    {
      textInt[j] = new TextFieldInt();
      Dimension d = new Dimension(150, 30);
      textInt[j].setPreferredSize(d);
      textInt[j].setMinimumSize(d);
      textInt[j].setMaximumSize(d);
    }

    for(int j=0;j<nfloat;j++)
    {
      textFloat[j] = new TextFieldFloat();
      Dimension d = new Dimension(150, 30);
      textFloat[j].setPreferredSize(d);
      textFloat[j].setMinimumSize(d);
      textFloat[j].setMaximumSize(d);
    }
     
    for(int j=0;j<nrange;j++)
    {
      rangeField[j] = new TextFieldSink();
      Dimension d = new Dimension(150, 30);
      rangeField[j].setPreferredSize(d);
      rangeField[j].setMinimumSize(d);
      rangeField[j].setMaximumSize(d);
    }


    int nsection = parseAcd.getNumSection();
    if(nsection==0)
      nsection = 1;

    int nfield = 0;

    advSectionBox = null;
    advSection = null;
    reqdSection = null;
    outSection = null;
    inpSection = null;

    if(withSoap)
    {
      if(parseAcd.isBatchable() && 
         !parseAcd.getExpectedCPU().equalsIgnoreCase("low"))
        Jemboss.resultsManager.updateMode("batch");
      else
        Jemboss.resultsManager.updateMode("interactive");
    }

    for(int j=0;j<nsection;j++)
    {
      if(nfield < numofFields)
      {
        SectionPanel sp = new SectionPanel(f,p3,fieldPane,parseAcd,
              nfield,textf,textInt,textFloat,rangeField,checkBox,
              inSeqAttr,fieldOption,multiOption,inSeq,db,appDescription,
              lab,numofFields,mysettings,withSoap);

        if(sp.isAdvancedSection())
        {
          advSectionBox = sp.getSectionBox();
          advSection = sp.getSectionPanel();
        }
        else
        {
          fieldPane.add(sp.getSectionBox());
          if(sp.isInputSection())
            inpSection = sp.getSectionPanel();     
          else if(sp.isRequiredSection())
            reqdSection = sp.getSectionPanel();
          else if(sp.isOutputSection())
            outSection = sp.getSectionPanel();
        }
        nfield = sp.getFieldNum();
      }
    }
 

  }


/**
*
*  Action events Exit, Help, GO, & Show results
* 
*
*/
  public void actionPerformed(ActionEvent ae)
  {

    String line;
    String text = "";

    if( ae.getActionCommand().startsWith("Advanced Option"))
    {
      if(advSectionBox.isVisible())
      {
        advSectionBox.setVisible(false);
        p2.setVisible(false);
        p2.setVisible(true);
      }
      else
      {
        advSectionBox.setVisible(true);
        p2.setVisible(false);
        p2.setVisible(true);
      }
    }
    else if ( ae.getActionCommand().startsWith("GO"))
    {
      f.setCursor(cbusy);
      if(!withSoap)
      {
        String embossCommand = getCommand();;

        if(!embossCommand.equals("NOT OK"))
        {
          RunEmbossApplication rea = new RunEmbossApplication(
                                           embossCommand,envp,null);
          rea.isProcessStdout();
          stdout = rea.getProcessStdout();
          Process p = rea.getProcess();
          try
          {
            p.waitFor();
          } 
          catch (InterruptedException interre)
          {
            f.setCursor(cdone);
          }
          bresults.setVisible(true);
        }
      }
      else
      {
        Hashtable filesToMove = new Hashtable();
        String embossCommand = getCommand(filesToMove);

        if(!embossCommand.equals("NOT OK"))
        {
          if (mysettings.getUseAuth() == true)
            if (mysettings.getServiceUserName() == null)
              System.out.println("OOPS! Authentication required!");

          try
          {
            JembossRun thisrun = new JembossRun(embossCommand,"",
                                          filesToMove,mysettings);
            if (mysettings.getCurrentMode().equals("batch"))
            {
              JembossProcess er = new JembossProcess((String)thisrun.get("jobid"));
              Jemboss.resultsManager.addResult(er);
              Jemboss.resultsManager.updateStatus();
              if(!Jemboss.resultsManager.isAutoUpdate())
              {
//              System.out.println("Start new batch update thread");
                Jemboss.resultsManager.setAutoUpdate(true);
                String freq = (String)AdvancedOptions.jobMgr.getSelectedItem();
                int ind = freq.indexOf(" ");
                new BatchUpdateTimer(Integer.parseInt(freq.substring(0,ind)));
              }
            }
            else
              new ShowResultSet(thisrun.hash());
          }
          catch (JembossSoapException eae)
          {
            new AuthPopup(mysettings,f);
            f.setCursor(cdone);
          }
        }
      }
      f.setCursor(cdone);
    }
    else if( ae.getActionCommand().startsWith("Show results"))
    {
      JFrame res = new JFrame(applName + " Results  : " + seqoutResult);
      res.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
      JTabbedPane fresults = new JTabbedPane();
      res.getContentPane().add(fresults,BorderLayout.CENTER);
      Hashtable hashRes = new Hashtable();
      res.setSize(450,400);
      JPanel presults;
      JPanel pscroll;
      JScrollPane rscroll;

      if(!stdout.equals(""))
      {
        presults = new JPanel(new BorderLayout());
        pscroll = new JPanel(new BorderLayout());
        rscroll = new JScrollPane(pscroll);
        presults.add(rscroll, BorderLayout.CENTER);
        JTextArea atext = new JTextArea(stdout);
        atext.setFont(new Font("monospaced", Font.PLAIN, 12));
        pscroll.add(atext, BorderLayout.CENTER);
        atext.setCaretPosition(0);
        fresults.add("stdout",presults);
        hashRes.put("stdout",stdout);
      }

      for(int j=0;j<numofFields;j++) 
      {
        presults = new JPanel(new BorderLayout());
        pscroll = new JPanel(new BorderLayout());
        rscroll = new JScrollPane(pscroll);
        presults.add(rscroll, BorderLayout.CENTER);

        if(parseAcd.isOutputSequence(j) || parseAcd.isOutputFile(j))
        {
          try
          {
            BufferedReader in;
            if(parseAcd.isOutputSequence(j))
              in = new BufferedReader(new FileReader(seqoutResult));
            else
              in = new BufferedReader(new FileReader(outfileResult));

            while((line = in.readLine()) != null)
              text = text.concat(line + "\n");

            in.close();
            JTextArea seqText = new JTextArea(text);
            seqText.setFont(new Font("monospaced", Font.PLAIN, 12));
            pscroll.add(seqText, BorderLayout.CENTER);
            seqText.setCaretPosition(0);
            fresults.add(seqoutResult,presults);
            hashRes.put("stdout",text);
          }
          catch (IOException ioe)
          {
            System.out.println("Failed to open sequence file " + seqoutResult);
          }
        }
        else if (parseAcd.isOutputGraph(j))
        {
          File cwdFile = new File(cwd);
          String pngFiles[] = cwdFile.list(new FilenameFilter()
          {
            public boolean accept(File cwd, String name)
            {
              if(name.endsWith(".png"))
                return name.startsWith(applName);
              else
                return false;
            };
          });


          for(int i=0;i<pngFiles.length;i++)
          {
            presults = new JPanel(new BorderLayout());
            pscroll = new JPanel(new BorderLayout());
            rscroll = new JScrollPane(pscroll);
            presults.add(rscroll, BorderLayout.CENTER);
            ImageIcon icon = null;
            URL iconURL = ClassLoader.getSystemResource(pngFiles[i]);
            if (iconURL != null) {
              icon = new ImageIcon(iconURL);
              JLabel picture = new JLabel(icon);
              pscroll.add(picture);
              fresults.add(pngFiles[i],presults);
              hashRes.put(pngFiles[i],getPNGByte(pngFiles[i]));
            }
            else
            {
              System.out.println("Not opened file " +cwd+applName+".1.png");
            }
          }
        }
      }
      new ResultsMenuBar(res,fresults,hashRes);
      res.setVisible(true);
    }
  }


  public byte[] getPNGByte(String pngFiles)
  {
    byte data[] = new byte[1];
    int nby = 0;
    DataInputStream dis;
    FileInputStream fis;

    try
    {
      fis = new FileInputStream(pngFiles);
      dis = new DataInputStream(fis);
      while(true)
      {
        dis.readByte();
        nby++;
      }
    }
    catch (EOFException eof){}
    catch (IOException ioe){}

    if(nby >0)
    {
      try
      {
        data = new byte[nby];
        fis = new FileInputStream(pngFiles);
        dis = new DataInputStream(fis);
        nby=0;
        while(true)
        {
          data[nby]=dis.readByte();
          nby++;
        }
      }
      catch (EOFException eof){}
      catch (IOException ioe){}
    }
    return data;
  }

  public String checkParameters(ParseAcd parseAcd, int numofFields, 
                                Hashtable filesToMove)
  {

    String params = new String("");
    String appN = "";
    String file = "";
    String options = "";
    String fn = "";
    String sfn;
 
    seqoutResult   = "";
    outfileResult  = "";
   
    for(int j=0;j<numofFields;j++)
    {
      String att = parseAcd.getParameterAttribute(j,0).toLowerCase();
      String val = parseAcd.getParamValueStr(j,0).toLowerCase();
      int h = parseAcd.getGuiHandleNumber(j);

      if(att.startsWith("appl"))
        appN = new String(att);
      else if (parseAcd.isOutputGraph(j))
        options = options.concat(" -" + val + " png");

      if ( att.startsWith("datafile")|| att.startsWith("featout")||
           att.startsWith("string")  || att.startsWith("seqout") ||
           att.startsWith("outfile") || att.startsWith("matrix") ||
           att.startsWith("regexp") || att.startsWith("codon") ||
           att.startsWith("dirlist") )
      {
        if(!(textf[h].getText()).equals("") && textf[h].isVisible()
                                            && textf[h].isEnabled()) 
        {
          options = options.concat(" -" + val + " " + textf[h].getText());

          if(att.startsWith("seqout"))
            seqoutResult = textf[h].getText();
          else if(att.startsWith("outfile"))
            outfileResult = textf[h].getText();;
        }

        if(att.startsWith("seqout"))
          options = options.concat(outSeqAttr.getOuputSeqAttr());

      }
      else if ( att.startsWith("int") )
      {
        if( (textInt[h].getText() != null) && textInt[h].isVisible()
                                           && textInt[h].isEnabled())
          options = options.concat(" -" + val + " " + textInt[h].getValue());
      }
      else if ( att.startsWith("float") )
      {
        if( (textFloat[h].getText() != null) && textFloat[h].isVisible()
                                             && textFloat[h].isEnabled())
          options = options.concat(" -" + val + " " + textFloat[h].getValue());
      }
      else if ( att.startsWith("select") )   
      {

        double max = 1.;
        if(parseAcd.isMaxParamValue(j))
          max = Double.parseDouble(parseAcd.getMaxParam(j));
                                                            //defined by a number
        if(max > 1.0 && multiOption[h].isVisible()
                     && multiOption[h].isEnabled())         //multi selection
        {
          int sel[] = multiOption[h].getSelectedIndices();
          options = options.concat(" -" + val + " ");
          for(int i=0;i<sel.length;i++)
          {
            options = options.concat(Integer.toString(sel[i]+1));
            if(i<sel.length-1)
              options = options.concat(",");
          }
        }
        else if (fieldOption[h].isVisible() && fieldOption[h].isEnabled())
          options = options.concat(" -" + val + " " +        //single selection
                    (fieldOption[h].getSelectedIndex()+1));
      }
      else if ( att.startsWith("list") )    
      {
        double max = 1.;
        if(parseAcd.isMaxParamValue(j))
          max = Double.parseDouble(parseAcd.getMaxParam(j));
                                                             //defined by a label
        if(max > 1.0 && multiOption[h].isVisible()
                     && multiOption[h].isEnabled())          //multi selection
        {
          int sel[] = multiOption[h].getSelectedIndices();
          options = options.concat(" -" + val + " ");
          for(int i=0;i<sel.length;i++)
          {
            options = options.concat(parseAcd.getListLabel(j,sel[i]));
            if(i<sel.length-1)
              options = options.concat(",");
          }
        }
        else if (fieldOption[h].isVisible() 
              && fieldOption[h].isEnabled())
        {
          int index = fieldOption[h].getSelectedIndex();     //single selection
          options = options.concat(" -" + val + " " +
                       parseAcd.getListLabel(j,index));
        }
      }
      else if ( att.startsWith("bool") && checkBox[h].isVisible()
                                       && checkBox[h].isEnabled())
      {
        if( checkBox[h].isSelected() )
          options = options.concat(" -" + val + " ");
        else
          options = options.concat(" -no" + val + " ");

      }
      else if( att.startsWith("range") && rangeField[h].isVisible()
                                       && rangeField[h].isEnabled() )
      {
        if( !rangeField[h].getText().equals("") )
        {
          String rangeText = rangeField[h].getText();
          int blank;
   
          //remove spaces from the range (not sure if this is 
          //a good idea in all cases)!
          while ((blank = rangeText.indexOf(" ")) > -1)
          {
            String rangePrefix = rangeText.substring(0,blank);
            String rangeSuffix = rangeText.substring(blank+1,rangeText.length());
            rangeText = rangePrefix.concat(rangeSuffix);
          }
          options = options.concat(" -" + val + " " + 
                         rangeText + " ");
        } 
      }
      else if ( att.startsWith("infile") )
      {
        if(!(textf[h].getText()).equals("") && textf[h].isVisible()
                                            && textf[h].isEnabled())
        {
          if(withSoap)
            options = filesForSoap(textf[h].getText(),options,val,filesToMove);
          else
            options = options.concat(" -" + val + " " +  textf[h].getText());
        }

      }
      else if ( att.startsWith("seqset") || att.startsWith("seqall") ||
                att.startsWith("sequence") )
      {
        int seq = h+1;
        if(inSeq[h].isFileName())                     // file or database
        {
          fn = new String(inSeq[h].getFileChosen());
          
          fn = fn.trim();
          if((fn.indexOf(":")>-1) && (fn.indexOf(":\\") < 0))  //remove spaces
          {                                                    //from db entries
            int n;
            while((n = fn.indexOf(" ")) > -1)
              fn = new String(fn.substring(0,n) + fn.substring(n+1));
          }

          if(withSoap)
            options = filesForSoap(fn,options,val,filesToMove);
          else
            options = options.concat(" -" + val + " " +  fn);

          if(fn.endsWith(":") || fn.endsWith(":*"))
          {
             int n = JOptionPane.showConfirmDialog(f,
                       "Do you really want to extract\n"+
                       "the whole of " + fn + " databese?",
                       "Confirm the sequence entry",
                       JOptionPane.YES_NO_OPTION);
             if(n == JOptionPane.NO_OPTION)
             {
               options = "NOT OK";
               break;
             }
          }

        }
        else if(inSeq[h].isListFile())                     // list file
        {
          String fns = inSeq[h].getListFile();
          String ls = System.getProperty("line.separator");
          if(withSoap)
          {
            options = filesForSoap("internalList::internalList"+ls+
                                   fns,options,val,filesToMove);
          }
          else
          { 
            String fna[] = inSeq[h].getArrayListFile();
            for(int i=0;i<fna.length;i++)
              options = options.concat(" -" + val + " " +  fna[i]);
          }
        } 
        else                                               // cut 'n paste
        {
          String cp = inSeq[h].getCutNPasteText();
          fn = new String(applName + (new Integer(h)).toString());

          if(withSoap)
          {
            MakeFileSafe sf = new MakeFileSafe(fn);
            sfn = sf.getSafeFileName();
            filesToMove.put(sfn,cp);
            options = options.concat(" -" + val + " " + sfn);

          }
          else
          {
            try
            {
              File tf = File.createTempFile(fn, ".jembosstmp",
                                  new File(cwd));
              PrintWriter out = new PrintWriter(new FileWriter(tf));
              out.println(cp);
              out.close();
              fn = new String(tf.getCanonicalPath());
            } catch (IOException ioe) {}
            options = options.concat(" -" + val + " " + fn);
          }
        }

        options = options.concat(inSeqAttr[h].getInputSeqAttr(seq));

      } 
    }

    return options;
  }


  public String filesForSoap(String fn, String options, String val,
                             Hashtable filesToMove)
  {

    String sfn;

    if (fn.startsWith("@")||fn.startsWith("list::")||
        fn.startsWith("internalList::"))        // list file
    {
      String lfn = "";
      if (fn.startsWith("@"))
        lfn = fn.substring(1);
      else if(fn.startsWith("list::"))
        lfn = fn.substring(6);

      File inFile = new File(lfn);  
      if( (inFile.exists() && inFile.canRead() && 
           inFile.isFile())||
           fn.startsWith("internalList::") )    // local list file 
      {
        ListFile.parse(fn, filesToMove);
        if(fn.startsWith("internalList::"))
          options = options.concat(" -" + val + " list::internalList");
        else
        {
          MakeFileSafe sf = new MakeFileSafe(lfn);
          String sfs = sf.getSafeFileName();
          options = options.concat(" -" + val + " list::" +  sfs);
        }
      }
      else                                      // presume remote
      {
//      System.out.println("Can't find list file "+lfn);
        options = options.concat(" -" + val + " list::" +  lfn);
      }
      
      sfn=lfn;
    }
    else                                        // not list file
    {                                  
      MakeFileSafe sf = new MakeFileSafe(fn);
      sfn = sf.getSafeFileName();
      if ((new File(fn)).exists())    // read & add to transfer list
      {
        File inFile = new File(fn);
        if (inFile.exists() && inFile.canRead() && inFile.isFile())
        {
          try
          {
            BufferedReader in = new BufferedReader(new FileReader(inFile));
            String text = "";
            String line;
            while((line = in.readLine()) != null)
              text = text.concat(line+"\n");
            in.close();
 
            filesToMove.put(sfn,text);
          } catch (IOException e) {}
          options = options.concat(" -" + val + " " +  sfn);
        }
        else
        {
//        System.out.println("Ignoring invalid local file "+fn);
          options = options.concat(" -" + val + " " +  fn);
        }
      }
      else     //presume remote
      {
//      System.out.println("Can't find plain file "+fn);
        options = options.concat(" -" + val + " " +  fn);
      }
    }

    return options;
  }

/**
*
* Get the command line for the Standalone version.
* @param Hashtable of the files to be transferrred
* @return String command line to use
*
*/
  public String getCommand()
  {

    String command = embossBin.concat(applName);
    int numofFields = parseAcd.getNumofFields();

    String options = checkParameters(parseAcd, numofFields, new Hashtable());
         
    if(options.equals("NOT OK"))
      command = "NOT OK";
    else 
      command = command.concat(options + " -stdout -auto");

    return command;
  }


/**
*
* Get the command line for the SOAP version.
* @param Hashtable of the files to be transferrred
* @return String command line to use
*
*/
  public String getCommand(Hashtable filesToMove)
  {

    String command = applName;
    int numofFields = parseAcd.getNumofFields();

    String options = checkParameters(parseAcd, numofFields, filesToMove);

    if(options.equals("NOT OK"))
      command = "NOT OK";
    else
      command = command.concat(options + " -auto");

    return command;
  }


  /**
  *
  * Ensures garbaged collected when there are
  * no more pointers to this.
  * 
  */
  public void finalize() throws Throwable
  {
    super.finalize();
  }

}

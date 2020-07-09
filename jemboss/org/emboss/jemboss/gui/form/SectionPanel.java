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
import javax.swing.border.*;
import javax.swing.event.*;

import java.util.*;
import java.awt.event.*;
import java.io.*;

import org.emboss.jemboss.gui.AdvancedOptions;
import org.emboss.jemboss.parser.*;
import org.emboss.jemboss.programs.ListFile;
import org.emboss.jemboss.gui.sequenceChooser.*;
import org.emboss.jemboss.soap.CallAjax;
import org.emboss.jemboss.soap.JembossSoapException;
import org.emboss.jemboss.JembossParams;


/**
*
* Responsible for displaying the graphical representation
* of an ACD section. This also handles events related to
* dependent parameters.
*
*/
public class SectionPanel
{

  private TextFieldSink textf[];
  private TextFieldInt textInt[];
  private TextFieldFloat textFloat[];
  private JTextField rangeField[];
  private JCheckBox  checkBox[];
  private InputSequenceAttributes inSeqAttr[];

  private myComboPopup fieldOption[];
  private JList multiOption[];
  private SetInFileCard inSeq[];
  private Box lab[];
  private String db[];

  private ParseAcd parseAcd;
  private int numofFields;
  private int nf;
  public static int ajaxLength;
  public static float ajaxWeight;
  public static boolean ajaxProtein;
  private boolean withSoap;
  private String appName = "";
  private JPanel p3;
  private JPanel sectionPane;
  private Box sectionBox;

  private JembossParams mysettings;

// input, required, advanced & output sections
  private boolean isInp = false;
  private boolean isReq = false;
  private boolean isAdv = false;
  private boolean isOut = false;

  private boolean isShadedGUI;

  public static Color labelColor = new Color(0, 0, 0);
  public static Font labfont = new Font("SansSerif", Font.BOLD, 12);
  public static Font labfont2 = new Font("SansSerif", Font.BOLD, 11);

  private final int maxSectionWidth = 498;
  private JFrame f;


/**
*
* @param JFrame Jemboss frame
* @param JPanel ACD form panel
* @param Box containing all the fields 
* @param ParseAcd representing the ACD file to display as a form
* @param int field number
* @param TextFieldSink for the text fields in the form
* @param TextFieldInt for the integer fields in the form
* @param JCheckBox for the boolean switches
* @param InputSequenceAttributes for the input sequence(s)
* @param myComboPopup for the list/selection data types
* @param JList for multiple selection lists
* @param String array containing the databases
* @param String containing the one line description for the application
* @param Box for all the component labels
* @param int total number of fields
* @param JembossParams mysettings
*
* 
*/
  public SectionPanel(JFrame f, JPanel p3, Box fieldPane, 
            ParseAcd parseAcd, int nff, TextFieldSink textf[], 
            TextFieldInt textInt[], TextFieldFloat textFloat[],
            JTextField rangeField[], JCheckBox  checkBox[],
            InputSequenceAttributes inSeqAttr[],
            myComboPopup fieldOption[], JList multiOption[], SetInFileCard inSeq[],
            String db[], String des, Box lab[], int numofFields,
            JembossParams mysettings, boolean withSoap)
  {

    Border etched = BorderFactory.createEtchedBorder();
    isShadedGUI = AdvancedOptions.prefShadeGUI.isSelected();

    this.p3 = p3;
    this.textf = textf;
    this.textInt = textInt;
    this.textFloat = textFloat;
    this.rangeField = rangeField;
    this.checkBox = checkBox;
    this.inSeqAttr = inSeqAttr;
    this.fieldOption = fieldOption;
    this.multiOption = multiOption;
    this.inSeq = inSeq;
    this.numofFields = numofFields;
    this.db = db;
    this.lab = lab;
    this.parseAcd = parseAcd;
    this.mysettings = mysettings;
    this.withSoap = withSoap; 
    this.f = f;

//using JNI?
    nf = nff;

    String att = parseAcd.getParameterAttribute(nf,0).toLowerCase();

//set the ACD title
    if(att.startsWith("appl"))
    {
      setAppTitle(des,p3);
      nf++;
      att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
    }
  
    Box section = new Box(BoxLayout.Y_AXIS);
    fieldPane.add(Box.createRigidArea(new Dimension(0,10)));

    JLabel bxlab = new JLabel();
    bxlab.setFont(labfont);

    sectionPane = new JPanel(new GridLayout(1,1));
    sectionPane.setBorder(BorderFactory.createEtchedBorder());
    String nameSect = parseAcd.getParamValueStr(nf, 0);

    if(att.equals("section"))
    {
      TitledBorder title;
      title = BorderFactory.createTitledBorder(etched, 
                parseAcd.getInfoParamValue(nf).toLowerCase(),
                TitledBorder.LEFT,TitledBorder.TOP,
                new Font("SansSerif", Font.BOLD, 13),
                Color.blue);
      sectionPane.setBorder(title);

      String secType = parseAcd.getInfoParamValue(nf).toLowerCase();
      if(secType.startsWith("advanced "))
        isAdv = true;
      else if(secType.startsWith("input "))
        isInp = true;
      else if(secType.startsWith("output "))
        isOut = true;
      else if(secType.startsWith("required "))
        isOut = true;
      else
        System.out.println("Unknown section type " + secType);

      nf++;
      att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
    }

    String varName = parseAcd.getParamValueStr(nf, 0);


// loop over all the fields in the section or the all fields in the ACD
    while( !( att.equals("endsection")  && varName.equals(nameSect)) &&
            nf < numofFields )
    {
      if(!(att.equals("graph") || att.equals("xygraph")
        || att.equals("var")   || att.equals("variable")) )
      {
        int h = parseAcd.getGuiHandleNumber(nf);
        Box pan = new Box(BoxLayout.X_AXIS);
        section.add(pan);

        String l = getMinMaxDefault(null,null,null,nf);
        lab[nf] = setLabelText(parseAcd.getInfoParamValue(nf),
                               parseAcd.getHelpParamValue(nf));

        if(l != null && !att.startsWith("bool"))
        {
          bxlab = new JLabel(" " + l);
          bxlab.setFont(labfont2);
          bxlab.setForeground(Color.blue);
          lab[nf].add(bxlab);
        }

        if(att.startsWith("appl"))
        {
          setAppTitle(des,p3);
        }
        else if(att.startsWith("int"))
        {
          if(parseAcd.isDefaultParamValueStr(nf)) 
          {
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textInt[h].setValue(Integer.parseInt(parseAcd.getDefaultParamValueStr(nf)));
          }
          else
          {
            Double val = new Double(parseAcd.getDefaultParamValueDbl(nf));
            textInt[h].setValue(val.intValue());
          }
//        pan.add(Box.createRigidArea(new Dimension(2,30)));
          pan.add(textInt[h]);
          pan.add(lab[nf]);
        } else if(att.startsWith("float")) {
          if(parseAcd.isDefaultParamValueStr(nf))
          {
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textFloat[h].setValue(Double.parseDouble(parseAcd.getDefaultParamValueStr(nf)));
          }
          else
          {
            Double val = new Double(parseAcd.getDefaultParamValueDbl(nf) );
            textFloat[h].setValue(val.doubleValue());
          }
//        pan.add(Box.createRigidArea(new Dimension(2,30)));
          pan.add(textFloat[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("matrix") || att.startsWith("string") ||
                att.startsWith("infile") || att.startsWith("regexp") ||
                att.startsWith("codon")  || att.startsWith("featout") ||
                att.startsWith("dirlist") )
        {
          if(parseAcd.isDefaultParamValueStr(nf)) 
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textf[h].setText( parseAcd.getDefaultParamValueStr(nf));

//        pan.add(Box.createRigidArea(new Dimension(2,30)));
          pan.add(textf[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("seqout"))
        {
          new SetOutFileCard(pan,textf[h],labelColor,sectionPane);
        }
        else if(att.startsWith("outfile") || att.startsWith("datafile"))
        {
          if(parseAcd.isDefaultParamValueStr(nf))
            if( !(parseAcd.getDefaultParamValueStr(nf).startsWith("@") ||
                  parseAcd.getDefaultParamValueStr(nf).startsWith("$")) )
              textf[h].setText( parseAcd.getDefaultParamValueStr(nf));

          if(parseAcd.getInfoParamValue(nf).equals(""))
          {
            bxlab = new JLabel(" " + att + " file name");
            bxlab.setForeground(labelColor);
          }
          lab[nf].add(bxlab);
//        pan.add(Box.createRigidArea(new Dimension(2,30)));
          pan.add(textf[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("seqset")) 
        {
          inSeq[h] = new SetInFileCard(sectionPane,h,db,
                              "Multiple Sequence Filename",
                              appName,inSeqAttr,true);

          pan.add(inSeq[h].getInCard());
//        pan.add(section.add(Box.createVerticalStrut(5)));
        }
        else if(att.startsWith("sequence") || att.startsWith("seqall"))
        {
          inSeq[h] = new SetInFileCard(sectionPane,h,db,
                                "Sequence Filename",
                                appName,inSeqAttr,true);

          pan.add(inSeq[h].getInCard());
//        pan.add(section.add(Box.createVerticalStrut(5)));
        }
        else if(att.startsWith("range"))
        {
          pan.add(rangeField[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("bool"))
        {
          checkBox[h] = new JCheckBox();
          if(parseAcd.isDefaultParamValueStr(nf))
            if(parseAcd.getDefaultParamValueStr(nf).equalsIgnoreCase("Y") ||
               parseAcd.getDefaultParamValueStr(nf).equalsIgnoreCase("Yes") )
              checkBox[h].setSelected(true);

//        pan.add(Box.createRigidArea(new Dimension(2,30)));
          pan.add(checkBox[h]);
          pan.add(lab[nf]);
        }
        else if(att.startsWith("list") || att.startsWith("select"))
        {
//        pan.add(Box.createRigidArea(new Dimension(2,30)));
          String list[];
          if(att.startsWith("list"))
            list = parseAcd.getList(nf);
          else
            list = parseAcd.getSelect(nf);

          double max = 1.;
          if(parseAcd.isMaxParamValue(nf))
            max = Double.parseDouble(parseAcd.getMaxParam(nf));

          if(max > 1.0)
          {
            multiOption[h] = new JList(list);
            multiOption[h].setSelectionMode
               (ListSelectionModel.MULTIPLE_INTERVAL_SELECTION);
            JScrollPane scrollPane = new JScrollPane(multiOption[h]);
            Dimension d = new Dimension(150,100);
            scrollPane.setMinimumSize(d);
            scrollPane.setMaximumSize(d);
            scrollPane.setPreferredSize(d);
            pan.add(scrollPane);
          }
          else
          {
            fieldOption[h] = new myComboPopup(list);
            fieldOption[h].setSelectedIndex(
                    parseAcd.getListOrSelectDefault());
            Dimension d = fieldOption[h].getPreferredSize();
            d = new Dimension(150,(int)d.getHeight());
            
            fieldOption[h].setMaximumSize(d);
            fieldOption[h].setPreferredSize(d);
            pan.add(fieldOption[h]);
          }
          pan.add(lab[nf]);
        }

        pan.add(Box.createHorizontalGlue());
        section.add(Box.createVerticalStrut(10));
      }

//using jni?
      if(AdvancedOptions.prefjni.isSelected())
        checkDependents(section);

      if((att.startsWith("seqset") || att.startsWith("seqall")||
          att.startsWith("sequence")) && !isInp )
      {
        section.add(new Separator(new Dimension(350,10)));
        section.add(Box.createVerticalStrut(10));
      }

      nf++;
      if(nf<numofFields)
      {
        att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
        varName = parseAcd.getParamValueStr(nf, 0);
      }
    }

    nf++;
    sectionPane.add(section);
    sectionBox = new Box(BoxLayout.X_AXIS);

    sectionResize(sectionPane);
    sectionBox.add(Box.createRigidArea(new Dimension(2,0)));
    sectionBox.add(sectionPane);
    sectionBox.add(Box.createHorizontalGlue());
    
  }


  public JPanel getSectionPanel()
  {
    return sectionPane;
  }


  public Box getSectionBox()
  {
    return sectionBox;
  }


  public boolean isInputSection()
  {
    return isInp;
  }

  public boolean isOutputSection()
  {
    return isOut;
  }

  public boolean isRequiredSection()
  {
    return isReq;
  }

  public boolean isAdvancedSection()
  {
    return isAdv;
  }

/**
*
* @param String info text for the Component label
* @param String help text for the tool tip
* @return Box containing the label wrapped information label
*
*/
  public Box setLabelText(String sl, String tt)
  {
    int stop;
    int width = 335;
    JLabel l;
    Box blab = new Box(BoxLayout.Y_AXIS);

    l = new JLabel();
    FontMetrics fm = l.getFontMetrics(labfont);


    if(!sl.equals(""))
    {
      sl = sl.replace('\n',' ');
      String subLabel;

      while(fm.stringWidth(sl) > width)
      {
        stop = sl.lastIndexOf(" ");
        subLabel = sl.substring(0,stop);
        while(fm.stringWidth(subLabel) > width)
        {
          stop = subLabel.lastIndexOf(" ");
          subLabel = subLabel.substring(0,stop);
        }

        l = new JLabel(" " + subLabel);
        blab.add(l);
        sl = sl.substring(stop+1,sl.length());
        l.setFont(labfont);
        l.setForeground(labelColor);
        if(!tt.equals(""))
          l.setToolTipText(tt);
      }
      l = new JLabel(" " + sl);
      l.setFont(labfont);
      l.setForeground(labelColor);
      if(!tt.equals(""))
        l.setToolTipText(tt);
      blab.add(l);
    }

    return blab;
  }


  public int getFieldNum()
  {
    return nf;
  }

/**
*
* @param String short description of the program
* @param JPanel for the ACD form
*
*/
  private void setAppTitle(String des, JPanel p3)
  {
    appName = parseAcd.getParamValueStr(nf,0).toUpperCase();
    Box bylabP = new Box(BoxLayout.Y_AXIS);
    Box bxlabP = new Box(BoxLayout.X_AXIS);
    
    JLabel labP = new JLabel("<html><font size=+1><FONT COLOR=RED>"  +
                                  appName + "</FONT>");
    bxlabP.add(Box.createHorizontalStrut(10));
    bxlabP.add(labP);
    bxlabP.add(Box.createHorizontalGlue());
    bylabP.add(bxlabP);
    labP = new JLabel(des);
    labP.setForeground(Color.black);
    labP.setFont(labfont);

    bxlabP = new Box(BoxLayout.X_AXIS);
    bxlabP.add(Box.createHorizontalStrut(10));
    bxlabP.add(labP);
    bxlabP.add(Box.createHorizontalGlue());
    bylabP.add(bxlabP);
    p3.add(bylabP, BorderLayout.NORTH);
  }


/**
*
* @param String minimum value for parameter 
* @param String maximum value for parameter
* @param int the number in the ACD of the parameter field
* @param String of (min: max: default:) if specified
*
*/
  private String getMinMaxDefault(String min,String max, String def, int nfield)
  {

    String l = new String("");

    if(parseAcd.isMinParamValue(nfield) && min == null)
      min = parseAcd.getMinParam(nfield);
    if(parseAcd.isMaxParamValue(nfield) && max == null)
      max = parseAcd.getMaxParam(nfield);
    if(parseAcd.isDefaultParamValueStr(nfield) && def == null)
      def = parseAcd.getDefaultParamValueStr(nfield);

    if(min != null && !min.startsWith("$") 
                   && !min.startsWith("@")) 
    {
      l = l.concat("(min:" + min);
      if(parseAcd.isMaxParamValue(nfield) && !max.startsWith("$") 
                             && !max.startsWith("@"))
        l = l.concat(" max:" + max);

      if(parseAcd.isDefaultParamValueStr(nfield) && !def.startsWith("$") 
                             && !def.startsWith("@") && !def.equals(""))
        l = l.concat(" default:" + def + ") ");
      else
        l = l.concat(") ");
    }
    else if(parseAcd.isMaxParamValue(nfield) && !max.startsWith("$")
                             && !max.startsWith("@"))
    {
      l = l.concat("(max:" + max);
      if(parseAcd.isDefaultParamValueStr(nfield) && !def.startsWith("$")
                             && !def.startsWith("@") && !def.equals(""))
        l = l.concat(" default:" + def + ") ");
      else
        l = l.concat(") ");
    }
    else if(parseAcd.isDefaultParamValueStr(nfield) && !def.startsWith("$")
                             && !def.startsWith("@") && !def.equals(""))
    {
      l = l.concat("(default:" + def + ") ");
    }
    else
    {
      if(parseAcd.isMinParamValue(nfield) || parseAcd.isMaxParamValue(nfield) ||
         parseAcd.isDefaultParamValueStr(nfield) )
        l = "";
      else
        l = null;
    }

    return l;

  }


/**
*
* Checks for dependent variables and adds in action listeners
* @param Box
*
*/
  private void checkDependents(Box section)
  {

    final Cursor cbusy = new Cursor(Cursor.WAIT_CURSOR);
    final Cursor cdone = new Cursor(Cursor.DEFAULT_CURSOR);
    final String att = parseAcd.getParameterAttribute(nf,0).toLowerCase();
    final String varName = parseAcd.getParamValueStr(nf,0);
    final String valS = parseAcd.getParamValueStr(nf,0).toLowerCase();
    final int nff = nf;


    if(parseAcd.isDependents(valS,nf,numofFields)) 
    {
      final int h = parseAcd.getGuiHandleNumber(nf);
      final int nod = parseAcd.getNumOfDependents();
      final Dependent dep[] = parseAcd.getDependents();

      if (att.startsWith("seqset") || att.startsWith("seqall")||
          att.startsWith("sequence") ) 
      {
        Box left = new Box(BoxLayout.X_AXIS);
        JButton upload = new JButton("LOAD SEQUENCE ATTRIBUTES");

        upload.setToolTipText(
                "After entering your sequence above, click here. This\n" +
                "will display the input parameters for " + appName + "\n" +
                "that are dependent on the sequence attributes.");

        upload.setForeground(Color.red);
        Dimension d = upload.getPreferredSize();
        upload.setPreferredSize(new Dimension(maxSectionWidth, (int)d.getHeight()));

        left.add(upload);
        left.add(Box.createHorizontalGlue());
        section.add(left);
        section.add(Box.createVerticalStrut(10));

        final SetInFileCard sifc = inSeq[h];
        upload.addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            f.setCursor(cbusy);
            String fc = null;
            String fname;
            if(sifc.isFileName() || sifc.isListFile())
            {
              if(sifc.isListFile())
                fname = sifc.getSequence(1);
              else
                fname = sifc.getFileChosen();
              fc = AjaxUtil.getFileOrDatabaseForAjax(fname,db,f,withSoap);
            }
            else                                     // Cut-n-Paste
            {
              fc = sifc.getCutNPasteText();
            }    

            if(!withSoap && fc!=null)    //Ajax without SOAP
            {
              Ajax aj = new Ajax();
              boolean ok;
              if(att.startsWith("seqset"))
                ok = aj.seqsetType(fc);
              else
                ok = aj.seqType(fc);
              if(ok)
              {
                ajaxLength  = aj.length;
                ajaxWeight  = aj.weight;
                ajaxProtein = aj.protein;
                if( (updateBeginEnd(inSeqAttr[h].getBegSeq(),
                                    inSeqAttr[h].getEndSeq())) &&
                    (!att.startsWith("seqset")) &&
                    (!att.startsWith("seqall"))  )
                {
                  inSeqAttr[h].setBegSeq(1);
                  inSeqAttr[h].setEndSeq(aj.length);
                }
                resolveDependents(nod,dep,sifc.getFileChosen(),varName);
              }
              else
              {
                JOptionPane.showMessageDialog(sectionPane,
                          "Sequence not found." +
                          "Check the sequence entered.",
                          "Error Message", JOptionPane.ERROR_MESSAGE);
              }
            }
            else if(fc!=null)    //Ajax with SOAP
            {

              try
              {
                CallAjax ca = new CallAjax(fc,att,mysettings);  
                if(ca.getStatus().equals("0"))
                {
                  ajaxLength  = ca.getLength();
                  ajaxWeight  = ca.getWeight();
                  ajaxProtein = ca.isProtein();
                  int seqLen  = ca.getLength();
                  if( (updateBeginEnd(inSeqAttr[h].getBegSeq(),
                                    inSeqAttr[h].getEndSeq())) &&
                    (!att.startsWith("seqset")) &&
                    (!att.startsWith("seqall"))  )
                  {
                    inSeqAttr[h].setBegSeq(1);     
                    inSeqAttr[h].setEndSeq(seqLen);
                  }
                  resolveDependents(nod,dep,sifc.getFileChosen(),varName);
                }
                else
                {
                  JOptionPane.showMessageDialog(sectionPane,
                          "Sequence not found." +
                          "\nCheck the sequence entered.",
                          "Error Message", JOptionPane.ERROR_MESSAGE);
                }
//              System.out.println("PROPERTIES::: " + ajaxLength + " " + ajaxWeight );
              }
              catch (JembossSoapException eae)
              {
                System.out.println("Call to Ajax library failed");
              }
            }
            f.setCursor(cdone);
//          resolveDependents(nod,dep,sifc.getFileChosen(),varName);
          }
        });
      }
      else if(att.startsWith("list") || att.startsWith("select"))
      {
        double max = 1.;
        if(parseAcd.isMaxParamValue(nff))
          max = Double.parseDouble(parseAcd.getMaxParam(nff));

        if(max <= 1.0)
        {
          fieldOption[h].addActionListener(new ActionListener()
          {
            public void actionPerformed(ActionEvent e)
            {
              String sel = "";
              int index = fieldOption[h].getSelectedIndex();
              if(att.startsWith("select"))
                sel = new String((new Integer(index+1)).toString());
              else if(att.startsWith("list"))
                sel = new String(parseAcd.getListLabel(nff,index));
              resolveDependents(nod,dep,sel,varName);
            }
          });
        }
      }
      else if(att.startsWith("bool"))
      {
        checkBox[h].addActionListener(new ActionListener()
        {
          public void actionPerformed(ActionEvent e)
          {
            String sel = "";
            if(checkBox[h].isSelected())
              sel = new String("true");
            else
              sel = new String("false");
            resolveDependents(nod,dep,sel,varName);
          }
        });
      }
    }

  }


  private boolean updateBeginEnd(String s, String e)
  {
    if( ((s!=null) && (!s.equals(""))) ||
        ((e!=null) && (!e.equals(""))) )
    {
      int n = JOptionPane.showConfirmDialog(f,
            "Overwrite the input sequence \n" + 
            "start :" + s + "\n" + "end :" + e + 
            "\n" + "values already set?",
            "Confirm",
            JOptionPane.YES_NO_OPTION);

      if(n == JOptionPane.NO_OPTION)
        return false;
    }

    return true;
  }


  private void resolveDependents(int nod, Dependent dep[], String textVal, 
                                 String varName)
  {
    for(int i=0;i<nod;i++)
    {
      String exp = dep[i].getDependentExp();
      int field = dep[i].getDependentField();
      int param = dep[i].getDependentParam();

      AcdVarResolve avr = new AcdVarResolve(exp,textVal,varName,parseAcd,
                            numofFields,textf,textInt,textFloat,fieldOption,
                            checkBox);

//    System.out.println(exp + "EXP ==> " + avr.getResult() + " " + textVal);
      exp = avr.getResult();

      AcdFunResolve afr = new AcdFunResolve(exp);
      String result = afr.getResult();
      String att = parseAcd.getParameterAttribute(
                    dep[i].getDependentField(),0).toLowerCase();
      String type = dep[i].getDependentType();
//    System.out.println(varName + " RES => " + result +" "+ type +" att "+ att
//            +" : "+ parseAcd.getParamValueStr(dep[i].getDependentField(),0));
      int h = parseAcd.getGuiHandleNumber(field);


      if( (att.equals("list")     || att.equals("select")) && 
          (type.startsWith("req") || type.startsWith("opt")) ) 
      {
        double max = 1.;
        if(parseAcd.isMaxParamValue(field))
          max = Double.parseDouble(parseAcd.getMaxParam(field));

        if(max <= 1.0)
        {
          if(result.equals("false"))
            setShadingAndVisibility(fieldOption[h], false, field);
          else
            setShadingAndVisibility(fieldOption[h], true , field);
        }
        else
        {
          if(result.equals("false"))
            setShadingAndVisibility(multiOption[h], false, field);
          else
            setShadingAndVisibility(multiOption[h], true, field);
        }

      }
      else if(att.startsWith("datafile")|| att.startsWith("featout")  ||
              att.startsWith("string")  || att.startsWith("seqout") ||
              att.startsWith("outfile") || att.startsWith("matrix") ||
              att.startsWith("infile")  || att.startsWith("regexp") ||
              att.startsWith("codon")   || att.startsWith("dirlist") )
      {

        if( (type.startsWith("opt") || type.startsWith("req"))
                                    && result.equals("false"))
        {
          setShadingAndVisibility(textf[h], false, field);
        }
        else if ( (type.startsWith("opt") || type.startsWith("req"))
                                           && result.equals("true"))
        {
          setShadingAndVisibility(textf[h], true, field);
        }
        if(type.startsWith("def"))
          textf[h].setText(result);

      }
      else if(att.startsWith("int"))
      {
        if( (type.startsWith("opt") || type.startsWith("req"))
                                    && result.equals("false"))
        {
          setShadingAndVisibility(textInt[h], false, field);
        }
        else if ( (type.startsWith("opt") || type.startsWith("req"))
                                           && result.equals("true"))
        {
          setShadingAndVisibility(textInt[h], true, field);
        }
        if(type.startsWith("def"))
          textInt[h].setValue(Integer.parseInt(result));
      }
      else if(att.startsWith("float"))
      {
        if( (type.startsWith("opt") || type.startsWith("req"))
                                    && result.equals("false"))
        {
          setShadingAndVisibility(textFloat[h], false, field);
        }
        else if ( (type.startsWith("opt") || type.startsWith("req"))
                                           && result.equals("true"))
        {
          setShadingAndVisibility(textFloat[h], true, field);
        }
        if(type.startsWith("def"))
          textFloat[h].setValue(Double.parseDouble(result));
      }
      else if(att.startsWith("bool"))
      {
        if(type.startsWith("opt") || type.startsWith("req")) 
        {
          if(result.equals("false"))
            setShadingAndVisibility(checkBox[h], false, field);
          else 
            setShadingAndVisibility(checkBox[h], true, field);
        }
        else if(type.startsWith("def"))
        {
          if(result.equals("false"))
            checkBox[h].setSelected(false);
          else
            checkBox[h].setSelected(true);
        }
      }
                                              //n.b. no default labels on bools
      if(type.startsWith("def") && (!att.startsWith("bool"))) 
      {
        String l = getMinMaxDefault(null,null,result,field);
        ((JLabel)lab[field].getComponent(1)).setText(" " + l);
      }
      else if(type.startsWith("min"))
      {
        String l = getMinMaxDefault(result,null,null,field);
        ((JLabel)lab[field].getComponent(1)).setText(" " + l);
      }
      else if(type.startsWith("max"))
      {
        String l = getMinMaxDefault(null,result,null,field);
        ((JLabel)lab[field].getComponent(1)).setText(" " + l);
      }

    }

// use to resize sections
//
    if(!isShadedGUI)
    {
      sectionResize(BuildJembossForm.advSection);
      sectionResize(BuildJembossForm.reqdSection);
      sectionResize(BuildJembossForm.outSection);
    }

    p3.setVisible(false);  //this seems to be necessary to force 
    p3.setVisible(true);   //it to re-display sections properly!!

  }


  private void setShadingAndVisibility(Component c, boolean useThis, int field)
  {
    if(isShadedGUI)
    {
      c.setEnabled(useThis);
    }
    else
    {
      c.setVisible(useThis);
      lab[field].setVisible(useThis);
    }
  }


  private void sectionResize(JPanel p)
  {
    if(p != null)
    {
      Dimension min = p.getMinimumSize();
      int w = maxSectionWidth;
      int h = (int)min.getHeight();
      Dimension d = new Dimension (w,h);
      p.setMaximumSize(d);
      p.setPreferredSize(d);
      p.setVisible(false);
      p.setVisible(true);
    }
  }


}


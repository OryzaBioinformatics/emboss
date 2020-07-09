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

package org.emboss.jemboss.parser;

import org.emboss.jemboss.parser.acd.*;
import java.io.*;
import java.util.StringTokenizer;
import java.util.Hashtable;


/**
*
* ACD (Ajax command line definition) file parser.
*
*
*/

public class ParseAcd
{

  private Application current;
  private int numofFields = 0;
  private int numofParams;
  private ApplicationFields appF[] = new ApplicationFields[200];
  private String svalue;
  private double nvalue;
  private String attr;
  private Dependent dep[];
  private int numOfDependents;
  private String groupList = "";

  private int nsection;
  private int nsubsection;

// number of data types in ACD
  private int ntextf;
  private int nint;
  private int nfloat;
  private int nbool;
  private int nseqs;
  private int nlist;
  private int mlist;
  private int nrange;

  private int listdefault;

// Program groups
  private boolean isPrimaryGp;
  private boolean isSecondaryGp;
  private String primaryGp;
  private String secondaryGp;

  private static String LINE_SEPARATOR = 
             System.getProperty("line.separator");


/**
*
* The constructor takes the ACD as a string. 
*
* @param acdText String representation of the ACD file
* @param groups  Boolean determing whether just to retieve the groups 
*
*/

  public ParseAcd(String acdText, boolean groups) 
  {

    ApplicationFields variables[] = new ApplicationFields[20];
    int nvars = 0;

    int len;
    int colonPos=0;
    int braketPos=0;
    int ttype;

    String param;
    String line;

    nsection = 0;
    nsubsection = 0;


    try 
    {
      BufferedReader in = new BufferedReader(new StringReader(acdText));
      line = new String();
      char c;

      in.mark(2500);
      line = in.readLine();

// loop over all parameter definitions
      do 
      {
        line = line.trim();    // removes leading & trailing whitespace
        len = line.length();
     
        if(line.startsWith("#") || len ==0)
        {
          in.mark(2500);
          continue;
        }

        colonPos  = line.indexOf(':');
        if(colonPos < 0) colonPos  = line.indexOf('=');
        if(colonPos < 0) continue;

        String dataType = new String(line.substring(0,colonPos));

        braketPos = line.indexOf('[');

        if(braketPos >= 0) 
          param = new String(line.substring(colonPos+1,braketPos).trim());
        else 
          param = new String(line.substring(colonPos+1).trim());

        dataType = dataType.toLowerCase();
        if(dataType.startsWith("appl"))
          current = new Application(param);

        if(line.startsWith("var:") || line.startsWith("variable"))
        {
          param = param.trim();
          variables[nvars] = new ApplicationFields();
          variables[nvars].setNumberOfParam(2);
      
          int ns = param.indexOf(" ");
          if(ns > -1)
          {
            String value = param.substring(0,ns);
            value = value.replace('"',' ').trim();
            variables[nvars].setParam(0,dataType,value);
            value = param.substring(ns);
            value = value.replace('"',' ').trim();
            variables[nvars].setParam(1, "value", value);
          }
          nvars++;
          in.mark(2500);
          continue;
        }
        else if(line.startsWith("endsection"))
        {
          appF[numofFields] = new ApplicationFields();
          appF[numofFields].setNumberOfParam(1);
          appF[numofFields].setParam(0, dataType, param);
          numofFields++;
          in.mark(2500);
          continue;
        }
        else if(line.startsWith("section"))
        {
          if(param.equals("input") || param.equals("required") ||
             param.equals("output") || param.equals("advanced") )
            nsection++;
          else
            nsubsection++;
        }
 
        numofParams = 1;

        StreamTokenizer st = new StreamTokenizer(in);
        if(! line.endsWith("[")) 
        {                                              // rewind to start 
         in.reset();                                   // tokeniser on same line
         
         st.nextToken();
         st.nextToken();
         st.nextToken();
         st.nextToken();
        }

        in.mark(2500);
        do 
        {
          ttype = parseParam(in, st);
        } while(attr != null);

        appF[numofFields] = new ApplicationFields();
        appF[numofFields].setNumberOfParam(numofParams);
        numofParams = 0;
        appF[numofFields].setParam(numofParams, dataType, param);
 
        in.reset();
        do 
        {
          ttype = parseParam(in, st);
          // is the value a number or string
          if( ttype != java.io.StreamTokenizer.TT_NUMBER &&
              attr != null) 
          {
             if(nvars>0)
               svalue = resolveVariables(variables,nvars,svalue);

             appF[numofFields].setParam(numofParams, attr, svalue);
//           System.out.println(" ATTR " + attr + " SVALUE " + 
//                          getParamValueStr(numofFields,numofParams));
          }   
          else if ( ttype == java.io.StreamTokenizer.TT_NUMBER) 
          {
//           System.out.println(" ATTR " + attr + " NVALUE " + nvalue);
             appF[numofFields].setParam(numofParams, attr, nvalue);
          }

        } while (attr != null);


// set gui handle
        if ( dataType.startsWith("datafile") || dataType.startsWith("featout")||
             dataType.startsWith("string")   || dataType.startsWith("seqout") ||
             dataType.startsWith("outfile")  || dataType.startsWith("matrix") ||
             dataType.startsWith("infile")   || dataType.startsWith("regexp") ||
             dataType.startsWith("codon") )
        {
          appF[numofFields].setGuiHandleNumber(ntextf);
          ntextf++;
        }
        else if (dataType.startsWith("int"))
        {
          appF[numofFields].setGuiHandleNumber(nint);
          nint++;
        }
        else if (dataType.startsWith("float"))
        {
          appF[numofFields].setGuiHandleNumber(nfloat);
          nfloat++;
        }
        else if (dataType.startsWith("bool"))
        {
          appF[numofFields].setGuiHandleNumber(nbool);
          nbool++;
        }
        else if (dataType.startsWith("seqset") || dataType.startsWith("seqall")||
                 dataType.startsWith("sequence") )
        {
          appF[numofFields].setGuiHandleNumber(nseqs);
          nseqs++;
        }
        else if (dataType.startsWith("list") || dataType.startsWith("select"))
        {
          double max = 1.;
          if(isMaxParamValue(numofFields))
            max = Double.parseDouble(getMaxParam(numofFields));

          if(max > 1.0)       // multiple list selection
          {
            appF[numofFields].setGuiHandleNumber(mlist);
            mlist++;
          }
          else                // single selection
          {
            appF[numofFields].setGuiHandleNumber(nlist);
            nlist++;
          }
        }
        else if (dataType.startsWith("range"))
        {
          appF[numofFields].setGuiHandleNumber(nrange);
          nrange++;
        }

        numofFields++;
        in.mark(2500);

        if(groups && dataType.startsWith("appl")) 
        {
           setGroups(--numofFields);
           break;
        }
        
      } while((line = in.readLine()) != null);
   
    }
    catch (IOException e) 
    {
      System.out.println("Parsing acd file error" );
    }

  }


  private String resolveVariables(ApplicationFields variables[],int nvars,
                                  String svalue)
  {
    String res=svalue;

    for(int i=0; i<nvars;i++)
    {
      String vName  = variables[i].getParamValueStr(0);
      String vValue = variables[i].getParamValueStr(1);
      AcdVariableResolve avresolve = new AcdVariableResolve(res,vName,vValue);
      res = avresolve.getResult();
    }

//  if(!res.equals(svalue))
//    System.out.println("START VALUE " + svalue  + " FINAL VALUE" + res + "\n");
    return res;
  }

/**
*
* Gets the application field.
*
* @return ApplicationField 
*
*/
  public ApplicationFields[] getApplicationFields() 
  {
    return appF;
  }


/**
*
* Gets the handle for a gui component on the Jemboss form.
*
* @param  int field number in the ACD file
* @return Handle integer.
*
*/
  public int getGuiHandleNumber(int field) 
  {
    return appF[field].getGuiHandleNumber();
  }


/**
*
* Gets the number of float, string, seqout, outfile,
* infile, regexp, codon & featout data types in the ACD.
* These make up the number of JTextAreas in the Jemboss form.
* @return Number of TextArea's in the Jemboss form.
*
*/
  public int getNumTextf() 
  {
    return ntextf;
  }

/**
*
* Gets the number of int data types in the ACD.
* @return Number of TextFieldInt in the Jemboss form.
*
*/
  public int getNumNint()
  {
    return nint;
  }

/**
*
* Gets the number of float data types in the ACD.
* @return Number of TextFieldFloat in the Jemboss form.
*
*/
  public int getNumNfloat()
  {
    return nfloat;
  }


/**
*
* Gets the number of boolean data types in the ACD.
* @return Number of check boxes in the Jemboss form.
*
*/
  public int getNumBool() 
  {
    return nbool;
  }


/**
*
* Gets the number of seqset, seqall & sequence data types in the ACD
* @return Number of sequence inputs in the Jemboss form.
*
*/
  public int getNumSeq() 
  {
    return nseqs;
  }

/**
*
* Gets the number of list & selection data types in the ACD,
* using single list selection
* @return Number of selection lists in the Jemboss form.
*
*/
  public int getNumList() 
  {
    return nlist;
  }

/**
*
* Gets the number of list & selection data types in the ACD,
* using multiple list selection
* @return Number of selection lists in the Jemboss form.
*
*/
  public int getNumMList()
  {
    return mlist;
  }


/**
*
* @return Number of range data types in the Jemboss form.
*
*/
  public int getNumRange()
  {
    return nrange;
  }


/**
*
* Gets the number of sections in the ACD
* @return Number of sections in the Jemboss form.
*
*/
  public int getNumSection()
  {
    return nsection;
  }


/**
*
* Gets the number of nested sections in the ACD
* @return Number of subsections in the Jemboss form.
*
*/
  public int getNumSubsection()
  {
    return nsubsection;
  }


/**
*
* Get a specified parameter attribute.
* @param int field number in the ACD file
* @param int parameter number in that field
* @return String of the value.
*
*/
  public String getParameterAttribute(int field, int param)
  {
    return appF[field].getParamAttribute(param);
  }


/**
*
* Determine if the value of the parameter is a String.
* @return True if the value of the parameter is a String.
*
*/
  public boolean isParamValueStr(int field, int param) 
  {
    return appF[field].isParamValueStr(param);
  }


/**
*
* Gets the String value of a parameter.
* @return String value of the parameter. 
*
*/
  public String getParamValueStr(int field, int param) 
  {
    return appF[field].getParamValueStr(param).trim();
  }


  public void setParamValueStr(int field, int param, String svalue)
  {
    appF[field].resetParam(param,svalue);
  }

/**
*
* Gets the double value of a parameter.
* @return Double value of the parameter. 
*
*/
  public double getParamValueDbl(int field, int param) 
  {
    return appF[field].getParamValueDbl(param);
  }


/**
*
* Determine if there is a default parameter in a field of the ACD.
* @param  int field number
* @return True if the default parameter value is a String
*
*/
  public boolean isDefaultParamValueStr(int field) 
  {
    int num = getNumofParams(field);
    int i;

    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("def")) 
        return isParamValueStr(field,i);
    }
    return false;
  }


/**
*
* Return a double default parameter
* @param  int field number
* @return Default parameter value as a double.
*
*/
  public double getDefaultParamValueDbl(int field) 
  {
    int num = getNumofParams(field);
    
    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("def")) 
      {
        return appF[field].getParamValueDbl(i);
      }
    }
    return 0.0;
  }




//  Methods for dealing with dependent variables. 

/**
*
* Always start by calling isDependents(), which calculates
* the number of dependents and construct the Dependent array.
*
* @param   String name of the attribute
* @param   int field number
* @param   int toatal number of fields  
* @return  True if there are dependent parameters.
*
*/
  public boolean isDependents(String attr, int field, int numofFields)
  {

    numOfDependents = 0;

    for(int i=field+1;i<numofFields;i++) 
    {
      int num = getNumofParams(i);
      for(int j=0;j<num;j++) 
      {
        if(isParamValueStr(i,j))   
        {
          String val = getParamValueStr(i,j);
          if(val.startsWith("$") || val.startsWith("@")) 
            numOfDependents++;
        }
      }
    }

    dep = new Dependent[numOfDependents];
    
    numOfDependents = 0;
    for(int i=field+1;i<numofFields;i++) 
    {
      int num = getNumofParams(i);

      for(int j=0;j<num;j++) 
      {
        if(isParamValueStr(i,j)) 
        {
          String val = getParamValueStr(i,j);
          if(val.startsWith("$") || val.startsWith("@")) 
          {
            String type = getParameterAttribute(i,j);
            dep[numOfDependents] = new Dependent(i,j,val,type);
            numOfDependents++;            
          }
        } 
      }
    }

    if(numOfDependents>0)
      return true;
    else
      return false;
  }


/**
*
* Gets the dependents associated with the ACD.
* @return the Dependent array for those fields
*         with dependents
*
*/
  public Dependent[] getDependents() 
  {
    return dep;
  }


/**
*
* Gets the number of dependent parameters in the ACD.
* @return Number of Dependent making up the array
*
*/
  public int getNumOfDependents() 
  {
    return numOfDependents;
  }



// Methods for finding min/max values for parameters

/**
*
* Locates the min parameter in a field and returns it as a String.
* @param   int field number
* @return  The minimum value defined in a field.
*
*/
  public String getMinParam(int field) 
  {
    int num = getNumofParams(field);
    String min = "";

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("min")) 
      {
        if(isParamValueStr(field,i)) 
          min = appF[field].getParamValueStr(i);
        else 
        {
          Double val = new Double(appF[field].getParamValueDbl(i));
          if(getParameterAttribute(field,0).startsWith("int"))
            min = (new Integer(val.intValue())).toString();           
          else
            min = val.toString();
        }
        return min;
      }
    }
   
    return min;
  }


/**
*
* Locates the max parameter in a field and returns it as a String.
* @param   int field number
* @return  The maximum value defined in a field.
*
*/
  public String getMaxParam(int field) 
  {
    int num = getNumofParams(field);
    String max = "";

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("max")) 
      {
        if(isParamValueStr(field,i))
          max = appF[field].getParamValueStr(i);
        else 
        {
          Double val = new Double(appF[field].getParamValueDbl(i));
          if(getParameterAttribute(field,0).startsWith("int"))
            max = (new Integer(val.intValue())).toString();
          else
            max = val.toString();
        }
        return max;
      }
    }
    return max;
  }


/**
*
* Determine if there is a min parameter in a field of the ACD.
* @param   int field number
* @return  True if there is a minimum value specified
*          in a field.
*
*/
  public boolean isMinParamValue(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("min")) 
      {
        if(isParamValueStr(field,i) )
          if(appF[field].getParamValueStr(i).startsWith("$"))
            return false;
        return true;
      }
    }
    return false;
  }


/**
*
* Determine if there is a max parameter in a field of the ACD.
* @param   int field number
* @return  True if there is a maximum value specified
*          in a field.
*
*/
  public boolean isMaxParamValue(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("max"))    
      {
//      if(isParamValueStr(field,i) )
//        if(appF[field].getParamValueStr(i).startsWith("$"))
//          return false;
        return true;
      }
    }
    return false;
  }


/**
*
* Finds the prompt, info or help parameter in an ACD field, in that
* order.
* @param   int field number
* @return  Information specified by the prompt parameter, if
*          present else the information parameter
*          or the help parameter.
*
*/
  public String getInfoParamValue(int field) 
  {
    int num = getNumofParams(field);
    int i;

    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("prompt")) 
        return appF[field].getParamValueStr(i);
    }
    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("info")) 
        return appF[field].getParamValueStr(i);
    }
    for(i=0;i<num;i++)
    {
      if (getParameterAttribute(field,i).startsWith("help"))
        return appF[field].getParamValueStr(i);
    }

    return "";
  }


/**
*
* Finds the help parameter in an ACD field.
* @param   int field number
* @return  Help parameter defined in a field or a blank
*          String if not specified.
*
*/
  public String getHelpParamValue(int field) 
  {
    int num = getNumofParams(field);
    int i;
    String help = "";
    String helpText = "";

    for(i=0;i<num;i++) 
      if (getParameterAttribute(field,i).startsWith("help")) 
        return formatHelpText(appF[field].getParamValueStr(i));

//  for(i=0;i<num;i++) 
//    if (getParameterAttribute(field,i).startsWith("info")) 
//      return formatHelpText(appF[field].getParamValueStr(i));
    
    return "";
  }

/**
*
* Finds if the program is identifies as being able to
* run in a batch queue
* @return true if the program is suitable for putting
*         in a batch queue 
*
*/
  public boolean isBatchable()
  {

    int num = getNumofParams(0);
    for(int i=0;i<num;i++)
    {
      if(getParameterAttribute(0,i).startsWith("batch"))
        if(appF[0].getParamValueStr(i).equalsIgnoreCase("Y") ||
           appF[0].getParamValueStr(i).equalsIgnoreCase("Yes") )
             return true;
    }

    return false;
  }


/**
*
* Finds any expected cpu level indicator (low, medium, high)
* @return cpu level indicator
*
*/
  public String getExpectedCPU()
  {
    String cpu = "";
    int num = getNumofParams(0);
    for(int i=0;i<num;i++)
    {
      if(getParameterAttribute(0,i).startsWith("cpu"))
        return appF[0].getParamValueStr(i);
    }

    return cpu;
  }

/**
*
* Limits the length of the line for the help text used in
* the tool tips.
*
* @param   String help text
* @return  formated help text.
*
*/
  protected String formatHelpText(String help) 
  {
    String helpText = "";
    int start = 0;
    int stop;

    help = help.replace('\n',' ');
    while((stop = help.indexOf(' ',55))>0)
    {
      helpText = helpText.concat(help.substring(start,stop) + "\n");   
      help = help.substring(stop+1,help.length());
    }
    helpText = helpText.concat(help);

    return helpText;
  }


/**
*
* Determine if there is a optional parameter in a field of the ACD.
* @param   int field number
* @return  True if this is an "optional" field.
*
*/
  public boolean isOptionalParamValue(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if(getParameterAttribute(field,i).startsWith("opt")) 
        return true;
    }
    return false;
  }


/**
*
* Determine if data type of a field is seqout.
* @param   int field number
* @return  True if this is an "seqout" field.
*
*/
  public boolean isOutputSequence(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("seqout"))
          return true;
    }
    return false;
  }


/**
*
* Gets the name of the output sequence field (seqout).
* @param   int field number
* @return  The parameter name for the seqout data type.
*
*/
  public String getOutputSequenceFile(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
      if (getParameterAttribute(field,i).startsWith("seqout")) 
          return getParamValueStr(field,i);
    return "";
  }


/**
*
* Determine if a field is data type graph or xygraph.
* @param   int field number
* @return  True if the field is of "graph" or "xygraph" type.
*
*/
  public boolean isOutputGraph(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if ( getParameterAttribute(field,i).startsWith("graph") ||
           getParameterAttribute(field,i).startsWith("xygraph") )
          return true;
    }
    return false;
  }


/**
*
* Determine if a field is data type outfile.
* @param   int field number
* @return  True if the field is of "outfile" type.
*
*/
  public boolean isOutputFile(int field)   
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("outfile"))
          return true;
    }
    return false;
  }


/**
*
* Finds the primary and secondary groups, defined in the
* ACD file. Call this before using the other group methods.
*
* @param   int field number
* 
*/
  public void setGroups(int field) 
  {
    
    int num = getNumofParams(field);
    int i;
    int sep;

    isPrimaryGp   = false;
    isSecondaryGp = false;

    for(i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("groups")) 
      {
       isPrimaryGp = true;
       groupList = appF[field].getParamValueStr(i);
       sep = groupList.indexOf(":");
       if(sep > 0) 
       {
         primaryGp = new String(groupList.substring(0,sep).trim());
         isSecondaryGp = true;
       } 
       else
       {
         primaryGp = new String(groupList);
       }
       if(isSecondaryGp) 
       {
         secondaryGp = new String(groupList.substring(sep+1,groupList.length()));
         sep = secondaryGp.indexOf(",");
         if(sep > 0)
           secondaryGp = new String(secondaryGp.substring(0,sep));
       }
       sep = primaryGp.indexOf(",");
       if(sep > 0) 
         primaryGp = new String(primaryGp.substring(0,sep));
  
       return;
      }
    }
    return;
  }

/**
*
* True if a primary group is defined.
* @return  True if a primary group is defined.
*
*/
  public boolean isPrimaryGroup()
  {
    return isPrimaryGp;
  }

/**
*
* True if a secondary group is defined.
* @return  True if a secondary group is defined.
*
*/
  public boolean isSecondaryGroup()
  {
    return isSecondaryGp;
  }

/**
*
* Gets the primary groups the application is a member of.
* @return  Primary groups the application is a member of.
*
*/
  public String getPrimaryGroup() 
  {
    return primaryGp;
  }


/**
*
* Gets the secondary groups the application is a member of.
* @return  Secondary groups the application is a member of.
*
*/
  public String getSecondaryGroup() 
  {
    return secondaryGp;
  }


/**
*
* Gets the list of groups defined by setGroups.
* @return Group list defined in setGroups().
*
*/
  public String getGroups() 
  {
    return groupList;
  }


/**
*
* Gets a String default parameter.
* @param  int field number
* @return Default parameter for this field.
*
*/
  public String getDefaultParamValueStr(int field) 
  {
    int num = getNumofParams(field);

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("def")) 
        return appF[field].getParamValueStr(i);
    }
    return "";
  }


/**
*
* Used for a list data type to put the list items in a String array.
* @param  int field number
* @return String array representation of the list type.
*
*/
  public String[] getList(int field) 
  {
    int num = getNumofParams(field);

    String delim     = ";";  // list item delimeter default
    String codedelim = ":";  // label delimeter default
    String listAll = null;
    String list[];
    String item;

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("val")) 
        listAll = appF[field].getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("delim")) 
        delim = appF[field].getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("codedelim"))
        codedelim = appF[field].getParamValueStr(i);
    }

    if(delim == null || listAll == null)
       System.out.println("getList ERROR");

    StringTokenizer st = new StringTokenizer(listAll,delim);

    int n = 0;
    while (st.hasMoreTokens()) 
    {
      st.nextToken(delim);
      n++;
    }

    list = new String[n];
    
    st = new StringTokenizer(listAll);
    n = 0;
    listdefault = 0;

    while (st.hasMoreTokens()) 
    {
      String key = st.nextToken(codedelim);
      item = st.nextToken(delim);
      if(isDefaultParamValueStr(field))
      {
        int newline = key.indexOf("\n");
        if(newline > -1)
          key = key.substring(newline+1,key.length());
        if (key.equals(getDefaultParamValueStr(field)))
          listdefault = n;
      }
      item = item.substring(1,item.length()).trim();
      list[n] = new String(item); 
      n++;
    }

    return list;
  }


/**
*
* For a list data type determine the appropriate String entry.
* @param  int field number
* @param  int index into the list
* @return String for that entry.
*
*/
  public String getListLabel(int field, int index) 
  {
    int num = getNumofParams(field);

    String delim     = ";";  // list item delimeter default
    String codedelim = ":";  // label delimeter default
    String listAll = null;
    String item;
    String key="";


    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("val"))
        listAll = appF[field].getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("delim"))
        delim = appF[field].getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("codedelim"))
        codedelim = appF[field].getParamValueStr(i);
    }

    if(delim == null || listAll == null)
       System.out.println("getList ERROR");

    StringTokenizer st = new StringTokenizer(listAll,delim);

    st = new StringTokenizer(listAll);

    for(int i=0;i<index+1;i++) 
    {
      key = st.nextToken(codedelim);
      item = st.nextToken(delim);
    }
    if(index>0)
      key = key.substring(1,key.length()).trim();
    else
      key = key.substring(0,key.length()).trim();

    return key;

  }


/**
*
* Used for a selection type to put the list items in a String array.
* @param  int field number
* @return String array representation of the select type.
*
*/
  public String[] getSelect(int field) 
  {
    int num = getNumofParams(field);

    String delim     = ";";  // select item delimeter default
    String listAll = null;
    String list[];
    String item;

    for(int i=0;i<num;i++) 
    {
      if (getParameterAttribute(field,i).startsWith("val"))
        listAll = appF[field].getParamValueStr(i);
      if (getParameterAttribute(field,i).startsWith("delim"))
        delim = appF[field].getParamValueStr(i);
    }

    if(delim == null || listAll == null)
       System.out.println("getSelect ERROR");

    StringTokenizer st = new StringTokenizer(listAll,delim);

    int n = 0;
    while (st.hasMoreTokens()) 
    {
      st.nextToken(delim);
      n++;
    }

    list = new String[n];
   
    st = new StringTokenizer(listAll);
    n = 0;
    listdefault = 0;

    while (st.hasMoreTokens()) 
    {
      item = st.nextToken(delim);
      if(isDefaultParamValueStr(field))
      {
        if (item.endsWith(getDefaultParamValueStr(field)))
          listdefault = n;
      }

      item = item.substring(0,item.length()).trim();
      list[n] = new String(item);
      n++;
    }

    return list;
  }


/**
*
* Use this after getList or getSelect to retrieve default
* @return int default for list or select data type
*
*/
  public int getListOrSelectDefault()
  {
    return listdefault;
  }


/**
*
* Determine if there is a optional parameter in any field of the ACD.
* @param  True if there is an optional parameter in any field.
*
*/
  public boolean isOptionalParam()
  {
     
    for(int i=0;i<numofFields;i++)
      if(isOptionalParamValue(i)) return true; 
    
    return false;
  }

/**
*
* Gets the number of fields in the ACD file.
* @return  Total number of fields in an ACD file
*
*/
  public int getNumofFields() 
  {
    return numofFields;
  }


/**
*
* Gets the number of parameters in a ACD field.
* @param  int field number
* @return number of parameters in the field.
*
*/ 
  public int getNumofParams(int field) 
  {
    return appF[field].getNumberOfParam();
  }


/**
*
* Parses each parameter in a field
*
* @param  BufferedReader 
* @param  StreamTokenizer
* @return int
*
*/
  public int parseParam(BufferedReader in, StreamTokenizer st) throws IOException 
  {
 
   char c;
   svalue = null;   

   st.eolIsSignificant(false);
   
// the following are not token delimeters
   st.wordChars((int)'-',(int)'-'); st.wordChars((int)'$',(int)'$');
   st.wordChars((int)'(',(int)'('); st.wordChars((int)')',(int)')');
   st.wordChars((int)'@',(int)'@'); st.wordChars((int)'?',(int)'?');
   st.wordChars((int)'!',(int)'!'); 
// the following are token delimeters
   st.whitespaceChars((int)'\n',(int)'\n');
   st.whitespaceChars((int)' ',(int)' ');
   st.whitespaceChars((int)':',(int)':');
   st.whitespaceChars((int)'=',(int)'=');
   st.ordinaryChars((int)'\"', (int)'\"');
   st.ordinaryChars((int)'\'',(int)'\'');
   st.wordChars((int)'#', (int)'#');

// loop over all parameter settings

   st.nextToken();

   attr = st.sval;

   if(attr == null) return 0;

//skip commented lines
   if(attr.startsWith("#")) 
   {
    while( (c = (char)in.read()) != '\n') {}
    st.nextToken();
    attr = st.sval;
   }
   if(attr == null) return 0;

   st.nextToken();
   svalue = st.sval;
   nvalue = st.nval;

   // cope with double quotes
   if( svalue == null &&
       st.ttype != java.io.StreamTokenizer.TT_NUMBER ) 
   {
     svalue = "";
     while( (c = (char)in.read()) != '\"') 
     {
       svalue = svalue.concat(java.lang.String.valueOf(c));
     }
   }

   numofParams++;
   return st.ttype;

  }

  
}



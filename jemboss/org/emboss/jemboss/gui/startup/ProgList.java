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


package org.emboss.jemboss.gui.startup;

import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;


/**
*
* Used on startup of Jemboss to calculate the alphabetical list
* of programs and uses the programs groups to create the  
* menu structure based on the program type. This uses the output 
* of the EMBOSS program wossname.
*
*/
public class ProgList 
{

   /** array of the program names */
   private String allProgs[];
   /** array of program one line descriptions */
   private String allDescription[];
   /** number of programs */
   private int numProgs;
   /** number of JMenuItem created */
   private int nm;
   /** number of primary group programs */
   private int npG;
   /** used to create the program menus */
   private JMenuItem mItem[];
   /** font used for the menu items */
   private Font menuFont = new Font("SansSerif", Font.BOLD, 11);
// private Font menuFont = new Font(null, Font.BOLD, 10);

   public ProgList(String woss, String currentDirectory,
                   JMenuBar menuBar)
   {

     numProgs = 0;

// get alphabetic program listing
     String line;
     String allProgLines[] = null;
     Vector wossLine = new Vector();

//parse the output of wossname 
     try 
     {
       BufferedReader in;
       while (numProgs == 0) 
       {
         in = new BufferedReader(new StringReader(woss));
         while((line = in.readLine()) != null)
         {
           while((line = in.readLine()) != null) 
           {
             line = line.trim();
             wossLine.add(line);
             if(!line.equals(""))
               numProgs++;
             else
               break;
            }
         }
         in.close();
       }
     }
     catch (IOException e) 
     {
       System.out.println("Cannot read wossname string");
     }

//find unique program names 
     allProgLines = new String[numProgs];
     numProgs = 0;
     Enumeration enumWoss = wossLine.elements();
     while(enumWoss.hasMoreElements())
     {
       line = (String)enumWoss.nextElement();
       if(!line.equals(""))
       {
         line = line.trim();
         boolean news =true;
         String progN = new String(line.substring(0,line.indexOf(" ")+1));
         for(int i=0;i<numProgs;i++)
         {
           if(allProgLines[i].startsWith(progN))
           {
             news = false;
             break;
           }
         }
         if(news)
         {
           allProgLines[numProgs] = new String(line);
           numProgs++;
         }
       }
     }

//sort alphabetically
     java.util.Arrays.sort(allProgLines,0,numProgs);
     allProgs = new String[numProgs];
     allDescription = new String[numProgs];

     for(int i=0;i<numProgs;i++)
     {
       line = allProgLines[i].trim();
       int split = line.indexOf(" ");
       int len   = line.length();
       allProgs[i] = new String(line.substring(0,split));
       line = line.substring(split+1,len).trim();
       allDescription[i] = new String(line);
     }


// get groups 

     HorizontalMenu primaryGroups[] = new HorizontalMenu[numProgs];
     JMenu secondaryGroups[] = new JMenu[numProgs*2];
     mItem = new JMenuItem[numProgs*2];
     String groups;
     String pg;
     String sg="";
     npG=0;
     int nsG=0;
     boolean exist;
     boolean sexist;
     int index;
     nm = 0;
     int start=0;

     try 
     {
       BufferedReader in = new BufferedReader(new StringReader(woss));

       while((groups = in.readLine()) != null)
       {
         groups = groups.trim();
         index = groups.indexOf(":");  
         sexist = false;

         if(index > 0) 
         {
           pg = groups.substring(0,index);
           sg = groups.substring(index+1,groups.length());
           sexist = true;
         }
         else 
           pg = groups;

         exist = false;
         for(int j=0;j<npG;j++)
         {
           if(pg.equalsIgnoreCase(primaryGroups[j].getText()))
           {
             exist = true;
             index = j;
           }
         }

         if(!exist) 
         {
           primaryGroups[npG] = new HorizontalMenu(pg);
           primaryGroups[npG].setBorder(new BevelBorder(BevelBorder.RAISED));
           menuBar.add(primaryGroups[npG]);
           index = npG;
           npG++;
         }

         exist = false;
         if(sexist) 
         {
           for(int j=start;j<nsG;j++) 
           {
             if(sg.equalsIgnoreCase(secondaryGroups[j].getText())) 
             {
               exist = true;
               index = j;
             }
           }
           if(!exist) 
           {
             secondaryGroups[nsG] = new JMenu(sg);
             secondaryGroups[nsG].setFont(menuFont);
             primaryGroups[index].add(secondaryGroups[nsG]);
             index = nsG;
             nsG++;
           }
         }

         while((line = in.readLine()) != null) 
         {
           if(!line.equals("")) 
           {
             int split = line.indexOf(" ");
             int app=0;
             String p = line.substring(0,split);
             for(int i=0;i<numProgs;i++) 
             {
               if(p.equalsIgnoreCase(allProgs[i]))
               {
                 app = i;
                 break;
               }
             }

//add the one line description to the menu 
             mItem[nm] = new JMenuItem(p + "   " + allDescription[app]);
             mItem[nm].setFont(menuFont);
             if(!sexist)
               primaryGroups[index].add(mItem[nm]);
             else
               secondaryGroups[index].add(mItem[nm]);
             nm++;
           } else 
             break;
         }
         start=nsG;
       }

     }
     catch (IOException e) 
     {
       System.out.println("Cannot open EMBOSS acd file ");
     }

   }

/**
*
* Returns the number of programs
* @return number of programs
*
*/
   public int getNumProgs() 
   {
     return numProgs;
   }

/**
*
* Returns the array of all the program names
* @return array of all the program names
*
*/
   public String[] getProgsList() 
   {
     return allProgs;
   }

/**
*
* Returns the array of the program descriptions
* @param array of the program descriptions
*
*/
   public String[] getProgDescription() 
   {
     return allDescription;
   }


/**
*
* Writes to screen the program names
*
*/
   public void writeList() 
   {
     for(int i=0;i<allProgs.length;i++)
     {
        System.out.println(allProgs[i]);
     }
   }

/**
*
* Returns the program menu items 
* @return program menu items
*
*/
   public JMenuItem[] getMenuItems() 
   {
     return mItem;
   }

/**
*
* Returns the number of program menu items
* @return number of program menu items
*
*/
   public int getNumberMenuItems() 
   {
     return nm;
   }

/**
*
* Returns the number of primary menu groups
* @return number of primary menu groups
*
*/
   public int getNumPrimaryGroups() 
   {
     return npG;
   }
  
/**
*
*  HorizontalMenu extends JMenu to produces horizontal 
*  menus.
*
*/
   class HorizontalMenu extends JMenu 
   {
     HorizontalMenu(String label) 
     {
       super(label);
       JPopupMenu pm = getPopupMenu();
       pm.setLayout(new BoxLayout(pm, BoxLayout.Y_AXIS));
       setFont(menuFont);
       setMinimumSize(getPreferredSize());
     }
        
     public void setPopupMenuVisible(boolean b) 
     {
       boolean isVisible = isPopupMenuVisible();
       if (b != isVisible) {
         if ((b==true) && isShowing()) {
           // Set location of popupMenu (pulldown or pullright)
           // Perhaps this should be dictated by L&F
           int x = 0;
           int y = 0;
           Container parent = getParent();
           if(parent instanceof JPopupMenu) 
           {
             x = 0;
             y = getHeight();
           } 
           else 
           {
             x = getWidth();
             y = 0;
           }
           getPopupMenu().show(this, x, y);
         } 
         else 
         {
           getPopupMenu().setVisible(false);
         }
       }
     }
   }

}


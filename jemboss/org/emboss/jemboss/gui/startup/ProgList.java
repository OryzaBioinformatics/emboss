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

public class ProgList 
{

   private String allFiles[];
   private String allDescription[];
   private int numProgs;
   private int nm;
   private int npG;
   private JMenuItem mItem[];
   private HorizontalMenu primaryGroups[];
   private Font menuFont = new Font("SansSerif", Font.BOLD, 11);
// private Font menuFont = new Font(null, Font.BOLD, 10);
   protected static Border raisedBorder = new BevelBorder(BevelBorder.RAISED);

   public ProgList(String woss, String currentDirectory,
                   JMenuBar menuBar)
   {

     Long lastMod;
     numProgs = 0;

// get alphabetic program listing
     String line;
     String allProgLines[] = new String[10];


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
             if(!line.equals(""))
               numProgs++;
             else
               break;
            }
         }
         in.close();
       }

       in = new BufferedReader(new StringReader(woss));
       allProgLines = new String[numProgs];
       numProgs = 0;

       while((line = in.readLine()) != null) 
       {
         while((line = in.readLine()) != null) 
         {
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
           } else 
             break;
         }
       }
       in.close();
     }
     catch (IOException e) 
     {
       System.out.println("Cannot read wossname string");
     }


     java.util.Arrays.sort(allProgLines,0,numProgs);
     allFiles = new String[numProgs];
     allDescription = new String[numProgs];

     for(int i=0;i<numProgs;i++)
     {
       line = allProgLines[i].trim();
       int split = line.indexOf(" ");
       int len   = line.length();
       allFiles[i] = new String(line.substring(0,split));
       line = line.substring(split+1,len).trim();
       allDescription[i] = new String(line);
     }



// get groups 

      primaryGroups = new HorizontalMenu[numProgs*2];
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
       BufferedReader in;
       in = new BufferedReader(new StringReader(woss));

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
           primaryGroups[npG].setBorder(raisedBorder);
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
               if(p.equalsIgnoreCase(allFiles[i]))
               {
                 app = i;
                 break;
               }
             }

//           mItem[nm] = new JMenuItem("<html><b>" + p + " <i>" +
//             "&nbsp;&nbsp;&nbsp;&nbsp;" + allDescription[app]);


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


   public int getNumProgs() 
   {
     return numProgs;
   }

   public String[] getProgsList() 
   {
     return allFiles;
   }

   public String[] getProgDescription() 
   {
     return allDescription;
   }


   public void writeList() 
   {
     int i;

     for(i=0;i<allFiles.length;i++)
     {
        System.out.println("xxxx " + allFiles[i]);
     }
   }

   public JMenuItem[] getMenuItems() 
   {
     return mItem;
   }

   public int getNumberMenuItems() 
   {
     return nm;
   }

   public int getNumPrimaryGroups() 
   {
     return npG;
   }
  
   public JMenu[] getPrimaryGroups()
   {
     return primaryGroups;
   }


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


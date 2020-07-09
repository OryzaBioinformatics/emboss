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


package org.emboss.jemboss.gui.filetree;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.event.*;
import java.io.*;
import java.net.URL;

/**
*
* Display for text, sequence editing, html, gif, png, jpeg.
*
*/
public class FileEditorDisplay
{

   private JTextPane seqText;
   private Document doc;
   private String filename;
   private JPopupMenu popup;
   private String text;
   private byte[] pngContent = null;


   public FileEditorDisplay(final String filename)
   {
     this.filename = filename;
     seqText = new JTextPane();
     doc = seqText.getDocument();
     initStylesForTextPane(seqText);

     popup = new JPopupMenu();
     MouseListener popupListener = new PopupListener();

     JMenuItem menuItem = new JMenuItem("Save",KeyEvent.VK_S);
     seqText.addMouseListener(popupListener);
     menuItem.addActionListener(new ActionListener()
     {
       public void actionPerformed(ActionEvent e)
       {
         new FileSaving(seqText,pngContent);
       }
     });
     popup.add(menuItem);
   }


   public FileEditorDisplay(JFrame ffile, final String filename)
   {

     this(filename);
 
     text = "";
     try
     {
       BufferedReader in = new BufferedReader(new FileReader(filename));
       String line;
       while((line = in.readLine()) != null)
         text = text.concat(line + "\n");
     }
     catch (IOException ioe)
     {
       System.out.println("Cannot read file: " + filename);
     }

     if(filename.endsWith(".html"))
       setText(text,"html",seqText);
     else if( filename.toLowerCase().endsWith(".png") || 
              filename.toLowerCase().endsWith(".gif") ||
              filename.toLowerCase().endsWith(".jpeg") ||
              filename.endsWith(".tif") || filename.endsWith(".ps") )
       setText(text,"png",seqText);
     else
       setText(text,"regular",seqText);

   }


   public FileEditorDisplay(JFrame ffile, final String filename,
                            Object contents)
   {
     this(filename);
    
     if(contents.getClass().equals(String.class))
     {
       try
       {
         doc.insertString(0, (String)contents, null);
       }
       catch (BadLocationException ble)
       {
         System.err.println("Couldn't insert initial text.");
       }
     }
     else if( filename.toLowerCase().endsWith(".png") ||
              filename.toLowerCase().endsWith(".gif") ||
              filename.toLowerCase().endsWith(".jpeg") ||
              filename.endsWith(".tif") || filename.endsWith(".ps") )
     {
       ImageIcon icon = new ImageIcon((byte [])contents);
       seqText.insertIcon(icon);
       pngContent = (byte [])contents;
     }
     else if(filename.endsWith(".html"))
     {
       try
       {
         seqText.setPage((URL)contents);
         seqText.setEditable(false);
       }
       catch (IOException e)
       {
         System.err.println("Attempted to read a bad URL: " + text);
       }
       catch (Exception e)
       {
         System.err.println("Couldn't create help URL: " + text);
       }
     }

   }

/**
*
* Returns instance of JTextPane.
* @return JTextPane created
*
*/
   public JTextPane getJTextPane()
   {
     return seqText; 
   }  



/**
*
*  Set the text & style in the JTextPane.
*  @param String text to be put into the JTextPane.
*  @param String style to use ("regular", "sequence", "png", "html").
*  @param JTextPane instance of JTextPane to set the style for.
*
*/
   public void setText(String text, String type, JTextPane seqText)
   {

     if(type.equalsIgnoreCase("regular"))
     {
       try
       {
         doc.insertString(0, text, null);
       }
       catch (BadLocationException ble)
       {
         System.err.println("Couldn't insert initial text.");
       }

     }
     else if(type.equalsIgnoreCase("sequence"))
     {

       try
       { 
         String hdr = findHeader(text);
         doc.insertString(doc.getLength(), hdr, seqText.getStyle("bold"));
         for (int i=hdr.length(); i < text.length(); i++)
         {
           String rescol = new String("darkGray ");
           if(text.substring(i,i+1).equalsIgnoreCase("A"))
             rescol = "green";
           else if(text.substring(i,i+1).equalsIgnoreCase("T"))
             rescol = "red";
           else if(text.substring(i,i+1).equalsIgnoreCase("G"))
             rescol = "black";
           else if(text.substring(i,i+1).equalsIgnoreCase("C"))
             rescol = "blue";
           doc.insertString(doc.getLength(), text.substring(i,i+1),
                             seqText.getStyle(rescol));
         }
       }
       catch (BadLocationException ble)
       {
         System.err.println("Couldn't insert initial text.");
       }
     } 
     else if(type.equalsIgnoreCase("png"))
     {
       ImageIcon icon = new ImageIcon(filename,filename); 
       seqText.insertIcon(icon);
       pngContent = loadPNGContent();
     }
     else if(type.equalsIgnoreCase("html"))
     {
       try
       {
         URL textURL = new URL("file:"+filename);
         seqText.setPage(textURL);
         seqText.setEditable(false);
       } 
       catch (IOException e) 
       {
         System.err.println("Attempted to read a bad URL: " + text);
       } 
       catch (Exception e) 
       {
         System.err.println("Couldn't create help URL: " + text);
       }
     }

   }

/**
*
*  Need to read png in, in case it is saved out
*
*/
  private byte[] loadPNGContent()
  {

    DataInputStream dis;
    FileInputStream fis;
    int nby = 0;
    byte data[] = new byte[1];
    try
    {
      fis = new FileInputStream(filename);
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
        fis = new FileInputStream(filename);
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

/**
*
*  Returns the content of a png file or null.
*  @return byte content of a png file
*
*/
   public byte[] getPNGContent()
   {
     return pngContent;
   }


/**
*
*  Initialise styles for a JTextPane.
*  @param JTextPane instance of JTextPane to initialise styles for.
*
*/
   protected void initStylesForTextPane(JTextPane textPane) 
   {
     //Initialize some styles.
     Style def = StyleContext.getDefaultStyleContext().
                              getStyle(StyleContext.DEFAULT_STYLE);

     Style regular = textPane.addStyle("regular", def);
     StyleConstants.setFontFamily(def, "SansSerif");

     Style s = textPane.addStyle("italic", regular);
     StyleConstants.setItalic(s, true);

     s = textPane.addStyle("bold", regular);
     StyleConstants.setBold(s, true);

     s = textPane.addStyle("small", regular);
     StyleConstants.setFontSize(s, 10);

     s = textPane.addStyle("large", regular);
     StyleConstants.setFontSize(s, 16);

     s = textPane.addStyle("red", regular);
     StyleConstants.setForeground(s,Color.red);
     StyleConstants.setBold(s, true);
     StyleConstants.setFontFamily(def, "monospaced");

     s = textPane.addStyle("green", regular);
     StyleConstants.setForeground(s,Color.green);
     StyleConstants.setBold(s, true);
     StyleConstants.setFontFamily(def, "monospaced");

     s = textPane.addStyle("blue", regular);
     StyleConstants.setForeground(s,Color.blue);
     StyleConstants.setBold(s, true);
     StyleConstants.setFontFamily(def, "monospaced");

     s = textPane.addStyle("orange", regular);
     StyleConstants.setForeground(s,Color.orange);
     StyleConstants.setBold(s, true);
     StyleConstants.setFontFamily(def, "monospaced");

     s = textPane.addStyle("black", regular);
     StyleConstants.setForeground(s,Color.black);
     StyleConstants.setBold(s, true);
     StyleConstants.setFontFamily(def, "monospaced");

     s = textPane.addStyle("darkGray", regular);
     StyleConstants.setForeground(s,Color.darkGray);
     StyleConstants.setBold(s, true);
     StyleConstants.setFontFamily(def, "monospaced");

     s = textPane.addStyle("icon", regular);
     StyleConstants.setAlignment(s, StyleConstants.ALIGN_CENTER);

   }

/**
*
*  Sequence header text.
*  @param String contents of the file.
*
*/
   private String findHeader(String text)
   {
     String hdr = "";
     String tmphdr = "";
     BufferedReader in = new BufferedReader(new StringReader(text));

     try
     {
       String line = in.readLine();
       if(line.startsWith(">"))                   //fasta
         hdr = line;
       else
       {
         tmphdr = line;
         while((line = in.readLine()) != null)
         {
           tmphdr = tmphdr.concat("\n" + line );
          
           if(line.equals("//") || line.startsWith("SQ ") 
                                || line.endsWith(".."))   //msf, embl, gcg
           {
             hdr = tmphdr;
             break;
           }              
         }
       }
     }
     catch( IOException ioe)
     {
       System.out.println("Cannot read " + text);
     }
 
     return hdr;
   }


  class PopupListener extends MouseAdapter 
  {
    public void mousePressed(MouseEvent e) 
    {
      maybeShowPopup(e);
    }

    public void mouseReleased(MouseEvent e)       
    {      
      maybeShowPopup(e);
    }

    private void maybeShowPopup(MouseEvent e) 
    {
      if (e.isPopupTrigger()) 
      {
        popup.show(e.getComponent(),
                   e.getX(), e.getY());
      }
    }
  }

}


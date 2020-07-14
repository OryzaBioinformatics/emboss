/***************************************************************
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*  @author: Copyright (C) Tim Carver
*
***************************************************************/

package org.emboss.jemboss.editor;

import java.awt.*;
import java.awt.print.Paper;
import java.awt.print.PageFormat;
import java.awt.print.PrinterJob;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.awt.event.*;
import javax.swing.*;
import java.util.*;
import java.io.*;
import javax.swing.border.*;
import javax.imageio.*;
import javax.imageio.stream.*;

import org.emboss.jemboss.gui.ScrollPanel;


/**
*
* Print png/jpeg image and print preview.
* Java 1.4 or higher is required for the imageio package
* which is used here to create jpeg and png images of the
* multiple alignment.
*
*/
public class PrintAlignmentImage extends ScrollPanel
{

  /** page format */
  private PageFormat format = null;   
  /** page number to print    */
  private int pageIndex = 0; 
  /** alignment sequence panel */
  private GraphicSequenceCollection gsc;
  /** prefix of file           */
  private String filePrefix;
  /** status field for print preview */
  private JTextField statusField = new JTextField("");
  /** number of residues per line    */
  private int nResPerLine = 0;

  /**
  *
  * @param gsc	sequence panel
  * @param 	page format
  *
  */
  public PrintAlignmentImage(GraphicSequenceCollection gsc,
                             PageFormat format)
  {
    this(gsc);
    this.format = format;
  }

  /**
  *
  * @param gsc  sequence panel
  * 
  */
  public PrintAlignmentImage(GraphicSequenceCollection gsc)
  {
    super();
    this.gsc = gsc;
    setBackground(Color.white);
  }
 
  /**
  *
  * Set the page format
  * @param format 	to use for the image
  *
  */
  protected void setFormat(PageFormat format)
  {
    this.format = format; 
  }

  /**
  *
  * Get the page format
  * @return format       to use for the image
  *
  */
  protected PageFormat getFormat()
  {
    return format;
  }

  /**
  *
  * Set the page number to create an image of
  * @param pageIndex 	page number
  *
  */
  public void setPageIndex(int pageIndex)
  {
    this.pageIndex = pageIndex;
  }

  /**
  *
  * Override this method to draw the sequences
  * @return Graphics g
  *
  */
  public void paintComponent(Graphics g)
  {
// let UI delegate paint first (incl. background filling)
    super.paintComponent(g);
    Graphics2D g2d = (Graphics2D) g.create();
    if(nResPerLine == 0)
      gsc.drawSequences(g2d,format,pageIndex); 
    else
      gsc.drawSequences(g2d,format,pageIndex,nResPerLine);
  } 


  /**
  *
  * Print to a jpeg or png file
  *
  */
  public void print()
  {
    if(format == null)
      getFormatDialog();

    try
    {
      String type = showOptions(true);
      int npages = gsc.getNumberPages(format,nResPerLine);
      for(int i=0;i<npages;i++)
      {
        RenderedImage rendImage = createAlignmentImage(i);
        writeImageToFile(rendImage,
                      new File(filePrefix+i+"."+type),type);
      }
    }
    catch(NoClassDefFoundError ex)
    {
      JOptionPane.showMessageDialog(this,
            "This option requires Java 1.4 or higher.");
    }
  }


  /**
  *
  * Print to a jpeg or png file
  *
  */
  public void print(int nResPerLine, String type,
                    String filePrefix, boolean landscape,
                    double leftMargin, double rightMargin,
                    double topMargin, double btmMargin)
  {
    this.nResPerLine = nResPerLine;
    PrinterJob printerJob = PrinterJob.getPrinterJob();
    format = new PageFormat();
    if(landscape)
      format.setOrientation(PageFormat.LANDSCAPE);
    else
      format.setOrientation(PageFormat.PORTRAIT);

    if(leftMargin > 0.d)
    {
      Paper paper  = format.getPaper();
      double width = paper.getWidth()-(72*(leftMargin+rightMargin)); 
      double hgt   = paper.getHeight()-(72*(topMargin+btmMargin));
      paper.setImageableArea(leftMargin*72,topMargin*72,
                             width,hgt);
      format.setPaper(paper);
    }

    if(nResPerLine <= 0)
      this.nResPerLine = gsc.getResiduesPerLine(format);

    try
    {
      int npages = gsc.getNumberPages(format,this.nResPerLine);
      for(int i=0;i<npages;i++)
      {
        RenderedImage rendImage = createAlignmentImage(i);
        writeImageToFile(rendImage,
                       new File(filePrefix+i+"."+type),type);
      }
    }
    catch(NoClassDefFoundError ex)
    {
      JOptionPane.showMessageDialog(this,
            "This option requires Java 1.4 or higher.");
    }
  }


  /**
  *
  * Print to one jpeg or png file
  *
  */
  public void print(String type,
                    double leftMargin, double rightMargin,
                    double topMargin, double btmMargin)
  {
    this.print(nResPerLine,type,filePrefix,
               leftMargin,rightMargin,topMargin,btmMargin);
  }


  /**
  *
  * Print to one jpeg or png file
  *
  */
  public void print(int nResPerLine, String type,
                    String filePrefix,
                    double leftMargin, double rightMargin,
                    double topMargin, double btmMargin)
  {
    this.nResPerLine = nResPerLine;
    PrinterJob printerJob = PrinterJob.getPrinterJob();
    format = new PageFormat();

    Dimension d = gsc.getImageableSize(nResPerLine);
    double imageWidth  = d.getWidth();
    double imageHeight = d.getHeight();
    Paper paper  = format.getPaper();

    if(leftMargin > 0.d)
    {
      leftMargin  = leftMargin*72;
      topMargin   = topMargin*72;
      rightMargin = rightMargin*72;
      btmMargin   = btmMargin*72;
      paper.setSize(imageWidth+(leftMargin+rightMargin),
                    imageHeight+(topMargin+btmMargin));
    }
    else
    {
      paper.setSize(imageWidth,imageHeight);
      leftMargin = 0;
      topMargin  = 0;
    }
    paper.setImageableArea(leftMargin,topMargin,
                           imageWidth,imageHeight);
    format.setPaper(paper);

    try
    {
      RenderedImage rendImage = createAlignmentImage(0);
      writeImageToFile(rendImage,
                       new File(filePrefix+"."+type),type);
    }
    catch(NoClassDefFoundError ex)
    {
      JOptionPane.showMessageDialog(this,
            "This option requires Java 1.4 or higher.");
    }
  }


  /**
  *
  * Provide some options for the image created
  * @param showFileOptions	display file options
  *	
  */
  protected String showOptions(boolean showFileOptions)
  {
    JPanel joptions = new JPanel();
    Box YBox = Box.createVerticalBox();
    joptions.add(YBox);

// file name prefix
    JComboBox formatSelect = null;
    JTextField fileField   = null;
    Box XBox;

    if(showFileOptions)
    {
      JLabel jlab = new JLabel("File prefix: ");
      String def = System.getProperty("user.dir")+
                   System.getProperty("file.separator");
                   
      try
      {
        def = def.concat(gsc.getName());
      }
      catch(Exception e){}
      fileField = new JTextField(def);
      XBox = Box.createHorizontalBox();
      XBox.add(jlab);
      XBox.add(fileField);
      XBox.add(Box.createHorizontalGlue());
      YBox.add(XBox);

// format
      formatSelect = 
         new JComboBox(javax.imageio.ImageIO.getWriterFormatNames());
      YBox.add(formatSelect);
    }

// no. of residues per line
    XBox = Box.createHorizontalBox();
    if(format == null)
      format = new PageFormat();
    String mres = Integer.toString(gsc.getResiduesPerLine(format));
    JLabel jres = new JLabel("Residues per line: [max:"+mres+"]");
    if(nResPerLine != 0)
      mres = Integer.toString(nResPerLine);

    JTextField maxResiduesField = new JTextField(mres);
    XBox.add(jres);
    XBox.add(maxResiduesField);
    XBox.add(Box.createHorizontalGlue());
    YBox.add(XBox);

    JOptionPane.showMessageDialog(null,joptions,"Options",
                               JOptionPane.PLAIN_MESSAGE);
  
    nResPerLine = Integer.parseInt(maxResiduesField.getText());
    if(showFileOptions)
    {  
      filePrefix = fileField.getText().trim();
      return (String)formatSelect.getSelectedItem();
    }

    return null;
  }


  /**
  *
  * Get a default page format
  * @return	page format
  *
  */
  protected PageFormat getFormatDialog()
  {
    PrinterJob printerJob = PrinterJob.getPrinterJob();
    format = new PageFormat();
    format = printerJob.pageDialog(format);
    return format;
  }
 

  /**
  *
  *  Returns a generated image 
  *  @param pageIndex	page number
  *  @return 		image
  *
  */
  private RenderedImage createAlignmentImage(int pageIndex)
  {
    int width  = (int)format.getWidth();
    int height = (int)format.getHeight();
    // Create a buffered image in which to draw
    BufferedImage bufferedImage = new BufferedImage(
                                  width,height, 
                                  BufferedImage.TYPE_INT_RGB);
    // Create a graphics contents on the buffered image
    Graphics2D g2d = bufferedImage.createGraphics();
    g2d.setColor(Color.white);
    g2d.fillRect(0,0,width,height);
    // Draw graphics
    if(nResPerLine == 0)
      gsc.drawSequences(g2d,format,pageIndex);
    else
      gsc.drawSequences(g2d,format,pageIndex,nResPerLine);
 
    return bufferedImage;
  }

  /**
  *
  * Display a single page print preview page
  *
  */
  protected void printSinglePagePreview()
  {
    Border loweredbevel = BorderFactory.createLoweredBevelBorder();
    Border raisedbevel = BorderFactory.createRaisedBevelBorder();
    Border compound = BorderFactory.createCompoundBorder(raisedbevel,loweredbevel);
    statusField.setBorder(compound);
    statusField.setEditable(false);
                                 
    format = new PageFormat();
    Dimension d = gsc.getImageableSize(nResPerLine);
    double imageWidth  = d.getWidth();
    double imageHeight = d.getHeight();
    Paper paper  = format.getPaper();
                                                                                                                               
//  if(leftMargin > 0.d)
//  {
//    leftMargin  = leftMargin*72;
//    topMargin   = topMargin*72;
//    rightMargin = rightMargin*72;
//    btmMargin   = btmMargin*72;
//    paper.setSize(imageWidth+(leftMargin+rightMargin),
//                  imageHeight+(topMargin+btmMargin));
//  }
//  else
//  {
//    paper.setSize(imageWidth,imageHeight);
//    leftMargin = 0;
//    topMargin  = 0;
//  }
 
    paper.setSize(imageWidth,imageHeight);
    paper.setImageableArea(0,0,imageWidth,imageHeight);
    format.setPaper(paper);
                                                                                              
//  showOptions(false);
    statusField.setText(pageIndex+"1 of 1 page(s)");
    final JFrame f = new JFrame("Print Preview");
    JPanel jpane = (JPanel)f.getContentPane();
    JScrollPane scrollPane = new JScrollPane(this);
    jpane.setLayout(new BorderLayout());
    jpane.add(scrollPane,BorderLayout.CENTER);
    jpane.add(statusField,BorderLayout.SOUTH);
 
    final Dimension dScreen = f.getToolkit().getScreenSize();
    d = new Dimension((int)format.getWidth(),(int)format.getHeight());
    setPreferredSize(d);
    d = new Dimension((int)(dScreen.getWidth()/2),
                      (int)((dScreen.getHeight()*3)/4));
    f.setSize(d);

// menus
    JMenuBar menuBar = new JMenuBar();
    JMenu filemenu = new JMenu("File");
    menuBar.add(filemenu);

// print png/jpeg
    JMenuItem printImage = new JMenuItem("Print Image File (png/jpeg)...");
    printImage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        print();
      }
    });
    filemenu.add(printImage);
                                                                                                                               
// close
    filemenu.add(new JSeparator());
    JMenuItem menuClose = new JMenuItem("Close");
    menuClose.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));
                                                                                                                               
    filemenu.add(menuClose);
    menuClose.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        f.dispose();
      }
    });
    f.setJMenuBar(menuBar);
    f.setVisible(true);
  }

  /**
  *
  * Display a print preview page
  *
  */
  protected void printPreview()
  {
    Border loweredbevel = BorderFactory.createLoweredBevelBorder();
    Border raisedbevel = BorderFactory.createRaisedBevelBorder();
    Border compound = BorderFactory.createCompoundBorder(raisedbevel,loweredbevel);
    statusField.setBorder(compound);
    statusField.setEditable(false);

    if(format == null)
      format = getFormatDialog();

    showOptions(false);
    final int npages = gsc.getNumberPages(format,nResPerLine);
    statusField.setText(pageIndex+"1 of "+npages+" page(s)");

    final JFrame f = new JFrame("Print Preview");
    JPanel jpane = (JPanel)f.getContentPane();
    JScrollPane scrollPane = new JScrollPane(this);
    jpane.setLayout(new BorderLayout());
    jpane.add(scrollPane,BorderLayout.CENTER);
    jpane.add(statusField,BorderLayout.SOUTH);

    final Dimension dScreen = f.getToolkit().getScreenSize();
    Dimension d = new Dimension((int)format.getWidth(),(int)format.getHeight());
    setPreferredSize(d);
    d = new Dimension((int)(dScreen.getWidth()/2),
                      (int)((dScreen.getHeight()*3)/4));
    f.setSize(d);

    JMenuBar menuBar = new JMenuBar();

    JMenu filemenu = new JMenu("File");
    menuBar.add(filemenu);

// print postscript
    JMenu printMenu = new JMenu("Print");
    filemenu.add(printMenu);

    JMenuItem print = new JMenuItem("Print Postscript...");
    print.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        new PrintAlignment(gsc);
      }
    });
    printMenu.add(print);

// print png/jpeg
    JMenuItem printImage = new JMenuItem("Print Image Files (png/jpeg)...");
    printImage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        print();
      }
    });
    printMenu.add(printImage);

// close
    filemenu.add(new JSeparator());
    JMenuItem menuClose = new JMenuItem("Close");
    menuClose.setAccelerator(KeyStroke.getKeyStroke(
              KeyEvent.VK_E, ActionEvent.CTRL_MASK));

    filemenu.add(menuClose);
    menuClose.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        f.dispose();
      }
    });

// page selection buttons
    final JButton nextPage = new JButton(">");
    final JButton endPage = new JButton(">>");
    final JButton previousPage = new JButton("<");
    final JButton firstPage = new JButton("<<");

    menuBar.add(firstPage);
    firstPage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        pageIndex = 0;
        repaint();
        previousPage.setEnabled(false);
        firstPage.setEnabled(false);
        nextPage.setEnabled(true);
        endPage.setEnabled(true);
        statusField.setText(pageIndex+1+" of "+npages+" pages");
      }
    });

    menuBar.add(previousPage);
    previousPage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        pageIndex--;
        repaint();
        if(pageIndex == 0)
        {
          previousPage.setEnabled(false);
          firstPage.setEnabled(false);
        }
        nextPage.setEnabled(true);
        endPage.setEnabled(true);
        statusField.setText(pageIndex+1+" of "+npages+" pages");
      }
    });
    previousPage.setEnabled(false);
    firstPage.setEnabled(false);


    menuBar.add(nextPage);
    nextPage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        pageIndex++;
        repaint();
        if(pageIndex == npages-1)
        {
          nextPage.setEnabled(false);
          endPage.setEnabled(false);
        }
        previousPage.setEnabled(true);
        firstPage.setEnabled(true);
        statusField.setText(pageIndex+1+" of "+npages+" pages");
      }
    });

    menuBar.add(endPage);
    endPage.addActionListener(new ActionListener()
    {
      public void actionPerformed(ActionEvent e)
      {
        pageIndex = npages-1;
        repaint();
        nextPage.setEnabled(false);
        endPage.setEnabled(false);
        previousPage.setEnabled(true);
        firstPage.setEnabled(true);
        statusField.setText(pageIndex+1+" of "+npages+" pages");
      }
    });
    if(pageIndex == npages-1)
    {
      nextPage.setEnabled(false);
      endPage.setEnabled(false);
    }


    f.setJMenuBar(menuBar);
    f.setVisible(true);
  }

  /**
  *
  * Write out the image 
  * @param image	image
  * @param file		file to write image to
  * @param type		type of image 
  *
  */
  private void writeImageToFile(RenderedImage image, 
                               File file, String type)
  {
    try
    {
      javax.imageio.ImageIO.write(image,type,file);
    }catch ( IOException e )
    {
      System.out.println("Java 1.4+ is required");
      e.printStackTrace();
    }
  }

}


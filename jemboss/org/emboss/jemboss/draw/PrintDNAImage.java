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

package org.emboss.jemboss.draw;

import java.awt.print.PageFormat;
import java.awt.print.PrinterJob;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.awt.image.RenderedImage;
import java.io.*;
import javax.swing.border.*;

import org.emboss.jemboss.gui.ScrollPanel;

/**
*
* Print png/jpeg image and print preview.
* Java 1.4 or higher is required for the imageio package
* which is used here to create jpeg and png images of the
* DNA diagram.
*
*/
public class PrintDNAImage extends ScrollPanel
{

  /** page format */
  private PageFormat format = null;
  /** page number to print    */
  //private int pageIndex = 0;
  /** alignment sequence panel */
  private DNADraw dna;
  /** prefix of file           */
//private String filePrefix;
  /** status field for print preview */
  private JTextField statusField = new JTextField("");
  /** number of residues per line    */
  //private int nResPerLine = 0;
  /** line attributes */
  //private Hashtable lineAttr;
  /** type (jpeg/png) */
  private String type;

  /**
  *
  * @param dna   dna panel
  *
  */
  public PrintDNAImage(DNADraw dna)
  {
    super();
    this.dna = dna;

    //lineAttr = dna.getLineAttributes();
    setBackground(Color.white);
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
    dna.drawAll(g2d,true);
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
      File fsave = showOptions();

      if(fsave == null)
        return;

      RenderedImage rendImage = createDNAImage(0);
      writeImageToFile(rendImage,fsave,type);
    }
    catch(NoClassDefFoundError ex)
    {
      JOptionPane.showMessageDialog(this,
            "This option requires Java 1.4 or higher.");
    }
  }


  /**
  *
  * Get a default page format
  * @return     page format
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
  *  @param pageIndex   page number
  *  @return            image
  *
  */
  private RenderedImage createDNAImage(int pageIndex)
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
    dna.drawAll(g2d,true);

    return bufferedImage;
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

    statusField.setText("DNA map");
    final JFrame f = new JFrame("Print Preview");
    JPanel jpane = (JPanel)f.getContentPane();
    JScrollPane scrollPane = new JScrollPane(this);
    jpane.setLayout(new BorderLayout());
    jpane.add(scrollPane,BorderLayout.CENTER);
    jpane.add(statusField,BorderLayout.SOUTH);

    Dimension d = new Dimension((int)format.getWidth(),
                                (int)format.getHeight());
    setPreferredSize(d);
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
        dna.doPrintActions();
      }
    });
    printMenu.add(print);

// print png/jpeg
    JMenuItem printImage = new JMenuItem("Print png/jpeg Image...");
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

    f.setJMenuBar(menuBar);
    f.setVisible(true);
  }


  /**
  *
  * Provide some options for the image created
  *
  */
  private File showOptions()
  {
    String cwd = System.getProperty("user.dir");
    JFileChooser fc = new JFileChooser(cwd);
    File fselect = new File(cwd+System.getProperty("file.separator")+
                            "dna_image.jpeg"); 
    fc.setSelectedFile(fselect);

// file name prefix
    Box bdown = Box.createVerticalBox();
    bdown.add(Box.createVerticalGlue());

    JLabel labFormat = new JLabel("Select Format:");
    Font font = labFormat.getFont();
    labFormat.setFont(font.deriveFont(Font.BOLD));

    bdown.add(labFormat);

    Box bacross = Box.createHorizontalBox();
    JComboBox formatSelect = 
       new JComboBox(javax.imageio.ImageIO.getWriterFormatNames());
    Dimension d = formatSelect.getPreferredSize();
    formatSelect.setMaximumSize(d);
    bacross.add(Box.createHorizontalGlue());
    bacross.add(formatSelect);
    bdown.add(bacross);

// file prefix & format options
    fc.setAccessory(bdown);
    int n = fc.showSaveDialog(null);
    if(n == JFileChooser.CANCEL_OPTION)
      return null;

    type = (String)formatSelect.getSelectedItem();
    return fc.getSelectedFile();
  }


  /**
  *
  * Write out the image
  * @param image        image
  * @param file         file to write image to
  * @param type         type of image
  *
  */
  private void writeImageToFile(RenderedImage image,
                               File file, String type)
  {
    try
    {
      javax.imageio.ImageIO.write(image,type,file);
    }
    catch (IOException e)
    {
      System.out.println("Java 1.4+ is required");
    }
  }


}



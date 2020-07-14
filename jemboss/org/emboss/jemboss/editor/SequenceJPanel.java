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
import java.awt.event.*;
import java.awt.print.*;
import javax.swing.*;
import java.util.Hashtable;
import java.util.Vector;

/**
*
* Applet for drawing sequence 
*
*/
public class SequenceJPanel extends JPanel
                            implements ActionListener
{
  private Color col;
  private Sequence seq;
  private int fontSize = 14;
  private Font font = new Font("Monospaced", 
                      Font.PLAIN, fontSize);
  private int boundWidth;
  private int boundWidth2;
  private int resWidth;
  private int seqHeight; 
  private int pressedResidue;
  private int interval;
  private int seqLength;
  private int ypad=0;

  private Hashtable colorTable;
  private String padChar = new String("-");
  private String pattern;
  private boolean drawSequence = false;
  private boolean drawBlackBox = false;
  private boolean drawColorBox = false;
  private boolean drawNumber   = false;
  private boolean prettyPlot   = false;
  private boolean highlightPattern = false;
  private JComponent viewPane;
  private JPopupMenu popup;


  public SequenceJPanel(Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int fontSize, int ypad)
  {
    this.drawSequence = drawSequence;
    this.drawBlackBox = drawBlackBox;
    this.drawColorBox = drawColorBox;
    this.viewPane = viewPane;
    this.seq = seq;
    this.ypad = ypad;

    if(colorTable != null)
      this.colorTable = colorTable;
    else
      setDefaultColorHashtable();

    setBackground(Color.white);
    if(fontSize != 0)
    {
      this.fontSize = fontSize;
      font = new Font("Monospaced",
                      Font.PLAIN, fontSize);
    }

    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    boundWidth2 = boundWidth/2;

    resWidth  = metrics.stringWidth("A")+boundWidth;
    if(seq != null)
      seqLength = seq.getLength();
    
    seqHeight = resWidth;
    init();

    // Popup menu
    addMouseListener(new PopupListener());
    popup = new JPopupMenu();
    JLabel labName = null;

    if(seq.getID() != null)
      labName = new JLabel(" "+seq.getID());
    else 
      labName = new JLabel(" "+seq.getName());

    popup.add(labName);
    popup.add(new JSeparator());
    JMenuItem menuItem = new JMenuItem("Delete ");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    popup.add(new JSeparator());
    menuItem = new JMenuItem("Reverse Complement ");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("Reverse ");
    menuItem.addActionListener(this);
    popup.add(menuItem);
    menuItem = new JMenuItem("Complement ");
    menuItem.addActionListener(this);
    popup.add(menuItem);

    //set size
    setPreferredSize(getPreferredSize());
//  setMaximumSize(getPreferredSize());
//  setMinimumSize(getPreferredSize());
  }

/**
*
* Constructor with default font size.
*
*/
  public SequenceJPanel(Sequence seq, JComponent viewPane,
                        boolean drawSequence,
                        boolean drawBlackBox, boolean drawColorBox,
                        Hashtable colorTable, int ypad)
  {
    this(seq,viewPane,drawSequence,drawBlackBox,drawColorBox,
         colorTable,0,ypad);
  }

  public SequenceJPanel(int interval, int seqLength)
  {
    seq = null;
    this.drawNumber = true;
    this.interval   = interval;
    this.seqLength  = seqLength+1;
    setBackground(Color.white);
    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    boundWidth2 = boundWidth/2;
    resWidth  = metrics.stringWidth("A")+boundWidth;
    seqHeight = resWidth;

    setPreferredSize(getPreferredSize());
//  setMaximumSize(getPreferredSize());
  }


 
  public void init()
  {
    if(!drawNumber)
    {
      addMouseListener(new MouseAdapter()
      {
        public void mousePressed(MouseEvent e)
        {
          Point loc = e.getPoint();
          pressedResidue = (int)(loc.x/resWidth);
        }
      });

      addMouseMotionListener(new MouseMotionAdapter()
      {
        public void mouseDragged(MouseEvent e)
        {
          Point loc = e.getPoint();
          int resPos = (int)(loc.x/resWidth);
          String seqS = seq.getSequence();

          if(resPos == pressedResidue+1 && resPos > 0)
          {
            seq.insertResidue(padChar,pressedResidue);
            ((GraphicSequenceCollection)viewPane).setMaxSequenceLength(seq.getLength());
            pressedResidue = pressedResidue+1;
            paintComponent(getGraphics());
            viewPaneResize();
          }
          else if(resPos == pressedResidue-1 && resPos > -1)
          {
            if(seqS.substring(resPos,resPos+1).equals(padChar))
            {
              seq.deleteResidue(resPos);
              pressedResidue = pressedResidue-1;
              paintComponent(getGraphics());
            }
          }
        }
      });
    }

  }

  public void paintComponent(Graphics g)
  {
// let UI delegate paint first (incl. background filling)
    super.paintComponent(g);
    g.setFont(font);
    FontMetrics metrics = g.getFontMetrics();
    boundWidth = metrics.stringWidth("A");
    boundWidth2 = boundWidth/2;
    resWidth = metrics.stringWidth("A")+boundWidth;
    seqHeight = resWidth;
    String sName = null;
    if(drawSequence)
      sName = seq.getName();
    String seqS = null;
    if(!drawNumber)
    {
      seqS = seq.getSequence();
      seqLength = seqS.length();
    }

    int istart = 0;
    int istop  = seqLength;
    try
    {
      Rectangle viewRect = ((GraphicSequenceCollection)viewPane).getViewRect();
      istart = viewRect.x/resWidth;
      istop  = istart + viewRect.width/resWidth + 2;
      if(istop > seqLength)
        istop = seqLength;
    }
    catch( NullPointerException npe) {}

// highlight patterns by making bold
    Vector pvec = null;
    Font fontBold = null;
//  System.out.println("istart "+istart+" istop "+istop);
    if(drawSequence && highlightPattern)
    {
//    pvec = getPatternPositions(istart,istop,seqS);
      int patlen = pattern.length();
      int pstart = istart-patlen;
      if(pstart < 0)
        pstart = 0;
      int pstop = istop+patlen;
      if(pstop > seqLength)
        pstop = seqLength;
      pvec = getPatternPositions(pstart,seqS.substring(pstart,pstop));
      fontBold = new Font(font.getName(),Font.BOLD,
                          font.getSize()+3);
    }

// draw 
    for(int i=istart;i<istop;i++)
    {
      int ipos = i*resWidth;
      if(drawColorBox)
      {
        g.setColor(getColor(seqS.substring(i,i+1)));
        g.fillRect(ipos,0,resWidth,seqHeight);
      }

      g.setColor(Color.black);
      if(drawSequence)
      {
        if(highlightPattern)
        {
          if(pvec.contains(new Integer(i)))
            g.setFont(fontBold);
        }

        String res = seqS.substring(i,i+1);
        if(prettyPlot)
          g.setColor(((GraphicSequenceCollection)viewPane)
                                   .getColor(res,i,sName));
        g.drawString(res, 
                     ipos+boundWidth2,
                     seqHeight-boundWidth2);
        if(highlightPattern)
          g.setFont(font);
      }
      else if(drawNumber && (int)((i+1)%interval) == 0)
      {
        String snum = Integer.toString(i+1);
        int numWidth = metrics.stringWidth(snum);
        g.drawString(snum,
                     ((int)((i+0.5)*resWidth)-(numWidth/2)),
                     seqHeight-boundWidth2);
      }
      if(drawBlackBox)
      {
        g.setColor(Color.black);
        g.drawRect(ipos,0,resWidth,seqHeight);
      }
    }
  }

/**
*
* Find all occurences of the pattern in the sequence between
* the start and stop positions. Returning all positions of these
* in a vector.
*
*/
  private Vector getPatternPositions(int istart, int istop,
                                     String seqS)
  {
    Vector pvec = new Vector();
    int patlen = pattern.length();
    int pstart = istart-patlen;
    int pstop  = istop+patlen;
    if(pstart < 0)
      pstart = 0;
    if(pstop > seqLength)
      pstop = seqLength;

    int ipat;
    while( (ipat = seqS.substring(pstart,pstop).indexOf(pattern)) > -1)
    {
      for(int i=0;i<patlen;i++)
        pvec.add(new Integer(ipat+pstart+i));
      pstart = ipat+pstart+1;
    }
//  System.out.println("Showing "+pattern+" :: "+ipat+pstart+" :: "+
//                     "istart "+istart+" istop "+istop);
    return pvec;
  }

/**
*
* Find all occurences of the pattern in the sequence between
* the start and stop positions. Returning all positions of these
* in a vector.
*
*/
  private Vector getPatternPositions(int subseqStart,
                                     String subseq)
  {
    subseq = subseq.toLowerCase();
    Vector pvec = new Vector();
    int patlen = pattern.length();
    int pstart = 0;
    int ipat;
    while( (ipat = subseq.substring(pstart).indexOf(pattern)) > -1)
    {
      for(int i=0;i<patlen;i++)
        pvec.add(new Integer(ipat+pstart+subseqStart+i));
      pstart = ipat+pstart+1;
    }
//  System.out.println("Showing "+pattern+" :: "+ipat+pstart+" :: "+
//                     "istart "+istart+" istop "+istop);
    return pvec;
  }

  protected void showPattern(String pattern)
  {
    highlightPattern = true;
    this.pattern = pattern.toLowerCase();
  }
  
  public void setPrettyPlot(boolean prettyPlot)
  {
    this.prettyPlot = prettyPlot;
  }

  public String getToolTipText(MouseEvent e)
  {
    Point loc = e.getPoint();
    int resPos = (int)(loc.x/resWidth);
    if(resPos < 0)
      return null;

    if(resPos > seq.getSequence().length()-1)
      return null;

    String res = seq.getSequence().substring(resPos,resPos+1);
    String ls = System.getProperty("line.separator");

    if(seq.getID() != null)
      return seq.getID()+"\nResidue: "+res+ls+
           "Position: "+Integer.toString(resPos+1);

    return seq.getName()+"\nResidue: "+res+ls+
           "Position: "+Integer.toString(resPos+1);
  }

  public Point getToolTipLocation(MouseEvent e)
  {
    Point loc = e.getPoint();
    int x = (int)((loc.x+resWidth)/resWidth)*resWidth;
    int y = seqHeight-(resWidth/2);
    return new Point(x,y);
  }

  private Color getColor(String s)
  {
    s = s.toUpperCase();
    if(colorTable.containsKey(s))
      return (Color)colorTable.get(s);
       
    return getBackground();
  }

  public void setDefaultColorHashtable()
  {
    colorTable = new Hashtable();
    colorTable.put("A",Color.green);
    colorTable.put("T",Color.red);
    colorTable.put("C",Color.blue);
    colorTable.put("G",Color.white);
  }


  public void setDrawBoxes(boolean drawBlackBox)
  {
    if(drawNumber)
      return;
    this.drawBlackBox = drawBlackBox;
    paintComponent(getGraphics());
  }

  public void setDrawColor(boolean drawColorBox)
  {
    if(drawNumber)
      return;
    this.drawColorBox = drawColorBox;
    paintComponent(getGraphics());
  }

  public void setFontSize(int size)
  {
    fontSize = size;
    font = new Font("Monospaced",
                      Font.PLAIN, fontSize);
    FontMetrics metrics = getFontMetrics(font);
    boundWidth = metrics.stringWidth("A");
    resWidth  = metrics.stringWidth("A")+boundWidth;
    seqHeight = resWidth;

    setPreferredSize(getPreferredSize());
//  setMaximumSize(getPreferredSize());
//  setMinimumSize(getPreferredSize());

    paintComponent(getGraphics());
  }
 
  public int getResidueWidth()
  {
    return resWidth;
  }

  public void setColorScheme(Hashtable colorHash)
  {
    this.colorTable = colorHash;
//  paintComponent(getGraphics());
  }

  public int getFontSize()
  {
    return fontSize;
  }

  public Dimension getPreferredSize()
  {
    return new Dimension(getSequenceWidth(),getSequenceHeight());
  }

  protected void viewPaneResize()
  {
    Dimension dpane = viewPane.getPreferredSize();
    GraphicSequenceCollection gsc = (GraphicSequenceCollection)viewPane;
    
    int xpane = (int)dpane.getWidth();
    int xsize = gsc.getPanelWidth();
    if(xsize > xpane)
      viewPane.setPreferredSize(new Dimension(xsize,
                              gsc.getPanelHeight()));
    gsc.setJScrollPaneViewportView();
  }


  public int getSequenceHeight()
  {
    return seqHeight+ypad;
  }

  public int getSequenceWidth()
  {
    return resWidth*seqLength;
  }

  public int getSequenceResidueWidth()
  {
    return resWidth;
  }

  public void setSequenceLength(int s)
  {
    seqLength = s;
  }

  public String getDescription()
  {
    return getName();
  }

  public void getNamePrintGraphic(Graphics g2d)
  {
    if(seq == null)
      return;

    String name = seq.getName();
    if(seq.getID() != null)
      name = seq.getID();

    g2d.setColor(Color.black);
    g2d.drawString(name,0,seqHeight-boundWidth2);
  }
 
  public void getSequencePrintGraphic(Graphics g2d, int MAXSEQNAME, 
                                      int istart, int istop) 
  {
    String sName = null;
    String seqS  = null; 

    if(drawSequence)
    {
      sName = seq.getName();
      if(seq.getID() != null)
        sName = seq.getID();

      seqS  = seq.getSequence();
      int seqLength = seq.getLength();
      if(seqLength < istop)
        istop = seqLength;
    }

    FontMetrics metrics = getFontMetrics(font);
    for(int i=istart;i<istop;i++)
    {
      if(drawColorBox)
      {
        g2d.setColor(getColor(seqS.substring(i,i+1)));
        g2d.fillRect((i-istart)*(resWidth)+MAXSEQNAME,0,resWidth,seqHeight);
      }
      g2d.setColor(Color.black);
      if(drawBlackBox)
        g2d.drawRect((i-istart)*(resWidth)+MAXSEQNAME,0,resWidth,seqHeight);


      if(drawSequence)
      {
        String res = seqS.substring(i,i+1);
        if(prettyPlot)
          g2d.setColor(((GraphicSequenceCollection)viewPane)
                                   .getColor(res,i,sName));
        g2d.drawString(res,
                      ((i-istart)*resWidth)+boundWidth2+MAXSEQNAME,
                      seqHeight-boundWidth2);
      
      }
      else if(drawNumber && (int)((i+1-istart)%interval) == 0)
      {
        String snum = Integer.toString(i+1);
        int numWidth = metrics.stringWidth(snum);
        g2d.drawString(snum,
                     ((int)((i-istart+0.5)*resWidth)-(numWidth/2))+MAXSEQNAME,
                     seqHeight-boundWidth2);
      }
    }
  }

  public void actionPerformed(ActionEvent e)
  {
    JMenuItem source = (JMenuItem)(e.getSource());
    if(source.getText().startsWith("Delete "))
    {
      ((GraphicSequenceCollection)viewPane).deleteSequence(seq.getName());
    }
    else if(source.getText().startsWith("Reverse Complement"))
    {
      seq.reverseComplementSequence();
      paintComponent(getGraphics());
    }
    else if(source.getText().startsWith("Reverse "))
    {
      seq.reverseSequence();
      paintComponent(getGraphics());
    }
    else if(source.getText().startsWith("Complement "))
    {
      seq.complementSequence();
      paintComponent(getGraphics());
    }
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
      if(e.isPopupTrigger())
        popup.show(e.getComponent(),
                e.getX(), e.getY());
    }
  }


}


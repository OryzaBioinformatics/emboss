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

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.print.*;
import java.util.*;
import java.io.File;

import org.emboss.jemboss.gui.form.MultiLineToolTipUI;

/**
*  
* This class can be used to get a grapical representation
* of a collection of sequences.
*
*/
public class GraphicSequenceCollection extends JPanel
                                       implements Printable, Scrollable
{

  private Vector removedSeqs = 
                     new Vector(); // Vector of seqs removed from panel
  private Vector seqs;             // Vector containing Sequence objects
  private Vector graphicSequence;  // Vector containing graphical seqs
  private Vector graphicName;      // Vector containing graphical names of seqs
  private Hashtable colorScheme;   // Colour scheme to use
  private PlotConsensus pc = null; // Consensus plot
  private int lenName;
  private int hgtName;
  private int hgt;
  private int len; 
  private int MAXSEQLENGTH = 0;
  private int numResiduePerLine = 0;   // no. of res on each line for print
  private SequenceJPanel numberDraw;
  private JScrollPane jspSequence; // Seq scrollpane
  private JPanel seqNamePanel;
  private Box seqBox;
  private Box seqNameBox;
  private Box plotconsSeqBox = null;

  private boolean drawSequence;
  private boolean drawBlackBox;
  private boolean drawColorBox;
  private boolean drawNumber;
  private boolean prettPlot = false;
  private int plotConStrut = 20;

  public GraphicSequenceCollection(Vector seqs, Hashtable colorScheme,
                         JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    super(new BorderLayout());
    this.seqs = seqs;
    this.colorScheme = colorScheme;
    this.jspSequence = jspSequence;
    this.drawSequence = drawSequence;
    this.drawBlackBox = drawBlackBox;
    this.drawColorBox = drawColorBox;
    this.drawNumber = drawNumber;

    setBackground(Color.white);
    MultiLineToolTipUI.initialize();
    graphicSequence = new Vector();
    graphicName = new Vector();

// find maximum seq length
    setMaxSeqLength();

    Box centerBox = new Box(BoxLayout.Y_AXIS);
    seqBox = new Box(BoxLayout.Y_AXIS);
    centerBox.add(seqBox);

    Box westBox = new Box(BoxLayout.Y_AXIS);
    seqNameBox = new Box(BoxLayout.Y_AXIS);
    westBox.add(seqNameBox);
    seqNamePanel = new JPanel(new BorderLayout());
    seqNamePanel.add(westBox,BorderLayout.CENTER);
    seqNamePanel.setBackground(Color.white);
    jspSequence.setRowHeaderView(seqNamePanel);

// draw residue/base numbering
    if(drawNumber)
    {
      numberDraw = new SequenceJPanel(10,MAXSEQLENGTH);
      graphicSequence.add(numberDraw);
      Box XBox = new Box(BoxLayout.X_AXIS);
      XBox.add(numberDraw);
      XBox.add(Box.createHorizontalGlue());
      seqBox.add(XBox);

      setNumberSize();
      SequenceNameJButton snjBlank = 
                 new SequenceNameJButton(new Sequence(" "),0);
      graphicName.add(snjBlank);
      seqNameBox.add(snjBlank);
    }

// draw names and sequences 
    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
      addSequence((Sequence)enum.nextElement(),false,0,0);

    westBox.add(Box.createVerticalGlue());
    centerBox.add(Box.createVerticalGlue());
    plotconsSeqBox = new Box(BoxLayout.Y_AXIS);
    centerBox.add(plotconsSeqBox);
    add(centerBox,BorderLayout.CENTER);
    
    int xfill = getNameWidth();
    seqNamePanel.setPreferredSize(new Dimension(xfill,2000));

  }
 
  public GraphicSequenceCollection(Vector seqs, JScrollPane jspSequence,
                         boolean drawSequence, boolean drawBlackBox,
                         boolean drawColorBox, boolean drawNumber,
                         JTextField statusField)
  {
    this(seqs,null,jspSequence,drawSequence,
         drawBlackBox,drawColorBox,drawNumber,statusField);
  }

  protected Vector getSequenceCollection()
  {
    return seqs;
  }

  protected Point getViewPosition() 
  {
    return jspSequence.getViewport().getViewPosition();
  }

  protected Rectangle getViewRect()
  {
    Rectangle r = jspSequence.getViewport().getViewRect();

// adjustment for the sequence names on the west
//  r.x = r.x - westBox.getWidth();
//  if(r.x < 0)
//    r.x = 0;
    return r;
  }

/**
*
* @param File matrix - scoring matrix
* @param int wsize window size to average scores over
*
*/
  protected void showConsensusPlot(File matrix, int wsize)
  {
    deleteConsensusPlot();
    SequenceJPanel sj = (SequenceJPanel)graphicSequence.get(0);
    int interval = sj.getSequenceResidueWidth();

    pc =  new PlotConsensus(matrix,seqs,wsize,interval,this);
    pc.setBackground(Color.white);
    
    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(pc);
    XBox.add(Box.createHorizontalGlue());
    plotconsSeqBox.add(Box.createVerticalStrut(plotConStrut));
    plotconsSeqBox.add(XBox);
    plotconsSeqBox.add(Box.createVerticalGlue());
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);
    setJScrollPaneViewportView();
  }

/**
*
* @param File matrix - scoring matrix
* @param int wsize window size to average scores over
*
*/
  protected void showConsensusPlot(Matrix mat, int wsize)
  {
    deleteConsensusPlot();
    SequenceJPanel sj = (SequenceJPanel)graphicSequence.get(0);
    int interval = sj.getSequenceResidueWidth();

    pc =  new PlotConsensus(mat,seqs,wsize,interval,this);
    pc.setBackground(Color.white);
   
    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(pc);
    XBox.add(Box.createHorizontalGlue());
    plotconsSeqBox.add(Box.createVerticalStrut(20));
    plotconsSeqBox.add(XBox);
    plotconsSeqBox.add(Box.createVerticalGlue());
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);
    setJScrollPaneViewportView();
  }

  protected void deleteConsensusPlot()
  {
    plotconsSeqBox.removeAll();
  }

  protected void setMaxSequenceLength(int max)
  {
    if(max > MAXSEQLENGTH)
      MAXSEQLENGTH = max;
  }

  private void setMaxSeqLength()
  {
    MAXSEQLENGTH = 0;
    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
    {
      Sequence seq = (Sequence)(enum.nextElement());
      if(seq.getSequence().length()>MAXSEQLENGTH)
        MAXSEQLENGTH = seq.getSequence().length();
    }
  }

  public int getMaxSeqLength()
  {
    return MAXSEQLENGTH;
  }

  protected void setSequenceSelection(boolean b)
  {
    Enumeration enum = graphicName.elements();
    while(enum.hasMoreElements())
    {
      SequenceNameJButton sbutt = (SequenceNameJButton)enum.nextElement();
      if(!sbutt.getText().equals(""))
        sbutt.setSelected(b);
    }
  }

  private void setNumberSize()
  {
    Dimension actual = numberDraw.getMaximumSize();
    int slen = numberDraw.getResidueWidth()*(int)(MAXSEQLENGTH*1.5);
    numberDraw.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
  }

  protected Color getColor(String s, int pos, String seqName)
  {
    int identical = 0;
    int nseqs = 0;
    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
    {
      nseqs++;
      Sequence seq = (Sequence)(enum.nextElement());
      if(!seqName.equals(seq.getName()))
      {
        if(pos < seq.getLength())
          if(seq.getResidue(pos).equalsIgnoreCase(s))
            identical++;
      }
    }
  
    if(identical+1 == nseqs)
      return Color.red;

    return Color.black;
  }

  private Sequence removeSequence(String name)
  {
    boolean removed = false;
    int index = 0;
    Enumeration enum = seqs.elements();
    Sequence seq = null;

    while(enum.hasMoreElements())
    {
      seq = (Sequence)enum.nextElement();
      if(seq.getName().equals(name))
      {
        removedSeqs.add(seq);
        removed = true;
        seqs.remove(seq);
        break;
      }
      index++;
    }

    if(!removed)
      return null;
    if(drawNumber)
      index++;

    seqBox.remove(index);
    seqNameBox.remove(index);
    graphicName.removeElementAt(index);
    graphicSequence.removeElementAt(index);
    return seq;
  }

/**
*
*  Delete a sequence from the sequence collection display
*  and resize the sequence panel display
*/
  protected void deleteSequence(String name)
  {
    removeSequence(name);
    setMaxSeqLength();
    numberDraw.setSequenceLength(MAXSEQLENGTH);
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);
    numberDraw.setMaximumSize(numberDraw.getPreferredSize());
    numberDraw.setMinimumSize(numberDraw.getPreferredSize());
    setJScrollPaneViewportView();
  }

  protected void moveSequence(String name, int i)
  {
    Sequence seq = removeSequence(name);
    addSequence(seq,true,0,0,i);
  }

  protected void idSort()
  {
    int nseqs = 0;

// get no. of sequences excl. consensus
    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
    {  
      String name = ((Sequence)enum.nextElement()).getName();
      if(!name.equals("Consensus"))
        nseqs++;
    }

    String seqName[] = new String[nseqs];
    int i = 0;
    enum = seqs.elements(); 
    while(enum.hasMoreElements())
    {
      String name = ((Sequence)enum.nextElement()).getName();
      if(!name.equals("Consensus"))
      {
        seqName[i] = new String(name);
        i++;
      }
    }

    Arrays.sort(seqName);
    for(i=0;i<nseqs;i++)
      moveSequence(seqName[i],i);
  }

/**
*
*  Add a sequence at a particular index to the sequence 
*  collection display and to the collection of sequences 
*  (seqs) with a specified y-padding.
*
*/
  protected void addSequence(Sequence seq, boolean addToSequences, 
                             int ypad, int fontSize, int index)
  {
    if(addToSequences)
      seqs.add(index,seq);
    if(drawNumber)
      index++;

    SequenceJPanel gs = new SequenceJPanel(seq,this,
                           drawSequence,drawBlackBox,drawColorBox,
                           colorScheme,ypad);
    graphicSequence.add(index,gs);

    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(gs);
    XBox.add(Box.createHorizontalGlue());
    seqBox.add(XBox,index);
    gs.setToolTipText("");   //enable tooltip display

    SequenceNameJButton snj = new SequenceNameJButton(seq,ypad);
    graphicName.add(index,snj);
    XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(Box.createHorizontalGlue());
    XBox.add(snj);
    seqNameBox.add(XBox,index);

    if(seq.getLength()>MAXSEQLENGTH)
      MAXSEQLENGTH = seq.getLength();

    Dimension actual = gs.getMaximumSize();
    int slen = gs.getResidueWidth()*(int)(MAXSEQLENGTH*1.2);
    gs.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
  }


/**
*
*  Add a sequence to the sequence collection display and
*  to the collection of sequences (seqs) with a specified
*  y-padding.
*
*/
  protected void addSequence(Sequence seq, boolean addToSequences,
                             int ypad, int fontSize)
  {
    if(addToSequences)
      seqs.add(seq);
    SequenceJPanel gs = new SequenceJPanel(seq,this,
                           drawSequence,drawBlackBox,drawColorBox,
                           colorScheme,fontSize,ypad);
    graphicSequence.add(gs);

    Box XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(gs);
    XBox.add(Box.createHorizontalGlue());
    seqBox.add(XBox);
    gs.setToolTipText("");   //enable tooltip display

    SequenceNameJButton snj = new SequenceNameJButton(seq,ypad);
    graphicName.add(snj);
    XBox = new Box(BoxLayout.X_AXIS);
    XBox.add(Box.createHorizontalGlue());
    XBox.add(snj);
    seqNameBox.add(XBox);

    if(seq.getLength()>MAXSEQLENGTH)
      MAXSEQLENGTH = seq.getLength();

    Dimension actual = gs.getMaximumSize();
    int slen = gs.getResidueWidth()*(int)(MAXSEQLENGTH*1.2);
    gs.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
  }


/**
*
* Get the sequence view size
*
*/
  public Dimension getViewSize()
  {
    hgt = 1;
    len = 0; 
    Enumeration enum = graphicSequence.elements();
    while(enum.hasMoreElements())
    {
      SequenceJPanel gs = (SequenceJPanel)enum.nextElement();
      hgt = hgt+gs.getSequenceHeight();
      if(len<gs.getSequenceWidth())
        len = gs.getSequenceWidth();
    }

    if(pc !=null) 
    {
      Dimension dplot = pc.getPreferredSize();
      hgt = hgt + (int)dplot.getHeight() + plotConStrut;
    }
    return new Dimension(len,hgt);
  }

/**
*
* Get the sequence name view size
*
*/
  public Dimension getNameViewSize()
  {
   
    hgtName = getNameHeight();
    lenName = getNameWidth();
    return new Dimension(lenName,hgtName);
  }

  public Dimension getPanelSize()
  {
    getViewSize();
    return new Dimension(len,hgt);
  }

  public int getNameHeight()
  {
    hgtName = 0;
    Enumeration enum = graphicName.elements();
    while(enum.hasMoreElements())
      hgtName = hgtName+
          ((SequenceNameJButton)enum.nextElement()).getPanelHeight();
    return hgtName;
  }

  public int getNameWidth()
  {
    lenName = 0;
    Enumeration enum = graphicName.elements();
    while(enum.hasMoreElements())
    {
      SequenceNameJButton gs = (SequenceNameJButton)enum.nextElement();
      if(lenName<gs.getPanelWidth())
        lenName = gs.getPanelWidth();
    }
    return lenName;
  }
 
  public void setNamePanelWidth(int x)
  {
    seqNamePanel.setPreferredSize(new Dimension(x,1000));
  }

  public int getPanelHeight()
  {
    getViewSize();
    return hgt;
  }

  public int getPanelWidth()
  {
//  getNameViewSize();
    getViewSize();
    return len;
  }

  public Vector getGraphicSequence()
  {
    return graphicSequence;
  }

  public void setDrawBoxes(boolean drawBlackBox)
  {
    this.drawBlackBox = drawBlackBox;
    Enumeration enum = graphicSequence.elements();
    while(enum.hasMoreElements())
      ((SequenceJPanel)(enum.nextElement())).setDrawBoxes(drawBlackBox);
    setJScrollPaneViewportView();
  }

  public void setDrawColor(boolean drawColorBox)
  {
    this.drawColorBox = drawColorBox;
    Enumeration enum = graphicSequence.elements();
    while(enum.hasMoreElements())
      ((SequenceJPanel)(enum.nextElement())).setDrawColor(drawColorBox);
    setJScrollPaneViewportView();
  }

  public void setFontSizeForCollection(int fs)
  {
    Enumeration enum = graphicSequence.elements();

    while(enum.hasMoreElements())
    {
      SequenceJPanel gs = (SequenceJPanel)enum.nextElement();
      gs.setFontSize(fs);
      Dimension actual = gs.getMaximumSize();
      int slen = gs.getResidueWidth()*(int)(MAXSEQLENGTH*1.2);
      gs.setMaximumSize(new Dimension(slen,(int)actual.getHeight()));
    }

    Enumeration enumName = graphicName.elements();
    while(enumName.hasMoreElements())
    {
      SequenceNameJButton snjp = (SequenceNameJButton)enumName.nextElement();
      snjp.setFontSize(fs);
      snjp.setMaximumSize(snjp.getPreferredSize());
    }
   
// rescale consensus plot
    if(pc != null)
    {
      SequenceJPanel sj = (SequenceJPanel)graphicSequence.get(0);
      int interval = sj.getSequenceResidueWidth();
      pc.setInterval(interval);
      pc.setPlotSize();
    }
   
//
    Dimension dpane = getPanelSize();
    setMinimumSize(dpane);
    setPreferredSize(dpane);

    setNamePanelWidth(getNameWidth());
    setJScrollPaneViewportView();
  }

  public int getFontSize()
  {
    return ((SequenceJPanel)graphicSequence.get(0)).getFontSize();
  }

/**
*
* Search sequences for a pattern and highlight matches. Set the
* viewport to that position.
*
* @param String pat pattern to match 
* @param int oldResPosition if this is a repeat of a search
*        this is the position the old search finished at
* @param boolean wrapAround true if the search should wrap
*        around the sequences
* @return int the matching position found (or -1 if none found)
*
*/
  public int findPattern(String pat, int oldResPosition,
                         boolean wrapAround)
  {
    int resWidth = ((SequenceJPanel)graphicSequence.get(0)).
                                           getSequenceResidueWidth();
    Rectangle r = getViewRect();
    int ypos = r.y;
    int xpos = r.x/resWidth;
    int viewWidth = r.width/resWidth;
    pat = pat.toLowerCase();
            
// highlight matching segments of seqs
    Enumeration enum = graphicSequence.elements();
    while(enum.hasMoreElements())
    {
      SequenceJPanel sjp = (SequenceJPanel)enum.nextElement();
      sjp.showPattern(pat);
    }

// move view to the next occurence of that pattern
    if(oldResPosition > -1)      // possibly this as well: (&& xpos < oldResPosition)
      xpos = oldResPosition;

    int newResPos = searchSequences(xpos,pat);

    if(newResPos > -1)
    {
      int mid = findMiddle(newResPos,viewWidth,pat);
      jspSequence.getViewport().setViewPosition(
                        new Point(mid*resWidth,ypos));
//    System.out.println("xpos "+xpos+" newResPos "+newResPos+
//                       " viewWidth "+viewWidth+" mid "+mid+
//                       " oldResPosition "+oldResPosition);
    }
    else if(wrapAround)     // search from start of seqs
    {
//    JOptionPane.show
      newResPos = searchSequences(0,pat);
//    System.out.println("SEARCH FROM START OF SEQUENCE");
      if(newResPos > -1)
      {
        int mid = findMiddle(newResPos,viewWidth,pat);
        jspSequence.getViewport().setViewPosition(
                         new Point(mid*resWidth,ypos));
      }
    }
    else if(!wrapAround)
      newResPos = oldResPosition;

    return newResPos+1;
  }

/**
*
* Search the sequences for the position in that matches
* the given pattern to search for. 
*
* @param int startSearch position at which the search is started
* @param String pat is the pattern to search
* @return int position in a sequence that next matches the pattern
*         (or -1 if none found)
*
*/
  private int searchSequences(int startSearch, String pat)
  {
    int newResPos = 0;
    int nfound = 0;

    Enumeration enum = seqs.elements();
    while(enum.hasMoreElements())
    {
      Sequence seq = (Sequence)enum.nextElement();
      int index = seq.getSequence().toLowerCase().indexOf(pat,startSearch);
      if(index > -1)
      {
        if(nfound == 0 || index < newResPos)
          newResPos = index;
        nfound++;
      }
    }
    if(nfound == 0)
      return -1;
    return newResPos;
  }

/**
*
* Locate the center of a vieport view that contains the
* defined position to display
*
* @param int newResPos position in the sequence to display
* @param int viewWidth width of the viewport
* @param String pat matching pattern
* @return int position to set the viewport to so that the
*         pattern is displayed in the middle of it 
*
*/
  private int findMiddle(int newResPos, int viewWidth, String pat)
  {
    int mid;
    int viewWidth2 = viewWidth/2;
    if(newResPos <= viewWidth2)
      mid = 0;
    else
      mid = newResPos-viewWidth2+(pat.length()/2);
    return mid;
  }

  public void setColorScheme(Hashtable colourTable)
  {
    this.colorScheme = colourTable;
    Enumeration enum = graphicSequence.elements();
    while(enum.hasMoreElements())
      ((SequenceJPanel)(enum.nextElement())).setColorScheme(colourTable);
  }
 
  public void setPrettyPlot(boolean bpretty)
  {
    Enumeration enum = graphicSequence.elements();
    while(enum.hasMoreElements())
      ((SequenceJPanel)(enum.nextElement())).setPrettyPlot(bpretty);
  }
 
  public void setJScrollPaneViewportView()
  {
    jspSequence.setViewportView(this);
  }

  public int getNumberPages(PageFormat format)
  {
    return getNumberPages(format,getResiduesPerLine(format));
  }

  public int getNumberPages(PageFormat format, int numResPerLine)
  {
    double pageHeight = format.getImageableHeight();
    int residueWidth  = ((SequenceJPanel)graphicSequence.get(0)).getSequenceHeight();
//  int numResPerLine = getResiduesPerLine(format);
    int nblockPerPage = (int)(pageHeight/((graphicSequence.size()+2)*residueWidth));
    int npage         = MAXSEQLENGTH/(nblockPerPage*numResPerLine)+1;
    return npage;
  }

  public int getResiduesPerLine(PageFormat format)
  {
    double pwidth = format.getImageableWidth()-(double)getNameWidth();
    int residueWidth = ((SequenceJPanel)graphicSequence.get(0)).getSequenceHeight();
    return (int)(pwidth/(double)residueWidth);
  }

  public int getResiduesPerPage(PageFormat format, int numResPerLine)
  {
    double pageHeight = format.getImageableHeight();
    int residueWidth  = ((SequenceJPanel)graphicSequence.get(0)).getSequenceHeight();
//  int numResPerLine = getResiduesPerLine(format);
    int nblockPerPage = (int)(pageHeight/(residueWidth*(graphicSequence.size()+2)));
    return nblockPerPage*numResPerLine;
  }


  public int print(Graphics g, PageFormat format, int pageIndex) 
                                             throws PrinterException
  {
    Graphics2D g2d = (Graphics2D) g.create();
    drawSequences(g2d,format,pageIndex,numResiduePerLine);
    return Printable.PAGE_EXISTS;
  }

  protected void setNumberOfResiduesPerLine(int numResiduePerLine)
  {
    this.numResiduePerLine = numResiduePerLine;
  }

  public void drawSequences(Graphics2D g2d, PageFormat format, 
                            int pageIndex)
  {
    int numResPerLine = getResiduesPerLine(format);
    drawSequences(g2d,format,pageIndex,numResPerLine);
  }

  public void drawSequences(Graphics2D g2d, PageFormat format, 
                            int pageIndex, int numResPerLine)
  {
    // move origin from the corner of the Paper to the corner of imageable area
    g2d.translate(format.getImageableX(), format.getImageableY());

//  int numResPerLine = getResiduesPerLine(format);
    int resPerPage  = getResiduesPerPage(format,numResPerLine);

    int istart = resPerPage*pageIndex;
    int istop  = istart+resPerPage;

    if(istop > MAXSEQLENGTH)
      istop = MAXSEQLENGTH;

    System.out.println("pageIndex "+pageIndex+" numResPerLine "+numResPerLine);
    for(int i=istart;i<istop;i+=numResPerLine)
    {
      Enumeration enum = graphicSequence.elements();
      SequenceJPanel gs = null;
      while(enum.hasMoreElements())
      {
        gs = (SequenceJPanel)(enum.nextElement());
        gs.getSequencePrintGraphic(g2d,getNameWidth(),i,i+numResPerLine);
        gs.getNamePrintGraphic(g2d);
        g2d.translate(0,gs.getSequenceHeight());
      }
      g2d.translate(0,gs.getSequenceHeight());
    }
  }

//scrollable interface methods
  public Dimension getPreferredScrollableViewportSize()
  {
    return getPreferredSize();
  }

  public boolean getScrollableTracksViewportHeight()
  {
    return false;
  }

  public boolean getScrollableTracksViewportWidth()
  {
    return false;
  }

  public int getScrollableBlockIncrement(Rectangle r,
                    int orientation, int direction)
  {
    return 60;
  }

  public int getScrollableUnitIncrement(Rectangle r,
                    int orientation, int direction)
  {
    return 60;
  }


  public static void main(String args[])
  {
    Vector seqs = new Vector();
    seqs.add(new Sequence("","ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"+
             "ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"+
             "ACCaaaaaaaaaaaaaaaaaaaaTAGAtTAT"));


    JScrollPane jspSequence = new JScrollPane();
    GraphicSequenceCollection gsc = new GraphicSequenceCollection(
                                          seqs,null,jspSequence,
                                          true,true,true,false,null);
    jspSequence.setViewportView(gsc); 
    JFrame f = new JFrame("Sequence Panel");
    JPanel pane = (JPanel)f.getContentPane();

    pane.add(jspSequence);
    f.pack();
    f.setVisible(true);

  }

}

